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
    mt_rdm.h//Jelly20150123
*/


#ifndef _MT_RDM_H_
#define _MT_RDM_H_

#ifdef MT_DFS_SUPPORT

#include "rt_config.h"


/* Remember add a RDM compile flag -- Shihwei 20141104 */
/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

#if defined(MT7615) || defined(MT7915) || defined(MT7622) || defined(MT7986) || defined(MT7916) || defined(MT7981)
/* Single PHY, DBDC, RDD0/RDD1 */
/* DBDC 0 is for 2G/5G (RDD0) */
/* DBDC 1 is for 2G/5G (RDD1) */
#define RDD_PROJECT_TYPE_1 1
#else
#define RDD_PROJECT_TYPE_1 0
#endif

#define RDD_PROJECT_TYPE_2 0

#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
#define RDD_2_SUPPORTED 1
#else
#define RDD_2_SUPPORTED 0
#endif

typedef enum {
	RDD_STOP = 0,
	RDD_START,
	RDD_DET_MODE,
	RDD_RADAR_EMULATE,
#ifdef	CONFIG_RCSA_SUPPORT
	RDD_DETECT_INFO = 11,
	RDD_ALTX_CTRL = 12,
#endif
	RDD_START_TXQ = 20,
	CAC_START = 50,
	CAC_END,
	NORMAL_START,
	DISABLE_DFS_CAL,
	RDD_PULSEDBG,
	RDD_READPULSE,
	RDD_RESUME_BF,
	RDD_IRQ_OFF,
	Dfs_CTRL_NUM,
} DFS_CTRL_TYPE;

typedef enum _ENUM_DFS_RadarReportMode {
	ENUM_RDD_PULSE_DATA_OFF = 0,
	ENUM_RDD_PULSE_DATA_ON,
	ENUM_RDD_REPORT_MODE_NUM,
} ENUM_DFS_RadarReportMode, *P_EENUM_DFS_RadarReportMode;

#define DFS_RDD_PULSEDATA_NUM 5

typedef enum _ENUM_RDD_REGION_D
{
    ENUM_RDM_CE = 0,
    ENUM_RDM_FCC,
    ENUM_RDM_JAP,
    ENUM_RDM_JAP_W53,
    ENUM_RDM_JAP_W56,
    ENUM_RDM_CHN,
    ENUM_RDM_KR,
    ENUM_RDM_REGION_NUM
} ENUM_RDD_REGION_D, *P_ENUM_RDD_REGION_D;

typedef enum _ENUM_RDD_RADAR_D
{
    ENUM_RDM_FCC_1_JP_1 = 0,
    ENUM_RDM_FCC_2,
    ENUM_RDM_FCC_3,
    ENUM_RDM_FCC_4,
    ENUM_RDM_FCC_6,
    ENUM_RDM_ETSI_1 = 5,
    ENUM_RDM_ETSI_2,
    ENUM_RDM_ETSI_3,
    ENUM_RDM_ETSI_4,
    ENUM_RDM_ETSI_5_2PRI,
    ENUM_RDM_ETSI_5_3PRI = 10,
    ENUM_RDM_ETSI_6_2PRI,
    ENUM_RDM_ETSI_6_3PRI,
    ENUM_RDM_JP_2,
    ENUM_RDM_JP_3,      /* New Japan radar */
    ENUM_RDM_JP_4 = 15, /* New Japan radar */
    ENUM_RDM_KR_1,      /* Korea radar */
    ENUM_RDM_KR_2,      /* Korea radar */
    ENUM_RDM_KR_3,      /* Korea radar */
    ENUM_RDM_RADARTYPE_NUM
} ENUM_RDD_RADAR_D, *P_ENUM_RDD_RADAR_D;

#if (RDD_2_SUPPORTED == 1)
enum {
	HW_RDD0 = 0,
	HW_RDD1,
	HW_RDD2,     /* Dedicated RX */
	HW_RDD_NUM,
};
#define RDD_DEDICATED_RX HW_RDD2

#else
enum {
	HW_RDD0 = 0,
	HW_RDD1,
	HW_RDD_NUM,
};
#define RDD_DEDICATED_RX HW_RDD1

#endif

#define RESTRICTION_BAND_LOW	116
#define RESTRICTION_BAND_HIGH	128
#define CHAN_SWITCH_PERIOD 10
#define CHAN_NON_OCCUPANCY 1800
#define CAC_NON_WETHER_BAND 65
#define CAC_WETHER_BAND 605
#define GROUP1_LOWER 36
#define GROUP1_UPPER 48
#define GROUP2_LOWER 52
#define GROUP2_UPPER 64
#define GROUP3_LOWER 100
#define GROUP3_UPPER 112
#define GROUP4_LOWER 116
#define GROUP4_UPPER 128

#define DFS_BW_CH_QUERY_LEVEL1 1
#define DFS_BW_CH_QUERY_LEVEL2 2
#define DFS_AVAILABLE_LIST_BW_NUM 4
#define DFS_AVAILABLE_LIST_CH_NUM 30/*MAX_NUM_OF_CHANNELS*/
#define DFS_BW40_GROUP_NUM	15
#define DFS_BW80_GROUP_NUM	9
#define DFS_BW160_GROUP_NUM	4
#define DFS_BW40_PRIMCH_NUM	2
#define DFS_BW80_PRIMCH_NUM	4
#define DFS_BW160_PRIMCH_NUM	8

#define RadarDetectSelectRandom 0
#define RadarDetectSelectDFS 1
#define RadarDetectSelectNonDFS 2
#define RadarDetectSelectNum    3

