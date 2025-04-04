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
/***************************************************************************
 ***************************************************************************

*/

#ifndef __LP_DVT_H__
#define __LP_DVT_H__

typedef enum _ENUM_SYSDVT_LP_TYPE_T {
	ENUM_SYSDVT_LP_TYPE_LMAC_OWN = 0,
	ENUM_SYSDVT_LP_TYPE_LMAC_OWNBACK,
} ENUM_SYSDVT_LP_TYPE_T;

/*export to features*/
VOID lp_dvt_init(struct dvt_framework *dvt_ctrl);

#endif
