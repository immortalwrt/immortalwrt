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

    Module Name:
    dot11n_ht.h

    Abstract:
	Defined IE/frame structures of 802.11n

    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
*/

#ifndef _DOT11N_HT_H_
#define _DOT11N_HT_H_

#include "rtmp_type.h"
#include "dot11_base.h"


/*  HT Capability INFO field in HT Cap IE . */
typedef struct GNU_PACKED _HT_CAP_INFO {
#ifdef RT_BIG_ENDIAN
	UINT16	LSIGTxopProSup:1;
	UINT16	Forty_Mhz_Intolerant:1;
	UINT16	PSMP:1;
	UINT16	CCKmodein40:1;
	UINT16	AMsduSize:1;
	UINT16	DelayedBA:1;
	UINT16	RxSTBC:2;
	UINT16	TxSTBC:1;
	UINT16	ShortGIfor40:1;
	UINT16	ShortGIfor20:1;
	UINT16	GF:1;
	UINT16	MimoPs:2;
	UINT16	ChannelWidth:1;
	UINT16	ht_rx_ldpc:1;
#else
	UINT16	ht_rx_ldpc:1;
	UINT16	ChannelWidth:1;
	UINT16	MimoPs:2;		/* mimo power safe */
	UINT16	GF:1;			/* green field */
	UINT16	ShortGIfor20:1;
	UINT16	ShortGIfor40:1;	/* for40MHz */
	UINT16	TxSTBC:1;		/* 0:not supported,  1:if supported */
	UINT16	RxSTBC:2;
	UINT16	DelayedBA:1;
	UINT16	AMsduSize:1;	/* only support as zero */
	UINT16	CCKmodein40:1;
	UINT16	PSMP:1;
	UINT16	Forty_Mhz_Intolerant:1;
	UINT16	LSIGTxopProSup:1;
#endif /* RT_BIG_ENDIAN */
} HT_CAP_INFO;


/*  HT Capability INFO field in HT Cap IE . */
typedef struct GNU_PACKED _HT_CAP_PARM {
#ifdef RT_BIG_ENDIAN
	UINT8	rsv:3;/*momi power safe */
	UINT8	MpduDensity:3;
	UINT8	MaxRAmpduFactor:2;
#else
	UINT8	MaxRAmpduFactor:2;
	UINT8	MpduDensity:3;
	UINT8	rsv:3;/*momi power safe */
#endif /* RT_BIG_ENDIAN */
} HT_CAP_PARM, *PHT_CAP_PARM;


typedef struct GNU_PACKED _HT_MCS_SET_TX_SUBFIELD {
#ifdef RT_BIG_ENDIAN
	UINT8	TxMCSSetDefined:1;
	UINT8	TxRxNotEqual:1;
	UINT8	TxMaxStream:2;
	UINT8	TxUnqualModulation:1;
	UINT8	rsv:3;
#else
	UINT8	rsv:3;
	UINT8	TxUnqualModulation:1;
	UINT8	TxMaxStream:2;
	UINT8	TxRxNotEqual:1;
	UINT8	TxMCSSetDefined:1;
#endif /* RT_BIG_ENDIAN */
} HT_MCS_SET_TX_SUBFIELD, *PHT_MCS_SET_TX_SUBFIELD;


/*  HT Capability INFO field in HT Cap IE . */
typedef struct GNU_PACKED _HT_MCS_SET {
	UINT8	MCSSet[10];
	UINT8	SupRate[2];  /* unit : 1Mbps */
#ifdef RT_BIG_ENDIAN
	UINT8	rsv:3;
	UINT8	MpduDensity:1;
	UINT8	TxStream:2;
	UINT8	TxRxNotEqual:1;
	UINT8	TxMCSSetDefined:1;
#else
	UINT8	TxMCSSetDefined:1;
	UINT8	TxRxNotEqual:1;
	UINT8	TxStream:2;
	UINT8	MpduDensity:1;
	UINT8	rsv:3;
#endif /* RT_BIG_ENDIAN */
	UINT8	rsv3[3];
} HT_MCS_SET, *PHT_MCS_SET;

