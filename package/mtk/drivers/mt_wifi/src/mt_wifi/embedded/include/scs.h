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

    Abstract:

 */

#ifndef __SCS_H__
#define __SCS_H__

#include "rt_config.h"


#define TriggerTrafficeTh							250000/*2M*/
#define MAX_LEN_OF_SCS_BSS_TABLE					6
#define MinRssiTolerance				10
#define ThTolerance                                 10
#define PdBlkEnabeOffset							19
#define RTSDropCntEnabeOffset						0
#define RTSDropRdClrEnabeOffset						8
#define RtsDropCountOffset							16
#define RTSDropCountMask							0x0000ffff
#define PdBlkEnabeOffsetB1							25
#define PdBlkOfmdThMask								0x1ff
#define PdBlkOfmdThOffset							20
#define PdBlkOfmdThOffsetB1							16 /* Band1 */
#define PdBlkCckThMask								0xff
#define PdBlkCckThOffset							1
#define PdBlkCck1RThOffset							24
#define PdBlkOfmdThDefault							0x13c
#define PdBlkCckThDefault							0x92 /*-110dBm*/
#define BandNum										1/*MT7615 HW not support DBDC/MT7622 only Band0*/
#define BssNumScs										4 /* BssNum coflicts with STA Mode definition */
#define BssOffset									0x10
#define RtsRtyCountOffset							16
#define RtsCountMask								0x0000ffff
#define TxTrafficTh									9
/* #define BandOffset		0x200 */
#define OneStep										2/* dB */
#define FastInitTh									0xa6/* -90dBm */
#define FastInitThOfdm								0x14c/* -90dBm */
#define SCS_STA_NUM									4

#define FalseCcaUpBondDefault						500
#define FalseCcaLowBondDefault						50
#define CckFixedRssiBondDefault						184/* -72dBm. -72+256 */
#define OfdmFixedRssiBondDefault					368/* -72dBm. (-72*2)+512 */
/*SCSGen4_for_MT7663*/
#define MT7663_PHY_MIN_PRI_PWR_OFFSET				0x54
#define MT7663_PHY_RXTD_CCKPD_7_OFFSET				0x3C
#define MT7663_PHY_RXTD_CCKPD_8_OFFSET				0x3C
/*SCSGen4_for_MT7663*/

enum {
	SCS_DISABLE,
	SCS_ENABLE,
	SCS_MANUAL,
};

enum {
	PD_BLOCKING_OFF,
	PD_BLOCKING_ON,
};

enum {
	SCS_Gen1,/* old chip */
	SCS_Gen2,/* MT7615   */
	SCS_Gen3,/* MT7622   */
	SCS_Gen4,/* MT7663 and CONNAC Project will unified and offload to fw  */
	SCS_Gen5,/* MT7626 offload to fw  */
	SCS_Gen6,/* MT7915, MT7986, MT7916 and MT7981 offload to fw  */
};

typedef struct _SCS_BSS_ENTRY {
	UCHAR Bssid[MAC_ADDR_LEN];
	/* UCHAR Channel; */
} SCS_BSS_ENTRY;

typedef struct {
	UINT           BssNr;
	UINT           BssOverlapNr;
	SCS_BSS_ENTRY       BssEntry[MAX_LEN_OF_SCS_BSS_TABLE]; /* Number is 6 */
} SCS_BSS_TABLE, *PSCS_BSS_TABLE;
#ifdef SCS_FW_OFFLOAD
enum {
	SCS_EVENT_SEND_DATA = 0,
	SCS_EVENT_SET_MANUAL_PD_TH,
	SCS_EVENT_CONFIG_SCS,
	SCS_EVENT_SCS_ENABLE,
	SCS_SHOW_INFO,
	SCS_GET_GLO_ADDR,
	SCS_EVENT_GET_GLO_ADDR,
	SCS_EVENT_SET_PD_THR_RANGE,
	SCS_MAX_EVENT
};

typedef struct _CMD_SMART_CARRIER_SENSE_CTRL_PD_TH {
	UINT8	BandIdx;
	UINT8	SCSEnable;
	UINT8	CckPdBlkTh;
	UINT16	OfdmPdBlkTh;
} CMD_SMART_CARRIER_SENSE_CTRL_PD_TH, *P_CMD_SMART_CARRIER_SENSE_CTRL_PD_TH;

