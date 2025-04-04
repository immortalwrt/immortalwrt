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
	icap.h
*/

#ifndef __ICAP_H_
#define __ICAP_H_

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
/* RBISTCR */
#define RBIST_BASE											0x810C0000
#define RBISTCR0											(RBIST_BASE + 0x90)
#define RBISTCR1											(RBIST_BASE + 0x94)
#define RBISTCR2											(RBIST_BASE + 0x98)
#define RBISTCR3											(RBIST_BASE + 0x9c)
#define RBISTCR4											(RBIST_BASE + 0xa0)
#define RBISTCR5											(RBIST_BASE + 0xa4)
#define RBISTCR6											(RBIST_BASE + 0xa8)
#define RBISTCR7											(RBIST_BASE + 0xac)
#define RBISTCR8											(RBIST_BASE + 0xb0)
#define RBISTCR9											(RBIST_BASE + 0xb4)
#define RBISTCR10											(RBIST_BASE + 0xb8)

/* RBISTCR0 */
#define ICAP_WRAP											17
#define CR_RBIST_CAPTURE									1
#define ICAP_SRC_SEL1										20
#define ICAP_SRC_SEL3										22

/* RBISTCR10 */
#define SYSRAM_INTF_SEL1									26
#define SYSRAM_INTF_SEL2									27
#define SYSRAM_INTF_SEL3									28

/* CR_DBGSGD_MODE */
#define CR_SGD_MODE1										0
#define CR_SGD_MODE4										3
#define CR_SGD_DBG_SEL										15

/* Trigger Event */
#define CAP_FREE_RUN										0

/* Ring Mode */
#define CAP_RING_MODE_ENABLE								1
#define CAP_RING_MODE_DISABLE								0

/* Capture Bit Width (legacy) */
#define CAP_32_BIT											0
#define CAP_96_BIT											1
#define CAP_128_BIT											2

/* Capture Bit Width (new) */
#define CAP_BW_128B_TO_128B									0
#define CAP_BW_96B_TO_128B									1
#define CAP_BW_24B_TO_32B									2
#define CAP_BW_BW_32B_TO_64B								3
#define CAP_BW_64B_TO_128B									4

/* Capture Architecture */
#define CAP_ON_CHIP											0
#define CAP_ON_THE_FLY										1

/* Capture Source */
#define CAP_WIFI_PHY										0x0
#define CAP_AFE												0x6

/* Capture Node Type */
#define CAP_RX_TYPE											0x0
#define CAP_TX_TYPE											0x1
#define CAP_DFS_TYPE										0x2
#define CAP_THERMAL_TYPE									0x3
#define CAP_INPHYSNIFFER_TYPE								0x4
#define CAP_SPECTRUM_TYPE									0x5
#define WF_COMM_CR_CAP_NODE_TYPE_MASK						BITS(28, 31)
#define WF_COMM_CR_CAP_NODE_TYPE_SHFT						28

/* Capture Node */
#define MT7615_CAP_WF0_ADC									0x000b
#define MT7615_CAP_WF1_ADC									0x000c
#define MT7615_CAP_WF2_ADC									0x000d
#define MT7615_CAP_WF3_ADC									0x000e
#define MT7615_CAP_WF0_FIIQ									0x200b
#define MT7615_CAP_WF1_FIIQ									0x200c
#define MT7615_CAP_WF2_FIIQ									0x200d
#define MT7615_CAP_WF3_FIIQ									0x200e
#define MT7615_CAP_WF0_FDIQ									0x300b
#define MT7615_CAP_WF1_FDIQ									0x300c
#define MT7615_CAP_WF2_FDIQ									0x300d
#define MT7615_CAP_WF3_FDIQ									0x300e
#define MT7622_CAP_FOUR_WAY_ADC								0x005d
#define MT7622_CAP_WF0_ADC									0x100b
#define MT7622_CAP_WF1_ADC									0x100c
#define MT7622_CAP_WF2_ADC									0x100d
#define MT7622_CAP_WF3_ADC									0x100e
#define MT7622_CAP_WF0_FIIQ									0x200b
#define MT7622_CAP_WF1_FIIQ									0x200c
#define MT7622_CAP_WF2_FIIQ									0x200d
#define MT7622_CAP_WF3_FIIQ									0x200e
#define MT7622_CAP_WF0_FDIQ									0x300b
#define MT7622_CAP_WF1_FDIQ									0x300c
#define MT7622_CAP_WF2_FDIQ									0x300d
#define MT7622_CAP_WF3_FDIQ									0x300e
#define MT7663_CAP_TWO_WAY_ADC								0x0006
#define MT7663_CAP_WF0_ADC									0x000b
#define MT7663_CAP_WF1_ADC									0x000c
#define MT7663_CAP_WF0_FIIQ									0x200b
#define MT7663_CAP_WF1_FIIQ									0x200c
#define MT7663_CAP_WF0_FDIQ									0x300b
#define MT7663_CAP_WF1_FDIQ									0x300c
#define MT7626_CAP_WF01_PACKED_ADC							0x0006
#define MT7626_CAP_WF12_PACKED_ADC							0x0076
#define MT7626_CAP_WF02_PACKED_ADC							0x0077
#define AXE_CAP_TWO_WAY_ADC									0x0082

