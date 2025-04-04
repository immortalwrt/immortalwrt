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

    Module Name:
    ap_muru.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#ifndef __AP_MURU_H__
#define __AP_MURU_H__

#include "he_cfg.h"

enum {
	/* debug commands */
	MURU_SET_BSRP_CTRL = 1,
	MURU_SET_GLOBAL_PROT_SEC_CTRL = 2,
	MURU_SET_TX_DATA_SEC_CTRL = 3,
	MURU_SET_TRIG_DATA_SEC_CTRL = 4,
	MURU_SET_TRIG_SND_SEC_CTRL = 5,
	MURU_GET_BSRP_CTRL = 6,
	MURU_GET_GLOBAL_PROT_SEC_CTRL = 7,
	MURU_GET_TX_DATA_SEC_CTRL = 8,
	MURU_GET_TRIG_DATA_SEC_CTRL = 9,
	MURU_GET_TRIG_SND_SEC_CTRL = 10,
	MURU_GET_GLO_ADDR = 11,
	MURU_SET_HESND_CTRL = 12,
	MURU_GET_HESND_CTRL = 13,
	MURU_SET_ARB_OP_MODE = 14,
	MURU_SET_ALGO_DBG_CTRL = 15,
	MURU_SET_SUTX = 16,
	MURU_MU_MIMO_CTRL = 17,
	MURU_GET_LAST_SPL = 18,
	MURU_GET_SPL_CNT = 19,
	MURU_DO_SRAM_SYNC = 20,
	MURU_SET_GLO_ADDR = 21,
	MURU_SET_IGNORE_NAV = 22,
	MURU_CERT_SEND_FRAME_CTRL = 23,
	MURU_SHOW_ULRU_STATUS = 24,
	MURU_SET_PLATFORM_TYPE = 25,
	MURU_DBDC_EN_CTRL_WORKAROUND = 26,
	MURU_SET_DBG_INFO = 27,

	/* Configure commands */
	MURU_CFG_DLUL_LIMIT = 80,
	MURU_SET_DLUL_EN = 81,

	/* HQA AP commands offset 100 */
	MURU_SET_MANUAL_CONFIG = 100,
	MURU_SET_ULTX_TRIGGER = 101,
	MURU_SET_ULTX_CNT_RESET = 102,
	MURU_GET_ULTX_CNT = 103,
	MURU_SET_AGGPOLICY = 104,
	MURU_SET_MU_TX_PKT_CNT = 105,
	MURU_SET_MU_TX_PKT_EN = 106,

    /*TX Cmd TX Statistics*/
    MURU_SET_TXC_TX_STATS_EN = 150,
    MURU_GET_TXC_TX_STATS = 151,

	/* UI commands offset 200 */
	MURU_SET_MUDL_ACK_POLICY = 200,
	MURU_SET_TRIG_TYPE = 201,
	MURU_SET_20M_DYN_ALGO = 202,
	MURU_SET_DIS_CNT_TX = 203,
	MURU_SET_PROT_FRAME_THR = 204,
	MURU_SET_CERT_MU_EDCA_OVERRIDE = 205,
	MURU_GET_FW_BLACKLIST_CTRL = 206,
	MURU_SET_DRV_BLACKLIST_CTRL = 207,
	MURU_SET_FW_DUMP_MANCFG = 208,
	MURU_SET_MU_TYPE_SELECT = 209,
	MURU_SET_TXOP_ONOFF = 210,
	MURU_SET_UL_ONOFF = 211,
	MURU_SET_STATISTIC_CONFIG = 212,
	/* DVT commands offset 250 */
	MURU_VOW_DVT_CLONE_STA = 250,

	/* QoS CFG offset 254 */
	MURU_SET_QOS_CFG = 254,


/*MURU END*/
};

enum {
	MURU_EVENT_FGSUTX = 1,
	MURU_EVENT_GET_TONE_PLAN_POLICY = 2,
	MURU_EVENT_GET_BITRATE_FOR_ALLSTA = 3,
	MURU_EVENT_GET_PPDU_DUR = 4,
	MURU_EVENT_GET_FG_TRIGGER_FLOW = 5,
	MURU_EVENT_GET_FG_UL_MU_BA = 6,
	MURU_EVENT_GET_FG_UL_MU_CONFIG = 7,
	MURU_EVENT_GET_MAX_MU_NUM = 8,
	MURU_EVENT_GET_RU_ALGO_TIMEOUT = 9,
	MURU_EVENT_GET_SW_PDA_POLICY = 10,
	MURU_EVENT_GET_FG_UL_MU_SND = 11,
	MURU_EVENT_GET_FG_TXOP_EXP = 12,
	MURU_EVENT_GET_PROT_FRAME_THR = 13,
	MURU_EVENT_GET_BSRP_CTRL = 14,
	MURU_EVENT_GET_GLOBAL_PROT_SEC_CTRL = 15,
	MURU_EVENT_GET_TX_DATA_SEC_CTRL = 16,
	MURU_EVENT_GET_TRIG_DATA_SEC_CTRL = 17,
	MURU_EVENT_GET_TRIG_SND_SEC_CTRL = 18,
	MURU_EVENT_GET_HESND_CTRL = 19,
	MURU_EVENT_GET_MUM_CTRL = 20,
	MURU_EVENT_GET_ULTX_CNT = 21,
	MURU_EVENT_GET_SPL_CNT = 22,
	MURU_EVENT_GET_GLO_ADDR = 23,
	MURU_EVENT_TUNE_AP_MUEDCA = 24,
	MURU_EVENT_GET_TXC_TX_STATS = 25,
	MURU_EVENT_GET_FW_BLACKLIST_CTRL = 26,
    MURU_EVENT_GET_MURU_STATS_MODE_A = 27,
    MURU_EVENT_GET_MUMIMO_STATS_MODE_B = 28,
    MURU_EVENT_GET_DBG_STATS_MODE_C = 29,

};

enum {
	/* Debug Commands*/
	MU_MIMO_SET_ENABLE = 0,
	MU_MIMO_GET_ENABLE,
	MU_MIMO_SET_PROFILE_ENTRY,
	MU_MIMO_GET_PROFILE_ENTRY,
	MU_MIMO_SET_GROUP_TBL_ENTRY,
	MU_MIMO_GET_GROUP_TBL_ENTRY,
	MU_MIMO_SET_CLUSTER_TBL_ENTRY,
	MU_MIMO_GET_CLUSTER_TBL_ENTRY,
	MU_MIMO_GET_DL_AC_TABLE,
	MU_MIMO_GET_UL_TID_TABLE,
	MU_MIMO_SET_FIXED_RATE,
	MU_MIMO_SET_FIXED_GROUP_RATE,
	MU_MIMO_SET_FORCE_MU
};

enum {
	RU_IDX_996x2 = 68,
};


#ifdef CFG_SUPPORT_FALCON_MURU
/*MURU START*/
typedef struct _CMD_MURU_HESND_CTRL {
#ifdef RT_BIG_ENDIAN
	UINT_16		ucBrRuAlloc;/*rualloc*/
	UINT_8		ucTriggerFlow; /*0: normal, 1: kick-and-stop*/
	UINT_8		ucInterval; /*Timer@ms*/
	UINT_32     ucPpduDur;/*@us*/

#else
	UINT_32     ucPpduDur;/*@us*/
	UINT_8		ucInterval;/*@ms*/
	UINT_8      ucTriggerFlow; /*0: normal, 1: kick-and-stop*/
	UINT_16     ucBrRuAlloc; /*rualloc*/
#endif
} CMD_MURU_HESND_CTRL, *P_CMD_MURU_HESND_CTRL;


