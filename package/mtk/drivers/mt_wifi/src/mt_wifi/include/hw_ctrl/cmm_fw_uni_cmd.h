/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc
 */



#ifndef _CMM_FW_UNI_CMD_H
#define _CMM_FW_UNI_CMD_H

#ifdef WIFI_UNIFIED_COMMAND
#ifdef CFG_SUPPORT_FALCON_PP
#include "pp_cmd.h"
#endif /* CFG_SUPPORT_FALCON_PP */


#define UNI_CMD_OPT_BIT_0_ACK        		BIT(0)
#define UNI_CMD_OPT_BIT_1_UNI_CMD    		BIT(1) /* 1: unified command, 0:original cmd */
#define UNI_CMD_OPT_BIT_2_SET_QUERY  		BIT(2) /* 1: set, 0:query */
#define UNI_CMD_OPT_BIT_1_UNI_EVENT  		BIT(1)
#define UNI_CMD_OPT_BIT_2_UNSOLICIT_EVENT   BIT(2)

#define UNICMD_VOW_AIRTIME_QUANTUM_IDX_TOTAL_NUM  8
#define UNICMD_VOW_BWC_GROUP_NUMBER               16
#define UNICMD_VOW_BW_GROUP_QUANTUM_LEVEL_NUM     16

#define RED_INUSE_BITSHIFT					5
#define RED_INUSE_BITMASK					(0x1f)

