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
	wf_int_wakeup_top.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------
----------------------------------------------
*/
#ifndef __WF_INT_WAKEUP_TOP_H__
#define __WF_INT_WAKEUP_TOP_H__

/* In range of 0x24000 ~ 0x24400 */
#define WF_INT_WAKEUP_TOP_BASE		0x24400
#define WISR0		(WF_INT_WAKEUP_TOP_BASE + 0x00)
#define WISR1		(WF_INT_WAKEUP_TOP_BASE + 0x04)
#define WIER0		(WF_INT_WAKEUP_TOP_BASE + 0x08)
#define WIER1		(WF_INT_WAKEUP_TOP_BASE + 0x0c)
#define AHB_CLK_CTL	(WF_INT_WAKEUP_TOP_BASE + 0x20)		/* 0x24420 */
#define WISR2		(WF_INT_WAKEUP_TOP_BASE + 0x24)
#define WIER2		(WF_INT_WAKEUP_TOP_BASE + 0x28)
#define WISR3		(WF_INT_WAKEUP_TOP_BASE + 0x2c)
#define WIER3		(WF_INT_WAKEUP_TOP_BASE + 0x30)		/* 0x24430 */
#define WISR4		(WF_INT_WAKEUP_TOP_BASE + 0x34)
#define WIER4		(WF_INT_WAKEUP_TOP_BASE + 0x38)
#define HWISR0		(WF_INT_WAKEUP_TOP_BASE + 0x3c)


#define HWIER0_TBTT1    BIT20
#define HWIER0_PRETBTT1 BIT23
#define HWIER0_TBTT2    BIT21
#define HWIER0_PRETBTT2 BIT24
#define HWIER0_TBTT3    BIT22
#define HWIER0_PRETBTT3 BIT25

#define HWIER0_TTTT1    BIT26
#define HWIER0_PRETTTT1 BIT29
#define HWIER0_TTTT2    BIT27
#define HWIER0_PRETTTT2 BIT30
#define HWIER0_TTTT3    BIT28
#define HWIER0_PRETTTT3 BIT31

#define HWIER0		(WF_INT_WAKEUP_TOP_BASE + 0x40)
#define HWISR1		(WF_INT_WAKEUP_TOP_BASE + 0x44)
#define HWIER1		(WF_INT_WAKEUP_TOP_BASE + 0x48)
#define HWISR2		(WF_INT_WAKEUP_TOP_BASE + 0x4c)		/* 0x2444c */
#define HWIER2		(WF_INT_WAKEUP_TOP_BASE + 0x50)		/* 0x24450 */
#define HWISR3		(WF_INT_WAKEUP_TOP_BASE + 0x54)		/* 0x24454 */

#define HWIER3_TBTT0    BIT15
#define HWIER3_PRETBTT0 BIT31

#define HWIER3		(WF_INT_WAKEUP_TOP_BASE + 0x58)
#define HWISR4		(WF_INT_WAKEUP_TOP_BASE + 0x5c)


#define HWIER4_TTTT0    BIT15
#define HWIER4_PRETTTT0 BIT31

#define HWIER4		(WF_INT_WAKEUP_TOP_BASE + 0x60)		/* 0x24460 */

#endif /* __WF_INT_WAKEUP_TOP_H__ */

