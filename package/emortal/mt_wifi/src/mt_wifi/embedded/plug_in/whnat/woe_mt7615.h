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
	woe_mt7615.h
*/


#ifndef _WOE_MT7615_H_
#define _WOE_MT7615_H_


#include "chip/mt7615_cr.h"
#include "token.h"
#include "mac/mac_mt/dmac/mt_dmac.h"

#define WPDMA_OFFSET 0x4000

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


#ifdef ERR_RECOVERY
#define WIFI_MCU_INT_EVENT MT_MCU_INT_EVENT
#define WIFI_ERR_RECOV_STOP_IDLE	ERR_RECOV_STAGE_STOP_IDLE
#define WIFI_ERR_RECOV_STOP_PDMA0	ERR_RECOV_STAGE_STOP_PDMA0
#define WIFI_ERR_RECOV_RESET_PDMA0	ERR_RECOV_STAGE_RESET_PDMA0
#define WIFI_ERR_RECOV_STOP_IDLE_DONE ERR_RECOV_STAGE_STOP_IDLE_DONE
#define WIFI_TRIGGER_SER			MCU_INT_SER_TRIGGER_FROM_HOST
#endif

/*
*
*/
static inline void wifi_card_fbuf_init(unsigned char *fbuf, unsigned int pkt_pa, unsigned int tkid)
{
	TMAC_TXD_L *txd;
	TMAC_TXD_0 *txd0;
	TMAC_TXD_1 *txd1;
	CR4_TXP_MSDU_INFO *txp;

	txd = (TMAC_TXD_L *)fbuf;
	txp = (CR4_TXP_MSDU_INFO *)(fbuf+sizeof(TMAC_TXD_L));
	memset(txd, 0, sizeof(*txd));
	memset(txp, 0, sizeof(*txp));
	/*initial txd*/
	txd0 = &txd->TxD0;
	txd0->TxByteCount = sizeof(*txd);
	txd0->p_idx = P_IDX_LMAC;
	txd0->q_idx = 0;
	txd1 = &txd->TxD1;
	txd1->ft = TMI_FT_LONG;
	txd1->txd_len = 0;
	txd1->pkt_ft = TMI_PKT_FT_HIF_CT;
	txd1->hdr_format = TMI_HDR_FT_NON_80211;
	TMI_HDR_INFO_VAL(TMI_HDR_FT_NON_80211, 0, 0, 0, 0, 0, 0, 0, txd1->hdr_info);
	txd1->hdr_pad = (TMI_HDR_PAD_MODE_HEAD << TMI_HDR_PAD_BIT_MODE) | 0x1;
	/*init txp*/
	txp->msdu_token = tkid;
	/*without TXD, CR4 will take care it*/
	txp->type_and_flags = 0;
	txp->buf_num = 1;
	txp->buf_ptr[0] = pkt_pa;
	txp->buf_len[0] = 0;
}

#endif /*_WOE_MT7615_H_*/
