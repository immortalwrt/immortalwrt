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

#ifndef __DBG_TXCMD_EXPORT_H__
#define __DBG_TXCMD_EXPORT_H__
struct dbg_txcmd_framework;

INT dbg_txcmd_feature_search(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
VOID dbg_txcmd_framework_exit(struct _RTMP_ADAPTER *ad, struct dbg_txcmd_framework *dvt_ctrl);

#endif

