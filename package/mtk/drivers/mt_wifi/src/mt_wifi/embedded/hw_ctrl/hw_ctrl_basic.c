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
	hw_ctrl_basic.c
*/
#include "rt_config.h"
#include "hw_ctrl_basic.h"

extern HW_CMD_TABLE_T *HwCmdTable[];

/*==========================================================/
 //	Basic Command API implement															/
/==========================================================*/
static inline HwCmdHdlr HwCtrlValidCmd(HwCmdQElmt *CmdQelmt)
{
	UINT32 CmdType =  CmdQelmt->type;
	UINT32 CmdIndex = CmdQelmt->command;
	SHORT CurIndex = 0;
	HwCmdHdlr Handler = NULL;
	HW_CMD_TABLE_T  *pHwTargetTable = NULL;

	if (CmdType >= HWCMD_TYPE_END) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "CMD TPYE(%u) OOB error! HWCMD_TYPE_END %u\n",
				  CmdType, HWCMD_TYPE_END);
		return NULL;
	}

	if (CmdIndex >= HWCMD_ID_END) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "CMD ID(%u) OOB error! HWCMD_ID_END %u\n",
				  CmdIndex, HWCMD_ID_END);
		return NULL;
	}

	pHwTargetTable = HwCmdTable[CmdType];

	if (!pHwTargetTable) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "No HwCmdTable entry for this CMD %u Type %u\n",
				  CmdIndex, CmdType);
		return NULL;
	}

	CurIndex = 0;

	do {
		if (pHwTargetTable[CurIndex].CmdID == CmdIndex) {
			Handler = pHwTargetTable[CurIndex].CmdHdlr;
			pHwTargetTable[CurIndex].RfCnt++;
			NdisGetSystemUpTime(&(pHwTargetTable[CurIndex].LastRfTime));
			break;
		}

		CurIndex++;
	} while (pHwTargetTable[CurIndex].CmdHdlr != NULL);

	if (Handler == NULL) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "No corresponding CMDHdlr for this CMD %u Type %u\n",
				  CmdIndex, CmdType);
	}

	return Handler;
}

static VOID HwCtrlDequeueCmd(HwCmdQ *cmdq, HwCmdQElmt **pcmdqelmt)
{
	HW_CMD_TABLE_T *pHwCmdTable = NULL;
	UINT32 j = 0;
	*pcmdqelmt = cmdq->head;

	if (*pcmdqelmt != NULL) {
		cmdq->head = cmdq->head->next;
		cmdq->size--;

		if (cmdq->size == 0)
			cmdq->tail = NULL;

		/* Decrease the Total */
		if (cmdq->util_flag & HW_CMDQ_UTIL_WAIT_TIME_ADJ) {
			if ((*pcmdqelmt)->NeedWait) {
				/* printk("%s: ID=%u, wait time=%u\n", __func__, (*pcmdqelmt)->command, ((*pcmdqelmt)->WaitTime)); */
				if (cmdq->TotalWaitTime >= ((*pcmdqelmt)->WaitTime))
					cmdq->TotalWaitTime -= ((*pcmdqelmt)->WaitTime);

				if (cmdq->TotalWaitCnt >= 1)
					cmdq->TotalWaitCnt--;
			}
		} else {
			cmdq->TotalWaitTime = 0;
			cmdq->TotalWaitCnt = 0;
		}

		/* Decrease  per ID's wait time */
		if (cmdq->util_flag & HW_CMDQ_UTIL_TIME) {
			if ((*pcmdqelmt)->type < HWCMD_TYPE_END) {
				pHwCmdTable = HwCmdTable[(*pcmdqelmt)->type];
				if (pHwCmdTable && ((*pcmdqelmt)->command < HWCMD_ID_END)) {
					j = 0;
					/* traverse to the match ID */
					while (pHwCmdTable[j].CmdID != HWCMD_ID_END) {
						if (pHwCmdTable[j].CmdID == (*pcmdqelmt)->command) {
							pHwCmdTable[j].TotalWaitTime -= ((*pcmdqelmt)->WaitTime);
							break;
						}
						j++;
					}
				}
			}
		}
	}
}

