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
	mt_txbf.h
*/


#ifndef _RT_TXBF_H_
#define _RT_TXBF_H_

/*
 * to prevent reference MAX_LEN_OF_MAC_TABLE compile error.
 * if the file don't reference MAX_LEN_OF_MAC_TABLE again, the include file can be removed.
 */

#include "mgmt/mgmt_entrytb.h"

#ifdef TXBF_SUPPORT

#define TXBF_DYNAMIC_DISABLE
struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _VHT_CAP_INFO;

#ifdef TXBF_DYNAMIC_DISABLE
/*  used in rStaRecBf.ucAutoSoundingCtrl
 *   if set, BF is dynamically disabled
 */
#define DYNAMIC_TXBF_DISABLE	BIT(6)
#endif /* TXBF_DYNAMIC_DISABLE */

#define Sportan_DBG 0

/* Divider phase calibration closed loop definition */
#define RX0TX0     0
#define RX1TX1     5

#define ADC0_RX0_2R   8
#define ADC1_RX1_2R   8

/* #define MRQ_FORCE_TX		//Force MRQ regardless the capability of the station */


/* TxSndgPkt Sounding type definitions */
#define SNDG_TYPE_DISABLE		0
#define SNDG_TYPE_SOUNDING	1
#define SNDG_TYPE_NDP			2

/* Explicit TxBF feedback mechanism */
#define ETXBF_FB_DISABLE	0
#define ETXBF_FB_CSI		1
#define ETXBF_FB_NONCOMP	2
#define ETXBF_FB_COMP		4


/* #define MRQ_FORCE_TX		//Force MRQ regardless the capability of the station */

/*
	eTxBfEnCond values:
	 0:no etxbf,
	 1:etxbf update periodically,
	 2:etxbf updated if mcs changes in RateSwitchingAdapt() or APQuickResponeForRateUpExecAdapt().
	 3:auto-selection: if mfb changes or timer expires, then send sounding packets <------not finished yet!!!
	 note:
		when = 1 or 3, NO_SNDG_CNT_THRD controls the frequency to update the
		matrix(ETXBF_EN_COND=1) or activate the whole bf evaluation process(not defined)
*/

/* Defines to include optional code. */
/* NOTE: Do not define these options. ETxBfEnCond==3 and */
/* MCS Feedback are not fully implemented */
/* #define MFB_SUPPORT				// Include MCS Feedback code */

/* MCS FB definitions */
#define MSI_TOGGLE_BF		6
#define TOGGLE_BF_PKTS		5    /* the number of packets with inverted BF status */

/* TXBF State definitions */
#define READY_FOR_SNDG0	    0    /* jump to WAIT_SNDG_FB0 when channel change or periodically */
#define WAIT_SNDG_FB0		1    /* jump to WAIT_SNDG_FB1 when bf report0 is received */
#define WAIT_SNDG_FB1		2
#define WAIT_MFB			3
#define WAIT_USELESS_RSP	4
#define WAIT_BEST_SNDG		5

#define NO_SNDG_CNT_THRD	0    /* send sndg packet if there is no sounding for (NO_SNDG_CNT_THRD+1)*500msec. If this =0, bf matrix is updated at each call of APMlmeDynamicTxRateSwitchingAdapt() */


/* ------------ BEAMFORMING PROFILE HANDLING ------------ */

#define IMP_MAX_BYTES		    14  /* Implicit: 14 bytes per subcarrier */
#define IMP_MAX_BYTES_ONE_COL	7   /* Implicit: 7 bytes per subcarrier, when reading first column */
#define EXP_MAX_BYTES		    18  /* Explicit: 18 bytes per subcarrier */
#define MAX_BYTES		2   /* 2 bytes per subcarrier for implicit and explicit TxBf */
#define IMP_COEFF_SIZE		    9   /* 9 bits/coeff */
#define IMP_COEFF_MASK		    0x1FF

#define PROFILE_MAX_CARRIERS_20	56	/* Number of subcarriers in 20 MHz mode */
#define PROFILE_MAX_CARRIERS_40	114	/* Number of subcarriers in 40 MHz mode */
#define PROFILE_MAX_CARRIERS_80	242	/* Number of subcarriers in 80 MHz mode */

#define NUM_CHAIN			      3

#define TXBF_PFMU_ARRAY_SIZE      64
#define MAX_PFMU_MEM_LEN_PER_ROW  6

#define STATUS_TRUE               0

#define TX_MCS_SET_DEFINED                          BIT(0)
#define TX_MCS_SET_DEFINED_OFFSET                   0
#define TX_RX_MCS_SET_N_EQUAL                       BIT(1)
#define TX_RX_MCS_SET_N_EQUAL_OFFSET                1
#define TX_MAX_NUM_SPATIAL_STREAMS_SUPPORTED        BITS(2, 3)
#define TX_MAX_NUM_SPATIAL_STREAMS_SUPPORTED_OFFSET 2

/*
 * BF Chip Capability
 */
#define TXBF_HW_CAP                                 BIT(0)
#define TXBF_AID_HW_LIMIT                           BIT(1)
#define TXBF_HW_2BF                                 BIT(2)

#ifdef MT_MAC

typedef struct _BF_DYNAMIC_MECHANISM {
	UINT_32		bfdm_bitmap; /* bitmap for each bf dynamic mechanism on/off */
	BOOLEAN		bfdm_bfee_enabled; /* current BFee HW enable or not. 1: on, 0: off */
} BF_DYNAMIC_MECHANISM;

enum ENUM_BFDM {
	BFDM_BFEE_ADAPTION = 0
};

enum ENUM_BFDM_BITMAP {
	BFDM_BFEE_ADAPTION_BITMAP = (1 << BFDM_BFEE_ADAPTION)
};


typedef enum _TXBF_DBW_T {
	P_DBW20M = 0,
	P_DBW40M,
	P_DBW80M,
	P_DBW160M,
	DBW20M,
	DBW40M,
	DBW80M,
	DBW160M
} TXBF_DBW_T;

enum ENUM_SUBF_CAP_T {
	SUBF_OFF,
	SUBF_ALL,
	SUBF_BFER,
	SUBF_BFEE
};

enum txbf_pfmu_tag {
	TAG1_PFMU_ID,
	TAG1_IEBF,
	TAG1_DBW,
	TAG1_SU_MU,
	TAG1_INVALID,
	TAG1_MEM_ROW0,
	TAG1_MEM_ROW1,
	TAG1_MEM_ROW2,
	TAG1_MEM_ROW3,
	TAG1_MEM_COL0,
	TAG1_MEM_COL1,
	TAG1_MEM_COL2,
	TAG1_MEM_COL3,
	TAG1_RMSD,
	TAG1_NR,
	TAG1_NC,
	TAG1_NG,
	TAG1_LM,
	TAG1_CODEBOOK,
	TAG1_HTC,
	TAG1_RU_START,
	TAG1_RU_END,
	TAG1_MOB_CAL_EN,
	TAG1_SNR_STS0,
	TAG1_SNR_STS1,
	TAG1_SNR_STS2,
	TAG1_SNR_STS3,
	TAG1_SNR_STS4,
	TAG1_SNR_STS5,
	TAG1_SNR_STS6,
	TAG1_SNR_STS7,
	TAG2_SMART_ANT,
	TAG2_SE_ID,
	TAG2_RMSD_THRESHOLD,
	TAG2_IBF_TIMEOUT,
	TAG2_IBF_DBW,
	TAG2_IBF_NROW,
	TAG2_IBF_NCOL,
	TAG2_IBF_RU_ALLOC
};

enum txbf_manual_conf {
	MANUAL_HE_SU_MU = 0,
	MANUAL_HE_RU_RANGE,
	MANUAL_HE_TRIGGER,
	MANUAL_HE_NG16,
	MANUAL_HE_CODEBOOK,
	MANUAL_HE_LTF,
	MANUAL_HE_IBF,
	NANUAL_HE_BW160
};

typedef enum ENUM_BF_OUI {
	ENUM_BF_OUI_MEDIATEK = 0,
	ENUM_BF_OUI_RALINK,
	ENUM_BF_OUI_METALINK,
	ENUM_BF_OUI_BROADCOM
} ENUM_BF_OUI_T;

typedef struct _VENDOR_BF_SETTING {
	BOOLEAN	fgIsBrcm2GeTxBFIe;
	UINT8	Nrow;
} VENDOR_BF_SETTING, *P_VENDOR_BF_SETTING;

