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

#ifndef _DIAG_H_
#define _DIAG_H_

#ifdef WIFI_DIAG

#include "rtmp_comm.h"
#include "rtmp_type.h"
#include "rtmp_os.h"
#include "rtmp.h"

typedef enum _ENUM_DIAG_CONN_ERROR_CODE{
	DIAG_CONN_FRAME_LOST = 0,
	DIAG_CONN_CAP_ERROR,
	DIAG_CONN_AUTH_FAIL,
	DIAG_CONN_ACL_BLK,
	DIAG_CONN_STA_LIM,
	DIAG_CONN_DEAUTH,
	DIAG_CONN_BAND_STE,
	DIAG_CONN_ERROR_MAX,
	DIAG_CONN_DEAUTH_COM
}ENUM_DIAG_CONN_ERROR_CODE;


void diag_conn_error(PRTMP_ADAPTER pAd, UCHAR apidx, UCHAR* addr,
	ENUM_DIAG_CONN_ERROR_CODE Code, UINT32 Reason);
void diag_conn_error_write(PRTMP_ADAPTER pAd);
void diag_add_pid(OS_TASK *pTask);
void diag_del_pid(OS_TASK *pTask);
void diag_get_process_info(PRTMP_ADAPTER	pAdapter, RTMP_IOCTL_INPUT_STRUCT	*wrq);
void diag_miniport_mm_request(PRTMP_ADAPTER pAd, UCHAR *pData, UINT Length);
void diag_bcn_tx(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, UCHAR *pBeaconFrame,ULONG FrameLen );
void diag_log_file_write(PRTMP_ADAPTER pAd);
void diag_dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
void diag_dev_rx_cntl_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
void diag_ap_mlme_one_sec_proc(PRTMP_ADAPTER pAd);
void diag_ctrl_alloc(PRTMP_ADAPTER pAd);
void diag_ctrl_free(PRTMP_ADAPTER pAd);
BOOLEAN diag_proc_init(PRTMP_ADAPTER pAd, struct wifi_dev *wdev);
BOOLEAN diag_proc_exit(PRTMP_ADAPTER pAd, struct wifi_dev *wdev);
#if defined(MT7663) || defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
void diag_get_snr(RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR *pData);
#endif

#endif
#endif /* #ifndef _DIAG_H_ */