static VOID free_hwcmd(os_kref *ref)
{
	struct _HwCmdQElmt *cmd = container_of(ref, struct _HwCmdQElmt, refcnt);

	if (cmd->NeedWait)
		RTMP_OS_EXIT_COMPLETION(&cmd->ack_done);

	if (cmd->buffer != NULL) {
		os_free_mem(cmd->buffer);
		cmd->buffer = NULL;
	}

	os_free_mem(cmd);
}

static VOID HwCtrlCmdHandler(RTMP_ADAPTER *pAd)
{
	PHwCmdQElmt	cmdqelmt;
	NTSTATUS		ntStatus;
	HwCmdHdlr		Handler = NULL;
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	UINT32			process_cnt = 0;

	while (pAd && pHwCtrl->HwCtrlQ.size > 0) {

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
			!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
			MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "System halted, exit HwCtrlCmdHandler!(HwCtrlQ.size = %d)\n",
					  pHwCtrl->HwCtrlQ.size);
			break;
		}

		/* For worst case, avoid process HwCtrlQ too long which cause RCU_sched stall */
		process_cnt++;
		if ((!in_interrupt()) && (process_cnt >= HWCTRL_QUE_SCH)) {/*process_cnt-16*/
			process_cnt = 0;
			OS_SCHEDULE();
		}

		NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
		HwCtrlDequeueCmd(&pHwCtrl->HwCtrlQ, &cmdqelmt);
		NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

		if (cmdqelmt == NULL)
			break;

		if (FwOwnSts(pAd))
			goto free_cmd;


		Handler = HwCtrlValidCmd(cmdqelmt);

		if (Handler) {
			ntStatus = Handler(pAd, cmdqelmt);

			if (cmdqelmt->CallbackFun)
				cmdqelmt->CallbackFun(pAd, cmdqelmt->CallbackArgs);
		}
#ifdef DBG_STARVATION
		starv_dbg_put(&cmdqelmt->starv);
#endif /*DBG_STARVATION*/
free_cmd:
		/*complete*/
		if (cmdqelmt->NeedWait)
			RTMP_OS_COMPLETE(&cmdqelmt->ack_done);

		os_kref_put(&cmdqelmt->refcnt, free_hwcmd);
	}	/* end of while */
}

static INT HwCtrlThread(ULONG Context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *pTask;
	HwCmdQElmt	*pCmdQElmt = NULL;
	HW_CTRL_T *pHwCtrl;
	int status;

	status = 0;
	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

	if (pAd == NULL)
		return 0;

	pHwCtrl = &pAd->HwCtrl;
	RtmpOSTaskCustomize(pTask);
	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	pHwCtrl->HwCtrlQ.CmdQState = RTMP_TASK_STAT_RUNNING;
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

	while (pHwCtrl->HwCtrlQ.CmdQState == RTMP_TASK_STAT_RUNNING) {
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		if (pHwCtrl->HwCtrlQ.CmdQState == RTMP_TASK_STAT_STOPED)
			break;

#ifdef KERNEL_RPS_ADJUST
		if (pAd->ixia_mode_ctl.rps_mask & APPLY_NEED_BH_APPLY_FLAG)
			apply_proc_rps_setting_bh(pAd);
#endif

		/*every time check command formate event*/
		HwCtrlCmdHandler(pAd);

		pHwCtrl->TotalCnt++;
	}

	/* Clear the CmdQElements. */
	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	pHwCtrl->HwCtrlQ.CmdQState = RTMP_TASK_STAT_STOPED;

	while (pHwCtrl->HwCtrlQ.size) {
		HwCtrlDequeueCmd(&pHwCtrl->HwCtrlQ, &pCmdQElmt);

		if (pCmdQElmt) {
#ifdef DBG_STARVATION
			starv_dbg_put(&pCmdQElmt->starv);
#endif /*DBG_STARVATION*/
			/*complete*/
			if (pCmdQElmt->NeedWait)
				RTMP_OS_COMPLETE(&pCmdQElmt->ack_done);

			os_kref_put(&pCmdQElmt->refcnt, free_hwcmd);
		}
	}

	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<---\n");
	RtmpOSTaskNotifyToExit(pTask);
	return 0;
}


