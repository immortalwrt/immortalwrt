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

#ifndef __TM_H__
#define __TM_H__

enum tm_method {
	TASKLET_METHOD,
	TASKLET_NAPI_METHOD,
};

enum task_type {
	TX_DEQ_TASK,
	RX_DEQ_TASK,
	CMD_MSG_TASK,
};

enum tx_task_state {
	TX_DEQ_RUNNING,
	TX_PKT_PROCESSING,
};

/**
 * @init: tm resource initialization
 * @exit: tm resource exit
 * @schedule_task: schedule task to coressponding tm according to task type
 * @schedule_task_on: schedule task to coressponding tm and specific cpu according to task type
 */
struct tm_ops {
	INT(*init)(struct _RTMP_ADAPTER *pAd);
	INT(*exit)(struct _RTMP_ADAPTER *pAd);
	INT(*schedule_task)(struct _RTMP_ADAPTER *pAd, enum task_type type, UINT8 idx);
	INT(*schedule_task_on)(struct _RTMP_ADAPTER *pAd, INT cpu, enum task_type type, UINT8 idx);
} ____cacheline_aligned;

INT tm_init(struct _RTMP_ADAPTER *pAd);
INT tm_exit(struct _RTMP_ADAPTER *pAd);

#endif
