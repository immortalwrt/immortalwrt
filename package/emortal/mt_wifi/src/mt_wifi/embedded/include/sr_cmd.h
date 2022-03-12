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
	sr_cmd.h
*/

#ifndef __CMM_SR_CMD_H__
#define __CMM_SR_CMD_H__

#ifdef CFG_SUPPORT_FALCON_SR
/*******************************************************************************
 *    INCLUDED FILES
 ******************************************************************************/
#include "rt_config.h"
/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/

/*******************************************************************************
 *    MACRO
 ******************************************************************************/

/*******************************************************************************
 *    TYPES
 ******************************************************************************/

/*******************************************************************************
 *    GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *    FUNCTION PROTOTYPES
 ******************************************************************************/
NDIS_STATUS SrRstNav(IN RTMP_ADAPTER *pAd, IN RX_BLK *pRxBlk);
NDIS_STATUS SrDisSrBfrConnected(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev, IN BOOLEAN fgSrEnable);
NDIS_STATUS SrProfileSREnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer);
NDIS_STATUS SrProfileSRMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *buffer);
NDIS_STATUS SrProfileSRSDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *buffer);
NDIS_STATUS SrProfileSRDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *buffer);
NDIS_STATUS SrMbssInit(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev);
/* For RTMP_PRIVATE_SUPPORT_PROC ap_cfg.c */
/** SET **/
/** SR_CMD_SET_SR_CAP_SREN_CTRL **/
NDIS_STATUS SetSrCapSrEn(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CAP_ALL_CTRL **/
NDIS_STATUS SetSrCapAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_PARA_ALL_CTRL	**/
NDIS_STATUS SetSrParaAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL **/
NDIS_STATUS SetSrDropTa(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_GLO_VAR_STA_CTRL **/
NDIS_STATUS SetSrSta(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL **/
NDIS_STATUS SetSrStaInit(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_COND_ALL_CTRL **/
NDIS_STATUS SetSrCondAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL **/
NDIS_STATUS SetSrRcpiTblAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL **/
NDIS_STATUS SetSrRcpiTblOfstAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_Q_CTRL_ALL_CTRL **/
NDIS_STATUS SetSrQCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_IBPD_ALL_CTRL **/
NDIS_STATUS SetSrIBPDAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_NRT_ALL_CTRL **/
NDIS_STATUS SetSrNRTAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_NRT_RESET_CTRL **/
NDIS_STATUS SetSrNRTResetAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL **/
NDIS_STATUS SetSrNRTCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_ENABLE **/
NDIS_STATUS SetSrCfgSrEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_SR_CFG_SR_SD_ENABLE **/
NDIS_STATUS SetSrCfgSrSdEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_SR_CFG_SR_BF **/
NDIS_STATUS SetSrCfgSrBf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_ATF **/
NDIS_STATUS SetSrCfgSrAtf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_MODE **/
NDIS_STATUS SetSrCfgSrMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_SR_CFG_DISRT_ENABLE **/
NDIS_STATUS SetSrCfgDISRTEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_SR_CFG_DISRT_MIN_RSSI **/
NDIS_STATUS SetSrCfgDISRTMinRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_SR_CFG_TXC_QUEUE **/
NDIS_STATUS SetSrCfgTxcQueue(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_TXC_QID **/
NDIS_STATUS SetSrCfgTxcQid(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_TXC_PATH **/
NDIS_STATUS SetSrCfgTxcPath(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_AC_METHOD **/
NDIS_STATUS SetSrCfgAcMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_PERIOD_THR **/
NDIS_STATUS SetSrCfgSrPeriodThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_QUERY_TXD_METHOD **/
NDIS_STATUS SetSrCfgQueryTxDMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_SD_CG_RATIO **/
NDIS_STATUS SetSrCfgSrSdCgRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_SD_OBSS_RATIO **/
NDIS_STATUS SetSrCfgSrSdObssRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_PROFILE **/
NDIS_STATUS SetSrCfgProfile(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_DPD_ENABLE **/
NDIS_STATUS SetSrCfgDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_SR_SRG_BITMAP **/
NDIS_STATUS SetSrSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_SRG_BITMAP_REFRESH **/
NDIS_STATUS SetSrSrgBitmapRefresh(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);

/** GET **/
/** SR_CMD_GET_SR_CAP_ALL_INFO **/
NDIS_STATUS ShowSrCap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_PARA_ALL_INFO **/
NDIS_STATUS ShowSrPara(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_IND_ALL_INFO **/
NDIS_STATUS ShowSrInd(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO **/
NDIS_STATUS ShowSrInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_COND_ALL_INFO **/
NDIS_STATUS ShowSrCond(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_RCPI_TBL_ALL_INFO **/
NDIS_STATUS ShowSrRcpiTbl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_RCPI_TBL_OFST_ALL_INFO **/
NDIS_STATUS ShowSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_Q_CTRL_ALL_INFO **/
NDIS_STATUS ShowSrQCtrl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_IBPD_ALL_INFO **/
NDIS_STATUS ShowSrIBPD(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_NRT_ALL_INFO **/
NDIS_STATUS ShowSrNRT(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_NRT_CTRL_ALL_INFO **/
NDIS_STATUS ShowSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_ENABLE **/
NDIS_STATUS ShowSrCfgSrEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_GET_SR_CFG_SR_SD_ENABLE **/
NDIS_STATUS ShowSrCfgSrSdEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_GET_SR_CFG_SR_BF **/
NDIS_STATUS ShowSrCfgSrBf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_ATF **/
NDIS_STATUS ShowSrCfgSrAtf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_MODE **/
NDIS_STATUS ShowSrCfgSrMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_GET_SR_CFG_DISRT_ENABLE **/
NDIS_STATUS ShowSrCfgDISRTEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_GET_SR_CFG_DISRT_MIN_RSSI **/
NDIS_STATUS ShowSrCfgDISRTMinRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_GET_SR_CFG_TXC_QUEUE **/
NDIS_STATUS ShowSrCfgTxcQueue(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_TXC_QID **/
NDIS_STATUS ShowSrCfgTxcQid(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_TXC_PATH **/
NDIS_STATUS ShowSrCfgTxcPath(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_AC_METHOD **/
NDIS_STATUS ShowSrCfgAcMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_PERIOD_THR **/
NDIS_STATUS ShowSrCfgSrPeriodThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_QUERY_TXD_METHOD **/
NDIS_STATUS ShowSrCfgQueryTxDMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_SD_CG_RATIO **/
NDIS_STATUS ShowSrCfgSrSdCgRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_SD_OBSS_RATIO **/
NDIS_STATUS ShowSrCfgSrSdObssRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_PROFILE **/
NDIS_STATUS ShowSrCfgProfile(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_DPD_ENABLE **/
NDIS_STATUS ShowSrCfgDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_GET_SR_CNT_ALL **/
NDIS_STATUS ShowSrCnt(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_SD_ALL **/
NDIS_STATUS ShowSrSd(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_SRG_BITMAP **/
NDIS_STATUS ShowSrSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);

/* Event Handler andes_mt.c */
VOID EventSrHandler(PRTMP_ADAPTER pAd, UINT8 *Data, UINT_32 Length);

#endif				/* CFG_SUPPORT_FALCON_SR */
#endif				/* __CMM_SR_CMD_H__ */
