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
	andes_core.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __ANDES_CORE_H__
#define __ANDES_CORE_H__

#include "mcu.h"

#ifdef LINUX
#ifndef WORKQUEUE_BH
#include <linux/interrupt.h>
#endif
#endif /* LINUX */

#ifdef MT_MAC
#include "mcu/andes_mt.h"
#endif
#include "mcu/mt_cmd.h"
#include "mcu/fwdl.h"

#ifdef WIFI_UNIFIED_COMMAND
#include "hw_ctrl/cmm_fw_uni_cmd.h"
#include "hw_ctrl/cmm_fw_uni_event.h"
#endif /* WIFI_UNIFIED_COMMAND */

#include "common/link_list.h"
#include "wifi_sys_notify.h"


struct _RTMP_ADAPTER;
struct _RXFCE_INFO;
struct _BANK_RF_REG_PAIR;
struct _R_M_W_REG;
struct _RF_R_M_W_REG;

enum {
	MCU_INIT,
	MCU_TX_HALT,
	MCU_RX_HALT,
};

enum {
	CMD_ACK,
};

enum cmd_msg_error_type {
	error_tx_kickout_fail,
	error_tx_timeout_fail,
	error_rx_receive_fail,
};

struct MCU_CTRL {
	UINT8 cmd_seq;
	ULONG flags; /* Use long, becasue we want to do atomic bit operation */
#ifdef LINUX
#ifndef WORKQUEUE_BH
	RTMP_NET_TASK_STRUCT cmd_msg_task;
#else
	struct tasklet_struct cmd_msg_task;
#endif
#endif /* LINUX */
	NDIS_SPIN_LOCK txq_lock;
	DL_LIST txq;
	NDIS_SPIN_LOCK rxq_lock;
	DL_LIST rxq;
	NDIS_SPIN_LOCK ackq_lock;
	DL_LIST ackq;
	NDIS_SPIN_LOCK kickq_lock;
	DL_LIST kickq;
	NDIS_SPIN_LOCK tx_doneq_lock;
	DL_LIST tx_doneq;
	NDIS_SPIN_LOCK rx_doneq_lock;
	DL_LIST rx_doneq;
	NDIS_SPIN_LOCK msg_lock;
	ULONG tx_kickout_fail_count;
	ULONG tx_timeout_fail_count;
	ULONG rx_receive_fail_count;
	ULONG alloc_cmd_msg;
	ULONG free_cmd_msg;
	BOOLEAN power_on;
	BOOLEAN dpd_on;
	UINT8 RxStream0, RxStream1;
	struct fwdl_ctrl fwdl_ctrl;
#ifdef DBG_STARVATION
	struct starv_dbg_block block;
#endif /*DBG_STARVATION*/
	struct notify_head fw_cmd_notify_head;
	struct _RTMP_ADAPTER *ad;
};

struct fw_cmd_notify_info {
	void *msg;
	UINT32 msg_len;
	struct _RTMP_ADAPTER *ad;
};

enum {
	FW_CMD_NOTIFY_PRIORITY_DEFAULT = 0,
	FW_CMD_NOTIFY_PRIORITY_PROXY,
};



struct cmd_msg;
typedef VOID(*MSG_EVENT_HANDLER)(struct _RTMP_ADAPTER *ad, char *payload, UINT16 payload_len);

struct cmd_msg_cb {
	struct cmd_msg *msg;
};

#define CMD_MSG_CB(pkt) ((struct cmd_msg_cb *)(GET_OS_PKT_CB(pkt)))
#define CMD_MSG_RETRANSMIT_TIMES 3
#define CMD_MSG_TIMEOUT 3000
#if defined(MT7626) && defined(RTMP_RBUS_SUPPORT)
#undef CMD_MSG_TIMEOUT
#define CMD_MSG_TIMEOUT 6000
#endif
#define MT_CMD_TX_HOOK AndesSendCmdMsg

enum BW_SETTING {
	BW20 = 1,
	BW40 = 2,
	BW10 = 4,
	BW80 = 8,
};

enum mcu_skb_state {
	ILLEAGAL = 0,
	MCU_CMD_START,
	MCU_CMD_DONE,
	MCU_RSP_START,
	MCU_RSP_DONE,
	MCU_RSP_CLEANUP,
	UNLINK_START,
};

