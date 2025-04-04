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
 ***************************************************************************/

/****************************************************************************
	Abstract:

***************************************************************************/

#ifndef __DOT11K_RRM_H
#define __DOT11K_RRM_H

#ifdef DOT11K_RRM_SUPPORT
#include "rtmp_type.h"

#define RRM_CAP_BIT				(1 << 12) /* Bit 12 in Capability information field. */

#define RRM_PHY_FHSS		1
#define RRM_PHY_DSSS		2
#define RRM_PHY_IRBASEBAND	3
#define RRM_PHY_OFDM		4
#define RRM_PHY_HRDSSS		5
#define RRM_PHY_ERP			6
#define RRM_PHY_HT			7

#define IE_RRM_NEIGHBOR_REP		52
#define IE_BSS_AVAILABLE_AC		67
#define IE_BSS_AC_DELAY			68	/* Bss Ac Access Dealy Element, Same as WAPI IE. */
#define IE_RRM_EN_CAP			70	/* 802.11k. RRM Enable Capability element. */

#define RRM_MEASURE_REQ			0
#define RRM_MEASURE_REP			1
#define RRM_LNK_MEASURE_REQ		2
#define RRM_LNK_MEASURE_RSP		3
#define RRM_NEIGHTBOR_REQ		4
#define RRM_NEIGHTBOR_RSP		5

#define RRM_MEASURE_SUBTYPE_BASIC			0
#define RRM_MEASURE_SUBTYPE_CCA				1
#define RRM_MEASURE_SUBTYPE_RPI_HISTOGRAM	2
#define RRM_MEASURE_SUBTYPE_CH_LOAD			3
#define RRM_MEASURE_SUBTYPE_NOISE_HISTOGRAM	4
#define RRM_MEASURE_SUBTYPE_BEACON			5
#define RRM_MEASURE_SUBTYPE_LCI				8 /*location add*/
#define RRM_MEASURE_SUBTYPE_TX_STREAM		9
#define RRM_MEASURE_SUBTYPE_LOCATION_CIVIC	11 /*location add*/
#define RRM_MEASURE_SUBTYPE_LOCATION_ID		12 /*location add*/



#define RRM_NEIGHBOR_REQ_SSID_SUB_ID	0
#define RRM_NEIGHBOR_REQ_VENDOR_SUB_ID	221
#define RRM_NEIGHBOR_REQ_MEASUREMENT_REQUEST_SUB_ID	38  /*location add*/


#define RRM_NEIGHBOR_REP_TSF_INFO_SUB_ID			1
#define RRM_NEIGHBOR_REP_COUNTRY_STRING_SUB_ID		2
#define RRM_NEIGHBOR_REP_MEASUREMENT_REPORT_SUB_ID	39  /*location add*/
#define RRM_NEIGHBOR_REP_MEASURE_PILOT_TX_SUB_ID	66
#define RRM_ENABLE_CAPABILTY_SUB_ID					70
#define RRM_MULTIPLE_BSSID_SUB_ID					71
#define RRM_VENDOR_SUB_ID							221

#define RRM_BCN_REQ_MODE_PASSIVE	0
#define RRM_BCN_REQ_MODE_ACTIVE		1
#define RRM_BCN_REQ_MODE_BCNTAB		2

#define RRM_BCN_REQ_SUBID_SSID			0
#define RRM_BCN_REQ_SUBID_BCN_REP_INFO	1
#define RRM_BCN_REQ_SUBID_RET_DETAIL	2
#define RRM_BCN_REQ_SUBID_REQUEST		10
#define RRM_BCN_REQ_SUBID_AP_CH_REP		51
#define RRM_BCN_REQ_SUBID_VENDOR		221

#define RRM_BCN_REP_SUBID_REPORT_FRAME_BODY		1
#define RRM_BCN_REP_SUBID_VENDOR				221

#define RRM_TX_STREAM_SUBID_TRIGGER_REPORT		1
#define RRM_TX_STREAM_SUBID_VENDOR				221

/*
 * IEEE Std 802.11-2016, Table 9-90 - Reporting Detail values
 */