typedef struct _CMD_MURU_BSRP_CTRL {
#ifdef RT_BIG_ENDIAN
	UINT_8      fgExtCmdBsrp; /* TRUE: timer control by ext cmd */
	UINT_8      u1TriggerFlow; /*0: normal, 1: kick-and-stop, 2: stop*/
	UINT_32     u4TriggerType; /* 0:Basic 4:BSRP 6:BQRP */
	UINT_16     u2BsrpRuAlloc;
	UINT_16     u2BsrpInterval;
#else
	UINT_16     u2BsrpInterval;
	UINT_16     u2BsrpRuAlloc;
	UINT_32     u4TriggerType; /* 0:Basic 4:BSRP 6:BQRP */
	UINT_8      u1TriggerFlow; /*0: normal, 1: kick-and-stop, 2: stop*/
	UINT_8      fgExtCmdBsrp; /* TRUE: timer control by ext cmd */
#endif
} CMD_MURU_BSRP_CTRL, *P_CMD_MURU_BSRP_CTRL;



typedef struct _CMD_MURU_GLBOAL_PROT_SEC_CTRL {
#ifdef RT_BIG_ENDIAN
	UINT_8		ucTriggerFlow:1;
	UINT_8		ucTpPolicy:3;
	UINT_8		ucSuTx:1;
	UINT_8		ucFixedRate:1;
	UINT_8		ucProtRuAlloc:3;
	UINT_8		ucProt:2;
	UINT_8		ucPdc:3;/*PDA*/
	UINT_8		ucTxOp:1;
	UINT_8		ucExp:1;
#else
	UINT_8		ucExp:1;
	UINT_8		ucTxOp:1;
	UINT_8		ucPdc:3;/*PDA-> PDCtrl*/
	UINT_8		ucProt:2;
	UINT_8		ucProtRuAlloc:3;
	UINT_8		ucFixedRate:1;
	UINT_8		ucSuTx:1;
	UINT_8		ucTpPolicy:3;
	UINT_8		ucTriggerFlow:1;
#endif
} CMD_MURU_GLBOAL_PROT_SEC_CTRL, *P_CMD_MURU_GLBOAL_PROT_SEC_CTRL;

typedef struct _CMD_MURU_TX_DATA_SEC_CTRL {
#ifdef RT_BIG_ENDIAN
	UINT_32		ucRsv:15;
	UINT_32		ucMuPpduDur:14;
	UINT_32		ucBw:3;
#else
	UINT_32		ucBw:3;
	UINT_32		ucMuPpduDur:14;
	UINT_32		ucRsv:15;
#endif
} CMD_MURU_TX_DATA_SEC_CTRL, *P_CMD_MURU_TX_DATA_SEC_CTRL;

typedef struct _CMD_MURU_TRIG_DATA_SEC_CTRL {
#ifdef RT_BIG_ENDIAN
	UINT_32		ucRsv:13;
	UINT_32		ucGBAMuPpduDur:14;
	UINT_32		ucGBABw:3;
	UINT_32		ucBaPolicy:2;
#else
	UINT_32		ucBaPolicy:2;
	UINT_32		ucGBABw:3;
	UINT_32		ucGBAMuPpduDur:14;
	UINT_32		ucRsv:13;
#endif
} CMD_MURU_TRIG_DATA_SEC_CTRL, *P_CMD_MURU_TRIG_DATA_SEC_CTRL;

typedef struct _CMD_MURU_ALGO_DBG_CTRL {
	UINT_8      u1OpMode;
	UINT_8      u1Enable;
	UINT_16     u2Period;
} CMD_MURU_ALGO_DBG_CTRL, *P_CMD_MURU_ALGO_DBG_CTRL;

typedef struct _CMD_MURU_VOW_CLONE_STA {
	UINT_16     u2StaCnt;
	UINT_16     u2Rsv;
} CMD_MURU_VOW_CLONE_STA, *P_CMD_MURU_VOW_CLONE_STA;

typedef struct _CMD_MURU_SET_PROT_FRAME_THR {
	UINT32     u4ProtFrameThr;
} CMD_MURU_SET_PROT_FRAME_THR, *P_CMD_MURU_SET_PROT_FRAME_THR;

#ifdef DABS_QOS
typedef enum _MURU_QOS_CMD_T {
	QOS_CMD_PARAM_RESET = 0,
	QOS_CMD_ENABLE_DLY_POLICY = 1,
	QOS_CMD_PARAM_SETTING = 2,
	QOS_CMD_RESULT_DUMP = 3,
	QOS_CMD_FORCE_QID = 4,
	QOS_CMD_ENABLE_AC_SPL_HANDLER = 5,
	QOS_CMD_PARAM_DUMP = 6,
	QOS_CMD_DBGLOG_ON = 7,
	QOS_CMD_DBGLOG_OFF = 8,
	QOS_CMD_FORCE_AGGPOL_RU = 9,
	QOS_CMD_PARAM_DELETE = 10
} MURU_QOS_CMD;

typedef struct _MURU_QOS_SETTING {
	UINT_16		u2WlanIdx;
	UINT_8		u1AC;
	UINT_8		u1ForceAC;
	UINT_16		u2DelayBound;
	UINT_16		u2DelayReq;
	UINT_8		u1DelayWeight;
	UINT_16		u2DataRate;
	UINT_16		u2BWReq;
	UINT_8		u1Dir;
	UINT_16		u2DropThres;
	UINT_8		u1Idx;
	UINT_8		u1Reserved[1];
} MURU_QOS_SETTING, *P_MURU_QOS_SETTING;

typedef struct _CMD_MURU_QOS_CFG {
	UINT_32		u4OpFlag;
	MURU_QOS_SETTING	QoSSetting;
} CMD_MURU_QOS_CFG, *P_CMD_MURU_QOS_CFG;
#endif

typedef struct _CMD_MURU_SET_PLATFORM_TYPE {
	UINT_8       ucPlatformType;
	UINT_8       ucReserved[3];
} CMD_MURU_SET_PLATFORM_TYPE, *P_CMD_MURU_SET_PLATFORM_TYPE;

typedef struct _CMD_MURU_SET_TXOP_ONOFF {
	UINT_32     u4TxopOnOff;
} CMD_MURU_SET_TXOP_ONOFF, *P_CMD_MURU_SET_TXOP_ONOFF;

typedef struct _CMD_MURU_SET_UL_ONOFF {
	UINT_16     u2UlBsrpOnOff;
	UINT_16     u2UlDataOnOff;
} CMD_MURU_SET_UL_ONOFF, *P_CMD_MURU_SET_UL_ONOFF;

struct CMD_MURU_SET_DBG_INFO {
	UINT16   u2Item;
	UINT8    u1Reserved[2];
	UINT32   u4Value;
};

typedef struct _CMD_MURU_STAT_RECORD_CTRL {
    UINT_8 u1Mode;
    UINT_8 ucReserved;
    UINT_16 u2StartWcid;
    UINT_16 u2EndWcid;
    UINT_16 u2TimerInterval;
} CMD_MURU_STAT_RECORD_CTRL, *P_CMD_MURU_STAT_RECORD_CTRL;

typedef struct _EVENT_MURU_GET_HESND_CTRL {
	UINT32 u4EventId;
	UINT32 u4Index;
	CMD_MURU_HESND_CTRL rEntry;
} EVENT_MURU_GET_HESND_CTRL, *P_EVENT_MURU_GET_HESND_CTRL;

typedef struct _EVENT_MURU_GET_BSRP_CTRL {
	UINT32 u4EventId;
	UINT32 u4Index;
	CMD_MURU_BSRP_CTRL rEntry;
} EVENT_MURU_GET_BSRP_CTRL, *P_EVENT_MURU_GET_BSRP_CTRL;

typedef struct _EVENT_MURU_GET_GLOBAL_PROT_SEC_CTRL {
	UINT32 u4EventId;
	CMD_MURU_GLBOAL_PROT_SEC_CTRL rEntry;
} EVENT_MURU_GET_GLOBAL_PROT_SEC_CTRL, *P_EVENT_MURU_GET_GLOBAL_PROT_SEC_CTRL;

