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
	cmm_rf_cal.c

	Abstract:
	RF calibration and profile related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Arvin Tai     2012/05/02
*/

#define DPD_CAL_PASS_THRES		5
#define DPD_CAL_MAX_RETRY		5

INT32 CalcRCalibrationCode(
	IN PRTMP_ADAPTER pAd,
	IN INT32 D1,
	IN INT32 D2);

INT Set_TestRxIQCalibration_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID R_Calibration(
	IN PRTMP_ADAPTER pAd);

VOID RtmpKickOutHwNullFrame(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bPrepareContent,
	IN BOOLEAN bTransmit);

VOID DPD_IQ_Swap_AM_PM_Inversion(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR AntIdx);

VOID DPD_AM_AM_LUT_Scaling(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR AntIdx);

UCHAR DPD_Calibration(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR AntIdx);

VOID DoDPDCalibration(
	IN PRTMP_ADAPTER pAd);

INT Set_DPDCalPassThres_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_TestDPDCalibration_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_TestDPDCalibrationTX0_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_TestDPDCalibrationTX1_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID LOFT_IQ_Calibration(
	IN RTMP_ADAPTER *pAd);

BOOLEAN BW_Filter_Calibration(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bTxCal);

VOID RxDCOC_Calibration(
	IN PRTMP_ADAPTER pAd);

VOID RXIQ_Calibration(
	IN PRTMP_ADAPTER pAd);

VOID RF_SELF_TXDC_CAL(
	IN PRTMP_ADAPTER pAd);