#define DFS_MACHINE_BASE	0
#define DFS_BEFORE_SWITCH    0
#define DFS_MAX_STATE		1
#define DFS_CAC_END 0
#define DFS_CHAN_SWITCH_TIMEOUT 1
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
#define DFS_OFF_CAC_END			2
#define DFS_V10_W56_APDOWN_FINISH	3
#define DFS_V10_W56_APDOWN_ENBL		4
#define DFS_V10_ACS_CSA_UPDATE		5
#define DFS_V10_ZW_DFS_ACS_UPDATE	6
#define DFS_MAX_MSG			7
#else
#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
#define DFS_OFF_CAC_END 	2
#define DFS_NOP_END 		3
#define DFS_MAX_MSG			4
#else
#define DFS_MAX_MSG			2
#endif
#endif
#define DFS_FUNC_SIZE (DFS_MAX_STATE * DFS_MAX_MSG)

#define DFS_NOP_END 		3
/* Radar type */
/* FCC1(JP-1)-FCC6, ETSI1-6, ETSI5-6 (3 PRI),  JP-2, KR-3, reserved type*2 */
#define RDD_RT_NUM 19
#define RAMP_TIME 0x4000000

/* DFS zero wait */
#define ZeroWaitCacApplyDefault      0xFF  /* Apply default setting */
#define BgnScanCacUnit               60000 /* unit is 1ms */
#define DEFAULT_OFF_CHNL_CAC_TIME    (1*BgnScanCacUnit+3000) /* 6*BgnScanCacUnit //6 mins, unit is 1minute for non-weather band channel */
#define WEATHER_OFF_CHNL_CAC_TIME    (10*BgnScanCacUnit+3000) /* 60*BgnScanCacUnit //60 mins, unit is 1minute for weather band channel */
#define DYNAMIC_ZEROWAIT_ON			1
#define DYNAMIC_ZEROWAIT_OFF			0

#define IS_CH_ABAND(_ch)	\
		(_ch > 14)

#define IS_CH_BETWEEN(_ch, _low, _high)       \
		(_ch >= _low) && (_ch <= _high)

#if defined(DFS_ADJ_BW_ZERO_WAIT) || defined(DFS_MT7916_DEDICATED_ZW) || defined(DFS_MT7981_DEDICATED_ZW)
#define IS_ADJ_BW_ZERO_WAIT(_state)	\
		((_state == DFS_BW160_TX160RX160) ||	\
		 (_state == DFS_BW160_TX80RX160) ||		\
		 (_state == DFS_BW160_TX80RX80) ||      \
		 (_state == DFS_BW80_TX80RX80) ||      \
		 (_state == DFS_BW80_TX80RX160))

#define IS_ADJ_BW_ZERO_WAIT_TX80RX160(_state)	\
		((_state == DFS_BW160_TX80RX160) ||		\
		 (_state == DFS_BW80_TX80RX160))

#define IS_ADJ_BW_ZERO_WAIT_TX80RX80(_state)	\
		((_state == DFS_BW160_TX80RX80) ||      \
		 (_state == DFS_BW80_TX80RX80))

#define IS_ADJ_BW_ZERO_WAIT_BW80(_state)	\
		((_state == DFS_BW160_TX160RX160) ||	\
		 (_state == DFS_BW160_TX80RX160) ||		\
		 (_state == DFS_BW160_TX80RX80))

#define IS_ADJ_BW_ZERO_WAIT_CAC_DONE(_state)	\
		((_state == DFS_BW80_TX80RX80) ||      \
		 (_state == DFS_BW160_TX160RX160))

#define IS_ADJ_BW_ZERO_WAIT_BW160(_state)	\
		((_state == DFS_BW80_TX80RX80) ||      \
		 (_state == DFS_BW80_TX80RX160))
#endif

#define GET_BGND_PARAM(_pAd, _param)		\
	DfsGetBgndParameter(pAd, _param)

#define IS_SUPPORT_MULTIPLE_RDD_TEST(_pAd) \
	(IS_MT7615(_pAd) || IS_MT7636(_pAd) || IS_MT7637(_pAd) || \
	 IS_MT7915(_pAd) || IS_MT7986(_pAd) || IS_MT7916(_pAd) || IS_MT7981(_pAd))
#define IS_SUPPORT_SINGLE_PHY_DBDC_DUAL_RDD(_pAd) (IS_MT7615(_pAd) || IS_MT7915(_pAd))
#define IS_SUPPORT_RDD2_DEDICATED_RX(_pAd) (IS_MT7915(_pAd) || IS_MT7916(_pAd))

enum {
	BW80Group1 = 1, /* CH36~48 */
	BW80Group2,     /* CH52~64 */
	BW80Group3,     /* CH100~112 */
	BW80Group4,     /* CH116~128 */
	BW80Group5,     /* CH132~144 */
	BW80Group6,     /* CH149~161 */
};

enum {
	RXSEL_0 = 0,    /*RxSel = 0*/
	RXSEL_1,	/*RxSel = 1*/
};

enum {
	REG_DEFAULT = 0,    /*No region distinguish*/
	REG_JP_53,	    /*JAP_53*/
	REG_JP_56,          /*JAP_56*/
};

enum {
	RDD_BAND0 = 0,
	RDD_BAND1,
	RDD_BAND_NUM
};