#ifdef ERR_RECOVERY
static INT ser_ctrl_task(ULONG context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *task;
	HW_CTRL_T *hw_ctrl;
	int status = 0;

	task = (RTMP_OS_TASK *)context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(task);

	if (pAd == NULL)
		return 0;

	hw_ctrl = &pAd->HwCtrl;
	RtmpOSTaskCustomize(task);
	NdisAcquireSpinLock(&hw_ctrl->ser_lock);
	hw_ctrl->ser_func_state = RTMP_TASK_STAT_RUNNING;
	NdisReleaseSpinLock(&hw_ctrl->ser_lock);

	while (task && !RTMP_OS_TASK_IS_KILLED(task)) {
		if (RtmpOSTaskWait(pAd, task, &status) == FALSE)
			break;

		HwRecoveryFromError(pAd);
	}

	NdisAcquireSpinLock(&hw_ctrl->ser_lock);
	hw_ctrl->ser_func_state = RTMP_TASK_STAT_UNKNOWN;
	NdisReleaseSpinLock(&hw_ctrl->ser_lock);
	status = RtmpOSTaskNotifyToExit(task);
	return status;
}


INT ser_init(RTMP_ADAPTER *pAd)
{
	INT Status = 0;
	HW_CTRL_T *hw_ctrl = &pAd->HwCtrl;
	RTMP_OS_TASK *task = &hw_ctrl->ser_task;

	NdisAllocateSpinLock(pAd, &hw_ctrl->ser_lock);
	hw_ctrl->ser_func_state = RTMP_TASK_STAT_INITED;
	RTMP_OS_TASK_INIT(task, "ser_task", pAd);
	Status = RtmpOSTaskAttach(task, ser_ctrl_task, (ULONG)task);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "%s: unable to start\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}


INT ser_exit(RTMP_ADAPTER *pAd)
{
	INT32 ret;
	HW_CTRL_T *hw_ctrl = &pAd->HwCtrl;

	/*kill task*/
	ret = RtmpOSTaskKill(&hw_ctrl->ser_task);
	NdisFreeSpinLock(&hw_ctrl->ser_lock);
	return ret;
}
#endif /* ERR_RECOVERY */

#ifdef SCAN_RADAR_COEX_SUPPORT
static INT radar_ctrl_task(ULONG context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *task;
	int status = 0;

	task = (RTMP_OS_TASK *)context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(task);
	if (pAd == NULL) {
		pAd->CommonCfg.DfsParameter.is_radar_emu = FALSE;
		pAd->radar_handling = FALSE;
		return 0;
	}
	RtmpOSTaskCustomize(task);
	while (task && !RTMP_OS_TASK_IS_KILLED(task)) {
		if (RtmpOSTaskWait(pAd, task, &status) == FALSE)
			break;
		UpdateRddReportHandle(pAd);
	}
	pAd->CommonCfg.DfsParameter.is_radar_emu = FALSE;
	pAd->radar_handling = FALSE;
	status = RtmpOSTaskNotifyToExit(task);
	return status;
}

INT radar_task_init(RTMP_ADAPTER *pAd)
{
	INT Status = 0;
	RTMP_OS_TASK *task = &pAd->radar_task;

	RTMP_OS_TASK_INIT(task, "radar_task", pAd);
	Status = RtmpOSTaskAttach(task, radar_ctrl_task, (ULONG)task);
	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"%s: unable to start %s\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev), __func__);
		return NDIS_STATUS_FAILURE;
	}
	return TRUE;
}

INT radar_task_exit(RTMP_ADAPTER *pAd)
{
	INT32 ret;
	/*kill task*/
	ret = RtmpOSTaskKill(&pAd->radar_task);
	return ret;
}
#endif /* SCAN_RADAR_COEX_SUPPORT */

#ifdef MTK_FE_RESET_RECOVER

static int mtk_fe_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct mtk_notifier_block *nb = (struct mtk_notifier_block *)this;
	RTMP_ADAPTER *pAd = nb->priv;
	P_ERR_RECOVERY_CTRL_T pErrRecoveryCtrl = &pAd->ErrRecoveryCtl;
#ifdef WIFI_UNIFIED_COMMAND
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return NOTIFY_DONE;

	switch (event) {
	case MTK_FE_START_RESET:
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"receive fe reset(hang) event, trigger ser by FE\n");
		atomic_set(&pErrRecoveryCtrl->notify_fe, 1);
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_TRIGGER, SER_SET_L1_RECOVER, DBDC_BAND0);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L1_RECOVER, DBDC_BAND0);
		break;
	case MTK_FE_RESET_DONE:
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"receive fe reset done event, continue SER\n");
		complete(&pErrRecoveryCtrl->fe_reset_done);
		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}

