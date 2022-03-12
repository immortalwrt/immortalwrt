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
/****************************************************************************
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All Related Structure & Definition for UBICOM platform.

	Only used in UTIL module.

***************************************************************************/

#ifndef __VR_UBICOM_H__
#define __VR_UBICOM_H__

#ifdef PLATFORM_UBM_IPX8

#include <asm/cachectl.h>

#undef RTMP_UTIL_DCACHE_FLUSH
#define RTMP_UTIL_DCACHE_FLUSH(__AddrStart, __Size)						\
	flush_dcache_range((ULONG)(__AddrStart),							\
					   (ULONG)(((UCHAR *)(__AddrStart)) + __Size - 1))

#endif /* PLATFORM_UBM_IPX8 */

#endif /* __VR_UBICOM_H__ */

/* End of vrut_ubm.h */