enum beacon_report_detail {
	/* No fixed-length fields or elements */
	BEACON_REPORT_DETAIL_NONE = 0,
	/* All fixed-length fields and any requested elements in the Request
	 * element if present */
	BEACON_REPORT_DETAIL_REQUESTED_ONLY = 1,
	/* All fixed-length fields and elements (default, used when Reporting
	 * Detail subelement is not included in a Beacon request) */
	BEACON_REPORT_DETAIL_ALL_FIELDS_AND_ELEMENTS = 2,
};

typedef struct GNU_PACKED _RRM_SUBFRAME_INFO {
	UINT8 SubId;
	UINT8 Length;
	UINT8 Oct[0];
} RRM_SUBFRAME_INFO, *PRRM_SUBFRAME_INFO;

typedef struct GNU_PACKED _RRM_BEACON_REQ_INFO {
	UINT8 RegulatoryClass;
	UINT8 ChNumber;
	UINT16 RandomInterval;
	UINT16 MeasureDuration;
	UINT8 MeasureMode;
	UINT8 Bssid[MAC_ADDR_LEN];
	UINT8 Option[0];
} RRM_BEACON_REQ_INFO, *PRRM_BEACON_REQ_INFO;

typedef union GNU_PACKED _RRM_BEACON_REP_INFO_FIELD {
	struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
		UINT8 ReportFrameType:1;
		UINT8 CondensePhyType:7;
#else
		UINT8 CondensePhyType:7;
		UINT8 ReportFrameType:1;
#endif
	} field;
	UINT8 word;
} RRM_BEACON_REP_INFO_FIELD, *PRRM_BEACON_REP_INFO_FIELD;

typedef struct GNU_PACKED _RRM_BEACON_REP_INFO {
	UINT8 RegulatoryClass;
	UINT8 ChNumber;
	UINT64 ActualMeasureStartTime;
	UINT16 MeasureDuration;
	UINT8 RepFrameInfo;
	UINT8 RCPI;
	UINT8 RSNI;
	UINT8 Bssid[MAC_ADDR_LEN];
	UINT8 AnntaId;
	UINT32 ParentTSF;
	UINT8 Option[0];
} RRM_BEACON_REP_INFO, *PRRM_BEACON_REP_INFO;
#ifndef WAPP_SUPPORT
typedef union GNU_PACKED _RRM_BSSID_INFO {
	struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
		UINT32 Reserved:14;
		UINT32 _20_TU_ProbeRspActive:1;
		UINT32 CO_locatedAP:1;
		UINT32 HE_ER_BSS:1;
		UINT32 HE:1;
		UINT32 FTM:1;
		UINT32 VHT:1;
		UINT32 HT:1;
		UINT32 MobilityDomain:1;
		UINT32 ImmediateBA:1;
		UINT32 DelayBlockAck:1;
		UINT32 RRM:1;
		UINT32 APSD:1;
		UINT32 Qos:1;
		UINT32 SpectrumMng:1;
		UINT32 KeyScope:1;
		UINT32 Security:1;
		UINT32 APReachAble:2;
#else
		UINT32 APReachAble:2;
		UINT32 Security:1;
		UINT32 KeyScope:1;
		UINT32 SpectrumMng:1;
		UINT32 Qos:1;
		UINT32 APSD:1;
		UINT32 RRM:1;
		UINT32 DelayBlockAck:1;
		UINT32 ImmediateBA:1;
		UINT32 MobilityDomain:1;
		UINT32 HT:1;
		UINT32 VHT:1;
		UINT32 FTM:1;
		UINT32 HE:1;
		UINT32 HE_ER_BSS:1;
		UINT32 CO_locatedAP:1;
		UINT32 _20_TU_ProbeRspActive:1;
		UINT32 Reserved:14;
#endif
	} field;
	UINT32 word;
} RRM_BSSID_INFO, *PRRM_BSSID_INFO;
#endif
typedef struct GNU_PACKED _RRM_NEIGHBOR_REP_INFO {
	UINT8 Bssid[MAC_ADDR_LEN];
	UINT32 BssidInfo;
	UINT8 RegulatoryClass;
	UINT8 ChNum;
	UINT8 PhyType;
	UINT8 Oct[0];
} RRM_NEIGHBOR_REP_INFO, *RRM_PNEIGHBOR_REP_INFO;

