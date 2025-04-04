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

#ifndef __FP_QM_H__

enum {
	FP_QUE0,
	FP_QUE1,
};

struct fp_qm {
	UINT16 max_tx_process_cnt;
	UINT16 max_mgmt_que_num;
	UINT16 max_data_que_num;
	UINT16 extra_reserved_que_num;
};

struct fp_tx_flow_control {
	ULONG flag;
	ULONG *TxFlowBlockState;
	DL_LIST *TxBlockDevList;
};

VOID fp_tx_pkt_deq_func(struct _RTMP_ADAPTER *pAd, UINT8 idx);
VOID fp_rx_pkt_deq_func(struct _RTMP_ADAPTER *pAd);

#endif


