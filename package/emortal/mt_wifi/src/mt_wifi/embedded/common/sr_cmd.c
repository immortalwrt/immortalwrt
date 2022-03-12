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
#include "chip/mt7915_cr.h"
/*End - For HWITS00021718*/

#if defined(MT7915)
#ifdef CFG_SUPPORT_FALCON_SR

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
#define SR_CMD_SET_DEFAULT_ARG_NUM                          1
#define SR_CMD_SET_SR_CAP_SREN_CTRL_ARG_NUM                 1
#define SR_CMD_SET_SR_CAP_ALL_CTRL_ARG_NUM                  12
#define SR_CMD_SET_SR_PARA_ALL_CTRL_ARG_NUM                 9
#define SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL_ARG_NUM          2
#define SR_CMD_SET_SR_GLO_VAR_STA_CTRL_ARG_NUM              4
#define SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL_ARG_NUM         2
#define SR_CMD_SET_SR_COND_ALL_CTRL_ARG_NUM                 10
#define SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL_ARG_NUM             SR_RCPITBL_MCS_NUM
#define SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL_ARG_NUM        7
#define SR_CMD_SET_SR_Q_CTRL_ALL_CTRL_ARG_NUM               4
#define SR_CMD_SET_SR_IBPD_ALL_CTRL_ARG_NUM                 5
#define SR_CMD_SET_SR_NRT_ALL_CTRL_ARG_NUM                  2 /*TO-DO split NRTValue*/
#define SR_CMD_SET_SR_NRT_RESET_CTRL_ARG_NUM                1
#define SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL_ARG_NUM             6
#define SR_CMD_SET_SR_SRG_BITMAP_ARG_NUM                    4

#define SR_CMD_GET_DEFAULT_ARG_NUM                          1
#define SR_CMD_GET_SR_CAP_ALL_INFO_ARG_NUM                  1
#define SR_CMD_GET_SR_PARA_ALL_INFO_ARG_NUM                 1
#define SR_CMD_GET_SR_IND_ALL_INFO_ARG_NUM                  1
#define SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO_ARG_NUM   2
#define SR_CMD_GET_SR_COND_ALL_INFO_ARG_NUM                 1
#define SR_CMD_GET_SR_RCPI_TBL_ALL_INFO_ARG_NUM             1
#define SR_CMD_GET_SR_RCPI_TBL_OFST_ALL_INFO_ARG_NUM        1
#define SR_CMD_GET_SR_Q_CTRL_ALL_INFO_ARG_NUM               1
#define SR_CMD_GET_SR_IBPD_ALL_INFO_ARG_NUM                 1
#define SR_CMD_GET_SR_NRT_ALL_INFO_ARG_NUM                  1
#define SR_CMD_GET_SR_NRT_CTRL_ALL_INFO_ARG_NUM             1

/* Global Variable */
#define SR_STA_NUM           32
#define SR_DROP_TA_NUM       16
#define SR_RCPITBL_MCS_NUM   12
#define SR_NRT_ROW_NUM       16

#define SR_SCENE_DETECTION_TIMER_PERIOD_MS  500 /* ms */
#define SR_SCENE_DETECTION_OBSS_RSP_TIME_US 100 /* us */
#define SR_SCENE_DETECTION_OBSS_AIRTIME_THR 500 /* 50.0% */
#define SR_SCENE_DETECTION_CONGESTION_THR   800 /* 80.0% */



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

/** SR Capability */
typedef struct _WH_SR_CAP_T {
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
} WH_SR_CAP_T, *P_WH_SR_CAP_T;

/** SR Parameter */
typedef struct _WH_SR_PARA_T {
    /** RMAC */
    UINT_8 u1NonSrgPdThr;
    UINT_8 u1SrgPdThr;
    UINT_8 u1PeriodOfst;
    UINT_8 u1RcpiSourceSel;
    /** TMAC */
    UINT_16 u2ObssPdMin;
    UINT_16 u2ObssPdMinSrg;
    ENUM_WH_SR_RESP_TXPWR_MODE_T eRespTxPwrMode;
    ENUM_WH_SR_TXPWR_RESTRIC_MODE_T eTxPwrRestricMode;
    UINT_8 u1ObssTxPwrRef;
    /** Reserve for 4-byte aligned*/
    UINT_8 u1Reserved[3];
} WH_SR_PARA_T, *P_WH_SR_PARA_T;


/** SR Indicator */
typedef struct _WH_SR_IND_T {
    /** RMAC */
    UINT_8 u1NonSrgInterPpduRcpi;
    UINT_8 u1SrgInterPpduRcpi;
    UINT_16 u2NonSrgVldCnt;
    UINT_16 u2SrgVldCnt;
    UINT_16 u2IntraBssPpduCnt;
    UINT_16 u2InterBssPpduCnt;
    UINT_16 u2NonSrgPpduVldCnt;
    UINT_16 u2SrgPpduVldCnt;
    /** Reserve for 4-byte aligned*/
    UINT_8 u1Reserved[2];
    /** MIB */
    UINT_32 u4SrAmpduMpduCnt;
    UINT_32 u4SrAmpduMpduAckedCnt;
} WH_SR_IND_T, *P_WH_SR_IND_T;

/** SR Queue Ctrl */
typedef struct _WH_SR_QUEUE_CTRL_T {
    /** RMAC */
    BOOLEAN fgSrRxRptEn;
    /** ARB */
    BOOLEAN fgSrCw;
    BOOLEAN fgSrSuspend;
    /** Reserve for 4-byte aligned*/
    UINT_8  u1Reserved;
    /** ARB */
    UINT_32 u4SrBackOffMask;
} WH_SR_QUEUE_CTRL_T, *P_WH_SR_QUEUE_CTRL_T;
/** End FW & DRV sync with wh_sr.h **/


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


typedef struct _SR_GLOBAL_VAR_SINGLE_DROP_TA_T {
    UINT_8 u1Rsv;
    UINT_8 u1CurSrDropTaIdx;
    UINT_16 u2SrTtlTxCntThr;
    SR_DROP_TA_INFO_T rSrDropTaInfo;
} SR_GLOBAL_VAR_SINGLE_DROP_TA_T, *P_SR_GLOBAL_VAR_SINGLE_DROP_TA_T;

/** SRCOND */
typedef struct _SR_COND_T {
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
} SR_COND_T, *P_SR_COND_T;

/** SRRCPITBL */
typedef struct _SR_RCPITBL_T {
    UINT_8  u1RcpiTblMcs[SR_RCPITBL_MCS_NUM];
} SR_RCPITBL_T, *P_SR_RCPITBL_T;

/** SRRCPITBLOFST */
typedef struct _SR_RCPITBL_OFST_T {
    UINT_16 u2RxBwRcpiOfst;
    UINT_16 u2StbcRcpiOfst;
    UINT_16 u2NumAntRcpiOfst;
    UINT_16 u2LdpcRcpiOfst;
    UINT_16 u2DcmRcpiOfst;
    UINT_16 u2MacRcpiOfst;
    UINT_16 u2SigRcpiOfst;
    /** Reserve for 4-byte aligned*/
    UINT_8  u1Reserved[2];
} SR_RCPITBL_OFST_T, *P_SR_RCPITBL_OFST_T;

/** SRIBPD*/
typedef struct _SR_IBPD_T {
    /** RMAC */
    UINT_8 u1InterBssByHdrBssid;
    UINT_8 u1InterBssByMu;
    UINT_8 u1InterBssByPbssColor;
    UINT_8 u1InterBssByPaid;
    UINT_8 u1InterBssByBssColor;
    /** Reserve for 4-byte aligned*/
    UINT_8 u1Reserved[3];
} SR_IBPD_T, *P_SR_IBPD_T;

/** SRNRTCtrl*/
typedef struct _SR_NRT_CTRL_T {
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
} SR_NRT_CTRL_T, *P_SR_NRT_CTRL_T;

/* SR CNT */
typedef enum _ENUM_SR_RXRPT_SRC_T {
    ENUM_SR_RXRPT_SRC_RXRPT = 0,
    ENUM_SR_RXRPT_SRC_CMDRPT_TX,
    ENUM_SR_RXRPT_SRC_NUM
} ENUM_SR_RXRPT_SRC_T, *P_ENUM_SR_RXRPT_SRC_T;

typedef enum _ENUM_SR_ENTRY_T {
    ENUM_SR_ENTRY_NEWRXV = 0,
    ENUM_SR_ENTRY_ORIRXVVHT,
    ENUM_SR_ENTRY_ORIRXVHE,
    ENUM_SR_ENTRY_NEWMAC,
    ENUM_SR_ENTRY_ORIMAC,
    ENUM_SR_ENTRY_NUM
} ENUM_SR_ENTRY_T, *P_ENUM_SR_ENTRY_T;

typedef struct _SR_CNT_ENTRY_T {
    UINT_16 u2EntryTtl;
    UINT_16 u2PeriodSuc;/* u2PeriodFail = u2EntryTtl  - u2PeriodSuc */
    UINT_16 u2GenTxcSuc;/* u2GenTxcFail = u2PeriodSuc - u2GenTxcSuc */
    UINT_16 u2SrTxSuc;  /* u2SrTxFail   = u2GenTxcSuc - u2SrTxSuc   */
} SR_CNT_ENTRY_T, *P_SR_CNT_ENTRY_T;

typedef struct _SR_CNT_T {
    UINT_32 u4Rsv[RAM_BAND_NUM];
    UINT_16 u2EntryNoSrTtl[RAM_BAND_NUM];
    UINT_16 u2EntryFailTtl[RAM_BAND_NUM];
    SR_CNT_ENTRY_T rSrCntEntry[RAM_BAND_NUM][ENUM_SR_RXRPT_SRC_NUM][ENUM_SR_ENTRY_NUM];
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
    UINT_8  u1Rsv[3][RAM_BAND_NUM];
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
    /*Reseve for SR_CFG = 0x21*/
    /*Reseve for SR_CFG = 0x22*/
    SR_CMD_SET_SR_CFG_DPD_ENABLE = 0x23,
    SR_CMD_GET_SR_CFG_DPD_ENABLE = 0x24,
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
    SR_CMD_SET_SR_SRG_BITMAP = 0x80,
    SR_CMD_GET_SR_SRG_BITMAP = 0x81,
    SR_CMD_SET_SR_SRG_BITMAP_REFRESH = 0x82,
    SR_CMD_GET_SR_CNT_ALL = 0x83,
    SR_CMD_GET_SR_SD_ALL = 0x84,
    SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL = 0x85,
    SR_CMD_SET_SR_GLO_VAR_STA_CTRL = 0x86,
    SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL = 0x87,
    SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO = 0x88,
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
    /*Reseve for SR HW Module = 0xD5*/
    /*Reseve for SR HW Module = 0xD6*/
    /*Reseve for SR HW Module = 0xD7*/
    /*Reseve for SR HW Module = 0xD8*/
    /*Reseve for SR HW Module = 0xD9*/
    /*Reseve for SR HW Module = 0xDA*/
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
} ENUM_SR_CMD_SUBID, *P_ENUM_SR_CMD_SUBID;