typedef struct _EVENT_MURU_GET_TX_DATA_SEC_CTRL {
	UINT32 u4EventId;
	CMD_MURU_TX_DATA_SEC_CTRL rEntry;
} EVENT_MURU_GET_TX_DATA_SEC_CTRL, *P_EVENT_MURU_GET_TX_DATA_SEC_CTRL;

typedef struct _EVENT_MURU_GET_TRIG_DATA_SEC_CTRL {
	UINT32 u4EventId;
	CMD_MURU_TRIG_DATA_SEC_CTRL rEntry;
} EVENT_MURU_GET_TRIG_DATA_SEC_CTRL, *P_EVENT_MURU_GET_TRIG_DATA_SEC_CTRL;

typedef struct _EVENT_MURU_GET_UL_TX_CNT {
	UINT32	u4EventId;
	UINT8	u1StaCnt;
	UINT32	u4TotSentPktCnt[MAX_NUM_TXCMD_USER_INFO+1];
	UINT32	u4TotOkCnt[MAX_NUM_TXCMD_USER_INFO+1];
} EVENT_MURU_GET_UL_TX_CNT, *P_EVENT_MURU_GET_UL_TX_CNT;

typedef struct _EVENT_MURU_GET_SPL_CNT {
	UINT32	u4EventId;
	UINT16	u2SplCnt[MAX_LEN_OF_MAC_TABLE];
} EVENT_MURU_GET_SPL_CNT, *P_EVENT_MURU_GET_SPL_CNT;

typedef struct _EVENT_SHOW_MURU_HESND_CTRL {
	UINT32	u4EventId;
	UINT32	u4Index;
#ifdef RT_BIG_ENDIAN
	UINT_32 ucPpduDur;/*@us*/
	UINT_16	ucBrRuAlloc;/*rualloc*/
	UINT_8	ucInterval; /*@ms*/
	UINT_8	ucTriggerFlow; /*0: normal, 1: kick-and-stop*/
#else
	UINT_8	ucTriggerFlow; /*0: normal, 1: kick-and-stop*/
	UINT_8	ucInterval; /*@ms*/
	UINT_16 ucBrRuAlloc;/*rualloc*/
	INT_32 ucPpduDur;/*@us*/
#endif
} EVENT_SHOW_MURU_HESND_CTRL, *P_EVENT_SHOW_MURU_HESND_CTRL;


typedef struct _EVENT_SHOW_MURU_BSRP_CTRL {
	UINT32	u4EventId;
	UINT32	u4Index;
#ifdef RT_BIG_ENDIAN
	UINT_8      fgExtCmdBsrp; /* TRUE: timer control by ext cmd */
	UINT_8      u1TriggerFlow; /*0: normal, 1: kick-and-stop, 2: stop*/
	UINT_32     u4TriggerType; /*@us*/
	UINT_16     u2BsrpRuAlloc;
	UINT_16     u2BsrpInterval;
#else
	UINT_16     u2BsrpInterval;
	UINT_16     u2BsrpRuAlloc;
	UINT_32     u4TriggerType; /*@us*/
	UINT_8      u1TriggerFlow; /*0: normal, 1: kick-and-stop, 2: stop*/
	UINT_8      fgExtCmdBsrp; /* TRUE: timer control by ext cmd */
#endif
} EVENT_SHOW_MURU_BSRP_CTRL, *P_EVENT_SHOW_MURU_BSRP_CTRL;

typedef struct _EVENT_SHOW_MURU_GLOBAL_PROT_SEC_CTRL {
	UINT32	u4EventId;
	UINT32	u4Index;
#ifdef RT_BIG_ENDIAN
		UINT_8		ucTriggerFlow:1;
		UINT_8		ucTpPolicy:3;
		UINT_8		ucSuTx:1;
		UINT_8		ucFixedRate:1;
		UINT_8		ucProtRuAlloc:3;
		UINT_8		ucProt:2;
		UINT_8		ucPdc:3;/*PDA*/
		UINT_8		ucTxOp:1;
		UINT_8		ucExp:1;
#else
		UINT_8		ucExp:1;
		UINT_8		ucTxOp:1;
		UINT_8		ucPdc:3;/*PDA-> PDCtrl*/
		UINT_8		ucProt:2;
		UINT_8		ucProtRuAlloc:3;
		UINT_8		ucFixedRate:1;
		UINT_8		ucSuTx:1;
		UINT_8		ucTpPolicy:3;
		UINT_8		ucTriggerFlow:1;
#endif
} EVENT_SHOW_MURU_GLOBAL_PROT_SEC_CTRL, *P_EVENT_SHOW_MURU_GLOBAL_PROT_SEC_CTRL;

typedef struct _EVENT_SHOW_MURU_TX_DATA_SEC_CTRL {
	UINT32	u4EventId;
	UINT32	u4Index;
#ifdef RT_BIG_ENDIAN
	UINT_32	ucRsv:15;
	UINT_32	ucMuPpduDur:14;
	UINT_32	ucBw:3;
#else
	UINT_32	ucBw:3;
	UINT_32	ucMuPpduDur:14;
	UINT_32	ucRsv:15;
#endif
} EVENT_SHOW_MURU_TX_DATA_SEC_CTRL, *P_EVENT_SHOW_MURU_TX_DATA_SEC_CTRL;

typedef struct _EVENT_SHOW_MURU_TRIG_DATA_SEC_CTRL {
	UINT32	u4EventId;
	UINT32	u4Index;
#ifdef RT_BIG_ENDIAN
	UINT_32	ucRsv:13;
	UINT_32	ucGBAMuPpduDur:14;
	UINT_32	ucGBABw:3;
	UINT_32	ucBaPolicy:2;
#else
	UINT_32	ucBaPolicy:2;
	UINT_32	ucGBABw:3;
	UINT_32	ucGBAMuPpduDur:14;
	UINT_32 ucRsv:13;
#endif
} EVENT_SHOW_MURU_TRIG_DATA_SEC_CTRL, *P_EVENT_SHOW_MURU_TRIG_DATA_SEC_CTRL;

typedef struct _MURU_GLO_CHECK {
	UINT_32 u4Addr;
	BOOLEAN fgError;
} MURU_GLO_CHECK, *P_MURU_GLO_CHECK;

typedef struct _DRV_MURU_GLO {
	MURU_GLO_CHECK rLocalData;
	MURU_GLO_CHECK rLocalDataMuruPara;
	MURU_GLO_CHECK rLocalDataQlenInfo;
	MURU_GLO_CHECK rLocalDataBsrpCtrl;
	MURU_GLO_CHECK rLocalDataTxCmdCtrl;

	MURU_GLO_CHECK rMuruTxInfo;
	MURU_GLO_CHECK rMuruTxInfoGlobalData;
	MURU_GLO_CHECK rMuruTxInfoProtectData;
	MURU_GLO_CHECK rMuruTxInfoSxnTxData;
	MURU_GLO_CHECK rMuruTxInfoSxnTrigData;

	MURU_GLO_CHECK rShareData;
	MURU_GLO_CHECK rShareDataRuAllocData;
	MURU_GLO_CHECK rShareDataUserInfo;
	MURU_GLO_CHECK rShareDataStaRuRecord;

	MURU_GLO_CHECK rMuruCmdManCfgInf;
	MURU_GLO_CHECK rMuTxPktCnt;
	MURU_GLO_CHECK rMuTxPktCntDwn;
	MURU_GLO_CHECK rAggPolicy;
	MURU_GLO_CHECK rDurationComp;

	MURU_GLO_CHECK rMuruMumGrpTable;
	MURU_GLO_CHECK rMuruMumCtrl;
	MURU_GLO_CHECK rMuruStaCapInfo;
	MURU_GLO_CHECK rMuruTxStatInfo;
	MURU_GLO_CHECK rCn4GidLookupTable;
} DRV_MURU_GLO, *P_DRV_MURU_GLO;