typedef struct _CMD_SMART_CARRIER_SENSE_CONFIG {
	UINT8 u1BandIdx;
	UINT8 u1SCSThTolerance;
	UINT8 u1SCSMinRssiTolerance;
	BOOL fgOfdmPdSupport;
	UINT16 u2OfdmFalseCcaUpBound;
	UINT16 u2OfdmFalseCcaLowBound;
	UINT16 u2CckFixedRssiBound;
	UINT16 u2OfdmFixedRssiBound;
	UINT32 u4SCSTrafficThreshold;
	INT32 i4CckFalseCcaUpBound;
	INT32 i4CckFalseCcaLowBound;
} CMD_SMART_CARRIER_SENSE_CONFIG, *P_CMD_SMART_CARRIER_SENSE_CONFIG;

typedef struct _CMD_SET_SCS_PD_THR_RANGE {
	UINT8	u1BandIdx;
	UINT8 	u1Reserved[3];
	UINT16  u2CckPdThrMax;
	UINT16  u2OfdmPdThrMax;
	UINT16  u2CckPdThrMin;
	UINT16  u2OfdmPdThrMin;
} CMD_SET_SCS_PD_THR_RANGE, *P_CMD_SET_SCS_PD_THR_RANGE;

typedef struct _CMD_SMART_CARRIER_ENABLE {
	UINT8	BandIdx;
	UINT8	SCSEnable;
} CMD_SMART_CARRIER_ENABLE, *P_CMD_SMART_CARRIER_ENABLE;

typedef struct _CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW {
	CHAR	SCSMinRssi[DBDC_BAND_NUM];
	UINT32	OneSecTxByteCount[DBDC_BAND_NUM];
	UINT32	OneSecRxByteCount[DBDC_BAND_NUM];
} CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW, *P_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW;

typedef struct _CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2 {
	UINT8 u1BandIdx;
	UINT8 ActiveSTA;
	UINT16 eTput;
	BOOL fgRxOnly;
	BOOL PDreset;
	INT8 SCSMinRssi;
} CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2, *P_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2;

#endif
/*
IPI Level							Power(dBm)
    0							IPI<=-92
    1							-92<IPI<=-89
    2							-89<IPI<=-86
    :							:
    9							-60<IPI<=-55
    10							-55<IPI
*/

typedef struct {
	UINT32           IPI_Histogram[11];
} IPI_TABLE, *PIPI_TABLE; /* idel power indicator */

typedef struct _SMART_CARRIER_SENSE_CTRL_FW_T {
    UINT32 au4SCSTrafficThreshold[DBDC_BAND_NUM];
    INT32 ai4CckFalseCcaUpBound[DBDC_BAND_NUM];
    INT32 ai4CckFalseCcaLowBound[DBDC_BAND_NUM];
    INT32  ai4CckFalseCcaCount[DBDC_BAND_NUM];
    INT32  ai4OFDMFalseCcaCount[DBDC_BAND_NUM];
    UINT32 au4OneSecTxByteCount[DBDC_BAND_NUM];
    UINT32 au4OneSecRxByteCount[DBDC_BAND_NUM];
    UINT32 au4RtsCount[DBDC_BAND_NUM];
    UINT32 au4RtsRtyCount[DBDC_BAND_NUM];
    UINT16 au2OfdmFalseCcaUpBound[DBDC_BAND_NUM];
    UINT16 au2OfdmFalseCcaLowBound[DBDC_BAND_NUM];
    UINT16 au2CckFixedRssiBound[DBDC_BAND_NUM];
    UINT16 au2OfdmFixedRssiBound[DBDC_BAND_NUM];
    UINT16 au2OfdmPdBlkTh[DBDC_BAND_NUM];
    INT8   ai1SCSMinRssi[DBDC_BAND_NUM];

    UINT8 au1SCSStatus[DBDC_BAND_NUM];
    UINT8 au1CckPdBlkTh[DBDC_BAND_NUM];
    UINT8 au1SCSThTolerance[DBDC_BAND_NUM];
    UINT8 au1SCSMinRssiTolerance[DBDC_BAND_NUM];
    UINT8 au1SCSEnable[DBDC_BAND_NUM];
    BOOL afgOfdmPdSupport[DBDC_BAND_NUM];
} SMART_CARRIER_SENSE_CTRL_FW_T, *PSMART_CARRIER_SENSE_CTRL_FW_T;

