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
	warp_wifi_mt7615.h
*/
#ifndef _WARP_WIFI_MT7615_H_
#define _WARP_WIFI_MT7615_H_

#include "chip/mt7615_cr.h"
#include "token.h"
#include "mac/mac_mt/dmac/mt_dmac.h"

#define WPDMA_OFFSET	0x4000

#ifdef ERR_RECOVERY
#define WIFI_MCU_INT_EVENT		WF_WFDMA_MCU_DMA1_PCI_BASE(WF_WFDMA_MCU_DMA1_HOST2MCU_SW_INT_SET_ADDR)
#define WIFI_TRIGGER_SER		MCU_INT_SER_TRIGGER_FROM_HOST
#endif

/*CR usage remapping*/
#define WIFI_TX_RING0_BASE	MT_WPDMA_TX_RING0_CTRL0
#define WIFI_TX_RING0_CNT	MT_WPDMA_TX_RING0_CTRL1
#define WIFI_TX_RING0_CIDX	MT_WPDMA_TX_RING0_CTRL2
#define WIFI_TX_RING0_DIDX	MT_WPDMA_TX_RING0_CTRL3
#define WIFI_TX_RING1_BASE	MT_WPDMA_TX_RING1_CTRL0
#define WIFI_TX_RING1_CNT	MT_WPDMA_TX_RING1_CTRL1
#define WIFI_TX_RING1_CIDX	MT_WPDMA_TX_RING1_CTRL2
#define WIFI_TX_RING1_DIDX	MT_WPDMA_TX_RING1_CTRL3

#define WIFI_RX_RING1_BASE	MT_WPDMA_RX_RING1_CTRL0
#define WIFI_RX_RING1_CNT	MT_WPDMA_RX_RING1_CTRL1
#define WIFI_RX_RING1_CIDX	MT_WPDMA_RX_RING1_CTRL2
#define WIFI_RX_RING1_DIDX	MT_WPDMA_RX_RING1_CTRL3

#define WIFI_INT_STA		MT_INT_SOURCE_CSR
#define WIFI_INT_MSK		MT_INT_MASK_CSR
#define WIFI_WPDMA_GLO_CFG	MT_WPDMA_GLO_CFG
#define WIFI_WPDMA_GLO_CFG_FLD_TX_DMA_EN                    (0)
#define WIFI_WPDMA_GLO_CFG_FLD_TX_DMA_BUSY                  (1)
#define WIFI_WPDMA_GLO_CFG_FLD_RX_DMA_EN                    (2)
#define WIFI_WPDMA_GLO_CFG_FLD_RX_DMA_BUSY                  (3)
#define WIFI_WPDMA_RESET_PTR WPDMA_RST_PTR
#define WIFI_WPDMA_RESET_PTR_FLD_RST_DRX_IDX1				(17)
#define WIFI_WPDMA_RESET_PTR_FLD_RST_DTX_IDX0				(0)
#define WIFI_WPDMA_RESET_PTR_FLD_RST_DTX_IDX1				(1)

#endif /*_WARP_WIFI_MT7615_H_*/