typedef struct _MURU_GLO_INFO {
	UINT_32 u4Addr;
	UINT_32 u4Size;
} MURU_GLO_INFO, *P_MURU_GLO_INFO;

typedef struct _EVENT_MURU_GLO {
	MURU_GLO_INFO rLocalData;
	MURU_GLO_INFO rLocalDataMuruPara;
	MURU_GLO_INFO rLocalDataQlenInfo;
	MURU_GLO_INFO rLocalDataBsrpCtrl;
	MURU_GLO_INFO rLocalDataTxCmdCtrl;

	MURU_GLO_INFO rMuruTxInfo;
	MURU_GLO_INFO rMuruTxInfoGlobalData;
	MURU_GLO_INFO rMuruTxInfoProtectData;
	MURU_GLO_INFO rMuruTxInfoSxnTxData;
	MURU_GLO_INFO rMuruTxInfoSxnTrigData;

	MURU_GLO_INFO rShareData;
	MURU_GLO_INFO rShareDataRuAllocData;
	MURU_GLO_INFO rShareDataUserInfo;
	MURU_GLO_INFO rShareDataStaRuRecord;

	MURU_GLO_INFO rMuruCmdManCfgInf;
	MURU_GLO_INFO rMuTxPktCnt;
	MURU_GLO_INFO rMuTxPktCntDwn;
	MURU_GLO_INFO rAggPolicy;
	MURU_GLO_INFO rDurationComp;

	MURU_GLO_INFO rMuruMumGrpTable;
	MURU_GLO_INFO rMuruMumCtrl;
	MURU_GLO_INFO rMuruStaCapInfo;
	MURU_GLO_INFO rMuruTxStatInfo;
	MURU_GLO_INFO rCn4GidLookupTable;
} EVENT_MURU_GLO, *P_EVENT_MURU_GLO;

typedef struct _MURU_TXCMD_DL_TX_STATS {
    UINT_32 u4TxCmdTxModeCckCnt;
    UINT_32 u4TxCmdTxModeOfdmCnt;
    UINT_32 u4TxCmdTxModeHtMmCnt;
    UINT_32 u4TxCmdTxModeHtGfCnt;
    UINT_32 u4TxCmdTxModeVhtSuCnt;
    UINT_32 u4TxCmdTxModeVht2MuCnt;
    UINT_32 u4TxCmdTxModeVht3MuCnt;
    UINT_32 u4TxCmdTxModeVht4MuCnt;
    UINT_32 u4TxCmdTxModeHeSuCnt;
    UINT_32 u4TxCmdTxModeHeExtSuCnt;
    UINT_32 u4TxCmdTxModeHeMu2RuCnt;
    UINT_32 u4TxCmdTxModeHeMu2MuCnt;
    UINT_32 u4TxCmdTxModeHeMu3RuCnt;
    UINT_32 u4TxCmdTxModeHeMu3MuCnt;
    UINT_32 u4TxCmdTxModeHeMu4RuCnt;
    UINT_32 u4TxCmdTxModeHeMu4MuCnt;
    UINT_32 u4TxCmdTxModeHeMu5to8RuCnt;
    UINT_32 u4TxCmdTxModeHeMu9to16RuCnt;
    UINT_32 u4TxCmdTxModeHeMuGtr16RuCnt;
} MURU_TXCMD_DL_TX_STATS, *P_MURU_TXCMD_DL_TX_STATS;

typedef struct _MURU_TXCMD_UL_TX_TRIG_STATS {
    UINT_32 u4TxCmdTxModeHeTrigSuCnt;
    UINT_32 u4TxCmdTxModeHeTrig2RuCnt;
    UINT_32 u4TxCmdTxModeHeTrig3RuCnt;
    UINT_32 u4TxCmdTxModeHeTrig4RuCnt;
    UINT_32 u4TxCmdTxModeHeTrig5to8RuCnt;
    UINT_32 u4TxCmdTxModeHeTrig9to16RuCnt;
    UINT_32 u4TxCmdTxModeHeTrigGtr16RuCnt;
    UINT_32 u4TxCmdTxModeHeTrig2MuCnt;
    UINT_32 u4TxCmdTxModeHeTrig3MuCnt;
    UINT_32 u4TxCmdTxModeHeTrig4MuCnt;
} MURU_TXCMD_UL_TX_TRIG_STATS, *P_MURU_TXCMD_UL_TX_TRIG_STATS;

typedef struct _EVENT_MURU_TXCMD_TX_STATS {
    UINT_32     u4EventId;
    MURU_TXCMD_DL_TX_STATS EventTxDlStats;
    MURU_TXCMD_UL_TX_TRIG_STATS EventTxTrigUlStats;
} EVENT_MURU_TXCMD_TX_STATS, *P_EVENT_MURU_TXCMD_TX_STATS;

typedef struct _EVENT_GET_MURU_GLO_ADDR {
	UINT_32 u4EventId;
	UINT_32 u4Index;
	EVENT_MURU_GLO rGloInfo;
} EVENT_GET_MURU_GLO_ADDR, *P_EVENT_GET_MURU_GLO_ADDR;

typedef struct _CMD_MURU_SET_GLOBAL_ADDR {
	UINT_32 u4Addr;
	UINT_32 u4Value;
} CMD_MURU_SET_GLOBAL_ADDR, *P_CMD_MURU_SET_GLOBAL_ADDR;

typedef struct _MUCOP_AC_TBL_T {
	UINT_16  u2WlanIdx;
	UINT_32  u4AC1;
	UINT_32  u4AC2;
	UINT_32  u4AC3;
	UINT_32  u4AC4;
} MUCOP_AC_TBL_T, *P_MUCOP_AC_TBL_T;

typedef struct _MUCOP_TID_TBL_T {
	UINT_16  u2WlanIdx;
	UINT_32  TID0;
	UINT_32  TID1;
	UINT_32  TID2;
	UINT_32  TID3;
	UINT_32  TID4;
	UINT_32  TID5;
	UINT_32  TID6;
	UINT_32  TID7;
} MUCOP_TID_TBL_T, *P_MUCOP_TID_TBL_T;

typedef struct _MUCOP_GROUP_TBL_ENTRY_T {
	UINT_32 u4EntryIdx;
	UINT_32 DW0;
	UINT_32 DW1;
	UINT_32 DW2;
	UINT_32 DW3;

	UINT_8 u1NumUser;
	UINT_8 u1Gi;
	UINT_8 u1Rsv1;
	UINT_8 u1Ax;
	UINT_8 u1PFIDUser0;
	UINT_8 u1PFIDUser1;
	UINT_8 u1PFIDUser2;
	UINT_8 u1PFIDUser3;
	UINT_8 u1Rsv2;
	UINT_8 u1DlVld;
	UINT_8 u1UlVld;

	UINT_8 u1RuAlloc;
	UINT_8 u1NssUser0;
	UINT_8 u1NssUser1;
	UINT_8 u1NssUser2;
	UINT_8 u1NssUser3;
	UINT_16 u2Rsv3;

	UINT_8 u1DlMcsUser0;
	UINT_8 u1DlMcsUser1;
	UINT_8 u1DlMcsUser2;
	UINT_8 u1DlMcsUser3;
	UINT_8 u1DlWfUser0;
	UINT_8 u1DlWfUser1;
	UINT_8 u1DlWfUser2;
	UINT_8 u1DlWfUser3;

	UINT_8 u1UlMcsUser0;
	UINT_8 u1UlMcsUser1;
	UINT_8 u1UlMcsUser2;
	UINT_8 u1UlMcsUser3;
	UINT_8 u1UlWfUser0;
	UINT_8 u1UlWfUser1;
	UINT_8 u1UlWfUser2;
	UINT_8 u1UlWfUser3;
} MUCOP_GROUP_TBL_ENTRY_T, *P_MUCOP_GROUP_TBL_ENTRY_T;