typedef struct _SCS_SHOW_INFO_T {
	UINT32 u4EventId;
	SMART_CARRIER_SENSE_CTRL_FW_T    rScsInfo;
} SCS_SHOW_INFO_T, *PSCS_SHOW_INFO_T;

typedef struct _SMART_CARRIER_SENSE_CTRL {
	UINT8	SCSGeneration;/* 2:MT7615 3:MT7622*/
	BOOL	SCSEnable[DBDC_BAND_NUM];	/* 0:Disable, 1:Enable */
	/* SCS_BSS_TABLE	SCSBssTab2G; */		/* store AP information for SCS */
	/* SCS_BSS_TABLE	SCSBssTab5G; */		/* store AP information for SCS */
	UINT8	SCSStatus[DBDC_BAND_NUM];/* 0: Normal, 1:Low_gain */
	CHAR	SCSMinRssi[DBDC_BAND_NUM];
	UINT32	SCSTrafficThreshold[DBDC_BAND_NUM]; /* Traffic Threshold */
	/* UINT8                    NoiseEnv[DBDC_BAND_NUM]; */
	/* IPI_TABLE               Ipi2G; */
	/* IPI_TABLE               Ipi5G; */
	UINT32	OneSecTxByteCount[DBDC_BAND_NUM];
	UINT32	OneSecRxByteCount[DBDC_BAND_NUM];
	INT32	CckPdBlkTh[DBDC_BAND_NUM];
	INT32	OfdmPdBlkTh[DBDC_BAND_NUM];
	INT32	SCSMinRssiTolerance[DBDC_BAND_NUM];
	INT32	SCSThTolerance[DBDC_BAND_NUM];
	UCHAR	OfdmPdSupport[DBDC_BAND_NUM];
	BOOL	ForceScsOff[DBDC_BAND_NUM];
	UINT32	RtsCount[DBDC_BAND_NUM];
	UINT32	RtsRtyCount[DBDC_BAND_NUM];
	/* MT7622  support Band0*/
	UINT32	RTS_MPDU_DROP_CNT;
	UINT32	Retry_MPDU_DROP_CNT;
	UINT32	LTO_MPDU_DROP_CNT;
	/*------------------------*/
	UINT32	CckFalseCcaCount[DBDC_BAND_NUM];
	UINT32	OfdmFalseCcaCount[DBDC_BAND_NUM];
	UINT16	CckFalseCcaUpBond[DBDC_BAND_NUM];
	UINT16	CckFalseCcaLowBond[DBDC_BAND_NUM];
	UINT16	OfdmFalseCcaUpBond[DBDC_BAND_NUM];
	UINT16	OfdmFalseCcaLowBond[DBDC_BAND_NUM];
	INT32	CckFixedRssiBond[DBDC_BAND_NUM];
	INT32	OfdmFixedRssiBond[DBDC_BAND_NUM];
	/*SCSGen4_for_MT7663*/
	UINT32 PHY_MIN_PRI_PWR_OFFSET; /* for Band0 */
	UINT32 PHY_RXTD_CCKPD_7_OFFSET;
	UINT32 PHY_RXTD_CCKPD_8_OFFSET;
	/*SCSGen4_for_MT7663*/
	BOOL	PDreset[DBDC_BAND_NUM];
	/*SCSGen6_for_MT7915 MT7986 MT7916 MT7981*/
	UINT16	LastETput[DBDC_BAND_NUM];
	UINT16	ActiveSTAIdx[DBDC_BAND_NUM];
} SMART_CARRIER_SENSE_CTRL, *PSMART_CARRIER_SENSE_CTRL;

