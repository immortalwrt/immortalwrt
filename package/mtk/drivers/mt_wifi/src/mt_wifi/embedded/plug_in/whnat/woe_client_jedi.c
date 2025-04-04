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

	Module Name: whnat
	whnat_mt7615.c
*/

#if defined(MT7615) || defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
#include "woe_client_jedi.h"
#include "woe.h"
#include <net/ra_nat.h>
#include <linux/pci.h>
#include <net/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

/*Local function part*/
/*
*
*/
static void wifi_pcie_match(struct wifi_entry *wifi)
{
	unsigned char idx = 0;
	idx = wifi_slot_get(wifi->cookie);
	wifi->slot_id = idx;
	wifi->wpdma_base = wifi_wpdma_base_get(wifi->cookie);
}



/*Gloable function*/
/*
*
*/
void wifi_fbuf_init(unsigned char *fbuf, unsigned int pkt_pa, unsigned int tkid)
{
	wifi_card_fbuf_init(fbuf, pkt_pa, tkid);
}

/*
*
*/
static inline void wifi_tx_info_wrapper(unsigned char *tx_info, struct wlan_tx_info *info)
{
	struct _TX_BLK *txblk = (struct _TX_BLK *)tx_info;

	info->pkt = txblk->pPacket;
	info->bssidx = txblk->wdev->bss_info_argument.ucBssIndex;
	info->ringidx = txblk->dbdc_band;

	if (txblk->pMacEntry && (IS_ENTRY_PEER_AP(txblk->pMacEntry) || IS_ENTRY_CLIENT(txblk->pMacEntry)))
		info->wcid = txblk->pMacEntry->wcid;
	else
#ifdef MAC_REPEATER_SUPPORT

	if (txblk->pMacEntry && IS_ENTRY_REPEATER(txblk->pMacEntry))
		info->wcid = txblk->pMacEntry->wcid;
	else
#endif
#ifdef A4_CONN
	if (txblk->pMacEntry && IS_ENTRY_A4(txblk->pMacEntry))
		info->wcid = txblk->pMacEntry->wcid;
	else
#endif /* A4_CONN */
	{
		info->wcid = 0xff;
	}
}

#ifdef WHNAT_DBG_EN
/*
*
*/
static void wifi_dump_skb(
	struct whnat_entry *entry,
	struct wlan_tx_info *info,
	struct sk_buff *skb)
{
	struct iphdr *hdr = ip_hdr(skb);

	WHNAT_DBG(WHNAT_DBG_INF,
		"%s(): add entry: wdma=%d,ringId=%d,wcid=%d,bssid=%d\n",
		__func__,
		entry->idx,
		info->ringidx,
		info->wcid,
		info->bssidx);

	if (hdr->version != 4)
		return;

	WHNAT_DBG(WHNAT_DBG_INF,
		"%s():src=%d.%d.%d.%d\n",
		__func__,
		(0xff & hdr->saddr),
		(0xff00 & hdr->saddr) >> 8,
		(0xff0000 & hdr->saddr) >> 16,
		(0xff000000 & hdr->saddr) >> 24);

	WHNAT_DBG(WHNAT_DBG_INF,
		"%s(): dst=%d.%d.%d.%d\n",
		__func__,
		(0xff & hdr->daddr),
		(0xff00 & hdr->daddr) >> 8,
		(0xff0000 & hdr->daddr) >> 16,
		(0xff000000 & hdr->daddr) >> 24);

	if (hdr->protocol == IPPROTO_TCP) {
		struct tcphdr *tcph = tcp_hdr(skb);
		WHNAT_DBG(WHNAT_DBG_INF,
			"%s(): protocol=TCP,sport=%d,dstport=%d\n",
			__func__,
			tcph->source,
			tcph->dest);
	}

	if (hdr->protocol == IPPROTO_UDP) {
		struct udphdr *udph = udp_hdr(skb);
		WHNAT_DBG(WHNAT_DBG_INF,
			"%s(): protocol=UDP,sport=%d,dstport=%d\n",
			__func__,
			udph->source,
			udph->dest);
	}
}
#endif /*WHNAT_DBG_EN*/

