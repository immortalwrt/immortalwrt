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
	phystate.h
*/

#ifndef __PHYSTATE_H__
#define __PHYSTATE_H__

/*******************************************************************************
 *	INCLUDED FILES
 ******************************************************************************/


/*******************************************************************************
 *	DEFINITIONS
 ******************************************************************************/


/*******************************************************************************
 *	MACRO
 ******************************************************************************/


/*******************************************************************************
 *	TYPES
 ******************************************************************************/
/* Phy State Type */
typedef enum _PHY_STATE_TYPE {
	PHY_STATE_TX_PHYRATE = 0,
	PHY_STATE_RX_PHYRATE = 1,
	PHY_STATE_CMD_NUM
} PHY_STATE_TYPE, *P_PHY_STATE_TYPE;

/* Show Phy State info FW Command ENUM */
typedef enum _PHY_STATE_INFO_CMD {
	CMD_PHY_STATE_TX_PHYRATE = 0,
	CMD_PHY_STATE_RX_PHYRATE = 1,
	CMD_PHY_STATE_RSSI = 2,
	CMD_PHY_STATE_CONTENTION_RX_PHYRATE = 3,
	CMD_PHY_STATE_OFDMLQ_CNINFO = 4,
	CMD_PHY_STATE_TX_TD_CCK = 5,
	CMD_PHY_STATE_NUM
} PHY_STATE_INFO_CMD, *P_PHY_STATE_INFO_CMD;

/* Show Phy State info event from fw ENUM */
typedef enum _PHY_STATE_INFO_EVENT_T {
	EVENT_PHY_STATE_TX_PHYRATE = 0,
	EVENT_PHY_STATE_RX_PHYRATE = 1,
	EVENT_PHY_STATE_RSSI = 2,
	EVENT_PHY_STATE_CONTENTION_RX_PHYRATE = 3,
	EVENT_PHY_STATE_OFDMLQ_CNINFO = 4,
	EVENT_PHY_STATE_NUM
} PHY_STATE_INFO_EVENT_T, *P_PHY_STATE_INFO_EVENT_T;

/* Show Phy Stat Info CMD Format */
typedef struct _CMD_PHY_STATE_SHOW_INFO_T {
	UINT8	 ucPhyStateInfoCatg;
	UINT8	 ucBandIdx;
	UINT16	 u2Wcid;
} CMD_PHY_STATE_SHOW_INFO_T, *P_CMD_PHY_STATE_SHOW_INFO_T;

typedef struct _CMD_PHY_STATE_SHOW_RSSI_T {
	UINT8 u1PhyStateInfoCatg;
	UINT8 u1Reserved;
	UINT16 u2WlanIdx;
} CMD_PHY_STATE_SHOW_RSSI_T, *P_CMD_PHY_STATE_SHOW_RSSI_T;

typedef struct _CMD_PHY_STATE_SHOW_OFDMLQ_CN_T {
    UINT8 u1PhyStateInfoCatg;
    UINT8 u1BandIdx;
    UINT16 u2Reserved;
} CMD_PHY_STATE_SHOW_OFDMLQ_CN_T, *P_CMD_PHY_STATE_SHOW_OFDMLQ_CN_T;

#ifdef SPECIAL_11B_OBW_FEATURE
typedef struct _CMD_PHY_SET_TXTD_CCK_T {
    UINT8 u1PhyStateInfoCatg;
    UINT8 u1Enable;
    UINT16 u2Reserved;
} CMD_PHY_SET_TXTD_CCK_T, *P_CMD_PHY_SET_TXTD_CCK_T;
#endif /* SPECIAL_11B_OBW_FEATURE */

typedef struct _EXT_EVENT_PHY_STATE_TX_RATE {
	UINT8 u1PhyStateCate;
	UINT8 u1TxRate;
	UINT8 u1TxMode;
	UINT8 u1TxNsts;
} EXT_EVENT_PHY_STATE_TX_RATE, *P_EXT_EVENT_PHY_STATE_TX_RATE;

typedef struct _EXT_EVENT_PHY_STATE_RX_RATE {
	UINT8 u1PhyStateCate;
	UINT8 u1RxRate;
	UINT8 u1RxMode;
	UINT8 u1RxNsts;
	UINT8 u1Gi;
	UINT8 u1Coding;
	UINT8 u1Stbc;
	UINT8 u1BW;
} EXT_EVENT_PHY_STATE_RX_RATE, *P_EXT_EVENT_PHY_STATE_RX_RATE;

typedef struct _EXT_EVENT_PHY_STATE_OFDMLQ_CN {
    UINT8  u1PhyStateCate;
    UINT16 u2OfdmLqCn;
    UINT8  u1Reserved;
} EXT_EVENT_PHY_STATE_OFDMLQ_CN, *P_EXT_EVENT_PHY_STATE_OFDMLQ_CN;

typedef struct _RSSI_REPORT {
	CHAR rssi[4];
} RSSI_REPORT, *P_RSSI_REPORT;

typedef struct _RSSI_PAIR {
	UINT16 u2WlanIdx;
	CHAR rssi[4];
} RSSI_PAIR, *P_RSSI_PAIR;

typedef struct _EXT_EVENT_PHY_STATE_RSSI {
	UINT8 u1PhyStateCate;
	UINT8 u1Reserved;
	UINT16 u2WlanIdx;
	UINT8 u1Rcpi[4];
} EXT_EVENT_PHY_STATE_RSSI, *P_EXT_EVENT_PHY_STATE_RSSI;

/*******************************************************************************
 *	GLOBAL VARIABLES
 ******************************************************************************/


/*******************************************************************************
 *	FUNCTION PROTOTYPES
 ******************************************************************************/
INT PhyStatGetRssi(RTMP_ADAPTER *pAd, UINT8 band_idx, CHAR *rssi, UINT8 *len);
INT PhyStatGetCnInfo(RTMP_ADAPTER *pAd, UINT8 ucband_idx, UINT16 *pCnInfo);
INT32 MtCmdPhyShowInfo(RTMP_ADAPTER *pAd, UCHAR ucPhyStateInfoCatg, UINT8 ucBandIdx);
INT32 MtCmdPhyGetRxRate(RTMP_ADAPTER *pAd, UCHAR ucPhyStateInfoCatg, UINT8 ucBandIdx, UINT16 u2Wcid, P_EXT_EVENT_PHY_STATE_RX_RATE prRxRateInfo);
INT32 MtCmdGetRssi(RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, RSSI_REPORT *rssi_rpt);
INT32 MtCmdMultiRssi(RTMP_ADAPTER *pAd, RSSI_PAIR *RssiPair, UINT32 Num);
INT32 MtCmdGetCnInfo(RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT16 *u2cninfo);
#ifdef SPECIAL_11B_OBW_FEATURE
INT32 MtCmdSetTxTdCck(RTMP_ADAPTER *pAd, UINT8 u1Enable);
#endif /* SPECIAL_11B_OBW_FEATURE */
VOID EventPhyStatHandler(PRTMP_ADAPTER pAd, UINT8 *Data, UINT_32 Length);
INT ShowTxPhyRate(RTMP_ADAPTER *pAd, RTMP_STRING *arg, BOOLEAN fgset);
INT ShowRxPhyRate(RTMP_ADAPTER *pAd, RTMP_STRING *arg, BOOLEAN fgset);
INT ShowLastRxPhyRate(RTMP_ADAPTER *pAd, UINT8 band_idx, UINT16 u2Wcid, UINT32 *rx_rate);
VOID EventPhyStatTxRate(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventPhyStatRxRate(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);

#endif /*__PHYSTATE_H__*/