#ifdef RT_BIG_ENDIAN
typedef	union _UNI_CMD_HEADER_0 {
	struct {
		UINT32 cid:16;
		UINT32 length:16;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_0;
#else
typedef union _UNI_CMD_HEADER_0 {
	struct {
		UINT32 length:16;
		UINT32 cid:16;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_0;
#endif

#ifdef RT_BIG_ENDIAN
typedef	union _UNI_CMD_HEADER_1 {
	struct {
		UINT32 seq_num:8;
		UINT32 frag_num:8;
		UINT32 pkt_type_id:8;
		UINT32 reserved:8;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_1;
#else
typedef union _UNI_CMD_HEADER_1 {
	struct {
		UINT32 reserved:8;
		UINT32 pkt_type_id:8;
		UINT32 frag_num:8;
		UINT32 seq_num:8;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_1;
#endif

#ifdef RT_BIG_ENDIAN
typedef	union _UNI_CMD_HEADER_2 {
	struct {
		UINT32 option:8;
		UINT32 s2d_index:8;
		UINT32 checksum:16;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_2;
#else
typedef union _UNI_CMD_HEADER_2 {
	struct {
		UINT32 checksum:16;
		UINT32 s2d_index:8;
		UINT32 option:8;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_2;
#endif

#ifdef RT_BIG_ENDIAN
typedef	union _UNI_CMD_HEADER_3 {
	struct {
		UINT32 reserved:32;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_3;
#else
typedef union _UNI_CMD_HEADER_3 {
	struct {
		UINT32 reserved:32;
	} field;
	UINT32 word;
} UNI_CMD_HEADER_3;
#endif

typedef struct GNU_PACKED _UNI_CMD_HEADER_ {
	UNI_CMD_HEADER_0 header_0;
	UNI_CMD_HEADER_1 header_1;
	UNI_CMD_HEADER_2 header_2;
	UNI_CMD_HEADER_3 header_3;
} UNI_CMD_HEADER;

enum UNI_CMD_TYPE {
	UNI_CMD_ID_DEVINFO            = 0x01, /* Update DEVINFO */
	UNI_CMD_ID_BSSINFO            = 0x02, /* Update BSSINFO */
	UNI_CMD_ID_STAREC_INFO        = 0x03, /* Update STAREC */
	UNI_CMD_ID_EDCA_SET           = 0x04, /* Update EDCA Set */
	UNI_CMD_ID_BAND_CONFIG        = 0x08, /* Band Config */
	UNI_CMD_ID_REPT_MUAR          = 0x09, /* Repeater Mode Muar Config */
	UNI_CMD_ID_NORM_MUAR          = 0x0A, /* Normal Mode Muar Config */
	UNI_CMD_ID_WSYS_CONFIG        = 0x0B, /* WSYS Configuration */
	UNI_CMD_ID_ACCESS_REG         = 0x0D, /* Access Register */
	UNI_CMD_ID_POWER_CTRL         = 0x0F, /* NIC Power control */
	UNI_CMD_ID_CFG_SMESH          = 0x10, /* Smesh Config */
	UNI_CMD_ID_RX_HDR_TRAN        = 0x12, /* Rx header translation */
	UNI_CMD_ID_SER                = 0x13, /* SER */
	UNI_CMD_ID_TWT                = 0x14, /* 80211AX TWT*/
	UNI_CMD_ID_ECC_OPER           = 0x18, /* ECC Operation */
	UNI_CMD_ID_RDD_ON_OFF_CTRL    = 0x19, /* RDD On/Off Control */
	UNI_CMD_ID_GET_MAC_INFO       = 0x1A, /* Get MAC info */
	UNI_CMD_ID_TXCMD_CTRL         = 0x1D, /* Txcmd ctrl */
	UNI_CMD_ID_MIB                = 0x22, /* Get MIB counter */
	UNI_CMD_ID_SNIFFER_MODE       = 0x24, /* Sniffer Mode */
	UNI_CMD_ID_SCS                = 0x26, /* SCS */
	UNI_CMD_ID_DVT                = 0x29, /* DVT */
	UNI_CMD_ID_GPIO               = 0x2A, /* GPIO setting*/
	UNI_CMD_ID_MURU               = 0x31, /* MURU*/
	UNI_CMD_ID_VOW                = 0x37, /* VOW */
	UNI_CMD_ID_PP                 = 0x38, /* PP */
	UNI_CMD_ID_TPC                = 0x39, /* TPC */
};

/* ============== Common Structure Part ============== */
typedef struct GNU_PACKED _UNI_CMD_TAG_HANDLE_T {
	UINT32  u2CmdFeature;
	UINT32  u4StructSize;
	VOID 	*pfHandler;
} UNI_CMD_TAG_HANDLE_T, *P_UNI_CMD_TAG_HANDLE_T;

/* ============== UNI_CMD_ID_DEVINFO Begin ============== */
typedef struct GNU_PACKED _UNI_CMD_DEVINFO_T {
	/* fixed field */
	UINT8 ucOwnMacIdx;
	UINT8 ucDbdcIdx;
	UINT8 aucPadding[2];

	/* tlv */
	UINT8 aucTlvBuffer[0];
} UNI_CMD_DEVINFO_T, *P_UNI_CMD_DEVINFO_T;

/* DevInfo information (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_DEVINFO_ACTIVE_T {
    UINT16 u2Tag;                   /* Tag = 0x00 */
    UINT16 u2Length;
    UINT8 ucActive;
    UINT8 aucPadding[1];
    UINT8 aucOwnMacAddr[6];
} UNI_CMD_DEVINFO_ACTIVE_T, *P_UNI_CMD_DEVINFO_ACTIVE_T;

typedef INT32 (*PFN_DEVINFO_ACTIVE_HANDLE)(struct _RTMP_ADAPTER *pAd,
											  UINT8 Active,
											  UINT8 *OwnMacAddr,
											  VOID *pHandle);

/* DevInfo command Tag */
typedef enum _UNI_CMD_DEVINFO_TAG_T {
    UNI_CMD_DEVINFO_ACTIVE = 0,
    UNI_CMD_DEVINFO_MAX_NUM
} UNI_CMD_DEVINFO_TAG_T;

/* ============== UNI_CMD_ID_DEVINFO End ============== */

/* ============== UNI_CMD_ID_BSSINFO Begin ============== */
/* BssInfo command Tag */
typedef enum _UNI_CMD_BSSINFO_TAG_T {
    UNI_CMD_BSSINFO_BASIC = 0,
    UNI_CMD_BSSINFO_RA = 1,
    UNI_CMD_BSSINFO_RLM = 2,
    UNI_CMD_BSSINFO_PROTECT = 3,
    UNI_CMD_BSSINFO_BSS_COLOR = 4,
    UNI_CMD_BSSINFO_HE = 5,
    UNI_CMD_BSSINFO_11V_MBSSID = 6,
    UNI_CMD_BSSINFO_BCN_CONTENT = 7,
    UNI_CMD_BSSINFO_BCN_CSA = 8,
    UNI_CMD_BSSINFO_BCN_BCC = 9,
    UNI_CMD_BSSINFO_BCN_MBSSID = 0xA,
    UNI_CMD_BSSINFO_RATE = 0xB,
    UNI_CMD_BSSINFO_WAPI = 0xC,
    UNI_CMD_BSSINFO_SAP = 0xD,
    UNI_CMD_BSSINFO_P2P = 0xE,
    UNI_CMD_BSSINFO_QBSS = 0xF,
    UNI_CMD_BSSINFO_SEC = 0x10,
    UNI_CMD_BSS_INFO_BCN_PROT = 0x11,
    UNI_CMD_BSSINFO_TXCMD = 0x12,
    UNI_CMD_BSSINFO_UAPSD = 0x13,
    UNI_CMD_BSSINFO_WMM_PS_TEST = 0x14,
    UNI_CMD_BSSINFO_POWER_SAVE = 0x15,
    UNI_CMD_BSSINFO_STA_CONNECTED = 0x16,
    UNI_CMD_BSSINFO_IFS_TIME = 0x17,
    UNI_CMD_BSSINFO_STA_IOT = 0x18,
    UNI_CMD_BSSINFO_OFFLOAD_PKT = 0x19,
    UNI_CMD_BSSINFO_MLD = 0x1A,
    UNI_CMD_BSSINFO_MAX_NUM
} UNI_CMD_BSSINFO_TAG_T;

typedef struct GNU_PACKED _UNI_CMD_BSSINFO_T {
    /* fixed field */
    UINT8 ucBssInfoIdx;
    UINT8 aucPadding[3];

	/* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_BSSINFO_T, *P_UNI_CMD_BSSINFO_T;

/* BssInfo basic information (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_BASIC_T {
    UINT16 u2Tag;          /* Tag = 0x00 */
    UINT16 u2Length;
    UINT8  ucActive;
    UINT8  ucOwnMacIdx;
    UINT8  ucHwBSSIndex;
    UINT8  ucDbdcIdx;
    UINT32 u4ConnectionType;
    UINT8  ucConnectionState;
    UINT8  ucWmmIdx;
    UINT8  aucBSSID[6];
    UINT16 u2BcMcWlanidx;  /* indicate which wlan-idx used for MC/BC transmission. */
    UINT16 u2BcnInterval;
    UINT8  ucDtimPeriod;
    UINT8  ucPhyMode;
    UINT16 u2StaRecIdxOfAP;
    UINT16 u2NonHTBasicPhyType;
    UINT8  ucPhyModeExt;  /* WMODE_AX_6G : BIT(0) */
    UINT8  aucPadding[1];
} UNI_CMD_BSSINFO_BASIC_T, *P_UNI_CMD_BSSINFO_BASIC_T;

typedef enum _ENUM_PARAM_MEDIA_STATE_T {
    MEDIA_STATE_CONNECTED = 0,
    MEDIA_STATE_DISCONNECTED,
    MEDIA_STATE_ROAMING_DISC_PREV,  /* transient disconnected state for normal/fast roamming purpose */
    MEDIA_STATE_TO_BE_INDICATED,
    MEDIA_STATE_NUM
} ENUM_PARAM_MEDIA_STATE, *P_ENUM_PARAM_MEDIA_STATE;

typedef INT32 (*PFN_BSSINFO_BASIC_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											UINT32 *offset,
											VOID *pHandle);
/* BssInfo RA information (Tag1) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_RA_T {
    UINT16  u2Tag;                 /* Tag = 0x01 */
    UINT16  u2Length;
    BOOLEAN  fgShortPreamble;
    BOOLEAN  fgTestbedForceShortGI;
    BOOLEAN  fgTestbedForceGreenField;
    UINT8   ucHtMode;
    BOOLEAN  fgSeOff;
    UINT8   ucAntennaIndex;
    UINT16  u2MaxPhyRate;
    UINT8   ucForceTxStream;
    UINT8   aucPadding[3];
} UNI_CMD_BSSINFO_RA_T, *P_UNI_CMD_BSSINFO_RA_T;

typedef INT32 (*PFN_BSSINFO_RA_HANDLE)(struct _RTMP_ADAPTER *pAd,
										struct _BSS_INFO_ARGUMENT_T *bss_info,
										VOID *pHandle);
/* BssInfo RLM information (Tag2) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_RLM_T {
    UINT16 u2Tag;  /* Tag = 0x02 */
    UINT16 u2Length;
    UINT8  ucPrimaryChannel;
    UINT8  ucCenterChannelSeg0;
    UINT8  ucCenterChannelSeg1;
    UINT8  ucBandwidth;
    UINT8  ucTxStream;
    UINT8  ucRxStream;
    UINT8  ucHtOpInfo1; /* for mobile segment */
    UINT8  ucSCO;    /* for mobile segment */
} UNI_CMD_BSSINFO_RLM_T, *P_UNI_CMD_BSSINFO_RLM_T;

typedef INT32 (*PFN_BSSINFO_RLM_HANDLE)(struct _RTMP_ADAPTER *pAd,
										struct _BSS_INFO_ARGUMENT_T *bss_info,
										VOID *pHandle);

/* BssInfo protection information (Tag3) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_PROT_T {
    UINT_16 u2Tag;  /* Tag = 0x03 */
    UINT_16 u2Length;
    UINT_32 u4ProtectMode;
} UNI_CMD_BSSINFO_PROT_T, *P_UNI_CMD_BSSINFO_PROT_T;

typedef INT32 (*PFN_BSSINFO_PROT_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo bss color information (Tag4) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_BSS_COLOR_T {
    UINT16 u2Tag;  /* Tag = 0x4 */
    UINT16 u2Length;
    BOOLEAN fgEnable;
    UINT8  ucBssColor;
    UINT8  aucPadding[2];
} UNI_CMD_BSSINFO_BSS_COLOR_T, *P_UNI_CMD_BSSINFO_BSS_COLOR_T;

typedef INT32 (*PFN_BSSINFO_BSS_COLOR_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												VOID *pHandle);

/* BssInfo HE information (Tag5) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_HE_T {
    UINT16 u2Tag;  /* Tag = 0x05 */
    UINT16 u2Length;
    UINT16 u2TxopDurationRtsThreshold;
    UINT8  ucDefaultPEDuration;
    BOOLEAN fgErSuDisable; /* for mobile segment */
    UINT16 au2MaxNssMcs[3];
    UINT8  aucPadding[2];
} UNI_CMD_BSSINFO_HE_T, *P_UNI_CMD_BSSINFO_HE_T;

typedef INT32 (*PFN_BSSINFO_HE_HANDLE)(struct _RTMP_ADAPTER *pAd,
										struct _BSS_INFO_ARGUMENT_T *bss_info,
										VOID *pHandle);

/* BssInfo 11v MBSSID information (Tag6) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_11V_MBSSID_T {
    UINT16 u2Tag;  /* Tag = 0x06 */
    UINT16 u2Length;
    UINT8  ucMaxBSSIDIndicator;
    UINT8  ucMBSSIDIndex;
    UINT8  aucPadding[2];
} UNI_CMD_BSSINFO_11V_MBSSID_T, *P_UNI_CMD_BSSINFO_11V_MBSSID_T;

typedef INT32 (*PFN_BSSINFO_11V_MBSSID_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												VOID *pHandle);

/* BssInfo BCN/PROB RSP information (Tag7) */
typedef INT32 (*PFN_BSSINFO_BCN_OFFLOAD_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												UINT32 *offset,
												VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_BSSINFO_BCN_CONTENT_T {
    UINT16 u2Tag;       /* Tag = 0x07 */
    UINT16 u2Length;
    UINT16 u2TimIeOffset;
    UINT16 u2CsaIeOffset;
    UINT16 u2BccIeOffset;
    UINT8  ucAction;
    UINT8  aucPktContentType;
    UINT16 u2PktLength;
    UINT8  aucPktContent[0];
} UNI_CMD_BSSINFO_BCN_CONTENT_T, *P_UNI_CMD_BSSINFO_BCN_CONTENT_T;

typedef enum _BCN_CONTENT_ACTION_T {
    BCN_ACTION_DISABLE = 0,
    BCN_ACTION_ENABLE = 1,
    UPDATE_PROBE_RSP = 2,
} BCN_CONTENT_ACTION_T, *P_BCN_CONTENT_ACTION_T;

typedef INT32 (*PFN_BSSINFO_BCN_CONTENT_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct _BSS_INFO_ARGUMENT_T *bss_info,
													VOID *pHandle);

/* BssInfo BCN CSA information (Tag8) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_BCN_CSA_T {
    UINT16 u2Tag;       /* Tag = 0x08 */
    UINT16 u2Length;
    UINT8  ucCsaCount;
    UINT8  aucPadding[3];
} UNI_CMD_BSSINFO_BCN_CSA_T, *P_UNI_CMD_BSSINFO_BCN_CSA_T;

typedef INT32 (*PFN_BSSINFO_BCN_CSA_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												VOID *pHandle);


/* BssInfo BCN BCC information (Tag9) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_BCN_BCC_T {
    UINT16 u2Tag;       /* Tag = 0x9 */
    UINT16 u2Length;
    UINT8  ucBccCount;
    UINT8  aucPadding[3];
} UNI_CMD_BSSINFO_BCN_BCC_T, *P_UNI_CMD_BSSINFO_BCN_BCC_T;

typedef INT32 (*PFN_BSSINFO_BCN_BCC_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												VOID *pHandle);

/* BssInfo BCN Mbssid-index ie information (Tag10) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_BCN_MBSSID_T {
    UINT16 u2Tag;       /* Tag = 0xA */
    UINT16 u2Length;
    UINT32 u4Dot11vMbssidBitmap;
    UINT16 u2MbssidIeOffset[32];
} UNI_CMD_BSSINFO_BCN_MBSSID_T, *P_UNI_CMD_BSSINFO_BCN_MBSSID_T;

typedef INT32 (*PFN_BSSINFO_BCN_MBSSID_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												VOID *pHandle);

/* BssInfo RATE information (Tag11) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_RATE_T {
    UINT16 u2Tag;  /* Tag = 0x0B */
    UINT16 u2Length;
    UINT16 u2OperationalRateSet; /* for mobile segment */
    UINT16 u2BSSBasicRateSet;    /* for mobile segment */
    UINT16 u2BcRate; /* for WA */
    UINT16 u2McRate; /* for WA */
    UINT8  ucPreambleMode; /* for WA */
    UINT8  aucPadding[3];
} UNI_CMD_BSSINFO_RATE_T, *P_UNI_CMD_BSSINFO_RATE_T;

typedef INT32 (*PFN_BSSINFO_RATE_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo WAPI information (Tag12) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_WAPI_T {
    UINT16 u2Tag;  /* Tag = 0x0C */
    UINT16 u2Length;
    BOOLEAN fgWapiMode;
    UINT8  aucPadding[3];
} UNI_CMD_BSSINFO_WAPI_T, *P_UNI_CMD_BSSINFO_WAPI_T;

typedef INT32 (*PFN_BSSINFO_WAPI_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo SAP information (Tag13) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_SAP_T {
    UINT16 u2Tag;  /* Tag = 0x0D */
    UINT16 u2Length;
    BOOLEAN fgIsHiddenSSID;
    UINT8  aucPadding[2];
    UINT8  ucSSIDLen;
    UINT8  aucSSID[32];
} UNI_CMD_BSSINFO_SAP_T, *P_UNI_CMD_BSSINFO_SAP_T;

typedef INT32 (*PFN_BSSINFO_SAP_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo P2P information (Tag14) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_P2P_T {
    UINT16 u2Tag;  /* Tag = 0x0E */
    UINT16 u2Length;
    UINT32  u4PrivateData;
} UNI_CMD_BSSINFO_P2P_T, *P_UNI_CMD_BSSINFO_P2P_T;

typedef INT32 (*PFN_BSSINFO_P2P_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo QBSS information (Tag15) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_QBSS_T {
    UINT16 u2Tag;  /* Tag = 0x0F */
    UINT16 u2Length;
    UINT8  ucIsQBSS;
    UINT8  aucPadding[3];
} UNI_CMD_BSSINFO_QBSS_T, *P_UNI_CMD_BSSINFO_QBSS_T;

typedef INT32 (*PFN_BSSINFO_QBSS_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo Security information (Tag16) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_SEC_T {
    UINT16 u2Tag;  /* Tag = 0x10 */
    UINT16 u2Length;
    UINT8  ucAuthMode;/**<
						*     Auth Mode             | Value | Note          |
						*     --------------------  | ------|-------------- |
						*     AUTH_MODE_OPEN        | 0     | -             |
						*     AUTH_MODE_SHARED      | 1     | Shared key    |
						*     AUTH_MODE_AUTO_SWITCH | 2     | Either open system or shared key |
						*     AUTH_MODE_WPA         | 3     | -             |
						*     AUTH_MODE_WPA_PSK     | 4     | -             |
						*     AUTH_MODE_WPA_NONE    | 5     | For Ad hoc    |
						*     AUTH_MODE_WPA2        | 6     | -             |
						*     AUTH_MODE_WPA2_PSK    | 7     | -             |
						*     AUTH_MODE_WPA2_FT     | 8     | 802.11r       |
						*     AUTH_MODE_WPA2_FT_PSK | 9     | 802.11r       |
						*     AUTH_MODE_WPA_OSEN    | 10    | -             |
						*     AUTH_MODE_WPA3_SAE    | 11    | -             |
						*/
    UINT8  ucEncStatus;
    UINT8  ucCipherSuit;
    UINT8  aucPadding[1];
} UNI_CMD_BSSINFO_SEC_T, *P_UNI_CMD_BSSINFO_SEC_T;

typedef INT32 (*PFN_BSSINFO_SEC_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo BCN Prot. information (Tag 0x11) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_BCN_PROT_T {
    UINT16 u2Tag; /* Tag = 0x11 */
    UINT16 u2Length;
    UINT8 aucBcnProtPN[LEN_WPA_TSC];
    UINT8 ucBcnProtEnabled;  /* 0: off, 1: SW mode, 2:HW mode */
    UINT8 ucBcnProtCipherId;
    UINT8 aucBcnProtKey[LEN_MAX_BIGTK];
    UINT8 ucBcnProtKeyId;
    UINT8 aucReserved[3];
} UNI_CMD_BSSINFO_BCN_PROT_T, *P_UNI_CMD_BSSINFO_BCN_PROT_T;

typedef INT32 (*PFN_BSSINFO_BCN_PROT_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												VOID *pHandle);

/* TxCMD Mode information (Tag 0x12) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_TXCMD_T {
    UINT_16 u2Tag;  /* Tag = 0x12 */
    UINT_16 u2Length;
    BOOLEAN fgUseTxCMD;
    UINT_8  aucPadding[3];
} UNI_CMD_BSSINFO_TXCMD_T, *P_UNI_CMD_BSSINFO_TXCMD_T;

typedef INT32 (*PFN_BSSINFO_TXCMD_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo UAPSD information (Tag 0x13) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_UAPSD_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8  ucBmpDeliveryAC;
    UINT8  ucBmpTriggerAC;
    UINT8  aucPadding[2];
} UNI_CMD_BSSINFO_UAPSD_T, *P_UNI_CMD_BSSINFO_UAPSD_T;

typedef INT32 (*PFN_BSSINFO_UAPSD_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo WMM PS test information (Tag 0x14) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_WMM_PS_TEST_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8  ucIsEnterPsAtOnce;
    UINT8  ucIsDisableUcTrigger;
    UINT8  aucPadding[2];
} UNI_CMD_BSSINFO_WMM_PS_TEST_T, *P_UNI_CMD_BSSINFO_WMM_PS_TEST_T;

typedef INT32 (*PFN_BSSINFO_WMM_PS_TEST_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct _BSS_INFO_ARGUMENT_T *bss_info,
													VOID *pHandle);

/* BssInfo Power Save information (Tag 0x15) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_POWER_SAVE_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8  ucPsProfile;/**<
						*     Power Save Mode                 | Value | Note                     |
						*     --------------------            | ------|--------------            |
						*     ENUM_PSP_CONTINUOUS_ACTIVE      | 0     | Leave power save mode    |
						*     ENUM_PSP_CONTINUOUS_POWER_SAVE  | 1     | Enter power save mode    |
						*     ENUM_PSP_FAST_SWITCH            | 2     | Fast switch mode         |
						*     ENUM_PSP_TWT                    | 3     | twt                      |
						*     ENUM_PSP_TWT_SP                 | 4     | twt sp                   |
						*/
    UINT8  aucPadding[3];
} UNI_CMD_BSSINFO_POWER_SAVE_T, *P_UNI_CMD_BSSINFO_POWER_SAVE_T;

typedef INT32 (*PFN_BSSINFO_POWER_SAVE_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct _BSS_INFO_ARGUMENT_T *bss_info,
													VOID *pHandle);

typedef enum _ENUM_POWER_SAVE_PROFILE_T {
    ENUM_PSP_CONTINUOUS_ACTIVE = 0,
    ENUM_PSP_CONTINUOUS_POWER_SAVE,
    ENUM_PSP_FAST_SWITCH,
    ENUM_PSP_TWT,
    ENUM_PSP_TWT_SP,
    ENUM_PSP_NUM
} ENUM_POWER_SAVE_PROFILE_T, *PENUM_POWER_SAVE_PROFILE_T;

/* BssInfo STA connection information (Tag 0x16) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_STA_CONNECTED_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT16 u2BcnInterval;
    UINT8  ucDtimPeriod;
    UINT8  aucPadding[1];
} UNI_CMD_BSSINFO_STA_CONNECTED_T, *P_UNI_CMD_BSSINFO_STA_CONNECTED_T;

typedef INT32 (*PFN_BSSINFO_STA_CONNECTED_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct _BSS_INFO_ARGUMENT_T *bss_info,
													VOID *pHandle);


/* BssInfo IFS time information (Tag 0x17) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_IFS_TIME_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    BOOLEAN fgSlotValid;
    BOOLEAN fgSifsValid;
    BOOLEAN fgRifsValid;
    BOOLEAN fgEifsValid;
    UINT16 u2SlotTime;
    UINT16 u2SifsTime;
    UINT16 u2RifsTime;
    UINT16 u2EifsTime;
} UNI_CMD_BSSINFO_IFS_TIME_T, *P_UNI_CMD_BSSINFO_IFS_TIME_T;

typedef INT32 (*PFN_BSSINFO_IFS_TIME_HANDLE)(struct _RTMP_ADAPTER *pAd,
												struct _BSS_INFO_ARGUMENT_T *bss_info,
												VOID *pHandle);

/* BssInfo Mobile need information (Tag 0x18) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_IOT_T {
    UINT16 u2Tag; /* Tag = 0x18 */
    UINT16 u2Length;
    UINT8 ucIotApBmp;
    UINT8 aucReserved[3];
} UNI_CMD_BSSINFO_IOT_T, *P_UNI_CMD_BSSINFO_IOT_T;

typedef INT32 (*PFN_BSSINFO_IOT_HANDLE)(struct _RTMP_ADAPTER *pAd,
											struct _BSS_INFO_ARGUMENT_T *bss_info,
											VOID *pHandle);

/* BssInfo IFS time information (Tag 0x19) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_OFFLOAD_PKT {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8  ucTxType;
    UINT8  ucTxMode;
    UINT8  ucTxInterval;
    UINT8  fgEnable;
    UINT16 u2Wcid;
    UINT16 u2OffloadPktLength;
    UINT8  aucPktContent[0];
} UNI_CMD_BSSINFO_OFFLOAD_PKT_T, *P_UNI_CMD_BSSINFO_OFFLOAD_PKT;

typedef INT32 (*PFN_BSSINFO_OFFLOAD_PKT_HANDLE)(struct _RTMP_ADAPTER *pAd,
													struct _BSS_INFO_ARGUMENT_T *bss_info,
													VOID *pHandle);
typedef enum bssinfo_unsolicit_txtype {
    BSSINFO_UNSOLICIT_TX_PROBE_RSP = 0,
    BSSINFO_UNSOLICIT_TX_FILS_DISC = 1,
    BSSINFO_UNSOLICIT_TX_QOS_NULL  = 2 /* packet injector */
} bssinfo_unsolicit_txtype_t;

/* BssInfo MLD information (Tag 0x1A) */
typedef struct GNU_PACKED _UNI_CMD_BSSINFO_MLD_T {
    UINT16 u2Tag;   /* Tag = 0x1A */
    UINT16 u2Length;
    UINT8  aucOwnMldAddr[MAC_ADDR_LEN];
    UINT8  ucOwnMldId; /* for UMAC, 0~ 63 */
    UINT8  ucOmRemapIdx;/* for AGG: 0~15, 0xFF means this is a legacy BSS, no need to do remapping */
} UNI_CMD_BSSINFO_MLD_T, *P_UNI_CMD_BSSINFO_MLD_T;

typedef INT32 (*PFN_BSSINFO_MLD_HANDLE)(struct _RTMP_ADAPTER *pAd,
										struct _BSS_INFO_ARGUMENT_T *bss_info,
										VOID *pHandle);
/* ============== UNI_CMD_ID_BSSINFO End ============== */

/* ============== UNI_CMD_ID_STAREC_INFO Begin ======== */
INT32 UniCmdWtblUpdate(RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT8 ucOperation,
							VOID *pBuffer, UINT32 u4BufferLen);

/* Common part of CMD_STAREC */
typedef struct GNU_PACKED _UNI_CMD_STAREC_T {
	/* Fixed field*/
    UINT8 ucBssInfoIdx;
    UINT8 ucWlanIdxL;
    UINT16 u2TotalElementNum;
    UINT8 ucAppendCmdTLV;
    UINT8 ucMuarIdx;
    UINT8 ucWlanIdxHnVer;
    UINT8 aucPadding[1];

	/* TLV */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_STAREC_T, *P_UNI_CMD_STAREC_T;

typedef struct GNU_PACKED _UNI_CMD_STAREC_WTBL_T {
	/* WTBL with STAREC update (Tag 0x0b) */
	/* STAREC format, content is WTBL format. */
	UINT16	u2Tag;
	UINT16	u2Length;
	UINT8	aucBuffer[0];
} UNI_CMD_CMD_STAREC_WTBL_T, *P_UNI_CMD_CMD_STAREC_WTBL_T;

typedef struct GNU_PACKED _UNI_CMD_STAREC_HE_INFO_T {
    UINT_16 u2Tag;
    UINT_16 u2Length;
    UINT_32 u4HeCap;
    UINT_8  ucTrigerFrameMacPadDuration;
    UINT_8  ucMaxAmpduLenExponentExtension;
    UINT_8  ucChBwSet;
    UINT_8  ucDeviceClass;
    UINT_8  ucDcmTxMode;
    UINT_8  ucDcmTxMaxNss;
    UINT_8  ucDcmRxMode;
    UINT_8  ucDcmRxMaxNss;
    UINT_8  ucDcmMaxRu;
    UINT_8  ucPuncPreamRx;
    UINT_8  ucPktExt;
    UINT_8  ucMaxAmpduLenExponent;
    /*0: BW80, 1: BW160, 2: BW8080*/
    UINT_16 au2MaxNssMcs[3];
    UINT_8  aucReserve1[2];
} UNI_CMD_STAREC_HE_INFO_T, *P_UNI_CMD_STAREC_HE_INFO_T;

typedef struct GNU_PACKED _UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T {
    UINT16 u2WlanIndex;      /* MLO */
    UINT8  ucMgmtProtection;
    UINT8  ucCipherId;
    UINT8  ucSubLength;      /* Length = total cipher subcmd structure size */
    UINT8  ucKeyIdx;         /* keyid 4,5 for IGTK; 6,7 for BIGTK */
    UINT8  ucKeyLength;
    UINT8  fgNeedRsp;
    UINT8  aucKeyMaterial[32];
} UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T, *P_UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T;

typedef INT32 (*PFN_STAREC_HANDLE)(struct _RTMP_ADAPTER *pAd,
									STA_REC_CFG_T *StaRecCfg,
									VOID *pHandle);

/*  STA record TLV tag */
typedef enum _UNI_CMD_STAREC_TAG_T {
    UNI_CMD_STAREC_BASIC               	= 0x00,
    UNI_CMD_STAREC_RA                  	= 0x01,
    /* UNI_CMD_STAREC_RA_COMMON_INFO      = 0x02, */
    UNI_CMD_STAREC_RA_UPDATE           	= 0x03,
    UNI_CMD_STAREC_BF                  	= 0x04,
    UNI_CMD_STAREC_MAUNAL_ASSOC        	= 0x05,
    UNI_CMD_STAREC_BA                  	= 0x06,
    UNI_CMD_STAREC_STATE_CHANGED       	= 0x07,
    UNI_CMD_STAREC_HT_BASIC            	= 0x09,
    UNI_CMD_STAREC_VHT_BASIC           	= 0x0a,
    UNI_CMD_STAREC_AP_PS               	= 0x0b,
    UNI_CMD_STAREC_INSTALL_KEY         	= 0x0c,
    UNI_CMD_STAREC_WTBL                	= 0x0d,
    UNI_CMD_STAREC_HE_BASIC            	= 0x0e,
    UNI_CMD_STAREC_HW_AMSDU             = 0x0f,
    UNI_CMD_STAREC_AAD_OM              	= 0x10,
    UNI_CMD_STAREC_INSTALL_KEY_V2      	= 0x11,
    UNI_CMD_STAREC_MURU                	= 0x12,
    UNI_CMD_STAREC_BFEE                	= 0x14,
    UNI_CMD_STAREC_PHY_INFO            	= 0x15,
    UNI_CMD_STAREC_BA_OFFLOAD          	= 0x16,
	UNI_CMD_STAREC_HE_6G_CAP       	   	= 0x17,
	UNI_CMD_STAREC_INSTALL_DEFAULT_KEY 	= 0x18,
	UNI_CMD_STAREC_MLD_SETUP 			= 0x20,
	UNI_CMD_STAREC_MLD_BASIC 			= 0x21,
	UNI_CMD_STAREC_EHT_BASIC 			= 0x22,
	UNI_CMD_STAREC_MLD_TEARDOWN 		= 0x23,
	UNI_CMD_STAREC_PSM		= 0x2a,

    UNI_CMD_STAREC_MAX_NUM
} UNI_CMD_STAREC_TAG_T;

/* Update HE 6g Info */
typedef struct GNU_PACKED _CMD_STAREC_HE_6G_CAP_T {
    UINT16      u2Tag;
    UINT16      u2Length;
    UINT16      u2He6gBandCapInfo;
    UINT8       aucReserve[2];
} CMD_STAREC_HE_6G_CAP_T, *P_CMD_STAREC_HE_6G_CAP_T;
/* ============== UNI_CMD_ID_STAREC_INFO End ========== */

/* ============== UNI_CMD_ID_EDCA_SET Begin ============== */
/* EDCA set command (0x04) */
typedef struct GNU_PACKED _UNI_CMD_EDCA_T {
    /* fixed field */
    UINT8 ucBssInfoIdx;
    UINT8 aucPadding[3];

    /* tlv */
    UINT_8 aucTlvBuffer[0];
} UNI_CMD_EDCA_T, *P_UNI_CMD_EDCA_T;

/* EDCA set command Tag */
typedef enum _UNI_CMD_EDCA_TAG_T {
    UNI_CMD_EDCA_AC_PARM = 0,
    UNI_CMD_EDCA_MAX_NUM
} UNI_CMD_EDCA_TAG_T;

/* EDCA AC Parameters (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_EDCA_AC_PARM_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucAcIndex;
    UINT8 ucValidBitmap;/**<
						*      Define            | BIT | Note              |
						*      ------------------|-----|------------------ |
						*      MASK_AIFS_SET     | 0   | 0x01, AIFSN       |
						*      MASK_WINMIN_SET   | 1   | 0x02, CW min      |
						*      MASK_WINMAX_SET   | 2   | 0x04, CW max      |
						*      MASK_TXOP_SET     | 3   | 0x08, TXOP Limit  |
						*/
    UINT8 ucCWmin;
    UINT8 ucCWmax;
    UINT16 u2TxopLimit;
    UINT8 ucAifsn;
    UINT8 aucPadding[1];
} UNI_CMD_EDCA_AC_PARM_T, *P_UNI_CMD_EDCA_AC_PARM_T;

#define MASK_AIFS_SET   BIT(0)
#define MASK_WINMIN_SET BIT(1)
#define MASK_WINMAX_SET BIT(2)
#define MASK_TXOP_SET   BIT(3)

typedef INT32 (*PFN_EDCA_AC_HANDLE)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
										MT_EDCA_CTRL_T EdcaParam, VOID *pHandle);

/* ============== UNI_CMD_ID_EDCA_SET End ============== */


/* ============== UNI_CMD_ID_WSYS_CONFIG Begin ============== */
/* WSYS Config set command (0x0B) */
typedef struct GNU_PACKED _UNI_CMD_WSYS_CONFIG_T {
    /* fixed field */
    UINT8 ucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
						*
						*   TAG                              | ID  | structure
						*   ---------------------------------|-----|--------------
						*   UNI_CMD_WSYS_CONFIG_FW_LOG_CTRL  | 0x0 | UNI_CMD_FW_LOG_CTRL_BASIC_T
						*   UNI_CMD_WSYS_CONFIG_FW_DBG_CTRL  | 0x1 | UNI_CMD_FW_DBG_CTRL_T
						*   UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL  | 0x2 | UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T
						*   UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG | 0x3 | UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T
						*   UNI_CMD_HOSTREPORT_TX_LATENCY_CONFIG| 0x4 | UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T
						*/
} UNI_CMD_WSYS_CONFIG_T, *P_UNI_CMD_WSYS_CONFIG_T;

/* WSYS Config set command TLV List */
typedef enum _UNI_CMD_WSYS_CONFIG_TAG_T {
    UNI_CMD_WSYS_CONFIG_FW_LOG_CTRL = 0,
    UNI_CMD_WSYS_CONFIG_FW_DBG_CTRL = 1,
    UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL = 2,
    UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG = 3,
    UNI_CMD_HOSTREPORT_TX_LATENCY_CONFIG = 4,
    UNI_CMD_WSYS_CONFIG_MAX_NUM
} UNI_CMD_WSYS_CONFIG_TAG_T;

/* FW Log Basic Setting (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_FW_LOG_CTRL_BASIC_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucFwLog2HostCtrl;
    UINT8 ucFwLog2HostInterval;/**<
								*      Time takes effect only if these conditions are true:
								*      1. FW log destinations include host
								*      2. ucFwLog2HostInterval > 0 (Unit: second)
								*/
    UINT8 aucPadding[2];
} UNI_CMD_FW_LOG_CTRL_BASIC_T, *P_UNI_CMD_FW_LOG_CTRL_BASIC_T;

typedef enum _ENUM_CMD_FW_LOG_2_HOST_CTRL_T {
    ENUM_CMD_FW_LOG_2_HOST_CTRL_OFF = 0,
    ENUM_CMD_FW_LOG_2_HOST_CTRL_2_UART = 1,
    ENUM_CMD_FW_LOG_2_HOST_CTRL_2_HOST = 2,
    ENUM_CMD_FW_LOG_2_HOST_CTRL_2_EMI = 4,
    ENUM_CMD_FW_LOG_2_HOST_CTRL_2_HOST_STORAGE = 8,
    ENUM_CMD_FW_LOG_2_HOST_CTRL_2_HOST_ETHNET = 16,
    ENUM_CMD_FW_LOG_2_HOST_CTRL_2_BUF = BIT(7),
} ENUM_CMD_FW_LOG_2_HOST_CTRL_T, *P_ENUM_CMD_FW_LOG_2_HOST_CTRL_T;

typedef INT32 (*PFN_WSYS_FW_LOG_CTRL_BASIC_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_FW_LOG_CTRL_BASIC_T pParam,
													VOID *pHandle);

/* FW Debug Level Setting (Tag1) */
typedef struct GNU_PACKED _UNI_CMD_FW_DBG_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT32 u4DbgModuleIdx;
    UINT8 ucDbgClass;
    UINT8 aucPadding[3];
} UNI_CMD_FW_DBG_CTRL_T, *P_UNI_CMD_FW_DBG_CTRL_T;

typedef INT32 (*PFN_WSYS_FW_DBG_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
												P_UNI_CMD_FW_DBG_CTRL_T pParam,
												VOID *pHandle);

/* FW Log UI Setting (Tag2) */
typedef struct GNU_PACKED _UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT32 ucVersion; /* default is 1 */
    UINT32 ucLogLevel;/* 0: Default, 1: More, 2: Extreme */
    UINT8  aucReserved[4];
} UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T, *P_UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T;

typedef INT32 (*PFN_WSYS_FW_LOG_UI_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T pParam,
													VOID *pHandle);

/* FW Debug Level Setting (Tag3) */
typedef struct GNU_PACKED _UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT16 u2RxChecksum;   /* bit0: IP, bit1: UDP, bit2: TCP */
    UINT16 u2TxChecksum;   /* bit0: IP, bit1: UDP, bit2: TCP */
    UINT8 ucCtrlFlagAssertPath;
    UINT8 aucPadding[3];
} UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T, *P_UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T;

typedef INT32 (*PFN_WSYS_FW_BASIC_CONFIG_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T pParam,
													VOID *pHandle);

/* FW Debug Level Setting (Tag4) */
typedef struct GNU_PACKED _UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucActive;
    UINT8 aucReserved[3];
} UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T, *P_UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T;

typedef INT32 (*PFN_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_HANDLE)(struct _RTMP_ADAPTER *pAd,
																P_UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T pParam,
																VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_WSYS_CFG_PARAM_T {
	UINT8 McuDest;
	BOOLEAN WsysCfgTagValid[UNI_CMD_WSYS_CONFIG_MAX_NUM];

	UNI_CMD_FW_LOG_CTRL_BASIC_T WsysFwLogCtrlBasic;
	UNI_CMD_FW_DBG_CTRL_T WsysFwDbgCtrl;
	UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T WsysCfgFwLogUICtrl;
	UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T WsysCfgFwFwBasicConfig;
	UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T WsysCfgHostReportTxLatency;
} UNI_CMD_WSYS_CFG_PARAM_T, *P_UNI_CMD_WSYS_CFG_PARAM_T;

/* ============== UNI_CMD_ID_WSYS_CONFIG End ============== */

/* ============== UNI_CMD_ID_ACCESS_REG Begin ============== */
/* register access command (0x0D) */
typedef struct GNU_PACKED _UNI_CMD_ACCESS_REG_T {
    /* fixed field */
    UINT8 ucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
						*
						*   TAG                              | ID  | structure
						*   ---------------------------------|-----|--------------
						*   UNI_CMD_ACCESS_REG_BASIC         | 0x0 | UNI_CMD_ACCESS_REG_BASIC_T
						*   UNI_CMD_ACCESS_RF_REG_BASIC      | 0x1 | UNI_CMD_ACCESS_RF_REG_BASIC_T
						*/
} UNI_CMD_ACCESS_REG_T, *P_UNI_CMD_ACCESS_REG_T;

/* Register access command TLV List */
typedef enum _UNI_CMD_ACCESS_REG_TAG_T {
    UNI_CMD_ACCESS_REG_BASIC = 0,
    UNI_CMD_ACCESS_RF_REG_BASIC,
    UNI_CMD_ACCESS_REG_MAX_NUM
} UNI_CMD_ACCESS_REG_TAG_T;

typedef struct GNU_PACKED _UNI_CMD_ACCESS_REG_BASIC_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT32 u4Addr;
    UINT32 u4Value;
} UNI_CMD_ACCESS_REG_BASIC_T, *P_UNI_CMD_ACCESS_REG_BASIC_T;

typedef INT32 (*PFN_ACCESS_REG_BASIC_HANDLE)(struct _RTMP_ADAPTER *pAd,
												RTMP_REG_PAIR *RegPair,
												VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_ACCESS_RF_REG_BASIC_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT16 u2WifiStream;
    UINT16 u2Reserved;
    UINT32 u4Addr;
    UINT32 u4Value;
} UNI_CMD_ACCESS_RF_REG_BASIC_T, *P_UNI_CMD_ACCESS_RF_REG_BASIC_T;

typedef INT32 (*PFN_ACCESS_RF_REG_BASIC_HANDLE)(struct _RTMP_ADAPTER *pAd,
													MT_RF_REG_PAIR *RfRegPair,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_ACCESS_REG_PARAM_T {
	BOOLEAN bQueryMode;
	UINT32 RegNum[UNI_CMD_ACCESS_REG_MAX_NUM];
	BOOLEAN AccessRegTagValid[UNI_CMD_ACCESS_REG_MAX_NUM];

	struct _RTMP_REG_PAIR *RegPair;
	struct _MT_RF_REG_PAIR *RfRegPair;
} UNI_CMD_ACCESS_REG_PARAM_T, *P_UNI_CMD_ACCESS_REG_PARAM_T;

/* ============== UNI_CMD_ID_ACCESS_REG End ============== */

/* ============== UNI_CMD_ID_POWER_CTRL Begin ============== */
typedef struct GNU_PACKED _UNI_CMD_ID_POWER_CTRL_T {
    /* fixed field */
    UINT8 ucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
						*
						*   TAG                      | ID  | structure
						*   -------------------------|-----|--------------
						*   UNI_CMD_POWER_OFF        | 0x0 | UNI_CMD_POWER_OFF_T
						*/
} UNI_CMD_ID_POWER_CTRL_T, *P_UNI_CMD_ID_POWER_CTRL_T;

/* Get power ctrl command TLV List */
typedef enum _UNI_CMD_POWER_CTRL_TAG_T {
    UNI_CMD_POWER_OFF = 0,
    UNI_CMD_POWER_CTRL_MAX_NUM
} UNI_CMD_POWER_CTRL_TAG_T;

/* Get tsf time (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_POWER_OFF_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucPowerMode;
    UINT8 aucReserved[3];
} UNI_CMD_POWER_OFF_T, *P_UNI_CMD_POWER_OFF_T;

/* ============== UNI_CMD_ID_POWER_CTRL End ============== */

/* ============== UNI_CMD_ID_CFG_SMESH Begin ============== */
#ifdef AIR_MONITOR
/* SMESH command (0x10) */
typedef struct GNU_PACKED _UNI_CMD_SMESH_T {
    /* fixed field */
    UINT8 ucBand;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_SMESH_T, *P_UNI_CMD_SMESH_T;

/* Smesh command Tag */
typedef enum _UNI_CMD_SMESH_TAG_T {
    UNI_CMD_SMESH_PARAM = 0,
    UNI_CMD_SMESH_MAX_NUM
} UNI_CMD_SMESH_TAG_T;

/* SMESH Config Parameters (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_SMESH_PARAM_T {
    UINT16 u2Tag;    /* Tag = 0x00 */
    UINT16 u2Length;
    UINT8 ucEntryEnable;
    BOOLEAN fgSmeshRxA2;
    BOOLEAN fgSmeshRxA1;
    BOOLEAN fgSmeshRxData;
    BOOLEAN fgSmeshRxMgnt;
    BOOLEAN fgSmeshRxCtrl;
    UINT8 aucPadding[2];
} UNI_CMD_SMESH_PARAM_T, *P_UNI_CMD_SMESH_PARAM_T;
#endif /* AIR_MONITOR */
/* ============== UNI_CMD_ID_CFG_SMESH End ============== */

/* ============== UNI_CMD_ID_RX_HDR_TRAN Begin ============== */
/* Rx header translation command (0x12) */
typedef struct GNU_PACKED _UNI_CMD_RX_HDR_TRAN_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0]; /**<the TLVs includer in this field:
							*
							*  TAG                                   | ID   | structure
							*  -------------                         | -----| -------------
							*  UNI_CMD_RX_HDR_TRAN_ENABLE            | 0x0  | UNI_CMD_RX_HDR_TRAN_ENABLE_T
							*  UNI_CMD_RX_HDR_TRAN_VLAN_CONFIG       | 0x1  | UNI_CMD_RX_HDR_TRAN_VLAN_T
							*  UNI_CMD_RX_HDR_TRAN_BLACKLIST_CONFIG  | 0x2  | UNI_CMD_RX_HDR_TRAN_BLACKLIST_T
							*/
} UNI_CMD_RX_HDR_TRAN_T, *P_UNI_CMD_RX_HDR_TRAN_T;

/* RX HDR TRAN command TLV List */
typedef enum _UNI_CMD_RX_HDR_TRAN_TAG_T {
    UNI_CMD_RX_HDR_TRAN_ENABLE = 0,
    UNI_CMD_RX_HDR_TRAN_VLAN_CONFIG = 1,
    UNI_CMD_RX_HDR_TRAN_BLACKLIST_CONFIG = 2,
    UNI_CMD_RX_HDR_TRAN_MAX_NUM
} UNI_CMD_RX_HDR_TRAN_TAG_T;

/* Rx header translation enable/disable (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_RX_HDR_TRAN_ENABLE_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    BOOLEAN fgEnable;
    BOOLEAN fgCheckBssid;
    UINT8 ucTranslationMode;
    UINT8 aucPadding[1];
} UNI_CMD_RX_HDR_TRAN_ENABLE_T, *P_UNI_CMD_RX_HDR_TRAN_ENABLE_T;

typedef INT32 (*PFN_RX_HDR_TRAN_ENABLE_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_RX_HDR_TRAN_ENABLE_T pParam,
													VOID *pHandle);

/* Rx header translation vlan config (Tag1) */
typedef struct GNU_PACKED _UNI_CMD_RX_HDR_TRAN_VLAN_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    BOOLEAN fgInsertVlan;
    BOOLEAN fgRemoveVlan;
    BOOLEAN fgUseQosTid;
    UINT8 aucPadding[1];
} UNI_CMD_RX_HDR_TRAN_VLAN_T, *P_UNI_CMD_RX_HDR_TRAN_VLAN_T;