#define TXBF_PFMU_STA_ETXBF_SUP    0x1
#define TXBF_PFMU_STA_ITXBF_SUP    0x2
#define IS_ETXBF_SUP(u1TxBfCap)    ((u1TxBfCap & TXBF_PFMU_STA_ETXBF_SUP) ? TRUE : FALSE)
#define IS_ITXBF_SUP(u1TxBfCap)    ((u1TxBfCap & TXBF_PFMU_STA_ITXBF_SUP) ? TRUE : FALSE)

typedef struct _TXBF_PFMU_STA_INFO {
	UINT_16   u2PfmuId;           /* 0xFFFF means no access right for PFMU */
	BOOLEAN   fgSU_MU;            /* 0 : SU, 1 : MU */
	UINT_8    u1TxBfCap;          /* 0 : ITxBf, 1 : ETxBf */
	UINT_8    ucSoundingPhy;      /* 0: legacy, 1: OFDM, 2: HT, 4: VHT */
	UINT_8    ucNdpaRate;
	UINT_8    ucNdpRate;
	UINT_8    ucReptPollRate;
	UINT_8    ucTxMode;           /* 0: legacy, 1: OFDM, 2: HT, 4: VHT */
	UINT_8    ucNc;
	UINT_8    ucNr;
	UINT_8    ucCBW;              /* 0 : 20M, 1 : 40M, 2 : 80M, 3 : 80 + 80M */
	UINT_8    ucTotMemRequire;
	UINT_8    ucMemRequire20M;
	UINT_8    ucMemRow0;
	UINT_8    ucMemCol0     : 6;
	UINT_8    ucMemRow0Msb  : 2;
	UINT_8    ucMemRow1;
	UINT_8    ucMemCol1     : 6;
	UINT_8    ucMemRow1Msb  : 2;
	UINT_8    ucMemRow2;
	UINT_8    ucMemCol2     : 6;
	UINT_8    ucMemRow2Msb  : 2;
	UINT_8    ucMemRow3;
	UINT_8    ucMemCol3     : 6;
	UINT_8    ucMemRow3Msb  : 2;
	UINT_16   u2SmartAnt;
	UINT_8    ucSEIdx;
	UINT_8    ucAutoSoundingCtrl; /* Bit7: low traffic indicator, Bit6: Stop sounding for this entry, Bit5~0: postpone sounding */
	UINT_8    uciBfTimeOut;
	UINT_8    uciBfDBW;
	UINT_8    uciBfNcol;
	UINT_8    uciBfNrow;
	UINT_8    nr_bw160;
	UINT_8	  nc_bw160;
	UINT_8    ru_start_idx;
	UINT_8    ru_end_idx;
	BOOLEAN   trigger_su;
	BOOLEAN   trigger_mu;
	BOOLEAN   ng16_su;
	BOOLEAN   ng16_mu;
	BOOLEAN   codebook42_su;
	BOOLEAN   codebook75_mu;
	UINT_8    he_ltf;
} TXBF_PFMU_STA_INFO, *P_TXBF_PFMU_STA_INFO;

#ifdef WIFI_UNIFIED_COMMAND
typedef struct _UNICMD_TXBF_PFMU_STA_INFO_T {
	UINT_16   u2PfmuId;           /* 0xFFFF means no access right for PFMU */
	BOOLEAN   fgSU_MU;            /* 0 : SU, 1 : MU */
	UINT_8    u1TxBfCap;          /* 0 : ITxBf, 1 : ETxBf */
	UINT_8    ucSoundingPhy;      /* 0: legacy, 1: OFDM, 2: HT, 4: VHT */
	UINT_8    ucNdpaRate;
	UINT_8    ucNdpRate;
	UINT_8    ucReptPollRate;
	UINT_8    ucTxMode;           /* 0: legacy, 1: OFDM, 2: HT, 4: VHT */
	UINT_8    ucNc;
	UINT_8    ucNr;
	UINT_8    ucCBW;              /* 0 : 20M, 1 : 40M, 2 : 80M, 3 : 80 + 80M */
	UINT_8    ucTotMemRequire;
	UINT_8    ucMemRequire20M;
	UINT_8    ucMemRow0;
	UINT_8    ucMemCol0;
	UINT_8    ucMemRow1;
	UINT_8    ucMemCol1;
	UINT_8    ucMemRow2;
	UINT_8    ucMemCol2;
	UINT_8    ucMemRow3;
	UINT_8    ucMemCol3;
	UINT_16   u2SmartAnt;
	UINT_8    ucSEIdx;
	UINT_8    ucAutoSoundingCtrl; /* Bit7: low traffic indicator, Bit6: Stop sounding for this entry, Bit5~0: postpone sounding */
	UINT_8    uciBfTimeOut;
	UINT_8    uciBfDBW;
	UINT_8    uciBfNcol;
	UINT_8    uciBfNrow;
	UINT_8    nr_bw160;
	UINT_8	  nc_bw160;
	UINT_8    ru_start_idx;
	UINT_8    ru_end_idx;
	BOOLEAN   trigger_su;
	BOOLEAN   trigger_mu;
	BOOLEAN   ng16_su;
	BOOLEAN   ng16_mu;
	BOOLEAN   codebook42_su;
	BOOLEAN   codebook75_mu;
	UINT_8    he_ltf;
} UNICMD_TXBF_PFMU_STA_INFO_T, *P_UNICMD_TXBF_PFMU_STA_INFO_T;

typedef struct _UNI_CMD_STAREC_BF_T {
	UINT16 u2Tag;      /* Tag = 0x04 */
	UINT16 u2Length;
	UNICMD_TXBF_PFMU_STA_INFO_T  rTxBfPfmuInfo;
} UNI_CMD_STAREC_BF_T, *P_UNI_CMD_STAREC_BF_T;
#endif

typedef struct _BFEE_STA_REC {
	BOOLEAN    fgFbIdentityMatrix;/* 0 : Default, 1 : Enable Feedback Identity Matrix */
	BOOLEAN    fgIgnFbk; /* 0 : Default, 1 : Ignore Feedback */
} BFEE_STA_REC, *P_BFEE_STA_REC;

struct txbf_pfmu_tags_info {
	/* tag1 */
	UINT16 pfmu_idx;
	UINT8 ebf;					/* 0: ief, 1: ebf */
	UINT8 dbw;					/* 0/1/2/3 : DW20/40/80/160NC */
	UINT8 lm;
	UINT8 su_mu;				/* 0: SU, 1: MU */
	UINT8 nr;
	UINT8 nc;
	UINT8 codebook;				/* only in falcon */
	UINT8 ng;
	UINT8 invalid_prof;
	UINT8 rmsd;
	UINT16 mem_row0;
	UINT16 mem_row1;
	UINT16 mem_row2;
	UINT16 mem_row3;
	UINT8 mem_col0;
	UINT8 mem_col1;
	UINT8 mem_col2;
	UINT8 mem_col3;
	UINT8 htc;
	UINT8 ru_start;				/* only in falcon */
	UINT8 ru_end;				/* only in falcon */
	UINT8 mobility_cal_en;
	UINT8 snr_sts0;
	UINT8 snr_sts1;
	UINT8 snr_sts2;
	UINT8 snr_sts3;
	UINT8 snr_sts4;
	UINT8 snr_sts5;
	UINT8 snr_sts6;
	UINT8 snr_sts7;
	/* tag 2 */
	UINT32 smart_ant;
	UINT8 se_idx;
	UINT8 rmsd_threshold;
	UINT8 ibf_timeout;
	UINT8 ibf_desired_dbw;
	UINT8 ibf_desired_nrow;
	UINT8 ibf_desired_ncol;
	UINT8 ibf_desired_ru_alloc;	/* only in falcon */
	UINT8 mobility_delta_t;
	UINT8 mobility_lq_result;
	UINT8 reserved[3];
};

struct txbf_starec_conf {
	UINT32 conf;
	UINT_8 conf_su_mu;
	UINT_8 conf_ru_start_idx;
	UINT_8 conf_ru_end_idx;
	BOOLEAN conf_trigger_su;
	BOOLEAN conf_trigger_mu;
	BOOLEAN conf_ng16_su;
	BOOLEAN conf_ng16_mu;
	BOOLEAN conf_codebook42_su;
	BOOLEAN conf_codebook75_mu;
	UINT_8 conf_he_ltf;
	UINT_8 conf_ibf_ncol;
	UINT_8 conf_ibf_nrow;
	UINT_8 conf_nr_bw160;
	UINT_8 conf_nc_bw160;
};