unsigned int mtk_fe_reset_notifier_init(struct _RTMP_ADAPTER  *pAd)
{
	int err = NDIS_STATUS_FAILURE;
	P_ERR_RECOVERY_CTRL_T pErrRecoveryCtrl = &pAd->ErrRecoveryCtl;
	struct mtk_notifier_block *mtk_nb = &pErrRecoveryCtrl->mtk_nb;

	init_completion(&pErrRecoveryCtrl->fe_reset_done);
	atomic_set(&pErrRecoveryCtrl->notify_fe, 0);

	mtk_nb->priv = pAd;
	mtk_nb->nb.notifier_call = mtk_fe_event;

	err = register_netdevice_notifier(&mtk_nb->nb);

	if (err)
		return err;

	rtnl_lock();
	call_netdevice_notifiers(MTK_WIFI_CHIP_ONLINE, pAd->net_dev);
	rtnl_unlock();

	return err;
}

void  mtk_fe_reset_notifier_exit(struct _RTMP_ADAPTER  *pAd)
{
	P_ERR_RECOVERY_CTRL_T pErrRecoveryCtrl = &pAd->ErrRecoveryCtl;
	struct mtk_notifier_block *mtk_nb = &pErrRecoveryCtrl->mtk_nb;

	rtnl_lock();
	call_netdevice_notifiers(MTK_WIFI_CHIP_OFFLINE, pAd->net_dev);
	rtnl_unlock();
	unregister_netdevice_notifier(&mtk_nb->nb);
}
#endif

#ifdef WF_RESET_SUPPORT
static INT wf_reset_ctrl_task(ULONG context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *task;
	int status = 0;

	task = (RTMP_OS_TASK *)context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(task);

	if (pAd == NULL)
		return 0;

	RtmpOSTaskCustomize(task);
	pAd->wf_reset_state = RTMP_TASK_STAT_RUNNING;

	while (task && !RTMP_OS_TASK_IS_KILLED(task)) {
		if (RtmpOSTaskWait(pAd, task, &status) == FALSE)
			break;

		wf_reset_func(pAd);
	}

	pAd->wf_reset_state = RTMP_TASK_STAT_UNKNOWN;
	status = RtmpOSTaskNotifyToExit(task);
	return status;
}

UINT32 wf_reset_init(RTMP_ADAPTER *pAd)
{
	INT Status = 0;
	RTMP_OS_TASK *task = &pAd->wf_reset_thread;

	pAd->wf_reset_state = RTMP_TASK_STAT_INITED;
	pAd->wf_reset_wm_count = 0;
	pAd->wf_reset_wa_count = 0;
	pAd->wf_reset_wo_count = 0;
	RTMP_OS_TASK_INIT(task, "wf_reset_task", pAd);
	Status = RtmpOSTaskAttach(task, wf_reset_ctrl_task, (ULONG)task);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "%s: unable to start wf reset task\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
		return NDIS_STATUS_FAILURE;
	}

	return TRUE;
}

UINT32 wf_reset_exit(RTMP_ADAPTER *pAd)
{
	INT32 ret;

	pAd->wf_reset_wm_count = 0;
	pAd->wf_reset_wa_count = 0;
	pAd->wf_reset_wo_count = 0;
	/*kill task*/
	ret = RtmpOSTaskKill(&pAd->wf_reset_thread);
	return ret;
}
#endif

#ifdef DBG_STARVATION
static void hwctrl_starv_timeout_handle(struct starv_dbg *starv, struct starv_log_entry *entry)
{
	struct _HwCmdQElmt *cmd = container_of(starv, struct _HwCmdQElmt, starv);
	struct _HW_CTRL_T *hw_ctrl = starv->block->priv;
	struct starv_log_basic *log = NULL;

	os_alloc_mem(NULL, (UCHAR **) &log, sizeof(struct starv_log_basic));
	if (log) {
		log->qsize = hw_ctrl->HwCtrlQ.size;
		log->id = cmd->command;
		entry->log = log;
	}
}

static void hwctrl_starv_block_init(struct starv_log *ctrl, struct _HW_CTRL_T *hw_ctrl)
{
	struct starv_dbg_block *block = &hw_ctrl->block;

	strncpy(block->name, "hwctrl", sizeof(block->name));
	block->priv = hw_ctrl;
	block->ctrl = ctrl;
	block->timeout = 100;
	block->timeout_fn = hwctrl_starv_timeout_handle;
	block->log_fn = starv_timeout_log_basic;
	register_starv_block(block);
}

