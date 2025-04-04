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
 p2p_inf.h

 Abstract:

 Revision History:
 Who		 When			 What
 --------	 ----------	 ----------------------------------------------

*/

#ifndef __P2P_INF_H__
#define __P2P_INF_H__

#define	P2P_DISABLE 0x0
#define P2P_GO_UP	0x1
#define P2P_CLI_UP	0x2
/*#define P2P_GO_ON(_p)                (OPSTATUS_TEST_FLAG(_p, fOP_STATUS_P2P_GO)) */
/*#define P2P_CLI_ON(_p)                (OPSTATUS_TEST_FLAG(_p, fOP_STATUS_P2P_CLI)) */



#define P2P_GO_ON(_pAd) \
	(((_pAd)->flg_p2p_init) \
	 && ((_pAd)->flg_p2p_OpStatusFlags == P2P_GO_UP))


#define P2P_CLI_ON(_pAd) \
	(((_pAd)->flg_p2p_init) \
	 && ((_pAd)->flg_p2p_OpStatusFlags == P2P_CLI_UP))

/* P2P interface hook function definition */
VOID RTMP_P2P_Init(RTMP_ADAPTER *ad_p, PNET_DEV main_dev_p);
VOID RTMP_P2P_Remove(RTMP_ADAPTER *pAd);

INT p2p_virtual_if_open(PNET_DEV dev_p);
INT p2p_virtual_if_close(PNET_DEV dev_p);
INT P2P_VirtualIF_PacketSend(PNDIS_PACKET	 skb_p, PNET_DEV dev_p);


#endif /* __P2P_INF_H__ */

