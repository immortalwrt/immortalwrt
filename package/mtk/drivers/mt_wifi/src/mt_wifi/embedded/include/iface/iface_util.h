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
	rtmp_util.h

    Abstract:
	Common for PCI/USB/RBUS.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/

#ifndef __RTMP_UTIL_H__
#define __RTMP_UTIL_H__

/* maximum of PCI, USB, or RBUS, int PCI, it is 0 but in USB, it is 11 */
#define RTMP_PKT_TAIL_PADDING	11 /* 3(max 4 byte padding) + 4 (last packet padding) + 4 (MaxBulkOutsize align padding) */

#define RTMP_PCI_DMA_TODEVICE		0xFF00
#define RTMP_PCI_DMA_FROMDEVICE		0xFF01



#define UNLINK_TIMEOUT_MS		3

#define USBD_TRANSFER_DIRECTION_OUT		0
#define USBD_TRANSFER_DIRECTION_IN		0
#define USBD_SHORT_TRANSFER_OK			0
#define PURB			purbb_t

#define OS_RTMP_MlmeUp					RtmpOsMlmeUp

#endif /* __RTMP_UTIL_H__ */