typedef struct _CMD_STAREC_BF {
	UINT_16 u2Tag;      /* Tag = 0x02 */
	UINT_16 u2Length;
	TXBF_PFMU_STA_INFO	rTxBfPfmuInfo;
	UINT_8  ucReserved[2];
} CMD_STAREC_BF, *P_CMD_STAREC_BF;

typedef struct _CMD_STAREC_BFEE {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	BFEE_STA_REC rBfeeStaRec;
	UINT_8  u1Reserved[2];
} CMD_STAREC_BFEE, *P_CMD_STAREC_BFEE;

#ifdef RT_BIG_ENDIAN
typedef union _PFMU_PROFILE_TAG1 {
	struct {
		/* DWORD0 */
		UINT_32 ucMemAddr2RowIdx    : 5; /* [31 : 27] : row index : 0 ~ 63 */
		UINT_32 ucMemAddr2ColIdx    : 3; /* [26 : 24] : column index : 0 ~ 5 */
		UINT_32 ucMemAddr1RowIdx    : 6; /* [23 : 18] : row index : 0 ~ 63 */
		UINT_32 ucMemAddr1ColIdx    : 3; /* [17 : 15] : column index : 0 ~ 5 */
		UINT_32 ucRMSD              : 3; /* [14:12]   : RMSD value from CE */
		UINT_32 ucInvalidProf       : 1; /* [11]      : 0:default, 1: This profile number is invalid by SW */
		UINT_32 ucSU_MU             : 1; /* [10]      : 0:SU, 1: MU */
		UINT_32 ucDBW               : 2; /* [9:8]     : 0/1/2/3: DW20/40/80/160NC */
		UINT_32 ucTxBf              : 1; /* [7]       : 0: iBF, 1: eBF */
		UINT_32 ucProfileID         : 7; /* [6:0]     : 0 ~ 63 */

		/* DWORD1 */
		UINT_32 ucReserved1         : 1; /* [63]      : Reserved */
		UINT_32 ucHtcExist          : 1; /* [62]      : HtcExist */
		UINT_32 ucCodeBook          : 2; /* [61:60]   : Code book */
		UINT_32 ucLM                : 2; /* [59 : 58] : 0/1/2 */
		UINT_32 ucNgroup            : 2; /* [57 : 56] : Ngroup */
		UINT_32 ucNcol              : 2; /* [55 : 54] : Ncol */
		UINT_32 ucNrow              : 2; /* [53 : 52] : Nrow */
		UINT_32 ucReserved          : 1; /* [51]      : Reserved */
		UINT_32 ucMemAddr4RowIdx    : 6; /* [50 : 45] : row index : 0 ~ 63 */
		UINT_32 ucMemAddr4ColIdx    : 3; /* [44 : 42] : column index : 0 ~ 5 */
		UINT_32 ucMemAddr3RowIdx    : 6; /* [41 : 36] : row index : 0 ~ 63 */
		UINT_32 ucMemAddr3ColIdx    : 3; /* [35 : 33] : column index : 0 ~ 5 */
		UINT_32 ucMemAddr2RowIdxMsb : 1; /* [32]      : MSB of row index */

		/* DWORD2 */
		UINT_32 ucSNR_STS3          : 8; /* [95:88]   : SNR_STS3 */
		UINT_32 ucSNR_STS2          : 8; /* [87:80]   : SNR_STS2 */
		UINT_32 ucSNR_STS1          : 8; /* [79:72]   : SNR_STS1 */
		UINT_32 ucSNR_STS0          : 8; /* [71:64]   : SNR_STS0 */

		/* DWORD3 */
		UINT_32 ucReserved2         : 24; /* reserved */
		UINT_32 ucIBfLnaIdx         : 8; /* [103:96]  : iBF LNA index */
	} rField;
	UINT_32 au4RawData[4];
} PFMU_PROFILE_TAG1, *P_PFMU_PROFILE_TAG1;

typedef union _PFMU_PROFILE_TAG2 {
	struct {
		/* DWORD0 */
		UINT_32 ucMCSThS1SS      : 4; /* [31:28]  : MCS TH short 1SS */
		UINT_32 ucMCSThL1SS      : 4; /* [27:24]  : MCS TH long 1SS */
		UINT_32 ucReserved1      : 1; /* [23]     : Reserved */
		UINT_32 ucRMSDThd        : 3; /* [22:20]  : RMSD Threshold */
		UINT_32 ucSEIdx          : 5; /* [19:15]  : SE index */
		UINT_32 ucReserved0      : 3; /* [14:12]  : Reserved */
		UINT_32 u2SmartAnt       : 12;/* [11:0]   : Smart Ant config */

		/* DWORD1 */
		UINT_32 ucReserved2      : 8; /* [63:56]  : Reserved */
		UINT_32 uciBfTimeOut     : 8; /* [55:48]  : iBF timeout limit */
		UINT_32 ucMCSThS3SS      : 4; /* [47:44]  : MCS TH short 3SS */
		UINT_32 ucMCSThL3SS      : 4; /* [43:40]  : MCS TH long 3SS */
		UINT_32 ucMCSThS2SS      : 4; /* [39:36]  : MCS TH short 2SS */
		UINT_32 ucMCSThL2SS      : 4; /* [35:32]  : MCS TH long 2SS */

		/* DWORD2 */
		UINT_32 u2Reserved5      : 10;/* [95:86]  : Reserved */
		UINT_32 uciBfNrow        : 2; /* [85:84]  : iBF desired Nrow = 1 ~ 4 */
		UINT_32 uciBfNcol        : 2; /* [83:82]  : iBF desired Ncol = 1 ~ 3 */
		UINT_32 uciBfDBW         : 2; /* [81:80]  : iBF desired DBW 0/1/2/3 : BW20/40/80/160NC */
		UINT_32 ucReserved4      : 8; /* [79:72]  : Reserved */
		UINT_32 ucReserved3      : 8; /* [71:64]  : Reserved */
	} rField;
	UINT_32 au4RawData[3];
} PFMU_PROFILE_TAG2, *P_PFMU_PROFILE_TAG2;