typedef struct _SR_CMD_T {
    UINT_8  u1CmdSubId;
    UINT_8  u1ArgNum;
    UINT_8  u1DbdcIdx;
    UINT_8  u1Status;
    UINT_8  u1DropTaIdx;
    UINT_8  u1StaIdx;	/* #256STA */
    UINT_8  u1Rsv[2];
    UINT_32 u4Value;
} SR_CMD_T, *P_SR_CMD_T;

typedef struct _SR_CMD_SR_CAP_T {
    SR_CMD_T rSrCmd;
    WH_SR_CAP_T rSrCap;
} SR_CMD_SR_CAP_T, *P_SR_CMD_SR_CAP_T;

typedef struct _SR_CMD_SR_PARA_T {
    SR_CMD_T rSrCmd;
    WH_SR_PARA_T rSrPara;
} SR_CMD_SR_PARA_T, *P_SR_CMD_SR_PARA_T;

typedef struct _SR_CMD_SR_IND_T {
    SR_CMD_T rSrCmd;
    WH_SR_IND_T rSrInd;
} SR_CMD_SR_IND_T, *P_SR_CMD_SR_IND_T;

typedef struct _SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T {
    SR_CMD_T rSrCmd;
    SR_GLOBAL_VAR_SINGLE_DROP_TA_T rSrGlobalVarSingleDropTa;
} SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T, *P_SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T;

typedef struct _SR_CMD_SR_COND_T {
    SR_CMD_T rSrCmd;
    SR_COND_T rSrCond;
} SR_CMD_SR_COND_T, *P_SR_CMD_SR_COND_T;

typedef struct _SR_CMD_SR_RCPITBL_T {
    SR_CMD_T rSrCmd;
    SR_RCPITBL_T rSrRcpiTbl;
} SR_CMD_SR_RCPITBL_T, *P_SR_CMD_SR_RCPITBL_T;

typedef struct _SR_CMD_SR_RCPITBL_OFST_T {
    SR_CMD_T rSrCmd;
    SR_RCPITBL_OFST_T rSrRcpiTblOfst;
} SR_CMD_SR_RCPITBL_OFST_T, *P_SR_CMD_SR_RCPITBL_OFST_T;

typedef struct _SR_CMD_SR_Q_CTRL_T {
    SR_CMD_T rSrCmd;
    WH_SR_QUEUE_CTRL_T rSrQCtrl;
} SR_CMD_SR_Q_CTRL_T, *P_SR_CMD_SR_Q_CTRL_T;

typedef struct _SR_CMD_SR_IBPD_T {
    SR_CMD_T rSrCmd;
    SR_IBPD_T rSrIBPD;
} SR_CMD_SR_IBPD_T, *P_SR_CMD_SR_IBPD_T;

/** SRNRT*/
typedef struct _SR_NRT_T {
    /** RMAC */
    UINT_8 u1TableIdx;
    UINT_32 u4NRTValue;
    /** Reserve for 4-byte aligned*/
    UINT_8 u1Reserved[3];
} SR_NRT_T, *P_SR_NRT_T;

typedef struct _SR_CMD_SR_NRT_T {
    SR_CMD_T rSrCmd;
    SR_NRT_T rSrNRT;
} SR_CMD_SR_NRT_T, *P_SR_CMD_SR_NRT_T;

typedef struct _SR_CMD_SR_NRT_CTRL_T {
    SR_CMD_T rSrCmd;
    SR_NRT_CTRL_T rSrNRTCtrl;
} SR_CMD_SR_NRT_CTRL_T, *P_SR_CMD_SR_NRT_CTRL_T;

typedef struct _SR_CMD_SR_SRG_BITMAP_T {
    SR_CMD_T rSrCmd;
    SR_SRG_BITMAP_T rSrSrgBitmap;
} SR_CMD_SR_SRG_BITMAP_T, *P_SR_CMD_SR_SRG_BITMAP_T;
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
    /*Reseve for SR_CFG = 0x11*/
    SR_EVENT_GET_SR_CFG_DPD_ENABLE = 0x12,
    /*Reseve for SR_CFG = 0x13*/
    /*Reseve for SR_CFG = 0x14*/
    /*Reseve for SR_CFG = 0x15*/
    /*Reseve for SR_CFG = 0x16*/
    /*Reseve for SR_CFG = 0x17*/
    /*Reseve for SR_CFG = 0x18*/
    /*Reseve for SR_CFG = 0x19*/
    /*Reseve for SR_CFG = 0x1A*/
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
    /*Reseve for SR SW Module = 0x84*/
    /*Reseve for SR SW Module = 0x85*/
    /*Reseve for SR SW Module = 0x86*/
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
    /*Reseve for SR HW Module = 0xCA*/
    /*Reseve for SR HW Module = 0xCB*/
    /*Reseve for SR HW Module = 0xCC*/
    /*Reseve for SR HW Module = 0xCD*/
    /*Reseve for SR HW Module = 0xCE*/
    /*Reseve for SR HW Module = 0xCF*/
    /*Reseve for SR HW Module = 0xD0*/
    /*Reseve for SR HW Module = 0xD1*/
    /*Reseve for SR HW Module = 0xD2*/
    /*Reseve for SR HW Module = 0xD3*/
    /*Reseve for SR HW Module = 0xD4*/
    /*Reseve for SR HW Module = 0xD5*/
    /*Reseve for SR HW Module = 0xD6*/
    /*Reseve for SR HW Module = 0xD7*/
    /*Reseve for SR HW Module = 0xD8*/
    /*Reseve for SR HW Module = 0xD9*/
    /*Reseve for SR HW Module = 0xDA*/
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

typedef struct _SR_EVENT_SR_CAP_T {
    SR_EVENT_T rSrEvent;
    WH_SR_CAP_T rSrCap;
} SR_EVENT_SR_CAP_T, *P_SR_EVENT_SR_CAP_T;

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

typedef struct _SR_EVENT_SR_COND_T {
    SR_EVENT_T rSrEvent;
    SR_COND_T rSrCond;
} SR_EVENT_SR_COND_T, *P_SR_EVENT_SR_COND_T;

typedef struct _SR_EVENT_SR_RCPITBL_T {
    SR_EVENT_T rSrEvent;
    SR_RCPITBL_T rSrRcpiTbl;
} SR_EVENT_SR_RCPITBL_T, *P_SR_EVENT_SR_RCPITBL_T;

typedef struct _SR_EVENT_SR_RCPITBL_OFST_T {
    SR_EVENT_T rSrEvent;
    SR_RCPITBL_OFST_T rSrRcpiTblOfst;
} SR_EVENT_SR_RCPITBL_OFST_T, *P_SR_EVENT_SR_RCPITBL_OFST_T;

typedef struct _SR_EVENT_SR_Q_CTRL_T {
    SR_EVENT_T rSrEvent;
    WH_SR_QUEUE_CTRL_T rSrQCtrl;
} SR_EVENT_SR_Q_CTRL_T, *P_SR_EVENT_SR_Q_CTRL_T;

typedef struct _SR_EVENT_SR_IBPD_T {
    SR_EVENT_T rSrEvent;
    SR_IBPD_T rSrIBPD;
} SR_EVENT_SR_IBPD_T, *P_SR_EVENT_SR_IBPD_T;

typedef struct _SR_EVENT_SR_NRT_T {
    SR_EVENT_T rSrEvent;
    SR_NRT_T rSrNRT[SR_NRT_ROW_NUM];
} SR_EVENT_SR_NRT_T, *P_SR_EVENT_SR_NRT_T;

typedef struct _SR_EVENT_SR_NRT_CTRL_T {
    SR_EVENT_T rSrEvent;
    SR_NRT_CTRL_T rSrNRTCtrl;
} SR_EVENT_SR_NRT_CTRL_T, *P_SR_EVENT_SR_NRT_CTRL_T;

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
/** End SR Event */
/** End FW & DRV sync with sr_cmd.c **/

/*******************************************************************************
 *    PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/
/* For Command*/
NDIS_STATUS SrCmd(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_T prSrcmd);
NDIS_STATUS SrCmdSRUpdateCap(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_CAP_T prSrCmdSrCap);
NDIS_STATUS SrCmdSRUpdatePara(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_PARA_T prSrCmdSrPara);
NDIS_STATUS SrCmdSRUpdateGloVarSingleDropTa(IN PRTMP_ADAPTER pAd,
					    IN P_SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T
					    prSrCmdSrGlobalVarSingleDropTa, IN UINT_32 u4DropTaIdx,
					    IN UINT_32 u4StaIdx);
NDIS_STATUS SrCmdSRUpdateCond(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_COND_T prSrCmdSrCond);
NDIS_STATUS SrCmdSRUpdateRcpiTbl(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_RCPITBL_T prSrCmdSrRcpiTbl);
NDIS_STATUS SrCmdSRUpdateRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_RCPITBL_OFST_T prSrCmdSrRcpiTblOfst);
NDIS_STATUS SrCmdSRUpdateQCtrl(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_Q_CTRL_T prSrCmdSrQCtrl);
NDIS_STATUS SrCmdSRUpdateIBPD(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_IBPD_T prSrCmdSrIBPD);
NDIS_STATUS SrCmdSRUpdateNRT(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_NRT_T prSrCmdSrNRT);
NDIS_STATUS SrCmdSRUpdateNRTCtrl(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_NRT_CTRL_T prSrCmdSrNRTCtrl);
NDIS_STATUS SrCmdSRUpdateSrgBitmap(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_SRG_BITMAP_T prSrCmdSrSrgBitmap);
NDIS_STATUS SrCmdShow(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg, IN UINT_8 u1CmdSubId, IN UINT_8 u1ArgNum);
/* For Check value */
NDIS_STATUS IsFlag(IN INT32 u4ArgVal, IN UINT_8 u1ArgNum);
NDIS_STATUS IsInRange(IN INT32 u4ArgVal, IN UINT_8 u1ArgNum, IN INT32 u4Valfrom, IN INT32 u4Valto);
/* For Print content */
VOID PrintSrCmd(IN P_SR_CMD_T prSrCmd);
VOID PrintSrCap(IN P_WH_SR_CAP_T prSrCap);
VOID PrintSrPara(IN P_WH_SR_PARA_T prSrPara);
VOID PrintSrInd(IN P_WH_SR_IND_T prSrInd);
VOID PrintSrGloVarSingleDropTa(IN P_SR_GLOBAL_VAR_SINGLE_DROP_TA_T prSrGlobalVarSingleDropTa,
			       IN UINT_8 u1DropTaIdx, IN UINT_8 u1StaIdx);
VOID PrintSrDropTaInfo(IN P_SR_DROP_TA_INFO_T prSrDropTaInfo, IN UINT_8 u1DropTaIdx,
		       IN UINT_8 u1StaIdx);
