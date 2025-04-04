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
    inf_ppa.c

    Abstract:
    Only for Infineon PPA Direct path feature.



    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    carella	06-01-2010    Created

 */

#ifdef INF_PPA_SUPPORT

#include "rt_config.h"
#include <linux/skbuff.h>
#include <linux/netdevice.h>

extern INT rt28xx_send_packets(struct sk_buff *skb_p, struct net_device *net_dev);

int ifx_ra_start_xmit(struct net_device *rx_dev, struct net_device *tx_dev, struct sk_buff *skb, int len)
{
	if (tx_dev != NULL) {
		SET_OS_PKT_NETDEV(skb, tx_dev);
		rt28xx_send_packets(skb, tx_dev);
	} else if (rx_dev != NULL) {
		skb->protocol = eth_type_trans(skb, skb->dev);
		netif_rx(skb);
	} else
		dev_kfree_skb_any(skb);

	return 0;
}
#endif /* INF_PPA_SUPPORT */
