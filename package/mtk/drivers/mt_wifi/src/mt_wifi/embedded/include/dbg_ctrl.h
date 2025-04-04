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

*/

#ifndef __DBG_CTRL_H__
#define __DBG_CTRL_H__

#define DEFAULT_FW_LOG_DESTINATION "/media/sda1/fw_log.bin"

#define FW_BIN_LOG_MAGIC_NUM    0x44D9C99A
#define FW_BIN_LOG_VERSION      0x01
#define FW_BIN_LOG_RSV          0x00
#ifdef DBG
#ifdef DBG_ENHANCE
#define DBG_PRINT_BUF_SIZE	160
#endif /* DBG_ENHANCE */
#endif /* DBG */

enum {
	FW_LOG_2_HOST_CTRL_OFF = 0,
	FW_LOG_2_HOST_CTRL_2_UART = 1,
	FW_LOG_2_HOST_CTRL_2_HOST = 2,
	FW_LOG_2_HOST_CTRL_2_EMI = 4,
	FW_LOG_2_HOST_CTRL_2_HOST_STORAGE = 8,
	FW_LOG_2_HOST_CTRL_2_HOST_ETHNET = 16,
};

typedef enum _ENUM_DGB_LOG_PKT_TYPE_T {
	DBG_LOG_PKT_TYPE_ICS = 0x0C,
	DBG_LOG_PKT_TYPE_PHY_ICS = 0x0D,
	DBG_LOG_PKT_TYPE_TXV = 0x11,
	DBG_LOG_PKT_TYPE_FTRACE = 0x12,
	DBG_LOG_PKT_TYPE_TRIG_FRAME = 0x13,
} ENUM_HW_LOG_PKT_TYPE_T;

typedef enum _ENUM_BIN_DBG_LOG_T {
	BIN_DBG_LOG_TRIGGER_FRAME = 0,
	BIN_DBG_LOG_NUM
} ENUM_BIN_DBG_LOG_T;

#ifdef FW_LOG_DUMP
#define SUPPORTED_FW_LOG_TYPE	0x1F
#define FW_LOG_TYPE_COUNT		5
#else
#define SUPPORTED_FW_LOG_TYPE	0x03
#define FW_LOG_TYPE_COUNT		2
#endif /* FW_LOG_DUMP */

typedef struct _ICS_AGG_HEADER {
	UINT16 rxByteCount;
	UINT16 frameCount:5;
	UINT16 reserved1:6;
	UINT16 pktType:5;
	UINT16 reserved2;
	UINT16 pseFid;
} ICS_AGG_HEADER, *PICS_AGG_HEADER;

#ifdef CONFIG_ICS_FRAME_HANDLE
typedef struct _ICS_RX_COMMON_B_FRAME_HEADER_V1 {
	UINT16 rxPpduDropCount:8;
	UINT16 icsSerialNumber:7;
	UINT16 bandIdx:1;
	UINT16 reserved1:11;
	UINT16 pktType:5;
	UINT32 timestamp;
	UINT32 rxv_c_b[18];
} ICS_RX_COMMON_B_FRAME_HEADER_V1, *PICS_RX_COMMON_B_FRAME_HEADER_V1;

typedef struct _ICS_RX_FRAME_0_HEADER_V1 {
	UINT16 reserved1:8;
	UINT16 icsSerialNumber:8;
	UINT16 reserved2:11;
	UINT16 pktType:5;
	UINT32 timestamp;
	UINT32 rxv_p_b[2];
	UINT32 rxv_p_a_0[2];
	UINT32 rxv_p_a_1[2];
	UINT32 _80211_MAC_header[15];
	UINT32 rxMpduCount:8;
	UINT32 fcsOkMpduCount:8;
	UINT32 reserved3:16;
} ICS_RX_FRAME_0_HEADER_V1, *PICS_RX_FRAME_0_HEADER_V1;

typedef struct _ICS_RX_FRAME_1_HEADER_V1 {
	UINT16 muUserId:4;
	UINT16 reserved1:4;
	UINT16 icsSerialNumber:8;
	UINT16 reserved2:11;
	UINT16 pktType:5;
	UINT32 timestamp;
	UINT32 rxv_p_b[2];
	UINT32 rxv_p_a_0[2];
	UINT32 rxv_p_a_1[2];
	UINT32 reserved3;
	UINT32 rxMpduCount:8;
	UINT32 fcsOkMpduCount:8;
	UINT32 reserved4:16;
} ICS_RX_FRAME_1_HEADER_V1, *PICS_RX_FRAME_1_HEADER_V1;