#endif /*DBG_STARVATION*/

/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
UINT32 HwCtrlInit(RTMP_ADAPTER *pAd)
{
	INT Status;
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	HwCmdQ *cmdq = &pHwCtrl->HwCtrlQ;
	RTMP_OS_TASK *pTask = &pHwCtrl->HwCtrlTask;

#ifdef DBG_STARVATION
	hwctrl_starv_block_init(&pAd->starv_log_ctrl, pHwCtrl);
#endif /*DBG_STARVATION*/
	NdisAllocateSpinLock(pAd, &pHwCtrl->HwCtrlQLock);
	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	cmdq->head = NULL;
	cmdq->tail = NULL;
	cmdq->size = 0;
	cmdq->CmdQState = RTMP_TASK_STAT_INITED;

	cmdq->util_flag = (HW_CMDQ_UTIL_TIME | HW_CMDQ_UTIL_CONDITION_DROP);

	/* for debug erevy 5 seconds : once full & reach the time, may dump the Queue list */
	cmdq->LastQfullTime = 0;

	/*for record & adjust the new comming wait cmd's wait time */
	if (cmdq->util_flag & HW_CMDQ_UTIL_WAIT_TIME_ADJ) {
		cmdq->TotalWaitCnt = 0;
		cmdq->TotalWaitTime = 0;
	}

	/* init the default max to MAX_LEN_OF_HWCTRL_QUEUE */
	cmdq->max_size = MAX_LEN_OF_HWCTRL_QUEUE;

	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
	pHwCtrl->TotalCnt = 0;
	pTask = &pHwCtrl->HwCtrlTask;
	RTMP_OS_TASK_INIT(pTask, "HwCtrlTask", pAd);
	Status = RtmpOSTaskAttach(pTask, HwCtrlThread, (ULONG)pTask);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: unable to start\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
		return NDIS_STATUS_FAILURE;
	}

#ifdef ERR_RECOVERY
	Status = ser_init(pAd);
	if (Status == NDIS_STATUS_FAILURE)
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "ser_init() return NDIS_STATUS_FAILURE\n");
#endif /* ERR_RECOVERY */
#ifdef SCAN_RADAR_COEX_SUPPORT
	Status = radar_task_init(pAd);
#endif /* SCAN_RADAR_COEX_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}


VOID HwCtrlExit(RTMP_ADAPTER *pAd)
{
	INT32 ret;
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	HWCTRL_OP *hwctrl_ops = &pHwCtrl->hwctrl_ops;
	/*flush all queued command*/
	HwCtrlCmdHandler(pAd);
	/*kill task*/
	ret = RtmpOSTaskKill(&pHwCtrl->HwCtrlTask);
	NdisFreeSpinLock(&pHwCtrl->HwCtrlQLock);
	hwctrl_ops->wifi_sys_open = NULL;
	hwctrl_ops->wifi_sys_close = NULL;
	hwctrl_ops->wifi_sys_link_up = NULL;
	hwctrl_ops->wifi_sys_link_down = NULL;
	hwctrl_ops->wifi_sys_connt_act = NULL;
	hwctrl_ops->wifi_sys_disconnt_act = NULL;
	hwctrl_ops->wifi_sys_peer_update = NULL;
#ifdef ERR_RECOVERY
	ret = ser_exit(pAd);
#endif /* ERR_RECOVERY */
#ifdef SCAN_RADAR_COEX_SUPPORT
	ret = radar_task_exit(pAd);
#endif /* SCAN_RADAR_COEX_SUPPORT */
#ifdef DBG_STARVATION
	unregister_starv_block(&pHwCtrl->block);
#endif /*DBG_STARVATION*/
}