typedef struct _MUCOP_SELECT_OUTPUT_T {
	UINT_32 u4Score;        /*DW0*/
	UINT_8  u1Ac0;
	UINT_8  u1Ac1;
	UINT_8  u1Ac2;
	UINT_8  u1Ac3;
	UINT_16 u2GrpEntryIdx;  /*DW1*/
	UINT_8  u1UPUser0;
	UINT_8  u1UPUser1;
	UINT_8  u1UPUser2;
	UINT_8  u1UPUser3;
	UINT_8  u1GrpId;
	BOOL    fgRsv;
	BOOL    fgDone;
} MUCOP_SELECT_OUTPUT_T, *P_MUCOP_SELECT_OUTPUT_T;

typedef struct _MUCOP_CLUSTER_TBL_T {
	UINT_32    au4GIDUsrMemberStatus_DW0;
	UINT_32    au4GIDUsrMemberStatus_DW1;
	UINT_32    au4GIDUsrPosition[4];
} MUCOP_CLUSTER_TBL_T, *P_MUCOP_CLUSTER_TBL_T;

typedef union _MUCOP_TABLE_DISPLAY {
	MUCOP_GROUP_TBL_ENTRY_T GroupTbl;
	MUCOP_CLUSTER_TBL_T ClusterTbl;
	MUCOP_AC_TBL_T AcTbl;
	MUCOP_TID_TBL_T TidTbl;
	UINT_16 WlanIdx;
} MUCOP_TABLE_DISPLAY;

typedef struct _EVENT_MURU_GET_MUM_CTRL {
	UINT32 u4EventId;
	UINT8 u1SubType;
	MUCOP_TABLE_DISPLAY rEntry;
} EVENT_MURU_GET_MUM_CTRL, *P_EVENT_MURU_GET_MUM_CTRL;

enum {
	TX_MODE_VHT = 0,
	TX_MODE_HE
};

#define MURU_MUM_MAX_PFID_NUM           8
#define MAX_CAP_MUM_GRP_BLOCK           5
#define RAM_BAND_NUM                    2
#define STA_REC_NUM					288

#define MURU_PPDU_HE_SU					BIT(0)
#define MURU_PPDU_HE_EXT_SU				BIT(1)
#define MURU_PPDU_HE_TRIG				BIT(2)
#define MURU_PPDU_HE_MU					BIT(3)

#define MURU_OFDMA_SCH_TYPE_DL			BIT(0)
#define MURU_OFDMA_SCH_TYPE_UL			BIT(1)

/* Common Config */
#define MURU_FIXED_CMM_PPDU_FMT			BIT(0)
#define MURU_FIXED_CMM_SCH_TYPE			BIT(1)
#define MURU_FIXED_CMM_BAND				BIT(2)
#define MURU_FIXED_CMM_WMM_SET			BIT(3)
#define MURU_FIXED_CMM_SPE_IDX			BIT(4)
#define MURU_FIXED_CMM_PROC_TYPE		BIT(5)

/* DL Config */
#define MURU_FIXED_BW					BIT(0)
#define MURU_FIXED_GI					BIT(1)
#define MURU_FIXED_TX_MODE				BIT(2)
#define MURU_FIXED_TONE_PLAN			BIT(3)
#define MURU_FIXED_TOTAL_USER_CNT		BIT(4)
#define MURU_FIXED_LTF					BIT(5)
#define MURU_FIXED_SIGB_MCS				BIT(6)
#define MURU_FIXED_SIGB_DCM				BIT(7)
#define MURU_FIXED_SIGB_CMPRS			BIT(8)
#define MURU_FIXED_ACK_PLY				BIT(9)
#define MURU_FIXED_TXPOWER				BIT(10)

/* DL Per User Config */
#define MURU_FIXED_USER_WLAN_ID			BIT(16)
#define MURU_FIXED_USER_COD				BIT(17)
#define MURU_FIXED_USER_MCS				BIT(18)
#define MURU_FIXED_USER_NSS				BIT(19)
#define MURU_FIXED_USER_RU_ALLOC		BIT(20)
#define MURU_FIXED_USER_MUMIMO_GRP		BIT(21)
#define MURU_FIXED_USER_MUMIMO_VHT		BIT(22)
#define MURU_FIXED_USER_ACK_POLICY		BIT(23)
#define MURU_FIXED_USER_MUMIMO_HE		BIT(24)
#define MURU_FIXED_USER_PWR_ALPHA		BIT(25)

/* UL Common Config */
#define MURU_FIXED_TRIG_TYPE			BIT(0)
#define MURU_FIXED_TRIG_CNT				BIT(1)
#define MURU_FIXED_TRIG_INTV			BIT(2)
#define MURU_FIXED_TRIG_PKT_SIZE		BIT(3)
#define MURU_FIXED_UL_TOTAL_USER_CNT	BIT(4)
#define MURU_FIXED_UL_BW				BIT(5)
#define MURU_FIXED_UL_GILTF				BIT(6)
#define MURU_FIXED_UL_LENGTH			BIT(7)
#define MURU_FIXED_UL_TF_PAD			BIT(8)
#define MURU_FIXED_UL_ACK_TYPE			BIT(9)
/* HE TB RX Debug */
#define MURU_FIXED_RX_HETB_CFG1			BIT(10)
#define MURU_FIXED_RX_HETB_CFG2			BIT(11)
#define MURU_FIXED_NONSF_EN_BITMAP		BIT(12)
#define MURU_FIXED_TRIG_TA				BIT(13)

/* UL Per User Config */
#define MURU_FIXED_USER_UL_WLAN_ID		BIT(16)
#define MURU_FIXED_USER_UL_TARGET_RSSI	BIT(17)
#define MURU_FIXED_USER_UL_COD			BIT(18)
#define MURU_FIXED_USER_UL_MCS			BIT(19)
#define MURU_FIXED_USER_UL_NSS			BIT(20)
#define MURU_FIXED_USER_UL_RU_ALLOC		BIT(21)

#define MURU_MANUAL_CFG_CHK(_value, _mask) (((_value & (_mask)) == _mask) ? TRUE : FALSE)

typedef enum _ENUM_MURU_CMD_DL_ACK_PLY_T {
    MURU_CMD_MUDL_ACK_POLICY_MU_BAR = 3,
    MURU_CMD_MUDL_ACK_POLICY_TF_FOR_ACK = 4,
    MURU_CMD_MUDL_ACK_POLICY_SU_BAR = 5,
} ENUM_MURU_CMD_DL_ACK_PLY_T;

typedef enum _ENUM_MURU_ACK_PLY_T {
    ACK_POLICY_NORMAL_ACK_IMPLICIT_BA_REQ,
    ACK_POLICY_NO_ACK,
    ACK_POLICY_NO_EXPLICIT_ACK_PSMP_ACK,
    ACK_POLICY_BA
} ENUM_MURU_ACK_PLY_T;

typedef struct _MURU_DL_USER_INFO {
	UINT16		u2WlanIdx;
	UINT8		u1RuAllocBn;
	UINT8		u1RuAllocIdx;
	UINT8		u1Ldpc;
	UINT8		u1Nss;
	UINT8		u1Mcs;
	UINT8		u1MuGroupIdx;
	UINT8		u1VhtGid;
	UINT8		u1VhtUp;
	UINT8		u1HeStartStream;
	UINT8		u1HeMuMimoSpatial;
	UINT8		u1AckPolicy;
	UINT16		u2TxPwrAlpha;
}  MURU_DL_USER_INFO, *P_MURU_DL_USER_INFO;

typedef struct _MURU_DL_MANUAL_CONFIG {
	UINT8		u1UserCnt;
	UINT8		u1TxMode;
	UINT8		u1Bw;
	UINT8		u1GI;
	UINT8		u1Ltf;
	UINT8		u1SigBMcs;
	UINT8		u1SigBDcm;
	UINT8		u1SigBCmprs;
	UINT8		u1TxPwr;
	UINT8		au1RU[8];
	UINT8		au1C26[2];
	UINT8		u1AckPly;
	MURU_DL_USER_INFO	arUserInfoDl[MAX_NUM_TXCMD_USER_INFO];
} MURU_DL_MANUAL_CONFIG, *P_MURU_DL_MANUAL_CONFIG;

