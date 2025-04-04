/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc
 */



#ifndef _CMM_FW_UNI_EVENT_H
#define _CMM_FW_UNI_EVENT_H

#ifdef WIFI_UNIFIED_COMMAND
#ifdef SMART_CARRIER_SENSE_SUPPORT
#include "scs.h"
#endif /*SMART_CARRIER_SENSE_SUPPORT */

/*
 * TLV element structure should start with a 2-byte Tag field and a 2-byte
 * length field and pad to 4-byte alignment. The u2Length field indicate
 * the length of the whole TLV including tag and length field.
 */
struct GNU_PACKED TAG_HDR {
	UINT16 u2Tag;
	UINT16 u2Length;
};

typedef void(*PROCESS_RX_UNSOLICIT_EVENT_FUNCTION) (
	struct _RTMP_ADAPTER *, UINT8 *, UINT32, EVENT_RXD *);

#define TAG_ID(fp)	(((struct TAG_HDR *) fp)->u2Tag)
#define TAG_LEN(fp)	(((struct TAG_HDR *) fp)->u2Length)
#define TAG_HDR_LEN sizeof(struct TAG_HDR)

#define TAG_FOR_EACH(_pucTlvBuf, _u2TlvBufLen, _u2Offset) \
for ((_u2Offset) = 0;	\
	((((_u2Offset) + 2) <= (_u2TlvBufLen)) && \
	(((_u2Offset) + TAG_LEN(_pucTlvBuf)) <= (_u2TlvBufLen))); \
	(_u2Offset) += TAG_LEN(_pucTlvBuf), (_pucTlvBuf) += TAG_LEN(_pucTlvBuf))

typedef struct GNU_PACKED _UNI_EVENT_CMD_RESULT {
	UINT16 u2CID;
	UINT8 aucReserved[2];
	UINT32 u4Status;
} UNI_EVENT_CMD_RESULT_T, *P_UNI_EVENT_CMD_RESULT_T;

typedef enum _ENUM_UNI_EVENT_ID_T {
    UNI_EVENT_ID_CMD_RESULT      = 0x01,  /* Generic event for return cmd status */
    UNI_EVENT_ID_BMC_RPY_DT      = 0x02,
    UNI_EVENT_ID_HIF_CTRL        = 0x03,
    UNI_EVENT_ID_FW_LOG_2_HOST   = 0x04,
    UNI_EVENT_ID_ACCESS_REG      = 0x06,
    UNI_EVENT_ID_CHIP_CONFIG     = 0x07,
    UNI_EVENT_ID_SMESH_INFO      = 0x08,
    UNI_EVENT_ID_IE_COUNTDOWN    = 0x09,
    UNI_EVENT_ID_ASSERT_DUMP     = 0x0A,
    UNI_EVENT_ID_SLEEP_NOTIFY    = 0x0b,
    UNI_EVENT_ID_BEACON_TIMEOUT  = 0x0C,
    UNI_EVENT_ID_PS_SYNC         = 0x0D,
    UNI_EVENT_ID_SCAN_DONE       = 0x0E,
    UNI_EVENT_ID_ECC_CAL         = 0x10,
    UNI_EVENT_ID_ADD_KEY_DONE    = 0x12,
    UNI_EVENT_ID_OBSS_UPDATE     = 0x13,
    UNI_EVENT_ID_SER             = 0x14,
    UNI_EVENT_ID_MAC_INFO        = 0x1A,
    UNI_EVENT_ID_TDLS            = 0x1B,
    UNI_EVENT_ID_SAP             = 0x1C,
    UNI_EVENT_ID_TXCMD_CTRL      = 0x1D,
    UNI_EVENT_ID_EDCCA           = 0x21,
    UNI_EVENT_ID_MIB             = 0x22,
    UNI_EVENT_ID_STATISTICS      = 0x23,
    UNI_EVENT_ID_SR              = 0x25,
    UNI_EVENT_ID_SCS             = 0x26,
    UNI_EVENT_ID_CNM             = 0x27,
    UNI_EVENT_ID_MBMC            = 0x28,
    UNI_EVENT_ID_BSS_IS_ABSENCE  = 0x29,
    UNI_EVENT_ID_TPC             = 0x38,

    UNI_EVENT_ID_MAX_NUM
} ENUM_UNI_EVENT_ID_T, *P_ENUM_UNI_EVENT_ID_T;

typedef struct GNU_PACKED _UNI_EVENT_FW_LOG2HOST_T
{
    /* fixed field */
    UINT8 ucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
						*
						*   TAG                              | ID  | structure
						*   ---------------------------------|-----|--------------
						*   UNI_EVENT_FW_LOG_FORMAT          | 0x0 | UNI_EVENT_FW_LOG_FORMAT (Should always be the last TLV element)
						*/
} UNI_EVENT_FW_LOG2HOST_T, *P_UNI_EVENT_FW_LOG2HOST_T;

/* FW Log 2 Host event Tag */
typedef enum _UNI_EVENT_FWLOG2HOST_TAG_T {
    UNI_EVENT_FW_LOG_FORMAT = 0,
    UNI_EVENT_FW_LOG_MAX_NUM
} UNI_EVENT_FWLOG2HOST_TAG_T;

/* mobile */
/* Rebb */
enum {
    DEBUG_MSG_TYPE_MEM8 = 0x00,
    DEBUG_MSG_TYPE_MEM32 = 0x01,
    DEBUG_MSG_TYPE_ASCII = 0x02,
    DEBUG_MSG_TYPE_BINARY = 0x03,
    DEBUG_MSG_TYPE_END
};