/*  HT Capability INFO field in HT Cap IE . */
typedef struct GNU_PACKED _EXT_HT_CAP_INFO {
#ifdef RT_BIG_ENDIAN
	UINT16	rsv2:4;
	UINT16	RDGSupport:1;	/*reverse Direction Grant  support */
	UINT16	PlusHTC:1;	/*+HTC control field support */
	UINT16	MCSFeedback:2;	/*0:no MCS feedback, 2:unsolicited MCS feedback, 3:Full MCS feedback,  1:rsv. */
	UINT16	rsv:5;/*momi power safe */
	UINT16	TranTime:2;
	UINT16	Pco:1;
#else
	UINT16	Pco:1;
	UINT16	TranTime:2;
	UINT16	rsv:5;/*momi power safe */
	UINT16	MCSFeedback:2;	/*0:no MCS feedback, 2:unsolicited MCS feedback, 3:Full MCS feedback,  1:rsv. */
	UINT16	PlusHTC:1;	/*+HTC control field support */
	UINT16	RDGSupport:1;	/*reverse Direction Grant  support */
	UINT16	rsv2:4;
#endif /* RT_BIG_ENDIAN */
} EXT_HT_CAP_INFO, *PEXT_HT_CAP_INFO;


/* HT Explicit Beamforming Feedback Capable */
#define HT_ExBF_FB_CAP_NONE			0
#define HT_ExBF_FB_CAP_DELAYED		1
#define HT_ExBF_FB_CAP_IMMEDIATE		2
#define HT_ExBF_FB_CAP_BOTH			3

/* HT Beamforming field in HT Cap IE */
typedef struct GNU_PACKED _HT_BF_CAP {
#ifdef RT_BIG_ENDIAN
	UINT32	rsv:3;
	UINT32	ChanEstimation:2;
	UINT32	CSIRowBFSup:2;
	UINT32	ComSteerBFAntSup:2;
	UINT32	NoComSteerBFAntSup:2;
	UINT32	CSIBFAntSup:2;
	UINT32	MinGrouping:2;
	UINT32	ExpComBF:2;
	UINT32	ExpNoComBF:2;
	UINT32	ExpCSIFbk:2;
	UINT32	ExpComSteerCapable:1;
	UINT32	ExpNoComSteerCapable:1;
	UINT32	ExpCSICapable:1;
	UINT32	Calibration:2;
	UINT32	ImpTxBFCapable:1;
	UINT32	TxNDPCapable:1;
	UINT32	RxNDPCapable:1;
	UINT32	TxSoundCapable:1;
	UINT32	RxSoundCapable:1;
	UINT32	TxBFRecCapable:1;
#else
	UINT32	TxBFRecCapable:1;
	UINT32	RxSoundCapable:1;
	UINT32	TxSoundCapable:1;
	UINT32	RxNDPCapable:1;
	UINT32	TxNDPCapable:1;
	UINT32	ImpTxBFCapable:1;
	UINT32	Calibration:2;
	UINT32	ExpCSICapable:1;
	UINT32	ExpNoComSteerCapable:1;
	UINT32	ExpComSteerCapable:1;
	UINT32	ExpCSIFbk:2;
	UINT32	ExpNoComBF:2;
	UINT32	ExpComBF:2;
	UINT32	MinGrouping:2;
	UINT32	CSIBFAntSup:2;
	UINT32	NoComSteerBFAntSup:2;
	UINT32	ComSteerBFAntSup:2;
	UINT32	CSIRowBFSup:2;
	UINT32	ChanEstimation:2;
	UINT32	rsv:3;
#endif /* RT_BIG_ENDIAN */
} HT_BF_CAP, *PHT_BF_CAP;

/*  HT antenna selection field in HT Cap IE . */
typedef struct GNU_PACKED _HT_AS_CAP {
#ifdef RT_BIG_ENDIAN
	UINT8	rsv:1;
	UINT8	TxSoundPPDU:1;
	UINT8	RxASel:1;
	UINT8	AntIndFbk:1;
	UINT8	ExpCSIFbk:1;
	UINT8	AntIndFbkTxASEL:1;
	UINT8	ExpCSIFbkTxASEL:1;
	UINT8	AntSelect:1;
#else
	UINT8	AntSelect:1;
	UINT8	ExpCSIFbkTxASEL:1;
	UINT8	AntIndFbkTxASEL:1;
	UINT8	ExpCSIFbk:1;
	UINT8	AntIndFbk:1;
	UINT8	RxASel:1;
	UINT8	TxSoundPPDU:1;
	UINT8	rsv:1;
#endif /* RT_BIG_ENDIAN */
} HT_AS_CAP, *PHT_AS_CAP;