typedef struct _MURU_UL_USER_INFO {
	UINT16		u2WlanIdx;
	UINT8		u1RuAllocBn;
	UINT8		u1RuAllocIdx;
	UINT8		u1Ldpc;
	UINT8		u1Nss;
	UINT8		u1Mcs;
	UINT8		u1TargetRssi;
	UINT32		u4TrigPktSize;
} MURU_UL_USER_INFO, *P_MURU_UL_USER_INFO;

typedef struct _MURU_UL_MANUAL_CONFIG {
	UINT8		u1UserCnt;
	/* ULTX */
	UINT8		u1TrigType;
	UINT16		u2TrigCnt;
	UINT16		u2TrigIntv;
	UINT8		u1UlBw;
	UINT8		u1UlGiLtf;
	UINT16		u2UlLength;
	UINT8		u1TfPad;
	UINT8		u1TrigTa[MAC_ADDR_LEN];
	UINT8		au1UlRU[8];
	UINT8		au1UlC26[2];
	MURU_UL_USER_INFO	arUserInfoUl[MAX_NUM_TXCMD_USER_INFO];
	/* HE TB RX Debug */
	UINT32      rx_hetb_nonsf_en_bitmap;
	UINT32      rx_hetb_cfg[2];

	/* DLTX */
	UINT8		u1BaType;

} MURU_UL_MANUAL_CONFIG, *P_MURU_UL_MANUAL_CONFIG;

typedef struct _MURU_CMM_MANUAL_CONFIG {
	UINT8		u1PpduFmt;
	UINT8		u1SchType;
	UINT8		u1Band;
	UINT8		u1WmmSet;
	UINT8		u1SpeIdx;
	UINT8		u1ProcType;
} MURU_CMM_MANUAL_CONFIG, *P_MURU_CMM_MANUAL_CONFIG;

typedef struct _CMD_MURU_MANCFG_INTERFACER {
	UINT32		u4ManCfgBmpCmm;
	UINT32		u4ManCfgBmpDl;
	UINT32		u4ManCfgBmpUl;
	MURU_CMM_MANUAL_CONFIG		rCfgCmm;
	MURU_DL_MANUAL_CONFIG		rCfgDl;
	MURU_UL_MANUAL_CONFIG		rCfgUl;
} CMD_MURU_MANCFG_INTERFACER, *P_CMD_MURU_MANCFG_INTERFACER;

typedef struct _CMD_MURU_SET_MU_TX_PKT_CNT {
	UINT_8 u1BandIdx;
	UINT_8 u1MuTxEn;
	UINT_8 u1Rsv[2];
	UINT_32 u4MuTxPktCnt; /* 0: Continueous Tx, Others: Limited Tx */
} CMD_MURU_SET_MU_TX_PKT_CNT, *P_CMD_MURU_SET_MU_TX_PKT_CNT;

typedef struct _CMD_MURU_CERT_SEND_FRAME_CTRL {
	UINT32     u4PpduDur; /*@us*/
	UINT16     u2TargetWcid;
	UINT8      u1Interval; /*@ms*/
} CMD_MURU_CERT_SEND_FRAME_CTRL, *P_CMD_MURU_CERT_SEND_FRAME_CTRL;

typedef struct _EVENT_GET_MURU_FW_BLACKLIST_CTRL {
	UINT_32 u4EventId;
	UINT_8  u1FwBlackListDlOfdmaTestFailCnt;
	UINT_8  u1FwBlackListUlOfdmaTestFailCnt;
	UINT_8  u1Rsv[2];
} EVENT_GET_MURU_FW_BLACKLIST_CTRL, *P_EVENT_GET_MURU_FW_BLACKLIST_CTRL;

typedef struct _CMD_SET_MURU_DRV_BLACKLIST_CTRL {
	UINT_16 u2WlanId;
	BOOLEAN fgDrvBlackListDlOfdmaDisable;
	BOOLEAN fgDrvBlackListUlOfdmaDisable;
	UINT_8  u1Rsv[4];
} CMD_SET_MURU_DRV_BLACKLIST_CTRL, *P_CMD_SET_MURU_DRV_BLACKLIST_CTRL;

#define MURU_DRV_BLACK_LIST_DL_OFDMA_DISABLE	BIT(0)
#define MURU_DRV_BLACK_LIST_UL_OFDMA_DISABLE	BIT(1)

typedef union MURU_MUM_GROUP_TBL_ENTRY_T {
	struct {
		/* DW0 */
		UINT_32 u1NumUser       : 2,
		u1DlGi          : 2,
		u1UlGi          : 2,
		u1Rsv1          : 2,
		u1Ax            : 1,
		u1PFIDUser0     : 5,
		u1PFIDUser1     : 5,
		u1PFIDUser2     : 5,
		u1PFIDUser3     : 5,
		u1Rsv2          : 1,
		u1DlVld         : 1,
		u1UlVld         : 1;
		/* DW1 */
		UINT_32 u1RuAlloc       : 8,
		u1NssUser0      : 2,
		u1NssUser1      : 2,
		u1NssUser2      : 2,
		u1NssUser3      : 2,
		u2Rsv3          : 16;
		/* DW2 */
		UINT_32 u1DlMcsUser0    : 4,
		u1DlMcsUser1    : 4,
		u1DlMcsUser2    : 4,
		u1DlMcsUser3    : 4,
		u1DlWfUser0     : 4,
		u1DlWfUser1     : 4,
		u1DlWfUser2     : 4,
		u1DlWfUser3     : 4;
		/* DW3 */
		UINT_32 u1UlMcsUser0    : 4,
		u1UlMcsUser1    : 4,
		u1UlMcsUser2    : 4,
		u1UlMcsUser3    : 4,
		u1UlWfUser0     : 4,
		u1UlWfUser1     : 4,
		u1UlWfUser2     : 4,
		u1UlWfUser3     : 4;
	} rField;
	UINT_32 au4RawData[4];
} MURU_MUM_GROUP_TBL_ENTRY_T, *P_MURU_MUM_GROUP_TBL_ENTRY_T;
typedef enum _ENUM_MU_GRP_USR_CAP_T {
	MU_GRP_USR_VHT_CAP = 0x0,
	MU_GRP_USR_HE_DLFUMUM_CAP = 0x1,
	MU_GRP_USR_HE_DLPBMUM_CAP = 0x2,
	MU_GRP_USR_HE_ULFBMUM_CAP = 0x3,
	MU_GRP_USR_HE_ULPBMUM_CAP = 0x4,
	MU_MAX_GRP_USR_CAP        = 0x5
} ENUM_MU_GRP_USR_CAP_T, *P_ENUM_MU_GRP_USR_CAP_T;

typedef enum _ENUM_MURU_MUM_T {
	MURU_MUM_IDLE = 0,
	MURU_MUM_ENGROUP = 1,
	MURU_MUM_REGROUP = 2,
	MURU_MUM_DEGROUP = 3,
	MURU_MUM_DEAUTH = 4
} ENUM_MURU_MUM_T, *P_ENUM_MURU_MUM_T;

typedef struct _LINK_K {
	UINT_32 prNext;			/* Set Host 8B pointer to 4B */
	UINT_32 prPrev;			/* Set Host 8B pointer to 4B */
	UINT_32 u4NumElem;
} LINK_K, *P_LINK_K;

typedef enum _ENUM_MUM_GRP_USR_NUM_T {
	MUM_GRP_USR_NUM_1 = 0x0,
	MUM_GRP_USR_NUM_2 = 0x1,
	MUM_GRP_USR_NUM_3 = 0x2,
	MUM_GRP_USR_NUM_4 = 0x3,
	MUM_GRP_USR_MAX_NUM = 0x4
} ENUM_MUM_GRP_USR_NUM_T, *P_ENUM_MUM_GRP_USR_NUM_T;

