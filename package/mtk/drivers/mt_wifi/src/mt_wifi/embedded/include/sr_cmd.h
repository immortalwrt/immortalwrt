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
/* SR SRG BITMAP */
typedef struct _SR_MESH_SRG_BITMAP_T {
    UINT_32 u4Color_31_0;
    UINT_32 u4Color_63_32;
    UINT_32 u4pBssid_31_0;
    UINT_32 u4pBssid_63_32;
} SR_MESH_SRG_BITMAP_T, *P_SR_MESH_SRG_BITMAP_T;
/* END SR SRG BITMAP */

struct GNU_PACKED sr_mesh_topology_params {
	UINT_8 map_dev_count;
	UINT_8 map_dev_sr_support_mode;
	UINT_8 self_role;
	UINT_8 map_remote_al_mac[MAC_ADDR_LEN];
	UINT_8 map_remote_fh_bssid[MAC_ADDR_LEN];
	UINT_8 map_remote_bh_mac[MAC_ADDR_LEN];
	UCHAR  ssid_len;
	UCHAR  ssid[MAX_LEN_OF_SSID + 1];
};

struct GNU_PACKED sr_mesh_topology_update_params {
	struct sr_mesh_topology_params topo_params;
	BOOLEAN skip_scan;
	BOOLEAN scan_fail;
	BOOLEAN scan_start;
	UINT_8  scan_count;
	INT_8   scan_rssi;
	struct wifi_dev *wdev;
};


/*******************************************************************************
 *    GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *    FUNCTION PROTOTYPES
 ******************************************************************************/