typedef union GNU_PACKED __RRM_EN_CAP_IE {
	struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
		UINT64 Reserved:28;
		UINT64 CIVICMeasureCap:1;
		UINT64 FTMRangeReportCapability:1;
		UINT64 AntennaInfoCap:1;
		UINT64 BssAvaiableAcmCap:1;
		UINT64 BssAvgAccessDelayCap:1;
		UINT64 RSNIMeasureCap:1;
		UINT64 RCPIMeasureCap:1;
		UINT64 NeighReportTSFOffsetCap:1;
		UINT64 MeasurePilotTxInfoCap:1;
		UINT64 MeasurePilotCap:3;
		UINT64 NotOperatingChMaxMeasureDuration:3;
		/*UINT64 RRMMibCap:1; */
		UINT64 OperatingChMaxMeasureDuration:3;
		UINT64 RRMMibCap:1;
		UINT64 APChannelReportCap:1;
		UINT64 TriggeredTransmitStreamCap:1;
		UINT64 TransmitStreamCap:1;
		UINT64 LCIAzimuthCap:1;
		UINT64 LCIMeasureCap:1;
		UINT64 StatisticMeasureCap:1;
		UINT64 NoiseHistogramMeasureCap:1;
		UINT64 ChannelLoadMeasureCap:1;
		UINT64 FrameMeasureCap:1;
		UINT64 BeaconMeasureReportCndCap:1;
		UINT64 BeaconTabMeasureCap:1;
		UINT64 BeaconActiveMeasureCap:1;
		UINT64 BeaconPassiveMeasureCap:1;
		UINT64 RepeatMeasureCap:1;
		UINT64 ParallelMeasureCap:1;
		UINT64 NeighborRepCap:1;
		UINT64 LinkMeasureCap:1;
#else
		UINT64 LinkMeasureCap:1;
		UINT64 NeighborRepCap:1;
		UINT64 ParallelMeasureCap:1;
		UINT64 RepeatMeasureCap:1;
		UINT64 BeaconPassiveMeasureCap:1;
		UINT64 BeaconActiveMeasureCap:1;
		UINT64 BeaconTabMeasureCap:1;
		UINT64 BeaconMeasureReportCndCap:1;
		UINT64 FrameMeasureCap:1;
		UINT64 ChannelLoadMeasureCap:1;
		UINT64 NoiseHistogramMeasureCap:1;
		UINT64 StatisticMeasureCap:1;
		UINT64 LCIMeasureCap:1;
		UINT64 LCIAzimuthCap:1;
		UINT64 TransmitStreamCap:1;
		UINT64 TriggeredTransmitStreamCap:1;
		UINT64 APChannelReportCap:1;
		UINT64 RRMMibCap:1;
		UINT64 OperatingChMaxMeasureDuration:3;
		UINT64 NotOperatingChMaxMeasureDuration:3;
		UINT64 MeasurePilotCap:3;
		UINT64 MeasurePilotTxInfoCap:1;
		UINT64 NeighReportTSFOffsetCap:1;
		UINT64 RCPIMeasureCap:1;
		UINT64 RSNIMeasureCap:1;
		UINT64 BssAvgAccessDelayCap:1;
		UINT64 BssAvaiableAcmCap:1;
		UINT64 AntennaInfoCap:1;
		UINT64 FTMRangeReportCapability:1;
		UINT64 CIVICMeasureCap:1;
		UINT64 Reserved:28;
#endif
	} field;
	UINT64 word;
} RRM_EN_CAP_IE, *PRRM_EN_CAP_IE;

typedef union GNU_PACKED _RRM_BSS_AVAILABLE_AC_BITMAP {
	struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
		UINT16 Reserved:4;
		UINT16 AC3:1;
		UINT16 AC2:1;
		UINT16 AC1:1;
		UINT16 AC0:1;
		UINT16 UP7:1;
		UINT16 UP6:1;
		UINT16 UP5:1;
		UINT16 UP4:1;
		UINT16 UP3:1;
		UINT16 UP2:1;
		UINT16 UP1:1;
		UINT16 UP0:1;
#else
		UINT16 UP0:1;
		UINT16 UP1:1;
		UINT16 UP2:1;
		UINT16 UP3:1;
		UINT16 UP4:1;
		UINT16 UP5:1;
		UINT16 UP6:1;
		UINT16 UP7:1;
		UINT16 AC0:1;
		UINT16 AC1:1;
		UINT16 AC2:1;
		UINT16 AC3:1;
		UINT16 Reserved:4;
#endif
	} field;
	UINT16 word;
} RRM_BSS_AVAILABLE_AC_BITMAP, *PRRM_BSS_AVAILABLE_AC_BITMAP;