NDIS_STATUS HwCtrlEnqueueCmd(
	RTMP_ADAPTER *pAd,
	HW_CTRL_TXD HwCtrlTxd)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PHwCmdQElmt	cmdqelmt = NULL;
	PHwCmdQ	cmdq = NULL;
	UINT32 wait_time = 0, adjust_total_wait_time = 0, j = 0;
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	BOOLEAN bDump = FALSE, bDrop = FALSE;
	HW_CMD_TABLE_T *pHwCmdTable = NULL;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "---> NIC is not exist!!\n");
		return NDIS_STATUS_FAILURE;
	}

	status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt, sizeof(HwCmdQElmt));

	if (cmdqelmt == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "---> os_alloc_mem failed!!\n");
		return NDIS_STATUS_RESOURCES;
	}

	NdisZeroMemory(cmdqelmt, sizeof(HwCmdQElmt));
	/*initial lock*/
	NdisAllocateSpinLock(NULL, &cmdqelmt->lock);
	/*creat wait */
	cmdqelmt->NeedWait = HwCtrlTxd.NeedWait;
	/*initial stravation dbg*/
#ifdef DBG_STARVATION
	starv_dbg_init(&pHwCtrl->block, &cmdqelmt->starv);
#endif /*DBG_STARVATION*/

	if (HwCtrlTxd.NeedWait)
		RTMP_OS_INIT_COMPLETION(&cmdqelmt->ack_done);

	if (HwCtrlTxd.InformationBufferLength > 0) {
		status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt->buffer, HwCtrlTxd.InformationBufferLength);
		if (cmdqelmt->buffer == NULL) {
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "---> os_alloc_mem failed!!\n");
			status =  NDIS_STATUS_RESOURCES;
			goto end;
		}
		/*initial buffer*/
		os_move_mem(cmdqelmt->buffer, HwCtrlTxd.pInformationBuffer, HwCtrlTxd.InformationBufferLength);
		cmdqelmt->bufferlength = HwCtrlTxd.InformationBufferLength;
	}
	/*initial cmd element*/
	cmdqelmt->command = HwCtrlTxd.CmdId;
	cmdqelmt->type = HwCtrlTxd.CmdType;
	cmdqelmt->RspBuffer = HwCtrlTxd.pRespBuffer;
	cmdqelmt->RspBufferLen = HwCtrlTxd.RespBufferLength;
	cmdqelmt->CallbackFun = HwCtrlTxd.CallbackFun;
	cmdqelmt->CallbackArgs = HwCtrlTxd.CallbackArgs;

	/*create reference count*/
	os_kref_init(&cmdqelmt->refcnt);

	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	/*check queue status*/
	if (!(pHwCtrl->HwCtrlQ.CmdQState & RTMP_TASK_CAN_DO_INSERT) ||
		(pHwCtrl->HwCtrlQ.size >= pHwCtrl->HwCtrlQ.max_size)) {
		ULONG now;
		NdisGetSystemUpTime(&now);
		/* every 5 second to dump queue once full */
		if (RTMP_TIME_AFTER(now, (pHwCtrl->HwCtrlQ.LastQfullTime  + (5 * OS_HZ)))) {
			if (pHwCtrl->HwCtrlQ.util_flag & HW_CMDQ_UTIL_TIME) {
				bDump = TRUE;
			}
			pHwCtrl->HwCtrlQ.LastQfullTime = now;
		}

		bDrop = TRUE; /* original logic , Drop when FULL */
		/* conditional drop the non wait cmd , keep the wait cmd till double queue size */
		if (pHwCtrl->HwCtrlQ.util_flag & HW_CMDQ_UTIL_CONDITION_DROP) {
				/* Drop non wait */
				if (!HwCtrlTxd.NeedWait) {
					MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "---> HWCtrlQ size (%d) overflow!!drop non-wait cmd TYPE:%d,ID:%d\n", pHwCtrl->HwCtrlQ.size, cmdqelmt->type, cmdqelmt->command);
				} else {
					/* Keep wait for survive within double queue size */
					if (pHwCtrl->HwCtrlQ.size <= (pHwCtrl->HwCtrlQ.max_size << 1)) {
						bDrop = FALSE;
						MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "---> HWCtrlQ size (%d) overflow!!keep wait-cmd TYPE:%d,ID:%d\n", pHwCtrl->HwCtrlQ.size, cmdqelmt->type, cmdqelmt->command);
					} else
						MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "---> HWCtrlQ size (%d) overflow!!drop wait-cmd TYPE:%d,ID:%d\n", pHwCtrl->HwCtrlQ.size, cmdqelmt->type, cmdqelmt->command);
				}
		} else { /* original logic drop always */
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "---> HWCtrlQ size (%d) overflow!!drop cmd TYPE:%d,ID:%d, needwait=%d\n", pHwCtrl->HwCtrlQ.size, cmdqelmt->type, cmdqelmt->command, cmdqelmt->NeedWait);
		}

		if (bDump) { /* dump statistics & table refence cnt & time */
			MTWF_PRINT("%s: QueSize: %d, Sum Wait Cnt=%u, Sum Wait Time=%u, LastQfullTime=%lu, util_flag=%x\n",
				__func__, pHwCtrl->HwCtrlQ.size, pHwCtrl->HwCtrlQ.TotalWaitCnt, pHwCtrl->HwCtrlQ.TotalWaitTime, pHwCtrl->HwCtrlQ.LastQfullTime, pHwCtrl->HwCtrlQ.util_flag);
			Show_HwCmdTable(pAd, TRUE);
		}

		if (bDrop) {
			/* record the drop cnt */
			if (cmdqelmt->type < HWCMD_TYPE_END) {
				pHwCmdTable = HwCmdTable[cmdqelmt->type];
				if (pHwCmdTable && (cmdqelmt->command < HWCMD_ID_END)) {
					j = 0;
					/* traverse to the match ID */
					while (pHwCmdTable[j].CmdID != HWCMD_ID_END) {
						if (pHwCmdTable[j].CmdID == cmdqelmt->command) {
							pHwCmdTable[j].DropCnt++;
							break;
						}
						j++;
					}
					pHwCmdTable[cmdqelmt->command].DropCnt++;
				}
			}
			NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
			status = NDIS_STATUS_FAILURE;
			goto end;
		}
	}
	/*add reference count for cmd due to send to thread*/
	os_kref_get(&cmdqelmt->refcnt);

	/*enqueue to cmdq*/
	cmdq = &pHwCtrl->HwCtrlQ;

	if (HwCtrlTxd.NeedWait)
		wait_time = HwCtrlTxd.wait_time ? HwCtrlTxd.wait_time : HWCTRL_CMD_TIMEOUT;

	if (cmdq->size == 0)
		cmdq->head = cmdqelmt;
	else
		cmdq->tail->next = cmdqelmt;

	cmdq->tail = cmdqelmt;
	cmdqelmt->next = NULL;
	cmdq->size++;

	/* Record original wait time  & Sum Total */
	cmdqelmt->WaitTime = wait_time;

	if (pHwCtrl->HwCtrlQ.util_flag & HW_CMDQ_UTIL_WAIT_TIME_ADJ) {
		cmdq->TotalWaitTime += cmdqelmt->WaitTime;
		if (wait_time)
			cmdq->TotalWaitCnt++;

		/*
			head (wait a ms) -> head + 1 (wait b ms) -> head + 2 (no wait)  ->  head + 3 (new insert  (c ms))
			Adjust Total of new insert cmd : a + b + c ms (this may propagate while : out rate < in rate)
		*/

		/* adjust this new insert cmd's wait time if queued some wait cmds ahead  */
		adjust_total_wait_time = cmdq->TotalWaitTime;
	} else { /* original no adjust cases */
		cmdq->TotalWaitTime = 0;
		cmdq->TotalWaitCnt = 0;
		adjust_total_wait_time = cmdqelmt->WaitTime;
	}

	/* Increase per ID's wait time */
	if (pHwCtrl->HwCtrlQ.util_flag & HW_CMDQ_UTIL_TIME) {
		if (cmdqelmt->type < HWCMD_TYPE_END) {
			pHwCmdTable = HwCmdTable[cmdqelmt->type];
			if (pHwCmdTable && (cmdqelmt->command < HWCMD_ID_END)) {
				/* traverse to the match ID */
				j = 0;
				while (pHwCmdTable[j].CmdID != HWCMD_ID_END) {
					if (pHwCmdTable[j].CmdID == cmdqelmt->command) {
						pHwCmdTable[j].TotalWaitTime += adjust_total_wait_time;
						break;
					}
					j++;
				}
			}
		}
	}

	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
	/*get stravation */
