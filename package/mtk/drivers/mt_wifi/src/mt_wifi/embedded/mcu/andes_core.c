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
	andes_core.c
*/
#include "rt_config.h"

#ifdef DBG_STARVATION
static void andes_starv_timeout_handle(struct starv_dbg *starv, struct starv_log_entry *entry)
{
	struct cmd_msg *cmd = container_of(starv, struct cmd_msg, starv);
	struct MCU_CTRL *ctrl = starv->block->priv;
	struct starv_log_basic *log = NULL;

	os_alloc_mem(NULL, (UCHAR **) &log, sizeof(struct starv_log_basic));
	if (!log)
		return;

	log->qsize = DlListLen(&ctrl->txq);
	log->id = cmd->attr.ext_type;
	entry->log = log;
}

static void andes_starv_block_init(struct starv_log *ctrl, struct MCU_CTRL *mcu_ctrl)
{
	struct starv_dbg_block *block = &mcu_ctrl->block;

	strncpy(block->name, "andes", sizeof(block->name));
	block->priv = mcu_ctrl;
	block->ctrl = ctrl;
	block->timeout = 1000;
	block->timeout_fn = andes_starv_timeout_handle;
	block->log_fn = starv_timeout_log_basic;
	register_starv_block(block);
}
#endif /*DBG_STARVATION*/

struct cmd_msg *AndesAllocCmdMsgGe(RTMP_ADAPTER *ad, unsigned int length, BOOLEAN bOldCmdFmt)
{
	struct cmd_msg *msg = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	PNDIS_PACKET net_pkt = NULL;
	UINT8 cmd_header_len = cap->cmd_header_len;
	INT32 AllocateSize = 0;

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support && (bOldCmdFmt == FALSE))
		cmd_header_len = cap->uni_cmd_header_len;
#endif /* WIFI_UNIFIED_COMMAND */
	AllocateSize = cmd_header_len + length + cap->cmd_padding_len;

	net_pkt = RTMP_AllocateFragPacketBuffer(ad, AllocateSize);

	if (!net_pkt) {
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "can not allocate net_pkt\n");
		goto error0;
	}

	OS_PKT_RESERVE(net_pkt, cmd_header_len);
	os_alloc_mem(NULL, (PUCHAR *)&msg, sizeof(*msg));

	if (!msg) {
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "can not allocate cmd msg\n");
		goto error1;
	}

	CMD_MSG_CB(net_pkt)->msg = msg;
	os_zero_mem(msg, sizeof(*msg));
	msg->priv = (void *)ad;
	msg->net_pkt = net_pkt;
	ctl->alloc_cmd_msg++;
	return msg;

error1:
	RTMPFreeNdisPacket(ad, net_pkt);
error0:
	return NULL;
}

struct cmd_msg *AndesAllocCmdMsg(RTMP_ADAPTER *ad, unsigned int length)
{
#ifdef WF_RESET_SUPPORT
	if (ad->wf_reset_in_progress == TRUE)
		return 0;
#endif

	return hif_mcu_alloc_msg(ad, length, TRUE);
}

VOID AndesInitCmdMsg(struct cmd_msg *msg, CMD_ATTRIBUTE attr)
{
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:: mcu_dest(%d), cmd_type(0x%x), ExtCmdType(0x%x)\n",
			  __func__, attr.mcu_dest, attr.type, attr.ext_type);
	SET_CMD_MSG_PORT_QUEUE_ID(msg, GetRealPortQueueID(msg, attr.type));
	SET_CMD_MSG_MCU_DEST(msg, attr.mcu_dest);
	SET_CMD_MSG_TYPE(msg, attr.type);
	SET_CMD_MSG_CTRL_FLAGS(msg, attr.ctrl.flags);
	SET_CMD_MSG_EXT_TYPE(msg, attr.ext_type);
	SET_CMD_MSG_CTRL_RSP_WAIT_MS_TIME(msg, attr.ctrl.wait_ms_time);

	if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
		RTMP_OS_INIT_COMPLETION(&msg->ack_done);

	if (IS_CMD_MSG_NEED_RETRY_FLAG_SET(msg))
		SET_CMD_MSG_RETRY_TIMES(msg, CMD_MSG_RETRANSMIT_TIMES);
	else
		SET_CMD_MSG_RETRY_TIMES(msg, 0);

	SET_CMD_MSG_CTRL_RSP_EXPECT_SIZE(msg, attr.ctrl.expect_size);
	SET_CMD_MSG_RSP_WB_BUF_IN_CALBK(msg, attr.rsp.wb_buf_in_calbk);
	SET_CMD_MSG_RSP_HANDLER(msg, attr.rsp.handler);