enum {
	RDD_DETMODE_OFF = 0,
	RDD_DETMODE_ON, /* for radar detection rate test */
	RDD_DETMODE_DEBUG, /* for radar detection debug if radar pulses cannot be recognized */
	RDD_DETMODE_NUM,
};

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
#define V10_W52_SIZE           4
#define V10_W53_SIZE           4
#define V10_W52_W53_SIZE       8
#define V10_W56_VHT160_A_SIZE  8
#define V10_W56_VHT80_A_SIZE   4
#define V10_W56_VHT80_B_SIZE   4
#define V10_W56_VHT80_C_SIZE   4
#define V10_W56_VHT80_SIZE     (V10_W56_VHT80_A_SIZE + V10_W56_VHT80_A_SIZE + V10_W56_VHT80_C_SIZE)
#define V10_W56_VHT20_SIZE     3
#define V10_W56_SIZE          12
#define V10_TOTAL_CHANNEL_COUNT (V10_W52_SIZE + V10_W53_SIZE \
				+ V10_W56_VHT80_A_SIZE + V10_W56_VHT80_B_SIZE + V10_W56_VHT80_C_SIZE)
#define V10_5G_TOTAL_CHNL_COUNT (V10_TOTAL_CHANNEL_COUNT + V10_LAST_SIZE)

typedef enum _V10_NEC_GRP_LIST {
	W52 = 0,    /* CH36~48 */
	W53,        /* CH52~64 */
	W52_53,		/* CH36~64 */
	W56_160_UA, /* CH100~128 */
	W56_160_UB, /* CH132~140 */
	W56_UA,     /* CH100~112 */
	W56_UB,     /* CH116~128 */
	W56_UC,     /* CH132~140 */
	W56_UAB,    /* VHT80 Ch 100 ~ 128*/
	W56,        /* All W56 */
	NA_GRP
} V10_NEC_GRP_LIST;

#define V10_LAST_SIZE 5

#define GROUP5_LOWER 132

#define GROUP6_LOWER 149

#define V10_WEIGH_FACTOR_W53 3
#define V10_WEIGH_FACTOR_W52 2
#define V10_WEIGH_FACTOR_W56 1

#define V10_W56_APDOWN_TIME 1805
#define V10_BGND_SCAN_TIME 20
#define V10_NORMAL_SCAN_TIME 200

#define IS_V10_W56_VHT80_SWITCHED(_pAd) \
	(_pAd->CommonCfg.bV10W56SwitchVHT80 == TRUE)
#define SET_V10_W56_VHT80_SWITCH(_pAd, switch) \
	(_pAd->CommonCfg.bV10W56SwitchVHT80 = switch)
#define IS_V10_W56_VHT160_SWITCHED(_pAd) \
		(_pAd->CommonCfg.bV10W56SwitchVHT160 == TRUE)
#define SET_V10_W56_VHT160_SWITCH(_pAd, switch) \
	(_pAd->CommonCfg.bV10W56SwitchVHT160 = switch)

#define GET_V10_OFF_CHNL_TIME(_pAd) \
	(_pAd->CommonCfg.DfsParameter.gV10OffChnlWaitTime)
#define SET_V10_OFF_CHNL_TIME(_pAd, waitTime) \
	(_pAd->CommonCfg.DfsParameter.gV10OffChnlWaitTime = waitTime)

#define IS_V10_W56_GRP_VALID(_pAd) \
	 (_pAd->CommonCfg.DfsParameter.bV10W56GrpValid == TRUE)
#define SET_V10_W56_GRP_VALID(_pAd, valid) \
	(_pAd->CommonCfg.DfsParameter.bV10W56GrpValid = valid)
#endif

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/


/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

typedef struct _DFS_CHANNEL_LIST {
	UCHAR Channel;
	USHORT NonOccupancy;
	UCHAR NOPClrCnt;
	UCHAR SupportBwBitMap;
	USHORT NOPSaveForClear;
	UCHAR NOPSetByBw;
} DFS_CHANNEL_LIST, *PDFS_CHANNEL_LIST;

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
typedef struct _V10_CHANNEL_LIST {
	UINT_32 BusyTime;
	UCHAR Channel;
	BOOLEAN isConsumed;
} V10_CHANNEL_LIST, *PV10_CHANNEL_LIST;
#endif

enum DFS_SET_NEWCH_MODE {
	DFS_SET_NEWCH_DISABLED = 0x0,
	DFS_SET_NEWCH_ENABLED = 0x1,
	DFS_SET_NEWCH_INIT = 0xa,	/* Magic number 0xa stands for initial value */
};

enum {
	DFS_IDLE = 0,
	DFS_INIT_CAC,
	DFS_CAC,          /* Channel avail check state */
	DFS_OFF_CHNL_CAC_TIMEOUT, /* Off channel CAC timeout state */
	DFS_INSERV_MONI, /* In service monitor state */
	DFS_RADAR_DETECT,  /* Radar detected  state */
	DFS_MBSS_CAC,
};

#if (RDD_2_SUPPORTED == 1)
enum {
	INBAND_CH_BAND0 = 0,
	INBAND_CH_BAND1,
	INBAND_BW_BAND0,
	INBAND_BW_BAND1,
	OUTBAND_CH,
	OUTBAND_BW,
	ORI_INBAND_CH,
	ORI_INBAND_BW,
};

#else
enum {
	INBAND_CH = 0,
	INBAND_BW,
	OUTBAND_CH,
	OUTBAND_BW,
	ORI_INBAND_CH,
	ORI_INBAND_BW,
};
#endif