VOID PrintSrStaInfo(IN P_SR_STA_INFO_T prSrStaInfo, IN UINT_8 u1StaIdx);
VOID PrintSrCond(IN P_SR_COND_T prSrCond);
VOID PrintSrRcpiTbl(IN P_SR_RCPITBL_T prSrRcpiTbl);
VOID PrintSrRcpiTblOfst(IN P_SR_RCPITBL_OFST_T prSrRcpiTblOfst);
VOID PrintSrQCtrl(IN P_WH_SR_QUEUE_CTRL_T prSrQCtrl);
VOID PrintSrIBPD(IN P_SR_IBPD_T prSrIBPD);
VOID PrintSrNRT(IN P_SR_NRT_T prSrNRT);
VOID PrintSrNRTCtrl(IN P_SR_NRT_CTRL_T prSrNRTCtrl);
VOID PrintSrSrgBitmap(IN UINT_8 u1DbdcIdx, IN P_SR_SRG_BITMAP_T prSrSrgBitmap);
VOID PrintSrCnt(IN UINT8 u1DbdcIdx, IN P_SR_CNT_T prSrCnt);
VOID PrintSrSd(IN UINT8 u1DbdcIdx, IN P_SR_SD_T prSrSd);

VOID PrintSrCmdSrCap(IN P_SR_CMD_SR_CAP_T prSrCmdSrCap);
VOID PrintSrCmdSrPara(IN P_SR_CMD_SR_PARA_T prSrCmdSrPara);
VOID PrintSrCmdSrGloVarSingleDropTa(IN P_SR_CMD_SR_GLOBAL_VAR_SINGLE_DROP_TA_T
				    prSrCmdSrGlobalVarSingleDropTa, IN UINT_8 u1DropTaIdx,
				    IN UINT_8 u1StaIdx);
VOID PrintSrCmdSrCond(IN P_SR_CMD_SR_COND_T prSrCmdSrCond);
VOID PrintSrCmdSrRcpiTbl(IN P_SR_CMD_SR_RCPITBL_T prSrCmdSrRcpiTbl);
VOID PrintSrCmdSrRcpiTblOfst(IN P_SR_CMD_SR_RCPITBL_OFST_T prSrCmdSrRcpiTblOfst);
VOID PrintSrCmdSrQCtrl(IN P_SR_CMD_SR_Q_CTRL_T prSrCmdSrQCtrl);
VOID PrintSrCmdSrIBPD(IN P_SR_CMD_SR_IBPD_T prSrCmdSrIBPD);
VOID PrintSrCmdSrNRT(IN P_SR_CMD_SR_NRT_T prSrCmdSrNRT);
VOID PrintSrCmdSrNRTCtrl(IN P_SR_CMD_SR_NRT_CTRL_T prSrCmdSrNRTCtrl);
VOID PrintSrCmdSrSrgBitmap(IN P_SR_CMD_SR_SRG_BITMAP_T prSrCmdSrSrgBitmap);
VOID PrintSrEvent(IN P_SR_EVENT_T prSrEvent);
VOID PrintSrEventSrCap(IN P_SR_EVENT_SR_CAP_T prSrEventSrCap);
VOID PrintSrEventSrPara(IN P_SR_EVENT_SR_PARA_T prSrEventSrPara);
VOID PrintSrEventSrInd(IN P_SR_EVENT_SR_IND_T prSrEventSrInd);
VOID PrintSrEventSrGloVarSingleDropTa(IN P_SR_EVENT_SR_GLOBAL_VAR_SINGLE_DROP_TA_T
				      prSrEventSrGlobalVarSingleDropT);
VOID PrintSrEventSrCond(IN P_SR_EVENT_SR_COND_T prSrEventSrCond);
VOID PrintSrEventSrRcpiTbl(IN P_SR_EVENT_SR_RCPITBL_T prSrEventSrRcpiTbl);
VOID PrintSrEventSrRcpiTblOfst(IN P_SR_EVENT_SR_RCPITBL_OFST_T prSrEventSrRcpiTblOfst);
VOID PrintSrEventSrQCtrl(IN P_SR_EVENT_SR_Q_CTRL_T prSrEventSrQCtrl);
VOID PrintSrEventSrIBPD(IN P_SR_EVENT_SR_IBPD_T prSrEventSrIBPD);
VOID PrintSrEventSrNRT(IN P_SR_EVENT_SR_NRT_T prSrEventSrNRT);
VOID PrintSrEventSrNRTCtrl(IN P_SR_EVENT_SR_NRT_CTRL_T prSrEventSrNRTCtrl);
VOID PrintSrEventSrCnt(IN P_SR_EVENT_SR_CNT_T prSrEventSrCnt);
VOID PrintSrEventSrSd(IN P_SR_EVENT_SR_SD_T prSrEventSrSd);
VOID PrintSrEventSrSrgBitmap(IN P_SR_EVENT_SR_SRG_BITMAP_T prSrEventSrSrgBitmap);

UINT_8 SRRcpiConv(IN INT_8 i1Dbm);
INT_8 SRDbmConv(IN UINT_8 u1Rcpi);

/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/
NDIS_STATUS SrRstNav(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s - Band:%d WlanIdx:%d Addr:%x Val:%x\n", __func__, u1DbdcIdx, u2WlanIdx, u4Addr, u4Val));
	}

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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: wdev == NULL \n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	/*SR not enable via profile*/
	if (pAd->CommonCfg.SREnable[u1DbdcIdx] == FALSE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s: SREnable[%d] = %d Return SUCCESS\n", __func__, u1DbdcIdx, pAd->CommonCfg.SREnable[u1DbdcIdx]));
		return NDIS_STATUS_SUCCESS;
	}

	/*Check DisSrBfrConnected avoid periodically send cmd*/
	if (pAd->CommonCfg.DisSrBfrConnected[u1DbdcIdx] == fgSrEnable) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s: DisSrBfrConnected[%d] = %d == fgSrEnable = %d Return SUCCESS\n", __func__, u1DbdcIdx, pAd->CommonCfg.DisSrBfrConnected[u1DbdcIdx], fgSrEnable));
		return NDIS_STATUS_SUCCESS;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
	("%s: u1DbdcIdx = %d, fgSrEnable = %d \n", __func__, u1DbdcIdx, fgSrEnable));

	if (fgSrEnable == FALSE) {

		/* Disable SRSDEnable First */
		os_zero_mem(&rSrCmd, sizeof(SR_CMD_T));
		/* Assign Cmd Id */
		rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CFG_SR_SD_ENABLE;
		rSrCmd.u1DbdcIdx = u1DbdcIdx;
		rSrCmd.u4Value = FALSE;
		Status = SrCmd(pAd, &rSrCmd);

		if (Status == NDIS_STATUS_FAILURE) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Set SRSDEnable[%d]=%d Fail!\n", __func__, u1DbdcIdx, FALSE));
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
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Set SREnable[%d]=%d Fail!\n", __func__, u1DbdcIdx, FALSE));
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
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Set SREnable[%d]=%d Fail!\n", __func__, u1DbdcIdx,
			  pAd->CommonCfg.SREnable[u1DbdcIdx]));
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
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Set SRSDEnable[%d]=%d Fail!\n", __func__, u1DbdcIdx,
			  pAd->CommonCfg.SRSDEnable[u1DbdcIdx]));
			return NDIS_STATUS_FAILURE;
		}
		/* End - Set SRSDEnable Part */
	}
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS SrProfileSREnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer)
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

NDIS_STATUS SrProfileSRMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer)
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


NDIS_STATUS SrProfileSRSDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer)
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

NDIS_STATUS SrProfileSRDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * buffer)
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
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 ("%s: Spatial Reuse initialize via profile.\n",
		  __func__));

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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s: Set SREnable[%d]=%d Fail!\n", __func__, u1DbdcIdx,
		  pAd->CommonCfg.SREnable[u1DbdcIdx]));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s: Set SRMode[%d]=%d Fail!\n", __func__, u1DbdcIdx,
		  pAd->CommonCfg.SRMode[u1DbdcIdx]));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s: Set SRSDEnable[%d]=%d Fail!\n", __func__, u1DbdcIdx,
		  pAd->CommonCfg.SRSDEnable[u1DbdcIdx]));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s: Set SRDPDEnable[%d]=%d Fail!\n", __func__, u1DbdcIdx,
		  pAd->CommonCfg.SRDPDEnable[u1DbdcIdx]));
	}
	/* End - Set SRDPDEnable Part */

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS SetSrCapSrEn(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	INT32 u4SrEn = 0;


	SR_CMD_SR_CAP_T rSrCmdSrCap;


	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&rSrCmdSrCap, sizeof(SR_CMD_SR_CAP_T));

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
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmdSrCap.rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, rSrCmdSrCap.rSrCmd.u1ArgNum));
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {
				rSrCmdSrCap.rSrCap.fgSrEn = u4SrEn;
				Status = SrCmdSRUpdateCap(pAd, &rSrCmdSrCap);
			}
		} while (0);
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		Status = NDIS_STATUS_FAILURE;

	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcapsren=[SrEn]\n", __func__));
	}

	return Status;
}

NDIS_STATUS SetSrCapAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
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
	UINT_32 u4NonSrgInterPpduPresv = 0, u4SrgInterPpduPresv = 0, u4SrRemTimeEn = 0;
	UINT_32 u4ProtInSrWinDis = 0, u4TxCmdDlRateSelEn = 0, u4AmpduTxCntEn = 0;


	SR_CMD_SR_CAP_T rSrCmdSrCap;


	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&rSrCmdSrCap, sizeof(SR_CMD_SR_CAP_T));

	/* Assign Cmd Id */
	rSrCmdSrCap.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CAP_ALL_CTRL;
	rSrCmdSrCap.rSrCmd.u1ArgNum = SR_CMD_SET_SR_CAP_ALL_CTRL_ARG_NUM;
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
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmdSrCap.rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, rSrCmdSrCap.rSrCmd.u1ArgNum));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcap=[SrEn]-[SrgEn]-[NonSrgEn]-[SingleMdpuRtsctsEn]-[HdrDurEn]-[TxopDurEn]-[NonSrgInterPpduPresv]-[SrgInterPpduPresv]-[SrRemTimeEn]-[ProtInSrWinDis]-[TxCmdDlRateSelEn]-[AmpduTxCntEn]\n",
			  __func__));
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
					u4PeriodOfst = simple_strtol(value, 0, 10);
					Status =
					    IsInRange(u4PeriodOfst, u1ArgNum,
						      SR_PARA_PERIOD_OFST_MIN,
						      SR_PARA_PERIOD_OFST_MAX);
					break;
				case 3:
					u4RcpiSourceSel = simple_strtol(value, 0, 10);
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
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmdSrPara.rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, rSrCmdSrPara.rSrCmd.u1ArgNum));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srpara=[NonSrgPdThr]-[SrgPdThr]-[PeriodOfst]-[RcpiSourceSel]-[ObssPdMin]-[ObssPdMinSrg]-[RespTxPwrMode]-[TxPwrRestricMode]-[ObssTxPwrRef]\n",
			  __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_GLO_VAR_DROP_TA_CTRL **/