#ifdef WIFI_UNIFIED_COMMAND
	msg->total_frag = 0;
	msg->frag_num = 0;
	msg->seq = 0;
#endif /* WIFI_UNIFIED_COMMAND */
}

VOID AndesAppendCmdMsg(struct cmd_msg *msg, char *data, unsigned int len)
{
	PNDIS_PACKET net_pkt;

	if (!msg)
		return;

	net_pkt = msg->net_pkt;

	if (data)
		memcpy(OS_PKT_TAIL_BUF_EXTEND(net_pkt, len), data, len);
}


VOID AndesAppendHeadCmdMsg(struct cmd_msg *msg, char *data, unsigned int len)
{
	PNDIS_PACKET net_pkt = msg->net_pkt;

	if (data)
		memcpy(OS_PKT_HEAD_BUF_EXTEND(net_pkt, len), data, len);
}

VOID AndesFreeCmdMsg(struct cmd_msg *msg)
{
	RTMP_ADAPTER *ad = NULL;
	ULONG flags = 0;
	struct MCU_CTRL *ctl = NULL;

	if (!msg)
		return;

	ad = (RTMP_ADAPTER *)(msg->priv);

	if (!ad) {
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "pAd is null\n");
		goto free_memory;
	}

	ctl = &ad->MCUCtrl;

	if (!ctl) {
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "MCUCtrl is null\n");
		goto free_memory;
	}

	if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
		RTMP_OS_EXIT_COMPLETION(&msg->ack_done);

	if (msg->retry_pkt)
		RTMPFreeNdisPacket(ad, msg->retry_pkt);

	OS_SPIN_LOCK_IRQSAVE(&ctl->msg_lock, &flags);
	ctl->free_cmd_msg++;
	OS_SPIN_UNLOCK_IRQRESTORE(&ctl->msg_lock, &flags);
free_memory:
	os_free_mem(msg);
}

VOID AndesForceFreeCmdMsg(struct cmd_msg *msg)
{
	RTMP_ADAPTER *ad = NULL;
	struct MCU_CTRL *ctl = NULL;
	ULONG flags = 0;

	if (!msg)
		return;

	ad = (RTMP_ADAPTER *)(msg->priv);

	if (!ad) {
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "pAd is null\n");
		goto free_memory;
	}

	ctl = &ad->MCUCtrl;

	if (!ctl) {
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "MCUCtrl is null\n");
		goto free_memory;
	}

	if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
		RTMP_OS_EXIT_COMPLETION(&msg->ack_done);

	OS_SPIN_LOCK_IRQSAVE(&ctl->msg_lock, &flags);
	ctl->free_cmd_msg++;
	OS_SPIN_UNLOCK_IRQRESTORE(&ctl->msg_lock, &flags);
free_memory:

	if (ad && msg->net_pkt)
		RTMPFreeNdisPacket(ad, msg->net_pkt);

	os_free_mem(msg);
}


BOOLEAN IsInbandCmdProcessing(RTMP_ADAPTER *ad)
{
	BOOLEAN ret = 0;
	return ret;
}


UCHAR GetCmdRspNum(RTMP_ADAPTER *ad)
{
	UCHAR Num = 0;
	return Num;
}


VOID AndesIncErrorCount(struct MCU_CTRL *ctl, enum cmd_msg_error_type type)
{
	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		switch (type) {
		case error_tx_kickout_fail:
			ctl->tx_kickout_fail_count++;
			break;

		case error_tx_timeout_fail:
			ctl->tx_timeout_fail_count++;
			break;

		case error_rx_receive_fail:
			ctl->rx_receive_fail_count++;
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "unknown cmd_msg_error_type(%d)\n",
					  type);
		}
	}
}

NDIS_SPIN_LOCK *AndesGetSpinLock(struct MCU_CTRL *ctl, DL_LIST *list)
{
	NDIS_SPIN_LOCK *lock = NULL;

	if (list == &ctl->txq)
		lock = &ctl->txq_lock;
	else if (list == &ctl->rxq)
		lock = &ctl->rxq_lock;
	else if (list == &ctl->ackq)
		lock = &ctl->ackq_lock;
	else if (list == &ctl->kickq)
		lock = &ctl->kickq_lock;
	else if (list == &ctl->tx_doneq)
		lock = &ctl->tx_doneq_lock;
	else if (list == &ctl->rx_doneq)
		lock = &ctl->rx_doneq_lock;

	else {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "list pointer = %p\n", list);
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "txq = %p, rxq = %p, ackq = %p, kickq = %p, tx_doneq = %p, rx_doneq = %p\n",
				  &ctl->txq, &ctl->rxq, &ctl->ackq, &ctl->kickq,
				  &ctl->tx_doneq, &ctl->rx_doneq);
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "illegal list\n");
	}

	return lock;
}