typedef enum _ENUM_MUM_GRP_CN_T {
	MUM_GRP_CN_2 = 0x0,
	MUM_GRP_CN_3 = 0x1,
	MUM_GRP_CN_4 = 0x2
} ENUM_MUM_GRP_CN_T, *P_ENUM_MUM_GRP_CN_T;

typedef struct _MURU_MUM_GRP_BITMAP_T {
	/* Use MUM_GRP_USR_NUM for CN2, CN3, CN4 */
	UINT_8 au1GrpBitmap[MUM_GRP_USR_NUM_4][MAX_CAP_MUM_GRP_BLOCK];
	UINT_8 au1GrpNum[MUM_GRP_USR_NUM_4];
} MURU_MUM_GRP_BITMAP_T, *P_MURU_MUM_GRP_BITMAP_T;


typedef struct _MURU_MUM_PFID_GRP_BITMAP_T {
	/* Use MUM_GRP_USR_NUM for CN2, CN3, CN4. Assignment: CN2:[0]/CN3[1]/CN4[2] */
	UINT_8 au1PfidGrpBitmap[MUM_GRP_USR_NUM_4][MAX_CAP_MUM_GRP_BLOCK];
} MURU_MUM_PFID_GRP_BITMAP_T, *P_MURU_MUM_PFID_GRP_BITMAP_T;

typedef struct _MURU_MUM_USER_MGMT_T {
	UINT_16 u2MumUserGrpCnt;
} MURU_MUM_USER_MGMT_T, *P_MURU_MUM_USER_MGMT_T;

typedef struct _MURU_MUM_CTRL_PARA_T {
	ENUM_MURU_MUM_T eMumState;
	ENUM_MU_GRP_USR_CAP_T eCap;
	UINT_16 u2CurGrpIndex;
	UINT_8  u1PfidIdx;
	UINT_8  u1ClusterIdx;
	UINT_8  u1MumPfidNum;
	UINT_8  u1CurUsrCapBitmap;

	/* Mum List Link */
	LINK_K  MumUserCapList[MU_MAX_GRP_USR_CAP][RAM_BAND_NUM];
	LINK_K  MumUserGrpList[MU_MAX_GRP_USR_CAP][RAM_BAND_NUM];

	/* MUM cap Pfid bitmap that shared by all VHT/HE DL/UL FB/PB*/
	/* Band0 PFID bitmap=> 1: sta active, 0: sta inactive */
	UINT_32 au4B0PfIdBitMap[MU_MAX_GRP_USR_CAP];
	UINT_32 au4B1PfIdBitMap[MU_MAX_GRP_USR_CAP];

	/* Used for  HW PFID assignment */
	UINT_32 u4PfidBitmap;

	/* Used for  HW Group assignment */
	MURU_MUM_GRP_BITMAP_T arMuGroupBitmap;

	/* Used for Group Statistics by mum number and UserCap */
	MURU_MUM_GRP_BITMAP_T arGroupBitmap[MU_MAX_GRP_USR_CAP][RAM_BAND_NUM];

	/* for compress Gid managment - Gid: 1~62, Cluster number is equal to Pfid */
	UINT_8 au1GidPfidUp[62][MURU_MUM_MAX_PFID_NUM];

	/* for VHT MUMIMO Gid action frame TX only */
	UINT_8 au4MuProfileToClusterIdx[MURU_MUM_MAX_PFID_NUM];
	BOOL   afgVhtGidTx[MURU_MUM_MAX_PFID_NUM];

	UINT_16 au2MuProfileIdxToWlanIdx[MURU_MUM_MAX_PFID_NUM];
	UINT_8 au1MuWlanIdxToProfileIdx[STA_REC_NUM];
	MURU_MUM_USER_MGMT_T arMuUserMgmt[MURU_MUM_MAX_PFID_NUM][RAM_BAND_NUM];

	BOOL fgDeauthInProgress;

	/* for Get group bitmap by mum number and multiple Pfid */
	BOOL fgMuMimoFixRate;
	UINT_16 u2LatestMuTxGrpIdx;
	MURU_MUM_PFID_GRP_BITMAP_T arB0PfidGrpBitmap[MURU_MUM_MAX_PFID_NUM][MU_MAX_GRP_USR_CAP];
	MURU_MUM_PFID_GRP_BITMAP_T arB1PfidGrpBitmap[MURU_MUM_MAX_PFID_NUM][MU_MAX_GRP_USR_CAP];
} MURU_MUM_CTRL_PARA_T, *P_MURU_MUM_CTRL_PARA_T;

typedef struct _CMD_MURU_MUM_SET_GROUP_TBL_ENTRY {
#ifdef RT_BIG_ENDIAN
	UINT_8		 u1Ns3:1;
	UINT_8		 u1Ns2:1;
	UINT_8		 u1Ns1:1;
	UINT_8		 u1Ns0:1;
	UINT_8		 u1Res:2;
	UINT_8		 u1NumUser:2;
#else
	UINT_8		 u1NumUser:2;
	UINT_8		 u1Res:2;
	UINT_8		 u1Ns0:1;
	UINT_8		 u1Ns1:1;
	UINT_8		 u1Ns2:1;
	UINT_8		 u1Ns3:1;
#endif

	UINT_16       u2WlidUser0; /* WLANID0 */
	UINT_16       u2WlidUser1; /* WLANID1 */
	UINT_16       u2WlidUser2; /* WLANID2 */
	UINT_16       u2WlidUser3; /* WLANID3 */
#ifdef RT_BIG_ENDIAN
	UINT_8		 u1DlMcsUser1:4;
	UINT_8		 u1DlMcsUser0:4;
#else
	UINT_8		 u1DlMcsUser0:4;
	UINT_8		 u1DlMcsUser1:4;
#endif

#ifdef RT_BIG_ENDIAN
	UINT_8		 u1DlMcsUser3:4;
	UINT_8		 u1DlMcsUser2:4;
#else
	UINT_8		 u1DlMcsUser2:4;
	UINT_8		 u1DlMcsUser3:4;
#endif
#ifdef RT_BIG_ENDIAN
	UINT_8		 u1UlMcsUser1:4;
	UINT_8		 u1UlMcsUser0:4;
#else
	UINT_8		 u1UlMcsUser0:4;
	UINT_8		 u1UlMcsUser1:4;
#endif

#ifdef RT_BIG_ENDIAN
	UINT_8		 u1UlMcsUser3:4;
	UINT_8		 u1UlMcsUser2:4;
#else
	UINT_8		 u1UlMcsUser2:4;
	UINT_8		 u1UlMcsUser3:4;
#endif

	UINT_8		u1RuAlloc;
	UINT_8		u1Capability;
	UINT_8		u1GI;
	UINT_8		u1Dl_Ul;
} CMD_MURU_MUM_SET_GROUP_TBL_ENTRY, *P_CMD_MURU_MUM_SET_GROUP_TBL_ENTRY;

typedef union MURU_MUM_GROUP_TBL_ENTRY_DW0 {
	struct {
		/* DW0 */
		UINT_32 u1NumUser       : 2,
		u1DlGi          : 2,
		u1UlGi          : 2,
		u1Rsv1          : 2,
		u1Ax            : 1,
		u1PFIDUser0     : 5,
		u1PFIDUser1     : 5,
		u1PFIDUser2     : 5,
		u1PFIDUser3     : 5,
		u1Rsv2          : 1,
		u1DlVld         : 1,
		u1UlVld         : 1;
	} rField;
	UINT_32 u4RawData;
} MURU_MUM_GROUP_TBL_ENTRY_DW0, *P_MURU_MUM_GROUP_TBL_ENTRY_DW0;