typedef struct GNU_PACKED _RRM_BSS_AVAILABLE_AC_INFO {
	UINT16 AvailableAcBitMap;
	UINT8 Oct[0];
} RRM_BSS_AVAILABLE_AC_INFO, *PRRM_BSS_AVAILABLE_AC_INFO;

typedef struct GNU_PACKED _RRM_BSS_AC_DELAY_INFO {
	UINT8 BE_ACDelay;
	UINT8 BK_ACDelay;
	UINT8 VI_ACDelay;
	UINT8 VO_ACDelay;
} RRM_BSS_AC_DELAY_INFO, *PRRM_BSS_AC_DELAY_INFO;

typedef union GNU_PACKED _RRM_TRANSMIT_MEASURE_TRIGGER_CONDITION {
	struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
		UINT8 Reserved:5;
		UINT8 Delay:1;
		UINT8 Consecutive:1;
		UINT8 Average:1;
#else
		UINT8 Average:1;
		UINT8 Consecutive:1;
		UINT8 Delay:1;
		UINT8 Reserved:5;
#endif
	} field;
	UINT8 word;
} RRM_TRANSMIT_MEASURE_TRIGGER_CONDITION, *PRRM_TRANSMIT_MEASURE_TRIGGER_CONDITION;

typedef struct GNU_PACKED _RRM_TRANSMIT_MEASURE_DELAY_THRESHOLD {
	struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
		UINT8 DealyMsduCnt:6;
		UINT8 DealyMsduRange:2;
#else
		UINT8 DealyMsduRange:2;
		UINT8 DealyMsduCnt:6;
#endif
	} field;
	UINT8 word;
} RRM_TRANSMIT_MEASURE_DELAY_THRESHOLD, *PRRM_TRANSMIT_MEASURE_DELAY_THRESHOLD;

typedef struct GNU_PACKED _RRM_TRANSMIT_MEASURE_TRIGGER_REPORT {
	UINT8 TriggerCondition;
	UINT8 AvrErrorThreshold;
	UINT8 ConsecutiveErrorThreshold;
	UINT8 DelayThreshold;
	UINT8 MeasurementCnt;
	UINT8 TriggerTimeout;
} RRM_TRANSMIT_MEASURE_TRIGGER_REPORT, *PRRM_TRANSMIT_MEASURE_TRIGGER_REPORT;

typedef struct GNU_PACKED _RRM_TID {

} RRM_TID, *PRRM_TID;

typedef struct GNU_PACKED _RRM_TRANSMIT_MEASURE_INFO {
	UINT16 RandomInterval;
	UINT16 MeasureDuration;
	UINT8 PeerStaMac[MAC_ADDR_LEN];
	struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
		UINT8 TID:4;
		UINT8 Rev:4;
#else
		UINT8 Rev:4;
		UINT8 TID:4;
#endif
	}  TIDField;
	UINT8 Bin0Range;
	UINT8 Oct[0];
} RRM_TRANSMIT_MEASURE_INFO, *PRRM_TRANSMIT_MEASURE_INFO;

typedef union GNU_PACKED _RRM_MEASURE_REPORT_MODE {
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
} RRM_MEASURE_REPORT_MODE, *PRRM_MEASURE_REPORT_MODE;

typedef struct GNU_PACKED _RRM_MEASURE_REP_INFO {
	UINT8 Token;
	RRM_MEASURE_REPORT_MODE ReportMode;
	UINT8 ReportType;
	UINT8 Octect[0];
} RRM_MEASURE_REP_INFO, *PRRM_MEASURE_REP_INFO;

#endif /* DOT11K_RRM_SUPPORT */

#endif /* __DOT11K_RRM_H */

