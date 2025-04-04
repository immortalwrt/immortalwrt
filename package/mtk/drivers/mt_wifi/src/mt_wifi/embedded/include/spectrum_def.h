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
	spectrum_def.h

    Abstract:
    Handle association related requests either from WSTA or from local MLME

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
	Fonchi Wu    2008		   created for 802.11h
 */

#ifndef __SPECTRUM_DEF_H__
#define __SPECTRUM_DEF_H__


#define MAX_MEASURE_REQ_TAB_SIZE		32
/* Size of hash tab must be power of 2. */
#define MAX_HASH_MEASURE_REQ_TAB_SIZE	MAX_MEASURE_REQ_TAB_SIZE

#define MAX_TPC_REQ_TAB_SIZE			32
/* Size of hash tab must be power of 2. */
#define MAX_HASH_TPC_REQ_TAB_SIZE		MAX_TPC_REQ_TAB_SIZE

#ifdef TPC_SUPPORT
#ifdef TPC_MODE_CTRL
#define LOW_RATE_SENSIT -82/*dBm, refer to spec*/
#define HIGH_RATE_SENSIT -43/*dBm, refer to spec*/
DECLARE_TIMER_FUNCTION(WaitPeerTPCRepTimeout);
#else
#define MIN_RCV_PWR				100		/* Negative value ((dBm) */
#endif
#endif

#define TPC_REQ_AGE_OUT			500		/* ms */
#define MQ_REQ_AGE_OUT			500		/* ms */

#define TPC_DIALOGTOKEN_HASH_INDEX(_DialogToken)	((_DialogToken) & (MAX_HASH_TPC_REQ_TAB_SIZE - 1))
#define MQ_DIALOGTOKEN_HASH_INDEX(_DialogToken)		((_DialogToken) & (MAX_MEASURE_REQ_TAB_SIZE - 1))

typedef enum {
	REG_LOCAL,
	REG_GLOBAL
} REG_DOMAIN;

typedef struct _MEASURE_REQ_ENTRY {
	struct _MEASURE_REQ_ENTRY *pNext;
	ULONG lastTime;
	BOOLEAN	Valid;
	UINT8 DialogToken;
	UINT8 measuretype;
	UINT8 MeasureDialogToken[3];	/* 0:basic measure, 1: CCA measure, 2: RPI_Histogram measure. */
	BOOLEAN skip_time_check;
	UINT8 BcnCurrentState;
	UINT8 NrCurrentState;
	void *Priv;
	RALINK_TIMER_STRUCT WaitNRRspTimer;
	UINT8 StaMac[MAC_ADDR_LEN];
	UCHAR ControlIndex;
	UCHAR RcvBcnRepCnt;
	RALINK_TIMER_STRUCT WaitBCNRepTimer;
} MEASURE_REQ_ENTRY, *PMEASURE_REQ_ENTRY;

typedef struct _MEASURE_REQ_TAB {
	UCHAR Size;
	PMEASURE_REQ_ENTRY Hash[MAX_HASH_MEASURE_REQ_TAB_SIZE];
	MEASURE_REQ_ENTRY Content[MAX_MEASURE_REQ_TAB_SIZE];
} MEASURE_REQ_TAB, *PMEASURE_REQ_TAB;
#ifdef TPC_SUPPORT
typedef struct _TPC_REQ_ENTRY {
	struct _TPC_REQ_ENTRY *pNext;
	ULONG lastTime;
	BOOLEAN Valid;
	UINT8 DialogToken;
#ifdef TPC_MODE_CTRL
	void *Priv;
	UCHAR state;
	UINT16 wcid;
	UCHAR mac[MAC_ADDR_LEN];
	RALINK_TIMER_STRUCT WaitTPCRspTimer;
#endif
} TPC_REQ_ENTRY, *PTPC_REQ_ENTRY;

typedef struct _TPC_REQ_TAB {
	UCHAR Size;
	PTPC_REQ_ENTRY Hash[MAX_HASH_TPC_REQ_TAB_SIZE];
	TPC_REQ_ENTRY Content[MAX_TPC_REQ_TAB_SIZE];
} TPC_REQ_TAB, *PTPC_REQ_TAB;
#endif

/* The regulatory information */
typedef struct _DOT11_CHANNEL_SET {
	UCHAR NumberOfChannels;
	UINT8 MaxTxPwr;
	UCHAR ChannelList[16];
} DOT11_CHANNEL_SET;


typedef struct _DOT11_REGULATORY_INFO {
	UCHAR RegulatoryClass;
	DOT11_CHANNEL_SET ChannelSet;
} DOT11_REGULATORY_INFO;



#define RM_TPC_REQ				0
#define RM_MEASURE_REQ			1

#define RM_BASIC				0
#define RM_CCA					1
#define RM_RPI_HISTOGRAM		2
#define RM_CH_LOAD				3
#define RM_NOISE_HISTOGRAM		4
#define CS_ANN_MODE_TX_ALLOW 0
#define CS_ANN_MODE_TX_DENY 1