/* Draft 1.0 set IE length 26, but is extensible.. */
#define SIZE_HT_CAP_IE		26
/* The structure for HT Capability IE. */
typedef struct GNU_PACKED _HT_CAPABILITY_IE {
	HT_CAP_INFO		HtCapInfo;
	HT_CAP_PARM		HtCapParm;
	/*	HT_MCS_SET		HtMCSSet; */
	UCHAR			MCSSet[16];
	EXT_HT_CAP_INFO	ExtHtCapInfo;
	HT_BF_CAP		TxBFCap;	/* beamforming cap. rt2860c not support beamforming. */
	HT_AS_CAP		ASCap;	/*antenna selection. */
} HT_CAPABILITY_IE, *PHT_CAPABILITY_IE;


/*   field in Addtional HT Information IE . */
typedef struct GNU_PACKED _ADD_HTINFO {
#ifdef RT_BIG_ENDIAN
	UCHAR	SerInterGranu:3;
	UCHAR	S_PSMPSup:1;
	UCHAR	RifsMode:1;
	UCHAR	RecomWidth:1;
	UCHAR	ExtChanOffset:2;
#else
	UCHAR	ExtChanOffset:2;
	UCHAR	RecomWidth:1;
	UCHAR	RifsMode:1;
	UCHAR	S_PSMPSup:1;	 /*Indicate support for scheduled PSMP */
	UCHAR	SerInterGranu:3;	 /*service interval granularity */
#endif
} ADD_HTINFO, *PADD_HTINFO;


typedef struct GNU_PACKED _ADD_HTINFO2 {
#ifdef RT_BIG_ENDIAN
	USHORT  rsv2:3;
	USHORT  CentFreq2:8
	USHORT	OBSS_NonHTExist:1;
	USHORT	rsv:1;
	USHORT	NonGfPresent:1;
	USHORT	OperaionMode:2;
#else
	USHORT	OperaionMode:2;
	USHORT	NonGfPresent:1;
	USHORT	rsv:1;
	USHORT	OBSS_NonHTExist:1;
	USHORT  CentFreq2:8;
	USHORT  rsv2:3;
#endif
} ADD_HTINFO2, *PADD_HTINFO2;


/* TODO: Need sync with spec about the definition of StbcMcs. In Draft 3.03, it's reserved. */
typedef struct GNU_PACKED _ADD_HTINFO3 {
#ifdef RT_BIG_ENDIAN
	USHORT	rsv:4;
	USHORT	PcoPhase:1;
	USHORT	PcoActive:1;
	USHORT	LsigTxopProt:1;
	USHORT	STBCBeacon:1;
	USHORT	DualCTSProtect:1;
	USHORT	DualBeacon:1;
	USHORT	StbcMcs:6;
#else
	USHORT	StbcMcs:6;
	USHORT	DualBeacon:1;
	USHORT	DualCTSProtect:1;
	USHORT	STBCBeacon:1;
	USHORT	LsigTxopProt:1;	/* L-SIG TXOP protection full support */
	USHORT	PcoActive:1;
	USHORT	PcoPhase:1;
	USHORT	rsv:4;
#endif /* RT_BIG_ENDIAN */
} ADD_HTINFO3, *PADD_HTINFO3;

#define SIZE_ADD_HT_INFO_IE		22
typedef struct  GNU_PACKED _ADD_HT_INFO_IE {
	UCHAR				ControlChan;
	ADD_HTINFO			AddHtInfo;
	ADD_HTINFO2			AddHtInfo2;
	ADD_HTINFO3			AddHtInfo3;
	UCHAR				MCSSet[16];		/* Basic MCS set */
} ADD_HT_INFO_IE, *PADD_HT_INFO_IE;

/* ht_caps_ie.ht_cap_info */
enum sm_power_save {
	SMPS_STATIC = 0,
	SMPS_DYNAMIC = 1,
	SMPS_RESERVED = 2,
	SMPS_DISABLE = 3
};