/* Capture Bw */
#define CAP_BW_20											0
#define CAP_BW_40											1
#define CAP_BW_80											2

/* Capture Antenna */
#define CAP_WF0												0
#define CAP_WF1												1
#define CAP_WF2												2
#define CAP_WF3												3

/* I/Q Type */
#define CAP_I_TYPE											0
#define CAP_Q_TYPE											1

/* Parameter Setting */
#define CAP_MODE											0
#define CAP_TRIGGER											1
#define CAP_RING_MODE										2
#define CAP_BBP_EVENT										3
#define CAP_NODE											4
#define CAP_LENGTH											5
#define CAP_STOP_CYCLE										6
#define CAP_BW												7
#define CAP_WF_PATH										    8
#define CAP_SOURCE_ADDR										9
#define CAP_BAND											10
#define CAP_PHY												11
#define CAP_SOURCE											12
#define CAP_PD_ENABLE                                       13
#define CAP_FIX_RX_GAIN                                     14

/* Capture Status */
#define CAP_SUCCESS											0
#define CAP_BUSY											1
#define CAP_FAIL											2

/* Capture BitWidth */
#define CAP_32BITS											32
#define CAP_96BITS											96

/* Dump raw data expired time */
#define CAP_DUMP_DATA_EXPIRE								100000

/* Length of file naming */
#define CAP_FILE_MSG_LEN									64

/*
===================================
	WIFISPECTRUM DEFINE
===================================
*/
/* Wifi-Spectrum Event Data Sample Cnt */
#define SPECTRUM_EVENT_DATA_SAMPLE							256

/* Wifi-Spectrum CMD Response Length */
#define SPECTRUM_DEFAULT_RESP_LEN							0

/* Wifi-Spectrum Wait CMD Response Time */
#define SPECTRUM_DEFAULT_WAIT_RESP_TIME						0
#define SPECTRUM_WAIT_RESP_TIME								10000

/* MT7615 */
#define MT7615_SPECTRUM_TOTAL_SIZE							128/* Unit:KBytes */
/*
===================================
	ICAP DEFINE
===================================
*/
/* ICap Event Data Sample Cnt */
#define ICAP_EVENT_DATA_SAMPLE								256

/* MT7622 */
#define MT7622_ICAP_BANK_SAMPLE_CNT							4096
#define MT7622_ICAP_FOUR_WAY_ADC_IQ_DATA_CNT				(4096 * 6)/* (4096(Samples/Bank) * 6Banks * 3(IQSamples/Sample))/3Samples(96bits) */
#define MT7622_ICAP_FOUR_WAY_IQC_IQ_DATA_CNT				(4096 * 2)/* (4096(Samples/Bank) * 6Banks * 1(IQSamples/Sample))/3Samples(96bits) */

/* MT7663 */
#define MT7663_ICAP_BANK_SAMPLE_CNT							8192
#define MT7663_ICAP_TWO_WAY_ADC_IQ_DATA_CNT					(8192 * 4)/* (8192(Samples/Bank) * 6Banks * 2(IQSamples/Sample))/3Samples(96bits) */
#define MT7663_ICAP_TWO_WAY_IQC_IQ_DATA_CNT					(8192 * 2)/* (8192(Samples/Bank) * 6Banks * 1(IQSamples/Sample))/3Samples(96bits) */