typedef struct _ICS_RX_COMMON_A_FRAME_HEADER_V1 {
	UINT16 reserved1:8;
	UINT16 icsSerialNumber:8;
	UINT16 reserved2:11;
	UINT16 pktType:5;
	UINT32 timestamp;
	UINT32 rxv_c_a[26];
	UINT32 reserved3[2];
} ICS_RX_COMMON_A_FRAME_HEADER_V1, *PICS_RX_COMMON_A_FRAME_HEADER_V1;

typedef enum _ENUM_ICS_PKT_TYPE_T {
	ICS_TX_COMMON_FRAME = 0x01,
	ICS_TX_FRAME		= 0x02,
	ICS_TX_FRAME_1		= 0x03,
	ICS_RX_COMMON_B_FRAME = 0x4,
	ICS_RX_FRAME_0		= 0x05,
	ICS_RX_FRAME_1		= 0x06,
	ICS_RX_COMMON_A_FRAME = 0x07,
	ICS_TX_ERROR_FRAME	= 0x08,
	ICS_RX_ERROR_FRAME	= 0x09,
	ICS_AGGREGATION_FRAME = 0x0c,
} ENUM_ICS_PKT_TYPE_T;

struct ics_frame_entry {
	UINT16 pkt_type;
	UINT16 frame_size;
	UINT32 (*handler)(RTMP_ADAPTER *pAd, UINT8 pktType, UINT8 *pFrameData,
						UINT8 user_idx, UINT8 band_idx);
};
#endif /* CONFIG_ICS_FRAME_HANDLE */

typedef struct _FW_LOG_CTRL {
	UINT32 fw_log_server_ip;
	UCHAR fw_log_server_mac[MAC_ADDR_LEN];
	UCHAR wmcpu_log_type;
	CHAR fw_log_dest_dir[32];
	UCHAR debug_level_ctrl[BIN_DBG_LOG_NUM];
	UINT16 fw_log_serialID_count;
} FW_LOG_CTRL;

typedef struct _FW_BIN_LOG_HDR_T {
	UINT32 u4MagicNum;
	UINT8  u1Version;
	UINT8  u1Rsv;
	UINT16 u2SerialID;
	UINT32 u4Timestamp;
	UINT16 u2MsgID;
	UINT16 u2Length;
} FW_BIN_LOG_HDR_T, *P_FW_BIN_LOG_HDR_T;

INT set_fw_log_dest_dir(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

INT set_binary_log(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

void rtmp_read_fw_log_dump_parms_from_file(
	RTMP_ADAPTER *pAd,
	CHAR *tmpbuf,
	CHAR *buffer);

NTSTATUS fw_log_to_file(
	IN PRTMP_ADAPTER pAd,
	IN PCmdQElmt CMDQelmt);

VOID fw_log_to_ethernet(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *fw_log,
	IN UINT32 log_len);

UINT16 Checksum16(UINT8 *pData, int len);

NTSTATUS
dbg_log_wrapper(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 ucPktType,
	IN UINT8 *pucData,
	IN UINT16 u2Length);
INT32 set_fwlog_serverip(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
INT32 set_fwlog_servermac(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
VOID print_fw_log_header(FW_BIN_LOG_HDR_T log_hdr);

#ifdef DBG
#ifdef DBG_ENHANCE
void mtwf_dbg_option(
	IN const BOOLEAN prtCatLvl,
	IN const BOOLEAN prtIntfName,
	IN const BOOLEAN prtThreadId,
	IN const BOOLEAN prtFuncLine);

void mtwf_dbg_prt(
	IN RTMP_ADAPTER *pAd,
	IN const UINT32 dbgCat,
	IN const UINT32 dbgLvl,
	IN const INT8   *pFunc,
	IN const UINT32 line,
	IN const INT8   *pFmt,
	...);
#endif /* DBG_ENHANCE */
#endif /* DBG */


#endif /* __DBG_CTRL_H__ */