#define DOT11N_CAP_LDPC (1 << 0)
#define DOT11N_CAP_SUPPORT_CH_WIDTH_SET (1 << 1)
#define DOT11N_CAP_SM_PS_SHIFT 2
#define DOT11N_CAP_SM_PS_MASK (0x3 << 2)
#define DOT11N_CAP_GF (1 << 4)
#define DOT11N_CAP_BW20_SGI (1 << 5)
#define DOT11N_CAP_BW40_SGI (1 << 6)
#define DOT11N_CAP_TX_STBC (1 << 7)
#define DOT11N_CAP_RX_STBC_SHIFT 8
#define DOT11N_CAP_RX_STBC_MASK (0x3 << 8)
#define DOT11N_CAP_DELAYED_BA (1 << 10)
#define DOT11N_CAP_MAX_AMSDU_LEN (1 << 11)
#define DOT11N_CAP_DSSS_CCK_IN_BW40 (1 << 12)
#define DOT11N_CAP_40M_INTOLERANT (1 << 14)
#define DOT11N_CAP_L_SIG_TXOP_PROTECT (1 << 15)

/* a-mpdu params */
enum min_mpdu_start_spacing {
	INTERVAL_NO_RESTRICTION,
	INTERVAL_QUARTER_US,
	INTERVAL_HALF_US,
	INTERVAL_1_US,
	INTERVAL_2_US,
	INTERVAL_4_US,
	INTERVAL_8_US,
	INTERVAL_16_US
};

/* support mcs set field */
#define RX_MCS_BITMASK(supp_mcs_ptr, mcs)\
do {\
	if ((mcs) != 0)\
		(supp_mcs_ptr)->rx_mcs_bitmask[(mcs) / 8] |= (1 << ((mcs) % 8));\
} while (0)
#define DOT11N_RX_HIGHEST_DATA_RATE_MASK 0x3FF
#define DOT11N_TX_MCS_SET (1 << 0)
#define DOT11N_TX_RX_MCS_SET_NOT_EQU (1 << 1)
#define DOT11N_TX_MAX_NSS_SUPPORT_SHIFT 2
#define DOT11N_TX_MAX_NSS_SUPPORT_MASK (0x3 << 2)
#define DOT11N_TX_UNEQU_MODULATION (1 << 4)

struct GNU_PACKED ht_support_mcs {
	UINT8 rx_mcs_bitmask[10];
	UINT16 rx_high_support_data_rate;
	UINT32 tx_mcs_set;
};

/* ht ext caps */
enum pco_transmit_time {
	PCO_TX_TIME_400_US = 1,
	PCO_TX_TIME_1_5_MS,
	PCO_TX_TIME_5_MS
};

enum ht_mfb {
	NO_HT_MFB,
	RESERVED_HT_MFB,
	UNSOLICITED_HT_MFB,
	BOTH_HT_MFB
};

#define DOT11N_EXT_CAP_PCO 0x1
#define DOT11N_EXT_CAP_PCO_TX_TIME_SHIFT 1
#define DOT11N_EXT_CAP_PCO_TX_TIME_MASK (0x3 << 1)
#define DOT11N_EXT_CAP_MFB_SHIFT 8
#define DOT11N_EXT_CAP_MFB_MASK (0x3 << 8)
#define DOT11N_EXT_CAP_HTC (1 << 10)
#define DOT11N_EXT_CAP_RD_RESPONDER (1 << 11)

/* tx_bf_caps */
enum txbf_calibration {
	TXBF_CALI_NOT_SUPPORT,
	TXBF_CALI_RSP_NO_INITIATE,
	TXBF_CALI_RSRV,
	TXBF_CALI_BOTH_RSP_INITIATE
};

enum txbf_feedback {
	TXBF_NOT_SUPPORT_FB,
	TXBF_DELAY_FB,
	TXBF_IMMED_FB,
	TXBF_DELAY_IMMED_FB
};

enum txbf_ant_sounding {
	TXBF_1_TX_ANT_SND,
	TXBF_2_TX_ANT_SND,
	TXBF_3_TX_ANT_SND,
	TXBF_4_TX_ANT_SND
};

enum txbf_min_group {
	TXBF_NO_GROUPING,
	TXBF_1_2_GROUPING,
	TXBF_1_4_GROUPING,
	TXBF_1_2_4_GROUPING
};

enum txbf_csi_rows {
	TXBF_1_ROW_CSI,
	TXBF_2_ROW_CSI,
	TXBF_3_ROW_CSI,
	TXBF_4_ROW_CSI
};

enum txbf_ch_estimation {
	TXBF_1_STS,
	TXBF_2_STS,
	TXBF_3_STS,
	TXBF_4_STS
};