typedef INT32 (*PFN_RX_HDR_TRAN_VLAN_HANDLE)(struct _RTMP_ADAPTER *pAd,
												P_UNI_CMD_RX_HDR_TRAN_VLAN_T pParam,
												VOID *pHandle);

/* Rx header translation black list config (Tag2) */
typedef struct GNU_PACKED _UNI_CMD_RX_HDR_TRAN_BLACKLIST_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucBlackListIdx;
    BOOLEAN fgEnable;
    UINT16 u2EtherType;
} UNI_CMD_RX_HDR_TRAN_BLACKLIST_T, *P_UNI_CMD_RX_HDR_TRAN_BLACKLIST_T;

typedef INT32 (*PFN_RX_HDR_TRAN_BLACKLIST_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_RX_HDR_TRAN_BLACKLIST_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_RX_HDR_TRAN_PARAM_T {
	BOOLEAN RxHdrTranValid[UNI_CMD_RX_HDR_TRAN_MAX_NUM];
	UNI_CMD_RX_HDR_TRAN_ENABLE_T RxHdrTranEnable;
	UNI_CMD_RX_HDR_TRAN_VLAN_T RxHdrTranVlan;
	UNI_CMD_RX_HDR_TRAN_BLACKLIST_T RxHdrTranBlackList;
} UNI_CMD_RX_HDR_TRAN_PARAM_T, *P_UNI_CMD_RX_HDR_TRAN_PARAM_T;
/* ============== UNI_CMD_ID_RX_HDR_TRAN End ============== */

/* ============== UNI_CMD_ID_BAND_CONFIG Begin ============ */
/**
 * Common Part of UNI_CMD_ID_BAND_CONFIG (0x08)
 */
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_T {
	/*fixed field*/
	UINT8 ucDbdcIdx;
	UINT8 aucPadding[3];

	/* tlv */
	UINT8 aucTlvBuffer[0]; /**< the TLVs included in this field:
		*
		*	TAG 							 | ID  | structure
		*	-------------					 | ----| -------------
		*	UNI_CMD_BAND_CONFIG_RADIO_ONOFF    | 0x0 | UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T
		*	UNI_CMD_BAND_CONFIG_RXV_CTRL	   | 0x1 | UNI_CMD_BAND_CONFIG_RXV_CTRL_T
		*	UNI_CMD_BAND_CONFIG_SET_RX_FILTER  | 0x2 | UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T
		*	UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME| 0x3 | UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T
		*	UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT   | 0x4 | UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T
		*	UNI_CMD_BAND_CONFIG_EDCCA_ENABLE   | 0x5 | UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T
		*	UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD| 0x6 | UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T
		*/
} UNI_CMD_BAND_CONFIG_T, *P_UNI_CMD_BAND_CONFIG_T;

/* Band Config command Tag */
typedef enum _UNI_CMD_BAND_CONFIG_TAG_T {
    UNI_CMD_BAND_CONFIG_RADIO_ONOFF = 0,
    UNI_CMD_BAND_CONFIG_RXV_CTRL = 1,
    UNI_CMD_BAND_CONFIG_SET_RX_FILTER = 2,
    UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME = 3,
    UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT = 4,
    UNI_CMD_BAND_CONFIG_EDCCA_ENABLE = 5,
    UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD = 6,
	UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN = 9,
	UNI_CMD_BAND_CONFIG_SCH_DET_DIS = 0xA,
	UNI_CMD_BAND_CONFIG_RTS0_PKT_THRESHOLD_CFG = 0xB,
	UNI_CMD_BAND_CONFIG_MAX_NUM
} UNI_CMD_BAND_CONFIG_TAG_T;

/** UNI_CMD_BAND_CONFIG_RADIO_ONOFF tag(0x0)
 * To turn on/off radio
 */
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T {
    UINT16 u2Tag;      /*should be 0x0*/
    UINT16 u2Length;   /*the length of this TLV, should be sizeof(UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T)*/
    BOOLEAN fgRadioOn;  /*TRUE: turn on radio, FALSE: turn off radio*/
    UINT8  aucPadding[3];
} UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T, *P_UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T;

typedef INT32 (*PFN_BAND_CFG_RADIO_ONOFF_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T pParam,
													VOID *pHandle);

/** UNI_CMD_BAND_CONFIG_RXV_CTRL tag(0x1)
 * To enable/disable RXV control
 */
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_RXV_CTRL_T {
    UINT16 u2Tag;              /*should be 0x1*/
    UINT16 u2Length;           /*the length of this TLV, should be sizeof(UNI_CMD_BAND_CONFIG_RXV_CTRL_T)*/
    UINT8  ucRxvOfRxEnable;    /*1: enable Rx's RXV control, 0: disable Rx's RXV control*/
    UINT8  ucRxvOfTxEnable;    /*1: enable Tx's RXV control, 0: disable Tx's RXV control*/
    UINT8  aucPadding[2];
} UNI_CMD_BAND_CONFIG_RXV_CTRL_T, *P_UNI_CMD_BAND_CONFIG_RXV_CTRL_T;

typedef INT32 (*PFN_BAND_CFG_RXV_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
												P_UNI_CMD_BAND_CONFIG_RXV_CTRL_T pParam,
												VOID *pHandle);

/** UNI_CMD_BAND_CONFIG_SET_RX_FILTER tag(0x2)
*/
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT32 u4RxPacketFilter;
} UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T, *P_UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T;