#ifdef DBG_STARVATION
	starv_dbg_get(&cmdqelmt->starv);
#endif /*DBG_STARVATION*/
	RTCMDUp(&pHwCtrl->HwCtrlTask);

	/*not need wait, goto end directly*/
	if (!HwCtrlTxd.NeedWait)
		goto end;

	/*wait handle*/
	if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&cmdqelmt->ack_done, RTMPMsecsToJiffies(adjust_total_wait_time))) {
		status = NDIS_STATUS_TIMEOUT;
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "HwCtrl CmdTimeout, TYPE:%d,ID:%d, adjust_total_wait_time=%lu, old_wait_time=%lu!!\n",
			cmdqelmt->type, cmdqelmt->command, RTMPMsecsToJiffies(adjust_total_wait_time), RTMPMsecsToJiffies(wait_time));
	}
end:
	os_kref_put(&cmdqelmt->refcnt, free_hwcmd);
	return status;
}


VOID Show_HwCmdTable(RTMP_ADAPTER *pAd, BOOLEAN bReset)
{
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	HW_CMD_TABLE_T *pHwCmdTable = NULL;
	UCHAR i = 0, j = 0;
	MTWF_PRINT("\tHwCtrlTask Totaol Ref. Cnt: %d\n", pHwCtrl->TotalCnt);
	MTWF_PRINT("\tHwCtrlTask CMD Statistic:\n");
	pHwCmdTable = HwCmdTable[i];
	while (pHwCmdTable != NULL) {
		j = 0;
		while (pHwCmdTable[j].CmdID != HWCMD_ID_END) {
			MTWF_PRINT("\tCMDID: %d, Handler: %p, RfCnt: %d, DropCnt=%u, LastRfTime=%lu, TotalWaitTime=%u\n",
					 pHwCmdTable[j].CmdID, pHwCmdTable[j].CmdHdlr, pHwCmdTable[j].RfCnt, pHwCmdTable[j].DropCnt, ((pHwCmdTable[j].LastRfTime * 1000) / OS_HZ), pHwCmdTable[j].TotalWaitTime);
			if (bReset) {
				pHwCmdTable[j].RfCnt = 0;
				pHwCmdTable[j].DropCnt = 0;
				pHwCmdTable[j].LastRfTime = 0;
			}
			j++;
		}
		pHwCmdTable = HwCmdTable[++i];
	}
}