#define DOT11N_TXBF_CAP_IMPL_TXBF_RECV 0x1
#define DOT11N_TXBF_CAP_RX_STAGGER_SOUNDING (1 << 1)
#define DOT11N_TXBF_CAP_TX_STAGGER_SOUNDING (1 << 2)
#define DOT11N_TXBF_CAP_RX_NDP (1 << 3)
#define DOT11N_TXBF_CAP_TX_NDP (1 << 4)
#define DOT11N_TXBF_CAP_IMPL_TXBF (1 << 5)
#define DOT11N_TXBF_CAP_CALI_SHIFT 6
#define DOT11N_TXBF_CAP_CALI_MASK (0x3 << 6)
#define DOT11N_TXBF_CAP_EXPL_CSI (1 << 8)
#define DOT11N_TXBF_CAP_EXPL_NCP_STEER (1 << 9)
#define DOT11N_TXBF_CAP_EXPL_CP_STEER (1 << 10)
#define DOT11N_TXBF_CAP_EXPL_CSI_FB_SHIFT 11
#define DOT11N_TXBF_CAP_EXPL_CSI_FB_MASK (0x3 << 11)
#define DOT11N_TXBF_CAP_EXPL_NCB_FB_SHIFT 13
#define DOT11N_TXBF_CAP_EXPL_NCB_FB_MASK (0x3 << 13)
#define DOT11N_TXBF_CAP_EXPL_CB_FB_SHIFT 15
#define DOT11N_TXBF_CAP_EXPL_CB_FB_MASK (0x3 << 15)
#define DOT11N_TXBF_CAP_MIN_GROUPING_SHIFT 17
#define DOT11N_TXBF_CAP_MIN_GROUPING_MAK (0x3 << 17)
#define DOT11N_TXBF_CAP_CSI_BFER_ANT_SHIFT 19
#define DOT11N_TXBF_CAP_CSI_BFER_ANT_MASK (0x3 << 19)
#define DOT11N_TXBF_CAP_NCP_STEER_BFER_ANT_SHIFT 21
#define DOT11N_TXBF_CAP_NCP_STEER_BFER_ANT_MASK (0x3 << 21)
#define DOT11N_TXBF_CAP_CP_STEER_BFER_ANT_SHIFT 23
#define DOT11N_TXBF_CAP_CP_STEER_BFER_ANT_MASK (0x3 << 23)
#define DOT11N_TXBF_CAP_CSI_MAX_BFER_ROWS_SHIFT 25
#define DOT11N_TXBF_CAP_CSI_MAX_BFER_ROWS_MASK (0x3 << 25)
#define DOT11N_TXBF_CAP_CH_ESTIMATION_SHIFT 27
#define DOT11N_TXBF_CAP_CH_ESTIMATION_MASK (0x3 << 27)

/* asel_caps */
#define DOT11N_ASEL_CAP_SELECT 0x1
#define DOT11N_ASEL_CAP_EXPL_CSI_FB_TX (1 << 1)
#define DOT11N_ASEL_CAP_ANT_IND_FB_TX (1 << 2)
#define DOT11N_ASEL_CAP_EXPL_CSI_FB (1 << 3)
#define DOT11N_ASEL_CAP_ANT_IND_FB (1 << 4)
#define DOT11N_ASEL_CAP_RECV (1 << 5)
#define DOT11N_ASEL_TX_SOUND_PPDU (1 << 6)

/* ht_caps_ie */
struct GNU_PACKED ht_caps_ie {
	UINT16 ht_cap_info;
	UINT8 ampdu_params;
	struct ht_support_mcs support_mcs_set;
	UINT16 ht_ext_caps;
	UINT32 tx_bf_caps;
	UINT8 asel_caps;
};