UCHAR AndesGetCmdMsgSeq(struct _RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	struct cmd_msg *msg;
	unsigned long flags;
	UINT8 msg_seq;

	RTMP_SPIN_LOCK_IRQSAVE(&ctl->ackq_lock, &flags);
get_seq:
	ctl->cmd_seq >= 0xf ? ctl->cmd_seq = 1 : ctl->cmd_seq++;
	DlListForEach(msg, &ctl->ackq, struct cmd_msg, list) {
		if (msg->seq == ctl->cmd_seq) {
			MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "command(seq: %d) is still running\n", ctl->cmd_seq);
			MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "command response nums = %d\n", GetCmdRspNum(ad));
			goto get_seq;
		}
	}
	msg_seq = ctl->cmd_seq;
	RTMP_SPIN_UNLOCK_IRQRESTORE(&ctl->ackq_lock, &flags);
	return msg_seq;
}


VOID _AndesQueueTailCmdMsg(DL_LIST *list, struct cmd_msg *msg, enum cmd_msg_state state)
{
	msg->state = state;
	DlListAddTail(list, &msg->list);
}


VOID AndesQueueTailCmdMsg(DL_LIST *list, struct cmd_msg *msg, enum cmd_msg_state state)
{
	unsigned long flags;
	NDIS_SPIN_LOCK *lock;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = AndesGetSpinLock(ctl, list);

	if (lock) {
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
		_AndesQueueTailCmdMsg(list, msg, state);
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
	}
}


static VOID _AndesQueueHeadCmdMsg(DL_LIST *list, struct cmd_msg *msg, enum cmd_msg_state state)
{
	msg->state = state;
	DlListAdd(list, &msg->list);
}


VOID AndesQueueHeadCmdMsg(DL_LIST *list, struct cmd_msg *msg, enum cmd_msg_state state)
{
	unsigned long flags;
	NDIS_SPIN_LOCK *lock;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = AndesGetSpinLock(ctl, list);

	if (lock) {
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
		_AndesQueueHeadCmdMsg(list, msg, state);
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
	}
}

UINT32 AndesQueueLen(struct MCU_CTRL *ctl, DL_LIST *list)
{
	UINT32 qlen = 0;
	unsigned long flags;
	NDIS_SPIN_LOCK *lock;

	lock = AndesGetSpinLock(ctl, list);

	if (lock) {
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
		qlen = DlListLen(list);
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
	}

	return qlen;
}

static VOID AndesQueueInit(struct MCU_CTRL *ctl, DL_LIST *list)
{
	unsigned long flags;
	NDIS_SPIN_LOCK *lock;

	lock = AndesGetSpinLock(ctl, list);

	if (lock) {
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
		DlListInit(list);
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
	}
}


VOID _AndesUnlinkCmdMsg(struct cmd_msg *msg, DL_LIST *list)
{
	if ((!msg) || (!msg->list.Next) || (!msg->list.Prev))
		return;

	DlListDel(&msg->list);
}


VOID AndesUnlinkCmdMsg(struct cmd_msg *msg, DL_LIST *list)
{
	unsigned long flags;
	NDIS_SPIN_LOCK *lock;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = AndesGetSpinLock(ctl, list);

	if (lock) {
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
		_AndesUnlinkCmdMsg(msg, list);
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
	}
}


static struct cmd_msg *_AndesDequeueCmdMsg(DL_LIST *list)
{
	struct cmd_msg *msg;

	msg = DlListFirst(list, struct cmd_msg, list);
	_AndesUnlinkCmdMsg(msg, list);
	return msg;
}


struct cmd_msg *AndesDequeueCmdMsg(struct MCU_CTRL *ctl, DL_LIST *list)
{
	unsigned long flags;
	struct cmd_msg *msg = NULL;
	NDIS_SPIN_LOCK *lock;

	lock = AndesGetSpinLock(ctl, list);

	if (lock) {
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
		msg = _AndesDequeueCmdMsg(list);
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
	}

	return msg;
}

VOID AndesRxProcessCmdMsg(RTMP_ADAPTER *ad, struct cmd_msg *rx_msg)
{
	hif_rx_event_process(ad, rx_msg);
}