/* FW Log with Format (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_FW_LOG_FORMAT_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT8 ucMsgFmt;
    UINT8 ucReserved[3];
    UINT8 acMsg[0];
} UNI_EVENT_FW_LOG_FORMAT_T, *P_UNI_EVENT_FW_LOG_FORMAT_T;

typedef struct GNU_PACKED _UNI_EVENT_ACCESS_REG_T {
    /* fixed field */
    UINT8 ucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
							*
							*   TAG                              | ID  | structure
							*   ---------------------------------|-----|---------------------------------
							*   UNI_EVENT_ACCESS_REG_BASIC       | 0x0 | UNI_EVENT_ACCESS_REG_BASIC_T
							*   UNI_EVENT_ACCESS_RF_REG_BASIC    | 0x1 | UNI_EVENT_ACCESS_RF_REG_BASIC_T
							*/
} UNI_EVENT_ACCESS_REG_T, *P_UNI_EVENT_ACCESS_REG_T;

/* Register access event Tag */
typedef enum _UNI_EVENT_ACCESS_REG_TAG_T {
    UNI_EVENT_ACCESS_REG_BASIC = 0,
    UNI_EVENT_ACCESS_RF_REG_BASIC,
    UNI_EVENT_ACCESS_REG_MAX_NUM
} UNI_EVENT_ACCESS_REG_TAG_T;

/* Access Register (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_ACCESS_REG_BASIC_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT32 u4Addr;
    UINT32 u4Value;
} UNI_EVENT_ACCESS_REG_BASIC_T, *P_UNI_EVENT_ACCESS_REG_BASIC_T;

/* Access RF address (Tag1) */
typedef struct GNU_PACKED _UNI_EVENT_ACCESS_RF_REG_BASIC_T {
    UINT16 u2Tag;
    UINT16 u2Length;
    UINT16 u2WifiStream;
    UINT16 u2Reserved;
    UINT32 u4Addr;
    UINT32 u4Value;
} UNI_EVENT_ACCESS_RF_REG_BASIC_T, *P_UNI_EVENT_ACCESS_RF_REG_BASIC_T;

typedef struct GNU_PACKED _UNI_EVENT_SMESH_INFO_T {
    /* fixed field */
    UINT8 ucBand;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_SMESH_INFO_T, *P_UNI_EVENT_SMESH_INFO_T;

typedef struct GNU_PACKED _UNI_EVENT_IE_COUNTDOWNT_T {
    /* fixed field */
    UINT8 ucBand;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_IE_COUNTDOWN_T, *P_UNI_EVENT_IE_COUNTDOWN_T;

/* IE countdown event Tag */
typedef enum _UNI_EVENT_IE_COUNTDOWN_TAG_T {
    UNI_EVENT_IE_COUNTDOWN_CSA = 0,
    UNI_EVENT_IE_COUNTDOWN_BCC = 1,
    UNI_EVENT_IE_COUNTDOWN_MAX_NUM
} UNI_EVENT_IE_COUNTDOWN_TAG_T;

/* CSA notify Parameters (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_CSA_NOTIFY_T {
    UINT16 u2Tag;    /* Tag = 0x00 */
    UINT16 u2Length;
    UINT8 ucOwnMacIdx;
    UINT8 ucChannelSwitchCount;
    UINT8 aucPadding[2];
} UNI_EVENT_CSA_NOTIFY_T, *P_UNI_EVENT_CSA_NOTIFY_T;

/* BCC notify Parameters (Tag1) */
typedef struct GNU_PACKED _UNI_EVENT_BCC_NOTIFY_T {
    UINT16 u2Tag;    /* Tag = 0x01 */
    UINT16 u2Length;
    UINT8 ucOwnMacIdx;
    UINT8 ucColorSwitchCount;
    UINT8 aucPadding[2];
} UNI_EVENT_BCC_NOTIFY_T, *P_UNI_EVENT_BCC_NOTIFY_T;

typedef struct GNU_PACKED _UNI_EVENT_ASSERT_DUMP_T {
    /*fixed field*/
    UINT8 ucBssIndex;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0]; /**< the TLVs included in this field:
							*
							*   TAG                             | ID  | structure
							*   -------------                   | ----| -------------
							*   UNI_EVENT_ASSERT_CONTENT_DUMP   | 0x0 | UNI_EVENT_ASSERT_CONTENT_T
							*/
} UNI_EVENT_ASSERT_DUMP_T, *P_UNI_EVENT_ASSERT_DUMP_T;

/* Assert Dump event Tag */
typedef enum _UNI_EVENT_ASSERT_DUMP_TAG_T {
    UNI_EVENT_ASSERT_CONTENT_DUMP = 0,
} UNI_EVENT_ASSERT_DUMP_TAG_T;


/* Sleep Notify (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_ASSERT_CONTENT_T {
    UINT16 u2Tag;                   /* Tag = 0x00 */
    UINT16 u2Length;

    UINT8 aucBuffer[0];
} UNI_EVENT_ASSERT_CONTENT_T, *P_UNI_EVENT_ASSERT_CONTENT_T;