NDIS_STATUS SetSrDropTa(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
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

	if(!prSrCmdSrGlobalVarSingleDropTa)
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
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srdropta=[u4DropTaIdx]-[u4Address2]\n", __func__));
	}

	if(prSrCmdSrGlobalVarSingleDropTa)
		os_free_mem(prSrCmdSrGlobalVarSingleDropTa);

	return Status;
}

/** SR_CMD_SET_SR_GLO_VAR_STA_CTRL **/
NDIS_STATUS SetSrSta(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
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
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srsta=[u4DropTaIdx]-[u4StaIdx]-[u4WlanId]-[u4SrRateOffset]\n",
			  __func__));
	}

	if(prSrCmdSrGlobalVarSingleDropTa)
		os_free_mem(prSrCmdSrGlobalVarSingleDropTa);

	return Status;
}

/** SR_CMD_SET_SR_GLO_VAR_STA_INIT_CTRL **/
NDIS_STATUS SetSrStaInit(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
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
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, prSrCmdSrGlobalVarSingleDropTa->rSrCmd.u1ArgNum));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srstainit=[u4DropTaIdx]-[u4StaIdx]\n",
			  __func__));
	}

	if(prSrCmdSrGlobalVarSingleDropTa)
		os_free_mem(prSrCmdSrGlobalVarSingleDropTa);

	return Status;
}

/** SR_CMD_SET_SR_COND_ALL_CTRL **/
NDIS_STATUS SetSrCondAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4SrRcpiCckRateEn = 0, u4SrMacRcpiRateEn = 0, u4SrRxvRcpiRateEn = 0;
	UINT_32 u4SrRcpiHeRateEn = 0, u4SrRcpiVhtRateEn = 0, u4SrRcpiHtRateEn = 0;
	UINT_32 u4SrRcpiLgRateEn = 0, u4SrRxvEntry = 0, u4SrPeriodLimitEn = 0;
	UINT_32 u4SrPeriodLimit = 0;

	SR_CMD_SR_COND_T rSrCmdSrCond;

	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&rSrCmdSrCond, sizeof(SR_CMD_SR_COND_T));

	/* Assign Cmd Id */
	rSrCmdSrCond.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_COND_ALL_CTRL;
	rSrCmdSrCond.rSrCmd.u1ArgNum = SR_CMD_SET_SR_COND_ALL_CTRL_ARG_NUM;
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
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmdSrCond.rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, rSrCmdSrCond.rSrCmd.u1ArgNum));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcond=[SrRcpiCckRateEn]-[SrMacRcpiRateEn]-[SrRxvRcpiRateEn]-[SrRcpiHeRateEn]-[u4SrRcpiVhtRateEn]-[SrRcpiHtRateEn]-[SrRcpiLgRateEn]-[SrRxvEntry]-[SrPeriodLimitEn]-[SrPeriodLimit]\n",
			  __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_RCPI_TBL_ALL_CTRL **/
NDIS_STATUS SetSrRcpiTblAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
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
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmdSrRcpiTbl.rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, rSrCmdSrRcpiTbl.rSrCmd.u1ArgNum));
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {
				for (u1Index = 0; u1Index < SR_RCPITBL_MCS_NUM; u1Index++)
					rSrCmdSrRcpiTbl.rSrRcpiTbl.u1RcpiTblMcs[u1Index] = SRRcpiConv((INT_8)i4RcpiTblMcs[u1Index]);

				Status = SrCmdSRUpdateRcpiTbl(pAd, &rSrCmdSrRcpiTbl);
			}
		} while (0);
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srrcpitbl=[RcpiTblMcs[0]]-...-[RcpiTblMcs[11]]\n",
			  __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL **/
NDIS_STATUS SetSrRcpiTblOfstAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4RxBwRcpiOfst = 0, u4StbcRcpiOfst = 0, u4NumAntRcpiOfst = 0;
	UINT_32 u4LdpcRcpiOfst = 0, u4DcmRcpiOfst = 0, u4MacRcpiOfst = 0;
	UINT_32 u4SigRcpiOfst = 0;


	SR_CMD_SR_RCPITBL_OFST_T rSrCmdSrRcpiTblOfst;


	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&rSrCmdSrRcpiTblOfst, sizeof(SR_CMD_SR_RCPITBL_OFST_T));

	/* Assign Cmd Id */
	rSrCmdSrRcpiTblOfst.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL;
	rSrCmdSrRcpiTblOfst.rSrCmd.u1ArgNum = SR_CMD_SET_SR_RCPI_TBL_OFST_ALL_CTRL_ARG_NUM;
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
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmdSrRcpiTblOfst.rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, rSrCmdSrRcpiTblOfst.rSrCmd.u1ArgNum));
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
					= cpu2le16( rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2DcmRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2MacRcpiOfst
					= cpu2le16( rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2MacRcpiOfst);
				rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2SigRcpiOfst
					= cpu2le16(rSrCmdSrRcpiTblOfst.rSrRcpiTblOfst.u2SigRcpiOfst);
#endif
				Status = SrCmdSRUpdateRcpiTblOfst(pAd, &rSrCmdSrRcpiTblOfst);
			}
		} while (0);
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srrcpitblofst=[RxBwRcpiOfst]-[StbcRcpiOfst]-[NumAntRcpiOfst]-[LdpcRcpiOfst]-[DcmRcpiOfst]-[MacRcpiOfst]-[SigRcpiOfst]\n",
			  __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_Q_CTRL_ALL_CTRL **/
NDIS_STATUS SetSrQCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4SrRxRptEn = 0, u4SrCw = 0, u4SrSuspend = 0;
	UINT_32 u4SrBackOffMask = 0;


	SR_CMD_SR_Q_CTRL_T rSrCmdSrQCtrl;


	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&rSrCmdSrQCtrl, sizeof(SR_CMD_SR_Q_CTRL_T));

	/* Assign Cmd Id */
	rSrCmdSrQCtrl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_Q_CTRL_ALL_CTRL;
	rSrCmdSrQCtrl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_Q_CTRL_ALL_CTRL_ARG_NUM;
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
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmdSrQCtrl.rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, rSrCmdSrQCtrl.rSrCmd.u1ArgNum));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srqctrl=[SrRxRptEn]-[SrCw]-[SrSuspend]-[SrBackOffMask]\n",
			  __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_IBPD_ALL_CTRL **/
NDIS_STATUS SetSrIBPDAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
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
					u4InterBssByHdrBssid = simple_strtol(value, 0, 10);
					Status = IsInRange(u4InterBssByHdrBssid, u1ArgNum, 0, BIT8);
					break;
				case 1:
					u4InterBssByMu = simple_strtol(value, 0, 10);
					Status = IsInRange(u4InterBssByMu, u1ArgNum, 0, BIT8);
					break;
				case 2:
					u4InterBssByPbssColor = simple_strtol(value, 0, 10);
					Status = IsInRange(u4InterBssByPbssColor, u1ArgNum, 0, BIT8);
					break;
				case 3:
					u4InterBssByPaid = simple_strtol(value, 0, 10);
					Status = IsInRange(u4InterBssByPaid, u1ArgNum, 0, BIT8);
					break;
				case 4:
					u4InterBssByBssColor = simple_strtol(value, 0, 10);
					Status = IsInRange(u4InterBssByBssColor, u1ArgNum, 0, BIT8);
					break;
				default:{
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmdSrIBPD.rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, rSrCmdSrIBPD.rSrCmd.u1ArgNum));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set sribpd=[InterBssByHdrBssid]-[InterBssByMu]-[InterBssByPbssColor]-[InterBssByPaid]-[InterBssByBssColor]\n",
			  __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_NRT_ALL_CTRL **/
NDIS_STATUS SetSrNRTAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4TableIdx = 0, u4NRTValue = 0;


	SR_CMD_SR_NRT_T rSrCmdSrNRT;


	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&rSrCmdSrNRT, sizeof(SR_CMD_SR_NRT_T));

	/* Assign Cmd Id */
	rSrCmdSrNRT.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_NRT_ALL_CTRL;
	rSrCmdSrNRT.rSrCmd.u1ArgNum = SR_CMD_SET_SR_NRT_ALL_CTRL_ARG_NUM;
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
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmdSrNRT.rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, rSrCmdSrNRT.rSrCmd.u1ArgNum));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srnrt=[TableIdx]-[NRTValue]\n",
			  __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_NRT_RESET_CTRL **/
NDIS_STATUS SetSrNRTResetAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_NRT_RESET_CTRL, SR_CMD_SET_SR_NRT_RESET_CTRL_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srnrtreset=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL **/
NDIS_STATUS SetSrNRTCtrlAll(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT_8 u1ArgNum = 0;
	CHAR *value;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
	    get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT_8 u1DbdcIdx = BAND0;

	UINT_32 u4SrtEn = 0, u4SrtSrpEn = 0, u4SrtAddrOrderEn = 0;
	UINT_32 u4SrtInRcpiTh = 0, u4SrtOutRcpiTh = 0, u4SrtUsedCntTh = 0;


	SR_CMD_SR_NRT_CTRL_T rSrCmdSrNRTCtrl;


	if (wdev != NULL)
		u1DbdcIdx = HcGetBandByWdev(wdev);
	else
		return NDIS_STATUS_FAILURE;

	os_zero_mem(&rSrCmdSrNRTCtrl, sizeof(SR_CMD_SR_NRT_CTRL_T));

	/* Assign Cmd Id */
	rSrCmdSrNRTCtrl.rSrCmd.u1CmdSubId = SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL;
	rSrCmdSrNRTCtrl.rSrCmd.u1ArgNum = SR_CMD_SET_SR_NRT_CTRL_ALL_CTRL_ARG_NUM;
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
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmdSrNRTCtrl.rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, rSrCmdSrNRTCtrl.rSrCmd.u1ArgNum));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srnrtctrl=[SrtEn]-[SrtSrpEn]-[SrtAddrOrderEn]-[SrtInRcpiTh]-[SrtOutRcpiTh]-[SrtUsedCntTh]\n",
			  __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_ENABLE **/