VOID AndesCmdMsgBh(unsigned long param)
{
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)param;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	struct cmd_msg *msg = NULL;

	while ((msg = AndesDequeueCmdMsg(ctl, &ctl->rx_doneq))) {
		AndesRxProcessCmdMsg(ad, msg);
		AndesFreeCmdMsg(msg);
	}

	while ((msg = AndesDequeueCmdMsg(ctl, &ctl->tx_doneq))) {
		switch (msg->state) {
		case tx_done:
		case tx_kickout_fail:
		case tx_timeout_fail:
#ifdef DBG_STARVATION
			starv_dbg_put(&msg->starv);
#endif /*DBG_STARVATION*/
			break;

		default:
			MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "unknown msg state(%d)\n", msg->state);
			break;
		}
		AndesFreeCmdMsg(msg);
	}

	if (OS_TEST_BIT(MCU_INIT, &ctl->flags))
		AndesBhSchedule(ad);
}

VOID AndesBhSchedule(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;

	if (((AndesQueueLen(ctl, &ctl->rx_doneq) > 0)
		 || (AndesQueueLen(ctl, &ctl->tx_doneq) > 0))
		&& OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
#ifndef WORKQUEUE_BH
		RTMP_NET_TASK_DATA_ASSIGN(&ctl->cmd_msg_task, (unsigned long)(ad));
		RTMP_OS_TASKLET_SCHE(&ctl->cmd_msg_task);
#else
		tasklet_hi_schedule(&ctl->cmd_msg_task);
#endif
	}
}


VOID AndesCleanupCmdMsg(RTMP_ADAPTER *ad, DL_LIST *list)
{
	unsigned long flags;
	struct cmd_msg *msg, *msg_tmp;
	NDIS_SPIN_LOCK *lock;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = AndesGetSpinLock(ctl, list);
	if (lock)
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	DlListForEachSafe(msg, msg_tmp, list, struct cmd_msg, list) {
		_AndesUnlinkCmdMsg(msg, list);

		/*If need wait, clean up need to trigger complete for andes send cmd to free msg*/
		if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
			RTMP_OS_EXIT_COMPLETION(&msg->ack_done);
		else
			AndesFreeCmdMsg(msg);
	}
	DlListInit(list);
	if (lock)
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
}

VOID AndesCtrlInit(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		/*general init*/
		RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
		ctl->cmd_seq = 0;
		RTMP_OS_TASKLET_INIT(ad, &ctl->cmd_msg_task, AndesCmdMsgBh, (unsigned long)ad);
		NdisAllocateSpinLock(ad, &ctl->txq_lock);
		AndesQueueInit(ctl, &ctl->txq);
		NdisAllocateSpinLock(ad, &ctl->rxq_lock);
		AndesQueueInit(ctl, &ctl->rxq);
		NdisAllocateSpinLock(ad, &ctl->ackq_lock);
		AndesQueueInit(ctl, &ctl->ackq);
		NdisAllocateSpinLock(ad, &ctl->kickq_lock);
		AndesQueueInit(ctl, &ctl->kickq);
		NdisAllocateSpinLock(ad, &ctl->tx_doneq_lock);
		AndesQueueInit(ctl, &ctl->tx_doneq);
		NdisAllocateSpinLock(ad, &ctl->rx_doneq_lock);
		AndesQueueInit(ctl, &ctl->rx_doneq);
		ctl->tx_kickout_fail_count = 0;
		ctl->tx_timeout_fail_count = 0;
		ctl->rx_receive_fail_count = 0;
		ctl->alloc_cmd_msg = 0;
		ctl->free_cmd_msg = 0;
		ctl->ad = ad;
		/*hif specific init*/
		hif_mcu_init(ad->hdev_ctrl);
		OS_SET_BIT(MCU_INIT, &ctl->flags);
	}

	ctl->power_on = FALSE;
	ctl->dpd_on = FALSE;
	ctl->RxStream0 = 0;
	ctl->RxStream1 = 0;
	NdisAllocateSpinLock(ad, &ctl->msg_lock);
#ifdef DBG_STARVATION
	andes_starv_block_init(&ad->starv_log_ctrl, ctl);
#endif /*DBG_STARVATION*/
}

