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
	eeprom.h

	Abstract:
	Miniport header file for eeprom related information

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#ifndef __FRQCAL_H__
#define __FRQCAL_H__

/* The frequency calibration control */
typedef struct _FREQUENCY_CALIBRATION_CONTROL {
	BOOLEAN bEnableFrequencyCalibration; /* Enable the frequency calibration algorithm */
	BOOLEAN bSkipFirstFrequencyCalibration; /* Avoid calibrating frequency at the time the STA is just link-up */
	BOOLEAN bApproachFrequency; /* Approach the frequency */
	CHAR AdaptiveFreqOffset; /* Adaptive frequency offset */
	CHAR LatestFreqOffsetOverBeacon; /* Latest frequency offset from the beacon */
	CHAR BeaconPhyMode; /* Latest frequency offset from the beacon */

} FREQUENCY_CALIBRATION_CONTROL, *PFREQUENCY_CALIBRATION_CONTROL;

#define RTMP_FREQ_CAL_DISABLE(__pAd)									\
	__pAd->FreqCalibrationCtrl.bEnableFrequencyCalibration = FALSE;

/* Invalid frequency offset */
#define INVALID_FREQUENCY_OFFSET			-128

/* The upperbound/lowerbound of the frequency offset */
#define UPPERBOUND_OF_FREQUENCY_OFFSET		127
#define LOWERBOUND_OF_FREQUENCY_OFFSET	-127

/* The trigger point of the high/low frequency */
#define HIGH_FREQUENCY_TRIGGER_POINT_OFDM		20
#define LOW_FREQUENCY_TRIGGER_POINT_OFDM		-20
#define HIGH_FREQUENCY_TRIGGER_POINT_CCK		4
#define LOW_FREQUENCY_TRIGGER_POINT_CCK		-4

/* The trigger point of decreasng/increasing the frequency offset */
#define DECREASE_FREQUENCY_OFFSET_OFDM			10
#define INCREASE_FREQUENCY_OFFSET_OFDM			-10
#define DECREASE_FREQUENCY_OFFSET_CCK			2
#define INCREASE_FREQUENCY_OFFSET_CCK			-2

/* The trigger point of decreasng/increasing the frequency offset */
#define DECREASE_FREQUENCY_OFFSET			3
#define INCREASE_FREQUENCY_OFFSET			-3

/* Frequency calibration period */
#define FREQUENCY_CALIBRATION_PERIOD		100

#endif /* __FRQCAL_H__ */
