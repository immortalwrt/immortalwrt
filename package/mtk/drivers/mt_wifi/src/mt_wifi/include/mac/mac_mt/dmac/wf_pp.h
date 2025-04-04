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
	wf_pp.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __WF_PP_H__
#define __WF_PP_H__

#define PP_BASE		0xe000


/* CT path control register */
#define PP_RXCUTDISP					(PP_BASE + 0x54)
#define PP_RXCUTDISP_CT_EN_MASK		(1<<0)
#define PP_RXCUTDISP_CR4_EN_MASK		(1<<1)

/* */
#define PP_PAGECTL_0					(PP_BASE + 0x58)
#define PAGECTL_0_PSE_PG_CNT_MASK	(0xfff)

/* */
#define PP_PAGECTL_1					(PP_BASE + 0x5c)
#define PAGECTL_1_PLE_PG_CNT_MASK	(0xfff)

/* */
#define PP_PAGECTL_2					(PP_BASE + 0x60)
#define PAGECTL_2_CUT_PG_CNT_MASK	(0xfff)

/* PP spare dummy CR */
#define PP_SPARE_DUMMY_CR5				(PP_BASE + 0x64)
#define PP_SPARE_DUMMY_CR6				(PP_BASE + 0x68)
#define PP_SPARE_DUMMY_CR7				(PP_BASE + 0x6c)
#define PP_SPARE_DUMMY_CR8				(PP_BASE + 0x70)

#endif /* __WF_PP_H__ */