/* ht_op_info.info_1 */
#define DOT11N_OPINFO_SEC_CH_OFFSET_MASK 0x3
#define DOT11N_OPINFO_SEC_STA_CH_WIDTH (1 << 2)
#define DOT11N_OPINFO_RIFS_MODE (1 << 3)
/* ht_op_info.info_2 */
#define DOT11N_OPINFO_HT_PROTECTION_MASK 0x3
#define DOT11N_OPINFO_NON_GF_STA_PRESENT (1 << 2)
#define DOT11N_OPINFO_OBSS_NONHT_STA_PRESENT (1 << 4)
#define DOT11N_OPINFO_CCFS_2_SHIFT 5
#define DOT11N_OPINFO_CCFS_2_MASK (0x7FF << 5)
#define DOT11N_OPINFO_DUAL_BEACON (1 << 22)
#define DOT11N_OPINFO_DUAL_CTS (1 << 23)
#define DOT11N_OPINFO_STBC_BEACON (1 << 24)
#define DOT11N_OPINFO_L_SIG_TXOP_PROTECT (1 << 25)
#define DOT11N_PCO_ACTIVE (1 << 26)
#define DOT11N_PCO_PHASE (1 << 27)
struct GNU_PACKED ht_op_info {
	UINT8 field_1;
	UINT32 field_2;
};

struct GNU_PACKED ht_op_ie {
	UINT8 prim_ch;
	struct ht_op_info op_info;
};

/* 802.11n draft3 related structure definitions. */
/* 7.3.2.60 */
#define dot11OBSSScanPassiveDwell							20	/* in TU. min amount of time that the STA continously scans each channel when performing an active OBSS scan. */
#define dot11OBSSScanActiveDwell							10	/* in TU.min amount of time that the STA continously scans each channel when performing an passive OBSS scan. */
#define dot11BSSWidthTriggerScanInterval					300  /* in sec. max interval between scan operations to be performed to detect BSS channel width trigger events. */
#define dot11OBSSScanPassiveTotalPerChannel					200	/* in TU. min total amount of time that the STA scans each channel when performing a passive OBSS scan. */
#define dot11OBSSScanActiveTotalPerChannel					20	/*in TU. min total amount of time that the STA scans each channel when performing a active OBSS scan */
#define dot11BSSWidthChannelTransactionDelayFactor			5	/* min ratio between the delay time in performing a switch from 20MHz BSS to 20/40 BSS operation and the maximum */
/*	interval between overlapping BSS scan operations. */
#define dot11BSSScanActivityThreshold						25	/* in %%, max total time that a STA may be active on the medium during a period of */
/*	(dot11BSSWidthChannelTransactionDelayFactor * dot11BSSWidthTriggerScanInterval) seconds without */
/*	being obligated to perform OBSS Scan operations. default is 25(== 0.25%) */

typedef struct GNU_PACKED _OVERLAP_BSS_SCAN_IE {
	USHORT		ScanPassiveDwell;
	USHORT		ScanActiveDwell;
	USHORT		TriggerScanInt;				/* Trigger scan interval */
	USHORT		PassiveTalPerChannel;		/* passive total per channel */
	USHORT		ActiveTalPerChannel;		/* active total per channel */
	USHORT		DelayFactor;				/* BSS width channel transition delay factor */
	USHORT		ScanActThre;				/* Scan Activity threshold */
} OVERLAP_BSS_SCAN_IE, *POVERLAP_BSS_SCAN_IE;


/*  7.3.2.56. 20/40 Coexistence element used in  Element ID = 72 = IE_2040_BSS_COEXIST */
typedef union GNU_PACKED _BSS_2040_COEXIST_IE {
	struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
		UCHAR	rsv:3;
		UCHAR	ObssScanExempGrant:1;
		UCHAR	ObssScanExempReq:1;
		UCHAR	BSS20WidthReq:1;
		UCHAR	Intolerant40:1;
		UCHAR	InfoReq:1;
#else
		UCHAR	InfoReq:1;
		UCHAR	Intolerant40:1;			/* Inter-BSS. set 1 when prohibits a receiving BSS from operating as a 20/40 Mhz BSS. */
		UCHAR	BSS20WidthReq:1;		/* Intra-BSS set 1 when prohibits a receiving AP from operating its BSS as a 20/40MHz BSS. */
		UCHAR	ObssScanExempReq:1;
		UCHAR	ObssScanExempGrant:1;
		UCHAR	rsv:3;
#endif /* RT_BIG_ENDIAN */
	} field;
	UCHAR   word;
} BSS_2040_COEXIST_IE, *PBSS_2040_COEXIST_IE;

/* EDCA Param Set IE */
enum aci_ac {
	ACI_AC_BE,
	ACI_AC_BK,
	ACI_AC_VI,
	ACI_AC_VO,
	ACI_AC_NUM
};
#endif /* _DOT11N_HT_H_ */