NDIS_STATUS SetSrCfgSrEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgsren=1\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_SD_ENABLE **/
NDIS_STATUS SetSrCfgSrSdEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_SD_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgsrsden=1\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_BF **/
NDIS_STATUS SetSrCfgSrBf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_BF, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgsrbf=1\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_ATF **/
NDIS_STATUS SetSrCfgSrAtf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_ATF, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgsratf=1\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_MODE **/
NDIS_STATUS SetSrCfgSrMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_MODE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgsrmode=1\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_DISRT_ENABLE **/
NDIS_STATUS SetSrCfgDISRTEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_DISRT_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgdisrten=1\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_DISRT_MIN_RSSI **/
NDIS_STATUS SetSrCfgDISRTMinRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_DISRT_MIN_RSSI, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgdisrtmin=62 (Range:0 ~ 110)\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_TXC_QUEUE **/
NDIS_STATUS SetSrCfgTxcQueue(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_TXC_QUEUE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgtxcq=1\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_TXC_QID **/
NDIS_STATUS SetSrCfgTxcQid(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_TXC_QID, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgtxcqid=86\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_TXC_PATH **/
NDIS_STATUS SetSrCfgTxcPath(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_TXC_PATH, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgtxcpath=1\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_AC_METHOD **/
NDIS_STATUS SetSrCfgAcMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_AC_METHOD, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgac=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_PERIOD_THR **/
NDIS_STATUS SetSrCfgSrPeriodThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_PERIOD_THR, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgsrperiodthr=480\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_QUERY_TXD_METHOD **/
NDIS_STATUS SetSrCfgQueryTxDMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_QUERY_TXD_METHOD, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgsrquerytxd=1\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_SD_CG_RATIO **/
NDIS_STATUS SetSrCfgSrSdCgRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_SD_CG_RATIO, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgsrsdcg=800\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_SR_SD_OBSS_RATIO **/
NDIS_STATUS SetSrCfgSrSdObssRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_SR_SD_OBSS_RATIO, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgsrsdobss=500\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_PROFILE **/
NDIS_STATUS SetSrCfgProfile(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_PROFILE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgsrprofile=3\n"
			 "SR_PROFILE_QUERY_TXD_TIME           BIT(0)\n"
			 "SR_PROFILE_SHOW_Q_LEN               BIT(1)\n"
			 "SR_PROFILE_RPT_HANDLE_TIME          BIT(2)\n"
			 "SR_PROFILE_GEN_TXC_TIME             BIT(3)\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_CFG_DPD_ENABLE **/
NDIS_STATUS SetSrCfgDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_CFG_DPD_ENABLE, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srcfgdpden=1\n", __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_SRG_BITMAP **/
NDIS_STATUS SetSrSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
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
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmdSrSrgBitmap.rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, rSrCmdSrSrgBitmap.rSrCmd.u1ArgNum));
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
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srsrgbm=[Color_31_0]-[Color_63_32]-[pBssid_31_0]-[pBssid_63_32]\n",
			  __func__));
	}

	return Status;
}

/** SR_CMD_SET_SR_SRG_BITMAP_REFRESH **/
NDIS_STATUS SetSrSrgBitmapRefresh(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_SET_SR_SRG_BITMAP_REFRESH, SR_CMD_SET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 set srsrgbmrefresh=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CAP_ALL_INFO **/
NDIS_STATUS ShowSrCap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CAP_ALL_INFO, SR_CMD_GET_SR_CAP_ALL_INFO_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcap=0\n", __func__));
	}

	return Status;
}


/** SR_CMD_GET_SR_PARA_ALL_INFO **/
NDIS_STATUS ShowSrPara(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_PARA_ALL_INFO, SR_CMD_GET_SR_PARA_ALL_INFO_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srpara=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_IND_ALL_INFO **/
NDIS_STATUS ShowSrInd(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_IND_ALL_INFO, SR_CMD_GET_SR_IND_ALL_INFO_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srind=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_GLO_VAR_SINGLE_DROP_TA_INFO **/
NDIS_STATUS ShowSrInfo(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
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
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, rSrCmd.u1ArgNum));
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {

				rSrCmd.u1DropTaIdx = u1DropTaIdx;
				rSrCmd.u1StaIdx = u1StaIdx;
				Status = SrCmd(pAd, &rSrCmd);
			}
		} while (0);
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srinfo=0-0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_COND_ALL_INFO **/
NDIS_STATUS ShowSrCond(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_COND_ALL_INFO, SR_CMD_GET_SR_COND_ALL_INFO_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcond=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_RCPI_TBL_ALL_INFO **/
NDIS_STATUS ShowSrRcpiTbl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_RCPI_TBL_ALL_INFO, SR_CMD_GET_SR_RCPI_TBL_ALL_INFO_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srrcpitbl=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_RCPI_TBL_OFST_ALL_INFO **/
NDIS_STATUS ShowSrRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_RCPI_TBL_OFST_ALL_INFO, SR_CMD_GET_SR_RCPI_TBL_OFST_ALL_INFO_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srrcpitblofst=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_Q_CTRL_ALL_INFO **/
NDIS_STATUS ShowSrQCtrl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_Q_CTRL_ALL_INFO, SR_CMD_GET_SR_Q_CTRL_ALL_INFO_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srqctrl=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_IBPD_ALL_INFO **/
NDIS_STATUS ShowSrIBPD(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_IBPD_ALL_INFO, SR_CMD_GET_SR_IBPD_ALL_INFO_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show sribpd=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_NRT_ALL_INFO **/
NDIS_STATUS ShowSrNRT(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_NRT_ALL_INFO, SR_CMD_GET_SR_NRT_ALL_INFO_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srnrt=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_NRT_CTRL_ALL_INFO **/
NDIS_STATUS ShowSrNRTCtrl(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_NRT_CTRL_ALL_INFO, SR_CMD_GET_SR_NRT_CTRL_ALL_INFO_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srnrtctrl=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_SR_ENABLE **/
NDIS_STATUS ShowSrCfgSrEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgsren=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_SR_SD_ENABLE **/
NDIS_STATUS ShowSrCfgSrSdEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_SD_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgsrsden=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_SR_BF **/
NDIS_STATUS ShowSrCfgSrBf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_BF, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgsrbf=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_SR_ATF **/
NDIS_STATUS ShowSrCfgSrAtf(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_ATF, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgsratf=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_SR_MODE **/
NDIS_STATUS ShowSrCfgSrMode(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_MODE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgsrmode=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_DISRT_ENABLE **/
NDIS_STATUS ShowSrCfgDISRTEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_DISRT_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgdisrten=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_DISRT_MIN_RSSI **/
NDIS_STATUS ShowSrCfgDISRTMinRssi(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_DISRT_MIN_RSSI, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgdisrtmin=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_TXC_QUEUE **/
NDIS_STATUS ShowSrCfgTxcQueue(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_TXC_QUEUE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgtxcq=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_TXC_QID **/
NDIS_STATUS ShowSrCfgTxcQid(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_TXC_QID, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgtxcqid=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_TXC_PATH **/
NDIS_STATUS ShowSrCfgTxcPath(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_TXC_PATH, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgtxcpath=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_AC_METHOD **/
NDIS_STATUS ShowSrCfgAcMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_AC_METHOD, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgac=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_SR_PERIOD_THR **/
NDIS_STATUS ShowSrCfgSrPeriodThr(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_PERIOD_THR, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgsrperiodthr=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_QUERY_TXD_METHOD **/
NDIS_STATUS ShowSrCfgQueryTxDMethod(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_QUERY_TXD_METHOD, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgsrquerytxd=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_SR_SD_CG_RATIO **/
NDIS_STATUS ShowSrCfgSrSdCgRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_SD_CG_RATIO, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgsrsdcg=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_SR_SD_OBSS_RATIO **/
NDIS_STATUS ShowSrCfgSrSdObssRatio(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_SR_SD_OBSS_RATIO, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgsrsdobss=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_PROFILE **/
NDIS_STATUS ShowSrCfgProfile(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_PROFILE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgsrprofile=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CFG_DPD_ENABLE **/
NDIS_STATUS ShowSrCfgDPDEnable(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CFG_DPD_ENABLE, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcfgdpden=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_CNT_ALL **/
NDIS_STATUS ShowSrCnt(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_CNT_ALL, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srcnt=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_SD_ALL **/
NDIS_STATUS ShowSrSd(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_SD_ALL, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srsd=0\n", __func__));
	}

	return Status;
}

/** SR_CMD_GET_SR_SRG_BITMAP **/
NDIS_STATUS ShowSrSrgBitmap(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	Status = SrCmdShow(pAd, arg, SR_CMD_GET_SR_SRG_BITMAP, SR_CMD_GET_DEFAULT_ARG_NUM);
	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: iwpriv ra0 show srsrgbm=0\n", __func__));
	}

	return Status;
}

/* for set/show function*/
NDIS_STATUS SrCmdSRUpdateCap(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_CAP_T prSrCmdSrCap)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrCap not null */
	if (!prSrCmdSrCap) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrCap */
	PrintSrCmdSrCap(prSrCmdSrCap);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_CAP_T));
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
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrCap, sizeof(SR_CMD_SR_CAP_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d)\n", __func__, ret));
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
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d)\n", __func__, ret));
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
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}

NDIS_STATUS SrCmdSRUpdateCond(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_COND_T prSrCmdSrCond)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrCond not null */
	if (!prSrCmdSrCond) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrCond */
	PrintSrCmdSrCond(prSrCmdSrCond);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_COND_T));
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
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrCond, sizeof(SR_CMD_SR_COND_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d)\n", __func__, ret));
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
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}

NDIS_STATUS SrCmdSRUpdateRcpiTblOfst(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_RCPITBL_OFST_T prSrCmdSrRcpiTblOfst)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrRcpiTblOfst not null */
	if (!prSrCmdSrRcpiTblOfst) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrRcpiTblOfst */
	PrintSrCmdSrRcpiTblOfst(prSrCmdSrRcpiTblOfst);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_RCPITBL_OFST_T));
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
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrRcpiTblOfst, sizeof(SR_CMD_SR_RCPITBL_OFST_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}

NDIS_STATUS SrCmdSRUpdateQCtrl(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_Q_CTRL_T prSrCmdSrQCtrl)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrQCtrl not null */
	if (!prSrCmdSrQCtrl) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrQCtrl */
	PrintSrCmdSrQCtrl(prSrCmdSrQCtrl);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_Q_CTRL_T));
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
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrQCtrl, sizeof(SR_CMD_SR_Q_CTRL_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d)\n", __func__, ret));
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
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}

NDIS_STATUS SrCmdSRUpdateNRT(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_NRT_T prSrCmdSrNRT)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrNRT not null */
	if (!prSrCmdSrNRT) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrNRT */
	PrintSrCmdSrNRT(prSrCmdSrNRT);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_NRT_T));
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
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrNRT, sizeof(SR_CMD_SR_NRT_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}

