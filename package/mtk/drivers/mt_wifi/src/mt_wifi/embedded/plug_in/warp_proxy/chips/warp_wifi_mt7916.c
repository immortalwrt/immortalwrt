/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: wifi_offload
	warp_wifi_mt7916.c
*/
#include "warp_wifi_mt7916.h"
#include "../warp_proxy.h"
#include <warp.h>

static void mt7916_fbuf_init(unsigned char *fbuf, unsigned int pkt_pa, unsigned int tkid)
{
	struct txd_l *txd;
	CR4_TXP_MSDU_INFO *txp;

	txd = (struct txd_l *)fbuf;
	txp = (CR4_TXP_MSDU_INFO *)(fbuf+sizeof(struct txd_l));
	memset(txd, 0, sizeof(*txd));
	memset(txp, 0, sizeof(*txp));
	/*initial txd*/
	txd->txd_0 |= (sizeof(*txd) & TXD_TX_BYTE_COUNT_MASK) << TXD_TX_BYTE_COUNT_SHIFT;
	txd->txd_0 |= (FT_HIF_CTD << TXD_PKT_FT_SHIFT);
	txd->txd_1 |= (HF_802_3_FRAME << TXD_HF_SHIFT);
	txd->txd_1 |= TXD_FT;
	txd->txd_1 &=  ~(TXD_HDR_PAD_MASK);
	/*init txp*/
	txp->msdu_token = tkid;
	/*without TXD, CR4 will take care it*/
	txp->type_and_flags = 0;
	txp->buf_num = 1;
	txp->buf_ptr[0] = pkt_pa;
	txp->buf_len[0] = 0;
}

static struct wifi_ops mt7916_jedi_ops = {
	.config_atc = client_config_atc,
	.swap_irq = client_swap_irq,
	.fbuf_init = mt7916_fbuf_init,
	.txinfo_wrapper = client_txinfo_wrapper,
	.txinfo_set_drop = client_txinfo_set_drop,
	.hw_tx_allow = client_hw_tx_allow,
	.tx_ring_info_dump = client_tx_ring_info_dump,
	.warp_ver_notify = client_warp_ver_notify,
	.token_rx_dmad_init = client_token_rx_dmad_init,
	.token_rx_dmad_lookup = client_token_rx_dmad_lookup,
	.rxinfo_wrapper = client_rxinfo_wrapper,
};

void mt7916_chip_specific_get(struct wifi_hw *hw)
{
	hw->dma_offset = WPDMA_OFFSET;
	hw->int_sta = WIFI_INT_STA;
	hw->int_mask = WIFI_INT_MSK;
	hw->tx_dma_glo_cfg = WIFI_WPDMA_GLO_CFG;
	hw->ring_offset = WIFI_RING_OFFSET;
	hw->txd_size = WIFI_PDMA_TXD_SIZE;
	hw->fbuf_size = WIFI_TX_1ST_BUF_SIZE;
	hw->tx_ring_size = WIFI_TX_RING_SIZE;
	hw->tx_pkt_size = WIFI_TX_BUF_SIZE;
	hw->rx_ring_size = WIFI_RX1_RING_SIZE;
	hw->int_ser = WIFI_MCU_INT_EVENT;
	hw->int_ser_value = WIFI_TRIGGER_SER;
	hw->rx_dma_glo_cfg = WIFI_HOST_DMA0_WPDMA_GLO_CFG;
	hw->rxd_size = WIFI_PDMA_RXD_SIZE;
	hw->rx_pkt_size = WIFI_RX_BUF_SIZE;

	/* tx ring */
	hw->tx[0].base = WIFI_TX_RING0_BASE;
	hw->tx[0].cnt = WIFI_TX_RING0_CNT;
	hw->tx[0].cidx = WIFI_TX_RING0_CIDX;
	hw->tx[0].didx = WIFI_TX_RING0_DIDX;
	hw->tx[1].base = WIFI_TX_RING1_BASE;
	hw->tx[1].cnt = WIFI_TX_RING1_CNT;
	hw->tx[1].cidx = WIFI_TX_RING1_CIDX;
	hw->tx[1].didx = WIFI_TX_RING1_DIDX;

	/* event ring for tx free notify */
	hw->event.base = WIFI_RX_RING1_BASE;
	hw->event.cnt = WIFI_RX_RING1_CNT;
	hw->event.cidx = WIFI_RX_RING1_CIDX;
	hw->event.didx = WIFI_RX_RING1_DIDX;

	/* rx data ring */
	hw->rx[0].base = WIFI_RX_DATA_RING0_BASE;
	hw->rx[0].cnt = WIFI_RX_DATA_RING0_CNT;
	hw->rx[0].cidx = WIFI_RX_DATA_RING0_CIDX;
	hw->rx[0].didx = WIFI_RX_DATA_RING0_DIDX;
	hw->rx[1].base = WIFI_RX_DATA_RING1_BASE;
	hw->rx[1].cnt = WIFI_RX_DATA_RING1_CNT;
	hw->rx[1].cidx = WIFI_RX_DATA_RING1_CIDX;
	hw->rx[1].didx = WIFI_RX_DATA_RING1_DIDX;

	hw->wfdma_tx_done_trig0_bit =
	WF_WFDMA_EXT_WRAP_CSR_WED_HOST_INT_STA_host_dma_tx_done_int_sts_0_SHFT;
	hw->wfdma_tx_done_trig1_bit =
	WF_WFDMA_EXT_WRAP_CSR_WED_HOST_INT_STA_host_dma_tx_done_int_sts_1_SHFT;
	hw->wfdma_tx_done_free_notify_trig_bit =
	WF_WFDMA_EXT_WRAP_CSR_WED_HOST_INT_STA_host_dma_rx_done_int_sts_1_SHFT;
	hw->wfdma_rx_done_trig0_bit =
	WF_WFDMA_EXT_WRAP_CSR_WED_HOST_INT_STA_host_dma_rx_done_int_sts_4_SHFT;
	hw->wfdma_rx_done_trig1_bit =
	WF_WFDMA_EXT_WRAP_CSR_WED_HOST_INT_STA_host_dma_rx_done_int_sts_5_SHFT;
	hw->mac_ver = 0x1; /* FMAC */
	hw->dbdc_mode = TRUE;
}

u32 mt7916_warp_register_client(struct wifi_hw *hw)
{
	return warp_register_client(hw, &mt7916_jedi_ops);
}