typedef struct GNU_PACKED _UNI_EVENT_BEACON_TIMEOUT_T {
    /* fixed field */
    UINT8 ucBssIndex;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_BEACON_TIMEOUT_T, *P_UNI_EVENT_BEACON_TIMEOUT_T;

typedef enum _UNI_EVENT_BEACON_TIMEOUT_TAG_T {
    UNI_EVENT_BEACON_TIMEOUT_INFO = 0,
    UNI_EVENT_BEACON_TIMEOUT_MAX_NUM
} UNI_EVENT_BEACON_TIMEOUT_TAG_T;

/* Beacon timeout reason (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_BEACON_TIMEOUT_INFO_T {
    UINT16 u2Tag;    /* Tag = 0x00 */
    UINT16 u2Length;
    UINT8 ucReasonCode;
    UINT8 aucPadding[3];
} UNI_EVENT_BEACON_TIMEOUT_INFO_T, *P_UNI_EVENT_BEACON_TIMEOUT_INFO_T;

typedef enum _UNI_ENUM_BCN_TIMEOUT_REASON_T {
    UNI_ENUM_BCN_LOSS_STA = 0x00,
    UNI_ENUM_BCN_LOSS_ADHOC = 0x01,
    UNI_ENUM_BCN_NULL_FRAME_THRESHOLD = 0x02,
    UNI_ENUM_BCN_PERIOD_NOT_ILLIGAL = 0x03,
    UNI_ENUM_BCN_CONNECTION_FAIL = 0x04,
    UNI_ENUM_BCN_ALLOCAT_NULL_PKT_FAIL_THRESHOLD = 0x05,
    UNI_ENUM_BCN_UNSPECIF_REASON = 0x06,
    UNI_ENUM_BCN_NULL_FRAME_LIFE_TIMEOUT = 0x07,
    UNI_ENUM_BCN_LOSS_AP_DISABLE = 0x08,
    UNI_ENUM_BCN_LOSS_AP_ERROR = 0x09,
    UNI_ENUM_BCN_TIMEOUT_REASON_MAX_NUM
} UNI_ENUM_BCN_TIMEOUT_REASON_T;


typedef struct GNU_PACKED _UNI_EVENT_PS_SYNC_T {
    /*fixed field*/
    UINT8 ucBssIndex;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0]; /**< the TLVs included in this field:
							*
							*   TAG                             | ID  | structure
							*   -------------                   | ----| -------------
							*   UNI_EVENT_CLIENT_PS_INFO          | 0x0 | UNI_EVENT_CLIENT_PS_INFO_T
							*/
} UNI_EVENT_PS_SYNC_T, *P_UNI_EVENT_PS_SYNC_T;

/* PS Sync event Tag */
typedef enum _UNI_EVENT_PS_SYNC_TAG_T {
    UNI_EVENT_CLIENT_PS_INFO = 0,
} UNI_EVENT_PS_SYNC_TAG_T;

/* PS SYNC (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_CLIENT_PS_INFO_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    UINT8 ucPsBit;
    UINT8 aucPadding[1];
    UINT16 ucWtblIndex;
    UINT8 ucBufferSize;
    UINT8 aucReserved[3];
} UNI_EVENT_CLIENT_PS_INFO_T, *P_UNI_EVENT_CLIENT_PS_INFO_T;

/* ECC event (0x10) */
typedef struct GNU_PACKED _UNI_EVENT_ECC_CAL_T {
    /* fixed field */
    UINT8 aucPadding[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_ECC_CAL_T, *P_UNI_EVENT_ECC_CAL_T;

/* ECC operation event tags */
typedef enum _UNI_EVENT_ECC_CAL_TAG_T {
    UNI_EVENT_ECC_CAL_GROUP_POINT_RESULT   = 0x00,
    UNI_EVENT_ECC_CAL_MAX_NUM
} UNI_EVENT_ECC_AL_TAG_T;

/* ECC Operation Parameters (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_ECC_CAL_RES_T {
    UINT16 u2Tag;    /* Tag = 0x00 */
    UINT16 u2Length;

    UINT8 ucDqxDataLength;
    UINT8 ucDqyDataLength;
    UINT8 ucEccCmdId;
    UINT8 ucIsResFail;
    UINT8 aucDqxBuffer[48];
    UINT8 aucDqyBuffer[48];
} UNI_EVENT_ECC_CAL_RES_T, *P_UNI_EVENT_ECC_CAL_RES_T;

typedef struct GNU_PACKED _UNI_EVENT_SER_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
						*
						*   TAG                          | ID  | structure
						*   -------------                | ----| -------------
						*   UNI_EVENT_SER_QUERY_CMM      | 0x0 | UNI_EVENT_SER_QUERY_CMM_T
						*   UNI_EVENT_SER_QUERY_BAND     | 0x1 | UNI_EVENT_SER_QUERY_BAND_T
						*   UNI_EVENT_SER_QUERY_WFDMA    | 0x2 | UNI_EVENT_SER_QUERY_WFDMA_T
						*/
} UNI_EVENT_SER_T, *P_UNI_EVENT_SER_T;

/* SER event Tag */
typedef enum _UNI_EVENT_SER_TAG_T {
    UNI_EVENT_SER_QUERY_CMM = 0,
    UNI_EVENT_SER_QUERY_BAND = 1,
    UNI_EVENT_SER_QUERY_WFDMA = 2,
    UNI_EVENT_SER_MAX_NUM
} UNI_EVENT_SER_TAG_T;

