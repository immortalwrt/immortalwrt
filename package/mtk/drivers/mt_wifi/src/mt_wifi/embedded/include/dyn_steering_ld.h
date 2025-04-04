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
	dyn_steering_ld.h
*/
#ifndef __DYN_STEERING_LD_H__
#define __DYN_STEERING_LD_H__
#include "mac/mac.h"
#include "rtmp_type.h"
#include "rtmp_comm.h"

typedef enum _ENUM_DYNAMIC_STEERING_STAT_TYPE {
	ENUM_FIRST_TX_COMBO_STAT,
	ENUM_SECOND_TX_COMBO_STAT,
	ENUM_RX_COMBO_STAT,
} ENUM_DYNAMIC_STEERING_STAT_TYPE, *P_ENUM_DYNAMIC_STEERING_STAT_TYPE;

typedef struct _DYNAMIC_STEERING_CTRL {
	RTMP_NET_TASK_STRUCT tx_combo_deque_tasklet;
	BOOLEAN combo_tx_dequeue_scheduable;
	NDIS_SPIN_LOCK tx_combo_que_lock;
	QUEUE_HEADER tx_combo_que;

	UINT32 con_tx_tp_running;
	UINT32 con_rx_tp_running;

	UINT32 tp_2g_tx;
	UINT32 tp_2g_rx;
	UINT32 tp_5g_tx;
	UINT32 tp_5g_rx;
	UINT32 disable_auto_txop;
	UINT32 disable_auto_rps;
	UINT32 disable_auto_ratio;
	UINT32 delta_tx_cnt;
	UINT32 delta_rx_cnt;
	UINT32 delta_tx_cnt_avg;
	UINT32 delta_rx_cnt_avg;
	UINT32 tr_ratio;
	UINT32 rt_ratio;
	UINT32 tr_ratio_avg;
	UINT32 rt_ratio_avg;
	UINT32 pre_tr_ratio_avg;
	UINT32 pre_rt_ratio_avg;

	UINT32 First_combo_TxRpsRatio;
	UINT32 First_combo_TxRpsRatiobaseF;
	UINT32 First_combo_cnt_core0;
	UINT32 First_combo_cnt_core1;
	UINT32 First_combo_cnt_core0_old;
	UINT32 First_combo_cnt_core1_old;
	UINT32 First_combo_tx_cpu;

	UINT32 Second_combo_cnt_core0;
	UINT32 Second_combo_cnt_core1;

	UINT32 First_combo_rx_cnt_core0;
	UINT32 First_combo_rx_cnt_core1;
	UINT32 First_combo_rx_cnt_core0_old;
	UINT32 First_combo_rx_cnt_core1_old;
	UINT32 First_combo_buf_underrun;

	UINT32 First_combo_buf_max_processed_cnt;
	UINT32 Second_combo_buf_max_processed_cnt;

	UINT32 First_combo_buf_max_cnt;
	UINT32 Second_combo_buf_max_cnt;

	UINT32 tcp_small_packet_combo_buf_max_cnt;
	UINT32 one_sec_rx_max_pkt_size;
	UINT32 one_sec_tx_max_pkt_size;

	UINT32 rx_intr_cpu;
	UINT32 total_tx_process_cnt_for_specific_cpu;
	UINT32 tasklet_schdule_lo_flag;
	UINT32 concurrent_tput_flag;
	UINT32 single_5g_tput_flag;
	UINT32 single_2g_tput_flag;
	UINT32 single_5g_one_pair_tput_flag;
	UINT32 single_2g_one_pair_tput_flag;
	UINT32 bidi_tput_flag;
	UINT32 tx_tput_flag;
	UINT32 rx_tput_flag;
	UINT32 rps_apply_idx;
	ULONG rps_update_tick;
} DYNAMIC_STEERING_CTRL;
VOID dyn_steering_tx_pkt_deq_func(struct _RTMP_ADAPTER *pAd);

VOID dyn_steering_ld_statistic_sync(struct _RTMP_ADAPTER *pAd, INT32 type, INT32 pkt_cnt);
BOOLEAN dyn_steering_ld_is_auto_rps_stop(struct _RTMP_ADAPTER *pAd);
BOOLEAN dyn_steering_ld_is_auto_txop_stop(struct _RTMP_ADAPTER *pAd);
VOID dyn_steering_ld_algo(struct _RTMP_ADAPTER *pAd);
INT32 dyn_steering_ld_set_rps_ratio(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID dyn_steering_ld_rx(struct _RTMP_ADAPTER *pAd, PNDIS_PACKET pRxPkt);
INT32 dyn_steering_ld_tx(struct wifi_dev *wdev, PNDIS_PACKET pPacket);
VOID dyn_steering_ld_tasklet_sched(struct _RTMP_ADAPTER *pAd);
VOID dyn_steering_qm_exit(struct _RTMP_ADAPTER *pAd);
VOID dyn_steering_qm_init(struct _RTMP_ADAPTER *pAd);
VOID dyn_steering_tasklet_exit(struct _RTMP_ADAPTER *pAd);
VOID dyn_steering_tasklet_init(struct _RTMP_ADAPTER *pAd);
#endif /* __DYN_STEERING_LD_H__ */