NDIS_STATUS SrCmdSRUpdateNRTCtrl(IN PRTMP_ADAPTER pAd, IN P_SR_CMD_SR_NRT_CTRL_T prSrCmdSrNRTCtrl)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct cmd_msg *msg;
	CMD_ATTRIBUTE attr = { 0 };

	/* Check prSrCmdSrNRTCtrl not null */
	if (!prSrCmdSrNRTCtrl) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	/* Print prSrCmdSrNRTCtrl */
	PrintSrCmdSrNRTCtrl(prSrCmdSrNRTCtrl);

	/* Allocate msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(SR_CMD_SR_NRT_CTRL_T));
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
	AndesAppendCmdMsg(msg, (char *)prSrCmdSrNRTCtrl, sizeof(SR_CMD_SR_NRT_CTRL_T));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d)\n", __func__, ret));
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
	PrintSrCmdSrSrgBitmap(prSrCmdSrSrgBitmap);

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
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}


NDIS_STATUS SrCmdShow(IN PRTMP_ADAPTER pAd, IN RTMP_STRING * arg, IN UINT_8 u1CmdSubId, IN UINT_8 u1ArgNum)
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
					rSrCmd.u4Value= cpu2le32(rSrCmd.u4Value);
#endif
					break;
				default:{
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: set wrong parameters\n", __func__));
						Status = NDIS_STATUS_FAILURE;
						break;
					}
				}
			}

			if (Status == NDIS_STATUS_FAILURE)
				break;

			if (u1ArgNum != rSrCmd.u1ArgNum) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Format Error! ArgNum = %d != %d\n", __func__,
					  u1ArgNum, rSrCmd.u1ArgNum));
				Status = NDIS_STATUS_FAILURE;
				break;
			} else {

				Status = SrCmd(pAd, &rSrCmd);
			}
		} while (0);
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Arg is Null\n", __func__));
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
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}


VOID EventSrHandler(PRTMP_ADAPTER pAd, UINT8 *Data, UINT_32 Length)
{
	/* Event ID */
	UINT8 u1EventSubId;

	/* Get Event Category ID */
	u1EventSubId = *Data;

	/**Prevent legitimate but wrong ID **/
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 ("%s: u1EventSubId = %d\n", __func__, u1EventSubId));

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
	case SR_EVENT_GET_SR_CFG_DPD_ENABLE:
		PrintSrEvent((P_SR_EVENT_T)Data);
		break;
	case SR_EVENT_GET_SR_SRG_BITMAP:
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
		PrintSrEventSrCap((P_SR_EVENT_SR_CAP_T)Data);
		break;
	case SR_EVENT_GET_SR_PARA_ALL_INFO:
		PrintSrEventSrPara((P_SR_EVENT_SR_PARA_T)Data);
		break;
	case SR_EVENT_GET_SR_COND_ALL_INFO:
		PrintSrEventSrCond((P_SR_EVENT_SR_COND_T)Data);
		break;
	case SR_EVENT_GET_SR_RCPI_TBL_ALL_INFO:
		PrintSrEventSrRcpiTbl((P_SR_EVENT_SR_RCPITBL_T)Data);
		break;
	case SR_EVENT_GET_SR_RCPI_TBL_OFST_ALL_INFO:
		PrintSrEventSrRcpiTblOfst((P_SR_EVENT_SR_RCPITBL_OFST_T)Data);
		break;
	case SR_EVENT_GET_SR_Q_CTRL_ALL_INFO:
		PrintSrEventSrQCtrl((P_SR_EVENT_SR_Q_CTRL_T)Data);
		break;
	case SR_EVENT_GET_SR_IBPD_ALL_INFO:
		PrintSrEventSrIBPD((P_SR_EVENT_SR_IBPD_T)Data);
		break;
	case SR_EVENT_GET_SR_NRT_ALL_INFO:
		PrintSrEventSrNRT((P_SR_EVENT_SR_NRT_T)Data);
		break;
	case SR_EVENT_GET_SR_NRT_CTRL_ALL_INFO:
		PrintSrEventSrNRTCtrl((P_SR_EVENT_SR_NRT_CTRL_T)Data);
		break;
	case SR_EVENT_GET_SR_IND_ALL_INFO:
		PrintSrEventSrInd((P_SR_EVENT_SR_IND_T)Data);
		break;
	default:
		break;
	}
}

/* for check value */
NDIS_STATUS IsFlag(IN INT32 u4ArgVal, IN UINT_8 u1ArgNum)
{
	if (u4ArgVal != 0 && u4ArgVal != 1) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("ArgNum[%d] = %d is invalid Value! (ArgVal !=0 && ArgVal !=1)\n",
			  u1ArgNum, u4ArgVal));
		return NDIS_STATUS_FAILURE;
	}
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS IsInRange(IN INT32 u4ArgVal, IN UINT_8 u1ArgNum, IN INT32 u4Valfrom, IN INT32 u4Valto)
{
	if (u4ArgVal < u4Valfrom || u4ArgVal > u4Valto) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("ArgNum[%d] = %d is invalid Value! (ArgVal < %d or ArgVal > %d)\n",
			  u1ArgNum, u4ArgVal, u4Valfrom, u4Valto));
		return NDIS_STATUS_FAILURE;
	}
	return NDIS_STATUS_SUCCESS;
}

VOID PrintSrCmd(IN P_SR_CMD_T prSrCmd)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s:\n"
		  "u1CmdSubId = %x, u1ArgNum = %d, u1DbdcIdx = %d, u1Status = %d\n"
		  "u1DropTaIdx = %d, u1StaIdx = %d, u4Value = %d\n",
		  __func__, prSrCmd->u1CmdSubId, prSrCmd->u1ArgNum, prSrCmd->u1DbdcIdx,
		  prSrCmd->u1Status, prSrCmd->u1DropTaIdx, prSrCmd->u1StaIdx, prSrCmd->u4Value));
}

VOID PrintSrCap(IN P_WH_SR_CAP_T prSrCap)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s:\n"
		  "fgSrEn                 = %x, fgSrgEn              = %x, fgNonSrgEn              = %x\n"
		  "fgSingleMdpuRtsctsEn   = %x, fgHdrDurEn           = %x, fgTxopDurEn             = %x\n"
		  "fgNonSrgInterPpduPresv = %x, fgSrgInterPpduPresv  = %x, fgSrRemTimeEn           = %x\n"
		  "fgProtInSrWinDis       = %x, fgTxCmdDlRateSelEn   = %x, fgAmpduTxCntEn          = %x\n",
		  __func__, prSrCap->fgSrEn, prSrCap->fgSrgEn, prSrCap->fgNonSrgEn,
		  prSrCap->fgSingleMdpuRtsctsEn, prSrCap->fgHdrDurEn, prSrCap->fgTxopDurEn,
		  prSrCap->fgNonSrgInterPpduPresv, prSrCap->fgSrgInterPpduPresv,
		  prSrCap->fgSrRemTimeEn, prSrCap->fgProtInSrWinDis, prSrCap->fgTxCmdDlRateSelEn,
		  prSrCap->fgAmpduTxCntEn));
}

VOID PrintSrPara(IN P_WH_SR_PARA_T prSrPara)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s:\n"
		  "u1NonSrgPdThr   = %x, u1SrgPdThr        = %x, u1PeriodOfst   = %x\n"
		  "u1RcpiSourceSel = %x, u2ObssPdMin       = %x, u2ObssPdMinSrg = %x\n"
		  "eRespTxPwrMode  = %x, eTxPwrRestricMode = %x, u1ObssTxPwrRef = %x\n",
		  __func__, prSrPara->u1NonSrgPdThr, prSrPara->u1SrgPdThr, prSrPara->u1PeriodOfst,
		  prSrPara->u1RcpiSourceSel, prSrPara->u2ObssPdMin, prSrPara->u2ObssPdMinSrg,
		  prSrPara->eRespTxPwrMode, prSrPara->eTxPwrRestricMode, prSrPara->u1ObssTxPwrRef));
}

VOID PrintSrInd(IN P_WH_SR_IND_T prSrInd)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s:\n"
		  "u1NonSrgInterPpduRcpi   = %x, u1SrgInterPpduRcpi     = %x\n"
		  "u2NonSrgVldCnt          = %x, u2SrgVldCnt            = %x\n"
		  "u2IntraBssPpduCnt       = %x, u2InterBssPpduCnt      = %x\n"
		  "u2NonSrgPpduVldCnt      = %x, u2SrgPpduVldCnt        = %x\n"
		  "u4SrAmpduMpduCnt        = %x, u4SrAmpduMpduAckedCnt  = %x\n",
		  __func__, prSrInd->u1NonSrgInterPpduRcpi, prSrInd->u1SrgInterPpduRcpi,
		  prSrInd->u2NonSrgVldCnt, prSrInd->u2SrgVldCnt, prSrInd->u2IntraBssPpduCnt,
		  prSrInd->u2InterBssPpduCnt, prSrInd->u2NonSrgPpduVldCnt, prSrInd->u2SrgPpduVldCnt,
		  prSrInd->u4SrAmpduMpduCnt, prSrInd->u4SrAmpduMpduAckedCnt));
}

VOID PrintSrGloVarSingleDropTa(IN P_SR_GLOBAL_VAR_SINGLE_DROP_TA_T prSrGlobalVarSingleDropTa,
			       IN UINT_8 u1DropTaIdx, IN UINT_8 u1StaIdx)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("SR Info - u1CurSrDropTaIdx = %d, u2SrTtlTxCntThr=%d\n",
		  prSrGlobalVarSingleDropTa->u1CurSrDropTaIdx,
		  prSrGlobalVarSingleDropTa->u2SrTtlTxCntThr));

	PrintSrDropTaInfo(&(prSrGlobalVarSingleDropTa->rSrDropTaInfo), u1DropTaIdx, u1StaIdx);

}

VOID PrintSrDropTaInfo(IN P_SR_DROP_TA_INFO_T prSrDropTaInfo, IN UINT_8 u1DropTaIdx,
		       IN UINT_8 u1StaIdx)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("    DropTa %2d - Address : %x\n",
		  u1DropTaIdx, prSrDropTaInfo->u4Address2));

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
	CHAR * mode[2] = {"AUTO", "FIXED"};
	CHAR * state[5] = {"Invailid", "Not Stable", "Good", "Bad", "Timeout"};
	CHAR * rastate[2] = {"Stable", "Active"};

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("        STA %2d\n"
		  "        u2WlanId       = %d, u1Mode       = %s,  u1State = %s\n",
		  u1StaIdx, prSrStaInfo->u2WlanId, mode[prSrStaInfo->u1Mode], state[prSrStaInfo->u1State]));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("        u1SrRateOffset = %d, u1SrRaTryCnt = %x, u1SrRaRound = %x, u1SrRaState = %s\n",
		  prSrStaInfo->u1SrRateOffset, prSrStaInfo->u1SrRaTryCnt, prSrStaInfo->u1SrRaRound, rastate[prSrStaInfo->u1SrRaState]));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("        u2SrSucCnt  = %x, u2SrTtlTxCnt = %x, PER = %d.%1d%%\n",
		  prSrStaInfo->u2SrSucCnt, prSrStaInfo->u2SrTtlTxCnt, per / 10, per % 10));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("        u4Score = %x, u2BadQuota = %d, u1BadLevel = %d, u1SrRate = %x\n",
		  prSrStaInfo->u4Score, prSrStaInfo->u2BadQuota, prSrStaInfo->u1BadLevel, prSrStaInfo->u1SrRate));

}