NDIS_STATUS SrRstNav(IN RTMP_ADAPTER * pAd, IN RX_BLK * pRxBlk);
NDIS_STATUS SrDisSrBfrConnected(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev, IN BOOLEAN fgSrEnable);
NDIS_STATUS SrProfileSREnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer);
NDIS_STATUS SrProfileSRMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer);
NDIS_STATUS SrProfileSRSDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer);
NDIS_STATUS SrProfileSRDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer);
NDIS_STATUS SrProfileSRDropMinMCS(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *buffer);
NDIS_STATUS SrProfileSRDPDThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *buffer);
NDIS_STATUS SrMbssInit(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev);
NDIS_STATUS SrMeshGetSrgBitmap(IN PRTMP_ADAPTER pAd, IN UINT8 band_idx, IN PUINT_8 pMeshSrgBitmap);
NDIS_STATUS SrMeshGetSrMode(IN PRTMP_ADAPTER pAd, IN UINT8 band_idx, IN PUINT_8 pu1SrMode);
NDIS_STATUS SrMeshSrgBitMapControl(IN PRTMP_ADAPTER pAd, IN BOOLEAN fgSet, IN PUINT_8 pMeshSrgBitmap);
VOID SrMeshSelfSrgBMChangeEvent(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, IN BOOLEAN fgPBssidUpd);
NDIS_STATUS SrMeshTopologyUpdate(IN PRTMP_ADAPTER pAd, IN PUINT_8 pTopologyUpdate, IN UINT8 Band_Idx);
VOID SrMeshTopologyUpdatePerBand(struct _RTMP_ADAPTER *pAd, UINT_8 u1DbdcIdx);
VOID SrMeshTopologyUpdatePeriodic(struct _RTMP_ADAPTER *pAd);
VOID SrMeshTopologyUpdateBcnRssi(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, MLME_QUEUE_ELEM *Elem, BCN_IE_LIST *ie_list);
NDIS_STATUS SrFindBHWcid(PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, PUINT_8 pBHMac);
NDIS_STATUS SrSetUplinkTrafficStatus(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 UlStatus);
NDIS_STATUS SrSetMapBalance(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 Value);
NDIS_STATUS SrCmdSetMeshUlMode(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 Value);
NDIS_STATUS SrSetSrgBitmapRefresh(IN PRTMP_ADAPTER pAd, IN UINT_8 u1DbdcIdx);
NDIS_STATUS SrMeshStaModeRptLockConfig(IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev, IN UINT8 BandIdx, IN UINT8 u1RptLock);
NDIS_STATUS SrSetMeshRemoteStaModeRpt(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 RemoteAPStaAllHe);
VOID SrMeshSrUpdateSTAMode(IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev_main, BOOL Assoc, UINT8 CurrStaIsHe);

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
/** SR_CMD_SET_SR_FNQ_CTRL_ALL_CTRL **/
NDIS_STATUS SetSrFNQCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_FRM_FILT_ALL_CTRL **/
NDIS_STATUS SetSrFrmFiltAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_INTERPS_CTRL_ALL_CTRL **/
NDIS_STATUS SetSrInterPsCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_ENABLE **/
NDIS_STATUS SetSrCfgSrEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_SD_ENABLE **/
NDIS_STATUS SetSrCfgSrSdEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_BF **/
NDIS_STATUS SetSrCfgSrBf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_ATF **/
NDIS_STATUS SetSrCfgSrAtf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_MODE **/
NDIS_STATUS SetSrCfgSrMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_DISRT_ENABLE **/
NDIS_STATUS SetSrCfgDISRTEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_DISRT_MIN_RSSI **/
NDIS_STATUS SetSrCfgDISRTMinRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
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
/** SR_CMD_SET_SR_CFG_FNQ_ENABLE **/
NDIS_STATUS SetSrCfgFnqEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_DPD_ENABLE **/
NDIS_STATUS SetSrCfgDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_TX_ENABLE **/
NDIS_STATUS SetSrCfgSrTxEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_SD_OM_ENABLE **/
NDIS_STATUS SetSrCfgObssMonitorEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_TX_ALIGN_ENABLE **/
NDIS_STATUS SetSrCfgSrTxAlignEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_TX_ALIGN_RSSI_THR **/
NDIS_STATUS SetSrCfgSrTxAlignRssiThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_DABS_MODE **/
NDIS_STATUS SetSrCfgDabsMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_SR_DROP_MIN_MCS **/
NDIS_STATUS SetSrCfgDropMinMCS(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_SR_CFG_SR_DPD_THRESHOLD **/
NDIS_STATUS SetSrCfgSrDPDThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_SR_CFG_SR_ENABLE **/
NDIS_STATUS SetSrMeshSDFlag(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_SR_SRG_BITMAP **/
NDIS_STATUS SetSrSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_SRG_BITMAP_REFRESH **/
NDIS_STATUS SetSrSrgBitmapRefresh(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_MESH_SRG_BITMAP **/
INT SetSrMeshSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_SIGA_FLAG_CTRL **/
INT SetSrSiga(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_SIGA_AUTO_FLAG_CTRL **/
INT SetSrSigaAuto(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_MESH_FH_RSSI_TH **/
INT SetSrMeshRemoteFhRssiTh(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** Driver Internal **/
INT SetSrSelfSrgInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** Driver Internal **/
INT SetSrMeshTopoLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_REMOTE_FH_RSSI **/
INT SetSrMeshRemoteFhRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_DOWNLINK_STA_THRESHOLD **/
INT SetSrMeshStaThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_BH_MESH_SR_BITMAP **/
INT SetSrMeshBHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_FH_MESH_SR_BITMAP **/
INT SetSrMeshFHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_BH_DOWNLINK_MESH_SR_THRESHOLD **/
INT SetSrMeshBHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_FH_DOWNLINK_MESH_SR_THRESHOLD **/
INT SetSrMeshFHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_FORHIB_MESH_SR **/
INT SetSrMeshBHDownLinkForbidSR(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_FORHIB_MESH_SR_RESET **/
INT SetSrMeshResetBhDLForbidSR(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_REMOTE_BH_INFO **/
INT SetSrMeshRemoteBhInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_MAP_TOPO **/
INT SetSrMeshTopo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_MAP_BALANCE **/
INT SetSrMeshMapBalance(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_MAP_TRAFFIC_STATUS **/
INT SetSrMeshUplinkEvent(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
NDIS_STATUS SetSrSiga(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_MESH_SR_SD_CTRL **/
NDIS_STATUS SetMeshSRsd(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_SET_MESH_UL_MODE **/
INT SetSrMeshUlMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
NDIS_STATUS SrMeshApcliDetect(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
NDIS_STATUS SetMeshMac(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
NDIS_STATUS SRMeshLinkSTAThreshold(IN PRTMP_ADAPTER pAd, IN UINT_8 u1WdevIdx, INT_8 i1Rssi);
NDIS_STATUS SrMultiAPBhMeshSrgBitmap(IN PRTMP_ADAPTER pAd, IN BOOLEAN fgSet, IN PUINT_8 prSrBhMeshSrgBitmap);
NDIS_STATUS SrMultiAPFhMeshSrgBitmap(IN PRTMP_ADAPTER pAd, IN BOOLEAN fgSet, IN PUINT_8 prSrFhMeshSrgBitmap);
NDIS_STATUS SrBHDownMeshSRThreshold(IN PRTMP_ADAPTER pAd, IN INT_8 i1Rssi);
NDIS_STATUS SrFHDownMeshSRThreshold(IN PRTMP_ADAPTER pAd, IN INT_8 i1Rssi);
NDIS_STATUS SrMeshForbidSrBssid(IN PRTMP_ADAPTER pAd, IN UINT_8 u1WdevIdx);

/** Driver Internal **/
INT SetSrMeshStaModeRptLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_SET_SR_CFG_MESH_SR_REMOTE_STA_MODE **/
INT SetSrMeshRemoteStaHe(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);


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
/** SR_CMD_GET_SR_FNQ_CTRL_ALL_INFO **/
NDIS_STATUS ShowSrFNQCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_FRM_FILT_ALL_INFO **/
NDIS_STATUS ShowSrFrmFiltAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_INTERPS_CTRL_ALL_INFO **/
NDIS_STATUS ShowSrInterPsCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_INTERPS_DBG_ALL_INFO **/
NDIS_STATUS ShowSrInterPsDbgAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_ENABLE **/
NDIS_STATUS ShowSrCfgSrEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_SD_ENABLE **/
NDIS_STATUS ShowSrCfgSrSdEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_BF **/
NDIS_STATUS ShowSrCfgSrBf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_ATF **/
NDIS_STATUS ShowSrCfgSrAtf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_MODE **/
NDIS_STATUS ShowSrCfgSrMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_DISRT_ENABLE **/
NDIS_STATUS ShowSrCfgDISRTEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_DISRT_MIN_RSSI **/
NDIS_STATUS ShowSrCfgDISRTMinRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
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
/** SR_CMD_GET_SR_CFG_FNQ_ENABLE **/
NDIS_STATUS ShowSrCfgFnqEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_DPD_ENABLE **/
NDIS_STATUS ShowSrCfgDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_TX_ENABLE **/
NDIS_STATUS ShowSrCfgSrTxEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_SD_OM_ENABLE **/
NDIS_STATUS ShowSrCfgObssMonitorEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_TX_ALIGN_ENABLE **/
NDIS_STATUS ShowSrCfgSrTxAlignEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_TX_ALIGN_RSSI_THR **/
NDIS_STATUS ShowSrCfgSrTxAlignRssiThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_DABS_MODE **/
NDIS_STATUS ShowSrCfgDabsMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_SR_DROP_MIN_MCS **/
NDIS_STATUS ShowSrCfgDropMinMCS(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_GET_SR_CFG_SR_DPD_THRESHOLD **/
NDIS_STATUS ShowSrCfgSrDPDThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** SR_CMD_GET_SR_CNT_ALL **/
NDIS_STATUS ShowSrCnt(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_SD_ALL **/
NDIS_STATUS ShowSrSd(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_SRG_BITMAP **/
NDIS_STATUS ShowSrSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_MESH_SRG_BITMAP **/
INT ShowSrMeshSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_SIGA_FLAG_INFO **/
INT ShowSrSiga(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_SIGA_AUTO_FLAG_INFO **/
INT ShowSrSigaAuto(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** Driver Internal **/
INT ShowSrSelfSrgInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** Driver Internal **/
INT ShowSrMeshTopoLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_REMOTE_FH_RSSI **/
INT ShowSrMeshRemoteFhRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_MESH_FH_RSSI_TH **/
INT ShowSrMeshFhRssiTh(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_REMOTE_BH_INFO **/
INT ShowSrMeshRemoteBhInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_DOWNLINK_STA_THRESHOLD **/
INT ShowSrMeshstatTh(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_BH_MESH_SR_BITMAP **/
INT ShowSrMeshBHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_FH_MESH_SR_BITMAP **/
INT ShowSrMeshFHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_BH_DOWNLINK_MESH_SR_THRESHOLD **/
INT ShowSrMeshBHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_FH_DOWNLINK_MESH_SR_THRESHOLD **/
INT ShowSrMeshFHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_FORHIB_MESH_SR **/
INT ShowSrMeshBHDownLinkForbidSR(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_MAP_TOPO **/
INT ShowSrMeshTopo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** Driver Internal **/
INT ShowSrMeshTopoUpdateParams(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_MAP_TRAFFIC_STATUS **/
INT ShowSrMeshUplinkEvent(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_MESH_PHASE **/
INT ShowSrMeshPhase(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** Driver Internal **/
INT ShowSrMeshUlMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
/** Driver Internal **/
INT ShowSrMeshStaModeRptLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);
/** SR_CMD_GET_SR_CFG_MESH_SR_REMOTE_STA_MODE **/
INT ShowSrMeshRemoteStaHe(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg);


/* Event Handler andes_mt.c */
VOID EventSrHandler(PRTMP_ADAPTER pAd, UINT8 *Data, UINT_32 Length);

#endif				/* CFG_SUPPORT_FALCON_SR */
#endif				/* __CMM_SR_CMD_H__ */