VOID AndesCtrlExit(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
#ifdef DBG_STARVATION
		unregister_starv_block(&ctl->block);
#endif /*DBG_STARVATION*/
		/*hif specific*/
		hif_mcu_exit(ad->hdev_ctrl);
		/*generic setting*/
		AndesBhSchedule(ad);
		OS_CLEAR_BIT(MCU_INIT, &ctl->flags);
		RTMP_OS_TASKLET_KILL(&ctl->cmd_msg_task);
		AndesCleanupCmdMsg(ad, &ctl->txq);
		NdisFreeSpinLock(&ctl->txq_lock);
		AndesCleanupCmdMsg(ad, &ctl->ackq);
		NdisFreeSpinLock(&ctl->ackq_lock);
		AndesCleanupCmdMsg(ad, &ctl->rxq);
		NdisFreeSpinLock(&ctl->rxq_lock);
		AndesCleanupCmdMsg(ad, &ctl->kickq);
		NdisFreeSpinLock(&ctl->kickq_lock);
		AndesCleanupCmdMsg(ad, &ctl->tx_doneq);
		NdisFreeSpinLock(&ctl->tx_doneq_lock);
		AndesCleanupCmdMsg(ad, &ctl->rx_doneq);
		NdisFreeSpinLock(&ctl->rx_doneq_lock);
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "tx_kickout_fail_count = %ld\n", ctl->tx_kickout_fail_count);
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "tx_timeout_fail_count = %ld\n", ctl->tx_timeout_fail_count);
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "rx_receive_fail_count = %ld\n", ctl->rx_receive_fail_count);
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "alloc_cmd_msg = %ld\n", ctl->alloc_cmd_msg);
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "free_cmd_msg = %ld\n", ctl->free_cmd_msg);
	}

	ctl->power_on = FALSE;
	ctl->dpd_on = FALSE;
}

static INT32 AndesDequeueAndKickOutCmdMsgs(RTMP_ADAPTER *ad)
{
	struct cmd_msg *msg = NULL;
	VOID *net_pkt = NULL;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	int ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(ad->hdev_ctrl);
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	while ((msg = AndesDequeueCmdMsg(ctl, &ctl->txq)) != NULL) {
		if (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD)     ||
			RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST)         ||
			RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_SUSPEND)) {
			if (!IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
				AndesForceFreeCmdMsg(msg);
			else if ((msg->retry_times > 1) && (NULL != msg->net_pkt) && (NULL == msg->retry_pkt)) {
				OS_PKT_COPY(RTPKT_TO_OSPKT(msg->net_pkt), msg->retry_pkt);
				RTMPFreeNdisPacket(ad, msg->net_pkt);
				msg->net_pkt = NULL;
			}
			MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				"skip msg for flag=%lx\n", ad->Flags);
			continue;
		}

		net_pkt = (VOID *)msg->net_pkt;

		if (msg->state != tx_retransmit) {
#ifdef WIFI_UNIFIED_COMMAND
			if (IS_CMD_MSG_UNI_CMD_FLAG_SET(msg)) {
				if (arch_ops->fill_uni_cmd_header != NULL)
					arch_ops->fill_uni_cmd_header(ad, msg, net_pkt);
			} else
#endif /* WIFI_UNIFIED_COMMAND */
			{
				if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
					msg->seq = AndesGetCmdMsgSeq(ad);
				else
					msg->seq = 0;
				if (arch_ops->fill_cmd_header != NULL)
					arch_ops->fill_cmd_header(ad, msg, net_pkt);
			}
		}

		if (msg->retry_times > 1) {
			OS_PKT_COPY(RTPKT_TO_OSPKT(net_pkt), msg->retry_pkt);
			if (msg->retry_pkt == NULL) {
				msg->retry_times = 0;
				MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory failed!\n");
			}
		}

		if (chip_ops->kick_out_cmd_msg != NULL)
			ret = chip_ops->kick_out_cmd_msg(ad, msg);

		if (ret) {
			MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "kick out msg fail\n");

			if (ret == NDIS_STATUS_FAILURE) {
				if (msg->retry_pkt) {
					RTMPFreeNdisPacket(ad, msg->retry_pkt);
					msg->retry_pkt = NULL;
				}
				AndesForceFreeCmdMsg(msg);
			}

			break;
		}
	}

	AndesBhSchedule(ad);
	return ret;
}

static INT32 AndesWaitForCompleteTimeout(struct cmd_msg *msg, ULONG timeout)
{
	ULONG ret = 0;
	ULONG expire = timeout ?
				   RTMPMsecsToJiffies(timeout) : RTMPMsecsToJiffies(CMD_MSG_TIMEOUT);
	ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&msg->ack_done, expire);
	return ret;
}

