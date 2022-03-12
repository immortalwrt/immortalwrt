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
/***************************************************************************
 ***************************************************************************

*/
#include "rt_config.h"


static VOID ge_rx_pkt_deq_tasklet(ULONG param)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)param;

	ge_rx_pkt_deq_func(pAd);
}

static VOID ge_tx_pkt_deq_tasklet(ULONG param)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)param;

	ge_tx_pkt_deq_func(pAd, 0);
}

static VOID fp_tx_pkt_deq_tasklet(ULONG param)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)param;

	fp_tx_pkt_deq_func(pAd, 0);
}

static VOID fp1_tx_pkt_deq_tasklet(ULONG param)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)param;

	fp_tx_pkt_deq_func(pAd, 1);
}

static VOID fp_fair_tx_pkt_deq_tasklet(ULONG param)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)param;

	fp_fair_tx_pkt_deq_func(pAd, 0);
}

static VOID fp1_fair_tx_pkt_deq_tasklet(ULONG param)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)param;

	fp_fair_tx_pkt_deq_func(pAd, 1);
}

static INT tm_tasklet_qm_init(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 idx;

	if (cap->qm == FAST_PATH_QM) {
		for (idx = 0; idx < 2; idx++) {
			RTMP_OS_TASKLET_INIT(pAd, &pAd->tx_deque_tasklet[0], fp_tx_pkt_deq_tasklet, (unsigned long)pAd);
			RTMP_OS_TASKLET_INIT(pAd, &pAd->tx_deque_tasklet[1], fp1_tx_pkt_deq_tasklet, (unsigned long)pAd);
			pAd->tx_dequeue_scheduable[idx] = TRUE;
		}
	} else if (cap->qm == FAST_PATH_FAIR_QM) {
		RTMP_OS_TASKLET_INIT(pAd, &pAd->tx_deque_tasklet[0], fp_fair_tx_pkt_deq_tasklet, (unsigned long)pAd);
		RTMP_OS_TASKLET_INIT(pAd, &pAd->tx_deque_tasklet[1], fp1_fair_tx_pkt_deq_tasklet, (unsigned long)pAd);
		for (idx = 0; idx < 2; idx++)
			pAd->tx_dequeue_scheduable[idx] = TRUE;
	} else if (IS_GE_QM(cap->qm)) {
		RTMP_OS_TASKLET_INIT(pAd, &pAd->tx_deque_tasklet[0], ge_tx_pkt_deq_tasklet, (unsigned long)pAd);
		pAd->tx_dequeue_scheduable[0] = TRUE;
	}
#ifdef RX_RPS_SUPPORT
	if (cap->rx_qm == GENERIC_QM) {
		UINT32 cpu;
		for (cpu = 0; cpu < NR_CPUS; cpu++)
			RTMP_OS_TASKLET_INIT(pAd, &pAd->rx_deque_tasklet[cpu], ge_rx_pkt_deq_tasklet, (unsigned long)pAd);
	}
#else
	/* RX QM, for sw RPS now */
	if (cap->rx_qm == GENERIC_QM)
		RTMP_OS_TASKLET_INIT(pAd, &pAd->rx_deque_tasklet, ge_rx_pkt_deq_tasklet, (unsigned long)pAd);
#endif

	return ret;
}

static INT tm_tasklet_qm_exit(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;
	UINT8 idx;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->qm == FAST_PATH_QM) {
		for (idx = 0; idx < 2; idx++) {
			pAd->tx_dequeue_scheduable[idx] = FALSE;
			RTMP_OS_TASKLET_KILL(&pAd->tx_deque_tasklet[idx]);
		}
	} else {
		pAd->tx_dequeue_scheduable[0] = FALSE;
		RTMP_OS_TASKLET_KILL(&pAd->tx_deque_tasklet[0]);
	}
#ifdef RX_RPS_SUPPORT
	if (cap->rx_qm == GENERIC_QM) {
		UINT32 cpu;
		for (cpu = 0; cpu < NR_CPUS; cpu++)
			RTMP_OS_TASKLET_KILL(&pAd->rx_deque_tasklet[cpu]);
	}
#else
	if (cap->rx_qm == GENERIC_QM)
		RTMP_OS_TASKLET_KILL(&pAd->rx_deque_tasklet);
#endif

	return ret;
}

static INT tm_tasklet_qm_schedule_task(RTMP_ADAPTER *pAd, enum task_type type, UINT8 idx)
{
	INT ret = NDIS_STATUS_SUCCESS;

	switch (type) {
	case TX_DEQ_TASK:
		if (pAd->tx_dequeue_scheduable[idx]) {
#ifdef KERNEL_RPS_ADJUST
			if (pAd->ixia_mode_ctl.tx_tasklet_sch)
				tasklet_schedule(&pAd->tx_deque_tasklet[idx]);
			else
#endif
			RTMP_OS_TASKLET_SCHE(&pAd->tx_deque_tasklet[idx]);
		}

		break;

	case RX_DEQ_TASK:
#ifdef RX_RPS_SUPPORT
		if (pAd->rx_dequeue_sw_rps_enable) {
#ifdef KERNEL_RPS_ADJUST
			if (pAd->ixia_mode_ctl.rx_tasklet_sch)
				tasklet_schedule(&pAd->rx_deque_tasklet[smp_processor_id()]);
			else
#endif
			RTMP_OS_TASKLET_SCHE(&pAd->rx_deque_tasklet[smp_processor_id()]);
		}
#else
		RTMP_OS_TASKLET_SCHE(&pAd->rx_deque_tasklet);
#endif
		break;

	default:

		break;
	}

	return ret;
}

static VOID tx_deq0_schedule_cpu(RTMP_ADAPTER *pAd)
{
	RTMP_OS_TASKLET_SCHE(&pAd->tx_deque_tasklet[0]);

	return;
}

static VOID tx_deq1_schedule_cpu(RTMP_ADAPTER *pAd)
{
	RTMP_OS_TASKLET_SCHE(&pAd->tx_deque_tasklet[1]);

	return;
}

static INT tm_tasklet_qm_schedule_task_on(RTMP_ADAPTER *pAd, int cpuid, enum task_type type, UINT8 idx)
{
	INT ret = NDIS_STATUS_SUCCESS;

	switch (type) {
	case TX_DEQ_TASK:
		if (pAd->tx_dequeue_scheduable[idx]) {
			if (idx == 0)
				smp_call_function_single(cpuid, (smp_call_func_t)tx_deq0_schedule_cpu, pAd, 0);
			else
				smp_call_function_single(cpuid, (smp_call_func_t)tx_deq1_schedule_cpu, pAd, 0);
		}

		break;

	default:

		break;
	}

	return ret;
}

struct tm_ops tm_tasklet_qm_ops = {
	.init = tm_tasklet_qm_init,
	.exit = tm_tasklet_qm_exit,
	.schedule_task = tm_tasklet_qm_schedule_task,
	.schedule_task_on = tm_tasklet_qm_schedule_task_on,
};

INT tm_init(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct tm_ops **tm_qm_ops = &pAd->tm_qm_ops;
	INT ret = NDIS_STATUS_SUCCESS;

	if (cap->qm_tm == TASKLET_METHOD)
		*tm_qm_ops = &tm_tasklet_qm_ops;

	ret = (*tm_qm_ops)->init(pAd);

	return ret;
}

INT tm_exit(RTMP_ADAPTER *pAd)
{
	struct tm_ops *tm_qm_ops = pAd->tm_qm_ops;
	INT ret = NDIS_STATUS_SUCCESS;

	ret = tm_qm_ops->exit(pAd);

	return ret;
}