/* Beacon timeout reason (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_SER_QUERY_CMM_T {
    UINT16   u2Tag;    /* Tag = 0x00 */
    UINT16   u2Length;
    UINT8    ucEnableSER;
    UINT8    ucSerL1RecoverCnt;
    UINT8    ucSerL2RecoverCnt;
    UINT8    aucPaddings[1];
    UINT16   u2PSEErrorCnt[32];
    UINT16   u2PSEError1Cnt[32];
    UINT16   u2PLEErrorCnt[32];
    UINT16   u2PLEError1Cnt[32];
    UINT16   u2PLEErrorAmsduCnt[32];
} UNI_EVENT_SER_QUERY_CMM_T, *P_UNI_EVENT_SER_QUERY_CMM_T;

/* Per band SER counter (Tag1) */
typedef struct GNU_PACKED _UNI_EVENT_SER_QUERY_BAND_T {
    UINT16   u2Tag;    /* Tag = 0x01 */
    UINT16   u2Length;
    UINT8    ucBandIdx;
    UINT8    ucL3RxAbortCnt;
    UINT8    ucL3TxAbortCnt;
    UINT8    ucL3TxDisableCnt;
    UINT8    ucL4RecoverCnt;
    UINT8    aucPaddings[3];
    UINT16   au2LMACError6Cnt[32];
    UINT16   au2LMACError7Cnt[32];
} UNI_EVENT_SER_QUERY_BAND_T, *P_UNI_EVENT_SER_QUERY_BAND_T;

/* wfdma counter (Tag2) */
typedef struct GNU_PACKED _UNI_EVENT_SER_QUERY_WFDMA_T {
    UINT16   u2Tag;    /* Tag = 0x02 */
    UINT16   u2Length;
    UINT16   au2WfdmaTxBusyCnt[4];
    UINT16   au2WfdmaRxBusyCnt[4];
} UNI_EVENT_SER_QUERY_WFDMA_T, *P_UNI_EVENT_SER_QUERY_WFDMA_T;

typedef struct GNU_PACKED _UNI_EVENT_MAC_IFNO_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_MAC_IFNO_T, *P_UNI_EVENT_MAC_IFNO_T;

/* Mac info event Tag */
typedef enum _UNI_EVENT_MAC_INFO_TAG_T {
    UNI_EVENT_MAC_INFO_TSF  = 0,
    UNI_EVENT_MAC_INFO_MAX_NUM
} UNI_EVENT_MAC_INFO_TAG_T;

/* Beacon timeout reason (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_MAC_INFO_TSF_T {
    UINT16   u2Tag;    /* Tag = 0x00 */
    UINT16   u2Length;
    UINT8    ucDbdcIdx;
    UINT8    ucHwBssidIndex;
    UINT8    aucPadding[2];
    UINT32   u4TsfBit0_31;
    UINT32   u4TsfBit63_32;
} UNI_EVENT_MAC_INFO_TSF_T, *P_UNI_EVENT_MAC_INFO_TSF_T;

/* TXCMD Ctrl command (0x1D) */
typedef struct GNU_PACKED _UNI_EVENT_TXCMD_CTRL_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_TXCMD_CTRL_T, *P_UNI_EVENT_TXCMD_CTRL_T;

/* TXCMD ctrl cmd tags */
typedef enum _UNI_EVENT_TXCMD_CTRL_TAG_T {
    UNI_EVENT_GET_TXCMD_DBG_STATUS = 0x0f,
    UNI_EVENT_GET_TXCMD_DBG_SXN_GLOBAL = 0x10,
    UNI_EVENT_GET_TXCMD_DBG_SXN_PROTECT = 0x11,
    UNI_EVENT_GET_TXCMD_DBG_SXN_TXDATA = 0x12,
    UNI_EVENT_GET_TXCMD_DBG_SXN_TRIGDATA = 0x13,
    UNI_EVENT_GET_TXCMD_DBG_TF_TXD = 0x14,
    UNI_EVENT_GET_TXCMD_DBG_TF_BASIC = 0x15,
    UNI_EVENT_GET_TXCMD_DBG_SXN_SW_FID = 0x16,
    UNI_EVENT_GET_TXCMD_DBG_SW_FID_TXD = 0x17,
    UNI_EVENT_TXCMD_CTRL_MAX_NUM
} UNI_EVENT_TXCMD_CTRL_TAG_T;

/* Txcmd ctrl Parameters (Tag) */
typedef struct GNU_PACKED _UNI_EVENT_GET_TXCMD_DBG_CMD_CTRL_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    UINT8  ucUserIndex;
    UINT8  ucDlUlidx;

    UINT8  aucBuffer[0];
} UNI_EVENT_GET_TXCMD_DBG_CMD_CTRL_T, *P_UNI_EVENT_GET_TXCMD_DBG_CMD_CTRL_T;

typedef struct GNU_PACKED _UNI_EVENT_EDCCA_T {
	/* fixed field */
	UINT8 ucBand;
	UINT8 aucPadding[3];

	/* tlv */
	UINT8 aucTlvBuffer[0];/**< the TLVs included in this field:
						*   TAG                             | ID  | structure
						*   -------------                   | ----| -------------
						*   UNI_EVENT_EDCCA_ENABLE          | 0x0 | UNI_EVENT_EDCCA_ENABLE_T
						*   UNI_EVENT_EDCCA_THRESHOLD       | 0x1 | UNI_EVENT_EDCCA_THRESHOLD_T
						*/
} UNI_EVENT_EDCCA_T, *P_UNI_EVENT_EDCCA_T;

/* EDCCA CTRL event Tag */
typedef enum _UNI_EVENT_EDCCA_TAG_T {
    UNI_EVENT_EDCCA_ENABLE = 0,
    UNI_EVENT_EDCCA_THRESHOLD,
    UNI_EVENT_EDCCA_NUM
} UNI_EVENT_EDCCA_TAG_T;