/* AXE */
#define AXE_ICAP_BANK_SAMPLE_CNT							8192
#define AXE_ICAP_TWO_WAY_ADC_IQ_DATA_CNT					(32768 * 2)
#define AXE_ICAP_TWO_WAY_IQC_IQ_DATA_CNT					(32768 * 2)
#define AXE_ICAP_TWO_WAY_AFE_IQ_DATA_CNT					(16384 * 4)

/* MT7626 */
#define MT7626_ICAP_BANK_SAMPLE_CNT							8192
#define MT7626_ICAP_TWO_WAY_ADC_IQ_DATA_CNT					(3413 * 8)/* ((8192(Samples/Bank) + 2048(Samples/Bank)) * 4Banks * 2(IQSamples/Sample))/3Samples(96bits) */
#define MT7626_ICAP_THREE_WAY_IQC_IQ_DATA_CNT				(3413 * 4)/* ((8192(Samples/Bank) + 2048(Samples/Bank)) * 4Banks * 1(IQSamples/Sample))/3Samples(96bits) */
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

#if defined(PHY_ICS_SUPPORT)
#define ICS_MODE											0
#define ICS_TRIGGER											1
#define ICS_RING_MODE										2
#define ICS_BW												3
#define ICS_BAND											4
#define ICS_PHY												5
#define ICS_SOURCE											6
#define ICS_PARTITION										7
#define ICS_EVENT_GROUP										8
#define ICS_EVENT_ID_MSB									9
#define ICS_EVENT_ID_LSB									10
#define ICS_TIMER											11
#endif /* defined(PHY_ICS_SUPPORT) */

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
typedef enum _ICAP_WIFI_SPECTRUM_MODE {
	ICAP_MODE = 0,
	WIFI_SPECTRUM_MODE = 1,
	PHY_ICS_MODE = 2
} ICAP_WIFI_SPECTRUM_MODE;

typedef struct GNU_PACKED _RBIST_IQ_DATA_T {
	INT32 IQ_Array[4][2]; /* IQ_Array[WF][IQ] */
} RBIST_IQ_DATA_T, *P_RBIST_IQ_DATA_T;

typedef struct GNU_PACKED _RBIST_CAP_START_T {
	UINT32 fgTrigger;
	UINT32 fgRingCapEn;
	UINT32 u4TriggerEvent;
	UINT32 u4CaptureNode;
	UINT32 u4CaptureLen;    /* Unit : IQ Sample */
	UINT32 u4CapStopCycle;  /* Unit : IQ Sample */
	UINT32 u4PdEnable;
	UINT32 u4FixRxGain;
	UINT32 u4WifiPath;
	UINT32 u4BandIdx;
	UINT32 u4BW;
    UINT32 u4EnBitWidth;/* 0:32bit, 1:96bit, 2:128bit */
    UINT32 u4Architech;/* 0:on-chip, 1:on-the-fly */
    UINT32 u4PhyIdx;
    UINT32 u4EmiStartAddress;
    UINT32 u4EmiEndAddress;
    UINT32 u4EmiMsbAddress;
	UINT32 u4CapSource;
    UINT32 u4Reserved[2];
} RBIST_CAP_START_T, *P_RBIST_CAP_START_T;

typedef struct GNU_PACKED _RBIST_DUMP_RAW_DATA_T {
	UINT32 u4Address;
	UINT32 u4AddrOffset;
	UINT32 u4Bank;
	UINT32 u4BankSize;/* Uint:Kbytes */
	UINT32 u4WFNum;
	UINT32 u4IQType;
	UINT32 u4Reserved[6];
} RBIST_DUMP_RAW_DATA_T, *P_RBIST_DUMP_RAW_DATA_T;

typedef struct GNU_PACKED _RBIST_DESC_T {
	UINT32 u4Address;
	UINT32 u4AddrOffset;
	UINT32 u4Bank;
	UINT32 u4BankSize; /* Uint:KBytes */
	UINT8  ucBitWidth; /* Uint:Bit */
	UINT8  ucADCRes;   /* Uint:Bit */
	UINT8  ucIQCRes;   /* Uint:Bit */
	PUINT8 pLBank;
	PUINT8 pMBank;
	PUINT8 pHBank;
} RBIST_DESC_T, *P_RBIST_DESC_T;

