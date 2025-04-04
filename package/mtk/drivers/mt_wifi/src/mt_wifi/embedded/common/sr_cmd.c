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
	sr_cmd.c
*/
#include "rt_config.h"
/*For HWITS00021718*/
#include "mac/mac_mt/fmac/mt_fmac.h"
#ifndef MT7915_MT7916_COEXIST_COMPATIBLE
#ifdef MT7986
#include "chip/mt7986_cr.h"
#endif
#ifdef MT7916
#include "chip/mt7916_cr.h"
#endif
#ifdef MT7981
#include "chip/mt7981_cr.h"
#endif
#endif
/*End - For HWITS00021718*/

#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
#ifdef CFG_SUPPORT_FALCON_SR
/*******************************************************************************
 *    MACRO
 ******************************************************************************/
#define IS_SR_V1(_pAd) (IS_MT7915(_pAd))
#define IS_SR_V2(_pAd) (IS_MT7986(_pAd) || IS_MT7981(_pAd)) /* Add || IS_MT7916(_pAd) */

/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/
#define SR_PARA_RCPI_SRC_SEL_ANT_0          0x0
#define SR_PARA_RCPI_SRC_SEL_ANT_1          0x1
#define SR_PARA_RCPI_SRC_SEL_ANT_2          0x2
#define SR_PARA_RCPI_SRC_SEL_ANT_3          0x3
/* Decimal dbm*/
#define SR_PARA_PD_THR_MAX                  -62
#define SR_PARA_PD_THR_MIN                  -110
#define SR_PARA_PD_THR_DEFAULT              SR_PARA_PD_THR_MAX
#define SR_PARA_PD_MIN_MAX                  0
#define SR_PARA_PD_MIN_MIN                  -127
#define SR_PARA_PD_MIN_DEFAULT              -82
#define SR_PARA_PERIOD_OFST_MAX             63
#define SR_PARA_PERIOD_OFST_MIN             0
#define SR_PARA_PERIOD_OFST_DEFAULT         SR_PARA_PERIOD_OFST_MIN
#define SR_PARA_OBSS_TXPWR_REF_MAX          30
#define SR_PARA_OBSS_TXPWR_REF_MIN          0
#define SR_PARA_OBSS_TXPWR_REF_DEFAULT      21

#define SR_COND_PERIOD_LIMIT_MAX            255
#define SR_COND_PERIOD_LIMIT_MIN            0
#define SR_RCPI_MAX                         0
#define SR_RCPI_MIN                         -110
#define SR_RCPI_DEFAULT                     SR_RCPI_MAX

/** ARG Number**/

/* SR_V1 & SR_V2 Conflict Part*/
#define SR_CMD_SET_SR_CAP_ALL_CTRL_ARG_NUM_SR_V1             12
#define SR_CMD_SET_SR_CAP_ALL_CTRL_ARG_NUM_SR_V2             20

#define SR_CMD_SET_SR_COND_ALL_CTRL_ARG_NUM_SR_V1            10
#define SR_CMD_SET_SR_COND_ALL_CTRL_ARG_NUM_SR_V2            11

#define SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL_ARG_NUM_SR_V1   7
#define SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL_ARG_NUM_SR_V2   8

#define SR_CMD_SET_SR_Q_CTRL_ALL_CTRL_ARG_NUM_SR_V1          4
#define SR_CMD_SET_SR_Q_CTRL_ALL_CTRL_ARG_NUM_SR_V2          6

#define SR_CMD_SET_SR_NRT_ALL_CTRL_ARG_NUM_SR_V1             2
#define SR_CMD_SET_SR_NRT_ALL_CTRL_ARG_NUM_SR_V2             4

#define SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL_ARG_NUM_SR_V1        6
#define SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL_ARG_NUM_SR_V2        7

/* SR_V2 New Feature */
#define SR_CMD_SET_SR_FNQ_CTRL_ALL_CTRL_ARG_NUM              6
#define SR_CMD_SET_SR_FRM_FILT_ALL_CTRL_ARG_NUM              1
#define SR_CMD_SET_SR_INTERPS_CTRL_ALL_CTRL_ARG_NUM          6

/* SR_V1 & SR_V2 Common Part */
#define SR_CMD_SET_DEFAULT_ARG_NUM                           1
#define SR_CMD_SET_SR_CAP_SREN_CTRL_ARG_NUM                  1
#define SR_CMD_SET_SR_PARA_ALL_CTRL_ARG_NUM                  9
#define SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL_ARG_NUM           2
#define SR_CMD_SET_SR_GLO_VAR_STA_CTRL_ARG_NUM               4
#define SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL_ARG_NUM          2
#define SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL_ARG_NUM              SR_RCPITBL_MCS_NUM
#define SR_CMD_SET_SR_IBPD_ALL_CTRL_ARG_NUM                  5
#define SR_CMD_SET_SR_NRT_RESET_CTRL_ARG_NUM                 1
#define SR_CMD_SET_SR_SRG_BITMAP_ARG_NUM                     4
#define SR_CMD_SET_SR_SELF_SRG_INFO_ARG_NUM                  5
#define SR_CMD_SET_SR_MESH_SRG_BITMAP_ARG_NUM                4
#define SR_CMD_SET_SR_REMOTE_BH_INFO_ARG_NUM                 2
#define SR_CMD_SET_SR_MAP_TOPOLOGY_ARG_NUM                   3
#define SR_CMD_SET_SR_MESH_FH_RSSI_TH_ARG_NUM                2
#define SR_CMD_SET_SR_DL_STA_MESH_THRESHOLD_TH_ARG_NUM	2
#define SR_CMD_SET_SR_MESH_FH_SRG_BITMAP_ARG_NUM	4

#define SR_CMD_GET_DEFAULT_ARG_NUM                           1
#define SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO_ARG_NUM    2

/* Global Variable */
#define SR_STA_NUM           32
#define SR_DROP_TA_NUM       16
#define SR_RCPITBL_MCS_NUM   12
#define SR_NRT_ROW_NUM       16
#define SR_BSSID_NUM         20
#define SR_SIGA_FLAG_RANGE   16
#define SR_PARA_PD_FH_RSSI_TH_DEFAULT		-127

#define SR_SCENE_DETECTION_TIMER_PERIOD_MS  500 /* ms */
#define SR_SCENE_DETECTION_OBSS_RSP_TIME_US 100 /* us */
#define SR_SCENE_DETECTION_OBSS_AIRTIME_THR 500 /* 50.0% */
#define SR_SCENE_DETECTION_CONGESTION_THR   800 /* 80.0% */
#define SR_SCENE_DETECTION_MAC_LENGTH       17
#define SR_BSSID_OMAC_OFFSET                12  /* Omac offset */

#define WH_SR_RCPITBL_MCS_NUM               12

#define SR_MESH_SCAN_COUNT_MAX               3

#define SR_MESH_PRIMARY_INTERFACE	    0x0
#define SR_MESH_AP_CLIENT		    0x1
#define SR_MESH_P2PGO			    0x2
#define SR_MESH_P2PGC			    0x3

BOOLEAN fgmeshdetect = 1;
BOOLEAN fghavebeensend = 1;
struct sr_mesh_topology_params g_rTopologyUpdate;


/*******************************************************************************
 *    PRIVATE TYPES
 ******************************************************************************/
/** FW & DRV sync with wh_sr.h **/
/*Follow HAL ENUM_WH_RESP_TXPWR_MODE_T */
typedef enum _ENUM_WH_SR_RESP_TXPWR_MODE {
    WH_ENUM_SR_RESP_TXPWR_NO_RESTRIC = 0,
    WH_ENUM_SR_RESP_TXPWR_RESTRIC,
    WH_ENUM_SR_RESP_TXPWR_MODE_NUM
} ENUM_WH_SR_RESP_TXPWR_MODE_T, *P_ENUM_WH_SR_RESP_TXPWR_MODE_T;

/*Follow HAL ENUM_SR_TXPWR_RESTRIC_MODE_T */
typedef enum _ENUM_WH_SR_TXPWR_RESTRIC_MODE {
    WH_ENUM_SR_TXPWR_RESTRIC_NO_RESTRIC = 0,
    WH_ENUM_SR_TXPWR_RESTRIC_FOLLOW_SPEC,
    WH_ENUM_SR_TXPWR_RESTRIC_FOLLOW_MTK,
    WH_ENUM_SR_TXPWR_RESTRIC_MODE_NUM
} ENUM_WH_SR_TXPWR_RESTRIC_MODE_T, *P_ENUM_WH_SR_TXPWR_RESTRIC_MODE_T;

/* SR Capability */

typedef struct _WH_SR_CAP_T_SR_V1 {
    /** RMAC */
    BOOLEAN fgSrEn;
    BOOLEAN fgSrgEn;
    BOOLEAN fgNonSrgEn;
    BOOLEAN fgSingleMdpuRtsctsEn;
    BOOLEAN fgHdrDurEn;
    BOOLEAN fgTxopDurEn;
    BOOLEAN fgNonSrgInterPpduPresv;
    BOOLEAN fgSrgInterPpduPresv;
    /** AGG */
    BOOLEAN fgSrRemTimeEn;
    BOOLEAN fgProtInSrWinDis;
    BOOLEAN fgTxCmdDlRateSelEn;
    /** MIB */
    BOOLEAN fgAmpduTxCntEn;
} WH_SR_CAP_T_SR_V1, *P_WH_SR_CAP_T_SR_V1;

typedef struct _WH_SR_CAP_T_SR_V2 {
    /* RMAC */
    /* DW_1 */
    BOOLEAN fgSrEn;
    BOOLEAN fgSrgEn;
    BOOLEAN fgNonSrgEn;
    BOOLEAN fgSingleMdpuRtsctsEn;
    /* DW_2 */
    BOOLEAN fgHdrDurEn;
    BOOLEAN fgTxopDurEn;
    BOOLEAN fgNonSrgInterPpduPresv;
    BOOLEAN fgSrgInterPpduPresv;
    /* DW_3 */
    BOOLEAN fgSMpduNoTrigEn;
    BOOLEAN fgSrgBssidOrder;
    BOOLEAN fgCtsAfterRts;
    BOOLEAN fgSrpOldRxvEn;
    /* DW_4 */
    BOOLEAN fgSrpNewRxvEn;
    BOOLEAN fgSrpDataOnlyEn;
    /* AGG  */
    BOOLEAN fgFixedRateSrREn;
    BOOLEAN fgWtblSrREn;
    /* DW_5 */
    BOOLEAN fgSrRemTimeEn;
    BOOLEAN fgProtInSrWinDis;
    BOOLEAN fgTxCmdDlRateSelEn;
    /* MIB  */
    BOOLEAN fgAmpduTxCntEn;
} WH_SR_CAP_T_SR_V2, *P_WH_SR_CAP_T_SR_V2;

/* SR Parameter */
typedef struct _WH_SR_PARA_T {
    /* RMAC */
    /* DW_1 */
    UINT_8 u1NonSrgPdThr;
    UINT_8 u1SrgPdThr;
    UINT_8 u1PeriodOfst;
    UINT_8 u1RcpiSourceSel;
    /* TMAC */
    /* DW_2 */
    UINT_16 u2ObssPdMin;
    UINT_16 u2ObssPdMinSrg;
    /* DW_3 */
    ENUM_WH_SR_RESP_TXPWR_MODE_T eRespTxPwrMode;
    /* DW_4 */
    ENUM_WH_SR_TXPWR_RESTRIC_MODE_T eTxPwrRestricMode;
    /* DW_5 */
    UINT_8 u1ObssTxPwrRef;
    UINT_8 RSV[3];
} WH_SR_PARA_T, *P_WH_SR_PARA_T;

/* SR Indicator */
typedef struct _WH_SR_IND_T {
    /* RMAC */
    /* DW_1 */
    UINT_8 u1NonSrgInterPpduRcpi;
    UINT_8 u1SrgInterPpduRcpi;
    UINT_16 u2NonSrgVldCnt;
    /* DW_2 */
    UINT_16 u2SrgVldCnt;
    UINT_16 u2IntraBssPpduCnt;
    /* DW_3 */
    UINT_16 u2InterBssPpduCnt;
    UINT_16 u2NonSrgPpduVldCnt;
    /* DW_4 */
    UINT_16 u2SrgPpduVldCnt;
    UINT_8 RSV[2];
    /* MIB */
    /* DW_5 */
    UINT_32 u4SrAmpduMpduCnt;
    /* DW_6 */
    UINT_32 u4SrAmpduMpduAckedCnt;
} WH_SR_IND_T, *P_WH_SR_IND_T;

/* SR Condition */
typedef struct _SR_COND_T_SR_V1 {
    BOOLEAN fgSrRcpiCckRateEn;
    BOOLEAN fgSrMacRcpiRateEn;
    BOOLEAN fgSrRxvRcpiRateEn;
    BOOLEAN fgSrRcpiHeRateEn;
    BOOLEAN fgSrRcpiVhtRateEn;
    BOOLEAN fgSrRcpiHtRateEn;
    BOOLEAN fgSrRcpiLgRateEn;
    BOOLEAN fgSrRxvEntry;
    BOOLEAN fgSrPeriodLimitEn;
    UINT_8  u1SrPeriodLimit;
    /** Reserve for 4-byte aligned*/
    UINT_8  u1Reserved[2];
} SR_COND_T_SR_V1, *P_SR_COND_T_SR_V1;

typedef struct _WH_SR_COND_T_SR_V2 {
    /* RMAC */
    /* DW_1 */
    BOOLEAN fgSrRcpiSel;
    BOOLEAN fgSrRcpiCckRateEn;
    BOOLEAN fgSrMacRcpiRateEn;
    BOOLEAN fgSrRxvRcpiRateEn;
    /* DW_2 */
    BOOLEAN fgSrRcpiHeRateEn;
    BOOLEAN fgSrRcpiVhtRateEn;
    BOOLEAN fgSrRcpiHtRateEn;
    BOOLEAN fgSrRcpiLgRateEn;
    /* DW_3*/
    BOOLEAN fgSrRxvEntry;
    BOOLEAN fgSrPeriodLimitEn;
    UINT_8  u1SrPeriodLimit;
    UINT_8  RSV[1];
} WH_SR_COND_T_SR_V2, *P_WH_SR_COND_T_SR_V2;

/* SR Queue Control */

typedef struct _WH_SR_QUEUE_CTRL_T_SR_V1 {
    /** RMAC */
    BOOLEAN fgSrRxRptEn;
    /** ARB */
    BOOLEAN fgSrCw;
    BOOLEAN fgSrSuspend;
    /** Reserve for 4-byte aligned*/
    UINT_8  u1Reserved;
    /** ARB */
    UINT_32 u4SrBackOffMask;
} WH_SR_QUEUE_CTRL_T_SR_V1, *P_WH_SR_QUEUE_CTRL_T_SR_V1;

typedef struct _WH_SR_QUEUE_CTRL_T_SR_V2 {
    /* RMAC */
    /* DW_1 */
    BOOLEAN fgSrRxRptEn;
    /* ARB */
    BOOLEAN fgSrCw;
    BOOLEAN fgSrSuspend;
    BOOLEAN fgSrDisSwAifsDis;
    /* DW_2 */
    UINT_32 u4SrBackOffMask;
    /* DW_3 */
    UINT_32 u4SrBackOffEnable;
} WH_SR_QUEUE_CTRL_T_SR_V2, *P_WH_SR_QUEUE_CTRL_T_SR_V2;

/* SR RCPI Table */
typedef struct _WH_SR_RCPITBL_T {
    /* RMAC */
    /* DW_1 - DW_3 */
    UINT_8  u1RcpiTblMcs[WH_SR_RCPITBL_MCS_NUM];
} WH_SR_RCPITBL_T, *P_WH_SR_RCPITBL_T;

/* SR RCPI Table Offset */
typedef struct _SR_RCPITBL_OFST_T_SR_V1 {
    UINT_16 u2RxBwRcpiOfst;
    UINT_16 u2StbcRcpiOfst;
    UINT_16 u2NumAntRcpiOfst;
    UINT_16 u2LdpcRcpiOfst;
    UINT_16 u2DcmRcpiOfst;
    UINT_16 u2MacRcpiOfst;
    UINT_16 u2SigRcpiOfst;
    /** Reserve for 4-byte aligned*/
    UINT_8  u1Reserved[2];
} SR_RCPITBL_OFST_T_SR_V1, *P_SR_RCPITBL_OFST_T_SR_V1;

typedef struct _WH_SR_RCPITBL_OFST_T_SR_V2 {
    /* RMAC */
    /* DW_1 */
    UINT_16 u2RxBwRcpiOfst;
    UINT_16 u2StbcRcpiOfst;
    /* DW_2 */
    UINT_16 u2NumAntRcpiOfst;
    UINT_16 u2LdpcRcpiOfst;
    /* DW_3 */
    UINT_16 u2DcmRcpiOfst;
    UINT_16 u2MacRcpiOfst;
    /* DW_4 */
    UINT_16 u2SigRcpiOfst;
    UINT_16 u2BfRcpiOfst;
} WH_SR_RCPITBL_OFST_T_SR_V2, *P_WH_SR_RCPITBL_OFST_T_SR_V2;

/* SR Inter Bss Ppdu Determination */
typedef struct _WH_SR_IBPD_T {
    /* RMAC */
    /* DW_1 */
    UINT_8 u1InterBssByHdrBssid;
    UINT_8 u1InterBssByMu;
    UINT_8 u1InterBssByPbssColor;
    UINT_8 u1InterBssByPaid;
    /* DW_2 */
    UINT_8 u1InterBssByBssColor;
    UINT_8 RSV[3];
} WH_SR_IBPD_T, *P_WH_SR_IBPD_T;

/* SR Neighbor Rssi Table */

/** SRNRT*/
typedef struct _SR_NRT_T_SR_V1 {
    /** RMAC */
    UINT_8 u1TableIdx;
    UINT_32 u4NRTValue;
    /** Reserve for 4-byte aligned*/
    UINT_8 u1Reserved[3];
} SR_NRT_T_SR_V1, *P_SR_NRT_T_SR_V1;

typedef struct _WH_SR_NRT_T_SR_V2 {
    /* RMAC */
    /* DW_1 */
    UINT_8 u1TableIdx;
    UINT_8 u1RaTaSel;
    BOOLEAN fgSwProtect;
    UINT_8 RSV[1];
    /* DW_2 */
    UINT_32 u4NRTValue;
} WH_SR_NRT_T_SR_V2, *P_WH_SR_NRT_T_SR_V2;

/* SR Neighbor Rssi Table Control */

/** SRNRTCtrl*/
typedef struct _SR_NRT_CTRL_T_SR_V1 {
    /** RMAC */
    BOOLEAN fgSrtEn;
    BOOLEAN fgSrtSrpEn;
    BOOLEAN fgSrtAddrOrderEn;
    /** Reserve for 4-byte aligned*/
    UINT_8 u1Reserved[1];
    UINT_16 u2SrtInRcpiTh;
    UINT_16 u2SrtOutRcpiTh;
    UINT_16 u2SrtUsedCntTh;
    /** Reserve for 4-byte aligned*/
    UINT_8 u1Reserved2[2];
} SR_NRT_CTRL_T_SR_V1, *P_SR_NRT_CTRL_T_SR_V1;

typedef struct _WH_SR_NRT_CTRL_T_SR_V2 {
    /* RMAC */
    /* DW_1 */
    BOOLEAN fgSrtEn;
    BOOLEAN fgSrtSrpEn;
    BOOLEAN fgSrtAddrOrderEn;
    BOOLEAN fgSrtByPassCtsAck;
    /* DW_2 */
    UINT_16 u2SrtInRcpiTh;
    UINT_16 u2SrtOutRcpiTh;
    /* DW_3 */
    UINT_16 u2SrtUsedCntTh;
    UINT_8 RSV[2];
} WH_SR_NRT_CTRL_T_SR_V2, *P_WH_SR_NRT_CTRL_T_SR_V2;

/* SR Freeze Normal Queue Control */
typedef struct _WH_SR_FNQ_CTRL_T {
    /* RMAC */
    /* DW_1 */
    UINT_16 u2SrpCondDis;
    UINT_8  u1PeriodOfst;
    BOOLEAN fgHdrDurEn;
    /* DW_2 */
    BOOLEAN fgTxopDurEn;
    BOOLEAN fgSrpCfendRst;
    BOOLEAN fgSrpNavToutRst;
    UINT_8  RSV[1];
} WH_SR_FNQ_CTRL_T, *P_WH_SR_FNQ_CTRL_T;

/* SR Inter Power Saving Control */
typedef struct _WH_SR_INTERPS_CTRL_T {
    /* RMAC */
    /* DW_1 */
    UINT_8  u1CondDis;
    UINT_8  u1DurAdj;
    UINT_8  u1DurLmt;
    UINT_8  u1EntryEn;
    /* DW_2 */
    BOOLEAN fgDurLmtEn;
    BOOLEAN fgInterpsEn;
    UINT_8  RSV[2];
} WH_SR_INTERPS_CTRL_T, *P_WH_SR_INTERPS_CTRL_T;

/* SR Inter Power Saving Debug */
typedef struct _WH_SR_INTERPS_DBG_T {
    /* RMAC */
    /* DW_1 */
    UINT_8  u1Entry0Cnt;
    UINT_8  u1Entry1Cnt;
    UINT_8  u1Entry2Cnt;
    UINT_8  u1EntryLat;
} WH_SR_INTERPS_DBG_T, *P_WH_SR_INTERPS_DBG_T;

static struct {
	RTMP_STRING *name;
	UINT8 u1srflag;
} *PENUM_WH_SR_SIGA_FLAG_T, ENUM_WH_SR_SIGA_FLAG_T[] = {
	{"PSR_DISALLOW", 0},
	{"SR_RESTRICTED", 13},
	{"SR_DELAYED", 14},
	{"PSR_AND_NON_SRG_OBSS_PD_PROHIBITED", 15},
	{NULL,}
};

/** End FW & DRV sync with wh_sr.h **/

/** FW & DRV sync with sr_cmm.h **/
typedef enum _ENUM_SR_RXRPT_SRC_T {
    ENUM_SR_RXRPT_SRC_RXRPT = 0,
    ENUM_SR_RXRPT_SRC_CMDRPT_TX,
    ENUM_SR_RXRPT_SRC_NUM
} ENUM_SR_RXRPT_SRC_T, *P_ENUM_SR_RXRPT_SRC_T;
/** END FW & DRV sync with sr_cmm.h **/

/** FW & DRV sync with sr_if.h **/
typedef enum _ENUM_SR_STA_MODE_T {
    ENUM_SR_STA_MODE_AUTO = 0x0,
    ENUM_SR_STA_MODE_FIXED,
    ENUM_SR_STA_MODE_NUM
} ENUM_SR_STA_MODE_T, *P_ENUM_SR_STA_MODE_T;

typedef struct _SR_STA_INFO_T {
    UINT_16 u2WlanId;
    UINT_8 u1Mode; /* ENUM_SR_STA_MODE_T */
    UINT_8 u1State; /* ENUM_SR_STA_STATE_T */
    UINT_8 u1SrRateOffset;
    UINT_8 u1SrRaTryCnt;
    UINT_8 u1SrRaRound;
    UINT_8 u1SrRaState;
    UINT_16 u2SrSucCnt;
    UINT_16 u2SrTtlTxCnt;
    UINT_32 u4Score;
    UINT_16 u2BadQuota;
    UINT_8 u1BadLevel;
    UINT_8 u1SrRate;
} SR_STA_INFO_T, *P_SR_STA_INFO_T;

typedef struct _SR_DROP_TA_INFO_T {
    UINT_32 u4Address2;
    SR_STA_INFO_T rSrStaInfo[SR_STA_NUM];
} SR_DROP_TA_INFO_T, *P_SR_DROP_TA_INFO_T;

/* SR CNT */
typedef enum _ENUM_SR_ENTRY_T {
    ENUM_SR_ENTRY_NEWRXV = 0,
    ENUM_SR_ENTRY_ORIRXVVHT,
    ENUM_SR_ENTRY_ORIRXVHE,
    ENUM_SR_ENTRY_NEWMAC,
    ENUM_SR_ENTRY_ORIMAC,
    ENUM_SR_ENTRY_NUM
} ENUM_SR_ENTRY_T, *P_ENUM_SR_ENTRY_T;

enum _ENUM_SR_NONSRG_PPDU_TYPE_T {
	ENUM_SR_NONSRG_OBSS_OR_NONHE = 0,
	ENUM_SR_NONSRG_MESH_HE_INTERNAL_NO_SR,
	ENUM_SR_NONSRG_MESH_HE,
	ENUM_SR_NONSRG_PPDU_TYPE_NUM
};

typedef struct _SR_CNT_ENTRY_T {
    UINT_16 u2EntryTtl;
    UINT_16 u2PeriodSuc;/* u2PeriodFail = u2EntryTtl  - u2PeriodSuc */
    UINT_16 u2GenTxcSuc;/* u2GenTxcFail = u2PeriodSuc - u2GenTxcSuc */
    UINT_16 u2SrTxSuc;  /* u2SrTxFail   = u2GenTxcSuc - u2SrTxSuc   */
} SR_CNT_ENTRY_T, *P_SR_CNT_ENTRY_T;

typedef struct _SR_CNT_T {
    UINT_16 u2EntryNoSrTtl[RAM_BAND_NUM];
    UINT_16 u2EntryFailTtl[RAM_BAND_NUM];
    SR_CNT_ENTRY_T rSrCntEntry[RAM_BAND_NUM][ENUM_SR_RXRPT_SRC_NUM][ENUM_SR_ENTRY_NUM][ENUM_SR_NONSRG_PPDU_TYPE_NUM];
} SR_CNT_T, *P_SR_CNT_T;
/* END SR CNT */

/* SR SD (Scene Detection) */
typedef struct _SR_SD_T {
    UINT_16 u2RxrptRxvCnt[RAM_BAND_NUM];
    UINT_16 u2RxrptMacCnt[RAM_BAND_NUM];
    UINT_32 u4DeltaTime[RAM_BAND_NUM];
    UINT_32 u4HWSrPeriodRatio[RAM_BAND_NUM];
    UINT_32 u4SWSrPeriodRatio[RAM_BAND_NUM];
    UINT_32 u4SrTxAirtime[RAM_BAND_NUM];
    UINT_32 u4OBSSAirtime[RAM_BAND_NUM];
    UINT_32 u4MyTxAirtime[RAM_BAND_NUM];
    UINT_32 u4MyRxAirtime[RAM_BAND_NUM];
    UINT_32 u4ChannelBusyTime[RAM_BAND_NUM];
    UINT_32 u4TtlAirTime[RAM_BAND_NUM];
    UINT_32 u4TtlAirTimeRatio[RAM_BAND_NUM];
    UINT_32 u4OBSSAirTimeRatio[RAM_BAND_NUM];
    UINT_8  u1Rule[RAM_BAND_NUM];
    /* SRSD - OBSS Monitor */
    UINT_8  u1ModeMcsIdx[RAM_BAND_NUM];
    UINT_8  u1SrTxState[RAM_BAND_NUM];
    UINT_8  u1LowTrafficCnt[RAM_BAND_NUM];
    UINT_8  u1ContWeakChkPnt[RAM_BAND_NUM];
    UINT_8  u1ObssLongPktPnt[RAM_BAND_NUM];
    UINT_16 u2ObssLongPkt[RAM_BAND_NUM][3];
    UINT_16 u2RxrptMcs[12][RAM_BAND_NUM];
    UINT_32 u4TxByteSum[RAM_BAND_NUM];
    UINT_32 u4TxdPgCnt[RAM_BAND_NUM];
    UINT_32 u4SrTxCnt[RAM_BAND_NUM];
    /*End - SRSD - OBSS Monitor*/
	/* SRSD- Mesh Flag */
	BOOLEAN  fgSrMeshSDFlag;
	UINT_8  u1Reserve[3];
    /* End SRSD- Mesh Flag */
} SR_SD_T, *P_SR_SD_T;
/* END SR SD (Scene Detection) */

/* SR SRG BITMAP */
typedef struct _SR_SRG_BITMAP_T {
    UINT_32 u4Color_31_0[RAM_BAND_NUM];
    UINT_32 u4Color_63_32[RAM_BAND_NUM];
    UINT_32 u4pBssid_31_0[RAM_BAND_NUM];
    UINT_32 u4pBssid_63_32[RAM_BAND_NUM];
} SR_SRG_BITMAP_T, *P_SR_SRG_BITMAP_T;
/* END SR SRG BITMAP */

/** SR SIGA FLAG*/
typedef struct _SR_SIGA_FLAG_T {
	UINT_8 u1Bssid;
	UINT_8 u1reserved[3];
	UINT_8 u1SigaFlag[20];
} SR_SIGA_FLAG_T, *P_SR_SIGA_FLAG_T;

/* END SR SIGA FLAG */

struct _SR_SIGA_AUTO_FLAG_T {
	UINT_8 u1SrSigaAutoFlag;
	UINT_8 au1Reserved[3];
};

enum _ENUM_SR_SELF_BM_MODE_T {
	ENUM_SR_SELF_BM_AUTO = 0,
	ENUM_SR_SELF_BM_MANUAL,
	ENUM_SR_SELF_BM_NUM
};

enum _ENUM_SR_TOPO_LOCK_T {
	ENUM_SR_TOPO_LOCK_AUTO = 0,
	ENUM_SR_TOPO_LOCK_MANUAL,
	ENUM_SR_TOPO_LOCK_NUM
};

enum _ENUM_SR_MESH_BH_TYPE_T {
	ENUM_BH_TYPE_NO_WIFI = 0,
	ENUM_BH_TYPE_WIFI,
	ENUM_BH_TYPE_NUM
};

enum _ENUM_MESH_RMT_FH_STAT_T {
	ENUM_RMT_FH_INVLD_BSSID = 0,
	ENUM_RMT_FH_SCAN_SUCCESS,
	ENUM_RMT_FH_SCAN_FAIL,
	ENUM_RMT_FH_SCAN_INTR_FAIL
};

struct _SET_REMOTE_FH_PARAMS {
	INT_8  i1Rssi;
	UINT_8 u1RemoteFhStat;
	UINT_8 au1Reserved[2];
};

struct _SET_REMOTE_BH_PARAMS {
	UINT_16 u2RemoteBhWcid;
	UINT_8  u1RemoteBhType;
	UINT_8  u1Reserved;
};

struct _SET_MAP_TOPO_PARAMS {
	UINT_8 u1MapDevCount;
	UINT_8 u1MapDevSrSupportMode;
	UINT_8 u1SelfRole;
	UINT_8 u1Reserved;
};

union _SR_MESH_TOPOLOGY_T {
	struct _SET_REMOTE_FH_PARAMS rRemoteFhParams;
	struct _SET_REMOTE_BH_PARAMS rRemoteBhParams;
	struct _SET_MAP_TOPO_PARAMS  rMapTopoParams;
};

/** SR UL TRAFFIC STATUS */
struct _SR_UL_STATUS_T {
	UINT_8 u1UlStatus;
	UINT_8 au1Reserved[3];
};
/* END  SR UL TRAFFIC STATUS */

struct _SR_CMD_MAP_BALANCE_T {
	UINT_8 u1MapBalance;
	UINT_8 au1Reserved[3];
};

struct _SR_CMD_MESH_UL_MODE_T {
	UINT_8 u1UlMode;
	UINT_8 au1Reserved[3];
};

struct _SR_CMD_STA_ALL_HE_T {
	UINT_8 u1StaAllHe;
	UINT_8 au1Reserved[3];
};

/** End FW & DRV sync with sr_if.h **/

/** FW & DRV sync with sr_cmd.c **/
/** SR Command */
typedef enum _ENUM_SR_CMD_SUBID {
    SR_CMD_Reserve = 0x0,
    SR_CMD_SET_SR_CFG_SR_ENABLE = 0x1,
    SR_CMD_GET_SR_CFG_SR_ENABLE = 0x2,
    SR_CMD_SET_SR_CFG_SR_SD_ENABLE = 0x3,
    SR_CMD_GET_SR_CFG_SR_SD_ENABLE = 0x4,
    SR_CMD_SET_SR_CFG_SR_MODE = 0x5,
    SR_CMD_GET_SR_CFG_SR_MODE = 0x6,
    SR_CMD_SET_SR_CFG_DISRT_ENABLE = 0x7,
    SR_CMD_GET_SR_CFG_DISRT_ENABLE = 0x8,
    SR_CMD_SET_SR_CFG_DISRT_MIN_RSSI = 0x9,
    SR_CMD_GET_SR_CFG_DISRT_MIN_RSSI = 0xA,
    SR_CMD_SET_SR_CFG_SR_BF = 0xB,
    SR_CMD_GET_SR_CFG_SR_BF = 0xC,
    SR_CMD_SET_SR_CFG_SR_ATF = 0xD,
    SR_CMD_GET_SR_CFG_SR_ATF = 0xE,
    SR_CMD_SET_SR_CFG_TXC_QUEUE = 0xF,
    SR_CMD_GET_SR_CFG_TXC_QUEUE = 0x10,
    SR_CMD_SET_SR_CFG_TXC_QID = 0x11,
    SR_CMD_GET_SR_CFG_TXC_QID = 0x12,
    SR_CMD_SET_SR_CFG_TXC_PATH = 0x13,
    SR_CMD_GET_SR_CFG_TXC_PATH = 0x14,
    SR_CMD_SET_SR_CFG_AC_METHOD = 0x15,
    SR_CMD_GET_SR_CFG_AC_METHOD = 0x16,
    SR_CMD_SET_SR_CFG_SR_PERIOD_THR = 0x17,
    SR_CMD_GET_SR_CFG_SR_PERIOD_THR = 0x18,
    SR_CMD_SET_SR_CFG_QUERY_TXD_METHOD = 0x19,
    SR_CMD_GET_SR_CFG_QUERY_TXD_METHOD = 0x1A,
    SR_CMD_SET_SR_CFG_SR_SD_CG_RATIO = 0x1B,
    SR_CMD_GET_SR_CFG_SR_SD_CG_RATIO = 0x1C,
    SR_CMD_SET_SR_CFG_SR_SD_OBSS_RATIO = 0x1D,
    SR_CMD_GET_SR_CFG_SR_SD_OBSS_RATIO = 0x1E,
    SR_CMD_SET_SR_CFG_PROFILE = 0x1F,
    SR_CMD_GET_SR_CFG_PROFILE = 0x20,
    SR_CMD_SET_SR_CFG_FNQ_ENABLE = 0x21,
    SR_CMD_GET_SR_CFG_FNQ_ENABLE = 0x22,
    SR_CMD_SET_SR_CFG_DPD_ENABLE = 0x23,
    SR_CMD_GET_SR_CFG_DPD_ENABLE = 0x24,
    SR_CMD_SET_SR_CFG_SR_TX_ENABLE = 0x25,
    SR_CMD_GET_SR_CFG_SR_TX_ENABLE = 0x26,
    SR_CMD_SET_SR_CFG_SR_SD_OM_ENABLE = 0x27,
    SR_CMD_GET_SR_CFG_SR_SD_OM_ENABLE = 0x28,
    SR_CMD_SET_SR_CFG_SR_TX_ALIGN_ENABLE = 0x29,
    SR_CMD_GET_SR_CFG_SR_TX_ALIGN_ENABLE = 0x2A,
    SR_CMD_SET_SR_CFG_SR_TX_ALIGN_RSSI_THR = 0x2B,
    SR_CMD_GET_SR_CFG_SR_TX_ALIGN_RSSI_THR = 0x2C,
    SR_CMD_SET_SR_CFG_SR_DABS_MODE = 0x2D,
    SR_CMD_GET_SR_CFG_SR_DABS_MODE = 0x2E,
    SR_CMD_SET_SR_CFG_MESH_SR_REMOTE_STA_MODE = 0x2F,
    SR_CMD_GET_SR_CFG_MESH_SR_REMOTE_STA_MODE = 0x30,
    SR_CMD_SET_SR_CFG_SR_DROP_MIN_MCS = 0x31,
    SR_CMD_GET_SR_CFG_SR_DROP_MIN_MCS = 0x32,
	SR_CMD_SET_SR_CFG_SR_DPD_THRESHOLD = 0x33,
	SR_CMD_GET_SR_CFG_SR_DPD_THRESHOLD = 0x34,
    /*Reseve for SR_CFG = 0x35*/
    /*Reseve for SR_CFG = 0x36*/
    /*Reseve for SR_CFG = 0x37*/
    /*Reseve for SR_CFG = 0x38*/
    /*Reseve for SR_CFG = 0x39*/
    /*Reseve for SR_CFG = 0x3A*/
    /*Reseve for SR_CFG = 0x3B*/
    /*Reseve for SR_CFG = 0x3C*/
    /*Reseve for SR_CFG = 0x3D*/
    /*Reseve for SR_CFG = 0x3E*/
    /*Reseve for SR_CFG = 0x3F*/
    /*Reseve for SR_CFG = 0x40*/
    /*Reseve for SR_CFG = 0x41*/
    /*Reseve for SR_CFG = 0x42*/
    /*Reseve for SR_CFG = 0x43*/
    /*Reseve for SR_CFG = 0x44*/
    /*Reseve for SR_CFG = 0x45*/
    /*Reseve for SR_CFG = 0x46*/
    /*Reseve for SR_CFG = 0x47*/
    /*Reseve for SR_CFG = 0x48*/
    /*Reseve for SR_CFG = 0x49*/
    /*Reseve for SR_CFG = 0x4A*/
    /*Reseve for SR_CFG = 0x4B*/
    /*Reseve for SR_CFG = 0x4C*/
    /*Reseve for SR_CFG = 0x4D*/
    /*Reseve for SR_CFG = 0x4E*/
    /*Reseve for SR_CFG = 0x4F*/
    /*Reseve for SR_CFG = 0x50*/
    /*Reseve for SR_CFG = 0x51*/
    /*Reseve for SR_CFG = 0x52*/
    /*Reseve for SR_CFG = 0x53*/
    /*Reseve for SR_CFG = 0x54*/
    /*Reseve for SR_CFG = 0x55*/
    /*Reseve for SR_CFG = 0x56*/
    /*Reseve for SR_CFG = 0x57*/
    /*Reseve for SR_CFG = 0x58*/
    /*Reseve for SR_CFG = 0x59*/
    /*Reseve for SR_CFG = 0x5A*/
    /*Reseve for SR_CFG = 0x5B*/
    /*Reseve for SR_CFG = 0x5C*/
    /*Reseve for SR_CFG = 0x5D*/
    /*Reseve for SR_CFG = 0x5E*/
    /*Reseve for SR_CFG = 0x5F*/
    /*Reseve for SR_CFG = 0x60*/
    /*Reseve for SR_CFG = 0x61*/
    /*Reseve for SR_CFG = 0x62*/
    /*Reseve for SR_CFG = 0x63*/
    /*Reseve for SR_CFG = 0x64*/
    /*Reseve for SR_CFG = 0x65*/
    /*Reseve for SR_CFG = 0x66*/
    /*Reseve for SR_CFG = 0x67*/
    /*Reseve for SR_CFG = 0x68*/
    /*Reseve for SR_CFG = 0x69*/
    /*Reseve for SR_CFG = 0x6A*/
    /*Reseve for SR_CFG = 0x6B*/
    /*Reseve for SR_CFG = 0x6C*/
    /*Reseve for SR_CFG = 0x6D*/
    /*Reseve for SR_CFG = 0x6E*/
    /*Reseve for SR_CFG = 0x6F*/
    /*Reseve for SR_CFG = 0x70*/
    /*Reseve for SR_CFG = 0x71*/
    /*Reseve for SR_CFG = 0x72*/
    /*Reseve for SR_CFG = 0x73*/
    /*Reseve for SR_CFG = 0x74*/
    /*Reseve for SR_CFG = 0x75*/
    /*Reseve for SR_CFG = 0x76*/
    /*Reseve for SR_CFG = 0x77*/
    /*Reseve for SR_CFG = 0x78*/
    /*Reseve for SR_CFG = 0x79*/
    /*Reseve for SR_CFG = 0x7A*/
    /*Reseve for SR_CFG = 0x7B*/
    /*Reseve for SR_CFG = 0x7C*/
    /*Reseve for SR_CFG = 0x7D*/
    /*Reseve for SR_CFG = 0x7E*/
    /*Reseve for SR_CFG = 0x7F*/
    SR_CMD_SET_SR_SRG_BITMAP = 0x80,
    SR_CMD_GET_SR_SRG_BITMAP = 0x81,
    SR_CMD_SET_SR_SRG_BITMAP_REFRESH = 0x82,
    SR_CMD_GET_SR_CNT_ALL = 0x83,
    SR_CMD_GET_SR_SD_ALL = 0x84,
    SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL = 0x85,
    SR_CMD_SET_SR_GLO_VAR_STA_CTRL = 0x86,
    SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL = 0x87,
    SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO = 0x88,
    SR_CMD_SET_SR_MESH_SRG_BITMAP = 0x89,
    SR_CMD_GET_SR_MESH_SRG_BITMAP = 0x8A,
    /*Reseve for SR SW Module = 0x8B*/
    /*Reseve for SR SW Module = 0x8C*/
    /*Reseve for SR SW Module = 0x8D*/
    /*Reseve for SR SW Module = 0x8E*/
    SR_CMD_SET_DOWNLINK_STA_THRESHOLD = 0x8F,
    SR_CMD_GET_DOWNLINK_STA_THRESHOLD = 0x90,
    SR_CMD_SET_BH_DOWNLINK_MESH_SR_THRESHOLD = 0x91,
    SR_CMD_GET_BH_DOWNLINK_MESH_SR_THRESHOLD = 0x92,
    SR_CMD_SET_FH_DOWNLINK_MESH_SR_THRESHOLD = 0x93,
    SR_CMD_GET_FH_DOWNLINK_MESH_SR_THRESHOLD = 0x94,
    SR_CMD_SET_FORHIB_MESH_SR = 0x95,
    SR_CMD_GET_FORHIB_MESH_SR = 0X96,
    SR_CMD_SET_BH_MESH_SR_BITMAP = 0x97,
    SR_CMD_GET_BH_MESH_SR_BITMAP = 0x98,
    SR_CMD_SET_FH_MESH_SR_BITMAP = 0x99,
    SR_CMD_GET_FH_MESH_SR_BITMAP = 0x9A,
    SR_CMD_SET_FORHIB_MESH_SR_RESET = 0x9B,
    /*Reseve for SR SW Module = 0x9C*/
    /*Reseve for SR SW Module = 0x9D*/
    /*Reseve for SR SW Module = 0x9E*/
    /*Reseve for SR SW Module = 0x9F*/
    /*Reseve for SR SW Module = 0xA0*/
    /*Reseve for SR SW Module = 0xA1*/
    /*Reseve for SR SW Module = 0xA2*/
    /*Reseve for SR SW Module = 0xA3*/
    /*Reseve for SR SW Module = 0xA4*/
    /*Reseve for SR SW Module = 0xA5*/
    /*Reseve for SR SW Module = 0xA6*/
    /*Reseve for SR SW Module = 0xA7*/
    /*Reseve for SR SW Module = 0xA8*/
    /*Reseve for SR SW Module = 0xA9*/
    /*Reseve for SR SW Module = 0xAA*/
    /*Reseve for SR SW Module = 0xAB*/
    /*Reseve for SR SW Module = 0xAC*/
    /*Reseve for SR SW Module = 0xAD*/
    /*Reseve for SR SW Module = 0xAE*/
    /*Reseve for SR SW Module = 0xAF*/
    /*Reseve for SR SW Module = 0xB0*/
    /*Reseve for SR SW Module = 0xB1*/
    /*Reseve for SR SW Module = 0xB2*/
    /*Reseve for SR SW Module = 0xB3*/
    /*Reseve for SR SW Module = 0xB4*/
    /*Reseve for SR SW Module = 0xB5*/
    /*Reseve for SR SW Module = 0xB6*/
    /*Reseve for SR SW Module = 0xB7*/
    /*Reseve for SR SW Module = 0xB8*/
    /*Reseve for SR SW Module = 0xB9*/
    /*Reseve for SR SW Module = 0xBA*/
    /*Reseve for SR SW Module = 0xBB*/
    /*Reseve for SR SW Module = 0xBC*/
    /*Reseve for SR SW Module = 0xBD*/
    /*Reseve for SR SW Module = 0xBE*/
    /*Reseve for SR SW Module = 0xBF*/
    SR_CMD_SET_SR_CAP_ALL_CTRL = 0xC0,
    SR_CMD_GET_SR_CAP_ALL_INFO = 0xC1,
    SR_CMD_SET_SR_PARA_ALL_CTRL = 0xC2,
    SR_CMD_GET_SR_PARA_ALL_INFO = 0xC3,
    SR_CMD_SET_SR_COND_ALL_CTRL = 0xC4,
    SR_CMD_GET_SR_COND_ALL_INFO = 0xC5,
    SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL = 0xC6,
    SR_CMD_GET_SR_RCPI_TBL_ALL_INFO = 0xC7,
    SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL = 0xC8,
    SR_CMD_GET_SR_RCPI_TBL_OFST_ALL_INFO = 0xC9,
    SR_CMD_SET_SR_Q_CTRL_ALL_CTRL = 0xCA,
    SR_CMD_GET_SR_Q_CTRL_ALL_INFO = 0xCB,
    SR_CMD_SET_SR_IBPD_ALL_CTRL = 0xCC,
    SR_CMD_GET_SR_IBPD_ALL_INFO = 0xCD,
    SR_CMD_SET_SR_NRT_ALL_CTRL = 0xCE,
    SR_CMD_GET_SR_NRT_ALL_INFO = 0xCF,
    SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL = 0xD0,
    SR_CMD_GET_SR_NRT_CTRL_ALL_INFO = 0xD1,
    SR_CMD_SET_SR_NRT_RESET_CTRL = 0xD2,
    SR_CMD_SET_SR_CAP_SREN_CTRL = 0xD3,
    SR_CMD_GET_SR_IND_ALL_INFO = 0xD4,
    SR_CMD_SET_SR_FNQ_CTRL_ALL_CTRL = 0xD5,
    SR_CMD_GET_SR_FNQ_CTRL_ALL_INFO = 0xD6,
    SR_CMD_SET_SR_FRM_FILT_ALL_CTRL = 0xD7,
    SR_CMD_GET_SR_FRM_FILT_ALL_INFO = 0xD8,
    SR_CMD_SET_SR_INTERPS_CTRL_ALL_CTRL = 0xD9,
    SR_CMD_GET_SR_INTERPS_CTRL_ALL_INFO = 0xDA,
    SR_CMD_GET_SR_INTERPS_DBG_ALL_INFO = 0xDB,
    SR_CMD_SET_SR_SIGA_FLAG_CTRL = 0xDC,
    SR_CMD_GET_SR_SIGA_FLAG_INFO = 0xDD,
	SR_CMD_SET_SR_SIGA_AUTO_FLAG_CTRL = 0xDE,
	SR_CMD_GET_SR_SIGA_AUTO_FLAG_INFO = 0xDF,
	SR_CMD_SET_REMOTE_FH_RSSI = 0xE0,
	SR_CMD_GET_REMOTE_FH_RSSI = 0xE1,
	SR_CMD_SET_REMOTE_BH_INFO = 0xE2,
	SR_CMD_GET_REMOTE_BH_INFO = 0xE3,
	SR_CMD_SET_MAP_TOPO = 0xE4,
	SR_CMD_GET_MAP_TOPO = 0xE5,
	SR_CMD_SET_MAP_TRAFFIC_STATUS = 0xE6,
	SR_CMD_GET_MAP_TRAFFIC_STATUS = 0xE7,
	SR_CMD_SET_MAP_BALANCE = 0xE8,
	SR_CMD_GET_MESH_PHASE = 0xE9,
	SR_CMD_SET_SR_MESH_SR_SD_CTRL = 0xEA,
	SR_CMD_SET_MESH_UL_MODE = 0xEB,
	SR_CMD_SET_MESH_FH_RSSI_TH = 0xEC,
    SR_CMD_GET_MESH_FH_RSSI_TH = 0xED,
    /*Reseve for SR HW Module = 0xED*/
    /*Reseve for SR HW Module = 0xEE*/
    /*Reseve for SR HW Module = 0xEF*/
    /*Reseve for SR HW Module = 0xF0*/
    /*Reseve for SR HW Module = 0xF1*/
    /*Reseve for SR HW Module = 0xF2*/
    /*Reseve for SR HW Module = 0xF3*/
    /*Reseve for SR HW Module = 0xF4*/
    /*Reseve for SR HW Module = 0xF5*/
    /*Reseve for SR HW Module = 0xF6*/
    /*Reseve for SR HW Module = 0xF7*/
    /*Reseve for SR HW Module = 0xF8*/
    /*Reseve for SR HW Module = 0xF9*/
    /*Reseve for SR HW Module = 0xFA*/
    /*Reseve for SR HW Module = 0xFB*/
    /*Reseve for SR HW Module = 0xFC*/
    /*Reseve for SR HW Module = 0xFD*/
    /*Reseve for SR HW Module = 0xFE*/
    /*Reseve for SR HW Module = 0xFF*/
} ENUM_SR_CMD_SUBID, *P_ENUM_SR_CMD_SUBID;

typedef struct _SR_CMD_T {
    UINT_8  u1CmdSubId;
    UINT_8  u1ArgNum;
    UINT_8  u1DbdcIdx;
    UINT_8  u1Status;
    UINT_8  u1DropTaIdx;
    UINT_8  u1StaIdx;	/* #256STA */
    UINT_8  u1bssid;
    UINT_8  u1Rsv;
    UINT_32 u4Value;
} SR_CMD_T, *P_SR_CMD_T;

typedef struct _SR_CMD_SR_CAP_T_SR_V1 {
    SR_CMD_T rSrCmd;
    WH_SR_CAP_T_SR_V1 rSrCap;
	/** Reserve for 8-byte aligned*/
	UINT_8 u4Rsv[8];
} SR_CMD_SR_CAP_T_SR_V1, *P_SR_CMD_SR_CAP_T_SR_V1;

typedef struct _SR_CMD_SR_CAP_T_SR_V2 {
    SR_CMD_T rSrCmd;
    WH_SR_CAP_T_SR_V2 rSrCap;
} SR_CMD_SR_CAP_T_SR_V2, *P_SR_CMD_SR_CAP_T_SR_V2;

typedef struct _SR_CMD_SR_PARA_T {
    SR_CMD_T rSrCmd;
    WH_SR_PARA_T rSrPara;
} SR_CMD_SR_PARA_T, *P_SR_CMD_SR_PARA_T;

typedef struct _SR_CMD_SR_IND_T {
    SR_CMD_T rSrCmd;
    WH_SR_IND_T rSrInd;
} SR_CMD_SR_IND_T, *P_SR_CMD_SR_IND_T;

typedef struct _SR_GLOBAL_VAR_SINGLE_DROP_TA_T {
    UINT_8 u1Rsv;
    UINT_8 u1CurSrDropTaIdx;
    UINT_16 u2SrTtlTxCntThr;
    SR_DROP_TA_INFO_T rSrDropTaInfo;
} SR_GLOBAL_VAR_SINGLE_DROP_TA_T, *P_SR_GLOBAL_VAR_SINGLE_DROP_TA_T;

typedef struct _SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T {
    SR_CMD_T rSrCmd;
    SR_GLOBAL_VAR_SINGLE_DROP_TA_T rSrGlobalVarSingleDropTa;
} SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T, *P_SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T;

typedef struct _SR_CMD_SR_COND_T_SR_V1 {
    SR_CMD_T rSrCmd;
    SR_COND_T_SR_V1 rSrCond;
} SR_CMD_SR_COND_T_SR_V1, *P_SR_CMD_SR_COND_T_SR_V1;

typedef struct _SR_CMD_SR_COND_T_SR_V2 {
    SR_CMD_T rSrCmd;
    WH_SR_COND_T_SR_V2 rSrCond;
} SR_CMD_SR_COND_T_SR_V2, *P_SR_CMD_SR_COND_T_SR_V2;

typedef struct _SR_CMD_SR_RCPITBL_T {
    SR_CMD_T rSrCmd;
    WH_SR_RCPITBL_T rSrRcpiTbl;
} SR_CMD_SR_RCPITBL_T, *P_SR_CMD_SR_RCPITBL_T;

typedef struct _SR_CMD_SR_RCPITBL_OFST_T_SR_V1 {
    SR_CMD_T rSrCmd;
    SR_RCPITBL_OFST_T_SR_V1 rSrRcpiTblOfst;
} SR_CMD_SR_RCPITBL_OFST_T_SR_V1, *P_SR_CMD_SR_RCPITBL_OFST_T_SR_V1;

typedef struct _SR_CMD_SR_RCPITBL_OFST_T_SR_V2 {
    SR_CMD_T rSrCmd;
    WH_SR_RCPITBL_OFST_T_SR_V2 rSrRcpiTblOfst;
} SR_CMD_SR_RCPITBL_OFST_T_SR_V2, *P_SR_CMD_SR_RCPITBL_OFST_T_SR_V2;

typedef struct _SR_CMD_SR_Q_CTRL_T_SR_V1 {
    SR_CMD_T rSrCmd;
    WH_SR_QUEUE_CTRL_T_SR_V1 rSrQCtrl;
	/** Reserve for 4-byte aligned*/
	UINT_8 u4Rsv[4];
} SR_CMD_SR_Q_CTRL_T_SR_V1, *P_SR_CMD_SR_Q_CTRL_T_SR_V1;

typedef struct _SR_CMD_SR_Q_CTRL_T_SR_V2 {
    SR_CMD_T rSrCmd;
    WH_SR_QUEUE_CTRL_T_SR_V2 rSrQCtrl;
} SR_CMD_SR_Q_CTRL_T_SR_V2, *P_SR_CMD_SR_Q_CTRL_T_SR_V2;

typedef struct _SR_CMD_SR_IBPD_T {
    SR_CMD_T rSrCmd;
    WH_SR_IBPD_T rSrIBPD;
} SR_CMD_SR_IBPD_T, *P_SR_CMD_SR_IBPD_T;

typedef struct _SR_CMD_SR_NRT_T_SR_V1 {
    SR_CMD_T rSrCmd;
    SR_NRT_T_SR_V1 rSrNRT;
} SR_CMD_SR_NRT_T_SR_V1, *P_SR_CMD_SR_NRT_T_SR_V1;

typedef struct _SR_CMD_SR_NRT_T_SR_V2 {
    SR_CMD_T rSrCmd;
    WH_SR_NRT_T_SR_V2 rSrNRT;
	/** Reserve for 4-byte aligned*/
	UINT_8 u4Rsv[4];
} SR_CMD_SR_NRT_T_SR_V2, *P_SR_CMD_SR_NRT_T_SR_V2;

typedef struct _SR_CMD_SR_NRT_CTRL_T_SR_V1 {
    SR_CMD_T rSrCmd;
    SR_NRT_CTRL_T_SR_V1 rSrNRTCtrl;
} SR_CMD_SR_NRT_CTRL_T_SR_V1, *P_SR_CMD_SR_NRT_CTRL_T_SR_V1;

typedef struct _SR_CMD_SR_NRT_CTRL_T_SR_V2 {
    SR_CMD_T rSrCmd;
    WH_SR_NRT_CTRL_T_SR_V2 rSrNRTCtrl;
} SR_CMD_SR_NRT_CTRL_T_SR_V2, *P_SR_CMD_SR_NRT_CTRL_T_SR_V2;

typedef struct _SR_CMD_SR_SRG_BITMAP_T {
    SR_CMD_T rSrCmd;
    SR_SRG_BITMAP_T rSrSrgBitmap;
} SR_CMD_SR_SRG_BITMAP_T, *P_SR_CMD_SR_SRG_BITMAP_T;

typedef struct _SR_CMD_SR_FNQ_CTRL_T {
    SR_CMD_T rSrCmd;
    WH_SR_FNQ_CTRL_T rSrFNQCtrl;
} SR_CMD_SR_FNQ_CTRL_T, *P_SR_CMD_SR_FNQ_CTRL_T;

typedef struct _SR_CMD_SR_FRM_FILT_T {
    SR_CMD_T rSrCmd;
    UINT_32 u4SrFrmFilt;
} SR_CMD_SR_FRM_FILT_T, *P_SR_CMD_SR_FRM_FILT_T;

typedef struct _SR_CMD_SR_INTERPS_CTRL_T {
    SR_CMD_T rSrCmd;
    WH_SR_INTERPS_CTRL_T rSrInterPsCtrl;
} SR_CMD_SR_INTERPS_CTRL_T, *P_SR_CMD_SR_INTERPS_CTRL_T;

typedef struct _SR_CMD_SR_SIGA_FLAG_T {
	SR_CMD_T rSrCmd;
	SR_SIGA_FLAG_T rSrSigaFlag;
} SR_CMD_SR_SIGA_FLAG_T, *P_SR_CMD_SR_SIGA_FLAG_T;

struct _SR_CMD_SR_SIGA_AUTO_FLAG_T {
	SR_CMD_T rSrCmd;
	struct _SR_SIGA_AUTO_FLAG_T rSrSigaAutoFlag;
};

struct _SR_CMD_MESH_TOPOLOGY_T {
	SR_CMD_T rSrCmd;
	union _SR_MESH_TOPOLOGY_T rSrCmdMeshTopo;
};

struct SR_MESH_SR_DL_STA_THRESHOLD_T {
	UINT_8 u1BSSID;
	INT_8 irssi;
	UINT_8 au1Reserved[2];
};

struct _SR_MESH_FH_RSSI_TH_T {
	INT_8 i1SrMeshFhRssiTh;
	INT_8 i1SrMeshSrAllowTargetTh;
	INT_8 i1SrMeshInterSrAllowTh;
	UINT_8 au1reserved;
};

struct _SR_CMD_MESH_FH_RSSI_TH_T {
	SR_CMD_T rSrCmd;
	struct _SR_MESH_FH_RSSI_TH_T rSrMeshFhRssiTh;
};

struct SR_CMD_MESH_SR_DL_STA_MESH_THRESHOLD_T {
	SR_CMD_T rSrCmd;
	struct SR_MESH_SR_DL_STA_THRESHOLD_T rSrDLStaThrehsold;
};

struct _SR_CMD_SR_UL_TRAFFIC_STATUS_T {
	SR_CMD_T rSrCmd;
	struct _SR_UL_STATUS_T rSrUlStatus;
};

struct _SR_CMD_SET_MAP_BALANCE_T {
	SR_CMD_T rSrCmd;
	struct _SR_CMD_MAP_BALANCE_T rSrMapBalance;
};

struct _SR_CMD_SET_MESH_UL_MODE_T {
	SR_CMD_T rSrCmd;
	struct _SR_CMD_MESH_UL_MODE_T rSrMeshUlMode;
};

/** End SR Command */
/** SR Event */
typedef enum _ENUM_SR_EVENT_SUBID {
    SR_EVENT_Reserve = 0x0,
    SR_EVENT_GET_SR_CFG_SR_ENABLE = 0x1,
    SR_EVENT_GET_SR_CFG_SR_SD_ENABLE = 0x2,
    SR_EVENT_GET_SR_CFG_SR_MODE = 0x3,
    SR_EVENT_GET_SR_CFG_DISRT_ENABLE = 0x4,
    SR_EVENT_GET_SR_CFG_DISRT_MIN_RSSI = 0x5,
    SR_EVENT_GET_SR_CFG_SR_BF = 0x6,
    SR_EVENT_GET_SR_CFG_SR_ATF = 0x7,
    SR_EVENT_GET_SR_CFG_TXC_QUEUE = 0x8,
    SR_EVENT_GET_SR_CFG_TXC_QID = 0x9,
    SR_EVENT_GET_SR_CFG_TXC_PATH = 0xA,
    SR_EVENT_GET_SR_CFG_AC_METHOD = 0xB,
    SR_EVENT_GET_SR_CFG_SR_PERIOD_THR = 0xC,
    SR_EVENT_GET_SR_CFG_QUERY_TXD_METHOD = 0xD,
    SR_EVENT_GET_SR_CFG_SR_SD_CG_RATIO = 0xE,
    SR_EVENT_GET_SR_CFG_SR_SD_OBSS_RATIO = 0xF,
    SR_EVENT_GET_SR_CFG_PROFILE = 0x10,
    SR_EVENT_GET_SR_CFG_FNQ_ENABLE = 0x11,
    SR_EVENT_GET_SR_CFG_DPD_ENABLE = 0x12,
    SR_EVENT_GET_SR_CFG_SR_TX_ENABLE = 0x13,
    SR_EVENT_GET_SR_CFG_SR_SD_OM_ENABLE = 0x14,
    SR_EVENT_GET_SR_CFG_SR_TX_ALIGN_ENABLE = 0x15,
    SR_EVENT_GET_SR_CFG_SR_TX_ALIGN_RSSI_THR = 0x16,
    SR_EVENT_GET_SR_CFG_SR_DABS_MODE = 0x17,
    SR_EVENT_GET_SR_CFG_MESH_REMOTE_STA_MODE = 0x18,
	SR_EVENT_GET_SR_CFG_SR_DPD_THRESHOLD = 0X19,
    SR_EVENT_GET_SR_CFG_SR_DROP_MIN_MCS = 0x1A,
    /*Reseve for SR_CFG = 0x1B*/
    /*Reseve for SR_CFG = 0x1C*/
    /*Reseve for SR_CFG = 0x1D*/
    /*Reseve for SR_CFG = 0x1E*/
    /*Reseve for SR_CFG = 0x1F*/
    /*Reseve for SR_CFG = 0x20*/
    /*Reseve for SR_CFG = 0x21*/
    /*Reseve for SR_CFG = 0x22*/
    /*Reseve for SR_CFG = 0x23*/
    /*Reseve for SR_CFG = 0x24*/
    /*Reseve for SR_CFG = 0x25*/
    /*Reseve for SR_CFG = 0x26*/
    /*Reseve for SR_CFG = 0x27*/
    /*Reseve for SR_CFG = 0x28*/
    /*Reseve for SR_CFG = 0x29*/
    /*Reseve for SR_CFG = 0x2A*/
    /*Reseve for SR_CFG = 0x2B*/
    /*Reseve for SR_CFG = 0x2C*/
    /*Reseve for SR_CFG = 0x2D*/
    /*Reseve for SR_CFG = 0x2E*/
    /*Reseve for SR_CFG = 0x2F*/
    /*Reseve for SR_CFG = 0x30*/
    /*Reseve for SR_CFG = 0x31*/
    /*Reseve for SR_CFG = 0x32*/
    /*Reseve for SR_CFG = 0x33*/
    /*Reseve for SR_CFG = 0x34*/
    /*Reseve for SR_CFG = 0x35*/
    /*Reseve for SR_CFG = 0x36*/
    /*Reseve for SR_CFG = 0x37*/
    /*Reseve for SR_CFG = 0x38*/
    /*Reseve for SR_CFG = 0x39*/
    /*Reseve for SR_CFG = 0x3A*/
    /*Reseve for SR_CFG = 0x3B*/
    /*Reseve for SR_CFG = 0x3C*/
    /*Reseve for SR_CFG = 0x3D*/
    /*Reseve for SR_CFG = 0x3E*/
    /*Reseve for SR_CFG = 0x3F*/
    /*Reseve for SR_CFG = 0x40*/
    /*Reseve for SR_CFG = 0x41*/
    /*Reseve for SR_CFG = 0x42*/
    /*Reseve for SR_CFG = 0x43*/
    /*Reseve for SR_CFG = 0x44*/
    /*Reseve for SR_CFG = 0x45*/
    /*Reseve for SR_CFG = 0x46*/
    /*Reseve for SR_CFG = 0x47*/
    /*Reseve for SR_CFG = 0x48*/
    /*Reseve for SR_CFG = 0x49*/
    /*Reseve for SR_CFG = 0x4A*/
    /*Reseve for SR_CFG = 0x4B*/
    /*Reseve for SR_CFG = 0x4C*/
    /*Reseve for SR_CFG = 0x4D*/
    /*Reseve for SR_CFG = 0x4E*/
    /*Reseve for SR_CFG = 0x4F*/
    /*Reseve for SR_CFG = 0x50*/
    /*Reseve for SR_CFG = 0x51*/
    /*Reseve for SR_CFG = 0x52*/
    /*Reseve for SR_CFG = 0x53*/
    /*Reseve for SR_CFG = 0x54*/
    /*Reseve for SR_CFG = 0x55*/
    /*Reseve for SR_CFG = 0x56*/
    /*Reseve for SR_CFG = 0x57*/
    /*Reseve for SR_CFG = 0x58*/
    /*Reseve for SR_CFG = 0x59*/
    /*Reseve for SR_CFG = 0x5A*/
    /*Reseve for SR_CFG = 0x5B*/
    /*Reseve for SR_CFG = 0x5C*/
    /*Reseve for SR_CFG = 0x5D*/
    /*Reseve for SR_CFG = 0x5E*/
    /*Reseve for SR_CFG = 0x5F*/
    /*Reseve for SR_CFG = 0x60*/
    /*Reseve for SR_CFG = 0x61*/
    /*Reseve for SR_CFG = 0x62*/
    /*Reseve for SR_CFG = 0x63*/
    /*Reseve for SR_CFG = 0x64*/
    /*Reseve for SR_CFG = 0x65*/
    /*Reseve for SR_CFG = 0x66*/
    /*Reseve for SR_CFG = 0x67*/
    /*Reseve for SR_CFG = 0x68*/
    /*Reseve for SR_CFG = 0x69*/
    /*Reseve for SR_CFG = 0x6A*/
    /*Reseve for SR_CFG = 0x6B*/
    /*Reseve for SR_CFG = 0x6C*/
    /*Reseve for SR_CFG = 0x6D*/
    /*Reseve for SR_CFG = 0x6E*/
    /*Reseve for SR_CFG = 0x6F*/
    /*Reseve for SR_CFG = 0x70*/
    /*Reseve for SR_CFG = 0x71*/
    /*Reseve for SR_CFG = 0x72*/
    /*Reseve for SR_CFG = 0x73*/
    /*Reseve for SR_CFG = 0x74*/
    /*Reseve for SR_CFG = 0x75*/
    /*Reseve for SR_CFG = 0x76*/
    /*Reseve for SR_CFG = 0x77*/
    /*Reseve for SR_CFG = 0x78*/
    /*Reseve for SR_CFG = 0x79*/
    /*Reseve for SR_CFG = 0x7A*/
    /*Reseve for SR_CFG = 0x7B*/
    /*Reseve for SR_CFG = 0x7C*/
    /*Reseve for SR_CFG = 0x7D*/
    /*Reseve for SR_CFG = 0x7E*/
    /*Reseve for SR_CFG = 0x7F*/
    SR_EVENT_GET_SR_SRG_BITMAP = 0x80,
    SR_EVENT_GET_SR_CNT_ALL = 0x81,
    SR_EVENT_GET_SR_SD_ALL = 0x82,
    SR_EVENT_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO = 0x83,
    SR_EVENT_GET_SR_MESH_SRG_BITMAP = 0x84,
    SR_EVENT_GET_SR_BH_MESH_SRG_BITMAP = 0x85,
    SR_EVENT_GET_SR_FH_MESH_SRG_BITMAP = 0x86,
    /*Reseve for SR SW Module = 0x87*/
    /*Reseve for SR SW Module = 0x88*/
    /*Reseve for SR SW Module = 0x89*/
    /*Reseve for SR SW Module = 0x8A*/
    /*Reseve for SR SW Module = 0x8B*/
    /*Reseve for SR SW Module = 0x8C*/
    /*Reseve for SR SW Module = 0x8D*/
    /*Reseve for SR SW Module = 0x8E*/
    /*Reseve for SR SW Module = 0x8F*/
    /*Reseve for SR SW Module = 0x90*/
    /*Reseve for SR SW Module = 0x91*/
    /*Reseve for SR SW Module = 0x92*/
    /*Reseve for SR SW Module = 0x93*/
    /*Reseve for SR SW Module = 0x94*/
    /*Reseve for SR SW Module = 0x95*/
    /*Reseve for SR SW Module = 0x96*/
    /*Reseve for SR SW Module = 0x97*/
    /*Reseve for SR SW Module = 0x98*/
    /*Reseve for SR SW Module = 0x99*/
    /*Reseve for SR SW Module = 0x9A*/
    /*Reseve for SR SW Module = 0x9B*/
    /*Reseve for SR SW Module = 0x9C*/
    /*Reseve for SR SW Module = 0x9D*/
    /*Reseve for SR SW Module = 0x9E*/
    /*Reseve for SR SW Module = 0x9F*/
    /*Reseve for SR SW Module = 0xA0*/
    /*Reseve for SR SW Module = 0xA1*/
    /*Reseve for SR SW Module = 0xA2*/
    /*Reseve for SR SW Module = 0xA3*/
    /*Reseve for SR SW Module = 0xA4*/
    /*Reseve for SR SW Module = 0xA5*/
    /*Reseve for SR SW Module = 0xA6*/
    /*Reseve for SR SW Module = 0xA7*/
    /*Reseve for SR SW Module = 0xA8*/
    /*Reseve for SR SW Module = 0xA9*/
    /*Reseve for SR SW Module = 0xAA*/
    /*Reseve for SR SW Module = 0xAB*/
    /*Reseve for SR SW Module = 0xAC*/
    /*Reseve for SR SW Module = 0xAD*/
    /*Reseve for SR SW Module = 0xAE*/
    /*Reseve for SR SW Module = 0xAF*/
    /*Reseve for SR SW Module = 0xB0*/
    /*Reseve for SR SW Module = 0xB1*/
    /*Reseve for SR SW Module = 0xB2*/
    /*Reseve for SR SW Module = 0xB3*/
    /*Reseve for SR SW Module = 0xB4*/
    /*Reseve for SR SW Module = 0xB5*/
    /*Reseve for SR SW Module = 0xB6*/
    /*Reseve for SR SW Module = 0xB7*/
    /*Reseve for SR SW Module = 0xB8*/
    /*Reseve for SR SW Module = 0xB9*/
    /*Reseve for SR SW Module = 0xBA*/
    /*Reseve for SR SW Module = 0xBB*/
    /*Reseve for SR SW Module = 0xBC*/
    /*Reseve for SR SW Module = 0xBD*/
    /*Reseve for SR SW Module = 0xBE*/
    /*Reseve for SR SW Module = 0xBF*/
    SR_EVENT_GET_SR_CAP_ALL_INFO = 0xC0,
    SR_EVENT_GET_SR_PARA_ALL_INFO = 0xC1,
    SR_EVENT_GET_SR_COND_ALL_INFO = 0xC2,
    SR_EVENT_GET_SR_RCPI_TBL_ALL_INFO = 0xC3,
    SR_EVENT_GET_SR_RCPI_TBL_OFST_ALL_INFO = 0xC4,
    SR_EVENT_GET_SR_Q_CTRL_ALL_INFO = 0xC5,
    SR_EVENT_GET_SR_IBPD_ALL_INFO = 0xC6,
    SR_EVENT_GET_SR_NRT_ALL_INFO = 0xC7,
    SR_EVENT_GET_SR_NRT_CTRL_ALL_INFO = 0xC8,
    SR_EVENT_GET_SR_IND_ALL_INFO = 0xC9,
    SR_EVENT_GET_SR_FNQ_CTRL_ALL_INFO = 0xCA,
    SR_EVENT_GET_SR_FRM_FILT_ALL_INFO = 0xCB,
    SR_EVENT_GET_SR_INTERPS_CTRL_ALL_INFO = 0xCC,
    SR_EVENT_GET_SR_INTERPS_DBG_ALL_INFO = 0xCD,
    SR_EVENT_GET_SR_SIGA_FLAG_ALL_INFO = 0xCE,
	SR_EVENT_GET_SR_SIGA_AUTO_FLAG_ALL_INFO = 0xCF,
	SR_EVENT_GET_REMOTE_FH_RSSI = 0xD0,
	SR_EVENT_GET_REMOTE_BH_INFO = 0xD1,
	SR_EVENT_GET_MAP_TOPO = 0xD2,
	SR_EVENT_GET_MAP_TRAFFIC_STATUS = 0xD3,
	SR_EVENT_SEND_MESH_UPLINK_TRAFFIC = 0xD4,
	SR_EVENT_GET_MESH_PHASE = 0xD5,
    SR_EVENT_GET_FH_RSSI_TH = 0xD6,
    SR_EVENT_GET_MESH_STA_RSSI_TH = 0xD7,
    SR_EVENT_GET_BH_DL_MESH_SR_RSSI_TH = 0xD8,
    SR_EVENT_GET_FH_DL_MESH_SR_RSSI_TH = 0xD9,
    SR_EVENT_GET_BH_FORBID_BITMAP = 0xDA,
    /*Reseve for SR HW Module = 0xDB*/
    /*Reseve for SR HW Module = 0xDC*/
    /*Reseve for SR HW Module = 0xDD*/
    /*Reseve for SR HW Module = 0xDE*/
    /*Reseve for SR HW Module = 0xDF*/
    /*Reseve for SR HW Module = 0xE0*/
    /*Reseve for SR HW Module = 0xE1*/
    /*Reseve for SR HW Module = 0xE2*/
    /*Reseve for SR HW Module = 0xE3*/
    /*Reseve for SR HW Module = 0xE4*/
    /*Reseve for SR HW Module = 0xE5*/
    /*Reseve for SR HW Module = 0xE6*/
    /*Reseve for SR HW Module = 0xE7*/
    /*Reseve for SR HW Module = 0xE8*/
    /*Reseve for SR HW Module = 0xE9*/
    /*Reseve for SR HW Module = 0xEA*/
    /*Reseve for SR HW Module = 0xEB*/
    /*Reseve for SR HW Module = 0xEC*/
    /*Reseve for SR HW Module = 0xED*/
    /*Reseve for SR HW Module = 0xEE*/
    /*Reseve for SR HW Module = 0xEF*/
    /*Reseve for SR HW Module = 0xF0*/
    /*Reseve for SR HW Module = 0xF1*/
    /*Reseve for SR HW Module = 0xF2*/
    /*Reseve for SR HW Module = 0xF3*/
    /*Reseve for SR HW Module = 0xF4*/
    /*Reseve for SR HW Module = 0xF5*/
    /*Reseve for SR HW Module = 0xF6*/
    /*Reseve for SR HW Module = 0xF7*/
    /*Reseve for SR HW Module = 0xF8*/
    /*Reseve for SR HW Module = 0xF9*/
    /*Reseve for SR HW Module = 0xFA*/
    /*Reseve for SR HW Module = 0xFB*/
    /*Reseve for SR HW Module = 0xFC*/
    /*Reseve for SR HW Module = 0xFD*/
    /*Reseve for SR HW Module = 0xFE*/
    /*Reseve for SR HW Module = 0xFF*/
} ENUM_SR_EVENT_SUBID, *P_ENUM_SR_EVENT_SUBID;

typedef struct _SR_EVENT_T {
    UINT_8  u1EventSubId;
    UINT_8  u1ArgNum;
    UINT_8  u1DbdcIdx;
    UINT_8  u1Status;
    UINT_8  u1DropTaIdx;
    UINT_8  u1StaIdx;	/* #256STA */
    UINT_8  u1Rsv[2];
    UINT_32 u4Value;
} SR_EVENT_T, *P_SR_EVENT_T;

typedef enum _ENUM_SR_EVENT_STATUS_T {
    SR_STATUS_SUCCESS = 0x0,
    SR_STATUS_SANITY_FAIL,
    SR_STATUS_CALL_MIDDLE_FAIL,
    SR_STATUS_SW_HW_VAL_NOT_SYNC,
    SR_STATUS_UNKNOWN,
    SR_STATUS_NUM
} ENUM_SR_EVENT_STATUS_T, *P_ENUM_SR_EVENT_STATUS_T;

typedef struct _SR_EVENT_SR_CAP_T_SR_V1 {
    SR_EVENT_T rSrEvent;
    WH_SR_CAP_T_SR_V1 rSrCap;
} SR_EVENT_SR_CAP_T_SR_V1, *P_SR_EVENT_SR_CAP_T_SR_V1;

typedef struct _SR_EVENT_SR_CAP_T_SR_V2 {
    SR_EVENT_T rSrEvent;
    WH_SR_CAP_T_SR_V2 rSrCap;
} SR_EVENT_SR_CAP_T_SR_V2, *P_SR_EVENT_SR_CAP_T_SR_V2;

typedef struct _SR_EVENT_SR_PARA_T {
    SR_EVENT_T rSrEvent;
    WH_SR_PARA_T rSrPara;
} SR_EVENT_SR_PARA_T, *P_SR_EVENT_SR_PARA_T;

typedef struct _SR_EVENT_SR_IND_T {
    SR_EVENT_T rSrEvent;
    WH_SR_IND_T rSrInd;
} SR_EVENT_SR_IND_T, *P_SR_EVENT_SR_IND_T;

typedef struct _SR_EVENT_SR_GLOBAL_VAR_SINGLE_DROP_TA_T {
    SR_EVENT_T rSrEvent;
    SR_GLOBAL_VAR_SINGLE_DROP_TA_T rSrGlobalVarSingleDropTa;
} SR_EVENT_SR_GLOBAL_VAR_SINGLE_DROP_TA_T, *P_SR_EVENT_SR_GLOBAL_VAR_SINGLE_DROP_TA_T;

typedef struct _SR_EVENT_SR_COND_T_SR_V1 {
    SR_EVENT_T rSrEvent;
    SR_COND_T_SR_V1 rSrCond;
} SR_EVENT_SR_COND_T_SR_V1, *P_SR_EVENT_SR_COND_T_SR_V1;

typedef struct _SR_EVENT_SR_COND_T_SR_V2 {
    SR_EVENT_T rSrEvent;
    WH_SR_COND_T_SR_V2 rSrCond;
} SR_EVENT_SR_COND_T_SR_V2, *P_SR_EVENT_SR_COND_T_SR_V2;

typedef struct _SR_EVENT_SR_RCPITBL_T {
    SR_EVENT_T rSrEvent;
    WH_SR_RCPITBL_T rSrRcpiTbl;
} SR_EVENT_SR_RCPITBL_T, *P_SR_EVENT_SR_RCPITBL_T;

typedef struct _SR_EVENT_SR_RCPITBL_OFST_T_SR_V1 {
    SR_EVENT_T rSrEvent;
    SR_RCPITBL_OFST_T_SR_V1 rSrRcpiTblOfst;
} SR_EVENT_SR_RCPITBL_OFST_T_SR_V1, *P_SR_EVENT_SR_RCPITBL_OFST_T_SR_V1;

typedef struct _SR_EVENT_SR_RCPITBL_OFST_T_SR_V2 {
    SR_EVENT_T rSrEvent;
    WH_SR_RCPITBL_OFST_T_SR_V2 rSrRcpiTblOfst;
} SR_EVENT_SR_RCPITBL_OFST_T_SR_V2, *P_SR_EVENT_SR_RCPITBL_OFST_T_SR_V2;

typedef struct _SR_EVENT_SR_Q_CTRL_T_SR_V1 {
    SR_EVENT_T rSrEvent;
    WH_SR_QUEUE_CTRL_T_SR_V1 rSrQCtrl;
} SR_EVENT_SR_Q_CTRL_T_SR_V1, *P_SR_EVENT_SR_Q_CTRL_T_SR_V1;

typedef struct _SR_EVENT_SR_Q_CTRL_T_SR_V2 {
    SR_EVENT_T rSrEvent;
    WH_SR_QUEUE_CTRL_T_SR_V2 rSrQCtrl;
} SR_EVENT_SR_Q_CTRL_T_SR_V2, *P_SR_EVENT_SR_Q_CTRL_T_SR_V2;

typedef struct _SR_EVENT_SR_IBPD_T {
    SR_EVENT_T rSrEvent;
    WH_SR_IBPD_T rSrIBPD;
} SR_EVENT_SR_IBPD_T, *P_SR_EVENT_SR_IBPD_T;

typedef struct _SR_EVENT_SR_NRT_T_SR_V1 {
    SR_EVENT_T rSrEvent;
    SR_NRT_T_SR_V1 rSrNRT[SR_NRT_ROW_NUM];
} SR_EVENT_SR_NRT_T_SR_V1, *P_SR_EVENT_SR_NRT_T_SR_V1;

typedef struct _SR_EVENT_SR_NRT_T_SR_V2 {
    SR_EVENT_T rSrEvent;
    WH_SR_NRT_T_SR_V2 rSrNRT[SR_NRT_ROW_NUM];
} SR_EVENT_SR_NRT_T_SR_V2, *P_SR_EVENT_SR_NRT_T_SR_V2;

typedef struct _SR_EVENT_SR_NRT_CTRL_T_SR_V1 {
    SR_EVENT_T rSrEvent;
    SR_NRT_CTRL_T_SR_V1 rSrNRTCtrl;
} SR_EVENT_SR_NRT_CTRL_T_SR_V1, *P_SR_EVENT_SR_NRT_CTRL_T_SR_V1;

typedef struct _SR_EVENT_SR_NRT_CTRL_T_SR_V2 {
    SR_EVENT_T rSrEvent;
    WH_SR_NRT_CTRL_T_SR_V2 rSrNRTCtrl;
} SR_EVENT_SR_NRT_CTRL_T_SR_V2, *P_SR_EVENT_SR_NRT_CTRL_T_SR_V2;

typedef struct _SR_EVENT_SR_CNT_T {
    SR_EVENT_T rSrEvent;
    SR_CNT_T rSrCnt;
} SR_EVENT_SR_CNT_T, *P_SR_EVENT_SR_CNT_T;

typedef struct _SR_EVENT_SR_SD_T {
    SR_EVENT_T rSrEvent;
    SR_SD_T rSrSd;
} SR_EVENT_SR_SD_T, *P_SR_EVENT_SR_SD_T;

typedef struct _SR_EVENT_SR_SRG_BITMAP_T {
    SR_EVENT_T rSrEvent;
    SR_SRG_BITMAP_T rSrSrgBitmap;
} SR_EVENT_SR_SRG_BITMAP_T, *P_SR_EVENT_SR_SRG_BITMAP_T;

typedef struct _SR_EVENT_SR_FNQ_CTRL_T {
    SR_EVENT_T rSrEvent;
    WH_SR_FNQ_CTRL_T rSrFNQCtrl;
} SR_EVENT_SR_FNQ_CTRL_T, *P_SR_EVENT_SR_FNQ_CTRL_T;

typedef struct _SR_EVENT_SR_FRM_FILT_T {
    SR_EVENT_T rSrEvent;
    UINT_32 u4SrFrmFilt;
} SR_EVENT_SR_FRM_FILT_T, *P_SR_EVENT_SR_FRM_FILT_T;

typedef struct _SR_EVENT_SR_INTERPS_CTRL_T {
    SR_EVENT_T rSrEvent;
    WH_SR_INTERPS_CTRL_T rSrInterPsCtrl;
} SR_EVENT_SR_INTERPS_CTRL_T, *P_SR_EVENT_SR_INTERPS_CTRL_T;

typedef struct _SR_EVENT_SR_INTERPS_DBG_T {
    SR_EVENT_T rSrEvent;
    WH_SR_INTERPS_DBG_T rSrInterPsDbg;
} SR_EVENT_SR_INTERPS_DBG_T, *P_SR_EVENT_SR_INTERPS_DBG_T;

typedef struct _SR_EVENT_SR_SIGA_T {
	SR_EVENT_T rSrEvent;
	SR_SIGA_FLAG_T rSrSigaFlag;
} SR_EVENT_SR_SIGA_T, *P_SR_EVENT_SR_SIGA_T;

struct _SR_EVENT_SR_SIGA_AUTO_T {
	SR_EVENT_T rSrEvent;
};

struct _SR_EVENT_MESH_TOPOLOGY_T {
	SR_EVENT_T rSrEvent;
	union _SR_MESH_TOPOLOGY_T rSrCmdMeshTopo;
};

struct _SR_EVENT_MESH_FH_RSSI_TH_STATUS_T {
	SR_EVENT_T rSrEvent;
	struct _SR_MESH_FH_RSSI_TH_T rSrMeshFhRssiTh;
};

struct SR_EVENT_SR_MESH_DL_STA_THRESHOLD_T {
    SR_EVENT_T rSrEvent;
    struct SR_MESH_SR_DL_STA_THRESHOLD_T rSrMeshSrDLStaThreshold;
};

struct _SR_EVENT_SR_UL_TRAFFIC_STATUS_T {
	SR_EVENT_T rSrEvent;
	struct _SR_UL_STATUS_T rSrUlStatus;
};

struct _SR_EVENT_SR_MESH_PHASE_T {
	SR_EVENT_T rSrEvent;
};

struct _SR_EVENT_SR_REMOTE_AP_STA_ALL_HE_T {
	SR_EVENT_T rSrEvent;
};

/** End SR Event */
/** End FW & DRV sync with sr_cmd.c **/

/*******************************************************************************
 *    PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/
/* For Command*/
NDIS_STATUS SrCmd(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_T prSrcmd);
NDIS_STATUS SrCmdSRUpdateCap(IN PRTMP_ADAPTER pAd, IN VOID * _prSrCmdSrCap);
NDIS_STATUS SrCmdSRUpdatePara(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_PARA_T prSrCmdSrPara);
NDIS_STATUS SrCmdSRUpdateGloVarSingleDropTa(IN PRTMP_ADAPTER pAd,
					    IN P_SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T
					    prSrCmdSrGlobalVarSingleDropTa, IN UINT_32 u4DropTaIdx,
					    IN UINT_32 u4StaIdx);
NDIS_STATUS SrCmdSRUpdateCond(IN PRTMP_ADAPTER pAd, IN VOID * _prSrCmdSrCond);
NDIS_STATUS SrCmdSRUpdateRcpiTbl(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_RCPITBL_T prSrCmdSrRcpiTbl);
NDIS_STATUS SrCmdSRUpdateRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID * _prSrCmdSrRcpiTblOfst);
NDIS_STATUS SrCmdSRUpdateQCtrl(IN PRTMP_ADAPTER pAd, IN VOID * _prSrCmdSrQCtrl);
NDIS_STATUS SrCmdSRUpdateIBPD(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_IBPD_T prSrCmdSrIBPD);
NDIS_STATUS SrCmdSRUpdateNRT(IN PRTMP_ADAPTER pAd, IN VOID * _prSrCmdSrNRT);
NDIS_STATUS SrCmdSRUpdateNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID * _prSrCmdSrNRTCtrl);
NDIS_STATUS SrCmdSRUpdateFNQCtrl(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_FNQ_CTRL_T prSrCmdSrFNQCtrl);
NDIS_STATUS SrCmdSRUpdateFrmFilt(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_FRM_FILT_T prSrCmdSrFrmFilt);
NDIS_STATUS SrCmdSRUpdateInterPsCtrl(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_INTERPS_CTRL_T prSrCmdSrInterPsCtrl);
NDIS_STATUS SrCmdSRUpdateSrgBitmap(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_SRG_BITMAP_T prSrCmdSrSrgBitmap);
NDIS_STATUS SrCmdSRUpdateSiga(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_SIGA_FLAG_T rSrCmdSrSigaFlag);
NDIS_STATUS SrCmdSRUpdateSigaAuto(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SR_SIGA_AUTO_FLAG_T *rSrCmdSrSigaAutoFlag);
NDIS_STATUS SrCmdMeshTopologyUpd(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_MESH_TOPOLOGY_T *prSrCmdMeshTopo);
NDIS_STATUS SrSetRemoteFHRssi(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, INT_8 Rssi, UINT_8 u1RemoteFhStat);
NDIS_STATUS SrSetRemoteAssocBHInfo(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, UINT_8 BhType, UINT_16 Wcid);
NDIS_STATUS SrSetMAPTopo(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, UINT_8 MapDevCount, UINT_8 MapDevSrSupportMode, UINT_8 SelfRole);
NDIS_STATUS SrCmdMeshUplinkStatusSet(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SR_UL_TRAFFIC_STATUS_T *prSrCmdSrUlStatus);
NDIS_STATUS SrCmdMeshMapBalanceSet(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SET_MAP_BALANCE_T *prSrCmdMapBalance);
NDIS_STATUS SrCmdMeshUlModeSet(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SET_MESH_UL_MODE_T *prSrCmdMeshUlMode);
NDIS_STATUS SrCmdSetFhRssiTh(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_MESH_FH_RSSI_TH_T *prSrCmdSrMeshFhRssiTh);
NDIS_STATUS SrCmdSetMeshSRLinkStaThreshold(IN PRTMP_ADAPTER pAd, IN UINT_8 u1BandIdx, IN UINT_8 u1Bssidx, IN INT_8 i1Rssi);
NDIS_STATUS SrCmdSetMeshMultiApBhDlSrTh(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, INT_8 Rssi);
NDIS_STATUS SrCmdSetMeshMultiApFhDlSrTh(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, INT_8 Rssi);
NDIS_STATUS SrCmdSetMeshForbidSrBssid(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, UINT_8 u1Bssid);
NDIS_STATUS SrCmdFlushMeshForbidSrBssid(IN PRTMP_ADAPTER pAd, IN UINT_8 u1DbdcIdx);
NDIS_STATUS SrCmdShow(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg, IN UINT_8 u1CmdSubId, IN UINT_8 u1ArgNum);
/* For Check value */
NDIS_STATUS IsFlag(IN INT32 u4ArgVal, IN UINT_8 u1ArgNum);
NDIS_STATUS IsInRange(IN INT32 u4ArgVal, IN UINT_8 u1ArgNum, IN INT32 u4Valfrom, IN INT32 u4Valto);
/* For Print content */
VOID PrintSrCmd(IN P_SR_CMD_T prSrCmd);
VOID PrintSrCap(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCap);
VOID PrintSrPara(IN P_WH_SR_PARA_T prSrPara);
VOID PrintSrInd(IN P_WH_SR_IND_T prSrInd);
VOID PrintSrGloVarSingleDropTa(IN P_SR_GLOBAL_VAR_SINGLE_DROP_TA_T prSrGlobalVarSingleDropTa,
			       IN UINT_8 u1DropTaIdx, IN UINT_8 u1StaIdx);
VOID PrintSrDropTaInfo(IN P_SR_DROP_TA_INFO_T prSrDropTaInfo, IN UINT_8 u1DropTaIdx,
		       IN UINT_8 u1StaIdx);
VOID PrintSrStaInfo(IN P_SR_STA_INFO_T prSrStaInfo, IN UINT_8 u1StaIdx);
VOID PrintSrCond(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCond);
VOID PrintSrRcpiTbl(IN P_WH_SR_RCPITBL_T prSrRcpiTbl);
VOID PrintSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID *_prSrRcpiTblOfst);
VOID PrintSrQCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrQCtrl);
VOID PrintSrIBPD(IN P_WH_SR_IBPD_T prSrIBPD);
VOID PrintSrNRT(IN PRTMP_ADAPTER pAd, IN VOID *_prSrNRT);
VOID PrintSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrNRTCtrl);
VOID PrintSrFNQCtrl(IN P_WH_SR_FNQ_CTRL_T prSrFNQCtrl);
VOID PrintSrFrmFilt(IN UINT_32 *pu4SrFrmFilt);
VOID PrintSrInterPsCtrl(IN P_WH_SR_INTERPS_CTRL_T prSrInterPsCtrl);
VOID PrintSrInterPsDbg(IN P_WH_SR_INTERPS_DBG_T prSrInterPsDbg);
VOID PrintSrSrgBitmap(IN UINT_8 u1DbdcIdx, IN P_SR_SRG_BITMAP_T prSrSrgBitmap);
VOID PrintSrDlMeshRssi(IN UINT_8 EventSubId, IN UINT_8 u1DbdcIdx, IN INT_8 i1Rssi);
VOID PrintSrSiga(IN PRTMP_ADAPTER pAD, IN UINT_8 u1DbdcIdx, IN P_SR_SIGA_FLAG_T prSrSigaflag, IN BOOLEAN fgread);
VOID PrintSrMeshTopo(IN UINT_8 u1SubId, IN union _SR_MESH_TOPOLOGY_T *prSrMeshTopo);
VOID PrintSrMeshFHRssiTh(IN UINT_8 u1SubId, IN struct _SR_MESH_FH_RSSI_TH_T  *prSrMeshFHRssi);
VOID PrintSrCnt(IN UINT8 u1DbdcIdx, IN P_SR_CNT_T prSrCnt, IN UINT8 u1PpduType);
VOID PrintSrSd(IN UINT8 u1DbdcIdx, IN P_SR_SD_T prSrSd);

VOID PrintSrCmdSrCap(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrCap);
VOID PrintSrCmdSrPara(IN P_SR_CMD_SR_PARA_T prSrCmdSrPara);
VOID PrintSrCmdSrGloVarSingleDropTa(IN P_SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T
				    prSrCmdSrGlobalVarSingleDropTa, IN UINT_8 u1DropTaIdx,
				    IN UINT_8 u1StaIdx);
VOID PrintSrCmdSrCond(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrCond);
VOID PrintSrCmdSrRcpiTbl(IN P_SR_CMD_SR_RCPITBL_T prSrCmdSrRcpiTbl);
VOID PrintSrCmdSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrRcpiTblOfst);
VOID PrintSrCmdSrQCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrQCtrl);
VOID PrintSrCmdSrIBPD(IN P_SR_CMD_SR_IBPD_T prSrCmdSrIBPD);
VOID PrintSrCmdSrNRT(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrNRT);
VOID PrintSrCmdSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrNRTCtrl);
VOID PrintSrCmdSrFNQCtrl(IN P_SR_CMD_SR_FNQ_CTRL_T prSrCmdSrFNQCtrl);
VOID PrintSrCmdSrFrmFilt(IN P_SR_CMD_SR_FRM_FILT_T prSrCmdSrFrmFilt);
VOID PrintSrCmdSrInterPsCtrl(IN P_SR_CMD_SR_INTERPS_CTRL_T prSrCmdSrInterPsCtrl);
VOID PrintSrCmdSrSrgBitmap(IN P_SR_CMD_SR_SRG_BITMAP_T prSrCmdSrSrgBitmap);
VOID PrintSrCmdSrSiga(IN PRTMP_ADAPTER pAD, IN P_SR_CMD_SR_SIGA_FLAG_T prSrCmdSrSigaFlag);
VOID PrintSrCmdSrSigaAuto(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SR_SIGA_AUTO_FLAG_T *prSrCmdSrSigaAutoFlag);
VOID PrintSrCmdMeshTopo(IN struct _SR_CMD_MESH_TOPOLOGY_T *prSrCmdMeshTopo);
VOID PrintSrCmdSrUlStatus(IN PRTMP_ADAPTER pAD, IN struct _SR_CMD_SR_UL_TRAFFIC_STATUS_T *prSrCmdSrUlStatus);
VOID PrintSrCmdSrMapBalance(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SET_MAP_BALANCE_T *prSrCmdMapBalance);
VOID PrintSrCmdSrMeshUlMode(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SET_MESH_UL_MODE_T *prSrCmdMeshUlMode);
VOID PrintSrEvent(IN P_SR_EVENT_T prSrEvent);
VOID PrintSrEventSrCap(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrCap);
VOID PrintSrEventSrPara(IN P_SR_EVENT_SR_PARA_T prSrEventSrPara);
VOID PrintSrEventSrInd(IN P_SR_EVENT_SR_IND_T prSrEventSrInd);
VOID PrintSrEventSrGloVarSingleDropTa(IN P_SR_EVENT_SR_GLOBAL_VAR_SINGLE_DROP_TA_T
				      prSrEventSrGlobalVarSingleDropT);
VOID PrintSrEventSrCond(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrCond);
VOID PrintSrEventSrRcpiTbl(IN P_SR_EVENT_SR_RCPITBL_T prSrEventSrRcpiTbl);
VOID PrintSrEventSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrRcpiTblOfst);
VOID PrintSrEventSrQCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrQCtrl);
VOID PrintSrEventSrIBPD(IN P_SR_EVENT_SR_IBPD_T prSrEventSrIBPD);
VOID PrintSrEventSrNRT(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrNRT);
VOID PrintSrEventSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrNRTCtrl);
VOID PrintSrEventSrFNQCtrl(IN P_SR_EVENT_SR_FNQ_CTRL_T prSrEventSrFNQCtrl);
VOID PrintSrEventSrFrmFilt(IN P_SR_EVENT_SR_FRM_FILT_T prSrEventSrFrmFilt);
VOID PrintSrEventSrInterPsCtrl(IN P_SR_EVENT_SR_INTERPS_CTRL_T prSrEventSrInterPsCtrl);
VOID PrintSrEventSrInterPsDbg(IN P_SR_EVENT_SR_INTERPS_DBG_T prSrEventSrInterPsDbg);
VOID PrintSrEventSrCnt(IN P_SR_EVENT_SR_CNT_T prSrEventSrCnt);
VOID PrintSrEventSrSd(IN P_SR_EVENT_SR_SD_T prSrEventSrSd);
VOID PrintSrEventSrSrgBitmap(IN P_SR_EVENT_SR_SRG_BITMAP_T prSrEventSrSrgBitmap);
VOID PrintSrEventMeshStaRssi(IN PRTMP_ADAPTER pAd, IN struct SR_EVENT_SR_MESH_DL_STA_THRESHOLD_T *prSrEventMeshStaRssiTh);
VOID PrintSrEventSrDlMeshRssi(IN P_SR_EVENT_T prSrEvent);
VOID PrintSrEventBhForbidBitMap(IN P_SR_EVENT_T prSrEvent);
VOID PrintSrEventSrSiga(IN PRTMP_ADAPTER pAD, IN P_SR_EVENT_SR_SIGA_T prSrEventSrSigaFlag);
VOID PrintSrEventSrSigaAuto(IN PRTMP_ADAPTER pAD, IN struct _SR_EVENT_SR_SIGA_AUTO_T *prSrEventSrSigaAutoFlag);
VOID PrintSrEventMeshTopo(IN PRTMP_ADAPTER pAD, IN struct _SR_EVENT_MESH_TOPOLOGY_T *prSrEventMeshTopology);
VOID PrintSrEventMeshFHRssi(IN PRTMP_ADAPTER pAd, IN struct _SR_EVENT_MESH_FH_RSSI_TH_STATUS_T *prSrEventMeshFHRssiTh);
VOID PrintSrEventSrUlStatus(IN PRTMP_ADAPTER pAD, IN struct _SR_EVENT_SR_UL_TRAFFIC_STATUS_T *prSrEventSrUlStatus);
VOID PrintSrEventSrMeshPhase(IN PRTMP_ADAPTER pAD, IN struct _SR_EVENT_SR_MESH_PHASE_T *prSrEventSrMeshPhase);
VOID PrintSrEventSrRemoteAPStaAllHe(IN PRTMP_ADAPTER pAD, IN struct _SR_EVENT_SR_REMOTE_AP_STA_ALL_HE_T *prSrEventSrRemoteAPStaAllHe);
VOID SrMeshSelfBssColorChangeEvent(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR band_idx);
VOID SrMeshSelfPBssidChangeEvent(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR band_idx);
VOID SrMeshSelfSrgInfoEvent(struct _RTMP_ADAPTER *pAd, UINT8 u1BandIdx);
VOID ExtEventMeshUplinkTraffic(IN PRTMP_ADAPTER pAD, IN struct _SR_EVENT_SR_UL_TRAFFIC_STATUS_T *prSrEventSrUlStatus);
VOID SrMeshSrReportSTAMode(IN PRTMP_ADAPTER pAd, UINT8 u1BandIdx, UINT8 u1StaAllHe);

UINT_8 SRRcpiConv(IN INT_8 i1Dbm);
INT_8 SRDbmConv(IN UINT_8 u1Rcpi);

/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/
struct sr_mesh_topology_update_params _rSrMeshTopologyUpdateParams[RAM_BAND_NUM] = {0};
struct _SR_MESH_SRG_BITMAP_T rSrSelfSrgBM[RAM_BAND_NUM] = {0};
struct _SR_MESH_SRG_BITMAP_T rSrSelfSrgBMMan[RAM_BAND_NUM] = {0};
UINT_8 _u1SrSelfSrgBMMode[RAM_BAND_NUM] = {0};
UINT_8 _u1SrMeshTopoLock[RAM_BAND_NUM] = {0};
UINT_8 _u1SrMeshUlMode[RAM_BAND_NUM] = {0};
UINT_8 _u1StaModeRptUnLock[RAM_BAND_NUM] = {0};

NDIS_STATUS SrRstNav(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
#ifdef MT7915_MT7916_COEXIST_COMPATIBLE
	/* as only MT7915 use this interface, so just add hook function in mt7915 */
	RTMP_CHIP_OP * chip_op = hc_get_chip_ops(pAd->hdev_ctrl);

	if (chip_op->sr_reset_nav)
		return chip_op->sr_reset_nav(pAd, (VOID *)pRxBlk);
	else
		AsicNotSupportFunc(pAd, __func__);
#else
	UINT_8 u1DbdcIdx = BAND0;
	UINT_16 u2WlanIdx = 0;
	UINT_32 u4Addr = BN0_WF_RMAC_TOP_LUNVR_RESET_ADDR, u4Val = BN0_WF_RMAC_TOP_LUNVR_RESET_MASK;
	struct rxd_grp_0 *rxd_grp0 = (struct rxd_grp_0 *)(pRxBlk->rmac_info);

	u1DbdcIdx = (rxd_grp0->rxd_1 & RXD_BN) ? BAND1 : BAND0;
	u2WlanIdx = (rxd_grp0->rxd_1 & RXD_WLAN_IDX_MASK) >> RXD_WLAN_IDX_SHIFT;

	if (IS_MT7915(pAd)) {
		if (u1DbdcIdx == BAND1)
			u4Addr += 0x10000;

		IO_W_32(u4Addr, u4Val);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Band:%d WlanIdx:%d Addr:%x Val:%x\n", u1DbdcIdx, u2WlanIdx, u4Addr, u4Val);
	}
#endif

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS SrDisSrBfrConnected(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev, IN BOOLEAN fgSrEnable)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = BAND0;
	SR_CMD_T rSrCmd;

	if (wdev != NULL) {
		u1DbdcIdx = HcGetBandByWdev(wdev);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"wdev == NULL\n");
		return NDIS_STATUS_FAILURE;
	}

	/*SR not enable via profile*/
	if (pAd->CommonCfg.SREnable[u1DbdcIdx] == FALSE) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"SREnable[%d] = %d Return SUCCESS\n", u1DbdcIdx, pAd->CommonCfg.SREnable[u1DbdcIdx]);
		return NDIS_STATUS_SUCCESS;
	}

	/*Check DisSrBfrConnected avoid periodically send cmd*/
	if (pAd->CommonCfg.DisSrBfrConnected[u1DbdcIdx] == fgSrEnable) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"DisSrBfrConnected[%d] = %d == fgSrEnable = %d Return SUCCESS\n", u1DbdcIdx, pAd->CommonCfg.DisSrBfrConnected[u1DbdcIdx], fgSrEnable);
		return NDIS_STATUS_SUCCESS;
	}

	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
	"u1DbdcIdx = %d, fgSrEnable = %d\n", u1DbdcIdx, fgSrEnable);

	if (fgSrEnable == FALSE) {

		/* Disable SRSDEnable First */
		os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));
		/* Assign Cmd Id */
		rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_SD_ENABLE;
		rSrCmd.u1DbdcIdx = u1DbdcIdx;
		rSrCmd.u4Value = FALSE;
		Status = SrCmd(pAd, &rSrCmd);

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Set SRSDEnable[%d]=%d Fail!\n", u1DbdcIdx, FALSE);
			return NDIS_STATUS_FAILURE;
		}

		/* Disable SREnable */
		os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));
		/* Assign Cmd Id */
		rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_ENABLE;
		rSrCmd.u1DbdcIdx = u1DbdcIdx;
		rSrCmd.u4Value = FALSE;
		Status = SrCmd(pAd, &rSrCmd);

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Set SREnable[%d]=%d Fail!\n", u1DbdcIdx, FALSE);
			return NDIS_STATUS_FAILURE;
		}
		/*Update DisSrBfrConnected*/
		pAd->CommonCfg.DisSrBfrConnected[u1DbdcIdx] = FALSE;
	} else {
		/*Restore Profile Setting*/
		/* Set SREnable Part */
		os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));
		/* Assign Cmd Id */
		rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_ENABLE;
		rSrCmd.u1DbdcIdx = u1DbdcIdx;
		rSrCmd.u4Value = pAd->CommonCfg.SREnable[u1DbdcIdx];
		Status = SrCmd(pAd, &rSrCmd);

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Set SREnable[%d]=%d Fail!\n",  u1DbdcIdx,
			  pAd->CommonCfg.SREnable[u1DbdcIdx]);
			return NDIS_STATUS_FAILURE;
		}
		/* End - Set SREnable Part */

		/*Update DisSrBfrConnected*/
		pAd->CommonCfg.DisSrBfrConnected[u1DbdcIdx] = pAd->CommonCfg.SREnable[u1DbdcIdx];

		/* Set SRSDEnable Part */
		os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));
		/* Assign Cmd Id */
		rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_SD_ENABLE;
		rSrCmd.u1DbdcIdx = u1DbdcIdx;
		rSrCmd.u4Value = pAd->CommonCfg.SRSDEnable[u1DbdcIdx];
		Status = SrCmd(pAd, &rSrCmd);

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Set SRSDEnable[%d]=%d Fail!\n", u1DbdcIdx,
			  pAd->CommonCfg.SRSDEnable[u1DbdcIdx]);
			return NDIS_STATUS_FAILURE;
		}
		/* End - Set SRSDEnable Part */
	}
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS SrProfileSREnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *buffer)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT8 u1BandIdx, u1BandNum;
	RTMP_STRING *ptr;

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode)
		u1BandNum = 2;
	else
		u1BandNum = 1;
#else
	u1BandNum = 1;
#endif /* DBDC_MODE */

	for (u1BandIdx = BAND0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), u1BandIdx++) {
		if (u1BandIdx >= u1BandNum)
			return NDIS_STATUS_INVALID_DATA;

		pAd->CommonCfg.SREnable[u1BandIdx] = simple_strtol(ptr, 0, 10);

	}
	return Status;
}

NDIS_STATUS SrProfileSRMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *buffer)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT8 u1BandIdx, u1BandNum;
	RTMP_STRING *ptr;

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode)
		u1BandNum = 2;
	else
		u1BandNum = 1;
#else
	u1BandNum = 1;
#endif /* DBDC_MODE */

	for (u1BandIdx = BAND0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), u1BandIdx++) {
		if (u1BandIdx >= u1BandNum)
			return NDIS_STATUS_INVALID_DATA;

		pAd->CommonCfg.SRMode[u1BandIdx] = simple_strtol(ptr, 0, 10);

	}
	return Status;
}


NDIS_STATUS SrProfileSRSDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *buffer)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT8 u1BandIdx, u1BandNum;
	RTMP_STRING *ptr;

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode)
		u1BandNum = 2;
	else
		u1BandNum = 1;
#else
	u1BandNum = 1;
#endif /* DBDC_MODE */

	for (u1BandIdx = BAND0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), u1BandIdx++) {
		if (u1BandIdx >= u1BandNum)
			return NDIS_STATUS_INVALID_DATA;

		pAd->CommonCfg.SRSDEnable[u1BandIdx] = simple_strtol(ptr, 0, 10);

	}
	return Status;
}

NDIS_STATUS SrProfileSRDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *buffer)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT8 u1BandIdx, u1BandNum;
	RTMP_STRING *ptr;

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode)
		u1BandNum = 2;
	else
		u1BandNum = 1;
#else
	u1BandNum = 1;
#endif /* DBDC_MODE */

	for (u1BandIdx = BAND0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), u1BandIdx++) {
		if (u1BandIdx >= u1BandNum)
			return NDIS_STATUS_INVALID_DATA;

		pAd->CommonCfg.SRDPDEnable[u1BandIdx] = simple_strtol(ptr, 0, 10);

	}
	return Status;
}

NDIS_STATUS SrProfileSRDPDThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT8 u1BandIdx, u1BandNum;
	RTMP_STRING *ptr;
	ULONG u4threshold = 0;

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode)
		u1BandNum = 2;
	else
		u1BandNum = 1;
#else
	u1BandNum = 1;
#endif /* DBDC_MODE */

	for (u1BandIdx = BAND0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), u1BandIdx++) {
		if (u1BandIdx >= u1BandNum)
			return NDIS_STATUS_INVALID_DATA;

		if (kstrtol(ptr, 10, &u4threshold) != 0)
			return NDIS_STATUS_INVALID_DATA;

		 pAd->CommonCfg.SRDPDThreshold[u1BandIdx] = (UINT_8)u4threshold;
	}
	return Status;
}

NDIS_STATUS SrProfileSRDropMinMCS(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT8 u1BandIdx, u1BandNum;
	RTMP_STRING *ptr;
	ULONG u4mcs = 0;

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode)
		u1BandNum = 2;
	else
		u1BandNum = 1;
#else
	u1BandNum = 1;
#endif /* DBDC_MODE */

	for (u1BandIdx = BAND0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), u1BandIdx++) {
		if (u1BandIdx >= u1BandNum)
			return NDIS_STATUS_INVALID_DATA;

		if (kstrtol(ptr, 10, &u4mcs) != 0)
			return NDIS_STATUS_INVALID_DATA;

		pAd->CommonCfg.SRDropMinMcs[u1BandIdx] = (UINT_8)u4mcs;

	}
	return Status;
}

NDIS_STATUS SrMbssInit(IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = BAND0;
	SR_CMD_T rSrCmd;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	/* ap.c will call this command enable SR by profile */
	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		 "%s: Spatial Reuse initialize via profile.\n",
		  __func__);

	/*Init DisSrBfrConnected */
	pAd->CommonCfg.DisSrBfrConnected[u1DbdcIdx] = pAd->CommonCfg.SREnable[u1DbdcIdx];

	/* Set SREnable Part */
	os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));
	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_ENABLE;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = pAd->CommonCfg.SREnable[u1DbdcIdx];
	Status = SrCmd(pAd, &rSrCmd);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 "Set SREnable[%d]=%d Fail!\n",  u1DbdcIdx,
		  pAd->CommonCfg.SREnable[u1DbdcIdx]);
	}
	/* End - Set SREnable Part */

	/* Set SRMode Part */
	os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));
	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_MODE;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = pAd->CommonCfg.SRMode[u1DbdcIdx];
	Status = SrCmd(pAd, &rSrCmd);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 "Set SRMode[%d]=%d Fail!\n", u1DbdcIdx,
		  pAd->CommonCfg.SRMode[u1DbdcIdx]);
	}
	/* End - Set SRMode Part */

	/* Set SRSDEnable Part */
	os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));
	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_SD_ENABLE;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = pAd->CommonCfg.SRSDEnable[u1DbdcIdx];
	Status = SrCmd(pAd, &rSrCmd);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 "Set SRSDEnable[%d]=%d Fail!\n", u1DbdcIdx,
		  pAd->CommonCfg.SRSDEnable[u1DbdcIdx]);
	}
	/* End - Set SRSDEnable Part */

	/* Set SRDPDEnable Part */
	os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));
	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_DPD_ENABLE;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = pAd->CommonCfg.SRDPDEnable[u1DbdcIdx];
	Status = SrCmd(pAd, &rSrCmd);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 "Set SRDPDEnable[%d]=%d Fail!\n", u1DbdcIdx,
		  pAd->CommonCfg.SRDPDEnable[u1DbdcIdx]);
	}
	/* End - Set SRDPDEnable Part */

	/* Set SRDropMinMcs Part */
	os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));
	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_DROP_MIN_MCS;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = pAd->CommonCfg.SRDropMinMcs[u1DbdcIdx];
	Status = SrCmd(pAd, &rSrCmd);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 "Set SRDropMinMcs[%d]=%d Fail!\n", u1DbdcIdx,
		  pAd->CommonCfg.SRDPDEnable[u1DbdcIdx]);
	}
	/* End - Set SRDropMinMcs Part */

	/* Set SRDPDThreshold Part */
	os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));
	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_DPD_THRESHOLD;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = pAd->CommonCfg.SRDPDThreshold[u1DbdcIdx];
	Status = SrCmd(pAd, &rSrCmd);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 "Set SRDPDThreshold[%d]=%d Fail!\n", u1DbdcIdx,
		  pAd->CommonCfg.SRDPDEnable[u1DbdcIdx]);
	}
	/* End - Set SRDPDThreshold Part */
	/* Init mac for test */
	COPY_MAC_ADDR(g_rTopologyUpdate.map_remote_bh_mac, pAd->CurrentAddress); /* from AP1 */
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS SrMeshGetSrgBitmap(IN PRTMP_ADAPTER pAd, IN UINT8 band_idx, IN PUINT_8 pMeshSrgBitmap)
{
	struct _SR_MESH_SRG_BITMAP_T *prSrgBitmap;

	if (band_idx >= RAM_BAND_NUM)
		return NDIS_STATUS_FAILURE;

	prSrgBitmap = &rSrSelfSrgBM[band_idx];

	os_move_mem(pMeshSrgBitmap, prSrgBitmap, sizeof(*prSrgBitmap));

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"Color:[63_32][%x]-[31_0][%x] Bssid:[63_32][%x]-[31_0][%x]\n",
		prSrgBitmap->u4Color_63_32, prSrgBitmap->u4Color_31_0,
		prSrgBitmap->u4pBssid_63_32, prSrgBitmap->u4pBssid_31_0);

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS SrMeshGetSrMode(IN PRTMP_ADAPTER pAd, IN UINT8 band_idx, IN PUINT_8 pu1SrMode)
{
	if (band_idx >= RAM_BAND_NUM)
		return NDIS_STATUS_FAILURE;

	*pu1SrMode = pAd->CommonCfg.SRMode[band_idx];

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "SrMode:%u\n", *pu1SrMode);

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS SRMeshLinkSTAThreshold(IN PRTMP_ADAPTER pAd, IN UINT_8 u1WdevIdx, INT_8 i1Rssi)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = BAND0, u1Bssid = 0;
	struct wifi_dev *wdev = NULL;

	wdev = pAd->wdev_list[u1WdevIdx];

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Wdev is Null, WdevIdx  = %d\n", u1WdevIdx);
		return FALSE;
	}

	u1DbdcIdx = HcGetBandByWdev(wdev);
	u1Bssid = (UINT_8)(wdev->DevInfo.OwnMacIdx > SR_MESH_P2PGC ? wdev->DevInfo.OwnMacIdx - SR_BSSID_OMAC_OFFSET : wdev->DevInfo.OwnMacIdx);

	MTWF_PRINT("BandIdx = %d, Bssid = %d, Rssi = %d\n", u1DbdcIdx, u1Bssid,  i1Rssi);

	Status = SrCmdSetMeshSRLinkStaThreshold(pAd, u1DbdcIdx, u1Bssid, i1Rssi);
	return Status;
}

NDIS_STATUS SrBHDownMeshSRThreshold(IN PRTMP_ADAPTER pAd, IN INT_8 i1Rssi)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = BAND0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	MTWF_PRINT("set Rssi = %d\n", i1Rssi);

	Status = SrCmdSetMeshMultiApBhDlSrTh(pAd, u1DbdcIdx, i1Rssi);
	return Status;
}

NDIS_STATUS SrFHDownMeshSRThreshold(IN PRTMP_ADAPTER pAd, IN INT_8 i1Rssi)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1DbdcIdx = BAND0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	MTWF_PRINT("set Rssi = %d\n", i1Rssi);

	Status = SrCmdSetMeshMultiApFhDlSrTh(pAd, u1DbdcIdx, i1Rssi);
	return Status;
}

NDIS_STATUS SrMultiAPBhMeshSrgBitmap(IN PRTMP_ADAPTER pAd, IN BOOLEAN fgSet, IN PUINT_8 prSrBhMeshSrgBitmap)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	P_SR_MESH_SRG_BITMAP_T prSrMeshSrgBitmap = (P_SR_MESH_SRG_BITMAP_T)prSrBhMeshSrgBitmap;

	if (wdev != NULL && prSrMeshSrgBitmap != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (fgSet) {
		SR_CMD_SR_SRG_BITMAP_T rSrCmdSrMeshSrgBitmap;

		os_zero_mem(&rSrCmdSrMeshSrgBitmap, sizeof(SR_CMD_SR_SRG_BITMAP_T));

		/* Assign Cmd Id */
		rSrCmdSrMeshSrgBitmap.rSrCmd.u1CmdSubId = SR_CMD_SET_BH_MESH_SR_BITMAP;
		rSrCmdSrMeshSrgBitmap.rSrCmd.u1DbdcIdx = u1DbdcIdx;
		/* Update Color & pBssid Bitmap */
		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4Color_31_0[u1DbdcIdx] = cpu2le32(prSrMeshSrgBitmap->u4Color_31_0);
		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4Color_63_32[u1DbdcIdx] = cpu2le32(prSrMeshSrgBitmap->u4Color_63_32);
		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4pBssid_31_0[u1DbdcIdx] = cpu2le32(prSrMeshSrgBitmap->u4pBssid_31_0);
		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4pBssid_63_32[u1DbdcIdx] = cpu2le32(prSrMeshSrgBitmap->u4pBssid_63_32);

		Status = SrCmdSRUpdateSrgBitmap(pAd, &rSrCmdSrMeshSrgBitmap);

		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: Update SR Mesh SRG Bitmap from Mesh System.\n", __func__);
		MTWF_PRINT("BandIdx:%u, BSS_Color_BM:[63:32][0x%x]-[31:0][0x%x], Par_Bssid_BM:[63:32][0x%x]-[31:0][0x%x]\n",
					u1DbdcIdx, prSrMeshSrgBitmap->u4Color_63_32, prSrMeshSrgBitmap->u4Color_31_0, prSrMeshSrgBitmap->u4pBssid_63_32, prSrMeshSrgBitmap->u4pBssid_31_0);
	}

	return Status;
}

NDIS_STATUS SrMultiAPFhMeshSrgBitmap(IN PRTMP_ADAPTER pAd, IN BOOLEAN fgSet, IN PUINT_8 prSrFhMeshSrgBitmap)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	P_SR_MESH_SRG_BITMAP_T prSrMeshSrgBitmap = (P_SR_MESH_SRG_BITMAP_T)prSrFhMeshSrgBitmap;

	if (wdev != NULL && prSrMeshSrgBitmap != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (fgSet) {
		SR_CMD_SR_SRG_BITMAP_T rSrCmdSrMeshSrgBitmap;

		os_zero_mem(&rSrCmdSrMeshSrgBitmap, sizeof(SR_CMD_SR_SRG_BITMAP_T));

		/* Assign Cmd Id */
		rSrCmdSrMeshSrgBitmap.rSrCmd.u1CmdSubId = SR_CMD_SET_FH_MESH_SR_BITMAP;
		rSrCmdSrMeshSrgBitmap.rSrCmd.u1DbdcIdx = u1DbdcIdx;
		/* Update Color & pBssid Bitmap */
		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4Color_31_0[u1DbdcIdx] = cpu2le32(prSrMeshSrgBitmap->u4Color_31_0);
		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4Color_63_32[u1DbdcIdx] = cpu2le32(prSrMeshSrgBitmap->u4Color_63_32);
		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4pBssid_31_0[u1DbdcIdx] = cpu2le32(prSrMeshSrgBitmap->u4pBssid_31_0);
		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4pBssid_63_32[u1DbdcIdx] = cpu2le32(prSrMeshSrgBitmap->u4pBssid_63_32);

		Status = SrCmdSRUpdateSrgBitmap(pAd, &rSrCmdSrMeshSrgBitmap);

		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: Update SR Mesh SRG Bitmap from Mesh System.\n", __func__);
		MTWF_PRINT("BandIdx:%u, BSS_Color_BM:[63:32][0x%x]-[31:0][0x%x], Par_Bssid_BM:[63:32][0x%x]-[31:0][0x%x]\n",
					u1DbdcIdx, prSrMeshSrgBitmap->u4Color_63_32, prSrMeshSrgBitmap->u4Color_31_0, prSrMeshSrgBitmap->u4pBssid_63_32, prSrMeshSrgBitmap->u4pBssid_31_0);
	}

	return Status;
}

NDIS_STATUS SrMeshForbidSrBssid(IN PRTMP_ADAPTER pAd, IN UINT_8 u1WdevIdx)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	struct wifi_dev *wdev = NULL;
	UINT_8 u1DbdcIdx = BAND0, u1Bssid = 0;

	wdev = pAd->wdev_list[u1WdevIdx];

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Wdev is Null, WdevIdx  = %d\n", u1WdevIdx);
		return NDIS_STATUS_FAILURE;
	}

	u1DbdcIdx = HcGetBandByWdev(wdev);
	u1Bssid = (UINT_8)wdev->DevInfo.OwnMacIdx > SR_MESH_P2PGC ? wdev->DevInfo.OwnMacIdx - SR_BSSID_OMAC_OFFSET : wdev->DevInfo.OwnMacIdx;

	MTWF_PRINT("WdevIdx = %d, BandIdx = %d, u1Bssid = %d\n", u1WdevIdx, u1DbdcIdx, u1Bssid);
	Status = SrCmdSetMeshForbidSrBssid(pAd, u1DbdcIdx, u1Bssid);

	return Status;
}

NDIS_STATUS SrMeshSrgBitMapControl(IN PRTMP_ADAPTER pAd, IN BOOLEAN fgSet, IN PUINT_8 pMeshSrgBitmap)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	P_SR_MESH_SRG_BITMAP_T prSrMeshSrgBitmap = (P_SR_MESH_SRG_BITMAP_T)pMeshSrgBitmap;

	if (wdev != NULL && prSrMeshSrgBitmap != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (fgSet) {
		SR_CMD_SR_SRG_BITMAP_T rSrCmdSrMeshSrgBitmap;

		os_zero_mem(&rSrCmdSrMeshSrgBitmap, sizeof(SR_CMD_SR_SRG_BITMAP_T));

		/* Assign Cmd Id */
		rSrCmdSrMeshSrgBitmap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_MESH_SRG_BITMAP;
		rSrCmdSrMeshSrgBitmap.rSrCmd.u1DbdcIdx = u1DbdcIdx;
		/* Update Color & pBssid Bitmap */
		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4Color_31_0[u1DbdcIdx] = prSrMeshSrgBitmap->u4Color_31_0;
		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4Color_63_32[u1DbdcIdx] = prSrMeshSrgBitmap->u4Color_63_32;
		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4pBssid_31_0[u1DbdcIdx] = prSrMeshSrgBitmap->u4pBssid_31_0;
		rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4pBssid_63_32[u1DbdcIdx] = prSrMeshSrgBitmap->u4pBssid_63_32;

		Status = SrCmdSRUpdateSrgBitmap(pAd, &rSrCmdSrMeshSrgBitmap);

		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: Update SR Mesh SRG Bitmap from Mesh System.\n", __func__);
	} else {
		SR_CMD_T rSrCmd;

		os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));

		/* Assign Cmd Id */
		rSrCmd.u1CmdSubId = SR_CMD_GET_SR_MESH_SRG_BITMAP;
		rSrCmd.u1DbdcIdx = u1DbdcIdx;

		Status = SrCmd(pAd, &rSrCmd);

		/* Please put your receiving function in the case SR_EVENT_GET_SR_MESH_SRG_BITMAP in EventSrHandler */

		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: Get SR Mesh SRG Bitmap from FW.\n", __func__);
	}

	return Status;
}

NDIS_STATUS SrFindBHWcid(PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, PUINT_8 pBHMac)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	UINT_8 BhType = ENUM_BH_TYPE_NO_WIFI;
	PMAC_TABLE_ENTRY pEntry = NULL;
	UINT_16 wcid = 0;
	struct wifi_dev *wdev = NULL;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"BH MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", PRINT_MAC(pBHMac));

	if (pBHMac) {
		if (!MAC_ADDR_EQUAL(pBHMac, BROADCAST_ADDR) && !MAC_ADDR_EQUAL(pBHMac, ZERO_MAC_ADDR)) {
			pEntry = MacTableLookup(pAd, pBHMac);

			if (pEntry) {
				wdev = pEntry->wdev;
				if (wdev) {
					if (HcGetBandByWdev(wdev) == u1DbdcIdx) {
						wcid = pEntry->wcid;
						BhType = ENUM_BH_TYPE_WIFI;
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
							"Band %d STA found with wcid:%u\n", u1DbdcIdx, wcid);
					}
				}
			}
		}
	}

	Status = SrSetRemoteAssocBHInfo(pAd, u1DbdcIdx, BhType, wcid);

	return Status;
}

NDIS_STATUS SrMeshTopologyUpdate(IN PRTMP_ADAPTER pAd, IN PUINT_8 pTopologyUpdate, IN UINT8 Band_Idx)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	struct sr_mesh_topology_params *prSrMeshTopologyParams = (struct sr_mesh_topology_params *)pTopologyUpdate;
	struct sr_mesh_topology_update_params *prParams;

	if ((pAd->bMapR3Enable == 0) || (pAd->CommonCfg.SRMode[Band_Idx] == 0)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"MapR3Enable:%u SRMode:%u Band:%u\n",
			pAd->bMapR3Enable, pAd->CommonCfg.SRMode[Band_Idx], Band_Idx);
		return NDIS_STATUS_FAILURE;
	}

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"MapDevCnt:%u MapDevSrSupportMode:%u SelfRole:%u\n",
		prSrMeshTopologyParams->map_dev_count,
		prSrMeshTopologyParams->map_dev_sr_support_mode,
		prSrMeshTopologyParams->self_role);

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"FH BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",
		PRINT_MAC(prSrMeshTopologyParams->map_remote_fh_bssid));

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"BH MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
		PRINT_MAC(prSrMeshTopologyParams->map_remote_bh_mac));

	if (_u1SrMeshTopoLock[Band_Idx] == ENUM_SR_TOPO_LOCK_MANUAL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"Topolock:%u Manual for band:%u\n",
			_u1SrMeshTopoLock[Band_Idx], Band_Idx);
		return NDIS_STATUS_FAILURE;
	}

	prParams = &_rSrMeshTopologyUpdateParams[Band_Idx];

	NdisCopyMemory(&prParams->topo_params, prSrMeshTopologyParams, sizeof(*prSrMeshTopologyParams));
	prParams->scan_start = TRUE;
	prParams->skip_scan = FALSE;
	prParams->wdev = wdev;
	prParams->scan_fail = TRUE;
	prParams->scan_rssi = -127;

	if (MAC_ADDR_EQUAL(prSrMeshTopologyParams->map_remote_fh_bssid, BROADCAST_ADDR)
		|| MAC_ADDR_EQUAL(prSrMeshTopologyParams->map_remote_fh_bssid, ZERO_MAC_ADDR)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"No need to scan for the band:%u!\n", Band_Idx);
		prParams->skip_scan = TRUE;
		prParams->scan_fail = TRUE;
	}
	/* Mesh Topology Update Per Command from WAPP */
	Status = IsInRange(prParams->topo_params.map_dev_sr_support_mode, 0, 0, 2);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_PRINT("Band:%u MapDevSrSupportMode:%u should be [0,2]!\n",
			Band_Idx, prParams->topo_params.map_dev_sr_support_mode);
	}

	Status = SrSetMAPTopo(pAd, Band_Idx,
		prParams->topo_params.map_dev_count,
		prParams->topo_params.map_dev_sr_support_mode,
		prParams->topo_params.self_role);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"SrSetMAPTopo Failed for band:%u MapDevCnt:%u MapDevSrSupportMode:%u SelfRole:%u!\n",
			Band_Idx, prParams->topo_params.map_dev_count,
			prParams->topo_params.map_dev_sr_support_mode,
			prParams->topo_params.self_role);
	}


	Status = SrFindBHWcid(pAd, Band_Idx,
		prParams->topo_params.map_remote_bh_mac);
	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"SrFindBHWcid Failed for band:%u!\n",
			Band_Idx);
	}



#endif

		return Status;

}

VOID SrMeshTopologyUpdatePeriodic(struct _RTMP_ADAPTER *pAd)
{
	UINT_8 bandidx;

	for (bandidx = 0; bandidx < DBDC_BAND_NUM; bandidx++) {
		if (pAd->CommonCfg.SRMode[bandidx] == TRUE)
			SrMeshTopologyUpdatePerBand(pAd, bandidx);
	}
}

VOID SrMeshTopologyUpdateBcnRssi(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, MLME_QUEUE_ELEM *Elem, BCN_IE_LIST *ie_list)
{
	RSSI_SAMPLE rssi_sample;
	struct sr_mesh_topology_update_params *prParams;
	CHAR RealRssi;

	prParams = &_rSrMeshTopologyUpdateParams[HcGetBandByWdev(wdev)];

	if (TRUE == prParams->skip_scan) {
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"band %d : MAC Addr Given:%02X:%02X:%02X:%02X:%02X:%02X\nMAC Addr Bcn:%02X:%02X:%02X:%02X:%02X:%02X\n",
		HcGetBandByWdev(wdev),
		PRINT_MAC(prParams->topo_params.map_remote_fh_bssid),
		PRINT_MAC(ie_list->Addr2));

	if (MAC_ADDR_EQUAL(prParams->topo_params.map_remote_fh_bssid, ie_list->Addr2)) {
		rssi_sample.AvgRssi[0] = Elem->rssi_info.raw_rssi[0];
		rssi_sample.AvgRssi[1] = Elem->rssi_info.raw_rssi[1];
		rssi_sample.AvgRssi[2] = Elem->rssi_info.raw_rssi[2];
		rssi_sample.AvgRssi[3] = Elem->rssi_info.raw_rssi[3];
		RealRssi = rtmp_avg_rssi(pAd, &rssi_sample);
		prParams->scan_rssi = RealRssi;
		prParams->scan_fail = FALSE;
		prParams->scan_count++;

		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"band %d : MAC Addr Match: RSSI %d %02X:%02X:%02X:%02X:%02X:%02X\n",
			HcGetBandByWdev(wdev), prParams->scan_rssi, PRINT_MAC(ie_list->Addr2));
	}
}

VOID SrMeshTopologyUpdatePerBand(struct _RTMP_ADAPTER *pAd, UINT_8 u1DbdcIdx)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	struct sr_mesh_topology_update_params *prParams;
	INT_8 Rssi = -127;
	UINT_8 u1RemoteFhStat = ENUM_RMT_FH_SCAN_FAIL;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"Band:%u for scan\n", u1DbdcIdx);

	prParams = &_rSrMeshTopologyUpdateParams[u1DbdcIdx];

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"scan_fail:%u skip_scan:%u scan_count:%u\n",
		prParams->scan_fail, prParams->skip_scan, prParams->scan_count);

	if (_u1SrMeshTopoLock[u1DbdcIdx] == ENUM_SR_TOPO_LOCK_MANUAL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Topolock:%u Manual for band:%u\n",
			_u1SrMeshTopoLock[u1DbdcIdx], u1DbdcIdx);
		goto error;
	}

	if (prParams->scan_start == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Band:%u no topology update triggered yet\n", u1DbdcIdx);
		goto error;
	}

	if (prParams->skip_scan == TRUE) {
		u1RemoteFhStat = ENUM_RMT_FH_INVLD_BSSID;
		Rssi = -127;
	} else if (prParams->scan_fail == TRUE) {
		u1RemoteFhStat = ENUM_RMT_FH_SCAN_FAIL;
		Rssi = -127;
	} else {/* scan success && not skip scan */
		u1RemoteFhStat = ENUM_RMT_FH_SCAN_SUCCESS;
		Rssi = prParams->scan_rssi;
	}

	Status = SrSetRemoteFHRssi(pAd, u1DbdcIdx, Rssi, u1RemoteFhStat);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"SrSetRemoteFHRssi Failed for band:%u!\n",
			u1DbdcIdx);
		goto error;
	}
	Status = SrSetSrgBitmapRefresh(pAd, u1DbdcIdx);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"SrSetSrgBitmapRefresh Failed for band:%u!\n",
			u1DbdcIdx);
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"Update SR Mesh Topology from Mesh System.\n");

error:

	prParams->scan_count = 0;
	prParams->scan_fail = TRUE;
}

NDIS_STATUS SrSetSrgBitmapRefresh(IN PRTMP_ADAPTER pAd, IN UINT_8 u1DbdcIdx)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	SR_CMD_T rSrCmd;
	P_SR_CMD_T prSrcmd = &rSrCmd;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));

	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_SR_SRG_BITMAP_REFRESH;
	rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = 0;

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_T));
	if (!msg) {
		Status = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrcmd, sizeof(SR_CMD_T));
	Status = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(Status = %d)\n", Status);
	return Status;
}

NDIS_STATUS SetSrCapSrEn(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	INT32 u4SrEn = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (IS_SR_V1(pAd)) {
		SR_CMD_SR_CAP_T_SR_V1 rSrCmdSrCap;

		os_zero_mem(&rSrCmdSrCap, sizeof(SR_CMD_SR_CAP_T_SR_V1));

		/* Assign Cmd Id */
		rSrCmdSrCap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CAP_SREN_CTRL;
		rSrCmdSrCap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_CAP_SREN_CTRL_ARG_NUM;
		rSrCmdSrCap.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrEn, u1ArgNum);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrCap.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrCap.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrCap.rSrCap.fgSrEn = u4SrEn;
					Status = SrCmdSRUpdateCap(pAd, &rSrCmdSrCap);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;

		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srcapsren=[SrEn]\n");
		}
	} else if (IS_SR_V2(pAd)) {
		SR_CMD_SR_CAP_T_SR_V2 rSrCmdSrCap;

		os_zero_mem(&rSrCmdSrCap, sizeof(SR_CMD_SR_CAP_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrCap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CAP_SREN_CTRL;
		rSrCmdSrCap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_CAP_SREN_CTRL_ARG_NUM;
		rSrCmdSrCap.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrEn, u1ArgNum);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrCap.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrCap.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrCap.rSrCap.fgSrEn = u4SrEn;
					Status = SrCmdSRUpdateCap(pAd, &rSrCmdSrCap);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;

		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srcapsren=[SrEn]\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}

	return Status;
}

NDIS_STATUS SetSrCapAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4SrEn = 0, u4SrgEn = 0, u4NonSrgEn = 0;
	UINT_32 u4SingleMdpuRtsctsEn = 0, u4HdrDurEn = 0, u4TxopDurEn = 0;
	UINT_32 u4NonSrgInterPpduPresv = 0, u4SrgInterPpduPresv = 0, u4SMpduNoTrigEn = 0;
	UINT_32 u4SrgBssidOrder = 0, u4CtsAfterRts = 0, u4SrpOldRxvEn = 0;
	UINT_32 u4SrpNewRxvEn = 0, u4SrpDataOnlyEn = 0, u4FixedRateSrREn = 0;
	UINT_32 u4WtblSrREn = 0, u4SrRemTimeEn = 0, u4ProtInSrWinDis = 0;
	UINT_32  u4TxCmdDlRateSelEn = 0, u4AmpduTxCntEn = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (IS_SR_V1(pAd)) {
		SR_CMD_SR_CAP_T_SR_V1 rSrCmdSrCap;

		os_zero_mem(&rSrCmdSrCap, sizeof(SR_CMD_SR_CAP_T_SR_V1));

		/* Assign Cmd Id */
		rSrCmdSrCap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CAP_ALL_CTRL;
		rSrCmdSrCap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_CAP_ALL_CTRL_ARG_NUM_SR_V1;
		rSrCmdSrCap.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrEn, u1ArgNum);
						break;
					case 1:
						u4SrgEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrgEn, u1ArgNum);
						break;
					case 2:
						u4NonSrgEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4NonSrgEn, u1ArgNum);
						break;
					case 3:
						u4SingleMdpuRtsctsEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SingleMdpuRtsctsEn, u1ArgNum);
						break;
					case 4:
						u4HdrDurEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4HdrDurEn, u1ArgNum);
						break;
					case 5:
						u4TxopDurEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4TxopDurEn, u1ArgNum);
						break;
					case 6:
						u4NonSrgInterPpduPresv = simple_strtol(value, 0, 10);
						Status = IsFlag(u4NonSrgInterPpduPresv, u1ArgNum);
						break;
					case 7:
						u4SrgInterPpduPresv = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrgInterPpduPresv, u1ArgNum);
						break;
					case 8:
						u4SrRemTimeEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRemTimeEn, u1ArgNum);
						break;
					case 9:
						u4ProtInSrWinDis = simple_strtol(value, 0, 10);
						Status = IsFlag(u4ProtInSrWinDis, u1ArgNum);
						break;
					case 10:
						u4TxCmdDlRateSelEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4TxCmdDlRateSelEn, u1ArgNum);
						break;
					case 11:
						u4AmpduTxCntEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4AmpduTxCntEn, u1ArgNum);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrCap.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrCap.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrCap.rSrCap.fgSrEn = u4SrEn;
					rSrCmdSrCap.rSrCap.fgSrgEn = u4SrgEn;
					rSrCmdSrCap.rSrCap.fgNonSrgEn = u4NonSrgEn;
					rSrCmdSrCap.rSrCap.fgSingleMdpuRtsctsEn = u4SingleMdpuRtsctsEn;
					rSrCmdSrCap.rSrCap.fgHdrDurEn = u4HdrDurEn;
					rSrCmdSrCap.rSrCap.fgTxopDurEn = u4TxopDurEn;
					rSrCmdSrCap.rSrCap.fgNonSrgInterPpduPresv = u4NonSrgInterPpduPresv;
					rSrCmdSrCap.rSrCap.fgSrgInterPpduPresv = u4SrgInterPpduPresv;
					rSrCmdSrCap.rSrCap.fgSrRemTimeEn = u4SrRemTimeEn;
					rSrCmdSrCap.rSrCap.fgProtInSrWinDis = u4ProtInSrWinDis;
					rSrCmdSrCap.rSrCap.fgTxCmdDlRateSelEn = u4TxCmdDlRateSelEn;
					rSrCmdSrCap.rSrCap.fgAmpduTxCntEn = u4AmpduTxCntEn;

					Status = SrCmdSRUpdateCap(pAd, &rSrCmdSrCap);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srcap=[SrEn]-[SrgEn]-[NonSrgEn]-[SingleMdpuRtsctsEn]-[HdrDurEn]-[TxopDurEn]-[NonSrgInterPpduPresv]-[SrgInterPpduPresv]-[SrRemTimeEn]-[ProtInSrWinDis]-[TxCmdDlRateSelEn]-[AmpduTxCntEn]\n"
				 );
		}

	} else if (IS_SR_V2(pAd)) {
		SR_CMD_SR_CAP_T_SR_V2 rSrCmdSrCap;

		os_zero_mem(&rSrCmdSrCap, sizeof(SR_CMD_SR_CAP_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrCap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CAP_ALL_CTRL;
		rSrCmdSrCap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_CAP_ALL_CTRL_ARG_NUM_SR_V2;
		rSrCmdSrCap.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
					 value = rstrtok(NULL, "-"), u1ArgNum++) {
					if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrEn, u1ArgNum);
						break;
					case 1:
						u4SrgEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrgEn, u1ArgNum);
						break;
					case 2:
						u4NonSrgEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4NonSrgEn, u1ArgNum);
						break;
					case 3:
						u4SingleMdpuRtsctsEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SingleMdpuRtsctsEn, u1ArgNum);
						break;
					case 4:
						u4HdrDurEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4HdrDurEn, u1ArgNum);
						break;
					case 5:
						u4TxopDurEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4TxopDurEn, u1ArgNum);
						break;
					case 6:
						u4NonSrgInterPpduPresv = simple_strtol(value, 0, 10);
						Status = IsFlag(u4NonSrgInterPpduPresv, u1ArgNum);
						break;
					case 7:
						u4SrgInterPpduPresv = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrgInterPpduPresv, u1ArgNum);
						break;
					case 8:
						u4SMpduNoTrigEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SMpduNoTrigEn, u1ArgNum);
						break;
					case 9:
						u4SrgBssidOrder = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrgBssidOrder, u1ArgNum);
						break;
					case 10:
						u4CtsAfterRts = simple_strtol(value, 0, 10);
						Status = IsFlag(u4CtsAfterRts, u1ArgNum);
						break;
					case 11:
						u4SrpOldRxvEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrpOldRxvEn, u1ArgNum);
						break;
					case 12:
						u4SrpNewRxvEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrpNewRxvEn, u1ArgNum);
						break;
					case 13:
						u4SrpDataOnlyEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrpDataOnlyEn, u1ArgNum);
						break;
					case 14:
						u4FixedRateSrREn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4FixedRateSrREn, u1ArgNum);
						break;
					case 15:
						u4WtblSrREn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4WtblSrREn, u1ArgNum);
						break;
					case 16:
						u4SrRemTimeEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRemTimeEn, u1ArgNum);
						break;
					case 17:
						u4ProtInSrWinDis = simple_strtol(value, 0, 10);
						Status = IsFlag(u4ProtInSrWinDis, u1ArgNum);
						break;
					case 18:
						u4TxCmdDlRateSelEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4TxCmdDlRateSelEn, u1ArgNum);
						break;
					case 19:
						u4AmpduTxCntEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4AmpduTxCntEn, u1ArgNum);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrCap.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrCap.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrCap.rSrCap.fgSrEn = u4SrEn;
					rSrCmdSrCap.rSrCap.fgSrgEn = u4SrgEn;
					rSrCmdSrCap.rSrCap.fgNonSrgEn = u4NonSrgEn;
					rSrCmdSrCap.rSrCap.fgSingleMdpuRtsctsEn = u4SingleMdpuRtsctsEn;
					rSrCmdSrCap.rSrCap.fgHdrDurEn = u4HdrDurEn;
					rSrCmdSrCap.rSrCap.fgTxopDurEn = u4TxopDurEn;
					rSrCmdSrCap.rSrCap.fgNonSrgInterPpduPresv = u4NonSrgInterPpduPresv;
					rSrCmdSrCap.rSrCap.fgSrgInterPpduPresv = u4SrgInterPpduPresv;
					rSrCmdSrCap.rSrCap.fgSMpduNoTrigEn = u4SMpduNoTrigEn;
					rSrCmdSrCap.rSrCap.fgSrgBssidOrder = u4SrgBssidOrder;
					rSrCmdSrCap.rSrCap.fgCtsAfterRts = u4CtsAfterRts;
					rSrCmdSrCap.rSrCap.fgSrpOldRxvEn = u4SrpOldRxvEn;
					rSrCmdSrCap.rSrCap.fgSrpNewRxvEn = u4SrpNewRxvEn;
					rSrCmdSrCap.rSrCap.fgSrpDataOnlyEn = u4SrpDataOnlyEn;
					rSrCmdSrCap.rSrCap.fgFixedRateSrREn = u4FixedRateSrREn;
					rSrCmdSrCap.rSrCap.fgWtblSrREn = u4WtblSrREn;
					rSrCmdSrCap.rSrCap.fgSrRemTimeEn = u4SrRemTimeEn;
					rSrCmdSrCap.rSrCap.fgProtInSrWinDis = u4ProtInSrWinDis;
					rSrCmdSrCap.rSrCap.fgTxCmdDlRateSelEn = u4TxCmdDlRateSelEn;
					rSrCmdSrCap.rSrCap.fgAmpduTxCntEn = u4AmpduTxCntEn;

					Status = SrCmdSRUpdateCap(pAd, &rSrCmdSrCap);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srcap=[SrEn]-[SrgEn]-[NonSrgEn]-[SingleMdpuRtsctsEn]-[HdrDurEn]-[TxopDurEn]\n"
				 "-[NonSrgInterPpduPresv]-[SrgInterPpduPresv]-[SMpduNoTrigEn]-[SrgBssidOrder]-[CtsAfterRts]\n"
				 "-[SrpOldRxvEn]-[SrpNewRxvEn]-[SrpDataOnlyEn]-[FixedRateSrREn]-[WtblSrREn]\n"
				 "-[SrRemTimeEn]-[ProtInSrWinDis]-[TxCmdDlRateSelEn]-[AmpduTxCntEn]\n");
		}

	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}

	return Status;
}

NDIS_STATUS SetSrParaAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	INT32 i4NonSrgPdThr = 0, i4SrgPdThr = 0;
	UINT_32 u4PeriodOfst = 0, u4RcpiSourceSel = 0;
	INT32 i4ObssPdMin = 0, i4ObssPdMinSrg = 0;
	UINT_32 u4RespTxPwrMode = 0, u4TxPwrRestricMode = 0, u4ObssTxPwrRef = 0;

	SR_CMD_SR_PARA_T rSrCmdSrPara;


	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&rSrCmdSrPara, sizeof(SR_CMD_SR_PARA_T));

	/* Assign Cmd Id */
	rSrCmdSrPara.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_PARA_ALL_CTRL;
	rSrCmdSrPara.rSrCmd.u1ArgNum = SR_CMD_SET_SR_PARA_ALL_CTRL_ARG_NUM;
	rSrCmdSrPara.rSrCmd.u1DbdcIdx = u1DbdcIdx;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					i4NonSrgPdThr = simple_strtol(value, 0, 10);
					i4NonSrgPdThr = i4NonSrgPdThr * (-1);
					Status =
					    IsInRange(i4NonSrgPdThr, u1ArgNum, SR_PARA_PD_THR_MIN,
						      SR_PARA_PD_THR_MAX);
					break;
				case 1:
					i4SrgPdThr = simple_strtol(value, 0, 10);
					i4SrgPdThr = i4SrgPdThr * (-1);
					Status =
					    IsInRange(i4SrgPdThr, u1ArgNum, SR_PARA_PD_THR_MIN,
						      SR_PARA_PD_THR_MAX);

					break;
				case 2:
					u4PeriodOfst = simple_strtol(value, 0, 16);
					Status =
					    IsInRange(u4PeriodOfst, u1ArgNum,
						      SR_PARA_PERIOD_OFST_MIN,
						      SR_PARA_PERIOD_OFST_MAX);
					break;
				case 3:
					u4RcpiSourceSel = simple_strtol(value, 0, 16);
					Status =
					    IsInRange(u4RcpiSourceSel, u1ArgNum,
						      SR_PARA_RCPI_SRC_SEL_ANT_0,
						      SR_PARA_RCPI_SRC_SEL_ANT_3);
					break;
				case 4:
					i4ObssPdMin = simple_strtol(value, 0, 10);
					i4ObssPdMin = i4ObssPdMin * (-1);
					Status =
					    IsInRange(i4ObssPdMin, u1ArgNum, SR_PARA_PD_MIN_MIN,
						      SR_PARA_PD_MIN_MAX);
					break;
				case 5:
					i4ObssPdMinSrg = simple_strtol(value, 0, 10);
					i4ObssPdMinSrg = i4ObssPdMinSrg * (-1);
					Status =
					    IsInRange(i4ObssPdMinSrg, u1ArgNum, SR_PARA_PD_MIN_MIN,
						      SR_PARA_PD_MIN_MAX);
					break;
				case 6:
					u4RespTxPwrMode = simple_strtol(value, 0, 10);
					Status =
					    IsInRange(u4RespTxPwrMode, u1ArgNum,
						      WH_ENUM_SR_RESP_TXPWR_NO_RESTRIC,
						      WH_ENUM_SR_RESP_TXPWR_RESTRIC);
					break;
				case 7:
					u4TxPwrRestricMode = simple_strtol(value, 0, 10);
					Status =
					    IsInRange(u4TxPwrRestricMode, u1ArgNum,
						      WH_ENUM_SR_TXPWR_RESTRIC_NO_RESTRIC,
						      WH_ENUM_SR_TXPWR_RESTRIC_FOLLOW_MTK);
					break;
				case 8:
					u4ObssTxPwrRef = simple_strtol(value, 0, 10);
					Status =
					    IsInRange(u4ObssTxPwrRef, u1ArgNum,
						      SR_PARA_OBSS_TXPWR_REF_MIN,
						      SR_PARA_OBSS_TXPWR_REF_MAX);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmdSrPara.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrPara.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {
				rSrCmdSrPara.rSrPara.u1NonSrgPdThr = i4NonSrgPdThr;
				rSrCmdSrPara.rSrPara.u1SrgPdThr = i4SrgPdThr;
				rSrCmdSrPara.rSrPara.u1PeriodOfst = u4PeriodOfst;
				rSrCmdSrPara.rSrPara.u1RcpiSourceSel = u4RcpiSourceSel;
				rSrCmdSrPara.rSrPara.u2ObssPdMin = i4ObssPdMin;
				rSrCmdSrPara.rSrPara.u2ObssPdMinSrg = i4ObssPdMinSrg;
				rSrCmdSrPara.rSrPara.eRespTxPwrMode = u4RespTxPwrMode;
				rSrCmdSrPara.rSrPara.eTxPwrRestricMode = u4TxPwrRestricMode;
				rSrCmdSrPara.rSrPara.u1ObssTxPwrRef = u4ObssTxPwrRef;

				Status = SrCmdSRUpdatePara(pAd, &rSrCmdSrPara);
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_PRINT("iwpriv ra0 set srpara=[NonSrgPdThr]-[SrgPdThr]-[PeriodOfst]-[RcpiSourceSel]-[ObssPdMin]-[ObssPdMinSrg]-[RespTxPwrMode]-[TxPwrRestricMode]-[ObssTxPwrRef]\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL **/
NDIS_STATUS SetSrDropTa(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4DropTaIdx = 0, u4Address2 = 0, u4StaIdx = 0;

	P_SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T prSrCmdSrGlobalVarSingleDropTa;

	os_alloc_mem(pAd, (UCHAR **)&prSrCmdSrGlobalVarSingleDropTa, sizeof(SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));

	if (!prSrCmdSrGlobalVarSingleDropTa)
		return NDIS_STATUS_FAILURE;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else {
		os_free_mem(prSrCmdSrGlobalVarSingleDropTa);
		return NDIS_STATUS_FAILURE;
	}
	/** For Update **/
	os_zero_mem(prSrCmdSrGlobalVarSingleDropTa, sizeof(SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));

	/* Assign Set Cmd Id */
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1CmdSubId = SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL;
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum = SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL_ARG_NUM;
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1DbdcIdx = u1DbdcIdx;


	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					u4DropTaIdx = simple_strtol(value, 0, 10);
					/** Not Support set all Drop Ta SR_DROP_TA_NUM - 1**/
					Status =
					    IsInRange(u4DropTaIdx, u1ArgNum, 0, SR_DROP_TA_NUM - 1);
					break;
				case 1:
					u4Address2 = simple_strtol(value, 0, 16);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {
				prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1DropTaIdx = u4DropTaIdx;
				prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.
				    rSrDropTaInfo.u4Address2 = u4Address2;
#ifdef RT_BIG_ENDIAN
		prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.rSrDropTaInfo.u4Address2 =
			cpu2le32(prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.rSrDropTaInfo.u4Address2);
#endif
				/** Set    GlobalVar **/
				Status =
				    SrCmdSRUpdateGloVarSingleDropTa(pAd,
								    prSrCmdSrGlobalVarSingleDropTa,
								    u4DropTaIdx, u4StaIdx);
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_PRINT(" iwpriv ra0 set srdropta=[u4DropTaIdx]-[u4Address2]\n");
	}

	if (prSrCmdSrGlobalVarSingleDropTa)
		os_free_mem(prSrCmdSrGlobalVarSingleDropTa);

	return Status;
}

/** SR_CMD_SET_SR_GLO_VAR_STA_CTRL **/
NDIS_STATUS SetSrSta(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4DropTaIdx = 0, u4StaIdx = 0, u4SrRateOffset = 0, u4WlanId = 0;

	P_SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T prSrCmdSrGlobalVarSingleDropTa;

	os_alloc_mem(pAd, (UCHAR **)&prSrCmdSrGlobalVarSingleDropTa, sizeof(SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));

	if (!prSrCmdSrGlobalVarSingleDropTa)
		return NDIS_STATUS_FAILURE;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else {
		os_free_mem(prSrCmdSrGlobalVarSingleDropTa);
		return NDIS_STATUS_FAILURE;
	}
	/** For Update **/
	os_zero_mem(prSrCmdSrGlobalVarSingleDropTa, sizeof(SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));

	/* Assign Set Cmd Id */
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1CmdSubId = SR_CMD_SET_SR_GLO_VAR_STA_CTRL;
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum = SR_CMD_SET_SR_GLO_VAR_STA_CTRL_ARG_NUM;
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1DbdcIdx = u1DbdcIdx;

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					u4DropTaIdx = simple_strtol(value, 0, 10);
					/** Not Support set all Drop Ta SR_DROP_TA_NUM - 1**/
					Status =
					    IsInRange(u4DropTaIdx, u1ArgNum, 0, SR_DROP_TA_NUM - 1);
					break;
				case 1:
					u4StaIdx = simple_strtol(value, 0, 10);
					/** Not Support set all STA SR_STA_NUM - 1**/
					Status =
					    IsInRange(u4StaIdx, u1ArgNum, 0, SR_STA_NUM - 1);
					break;
				case 2:
					u4WlanId = simple_strtol(value, 0, 10);
					Status = IsInRange(u4WlanId, u1ArgNum, 0, 256);
					break;
				case 3:
					u4SrRateOffset = simple_strtol(value, 0, 10);
					Status =
						IsInRange(u4SrRateOffset, u1ArgNum, 0, 11);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {
				/** Update GlobalVar **/
				prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1DropTaIdx = u4DropTaIdx;
				prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1StaIdx = u4StaIdx;
				prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.
				    rSrDropTaInfo.rSrStaInfo[u4StaIdx].u2WlanId = u4WlanId;
#ifdef RT_BIG_ENDIAN
	prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.rSrDropTaInfo.rSrStaInfo[u4StaIdx].u2WlanId =
		cpu2le16(prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.rSrDropTaInfo.rSrStaInfo[u4StaIdx].u2WlanId);
#endif
				prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.
				    rSrDropTaInfo.rSrStaInfo[u4StaIdx].u1Mode =
				    ENUM_SR_STA_MODE_FIXED;
				prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa.
				    rSrDropTaInfo.rSrStaInfo[u4StaIdx].u1SrRateOffset =
				    u4SrRateOffset;

				/** Set    GlobalVar **/
				Status =
				    SrCmdSRUpdateGloVarSingleDropTa(pAd,
								    prSrCmdSrGlobalVarSingleDropTa,
								    u4DropTaIdx, u4StaIdx);
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_PRINT("iwpriv ra0 set srsta=[u4DropTaIdx]-[u4StaIdx]-[u4WlanId]-[u4SrRateOffset]\n");
	}

	if (prSrCmdSrGlobalVarSingleDropTa)
		os_free_mem(prSrCmdSrGlobalVarSingleDropTa);

	return Status;
}

/** SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL **/
NDIS_STATUS SetSrStaInit(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4DropTaIdx = 0, u4StaIdx = 0;

	P_SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T prSrCmdSrGlobalVarSingleDropTa;

	os_alloc_mem(pAd, (UCHAR **)&prSrCmdSrGlobalVarSingleDropTa, sizeof(SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));

	if (!prSrCmdSrGlobalVarSingleDropTa)
		return NDIS_STATUS_FAILURE;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else {
		os_free_mem(prSrCmdSrGlobalVarSingleDropTa);
		return NDIS_STATUS_FAILURE;
	}
	/** For Update **/
	os_zero_mem(prSrCmdSrGlobalVarSingleDropTa, sizeof(SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));

	/* Assign Set Cmd Id */
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1CmdSubId = SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL;
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum = SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL_ARG_NUM;
	prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1DbdcIdx = u1DbdcIdx;

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					u4DropTaIdx = simple_strtol(value, 0, 10);
					/** Not Support init all Drop Ta SR_DROP_TA_NUM - 1**/
					Status =
					    IsInRange(u4DropTaIdx, u1ArgNum, 0, SR_DROP_TA_NUM - 1);
					break;
				case 1:
					u4StaIdx = simple_strtol(value, 0, 10);
					/** Support init all STA SR_STA_NUM**/
					Status =
					    IsInRange(u4StaIdx, u1ArgNum, 0, SR_STA_NUM);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {
				/** Update GlobalVar **/
				prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1DropTaIdx = u4DropTaIdx;
				prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1StaIdx = u4StaIdx;
				/** Set    GlobalVar **/
				Status =
				    SrCmdSRUpdateGloVarSingleDropTa(pAd,
								    prSrCmdSrGlobalVarSingleDropTa,
								    u4DropTaIdx, u4StaIdx);
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_PRINT("iwpriv ra0 set srstainit=[u4DropTaIdx]-[u4StaIdx]\n");
	}

	if (prSrCmdSrGlobalVarSingleDropTa)
		os_free_mem(prSrCmdSrGlobalVarSingleDropTa);

	return Status;
}

/** SR_CMD_SET_SR_COND_ALL_CTRL **/
NDIS_STATUS SetSrCondAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4SrRcpiSel      = 0, u4SrRcpiCckRateEn = 0, u4SrMacRcpiRateEn = 0, u4SrRxvRcpiRateEn = 0;
	UINT_32 u4SrRcpiHeRateEn = 0, u4SrRcpiVhtRateEn = 0, u4SrRcpiHtRateEn  = 0, u4SrRcpiLgRateEn  = 0;
	UINT_32 u4SrRxvEntry     = 0, u4SrPeriodLimitEn = 0, u4SrPeriodLimit   = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (IS_SR_V1(pAd)) {
		SR_CMD_SR_COND_T_SR_V1 rSrCmdSrCond;

		os_zero_mem(&rSrCmdSrCond, sizeof(SR_CMD_SR_COND_T_SR_V1));

		/* Assign Cmd Id */
		rSrCmdSrCond.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_COND_ALL_CTRL;
		rSrCmdSrCond.rSrCmd.u1ArgNum = SR_CMD_SET_SR_COND_ALL_CTRL_ARG_NUM_SR_V1;
		rSrCmdSrCond.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrRcpiCckRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiCckRateEn, u1ArgNum);
						break;
					case 1:
						u4SrMacRcpiRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrMacRcpiRateEn, u1ArgNum);
						break;
					case 2:
						u4SrRxvRcpiRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRxvRcpiRateEn, u1ArgNum);
						break;
					case 3:
						u4SrRcpiHeRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiHeRateEn, u1ArgNum);
						break;
					case 4:
						u4SrRcpiVhtRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiVhtRateEn, u1ArgNum);
						break;
					case 5:
						u4SrRcpiHtRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiHtRateEn, u1ArgNum);
						break;
					case 6:
						u4SrRcpiLgRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiLgRateEn, u1ArgNum);
						break;
					case 7:
						u4SrRxvEntry = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRxvEntry, u1ArgNum);
						break;
					case 8:
						u4SrPeriodLimitEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrPeriodLimitEn, u1ArgNum);
						break;
					case 9:
						u4SrPeriodLimit = simple_strtol(value, 0, 10);
						Status = IsInRange(u4SrPeriodLimit, u1ArgNum, SR_COND_PERIOD_LIMIT_MIN,
							      SR_COND_PERIOD_LIMIT_MAX);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrCond.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrCond.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrCond.rSrCond.fgSrRcpiCckRateEn = u4SrRcpiCckRateEn;
					rSrCmdSrCond.rSrCond.fgSrMacRcpiRateEn = u4SrMacRcpiRateEn;
					rSrCmdSrCond.rSrCond.fgSrRxvRcpiRateEn = u4SrRxvRcpiRateEn;
					rSrCmdSrCond.rSrCond.fgSrRcpiHeRateEn = u4SrRcpiHeRateEn;
					rSrCmdSrCond.rSrCond.fgSrRcpiVhtRateEn = u4SrRcpiVhtRateEn;
					rSrCmdSrCond.rSrCond.fgSrRcpiHtRateEn = u4SrRcpiHtRateEn;
					rSrCmdSrCond.rSrCond.fgSrRcpiLgRateEn = u4SrRcpiLgRateEn;
					rSrCmdSrCond.rSrCond.fgSrRxvEntry = u4SrRxvEntry;
					rSrCmdSrCond.rSrCond.fgSrPeriodLimitEn = u4SrPeriodLimitEn;
					rSrCmdSrCond.rSrCond.u1SrPeriodLimit = u4SrPeriodLimit;

					Status = SrCmdSRUpdateCond(pAd, &rSrCmdSrCond);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srcond=[SrRcpiCckRateEn]-[SrMacRcpiRateEn]-[SrRxvRcpiRateEn]-[SrRcpiHeRateEn]-[u4SrRcpiVhtRateEn]-[SrRcpiHtRateEn]-[SrRcpiLgRateEn]-[SrRxvEntry]-[SrPeriodLimitEn]-[SrPeriodLimit]\n");
		}
	} else if (IS_SR_V2(pAd)) {
		SR_CMD_SR_COND_T_SR_V2 rSrCmdSrCond;

		os_zero_mem(&rSrCmdSrCond, sizeof(SR_CMD_SR_COND_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrCond.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_COND_ALL_CTRL;
		rSrCmdSrCond.rSrCmd.u1ArgNum = SR_CMD_SET_SR_COND_ALL_CTRL_ARG_NUM_SR_V2;
		rSrCmdSrCond.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrRcpiSel = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiSel, u1ArgNum);
						break;
					case 1:
						u4SrRcpiCckRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiCckRateEn, u1ArgNum);
						break;
					case 2:
						u4SrMacRcpiRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrMacRcpiRateEn, u1ArgNum);
						break;
					case 3:
						u4SrRxvRcpiRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRxvRcpiRateEn, u1ArgNum);
						break;
					case 4:
						u4SrRcpiHeRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiHeRateEn, u1ArgNum);
						break;
					case 5:
						u4SrRcpiVhtRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiVhtRateEn, u1ArgNum);
						break;
					case 6:
						u4SrRcpiHtRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiHtRateEn, u1ArgNum);
						break;
					case 7:
						u4SrRcpiLgRateEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRcpiLgRateEn, u1ArgNum);
						break;
					case 8:
						u4SrRxvEntry = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRxvEntry, u1ArgNum);
						break;
					case 9:
						u4SrPeriodLimitEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrPeriodLimitEn, u1ArgNum);
						break;
					case 10:
						u4SrPeriodLimit = simple_strtol(value, 0, 16);
						Status = IsInRange(u4SrPeriodLimit, u1ArgNum, SR_COND_PERIOD_LIMIT_MIN,
							      SR_COND_PERIOD_LIMIT_MAX);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrCond.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrCond.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrCond.rSrCond.fgSrRcpiSel = u4SrRcpiSel;
					rSrCmdSrCond.rSrCond.fgSrRcpiCckRateEn = u4SrRcpiCckRateEn;
					rSrCmdSrCond.rSrCond.fgSrMacRcpiRateEn = u4SrMacRcpiRateEn;
					rSrCmdSrCond.rSrCond.fgSrRxvRcpiRateEn = u4SrRxvRcpiRateEn;
					rSrCmdSrCond.rSrCond.fgSrRcpiHeRateEn = u4SrRcpiHeRateEn;
					rSrCmdSrCond.rSrCond.fgSrRcpiVhtRateEn = u4SrRcpiVhtRateEn;
					rSrCmdSrCond.rSrCond.fgSrRcpiHtRateEn = u4SrRcpiHtRateEn;
					rSrCmdSrCond.rSrCond.fgSrRcpiLgRateEn = u4SrRcpiLgRateEn;
					rSrCmdSrCond.rSrCond.fgSrRxvEntry = u4SrRxvEntry;
					rSrCmdSrCond.rSrCond.fgSrPeriodLimitEn = u4SrPeriodLimitEn;
					rSrCmdSrCond.rSrCond.u1SrPeriodLimit = u4SrPeriodLimit;

					Status = SrCmdSRUpdateCond(pAd, &rSrCmdSrCond);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srcond=[SrRcpiSel]-[SrRcpiCckRateEn]-[SrMacRcpiRateEn]-[SrRxvRcpiRateEn]-[SrRcpiHeRateEn]-[u4SrRcpiVhtRateEn]-[SrRcpiHtRateEn]-[SrRcpiLgRateEn]-[SrRxvEntry]-[SrPeriodLimitEn]-[SrPeriodLimit]\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL **/
NDIS_STATUS SetSrRcpiTblAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0, u1Index = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	INT32 i4RcpiTblMcs[SR_RCPITBL_MCS_NUM];

	SR_CMD_SR_RCPITBL_T rSrCmdSrRcpiTbl;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&rSrCmdSrRcpiTbl, sizeof(SR_CMD_SR_RCPITBL_T));

	/* Assign Cmd Id */
	rSrCmdSrRcpiTbl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL;
	rSrCmdSrRcpiTbl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL_ARG_NUM;
	rSrCmdSrRcpiTbl.rSrCmd.u1DbdcIdx = u1DbdcIdx;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
				case 11:
					i4RcpiTblMcs[u1ArgNum] = simple_strtol(value, 0, 10);
					i4RcpiTblMcs[u1ArgNum] = i4RcpiTblMcs[u1ArgNum] * (-1);
					Status = IsInRange(i4RcpiTblMcs[u1ArgNum], u1ArgNum, SR_RCPI_MIN, SR_RCPI_MAX);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmdSrRcpiTbl.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrRcpiTbl.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {
				for (u1Index = 0; u1Index < SR_RCPITBL_MCS_NUM; u1Index++)
					rSrCmdSrRcpiTbl.rSrRcpiTbl.u1RcpiTblMcs[u1Index] = SRRcpiConv((INT_8)i4RcpiTblMcs[u1Index]);

				Status = SrCmdSRUpdateRcpiTbl(pAd, &rSrCmdSrRcpiTbl);
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_PRINT("iwpriv ra0 set srrcpitbl=[RcpiTblMcs[0]]-...-[RcpiTblMcs[11]]\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL **/
NDIS_STATUS SetSrRcpiTblOfstAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4RxBwRcpiOfst = 0, u4StbcRcpiOfst = 0, u4NumAntRcpiOfst = 0;
	UINT_32 u4LdpcRcpiOfst = 0, u4DcmRcpiOfst  = 0, u4MacRcpiOfst = 0;
	UINT_32 u4SigRcpiOfst  = 0, u4BfRcpiOfst   = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (IS_SR_V1(pAd)) {
		SR_CMD_SR_RCPITBL_OFST_T_SR_V1 rSrCmdSrRcpiTblOfst;

		os_zero_mem(&rSrCmdSrRcpiTblOfst, sizeof(SR_CMD_SR_RCPITBL_OFST_T_SR_V1));

		/* Assign Cmd Id */
		rSrCmdSrRcpiTblOfst.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL;
		rSrCmdSrRcpiTblOfst.rSrCmd.u1ArgNum = SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL_ARG_NUM_SR_V1;
		rSrCmdSrRcpiTblOfst.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4RxBwRcpiOfst = simple_strtol(value, 0, 10);
						Status = IsInRange(u4RxBwRcpiOfst, u1ArgNum, 0, BIT12);
						break;
					case 1:
						u4StbcRcpiOfst = simple_strtol(value, 0, 10);
						Status = IsInRange(u4StbcRcpiOfst, u1ArgNum, 0, BIT4);
						break;
					case 2:
						u4NumAntRcpiOfst = simple_strtol(value, 0, 10);
						Status = IsInRange(u4NumAntRcpiOfst, u1ArgNum, 0, BIT12);
						break;
					case 3:
						u4LdpcRcpiOfst = simple_strtol(value, 0, 10);
						Status = IsInRange(u4LdpcRcpiOfst, u1ArgNum, 0, BIT4);
						break;
					case 4:
						u4DcmRcpiOfst = simple_strtol(value, 0, 10);
						Status = IsInRange(u4LdpcRcpiOfst, u1ArgNum, 0, BIT8);
						break;
					case 5:
						u4MacRcpiOfst = simple_strtol(value, 0, 10);
						Status = IsInRange(u4LdpcRcpiOfst, u1ArgNum, 0, BIT8);
						break;
					case 6:
						u4SigRcpiOfst = simple_strtol(value, 0, 10);
						Status = IsInRange(u4LdpcRcpiOfst, u1ArgNum, 0, BIT8);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrRcpiTblOfst.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrRcpiTblOfst.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2RxBwRcpiOfst = u4RxBwRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2StbcRcpiOfst = u4StbcRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2NumAntRcpiOfst = u4NumAntRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2LdpcRcpiOfst = u4LdpcRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2DcmRcpiOfst = u4DcmRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2MacRcpiOfst = u4MacRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2SigRcpiOfst = u4SigRcpiOfst;
#ifdef RT_BIG_ENDIAN
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2RxBwRcpiOfst
						= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2RxBwRcpiOfst);
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2StbcRcpiOfst
						= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2StbcRcpiOfst);
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2NumAntRcpiOfst
						= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2NumAntRcpiOfst);
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2LdpcRcpiOfst
						= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2LdpcRcpiOfst);
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2DcmRcpiOfst
						= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2DcmRcpiOfst);
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2MacRcpiOfst
						= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2MacRcpiOfst);
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2SigRcpiOfst
						= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2SigRcpiOfst);
#endif
					Status = SrCmdSRUpdateRcpiTblOfst(pAd, &rSrCmdSrRcpiTblOfst);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srrcpitblofst=[RxBwRcpiOfst]-[StbcRcpiOfst]-[NumAntRcpiOfst]-[LdpcRcpiOfst]-[DcmRcpiOfst]-[MacRcpiOfst]-[SigRcpiOfst]\n");
		}

	} else if (IS_SR_V2(pAd)) {

		SR_CMD_SR_RCPITBL_OFST_T_SR_V2 rSrCmdSrRcpiTblOfst;

		os_zero_mem(&rSrCmdSrRcpiTblOfst, sizeof(SR_CMD_SR_RCPITBL_OFST_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrRcpiTblOfst.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL;
		rSrCmdSrRcpiTblOfst.rSrCmd.u1ArgNum = SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL_ARG_NUM_SR_V2;
		rSrCmdSrRcpiTblOfst.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
					 value = rstrtok(NULL, "-"), u1ArgNum++) {
					if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4RxBwRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4RxBwRcpiOfst, u1ArgNum, 0, BIT12);
						break;
					case 1:
						u4StbcRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4StbcRcpiOfst, u1ArgNum, 0, BIT4);
						break;
					case 2:
						u4NumAntRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4NumAntRcpiOfst, u1ArgNum, 0, BIT12);
						break;
					case 3:
						u4LdpcRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4LdpcRcpiOfst, u1ArgNum, 0, BIT4);
						break;
					case 4:
						u4DcmRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4DcmRcpiOfst, u1ArgNum, 0, BIT8);
						break;
					case 5:
						u4MacRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4MacRcpiOfst, u1ArgNum, 0, BIT8);
						break;
					case 6:
						u4SigRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4SigRcpiOfst, u1ArgNum, 0, BIT8);
						break;
					case 7:
						u4BfRcpiOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4BfRcpiOfst, u1ArgNum, 0, BIT4);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrRcpiTblOfst.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrRcpiTblOfst.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2RxBwRcpiOfst = u4RxBwRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2StbcRcpiOfst = u4StbcRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2NumAntRcpiOfst = u4NumAntRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2LdpcRcpiOfst = u4LdpcRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2DcmRcpiOfst = u4DcmRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2MacRcpiOfst = u4MacRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2SigRcpiOfst = u4SigRcpiOfst;
					rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2BfRcpiOfst = u4BfRcpiOfst;

#ifdef RT_BIG_ENDIAN
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2RxBwRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2RxBwRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2StbcRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2StbcRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2NumAntRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2NumAntRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2LdpcRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2LdpcRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2DcmRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2DcmRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2MacRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2MacRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2SigRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2SigRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2BfRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2BfRcpiOfst);
#endif

					Status = SrCmdSRUpdateRcpiTblOfst(pAd, &rSrCmdSrRcpiTblOfst);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srrcpitblofst=[RxBwRcpiOfst]-[StbcRcpiOfst]-[NumAntRcpiOfst]-[LdpcRcpiOfst]-[DcmRcpiOfst]-[MacRcpiOfst]-[SigRcpiOfst]-[BfRcpiOfst]\n");
		}

	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_Q_CTRL_ALL_CTRL **/
NDIS_STATUS SetSrQCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4SrRxRptEn = 0, u4SrCw = 0, u4SrSuspend = 0, u4SrDisSwAifsDis = 0;
	UINT_32 u4SrBackOffMask = 0;
	UINT_32 u4SrBackOffEnable = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (IS_SR_V1(pAd)) {
		SR_CMD_SR_Q_CTRL_T_SR_V1 rSrCmdSrQCtrl;

		os_zero_mem(&rSrCmdSrQCtrl, sizeof(SR_CMD_SR_Q_CTRL_T_SR_V1));

		/* Assign Cmd Id */
		rSrCmdSrQCtrl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_Q_CTRL_ALL_CTRL;
		rSrCmdSrQCtrl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_Q_CTRL_ALL_CTRL_ARG_NUM_SR_V1;
		rSrCmdSrQCtrl.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrRxRptEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRxRptEn, u1ArgNum);
						break;
					case 1:
						u4SrCw = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrCw, u1ArgNum);
						break;
					case 2:
						u4SrSuspend = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrSuspend, u1ArgNum);
					break;
					case 3:
						u4SrBackOffMask = simple_strtol(value, 0, 10);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrQCtrl.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrQCtrl.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrQCtrl.rSrQCtrl.fgSrRxRptEn = u4SrRxRptEn;
					rSrCmdSrQCtrl.rSrQCtrl.fgSrCw = u4SrCw;
					rSrCmdSrQCtrl.rSrQCtrl.fgSrSuspend = u4SrSuspend;
					rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffMask = u4SrBackOffMask;
#ifdef RT_BIG_ENDIAN
					rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffMask
						= cpu2le32(rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffMask);
#endif
					Status = SrCmdSRUpdateQCtrl(pAd, &rSrCmdSrQCtrl);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT(" iwpriv ra0 set srqctrl=[SrRxRptEn]-[SrCw]-[SrSuspend]-[SrBackOffMask]\n");
		}
	} else if (IS_SR_V2(pAd)) {
		SR_CMD_SR_Q_CTRL_T_SR_V2 rSrCmdSrQCtrl;

		os_zero_mem(&rSrCmdSrQCtrl, sizeof(SR_CMD_SR_Q_CTRL_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrQCtrl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_Q_CTRL_ALL_CTRL;
		rSrCmdSrQCtrl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_Q_CTRL_ALL_CTRL_ARG_NUM_SR_V2;
		rSrCmdSrQCtrl.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrRxRptEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrRxRptEn, u1ArgNum);
						break;
					case 1:
						u4SrCw = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrCw, u1ArgNum);
						break;
					case 2:
						u4SrSuspend = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrSuspend, u1ArgNum);
						break;
					case 3:
						u4SrDisSwAifsDis = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrDisSwAifsDis, u1ArgNum);
						break;
					case 4:
						u4SrBackOffMask = simple_strtol(value, 0, 16);
						break;
					case 5:
						u4SrBackOffEnable = simple_strtol(value, 0, 16);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

			if (u1ArgNum != rSrCmdSrQCtrl.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrQCtrl.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {
				rSrCmdSrQCtrl.rSrQCtrl.fgSrRxRptEn = u4SrRxRptEn;
				rSrCmdSrQCtrl.rSrQCtrl.fgSrCw = u4SrCw;
				rSrCmdSrQCtrl.rSrQCtrl.fgSrSuspend = u4SrSuspend;
				rSrCmdSrQCtrl.rSrQCtrl.fgSrDisSwAifsDis = u4SrDisSwAifsDis;
				rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffMask = u4SrBackOffMask;
				rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffEnable = u4SrBackOffEnable;

#ifdef RT_BIG_ENDIAN
					rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffMask
						= cpu2le32(rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffMask);
					rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffEnable
						= cpu2le32(rSrCmdSrQCtrl.rSrQCtrl.u4SrBackOffEnable);
#endif
					Status = SrCmdSRUpdateQCtrl(pAd, &rSrCmdSrQCtrl);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srqctrl=[SrRxRptEn]-[SrCw]-[SrSuspend]-[SrDisSwAifsDis]-[SrBackOffMask]-[u4SrBackOffEnable]\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_IBPD_ALL_CTRL **/
NDIS_STATUS SetSrIBPDAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4InterBssByHdrBssid = 0, u4InterBssByMu = 0, u4InterBssByPbssColor = 0;
	UINT_32 u4InterBssByPaid = 0, u4InterBssByBssColor = 0;


	SR_CMD_SR_IBPD_T rSrCmdSrIBPD;


	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&rSrCmdSrIBPD, sizeof(SR_CMD_SR_IBPD_T));

	/* Assign Cmd Id */
	rSrCmdSrIBPD.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_IBPD_ALL_CTRL;
	rSrCmdSrIBPD.rSrCmd.u1ArgNum = SR_CMD_SET_SR_IBPD_ALL_CTRL_ARG_NUM;
	rSrCmdSrIBPD.rSrCmd.u1DbdcIdx = u1DbdcIdx;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					u4InterBssByHdrBssid = simple_strtol(value, 0, 16);
					Status = IsInRange(u4InterBssByHdrBssid, u1ArgNum, 0, BIT8);
					break;
				case 1:
					u4InterBssByMu = simple_strtol(value, 0, 16);
					Status = IsInRange(u4InterBssByMu, u1ArgNum, 0, BIT8);
					break;
				case 2:
					u4InterBssByPbssColor = simple_strtol(value, 0, 16);
					Status = IsInRange(u4InterBssByPbssColor, u1ArgNum, 0, BIT8);
					break;
				case 3:
					u4InterBssByPaid = simple_strtol(value, 0, 16);
					Status = IsInRange(u4InterBssByPaid, u1ArgNum, 0, BIT8);
					break;
				case 4:
					u4InterBssByBssColor = simple_strtol(value, 0, 16);
					Status = IsInRange(u4InterBssByBssColor, u1ArgNum, 0, BIT8);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmdSrIBPD.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrIBPD.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {
				rSrCmdSrIBPD.rSrIBPD.u1InterBssByHdrBssid = u4InterBssByHdrBssid;
				rSrCmdSrIBPD.rSrIBPD.u1InterBssByMu = u4InterBssByMu;
				rSrCmdSrIBPD.rSrIBPD.u1InterBssByPbssColor = u4InterBssByPbssColor;
				rSrCmdSrIBPD.rSrIBPD.u1InterBssByPaid = u4InterBssByPaid;
				rSrCmdSrIBPD.rSrIBPD.u1InterBssByBssColor = u4InterBssByBssColor;

				Status = SrCmdSRUpdateIBPD(pAd, &rSrCmdSrIBPD);
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_PRINT(" iwpriv ra0 set sribpd=[InterBssByHdrBssid]-[InterBssByMu]-[InterBssByPbssColor]-[InterBssByPaid]-[InterBssByBssColor]\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_NRT_ALL_CTRL **/
NDIS_STATUS SetSrNRTAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4TableIdx = 0, u4RaTaSel = 0, u4SwProtect = 0, u4NRTValue = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (IS_SR_V1(pAd)) {
		SR_CMD_SR_NRT_T_SR_V1 rSrCmdSrNRT;

		os_zero_mem(&rSrCmdSrNRT, sizeof(SR_CMD_SR_NRT_T_SR_V1));

		/* Assign Cmd Id */
		rSrCmdSrNRT.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_NRT_ALL_CTRL;
		rSrCmdSrNRT.rSrCmd.u1ArgNum = SR_CMD_SET_SR_NRT_ALL_CTRL_ARG_NUM_SR_V1;
		rSrCmdSrNRT.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4TableIdx = simple_strtol(value, 0, 10);
						Status = IsInRange(u4TableIdx, u1ArgNum, 0, SR_NRT_ROW_NUM);
						break;
					case 1:
						u4NRTValue = simple_strtol(value, 0, 10);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrNRT.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrNRT.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrNRT.rSrNRT.u1TableIdx = u4TableIdx;
					rSrCmdSrNRT.rSrNRT.u4NRTValue = u4NRTValue;
#ifdef RT_BIG_ENDIAN
					rSrCmdSrNRT.rSrNRT.u4NRTValue
						= cpu2le32(rSrCmdSrNRT.rSrNRT.u4NRTValue);
#endif
					Status = SrCmdSRUpdateNRT(pAd, &rSrCmdSrNRT);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srnrt=[TableIdx]-[NRTValue]\n");
		}
	} else if (IS_SR_V2(pAd)) {
		SR_CMD_SR_NRT_T_SR_V2 rSrCmdSrNRT;

		os_zero_mem(&rSrCmdSrNRT, sizeof(SR_CMD_SR_NRT_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrNRT.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_NRT_ALL_CTRL;
		rSrCmdSrNRT.rSrCmd.u1ArgNum = SR_CMD_SET_SR_NRT_ALL_CTRL_ARG_NUM_SR_V2;
		rSrCmdSrNRT.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4TableIdx = simple_strtol(value, 0, 10);
						Status = IsInRange(u4TableIdx, u1ArgNum, 0, SR_NRT_ROW_NUM);
						break;
					case 1:
						u4RaTaSel = simple_strtol(value, 0, 10);
						Status = IsInRange(u4RaTaSel, u1ArgNum, 0, BIT2);
						break;
					case 2:
						u4SwProtect = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SwProtect, u1ArgNum);
						break;
					case 3:
						u4NRTValue = simple_strtol(value, 0, 16);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrNRT.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrNRT.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrNRT.rSrNRT.u1TableIdx = u4TableIdx;
					rSrCmdSrNRT.rSrNRT.u1RaTaSel = u4RaTaSel;
					rSrCmdSrNRT.rSrNRT.fgSwProtect = u4SwProtect;
					rSrCmdSrNRT.rSrNRT.u4NRTValue = u4NRTValue;
#ifdef RT_BIG_ENDIAN
					rSrCmdSrNRT.rSrNRT.u4NRTValue
						= cpu2le32(rSrCmdSrNRT.rSrNRT.u4NRTValue);
#endif
					Status = SrCmdSRUpdateNRT(pAd, &rSrCmdSrNRT);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srnrt=[TableIdx]-[RaTaSel]-[SwProtect]-[NRTValue]\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_NRT_RESET_CTRL **/
NDIS_STATUS SetSrNRTResetAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_NRT_RESET_CTRL, SR_CMD_SET_SR_NRT_RESET_CTRL_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srnrtreset=0\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL **/
NDIS_STATUS SetSrNRTCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4SrtEn = 0, u4SrtSrpEn = 0, u4SrtAddrOrderEn = 0, u4SrtByPassCtsAck = 0;
	UINT_32 u4SrtInRcpiTh = 0, u4SrtOutRcpiTh = 0;
	UINT_32 u4SrtUsedCntTh = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (IS_SR_V1(pAd)) {
		SR_CMD_SR_NRT_CTRL_T_SR_V1 rSrCmdSrNRTCtrl;

		os_zero_mem(&rSrCmdSrNRTCtrl, sizeof(SR_CMD_SR_NRT_CTRL_T_SR_V1));

		/* Assign Cmd Id */
		rSrCmdSrNRTCtrl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL;
		rSrCmdSrNRTCtrl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL_ARG_NUM_SR_V1;
		rSrCmdSrNRTCtrl.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrtEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrtEn, u1ArgNum);
						break;
					case 1:
						u4SrtSrpEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrtSrpEn, u1ArgNum);
						break;
					case 2:
						u4SrtAddrOrderEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrtAddrOrderEn, u1ArgNum);
						break;
					case 3:
						u4SrtInRcpiTh = simple_strtol(value, 0, 10);
						Status = IsInRange(u4SrtInRcpiTh, u1ArgNum, 0, BIT16);
						break;
					case 4:
						u4SrtOutRcpiTh = simple_strtol(value, 0, 10);
						Status = IsInRange(u4SrtOutRcpiTh, u1ArgNum, 0, BIT16);
						break;
					case 5:
						u4SrtUsedCntTh = simple_strtol(value, 0, 10);
						Status = IsInRange(u4SrtUsedCntTh, u1ArgNum, 0, BIT16);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrNRTCtrl.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrNRTCtrl.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrNRTCtrl.rSrNRTCtrl.fgSrtEn = u4SrtEn;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.fgSrtSrpEn = u4SrtSrpEn;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.fgSrtAddrOrderEn = u4SrtAddrOrderEn;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtInRcpiTh = u4SrtInRcpiTh;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtOutRcpiTh = u4SrtOutRcpiTh;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtUsedCntTh = u4SrtUsedCntTh;
#ifdef RT_BIG_ENDIAN
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtInRcpiTh
						= cpu2le16(rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtInRcpiTh);
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtOutRcpiTh
						= cpu2le16(rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtOutRcpiTh);
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtUsedCntTh
						= cpu2le16(rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtUsedCntTh);
#endif
					Status = SrCmdSRUpdateNRTCtrl(pAd, &rSrCmdSrNRTCtrl);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srnrtctrl=[SrtEn]-[SrtSrpEn]-[SrtAddrOrderEn]-[SrtInRcpiTh]-[SrtOutRcpiTh]-[SrtUsedCntTh]\n");
		}
	} else if (IS_SR_V2(pAd)) {
		SR_CMD_SR_NRT_CTRL_T_SR_V2 rSrCmdSrNRTCtrl;

		os_zero_mem(&rSrCmdSrNRTCtrl, sizeof(SR_CMD_SR_NRT_CTRL_T_SR_V2));

		/* Assign Cmd Id */
		rSrCmdSrNRTCtrl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL;
		rSrCmdSrNRTCtrl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL_ARG_NUM_SR_V2;
		rSrCmdSrNRTCtrl.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
					 value = rstrtok(NULL, "-"), u1ArgNum++) {
					if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrtEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrtEn, u1ArgNum);
						break;
					case 1:
						u4SrtSrpEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrtSrpEn, u1ArgNum);
						break;
					case 2:
						u4SrtAddrOrderEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrtAddrOrderEn, u1ArgNum);
						break;
					case 3:
						u4SrtByPassCtsAck = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrtByPassCtsAck, u1ArgNum);
						break;
					case 4:
						u4SrtInRcpiTh = simple_strtol(value, 0, 16);
						Status = IsInRange(u4SrtInRcpiTh, u1ArgNum, 0, BIT16);
						break;
					case 5:
						u4SrtOutRcpiTh = simple_strtol(value, 0, 16);
						Status = IsInRange(u4SrtOutRcpiTh, u1ArgNum, 0, BIT16);
						break;
					case 6:
						u4SrtUsedCntTh = simple_strtol(value, 0, 16);
						Status = IsInRange(u4SrtUsedCntTh, u1ArgNum, 0, BIT16);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

				if (u1ArgNum != rSrCmdSrNRTCtrl.rSrCmd.u1ArgNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 " Format Error! ArgNum = %d != %d\n",
						  u1ArgNum, rSrCmdSrNRTCtrl.rSrCmd.u1ArgNum);
					Status = NDIS_STATUS_FAILURE;
					break;
				} else {
					rSrCmdSrNRTCtrl.rSrNRTCtrl.fgSrtEn = u4SrtEn;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.fgSrtSrpEn = u4SrtSrpEn;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.fgSrtAddrOrderEn = u4SrtAddrOrderEn;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.fgSrtByPassCtsAck = u4SrtByPassCtsAck;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtInRcpiTh = u4SrtInRcpiTh;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtOutRcpiTh = u4SrtOutRcpiTh;
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtUsedCntTh = u4SrtUsedCntTh;
#ifdef RT_BIG_ENDIAN
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtInRcpiTh
						= cpu2le16(rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtInRcpiTh);
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtOutRcpiTh
						= cpu2le16(rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtOutRcpiTh);
					rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtUsedCntTh
						= cpu2le16(rSrCmdSrNRTCtrl.rSrNRTCtrl.u2SrtUsedCntTh);
#endif
					Status = SrCmdSRUpdateNRTCtrl(pAd, &rSrCmdSrNRTCtrl);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srnrtctrl=[SrtEn]-[SrtSrpEn]-[SrtAddrOrderEn]-[SrtByPassCtsAck]-[SrtInRcpiTh]-[SrtOutRcpiTh]-[SrtUsedCntTh]\n");
		}

	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_FNQ_CTRL_ALL_CTRL **/
NDIS_STATUS SetSrFNQCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4SrpCondDis = 0, u4PeriodOfst = 0, u4HdrDurEn = 0;
	UINT_32 u4TxopDurEn = 0, u4SrpCfendRst = 0, u4SrpNavToutRst = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (IS_SR_V2(pAd)) {
		SR_CMD_SR_FNQ_CTRL_T rSrCmdSrFNQCtrl;

		os_zero_mem(&rSrCmdSrFNQCtrl, sizeof(SR_CMD_SR_FNQ_CTRL_T));

		/* Assign Cmd Id */
		rSrCmdSrFNQCtrl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_FNQ_CTRL_ALL_CTRL;
		rSrCmdSrFNQCtrl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_FNQ_CTRL_ALL_CTRL_ARG_NUM;
		rSrCmdSrFNQCtrl.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrpCondDis = simple_strtol(value, 0, 16);
						Status = IsInRange(u4SrpCondDis, u1ArgNum, 0, BIT16);
						break;
					case 1:
						u4PeriodOfst = simple_strtol(value, 0, 16);
						Status = IsInRange(u4PeriodOfst, u1ArgNum, 0, BIT8);
						break;
					case 2:
						u4HdrDurEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4HdrDurEn, u1ArgNum);
						break;
					case 3:
						u4TxopDurEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4TxopDurEn, u1ArgNum);
						break;
					case 4:
						u4SrpCfendRst = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrpCfendRst, u1ArgNum);
						break;
					case 5:
						u4SrpNavToutRst = simple_strtol(value, 0, 10);
						Status = IsFlag(u4SrpNavToutRst, u1ArgNum);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

			if (u1ArgNum != rSrCmdSrFNQCtrl.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrFNQCtrl.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {

				rSrCmdSrFNQCtrl.rSrFNQCtrl.u2SrpCondDis = u4SrpCondDis;
				rSrCmdSrFNQCtrl.rSrFNQCtrl.u1PeriodOfst = u4PeriodOfst;
				rSrCmdSrFNQCtrl.rSrFNQCtrl.fgHdrDurEn = u4HdrDurEn;
				rSrCmdSrFNQCtrl.rSrFNQCtrl.fgTxopDurEn = u4TxopDurEn;
				rSrCmdSrFNQCtrl.rSrFNQCtrl.fgSrpCfendRst = u4SrpCfendRst;
				rSrCmdSrFNQCtrl.rSrFNQCtrl.fgSrpNavToutRst = u4SrpNavToutRst;

				Status = SrCmdSRUpdateFNQCtrl(pAd, &rSrCmdSrFNQCtrl);
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srfnqctrl=[SrpCondDis]-[PeriodOfst]-[HdrDurEn]-[TxopDurEn]-[SrpCfendRst]-[SrpNavToutRst]\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_FRM_FILT_ALL_CTRL **/
NDIS_STATUS SetSrFrmFiltAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4SrFrmFilt = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (IS_SR_V2(pAd)) {
		SR_CMD_SR_FRM_FILT_T rSrCmdSrFrmFilt;

		os_zero_mem(&rSrCmdSrFrmFilt, sizeof(SR_CMD_SR_FRM_FILT_T));

		/* Assign Cmd Id */
		rSrCmdSrFrmFilt.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_FRM_FILT_ALL_CTRL;
		rSrCmdSrFrmFilt.rSrCmd.u1ArgNum = SR_CMD_SET_SR_FRM_FILT_ALL_CTRL_ARG_NUM;
		rSrCmdSrFrmFilt.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4SrFrmFilt = simple_strtol(value, 0, 16);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;

			if (u1ArgNum != rSrCmdSrFrmFilt.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrFrmFilt.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {

				rSrCmdSrFrmFilt.u4SrFrmFilt = u4SrFrmFilt;

					Status = SrCmdSRUpdateFrmFilt(pAd, &rSrCmdSrFrmFilt);
				}
			} while (0);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Arg is Null\n");
			Status = NDIS_STATUS_FAILURE;
		}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srfrmfilt=[SrFrmFilt]\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}
	return Status;
}

/** SR_CMD_SET_SR_INTERPS_CTRL_ALL_CTRL **/
NDIS_STATUS SetSrInterPsCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4CondDis = 0, u4DurAdj   = 0, u4DurLmt = 0;
	UINT_32 u4EntryEn = 0, u4DurLmtEn = 0, u4InterpsEn = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	if (IS_SR_V2(pAd)) {
		SR_CMD_SR_INTERPS_CTRL_T rSrCmdSrInterPsCtrl;

		os_zero_mem(&rSrCmdSrInterPsCtrl, sizeof(SR_CMD_SR_INTERPS_CTRL_T));

		/* Assign Cmd Id */
		rSrCmdSrInterPsCtrl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_INTERPS_CTRL_ALL_CTRL;
		rSrCmdSrInterPsCtrl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_INTERPS_CTRL_ALL_CTRL_ARG_NUM;
		rSrCmdSrInterPsCtrl.rSrCmd.u1DbdcIdx = u1DbdcIdx;

		if (arg) {
			do {

				/* parameter parsing */
				for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				     value = rstrtok(NULL, "-"), u1ArgNum++) {
				    if (Status == NDIS_STATUS_FAILURE)
						break;
					switch (u1ArgNum) {
					case 0:
						u4CondDis = simple_strtol(value, 0, 16);
						Status = IsInRange(u4CondDis, u1ArgNum, 0, BIT8);
						break;
					case 1:
						u4DurAdj = simple_strtol(value, 0, 16);
						Status = IsInRange(u4DurAdj, u1ArgNum, 0, BIT8);
						break;
					case 2:
						u4DurLmt = simple_strtol(value, 0, 16);
						Status = IsInRange(u4DurLmt, u1ArgNum, 0, BIT8);
						break;
					case 3:
						u4EntryEn = simple_strtol(value, 0, 16);
						Status = IsInRange(u4EntryEn, u1ArgNum, 0, BIT8);
						break;
					case 4:
						u4DurLmtEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4DurLmtEn, u1ArgNum);
						break;
					case 5:
						u4InterpsEn = simple_strtol(value, 0, 10);
						Status = IsFlag(u4InterpsEn, u1ArgNum);
						break;
					default:{
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "set wrong parameters\n");
							Status = NDIS_STATUS_FAILURE;
							break;
						}
					}
				}

				if (Status == NDIS_STATUS_FAILURE)
					break;


			if (u1ArgNum != rSrCmdSrInterPsCtrl.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrInterPsCtrl.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {

				rSrCmdSrInterPsCtrl.rSrInterPsCtrl.u1CondDis = u4CondDis;
				rSrCmdSrInterPsCtrl.rSrInterPsCtrl.u1DurAdj = u4DurAdj;
				rSrCmdSrInterPsCtrl.rSrInterPsCtrl.u1DurLmt = u4DurLmt;
				rSrCmdSrInterPsCtrl.rSrInterPsCtrl.u1EntryEn = u4EntryEn;
				rSrCmdSrInterPsCtrl.rSrInterPsCtrl.fgDurLmtEn = u4DurLmtEn;
				rSrCmdSrInterPsCtrl.rSrInterPsCtrl.fgInterpsEn = u4InterpsEn;

				Status = SrCmdSRUpdateInterPsCtrl(pAd, &rSrCmdSrInterPsCtrl);
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_PRINT("iwpriv ra0 set srinterpsctrl=[u4CondDis]-[DurAdj]-[DurLmt]-[EntryEn]-[DurLmtEn]-[InterpsEn]\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_ENABLE **/
NDIS_STATUS SetSrCfgSrEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgsren=1\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_SD_ENABLE **/
NDIS_STATUS SetSrCfgSrSdEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_SD_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgsrsden=1\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_BF **/
NDIS_STATUS SetSrCfgSrBf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_BF, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgsrbf=1\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_ATF **/
NDIS_STATUS SetSrCfgSrAtf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_ATF, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgsratf=1\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_MODE **/
NDIS_STATUS SetSrCfgSrMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_MODE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgsrmode=1\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_DISRT_ENABLE **/
NDIS_STATUS SetSrCfgDISRTEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_DISRT_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgdisrten=1\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_DISRT_MIN_RSSI **/
NDIS_STATUS SetSrCfgDISRTMinRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_DISRT_MIN_RSSI, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgdisrtmin=62 (Range:0 ~ 110)\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_TXC_QUEUE **/
NDIS_STATUS SetSrCfgTxcQueue(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_TXC_QUEUE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgtxcq=1\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_TXC_QID **/
NDIS_STATUS SetSrCfgTxcQid(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_TXC_QID, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgtxcqid=86\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_TXC_PATH **/
NDIS_STATUS SetSrCfgTxcPath(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_TXC_PATH, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgtxcpath=1\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_AC_METHOD **/
NDIS_STATUS SetSrCfgAcMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_AC_METHOD, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgac=0\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_PERIOD_THR **/
NDIS_STATUS SetSrCfgSrPeriodThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_PERIOD_THR, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgsrperiodthr=480\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_QUERY_TXD_METHOD **/
NDIS_STATUS SetSrCfgQueryTxDMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_QUERY_TXD_METHOD, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgsrquerytxd=1\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_SD_CG_RATIO **/
NDIS_STATUS SetSrCfgSrSdCgRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_SD_CG_RATIO, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgsrsdcg=800\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_SD_OBSS_RATIO **/
NDIS_STATUS SetSrCfgSrSdObssRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_SD_OBSS_RATIO, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgsrsdobss=500\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_PROFILE **/
NDIS_STATUS SetSrCfgProfile(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_PROFILE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgsrprofile=3\n"
			 "SR_PROFILE_QUERY_TXD_TIME           BIT(0)\n"
			 "SR_PROFILE_SHOW_Q_LEN               BIT(1)\n"
			 "SR_PROFILE_RPT_HANDLE_TIME          BIT(2)\n"
			 "SR_PROFILE_GEN_TXC_TIME             BIT(3)\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_FNQ_ENABLE **/
NDIS_STATUS SetSrCfgFnqEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	if (IS_SR_V2(pAd)) {
		Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_FNQ_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_PRINT("iwpriv ra0 set srcfgfnqen=1\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_DPD_ENABLE **/
NDIS_STATUS SetSrCfgDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_DPD_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgdpden=1\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_TX_ENABLE **/
NDIS_STATUS SetSrCfgSrTxEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_TX_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgsrtxen=1\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_SD_OM_ENABLE **/
NDIS_STATUS SetSrCfgObssMonitorEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_SD_OM_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgomen=1\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_TX_ALIGN_ENABLE **/
NDIS_STATUS SetSrCfgSrTxAlignEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_TX_ALIGN_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgsrtxalignen=1\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_TX_ALIGN_RSSI_THR **/
NDIS_STATUS SetSrCfgSrTxAlignRssiThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_TX_ALIGN_RSSI_THR, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgsrtxalignrssi=40\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_DABS_MODE **/
NDIS_STATUS SetSrCfgDabsMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_DABS_MODE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srcfgdabsmode=1\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_DROP_MIN_MCS **/
NDIS_STATUS SetSrCfgDropMinMCS(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_DROP_MIN_MCS, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS)
		MTWF_PRINT("iwpriv ra0 set srcfgdropminmcs=1\n");

	return Status;
}

/*SR_CMD_SET_SR_CFG_SR_DPD_THRESHOLD */
NDIS_STATUS SetSrCfgSrDPDThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_DPD_THRESHOLD, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS)
		MTWF_PRINT("iwpriv ra0 set srcfgdabsmode=1\n");

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_ENABLE **/
NDIS_STATUS SetSrMeshSDFlag(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_MESH_SR_SD_CTRL, SR_CMD_SET_DEFAULT_ARG_NUM);

	return Status ? FALSE : TRUE;
}


/** SR_CMD_SET_SR_SRG_BITMAP **/
NDIS_STATUS SetSrSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	UINT_8 u1DbdcIdx = BAND0;
	UINT_32 u4Color_31_0 = 0, u4Color_63_32 = 0, u4pBssid_31_0 = 0, u4pBssid_63_32 = 0;

	SR_CMD_SR_SRG_BITMAP_T rSrCmdSrSrgBitmap;


	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&rSrCmdSrSrgBitmap, sizeof(SR_CMD_SR_SRG_BITMAP_T));

	/* Assign Cmd Id */
	rSrCmdSrSrgBitmap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_SRG_BITMAP;
	rSrCmdSrSrgBitmap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_SRG_BITMAP_ARG_NUM;
	rSrCmdSrSrgBitmap.rSrCmd.u1DbdcIdx = u1DbdcIdx;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					u4Color_31_0 = simple_strtol(value, 0, 16);
					break;
				case 1:
					u4Color_63_32 = simple_strtol(value, 0, 16);
					break;
				case 2:
					u4pBssid_31_0 = simple_strtol(value, 0, 16);
					break;
				case 3:
					u4pBssid_63_32 = simple_strtol(value, 0, 16);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmdSrSrgBitmap.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 " Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrSrgBitmap.rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {
				rSrCmdSrSrgBitmap.rSrSrgBitmap.u4Color_31_0[u1DbdcIdx] = u4Color_31_0;
				rSrCmdSrSrgBitmap.rSrSrgBitmap.u4Color_63_32[u1DbdcIdx] = u4Color_63_32;
				rSrCmdSrSrgBitmap.rSrSrgBitmap.u4pBssid_31_0[u1DbdcIdx] = u4pBssid_31_0;
				rSrCmdSrSrgBitmap.rSrSrgBitmap.u4pBssid_63_32[u1DbdcIdx] = u4pBssid_63_32;

				Status = SrCmdSRUpdateSrgBitmap(pAd, &rSrCmdSrSrgBitmap);
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_PRINT("iwpriv ra0 set srsrgbm=[Color_31_0]-[Color_63_32]-[pBssid_31_0]-[pBssid_63_32]\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_SRG_BITMAP_REFRESH **/
NDIS_STATUS SetSrSrgBitmapRefresh(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_SRG_BITMAP_REFRESH, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 set srsrgbmrefresh=0\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_MESH_SRG_BITMAP **/
INT SetSrMeshSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	UINT_8 u1DbdcIdx = BAND0;
	UINT_32 u4Color_31_0 = 0, u4Color_63_32 = 0, u4pBssid_31_0 = 0, u4pBssid_63_32 = 0;

	SR_CMD_SR_SRG_BITMAP_T rSrCmdSrMeshSrgBitmap;


	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	os_zero_mem(&rSrCmdSrMeshSrgBitmap, sizeof(SR_CMD_SR_SRG_BITMAP_T));

	/* Assign Cmd Id */
	rSrCmdSrMeshSrgBitmap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_MESH_SRG_BITMAP;
	rSrCmdSrMeshSrgBitmap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_MESH_SRG_BITMAP_ARG_NUM;
	rSrCmdSrMeshSrgBitmap.rSrCmd.u1DbdcIdx = u1DbdcIdx;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-");
				value; value = rstrtok(NULL, "-"), u1ArgNum++) {

				if (Status == FALSE)
					break;

				switch (u1ArgNum) {

				case 0:
					u4Color_31_0 = simple_strtol(value, 0, 16);
					break;

				case 1:
					u4Color_63_32 = simple_strtol(value, 0, 16);
					break;

				case 2:
					u4pBssid_31_0 = simple_strtol(value, 0, 16);
					break;

				case 3:
					u4pBssid_63_32 = simple_strtol(value, 0, 16);
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != rSrCmdSrMeshSrgBitmap.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					" Format Error! ArgNum = %d != %d\n",
					u1ArgNum, rSrCmdSrMeshSrgBitmap.rSrCmd.u1ArgNum);
				Status = FALSE;
				break;
			} else {
				rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4Color_31_0[u1DbdcIdx] = u4Color_31_0;
				rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4Color_63_32[u1DbdcIdx] = u4Color_63_32;
				rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4pBssid_31_0[u1DbdcIdx] = u4pBssid_31_0;
				rSrCmdSrMeshSrgBitmap.rSrSrgBitmap.u4pBssid_63_32[u1DbdcIdx] = u4pBssid_63_32;

				if (SrCmdSRUpdateSrgBitmap(pAd, &rSrCmdSrMeshSrgBitmap) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT("iwpriv ra0 set srmeshsrgbm=[Color_31_0]-[Color_63_32]-[pBssid_31_0]-[pBssid_63_32]\n");
	}

	return Status;
}

/** SR_CMD_SET_SR_SIGA_FLAG_CTRL **/
INT SetSrSiga(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0;
	CHAR *value;
	UINT_32 u4readvalue;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	UINT_8 u1DbdcIdx = BAND0, u1Bssid, u1SigaFlag = 0;

	SR_CMD_SR_SIGA_FLAG_T rSrCmdSrSigaFlag;


	if (wdev != NULL) {
		u1DbdcIdx = HcGetBandByWdev(wdev);
		u1Bssid = wdev->DevInfo.OwnMacIdx > 3 ? wdev->DevInfo.OwnMacIdx - SR_BSSID_OMAC_OFFSET : wdev->DevInfo.OwnMacIdx;
	} else
		return FALSE;

	os_zero_mem(&rSrCmdSrSigaFlag, sizeof(SR_CMD_SR_SIGA_FLAG_T));

	/* Assign Cmd Id */
	rSrCmdSrSigaFlag.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_SIGA_FLAG_CTRL;
	rSrCmdSrSigaFlag.rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmdSrSigaFlag.rSrCmd.u1DbdcIdx = u1DbdcIdx;

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-");
				value; value = rstrtok(NULL, "-"), u1ArgNum++) {
				u4readvalue = simple_strtol(value, 0, 10);

				if (Status == FALSE)
					break;

				switch (u1ArgNum) {

				case 0:
					if (IsInRange(u4readvalue, u1ArgNum, 0, SR_SIGA_FLAG_RANGE) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					u1SigaFlag = u4readvalue;
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != rSrCmdSrSigaFlag.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, rSrCmdSrSigaFlag.rSrCmd.u1ArgNum);
				Status = FALSE;
			} else {
				rSrCmdSrSigaFlag.rSrSigaFlag.u1Bssid = u1Bssid;
				rSrCmdSrSigaFlag.rSrSigaFlag.u1SigaFlag[u1Bssid] = u1SigaFlag;
				if (SrCmdSRUpdateSiga(pAd, &rSrCmdSrSigaFlag) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT("iwpriv ra0 set srsrgbm=[BssIDindx][SRSiga Flag]\n");
	}

	return Status;
}

/** SR_CMD_SET_MESH_SR_SD_CTRL **/
NDIS_STATUS SrMeshApcliDetect(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = TRUE;// TRUE =
	UCHAR ucinputmac[MAC_ADDR_LEN];
	UINT_8 u1ArgNum = 0;
	struct sr_mesh_topology_params *prSrMeshTopologyParams = &g_rTopologyUpdate;

	if (arg) {
		if (strlen(arg) != SR_SCENE_DETECTION_MAC_LENGTH)
			return FALSE;
		do {
			for (u1ArgNum = 0; u1ArgNum < MAC_ADDR_LEN; u1ArgNum++) {
				AtoH(arg, &ucinputmac[u1ArgNum], 1);
				arg = arg + 3;
			}

			if (IS_EQUAL_MAC(ucinputmac, prSrMeshTopologyParams->map_remote_bh_mac))
				fgmeshdetect &= 1;
			else
				fgmeshdetect &= 0;

			return Status;
		} while (0);
	} else {

		Status = FALSE;
	}

	return Status;
}

/** SR_CMD_SET_MESH_SR_SD_CTRL **/
NDIS_STATUS SetMeshMac(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = TRUE;// TRUE = 1
	UINT_8 u1ArgNum = 0;
	struct sr_mesh_topology_params *prSrMeshTopologyParams = &g_rTopologyUpdate;

	if (strlen(arg) != SR_SCENE_DETECTION_MAC_LENGTH)
		return FALSE;


	for (u1ArgNum = 0; u1ArgNum < MAC_ADDR_LEN; u1ArgNum++) {
		AtoH(arg, &prSrMeshTopologyParams->map_remote_bh_mac[u1ArgNum], 1);
		arg = arg + 3;
	}

	return Status;
}

NDIS_STATUS SetMeshSRsd(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = TRUE;

	RTMP_STRING MeshSdFlag[2] = "0";
	RTMP_STRING SCSEnable[2] = "0";

	if (arg) {
		if (fghavebeensend != fgmeshdetect) {
			fghavebeensend = fgmeshdetect;
			if (fgmeshdetect) {
				strncpy(MeshSdFlag, "0", 1); //There is no unknown apcli
				strncpy(SCSEnable, "1", 1);  //scs on
			} else {
				strncpy(MeshSdFlag, "1", 1); //There is a unknown apcli
				strncpy(SCSEnable, "0", 1);  //scs off

			}
			fgmeshdetect = 1;
			MeshSdFlag[1] = '\0';
			SCSEnable[1] = '\0';
		} else {
			/*No need to send fw command*/
			fgmeshdetect = 1;
			return Status;
		}

		Status = SetSrMeshSDFlag(pAd, MeshSdFlag);


		return Status;
	} else
		return FALSE;

}

/** SR_CMD_SET_SR_SIGA_AUTO_FLAG_CTRL **/
INT SetSrSigaAuto(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)

{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0;
	CHAR *value;
	UINT_32 u4readvalue;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	UINT_8 u1DbdcIdx = BAND0;
	struct _SR_CMD_SR_SIGA_AUTO_FLAG_T rSrCmdSrSigaAutoFlag;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	os_zero_mem(&rSrCmdSrSigaAutoFlag, sizeof(rSrCmdSrSigaAutoFlag));

	/* Assign Cmd Id */
	rSrCmdSrSigaAutoFlag.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_SIGA_AUTO_FLAG_CTRL;
	rSrCmdSrSigaAutoFlag.rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmdSrSigaAutoFlag.rSrCmd.u1DbdcIdx = u1DbdcIdx;

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				 value = rstrtok(NULL, "-"), u1ArgNum++) {
				u4readvalue = simple_strtol(value, 0, 10);
				if (Status == FALSE)
					break;
				switch (u1ArgNum) {
				case 0:
					if (IsInRange(u4readvalue, u1ArgNum, 0, 1) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					rSrCmdSrSigaAutoFlag.rSrSigaAutoFlag.u1SrSigaAutoFlag = u4readvalue;
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != rSrCmdSrSigaAutoFlag.rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmdSrSigaAutoFlag.rSrCmd.u1ArgNum);
				Status = FALSE;
			} else {
				if (SrCmdSRUpdateSigaAuto(pAd, &rSrCmdSrSigaAutoFlag) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT(" iwpriv ra0 set srsigaauto=[SRSigaAuto_Flag]\n");
	}

	return Status;
}

/** Driver Internal **/
INT SetSrSelfSrgInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0, u1Mode;
	CHAR *value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	UINT_32 u4Color_31_0 = 0, u4Color_63_32 = 0, u4pBssid_31_0 = 0, u4pBssid_63_32 = 0;
	struct _SR_MESH_SRG_BITMAP_T *prSrgBitmap = NULL;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-");
				value; value = rstrtok(NULL, "-"), u1ArgNum++) {

				if (Status == FALSE)
					break;

				switch (u1ArgNum) {

				case 0:
					u1Mode = simple_strtol(value, 0, 10);

					if (IsInRange(u1Mode, u1ArgNum, 0, 1) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;

				case 1:
					u4Color_31_0 = simple_strtol(value, 0, 16);
					break;

				case 2:
					u4Color_63_32 = simple_strtol(value, 0, 16);
					break;

				case 3:
					u4pBssid_31_0 = simple_strtol(value, 0, 16);
					break;

				case 4:
					u4pBssid_63_32 = simple_strtol(value, 0, 16);
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_SR_SELF_SRG_INFO_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, SR_CMD_SET_SR_SELF_SRG_INFO_ARG_NUM);
				Status = FALSE;
				break;
			}

			if (u1Mode == 1) {
				_u1SrSelfSrgBMMode[u1DbdcIdx] = ENUM_SR_SELF_BM_MANUAL;

				prSrgBitmap = &rSrSelfSrgBMMan[u1DbdcIdx];
				prSrgBitmap->u4Color_31_0 = u4Color_31_0;
				prSrgBitmap->u4Color_63_32 = u4Color_63_32;
				prSrgBitmap->u4pBssid_31_0 = u4pBssid_31_0;
				prSrgBitmap->u4pBssid_63_32 = u4pBssid_63_32;

				SrMeshSelfSrgInfoEvent(pAd, u1DbdcIdx);
			} else
				_u1SrSelfSrgBMMode[u1DbdcIdx] = ENUM_SR_SELF_BM_AUTO;
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT(" iwpriv ra0 set srselfsrginfo=[Mode]-[Color_31_0]-[Color_63_32]-[pBssid_31_0]-[pBssid_63_32]\n");
	}
	if (prSrgBitmap)
		MTWF_PRINT("BandIdx:%u, Mode:%u BSS_Color_BM:[63:32][0x%x]-[31:0][0x%x], Par_Bssid_BM:[63:32][0x%x]-[31:0][0x%x]\n",
			u1DbdcIdx, _u1SrSelfSrgBMMode[u1DbdcIdx], prSrgBitmap->u4Color_63_32, prSrgBitmap->u4Color_31_0,
			prSrgBitmap->u4pBssid_63_32, prSrgBitmap->u4pBssid_31_0);

	return Status;
}

/** Driver Internal **/
INT SetSrMeshTopoLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	CHAR *value;
	UINT8 u1ArgNum = 0, u1TopoLock = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 u1DbdcIdx = BAND0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				 value = rstrtok(NULL, "-"), u1ArgNum++) {
				if (Status == FALSE)
					break;
				switch (u1ArgNum) {
				case 0:
					u1TopoLock = simple_strtol(value, 0, 10);
					if (IsInRange(u1TopoLock, u1ArgNum, 0, 1) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_DEFAULT_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, SR_CMD_SET_DEFAULT_ARG_NUM);
				Status = FALSE;
			} else {
				_u1SrMeshTopoLock[u1DbdcIdx] = u1TopoLock;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	MTWF_PRINT("BandIdx:%u, _u1SrMeshTopoLock:%u\n",
		 u1DbdcIdx, _u1SrMeshTopoLock[u1DbdcIdx]);

	return Status;
}

/** SR_CMD_SET_REMOTE_FH_RSSI_TH **/
INT SetSrMeshRemoteFhRssiTh(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	INT iSrMeshFhRssiTh = 0, iSrMeshSrAllowTargetTh = 0;
	UINT32 rv;
	struct _SR_CMD_MESH_FH_RSSI_TH_T rSrCmdSrFhRssiTh;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	os_zero_mem(&rSrCmdSrFhRssiTh, sizeof(rSrCmdSrFhRssiTh));

	/* Assign Cmd Id */
	rSrCmdSrFhRssiTh.rSrCmd.u1CmdSubId = SR_CMD_SET_MESH_FH_RSSI_TH;
	rSrCmdSrFhRssiTh.rSrCmd.u1ArgNum = SR_CMD_SET_SR_MESH_FH_RSSI_TH_ARG_NUM;
	rSrCmdSrFhRssiTh.rSrCmd.u1DbdcIdx = u1DbdcIdx;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &iSrMeshFhRssiTh, &iSrMeshSrAllowTargetTh);

		if (rv > 1) {
			rSrCmdSrFhRssiTh.rSrMeshFhRssiTh.i1SrMeshFhRssiTh = (INT_8)iSrMeshFhRssiTh;
			rSrCmdSrFhRssiTh.rSrMeshFhRssiTh.i1SrMeshSrAllowTargetTh = (INT_8)iSrMeshSrAllowTargetTh;
			if (SrCmdSetFhRssiTh(pAd, &rSrCmdSrFhRssiTh) != NDIS_STATUS_SUCCESS)
				Status = FALSE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"set wrong parameters\n");
			Status = FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT(" iwpriv ra0 set srmeshremotefhrssiTh=[SR_FHRSSI_Th]-[SR_ALLOW_TARGET_TH]\n");
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Status:%d\n", Status);

	return Status;
}

/** SR_CMD_SET_DOWNLINK_STA_THRESHOLD **/
INT SetSrMeshStaThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	INT idevidx = 0, iSrMeshSrRssi = 0;
	UINT32 rv;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &idevidx, &iSrMeshSrRssi);

		if (rv > 1) {
			if (SRMeshLinkSTAThreshold(pAd, (UINT_8)idevidx, (INT_8)iSrMeshSrRssi) != NDIS_STATUS_SUCCESS)
				Status = FALSE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"set wrong parameters\n");
			Status = FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT(" iwpriv ra0 set srmeshstath=[WDEV_IDX]-[RSSI]\n");
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Status:%d\n", Status);

	return Status;
}

/** SR_CMD_SET_BH_MESH_SR_BITMAP **/
INT SetSrMeshBHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0;
	CHAR *value;
	UINT_32 u4Color_31_0 = 0, u4Color_63_32 = 0, u4pBssid_31_0 = 0, u4pBssid_63_32 = 0;
	SR_MESH_SRG_BITMAP_T rSrSrgBitmap;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-");
				value; value = rstrtok(NULL, "-"), u1ArgNum++) {

				if (Status == FALSE)
					break;

				switch (u1ArgNum) {

				case 0:
					u4Color_31_0 = simple_strtol(value, 0, 16);
					break;

				case 1:
					u4Color_63_32 = simple_strtol(value, 0, 16);
					break;

				case 2:
					u4pBssid_31_0 = simple_strtol(value, 0, 16);
					break;

				case 3:
					u4pBssid_63_32 = simple_strtol(value, 0, 16);
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_SR_MESH_FH_SRG_BITMAP_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, SR_CMD_SET_SR_MESH_FH_SRG_BITMAP_ARG_NUM);
				Status = FALSE;
				break;
			}

			os_zero_mem(&rSrSrgBitmap, sizeof(rSrSrgBitmap));

			/* Update Color & pBssid Bitmap */
			rSrSrgBitmap.u4Color_31_0 = u4Color_31_0;
			rSrSrgBitmap.u4Color_63_32 = u4Color_63_32;
			rSrSrgBitmap.u4pBssid_31_0 = u4pBssid_31_0;
			rSrSrgBitmap.u4pBssid_63_32 = u4pBssid_63_32;

			if (SrMultiAPBhMeshSrgBitmap(pAd, TRUE, (PUINT_8)&rSrSrgBitmap) != NDIS_STATUS_SUCCESS)
				Status = FALSE;
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT(" iwpriv ra0 set srmeshbhsrgbm=[Color_31_0]-[Color_63_32]-[pBssid_31_0]-[pBssid_63_32]\n");
	}

	return Status;
}

/** SR_CMD_SET_FH_MESH_SR_BITMAP **/
INT SetSrMeshFHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0;
	CHAR *value;
	UINT_32 u4Color_31_0 = 0, u4Color_63_32 = 0, u4pBssid_31_0 = 0, u4pBssid_63_32 = 0;
	SR_MESH_SRG_BITMAP_T rSrSrgBitmap;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-");
				value; value = rstrtok(NULL, "-"), u1ArgNum++) {

				if (Status == FALSE)
					break;

				switch (u1ArgNum) {

				case 0:
					u4Color_31_0 = simple_strtol(value, 0, 16);
					break;

				case 1:
					u4Color_63_32 = simple_strtol(value, 0, 16);
					break;

				case 2:
					u4pBssid_31_0 = simple_strtol(value, 0, 16);
					break;

				case 3:
					u4pBssid_63_32 = simple_strtol(value, 0, 16);
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_SR_MESH_FH_SRG_BITMAP_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, SR_CMD_SET_SR_MESH_FH_SRG_BITMAP_ARG_NUM);
				Status = FALSE;
				break;
			}

			os_zero_mem(&rSrSrgBitmap, sizeof(rSrSrgBitmap));

			/* Update Color & pBssid Bitmap */
			rSrSrgBitmap.u4Color_31_0 = u4Color_31_0;
			rSrSrgBitmap.u4Color_63_32 = u4Color_63_32;
			rSrSrgBitmap.u4pBssid_31_0 = u4pBssid_31_0;
			rSrSrgBitmap.u4pBssid_63_32 = u4pBssid_63_32;

			if (SrMultiAPFhMeshSrgBitmap(pAd, TRUE, (PUINT_8)&rSrSrgBitmap) != NDIS_STATUS_SUCCESS)
				Status = FALSE;
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT(" iwpriv ra0 set srmeshfhsrgbm=[Color_31_0]-[Color_63_32]-[pBssid_31_0]-[pBssid_63_32]\n");
	}

	return Status;
}

/** SR_CMD_SET_BH_DOWNLINK_MESH_SR_THRESHOLD **/
INT SetSrMeshBHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	INT_32 i4Rssi = 0;
	UINT_32 rv;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		rv = sscanf(arg, "%d", &i4Rssi);

		if (rv > 0) {

			if (SrBHDownMeshSRThreshold(pAd, (INT_8)i4Rssi) != NDIS_STATUS_SUCCESS)
				Status = FALSE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"set %d\n", i4Rssi);
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"set wrong parameters\n");
			Status = FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT(" iwpriv ra0 set srmeshbhdlobsspdth=[RSSI]\n");
	}

	return Status;
}

/** SR_CMD_SET_FH_DOWNLINK_MESH_SR_THRESHOLD **/
INT SetSrMeshFHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	INT_32 i4Rssi = 0;
	UINT_32 rv;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		rv = sscanf(arg, "%d", &i4Rssi);

		if (rv > 0) {

			if (SrFHDownMeshSRThreshold(pAd, (INT_8)i4Rssi) != NDIS_STATUS_SUCCESS)
				Status = FALSE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"set %d\n", i4Rssi);
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"set wrong parameters\n");
			Status = FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT(" iwpriv ra0 set srmeshfhdlobsspdth=[RSSI]\n");
	}

	return Status;
}

/** SR_CMD_SET_FORHIB_MESH_SR **/
INT SetSrMeshBHDownLinkForbidSR(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	INT idevidx = 0;
	UINT_32 rv;

	if (arg) {
		rv = sscanf(arg, "%d", &idevidx);

		if (rv > 0) {
			if (SrMeshForbidSrBssid(pAd, (UINT_8)idevidx) != NDIS_STATUS_SUCCESS)
				Status = FALSE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"set wrong parameters\n");
			Status = FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT(" iwpriv ra0 set srmeshforbitsr=[WdevIdx]\n");
	}

	return Status;
}

/** SR_CMD_SET_FORHIB_MESH_SR_RESET **/
INT SetSrMeshResetBhDLForbidSR(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	INT input = 0;
	UINT_32 rv;
	UINT_8 u1DbdcIdx = BAND0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {

		/*Input can be any number*/
		rv = sscanf(arg, "%d", &input);

		if (rv > 0) {
			MTWF_PRINT("BandIdx = %d\n", u1DbdcIdx);

			if (SrCmdFlushMeshForbidSrBssid(pAd, u1DbdcIdx) != NDIS_STATUS_SUCCESS)
				Status = FALSE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"set wrong parameters\n");
			Status = FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT(" iwpriv ra0 set srmeshflushforbitsr=[X]\n");
	}

	return Status;
}

/** SR_CMD_SET_REMOTE_FH_RSSI **/
INT SetSrMeshRemoteFhRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	INT value;
	INT_8 Rssi;
	UINT32 rv;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (_u1SrMeshTopoLock[u1DbdcIdx] == ENUM_SR_TOPO_LOCK_AUTO) {
		MTWF_PRINT("_u1SrMeshTopoLock:%d Auto for band:%d\n",
				_u1SrMeshTopoLock[u1DbdcIdx], u1DbdcIdx);
		return FALSE;
	}

	if (arg) {
		rv = sscanf(arg, "%d", &value);

		if (rv > 0) {
			Rssi = (INT_8)value;
			if (SrSetRemoteFHRssi(pAd, u1DbdcIdx, Rssi, ENUM_RMT_FH_SCAN_SUCCESS) != NDIS_STATUS_SUCCESS)
				Status = FALSE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"set %d\n", Rssi);
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"set wrong parameters\n");
			Status = FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT(" iwpriv ra0 set srmeshremotefhrssi=[Rssi]\n");
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Status:%d\n", Status);

	return Status;
}

/** SR_CMD_SET_REMOTE_FH_RSSI **/
NDIS_STATUS SrSetRemoteFHRssi(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, INT_8 Rssi, UINT_8 u1RemoteFhStat)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	struct _SR_CMD_MESH_TOPOLOGY_T rSrCmdMeshTopology;

	os_zero_mem(&rSrCmdMeshTopology, sizeof(rSrCmdMeshTopology));

	/* Assign Cmd Id */
	rSrCmdMeshTopology.rSrCmd.u1CmdSubId = SR_CMD_SET_REMOTE_FH_RSSI;
	rSrCmdMeshTopology.rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmdMeshTopology.rSrCmd.u1DbdcIdx = u1DbdcIdx;

	rSrCmdMeshTopology.rSrCmdMeshTopo.rRemoteFhParams.i1Rssi = Rssi;
	rSrCmdMeshTopology.rSrCmdMeshTopo.rRemoteFhParams.u1RemoteFhStat = u1RemoteFhStat;
	Status = SrCmdMeshTopologyUpd(pAd, &rSrCmdMeshTopology);

	return Status;
}

/** SR_CMD_SET_REMOTE_BH_INFO **/
INT SetSrMeshRemoteBhInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0;
	CHAR *value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	UINT_8 BhType;
	UINT_16 Wcid;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (_u1SrMeshTopoLock[u1DbdcIdx] == ENUM_SR_TOPO_LOCK_AUTO) {
		MTWF_PRINT("_u1SrMeshTopoLock:%d Auto for band:%d\n",
				_u1SrMeshTopoLock[u1DbdcIdx], u1DbdcIdx);
		return FALSE;
	}

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				value = rstrtok(NULL, "-"), u1ArgNum++) {

				if (Status == FALSE)
					break;

				switch (u1ArgNum) {

				case 0:
					BhType = simple_strtol(value, 0, 10);
					if (IsInRange(BhType, u1ArgNum, ENUM_BH_TYPE_NO_WIFI,
						ENUM_BH_TYPE_WIFI) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;

				case 1:
					Wcid = simple_strtol(value, 0, 10);
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_SR_REMOTE_BH_INFO_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_SR_REMOTE_BH_INFO_ARG_NUM);
				Status = FALSE;
			} else {
				if (SrSetRemoteAssocBHInfo(pAd, u1DbdcIdx, BhType, Wcid) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT(" iwpriv ra0 set srmeshremotebhinfo=[BhType]-[Wcid]\n");
	}

	return Status;
}

/** SR_CMD_SET_REMOTE_BH_INFO **/
NDIS_STATUS SrSetRemoteAssocBHInfo(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, UINT_8 BhType, UINT_16 Wcid)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	struct _SR_CMD_MESH_TOPOLOGY_T rSrCmdMeshTopology;

	os_zero_mem(&rSrCmdMeshTopology, sizeof(rSrCmdMeshTopology));

	/* Assign Cmd Id */
	rSrCmdMeshTopology.rSrCmd.u1CmdSubId = SR_CMD_SET_REMOTE_BH_INFO;
	rSrCmdMeshTopology.rSrCmd.u1ArgNum = SR_CMD_SET_SR_REMOTE_BH_INFO_ARG_NUM;
	rSrCmdMeshTopology.rSrCmd.u1DbdcIdx = u1DbdcIdx;

	rSrCmdMeshTopology.rSrCmdMeshTopo.rRemoteBhParams.u2RemoteBhWcid = Wcid;
	rSrCmdMeshTopology.rSrCmdMeshTopo.rRemoteBhParams.u1RemoteBhType = BhType;
	Status = SrCmdMeshTopologyUpd(pAd, &rSrCmdMeshTopology);

	return Status;
}

/** SR_CMD_SET_MAP_BALANCE **/
INT SetSrMeshMapBalance(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	INT value;
	UINT8 u1MapBalance;
	UINT32 rv;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		rv = sscanf(arg, "%u", &value);

		if (rv > 0) {
			u1MapBalance = (UINT8)value;
			if (SrSetMapBalance(pAd, u1DbdcIdx, u1MapBalance) != NDIS_STATUS_SUCCESS)
				Status = FALSE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"set %d\n", u1MapBalance);
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"set wrong parameters\n");
			Status = FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT(" iwpriv ra0 set srmapbalance=[Map_Balance]\n");
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Status:%d\n", Status);

	return Status;
}

/** SR_CMD_SET_MAP_TOPO **/
INT SetSrMeshTopo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	UINT_8 u1ArgNum = 0;
	CHAR *value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	UINT_8 MapDevCount = 0, MapDevSrSupportMode = 0, SelfRole = 0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (_u1SrMeshTopoLock[u1DbdcIdx] == ENUM_SR_TOPO_LOCK_AUTO) {
		MTWF_PRINT("_u1SrMeshTopoLock:%u Auto for band:%u\n",
			_u1SrMeshTopoLock[u1DbdcIdx], u1DbdcIdx);
		return FALSE;
	}

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				value = rstrtok(NULL, "-"), u1ArgNum++) {

				if (Status == FALSE)
					break;

				switch (u1ArgNum) {

				case 0:
					MapDevCount = simple_strtol(value, 0, 10);
					break;

				case 1:
					MapDevSrSupportMode = simple_strtol(value, 0, 10);
					if (IsInRange(MapDevSrSupportMode, u1ArgNum, 0, 2) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;

				case 2:
					SelfRole = simple_strtol(value, 0, 10);
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_SR_MAP_TOPOLOGY_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_SR_MAP_TOPOLOGY_ARG_NUM);
				Status = FALSE;
			} else {
				if (SrSetMAPTopo(pAd, u1DbdcIdx, MapDevCount, MapDevSrSupportMode, SelfRole) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT(" iwpriv ra0 set srmeshtopo=[MapDevCount]-[MapMtkMeshSrOnly]-[SelfRole]\n");
	}

	return Status;
}

/** SR_CMD_SET_MAP_TOPO **/
NDIS_STATUS SrSetMAPTopo(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, UINT_8 MapDevCount, UINT_8 MapDevSrSupportMode, UINT_8 SelfRole)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	struct _SR_CMD_MESH_TOPOLOGY_T rSrCmdMeshTopology;

	os_zero_mem(&rSrCmdMeshTopology, sizeof(rSrCmdMeshTopology));

	/* Assign Cmd Id */
	rSrCmdMeshTopology.rSrCmd.u1CmdSubId = SR_CMD_SET_MAP_TOPO;
	rSrCmdMeshTopology.rSrCmd.u1ArgNum = SR_CMD_SET_SR_MAP_TOPOLOGY_ARG_NUM;
	rSrCmdMeshTopology.rSrCmd.u1DbdcIdx = u1DbdcIdx;

	rSrCmdMeshTopology.rSrCmdMeshTopo.rMapTopoParams.u1MapDevCount = MapDevCount;
	rSrCmdMeshTopology.rSrCmdMeshTopo.rMapTopoParams.u1MapDevSrSupportMode = MapDevSrSupportMode;
	rSrCmdMeshTopology.rSrCmdMeshTopo.rMapTopoParams.u1SelfRole = SelfRole;
	Status = SrCmdMeshTopologyUpd(pAd, &rSrCmdMeshTopology);

	return Status;
}

/** SR_CMD_SET_MAP_TRAFFIC_STATUS **/
INT SetSrMeshUplinkEvent(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	CHAR *value;
	UINT8 u1ArgNum = 0, u1UlStatus = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 u1DbdcIdx = BAND0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				value = rstrtok(NULL, "-"), u1ArgNum++) {

				if (Status == FALSE)
					break;

				switch (u1ArgNum) {

				case 0:
					u1UlStatus = simple_strtol(value, 0, 10);
					if (IsInRange(u1UlStatus, u1ArgNum, 0, 1) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_DEFAULT_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_DEFAULT_ARG_NUM);
				Status = FALSE;
			} else {
				if (SrSetUplinkTrafficStatus(pAd, u1DbdcIdx, u1UlStatus) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	return Status;
}

/** SR_CMD_SET_MESH_UL_MODE **/
INT SetSrMeshUlMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	INT Status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;
	INT value;
	UINT8 u1UlMode;
	UINT32 rv;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		rv = sscanf(arg, "%u", &value);

		if (rv > 0) {
			u1UlMode = (UINT8)value;
			if (SrCmdSetMeshUlMode(pAd, u1DbdcIdx, u1UlMode) != NDIS_STATUS_SUCCESS)
				Status = FALSE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"set %d\n", u1UlMode);
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"set wrong parameters\n");
			Status = FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	if (Status == FALSE) {
		MTWF_PRINT(" iwpriv ra0 set srmeshulmode=[u1UlMode]\n");
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Status:%d\n", Status);

	return Status;
}

NDIS_STATUS SrSetUplinkTrafficStatus(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 BandIdx,
	IN UINT8 UlStatus
)
{
	struct _SR_CMD_SR_UL_TRAFFIC_STATUS_T rSrCmdSrUlStatus;

	os_zero_mem(&rSrCmdSrUlStatus, sizeof(rSrCmdSrUlStatus));

	/* Assign Cmd Id */
	rSrCmdSrUlStatus.rSrCmd.u1CmdSubId = SR_CMD_SET_MAP_TRAFFIC_STATUS;
	rSrCmdSrUlStatus.rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmdSrUlStatus.rSrCmd.u1DbdcIdx = BandIdx;
	rSrCmdSrUlStatus.rSrUlStatus.u1UlStatus = UlStatus;

	return SrCmdMeshUplinkStatusSet(pAd, &rSrCmdSrUlStatus);
}

NDIS_STATUS SrCmdMeshUplinkStatusSet(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SR_UL_TRAFFIC_STATUS_T *prSrCmdSrUlStatus)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrSigaFlag not null */
	if (!prSrCmdSrUlStatus) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrUlStatus */
	/* PrintSrCmdSrUlStatus(pAd, prSrCmdSrUlStatus); */

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(*prSrCmdSrUlStatus));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrUlStatus, sizeof(*prSrCmdSrUlStatus));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrSetMapBalance(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 Value)
{
	struct _SR_CMD_SET_MAP_BALANCE_T rSrCmdMapBalance;

	os_zero_mem(&rSrCmdMapBalance, sizeof(rSrCmdMapBalance));

	/* Assign Cmd Id */
	rSrCmdMapBalance.rSrCmd.u1CmdSubId = SR_CMD_SET_MAP_BALANCE;
	rSrCmdMapBalance.rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmdMapBalance.rSrCmd.u1DbdcIdx = BandIdx;
	rSrCmdMapBalance.rSrMapBalance.u1MapBalance = Value;

	return SrCmdMeshMapBalanceSet(pAd, &rSrCmdMapBalance);
}

NDIS_STATUS SrCmdMeshMapBalanceSet(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SET_MAP_BALANCE_T *prSrCmdMapBalance)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdMapBalance not null */
	if (!prSrCmdMapBalance) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdMapBalance */
	/* PrintSrCmdSrMapBalance(pAd, prSrCmdMapBalance); */

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(*prSrCmdMapBalance));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdMapBalance, sizeof(*prSrCmdMapBalance));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSetMeshUlMode(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 Value)
{
	struct _SR_CMD_SET_MESH_UL_MODE_T rSrCmdMeshUlMode;
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	if (BandIdx >= RAM_BAND_NUM)
		return NDIS_STATUS_INVALID_DATA;

	os_zero_mem(&rSrCmdMeshUlMode, sizeof(rSrCmdMeshUlMode));

	/* Assign Cmd Id */
	rSrCmdMeshUlMode.rSrCmd.u1CmdSubId = SR_CMD_SET_MESH_UL_MODE;
	rSrCmdMeshUlMode.rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmdMeshUlMode.rSrCmd.u1DbdcIdx = BandIdx;
	rSrCmdMeshUlMode.rSrMeshUlMode.u1UlMode = Value;

	Status = SrCmdMeshUlModeSet(pAd, &rSrCmdMeshUlMode);

	if (Status == NDIS_STATUS_SUCCESS)
		_u1SrMeshUlMode[BandIdx] = Value;

	return Status;
}

NDIS_STATUS SrCmdMeshUlModeSet(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SET_MESH_UL_MODE_T *prSrCmdMeshUlMode)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdMeshUlMode not null */
	if (!prSrCmdMeshUlMode) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdMeshUlMode */
	/* PrintSrCmdSrMeshUlMode(pAd, prSrCmdMeshUlMode); */

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(*prSrCmdMeshUlMode));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdMeshUlMode, sizeof(*prSrCmdMeshUlMode));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

/** Driver Internal **/
INT SetSrMeshStaModeRptLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	CHAR *value;
	UINT8 u1ArgNum = 0, u1RptLock = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 u1DbdcIdx = BAND0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				 value = rstrtok(NULL, "-"), u1ArgNum++) {
				if (Status == FALSE)
					break;
				switch (u1ArgNum) {
				case 0:
					u1RptLock = simple_strtol(value, 0, 10);
					if (IsInRange(u1RptLock, u1ArgNum, 0, 1) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_DEFAULT_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, SR_CMD_SET_DEFAULT_ARG_NUM);
				Status = FALSE;
			} else {
				if (SrMeshStaModeRptLockConfig(pAd, wdev, u1DbdcIdx, u1RptLock) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 "BandIdx:%u, _u1StaModeRptUnLock:%u\n",
		 u1DbdcIdx, _u1StaModeRptUnLock[u1DbdcIdx]);

	return Status;
}

NDIS_STATUS SrMeshStaModeRptLockConfig(IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev, IN UINT8 BandIdx, IN UINT8 u1RptLock)
{
	if (BandIdx >= RAM_BAND_NUM)
		return NDIS_STATUS_INVALID_DATA;

	_u1StaModeRptUnLock[BandIdx] = u1RptLock ? 0 : 1;

	SrMeshSrUpdateSTAMode(pAd, wdev, FALSE, FALSE);

	return NDIS_STATUS_SUCCESS;
}

/** SR_CMD_SET_SR_CFG_MESH_SR_REMOTE_STA_MODE **/
INT SetSrMeshRemoteStaHe(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	CHAR *value;
	UINT8 u1ArgNum = 0, u1StaAllHe = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 u1DbdcIdx = BAND0;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		do {
			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				value = rstrtok(NULL, "-"), u1ArgNum++) {

				if (Status == FALSE)
					break;

				switch (u1ArgNum) {

				case 0:
					u1StaAllHe = simple_strtol(value, 0, 10);
					if (IsInRange(u1StaAllHe, u1ArgNum, 0, 1) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;

				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_SET_DEFAULT_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_SET_DEFAULT_ARG_NUM);
				Status = FALSE;
			} else {
				if (SrSetMeshRemoteStaModeRpt(pAd, u1DbdcIdx, u1StaAllHe) != NDIS_STATUS_SUCCESS)
					Status = FALSE;
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}

	return Status;
}

NDIS_STATUS SrSetMeshRemoteStaModeRpt(IN PRTMP_ADAPTER pAd, IN UINT8 BandIdx, IN UINT8 RemoteAPStaAllHe)
{
	SR_CMD_T rSrCmd;

	if (BandIdx >= RAM_BAND_NUM)
		return NDIS_STATUS_INVALID_DATA;

	os_zero_mem(&rSrCmd, sizeof(rSrCmd));

	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_MESH_SR_REMOTE_STA_MODE;
	rSrCmd.u1ArgNum = SR_CMD_SET_DEFAULT_ARG_NUM;
	rSrCmd.u1DbdcIdx = BandIdx;
	rSrCmd.u4Value = RemoteAPStaAllHe;

	return SrCmd(pAd, &rSrCmd);
}

/** SR_CMD_GET_SR_CAP_ALL_INFO **/
NDIS_STATUS ShowSrCap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CAP_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcap=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}


/** SR_CMD_GET_SR_PARA_ALL_INFO **/
NDIS_STATUS ShowSrPara(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_PARA_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srpara=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_IND_ALL_INFO **/
NDIS_STATUS ShowSrInd(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_IND_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srind=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO **/
NDIS_STATUS ShowSrInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0, u1DropTaIdx = 0, u1StaIdx = 0;
	SR_CMD_T rSrCmd;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));

	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO;
	rSrCmd.u1ArgNum = SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO_ARG_NUM;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					u1DropTaIdx = simple_strtol(value, 0, 10);
					Status =
					    IsInRange(u1DropTaIdx, u1ArgNum, 0, SR_DROP_TA_NUM - 1);
					break;
				case 1:
					u1StaIdx = simple_strtol(value, 0, 10);
					Status = IsInRange(u1StaIdx, u1ArgNum, 0, SR_STA_NUM);
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Format Error! ArgNum = %d != %d\n",
					  u1ArgNum, rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {

				rSrCmd.u1DropTaIdx = u1DropTaIdx;
				rSrCmd.u1StaIdx = u1StaIdx;
				Status = SrCmd(pAd, &rSrCmd);
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srinfo=0-0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_COND_ALL_INFO **/
NDIS_STATUS ShowSrCond(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_COND_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcond=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_RCPI_TBL_ALL_INFO **/
NDIS_STATUS ShowSrRcpiTbl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_RCPI_TBL_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srrcpitbl=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_RCPI_TBL_OFST_ALL_INFO **/
NDIS_STATUS ShowSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_RCPI_TBL_OFST_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srrcpitblofst=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_Q_CTRL_ALL_INFO **/
NDIS_STATUS ShowSrQCtrl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_Q_CTRL_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srqctrl=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_IBPD_ALL_INFO **/
NDIS_STATUS ShowSrIBPD(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_IBPD_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show sribpd=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_NRT_ALL_INFO **/
NDIS_STATUS ShowSrNRT(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_NRT_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srnrt=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_NRT_CTRL_ALL_INFO **/
NDIS_STATUS ShowSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_NRT_CTRL_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srnrtctrl=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_FNQ_CTRL_ALL_INFO **/
NDIS_STATUS ShowSrFNQCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	if (IS_SR_V2(pAd)) {
		Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_FNQ_CTRL_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_PRINT("iwpriv ra0 show srfnqctrl=0\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_FRM_FILT_ALL_INFO **/
NDIS_STATUS ShowSrFrmFiltAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	if (IS_SR_V2(pAd)) {
		Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_FRM_FILT_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_PRINT("iwpriv ra0 show srfrmfilt=0\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_INTERPS_CTRL_ALL_INFO **/
NDIS_STATUS ShowSrInterPsCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	if (IS_SR_V2(pAd)) {
		Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_INTERPS_CTRL_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_PRINT("iwpriv ra0 show srinterpsctrl=0\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_INTERPS_DBG_ALL_INFO **/
NDIS_STATUS ShowSrInterPsDbgAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	if (IS_SR_V2(pAd)) {
		Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_INTERPS_DBG_ALL_INFO, SR_CMD_GET_DEFAULT_ARG_NUM);
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_PRINT("iwpriv ra0 show srinterpsdbg=0\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_ENABLE **/
NDIS_STATUS ShowSrCfgSrEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgsren=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_SD_ENABLE **/
NDIS_STATUS ShowSrCfgSrSdEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_SD_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgsrsden=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_BF **/
NDIS_STATUS ShowSrCfgSrBf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_BF, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgsrbf=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_ATF **/
NDIS_STATUS ShowSrCfgSrAtf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_ATF, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgsratf=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_MODE **/
NDIS_STATUS ShowSrCfgSrMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_MODE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgsrmode=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_DISRT_ENABLE **/
NDIS_STATUS ShowSrCfgDISRTEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_DISRT_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgdisrten=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_DISRT_MIN_RSSI **/
NDIS_STATUS ShowSrCfgDISRTMinRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_DISRT_MIN_RSSI, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgdisrtmin=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_TXC_QUEUE **/
NDIS_STATUS ShowSrCfgTxcQueue(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_TXC_QUEUE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgtxcq=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_TXC_QID **/
NDIS_STATUS ShowSrCfgTxcQid(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_TXC_QID, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgtxcqid=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_TXC_PATH **/
NDIS_STATUS ShowSrCfgTxcPath(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_TXC_PATH, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgtxcpath=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_AC_METHOD **/
NDIS_STATUS ShowSrCfgAcMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_AC_METHOD, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgac=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_PERIOD_THR **/
NDIS_STATUS ShowSrCfgSrPeriodThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_PERIOD_THR, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgsrperiodthr=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_QUERY_TXD_METHOD **/
NDIS_STATUS ShowSrCfgQueryTxDMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_QUERY_TXD_METHOD, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgsrquerytxd=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_SD_CG_RATIO **/
NDIS_STATUS ShowSrCfgSrSdCgRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_SD_CG_RATIO, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgsrsdcg=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_SD_OBSS_RATIO **/
NDIS_STATUS ShowSrCfgSrSdObssRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_SD_OBSS_RATIO, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgsrsdobss=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_PROFILE **/
NDIS_STATUS ShowSrCfgProfile(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_PROFILE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show srcfgsrprofile=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_FNQ_ENABLE **/
NDIS_STATUS ShowSrCfgFnqEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	if (IS_SR_V2(pAd)) {
		Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_FNQ_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_PRINT("iwpriv ra0 show srcfgfnqen=0\n");
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Command not supported\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_DPD_ENABLE **/
NDIS_STATUS ShowSrCfgDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_DPD_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show  srcfgdpden=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_TX_ENABLE **/
NDIS_STATUS ShowSrCfgSrTxEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_TX_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show  srcfgsrtxen=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_SD_OM_ENABLE **/
NDIS_STATUS ShowSrCfgObssMonitorEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_SD_OM_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show  srcfgomen=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_TX_ALIGN_ENABLE **/
NDIS_STATUS ShowSrCfgSrTxAlignEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_TX_ALIGN_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show  srcfgsrtxalignen=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_TX_ALIGN_RSSI_THR **/
NDIS_STATUS ShowSrCfgSrTxAlignRssiThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_TX_ALIGN_RSSI_THR, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show  srcfgsrtxalignrssi=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_DABS_MODE **/
NDIS_STATUS ShowSrCfgDabsMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_DABS_MODE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show  srcfgdabsmode=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CFG_SR_DABS_MODE **/
NDIS_STATUS ShowSrCfgDropMinMCS(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_DROP_MIN_MCS, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS)
		MTWF_PRINT("iwpriv ra0 show  srcfgdropminmcs=0\n");

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}


/** ShowSrCfgSrDPDThreshold **/
NDIS_STATUS ShowSrCfgSrDPDThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_DPD_THRESHOLD, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS)
		MTWF_PRINT("iwpriv ra0 show  SrDPDThreshold=\n");

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_CNT_ALL **/
NDIS_STATUS ShowSrCnt(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CNT_ALL, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show  srcnt=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_SD_ALL **/
NDIS_STATUS ShowSrSd(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_SD_ALL, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show  srsd=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_SRG_BITMAP **/
NDIS_STATUS ShowSrSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_SRG_BITMAP, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_PRINT("iwpriv ra0 show  srsrgbm=0\n");
	}

	return (Status == NDIS_STATUS_SUCCESS)?TRUE:FALSE;
}

/** SR_CMD_GET_SR_MESH_SRG_BITMAP **/
INT ShowSrMeshSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

	if (SrCmdShow(pAd, arg, SR_CMD_GET_SR_MESH_SRG_BITMAP, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_PRINT("iwpriv ra0 show  srmeshsrgbm=0\n");
	}

	return Status;
}

/** SR_CMD_GET_SR_SIGA_FLAG_INFO **/
INT ShowSrSiga(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

	if (SrCmdShow(pAd, arg, SR_CMD_GET_SR_SIGA_FLAG_INFO, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_PRINT("iwpriv ra0 show srsiga=0\n");
	}

	return Status;
}

/** SR_CMD_GET_SR_SIGA_AUTO_FLAG_INFO **/
INT ShowSrSigaAuto(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

	if (SrCmdShow(pAd, arg, SR_CMD_GET_SR_SIGA_AUTO_FLAG_INFO, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_PRINT("iwpriv ra0 show  srsigaauto=0\n");
	}

	return Status;
}

/** Driver Internal **/
INT ShowSrSelfSrgInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	CHAR *value;
	UINT_8 u1ArgNum, u1Mode = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1BandIdx = BAND0;
	struct _SR_MESH_SRG_BITMAP_T *prSrgBitmap = NULL;

	if (wdev != NULL)
		u1BandIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
				 value = rstrtok(NULL, "-"), u1ArgNum++) {
				if (Status == FALSE)
					break;
				switch (u1ArgNum) {
				case 0:
					u1Mode = simple_strtol(value, 0, 10);
					if (IsInRange(u1Mode, u1ArgNum, 0, 1) != NDIS_STATUS_SUCCESS)
						Status = FALSE;
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"set wrong parameters\n");
						Status = FALSE;
						break;
					}
				}
			}

			if (Status == FALSE)
				break;

			if (u1ArgNum != SR_CMD_GET_DEFAULT_ARG_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Format Error! ArgNum = %d != %d\n",
					u1ArgNum, SR_CMD_GET_DEFAULT_ARG_NUM);
				Status = FALSE;
				break;
			}

			if (u1Mode == 1)
				prSrgBitmap = &rSrSelfSrgBMMan[u1BandIdx];
			else
				prSrgBitmap = &rSrSelfSrgBM[u1BandIdx];
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Arg is Null\n");
		Status = FALSE;
	}
	if (prSrgBitmap)
		MTWF_PRINT("BandIdx:%u, Mode:%u BSS_Color_BM:[63:32][0x%x]-[31:0][0x%x], Par_Bssid_BM:[63:32][0x%x]-[31:0][0x%x]\n",
			u1BandIdx, u1Mode, prSrgBitmap->u4Color_63_32,
			prSrgBitmap->u4Color_31_0, prSrgBitmap->u4pBssid_63_32,
			prSrgBitmap->u4pBssid_31_0);

	return Status;
}

/** Driver Internal **/
INT ShowSrMeshTopoLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1BandIdx = BAND0;

	if (wdev != NULL)
		u1BandIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	MTWF_PRINT("BandIdx:%u, _u1SrMeshTopoLock=%u\n",
		u1BandIdx, _u1SrMeshTopoLock[u1BandIdx]);

	return Status;
}

/** SR_CMD_GET_REMOTE_FH_RSSI **/
INT ShowSrMeshRemoteFhRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

	if (SrCmdShow(pAd, arg, SR_CMD_GET_REMOTE_FH_RSSI, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_PRINT("iwpriv ra0 show  srmeshremotefhrssi=0\n");
	}

	return Status;
}

/** SR_CMD_GET_MESH_FH_RSSI_TH **/
INT ShowSrMeshFhRssiTh(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

	if (SrCmdShow(pAd, arg, SR_CMD_GET_MESH_FH_RSSI_TH, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_PRINT("iwpriv ra0 show  srmeshremotefhrssiTh=0\n");
	}

	return Status;
}

/** SR_CMD_GET_DOWNLINK_STA_THRESHOLD **/
INT ShowSrMeshstatTh(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status;
	INT idevidx = 0;
	UINT32 rv;
	struct wifi_dev *wdev = NULL;
	UINT_8 u1DbdcIdx = BAND0, u1Bssidx = 0;
	SR_CMD_T rSrCmd;

	os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));

	if (arg) {
		rv = sscanf(arg, "%d", &idevidx);

		if (rv == 1)  {
			wdev = pAd->wdev_list[idevidx];

			if (wdev == NULL) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Wdev is Null, WdevIdx  = %d\n", idevidx);
				return FALSE;
			}

			u1DbdcIdx = HcGetBandByWdev(wdev);
			u1Bssidx = (UINT_8)wdev->DevInfo.OwnMacIdx > SR_MESH_P2PGC ? wdev->DevInfo.OwnMacIdx - SR_BSSID_OMAC_OFFSET : wdev->DevInfo.OwnMacIdx;

			/* Assign Cmd Id */
			rSrCmd.u1CmdSubId = SR_CMD_GET_DOWNLINK_STA_THRESHOLD;
			rSrCmd.u1DbdcIdx = u1DbdcIdx;
			rSrCmd.u1bssid = u1Bssidx;
			Status = SrCmd(pAd, &rSrCmd);
		} else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "set wrong parameters\n");
			Status = FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = FALSE;
	}

	if (Status == NDIS_STATUS_SUCCESS)
		Status = TRUE;

	return Status;
}

/** SR_CMD_GET_BH_MESH_SR_BITMAP **/
INT ShowSrMeshBHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

	if (SrCmdShow(pAd, arg, SR_CMD_GET_BH_MESH_SR_BITMAP, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_PRINT("iwpriv ra0 show  srmeshbhsrgbm=0\n");
	}

	return Status;
}

/** SR_CMD_GET_FH_MESH_SR_BITMAP **/
INT ShowSrMeshFHSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

	if (SrCmdShow(pAd, arg, SR_CMD_GET_FH_MESH_SR_BITMAP, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_PRINT("iwpriv ra0 show  srmeshfhsrgbm=0\n");
	}

	return Status;
}

/** SR_CMD_GET_BH_DOWNLINK_MESH_SR_THRESHOLD **/
INT ShowSrMeshBHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

	if (SrCmdShow(pAd, arg, SR_CMD_GET_BH_DOWNLINK_MESH_SR_THRESHOLD, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_PRINT("iwpriv ra0 show  srmeshbhdlobsspdth=0\n");
	}

	return Status;
}

/** SR_CMD_GET_FH_DOWNLINK_MESH_SR_THRESHOLD **/
INT ShowSrMeshFHDownThreshold(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

	if (SrCmdShow(pAd, arg, SR_CMD_GET_FH_DOWNLINK_MESH_SR_THRESHOLD, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_PRINT("iwpriv ra0 show  srmeshfhdlobsspdth=0\n");
	}

	return Status;
}

/** SR_CMD_GET_FORHIB_MESH_SR **/
INT ShowSrMeshBHDownLinkForbidSR(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

	if (SrCmdShow(pAd, arg, SR_CMD_GET_FORHIB_MESH_SR, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_PRINT("iwpriv ra0 show  srmeshforbitsr=0\n");
	}

	return Status;
}

/** SR_CMD_GET_REMOTE_BH_INFO **/
INT ShowSrMeshRemoteBhInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

	if (SrCmdShow(pAd, arg, SR_CMD_GET_REMOTE_BH_INFO, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_PRINT("iwpriv ra0 show  srmeshremotebhinfo=0\n");
	}

	return Status;
}

/** SR_CMD_GET_MAP_TOPO **/
INT ShowSrMeshTopo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

	if (SrCmdShow(pAd, arg, SR_CMD_GET_MAP_TOPO, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_PRINT("iwpriv ra0 show  srmeshtopo=0\n");
	}

	return Status;
}

/** Driver Internal **/
INT ShowSrMeshTopoUpdateParams(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1BandIdx = BAND0;
	struct sr_mesh_topology_update_params *prParams;

	if (wdev != NULL)
		u1BandIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	prParams = &_rSrMeshTopologyUpdateParams[u1BandIdx];

	MTWF_PRINT("BandIdx:%u, map_dev_count=%u map_dev_sr_support_mode:%u self_role:%u\n",
		u1BandIdx, prParams->topo_params.map_dev_count,
		prParams->topo_params.map_dev_sr_support_mode, prParams->topo_params.self_role);

	MTWF_PRINT("AL MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
		PRINT_MAC(prParams->topo_params.map_remote_al_mac));

	MTWF_PRINT("FH BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",
		PRINT_MAC(prParams->topo_params.map_remote_fh_bssid));

	MTWF_PRINT("FH SSID:%s SSIDLen:%u\n",
		prParams->topo_params.ssid, prParams->topo_params.ssid_len);

	MTWF_PRINT("BH MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
		PRINT_MAC(prParams->topo_params.map_remote_bh_mac));

	return Status;
}

/** SR_CMD_GET_MAP_TRAFFIC_STATUS **/
INT ShowSrMeshUplinkEvent(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

	if (SrCmdShow(pAd, arg, SR_CMD_GET_MAP_TRAFFIC_STATUS, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_PRINT("iwpriv ra0 show srmeshuplinkevent=0\n");
	}

	return Status;
}

/** SR_CMD_GET_MESH_PHASE **/
INT ShowSrMeshPhase(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

	if (SrCmdShow(pAd, arg, SR_CMD_GET_MESH_PHASE, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_PRINT("iwpriv ra0 show srmeshphase=0\n");
	}

	return Status;
}

INT ShowSrMeshUlMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	INT Status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1BandIdx = BAND0;

	if (wdev != NULL)
		u1BandIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	MTWF_PRINT("BandIdx:%u, SRMeshUlMode:%u\n",
		u1BandIdx, _u1SrMeshUlMode[u1BandIdx]);

	return Status;

}

/** Driver Internal **/
INT ShowSrMeshStaModeRptLock(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1BandIdx = BAND0;

	if (wdev != NULL)
		u1BandIdx = HcGetBandByWdev(wdev);
	else
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"BandIdx:%u, _u1StaModeRptUnLock=%u\n",
		u1BandIdx, _u1StaModeRptUnLock[u1BandIdx]);

	return Status;
}

/** SR_CMD_GET_SR_CFG_MESH_SR_REMOTE_STA_MODE **/
INT ShowSrMeshRemoteStaHe(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	INT Status = TRUE;

	if (SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_MESH_SR_REMOTE_STA_MODE, SR_CMD_GET_DEFAULT_ARG_NUM) != NDIS_STATUS_SUCCESS) {
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "iwpriv ra0 show srmeshallremotestahe=0\n");
	}

	return Status;
}

/* for set/show function*/
NDIS_STATUS SrCmdSRUpdateCap(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrCap)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrCap not null */
	if (!_prSrCmdSrCap) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	if (IS_SR_V1(pAd)) {
		P_SR_CMD_SR_CAP_T_SR_V1 prSrCmdSrCap = (P_SR_CMD_SR_CAP_T_SR_V1)_prSrCmdSrCap;
		/* Print prSrCmdSrCap */
		PrintSrCmdSrCap(pAd, prSrCmdSrCap);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_CAP_T_SR_V1));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrCap, sizeof(SR_CMD_SR_CAP_T_SR_V1));
		ret = AndesSendCmdMsg(pAd, msg);

	} else if (IS_SR_V2(pAd)) {
		P_SR_CMD_SR_CAP_T_SR_V2 prSrCmdSrCap = (P_SR_CMD_SR_CAP_T_SR_V2)_prSrCmdSrCap;
		/* Print prSrCmdSrCap */
		PrintSrCmdSrCap(pAd, prSrCmdSrCap);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_CAP_T_SR_V2));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrCap, sizeof(SR_CMD_SR_CAP_T_SR_V2));
		ret = AndesSendCmdMsg(pAd, msg);
	}

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdatePara(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_PARA_T prSrCmdSrPara)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrPara not null */
	if (!prSrCmdSrPara) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrPara */
	PrintSrCmdSrPara(prSrCmdSrPara);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_PARA_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrPara, sizeof(SR_CMD_SR_PARA_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS
SrCmdSRUpdateGloVarSingleDropTa(IN PRTMP_ADAPTER pAd,
				IN P_SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T
				prSrCmdSrGlobalVarSingleDropTa, IN UINT_32 u4DropTaIdx,
				IN UINT_32 u4StaIdx)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrEventSrGlobalVar not null */
	if (!prSrCmdSrGlobalVarSingleDropTa) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrEventSrGlobalVar */
	PrintSrCmdSrGloVarSingleDropTa(prSrCmdSrGlobalVarSingleDropTa, u4DropTaIdx, u4StaIdx);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrGlobalVarSingleDropTa,
			  sizeof(SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateCond(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrCond)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check _prSrCmdSrCond not null */
	if (!_prSrCmdSrCond) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	if (IS_SR_V1(pAd)) {
		P_SR_CMD_SR_COND_T_SR_V1 prSrCmdSrCond = (P_SR_CMD_SR_COND_T_SR_V1)_prSrCmdSrCond;
		/* Print prSrCmdSrCond */
		PrintSrCmdSrCond(pAd, prSrCmdSrCond);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_COND_T_SR_V1));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrCond, sizeof(SR_CMD_SR_COND_T_SR_V1));
		ret = AndesSendCmdMsg(pAd, msg);

	} else if (IS_SR_V2(pAd)) {
		P_SR_CMD_SR_COND_T_SR_V2 prSrCmdSrCond = (P_SR_CMD_SR_COND_T_SR_V2)_prSrCmdSrCond;
		/* Print prSrCmdSrCond */
		PrintSrCmdSrCond(pAd, prSrCmdSrCond);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_COND_T_SR_V2));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrCond, sizeof(SR_CMD_SR_COND_T_SR_V2));
		ret = AndesSendCmdMsg(pAd, msg);
	}

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateRcpiTbl(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_RCPITBL_T prSrCmdSrRcpiTbl)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrRcpiTbl not null */
	if (!prSrCmdSrRcpiTbl) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrRcpiTbl */
	PrintSrCmdSrRcpiTbl(prSrCmdSrRcpiTbl);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_RCPITBL_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrRcpiTbl, sizeof(SR_CMD_SR_RCPITBL_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrRcpiTblOfst)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check _prSrCmdSrRcpiTblOfst not null */
	if (!_prSrCmdSrRcpiTblOfst) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	if (IS_SR_V1(pAd)) {
		P_SR_CMD_SR_RCPITBL_OFST_T_SR_V1 prSrCmdSrRcpiTblOfst = (P_SR_CMD_SR_RCPITBL_OFST_T_SR_V1) _prSrCmdSrRcpiTblOfst;
		/* Print prSrCmdSrRcpiTblOfst */
		PrintSrCmdSrRcpiTblOfst(pAd, prSrCmdSrRcpiTblOfst);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_RCPITBL_OFST_T_SR_V1));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrRcpiTblOfst, sizeof(SR_CMD_SR_RCPITBL_OFST_T_SR_V1));
		ret = AndesSendCmdMsg(pAd, msg);

	} else if (IS_SR_V2(pAd)) {
		P_SR_CMD_SR_RCPITBL_OFST_T_SR_V2 prSrCmdSrRcpiTblOfst = (P_SR_CMD_SR_RCPITBL_OFST_T_SR_V2) _prSrCmdSrRcpiTblOfst;
		/* Print prSrCmdSrRcpiTblOfst */
		PrintSrCmdSrRcpiTblOfst(pAd, prSrCmdSrRcpiTblOfst);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_RCPITBL_OFST_T_SR_V2));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrRcpiTblOfst, sizeof(SR_CMD_SR_RCPITBL_OFST_T_SR_V2));
		ret = AndesSendCmdMsg(pAd, msg);
	}

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateQCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrQCtrl)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check _prSrCmdSrQCtrl not null */
	if (!_prSrCmdSrQCtrl) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	if (IS_SR_V1(pAd)) {
		P_SR_CMD_SR_Q_CTRL_T_SR_V1 prSrCmdSrQCtrl = (P_SR_CMD_SR_Q_CTRL_T_SR_V1)_prSrCmdSrQCtrl;
		/* Print prSrCmdSrQCtrl */
		PrintSrCmdSrQCtrl(pAd, prSrCmdSrQCtrl);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_Q_CTRL_T_SR_V1));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrQCtrl, sizeof(SR_CMD_SR_Q_CTRL_T_SR_V1));
		ret = AndesSendCmdMsg(pAd, msg);
	} else if (IS_SR_V2(pAd)) {
		P_SR_CMD_SR_Q_CTRL_T_SR_V2 prSrCmdSrQCtrl = (P_SR_CMD_SR_Q_CTRL_T_SR_V2)_prSrCmdSrQCtrl;
		/* Print prSrCmdSrQCtrl */
		PrintSrCmdSrQCtrl(pAd, prSrCmdSrQCtrl);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_Q_CTRL_T_SR_V2));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrQCtrl, sizeof(SR_CMD_SR_Q_CTRL_T_SR_V2));
		ret = AndesSendCmdMsg(pAd, msg);
	}

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateIBPD(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_IBPD_T prSrCmdSrIBPD)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrIBPD not null */
	if (!prSrCmdSrIBPD) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrIBPD */
	PrintSrCmdSrIBPD(prSrCmdSrIBPD);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_IBPD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrIBPD, sizeof(SR_CMD_SR_IBPD_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateNRT(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrNRT)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check _prSrCmdSrNRT not null */
	if (!_prSrCmdSrNRT) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	if (IS_SR_V1(pAd)) {
		P_SR_CMD_SR_NRT_T_SR_V1 prSrCmdSrNRT = (P_SR_CMD_SR_NRT_T_SR_V1)_prSrCmdSrNRT;
		/* Print prSrCmdSrNRT */
		PrintSrCmdSrNRT(pAd, prSrCmdSrNRT);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_NRT_T_SR_V1));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrNRT, sizeof(SR_CMD_SR_NRT_T_SR_V1));
		ret = AndesSendCmdMsg(pAd, msg);
	} else if (IS_SR_V2(pAd)) {
		P_SR_CMD_SR_NRT_T_SR_V2 prSrCmdSrNRT = (P_SR_CMD_SR_NRT_T_SR_V2)_prSrCmdSrNRT;
		/* Print prSrCmdSrNRT */
		PrintSrCmdSrNRT(pAd, prSrCmdSrNRT);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_NRT_T_SR_V2));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrNRT, sizeof(SR_CMD_SR_NRT_T_SR_V2));
		ret = AndesSendCmdMsg(pAd, msg);
	}

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrNRTCtrl)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check _prSrCmdSrNRTCtrl not null */
	if (!_prSrCmdSrNRTCtrl) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	if (IS_SR_V1(pAd)) {
		P_SR_CMD_SR_NRT_CTRL_T_SR_V1 prSrCmdSrNRTCtrl = (P_SR_CMD_SR_NRT_CTRL_T_SR_V1)_prSrCmdSrNRTCtrl;
		/* Print prSrCmdSrNRTCtrl */
		PrintSrCmdSrNRTCtrl(pAd, prSrCmdSrNRTCtrl);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_NRT_CTRL_T_SR_V1));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrNRTCtrl, sizeof(SR_CMD_SR_NRT_CTRL_T_SR_V1));
		ret = AndesSendCmdMsg(pAd, msg);
	} else if (IS_SR_V2(pAd)) {
		P_SR_CMD_SR_NRT_CTRL_T_SR_V2 prSrCmdSrNRTCtrl = (P_SR_CMD_SR_NRT_CTRL_T_SR_V2)_prSrCmdSrNRTCtrl;
		/* Print prSrCmdSrNRTCtrl */
		PrintSrCmdSrNRTCtrl(pAd, prSrCmdSrNRTCtrl);

		/* Allocate msg */
		msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_NRT_CTRL_T_SR_V2));
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		AndesInitCmdMsg(msg, attr);
		AndesAppendCmdMsg(msg, (char *)prSrCmdSrNRTCtrl, sizeof(SR_CMD_SR_NRT_CTRL_T_SR_V2));
		ret = AndesSendCmdMsg(pAd, msg);
	}

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateFNQCtrl(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_FNQ_CTRL_T prSrCmdSrFNQCtrl)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrFNQCtrl not null */
	if (!prSrCmdSrFNQCtrl) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrFNQCtrl */
	PrintSrCmdSrFNQCtrl(prSrCmdSrFNQCtrl);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_FNQ_CTRL_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrFNQCtrl, sizeof(SR_CMD_SR_FNQ_CTRL_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateFrmFilt(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_FRM_FILT_T prSrCmdSrFrmFilt)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrFrmFilt not null */
	if (!prSrCmdSrFrmFilt) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrFrmFilt */
	PrintSrCmdSrFrmFilt(prSrCmdSrFrmFilt);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_FRM_FILT_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrFrmFilt, sizeof(SR_CMD_SR_FRM_FILT_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateInterPsCtrl(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_INTERPS_CTRL_T prSrCmdSrInterPsCtrl)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrInterPsCtrl not null */
	if (!prSrCmdSrInterPsCtrl) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrInterPsCtrl */
	PrintSrCmdSrInterPsCtrl(prSrCmdSrInterPsCtrl);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_INTERPS_CTRL_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrInterPsCtrl, sizeof(SR_CMD_SR_INTERPS_CTRL_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateSrgBitmap(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_SRG_BITMAP_T prSrCmdSrSrgBitmap)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrSrgBitmap not null */
	if (!prSrCmdSrSrgBitmap) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrSrgBitmap */
	/* PrintSrCmdSrSrgBitmap(prSrCmdSrSrgBitmap); */

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_SRG_BITMAP_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrSrgBitmap, sizeof(SR_CMD_SR_SRG_BITMAP_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateSiga(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_SIGA_FLAG_T prSrCmdSrSigaFlag)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrSigaFlag not null */
	if (!prSrCmdSrSigaFlag) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrSigaFlag */
	PrintSrCmdSrSiga(pAd, prSrCmdSrSigaFlag);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_SIGA_FLAG_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrSigaFlag, sizeof(SR_CMD_SR_SIGA_FLAG_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSRUpdateSigaAuto(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SR_SIGA_AUTO_FLAG_T *prSrCmdSrSigaAutoFlag)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrSigaAutoFlag not null */
	if (!prSrCmdSrSigaAutoFlag) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrSigaAutoFlag */
	/* PrintSrCmdSrSigaAuto(pAd, prSrCmdSrSigaAutoFlag); */

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(*prSrCmdSrSigaAutoFlag));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrSigaAutoFlag, sizeof(*prSrCmdSrSigaAutoFlag));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdMeshTopologyUpd(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_MESH_TOPOLOGY_T *prSrCmdMeshTopo)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdMeshTopo not null */
	if (!prSrCmdMeshTopo) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdMeshTopo */
	/* PrintSrCmdMeshTopo(prSrCmdMeshTopo); */

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(*prSrCmdMeshTopo));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdMeshTopo, sizeof(*prSrCmdMeshTopo));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSetMeshSRLinkStaThreshold(IN PRTMP_ADAPTER pAd, IN UINT_8 u1BandIdx, IN UINT_8 u1Bssidx, IN INT_8 i1Rssi)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };
	struct SR_CMD_MESH_SR_DL_STA_MESH_THRESHOLD_T rSrCmdDLStaThreshold;

	os_zero_mem(&rSrCmdDLStaThreshold, sizeof(rSrCmdDLStaThreshold));

	/* Assign Cmd Id */
	rSrCmdDLStaThreshold.rSrCmd.u1CmdSubId = SR_CMD_SET_DOWNLINK_STA_THRESHOLD;
	rSrCmdDLStaThreshold.rSrCmd.u1DbdcIdx = u1BandIdx;
	rSrCmdDLStaThreshold.rSrDLStaThrehsold.irssi = i1Rssi;
	rSrCmdDLStaThreshold.rSrDLStaThrehsold.u1BSSID = u1Bssidx;

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(rSrCmdDLStaThreshold));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&rSrCmdDLStaThreshold, sizeof(rSrCmdDLStaThreshold));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSetMeshMultiApBhDlSrTh(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, INT_8 Rssi)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };
	SR_CMD_T rSrCmd;

	os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));

	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_BH_DOWNLINK_MESH_SR_THRESHOLD;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = Rssi;

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&rSrCmd, sizeof(SR_CMD_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSetMeshMultiApFhDlSrTh(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, INT_8 Rssi)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };
	SR_CMD_T rSrCmd;

	os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));

	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_FH_DOWNLINK_MESH_SR_THRESHOLD;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = Rssi;

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&rSrCmd, sizeof(SR_CMD_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSetMeshForbidSrBssid(IN PRTMP_ADAPTER pAd, UINT_8 u1DbdcIdx, UINT_8 u1Bssid)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };
	SR_CMD_T rSrCmd;

	os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));

	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_FORHIB_MESH_SR;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;
	rSrCmd.u4Value = u1Bssid;

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&rSrCmd, sizeof(SR_CMD_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdFlushMeshForbidSrBssid(IN PRTMP_ADAPTER pAd, IN UINT_8 u1DbdcIdx)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };
	SR_CMD_T rSrCmd;

	os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));

	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = SR_CMD_SET_FORHIB_MESH_SR_RESET;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&rSrCmd, sizeof(SR_CMD_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdSetFhRssiTh(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_MESH_FH_RSSI_TH_T *prSrCmdSrMeshFhRssiTh)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrMeshFhRssiTh not null */
	if (!prSrCmdSrMeshFhRssiTh) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(*prSrCmdSrMeshFhRssiTh));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrMeshFhRssiTh, sizeof(*prSrCmdSrMeshFhRssiTh));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(ret = %d)\n", ret);
	return ret;
}

NDIS_STATUS SrCmdShow(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg, IN UINT_8 u1CmdSubId, IN UINT_8 u1ArgNum)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	SR_CMD_T rSrCmd;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));

	/* Assign Cmd Id */
	rSrCmd.u1CmdSubId = u1CmdSubId;
	rSrCmd.u1ArgNum = u1ArgNum;
	rSrCmd.u1DbdcIdx = u1DbdcIdx;

	if (arg) {
		do {

			/* parameter parsing */
			for (u1ArgNum = 0, value = rstrtok(arg, "-"); value;
			     value = rstrtok(NULL, "-"), u1ArgNum++) {
			    if (Status == NDIS_STATUS_FAILURE)
					break;
				switch (u1ArgNum) {
				case 0:
					rSrCmd.u4Value = simple_strtol(value, 0, 10);
#ifdef RT_BIG_ENDIAN
					rSrCmd.u4Value = cpu2le32(rSrCmd.u4Value);
#endif
					break;
				default:{
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "set wrong parameters\n");
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmd.u1ArgNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					" Format Error! ArgNum = %d != %d\n",
					 u1ArgNum, rSrCmd.u1ArgNum);
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {

				Status = SrCmd(pAd, &rSrCmd);
			}
		} while (0);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Arg is Null\n");
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}


NDIS_STATUS
SrCmd(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_T prSrcmd)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrcmd not null */
	if (!prSrcmd) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmd */
	if (prSrcmd->u1CmdSubId != SR_CMD_SET_SR_MESH_SR_SD_CTRL)
		PrintSrCmd(prSrcmd);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)prSrcmd, sizeof(SR_CMD_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d)\n", ret);
	return ret;
}


VOID EventSrHandler(PRTMP_ADAPTER pAd, UINT8 *Data, UINT_32 Length)
{
	/* Event ID */
	UINT8 u1EventSubId;

	/* Get Event Category ID */
	u1EventSubId = *Data;

	/**Prevent legitimate but wrong ID **/
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		 "%s: u1EventSubId = %d\n", __func__, u1EventSubId);

	/* Event Handle for different Category ID */
	switch (u1EventSubId) {
	case SR_EVENT_GET_SR_CFG_SR_ENABLE:
	case SR_EVENT_GET_SR_CFG_SR_SD_ENABLE:
	case SR_EVENT_GET_SR_CFG_SR_MODE:
	case SR_EVENT_GET_SR_CFG_DISRT_ENABLE:
	case SR_EVENT_GET_SR_CFG_DISRT_MIN_RSSI:
	case SR_EVENT_GET_SR_CFG_SR_BF:
	case SR_EVENT_GET_SR_CFG_SR_ATF:
	case SR_EVENT_GET_SR_CFG_TXC_QUEUE:
	case SR_EVENT_GET_SR_CFG_TXC_QID:
	case SR_EVENT_GET_SR_CFG_TXC_PATH:
	case SR_EVENT_GET_SR_CFG_AC_METHOD:
	case SR_EVENT_GET_SR_CFG_SR_PERIOD_THR:
	case SR_EVENT_GET_SR_CFG_QUERY_TXD_METHOD:
	case SR_EVENT_GET_SR_CFG_SR_SD_CG_RATIO:
	case SR_EVENT_GET_SR_CFG_SR_SD_OBSS_RATIO:
	case SR_EVENT_GET_SR_CFG_PROFILE:
	case SR_EVENT_GET_SR_CFG_FNQ_ENABLE:
	case SR_EVENT_GET_SR_CFG_DPD_ENABLE:
	case SR_EVENT_GET_SR_CFG_SR_TX_ENABLE:
	case SR_EVENT_GET_SR_CFG_SR_SD_OM_ENABLE:
	case SR_EVENT_GET_SR_CFG_SR_TX_ALIGN_ENABLE:
	case SR_EVENT_GET_SR_CFG_SR_TX_ALIGN_RSSI_THR:
	case SR_EVENT_GET_SR_CFG_SR_DABS_MODE:
	case SR_EVENT_GET_SR_CFG_SR_DROP_MIN_MCS:
	case SR_EVENT_GET_SR_CFG_SR_DPD_THRESHOLD:
		PrintSrEvent((P_SR_EVENT_T)Data);
		break;
	case SR_EVENT_GET_SR_SRG_BITMAP:
		PrintSrEventSrSrgBitmap((P_SR_EVENT_SR_SRG_BITMAP_T)Data);
		break;
	case SR_EVENT_GET_SR_MESH_SRG_BITMAP:
		PrintSrEventSrSrgBitmap((P_SR_EVENT_SR_SRG_BITMAP_T)Data);
		break;
	case SR_EVENT_GET_SR_CNT_ALL:
		PrintSrEventSrCnt((P_SR_EVENT_SR_CNT_T)Data);
		break;
	case SR_EVENT_GET_SR_SD_ALL:
		PrintSrEventSrSd((P_SR_EVENT_SR_SD_T)Data);
		break;
	case SR_EVENT_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO:
		PrintSrEventSrGloVarSingleDropTa((P_SR_EVENT_SR_GLOBAL_VAR_SINGLE_DROP_TA_T)Data);
		break;
	case SR_EVENT_GET_SR_CAP_ALL_INFO:
		PrintSrEventSrCap(pAd, (VOID *)Data);
		break;
	case SR_EVENT_GET_SR_PARA_ALL_INFO:
		PrintSrEventSrPara((P_SR_EVENT_SR_PARA_T)Data);
		break;
	case SR_EVENT_GET_SR_COND_ALL_INFO:
		PrintSrEventSrCond(pAd, (VOID *)Data);
		break;
	case SR_EVENT_GET_SR_RCPI_TBL_ALL_INFO:
		PrintSrEventSrRcpiTbl((P_SR_EVENT_SR_RCPITBL_T)Data);
		break;
	case SR_EVENT_GET_SR_RCPI_TBL_OFST_ALL_INFO:
		PrintSrEventSrRcpiTblOfst(pAd, (VOID *)Data);
		break;
	case SR_EVENT_GET_SR_Q_CTRL_ALL_INFO:
		PrintSrEventSrQCtrl(pAd, (VOID *)Data);
		break;
	case SR_EVENT_GET_SR_IBPD_ALL_INFO:
		PrintSrEventSrIBPD((P_SR_EVENT_SR_IBPD_T)Data);
		break;
	case SR_EVENT_GET_SR_NRT_ALL_INFO:
		PrintSrEventSrNRT(pAd, (VOID *)Data);
		break;
	case SR_EVENT_GET_SR_NRT_CTRL_ALL_INFO:
		PrintSrEventSrNRTCtrl(pAd, (VOID *)Data);
		break;
	case SR_EVENT_GET_SR_IND_ALL_INFO:
		PrintSrEventSrInd((P_SR_EVENT_SR_IND_T)Data);
		break;
	case SR_EVENT_GET_SR_FNQ_CTRL_ALL_INFO:
		PrintSrEventSrFNQCtrl((P_SR_EVENT_SR_FNQ_CTRL_T)Data);
		break;
	case SR_EVENT_GET_SR_FRM_FILT_ALL_INFO:
		PrintSrEventSrFrmFilt((P_SR_EVENT_SR_FRM_FILT_T)Data);
		break;
	case SR_EVENT_GET_SR_INTERPS_CTRL_ALL_INFO:
		PrintSrEventSrInterPsCtrl((P_SR_EVENT_SR_INTERPS_CTRL_T)Data);
		break;
	case SR_EVENT_GET_SR_INTERPS_DBG_ALL_INFO:
		PrintSrEventSrInterPsDbg((P_SR_EVENT_SR_INTERPS_DBG_T)Data);
		break;
	case SR_EVENT_GET_SR_SIGA_FLAG_ALL_INFO:
		PrintSrEventSrSiga(pAd, (P_SR_EVENT_SR_SIGA_T)Data);
		break;
	case SR_EVENT_GET_SR_SIGA_AUTO_FLAG_ALL_INFO:
		PrintSrEventSrSigaAuto(pAd, (struct _SR_EVENT_SR_SIGA_AUTO_T *)Data);
		break;
	case SR_EVENT_GET_REMOTE_FH_RSSI:
	case SR_EVENT_GET_REMOTE_BH_INFO:
	case SR_EVENT_GET_MAP_TOPO:
		PrintSrEventMeshTopo(pAd, (struct _SR_EVENT_MESH_TOPOLOGY_T *)Data);
		break;
	case SR_EVENT_GET_MAP_TRAFFIC_STATUS:
		PrintSrEventSrUlStatus(pAd, (struct _SR_EVENT_SR_UL_TRAFFIC_STATUS_T *)Data);
		break;
	case SR_EVENT_SEND_MESH_UPLINK_TRAFFIC:
		ExtEventMeshUplinkTraffic(pAd, (struct _SR_EVENT_SR_UL_TRAFFIC_STATUS_T *)Data);
		break;
	case SR_EVENT_GET_MESH_PHASE:
		PrintSrEventSrMeshPhase(pAd, (struct _SR_EVENT_SR_MESH_PHASE_T *)Data);
		break;
	case SR_EVENT_GET_SR_CFG_MESH_REMOTE_STA_MODE:
		PrintSrEventSrRemoteAPStaAllHe(pAd, (struct _SR_EVENT_SR_REMOTE_AP_STA_ALL_HE_T *)Data);
		break;
	case SR_EVENT_GET_FH_RSSI_TH:
		PrintSrEventMeshFHRssi(pAd, (struct _SR_EVENT_MESH_FH_RSSI_TH_STATUS_T *)Data);
		break;
	case SR_EVENT_GET_MESH_STA_RSSI_TH:
		PrintSrEventMeshStaRssi(pAd, (struct SR_EVENT_SR_MESH_DL_STA_THRESHOLD_T *)Data);
		break;
	case SR_EVENT_GET_SR_BH_MESH_SRG_BITMAP:
	case SR_EVENT_GET_SR_FH_MESH_SRG_BITMAP:
		PrintSrEventSrSrgBitmap((P_SR_EVENT_SR_SRG_BITMAP_T)Data);
		break;
	case SR_EVENT_GET_BH_DL_MESH_SR_RSSI_TH:
	case SR_EVENT_GET_FH_DL_MESH_SR_RSSI_TH:
		PrintSrEventSrDlMeshRssi((P_SR_EVENT_T)Data);
		break;
	case SR_EVENT_GET_BH_FORBID_BITMAP:
		PrintSrEventBhForbidBitMap((P_SR_EVENT_T)Data);
	default:
		break;
	}
}

/* for check value */
NDIS_STATUS IsFlag(IN INT32 u4ArgVal, IN UINT_8 u1ArgNum)
{
	if (u4ArgVal != 0 && u4ArgVal != 1) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "ArgNum[%d] = %d is invalid Value! (ArgVal !=0 && ArgVal !=1)\n",
			  u1ArgNum, u4ArgVal);
		return NDIS_STATUS_FAILURE;
	}
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS IsInRange(IN INT32 u4ArgVal, IN UINT_8 u1ArgNum, IN INT32 u4Valfrom, IN INT32 u4Valto)
{
	if (u4ArgVal < u4Valfrom || u4ArgVal > u4Valto) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "ArgNum[%d] = %d is invalid Value! (ArgVal < %d or ArgVal > %d)\n",
			  u1ArgNum, u4ArgVal, u4Valfrom, u4Valto);
		return NDIS_STATUS_FAILURE;
	}
	return NDIS_STATUS_SUCCESS;
}

VOID PrintSrCmd(IN P_SR_CMD_T prSrCmd)
{
	MTWF_PRINT("%s:\nu1CmdSubId = %x, u1ArgNum = %d, u1DbdcIdx = %d, u1Status = %d\n"
		  "u1DropTaIdx = %d, u1StaIdx = %d, u4Value = %d\n",
		  __func__, prSrCmd->u1CmdSubId, prSrCmd->u1ArgNum, prSrCmd->u1DbdcIdx,
		  prSrCmd->u1Status, prSrCmd->u1DropTaIdx, prSrCmd->u1StaIdx, prSrCmd->u4Value);
}

VOID PrintSrCap(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCap)
{
	if (IS_SR_V1(pAd)) {
		P_WH_SR_CAP_T_SR_V1 prSrCap = (P_WH_SR_CAP_T_SR_V1)_prSrCap;

		MTWF_PRINT("%s:\nfgSrEn                 = %x, fgSrgEn              = %x, fgNonSrgEn              = %x\n"
		  "fgSingleMdpuRtsctsEn   = %x, fgHdrDurEn           = %x, fgTxopDurEn             = %x\n"
		  "fgNonSrgInterPpduPresv = %x, fgSrgInterPpduPresv  = %x, fgSrRemTimeEn           = %x\n"
		  "fgProtInSrWinDis       = %x, fgTxCmdDlRateSelEn   = %x, fgAmpduTxCntEn          = %x\n",
		  __func__, prSrCap->fgSrEn, prSrCap->fgSrgEn, prSrCap->fgNonSrgEn,
		  prSrCap->fgSingleMdpuRtsctsEn, prSrCap->fgHdrDurEn, prSrCap->fgTxopDurEn,
		  prSrCap->fgNonSrgInterPpduPresv, prSrCap->fgSrgInterPpduPresv,
		  prSrCap->fgSrRemTimeEn, prSrCap->fgProtInSrWinDis, prSrCap->fgTxCmdDlRateSelEn,
		  prSrCap->fgAmpduTxCntEn);
	}

	if (IS_SR_V2(pAd)) {
		P_WH_SR_CAP_T_SR_V2 prSrCap = (P_WH_SR_CAP_T_SR_V2)_prSrCap;

		MTWF_PRINT("%s:\nfgSrEn          = %x, fgSrgEn          = %x, fgNonSrgEn             = %x, fgSingleMdpuRtsctsEn = %x\n"
		  "fgHdrDurEn      = %x, fgTxopDurEn      = %x, fgNonSrgInterPpduPresv = %x, fgSrgInterPpduPresv  = %x\n"
		  "fgSMpduNoTrigEn = %x, fgSrgBssidOrder  = %x, fgCtsAfterRts          = %x, fgSrpOldRxvEn        = %x\n"
		  "fgSrpNewRxvEn   = %x, fgSrpDataOnlyEn  = %x, fgFixedRateSrREn       = %x, fgWtblSrREn          = %x\n"
		  "fgSrRemTimeEn   = %x, fgProtInSrWinDis = %x, fgTxCmdDlRateSelEn     = %x, fgAmpduTxCntEn       = %x\n",
		  __func__, prSrCap->fgSrEn, prSrCap->fgSrgEn, prSrCap->fgNonSrgEn, prSrCap->fgSingleMdpuRtsctsEn,
		  prSrCap->fgHdrDurEn, prSrCap->fgTxopDurEn, prSrCap->fgNonSrgInterPpduPresv, prSrCap->fgSrgInterPpduPresv,
		  prSrCap->fgSMpduNoTrigEn, prSrCap->fgSrgBssidOrder, prSrCap->fgCtsAfterRts, prSrCap->fgSrpOldRxvEn,
		  prSrCap->fgSrpNewRxvEn, prSrCap->fgSrpDataOnlyEn, prSrCap->fgFixedRateSrREn, prSrCap->fgWtblSrREn,
		  prSrCap->fgSrRemTimeEn, prSrCap->fgProtInSrWinDis, prSrCap->fgTxCmdDlRateSelEn, prSrCap->fgAmpduTxCntEn);
	}
}

VOID PrintSrPara(IN P_WH_SR_PARA_T prSrPara)
{
	MTWF_PRINT("%s:\nu1NonSrgPdThr   = %x, u1SrgPdThr        = %x, u1PeriodOfst   = %x\n"
		  "u1RcpiSourceSel = %x, u2ObssPdMin       = %x, u2ObssPdMinSrg = %x\n"
		  "eRespTxPwrMode  = %x, eTxPwrRestricMode = %x, u1ObssTxPwrRef = %x\n",
		  __func__, prSrPara->u1NonSrgPdThr, prSrPara->u1SrgPdThr, prSrPara->u1PeriodOfst,
		  prSrPara->u1RcpiSourceSel, prSrPara->u2ObssPdMin, prSrPara->u2ObssPdMinSrg,
		  prSrPara->eRespTxPwrMode, prSrPara->eTxPwrRestricMode, prSrPara->u1ObssTxPwrRef);
}

VOID PrintSrInd(IN P_WH_SR_IND_T prSrInd)
{
	MTWF_PRINT("%s:\nu1NonSrgInterPpduRcpi   = %x, u1SrgInterPpduRcpi     = %x\n"
		  "u2NonSrgVldCnt          = %x, u2SrgVldCnt            = %x\n"
		  "u2IntraBssPpduCnt       = %x, u2InterBssPpduCnt      = %x\n"
		  "u2NonSrgPpduVldCnt      = %x, u2SrgPpduVldCnt        = %x\n"
		  "u4SrAmpduMpduCnt        = %x, u4SrAmpduMpduAckedCnt  = %x\n",
		  __func__, prSrInd->u1NonSrgInterPpduRcpi, prSrInd->u1SrgInterPpduRcpi,
		  prSrInd->u2NonSrgVldCnt, prSrInd->u2SrgVldCnt, prSrInd->u2IntraBssPpduCnt,
		  prSrInd->u2InterBssPpduCnt, prSrInd->u2NonSrgPpduVldCnt, prSrInd->u2SrgPpduVldCnt,
		  prSrInd->u4SrAmpduMpduCnt, prSrInd->u4SrAmpduMpduAckedCnt);
}

VOID PrintSrGloVarSingleDropTa(IN P_SR_GLOBAL_VAR_SINGLE_DROP_TA_T prSrGlobalVarSingleDropTa,
			       IN UINT_8 u1DropTaIdx, IN UINT_8 u1StaIdx)
{
	MTWF_PRINT("%s:\nSR Info - u1CurSrDropTaIdx = %d, u2SrTtlTxCntThr=%d\n",
		  __func__, prSrGlobalVarSingleDropTa->u1CurSrDropTaIdx,
		  prSrGlobalVarSingleDropTa->u2SrTtlTxCntThr);

	PrintSrDropTaInfo(&(prSrGlobalVarSingleDropTa->rSrDropTaInfo), u1DropTaIdx, u1StaIdx);

}

VOID PrintSrDropTaInfo(IN P_SR_DROP_TA_INFO_T prSrDropTaInfo, IN UINT_8 u1DropTaIdx,
		       IN UINT_8 u1StaIdx)
{
	MTWF_PRINT("    DropTa %2d - Address : %x\n",
		  u1DropTaIdx, prSrDropTaInfo->u4Address2);

	if (u1StaIdx == SR_STA_NUM) {
		for (u1StaIdx = 0; u1StaIdx < SR_STA_NUM; u1StaIdx++) {
			PrintSrStaInfo(&(prSrDropTaInfo->rSrStaInfo[u1StaIdx]), u1StaIdx);
		}
	} else {
		PrintSrStaInfo(&(prSrDropTaInfo->rSrStaInfo[u1StaIdx]), u1StaIdx);
	}

}

VOID PrintSrStaInfo(IN P_SR_STA_INFO_T prSrStaInfo, IN UINT_8 u1StaIdx)
{
	UINT_32 per = prSrStaInfo->u2SrTtlTxCnt == 0 ? 0 : 1000 * (prSrStaInfo->u2SrTtlTxCnt - prSrStaInfo->u2SrSucCnt) / prSrStaInfo->u2SrTtlTxCnt;
	CHAR *mode[2] = {"AUTO", "FIXED"};
	CHAR *state[5] = {"Invailid", "Not Stable", "Good", "Bad", "Timeout"};
	CHAR *rastate[2] = {"Stable", "Active"};

	MTWF_PRINT("        STA %2d\n"
		  "        u2WlanId       = %d, u1Mode       = %s,  u1State = %s\n",
		  u1StaIdx, prSrStaInfo->u2WlanId, mode[prSrStaInfo->u1Mode], state[prSrStaInfo->u1State]);

	MTWF_PRINT("        u1SrRateOffset = %d, u1SrRaTryCnt = %x, u1SrRaRound = %x, u1SrRaState = %s\n",
		  prSrStaInfo->u1SrRateOffset, prSrStaInfo->u1SrRaTryCnt, prSrStaInfo->u1SrRaRound, rastate[prSrStaInfo->u1SrRaState]);

	MTWF_PRINT("        u2SrSucCnt  = %x, u2SrTtlTxCnt = %x, PER = %d.%1d%%\n",
		  prSrStaInfo->u2SrSucCnt, prSrStaInfo->u2SrTtlTxCnt, per / 10, per % 10);

	MTWF_PRINT("        u4Score = %x, u2BadQuota = %d, u1BadLevel = %d, u1SrRate = %x\n",
		  prSrStaInfo->u4Score, prSrStaInfo->u2BadQuota, prSrStaInfo->u1BadLevel, prSrStaInfo->u1SrRate);

}

VOID PrintSrCond(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCond)
{
	if (IS_SR_V1(pAd)) {
		P_SR_COND_T_SR_V1 prSrCond = (P_SR_COND_T_SR_V1)_prSrCond;

		MTWF_PRINT("%s:\nfgSrRcpiCckRateEn = %x, fgSrMacRcpiRateEn = %x, fgSrRxvRcpiRateEn = %x\n"
			  "fgSrRcpiHeRateEn  = %x, fgSrRcpiVhtRateEn = %x, fgSrRcpiHtRateEn  = %x\n"
			  "fgSrRcpiLgRateEn  = %x, fgSrRxvEntry 	 = %x, fgSrPeriodLimitEn = %x\n"
			  "u1SrPeriodLimit	 = %x\n",
			  __func__, prSrCond->fgSrRcpiCckRateEn, prSrCond->fgSrMacRcpiRateEn, prSrCond->fgSrRxvRcpiRateEn,
			  prSrCond->fgSrRcpiHeRateEn, prSrCond->fgSrRcpiVhtRateEn, prSrCond->fgSrRcpiHtRateEn,
			  prSrCond->fgSrRcpiLgRateEn, prSrCond->fgSrRxvEntry, prSrCond->fgSrPeriodLimitEn,
			  prSrCond->u1SrPeriodLimit);
	}

	if (IS_SR_V2(pAd)) {
		P_WH_SR_COND_T_SR_V2 prSrCond = (P_WH_SR_COND_T_SR_V2)_prSrCond;

		MTWF_PRINT("%s:\nfgSrRcpiSel      = %x, fgSrRcpiCckRateEn = %x, fgSrMacRcpiRateEn = %x, fgSrRxvRcpiRateEn = %x\n"
			  "fgSrRcpiHeRateEn = %x, fgSrRcpiVhtRateEn = %x, fgSrRcpiHtRateEn  = %x, fgSrRcpiLgRateEn  = %x\n"
			  "fgSrRxvEntry     = %x, fgSrPeriodLimitEn = %x, u1SrPeriodLimit   = %x\n",
			  __func__, prSrCond->fgSrRcpiSel, prSrCond->fgSrRcpiCckRateEn, prSrCond->fgSrMacRcpiRateEn, prSrCond->fgSrRxvRcpiRateEn,
			  prSrCond->fgSrRcpiHeRateEn, prSrCond->fgSrRcpiVhtRateEn, prSrCond->fgSrRcpiHtRateEn, prSrCond->fgSrRcpiLgRateEn,
			  prSrCond->fgSrRxvEntry, prSrCond->fgSrPeriodLimitEn, prSrCond->u1SrPeriodLimit);
	}
}

VOID PrintSrRcpiTbl(IN P_WH_SR_RCPITBL_T prSrRcpiTbl)
{
	UINT_8 u1Index;

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "\n");
	for (u1Index = 0; u1Index < SR_RCPITBL_MCS_NUM; u1Index++)
		MTWF_PRINT("u1RcpiTblMcs[%d] = %x (%d dBm)\n", u1Index, prSrRcpiTbl->u1RcpiTblMcs[u1Index], SRDbmConv(prSrRcpiTbl->u1RcpiTblMcs[u1Index]));
}

VOID PrintSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID *_prSrRcpiTblOfst)
{
	if (IS_SR_V1(pAd)) {
		P_SR_RCPITBL_OFST_T_SR_V1 prSrRcpiTblOfst = (P_SR_RCPITBL_OFST_T_SR_V1)_prSrRcpiTblOfst;

		MTWF_PRINT("%s:\nu2RxBwRcpiOfst = %x, u2StbcRcpiOfst = %x, u2NumAntRcpiOfst = %x\n"
			  "u2LdpcRcpiOfst = %x, u2DcmRcpiOfst  = %x, u2MacRcpiOfst	  = %x\n"
			  "u2SigRcpiOfst  = %x\n",
			  __func__, prSrRcpiTblOfst->u2RxBwRcpiOfst, prSrRcpiTblOfst->u2StbcRcpiOfst, prSrRcpiTblOfst->u2NumAntRcpiOfst,
			  prSrRcpiTblOfst->u2LdpcRcpiOfst, prSrRcpiTblOfst->u2DcmRcpiOfst, prSrRcpiTblOfst->u2MacRcpiOfst,
			  prSrRcpiTblOfst->u2SigRcpiOfst);
	}

	if (IS_SR_V2(pAd)) {
		P_WH_SR_RCPITBL_OFST_T_SR_V2 prSrRcpiTblOfst = (P_WH_SR_RCPITBL_OFST_T_SR_V2)_prSrRcpiTblOfst;

		MTWF_PRINT("%s:\nu2RxBwRcpiOfst = %x, u2StbcRcpiOfst = %x, u2NumAntRcpiOfst = %x\n"
			  "u2LdpcRcpiOfst = %x, u2DcmRcpiOfst  = %x, u2MacRcpiOfst    = %x\n"
			  "u2SigRcpiOfst  = %x, u2BfRcpiOfst   = %x\n",
			  __func__, prSrRcpiTblOfst->u2RxBwRcpiOfst, prSrRcpiTblOfst->u2StbcRcpiOfst, prSrRcpiTblOfst->u2NumAntRcpiOfst,
			  prSrRcpiTblOfst->u2LdpcRcpiOfst, prSrRcpiTblOfst->u2DcmRcpiOfst, prSrRcpiTblOfst->u2MacRcpiOfst,
			  prSrRcpiTblOfst->u2SigRcpiOfst, prSrRcpiTblOfst->u2BfRcpiOfst);
	}
}

VOID PrintSrQCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrQCtrl)
{
	if (IS_SR_V1(pAd)) {
		P_WH_SR_QUEUE_CTRL_T_SR_V1 prSrQCtrl = (P_WH_SR_QUEUE_CTRL_T_SR_V1)_prSrQCtrl;

		MTWF_PRINT("%s:\nfgSrRxRptEn = %x, fgSrCw = %x, fgSrSuspend = %x, u4SrBackOffMask = %x\n",
		  __func__, prSrQCtrl->fgSrRxRptEn, prSrQCtrl->fgSrCw, prSrQCtrl->fgSrSuspend, prSrQCtrl->u4SrBackOffMask);
	}

	if (IS_SR_V2(pAd)) {
		P_WH_SR_QUEUE_CTRL_T_SR_V2 prSrQCtrl = (P_WH_SR_QUEUE_CTRL_T_SR_V2)_prSrQCtrl;

		MTWF_PRINT("%s:\nfgSrRxRptEn     = %x, fgSrCw            = %x, fgSrSuspend = %x, fgSrDisSwAifsDis = %x\n"
			  "u4SrBackOffMask = %x, u4SrBackOffEnable = %x\n",
			  __func__, prSrQCtrl->fgSrRxRptEn, prSrQCtrl->fgSrCw, prSrQCtrl->fgSrSuspend, prSrQCtrl->fgSrDisSwAifsDis,
			  prSrQCtrl->u4SrBackOffMask, prSrQCtrl->u4SrBackOffEnable);
	}
}

VOID PrintSrIBPD(IN P_WH_SR_IBPD_T prSrIBPD)
{
	MTWF_PRINT("%s:\nu1InterBssByHdrBssid = %x, u1InterBssByMu        = %x, u1InterBssByPbssColor = %x\n"
		  "u1InterBssByPaid     = %x, u1InterBssByBssColor  = %x\n",
		  __func__, prSrIBPD->u1InterBssByHdrBssid, prSrIBPD->u1InterBssByMu, prSrIBPD->u1InterBssByPbssColor,
		  prSrIBPD->u1InterBssByPaid, prSrIBPD->u1InterBssByBssColor);
}

VOID PrintSrNRT(IN PRTMP_ADAPTER pAd, IN VOID *_prSrNRT)
{
	if (IS_SR_V1(pAd)) {
		P_SR_NRT_T_SR_V1 prSrNRT = (P_SR_NRT_T_SR_V1)_prSrNRT;

		MTWF_PRINT("u1TableIdx = %x, u4NRTValue = %x\n",
			prSrNRT->u1TableIdx, prSrNRT->u4NRTValue);
	}

	if (IS_SR_V2(pAd)) {
		P_WH_SR_NRT_T_SR_V2 prSrNRT = (P_WH_SR_NRT_T_SR_V2)_prSrNRT;

		MTWF_PRINT("u1TableIdx = %d, u1RaTaSel = %x, fgSwProtect = %x, u4NRTValue = %x\n",
			  prSrNRT->u1TableIdx, prSrNRT->u1RaTaSel, prSrNRT->fgSwProtect, prSrNRT->u4NRTValue);
	}
}

VOID PrintSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrNRTCtrl)
{
	if (IS_SR_V1(pAd)) {
		P_SR_NRT_CTRL_T_SR_V1 prSrNRTCtrl = (P_SR_NRT_CTRL_T_SR_V1)_prSrNRTCtrl;

		MTWF_PRINT("%s:\nfgSrtEn       = %x, fgSrtSrpEn     = %x, fgSrtAddrOrderEn = %x\n"
			  "u2SrtInRcpiTh = %x, u2SrtOutRcpiTh = %x, u2SrtUsedCntTh   = %x\n",
			  __func__, prSrNRTCtrl->fgSrtEn, prSrNRTCtrl->fgSrtSrpEn, prSrNRTCtrl->fgSrtAddrOrderEn,
			  prSrNRTCtrl->u2SrtInRcpiTh, prSrNRTCtrl->u2SrtOutRcpiTh, prSrNRTCtrl->u2SrtUsedCntTh);
	}

	if (IS_SR_V2(pAd)) {
		P_WH_SR_NRT_CTRL_T_SR_V2 prSrNRTCtrl = (P_WH_SR_NRT_CTRL_T_SR_V2)_prSrNRTCtrl;

		MTWF_PRINT("%s:\nfgSrtEn       = %x, fgSrtSrpEn     = %x, fgSrtAddrOrderEn = %x, fgSrtByPassCtsAck = %x\n"
			  "u2SrtInRcpiTh = %x, u2SrtOutRcpiTh = %x, u2SrtUsedCntTh   = %x\n",
			  __func__, prSrNRTCtrl->fgSrtEn, prSrNRTCtrl->fgSrtSrpEn, prSrNRTCtrl->fgSrtAddrOrderEn, prSrNRTCtrl->fgSrtByPassCtsAck,
			  prSrNRTCtrl->u2SrtInRcpiTh, prSrNRTCtrl->u2SrtOutRcpiTh, prSrNRTCtrl->u2SrtUsedCntTh);
	}
}

VOID PrintSrFNQCtrl(IN P_WH_SR_FNQ_CTRL_T prSrFNQCtrl)
{
	MTWF_PRINT("u2SrpCondDis = %x, u1PeriodOfst  = %x, fgHdrDurEn      = %x\n"
		  "fgTxopDurEn  = %x, fgSrpCfendRst = %x, fgSrpNavToutRst = %x\n",
		  prSrFNQCtrl->u2SrpCondDis, prSrFNQCtrl->u1PeriodOfst, prSrFNQCtrl->fgHdrDurEn,
		  prSrFNQCtrl->fgTxopDurEn, prSrFNQCtrl->fgSrpCfendRst, prSrFNQCtrl->fgSrpNavToutRst);
}

VOID PrintSrFrmFilt(IN UINT_32 *pu4SrFrmFilt)
{
	MTWF_PRINT("u4SrFrmFilt = %x\n", *pu4SrFrmFilt);
}

VOID PrintSrInterPsCtrl(IN P_WH_SR_INTERPS_CTRL_T prSrInterPsCtrl)
{
	MTWF_PRINT("u1CondDis = %x, u1DurAdj   = %x, u1DurLmt    = %x\n"
		  "u1EntryEn = %x, fgDurLmtEn = %x, fgInterpsEn = %x\n",
		  prSrInterPsCtrl->u1CondDis, prSrInterPsCtrl->u1DurAdj, prSrInterPsCtrl->u1DurLmt,
		  prSrInterPsCtrl->u1EntryEn, prSrInterPsCtrl->fgDurLmtEn, prSrInterPsCtrl->fgInterpsEn);
}

VOID PrintSrInterPsDbg(IN P_WH_SR_INTERPS_DBG_T prSrInterPsDbg)
{
	MTWF_PRINT("u1Entry0Cnt = %x, u1Entry1Cnt   = %x, u1Entry2Cnt    = %x\n"
		  "u1EntryLat  = %x\n",
		  prSrInterPsDbg->u1Entry0Cnt, prSrInterPsDbg->u1Entry1Cnt, prSrInterPsDbg->u1Entry2Cnt,
		  prSrInterPsDbg->u1EntryLat);
}

VOID PrintSrSrgBitmap(IN UINT_8 u1DbdcIdx, IN P_SR_SRG_BITMAP_T prSrSrgBitmap)
{
	UINT_8 u1BitmapIdx = 0, u1ColorEn = 0, u1pBssidEn = 0;
	CHAR *enable[2] = {" ", "V"};
	/*UINT_32 u4Color = 0, u4pBssid = 0;*/

	MTWF_PRINT("Color - 31_0:%x, 63_32:%x pBssid - 31_0:%x, 63_32:%x\n",
		prSrSrgBitmap->u4Color_31_0[u1DbdcIdx], prSrSrgBitmap->u4Color_63_32[u1DbdcIdx],
		prSrSrgBitmap->u4pBssid_31_0[u1DbdcIdx], prSrSrgBitmap->u4pBssid_63_32[u1DbdcIdx]);
	MTWF_PRINT("BIT  Color  pBssid\n");


	for (u1BitmapIdx = 0; u1BitmapIdx < 64; u1BitmapIdx++) {
		if (u1BitmapIdx < 32) {
			u1ColorEn = (prSrSrgBitmap->u4Color_31_0[u1DbdcIdx] & BIT(u1BitmapIdx)) >> u1BitmapIdx;
			u1pBssidEn = (prSrSrgBitmap->u4pBssid_31_0[u1DbdcIdx] & BIT(u1BitmapIdx)) >> u1BitmapIdx;
		} else {
			u1ColorEn = (prSrSrgBitmap->u4Color_63_32[u1DbdcIdx] & BIT(u1BitmapIdx - 32)) >> (u1BitmapIdx - 32);
			u1pBssidEn = (prSrSrgBitmap->u4pBssid_63_32[u1DbdcIdx] & BIT(u1BitmapIdx - 32)) >> (u1BitmapIdx - 32);
		}

		MTWF_PRINT("%2d     %s      %s   \n",
		u1BitmapIdx, enable[u1ColorEn], enable[u1pBssidEn]);
	}
}

VOID PrintSrDlMeshRssi(IN UINT_8 EventSubId, IN UINT_8 u1DbdcIdx, IN INT_8 i1Rssi)
{
	if (EventSubId == SR_EVENT_GET_BH_DL_MESH_SR_RSSI_TH)
		MTWF_PRINT("u1DbdcIdx = %d, BhDlSrThresh = %d\n",
			u1DbdcIdx, i1Rssi);
	else if (EventSubId == SR_EVENT_GET_FH_DL_MESH_SR_RSSI_TH)
		MTWF_PRINT("u1DbdcIdx = %d, FhDlSrThresh = %d\n",
			u1DbdcIdx, i1Rssi);
}

VOID PrintSrSiga(IN PRTMP_ADAPTER pAD, IN UINT_8 u1DbdcIdx, IN P_SR_SIGA_FLAG_T prSrSigaflag, IN BOOLEAN fgread)
{
	INT i1devidx;
	UINT8 u1Bssid;
	POS_COOKIE pObj = (POS_COOKIE) pAD->OS_Cookie;
	struct wifi_dev *counterdev;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAD, pObj->ioctl_if, pObj->ioctl_if_type);
	RTMP_STRING *flaginfo = "Reserved";

	if (fgread) {
		for (i1devidx = 0; i1devidx < WDEV_NUM_MAX; i1devidx++) {
			if (pAD->wdev_list[i1devidx] == NULL)
				break;
			counterdev = pAD->wdev_list[i1devidx];

			u1Bssid = counterdev->DevInfo.OwnMacIdx > 3 ? counterdev->DevInfo.OwnMacIdx - SR_BSSID_OMAC_OFFSET : counterdev->DevInfo.OwnMacIdx;
			flaginfo = "Reserved";
			if (counterdev->if_up_down_state) {
				if (u1DbdcIdx == HcGetBandByWdev(counterdev)) {
					for (PENUM_WH_SR_SIGA_FLAG_T = ENUM_WH_SR_SIGA_FLAG_T; PENUM_WH_SR_SIGA_FLAG_T->name ; PENUM_WH_SR_SIGA_FLAG_T++) {
						if (PENUM_WH_SR_SIGA_FLAG_T->u1srflag == prSrSigaflag->u1SigaFlag[u1Bssid]) {
							flaginfo = PENUM_WH_SR_SIGA_FLAG_T->name;
							break;
						}
					}
					MTWF_PRINT("Interface = %s  omac_indx = %2d, Flag = %2d (%s)\n",
								RtmpOsGetNetDevName(counterdev->if_dev), counterdev->DevInfo.OwnMacIdx, prSrSigaflag->u1SigaFlag[u1Bssid], flaginfo);
				}
			}
		}
	} else {
		u1Bssid = wdev->DevInfo.OwnMacIdx > 3 ? wdev->DevInfo.OwnMacIdx - SR_BSSID_OMAC_OFFSET : wdev->DevInfo.OwnMacIdx;

		for (PENUM_WH_SR_SIGA_FLAG_T = ENUM_WH_SR_SIGA_FLAG_T; PENUM_WH_SR_SIGA_FLAG_T->name ; PENUM_WH_SR_SIGA_FLAG_T++) {
			if (PENUM_WH_SR_SIGA_FLAG_T->u1srflag == prSrSigaflag->u1SigaFlag[u1Bssid]) {
				flaginfo = PENUM_WH_SR_SIGA_FLAG_T->name;
				break;
			}
		}
		MTWF_PRINT("Interface = %s  omac_indx = %2d, Flag = %2d (%s)\n",
					RtmpOsGetNetDevName(wdev->if_dev), wdev->DevInfo.OwnMacIdx, prSrSigaflag->u1SigaFlag[u1Bssid], flaginfo);
	}
}

VOID PrintSrMeshTopo(IN UINT_8 u1SubId, IN union _SR_MESH_TOPOLOGY_T *prSrMeshTopo)
{
	UINT_8 cmd_u1SubId = u1SubId;
	UINT_8 event_u1SubId = u1SubId;
	if (cmd_u1SubId == SR_CMD_SET_REMOTE_FH_RSSI || event_u1SubId == SR_EVENT_GET_REMOTE_FH_RSSI)
		MTWF_PRINT("u1CmdSubId:%u i1Rssi:%d FHStat:%u\n",
			u1SubId, prSrMeshTopo->rRemoteFhParams.i1Rssi,
			prSrMeshTopo->rRemoteFhParams.u1RemoteFhStat);
	else if (cmd_u1SubId == SR_CMD_SET_REMOTE_BH_INFO || event_u1SubId == SR_EVENT_GET_REMOTE_BH_INFO)
		MTWF_PRINT("u1CmdSubId:%u u2RemoteBhWcid:%u u1RemoteBhType:%u\n",
			u1SubId, prSrMeshTopo->rRemoteBhParams.u2RemoteBhWcid,
			prSrMeshTopo->rRemoteBhParams.u1RemoteBhType);
	else if (cmd_u1SubId == SR_CMD_SET_MAP_TOPO || event_u1SubId == SR_EVENT_GET_MAP_TOPO)
		MTWF_PRINT("u1CmdSubId:%u u1MapDevCount:%u u1MapDevSrSupportMode:%u u1SelfRole:%u\n",
			u1SubId, prSrMeshTopo->rMapTopoParams.u1MapDevCount,
			prSrMeshTopo->rMapTopoParams.u1MapDevSrSupportMode,
			prSrMeshTopo->rMapTopoParams.u1SelfRole);
	else
		return;
}

VOID PrintSrMeshFHRssiTh(IN UINT_8 u1SubId, IN struct _SR_MESH_FH_RSSI_TH_T  *prSrMeshFHRssi)
{
	MTWF_PRINT("u1CmdSubId:%u i1SrMeshFhRssiTh:%d, i1SrMeshSrAllowTargetTh:%d, i1SrMeshInterSrAllowTh:%d\n",
		u1SubId, prSrMeshFHRssi->i1SrMeshFhRssiTh,
		prSrMeshFHRssi->i1SrMeshSrAllowTargetTh,
		prSrMeshFHRssi->i1SrMeshInterSrAllowTh);
}

VOID PrintSrMeshStaRssiTh(IN UINT_8 u1SubId, IN struct SR_MESH_SR_DL_STA_THRESHOLD_T  *prSrMeshStaRssi)
{
	MTWF_PRINT("u1CmdSubId:%u u1BSSID:%d, Rssi:%d\n",
		u1SubId,
		prSrMeshStaRssi->u1BSSID,
		prSrMeshStaRssi->irssi);
}

VOID PrintSrCnt(IN UINT_8 u1DbdcIdx, IN P_SR_CNT_T prSrCnt, IN UINT8 u1PpduType)
{
	UINT_8  u1SrRxrptSrc = ENUM_SR_RXRPT_SRC_RXRPT;
	UINT_8  u1SrEntry = ENUM_SR_ENTRY_NEWRXV;
	CHAR *srrxrptsrc[2] = {"RXRPT", "CMDRPT-TX"};

	UINT_16 au2RxrptTtl[ENUM_SR_RXRPT_SRC_NUM]     = {0};
	UINT_16	au2PeriodSucTtl[ENUM_SR_RXRPT_SRC_NUM] = {0}, au2PeriodFailTtl[ENUM_SR_RXRPT_SRC_NUM]  = {0};
	UINT_16 au2GenTxcSucTtl[ENUM_SR_RXRPT_SRC_NUM] = {0}, au2GenTxcFailTtl[ENUM_SR_RXRPT_SRC_NUM]  = {0};
	UINT_16 au2SrTxSucTtl[ENUM_SR_RXRPT_SRC_NUM]   = {0}, au2SrTxFailTtl[ENUM_SR_RXRPT_SRC_NUM]    = {0};

	if (u1PpduType == ENUM_SR_NONSRG_OBSS_OR_NONHE) {
		MTWF_PRINT("Band%d Reuse NonSgr:OBSS Packets\n", u1DbdcIdx);
	} else if (u1PpduType == ENUM_SR_NONSRG_MESH_HE_INTERNAL_NO_SR) {
		MTWF_PRINT("Band%d Reuse NonSgr:Mesh HE Internal No SR Packets\n", u1DbdcIdx);
	} else if (u1PpduType == ENUM_SR_NONSRG_MESH_HE) {
		MTWF_PRINT("Band%d Reuse NonSgr:Mesh HE Packets\n", u1DbdcIdx);
	} else {
		MTWF_PRINT("Arg. is Invaild! 0:OBSS 1:Mesh HE RESTR 2:Mesh HE\n");
		return;
	}

	for (u1SrRxrptSrc = ENUM_SR_RXRPT_SRC_RXRPT; u1SrRxrptSrc < ENUM_SR_RXRPT_SRC_NUM; u1SrRxrptSrc++) {

		for (u1SrEntry = ENUM_SR_ENTRY_NEWRXV; u1SrEntry < ENUM_SR_ENTRY_NUM; u1SrEntry++) {
			au2RxrptTtl[u1SrRxrptSrc]     += prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][u1SrEntry][u1PpduType].u2EntryTtl;
			au2PeriodSucTtl[u1SrRxrptSrc] += prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][u1SrEntry][u1PpduType].u2PeriodSuc;
			au2GenTxcSucTtl[u1SrRxrptSrc] += prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][u1SrEntry][u1PpduType].u2GenTxcSuc;
			au2SrTxSucTtl[u1SrRxrptSrc]   += prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][u1SrEntry][u1PpduType].u2SrTxSuc;
		}
		au2SrTxFailTtl[u1SrRxrptSrc] = au2GenTxcSucTtl[u1SrRxrptSrc] - au2SrTxSucTtl[u1SrRxrptSrc];
		au2GenTxcFailTtl[u1SrRxrptSrc] = au2PeriodSucTtl[u1SrRxrptSrc] - au2GenTxcSucTtl[u1SrRxrptSrc];
		au2PeriodFailTtl[u1SrRxrptSrc] = au2RxrptTtl[u1SrRxrptSrc] - au2PeriodSucTtl[u1SrRxrptSrc];

		if (u1SrRxrptSrc == ENUM_SR_RXRPT_SRC_RXRPT) {
			au2RxrptTtl[ENUM_SR_RXRPT_SRC_RXRPT] += prSrCnt->u2EntryNoSrTtl[u1DbdcIdx];
			au2RxrptTtl[ENUM_SR_RXRPT_SRC_RXRPT] += prSrCnt->u2EntryFailTtl[u1DbdcIdx];
		}

		MTWF_PRINT("-------------------------------------------------------------------\n"
			  "SR Rxrpt Source : %s\n"
			  "Total Rxrpt  = %4x,\n"
			  "      NewRxv = %4x, OriRxvVht = %4x, OriRxvHe = %4x,\n"
			  "      NewMac = %4x, OriMac    = %4x,\n",
			  srrxrptsrc[u1SrRxrptSrc],
			  au2RxrptTtl[u1SrRxrptSrc],
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2EntryTtl,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2EntryTtl,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2EntryTtl,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2EntryTtl,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2EntryTtl);

		if (u1SrRxrptSrc == ENUM_SR_RXRPT_SRC_RXRPT) {
			MTWF_PRINT("      NoSr   = %4x, Fail      = %4x,\n"
				  "-------------------------------------------------------------------\n",
				  prSrCnt->u2EntryNoSrTtl[u1DbdcIdx],
				  prSrCnt->u2EntryFailTtl[u1DbdcIdx]);
		} else {
			MTWF_PRINT("-------------------------------------------------------------------\n");
		}

		MTWF_PRINT("Total Period    Succ  = %4x, Fail = %4x,\n"
			  "      NewRxv    Succ  = %4x, Fail = %4x,\n"
			  "      OriRxvVht Succ  = %4x, Fail = %4x,\n"
			  "      OriRxvHe  Succ  = %4x, Fail = %4x,\n"
			  "      NewMac    Succ  = %4x, Fail = %4x,\n"
			  "      OriMac    Succ  = %4x, Fail = %4x,\n"
			  "-------------------------------------------------------------------\n",
			  au2PeriodSucTtl[u1SrRxrptSrc], au2PeriodFailTtl[u1SrRxrptSrc],
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2EntryTtl - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2EntryTtl - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2EntryTtl - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2EntryTtl - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2EntryTtl - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2PeriodSuc);

		MTWF_PRINT("Total Gen Txc   Succ  = %4x, Fail = %4x,\n"
			  "      NewRxv    Succ  = %4x, Fail = %4x,\n"
			  "      OriRxvVht Succ  = %4x, Fail = %4x,\n"
			  "      OriRxvHe  Succ  = %4x, Fail = %4x,\n"
			  "      NewMac    Succ  = %4x, Fail = %4x,\n"
			  "      OriMac    Succ  = %4x, Fail = %4x,\n"
			  "-------------------------------------------------------------------\n",
			  au2GenTxcSucTtl[u1SrRxrptSrc], au2GenTxcFailTtl[u1SrRxrptSrc],
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2PeriodSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2PeriodSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2PeriodSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2PeriodSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2PeriodSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2GenTxcSuc);


		MTWF_PRINT("Total SR Tx     Succ  = %4x, Fail = %4x,\n"
			  "      NewRxv    Succ  = %4x, Fail = %4x,\n"
			  "      OriRxvVht Succ  = %4x, Fail = %4x,\n"
			  "      OriRxvHe  Succ  = %4x, Fail = %4x,\n"
			  "      NewMac    Succ  = %4x, Fail = %4x,\n"
			  "      OriMac    Succ  = %4x, Fail = %4x,\n"
			  "-------------------------------------------------------------------\n",
			  au2SrTxSucTtl[u1SrRxrptSrc], au2SrTxFailTtl[u1SrRxrptSrc],
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2GenTxcSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV][u1PpduType].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2GenTxcSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT][u1PpduType].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2GenTxcSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE][u1PpduType].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2GenTxcSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC][u1PpduType].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2GenTxcSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC][u1PpduType].u2SrTxSuc);
	}
}

VOID PrintSrSd(IN UINT_8 u1DbdcIdx, IN P_SR_SD_T prSrSd)
{
	UINT_8 u1McsIdx = 0;
	CHAR *srsdrules[4] = {"1 - NO CONNECTED", "2 - NO CONGESTION", "3 - NO INTERFERENCE", "4 - SR ON"};
	CHAR *srtxstate[9] = {"1 - ON OM INIT", "2 - OFF SRSD FAIL", "3 - ON SRSD PASS", "4 - ON LOW TPUT", "5 - ON OM START", "6 - ON CONT WEAK", "7 - ON CONT NORM", "8 - ON LONG PKT CHK", "9 - OFF ENV DETECTED"};

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Band%d\n", u1DbdcIdx);

	MTWF_PRINT("-------------------------------------------------------------------\n"
		  "Hit Rule      = %s\n"
		  "Rxrpt Count   = %d\n"
		  " 	 RXV     = %d\n"
		  " 	 MAC     = %d\n"
		  "-------------------------------------------------------------------\n",
		  srsdrules[prSrSd->u1Rule[u1DbdcIdx]],
		  prSrSd->u2RxrptRxvCnt[u1DbdcIdx] + prSrSd->u2RxrptMacCnt[u1DbdcIdx],
		  prSrSd->u2RxrptRxvCnt[u1DbdcIdx],
		  prSrSd->u2RxrptMacCnt[u1DbdcIdx]);

	MTWF_PRINT("Timer Period  = %d(us)\n"
		  " 	 HwSR Period Ratio     = %d.%1d%%\n"
		  " 	 SwSR Period Ratio     = %d.%1d%%\n"
		  " 	 Congestion  Ratio     = %d.%1d%%\n"
		  "-------------------------------------------------------------------\n",
		  SR_SCENE_DETECTION_TIMER_PERIOD_MS * 1000,
		  prSrSd->u4HWSrPeriodRatio[u1DbdcIdx] / 10,
		  prSrSd->u4HWSrPeriodRatio[u1DbdcIdx] % 10,
		  prSrSd->u4SWSrPeriodRatio[u1DbdcIdx] / 10,
		  prSrSd->u4SWSrPeriodRatio[u1DbdcIdx] % 10,
		  prSrSd->u4TtlAirTimeRatio[u1DbdcIdx] / 10,
		  prSrSd->u4TtlAirTimeRatio[u1DbdcIdx] % 10);

	MTWF_PRINT("Total Airtime = %d(us)\n"
		  " 	 ChBusy                = %d\n"
		  " 	 SrTx                  = %d\n"
		  " 	 OBSS                  = %d\n"
		  " 	 Delta                 = %d\n"
		  " 	 MyTx                  = %d\n"
		  " 	 MyRx                  = %d\n"
		  " 	 Interference   Ratio  = %d.%1d%%\n"
		  "-------------------------------------------------------------------\n",
		  prSrSd->u4TtlAirTime[u1DbdcIdx],
		  prSrSd->u4ChannelBusyTime[u1DbdcIdx],
		  prSrSd->u4SrTxAirtime[u1DbdcIdx],
		  prSrSd->u4OBSSAirtime[u1DbdcIdx],
		  prSrSd->u4DeltaTime[u1DbdcIdx],
		  prSrSd->u4MyTxAirtime[u1DbdcIdx],
		  prSrSd->u4MyRxAirtime[u1DbdcIdx],
		  prSrSd->u4OBSSAirTimeRatio[u1DbdcIdx] / 10,
		  prSrSd->u4OBSSAirTimeRatio[u1DbdcIdx] % 10);

	MTWF_PRINT("Apcli Detect\n"
		"	Apcli Detect flag = %d\n"
		"-------------------------------------------------------------------\n",
		prSrSd->fgSrMeshSDFlag);

	MTWF_PRINT("OBSS Monitor\n"
		  " 	 Tx         State      = %s\n"
		  " 	 Tx   Byte  Sum        = %d\n"
		  " 	 TxD  PG    Cnt        = %d\n"
		  " 	 Low  Tput  Cnt        = %d\n"
		  " 	 Contention Point      = %d\n"
		  " 	 Long Pkt   Point      = %d\n"
		  " 	 Pkt  Len   Short      = %d\n"
		  " 	 Pkt  Len   Middle     = %d\n"
		  " 	 Pkt  Len   Long       = %d\n"
		  " 	 SR   Tx    Cnt        = %d\n"
		  " 	 Mode       MCS        = %d\n"
		  "-------------------------------------------------------------------\n"
		  "OBSS Rate Distribution\n",
		  srtxstate[prSrSd->u1SrTxState[u1DbdcIdx]],
		  prSrSd->u4TxByteSum[u1DbdcIdx],
		  prSrSd->u4TxdPgCnt[u1DbdcIdx],
		  prSrSd->u1LowTrafficCnt[u1DbdcIdx],
		  prSrSd->u1ContWeakChkPnt[u1DbdcIdx],
		  prSrSd->u1ObssLongPktPnt[u1DbdcIdx],
		  prSrSd->u2ObssLongPkt[u1DbdcIdx][0],
		  prSrSd->u2ObssLongPkt[u1DbdcIdx][1],
		  prSrSd->u2ObssLongPkt[u1DbdcIdx][2],
		  prSrSd->u4SrTxCnt[u1DbdcIdx],
		  prSrSd->u1ModeMcsIdx[u1DbdcIdx]);

	for (u1McsIdx = 0; u1McsIdx < SR_RCPITBL_MCS_NUM; u1McsIdx++)
		MTWF_PRINT("MCS[%2d] = %x\n", u1McsIdx, prSrSd->u2RxrptMcs[u1McsIdx][u1DbdcIdx]);

}

VOID PrintSrCmdSrCap(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrCap)
{
	if (IS_SR_V1(pAd)) {
		P_SR_CMD_SR_CAP_T_SR_V1 prSrCmdSrCap = (P_SR_CMD_SR_CAP_T_SR_V1)_prSrCmdSrCap;

		PrintSrCmd(&(prSrCmdSrCap->rSrCmd));
		PrintSrCap(pAd, &(prSrCmdSrCap->rSrCap));
	}

	if (IS_SR_V2(pAd)) {
		P_SR_CMD_SR_CAP_T_SR_V2 prSrCmdSrCap = (P_SR_CMD_SR_CAP_T_SR_V2)_prSrCmdSrCap;

		PrintSrCmd(&(prSrCmdSrCap->rSrCmd));
		PrintSrCap(pAd, &(prSrCmdSrCap->rSrCap));
	}

}

VOID PrintSrCmdSrPara(IN P_SR_CMD_SR_PARA_T prSrCmdSrPara)
{
	PrintSrCmd(&(prSrCmdSrPara->rSrCmd));
	PrintSrPara(&(prSrCmdSrPara->rSrPara));
}

VOID PrintSrCmdSrGloVarSingleDropTa(IN P_SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T
				    prSrCmdSrGlobalVarSingleDropTa, IN UINT_8 u1DropTaIdx,
				    IN UINT_8 u1StaIdx)
{
	PrintSrCmd(&(prSrCmdSrGlobalVarSingleDropTa->rSrCmd));
	PrintSrGloVarSingleDropTa(&(prSrCmdSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa),
				  u1DropTaIdx, u1StaIdx);
}

VOID PrintSrCmdSrCond(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrCond)
{
	if (IS_SR_V1(pAd)) {
		P_SR_CMD_SR_COND_T_SR_V1 prSrCmdSrCond = (P_SR_CMD_SR_COND_T_SR_V1)_prSrCmdSrCond;

		PrintSrCmd(&(prSrCmdSrCond->rSrCmd));
		PrintSrCond(pAd, &(prSrCmdSrCond->rSrCond));
	}

	if (IS_SR_V2(pAd)) {
		P_SR_CMD_SR_COND_T_SR_V2 prSrCmdSrCond = (P_SR_CMD_SR_COND_T_SR_V2)_prSrCmdSrCond;

		PrintSrCmd(&(prSrCmdSrCond->rSrCmd));
		PrintSrCond(pAd, &(prSrCmdSrCond->rSrCond));
	}
}

VOID PrintSrCmdSrRcpiTbl(IN P_SR_CMD_SR_RCPITBL_T prSrCmdSrRcpiTbl)
{
	PrintSrCmd(&(prSrCmdSrRcpiTbl->rSrCmd));
	PrintSrRcpiTbl(&(prSrCmdSrRcpiTbl->rSrRcpiTbl));
}

VOID PrintSrCmdSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrRcpiTblOfst)
{
	if (IS_SR_V1(pAd)) {
		P_SR_CMD_SR_RCPITBL_OFST_T_SR_V1 prSrCmdSrRcpiTblOfst = (P_SR_CMD_SR_RCPITBL_OFST_T_SR_V1)_prSrCmdSrRcpiTblOfst;

		PrintSrCmd(&(prSrCmdSrRcpiTblOfst->rSrCmd));
		PrintSrRcpiTblOfst(pAd, &(prSrCmdSrRcpiTblOfst->rSrRcpiTblOfst));
	}

	if (IS_SR_V2(pAd)) {
		P_SR_CMD_SR_RCPITBL_OFST_T_SR_V2 prSrCmdSrRcpiTblOfst = (P_SR_CMD_SR_RCPITBL_OFST_T_SR_V2)_prSrCmdSrRcpiTblOfst;

		PrintSrCmd(&(prSrCmdSrRcpiTblOfst->rSrCmd));
		PrintSrRcpiTblOfst(pAd, &(prSrCmdSrRcpiTblOfst->rSrRcpiTblOfst));
	}
}

VOID PrintSrCmdSrQCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrQCtrl)
{
	if (IS_SR_V1(pAd)) {
		P_SR_CMD_SR_Q_CTRL_T_SR_V1 prSrCmdSrQCtrl = (P_SR_CMD_SR_Q_CTRL_T_SR_V1)_prSrCmdSrQCtrl;

		PrintSrCmd(&(prSrCmdSrQCtrl->rSrCmd));
		PrintSrQCtrl(pAd, &(prSrCmdSrQCtrl->rSrQCtrl));
	}

	if (IS_SR_V2(pAd)) {
		P_SR_CMD_SR_Q_CTRL_T_SR_V2 prSrCmdSrQCtrl = (P_SR_CMD_SR_Q_CTRL_T_SR_V2)_prSrCmdSrQCtrl;

		PrintSrCmd(&(prSrCmdSrQCtrl->rSrCmd));
		PrintSrQCtrl(pAd, &(prSrCmdSrQCtrl->rSrQCtrl));
	}
}

VOID PrintSrCmdSrIBPD(IN P_SR_CMD_SR_IBPD_T prSrCmdSrIBPD)
{
	PrintSrCmd(&(prSrCmdSrIBPD->rSrCmd));
	PrintSrIBPD(&(prSrCmdSrIBPD->rSrIBPD));
}

VOID PrintSrCmdSrNRT(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrNRT)
{
	if (IS_SR_V1(pAd)) {
		P_SR_CMD_SR_NRT_T_SR_V1 prSrCmdSrNRT = (P_SR_CMD_SR_NRT_T_SR_V1)_prSrCmdSrNRT;

		PrintSrCmd(&(prSrCmdSrNRT->rSrCmd));
		PrintSrNRT(pAd, &(prSrCmdSrNRT->rSrNRT));
	}

	if (IS_SR_V2(pAd)) {
		P_SR_CMD_SR_NRT_T_SR_V2 prSrCmdSrNRT = (P_SR_CMD_SR_NRT_T_SR_V2)_prSrCmdSrNRT;

		PrintSrCmd(&(prSrCmdSrNRT->rSrCmd));
		PrintSrNRT(pAd, &(prSrCmdSrNRT->rSrNRT));
	}
}

VOID PrintSrCmdSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrCmdSrNRTCtrl)
{
	if (IS_SR_V1(pAd)) {
		P_SR_CMD_SR_NRT_CTRL_T_SR_V1 prSrCmdSrNRTCtrl = (P_SR_CMD_SR_NRT_CTRL_T_SR_V1)_prSrCmdSrNRTCtrl;

		PrintSrCmd(&(prSrCmdSrNRTCtrl->rSrCmd));
		PrintSrNRTCtrl(pAd, &(prSrCmdSrNRTCtrl->rSrNRTCtrl));
	}

	if (IS_SR_V2(pAd)) {
		P_SR_CMD_SR_NRT_CTRL_T_SR_V2 prSrCmdSrNRTCtrl = (P_SR_CMD_SR_NRT_CTRL_T_SR_V2)_prSrCmdSrNRTCtrl;

		PrintSrCmd(&(prSrCmdSrNRTCtrl->rSrCmd));
		PrintSrNRTCtrl(pAd, &(prSrCmdSrNRTCtrl->rSrNRTCtrl));
	}
}

VOID PrintSrCmdSrFNQCtrl(IN P_SR_CMD_SR_FNQ_CTRL_T prSrCmdSrFNQCtrl)
{
	PrintSrCmd(&(prSrCmdSrFNQCtrl->rSrCmd));
	PrintSrFNQCtrl(&(prSrCmdSrFNQCtrl->rSrFNQCtrl));
}

VOID PrintSrCmdSrFrmFilt(IN P_SR_CMD_SR_FRM_FILT_T prSrCmdSrFrmFilt)
{
	PrintSrCmd(&(prSrCmdSrFrmFilt->rSrCmd));
	PrintSrFrmFilt(&(prSrCmdSrFrmFilt->u4SrFrmFilt));
}

VOID PrintSrCmdSrInterPsCtrl(IN P_SR_CMD_SR_INTERPS_CTRL_T prSrCmdSrInterPsCtrl)
{
	PrintSrCmd(&(prSrCmdSrInterPsCtrl->rSrCmd));
	PrintSrInterPsCtrl(&(prSrCmdSrInterPsCtrl->rSrInterPsCtrl));
}

VOID PrintSrCmdSrSrgBitmap(IN P_SR_CMD_SR_SRG_BITMAP_T prSrCmdSrSrgBitmap)
{
	PrintSrCmd(&(prSrCmdSrSrgBitmap->rSrCmd));
	PrintSrSrgBitmap(prSrCmdSrSrgBitmap->rSrCmd.u1DbdcIdx, &(prSrCmdSrSrgBitmap->rSrSrgBitmap));
}

VOID PrintSrCmdSrSiga(IN PRTMP_ADAPTER pAD, IN P_SR_CMD_SR_SIGA_FLAG_T prSrCmdSrSigaFlag)
{
	PrintSrCmd(&(prSrCmdSrSigaFlag->rSrCmd));
	PrintSrSiga(pAD, prSrCmdSrSigaFlag->rSrCmd.u1DbdcIdx, &(prSrCmdSrSigaFlag->rSrSigaFlag), FALSE);
}

VOID PrintSrCmdSrSigaAuto(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SR_SIGA_AUTO_FLAG_T *prSrCmdSrSigaAutoFlag)
{
	PrintSrCmd(&(prSrCmdSrSigaAutoFlag->rSrCmd));
	MTWF_PRINT("SrSigaAutoFlag = %d\n",
		prSrCmdSrSigaAutoFlag->rSrSigaAutoFlag.u1SrSigaAutoFlag);
}

VOID PrintSrCmdMeshTopo(IN struct _SR_CMD_MESH_TOPOLOGY_T *prSrCmdMeshTopo)
{
	PrintSrCmd(&(prSrCmdMeshTopo->rSrCmd));
	PrintSrMeshTopo(prSrCmdMeshTopo->rSrCmd.u1CmdSubId, &(prSrCmdMeshTopo->rSrCmdMeshTopo));
}

VOID PrintSrCmdSrUlStatus(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SR_UL_TRAFFIC_STATUS_T *prSrCmdSrUlStatus)
{
	MTWF_PRINT("u1UlStatus = %d DbdcIdx:%u CmdSubId:%u\n",
		prSrCmdSrUlStatus->rSrUlStatus.u1UlStatus, prSrCmdSrUlStatus->rSrCmd.u1DbdcIdx, prSrCmdSrUlStatus->rSrCmd.u1CmdSubId);
}

VOID PrintSrCmdSrMapBalance(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SET_MAP_BALANCE_T *prSrCmdMapBalance)
{
	PrintSrCmd(&(prSrCmdMapBalance->rSrCmd));
	MTWF_PRINT("MapBalance = %u\n",
		prSrCmdMapBalance->rSrMapBalance.u1MapBalance);
}

VOID PrintSrCmdSrMeshUlMode(IN PRTMP_ADAPTER pAd, IN struct _SR_CMD_SET_MESH_UL_MODE_T *prSrCmdMeshUlMode)
{
	PrintSrCmd(&(prSrCmdMeshUlMode->rSrCmd));
	MTWF_PRINT("u1UlMode = %u\n",
		prSrCmdMeshUlMode->rSrMeshUlMode.u1UlMode);
}

VOID PrintSrEvent(IN P_SR_EVENT_T prSrEvent)
{
	CHAR *status[5] = {"SUCCESS", "SANITY_FAIL", "CALL_MIDDLE_FAIL", "SW_HW_VAL_NOT_SYNC", "UNKNOWN"};

	MTWF_PRINT("u1EventSubId = %x, u1ArgNum = %d, u1DbdcIdx = %d, u1Status = %s\n"
			"u1DropTaIdx = %d, u1StaIdx = %d, u4Value = %d\n",
			prSrEvent->u1EventSubId, prSrEvent->u1ArgNum, prSrEvent->u1DbdcIdx, status[prSrEvent->u1Status],
			prSrEvent->u1DropTaIdx, prSrEvent->u1StaIdx, prSrEvent->u4Value);
}

VOID PrintSrEventSrCap(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrCap)
{
	if (IS_SR_V1(pAd)) {
		P_SR_EVENT_SR_CAP_T_SR_V1 prSrEventSrCap = (P_SR_EVENT_SR_CAP_T_SR_V1)_prSrEventSrCap;

		PrintSrEvent(&(prSrEventSrCap->rSrEvent));
		PrintSrCap(pAd, &(prSrEventSrCap->rSrCap));
	}

	if (IS_SR_V2(pAd)) {
		P_SR_EVENT_SR_CAP_T_SR_V2 prSrEventSrCap = (P_SR_EVENT_SR_CAP_T_SR_V2)_prSrEventSrCap;

		PrintSrEvent(&(prSrEventSrCap->rSrEvent));
		PrintSrCap(pAd, &(prSrEventSrCap->rSrCap));
	}
}

VOID PrintSrEventSrPara(IN P_SR_EVENT_SR_PARA_T prSrEventSrPara)
{
	PrintSrEvent(&(prSrEventSrPara->rSrEvent));
	PrintSrPara(&(prSrEventSrPara->rSrPara));
}

VOID PrintSrEventSrInd(IN P_SR_EVENT_SR_IND_T prSrEventSrInd)
{
	PrintSrEvent(&(prSrEventSrInd->rSrEvent));
	PrintSrInd(&(prSrEventSrInd->rSrInd));
}


VOID PrintSrEventSrGloVarSingleDropTa(IN P_SR_EVENT_SR_GLOBAL_VAR_SINGLE_DROP_TA_T
				      prSrEventSrGlobalVarSingleDropTa)
{
	UINT_8 u1DropTaIdx, u1StaIdx;

	u1DropTaIdx = prSrEventSrGlobalVarSingleDropTa->rSrEvent.u1DropTaIdx;
	u1StaIdx = prSrEventSrGlobalVarSingleDropTa->rSrEvent.u1StaIdx;

	PrintSrEvent(&(prSrEventSrGlobalVarSingleDropTa->rSrEvent));
	PrintSrGloVarSingleDropTa(&(prSrEventSrGlobalVarSingleDropTa->rSrGlobalVarSingleDropTa),
				  u1DropTaIdx, u1StaIdx);
}

VOID PrintSrEventSrCond(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrCond)
{
	if (IS_SR_V1(pAd)) {
		P_SR_EVENT_SR_COND_T_SR_V1 prSrEventSrCond = (P_SR_EVENT_SR_COND_T_SR_V1)_prSrEventSrCond;

		PrintSrEvent(&(prSrEventSrCond->rSrEvent));
		PrintSrCond(pAd, &(prSrEventSrCond->rSrCond));
	}

	if (IS_SR_V2(pAd)) {
		P_SR_EVENT_SR_COND_T_SR_V2 prSrEventSrCond = (P_SR_EVENT_SR_COND_T_SR_V2)_prSrEventSrCond;

		PrintSrEvent(&(prSrEventSrCond->rSrEvent));
		PrintSrCond(pAd, &(prSrEventSrCond->rSrCond));
	}
}

VOID PrintSrEventSrRcpiTbl(IN P_SR_EVENT_SR_RCPITBL_T prSrEventSrRcpiTbl)
{
	PrintSrEvent(&(prSrEventSrRcpiTbl->rSrEvent));
	PrintSrRcpiTbl(&(prSrEventSrRcpiTbl->rSrRcpiTbl));
}

VOID PrintSrEventSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrRcpiTblOfst)
{
	if (IS_SR_V1(pAd)) {
		P_SR_EVENT_SR_RCPITBL_OFST_T_SR_V1 prSrEventSrRcpiTblOfst = (P_SR_EVENT_SR_RCPITBL_OFST_T_SR_V1)_prSrEventSrRcpiTblOfst;

		PrintSrEvent(&(prSrEventSrRcpiTblOfst->rSrEvent));
		PrintSrRcpiTblOfst(pAd, &(prSrEventSrRcpiTblOfst->rSrRcpiTblOfst));
	}

	if (IS_SR_V2(pAd)) {
		P_SR_EVENT_SR_RCPITBL_OFST_T_SR_V2 prSrEventSrRcpiTblOfst = (P_SR_EVENT_SR_RCPITBL_OFST_T_SR_V2)_prSrEventSrRcpiTblOfst;

		PrintSrEvent(&(prSrEventSrRcpiTblOfst->rSrEvent));
		PrintSrRcpiTblOfst(pAd, &(prSrEventSrRcpiTblOfst->rSrRcpiTblOfst));
	}
}
VOID PrintSrEventSrQCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrQCtrl)
{
	if (IS_SR_V1(pAd)) {
		P_SR_EVENT_SR_Q_CTRL_T_SR_V1 prSrEventSrQCtrl = (P_SR_EVENT_SR_Q_CTRL_T_SR_V1)_prSrEventSrQCtrl;

		PrintSrEvent(&(prSrEventSrQCtrl->rSrEvent));
		PrintSrQCtrl(pAd, &(prSrEventSrQCtrl->rSrQCtrl));
	}

	if (IS_SR_V2(pAd)) {
		P_SR_EVENT_SR_Q_CTRL_T_SR_V2 prSrEventSrQCtrl = (P_SR_EVENT_SR_Q_CTRL_T_SR_V2)_prSrEventSrQCtrl;

		PrintSrEvent(&(prSrEventSrQCtrl->rSrEvent));
		PrintSrQCtrl(pAd, &(prSrEventSrQCtrl->rSrQCtrl));
	}
}

VOID PrintSrEventSrIBPD(IN P_SR_EVENT_SR_IBPD_T prSrEventSrIBPD)
{
	PrintSrEvent(&(prSrEventSrIBPD->rSrEvent));
	PrintSrIBPD(&(prSrEventSrIBPD->rSrIBPD));
}

VOID PrintSrEventSrNRT(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrNRT)
{
	UINT_8 u1Index;

	if (IS_SR_V1(pAd)) {
		P_SR_EVENT_SR_NRT_T_SR_V1 prSrEventSrNRT = (P_SR_EVENT_SR_NRT_T_SR_V1)_prSrEventSrNRT;

		PrintSrEvent(&(prSrEventSrNRT->rSrEvent));
		for (u1Index = 0; u1Index < SR_NRT_ROW_NUM; u1Index++)
			PrintSrNRT(pAd, &(prSrEventSrNRT->rSrNRT[u1Index]));
	}

	if (IS_SR_V2(pAd)) {
		P_SR_EVENT_SR_NRT_T_SR_V2 prSrEventSrNRT = (P_SR_EVENT_SR_NRT_T_SR_V2)_prSrEventSrNRT;

		PrintSrEvent(&(prSrEventSrNRT->rSrEvent));
		for (u1Index = 0; u1Index < SR_NRT_ROW_NUM; u1Index++)
			PrintSrNRT(pAd, &(prSrEventSrNRT->rSrNRT[u1Index]));
	}
}

VOID PrintSrEventSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN VOID *_prSrEventSrNRTCtrl)
{
	if (IS_SR_V1(pAd)) {
		P_SR_EVENT_SR_NRT_CTRL_T_SR_V1 prSrEventSrNRTCtrl = (P_SR_EVENT_SR_NRT_CTRL_T_SR_V1)_prSrEventSrNRTCtrl;

		PrintSrEvent(&(prSrEventSrNRTCtrl->rSrEvent));
		PrintSrNRTCtrl(pAd, &(prSrEventSrNRTCtrl->rSrNRTCtrl));
	}

	if (IS_SR_V2(pAd)) {
		P_SR_EVENT_SR_NRT_CTRL_T_SR_V2 prSrEventSrNRTCtrl = (P_SR_EVENT_SR_NRT_CTRL_T_SR_V2)_prSrEventSrNRTCtrl;

		PrintSrEvent(&(prSrEventSrNRTCtrl->rSrEvent));
		PrintSrNRTCtrl(pAd, &(prSrEventSrNRTCtrl->rSrNRTCtrl));
	}
}

VOID PrintSrEventSrFNQCtrl(IN P_SR_EVENT_SR_FNQ_CTRL_T prSrEventSrFNQCtrl)
{
	PrintSrEvent(&(prSrEventSrFNQCtrl->rSrEvent));
	PrintSrFNQCtrl(&(prSrEventSrFNQCtrl->rSrFNQCtrl));
}

VOID PrintSrEventSrFrmFilt(IN P_SR_EVENT_SR_FRM_FILT_T prSrEventSrFrmFilt)
{
	PrintSrEvent(&(prSrEventSrFrmFilt->rSrEvent));
	PrintSrFrmFilt(&(prSrEventSrFrmFilt->u4SrFrmFilt));
}

VOID PrintSrEventSrInterPsCtrl(IN P_SR_EVENT_SR_INTERPS_CTRL_T prSrEventSrInterPsCtrl)
{
	PrintSrEvent(&(prSrEventSrInterPsCtrl->rSrEvent));
	PrintSrInterPsCtrl(&(prSrEventSrInterPsCtrl->rSrInterPsCtrl));
}

VOID PrintSrEventSrInterPsDbg(IN P_SR_EVENT_SR_INTERPS_DBG_T prSrEventSrInterPsDbg)
{
	PrintSrEvent(&(prSrEventSrInterPsDbg->rSrEvent));
	PrintSrInterPsDbg(&(prSrEventSrInterPsDbg->rSrInterPsDbg));
}

VOID PrintSrEventSrCnt(IN P_SR_EVENT_SR_CNT_T prSrEventSrCnt)
{
	PrintSrEvent(&(prSrEventSrCnt->rSrEvent));
	PrintSrCnt(prSrEventSrCnt->rSrEvent.u1DbdcIdx, &(prSrEventSrCnt->rSrCnt), prSrEventSrCnt->rSrEvent.u4Value);
}

VOID PrintSrEventSrSd(IN P_SR_EVENT_SR_SD_T prSrEventSrSd)
{
	PrintSrEvent(&(prSrEventSrSd->rSrEvent));
	PrintSrSd(prSrEventSrSd->rSrEvent.u1DbdcIdx, &(prSrEventSrSd->rSrSd));
}

VOID PrintSrEventSrSrgBitmap(IN P_SR_EVENT_SR_SRG_BITMAP_T prSrEventSrSrgBitmap)
{
	PrintSrEvent(&(prSrEventSrSrgBitmap->rSrEvent));
	PrintSrSrgBitmap(prSrEventSrSrgBitmap->rSrEvent.u1DbdcIdx, &(prSrEventSrSrgBitmap->rSrSrgBitmap));
}

VOID PrintSrEventSrDlMeshRssi(IN P_SR_EVENT_T prSrEvent)
{
	PrintSrEvent(prSrEvent);
	PrintSrDlMeshRssi(prSrEvent->u1EventSubId, prSrEvent->u1DbdcIdx, (INT_8)prSrEvent->u4Value);
}

VOID PrintSrEventBhForbidBitMap(IN P_SR_EVENT_T prSrEvent)
{
	PrintSrEvent(prSrEvent);
}

VOID PrintSrEventSrSiga(IN PRTMP_ADAPTER pAD, IN P_SR_EVENT_SR_SIGA_T prSrEventSrSigaFlag)
{
	PrintSrEvent(&(prSrEventSrSigaFlag->rSrEvent));
	PrintSrSiga(pAD, prSrEventSrSigaFlag->rSrEvent.u1DbdcIdx, &(prSrEventSrSigaFlag->rSrSigaFlag), TRUE);
}

VOID PrintSrEventSrSigaAuto(IN PRTMP_ADAPTER pAd, IN struct _SR_EVENT_SR_SIGA_AUTO_T *prSrEventSrSigaAutoFlag)
{
	PrintSrEvent(&(prSrEventSrSigaAutoFlag->rSrEvent));
	MTWF_PRINT("SrSigaAuto = %d\n",
		prSrEventSrSigaAutoFlag->rSrEvent.u4Value);
}

VOID PrintSrEventMeshTopo(IN PRTMP_ADAPTER pAd, IN struct _SR_EVENT_MESH_TOPOLOGY_T *prSrEventMeshTopology)
{
	PrintSrEvent(&(prSrEventMeshTopology->rSrEvent));
	PrintSrMeshTopo(prSrEventMeshTopology->rSrEvent.u1EventSubId, &(prSrEventMeshTopology->rSrCmdMeshTopo));
}

VOID PrintSrEventMeshFHRssi(IN PRTMP_ADAPTER pAd, IN struct _SR_EVENT_MESH_FH_RSSI_TH_STATUS_T *prSrEventMeshFHRssiTh)
{
	PrintSrEvent(&(prSrEventMeshFHRssiTh->rSrEvent));
	PrintSrMeshFHRssiTh(prSrEventMeshFHRssiTh->rSrEvent.u1EventSubId, &(prSrEventMeshFHRssiTh->rSrMeshFhRssiTh));
}

VOID PrintSrEventMeshStaRssi(IN PRTMP_ADAPTER pAd, IN struct SR_EVENT_SR_MESH_DL_STA_THRESHOLD_T *prSrEventMeshStaRssiTh)
{
	PrintSrEvent(&(prSrEventMeshStaRssiTh->rSrEvent));
	PrintSrMeshStaRssiTh(prSrEventMeshStaRssiTh->rSrEvent.u1EventSubId, &(prSrEventMeshStaRssiTh->rSrMeshSrDLStaThreshold));
}

VOID PrintSrEventSrUlStatus(IN PRTMP_ADAPTER pAd, IN struct _SR_EVENT_SR_UL_TRAFFIC_STATUS_T *prSrEventSrUlStatus)
{
	PrintSrEvent(&(prSrEventSrUlStatus->rSrEvent));
	MTWF_PRINT("u1UlStatus = %d\n",
		prSrEventSrUlStatus->rSrUlStatus.u1UlStatus);
}

VOID PrintSrEventSrMeshPhase(IN PRTMP_ADAPTER pAd, IN struct _SR_EVENT_SR_MESH_PHASE_T *prSrEventSrMeshPhase)
{
	PrintSrEvent(&(prSrEventSrMeshPhase->rSrEvent));
	MTWF_PRINT("SrMeshPhase = %d\n",
		prSrEventSrMeshPhase->rSrEvent.u4Value);
}

VOID PrintSrEventSrRemoteAPStaAllHe(IN PRTMP_ADAPTER pAd, IN struct _SR_EVENT_SR_REMOTE_AP_STA_ALL_HE_T *prSrEventSrRemoteAPStaAllHe)
{
	PrintSrEvent(&(prSrEventSrRemoteAPStaAllHe->rSrEvent));
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"SrMeshRemoteAPStaAllHe = %u\n",
		prSrEventSrRemoteAPStaAllHe->rSrEvent.u4Value);
}

VOID SrMeshSelfSrgBMChangeEvent(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, IN BOOLEAN fgPBssidUpd)
{
	UCHAR band_idx;

	band_idx = HcGetBandByWdev(wdev);

	if (fgPBssidUpd)
		SrMeshSelfPBssidChangeEvent(pAd, wdev, band_idx);

	SrMeshSelfBssColorChangeEvent(pAd, wdev, band_idx);

	if (_u1SrSelfSrgBMMode[band_idx] == ENUM_SR_SELF_BM_MANUAL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"_u1SrSelfSrgBMMode:%u Manual for band:%u\n",
			_u1SrSelfSrgBMMode[band_idx], band_idx);
		return;
	}

	SrMeshSelfSrgInfoEvent(pAd, band_idx);
}

VOID SrMeshSelfBssColorChangeEvent(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR band_idx)
{
	struct _SR_MESH_SRG_BITMAP_T *prSrgBitmap;
	UINT_8 bitmap[8];

	prSrgBitmap = &rSrSelfSrgBM[band_idx];

	prSrgBitmap->u4Color_31_0 = 0;
	prSrgBitmap->u4Color_63_32 = 0;

	bss_color_for_all_wdev(wdev, bitmap);

	prSrgBitmap->u4Color_31_0 = (bitmap[3] << 24) | (bitmap[2] << 16) | (bitmap[1] << 8) | bitmap[0];
	prSrgBitmap->u4Color_63_32 = (bitmap[7] << 24) | (bitmap[6] << 16) | (bitmap[5] << 8) | bitmap[4];

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"Color:[63_32][%x]-[31_0][%x]\n",
		prSrgBitmap->u4Color_63_32, prSrgBitmap->u4Color_31_0);
}

VOID SrMeshSelfPBssidChangeEvent(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR band_idx)
{
	struct _SR_MESH_SRG_BITMAP_T *prSrgBitmap;
	UCHAR index, par_bssid;
	struct wifi_dev *temp_wdev = NULL;

	prSrgBitmap = &rSrSelfSrgBM[band_idx];

	prSrgBitmap->u4pBssid_31_0 = 0;
	prSrgBitmap->u4pBssid_63_32 = 0;

	for (index = MAIN_MBSSID; index < pAd->ApCfg.BssidNum; index++) {
		temp_wdev = &pAd->ApCfg.MBSSID[index].wdev;

		if (band_idx != HcGetBandByWdev(temp_wdev))
			continue;

		if (WDEV_BSS_STATE(temp_wdev) != BSS_READY)
			continue;

		par_bssid = (temp_wdev->bssid[4] >> 7) | ((temp_wdev->bssid[5] & 0x1F) << 1);

		if (par_bssid < 32)
			prSrgBitmap->u4pBssid_31_0 |= (1 << par_bssid);
		else
			prSrgBitmap->u4pBssid_63_32 |= (1 << (par_bssid - 32));
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"PBssid:[63_32][%x]-[31_0][%x]\n",
		prSrgBitmap->u4pBssid_63_32, prSrgBitmap->u4pBssid_31_0);
}

VOID SrMeshSelfSrgInfoEvent(struct _RTMP_ADAPTER *pAd, UINT8 u1BandIdx)
{
	struct _SR_MESH_SRG_BITMAP_T *prSrgBitmap;

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
	struct wifi_dev *wdev = wdev_search_by_band_omac_idx(pAd, u1BandIdx, HW_BSSID_0);

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"%s: wdev is NULL", __func__);
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"%s: _u1SrSelfSrgBMMode:%u for band:%u and MAP_ENABLE:%u MAP_R3_ENABLE:%u SRMode:%u\n",
		__func__, _u1SrSelfSrgBMMode[u1BandIdx], u1BandIdx, IS_MAP_ENABLE(pAd),
		IS_MAP_R3_ENABLE(pAd), pAd->CommonCfg.SRMode[u1BandIdx]);
#endif

	if (_u1SrSelfSrgBMMode[u1BandIdx] == ENUM_SR_SELF_BM_MANUAL)
		prSrgBitmap = &rSrSelfSrgBMMan[u1BandIdx];
	else
		prSrgBitmap = &rSrSelfSrgBM[u1BandIdx];

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Color:[63_32][%x]-[31_0][%x] Bssid:[63_32][%x]-[31_0][%x]\n",
		prSrgBitmap->u4Color_63_32, prSrgBitmap->u4Color_31_0,
		prSrgBitmap->u4pBssid_63_32, prSrgBitmap->u4pBssid_31_0);

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) && defined(WAPP_SUPPORT)
	if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd))
		wapp_send_sr_self_srg_bm_event(pAd, wdev, (PUINT8)prSrgBitmap);
#endif /* defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) && defined(WAPP_SUPPORT) */
}

VOID ExtEventMeshUplinkTraffic(IN PRTMP_ADAPTER pAd, IN struct _SR_EVENT_SR_UL_TRAFFIC_STATUS_T *prSrEventSrUlStatus)
{
	struct wifi_dev *wdev;

	wdev = wdev_search_by_band_omac_idx(pAd, prSrEventSrUlStatus->rSrEvent.u1DbdcIdx, HW_BSSID_0);

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"u1UlStatus = %u Band:%u\n",
		prSrEventSrUlStatus->rSrUlStatus.u1UlStatus, prSrEventSrUlStatus->rSrEvent.u1DbdcIdx);

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) && defined(WAPP_SUPPORT)
	wapp_send_uplink_traffic_event(pAd, wdev,
		prSrEventSrUlStatus->rSrUlStatus.u1UlStatus);
#endif /* defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) && defined(WAPP_SUPPORT) */
}

VOID SrMeshSrUpdateSTAMode(IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev_main, BOOL Assoc, UINT8 CurrStaIsHe)
{
	struct wifi_dev *wdev;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT16 wcid, stacnt = 0;
	UCHAR band_idx;

	band_idx = HcGetBandByWdev(wdev_main);

	if (_u1StaModeRptUnLock[band_idx] == 0)
		return;

	if (Assoc) {
		if (!CurrStaIsHe) {
			SrMeshSrReportSTAMode(pAd, band_idx, FALSE);
			return;
		}
	}

	for (wcid = 0; wcid < MAX_LEN_OF_MAC_TABLE; wcid++) {
		pEntry = &pAd->MacTab.Content[wcid];

		if (pEntry && !IS_ENTRY_NONE(pEntry) && !IS_ENTRY_MCAST(pEntry)) {
			wdev = pEntry->wdev;

			if (!wdev)
				continue;

			if (band_idx != HcGetBandByWdev(wdev))
				continue;

			stacnt++;

			if (!IS_HE_STA(pEntry->cap.modes)) {
				SrMeshSrReportSTAMode(pAd, band_idx, FALSE);
				return;
			}
		}
	}

	if (stacnt)
		SrMeshSrReportSTAMode(pAd, band_idx, TRUE);
	else
		SrMeshSrReportSTAMode(pAd, band_idx, FALSE);
}

VOID SrMeshSrReportSTAMode(IN PRTMP_ADAPTER pAd, UINT8 u1BandIdx, UINT8 u1StaAllHe)
{
	struct wifi_dev *wdev;

	wdev = wdev_search_by_band_omac_idx(pAd, u1BandIdx, HW_BSSID_0);

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"u1StaAllHe = %u Band:%u\n", u1StaAllHe, u1BandIdx);

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) && defined(WAPP_SUPPORT)
	if (IS_MAP_ENABLE(pAd) && (pAd->CommonCfg.SRMode[u1BandIdx] == TRUE))
		wapp_send_sta_mode_rpt_event(pAd, wdev, u1StaAllHe);
#endif /* defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) && defined(WAPP_SUPPORT) */
}

UINT_8 SRRcpiConv(IN INT_8 i1Dbm)
{
	/*
	   dBm = (RCPI-220)/2
	   RCPI = (dBm * 2) + 220
		= (dBm * 2) + (110 * 2)
		= (dBm + 110) * 2
				  *dBm must be negative
	*/

	if (i1Dbm < SR_RCPI_MIN) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " Err. i1Dbm:%d\n", i1Dbm);
		return (UINT_8)((SR_RCPI_MIN + 110) << 1);
	} else if (i1Dbm > SR_RCPI_MAX) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " Err. i1Dbm:%d\n", i1Dbm);
		return (UINT_8)((SR_RCPI_MAX + 110) << 1);
	} else {
		return (UINT_8)((i1Dbm + 110) << 1);
	}

}

INT_8 SRDbmConv(IN UINT_8 u1Rcpi)
{
	/*
	   dBm = (RCPI-220)/2
	*/
	return (u1Rcpi >> 1) - 110;

}


#endif				/* CFG_SUPPORT_FALCON_SR */
#endif				/* #if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981) */
