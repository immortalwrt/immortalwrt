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
	wf_wtblon.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __WF_WTBL_ON_H__
#define __WF_WTBL_ON_H__

#if defined(P18) || defined(MT7663) || defined(AXE) || defined(MT7626) || \
	defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
#define WF_WTBL_ON_BASE		0x29000
#else
#define WF_WTBL_ON_BASE		0x23000
#endif

#define WTBL_ON_WTBLOR		(WF_WTBL_ON_BASE + 0x0)
#define WTBL_OR_PSM_W_FLAG	(1<<31)

#define WTBL_ON_RICR0		(WF_WTBL_ON_BASE + 0x10)	/* DW0 */
#define WTBL_ON_RICR1		(WF_WTBL_ON_BASE + 0x14)	/* DW1 */
#define WTBL_ON_RIUCR0		(WF_WTBL_ON_BASE + 0x20)	/* DW5 */
#define WTBL_ON_RIUCR1		(WF_WTBL_ON_BASE + 0x24)	/* DW6 */
#define WTBL_ON_RIUCR2		(WF_WTBL_ON_BASE + 0x28)	/* DW7 */
#define WTBL_ON_RIUCR3		(WF_WTBL_ON_BASE + 0x2C)	/* DW8 */

#define WTBL_ON_TCGSBR		(WF_WTBL_ON_BASE + 0x40)
#define WTBL_ON_ACGSBR		(WF_WTBL_ON_BASE + 0x44)
#define WTBL_ON_RVCDARx		(WF_WTBL_ON_BASE + 0x80)	/* RV bit for wtbl */

#define WTBL_BTCRn		(WF_WTBL_ON_BASE + 0x100)
#define WTBL_BTBCRn		(WF_WTBL_ON_BASE + 0x110)
#define WTBL_BRCRn		(WF_WTBL_ON_BASE + 0x120)
#define WTBL_BRBCRn		(WF_WTBL_ON_BASE + 0x130)
#define WTBL_BTDCRn		(WF_WTBL_ON_BASE + 0x140)
#define WTBL_BRDCRn		(WF_WTBL_ON_BASE + 0x150)

#define WTBL_MBTCRn		(WF_WTBL_ON_BASE + 0x200)
#define WTBL_MBTBCRn		(WF_WTBL_ON_BASE + 0x240)
#define WTBL_MBRCRn		(WF_WTBL_ON_BASE + 0x280)
#define WTBL_MBRBCRn		(WF_WTBL_ON_BASE + 0x2C0)


#define PSM_W_FLAG			(1 << 31)
#endif /* __WF_WTBL_ON_H__ */

