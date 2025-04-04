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
	mt_mac_pci.h

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __MAC_PCI_H__
#define __MAC_PCI_H__

#include "rtmp_type.h"
#include "phy/phy.h"
#include "rtmp_iface.h"
#include "rtmp_dot11.h"



/* ----------------- Interface Related MACRO ----------------- */

typedef enum _RTMP_TX_DONE_MASK {
	TX_AC0_DONE = 0,
	TX_AC1_DONE = 1,
	TX_AC2_DONE = 2,
	TX_AC3_DONE = 3,
	TX_HCCA_DONE = 4,
	TX_MGMT_DONE = 5,
	TX_BMC_DONE = 6,
} RTMP_TX_DONE_MASK;


/* For RTMPPCIePowerLinkCtrlRestore () function */
#define RESTORE_HALT		1
#define RESTORE_WAKEUP		2
#define RESTORE_CLOSE           3

#define PowerSafeCID		1
#define PowerRadioOffCID	2
#define PowerWakeCID		3
#define CID0MASK		0x000000ff
#define CID1MASK		0x0000ff00
#define CID2MASK		0x00ff0000
#define CID3MASK		0xff000000

struct _RTMP_ADAPTER;
enum _RTMP_TX_DONE_MASK;

VOID RTMPHandleMgmtRingDmaDoneInterrupt(struct _RTMP_ADAPTER *pAd);
VOID RTMPHandleTBTTInterrupt(struct _RTMP_ADAPTER *pAd);
VOID RTMPHandlePreTBTTInterrupt(struct _RTMP_ADAPTER *pAd);

void RTMPHandleTwakeupInterrupt(struct _RTMP_ADAPTER *pAd);

/*pci state contrl*/
USHORT	pci_write_frag_tx_resource(struct _RTMP_ADAPTER *pAd,
									  struct _TX_BLK *pTxBlk,
									  UCHAR fragNum,
									  USHORT *FreeNumber);

VOID pci_inc_resource_full_cnt(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
VOID pci_dec_resource_full_cnt(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
BOOLEAN pci_get_resource_state(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
BOOLEAN pci_get_all_resource_state(struct _RTMP_ADAPTER *pAd);
INT pci_set_resource_state(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx, BOOLEAN state);
UINT32 pci_check_resource_state(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);

UINT32 pci_get_tx_resource_free_num_nolock(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
UINT32 pci_get_rx_resource_pending_num(struct _RTMP_ADAPTER *pAd, UINT8 que_idx);
BOOLEAN pci_is_tx_resource_empty(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);

VOID mtd_asic_init_txrx_ring(struct _RTMP_ADAPTER *pAd);

VOID mt_asic_init_txrx_ring(struct _RTMP_ADAPTER *pAd);

#endif /*__MAC_PCI_H__ */