typedef union _PFMU_PN {
	struct {
		/* DWORD0 */
		UINT_32 u2CMM_1STS_Tx2   : 10;
		UINT_32 u2CMM_1STS_Tx1   : 11;
		UINT_32 u2CMM_1STS_Tx0   : 11;

		/* DWORD1 */
		UINT_32 u2CMM_2STS_Tx1   : 9;
		UINT_32 u2CMM_2STS_Tx0   : 11;
		UINT_32 u2CMM_1STS_Tx3   : 11;
		UINT_32 u2CMM_1STS_Tx2Msb : 1;

		/* DWORD2 */
		UINT_32 u2CMM_3STS_Tx0   : 8;
		UINT_32 u2CMM_2STS_Tx3   : 11;
		UINT_32 u2CMM_2STS_Tx2   : 11;
		UINT_32 u2CMM_2STS_Tx1Msb : 2;

		/* DWORD3 */
		UINT_32 u2CMM_3STS_Tx3   : 7;
		UINT_32 u2CMM_3STS_Tx2   : 11;
		UINT_32 u2CMM_3STS_Tx1   : 11;
		UINT_32 u2CMM_3STS_Tx0Msb : 3;

		/* DWORD4 */
		UINT_32 reserved         : 28;
		UINT_32 u2CMM_3STS_Tx3Msb : 4;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN, *P_PFMU_PN;

typedef union _PFMU_PN_DBW20 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW20_1STS_Tx2   : 10;
		UINT_32 u2DBW20_1STS_Tx1   : 11;
		UINT_32 u2DBW20_1STS_Tx0   : 11;

		/* DWORD1 */
		UINT_32 u2DBW20_2STS_Tx1   : 9;
		UINT_32 u2DBW20_2STS_Tx0   : 11;
		UINT_32 u2DBW20_1STS_Tx3   : 11;
		UINT_32 u2DBW20_1STS_Tx2Msb : 1;

		/* DWORD2 */
		UINT_32 u2DBW20_3STS_Tx0   : 8;
		UINT_32 u2DBW20_2STS_Tx3   : 11;
		UINT_32 u2DBW20_2STS_Tx2   : 11;
		UINT_32 u2DBW20_2STS_Tx1Msb : 2;

		/* DWORD3 */
		UINT_32 u2DBW20_3STS_Tx3   : 7;
		UINT_32 u2DBW20_3STS_Tx2   : 11;
		UINT_32 u2DBW20_3STS_Tx1   : 11;
		UINT_32 u2DBW20_3STS_Tx0Msb : 3;

		/* DWORD4 */
		UINT_32 reserved           : 28;
		UINT_32 u2DBW20_3STS_Tx3Msb : 4;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN_DBW20M, *P_PFMU_PN_DBW20M;

typedef union _PFMU_PN_DBW40 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW40_1STS_Tx2   : 10;
		UINT_32 u2DBW40_1STS_Tx1   : 11;
		UINT_32 u2DBW40_1STS_Tx0   : 11;

		/* DWORD1 */
		UINT_32 u2DBW40_2STS_Tx1   : 9;
		UINT_32 u2DBW40_2STS_Tx0   : 11;
		UINT_32 u2DBW40_1STS_Tx3   : 11;
		UINT_32 u2DBW40_1STS_Tx2Msb : 1;

		/* DWORD2 */
		UINT_32 u2DBW40_3STS_Tx0   : 8;
		UINT_32 u2DBW40_2STS_Tx3   : 11;
		UINT_32 u2DBW40_2STS_Tx2   : 11;
		UINT_32 u2DBW40_2STS_Tx1Msb : 2;

		/* DWORD3 */
		UINT_32 u2DBW40_3STS_Tx3   : 7;
		UINT_32 u2DBW40_3STS_Tx2   : 11;
		UINT_32 u2DBW40_3STS_Tx1   : 11;
		UINT_32 u2DBW40_3STS_Tx0Msb : 3;

		/* DWORD4 */
		UINT_32 reserved           : 28;
		UINT_32 u2DBW40_3STS_Tx3Msb : 4;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN_DBW40M, *P_PFMU_PN_DBW40M;

typedef union _PFMU_PN_DBW80 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW80_1STS_Tx2   : 10;
		UINT_32 u2DBW80_1STS_Tx1   : 11;
		UINT_32 u2DBW80_1STS_Tx0   : 11;

		/* DWORD1 */
		UINT_32 u2DBW80_2STS_Tx1   : 9;
		UINT_32 u2DBW80_2STS_Tx0   : 11;
		UINT_32 u2DBW80_1STS_Tx3   : 11;
		UINT_32 u2DBW80_1STS_Tx2Msb : 1;

		/* DWORD2 */
		UINT_32 u2DBW80_3STS_Tx0   : 8;
		UINT_32 u2DBW80_2STS_Tx3   : 11;
		UINT_32 u2DBW80_2STS_Tx2   : 11;
		UINT_32 u2DBW80_2STS_Tx1Msb : 2;

		/* DWORD3 */
		UINT_32 u2DBW80_3STS_Tx3   : 7;
		UINT_32 u2DBW80_3STS_Tx2   : 11;
		UINT_32 u2DBW80_3STS_Tx1   : 11;
		UINT_32 u2DBW80_3STS_Tx0Msb : 3;

		/* DWORD4 */
		UINT_32 reserved           : 28;
		UINT_32 u2DBW80_3STS_Tx3Msb : 4;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN_DBW80M, *P_PFMU_PN_DBW80M;

typedef union _PFMU_PN_DBW80_80 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW160_2STS_Tx0  : 10;
		UINT_32 u2DBW160_1STS_Tx1  : 11;
		UINT_32 u2DBW160_1STS_Tx0  : 11;

		/* DWORD1 */
		UINT_32 reserved           : 20;
		UINT_32 u2DBW160_2STS_Tx1  : 11;
		UINT_32 u2DBW160_2STS_Tx0Msb : 1;
	} rField;
	UINT_32 au4RawData[2];
} PFMU_PN_DBW80_80M, *P_PFMU_PN_DBW80_80M;