/* EDCCA OnOff Control (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_EDCCA_ENABLE_T {
    UINT16 u2Tag;    /* Tag = 0x00 */
    UINT16 u2Length;
    UINT8 fgEDCCAEnable;
    UINT8 aucPadding[3];
} UNI_EVENT_EDCCA_ENABLE_T, *P_UNI_EVENT_EDCCA_ENABLE_T;

/* EDCCA Threshold Control (Tag1) */
typedef struct GNU_PACKED _UNI_EVENT_EDCCA_THRESHOLD_T {
    UINT16 u2Tag;    /* Tag = 0x01 */
    UINT16 u2Length;
    UINT8 u1EDCCAThreshold[3];
    UINT8 fginit;
} UNI_EVENT_EDCCA_THRESHOLD_T, *P_UNI_EVENT_EDCCA_THRESHOLD_T;

/* Get MIB event Tag */
typedef enum _UNI_EVENT_MIB_TAG_T {
    UNI_EVENT_MIB_DATA = 0,
    UNI_EVENT_MIB_MAX_NUM
} UNI_EVENT_MIB_TAG_T;
typedef struct GNU_PACKED _UNI_EVENT_MIB_T {
    /* fixed field */
    UINT8 ucBand;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_MIB_T, *P_UNI_EVENT_MIB_T;

typedef struct GNU_PACKED _UNI_EVENT_MIB_DATA_T {
    UINT16 u2Tag;       /* should be 0x00 */
    UINT16 u2Length;    /* the length of this TLV */
    UINT32 u4Counter;   /* MIB ID of demanded MIB counter */
    UINT64 u8Data;      /* cumulated MIB counter for corresponded MIB ID */
} UNI_EVENT_MIB_DATA_T, *P_UNI_EVENT_MIB_DATA_T;

#ifdef SCS_FW_OFFLOAD
typedef struct GNU_PACKED _UNI_EVENT_SCS {
    /* fixed field */
    UINT8 u1Bandidx;
    UINT8 aucPadding[3];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_SCS_T, *P_UNI_EVENT_SCS_T;

/** SCS EVENT TLV List */
typedef enum _UNI_EVENT_SCS_TAG_T {
    UNI_EVENT_SCS_GET_GLO_ADDR = 0,
    UNI_EVENT_SCS_MAX_NUM
} UNI_EVENT_SCS_TAG_T;

#ifdef SMART_CARRIER_SENSE_SUPPORT
typedef struct GNU_PACKED _UNI_EVENT_GET_SCS_GLO_ADDR {
    UINT16 u2Tag;
    UINT16 u2Length;

    UINT32 u4Index;
    EVENT_SCS_GLO rGloInfo;
} UNI_EVENT_GET_SCS_GLO_ADDR, *P_UNI_EVENT_GET_SCS_GLO_ADDR;
#endif /* SMART_CARRIER_SENSE_SUPPORT */
#endif /* SCS_FW_OFFLOAD */

/* TPC event Tag */
typedef enum _UNI_EVENT_ID_TPC_TAG_T {
    UNI_EVENT_TPC_DOWNLINK_TABLE = 0,
    UNI_EVENT_TPC_UPLINK_TABLE = 1,
    UNI_EVENT_TPC_MAX_NUM
} UNI_EVENT_ID_TPC_TAG_T;

typedef struct GNU_PACKED _UNI_EVENT_TPC_T {
    /* fixed field */
    UINT8 aucReserved[4];

    /* tlv */
    UINT8 aucTlvBuffer[0];
} UNI_EVENT_TPC_T, *P_UNI_EVENT_TPC_T;

typedef struct GNU_PACKED _UNI_TPC_DL_MAN_MODE_PARAM_ELEMENT {
    UINT16 u2WlanId;
    UINT8 u1Reserved[2];
    INT16 i2DlTxPwrAlpha[UNI_TPC_DL_TX_TYPE_NUM];
} UNI_TPC_DL_MAN_MODE_PARAM_ELEMENT, *P_UNI_TPC_DL_MAN_MODE_PARAM_ELEMENT;

#define UNI_TPC_SUPPORT_STA_NUM    16

/* TPC DL info (Tag0) */
typedef struct GNU_PACKED _UNI_EVENT_TPC_INFO_DOWNLINK_TABLE_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8 u1TpcCategory;
    /*AP Info*/
    BOOLEAN fgCmdPwrCtrl[UNI_TPC_DL_TX_TYPE_NUM];
    INT8 i1DlTxPwr[UNI_TPC_DL_TX_TYPE_NUM];
    UINT8 au1Reserved[3];
    UNI_TPC_DL_MAN_MODE_PARAM_ELEMENT rTpcDlManModeParamElem[UNI_TPC_SUPPORT_STA_NUM];
} UNI_EVENT_TPC_INFO_DOWNLINK_TABLE_T, *P_UNI_EVENT_TPC_INFO_DOWNLINK_TABLE_T;

typedef struct GNU_PACKED _UNI_TPC_UL_STA_COMM_INFO {
    UINT8 u1TargetRssi;
    UINT8 u1PwrHeadRoom;
    BOOLEAN fgMinPwr;
} UNI_TPC_UL_STA_COMM_INFO, *P_UNI_TPC_UL_STA_COMM_INFO;

typedef struct GNU_PACKED _UNI_TPC_UL_MAN_MODE_PARAM_ELEMENT {
    UINT16 u2WlanId;
    UNI_TPC_UL_STA_COMM_INFO rTpcUlStaCmmInfo;
} UNI_TPC_UL_MAN_MODE_PARAM_ELEMENT, *P_UNI_TPC_UL_MAN_MODE_PARAM_ELEMENT;

/* TPC UL info (Tag1) */
typedef struct GNU_PACKED _UNI_EVENT_TPC_INFO_UPLINK_TABLE_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    /* tag specific part */
    UINT8  u1TpcCategory;

    /*AP Info*/
    UINT8  u1ApTxPwr;
    UINT8  au1Reserved[2];

    UNI_TPC_UL_MAN_MODE_PARAM_ELEMENT rTpcUlManModeParamElem[UNI_TPC_SUPPORT_STA_NUM];
} UNI_EVENT_TPC_INFO_UPLINK_TABLE_T, *P_UNI_EVENT_TPC_INFO_UPLINK_TABLE_T;