typedef INT32 (*PFN_BAND_CFG_SET_RX_FILTER_HANDLE)(struct _RTMP_ADAPTER *pAd,
														P_UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T pParam,
														VOID *pHandle);

/** UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME tag(0x3)
*/
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucDropRts;
    UINT8 ucDropCts;
    UINT8 ucDropUnwantedCtrl;
    UINT8 aucReserved[1];
} UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T, *P_UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T;

typedef INT32 (*PFN_BAND_CFG_DROP_CTRL_FRAME_HANDLE)(struct _RTMP_ADAPTER *pAd,
												  P_UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T pParam,
												  VOID *pHandle);

/** UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT tag(0x4)
*/
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucWmmIdx;
    UINT8 ucAc;
    UINT8 ucAggLimit;
    UINT8 aucReserved[1];
} UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T, *P_UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T;

typedef INT32 (*PFN_BAND_CFG_AGG_AC_LIMIT_HANDLE)(struct _RTMP_ADAPTER *pAd,
												  P_UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T pParam,
												  VOID *pHandle);

/* EDCCA OnOff Control (Tag5) */
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T {
    UINT16 u2Tag;    /* Tag = 0x05 */
    UINT16 u2Length;
    UINT8 fgEDCCAEnable;
    UINT8 aucPadding[3];
} UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T, *P_UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T;

typedef INT32 (*PFN_BAND_CFG_EDCCA_ENABLE_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
												  P_UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T pParam,
												  VOID *pHandle);

/* EDCCA Threshold Control (Tag6) */
typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T {
	UINT16 u2Tag;    /* Tag = 0x06 */
	UINT16 u2Length;
	UINT8 u1EDCCAThreshold[3];
	UINT8 fginit;
} UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T, *P_UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T;

typedef INT32 (*PFN_BAND_CFG_EDCCA_THRESHOLD_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
												  P_UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T pParam,
												  VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN_T {
	UINT16  u2Tag;
	UINT16  u2Length;
	BOOLEAN Enable;
	UINT8   aucReserve[3];
} UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN_T, *P_UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN_T;

typedef INT32 (*PFN_BAND_CFG_RTS_SIGTA_EN_HANDLE)(struct _RTMP_ADAPTER *pAd,
									P_UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN_T pParam,
									VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T {
	UINT16  u2Tag;
	UINT16  u2Length;
	BOOLEAN Disable;
	UINT8   aucReserve[3];
} UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T, *P_UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T;

typedef INT32 (*PFN_BAND_CFG_SCH_DET_DIS_HANDLE)(struct _RTMP_ADAPTER *pAd,
									P_UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T pParam,
									VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_BAND_CONFIG_RTS0_PKT_THRESHOLD_CFG_T {
	UINT16  u2Tag;
	UINT16  u2Length;
	UINT32  u4Value;
	BOOLEAN Enable;
	UINT8   ucType;
	UINT8   aucReserve[2];
} UNI_CMD_BAND_CONFIG_RTS0_PKT_THRESHOLD_CFG_T, *P_UNI_CMD_BAND_CONFIG_RTS0_PKT_THRESHOLD_CFG_T;

typedef INT32 (*PFN_BAND_CFG_RTS0_PKT_THRESHOLD_HANDLE)(struct _RTMP_ADAPTER *pAd,
									P_UNI_CMD_BAND_CONFIG_RTS0_PKT_THRESHOLD_CFG_T pParam,
									VOID *pHandle);


typedef struct GNU_PACKED _UNI_CMD_BAND_CFG_PARAM_T {
	UINT8 ucDbdcIdx;
	BOOLEAN bQuery;
	BOOLEAN BandCfgTagValid[UNI_CMD_BAND_CONFIG_MAX_NUM];

	UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T BandCfgRadioOnOff;
	UNI_CMD_BAND_CONFIG_RXV_CTRL_T BandCfgRXVCtrl;
	UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T BandCfgSetRxFilter;
	UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T BandCfgDropCtrlFrame;
	UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T BandCfgAGGAcLimit;
	UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T BandCfgEDCCAEnable;
	UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T BandCfgEDCCAThreshold;
	UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN_T BandCfgRtsSigtaen;
	UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T BandCfgSCHDetDis;
	UNI_CMD_BAND_CONFIG_RTS0_PKT_THRESHOLD_CFG_T BandCfgRTS0PktThreshold;
} UNI_CMD_BAND_CFG_PARAM_T, *P_UNI_CMD_BAND_CFG_PARAM_T;
/* ============== UNI_CMD_ID_BAND_CONFIG End ============== */


/* ============== UNI_CMD_ID_SER Begin ============== */
typedef struct GNU_PACKED _UNI_CMD_SER_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0]; /**<the TLVs includer in this field:
							*
							*  TAG                    | ID   | structure
							*  -------------          | -----| -------------
							*  UNI_CMD_SER_QUERY      | 0x0  | UNI_CMD_SER_QUERY_T
							*  UNI_CMD_SER_ENABLE     | 0x1  | UNI_CMD_SER_ENABLE_T
							*  UNI_CMD_SER_SET        | 0x2  | UNI_CMD_SER_SET_T
							*  UNI_CMD_SER_TRIGGER    | 0x3  | UNI_CMD_SER_TRIGGER_T
							*/
} UNI_CMD_SER_T, *P_UNI_CMD_SER_T;

/* SER command TLV List */
typedef enum _UNI_CMD_SER_TAG_T {
    UNI_CMD_SER_QUERY = 0,
    UNI_CMD_SER_ENABLE = 1,
    UNI_CMD_SER_SET = 2,
    UNI_CMD_SER_TRIGGER = 3,
    UNI_CMD_SER_L0P5_CTRL = 4,
    UNI_CMD_SER_MAX_NUM
} UNI_CMD_SER_TAG_T;

/* Show ser (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_SER_QUERY_T {
    UINT16 u2Tag;
    UINT16 u2Length;
} UNI_CMD_SER_QUERY_T, *P_UNI_CMD_SER_QUERY_T;

typedef INT32 (*PFN_SER_QUERY_HANDLE)(struct _RTMP_ADAPTER *pAd,
										VOID *pHandle);

/* Enable/disable ser (Tag1) */
typedef struct GNU_PACKED _UNI_CMD_SER_ENABLE_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    BOOLEAN fgEnable;
    UINT8 aucPadding[3];
} UNI_CMD_SER_ENABLE_T, *P_UNI_CMD_SER_ENABLE_T;

typedef INT32 (*PFN_SER_ENABLE_HANDLE)(struct _RTMP_ADAPTER *pAd,
										BOOLEAN fgEnable,
										VOID *pHandle);

/* config ser (Tag2) */
typedef struct GNU_PACKED _UNI_CMD_SER_SET_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT32 u4EnableMask;
} UNI_CMD_SER_SET_T, *P_UNI_CMD_SER_SET_T;

typedef INT32 (*PFN_SER_ENABLE_MASK_HANDLE)(struct _RTMP_ADAPTER *pAd,
												UINT32 u4EnableMask,
												VOID *pHandle);

/* trigger ser recovery (Tag3) */
typedef struct GNU_PACKED _UNI_CMD_SER_TRIGGER_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucTriggerMethod;
    UINT8 ucDbdcIdx;
    UINT8 aucPadding[2];
} UNI_CMD_SER_TRIGGER_T, *P_UNI_CMD_SER_TRIGGER_T;

typedef INT32 (*PFN_SER_TRIGGER_HANDLE)(struct _RTMP_ADAPTER *pAd,
											UINT8 ucTriggerMethod,
											UINT8 ucDbdcIdx,
											VOID *pHandle);

/* do some controls in L0.5 reset (Tag4) */
typedef struct GNU_PACKED _UNI_CMD_SER_L0P5_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucCtrlAction;
    UINT8 aucPadding[3];
} UNI_CMD_SER_L0P5_CTRL_T, *P_UNI_CMD_SER_L0P5_CTRL_T;

/* ============== UNI_CMD_ID_SER End ============== */


/* ============== UNI_CMD_ID_TWT Begin ============== */
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
typedef struct GNU_PACKED _UNI_CMD_TWT_T {
    /*fixed field*/
    UINT8 ucBssInfoIdx;
    UINT8 aucPadding[3];
    /* tlv */
    UINT8 aucTlvBuffer[0]; /**< the TLVs included in this field:
							*
							*  TAG                         | ID   | structure
							*  -------------               | -----| -------------
							*  UNI_CMD_TWT_AGRT_UPDATE       | 0x0  | UNI_CMD_TWT_ARGT_UPDATE_T
							*/
} UNI_CMD_TWT_T, *P_UNI_CMD_TWT_T;

typedef enum _UNI_CMD_TWT_TAG_T {
    UNI_CMD_TWT_AGRT_UPDATE = 0,
    UNI_CMD_TWT_MAX_NUM
} UNI_CMD_TWT_TAG_T;

/* Basic Scan down notify Parameters (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_TWT_ARGT_UPDATE_T {
    UINT16    u2Tag;    /* Tag = 0x00 */
    UINT16    u2Length;
    UINT8     ucAgrtTblIdx;
    UINT8     ucAgrtCtrlFlag;
    UINT8     ucOwnMacId;
    UINT8     ucFlowId;
    UINT16    u2PeerIdGrpId;
    UINT8     ucAgrtSpDuration;
    UINT8     ucBssIndex;
    UINT32    u4AgrtSpStartTsf_low;
    UINT32    u4AgrtSpStartTsf_high;
    UINT16    u2AgrtSpWakeIntvlMantissa;
    UINT8     ucAgrtSpWakeIntvlExponent;
    UINT8     fgIsRoleAp;
    UINT8     ucAgrtParaBitmap;
    UINT8     ucReserved_a;
    UINT16    u2Reserved_b;
    UINT8     ucGrpMemberCnt;
    UINT8     ucReserved_c;
    UINT16    u2Reserved_d;
    UINT16    au2StaList[TWT_HW_GRP_MAX_MEMBER_CNT];
} UNI_CMD_TWT_ARGT_UPDATE_T, *P_UNI_CMD_TWT_ARGT_UPDATE_T;
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
/* ============== UNI_CMD_ID_TWT End ============== */

/* ======== UNI_CMD_ID_REPT_MUAR & UNI_CMD_ID_NORM_MUAR Begin ======== */
/* NORM/REPT_MUAR command Tag */
typedef enum _UNI_CMD_MUAR_TAG_T {
    UNI_CMD_MUAR_CLEAN = 0, /* No used by Rebb */
    UNI_CMD_MUAR_MC_FILTER = 1, /* No used by Rebb */
    UNI_CMD_MUAR_ENTRY = 2,
    UNI_CMD_MUAR_MAX_NUM
} UNI_CMD_MUAR_TAG_T;

