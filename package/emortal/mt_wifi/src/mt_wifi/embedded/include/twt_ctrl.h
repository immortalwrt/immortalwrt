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
    twt_ctrl.h

    Abstract:
    Support twt hardware control

    Who             When            What
    --------------  ----------      --------------------------------------------

*/

#ifndef __TWT_CTRL_H__
#define __TWT_CTRL_H__

#define MTK_TWT_GROUP_EN			0

/* TWT definitions for hw */
#define TWT_HW_AGRT_MAX_NUM			16
#if (MTK_TWT_GROUP_EN == 1)
#define TWT_HW_GRP_MAX_NUM			8
#else
#define TWT_HW_GRP_MAX_NUM			0
#endif
#define TWT_HW_GRP_MAX_MEMBER_CNT	8

#define TWT_TYPE_INDIVIDUAL			0
#define TWT_TYPE_GROUP				1

#define INVALID_TWT_HW_ID			0xff

/* max group grade */
#define MAX_GRP_GRADE				100

/* in unit of 256usec */
#define TWT_MAX_SP					255
#define TWT_SP_SPAN_TIME			((TWT_MAX_SP << 8) * TWT_HW_AGRT_MAX_NUM)

/* 16TU = 16*1024usec*/
#define TWT_TSF_ALIGNMNET_UINT		(16 * 1024)

#define SCH_LINK					0
#define USCH_LINK					1
#define SCH_LINK_NUM				2

#endif /* __TWT_CTRL_H__ */
