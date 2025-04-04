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
    bg_ftph.c

    Abstract:
    Provide fast path between LAN and WLAN.

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Sample Lin	01-22-2008    Created

 */

#include "rt_config.h"

#ifdef BG_FT_SUPPORT
#if defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE)
#include <linux/netfilter_bridge.h>
#include "../net/bridge/br_private.h"

/* extern export symbol in other drivers */
/*
 *	Example in other drivers:
 *		UINT32 (*RALINK_FP_Handle)(PNDIS_PACKET pPacket);
 *		EXPORT_SYMBOL(RALINK_FP_Handle);
 *
 *	packet_forward()
 *	{
 *		UINT32 HandRst = 1;
 *
 *		......
 *
 *		if (RALINK_FP_Handle != NULL)
 *			HandRst = RALINK_FP_Handle(skb);
 *
 *		if (HandRst != 0)
 *		{
 *			// pass the packet to upper layer
 *			skb->protocol = eth_type_trans(skb, skb->dev);
 *			netif_rx(skb);
 *		}
 *	}
 */
UINT32 BG_FTPH_PacketFromApHandle(PNDIS_PACKET pPacket);

#ifdef BG_FT_OPEN_SUPPORT
extern UINT32 (*RALINK_FP_Handle)(PNDIS_PACKET pPacket);
#else
UINT32 (*RALINK_FP_Handle)(PNDIS_PACKET pPacket);
#endif /* BG_FT_OPEN_SUPPORT */




/* --------------------------------- Public -------------------------------- */

/*
 * ========================================================================
 * Routine Description:
 *	Init bridge fast path module.
 *
 * Arguments:
 *	None
 *
 * Return Value:
 *	None
 *
 * Note:
 *	Used in module init.
 * ========================================================================
 */
VOID BG_FTPH_Init(VOID)
{
	RALINK_FP_Handle = BG_FTPH_PacketFromApHandle;
}


/*
 * ========================================================================
 * Routine Description:
 *	Remove bridge fast path module.
 *
 * Arguments:
 *	None
 *
 * Return Value:
 *	None
 *
 * Note:
 *	Used in module remove.
 * ========================================================================
 */
VOID BG_FTPH_Remove(VOID)
{
	RALINK_FP_Handle = NULL;
} /* End of BG_FTPH_Init */




/*
 * ========================================================================
 * Routine Description:
 *	Forward the received packet.
 *
 * Arguments:
 *	pPacket			- the received packet
 *
 * Return Value:
 *	None
 *
 * ========================================================================
 */
UINT32 BG_FTPH_PacketFromApHandle(
	IN		PNDIS_PACKET	pPacket)
{
	struct net_device	*pNetDev;
	struct sk_buff		*pRxPkt;
	struct net_bridge_fdb_entry *pSrcFdbEntry, *pDstFdbEntry;
	/* init */
	pRxPkt = RTPKT_TO_OSPKT(pPacket);
	pNetDev = pRxPkt->dev;
	/* if pNetDev is promisc mode ??? */
	MTWF_DBG(NULL, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "ft bg> BG_FTPH_PacketFromApHandle\n");

	if (pNetDev != NULL) {
		if (pNetDev->br_port != NULL) {
#if (KERNEL_VERSION(2, 6, 12) <= LINUX_VERSION_CODE)
			pDstFdbEntry = br_fdb_get_hook(pNetDev->br_port->br, pRxPkt->data);
			pSrcFdbEntry = br_fdb_get_hook(pNetDev->br_port->br, pRxPkt->data + 6);
#else
			/* br_fdb_get is not exported symbol, need exported in net/bridge/br.c */
			pDstFdbEntry = br_fdb_get(pNetDev->br_port->br, pRxPkt->data);
			pSrcFdbEntry = br_fdb_get(pNetDev->br_port->br, pRxPkt->data + 6);
#endif

			/* check destination address in bridge forwarding table */
			if ((pSrcFdbEntry == NULL) ||
				(pDstFdbEntry == NULL) ||
				(pDstFdbEntry->is_local) ||
				(pDstFdbEntry->dst == NULL) ||
				(pDstFdbEntry->dst->dev == NULL) ||
				(pDstFdbEntry->dst->dev == pNetDev) ||
				(pNetDev->br_port->state != BR_STATE_FORWARDING) ||
				((pSrcFdbEntry->dst != NULL) &&
				 (pSrcFdbEntry->dst->dev != NULL) &&
				 (pSrcFdbEntry->dst->dev != pNetDev))) {
					goto LabelPassToUpperLayer;
				} /* End of if */

			if ((!pDstFdbEntry->is_local) &&
				(pDstFdbEntry->dst != NULL) &&
				(pDstFdbEntry->dst->dev != NULL)) {
				pRxPkt->dev = pDstFdbEntry->dst->dev;
				pDstFdbEntry->dst->dev->hard_start_xmit(pRxPkt, pDstFdbEntry->dst->dev);
				return 0;
			} /* End of if */
		} /* End of if */
	} /* End of if */

LabelPassToUpperLayer:
	MTWF_DBG(NULL, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ft bg> Pass packet to bridge module.\n");
	return 1;
} /* End of BG_FTPH_PacketFromApHandle */


#endif /* CONFIG_BRIDGE || CONFIG_BRIDGE_MODULE */
#endif /* BG_FT_SUPPORT */

/* End of bg_ftph.c */