/*
*
*/
void wifi_tx_tuple_add(void *entry, unsigned char *tx_info)
{
	struct whnat_entry *whnat = (struct whnat_entry *)entry;
	struct wlan_tx_info t, *info =  &t;
	struct _TX_BLK *txblk = (struct _TX_BLK *)tx_info;

	memset(info, 0, sizeof(*info));
	wifi_tx_info_wrapper(tx_info, info);
	WHNAT_DBG(WHNAT_DBG_INF, "WDMAID: %d,RingID: %d, Wcid: %d, Bssid: %d\n",
			whnat->idx, info->ringidx, info->wcid, info->bssidx);

	if (whnat && ra_sw_nat_hook_tx && whnat->cfg.hw_tx_en) {
		struct sk_buff *skb = (struct sk_buff *)info->pkt;

		if ((FOE_AI_HEAD(skb) == HIT_UNBIND_RATE_REACH) || (FOE_AI_TAIL(skb) == HIT_UNBIND_RATE_REACH)) {
			if (IS_SPACE_AVAILABLE_HEAD(skb)) {
				/*WDMA idx*/
				FOE_WDMA_ID_HEAD(skb) = whnat->idx;
				/*Ring idx*/
				FOE_RX_ID_HEAD(skb) = info->ringidx;
				/*wtable Idx*/
				FOE_WC_ID_HEAD(skb) = info->wcid;
				/*Bssidx*/
				FOE_BSS_ID_HEAD(skb) = info->bssidx;
			}
			if (IS_SPACE_AVAILABLE_TAIL(skb)) {
				/*WDMA idx*/
				FOE_WDMA_ID_TAIL(skb) = whnat->idx;
				/*Ring idx*/
				FOE_RX_ID_TAIL(skb) = info->ringidx;
				/*wtable Idx*/
				FOE_WC_ID_TAIL(skb) = info->wcid;
				/*Bssidx*/
				FOE_BSS_ID_TAIL(skb) = info->bssidx;
			}
		}

		/*use port for specify which hw_nat architecture*/
		if (ra_sw_nat_hook_tx) {
			if (ra_sw_nat_hook_tx(skb, WHNAT_WDMA_PORT) != 1) {
				txblk->DropPkt = TRUE;
			}
		}
#ifdef WHNAT_DBG_EN
		wifi_dump_skb(whnat, info, skb);
#endif /*WHNAT_DBG_EN*/
	}
}

/*
*
*/
char wifi_hw_tx_allow(void *cookie, unsigned char *tx_info)
{
	struct _TX_BLK *txblk = (struct _TX_BLK *)tx_info;
	struct wifi_dev *wdev = txblk->wdev;

	if (!wdev)
		return FALSE;

	if (wlan_operate_get_frag_thld(wdev) != DEFAULT_FRAG_THLD)
		return FALSE;

	return TRUE;
}

/*
*
*/
void wifi_dma_cfg_wrapper(int wifi_cfg, unsigned char *dma_cfg)
{
	if (wifi_cfg == -1)
		*dma_cfg = WHNAT_DMA_DISABLE;
	else {
		switch (wifi_cfg) {
		case DMA_TX_RX:
			*dma_cfg = WHNAT_DMA_TXRX;
			break;

		case DMA_TX:
			*dma_cfg = WHNAT_DMA_TX;
			break;

		case DMA_RX:
			*dma_cfg = DMA_TX_RX;
			break;
		}
	}
}

/*
*
*/
void wifi_tx_tuple_reset(void)
{
	/* FoeTblClean(); */
}

/*
*
*/
unsigned int wifi_ser_status(void *ser_ctrl)
{
#ifdef ERR_RECOVERY
	ERR_RECOVERY_CTRL_T *ctrl = (ERR_RECOVERY_CTRL_T *)ser_ctrl;

	if (ctrl)
		return ctrl->errRecovStage;

#endif /*ERR_RECOVERY*/
	return WIFI_ERR_RECOV_NONE;
}

/*
* Wifi function part
*/

/*
*
*/
void dump_wifi_value(struct wifi_entry *wifi, char *name, unsigned int addr)
{
	unsigned int value;

	WHNAT_IO_READ32(wifi, addr, &value);
	WHNAT_DBG(WHNAT_DBG_OFF, "%s\t:%x\n", name, value);
}

/*
*
*/
void wifi_dump_tx_ring_info(struct wifi_entry *wifi, unsigned char ring_id, unsigned int idx)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wifi->cookie;
	struct _PCI_HIF_T *pci_cfg = hc_get_hif_ctrl(ad->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(pci_cfg, ring_id);
	RTMP_DMACB *cb = &tx_ring->Cell[idx];

	WHNAT_DBG(WHNAT_DBG_OFF, "AllocPA\t: %pad\n", &cb->AllocPa);
	WHNAT_DBG(WHNAT_DBG_OFF, "AllocVa\t: %p\n", cb->AllocVa);
	WHNAT_DBG(WHNAT_DBG_OFF, "Size\t: %lu\n", cb->AllocSize);
	WHNAT_DBG(WHNAT_DBG_OFF, "pNdisPacket\t: %p\n", cb->pNdisPacket);
	whnat_dump_raw("WED_TX_RING", cb->AllocVa, cb->AllocSize);
}

/*
*
*/
unsigned int wifi_chip_id_get(void *cookie)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)cookie;

	return ad->ChipID;
}

/*
*
*/
unsigned int wifi_whnat_en_get(void *cookie)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)cookie;

	return ad->CommonCfg.whnat_en;
}

/*
*
*/
void wifi_whnat_en_set(void *cookie, unsigned int en)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)cookie;

	ad->CommonCfg.whnat_en = en;
}

