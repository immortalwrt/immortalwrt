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
 ****************************************************************************

    Module Name:
	rt_iface.h

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RTMP_IFACE_H__
#define __RTMP_IFACE_H__

#ifdef RTMP_PCI_SUPPORT
#include "iface/rtmp_pci.h"
#endif /* RTMP_PCI_SUPPORT */

#ifdef RTMP_RBUS_SUPPORT
#include "iface/rtmp_rbs.h"
#endif /* RTMP_RBUS_SUPPORT */




typedef struct _INF_PCI_CONFIG_ {
	unsigned long CSRBaseAddress;	/* PCI MMIO Base Address, all access will use */
	unsigned int irq_num;
} INF_PCI_CONFIG;

typedef struct _INF_RBUS_CONFIG_ {
	unsigned long csr_addr;
	unsigned int irq;
} INF_RBUS_CONFIG;


typedef union _RTMP_INF_CONFIG_ {
	struct _INF_PCI_CONFIG_ pciConfig;
	struct _INF_RBUS_CONFIG_ rbusConfig;
} RTMP_INF_CONFIG;

#endif /* __RTMP_IFACE_H__ */
