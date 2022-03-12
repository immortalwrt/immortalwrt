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
	mcu.h

	Abstract:
	MCU related information

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __MCU_H__
#define __MCU_H__

#define CONFG_BASE_ADDR							0x2000
#define CONFG_COM1_REG3							(CONFG_BASE_ADDR + 0x0000020C)
#define CONFG_COM1_REG3_FWOPMODE				BIT(4)
#define CONFG_COM2_REG3							(CONFG_BASE_ADDR + 0x0000060C)
#define CONFG_COM2_REG3_FWOPMODE				BIT(4)

#define WF_SW_DEF_CR_BASE						0x0041F200
#define WF_SW_DEF_CR_ICAP_SPECTRUM_MODE_ADDR	(WF_SW_DEF_CR_BASE + 0x03C)
#define WF_SW_DEF_CR_FWOPMODE					BITS(0, 1)
#define WF_SW_DEF_CR_FWOPMODE_SHFT				0


#define ENABLE_RXD_LOG 0

enum MCU_TYPE {
	SWMCU = (1 << 0),
	M8051 = (1 << 1),
	ANDES = (1 << 2),
	CR4 = (1 << 3),
};

/*
 * Power opration
 */
enum PWR_OP {
	RADIO_OFF = 0x30,
	RADIO_ON,
	RADIO_OFF_AUTO_WAKEUP,
	RADIO_OFF_ADVANCE,
	RADIO_ON_ADVANCE,
};

struct _RTMP_ADAPTER;

VOID ChipOpsMCUHook(struct _RTMP_ADAPTER *pAd, enum MCU_TYPE MCUType);
VOID MCUCtrlInit(struct _RTMP_ADAPTER *pAd);
VOID MCUCtrlExit(struct _RTMP_ADAPTER *pAd);

INT32 MCUSysPrepare(struct _RTMP_ADAPTER *pAd);


INT32 MCUSysInit(struct _RTMP_ADAPTER *pAd);
INT32 MCUSysExit(struct _RTMP_ADAPTER *pAd);

#endif
