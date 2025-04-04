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
	warp_wifi_mt7986.c
*/
#include "warp_wifi_mt7986.h"
#include "../warp_proxy.h"
#include <warp.h>

static void mt7986_fbuf_init(unsigned char *fbuf, unsigned int pkt_pa, unsigned int tkid)
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

static struct wifi_ops mt7986_jedi_ops = {
	.config_atc = client_config_atc,
	.swap_irq = client_swap_irq,
	.fbuf_init = mt7986_fbuf_init,
	.txinfo_wrapper = client_txinfo_wrapper,
	.txinfo_set_drop = client_txinfo_set_drop,
	.hw_tx_allow = client_hw_tx_allow,
	.tx_ring_info_dump = client_tx_ring_info_dump,
	.warp_ver_notify = client_warp_ver_notify,
	.token_rx_dmad_init = client_token_rx_dmad_init,
	.token_rx_dmad_lookup = client_token_rx_dmad_lookup,
	.rxinfo_wrapper = client_rxinfo_wrapper,
	.do_wifi_reset = client_do_wifi_reset,
	.update_wo_rxcnt = client_update_wo_rxcnt
};

void mt7986_chip_specific_get(struct wifi_hw *hw)
{
	hw->dma_offset = WPDMA_OFFSET;
	hw->int_sta = WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR - hw->base_phy_addr;
	hw->int_mask = WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR - hw->base_phy_addr;
	hw->tx_dma_glo_cfg = WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR - hw->base_phy_addr;
	hw->ring_offset = WIFI_RING_OFFSET;
	hw->txd_size = WIFI_PDMA_TXD_SIZE;
	hw->fbuf_size = WIFI_TX_1ST_BUF_SIZE;
	hw->tx_ring_size = WIFI_TX_RING_SIZE;
	hw->tx_pkt_size = WIFI_TX_BUF_SIZE;
	hw->rx_ring_size = WIFI_RX1_RING_SIZE;
	hw->int_ser = WF_WFDMA_MCU_DMA0_HOST2MCU_SW_INT_SET_ADDR - hw->base_phy_addr;
	hw->int_ser_value = MCU_INT_SER_TRIGGER_FROM_HOST;
	hw->rx_dma_glo_cfg = WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR - hw->base_phy_addr;
	hw->rxd_size = WIFI_PDMA_RXD_SIZE;
	hw->rx_pkt_size = WIFI_RX_BUF_SIZE;

	/* tx ring */
	hw->tx[0].base = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING18_CTRL0_ADDR - hw->base_phy_addr;
	hw->tx[0].cnt = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING18_CTRL1_ADDR - hw->base_phy_addr;
	hw->tx[0].cidx = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING18_CTRL2_ADDR - hw->base_phy_addr;
	hw->tx[0].didx = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING18_CTRL3_ADDR - hw->base_phy_addr;
	hw->tx[1].base = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING19_CTRL0_ADDR - hw->base_phy_addr;
	hw->tx[1].cnt = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING19_CTRL1_ADDR - hw->base_phy_addr;
	hw->tx[1].cidx = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING19_CTRL2_ADDR - hw->base_phy_addr;
	hw->tx[1].didx = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING19_CTRL3_ADDR - hw->base_phy_addr;

	/* event ring for tx free notify */
	hw->event.base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_CTRL0_ADDR - hw->base_phy_addr;
	hw->event.cnt = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_CTRL1_ADDR - hw->base_phy_addr;
	hw->event.cidx = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_CTRL2_ADDR - hw->base_phy_addr;
	hw->event.didx = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_CTRL3_ADDR - hw->base_phy_addr;

	/* rx data ring */
	hw->rx[0].base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_CTRL0_ADDR - hw->base_phy_addr;
	hw->rx[0].cnt = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_CTRL1_ADDR - hw->base_phy_addr;
	hw->rx[0].cidx = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_CTRL2_ADDR - hw->base_phy_addr;
	hw->rx[0].didx = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_CTRL3_ADDR - hw->base_phy_addr;
	hw->rx[1].base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_CTRL0_ADDR - hw->base_phy_addr;
	hw->rx[1].cnt = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_CTRL1_ADDR - hw->base_phy_addr;
	hw->rx[1].cidx = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_CTRL2_ADDR - hw->base_phy_addr;
	hw->rx[1].didx = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_CTRL3_ADDR - hw->base_phy_addr;

	hw->wfdma_tx_done_trig0_bit =
	WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_18_SHFT;
	hw->wfdma_tx_done_trig1_bit =
	WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_19_SHFT;
	hw->wfdma_tx_done_free_notify_trig_bit =
	WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_2_SHFT;
	hw->wfdma_rx_done_trig0_bit =
	WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_4_SHFT;
	hw->wfdma_rx_done_trig1_bit =
	WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_5_SHFT;
	hw->dbdc_mode = true;
}

u32 mt7986_warp_register_client(struct wifi_hw *hw)
{
	return warp_register_client(hw, &mt7986_jedi_ops);
}