/*
*
*/
INT Show_HwCtrlStatistic_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	PHwCmdQElmt cmdqelmt = NULL;
	LONG reset = 0;
	if (!(arg == NULL || strlen(arg) == 0)) {
		reset = os_str_tol(arg, 0, 10);
	}
	Show_HwCmdTable(pAd, (reset ? TRUE : FALSE));
	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	MTWF_PRINT("\tQueSize: %d, Sum Wait Cnt=%u, Sum Wait Time=%u, LastQfullTime=%lu, util_flag=%x\n",
		pHwCtrl->HwCtrlQ.size, pHwCtrl->HwCtrlQ.TotalWaitCnt, pHwCtrl->HwCtrlQ.TotalWaitTime, ((pHwCtrl->HwCtrlQ.LastQfullTime * 1000) / OS_HZ), pHwCtrl->HwCtrlQ.util_flag);
	cmdqelmt = pHwCtrl->HwCtrlQ.head;
	while (cmdqelmt) {
		MTWF_PRINT("\tTYPE:%d, CID:%d, WaitTime=%u\n",
			cmdqelmt->type, cmdqelmt->command, cmdqelmt->WaitTime);
		cmdqelmt = cmdqelmt->next;
	}
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
	return TRUE;
}

UINT32 HWCtrlOpsReg(RTMP_ADAPTER *pAd)
{
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	HWCTRL_OP *hwctrl_ops = &pHwCtrl->hwctrl_ops;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/*hook funcion*/
	switch (cap->hw_ops_ver) {
#ifdef WIFI_SYS_FW_V2

	case HWCTRL_OP_TYPE_V2:
		hw_ctrl_ops_v2_register(hwctrl_ops);
		break;
#endif /*WIFI_SYS_FW_V2*/
#ifdef WIFI_SYS_FW_V1

	case HWCTRL_OP_TYPE_V1:
	default:
		hw_ctrl_ops_v1_register(hwctrl_ops);
		break;
#endif /*WIFI_SYS_FW_V1*/
	}

	return NDIS_STATUS_SUCCESS;
}


UINT32 hwctrl_queue_len(RTMP_ADAPTER *pAd)
{
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	UINT32 qlen = 0;

	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	qlen = pHwCtrl->HwCtrlQ.size;
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

	return qlen;
}