typedef union MURU_MUM_GROUP_TBL_ENTRY_DW1 {
	struct {
		/* DW1 */
		UINT_32 u1RuAlloc       : 8,
		u1NssUser0      : 2,
		u1NssUser1      : 2,
		u1NssUser2      : 2,
		u1NssUser3      : 2,
		u2Rsv3          : 16;
	} rField;
	UINT_32 u4RawData;
} MURU_MUM_GROUP_TBL_ENTRY_DW1, *P_MURU_MUM_GROUP_TBL_ENTRY_DW1;

typedef union MURU_MUM_GROUP_TBL_ENTRY_DW2 {
	struct {
		/* DW2 */
		UINT_32 u1DlMcsUser0    : 4,
		u1DlMcsUser1    : 4,
		u1DlMcsUser2    : 4,
		u1DlMcsUser3    : 4,
		u1DlWfUser0     : 4,
		u1DlWfUser1     : 4,
		u1DlWfUser2     : 4,
		u1DlWfUser3     : 4;
	} rField;
	UINT_32 u4RawData;
} MURU_MUM_GROUP_TBL_ENTRY_DW2, *P_MURU_MUM_GROUP_TBL_ENTRY_DW2;

typedef union MURU_MUM_GROUP_TBL_ENTRY_DW3 {
	struct {
		/* DW3 */
		UINT_32 u1UlMcsUser0    : 4,
		u1UlMcsUser1    : 4,
		u1UlMcsUser2    : 4,
		u1UlMcsUser3    : 4,
		u1UlWfUser0     : 4,
		u1UlWfUser1     : 4,
		u1UlWfUser2     : 4,
		u1UlWfUser3     : 4;
	} rField;
	UINT_32 u4RawData;
} MURU_MUM_GROUP_TBL_ENTRY_DW3, *P_MURU_MUM_GROUP_TBL_ENTRY_DW3;

typedef struct _CMD_MURU_CMM_DLUL_CFG {
    UINT8   u1BandIdx;
    UINT8   u1Dis160RuMu;
    UINT8   u1MaxRuOfdma;
    UINT8   u1MaxDLMuMimo;
    UINT8   u1MaxULMuMimo;
    UINT8   au1Reserved[3];
} CMD_MURU_CMM_DLUL_CFG, *P_CMD_MURU_CMM_DLUL_CFG;

enum {
	/* Set MURU DLUL Command, select Band or BSS */
	MURU_SET_DLUL_BY_BAND = 0,
	MURU_SET_DLUL_BY_BSS = 1,
	MURU_SET_DLUL_READ = 2,
	MURU_SET_DLUL_MAX
};

enum {
	MURU_CFG_DL_OFDMA_BIT = 0,
	MURU_CFG_UL_OFDMA_BIT = 1,
	MURU_CFG_DL_MIMO_BIT = 2,
	MURU_CFG_UL_MIMO_BIT = 3,
	MURU_CFG_DLUL_OFDMA_MIMO_MAX
};

typedef struct _CMD_MURU_SET_DLUL_VAL {
	UINT_8  u1BandBssSelect;
	UINT_8  u1Index;
	UINT_8  u1DlUlUpdList;
	UINT_8  u1DlUlVal;
} CMD_MURU_SET_DLUL_VAL, *P_CMD_MURU_SET_DLUL_VAL;

INT SetMuruHeSndCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuruMuEdcaParam(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruBsrpCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruGlobalProtSecCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruTxDataSecCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruTrigDataSecCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruArbOpMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruAlgoDbgCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruVowCloneSta(RTMP_ADAPTER *pAd, UINT16 sta_cnt);
INT SetMuruSuTx(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruTxcTxStats(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruFixedRate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruData(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuru20MDynAlgo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruProtFrameThr(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruDbdcEnCtrlWorkaround(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruPlatformTypeProc(RTMP_ADAPTER *pAd);
INT SetMuruCfgDlUlVal(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuOfdmaDlEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuOfdmaUlEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuMimoDlEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuMimoUlEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_muru_debug_flow_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT MtCmdSetMuruCfgDlUlVal(RTMP_ADAPTER *pAd, UINT_8 u1BandBssSelect, UINT_8 u1Index, UINT_8 DlUlUpdList, UINT_8 DlUlVal);
INT SetMuruTxopOnOff(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruUlOnOff(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruTypeSelect(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuruStatisticConfig(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT ShowMuruHeSndCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuruTxcTxStats(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuruBsrpCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuruGlobalProtSecCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuruTxDataSecCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuruTrigDataSecCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuruMumCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuruSplCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuruGloAddr(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuruUlRuStatus(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuruLocalData(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuruTxInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuruSharedData(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuruManCfgData(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuruStaCapInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 hqa_muru_reset_ul_tx_cnt(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_muru_set_dl_tx_muru_config(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_muru_set_ul_tx_muru_config(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_muru_set_ul_tx_trigger(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_muru_get_ul_tx_cnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 hqa_muru_set_agg_policy(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT32 hqa_muru_set_mu_tx_pkt_en(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 hqa_muru_set_mu_tx_pkt_cnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 set_muru_mu_tx_pkt_en(RTMP_ADAPTER *pAd, P_CMD_MURU_SET_MU_TX_PKT_CNT prSetMuTxPktStart);
INT32 set_muru_mu_tx_pkt_cnt(RTMP_ADAPTER *pAd, P_CMD_MURU_SET_MU_TX_PKT_CNT prSetMuTxPktCnt);
INT32 set_muru_mudl_ack_policy(RTMP_ADAPTER *ad, UINT8 policy_num);
INT32 set_muru_trig_type(RTMP_ADAPTER *ad, UINT8 type);
INT32 set_muru_ignore_nav(RTMP_ADAPTER *ad, UINT8 ignore);
INT32 set_muru_cert_send_frame_ctrl(RTMP_ADAPTER *ad, UINT32 ppdu_dur, UINT16 target_wcid, UINT8  interval);
INT32 set_muru_cert_muedca_override(RTMP_ADAPTER *ad, UINT8 capi_override);

INT32 set_muru_manual_config(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 set_muru_debug_info(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 set_disable_contention_tx(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 wifi_test_muru_set_manual_config(PRTMP_ADAPTER pAd, P_CMD_MURU_MANCFG_INTERFACER pMuruManCfg);
INT32 wifi_test_muru_ul_tx_trigger(PRTMP_ADAPTER pAd, BOOLEAN IsUlTxTrigger);
INT32 wifi_test_muru_set_arb_op_mode(PRTMP_ADAPTER pAd, UINT8 arbOpMode);

VOID muru_tam_arb_op_mode(PRTMP_ADAPTER pAd);
VOID muru_update_he_cfg(PRTMP_ADAPTER pAd);
INT32 muru_cfg_dlul_limits(PRTMP_ADAPTER pAd, UINT8 u1BandIdx);

INT32 ShowMuruLastSplByQid(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID muru_tune_ap_muedca_handler(PRTMP_ADAPTER pAd,	char *rsp_payload, UINT16 rsp_payload_len);
#ifdef DABS_QOS
INT SetMuruQoSCfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
BOOLEAN SendQoSCmd(RTMP_ADAPTER *pAd, UINT32 op_flag, MURU_QOS_SETTING *pqos_setting);
#endif

INT get_muru_fw_black_list_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_muru_drv_black_list_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID muru_statistic_handler(PRTMP_ADAPTER pAd, char *rsp_payload, UINT16 rsp_payload_len);
VOID muru_mimo_stat_handler(PRTMP_ADAPTER pAd, char *rsp_payload, UINT16 rsp_payload_len);
VOID muru_dbg_stat_handler(PRTMP_ADAPTER pAd, char *rsp_payload, UINT16 rsp_payload_len);

/*MURU END*/
#endif/*CFG_SUPPORT_FALCON_MURU*/
#endif/* __AP_MURU_H__ */
