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
	wf_wtbloff.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_WTBL_OFF_H__
#define __WF_WTBL_OFF_H__

#define WF_WTBL_OFF_BASE		0x23000
#define WTBL_OFF_WIUCR			(WF_WTBL_OFF_BASE + 0x0)	/* 0x23000 */
#define WLAN_IDX_MASK			(0xff)
#define WLAN_IDX(p)				(((p) & 0xff))
#define WTBL2_UPDATE_FLAG		(1 << 11)
#define ADM_CNT_CLEAR			(1 << 12)
#define RATE_UPDATE				(1 << 13)
#define TX_CNT_CLEAR			(1 << 14)
#define RX_CNT_CLEAR			(1 << 15)
#define IU_BUSY					(1 << 16)

#define WTBL_OFF_BCR			(WF_WTBL_OFF_BASE + 0x4)	/* 0x23204 */
#define BIP_EN				(1 << 0)

#define WTBL_OFF_RMVTCR		(WF_WTBL_OFF_BASE + 0x8)	/* 0x23008 */
#define RX_MV_MODE				(BIT23)
#define RCPI_AVG_PARAM_MASK	(0x3 << 20)
#define RCPI_AVG_PARAM(p)		(((p) & 0x3) << 20)

#endif /* __WF_WTBL_OFF_H__ */