typedef struct GNU_PACKED _UNI_CMD_MUAR_T {
    /* fixed field */
    UINT8 ucBand;       /* operation band */
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_MUAR_T, *P_UNI_CMD_MUAR_T;

/* MC Config Parameters (Tag2) */
typedef struct GNU_PACKED _UNI_CMD_MUAR_ENTRY_T {
    UINT16 u2Tag;             /* should be 0x02 */
    UINT16 u2Length;          /* the length of this TLV, should be 8 */
    BOOLEAN fgSmesh;           /* future used. TRUE: config smart mesh operation, FALSE: config multicast operation */
    UINT8 ucHwBssIndex;       /* hw bss index */
    UINT8 ucMuarIdx;          /* config MUAR table index, set 0xFF means fw auto search index and record */
    UINT8 ucEntryAdd;         /* set TRUE means add one, FALSE means to remove one */
    UINT8 aucMacAddr[6];      /* config mac address in MUAR table */
    UINT8 aucPadding[2];
} UNI_CMD_MUAR_ENTRY_T, *P_UNI_CMD_MUAR_ENTRY_T;
/* ======== UNI_CMD_ID_REPT_MUAR & UNI_CMD_ID_NORM_MUAR End ======== */

/* ======== UNI_CMD_ID_GET_MAC_INFO Begin ======== */
/* Get mac info command (0x1A) */
typedef struct GNU_PACKED _UNI_CMD_GET_MAC_INFO_T {
	/* fixed field */
	UINT8 ucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
							*
							*   TAG                      | ID  | structure
							*   -------------------------|-----|--------------
							*   UNI_CMD_MAC_INFO_TSF     | 0x0 | UNI_CMD_MAC_INFO_TSF_T
							*/
} UNI_CMD_GET_MAC_INFO_T, *P_UNI_CMD_GET_MAC_INFO_T;

/* Get mac info command TLV List */
typedef enum _UNI_CMD_MAC_INFO_TAG_T {
    UNI_CMD_MAC_INFO_TSF = 0,
    UNI_CMD_MAC_INFO_MAX_NUM
} UNI_CMD_MAC_INFO_TAG_T;

/* Get tsf time (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_MAC_INFO_TSF_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucDbdcIdx;
    UINT8 ucHwBssidIndex;
    UINT8 aucPadding[2];
} UNI_CMD_MAC_INFO_TSF_T, *P_UNI_CMD_MAC_INFO_TSF_T;
/* ======== UNI_CMD_ID_GET_MAC_INFO End ======== */

/* ======== UNI_CMD_ID_TXCMD_CTRL Begin ======== */
/* TXCMD Ctrl command (0x1D) */
typedef struct GNU_PACKED _UNI_CMD_TXCMD_CTRL_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_TXCMD_CTRL_T, *P_UNI_CMD_TXCMD_CTRL_T;

/* TXCMD ctrl cmd tags */
typedef enum _UNI_CMD_TXCMD_CTRL_TAG_T {
    UNI_CMD_SET_TXCMD_DBG_CTRL = 0x00,
    UNI_CMD_SET_TXCMD_DBG_CLEAR = 0x01,
    UNI_CMD_SET_TXCMD_DBG_SXN_GLOBAL = 0x02,
    UNI_CMD_SET_TXCMD_DBG_SXN_PROTECT = 0x03,
    UNI_CMD_SET_TXCMD_DBG_SXN_PROTECT_RUINFO = 0x04,
    UNI_CMD_SET_TXCMD_DBG_SXN_TXDATA = 0x05,
    UNI_CMD_SET_TXCMD_DBG_SXN_TXDATA_USER_INFO = 0x06,
    UNI_CMD_SET_TXCMD_DBG_SXN_TRIGDATA = 0x07,
    UNI_CMD_SET_TXCMD_DBG_SXN_TRIGDATA_USER_ACK_INFO = 0x08,
    UNI_CMD_SET_TXCMD_DBG_TF_TXD = 0x09,
    UNI_CMD_SET_TXCMD_DBG_TF_BASIC = 0x0a,
    UNI_CMD_SET_TXCMD_DBG_TF_BASIC_USER = 0x0b,
    UNI_CMD_SET_TXCMD_DBG_SXN_SW_FID = 0x0c,
    UNI_CMD_SET_TXCMD_DBG_SXN_SW_FID_INFO = 0x0d,
    UNI_CMD_SET_TXCMD_DBG_SW_FID_TXD = 0x0e,
    UNI_CMD_GET_TXCMD_DBG_STATUS = 0x0f,
    UNI_CMD_GET_TXCMD_DBG_SXN_GLOBAL = 0x10,
    UNI_CMD_GET_TXCMD_DBG_SXN_PROTECT = 0x11,
    UNI_CMD_GET_TXCMD_DBG_SXN_TXDATA = 0x12,
    UNI_CMD_GET_TXCMD_DBG_SXN_TRIGDATA = 0x13,
    UNI_CMD_GET_TXCMD_DBG_TF_TXD = 0x14,
    UNI_CMD_GET_TXCMD_DBG_TF_BASIC = 0x15,
    UNI_CMD_GET_TXCMD_DBG_SXN_SW_FID = 0x16,
    UNI_CMD_GET_TXCMD_DBG_SW_FID_TXD = 0x17,
    UNI_CMD_SET_TXCMD_DBG_SOP = 0x18,
    UNI_CMD_TXCMD_CTRL_MAX_NUM
} UNI_CMD_TXCMD_CTRL_TAG_T;

/* Txcmd ctrl Parameters (Tag) */
typedef struct GNU_PACKED _UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T {
    UINT16 u2Tag;    /* Tag = 0x00 */
    UINT16 u2Length;

    UINT8  ucUserIndex;
    UINT8  ucDlUlidx;
    UINT8  ucRsv[2];

    UINT8  aucBuffer[0];
} UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T, *P_UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T;

typedef struct GNU_PACKED _UNI_CMD_SET_TXCMD_DBG_CTRL_ENTRY_T {
	UINT16 u2Tag;
    UINT8 ucUserIndex;
    UINT8 ucDlUlidx;

	UINT32 u4DataLen;
    UINT8 *pData;
} UNI_CMD_SET_TXCMD_DBG_CTRL_ENTRY_T, *P_UNI_CMD_SET_TXCMD_DBG_CTRL_ENTRY_T;

typedef INT32 (*PFN_SET_TXCMD_DBG_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
												P_UNI_CMD_SET_TXCMD_DBG_CTRL_ENTRY_T pParam,
												VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TXCMD_DBG_CTRL_PARAM_T {
	BOOLEAN bQuery; /* for Query */
	UNI_CMD_SET_TXCMD_DBG_CTRL_ENTRY_T SetTxCmdDbgEntry[UNI_CMD_TXCMD_CTRL_MAX_NUM];
	BOOLEAN TxCmdDbgCtrlValid[UNI_CMD_TXCMD_CTRL_MAX_NUM];
} UNI_CMD_TXCMD_DBG_CTRL_PARAM_T, *P_UNI_CMD_TXCMD_DBG_CTRL_PARAM_T;

/* ======== UNI_CMD_ID_TXCMD_CTRL End ======== */

/* =================== UNI_CMD_ID_ECC_OPER Begin =================== */
/* ECC operation cmd tags */
typedef enum _UNI_CMD_ECC_OP_TAG_T {
    UNI_CMD_ECC_OP_CAL_GROUP_POINT   = 0x00,
    UNI_CMD_ECC_OP_MAX_NUM
} UNI_CMD_ECC_OP_TAG_T;

/* This structure is used for UNI_CMD_ID_ECC_OPER command (0x18) to calculate ECC key */
typedef struct GNU_PACKED _UNI_CMD_ECC_OP_T {
    /* fixed field */
    UINT8 aucReserved[4];  /*reserved fixed field*/

    /* tlv */
    UINT8 aucTlvBuffer[0];  /*TLVs*/
} UNI_CMD_ECC_OP_T, *P_UNI_CMD_ECC_OP_T;

/* ECC Operation Parameters (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_ECC_OP_CAL_T {
    UINT16 u2Tag;          /* Tag = 0x00 */
    UINT16 u2Length;       /* the length of this TLV */

    UINT8 ucGroupID;       /* group idx */
    UINT8 ucDataLength;    /* it means the length of total */
    UINT8 ucDataType;      /* 0: only scalar(DG mode in HW), 1: scalar and point(x and y)(DQ mode in HW) */
    UINT8 ucEccCmdId;      /* Ecc cmd queue idx */
    UINT8 aucBuffer[0];    /* key data */
} UNI_CMD_ECC_OP_CAL_T, *P_UNI_CMD_ECC_OP_CAL_T;
/* ==================== UNI_CMD_ID_ECC_OPER End ==================== */

/* =================== UNI_CMD_ID_RDD_ON_OFF_CTRL Begin =================== */
/* RDD set command (0x19) */
typedef struct GNU_PACKED _UNI_CMD_RDD_T {
    /*fixed field*/
    UINT8 aucPadding[4];
    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_RDD_T, *P_UNI_CMD_RDD_T;

/* RDD on off command (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_RDD_ON_OFF_CTRL_PARM_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 u1DfsCtrl;
    UINT8 u1RddIdx;
    UINT8 u1RddRxSel;
    UINT8 u1SetVal;
    UINT8 aucReserve[4];
} UNI_CMD_RDD_ON_OFF_CTRL_PARM_T, *P_UNI_CMD_RDD_ON_OFF_CTRL_PARM_T;

/* RDD set command Tag */
typedef enum _UNI_CMD_RDD_TAG_T {
    UNI_CMD_RDD_ON_OFF_CTRL_PARM = 0,
    UNI_CMD_RDD_MAX_NUM
} UNI_CMD_RDD_TAG_T;

/* ==================== UNI_CMD_ID_RDD_ON_OFF_CTRL End ==================== */


/* ====================== UNI_CMD_ID_MIB Begin ======================*/
/* MIB command Tag */
typedef enum _UNI_CMD_MIB_TAG_T {
    UNI_CMD_MIB_DATA = 0,
    UNI_CMD_MIB_MAX_NUM
} UNI_CMD_MIB_TAG_T;

/* MIB command (0x22) */
typedef struct GNU_PACKED _UNI_CMD_MIB_T {
    /* fixed field */
    UINT8 ucBand;          /* operation band */
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_MIB_T, *P_UNI_CMD_MIB_T;

/* MIB Config Parameters (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_MIB_DATA_T {
    UINT16 u2Tag;      /* should be 0x00 */
    UINT16 u2Length;   /* the length of this TLV */
    UINT32 u4Counter;  /* MIB ID of demanded MIB counter */
} UNI_CMD_MIB_DATA_T, *P_UNI_CMD_MIB_DATA_T;

/* ======================= UNI_CMD_ID_MIB End =======================*/

/* ================= UNI_CMD_ID_SNIFFER_MODE Begin ================= */
/* Sniffer mode command TLV List */
typedef enum _UNI_CMD_SNIFFER_MODE_TAG_T {
    UNI_CMD_SNIFFER_MODE_ENABLE = 0,
    UNI_CMD_SNIFFER_MODE_CONFIG = 1,    /* rebb don't need*/
    UNI_CMD_SNIFFER_MODE_MAX_NUM
} UNI_CMD_SNIFFER_MODE_TAG_T;

/* Sniffer mode command (0x24) */
typedef struct GNU_PACKED _UNI_CMD_SNIFFER_MODE_T {
    /* fixed field */
    UINT8 ucBandIdx;       /* the band index */
    UINT8 ucReserved[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
							*   TAG                        | ID  | structure
							*   ---------------------------|-----|--------------
							*   UNI_CMD_SNIFFER_MODE_ENABLE | 0x00 | UNI_CMD_SNIFFER_MODE_ENABLE_T
							*   UNI_CMD_SNIFFER_MODE_CONFIG | 0x01 | UNI_CMD_SNIFFER_MODE_CONFIG_T
							*/
} UNI_CMD_SNIFFER_MODE_T, *P_UNI_CMD_SNIFFER_MODE_T;


/* Set sniffer mode parameters (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_SNIFFER_MODE_ENABLE_T {
    UINT16 u2Tag;      /* should be 0x00 */
    UINT16 u2Length;   /* should be sizeof(UNI_CMD_SNIFFER_MODE_ENABLE_T) */
    UINT8 ucSNEnable;  /* 0: disable, 1: enable */
    UINT8 aucPadding[3];
} UNI_CMD_SNIFFER_MODE_ENABLE_T, *P_UNI_CMD_SNIFFER_MODE_ENABLE_T;

/* Set sniffer mode parameters (Tag1) */
typedef struct GNU_PACKED _UNI_CMD_SNIFFER_MODE_CONFIG_T {
    UINT16 u2Tag;      /* should be 0x01 */
    UINT16 u2Length;   /* should be sizeof(UNI_CMD_SNIFFER_MODE_CONFIG_T) */
    UINT16 u2Aid;      /* the association ID */
    UINT8 ucBand;      /* the starting freq of the band (the unit is kHz) */
    UINT8 ucChannelWidth;  /* the channel bandwidth */
    UINT8 ucPriChannel;    /* primary channel (central channel on BW20) */
    UINT8 ucSco;           /* the channel offset used in BW40 */
    UINT8 ucChannelS1;     /* central channel 1 (used for BW80/BW160: SX center freq, BW80+80: SX lower center freq) */
    UINT8 ucChannelS2;     /* central channel 2 (used for BW80+80: SX higher center freq) */
    BOOLEAN fgDropFcsErrorFrame; /* TRUE: drop FCS error frame, FALSE: receive FCS error frame */
    UINT8 aucPadding[3];
} UNI_CMD_SNIFFER_MODE_CONFIG_T, *P_UNI_CMD_SNIFFER_MODE_CONFIG_T;

/* ================== UNI_CMD_ID_SNIFFER_MODE End ================== */

/* ================= UNI_CMD_ID_SCS Begin ================= */
typedef struct GNU_PACKED _UNI_CMD_SCS_T {
    /* fixed field */
    UINT8 ubandid;
    UINT8 ucReserved[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_SCS_T, *P_UNI_CMD_SCS_T;

/* SCS Tag value */
typedef enum _UNI_CMD_SCS_CTRL_TAG_T {
    UNI_CMD_SCS_EVENT_SEND_DATA = 0,
    UNI_CMD_SCS_EVENT_GET_GLO_ADDR,
    UNI_CMD_SCS_EVENT_SET_PD_THR_RANGE,
    UNI_CMD_SCS_EVENT_SCS_ENABLE,
    UNI_CMD_SCS_MAX_EVENT
} UNI_CMD_SCS_CTRL_TAG_T;

/* UNI_CMD_SCS_EVENT_SEND_DATA (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2 {
    /* DW0 */
    UINT16 u2Tag;
    UINT16 u2Length;
    /* DW1 - DW2 */
    UINT16 u2ActiveSTA;
    UINT16 u2eTput;
    UINT8 fgRxOnly;
    UINT8 fgPDreset;
    INT8  i1SCSMinRSSI;
    UINT8 ucReserved;
} UNI_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2, *P_UNI_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2;

typedef INT32 (*PFN_SCS_SEND_DATA_HANDLE)(struct _RTMP_ADAPTER *pAd,
											P_UNI_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2 pParam,
											VOID *pHandle);

/* UNI_CMD_SCS_EVENT_GET_GLO_ADDR (Tag1) */
typedef struct GNU_PACKED _UNI_CMD_SCS_GET_GLO_ADDR_CTRL {
    /* DW0 */
    UINT16 u2Tag;
    UINT16 u2Length;
} UNI_CMD_SCS_GET_GLO_ADDR_CTRL, *P_UNI_CMD_SCS_GET_GLO_ADDR_CTRL;

typedef INT32 (*PFN_SCS_GET_GLO_ADDR_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
											P_UNI_CMD_SCS_GET_GLO_ADDR_CTRL pParam,
											VOID *pHandle);

/* UNI_CMD_SCS_EVENT_SET_PD_THR_RANGE (Tag2) */
typedef struct GNU_PACKED _UNI_CMD_SET_SCS_PD_THR_RANGE {
    /* DW0 */
    UINT16 u2Tag;
    UINT16 u2Length;
    /* DW1 - DW2 */
    UINT16 u2CckPdThrMax;
    UINT16 u2OfdmPdThrMax;
    UINT16 u2CckPdThrMin;
    UINT16 u2OfdmPdThrMin;
} UNI_CMD_SET_SCS_PD_THR_RANGE, *P_UNI_CMD_SET_SCS_PD_THR_RANGE;

typedef INT32 (*PFN_SCS_PD_THR_RANGE_HANDLE)(struct _RTMP_ADAPTER *pAd,
											P_UNI_CMD_SET_SCS_PD_THR_RANGE pParam,
											VOID *pHandle);

/* UNI_CMD_SCS_EVENT_SET_MANUAL_PD_TH (Tag3) */
typedef struct GNU_PACKED _UNI_CMD_SMART_CARRIER_ENABLE {
    /* DW0 */
    UINT16 u2Tag;
    UINT16 u2Length;
    /* DW1 */
    UINT8 u1SCSEnable;
    UINT8 ucRreserved[3];
} UNI_CMD_SMART_CARRIER_ENABLE, *P_UNI_CMD_SMART_CARRIER_ENABLE;

typedef INT32 (*PFN_SCS_SMART_CARRIER_ENABLE_HANDLE)(struct _RTMP_ADAPTER *pAd,
											P_UNI_CMD_SMART_CARRIER_ENABLE pParam,
											VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_SCS_PARAM_T {
	UINT8 ucDbdcIdx;
	BOOLEAN bQuery;
	BOOLEAN SCSTagValid[UNI_CMD_SCS_MAX_EVENT];

	UNI_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2 SCSSendData;
	UNI_CMD_SCS_GET_GLO_ADDR_CTRL SCSGetGloAddrCtrl;
	UNI_CMD_SET_SCS_PD_THR_RANGE SCSPDThrRange;
	UNI_CMD_SMART_CARRIER_ENABLE SCSEnable;
} UNI_CMD_SCS_PARAM_T, *P_UNI_CMD_SCS_PARAM_T;

/* ================= UNI_CMD_ID_SCS End ================= */

/* =================== UNI_CMD_ID_DVT Begin =================== */
typedef struct GNU_PACKED _UNI_CMD_DVT_CONFIG_T {
	/*fixed field*/
	UINT8 ucTestType;
	UINT8 aucPadding[3];

	/* tlv */
	UINT8 aucTlvBuffer[0]; /**< the TLVs included in this field:
							*
							*   TAG                              | ID  | structure
							*   -------------                    | ----| -------------
							*   UNI_CMD_MDVT_SET_PARA            | 0x0 | UNI_CMD_MDVT_SET_PARA_T
							*/
} UNI_CMD_DVT_CONFIG_T, *P_UNI_CMD_DVT_CONFIG_T;

/* dvt config Tag */
typedef enum _UNI_CMD_DVT_TYPE_T {
    UNI_CMD_MODULE_DVT = 0,
    UNI_CMD_DVT_MAX_NUM
} UNI_CMD_DVT_TYPE_T;

/* module dvt config Tag */
typedef enum _UNI_CMD_DVT_TAG_T {
    UNI_CMD_MDVT_SET_PARA = 0,
    UNI_CMD_DVT_TAG_MAX_NUM
} UNI_CMD_DVT_TAG_T;

/* MDVT Parameter Setting (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_MDVT_SET_PARA_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT16 u2ModuleId;
    UINT16 u2CaseId;
} UNI_CMD_MDVT_SET_PARA_T, *P_UNI_CMD_MDVT_SET_PARA_T;

/* =================== UNI_CMD_ID_DVT End =================== */

/* =================== UNI_CMD_ID_GPIO Begin =================== */
typedef struct GNU_PACKED _UNI_CMD_GPIO_CONFIG_T {
	/*fixed field*/
	UINT8 ucGpioIdx;
	UINT8 aucPadding[3];

	/* tlv */
	UINT8 aucTlvBuffer[0]; /**< the TLVs included in this field:
							*
							*   TAG                     | ID  | structure
							*   -------------           | ----| -------------
							*   UNI_CMD_GPIO_ENABLE     | 0x0 | UNI_CMD_GPIO_ENABLE_T
							*   UNI_CMD_GPIO_SET_VALUE  | 0x1 | UNI_CMD_GPIO_SET_VALUE_T
							*/
} UNI_CMD_GPIO_CONFIG_T, *P_UNI_CMD_GPIO_CONFIG_T;

/* Gpio config Tag */
typedef enum _UNI_CMD_GPIO_TAG_T {
    UNI_CMD_GPIO_ENABLE = 0,
    UNI_CMD_GPIO_SET_VALUE = 1,
    UNI_CMD_GPIO_TAG_MAX_NUM
} UNI_CMD_GPIO_TAG_T;

/* MDVT Parameter Setting (Tag0) */
typedef struct GNU_PACKED _UNI_CMD_GPIO_ENABLE_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    BOOLEAN fgEnable;
    UINT8 aucReserved[3];
} UNI_CMD_GPIO_ENABLE_T, *P_UNI_CMD_GPIO_ENABLE_T;

typedef INT32 (*PFN_GPIO_ENABLE_HANDLE)(struct _RTMP_ADAPTER *pAd,
											P_UNI_CMD_GPIO_ENABLE_T pParam,
											VOID *pHandle);

/* MDVT Parameter Setting (Tag1) */
typedef struct GNU_PACKED _UNI_CMD_GPIO_SET_VALUE_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucGpioValue;
    UINT8 aucReserved[3];
} UNI_CMD_GPIO_SET_VALUE_T, *P_UNI_CMD_GPIO_SET_VALUE_T;

typedef INT32 (*PFN_GPIO_SET_VALUE_HANDLE)(struct _RTMP_ADAPTER *pAd,
											P_UNI_CMD_GPIO_SET_VALUE_T pParam,
											VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_GPIO_CFG_PARAM_T {
	UINT8 ucGpioIdx;
	BOOLEAN GpioCfgValid[UNI_CMD_GPIO_TAG_MAX_NUM];

	UNI_CMD_GPIO_ENABLE_T GpioEnable;
	UNI_CMD_GPIO_SET_VALUE_T GpioSetValue;
} UNI_CMD_GPIO_CFG_PARAM_T, *P_UNI_CMD_GPIO_CFG_PARAM_T;

/* =================== UNI_CMD_ID_GPIO End =================== */

/* =================== UNI_CMD_ID_MURU Begin =================== */

typedef INT32 (*PFN_MURU_HANDLE)(struct _RTMP_ADAPTER *pAd, IN RTMP_STRING *arg, VOID *pHandle);

enum UNI_CMD_MURU_TAG_T {
	UNI_CMD_MURU_BSRP_CTRL = 0x01,
	UNI_CMD_MURU_SET_ARB_OP_MODE = 0xe,
	UNI_CMD_MURU_SUTX_CTRL = 0x10,
	UNI_CMD_MURU_FIXED_RATE_CTRL = 0x11,
	UNI_CMD_MURU_FIXED_GROUP_RATE_CTRL = 0x12,
	UNI_CMD_MURU_DBG_INFO = 0x18,

	/* HQA AP commands offset 100 */
	UNI_CMD_MURU_MANUAL_CONFIG = 0x64,

	UNI_CMD_MURU_SET_MUDL_ACK_POLICY = 0xC8,
	UNI_CMD_MURU_SET_TRIG_TYPE = 0xC9,
	UNI_CMD_MURU_SET_20M_DYN_ALGO = 0xCA,
	UNI_CMD_MURU_PROT_FRAME_THR = 0xCC,
	UNI_CMD_MURU_SET_TXOP_ONOFF = 0xD2,
	UNI_CMD_MURU_UL_ONOFF = 0xD3,
	UNI_CMD_MURU_MAX
};

enum UNI_CMD_MURU_VER_T {
	/* Command Version */
	UNI_CMD_MURU_VER_LEG = 0,
	UNI_CMD_MURU_VER_HE,
	UNI_CMD_MURU_VER_EHT,
	UNI_CMD_MURU_VER_MAX
};

#define UNI_MAX_MCS_SUPPORT_HE      11
#define UNI_MAX_MCS_SUPPORT_EHT     13

struct GNU_PACKED UNI_CMD_MURU_BSRP_CTRL_T {
	/* DW_0 */
	UINT16     u2Tag;
	UINT16     u2Length;
	/* DW_1*/
	UINT16     u2BsrpInterval;
	UINT16     u2BsrpRuAlloc;
	/* DW_2*/
	UINT32     u4TriggerType; /*@us*/
	/* DW_3*/
	UINT8      u1TriggerFlow; /*0: normal, 1: kick-and-stop, 2: stop, 3: CheckULQueueLen*/
	UINT8      fgExtCmdBsrp; /* TRUE: timer control by ext cmd */
	UINT8      u1Reserved[2];
};

#define UNI_MURU_MAX_NUM_TXCMD_TX_USER       (16)

struct GNU_PACKED UNI_MURU_DL_USER_INFO {
	UINT16     u2WlanIdx;
	UINT8      u1RuAllocBn;
	UINT8      u1RuAllocIdx;

	UINT8      u1Ldpc;
	UINT8      u1Nss;
	UINT8      u1Mcs;
	UINT8      u1MuGroupIdx;

	UINT8      u1VhtGid;
	UINT8      u1VhtUp;
	UINT8      u1HeStartStream;
	UINT8      u1HeMuMimoSpatial;

	UINT16     u2TxPwrAlpha;
	UINT8      u1AckPolicy;
	UINT8      u1RuAllocPs160;
};

struct GNU_PACKED UNI_MURU_UL_USER_INFO {
	UINT16     u2WlanIdx;
	UINT8      u1RuAllocBn;
	UINT8      u1RuAllocIdx;

	UINT8      u1Ldpc;
	UINT8      u1Nss;
	UINT8      u1Mcs;
	UINT8      u1TargetRssi;

	UINT32     u4TrigPktSize;
	UINT8      u1RuAllocPs160;
	UINT8      au1Reserved[3];
};

struct GNU_PACKED UNI_MURU_CMM_MANUAL_CONFIG {
	UINT8      u1PdaPol;
	UINT8      u1Band;
	UINT8      u1SpeIdx;
	UINT8      u1ProcType;

	UINT16     u2MloCtrl;
	UINT8      u1SchType;
	UINT8      u1PpduFmt;
	UINT8      u1WmmSet;
	UINT8      au1Reserved[3];
};

struct GNU_PACKED UNI_MURU_DL_MANUAL_CONFIG {
	UINT8      u1UserCnt;
	UINT8      u1TxMode;
	UINT8      u1Bw;
	UINT8      u1GI;

	UINT8      u1Ltf;
	UINT8      u1SigMcs;
	UINT8      u1SigDcm;
	UINT8      u1SigCmprs;

	UINT16     au2RU[16];

	UINT8      au1C26[2];
	UINT8      u1AckPly;
	UINT8      u1TxPwr;

	UINT16     u2MuPpduDur;
	UINT8      u1AgcDispOrder;
	UINT8      u1Reserved;

	UINT8      u1AgcDispPol;
	UINT8      u1AgcDispRatio;
	UINT16     u2AgcDispLinkMGF;

	struct UNI_MURU_DL_USER_INFO   arUserInfoDl[UNI_MURU_MAX_NUM_TXCMD_TX_USER];
};

struct GNU_PACKED UNI_MURU_UL_MANUAL_CONFIG {
	UINT8      u1UserCnt;
	UINT8      u1TxMode;

	/* DLTX */
	UINT8      u1BaType;
	UINT8      u1Reserved;

	/* ULTX */
	UINT8      u1UlBw;
	UINT8      u1UlGiLtf;
	UINT16     u2UlLength;

	UINT16     u2TrigCnt;
	UINT8      u1TfPad;
	UINT8      u1TrigType;

	UINT16     u2TrigIntv;
	UINT8      u1TrigTa[MAC_ADDR_LEN];
	UINT16     au2UlRU[16];

	UINT8      au1UlC26[2];
	UINT16     u2AgcDispLinkMGF;

	UINT8      u1AgcDispMuLen;
	UINT8      u1AgcDispPol;
	UINT8      u1AgcDispRatio;
	UINT8      u1AgcDispPuIdx;

	struct UNI_MURU_UL_USER_INFO   arUserInfoUl[UNI_MURU_MAX_NUM_TXCMD_TX_USER];
};

struct GNU_PACKED UNI_MURU_DBG_MANUAL_CONFIG {
	/* HE TB RX Debug */
	UINT32     u4RxHetbNonsfEnBitmap; /* Maximum user:16 */
	UINT32     au4RxHetbCfg[2];
};

struct GNU_PACKED UNI_CMD_MURU_T {
	/*Fixed Fields*/
	UINT8 au1Padding[4];
	/*TLV*/
	UINT8 au1TlvBuffer[0];
};

struct GNU_PACKED UNI_MURU_MANUAL_CONFIG_T {
	UINT32     u4ManCfgBmpCmm;
	UINT32     u4ManCfgBmpDl;
	UINT32     u4ManCfgBmpUl;
	UINT32     u4ManCfgBmpDbg;

	struct UNI_MURU_CMM_MANUAL_CONFIG  rCfgCmm;
	struct UNI_MURU_DL_MANUAL_CONFIG   rCfgDl;
	struct UNI_MURU_UL_MANUAL_CONFIG   rCfgUl;
	struct UNI_MURU_DBG_MANUAL_CONFIG  rCfgDbg;
};

struct GNU_PACKED UNI_CMD_MURU_MANUAL_CONFIG_T {
	/* DW_0 */
	UINT16 u2Tag;
	UINT16 u2Length;
	/* DW_1*/
	UINT8   u1CmdVersion;
	UINT8   u1CmdRevision;
	UINT16  u2Reserved;

	struct UNI_MURU_MANUAL_CONFIG_T rMuruManCfg;
};

struct GNU_PACKED UNI_MURU_MUM_SET_GROUP_TBL_ENTRY {
	UINT16      u2WlidUser0; /* WLANID0 */
	UINT16      u2WlidUser1; /* WLANID1 */
	UINT16      u2WlidUser2; /* WLANID2 */
	UINT16      u2WlidUser3; /* WLANID3 */

	UINT8       u1DlMcsUser0: 4;
	UINT8       u1DlMcsUser1: 4;
	UINT8       u1DlMcsUser2: 4;
	UINT8       u1DlMcsUser3: 4;
	UINT8       u1UlMcsUser0: 4;
	UINT8       u1UlMcsUser1: 4;
	UINT8       u1UlMcsUser2: 4;
	UINT8       u1UlMcsUser3: 4;

	UINT8       u1NumUser: 2;
	UINT8       u1Res: 6;
	UINT8       u1Nss0: 2;
	UINT8       u1Nss1: 2;
	UINT8       u1Nss2: 2;
	UINT8       u1Nss3: 2;
	UINT8       u1RuAlloc;
	UINT8       u1RuAllocExt;

	UINT8       u1Capability;
	UINT8       u1GI;
	UINT8       u1Dl_Ul;
	UINT8       u1Reserved2;
};

struct GNU_PACKED UNI_CMD_MURU_FIXED_GRP_RATE_CTRL_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1CmdVersion;
	UINT8    u1CmdRevision;
	UINT16   u2Reserved;

	struct UNI_MURU_MUM_SET_GROUP_TBL_ENTRY rMuruSetGrpTblEntry;
};

struct GNU_PACKED UNI_CMD_MURU_FIXED_RATE_CTRL_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT16   u2Value;
	UINT16   u2Reserved;
};

struct GNU_PACKED UNI_CMD_MURU_SET_ARB_OP_MODE_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1OpMode;
	UINT8    au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_SET_SUTX_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1ForceSuTx;
	UINT8    au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_SET_DBG_INFO {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1 */
	UINT16   u2Item;
	UINT8    au1Reserved[2];
	UINT32   u4Value;
};

struct GNU_PACKED UNI_CMD_MURU_SHOW_ULRU_STATUS_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
};

