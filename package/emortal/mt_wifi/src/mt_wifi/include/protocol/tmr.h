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
    tmr.h

    Abstract:
    802.11v-Timing Measurement,
    802.11mc-Fine Timing Measurement related function and state machine

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Carter      2014-1120     create

*/

#ifndef _TMR_H_
#define _TMR_H_

#ifdef MT_MAC
#ifndef COMPOS_TESTMODE_WIN
#include "rtmp.h"
#endif

#define TOAE_FSM_ITERATION   10
#define ERROR_DEFAULT_DBM   8

enum TMR_TYPE {
	TMR_DISABLE = 0,
	TMR_INITIATOR,
	TMR_RESPONDER
};

enum TMR_IDENRIRY {
	TMR_IR0_TX = 0,
	TMR_IR1_RX = 1
};

enum TMR_INITIATOR_SEND_PKT_STATE {
	SEND_IDLE = 0,
	SEND_OUT
};

VOID TmrReportParser(struct _RTMP_ADAPTER *pAd, TMR_FRM_STRUC *tmr,
	BOOLEAN fgFinalResult, UINT32 TOAECalibrationResult);

VOID MtSetTmrEnable(struct _RTMP_ADAPTER *pAd, UCHAR enable);
INT TmrUpdateParameter(RTMP_ADAPTER *pAd, UCHAR throughold, UCHAR iter);
INT TmrCtrlInit(struct _RTMP_ADAPTER *pAd, UCHAR TmrType, UCHAR Ver);

VOID TmrCtrl(struct _RTMP_ADAPTER *pAd, UCHAR enable, UCHAR Ver);

#endif /* MT_MAC */
#endif /* _TMR_H_ */
