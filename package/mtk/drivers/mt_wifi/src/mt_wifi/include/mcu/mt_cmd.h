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
	mt_cmd.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __MT_CMD_H__
#define __MT_CMD_H__

#include "rtmp_def.h"
#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
#include "icap.h"
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
#include "twt_ctrl.h"
#endif /* WIFI_TWT_SUPPORT */
#include "he.h"
#endif /* DOT11_HE_AX */
#include "mgmt/mgmt_entrytb.h"
#include "txpwr/txpwr.h"

struct cmd_msg;
struct _MT_SWITCH_CHANNEL_CFG;
struct _MT_RTS_THRESHOLD_T;
struct _MT_RDG_CTRL_T;
struct _BSS_INFO_ARGUMENT_T;
struct _STA_ADMIN_CONFIG;
struct he_sta_info;

#ifdef BACKGROUND_SCAN_SUPPORT
struct _MT_BGND_SCAN_CFG;
struct _MT_BGND_SCAN_NOTIFY;
#endif /* BACKGROUND_SCAN_SUPPORT */

typedef VOID(*MSG_RSP_HANDLER)(struct cmd_msg *msg, char *payload, UINT16 payload_len);

#define CMD_QUERY 0
#define CMD_SET 1
#define CMD_NA 3

#define EXT_CMD_NA 0
#define P1_Q0 0x8000
#define CPU_TX_PORT 2
#define EFUSE_BLOCK_SIZE 16

#define NEW_MCU_INIT_CMD_API 1

#define MT_IGNORE_PAYLOAD_LEN_CHECK 0xffff

#define ROM_CODE_CMD_HEADER_LEN	12

#define BAND0                         0
#define BAND1                         1
#define BAND_NUM                      2

#define BAND0_RX_PCIE0				  1
#define BAND1_RX_PCIE0				  2
#define BAND1_RX_PCIE1				  3
#define BAND1_TX_PCIE1				  4
#define SKU_TABLE_SIZE               49
#define SKU_TOTAL_SIZE               53
#define SKU_TX_SPATIAL_STREAM_NUM     4
#define SKU_TX_SPATIAL_STREAM_1SS     0
#define SKU_TX_SPATIAL_STREAM_2SS     1
#define SKU_TX_SPATIAL_STREAM_3SS     2
#define SKU_TX_SPATIAL_STREAM_4SS     3
#define SKU_TX_SPATIAL_STREAM_NUM     4

#define THERMAL_TABLE_SIZE           15
#define THERMAL_TASK_NUM             34
#define MAX_CR4_QUERY_NUM		100

#define IPV6_ADDR_LEN                16

enum EEPROM_STORAGE_TYPE {
	EEPROM_PROM = 0,
	EEPROM_EFUSE = 1,
	EEPROM_FLASH = 2,
};

enum MCU_SRC_TO_DEST_INDEX_TYPE {
	HOST2N9     = 0,
	CR42N9      = 1,
	HOST2CR4    = 2,
	HOST2CR4N9  = 3,
	HOST2WO     = 4,
};

#define N92HOST HOST2N9
#define CR42HOST HOST2CR4

enum cmd_msg_state {
	illegal,                /* 0 */
	tx_start,               /* 1 */
	tx_kickout_fail,        /* 2 */
	tx_timeout_fail,        /* 3 */
	tx_retransmit,          /* 4 */
	tx_done,                /* 5 */
	wait_cmd_out,           /* 6 */
	wait_cmd_out_and_ack,   /* 7 */
	wait_ack,               /* 8 */
	rx_start,               /* 9 */
	rx_receive_fail,        /* a */
	rx_done,                /* b */
};

#define TX_DELAY_MODE_ARG1_TX_BATCH_CNT 1
#define TX_DELAY_MODE_ARG1_TX_DELAY_TIMEOUT_US 2
#define TX_DELAY_MODE_ARG1_PKT_LENGTHS 3

typedef enum _ENUM_EXT_CMD_CR4_SET_ID_T {
	CR4_SET_ID_HELP = 0,
	CR4_SET_ID_MAX_AMSDU_QUEUE_NUM = 1,
	CR4_SET_ID_READ_ONE_RX_TOKEN_WRITE_BACK_FIFO = 2,
	CR4_SET_ID_READ_ALL_RX_TOKEN_WRITE_BACK_FIFO = 3,
	CR4_SET_ID_TRIGGER_PDMA_RECEIVE = 4,
	CR4_SET_ID_TRIGGER_ASSERT = 5,
	CR4_SET_ID_TX_FRAGMENT_THRESHOLD = 6,
	CR4_SET_ID_MEM_QUERY = 7,
	CR4_SET_ID_MEM_SET = 8,
	CR4_SET_ID_STOP_RX_PDMA_RING_DEQUEUE = 9,
	CR4_SET_ID_START_RX_PDMA_RING_DEQUEUE = 0xa,
	CR4_SET_ID_CONFIG_POWER_SAVING_MODE = 0xb,
	CR4_SET_ID_CONFIG_TX_DELAY_MODE = 0xc,
	CR4_SET_ID_CONFIG_STA_AMSDU_MAX_NUM = 0xd,
	CR4_SET_ID_RED_ENABLE = 0xe,
	CR4_SET_ID_RED_SHOW_STA = 0xf,
	CR4_SET_ID_RED_TARGET_DELAY = 0x10,
	CR4_SET_ID_RED_ENTER_LOW_FREE_PLE_MODE = 0x11,
	CR4_SET_ID_WA_CAP = 0x12,
	WA_SET_OPTION_AC_TAIL_DROP_MIN_QUOTA = 0x13,
	WA_SET_OPTION_AC_TAIL_DROP_MAX_QUOTA = 0x14,
	WA_SET_OPTION_TXD_FLOW_CTRL = 0x20,
	WA_SET_OPTION_CONFIG_WMM_MODE = 0x21,
	WA_SET_OPTION_PKTLOSS_CHK = 0x23,
	WA_SET_OPTION_RED_QLEN_DROP_TOKEN = 0x24,
	WA_SET_OPTION_RED_QLEN_DROP_FREE_BOUND = 0x25,
	WA_SET_OPTION_RED_QLEN_DROP_THRESHOLD = 0x26,
	WA_SET_OPTION_RED_QLEN_DROP_DUMP = 0x27,
	WA_SET_OPTION_MPDU_RETRY_LIMIT = 0x30,
	WA_SET_OPTION_DABS_QOS_CMD = 0x31,
	WA_SET_OPTION_WED_VERSION = 0x32,
	WA_SET_OPTION_RED_SETTING_CMD = 0x40,
#ifdef AMPDU_CONF_SUPPORT
	WA_SET_AMPDU_RETRY_COUNT = 0xa3,
#endif
	CR4_SET_ID_NUM
} ENUM_EXT_CMD_CR4_SET_ID_T, *P_ENUM_EXT_CMD_CR4_SET_ID_T;

enum WA_CAPS_WED_WRAP_FORMAT_VERSION {
	WED_FORMAT_MT7615,
	WED_FORMAT_MT7622_CONNAC,
	WED_FORMAT_MT7915,
	WED_FORMAT_MT7986 = WED_FORMAT_MT7915,
	WED_FORMAT_MT7916 = WED_FORMAT_MT7915,
	WED_FORMAT_MT7981 = WED_FORMAT_MT7915,
};
#define WA_CAPS_OPTION_WED_WRAP_VERSION_MASK	BITS(0, 2)
#define WA_CAPS_OPTION_MEM_DMA			BIT(3)
#define WA_CAPS_OPTION_RX_REORDER		BIT(4)
#define WA_CAPS_OPTION_TX_FRAGMENT		BIT(5)
#define WA_CAPS_OPTION_RX_DEFRAGMENT		BIT(6)
#define WA_CAPS_OPTION_TX_MSUD_REF_COUNT	BIT(7)
#define WA_CAPS_OPTION_RED			BIT(8)
#define WA_CAPS_OPTION_TXD_OFFLOAD		BIT(9)
#define WA_CAPS_OPTION_ADAPT_AMSDU_LEN_PRE_RATE	BIT(10)
#define WA_CAPS_OPTION_TXS			BIT(11)
#define WA_CAPS_OPTION_BWF_LWC			BIT(12)


/**
 * The CONTROL_FLAG_INIT_COMBINATION enum parameters used by Host to init
 * the flags of one msg.
 *
 * @BIT0        NEED RSP        specific the cmd need event response.
 * @BIT1        NEED RETRY      specific if the cmd fail, it can retry.
 * @BIT2        NEED WAIT       specific the cmd will sync with FW.
 * @BIT3        CMD_SET_QUERY   specific cmd type is belong to set or query.
 *
 * @BIT4        CMD_NA          specific cmd type is Neither set nor query.
 *                              Note:
 *                                      bit 3 & 4 always mutual
 *                                      exclusive to each other.
 *
 * @BIT5        LEN_VAR         specific cmd type is expected event len variable.
 *                              The RX handler will only check the minimum event
 *                              size which specific in msg.
 *
 * @BIT7        NEED FRAG       expected cmd len exceed default block size
 *                              the cmd will be sent in Frag way.
 *
 * @Others                      Reserved
 *
 */
enum CONTROL_FLAG_INIT_COMBINATION {
	INIT_CMD_QUERY			            = 0x00,
	INIT_CMD_QUERY_AND_RSP              = 0x01,
	INIT_CMD_QUERY_AND_WAIT_RSP	    = 0x05,
	INIT_CMD_QUERY_AND_WAIT_RETRY_RSP	= 0x07,

	INIT_CMD_SET			            = 0x08,
	INIT_CMD_SET_AND_RSP			    = 0x09,
	INIT_CMD_SET_AND_RETRY		        = 0x0A,
	INIT_CMD_SET_AND_WAIT_RSP		    = 0x0D,
	INIT_CMD_SET_AND_WAIT_RETRY         = 0x0E,
	INIT_CMD_SET_AND_WAIT_RETRY_RSP	= 0x0F,

	INIT_CMD_NA			            = 0x10,
	INIT_CMD_NA_AND_WAIT_RETRY_RSP	    = 0x17,

	INIT_LEN_VAR_CMD_SET_AND_WAIT_RETRY_RSP = 0x27,
#ifdef WIFI_UNIFIED_COMMAND
	INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY 			  = 0x6A,
	INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP   = 0x6F,
	INIT_LEN_VAR_UNI_CMD_QUERY_AND_RETRY 		  = 0x62,
	INIT_LEN_VAR_UNI_CMD_QUERY_AND_WAIT_RETRY_RSP = 0x67,
#endif /* WIFI_UNIFIED_COMMAND */
};

#define CMD_FLAGS_MASK                          (0x7F)

#ifdef WIFI_UNIFIED_COMMAND
#define CMD_FLAG_UNI_CMD_MASK                   (0x40)
#define CMD_FLAG_UNI_CMD_OFFSET                 (6)
#endif /* WIFI_UNIFIED_COMMAND */

#define CMD_FLAG_CMD_LEN_VAR_MASK               (0x20)
#define CMD_FLAG_CMD_LEN_VAR_OFFSET             (5)

#define CMD_FLAG_CMD_NA_MASK                    (0x10)
#define CMD_FLAG_CMD_NA_OFFSET                  (4)

#define CMD_FLAG_SET_QUERY_MASK                 (0x08)
#define CMD_FLAG_SET_QUERY_OFFSET               (3)

#define CMD_FLAG_NEED_SYNC_WITH_FW_MASK         (0x04)
#define CMD_FLAG_NEED_SYNC_WITH_FW_OFFSET       (2)

#define CMD_FLAG_NEED_RETRY_MASK                (0x02)
#define CMD_FLAG_NEED_RETRY_OFFSET              (1)

#define CMD_FLAG_NEED_FW_RSP_MASK               (0x01)
#define CMD_FLAG_NEED_FW_RSP_OFFSET             (0)

/* CMD ATTRIBUTE initlized related Marco */
#define SET_CMD_ATTR_MCU_DEST(attr, cmd_dest) \
	((attr).mcu_dest = cmd_dest)
#define SET_CMD_ATTR_TYPE(attr, cmd_type) \
	((attr).type = cmd_type)
#define SET_CMD_ATTR_EXT_TYPE(attr, ext_cmd_type) \
	((attr).ext_type = ext_cmd_type)
#define SET_CMD_ATTR_CTRL_FLAGS(attr, cmd_flags) \
	((attr).ctrl.flags = cmd_flags)
#define SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, size) \
	((attr).ctrl.expect_size = (UINT16)size)
#define SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, wait_time) \
	((attr).ctrl.wait_ms_time = (UINT16)wait_time)
#define SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, write_back_buffer_in_callback) \
	((attr).rsp.wb_buf_in_calbk = (UINT32 *)write_back_buffer_in_callback)
#define SET_CMD_ATTR_RSP_HANDLER(attr, callback) \
	((attr).rsp.handler = callback)

/* CMD ATTRIBUTE ctrl flags related Macro */
#define GET_CMD_ATTR_CTRL_FLAGS(attr) \
	(attr.ctrl.flags & CMD_FLAGS_MASK)

#define IS_CMD_ATTR_LEN_VAR_FLAG_SET(attr) \
	(((attr).ctrl.flags & CMD_FLAG_CMD_LEN_VAR_MASK) ? TRUE : FALSE)
#define IS_CMD_ATTR_NA_FLAG_SET(attr) \
	(((attr).ctrl.flags & CMD_FLAG_CMD_NA_MASK) ? TRUE : FALSE)
#define IS_CMD_ATTR_SET_QUERY_FLAG_SET(attr) \
	(((attr).ctrl.flags & CMD_FLAG_SET_QUERY_MASK) ? TRUE : FALSE)
#define IS_CMD_ATTR_NEED_SYNC_WITH_FW_FLAG_SET(attr) \
	(((attr).ctrl.flags & CMD_FLAG_NEED_SYNC_WITH_FW_MASK) ? TRUE : FALSE)
#define IS_CMD_ATTR_NEED_RETRY_FLAG_SET(attr) \
	(((attr).ctrl.flags & CMD_FLAG_NEED_RETRY_MASK) ? TRUE : FALSE)
#define IS_CMD_ATTR_NEED_FW_RSP_FLAG_SET(attr) \
	(((attr).ctrl.flags & CMD_FLAG_NEED_FW_RSP_MASK) ? TRUE : FALSE)
#ifdef WIFI_UNIFIED_COMMAND
#define IS_CMD_ATTR_UNI_CMD(attr) \
		(((attr).ctrl.flags & CMD_FLAG_UNI_CMD_MASK) ? TRUE : FALSE)
#endif /* WIFI_UNIFIED_COMMAND */

/* CMD MSG initlize related Marco */
#define SET_CMD_MSG_PORT_QUEUE_ID(msg, hardware_port) \
	((msg)->pq_id = hardware_port)
#define SET_CMD_MSG_SEQUENCE(msg, msg_sequence) \
	((msg)->seq = msg_sequence)
#define SET_CMD_MSG_RETRY_TIMES(msg, cmd_retry_times) \
	((msg)->retry_times = cmd_retry_times)

#define SET_CMD_MSG_MCU_DEST(msg, cmd_dest) \
	SET_CMD_ATTR_MCU_DEST((msg)->attr, cmd_dest)
/* (msg)->attr.mcu_dest = msg_dest; */
#define SET_CMD_MSG_TYPE(msg, cmd_type) \
	SET_CMD_ATTR_TYPE((msg)->attr, cmd_type)
/* (msg)->attr.type = cmd_type; */
#define SET_CMD_MSG_EXT_TYPE(msg, ext_cmd_type) \
	SET_CMD_ATTR_EXT_TYPE((msg)->attr, ext_cmd_type)
/* (msg)->attr.ext_type = ext_cmd_type; */
#define SET_CMD_MSG_CTRL_FLAGS(msg, cmd_flags) \
	SET_CMD_ATTR_CTRL_FLAGS((msg)->attr, cmd_flags)
/* (msg)->attr.ctrl.flags = cmd_flags; */
#define SET_CMD_MSG_CTRL_RSP_EXPECT_SIZE(msg, size) \
	SET_CMD_ATTR_RSP_EXPECT_SIZE((msg)->attr, size)
/* (msg)->attr.ctrl.expect_size = size; */
#define SET_CMD_MSG_CTRL_RSP_WAIT_MS_TIME(msg, wait_time) \
	SET_CMD_ATTR_RSP_WAIT_MS_TIME((msg)->attr, wait_time)
/* (msg)->attr.ctrl.wait_ms_time = wait_time; */
#define SET_CMD_MSG_RSP_WB_BUF_IN_CALBK(msg, write_back_buffer_in_callback) \
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK((msg)->attr, write_back_buffer_in_callback)
/* (msg)->attr.rsp.wb_buf_in_calbk = (UINT32 *)write_back_buffer_in_callback; */
#define SET_CMD_MSG_RSP_HANDLER(msg, callback) \
	SET_CMD_ATTR_RSP_HANDLER((msg)->attr, callback)
/* (msg)->attr.rsp.handler = callback; */

/* CMD MSG flags related Macro, part I */
#define SET_MSG_FLAGS_VALUE(msg, val) \
	(msg)->attr.ctrl.flags = (((msg)->attr.ctrl.flags & CMD_FLAGS_MASK) \
							  | ((val) & CMD_FLAGS_MASK))
#define GET_MSG_FLAGS_VALUE(msg) \
	((((msg)->attr.ctrl.flags & CMD_FLAGS_MASK)) ? TRUE : FALSE)

#define SET_MSG_CMD_NA_FLAG(msg) \
	(msg)->attr.ctrl.flags = (((msg)->attr.ctrl.flags & CMD_FLAGS_MASK) \
							  | (1 << CMD_FLAG_CMD_NA_OFFSET))
#define GET_MSG_CMD_NA_FLAG(msg) \
	((((msg)->attr.ctrl.flags & CMD_FLAG_CMD_NA_MASK)) ? TRUE : FALSE)

#define SET_MSG_SET_QUERY_FLAG(msg) \
	(msg)->attr.ctrl.flags = (((msg)->attr.ctrl.flags & CMD_FLAGS_MASK) \
							  | (1 << CMD_FLAG_SET_QUERY_OFFSET))

#define GET_MSG_SET_QUERY_FLAG(msg) \
	((((msg)->attr.ctrl.flags & CMD_FLAG_SET_QUERY_MASK)) ? TRUE : FALSE)

#define SET_MSG_NEED_SYNC_WITH_FW_FLAG(msg) \
	(msg)->attr.ctrl.flags = (((msg)->attr.ctrl.flags & CMD_FLAGS_MASK) \
							  | (1 << CMD_FLAG_NEED_SYNC_WITH_FW_OFFSET))

#define GET_MSG_NEED_SYNC_WITH_FW_FLAG(msg) \
	((((msg)->attr.ctrl.flags & CMD_FLAG_NEED_SYNC_WITH_FW_MASK)) ? TRUE : FALSE)

#define SET_MSG_NEED_RETRY_FLAG(msg) \
	(msg)->attr.ctrl.flags = (((msg)->attr.ctrl.flags & CMD_FLAGS_MASK) \
							  | (1 << CMD_FLAG_NEED_RETRY_OFFSET))

#define GET_MSG_NEED_RETRY_FLAG(msg) \
	((((msg)->attr.ctrl.flags & CMD_FLAG_NEED_RETRY_MASK)) ? TRUE : FALSE)

#define SET_MSG_NEED_FW_RSP_FLAG(msg) \
	(msg)->attr.ctrl.flags = (((msg)->attr.ctrl.flags & CMD_FLAGS_MASK) \
							  | (1 << CMD_FLAG_NEED_FW_RSP_OFFSET))
#define GET_MSG_NEED_FW_RSP_FLAG(msg) \
	((((msg)->attr.ctrl.flags & CMD_FLAG_NEED_FW_RSP_MASK)) ? TRUE : FALSE)

/* CMD MSG flags related Macro, part II */
#define GET_CMD_MSG_CTRL_FLAGS(msg) \
	GET_CMD_ATTR_CTRL_FLAGS((msg)->attr)

#define IS_CMD_MSG_LEN_VAR_FLAG_SET(msg) \
	IS_CMD_ATTR_LEN_VAR_FLAG_SET((msg)->attr)
#define IS_CMD_MSG_NA_FLAG_SET(msg) \
	IS_CMD_ATTR_NA_FLAG_SET((msg)->attr)
#define IS_CMD_MSG_SET_QUERY_FLAG_SET(msg) \
	IS_CMD_ATTR_SET_QUERY_FLAG_SET((msg)->attr)
#define IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg) \
	IS_CMD_ATTR_NEED_SYNC_WITH_FW_FLAG_SET((msg)->attr)
#define IS_CMD_MSG_NEED_RETRY_FLAG_SET(msg) \
	IS_CMD_ATTR_NEED_RETRY_FLAG_SET((msg)->attr)
#define IS_CMD_MSG_NEED_FW_RSP_FLAG_SET(msg) \
	IS_CMD_ATTR_NEED_FW_RSP_FLAG_SET((msg)->attr)
#ifdef WIFI_UNIFIED_COMMAND
#define IS_CMD_MSG_UNI_CMD_FLAG_SET(msg) \
	IS_CMD_ATTR_UNI_CMD((msg)->attr)
#define UNI_CMD_WRAP_FRAG_NUM(total_frag, frag_num) \
			(((frag_num & 0xf) << 4) | (total_frag & 0xf))
#endif /* WIFI_UNIFIED_COMMAND */

#define IS_EXT_CMD_AND_SET_NEED_RSP(m) \
	((((((struct cmd_msg *)(m))->attr.type) == EXT_CID) && (IS_CMD_MSG_NEED_FW_RSP_FLAG_SET(m))) ? TRUE : FALSE)

typedef enum _RA_ACTION_CATEGORY {
	RA_DBG_CTRL = 0x0,
	SUPPORT_RATE_MCS_CAP_CTRL = 0x1,
	RA_ACTION_NUM
} RA_ACTION_CATEGORY, *P_RA_ACTION_CATEGORY;

typedef struct _CTRL_PARAM {
	UINT8       flags;
	UINT16      expect_size;
	UINT16      wait_ms_time;
} CTRL_PARAM, *P_CTRL_PARAM;

typedef struct _RSP_PARAM {
	UINT32 *wb_buf_in_calbk;
	MSG_RSP_HANDLER  handler;
} RSP_PARAM, *P_RSP_PARAM;

typedef struct _CMD_ATTRIBUTE {
	UINT16          mcu_dest;
	UINT8           type;
	UINT8           ext_type;
	CTRL_PARAM      ctrl;
	RSP_PARAM       rsp;
} CMD_ATTRIBUTE, *P_CMD_ATTRIBUTE;

/**
 * The cmd_msg is used by Host to communicate with FW.(i.e issue request)
 * The FW may response event or not depend on cmd flags setting.
 *
 * ----------------------------- Common Part -----------------------------
 * @Field attr                  cmd attribute descriptation
 * @Filed pq_id                 specific cmd physical target
 * @Field seq                   cmd sequence
 * @Filed retransmit_times      specific cmd retransmit_times
 * @Field priv
 * @Filed net_pkt
 * @Field cmd_tx_len

 * ------------------------------  OS Part  ------------------------------
 *
 *
 */
struct cmd_msg {
	CMD_ATTRIBUTE       attr;
	UINT16              pq_id;
	UINT8               seq;
#ifdef WIFI_UNIFIED_COMMAND
	UINT8               total_frag;
	UINT8               frag_num;
#endif /* WIFI_UNIFIED_COMMAND */
	UINT8               retry_times;
	enum cmd_msg_state  state;

	VOID                *priv;
	VOID                *net_pkt;
	VOID                *retry_pkt;
	UINT32              wcid;       /* Index of MacTableEntry */
	UINT32              cmd_tx_len;

	UINT8               need_sent_in_frag;
	UINT8               frag_cmd_sent_count;
	UINT16              orig_cmd_whole_len;

	UINT8               *back_orig_frag_fw_txd_and_cmd_struct_content;
	/* UINT8               *back_orig_frag_cmd_struct_content; */
	UINT32              back_orig_frag_cmd_struct_content_offset;
	UINT8               receive_frag_event_count;

	ULONG              sending_time_in_jiffies;        /* record the time in jiffies for send-the-command */
	ULONG              receive_time_in_jiffies;        /* record the time in jiffies for N9-firmware-response */

	DL_LIST             list;
	RTMP_OS_COMPLETION  ack_done;
	UINT32		cmd_return_status;
#ifdef DBG_STARVATION
	struct starv_dbg starv;
#endif /*DBG_STARVATION*/
};

#ifdef RT_BIG_ENDIAN
typedef	union _FW_TXD_0 {
	struct {
		UINT32 pq_id:16;
		UINT32 length:16;
	} field;
	UINT32 word;
} FW_TXD_0;
#else
typedef union _FW_TXD_0 {
	struct {
		UINT32 length:16;
		UINT32 pq_id:16;
	} field;
	UINT32 word;
} FW_TXD_0;
#endif

#define PKT_ID_CMD 0xA0
#define PKT_ID_EVENT 0xE000

#define WF_NUM 4

#ifdef RT_BIG_ENDIAN
typedef union _FW_TXD_1 {
	struct {
		UINT32 seq_num:8;
		UINT32 set_query:8;
		UINT32 pkt_type_id:8;
		UINT32 cid:8;
	} field;
	UINT32 word;
} FW_TXD_1;
#else
typedef union _FW_TXD_1 {
	struct {
		UINT32 cid:8;
		UINT32 pkt_type_id:8;
		UINT32 set_query:8;
		UINT32 seq_num:8;
	} field;
	UINT32 word;
} FW_TXD_1;
#endif

#define EXT_CID_OPTION_NEED_ACK 1
#define EXT_CID_OPTION_NO_NEED_ACK 0


#ifdef RT_BIG_ENDIAN
typedef union _FW_TXD_2 {
	struct {
		UINT32 ext_cid_option:8;
		UINT32 ucS2DIndex:8;
		UINT32 ext_cid:8;
		UINT32 ucD2B0Rev:8;
	} field;
	UINT32 word;
} FW_TXD_2;
#else
typedef union _FW_TXD_2 {
	struct {
		UINT32 ucD2B0Rev:8;
		UINT32 ext_cid:8;
		UINT32 ucS2DIndex:8;
		UINT32 ext_cid_option:8;
	} field;
	UINT32 word;
} FW_TXD_2;
#endif /* RT_BIG_ENDIAN */

/*
 * FW TX descriptor
 */
typedef struct GNU_PACKED _FW_TXD_ {
	FW_TXD_0 fw_txd_0;
	FW_TXD_1 fw_txd_1;
	FW_TXD_2 fw_txd_2;
	UINT32 au4D3toD7rev[5];
} FW_TXD;

/*
 * Command type table   layer 0
 */
enum MT_CMD_TYPE {
	MT_TARGET_ADDRESS_LEN_REQ = 0x01,
	MT_FW_START_REQ = 0x02,
	INIT_CMD_ACCESS_REG = 0x3,
	CMD_ID_NIC_POWER_CTRL = 0x4,
	MT_PATCH_START_REQ = 0x05,
	MT_PATCH_FINISH_REQ = 0x07,
	MT_PATCH_SEM_CONTROL = 0x10,
	MT_HIF_LOOPBACK = 0x20,
	CMD_CH_PRIVILEGE = 0x20,
	INIT_CMD_WIFI_DECOMPRESSION_START = 0x30,
	CMD_ACCESS_REG = 0xC2,
	INIT_CMD_ID_CR4 = 0xC4, /* for CR4test */
	EXT_CID = 0xED,
	MT_FW_SCATTER = 0xEE,
	MT_RESTART_DL_REQ = 0xEF,
};

#define CMD_START_LOAD		0x01
#define CMD_RAM_START_RUN	0x02
#define CMD_ROM_ACCESS_REG			0x03
#define CMD_PATCH_SEMAPHORE_CONTROL		0x10
#define CMD_PATCH_START			0x05
#define CMD_PATCH_FINISH			0x07
#define CMD_LOOPBACK_TEST			0x20

/*
 * Extension Command
 */
enum EXT_CMD_TYPE {
	EXT_CMD_ID_EFUSE_ACCESS = 0x01,
	EXT_CMD_RF_REG_ACCESS = 0x02,
	EXT_CMD_RF_TEST = 0x04,
	EXT_CMD_RADIO_ON_OFF_CTRL = 0x05,
	EXT_CMD_WIFI_RX_DISABLE = 0x06,
	EXT_CMD_PM_STATE_CTRL = 0x07,
	EXT_CMD_CHANNEL_SWITCH = 0x08,
	EXT_CMD_NIC_CAPABILITY = 0x09,
	EXT_CMD_PWR_SAVING = 0x0A,
	EXT_CMD_MULTIPLE_REG_ACCESS = 0x0E,
	EXT_CMD_AP_PWR_SAVING_CAPABILITY = 0xF,
	EXT_CMD_SEC_ADDREMOVE_KEY = 0x10,
	EXT_CMD_SET_TX_POWER_CTRL = 0x11,
	EXT_CMD_THERMO_CAL =	0x12,
	EXT_CMD_FW_LOG_2_HOST = 0x13,
	EXT_CMD_PS_RETRIEVE_START = 0x14,
#ifdef CONFIG_MULTI_CHANNEL
	EXT_CMD_ID_MCC_OFFLOAD_START = 0x15,
	EXT_CMD_ID_MCC_OFFLOAD_STOP  = 0x16,
#endif /* CONFIG_MULTI_CHANNEL */
	EXT_CMD_ID_LED = 0x17,
	EXT_CMD_ID_PACKET_FILTER = 0x18,
	EXT_CMD_BT_COEX = 0x19,
	EXT_CMD_ID_PWR_MGT_BIT_WIFI = 0x1B,
	EXT_CMD_ID_GET_TX_POWER = 0x1C,
#if defined(MT_MAC)
	EXT_CMD_ID_BF_ACTION = 0x1E,
#endif /* MT_MAC */
	EXT_CMD_ID_MEC_CTRL = 0x1F,
	EXT_CMD_EFUSE_BUFFER_MODE = 0x21,
	EXT_CMD_THERMAL_PROTECT = 0x23,
	EXT_CMD_ID_CLOCK_SWITCH_DISABLE = 0x24,
	EXT_CMD_STAREC_UPDATE = 0x25,
	EXT_CMD_ID_BSSINFO_UPDATE  = 0x26,
	EXT_CMD_ID_EDCA_SET = 0x27,
	EXT_CMD_ID_SLOT_TIME_SET = 0x28,
	EXT_CMD_ID_DEVINFO_UPDATE = 0x2a,
	EXT_CMD_ID_NOA_OFFLOAD_CTRL = 0x2B,
	EXT_CMD_ID_THERMAL_FEATURE_CTRL = 0x2C,
	EXT_CMD_TMR_CAL = 0x2D,
	EXT_CMD_ID_WAKEUP_OPTION = 0x2E,
	EXT_CMD_ID_GET_TX_STATISTICS = 0x30,
	EXT_CMD_ID_WTBL_UPDATE = 0x32,
	EXT_CMD_ID_TRGR_PRETBTT_INT_EVENT = 0x33,
	EXT_CMD_TDLS_CHSW = 0x34,
	EXT_CMD_ID_DRR_CTRL = 0x36,
	EXT_CMD_ID_BSSGROUP_CTRL = 0x37,
	EXT_CMD_ID_VOW_FEATURE_CTRL = 0x38,
#ifdef MT_DFS_SUPPORT    /* Jelly20141229 */
	EXT_CMD_ID_RDD_ON_OFF_CTRL = 0x3A,
#endif
	EXT_CMD_ID_GET_MAC_INFO = 0x3C,
#ifdef CONFIG_HW_HAL_OFFLOAD
	EXT_CMD_ID_ATE_TEST_MODE = 0x3D,
#endif
	EXT_CMD_ID_PROTECT_CTRL = 0x3e,
	EXT_CMD_ID_RDG_CTRL = 0x3f,
#ifdef CFG_SUPPORT_MU_MIMO
	EXT_CMD_ID_MU_MIMO = 0x40,
#endif
	EXT_CMD_ID_SNIFFER_MODE = 0x42,
	EXT_CMD_ID_GENERAL_TEST = 0x41,
	EXT_CMD_ID_WIFI_HIF_CTRL = 0x43,
	EXT_CMD_ID_TMR_CTRL = 0x44,
	EXT_CMD_ID_DBDC_CTRL = 0x45,
	EXT_CMD_MAC_ENABLE_CTRL = 0x46,
	EXT_CMD_ID_RX_HDR_TRANS  = 0x47,
	EXT_CMD_ID_CONFIG_MUAR = 0x48,
	EXT_CMD_ID_BCN_OFFLOAD = 0x49,
	EXT_CMD_ID_RX_AIRTIME_CTRL = 0x4a,
	EXT_CMD_ID_AT_PROC_MODULE = 0x4b,
#ifdef BACKGROUND_SCAN_SUPPORT
	EXT_CMD_ID_BGND_SCAN_NOTIFY = 0x4D,
#endif /* BACKGROUND_SCAN_SUPPORT */
	EXT_CMD_ID_SET_RX_PATH = 0x4e,
	EXT_CMD_ID_EFUSE_FREE_BLOCK = 0x4f,
	EXT_CMD_ID_AUTO_BA = 0x51,
	EXT_CMD_ID_MCAST_CLONE = 0x52,
	EXT_CMD_ID_MULTICAST_ENTRY_INSERT = 0x53,
	EXT_CMD_ID_MULTICAST_ENTRY_DELETE = 0x54,
#ifdef CFG_SUPPORT_MU_MIMO_RA
	EXT_CMD_ID_MU_MIMO_RA = 0x55,
#endif
#ifdef WIFI_SPECTRUM_SUPPORT
	EXT_CMD_ID_WIFI_SPECTRUM = 0x56,
#endif /* WIFI_SPECTRUM_SUPPORT */
	EXT_CMD_ID_DUMP_MEM = 0x57,
	EXT_CMD_ID_TX_POWER_FEATURE_CTRL = 0x58,
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	EXT_CMD_ID_RXDCOC_CAL_RESULT = 0x59,
	EXT_CMD_ID_TXDPD_CAL_RESULT = 0x60,
	EXT_CMD_ID_RDCE_VERIFY = 0x61,
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
	EXT_CMD_ID_GET_MIB_INFO = 0x5a,
#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) || defined(PRE_CAL_MT7622_SUPPORT)
	EXT_CMD_ID_TXLPF_CAL_INFO = 0x62,
	EXT_CMD_ID_TXIQ_CAL_INFO = 0x63,
	EXT_CMD_ID_TXDC_CAL_INFO = 0x64,
	EXT_CMD_ID_RXFI_CAL_INFO = 0x65,
	EXT_CMD_ID_RXFD_CAL_INFO = 0x66,
	EXT_CMD_ID_POR_CAL_INFO = 0x67,
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) || defined(PRE_CAL_MT7622_SUPPORT) */
#ifdef RED_SUPPORT
	EXT_CMD_ID_RED_ENABLE = 0x68,
	EXT_CMD_ID_RED_SHOW_STA = 0x69,
	EXT_CMD_ID_RED_TARGET_DELAY = 0x6A,
	EXT_CMD_ID_RED_TX_RPT = 0x6B,
#endif /* RED_SUPPORT */
	EXT_CMD_ID_PKT_BUDGET_CTRL_CFG = 0x6C,
	EXT_CMD_ID_TOAE_ENABLE = 0x6D,
	EXT_CMD_ID_BWF_LWC_ENABLE = 0x6E,
	EXT_CMD_ID_EDCCA_CTRL = 0x70,
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_PROXY_ARP)
	EXT_CMD_ID_HOTSPOT_INFO_UPDATE = 0x71,
#endif /* CONFIG_HOTSPOT_R2 */
	EXT_CMD_ID_EFUSE_ACCESS_CHECK = 0x72,
#ifdef GREENAP_SUPPORT
	EXT_CMD_ID_GREENAP_CTRL = 0x73,
#endif /* GREENAP_SUPPORT */
	EXT_CMD_ID_SET_MAX_PHY_RATE = 0x74,
	EXT_CMD_ID_CP_SUPPORT = 0x75,
#ifdef PRE_CAL_TRX_SET2_SUPPORT
	EXT_CMD_ID_PRE_CAL_RESULT = 0x76,
#endif /* PRE_CAL_TRX_SET2_SUPPORT */
#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
	EXT_CMD_ID_CAL_RESTORE_FROM_FILE = 0x77,
#endif /* CAL_BIN_FILE_SUPPORT */

	EXT_CMD_ID_LINK_TEST_FEATURE_CTRL = 0x78,

	EXT_CMD_ID_THERMAL_DBG_CMD = 0x79,

	/*ZERO_LOSS_CSA_SUPPORT*/
	EXT_CMD_ID_CHECK_PEER_STA_LINK = 0x7A,

	/*ZERO_PKT_LOSS_CSA_REFINE*/
	EXT_CMD_ID_SET_ZERO_PKT_LOSS_VARIABLE	= 0x7B,

	/*ZERO_PKT_LOSS_CSA_MAC_TX_STOP*/
	EXT_CMD_ID_SET_MAC_TX_ENABLE = 0x7C,

	/*ZERO_LOSS_CSA_SUPPORT*/
	EXT_CMD_ID_STA_PS_Q_LIMIT = 0x7D,

#if defined(A4_CONN) || defined(MBSS_AS_WDS_AP_SUPPORT)
	EXT_CMD_ID_MWDS_SUPPORT = 0x80,
#endif
	EXT_CMD_ID_SER = 0x81,
#ifdef SCS_FW_OFFLOAD
	EXT_CMD_ID_SCS_FEATURE_CTRL = 0x82,
#endif
#ifdef HOST_RESUME_DONE_ACK_SUPPORT
	EXT_CMD_ID_HOST_RESUME_DONE_ACK = 0x83,
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	EXT_CMD_ID_PCIE_ASPM_DYM_CTRL = 0x84,
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
#ifdef PRE_CAL_MT7622_SUPPORT
	EXT_CMD_ID_RFTEST_RECAL = 0x85,
	EXT_CMD_ID_TXDPD_CAL_INFO = 0x86,
#endif /* PRE_CAL_MT7622_SUPPORT */
	EXT_CMD_ID_HE_RA_CTRL = 0x87,
	EXT_CMD_ID_USE_VHTRATE_FOR_2G = 0x90,
	EXT_CMD_ID_FIX_RATE_WO_STA = 0x91,
#ifdef GN_MIXMODE_SUPPORT
	EXT_CMD_ID_GN_ENABLE = 0x92,
#endif /* GN_MIXMODE_SUPPORT */
	EXT_CMD_RXV_ENABLE_CTRL = 0x93,
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	EXT_CMD_ID_TWT_AGRT_UPDATE = 0x94,
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
	EXT_CMD_ID_FW_DBG_CTRL = 0x95,

	EXT_CMD_ID_TX_CCK_STREAM_CTRL = 0x96,
	EXT_CMD_ID_SHAPING_FILTER_DISABLE = 0x97,
	EXT_CMD_ID_TXCMD_CTRL = 0x98,
	EXT_CMD_ID_SYSDVT_TEST = 0x99,
	EXT_CMD_ID_OFF_CH_SCAN_CTRL = 0x9A,
	EXT_CMD_ID_EFUSE_BUFFER_RD = 0x9B,
	EXT_CMD_ID_ECC_OPER = 0x9C,
#ifdef MT_DFS_SUPPORT
	EXT_CMD_ID_SET_RDM_RADAR_THRES = 0x9D,
#endif
	EXT_CMD_ID_CAL_CTRL = 0x9E,

#ifdef CFG_SUPPORT_FALCON_MURU
	EXT_CMD_ID_MURU_CTRL = 0x9F,
#endif
#ifdef WIFI_EAP_FEATURE
	EXT_CMD_ID_EAP_CTRL = 0xA0,
#endif
	EXT_CMD_ID_TPC_CTRL = 0xA1,
	EXT_CMD_ID_DBG_TXCMD = 0xA2,
	EXT_CMD_ID_RDD_IPI_HIST_CTRL = 0xA3,
	EXT_CMD_ID_RX_STAT_INFO = 0xA4,
#ifdef WIFI_GPIO_CTRL
	EXT_CMD_ID_GPIO_CTRL = 0xA5,
#endif /* WIFI_GPIO_CTRL */
#ifdef OCE_SUPPORT
	EXT_CMD_ID_FD_FRAME_OFFLOAD = 0xA6,
#endif /* OCE_SUPPORT */
#ifdef IGMP_TVM_SUPPORT
	EXT_CMD_ID_IGMP_MULTICAST_SET_GET = 0xA7,
#endif /* IGMP_TVM_SUPPORT */

#ifdef CFG_SUPPORT_FALCON_SR
	EXT_CMD_ID_SR_CTRL = 0xA8,
#endif /* CFG_SUPPORT_FALCON_SR */
#if defined(PRE_CAL_MT7626_SUPPORT) || defined(PRE_CAL_MT7915_SUPPORT) || \
	defined(PRE_CAL_MT7986_SUPPORT) || defined(PRE_CAL_MT7981_SUPPORT) || \
	defined(PRE_CAL_MT7916_SUPPORT)
	EXT_CMD_ID_GROUP_PRE_CAL_INFO = 0xAB,
	EXT_CMD_ID_DPD_FLATNESS_INFO = 0xAC,
#endif
/* PRE_CAL_MT7663_SUPPORT || PRE_CAL_MT7626_SUPPORT || PRE_CAL_MT7915_SUPPORT ||
 * PRE_CAL_MT7986_SUPPORT || PRE_CAL_MT7981_SUPPORT
 */
	EXT_CMD_ID_PHY_STAT_INFO = 0xAD,
	EXT_CMD_ID_CONFIG_SMESH = 0xAE,
#ifdef WIFI_MODULE_DVT
	EXT_CMD_ID_MDVT = 0xAF,
#endif /* WIFI_MODULE_DVT */
#ifdef TXRX_STAT_SUPPORT
	EXT_CMD_ID_GET_STA_TX_STAT = 0xB0,
#endif
#ifdef MT_DFS_SUPPORT
	EXT_CMD_ID_SET_RDM_TEST_PATTERN = 0xB2,
#endif
	EXT_CMD_ID_RX_STAT_USER_CTRL = 0xB3,
#ifdef DSCP_PRI_SUPPORT
	EXT_CMD_ID_SET_DSCP_PRI = 0xB4,
#endif /*DSCP_PRI_SUPPORT*/
	EXT_CMD_ID_GET_ALL_STA_STATS = 0xB5,
	EXT_CMD_ID_PP_CTRL = 0xB6,
	EXT_CMD_ID_CFG = 0xB7,
	EXT_CMD_ID_ENABLE_NOISEFLOOR = 0xB8,
	EXT_CMD_ID_RA_CTRL = 0xB9,
	EXT_CMD_ID_EDCCA = 0xBA,
	EXT_CMD_ID_IGMP_CMD = 0xBC,
	EXT_CMD_ID_IGMP_FLOODING_CMD = 0xBD,
	EXT_CMD_ID_HWCFG = 0xBE,
	EXT_CMD_ID_VLAN_UPDATE = 0xBF,
#ifdef CFG_SUPPORT_CSI
	EXT_CMD_ID_CSI_CTRL = 0xC2,
#endif
#ifdef DYNAMIC_WMM_SUPPORT
	EXT_CMD_ID_SET_DYN_WMM = 0xC3,
#endif /* DYNAMIC_WMM_SUPPORT */
	EXT_CMD_ID_CFG_RD = 0xC4,
#ifdef IPI_SCAN_SUPPORT
	EXT_CMD_ID_RDD_IPI_SCAN_HIST = 0xC5,
#endif
	EXT_CMD_ID_RTS_THEN_CTS = 0xC6,
	EXT_CMD_ID_FAST_PATH_CAL_MIC = 0xD5,
#ifdef WIFI_MD_COEX_SUPPORT
	EXT_CMD_ID_UPDATE_3WIRE_GRP = 0xFA,
	EXT_CMD_ID_SET_IDC_STATE = 0xFB,
	EXT_CMD_ID_GET_IDC_INFO = 0xFC,
	EXT_CMD_ID_GET_LTE_CHN = 0xFD,
	EXT_CMD_ID_APCCCI_MSG = 0xFE,
#endif /* WIFI_MD_COEX_SUPPORT */
#ifdef SWACI_MECHANISM
	EXT_CMD_ID_RLM_SWLNA_ACI_CTRL = 0xC0,
#endif
};

typedef enum _LINK_TEST_ACTION_CATEGORY {
	LINK_TEST_TX_CSD = 0,
	LINK_TEST_RX,
	LINK_TEST_TXPWR,
	LINK_TEST_TXPWR_UP_TABLE,
	LINK_TEST_ACR,
	LINK_TEST_RCPI,
	LINK_TEST_SEIDX,
	LINK_TEST_RCPI_MA,
	LINK_TEST_ACTION_NUM
} LINK_TEST_ACTION_CATEGORY, *P_LINK_TEST_ACTION_CATEGORY;

/* CR4 test */
typedef enum _EXT_CMD_ID_CR4_T {
	EXT_CMD_ID_CR4_QUERY = 0,
	EXT_CMD_ID_CR4_SET,
	EXT_CMD_ID_CR4_CAPABILITY,
	EXT_CMD_ID_CR4_DEBUG,
	EXT_CMD_ID_CR4_MAX_NUM,
} EXT_CMD_ID_CR4_T;

typedef enum _EXT_CMD_CR4_QUERY_OPTION_T {
	CR4_QUERY_OPTION_HELP = 0,
	CR4_QUERY_OPTION_SYSTEM = 1,
	CR4_QUERY_OPTION_STACK_BOUNDARY = 2,
	CR4_QUERY_OPTION_OS_TASK = 3,
	CR4_QUERY_OPTION_PDMA_INFO = 4,
	CR4_QUERY_OPTION_PDMA_DEBUG_PROBE = 5,
	CR4_QUERY_OPTION_CPU_UTILIZATION = 6,
	CR4_QUERY_OPTION_PACKET_STATISTICS = 7,
	CR4_QUERY_OPTION_WIFI_INFO = 8,
	CR4_QUERY_OPTION_RESET_WIFI_INFO = 9,
	CR4_QUERY_OPTION_BSS_TLB_STA_RECORD = 0xa,
	CR4_QUERY_OPTION_CR4_STATUS_CR = 0xb,
	CR4_QUERY_OPTION_SHOW_CR4_INTERNAL_BUFFER_STATUS = 0xc,
	CR4_QUERY_OPTION_DUMP_RX_REORDER_QUEUE_INFO = 0xd,
	CR4_QUERY_OPTION_SHOW_RX_REORDER_QUEUE_LEN = 0xe,
	CR4_QUERY_OPTION_SHOW_RXCUTDISP_REG_CONTENT = 0xf,
	CR4_QUERY_OPTION_SHOW_CR4_CAPABILITY_DEBUG_SETTING = 0x10,
	CR4_QUERY_OPTION_SHOW_CR4_POWER_SAVING_MODE = 0x11,
	CR4_QUERY_OPTION_GET_BSS_ACQ_PKT_NUM = 0x12,
	CR4_QUERY_OPTION_GET_BSS_HOTSPOT_CAPABILITY = 0x13,
	CR4_QUERY_OPTION_SHOW_TASK_INFO = 0x14,
	CR4_QUERY_OPTION_GET_TX_STATISTICS = 0x15,
	CR4_QUERY_OPTION_MAX_NUM,
} EXT_CMD_CR4_QUERY_OPTION_T;

/*
 * WO CPU Command
 */
enum WO_CMD_ID {
	WO_CMD_WED_START = 0x0000,
	WO_CMD_WED_CFG  = WO_CMD_WED_START,
	WO_CMD_WED_RX_STAT = 0x0001,
	WO_CMD_RRO_SER = 0x0002,
	WO_CMD_DBG_INFO = 0x0003,
	WO_CMD_DEV_INFO = 0x0004,
	WO_CMD_BSS_INFO = 0x0005,
	WO_CMD_STA_REC = 0x0006,
	WO_CMD_DEV_INFO_DUMP = 0x0007,
	WO_CMD_BSS_INFO_DUMP = 0x0008,
	WO_CMD_STA_REC_DUMP = 0x0009,
	WO_CMD_BA_INFO_DUMP = 0x000A,
	WO_CMD_FBCMD_Q_DUMP = 0x000B,
	WO_CMD_FW_LOG_CTRL = 0x000C,
	WO_CMD_LOG_FLUSH = 0x000D,
	WO_CMD_CHANGE_STATE = 0x000E,
	WO_CMD_CPU_STATS_ENABLE = 0x000F,
	WO_CMD_CPU_STATS_DUMP = 0x0010,
	WO_CMD_EXCEPTION_INIT = 0x0011,
	WO_CMD_PROF_CTRL = 0x0012,
	WO_CMD_STA_BA_DUMP = 0x0013,
	WO_CMD_BA_CTRL_DUMP = 0x0014,
	WO_CMD_RXCNT_CTRL = 0x0015,
	WO_CMD_RXCNT_INFO = 0x0016,
	WO_CMD_SET_CAP = 0x0017,
	WO_CMD_WED_END
};

enum EXT_CMD_TAG_ID {
	EXT_CMD_TAG_RDG = 0x01,
	EXT_CMD_TAG_RXMAXLEN = 0x02,
	EXT_CMD_TAG_TR_STREAM = 0x03,
	EXT_CMD_TAG_UPDATE_BA = 0x04,
	EXT_CMD_TAG_SET_MacTXRX = 0x05,
	EXT_CMD_TAG_SET_TXSCF = 0x06,
	EXT_CMD_TAG_SET_RXPATH = 0x07,
	EXT_CMD_TAG_RX_HDR_TRNS = 0x08,
	EXT_CMD_TAG_RX_HDR_TRNSBL = 0x09,
	EXT_CMD_TAG_RX_GROUP = 0x0a,
};

enum {
	CH_SWITCH_BY_NORMAL_TX_RX         = 0,
	CH_SWITCH_INTERNAL_USED_BY_FW_0   = 1,
	CH_SWITCH_INTERNAL_USED_BY_FW_1   = 1,
	CH_SWITCH_SCAN                    = 3,
	CH_SWITCH_INTERNAL_USED_BY_FW_3   = 4,
	CH_SWITCH_DFS                     = 5, /* Jelly20150123 */
	CH_SWITCH_BACKGROUND_SCAN_START   = 6,
	CH_SWITCH_BACKGROUND_SCAN_RUNNING = 7,
	CH_SWITCH_BACKGROUND_SCAN_STOP    = 8,
	CH_SWITCH_SCAN_BYPASS_DPD         = 9,
	CH_SWITCH_THERMAL_RE_CAL          = 10,
	CH_SWITCH_RE_CAL_TRIGGER          = 11,
	CH_SWITCH_TEMP_BB_HL              = 12,
	CH_SWITCH_TEST_MODE_RE_CAL        = 13,
	CH_SWITCH_MP_LINE_DNL_CAL         = 14,
	CH_IGNORE_RADAR                   = 100
};

#ifdef CONFIG_MULTI_CHANNEL
typedef struct GNU_PACKED _EXT_CMD_MCC_START_T {
	/* Common setting from DW0~3 */
	/* DW0 */
	UINT16     u2IdleInterval;
	UINT8      ucRepeatCnt;
	UINT8      ucStartIdx;

	/* DW1 */
	UINT32     u4StartInstant;

	/* DW2,3 */
	UINT16     u2FreePSEPageTh;
	UINT8      ucPreSwitchInterval;
	UINT8      aucReserved0[0x5];

	/* BSS0 setting from DW4~7 */
	/* DW4 */
	UINT8      ucWlanIdx0;
	UINT8      ucPrimaryChannel0;
	UINT8      ucCenterChannel0Seg0;
	UINT8      ucCenterChannel0Seg1;

	/* DW5 */
	UINT8      ucBandwidth0;
	UINT8      ucTrxStream0;
	UINT16     u2StayInterval0;

	/* DW6 */
	UINT8     ucRole0;
	UINT8     ucOmIdx0;
	UINT8     ucBssIdx0;
	UINT8     ucWmmIdx0;

	/* DW7 */
	UINT8     aucReserved1[0x4];

	/* BSS1 setting from DW8~11 */
	/* DW8 */
	UINT8     ucWlanIdx1;
	UINT8     ucPrimaryChannel1;
	UINT8     ucCenterChannel1Seg0;
	UINT8     ucCenterChannel1Seg1;

	/* DW9 */
	UINT8     ucBandwidth1;
	UINT8     ucTrxStream1;
	UINT16    u2StayInterval1;

	/* DW10 */
	UINT8     ucRole1;
	UINT8     ucOmIdx1;
	UINT8     ucBssIdx1;
	UINT8     ucWmmIdx1;

	/* DW11 */
	UINT8     aucReserved2[0x4];
} EXT_CMD_MCC_START_T, *P_EXT_CMD_MCC_START_T;

typedef struct GNU_PACKED _EXT_CMD_MCC_STOP_T {
	/* DW0 */
	UINT8      ucParkIdx;
	UINT8      ucAutoResumeMode;
	UINT16     u2AutoResumeInterval;

	/* DW1 */
	UINT32     u4AutoResumeInstant;

	/* DW2 */
	UINT16     u2IdleInterval;
	UINT8      aucReserved[2];

	/* DW3 */
	UINT16     u2StayInterval0;
	UINT16     u2StayInterval1;
} EXT_CMD_MCC_STOP_T, *P_EXT_CMD_MCC_STOP_T;
#endif /* CONFIG_MULTI_CHANNEL */

#ifdef MT_MAC_BTCOEX
/*
 * Coex Sub
 */
enum EXT_BTCOEX_SUB {
	COEX_SET_PROTECTION_FRAME = 0x1,
	COEX_WIFI_STATUS_UPDATE  = 0x2,
	COEX_UPDATE_BSS_INFO = 0x03,
};

/*
 * Coex status bit
 */
enum EXT_BTCOEX_STATUS_bit {
	COEX_STATUS_RADIO_ON = 0x01,
	COEX_STATUS_SCAN_G_BAND = 0x02,
	COEX_STATUS_SCAN_A_BAND = 0x04,
	COEX_STATUS_LINK_UP = 0x08,
	COEX_STATUS_BT_OVER_WIFI = 0x10,
};

enum EXT_BTCOEX_PROTECTION_MODE {
	COEX_Legacy_CCK = 0x00,
	COEX_Legacy_OFDM = 0x01,
	COEX_HT_MIX = 0x02,
	COEX_HT_Green = 0x03,
	COEX_VHT = 0x04,
};

enum EXT_BTCOEX_OFDM_PROTECTION_RATE {
	PROTECTION_OFDM_6M = 0x00,
	PROTECTION_OFDM_9M = 0x01,
	PROTECTION_OFDM_12M = 0x02,
	PROTECTION_OFDM_18M = 0x03,
	PROTECTION_OFDM_24M = 0x04,
	PROTECTION_OFDM_36M = 0x05,
	PROTECTION_OFDM_48M = 0x06,
	PROTECTION_OFDM_54M = 0x07,
};
/*
 * Coex status bit
 */

typedef enum _WIFI_STATUS {
	STATUS_RADIO_ON = 0,
	STATUS_RADIO_OFF = 1,
	STATUS_SCAN_G_BAND = 2,
	STATUS_SCAN_G_BAND_END = 3,
	STATUS_SCAN_A_BAND = 4,
	STATUS_SCAN_A_BAND_END = 5,
	STATUS_LINK_UP = 6,
	STATUS_LINK_DOWN = 7,
	STATUS_BT_OVER_WIFI = 8,
	STATUS_BT_MAX,
} WIFI_STATUS;
#endif
/*
 * Extension Event
 */
enum EXT_EVENT_TYPE {
	EXT_EVENT_CMD_RESULT = 0x0,
	EXT_EVENT_CMD_ID_EFUSE_ACCESS  = 0x1,
	EXT_EVENT_RF_REG_ACCESS = 0x2,
	EXT_EVENT_ID_RF_TEST = 0x4,
	EXT_EVENT_ID_PS_SYNC = 0x5,
	EXT_EVENT_MULTI_CR_ACCESS = 0x0E,
	EXT_EVENT_FW_LOG_2_HOST = 0x13,
	EXT_EVENT_BT_COEX = 0x19,
	EXT_EVENT_BEACON_LOSS = 0x1A,
	EXT_EVENT_ID_GET_TX_POWER = 0x1C,
	EXT_EVENT_THERMAL_PROTECT = 0x22,
	EXT_EVENT_ID_ASSERT_DUMP = 0x23,
	EXT_EVENT_ID_THERMAL_FEATURE_CTRL = 0x2C,
	EXT_EVENT_ID_ROAMING_DETECTION_NOTIFICATION = 0x2d,
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	EXT_EVENT_RA_THROUGHPUT_BURST = 0x2F,
	EXT_EVENT_GET_TX_STATISTIC = 0x30,
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	EXT_EVENT_PRETBTT_INT = 0x31,
	EXT_EVENT_ID_BF_STATUS_READ = 0x35,
	EXT_EVENT_ID_DRR_CTRL = 0x36,
	EXT_EVENT_ID_BSSGROUP_CTRL = 0x37,
	EXT_EVENT_ID_VOW_FEATURE_CTRL = 0x38,
#ifdef MT_DFS_SUPPORT    /* Jelly20141229 */
	EXT_EVENT_ID_RDD_REPORT = 0x3A,
	EXT_EVENT_ID_CAC_END = 0x3E,
#endif
	EXT_EVENT_ID_MEC_INFO_READ = 0x3F,
	EXT_EVENT_ID_MAC_INFO = 0x3C,
	EXT_EVENT_ID_ATE_TEST_MODE = 0x3D,
	EXT_EVENT_ID_RX_AIRTIME_CTRL = 0x4a,
	EXT_EVENT_ID_AT_PROC_MODULE = 0x4b,
	EXT_EVENT_ID_MAX_AMSDU_LENGTH_UPDATE = 0x4c,
	EXT_EVENT_ID_EFUSE_FREE_BLOCK = 0x4D,
	EXT_EVENT_ID_BA_TRIGGER = 0x4E,
	EXT_EVENT_CSA_NOTIFY = 0x4F,
#ifdef WIFI_SPECTRUM_SUPPORT
	EXT_EVENT_ID_WIFI_SPECTRUM = 0x50,
#endif /* WIFI_SPECTRUM_SUPPORT */
	EXT_EVENT_TMR_CALCU_INFO = 0x51,
	EXT_EVENT_ID_BSS_ACQ_PKT_NUM = 0x52,
	EXT_EVENT_ID_TX_POWER_FEATURE_CTRL = 0x58,
#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
	EXT_EVENT_ID_TXLPF_CAL_INFO = 0x62,
	EXT_EVENT_ID_TXIQ_CAL_INFO = 0x63,
	EXT_EVENT_ID_TXDC_CAL_INFO = 0x64,
	EXT_EVENT_ID_RXFI_CAL_INFO = 0x65,
	EXT_EVENT_ID_RXFD_CAL_INFO = 0x66,
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */
#ifdef ZERO_LOSS_CSA_SUPPORT
	EXT_EVENT_BCN_TX_NOTIFY = 0x67,
	EXT_EVENT_ID_SEND_WCID = 0x68,
#endif /*ZERO_LOSS_CSA_SUPPORT*/
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	EXT_EVENT_G_BAND_256QAM_PROBE_RESULT = 0x6B,
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
#ifdef DABS_QOS
	EXT_EVENT_ID_FASTPATH_RPT = 0x6E,
#endif
#ifdef RED_SUPPORT
	EXT_EVENT_ID_MPDU_TIME_UPDATE = 0x6F,
	EXT_EVENT_ID_RED_TX_RPT = 0x70,
#endif
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_PROXY_ARP)
	EXT_EVENT_ID_INFORM_HOST_REPROCESS_PKT = 0x71,
	EXT_EVENT_ID_GET_CR4_HOTSPOT_CAPABILITY = 0x72,
#endif /* CONFIG_HOTSPOT_R2 */
	EXT_EVENT_ID_ACCESS_EFUSE_CHECK = 0x73,
	EXT_EVENT_GET_CR4_TX_STATISTICS = 0x74,
	EXT_EVENT_ID_BCC_NOTIFY = 0x75,
	EXT_EVENT_ID_HERA_INFO_CTRL = 0x87,
#ifdef CFG_SUPPORT_FALCON_MURU
	EXT_EVENT_ID_MURU_CTRL = 0x89,
#endif
	EXT_EVENT_ID_EFUSE_BUFFER_MODE_READ = 0x8a,
	EXT_EVENT_ID_ECC_RESULT = 0x8B,
#ifdef IGMP_TVM_SUPPORT
	EXT_EVENT_ID_IGMP_MULTICAST_RESP = 0x92,
#endif
	EXT_EVENT_ID_SYSDVT_TEST = 0x99,
	EXT_EVENT_ID_DBG_TXCMD = 0xA2,
	EXT_EVENT_ID_RDD_IPI_HIST_CTRL = 0xA3,
	EXT_EVENT_ID_RX_STAT_INFO = 0xA4,
	EXT_EVENT_ID_TPC_INFO = 0xA5,
#ifdef CFG_SUPPORT_FALCON_SR
	EXT_EVENT_ID_SR_INFO = 0xA8,
#endif /* CFG_SUPPORT_FALCON_SR */
	EXT_EVENT_ID_PHY_STAT_INFO = 0xAD,
#ifdef TXRX_STAT_SUPPORT
	EXT_EVENT_ID_GET_STA_TX_STAT = 0xA9,
#endif
	EXT_EVENT_ID_GET_ALL_STA_STATS = 0xB5,
	EXT_EVENT_ID_ENABLE_NOISEFLOOR = 0xB8,
	EXT_EVENT_ID_EDCCA = 0XBA,
	EXT_EVENT_ID_RXFE_LOSS_COMP    = 0xBD,
	EXT_EVENT_ID_HWCFG_READ = 0xBE,
#ifdef CFG_SUPPORT_CSI
	EXT_EVENT_ID_CSI_CTRL = 0xC2,
#endif
#ifdef WIFI_TWT_SUPPORT
	EXT_EVENT_ID_TWT_RESUME_INFO = 0xC3,
#endif /* WIFI_TWT_SUPPORT */
#ifdef IPI_SCAN_SUPPORT
	EXT_EVENT_ID_RDD_IPI_SCAN_HIST = 0xC5,
#endif
#ifdef WIFI_MD_COEX_SUPPORT
	EXT_EVENT_ID_LTE_UNSAFE_CHN_REPORT    = 0xFD,
	EXT_EVENT_ID_APCCCI_MSG = 0xFE,
#endif /* WIFI_MD_COEX_SUPPORT */
};

/*
 * DownLoad Type
 */
enum {
	DownLoadTypeA,
	DownLoadTypeB,
	DownLoadTypeCONNAC,
};

#ifdef RT_BIG_ENDIAN
typedef union _FW_RXD_0 {
	struct {
		UINT32 pkt_type_id:16;
		UINT32 length:16;
	} field;
	UINT32 word;
} FW_RXD_0;
#else
typedef union _FW_RXD_0 {
	struct {
		UINT32 length:16;
		UINT32 pkt_type_id:16;
	} field;
	UINT32 word;
} FW_RXD_0;
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
typedef union _FW_RXD_1 {
	struct {
		UINT32 rsv:8;
		UINT32 option:8;
		UINT32 seq_num:8;
		UINT32 eid:8;
	} field;
	UINT32 word;
} FW_RXD_1;
#else
typedef union _FW_RXD_1 {
	struct {
		UINT32 eid:8;
		UINT32 seq_num:8;
		UINT32 option:8;
		UINT32 rsv:8;
	} field;
	UINT32 word;
} FW_RXD_1;
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
typedef union _FW_RXD_2 {
	struct {
		UINT32 s2d_index:8;
		UINT32 rsv:16;
		UINT32 ext_eid:8;
	} field;
	UINT32 word;
} FW_RXD_2;
#else
typedef union _FW_RXD_2 {
	struct {
		UINT32 ext_eid:8;
		UINT32 rsv:16;
		UINT32 s2d_index:8;
	} field;
	UINT32 word;
} FW_RXD_2;
#endif /* RT_BIG_ENDIAN */

/*
 * Event structure
 */
typedef struct GNU_PACKED _EVENT_RXD_ {
	FW_RXD_0 fw_rxd_0;
	FW_RXD_1 fw_rxd_1;
	FW_RXD_2 fw_rxd_2;
} EVENT_RXD;

/*
 * Event type table
 */
enum MT_EVENT_TYPE {
	MT_TARGET_ADDRESS_LEN_RSP = 0x01,
	MT_FW_START_RSP = 0x01,
	GENERIC_EVENT = 0x01,
	EVENT_ACCESS_REG = 0x02,
	MT_PATCH_SEM_RSP = 0x04,
	EVENT_CH_PRIVILEGE = 0x18,
	EXT_EVENT = 0xED,
	MT_RESTART_DL_RSP = 0xEF,
};

#define REL_PATCH_SEM	0
#define GET_PATCH_SEM	1

/*
 * target address/length request cmd: data mode
 * bit(0)  : encrypt or not.
 * bit(1,2): encrypt key index.
 * bit(3)  : reset security engine's IV.
 * bit(4)  : working PDA. 0 for WMCPU, 1 for WACPU.
 * bit(6)  : encrypt mode, 1 for scramble, 0 for AES.
 * bit(31) : need command response
 */
#define MODE_ENABLE_ENCRY (1 << 0)
#define MODE_SET_KEY(p) (((p) & 0x03) << 1)
#define MODE_RESET_SEC_IV (1 << 3)
#define MODE_WORKING_PDA_OPTION (1 << 4)
#define MODE_ENCRY_MODE_SEL (1 << 6)
#define MODE_TARGET_ADDR_LEN_NEED_RSP (1 << 31)

/*
 * fw start cmd: override
 * bit(0)  : override RAM code starting address or not.
 * bit(2)  : working PDA. 0 for WMCPU, 1 for WACPU.
 */
#define FW_START_OVERRIDE_START_ADDRESS (1 << 0)
#define FW_START_DELAY_CALIBRATION      (1 << 1)
#define FW_START_WORKING_PDA_OPTION     (1 << 2)
#define FW_START_CRC_CHECK_OFFSET       (1 << 3)
#define FW_CHANGE_DECOMPRESSION_TMP_ADDRESS     (1 << 4)
/*
 * Erro code for target address/length response
 */
enum {
	TARGET_ADDRESS_LEN_SUCCESS,
};

/*
 * Error code for cmd(event) firmware start response
 */
enum {
	WIFI_FW_DOWNLOAD_SUCCESS,
	WIFI_FW_DOWNLOAD_INVALID_PARAM,
	WIFI_FW_DOWNLOAD_INVALID_CRC,
	WIFI_FW_DOWNLOAD_DECRYPTION_FAIL,
	WIFI_FW_DOWNLOAD_UNKNOWN_CMD,
	WIFI_FW_DOWNLOAD_TIMEOUT,
};

struct _INIT_CMD_ACCESS_REG {
	UINT8 ucSetQuery;
	UINT8 aucReserved[3];
	UINT32 u4Address;
	UINT32 u4Data;
};

#define CMD_CH_PRIV_ACTION_REQ 0
#define CMD_CH_PRIV_ACTION_ABORT 1
#define CMD_CH_PRIV_ACTION_BW_REQ 2

#define CMD_CH_PRIV_SCO_SCN 0
#define CMD_CH_PRIV_SCO_SCA 1
#define CMD_CH_PRIV_SCO_SCB 3

#define CMD_CH_PRIV_BAND_G 1
#define CMD_CH_PRIV_BAND_A 2

#define CMD_CH_PRIV_CH_WIDTH_20_40 0
#define CMD_CH_PRIV_CH_WIDTH_80	   1
#define CMD_CH_PRIV_CH_WIDTH_160   2
#define CMD_CH_PRIV_CH_WIDTH_80_80 3

#define CMD_CH_PRIV_REQ_JOIN 0
#define CMD_CH_PRIV_REQ_P2P_LISTEN 1

typedef struct GNU_PACKED _CMD_SEC_ADDREMOVE_KEY_STRUC_T {
	UINT8		ucAddRemove;
	UINT8		ucTxKey;
	UINT8		ucKeyType;
	UINT8		ucIsAuthenticator;
	UINT8		aucPeerAddr[6];
	UINT8		ucBssIndex;
	UINT8		ucAlgorithmId;
	UINT8		ucKeyId;
	UINT8		ucKeyLen;
	UINT8		ucWlanIndex;
	UINT8		ucReverved;
	UINT8		aucKeyRsc[16];
	UINT8		aucKeyMaterial[32];
} CMD_SEC_ADDREMOVE_KEY_STRUC_T, *P_CMD_ADDREMOVE_KEY_STRUC_T;

typedef struct GNU_PACKED _EVENT_SEC_ADDREMOVE_STRUC_T {
	UINT32		u4WlanIdx;
	UINT32		u4Status;
	UINT32		u4Resv;
} EVENT_SEC_ADDREMOVE_STRUC_T, *P_EVENT_SEC_ADDREMOVE_STRUC_T;

typedef struct GNU_PACKED _EXT_CMD_AP_PWS_START_T {
	UINT32 u4WlanIdx;
	UINT32 u4Resv;
	UINT32 u4Resv2;
} EXT_CMD_AP_PWS_START_T, *P_EXT_CMD_AP_PWS_START_T;

typedef struct GNU_PACKED _CMD_AP_PS_CLEAR_STRUC_T {
	UINT32		u4WlanIdx;
	UINT32		u4Status;
	UINT32		u4Resv;
} CMD_AP_PS_CLEAR_STRUC_T, *P_CMD_AP_PS_CLEAR_STRUC_T;

typedef struct GNU_PACKED _CMD_CH_PRIVILEGE_T {
	UINT8      ucBssIndex;
	UINT8      ucTokenID;
	UINT8      ucAction;
	UINT8      ucPrimaryChannel;
	UINT8      ucRfSco;
	UINT8      ucRfBand;
	UINT8      ucRfChannelWidth;   /* To support 80/160MHz bandwidth */
	UINT8      ucRfCenterFreqSeg1; /* To support 80/160MHz bandwidth */
	UINT8      ucRfCenterFreqSeg2; /* To support 80/160MHz bandwidth */
	UINT8      ucReqType;
	UINT8      aucReserved[2];
	UINT32     u4MaxInterval;      /* In unit of ms */
} CMD_CH_PRIVILEGE_T, *P_CMD_CH_PRIVILEGE_T;

typedef struct GNU_PACKED _CMD_RF_REG_ACCESS_T {
	UINT32 WiFiStream;
	UINT32 Address;
	UINT32 Data;
} CMD_RF_REG_ACCESS_T;

typedef struct GNU_PACKED _CMD_ACCESS_REG_T {
	UINT32 u4Address;
	UINT32 u4Data;
} CMD_ACCESS_REG_T;

/* test CR4 */
typedef struct GNU_PACKED _EXT_CMD_CR4_QUERY_T {
	UINT32 u4Cr4QueryOptionArg0;
	UINT32 u4Cr4QueryOptionArg1;
	UINT32 u4Cr4QueryOptionArg2;
} EXT_CMD_CR4_QUERY_T, *P_EXT_CMD_CR4_QUERY_T;

typedef struct GNU_PACKED _EXT_CMD_CR4_MULTI_QUERY_T {
	UINT32 u4Cr4QueryOptionArg0;
	UINT32 u4Cr4QueryOptionArg1;
	UINT32 u4Cr4QueryOptionArg2;
	UINT16 u4Cr4QueryList[0];
} EXT_CMD_CR4_MULTI_QUERY_T, *P_EXT_CMD_CR4_MULTI_QUERY_T;

typedef struct GNU_PACKED _EXT_CMD_CR4_SET_T {
	UINT32 u4Cr4SetArg0;
	UINT32 u4Cr4SetArg1;
	UINT32 u4Cr4SetArg2;
} EXT_CMD_CR4_SET_T, *P_EXT_CMD_CR4_SET_T;

typedef struct GNU_PACKED _EXT_CMD_CR4_CAPABILITY_T {
	UINT32 u4Cr4Capability;
} EXT_CMD_CR4_CAPABILITY_T, *P_EXT_CMD_CR4_CAPABILITY_T;

typedef struct GNU_PACKED _EXT_CMD_CR4_DEBUG_T {
	UINT32 u4Cr4Debug;
} EXT_CMD_CR4_DEBUG_T, *P_EXT_CMD_CR4_DEBUG_T;

/* WOCPU */
typedef struct GNU_PACKED _EXT_CMD_WO_QUERY_T {
	UINT32 query_arg0;
	UINT32 query_arg1;
} EXT_CMD_WO_QUERY_T, *P_EXT_CMD_WO_QUERY_T;

typedef struct GNU_PACKED _EXT_CMD_ID_LED {
	UINT32 u4LedNo;
	UINT32 u4LedCtrl;
} EXT_CMD_ID_LED_T, *PEXT_CMD_ID_LED_T;

typedef struct GNU_PACKED _EVENT_WIFI_RDD_TEST_T {
	UINT32 u4FuncIndex;
	UINT32 u4FuncLength;
	UINT32 u4Prefix;
	UINT32 u4Count;
	UINT8 ucRddIdx;
	UINT8 aucReserve[3];
	UINT8 aucBuffer[0];
} EVENT_WIFI_RDD_TEST_T, *PEVENT_WIFI_RDD_TEST_T;

#define WIFI_RADIO_ON 1
#define WIFI_RADIO_OFF 2
typedef struct GNU_PACKED _EXT_CMD_RADIO_ON_OFF_CTRL_T {
	UINT8 ucWiFiRadioCtrl;
	UINT8 aucReserve[3];
} EXT_CMD_RADIO_ON_OFF_CTRL_T;

#if defined(MT_MAC)
typedef struct GNU_PACKED _EXT_CMD_TXBf_APCLIENT_CLUSTER_T {
	UINT8  ucPfmuProfileFormatId;
	UINT8  ucWlanIdxHnVer;	/* #256STA - High Byte and Version */
	UINT8  ucWlanIdxL;		/* #256STA - Low Byte  */
	UINT8  ucCmmWlanId;
} EXT_CMD_TXBf_APCLIENT_CLUSTER_T, *P_EXT_CMD_TXBf_APCLIENT_CLUSTER_T;

typedef struct GNU_PACKED _EXT_CMD_REPT_CLONED_STA_BF_T {
	UINT8  ucCmdCategoryID;
	UINT8  ucWlanIdxHnVer;	/* #256STA - High Byte and Version */
	UINT8  ucWlanIdxL;		/* #256STA - Low Byte  */
	UINT8  ucCliIdx;
} EXT_CMD_REPT_CLONED_STA_BF_T, *P_EXT_CMD_REPT_CLONED_STA_BF_T;

typedef struct GNU_PACKED _EXT_CMD_TXBf_BFEE_CTRL_T {
	UINT8   ucCmdCategoryID;
	BOOLEAN fgBFeeNullPktFeedbackEn;
	UINT8   ucReserved[2];
} EXT_CMD_TXBf_BFEE_CTRL_T, *P_EXT_CMD_TXBf_BFEE_CTRL_T;

typedef struct GNU_PACKED _EXT_CMD_ITXBf_PHASE_CAL_CTRL_T {
	UINT8   ucCmdCategoryID;
	UINT8   ucGroup_L_M_H;
	UINT8   ucGroup;
	BOOLEAN fgSX2;
	UINT8   ucPhaseCalType;
	UINT8   ucPhaseVerifyLnaGainLevel;
	UINT8   ucReserved[2];
} EXT_CMD_ITXBf_PHASE_CAL_CTRL_T, *P_EXT_CMD_ITXBf_PHASE_CAL_CTRL_T;

typedef struct GNU_PACKED _EXT_CMD_ITXBf_PHASE_COMP_CTRL_T {
	UINT8   ucCmdCategoryID;
	UINT8   ucWlanIdxL;		/* #256STA - Low Byte  */
	UINT8   ucBW;
	UINT8   ucBand;
	UINT8   ucDbdcBandIdx;
	BOOLEAN fgRdFromE2p;
	BOOLEAN fgDisComp;
	UINT8   ucWlanIdxHnVer;	/* #256STA - High Byte and Version */
	UINT8   aucBuf[40];
} EXT_CMD_ITXBf_PHASE_COMP_CTRL_T, *P_EXT_CMD_ITXBf_PHASE_COMP_CTRL_T;

typedef struct _EXT_CMD_PEER_AID_T {
	UINT8  u1CmdCategoryID;
	UINT8  u1WlanIdxL;		/* #256STA - Low Byte  */
	UINT16 u2Aid;
	UINT8  u1OwnMacIdx;
	UINT8  u1WlanIdxHnVer;	/* #256STA - High Byte and Version */
	UINT8  u1BandIdx;
	UINT8  u1Reserved;
} EXT_CMD_PEER_AID_T, *P_EXT_CMD_PEER_AID_T;

typedef struct GNU_PACKED _EXT_CMD_BF_CONFIG_T {
	UINT8  cmd_category_id;
	UINT8  config_type;
	UINT8  config_para[6];
} EXT_CMD_BF_CONFIG_T, *P_EXT_CMD_BF_CONFIG_T;

typedef struct GNU_PACKED _EXT_CMD_TXBf_TX_APPLY_CTRL_T {
	UINT8   ucCmdCategoryID;
	UINT8   ucWlanIdxL;		/* #256STA - Low Byte  */
	BOOLEAN fgETxBf;
	BOOLEAN fgITxBf;
	BOOLEAN fgMuTxBf;
	BOOLEAN fgPhaseCali;
	UINT8   ucWlanIdxHnVer;	/* #256STA - High Byte and Version */
	UINT8   ucReserved;
} EXT_CMD_TXBf_TX_APPLY_CTRL_T, *P_EXT_CMD_TXBf_TX_APPLY_CTRL_T;

typedef struct GNU_PACKED _EXT_CMD_ETXBf_SND_PERIODIC_TRIGGER_CTRL_T {
	UINT8   ucCmdCategoryID;
	UINT8   ucSuMuSndMode;
    UINT8   ucStaNum;
    UINT8   ucReserved;
    UINT8   ucWlanIdx[4];
	UINT32  u4SoundingInterval;     /* By ms */
} EXT_CMD_ETXBf_SND_PERIODIC_TRIGGER_CTRL_T, *P_EXT_CMD_ETXBf_SND_PERIODIC_TRIGGER_CTRL_T;

typedef struct GNU_PACKED _EXT_CMD_ETXBf_MU_SND_PERIODIC_TRIGGER_CTRL_T {
	UINT8   ucCmdCategoryID;
	UINT8   ucSuMuSndMode;
	UINT8   ucStaNum;
	UINT8   ucReserved;
	UINT8   ucWlanIdx[4];
	UINT32  u4SoundingInterval;     /* By ms */
} EXT_CMD_ETXBf_MU_SND_PERIODIC_TRIGGER_CTRL_T, *P_EXT_CMD_ETXBf_MU_SND_PERIODIC_TRIGGER_CTRL_T;

typedef struct GNU_PACKED _EXT_CMD_BF_TX_PWR_BACK_OFF_T {
	UINT8  ucCmdCategoryID;
	UINT8  ucBandIdx;
	UINT8  aucReserved1[2];
	INT8   acTxPwrFccBfOnCase[10];
	UINT8  aucReserved2[2];
	INT8   acTxPwrFccBfOffCase[10];
	UINT8  aucReserved3[2];
} EXT_CMD_BF_TX_PWR_BACK_OFF_T, *P_EXT_CMD_BF_TX_PWR_BACK_OFF_T;

typedef struct GNU_PACKED _EXT_CMD_BF_AWARE_CTRL_T {
	UINT8    ucCmdCategoryID;
	BOOLEAN  fgBfAwareCtrl;
	UINT8    aucReserved[2];
} EXT_CMD_BF_AWARE_CTRL_T, *P_EXT_CMD_BF_AWARE_CTRL_T;

struct dynsnd_en_intr_info {
	UINT8  category_id;
	BOOLEAN is_intr_en;
	UINT16 reserved;
};

#ifdef CFG_SUPPORT_MU_MIMO
struct dynsnd_cfg_dmcsth_info {
	UINT8 category_id;
	UINT8 mcs_index;
	UINT8 mcs_th;
	UINT8 reserved;
};

struct dynsnd_en_mu_intr_info {
	UINT8 category_id;
	BOOLEAN mu_intr_en;
	UINT8 pfid;
	UINT8 reserved;
};
#endif

typedef struct GNU_PACKED _EXT_CMD_BFEE_HW_CTRL_T
{
    UINT8    ucCmdCategoryID;
    BOOLEAN  fgBfeeHwCtrl;
    UINT8    aucReserved[2];
} EXT_CMD_BFEE_HW_CTRL_T, *P_EXT_CMD_BFEE_HW_CTRL_T;

typedef struct GNU_PACKED _EXT_CMD_BF_HW_ENABLE_STATUS_UPDATE_T {
	UINT8    ucCmdCategoryID;
	BOOLEAN  fgEBfHwEnStatus;
	BOOLEAN  fgIBfHwEnStatus;
	UINT8    ucReserved;
} EXT_CMD_BF_HW_ENABLE_STATUS_UPDATE_T, *P_EXT_CMD_BF_HW_ENABLE_STATUS_UPDATE_T;

typedef struct GNU_PACKED _EXT_CMD_BF_MOD_EN_CTRL_T {
	UINT8  u1CmdCategoryID;
	UINT8  u1BfNum;
	UINT8  u1BfBitmap;
	UINT8  au1BFSel[8];
	UINT8  au1Reserved[5];
} EXT_CMD_BF_MOD_EN_CTRL_T, *P_EXT_CMD_BF_MOD_EN_CTRL_T;

typedef struct GNU_PACKED _EXT_CMD_ETXBf_PFMU_PROFILE_TAG_R_T {
	UINT8  ucPfmuProfileFormatId;
	UINT8  ucPfmuIdx;
	UINT8  fgBFer;
	UINT8  ucBandIdx;
} EXT_CMD_ETXBf_PFMU_PROFILE_TAG_R_T, *P_EXT_CMD_ETXBf_PFMU_PROFILE_TAG_R_T;

typedef struct GNU_PACKED _EXT_CMD_ETXBf_PFMU_PROFILE_TAG_W_T {
	UINT8  ucPfmuProfileFormatId;
	UINT8  ucPfmuIdx;
	BOOLEAN fgBFer;
	UINT8  ucBandIdx;
	UINT8  ucBuf[64];
} EXT_CMD_ETXBf_PFMU_PROFILE_TAG_W_T, *P_EXT_CMD_ETXBf_PFMU_PROFILE_TAG_W_T;

typedef struct GNU_PACKED _EXT_CMD_ETXBf_PFMU_PROFILE_DATA_R_T {
	UINT8   ucPfmuProfileFormatId;
	UINT8   ucPfmuIdx;
	BOOLEAN fgBFer;
	UINT8   ucBandIdx;
	UINT8   ucReserved[2];
	UINT16  u2SubCarrIdx;
} EXT_CMD_ETXBf_PFMU_PROFILE_DATA_R_T, *P_EXT_CMD_ETXBf_PFMU_PROFILE_DATA_R_T;

typedef struct GNU_PACKED _EXT_CMD_ETXBf_PFMU_PROFILE_DATA_W_T {
	UINT8      ucPfmuProfileFormatId;
	UINT8      ucPfmuIdx;
	UINT16     u2SubCarr;
	UINT8      ucBuf[16];	/* [11:0] Phi11 ~ Psi43, [15:12] sSNR0 ~ dSNR3 */
	UINT8      ucReserved[3];
	UINT8      ucBandIdx;
} EXT_CMD_ETXBf_PFMU_PROFILE_DATA_W_T, *P_EXT_CMD_ETXBf_PFMU_PROFILE_DATA_W_T;

typedef struct GNU_PACKED _EXT_CMD_ETXBf_PFMU_FULL_DIM_DATA_W_T {
	UINT8   u1PfmuProfileFormatId;
	UINT8   u1PfmuIdx;
	UINT16  u2SubCarr;
	UINT8   u1BandIdx;
	BOOLEAN fgBfer;
	UINT8   u1Reserved[2];
	UCHAR   aucBuf[256];
} EXT_CMD_ETXBf_PFMU_FULL_DIM_DATA_W_T, *P_EXT_CMD_ETXBf_PFMU_FULL_DIM_DATA_W_T;

typedef struct GNU_PACKED _EXT_CMD_ETXBf_PFMU_PROFILE_DATA_W_20M_ALL_T {
	UINT8      ucPfmuProfileFormatId;
	UINT8      ucPfmuIdx;
	UINT8      ucBandIdx;
	UINT8      aucReserved[1];
	UINT8      ucBuf[512];
} EXT_CMD_ETXBf_PFMU_PROFILE_DATA_W_20M_ALL_T, *P_EXT_CMD_ETXBf_PFMU_PROFILE_DATA_W_20M_ALL_T;

typedef struct GNU_PACKED _EXT_CMD_ETXBf_PFMU_PROFILE_PN_R_T {
	UINT8  ucPfmuProfileFormatId;
	UINT8  ucPfmuIdx;
	UINT8  ucBandIdx;
	UINT8  ucReserved[1];
} EXT_CMD_ETXBf_PFMU_PROFILE_PN_R_T, *P_EXT_CMD_ETXBf_PFMU_PROFILE_PN_R_T;

typedef struct GNU_PACKED _EXT_CMD_ETXBf_PFMU_PROFILE_PN_W_T {
	UINT8  ucPfmuProfileFormatId;
	UINT8  ucPfmuIdx;
	UINT8  ucBW;
	UINT8  ucBandIdx;
	UINT8  ucBuf[32];
} EXT_CMD_ETXBf_PFMU_PROFILE_PN_W_T, *P_EXT_CMD_ETXBf_PFMU_PROFILE_PN_W_T;

typedef struct GNU_PACKED _EXT_CMD_TXBf_QD_R_T
{
	UINT8   ucCmdCategoryID;
	INT8	cSubCarr;
	UINT8   ucBandIdx;
	UINT8   ucReserved[1];
} EXT_CMD_TXBf_QD_R_T, *P_EXT_CMD_TXBf_QD_R_T;

typedef enum _BF_SND_FB_RPT_DBG_INFO_ACTION {
	BF_READ_AND_CLEAR_FBK_STAT_INFO = 0,
	BF_READ_FBK_STAT_INFO,
	BF_SET_POLL_PFMU_INTR_STAT_TIMEOUT,
	BF_SET_PFMU_DEQ_INTERVAL,
	BF_DYNAMIC_PFMU_UPDATE
} BF_SND_FB_RPT_DBG_INFO_ACTION;

typedef struct GNU_PACKED _EXT_CMD_TXBF_FBRPT_DBG_INFO_T {
	UINT8 u1CmdCategoryID;
	UINT8 u1Action;
	UINT16 u2WlanIdx;
	UINT8 u1BandIdx;
	UINT8 u1PollPFMUIntrStatTimeOut;
	UINT8 u1FbRptDeQInterval;
	UINT8 u1PFMUUpdateEn;
	UINT8 u1Reserved[8];
} EXT_CMD_TXBF_FBRPT_DBG_INFO_T, *P_EXT_CMD_TXBF_FBRPT_DBG_INFO_T;

typedef enum _TXBF_SND_CMD_ACTION {
	BF_SND_READ_INFO = 0,
	BF_SND_CFG_OPT,
	BF_SND_CFG_INTV,
	BF_SND_STA_STOP,
	BF_SND_CFG_MAX_STA,
	BF_SND_CFG_BFRP,
	BF_SND_CFG_INF
} TXBF_SND_CMD_ACTION;

typedef enum _TXBF_PLY_CMD_ACTION {
	BF_PLY_READ_INFO = 0,
	BF_PLY_CFG_OPT,
	BF_PLY_CFG_STA_PLY
} TXBF_PLY_CMD_ACTION;

typedef enum _TXBF_TXCMD_CMD_ACTION {
	BF_TXCMD_READ_INFO = 0,
	BF_TXCMD_BF_CFG
} TXBF_TXCMD_CMD_ACTION;

typedef enum _TXBF_SND_CNT_CMD_ACTION {
	BF_SND_CNT_READ = 0,
	BF_SND_CNT_SET_LMT_MAN
} TXBF_SND_CNT_CMD_ACTION;

typedef enum _TXBF_CFG_BF_PHY {
	BF_PHY_SMTH_INTL_BYPASS = 0,
} TXBF_CFG_BF_PHY;

typedef enum _HERA_METRIC_CMD_ACTION {
	HERA_METRIC_READ_INFO = 0,
	HERA_METRIC_START_CALC,
	HERA_METRIC_CHANGE_POLLING_TIME
} HERA_METRIC_CMD_ACTION;

typedef struct GNU_PACKED _EXT_CMD_TXBF_SND_CMD_T {
	UINT8 ucCmdCategoryID;
	UINT8 ucAction;
	UINT8 ucReadClr;
	UINT8 ucVhtOpt;
	UINT8 ucHeOpt;
	UINT8 ucGloOpt;
	UINT16 u2WlanIdx;
	UINT8 ucSndIntv;
	UINT8 ucSndStop;
	UINT8 ucMaxSndStas;
	UINT8 ucTxTime;
	UINT8 ucMcs;
	BOOLEAN fgLDPC;
	UINT8 ucInf;
	UINT8 ucReserved;
} EXT_CMD_TXBF_SND_CMD_T, *P_EXT_CMD_TXBF_SND_CMD_T;

typedef struct GNU_PACKED _EXT_CMD_TXBF_PLY_CMD_T {
	UINT8 ucCmdCategoryID;
	UINT8 ucAction;
	UINT16 u2WlanIdx;
	UINT8 ucGloOpt;
	UINT8 ucGrpIBfOpt;
	UINT8 ucGrpEBfOpt;
	UINT8 ucNss;
	UINT8 ucSSPly;
	UINT8 ucReserved[7];
} EXT_CMD_TXBF_PLY_CMD_T, *P_EXT_CMD_TXBF_PLY_CMD_T;

typedef struct GNU_PACKED _EXT_CMD_TXBF_TXCMD_CMD_T {
	UINT8 ucCmdCategoryID;
	UINT8 ucAction;
	BOOLEAN fgTxCmdBfManual;
	UINT8 ucTxCmdBfBit;
	UINT8 ucReserved[4];
} EXT_CMD_TXBF_TXCMD_CMD_T, *P_EXT_CMD_TXBF_TXCMD_CMD_T;

typedef struct GNU_PACKED _EXT_CMD_TXBF_SND_CNT_CMD_T {
	UINT8 ucCmdCategoryID;
	UINT8 u1Action;
	UINT16 u2SndCntLmtMan;
	UINT8 u1Reserved[4];
} EXT_CMD_TXBF_SND_CNT_CMD_T, *P_EXT_CMD_TXBF_SND_CNT_CMD_T;

typedef struct GNU_PACKED _EXT_CMD_TXBF_CFG_BF_PHY_T {
	UINT8 ucCmdCategoryID;
	UINT8 ucAction;
	UINT8 ucBandIdx;
	UINT8 ucSmthIntlBypass;
	UINT8 ucReserved[12];
} EXT_CMD_TXBF_CFG_BF_PHY_T, *P_EXT_CMD_TXBF_CFG_BF_PHY_T;

typedef struct GNU_PACKED _HERA_MU_METRIC_CMD_T
{
	UINT8 u1CmdCategoryID;
	UINT8 u1Action;
	UINT8 u1ReadClr;
	UINT8 u1Band;
	UINT8 u1NUser;
	UINT8 u1DBW;
	UINT8 u1NTxer;
	UINT8 u1PFD;
	UINT8 u1RuSize;
	UINT8 u1RuIdx;
	UINT8 u1SpeIdx;
	UINT8 u1SpeedUp;
	UINT8 u1LDPC;
	UINT8 u1PollingTime;
	UINT8 reserved[2];
	UINT8 u1NStsUser[4];
	UINT16 u2PfidUser[4];
} HERA_MU_METRIC_CMD_T, *P_HERA_MU_METRIC_CMD_T;

typedef struct GNU_PACKED _EXT_EVENT_BF_STATUS_T {
	UINT8   ucBfDataFormatID;
	UINT8   ucBw;
	UINT16  u2subCarrIdx;
	BOOLEAN fgBFer;
	UINT8   aucReserved[3];
	UINT8	aucBuffer[1000]; /* temparary size */
} EXT_EVENT_BF_STATUS_T, *P_EXT_EVENT_BF_STATUS_T;

typedef struct GNU_PACKED _EXT_EVENT_IBF_STATUS_T {
	UINT8   ucBfDataFormatID;
	UINT8   ucGroup_L_M_H;
	UINT8   ucGroup;
	BOOLEAN fgSX2;
	UINT8   ucStatus;
	UINT8   ucPhaseCalType;
	UINT8   aucReserved[2];
	UINT8	aucBuffer[1000]; /* temparary size */
} EXT_EVENT_IBF_STATUS_T, *P_EXT_EVENT_IBF_STATUS_T;

typedef enum _BF_EVENT_CATEGORY {
	BF_PFMU_TAG = 0x10,
	BF_PFMU_DATA,
	BF_PFMU_PN,
	BF_PFMU_MEM_ALLOC_MAP,
	BF_STAREC,
	BF_CAL_PHASE,
	BF_QD_DATA,
	BF_FBRPT_DBG_INFO,
	BF_EVT_TXSND_INFO,
	BF_EVT_PLY_INFO,
	BF_EVT_METRIC_INFO,
	BF_EVT_TXCMD_CFG_INFO,
	BF_EVT_SND_CNT_INFO
} BF_EVENT_CATEGORY;

typedef struct _EXT_CMD_ETXBF_PFMU_SW_TAG_T {
	UINT8 ucPfmuProfileFormatId;
	UINT8 ucLm;
	UINT8 ucNr;
	UINT8 ucNc;
	UINT8 ucBw;
	UINT8 ucCodebook;
	UINT8 ucgroup;
	UINT8 ucTxBf;
	UINT8 ucReserved[1];
} EXT_CMD_ETXBF_PFMU_SW_TAG_T, *PEXT_CMD_ETXBF_PFMU_SW_TAG_T;
#endif/* MT_MAC */

typedef enum _MEC_CTRL_AMSDU_THR_LEN {
	MEC_CTRL_AMSDU_THR_1_DOT_7KB = 0,
	MEC_CTRL_AMSDU_THR_3_DOT_3KB,
	MEC_CTRL_AMSDU_THR_4_DOT_8KB,
	MEC_CTRL_AMSDU_THR_MAX
} MEC_CTRL_AMSDU_THR_LEN;

typedef enum _MEC_CTRL_BA_TYPE_NUM {
	MEC_CTRL_BA_NUM_64 = 0,
	MEC_CTRL_BA_NUM_256,
	MEC_CTRL_BA_NUM_MAX
} MEC_CTRL_BA_TYPE_NUM;

typedef enum _MEC_CTRL_CMD_ACTION {
	MEC_CTRL_ACTION_READ_INFO = 0,
	MEC_CTRL_ACTION_AMSDU_ALGO_EN_STA,
	MEC_CTRL_ACTION_AMSDU_PARA_STA,
	MEC_CTRL_ACTION_AMSDU_ALGO_THRESHOLD,
	MEC_CTRL_INTF_SPEED,
	MEC_CTRL_ACTION_MAX = 8
} MEC_CTRL_CMD_ACTION;

typedef enum _MEC_CTRL_READ_TYPE {
	MEC_EVENT_READ_TYPE_ALL = 0,
	MEC_EVENT_READ_TYPE_ALGO_EN = BIT(0),
	MEC_EVENT_READ_TYPE_AMSDU_THR = BIT(1),
	MEC_EVENT_READ_TYPE_AMSDU_ERROR_NOTIFY = BIT(2),
} MEC_CTRL_READ_TYPE;

typedef struct GNU_PACKED _MEC_CTRL_T {
	UINT32 au4MecAlgoEnSta[MAX_LEN_OF_MAC_TABLE / 32];
	UINT16 au2MecAmsduThr[MEC_CTRL_BA_NUM_MAX][MEC_CTRL_AMSDU_THR_MAX];
} MEC_CTRL_T, *P_MEC_CTRL_T;

typedef struct GNU_PACKED _MEC_INFO_EVENT_T {
	MEC_CTRL_T rMecCtrl;
} MEC_INFO_EVENT_T, *P_MEC_INFO_EVENT_T;

typedef struct GNU_PACKED _EXT_EVENT_MEC_INFO_T {
	UINT16  u2ReadType;
    UINT16  u2wlanID;
	UINT32  au4Buf[MAX_LEN_OF_MAC_TABLE];
} EXT_EVENT_MEC_INFO_T, *P_EXT_EVENT_MEC_INFO_T;

typedef struct GNU_PACKED _MEC_EVENT_INFO_T {
	MEC_CTRL_T rMecCtrl;
} MEC_EVENT_INFO_T, *P_MEC_EVENT_INFO_T;

typedef struct GNU_PACKED _MEC_READ_INFO_T {
	UINT16 u2ReadType;
	UINT8 u1Reserved[6];
} CMD_MEC_READ_INFO_T, *P_CMD_MEC_READ_INFO_T;

typedef struct GNU_PACKED _MEC_ALGO_EN_STA_T {
	UINT16 u2WlanIdx;
	UINT8 u1AmsduAlgoEn;
	UINT8 u1Reserved[5];
} CMD_MEC_ALGO_EN_STA_T, *P_CMD_MEC_ALGO_EN_STA_T;

typedef struct GNU_PACKED _MEC_AMSDU_PARA_STA_T {
	UINT16 u2WlanIdx;
	UINT8 u1AmsduEn;
	UINT8 u1AmsduNum;
	UINT16 u2AmsduLen;
	UINT8 u1Reserved[2];
} CMD_MEC_AMSDU_PARA_STA_T, *P_CMD_MEC_AMSDU_PARA_STA_T;

typedef struct GNU_PACKED _MEC_AMSDU_ALGO_THR_T {
	UINT8 u1BaNum;
	UINT8 u1AmsduNum;
	UINT16 u2AmsduRateThr;
	UINT8 u1Reserved[4];
} CMD_MEC_AMSDU_ALGO_THR_T, *P_CMD_MEC_AMSDU_ALGO_THR_T;

struct GNU_PACKED MEC_INTF_SPEED_T {
	UINT32 u4InterfacSpeed;
	UINT8 u1Reserved[4];
};

typedef struct GNU_PACKED _CMD_MEC_CTRL_CMD_T {
	UINT16 u2Action;
	UINT8 u1Reserved[2];
	union MEC_CMD_PARA {
		CMD_MEC_READ_INFO_T mec_read_info_t;
		CMD_MEC_ALGO_EN_STA_T mec_algo_en_sta_t;
		CMD_MEC_AMSDU_PARA_STA_T mec_amsdu_para_sta_t;
		CMD_MEC_AMSDU_ALGO_THR_T mec_amsdu_algo_thr;
		struct MEC_INTF_SPEED_T mec_ifac_speed;
	} mecCmdPara;
} CMD_MEC_CTRL_CMD_T, *P_CMD_MEC_CTRL_CMD_T;

/* EXT_CMD_RF_TEST */
/* ACTION */
#define ACTION_SWITCH_TO_RFTEST 0 /* to switch firmware mode between normal mode or rf test mode */
#define ACTION_IN_RFTEST        1
/* OPMODE */
#define OPERATION_NORMAL_MODE     0
#define OPERATION_RFTEST_MODE     1
#define OPERATION_ICAP_MODE       2
#define OPERATION_ICAP_OVERLAP	  3
#define OPERATION_WIFI_SPECTRUM   4

/* FuncIndex */
typedef enum {
	RE_CALIBRATION = 0x01,
	CALIBRATION_BYPASS = 0x02,
	TX_TONE_START = 0x03,
	TX_TONE_STOP = 0x04,
	CONTINUOUS_TX_START = 0x05,
	CONTINUOUS_TX_STOP = 0x06,
	RF_AT_EXT_FUNCID_TX_TONE_RF_GAIN = 0x07,
	RF_AT_EXT_FUNCID_TX_TONE_DIGITAL_GAIN = 0x08,
	CAL_RESULT_DUMP_FLAG = 0x09,
	RDD_TEST_MODE  = 0x0A,
	SET_ICAP_CAPTURE_START = 0x0B,
	GET_ICAP_CAPTURE_STATUS = 0x0C,
	SET_ADC = 0x0D,
	SET_RX_GAIN = 0x0E,
	SET_TTG = 0x0F,
	TTG_ON_OFF = 0x10,
	GET_ICAP_RAW_DATA = 0x11,
	SET_TX_TONE_GAIN_OFFSET = 0x12,
	GET_TX_TONE_GAIN_OFFSET = 0x13,
	DO_RX_GAIN_CAL = 0x14,
	RF_AT_EXT_FUNCID_GET_PHY_ICS_DATA = 0x15
} FUNC_IDX;

#define RF_TEST_DEFAULT_RESP_LEN		8	/* sizeof(struct _EVENT_EXT_CMD_RESULT_T) */
#define	RC_CAL_RESP_LEN					112
#define RX_RSSI_DCOC_CAL_RESP_LEN		304
#define	RX_DCOC_CAL_RESP_LEN			816	/* Total 4 event 808 *4 */
#define TX_DPD_RX_FI_FD_MPM_RESP_LEN	8	/* MT7615 not support */
#define TX_DPD_SCAN_HPM_RESP_LEN		8	/* MT7615 not support */
#define RX_FIIQ_CAL_RESP_LEN		208
#define RX_FDIQ_CAL_RESP_LEN		416		/* Total 408*6 */
#define TX_DPD_LINK_RESP_LEN		272		/* Total 264*6 */
#define TX_LPFG_RESP_LEN			64
#define TX_DCIQ_RESP_LEN			592
#define TX_IQM_RESP_LEN				112
#define TX_PGA_RESP_LEN				112
#define CAL_ALL_LEN				8712
#define RF_TEST_ICAP_LEN			120

/* Cal Items */
typedef enum {
	RC_CAL              = 0x00000001, /* bit  0 */
	RX_RSSI_DCOC_CAL    = 0x00000002, /* bit  1 */
	RX_DCOC_CAL         = 0x00000004, /* bit  2 */
	TX_DPD_RX_FI_FD_MPM = 0x00000008, /* bit  3 */
	TX_DPD_SCAN_HPM     = 0x00000010, /* bit  4 */
	RX_FIIQ_CAL         = 0x00000020, /* bit  5 */
	RX_FDIQ_CAL         = 0x00000040, /* bit  6 */
	TX_DPD_LINK         = 0x00000080, /* bit  7 */
	TX_LPFG             = 0x00000100, /* bit  8 */
	TX_DCIQC            = 0x00000200, /* bit  9 */
	TX_IQM              = 0x00000400, /* bit 10 */
	TX_PGA              = 0x00000800, /* bit 11 */
	RX_GAIN_CAL         = 0x01000000, /* bit 24 */
	TX_TSSI_CAL_2G      = 0x02000000, /* bit 25 */
	TX_TSSI_CAL_5G      = 0x04000000, /* bit 26 */
	TX_DNL_CAL          = 0x08000000, /* bit 27 */
	TX_DPD_FLATNESS_CAL = 0x10000000, /* bit 28 */
	TX_DPD_FLATNESS_CAL_A5 = 0x30000000, /* use bit 28 and bit 29 represent PerCH 5G Cal*/
	TX_DPD_FLATNESS_CAL_A6 = 0x50000000, /* use bit 28 and bit 30 represent PerCH 6G Cal */
	PRE_CAL             = 0x20000000, /* bit 29 */
	CAL_ALL             = 0x80000000, /* bit 31 */
} CAL_ITEM_IDX;

typedef enum {
	CAL_SPEC_ID1 = 0x00000001,
	CAL_SPEC_ID2 = 0x00000002,
	CAL_SPEC_ID3 = 0x00000004,
	CAL_SPEC_ID4 = 0x00000008,
	CAL_SPEC_ID5 = 0x00000010,
	CAL_SPEC_ID6 = 0x00000020,
	CAL_SPEC_ID7 = 0x00000040,
	CAL_SPEC_ID8 = 0x00000080,
	CAL_SPEC_ID9 = 0x00000100,
	CAL_SPEC_ID10 = 0x00000200,
	CAL_SPEC_ID11 = 0x00000400,
	CAL_SPEC_ID12 = 0x00000800,
	/* reserved */
	CAL_ID_NUM = 0x80000000
} SPECIFIC_CAL_ID_T;

/*u4CalDump*/
typedef enum {
	DISABLE_DUMP = 0x0,
	ENABLE_DUMP = 0x1
} TEST_CAL_DUMP;

typedef struct _TX_TONE_PARAM_T {
	UINT8 ucAntIndex;
	UINT8 ucToneType;
	UINT8 ucToneFreq;
	UINT8 ucDbdcIdx;
	INT32 i4DcOffsetI;
	INT32 i4DcOffsetQ;
	UINT32 u4Band;
} TX_TONE_PARAM_T, *PTX_TONE_PARAM_T;

typedef struct GNU_PACKED _EXT_CMD_RDD_ON_OFF_CTRL_T { /* Jelly20150211 */
	UINT8 ucDfsCtrl;
	UINT8 ucRddIdx;
	UINT8 ucRddRxSel;
	UINT8 ucSetVal;
	UINT8 aucReserved[4];
} EXT_CMD_RDD_ON_OFF_CTRL_T, *P_EXT_CMD_RDD_ON_OFF_CTRL_T;

#if OFF_CH_SCAN_SUPPORT
typedef struct _EXT_CMD_OFF_CH_SCAN_CTRL_T {
	UINT8 work_prim_ch;     /* Working primary channel */
	UINT8 work_cntrl_ch;    /* Working central channel */
	UINT8 work_bw;          /* Bandwidth of working channel */
	UINT8 work_tx_strm_pth; /* Working TX stream path */

	UINT8 work_rx_strm_pth; /* Working RX stream path */
	UINT8 mntr_prim_ch;     /* Monitoring primary channel */
	UINT8 mntr_cntrl_ch;    /* Monitoring central channel */
	UINT8 mntr_bw;          /* Bandwidth of monitoring channel */

	UINT8 mntr_tx_strm_pth; /* Monitoring TX stream path */
	UINT8 mntr_rx_strm_pth; /* Monitoring RX stream path */
	UINT8 scan_mode;        /* Scan Mode: ScanStart/ScanRunning/ScanStop */
	UINT8 dbdc_idx;         /* DbdcIdx */

	UINT8 off_ch_scn_type;  /* OffChScanType */
	UINT8 is_aband;	        /* 0: 2.4G channel, 1: 5G channel */
	UINT8 reserved[2];      /* reserved bytes */
} EXT_CMD_OFF_CH_SCAN_CTRL_T, *P_EXT_CMD_OFF_CH_SCAN_CTRL_T;

typedef enum _ENUM_WH_PHY_OFF_CHANNEL_SCAN_TYPE_T {
	off_ch_scan_off = 0,
	off_ch_scan_balanced_band,
	off_ch_scan_simple_rx,
	off_ch_scan_type_num
} ENUM_WH_PHY_OFF_CHANNEL_SCAN_TYPE_T, *P_ENUM_WH_PHY_OFF_CHANNEL_SCAN_TYPE_T;

typedef enum _ENUM_WH_PHY_OFF_CHANNEL_SCAN_CH_IDX_T {
	off_ch_wrk_ch_idx = 0,	/* Working channel index */
	off_ch_mntr_ch_idx,		/* Monitoring channel index */
	off_ch_ch_idx_num
} ENUM_WH_PHY_OFF_CHANNEL_SCAN_CH_IDX_T, *P_ENUM_WH_PHY_OFF_CHANNEL_SCAN_CH_IDX_T;

typedef enum _ENUM_SCAN_MODE_NUM_T {
	off_ch_scan_mode_stop = 0,
	off_ch_scan_mode_start,
	off_ch_scan_mode_running,
	off_ch_scan_mode_num
} ENUM_SCAN_MODE_NUM_T, *P_ENUM_SCAN_MODE_NUM_T;

#endif

typedef struct _SET_ADC_T {
	UINT32  u4ChannelFreq;
	UINT8	ucAntIndex;
	UINT8	ucBW;
	UINT8   ucSX;
	UINT8	ucDbdcIdx;
	UINT8	ucRunType;
	UINT8	ucFType;
	UINT8	aucReserved[2];		/* Reserving For future */
} SET_ADC_T, *P_SET_ADC_T;

typedef struct _SET_RX_GAIN_T {
	UINT8	ucLPFG;
	UINT8   ucLNA;
	UINT8	ucDbdcIdx;
	UINT8	ucAntIndex;
} SET_RX_GAIN_T, *P_EXT_SET_RX_GAIN_T;

typedef struct _SET_TTG_T {
	UINT32  u4ChannelFreq;
	UINT32  u4ToneFreq;
	UINT8	ucTTGPwrIdx;
	UINT8	ucDbdcIdx;
	UINT8	ucXtalFreq;
	UINT8	aucReserved[1];
} SET_TTG_T, *P_SET_TTG_T;

typedef struct _TTG_ON_OFF_T {
	UINT8	ucTTGEnable;
	UINT8	ucDbdcIdx;
	UINT8	ucAntIndex;
	UINT8	aucReserved[1];
} TTG_ON_OFF_T, *P_TTG_ON_OFF_T;

typedef struct _CONTINUOUS_TX_PARAM_T {
	UINT8 ucCtrlCh;
	UINT8 ucCentralCh;
	UINT8 ucBW;
	UINT8 ucAntIndex;
	UINT16 u2RateCode;
	UINT8 ucBand;
	UINT8 ucTxfdMode;
} CONTINUOUS_TX_PARAM_T, *P_CONTINUOUS_TX_PARAM_T;

typedef struct _TX_TONE_POWER_GAIN_T {
	UINT8 ucAntIndex;
	UINT8 ucTonePowerGain;
	UINT8 ucBand;
	UINT8 aucReserved[1];
} TX_TONE_POWER_GAIN_T, *P_TX_TONE_POWER_GAIN_T;

/*
u4FuncIndex	0x01: RE_CALIBRATION
			0x02: CALIBRATION_BYPASS
			0x03: TX_TONE_START
			0x04: TX_TONE_STOP
			0x05: CONTINUOUS_TX_START
			0x06: CONTINUOUS_TX_STOP
			0x07. RF_AT_EXT_FUNCID_TX_TONE_RF_GAIN
			0x08. RF_AT_EXT_FUNCID_TX_TONE_DIGITAL_GAIN
			0x09: CAL_RESULT_DUMP_FLAG
			0x0A: RDD_TEST_MODE
u4FuncData:
	works when u4FuncIndex = 1 or 2 or 0xA
u4CalDump:
	works when u4FuncIndex = 9
TX_TONE_PARAM_T:
	works when u4FuncIndex = 3
CONTINUOUS_TX_PARAM_T:
	works when u4FuncIndex = 5
TX_TONE_POWER_GAIN_T:
	works when u4FuncIndex = 7 or 8

*/
typedef struct _RF_TEST_CALIBRATION_T {
	UINT32	u4FuncData;
	UINT8	ucDbdcIdx;
	UINT8	aucReserved[3];
} RF_TEST_CALIBRATION_T, *P_RF_TEST_CALIBRATION_T;

typedef struct _PARAM_MTK_WIFI_TEST_STRUC_T {
	UINT32         u4FuncIndex;
	union {
		UINT32 u4FuncData;
		UINT32 u4CalDump;
		RF_TEST_CALIBRATION_T rCalParam;
		TX_TONE_PARAM_T rTxToneParam;
		CONTINUOUS_TX_PARAM_T rConTxParam;
		TX_TONE_POWER_GAIN_T rTxToneGainParam;
#ifdef INTERNAL_CAPTURE_SUPPORT
		RBIST_CAP_START_T rICapInfo;
		RBIST_DUMP_RAW_DATA_T rICapDump;
#endif/*INTERNAL_CAPTURE_SUPPORT*/
		EXT_CMD_RDD_ON_OFF_CTRL_T rRDDParam;
		SET_ADC_T rSetADC;
		SET_RX_GAIN_T rSetRxGain;
		SET_TTG_T	rSetTTG;
		TTG_ON_OFF_T rTTGOnOff;
	} Data;
} PARAM_MTK_WIFI_TEST_STRUC_T, *P_PARAM_MTK_WIFI_TEST_STRUC_T;

typedef struct _CMD_TEST_CTRL_T {
	UINT8 ucAction;
	UINT8 ucIcapLen;
	UINT8 aucReserved[2];
	union {
		UINT32 u4OpMode;
		UINT32 u4ChannelFreq;
		PARAM_MTK_WIFI_TEST_STRUC_T rRfATInfo;
	} u;
} CMD_TEST_CTRL_T, *P_CMD_TEST_CTRL_T;

typedef struct _EXT_EVENT_RF_TEST_RESULT_T {
	UINT32 u4FuncIndex;
	UINT32 u4PayloadLength;
	UINT8  aucEvent[0];
} EXT_EVENT_RF_TEST_RESULT_T, *PEXT_EVENT_RF_TEST_RESULT_T;

typedef struct _EXT_EVENT_RF_TEST_DATA_T {
	UINT32 u4CalIndex;
	UINT32 u4CalType;
	UINT8  aucData[0];
} EXT_EVENT_RF_TEST_DATA_T, *PEXT_EVENT_RF_TEST_DATA_T;

#define WIFI_RX_DISABLE 1
typedef struct GNU_PACKED _EXT_CMD_WIFI_RX_DISABLE_T {
	UINT8 ucWiFiRxDisableCtrl;
	UINT8 aucReserve[3];
} EXT_CMD_WIFI_RX_DISABLE_T;

#define PHY_SHAPING_FILTER_DISABLE 1
typedef struct GNU_PACKED _EXT_CMD_PHY_SHAPING_FILTER_DISABLE_T {
	UINT8 ucPhyShapingFilterDisable;
	UINT8 aucReserve[3];
} EXT_CMD_PHY_SHAPING_FILTER_DISABLE_T;

/* Power Management Level */
#define PM2         2
#define PM4         4
#define PM5         5
#define PM6         6
#define PM7         7
#define ENTER_PM_STATE 1
#define EXIT_PM_STATE 2

#define KEEP_ALIVE_INTERVAL_IN_SEC	10		/* uint: sec */
/* Beacon lost timing */
#define BEACON_OFFLOAD_LOST_TIME	30		/* unit: beacon --> 30 beacons about 3sec */

typedef struct GNU_PACKED _EXT_CMD_PM_STATE_CTRL_T {
	UINT8 ucPmNumber;
	UINT8 ucPmState;
	UINT8 aucBssid[6];
	UINT8 ucDtimPeriod;
	UINT8 ucWlanIdxL;		/* #256STA - Low Byte */
	UINT16 u2BcnInterval;
	UINT32 u4Aid;
	UINT32 u4RxFilter;
	UINT8 ucDbdcIdx;
	UINT8 ucWlanIdxHnVer;	/* #256STA - High Byte and Version */
	UINT8 aucReserve0[2];
	UINT32 u4Feature;
	UINT8 ucOwnMacIdx;
	UINT8 ucWmmIdx;
	UINT8 ucBcnLossCount;
	UINT8 ucBcnSpDuration;
} EXT_CMD_PM_STATE_CTRL_T, *P_EXT_CMD_PM_STATE_CTRL_T;

typedef struct GNU_PACKED _EXT_CMD_GREENAP_CTRL_T {
	UINT8 ucDbdcIdx;
	BOOLEAN ucGreenAPOn;
	UINT8 aucReserve[2];
} EXT_CMD_GREENAP_CTRL_T, *P_EXT_GREENAP_CTRL_T;

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
typedef struct GNU_PACKED _EXT_CMD_PCIE_ASPM_DYM_CTRL_T {
	UINT8       ucDbdcIdx;
	BOOLEAN     fgL1Enable;
	BOOLEAN     fgL0sEnable;
	UINT8       ucReserve;
} EXT_CMD_PCIE_ASPM_DYM_CTRL_T, *P_EXT_PCIE_ASPM_DYM_CTRL_T;
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
/* MT CMD-EVENT */
/***** TWT Application Note Start *****/
/* AP TWT negotiation rule: It is okay for a STA to setup multiple Individual TWT AGRT with the same AP */
/* AP TWT negotiation rule: A STA is not allowed to join both Individual and Group TWT agreement */
/* AP TWT negotiation rule: A STA is not allowed to join more than one group TWT agreement **/
/* If the host issue TWT Update with agrt_ctrl_flag==TWT_AGRT_CTRL_MODIFY, then the content of the */
/*    AGRT ID Tuple (own_mac_idx, flow_id, peer_id_grp_id) should be keep unchanged. */
/***** TWT Application Note End *****/

#define TWT_BTWT_ID_BIT                         BIT(14)
#define TWT_GROUP_ID_BIT                        BIT(15)

#define TWT_BTWT_ID                             BITS(0, 4)


/* Bitmap definition for AgrtParaBitmap */
#define TWT_AGRT_PARA_BITMAP_IS_TRIGGER         BIT(0)
#define TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE        BIT(1)
#define TWT_AGRT_PARA_BITMAP_IS_PROTECT         BIT(2)
#define TWT_AGRT_PARA_BITMAP_IS_ALL_TWT         BIT(3) /* twt information, all twt*/
#define TWT_AGRT_PARA_BITMAP_NEXT_TWT_32_BITS   BIT(4) /* twt information, next twt*/
#define TWT_AGRT_PARA_BITMAP_NEXT_TWT_48_BITS   BIT(5) /* twt information, next twt*/
#define TWT_AGRT_PARA_BITMAP_NEXT_TWT_64_BITS   BIT(6) /* twt information, next twt*/
#define TWT_AGRT_PARA_BITMAP_WAKE_DUR_UINT      BIT(7) /* control.wake_dur_unit 0:256us, 1:TU */

typedef enum _TWT_AGRT_CTRL_CODE_T {
	TWT_AGRT_CTRL_ADD = 0,
	TWT_AGRT_CTRL_MODIFY = 1,
	TWT_AGRT_CTRL_DELETE = 2,
	TWT_AGRT_CTRL_TEARDOWN = 3,
	TWT_AGRT_CTRL_GET_CURR_TSF = 4,
	TWT_AGRT_CTRL_SUSPEND = 5,
	TWT_AGRT_CTRL_SUSPEND_RESUME = 6,
	TWT_AGRT_CTRL_DBG_DUMP = 7
} TWT_AGRT_CTRL_CODE_T, *P_TWT_AGRT_CTRL_CODE_T;

struct GNU_PACKED ext_cmd_twt_agrt_update {
	/* Important: Used for Communication between Host and WM-CPU, should be packed and DW-aligned and in little-endian format */
	/* DW0 */
	UINT8		agrt_tbl_idx;
	UINT8		agrt_ctrl_flag;
	UINT8		own_mac_idx;
	/* It is set to 0xff when peerGrpId is a group ID */
	UINT8		flow_id;
	/* DW1 */
	/* Specify the peer ID (MSB=0) or group ID (MSB=1)  (10 bits for StaIdx, MSB to identify if it is for groupId) */
	UINT16		peer_id_grp_id;
	/* Same as SPEC definition. 8 bits, in unit of 256 us */
	UINT8		agrt_sp_duration;
	/* So that we know which BSS TSF should be used for this AGRT */
	UINT8		bss_idx;
	/* DW2, DW3, DW4 */
	UINT32		agrt_sp_start_tsf_low;
	UINT32		agrt_sp_start_tsf_high;
	UINT16		agrt_sp_wake_intvl_mantissa;
	UINT8		agrt_sp_wake_intvl_exponent;
	UINT8		is_role_ap;
	/* DW5 */
	/* For Bitmap definition,please refer to TWT_AGRT_PARA_BITMAP_IS_TRIGGER and etc */
	UINT8		agrt_para_bitmap;
	UINT8		persistence;
	UINT16		reserved_b;

	/* Following field is valid ONLY when peer_id_grp_id is a group ID */
	/* DW6 */
	UINT8		grp_member_cnt;
	UINT8		reserved_c;
	UINT16		reserved_d;
	/* DW7 ... */
	UINT16		sta_list[TWT_HW_GRP_MAX_MEMBER_CNT];
};

typedef struct _EVENT_TWT_RESUME_INFO_T {
	UINT8 bssinfo_idx;
	UINT16 wcid;
	UINT8 flow_id;
	UINT8 idle;
	UINT8 reserved[3];
} EVENT_TWT_RESUME_INFO_T, *P_EVENT_TWT_RESUME_INFO;

#define SET_BTWT_ID(_ctrl, btwt_id) \
	((_ctrl->peer_id_grp_id) = (TWT_BTWT_ID_BIT | btwt_id))

#define GET_BTWT_ID(_ctrl) \
	((_ctrl->peer_id_grp_id) & TWT_BTWT_ID)

#define IS_BTWT_ID(_ctrl) \
	(((_ctrl->peer_id_grp_id) & TWT_BTWT_ID_BIT) ? TRUE : FALSE)


#define CLR_AGRT_PARA_BITMAP(_ctrl) \
	(_ctrl->agrt_para_bitmap = 0)

#define SET_AGRT_PARA_BITMAP(_ctrl, bit_value) \
	((_ctrl->agrt_para_bitmap) |= bit_value)

#define GET_AGRT_PARA_BITMAP(_ctrl, bit_value) \
	(((_ctrl->agrt_para_bitmap) & bit_value) ? TRUE : FALSE)

#endif /* WIFI_TWT_SUPPORT */

typedef struct _CMD_STAREC_MU_EDCA_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	struct he_mu_edca_params arMUEdcaParams[ACI_AC_NUM];
} CMD_STAREC_MU_EDCA_T, *P_CMD_STAREC_MU_EDCA_T;

#endif /* DOT11_HE_AX */

#define BITS2(m, n)              (BIT(m) | BIT(n))
#define BITS3(m, n, o)            (BIT(m) | BIT(n) | BIT(o))
#define BITS4(m, n, o, p)          (BIT(m) | BIT(n) | BIT(o) | BIT(p))
#define BITS(m, n)              (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))

/* Station role */
#define STA_TYPE_STA		BIT(0)
#define STA_TYPE_AP		BIT(1)
#define STA_TYPE_ADHOC	BIT(2)
#define STA_TYPE_TDLS		BIT(3)
#define STA_TYPE_WDS		BIT(4)
#define STA_TYPE_BC		BIT(5)

/* Network type */
#define NETWORK_INFRA	BIT(16)
#define NETWORK_P2P		BIT(17)
#define NETWORK_IBSS		BIT(18)
#define NETWORK_MESH	BIT(19)
#define NETWORK_BOW		BIT(20)
#define NETWORK_WDS		BIT(21)

/* Connection type */
#define CONNECTION_INFRA_STA		(STA_TYPE_STA|NETWORK_INFRA)
#define CONNECTION_INFRA_AP		(STA_TYPE_AP|NETWORK_INFRA)
#define CONNECTION_P2P_GC			(STA_TYPE_STA|NETWORK_P2P)
#define CONNECTION_P2P_GO			(STA_TYPE_AP|NETWORK_P2P)
#define CONNECTION_MESH_STA		(STA_TYPE_STA|NETWORK_MESH)
#define CONNECTION_MESH_AP		(STA_TYPE_AP|NETWORK_MESH)
#define CONNECTION_IBSS_ADHOC		(STA_TYPE_ADHOC|NETWORK_IBSS)
#define CONNECTION_TDLS			(STA_TYPE_STA|NETWORK_INFRA|STA_TYPE_TDLS)
#define CONNECTION_WDS			(STA_TYPE_WDS|NETWORK_WDS)
#define CONNECTION_INFRA_BC		(STA_TYPE_BC|NETWORK_INFRA)

#define	MAX_BUF_SIZE_OF_DEVICEINFO (sizeof(CMD_DEVINFO_UPDATE_T) + sizeof(CMD_DEVINFO_ACTIVE_T))

enum {
	DEVINFO_ACTIVE = 0,
	DEVINFO_MAX_NUM = 1,
};

#define MAX_WO_PROF_LVL	(10)

/* WO_CMD_WED_RX_STAT */
struct wo_cmd_rxstat_para {
	u32	rx_err_cnt;
	u32	rx_drop_cnt;
	u32	rx_rev_cnt;
	u32	rx_ack_cnt;
	u32	prof_record[MAX_WO_PROF_LVL*2];
};

typedef struct _CMD_HIF_LOOPBACK {
	UINT32				Loopback_Enable:16;
	UINT32				DestinationQid:16;
} CMD_HIF_LOOPBACK, *PCMD_HIF_LOOPBACK;

enum {
	DEVINFO_ACTIVE_FEATURE = (1 << DEVINFO_ACTIVE),
	DEVINFO_MAX_NUM_FEATURE = (1 << DEVINFO_MAX_NUM)
};

enum {
	HIF_CTRL_ID_RESERVED = 0,
	HIF_CTRL_ID_HIF_USB_TX_RX_IDLE = 1,
};

typedef struct GNU_PACKED _EXT_CMD_WIFI_HIF_CTRL_T {
	UINT8  ucHifCtrlId;
	UINT8  ucDbdcIdx;
	UINT8  aucReserved[6];
} EXT_CMD_WIFI_HIF_CTRL_T, *P_EXT_CMD_WIFI_HIF_CTRL_T;

typedef struct GNU_PACKED _CMD_DEVINFO_UPDATE_T {
	UINT8	ucOwnMacIdx;
	UINT8	ucBandIdx;
	UINT16	u2TotalElementNum;
	UINT8    ucAppendCmdTLV;
	UINT8	aucReserve[3];
	UINT8	aucBuffer[];
} CMD_DEVINFO_UPDATE_T, *P_CMD_DEVINFO_UPDATE_T;

typedef struct GNU_PACKED _CMD_DEVINFO_ACTIVE_T {
	/* Device information (Tag0) */
	UINT16	u2Tag;		/* Tag = 0x00 */
	UINT16	u2Length;
	UINT8	ucActive;
	UINT8	ucDbdcIdx;
	UINT8	aucOwnMAC[6];
} CMD_DEVINFO_ACTIVE_T, *P_CMD_DEVINFO_ACTIVE_T;

typedef struct GNU_PACKED _CMD_DEVINFO_BSSIDX_T {
	/* Dev information (Tag1) */
	UINT16	u2Tag;		/* Tag = 0x01 */
	UINT16	u2Length;
	UINT8	ucBSSIndex;
	UINT8	aucReserve[3];
	UINT32	ucDevConnectionType;
} CMD_DEVINFO_BSSIDX_T, *P_CMD_BSSINFO_BSSIDX_T;

#define ORI_BA  1
#define RCV_BA  2

enum {
	STA_REC_BASIC_STA_RECORD = 0,
	STA_REC_RA = 1,
	STA_REC_RA_COMMON_INFO = 2,
	STA_REC_RA_UPDATE = 3,
	STA_REC_BF = 4,
	STA_REC_AMSDU = 5,
	STA_REC_BA = 6,
	STA_REC_RED = 7,
	STA_REC_TX_PROC = 8,
	STA_REC_BASIC_HT_INFO = 9,
	STA_REC_BASIC_VHT_INFO = 10,
	STA_REC_AP_PS = 11,
	STA_REC_INSTALL_KEY = 12,
	STA_REC_WTBL = 13,
	STA_REC_BASIC_HE_INFO = 14,
	STA_REC_HW_AMSDU = 15,
	STA_REC_WTBL_AADOM = 16,
	STA_REC_INSTALL_KEY_V2 = 17,
	STA_REC_MURU = 18,
	STA_REC_MUEDCA = 19,
	STA_REC_BFEE = 20,
	STA_REC_HE_6G_CAP = 21,

	STA_REC_UWTBL_RAW = 22,
	STA_REC_SN = 24,
	STA_REC_MAX_NUM
};

#define STA_REC_WTBL_PSM	0x24

enum {
	STA_REC_BASIC_STA_RECORD_FEATURE = (1 << STA_REC_BASIC_STA_RECORD),
	STA_REC_RA_FEATURE = (1 << STA_REC_RA),
	STA_REC_RA_COMMON_INFO_FEATURE = (1 << STA_REC_RA_COMMON_INFO),
	STA_REC_RA_UPDATE_FEATURE = (1 << STA_REC_RA_UPDATE),
	STA_REC_BF_FEATURE = (1 << STA_REC_BF),
	STA_REC_AMSDU_FEATURE = (1 << STA_REC_AMSDU),
	STA_REC_BA_FEATURE = (1 << STA_REC_BA),
	STA_REC_RED_FEATURE = (1 << STA_REC_RED),
	STA_REC_TX_PROC_FEATURE = (1 << STA_REC_TX_PROC),
	STA_REC_BASIC_HT_INFO_FEATURE  = (1 << STA_REC_BASIC_HT_INFO),
	STA_REC_BASIC_VHT_INFO_FEATURE = (1 << STA_REC_BASIC_VHT_INFO),
	STA_REC_AP_PS_FEATURE = (1 << STA_REC_AP_PS),
	STA_REC_INSTALL_KEY_FEATURE = (1 << STA_REC_INSTALL_KEY),
	STA_REC_WTBL_FEATURE = (1 << STA_REC_WTBL),
	STA_REC_HW_AMSDU_FEATURE = (1 << STA_REC_HW_AMSDU),
	STA_REC_BASIC_HE_INFO_FEATURE = (1 << STA_REC_BASIC_HE_INFO),
	STA_REC_INSTALL_KEY_V2_FEATURE = (1 << STA_REC_INSTALL_KEY_V2),
	STA_REC_MURU_FEATURE = (1 << STA_REC_MURU),
	STA_REC_MUEDCA_FEATURE = (1 << STA_REC_MUEDCA),
	STA_REC_BFEE_FEATURE = (1 << STA_REC_BFEE),
	STA_REC_HE_6G_CAP_FEATURE = (1 << STA_REC_HE_6G_CAP),
	STA_REC_UWTBL_RAW_FEATURE = (1 << STA_REC_UWTBL_RAW),
	STA_REC_MAX_NUM_FEATURE = (1 << STA_REC_MAX_NUM)
};


enum {
	CMD_HE_MCS_BW80,
	CMD_HE_MCS_BW160,
	CMD_HE_MCS_BW8080,
	CMD_HE_MCS_BW_NUM
};

typedef struct _STAREC_HANDLE_T {
	UINT32 StaRecTag;
	UINT32 StaRecTagLen;
	INT32 (*StaRecTagHandler)(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *args);
} STAREC_HANDLE_T, *P_STAREC_HANDLE_T;

typedef struct GNU_PACKED _CMD_STAREC_UPDATE_T {
	UINT8	ucBssIndex;
	UINT8	ucWlanIdxL;		/* #256STA - Low Byte */
	UINT16	u2TotalElementNum;
	UINT8	ucAppendCmdTLV;
	UINT8   ucMuarIdx;
	UINT8	ucWlanIdxHnVer;		/* #256STA - High Byte and Version */
	UINT8	aucReserve;
	UINT8	aucBuffer[];
} CMD_STAREC_UPDATE_T, *P_CMD_STAREC_UPDATE_T;

typedef struct GNU_PACKED _CMD_STAREC_HT_INFO_T {
	UINT16	u2Tag;
	UINT16	u2Length;
	UINT16	u2HtCap;
	UINT16	u2Reserved;
} CMD_STAREC_HT_INFO_T, *P_CMD_STAREC_HT_INFO_T;

typedef struct GNU_PACKED _CMD_STAREC_VHT_INFO_T {
	UINT16	u2Tag;
	UINT16	u2Length;
	UINT32	u4VhtCap;
	UINT16	u2VhtRxMcsMap;
	UINT16	u2VhtTxMcsMap;
	UINT8	ucRTSBWSig;	/* 0-disable DynBW, 1-static BW, 2 Dynamic BW */
	UINT8	aucReserve[3];
} CMD_STAREC_VHT_INFO_T, *P_CMD_STAREC_VHT_INFO_T;

enum {
	/*MAC cap*/
	STA_REC_HE_CAP_HTC = 0,
	STA_REC_HE_CAP_BQR = 1,
	STA_REC_HE_CAP_BSR = 2,
	STA_REC_HE_CAP_OM = 3,
	STA_REC_HE_CAP_AMSDU_IN_AMPDU = 4,
	/*PHY cap*/
	STA_REC_HE_CAP_DUAL_BAND = 5,
	STA_REC_HE_CAP_LDPC = 6,
	STA_REC_HE_CAP_TRIG_CQI_FK = 7,
	STA_REC_HE_CAP_PARTIAL_BW_EXT_RANGE = 8,
	/*STBC cap*/
	STA_REC_HE_CAP_LE_EQ_80M_TX_STBC = 9,
	STA_REC_HE_CAP_LE_EQ_80M_RX_STBC = 10,
	STA_REC_HE_CAP_GT_80M_TX_STBC = 11,
	STA_REC_HE_CAP_GT_80M_RX_STBC = 12,
	/*GI cap*/
	STA_REC_HE_CAP_SU_PPDU_1x_LTF_DOT8US_GI = 13,
	STA_REC_HE_CAP_SU_PPDU_MU_PPDU_4x_LTF_DOT8US_GI = 14,
	STA_REC_HE_CAP_ER_SU_PPDU_1x_LTF_DOT8US_GI = 15,
	STA_REC_HE_CAP_ER_SU_PPDU_4x_LTF_DOT8US_GI = 16,
	STA_REC_HE_CAP_NDP_4x_LTF_3DOT2MS_GI = 17,
	/*BW20_242TONE*/
	STA_REC_HE_CAP_BW20_RU242_SUPPORT = 18,
	/*1024 QAM under 242TONE*/
	STA_REC_HE_CAP_TX_1024QAM_UNDER_RU242 = 19,
	STA_REC_HE_CAP_RX_1024QAM_UNDER_RU242 = 20,
	/*MAC cap*/
	STA_REC_HE_CAP_HE_DYNAMIC_SMPS = 21,
	STA_REC_HE_CAP_MAX
};

typedef struct GNU_PACKED _CMD_STAREC_HE_INFO_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT32 u4HeCap;
	UINT8  ucTrigerFrameMacPadDuration;
	UINT8  ucMaxAmpduLenExponent;
	UINT8  ucChBwSet;
	UINT8  ucDeviceClass;
	UINT8  ucDcmTxMode;
	UINT8  ucDcmTxMaxNss;
	UINT8  ucDcmRxMode;
	UINT8  ucDcmRxMaxNss;
	UINT8  ucDcmMaxRu;
	UINT8  ucPuncPreamRx;
	UINT8  ucPktExt;
	UINT8  aucReserve0[1];
	UINT16 au2MaxNssMcs[CMD_HE_MCS_BW_NUM];
	UINT8  aucReserve1[2];
} CMD_STAREC_HE_INFO_T, *P_CMD_STAREC_HE_INFO_T;

typedef struct _MURU_WDEV_CFG {
	BOOLEAN fgDlOfdmaEn;
	BOOLEAN fgUlOfdmaEn;
	BOOLEAN fgDlMimoEn;
	BOOLEAN fgUlMimoEn;
	UINT8 u1Reserved[4];
} MURU_WDEV_CFG, *P_MURU_WDEV_CFG;

typedef struct _MURU_STA_DL_OFDMA {
	UINT8 u1PhyPunRx;			/* PHY B8-B11 : Punctured Preamble RX */
	BOOLEAN u120MIn40M2G;		/* PHY B65 : 20 MHz In 40 MHz HE PPDU In 2.4 GHz */
	BOOLEAN u120MIn160M;		/* PHY B66 : 20 MHz In 160/ 80+80 MHz HE PPDU */
	BOOLEAN u180MIn160M;		/* PHY B67 : 80 MHz In 160/ 80+80 MHz HE PPDU */
	BOOLEAN u1Lt16SigB;			/* PHY B72 : Longer Than 16 HE SIG-B OFDM Symbols Support  */
	BOOLEAN u1RxSUCompSigB;		/* PHY B76 : Rx Full BW SU Using HE MU PPDU With Compressed SIGB */
	BOOLEAN u1RxSUNonCompSigB;	/* PHY B77 : Rx Full BW SU Using HE MU PPDU With Non- Compressed SIGB  */
	UINT8 u1Reserve;
} MURU_STA_DL_OFDMA, *P_MURU_STA_DL_OFDMA;

typedef struct _MURU_STA_UL_OFDMA {
	UINT8 u1TrigFrmPad;		/* MAC B10-11 : Trigger Frame MAC Padding Duration */
	UINT8 u1MuCascading;		/* MAC B22 : MU Cascading Support */
	UINT8 u1UoRa;				/* MAC B26 : OFDMA RA Support */
	UINT8 u12x996Tone;			/* MAC B43 : UL 2x996- tone RU Support */
	UINT8 u1RxTrgFrmBy11ac;	/* MAC B47 : HT And VHT Trigger Frame RX Support  */
#ifdef WIFI_UNIFIED_COMMAND
	UINT_8 u1RxCtrlFrmToMBss;   /* MAC B31 : Rx Control Frame To MultiBSS Support  */
    UINT_8 u1Reserved[2];
#else  /*WIFI_UNIFIED_COMMAND*/
	UINT8 u1Reserved[3];
#endif /*WIFI_UNIFIED_COMMAND*/
} MURU_STA_UL_OFDMA, *P_MURU_STA_UL_OFDMA;

typedef struct _MURU_STA_DL_MIMO {
	BOOLEAN fgVhtMuBfee;
	BOOLEAN fgParBWDlMimo;		/* Phy B54 : Partial Bandwidth DL MU-MIMO */
	UINT8 u1Reserved[2];
} MURU_STA_DL_MIMO, *P_MURU_STA_DL_MIMO;

typedef struct _MURU_STA_UL_MIMO {
	BOOLEAN fgFullUlMimo;		/* Phy B22 : Full BW UL MU-MIMO */
	BOOLEAN fgParUlMimo;		/* Phy B23 : Partial BW UL MU-MIMO */
	UINT8 u1Reserved[2];
} MURU_STA_UL_MIMO, *P_MURU_STA_UL_MIMO;

typedef struct _MURU_STACAP_INFO {
	MURU_WDEV_CFG rWdevCfg;
	MURU_STA_DL_OFDMA rDlOfdma;
	MURU_STA_UL_OFDMA rUlOfdma;
	MURU_STA_DL_MIMO rDlMimo;
	MURU_STA_UL_MIMO rUlMimo;
} MURU_STACAP_INFO, *P_MURU_STACAP_INFO;

typedef struct _CMD_STAREC_MURU_T {
	UINT16 u2Tag;
	UINT16 u2Length;
	MURU_STACAP_INFO rMuRuStaCap;
} CMD_STAREC_MURU_T, *P_CMD_STAREC_MURU_T;

#define STAREC_COMMON_EXTRAINFO_V2		BIT(0)
#define STAREC_COMMON_EXTRAINFO_NEWSTAREC	BIT(1)
#define STAREC_COMMON_EXTRAINFO_NEWSTAREC_DUMMY	BIT(2)

typedef struct GNU_PACKED _STAREC_COMMON_T {
	/* Basic STA record (Group0) */
	UINT16	u2Tag;		/* Tag = 0x00 */
	UINT16	u2Length;
	UINT32	u4ConnectionType;
	UINT8	ucConnectionState;
	UINT8	ucIsQBSS;
	UINT16	u2AID;
	UINT8	aucPeerMacAddr[6];
	/*This is used especially for 7615 to indicate this STAREC is to create new one or simply update
	In some case host may send new STAREC without delete old STAREC in advance. (ex: lost de-auth or get assoc twice)
	We need extra info to know if this is a brand new STAREC or not
	Consider backward compatibility, we check bit 0 in this reserve.
	Only the bit 0 is on, N9 go new way to update STAREC if bit 1 is on too.
	If neither bit match, N9 go orinal way to update STAREC. */
	UINT16	u2ExtraInfo;

} CMD_STAREC_COMMON_T, *P_CMD_STAREC_COMMON_T;

#define STATE_DISCONNECT 0
#define STATE_CONNECTED 1
#define STATE_PORT_SECURE 2

typedef struct GNU_PACKED _STAREC_AMSDU_T {
	UINT16  u2Tag;		/* Tag = 0x05 */
	UINT16  u2Length;
	UINT8   ucMaxAmsduNum;
	UINT8   ucMaxMpduSize;
    UINT8   ucAmsduEnable;
	UINT8   acuReserve[1];
} CMD_STAREC_AMSDU_T, *P_CMD_STAREC_AMSDU_T;

typedef struct GNU_PACKED _STAREC_BA_T {
	UINT16 u2Tag;       /* Tag = 0x06 */
	UINT16 u2Length;
	UINT8 ucTid;
	UINT8 ucBaDirection;
	UINT8 ucAmsduCap;
	UINT8 ucBaEenable;
	UINT16 u2BaStartSeq;
	UINT16 u2BaWinSize;
} CMD_STAREC_BA_T, *P_CMD_STAREC_BA_T;

typedef struct GNU_PACKED _STAREC_RED_T {
	UINT16	u2Tag;		/* Tag = 0x07 */
	UINT16	u2Length;
	UINT32	u4RED[8];
} CMD_STAREC_RED_T, *P_CMD_STAREC_RED_T;

typedef struct GNU_PACKED _STAREC_TX_PROC_T {
	UINT16	u2Tag;		/* Tag = 0x08 */
	UINT16	u2Length;
	UINT32	u4TxProcFlag;
} CMD_STAREC_TX_PROC_T, *P_CMD_STAREC_TX_PROC_T;

typedef struct GNU_PACKED _STAREC_PS_T {
	UINT16	u2Tag;		/* Tag = 11 */
	UINT16	u2Length;
	UINT8	ucStaBmpDeliveryAC;
	UINT8	ucStaBmpTriggerAC;
	UINT8	ucStaMaxSPLength;
	UINT8	ucReserve1[1];
	UINT16	u2StaListenInterval;
	UINT8	ucReserve2[2];
} CMD_STAREC_PS_T, *P_CMD_STAREC_PS_T;

#ifdef HTC_DECRYPT_IOT
typedef struct GNU_PACKED _STAREC_AADOM_T {
	UINT16 u2Tag;       /* Tag = 0x10 */
	UINT16 u2Length;
	UINT8 ucAadOm;
	UINT8 acuReserve[1];
} CMD_STAREC_AADOM_T, *P_CMD_STAREC_AADOM_T;
#endif /* HTC_DECRYPT_IOT */

typedef struct GNU_PACKED _STAREC_PSM_T {
	UINT16 u2Tag;       /* Tag = 0x10 */
	UINT16 u2Length;
	UINT8 ucPsmMode;
	UINT8 acuReserve[1];
} CMD_STAREC_PSM_T, *P_CMD_STAREC_PSM_T;

typedef enum _ENUM_SN_SOURCE_TYPE_T {
	TID0_AC0_SN = 0,
	TID0_AC1_SN,
	TID0_AC2_SN,
	TID0_AC3_SN,
	TID4_SN_T,
	TID5_SN_T,
	TID6_SN_T,
	TID7_SN_T,
	COMMON_SN,
	MAX_NUM_SN_SOURCE
} ENUM_SN_SOURCE_TYPE_T;

typedef struct GNU_PACKED _STAREC_SN_T {
	UINT16 u2Tag;       /* Tag = 0x25 */
	UINT16 u2Length;
	UINT16 u2SN;
	UINT8  acuReserve[2];
} CMD_STAREC_SN_T, *P_CMD_STAREC_SN_T;

#define RVLAN BIT(0);
#define IPCSO BIT(1);
#define TCPUDPCSO BIT(2);
#define	TX_PROC_ACM_CFG_EN BIT(3);
#define TX_PROC_ACM_CFG_BK BIT(4);
#define TX_PROC_ACM_CFG_BE BIT(5);
#define TX_PROC_ACM_CFG_VI BIT(6);
#define TX_PROC_ACM_CFG_VO BIT(7);

#define MAX_BUF_SIZE_OF_BSS_INFO \
	(sizeof(CMD_BSSINFO_UPDATE_T) + \
	 sizeof(CMD_BSSINFO_CONNECT_OWN_DEV_T) + \
	 sizeof(CMD_BSSINFO_BASIC_T) + \
	 sizeof(CMD_BSSINFO_RF_CH_T) + \
	 sizeof(CMD_BSSINFO_PM_T) + \
	 sizeof(CMD_BSSINFO_UAPSD_T) + \
	 sizeof(CMD_BSSINFO_RSSI_RM_DET_T) + \
	 sizeof(CMD_BSSINFO_EXT_BSS_INFO_T) + \
	 sizeof(CMD_BSSINFO_BMC_RATE_T) + \
	 sizeof(CMD_BSSINFO_SYNC_MODE_CTRL_T) + \
	 sizeof(CMD_BSSINFO_AUTO_RATE_CFG_T) +\
	 sizeof(CMD_BSSINFO_HW_AMSDU_INFO_T) +\
	 sizeof(CMD_BSSINFO_BSS_COLOR_T) +\
	 sizeof(CMD_BSSINFO_HE_BASIC_T))

#define MAX_BUF_SIZE_OF_BSSINFO_OFFLOAD_PKT	1520

enum {
	BSS_INFO_OWN_MAC = 0,
	BSS_INFO_BASIC = 1,
	BSS_INFO_RF_CH = 2,
	BSS_INFO_PM = 3,
	BSS_INFO_UAPSD = 4,
	BSS_INFO_ROAM_DETECTION = 5,
	BSS_INFO_LQ_RM = 6,
	BSS_INFO_EXT_BSS = 7,
	BSS_INFO_BROADCAST_INFO = 8,
	BSS_INFO_SYNC_MODE = 9,
	BSS_INFO_RA = 10,
	BSS_INFO_HW_AMSDU = 11,
	BSS_INFO_BSS_COLOR = 12,
	BSS_INFO_HE_BASIC = 13,
	BSS_INFO_PROTECT_INFO = 14,
	BSS_INFO_OFFLOAD_PKT = 15,
	BSS_INFO_11V_MBSSID = 16,
	BSS_INFO_BCN_PROT = 17,
#ifdef HIGHPRI_RATE_SPECIFIC
	BSS_INFO_HIGHPRI_RATE_ARP = 18,
	BSS_INFO_HIGHPRI_RATE_DHCP = 19,
	BSS_INFO_HIGHPRI_RATE_EAPOL = 20,
#endif
#ifdef ZERO_LOSS_CSA_SUPPORT
	BSS_INFO_APCLI_TSF_SYNC = 0x1B,
#endif
	BSS_INFO_MAX_NUM
};

enum {
	BSS_INFO_OWN_MAC_FEATURE = (1 << BSS_INFO_OWN_MAC),
	BSS_INFO_BASIC_FEATURE = (1 << BSS_INFO_BASIC),
	BSS_INFO_RF_CH_FEATURE = (1 << BSS_INFO_RF_CH),
	BSS_INFO_PM_FEATURE = (1 << BSS_INFO_PM),
	BSS_INFO_UAPSD_FEATURE = (1 << BSS_INFO_UAPSD),
	BSS_INFO_ROAM_DETECTION_FEATURE = (1 << BSS_INFO_ROAM_DETECTION),
	BSS_INFO_LQ_RM_FEATURE = (1 << BSS_INFO_LQ_RM),
	BSS_INFO_EXT_BSS_FEATURE = (1 << BSS_INFO_EXT_BSS),
	BSS_INFO_BROADCAST_INFO_FEATURE = (1 << BSS_INFO_BROADCAST_INFO),
	BSS_INFO_SYNC_MODE_FEATURE = (1 << BSS_INFO_SYNC_MODE),
	BSS_INFO_RA_FEATURE = (1 << BSS_INFO_RA),
	BSS_INFO_HW_AMSDU_FEATURE = (1 << BSS_INFO_HW_AMSDU),
	BSS_INFO_BSS_COLOR_FEATURE = (1 << BSS_INFO_BSS_COLOR),
	BSS_INFO_HE_BASIC_FEATURE = (1 << BSS_INFO_HE_BASIC),
	BSS_INFO_PROTECT_INFO_FEATURE = (1 << BSS_INFO_PROTECT_INFO),
	BSS_INFO_OFFLOAD_PKT_FEATURE = (1 << BSS_INFO_OFFLOAD_PKT),
	BSS_INFO_11V_MBSSID_FEATURE = (1 << BSS_INFO_11V_MBSSID),
	BSS_INFO_BCN_PROT_FEATURE = (1 << BSS_INFO_BCN_PROT),
#ifdef HIGHPRI_RATE_SPECIFIC
	BSS_INFO_HIGHPRI_ARP_FEATURE = (1 << BSS_INFO_HIGHPRI_RATE_ARP),
	BSS_INFO_HIGHPRI_DHCP_FEATURE = (1 << BSS_INFO_HIGHPRI_RATE_DHCP),
	BSS_INFO_HIGHPRI_EAPOL_FEATURE = (1 << BSS_INFO_HIGHPRI_RATE_EAPOL),
#endif
#ifdef ZERO_LOSS_CSA_SUPPORT
	BSS_INFO_APCLI_TSF_SYNC_FEATURE = (1 << BSS_INFO_APCLI_TSF_SYNC),
#endif
	BSS_INFO_MAX_NUM_FEATURE = (1 << BSS_INFO_MAX_NUM)
};

#ifdef WIFI_MODULE_DVT
typedef enum _ENUM_MDVT_MODULE_T {
	MDVT_MODULE_WFARB = 0,
	MDVT_MODULE_AGG,
	MDVT_MODULE_DMA,
	MDVT_MODULE_WFMIMO,
	MDVT_MODULE_WFCTRL,
	MDVT_MODULE_WFETXBF,
	MDVT_MODULE_WFCFG,
	MDVT_MODULE_WFHIF,
	MDVT_MODULE_WFOFF,
	MDVT_MODULE_WFON,
	MDVT_MODULE_WFPF = 10,
	MDVT_MODULE_WFRMAC,
	MDVT_MODULE_WFUMAC_PLE,
	MDVT_MODULE_WFUMAC_PSE,
	MDVT_MODULE_WFUMAC_PP,
	MDVT_MODULE_WFUMAC_AMSDU,
	MDVT_MODULE_WFSEC,
	MDVT_MODULE_WFTMAC,
	MDVT_MODULE_WFTMAC_TXPWR,
	MDVT_MODULE_WFTXCR,
	MDVT_MODULE_WFMIB = 20,
	MDVT_MODULE_WFSYSON,
	MDVT_MODULE_WFLPON,
	MDVT_MODULE_WFINT,
	MDVT_MODULE_CONNCFG,
	MDVT_MODULE_MUCOP,
	MDVT_MODULE_WFMDP,
	MDVT_MODULE_WFRDM_PHYRX,
	MDVT_MODULE_WFRDM_PHYDFS,
	MDVT_MODULE_WFRDM_PHYRX_COMM,
	MDVT_MODULE_WFRDM_WTBLOFF = 30,
	MDVT_MODULE_PHYDFE_CTRL_WF_TSSI,
	MDVT_MODULE_PHYDFE_RFINTF_WF_CMM,
	MDVT_MODULE_PHYRX_CTRL_WF_COMM_RDD,
	MDVT_MODULE_PHYRX_CTRL_WF_COMM_CSI,
	MDVT_MODULE_PHYRX_CTRL_WF_COMM_CMM,
	MDVT_MODULE_PHYRX_CTRL_WF_COMM_TOAE,
	MDVT_MODULE_PHYRX_CSD_WF_COMM_CMM,
	MDVT_MODULE_PHYRX_POST_CMM,
	MDVT_MODULE_PHYDFS_WF_COMM_RDD,
	MDVT_MODULE_PHYRX_CTRL_TOAE = 40,
	MDVT_MODULE_PHYRX_CTRL_MURU,
	MDVT_MODULE_PHYRX_CTRL_RDD,
	MDVT_MODULE_PHYRX_CTRL_MULQ,
	MDVT_MODULE_PHYRX_CTRL_CMM,
	MDVT_MODULE_PHYRX_CTRL_CSI,
	MDVT_MODULE_PHYDFE_CTRL_PWR_REGU,
	MDVT_MODULE_PHYRX_CTRL_BF,
	MDVT_MODULE_PHYDFE_CTRL_CMM,
	MDVT_MODULE_WFRBIST,
	MDVT_MODULE_WTBL = 50,
	MDVT_MODULE_RX,
	MDVT_MODULE_LPON,
	MDVT_MODULE_MDP_RX,
	MDVT_MODULE_TXCMD,
	MDVT_MODULE_SEC_ECC,
	MDVT_MODULE_MIB,
	MDVT_MODULE_WFTWT,
	MDVT_MODULE_DRR,
	MDVT_MODULE_RUOFDMA,
	MDVT_MODULE_WFCMDRPTTX = 60,
	MDVT_MODULE_WFCMDRPT_TRIG,
	MDVT_MODULE_MLO,
	MDVT_MODULE_TXD,
	MDVT_MODULE_PH_TPUT,
	MDVT_MODULE_MAX,
} ENUM_MDVT_MODULE_T, *P_ENUM_MDVT_MODULE_T;

typedef struct _CMD_MDVT_TEST_T {
	UINT8	ucTestMode;
	UINT16	u2TestModule;
	UINT16  u2TestCaseIdx;
	UINT8	aucReserve[3];
} CMD_MDVT_TEST_T, *P_CMD_MDVT_TEST_T;
#endif

#define	UNDEFINED_VALUE_TBD	0

#ifdef DOT11_HE_AX
typedef struct GNU_PACKED _CMD_HE_SR_INFO_UPDATE_T {
	/* DWORD_0 - Common Part */
	UINT8  ucCmdVer;
	UINT8  aucPadding0[1];
	UINT16 u2CmdLen;       /* Cmd size including common part and body */

	/* DWORD_1 afterwards - Command Body */
	UINT8  ucBssIndex;
	UINT8  ucSRControl;
	UINT8  ucNonSRGObssPdMaxOffset;
	UINT8  ucSRGObssPdMinOffset;
	UINT8  ucSRGObssPdMaxOffset;
	UINT8  aucPadding1[3];
	UINT32 u4SRGBSSColorBitmapLow;
	UINT32 u4SRGBSSColorBitmapHigh;
	UINT32 u4SRGPartialBSSIDBitmapLow;
	UINT32 u4SRGPartialBSSIDBitmapHigh;

	UINT8  aucPadding2[32];
} CMD_HE_SR_INFO_UPDATE_T, *P_CMD_HE_SR_INFO_UPDATE_T;
#endif

typedef struct GNU_PACKED _CMD_BSSINFO_UPDATE_T {
	UINT8	ucBssIndex;
	UINT8	ucReserve;
	UINT16	u2TotalElementNum;
	UINT8	ucAppendCmdTLV;
	UINT8	aucReserve[3];
	UINT8	aucBuffer[];
} CMD_BSSINFO_UPDATE_T, *P_CMD_BSSINFO_UPDATE_T;

typedef struct GNU_PACKED _BSSINFO_CONNECT_OWN_DEV_T {
	/* BSS connect to own dev (Tag0) */
	UINT16	u2Tag;		/* Tag = 0x00 */
	UINT16	u2Length;
	UINT8	ucHwBSSIndex;
	UINT8	ucOwnMacIdx;
	UINT8  ucBandIdx;
	UINT8  aucReserve;
	UINT32	u4ConnectionType;
	UINT32	u4Reserved;
} CMD_BSSINFO_CONNECT_OWN_DEV_T, *P_CMD_BSSINFO_CONNECT_OWN_DEV_T;

typedef struct GNU_PACKED _BSSINFO_BASIC_T {
	/* Basic BSS information (Tag1) */
	UINT16	u2Tag;		/* Tag = 0x01 */
	UINT16	u2Length;
	UINT32	u4NetworkType;
	UINT8	ucActive;
	UINT8	ucReserve0;
	UINT16	u2BcnInterval;
	UINT8	aucBSSID[6];
	UINT8	ucWmmIdx;
	UINT8	ucDtimPeriod;
	UINT8	ucBmcWlanIdxL;			/* indicate which wlan-idx used for MC/BC transmission. */
	UINT8	ucCipherSuit;
	UINT8   ucPhyMode;
	UINT8	uc11vMaxBssidIndicator;	/* Max BSSID indicator. Range from 1 to 8, 0 means MBSSID disabled */
	UINT8	uc11vBssidIdx;			/* BSSID index of non-transmitted BSSID, 0 means transmitted BSSID */
	UINT8	ucBmcWlanIdxHnVer;		/* High Byte and Version */
	UINT8   acuReserve[2];
} CMD_BSSINFO_BASIC_T, *P_CMD_BSSINFO_BASIC_T;

typedef struct GNU_PACKED _BSSINFO_RF_CH_T {
	/* RF channel (Tag2) */
	UINT16	u2Tag;		/* Tag = 0x02 */
	UINT16	u2Length;
	UINT8	ucPrimaryChannel;
	UINT8	ucCenterChannelSeg0;
	UINT8	ucCenterChannelSeg1;
	UINT8	ucBandwidth;
	UINT8   ucHetbRU26Disable;  /* 1 means dont send HETB in RU26, 0 means allow */
	UINT8   ucHetbAllDisable; /* 1 means disallow all HETB, 0 means allow */
	UINT8   acuReserve[2];
} CMD_BSSINFO_RF_CH_T, *P_CMD_BSSINFO_RF_CH_T;

typedef struct GNU_PACKED _BSSINFO_PM_T {
	/* Power management (Tag3) */
	UINT16	u2Tag;		/* Tag = 0x03 */
	UINT16	u2Length;
	UINT8	ucKeepAliveEn;
	UINT8	ucKeepAlivePeriod;		/* unit is second */
	UINT8	ucBeaconLossReportEn;
	UINT8	ucBeaconLossCount;
	UINT8	ucBcnSpState0Min;
	UINT8	ucBcnSpState0Max;
	UINT8	ucBcnSpState1Min;
	UINT8	ucBcnSpState1Max;
	UINT8	ucBcnSpState2Min;
	UINT8	ucBcnSpState2Max;
	UINT8	ucBcnSpState3Min;
	UINT8	ucBcnSpState3Max;
	UINT8	ucBcnSpState4Min;
	UINT8	ucBcnSpState4Max;
	UINT16	u2Reserve;
} CMD_BSSINFO_PM_T, *P_CMD_BSSINFO_PM_T;

typedef struct GNU_PACKED _BSSINFO_UAPSD_T {
	/* UAPSD offload (Tag4) */
	UINT16	u2Tag;		/* Tag = 0x04 */
	UINT16	u2Length;
	UINT8	ucIsUapsdSupported;
	UINT8	ucUapsdTriggerAC;
	UINT8	ucUapsdDeliveryAC;
	UINT8	ucReportSpToEvent;
	UINT16	u2UapsdServicePeriodTO;   /* unit is second */
	UINT16	u2Reserve;
} CMD_BSSINFO_UAPSD_T, *P_CMD_BSSINFO_UAPSD_T;

typedef struct GNU_PACKED _BSSINFO_RSSI_RM_DET_T {
	/* RSSI Roaming Detection (Tag5) */
	UINT16	u2Tag;				/* Tag = 0x05 */
	UINT16	u2Length;
	UINT8	fgEnable;				/* Enable the Packet RSSI Detection( and moving average) */
	UINT8	ucPktSource;			/* Packet Seletction */
	UINT8	ucPktMAPara;			/* Moving  Average Parameter for Received Packets */
	INT8		cRssiCCKLowThr;		/* input in RSSI, required by driver */
	INT8		cRssiCCKHighThr;		/* input in RSSI, required by driver */
	INT8		cRssiOFDMLowThr;		/* input in RSSI, required by driver */
	INT8		cRssiOFDMHighThr;		/* input in RSSI, required by driver */
	UINT8	ucReserved0;
} CMD_BSSINFO_RSSI_RM_DET_T, *P_CMD_BSSINFO_RSSI_RM_DET_T;

/* Extension BSS information (Tag7) */
typedef struct GNU_PACKED _BSSINFO_EXT_BSS_INFO_T {
	UINT16 u2Tag;              /* Tag = 0x07 */
	UINT16 u2Length;
	UINT32 ucMbssTsfOffset;
	UINT8  aucReserved[8];
} CMD_BSSINFO_EXT_BSS_INFO_T, *P_BSSINFO_EXT_BSS_INFO_T;

/* Extension BSS information (Tag8) */
typedef struct GNU_PACKED _BSSINFO_BMC_RATE_T {
	/* Broad Mcast Frame Rate (Tag8) */
	UINT16 u2Tag;              /* Tag = 0x08 */
	UINT16 u2Length;
	UINT16 u2BcTransmit;
	UINT16 u2McTransmit;
	UINT8 ucPreambleMode;
	UINT8 aucReserved[7];
} CMD_BSSINFO_BMC_RATE_T, *P_BSSINFO_BMC_RATE_T;

/* Sync Mode control (Tag9) */
typedef struct GNU_PACKED _BSSINFO_SYNC_MODE_CTRL_T {
	UINT16 u2Tag;              /* Tag = 0x09 */
	UINT16 u2Length;
	UINT16 u2BcnInterval;
	UINT8  fgIsEnableSync;
	UINT8  ucDtimPeriod;
	UINT8  aucReserved[8];
} CMD_BSSINFO_SYNC_MODE_CTRL_T, *P_BSSINFO_SYNC_MODE_CTRL_T;

/* Hw AMSDU global information (Tag11) */
typedef struct GNU_PACKED _CMD_BSSINFO_HW_AMSDU_INFO_T {
	UINT16 u2Tag;              /* Tag = 0xb */
	UINT16 u2Length;
	UINT32 u4TxdCmpBitmap_0;
	UINT32 u4TxdCmpBitmap_1;
	UINT16 u2TxdTriggerThres;
	UINT8  fgHwAmsduEn;
	UINT8  aucReserved[1];
} CMD_BSSINFO_HW_AMSDU_INFO_T, *P_CMD_BSSINFO_HW_AMSDU_INFO_T;

/* BSS Color information (Tag12) */
typedef struct GNU_PACKED _CMD_BSSINFO_BSS_COLOR_T {
	UINT16 u2Tag;       /* Tag = 0x0c */
	UINT16 u2Length;
	UINT8  fgIsDisable;
	UINT8  ucBssColor;
	UINT8  aucReserved[2];
} CMD_BSSINFO_BSS_COLOR_T, *P_CMD_BSSINFO_BSS_COLOR_T;

typedef struct GNU_PACKED _BSSINFO_HE_BASIC_T {
	UINT16	u2Tag;		/* Tag = 0x0d */
	UINT16	u2Length;
	UINT8	ucDefaultPEDuration;
	UINT8	ucVhtOperInfoPresent;
	UINT16	u2TxopDurationRtsThreshold;
	UINT16	au2MaxNssMcs[CMD_HE_MCS_BW_NUM];
	UINT8	aucReserved[6];
} CMD_BSSINFO_HE_BASIC_T, *P_CMD_BSSINFO_HE_BASIC_T;

typedef struct GNU_PACKED _BSSINFO_PROT_INFO_T {
	UINT16	u2Tag;		/* Tag = 0x0e */
	UINT16	u2Length;
	UINT32	u4ProtectUpdateType;
	UINT32	u4ProtectMode;
	UINT32	u4RtsLengthThld;
	UINT16	u2TxopDurRtsThld;
	UINT8	ucRtsPktCntThld;
	UINT8	aucReserved[5];
} CMD_BSSINFO_PROT_INFO_T, *P_CMD_BSSINFO_PROT_INFO_T;

/* BSS offload packet information (Tag15) */
typedef struct GNU_PACKED _CMD_BSSINFO_OFFLOAD_PKT_T {
	UINT16	u2Tag;			/* Tag = 0x0f */
	UINT16	u2Length;
	UINT8	ucVer;			/* Cmd Version */
	UINT8	fgEnable;
	UINT16	u2SubElementNum;
	UINT8	aucBuffer[];
} CMD_BSSINFO_OFFLOAD_PKT_T, *P_CMD_BSSINFO_OFFLOAD_PKT_T;

#ifdef HIGHPRI_RATE_SPECIFIC
typedef struct _BSSINFO_HIGHPRI_RATE_T {
	/* High priority Frame Rate (Tag = 0x18, 0x19, 0x20) */
	UINT16 u2Tag;
	UINT16 u2Length;
	UINT16 u2HighPriTransmit;
	UINT8 ucPreambleMode;
	UINT8 aucReserved[1];
} CMD_BSSINFO_HIGHPRI_RATE_T, *P_CMD_BSSINFO_HIGHPRI_RATE_T;
#endif

enum ENUM_SUB_TAG_OFFLOAD_PKT_CMD {
	SUB_TAG_BCN_CSA				= 0,
	SUB_TAG_BCN_BCC				= 1,
	SUB_TAG_BCN_MBSSID			= 2,
	SUB_TAG_BCN_CONTENT			= 3,
	SUB_TAG_UNSOL_OFFLOAD_PKT	= 4,
	SUB_TAG_BCN_BTWT		= 5,
	SUB_TAG_BCN_MAX_NUM
};

enum offload_txtype {
	OFFLOAD_TX_PROBE_RSP,
	OFFLOAD_TX_FILS_DISC,
	OFFLOAD_TX_QOS_NULL
};

typedef struct GNU_PACKED _CMD_OFFLOAD_BCN_CSA_T {
	UINT16	u2SubTag;		/* SubTag(SUB_TAG_BCN_CSA) = 0x0 */
	UINT16	u2Length;
	UINT8	ucCsaCount;
	UINT8	ucReserve[3];
} CMD_OFFLOAD_BCN_CSA_T, *P_CMD_OFFLOAD_BCN_CSA_T;

typedef struct GNU_PACKED _CMD_OFFLOAD_BCN_BCC_T {
	UINT16	u2SubTag;		/* SubTag(SUB_TAG_BCN_BCC) = 0x1 */
	UINT16	u2Length;
	UINT8	ucBccCount;
	UINT8	ucReserve[3];
} CMD_OFFLOAD_BCN_BCC_T, *P_CMD_OFFLOAD_BCN_BCC_T;

typedef struct GNU_PACKED _CMD_OFFLOAD_BCN_MBSSID_T {
	UINT16	u2SubTag;		/* SubTag(SUB_TAG_BCN_BCC) = 0x2 */
	UINT16	u2Length;
	UINT32	u4Dot11vMbssidBitmap;
	UINT16	u2MbssidIeOffset[MAX_BEACON_NUM];
	UINT8	aucReserve[8];
} CMD_OFFLOAD_BCN_MBSSID_T, *P_CMD_OFFLOAD_BCN_MBSSID_T;

typedef struct GNU_PACKED _CMD_OFFLOAD_BCN_CONTENT_T {
	UINT16	u2SubTag;		/* SubTag(SUB_TAG_BCN_CONTENT) = 0x3 */
	UINT16	u2Length;
	UINT16	u2TimIeOffset;
	UINT16	u2CsaIeOffset;
	UINT16	u2BccIeOffset;
	UINT16	u2BcnLength;	/* Length: TXD + Beacon */
	UINT8	aucBcnContent[];
} CMD_OFFLOAD_BCN_CONTENT_T, *P_CMD_OFFLOAD_BCN_CONTENT_T;

#define PROBE_RSP_TX_MODE_SU	BIT(0)
#define PROBE_RSP_TX_MODE_MU	BIT(1)

typedef struct GNU_PACKED _CMD_OFFLOAD_UNSOL_PKT_T {
	UINT16	u2SubTag;			/* SubTag(SUB_TAG_UNSOL_OFFLOAD_PKT) = 0x4 */
	UINT16	u2Length;
	UINT8	ucTxType;
	UINT8	ucTxMode;			/* SU/MU/SU+MU */
	UINT8	ucTxInterval;		/* in unit of TU */
	UINT8	fgEnable;
	UINT16	u2Reserved;			/* reserved for unicast wcid */
	UINT16	u2ProbeRspLen;		/* Length: TXD + Probe.Rsp */
	UINT8	aucProbeRspContent[];
} CMD_OFFLOAD_UNSOL_PKT_T, *P_CMD_OFFLOAD_UNSOL_PKT_T;

typedef struct GNU_PACKED _CMD_OFFLOAD_BCN_BTWT_T {
	UINT16	u2SubTag;		/* SubTag(SUB_TAG_BCN_BTWT) = 0x5 */
	UINT16	u2Length;
	UINT16	u2bTWTIeOffset;
	UINT8	aucReserve[2];
} CMD_OFFLOAD_BCN_BTWT_T, *P_CMD_OFFLOAD_BCN_BTWT_T;

#ifdef DOT11V_MBSSID_SUPPORT
typedef struct _BSSINFO_11V_MBSSID_T {
	UINT16  u2Tag;  /* Tag = 0x10 */
	UINT16  u2Length;
	UINT8   ucMaxBSSIDIndicator;
	UINT8   ucMBSSIDIndex;
	UINT8   aucReserved[2];
} CMD_BSSINFO_11V_MBSSID_T, *P_CMD_BSSINFO_11V_MBSSID_T;
#endif

#ifdef BCN_PROTECTION_SUPPORT
typedef struct _BSSINFO_BCN_PROT_T {
    UINT16 u2Tag;      /* Tag = 0x11 */
    UINT16 u2Length;
    UINT8 aucBcnProtPN[6];
    UINT8 ucBcnProtEnabled;
    UINT8 ucBcnProtCipherId;
    UINT8 aucBcnProtKey[32];
    UINT8 ucBcnProtKeyId;
    UINT8 aucReserved[3];
} CMD_BSSINFO_BCN_PROT_T, *P_BSSINFO_BCN_PROT_T;

#define BSS_INFO_BCN_PROT_EN_OFF     0
#define BSS_INFO_BCN_PROT_EN_SW_MODE 1
#define BSS_INFO_BCN_PROT_EN_HW_MODE 2
#endif

#ifdef ZERO_LOSS_CSA_SUPPORT
typedef struct _CMD_BSSINFO_APCLI_TSF_TO_AP_T {
    UINT_16 u2Tag;   /* Tag = 0x1B */
    UINT_16 u2Length;
} CMD_BSSINFO_APCLI_TSF_TO_AP_T, *P_CMD_BSSINFO_APCLI_TSF_TO_AP_T;
#endif

typedef struct _BSS_INFO_HANDLE_T {
	UINT32 BssInfoTag;
	VOID (*BssInfoTagHandler)(
		struct _RTMP_ADAPTER *pAd,
		struct _BSS_INFO_ARGUMENT_T *bss_info_argument,
		struct cmd_msg *msg);
} BSS_INFO_HANDLE_T, *P_BSS_INFO_HANDLE_T;

/* WTBL */

/**
 * @addtogroup wtbl
 * @{
 * @name wtbl TLV
 * @{
 */
enum WTBL_TLV {
	WTBL_GENERIC = 0,
	WTBL_RX = 1,
	WTBL_HT = 2,
	WTBL_VHT = 3,
	WTBL_PEER_PS = 4,
	WTBL_TX_PS = 5,
	WTBL_HDR_TRANS = 6,
	WTBL_SECURITY_KEY = 7,
	WTBL_BA = 8,
	WTBL_RDG = 9,
	WTBL_PROTECTION = 10,
	WTBL_CLEAR = 11,
	WTBL_BF = 12,
	WTBL_SMPS = 13,
	WTBL_RAW_DATA_RW = 14,
	WTBL_PN = 15,
	WTBL_SPE = 16,
	WTBL_SECURITY_KEY_V2 = 17,
	WTBL_RATE = 18,
	WTBL_PWR_OFFSET = 19,
	WTBL_DUMP = 20,
	WTBL_HDRT_MODE = 21,
	WTBL_MAX_NUM = 22,
};
/** @} */
/** @} */

typedef struct _CMD_WTBL_UPDATE_T {
	UINT8	ucWlanIdxL;		/* #256STA - Low Byte */
	UINT8	ucOperation;
	UINT16	u2TotalElementNum;
	UINT8	ucWlanIdxHnVer;	/* #256STA - High Byte and Version */
	UINT8	u4Reserve[3];
	UINT8	aucBuffer[];
} CMD_WTBL_UPDATE_T, *P_CMD_WTBL_UPDATE_T;

enum {
	RESET_WTBL_AND_SET	= 1,
	SET_WTBL				= 2,
	QUERY_WTBL			= 3,
	RESET_ALL_WTBL		= 4,
};

typedef struct GNU_PACKED _WTBL_GENERIC_TLV_T {
	UINT16	u2Tag;
	UINT16	u2Length;
	UINT8	aucBuffer[];
} CMD_WTBL_GENERIC_TLV_T, *P_CMD_WTBL_GENERIC_TLV_T;

typedef struct GNU_PACKED _WTBL_GENERIC_T {
	UINT16	u2Tag;		/* Tag = 0x00 */
	UINT16	u2Length;
	UINT8	aucPeerAddress[6];
	UINT8	ucMUARIndex;
	UINT8	ucSkipTx;
	UINT8	ucCfAck;
	UINT8	ucQos;
	UINT8	ucMesh;
	UINT8	ucAdm;
	UINT16	u2PartialAID;
	UINT8	ucBafEn;
	UINT8   ucAadOm;
} CMD_WTBL_GENERIC_T, *P_CMD_WTBL_GENERIC_T;

typedef struct GNU_PACKED _WTBL_RX_T {
	UINT16	u2Tag;		/* Tag = 0x01 */
	UINT16	u2Length;
	UINT8	ucRcid;
	UINT8	ucRca1;
	UINT8	ucRca2;
	UINT8	ucRv;
	UINT8	aucReserved[4];
} CMD_WTBL_RX_T, *P_CMD_WTBL_RX_T;

typedef struct GNU_PACKED _WTBL_HT_T {
	UINT16	u2Tag;		/* Tag = 0x02 */
	UINT16	u2Length;
	UINT8	ucHt;
	UINT8	ucLdpc;
	UINT8	ucAf;
	UINT8	ucMm;
} CMD_WTBL_HT_T, *P_CMD_WTBL_HT_T;

typedef struct GNU_PACKED _WTBL_VHT_T {
	UINT16	u2Tag;		/* Tag = 0x03 */
	UINT16	u2Length;
	UINT8	ucLdpcVht;
	UINT8	ucDynBw;
	UINT8	ucVht;
	UINT8	ucTxopPsCap;
} CMD_WTBL_VHT_T, *P_CMD_WTBL_VHT_T;

typedef struct GNU_PACKED _WTBL_PEER_PS_T {
	UINT16	u2Tag;		/* Tag = 0x04 */
	UINT16	u2Length;
	UINT8	ucDuIPsm;
	UINT8	ucIPsm;
	UINT8	ucPsm;/*Psm bit only can be set by HW, the field is used for debug purpose.*/
	UINT8	ucRsvd1;
} CMD_WTBL_PEER_PS_T, *P_CMD_WTBL_PEER_PS_T;

typedef struct GNU_PACKED _WTBL_TX_PS_T {
	UINT16	u2Tag;		/* Tag = 0x05 */
	UINT16	u2Length;
	UINT8	ucTxPs;
	UINT8	ucRsvd0;
	UINT8	ucRsvd1;
	UINT8	ucRsvd2;
} CMD_WTBL_TX_PS_T, *P_CMD_WTBL_TX_PS_T;

typedef struct GNU_PACKED _WTBL_HDR_TRANS_T {
	UINT16	u2Tag;		/* Tag = 0x06 */
	UINT16	u2Length;
	UINT8	ucTd;
	UINT8	ucFd;
	UINT8	ucDisRhtr;
	UINT8	ucRsvd0;
} CMD_WTBL_HDR_TRANS_T, *P_CMD_WTBL_HDR_TRANS_T;

typedef struct GNU_PACKED _WTBL_SECURITY_KEY_T {
	UINT16	u2Tag;		/* Tag = 0x07 */
	UINT16	u2Length;
	UINT8	ucAddRemove; /* 0: add, 1: remove */
	/* UINT8	ucKeyType; */	/* 0: SHAREDKEYTABLE, 1: PAIRWISEKEYTABLE */
	UINT8	ucRkv;
	UINT8	ucIkv;
	UINT8	ucAlgorithmId; /* refer to ENUM_CIPHER_SUIT_T256 */
	UINT8	ucKeyId;
	UINT8	ucKeyLen;
	UINT8	ucrRsvd0;
	UINT8	ucrRsvd1;
	UINT8	aucKeyMaterial[32];
} CMD_WTBL_SECURITY_KEY_T, *P_CMD_WTBL_SECURITY_KEY_T;

enum ENUM_SEC_CIPHER_ID_T {
	SEC_CIPHER_ID_NONE         = 0,
	SEC_CIPHER_ID_WEP40        = 1,
	SEC_CIPHER_ID_WEP104       = 2,
	SEC_CIPHER_ID_WEP128       = 3,
	SEC_CIPHER_ID_TKIP         = 4,
	SEC_CIPHER_ID_CCMP128      = 5,
	SEC_CIPHER_ID_CCMP256      = 6,
	SEC_CIPHER_ID_GCMP128      = 7,
	SEC_CIPHER_ID_GCMP256      = 8,
	SEC_CIPHER_ID_WPI_SMS4     = 9,
	SEC_CIPHER_ID_BIP_CMAC_128 = 10,
	SEC_CIPHER_ID_BIP_CMAC_256 = 11,
};

#define MAX_STA_REC_SEC_KEY_CMD_SIZE (sizeof(CMD_WTBL_SECURITY_KEY_V2_T) + sizeof(CMD_WTBL_SEC_CIPHER_AES_T) + 2 * sizeof(CMD_WTBL_SEC_CIPHER_BIP_T))

#define WTBL_SEC_KEY_METHOD 0
#define STAREC_SEC_KEY_METHOD 1

#define CMD_SEC_KEY_ADD_KEY_OP 0
#define CMD_SEC_KEY_REMOVE_KEY_OP 1


typedef struct GNU_PACKED _CMD_WTBL_SECURITY_KEY_V2_T {
	UINT16 u2Tag;
	UINT16 u2Length; /* Length = total cmd size */
	UINT8  ucAddRemove;
	UINT8  ucEntryCount;
	UINT8  ucReserved[2];
	UINT8  aucBuffer[];
} CMD_WTBL_SECURITY_KEY_V2_T, *P_CMD_WTBL_SECURITY_KEY_V2_T;

typedef struct GNU_PACKED _CMD_WTBL_SEC_CIPHER_GENERAL_T {
	UINT8 ucCipherId;
	UINT8 ucSubLength; /* Length = total cipher subcmd structure size */
	UINT8 ucKeyIdx;
	UINT8 ucKeyLength;
	UINT8 aucKeyMaterial[32];
} CMD_WTBL_SEC_CIPHER_GENERAL_T, *P_CMD_WTBL_SEC_CIPHER_GENERAL_T;

typedef struct GNU_PACKED _CMD_WTBL_SEC_CIPHER_WEP_T {
	UINT8 ucCipherId;
	UINT8 ucSubLength; /* Length = total cipher subcmd structure size */
	UINT8 ucKeyIdx;
	UINT8 ucKeyLength;
	UINT8 aucKeyMaterial[32];
} CMD_WTBL_SEC_CIPHER_WEP_T, *P_CMD_WTBL_SEC_CIPHER_WEP_T;

typedef struct GNU_PACKED _CMD_WTBL_SEC_CIPHER_TKIP_T {
	UINT8 ucCipherId;
	UINT8 ucSubLength; /* Length = total cipher subcmd structure size */
	UINT8 ucKeyIdx;
	UINT8 ucKeyLength;
	UINT8 aucKeyMaterial[16];
	UINT8 aucRxMicKey[LEN_TKIP_MIC];
	UINT8 aucTxMicKey[LEN_TKIP_MIC];
} CMD_WTBL_SEC_CIPHER_TKIP_T, *P_CMD_WTBL_SEC_CIPHER_TKIP_T;

typedef struct GNU_PACKED _CMD_WTBL_SEC_CIPHER_AES_T {
	UINT8 ucCipherId;
	UINT8 ucSubLength; /* Length = total cipher subcmd structure size */
	UINT8 ucKeyIdx;
	UINT8 ucKeyLength;
	UINT8 aucKeyMaterial[32];
}  CMD_WTBL_SEC_CIPHER_AES_T, *P_CMD_WTBL_SEC_CIPHER_AES_T;

typedef struct GNU_PACKED _CMD_WTBL_SEC_CIPHER_SMS4_T {
	UINT8 ucCipherId;
	UINT8 ucSubLength; /* Length = total cipher subcmd structure size */
	UINT8 ucKeyIdx;
	UINT8 ucKeyLength;
	UINT8 aucKeyMaterial[16];
	UINT8 aucRxMicKey[LEN_WPI_MIC];
	UINT8 aucTxMicKey[LEN_WPI_MIC];
} CMD_WTBL_SEC_CIPHER_SMS4_T, *P_CMD_WTBL_SEC_CIPHER_SMS4_T;

typedef struct GNU_PACKED _CMD_WTBL_SEC_CIPHER_BIP_T {
	UINT8 ucCipherId;
	UINT8 ucSubLength; /* Length = total cipher subcmd structure size */
	UINT8 ucKeyIdx;
	UINT8 ucKeyLength;
	UINT8 aucKeyMaterial[32];
} CMD_WTBL_SEC_CIPHER_BIP_T, *P_CMD_WTBL_SEC_CIPHER_BIP_T;



typedef struct GNU_PACKED _WTBL_BA_T {
	UINT16	u2Tag;		/* Tag = 0x08 */
	UINT16	u2Length;
	/* Recipient + Originator */
	UINT8	ucTid;
	UINT8	ucBaSessionType;
	UINT8	aucReserved[2];
	/* Originator */
	UINT16	u2Sn;
	UINT8	ucBaEn;
	UINT8	ucBaWinSizeIdx;
	/* Originator & Recipient */
	UINT16	u2BaWinSize;
	/* Recipient */
	UINT8	aucPeerAddress[MAC_ADDR_LEN];
	UINT8	ucRstBaTid;
	UINT8	ucRstBaSel;
	UINT8	ucStartRstBaSb;
	UINT8	ucBandIdx;
	UINT8	aucReserved2[4];
} CMD_WTBL_BA_T, *P_CMD_WTBL_BA_T;

typedef struct GNU_PACKED _WTBL_RDG_T {
	UINT16	u2Tag;		/* Tag = 0x09 */
	UINT16	u2Length;
	UINT8	ucRdgBa;
	UINT8	ucR;
	UINT8	ucrRsvd0;
	UINT8	ucrRsvd1;
} CMD_WTBL_RDG_T, *P_CMD_WTBL_RDG_T;

typedef struct GNU_PACKED _WTBL_PROTECTION_T {
	UINT16	u2Tag;		/* Tag = 0x0a */
	UINT16	u2Length;
	UINT8	ucRts;
	UINT8	ucrRsvd0;
	UINT8	ucrRsvd1;
	UINT8	ucrRsvd2;
} CMD_WTBL_PROTECTION_T, *P_CMD_WTBL_PROTECTION_T;

/* bit 0: Clear PSM (WF_WTBLON: 0x60322300, Bit 31 set 1 then set 0) */
/* bit 1: Clear BA (WTBL2.DW15) */
/* bit 2: Clear Rx Counter (6019_00002, bit 14) */
/* bit 3: Clear Tx Counter (6019_0000, bit 15) */
/* bit 4: Clear ADM Counter (6019_0000, bit 12) */
/* bit 5: Clear Cipher key (WTBL3)*/

typedef struct GNU_PACKED _WTBL_CLEAR_T {
	UINT16	u2Tag;		/* Tag = 0x0b */
	UINT16	u2Length;
	UINT8	ucClear;
	UINT8	ucrRsvd0;
	UINT8	ucrRsvd1;
	UINT8	ucrRsvd2;
} CMD_WTBL_CLEAR_T, *P_CMD_WTBL_CLEAR_T;

typedef struct GNU_PACKED _WTBL_BF_T {
	UINT16	u2Tag;		/* Tag = 0x0c */
	UINT16	u2Length;
	UINT8	ucTiBf;
	UINT8	ucTeBf;
	UINT8	ucTibfVht;
	UINT8	ucTebfVht;
	UINT8	ucGid;
	UINT8	ucPFMUIdx;
	UINT8	ucrRsvd1;
	UINT8	ucrRsvd2;
} CMD_WTBL_BF_T, *P_CMD_WTBL_BF_T;

typedef struct GNU_PACKED _WTBL_SMPS_T {
	UINT16	u2Tag;		/* Tag = 0x0d */
	UINT16	u2Length;
	UINT8	ucSmPs;
	UINT8	ucrRsvd0;
	UINT8	ucrRsvd1;
	UINT8	ucrRsvd2;
} CMD_WTBL_SMPS_T, *P_CMD_WTBL_SMPS_T;

typedef struct GNU_PACKED _WTBL_RAW_DATA_RW_T {
	UINT16	u2Tag;		/* Tag = 0x0e */
	UINT16	u2Length;
	UINT8	ucWtblIdx;           /* WTBL 1/2/3/4 in MT7636, the field don't care in MT7615 */
	UINT8	ucWhichDW;
	UINT8	aucReserve[2];
	UINT32	u4DwMask;		/* Show these bits don't be writen */
	UINT32	u4DwValue;
} CMD_WTBL_RAW_DATA_RW_T, *P_CMD_WTBL_RAW_DATA_RW_T;

typedef struct GNU_PACKED _CMD_WTBL_PN_T {
	UINT16 u2Tag;      /* Tag = 0x0F */
	UINT16 u2Length;
	UINT8  aucPn[6];
	UINT8 ucTscType; /*BIT0 for GTK_PN, BIT1 for IPN, BIT2 for BIPN*/
	UINT8 ucReserved;
} CMD_WTBL_PN_T, *P_CMD_WTBL_PN_T;

enum {
	TSC_TYPE_GTK_PN = 0,
	TSC_TYPE_IGTK_PN = 1,
	TSC_TYPE_BIGTK_PN = 2,
	MAX_TSC_TYPE = 3
};

enum {
	TSC_TYPE_GTK_PN_MASK = (1 << TSC_TYPE_GTK_PN),
	TSC_TYPE_IGTK_PN_MASK = (1 << TSC_TYPE_IGTK_PN),
	TSC_TYPE_BIGTK_PN_MASK = (1 << TSC_TYPE_BIGTK_PN),
};


typedef struct GNU_PACKED _WTBL_SPE_T {
	UINT16	u2Tag;		/* Tag = 0x10 */
	UINT16	u2Length;
	UINT8	ucSpeIdx;
	UINT8	ucrRsvd0;
	UINT8	ucrRsvd1;
	UINT8	ucrRsvd2;
} CMD_WTBL_SPE_T, *P_CMD_WTBL_SPE_T;
#ifdef VLAN_SUPPORT
typedef struct GNU_PACKED _WTBL_HDRT_MODE {
	UINT16	u2Tag;		/* Tag = 0x15 */
	UINT16	u2Length;
	UINT8	ucEnable;
	UINT8	aucReserved[3];
} CMD_WTBL_HDRT_MODE_T, *P_CMD_WTBL_HDRT_MODE_T;
#endif
#if defined(MGMT_TXPWR_CTRL) || (defined(TPC_SUPPORT) && defined (TPC_MODE_CTRL))
typedef struct GNU_PACKED _WTBL_PWR_T {
	UINT16	u2Tag;		/* Tag = 0x17 */
	UINT16	u2Length;
	UINT8	ucPwrOffset;
	UINT8	ucrRsvd0;
	UINT8	ucrRsvd1;
	UINT8	ucrRsvd2;
} CMD_WTBL_PWR_T, *P_CMD_WTBL_PWR_T;
#endif
#ifdef MGMT_TXPWR_CTRL
typedef struct GNU_PACKED _WTBL_RATE_T {
	UINT16	u2Tag;		/* Tag = 0x18 */
	UINT16	u2Length;
	UINT16	u2Rate1;
	UINT16	u2Rate2;
	UINT16	u2Rate3;
	UINT16	u2Rate4;
	UINT16	u2Rate5;
	UINT16	u2Rate6;
	UINT16	u2Rate7;
	UINT16	u2Rate8;
} CMD_WTBL_RATE_T, *P_CMD_WTBL_RATE_T;
#endif

#define	MAX_BUF_SIZE_OF_WTBL_INFO	(sizeof(CMD_WTBL_UPDATE_T) + \
									 sizeof(CMD_WTBL_GENERIC_T) + \
									 sizeof(CMD_WTBL_RX_T) + \
									 sizeof(CMD_WTBL_HT_T) + \
									 sizeof(CMD_WTBL_VHT_T) + \
									 sizeof(CMD_WTBL_PEER_PS_T) + \
									 sizeof(CMD_WTBL_TX_PS_T) + \
									 sizeof(CMD_WTBL_HDR_TRANS_T) + \
									 MAX_STA_REC_SEC_KEY_CMD_SIZE + \
									 sizeof(CMD_WTBL_BA_T) + \
									 sizeof(CMD_WTBL_RDG_T) + \
									 sizeof(CMD_WTBL_PROTECTION_T) + \
									 sizeof(CMD_WTBL_CLEAR_T) + \
									 sizeof(CMD_WTBL_BF_T) + \
									 sizeof(CMD_WTBL_SMPS_T) + \
									 sizeof(CMD_WTBL_RAW_DATA_RW_T))

typedef struct GNU_PACKED _STAREC_WTBL_T {
	/* WTBL with STAREC update (Tag 0x0b) */
	/* STAREC format, content is WTBL format. */
	UINT16	u2Tag;
	UINT16	u2Length;
	UINT8	aucBuffer[MAX_BUF_SIZE_OF_WTBL_INFO];
} CMD_STAREC_WTBL_T, *P_CMD_STAREC_WTBL_T;

#define	WTBL_BUFFER_SIZE		0x100

#ifdef WTBL_TDD_SUPPORT
typedef struct GNU_PACKED _STAREC_UWTBL_RAW_T
{
    UINT_16 u2Tag;  /* Tag = 0x16 */
    UINT_16 u2Length;
    UINT_16 u2WtblIdx; /* ==> 2 bytes change */
    UINT_8   ucOp; /* query : 0 ; set : 1 */
    UINT_8   acuReserve[1];
    UINT_32 u4UWBLRaw[5]; /* PN & SN */
} CMD_STAREC_UWTBL_RAW_T, *P_CMD_STAREC_UWTBL_RAW_T;
#endif /* WTBL_TDD_SUPPORT */

typedef struct GNU_PACKED _WTBL_DUMP_T {
	UINT16	u2Tag;		/* Tag = 0x0f */
	UINT16	u2Length;
	UINT8	aucWtblBuffer[WTBL_BUFFER_SIZE];    /* need 4 byte alignment */
} CMD_WTBL_DUMP_T, *P_CMD_WTBL_DUMP_T;

typedef union _wtbl_debug_u {
	CMD_WTBL_GENERIC_T wtbl_generic_t;
	CMD_WTBL_RX_T wtbl_rx_t;
	CMD_WTBL_HT_T wtbl_ht_t;
	CMD_WTBL_VHT_T wtbl_vht_t;
	CMD_WTBL_PEER_PS_T wtbl_peer_ps_t;
	CMD_WTBL_TX_PS_T wtbl_tx_ps_t;
	CMD_WTBL_HDR_TRANS_T wtbl_hdr_trans_t;
	CMD_WTBL_SECURITY_KEY_T wtbl_sec_key_t;
	CMD_WTBL_BA_T wtbl_ba_t;
	CMD_WTBL_RDG_T wtbl_rdg_t;
	CMD_WTBL_PROTECTION_T wtbl_prot_t;
	CMD_WTBL_CLEAR_T wtbl_clear_t;
	CMD_WTBL_BF_T wtbl_bf_t;
	CMD_WTBL_SMPS_T wtbl_smps_t;
	CMD_WTBL_RAW_DATA_RW_T wtbl_raw_data_rw_t;
	CMD_WTBL_SPE_T wtbl_spe_t;
	CMD_WTBL_PN_T wtbl_pn_t;
} wtbl_debug_u;


#define SKU_SIZE 49

enum {
	SKU_CCK_1_2 = 0,
	SKU_CCK_55_11,
	SKU_OFDM_6_9,
	SKU_OFDM_12_18,
	SKU_OFDM_24_36,
	SKU_OFDM_48,
	SKU_OFDM_54,
	SKU_HT20_0_8,
	SKU_HT20_32,
	SKU_HT20_1_2_9_10,
	SKU_HT20_3_4_11_12,
	SKU_HT20_5_13,
	SKU_HT20_6_14,
	SKU_HT20_7_15,
	SKU_HT40_0_8,
	SKU_HT40_32,
	SKU_HT40_1_2_9_10,
	SKU_HT40_3_4_11_12,
	SKU_HT40_5_13,
	SKU_HT40_6_14,
	SKU_HT40_7_15,
	SKU_VHT20_0,
	SKU_VHT20_1_2,
	SKU_VHT20_3_4,
	SKU_VHT20_5_6,
	SKU_VHT20_7,
	SKU_VHT20_8,
	SKU_VHT20_9,
	SKU_VHT40_0,
	SKU_VHT40_1_2,
	SKU_VHT40_3_4,
	SKU_VHT40_5_6,
	SKU_VHT40_7,
	SKU_VHT40_8,
	SKU_VHT40_9,
	SKU_VHT80_0,
	SKU_VHT80_1_2,
	SKU_VHT80_3_4,
	SKU_VHT80_5_6,
	SKU_VHT80_7,
	SKU_VHT80_8,
	SKU_VHT80_9,
	SKU_VHT160_0,
	SKU_VHT160_1_2,
	SKU_VHT160_3_4,
	SKU_VHT160_5_6,
	SKU_VHT160_7,
	SKU_VHT160_8,
	SKU_VHT160_9,
	SKU_1SS_Delta,
	SKU_2SS_Delta,
	SKU_3SS_Delta,
	SKU_4SS_Delta,
};

enum {
	ENTRY_1 = 0,  /* 1T 1Nss */
	ENTRY_2,      /* 2T 1Nss */
	ENTRY_3,      /* 2T 2Nss */
	ENTRY_4,      /* 3T 1Nss */
	ENTRY_5,      /* 3T 2Nss */
	ENTRY_6,      /* 3T 3Nss */
	ENTRY_7,      /* 4T 1Nss */
	ENTRY_8,      /* 4T 2Nss */
	ENTRY_9,      /* 4T 3Nss */
	ENTRY_10,     /* 4T 4Nss */
};

enum CMD_CH_BAND {
	CMD_CH_BAND_24G = 0,
	CMD_CH_BAND_5G = 1,
	CMD_CH_BAND_6G = 2,
};

enum {
	CMD_BW_20,
	CMD_BW_40,
	CMD_BW_80,
	CMD_BW_160,
	CMD_BW_10,
	CMD_BW_5,
	CMD_BW_8080
};

typedef struct GNU_PACKED _EXT_CMD_CHAN_SWITCH_T {
	UINT8	ucPrimCh;
	UINT8	ucCentralCh;
	UINT8	ucBW;
	UINT8	ucTxStreamNum;

	UINT8	ucRxStreamNum;
	UINT8	ucSwitchReason;
	UINT8	ucDbdcIdx;
	UINT8	ucCentralCh2;

	UINT16	u2CacCase;
	UINT8	ucBand;
	UINT8	aucReserve0[1];

	UINT32  u4OutBandFreq;

	INT8	cTxPowerDrop;
	UINT8	ucAPBW;
	UINT8	ucAPCentralCh;
	UINT8	aucReserve1[1];

	INT8	acTxPowerSKU[SKU_TOTAL_SIZE];
	UINT8	aucReserve2[3];
} EXT_CMD_CHAN_SWITCH_T, *P_EXT_CMD_CHAN_SWITCH_T;


typedef struct _EXT_EVENT_ID_GET_TX_POWER_T {
	UINT8 u1EventCategoryID;
	UINT8 i1TargetPower;
	UINT8 u1BandIdx;
	UINT8 u1Reserved;
} EXT_EVENT_ID_GET_TX_POWER_T, *P_EXT_EVENT_ID_GET_TX_POWER_T;

typedef struct _EXT_CMD_GET_TX_POWER_T {
	UINT8   u1PowerCtrlFormatId;
	UINT8   u1DbdcIdx;
	UINT8   u1AntIdx;
	UINT8   u1CenterCh;
} EXT_CMD_GET_TX_POWER_T, *P_EXT_CMD_GET_TX_POWER_T;

typedef struct GNU_PACKED _EXT_CMD_TX_POWER_CTRL_T
{
	UINT8 u1PowerCtrlFormatId;
	UINT8 u1DbdcIdx;
	INT8  i1TargetPower;
	UINT8 u1AntIdx;
	UINT8 u1CenterChannel;
	UINT8 u1Reserved[3];
} EXT_CMD_TX_POWER_CTRL_T, *P_EXT_CMD_TX_POWER_CTRL_T;

typedef struct _CMD_POWER_RATE_TXPOWER_CTRL_T {
	UINT8 ucPowerCtrlFormatId;
	UINT8 ucPhyMode;
	UINT8 ucTxRate;
	UINT8 ucBW;
	UINT8 ucBandIdx;
	INT8  cTxPower;
	UINT8 ucReserved[2];
} CMD_POWER_RATE_TXPOWER_CTRL_T, *P_CMD_POWER_RATE_TXPOWER_CTRL_T;

#ifdef BACKGROUND_SCAN_SUPPORT

typedef struct GNU_PACKED _EXT_CMD_BGND_SCAN_NOTIFY_T {
	UINT8 ucNotifyFunc;
	UINT8 ucBgndScanStatus;
	UINT8 resv[2];
} EXT_CMD_BGND_SCAN_NOTIFY_T, *P_EXT_CMD_BGND_SCAN_NOTIFY_T;
#endif /* BACKGROUND_SCAN_SUPPORT */

typedef struct _INIT_EVENT_ACCESS_REG {
	UINT32 u4Address;
	UINT32 u4Data;
} INIT_EVENT_ACCESS_REG, *P_INIT_EVENT_ACCESS_REG;

#define CMD_RESULT_SUCCESS 0
#define CMD_RESULT_NONSUPPORT 254

typedef struct _INIT_EVENT_CMD_RESULT {
	UINT8 ucStatus;
	UINT8 ucCID;
	UINT8 aucReserved[2];
} INIT_EVENT_CMD_RESULT;

typedef struct GNU_PACKED _EVENT_EXT_CMD_RESULT_T {
	UINT8 ucExTenCID;
	UINT8 aucReserve[3];
	UINT32 u4Status;
} EVENT_EXT_CMD_RESULT_T, *PEVENT_EXT_CMD_RESULT_T;

typedef struct GNU_PACKED _EVENT_STAREC_UPDATE_T {
	UINT8   ucExtenCID;		/* Fix at 0x25 */
	UINT8   aucReserve[3];
	UINT32  u4Status;
	UINT8   ucBssInfoIdx;
	UINT8   ucWlanIdxL;		/* #256STA - Low Byte */
	UINT16  u2TotalElementNum;
	UINT8   ucMuarIdx;
	UINT8   ucWlanIdxHnVer;		/* #256STA - High Byte and Version */
	UINT8   aucReserved[2];
	UINT8   aucBuffer[];
} EVENT_STAREC_UPDATE_T, *P_EVENT_STAREC_UPDATE_T;

typedef struct GNU_PACKED _EVENT_BSSINFO_UPDATE_T {
	UINT8   ucExtenCID;		/* Fix at 0x26 */
	UINT8   aucReserve[3];
	UINT32  u4Status;
	UINT8	ucBssInfoIdx;
	UINT8	ucReserve;
	UINT16	u2TotalElementNum;
	UINT8	aucReserved[4];
	UINT8	aucBuffer[];
} EVENT_BSSINFO_UPDATE_T, *P_EVENT_BSSINFO_UPDATE_T;

typedef struct GNU_PACKED _EVENT_DEVINFO_UPDATE_T {
	UINT8   ucExtenCID;		/* Fix at 0x2A */
	UINT8   aucReserve[3];
	UINT32  u4Status;
	UINT8	ucOwnMacIdx;
	UINT8	ucReserve;
	UINT16	u2TotalElementNum;
	UINT8	aucReserved[4];
	UINT8	aucBuffer[];
} EVENT_DEVINFO_UPDATE_T, *P_EVENT_DEVINFO_UPDATE_T;

typedef struct GNU_PACKED _EXT_EVENT_NIC_CAPABILITY_T {
	UINT8 aucDateCode[16];
	UINT8 aucVersionCode[12];
} EXT_EVENT_NIC_CAPABILITY;

#ifdef MT_MAC
/* TODO: Star, fix me, the "RF_CR" is dupicated with andes_rlt.h */
enum {
	MAC_CR,
	RF_CR,
};
#endif /* MT_MAC */

typedef struct GNU_PACKED _CMD_MULTI_CR_ACCESS_T {
	UINT32 u4Type;
	UINT32 u4Addr;
	UINT32 u4Data;
} CMD_MULTI_CR_ACCESS_T;

typedef struct GNU_PACKED _CMD_MULTI_MIB_ACCESS_T {
	UINT32 u4Band;
	UINT32 u4Counter;
	UINT64 u8Data;
} CMD_MULTI_MIB_ACCESS_T;

typedef struct GNU_PACKED _EXT_EVENT_MULTI_CR_ACCESS_WR_T {
	UINT32 u4Status;
	UINT32 u4Resv;
	UINT32 u4Resv2;
} EXT_EVENT_MULTI_CR_ACCESS_WR_T;

typedef struct GNU_PACKED _EXT_EVENT_MULTI_CR_ACCESS_RD_T {
	UINT32 u4Type;
	UINT32 u4Addr;
	UINT32 u4Data;
} EXT_EVENT_MULTI_CR_ACCESS_RD_T;

typedef struct GNU_PACKED _EXT_EVENT_MULTI_MIB_ACCESS_RD_T {
	UINT32 u4Band;
	UINT32 u4Counter;
	UINT64 u8Data;
} EXT_EVENT_MULTI_MIB_ACCESS_RD_T;

enum {
	ANDES_LOG_DISABLE,
	ANDES_LOG_TO_UART,
	ANDES_LOG_TO_EVENT,
};

typedef struct GNU_PACKED _EXT_CMD_FW_LOG_2_HOST_CTRL_T {
	UINT8 ucFwLog2HostCtrl;
	UINT8 ucFwLog2HostInterval; /* For FwLog2Host Timer (second)*/
	UINT8 ucReserve[2];
} EXT_CMD_FW_LOG_2_HOST_CTRL_T;

#ifdef TXRX_STAT_SUPPORT
typedef struct _EXT_EVENT_STA_TX_STAT_RESULT_T {
	UINT32	PerStaTxPktCnt[MAX_LEN_OF_MAC_TABLE];
	UINT32	PerStaTxFailPktCnt[MAX_LEN_OF_MAC_TABLE];
	UINT8	ucEntryBitmap[16];
	UINT8	ucEntryCount;
	UINT8	aucReserved[3];
} EXT_EVENT_STA_TX_STAT_RESULT_T;

typedef struct _EXT_CMD_GET_STA_TX_STAT_T {
	UINT8	ucEntryBitmap[16];
	UINT8	ucEntryCount;
	UINT8	aucReserved[3];
} EXT_CMD_GET_STA_TX_STAT_T, *P_EXT_CMD_GET_STA_TX_STAT_T;
#endif

typedef struct _EXT_EVENT_EDCCA_T {
	UINT_8 u1CmdIdx;
	UINT_8 u1BandIdx;
	INT_8 i1CrVal[3];
	BOOLEAN fginit;
	UINT_8 aucReserve[2];
} EXT_EVENT_EDCCA_T, *P_EXT_EVENT_EDCCA_T;

#define IPI_ANT_NUM 8

typedef struct _EXT_EVENT_ENABLE_NOISE_FLOOR_T {
	UINT8 u1mode; /*0: idle power, 1: IPI */
	UINT8 au1reserved[3];
	UINT32 au4avgpwr[4];
	UINT32 au4avgPIHist[IPI_ANT_NUM][11] /* ant * ipi */;
} EXT_EVENT_ENABLE_NOISE_FLOOR_T, *P_EXT_EVENT_ENABLE_NOISE_FLOOR_T;

#ifdef WIFI_MD_COEX_SUPPORT
typedef struct _EXT_EVENT_FW2APCCCI_T {
	UINT32 pci_base_addr;
	UINT16 len;
	UINT8 pci_num;
	UINT8 pci_slot_id;
	UINT8 card_type;
	UINT8 data[0]; /* FW cmd length limit 1.6k bytes*/
} EXT_EVENT_FW2APCCCI_T, *P_EXT_EVENT_FW2APCCCI_T;

typedef struct _EVENT_LTE_SAFE_CHN_T {
	UINT_8	ucVersion;
	UINT_8	aucReserved1[3];
	UINT_32	u4Flags;
	UINT_32	u4SafeChannelBitmask[4];
} EVENT_LTE_SAFE_CHN_T, *P_EVENT_LTE_SAFE_CHN_T;
#endif /* WIFI_MD_COEX_SUPPORT */

typedef struct _EXT_CMD_ENABLE_NOISE_FLOOR_T {
	BOOLEAN fgEnable;
	UINT8 u1TimeOut;
	UINT8 u1Count;
	UINT8 u1EventCount;
} EXT_CMD_ENABLE_NOISE_FLOOR_T, *P_EXT_CMD_ENABLE_NOISE_FLOOR_T;

#ifdef SWACI_MECHANISM
typedef struct _EXT_CMD_DENSE_LNA_PARAM {
	INT8 i1MinSubRssi;
	INT8 i1MaxRcpi;
	UINT8 u1ReadCount;
	UINT8 u1MinDenseCount;
	UINT8 u1MaxDenseCount;
	UINT8 u1DenseCRValue;
	UINT8 au1Reserved[2];
} EXT_CMD_DENSE_LNA_PARAM, *P_EXT_CMD_DENSE_LNA_PARAM;

typedef struct _EXT_CMD_SWCR_PARAMS {
	UINT8  u1SWCR0;
	UINT8  u1SWCR1;
	UINT8  u1SWCR2;
	INT8   i1SWCR4;
} EXT_CMD_SWCR_PARAMS, *P_EXT_CMD_SWCR_PARAMS;

typedef struct _EXT_CMD_LNA_TIMER_ENABLE {
	UINT8 u1DbdcIdx;
	BOOLEAN fgEnable;
} EXT_CMD_LNA_TIMER_ENABLE, *P_EXT_CMD_LNA_TIMER_ENABLE;

typedef struct _EXT_CMD_SET_RCPI_TEST {
	UINT16 au2Rcpi[2][4];
} EXT_CMD_SET_RCPI_TEST, *P_EXT_CMD_SET_RCPI_TEST;

typedef struct _EXT_CMD_LNA_GAIN_ADJUST_PARAMS {
	INT8   i1AdcSetPointAci;
	INT8   i1AdcSetPointAAciBw20;
	INT8   i1AdcSetPointAAciBw40;
	INT8   i1AdcSetPointAAciBw80;
	INT8   i1AdcSetPointWanted;
	INT8   i1MaxRfGain;
	INT8   i1RfdgcSetPointAci;
	INT8   i1RfdgcSetPointWanted;
	INT8   i1MaxTotalGain;
	INT8   i1FarGainBoundLong;
	INT8   i1FarGainBoundShort;
	INT8   i1AdcSetPoint;
} EXT_CMD_LNA_GAIN_ADJUST_PARAMS, *P_EXT_CMD_LNA_GAIN_ADJUST_PARAMS;


typedef struct _EXT_CMD_LNA_TABLE_PARAMS {
	UINT8  u1PhyBw;
	INT8   i1LnaParam[5];
} EXT_CMD_LNA_TABLE_PARAMS, *P_EXT_CMD_LNA_TABLE_PARAMS;

typedef enum _ENUM_SWLNA_SUBCMD_ID {
	SWLNA_GAIN_ADJUST_PARAM = 0,
	SWLNA_CONDITION_PARAM,
	SWLNA_LNA_TABLE_PARAM,
	SWLNA_TIMER_START,
	SWLNA_RCPI_TESTMODE,
	SWLNA_TESTMODE,
	SWLNA_ENABLE,
	SWLNA_DENSE_PARAMS,
	SWLNA_THRESH_CONFIG,
	SWLNA_DBG_CONFIG,
	SWLNA_SUBCMD_NUM
} ENUM_SWLNA_SUBCMD_ID, *P_ENUM_SWLNA_SUBCMD_ID;
#endif
typedef enum _STA_STAT_EVENT_TYPE {
	EVENT_PHY_PER_STA_TX_RATE = 0x1,
    EVENT_PHY_PER_STA_TX_STAT = 0x2,
    EVENT_PHY_RX_STAT = 0x03,
    EVENT_PHY_PER_STA_TXRX_ADM_STAT = 0x04,
    EVENT_PHY_PER_STA_TXRX_AIR_TIME = 0x05,
    EVENT_PHY_PER_STA_DATA_TX_RETRY_COUNT = 0x06,
    EVENT_PHY_PER_STA_GI_MODE = 0x07
} STA_STAT_EVENT_TYPE, *P_STA_STAT_EVENT_TYPE;

typedef struct _EXT_CMD_GET_ALL_STA_STAT_T {
	UINT_8 ucEventType /*Sub-event to FW*/;
	UINT_8 aucReserved[3];
} EXT_CMD_GET_ALL_STA_STAT_T, *P_EXT_CMD_GET_ALL_STA_STAT_T;

#ifdef EAP_STATS_SUPPORT
#define TX_RATE_NUM_PER_EVENT                       75
#endif
#ifdef CONFIG_MAP_SUPPORT
#define TX_STAT_NUM_PER_EVENT                       75
#endif
#define TXRX_ADM_STA_NUM_PER_EVENT                  30
#define TXRX_AIR_TIME_NUM_PER_EVENT                 30
#define DATA_TX_RETRY_COUNT_NUM_PER_EVENT           300
#define GI_MODE_NUM_PER_EVENT                       375

#ifdef EAP_STATS_SUPPORT
typedef struct _EXT_EVENT_ONE_TX_STAT_T {
    UINT_16 u2WlanIdx;
    UINT_32 u4TotalTxCount;
    UINT_32 u4TotalTxFailCount;
} EXT_EVENT_ONE_TX_STAT_T, *P_EXT_EVENT_ONE_TX_STAT_T;

typedef struct _EXT_EVENT_TX_STAT_RESULT_T {
	UINT_8 u1PhyEventId;
	UINT_8 u1FlagMoreEvent;
	UINT_16 u2StaNum;
	EXT_EVENT_ONE_TX_STAT_T rTxStatResult[TX_RATE_NUM_PER_EVENT];
} EXT_EVENT_TX_STAT_RESULT_T, *P_EXT_EVENT_TX_STAT_RESULT_T;

typedef struct _EXT_EVENT_RX_STAT_T {
    UINT_16 u2PhyRxPdCck;
    UINT_16 u2PhyRxPdOfdm;
    UINT_16 u2PhyRxMdrdyCntCck;
    UINT_16 u2PhyRxMdrdyCntOfdm;
} EXT_EVENT_RX_STAT_T, *P_EXT_EVENT_RX_STAT_T;

typedef struct _EXT_EVENT_RX_STAT_RESULT_T {
    UINT_8 u1PhyEventId;
    UINT_8 u1Reserved[3];
    EXT_EVENT_RX_STAT_T rRxStatResult[DBDC_BAND_NUM];
} EXT_EVENT_RX_STAT_RESULT_T, *P_EXT_EVENT_RX_STAT_RESULT_T;

#endif

#ifdef CONFIG_MAP_SUPPORT
typedef struct _TX_RX_PHY_CFG_T {
    UINT_8  MODE;
    UINT_8  FLAGS;
    UINT_8  STBC;
    UINT_8  ShortGI;
    UINT_8  BW;
    UINT_8  ldpc;
    UINT_8  MCS;
    UINT_8  VhtNss;
    UINT_8  u1RxRate;
    UINT_8  u1RxMode;
    UINT_8  u1RxNsts;
    UINT_8  u1RxGi;
    UINT_8  u1RxCoding;
    UINT_8  u1RxStbc;
    UINT_8  u1RxBW;
    UINT_8  u1Reserved;
} TX_RX_PHY_CFG_T, *P_TX_RX_PHY_CFG_T;


typedef struct _ONE_TX_RATE_RESULT_T {
	UINT_16 ucWlanIdx;
	TX_RX_PHY_CFG_T rEntryTxRate;
} ONE_TX_RATE_RESULT_T, *P_ONE_TX_RATE_RESULT_T;

typedef struct _EXT_EVENT_TX_RATE_RESULT_T {
	UINT_8 u1PhyEventId;
	UINT_8 u2FlagMoreEvent;
	UINT_16 ucStaNum;
	ONE_TX_RATE_RESULT_T rAllTxRateResult[TX_STAT_NUM_PER_EVENT];
} EXT_EVENT_TX_RATE_RESULT_T, *P_EXT_EVENT_TX_RATE_RESULT_T;
#endif

typedef struct _EXT_EVENT_TXRX_AIRTIME_T {
	UINT_16 u2WlanId;
	UINT_32 u4WtblTxTime[ACI_AC_NUM];
	UINT_32 u4WtblRxTime[ACI_AC_NUM];
} EXT_EVENT_TXRX_AIRTIME_T, *P_EXT_EVENT_TXRX_AIRTIME_T;

typedef struct _EXT_EVENT_TXRX_AIRTIME_INFO_T {
	UINT_8 u1PhyEventId;
	UINT_8 u1FlagMoreEvent;
	UINT_16 u2StaNum;
	EXT_EVENT_TXRX_AIRTIME_T rTxRxAirTimeStat[TXRX_ADM_STA_NUM_PER_EVENT];
} EXT_EVENT_TXRX_AIRTIME_INFO_T, *P_EXT_EVENT_TXRX_AIRTIME_INFO_T;

typedef struct _EXT_EVENT_DATA_TX_RETRY_T {
	UINT_16 u2WlanId;
	UINT_16 u2TxRetryCnt;
} EXT_EVENT_DATA_TX_RETRY_T, *P_EXT_EVENT_DATA_TX_RETRY_T;

typedef struct _EXT_EVENT_TX_RETRY_T {
	UINT_8 u1PhyEventId;
	UINT_8 u1FlagMoreEvent;
	UINT_16 u2StaNum;
	EXT_EVENT_DATA_TX_RETRY_T u2DataTxRetryCnt[DATA_TX_RETRY_COUNT_NUM_PER_EVENT];
} EXT_EVENT_TX_RETRY_T, *P_EXT_EVENT_TX_RETRY_T;

typedef struct GNU_PACKED _EXT_CMD_FW_DBG_CTRL_T
{
	UINT8  ucCmdVer;
	UINT8  aucPadding0[1];
	UINT16 u2CmdLen;
	UINT8 ucDbgClass;
	UINT8 ucReserve[3];
	UINT32 u4DbgModuleIdx;
} EXT_CMD_FW_DBG_CTRL_T, *P_EXT_CMD_FW_DBG_CTRL_T;

typedef struct GNU_PACKED _CMD_AP_PS_RETRIEVE_T {
	UINT32 u4Option; /* 0: AP_PWS enable, 1: redirect disable */
	UINT32 u4Param1; /* for 0: enable/disable. for 1: wlan idx */
	UINT32 u4Resv;
} CMD_AP_PS_RETRIEVE_STRUC_T, *P_CMD_AP_PS_RETRIEVE_STRUC_T;

typedef struct GNU_PACKED _EXT_EVENT_AP_PS_RETRIEVE_T {
	UINT32 u4Param1; /* for 0: enable/disable. for 1: wlan idx */
	UINT32 u4Resv;
	UINT32 u4Resv2;
} EXT_EVENT_AP_PS_RETRIEVE_T, *P_EXT_EVENT_AP_PS_RETRIEVE_T;

typedef struct GNU_PACKED _BIN_CONTENT_T {
	UINT16 u2Addr;
	UINT8 ucValue;
	UINT8 ucReserved;
} BIN_CONTENT_T, *P_BIN_CONTENT_T;

typedef enum _ENUM_THERMO_ITEM_T {
	THERMO_ITEM_DPD_CAL = 0,
	THERMO_ITEM_OVERHEAT = 1,
	THERMO_ITEM_BB_HI = 2,
	THERMO_ITEM_BB_LO = 3,
	NTX_THERMAL_PROTECT_HI = 4,
	NTX_THERMAL_PROTECT_LO = 5,
	ADM_THERMAL_PROTECT_HI = 6,
	ADM_THERMAL_PROTECT_LO = 7,
	RF_THERMAL_PROTECT_HI = 8,
	THERMO_ITEM_TSSI_COMP = 9,
	TX_POWER_TEMP_COMP_N7_2G4 = 10,
	TX_POWER_TEMP_COMP_N6_2G4 = 11,
	TX_POWER_TEMP_COMP_N5_2G4 = 12,
	TX_POWER_TEMP_COMP_N4_2G4 = 13,
	TX_POWER_TEMP_COMP_N3_2G4 = 14,
	TX_POWER_TEMP_COMP_N2_2G4 = 15,
	TX_POWER_TEMP_COMP_N1_2G4 = 16,
	TX_POWER_TEMP_COMP_N0_2G4 = 17,
	TX_POWER_TEMP_COMP_P1_2G4 = 18,
	TX_POWER_TEMP_COMP_P2_2G4 = 19,
	TX_POWER_TEMP_COMP_P3_2G4 = 20,
	TX_POWER_TEMP_COMP_P4_2G4 = 21,
	TX_POWER_TEMP_COMP_P5_2G4 = 22,
	TX_POWER_TEMP_COMP_P6_2G4 = 23,
	TX_POWER_TEMP_COMP_P7_2G4 = 24,
	TX_POWER_TEMP_COMP_N7_5G = 25,
	TX_POWER_TEMP_COMP_N6_5G = 26,
	TX_POWER_TEMP_COMP_N5_5G = 27,
	TX_POWER_TEMP_COMP_N4_5G = 28,
	TX_POWER_TEMP_COMP_N3_5G = 29,
	TX_POWER_TEMP_COMP_N2_5G = 30,
	TX_POWER_TEMP_COMP_N1_5G = 31,
	TX_POWER_TEMP_COMP_N0_5G = 32,
	TX_POWER_TEMP_COMP_P1_5G = 33,
	TX_POWER_TEMP_COMP_P2_5G = 34,
	TX_POWER_TEMP_COMP_P3_5G = 35,
	TX_POWER_TEMP_COMP_P4_5G = 36,
	TX_POWER_TEMP_COMP_P5_5G = 37,
	TX_POWER_TEMP_COMP_P6_5G = 38,
	TX_POWER_TEMP_COMP_P7_5G = 39,
	THERMO_ITEM_DYNAMIC_G0 = 40,
	THERMO_ITEM_NUM = 41
} ENUM_THERMO_ITEM_T, *P_ENUM_THERMO_ITEM_T;

typedef struct _THERMO_ITEM_T {
	UINT32 u4ThermalTaskProp;
	BOOLEAN fgTrigEn;
	UINT8 u1Thres;
	UINT32 u4BoundHandle;
	BOOLEAN fgTag;
	UINT32 u4Data;
} THERMO_ITEM_T, *P_THERMO_ITEM_T;

typedef struct _THERMO_ITEM_INFO_T {
	UINT8   ucThermoItem;
	UINT8	ucThermoType;
	BOOLEAN fgLowerEn;
	BOOLEAN fgUpperEn;
	INT8	cLowerBound;
	INT8	cUpperBound;
} THERMO_ITEM_INFO_T, *P_THERMO_ITEM_INFO_T;

typedef struct GNU_PACKED _CMD_THERMAL_SENSOR_INFO_T {
	UINT8  u1ThermalCtrlFormatId;
	UINT8  u1ActionIdx;    /* 0: get temperature, 1: get thermal sensor ADC */
	UINT8  u1BandIdx;
	UINT8  u1Reserved;
	UINT32 u4SensorResult;
} CMD_THERMAL_SENSOR_INFO_T, *P_CMD_THERMAL_SENSOR_INFO_T;

typedef struct _CMD_TEMPERATURE_CTRL_T {
	UINT8   u1PowerCtrlFormatId;
	BOOLEAN fgManualMode;	/* 1: Enable Temperature Manual Ctrl,  0: Disable Temperature Manual Ctrl */
	CHAR	i1Temperature;   /* Temperature (Celsius Degree) */
	UINT8   u1BandIdx;
} CMD_TEMPERATURE_CTRL_T, *P_CMD_TEMPERATURE_CTRL_T;

typedef struct GNU_PACKED _EXT_EVENT_THERMAL_SENSOR_INFO_T {
	UINT8  u1ThermalCategory;
	UINT8  u1Reserved[3];
	UINT32 u4SensorResult;
} EXT_EVENT_THERMAL_SENSOR_INFO_T, *P_EXT_EVENT_THERMAL_SENSOR_INFO_T;

typedef struct _EXT_EVENT_THERMAL_SENSOR_ITEM_INFO_T {
	UINT8 u1ThermalCategory;
	UINT8 u1ThermoTaskNum;
	UINT8 u1SensorThLow;
	UINT8 u1SensorThHigh;
	THERMO_ITEM_T arThermoItems[THERMAL_TASK_NUM];
} EXT_EVENT_THERMAL_SENSOR_ITEM_INFO_T, *P_EXT_EVENT_THERMAL_SENSOR_ITEM_INFO_T;

typedef struct _EXT_EVENT_THERMAL_SENSOR_TASK_RESPONSE_T {
	UINT8 u1ThermalCategory;
	UINT8 u1ThermalAdc;
	UINT8 au1Reserved[2];
	UINT32 u4PhyIdx;
	UINT32 u4ThermalTaskProp;
	UINT32 u4FuncPtr;
} EXT_EVENT_THERMAL_SENSOR_TASK_RESPONSE_T, *P_EXT_EVENT_THERMAL_SENSOR_TASK_RESPONSE_T;


typedef struct GNU_PACKED _CMD_THERMAL_BASIC_INFO_T {
	UINT8 u1PowerCtrlFormatId;
	UINT8 u1BandIdx;
	UINT8 u1Reserved[2];
} CMD_THERMAL_BASIC_INFO_T, *P_CMD_THERMAL_BASIC_INFO_T;

typedef struct GNU_PACKED _EXT_EVENT_TDLS_SETUP_T {
	UINT8  ucResultId;
	UINT8  aucReserved[3];

	UINT32 u4StartTime;
	UINT32 u4EndTime;
	UINT32 u4TbttTime;
	UINT32 u4StayTime;
	UINT32 u4RestTime;
} EXT_EVENT_TDLS_SETUP_T, *P_EXT_EVENT_TDLS_SETUP_T;

typedef struct GNU_PACKED _EXT_EVENT_TDLS_STATUS_T {
	UINT8  ucResultId;
	UINT8  aucReserved[3];
} EXT_EVENT_TDLS_STATUS_T, *P_EXT_EVENT_TDLS_STATUS_T;

#define BUFFER_BIN_TOTAL_PAGE_MASK	BITS(5, 7)
#define BUFFER_BIN_TOTAL_PAGE_SHIFT	5
#define BUFFER_BIN_PAGE_INDEX_MASK	BITS(2, 4)
#define BUFFER_BIN_PAGE_INDEX_SHIFT	2

#ifdef AIR_MONITOR
typedef struct _EXT_CMD_SMESH_T {
    UINT8 ucBand;
    UINT8 ucAccessMode; /* 0:read, 1:write */
    UINT8 ucSmeshEn;
    BOOLEAN fgSmeshRxA2;
    BOOLEAN fgSmeshRxA1;
    BOOLEAN fgSmeshRxData;
    BOOLEAN fgSmeshRxMgnt;
    BOOLEAN fgSmeshRxCtrl;
} EXT_CMD_SMESH_T, *P_EXT_CMD_SMESH_T;

typedef struct _EXT_EVENT_SMESH_T {
    UINT8 ucBand;
    UINT32 u4SmeshVal;
} EXT_EVENT_SMESH_T, *P_EXT_EVENT_SMESH_T;
#endif /* AIR_MONITOR */

#if defined(MT7663) || defined(MT7626) || defined(AXE) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)
struct _EXT_CMD_EFUSE_BUFFER_MODE_V2_T {
	UINT8  ucSourceMode;	/* 0: eFuse mode; 1: Buffer mode */
	UINT8  ucContentFormat;	/* 0: Bin Content mode; 1: Contiguous Byte mode */
	UINT16 ucCount;        /* Total number of aBinContent elements */
	UINT8 BinContent[];
};

typedef struct _EXT_CMD_HWCFG_READ_T {
    UINT16 u2Offset;           /* Read Offset */
    UINT16 u2Count;            /* Read Total Counts */
} EXT_CMD_HWCFG_READ_T, *P_EXT_CMD_HWCFG_READ_T;

typedef struct _EXT_EVENT_HWCFG_READ_T {
    UINT16 u2Offset;           /* Read Offset */
    UINT16 u2Count;            /* Read Total Counts */
    UINT8  BinContent[];
} EXT_EVENT_HWCFG_READ_T, *P_EXT_EVENT_HWCFG_READ_T;

typedef struct _EXT_CMD_EFUSE_BUFFER_MODE_READ_T {
	UINT8  ucSourceMode;	/* 0: eFuse mode; 1: Buffer mode */
	UINT8  ucContentFormat;	/* 0: Bin Content mode; 1: Contiguous Byte mode */
	UINT16 u2Offset;        /* Read Offset */
	UINT16 u2Count;         /* Read Total Counts */
} EXT_CMD_EFUSE_BUFFER_MODE_READ_T, *P_EXT_CMD_EFUSE_BUFFER_MODE_READ_T;

typedef struct _EXT_EVENT_EFUSE_BUFFER_MODE_READ_T {
	UINT8  ucSourceMode;	/* 0: eFuse mode; 1: Buffer mode */
	UINT8  ucContentFormat;	/* 0: Bin Content mode; 1: Contiguous Byte mode */
	UINT16 u2Offset;        /* Read Offset */
	UINT16 u2Count;         /* Read Total Counts */
	UINT8 BinContent[];
} EXT_EVENT_EFUSE_BUFFER_MODE_READ_T, *P_EXT_EVENT_EFUSE_BUFFER_MODE_READ_T;
#endif
/* defined(MT7663) || defined(MT7626) || defined(AXE) || defined(MT7915) ||
 * defined(MT7986) || defined(MT7916) || defined(MT7981)
 */

union _EXT_CMD_EFUSE_BUFFER_MODE_T {
#if defined(MT7663) || defined(MT7626) || defined(AXE) || defined(MT7915) || \
	defined(MT7986) || defined(MT7916) || defined(MT7981)
	struct _EXT_CMD_EFUSE_BUFFER_MODE_V2_T v2;
#endif
};

typedef struct GNU_PACKED _EXT_CMD_EVENT_DUMP_MEM_T {
	UINT32	u4MemAddr;
	UINT8	ucData[64];
} EXT_CMD_EVENT_DUMP_MEM_T, *P_EXT_CMD_EVENT_DUMP_MEM_T;

typedef struct _MEM_DUMP_DATA_T {
	PUINT8 pValue;
} MEM_DUMP_DATA_T;

typedef enum _EXT_ENUM_PM_FEATURE_T {
	PM_CMD_FEATURE_PSPOLL_OFFLOAD       = 0x00000001,
	PM_CMD_FEATURE_PS_TX_REDIRECT        = 0x00000002,
	PM_CMD_FEATURE_SMART_BCN_SP          = 0x00000004,
	PM_CMD_FEATURE_SEND_NULL_FRAME		 = 0x00000008,
} EXT_ENUM_PM_FEATURE_T;


enum _ENUM_BCN_LOSS_REASON_T {
	/* For STA/ApCli mode (Beacon stop receiving) */
	ENUM_BCN_LOSS_STA =			0x00,

	/* For AP mode (Beacon stop sending) */
	ENUM_BCN_LOSS_AP_DISABLE =		0x10,
	ENUM_BCN_LOSS_AP_SER_TRIGGER =	0x11,
	ENUM_BCN_LOSS_AP_ERROR =		0x12
};

typedef struct GNU_PACKED _EXT_EVENT_BEACON_LOSS_T {
	UINT8		aucBssid[6];
	UINT8		ucReason;
	UINT8		ucReserve;
} EXT_EVENT_BEACON_LOSS_T, *P_EXT_EVENT_BEACON_LOSS_T;

/** struct for EVENT */
typedef struct _EXT_EVENT_IDLE_PWR_GET_T {
	UINT8   obss_percent;
	UINT8   ipi_percent;
	UINT8   band_idx;
	UINT8   reserved;
} EXT_EVENT_IDLE_PWR_GET_T, *P_EXT_EVENT_IDLE_PWR_GET_T;

#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
#define RDD_2_SUPPORTED 1
#define DFS_ZEROWAIT_DEFAULT_FLOW 1
#define DFS_ZEROWAIT_SUPPORT_8080 1
#else
#define RDD_2_SUPPORTED 0
#define DFS_ZEROWAIT_DEFAULT_FLOW 0
#define DFS_ZEROWAIT_SUPPORT_8080 0
#endif

#ifdef MT_DFS_SUPPORT/* Jelly20150123 */
typedef struct GNU_PACKED _SW_RADAR_TYPE_T
{
	UINT8 rt_det;          /* Radar type is detected */
	UINT8 rt_en;           /* Radar type is enabled */
	UINT8 rt_stgr;         /* Radar type is staggred radar */
	UINT8 rt_crpn_min;     /* Constant PRF Radar: minimum Pulse Number */
	UINT8 rt_crpn_max;
	UINT8 rt_crpr_min;     /* Request (RT_CRPR_MIN/32)*100% above pulses among short pulses should be periodic */
	UINT8 rt_pw_min;
	UINT8 rt_pw_max;
	UINT32 rt_pri_min;
	UINT32 rt_pri_max;
	UINT8 rt_crbn_min;     /* Constant PRF Radar: Burst Number */
	UINT8 rt_crbn_max;
	UINT8 rt_stg_pn_min;   /* Staggered PRF radar: Staggered pulse number */
	UINT8 rt_stg_pn_max;
	UINT8 rt_stg_pr_min;
	UINT8 reserved[3];
	UINT32 rt_stg_pri_diff_min; /* Staggered PRF radar: min PRI Difference between 1st and 2nd  */
} SW_RADAR_TYPE_T, *PSW_RADAR_TYPE_T;

/* size of period pulse buffer and long pulse buffer */
#define PPB_SIZE 32
#define LPB_SIZE 32
#define MAX_HW_PB_SIZE 32

typedef struct GNU_PACKED _LONG_PULSE_BUFFER_T
{
	UINT32 lng_strt_time;       /* Long pulse start time */
	UINT16 lng_pls_wdth;        /* Long pulse width */
	INT16 lng_pls_pwr;          /* Long pulse power */
	UINT8 lng_mdrdy_flg;        /* Long pulse MDRDY flag, bit1: mdray_early_flag, bit0: mdrdy_late_flag */
	UINT8 reserved[3];          /* Reserved bytes */
} LONG_PULSE_BUFFER_T, *PLONG_PULSE_BUFFER_T;

typedef struct GNU_PACKED _PERIODIC_PULSE_BUFFER_T
{
	UINT32 prd_strt_time;       /* Periodic pulse start time */
	UINT16 prd_pls_wdth;        /* Periodic pulse width */
	INT16 prd_pls_pwr;          /* Periodic pulse power */
	UINT8 prd_mdrdy_flg;        /* Periodic pulse MDRDY flag, bit1: mdray_early_flag, bit0: mdrdy_late_flag */
	UINT8 reserved[3];          /* Reserved bytes */
} PERIODIC_PULSE_BUFFER_T, *PPERIODIC_PULSE_BUFFER_T;

typedef struct GNU_PACKED _HW_PULSE_CONTENT_T
{
	UINT32 hw_start_time;
	UINT16 hw_pls_width;
	INT16 hw_pls_pwr;
	BOOLEAN hw_sc_pass;
	BOOLEAN hw_sw_reset;
	UINT8 hw_mdrdy_flag; /* bit1: mdray_early_flag, bit0: mdrdy_late_flag */
	UINT8 hw_tx_active;  /* bit1: tx_early_flag, bit0: tx_late_flag */
} HW_PULSE_CONTENT_T, *PHW_PULSE_CONTENT_T;

typedef struct GNU_PACKED _EXT_EVENT_RDD_REPORT_T
{
	UINT8 rdd_idx;
	UINT8 lng_pls_detected;
	UINT8 cr_pls_detected;
	UINT8 stgr_pls_detected;
	UINT8 rt_idx;      /* radar type index */
	UINT8 prd_pls_num; /* period pulse num */
	UINT8 lng_pls_num; /* long pulse num */
	UINT8 hw_pls_num;
	UINT8 out_lpn;     /* Long Pulse Number */
	UINT8 out_spn;     /* Short Pulse Number */
	UINT8 out_crpn;    /* Constant PRF Radar: Pulse Number */
	UINT8 out_crpw;    /* Constant PRF Radar: Pulse width */
	UINT8 out_crbn;   /* Constant PRF Radar: Burst Number */
	UINT8 out_stg_pn;  /* Staggered PRF radar: Staggered pulse number */
	UINT8 out_stg_pw;  /* Staggered PRF radar: maximum pulse width */
	UINT8 channel;
	UINT32 out_pri_const;
	UINT32 out_pri_stg1;
	UINT32 out_pri_stg2;
	UINT32 out_pri_stg3;
	UINT32 out_pri_stg_dmin; /* Staggered PRF radar: min PRI Difference between 1st and 2nd  */
	LONG_PULSE_BUFFER_T lng_pls_buff[LPB_SIZE];
	PERIODIC_PULSE_BUFFER_T prd_pls_buff[PPB_SIZE];
	HW_PULSE_CONTENT_T hw_pls_buff[MAX_HW_PB_SIZE];
} EXT_EVENT_RDD_REPORT_T, *P_EXT_EVENT_RDD_REPORT_T;

typedef struct _EXT_CMD_RDM_TEST_RADAR_PATTERN_T {
	UINT8 pls_num;
	UINT8 reserved[3];
	PERIODIC_PULSE_BUFFER_T prd_pls_buff[PPB_SIZE];
} CMD_RDM_TEST_RADAR_PATTERN_T, *P_CMD_RDM_TEST_RADAR_PATTERN_T;

typedef struct GNU_PACKED _RDM_FCC5_LPN_UPDATE_T {
	UINT32 tag;            /* Tag = 0x01 */
	UINT16 fcc_lpn_min;
	UINT8  reserved[2];
} CMD_RDM_FCC5_LPN_UPDATE_T, *P_CMD_RDM_FCC5_LPN_UPDATE_T;

typedef struct GNU_PACKED _RDM_RADAR_THRESHOLD_UPDATE_T {
	UINT32 tag;            /* Tag = 0x02 */
	UINT16 radar_type_idx; /* Valid Range 0~15*/
	UINT8  rt_en;          /* Radar type is enabled */
	UINT8  rt_stgr;        /* Radar type is staggred radar */
	UINT8  rt_crpn_min;    /* minimum constant PRF radar pulse number */
	UINT8  rt_crpn_max;    /* maximum constant PRF radar pulse number */
	UINT8  rt_crpr_min;    /* minimum constant PRF radar pulse ratio */
	UINT8  rt_pw_min;
	UINT32 rt_pri_min;
	UINT32 rt_pri_max;
	UINT8  rt_pw_max;
	UINT8  rt_crbn_min;  /* Constant PRF Radar: Burst Number */
	UINT8  rt_crbn_max;
	UINT8  rt_stg_pn_min;
	UINT8  rt_stg_pn_max;
	UINT8  rt_stg_pr_min;
	UINT8  reserved[2];
	UINT32 rt_stg_pri_diff_min; /* Staggered PRF radar: min PRI Difference between 1st and 2nd  */
} CMD_RDM_RADAR_THRESHOLD_UPDATE_T, *P_CMD_RDM_RADAR_THRESHOLD_UPDATE_T;

typedef struct GNU_PACKED _RDM_PULSE_THRESHOLD_UPDATE_T {
	UINT32 tag;                 /* Tag = 0x03 */
	UINT32 prd_pls_width_max;   /* unit us */
	INT32 pls_pwr_max;          /* unit dbm */
	INT32 pls_pwr_min;          /* unit dbm */
	UINT32 pri_min_stgr;		/* unit us */
	UINT32 pri_max_stgr;		/* unit us */
	UINT32 pri_min_cr;			/* unit us */
	UINT32 pri_max_cr;			/* unit us */
} CMD_RDM_PULSE_THRESHOLD_UPDATE_T, *P_CMD_RDM_PULSE_THRESHOLD_UPDATE_T;

typedef struct GNU_PACKED _RDM_RDD_LOG_CONFIG_UPDATE_T {
	UINT16 tag;                 /* Tag = 0x04 */
	UINT8 hw_rdd_log_en;        /* 0: no dump, 1: dump log */
	UINT8 sw_rdd_log_en;        /* 0: no dump, 1: dump log */
	UINT8 sw_rdd_log_cond;      /*0: send log for every interrupt, 1: send log only when a radar is detected. */
} CMD_RDM_RDD_LOG_CONFIG_UPDATE_T, *P_CMD_RDM_RDD_LOG_CONFIG_UPDATE_T;


typedef struct GNU_PACKED _EXT_EVENT_CAC_END_T {
	UINT8       ucRddIdx;
	UINT8       aucReserve[3];
} EXT_EVENT_CAC_END_T, *P_EXT_EVENT_CAC_END_T;
#endif

typedef struct GNU_PACKED _EXT_EVENT_ROAMING_DETECT_RESULT_T {
	UINT8		ucBssidIdx;
	UINT8		aucReserved[3];
	UINT32		u4RoamReason;
} EXT_EVENT_ROAMING_DETECT_RESULT_T, *P_EXT_EVENT_ROAMING_DETECT_RESULT_T;

enum {
	ROAMING_STATUS_NOT_DETERMINED = 0x00,
	ROAMING_RCPI_CCK_EXCEED_MAX = 0x01,
	ROAMING_RCPI_CCK_LOWER_MIN = 0x02,
	ROAMING_RCPI_OFDM_EXCEED_MAX = 0x04,
	ROAMING_RCPI_OFDM_LOWER_MIN  = 0x08,
	ROAMING_LQ_CCK_EXCEED_MAX = 0x10,
	ROAMING_LQ_CCK_LOWER_MIN  = 0x20,
	ROAMING_LQ_OFDM_EXCEED_MAX = 0x40,
	ROAMING_LQ_OFDM_LOWER_MIN = 0x80,
};

typedef struct GNU_PACKED _EXT_EVENT_ASSERT_DUMP_T {
	UINT8	aucBuffer[1000]; /* temparary size */
} EXT_EVENT_ASSERT_DUMP_T, *P_EXT_EVENT_ASSERT_DUMP_T;

typedef struct GNU_PACKED _EXT_CMD_PWR_MGT_BIT_T {
	UINT8		ucWlanIdxL;			/* #256STA - Low Byte */
	UINT8		ucPwrMgtBit;
	UINT8		ucWlanIdxHnVer;		/* #256STA - High Byte and Version */
	UINT8		aucReserve;
} EXT_CMD_PWR_MGT_BIT_T, *P_EXT_CMD_PWR_MGT_BIT_T;

typedef struct GNU_PACKED _EXT_CMD_HOST_RESUME_DONE_ACK_T {
	UINT8		ucReserved0;
	UINT8		ucReserved1;
	UINT8		ucReserved2;
	UINT8		ucReserved3;
} EXT_CMD_HOST_RESUME_DONE_ACK_T, *P_EXT_CMD_HOST_RESUME_DONE_ACK_T;

typedef struct GNU_PACKED _EXT_EVENT_PS_SYNC_T
{
	UINT8		ucWtblIndexL;		/* #256STA - Low Byte */
	UINT8		ucPsBit;
	UINT8		ucWtblIndexHnVer;	/* #256STA - High Byte and Version */
	UINT8		aucReserves;
} EXT_EVENT_PS_SYNC_T, *P_EXT_EVENT_PS_SYNC_T;

enum {
	EEPROM_MODE_EFUSE = 0,
	EEPROM_MODE_BUFFER = 1,
};

/* 0: 2g + 5g pwr on cal ; 1: 2g + 6g pwr on cal */
#define PWR_ON_CAL_SEL_MASK    BIT(1)
#define PWR_ON_CAL_SEL_SHIFT   1

/* 0: eFuse mode; 1: Buffer mode */
#define SOURCE_MODE_MASK    BIT(0)
#define SOURCE_MODE_SHIFT   0

#define EEPROM_BUFFER_MODE_DATA_LIMIT (0x400)

typedef struct GNU_PACKED _CMD_ACCESS_EFUSE_T {
	UINT32         u4Address;
	UINT32         u4Valid;
	UINT8          aucData[16];
} CMD_ACCESS_EFUSE_T, *P_CMD_ACCESS_EFUSE_T, _EXT_EVENT_ACCESS_EFUSE_T;

typedef struct _EFUSE_ACCESS_DATA_T {
	PUINT pIsValid;
	PUSHORT pValue;
} EFUSE_ACCESS_DATA_T;

typedef struct GNU_PACKED _EXT_CMD_EFUSE_FREE_BLOCK_T {
    UINT8   ucGetFreeBlock;
    UINT8   ucVersion; /* 0: original format ; 1: modified format */
    UINT8   ucDieIndex; /* for 7663, 0: D die ; 1: A die */
    UINT8   ucReserved;
} EXT_CMD_EFUSE_FREE_BLOCK_T, *P_EXT_CMD_EFUSE_FREE_BLOCK_T;

typedef struct GNU_PACKED _EXT_EVENT_EFUSE_FREE_BLOCK_T {
	UINT32 ucFreeBlockNum;
	UINT8 aucReserve[4];
} EXT_EVENT_EFUSE_FREE_BLOCK_T, *P_EXT_EVENT_EFUSE_FREE_BLOCK_T;

typedef struct _EXT_EVENT_EFUSE_FREE_BLOCK_V1_T {
    UINT8  ucFreeBlockNum;
    UINT8  ucVersion; /* 0: original format ; 1: modified format */
    UINT8  ucTotalBlockNum; /* Total Block */
    UINT8  ucReserved;
} EXT_EVENT_EFUSE_FREE_BLOCK_V1_T, *P_EXT_EVENT_EFUSE_FREE_BLOCK_V1_T;

typedef struct _EXT_CMD_ACCESS_EFUSE_CHECK_T {
	UINT32 u4Address;
	UINT8 aucData[16];
} EXT_CMD_ACCESS_EFUSE_CHECK_T, *P_EXT_CMD_ACCESS_EFUSE_CHECK_T;

typedef struct _EXT_EVENT_ACCESS_EFUSE_CHECK_T {
	UINT32 u4Address;
	UINT8 ucStatus;
	UINT8 aucReserve[3];
} EXT_EVENT_ACCESS_EFUSE_CHECK_T, *P_EXT_EVENT_ACCESS_EFUSE_CHECK_T;

enum {
	HIGH_TEMP_THD = 0,
	LOW_TEMP_THD = 1,
};

enum _ENUM_THERMAL_PROTECTION_EXTRA_TAG {
	THERAML_PROTECTION_TAG_SET_ADMIT_DUTY = 1,
};

typedef enum _ENUM_THERMAL_PROTECTION_REASON_T {
	THERAML_PROTECTION_REASON_NTX,
	THERAML_PROTECTION_REASON_ADM,
	THERAML_PROTECTION_REASON_RADIO
} ENUM_THERMAL_PROTECTION_REASON_T, *P_ENUM_THERMAL_PROTECTION_REASON_T;


typedef enum _ENUM_THERMAL_PROTECT_TX_DUTY_LEVEL_ITEM_T {
	TX_DUTY_LEVEL_0 = 0,
	TX_DUTY_LEVEL_1,
	TX_DUTY_LEVEL_2,
	TX_DUTY_LEVEL_3,
	TX_DUTY_LEVEL_NUM
} ENUM_THERMAL_PROTECT_TX_DUTY_LEVEL_ITEM_T, *P_ENUM_THERMAL_PROTECT_TX_DUTY_LEVEL_ITEM_T;

/** average admin period */
typedef enum _ENUM_AVG_ADMIN_PERIOD {
	WH_TX_AVG_ADMIN_PERIOD_64US = 0,
	WH_TX_AVG_ADMIN_PERIOD_1000US,
	WH_TX_AVG_ADMIN_PERIOD_NUM
} ENUM_AVG_ADMIN_PERIOD, *P_ENUM_AVG_ADMIN_PERIOD;

typedef struct _EXT_CMD_THERMAL_PROTECT_T {
	UINT8 u1ThermalCtrlFormatId;
	UINT8 u1BandIdx;
	UINT8 u1HighEnable;				/* 0: Disable High temperature event to driver 1: Enable */
	INT8  i1HighTempThreshold;		/* if (current temperature >= cHighTempThreshold) FW notify driver with EXT_EVENT_THERMAL_PROTECT */
	UINT8 u1LowEnable;				/* 0: Disable Low temperature event to driver 1: Enable */
	INT8  i1LowTempThreshold;		/* if (current temperature <= cLowTempThreshold) FW notify driver with EXT_EVENT_THERMAL_PROTECT */
	UINT32 u4RecheckTimer;			/* Set recheck timer in Sec */
	UINT8 u1RFOffEnable;			/* 0: Disable RFOff event to driver 1: Enable */
	CHAR  i1RFOffThreshold;			/* if (current temperature >= cRFOffThreshold) FW notify driver with EXT_EVENT_THERMAL_PROTECT; */
	UINT8 u1Type;					/* 0: Limit Tx Stream  1: admission control */
	UINT8 u1ExtraTag;
	UINT8 u1Lv0Duty;
	UINT8 u1Lv1Duty;
	UINT8 u1Lv2Duty;
	UINT8 u1Lv3Duty;
} EXT_CMD_THERMAL_PROTECT_T, *P_EXT_CMD_THERMAL_PROTECT_T;

typedef struct _EXT_CMD_THERMAL_PROTECT_INFO_T {
	UINT8 u1ThermalCtrlFormatId;
	UINT8 aucReserve[3];
} EXT_CMD_THERMAL_PROTECT_INFO_T, *P_EXT_CMD_THERMAL_PROTECT_INFO_T;

typedef struct GNU_PACKED _EXT_EVENT_THERMAL_PROTECT_T {
	UINT8 u1ThermalProtCategory;
	UINT8 ucHLType;
	CHAR cCurrentTemp;
	UINT8 ucReason;
	UINT8 aucReserve;
} EXT_EVENT_THERMAL_PROTECT_T, *P_EXT_EVENT_THERMAL_PROTECT_T;

typedef struct _EXT_EVENT_THERMAL_PROT_ITEM_INFO_T {
	UINT8   u1ThermalProtCategory;
	UINT8   u1AdmitPeriod;
	UINT8   u1AvrgPeriod;

	UINT16 u2DutyLevel[TX_DUTY_LEVEL_NUM];
	UINT16 u2AdmitDutyLevel[TX_DUTY_LEVEL_NUM];
} EXT_EVENT_THERMAL_PROT_ITEM_INFO_T, *P_EXT_EVENT_THERMAL_PROT_ITEM_INFO_T;

typedef enum _ENUM_THERMAL_PROTECT_ACT_TYPE {
	THERMAL_PROTECT_ACT_TYPE_TRIG = 0,
	THERMAL_PROTECT_ACT_TYPE_RESTORE,
	THERMAL_PROTECT_ACT_TYPE_NUM
} ENUM_THERMAL_PROTECT_ACT_TYPE, *P_ENUM_THERMAL_PROTECT_ACT_TYPE;

struct _EXT_CMD_THERMAL_PROTECT_ENABLE {
	UINT8 sub_cmd_id;
	UINT8 band_idx;
	UINT8 protection_type;
	UINT8 trigger_type;
	INT32 trigger_temp;
	INT32 restore_temp;
	UINT16 recheck_time;
	UINT8 reserved[2];
};

struct _EXT_CMD_THERMAL_PROTECT_DISABLE {
	UINT8 sub_cmd_id;
	UINT8 band_idx;
	UINT8 protection_type;
	UINT8 trigger_type;
};

struct _EXT_CMD_THERMAL_PROTECT_DUTY_CFG {
	UINT8 sub_cmd_id;
	UINT8 band_idx;
	UINT8 level_idx;
	UINT8 duty;
};

struct _EXT_CMD_THERMAL_PROTECT_INFO {
	UINT8 sub_cmd_id;
	UINT8 band_idx;
	UINT8 reserved[2];
};

struct _EXT_CMD_THERMAL_PROTECT_DUTY_INFO {
	UINT8 sub_cmd_id;
	UINT8 band_idx;
	UINT8 reserved[2];
};

struct _EXT_CMD_THERMAL_PROTECT_STATE_ACT {
	UINT8 sub_cmd_id;
	UINT8 band_idx;
	UINT8 protect_type;
	UINT8 trig_type;
	UINT8 state;
	UINT8 reserved[3];
};

struct _EXT_EVENT_THERMAL_PROTECT_DUTY_NOTIFY {
	UINT8 sub_event_id;
	UINT8 band_idx;
	UINT8 level_idx;
	UINT8 duty_percent;
	INT32 temp;
	ENUM_THERMAL_PROTECT_ACT_TYPE act_type;
	UINT8 reserved[3];
};

struct _EXT_EVENT_THERMAL_PROTECT_RADIO_NOTIFY {
	UINT8 sub_event_id;
	UINT8 band_idx;
	UINT8 level_idx;
	UINT8 reserved;
	INT32 temp;
	ENUM_THERMAL_PROTECT_ACT_TYPE act_type;
	UINT8 reserved2[3];
};

struct THERMAL_PROTECT_DUTY_INFO {
	UINT8 band_idx;
	UINT8 duty0;
	UINT8 duty1;
	UINT8 duty2;
	UINT8 duty3;
};

struct _EXT_EVENT_THERMAL_PROTECT_DUTY_INFO {
	UINT8 sub_event_id;
	UINT8 band_idx;
	UINT8 duty0;
	UINT8 duty1;
	UINT8 duty2;
	UINT8 duty3;
	UINT8 reserved[2];
};

enum _ENUM_THERMAL_PROTECT_TYPE {
	THERMAL_PROTECT_TYPE_NTX_CTRL = 0,
	THERMAL_PROTECT_TYPE_DUTY_CTRL,
	THERMAL_PROTECT_TYPE_RADIO_CTRL,
	THERMAL_PROTECT_TYPE_NUM
};

struct THERMAL_PROTECT_MECH_INFO {
	UINT8 band_idx;
	UINT8 protect_type[THERMAL_PROTECT_TYPE_NUM];
	UINT8 trigger_type[THERMAL_PROTECT_TYPE_NUM];
	INT32 trigger_temp[THERMAL_PROTECT_TYPE_NUM];
	INT32 restore_temp[THERMAL_PROTECT_TYPE_NUM];
	UINT16 recheck_time[THERMAL_PROTECT_TYPE_NUM];
	UINT8 state[THERMAL_PROTECT_TYPE_NUM];
	BOOLEAN enable[THERMAL_PROTECT_TYPE_NUM];
};

struct _EXT_EVENT_THERMAL_PROTECT_MECH_INFO {
	UINT8 sub_event_id;
	UINT8 band_idx;
	UINT8 reserved[2];
	UINT8 protect_type[THERMAL_PROTECT_TYPE_NUM];
	UINT8 reserved2;
	UINT8 trigger_type[THERMAL_PROTECT_TYPE_NUM];
	UINT8 reserved3;
	INT32 trigger_temp[THERMAL_PROTECT_TYPE_NUM];
	INT32 restore_temp[THERMAL_PROTECT_TYPE_NUM];
	UINT16 recheck_time[THERMAL_PROTECT_TYPE_NUM];
	UINT8 reserved4[2];
	UINT8 state[THERMAL_PROTECT_TYPE_NUM];
	UINT8 reserved6;
	BOOLEAN enable[THERMAL_PROTECT_TYPE_NUM];
	UINT8 reserved7;
};

typedef struct GNU_PACKED _EXT_CMD_TMR_CAL_T {
	UINT8 ucEnable;
	UINT8 ucBand;/* 0: 2G, 1: 5G */
	UINT8 ucBW;/* 0: 20MHz, 1: 40MHz, 2: 80MHz, 3: 160MHz, 4: 10MHz, 5: 5MHz */
	UINT8 ucAnt;/* 0: Atn0, 1: Ant1 */

	UINT8 ucRole;/* 0: initiator, 1: responder */
	UINT8 aucReserve[3];
} EXT_CMD_TMR_CAL_T, *P_EXT_CMD_TMR_CAL_T;

typedef struct GNU_PACKED _UPDATE_RTS_THRESHOLD_T {
	UINT32 u4RtsPktLenThreshold;
	UINT32 u4RtsPktNumThreshold;
} UPDATE_RTS_THRESHOLD_T, *P_UPDATE_RTS_THRESHOLD_T;

typedef struct GNU_PACKED _UPDATE_PROTECTION_T {
	UINT8 ucLongNav;
	UINT8 ucMMProtect;
	UINT8 ucGFProtect;
	UINT8 ucBW40Protect;
	UINT8 ucRifsProtect;
	UINT8 ucBW80Protect;
	UINT8 ucBW160Protect;
	UINT8 ucERProtectMask;
} UPDATE_PROTECTION_T, *P_UPDATE_PROTECTION_T;

typedef struct GNU_PACKED _UPDATE_DURATION_BASE_PROTECT_T {
	UINT32 u4DurationThreshold;
} UPDATE_DURATION_BASE_PROTECT_T, *P_UPDATE_DURATION_BASE_PROTECT_T;

#define UPDATE_RTS_THRESHOLD		0x1
#define UPDATE_PROTECTION_CTRL		0x2
#define UPDATE_DURATION_PROT_CTRL	0x3

typedef struct GNU_PACKED _EXT_CMD_UPDATE_PROTECT_T {
	/* 0: Rsv,
	 * 1: Rts Threshold,
	 * 2: Protect Threshold,
	 * 3: Duration Based Threshold
	 */
	UINT8 ucProtectIdx;
	UINT8 ucDbdcIdx;
	UINT8 aucRsv[2];

	union {
		UPDATE_RTS_THRESHOLD_T  rUpdateRtsThld;
		UPDATE_PROTECTION_T     rUpdateProtect;
		UPDATE_DURATION_BASE_PROTECT_T     rUpdateDurationBaseProt;
	} Data;
} EXT_CMD_UPDATE_PROTECT_T, *P_EXT_CMD_UPDATE_PROTECT_T;

typedef struct GNU_PACKED _EXT_CMD_RDG_CTRL_T {
	UINT32 u4TxOP;
	UINT8 ucLongNav;
	UINT8 ucInit;
	UINT8 ucResp;
	UINT8 ucWlanIdxL;		/* #256STA - Low Byte */
	UINT8 ucBand;
	UINT8 ucWlanIdxHnVer;	/* #256STA - High Byte and Version */
	UINT8 aucReserved[2];
} EXT_CMD_RDG_T, *P_EXT_CMD_RDG_T;

/*************************************************/
/* EXT_CMD_ID_DRR_CTRL = 0x36 */
/*************************************************/
typedef struct GNU_PACKED _AIRTIME_QUA_ALL_FIELD_T {
	UINT8     ucAirTimeQuantum[8];
} AIRTIME_QUA_ALL_FIELD_T, *P_AIRTIME_QUA_ALL_FIELD_T;

typedef union GNU_PACKED _VOW_DRR_CTRL_VALUE_T {
	UINT32                     u4ComValue;
	AIRTIME_QUA_ALL_FIELD_T     rAirTimeQuantumAllField;    /* used for ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_ALL ctrl only */
} VOW_DRR_CTRL_VALUE_T, *P_VOW_DRR_CTRL_VALUE_T;

typedef struct GNU_PACKED _EXT_CMD_VOW_DRR_CTRL_T {
	UINT32                 u4CtrlFieldID;
	UINT8                  ucStaIDL;	/* #256STA - Low Byte */
	UINT8                  ucCtrlStatus;
	UINT8                  ucStaIDHnVer;	/* #256STA - High Byte and Version */
	UINT8                  ucReserve;
	UINT32                 u4ReserveDW;
	VOW_DRR_CTRL_VALUE_T    rAirTimeCtrlValue;
} EXT_CMD_VOW_DRR_CTRL_T, *P_EXT_CMD_VOW_DRR_CTRL_T;

typedef enum _ENUM_EXT_CMD_VOW_DRR_CTRL_ID_T {
	/* Type 1 */
	ENUM_VOW_DRR_CTRL_FIELD_STA_ALL             = 0x00000000,
	ENUM_VOW_DRR_CTRL_FIELD_STA_BSS_GROUP       = 0x00000001,
	ENUM_VOW_DRR_CTRL_FIELD_STA_PRIORITY        = 0x00000002,
	ENUM_VOW_DRR_CTRL_FIELD_STA_AC0_QUA_ID      = 0x00000003,
	ENUM_VOW_DRR_CTRL_FIELD_STA_AC1_QUA_ID      = 0x00000004,
	ENUM_VOW_DRR_CTRL_FIELD_STA_AC2_QUA_ID      = 0x00000005,
	ENUM_VOW_DRR_CTRL_FIELD_STA_AC3_QUA_ID      = 0x00000006,
	ENUM_VOW_DRR_CTRL_FIELD_STA_WMM_ID              = 0x00000007,
	ENUM_VOW_DRR_CTRL_FIELD_STA_BWC_GROUP		= 0x00000008,
	ENUM_VOW_DRR_CTRL_FIELD_STA_EXCLUDE_GROUP	= 0x00000009,

	ENUM_VOW_DRR_CTRL_FIELD_TYPE_1_BOUNDARY     = 0x0000000f,

	/* Type 2 */
	ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_DEFICIT_BOUND   = 0x00000010,
	ENUM_VOW_DRR_CTRL_FIELD_BW_DEFICIT_BOUND        = 0x00000011,

	ENUM_VOW_DRR_CTRL_FIELD_TYPE_2_BOUNDARY     = 0x0000001f,

	/* Type 3 */
	ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0  = 0x00000020,
	ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L1  = 0x00000021,
	ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L2  = 0x00000022,
	ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L3  = 0x00000023,
	ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L4  = 0x00000024,
	ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L5  = 0x00000025,
	ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L6  = 0x00000026,
	ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L7  = 0x00000027,
	ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_ALL = 0x00000028,

	ENUM_VOW_DRR_CTRL_FIELD_TYPE_3_BOUNDARY     = 0x0000002f,

	/* Type 4 */
	ENUM_VOW_DRR_CTRL_FIELD_STA_PAUSE_SETTING       = 0x00000030,
	ENUM_VOW_DRR_CTRL_FIELD_TYPE_4_BOUNDARY         = 0x0000003f,

} ENUM_EXT_CMD_VOW_DRR_CTRL_ID_T, P_ENUM__EXT_CMD_VOW_DRR_CTRL_ID_T;
/* end of EXT_CMD_ID_DRR_CTRL = 0x36 */

/*************************************************/
/* EXT_CMD_ID_BSSGROUP_CTRL = 0x37 */
/*************************************************/
typedef struct GNU_PACKED _BW_BSS_TOKEN_SETTING_T {
	/* DW#0 */
	UINT16     u2MinRateToken;                 /* unit: 1 bit */
	UINT16     u2MaxRateToken;                 /* unit: 1 bit */

	/* DW#1 */
#ifdef RT_BIG_ENDIAN
	UINT32	   u4MinTokenBucketLengSize:12;    /* unit: 1024 bit */
	UINT32	   u4D1B19Rev:1;				   /* reserve */
	UINT32	   u4MinAirTimeToken:11;		   /* unit: 1/8 us */
	UINT32	   u4MinTokenBucketTimeSize:8;	   /* unit: 1024 us */
#else
	UINT32	   u4MinTokenBucketTimeSize:8;	   /* unit: 1024 us */
	UINT32	   u4MinAirTimeToken:11;		   /* unit: 1/8 us */
	UINT32	   u4D1B19Rev:1;				   /* reserve */
	UINT32	   u4MinTokenBucketLengSize:12;    /* unit: 1024 bit */
#endif
	/* DW#2 */
#ifdef RT_BIG_ENDIAN
	UINT32	   u4MaxTokenBucketLengSize:12;    /* unit: 1024 bit */
	UINT32	   u4D2B19Rev:1;				   /* reserve */
	UINT32	   u4MaxAirTimeToken:11;		   /* unit: 1/8 us */
	UINT32	   u4MaxTokenBucketTimeSize:8;	   /* unit: 1024 us */
#else
	UINT32	   u4MaxTokenBucketTimeSize:8;	   /* unit: 1024 us */
	UINT32	   u4MaxAirTimeToken:11;		   /* unit: 1/8 us */
	UINT32	   u4D2B19Rev:1;				   /* reserve */
	UINT32	   u4MaxTokenBucketLengSize:12;    /* unit: 1024 bit */
#endif
	/* DW#3 */
#ifdef RT_BIG_ENDIAN
	UINT32	   u4D3B28toB31Rev:4;			   /* reserve */
	UINT32	   u4MaxBacklogSize:12;		   /* unit: 1024 bit */
	UINT32	   u4D3B8toB15Rev:8;			   /* reserve */
	UINT32	   u4MaxWaitTime:8;			   /* unit: 1024 us */
#else
	UINT32	   u4MaxWaitTime:8;			   /* unit: 1024 us */
	UINT32	   u4D3B8toB15Rev:8;			   /* reserve */
	UINT32	   u4MaxBacklogSize:12;		   /* unit: 1024 bit */
	UINT32	   u4D3B28toB31Rev:4;			   /* reserve */
#endif

} BW_BSS_TOKEN_SETTING_T, *P_BW_BSS_TOKEN_SETTING_T;

typedef struct GNU_PACKED _EXT_CMD_BSS_CTRL_T {
	UINT32                     u4CtrlFieldID;
	UINT8                      ucBssGroupID;
	UINT8                      ucCtrlStatus;
	UINT8                      ucReserve[2];
	UINT32                     u4ReserveDW;
	UINT32                     u4SingleFieldIDValue;
	BW_BSS_TOKEN_SETTING_T   arAllBssGroupMultiField[16];
	UINT8                      ucBssGroupQuantumTime[16];
} EXT_CMD_BSS_CTRL_T, *P_EXT_CMD_BSS_CTRL_T;

typedef enum _ENUM_EXT_CMD_BSSGROUP_CTRL_ID_T {
	ENUM_BSSGROUP_CTRL_ALL_ITEM_FOR_1_GROUP                     = 0x00,
	ENUM_BSSGROUP_CTRL_MIN_RATE_TOKEN_CFG_ITEM                  = 0x01,
	ENUM_BSSGROUP_CTRL_MAX_RATE_TOKEN_CFG_ITEM                  = 0x02,
	ENUM_BSSGROUP_CTRL_MIN_TOKEN_BUCKET_TIME_SIZE_CFG_ITEM      = 0x03,
	ENUM_BSSGROUP_CTRL_MIN_AIRTIME_TOKEN_CFG_ITEM               = 0x04,
	ENUM_BSSGROUP_CTRL_MIN_TOKEN_BUCKET_LENG_SIZE_CFG_ITEM = 0x05,
	ENUM_BSSGROUP_CTRL_MAX_TOKEN_BUCKET_TIME_SIZE_CFG_ITEM = 0x06,
	ENUM_BSSGROUP_CTRL_MAX_AIRTIME_TOKEN_CFG_ITEM               = 0x07,
	ENUM_BSSGROUP_CTRL_MAX_TOKEN_BUCKET_LENG_SIZE_CFG_ITEM = 0x08,
	ENUM_BSSGROUP_CTRL_MAX_WAIT_TIME_CFG_ITEM                   = 0x09,
	ENUM_BSSGROUP_CTRL_MAX_BACKLOG_SIZE_CFG_ITEM                = 0x0a,
	ENUM_BSSGROUP_CTRL_ALL_ITEM_FOR_ALL_GROUP                   = 0x10,

	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_BASE                    = 0x20,

	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_00                    = 0x20,
	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_01                    = 0x21,
	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_02                    = 0x22,
	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_03                    = 0x23,
	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_04                    = 0x24,
	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_05                    = 0x25,
	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_06                    = 0x26,
	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_07                    = 0x27,
	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_08                    = 0x28,
	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_09                    = 0x29,
	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0A                    = 0x2A,
	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0B                    = 0x2B,
	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0C                    = 0x2C,
	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0D                    = 0x2D,
	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0E                    = 0x2E,
	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0F                    = 0x2F,

	ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_ALL                     = 0x30,

} ENUM_EXT_CMD_BSSGROUP_CTRL_ID_T, P_ENUM_EXT_CMD_BSSGROUP_CTRL_ID_T;
/* end of EXT_CMD_ID_BSSGROUP_CTRL = 0x37 */

/*************************************************/
/* EXT_CMD_ID_VOW_FEATURE_CTRL = 0x38 */
/*************************************************/
typedef struct GNU_PACKED _EXT_CMD_VOW_FEATURE_CTRL_T {
	/* DW#0 */
	UINT16     u2IfApplyBss_0_to_16_CtrlFlag;                  /* BITWISE */
#ifdef RT_BIG_ENDIAN
	UINT16	   u2IfApplyEnbwCtrlFlag:1;
	UINT16	   u2IfApplyEnbwrefillFlag:1;
	UINT16	   u2IfApplyAirTimeFairnessFlag:1;
	UINT16	   u2IfApplyEnTxopNoChangeBssFlag:1;
	UINT16     u2Reserve_b26_to_b27Flag:2;
	UINT16     u2IfApplyWeightedAirTimeFairnessFlag:1;
	UINT16     u2Reserve_b22_to_b24Flag:3;
	UINT16	   u2IfApplyDbdc0SearchRuleFlag:1;
	UINT16	   u2IfApplyDbdc1SearchRuleFlag:1;
	UINT16	   u2Reserve_b19Flag:1;
	UINT16	   u2Reserve_b17_to_b18Flag:2;
	UINT16	   u2IfApplyRefillPerildFlag:1;
#else
	UINT16     u2IfApplyRefillPerildFlag:1;
	UINT16     u2Reserve_b17_to_b18Flag:2;
	UINT16     u2Reserve_b19Flag:1;
	UINT16     u2IfApplyDbdc1SearchRuleFlag:1;
	UINT16     u2IfApplyDbdc0SearchRuleFlag:1;
	UINT16     u2Reserve_b22_to_b24Flag:3;
	UINT16     u2IfApplyWeightedAirTimeFairnessFlag:1;
	UINT16     u2Reserve_b26_to_b27Flag:2;
	UINT16     u2IfApplyEnTxopNoChangeBssFlag:1;
	UINT16     u2IfApplyAirTimeFairnessFlag:1;
	UINT16     u2IfApplyEnbwrefillFlag:1;
	UINT16     u2IfApplyEnbwCtrlFlag:1;
#endif
	/* DW#1 */
	UINT16     u2IfApplyBssCheckTimeToken_0_to_16_CtrlFlag;    /* BITWISE */
	UINT16     u2Resreve1Flag;

	/* DW#2 */
	UINT16     u2IfApplyBssCheckLengthToken_0_to_16_CtrlFlag;  /* BITWISE */
	UINT16     u2Resreve2Flag;

	/* DW#3, 4 */
	UINT32     u2ResreveBackupFlag[2];

	/* DW#5 */
	UINT16     u2Bss_0_to_16_CtrlValue;                        /* BITWISE */
#ifdef RT_BIG_ENDIAN
	UINT16	   u2EnbwCtrlValue:1;
	UINT16	   u2EnbwrefillValue:1;
	UINT16	   u2AirTimeFairnessValue:1;
	UINT16	   u2EnTxopNoChangeBssValue:1;
	UINT16	   u2Reserve_b26_to_b27Value:2;
	UINT16	   u2WeightedAirTimeFairnessValue:1;
	UINT16	   u2Reserve_b22_to_b24Value:3;
	UINT16	   u2Dbdc0SearchRuleValue:1;
	UINT16	   u2Dbdc1SearchRuleValue:1;
	UINT16	   u2Reserve_b19Value:1;
	UINT16	   u2RefillPerildValue:3;
#else

	UINT16     u2RefillPerildValue:3;
	UINT16     u2Reserve_b19Value:1;
	UINT16     u2Dbdc1SearchRuleValue:1;
	UINT16     u2Dbdc0SearchRuleValue:1;
	UINT16     u2Reserve_b22_to_b24Value:3;
	UINT16     u2WeightedAirTimeFairnessValue:1;
	UINT16     u2Reserve_b26_to_b27Value:2;
	UINT16     u2EnTxopNoChangeBssValue:1;
	UINT16     u2AirTimeFairnessValue:1;
	UINT16     u2EnbwrefillValue:1;
	UINT16     u2EnbwCtrlValue:1;
#endif
	/* DW#6 */
	UINT16     u2BssCheckTimeToken_0_to_16_CtrlValue;    /* BITWISE */
	UINT16     u2Resreve1Value;

	/* DW#7 */
	UINT16     u2BssCheckLengthToken_0_to_16_CtrlValue;  /* BITWISE */
	UINT16     u2Resreve2Value;

	/* DW#8 */
#ifdef RT_BIG_ENDIAN
	UINT32	   u4Resreve1Value:1;
	UINT32	   u4VowKeepSettingBit:5;
	UINT32	   u4VowKeepSettingValue:1;
	UINT32	   u4IfApplyKeepVoWSettingForSerFlag:1;
	UINT32	   u4RxRifsModeforCckCtsValue:1;
	UINT32	   u4IfApplyRxRifsModeforCckCtsFlag:1;
	UINT32	   u4ApplyRxEifsToZeroValue:1;
	UINT32	   u4IfApplyRxEifsToZeroFlag:1;
	UINT32	   u4RtsFailedChargeDisValue:1;
	UINT32	   u4IfApplyRtsFailedChargeDisFlag:1;
	UINT32	   u4TxBackOffBoundValue:5;
	UINT32	   u4TxBackOffBoundEnable:1;
	UINT32	   u4IfApplyTxBackOffBoundFlag:1;
	UINT32	   u4TxMeasurementModeValue:1;
	UINT32	   u4IfApplyTxMeasurementModeFlag:1;
	UINT32	   u4TxCountValue:4;
	UINT32	   u4IfApplyTxCountModeFlag:1;
	UINT32	   u4KeepQuantumValue:1;
	UINT32	   u4IfApplyKeepQuantumFlag:1;
	UINT32	   u4RtsStaLockValue:1;
	UINT32	   u4IfApplyStaLockForRtsFlag:1;
#else
	UINT32     u4IfApplyStaLockForRtsFlag:1;
	UINT32     u4RtsStaLockValue:1;
	UINT32     u4IfApplyKeepQuantumFlag:1;
	UINT32     u4KeepQuantumValue:1;
	UINT32     u4IfApplyTxCountModeFlag:1;
	UINT32     u4TxCountValue:4;
	UINT32     u4IfApplyTxMeasurementModeFlag:1;
	UINT32     u4TxMeasurementModeValue:1;
	UINT32     u4IfApplyTxBackOffBoundFlag:1;
	UINT32     u4TxBackOffBoundEnable:1;
	UINT32     u4TxBackOffBoundValue:5; /* ms */
	UINT32     u4IfApplyRtsFailedChargeDisFlag:1;      /* don't charge airtime when RTS failed */
	UINT32     u4RtsFailedChargeDisValue:1;
	UINT32     u4IfApplyRxEifsToZeroFlag:1;
	UINT32     u4ApplyRxEifsToZeroValue:1;
	UINT32     u4IfApplyRxRifsModeforCckCtsFlag:1;
	UINT32     u4RxRifsModeforCckCtsValue:1;
	UINT32     u4IfApplyKeepVoWSettingForSerFlag:1;
	UINT32     u4VowKeepSettingValue:1;
	UINT32     u4VowKeepSettingBit:5;
	UINT32     u4Resreve1Value:1;
#endif
	/* DW#9 */
#ifdef RT_BIG_ENDIAN
	UINT32     u4ResreveBackupValue:21;
	UINT32     u4VowSchedulePolicy: 2;
	UINT32     u4VowScheduleType: 2;
	UINT32     u4IfApplyVowSchCtrl: 1;
	UINT32     u4DbgPrnLvl: 2;
	UINT32     u4SplStaNumValue:3;
	UINT32     u4IfApplySplFlag:1;
#else
	UINT32     u4IfApplySplFlag:1;
	UINT32     u4SplStaNumValue:3;
	UINT32     u4DbgPrnLvl: 2;
	UINT32     u4IfApplyVowSchCtrl: 1;
	UINT32     u4VowScheduleType: 2;
	UINT32     u4VowSchedulePolicy: 2;
	UINT32     u4ResreveBackupValue: 21;
#endif

} EXT_CMD_VOW_FEATURE_CTRL_T, *P_EXT_CMD_VOW_FEATURE_CTRL_T;

typedef enum _ENUM_UMAC_VOW_REFILL_PERIOD_T {
	ENUM_UMAC_VOW_REFILL_IDX_0_1_US = 0,
	ENUM_UMAC_VOW_REFILL_IDX_1_2_US = 1,
	ENUM_UMAC_VOW_REFILL_IDX_2_4_US = 2,
	ENUM_UMAC_VOW_REFILL_IDX_3_8_US = 3,
	ENUM_UMAC_VOW_REFILL_IDX_4_16_US = 4,
	ENUM_UMAC_VOW_REFILL_IDX_5_32_US = 5,
	ENUM_UMAC_VOW_REFILL_IDX_6_64_US = 6,
	ENUM_UMAC_VOW_REFILL_IDX_7_128_US = 7,
	ENUM_UMAC_VOW_REFILL_IDX_TOTAL_NUM = 8,
} ENUM_UMAC_VOW_REFILL_PERIOD_T, *P_ENUM_UMAC_VOW_REFILL_PERIOD_T;
/* end of EXT_CMD_ID_VOW_FEATURE_CTRL = 0x38 */

/*************************************************/
/*       EXT_CMD_ID_RX_AIRTIME_CTRL = 0x4a       */
/*************************************************/
/* MIB IFS related data type */
typedef struct GNU_PACKED _RX_AT_IFS_CFG_T {
	UINT8 ucAC0Ifs;
	UINT8 ucAC1Ifs;
	UINT8 ucAC2Ifs;
	UINT8 ucAC3Ifs;
}  RX_AT_IFS_CFG_T, *P_RX_AT_IFS_CFG_T;

typedef enum _ENUM_RX_AT_AC_Q_MASK_T {
	ENUM_RX_AT_AC_Q0_MASK_T = BIT(0),
	ENUM_RX_AT_AC_Q1_MASK_T = BIT(1),
	ENUM_RX_AT_AC_Q2_MASK_T = BIT(2),
	ENUM_RX_AT_AC_Q3_MASK_T = BIT(3),
	ENUM_RX_AT_AC_ALL_MASK_T = (BIT(3)|BIT(2)|BIT(1)|BIT(0)),
} ENUM_RX_AT_AC_Q_MASK_T, *P_ENUM_RX_AT_AC_Q_MASK_T;

typedef enum _ENUM_RX_AT_WMM_GROUP_IDX_T {

	ENUM_RX_AT_WMM_GROUP_0 = 0,             /* MIBTIME1 */
	ENUM_RX_AT_WMM_GROUP_1 = 1,             /* MIBTIME2 */
	ENUM_RX_AT_WMM_GROUP_2 = 2,             /* MIBTIME3 */
	ENUM_RX_AT_WMM_GROUP_3 = 3,             /* MIBTIME4 */
	ENUM_RX_AT_WMM_GROUP_PEPEATER = 4,      /* MIBTIME7 */
	ENUM_RX_AT_WMM_GROUP_STA = 5,           /* MIBTIME8 */
	ENUM_RX_AT_NON_QOS = 6,                 /* MIBTIME0 */
	ENUM_RX_AT_OBSS = 7,                    /* MIBTIME0 */

} ENUM_RX_AT_WMM_GROUP_IDX_T, *P_ENUM_RX_AT_WMM_GROUP_IDX_T;

/* RX AT Backoff related data type */

typedef struct GNU_PACKED _RX_AT_BACKOFF_CFG_T {
	UINT16 u2AC0Backoff;
	UINT16 u2AC1Backoff;
	UINT16 u2AC2Backoff;
	UINT16 u2AC3Backoff;
}  RX_AT_BACKOFF_CFG_T, *P_RX_AT_BACKOFF_CFG_T;

/* Compensate mode enum definition */
typedef enum _ENUM_RX_AT_SW_COMPENSATE_MODE_T {
	ENUM_RX_AT_SW_COMPENSATE_OBSS = 1,
	ENUM_RX_AT_SW_COMPENSATE_NON_WIFI = 2,
} ENUM_RX_AT_SW_COMPENSATE_MODE_T, *P_ENUM_RX_AT_SW_COMPENSATE_MODE_T;

/* ENUM list for u4CtrlFieldID */
typedef enum _ENUM_RX_AT_CTRL_FIELD_T {
	ENUM_RX_AT_FEATURE_CTRL = 1,
	ENUM_RX_AT_BITWISE_CTRL = 2,
	ENUM_RX_AT_TIMER_VALUE_CTRL = 3,
	EMUM_RX_AT_REPORT_CTRL = 4,
} ENUM_RX_AT_CTRL_FIELD_T, *P_ENUM_RX_AT_CTRL_FIELD_T;

/* ENUM list for (u4CtrlSubFieldID == ENUM_RX_AT_FEATURE_CTRL) */
typedef enum _ENUM_RX_AT_FEATURE_SUB_FIELD_T {
	ENUM_RX_AT_FEATURE_SUB_TYPE_AIRTIME_EN = 1,
	ENUM_RX_AT_FEATURE_SUB_TYPE_MIBTIME_EN = 2,
	ENUM_RX_AT_FEATURE_SUB_TYPE_EARLYEND_EN = 3,
} ENUM_RX_AT_FEATURE_SUB_FIELD_T, *P_ENUM_RX_AT_FEATURE_SUB_FIELD_T;

/* ENUM list for (u4CtrlSubFieldID == ENUM_RX_AT_FEATURE_CTRL) */
typedef enum _ENUM_RX_AT_BITWISE_SUB_FIELD_T {
	ENUM_RX_AT_BITWISE_SUB_TYPE_AIRTIME_CLR = 1,
	ENUM_RX_AT_BITWISE_SUB_TYPE_MIBTIME_CLR = 2,
	ENUM_RX_AT_BITWISE_SUB_TYPE_STA_WMM_CTRL = 3,
	ENUM_RX_AT_BITWISE_SUB_TYPE_MBSS_WMM_CTRL = 4,
} ENUM_RX_AT_BITWISE_SUB_FIELD_T, *_ENUM_RX_AT_BITWISE_SUB_FIELD_T;

/* ENUM list for (u4CtrlSubFieldID == ENUM_RX_AT_TIMER_VALUE_CTRL) */
typedef enum _ENUM_RX_AT_TIME_VALUE_SUB_FIELD_T {
	ENUM_RX_AT_TIME_VALUE_SUB_TYPE_ED_OFFSET_CTRL = 1,
	ENUM_RX_AT_TIME_VALUE_SUB_TYPE_SW_TIMER = 2,
	ENUM_RX_AT_TIME_VALUE_SUB_TYPE_BACKOFF_TIMER = 3,
	ENUM_RX_AT_TIME_VALUE_SUB_TYPE_IFS_TIMER = 4,
} ENUM_RX_AT_TIME_VALUE_SUB_FIELD_T, *P_ENUM_RX_AT_TIME_VALUE_SUB_FIELD_T;

/* ENUM list for (u4CtrlSubFieldID == EMUM_RX_AT_REPORT_CTRL) */
typedef enum _ENUM_RX_AT_TIME_REPORT_SUB_FIELD_T {
	ENUM_RX_AT_REPORT_SUB_TYPE_RX_NONWIFI_TIME = 1,
	ENUM_RX_AT_REPORT_SUB_TYPE_RX_OBSS_TIME = 2,
	ENUM_RX_AT_REPORT_SUB_TYPE_MIB_OBSS_TIME = 3,
	ENUM_RX_AT_REPORT_SUB_TYPE_PER_STA_RX_TIME = 4,
} ENUM_RX_AT_TIME_REPORT_SUB_FIELD_T, *P_ENUM_RX_AT_TIME_REPORT_SUB_FIELD_T;

typedef struct GNU_PACKED _RX_AT_FEATURE_SUB_FIELD_CTRL_T {
	BOOLEAN                 fgRxAirTimeEn;
	BOOLEAN                 fgRxMibTimeEn;
	BOOLEAN                 fgRxEarlyEndEn;
	UINT8                  ucReserve[1];
	UINT32                 u4ReserveDW[2];
} RX_AT_FEATURE_SUB_FIELD_CTRL_T, *P_RX_AT_FEATURE_SUB_FIELD_CTRL_T;

typedef struct GNU_PACKED _RX_AT_BITWISE_SUB_FIELD_CTRL_T {
	/* DW#0 */
	BOOLEAN                 fgRxAirTimeClrEn;
	BOOLEAN                 fgRxMibTimeClrEn;
	UINT8                  ucReserve[2];

	/* DW#1 */
	UINT8                  ucOwnMacID;
	BOOLEAN                 fgtoApplyWm00to03MibCfg;
	UINT8                  ucReserve1[2];

	/* DW#2 */
	UINT8                  ucMbssGroup;
	UINT8                  ucWmmGroup;
	UINT8                  ucReserve2[2];

	/* DW#3,4 */
	UINT32                 u4ReserveDW[2];
} RX_AT_BITWISE_SUB_FIELD_CTRL_T, *P_RX_AT_BITWISE_SUB_FIELD_CTRL_T;

typedef struct GNU_PACKED _RX_AT_TIMER_VALUE_SUB_FIELD_CTRL_T {
	/* DW#0 */
	UINT8	ucEdOffsetValue;
	UINT8	ucReserve0[3];

	/* DW#1 */
	UINT8	rCompensateMode;
	UINT8	rRxBand;
	UINT8	ucSwCompensateTimeValue;
	UINT8	ucReserve1;

	/* D2#2/3/4 */
	RX_AT_BACKOFF_CFG_T	rRxATBackOffCfg;
	UINT8	rRxATBackoffWmmGroupIdx;
	UINT8	rRxAtBackoffAcQMask;
	UINT8	ucReserve2[2];

	/* DW#5/6 */
	RX_AT_IFS_CFG_T	rRxATIfsCfg;
	UINT8	rRxATIfsWmmGroupIdx;
	UINT8	rRxAtIfsAcQMask;
	UINT8	ucReserve3[2];

	UINT32	u4ReserveDW[2];
} RX_AT_TIMER_VALUE_SUB_FIELD_CTRL_T, *P_RX_AT_TIMER_VALUE_SUB_FIELD_CTRL_T;

typedef struct GNU_PACKED _RX_AT_REPORT_SUB_FIELD_CTRL_T {
	/* DW# 0/1 */
	UINT32	u4RxNonWiFiBandTimer;
	UINT8	ucRxNonWiFiBandIdx;
	UINT8	ucReserve0[3];

	/* DW# 2/3 */
	UINT32	u4RxObssBandTimer;
	UINT8	ucRxObssBandIdx;
	UINT8	ucReserve1[3];

	/* DW# 4/5 */
	UINT32	u4RxMibObssBandTimer;
	UINT8	ucRxMibObssBandIdx;
	UINT8	ucReserve2[3];

	/* DW# 6/7/8/9/10 */
	UINT32	u4StaAcRxTimer[4];
	UINT8	ucStaIDL;	/* #256STA - Low Byte */
	UINT8	ucStaIDHnVer;	/* #256STA - High Byte and Version */
	UINT8	ucReserve3[2];

} RX_AT_REPORT_SUB_FIELD_CTRL_T, *P_RX_AT_REPORT_SUB_FIELD_CTRL_T;

typedef union GNU_PACKED _RX_AT_GENERAL_CTRL_FIELD_T {
	RX_AT_FEATURE_SUB_FIELD_CTRL_T          rRxAtFeatureSubCtrl;
	RX_AT_BITWISE_SUB_FIELD_CTRL_T          rRxAtBitWiseSubCtrl;
	RX_AT_TIMER_VALUE_SUB_FIELD_CTRL_T      rRxAtTimeValueSubCtrl;
	RX_AT_REPORT_SUB_FIELD_CTRL_T           rRxAtReportSubCtrl;
} RX_AT_GENERAL_CTRL_FIELD_T, *P_RX_AT_GENERAL_CTRL_FIELD_T;

typedef struct GNU_PACKED _EXT_CMD_RX_AT_CTRL_T {
	UINT16                     u4CtrlFieldID;
	UINT16                     u4CtrlSubFieldID;
	UINT32                     u4CtrlSetStatus;
	UINT32                     u4CtrlGetStatus;
	UINT8                      ucReserve[4];
	UINT32                     u4ReserveDW[2];

	RX_AT_GENERAL_CTRL_FIELD_T  rRxAtGeneralCtrl;

} EXT_CMD_RX_AT_CTRL_T, *P_EXT_CMD_RX_AT_CTRL_T;

/* end of EXT_CMD_ID_RX_AIRTIME_CTRL = 0x4a */

/*************************************************/
/*      EXT_CMD_ID_AT_PROC_MODULE = 0x4b         */
/*************************************************/
typedef struct GNU_PACKED _AT_ESTIMATE_SUB_FIELD_CTRL_T {
	/* DW#0 */
	BOOLEAN         fgAtEstimateOnOff;
	UINT8          ucReserve;
	UINT16         u2AtEstMonitorPeriod;

	/* DW#1, 2~9 */
	UINT32         u4GroupRatioBitMask;
	UINT16         u2GroupMaxRatioValue[16];
	UINT16         u2GroupMinRatioValue[16];

	/* DW#10 */
	UINT8          ucGrouptoSelectBand;
	UINT8          ucBandSelectedfromGroup;
	UINT8          ucReserve1[2];

} AT_ESTIMATE_SUB_FIELD_CTRL_T, *P_AT_ESTIMATE_SUB_FIELD_CTRL_T;

typedef struct GNU_PACKED _AT_BAD_NODE_SUB_FIELD_CTRL_T {
	/* DW#0 */
	BOOLEAN         fgAtBadNodeOnOff;
	UINT8          ucReserve;
	UINT16         u2AtBadNodeMonitorPeriod;

	/* DW#1 */
	UINT8          ucFallbackThreshold;
	UINT8         ucTxPERThreshold;
	UINT8          ucReserve1[2];

} AT_BAD_NODE_SUB_FIELD_CTRL_T, *P_AT_BAD_NODE_SUB_FIELD_CTRL_T;

typedef union _AT_PROC_GENERAL_CTRL_FIELD_T {
	AT_ESTIMATE_SUB_FIELD_CTRL_T          rAtEstimateSubCtrl;
	AT_BAD_NODE_SUB_FIELD_CTRL_T          rAtBadNodeSubCtrl;
} AT_PROC_GENERAL_CTRL_FIELD_T, *P_AT_PROC_GENERAL_CTRL_FIELD_T;

typedef struct GNU_PACKED _EXT_CMD_AT_PROC_MODULE_CTRL_T {
	UINT16                      u4CtrlFieldID;
	UINT16                      u4CtrlSubFieldID;
	UINT32                      u4CtrlSetStatus;
	UINT32                      u4CtrlGetStatus;
	UINT8                       ucReserve[4];
	UINT32                      u4ReserveDW[2];

	AT_PROC_GENERAL_CTRL_FIELD_T rAtProcGeneralCtrl;
} EXT_CMD_AT_PROC_MODULE_CTRL_T, *P_EXT_CMD_AT_PROC_MODULE_CTRL_T;

typedef enum _ENUM_AT_ESTIMATE_SUB_FIELD_T {
	ENUM_AT_PROC_EST_FEATURE_CTRL = 1,
	ENUM_AT_PROC_EST_MONITOR_PERIOD_CTRL = 2,
	ENUM_AT_PROC_EST_GROUP_RATIO_CTRL = 3,
	ENUM_AT_PROC_EST_GROUP_TO_BAND_MAPPING = 4,
} ENUM_AT_ESTIMATE_SUB_FIELD_T, *P_ENUM_AT_ESTIMATE_SUB_FIELD_T;

typedef enum _ENUM_AT_BAD_NODE_SUB_FIELD_T {
	ENUM_AT_PROC_BAD_NODE_FEATURE_CTRL = 1,
	ENUM_AT_PROC_BAD_NODE_MONITOR_PERIOD_CTRL = 2,
	ENUM_AT_PROC_BAD_NODE_FALLBACK_THRESHOLD = 3,
	ENUM_AT_PROC_BAD_NODE_PER_THRESHOLD = 4,
} ENUM_AT_BAD_NODE_SUB_FIELD_T, *P_ENUM_AT_BAD_NODE_SUB_FIELD_T;

/* ENUM list for u4CtrlFieldID in AT PROCESS control field */
typedef enum _ENUM_AT_RPOCESS_FIELD_T {
	ENUM_AT_RPOCESS_ESTIMATE_MODULE_CTRL = 1,
	ENUM_AT_RPOCESS_BAD_NODE_MODULE_CTRL = 2,
} ENUM_AT_RPOCESS_FIELD_T, *P_ENUM_AT_RPOCESS_FIELD_T;

/* end of EXT_CMD_ID_AT_PROC_MODULE = 0x4b */

typedef enum _ENUM_DBDC_BN_T {
	ENUM_BAND_0,
	ENUM_BAND_1,
	ENUM_BAND_NUM,
	ENUM_BAND_ALL
} ENUM_DBDC_BN_T, *P_ENUM_DBDC_BN_T;

#define SET_RX_MAX_PKT_LEN(x)	((x) << 2)
typedef struct _EXT_CMD_RX_MAX_PKT_LEN_T {
	UINT16 u2Tag; /* EXT_CMD_TAG_RXMAXLEN */
	UINT16 u2RxMaxPktLength; /* CR unit is DWORD(4B) */
} EXT_CMD_RX_MAX_PKT_LEN_T, *P_EXT_CMD_RX_MAX_PKT_LEN_T;

typedef struct GNU_PACKED _CMD_SLOT_TIME_SET_T {
	UINT8   ucSlotTime;
	UINT8   ucSifs;
	UINT8   ucRifs;
	UINT8 ucOldEifs;		/* occupied for backward compatible */
	UINT16  u2Eifs;
	UINT8 ucBandNum;
	UINT8 aucReserved[5];
} CMD_SLOT_TIME_SET_T, *P_CMD_SLOT_TIME_SET_T;

typedef struct GNU_PACKED _CMD_POWER_PWERCENTAGE_LEVEL_SET_T
{
    INT8   cPowerDropLevel;
    UINT8  ucBand;
    UINT8  aucReserved[10];
} CMD_POWER_PWERCENTAGE_LEVEL_SET_T, *P_CMD_POWER_PWERCENTAGE_LEVEL_SET_T;

#define TX_STREAM	0x0
#define RX_STREAM	0x1
#define SET_TR_STREAM_NUM(x, y)	(((x)<<16)|(y))

enum EXT_CMD_ATE_CFG_ONOFF_TYPE {
	EXT_CFG_ONOFF_TSSI = 0x0,
	EXT_CFG_ONOFF_DPD = 0x1,
	EXT_CFG_ONOFF_RATE_POWER_OFFSET = 0x2,
	EXT_CFG_ONOFF_TEMP_COMP = 0x3,
	EXT_CFG_ONOFF_THERMAL_SENSOR = 0x4,
	EXT_CFG_ONOFF_TXPOWER_CTRL = 0x5,
	EXT_CFG_ONOFF_SINGLE_SKU = 0x6,
	EXT_CFG_ONOFF_POWER_PERCENTAGE = 0x7,
	EXT_CFG_ONOFF_BF_BACKOFF = 0x8,
};

#ifdef CONFIG_HW_HAL_OFFLOAD
enum EXT_CMD_ATE_TRX_SET_IDX {
	EXT_ATE_SET_RESERV = 0x0,
	EXT_ATE_SET_TRX = 0x1,
	EXT_ATE_SET_RX_PATH = 0x2,
	EXT_ATE_SET_RX_FILTER = 0x3,
	EXT_ATE_SET_TX_STREAM = 0x4,
	EXT_ATE_SET_TSSI = 0x5,
	EXT_ATE_SET_DPD = 0x6,
	EXT_ATE_SET_RATE_POWER_OFFSET = 0X7,
	EXT_ATE_SET_THERNAL_COMPENSATION = 0X8,
	EXT_ATE_SET_RX_FILTER_PKT_LEN = 0x09,
	EXT_ATE_SET_FREQ_OFFSET = 0x0A,
	EXT_ATE_GET_FREQ_OFFSET = 0x0B,
	EXT_ATE_GET_TSSI = 0xC,
	EXT_ATE_GET_DPD = 0xD,
	EXT_ATE_GET_THERNAL_COMPENSATION = 0XE,
	EXT_ATE_SET_RXV_INDEX = 0x0F,
	EXT_ATE_SET_FAGC_PATH = 0x10,
	EXT_ATE_SET_PHY_COUNT = 0x11,
	EXT_ATE_SET_ANTENNA_PORT = 0x12,
	EXT_ATE_SET_SLOT_TIME = 0x13,
	EXT_ATE_CFG_THERMAL_ONOFF = 0x14,
	EXT_ATE_SET_TX_POWER_CONTROL_ALL_RF = 0x15,
	EXT_ATE_GET_RATE_POWER_OFFSET = 0x16,
	EXT_ATE_SET_SINGLE_SKU = 0x18,
	EXT_ATE_SET_POWER_PERCENTAGE = 0x19,
	EXT_ATE_SET_BF_BACKOFF = 0x1a,
	EXT_ATE_SET_POWER_PERCENTAGE_LEVEL = 0x1b,
	EXT_ATE_SET_CLEAN_PERSTA_TXQUEUE = 0x1c,
	ENUM_ATE_SET_AMPDU_WTBL = 0x1d,
	ENUM_ATE_SET_MU_RX_AID = 0x1e,
	EXT_ATE_SET_PHY_MANUAL_TX = 0x1f
};

typedef struct GNU_PACKED _EXT_EVENT_ATE_TEST_MODE_T {
	UINT8 ucAteIdx;
	UINT8 aucReserved[3];
	UINT8 aucAteResult[0];
} EXT_EVENT_ATE_TEST_MODE_T, *P_EXT_EVENT_ATE_TEST_MODE_T;

typedef struct _GET_FREQ_OFFSET_T {
	UINT32 u4FreqOffset;
} GET_FREQ_OFFSET_T, *P_GET_FREQ_OFFSET_T;

typedef struct _GET_TSSI_STATUS_T {
	UINT8 ucEnable;
	UINT8 ucBand;
	UINT8 aucReserved[2];
} GET_TSSI_STATUS_T, *P_GET_TSSI_STATUS_T;

typedef struct _GET_DPD_STATUS_T {
	UINT8 ucEnable;
	UINT8 ucBand;
	UINT8 aucReserved[2];
} GET_DPD_STATUS_T, *P_GET_DPD_STATUS_T;

typedef struct _GET_THERMO_COMP_STATUS_T {
	UINT8 ucEnable;
	UINT8 aucReserved[3];
} GET_THERMO_COMP_STATUS_T, *P_GET_THERMO_COMP_STATUS_T;

typedef struct GNU_PACKED _EVENT_EXT_GET_FREQOFFSET_T {
	UINT8  ucAteTestModeEn;
	UINT8  ucAteIdx;
	UINT8  aucReserved[2];
	UINT32 u4FreqOffset;
} EVENT_EXT_GET_FREQOFFSET_T;

typedef struct _ATE_TEST_SET_TX_STREAM_T {
	UINT8  ucStreamNum;
	UINT8  ucBand;
	UINT8  aucReserved[2];
} ATE_TEST_SET_TX_STREAM_T, *P_ATE_TEST_SET_TX_STREAM_T;

typedef struct _ATE_TEST_SET_RX_FILTER_T {
	UINT8  ucPromiscuousMode;
	UINT8  ucReportEn;
	UINT8  ucBand;
	UINT8  ucReserved;
	UINT32 u4FilterMask;
	UINT8  aucReserved[4];
} ATE_TEST_SET_RX_FILTER_T, *P_ATE_TEST_SET_RX_FILTER_T;

typedef struct _ATE_TEST_SET_RX_PATH_T {
	UINT8  ucType;
	UINT8  ucBand;
	UINT8  aucReserved[2];
} ATE_TEST_SET_RX_PATH_T, *P_ATE_TEST_SET_RX_PATH_T;

typedef struct _ATE_TEST_SET_TRX_T {
	UINT8  ucType;
	UINT8  ucEnable;
	UINT8  ucBand;
	UINT8  ucReserved;
} ATE_TEST_SET_TRX_T, *P_ATE_TEST_SET_TRX_T;

typedef struct _RF_TEST_ON_OFF_SETTING_T {
	UINT8 ucEnable;
	UINT8 ucBand;
	UINT8 aucReserved[2];
} RF_TEST_ON_OFF_SETTING_T, *P_RF_TEST_ON_OFF_SETTING_T;

typedef struct _CFG_RX_FILTER_PKT_LEN_T {
	UINT8 ucEnable;
	UINT8 ucBand;
	UINT8 aucReserved[2];
	UINT32 u4RxPktLen;
} RX_FILTER_PKT_LEN_T, *P_RX_FILTER_PKT_LEN_T;

typedef struct _CFG_PHY_SETTING_RXV_IDX_T {
	UINT8 ucValue1;
	UINT8 ucValue2;
	UINT8 ucDbdcIdx;
	UINT8 ucReserved;
} CFG_PHY_SETTING_RXV_IDX_T, *P_CFG_PHY_SETTING_RXV_IDX_T;

typedef struct _CFG_PHY_SETTING_RSSI_PATH_T {
	UINT8 ucValue;
	UINT8 ucDbdcIdx;
	UINT8 aucReserved[2];
} CFG_PHY_SETTING_RSSI_PATH_T, *P_CFG_PHY_SETTING_RSSI_PATH_T;

typedef struct _CFG_RF_ANT_PORT_SETTING_T {
	UINT8  ucRfModeMask;
	UINT8  ucRfPortMask;
	UINT8  ucAntPortMask;
	UINT8  aucReserved[1];
} CFG_RF_ANT_PORT_SETTING_T, *P_CFG_RF_ANT_PORT_SETTING_T;

typedef struct _ANT_ID_CONFIG_T {
    UINT8                 ucANTIDSts0;
    UINT8                 ucANTIDSts1;
    UINT8                 ucANTIDSts2;
    UINT8                 ucANTIDSts3;
} ANT_ID_CONFIG_T, *P_ANT_ID_CONFIG_T;

typedef struct _HT_CAP_T {
    BOOLEAN               fgIsHT;
    BOOLEAN               fgLDPC;
    UINT8                 ucAmpduFactor;
    UINT8                 ucMMSS;
} HT_CAP_T, *P_HT_CAP_T;

typedef struct _VHT_CAP_T {
    BOOLEAN               fgIsVHT;
    BOOLEAN               fgVhtLDPC;
    BOOLEAN               fgDynBw;
    BOOLEAN               fgTxopPS;
} VHT_CAP_T, *P_VHT_CAP_T;

typedef struct _ANT_CAP_T {
	UINT8                  ucSpe;
    ANT_ID_CONFIG_T        AntIDConfig;
} ANT_CAP_T, *P_ANT_CAP_T;

typedef struct _BA_CAP_T {
	UINT8                  ucBaEn;
    UINT8				   ucBaSize;
} BA_CAP_T, *P_BA_CAP_T;

typedef struct _RATE_CAP_T {
	UINT8                  ucFcap;
	UINT8                  ucMode;
	UINT8                  ucStbc;
	UINT8                  ucSgi;
	UINT8                  ucBw;
	UINT8                  ucPreamble;
	UINT8                  ucLdpc;
	UINT8                  ucNss;
    BOOLEAN                fgG2;
    BOOLEAN                fgG4;
    BOOLEAN                fgG8;
    BOOLEAN                fgG16;
	UINT16				   au2RateCode;
} RATE_CAP_T, *P_RATE_CAP_T;

typedef struct _ATE_TEST_SET_CLEAN_PERSTA_TXQUEUE_T {
	BOOLEAN fgStaPauseEnable;
	UINT8  ucStaID;		/* #256STA */
	UINT8  ucBand;
	UINT8  aucReserved[1];	/* used for omac idx*/
} ATE_TEST_SET_CLEAN_PERSTA_TXQUEUE_T, *P_ATE_TEST_SET_CLEAN_PERSTA_TXQUEUE_T;

typedef struct _ATE_TEST_SET_AMPDU_WTBL_T {
	HT_CAP_T         rWtblHt;
	VHT_CAP_T        rWtblVht;
	ANT_CAP_T        rWtblAnt;
	BA_CAP_T		 rWtblBa;
	RATE_CAP_T		 rWtblRate;
	UINT8            ucIPsm;
	UINT8            ucQos;
} ATE_TEST_SET_AMPDU_WTBL_T, *P_ATE_TEST_SET_AMPDU_WTBL_T;

struct _ATE_TEST_SET_MURX_AID_T {
	UINT32 band_idx;
	UINT16 aid;
};

struct _PHY_MANUAL_TX_PARAM {
	BOOLEAN start;
	USHORT ipg;
	USHORT pkt_cnt;
};

struct _PHY_MANUAL_HETB_TX_CG1_DDW1 {
    UINT8 ant_id;
    UINT8 total_pwr_dbm;
    UINT8 doppler;
    UINT8 user_cnt;
    UINT8 pwr_dbm;
    UINT8 tx_mode;
    UINT8 bw;
    UINT8 stbc;
    UINT8 ru_mu;
    UINT8 spatial_ext;
};

struct _PHY_MANUAL_HETB_TX_CG1_DDW2 {
    UINT8 trigger_frame_ind;
    UINT8 format;
    UINT8 ltf;
    UINT8 gi;
    UINT8 sig_a_rsvd;
    UINT8 total_pwr_ind;
    UINT8 tf_rsp_ind;
    UINT8 cfo_ind;
    UINT8 precomp_cfo_idx;
};

struct _PHY_MANUAL_HETB_TX_CG2_DDW1 {
    UINT8 lg_txlen_exsym;
    UINT8 spatial_reuse[4];
    UINT8 mimo_ltf;
    UINT8 txop;
    UINT8 afactor;
    UINT8 bss_color;
    UINT8 pe_disamb;
    UINT8 ltf_symbol;
    USHORT ppdu_sym_cnt;
    UINT8 max_pe;
    UINT8 ldpc_extra_symbol;
    USHORT lg_txlen;
};

struct _PHY_MANUAL_HETB_TX_CG2_DDW2 {
	UINT8 ru_allocation[8];
};

struct _PHY_MANUAL_HETB_TX_USER_G1 {
    UINT8 start_spatial_stream;
    UINT8 pfmu_idx_ibf;
    UINT8 pfmu_idx_ebf;
    UINT8 ru_size;
    UINT8 fec_coding;
    UINT8 ibf_fisrt;
    UINT8 ibf;
    UINT8 ebf;
    UINT8 nsts;
    UINT8 rate;
};

struct _PHY_MANUAL_HETB_TX_USER_G2 {
    USHORT aid;
    UINT8 sigb_ch_idx;
    UINT32 length;
};

struct _ATE_TEST_SET_PHY_MANUAL_TX {
	UINT8 band_idx;
	struct _PHY_MANUAL_TX_PARAM param;
	struct _PHY_MANUAL_HETB_TX_CG1_DDW1 cg1_ddw1;
	struct _PHY_MANUAL_HETB_TX_CG1_DDW2 cg1_ddw2;
	struct _PHY_MANUAL_HETB_TX_CG2_DDW1 cg2_ddw1;
	struct _PHY_MANUAL_HETB_TX_CG2_DDW2 cg2_ddw2;
	struct _PHY_MANUAL_HETB_TX_USER_G1 user_g1;
	struct _PHY_MANUAL_HETB_TX_USER_G2 user_g2;
};

typedef struct _ATE_TEST_FREQOFFSET_T {
	UINT8			ucBandIdx;
	UINT32			u4FreqOffset;
} ATE_TEST_FREQOFFSET_T, *P_ATE_TEST_FREQOFFSET_T;

typedef struct GNU_PACKED _EXT_CMD_ATE_TEST_MODE_T {
	UINT8  ucAteTestModeEn;
	UINT8  ucAteIdx;
	UINT8  aucReserved[2];
	union {
		UINT32 u4Data;
		ATE_TEST_SET_TRX_T rAteSetTrx;
		ATE_TEST_SET_RX_PATH_T rAteSetRxPath;
		ATE_TEST_SET_RX_FILTER_T rAteSetRxFilter;
		ATE_TEST_SET_TX_STREAM_T rAteSetTxStream;
		RF_TEST_ON_OFF_SETTING_T rCfgOnOff;
		RX_FILTER_PKT_LEN_T rRxFilterPktLen;
		CFG_PHY_SETTING_RXV_IDX_T rSetRxvIdx;
		CFG_PHY_SETTING_RSSI_PATH_T rSetFagcRssiPath;
		RF_TEST_ON_OFF_SETTING_T rPhyStatusCnt;
		CFG_RF_ANT_PORT_SETTING_T rCfgRfAntPortSetting;
		CMD_SLOT_TIME_SET_T rSlotTimeSet;
		CMD_POWER_PWERCENTAGE_LEVEL_SET_T rPowerLevelSet;
		ATE_TEST_SET_CLEAN_PERSTA_TXQUEUE_T rAteSetCleanPerStaTxQueue;
		ATE_TEST_SET_AMPDU_WTBL_T rAteSetAmpduWtbl;
		ATE_TEST_FREQOFFSET_T rFreqOffset;
		struct _ATE_TEST_SET_MURX_AID_T set_mu_rx_aid;
		struct _ATE_TEST_SET_PHY_MANUAL_TX set_phy_manual_tx;
	} Data;
} EXT_CMD_ATE_TEST_MODE_T, *P_EXT_CMD_ATE_TEST_MODE_T;
#endif /* CONFIG_HW_HAL_OFFLOAD */

typedef struct GNU_PACKED _CMD_MCU_CLK_SWITCH_DISABLE_T {
	UINT8 disable;
	UINT8 aucReserved[3];
} CMD_MCU_CLK_SWITCH_DISABLE_T, *P_CMD_MCU_CLK_SWITCH_DISABLE_T;

typedef struct GNU_PACKED _EXT_CMD_SNIFFER_MODE_T {
	UINT8  ucSnifferEn;
	UINT8  ucDbdcIdx;
	UINT8  aucReserved[6];
} EXT_CMD_SNIFFER_MODE_T, *P_EXT_CMD_SNIFFER_MODE_T;

typedef struct GNU_PACKED _EXT_CMD_TR_STREAM_T {
	UINT16 u2Tag; /* EXT_CMD_TAG_TR_STREAM */
	UINT16 u2TRStreamNum; /* [31..16] Tx:1/Rx:0, [15..0] s1:0/s2:1/s3:2 */
} EXT_CMD_TR_STREAM_T, *P_EXT_CMD_TR_STREAM_T;

typedef struct GNU_PACKED _EXT_CMD_BA_CONTROL_T {
	UINT16 u2Tag; /* EXT_CMD_TAG_UPDATE_BA */
	BOOLEAN bIsAdd; /* BOOLEAN in host is 1Byte */
	UINT8 ucWcidL;	/* #STA256 - Low Byte */
	UINT8 ucTid;
	UINT16 u2BaWinSize;
	UINT8 ucBaSessionType;
	UINT8 aucPeerAddr[MAC_ADDR_LEN];
	UINT16 u8Sn;
	UINT8 ucWcidHnVer;	/* #STA256 - High Byte and Version */
} EXT_CMD_CONTROL_BA_T, *P_EXT_CMD_CONTROL_BA_T;

#define MT_UPLOAD_FW_UNIT (1024 * 4)

#define CMD_EDCA_AIFS_BIT	(1 << 0)
#define CMD_EDCA_WIN_MIN_BIT	(1 << 1)
#define CMD_EDCA_WIN_MAX_BIT	(1 << 2)
#define CMD_EDCA_TXOP_BIT	(1 << 3)
#define CMD_EDCA_ALL_BITS	(CMD_EDCA_AIFS_BIT | CMD_EDCA_WIN_MIN_BIT | CMD_EDCA_WIN_MAX_BIT | CMD_EDCA_TXOP_BIT)

#define CMD_EDCA_AC_MAX 4

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)
typedef enum _ENUM_HW_TX_QUE_T {
	TX_Q_AC0 = 0,
	TX_Q_AC1,
	TX_Q_AC2,
	TX_Q_AC3,
	TX_Q_AC10,
	TX_Q_AC11,
	TX_Q_AC12,
	TX_Q_AC13,
	TX_Q_AC20,
	TX_Q_AC21,
	TX_Q_AC22,
	TX_Q_AC23,
	TX_Q_AC30,
	TX_Q_AC31,
	TX_Q_AC32,
	TX_Q_AC33,
	TX_Q_ALTX0,
	TX_Q_BMC,
	TX_Q_BCN,
	TX_Q_PSMP,
	TX_Q_ALTX1,
	TX_Q_BMC1,
	TX_Q_BCN1,
	TX_Q_PSMP1,
	HW_TX_QUE_NUM
} ENUM_HW_TX_QUE_T;
#else
typedef enum _ENUM_HW_TX_QUE_T {
	TX_Q_AC0 = 0,
	TX_Q_AC1,
	TX_Q_AC2,
	TX_Q_AC3,
	TX_Q_AC4,
	TX_Q_AC5,
	TX_Q_AC6,
	TX_Q_BCN,
	TX_Q_BMC,
	TX_Q_AC10,
	TX_Q_AC11,
	TX_Q_AC12,
	TX_Q_AC13,
	TX_Q_AC14,
	HW_TX_QUE_NUM
} ENUM_HW_TX_QUE_T;
#endif /* defined(MT7615) || defined(MT7622) */

typedef struct GNU_PACKED _WIFI_EVENT_MUAR_T {
	UINT8      ucEntryCnt;
	UINT8      ucAccessMode;
	UINT8      aucReserved[2];
	UINT32     u4Status;
} WIFI_EVENT_MUAR_T, *P_WIFI_EVENT_MUAR_T;

typedef struct GNU_PACKED _WIFI_EVENT_MUAR_MULTI_RW_T {
	UINT8      ucEntryIdx;
	UINT8      ucReserved;
	UINT8      aucMacAddr[6];

} WIFI_EVENT_MUAR_MULTI_RW_T, *P_WIFI_EVENT_MUAR_MULTI_RW_T;

typedef struct GNU_PACKED _EXT_CMD_MUAR_MULTI_ENTRY_T {
	UINT8 ucMuarIdx;
	UINT8 ucBssid;
	UINT8 aucMacAddr[6];
} EXT_CMD_MUAR_MULTI_ENTRY_T, *P_EXT_CMD_MUAR_MULTI_ENTRY_T;

typedef enum _MUAR_MODE {
	MUAR_NORMAL = 0,
	MUAR_REPEATER,
	MUAR_HASH,
	MUAR_MAX
} MUAR_MODE;

typedef enum _MUAR_ACCESS_MODE {
	MUAR_READ = 0,
	MUAR_WRITE,
} _MUAR_ACCESS_MODE;

typedef struct GNU_PACKED _EXT_CMD_MUAR_T {
	UINT8 ucMuarModeSel;
	UINT8 ucForceClear;
	UINT8 ausClearBitmap[8];
	UINT8 ucEntryCnt;
	UINT8 ucAccessMode; /* 0:read, 1:write */
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	UINT8 ucBand;
#endif
} EXT_CMD_MUAR_T, *P_EXT_CMD_MUAR_T;

typedef struct GNU_PACKED _EXT_CMD_TRGR_PRETBTT_INT_EVENT_T {
	UINT8 ucHwBssidIdx;
	UINT8 ucExtBssidIdx;
	UINT8 ucEnable;
	UINT8 aucReserved1[1];

	UINT16 u2BcnPeriod;
	UINT8 aucReserved2[2];

} CMD_TRGR_PRETBTT_INT_EVENT_T, *P_CMD_TRGR_PRETBTT_INT_EVENT_T;

typedef struct GNU_PACKED _EXT_CMD_FD_FRAME_OFFLOAD_T {
    UINT8 ucOwnMacIdx;
    UINT8 ucEnable;
    UINT8 ucWlanIdx;	/* #256STA */
    UINT8 ucBandIdx;

    UINT16 u2PktLength;
    UINT16 u2TimestampFieldPos;

	UINT8 acPktContent[1520];
} CMD_FD_FRAME_OFFLOAD_T, *P_CMD_FD_FRAME_OFFLOAD_T;

typedef enum _OFFLOAD_PKT_TYPE_T {
	PKT_V1_BCN		= 0,	/* buf size = 512 */
	PKT_V1_BTIM		= 1,	/* buf size = 512 */
	PKT_V2_BCN		= 2,	/* buf size = 1520 */
	PKT_V2_BTIM		= 3,	/* buf size = 1520 */
	PKT_V3_TLV		= 4		/* TLV base cmd format, variable length */
} OFFLOAD_PKT_TYPE_T;

typedef struct GNU_PACKED _EXT_CMD_BCN_OFFLOAD_T {
	UINT8 ucOwnMacIdx;
	UINT8 ucEnable;
	UINT8 ucWlanIdx;
	UINT8 ucBandIdx;/* 0: band 0, 1: band 1 */

	UINT8 ucPktType;/* 0: Bcn, 1: Tim Frame. */
	BOOLEAN fgNeedPretbttIntEvent;
	UINT16  u2CsaIePos; /* CSA IE position */

	UINT16 u2PktLength;
	UINT16 u2TimIePos;/* Tim IE position in pkt. */

	UINT8 acPktContent[512];/* whole pkt template length which include TXD, max shall not exceed 512 bytes. */
	UINT8 ucCsaCount; /* count down value of CSA IE */
	UINT8 ucBccCount; /* count down value of BCCA IE */
	UINT16 u2BccIePos; /* BSS Color Change Announcement IE postion */
} CMD_BCN_OFFLOAD_T, *P_CMD_BCN_OFFLOAD_T;

typedef struct GNU_PACKED _EXT_CMD_BCN_OFFLOAD_T_V2 {
	UINT8 ucOwnMacIdx;
	UINT8 ucEnable;
	UINT8 ucWlanIdx;
	UINT8 ucBandIdx;/* 0: band 0, 1: band 1 */

	UINT8 ucPktType;/* 0: Bcn, 1: Tim Frame. */
	BOOLEAN fgNeedPretbttIntEvent;
	UINT16  u2CsaIePos; /* CSA IE position */

	UINT16 u2PktLength;
	UINT16 u2TimIePos;/* Tim IE position in pkt. */
	UINT8 acPktContent[1520];/* whole pkt template length which include TXD, max shall not exceed 1520 bytes. */
	UINT8 ucCsaCount; /* count down value of CSA IE */
	UINT8 aucReserved[3];
} CMD_BCN_OFFLOAD_T_V2, *P_CMD_BCN_OFFLOAD_T_V2;

typedef struct _EXT_EVENT_PRETBTT_INT_T {
	UINT8 ucHwBssidIdx;
	UINT8 aucReserved[3];
} EXT_EVENT_PRETBTT_INT_T, *P_EXT_EVENT_PRETBTT_INT_T;

enum _TMR_CTRL_TYPE_T {
	SET_TMR_ENABLE  = 0x00,
	TMR_CALIBRATION = 0x01
};

typedef struct GNU_PACKED _CMD_TMR_CTRL_T {
	UINT8 ucTmrCtrlType;
	UINT8 ucTmrVer;
	UINT8 ucTmrThroughold;
	UINT8 ucTmrIter;
	UINT8 pTmrCtrlPayload[];
} CMD_TMR_CTRL_T, *P_CMD_TMR_CTRL_T;

typedef struct GNU_PACKED _TMR_CTRL_SET_TMR_EN_T {
	UINT8 ucEnable;
	UINT8 ucRole;
	UINT8 ucCatEnable; /* TMR2.0 use 4 bits to indicate 4 types of category [27:24] */
	UINT8 ucDbdcIdx;
	UINT8 aucType_Subtype[4];
} TMR_CTRL_SET_TMR_EN_T, *P_TMR_CTRL_SET_TMR_EN_T;

typedef struct GNU_PACKED  _CMD_SET_THERMO_CAL_T {
	UINT8	ucEnable;
	UINT8	ucSourceMode;
	UINT8     ucRFDiffTemp;
	UINT8     ucHiBBPHT;
	UINT8     ucHiBBPNT;
	INT8       cLoBBPLT;
	INT8       cLoBBPNT;
	UINT8     ucReserve;
	BIN_CONTENT_T ucThermoSetting[3];
} CMD_SET_THERMO_CAL_T, *P_CMD_SET_THERMO_CAL_T;

#ifdef MT_WOW_SUPPORT
enum ENUM_PACKETFILTER_TYPE {
	_ENUM_TYPE_MAGIC			= 0,
	_ENUM_TYPE_BITMAP			= 1,
	_ENUM_TYPE_ARPNS			= 2,
	_ENUM_TYPE_GTK_REKEY		= 3,
	_ENUM_TYPE_CF				= 4,
	_ENUM_TYPE_GLOBAL_EN		= 5,
	_ENUM_TYPE_TCP_SYN			= 6,
	_ENUM_TYPE_DETECTION_MASK	= 7,
};

enum ENUM_FUNCTION_SELECT {
	_ENUM_PF					= 0,
	_ENUM_GLOBAL_MAGIC			= 1,
	_ENUM_GLOBAL_BITMAP			= 2,
	_ENUM_GLOBAL_EAPOL			= 3,
	_ENUM_GLOBAL_TDLS			= 4,
	_ENUM_GLOBAL_ARPNS			= 5,
	_ENUM_GLOBAL_CF				= 6,
	_ENUM_GLOBAL_MODE			= 7,
	_ENUM_GLOBAL_BSSID			= 8,
	_ENUM_GLOBAL_MGMT			= 9,
	_ENUM_GLOBAL_BMC_DROP		= 10,
	_ENUM_GLOBAL_UC_DROP		= 11,
	_ENUM_GLOBAL_ALL_TOMCU		= 12,
	_ENUM_GLOBAL_WOW_EN         = 16,
};

enum ENUM_PF_MODE_T {
	PF_MODE_WHITE_LIST		= 0,
	PF_MODE_BLACK_LIST		= 1,
	PF_MODE_NUM
};

enum ENUM_PF_BSSID_IDX_T {
	PF_BSSID_DISABLE	= 0,
	PF_BSSID_0	= (1 << 0),
	PF_BSSID_1	= (1 << 1),
	PF_BSSID_2	= (1 << 2),
	PF_BSSID_3	= (1 << 3),
};

enum ENUM_PF_BAND_IDX_T {
	PF_BAND_0					= 0,
	PF_BAND_1					= 1,
};

enum ENUM_PF_WAP_VER_T {
	PF_WPA						= 0,
	PF_WPA2					= 1,
};

enum ENUM_PF_WMM_IDX_T {
	PF_WMM_0				= 0,
	PF_WMM_1				= 1,
	PF_WMM_2				= 2,
	PF_WMM_3				= 3,
};

enum ENUM_PF_ARPNS_SET_T {
	PF_ARP_NS_SET_0			= 0,
	PF_ARP_NS_SET_1			= 1,
};

enum ENUM_PF_ARPNS_ENSABLE_T {
	PF_ARP_NS_DISABLE			= 0,
	PF_ARP_NS_ENABLE			= 1,
};

enum ENUM_PF_ARPNS_OFFLOAD_TYPE_T {
	PF_ARP_OFFLOAD			= 0,
	PF_NS_OFFLOAD				= 1,
};

enum ENUM_PF_ARPNS_PKT_TYPE_T {
	PF_ARP_NS_UC_PKT			= (1 << 0),
	PF_ARP_NS_BC_PKT			= (1 << 1),
	PF_ARP_NS_MC_PKT			= (1 << 2),
	PF_ARP_NS_ALL_PKT			= ((1 << 0) | (1 << 1) | (1 << 2)),
};

typedef struct GNU_PACKED _CMD_PACKET_FILTER_WAKEUP_OPTION_T {
	UINT32	WakeupInterface;
	UINT32	GPIONumber;
	UINT32	GPIOTimer;
	UINT32	GpioParameter;
} CMD_PACKET_FILTER_WAKEUP_OPTION_T, *P_CMD_PACKET_FILTER_WAKEUP_OPTION_T;

typedef struct GNU_PACKED _CMD_PACKET_FILTER_GLOBAL_T {
	UINT32	PFType;
	UINT32	FunctionSelect;
	UINT32	Enable;
	UINT32	Band;
} CMD_PACKET_FILTER_GLOBAL_T, *P_CMD_PACKET_FILTER_GLOBAL_T;

typedef struct GNU_PACKED _CMD_PACKET_FILTER_MAGIC_PACKET_T {
	UINT32	PFType;
	UINT32	BssidEnable;
} CMD_PACKET_FILTER_MAGIC_PACKET_T, *P_CMD_PACKET_FILTER_MAGIC_PACKET_T;

typedef struct GNU_PACKED _CMD_PACKET_FILTER_BITMAP_PATTERN_T {
	UINT32	PFType;
	UINT32	Index;
	UINT32	Enable;
	UINT32	BssidEnable;
	UINT32	Offset;
	UINT32	FeatureBits;
	UINT32	Resv;
	UINT32	PatternLength;
	UINT32	Mask[4];
	UINT32	Pattern[32];
} CMD_PACKET_FILTER_BITMAP_PATTERN_T, *P_CMD_PACKET_FILTER_BITMAP_PATTERN_T;

typedef struct GNU_PACKED _CMD_PACKET_FILTER_ARPNS_T {
	UINT32	PFType;
	UINT32	IPIndex;
	UINT32	Enable;
	UINT32	BssidEnable;
	UINT32	Offload;
	UINT32	Type;
	UINT32	FeatureBits;
	UINT32	Resv;
	UINT8	IPAddress[16];
} CMD_PACKET_FILTER_ARPNS_T, *P_CMD_PACKET_FILTER_ARPNS_T;

typedef struct GNU_PACKED _CMD_PACKET_FILTER_GTK_T {
	UINT32	PFType;
	UINT32	WPAVersion;
	UINT32	PTK[16];
	UINT32	ReplayCounter[2];
	UINT32	PairKeyIndex;
	UINT32	GroupKeyIndex;
	UINT32	BssidIndex;
	UINT32	OwnMacIndex;
	UINT32	WmmIndex;
	UINT32	Resv1;
} CMD_PACKET_FILTER_GTK_T, *P_CMD_PACKET_FILTER_GTK_T;

typedef struct GNU_PACKED _CMD_PACKET_FILTER_COALESCE_T {
	UINT32	PFType;
	UINT32	FilterID;
	UINT32	Enable;
	UINT32	BssidEnable;
	UINT32	PacketType;
	UINT32	CoalesceOP;
	UINT32	FeatureBits;
	UINT8	Resv;
	UINT8	FieldLength;
	UINT8	CompareOP;
	UINT8	FieldID;
	UINT32	Mask[2];
	UINT32	Pattern[4];
} CMD_PACKET_FILTER_COALESCE_T, *P_CMD_PACKET_FILTER_COALESCE_T;

typedef struct GNU_PACKED _CMD_PACKET_TCPSYN_T {
	UINT32	PFType;
	UINT32	AddressType;
	UINT32	Enable;
	UINT32	BssidEnable;
	UINT32	PacketType;
	UINT32	FeatureBits;
	UINT32	TCPSrcPort;
	UINT32	TCPDstPort;
	UINT32	SourceIP[4];
	UINT32	DstIP[4];
} CMD_PACKET_FILTER_TCPSYN_T, *P_CMD_PACKET_FILTER_TCPSYN_T;

typedef struct GNU_PACKED _EXT_EVENT_PF_GENERAL_T {
	UINT32   u4PfCmdType;
	UINT32   u4Status;
	UINT32   u4Resv;
} EXT_EVENT_PF_GENERAL_T, *P_EXT_EVENT_PF_GENERAL_T;

typedef struct GNU_PACKED _EXT_EVENT_WAKEUP_OPTION_T {
	UINT32   u4PfCmdType;
	UINT32   u4Status;
} EXT_EVENT_WAKEUP_OPTION_T, *P_EXT_EVENT_WAKEUP_OPTION_T;
#endif

#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
typedef struct GNU_PACKED _EXT_CMD_NOA_CTRL_T {
	UINT8 ucMode0;
	UINT8 acuReserved0[3];

	UINT8 ucMode1;
	UINT8 acuReserved1[3];

	UINT8 ucMode2;
	UINT8 acuReserved2[3];

	UINT8 ucMode3;
	UINT8 acuReserved3[3];
} EXT_CMD_NOA_CTRL_T, *P_EXT_CMD_NOA_CTRL_T;

#endif /* defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA) */
#ifdef CFG_TDLS_SUPPORT
typedef struct GNU_PACKED _EXT_CMD_CFG_TDLS_CHSW_T {
	/* UINT8 ucCmd; */
	UINT8	ucOffPrimaryChannel;
	UINT8	ucOffCenterChannelSeg0;
	UINT8	ucOffCenterChannelSeg1;
	UINT8	ucOffBandwidth;
	UINT32	u4StartTimeTsf;
	UINT32	u4SwitchTime;	 /* us */
	UINT32	u4SwitchTimeout;	/* us */
	UINT8	ucRole;
	UINT8	ucBssIndex;
	UINT8	Reserved[4];

} EXT_CMD_CFG_TDLS_CHSW_T, *P_EXT_CMD_CFG_TDLS_CHSW_T;
#endif /*CFG_TDLS_SUPPORT*/

/*
CMD prototype structure
*/
typedef struct _MT_RF_REG_PAIR {
	UINT8 WiFiStream;
	UINT32 Register;
	UINT32 Value;
} MT_RF_REG_PAIR;

typedef struct _MT_PWR_MGT_BIT_WIFI_T {
	UINT16 u2WlanIdx;
	UINT8  ucPwrMgtBit;
} MT_PWR_MGT_BIT_WIFI_T, *PMT_PWR_MGT_BIT_WIFI_T;

typedef struct _MT_STA_CFG_PTR_T {
	struct _STA_ADMIN_CONFIG *pStaCfg;
} MT_STA_CFG_PTR_T, *PMT_STA_CFG_PTR_T;

typedef struct _PSM_BIT_CTRL_T {
	struct _STA_ADMIN_CONFIG *pStaCfg;
	USHORT psm_val;
} PSM_BIT_CTRL_T, *PPSM_BIT_CTRL_T;

typedef struct _RADIO_ON_OFF_T {
	UINT8 ucDbdcIdx;
	UINT8 ucRadio;
} RADIO_ON_OFF_T, *PRADIO_ON_OFF_T;

typedef struct _GREENAP_ON_OFF_T {
	UINT8 ucDbdcIdx;
	BOOLEAN ucGreenAPOn;
} GREENAP_ON_OFF_T, *PGREENAP_ON_OFF_T;

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
typedef struct _PCIE_ASPM_DYM_CTRL_T {
	UINT8       ucDbdcIdx;
	BOOLEAN     fgL1Enable;
	BOOLEAN     fgL0sEnable;
} PCIE_ASPM_DYM_CTRL_T, *P_PCIE_ASPM_DYM_CTRL_T;
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
/* hardware control interface */
struct twt_agrt_para {
	/* DW0 */
	UINT8		agrt_tbl_idx;
	UINT8		agrt_ctrl_flag;
	UINT8		own_mac_idx;
	/* It is set to 0xff when peerGrpId is a group ID */
	UINT8		flow_id;
	/* DW1 */
	/* Specify the peer ID (MSB=0) or group ID (MSB=1)  (10 bits for StaIdx, MSB to identify if it is for groupId) */
	UINT16		peer_id_grp_id;
	/* Same as SPEC definition. 8 bits, in unit of 256 us */
	UINT8		agrt_sp_duration;
	/* So that we know which BSS TSF should be used for this AGRT */
	UINT8		bss_idx;
	/* DW2, DW3, DW4 */
	UINT32		agrt_sp_start_tsf_low;
	UINT32		agrt_sp_start_tsf_high;
	UINT16		agrt_sp_wake_intvl_mantissa;
	UINT8		agrt_sp_wake_intvl_exponent;
	UINT8		is_role_ap;
	/* DW5 */
	/* For Bitmap definition,please refer to TWT_AGRT_PARA_BITMAP_IS_TRIGGER and etc */
	UINT8		agrt_para_bitmap;
	UINT8		persistence;
	UINT16		ntbtt_before_reject;

	/* Following field is valid ONLY when peer_id_grp_id is a group ID */
	/* DW6 */
	UINT8		grp_member_cnt;
	UINT8		reserved_c;
	UINT16		reserved_d;
	/* DW7 ... */
	UINT16		sta_list[TWT_HW_GRP_MAX_MEMBER_CNT];
};
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

typedef struct  {
	UINT8 PmNumber;
	UINT8 PmState;
	UINT8 Bssid[6];
	UINT8 DtimPeriod;
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	UINT16 WlanIdx;
#else
	UINT8 WlanIdx;
#endif
	UINT16 BcnInterval;
	UINT32 Aid;
	UINT8 OwnMacIdx;
	UINT8 BcnLossCount;
	UINT8 DbdcIdx;
	UINT8 WmmIdx;
} MT_PMSTAT_CTRL_T;

typedef struct  {
	UINT8 ucDbdcIdx;
	BOOLEAN ucGreenAPOn;
} MT_GREENAP_CTRL_T;

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
typedef struct  {
	UINT8       ucDbdcIdx;
	BOOLEAN     fgL1Enable;
	BOOLEAN     fgL0sEnable;
} MT_PCIE_ASPM_DYM_CTRL_T;
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
/* middle between hw ctrl and mt cmd */
struct mt_twt_agrt_para {
	/* DW0 */
	UINT8		agrt_tbl_idx;
	UINT8		agrt_ctrl_flag;
	UINT8		own_mac_idx;
	/* It is set to 0xff when peerGrpId is a group ID */
	UINT8		flow_id;
	/* DW1 */
	/* Specify the peer ID (MSB=0) or group ID (MSB=1)  (10 bits for StaIdx, MSB to identify if it is for groupId) */
	UINT16		peer_id_grp_id;
	/* Same as SPEC definition. 8 bits, in unit of 256 us */
	UINT8		agrt_sp_duration;
	/* So that we know which BSS TSF should be used for this AGRT */
	UINT8		bss_idx;
	/* DW2, DW3, DW4 */
	UINT32		agrt_sp_start_tsf_low;
	UINT32		agrt_sp_start_tsf_high;
	UINT16		agrt_sp_wake_intvl_mantissa;
	UINT8		agrt_sp_wake_intvl_exponent;
	UINT8		is_role_ap;
	/* DW5 */
	/* For Bitmap definition,please refer to TWT_AGRT_PARA_BITMAP_IS_TRIGGER and etc */
	UINT8		agrt_para_bitmap;
	UINT8		persistence;
	UINT16		reserved_b;

	/* Following field is valid ONLY when peer_id_grp_id is a group ID */
	/* DW6 */
	UINT8		grp_member_cnt;
	UINT8		reserved_c;
	UINT16		reserved_d;
	/* DW7 ... */
	UINT16		sta_list[TWT_HW_GRP_MAX_MEMBER_CNT];
};
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

typedef struct {
	UCHAR Channel;
	UCHAR Bw;
	UCHAR CentralSeg0;
	UCHAR CentralSeg1;
	UCHAR Role;
	USHORT StayTime;
	UCHAR OwnMACAddressIdx;
	UCHAR WlanIdx;
	UCHAR BssIdx;
	UCHAR WmmIdx;
} MT_MCC_ENTRT_T;

/* #ifdef ERR_RECOVERY */
typedef struct _GENERAL_TEST_SIM_ERR_SWITCH_ON_OFF_T {
	BOOLEAN ucSwitchMode;
	UINT8 ucReserved[3];
} GENERAL_TEST_SIM_ERR_SWITCH_ON_OFF_T, *P_GENERAL_TEST_SIM_ERR_SWITCH_ON_OFF_T;

typedef struct _GENERAL_TEST_SIM_ERR_DET_RECOVERY_T {
	UINT8 ucModule;
	UINT8 ucSubModule;
	UINT8 ucReserved[2];
} GENERAL_TEST_SIM_ERR_DET_RECOVERY_T, *P_GENERAL_TEST_SIM_ERR_DET_RECOVERY_T;

typedef struct GNU_PACKED _EXT_CMD_GENERAL_TEST_T {
	UINT8 ucCategory;
	UINT8 ucAction;
	UINT8 ucDiaglogToken;
	UINT8 ucReserved;

	union {
		/* for category = GENERAL_TEST_CATEGORY_SIM_ERROR_DETECTION
		 * and ucAction = GENERAL_TEST_ACTION_SWITCH_ON_OFF
		 */
		GENERAL_TEST_SIM_ERR_SWITCH_ON_OFF_T rGeneralTestSimErrorSwitchOnOff;

		/* for category = GENERAL_TEST_CATEGORY_SIM_ERROR_DETECTION
		 * and ucAction = GENERAL_TEST_ACTION_RECOVERY
		 */
		GENERAL_TEST_SIM_ERR_DET_RECOVERY_T rGeneralTestSimErrDetRecovery;
	} Data;
} EXT_CMD_GENERAL_TEST_T, *P_EXT_CMD_GENERAL_TEST_T;

#define GENERAL_TEST_CATEGORY_NON (0x0)
#define GENERAL_TEST_CATEGORY_SIM_ERROR_DETECTION (0x1)
#define GENERAL_TEST_CATEGORY_APPWS               (0x2)

#define GENERAL_TEST_ACTION_NON (0x0)
#define GENERAL_TEST_ACTION_SWITCH_ON_OFF (0x1)
#define GENERAL_TEST_ACTION_RECOVERY (0x2)

#define GENERAL_TEST_MODULE_NON (0x0)
#define GENERAL_TEST_MODULE_LMAC (0x1)
#define GENERAL_TEST_MODULE_UMAC (0x2)
#define GENERAL_TEST_MODULE_HIF (0x3)
#define GENERAL_TEST_MODULE_MCU (0x4)

#define GENERAL_TEST_SUBMOD_LMAC_NON (0x0)
#define GENERAL_TEST_SUBMOD_LMAC_TXRXR (0x1)
#define GENERAL_TEST_SUBMOD_LMAC_TX (0x2)
#define GENERAL_TEST_SUBMOD_LMAC_RX (0x3)
#define GENERAL_TEST_SUBMOD_UMAC_NON (0x0)
#define GENERAL_TEST_SUBMOD_UMAC_RESET (0x1)
#define GENERAL_TEST_SUBMOD_HIF_NON (0x0)
#define GENERAL_TEST_SUBMOD_HIF_PDMA0 (0x1)
#define GENERAL_TEST_SUBMOD_HIF_PDMA1 (0x2)
#define GENERAL_TEST_SUBMOD_HIF_PDMA2 (0x3)
/*#endif */	/* ERR_RECOVERY */

/* Action ID of Category GENERAL_TEST_CATEGORY_APPWS */
#define APPWS_ACTION_DUMP_INFO                     0

#define SER_ACTION_QUERY                    0
#define SER_ACTION_SET                      1
#define SER_ACTION_SET_ENABLE_MASK          2
#define SER_ACTION_RECOVER                  3

#ifdef WIFI_UNIFIED_COMMAND
#define UNI_SER_ACTION_SET_QUERY            BIT(0)
#define UNI_SER_ACTION_SET_ENABLE           BIT(1)
#define UNI_SER_ACTION_SET_ENABLE_MASK      BIT(2)
#define UNI_SER_ACTION_SET_TRIGGER          BIT(3)
#endif /* WIFI_UNIFIED_COMMAND */

/* SER_ACTION_SET sub action */
#define SER_SET_DISABLE         0
#define SER_SET_ENABLE          1

/* SER_ACTION_SET_ENABLE_MASK mask define */
#define SER_ENABLE_TRACKING         BIT(0)
#define SER_ENABLE_L1_RECOVER       BIT(1)
#define SER_ENABLE_L2_RECOVER       BIT(2)
#define SER_ENABLE_L3_RX_ABORT      BIT(3)
#define SER_ENABLE_L3_TX_ABORT      BIT(4)
#define SER_ENABLE_L3_TX_DISABLE    BIT(5)
#define SER_ENABLE_L3_BF_RECOVER    BIT(6)

/* SER_ACTION_RECOVER recover method */
#define SER_SET_L0_RECOVER         0
#define SER_SET_L1_RECOVER         1
#define SER_SET_L2_RECOVER         2
#define SER_SET_L3_RX_ABORT        3
#define SER_SET_L3_TX_ABORT        4
#define SER_SET_L3_TX_DISABLE      5
#define SER_SET_L3_BF_RECOVER      6


typedef struct GNU_PACKED _EXT_CMD_SER_T {
	UINT8	action;
	UINT8	ser_set;
	UINT8	ucDbdcIdx;
	UINT8	ucReserve[1];
} EXT_CMD_SER_T, *P_EXT_CMD_SER_T;

#ifdef DBDC_MODE

typedef enum {
	DBDC_TYPE_WMM = 0,
	DBDC_TYPE_MGMT,
	DBDC_TYPE_BSS,
	DBDC_TYPE_MBSS,
	DBDC_TYPE_REPEATER,
	DBDC_TYPE_MU,
	DBDC_TYPE_BF,
	DBDC_TYPE_PTA,
} DBDC_TYPE;

typedef struct _BAND_CTRL_ENTRY_T {
	UINT8  ucType;
	UINT8  ucIndex;
	UINT8  ucBandIdx;
	UINT8  ucReserve;
} BAND_CTRL_ENTRY_T, *P_BAND_CTRL_ENTRY_T;

typedef struct GNU_PACKED _EXT_CMD_DBDC_CTRL_T {
	UINT8  ucDbdcEnable;
	UINT8  ucTotalNum;
	UINT8  aucReserved[2];
	BAND_CTRL_ENTRY_T  aBCtrlEntry[64];
} EXT_CMD_DBDC_CTRL_T, *P_EXT_CMD_DBDC_CTRL_T, EXT_EVENT_DBDC_CTRL_T, *P_EXT_EVENT_DBDC_CTRL_T;

#endif /*DBDC_MODE*/

enum {
	MAC_INFO_TYPE_RESERVE = 0,
	MAC_INFO_TYPE_CHANNEL_BUSY_CNT = 0x1,
	MAC_INFO_TYPE_TSF = 0x2,
	MAC_INFO_TYPE_MIB = 0x3,
	MAC_INFO_TYPE_EDCA = 0x4,
	MAC_INFO_TYPE_WIFI_INT_CNT = 0x5,
};

/*MAC INFO ID:  Get Channel Busy Cnt (0x01)*/
typedef struct _EXTRA_ARG_CH_BUSY_CNT_T {
	UINT8  ucBand;
	UINT8  aucReserved[3];
} EXTRA_ARG_CH_BUSY_CNT_T, *P_EXTRA_ARG_CH_BUSY_CNT_T;

/*MAC INFO ID:  Get TSF (0x02)*/
typedef struct _EXTRA_ARG_TSF_T {
	UINT8  ucHwBssidIndex;
	UINT8  aucReserved[3];
} EXTRA_ARG_TSF_T, *P_EXTRA_ARG_TSF_T;

/*MAC INFO ID:  Partial MIB info count (0x03)*/
typedef struct _EXTRA_ARG_PARTIAL_MIB_INFO_CNT_T {
	UINT8  ucBand;
	UINT8  aucReserved[3];
	UINT32 au4MibInfo;
} EXTRA_ARG_PARTIAL_MIB_INFO_CNT_T, *P_EXTRA_ARG_PARTIAL_MIB_INFO_CNT_T;

/*MAC INFO ID:  EDCA (0x04)*/
typedef struct _EXTRA_ARG_EDCA_T {
	UINT8  ucTotalAcNum;
	UINT8  aucReserved[3];
	UINT32 au4AcIndex[HW_TX_QUE_NUM];
} EXTRA_ARG_EDCA_T, *P_EXTRA_ARG_EDCA_T;

/* MAC INFO ID: Get wifi interrupt counter (0x05)*/
typedef struct _EXTRA_ARG_WF_INTERRUPT_CNT_T {
	UINT8  ucBand;
	UINT8  ucWifiInterruptNum;
	UINT8  aucReserved[2];
	UINT32 u4WifiInterruptMask;
} EXTRA_ARG_WF_INTERRUPT_CNT_T, *P_EXTRA_ARG_WF_INTERRUPT_CNT_T;

typedef union {
	EXTRA_ARG_CH_BUSY_CNT_T  ChBusyCntArg;
	EXTRA_ARG_TSF_T  TsfArg;
	EXTRA_ARG_PARTIAL_MIB_INFO_CNT_T  PartialMibInfoCntArg;
	EXTRA_ARG_EDCA_T  EdcaArg;
	EXTRA_ARG_WF_INTERRUPT_CNT_T  WifiInterruptCntArg;
} EXTRA_ARG_MAC_INFO_T;

#define IPI_HIST_TYPE_NUM 12

#ifdef IPI_SCAN_SUPPORT
#define PWR_INDICATE_HIST_MAX 11
#endif

typedef struct _EXT_CMD_RDD_IPI_HIST_T {
	UINT8 ipi_hist_idx;
	UINT8 band_idx;
	UINT8 set_val;
	UINT8 reserved;
	INT32 idle_pwr_thres;
	UINT32 idle_pwr_max_cnt;
	UINT32 idle_pwr_duration;
	UINT32 idle_pwr_cmd_type;
} EXT_CMD_RDD_IPI_HIST_T, *P_EXT_CMD_RDD_IPI_HIST_T;

typedef struct _EXT_EVENT_RDD_IPI_HIST {
	UINT8 ipi_hist_idx;
	UINT8 band_idx;
	UINT8 reserved[2];
	UINT32 ipi_hist_val[IPI_HIST_TYPE_NUM];
	UINT32 tx_assert_time /* unit: us */;
} EXT_EVENT_RDD_IPI_HIST, *P_EXT_EVENT_RDD_IPI_HIST;

#ifdef IPI_SCAN_SUPPORT

#define RDM_NF_MAX_WF_IDX 8

typedef struct _EXT_CMD_RDD_IPI_SCAN_T {
    UINT_8 u1mode;
    UINT_8 aucReserve[2];
} EXT_CMD_RDD_IPI_SCAN_T, *P_EXT_CMD_RDD_IPI_SCAN_T;

typedef struct _EXT_EVENT_RDD_IPI_SCAN {
    UINT_32 au4IpiHistVal[RDM_NF_MAX_WF_IDX][PWR_INDICATE_HIST_MAX] /* ant * ipi */;
    UINT_8 u1BandIdx;
    UINT_8 aucReserve[2];
    UINT_32 u4TxAssertTime;
} EXT_EVENT_RDD_IPI_SCAN, *P_EXT_EVENT_RDD_IPI_SCAN;
#endif

typedef enum _ENUM_RDD_SET_IPI_HIST_TYPE {
	RDD_SET_IPI_CR_INIT = 0, /* CR initialization */
	RDD_SET_IPI_HIST_RESET,  /* Reset IPI histogram counter */
	RDD_SET_IDLE_PWR,        /* Idle Power Info */
	RDD_SET_IPI_HIST_NUM
} ENUM_RDD_SET_IPI_HIST_TYPE, *P_ENUM_RDD_SET_IPI_HIST_TYPE;

typedef enum _ENUM_RDD_GET_IPI_HIST_TYPE {
	RDD_IPI_HIST_0 = 0,       /* Range: Power <= -92 (dBm) */
	RDD_IPI_HIST_1,           /* Range: -92 < Power <= -89 (dBm) */
	RDD_IPI_HIST_2,           /* Range: -89 < Power <= -86 (dBm) */
	RDD_IPI_HIST_3,           /* Range: -86 < Power <= -83 (dBm) */
	RDD_IPI_HIST_4,           /* Range: -83 < Power <= -80 (dBm) */
	RDD_IPI_HIST_5,           /* Range: -80 < Power <= -75 (dBm) */
	RDD_IPI_HIST_6,           /* Range: -75 < Power <= -70 (dBm) */
	RDD_IPI_HIST_7,           /* Range: -70 < Power <= -65 (dBm) */
	RDD_IPI_HIST_8,           /* Range: -65 < Power <= -60 (dBm) */
	RDD_IPI_HIST_9,           /* Range: -60 < Power <= -55 (dBm) */
	RDD_IPI_HIST_10,          /* Range: -55 < Power        (dBm) */
	RDD_IPI_FREE_RUN_CNT = 11,/* counter++ per 8us */
	RDD_IPI_HIST_ALL_CNT,     /* Get all IPI */
	RDD_IPI_HIST_0_TO_10_CNT, /* Get IPI histogram 0 to 10 */
	RDD_IPI_HIST_2_TO_10_CNT, /* Get IPI histogram 2 to 10 */
	RDD_TX_ASSERT_TIME,       /* Get band 1 TX assert time */
	RDD_IPI_HIST_NUM
} ENUM_RDD_GET_IPI_HIST_TYPE, *P_ENUM_RDD_GET_IPI_HIST_TYPE;

typedef struct _EXT_CMD_EDCCA_CMD_T {
	UINT8 u1BandIdx;
	UINT8 u1CmdIdx;
	UINT8 u1Val[3];
	BOOLEAN fginit;
	UINT8 u1EDCCAStd;
	INT8 icompensation;
} EXT_CMD_EDCCA_CMD_T, *P_EXT_CMD_EDCCA_CMD_T;

typedef enum _ENUM_EDCCA_CTRL_T {
	SET_EDCCA_CTRL_EN = 0,
	SET_EDCCA_CTRL_THRES,
	GET_EDCCA_CTRL_EN,
	GET_EDCCA_CTRL_THRES,
	EDCCA_CTRL_NUM
} ENUM_EDCCA_CTRL_T, *P_ENUM_EDCCA_CTRL_T;

typedef struct GNU_PACKED _EXT_CMD_GET_MAC_INFO_T {
	UINT16 u2MacInfoId;
	UINT8  aucReserved[2];
	EXTRA_ARG_MAC_INFO_T aucExtraArgument;
} EXT_CMD_GET_MAC_INFO_T, *P_EXT_CMD_GET_MAC_INFO_T;

/* RXV cmd enum */
typedef enum _TESTMODE_RXV_CMD_CATEGORY {
	TESTMODE_RXV_CMD_GET_RX_STAT = 0x0,
	TESTMODE_RXV_CMD_SET_RXV_CTRL = 0x1,
	TESTMODE_RXV_CMD_SET_RXV_RU_CTRL = 0x2,
	TESTMODE_RXV_CMD_GET_RX_STAT_BAND = 0x3,
	TESTMODE_RXV_CMD_GET_RX_STAT_PATH = 0x4,
	TESTMODE_RXV_CMD_GET_RX_STAT_USER = 0x5,
	TESTMODE_RXV_CMD_GET_RX_STAT_COMM = 0x6,
	TESTMODE_RXV_CMD_NUM
} TESTMODE_RXV_CMD_CATEGORY, *P_TESTMODE_RXV_CMD_CATEGORY;

/* RXV control cmd struct */
typedef struct _EXT_CMD_TESTMODE_RXV_CTRL {
	UINT8 u1RxvCtrlFormatId;
	BOOLEAN fgRxvEnable;
	UINT8 reserved[2];
} EXT_CMD_TESTMODE_RXV_CTRL, *P_EXT_CMD_TESTMODE_RXV_CTRL;

/* RXV RU control cmd struct */
typedef struct _EXT_CMD_TESTMODE_RXV_RU_CTRL {
	UINT8 u1RxvCtrlFormatId;
	UINT8 u1RxvRuIdx;
	UINT8 reserved[2];
} EXT_CMD_TESTMODE_RXV_RU_CTRL, *P_EXT_CMD_TESTMODE_RXV_RU_CTRL;

typedef struct _EXT_CMD_GET_RX_STATISTIC_INFO {
	UINT8 u1RxvCtrlFormatId;
	UINT8 band_idx;
	UINT8 reserved[2];
} EXT_CMD_GET_RX_STATISTIC_INFO, *P_EXT_CMD_GET_RX_STATISTIC_INFO;

typedef struct _EXT_CMD_GET_RX_STATISTIC_INFO_BAND {
	UINT8 u1RxvCtrlFormatId;
	UINT8 band_idx;
	UINT8 reserved[2];
} EXT_CMD_GET_RX_STATISTIC_INFO_BAND, *P_EXT_CMD_GET_RX_STATISTIC_INFO_BAND;

typedef struct _EXT_CMD_GET_RX_STATISTIC_INFO_PATH {
	UINT8 u1RxvCtrlFormatId;
	UINT8 path_idx;
	UINT8 band_idx;
	UINT8 reserved[1];
} EXT_CMD_GET_RX_STATISTIC_INFO_PATH, *P_EXT_CMD_GET_RX_STATISTIC_INFO_PATH;

typedef struct _EXT_CMD_GET_RX_STATISTIC_INFO_USER {
	UINT8 u1RxvCtrlFormatId;
	UINT8 reserved;
	UINT16 user_idx;
} EXT_CMD_GET_RX_STATISTIC_INFO_USER, *P_EXT_CMD_GET_RX_STATISTIC_INFO_USER;

typedef struct _EXT_CMD_GET_RX_STATISTIC_INFO_COMM {
	UINT8 u1RxvCtrlFormatId;
	UINT8 reserved[3];
} EXT_CMD_GET_RX_STATISTIC_INFO_COMM, *P_EXT_CMD_GET_RX_STATISTIC_INFO_COMM;

typedef struct _EXT_CMD_SET_RX_STAT_USER {
	UINT8 band_idx;
	UINT8 reserved;
	UINT16 user_idx;
} EXT_CMD_SET_RX_STAT_USER, *P_EXT_CMD_SET_RX_STAT_USER;

typedef enum _TESTMODE_RXV_EVENT_CATEGORY {
	TESTMODE_RXV_EVENT_RXV_REPORT = 0x0,
	TESTMODE_RXV_EVENT_RX_STAT = 0x1,
	TESTMODE_RXV_EVENT_NUM
} TESTMODE_RXV_EVENT_CATEGORY, *P_TESTMODE_RXV_EVENT_CATEGORY;

/** struct for rx vector report event */
typedef struct _EXT_EVENT_TESTMODE_RX_VECTOR_REPORT_T {
	UINT8 u1EventCategoryID;
	UINT8 u1RxvFormatType;
	UINT8 u1RxvCbEntryType;
	UINT8 u1Reserved;

	/* rxv entry content */
	UINT16 u2PostMd;
	UINT16 u2PostRssiRu;

	UINT8 u1RxCeLtfSnr;
	UINT8 u1Reserved1[3];

	UINT32 u4TftFoe;

	UINT8 u1PostNoiseFloorRx0;
	UINT8 u1PostNoiseFloorRx1;
	UINT8 u1PostNoiseFloorRx2;
	UINT8 u1PostNoiseFloorRx3;

	UINT8 u1DecUserNum;
	UINT8 u1UserRate;
	UINT8 u1UserStreamNum;
	UINT8 u1UserRuAlloc;

	UINT16 u2MuAid;
	BOOLEAN fgRxFcsErr;
	UINT8 u1Reserved2;

	UINT32 u4OfdmRu26Snr0;
	UINT32 u4OfdmRu26Snr1;
	UINT32 u4OfdmRu26Snr2;
	UINT32 u4OfdmRu26Snr3;
	UINT32 u4OfdmRu26Snr4;
	UINT32 u4OfdmRu26Snr5;
	UINT32 u4OfdmRu26Snr6;
	UINT32 u4OfdmRu26Snr7;
	UINT32 u4OfdmRu26Snr8;
	UINT32 u4OfdmRu26Snr9;
} EXT_EVENT_TESTMODE_RX_VECTOR_REPORT_T, *P_EXT_EVENT_TESTMODE_RX_VECTOR_REPORT_T;

/* Test rx stat band info */
typedef struct _TEST_RX_STAT_BAND_INFO {
	UINT16 mac_rx_fcs_err_cnt;
	UINT32 mac_rx_mdrdy_cnt;
	UINT16 mac_rx_len_mismatch;
	UINT16 mac_rx_fcs_ok_cnt;
	UINT16 phy_rx_fcs_err_cnt_cck;
	UINT16 phy_rx_fcs_err_cnt_ofdm;
	UINT16 phy_rx_pd_cck;
	UINT16 phy_rx_pd_ofdm;
	UINT16 phy_rx_sig_err_cck;
	UINT16 phy_rx_sfd_err_cck;
	UINT16 phy_rx_sig_err_ofdm;
	UINT16 phy_rx_tag_err_ofdm;
	UINT16 phy_rx_mdrdy_cnt_cck;
	UINT16 phy_rx_mdrdy_cnt_ofdm;
} TEST_RX_STAT_BAND_INFO, *P_TEST_RX_STAT_BAND_INFO;

/* Test rx stat path info */
typedef struct _TEST_RX_STAT_PATH_INFO {
	UINT16 rcpi;
	INT16 rssi;
	CHAR fagc_ib_rssi;
	CHAR fagc_wb_rssi;
	CHAR inst_ib_rssi;
	CHAR inst_wb_rssi;
} TEST_RX_STAT_PATH_INFO, *P_TEST_RX_STAT_PATH_INFO;

/* Test rx stat user info */
typedef struct _TEST_RX_STAT_USER_INFO {
	INT32 freq_offset_from_rx;
	INT32 snr;
	UINT32 fcs_error_cnt;
} TEST_RX_STAT_USER_INFO, *P_TEST_RX_STAT_USER_INFO;

/* Test rx stat comm info */
typedef struct _TEST_RX_STAT_COMM_INFO {
	UINT16 rx_fifo_full;
	UINT32 aci_hit_low;
	UINT32 aci_hit_high;
	UINT32 mu_pkt_count;
	UINT32 sig_mcs;
	UINT32 sinr;
	UINT32 driver_rx_count;
} TEST_RX_STAT_COMM_INFO, *P_TEST_RX_STAT_COMM_INFO;

typedef struct _TESTMODE_STATISTIC_INFO {
	UINT8 u1EventCategoryID;
	/* mac part */
	UINT16 mac_rx_fcs_err_cnt;
	UINT16 mac_rx_len_mismatch;
	UINT16 mac_rx_fcs_ok_cnt;
	UINT16 mac_rx_fifo_full;
	UINT32 mac_rx_mdrdy_cnt;

	/* phy part */
	UINT16 phy_rx_fcs_err_cnt_cck;
	UINT16 phy_rx_fcs_err_cnt_ofdm;
	UINT16 phy_rx_pd_cck;
	UINT16 phy_rx_pd_ofdm;
	UINT16 phy_rx_sig_err_cck;
	UINT16 phy_rx_sfd_err_cck;
	UINT16 phy_rx_sig_err_ofdm;
	UINT16 phy_rx_tag_err_ofdm;
	UINT16 phy_rx_mdrdy_cnt_cck;
	UINT16 phy_rx_mdrdy_cnt_ofdm;

	UINT32 aci_hit_low;
	UINT32 aci_hit_high;

	/* rxv part */
	UINT16 rcpi[4];
	INT16 rssi[4];
	INT16 snr[4];

	/* RSSI */
	CHAR fagc_ib_rssi[4];
	CHAR fagc_wb_rssi[4];
	CHAR inst_ib_rssi[4];
	CHAR inst_wb_rssi[4];
} TESTMODE_STATISTIC_INFO, *P_TESTMODE_STATISTIC_INFO;

typedef struct _TESTMODE_STATISTIC_INFO_BAND {
	UINT8 u1EventCategoryID;
	/* mac part */
	UINT16 mac_rx_fcs_err_cnt;
	UINT16 mac_rx_len_mismatch;
	UINT16 mac_rx_fcs_ok_cnt;
	UINT32 mac_rx_mdrdy_cnt;
	/* phy part */
	UINT16 phy_rx_fcs_err_cnt_cck;
	UINT16 phy_rx_fcs_err_cnt_ofdm;
	UINT16 phy_rx_pd_cck;
	UINT16 phy_rx_pd_ofdm;
	UINT16 phy_rx_sig_err_cck;
	UINT16 phy_rx_sfd_err_cck;
	UINT16 phy_rx_sig_err_ofdm;
	UINT16 phy_rx_tag_err_ofdm;
	UINT16 phy_rx_mdrdy_cnt_cck;
	UINT16 phy_rx_mdrdy_cnt_ofdm;
} TESTMODE_STATISTIC_INFO_BAND, *P_TESTMODE_STATISTIC_INFO_BAND;

typedef struct _TESTMODE_STATISTIC_INFO_PATH {
	UINT8 u1EventCategoryID;
	CHAR inst_ib_rssi;
	CHAR inst_wb_rssi;
	UCHAR reserved;
} TESTMODE_STATISTIC_INFO_PATH, *P_TESTMODE_STATISTIC_INFO_PATH;

typedef struct _TESTMODE_STATISTIC_INFO_USER {
	UINT8 u1EventCategoryID;
	UCHAR reserved[3];
} TESTMODE_STATISTIC_INFO_USER, *P_TESTMODE_STATISTIC_INFO_USER;

typedef struct _TESTMODE_STATISTIC_INFO_COMM {
	UINT8 u1EventCategoryID;
	UINT16 mac_rx_fifo_full;
	UINT32 aci_hit_low;
	UINT32 aci_hit_high;
} TESTMODE_STATISTIC_INFO_COMM, *P_TESTMODE_STATISTIC_INFO_COMM;

typedef struct _EVENT_PHY_RXFELOSS_T {
	UINT8 u1BandIdx;
	INT8 i1FeLossComp[MAX_ANTENNA_NUM];
} EVENT_PHY_RXFELOSS_T, *P_EVENT_PHY_RXFELOSS_T;

/*MacInfo ID: Get Channel Busy Cnt(0x01) */
typedef struct _GET_CH_BUSY_CNT_T {
	UINT32 u4ChBusyCnt;
} GET_CH_BUSY_CNT_T, *P_GET_CH_BUSY_CNT_T;

/*MacInfo ID: 0x02 TSF*/
typedef struct _TSF_RESULT_T {
	UINT32 u4TsfBit0_31;
	UINT32 u4TsfBit63_32;
} TSF_RESULT_T, *P_TSFRESULT_T;

/*MacInfo ID: 0x03 Get partial MIB info*/
typedef struct _MIB_INFO_CNT_PARAM_T {
	UINT32 u4RxFcsErrCnt;
	UINT32 u4RxFifoOverflowCnt;
	UINT32 u4RxMpduCnt;
	UINT32 u4RxChannelIdleCnt;
	UINT32 u4CcaNavTxTimeCnt;
	UINT32 u4MdrdyCnt;
	UINT32 u4SCcaCnt;
	UINT32 u4PEdCnt;
	UINT32 u4RxTotalByteCnt;
} MIB_INFO_CNT_PARAM_T, *P_MIB_INFO_CNT_PARAM_T;

typedef struct GNU_PACKED _CMD_PARTIAL_MIB_INFO_CNT_SET_T {
	UINT8 ucBand;
	UINT8 ucAction;
	UINT8 ucTxModeValid;
	UINT8 ucTxMode;
	MIB_INFO_CNT_PARAM_T rMibInfoParam;
} CMD_PARTIAL_MIB_INFO_CNT_SET_T, CMD_PARTIAL_MIB_INFO_CNT_CTRL_T, *P_CMD_PARTIAL_MIB_INFO_CNT_CTRL_T, MT_PARTIAL_MIB_INFO_CNT_CTRL_T;

#if defined(MT7986) || defined(MT7916) || defined(MT7981)
enum ENUM_MIB_COUNTER_T {
    MIB_CNT_CCA_NAV_TX_TIME = 0, /* M0SDR9 */
    MIB_CNT_NAV_TIME = 1, /* MNCR */
    MIB_CNT_P_ED_TIME = 2, /* M0SDR18 */
    MIB_CNT_MAC2PHY_TX_TIME = 3, /* M0SDR35 */
    MIB_CNT_MAC2PHY_IDLE_TIME = 4, /* M2PITCR */
    MIB_CNT_TX_TIME_CNT = 5, /* M0SDR45 */
    MIB_CNT_TX_DUR_CNT = 6, /* M0SDR36 */
    MIB_CNT_TX_DUR_BACKOFF_CNT = 7, /* MAR1 */
    MIB_CNT_RX_DUR_CNT = 8, /* M0SDR37 */
    MIB_CNT_RX_DUR_BACKOFF_CNT = 9, /* MAR3 */
    MIB_CNT_OPPO_PS_RX_DIS = 10, /* M0SDR46 */
    MIB_CNT_OPPO_PS_RX_DIS_TIME = 11, /* M0SDR47 */
    MIB_CNT_OPPO_SX_OFF = 12, /* M0SDR48 */
    MIB_CNT_TXOP_TRUNC = 13, /* TXOPC2 */
    MIB_CNT_ARB_RWP_FAIL = 14, /* M0SDR27 */
    MIB_CNT_ARB_RWP_NEED = 15, /* M0SDR28 */
    MIB_CNT_ARB_TXCMD_UL_TXOK = 16, /* M0SDR66 */
    MIB_CNT_AMPDU = 17, /* M0SDR12 */
    MIB_CNT_BA_CNT = 18, /* M0SDR31 */
    MIB_CNT_AMPDU_EARLYSTOP = 19, /* M0SDR13 */
    MIB_CNT_AMPDU_MPDU = 20, /* M0SDR14 */
    MIB_CNT_AMPDU_ACKED = 21, /* M0SDR15 */
    MIB_CNT_MU_TX = 22, /* M0DR8 */
    MIB_CNT_MU_TX_OK = 23, /* M0DR9 */
    MIB_CNT_SU_TX_OK = 24, /* M0DR11 */
    MIB_CNT_MU_FAIL_PPDU = 25, /* M0DR10 */
    MIB_CNT_SR_AMPDU_MPDU = 26, /* SRC0 */
    MIB_CNT_SR_AMPDU_MPDU_ACKED = 27, /* SRC1 */
    MIB_CNT_DBNSS = 28, /* M0SDR50 */
    MIB_CNT_LTO_DROP = 29, /* M0DROPSR01 */
    MIB_CNT_RX_CCK_MDRDY_TIME = 30, /* M0SDR19 */
    MIB_CNT_RX_OFDM_LG_MIXED_MDRDY_TIME = 31, /* M0SDR20 */
    MIB_CNT_RX_OFDM_GREEN_MDRDY_TIME = 32, /* M0SDR21 */
    MIB_CNT_PF_DROP = 33, /* M0SDR29 */
    MIB_CNT_TX_DDLMT_RNG0 = 34, /* M0DR12 */
    MIB_CNT_SW_NDPFRP_TRIG = 35, /* TCSR4 */
    MIB_CNT_MDRDY = 36, /* M0SDR10 */
    MIB_CNT_AMDPU_RX_COUNT = 37, /* M0SDR22 */
    MIB_CNT_RX_TOTAL_BYTE = 38, /* M0SDR23 */
    MIB_CNT_RX_VALID_AMPDU_SF = 39, /* M0SDR24 */
    MIB_CNT_RX_VALID_BYTE = 40, /* M0SDR25 */
    MIB_CNT_RX_MPDU = 41, /* M0SDR5 */
    MIB_CNT_RX_NON_NO_DATA = 42, /* M0SDR41 */
    MIB_CNT_DELIMITER_FAIL = 43, /* M0SDR8 */
    MIB_CNT_LEN_MISMATCH = 44, /* M0SDR11 */
    MIB_CNT_PHY_MIB_COUNTER0 = 45, /* M0SDR26 */
    MIB_CNT_SRT_R = 46, /* SRTRCR */
    MIB_CNT_MUBF_TX = 47, /* M0SDR33 */
    MIB_CNT_RX_BF_HEFBK = 48, /* BFCR1 */
    MIB_CNT_BFEE_TXFBK_CPL = 49, /* BFCR7 */
    MIB_CNT_BFEE_COANT_BLKTX = 50, /* M0SDR58 */
    MIB_CNT_BFEE_FBK_SEG = 51, /* BFCR8 */
    MIB_CNT_P_CCA_TIME = 52, /* M0SDR16 */
    MIB_CNT_S_CCA_TIME = 53, /* M0SDR17 */
    MIB_CNT_CCA_TIME = 54, /* MCTR0 */
    MIB_CNT_S_P20BW_3_ED_TIME = 55, /* MCS2TR0 */
    MIB_CNT_S_P20BW_4_ED_TIME = 56, /* MCS2TR1 */
    MIB_CNT_S_P20BW_5_ED_TIME = 57, /* MCS2TR2 */
    MIB_CNT_S_P20BW_6_ED_TIME = 58, /* MCS2TR3 */
    MIB_CNT_S_P20BW_7_ED_TIME = 59, /* MCS2TR4 */
    MIB_CNT_S_20BW_CCA_TIME = 60, /* MCS2TR5 */
    MIB_CNT_S_40BW_CCA_TIME = 61, /* MCS2TR6 */
    MIB_CNT_S_80BW_CCA_TIME = 62, /* MCS2TR7 */
    MIB_CNT_S_P20BW_0_ED_TIME = 63, /* MCTR1 */
    MIB_CNT_S_P20BW_1_ED_TIME = 64, /* MCTR2 */
    MIB_CNT_S_P20BW_2_ED_TIME = 65, /* MCTR3 */
    MIB_CNT_EIFS_SLOT = 66, /* M0SDR6 */
    MIB_CNT_CHANNEL_IDLE = 67, /* M0SDR6 */
    MIB_CNT_EIFS_CCK = 68, /* M0SDR59 */
    MIB_CNT_EIFS_OFDM = 69, /* M0SDR59 */
    MIB_CNT_ED_LISTEN_BELOW = 70, /* M0SDR44 */
    MIB_CNT_ED_LISTEN_ABOVE = 71, /* M0SDR44 */
    MIB_CNT_TXOP_BURST = 72, /* TXOPC0 */
    MIB_CNT_TXOP_INIT = 73, /* TXOPC0 */
    MIB_CNT_TXOP_TXV_TOUT = 74, /* TXOPC1 */
    MIB_CNT_TXOP_EXCEED = 75, /* TXOPC1 */
    MIB_CNT_TX_ABORT_CNT1 = 76, /* M0CABT0 */
    MIB_CNT_TX_ABORT_CNT0 = 77, /* M0CABT0 */
    MIB_CNT_TX_ABORT_CNT3 = 78, /* M0CABT1 */
    MIB_CNT_TX_ABORT_CNT2 = 79, /* M0CABT1 */
    MIB_CNT_TX_ABORT_CNT5 = 80, /* M0TABT2 */
    MIB_CNT_TX_ABORT_CNT4 = 81, /* M0TABT2 */
    MIB_CNT_TX_ABORT_CNT7 = 82, /* M0TABT3 */
    MIB_CNT_TX_ABORT_CNT6 = 83, /* M0TABT3 */
    MIB_CNT_TX_BW_40MHZ = 84, /* M0DR0 */
    MIB_CNT_TX_BW_20MHZ = 85, /* M0DR0 */
    MIB_CNT_TX_BW_160MHZ = 86, /* M0DR1 */
    MIB_CNT_TX_BW_80MHZ = 87, /* M0DR1 */
    MIB_CNT_COANT_BF_SEQ_DROP = 88, /* M0SDR56 */
    MIB_CNT_COANT_HW_FB_TX_DIM = 89, /* M0SDR56 */
    MIB_CNT_MPDU_RETRY_DROP = 90, /* M0DROPSR00 */
    MIB_CNT_RTS_DROP = 91, /* M0DROPSR00 */
    MIB_CNT_VEC_DROP = 92, /* M0SDR7 */
    MIB_CNT_VEC_MISMATCH = 93, /* M0SDR7 */
    MIB_CNT_TX_DDLMT_RNG2 = 94, /* M0DR6 */
    MIB_CNT_TX_DDLMT_RNG1 = 95, /* M0DR6 */
    MIB_CNT_TX_DDLMT_RNG4 = 96, /* M0DR7 */
    MIB_CNT_TX_DDLMT_RNG3 = 97, /* M0DR7 */
    MIB_CNT_RU0_TX = 98, /* RUTCR */
    MIB_CNT_RU1_TX = 99, /* RUTCR */
    MIB_CNT_SW_BASIC_TRIG = 100, /* TCSR0 */
    MIB_CNT_HW_BASIC_TRIG = 101, /* TCSR0 */
    MIB_CNT_HW_MUBAR_TRIG = 102, /* TCSR1 */
    MIB_CNT_SW_BRP_TRIG = 103, /* TCSR1 */
    MIB_CNT_SW_BSRP_TRIG = 104, /* TCSR2 */
    MIB_CNT_HW_MURTS_TRIG = 105, /* TCSR2 */
    MIB_CNT_SW_BQRP_TRIG = 106, /* TCSR3 */
    MIB_CNT_SW_GCRMUBAR_TRIG = 107, /* TCSR3 */
    MIB_CNT_RX_FCS_ERR = 108, /* M0SDR3 */
    MIB_CNT_RX_FCS_OK = 109, /* M0SDR3 */
    MIB_CNT_NSS1_MCS0_FCS_ERR = 110, /* M0NSS1MCS0 */
    MIB_CNT_NSS1_MCS0_FCS_OK = 111, /* M0NSS1MCS0 */
    MIB_CNT_NSS1_MCS1_FCS_ERR = 112, /* M0NSS1MCS1 */
    MIB_CNT_NSS1_MCS1_FCS_OK = 113, /* M0NSS1MCS1 */
    MIB_CNT_NSS1_MCS2_FCS_ERR = 114, /* M0NSS1MCS2 */
    MIB_CNT_NSS1_MCS2_FCS_OK = 115, /* M0NSS1MCS2 */
    MIB_CNT_NSS1_MCS3_FCS_ERR = 116, /* M0NSS1MCS3 */
    MIB_CNT_NSS1_MCS3_FCS_OK = 117, /* M0NSS1MCS3 */
    MIB_CNT_NSS1_MCS4_FCS_ERR = 118, /* M0NSS1MCS4 */
    MIB_CNT_NSS1_MCS4_FCS_OK = 119, /* M0NSS1MCS4 */
    MIB_CNT_NSS1_MCS5_FCS_ERR = 120, /* M0NSS1MCS5 */
    MIB_CNT_NSS1_MCS5_FCS_OK = 121, /* M0NSS1MCS5 */
    MIB_CNT_NSS1_MCS6_FCS_ERR = 122, /* M0NSS1MCS6 */
    MIB_CNT_NSS1_MCS6_FCS_OK = 123, /* M0NSS1MCS6 */
    MIB_CNT_NSS1_MCS7_FCS_ERR = 124, /* M0NSS1MCS7 */
    MIB_CNT_NSS1_MCS7_FCS_OK = 125, /* M0NSS1MCS7 */
    MIB_CNT_NSS1_MCS8_FCS_ERR = 126, /* M0NSS1MCS8 */
    MIB_CNT_NSS1_MCS8_FCS_OK = 127, /* M0NSS1MCS8 */
    MIB_CNT_NSS1_MCS9_FCS_ERR = 128, /* M0NSS1MCS9 */
    MIB_CNT_NSS1_MCS9_FCS_OK = 129, /* M0NSS1MCS9 */
    MIB_CNT_NSS2_MCS0_FCS_ERR = 130, /* M0NSS2MCS0 */
    MIB_CNT_NSS2_MCS0_FCS_OK = 131, /* M0NSS2MCS0 */
    MIB_CNT_NSS2_MCS1_FCS_ERR = 132, /* M0NSS2MCS1 */
    MIB_CNT_NSS2_MCS1_FCS_OK = 133, /* M0NSS2MCS1 */
    MIB_CNT_NSS2_MCS2_FCS_ERR = 134, /* M0NSS2MCS2 */
    MIB_CNT_NSS2_MCS2_FCS_OK = 135, /* M0NSS2MCS2 */
    MIB_CNT_NSS2_MCS3_FCS_ERR = 136, /* M0NSS2MCS3 */
    MIB_CNT_NSS2_MCS3_FCS_OK = 137, /* M0NSS2MCS3 */
    MIB_CNT_NSS2_MCS4_FCS_ERR = 138, /* M0NSS2MCS4 */
    MIB_CNT_NSS2_MCS4_FCS_OK = 139, /* M0NSS2MCS4 */
    MIB_CNT_NSS2_MCS5_FCS_ERR = 140, /* M0NSS2MCS5 */
    MIB_CNT_NSS2_MCS5_FCS_OK = 141, /* M0NSS2MCS5 */
    MIB_CNT_NSS2_MCS6_FCS_ERR = 142, /* M0NSS2MCS6 */
    MIB_CNT_NSS2_MCS6_FCS_OK = 143, /* M0NSS2MCS6 */
    MIB_CNT_NSS2_MCS7_FCS_ERR = 144, /* M0NSS2MCS7 */
    MIB_CNT_NSS2_MCS7_FCS_OK = 145, /* M0NSS2MCS7 */
    MIB_CNT_NSS2_MCS8_FCS_ERR = 146, /* M0NSS2MCS8 */
    MIB_CNT_NSS2_MCS8_FCS_OK = 147, /* M0NSS2MCS8 */
    MIB_CNT_NSS2_MCS9_FCS_ERR = 148, /* M0NSS2MCS9 */
    MIB_CNT_NSS2_MCS9_FCS_OK = 149, /* M0NSS2MCS9 */
    MIB_CNT_NSS3_MCS0_FCS_ERR = 150, /* M0NSS3MCS0 */
    MIB_CNT_NSS3_MCS0_FCS_OK = 151, /* M0NSS3MCS0 */
    MIB_CNT_NSS3_MCS1_FCS_ERR = 152, /* M0NSS3MCS1 */
    MIB_CNT_NSS3_MCS1_FCS_OK = 153, /* M0NSS3MCS1 */
    MIB_CNT_NSS3_MCS2_FCS_ERR = 154, /* M0NSS3MCS2 */
    MIB_CNT_NSS3_MCS2_FCS_OK = 155, /* M0NSS3MCS2 */
    MIB_CNT_NSS3_MCS3_FCS_ERR = 156, /* M0NSS3MCS3 */
    MIB_CNT_NSS3_MCS3_FCS_OK = 157, /* M0NSS3MCS3 */
    MIB_CNT_NSS3_MCS4_FCS_ERR = 158, /* M0NSS3MCS4 */
    MIB_CNT_NSS3_MCS4_FCS_OK = 159, /* M0NSS3MCS4 */
    MIB_CNT_NSS3_MCS5_FCS_ERR = 160, /* M0NSS3MCS5 */
    MIB_CNT_NSS3_MCS5_FCS_OK = 161, /* M0NSS3MCS5 */
    MIB_CNT_NSS3_MCS6_FCS_ERR = 162, /* M0NSS3MCS6 */
    MIB_CNT_NSS3_MCS6_FCS_OK = 163, /* M0NSS3MCS6 */
    MIB_CNT_NSS3_MCS7_FCS_ERR = 164, /* M0NSS3MCS7 */
    MIB_CNT_NSS3_MCS7_FCS_OK = 165, /* M0NSS3MCS7 */
    MIB_CNT_NSS3_MCS8_FCS_ERR = 166, /* M0NSS3MCS8 */
    MIB_CNT_NSS3_MCS8_FCS_OK = 167, /* M0NSS3MCS8 */
    MIB_CNT_NSS3_MCS9_FCS_ERR = 168, /* M0NSS3MCS9 */
    MIB_CNT_NSS3_MCS9_FCS_OK = 169, /* M0NSS3MCS9 */
    MIB_CNT_NSS4_MCS0_FCS_ERR = 170, /* M0NSS4MCS0 */
    MIB_CNT_NSS4_MCS0_FCS_OK = 171, /* M0NSS4MCS0 */
    MIB_CNT_NSS4_MCS1_FCS_ERR = 172, /* M0NSS4MCS1 */
    MIB_CNT_NSS4_MCS1_FCS_OK = 173, /* M0NSS4MCS1 */
    MIB_CNT_NSS4_MCS2_FCS_ERR = 174, /* M0NSS4MCS2 */
    MIB_CNT_NSS4_MCS2_FCS_OK = 175, /* M0NSS4MCS2 */
    MIB_CNT_NSS4_MCS3_FCS_ERR = 176, /* M0NSS4MCS3 */
    MIB_CNT_NSS4_MCS3_FCS_OK = 177, /* M0NSS4MCS3 */
    MIB_CNT_NSS4_MCS4_FCS_ERR = 178, /* M0NSS4MCS4 */
    MIB_CNT_NSS4_MCS4_FCS_OK = 179, /* M0NSS4MCS4 */
    MIB_CNT_NSS4_MCS5_FCS_ERR = 180, /* M0NSS4MCS5 */
    MIB_CNT_NSS4_MCS5_FCS_OK = 181, /* M0NSS4MCS5 */
    MIB_CNT_NSS4_MCS6_FCS_ERR = 182, /* M0NSS4MCS6 */
    MIB_CNT_NSS4_MCS6_FCS_OK = 183, /* M0NSS4MCS6 */
    MIB_CNT_NSS4_MCS7_FCS_ERR = 184, /* M0NSS4MCS7 */
    MIB_CNT_NSS4_MCS7_FCS_OK = 185, /* M0NSS4MCS7 */
    MIB_CNT_NSS4_MCS8_FCS_ERR = 186, /* M0NSS4MCS8 */
    MIB_CNT_NSS4_MCS8_FCS_OK = 187, /* M0NSS4MCS8 */
    MIB_CNT_NSS4_MCS9_FCS_ERR = 188, /* M0NSS4MCS9 */
    MIB_CNT_NSS4_MCS9_FCS_OK = 189, /* M0NSS4MCS9 */
    MIB_CNT_UC2ME_DATA_NSS_BSSID1 = 190, /* M0SDR52 */
    MIB_CNT_UC2ME_DATA_NSS_BSSID0 = 191, /* M0SDR52 */
    MIB_CNT_UC2ME_DATA_NSS_BSSID3 = 192, /* M0SDR53 */
    MIB_CNT_UC2ME_DATA_NSS_BSSID2 = 193, /* M0SDR53 */
    MIB_CNT_RX_DUP_DROP_BSSID1 = 194, /* M0SDR60 */
    MIB_CNT_RX_DUP_DROP_BSSID0 = 195, /* M0SDR60 */
    MIB_CNT_RX_DUP_DROP_BSSID3 = 196, /* M0SDR61 */
    MIB_CNT_RX_DUP_DROP_BSSID2 = 197, /* M0SDR61 */
    MIB_CNT_RX_PARTIAL_BCN1 = 198, /* M0SDR42 */
    MIB_CNT_RX_PARTIAL_BCN0 = 199, /* M0SDR42 */
    MIB_CNT_RX_PARTIAL_BCN3 = 200, /* M0SDR43 */
    MIB_CNT_RX_PARTIAL_BCN2 = 201, /* M0SDR43 */
    MIB_CNT_DTIM1_UPDATE_CHK_FAIL = 202, /* M0SDR49 */
    MIB_CNT_DTIM0_UPDATE_CHK_FAIL = 203, /* M0SDR49 */
    MIB_CNT_DTIM3_UPDATE_CHK_FAIL = 204, /* M0SDR67 */
    MIB_CNT_DTIM2_UPDATE_CHK_FAIL = 205, /* M0SDR67 */
    MIB_CNT_RX_TSF_UP_BSSID1 = 206, /* M0SDR62 */
    MIB_CNT_RX_TSF_UP_BSSID0 = 207, /* M0SDR62 */
    MIB_CNT_RX_TSF_UP_BSSID3 = 208, /* M0SDR63 */
    MIB_CNT_RX_TSF_UP_BSSID2 = 209, /* M0SDR63 */
    MIB_CNT_RX_BTIM_UP_BSSID1 = 210, /* M0SDR64 */
    MIB_CNT_RX_BTIM_UP_BSSID0 = 211, /* M0SDR64 */
    MIB_CNT_RX_BTIM_UP_BSSID3 = 212, /* M0SDR65 */
    MIB_CNT_RX_BTIM_UP_BSSID2 = 213, /* M0SDR65 */
    MIB_CNT_RX_OUT_OF_RANGE_COUNT = 214, /* M0SDR4 */
    MIB_CNT_RX_FIFO_OVERFLOW = 215, /* M0SDR4 */
    MIB_CNT_SRG_VLD = 216, /* SRVCR */
    MIB_CNT_NONSRG_VLD = 217, /* SRVCR */
    MIB_CNT_SRG_PPDUVLD = 218, /* SRCPVCR */
    MIB_CNT_NONSRG_PPDUVLD = 219, /* SRCPVCR */
    MIB_CNT_INTRABSS_PPDU = 220, /* OPCR */
    MIB_CNT_INTERBSS_PPDU = 221, /* OPCR */
    MIB_CNT_IBF_TX = 222, /* M0SDR32 */
    MIB_CNT_EBF_TX = 223, /* M0SDR32 */
    MIB_CNT_RX_BF_VHTFBK = 224, /* BFCR0 */
    MIB_CNT_RX_BF_HTFBK = 225, /* BFCR0 */
    MIB_CNT_BFEE_TXFBK_BFPOLL_TRI = 226, /* BFCR2 */
    MIB_CNT_BFEE_TXFBK_TRI = 227, /* BFCR2 */
    MIB_CNT_BFEE_RX_FBKCQI = 228, /* BFCR3 */
    MIB_CNT_BFEE_RX_NDP = 229, /* BFCR3 */
    MIB_CNT_RX_BF_IBF_UPT = 230, /* BFCR4 */
    MIB_CNT_RX_BF_EBF_UPT = 231, /* BFCR4 */
    MIB_CNT_BFEE_SP_ABORT = 232, /* BFCR5 */
    MIB_CNT_BFEE_TB_LEN_ERR = 233, /* BFCR5 */
    MIB_CNT_BFEE_TXFBK_MUTE = 234, /* BFCR6 */
    MIB_CNT_BFEE_TMAC_ABORT = 235, /* BFCR6 */
    MIB_CNT_BCN_RX = 236, /* M0SDR0 */
    MIB_CNT_BCN_TX = 237, /* M0SDR0 */
    MIB_CNT_TRX_AGG_RANGE1 = 238, /* M0DR2 */
    MIB_CNT_TRX_AGG_RANGE0 = 239, /* M0DR2 */
    MIB_CNT_TRX_AGG_RANGE3 = 240, /* M0DR3 */
    MIB_CNT_TRX_AGG_RANGE2 = 241, /* M0DR3 */
    MIB_CNT_TRX_AGG_RANGE5 = 242, /* M0DR4 */
    MIB_CNT_TRX_AGG_RANGE4 = 243, /* M0DR4 */
    MIB_CNT_TRX_AGG_RANGE7 = 244, /* M0DR5 */
    MIB_CNT_TRX_AGG_RANGE6 = 245, /* M0DR5 */
    MIB_CNT_TRX_AGG_RANGE9 = 246, /* M0DR13 */
    MIB_CNT_TRX_AGG_RANGE8 = 247, /* M0DR13 */
    MIB_CNT_TRX_AGG_RANGE11 = 248, /* M0DR14 */
    MIB_CNT_TRX_AGG_RANGE10 = 249, /* M0DR14 */
    MIB_CNT_TRX_AGG_RANGE13 = 250, /* M0DR15 */
    MIB_CNT_TRX_AGG_RANGE12 = 251, /* M0DR15 */
    MIB_CNT_TRX_AGG_RANGE15 = 252, /* M0DR16 */
    MIB_CNT_TRX_AGG_RANGE14 = 253, /* M0DR16 */
    MIB_CNT_BSS0_TX_OK = 254, /* BTOCR */
    MIB_CNT_BSS1_TX_OK = 255, /* BTOCR */
    MIB_CNT_BSS2_TX_OK = 256, /* BTOCR */
    MIB_CNT_BSS3_TX_OK = 257, /* BTOCR */
    MIB_CNT_MBSS0_TX_OK = 258, /* BTOCR */
    MIB_CNT_MBSS1_TX_OK = 259, /* BTOCR */
    MIB_CNT_MBSS2_TX_OK = 260, /* BTOCR */
    MIB_CNT_MBSS3_TX_OK = 261, /* BTOCR */
    MIB_CNT_MBSS4_TX_OK = 262, /* BTOCR */
    MIB_CNT_MBSS5_TX_OK = 263, /* BTOCR */
    MIB_CNT_MBSS6_TX_OK = 264, /* BTOCR */
    MIB_CNT_MBSS7_TX_OK = 265, /* BTOCR */
    MIB_CNT_MBSS8_TX_OK = 266, /* BTOCR */
    MIB_CNT_MBSS9_TX_OK = 267, /* BTOCR */
    MIB_CNT_MBSS10_TX_OK = 268, /* BTOCR */
    MIB_CNT_MBSS11_TX_OK = 269, /* BTOCR */
    MIB_CNT_MBSS12_TX_OK = 270, /* BTOCR */
    MIB_CNT_MBSS13_TX_OK = 271, /* BTOCR */
    MIB_CNT_MBSS14_TX_OK = 272, /* BTOCR */
    MIB_CNT_MBSS15_TX_OK = 273, /* BTOCR */
    MIB_CNT_BSS0_TX_BYTE = 274, /* BTBCR */
    MIB_CNT_BSS1_TX_BYTE = 275, /* BTBCR */
    MIB_CNT_BSS2_TX_BYTE = 276, /* BTBCR */
    MIB_CNT_BSS3_TX_BYTE = 277, /* BTBCR */
    MIB_CNT_MBSS0_TX_BYTE = 278, /* BTBCR */
    MIB_CNT_MBSS1_TX_BYTE = 279, /* BTBCR */
    MIB_CNT_MBSS2_TX_BYTE = 280, /* BTBCR */
    MIB_CNT_MBSS3_TX_BYTE = 281, /* BTBCR */
    MIB_CNT_MBSS4_TX_BYTE = 282, /* BTBCR */
    MIB_CNT_MBSS5_TX_BYTE = 283, /* BTBCR */
    MIB_CNT_MBSS6_TX_BYTE = 284, /* BTBCR */
    MIB_CNT_MBSS7_TX_BYTE = 285, /* BTBCR */
    MIB_CNT_MBSS8_TX_BYTE = 286, /* BTBCR */
    MIB_CNT_MBSS9_TX_BYTE = 287, /* BTBCR */
    MIB_CNT_MBSS10_TX_BYTE = 288, /* BTBCR */
    MIB_CNT_MBSS11_TX_BYTE = 289, /* BTBCR */
    MIB_CNT_MBSS12_TX_BYTE = 290, /* BTBCR */
    MIB_CNT_MBSS13_TX_BYTE = 291, /* BTBCR */
    MIB_CNT_MBSS14_TX_BYTE = 292, /* BTBCR */
    MIB_CNT_MBSS15_TX_BYTE = 293, /* BTBCR */
    MIB_CNT_BSS0_MGMT_RETRY = 294, /* BTMRCR */
    MIB_CNT_BSS1_MGMT_RETRY = 295, /* BTMRCR */
    MIB_CNT_BSS2_MGMT_RETRY = 296, /* BTMRCR */
    MIB_CNT_BSS3_MGMT_RETRY = 297, /* BTMRCR */
    MIB_CNT_MBSS0_MGMT_RETRY = 298, /* BTMRCR */
    MIB_CNT_MBSS1_MGMT_RETRY = 299, /* BTMRCR */
    MIB_CNT_MBSS2_MGMT_RETRY = 300, /* BTMRCR */
    MIB_CNT_MBSS3_MGMT_RETRY = 301, /* BTMRCR */
    MIB_CNT_MBSS4_MGMT_RETRY = 302, /* BTMRCR */
    MIB_CNT_MBSS5_MGMT_RETRY = 303, /* BTMRCR */
    MIB_CNT_MBSS6_MGMT_RETRY = 304, /* BTMRCR */
    MIB_CNT_MBSS7_MGMT_RETRY = 305, /* BTMRCR */
    MIB_CNT_MBSS8_MGMT_RETRY = 306, /* BTMRCR */
    MIB_CNT_MBSS9_MGMT_RETRY = 307, /* BTMRCR */
    MIB_CNT_MBSS10_MGMT_RETRY = 308, /* BTMRCR */
    MIB_CNT_MBSS11_MGMT_RETRY = 309, /* BTMRCR */
    MIB_CNT_MBSS12_MGMT_RETRY = 310, /* BTMRCR */
    MIB_CNT_MBSS13_MGMT_RETRY = 311, /* BTMRCR */
    MIB_CNT_MBSS14_MGMT_RETRY = 312, /* BTMRCR */
    MIB_CNT_MBSS15_MGMT_RETRY = 313, /* BTMRCR */
    MIB_CNT_BSS0_DATA_RETRY = 314, /* BTDRCR */
    MIB_CNT_BSS1_DATA_RETRY = 315, /* BTDRCR */
    MIB_CNT_BSS2_DATA_RETRY = 316, /* BTDRCR */
    MIB_CNT_BSS3_DATA_RETRY = 317, /* BTDRCR */
    MIB_CNT_MBSS0_DATA_RETRY = 318, /* BTDRCR */
    MIB_CNT_MBSS1_DATA_RETRY = 319, /* BTDRCR */
    MIB_CNT_MBSS2_DATA_RETRY = 320, /* BTDRCR */
    MIB_CNT_MBSS3_DATA_RETRY = 321, /* BTDRCR */
    MIB_CNT_MBSS4_DATA_RETRY = 322, /* BTDRCR */
    MIB_CNT_MBSS5_DATA_RETRY = 323, /* BTDRCR */
    MIB_CNT_MBSS6_DATA_RETRY = 324, /* BTDRCR */
    MIB_CNT_MBSS7_DATA_RETRY = 325, /* BTDRCR */
    MIB_CNT_MBSS8_DATA_RETRY = 326, /* BTDRCR */
    MIB_CNT_MBSS9_DATA_RETRY = 327, /* BTDRCR */
    MIB_CNT_MBSS10_DATA_RETRY = 328, /* BTDRCR */
    MIB_CNT_MBSS11_DATA_RETRY = 329, /* BTDRCR */
    MIB_CNT_MBSS12_DATA_RETRY = 330, /* BTDRCR */
    MIB_CNT_MBSS13_DATA_RETRY = 331, /* BTDRCR */
    MIB_CNT_MBSS14_DATA_RETRY = 332, /* BTDRCR */
    MIB_CNT_MBSS15_DATA_RETRY = 333, /* BTDRCR */
    MIB_CNT_BSS0_CTRL_FRAME_CNT = 334, /* BTCCR */
    MIB_CNT_BSS1_CTRL_FRAME_CNT = 335, /* BTCCR */
    MIB_CNT_BSS2_CTRL_FRAME_CNT = 336, /* BTCCR */
    MIB_CNT_BSS3_CTRL_FRAME_CNT = 337, /* BTCCR */
    MIB_CNT_MBSS0_CTRL_FRAME_CNT = 338, /* BTCCR */
    MIB_CNT_MBSS1_CTRL_FRAME_CNT = 339, /* BTCCR */
    MIB_CNT_MBSS2_CTRL_FRAME_CNT = 340, /* BTCCR */
    MIB_CNT_MBSS3_CTRL_FRAME_CNT = 341, /* BTCCR */
    MIB_CNT_MBSS4_CTRL_FRAME_CNT = 342, /* BTCCR */
    MIB_CNT_MBSS5_CTRL_FRAME_CNT = 343, /* BTCCR */
    MIB_CNT_MBSS6_CTRL_FRAME_CNT = 344, /* BTCCR */
    MIB_CNT_MBSS7_CTRL_FRAME_CNT = 345, /* BTCCR */
    MIB_CNT_MBSS8_CTRL_FRAME_CNT = 346, /* BTCCR */
    MIB_CNT_MBSS9_CTRL_FRAME_CNT = 347, /* BTCCR */
    MIB_CNT_MBSS10_CTRL_FRAME_CNT = 348, /* BTCCR */
    MIB_CNT_MBSS11_CTRL_FRAME_CNT = 349, /* BTCCR */
    MIB_CNT_MBSS12_CTRL_FRAME_CNT = 350, /* BTCCR */
    MIB_CNT_MBSS13_CTRL_FRAME_CNT = 351, /* BTCCR */
    MIB_CNT_MBSS14_CTRL_FRAME_CNT = 352, /* BTCCR */
    MIB_CNT_MBSS15_CTRL_FRAME_CNT = 353, /* BTCCR */
    MIB_CNT_BSS0_TX_DATA = 354, /* BTDCR */
    MIB_CNT_BSS1_TX_DATA = 355, /* BTDCR */
    MIB_CNT_BSS2_TX_DATA = 356, /* BTDCR */
    MIB_CNT_BSS3_TX_DATA = 357, /* BTDCR */
    MIB_CNT_BSS0_TX = 358, /* BTCR */
    MIB_CNT_BSS1_TX = 359, /* BTCR */
    MIB_CNT_BSS2_TX = 360, /* BTCR */
    MIB_CNT_BSS3_TX = 361, /* BTCR */
    MIB_CNT_BSS0_TX_FAIL = 362, /* BTFCR */
    MIB_CNT_BSS1_TX_FAIL = 363, /* BTFCR */
    MIB_CNT_BSS2_TX_FAIL = 364, /* BTFCR */
    MIB_CNT_BSS3_TX_FAIL = 365, /* BTFCR */
    MIB_CNT_BSS0_FDD_RW_TX = 366, /* BFTCR */
    MIB_CNT_BSS1_FDD_RW_TX = 367, /* BFTCR */
    MIB_CNT_BSS2_FDD_RW_TX = 368, /* BFTCR */
    MIB_CNT_BSS3_FDD_RW_TX = 369, /* BFTCR */
    MIB_CNT_BSS0_FDD_RW_TX_FAIL = 370, /* BFTFCR */
    MIB_CNT_BSS1_FDD_RW_TX_FAIL = 371, /* BFTFCR */
    MIB_CNT_BSS2_FDD_RW_TX_FAIL = 372, /* BFTFCR */
    MIB_CNT_BSS3_FDD_RW_TX_FAIL = 373, /* BFTFCR */
    MIB_CNT_BSS0_BA_MISS = 374, /* BSDR2 */
    MIB_CNT_BSS1_BA_MISS = 375, /* BSDR2 */
    MIB_CNT_BSS2_BA_MISS = 376, /* BSDR2 */
    MIB_CNT_BSS3_BA_MISS = 377, /* BSDR2 */
    MIB_CNT_BSS0_ACK_FAIL = 378, /* BSDR3 */
    MIB_CNT_BSS1_ACK_FAIL = 379, /* BSDR3 */
    MIB_CNT_BSS2_ACK_FAIL = 380, /* BSDR3 */
    MIB_CNT_BSS3_ACK_FAIL = 381, /* BSDR3 */
    MIB_CNT_BSS0_FRAME_RETRY = 382, /* BSDR4 */
    MIB_CNT_BSS1_FRAME_RETRY = 383, /* BSDR4 */
    MIB_CNT_BSS2_FRAME_RETRY = 384, /* BSDR4 */
    MIB_CNT_BSS3_FRAME_RETRY = 385, /* BSDR4 */
    MIB_CNT_BSS0_FRAME_RETRY_2 = 386, /* BSDR5 */
    MIB_CNT_BSS1_FRAME_RETRY_2 = 387, /* BSDR5 */
    MIB_CNT_BSS2_FRAME_RETRY_2 = 388, /* BSDR5 */
    MIB_CNT_BSS3_FRAME_RETRY_2 = 389, /* BSDR5 */
    MIB_CNT_BSS0_FRAME_RETRY_3 = 390, /* BSDR6 */
    MIB_CNT_BSS1_FRAME_RETRY_3 = 391, /* BSDR6 */
    MIB_CNT_BSS2_FRAME_RETRY_3 = 392, /* BSDR6 */
    MIB_CNT_BSS3_FRAME_RETRY_3 = 393, /* BSDR6 */
    MIB_CNT_TX_AC0 = 394, /* ATCR */
    MIB_CNT_TX_AC1 = 395, /* ATCR */
    MIB_CNT_TX_AC2 = 396, /* ATCR */
    MIB_CNT_TX_AC3 = 397, /* ATCR */
    MIB_CNT_TX_RETRY_AC0 = 398, /* ATRCR */
    MIB_CNT_TX_RETRY_AC1 = 399, /* ATRCR */
    MIB_CNT_TX_RETRY_AC2 = 400, /* ATRCR */
    MIB_CNT_TX_RETRY_AC3 = 401, /* ATRCR */
    MIB_CNT_TX_DROP_AC0 = 402, /* ATDCR */
    MIB_CNT_TX_DROP_AC1 = 403, /* ATDCR */
    MIB_CNT_TX_DROP_AC2 = 404, /* ATDCR */
    MIB_CNT_TX_DROP_AC3 = 405, /* ATDCR */
    MIB_CNT_BSS0_RX_OK = 406, /* BROCR */
    MIB_CNT_BSS1_RX_OK = 407, /* BROCR */
    MIB_CNT_BSS2_RX_OK = 408, /* BROCR */
    MIB_CNT_BSS3_RX_OK = 409, /* BROCR */
    MIB_CNT_MBSS0_RX_OK = 410, /* BROCR */
    MIB_CNT_MBSS1_RX_OK = 411, /* BROCR */
    MIB_CNT_MBSS2_RX_OK = 412, /* BROCR */
    MIB_CNT_MBSS3_RX_OK = 413, /* BROCR */
    MIB_CNT_MBSS4_RX_OK = 414, /* BROCR */
    MIB_CNT_MBSS5_RX_OK = 415, /* BROCR */
    MIB_CNT_MBSS6_RX_OK = 416, /* BROCR */
    MIB_CNT_MBSS7_RX_OK = 417, /* BROCR */
    MIB_CNT_MBSS8_RX_OK = 418, /* BROCR */
    MIB_CNT_MBSS9_RX_OK = 419, /* BROCR */
    MIB_CNT_MBSS10_RX_OK = 420, /* BROCR */
    MIB_CNT_MBSS11_RX_OK = 421, /* BROCR */
    MIB_CNT_MBSS12_RX_OK = 422, /* BROCR */
    MIB_CNT_MBSS13_RX_OK = 423, /* BROCR */
    MIB_CNT_MBSS14_RX_OK = 424, /* BROCR */
    MIB_CNT_MBSS15_RX_OK = 425, /* BROCR */
    MIB_CNT_BSS0_RX_BYTE = 426, /* BRBCR */
    MIB_CNT_BSS1_RX_BYTE = 427, /* BRBCR */
    MIB_CNT_BSS2_RX_BYTE = 428, /* BRBCR */
    MIB_CNT_BSS3_RX_BYTE = 429, /* BRBCR */
    MIB_CNT_MBSS0_RX_BYTE = 430, /* BRBCR */
    MIB_CNT_MBSS1_RX_BYTE = 431, /* BRBCR */
    MIB_CNT_MBSS2_RX_BYTE = 432, /* BRBCR */
    MIB_CNT_MBSS3_RX_BYTE = 433, /* BRBCR */
    MIB_CNT_MBSS4_RX_BYTE = 434, /* BRBCR */
    MIB_CNT_MBSS5_RX_BYTE = 435, /* BRBCR */
    MIB_CNT_MBSS6_RX_BYTE = 436, /* BRBCR */
    MIB_CNT_MBSS7_RX_BYTE = 437, /* BRBCR */
    MIB_CNT_MBSS8_RX_BYTE = 438, /* BRBCR */
    MIB_CNT_MBSS9_RX_BYTE = 439, /* BRBCR */
    MIB_CNT_MBSS10_RX_BYTE = 440, /* BRBCR */
    MIB_CNT_MBSS11_RX_BYTE = 441, /* BRBCR */
    MIB_CNT_MBSS12_RX_BYTE = 442, /* BRBCR */
    MIB_CNT_MBSS13_RX_BYTE = 443, /* BRBCR */
    MIB_CNT_MBSS14_RX_BYTE = 444, /* BRBCR */
    MIB_CNT_MBSS15_RX_BYTE = 445, /* BRBCR */
    MIB_CNT_BSS0_RX_DATA = 446, /* BRDCR */
    MIB_CNT_BSS1_RX_DATA = 447, /* BRDCR */
    MIB_CNT_BSS2_RX_DATA = 448, /* BRDCR */
    MIB_CNT_BSS3_RX_DATA = 449, /* BRDCR */
    MIB_CNT_BSS0_RX_MSDU = 450, /* BRMCR */
    MIB_CNT_BSS1_RX_MSDU = 451, /* BRMCR */
    MIB_CNT_BSS2_RX_MSDU = 452, /* BRMCR */
    MIB_CNT_BSS3_RX_MSDU = 453, /* BRMCR */
    MIB_CNT_BSS0_PF_DROP = 454, /* BPFCR */
    MIB_CNT_BSS1_PF_DROP = 455, /* BPFCR */
    MIB_CNT_BSS2_PF_DROP = 456, /* BPFCR */
    MIB_CNT_BSS3_PF_DROP = 457, /* BPFCR */
    MIB_CNT_RX_AC0 = 458, /* ARCR */
    MIB_CNT_RX_AC1 = 459, /* ARCR */
    MIB_CNT_RX_AC2 = 460, /* ARCR */
    MIB_CNT_RX_AC3 = 461, /* ARCR */
    MIB_CNT_BSS0_RTS_TX_CNT = 462, /* BSDR0 */
    MIB_CNT_BSS1_RTS_TX_CNT = 463, /* BSDR0 */
    MIB_CNT_BSS2_RTS_TX_CNT = 464, /* BSDR0 */
    MIB_CNT_BSS3_RTS_TX_CNT = 465, /* BSDR0 */
    MIB_CNT_BSS0_RTS_RETRY = 466, /* BSDR1 */
    MIB_CNT_BSS1_RTS_RETRY = 467, /* BSDR1 */
    MIB_CNT_BSS2_RTS_RETRY = 468, /* BSDR1 */
    MIB_CNT_BSS3_RTS_RETRY = 469, /* BSDR1 */
    MIB_CNT_BSS0_RX_IPV4 = 470, /* BRIPCR0 */
    MIB_CNT_BSS1_RX_IPV4 = 471, /* BRIPCR0 */
    MIB_CNT_BSS2_RX_IPV4 = 472, /* BRIPCR0 */
    MIB_CNT_BSS3_RX_IPV4 = 473, /* BRIPCR0 */
    MIB_CNT_BSS0_RX_IPV4_DROP = 474, /* BRIPCR1 */
    MIB_CNT_BSS1_RX_IPV4_DROP = 475, /* BRIPCR1 */
    MIB_CNT_BSS2_RX_IPV4_DROP = 476, /* BRIPCR1 */
    MIB_CNT_BSS3_RX_IPV4_DROP = 477, /* BRIPCR1 */
    MIB_CNT_BSS0_RX_ICMP4 = 478, /* BRIPCR2 */
    MIB_CNT_BSS1_RX_ICMP4 = 479, /* BRIPCR2 */
    MIB_CNT_BSS2_RX_ICMP4 = 480, /* BRIPCR2 */
    MIB_CNT_BSS3_RX_ICMP4 = 481, /* BRIPCR2 */
    MIB_CNT_BSS0_RX_IPV6 = 482, /* BRIPCR3 */
    MIB_CNT_BSS1_RX_IPV6 = 483, /* BRIPCR3 */
    MIB_CNT_BSS2_RX_IPV6 = 484, /* BRIPCR3 */
    MIB_CNT_BSS3_RX_IPV6 = 485, /* BRIPCR3 */
    MIB_CNT_BSS0_RX_IPV6_DROP = 486, /* BRIPCR4 */
    MIB_CNT_BSS1_RX_IPV6_DROP = 487, /* BRIPCR4 */
    MIB_CNT_BSS2_RX_IPV6_DROP = 488, /* BRIPCR4 */
    MIB_CNT_BSS3_RX_IPV6_DROP = 489, /* BRIPCR4 */
    RMAC_CNT_OBSS_AIRTIME,
	RMAC_CNT_NONWIFI_AIRTIME
};

#else
enum ENUM_MIB_COUNTER_T {
	MIB_CNT_RX_FCS_ERR = 0,
	MIB_CNT_RX_FIFO_OVERFLOW,
	MIB_CNT_RX_MPDU,
	MIB_CNT_CHANNEL_IDLE,
	MIB_CNT_VEC_DROP,
	MIB_CNT_DELIMITER_FAIL,
	MIB_CNT_VEC_MISMATCH,
	MIB_CNT_MDRDY,
	MIB_CNT_PF_DROP,
	MIB_CNT_LEN_MISMATCH,
	MIB_CNT_AMDPU_RX_COUNT,
	MIB_CNT_P_CCA_TIME = 11,
	MIB_CNT_S_CCA_TIME,
	MIB_CNT_CCA_NAV_TX_TIME = 14,
	MIB_CNT_BCN_TX = 17,
	MIB_CNT_ARB_RWP_NEED,
	MIB_CNT_ARB_RWP_FAIL,
	MIB_CNT_TX_BW_20MHZ,
	MIB_CNT_TX_BW_40MHZ,
	MIB_CNT_TX_BW_80MHZ,
	MIB_CNT_TX_BW_160MHZ,
	MIB_CNT_TX_AGG_RANGE,
	MIB_CNT_AMPDU_TX_COUNT,
	MIB_CNT_PHY_MIB_COUNTER0,
	MIB_CNT_PHY_MIB_COUNTER1,

	MIB_CNT_RX_CCK_MDRDY_TIME,
	MIB_CNT_P_ED_TIME,
	MIB_CNT_RX_TOTAL_BYTE,
	MIB_CNT_RX_VALID_AMPDU_SF,
	MIB_CNT_RX_VALID_BYTE,

	/* PerBSS MIB Counter*/
	MIB_CNT_BSS0_RTS_TX_CNT,
	MIB_CNT_BSS0_RTS_RETRY,
	MIB_CNT_BSS0_BA_MISS,
	MIB_CNT_BSS0_ACK_FAIL,
	MIB_CNT_BSS0_FRAME_RETRY,
	MIB_CNT_BSS0_FRAME_RETRY_2,
	MIB_CNT_BSS0_FRAME_RETRY_3,

	MIB_CNT_BSS1_RTS_TX_CNT,
	MIB_CNT_BSS1_RTS_RETRY,
	MIB_CNT_BSS1_BA_MISS,
	MIB_CNT_BSS1_ACK_FAIL,
	MIB_CNT_BSS1_FRAME_RETRY,
	MIB_CNT_BSS1_FRAME_RETRY_2,
	MIB_CNT_BSS1_FRAME_RETRY_3,

	MIB_CNT_BSS2_RTS_TX_CNT,
	MIB_CNT_BSS2_RTS_RETRY,
	MIB_CNT_BSS2_BA_MISS,
	MIB_CNT_BSS2_ACK_FAIL,
	MIB_CNT_BSS2_FRAME_RETRY,
	MIB_CNT_BSS2_FRAME_RETRY_2,
	MIB_CNT_BSS2_FRAME_RETRY_3,

	MIB_CNT_BSS3_RTS_TX_CNT,
	MIB_CNT_BSS3_RTS_RETRY,
	MIB_CNT_BSS3_BA_MISS,
	MIB_CNT_BSS3_ACK_FAIL,
	MIB_CNT_BSS3_FRAME_RETRY,
	MIB_CNT_BSS3_FRAME_RETRY_2,
	MIB_CNT_BSS3_FRAME_RETRY_3,

	MIB_CNT_RX_OFDM_LG_MIXED_MDRDY_TIME = 64,
	MIB_CNT_RX_OFDM_GREEN_MDRDY_TIME,

	MIB_CNT_BSS0_DATA_RETRY,
	MIB_CNT_BSS1_DATA_RETRY,
	MIB_CNT_BSS2_DATA_RETRY,
	MIB_CNT_BSS3_DATA_RETRY,
	MIB_CNT_DATA_RETRY,

	MIB_CNT_BSS0_CTRL_FRAME_CNT,
	MIB_CNT_BSS1_CTRL_FRAME_CNT,
	MIB_CNT_BSS2_CTRL_FRAME_CNT,
	MIB_CNT_BSS3_CTRL_FRAME_CNT,
	MIB_CNT_CTRL_FRAME_CNT,

	MIB_CNT_BSS0_MGMT_RETRY,
	MIB_CNT_BSS1_MGMT_RETRY,
	MIB_CNT_BSS2_MGMT_RETRY,
	MIB_CNT_BSS3_MGMT_RETRY,
	MIB_CNT_MGMT_RETRY,

	MIB_CNT_TX_DUR_CNT,
	MIB_CNT_RX_DUR_CNT,
	MIB_CNT_RX_OUT_OF_RANGE_COUNT,
	MIB_CNT_BA_CNT,
	MIB_CNT_MAC2PHY_TX_TIME,
	RMAC_CNT_OBSS_AIRTIME,
	RMAC_CNT_NONWIFI_AIRTIME
};
#endif /* MT7986 || MT7916 || MT7981 */
/*MacInfo ID: 0x04 EDCA*/
typedef struct _TX_AC_PARAM_T {
	UINT8 ucAcNum;
	UINT8 ucVaildBit;
	UINT8 ucAifs;
	UINT8 ucWinMin;
	UINT16 u2WinMax;
	UINT16 u2Txop;
} TX_AC_PARAM_T, *P_TX_AC_PARAM_T;

enum {
	EDCA_ACT_SET,
	EDCA_ACT_GET,
	EDCA_ACT_MAX
};

typedef struct GNU_PACKED _CMD_EDCA_SET_T {
	UINT8 ucTotalNum;
	UINT8 ucAction;
	UINT8 ucTxModeValid;
	UINT8 ucTxMode;
	TX_AC_PARAM_T rAcParam[HW_TX_QUE_NUM];
} CMD_EDCA_SET_T, CMD_EDCA_CTRL_T, *P_CMD_EDCA_CTRL_T, MT_EDCA_CTRL_T;

/* MacInfo ID: 0x05 Get wifi interrupt counter */
typedef struct _GET_WF_INTERRUPT_CNT_T {
	UINT8 ucWifiInterruptNum;
	UINT8 aucReserved[3];
	UINT32 u4WifiInterruptCounter[0];
} GET_WF_INTERRUPT_CNT_T, *P_WF_INTERRUPT_CNT_T;

typedef union {
	GET_CH_BUSY_CNT_T			ChBusyCntResult;
	TSF_RESULT_T				TsfResult;
	MT_PARTIAL_MIB_INFO_CNT_CTRL_T		PartialMibInfoCntResult;
	MT_EDCA_CTRL_T				EdcaResult;
	GET_WF_INTERRUPT_CNT_T			WifiIntCntResult;
} MAC_INFO_RESULT_T;

typedef struct GNU_PACKED _EXT_EVENT_MAC_INFO_T {
	UINT16 u2MacInfoId;
	UINT8 aucReserved[2];
	MAC_INFO_RESULT_T aucMacInfoResult;
} EXT_EVENT_MAC_INFO_T, *P_EXT_EVENT_MAC_INFO_T;

typedef enum _ENUM_MAC_ENABLE_CTRL_T {
	ENUM_MAC_DISABLE = 0,
	ENUM_MAC_ENABLE = 1,
	ENUM_MAC_DFS_TXSTART = 2,
	MAX_MAC_ENABLE_CTRL_NUM
} ENUM_MAC_ENABLE_CTRL_T, *P_ENUM_MAC_ENABLE_CTRL_T;

typedef struct GNU_PACKED _EXT_CMD_MAC_ENABLE_CTRL_T {
	UINT8         ucMacEnable;
	UINT8         ucBand;
	UINT8         aucReserve[2];
} EXT_CMD_MAC_ENABLE_CTRL_T, *P_EXT_CMD_MAC_ENABLE_CTRL_T;

typedef struct GNU_PACKED _EXT_CMD_RXV_ENABLE_CTRL_T {
	UINT8         ucBandIdx;
	UINT8         ucRxvEnable;
	UINT8         aucReserve[2];
} EXT_CMD_RXV_ENABLE_CTRL_T, *P_EXT_CMD_RXV_ENABLE_CTRL_T;

typedef struct GNU_PACKED _EXT_CMD_ID_BWF_LWC_ENABLE {
	UINT8		ucBwfLwcEnable; /* 0: Disable, 1: Enable */
	UINT8		aucReserve[3];
} EXT_CMD_ID_BWF_LWC_ENABLE_T, *P_EXT_CMD_ID_BWF_LWC_ENABLE_T;

#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_PROXY_ARP)
typedef struct GNU_PACKED  _MT_HOTSPOT_INFO_UPDATE {
	/* hs bss flag */
	UINT8			ucUpdateType;
	UINT8			ucHotspotBssFlags;
	UINT8			ucHotspotBssId;
	/* sta DSCP */
	UINT16			u2StaWcid;		/* #256STA */
	UINT8			ucStaQosMapFlagAndIdx;
	/* DSCP pool */
	UINT8			ucPoolID;
	UINT8			ucTableValid;
	UINT8			ucPoolDscpExceptionCount;
	UINT32			u4Ac;
	UINT16			au2PoolDscpRange[8];
	UINT16			au2PoolDscpException[21];
} MT_HOTSPOT_INFO_UPDATE_T, *PMT_HOTSPOT_INFO_UPDATE_T;

typedef struct GNU_PACKED  _EXT_CMD_ID_HOTSPOT_INFO_UPDATE {
	/* hs bss flag */
	UINT8			ucUpdateType;
	UINT8			ucHotspotBssFlags;
	UINT8			ucHotspotBssId;
	/* sta DSCP */
	UINT8			ucStaWcidL; /* #256STA - Low Byte */
	UINT8			ucStaQosMapFlagAndIdx;
	/* DSCP pool */
	UINT8			ucPoolID;
	UINT8			ucTableValid;
	UINT8			ucPoolDscpExceptionCount;
	UINT32          u4Ac;
	UINT16			au2PoolDscpRange[8];
	UINT16			au2PoolDscpException[21];
	UINT8			ucStaWcidHnVer; /* #256STA - High Byte and Version */
} EXT_CMD_ID_HOTSPOT_INFO_UPDATE_T, *P_EXT_CMD_ID_HOTSPOT_INFO_UPDATE_T;


#endif /* CONFIG_HOTSPOT_R2 */

#if defined(PRE_CAL_TRX_SET1_SUPPORT) || defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
#define RXDCOC_SIZE 256
#define TXDPD_SIZE 216
#endif

#ifdef PRE_CAL_TRX_SET1_SUPPORT
typedef struct _RXDCOC_RESULT_T {
	UINT16         u2ChFreq;/* Primary Channel Number */
	UINT8          ucBW;    /* 0: BW20, 1: BW40, 2: BW80, 3:BW160 */
	UINT8          ucBand;   /* 0: 2.4G, 1: 5G */
	BOOLEAN        bSecBW80; /* 0: primary BW80, 1: secondary BW80  - used only in BW160*/
	BOOLEAN        ResultSuccess;
	UINT8		   DBDCEnable;  /* 1: DBDC enable, 0: not in DBDC mode  */
	UINT8          ucReserved;

	/* WF0 SX0 */
	UINT32         ucDCOCTBL_I_WF0_SX0_LNA[4];
	UINT32         ucDCOCTBL_Q_WF0_SX0_LNA[4];
	/* WF0 SX2 */
	UINT32         ucDCOCTBL_I_WF0_SX2_LNA[4];
	UINT32         ucDCOCTBL_Q_WF0_SX2_LNA[4];
	/* WF1 SX0 */
	UINT32         ucDCOCTBL_I_WF1_SX0_LNA[4];
	UINT32         ucDCOCTBL_Q_WF1_SX0_LNA[4];
	/* WF1 SX2 */
	UINT32         ucDCOCTBL_I_WF1_SX2_LNA[4];
	UINT32         ucDCOCTBL_Q_WF1_SX2_LNA[4];
	/* WF2 SX0 */
	UINT32         ucDCOCTBL_I_WF2_SX0_LNA[4];
	UINT32         ucDCOCTBL_Q_WF2_SX0_LNA[4];
	/* WF2 SX2 */
	UINT32         ucDCOCTBL_I_WF2_SX2_LNA[4];
	UINT32         ucDCOCTBL_Q_WF2_SX2_LNA[4];
	/* WF3 SX0 */
	UINT32         ucDCOCTBL_I_WF3_SX0_LNA[4];
	UINT32         ucDCOCTBL_Q_WF3_SX0_LNA[4];
	/* WF3 SX2 */
	UINT32         ucDCOCTBL_I_WF3_SX2_LNA[4];
	UINT32         ucDCOCTBL_Q_WF3_SX2_LNA[4];
} RXDCOC_RESULT_T, *P_RXDCOC_RESULT_T;

typedef struct GNU_PACKED _EXT_CMD_GET_RXDCOC_RESULT_T {
	BOOLEAN             DirectionToCR;
	UINT8				ucDoRuntimeCalibration;
	UINT8				aucReserved[2];
	RXDCOC_RESULT_T     RxDCOCResult;
} EXT_CMD_GET_RXDCOC_RESULT_T, *P_EXT_CMD_GET_RXDCOC_RESULT_T;

typedef struct _TXDPD_RESULT_T {
	UINT16		u2ChFreq;/* Primary Channel Number */
	UINT8		ucBW;    /* 0: BW20, 1: BW40, 2: BW80, 3:BW160 */
	UINT8		ucBand;   /* 0: 2.4G, 1: 5G */
	BOOLEAN		bSecBW80; /* 0: primary BW80, 1: secondary BW80  - used only in BW160*/
	BOOLEAN		ResultSuccess;
	UINT8		DBDCEnable;  /* 1: DBDC enable, 0: not in DBDC mode  */
	UINT8		ucReserved;
	/* WF0 */
	UINT32		u4DPDG0_WF0_Prim;
	UINT8		ucDPDLUTEntry_WF0_B0_6[16];	 /* WF0 entry prim part I */
	UINT8		ucDPDLUTEntry_WF0_B16_23[16]; /* WF0 entry prim part II */
	/* WF1 */
	UINT32		u4DPDG0_WF1_Prim;
	UINT8		ucDPDLUTEntry_WF1_B0_6[16];	 /* WF1 entry prim part I */
	UINT8		ucDPDLUTEntry_WF1_B16_23[16]; /* WF1 entry prim part II */
	/* WF2 */
	UINT32		u4DPDG0_WF2_Prim;
	UINT32		u4DPDG0_WF2_Sec;
	UINT8		ucDPDLUTEntry_WF2_B0_6[16];		/* WF2 entry prim part I */
	UINT8		ucDPDLUTEntry_WF2_B16_23[16];	/* WF2 entry prim part II */
	UINT8		ucDPDLUTEntry_WF2_B8_14[16];		/* WF2 entry secondary part I */
	UINT8		ucDPDLUTEntry_WF2_B24_31[16];	/* WF2 entry secondary part II */
	/* WF3 */
	UINT32		u4DPDG0_WF3_Prim;
	UINT32		u4DPDG0_WF3_Sec;
	UINT8		ucDPDLUTEntry_WF3_B0_6[16];		/* WF3 entry prim part I */
	UINT8		ucDPDLUTEntry_WF3_B16_23[16];	/* WF3 entry prim part II */
	UINT8		ucDPDLUTEntry_WF3_B8_14[16];		/* WF3 entry secondary part I */
	UINT8		ucDPDLUTEntry_WF3_B24_31[16];	/* WF3 entry secondary part II */
} TXDPD_RESULT_T, *P_TXDPD_RESULT_T;

typedef struct GNU_PACKED _EXT_CMD_GET_TXDPD_RESULT_T {
	BOOLEAN			DirectionToCR;
	UINT8			ucDoRuntimeCalibration;
	UINT8           aucReserved[2];
	TXDPD_RESULT_T	TxDpdResult;

} EXT_CMD_GET_TXDPD_RESULT_T, *P_EXT_CMD_GET_TXDPD_RESULT_T;

typedef struct GNU_PACKED _EXT_CMD_RDCE_VERIFY_T {
	BOOLEAN		Result; /* 1 -success ,0 - fail */
	UINT8		ucType; /* 0 - RDCE without compensation , 1 - RDCE with compensation */
	UINT8		ucBW;    /* 0: BW20, 1: BW40, 2: BW80, 3:BW160 */
	UINT8		ucBand;   /* 0: 2.4G, 1: 5G */
} EXT_CMD_RDCE_VERIFY_T, *P_EXT_CMD_RDCE_VERIFY_T;

#endif /* PRE_CAL_TRX_SET1_SUPPORT */

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
typedef struct GNU_PACKED _EXT_CMD_GET_PRECAL_RESULT_T
{
    UINT16          u2PreCalBitMap;
    UINT8           ucCalId;
    UINT8           aucReserved;
} EXT_CMD_GET_PRECAL_RESULT_T, *P_EXT_CMD_GET_PRECAL_RESULT_T;

typedef enum _PRE_CAL_TYPE {
    PRECAL_TXLPF,
    PRECAL_TXIQ,
    PRECAL_TXDC,
    PRECAL_RXFI,
    PRECAL_RXFD
} PRE_CAL_TYPE;
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */


typedef struct GNU_PACKED _EXT_CMD_THERMAL_MODE_CTRL_T
{
    UINT8  ucMode;
    UINT8  ucAction;
    UINT8  aucReserved[2];
} EXT_CMD_THERMAL_MODE_CTRL_T, *P_EXT_CMD_THERMAL_MODE_CTRL_T;

#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
typedef struct GNU_PACKED _EXT_CMD_CAL_CTRL_T {
	UINT8  ucFuncIndex;
	UINT8  aucReserved[3];
	UINT32 u4DataLen;
} EXT_CMD_CAL_CTRL_T, *P_EXT_CMD_CAL_CTRL_T;

typedef struct GNU_PACKED _EXT_CMD_PA_TRIM_T {
	EXT_CMD_CAL_CTRL_T Header;
	UINT32 u4Data[4];
} EXT_CMD_PA_TRIM_T, *P_EXT_CMD_PA_TRIM_T;

typedef enum {
	CAL_RESTORE_PA_TRIM = 0x00
} CAL_RESTORE_FUNC_IDX;
#endif /* CAL_BIN_FILE_SUPPORT */

#ifdef ZERO_LOSS_CSA_SUPPORT
typedef struct _EXT_CMD_CHK_PEER_STA_LINK {
	UINT8      u1NumOfSta;
	UINT16      u2Wcid[3];
	UINT8	   u1Reserve;
} EXT_CMD_CHK_PEER_STA_LINK_T, *P_EXT_CMD_CHK_PEER_STA_LINK_T;

typedef struct _EXT_CMD_SET_ZERO_PKT_LOSS_VARIABLE {
	UINT_8		u1ZeroPktLossVariable;
	UINT_8		u1Value;
	UINT_8		u1Reserve[2];
} EXT_CMD_SET_ZERO_PKT_LOSS_VARIABLE_T, *P_EXT_CMD_SET_ZERO_PKT_LOSS_VARIABLE_T;
#endif /*ZERO_LOSS_CSA_SUPPORT*/

#ifdef CONFIG_3_WIRE_SUPPORT
typedef struct GNU_PACKED _EXT_CMD_SET_3WIRE_EXT_EN_T {
	UINT_16 u2Tag;
	UINT_16 u2Length;
	UINT_8 fg3WireExtEn; // 1: Enable 3Wire function 0: Disable 3Wire function
} EXT_CMD_SET_3WIRE_EXT_EN_T, *P_EXT_CMD_SET_3WIRE_EXT_EN_T;
#endif

enum {
	RXHDR_TRANS = 0,
	RXHDR_BL = 1,
};

typedef struct _EXT_RX_HEADER_TRANSLATE_T {
	UINT8	ucOperation;
	UINT8	ucEnable;
	UINT8	ucCheckBssid;
	UINT8	ucInsertVlan;
	UINT8	ucRemoveVlan;
	UINT8	ucUserQosTid;
	UINT8	ucTranslationMode;
	UINT8	ucReserve;
} EXT_RX_HEADER_TRANSLATE_T, *P_EXT_RX_HEADER_TRANSLATE_T;

typedef struct _EXT_RX_HEADER_TRANSLATE_BL_T {
	UINT8	ucOperation;
	UINT8	ucCount;
	UINT8	ucReserv[2];
	UINT8	ucBlackListIndex;
	UINT8	ucEnable;
	UINT16	usEtherType;
} EXT_RX_HEADER_TRANSLATE_BL_T, *P_EXT_RX_HEADER_TRANSLATE_BL_T;

typedef struct _CMD_PATCH_FINISH_T {
	UINT8	ucCheckCrc;
	UINT8	aucReserved[3];
} CMD_PATCH_FINISH_T, *P_CMD_PATCH_FINISH_T;

#define MAX_BCTRL_ENTRY 64

#ifdef DBDC_MODE

typedef struct _BCTRL_ENTRY {
	UINT8 Type;
	UINT8 Index;
	UINT8 BandIdx;
} BCTRL_ENTRY_T;

typedef struct  _BCTRL_INFO_T {
	BOOLEAN DBDCEnable;
	UINT32 TotalNum;
	BCTRL_ENTRY_T BctrlEntries[MAX_BCTRL_ENTRY];
} BCTRL_INFO_T;

#endif /*DBDC_MODE*/

typedef struct _STA_REC_BA_CFG {
	UCHAR MuarIdx;
	UCHAR BssIdx;
	UINT16 WlanIdx;
	UCHAR tid;
	UCHAR baDirection;
	BOOLEAN BaEnable;
	UINT16 sn;
	UINT16 ba_wsize;
	UCHAR amsdu;
} STA_REC_BA_CFG_T;

typedef struct _STA_REC_CFG {
	UINT8 ucBssIndex;
	UINT16 u2WlanIdx;
#ifdef SW_CONNECT_SUPPORT
	/* u2WlanIdx means the HW wcid before fill send in-band cmd */
	UINT16 u2SwWlanIdx;
#endif /* SW_CONNECT_SUPPORT */
	UINT8 ConnectionState;
	UINT8 MuarIdx;
	UINT32 ConnectionType;
	UINT32 u4EnableFeature;
	UINT8 IsNewSTARec;
	ASIC_SEC_INFO asic_sec_info;
	struct _MAC_TABLE_ENTRY *pEntry;
	struct _STAREC_AUTO_RATE_UPDATE_T *pRaParam;
#ifdef DOT11_HE_AX
	struct he_sta_info he_sta;
#endif /*DOT11_HE_AX*/
} STA_REC_CFG_T;

typedef struct _EXT_CMD_CFG_BASIC_INFO_T {
	/*fixed field*/
	UINT8 ucDbdcIdx;
	UINT8 aucReserved[3];

	/* tlv */
	UINT32 u4TotalTlvNum;
	UINT8 aucTlvBuffer[0]; /**< the TLVs included in this field: **/

} EXT_CMD_CFG_BASIC_INFO_T, *P_EXT_CMD_CFG_BASIC_INFO_T;

typedef struct GNU_PACKED _EXT_CMD_CFG_HOSTREPORT_UPDATE_T {
	/* HostReport information (Tag0) */
	UINT16	u2Tag;		/* Tag = 0x00 */
	UINT16	u2Length;
	UINT8	ucActive;
	UINT8	aucReserved[3];
} EXT_CMD_CFG_HOSTREPORT_UPDATE_T, *P_EXT_CMD_CFG_HOSTREPORT_UPDATE_T;

#define CFGINFO_DROP_RTS_CTRL_FRAME (1 << 0)
#define CFGINFO_DROP_CTS_CTRL_FRAME (1 << 1)
#define CFGINFO_DROP_UNWANTED_CTRL_FRAME (1 << 2)

typedef struct GNU_PACKED _EXT_CMD_CFG_DROP_CTRL_FRAME_T {
	/* RX Filter Drop Ctrl Frame*/
	UINT16	u2Tag;
	UINT16	u2Length;
	UINT8	ucDropRts;
	UINT8	ucDropCts;
	UINT8	ucDropUnwantedCtrl;
	UINT8	aucReserved[1];
} EXT_CMD_CFG_DROP_CTRL_FRAME_T, *P_EXT_CMD_CFG_DROP_CTRL_FRAME_T;

typedef struct GNU_PACKED _EXT_CMD_CFG_SET_AGG_AC_LIMIT_T {
	UINT16	u2Tag;
	UINT16	u2Length;
	UINT8	ucWmmIdx;
	UINT8	ucAc;
	UINT8	ucAggLimit;
} EXT_CMD_CFG_SET_AGG_AC_LIMIT_T, *P_EXT_CMD_CFG_SET_AGG_AC_LIMIT_T;

typedef struct GNU_PACKED _EXT_CMD_CFG_CERT_CFG_T {
	UINT16	u2Tag;
	UINT16	u2Length;
	UINT8	ucCertProgram;
	UINT8	aucReserved[3];
} EXT_CMD_CFG_CERT_CFG_T, *P_EXT_CMD_CFG_CERT_CFG_T;

typedef struct GNU_PACKED _EXT_CMD_CFG_POWER_BACKOFF_T {
	UINT16	u2Tag;
	UINT16	u2Length;
	INT8	i1PowerBackoff;
} EXT_CMD_CFG_POWER_BACKOFF_T, *P_EXT_CMD_CFG_POWER_BACKOFF_T;

typedef struct GNU_PACKED _EXT_CMD_CFG_SET_ACK_CTS_T {
	UINT16	u2Tag;
	UINT16	u2Length;
	UINT32	u4TimeoutValue; /* unit: 1us */
	UINT8	u1Type;
	/* 0: CCK DCF Timeout, 1: OFDM DCF Timeout, 2: OFDMA-MU DCF Timeout */
	UINT8	aucReserved[3];
} EXT_CMD_CFG_SET_ACK_CTS_T, *P_EXT_CMD_CFG_SET_ACK_CTS_T;

typedef struct _EXT_CMD_CFG_SET_RTS_SIGTA_EN_T {
	UINT16  u2Tag;
	UINT16  u2Length;
	BOOLEAN Enable; /* 0: Disable, 1: Enable */
	UINT8   aucReserve[3];
} EXT_CMD_CFG_SET_RTS_SIGTA_EN_T, *P_EXT_CMD_CFG_SET_RTS_SIGTA_EN_T;

typedef struct _EXT_CMD_CFG_SET_SCH_DET_DIS_T {
	UINT16  u2Tag;
	UINT16  u2Length;
	BOOLEAN Disable;   /* 0: Enable, 1: Disable */
	UINT8   aucReserve[3];
} EXT_CMD_CFG_SET_SCH_DET_DIS_T, *P_EXT_CMD_CFG_SET_SCH_DET_DIS_T;

typedef struct _EXT_CMD_CFG_SET_RTS0_PKT_THRESHOLD_CFG_T {
	UINT16  u2Tag;
	UINT16  u2Length;
	BOOLEAN Enable;   /* 0: Disable, 1: Enable */
	UINT8   ucType;   /* 0: Len, 1: Number */
	UINT32  u4Value;
	UINT8   aucReserve[3];
} EXT_CMD_CFG_SET_RTS0_PKT_THRESHOLD_CFG_T, *P_EXT_CMD_CFG_SET_RTS0_PKT_THRESHOLD_CFG_T;

typedef struct GNU_PACKED _EXT_CMD_CFG_GET_ACK_CTS_T {
	UINT8	u1Type;
	/* 0: CCK DCF Timeout, 1: OFDM DCF Timeout, 2: OFDMA-MU DCF Timeout */
	UINT8   ucDbdcIdx;
	UINT8	aucReserved[2];
	UINT32	u4TimeoutValue; /* unit: 1us */
} EXT_CMD_CFG_GET_ACK_CTS_T, *P_EXT_CMD_CFG_GET_ACK_CTS_T;

typedef enum _EXT_CMD_CFGINFO_TAG_T {
	EXT_CMD_CFGINFO_HOSTREPORT_TX_LATENCY = 0,
	EXT_CMD_CFGINFO_CHECKSUM,
	EXT_CMD_CFGINFO_RX_FILTER_DROP_CTRL_FRAME,
	EXT_CMD_CFGINFO_AGG_AC_LIMIT,
	EXT_CMD_CFGINFO_CERT_CFG,
	EXT_CMD_CFGINFO_POWER_BACKOFF,
	EXT_CMD_CFGINFO_ACK_CTS,
	EXT_CMD_CFGINFO_RTS_SIGTA_EN,
	EXT_CMD_CFGINFO_SCH_DET_DIS,
	EXT_CMD_CFGINFO_RTS0_PKT_THRESHOLD_CFG,
#ifdef CONFIG_3_WIRE_SUPPORT
	EXT_CMD_CFGINFO_3WIRE_EXT_EN_CFG,
#endif
	EXT_CMD_CFG_MAX_NUM,
} EXT_CMD_CFGINFO_TAG_T;

typedef enum _ENUM_CFG_FEATURE {
	CFGINFO_HOSTREPORT_TXLATENCY_FEATURE = (1 << EXT_CMD_CFGINFO_HOSTREPORT_TX_LATENCY),
	CFGINFO_CHECKSUM = (1 << EXT_CMD_CFGINFO_CHECKSUM),
	CFGINFO_RX_FILTER_DROP_CTRL_FRAME_FEATURE = (1 << EXT_CMD_CFGINFO_RX_FILTER_DROP_CTRL_FRAME),
	CFGINFO_AGG_AC_LIMT_FEATURE = (1 << EXT_CMD_CFGINFO_AGG_AC_LIMIT),
	CFGINFO_CERT_CFG_FEATURE = (1 << EXT_CMD_CFGINFO_CERT_CFG),
	CFGINFO_POWER_BACKOFF_FEATURE = (1 << EXT_CMD_CFGINFO_POWER_BACKOFF),
	CFGINFO_ACK_CTS_FEATURE = (1 << EXT_CMD_CFGINFO_ACK_CTS),
	CFGINFO_RTS_SIGTA_EN_FEATURE = (1 << EXT_CMD_CFGINFO_RTS_SIGTA_EN),
	CFGINFO_SCH_DET_DIS_FEATURE = (1 << EXT_CMD_CFGINFO_SCH_DET_DIS),
	CFGINFO_RTS0_PKT_THRESHOLD_CFG_FEATURE = (1 << EXT_CMD_CFGINFO_RTS0_PKT_THRESHOLD_CFG),
#ifdef CONFIG_3_WIRE_SUPPORT
	CFGINFO_3WIRE_EXT_EN_CFG_FEATURE = (1 << EXT_CMD_CFGINFO_3WIRE_EXT_EN_CFG),
#endif
} ENUM_CFG_FEATURE, *P_ENUM_CFG_FEATURE;

UINT16 GetRealPortQueueID(struct cmd_msg *msg, UINT8 cmd_type);

#ifdef CONFIG_ATE
typedef struct _ATE_TXPOWER {
	UINT32 Ant_idx;
	UINT32 Power;
	UINT32 Channel;
	UINT32 Dbdc_idx;
	UINT32 Band_idx;
} ATE_TXPOWER;

#if OFF_CH_SCAN_SUPPORT
typedef struct _ATE_OFF_CH_SCAN {
	UINT32 ext_id;			/* ExtendId of command */
	UINT32 dbdc_idx;		/* DBDC index */
	UINT32 mntr_ch;			/* Monitoring channel */
	UINT32 is_aband;		/* 0: 2.4G channel, 1: 5G channel */
	UINT32 mntr_bw;			/* Bandwidth of monitoring channel */
	UINT32 mntr_tx_rx_pth;	/* Monitoring TX/RX stream path */
	UINT32 scan_mode;		/* ScanStart/ScanRunning/ScanStop */
} ATE_OFF_CH_SCAN, *P_ATE_OFF_CH_SCAN;
#endif

#endif /* CONFIG_ATE */

typedef struct _EXT_CMD_ID_AUTO_BA {
	UINT8 ucAutoBaEnable; /* 0: No reload, 1: do reload */
	UINT8 ucTarget;
	UINT8 aucReserve[2];
	UINT32 u4Timeout; /* timeout value, unit ms. */
} EXT_CMD_ID_AUTO_BA_T, *P_EXT_CMD_ID_AUTO_BA_T;

typedef struct GNU_PACKED _CMD_BA_TRIGGER_EVENT_T {
	UINT8 ucWlanIdxL;		/* #256STA - Low Byte */
	UINT8 ucTid;
	UINT8 ucWlanIdxHnVer;	/* #256STA - High Byte and Version */
	UINT8 aucReserved;
} CMD_BA_TRIGGER_EVENT_T, *P_CMD_BA_TRIGGER_EVENT_T;

#define CR4_GET_BSS_ACQ_PKT_NUM_READ_CLEAR_EN		BIT(31)
#define CR4_GET_BSS_ACQ_PKT_NUM_BSS_GROUP_DEFAULT	0x00FFFFFF
#define CR4_GET_BSS_ACQ_PKT_NUM_CMD_DEFAULT			(CR4_GET_BSS_ACQ_PKT_NUM_READ_CLEAR_EN | CR4_GET_BSS_ACQ_PKT_NUM_BSS_GROUP_DEFAULT)

#define CR4_CFG_BSS_NUM		24
#define CR4_NUM_OF_WMM_AC	4
typedef struct _EVENT_PER_BSS_ACQ_PKT_NUM_T {
	UINT32	au4AcqPktCnt[CR4_NUM_OF_WMM_AC];
} EVENT_PER_BSS_ACQ_PKT_NUM_T, *P_EVENT_PER_BSS_ACQ_PKT_NUM_T;

typedef struct _EVENT_BSS_ACQ_PKT_NUM_T {
	UINT32				u4BssMap;
	EVENT_PER_BSS_ACQ_PKT_NUM_T	bssPktInfo[CR4_CFG_BSS_NUM];
} EVENT_BSS_ACQ_PKT_NUM_T, *P_EVENT_BSS_ACQ_PKT_NUM_T;

typedef struct _CMD_PKT_REPROCESS_EVENT_T {
	/* MSDU token ID */
	UINT16         u2MsduToken;
} CMD_PKT_REPROCESS_EVENT_T, *P_CMD_PKT_REPROCESS_EVENT_T;

typedef struct _CMD_GET_CR4_HOTSPOT_CAPABILITY_T {
	UINT8         ucHotspotBssFlags[CR4_CFG_BSS_NUM];
} CMD_GET_CR4_HOTSPOT_CAPABILITY_T, *P_CMD_GET_CR4_HOTSPOT_CAPABILITY_T;

typedef struct _EXT_EVENT_GET_CR4_TX_STATISTICS_T {
	UINT16 wlan_index;
	UINT8 reserved[2];
	UINT32 one_sec_tx_bytes;
	UINT32 one_sec_tx_cnts;
} EXT_EVENT_GET_CR4_TX_STATISTICS_T, *P_EXT_EVENT_GET_CR4_TX_STATISTICS_T;

typedef struct _EXT_EVENT_GET_CR4_MULTI_TX_STATISTICS_T {
	UINT16 number;
	UINT8 reserved[2];
	EXT_EVENT_GET_CR4_TX_STATISTICS_T stat_list[0];
} EXT_EVENT_GET_CR4_MULTI_TX_STATISTICS_T, *P_EXT_EVENT_GET_CR4_MULTI_TX_STATISTICS_T;

typedef struct GNU_PACKED _EXT_EVENT_CSA_NOTIFY_T {
	UINT8 ucOwnMacIdx;
	UINT8 ucChannelSwitchCount;
	UINT8 ucBandIdx;
	UINT8 aucReserved;
} EXT_EVENT_CSA_NOTIFY_T, *P_EXT_EVENT_CSA_NOTIFY_T;

#ifdef ZERO_LOSS_CSA_SUPPORT
typedef struct EXT_EVENT_NULL_ACK_WCID {
	UINT_16 u2NullAckWcid;
	UINT_8 u1Resrved[2];
} EXT_EVENT_NULL_ACK_WCID_T, *P_EXT_EVENT_NULL_ACK_WCID_T;

typedef struct GNU_PACKED _EXT_EVENT_BCN_TX_NOTIFY_T {
	UINT8 ucBandIdx;
	UINT8 ucChannelSwitchCount;
	UINT8 ucOwnMacIdx;
	UINT8 aucReserved[1];
} EXT_EVENT_BCN_TX_NOTIFY_T, *P_EXT_EVENT_BCN_TX_NOTIFY_T;
#endif /*ZERO_LOSS_CSA_SUPPORT*/

typedef struct GNU_PACKED _EXT_EVENT_BCC_NOTIFY_T {
	UINT8 ucBandIdx;
	UINT8 ucOwnMacIdx;
	UINT8 ucColorSwitchCount;
	UINT8 aucReserved[1];
} EXT_EVENT_BCC_NOTIFY_T, *P_EXT_EVENT_BCC_NOTIFY_T;

typedef struct GNU_PACKED _EXT_EVENT_TMR_CALCU_INFO_T {
	UINT8 aucTmrFrm[36];
	INT16 i2LtfStartAddr;
	UINT16 u2OfdmCoarseTimeMSB;
	UINT32 u4OfdmCoarseTimeLSB;
	INT16 i2MinTFineTime;
	UINT8 ucChBw;
	UINT8 aucResv[1];
	UINT32 u4TOAECalibrationResult;
} EXT_EVENT_TMR_CALCU_INFO_T, *P_EXT_EVENT_TMR_CALCU_INFO_T;

typedef struct GNU_PACKED _EXT_CMD_ID_MCAST_CLONE {
	UINT8 ucMcastCloneEnable; /* 0: Disable, 1: Enable */
	UINT8 uc_omac_idx;
	UINT8 uc_band_idx;
	UINT8 ucReserve;
} EXT_CMD_ID_MCAST_CLONE_T, *P_EXT_CMD_ID_MCAST_CLONE_T;

typedef struct GNU_PACKED _EXT_CMD_ID_MCAST_POLICY {
	UINT8 uIgmpType;
	UINT8 uMcastPolicy;
	UINT16 u2Reserve;
} EXT_CMD_ID_MCAST_POLICY_T, *P_EXT_CMD_ID_MCAST_POLICY_T;


typedef struct GNU_PACKED _EXT_CMD_ID_MULTICAST_ENTRY_INSERT {
	UINT8 aucGroupId[IPV6_ADDR_LEN];
	UINT8 ucBssInfoIdx;
	UINT8 ucMcastEntryType; /* 0: STATIC, 1: DYNAMIC */
	UINT8 ucMemberNum; /* 0: no member. 1: one member, 2: */
	UINT8 aucReserve[3];
	UINT8 aucMemberAddr[6];
	UINT16 u2Wcid;
} EXT_CMD_ID_MULTICAST_ENTRY_INSERT_T, *P_EXT_CMD_ID_MULTICAST_ENTRY_INSERT_T;

typedef struct GNU_PACKED _EXT_CMD_ID_MULTICAST_ENTRY_DELETE {
	UINT8 aucGroupId[IPV6_ADDR_LEN];
	UINT8 ucBssInfoIdx;
	UINT8 ucMemberNum; /* 0: no member. 1: one member, 2: */
	UINT8 aucMemberAddr[6];
	UINT16 u2Wcid;
} EXT_CMD_ID_MULTICAST_ENTRY_DELETE_T, *P_EXT_CMD_ID_MULTICAST_ENTRY_DELETE_T;

typedef struct GNU_PACKED _EXT_CMD_ID_IGMP_FLOODING_CMD {
	UINT8 bInsert;
	UINT8 uEntryIPType;
	UINT8 auMacData[6];
	UINT32 auPrefixMask[4];
} EXT_CMD_ID_IGMP_FLOODING_CMD_T, *P_EXT_CMD_ID_IGMP_FLOODING_CMD_T;

#ifdef IGMP_TVM_SUPPORT
typedef enum {
	IGMP_MCAST_SET_AGEOUT_TIME = 0x01,
	IGMP_MCAST_GET_ENTRY_TABLE = 0x02,
	IGMP_MCAST_MAX_ID_INVALID = 0xFF,
} IGMP_MCAST_SET_GET_CMD_TYPE;

typedef struct GNU_PACKED _EXT_CMD_ID_MULTICAST_SET_GET {
	UINT8 ucCmdType;
	UINT8 ucOwnMacIdx;
	UINT8 Rsvd[2];
	union {
		UINT32 u4AgeOutTime;
	} SetData;
} EXT_CMD_ID_IGMP_MULTICAST_SET_GET_T, *P_EXT_CMD_ID_IGMP_MULTICAST_SET_GET_T;
#endif /* IGMP_TVM_SUPPORT */


/* Manually setting Tx power */
typedef struct _CMD_All_POWER_MANUAL_CTRL_T {
	UINT8   ucPowerManualCtrlFormatId;
	BOOLEAN fgPwrManCtrl;
	UINT8   u1TxPwrModeManual;
	UINT8   u1TxPwrBwManual;
	UINT8   u1TxPwrRateManual;
	INT8	i1TxPwrValueManual;
	UCHAR   ucBandIdx;
} CMD_All_POWER_MANUAL_CTRL_T, *P_CMD_All_POWER_MANUAL_CTRL_T;

typedef enum _CMD_TPC_ALGO_CATEGORY {
	TPC_ACT_MANUAL_MODE = 0x0,
	TPC_ACT_UL_TX_POWER_CONFIG = 0x1,
	TPC_ACT_UL_TARGET_RSSI_CONFIG = 0x2,
	TPC_ACT_UL_UPH_MIN_PWR_FG_CONFIG  = 0x3,
	TPC_ACT_DL_TX_POWER_CMD_CTRL_CONFIG = 0x4,
	TPC_ACT_DL_TX_POWER_CONFIG = 0x5,
	TPC_ACT_DL_TX_POWER_ALPHA_CONFIG = 0x6,
	TPC_ACT_MAN_TBL_INFO = 0x7,
	TPC_ACT_WLANID_CTRL = 0x8,
	TPC_ACT_UL_UNIT_TEST_CONFIG = 0x9,
	TPC_ACT_UL_UNIT_TEST_GO = 0xA,
	TPC_ACT_ENABLE_CFG = 0xB,
	TPC_ACT_NUM
} CMD_TPC_ALGO_CATEGORY, *P_CMD_TPC_ALGO_CATEGORY;

typedef enum _ENUM_TPC_EVENT_TYPE {
	TPC_EVENT_DOWNLINK_TABLE = 0,
	TPC_EVENT_UPLINK_TABLE = 1,
	TPC_EVENT_TYPE_NUM
} ENUM_TPC_EVENT_TYPE, *P_ENUM_TPC_EVENT_TYPE;

typedef struct _TPC_UL_STA_COMM_INFO {
	UINT8 u1TargetRssi;
	UINT8 u1PwrHeadRoom;
	BOOLEAN fgMinPwr;
} TPC_UL_STA_COMM_INFO, *P_TPC_UL_STA_COMM_INFO;

typedef struct _TPC_UL_MAN_MODE_PARAM_ELEMENT {
	UINT16 u2WlanId;
	TPC_UL_STA_COMM_INFO rTpcUlStaCmmInfo;
} TPC_UL_MAN_MODE_PARAM_ELEMENT, *P_TPC_UL_MAN_MODE_PARAM_ELEMENT;

/* structure of tpc table event */
typedef struct _EXT_EVENT_TPC_INFO_UPLINK_TABLE_T {
	UINT8  u1TpcCategory;
	/*AP Info*/
	UINT8  u1ApTxPwr;
	UINT8  u1Reserved[2];
	TPC_UL_MAN_MODE_PARAM_ELEMENT rTpcUlManModeParamElem[32];
} EXT_EVENT_TPC_INFO_UPLINK_TABLE_T, *P_EXT_EVENT_TPC_INFO_UPLINK_TABLE_T;

/** enum for down-link tx type */
typedef enum _ENUM_TPC_DL_TX_TYPE {
	TPC_DL_TX_TYPE_MU_MIMO = 0,
	TPC_DL_TX_TYPE_MU_OFDMA,
	TPC_DL_TX_TYPE_NUM
} ENUM_TPC_DL_TX_TYPE, *P_ENUM_TPC_DL_TX_TYPE;


typedef struct _TPC_DL_MAN_MODE_PARAM_ELEMENT {
	UINT16 u2WlanId;
	UINT8 u1Reserved[2];
	INT16 DlTxPwrAlpha[TPC_DL_TX_TYPE_NUM];
} TPC_DL_MAN_MODE_PARAM_ELEMENT, *P_TPC_DL_MAN_MODE_PARAM_ELEMENT;

/** enum for tpc parameter mode */
typedef enum _ENUM_TPC_PARAM_MODE {
	TPC_PARAM_AUTO_MODE = 0,
	TPC_PARAM_MAN_MODE,
	TPC_PARAM_MODE_NUM
} ENUM_TPC_PARAM_MODE, *P_ENUM_TPC_PARAM_MODE;

typedef struct _EXT_EVENT_TPC_INFO_DOWNLINK_TABLE_T {
	UINT8  u1TpcCategory;
	/*AP Info*/
	BOOLEAN fgCmdPwrCtrl[TPC_DL_TX_TYPE_NUM];
	CHAR DlTxPwr[TPC_DL_TX_TYPE_NUM];
	UINT8  u1Reserved[3];
	TPC_DL_MAN_MODE_PARAM_ELEMENT rTpcDlManModeParamElem[32];
} EXT_EVENT_TPC_INFO_DOWNLINK_TABLE_T, *P_EXT_EVENT_TPC_INFO_DOWNLINK_TABLE_T;

typedef struct _CMD_TPC_MAN_CTRL_T {
	UINT8 u1TpcCtrlFormatId;
	BOOLEAN fgTpcEnable;
	UINT8 u1Reserved[2];
	ENUM_TPC_PARAM_MODE u1TpcManual;
} CMD_TPC_MAN_CTRL_T, *P_CMD_TPC_MAN_CTRL_T;

typedef struct _CMD_TPC_MAN_WLAN_ID_CTRL_T {
	UINT8 u1TpcCtrlFormatId;
	UINT8 u1EntryIdx;
	UINT16 u2WlanId;
	BOOLEAN fgUplink;
	ENUM_TPC_DL_TX_TYPE u1DlTxType;
	UINT8 u1Reserved[2];
} CMD_TPC_MAN_WLAN_ID_CTRL_T, *P_CMD_TPC_MAN_WLAN_ID_CTRL_T;

typedef struct _CMD_TPC_UL_ALGO_CTRL_T {
	UINT8 u1TpcCtrlFormatId;
	UINT8 u1ApTxPwr;
	UINT8 u1EntryIdx;
	UINT8 u1TargetRssi;
	UINT8 u1UPH;
	BOOLEAN fgMinPwrFlag;
	UINT8 u1Reserved[2];
} CMD_TPC_UL_ALGO_CTRL_T, *P_CMD_TPC_UL_ALGO_CTRL_T;

typedef struct _CMD_TPC_DL_ALGO_CTRL_T {
	UINT8 u1TpcCtrlFormatId;
	CHAR DlTxPwr;
	BOOLEAN fgDlTxPwrCmdCtrl;
	UINT8 u1EntryIdx;
	INT16 DlTxPwrAlpha;
	ENUM_TPC_DL_TX_TYPE u1DlTxType;
	UINT8 u1Reserved;
} CMD_TPC_DL_ALGO_CTRL_T, *P_CMD_TPC_DL_ALGO_CTRL_T;

typedef struct _CMD_TPC_MAN_TBL_INFO_T {
	UINT8 u1TpcCtrlFormatId;
	BOOLEAN fgUplink;
	UINT8 u1Reserved[2];
} CMD_TPC_MAN_TBL_INFO_T, *P_CMD_TPC_MAN_TBL_INFO_T;

typedef struct _CMD_TPC_UL_UT_VAR_CFG_T {
	UINT8 u1TpcCtrlFormatId;
	UINT8 u1EntryIdx;
	UINT8 u1VarType;
	UINT8 u1Reserved;
	INT16 i2Value;
	UINT8 u1Reserved2[2];
} CMD_TPC_UL_UT_VAR_CFG_T, *P_CMD_TPC_UL_UT_VAR_CFG_T;

typedef struct _CMD_TPC_UL_UT_CTRL_T {
	UINT8 u1TpcCtrlFormatId;
	BOOLEAN fgTpcUtGo;
	UINT8 u1Reserved[2];
} CMD_TPC_UL_UT_CTRL_T, *P_CMD_TPC_UL_UT_CTRL_T;

typedef struct _CMD_RA_SUPPORT_MCS_CAP_CTRL_T {
	UINT8 rate_ctrl_format_id;
	UINT8 tx_mode;
	UINT8 tx_nss;
	UINT8 tx_bw;
	UINT16 mcs_cap;
	BOOLEAN set;
	UINT8 reserved;
} CMD_RA_SUPPORT_MCS_CAP_CTRL_T, *P_CMD_RA_SUPPORT_MCS_CAP_CTRL_T;

typedef struct _CMD_RA_DBG_CTRL_T {
	UINT8 rate_ctrl_format_id;
	UINT8 param_num;
	UINT8 reserved[2];
	UINT32 param[20];
} CMD_RA_DBG_CTRL_T, *P_CMD_RA_DBG_CTRL_T;

typedef struct _CMD_POWER_SKU_CTRL_T {
	UINT8  ucPowerCtrlFormatId;
	UCHAR  ucSKUEnable;
	UCHAR  ucBandIdx;
	UINT8  ucReserved;
} CMD_POWER_SKU_CTRL_T, *P_CMD_POWER_SKU_CTRL_T;

typedef struct _CMD_POWER_PERCENTAGE_CTRL_T {
	UINT8  ucPowerCtrlFormatId;
	UCHAR  ucPercentageEnable;
	UCHAR  ucBandIdx;
	UINT8  ucReserved;
} CMD_POWER_PERCENTAGE_CTRL_T, *P_CMD_POWER_PERCENTAGE_CTRL_T;

typedef struct _CMD_POWER_PERCENTAGE_DROP_CTRL_T {
	UINT8  ucPowerCtrlFormatId;
	INT8   cPowerDropLevel;
	UINT8  ucBandIdx;
	UINT8  ucReserved;
} CMD_POWER_PERCENTAGE_DROP_CTRL_T, *P_CMD_POWER_PERCENTAGE_DROP_CTRL_T;

typedef struct _CMD_TX_CCK_STREAM_CTRL_T {
	UINT8  u1CCKTxStream;
	UINT8  ucBandIdx;
	UINT8  ucReserved[2];
} CMD_TX_CCK_STREAM_CTRL_T, *P_CMD_TX_CCK_STREAM_CTRL_T;

typedef struct _CMD_POWER_BF_BACKOFF_CTRL_T {
	UINT8  ucPowerCtrlFormatId;
	UCHAR  ucBFBackoffEnable;
	UCHAR  ucBandIdx;
	UINT8  ucReserved;
} CMD_POWER_BF_BACKOFF_CTRL_T, *P_CMD_POWER_BF_BACKOFF_CTRL_T;

typedef struct _CMD_POWER_THERMAL_COMP_CTRL_T {
	UINT8	ucPowerCtrlFormatId;
	BOOLEAN  fgThermalCompEn;
	UINT8	ucBandIdx;
	UINT8	ucReserved;
} CMD_POWER_THERMAL_COMP_CTRL_T, *P_CMD_POWER_THERMAL_COMP_CTRL_T;

typedef struct _CMD_POWER_RF_TXANT_CTRL_T {
	UINT8  ucPowerCtrlFormatId;
	UINT8  ucTxAntIdx;		  /* bitwise representation. 0x5 means only TX0, TX2 enabled */
	UINT8  ucReserved[2];
} CMD_POWER_RF_TXANT_CTRL_T, *P_CMD_POWER_RF_TXANT_CTRL_T;

typedef struct _CMD_TX_POWER_SHOW_INFO_T {
	UINT8    ucPowerCtrlFormatId;
	BOOLEAN  ucTxPowerInfoCatg;
	UINT8    ucBandIdx;
	UINT8    ucReserved;
} CMD_TX_POWER_SHOW_INFO_T, *P_CMD_TX_POWER_SHOW_INFO_T;

typedef struct _CMD_TOAE_ON_OFF_CTRL {
	BOOLEAN fgTOAEEnable;
	UINT8   aucReserve[3];
} CMD_TOAE_ON_OFF_CTRL, *P_CMD_TOAE_ON_OFF_CTRL;

#ifdef WIFI_EAP_FEATURE
typedef enum _EAP_FEATURE_CATEGORY {
	INIT_IPI_CTRL = 0x0,
	GET_IPI_VALUE = 0x1,
	SET_DATA_TXPWR_OFFSET = 0x2,
	SET_RA_TABLE_DATA = 0x3,
	GET_RATE_INFO = 0x4,
	EAP_FEATURE_NUM
} EAP_FEATURE_CATEGORY, *P_EAP_FEATURE_CATEGORY;

enum {
	EAP_EVENT_IPI_VALUE,
	EAP_EVENT_SHOW_RATE_TABLE,
	EAP_EVENT_NUM,
};

#define EAP_FW_RA_SWITCH_TBL_PATH            "/etc/FwRASwitchTbl.dat"
#define EAP_FW_RA_HW_FB_TBL_PATH             "/etc/FwRAHwFbTbl.dat"

#define EAP_FW_RA_SWITCH_TBL_UPD_PATH_7615   "/etc/FwRASwitchTblUpd7615.dat"
#define EAP_FW_RA_HW_FB_TBL_UPD_PATH_7615    "/etc/FwRAHwFbTblUpd7615.dat"
#define EAP_FW_RA_SWITCH_TBL_UPD_PATH_7622   "/etc/FwRASwitchTblUpd7622.dat"
#define EAP_FW_RA_HW_FB_TBL_UPD_PATH_7622    "/etc/FwRAHwFbTblUpd7622.dat"
#define EAP_FW_RA_SWITCH_TBL_UPD_PATH_7663   "/etc/FwRASwitchTblUpd7663.dat"
#define EAP_FW_RA_HW_FB_TBL_UPD_PATH_7663    "/etc/FwRAHwFbTblUpd7663.dat"
#define EAP_FW_RA_SWITCH_TBL_UPD_PATH_7626   "/etc/FwRASwitchTblUpd7626.dat"
#define EAP_FW_RA_HW_FB_TBL_UPD_PATH_7626    "/etc/FwRAHwFbTblUpd7626.dat"

#define NUM_OF_COL_RATE_SWITCH_TABLE  15
#define NUM_OF_COL_RATE_HWFB_TABLE    8
#define RA_TBL_INDEX_INVALID          0xFF

typedef enum _ENUM_RA_TABLE {
	eRateSwitchTable = 0,
	eRateHwFbTable,
	eRateTableMax
} ENUM_RA_TABLE, *P_ENUM_RA_TABLE;

typedef enum _ENUM_RA_SWITCH_TABLE {
	eRateSwTbl11b = 0,
	eRateSwTbl11g,
	eRateSwTbl11bg,
	eRateSwTbl11n1ss = 0x10,
	eRateSwTbl11n2ss,
	eRateSwTbl11n3ss,
	eRateSwTbl11n4ss,
	eRateSwTblvht1ss = 0x20,
	eRateSwTblvht2ss,
	eRateSwTblvht3ss,
	eRateSwTblvht4ss,
	eRateSwTblvht2ssbccbw80,
	eRateSwTblhe1ss = 0x30,
	eRateSwTblhe2ss,
	eRateSwTblMax = 0xff
} ENUM_RA_SWITCH_TABLE, *P_ENUM_RA_SWITCH_TABLE;

typedef enum _ENUM_RA_HWFB_TABLE {
	eRateHwFbTbl11b = 0,
	eRateHwFbTbl11g,
	eRateHwFbTbl11bg,
	eRateHwFbTbl11n1ss = 0x10,
	eRateHwFbTbl11n2ss,
	eRateHwFbTbl11n3ss,
	eRateHwFbTbl11n4ss,
	eRateHwFbTblbgn1ss,
	eRateHwFbTblbgn2ss,
	eRateHwFbTblbgn3ss,
	eRateHwFbTblbgn4ss,
	eRateHwFbTblvht1ss = 0x20,
	eRateHwFbTblvht2ss,
	eRateHwFbTblvht3ss,
	eRateHwFbTblvht4ss,
	eRateHwFbTblvht2ssbccbw80,
	eRateHwFbTblhe1ss = 0x30,
	eRateHwFbTblhe2ss,
	eRateHwFbTblMax = 0xff
} ENUM_RA_HWFB_TABLE, *P_ENUM_RA_HWFB_TABLE;

typedef struct _RATE_TABLE_UPDATE {
	UINT8 u1RaTblType;
	UINT8 u1RaTblIdx;
	CHAR  acTableName[40];
} RATE_TABLE_UPDATE, *P_RATE_TABLE_UPDATE;

typedef struct _CMD_INIT_IPI_CTRL_T {
	UINT32 u4EapCtrlCmdId;
	UINT8  u1BandIdx;
	UINT8  au1Reserved[3];
} CMD_INIT_IPI_CTRL_T, *P_CMD_INIT_IPI_CTRL_T;

typedef struct _CMD_GET_IPI_VALUE {
	UINT32 u4EapCtrlCmdId;
	UINT8  u1BandIdx;
	UINT8  au1Reserved[3];
} CMD_GET_IPI_VALUE, *P_CMD_GET_IPI_VALUE;

typedef struct _EVENT_GET_IPI_VALUE {
	UINT32 u4EapCtrlEventId;
	UINT32 au4IPIValue[11];
} EVENT_GET_IPI_VALUE, *P_EVENT_GET_IPI_VALUE;

typedef struct _CMD_SET_DATA_TXPWR_OFFSET {
	UINT32 u4EapCtrlCmdId;
	UINT8  u1WlanIdx;		/* #256STA */
	INT8   i1TxPwrOffset;
	UINT8  u1BandIdx;
} CMD_SET_DATA_TXPWR_OFFSET, *P_CMD_SET_DATA_TXPWR_OFFSET;

typedef struct _CMD_SET_RA_TABLE {
	UINT32 u4EapCtrlCmdId;
	UINT8  u1RaTblTypeIdx;
	UINT8  u1RaTblIdx;
	UINT8  u1BandIdx;
	UINT8  u1Reserved1;
	UINT16 u2RaTblLength;
	UINT16 u2Reserved2;
	UCHAR  ucBuf[512];
} CMD_SET_RA_TABLE, *P_CMD_SET_RA_TABLE;

typedef struct _EVENT_SHOW_RATE_TABLE {
	UINT32 u4EapCtrlEventId;
	UINT16 u2RaTblLength;
	UINT8  u1RaTblTypeIdx;
	UINT8  u1RaTblIdx;
	UINT8  u1RW;
	UINT8  u1Reserved[3];
	UCHAR  ucBuf[512];
} EVENT_SHOW_RATE_TABLE, *P_EVENT_SHOW_RATE_TABLE;

typedef struct _CMD_SHOW_RATE_TABLE {
	UINT32 u4EapCtrlCmdId;
	UINT8  u1RaTblTypeIdx;
	UINT8  u1RaTblIdx;
	UINT8  u1BandIdx;
	UINT8  u1RW;
} CMD_SHOW_RATE_TABLE, *P_CMD_SHOW_RATE_TABLE;

PCHAR getRaTableName(UINT8 TblType, UINT8 TblIdx);
UINT8 getRaTableIndex(UINT8 TblType, CHAR *TblName);
#endif /* WIFI_EAP_FEATURE */

#ifdef WIFI_GPIO_CTRL
#define GPIO_INDEX_MIN_VAL  10
#define GPIO_INDEX_MAX_VAL  13

typedef enum _GPIO_CTRL_CATEGORY {
	GPIO_GPO_SET_ENABLE = 0x0,
	GPIO_GPO_SET_VALUE = 0x1,
	GPIO_CTRL_NUM
} GPIO_CTRL_CATEGORY, *P_GPIO_CTRL_CATEGORY;

typedef struct _CMD_SET_GPIO_ENABLE {
	UINT32  u4GpioCtrlCmdId;
	UINT8   u1GpioIdx;
	BOOLEAN fgEnable;
	UINT8   au1Reserved[2];
} CMD_SET_GPIO_ENABLE, *P_CMD_SET_GPIO_ENABLE;

typedef struct _CMD_SET_GPIO_VALUE {
	UINT32 u4GpioCtrlCmdId;
	UINT8  u1GpioIdx;
	UINT8  u1Value;
	UINT8  au1Reserved[2];
} CMD_SET_GPIO_VALUE, *P_CMD_SET_GPIO_VALUE;
#endif /* WIFI_GPIO_CTRL */

typedef struct _CMD_POWER_MU_CTRL_T {
	UINT8   ucPowerCtrlFormatId;
	BOOLEAN fgMuTxPwrManEn;
	INT8    cMuTxPwr;
	UINT8   u1BandIdx;
} CMD_POWER_MU_CTRL_T, *P_CMD_POWER_MU_CTRL_T;

#ifdef DATA_TXPWR_CTRL
typedef struct _CMD_SET_PER_PKT_TX_POWER_T {
	UINT8 u1PowerCtrlFormatId;
	UINT8 u1Reserved[3];
	UINT8 u1BandIdx;
	INT8 i1MaxBasePwr;
	UINT16 u2WlanIdx;
	INT8  i1PowerOffset[DATA_TXPOWER_MAX_BW_NUM][DATA_TXPOWER_MAX_MCS_NUM];
} CMD_SET_PER_PKT_TX_POWER_T, *P_CMD_SET_PER_PKT_TX_POWER_T;

typedef struct _CMD_SET_MIN_TX_POWER_T {
	UINT8 u1PowerCtrlFormatId;
	UINT8 u1Reserved;
	UINT8 u1BandIdx;
	INT8 i1MinBasePwr;
} CMD_SET_MIN_TX_POWER_T, *P_CMD_SET_MIN_TX_POWER_T;
#endif

typedef struct _CMD_BF_NDPA_TXD_CTRL_T {
	UINT8    ucPowerCtrlFormatId;
	BOOLEAN  fgNDPA_ManualMode;
	UINT8    ucNDPA_TxMode;
	UINT8    ucNDPA_Rate;
	UINT8    ucNDPA_BW;
	UINT8    ucNDPA_PowerOffset;
	UINT8    ucReserved[2];
} CMD_BF_NDPA_TXD_CTRL_T, *P_CMD_BF_NDPA_TXD_CTRL_T;

typedef struct _CMD_SET_TSSI_TRAINING_T {
	UINT8    ucPowerCtrlFormatId;
	UINT8    ucSubFuncId;
	BOOLEAN  fgEnable;
	UINT8    ucReserved;
} CMD_SET_TSSI_TRAINING_T, *P_CMD_SET_TSSI_TRAINING_T;

typedef struct _CMD_POWER_TEMPERATURE_CTRL_T {
	UINT8    ucPowerCtrlFormatId;
	BOOLEAN  fgManualMode;    /* 1: Enable Temperature Manual Ctrl,  0: Disable Temperature Manual Ctrl */
	CHAR     cTemperature;    /* Temperature (Celsius Degree) */
	UINT8    u1BandIdx;
} CMD_POWER_TEMPERATURE_CTRL_T, *P_CMD_POWER_TEMPERATURE_CTRL_T;

typedef struct _CMD_THERMAL_MAN_CTRL_T {
	UINT8 u1PowerCtrlFormatId;
	UINT8 u1BandIdx;
	BOOLEAN fgManualMode;
	UINT8 u1ThermalAdc;
} CMD_THERMAL_MAN_CTRL_T, *P_CMD_THERMAL_MAN_CTRL_T;

typedef struct _CMD_THERMAL_SENSOR_TASK_T {
	UINT8 u1ThermalCtrlFormatId;
	UINT8 u1BandIdx;
	UINT8 u1Thres;
	BOOLEAN fgTrigEn;
	UINT32 u4FuncPtr;
} CMD_THERMAL_SENSOR_TASK_T, *P_CMD_THERMAL_SENSOR_TASK_T;

#define SKU_TABLE_SIZE_ALL      53
#define BF_BACKOFF_ON_MODE       0
#define BF_BACKOFF_OFF_MODE      1
#define BF_BACKOFF_MODE          2
#define BF_BACKOFF_CASE         10

typedef struct _EXT_EVENT_TXPOWER_INFO_T {
	UINT8	ucEventCategoryID;
	UINT8	ucBandIdx;
	BOOLEAN	fg2GEPA;
	BOOLEAN	fg5GEPA;

	BOOLEAN	fgSKUEnable;
	BOOLEAN	fgPERCENTAGEEnable;
	BOOLEAN	fgBFBACKOFFEnable;
	BOOLEAN	fgThermalCompEnable;

	INT8	cSKUTable[SKU_TABLE_SIZE_ALL];
	INT8	cThermalCompValue;
	INT8	cPowerDrop;
	UINT8	ucChannelBandIdx;

	UINT32	u4RatePowerCRValue[8];	/* (TMAC) Band0: 0x820F4020~0x820F403C, Band1: 0x820F4040~0x820F405C */
	CHAR	cTxPwrBFBackoffValue[BF_BACKOFF_MODE][BF_BACKOFF_CASE];
	UINT32	u4BackoffCRValue[6];	/* (BBP) Band0: 0x8207067C~82070690, Band1: 0x8207087C~82070890 */
	UINT32	u4PowerBoundCRValue;	/* (TMAC) 0x820F4080 */
} EXT_EVENT_TXPOWER_INFO_T, *P_EXT_EVENT_TXPOWER_INFO_T;

typedef struct _EXT_EVENT_TXPOWER_INFO_V1_T {
	UINT8    u1TxPowerCategory;
	UINT8    au1Reserved;

	/* Basic Info */
	UINT8    u1BandIdx;
	UINT8    u1ChBand;

	/* Board type info */
	BOOLEAN  fgPaType;
	BOOLEAN  fgLnaType;

	/* Power percentage info */
	BOOLEAN  fgPercentageEnable;
	CHAR     cPowerDrop;

	/* Frond-End Loss Tx info */
	CHAR     cFrondEndLossTx[4];

	/* Frond-End Loss Rx info */
	CHAR     cFrondEndLossRx[4];

	/* Thermal info */
	BOOLEAN  fgThermalCompEnable;
	CHAR     cThermalCompValue;
	UINT8	 au1Reserved2;

	/* tx Power Max/Min Limit info */
	CHAR    i1PwrMaxBnd;
	CHAR    i1PwrMinBnd;

	/* Power Limit info */
	BOOLEAN  fgSkuEnable;
	BOOLEAN  fgBackoffEnable;

	/* Mu Tx Power info */
	BOOLEAN  fgMuTxPwrManEn;
	CHAR     cMuTxPwr;
	CHAR     cMuTxPwrMan;
	UINT8   au1Reserved3;
} EXT_EVENT_TXPOWER_INFO_V1_T, *P_EXT_EVENT_TXPOWER_INFO_V1_T;

typedef struct _EXT_EVENT_TXPOWER_BACKUP_T {
	UINT8	ucEventCategoryID;
	UINT8	ucBandIdx;
	UINT8	aucReserve1[2];
	CHAR	cTxPowerCompBackup[SKU_TABLE_SIZE][SKU_TX_SPATIAL_STREAM_NUM];
} EXT_EVENT_TXPOWER_BACKUP_T, *P_EXT_EVENT_TXPOWER_BACKUP_T;

typedef struct _EXT_EVENT_EPA_STATUS_T {
	UINT8	ucEventCategoryID;
	BOOLEAN	fgEPA;
	UINT8	aucReserve1[2];
} EXT_EVENT_EPA_STATUS_T, *P_EXT_EVENT_EPA_STATUS_T;

typedef enum _TX_MODE_RATE {
	RATE_NONE = 0xffff
} TX_MODE_RATE;

typedef struct _TxCMD_RATE_INFO_T {/* 42 bytes */
    UINT16 rSuggestTxModeRate;
    UINT16 eUsrMinRate;/* for dynamic sounding */
    UINT16 eUsrMaxRate;/* for dynamic sounding */
    UINT16 initRateDownMCS;
    UINT16 u2CurrRate;/* Get form TxCMD report. 4bytes for Nss, 4bytes for MCS */
    UINT16 initRateDownTotalCnt;
    UINT16 initRateDownOkCnt;
    UINT16 u2StSucceCnt;
    UINT16 u2StTotalTxCnt;/* ST_TotalTxCnt; */
    UINT16 u2RuPrevRate;
    UINT16 u2StartProbeUpMCS;
    UINT8  u1NoRateUpCnt; /* the PPDU conter to trigger quick fall back */
    UINT8  u1StTotalPpduCnt; /* for low traffic RA */
    UINT8  u1Gi;
    UINT8  u1RuTryupFailCnt;
    UINT8  u1RuTryupCnt;
    UINT8  u1SuggestWF;
    BOOLEAN fgRuTryupCheck;
    BOOLEAN fgIsProbeUpPeriod;
    BOOLEAN fgProbeDownPending;
} TxCMD_RATE_INFO_T, *P_TxCMD_RATE_INFO_T;

typedef struct _EXT_EVENT_RU_RA_INFO_T {
	UINT32  u4EventId;
	UINT16  u2WlanIdx;
	UINT16  u2RuIdx;
	UINT16  u2Direction;
	UINT16  u2DumpGroup;
	UINT8 u1Version;
	UINT8 u1Reserved;
	TxCMD_RATE_INFO_T rRuIdxRateInfo;
} EXT_EVENT_RU_RA_INFO_T;

typedef struct _EXT_EVENT_MU_RA_INFO_T {
	UINT32  u4EventId;
	UINT16  u2MuGroupIdx;
	UINT16  u2UserIdx;
	UINT16  u2Direction;
	UINT16  u2DumpGroup;
	TxCMD_RATE_INFO_T rRuIdxRateInfo;
} EXT_EVENT_MU_RA_INFO_T;

typedef struct _EXT_EVENT_THERMAL_STATE_INFO_T {
    UINT8   ucEventCategoryID;
	UINT8   ucThermoItemsNum;
	UINT8   aucReserve[2];
	THERMO_ITEM_INFO_T arThermoItems[THERMO_ITEM_NUM];
} EXT_EVENT_THERMAL_STATE_INFO_T, *P_EXT_EVENT_THERMAL_STATE_INFO_T;

typedef struct _EXT_EVENT_TXV_BBP_POWER_INFO_T {
	UINT8  ucEventCategoryID;
	UINT8  ucBandIdx;
	CHAR  cTxvPower;
	CHAR  cTxvPowerDac;

	CHAR  cBbpPower[WF_NUM];

	CHAR  cBbpPowerDac[WF_NUM];

	UINT32  u2BbpPowerCR[WF_NUM];

	UINT32  u2TxvPowerCR;

	UINT8  ucTxvPowerMaskBegin;
	UINT8  ucTxvPowerMaskEnd;
	UINT8  ucBbpPowerMaskBegin;
	UINT8  ucBbpPowerMaskEnd;

	UINT8  ucWfNum;
	UINT8  aucReserve1[3];
} EXT_EVENT_TXV_BBP_POWER_INFO_T, *P_EXT_EVENT_TXV_BBP_POWER_INFO_T;

typedef struct _EXT_EVENT_TX_POWER_BACKUP_TABLE_INFO_T {
    UINT8   ucEventCategoryID;
	UINT8   ucBandIdx;
	UINT8   aucReserve[2];

	INT8    cTxPowerBackup[SKU_TABLE_SIZE][SKU_TX_SPATIAL_STREAM_NUM];
} EXT_EVENT_TX_POWER_BACKUP_TABLE_INFO_T, *P_EXT_EVENT_TX_POWER_BACKUP_TABLE_INFO_T;

typedef struct _CMD_ATE_MODE_CTRL_T {
	UINT8	ucPowerCtrlFormatId;
	BOOLEAN	fgATEModeEn;	/* 1: Enable ATE mode  0: disable ATE mode */
	UINT8	ucReserved[2];
} CMD_ATE_MODE_CTRL_T, *P_CMD_ATE_MODE_CTRL_T;

typedef struct _EXT_EVENT_THERMAL_COMPENSATION_TABLE_INFO_T {
	UINT8   ucEventCategoryID;
	UINT8   ucBand;
	UINT8   ucBandIdx;
	UINT8   aucReserve;

	INT8    cThermalComp[THERMAL_TABLE_SIZE];
	UINT8   aucReserve2;
} EXT_EVENT_THERMAL_COMPENSATION_TABLE_INFO_T, *P_EXT_EVENT_THERMAL_COMPENSATION_TABLE_INFO_T;

typedef struct _CMD_POWER_TPC_CTRL_T {
	UINT8	ucPowerCtrlFormatId;
	INT8	cTPCPowerValue;
	UINT8	ucBand;
	UINT8	ucChannelBand;
	UINT8	ucCentralChannel;
	UINT8	ucReserved[3];
} CMD_POWER_TPC_CTRL_T, *P_CMD_POWER_TPC_CTRL_T;

#ifdef DABS_QOS
typedef struct GNU_PACKED _CMD_FAST_PATH_T {
	/*Common Part*/
	UINT_8  ucOpMode;
	UINT_8  ucCmdVer;
	UINT_8  aucPadding0[2];
	UINT_16 u2CmdLen;

	/*afterwards*/
	UINT_8  aucOwnMac[6]; /* Own Mac address*/
	UINT_16 u2RandomNum; /* Random number genetate by Driver*/
	UINT_8  aucPadding1[2];
	UINT_32 u4Keybitmap[4]; /* Keybitmap send from Driver */
	UINT_16 u2WlanId;
	UINT_16 u2Mic;
} CMD_FAST_PATH_T, *P_CMD_FAST_PATH_T;

typedef struct _CAP_FAST_PATH_T {
	UINT_8 ucVersion;           /* Version of Fast Path feature */
	UINT_8 ucSupportFastPath;   /* 1: Support, 0: Not Support */
	UINT_32 u4Keybitmap[4];      /* Keybitmap of STA */
	UINT_8  ucReserved[2];
} CAP_FAST_PATH_T, *P_CAP_FAST_PATH_T;

typedef struct GNU_PACKED _EVENT_FAST_PATH_T {
	/* DWORD_0 - Common Part */
	UINT_8  ucEvtVer;
	UINT_8  aucPadding0[1];
	UINT_16 u2EvtLen;

	/* DWORD_1 - afterwards */
	UINT_16 u2Mic; /* message integrity check */
	UINT_8 ucKeynum; /* To tell AP side about STA use which key */
	UINT_8 u4KeybitmapMatchStatus; /* Tell if Keybitmap match success or not */

	CAP_FAST_PATH_T cap;
} EVENT_FAST_PATH_T, *P_EVENT_FAST_PATH_T;

/* for FastPath */
typedef struct GNU_PACKED _EXT_EVENT_FASTPATH_RPT_T {
	UINT8	ucfgValid;
	UINT8	wordlen;
	UINT8	u1Seq;
	UINT8	u1Reserved0[1];
	UINT16	u2FreeTokenUnderflowCnt[BAND_NUM];
	UINT16	u2FreePageUnderflowCnt;
	UINT8	u1Reserved1[2];
	UINT32	RecInUseBitmap[32];
} EXT_EVENT_FASTPATH_RPT_T, *P_EXT_EVENT_FASTPATH_RPT_T;

typedef struct GNU_PACKED _FASTPATH_RPT_T {
	UINT32	u4TxDly;
	UINT32	u4TxCnt;
	UINT32	u4DropCnt;
	UINT8	u1Reserved[4];
} FASTPATH_RPT_T, *P_FASTPATH_RPT_T;
#endif

#ifdef RED_SUPPORT

typedef struct GNU_PACKED _EXT_EVENT_RED_TX_RPT_T {
	UINT8 ucfgValid;
	UINT8 wordlen;
	UINT8 Reserve[2];
	UINT32 staInUseBitmap[32];
} EXT_EVENT_RED_TX_RPT_T, *P_EXT_EVENT_RED_TX_RPT_T;

typedef struct GNU_PACKED _RED_TX_RPT_T {
	UINT32                     u4TCPCnt;
	UINT32                     u4TCPAckCnt;
	UINT16  u2MsduInQueShortTimes;
	UINT16  u2MsduInQueLongTimes;
	UINT8	u1TCPMask;
	UINT8   u1Reserved[3];
} RED_TX_RPT_T, *P_RED_TX_RPT_T;

#define MPDU_TIME_FORMAT_VER                       (2)
#define MPDU_TIME_FORMAT_V3                        (3)
#define MPDU_TIME_BITMAP_TAG                       (0x5A)
typedef struct _MPDU_SHORT_TIME_UPDATE_T {
	UINT8	arPhymodeBW;
	UINT8	arAirtimeRatio;
	UINT16	wcid;
	UINT8	Reserve;
	UINT16	arN9TxARCnt;
	UINT16	arN9TxFRCnt;
	INT32	arMpduTime;
} MPDU_SHORT_TIME_UPDATE_T, *P_MPDU_SHORT_TIME_UPDATE_T;

typedef struct _MPDU_SHORT_AVG_TIME_UPDATE_T {
	UINT8	arPhymodeBW;
	UINT8	arAirtimeRatio;
	UINT16	wcid;
	UINT8	ucMcsShortGI;
	UINT16	arN9TxARCnt;
	UINT16	arN9TxFRCnt;
	INT32	arMpduTime;
	INT32	arMpduTime_avg;
} MPDU_SHORT_AVG_TIME_UPDATE_T, *P_MPDU_SHORT_AVG_TIME_UPDATE_T;

typedef struct _MPDU_SHORT_TIME_V3_UPDATE_T {
	UINT8   arPhymodeBW;
	UINT8   arAirtimeRatio;
	UINT8   ucMcsShortGI;
	UINT8   Reserved[1];
	UINT16  arN9TxARCnt;
	UINT16  arN9TxFRCnt;
	UINT16  arN9TxFailCnt;
	UINT8   Reserved1[2];
	INT32   arMpduTime;
	INT32   arMpduTime_avg;
} MPDU_SHORT_TIME_V3_UPDATE_T, *P_MPDU_SHORT_TIME_V3_UPDATE_T;

typedef struct _EXT_EVENT_MPDU_SHORT_TIME_UPDATE_T {
	UINT8	ucfgValid;
	UINT8	Reserve[3];
} EXT_EVENT_MPDU_TIME_UPDATE_T, *P_EXT_EVENT_MPDU_TIME_UPDATE_T;
#endif

#if defined(RED_SUPPORT) && defined(FQ_SCH_SUPPORT)
typedef struct _EXT_EVENT_MPDU_TIME_FQ_UPDATE_T {
	UINT8 ucfgValid;
	UINT8 Reserve[3];
	UINT8 arAirtimeRatio[MAX_LEN_OF_MAC_TABLE];
	INT32 arMpduTime[MAX_LEN_OF_MAC_TABLE];
} EXT_EVENT_MPDU_TIME_FQ_UPDATE_T, *P_EXT_EVENT_MPDU_TIME_FQ_UPDATE_T;
#endif /* defined(RED_SUPPORT) && defined(FQ_SCH_SUPPORT) */

typedef struct _CMD_LINK_TEST_TX_CSD_CTRL_T {
	UINT8	ucLinkTestCtrlFormatId;
	BOOLEAN	fgTxCsdConfigEn;
	UINT8	ucDbdcBandIdx;
	UINT8	ucBandIdx;
	UINT8	ucReserved;
} CMD_LINK_TEST_TX_CSD_CTRL_T, *P_CMD_LINK_TEST_TX_CSD_CTRL_T;

typedef struct _CMD_LINK_TEST_RX_CTRL_T {
	UINT8	ucLinkTestCtrlFormatId;
	UINT8	ucRxAntIdx;
	UINT8	ucBandIdx;
	UINT8	ucReserved;
} CMD_LINK_TEST_RX_CTRL_T, *P_CMD_LINK_TEST_RX_CTRL_T;

typedef struct _CMD_LINK_TEST_TXPWR_CTRL_T {
	UINT8	ucLinkTestCtrlFormatId;
	BOOLEAN	fgTxPwrConfigEn;
	UINT8	ucDbdcBandIdx;
	UINT8	ucBandIdx;
} CMD_LINK_TEST_TXPWR_CTRL_T, *P_CMD_LINK_TEST_TXPWR_CTRL_T;

typedef struct _CMD_LINK_TEST_TXPWR_UP_TABLE_CTRL_T {
	UINT8	ucLinkTestCtrlFormatId;
	UINT8	ucTxPwrUpCat;
	UINT8	ucTxPwrUpValue[13];
	UINT8	ucReserved;
} CMD_LINK_TEST_TXPWR_UP_TABLE_CTRL_T, *P_CMD_LINK_TEST_TXPWR_UP_TABLE_CTRL_T;

typedef struct _CMD_LINK_TEST_ACR_CTRL_T {
	UINT8	ucLinkTestCtrlFormatId;
	BOOLEAN	fgACRConfigEn;
	UINT8	ucDbdcBandIdx;
	UINT8	ucReserved;
} CMD_LINK_TEST_ACR_CTRL_T, *P_CMD_LINK_TEST_ACR_CTRL_T;

typedef struct _CMD_LINK_TEST_RCPI_CTRL_T {
	UINT8	ucLinkTestCtrlFormatId;
	BOOLEAN	fgRCPIConfigEn;
	UINT8	ucReserved[2];
} CMD_LINK_TEST_RCPI_CTRL_T, *P_CMD_LINK_TEST_RCPI_CTRL_T;

typedef struct _CMD_LINK_TEST_SEIDX_CTRL_T {
	UINT8	ucLinkTestCtrlFormatId;
	BOOLEAN	fgSeIdxConfigEn;
	UINT8	ucReserved[2];
} CMD_LINK_TEST_SEIDX_CTRL_T, *P_CMD_LINK_TEST_SEIDX_CTRL_T;

typedef struct _CMD_LINK_TEST_RCPI_MA_CTRL_T {
	UINT8	ucLinkTestCtrlFormatId;
	UINT8	ucMAParameter;
	UINT8	ucReserved[2];
} CMD_LINK_TEST_RCPI_MA_CTRL_T, *P_CMD_LINK_TEST_RCPI_MA_CTRL_T;

typedef struct _DECOMPRESS_REGION_INFO {
	UINT32 u4RegionAddress;
	UINT32 u4Regionlength;
	UINT32 u4RegionCRC;
} DECOMPRESS_REGION_INFO, *P_DECOMPRESS_REGION_INFO;

typedef struct _INIT_CMD_WIFI_START_WITH_DECOMPRESSION {
	UINT32 u4Override;
	UINT32 u4Address;
	UINT32 u4DecompressTmpAddress;
	UINT32 u4BlockSize;
	UINT32 u4RegionNumber;
	DECOMPRESS_REGION_INFO aucDecompRegion[3]; /* ilm, dlm, cmdbt*/
} INIT_CMD_WIFI_START_WITH_DECOMPRESSION, *P_INIT_CMD_WIFI_START_WITH_DECOMPRESSION;

/*align WMCPU part.*/
typedef enum _ENUM_SYSDVT_FEATURE_T {
	ENUM_SYSDVT_APPS = 0,
	ENUM_SYSDVT_BSS_COCLOR = 1,
	ENUM_SYSDVT_CMDBT = 2,
	ENUM_SYSDVT_DMASHDL = 3,
	ENUM_SYSDVT_GREENAP = 4,
	ENUM_SYSDVT_LP = 5,
	ENUM_SYSDVT_MURU = 6,
	ENUM_SYSDVT_NAN = 7,
	ENUM_SYSDVT_RA = 8,
	ENUM_SYSDVT_RADIO_ONOFF = 9,
	ENUM_SYSDVT_REPEATER = 10,
	ENUM_SYSDVT_TWTAP = 11,
	ENUM_SYSDVT_TWTSTA = 12,
	ENUM_SYSDVT_TXRX = 13,
	ENUM_SYSDVT_WDS = 14,
	ENUM_SYSDVT_DBDC = 15,
	ENUM_SYSDVT_WTBL = 16,
	ENUM_SYSDVT_TXBF = 17,
	ENUM_SYSDVT_SER = 18,
	ENUM_SYSDVT_PF = 19,
	ENUM_SYSDVT_STAPS = 20,
	ENUM_SYSDVT_MBSS = 21,
	ENUM_SYSDVT_CSI = 22,
	ENUM_SYSDVT_LOC = 23,
	ENUM_SYSDVT_SEC = 24,
	ENUM_SYSDVT_COEX = 25,
	ENUM_SYSDVT_TXCMDSU = 26
} _ENUM_SYSDVT_FEATURE_T;

typedef enum _ENUM_DBG_TXCMD_HANDLER_T {
    ENUM_DBG_TXCMDSU = 26
} ENUM_DBG_TXCMD_HANDLER_T;


/*command/event not need to follow linux coding style*/
typedef struct GNU_PACKED _CMD_SYSDVT_CTRL_EXT_T
{
	/** DWORD_0 - Common Part */
	UINT8  ucCmdVer;
	UINT8  aucPadding0[1];
	/** Cmd size including common part and body */
	UINT16 u2CmdLen;
	/** DWORD_N - Body Part */
	/** Feature  ID(_ENUM_SYSDVT_FEATURE_T)*/
	UINT32  u4FeatureIdx;
	/** Test case  ID (Type) */
	UINT32  u4Type;
	/** dvt parameter's data struct size (Length) */
	UINT32  u4Lth;
    /** dvt parameter's data struct (Value) */
	UINT8   u1cBuffer[0];
} CMD_SYSDVT_CTRL_EXT_T, *P_CMD_SYSDVT_CTRL_EXT_T;

enum {
	MU_EDCA_MIN = 0,
	MU_EDCA_MAX = 1,
};

typedef struct _EVENT_MURU_TUNE_AP_MUEDCA {
    UINT32     u4EventId;
    UINT16     u2MuEdcaSetting; /*255:max */
    UINT8      u1BandIdx;
} EVENT_MURU_TUNE_AP_MUEDCA, *P_EVENT_MURU_TUNE_AP_MUEDCA;

/* WIFI_MURU_STAT */
#define MURU_MAX_STA_CNT_PER_EVENT   8
#define MAX_AC_NUM                   4
#define MURU_STAT_RECORD_POOL_SIZE   128  /* kyle */
#define MURU_STAT_MU_GROUP_NUM       64

/* MODE A RECORD */
typedef struct _MURU_STAT_PER_WCID_RECORD {
    /* Qlen */
    UINT_32     au4DlAvgQlenBytes[MAX_AC_NUM];
    UINT_32     au4DlLatestQlenBytes[MAX_AC_NUM];
    UINT_32     au4UlAvgQlenBytes[MAX_AC_NUM];
    UINT_32     au4UlLatestQlenBytes[MAX_AC_NUM];
    UINT_16     au2RxAvgMpduSize[MAX_AC_NUM];
    UINT_16     au2TxAvgMpduSize[MAX_AC_NUM];
    UINT_16     au2DlLatestHeadPktDelay[MAX_AC_NUM];
    /* RA */
    UINT_8      u1TxNss:4;
    UINT_8      u1ContentionRxNsts:4;
    UINT_8      u1TxMcs:4;
    UINT_8      u1ContentionRxMcs:4;
    UINT_8      u1TrigRxMcs:4;
    UINT_8      u1ContentionRxMode:4;
    UINT_8      u1TxPER;
    UINT_8      u1ContentionRxPER;
    UINT_8      u1TrigRxPER;
	UINT_8      u1TrigRxMode;
	UINT_8      u1TxMode;
    UINT_16     u1TrigRxNsts:4;
    UINT_16     u1TxGi:3;
    UINT_16     u1ContentionRxGi:3;
    UINT_16     u1TrigRxGi:3;
    UINT_16     u1TxCoding:1;
    UINT_16     u1ContentionRxCoding:1;
    UINT_16     u1TrigRxCoding:1;
    UINT_16     u1TxHeLtf:3;
    UINT_16     u1TxBw:3;
    UINT_16     u1ContentionRxBw:3;
    UINT_16     u1TrigRxBw:3;
    UINT_16     u1TxStbc:1;
    UINT_16     u1ContentionRxStbc:1;
    UINT_16     u1TrigRxStbc:1;
    UINT_16     u1Reserved:1;

    /* TPC */
    UINT_8      u1LastRuIdx;
    INT_8       i1UlPwrHeadroom_dB;
    INT_16      i2LastPerUserRssi_dBm; /* unit: 0.5 dBm */
    UINT_16     u2reserved0;
} MURU_STAT_PER_WCID_RECORD, *P_MURU_STAT_PER_WCID_RECORD;

typedef struct _MURU_STAT_SURU {
    MURU_STAT_PER_WCID_RECORD     arRuStatPerWcid[MURU_MAX_STA_CNT_PER_EVENT];
} MURU_STAT_SURU, *P_MURU_STAT_SURU;

typedef struct _EVENT_MURU_STAT_PER_WCID {
	UINT16     u2Wcid;
	MURU_STAT_PER_WCID_RECORD  rRuStat;
} EVENT_MURU_STAT_PER_WCID, *P_EVENT_MURU_STAT_PER_WCID;

/* MODE B RECORD */
typedef struct _MURU_STAT_MIMO_RECORD {
    UINT_16   u2Wcid1;
    UINT_8    u1Wcid1TxMcs:4;
    UINT_8    u1Wcid1RxMcs:4;
    UINT_16   u2Wcid2;
    UINT_8    u1Wcid2TxMcs:4;
    UINT_8    u1Wcid2RxMcs:4;
    UINT_16   u2Wcid3;
    UINT_8    u1Wcid3TxMcs:4;
    UINT_8    u1Wcid3RxMcs:4;
    UINT_16   u2Wcid4;
    UINT_8    u1Wcid4TxMcs:4;
    UINT_8    u1Wcid4RxMcs:4;
} MURU_STAT_MIMO_RECORD, *P_MURU_STAT_MIMO_RECORD;

/* MODE C RECORD */
typedef struct _MURU_STAT_CH_BUSY_PERCENTAGE {
	UINT_8    u1Tx1stChBusyPCT;
	UINT_8    u1Tx2ndChBusyPCT;
	UINT_8    u1Tx3rdChBusyPCT;
	UINT_8    u1Tx4thChBusyPCT;
	UINT_8    u1Tx5thChBusyPCT;
	UINT_8    u1Tx6thChBusyPCT;
	UINT_8    u1Tx7thChBusyPCT;
	UINT_8    u1Tx8thChBusyPCT;
} MURU_STAT_CH_BUSY_PERCENTAGE, *P_MURU_STAT_CH_BUSY_PERCENTAGE;

typedef struct _MURU_STAT_TXBW_PERCENTAGE {
	UINT_8    u1TxBw80PCT;
	UINT_8    u1TxBw160PCT;
	UINT_8    u1TxBw80_Pp_Sec20PCT;
	UINT_8    u1TxBw80_Pp_Sec40PCT;
	UINT_8    u1TxBw160_Pp_Sec20PCT;
	UINT_8    u1TxBw160_Pp_Sec40PCT;
} MURU_STAT_TXBW_PERCENTAGE, *P_MURU_STAT_TXBW_PERCENTAGE;

typedef struct _MURU_STAT_DBG_RECORD {
	EVENT_MURU_STAT_PER_WCID      arRuStatPerWcid[MURU_MAX_STA_CNT_PER_EVENT];
	MURU_STAT_CH_BUSY_PERCENTAGE  rChBusyPercent;
	MURU_STAT_TXBW_PERCENTAGE     rMuRuStatTxBW;
} MURU_STAT_DBG_RECORD, *P_MURU_STAT_DBG_RECORD;

/* MODE A EVENT */
typedef struct _EVENT_MURU_STAT_MODE_A {
    UINT32     u4EventId;
    EVENT_MURU_STAT_PER_WCID arRuStatPerWcid[MURU_MAX_STA_CNT_PER_EVENT];
} EVENT_MURU_STAT_MODE_A, *P_EVENT_MURU_STAT_MODE_A;

typedef struct _EVENT_MURU_STAT_MODE_B {
    UINT_32     u4EventId;
    MURU_STAT_MIMO_RECORD rMuStatPerGroup[MURU_STAT_MU_GROUP_NUM];
} EVENT_MURU_STAT_MODE_B, *P_EVENT_MURU_STAT_MODE_B;

typedef struct _EVENT_MURU_STAT_MODE_C {
    UINT_32     u4EventId;
	MURU_STAT_DBG_RECORD rMuRuDbgRecord;
} EVENT_MURU_STAT_MODE_C, *P_EVENT_MURU_STAT_MODE_C;

enum {
    MODE_DISABLE = 0,
    MODE_A = 1,
    MODE_B = 2,
    MODE_C = 3
};

/* MURU_STAT END */


INT32 MtCmdFwDecompressStart(struct _RTMP_ADAPTER *ad, P_INIT_CMD_WIFI_START_WITH_DECOMPRESSION decompress_info);

#ifdef BCN_OFFLOAD_SUPPORT
#ifdef DOT11V_MBSSID_SUPPORT
BOOLEAN MtUpdateBcnToMcuV2(
	IN struct _RTMP_ADAPTER *pAd,
	VOID *wdev_void,
	BOOLEAN BcnSntReq,
	UCHAR UpdateReason);
#else
BOOLEAN MtUpdateBcnToMcu(
	IN struct _RTMP_ADAPTER *pAd,
	VOID *wdev_void,
	BOOLEAN BcnSntReq,
	UCHAR UpdateReason);
#endif /* DOT11V_MBSSID_SUPPORT */

INT32 MtCmdBcnOffloadSet(struct _RTMP_ADAPTER *pAd, CMD_BCN_OFFLOAD_T * bcn_offload);
#endif

INT32 MtCmdFdFrameOffloadSet(struct _RTMP_ADAPTER *pAd, P_CMD_FD_FRAME_OFFLOAD_T fdFrame_offload);

INT32 MtCmdMuarConfigSet(struct _RTMP_ADAPTER *pAd, UCHAR *pdata);

INT32 MtCmdExtPwrMgtBitWifi(struct _RTMP_ADAPTER *pAd, MT_PWR_MGT_BIT_WIFI_T rPwrMgtBitWifi);

#ifdef HOST_RESUME_DONE_ACK_SUPPORT
INT32 mt_cmd_host_resume_done_ack(struct _RTMP_ADAPTER *pAd);
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */

#ifdef GREENAP_SUPPORT
INT32 MtCmdExtGreenAPOnOffCtrl(struct _RTMP_ADAPTER *pAd, MT_GREENAP_CTRL_T GreenAPCtrl);
#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
INT32 mt_cmd_ext_pcie_aspm_dym_ctrl(struct _RTMP_ADAPTER *pAd, MT_PCIE_ASPM_DYM_CTRL_T PcieAspmDymCtrl);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
INT32 mt_cmd_ext_twt_agrt_update(struct _RTMP_ADAPTER *ad, struct mt_twt_agrt_para mt_twt_agrt_para);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

INT32 MtCmdExtPmStateCtrl(struct _RTMP_ADAPTER *pAd, MT_PMSTAT_CTRL_T PmStatCtrl);

UCHAR GetCfgBw2RawBw(UCHAR CfgBw);

INT32 CmdExtPwrMgtBitWifi(struct _RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT8 ucPwrMgtBit);

INT32 MtCmdRestartDLReq(struct _RTMP_ADAPTER *pAd);

INT32 MtCmdHIFLoopBackTest(struct _RTMP_ADAPTER *pAdapter, BOOLEAN IsEnable, UINT8 RxQ);

INT32 MtCmdPatchSemGet(struct _RTMP_ADAPTER *pAd, UINT32 Semaphore);

INT32 CmdHIFLoopbackReq(struct _RTMP_ADAPTER *pAd, UINT32 enable, UINT32 qidx);

INT32 MtCmdAddressLenReq(struct _RTMP_ADAPTER *pAd, UINT32 address, UINT32 len, UINT32 data_mode);

INT32 MtCmdFwScatter(struct _RTMP_ADAPTER *pAd, UINT8 *dl_payload, UINT32 dl_len, UINT32 count);

INT32 MtCmdPatchFinishReq(struct _RTMP_ADAPTER *pAd);

INT32 MtCmdFwStartReq(struct _RTMP_ADAPTER *pAd, UINT32 override, UINT32 address);

INT32 MtCmdWifiHifCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 ucHifCtrlId, VOID *pRsult);

INT32 CmdInitAccessRegWrite(struct _RTMP_ADAPTER *pAd, UINT32 address, UINT32 data);

INT32 CmdInitAccessRegRead(struct _RTMP_ADAPTER *pAd, UINT32 address, UINT32 *data);

INT32 CmdChPrivilege(struct _RTMP_ADAPTER *pAd, UINT8 Action, UINT8 control_chl, UINT8 central_chl,
					 UINT8 BW, UINT8 TXStream, UINT8 RXStream);

INT32 CmdAccessRegWrite(struct _RTMP_ADAPTER *pAd, UINT32 address, UINT32 data);

INT32 CmdAccessRegRead(struct _RTMP_ADAPTER *pAd, UINT32 address, UINT32 *data);

INT32 MtCmdRFRegAccessWrite(struct _RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 Value);

INT32 MtCmdRFRegAccessRead(struct _RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 *Value);
INT32 MtCmdRadioOnOffCtrl(struct _RTMP_ADAPTER *pAd, UINT8 On);

INT32 MtCmdWiFiRxDisable(struct _RTMP_ADAPTER *pAd);

INT32 MtCmdChannelSwitch(struct _RTMP_ADAPTER *pAd, struct _MT_SWITCH_CHANNEL_CFG SwChCfg);

#ifdef NEW_SET_RX_STREAM
INT MtCmdSetRxPath(struct _RTMP_ADAPTER *pAd, UINT32 Path, UCHAR BandIdx);
#endif

INT32 CmdNicCapability(struct _RTMP_ADAPTER *pAd);

INT32 MtCmdSecKeyReq(struct _RTMP_ADAPTER *pAd, UINT8 AddRemove, UINT8 Keytype, UINT8 *pAddr, UINT8 Alg,
					 UINT8 KeyID, UINT8 KeyLen, UINT8 WlanIdx, UINT8 *KeyMaterial);

INT32 MtCmdRfTestSwitchMode(struct _RTMP_ADAPTER *pAd,  UINT32 OpMode, UINT8 IcapLen, UINT16 rsp_len);

#ifdef PHY_ICS_SUPPORT
INT32 MtCmdPhyIcsStart(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pData);
#endif /* PHY_ICS_SUPPORT */

#ifdef WIFI_SPECTRUM_SUPPORT
INT32 MtCmdWifiSpectrumStart(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pData);

INT32 MtCmdWifiSpectrumUnSolicitCapStatus(
	IN struct _RTMP_ADAPTER *pAd);

INT32 MtCmdWifiSpectrumSolicitCapStatus(
	IN struct _RTMP_ADAPTER *pAd);

VOID MtCmdWifiSpectrumSolicitCapStatusCb(
	IN struct cmd_msg *msg,
	IN INT8 *pData,
	IN UINT16 Length);

INT32 MtCmdWifiSpectrumUnSolicitRawDataProc(
	IN struct _RTMP_ADAPTER *pAd);
#endif /* WIFI_SPECTRUM_SUPPORT */

#ifdef INTERNAL_CAPTURE_SUPPORT
INT32 MtCmdRfTestICapStart(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pData);

INT32 MtCmdRfTestUnSolicitICapStatus(
	IN struct _RTMP_ADAPTER *pAd);

INT32 MtCmdRfTestSolicitICapStatus(
	IN struct _RTMP_ADAPTER *pAd);

VOID MtCmdRfTestSolicitICapStatusCb(
	IN struct cmd_msg *msg,
	IN PINT32 pData,
	IN UINT16 Length);

INT32 MtCmdRfTestUnSolicitICapRawDataProc(
	IN struct _RTMP_ADAPTER *pAd);

INT32 MtCmdRfTestSolicitICapRawDataProc(
	IN struct _RTMP_ADAPTER *pAd,
	IN PINT32 pData,
	IN PINT32 pDataLen,
	IN UINT32 IQ_Type,
	IN UINT32 WF_Num);

VOID MtCmdRfTestSolicitICapIQDataCb(
	IN struct cmd_msg *msg,
	IN PINT32 pData,
	IN UINT16 Length);
#endif /* INTERNAL_CAPTURE_SUPPORT */

INT32 MtCmdRfTestSetADC(struct _RTMP_ADAPTER *pAd, UINT32 ChannelFreq, UINT8 AntIndex, UINT8 BW, UINT8 SX, UINT8 DbdcIdx, UINT8	RunType, UINT8 FType);

INT32 MtCmdRfTestSetRxGain(struct _RTMP_ADAPTER *pAd, UINT8 LPFG, UINT8	LNA, UINT8 DbdcIdx, UINT8 AntIndex);

INT32 MtCmdRfTestSetTTG(struct _RTMP_ADAPTER *pAd, UINT32 ChannelFreq, UINT32 ToneFreq, UINT8 TTGPwrIdx,
				UINT8 XtalFreq, UINT8 DbdcIdx);

INT32 MtCmdRfTestSetTTGOnOff(struct _RTMP_ADAPTER *pAd, UINT8 TTGEnable, UINT8 DbdcIdx, UINT8 AntIndex);

INT32 MtCmdDoCalibration(struct _RTMP_ADAPTER *pAd, UINT32 func_idx, UINT32 CalItem, UINT32 band_idx);

INT32 MtCmdTxContinous(struct _RTMP_ADAPTER *pAd, UINT32 PhyMode, UINT32 BW, UINT32 PriCh, UINT32 Central_Ch, UINT32 Mcs, UINT32 WFSel, UINT32 Txfd, UINT8 Band, UINT8 onoff);

INT32 MtCmdTxTone(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx, UINT8 Control, UINT8 AntIndex, UINT8 ToneType,
				  UINT8 ToneFreq, INT32 DcOffset_I, INT32 DcOffset_Q, UINT32 Band);

INT32 MtCmdTxTonePower(struct _RTMP_ADAPTER *pAd, INT32 type, INT32 dec, UINT8 TxAntennaSel, UINT8 Band);

INT32 MtCmdRfTestGetTxTonePower(struct _RTMP_ADAPTER *pAd, INT32 *pPower, UINT8 TxAntennaSel, UINT8 Band);

INT32 MtCmdSetRDDTestExt(struct _RTMP_ADAPTER *pAd, UINT32 rdd_idx, UINT32 rdd_in_sel, UINT32 IsStart);

INT32 MtCmdSetRDDTest(struct _RTMP_ADAPTER *pAd, UINT32 IsStart);

INT32 MtCmdSetCalDump(struct _RTMP_ADAPTER *pAd, UINT32 IsEnable);

INT32 MtCmdMultipleMacRegAccessWrite(struct _RTMP_ADAPTER *pAd, struct _RTMP_REG_PAIR *RegPair, UINT32 Num);

INT32 MtCmdMultipleMacRegAccessRead(struct _RTMP_ADAPTER *pAd, struct _RTMP_REG_PAIR *RegPair, UINT32 Num);

INT32 MtCmdMultipleRfRegAccessWrite(struct _RTMP_ADAPTER *pAd, struct _MT_RF_REG_PAIR *RegPair, UINT32 Num);

INT32 MtCmdMultipleRfRegAccessRead(struct _RTMP_ADAPTER *pAd, struct _MT_RF_REG_PAIR *RegPair, UINT32 Num);

INT32 MtCmdMultipleMibRegAccessRead(struct _RTMP_ADAPTER *pAd, UCHAR ChIdx, struct _RTMP_MIB_PAIR *RegPair, UINT32 Num);

INT32 MtCmdThermoCal(struct _RTMP_ADAPTER *pAd, UINT8 IsEnable, UINT8 SourceMode, UINT8 RFDiffTemp, UINT8 HiBBPHT, UINT8 HiBBPNT, INT8 LoBBPLT, INT8 LoBBPNT);

INT32 MtCmdFwLog2Host(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT8 FWLog2HostCtrl);

INT32 MtCmdFwDbgCtrl(struct _RTMP_ADAPTER *pAd, UINT8 dbg_lvl, UINT32 module_idx);

VOID CmdIOWrite32(void *hdev_ctrl, UINT32 Offset, UINT32 Value);

VOID CmdIORead32(void *hdev_ctrl, UINT32 Offset, UINT32 *Value);

VOID MtCmdEfusBufferModeSet(struct _RTMP_ADAPTER *pAd, UINT8 EepromType);

#ifdef EEPROM_RETRIEVE_SUPPORT
VOID MtCmdEfusBufferModeGet(struct _RTMP_ADAPTER *pAd, UINT8 EepromType, UINT16 dump_offset, UINT16 dump_size, UINT8 *epprom_content);
#endif /* EEPROM_RETRIEVE_SUPPORT*/

VOID MtCmdHwcfgGet(struct _RTMP_ADAPTER *pAd, UINT16 dump_offset, UINT16 dump_size, UINT8 *epprom_content);

INT32 MtCmdSetRxvFilter(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx, BOOLEAN bEnable);

NTSTATUS MtCmdPowerOnWiFiSys(struct _RTMP_ADAPTER *pAd);

VOID CmdExtEventRsp(struct cmd_msg *msg, char *Data, UINT16 Len);

INT32 MtCmdSendRaw(struct _RTMP_ADAPTER *pAd, UCHAR ExtendID, UCHAR *Input, INT len, UCHAR SetQuery);
#ifdef TXRX_STAT_SUPPORT
INT32 MtCmdGetPerStaTxStat(struct _RTMP_ADAPTER *pAd, UINT8 *ucEntryBitmap, UINT8 ucEntryCount);
#endif

#ifdef CONFIG_ATE
INT32 MtCmdGetTxPower(struct _RTMP_ADAPTER *pAd, UINT8 u1DbDcIdx, UINT8 u1CenterCh, UINT8 u1AntIdx, P_EXT_EVENT_ID_GET_TX_POWER_T prTxPwrResult);
INT32 MtCmdSetTxPowerCtrl(struct _RTMP_ADAPTER *pAd, struct _ATE_TXPOWER TxPower);
INT32 MtCmdSetForceTxPowerCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucBandIdx, INT8 cTxPower, UINT8 ucPhyMode, UINT8 ucTxRate, UINT8 ucBW);
#endif /* CONFIG_ATE */

#ifdef MT_MAC
INT32 CmdETxBfAidSetting(
	struct _RTMP_ADAPTER *pAd,
	UINT16       u2Aid,
	UINT8        u1BandIdx,
	UINT8        u1OwnMacIdx);

INT32 CmdTxBfApClientCluster(
	struct _RTMP_ADAPTER *pAd,
	UINT16  u2WlanIdx,
	UCHAR   ucCmmWlanId);

INT32 CmdTxBfReptClonedStaToNormalSta(
	struct _RTMP_ADAPTER *pAd,
	UINT16  u2WlanIdx,
	UCHAR   ucCliIdx);

INT32 cmd_txbf_config(
	struct _RTMP_ADAPTER *pAd,
	UINT8   config_type,
	UINT8   config_para[]);

INT32 CmdTxBfTxApplyCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT16  ucWlanId,
	BOOLEAN fgETxBf,
	BOOLEAN fgITxBf,
	BOOLEAN fgMuTxBf,
	BOOLEAN fgPhaseCali);

INT32 CmdITxBfPhaseCal(
	struct _RTMP_ADAPTER *pAd,
	UCHAR   ucGroup,
	UCHAR   ucGroupL_M_H,
	BOOLEAN fgSX2,
	BOOLEAN ucPhaseCal,
	UCHAR   ucPhaseVerifyLnaGainLevel);

INT32 CmdITxBfPhaseComp(
	struct _RTMP_ADAPTER *pAd,
	UCHAR   ucBW,
	UCHAR   ucBand,
	UCHAR   ucDbdcBandIdx,
	UCHAR	  ucGroup,
	BOOLEAN fgRdFromE2p,
	BOOLEAN fgDisComp);

INT32 CmdTxBfLnaGain(
	struct _RTMP_ADAPTER *pAd,
	UCHAR   ucLnaGain);

INT32 CmdETxBfSoundingPeriodicTriggerCtrl(
	struct _RTMP_ADAPTER *pAd,
	UCHAR   SndgEn,
	UINT32  u4SNDPeriod,
	UCHAR   ucSu_Mu,
	UCHAR   ucMuNum,
	PUCHAR  pwlanidx);

INT32 CmdPfmuMemAlloc(
	struct _RTMP_ADAPTER *pAd,
	UCHAR ucSu_Mu,
	UINT16 u2WlanIdx);

INT32 CmdPfmuMemRelease(
	struct _RTMP_ADAPTER *pAd,
	UINT16 u2WlanIdx);

INT32 CmdPfmuMemAllocMapRead(
	struct _RTMP_ADAPTER *pAd);

INT32 CmdETxBfPfmuProfileTagRead(
	struct _RTMP_ADAPTER *pAd,
	UCHAR                PfmuIdx,
	BOOLEAN              fgBFer);

INT32 CmdETxBfPfmuProfileTagWrite(
	struct _RTMP_ADAPTER *pAd,
	PUCHAR               prPfmuTag1,
	PUCHAR               prPfmuTag2,
	UINT8               tag1_len,
	UINT8               tag2_len,
	UCHAR                PfmuIdx);

INT32 CmdETxBfPfmuProfileDataRead(
	struct _RTMP_ADAPTER *pAd,
	UCHAR                PfmuIdx,
	BOOLEAN              fgBFer,
	USHORT               SubCarrIdx);

INT32 CmdETxBfPfmuProfileDataWrite(
	struct _RTMP_ADAPTER *pAd,
	UCHAR  PfmuIdx,
	USHORT SubCarrIdx,
	PUCHAR pProfileData);

INT32 CmdETxBfPfmuFullDimDataWrite(
	struct _RTMP_ADAPTER *pAd,
	UCHAR   PfmuIdx,
	USHORT  SubCarrIdx,
	BOOLEAN bfer,
	PUCHAR  pProfileData,
	UCHAR   DataLength);

INT32 CmdETxBfPfmuProfileDataWrite20MAll(
	struct _RTMP_ADAPTER *pAd,
	UCHAR                PfmuIdx,
	PUCHAR               pProfileData);

INT32 CmdETxBfPfmuProfilePnRead(
	struct _RTMP_ADAPTER *pAd,
	UCHAR                PfmuIdx);

INT32 CmdETxBfPfmuProfilePnWrite(
	struct _RTMP_ADAPTER *pAd,
	UCHAR                PfmuIdx,
	UCHAR                ucBw,
	PUCHAR               pProfileData);

INT32 CmdETxBfQdRead(
	struct _RTMP_ADAPTER *pAd,
	INT8                 subCarrIdx);

INT32 CmdETxBfFbRptDbgInfo(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pucData);

INT32 CmdETxBfTxSndInfo(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pucData);

INT32 CmdETxBfPlyInfo(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pucData);

INT32 CmdETxBfTxCmd(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pucData);

INT32 CmdETxBfSndCnt(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pucData);

INT32 CmdETxBfCfgBfPhy(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pucData);

INT32 CmdHeRaMuMetricInfo(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pucData);
#endif /* MT_MAC */

#ifdef COEX_SUPPORT
/*
 * Coex Sub
 */
enum EXT_BTCOEX_SUB {
	COEX_SET_PROTECTION_FRAME = 0x1,
	COEX_WIFI_STATUS_UPDATE  = 0x2,
	COEX_UPDATE_BSS_INFO = 0x03,
};

/*
 * Coex status bit
 */
enum EXT_BTCOEX_STATUS_bit {
	COEX_STATUS_RADIO_ON = 0x01,
	COEX_STATUS_SCAN_G_BAND = 0x02,
	COEX_STATUS_SCAN_A_BAND = 0x04,
	COEX_STATUS_LINK_UP = 0x08,
	COEX_STATUS_BT_OVER_WIFI = 0x10,
};

enum EXT_BTCOEX_PROTECTION_MODE {
	COEX_Legacy_CCK = 0x00,
	COEX_Legacy_OFDM = 0x01,
	COEX_HT_MIX = 0x02,
	COEX_HT_Green = 0x03,
	COEX_VHT = 0x04,
};

enum EXT_BTCOEX_OFDM_PROTECTION_RATE {
	PROTECTION_OFDM_6M = 0x00,
	PROTECTION_OFDM_9M = 0x01,
	PROTECTION_OFDM_12M = 0x02,
	PROTECTION_OFDM_18M = 0x03,
	PROTECTION_OFDM_24M = 0x04,
	PROTECTION_OFDM_36M = 0x05,
	PROTECTION_OFDM_48M = 0x06,
	PROTECTION_OFDM_54M = 0x07,
};
/*
 * Coex status bit
 */

typedef enum _WIFI_STATUS {
	STATUS_RADIO_ON = 0,
	STATUS_RADIO_OFF = 1,
	STATUS_SCAN_G_BAND = 2,
	STATUS_SCAN_G_BAND_END = 3,
	STATUS_SCAN_A_BAND = 4,
	STATUS_SCAN_A_BAND_END = 5,
	STATUS_LINK_UP = 6,
	STATUS_LINK_DOWN = 7,
	STATUS_BT_OVER_WIFI = 8,
	STATUS_BT_MAX,
} WIFI_STATUS;

typedef struct GNU_PACKED _CMD_COEXISTENCE_T {
	UINT8         ucSubOpCode;
	UINT8         aucReserve[3];
	UINT8          aucData[48];
} EXT_CMD_COEXISTENCE_T, *P_EXT_CMD_COEXISTENCE_T;

typedef struct GNU_PACKED _EVENT_EXT_COEXISTENCE_T {
	UINT8         ucSubOpCode;
	UINT8         aucReserve[3];
	UINT8         aucBuffer[64];
} EVENT_EXT_COEXISTENCE_T, *P_EVENT_EXT_COEXISTENCE_T;

typedef struct GNU_PACKED _COEX_WIFI_STATUS_UPDATE_T {
	UINT32      u4WIFIStatus;
} COEX_WIFI_STATUS_UPDATE_T, *P_COEX_WIFI_STATUS_UPDATE_T;

typedef struct GNU_PACKED _COEX_SET_PROTECTION_FRAME_T {
	UINT8      ucProFrameMode;
	UINT8      ucProFrameRate;
	UINT8      aucReserve[2];
} COEX_SET_PROTECTION_FRAME_T, *P_COEX_SET_PROTECTION_FRAME_T;

typedef struct GNU_PACKED _COEX_UPDATE_BSS_INFO_T {
	UINT8      u4BSSPresence[4];
	UINT8      u4BSSAPMode[4];
	UINT8      u4IsQBSS[4];
} COEX_UPDATE_BSS_INFO_T, *P_COEX_UPDATE_BSS_INFO_T;

typedef struct GNU_PACKED _EVENT_COEX_CMD_RESPONSE_T {
	UINT32         u4Status;
} EVENT_COEX_CMD_RESPONSE_T, *P_EVENT_COEX_CMD_RESPONSE_T;

typedef struct GNU_PACKED _EVENT_COEX_REPORT_COEX_MODE_T {
	UINT32         u4SupportCoexMode;
	UINT32         u4CurrentCoexMode;
} EVENT_COEX_REPORT_COEX_MODE_T, *P_EVENT_COEX_REPORT_COEX_MODE_T;

typedef struct GNU_PACKED _EVENT_COEX_MASK_OFF_TX_RATE_T {
	UINT8         ucOn;
	UINT8         aucReserve[3];
} EVENT_COEX_MASK_OFF_TX_RATE_T, *P_EVENT_COEX_MASK_OFF_TX_RATE_T;

typedef struct GNU_PACKED _EVENT_COEX_CHANGE_RX_BA_SIZE_T {
	UINT8         ucOn;
	UINT8         ucRXBASize;
	UINT8         aucReserve[2];
} EVENT_COEX_CHANGE_RX_BA_SIZE_T, *P_EVENT_COEX_CHANGE_RX_BA_SIZE_T;

typedef struct GNU_PACKED _EVENT_COEX_LIMIT_BEACON_SIZE_T {
	UINT8         ucOn;
	UINT8         aucReserve[3];
} EVENT_COEX_LIMIT_BEACON_SIZE_T, *P_EVENT_COEX_LIMIT_BEACON_SIZE_T;

typedef struct GNU_PACKED _EVENT_COEX_EXTEND_BTO_ROAMING_T {
	UINT8         ucOn;
	UINT8         aucReserve[3];
} EVENT_COEX_EXTEND_BTO_ROAMING_T, *P_EVENT_COEX_EXTEND_BTO_ROAMING_T;

typedef struct GNU_PACKED _COEX_TMP_FRAME_T {
	UINT8      ucProFrame1;
	UINT8      ucProFrame2;
	UINT8      ucProFrame3;
	UINT8      ucProFrame4;
} COEX_TMP_FRAME_T, *P_COEX_TMP_FRAME_T;

INT AndesCoexOP(struct _RTMP_ADAPTER *pAd,  UCHAR Status);

INT AndesCoexProtectionFrameOP(struct _RTMP_ADAPTER *pAd, UCHAR Mode, UCHAR Rate);

INT AndesCoexBSSInfo(struct _RTMP_ADAPTER *pAd, BOOLEAN Enable, UCHAR bQoS);

#endif /* COEX_SUPPORT */

#ifdef IGMP_TVM_SUPPORT
#define MCAST_RSP_ENTRY_TABLE 0x01

typedef struct _IGMP_MULTICAST_TABLE_MEMBER {
	UINT8 Addr[MAC_ADDR_LEN];
	UINT8 TVMode;
	UINT8 Rsvd;
} IGMP_MULTICAST_TABLE_MEMBER, *P_IGMP_MULTICAST_TABLE_MEMBER;

typedef struct _IGMP_MULTICAST_TABLE_ENTRY {
	UINT8 NumOfMember;
	UINT8 Rsvd1;
	UINT16 ThisGroupSize;
	UINT32 lastTime;
	UINT32 AgeOut;
	UINT32 type;	/* 0: static, 1: dynamic. */
	UINT8 GroupAddr[MAC_ADDR_LEN];
	UINT8 Rsvd2[2];
	IGMP_MULTICAST_TABLE_MEMBER IgmpMcastMember[1]; /* This member will be multiple of NumOfMember, shows variable structure */
} IGMP_MULTICAST_TABLE_ENTRY, *P_IGMP_MULTICAST_TABLE_ENTRY;

typedef struct _IGMP_MULTICAST_TABLE {
	UINT8 EvtSeqNum; /* Since there will be multiple events, this will store the sequence, starting from 1 */
	UINT8 NumOfGroup;
	UINT8 TotalGroup;
	UINT8 Rsvd;
	UINT16 ThisTableSize; /* Total size in current event. Only valid for event */
	UINT16 TotalSize;
	IGMP_MULTICAST_TABLE_ENTRY *pNxtFreeGroupLocation; /* Used only in driver */
	IGMP_MULTICAST_TABLE_ENTRY IgmpMcastTableEntry[1]; /* This member will be multiple of NumOfGroup, shows variable structure */
} IGMP_MULTICAST_TABLE, *P_IGMP_MULTICAST_TABLE;

typedef struct GNU_PACKED _EXT_EVENT_ID_IGMP_MULTICAST_SET_GET {
	UINT8 ucRspType;
	UINT8 ucOwnMacIdx;
	UINT8 Rsvd[2];
	union {
		IGMP_MULTICAST_TABLE McastTable;
	} RspData;
}  EXT_EVENT_ID_IGMP_MULTICAST_SET_GET, *P_EXT_EVENT_ID_IGMP_MULTICAST_SET_GET_T;
#endif /* IGMP_TVM_SUPPORT */


#ifdef RTMP_EFUSE_SUPPORT

INT32 MtCmdEfuseAccessRead(struct _RTMP_ADAPTER *pAd, USHORT offset, PUCHAR pData, PUINT isVaild);

VOID MtCmdEfuseAccessWrite(struct _RTMP_ADAPTER *pAd, USHORT offset, PUCHAR pData);

INT32 MtCmdEfuseFreeBlockCount(struct _RTMP_ADAPTER *pAd, PVOID GetFreeBlock, PVOID Result);
INT32 MtCmdEfuseAccessCheck(struct _RTMP_ADAPTER *pAd, UINT32 offset, PUCHAR pData);

#endif /* RTMP_EFUSE_SUPPORT */

INT32 MtCmdThermalProtect(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucBand,
	UINT8 HighEn,
	CHAR HighTempTh,
	UINT8 LowEn,
	CHAR LowTempTh,
	UINT32 RechkTimer,
	UINT8 RFOffEn,
	CHAR RFOffTh,
	UINT8 ucType
);

INT32
MtCmdThermalProtectAdmitDuty(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucBand,
	UINT32 u4Lv0Duty,
	UINT32 u4Lv1Duty,
	UINT32 u4Lv2Duty,
	UINT32 u4Lv3Duty
);

#ifdef PRETBTT_INT_EVENT_SUPPORT
VOID MtSetTriggerPretbttIntEvent(struct _RTMP_ADAPTER *ad, INT apidx, UCHAR HWBssidIdx, BOOLEAN Enable, UINT16 BeaconPeriod);
INT32 MtCmdTrgrPretbttIntEventSet(struct _RTMP_ADAPTER *ad, CMD_TRGR_PRETBTT_INT_EVENT_T trgr_pretbtt_int_event);
#endif /*PRETBTT_INT_EVENT_SUPPORT*/

INT32 CmdCrUpdate(struct _RTMP_ADAPTER *pAd, VOID *Context, UINT16 Length);

/* PNDIS_PACKET WtblTlvBufferAlloc(struct _RTMP_ADAPTER *pAd,  UINT32 u4AllocateSize); */
/* VOID* WtblNextTlvBuffer(PNDIS_PACKET pWtblTlvBuffer, UINT16 u2Length); */
/* VOID WtblTlvBufferAppend(PNDIS_PACKET pWtblTlvBuffer,  UINT16 u2Type, UINT16 u2Length, PUCHAR pNextWtblTlvBuffer); */
/* VOID WtblTlvBufferFree(struct _RTMP_ADAPTER *pAd, PNDIS_PACKET pWtblTlvBuffer); */
VOID *pTlvAppend(VOID *pTlvBuffer, UINT16 u2Type, UINT16 u2Length, VOID *pNextTlvBuffer, UINT32 *pu4TotalTlvLen, UCHAR *pucTotalTlvNumber);
INT32 CmdExtTlvBufferSend(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ExtCmdType,
	VOID *pTlvBuffer,
	UINT32 u4TlvLength);
INT32 CmdExtWtblUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16 u2WlanIdx,
	UINT8 ucOperation,
	VOID *pBuffer,
	UINT32 u4BufferLen);
UINT32 WtblDwQuery(struct _RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT8 ucWtbl1234, UINT8 ucWhichDW);
INT32 WtblDwSet(
	struct _RTMP_ADAPTER *pAd,
	UINT16 u2WlanIdx,
	UINT8 ucWtbl1234,
	UINT8 ucWhichDW,
	UINT32 u4DwMask,
	UINT32 u4DwValue);

#ifdef WTBL_TDD_SUPPORT
UINT32 UWtblRRaw(struct _RTMP_ADAPTER *pAd, UINT16 ucWlanIdx, IN CMD_STAREC_UWTBL_RAW_T *pStaUWBLRaw);
#endif /* WTBL_TDD_SUPPORT */

struct cmd_wtbl_dw_mask_set {
	UINT8 ucWhichDW;
	UINT32 u4DwMask;
	UINT32 u4DwValue;
};
INT32 WtblResetAndDWsSet(
	struct _RTMP_ADAPTER *pAd,
	UINT16 u2WlanIdx,
	UINT8 ucWtbl1234,
	INT dw_cnt,
	struct cmd_wtbl_dw_mask_set *dw_set);


/* please pass NULL pointer wtbl_security_key, this api will dynamic allocate */
INT32 fill_key_install_cmd(
	struct _ASIC_SEC_INFO *asic_sec_info,
	UCHAR is_sta_rec_update, /* TRUE: sta_rec, FALSE: wtbl */
	VOID **wtbl_security_key,
	OUT UINT32 *cmd_len);

/* please pass NULL pointer wtbl_security_key, this api will dynamic allocate */
INT32 fill_key_install_cmd_v2(
	struct _ASIC_SEC_INFO *asic_sec_info,
	UCHAR is_sta_rec_update, /* TRUE: sta_rec, FALSE: wtbl */
	VOID **wtbl_security_key,
	OUT UINT32 *cmd_len);

#ifdef WIFI_UNIFIED_COMMAND
INT32 fill_key_install_uni_cmd_dynsize_check_v2(
	struct _ASIC_SEC_INFO *asic_sec_info,
	OUT UINT32 *cmd_len);

INT32 fill_key_install_uni_cmd_v2(
	struct _ASIC_SEC_INFO *asic_sec_info,
	UCHAR is_sta_rec_update, /* TRUE: sta_rec, FALSE: wtbl */
	VOID *wtbl_security_key,
	OUT UINT32 *cmd_len);
#endif /* WIFI_UNIFIED_COMMAND */

INT32 CmdExtDevInfoUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 EnableFeature);

INT32 CmdExtSetTmrCR(
	struct _RTMP_ADAPTER *pAd,
	UCHAR enable,
	UCHAR BandIdx);

INT32 CmdExtStaRecUpdate(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg);

INT32 CmdETxBfStaRecRead(
	struct _RTMP_ADAPTER *pAd,
	UINT16  u2WlanId);

INT32 MtCmdThermalProtectAdmitDutyInfo(struct _RTMP_ADAPTER *pAd);

INT32 CmdTxBfAwareCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgBfAwareCtrl);

INT32 cmd_txbf_en_dynsnd_intr(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN is_intr_en);

#ifdef CFG_SUPPORT_MU_MIMO
INT32 cmd_txbf_cfg_dynsnd_dmcsth(
	struct _RTMP_ADAPTER *pAd,
	UINT8 mcs_index,
	UINT8 mcsth);

INT32 cmd_txbf_en_dynsnd_pfid_intr(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN mu_intr_en,
	UINT8 pfid);
#endif

INT32 CmdTxBfeeHwCtrl(
    struct _RTMP_ADAPTER *pAd,
    BOOLEAN fgBfeeHwEn);

INT32 CmdTxBfHwEnableStatusUpdate(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgEBf,
	BOOLEAN fgIBf);

INT32 CmdTxBfModuleEnCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BfNum,
	UINT8 u1BfBitmap,
	UINT8 u1BfSelBand[]);

INT32 CmdETxBfPseudoTagWrite(
	struct _RTMP_ADAPTER *pAd,
	EXT_CMD_ETXBF_PFMU_SW_TAG_T rEBfPfmuSwTag);

INT32 CmdMecCtrl(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pucData);

INT32 CmdHeraStbcPriorityCtrl(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pucData);

INT32 CmdExtStaRecBaUpdate(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_BA_CFG_T StaRecBaCfg);

#ifdef HTC_DECRYPT_IOT
INT32 CmdExtStaRecAADOmUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16 Wcid,
	UINT8 AadOm);
#endif /* HTC_DECRYPT_IOT */

INT32 CmdExtStaRecPsmUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16 Wcid,
	UINT8 Psm);

INT32 CmdExtStaRecSNUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16 Wcid,
	UINT16 Sn);

INT32 CmdSetSyncModeByBssInfoUpdate(
	struct _RTMP_ADAPTER *pAd,
	struct _BSS_INFO_ARGUMENT_T *bss_info_argument);

INT32 CmdExtBssInfoUpdate(
	struct _RTMP_ADAPTER *pAd,
	struct _BSS_INFO_ARGUMENT_T *bss_info_argument);
#ifdef ZERO_LOSS_CSA_SUPPORT
INT32 CmdExtBssInfoUpdateTsf (
	struct _RTMP_ADAPTER *pAd,
	struct _BSS_INFO_ARGUMENT_T *bss_info_argument);
#endif

#ifdef CONFIG_HW_HAL_OFFLOAD
INT32 MtCmdATETest(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_ATE_TEST_MODE_T *param);
INT32 MtCmdCfgOnOff(struct _RTMP_ADAPTER *pAd, UINT8 Type, UINT8 Enable, UINT8 Band);
INT32 MtCmdSetAntennaPort(struct _RTMP_ADAPTER *pAd, UINT8 RfModeMask, UINT8 RfPortMask, UINT8 AntPortMask);
INT32 MtCmdATESetSlotTime(struct _RTMP_ADAPTER *pAd, UINT8 SlotTime, UINT8 SifsTime, UINT8 RifsTime, UINT16 EifsTime, UCHAR BandIdx);
INT32 MtCmdATESetPowerDropLevel(struct _RTMP_ADAPTER *pAd, UINT8 PowerDropLevel, UCHAR BandIdx);
INT32 MtCmdRxFilterPktLen(struct _RTMP_ADAPTER *pAd, UINT8 Enable, UINT8 Band, UINT32 RxPktLen);
INT32 MtCmdSetFreqOffset(struct _RTMP_ADAPTER *pAd, UINT32 FreqOffset, UINT8 BandIdx);
INT32 MtCmdGetFreqOffset(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx, UINT32 *pFreqOffsetResult);
INT32 MtCmdGetCfgOnOff(struct _RTMP_ADAPTER *pAd, UINT32 Type, UINT8 Band, UINT32 *Status);
INT32 MtCmdSetPhyCounter(struct _RTMP_ADAPTER *pAd, UINT32 Control, UINT8 band_idx);
INT32 MtCmdSetRxvIndex(struct _RTMP_ADAPTER *pAd, UINT8 Group_1, UINT8 Group_2, UINT8 band_idx);
INT32 MtCmdSetFAGCPath(struct _RTMP_ADAPTER *pAd, UINT8 Path, UINT8 band_idx);
#endif
INT32 MtCmdClockSwitchDisable(struct _RTMP_ADAPTER *pAd, UINT8 isDisable);
INT32 MtCmdUpdateProtect(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_UPDATE_PROTECT_T *param);
INT32 MtCmdSetRdg(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_RDG_CTRL_T *param);
INT32 MtCmdSetSnifferMode(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_SNIFFER_MODE_T *param);

VOID MtCmdMemDump(struct _RTMP_ADAPTER *pAd, UINT32 Addr, PUINT8 pData);

#ifdef CONFIG_ATE
INT32 CmdTxContinous(struct _RTMP_ADAPTER *pAd, UINT32 PhyMode, UINT32 BW, UINT32 PriCh, UINT32 Mcs, UINT32 WFSel, UCHAR onoff);
INT32 CmdTxTonePower(struct _RTMP_ADAPTER *pAd, INT32 type, INT32 dec);
#endif

INT32 MtCmdGetThermalSensorResult(struct _RTMP_ADAPTER *pAd, UINT8 ActionIdx, UINT8 ucDbdcIdx, UINT32 *SensorResult);

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
struct _EXT_EVENT_TX_STATISTIC_RESULT_T;
INT32 MtCmdGetTxStatistic(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucField,
	UINT8 ucBand,
	UINT16 u2Wcid,
	struct _EXT_EVENT_TX_STATISTIC_RESULT_T *prTxStatResult);
INT32 mt_cmd_get_sta_tx_statistic(struct _RTMP_ADAPTER *ad, TX_STAT_STRUC *pTxStat, UCHAR num);

struct _TX_STATISTIC_RESULT_PAIR_T;
INT32 MtCmdGetMutiTxStatistic(struct _RTMP_ADAPTER *pAd, TX_STATISTIC_RESULT_PAIR *TxStatResultPair, UINT32 Num);

#ifdef RACTRL_LIMIT_MAX_PHY_RATE
INT32 MtCmdSetMaxPhyRate(struct _RTMP_ADAPTER *pAd, UINT16 u2MaxPhyRate);
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

INT32 MtCmdSetUseVhtRateFor2G(struct _RTMP_ADAPTER *pAd);

INT32 MtCmdSetVht1024QamSupport(struct _RTMP_ADAPTER *pAd);

INT SetHeraProtectionPerPpduDis(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT SetHeraMuInitRateInterval(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT SetHeraMuDisableSwitchSu(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT SetHeraSingleNssTxEnable(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT32 MtCmdTmrCal(struct _RTMP_ADAPTER *pAd, UINT8 enable, UINT8 band, UINT8 bw, UINT8 ant, UINT8 role);

INT32 MtCmdEdcaParameterSet(struct _RTMP_ADAPTER *pAd, MT_EDCA_CTRL_T *EdcaParam);

INT32 MtCmdSlotTimeSet(struct _RTMP_ADAPTER *pAd, UINT8 SlotTime, UINT8 SifsTime, UINT8 RifsTime, UINT16 EifsTime, UCHAR BandIdx);

#ifdef CONFIG_MULTI_CHANNEL
INT MtCmdMccStart(struct _RTMP_ADAPTER *pAd, UINT32 Num, MT_MCC_ENTRT_T *MccEntries, USHORT IdleTime, USHORT NullRepeatCnt, ULONG StartTsf);

INT32 MtCmdMccStop(struct _RTMP_ADAPTER *pAd, UCHAR ParkingIndex, UCHAR   AutoResumeMode, UINT16 AutoResumeInterval, ULONG  AutoResumeTsf);
#endif /* CONFIG_MULTI_CHANNEL */

#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
INT32 CmdP2pNoaOffloadCtrl(struct _RTMP_ADAPTER *ad, UINT8 enable);
#endif /* defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA) */

INT32 MtCmdSendMsg(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg);
INT32 MtCmdLEDCtrl(struct _RTMP_ADAPTER *pAd, UINT32 LEDNumber, UINT32 LEDBehavior);

#ifdef ERR_RECOVERY
INT32 CmdExtGeneralTestOn(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
INT32 CmdExtGeneralTestMode(struct _RTMP_ADAPTER *pAd, UINT8 mode, UINT8 submode);
#endif /* ERR_RECOVERY */

#ifdef DBDC_MODE
INT32 MtCmdGetDbdcCtrl(struct _RTMP_ADAPTER *pAd, struct _BCTRL_INFO_T *pBandInfo);
INT32 MtCmdSetDbdcCtrl(struct _RTMP_ADAPTER *pAd, struct _BCTRL_INFO_T *pBandInfo);
#endif

#ifdef MT_DFS_SUPPORT/* Jelly20150123 */
INT32 MtCmdRddCtrl(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR ucRddCtrl,
	IN UCHAR ucRddIdex,
	IN UCHAR ucRddRxSel,
	IN UCHAR ucSetVal);

INT32 mt_cmd_set_fcc5_min_lpn(struct _RTMP_ADAPTER *pAd, UINT16 min_lpn_update);
INT32 mt_cmd_set_radar_thres_param(struct _RTMP_ADAPTER *pAd, P_CMD_RDM_RADAR_THRESHOLD_UPDATE_T  p_radar_threshold);
INT32 mt_cmd_set_pls_thres_param(struct _RTMP_ADAPTER *pAd, P_CMD_RDM_PULSE_THRESHOLD_UPDATE_T p_pls_threshold);
INT32 mt_cmd_set_rdd_log_config(struct _RTMP_ADAPTER *pAd, UINT8 hw_rdd_log_en,	UINT8 sw_rdd_log_en, UINT8 sw_rdd_log_cond);
INT32 mt_cmd_set_test_radar_pattern(struct _RTMP_ADAPTER *pAd, P_CMD_RDM_TEST_RADAR_PATTERN_T cmd_set_test_pls_pattern);

typedef enum _ENUM_RDD_THRESCMD_T {
	ENUM_RDM_FCC5_LPN_UPDATE = 1,
	ENUM_RDM_RADAR_THRESHOLD_UPDATE,
	ENUM_RDM_PULSE_THRESHOLD_UPDATE,
	ENUM_RDM_RDD_LOG_CONFIG_UPDATE,
	ENUM_RDM_MAX_COMMAND
} ENUM_RDD_THRESCMD_T, *P_ENUM_RDD_THRESCMD_T;

#endif /*MT_DFS_SUPPORT*/

#if OFF_CH_SCAN_SUPPORT
INT32 mt_cmd_off_ch_scan(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _EXT_CMD_OFF_CH_SCAN_CTRL_T *ext_cmd_param);
#endif

INT32 MtCmdGetChBusyCnt(struct _RTMP_ADAPTER *pAd, UCHAR ChIdx, UINT32 *pChBusyCnt);
INT32 MtCmdGetTsfTime(struct _RTMP_ADAPTER *pAd, UCHAR HwBssidIdx, TSF_RESULT_T *pTsfResult);
INT32 MtCmdGetPartialMibInfoCnt(struct _RTMP_ADAPTER *pAd, UCHAR ChIdx, MT_PARTIAL_MIB_INFO_CNT_CTRL_T *pPartialMibInfoCtrl);
INT32 MtCmdGetEdca(struct _RTMP_ADAPTER *pAd, MT_EDCA_CTRL_T *pEdcaCtrl);
INT32 MtCmdGetWifiInterruptCnt(struct _RTMP_ADAPTER *pAd, UCHAR ChIdx, UCHAR WifiIntNum, UINT32 WifiIntMask, UINT32 *pWifiInterruptCnt);

INT32 MtCmdSetMacTxRx(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx, BOOLEAN bEnable);
#ifdef MT_DFS_SUPPORT
INT32 MtCmdSetDfsTxStart(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx);
#endif
#ifdef PRE_CAL_TRX_SET1_SUPPORT
INT32 MtCmdGetRXDCOCCalResult(struct _RTMP_ADAPTER *pAd, BOOLEAN DirectionToCR
							  , UINT16 CentralFreq, UINT8 BW, UINT8 Band, BOOLEAN IsSecondary80, BOOLEAN DoRuntimeCalibration, RXDCOC_RESULT_T *pRxDcocResult);
INT32 MtCmdGetTXDPDCalResult(struct _RTMP_ADAPTER *pAd, BOOLEAN DirectionToCR
							 , UINT16 CentralFreq, UINT8 BW, UINT8 Band, BOOLEAN IsSecondary80, BOOLEAN DoRuntimeCalibration, TXDPD_RESULT_T *pTxDPDResult);
INT32 MtCmdRDCE(struct _RTMP_ADAPTER *pAd, UINT8 type, UINT8 BW, UINT8 Band);
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
#ifdef PRE_CAL_MT7622_SUPPORT
INT32 MtCmdRfTestRecal(struct _RTMP_ADAPTER *pAd, UINT32 u4CalId, UINT16 rsp_len);
#endif /*PRE_CAL_MT7622_SUPPORT*/
#ifdef RLM_CAL_CACHE_SUPPORT
VOID rlmCalCacheApply(struct _RTMP_ADAPTER *pAd, VOID *rlmCache);
#endif /* RLM_CAL_CACHE_SUPPORT */

#ifdef PRE_CAL_TRX_SET2_SUPPORT
INT32 MtCmdGetPreCalResult(struct _RTMP_ADAPTER *pAd, UINT8 CalId, UINT16 PreCalBitMap);
INT32 MtCmdPreCalReStoreProc(struct _RTMP_ADAPTER *pAd, INT32 *pPreCalBuffer);
#endif/* PRE_CAL_TRX_SET2_SUPPORT */
INT32 MtCmdThermalMode(struct _RTMP_ADAPTER *pAd, UINT8 Mode, UINT8 Action);

#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
INT32 MtCmdCalReStoreFromFileProc(struct _RTMP_ADAPTER *pAd, CAL_RESTORE_FUNC_IDX FuncIdx);
INT32 MtCmdPATrimReStoreProc(struct _RTMP_ADAPTER *pAd);
#endif /* CAL_BIN_FILE_SUPPORT */
#ifdef ZERO_LOSS_CSA_SUPPORT
typedef enum _ENUM_ZERO_PKT_LOSS_VARIABLE {
	CHANNEL_SWITCH_TRIGGER_COUNT = 0,
	ZERO_PKT_LOSS_ENABLE,
} ENUM_ZERO_PKT_LOSS_VARIABLE, *P_ZERO_PKT_LOSS_VARIABLE;

INT32 MtCmdSetChkPeerLink(struct _RTMP_ADAPTER *pAd, UINT8 WcidCount, UINT8 *wcidlist);
INT32 MtCmdSetZeroPktLossVariable(struct _RTMP_ADAPTER *pAd, ENUM_ZERO_PKT_LOSS_VARIABLE eVariable, UINT8 Value);
INT32 MtCmdSetMacTxEnable(struct _RTMP_ADAPTER *pAd, UINT8 enable);
#endif /*ZERO_LOSS_CSA_SUPPORT*/

INT32 CmdRxHdrTransUpdate(struct _RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan, BOOLEAN SwPcP);
INT32 CmdRxHdrTransBLUpdate(struct _RTMP_ADAPTER *pAd, UINT8 Index, UINT8 En, UINT16 EthType);

INT32 MtCmdSetVoWDRRCtrl(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_VOW_DRR_CTRL_T *param);
INT32 MtCmdSetVoWGroupCtrl(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_BSS_CTRL_T *param);
INT32 MtCmdSetVoWFeatureCtrl(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_VOW_FEATURE_CTRL_T *param);
INT32 MtCmdSetVoWRxAirtimeCtrl(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_RX_AT_CTRL_T *param);
INT32 MtCmdGetVoWRxAirtimeCtrl(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_RX_AT_CTRL_T *param);
INT32 MtCmdSetVoWModuleCtrl(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_AT_PROC_MODULE_CTRL_T *param);
INT32 MtCmdSetVoWCounterCtrl(struct _RTMP_ADAPTER *pAd, UCHAR cmd, UCHAR val);
#if defined(MT7615_FPGA) || defined(MT7622_FPGA)
INT32 MtCmdSetStaQLen(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 qLen);
INT32 MtCmdSetSta2QLen(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 qLen);
INT32 MtCmdSetEmptyThreshold(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 threshold);
INT32 MtCmdSetStaCnt(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 cnt);
#endif /* defined(MT7615_FPGA) || defined(MT7622_FPGA) */
#ifdef GN_MIXMODE_SUPPORT
INT32 MtCmdSetGNMixModeEnable(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 en);
#endif /* GN_MIXMODE_SUPPORT */
#ifdef RED_SUPPORT
INT32 MtCmdSetRedShowSta(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 Num);
INT32 MtCmdSetRedEnable(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 en);
INT32 MtCmdSetRedTargetDelay(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 Num);
INT32 MtCmdSetRedTxReport(struct _RTMP_ADAPTER *pAd, UCHAR cmd, PUCHAR buffer, UINT16 len);
#endif /* RED_SUPPORT */
INT32 MtCmdSetCPSEnable(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 en);
INT32 CmdAutoBATrigger(struct _RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Timeout);
#if defined(MT7986) || defined(MT7916) || defined(MT7981)
INT32 MtCmdCr4MultiQuery(struct _RTMP_ADAPTER *pAd, UINT32 arg0, UINT32 arg1, UINT32 arg2, void *para);
#endif
INT32 MtCmdCr4Query(struct _RTMP_ADAPTER *pAd, UINT32 arg0, UINT32 arg1, UINT32 arg2);
INT32 MtCmdCr4Set(struct _RTMP_ADAPTER *pAd, UINT32 arg0, UINT32 arg1, UINT32 arg2);
INT32 MtCmdCr4Capability(struct _RTMP_ADAPTER *pAd, UINT32 option);
INT32 MtCmdCr4Debug(struct _RTMP_ADAPTER *pAd, UINT32 option);
INT32 mt_cmd_wo_query(struct _RTMP_ADAPTER *pAd, UINT32 option, UINT32 param0, UINT32 param1);
INT MtCmdSetTxRxPath(struct _RTMP_ADAPTER *pAd, struct _MT_SWITCH_CHANNEL_CFG SwChCfg);
INT32 MtCmdCr4QueryBssAcQPktNum(
	struct _RTMP_ADAPTER *pAd,
	UINT32 u4bssbitmap);

#ifdef BACKGROUND_SCAN_SUPPORT
INT32 MtCmdBgndScan(struct _RTMP_ADAPTER *pAd, struct _MT_BGND_SCAN_CFG BgScCfg);
INT32 MtCmdBgndScanNotify(struct _RTMP_ADAPTER *pAd, struct _MT_BGND_SCAN_NOTIFY BgScNotify);
#endif /* BACKGROUND_SCAN_SUPPORT */

INT32 CmdExtGeneralTestAPPWS(struct _RTMP_ADAPTER *pAd, UINT action);
#ifdef IGMP_SNOOP_SUPPORT
INT32 CmdMcastCloneEnable(struct _RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT8 band_idx, UINT8 omac_idx);
INT32 CmdMcastAllowNonMemberEnable(struct _RTMP_ADAPTER *pAd, UINT8 Msg_type, BOOLEAN Enable);

BOOLEAN CmdMcastEntryInsert(struct _RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, UINT8 Type, PUCHAR MemberAddr, PNET_DEV dev, UINT16 wcid);
BOOLEAN CmdMcastEntryDelete(struct _RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, PUCHAR MemberAddr, PNET_DEV dev, UINT16 wcid);

INT32 CmdMcastFloodingCIDR(struct _RTMP_ADAPTER *pAd, UCHAR EntryIPType, BOOLEAN bInsert, PUCHAR MacData, PUINT32 PrefixMask);
#ifdef IGMP_TVM_SUPPORT
BOOLEAN CmdSetMcastEntryAgeOut(struct _RTMP_ADAPTER *pAd, UINT8 AgeOutTime, UINT8 omac_idx);
BOOLEAN CmdGetMcastEntryTable(struct _RTMP_ADAPTER *pAd, UINT8 omac_idx, struct wifi_dev *wdev);
VOID CmdExtEventIgmpMcastTableRsp(struct cmd_msg *msg, char *Data, UINT16 Len);
#endif /* IGMP_TVM_SUPPORT */

#endif

INT32 mt_cmd_support_rate_table_ctrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 tx_mode,
	UINT8 tx_nss,
	UINT8 tx_bw,
	UINT16 *mcs_cap,
	BOOLEAN set);

INT32 mt_cmd_ra_dbg_ctrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 param_num,
	UINT32 *param);

INT32 MtCmdTxPowerSKUCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN tx_pwr_sku_en, UCHAR BandIdx);
/*TPC Algo Control*/
INT32 MtCmdTpcManCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgTpcManual);
INT32 MtCmdTpcEnableCfg(struct _RTMP_ADAPTER *pAd, BOOLEAN fgTpcEnable);
INT32 MtCmdTpcWlanIdCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgUplink, UINT8 u1EntryIdx, UINT16 u2WlanId, UINT8 u1DlTxType);
INT32 MtCmdTpcUlAlgoCtrl(struct _RTMP_ADAPTER *pAd, UINT8 u1TpcCmd, UINT8 u1ApTxPwr, UINT8 u1EntryIdx, UINT8 u1TargetRssi, UINT8 u1UPH, BOOLEAN fgMinPwrFlag);
INT32 MtCmdTpcDlAlgoCtrl(struct _RTMP_ADAPTER *pAd, UINT8 u1TpcCmd, BOOLEAN fgCmdCtrl, UINT8 u1DlTxType, CHAR DlTxPwr, UINT8 u1EntryIdx, INT16 DlTxpwrAlpha);
INT32 MtCmdTpcUlUtVarCfg(struct _RTMP_ADAPTER *pAd, UINT8 u1EntryIdx, UINT8 u1VarType, INT16 i2Value);
INT32 MtCmdTpcUlUtGo(struct _RTMP_ADAPTER *pAd, BOOLEAN fgTpcUtGo);

INT32 MtCmdTpcManTblInfo(struct _RTMP_ADAPTER *pAd, BOOLEAN fgUplink);
INT32 MtCmdTxCCKStream(struct _RTMP_ADAPTER *pAd, UINT8 u1CCKTxStream, UCHAR BandIdx);
INT32 MtCmdTxPowerPercentCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgTxPowerPercentEn, UCHAR BandIdx);
INT32 MtCmdTxPowerDropCtrl(struct _RTMP_ADAPTER *pAd, INT8 cPowerDropLevel, UCHAR BandIdx);
INT32 MtCmdTxBfBackoffCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgTxBFBackoffEn, UCHAR BandIdx);
INT32 MtCmdThermoCompCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgThermoCompEn, UCHAR BandIdx);
INT32 MtCmdTxPwrRfTxAntCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucTxAntIdx);
INT32 MtCmdTxPwrShowInfo(struct _RTMP_ADAPTER *pAd, UCHAR ucTxPowerInfoCatg, UINT8 ucBandIdx);
#ifdef WIFI_EAP_FEATURE
INT32 MtCmdInitIPICtrl(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx);
INT32 MtCmdGetIPIValue(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx);
INT32 MtCmdSetDataTxPwrOffset(struct _RTMP_ADAPTER *pAd, UINT16 WlanIdx,
		INT8 TxPwr_Offset, UINT8 BandIdx);
INT32 MtCmdSetRaTable(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx, UINT8 TblType,
		UINT8 TblIndex, UINT16 TblLength, PUCHAR Buffer);
INT32 MtCmdGetRaTblInfo(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx,
		UINT8 TblType, UINT8 TblIndex, UINT8 ReadnWrite);
#endif /* WIFI_EAP_FEATURE */
INT32 MtCmdSetEDCCAThreshold(struct _RTMP_ADAPTER *pAd, UINT8 edcca_threshold[], UINT8 BandIdx);
INT32 MtCmdSetEDCCACEnable(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx, UCHAR EDCCACtrl, UINT8 u1EDCCAStd, INT8 i1compensation);
INT32 MtCmdGetEDCCAThreshold(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx, BOOLEAN fginit);
INT32 MtCmdGetEDCCAEnable(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx);


#ifdef WIFI_GPIO_CTRL
INT MtCmdSetGpioCtrl(struct _RTMP_ADAPTER *pAd, UINT8 GpioIdx, BOOLEAN GpioEn);
INT MtCmdSetGpioVal(struct _RTMP_ADAPTER *pAd, UINT8 GpioIdx, UINT8 GpioVal);
#endif /* WIFI_GPIO_CTRL */
INT32 MtCmdTOAECalCtrl(struct _RTMP_ADAPTER *pAd, UCHAR TOAECtrl);
INT32 MtCmdMuPwrCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgMuTxPwrManEn, CHAR cMuTxPwr, UINT8 u1BandIdx);
#ifdef DATA_TXPWR_CTRL
INT32 MtCmdTxPwrDataPktCtrl(struct _RTMP_ADAPTER *pAd, IN struct _MAC_TABLE_ENTRY *pEntry, IN INT8 i1MaxBasePwr, IN UINT8 u1BandIdx);
INT32 MtCmdTxPwrMinDataPktCtrl(struct _RTMP_ADAPTER *pAd, IN INT8 i1MinBasePwr, IN UINT8 u1BandIdx);
#endif
INT32 MtCmdBFNDPATxDCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgNDPA_ManualMode, UINT8 ucNDPA_TxMode, UINT8 ucNDPA_Rate, UINT8 ucNDPA_BW, UINT8 ucNDPA_PowerOffset);
INT32 MtEPAcheck(struct _RTMP_ADAPTER *pAd);
INT32 MtATETSSITracking(struct _RTMP_ADAPTER *pAd, BOOLEAN fgEnable);
INT32 MtATEFCBWCfg(struct _RTMP_ADAPTER *pAd, BOOLEAN fgEnable);
INT32 MtTSSICompBackup(struct _RTMP_ADAPTER *pAd, BOOLEAN fgEnable);
INT32 MtTSSICompCfg(struct _RTMP_ADAPTER *pAd);
INT32 MtCmdTemperatureCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgManualMode, UINT8 ucDbdcBandIdx, CHAR cTemperature);
INT32 MtCmdThermalItemInfo(struct _RTMP_ADAPTER *pAd);
INT32 MtCmdThermalManCtrl(IN struct _RTMP_ADAPTER *pAd, IN UINT8 u1BandIdx, IN BOOLEAN fgManualMode, IN UINT8 u1ThermalAdc);
INT32 MtCmdThermalBasicInfo(IN struct _RTMP_ADAPTER *pAd, IN UINT8 u1BandIdx);
INT32 MtCmdThermalTaskCtrl(IN struct _RTMP_ADAPTER *pAd, IN UINT8 u1BandIdx, IN BOOLEAN fgTrigEn, IN UINT8 u1Thres, IN UINT32 u4FuncPtr);

INT32 MtCmdLinkTestTxCsdCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgTxCsdConfigEn, UINT8 ucDbdcBandIdx, UINT8 ucBandIdx);
INT32 MtCmdLinkTestRxCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucRxAntIdx, UINT8 ucBandIdx);
INT32 MtCmdLinkTestTxPwrCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgTxPwrConfigEn, UINT8 ucDbdcBandIdx, UINT8 ucBandIdx);
INT32 MtCmdLinkTestTxPwrUpTblCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucTxPwrUpCat, PUINT8 pucTxPwrUpValue);
INT32 MtCmdLinkTestACRCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgACRConfigEn, UINT8 ucDbdcBandIdx);
INT32 MtCmdLinkTestRcpiCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgRCPIConfigEn);
INT32 MtCmdLinkTestSeIdxCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgSeIdxConfigEn);
INT32 MtCmdLinkTestRcpiMACtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucMAParameter);

#ifdef TPC_SUPPORT
INT32 MtCmdTpcFeatureCtrl(struct _RTMP_ADAPTER *pAd, INT8 TpcPowerValue, UINT8 BandIdx, UINT8 CentralChannel);
#endif /* TPC_SUPPORT */
INT32 MtCmdATEModeCtrl(struct _RTMP_ADAPTER *pAd, UCHAR ATEMode);

INT32 CmdExtSER(struct _RTMP_ADAPTER *pAd, UINT8 action, UINT8 ser_set, UINT8 ucDbdcIdx);

INT32 CmdExtCmdCfgUpdate(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ENUM_CFG_FEATURE eFeature, VOID *param, UINT16 length);
INT32 CmdExtCmdCfgRead(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 Type, UINT32 *Value);

INT32 CmdExtRtsThenCtsRetryCnt(struct _RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT_8 u1Ac, UINT_8 u1RtsFailThenCtsRetryCnt);


#ifdef DSCP_PRI_SUPPORT
INT32 MtCmdSetDscpPri(struct _RTMP_ADAPTER *pAd, UINT8 bss_idx);

/*DSCP PRI CMD Struct, dscp range=0:63*/
typedef struct GNU_PACKED _CMD_SET_DSCP_PRI_T {
	UINT8 bss_id;
	UINT8 dscp_pri_enable;
	UINT8 aucReserved[2];
	INT8 dscpPriMap[64];
} CMD_SET_DSCP_PRI_T, *P_CMD_SET_DSCP_PRI_T;
#endif /*DSCP_PRI_SUPPORT*/

/*CR4 commond for budget control*/

#ifdef PKT_BUDGET_CTRL_SUPPORT
#define PBC_NUM_OF_PKT_BUDGET_CTRL_QUE  (5)
#define PBC_BSS_IDX_FOR_ALL             (0xFF)
#define PBC_WLAN_IDX_FOR_ALL            (0xFFFF)
#define PBC_BOUNDARY_RESET_TO_DEFAULT	(0xFFFF)

#define PBC_WMM_UP_DEFAULT_BK (900)
#define PBC_WMM_UP_DEFAULT_BE (1500)
#define PBC_WMM_UP_DEFAULT_VI (1900)
#define PBC_WMM_UP_DEFAULT_VO (1900)
#define PBC_WMM_UP_DEFAULT_MGMT (32)

#define PBC_WMM_UP_DEFAULT_BK_BAND0 (450)
#define PBC_WMM_UP_DEFAULT_BE_BAND0 (750)
#define PBC_WMM_UP_DEFAULT_VI_BAND0 (950)
#define PBC_WMM_UP_DEFAULT_VO_BAND0 (950)
#define PBC_WMM_UP_DEFAULT_MGMT_BAND0 (32)

typedef struct GNU_PACKED _CMD_PKT_BUDGET_CTRL_ENTRY_T {
	UINT16 lower_bound;
	UINT16 upper_bound;
} CMD_PKT_BUDGET_CTRL_ENTRY_T, *P_CMD_PKT_BUDGET_CTRL_ENTRY_T;

typedef struct GNU_PACKED _CMD_PKT_BUDGET_CTRL_T {
	UINT8 bss_id;
	UINT8 queue_num;
	UINT16 wlan_idx;
	UINT8 aucReserved[4];
	CMD_PKT_BUDGET_CTRL_ENTRY_T aacQue[DBDC_BAND_NUM * PBC_NUM_OF_PKT_BUDGET_CTRL_QUE];
} CMD_PKT_BUDGET_CTRL_T, *P_CMD_PKT_BUDGET_CTRL_T;

enum {
	PBC_TYPE_FIRST = 0,
	PBC_TYPE_NORMAL = PBC_TYPE_FIRST,
	PBC_TYPE_WMM,
	PBC_TYPE_END
};

enum {
	PBC_AC_BK = 0,
	PBC_AC_BE = 1,
	PBC_AC_VI = 2,
	PBC_AC_VO = 3,
	PBC_AC_MGMT = 4,
	PBC_AC_NUM = PBC_NUM_OF_PKT_BUDGET_CTRL_QUE,
};

INT32 MtCmdPktBudgetCtrl(struct _RTMP_ADAPTER *pAd, UINT8 bss_idx, UINT16 wcid, UCHAR type);

#endif /*PKT_BUDGET_CTRL_SUPPORT*/

#ifdef PS_STA_FLUSH_SUPPORT
typedef struct GNU_PACKED _CMD_PS_FLUSH_CTRL_T {
	UINT16 u2PerStaMaxMsduNum;
	UINT16 u2FlushThldTotalMsduNum;
	UINT8 fgPsSTAFlushEnable;
	UINT8 aucReserved[3];
} CMD_PS_FLUSH_CTRL_T, *P_CMD_PS_FLUSH_CTRL_T;
INT32 MtCmdPsStaFlushCtrl(struct _RTMP_ADAPTER *pAd);
#endif /*PS_STA_FLUSH_SUPPORT*/

#ifdef ZERO_LOSS_CSA_SUPPORT
typedef struct GNU_PACKED _CMD_STA_PS_Q_LIMIT_T {
	UINT16 u2Wcid;
	UINT16 u2PsQLimit;
} CMD_STA_PS_Q_LIMIT_T, *P_CMD_STA_PS_Q_LIMIT_T;
INT32 MtCmdStaPsQLimit(struct _RTMP_ADAPTER *pAd, UINT16 wcid, UINT16 PsQLimit);
#endif /*ZERO_LOSS_CSA_SUPPORT*/

INT32 MtCmdSetBWFEnable(struct _RTMP_ADAPTER *pAd, UINT8 Enable);
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_PROXY_ARP)
INT32 MtCmdHotspotInfoUpdate(struct _RTMP_ADAPTER *pAd, MT_HOTSPOT_INFO_UPDATE_T *InfoUpdateT);
#endif /* CONFIG_HOTSPOT_R2 */
#if defined(A4_CONN) || defined(MBSS_AS_WDS_AP_SUPPORT)
INT32 MtCmdSetA4Enable(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT8 Enable);
#endif /* A4_CONN */

INT SetHeraOptionDyncBW_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetHeraOptionFrequecyDup_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetHeraIara_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowHeraRuRaInfoProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowHeraMuRaInfoProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowHeraRelatedInfoProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef WIFI_MD_COEX_SUPPORT
VOID QueryLteSafeChannel(struct _RTMP_ADAPTER *pAd);
VOID MtCmdIdcInfoQuery(struct _RTMP_ADAPTER *pAd);
INT32 MtCmdIdcStateUpdate(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
#ifdef COEX_DIRECT_PATH
INT32 CoexUpdate3WireGrp(struct _RTMP_ADAPTER *pAd, VOID *pBuf, UINT32 len);
#endif
#endif

#ifdef PRE_CAL_MT7622_SUPPORT
INT32 MtCmdSetTxLpfCal_7622(struct _RTMP_ADAPTER *pAd);
INT32 MtCmdSetTxDcIqCal_7622(struct _RTMP_ADAPTER *pAd);
INT32 MtCmdSetTxDpdCal_7622(struct _RTMP_ADAPTER *pAd, UINT32 chan);
#endif /* PRE_CAL_MT7622_SUPPORT */
#ifdef PRE_CAL_MT7626_SUPPORT
INT32 MtCmdSetGroupPreCal_7626(struct _RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length);
INT32 MtCmdSetDpdFlatnessCal_7626(struct _RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length);
#endif /* PRE_CAL_MT7626_SUPPORT */
#ifdef PRE_CAL_MT7915_SUPPORT
INT32 MtCmdSetGroupPreCal_7915(struct _RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length);
INT32 MtCmdSetDpdFlatnessCal_7915(struct _RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length, BOOLEAN bSecBw80);
#endif /* PRE_CAL_MT7915_SUPPORT */
#ifdef PRE_CAL_MT7916_SUPPORT
INT32 MtCmdSetGroupPreCal_7916(struct _RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length);
INT32 MtCmdSetDpdFlatnessCal_7916(struct _RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length,  UINT32 eeprom_ofst);
#endif /* PRE_CAL_MT7986_SUPPORT */
#ifdef PRE_CAL_MT7986_SUPPORT
INT32 MtCmdSetGroupPreCal_7986(struct _RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length);
INT32 MtCmdSetDpdFlatnessCal_7986(struct _RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length, UINT32 eeprom_offset);
#endif /* PRE_CAL_MT7986_SUPPORT */
#ifdef PRE_CAL_MT7981_SUPPORT
INT32 MtCmdSetGroupPreCal_7981(struct _RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length);
INT32 MtCmdSetDpdFlatnessCal_7981(struct _RTMP_ADAPTER *pAd, UINT16 idx, UINT32 length, UINT32 eeprom_offset);
#endif /* PRE_CAL_MT7981_SUPPORT */
INT32 mt_cmd_set_rdd_ipi_hist(struct _RTMP_ADAPTER *pAd, P_EXT_CMD_RDD_IPI_HIST_T p_cmd_rdd_ipi_hist);
INT32 mt_cmd_get_rdd_ipi_hist(struct _RTMP_ADAPTER *pAd, UINT8 rdd_ipi_hist_idx, P_EXT_EVENT_RDD_IPI_HIST p_rdd_ipi_hist_rlt);

#ifdef IPI_SCAN_SUPPORT
INT32 mt_cmd_set_rdd_ipi_scan(struct _RTMP_ADAPTER *pAd, P_EXT_CMD_RDD_IPI_SCAN_T p_cmd_rdd_ipi_scan);
INT32 mt_cmd_get_rdd_ipi_scan(struct _RTMP_ADAPTER *pAd, P_EXT_EVENT_RDD_IPI_SCAN p_rdd_ipi_hist_rlt);
#endif
INT32 MtCmdPhyShapingFilterDisable(struct _RTMP_ADAPTER *pAd);
INT32 mt_cmd_get_rx_stat(struct _RTMP_ADAPTER *pAd, UCHAR band_idx, P_TESTMODE_STATISTIC_INFO p_rx_stat_rlt);
INT32 mt_cmd_set_rx_stat_user_idx(struct _RTMP_ADAPTER *pAd, UCHAR band_idx, UINT16 user_idx);
INT32 mt_cmd_set_rxv_ctrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgRxvEnable);
INT32 mt_cmd_set_rxv_ru_ctrl(struct _RTMP_ADAPTER *pAd, UINT8 rxv_ru_idx);
INT32 mt_cmd_get_rx_stat_band(struct _RTMP_ADAPTER *pAd, UCHAR band_idx, TESTMODE_STATISTIC_INFO_BAND *rx_stat_band);
INT32 mt_cmd_get_rx_stat_path(struct _RTMP_ADAPTER *pAd, UCHAR path_idx, UCHAR band_idx, TESTMODE_STATISTIC_INFO_PATH *rx_stat_path);
INT32 mt_cmd_get_rx_stat_user(struct _RTMP_ADAPTER *pAd, UCHAR user_idx, TESTMODE_STATISTIC_INFO_USER *rx_stat_user);
INT32 mt_cmd_get_rx_stat_comm(struct _RTMP_ADAPTER *pAd, TESTMODE_STATISTIC_INFO_COMM *rx_stat_comm);

/*NOTE: the definition need to sync with bora code.*/

/*
 * https://www.iana.org/assignments/ikev2-parameters/ikev2-parameters.xhtml
 * Transform Type 4 - Diffie-Hellman Group Transform IDs
 */
typedef enum _ENUM_ECC_DH_GROUP_ID_T {
	ECDH_GROUP_ID_256BIT = 19,
	ECDH_GROUP_ID_384BIT = 20,
	ECDH_GROUP_ID_521BIT = 21,
	ECDH_GROUP_ID_192BIT = 25,
	ECDH_GROUP_ID_224BIT = 26
} ENUM_ECC_DH_GROUP_ID_T;

typedef enum _ENUM_ECDH_LENGTH_T {
	ECDH_LENGTH_256BIT = 32,
	ECDH_LENGTH_384BIT = 48,
	ECDH_LENGTH_521BIT = 64,
	ECDH_LENGTH_192BIT = 24,
	ECDH_LENGTH_224BIT = 28,
} ENUM_ECDH_LENGTH_T;

struct _EC_GROUP_LENGTH_MAP_T {
	ENUM_ECC_DH_GROUP_ID_T group_id;
	ENUM_ECDH_LENGTH_T element_len;
};

/* ECC operation tags */
typedef enum _ENUM_ECC_OP_HANDLE_T {
	ECC_OP_CAL_GROUP_POINT   = 0x00,
	ECC_OP_MAX
} ENUM_ECC_OP_HANDLE_T;

typedef struct _CMD_ECC_OP_T {
	/** DWORD_0 - Common Part */
	UINT8  ucCmdVer;
	UINT8  aucPadding0[1];
	/** Cmd size including common part and body */
	UINT16 u2CmdLen;

	UINT8 eEccOperation; /*0: calculate point in group, 1: further extension*/
	UINT8 ucGroupID;
	UINT8 ucDataLength; /*if operation is 1, it means the length of total */
	UINT8 ucDataType; /*0: only scalar(DG mode in HW), 1: scalar and point(x and y)(DQ mode in HW) */
	UINT8 ucEccCmdId;
	UINT8 aucReserved[3];
	UINT8 aucBuffer[0]; /*data*/
} CMD_ECC_OP_T, *P_CMD_ECC_OP_T;

#define ECC_CAL_DG_MODE 0 /*provide scalar, the base point is the default generator of group predefinition.*/
#define ECC_CAL_DQ_MODE 1 /*provide scalar, and point. */

typedef struct _EVENT_ECC_RES_T {
#define ECC_GROUP_20_ECDH_LEN 48
	/** DWORD_0 - Common Part */
	UINT8  ucEvtVer;
	UINT8  aucPadding0[1];
	/** Cmd size including common part and body */
	UINT16 u2EvtLen;

	UINT8 ucDqxDataLength;
	UINT8 ucDqyDataLength;
	UINT8 ucEccCmdId;
	UINT8 ucIsResFail;
	UINT8 aucDqxBuffer[ECC_GROUP_20_ECDH_LEN];
	UINT8 aucDqyBuffer[ECC_GROUP_20_ECDH_LEN];
} EVENT_ECC_RES_T, *P_EVENT_ECC_RES_T;

INT32 cmd_calculate_ecc(struct _RTMP_ADAPTER *ad, UINT32 oper, UINT32 group, UINT8 *scalar, UINT8 *point_x, UINT8 *point_y);

INT32
MtCmdThermalProtectEnable(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	UINT8 protection_type,
	UINT8 trigger_type,
	INT32 trigger_temp,
	INT32 restore_temp,
	UINT16 recheck_time);

INT32
MtCmdThermalProtectDisable(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	UINT8 protection_type,
	UINT8 trigger_type);

INT32
MtCmdThermalProtectDutyCfg(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	UINT8 level_idx,
	UINT8 duty);

INT32
MtCmdThermalProtectInfo(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	struct THERMAL_PROTECT_MECH_INFO *info_buf);

INT32
MtCmdThermalProtectDutyInfo(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	struct THERMAL_PROTECT_DUTY_INFO *info_buf);

INT32
MtCmdThermalProtectStateAct(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	UINT8 protect_type,
	UINT8 trig_type,
	UINT8 state);

#ifdef AIR_MONITOR
INT32 MtCmdSmeshConfigSet(struct _RTMP_ADAPTER *pAd, UCHAR *pdata, P_EXT_EVENT_SMESH_T prSmeshResult);
#endif /* AIR_MONITOR */

INT32 MtCmdGetAllStaStats(struct _RTMP_ADAPTER *pAd, UINT8 subevent_type);

enum {
	BSS_INFO_SET_VLAN_ID,
	BSS_INFO_SET_VLAN_PRIORITY,
	BSS_INFO_SET_VLAN_MAX_NUM
};

typedef struct _CMD_VLAN_INFO_UPDATE_T {
	/** DWORD_0 - Common Part */
	UINT8  ucCmdVer;
	UINT8  aucPadding0[1];
	/** Cmd size including common part and body */
	UINT16 u2CmdLen;

	UINT8 ucBandIdx;
	UINT8 ucOwnMacIdx;
	UINT16 u2Value;
	UINT8 ucOpCode;
	UINT8 aucReserved[3];
} CMD_VLAN_INFO_UPDATE_T, *P_CMD_VLAN_INFO_UPDATE_T;
#ifdef VLAN_SUPPORT
INT32 cmd_vlan_update(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT8 omac_idx, UINT8 op_code, UINT16 value);
#endif
#ifdef PKTLOSS_CHK
typedef enum _PKTLOSS_CHK_ENUM_CMD {
	PKTLOSS_CHK_SET_DUMP_INFO = 0,
	PKTLOSS_CHK_SET_DUMP_SHORT = 1,
	PKTLOSS_CHK_SET_RESET = 2,
	PKTLOSS_CHK_SET_SRC_IP = 3,
	PKTLOSS_CHK_SET_DEST_IP = 4,
	PKTLOSS_CHK_SET_PORT = 5,
	PKTLOSS_CHK_SET_OFFSET = 6,
	PKTLOSS_CHK_SET_IS_SEQ_SIGNED = 7,
	PKTLOSS_CHK_SET_IS_SEQ_CROSS_ZERO = 8,
	PKTLOSS_CHK_SET_SEQ_MASK = 9,
	PKTLOSS_CHK_SET_TS_THRESHOLD = 10,
	PKTLOSS_CHK_SET_CTRL_FLAG = 20,
	PKTLOSS_CHK_SET_HEX_DUMP = 21,
	PKTLOSS_CHK_SET_CONTINUE_HEX_DUMP = 22,
	PKTLOSS_CHK_SET_TXS_LOG_ENABLE = 27,
	PKTLOSS_CHK_SET_IPERF_CASE = 28,
	PKTLOSS_CHK_SET_RTP_CASE = 29,
	PKTLOSS_CHK_SET_ENABLE = 30,
	PKTLOSS_CHK_SET_MAX
} PKTLOSS_CHK_ENUM_CMD;

typedef enum _PKTLOSS_CHK_ENUM_CTRL {
	PKTLOSS_CHK_BY_PORT = 0,
	PKTLOSS_CHK_BY_IP = 1,
	PKTLOSS_CHK_BY_SEQ = 2,
	PKTLOSS_CHK_BY_TS = 3,
	PKTLOSS_CHK_HEX_DUMP = 4,
	PKTLOSS_CHK_CONTINUE_HEX_DUMP = 5
} PKTLOSS_CHK_ENUM_CTRL;
#endif

struct EXT_CMD_SET_RTS_THEN_CTS_RETRY {
	UINT_16  u2Wcid;
	UINT_8   u1Ac;
	UINT_8   u1RtsFailThenCtsRetryCnt;
};
#endif /* __MT_CMD_H__ */
