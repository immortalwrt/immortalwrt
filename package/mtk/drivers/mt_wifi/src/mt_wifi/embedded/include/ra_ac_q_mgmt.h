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
	ra_ac_q_mgmt.h
*/

#include    "rt_config.h"

#ifndef _RA_AC_Q_MGMT_H_
#define _RA_AC_Q_MGMT_H_

#if defined(RED_SUPPORT) && (defined(MT7622) || defined(P18) || defined(MT7663) || defined(AXE) || defined(MT7626))
#define	RED_SUPPORT_BY_HOST
#endif

#define CFG_RED_TRUN_ON_RANDOM_DROP					(0)
#define CFG_ESTIMATION_RED_CPU_UTILIZATION			(0)

#define RED_STA_REC_NUM			MAX_LEN_OF_MAC_TABLE

#define QLEN_SCALED_BIT								(0)
#define PROB_SCALED_BIT								(0)
#define QTH_WRIGTH_BIT								(2)
#define WSCALING_BIT								(16)
#define QLEN_SCALED									(1 << QLEN_SCALED_BIT)
#define PROB_SCALED									(1 << PROB_SCALED_BIT)

#define RED_DROP_TH_LOWER_BOUND						(20)
#define RED_DROP_TH_UPPER_BOUND						(600)

#define RED_VHT_BW20_DEFAULT_THRESHOLD 				(384)
#define RED_VHT_BW40_DEFAULT_THRESHOLD 				(768)
#define RED_VHT_BW80_DEFAULT_THRESHOLD 				(1536)
#define RED_HT_BW20_DEFAULT_THRESHOLD				(192)
#define RED_HT_BW40_DEFAULT_THRESHOLD				(384)
#define RED_LEGACY_DEFAULT_THRESHOLD				(192)

#define RED_WLOG_DEFAULT							(10)
#define RED_MPDU_TIME_INIT							(200)
#define RED_MULTIPLE_NUM_DEFAULT					(30)
#define RA_AC_FREE_FOR_ALL							(3072)

#define RED_BAD_NODE_DROP_THRESHOLD 				(192)
#define RED_BAD_NODE_HT_VHT_DEFAULT_THRESHOLD		(192)
#define RED_BAD_NODE_LEGACY_DEFAULT_THRESHOLD 		(60)
#define	RED_MAX_BAD_NODE_CNT						(10)
#define	RED_MAX_GOOD_NODE_CNT						(7)
#define	RED_BAD_NODE_CNT_MASK						(0x0f)
#define	RED_GOOD_NODE_CNT_MASK						(0x70)
#define	RED_IS_BAD_NODE_MASK						(0x80)
#define	RED_GOOD_NODE_CNT_SHIFT_BIT					(4)
#define	RED_IS_BAD_NODE_SHIFT_BIT					(7)

#define VOW_FEATURE_CONFIGURE_CR					(0x82060370)
#define VOW_WATF_MASK								(0x02000000)
#define VOW_ATF_MASK								(0x20000000)
#define VOW_ATC_SHIFT_BIT							(31)
#define VOW_ATF_SHIFT_BIT							(29)
#define VOW_WATF_SHIFT_BIT							(25)

#define FORCE_RATIO_THRESHOLD						(25)

#define TX_DONE_EVENT_Q_IDX_MASK					(0xf8000000)
#define TX_DONE_EVENT_Q_IDX_SHIFT_BIT				(27)
#define TX_DONE_EVENT_WLAN_ID_MASK					(0x03ff0000)
#define TX_DONE_EVENT_WLAN_ID_SHIFT_BIT				(16)

#define RED_INUSE_BITSHIFT					5
#define RED_INUSE_BITMASK					(0x1f)

/* per AC data structure */
typedef struct _RED_AC_ElEMENT_T {
	UINT32 u2TotalDropCnt;
	UINT16 u2DropCnt;
	UINT16 u2EnqueueCnt;
	UINT16 u2DequeueCnt;
	UINT16 u2qEmptyCnt;
	UINT8 ucShiftBit;
	UINT8 ucGBCnt;
#if (CFG_RED_TRUN_ON_RANDOM_DROP == 1)
	INT8 iWlogBit;
	UINT32 u4AvgLen;
	UINT16 u2qRan;
	UINT16 u2qCount;
	UINT16 u2DropProbCnt;
	UINT16 u2DropTailCnt;
#endif
} RED_AC_ElEMENT_T, *P_RED_AC_ElEMENT_T;