#ifdef WF_RESET_SUPPORT
#ifdef CONFIG_CONNINFRA_SUPPORT
enum consys_drv_type {
        CONNDRV_TYPE_BT = 0,
        CONNDRV_TYPE_FM = 1,
        CONNDRV_TYPE_GPS = 2,
        CONNDRV_TYPE_WIFI = 3,
        CONNDRV_TYPE_CONNINFRA = 4,
        CONNDRV_TYPE_MAX
};
int conninfra_pwr_on(enum consys_drv_type drv_type);
int conninfra_pwr_off(enum consys_drv_type drv_type);
#endif /* CONFIG_CONNINFRA_SUPPORT */
#endif

INT32 AndesSendCmdMsg(PRTMP_ADAPTER ad, struct cmd_msg *msg)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	BOOLEAN is_cmd_need_wait = IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg);
	int ret = NDIS_STATUS_SUCCESS;
	static BOOLEAN is_dump_stack = FALSE;
#ifdef CONFIG_AP_SUPPORT
	int exp_type = 0;
#endif

	if (in_interrupt() && IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg)) {
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "BUG: called from invalid context\n");
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Command type = %x, Extension command type = %x\n",
				  msg->attr.type, msg->attr.ext_type);
		if (!is_dump_stack) {
			dump_stack();
			is_dump_stack = TRUE;
		}
		AndesForceFreeCmdMsg(msg);
		return NDIS_STATUS_FAILURE;
	}

	if (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD)     ||
		RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST)         ||
		RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_SUSPEND)) {
		if (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD)) {
			MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Could not send in band command due to diablefRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD\n");
		} else if (RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
			MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Could not send in band command due to fRTMP_ADAPTER_NIC_NOT_EXIST\n");
		} else if (RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_SUSPEND)) {
			MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Could not send in band command due to fRTMP_ADAPTER_SUSPEND\n");
		}

		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"Command type = %x, Extension command type = %x\n",
				  msg->attr.type, msg->attr.ext_type);

		AndesForceFreeCmdMsg(msg);
		return NDIS_STATUS_FAILURE;
	}

#ifdef ERR_RECOVERY
	/* prohibit to sned fw commnad during SER period */
	if (IsStopingPdma(&ad->ErrRecoveryCtl)) {
		MTWF_DBG(ad, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
			"(SER Period): Command type = %x, Extension command type = %x\n",
			 msg->attr.type, msg->attr.ext_type);
		AndesForceFreeCmdMsg(msg);
		return NDIS_STATUS_FAILURE;
	}
#endif /* ERR_RECOVERY */

#ifdef WIFI_MODULE_DVT
	if (mdvt_block_command(ad, msg) == TRUE) {
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"MDVT Block,Command type = %x, Extension command type = %x\n",
			 msg->attr.type, msg->attr.ext_type);
		AndesForceFreeCmdMsg(msg);
		return NDIS_STATUS_FAILURE;
	}
#endif

#ifdef DBG_STARVATION
	starv_dbg_init(&ctl->block, &msg->starv);
	starv_dbg_get(&msg->starv);
#endif /*DBG_STARVATION*/
	AndesQueueTailCmdMsg(&ctl->txq, msg, tx_start);
retransmit:
	if (AndesDequeueAndKickOutCmdMsgs(ad) != NDIS_STATUS_SUCCESS)
		goto bailout;

#ifdef WF_RESET_SUPPORT
	if(ad->wf_reset_in_progress == TRUE)
		goto bailout;