/* =================== UNI_CMD_ID_VOW Begin ================== */
/* VOW event (0x37) */
typedef struct _UNI_EVENT_VOW_T {
	/* fixed field */
	UINT8 aucReserved[4];

	/* tlv */
	UINT8 aucTlvBuffer[0]; /**<the TLVs includer in this field:
		*
		*  TAG                                         | ID    | structure
		*  ------------------------------------        | ----- | -------------
		*  UNI_EVENT_VOW_DRR_CTRL                      |  0x0  | UNI_EVENT_VOW_DRR_CTRL_T
		*  UNI_EVENT_VOW_FEATURE_CTRL                  |  0x1  | UNI_EVENT_VOW_FEATURE_CTRL_T
		*  UNI_EVENT_VOW_BSSGROUP_CTRL_1_GROUP         |  0x2  | UNI_EVENT_VOW_BSSGROUP_CTRL_1_GROUP_T
		*  UNI_EVENT_VOW_BSSGROUP_TOKEN_CFG            |  0x3  | UNI_EVENT_VOW_BSSGROUP_TOKEN_CFG_T
		*  UNI_EVENT_VOW_BSSGROUP_CTRL_ALL_GROUP       |  0x4  | UNI_EVENT_VOW_BSSGROUP_CTRL_ALL_GROUP_T
		*  UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM     |  0x5  | UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_T
		*  UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL |  0x6  | UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_T
		*  UNI_EVENT_VOW_AT_PROC_EST_FEATURE           |  0x7  | UNI_EVENT_VOW_AT_PROC_EST_FEATURE_T
		*  UNI_EVENT_VOW_AT_PROC_EST_MONITOR_PERIOD    |  0x8  | UNI_EVENT_VOW_AT_PROC_EST_MONITOR_PERIOD_T
		*  UNI_EVENT_VOW_AT_PROC_EST_GROUP_RATIO       |  0x9  | UNI_EVENT_VOW_AT_PROC_EST_GROUP_RATIO_T
		*  UNI_EVENT_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING  | 0xA  | UNI_EVENT_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T
		*  UNI_EVENT_VOW_RX_AT_AIRTIME_EN              |  0xB  | UNI_EVENT_VOW_RX_AT_AIRTIME_EN_T
		*  UNI_EVENT_VOW_RX_AT_STA_WMM_CTRL            |  0xF  | UNI_EVENT_VOW_RX_AT_STA_WMM_CTRL_T
		*  UNI_EVENT_VOW_RX_AT_MBSS_WMM_CTRL           | 0x10  | UNI_EVENT_VOW_RX_AT_MBSS_WMM_CTRL_T
		*  UNI_EVENT_VOW_RX_AT_ED_OFFSET               | 0x11  | UNI_EVENT_VOW_RX_AT_ED_OFFSET_T
		*  UNI_EVENT_VOW_RX_AT_SW_TIMER                | 0x12  | UNI_EVENT_VOW_RX_AT_SW_TIMER_T
		*  UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER           | 0x13  | UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T
		*  UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME  | 0x14  | UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T
		*  UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME     | 0x15  | UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T
		*  UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME    | 0x16  | UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T
		*  UNI_EVENT_VOW_RX_AT_REPORT_PER_STA_RX_TIME  | 0x17  | UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T
		*/
} UNI_EVENT_VOW_T, *P_UNI_EVENT_VOW_T;

typedef enum _UNI_EVENT_VOW_TAG_T {
	UNI_EVENT_VOW_DRR_CTRL = 0x00,
	UNI_EVENT_VOW_FEATURE_CTRL = 0x01,
	UNI_EVENT_VOW_BSSGROUP_CTRL_1_GROUP = 0x02,
	UNI_EVENT_VOW_BSSGROUP_TOKEN_CFG = 0x03,
	UNI_EVENT_VOW_BSSGROUP_CTRL_ALL_GROUP = 0x04,
	UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM = 0x05,
	UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL = 0x06,
	UNI_EVENT_VOW_AT_PROC_EST_FEATURE = 0x07,
	UNI_EVENT_VOW_AT_PROC_EST_MONITOR_PERIOD = 0x08,
	UNI_EVENT_VOW_AT_PROC_EST_GROUP_RATIO = 0x09,
	UNI_EVENT_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING = 0x0A,
	UNI_EVENT_VOW_RX_AT_AIRTIME_EN = 0x0B,
	UNI_EVENT_VOW_RX_AT_STA_WMM_CTRL = 0x0F,
	UNI_EVENT_VOW_RX_AT_MBSS_WMM_CTRL = 0x10,
	UNI_EVENT_VOW_RX_AT_ED_OFFSET = 0x11,
	UNI_EVENT_VOW_RX_AT_SW_TIMER = 0x12,
	UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER = 0x13,
	UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME = 0x14,
	UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME = 0x15,
	UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME = 0x16,
	UNI_EVENT_VOW_RX_AT_REPORT_PER_STA_RX_TIME = 0x17,
} UNI_EVENT_VOW_TAG_T;