/* per STA data structure */
typedef struct _RED_STA_T{
	INT32 i4MpduTime;
	INT32 tx_msdu_avg_cnt;
	INT32 tx_msdu_cnt;
	UINT16 u4Dropth;
	UINT16 u2DriverFRCnt; /* Record ForceRate counter which is from Driver. */
	UINT8 ucMultiplyNum;
	RED_AC_ElEMENT_T arRedElm[WMM_NUM_OF_AC];
#if (CFG_RED_TRUN_ON_RANDOM_DROP == 1)
	BOOLEAN ucIsBadNode;
	UINT32 u4IGMPCnt;
	UINT32 u4TxmCnt;
#endif
} RED_STA_T, *P_RED_STA_T;

typedef enum {
	RED_DISABLE = 0,
	RED_BY_HOST_ENABLE,
	RED_BY_WA_ENABLE
} RED_ENABLE_TYPE_T;

/* Red token tail drop CMD format */
typedef enum _ENUM_RED_CMD_TYPE {
	RED_SET_GLOBAL_WATERMARK = 1,
	RED_SET_GLOBAL_TOKEN_WATERMARK = 2,
	RED_SET_GLOBAL_PAGE_WATERMARK = 3,
	RED_SET_GLOBAL_BAND_WATERMARK = 4,
	RED_SET_STA_THRES_BY_HOST = 5,
	RED_RELEASE_STA_THRES_FROM_HOST = 6,
	RED_SET_CTRL = 7,
	RED_DUMP_CTRL = 8,
	RED_DUMP_STATISTICS = 9,
	RED_CMD_MAX
} ENUM_RED_CMD_TYPE, *P_ENUM_RED_CMD_TYPE;

typedef enum _ENUM_TAIL_DROP_SCEN_T {
	TAIL_DROP_SCEN_NORMAL = 0,
	TAIL_DROP_SCEN_IXIA = 1
} ENUM_TAIL_DROP_SCEN_T, *P_ENUM_TAIL_DROP_SCEN_T;

typedef struct GNU_PACKED _CMD_RED_WATERMARK_T {
	UINT_16 u2AllTokenHighMark;
	UINT_16 u2AllTokenLowMark;
	UINT_16 u2TokenHighMark[BAND_NUM];
	UINT_16 u2TokenLowMark[BAND_NUM];
	UINT_16 u2PageHighMark;
	UINT_16 u2PageLowMark;
} CMD_RED_WATERMARK_T, *P_CMD_RED_WATERMARK_T;

typedef struct GNU_PACKED _CMD_RED_RAAC_THRES_T {
	UINT_16 u2Idx;
	UINT_8  ucAc;
	UINT_8  ucReserved;
	UINT_16 u2TokenHighMark;
	UINT_16 u2TokenLowMark;
	UINT_16 u2PageHighMark;
	UINT_16 u2PageLowMark;
	UINT_16 u2TokenComp;
	UINT_16 u2PageComp;
} CMD_RED_RAAC_THRES_T, *P_CMD_RED_RAAC_THRES_T;

typedef struct GNU_PACKED _CMD_RED_RELEASE_RAAC_THRES_T {
	UINT_16 u2Idx;
	UINT_8  ucAc;
	UINT_8  ucReserved;
} CMD_RED_RELEASE_RAAC_THRES_T, *P_CMD_RED_RELEASE_RAAC_THRES_T;

typedef struct GNU_PACKED _CMD_RED_SET_CTRL_T {
	BOOLEAN fgEnable;
	BOOLEAN fgDbgShow;
	BOOLEAN fgDbgNoDrop;
	UINT_8 tx_bh_period;
	UINT_8 rx_bh_period;
	UINT_8 u1PfmEvent1;
	UINT_8 u1PfmEvent2;
	UINT_8 aucReserved[1];
} CMD_RED_SET_CTRL_T, *P_CMD_RED_SET_CTRL_T;