typedef union _PFMU_DATA {
	struct {
		/* DWORD0 */
		UINT_32 ucPsi31          : 7;
		UINT_32 u2Phi21          : 9;
		UINT_32 ucPsi21          : 7;
		UINT_32 u2Phi11          : 9;

		/* DWORD1 */
		UINT_32 ucPsi32          : 7;
		UINT_32 u2Phi22          : 9;
		UINT_32 ucPsi41          : 7;
		UINT_32 u2Phi31          : 9;

		/* DWORD2 */
		UINT_32 ucPsi43          : 7;
		UINT_32 u2Phi33          : 9;
		UINT_32 ucPsi42          : 7;
		UINT_32 u2Phi32          : 9;

		/* DWORD3 */
		UINT_32 u2Reserved       : 16;
		UINT_32 u2dSNR03         : 4;
		UINT_32 u2dSNR02         : 4;
		UINT_32 u2dSNR01         : 4;
		UINT_32 u2dSNR00         : 4;

		/* DWORD4 */
		UINT_32 u4Reserved;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_DATA, *P_PFMU_DATA;

#else

typedef union _PFMU_PROFILE_TAG1 {
	struct {
		/* DWORD0 */
		UINT_32 ucProfileID         : 7; /* [6:0]     : 0 ~ 63 */
		UINT_32 ucTxBf              : 1; /* [7]       : 0: iBF, 1: eBF */
		UINT_32 ucDBW               : 2; /* [9:8]     : 0/1/2/3: DW20/40/80/160NC */
		UINT_32 ucSU_MU             : 1; /* [10]      : 0:SU, 1: MU */
		UINT_32 ucInvalidProf       : 1; /* [11]      : 0:default, 1: This profile number is invalid by SW */
		UINT_32 ucRMSD              : 3; /* [14:12]   : RMSD value from CE */
		UINT_32 ucMemAddr1ColIdx    : 3; /* [17 : 15] : column index : 0 ~ 5 */
		UINT_32 ucMemAddr1RowIdx    : 6; /* [23 : 18] : row index : 0 ~ 63 */
		UINT_32 ucMemAddr2ColIdx    : 3; /* [26 : 24] : column index : 0 ~ 5 */
		UINT_32 ucMemAddr2RowIdx    : 5; /* [31 : 27] : row index : 0 ~ 63 */
		UINT_32 ucMemAddr2RowIdxMsb : 1; /* [32]      : MSB of row index */
		UINT_32 ucMemAddr3ColIdx    : 3; /* [35 : 33] : column index : 0 ~ 5 */
		UINT_32 ucMemAddr3RowIdx    : 6; /* [41 : 36] : row index : 0 ~ 63 */
		UINT_32 ucMemAddr4ColIdx    : 3; /* [44 : 42] : column index : 0 ~ 5 */
		UINT_32 ucMemAddr4RowIdx    : 6; /* [50 : 45] : row index : 0 ~ 63 */
		UINT_32 ucReserved          : 1; /* [51]      : Reserved */
		UINT_32 ucNrow              : 2; /* [53 : 52] : Nrow */
		UINT_32 ucNcol              : 2; /* [55 : 54] : Ncol */
		UINT_32 ucNgroup            : 2; /* [57 : 56] : Ngroup */
		UINT_32 ucLM                : 2; /* [59 : 58] : 0/1/2 */
		UINT_32 ucCodeBook          : 2; /* [61:60]   : Code book */
		UINT_32 ucHtcExist          : 1; /* [62]      : HtcExist */
		UINT_32 ucReserved1         : 1; /* [63]      : Reserved */
		/* DWORD2 */
		UINT_32 ucSNR_STS0          : 8; /* [71:64]   : SNR_STS0 */
		UINT_32 ucSNR_STS1          : 8; /* [79:72]   : SNR_STS1 */
		UINT_32 ucSNR_STS2          : 8; /* [87:80]   : SNR_STS2 */
		UINT_32 ucSNR_STS3          : 8; /* [95:88]   : SNR_STS3 */
		/* DWORD3 */
		UINT_32 ucIBfLnaIdx         : 8; /* [103:96]  : iBF LNA index */
		UINT_32 ucReserved2         : 24; /*     : Reserved */
	} rField;
	UINT_32 au4RawData[4];
} PFMU_PROFILE_TAG1, *P_PFMU_PROFILE_TAG1;

typedef union _PFMU_PROFILE_TAG2 {
	struct {
		/* DWORD0 */
		UINT_32 u2SmartAnt       : 12;/* [11:0]   : Smart Ant config */
		UINT_32 ucReserved0      : 3; /* [14:12]  : Reserved */
		UINT_32 ucSEIdx          : 5; /* [19:15]  : SE index */
		UINT_32 ucRMSDThd        : 3; /* [22:20]  : RMSD Threshold */
		UINT_32 ucReserved1      : 1; /* [23]     : Reserved */
		UINT_32 ucMCSThL1SS      : 4; /* [27:24]  : MCS TH long 1SS */
		UINT_32 ucMCSThS1SS      : 4; /* [31:28]  : MCS TH short 1SS */
		/* DWORD1 */
		UINT_32 ucMCSThL2SS      : 4; /* [35:32]  : MCS TH long 2SS */
		UINT_32 ucMCSThS2SS      : 4; /* [39:36]  : MCS TH short 2SS */
		UINT_32 ucMCSThL3SS      : 4; /* [43:40]  : MCS TH long 3SS */
		UINT_32 ucMCSThS3SS      : 4; /* [47:44]  : MCS TH short 3SS */
		UINT_32 uciBfTimeOut     : 8; /* [55:48]  : iBF timeout limit */
		UINT_32 ucReserved2      : 8; /* [63:56]  : Reserved */
		/* DWORD2 */
		UINT_32 ucReserved3      : 8; /* [71:64]  : Reserved */
		UINT_32 ucReserved4      : 8; /* [79:72]  : Reserved */
		UINT_32 uciBfDBW         : 2; /* [81:80]  : iBF desired DBW 0/1/2/3 : BW20/40/80/160NC */
		UINT_32 uciBfNcol        : 2; /* [83:82]  : iBF desired Ncol = 1 ~ 3 */
		UINT_32 uciBfNrow        : 2; /* [85:84]  : iBF desired Nrow = 1 ~ 4 */
		UINT_32 u2Reserved5      : 10;/* [95:86]  : Reserved */
	} rField;
	UINT_32 au4RawData[3];
} PFMU_PROFILE_TAG2, *P_PFMU_PROFILE_TAG2;

typedef union _PFMU_PN {
	struct {
		/* DWORD0 */
		UINT_32 u2CMM_1STS_Tx0   : 11;
		UINT_32 u2CMM_1STS_Tx1   : 11;
		UINT_32 u2CMM_1STS_Tx2   : 10;
		/* DWORD1 */
		UINT_32 u2CMM_1STS_Tx2Msb : 1;
		UINT_32 u2CMM_1STS_Tx3   : 11;
		UINT_32 u2CMM_2STS_Tx0   : 11;
		UINT_32 u2CMM_2STS_Tx1   : 9;
		/* DWORD2 */
		UINT_32 u2CMM_2STS_Tx1Msb : 2;
		UINT_32 u2CMM_2STS_Tx2   : 11;
		UINT_32 u2CMM_2STS_Tx3   : 11;
		UINT_32 u2CMM_3STS_Tx0   : 8;
		/* DWORD3 */
		UINT_32 u2CMM_3STS_Tx0Msb : 3;
		UINT_32 u2CMM_3STS_Tx1   : 11;
		UINT_32 u2CMM_3STS_Tx2   : 11;
		UINT_32 u2CMM_3STS_Tx3   : 7;
		/* DWORD4 */
		UINT_32 u2CMM_3STS_Tx3Msb : 4;
		UINT_32 reserved         : 28;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN, *P_PFMU_PN;

typedef union _PFMU_PN_DBW20 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW20_1STS_Tx0   : 11;
		UINT_32 u2DBW20_1STS_Tx1   : 11;
		UINT_32 u2DBW20_1STS_Tx2   : 10;
		/* DWORD1 */
		UINT_32 u2DBW20_1STS_Tx2Msb : 1;
		UINT_32 u2DBW20_1STS_Tx3   : 11;
		UINT_32 u2DBW20_2STS_Tx0   : 11;
		UINT_32 u2DBW20_2STS_Tx1   : 9;
		/* DWORD2 */
		UINT_32 u2DBW20_2STS_Tx1Msb : 2;
		UINT_32 u2DBW20_2STS_Tx2   : 11;
		UINT_32 u2DBW20_2STS_Tx3   : 11;
		UINT_32 u2DBW20_3STS_Tx0   : 8;
		/* DWORD3 */
		UINT_32 u2DBW20_3STS_Tx0Msb : 3;
		UINT_32 u2DBW20_3STS_Tx1   : 11;
		UINT_32 u2DBW20_3STS_Tx2   : 11;
		UINT_32 u2DBW20_3STS_Tx3   : 7;
		/* DWORD4 */
		UINT_32 u2DBW20_3STS_Tx3Msb : 4;
		UINT_32 reserved           : 28;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN_DBW20M, *P_PFMU_PN_DBW20M;

typedef union _PFMU_PN_DBW40 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW40_1STS_Tx0   : 11;
		UINT_32 u2DBW40_1STS_Tx1   : 11;
		UINT_32 u2DBW40_1STS_Tx2   : 10;
		/* DWORD1 */
		UINT_32 u2DBW40_1STS_Tx2Msb : 1;
		UINT_32 u2DBW40_1STS_Tx3   : 11;
		UINT_32 u2DBW40_2STS_Tx0   : 11;
		UINT_32 u2DBW40_2STS_Tx1   : 9;
		/* DWORD2 */
		UINT_32 u2DBW40_2STS_Tx1Msb : 2;
		UINT_32 u2DBW40_2STS_Tx2   : 11;
		UINT_32 u2DBW40_2STS_Tx3   : 11;
		UINT_32 u2DBW40_3STS_Tx0   : 8;
		/* DWORD3 */
		UINT_32 u2DBW40_3STS_Tx0Msb : 3;
		UINT_32 u2DBW40_3STS_Tx1   : 11;
		UINT_32 u2DBW40_3STS_Tx2   : 11;
		UINT_32 u2DBW40_3STS_Tx3   : 7;
		/* DWORD4 */
		UINT_32 u2DBW40_3STS_Tx3Msb : 4;
		UINT_32 reserved           : 28;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN_DBW40M, *P_PFMU_PN_DBW40M;

typedef union _PFMU_PN_DBW80 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW80_1STS_Tx0   : 11;
		UINT_32 u2DBW80_1STS_Tx1   : 11;
		UINT_32 u2DBW80_1STS_Tx2   : 10;
		/* DWORD1 */
		UINT_32 u2DBW80_1STS_Tx2Msb : 1;
		UINT_32 u2DBW80_1STS_Tx3   : 11;
		UINT_32 u2DBW80_2STS_Tx0   : 11;
		UINT_32 u2DBW80_2STS_Tx1   : 9;
		/* DWORD2 */
		UINT_32 u2DBW80_2STS_Tx1Msb : 2;
		UINT_32 u2DBW80_2STS_Tx2   : 11;
		UINT_32 u2DBW80_2STS_Tx3   : 11;
		UINT_32 u2DBW80_3STS_Tx0   : 8;
		/* DWORD3 */
		UINT_32 u2DBW80_3STS_Tx0Msb : 3;
		UINT_32 u2DBW80_3STS_Tx1   : 11;
		UINT_32 u2DBW80_3STS_Tx2   : 11;
		UINT_32 u2DBW80_3STS_Tx3   : 7;
		/* DWORD4 */
		UINT_32 u2DBW80_3STS_Tx3Msb : 4;
		UINT_32 reserved           : 28;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN_DBW80M, *P_PFMU_PN_DBW80M;

typedef union _PFMU_PN_DBW80_80 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW160_1STS_Tx0  : 11;
		UINT_32 u2DBW160_1STS_Tx1  : 11;
		UINT_32 u2DBW160_2STS_Tx0  : 10;
		/* DWORD1 */
		UINT_32 u2DBW160_2STS_Tx0Msb : 1;
		UINT_32 u2DBW160_2STS_Tx1  : 11;
		UINT_32 reserved           : 20;
	} rField;
	UINT_32 au4RawData[2];
} PFMU_PN_DBW80_80M, *P_PFMU_PN_DBW80_80M;

typedef union _PFMU_DATA {
	struct {
		/* DWORD0 */
		UINT_32 u2Phi11          : 9;
		UINT_32 ucPsi21          : 7;
		UINT_32 u2Phi21          : 9;
		UINT_32 ucPsi31          : 7;
		/* DWORD1 */
		UINT_32 u2Phi31          : 9;
		UINT_32 ucPsi41          : 7;
		UINT_32 u2Phi22          : 9;
		UINT_32 ucPsi32          : 7;
		/* DWORD2 */
		UINT_32 u2Phi32          : 9;
		UINT_32 ucPsi42          : 7;
		UINT_32 u2Phi33          : 9;
		UINT_32 ucPsi43          : 7;
		/* DWORD3 */
		UINT_32 u2dSNR00         : 4;
		UINT_32 u2dSNR01         : 4;
		UINT_32 u2dSNR02         : 4;
		UINT_32 u2dSNR03         : 4;
		UINT_32 u2Reserved       : 16;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_DATA, *P_PFMU_DATA;
#endif

typedef struct _PFMU_HALF_DATA {
	UINT_16 u2SubCarrIdx;
	INT_16  i2Phi11;
	INT_16  i2Phi21;
	INT_16  i2Phi31;
} PFMU_HALF_DATA, *P_PFMU_HALF_DATA;

typedef struct _BF_QD {
	UINT_32 u4qd[14];
} BF_QD, *P_BF_QD;

typedef struct _BF_SND_FBRPT_STATISTICS_T {
	UINT_32 u4PFMUWRTimeOutCnt;
	UINT_32 u4PFMUWRFailCnt;
	UINT_32 u4PFMUWRDoneCnt;
	UINT_32 u4PFMUWRTimeoutFreeCnt;
	UINT_32 u4FbRptPktDropCnt; /* Parsing failed and over-queue. */
	UINT_32 u4RxPerStaFbRptCnt[MAX_LEN_OF_MAC_TABLE];
} BF_SND_FBRPT_STATISTICS_T, *P_BF_SND_FBRPT_STATISTICS_T;

typedef struct _BF_SND_STA_INFO_T {
	UINT_8 u1SndIntv;               /* Sounding interval upper bound, unit : 15ms */
	UINT_8 u1SndCntDn;              /* Sounding interval count down */
	UINT_8 u1SndTxCnt;              /* Tx sounding count for debug */
	UINT_8 u1SndStopReason;         /* Bitwise reason about why it is put in Stop Queue */
} BF_SND_STA_INFO_T, *P_BF_SND_STA_INFO_T;

typedef struct _BF_SND_CFG_T {
	UINT_8 u1VhtOpt;
	UINT_8 u1HeOpt;
	UINT_8 u1GloOpt;
	UINT_8 reserved;
} BF_SND_CFG_T, *P_BF_SND_CFG_T;

typedef struct _BF_SND_CNT_CFG_T
{
    UINT_16 u2SndCntLmt;
    UINT_16 u2SndCntLmtMan;
    UINT_16 u2SndCnt[DBDC_BAND_NUM];
    UINT_8  u1SndCndCondi;
    UINT_8  u1Reserved[3];
} BF_SND_CNT_CFG_T, *P_BF_SND_CNT_CFG_T;

typedef struct _BF_SND_TRG_INFO_T {
	UINT_16 u2ULLength;
	UINT_8 u1Mcs;
	UINT_8 u1LDPC;
	UINT_8 reserved[12];
} BF_SND_TRG_INFO_T, *P_BF_SND_TRG_INFO_T;

typedef struct _BF_SND_CTRL_EVT_T {
	BF_SND_CFG_T rSndCfg;
	UINT_32 au4SndRecSuSta[MAX_LEN_OF_MAC_TABLE / 32];
	UINT_32 au4SndRecVhtMuSta[MAX_LEN_OF_MAC_TABLE / 32];
	UINT_32 au4SndRecHeTBSta[MAX_LEN_OF_MAC_TABLE / 32];
	UINT_16 u2WlanIdxForMcSnd[DBDC_BAND_NUM];
	UINT_16 u2WlanIdxForTbSnd[DBDC_BAND_NUM];
	BF_SND_TRG_INFO_T rSndTrgInfo;
	BF_SND_STA_INFO_T arSndStaInfo[MAX_LEN_OF_MAC_TABLE];
} BF_SND_CTRL_EVT_T, *P_BF_SND_CTRL_EVT_T;

typedef struct _BF_PLY_CFG_T {
	UINT_8 u1GloOpt;
	UINT_8 u1GrpIBfOpt;
	UINT_8 u1GrpEBfOpt;
	UINT_8 reserved;
} BF_PLY_CFG_T, *P_BF_PLY_CFG_T;

typedef struct _BF_PLY_NSS_T {
	UINT_8 u1SSGrp: 4;
	UINT_8 u1SSPly: 4;
} BF_PLY_NSS_T, *P_BF_PLY_NSS_T;

typedef struct _BF_PLY_RLT_T {
	UINT_8 u1CurrRlt;
} BF_PLY_RLT_T, *P_BF_PLY_RLT_T;

typedef struct _BF_PLY_MGMT_EVT_T {
	BF_PLY_CFG_T rPlyOpt;
	BF_PLY_NSS_T arStaSS[MAX_LEN_OF_MAC_TABLE][4];
	BF_PLY_RLT_T arStaRlt[MAX_LEN_OF_MAC_TABLE];
} BF_PLY_MGMT_EVT_T, *P_BF_PLY_MGMT_EVT_T;

typedef enum _BF_PLY_RULE {
	BF_PLY_RULE_NONBF = 0,
	BF_PLY_RULE_EBF = 1,
	BF_PLY_RULE_IBF = 2,
	BF_PLY_RULE_EBF_THEN_IBF = 3,
	BF_PLY_RULE_IBF_THEN_EBF = 4
} BF_PLY_RULE;

typedef enum _BF_PLY_GRP {
	BF_PLY_NON = 0,         /* initial state - no policy */
	BF_PLY_SSS = 1,
	BF_PLY_SSE = 2,
	BF_PLY_SSL = 3,
	BF_PLY_SEE = 4,
	BF_PLY_SEL = 5,
	BF_PLY_SLL = 6,
	BF_PLY_ESE = 7,
	BF_PLY_ESL = 8,
	BF_PLY_EEE = 9,
	BF_PLY_EEL = 10,
	BF_PLY_ELL = 11,
	BF_PLY_ZERO_SNR = 12,   /* Policy for Feedback Report SNR=0 */
	BF_PLY_MAN = 13,        /* Manual Policy */
	BF_PLY_ERR = 14         /* Policy error */
} BF_PLY_GRP;

typedef struct _WH_HERA_METRIC_RPT {
	UINT_8 u1BPSK;
	UINT_8 u1QPSK;
	UINT_8 u116QAM;
	UINT_8 u164QAM;
	UINT_8 u1256QAM;
	UINT_8 u11024QAM;
	UINT_8 u1Capacity;
	UINT_8 reserved;
} WH_HERA_METRIC_RPT, *P_WH_HERA_METRIC_RPT;

typedef struct _HERA_MU_METRIC_CMD_RPT {
	UINT_8 u1CurState;          /* Current State - HERA_METRIC_DESC */
	UINT_8 u1RunningFailCnt;
	UINT_8 u1ErrRptCnt;
	UINT_8 u1FreeReqCnt;
	UINT_8 u1PendingReqCnt;
	UINT_8 u1PollingTime;
	UINT_8 u1NUser;
	BOOLEAN fgIsLQErr;
	UINT_16 u2LQErr;
	UINT_16 reserved;
	WH_HERA_METRIC_RPT rMetricRpt[4];
	UINT_8 u1InitMCSUser[4];
} HERA_MU_METRIC_CMD_RPT, *P_HERA_MU_METRIC_CMD_RPT;

typedef struct _BF_TXCMD_CFG_EVT_T {
	BOOLEAN fgTxCmdBfManual;
	UINT_8 ucTxCmdBfBit;
} BF_TXCMD_CFG_EVT_T, *P_BF_TXCMD_CFG_EVT_T;

typedef enum _BF_SOUNDING_MODE {
	SU_SOUNDING = 0,
	MU_SOUNDING,
	SU_PERIODIC_SOUNDING,
	MU_PERIODIC_SOUNDING,
	BF_PROCESSING,
	TXCMD_NONTB_SU_SOUNDING,
	TXCMD_VHT_MU_SOUNDING,
	TXCMD_TB_PER_BRP_SOUNDING,
	TXCMD_TB_SOUNDING,
	SOUNDING_MAX
} BF_SOUNDING_MODE;
#endif /* #ifdef MT_MAC */

typedef enum _BF_ACTION_CATEGORY {
	BF_SOUNDING_OFF = 0,
	BF_SOUNDING_ON,
	BF_DATA_PACKET_APPLY,
	BF_PFMU_MEM_ALLOCATE,
	BF_PFMU_MEM_RELEASE,
	BF_PFMU_TAG_READ,
	BF_PFMU_TAG_WRITE,
	BF_PROFILE_READ,
	BF_PROFILE_WRITE,
	BF_PN_READ,
	BF_PN_WRITE,
	BF_PFMU_MEM_ALLOC_MAP_READ,
	BF_AID_SET,
	BF_STA_REC_READ,
	BF_PHASE_CALIBRATION,
	BF_IBF_PHASE_COMP,
	BF_LNA_GAIN_CONFIG,
	BF_PROFILE_WRITE_20M_ALL,
	BF_APCLIENT_CLUSTER,
	BF_AWARE_CTRL,
	BF_HW_ENABLE_STATUS_UPDATE,
	BF_REPT_CLONED_STA_TO_NORMAL_STA,
	BF_GET_QD,
	BF_BFEE_HW_CTRL,
	BF_PFMU_SW_TAG_WRITE,
	BF_MOD_EN_CTRL,
	BF_DYNSND_EN_INTR,
	BF_DYNSND_CFG_DMCS_TH,
	BF_DYNSND_EN_PFID_INTR,
	BF_CONFIG,
	BF_PFMU_DATA_WRITE,
	BF_FBRPT_DBG_INFO_READ,
	BF_CMD_TXSND_INFO,
	BF_CMD_PLY_INFO,
	BF_CMD_MU_METRIC,
	BF_CMD_TXCMD,
	BF_CMD_CFG_PHY,
	BF_CMD_SND_CNT,
	BF_CMD_MAX
} BF_ACTION_CATEGORY;

/* Tx BF customized configuration types*/
typedef enum _BF_CONFIG_TYPE {
	BF_CONFIG_TYPE_STOP_REPORT_POLL = 0,
	BF_CONFIG_TYPE_MAX
} BF_CONFIG_TYPE;

/* Indices of valid rows in Implicit and Explicit profiles for 20 and 40 MHz */
typedef struct {
	int lwb1, upb1;
	int lwb2, upb2;
} SC_TABLE_ENTRY;

typedef
struct {
	UCHAR E1gBeg;
	UCHAR E1gEnd;
	UCHAR E1aHighBeg;
	UCHAR E1aHighEnd;
	UCHAR E1aLowBeg;
	UCHAR E1aLowEnd;
	UCHAR E1aMidBeg;
	UCHAR E1aMidMid;
	UCHAR E1aMidEnd;
} ITXBF_PHASE_PARAMS;			/* ITxBF BBP reg phase calibration parameters */

typedef
struct {
	UCHAR E1gBeg[3];
	UCHAR E1gEnd[3];
	UCHAR E1aHighBeg[3];
	UCHAR E1aHighEnd[3];
	UCHAR E1aLowBeg[3];
	UCHAR E1aLowEnd[3];
	UCHAR E1aMidBeg[3];
	UCHAR E1aMidMid[3];
	UCHAR E1aMidEnd[3];
} ITXBF_LNA_PARAMS;			/* ITxBF BBP reg LNA calibration parameters */

typedef
struct {
	UCHAR E1gBeg;
	UCHAR E1gEnd;
	UCHAR E1aHighBeg;
	UCHAR E1aHighEnd;
	UCHAR E1aLowBeg;
	UCHAR E1aLowEnd;
	UCHAR E1aMidBeg;
	UCHAR E1aMidMid;
	UCHAR E1aMidEnd;
} ITXBF_DIV_PARAMS;				/* ITxBF Divider Calibration parameters */


typedef struct _TXBF_STATUS_INFO {
	UCHAR               ucPhyMode;
	UCHAR               ucBW;
	USHORT              u2Channel;
	UCHAR               ucTxPathNum;
	UCHAR               ucRxPathNum;
	UCHAR               ucETxBfTxEn;
	UCHAR               ucITxBfTxEn;
	UCHAR               ucNDPARate;
	UCHAR               ucNDPRate;
	UINT16              u2Wcid;
	UINT32              u4WTBL1;
	UINT32              u4WTBL2;
	HT_BF_CAP           *pHtTxBFCap;
#ifdef VHT_TXBF_SUPPORT
	struct _VHT_CAP_INFO *pVhtTxBFCap;
#endif
#ifdef HE_TXBF_SUPPORT
	struct he_bf_info *he_bf_info;
#endif
	ULONG               cmmCfgITxBfTimeout;
	ULONG               cmmCfgETxBfTimeout;
	ULONG	            cmmCfgETxBfEnCond;		/* Enable sending of sounding and beamforming */
	BOOLEAN	            cmmCfgETxBfNoncompress;	/* Force non-compressed Sounding Response */
	BOOLEAN	            cmmCfgETxBfIncapable;		/* Report Incapable of BF in TX BF Capabilities */

} TXBF_STATUS_INFO;

typedef struct {
	UCHAR		eTxBfEnCond;
	UCHAR		iTxBfEn;
} TXBF_MAC_TABLE_ENTRY;

VOID mt_TxBFInit(
	IN struct _RTMP_ADAPTER   *pAd,
	IN TXBF_STATUS_INFO * pTxBfInfo,
	IN TXBF_MAC_TABLE_ENTRY * pEntryTxBf,
	IN BOOLEAN			      supportsETxBF);

VOID mt_TxBFFwInit(
	IN struct _RTMP_ADAPTER  *pAd);

VOID mt_WrapTxBFInit(
	IN struct _RTMP_ADAPTER  *pAd,
	IN struct _MAC_TABLE_ENTRY	*pEntry,
	IN IE_LISTS * ie_list,
	IN BOOLEAN			     supportsETxBF);

BOOLEAN mt_clientSupportsETxBF(
	IN struct _RTMP_ADAPTER   *pAd,
	IN HT_BF_CAP              *pTxBFCap,
	IN BOOLEAN                ETxBfNoncompress);

BOOLEAN mt_WrapClientSupportsETxBF(
	IN struct _RTMP_ADAPTER  *pAd,
	IN HT_BF_CAP             *pTxBFCap);

void mt_WrapSetETxBFCap(
	IN struct _RTMP_ADAPTER  *pAd,
	IN struct wifi_dev *wdev,
	IN HT_BF_CAP       *pTxBFCap);

#ifdef VHT_TXBF_SUPPORT
BOOLEAN mt_clientSupportsVhtETxBF(
	IN struct _RTMP_ADAPTER   *pAd,
	IN VHT_CAP_INFO * pTxBFCap);

BOOLEAN mt_WrapClientSupportsVhtETxBF(
	IN struct _RTMP_ADAPTER  *pAd,
	IN VHT_CAP_INFO * pTxBFCap);

void mt_WrapSetVHTETxBFCap(
	IN struct _RTMP_ADAPTER  *pAd,
	IN struct wifi_dev *wdev,
	IN struct _VHT_CAP_INFO  *pTxBFCap);
#endif /* VHT_TXBF_SUPPORT */

#ifdef HE_TXBF_SUPPORT
BOOLEAN txbf_peer_he_bfee_cap(
	struct he_bf_info *he_bf_struct);

void mt_wrap_get_he_bf_cap(
	struct wifi_dev *wdev,
	struct he_bf_info *he_bf_struct);
#endif

void mt_WrapIBfCalGetEBfMemAlloc(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  PCHAR pPfmuMemRow,
	IN  PCHAR pPfmuMemCol);

void mt_WrapIBfCalGetIBfMemAlloc(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  PCHAR pPfmuMemRow,
	IN  PCHAR pPfmuMemCol);

VOID TxBfProfileTag_PfmuIdx(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN UCHAR ucProfileID);

VOID TxBfProfileTag_TxBfType(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN UCHAR ucTxBf);

VOID TxBfProfileTag_DBW(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN UCHAR ucBw);

VOID TxBfProfileTag_SuMu(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN UCHAR ucSuMu);

VOID TxBfProfileTag_InValid(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN UCHAR InvalidFlg);

VOID TxBfProfileTag_Mem(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN PUCHAR pMemAddrColIdx,
	IN PUCHAR pMemAddrRowIdx);

VOID TxBfProfileTag_Matrix(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN UCHAR ucNrow,
	IN UCHAR ucNcol,
	IN UCHAR ucNgroup,
	IN UCHAR ucLM,
	IN UCHAR ucCodeBook,
	IN UCHAR ucHtcExist);

VOID TxBfProfileTag_SNR(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
#endif
	IN UCHAR ucSNR_STS0,
	IN UCHAR ucSNR_STS1,
	IN UCHAR ucSNR_STS2,
	IN UCHAR ucSNR_STS3);

VOID TxBfProfileTag_SmtAnt(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif
	IN USHORT u2SmartAnt);

VOID TxBfProfileTag_SeIdx(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif
	IN UCHAR ucSEIdx);

VOID TxBfProfileTag_RmsdThd(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif
	IN UCHAR ucRMSDThd);

VOID TxBfProfileTag_McsThd(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif
	IN PUCHAR pMCSThLSS,
	IN PUCHAR pMCSThSSS);

VOID TxBfProfileTag_TimeOut(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif
	IN UCHAR uciBfTimeOut);

VOID TxBfProfileTag_DesiredBW(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif
	IN UCHAR uciBfDBW);

VOID TxBfProfileTag_DesiredNc(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif
	IN UCHAR uciBfNcol);

VOID TxBfProfileTag_DesiredNr(
#if defined(DOT11_HE_AX)
	IN struct txbf_pfmu_tags_info *pfmu_tag_info,
#else
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
#endif
	IN UCHAR uciBfNrow);

INT TxBfProfileTagRead(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR		        PfmuIdx,
	IN BOOLEAN              fgBFer);

INT TxBfProfileTagWrite(
	IN struct _RTMP_ADAPTER *pAd,
	IN P_PFMU_PROFILE_TAG1  prPfmuTag1,
	IN P_PFMU_PROFILE_TAG2  prPfmuTag2,
	IN UCHAR		        profileIdx);

#ifdef CONFIG_ATE
BOOLEAN TxBfProfileDataFormatTranslate(
	IN struct _RTMP_ADAPTER *pAd,
	IN PUCHAR               pucDataIn,
	IN P_PFMU_HALF_DATA     pPfmuHalfData);

VOID TxBf_Status_Update(
	IN struct _RTMP_ADAPTER *pAd,
	IN PUCHAR               data,
	IN UINT32 len);
#endif

VOID TxBfProfileTagPrint(
	IN struct _RTMP_ADAPTER *pAd,
	IN BOOLEAN              fgBFer,
	IN PUCHAR               pBuf);

INT TxBfProfilePnRead(
	IN  struct _RTMP_ADAPTER *pAd,
	IN UCHAR		         profileIdx);

INT TxBfProfilePnWrite(
	IN  struct _RTMP_ADAPTER *pAd,
	IN UCHAR                PfmuIdx,
	IN UCHAR                ucBw,
	IN PUCHAR		        pProfileData);

VOID TxBfProfilePnPrint(
	IN UCHAR                 ucBw,
	IN PUCHAR                pBuf);

INT TxBfProfileDataRead(
	IN  struct _RTMP_ADAPTER *pAd,
	IN UCHAR		         profileIdx,
	IN BOOLEAN               fgBFer,
	IN USHORT                subCarrIdx);

INT TxBfProfileDataWrite(
	IN  struct _RTMP_ADAPTER *pAd,
	IN PUSHORT Input);

INT TxBfProfileDataWrite20MAll(
	IN  struct _RTMP_ADAPTER *pAd,
	IN UCHAR                 profileIdx,
	IN PUCHAR                pProfileData);

VOID TxBfProfileDataPrint(
	IN  struct _RTMP_ADAPTER *pAd,
	IN USHORT                 subCarrIdx,
	IN PUCHAR                 pBuf);

INT TxBfQdRead(
	IN  struct _RTMP_ADAPTER *pAd,
	IN INT8                  subCarrIdx);

INT TxBfFbRptDbgInfo(
	IN  struct _RTMP_ADAPTER *pAd,
	IN PUINT8 pucData);

VOID TxBfFbRptDbgInfoPrint(
	IN struct _RTMP_ADAPTER *pAd,
	IN PUINT8 pucBuf);

INT TxBfTxSndInfo(
	IN struct _RTMP_ADAPTER *pAd,
	IN PUINT8 pucData);

INT TxBfPlyInfo(
	IN struct _RTMP_ADAPTER *pAd,
	IN PUINT8 pucData);

INT TxBfTxCmd(
	IN struct _RTMP_ADAPTER *pAd,
	IN PUINT8 pucData);

INT TxBfSndCnt(
	IN struct _RTMP_ADAPTER *pAd,
	IN PUINT8 pucData);

INT HeRaMuMetricInfo(
	IN struct _RTMP_ADAPTER *pAd,
	IN PUINT8 pucData);

VOID TxBfTxSndInfoPrint(
	IN struct _RTMP_ADAPTER *pAd,
	IN PUINT8 pucBuf);

VOID TxBfPlyInfoPrint(
	IN struct _RTMP_ADAPTER *pAd,
	IN PUINT8 pucBuf);

VOID TxBfSndCntInfoPrint(
	IN struct _RTMP_ADAPTER *pAd,
	IN PUINT8 pucBuf);

VOID HeRaMuMetricInfoPrint(
	IN struct _RTMP_ADAPTER *pAd,
	IN PUINT8 pucBuf);

VOID TxBfTxCmdCfgInfoPrint(
	IN struct _RTMP_ADAPTER *pAd,
	IN PUINT8 pucBuf);

INT mt_Trigger_Sounding_Packet(
	IN  struct _RTMP_ADAPTER *pAd,
	IN    UCHAR              SndgEn,
	IN    UINT32             u4SNDPeriod,
	IN    UCHAR              ucSu_Mu,
	IN    UCHAR              ucMuNum,
	IN    PUCHAR             pWlanId);

VOID TxBfProfileMemAllocMap(
	IN PUCHAR                pBuf);

BOOLEAN TxBfModuleEnCtrl(
	IN  struct _RTMP_ADAPTER *pAd);

BOOLEAN TxBfCfgBfPhy(
	IN struct _RTMP_ADAPTER *pAd);

INT txbf_config(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  UINT8                 config_type,
	IN  UINT8                 config_para[]);

VOID txbf_dyn_mech(
	IN  struct _RTMP_ADAPTER *pAd);

BOOLEAN txbf_bfee_adaption(
	IN  struct _RTMP_ADAPTER *pAd);

BOOLEAN has_rtk_sta_bfer(
	IN  struct _RTMP_ADAPTER *pAd);

#ifdef TXBF_DYNAMIC_DISABLE
INT DynamicTxBfDisable(
	IN  struct _RTMP_ADAPTER *pAd,
	IN BOOLEAN               fgDisable);
#endif /* TXBF_DYNAMIC_DISABLE */

VOID StaRecBfUpdate(
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN P_CMD_STAREC_BF        pCmdStaRecBf);

#ifdef WIFI_UNIFIED_COMMAND
VOID UniCmdStaRecBfUpdate(
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN P_UNI_CMD_STAREC_BF_T pCmdStaRecBf);
#endif /* WIFI_UNIFIED_COMMAND */

VOID StaRecBfeeUpdate(
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN P_CMD_STAREC_BFEE        pCmdStaRecBfee);

VOID StaRecBfRead(
	IN struct _RTMP_ADAPTER *pAd,
	IN PUCHAR pBuf);

INT32 mt_AsicBfStaRecUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR                ucPhyMode,
	IN UCHAR                ucBssIdx,
	IN UINT16               u2WlanIdx);

INT32 mt_AsicBfeeStaRecUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR                u1PhyMode,
	IN UCHAR                u1BssIdx,
	IN UINT16               u2WlanIdx);

INT32 mt_AsicBfStaRecRelease(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR                ucBssIdx,
	IN UINT16               u2WlanIdx);

VOID mt_AsicClientBfCap(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry);

UCHAR AsicTxBfEnCondProc(
	IN	struct _RTMP_ADAPTER *pAd,
	IN	TXBF_STATUS_INFO * pTxBfInfo);

#ifdef CONFIG_STA_SUPPORT
VOID mt_BfSoundingAdjust(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 ConnectionState,
	IN struct wifi_dev *wdev
);
#endif /* CONFIG_STA_SUPPORT */

/* displayTagfield - display one tagfield */
void displayTagfield(
	IN struct _RTMP_ADAPTER  *pAd,
	IN	int		profileNum,
	IN	BOOLEAN implicitProfile);

#endif /* TXBF_SUPPORT // */

UINT32 starec_txbf_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature);

INT TxBfPseudoTagUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR         lm,
	IN UCHAR         nr,
	IN UCHAR         nc,
	IN UCHAR         bw,
	IN UCHAR         codebook,
	IN UCHAR         group);

VOID txbf_bfee_cap_unset(
	IN VOID);

UINT8 txbf_bfee_get_bfee_sts(
	IN UINT8 bfee_sts);

VOID txbf_bfee_cap_set(
	IN BOOLEAN valid,
	IN UINT8 bfer_cap_su,
	IN UINT8 num_snd_dimension);

VOID txbf_set_oui(
	IN UINT8 u1BfOui);

VOID txbf_clear_oui(
	VOID);

#endif /* _RT_TXBF_H_ */