/* TAG: UNI_EVENT_VOW_DRR_CTRL (0x00) */
typedef struct _UNI_EVENT_VOW_DRR_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2StaID;
	UINT8 ucReserve[2];

	UINT32 u4CtrlFieldID;
	UINT32 u4ComValue;
	UINT8 aucAirTimeQuantum[UNICMD_VOW_AIRTIME_QUANTUM_IDX_TOTAL_NUM];
} UNI_EVENT_VOW_DRR_CTRL_T, *P_UNI_EVENT_VOW_DRR_CTRL_T;

/* TAG: UNI_EVENT_VOW_FEATURE_CTRL (0x01) */
typedef struct _UNI_CMD_VOW_FEATURE_CTRL_T    UNI_EVENT_VOW_FEATURE_CTRL_T, *P_UNI_EVENT_VOW_FEATURE_CTRL_T;

/* TAG: UNI_EVENT_VOW_BSSGROUP_CTRL_1_GROUP (0x01) */
typedef struct _UNI_EVENT_VOW_BSSGROUP_CTRL_1_GROUP_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucBssGroupID;
	UINT8 aucPadding[3];
	UNI_CMD_BW_BSS_TOKEN_SETTING_T rAllBssGroupMultiField;
} UNI_EVENT_VOW_BSSGROUP_CTRL_1_GROUP_T, *P_UNI_EVENT_VOW_BSSGROUP_CTRL_1_GROUP_T;

/* TAG: UNI_EVENT_VOW_BSSGROUP_TOKEN_CFG (0x03) */
typedef struct _UNI_EVENT_VOW_BSSGROUP_TOKEN_CFG_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucBssGroupID;
	UINT8 aucPadding[3];
	UINT32 u4SingleFieldIDValue;
	UINT32 u4CfgItemId;
} UNI_EVENT_VOW_BSSGROUP_TOKEN_CFG_T, *P_UNI_EVENT_VOW_BSSGROUP_TOKEN_CFG_T;

/* TAG: UNI_EVENT_VOW_BSSGROUP_CTRL_ALL_GROUP (0x04) */
typedef struct _UNI_EVENT_VOW_BSSGROUP_CTRL_ALL_GROUP_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UNI_CMD_BW_BSS_TOKEN_SETTING_T arAllBssGroupMultiField[UNICMD_VOW_BWC_GROUP_NUMBER];
} UNI_EVENT_VOW_BSSGROUP_CTRL_ALL_GROUP_T, *P_UNI_EVENT_VOW_BSSGROUP_CTRL_ALL_GROUP_T;

/* TAG: UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM (0x05) */
typedef struct _UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucBssGroupQuantumID;
	UINT8 ucBssGroupQuantumTime;
	UINT8 aucPadding[2];
} UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_T, *P_UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_T;

/* TAG: UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL (0x06) */
typedef struct _UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 aucBssGroupQuantumTime[UNICMD_VOW_BW_GROUP_QUANTUM_LEVEL_NUM];
} UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T, *P_UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T;

/* TAG: UNI_EVENT_VOW_AT_PROC_EST_FEATURE (0x07) */
typedef struct _UNI_EVENT_VOW_AT_PROC_EST_FEATURE_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	BOOLEAN fgAtEstimateOnOff;
	UINT8 aucPadding[3];
} UNI_EVENT_VOW_AT_PROC_EST_FEATURE_T, *P_UNI_EVENT_VOW_AT_PROC_EST_FEATURE_T;

/* TAG: UNI_EVENT_VOW_AT_PROC_EST_MONITOR_PERIOD (0x08) */
typedef struct _UNI_EVENT_VOW_AT_PROC_EST_MONITOR_PERIOD_T {
	UINT16         u2Tag;
	UINT16         u2Length;

	UINT16         u2AtEstMonitorPeriod;
	UINT8          aucPadding[2];
} UNI_EVENT_VOW_AT_PROC_EST_MONITOR_PERIOD_T, *P_UNI_EVENT_VOW_AT_PROC_EST_MONITOR_PERIOD_T;

/* TAG: UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO (0x09) */
typedef struct _UNI_EVENT_VOW_AT_PROC_EST_GROUP_RATIO_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2GroupMaxRatioValue[UNICMD_VOW_BWC_GROUP_NUMBER];
	UINT16 u2GroupMinRatioValue[UNICMD_VOW_BWC_GROUP_NUMBER];
} UNI_EVENT_VOW_AT_PROC_EST_GROUP_RATIO_T, *P_UNI_EVENT_VOW_AT_PROC_EST_GROUP_RATIO_T;

/* TAG: UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING (0x0A) */
typedef struct _UNI_EVENT_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucGrouptoSelectBand;
	UINT8 ucBandSelectedfromGroup;
	UINT8 aucPadding[2];
} UNI_EVENT_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T, *P_UNI_EVENT_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T;