#define SDIO_END_PADDING 4

struct mcu_skb_data {
	enum mcu_skb_state state;
};

#ifdef LED_CONTROL_SUPPORT
#ifdef RT_BIG_ENDIAN
typedef union _LED_ENHANCE {
	struct {
		UINT32 tx_over_blink:1;
		UINT32 reverse_polarity:1;
		UINT32 rsv:4;
		UINT32 tx_blink:2;
		UINT32 on_time:8;
		UINT32 off_time:8;
		UINT32 idx:8;
	} field;
	UINT32 word;
} LED_ENHANCE;
#else
typedef union _LED_ENHANCE {
	struct {
		UINT32 idx:8;
		UINT32 off_time:8;
		UINT32 on_time:8;
		UINT32 tx_blink:2;
		UINT32 rsv:4;
		UINT32 reverse_polarity:1;
		UINT32 tx_over_blink:1;
	} field;
	UINT32 word;
} LED_ENHANCE;
#endif /* RT_BIG_ENDIAN */
#endif /*LED_CONTROL_SUPPORT*/

VOID AndesCtrlInit(struct _RTMP_ADAPTER *pAd);
VOID AndesCtrlExit(struct _RTMP_ADAPTER *pAd);
INT32 AndesSendCmdMsg(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg);
BOOLEAN IsInbandCmdProcessing(struct _RTMP_ADAPTER *pAd);
VOID AndesCmdMsgBh(unsigned long param);
VOID AndesBhSchedule(struct _RTMP_ADAPTER *pAd);

struct cmd_msg *AndesAllocCmdMsg(struct _RTMP_ADAPTER *pAd, unsigned int length);
VOID AndesInitCmdMsg(struct cmd_msg *msg, CMD_ATTRIBUTE attr);

VOID AndesAppendCmdMsg(struct cmd_msg *msg, char *data, unsigned int len);
VOID AndesAppendHeadCmdMsg(struct cmd_msg *msg, char *data, unsigned int len);
VOID AndesFreeCmdMsg(struct cmd_msg *msg);
VOID AndesQueueTailCmdMsg(DL_LIST *list, struct cmd_msg *msg,
						  enum cmd_msg_state state);
VOID AndesIncErrorCount(struct MCU_CTRL *ctl, enum cmd_msg_error_type type);
VOID _AndesUnlinkCmdMsg(struct cmd_msg *msg, DL_LIST *list);
VOID AndesUnlinkCmdMsg(struct cmd_msg *msg, DL_LIST *list);
struct cmd_msg *AndesDequeueCmdMsg(struct MCU_CTRL *ctl, DL_LIST *list);
VOID AndesQueueHeadCmdMsg(DL_LIST *list, struct cmd_msg *msg,
						  enum cmd_msg_state state);
VOID AndesQueueHeadCmdMsg(DL_LIST *list, struct cmd_msg *msg,
						  enum cmd_msg_state state);
UINT32 AndesQueueLen(struct MCU_CTRL *ctl, DL_LIST *list);
NDIS_SPIN_LOCK *AndesGetSpinLock(struct MCU_CTRL *ctl, DL_LIST *list);
UCHAR AndesGetCmdMsgSeq(struct _RTMP_ADAPTER *ad);
VOID _AndesQueueTailCmdMsg(DL_LIST *list, struct cmd_msg *msg, enum cmd_msg_state state);
VOID AndesRxProcessCmdMsg(struct _RTMP_ADAPTER *pAd, struct cmd_msg *rx_msg);
struct cmd_msg *AndesAllocCmdMsgGe(struct _RTMP_ADAPTER *ad, unsigned int length, BOOLEAN bOldCmdFmt);

INT register_fw_cmd_notifier(struct _RTMP_ADAPTER *ad, struct notify_entry *ne);

INT unregister_fw_cmd_notifier(struct _RTMP_ADAPTER *ad, struct notify_entry *ne);

INT call_fw_cmd_notifieriers(INT val, struct _RTMP_ADAPTER *ad, void *msg);

#endif /* __ANDES_CORE_H__ */