struct GNU_PACKED UNI_CMD_MURU_SET_POLICY_TYPE_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1AckPolicy;
	UINT8    au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_SET_TRIG_TYPE_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1TrigValue;
	UINT8    au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_SET_20M_DYN_ALGO_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1DynAlgoEnable;
	UINT8    au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_SET_PROT_FRAME_THR_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT32   u4ProtFrameThr;
};

struct GNU_PACKED UNI_CMD_MURU_SET_TXOP_ONOFF_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT8    u1TxopOnOff;
	UINT8    au1Reserved[3];
};

struct GNU_PACKED UNI_CMD_MURU_UL_ONOFF_T {
	/* DW_0 */
	UINT16   u2Tag;
	UINT16   u2Length;
	/* DW_1*/
	UINT16   u2UlBsrpOnOff;
	UINT16   u2UlDataOnOff;
};

/* =================== UNI_CMD_ID_MURU End =================== */

/* =================== UNI_CMD_ID_PP Begin =================== */
typedef struct GNU_PACKED _UNI_CMD_PP_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_PP_T, *P_UNI_CMD_PP_T;

typedef enum _UNI_CMD_ID_PP_TAG_T {
    UNI_CMD_PP_EN_CTRL = 0x0,
    UNI_CMD_PP_NUM
} UNI_CMD_ID_PP_TAG_T;

typedef struct GNU_PACKED _UNI_CMD_PP_EN_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8  u1DbdcIdx;
    UINT8  u1PpCtrl;
    UINT8  u1PpAutoMode;
    UINT8  u1Reserved;
} UNI_CMD_PP_EN_CTRL_T, *P_UNI_CMD_PP_EN_CTRL_T;

typedef INT32 (*PFN_PP_EN_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
										P_PP_CMD_T pp_cmd_cap, VOID *pHandle);

/* =================== UNI_CMD_ID_PP End =================== */

/* =================== UNI_CMD_ID_TPC Begin =================== */
typedef struct GNU_PACKED _UNI_CMD_TPC_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_CMD_TPC_T, *P_UNI_CMD_TPC_T;

typedef enum _UNI_CMD_ID_TPC_TAG_T {
    UNI_CMD_TPC_ACT_MANUAL_MODE = 0x0,
    UNI_CMD_TPC_ACT_UL_TX_POWER_CONFIG = 0x1,
    UNI_CMD_TPC_ACT_UL_TARGET_RSSI_CONFIG = 0x2,
    UNI_CMD_TPC_ACT_UL_UPH_MIN_PWR_FG_CONFIG = 0x3,
    UNI_CMD_TPC_ACT_DL_TX_POWER_CMD_CTRL_CONFIG = 0x4,
    UNI_CMD_TPC_ACT_DL_TX_POWER_CONFIG = 0x5,
    UNI_CMD_TPC_ACT_DL_TX_POWER_ALPHA_CONFIG = 0x6,
    UNI_CMD_TPC_ACT_MAN_TBL_INFO = 0x7,
    UNI_CMD_TPC_ACT_WLANID_CTRL = 0x8,
    UNI_CMD_TPC_ACT_UL_UNIT_TEST_CONFIG = 0x9,
    UNI_CMD_TPC_ACT_UL_UNIT_TEST_GO = 0xA,
    UNI_CMD_TPC_ACT_ENABLE_CONFIG = 0xB,
    UNI_CMD_TPC_ALGO_ACTION_NUM
} UNI_CMD_ID_TPC_TAG_T;

/** enum for tpc parameter mode */
typedef enum _UNI_ENUM_TPC_PARAM_MODE {
    UNI_TPC_PARAM_AUTO_MODE = 0,
    UNI_TPC_PARAM_MAN_MODE,
    UNI_TPC_PARAM_MODE_NUM
} UNI_ENUM_TPC_PARAM_MODE, *P_UNI_ENUM_TPC_PARAM_MODE;

/** enum for down-link tx type */
typedef enum _UNI_ENUM_TPC_DL_TX_TYPE {
    UNI_TPC_DL_TX_TYPE_MU_MIMO = 0,
    UNI_TPC_DL_TX_TYPE_MU_OFDMA,
    UNI_TPC_DL_TX_TYPE_NUM
} UNI_ENUM_TPC_DL_TX_TYPE, *P_UNI_ENUM_TPC_DL_TX_TYPE;

typedef struct GNU_PACKED _UNI_CMD_TPC_MAN_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCtrlFormatId;
    BOOLEAN fgTpcEnable;
    UINT8 u1Reserved[2];
    UNI_ENUM_TPC_PARAM_MODE eTpcParamMode;
} UNI_CMD_TPC_MAN_CTRL_T, *P_UNI_CMD_TPC_MAN_CTRL_T;

typedef INT32 (*PFN_TPC_MAN_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_TPC_MAN_CTRL_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TPC_UL_ALGO_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCtrlFormatId;
    UINT8 u1ApTxPwr;
    UINT8 u1EntryIdx;
    UINT8 u1TargetRssi;
    UINT8 u1UPH;
    BOOLEAN fgMinPwrFlag;
    UINT8 u1Reserved[2];
} UNI_CMD_TPC_UL_ALGO_CTRL_T, *P_UNI_CMD_TPC_UL_ALGO_CTRL_T;

typedef INT32 (*PFN_TPC_UL_ALGO_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_TPC_UL_ALGO_CTRL_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TPC_DL_ALGO_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCtrlFormatId;
    INT8 i1DlTxPwr;
    BOOLEAN fgDlTxPwrCmdCtrl;
    UINT8 u1EntryIdx;
    INT16 i2DlTxPwrAlpha;
    UNI_ENUM_TPC_DL_TX_TYPE eTpcDlTxType;
    UINT8 u1Reserved;
} UNI_CMD_TPC_DL_ALGO_CTRL_T, *P_UNI_CMD_TPC_DL_ALGO_CTRL_T;

typedef INT32 (*PFN_TPC_DL_ALGO_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_TPC_DL_ALGO_CTRL_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TPC_MAN_TBL_INFO_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCtrlFormatId;
    BOOLEAN fgUplink;
    UINT8 u1Reserved[2];
} UNI_CMD_TPC_MAN_TBL_INFO_T, *P_UNI_CMD_TPC_MAN_TBL_INFO_T;

typedef INT32 (*PFN_TPC_MAN_TBL_INFO_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_TPC_MAN_TBL_INFO_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TPC_MAN_WLAN_ID_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCtrlFormatId;
    UINT8 u1EntryIdx;
    UINT16 u2WlanId;
    BOOLEAN fgUplink;
    UNI_ENUM_TPC_DL_TX_TYPE eTpcDlTxType;
    UINT8 u1Reserved[2];
} UNI_CMD_TPC_MAN_WLAN_ID_CTRL_T, *P_UNI_CMD_TPC_MAN_WLAN_ID_CTRL_T;

typedef INT32 (*PFN_TPC_MAN_WLAN_ID_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_TPC_MAN_WLAN_ID_CTRL_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TPC_UL_UT_VAR_CFG_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCtrlFormatId;
    UINT8 u1EntryIdx;
    INT16 i2Value;
    UINT8 u1VarType;
    UINT8 u1Reserved;
} UNI_CMD_TPC_UL_UT_VAR_CFG_T, *P_UNI_CMD_TPC_UL_UT_VAR_CFG_T;

typedef INT32 (*PFN_TPC_UL_UT_VAR_CFG_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_TPC_UL_UT_VAR_CFG_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TPC_UL_UT_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCtrlFormatId;
    BOOLEAN fgTpcUtGo;
    UINT8 u1Reserved[2];
} UNI_CMD_TPC_UL_UT_CTRL_T, *P_UNI_CMD_TPC_UL_UT_CTRL_T;

typedef INT32 (*PFN_TPC_UL_UT_CTRL_HANDLE)(struct _RTMP_ADAPTER *pAd,
													P_UNI_CMD_TPC_UL_UT_CTRL_T pParam,
													VOID *pHandle);

typedef struct GNU_PACKED _UNI_CMD_TPC_PARAM_T {
	BOOLEAN bQuery;
	BOOLEAN TPCTagValid[UNI_CMD_TPC_ALGO_ACTION_NUM];

	UNI_CMD_TPC_MAN_CTRL_T TPCManCtrl;
	UNI_CMD_TPC_UL_ALGO_CTRL_T TPCUlAlgoCtrl;
	UNI_CMD_TPC_DL_ALGO_CTRL_T TPCDlAlgoCtrl;
	UNI_CMD_TPC_MAN_TBL_INFO_T TPCManTblInfo;
	UNI_CMD_TPC_MAN_WLAN_ID_CTRL_T TPCManWlanIDCtrl;
	UNI_CMD_TPC_UL_UT_VAR_CFG_T TPCUlUTVarCfg;
	UNI_CMD_TPC_UL_UT_CTRL_T TPCUlUTCtrl;
} UNI_CMD_TPC_PARAM_T, *P_UNI_CMD_TPC_PARAM_T;

/* =================== UNI_CMD_ID_TPC End =================== */


/* =================== UNI_CMD_ID_VOW Begin ================== */
/* VOW command (0x37) */
typedef struct _UNI_CMD_VOW_T {
	/* fixed field */
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0]; /**<the TLVs includer in this field:
		*
		*  TAG                                       | ID   | structure
		*  ------------------------------------      | -----| -------------
		*  UNI_CMD_VOW_DRR_CTRL                      |  0x0  | UNI_CMD_VOW_DRR_CTRL_T
		*  UNI_CMD_VOW_FEATURE_CTRL                  |  0x1  | UNI_CMD_VOW_FEATURE_CTRL_T
		*  UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP         |  0x2  | UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T
		*  UNI_CMD_VOW_BSSGROUP_TOKEN_CFG            |  0x3  | UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T
		*  UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP       |  0x4  | UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T
		*  UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM     |  0x5  | UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T
		*  UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL |  0x6  | UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T
		*  UNI_CMD_VOW_AT_PROC_EST_FEATURE           |  0x7  | UNI_CMD_VOW_AT_PROC_EST_FEATURE_T
		*  UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD    |  0x8  | UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T
		*  UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO       |  0x9  | UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T
		*  UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING  | 0xA  | UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T
		*  UNI_CMD_VOW_RX_AT_AIRTIME_EN              |  0xB  | UNI_CMD_VOW_RX_AT_AIRTIME_EN_T
		*  UNI_CMD_VOW_RX_AT_MIBTIME_EN              |  0xC  | UNI_CMD_VOW_RX_AT_MIBTIME_EN_T
		*  UNI_CMD_VOW_RX_AT_EARLYEND_EN             |  0xD  | UNI_CMD_VOW_RX_AT_EARLYEND_EN_T
		*  UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN          |  0xE  | UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T
		*  UNI_CMD_VOW_RX_AT_STA_WMM_CTRL            |  0xF  | UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T
		*  UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL           | 0x10  | UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T
		*  UNI_CMD_VOW_RX_AT_ED_OFFSET               | 0x11  | UNI_CMD_VOW_RX_AT_ED_OFFSET_T
		*  UNI_CMD_VOW_RX_AT_SW_TIMER                | 0x12  | UNI_CMD_VOW_RX_AT_SW_TIMER_T
		*  UNI_CMD_VOW_RX_AT_BACKOFF_TIMER           | 0x13  | UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T
		*  UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME  | 0x14  | CMD_TLV_GENERAL_T
		*  UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME     | 0x15  | CMD_TLV_GENERAL_T
		*  UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME    | 0x16  | CMD_TLV_GENERAL_T
		*  UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME  | 0x17  | CMD_TLV_GENERAL_T
		*  UNI_CMD_VOW_RED_ENABLE                    | 0x18  | UNI_CMD_VOW_RED_ENABLE_T
		*  UNI_CMD_VOW_RED_TX_RPT                    | 0x19  | UNI_CMD_VOW_RED_TX_RPT_T
		*/
} UNI_CMD_VOW_T, *P_UNI_CMD_VOW_T;