typedef struct GNU_PACKED _CMD_RED_HEADER_T {
	/*Common Part*/
	UINT_8  ucOpMode;
	UINT_8  ucCmdVer;
	UINT_8  aucPadding0[2+2];
	UINT_16 u2CmdLen;
} CMD_RED_HEADER_T, *P_CMD_RED_HEADER_T;

typedef struct _RED_CTRL_T {
	UINT_16 u2AllTokenLowMark;
	UINT_16 u2AllTokenHighMark;
	UINT_16 u2TokenLowMark[BAND_NUM];
	UINT_16 u2TokenHighMark[BAND_NUM];
	UINT_16 u2PageLowMark;
	UINT_16 u2PageHighMark;
	UINT_32 u4AllTokenFullCnt;
	UINT_32 u4TokenFullCnt[BAND_NUM];
	UINT_32 u4PageFullCnt;
	UINT_8 tx_bh_period;
	UINT_8 rx_bh_period;
	UINT_8 u1PfmEvent1;
	UINT_8 u1PfmEvent2;
	BOOLEAN fgEnable;
	BOOLEAN fgDbgShow;
	BOOLEAN fgDbgNoDrop;
	UINT_32 u4CheckDropCnt;
	UINT_32 u4TotInputCnt;
	UINT_32 u4IterCnt;
	UINT_32 u4DropCnt[BAND_NUM];
	ENUM_TAIL_DROP_SCEN_T    eScenario;
} RED_CTRL_T, *P_RED_CTRL_T;


VOID RedInit(PRTMP_ADAPTER pAd);

VOID RedResetSta(UINT16 u2WlanIdx, UINT_8 ucMode, UINT_8 ucBW, struct _RTMP_ADAPTER *pAd);

VOID RedBadNode(UINT16 u2WlanIdx, UINT8 ATC_WATF_Enable, struct _RTMP_ADAPTER *pAd);

bool RedMarkPktDrop(UINT16 u2WlanIdx, UINT8 ucQidx, struct _RTMP_ADAPTER *pAd);

bool red_mark_pktdrop_cr4(UINT16 u2WlanIdx, UINT8 ucQidx, struct _RTMP_ADAPTER *pAd);

VOID red_record_data(PRTMP_ADAPTER pAd, UINT16 u2WlanIdx, PNDIS_PACKET pPacket);

VOID RedSetTargetDelay(INT16 i2TarDelay, struct _RTMP_ADAPTER *pAd);

INT32 RedCalProbB(UINT16 u2WlanIdx, UINT8 ac);

/* VOID RedUpdatePBC(P_WIFI_CMD_T prWiFiCmd); */

VOID RedRecordForceRateFromDriver(RTMP_ADAPTER *pAd, UINT16 wcid);

VOID RedCalForceRateRatio(UINT16 u2Wcid, UINT16 u2N9ARCnt, UINT16 u2N9FRCnt, struct _RTMP_ADAPTER *pAd);

VOID red_tx_free_handle(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt);

VOID appShowRedDebugMessage(struct _RTMP_ADAPTER *pAd);

VOID UpdateWlogBit(UINT16 u2WlanIdx, UINT32 u4Mpdutime);

VOID UpdateThreshold(UINT16 u2WlanIdx, RTMP_ADAPTER *pAd);

VOID UpdateAirtimeRatio(UINT16 u2WlanIdx, UINT8 ucAirtimeRatio, UINT8 ATCEnable, struct _RTMP_ADAPTER *pAd);

VOID UpdateTargetDelay(UINT8 ATC_WATF_Enable, struct _RTMP_ADAPTER *pAd);

int uint32_log2(UINT32 n);

INT set_red_enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT set_red_show_sta(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT set_red_target_delay(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT set_red_debug_enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT show_red_info(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT set_red_dump_reset(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT set_red_drop(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

VOID red_qlen_drop_setting(PRTMP_ADAPTER pAd, UINT8 op);

INT set_red_config(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

BOOLEAN SendRedCmd(RTMP_ADAPTER *pAd, UINT_8 ucOpMode, UINT_8 *pr_red_param, UINT_16 sta_num);

#endif /* _RA_AC_Q_MGMT_H_ */