#endif

	/* Wait for response */
	if (is_cmd_need_wait) {
		enum cmd_msg_state state = 0;
		ULONG IsComplete;

		IsComplete = AndesWaitForCompleteTimeout(msg, msg->attr.ctrl.wait_ms_time);

		if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
			/*If need wait, clean up will trigger complete for here to free msg*/
			AndesFreeCmdMsg(msg);
			MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				"fail for MCU_INIT\n");
			goto bailout;
		}

		if (!IsComplete) {
			P_FWCMD_TIMEOUT_RECORD pToRec = NULL;
			BOOLEAN bDump = TRUE;

			ret = NDIS_STATUS_FAILURE;

			/* record timeout info */
			pToRec = &ad->FwCmdTimeoutRecord[ad->FwCmdTimeoutCnt % FW_CMD_TO_RECORD_CNT];
			NdisGetSystemUpTime(&pToRec->timestamp);
			pToRec->type = 		msg->attr.type;
			pToRec->ext_type = 	msg->attr.ext_type;
			pToRec->seq = 		msg->seq;
			pToRec->state = 	msg->state;
			ad->FwCmdTimeoutCnt++;
#ifdef WF_RESET_SUPPORT
			ad->FwCmdTimeoutcheckCnt++;
#endif
			/* check timeout print count */
			if ((ad->FwCmdTimeoutCnt > ad->FwCmdTimeoutPrintCnt) &&
				(ad->FwCmdTimeoutPrintCnt != 0))
				bDump = FALSE;

#ifdef ERR_RECOVERY
			/* check timeout (possibly) caused by SER */
			{
				UINT32 Highpart = 0;
				UINT32 Lowpart = 0;

				/* extra timeout allowance 3 sec for cmd timeout */
				#define SER_TIMEOUT_ALLOWANCE 3000

				AsicGetTsfTime(ad, &Highpart, &Lowpart, HW_BSSID_0);
				if ((Lowpart - ad->HwCtrl.ser_times[SER_TIME_ID_T0]) <
					 ((CMD_MSG_TIMEOUT + SER_TIMEOUT_ALLOWANCE) * 1000)) {
					MTWF_DBG(ad, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
							  "FWCmdTimeout(SER Period): command (%x), ext_cmd_type (%x), seq(%d), delta(%dus)\n",
							  msg->attr.type, msg->attr.ext_type,
							  msg->seq,
							  Lowpart - ad->HwCtrl.ser_times[SER_TIME_ID_T0]);
					bDump = FALSE;
				}
			}
#endif

			if (bDump) {
				MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "FWCmdTimeout: command (%x), ext_cmd_type (%x), seq(%d), timeout(%dms)\n",
						  msg->attr.type, msg->attr.ext_type,
						  msg->seq,
						  (msg->attr.ctrl.wait_ms_time == 0) ?
						  CMD_MSG_TIMEOUT : msg->attr.ctrl.wait_ms_time);
				MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "pAd->Flags  = 0x%.8lx\n", ad->Flags);
				MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "txq qlen = %d\n", AndesQueueLen(ctl, &ctl->txq));
				MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "rxq qlen = %d\n", AndesQueueLen(ctl, &ctl->rxq));
				MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "kickq qlen = %d\n", AndesQueueLen(ctl, &ctl->kickq));
				MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "ackq qlen = %d\n", AndesQueueLen(ctl, &ctl->ackq));
				MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "tx_doneq.qlen = %d\n", AndesQueueLen(ctl, &ctl->tx_doneq));
				MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "rx_done qlen = %d\n", AndesQueueLen(ctl, &ctl->rx_doneq));
			}

			if (msg->state == wait_cmd_out_and_ack) {
				/*unlink acq recycle msg*/
				hif_mcu_unlink_ackq(msg);
			} else if (msg->state == wait_ack)
				AndesUnlinkCmdMsg(msg, &ctl->ackq);

			AndesIncErrorCount(ctl, error_tx_timeout_fail);
			state = tx_timeout_fail;

			if (msg->retry_times > 0)
				msg->retry_times--;

			if (bDump) {
				MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "msg state = %d\n", msg->state);
				MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "msg->retry_times = %d\n", msg->retry_times);
				MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "FwCmdTimeoutCnt  = %d\n", ad->FwCmdTimeoutCnt);

#ifdef CONFIG_AP_SUPPORT
				/* FW status check and core_dump to file */
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
				if (IS_MT7915(ad) || IS_MT7986(ad) || IS_MT7916(ad) || IS_MT7981(ad))
					exp_type = ChkExceptionType(ad);
#endif

				if (ad->FwCmdTimeoutCnt < FW_CMD_TO_DBG_INFO_PRINT_CNT) {
					if (exp_type == 0) {
						MTWF_PRINT("FW is normal\n");
						Show_FwDbgInfo_Proc(ad, NULL);
						show_trinfo_proc(ad, NULL);
					} else {
						MTWF_PRINT("FW is exception\n\n\n\n");
						Show_FwDbgInfo_Proc(ad, NULL);
						show_trinfo_proc(ad, NULL);

						RtmpusecDelay(5000);
						if (ad->bIsBeenDumped == FALSE) {
							ad->bIsBeenDumped = TRUE;
							Show_CoreDump_Proc(ad, NULL);
						}

#ifdef WF_RESET_SUPPORT
						RTMP_CHIP_OP *chip_op = hc_get_chip_ops(ad->hdev_ctrl);
#ifdef CONFIG_CONNINFRA_SUPPORT
						conninfra_pwr_off(CONNDRV_TYPE_WIFI);
						mdelay(15);
						conninfra_pwr_on(CONNDRV_TYPE_WIFI);
						mdelay(15);
#endif /* CONFIG_CONNINFRA_SUPPORT */
						if (IS_MT7916(ad)) {
							UINT32 macVal;
							RTMP_IO_READ32(ad->hdev_ctrl, 0x70002600, &macVal);
							macVal |= 0x1;
							RTMP_IO_WRITE32(ad->hdev_ctrl, 0x70002600, macVal);
							mdelay(15);
							macVal &= 0xfffffffe;
							RTMP_IO_WRITE32(ad->hdev_ctrl, 0x70002600, macVal);
							mdelay(15);
						}

						ad->wf_reset_wm_count++;
						if (chip_op->do_wifi_reset)
							chip_op->do_wifi_reset(ad);
#endif


					}
				}
