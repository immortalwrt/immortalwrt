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
	client_wds_cmm.h

	Abstract:
*/

#ifndef __CLIENT_WDS_CMM_H__
#define __CLIENT_WDS_CMM_H__

#include "rtmp_def.h"

#ifdef CLIENT_WDS

#ifdef ETH_CONVERT_SUPPORT
#error: "ETH_CONVERT function are conflict with CLIENT_WDS function. And Can't support both of them at same time."
#endif /* ETH_CONVERT_SUPPORT */

#ifdef MBSS_AS_WDS_AP_SUPPORT
#define CLI_WDS_ENTRY_AGEOUT 300000  /* 300 seconds */
#else
#define CLI_WDS_ENTRY_AGEOUT 5000  /* seconds */
#endif

#define CLIWDS_POOL_SIZE 128
#define CLIWDS_HASH_TAB_SIZE 64  /* the legth of hash table must be power of 2. */
typedef struct _CLIWDS_PROXY_ENTRY {
	struct _CLIWDS_PROXY_ENTRY *pNext;
	ULONG LastRefTime;
	SHORT Aid;
	UCHAR Addr[MAC_ADDR_LEN];
} CLIWDS_PROXY_ENTRY, *PCLIWDS_PROXY_ENTRY;

#endif /* CLIENT_WDS */

#endif /* __CLIENT_WDS_CMM_H__ */

