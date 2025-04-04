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
	wf_trb.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_TRB_H__
#define __WF_TRB_H__

#define WF_TRB_BASE		0x820F1000


#define TRB_GCR			(WF_TRB_BASE + 0x000)

#define TRB_TRBTHR		(WF_TRB_BASE + 0x014)
#define TRB_RXBTHR		(WF_TRB_BASE + 0x01C)

#ifdef DBG
#define TRB_TXBSR		(WF_TRB_BASE + 0x020)
#define TRB_TXPSR		(WF_TRB_BASE + 0x024)

#define TRB_RXBSR		(WF_TRB_BASE + 0x040)
#define TRB_RXPSR0		(WF_TRB_BASE + 0x044)
#define TRB_RXPSR1		(WF_TRB_BASE + 0x048)
#define TRB_RXPSR2		(WF_TRB_BASE + 0x04c)
#endif /* DBG */
#endif /* __WF_TRB_H__ */