#endif /* CONFIG_AP_SUPPORT */

				ASSERT(FALSE);
#ifdef WF_RESET_SUPPORT
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

				if (ops->heart_beat_check)
					ops->heart_beat_check(ad);
#endif

				if (ad->FwCmdTimeoutCnt == ad->FwCmdTimeoutPrintCnt) {
					struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);
					UCHAR BandIdx;

					if (ops->hw_auto_debug_trigger) {
						for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++)
							ops->hw_auto_debug_trigger(ad, BandIdx, ENUM_AHDBUG_L1_WFDMA, 0);
					}

					MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							 "!!! FWCmdTimeout stop dumping... !!!\n");
				}
			}
		} else {
			if (msg->state == tx_kickout_fail) {
				state = tx_kickout_fail;
				msg->retry_times--;
			} else {
				if (msg->state == wait_cmd_out_and_ack) {
					hif_mcu_unlink_ackq(msg);
				} else if (msg->state == wait_ack)
					AndesUnlinkCmdMsg(msg, &ctl->ackq);

				state = tx_done;
				msg->retry_times = 0;
			}
#ifdef WF_RESET_SUPPORT
			ad->FwCmdTimeoutcheckCnt = 0;
#endif
		}

		if (is_cmd_need_wait && (msg->retry_times > 0)) {
			RTMP_OS_EXIT_COMPLETION(&msg->ack_done);
			RTMP_OS_INIT_COMPLETION(&msg->ack_done);
			msg->net_pkt = msg->retry_pkt;
			msg->retry_pkt = NULL;
			state = tx_retransmit;
			AndesQueueHeadCmdMsg(&ctl->txq, msg, state);
			goto retransmit;
		} else {
			if (msg->attr.ext_type == EXT_CMD_STAREC_UPDATE) {
				/* Only StaRec update command read FW's response to minimize the impact.
				 FW's response will become the final return value.
				*/
				ret = msg->cmd_return_status;
			}

			MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					 "%s: msg state = %d\n", __func__, state);
			/* msg will be free after enqueuing to tx_doneq. So msg is not able to pass FW's response to caller. */
			AndesQueueTailCmdMsg(&ctl->tx_doneq, msg, state);
		}
	}
bailout:
	return ret;
}

INT32 MtCmdSendMsg(PRTMP_ADAPTER ad, struct cmd_msg *msg)
{
	INT32 ret = 0;

	ret = AndesSendCmdMsg(ad, msg);
	return ret;
}


/*export function*/
/*
*
*/
INT register_fw_cmd_notifier(struct _RTMP_ADAPTER *ad, struct notify_entry *ne)
{
	INT ret;

	ret = mt_notify_chain_register(&ad->MCUCtrl.fw_cmd_notify_head, ne);

	return ret;
}
EXPORT_SYMBOL(register_fw_cmd_notifier);
/*
*
*/
INT unregister_fw_cmd_notifier(struct _RTMP_ADAPTER *ad, struct notify_entry *ne)
{
	INT ret;

	ret = mt_notify_chain_unregister(&ad->MCUCtrl.fw_cmd_notify_head, ne);
	return ret;
}
EXPORT_SYMBOL(unregister_fw_cmd_notifier);

/*
*
*/
INT call_fw_cmd_notifieriers(INT val, struct _RTMP_ADAPTER *ad, void *msg)
{
	INT ret;
	struct fw_cmd_notify_info info;
	PACKET_INFO pkt_info;
	UCHAR *msg_buf;
	UINT32 msg_len;

	if (!msg) {
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "msg is null (val=0x%x)\n", val);
		return NOTIFY_STAT_FAILURE;
	}

	RTMP_QueryPacketInfo(msg, &pkt_info, &msg_buf, &msg_len);

	info.msg = msg_buf;
	info.msg_len = msg_len;
	info.ad = ad;

	/*traversal caller for traffic notify chain*/
	ret = mt_notify_call_chain(&ad->MCUCtrl.fw_cmd_notify_head, val, &info);
	return ret;
}