typedef enum _UNI_CMD_VOW_TAG_T {
	UNI_CMD_VOW_DRR_CTRL = 0x00,
	UNI_CMD_VOW_FEATURE_CTRL = 0x01,
	UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP = 0x02,
	UNI_CMD_VOW_BSSGROUP_TOKEN_CFG = 0x03,
	UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP = 0x04,
	UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM = 0x05,
	UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL = 0x06,
	UNI_CMD_VOW_AT_PROC_EST_FEATURE = 0x07,
	UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD = 0x08,
	UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO = 0x09,
	UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING = 0x0A,
	UNI_CMD_VOW_RX_AT_AIRTIME_EN = 0x0B,
	UNI_CMD_VOW_RX_AT_MIBTIME_EN = 0x0C,
	UNI_CMD_VOW_RX_AT_EARLYEND_EN = 0x0D,
	UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN = 0x0E,
	UNI_CMD_VOW_RX_AT_STA_WMM_CTRL = 0x0F,
	UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL = 0x10,
	UNI_CMD_VOW_RX_AT_ED_OFFSET = 0x11,
	UNI_CMD_VOW_RX_AT_SW_TIMER = 0x12,
	UNI_CMD_VOW_RX_AT_BACKOFF_TIMER = 0x13,
	UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME = 0x14,
	UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME = 0x15,
	UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME = 0x16,
	UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME = 0x17,
	UNI_CMD_VOW_RED_ENABLE = 0x18,
	UNI_CMD_VOW_RED_TX_RPT = 0x19,
	UNI_CMD_VOW_MAX_NUM
} UNI_CMD_VOW_TAG_T;

/* DRR ctrl (Tag 0x00) */
typedef struct _UNI_CMD_VOW_DRR_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2StaID;
	UINT8 ucReserve[2];

	UINT32 u4CtrlFieldID;
	UINT32 u4ComValue;
	UINT8 aucAirTimeQuantum[UNICMD_VOW_AIRTIME_QUANTUM_IDX_TOTAL_NUM];
} UNI_CMD_VOW_DRR_CTRL_T, *P_UNI_CMD_VOW_DRR_CTRL_T;

/* VOW feature ctrl (Tag 0x01) */
typedef struct _UNI_CMD_VOW_FEATURE_CTRL_T {
	UINT16         u2Tag;
	UINT16         u2Length;

	/* DW#0 */
	UINT16 u2IfApplyBss_0_to_16_CtrlFlag;          /* BITWISE */

#ifdef RT_BIG_ENDIAN
	UINT16 u2IfApplyEnbwCtrlFlag: 1;
	UINT16 u2IfApplyEnbwrefillFlag: 1;
	UINT16 u2IfApplyAirTimeFairnessFlag: 1;
	UINT16 u2IfApplyEnTxopNoChangeBssFlag: 1;
	UINT16 u2Reserve_b26_to_b27Flag: 2;
	UINT16 u2IfApplyWeightedAirTimeFairnessFlag: 1;
	UINT16 u2Reserve_b22_to_b24Flag: 3;
	UINT16 u2IfApplyDbdc0SearchRuleFlag: 1;
	UINT16 u2IfApplyDbdc1SearchRuleFlag: 1;
	UINT16 u2Reserve_b19Flag: 1;
	UINT16 u2Reserve_b17_to_b18Flag: 2;
	UINT16 u2IfApplyRefillPerildFlag: 1;
#else /* RT_BIG_ENDIAN */
	UINT16 u2IfApplyRefillPerildFlag: 1;
	UINT16 u2Reserve_b17_to_b18Flag: 2;
	UINT16 u2Reserve_b19Flag: 1;
	UINT16 u2IfApplyDbdc1SearchRuleFlag: 1;
	UINT16 u2IfApplyDbdc0SearchRuleFlag: 1;
	UINT16 u2Reserve_b22_to_b24Flag: 3;
	UINT16 u2IfApplyWeightedAirTimeFairnessFlag: 1;
	UINT16 u2Reserve_b26_to_b27Flag: 2;
	UINT16 u2IfApplyEnTxopNoChangeBssFlag: 1;
	UINT16 u2IfApplyAirTimeFairnessFlag: 1;
	UINT16 u2IfApplyEnbwrefillFlag: 1;
	UINT16 u2IfApplyEnbwCtrlFlag: 1;
#endif /* RT_BIG_ENDIAN */

	/* DW#1 */
	UINT16 u2IfApplyBssCheckTimeToken_0_to_16_CtrlFlag;    /* BITWISE */
	UINT16 u2Resreve1Flag;

	/* DW#2 */
	UINT16 u2IfApplyBssCheckLengthToken_0_to_16_CtrlFlag;  /* BITWISE */
	UINT16 u2Resreve2Flag;

	/* DW#3, 4 */
	UINT32 u2ResreveBackupFlag[2];

	/***********************************************************************/

	/* DW#5 */
	UINT16 u2Bss_0_to_16_CtrlValue;          /* BITWISE */

#ifdef RT_BIG_ENDIAN
	UINT16 u2EnbwCtrlValue: 1;
	UINT16 u2EnbwrefillValue: 1;
	UINT16 u2AirTimeFairnessValue: 1;
	UINT16 u2EnTxopNoChangeBssValue: 1;
	UINT16 u2Reserve_b26_to_b27Value: 2;
	UINT16 u2WeightedAirTimeFairnessValue: 1;
	UINT16 u2Reserve_b22_to_b24Value: 3;
	UINT16 u2Dbdc0SearchRuleValue: 1;
	UINT16 u2Dbdc1SearchRuleValue: 1;
	UINT16 u2Reserve_b19Value: 1;
	UINT16 u2RefillPerildValue: 3;
#else /* RT_BIG_ENDIAN */
	UINT16 u2RefillPerildValue: 3;
	UINT16 u2Reserve_b19Value: 1;
	UINT16 u2Dbdc1SearchRuleValue: 1;
	UINT16 u2Dbdc0SearchRuleValue: 1;
	UINT16 u2Reserve_b22_to_b24Value: 3;
	UINT16 u2WeightedAirTimeFairnessValue: 1;
	UINT16 u2Reserve_b26_to_b27Value: 2;
	UINT16 u2EnTxopNoChangeBssValue: 1;
	UINT16 u2AirTimeFairnessValue: 1;
	UINT16 u2EnbwrefillValue: 1;
	UINT16 u2EnbwCtrlValue: 1;
#endif /* RT_BIG_ENDIAN */

	/* DW#6 */
	UINT16 u2BssCheckTimeToken_0_to_16_CtrlValue;    /* BITWISE */
	UINT16 u2Resreve1Value;

	/* DW#7 */
	UINT16 u2BssCheckLengthToken_0_to_16_CtrlValue;  /* BITWISE */
	UINT16 u2Resreve2Value;

	/* DW#8 */
#ifdef RT_BIG_ENDIAN
	UINT32 u4Resreve1Value: 1;
	UINT32 u4VowKeepSettingBit: 5;
	UINT32 u4VowKeepSettingValue: 1;
	UINT32 u4IfApplyKeepVoWSettingForSerFlag: 1;
	UINT32 u4RxRifsModeforCckCtsValue: 1;
	UINT32 u4IfApplyRxRifsModeforCckCtsFlag: 1;
	UINT32 u4ApplyRxEifsToZeroValue: 1;
	UINT32 u4IfApplyRxEifsToZeroFlag: 1;
	UINT32 u4RtsFailedChargeDisValue: 1;
	UINT32 u4IfApplyRtsFailedChargeDisFlag: 1; /* don't charge airtime when RTS failed */
	UINT32 u4TxBackOffBoundValue: 5; /* ms */
	UINT32 u4TxBackOffBoundEnable: 1;
	UINT32 u4IfApplyTxBackOffBoundFlag: 1;
	UINT32 u4TxMeasurementModeValue: 1;
	UINT32 u4IfApplyTxMeasurementModeFlag: 1;
	UINT32 u4TxCountValue: 4;
	UINT32 u4IfApplyTxCountModeFlag: 1;
	UINT32 u4KeepQuantumValue: 1;
	UINT32 u4IfApplyKeepQuantumFlag: 1;
	UINT32 u4RtsStaLockValue: 1;
	UINT32 u4IfApplyStaLockForRtsFlag: 1;
#else /* RT_BIG_ENDIAN */
	UINT32 u4IfApplyStaLockForRtsFlag: 1;
	UINT32 u4RtsStaLockValue: 1;
	UINT32 u4IfApplyKeepQuantumFlag: 1;
	UINT32 u4KeepQuantumValue: 1;
	UINT32 u4IfApplyTxCountModeFlag: 1;
	UINT32 u4TxCountValue: 4;
	UINT32 u4IfApplyTxMeasurementModeFlag: 1;
	UINT32 u4TxMeasurementModeValue: 1;
	UINT32 u4IfApplyTxBackOffBoundFlag: 1;
	UINT32 u4TxBackOffBoundEnable: 1;
	UINT32 u4TxBackOffBoundValue: 5; /* ms */
	UINT32 u4IfApplyRtsFailedChargeDisFlag: 1; /* don't charge airtime when RTS failed */
	UINT32 u4RtsFailedChargeDisValue: 1;
	UINT32 u4IfApplyRxEifsToZeroFlag: 1;
	UINT32 u4ApplyRxEifsToZeroValue: 1;
	UINT32 u4IfApplyRxRifsModeforCckCtsFlag: 1;
	UINT32 u4RxRifsModeforCckCtsValue: 1;
	UINT32 u4IfApplyKeepVoWSettingForSerFlag: 1;
	UINT32 u4VowKeepSettingValue: 1;
	UINT32 u4VowKeepSettingBit: 5;
	UINT32 u4Resreve1Value: 1;
#endif /* RT_BIG_ENDIAN */

	/* DW#9 */
#ifdef RT_BIG_ENDIAN
	UINT32 u4ResreveBackupValue: 21;
	UINT32 u4VowSchedulePolicy: 2;
	UINT32 u4VowScheduleType: 2;
	UINT32 u4IfApplyVowSchCtrl: 1;
	UINT32 u4DbgPrnLvl: 2;
	UINT32 u4SplStaNumValue: 3;
	UINT32 u4IfApplySplFlag: 1;
#else
	UINT32 u4IfApplySplFlag: 1;
	UINT32 u4SplStaNumValue: 3;
	UINT32 u4DbgPrnLvl: 2;
	UINT32 u4IfApplyVowSchCtrl: 1;
	UINT32 u4VowScheduleType: 2;
	UINT32 u4VowSchedulePolicy: 2;
	UINT32 u4ResreveBackupValue: 21;
#endif
} UNI_CMD_VOW_FEATURE_CTRL_T, *P_UNI_CMD_VOW_FEATURE_CTRL_T;

typedef struct _UNI_CMD_BW_BSS_TOKEN_SETTING_T {
	/* DW#0 */
	UINT16 u2MinRateToken;				   /* unit: 1 bit */
	UINT16 u2MaxRateToken;				   /* unit: 1 bit */

	/* DW#1 */
#ifdef RT_BIG_ENDIAN
	UINT32 u4MinTokenBucketLengSize: 12;   /* unit: 1024 bit */
	UINT32 u4D1B19Rev: 1;				   /* reserve */
	UINT32 u4MinAirTimeToken: 11;		   /* unit: 1/8 us */
	UINT32 u4MinTokenBucketTimeSize: 8;    /* unit: 1024 us */
#else /* RT_BIG_ENDIAN */
	UINT32 u4MinTokenBucketTimeSize: 8;    /* unit: 1024 us */
	UINT32 u4MinAirTimeToken: 11;		   /* unit: 1/8 us */
	UINT32 u4D1B19Rev: 1;				   /* reserve */
	UINT32 u4MinTokenBucketLengSize: 12;   /* unit: 1024 bit */
#endif /* RT_BIG_ENDIAN */

	/* DW#2 */
#ifdef RT_BIG_ENDIAN
	UINT32 u4MaxTokenBucketLengSize: 12;   /* unit: 1024 bit */
	UINT32 u4D2B19Rev: 1;				   /* reserve */
	UINT32 u4MaxAirTimeToken: 11;		   /* unit: 1/8 us */
	UINT32 u4MaxTokenBucketTimeSize: 8;    /* unit: 1024 us */
#else /* RT_BIG_ENDIAN */
	UINT32 u4MaxTokenBucketTimeSize: 8;    /* unit: 1024 us */
	UINT32 u4MaxAirTimeToken: 11;		   /* unit: 1/8 us */
	UINT32 u4D2B19Rev: 1;				   /* reserve */
	UINT32 u4MaxTokenBucketLengSize: 12;   /* unit: 1024 bit */
#endif /* RT_BIG_ENDIAN */

	/* DW#3 */
#ifdef RT_BIG_ENDIAN
	UINT32 u4D3B28toB31Rev: 4;			   /* reserve */
	UINT32 u4MaxBacklogSize: 12;		   /* unit: 1024 bit */
	UINT32 u4D3B8toB15Rev: 8;			   /* reserve */
	UINT32 u4MaxWaitTime: 8;			   /* unit: 1024 us */
#else /* RT_BIG_ENDIAN */
	UINT32 u4MaxWaitTime: 8;			   /* unit: 1024 us */
	UINT32 u4D3B8toB15Rev: 8;			   /* reserve */
	UINT32 u4MaxBacklogSize: 12;		   /* unit: 1024 bit */
	UINT32 u4D3B28toB31Rev: 4;			   /* reserve */
#endif /* RT_BIG_ENDIAN */
} UNI_CMD_BW_BSS_TOKEN_SETTING_T, *P_UNI_CMD_BW_BSS_TOKEN_SETTING_T;

/* bssgroup ctrl (Tag 0x02) */
typedef struct _UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucBssGroupID;
	UINT8 aucPadding[3];
	UNI_CMD_BW_BSS_TOKEN_SETTING_T  rAllBssGroupMultiField;
} UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T, *P_UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T;

/* bss token cfg (Tag 0x03) */
typedef struct _UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucBssGroupID;
	UINT8 aucPadding[3];
	UINT32 u4SingleFieldIDValue;
	UINT32 u4CfgItemId;
} UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T, *P_UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T;

/* bssgroup ctrl all group (Tag 0x04) */
typedef struct _UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UNI_CMD_BW_BSS_TOKEN_SETTING_T arAllBssGroupMultiField[UNICMD_VOW_BWC_GROUP_NUMBER];
} UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T, *P_UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T;

/* bw group quantum ctrl (Tag 0x05) */
typedef struct _UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucBssGroupQuantumID;
	UINT8 ucBssGroupQuantumTime;
	UINT8 aucPadding[2];
} UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T, *P_UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T;

/* all bw group quantum ctrl (Tag 0x06) */
typedef struct _UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 aucBssGroupQuantumTime[UNICMD_VOW_BW_GROUP_QUANTUM_LEVEL_NUM];
} UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T, *P_UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T;

/* airtime process module estimate ctrl (Tag 0x07) */
typedef struct _UNI_CMD_VOW_AT_PROC_EST_FEATURE_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	BOOLEAN fgAtEstimateOnOff;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_AT_PROC_EST_FEATURE_T, *P_UNI_CMD_VOW_AT_PROC_EST_FEATURE_T;

/* airtime process module estimate monitor period (Tag 0x08) */
typedef struct _UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2AtEstMonitorPeriod;
	UINT8 aucPadding[2];
} UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T, *P_UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T;

/* airtime process module estimate group ratio (Tag 0x09) */
typedef struct _UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT32 u4GroupRatioBitMask;
	UINT16 u2GroupMaxRatioValue[UNICMD_VOW_BWC_GROUP_NUMBER];
	UINT16 u2GroupMinRatioValue[UNICMD_VOW_BWC_GROUP_NUMBER];
} UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T, *P_UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T;

/* airtime process module estimate group to band mapping (Tag 0x0A) */
typedef struct _UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucGrouptoSelectBand;
	UINT8 ucBandSelectedfromGroup;
	UINT8 aucPadding[2];
} UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T, *P_UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T;

/* rx airtime enable (Tag 0x0B) */
typedef struct _UNI_CMD_VOW_RX_AT_AIRTIME_EN_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 fgRxAirTimeEn;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_RX_AT_AIRTIME_EN_T, *P_UNI_CMD_VOW_RX_AT_AIRTIME_EN_T;

/* rx mibtime enable (Tag 0x0C) */
typedef struct _UNI_CMD_VOW_RX_AT_MIBTIME_EN_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 fgRxMibTimeEn;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_RX_AT_MIBTIME_EN_T, *P_UNI_CMD_VOW_RX_AT_MIBTIME_EN_T;

/* rx airtime early end enable (Tag 0x0D) */
typedef struct _UNI_CMD_VOW_RX_AT_EARLYEND_EN_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 fgRxEarlyEndEn;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_RX_AT_EARLYEND_EN_T, *P_UNI_CMD_VOW_RX_AT_EARLYEND_EN_T;