VOID PrintSrCond(IN P_SR_COND_T prSrCond)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s:\n"
		  "fgSrRcpiCckRateEn = %x, fgSrMacRcpiRateEn = %x, fgSrRxvRcpiRateEn = %x\n"
		  "fgSrRcpiHeRateEn  = %x, fgSrRcpiVhtRateEn = %x, fgSrRcpiHtRateEn  = %x\n"
		  "fgSrRcpiLgRateEn  = %x, fgSrRxvEntry      = %x, fgSrPeriodLimitEn = %x\n"
		  "u1SrPeriodLimit   = %x\n",
		  __func__, prSrCond->fgSrRcpiCckRateEn, prSrCond->fgSrMacRcpiRateEn, prSrCond->fgSrRxvRcpiRateEn,
		  prSrCond->fgSrRcpiHeRateEn, prSrCond->fgSrRcpiVhtRateEn, prSrCond->fgSrRcpiHtRateEn,
		  prSrCond->fgSrRcpiLgRateEn, prSrCond->fgSrRxvEntry, prSrCond->fgSrPeriodLimitEn,
		  prSrCond->u1SrPeriodLimit));
}

VOID PrintSrRcpiTbl(IN P_SR_RCPITBL_T prSrRcpiTbl)
{
	UINT_8 u1Index;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:\n", __func__));
	for (u1Index = 0; u1Index < SR_RCPITBL_MCS_NUM; u1Index++)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("u1RcpiTblMcs[%d] = %x (%d dBm)\n", u1Index, prSrRcpiTbl->u1RcpiTblMcs[u1Index], SRDbmConv(prSrRcpiTbl->u1RcpiTblMcs[u1Index])));
}

VOID PrintSrRcpiTblOfst(IN P_SR_RCPITBL_OFST_T prSrRcpiTblOfst)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s:\n"
		  "u2RxBwRcpiOfst = %x, u2StbcRcpiOfst = %x, u2NumAntRcpiOfst = %x\n"
		  "u2LdpcRcpiOfst = %x, u2DcmRcpiOfst  = %x, u2MacRcpiOfst    = %x\n"
		  "u2SigRcpiOfst  = %x\n",
		  __func__, prSrRcpiTblOfst->u2RxBwRcpiOfst, prSrRcpiTblOfst->u2StbcRcpiOfst, prSrRcpiTblOfst->u2NumAntRcpiOfst,
		  prSrRcpiTblOfst->u2LdpcRcpiOfst, prSrRcpiTblOfst->u2DcmRcpiOfst, prSrRcpiTblOfst->u2MacRcpiOfst,
		  prSrRcpiTblOfst->u2SigRcpiOfst));
}

VOID PrintSrQCtrl(IN P_WH_SR_QUEUE_CTRL_T prSrQCtrl)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s:\n"
		  "fgSrRxRptEn = %x, fgSrCw = %x, fgSrSuspend = %x, u4SrBackOffMask = %x\n",
		  __func__, prSrQCtrl->fgSrRxRptEn, prSrQCtrl->fgSrCw, prSrQCtrl->fgSrSuspend, prSrQCtrl->u4SrBackOffMask));
}

VOID PrintSrIBPD(IN P_SR_IBPD_T prSrIBPD)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s:\n"
		  "u1InterBssByHdrBssid = %x, u1InterBssByMu        = %x, u1InterBssByPbssColor = %x\n"
		  "u1InterBssByPaid     = %x, u1InterBssByBssColor  = %x\n",
		  __func__, prSrIBPD->u1InterBssByHdrBssid, prSrIBPD->u1InterBssByMu, prSrIBPD->u1InterBssByPbssColor,
		  prSrIBPD->u1InterBssByPaid, prSrIBPD->u1InterBssByBssColor));
}

VOID PrintSrNRT(IN P_SR_NRT_T prSrNRT)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("u1TableIdx = %x, u4NRTValue = %x\n",
		  prSrNRT->u1TableIdx, prSrNRT->u4NRTValue));
}

VOID PrintSrNRTCtrl(IN P_SR_NRT_CTRL_T prSrNRTCtrl)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s:\n"
		  "fgSrtEn       = %x, fgSrtSrpEn     = %x, fgSrtAddrOrderEn = %x\n"
		  "u2SrtInRcpiTh = %x, u2SrtOutRcpiTh = %x, u2SrtUsedCntTh   = %x\n",
		  __func__, prSrNRTCtrl->fgSrtEn, prSrNRTCtrl->fgSrtSrpEn, prSrNRTCtrl->fgSrtAddrOrderEn,
		  prSrNRTCtrl->u2SrtInRcpiTh, prSrNRTCtrl->u2SrtOutRcpiTh, prSrNRTCtrl->u2SrtUsedCntTh));
}

VOID PrintSrSrgBitmap(IN UINT_8 u1DbdcIdx, IN P_SR_SRG_BITMAP_T prSrSrgBitmap)
{
	UINT_8 u1BitmapIdx = 0, u1ColorEn = 0, u1pBssidEn = 0;
	CHAR *enable[2] = {" ", "V"};
	/*UINT_32 u4Color = 0, u4pBssid = 0;*/

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Color - 31_0:%x, 63_32:%x pBssid - 31_0:%x, 63_32:%x\n",
		__func__, prSrSrgBitmap->u4Color_31_0[u1DbdcIdx], prSrSrgBitmap->u4Color_63_32[u1DbdcIdx],
		prSrSrgBitmap->u4pBssid_31_0[u1DbdcIdx], prSrSrgBitmap->u4pBssid_63_32[u1DbdcIdx]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BIT  Color  pBssid\n"));


	for (u1BitmapIdx = 0; u1BitmapIdx < 64; u1BitmapIdx++) {
		if (u1BitmapIdx < 32) {
			u1ColorEn = (prSrSrgBitmap->u4Color_31_0[u1DbdcIdx] & BIT(u1BitmapIdx)) >> u1BitmapIdx;
			u1pBssidEn = (prSrSrgBitmap->u4pBssid_31_0[u1DbdcIdx] & BIT(u1BitmapIdx)) >> u1BitmapIdx;
		} else {
			u1ColorEn = (prSrSrgBitmap->u4Color_63_32[u1DbdcIdx] & BIT(u1BitmapIdx - 32)) >> (u1BitmapIdx - 32);
			u1pBssidEn = (prSrSrgBitmap->u4pBssid_63_32[u1DbdcIdx] & BIT(u1BitmapIdx - 32)) >> (u1BitmapIdx - 32);
		}

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%2d     %s      %s   \n",
		u1BitmapIdx, enable[u1ColorEn], enable[u1pBssidEn]));
	}
}

VOID PrintSrCnt(IN UINT_8 u1DbdcIdx, IN P_SR_CNT_T prSrCnt)
{
	UINT_8  u1SrRxrptSrc = ENUM_SR_RXRPT_SRC_RXRPT;
	UINT_8  u1SrEntry = ENUM_SR_ENTRY_NEWRXV;
	CHAR * srrxrptsrc[2] = {"RXRPT", "CMDRPT-TX"};

	UINT_16 au2RxrptTtl[ENUM_SR_RXRPT_SRC_NUM]     = {0};
	UINT_16	au2PeriodSucTtl[ENUM_SR_RXRPT_SRC_NUM] = {0}, au2PeriodFailTtl[ENUM_SR_RXRPT_SRC_NUM]  = {0};
	UINT_16 au2GenTxcSucTtl[ENUM_SR_RXRPT_SRC_NUM] = {0}, au2GenTxcFailTtl[ENUM_SR_RXRPT_SRC_NUM]  = {0};
	UINT_16 au2SrTxSucTtl[ENUM_SR_RXRPT_SRC_NUM]   = {0}, au2SrTxFailTtl[ENUM_SR_RXRPT_SRC_NUM]    = {0};

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Band%d\n", __func__, u1DbdcIdx));

	for (u1SrRxrptSrc = ENUM_SR_RXRPT_SRC_RXRPT; u1SrRxrptSrc < ENUM_SR_RXRPT_SRC_NUM; u1SrRxrptSrc++) {

		for (u1SrEntry = ENUM_SR_ENTRY_NEWRXV; u1SrEntry < ENUM_SR_ENTRY_NUM; u1SrEntry++) {
			au2RxrptTtl[u1SrRxrptSrc]     += prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][u1SrEntry].u2EntryTtl;
			au2PeriodSucTtl[u1SrRxrptSrc] += prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][u1SrEntry].u2PeriodSuc;
			au2GenTxcSucTtl[u1SrRxrptSrc] += prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][u1SrEntry].u2GenTxcSuc;
			au2SrTxSucTtl[u1SrRxrptSrc]   += prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][u1SrEntry].u2SrTxSuc;
		}
		au2SrTxFailTtl[u1SrRxrptSrc] = au2GenTxcSucTtl[u1SrRxrptSrc] - au2SrTxSucTtl[u1SrRxrptSrc];
		au2GenTxcFailTtl[u1SrRxrptSrc] = au2PeriodSucTtl[u1SrRxrptSrc] - au2GenTxcSucTtl[u1SrRxrptSrc];
		au2PeriodFailTtl[u1SrRxrptSrc] = au2RxrptTtl[u1SrRxrptSrc] - au2PeriodSucTtl[u1SrRxrptSrc];

		if (u1SrRxrptSrc == ENUM_SR_RXRPT_SRC_RXRPT) {
			au2RxrptTtl[ENUM_SR_RXRPT_SRC_RXRPT] += prSrCnt->u2EntryNoSrTtl[u1DbdcIdx];
			au2RxrptTtl[ENUM_SR_RXRPT_SRC_RXRPT] += prSrCnt->u2EntryFailTtl[u1DbdcIdx];
		}

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("-------------------------------------------------------------------\n"
			  "SR Rxrpt Source : %s\n"
			  "Total Rxrpt  = %4x,\n"
			  "      NewRxv = %4x, OriRxvVht = %4x, OriRxvHe = %4x,\n"
			  "      NewMac = %4x, OriMac    = %4x,\n",
			  srrxrptsrc[u1SrRxrptSrc],
			  au2RxrptTtl[u1SrRxrptSrc],
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV].u2EntryTtl,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT].u2EntryTtl,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE].u2EntryTtl,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC].u2EntryTtl,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC].u2EntryTtl));

		if (u1SrRxrptSrc == ENUM_SR_RXRPT_SRC_RXRPT) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("      NoSr   = %4x, Fail      = %4x,\n"
				  "-------------------------------------------------------------------\n",
				  prSrCnt->u2EntryNoSrTtl[u1DbdcIdx],
				  prSrCnt->u2EntryFailTtl[u1DbdcIdx]));
		} else {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("-------------------------------------------------------------------\n"));
		}

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Total Period    Succ  = %4x, Fail = %4x,\n"
			  "      NewRxv    Succ  = %4x, Fail = %4x,\n"
			  "      OriRxvVht Succ  = %4x, Fail = %4x,\n"
			  "      OriRxvHe  Succ  = %4x, Fail = %4x,\n"
			  "      NewMac    Succ  = %4x, Fail = %4x,\n"
			  "      OriMac    Succ  = %4x, Fail = %4x,\n"
			  "-------------------------------------------------------------------\n",
			  au2PeriodSucTtl[u1SrRxrptSrc], au2PeriodFailTtl[u1SrRxrptSrc],
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV].u2EntryTtl - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT].u2EntryTtl - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE].u2EntryTtl - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC].u2EntryTtl - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC].u2PeriodSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC].u2EntryTtl - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC].u2PeriodSuc));

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Total Gen Txc   Succ  = %4x, Fail = %4x,\n"
			  "      NewRxv    Succ  = %4x, Fail = %4x,\n"
			  "      OriRxvVht Succ  = %4x, Fail = %4x,\n"
			  "      OriRxvHe  Succ  = %4x, Fail = %4x,\n"
			  "      NewMac    Succ  = %4x, Fail = %4x,\n"
			  "      OriMac    Succ  = %4x, Fail = %4x,\n"
			  "-------------------------------------------------------------------\n",
			  au2GenTxcSucTtl[u1SrRxrptSrc], au2GenTxcFailTtl[u1SrRxrptSrc],
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV].u2PeriodSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT].u2PeriodSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE].u2PeriodSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC].u2PeriodSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC].u2GenTxcSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC].u2PeriodSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC].u2GenTxcSuc));


		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Total SR Tx     Succ  = %4x, Fail = %4x,\n"
			  "      NewRxv    Succ  = %4x, Fail = %4x,\n"
			  "      OriRxvVht Succ  = %4x, Fail = %4x,\n"
			  "      OriRxvHe  Succ  = %4x, Fail = %4x,\n"
			  "      NewMac    Succ  = %4x, Fail = %4x,\n"
			  "      OriMac    Succ  = %4x, Fail = %4x,\n"
			  "-------------------------------------------------------------------\n",
			  au2SrTxSucTtl[u1SrRxrptSrc], au2SrTxFailTtl[u1SrRxrptSrc],
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV].u2GenTxcSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWRXV].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT].u2GenTxcSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVVHT].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE].u2GenTxcSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIRXVHE].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC].u2GenTxcSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_NEWMAC].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC].u2SrTxSuc,
			  prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC].u2GenTxcSuc - prSrCnt->rSrCntEntry[u1DbdcIdx][u1SrRxrptSrc][ENUM_SR_ENTRY_ORIMAC].u2SrTxSuc));
	}
}