enum ZEROWAIT_ACT_CODE{
	ZERO_WAIT_DFS_ENABLE = 0,/*0*/
	INIT_AVAL_CH_LIST_UPDATE,
	MONITOR_CH_ASSIGN,
	NOP_FORCE_SET,
	PRE_ASSIGN_NEXT_TARGET,
	SHOW_TARGET_INFO,
	QUERY_AVAL_CH_LIST = 20,
	QUERY_NOP_OF_CH_LIST,

};

/*Report to Customer*/
typedef struct _DFS_REPORT_AVALABLE_CH_LIST {
	UCHAR Channel;
	UCHAR RadarHitCnt;
} DFS_REPORT_AVALABLE_CH_LIST, *PDFS_REPORT_AVALABLE_CH_LIST;

typedef struct _NOP_REPORT_CH_LIST {
	UCHAR Channel;
	UCHAR Bw;
	USHORT NonOccupancy;
} NOP_REPORT_CH_LIST, *PNOP_REPORT_CH_LIST;

union dfs_zero_wait_msg {
	struct _aval_channel_list_msg{
		UCHAR Action;
		UCHAR Bw80TotalChNum;
		UCHAR Bw40TotalChNum;
		UCHAR Bw20TotalChNum;
		UCHAR Bw160TotalChNum;
		DFS_REPORT_AVALABLE_CH_LIST Bw80AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
		DFS_REPORT_AVALABLE_CH_LIST Bw40AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
		DFS_REPORT_AVALABLE_CH_LIST Bw20AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
		DFS_REPORT_AVALABLE_CH_LIST Bw160AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
	} aval_channel_list_msg;

	struct _nop_of_channel_list_msg{
		UCHAR Action;
		UCHAR NOPTotalChNum[DBDC_BAND_NUM];
		NOP_REPORT_CH_LIST NopReportChList[DBDC_BAND_NUM][DFS_AVAILABLE_LIST_CH_NUM];
	} nop_of_channel_list_msg;

	struct _set_monitored_ch_msg{
		UCHAR Action;
		UCHAR SyncNum;
		UCHAR Channel;
		UCHAR Bw;
		UCHAR doCAC;
	} set_monitored_ch_msg;

	struct _zerowait_dfs_ctrl_msg{
		UCHAR Action;
		UCHAR Enable;
	} zerowait_dfs_ctrl_msg;

	struct _nop_force_set_msg{
		UCHAR Action;
		UCHAR Channel;
		UCHAR Bw;
		USHORT NOPTime;
	} nop_force_set_msg;

	struct _assign_next_target{
		UCHAR Channel;
		UCHAR Bw;
		USHORT CacValue;
	} assign_next_target;

	struct _target_ch_show{
		UCHAR mode;
	} target_ch_show;
};

#ifdef DFS_ZEROWAIT_SUPPORT
typedef struct _CHANNEL_SWITCH_CAC {
	UCHAR Channel;
	UCHAR cac_req;
} CHAN_SWITCH_CAC, *PCHAN_SWITCH_CAC;
#endif

typedef struct _DFS_CH_GRP {
	UCHAR AvailableBwChIdx[DFS_AVAILABLE_LIST_BW_NUM][DFS_AVAILABLE_LIST_CH_NUM];
	UCHAR Bw40GroupIdx[DFS_BW40_GROUP_NUM][DFS_BW40_PRIMCH_NUM];
	UCHAR Bw80GroupIdx[DFS_BW80_GROUP_NUM][DFS_BW80_PRIMCH_NUM];
	UCHAR Bw160GroupIdx[DFS_BW160_GROUP_NUM][DFS_BW160_PRIMCH_NUM];
} DFS_CH_GRP, *PDFS_CH_GRP;

typedef enum _ENUM_DFS_INB_CH_SWITCH_STAT_T {
	DFS_INB_CH_INIT = 0,
	DFS_OUTB_CH_CAC,
	DFS_INB_CH_SWITCH_CH,
	DFS_INB_DFS_OUTB_CH_CAC,
	DFS_INB_DFS_OUTB_CH_CAC_DONE,
	DFS_INB_DFS_RADAR_OUTB_CAC_DONE
} ENUM_DFS_INB_CH_SWITCH_STAT_T, *P_ENUM_DFS_INB_CH_SWITCH_STAT_T;

typedef struct _DFS_PULSE_THRESHOLD_PARAM {
	UINT32 pls_width_max;		/* unit us */
	INT32 pls_pwr_max;			/* unit dbm */
	INT32 pls_pwr_min;			/* unit dbm */
	UINT32 pri_min_stgr;		/* unit us */
	UINT32 pri_max_stgr;		/* unit us */
	UINT32 pri_min_cr;			/* unit us */
	UINT32 pri_max_cr;			/* unit us */
} DFS_PULSE_THRESHOLD_PARAM, *PDFS_PULSE_THRESHOLD_PARAM;

typedef struct _DFS_RADAR_THRESHOLD_PARAM {
	DFS_PULSE_THRESHOLD_PARAM pls_thrshld_param;
	BOOLEAN afgSupportedRT[RDD_RT_NUM];
	SW_RADAR_TYPE_T sw_radar_type[RDD_RT_NUM];
} DFS_RADAR_THRESHOLD_PARAM, *PDFS_RADAR_THRESHOLD_PARAM;