typedef struct GNU_PACKED _TPC_REPORT_INFO {
	INT8 TxPwr;
	INT8 LinkMargin;
} TPC_REPORT_INFO, *PTPC_REPORT_INFO;

typedef struct GNU_PACKED _CH_SW_ANN_INFO {
	UINT8 ChSwMode;
	UINT8 Channel;
	UINT8 ChSwCnt;
} CH_SW_ANN_INFO, *PCH_SW_ANN_INFO;

typedef union GNU_PACKED _MEASURE_REQ_MODE {
#ifdef RT_BIG_ENDIAN
	struct GNU_PACKED {

		UINT8 Reserved:3;
		UINT8 DurationMandatory:1;
		UINT8 Report:1;
		UINT8 Request:1;
		UINT8 Enable:1;
		UINT8 Parallel:1;
	} field;
#else
	struct GNU_PACKED {
		UINT8 Parallel:1;
		UINT8 Enable:1;
		UINT8 Request:1;
		UINT8 Report:1;
		UINT8 DurationMandatory:1;
		UINT8 Reserved:3;
	} field;
#endif /* RT_BIG_ENDIAN */
	UINT8 word;
} MEASURE_REQ_MODE, *PMEASURE_REQ_MODE;

typedef struct GNU_PACKED _MEASURE_REQ {
	UINT8 ChNum;
	UINT64 MeasureStartTime;
	UINT16 MeasureDuration;
} MEASURE_REQ, *PMEASURE_REQ;

typedef struct GNU_PACKED _MEASURE_REQ_INFO {
	UINT8 Token;
	MEASURE_REQ_MODE ReqMode;
	UINT8 ReqType;
	UINT8 Oct[0];
} MEASURE_REQ_INFO, *PMEASURE_REQ_INFO;

typedef union GNU_PACKED _MEASURE_BASIC_REPORT_MAP {
#ifdef RT_BIG_ENDIAN
	struct GNU_PACKED {
		UINT8 Rev:3;

		UINT8 Unmeasure:1;
		UINT8 Radar:1;
		UINT8 UnidentifiedSignal:1;
		UINT8 OfdmPreamble:1;
		UINT8 BSS:1;
	} field;
#else
	struct GNU_PACKED {
		UINT8 BSS:1;

		UINT8 OfdmPreamble:1;
		UINT8 UnidentifiedSignal:1;
		UINT8 Radar:1;
		UINT8 Unmeasure:1;
		UINT8 Rev:3;
	} field;
#endif /* RT_BIG_ENDIAN */
	UINT8 word;
} MEASURE_BASIC_REPORT_MAP, *PMEASURE_BASIC_REPORT_MAP;

typedef struct GNU_PACKED _MEASURE_BASIC_REPORT {
	UINT8 ChNum;
	UINT64 MeasureStartTime;
	UINT16 MeasureDuration;
	MEASURE_BASIC_REPORT_MAP Map;
} MEASURE_BASIC_REPORT, *PMEASURE_BASIC_REPORT;

typedef struct GNU_PACKED _MEASURE_CCA_REPORT {
	UINT8 ChNum;
	UINT64 MeasureStartTime;
	UINT16 MeasureDuration;
	UINT8 CCA_Busy_Fraction;
} MEASURE_CCA_REPORT, *PMEASURE_CCA_REPORT;

typedef struct GNU_PACKED _MEASURE_RPI_REPORT {
	UINT8 ChNum;
	UINT64 MeasureStartTime;
	UINT16 MeasureDuration;
	UINT8 RPI_Density[8];
} MEASURE_RPI_REPORT, *PMEASURE_RPI_REPORT;

typedef union GNU_PACKED _MEASURE_REPORT_MODE {
	struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
		UINT8 Rev:5;
		UINT8 Refused:1;
		UINT8 Incapable:1;
		UINT8 Late:1;
#else
		UINT8 Late:1;
		UINT8 Incapable:1;
		UINT8 Refused:1;
		UINT8 Rev:5;
#endif /* RT_BIG_ENDIAN */
	} field;
	UINT8 word;
} MEASURE_REPORT_MODE, *PMEASURE_REPORT_MODE;

typedef struct GNU_PACKED _MEASURE_REPORT_INFO {
	UINT8 Token;
	UINT8 ReportMode;
	UINT8 ReportType;
	UINT8 Octect[0];
} MEASURE_REPORT_INFO, *PMEASURE_REPORT_INFO;

#ifdef QUIET_SUPPORT
typedef struct GNU_PACKED _QUIET_INFO {
	UINT8 QuietCnt;
	UINT8 QuietPeriod;
	UINT16 QuietDuration;
	UINT16 QuietOffset;
} QUIET_INFO, *PQUIET_INFO;
#endif

typedef struct GNU_PACKED _EXT_CH_SW_ANN_INFO {
	UINT8 ChSwMode;
	UINT8 RegClass;
	UINT8 Channel;
	UINT8 ChSwCnt;
} EXT_CH_SW_ANN_INFO, *PEXT_CH_SW_ANN_INFO;
#endif /* __SPECTRUM_DEF_H__ */

