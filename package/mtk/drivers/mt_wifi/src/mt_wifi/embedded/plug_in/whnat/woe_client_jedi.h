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

	Module Name: wifi_offload
	woe_client_jedi.h
*/


#ifndef _WOE_CLIENT_JEDI_H_
#define _WOE_CLIENT_JEDI_H_

#include "rt_config.h"
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include <os/rt_linux_txrx_hook.h>



#ifdef MT7916
#include "woe_mt7916.h"
#endif

#ifdef MT7981
#include "woe_mt7981.h"
#endif

extern int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no);
extern struct _RTMP_CHIP_CAP *hc_get_chip_cap(void *hdev_ctrl);
#ifdef MULTI_INF_SUPPORT
/*EXPORT symbol from wifi drvier*/
extern int multi_inf_get_idx(VOID *pAd);
#endif /*MULTI_INF_SUPPORT*/

#define WIFI_RING_OFFSET		0x10
#define WIFI_TX_RING_SIZE		(2048)
#define WIFI_PDMA_TXD_SIZE		(TXD_SIZE)
#define WIFI_TX_1ST_BUF_SIZE	128
#define WIFI_RX1_RING_SIZE		(512)
#define WIFI_TX_BUF_SIZE		1900

#define WIFI_TXD_INIT(_txd) (((struct _TXD_STRUC *) _txd)->DMADONE = DMADONE_DONE)

#endif /*_WOE_CLIENT_JEDI_H_*/