typedef struct _DFS_PARAM {
	UCHAR band_ch[RDD_BAND_NUM];
	UCHAR PrimCh;
	UCHAR PrimBand;
	/*UCHAR bw;*/
	UCHAR band_bw[RDD_BAND_NUM];
	UCHAR RDDurRegion;
	DFS_CH_GRP dfs_ch_grp[DBDC_BAND_NUM];
	struct DOT11_H Dot11_H[DBDC_BAND_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw80AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw40AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw20AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	BOOLEAN bIEEE80211H;
	BOOLEAN DfsChBand[HW_RDD_NUM];
	BOOLEAN RadarDetected[HW_RDD_NUM];
	BOOLEAN RadarDetectState[RDD_BAND_NUM];
	UCHAR NeedSetNewChList[RDD_BAND_NUM];
	BOOLEAN bNoAvailableCh[RDD_BAND_NUM];
	BOOLEAN DisableDfsCal;
	BOOLEAN bNoSwitchCh;
	BOOLEAN bDfsEnable;
	UCHAR RadarHitIdxRecord;
	UCHAR targetCh;
	UCHAR targetBw;
	USHORT targetCacValue;
	UCHAR DfsChSelPrefer;                /*Select prefer channel DFS/non-DFS/All when hit radar*/

	/* DFS zero wait */
	BOOLEAN bZeroWaitSupport;    /* Save the profile setting of DfsZeroWait */
	UCHAR   ZeroWaitDfsState;    /* for DFS zero wait state machine using */
	UCHAR   DfsZeroWaitCacTime;  /* unit is minute and Maximum Off-Channel CAC time is one hour */
	BOOLEAN bZeroWaitCacSecondHandle;
	BOOLEAN bDedicatedZeroWaitSupport;
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	ULONG   gV10OffChnlWaitTime;
	ULONG   gV10W56TrgrApDownTime;
	BOOLEAN bDFSV10Support; /* NEC DFS Support */
	BOOLEAN bV10ChannelListValid;
	BOOLEAN bV10BootACSValid;
	BOOLEAN bV10W56GrpValid;
	BOOLEAN bV10W56SwitchVHT80;
	BOOLEAN bV10W56SwitchVHT160;
	BOOLEAN bV10W56APDownEnbl;
	BOOLEAN bV10APBcnUpdateEnbl;
	BOOLEAN bV10APInterfaceDownEnbl;
	BOOLEAN bV10ZWDFSACSEnbl;
	BOOLEAN bV10APUpChUpdate;
	UCHAR   GroupCount; /* Max Group Count from ACS */
	V10_CHANNEL_LIST DfsV10SortedACSList[V10_TOTAL_CHANNEL_COUNT];
#endif
	UCHAR	OutBandCh;
	UCHAR	OutBandBw;
	UCHAR	OrigInBandCh;
	UCHAR	OrigInBandBw;
	USHORT	DedicatedOutBandCacCount;
	BOOLEAN bOutBandAvailable;
	BOOLEAN bSetInBandCacReStart;
	BOOLEAN bDedicatedZeroWaitDefault;
	BOOLEAN bInitOutBandBranch;
	USHORT	DedicatedOutBandCacTime;
	BOOLEAN RadarHitReport;
	UCHAR	OutBandAvailableCh;
	/* MBSS DFS zero wait */
	BOOLEAN bInitMbssZeroWait;
	ENUM_DFS_INB_CH_SWITCH_STAT_T inband_ch_stat;

	STATE_MACHINE_FUNC		DfsStateFunc[DFS_FUNC_SIZE];
	STATE_MACHINE			DfsStatMachine;

	/*Threshold params*/
	UINT16 fcc_lpn_min;
	BOOLEAN is_hw_rdd_log_en;
	BOOLEAN is_sw_rdd_log_en;
	BOOLEAN sw_rdd_log_cond; /* 0: dump every interrupt (DEBUG), 1: only a radar is detected */
	BOOLEAN is_radar_emu;
	DFS_RADAR_THRESHOLD_PARAM radar_thrshld_param;
#ifdef CONFIG_RCSA_SUPPORT
	BOOLEAN bRCSAEn;
	BOOLEAN fSendRCSA;
	BOOLEAN fUseCsaCfg;
	BOOLEAN fCheckRcsaTxDone;
	UCHAR	ChSwMode;
#endif
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
	UCHAR DFSChHitBand;
#endif
#ifdef DFS_ADJ_BW_ZERO_WAIT
	BOOLEAN BW160ZeroWaitState;
	BOOLEAN BW160ZeroWaitSupport;   /* indicate ap support BW160 zeor-wait */
	BOOLEAN BW160ZeroWaitStartNOPCounter;
#endif
#if defined(DFS_MT7916_DEDICATED_ZW) || defined(DFS_MT7981_DEDICATED_ZW)
	BOOLEAN BW160DedicatedSup;
	BOOLEAN BW160DedicatedZWSupport;
	BOOLEAN BW160DedicatedZWState;
#endif
	UINT32	TriggerEventIntvl;
} DFS_PARAM, *PDFS_PARAM;

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/


/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
BOOLEAN IsChABand(
	IN USHORT PhyMode,
	IN UCHAR channel
	);

INT ZeroWaitDfsCmdHandler(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT * wrq
	);

#ifdef DFS_ZEROWAIT_SUPPORT
INT ZeroWaitDfsChannelSwitch(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT * wrq
	);
#endif

INT ZeroWaitDfsQueryCmdHandler(
	RTMP_ADAPTER *pAd,
	RTMP_IOCTL_INPUT_STRUCT * wrq
	);

INT zero_wait_dfs_update_inband_nondfsch(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	INOUT PUCHAR ch
	);

INT zero_wait_dfs_update_ch(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR OriChannel,
	INOUT PUCHAR ch
	);

INT zero_wait_dfs_switch_ch(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR band_idx
	);

#ifdef DFS_ADJ_BW_ZERO_WAIT
VOID Adj_ZeroWait_Status_Update(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	INOUT PUCHAR ch
	);
#endif

#if defined(DFS_ADJ_BW_ZERO_WAIT) || defined(DFS_MT7916_DEDICATED_ZW) || defined(DFS_MT7981_DEDICATED_ZW)
VOID DfsZeroWaitBW160StateUpdate(
	IN PRTMP_ADAPTER pAd,
	INOUT PUCHAR vht_bw,
	IN UCHAR Ch
	);
#endif

INT Set_RadarDetectMode_Proc(
	RTMP_ADAPTER * pAd, RTMP_STRING *arg);

INT Set_RadarDetectStart_Proc(
	RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_RadarDetectStop_Proc(
	RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ByPassCac_Proc(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

INT Set_RDDReport_Proc(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

/**
* Trigger_RDD_Event - Trigger RDD related event.
* @pAd: pointer of the RTMP_ADAPTER
* @arg: event type (0: not send event, 1: radar detect; 2: CAC timeout; 3: CSA done.)
*
* This function is for feature debug
*
**/
INT Trigger_RDD_Event(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg);

/**
* MakeUpRDDEvent - Make up radar detected event.
* @pAd: pointer of the RTMP_ADAPTER
*
* This function is for feature debug
*
**/
VOID MakeUpRDDEvent(RTMP_ADAPTER *pAd);

UCHAR DfsPrimToCent(
	UCHAR Channel, UCHAR Bw);

UCHAR DfsGetBgndParameter(
	IN PRTMP_ADAPTER pAd, UCHAR QueryParam);

VOID DfsGetSysParameters(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR vht_cent2,
	UCHAR phy_bw);

VOID DfsParamInit(/* finish */
	IN PRTMP_ADAPTER	pAd);

VOID DfsStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

INT Set_DfsChannelShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);

INT Set_DfsBwShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);

INT Set_DfsRDModeShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);

INT Set_DfsRDDRegionShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);

INT Show_DfsNonOccupancy_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);