/*
*
*/
void *wifi_get_hw_ctrl(void *cookie)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)cookie;

	return ad->hdev_ctrl;
}

/*
*
*/
void wifi_chip_cr_mirror_set(struct wifi_entry *wifi, unsigned char enable)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wifi->cookie;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);
	if (enable) {
		ops->hif_io_read32 = NULL;
		ops->hif_io_write32 = NULL;
	} else {
		ops->hif_io_read32 = whnat_hal_io_read;
		ops->hif_io_write32 = whnat_hal_io_write;
	}
}

/*
* CHIP related setting
*/
void wifi_chip_probe(struct wifi_entry *wifi, unsigned int irq)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wifi->cookie;
	struct os_cookie *os_cookie = (struct os_cookie *)ad->OS_Cookie;
	struct pci_dev *pci_dev = os_cookie->pci_dev;
	struct net_device *dev = ad->net_dev;
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(ad->hdev_ctrl);
	struct pci_hif_chip *hif_chip = pci_hif->main_hif_chip;

	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): Chang CHIP IRQ: %d to WHNAT IRQ: %d\n", __func__, pci_dev->irq, irq);
	wifi->irq = pci_dev->irq;
	pci_dev->irq = irq;
	dev->irq = irq;
	hif_chip->irq = irq;
	/*always disable hw cr mirror first */
	wifi_chip_cr_mirror_set(wifi, FALSE);
	wifi->base_addr = (unsigned long)pci_hif->CSRBaseAddress;
	wifi->int_mask = &hif_chip->int_enable_mask;
	wifi_pcie_match(wifi);

	ad->CommonCfg.wed_version = 0x10000000;
}

/*
*
*/
void wifi_chip_remove(struct wifi_entry *wifi)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wifi->cookie;
	struct os_cookie *os_cookie = (struct os_cookie *)ad->OS_Cookie;
	struct pci_dev *pci_dev = os_cookie->pci_dev;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): Chang WED IRQ: %d to CHIP IRQ: %d\n", __func__, pci_dev->irq, wifi->irq);
	/*revert pci irq as original irq*/
	pci_dev->irq = wifi->irq;
	wifi->irq = 0;
	ops->hif_io_read32 = NULL;
	ops->hif_io_write32 = NULL;
	wifi->base_addr = 0;
	wifi->int_mask = NULL;
}

/*
*
*/
int wifi_slot_get(void *cookie)
{
	struct _RTMP_ADAPTER *ad = (RTMP_ADAPTER *) cookie;
	struct os_cookie *os_cookie = (struct os_cookie *)ad->OS_Cookie;
	struct pci_dev *pci_dev = os_cookie->pci_dev;
	unsigned int id = 1;
	if (pci_dev->bus) {
		id = (pci_dev->bus->self->devfn >> 3) & 0x1f;
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): bus name=%s, funid=%d, get slot id=%d\n",
			__func__,
			pci_dev->bus->name,
			pci_dev->bus->self->devfn,
			id);
	}
	return id;
}



/*
*
*/
unsigned int wifi_wpdma_base_get(void *cookie)
{
	struct _RTMP_ADAPTER *ad = (RTMP_ADAPTER *) cookie;
	struct os_cookie *os_cookie = (struct os_cookie *)ad->OS_Cookie;
	struct pci_dev *pci_dev = os_cookie->pci_dev;
	unsigned int wpdma_base = 0;

	if (pci_dev->bus) {
		wpdma_base = (unsigned int) pci_resource_start(pci_dev, 0);
		wpdma_base |= WPDMA_OFFSET;
	}
	return wpdma_base;
}


/*
*
*/
void wifi_cap_get(struct wifi_entry *wifi)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wifi->cookie;
	struct	_PCI_HIF_T *pci_hif = hc_get_hif_ctrl(ad->hdev_ctrl);
	struct pci_hif_chip *hif_chip = pci_hif->main_hif_chip;
	const struct hif_pci_tx_ring_desc *tx_ring_layout = hif_chip->ring_layout.tx_ring_layout;
	unsigned int tx_ring_num = hif_chip->tx_res_num;
	unsigned int data_tx_ring_num = 0;
	unsigned int data_tx_ring_len = 0;
	unsigned char i;

	for (i = 0; i < tx_ring_num; i++) {
		struct hif_pci_tx_ring_desc tx_ring_desc = tx_ring_layout[i];
		if (tx_ring_desc.ring_attr == HIF_TX_DATA) {
			data_tx_ring_num++;

			if (tx_ring_desc.ring_size > data_tx_ring_len)
				data_tx_ring_len = tx_ring_desc.ring_size;
		}
	}

	wifi->tx_ring_num = data_tx_ring_num;
	wifi->tx_ring_len = data_tx_ring_len;
	wifi->tx_token_nums = hc_get_chip_tx_token_nums(ad->hdev_ctrl);
	wifi->sw_tx_token_nums = hc_get_chip_sw_tx_token_nums(ad->hdev_ctrl);
}

#endif /*MT7615*/