#ifdef SCS_FW_OFFLOAD
typedef struct _SMART_CARRIER_SENSE_CTRL_GEN2_T {
    UINT_8    u1SCSEnable;
    INT_8     i1SCSMinRssi;
    UINT_32   u4OneSecTxByteCount;
    UINT_32   u4OneSecRxByteCount;
    UINT_16   u2CckPdBlkTh;
    UINT_16   u2OfdmPdBlkTh;
    UINT_16   u2SCSMinRssiTolerance;
    UINT_16   u2CckPdThrMax;
    UINT_16   u2OfdmPdThrMax;
    UINT_16   u2CckPdThrMin;
    UINT_16   u2OfdmPdThrMin;

    UINT_16   u2IniAvgTput[SCS_STA_NUM];
    UINT_16   u2LastTputDiff[SCS_STA_NUM];
    UINT_16   u2LastAvgTput[SCS_STA_NUM];
    UINT_16   u2LastMaxTput[SCS_STA_NUM];
    UINT_16   u2LastMinTput[SCS_STA_NUM];
    UINT_16   u2LastTputIdx[SCS_STA_NUM];
    BOOL      fgLastTputDone[SCS_STA_NUM];
    UINT_16   u2CurAvgTput[SCS_STA_NUM];
    UINT_16   u2CurTputIdx[SCS_STA_NUM];
    UINT_8    u1TputPeriodScaleBit[SCS_STA_NUM];

    UINT_8    u1LastActiveSTA;
    UINT_8    u1ContinuousActiveSTAZeroCnt;
    UINT_8    u1ChannelBusyTh;
    BOOL      fgChBusy;
    UINT_8    u1MyTxRxTh;
    BOOL      fgPDreset;

    UINT_32   u4ChannelBusyTime;
    UINT_32   u4MyTxAirtime;
    UINT_32   u4MyRxAirtime;
    UINT_32   u4OBSSAirtime;
} SMART_CARRIER_SENSE_CTRL_GEN2_T, *P_SMART_CARRIER_SENSE_CTRL_GEN2_T;

typedef struct _SCS_GLO_CHECK {
	UINT_32 u4Addr;
	BOOLEAN fgError;
} SCS_GLO_CHECK, *P_SCS_GLO_CHECK;

typedef struct _DRV_SCS_GLO {
	SCS_GLO_CHECK rscsband[2];
} DRV_SCS_GLO, *P_DRV_SCS_GLO;

typedef struct _SCS_GLO_INFO {
	UINT_32 u4Addr;
	UINT_32 u4Size;
} SCS_GLO_INFO, *P_SCS_GLO_INFO;

typedef struct _EVENT_SCS_GLO {
	SCS_GLO_INFO rscsband[2];
} EVENT_SCS_GLO, *P_EVENT_SCS_GLO;

typedef struct _EVENT_GET_SCS_GLO_ADDR {
	UINT_32 u4EventId;
	UINT_32 u4Index;
	EVENT_SCS_GLO rGloInfo;
} EVENT_GET_SCS_GLO_ADDR, *P_EVENT_GET_SCS_GLO_ADDR;
#endif /*SCS_FW_OFFLOAD*/

VOID SmartCarrierSense_Gen2(RTMP_ADAPTER *pAd);
VOID SmartCarrierSense_Gen3(RTMP_ADAPTER *pAd);
VOID SmartCarrierSense_Gen4(RTMP_ADAPTER *pAd);
VOID SetSCS(RTMP_ADAPTER *pAd, UCHAR BandIdx, UINT32 value);
VOID SCS_init(RTMP_ADAPTER *pAd);
VOID SCS_Disable(RTMP_ADAPTER *pAd);

#ifdef SCS_FW_OFFLOAD
VOID SmartCarrierSense_Gen5(RTMP_ADAPTER *pAd);
VOID SmartCarrierSense_Gen6(RTMP_ADAPTER *pAd);
int SCS_Set_FW_Offload(RTMP_ADAPTER *pAd, CMD_SMART_CARRIER_ENABLE Param);
INT ShowScsGloAddr(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef WIFI_UNIFIED_COMMAND
VOID UniEventSCSGetGloAddrHandler(struct cmd_msg *msg, char *rsp_payload);
#endif /* WIFI_UNIFIED_COMMAND */
#endif

#endif /* __SCS_H__ */

