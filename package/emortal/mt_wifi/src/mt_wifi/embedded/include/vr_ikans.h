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
    vr_ikans.h

    Abstract:
    Only for IKANOS Vx160 or Vx180 platform.

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Sample Lin	01-28-2008    Created

 */

#ifndef __VR_IKANS_H__
#define __VR_IKANS_H__

#ifdef IKANOS_VX_1X0
typedef void (*IkanosWlanTxCbFuncP)(void *, void *);

struct IKANOS_TX_INFO {
	struct net_device *netdev;
	IkanosWlanTxCbFuncP *fp;
};
#endif /* IKANOS_VX_1X0 */

#ifndef MODULE_IKANOS
extern void VR_IKANOS_FP_Init(UINT8 BssNum, UINT8 *pApMac);
extern INT32 IKANOS_DataFramesTx(struct sk_buff *pSkb, struct net_device *pNetDev);
extern void IKANOS_DataFrameRx(PRTMP_ADAPTER pAd, struct sk_buff *pSkb);
#else
void VR_IKANOS_FP_Init(UINT8 BssNum, UINT8 *pApMac);
INT32 IKANOS_DataFramesTx(struct sk_buff *pSkb, struct net_device *pNetDev);
void IKANOS_DataFrameRx(PRTMP_ADAPTER pAd, struct sk_buff *pSkb);
#endif /* MODULE_IKANOS */


#endif /* __VR_IKANS_H__ */

/* End of vr_ikans.h */
