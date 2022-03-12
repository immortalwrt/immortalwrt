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

#ifndef __DVT_EXPORT_H__
#define __DVT_EXPORT_H__

struct dvt_framework;

INT dvt_feature_search(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
VOID dvt_framework_exit(struct _RTMP_ADAPTER *ad, struct dvt_framework *dvt_ctrl);
#ifdef DOT11_HE_AX
INT dvt_enable_sta_he_test(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
#endif /* DOT11_HE_AX */

#endif