VOID PrintSrSd(IN UINT_8 u1DbdcIdx, IN P_SR_SD_T prSrSd)
{
	CHAR * srsdrules[4] = {"1 - NO CONNECTED", "2 - NO CONGESTION", "3 - NO INTERFERENCE", "4 - ALL RULE PASS" };
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Band%d\n", __func__, u1DbdcIdx));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("-------------------------------------------------------------------\n"
		  "Hit Rule      = %s\n"
		  "Rxrpt Count   = %d\n"
		  " 	 RXV     = %d\n"
		  " 	 MAC     = %d\n"
		  "-------------------------------------------------------------------\n",
		  srsdrules[prSrSd->u1Rule[u1DbdcIdx]],
		  prSrSd->u2RxrptRxvCnt[u1DbdcIdx] + prSrSd->u2RxrptMacCnt[u1DbdcIdx],
		  prSrSd->u2RxrptRxvCnt[u1DbdcIdx],
		  prSrSd->u2RxrptMacCnt[u1DbdcIdx]));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("Timer Period  = %d(us)\n"
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
		  prSrSd->u4TtlAirTimeRatio[u1DbdcIdx] % 10));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("Total Airtime = %d(us)\n"
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
		  prSrSd->u4OBSSAirTimeRatio[u1DbdcIdx] % 10));

}


VOID PrintSrCmdSrCap(IN P_SR_CMD_SR_CAP_T prSrCmdSrCap)
{
	PrintSrCmd(&(prSrCmdSrCap->rSrCmd));
	PrintSrCap(&(prSrCmdSrCap->rSrCap));
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

VOID PrintSrCmdSrCond(IN P_SR_CMD_SR_COND_T prSrCmdSrCond)
{
	PrintSrCmd(&(prSrCmdSrCond->rSrCmd));
	PrintSrCond(&(prSrCmdSrCond->rSrCond));
}

VOID PrintSrCmdSrRcpiTbl(IN P_SR_CMD_SR_RCPITBL_T prSrCmdSrRcpiTbl)
{
	PrintSrCmd(&(prSrCmdSrRcpiTbl->rSrCmd));
	PrintSrRcpiTbl(&(prSrCmdSrRcpiTbl->rSrRcpiTbl));
}

VOID PrintSrCmdSrRcpiTblOfst(IN P_SR_CMD_SR_RCPITBL_OFST_T prSrCmdSrRcpiTblOfst)
{
	PrintSrCmd(&(prSrCmdSrRcpiTblOfst->rSrCmd));
	PrintSrRcpiTblOfst(&(prSrCmdSrRcpiTblOfst->rSrRcpiTblOfst));
}

VOID PrintSrCmdSrQCtrl(IN P_SR_CMD_SR_Q_CTRL_T prSrCmdSrQCtrl)
{
	PrintSrCmd(&(prSrCmdSrQCtrl->rSrCmd));
	PrintSrQCtrl(&(prSrCmdSrQCtrl->rSrQCtrl));
}

VOID PrintSrCmdSrIBPD(IN P_SR_CMD_SR_IBPD_T prSrCmdSrIBPD)
{
	PrintSrCmd(&(prSrCmdSrIBPD->rSrCmd));
	PrintSrIBPD(&(prSrCmdSrIBPD->rSrIBPD));
}

VOID PrintSrCmdSrNRT(IN P_SR_CMD_SR_NRT_T prSrCmdSrNRT)
{
	PrintSrCmd(&(prSrCmdSrNRT->rSrCmd));
	PrintSrNRT(&(prSrCmdSrNRT->rSrNRT));
}

VOID PrintSrCmdSrNRTCtrl(IN P_SR_CMD_SR_NRT_CTRL_T prSrCmdSrNRTCtrl)
{
	PrintSrCmd(&(prSrCmdSrNRTCtrl->rSrCmd));
	PrintSrNRTCtrl(&(prSrCmdSrNRTCtrl->rSrNRTCtrl));
}

VOID PrintSrCmdSrSrgBitmap(IN P_SR_CMD_SR_SRG_BITMAP_T prSrCmdSrSrgBitmap)
{
	PrintSrCmd(&(prSrCmdSrSrgBitmap->rSrCmd));
	PrintSrSrgBitmap(prSrCmdSrSrgBitmap->rSrCmd.u1DbdcIdx, &(prSrCmdSrSrgBitmap->rSrSrgBitmap));
}

VOID PrintSrEvent(IN P_SR_EVENT_T prSrEvent)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s:\n"
		  "u1EventSubId = %x, u1ArgNum = %d, u1DbdcIdx = %d, u1Status = %d\n"
		  "u1DropTaIdx = %d, u1StaIdx = %d, u4Value = %d\n",
		  __func__, prSrEvent->u1EventSubId, prSrEvent->u1ArgNum, prSrEvent->u1DbdcIdx,
		  prSrEvent->u1Status, prSrEvent->u1DropTaIdx, prSrEvent->u1StaIdx, prSrEvent->u4Value));

}

VOID PrintSrEventSrCap(IN P_SR_EVENT_SR_CAP_T prSrEventSrCap)
{
	PrintSrEvent(&(prSrEventSrCap->rSrEvent));
	PrintSrCap(&(prSrEventSrCap->rSrCap));
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

VOID PrintSrEventSrCond(IN P_SR_EVENT_SR_COND_T prSrEventSrCond)
{
	PrintSrEvent(&(prSrEventSrCond->rSrEvent));
	PrintSrCond(&(prSrEventSrCond->rSrCond));
}

VOID PrintSrEventSrRcpiTbl(IN P_SR_EVENT_SR_RCPITBL_T prSrEventSrRcpiTbl)
{
	PrintSrEvent(&(prSrEventSrRcpiTbl->rSrEvent));
	PrintSrRcpiTbl(&(prSrEventSrRcpiTbl->rSrRcpiTbl));
}

VOID PrintSrEventSrRcpiTblOfst(IN P_SR_EVENT_SR_RCPITBL_OFST_T prSrEventSrRcpiTblOfst)
{
	PrintSrEvent(&(prSrEventSrRcpiTblOfst->rSrEvent));
	PrintSrRcpiTblOfst(&(prSrEventSrRcpiTblOfst->rSrRcpiTblOfst));
}

VOID PrintSrEventSrQCtrl(IN P_SR_EVENT_SR_Q_CTRL_T prSrEventSrQCtrl)
{
	PrintSrEvent(&(prSrEventSrQCtrl->rSrEvent));
	PrintSrQCtrl(&(prSrEventSrQCtrl->rSrQCtrl));
}

VOID PrintSrEventSrIBPD(IN P_SR_EVENT_SR_IBPD_T prSrEventSrIBPD)
{
	PrintSrEvent(&(prSrEventSrIBPD->rSrEvent));
	PrintSrIBPD(&(prSrEventSrIBPD->rSrIBPD));
}

VOID PrintSrEventSrNRT(IN P_SR_EVENT_SR_NRT_T prSrEventSrNRT)
{
	UINT_8 u1Index;

	PrintSrEvent(&(prSrEventSrNRT->rSrEvent));
	for (u1Index = 0; u1Index < SR_NRT_ROW_NUM; u1Index++)
		PrintSrNRT(&(prSrEventSrNRT->rSrNRT[u1Index]));
}

VOID PrintSrEventSrNRTCtrl(IN P_SR_EVENT_SR_NRT_CTRL_T prSrEventSrNRTCtrl)
{
	PrintSrEvent(&(prSrEventSrNRTCtrl->rSrEvent));
	PrintSrNRTCtrl(&(prSrEventSrNRTCtrl->rSrNRTCtrl));
}

VOID PrintSrEventSrCnt(IN P_SR_EVENT_SR_CNT_T prSrEventSrCnt)
{
	PrintSrEvent(&(prSrEventSrCnt->rSrEvent));
	PrintSrCnt(prSrEventSrCnt->rSrEvent.u1DbdcIdx, &(prSrEventSrCnt->rSrCnt));
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

UINT_8 SRRcpiConv(IN INT_8 i1Dbm)
{
	/*
	   dBm = (RCPI-220)/2
	   RCPI = (dBm * 2) + 220
		= (dBm * 2) + (110 * 2)
		= (dBm + 110) * 2
				  *dBm must be negative
	*/

	if (i1Dbm < SR_RCPI_MIN)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Err. i1Dbm:%d\n", __func__, i1Dbm));
		return (UINT_8)((SR_RCPI_MIN + 110) << 1);;
	}
	else if (i1Dbm > SR_RCPI_MAX)
	{
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Err. i1Dbm:%d\n", __func__, i1Dbm));
		return (UINT_8)((SR_RCPI_MAX + 110) << 1);;
	}
	else
	{
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
#endif				/* defined(MT7915) */