/* rx airtime airtime enable (Tag 0x0B) */
typedef struct _UNI_EVENT_VOW_RX_AT_AIRTIME_EN_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    UINT8 fgRxAirTimeEn;
    UINT8 aucPadding[3];
} UNI_EVENT_VOW_RX_AT_AIRTIME_EN_T, *P_UNI_EVENT_VOW_RX_AT_AIRTIME_EN_T;

/* rx airtime sta wmm ctrl (Tag 0x0F) */
typedef struct _UNI_EVENT_VOW_RX_AT_STA_WMM_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucOwnMacID;
	UINT8 fgtoApplyWm00to03MibCfg;
	UINT8 aucPadding[2];
} UNI_EVENT_VOW_RX_AT_STA_WMM_CTRL_T, *P_UNI_EVENT_VOW_RX_AT_STA_WMM_CTRL_T;

/* rx airtime airtime clr enable (Tag 0x10) */
typedef struct _UNI_EVENT_VOW_RX_AT_MBSS_WMM_CTRL_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucMbssGroup;
	UINT8 ucWmmGroup;
	UINT8 aucPadding[2];
} UNI_EVENT_VOW_RX_AT_MBSS_WMM_CTRL_T, *P_UNI_EVENT_VOW_RX_AT_MBSS_WMM_CTRL_T;

/* rx airtime ed offset value (Tag 0x11) */
typedef struct _UNI_EVENT_VOW_RX_AT_ED_OFFSET_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucEdOffsetValue;
	UINT8 aucPadding[3];
} UNI_EVENT_VOW_RX_AT_ED_OFFSET_T, *P_UNI_EVENT_VOW_RX_AT_ED_OFFSET_T;

/* rx airtime sw timer value (Tag 0x12) */
typedef struct _UNI_EVENT_VOW_RX_AT_SW_TIMER_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucCompensateMode;
	UINT8 ucRxBand;
	UINT8 ucSwCompensateTimeValue;
	UINT8 aucPadding[1];
} UNI_EVENT_VOW_RX_AT_SW_TIMER_T, *P_UNI_EVENT_VOW_RX_AT_SW_TIMER_T;

/* rx airtime sw timer value (Tag 0x13) */
typedef struct _UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT16 u2AC0Backoff;
	UINT16 u2AC1Backoff;
	UINT16 u2AC2Backoff;
	UINT16 u2AC3Backoff;
	UINT8 ucRxATBackoffWmmGroupIdx;
	UINT8 ucRxAtBackoffAcQMask;
	UINT8 aucPadding[2];
} UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T, *P_UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T;

/* rx airtime report non wifi time (Tag 0x14) */
typedef struct _UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucRxNonWiFiBandIdx;
	UINT8 aucPadding[3];
	UINT32 u4RxNonWiFiBandTimer;
} UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T, *P_UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T;

/* rx airtime report rx obss time (Tag 0x15) */
typedef struct _UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucRxObssBandIdx;
	UINT8 aucPadding[3];
	UINT32 u4RxObssBandTimer;
} UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME_T, *P_UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME_T;

/* rx airtime report rx mib time (Tag 0x16) */
typedef struct _UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T {
	UINT16 u2Tag;
	UINT16 u2Length;

	UINT8 ucRxMibObssBandIdx;
	UINT8 aucPadding[3];
	UINT32 u4RxMibObssBandTimer;
} UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T, *P_UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T;

/* rx airtime report rx mib time (Tag 0x17) */
typedef struct _UNI_EVENT_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T {
    UINT16 u2Tag;
    UINT16 u2Length;

    UINT16 u2StaId;
    UINT8 aucPadding[2];
    UINT32 au4StaAcRxTimer[4];
} UNI_EVENT_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T, *P_UNI_EVENT_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T;

typedef struct GNU_PACKED _UNI_EVENT_VOW_PARAM_T {
	/* Resp */
	UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T EventVowRxAtReportRxNonwifiTime; /* TAG 14 */
	UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME_T EventVowRxAtReportRxObssTime; /* TAG 15 */
	UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T EventVowRxAtReportMibObssTime; /* TAG 16 */
} UNI_EVENT_VOW_PARAM_T, *P_UNI_EVENT_VOW_PARAM_T;

/* =================== UNI_CMD_ID_VOW End ==================== */

VOID UniCmdResultRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len);

VOID UniEventAccessRegHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

#ifdef AIR_MONITOR
VOID UniEventSmeshInfoRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len);
#endif /* AIR_MONITOR */

VOID UniEventSERHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

VOID UniEventEDCCAHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

VOID UniEventMACInfoHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

#ifdef CFG_SUPPORT_FALCON_TXCMD_DBG
VOID UniEventTxCmdDbgHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);
#endif /* CFG_SUPPORT_FALCON_TXCMD_DBG */

VOID UniEventMibHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

#ifdef SCS_FW_OFFLOAD
VOID UniEventSCSHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);
#endif /* SCS_FW_OFFLOAD */

VOID UniEventTPCHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

VOID UniEventUnsolicitMainHandler(struct _RTMP_ADAPTER *pAd, PNDIS_PACKET net_pkt);

/* Here is related to Unsolicit Event */
VOID UniEventFwLog2HostHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

VOID UniEventIECountDownHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

VOID UniEventAssertDumpHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

VOID UniEventBeaconTimeoutHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

VOID UniEventPSSyncHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

VOID UniEventECCCalHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd);

VOID UniEventVowHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len);

#endif /* WIFI_UNIFIED_COMMAND */
#endif /* _CMM_FW_UNI_EVENT_H */