INT Nop_List_Backup(
	IN PRTMP_ADAPTER pAd);

INT show_dfs_ch_info_proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg);

INT Set_DfsNOP_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);

INT Set_DfsChSelPrefer_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/* DFS Zero Wait */
INT Set_DfsZeroWaitEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DfsZeroWaitCacTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DedicatedBwCh_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DfsZeroWaitDynamicCtrl_Proc(
	RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DfsZeroWaitNOP_Proc(
		RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DfsTargetCh_Proc(
		RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef DFS_ADJ_BW_ZERO_WAIT
INT Set_DfsBypassNop_Proc(
		RTMP_ADAPTER * pAd, RTMP_STRING *arg);
#endif
#ifdef DFS_ZEROWAIT_SUPPORT
INT Set_ZWDfsChannelSwitch_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
VOID DfsSetCalibration(
	IN PRTMP_ADAPTER pAd, UINT_32 DisableDfsCal);

VOID DfsSetZeroWaitCacSecond(
	IN PRTMP_ADAPTER pAd);

BOOLEAN DfsBypassRadarStateCheck(
	struct wifi_dev *wdev);

BOOLEAN DfsRadarChannelCheck(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR vht_cent2,
	UCHAR phy_bw);

#ifdef SCAN_RADAR_COEX_SUPPORT
BOOLEAN DfsRadarChannelCheckForCMD(
	PRTMP_ADAPTER pAd,
	UCHAR channel,
	UCHAR vht_cent2,
	UCHAR phy_bw);
#endif /* SCAN_RADAR_COEX_SUPPORT */

VOID DfsSetNewChInit(
	IN PRTMP_ADAPTER pAd);

VOID DfsCacEndUpdate(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem);

#ifdef DFS_ADJ_BW_ZERO_WAIT
VOID DfsNopEndUpdate(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem);
#endif

#if (DFS_ZEROWAIT_DEFAULT_FLOW == 1)
VOID dfs_off_cac_end_update(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem);

UCHAR dfs_get_band_by_ch(
	RTMP_ADAPTER *pAd,
	UCHAR ch);

#endif

NTSTATUS DfsChannelSwitchTimeoutAction(
	PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt);

NTSTATUS DfsAPRestart(
	PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt);

NTSTATUS DfsSwitchChAfterRadarDetected(
	PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt);

VOID DfsCacNormalStart(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR CompareMode);

BOOLEAN DfsCacRestrictBand(
	IN PRTMP_ADAPTER pAd, IN UCHAR Bw, IN UCHAR Ch, IN UCHAR SecCh);

VOID DfsBuildChannelList(
	IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev);

VOID DfsBuildChannelGroupByBw(
	IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev);

BOOLEAN DfsCheckBwGroupAllAvailable(
	IN UCHAR ChechChIdx, IN UCHAR Bw, IN PRTMP_ADAPTER pAd, IN UCHAR band_idx);

BOOLEAN DfsSwitchCheck(/* finish */
	IN PRTMP_ADAPTER	pAd,
	UCHAR	Channel,
	UCHAR bandIdx);

BOOLEAN DfsStopWifiCheck(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev);

VOID DfsNonOccupancyUpdate(
	IN PRTMP_ADAPTER pAd);

VOID DfsNonOccupancyCountDown(/*NonOccupancy --, finish*/
	IN PRTMP_ADAPTER pAd);

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
USHORT DfsV10SelectBestChannel(/*Select the Channel from Rank List by ACS*/
	IN PRTMP_ADAPTER pAd,
	IN UCHAR oldChannel,
	IN UCHAR band_idx);

UCHAR DfsV10CheckChnlGrp(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN UCHAR Channel);

BOOLEAN DfsV10CheckW56Grp(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN UCHAR channel);

VOID DfsV10AddWeighingFactor(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *pwdev);

BOOLEAN DfsV10CheckGrpChnlLeft(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR chGrp,
	IN UCHAR grpWidth,
	IN UCHAR band_idx);

USHORT DfsV10W56FindMaxNopDuration(
	IN PRTMP_ADAPTER pAd);

UINT_8 DfsV10FindNonNopChannel(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR chGrp,
	IN UCHAR grpWidth);

BOOLEAN DfsV10W56APDownStart(
	IN PRTMP_ADAPTER pAd,
	IN PAUTO_CH_CTRL pAutoChCtrl,
	IN ULONG V10W56TrgrApDownTime,
	IN UCHAR band_idx);

VOID DfsV10W56APDownTimeCountDown(/*RemainingTimeForUse --*/
	IN PRTMP_ADAPTER pAd);

VOID DfsV10W56APDownPass(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem);

VOID DfsV10W56APDownEnbl(
	RTMP_ADAPTER *pAd,
	PMLME_QUEUE_ELEM pElem);

VOID DfsV10APBcnUpdate(
	RTMP_ADAPTER *pAd,
	PMLME_QUEUE_ELEM pElem);

VOID DfsV10APZWDfsChUpdate(
	RTMP_ADAPTER *pAd,
	PMLME_QUEUE_ELEM pElem);
#endif

VOID WrapDfsRddReportHandle(/*handle the event of EXT_EVENT_ID_RDD_REPORT*/
	IN PRTMP_ADAPTER pAd, UCHAR ucRddIdx);

BOOLEAN DfsRddReportHandle(/*handle the event of EXT_EVENT_ID_RDD_REPORT*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam, UCHAR rddidx, UCHAR bandIdx);

VOID WrapDfsSetNonOccupancy(/*Set Channel non-occupancy time, finish */
	IN PRTMP_ADAPTER pAd, IN UCHAR rddidx, IN UCHAR bandIdx);

VOID DfsSetNonOccupancy(/*Set Channel non-occupancy time, finish*/
	IN PRTMP_ADAPTER pAd,
	IN UCHAR band_idx,
	IN UINT_8 TargetCh,
	IN UINT_8 TargetBw,
	IN BOOLEAN TargetChDfsBand);

VOID WrapDfsSelectChannel(/*Select new channel, finish*/
	IN PRTMP_ADAPTER pAd, UCHAR band_idx);

VOID DfsSelectChannel(/*Select new channel, finish*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam, UCHAR band_idx);

UCHAR WrapDfsRandomSelectChannel(/*Select new channel using random selection, finish*/
	IN PRTMP_ADAPTER pAd, UCHAR avoidCh, UCHAR band_idx);

UCHAR DfsRandomSelectChannel(/*Select new channel using random selection, finish*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam, UCHAR avoidCh, UCHAR band_idx);

USHORT DfsBwChQueryByDefault(/*Query current available BW & Channel list or select default*/
	IN PRTMP_ADAPTER pAd, UCHAR Bw, PDFS_PARAM pDfsParam, UCHAR level, BOOLEAN bDefaultSelect, BOOLEAN SkipNonDfsCh, UCHAR band_idx);

VOID DfsBwChQueryAllList(/*Query current All available BW & Channel list*/
	IN PRTMP_ADAPTER pAd, UCHAR Bw, PDFS_PARAM pDfsParam, BOOLEAN SkipWorkingCh, UCHAR band_idx);

BOOLEAN DfsDedicatedCheckChBwValid(
	IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw, UCHAR band_idx);

VOID DfsAdjustBwSetting(
	IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR CurrentBw, UCHAR NewBw);

UCHAR DfsAdjustBwSettingAllBssid(
	IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR band_idx, UCHAR KeepBw);

BOOLEAN DfsSanityCheck(
	IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR band_idx);

VOID WrapDfsRadarDetectStart(/*Start Radar Detection or not, finish*/
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev);

VOID DfsRadarDetectStart(/*Start Radar Detection or not, finish*/
	IN PRTMP_ADAPTER pAd,
	PDFS_PARAM pDfsParam,
	struct wifi_dev *wdev);

VOID WrapDfsRadarDetectStop(/*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd);

VOID DfsRadarDetectStop(/*Start Radar Detection or not, finish*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam);

VOID DfsDedicatedOutBandRDDStart(
	IN PRTMP_ADAPTER pAd);

VOID DfsDedicatedOutBandRDDRunning(
	IN PRTMP_ADAPTER pAd);

VOID DfsDedicatedOutBandRDDStop(
	IN PRTMP_ADAPTER pAd);

BOOLEAN DfsIsRadarHitReport(
	IN PRTMP_ADAPTER pAd);

VOID DfsRadarHitReportReset(
	IN PRTMP_ADAPTER pAd);

BOOLEAN DfsIsTargetChAvailable(
	IN PRTMP_ADAPTER pAd);

VOID DfsReportCollision(
	IN PRTMP_ADAPTER pAd);

BOOLEAN DfsIsOutBandAvailable(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev);

VOID DfsOutBandCacReset(
	IN PRTMP_ADAPTER pAd);

VOID DfsSetCacRemainingTime(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev);

VOID DfsOutBandCacCountUpdate(
	IN PRTMP_ADAPTER pAd);

UCHAR DfsGetCentCh(IN PRTMP_ADAPTER pAd, IN UCHAR Channel, IN UCHAR bw, IN struct wifi_dev *wdev);

VOID DfsDedicatedExamineSetNewCh(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR Channel);

VOID DfsDedicatedSetNewChStat(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR Channel);

INT mtRddControl(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR ucRddCtrl,
	IN UCHAR ucRddIdex,
	IN UCHAR ucRddInSel,
	IN UCHAR ucSetVal);

#ifdef BACKGROUND_SCAN_SUPPORT
VOID DfsDedicatedScanStart(IN PRTMP_ADAPTER pAd);
VOID DfsInitDedicatedScanStart(IN PRTMP_ADAPTER pAd);
VOID DfsSetInitDediatedScanStart(IN PRTMP_ADAPTER pAd);
VOID DfsDedicatedInBandSetChannel(IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw, BOOLEAN doCAC, UCHAR band_idx);
VOID DfsDedicatedOutBandSetChannel(IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw, UCHAR band_idx);
VOID DfsDedicatedDynamicCtrl(IN PRTMP_ADAPTER pAd, UINT_32 DfsDedicatedOnOff);
#endif /* BACKGROUND_SCAN_SUPPORT */

INT Set_ModifyChannelList_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Show_available_BwCh_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_NOP_Of_ChList(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_Target_Ch_Info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID ZeroWait_DFS_Initialize_Candidate_List(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR Bw80Num, IN PDFS_REPORT_AVALABLE_CH_LIST pBw80AvailableChList,
	IN UCHAR Bw40Num, IN PDFS_REPORT_AVALABLE_CH_LIST pBw40AvailableChList,
	IN UCHAR Bw20Num, IN PDFS_REPORT_AVALABLE_CH_LIST pBw20AvailableChList);

VOID DfsProvideAvailableChList(
	IN PRTMP_ADAPTER pAd, IN UCHAR band_idx);

VOID DfsProvideNopOfChList(
	IN PRTMP_ADAPTER pAd, union dfs_zero_wait_msg *msg);

VOID ZeroWait_DFS_set_NOP_to_Channel_List(
	IN PRTMP_ADAPTER pAd, IN UCHAR Channel, UCHAR Bw, USHORT NOPTime);

VOID ZeroWait_DFS_Pre_Assign_Next_Target_Channel(
	IN PRTMP_ADAPTER pAd, IN UCHAR Channel, IN UCHAR Bw, IN USHORT CacValue);

VOID ZeroWait_DFS_Next_Target_Show(
	IN PRTMP_ADAPTER pAd, IN UCHAR mode);

VOID ZeroWait_DFS_collision_report(
	IN PRTMP_ADAPTER pAd, IN UCHAR SynNum, IN UCHAR Channel, UCHAR Bw);

VOID DfsZeroHandOffRecovery(IN struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

/*----------------------------------------------------------------------------*/
/*!
* @brief        Mapping RDD index to DBDC index
* @param[in]    PRTMP_ADAPTER pAd
* @param[in]    rddidx: RDD index
* @return       bandIdx: DBDC index
*/
/*----------------------------------------------------------------------------*/
UCHAR dfs_rddidx_to_dbdc(IN PRTMP_ADAPTER pAd, IN UINT8 rddidx);

/* Parsing radar pulse data */
/*----------------------------------------------------------------------------*/
/*!
* @brief        dfs_rdd_pulsedata:  parsing radar pulse data
* @param[in]    pAd                 RTMP_ADAPTER
* @param[in]    pExtEventRddReport  P_EXT_EVENT_RDD_REPORT_T
* @return       None
*/
/*----------------------------------------------------------------------------*/
VOID dfs_rdd_pulsedata(IN RTMP_ADAPTER *pAd, IN P_EXT_EVENT_RDD_REPORT_T prRadarReport);

/*----------------------------------------------------------------------------*/
/*!
* @brief        set_dfs_debug_proc: handle iwpriv command DfsDebug
* @param[in]    pAd                 RTMP_ADAPTER
* @param[in]    arg                 Trigger type
*                                   0: clear all radar data, 1: print all radar data
* @return       TRUE                Success
*/
/*----------------------------------------------------------------------------*/
INT set_dfs_debug_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_dfs_debug_proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT show_radar_threshold_param_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
VOID dfs_dump_radar_hw_pls_info(IN PRTMP_ADAPTER pAd, IN P_EXT_EVENT_RDD_REPORT_T prRadarReport);
VOID dfs_dump_radar_sw_pls_info(IN PRTMP_ADAPTER pAd, IN P_EXT_EVENT_RDD_REPORT_T prRadarReport);

VOID dfs_dump_radar_hw_pls_info(IN PRTMP_ADAPTER pAd, IN P_EXT_EVENT_RDD_REPORT_T prRadarReport);
VOID dfs_dump_radar_sw_pls_info(IN PRTMP_ADAPTER pAd, IN P_EXT_EVENT_RDD_REPORT_T prRadarReport);
VOID dfs_update_radar_info(IN P_EXT_EVENT_RDD_REPORT_T prRadarReport);

#endif /*MT_DFS_SUPPORT*/
#endif /*_MT_RDM_H_ */