typedef struct GNU_PACKED _EXT_CMD_SPECTRUM_CTRL_T {
	UINT32 u4FuncIndex;
	RBIST_CAP_START_T rSpectrumInfo;
	RBIST_DUMP_RAW_DATA_T rSpectrumDump;
} EXT_CMD_SPECTRUM_CTRL_T, *P_EXT_CMD_SPECTRUM_CTRL_T;

typedef struct _EXT_EVENT_SPECTRUM_RESULT_T {
	UINT32 u4FuncIndex;
	UINT32 u4FuncLength;
	UINT8  aucEvent[0];
} EXT_EVENT_SPECTRUM_RESULT_T, *PEXT_EVENT_SPECTRUM_RESULT_T;

typedef struct GNU_PACKED _EXT_EVENT_RBIST_ADDR_T {
	UINT32 u4FuncIndex;
	UINT32 u4FuncLength;
	UINT32 u4StartAddr1;
	UINT32 u4StartAddr2;
	UINT32 u4StartAddr3;
	UINT32 u4EndAddr;
	UINT32 u4StopAddr;
	UINT32 u4Wrap;
} EXT_EVENT_RBIST_ADDR_T, *P_EXT_EVENT_RBIST_ADDR_T;

typedef struct GNU_PACKED _EXT_EVENT_RBIST_DUMP_DATA_T {
	UINT32 u4FuncIndex;
	UINT32 u4PktNum;
	UINT32 u4Bank;
    UINT32 u4DataLen;
    UINT32 u4WFCnt;
    UINT32 u4SmplCnt;
    UINT32 u4Reserved[6];
	UINT32 u4Data[256];
} EXT_EVENT_RBIST_DUMP_DATA_T, *P_EXT_EVENT_RBIST_DUMP_DATA_T;

typedef struct GNU_PACKED _EXT_EVENT_RBIST_CAP_STATUS_T {
	UINT32 u4FuncIndex;
	UINT32 u4CapDone;
    UINT32 u4Reserved[15];
} EXT_EVENT_RBIST_CAP_STATUS_T, *P_EXT_EVENT_RBIST_CAP_STATUS_T;

typedef enum _ENUM_SPECTRUM_CTRL_FUNCID_T {
	SPECTRUM_CTRL_FUNCID_SET_PARAMETER = 0,
	SPECTRUM_CTRL_FUNCID_GET_CAPTURE_STATUS,
	SPECTRUM_CTRL_FUNCID_DUMP_RAW_DATA,
	SPECTRUM_CTRL_FUNCID_SET_PHY_ICS_PARAMETER
} ENUM_SPECTRUM_CTRL_FUNCID_T;
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

#if defined(PHY_ICS_SUPPORT)
typedef struct _PHY_ICS_START_T
{
	UINT32 fgTrigger;
	UINT32 fgRingCapEn;
	UINT32 u4BandIdx;
	UINT32 u4BW;
	UINT32 u4EnBitWidth;
	UINT32 u4Architech;
	UINT32 u4PhyIdx;
	UINT32 u4EmiStartAddress;
	UINT32 u4EmiEndAddress;
	UINT32 u4CapSource;
	UINT32 u4PhyIcsType;
	UINT32 u4PhyIcsEventGroup;
	UINT32 u4PhyIcsEventID[2];
	UINT32 u4PhyIcsTimer;
	UINT32 u4Reserved[2];
} PHY_ICS_START_T, *P_PHY_ICS_START_T;

typedef struct _EXT_CMD_PHY_ICS_CTRL_T
{
    UINT32 u4FuncIndex;
    PHY_ICS_START_T rPhyIcsInfo;
} EXT_CMD_PHY_ICS_CTRL_T, *P_EXT_CMD_PHY_ICS_CTRL_T;

typedef struct GNU_PACKED _EXT_EVENT_PHY_ICS_DUMP_DATA_T {
	UINT32 u4FuncIndex; /* 0x15 = 21 */
	UINT32 u4PktNum;
	UINT32 u4PhyTimestamp;
	UINT32 u4DataLen;
	UINT32 u4Reserved[5];
	UINT32 u4Data[256];
} EXT_EVENT_PHY_ICS_DUMP_DATA_T, *P_EXT_EVENT_PHY_ICS_DUMP_DATA_T;
#endif /* defined(PHY_ICS_SUPPORT) */

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/


#endif /* __ICAP_H_ */