/* rx airtime airtime clr enable (Tag 0x0E) */
typedef struct _UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 fgRxAirTimeClrEn;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T, *P_UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T;

/* rx airtime sta wmm ctrl (Tag 0x0F) */
typedef struct _UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucOwnMacID;
	UINT8 fgtoApplyWm00to03MibCfg;
	UINT8 aucPadding[2];
} UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T, *P_UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T;

/* rx airtime airtime clr enable (Tag 0x10) */
typedef struct _UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucMbssGroup;
	UINT8 ucWmmGroup;
	UINT8 aucPadding[2];
} UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T, *P_UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T;

/* rx airtime ed offset value (Tag 0x11) */
typedef struct _UNI_CMD_VOW_RX_AT_ED_OFFSET_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucEdOffsetValue;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_RX_AT_ED_OFFSET_T, *P_UNI_CMD_VOW_RX_AT_ED_OFFSET_T;

/* rx airtime sw timer value (Tag 0x12) */
typedef struct _UNI_CMD_VOW_RX_AT_SW_TIMER_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucCompensateMode;
	UINT8 ucRxBand;
	UINT8 ucSwCompensateTimeValue;
	UINT8 aucPadding[1];
} UNI_CMD_VOW_RX_AT_SW_TIMER_T, *P_UNI_CMD_VOW_RX_AT_SW_TIMER_T;

/* rx airtime sw timer value (Tag 0x13) */
typedef struct _UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2AC0Backoff;
	UINT16 u2AC1Backoff;
	UINT16 u2AC2Backoff;
	UINT16 u2AC3Backoff;
	UINT8 ucRxATBackoffWmmGroupIdx;
	UINT8 ucRxAtBackoffAcQMask;
	UINT8 aucPadding[2];
} UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T, *P_UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T;

/* rx airtime report non wifi time (Tag 0x14) */
typedef struct _UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucRxNonWiFiBandIdx;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T, *P_UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T;

/* rx airtime report rx obss time (Tag 0x15) */
typedef struct _UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucRxObssBandIdx;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME_T, *P_UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME_T;

/* rx airtime report rx mib time (Tag 0x16) */
typedef struct _UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucRxMibObssBandIdx;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T, *P_UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T;

/* rx airtime report rx mib time (Tag 0x17) */
typedef struct _UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2StaId;
	UINT8 aucPadding[2];
} UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T, *P_UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T;

/* red enable (Tag 0x18) */
typedef struct _UNI_CMD_VOW_RED_ENABLE_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucRedEnable;
	UINT8 aucPadding[3];
} UNI_CMD_VOW_RED_ENABLE_T, *P_UNI_CMD_VOW_RED_ENABLE_T;

typedef struct _UNI_CMD_RED_TX_RPT_T {
	UINT32 u4TCPCnt;
	UINT32 u4TCPAckCnt;
} UNI_CMD_RED_TX_RPT_T, *P_UNI_CMD_RED_TX_RPT_T;

typedef struct GNU_PACKED _UNI_PARSE_EXT_EVENT_RED_TX_RPT_T {
	UINT8 ucfgValid;
	UINT8 wordlen;
	UINT8 Reserve[2];
	UINT32 staInUseBitmap[32];
} UNI_PARSE_EXT_EVENT_RED_TX_RPT_T, *P_UNI_PARSE_EXT_EVENT_RED_TX_RPT_T;

typedef struct GNU_PACKED _UNI_PARSE_RED_TX_RPT_T {
	UINT32 u4TCPCnt;
	UINT32 u4TCPAckCnt;
	UINT16 u2MsduInQueShortTimes;
	UINT16 u2MsduInQueLongTimes;
	UINT8 u1TCPMask;
	UINT8 u1Reserved[3];
} UNI_PARSE_RED_TX_RPT_T, *P_UNI_PARSE_RED_TX_RPT_T;

/* red enable (Tag 0x19) */
typedef struct _UNI_CMD_VOW_RED_TX_RPT_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucWordlen;
	UINT8 aucPadding[3];
	UINT32 u4StaInUseBitmap[32];

	UNI_CMD_RED_TX_RPT_T arTxRpt[0];
} UNI_CMD_VOW_RED_TX_RPT_T, *P_UNI_CMD_VOW_RED_TX_RPT_T;

typedef struct GNU_PACKED _UNI_CMD_VOW_PARAM_T {
	BOOLEAN VOWTagValid[UNI_CMD_VOW_MAX_NUM];

	UNI_CMD_VOW_DRR_CTRL_T VowDrrCtrl; /* TAG 0x00 */
	UNI_CMD_VOW_FEATURE_CTRL_T VowFeatureCtrl; /* TAG 0x01 */
	UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T VowBssgroupCtrl1Group; /* TAG 0x02 */
	UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T VowBssgroupTokenCfg; /* TAG 0x03 */
	UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T VowBssgroupCtrlAllGroup; /* TAG 0x04 */
	UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T VowBssgroupBWGroupQuantum; /* TAG 0x05 */
	UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T VowBssgroupBWGroupQuantumAll; /* TAG 0x06 */
	UNI_CMD_VOW_AT_PROC_EST_FEATURE_T VowATProcEstFeature; /* TAG 0x07 */
	UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T VowATProcEstMonitorPeriod; /* TAG 0x08 */
	UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T VowATProcEstGroupRatio; /* TAG 0x09 */
	UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T VowATProcEstGroupToBandMapping; /* TAG 0x0A */

	UNI_CMD_VOW_RX_AT_AIRTIME_EN_T VowRxAtAirtimeEn; /* TAG 0x0B */
	UNI_CMD_VOW_RX_AT_MIBTIME_EN_T VowRxAtMibtimeEn; /* TAG 0x0C */
	UNI_CMD_VOW_RX_AT_EARLYEND_EN_T VowRxAtEarlyendEn; /* TAG 0x0D */
	UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T VowRxAtAirtimeClrEn; /* TAG 0x0E */
	UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T VowRxAtStaWmmCtrl; /* TAG 0x0F */
	UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T VowRxAtMbssWmmCtrl; /* TAG 0x10 */
	UNI_CMD_VOW_RX_AT_ED_OFFSET_T VowRxAtEdOffset; /* TAG 0x11 */
	UNI_CMD_VOW_RX_AT_SW_TIMER_T VowRxAtSwTimer; /* TAG 0x12 */
	UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T VowRxAtBackoffTimer; /* TAG 0x13 */
	UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T VowRxAtReportRxNonwifiTime; /* TAG 0x14 */
	UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME_T VowRxAtReportRxObssTime; /* TAG 0x15 */
	UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T VowRxAtReportMibObssTime; /* TAG 0x16 */
	UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T VowRxAtReportPerStaRxTime; /* TAG 0x17 */

	UNI_CMD_VOW_RED_ENABLE_T VowRedEnable; /* TAG 0x18 */
	UNI_CMD_VOW_RED_TX_RPT_T VowRedTxRpt; /* TAG 0x19 */
} UNI_CMD_VOW_PARAM_T, *P_UNI_CMD_VOW_PARAM_T;

typedef INT32 (*PFN_VOW_HANDLE)(struct _RTMP_ADAPTER *pAd,
									P_UNI_CMD_VOW_PARAM_T pVowParam,
									VOID *pHandle,
									UINT32 *u4RespStructSize);
/* =================== UNI_CMD_ID_VOW End ==================== */



BOOLEAN UniCmdCheckInitReady(struct _RTMP_ADAPTER *pAd);

struct cmd_msg *AndesAllocUniCmdMsg(struct _RTMP_ADAPTER *pAd, unsigned int length);

INT32 UniCmdDevInfoUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 u4EnableFeature);

INT32 UniCmdBssInfoUpdate(struct _RTMP_ADAPTER *pAd, struct _BSS_INFO_ARGUMENT_T *bss_info_argument);

INT32 MtUniCmdPmStateCtrl(struct _RTMP_ADAPTER *pAd, MT_PMSTAT_CTRL_T PmStatCtrl);

INT32 MtUniCmdSlotTimeSet(
	struct _RTMP_ADAPTER *pAd,
	UINT16 SlotTime,
	UINT16 SifsTime,
	UINT16 RifsTime,
	UINT16 EifsTime,
	struct wifi_dev *wdev);

#ifdef OCE_SUPPORT
INT32 MtUniCmdFdFrameOffloadSet(struct _RTMP_ADAPTER *pAd, P_CMD_FD_FRAME_OFFLOAD_T fdFrame_offload);
#endif /* OCE_SUPPORT */

INT32 UniCmdStaRecUpdate(struct _RTMP_ADAPTER *pAd, STA_REC_CFG_T *pStaRecCfg);

INT32 UniCmdStaRecBaUpdate(struct _RTMP_ADAPTER *pAd, STA_REC_BA_CFG_T StaRecBaCfg);

#ifdef HTC_DECRYPT_IOT
INT32 UniCmdStaRecAADOmUpdate(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, UINT8 AadOm);
#endif /* HTC_DECRYPT_IOT */

INT32 UniCmdStaRecPsmUpdate(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, UINT8 Psm);

INT32 UniCmdStaRecSNUpdate(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, UINT16 Sn);

INT32 MtUniCmdEdcaParameterSet(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, MT_EDCA_CTRL_T EdcaParam);

INT32 UniCmdBandConfig(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_BAND_CFG_PARAM_T pParamCtrl);

INT32 UniCmdRadioOnOff(struct _RTMP_ADAPTER *pAd, MT_PMSTAT_CTRL_T PmStatCtrl);

INT32 UniCmdRxvCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 TxRx, UINT8 ucEnable);

INT32 UniCmdSetEDCCAThreshold(struct _RTMP_ADAPTER *pAd, UINT8 u1edcca_threshold[], UINT8 u1BandIdx, BOOLEAN bInit);

INT32 UniCmdGetEDCCAThreshold(struct _RTMP_ADAPTER *pAd, UINT8 u1BandIdx, BOOLEAN bInit);

INT32 UniCmdSetEDCCAEnable(struct _RTMP_ADAPTER *pAd, UINT8 u1EDCCACtrl, UINT8 u1BandIdx);

INT32 UniCmdGetEDCCAEnable(struct _RTMP_ADAPTER *pAd, UINT8 u1BandIdx);

INT32 UniCmdWsysConfig(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_WSYS_CFG_PARAM_T pParamCtrl);

INT32 UniCmdAccessReg(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_ACCESS_REG_PARAM_T pParamCtrl);

INT32 UniCmdMultipleMacRegAccessRead(struct _RTMP_ADAPTER *pAd, struct _RTMP_REG_PAIR *RegPair, UINT32 Num);

INT32 UniCmdMultipleMacRegAccessWrite(struct _RTMP_ADAPTER *pAd, struct _RTMP_REG_PAIR *RegPair, UINT32 Num);

INT32 UniCmdMultipleRfRegAccessRead(struct _RTMP_ADAPTER *pAd, struct _MT_RF_REG_PAIR *RegPair, UINT32 Num);

INT32 UniCmdMultipleRfRegAccessWrite(struct _RTMP_ADAPTER *pAd, struct _MT_RF_REG_PAIR *RegPair, UINT32 Num);

INT32 UniCmdRFRegAccessRead(struct _RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 *Value);

INT32 UniCmdRFRegAccessWrite(struct _RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 Value);

INT32 MtUniCmdRestartDLReqNoRsp(struct _RTMP_ADAPTER *pAd);

INT32 UniCmdCfgInfoUpdate(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	ENUM_CFG_FEATURE eFeature,
	VOID *param);

INT32 MtUniCmdFwLog2Host(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 FWLog2HostCtrl);

INT32 MtUniCmdFwDbgCtrl(struct _RTMP_ADAPTER *pAd, UINT32 DbgClass, UINT32 ModuleIdx);

#ifdef AIR_MONITOR
INT32 MtUniCmdSmeshConfigSet(struct _RTMP_ADAPTER *pAd, UCHAR *pdata, P_UNI_CMD_SMESH_PARAM_T prSmeshResult);
#endif /* AIR_MONITOR */

INT32 UniCmdRxHdrTrans(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_RX_HDR_TRAN_PARAM_T pParamCtrl);

INT32 UniCmdSER(struct _RTMP_ADAPTER *pAd, UINT32 u4Action, UINT32 u4SetValue, UINT8 ucDbdcIdx);
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
INT32 UniCmdTWT(struct _RTMP_ADAPTER *pAd, struct mt_twt_agrt_para TwtAgrtPara);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

INT32 UniCmdMuarConfigSet(struct _RTMP_ADAPTER *pAd, UCHAR *pdata);

INT32 UniCmdCalculateECC(
	struct _RTMP_ADAPTER *pAd,
	UINT32 oper, UINT32 group,
	UINT8 *scalar,
	UINT8 *point_x,
	UINT8 *point_y);

#ifdef MT_DFS_SUPPORT
INT32 UniCmdRddCtrl(
	struct _RTMP_ADAPTER *pAd,
	UCHAR ucDfsCtrl,
	UCHAR ucRddIdex,
	UCHAR ucRddRxSel,
	UCHAR ucSetVal);
#endif /* MT_DFS_SUPPORT */

INT32 UniCmdGetTsfTime(struct _RTMP_ADAPTER *pAd, UCHAR HwBssidIdx, TSF_RESULT_T *pTsfResult);

#ifdef CFG_SUPPORT_FALCON_TXCMD_DBG
INT32 UniCmdTxCmdDbgCtrl(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_TXCMD_DBG_CTRL_PARAM_T pParamCtrl);
#endif /* CFG_SUPPORT_FALCON_TXCMD_DBG */

INT32 UniCmdMib(struct _RTMP_ADAPTER *pAd, UCHAR ChIdx, RTMP_MIB_PAIR *RegPair, UINT32 Num);

#ifdef CONFIG_HW_HAL_OFFLOAD
INT32 UniCmdSetSnifferMode(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_SNIFFER_MODE_T param);
#endif /* CONFIG_HW_HAL_OFFLOAD */

#ifdef SCS_FW_OFFLOAD
INT32 UniCmdSCS(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_SCS_PARAM_T pParamCtrl);
#endif /* SCS_FW_OFFLOAD */

#ifdef WIFI_MODULE_DVT
INT32 UniCmdMDVT(struct _RTMP_ADAPTER *pAd, UINT16 u2ModuleId, UINT16 u2CaseId);
#endif /* WIFI_MODULE_DVT */

#ifdef WIFI_GPIO_CTRL
INT32 UniCmdGPIO(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_GPIO_CFG_PARAM_T pParamCtrl);
#endif /* WIFI_GPIO_CTRL */

INT32 UniCmdMuruParameterSet(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	UINT32 u4EnableFeature);

INT32 UniCmdMuruParameterGet(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	UINT32 u4EnableFeature);

INT32 uni_hqa_muru_set_dl_tx_muru_config(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

INT32 uni_hqa_muru_set_ul_tx_muru_config(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

INT32 uni_set_muru_manual_config(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

VOID uni_muru_update_he_cfg(
	struct _RTMP_ADAPTER *pAd);

#ifdef CFG_SUPPORT_FALCON_PP
INT32 UniCmdPPCapCtrl(struct _RTMP_ADAPTER *pAd, P_PP_CMD_T pp_cmd_cap);
#endif /* CFG_SUPPORT_FALCON_PP */

INT32 UniCmdTPC(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_TPC_PARAM_T pParamCtrl);

INT32 UniCmdTPCManCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgTpcManual);

INT32 UniCmdTPCUlAlgoCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8	u1TpcCmd,
	UINT8	u1ApTxPwr,
	UINT8	u1EntryIdx,
	UINT8	u1TargetRssi,
	UINT8	u1UPH,
	BOOLEAN	fgMinPwrFlag
);

INT32 UniCmdTPCDlAlgoCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8	u1TpcCmd,
	BOOLEAN	fgCmdCtrl,
	UINT8	u1DlTxType,
	CHAR	DlTxPwr,
	UINT8	u1EntryIdx,
	INT16	DlTxpwrAlpha
);

INT32 UniCmdTPCManTblInfo(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgUplink
);

INT32 UniCmdTPCWlanIdCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgUplink,
	UINT8   u1EntryIdx,
	UINT16  u2WlanId,
	UINT8 u1DlTxType
);

INT32 UniCmdTPCUlUtVarCfg(
	struct _RTMP_ADAPTER *pAd,
	UINT8	u1EntryIdx,
	UINT8	u1VarType,
	INT16	i2Value);

INT32 UniCmdTPCUlUtGo(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgTpcUtGo
);

INT32 UniCmdTPCEnableCfg(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgTpcEnable
);

INT32 UniCmdVOWUpdate(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_VOW_PARAM_T pVOWParam,	BOOLEAN isSet, UINT8 McuDest,	VOID *pResult);

INT32 uni_cmd_vow_set_sta(PRTMP_ADAPTER pad, UINT16 sta_id, UINT32 subcmd);

INT uni_vmd_vow_set_sta_DWRR_max_time(PRTMP_ADAPTER pad);

INT uni_cmd_vow_set_group_DWRR_max_time(PRTMP_ADAPTER pad);

INT uni_vmd_vow_set_feature_all(PRTMP_ADAPTER pad);

INT uni_cmd_vow_set_at_estimator(PRTMP_ADAPTER pad, UINT32 subcmd);

INT uni_cmd_vow_set_at_estimator_group(PRTMP_ADAPTER pad, UINT32 subcmd, UINT8 group_id);

INT uni_cmd_vow_set_rx_airtime(PRTMP_ADAPTER pad, UINT8 cmd, UINT32 subcmd);

INT uni_cmd_vow_set_wmm_selection(PRTMP_ADAPTER pad, UINT8 om);

INT uni_cmd_vow_set_mbss2wmm_map(PRTMP_ADAPTER pad, UINT8 bss_idx);

INT uni_cmd_vow_set_backoff_time(PRTMP_ADAPTER pad, UINT8 target);

INT uni_cmd_vow_get_rx_time_counter(PRTMP_ADAPTER pad, UINT8 target, UINT8 band_idx);

INT32 UniCmdSetRedEnable(RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 en);

VOID UniCmdExtEventRedTxReportHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);

#endif /* WIFI_UNIFIED_COMMAND */
#endif /* _CMM_FW_UNI_CMD_H */
