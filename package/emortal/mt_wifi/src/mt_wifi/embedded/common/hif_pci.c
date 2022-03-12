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
/***************************************************************************
 ***************************************************************************

*/

#include "rt_config.h"
#include "common/link_list.h"
#include "multi_hif.h"
#ifdef RX_RPS_SUPPORT
#include "mac/mac_mt/fmac/mt_fmac.h"
#endif

#ifdef CONFIG_WIFI_MSI_SUPPORT
#include "chip/mt7915_cr.h"
#endif

/*local func.*/
/*internal io read/write*/
/*Read Ops*/
/*
*
*/
static VOID int_pci_io_force_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	*val = readl((void *)(pci_hif->CSRBaseAddress + reg));
}

/*
*
*/
static VOID int_pci_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		*val = readl((void *)(pci_hif->CSRBaseAddress + reg));
	else
		*val = 0;
}

/*Write Ops*/
/*
*
*/
static VOID int_pci_io_force_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	writel(val, (void *)(pci_hif->CSRBaseAddress + reg));
}

/*
*
*/
static VOID int_pci_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		writel(val, (void *)(pci_hif->CSRBaseAddress + reg));
}

/*handle specific CR and forwarding to platform handle*/
#if defined(WHNAT_SUPPORT)
/*
*
*/
static VOID hook_pci_io_read32(void *hdev_ctrl, UINT32 addr, UINT32 *value)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);

	if (ops->hif_io_read32) {
		if (ops->hif_io_read32(hdev_ctrl, addr, value) >= 0)
			return;
	}

	int_pci_io_read32(hdev_ctrl, addr, value);
}

/*
*
*/
static VOID hook_pci_io_write32(void *hdev_ctrl, UINT32 addr, UINT32 value)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);

	if (ops->hif_io_write32) {
		if (ops->hif_io_write32(hdev_ctrl, addr, value) >= 0)
			return;
	}
	int_pci_io_write32(hdev_ctrl, addr, value);
}
#endif /*WHNAT_SUPPORT*/

/*
*
*/
static VOID hook_pci_io_remap_read32(void *hdev_ctrl, UINT32 addr, UINT32 *value)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	ULONG flags = 0;

	if (ops->hif_io_remap_read32) {
		RTMP_SPIN_LOCK_IRQSAVE(&pci_hif->io_remap_lock, &flags);
		ops->hif_io_remap_read32(hdev_ctrl, addr, value);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pci_hif->io_remap_lock, &flags);
		return;
	}
}

/*
*
*/
static VOID hook_pci_io_remap_write32(void *hdev_ctrl, UINT32 addr, UINT32 value)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	ULONG flags = 0;

	if (ops->hif_io_remap_write32) {
		RTMP_SPIN_LOCK_IRQSAVE(&pci_hif->io_remap_lock, &flags);
		ops->hif_io_remap_write32(hdev_ctrl, addr, value);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pci_hif->io_remap_lock, &flags);
		return;
	}
}

/*
*
*/
static VOID pci_map_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);

	if (mt_mac_cr_range_mapping(ad, &reg) == TRUE) {

		if (pci_hif->bPCIclkOff == FALSE)
			*val = readl((void *)(pci_hif->CSRBaseAddress + reg));
		else {
			*val = 0;
		}
	} else {
		hook_pci_io_remap_read32(hdev_ctrl, reg, val);
	}
}

/*
*
*/
static VOID pci_map_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);

	if (mt_mac_cr_range_mapping(ad, &reg) == TRUE) {

		if (pci_hif->bPCIclkOff == FALSE)
			writel(val, (void *)(pci_hif->CSRBaseAddress + reg));
	} else {
		hook_pci_io_remap_write32(hdev_ctrl, reg, val);
	}
}

/*
*
*/
static VOID int_pci_io_ops_init(void *hdev_ctrl)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);
#ifdef WHNAT_SUPPORT
	UINT32 cap = hc_get_asic_cap(hdev_ctrl);
#endif /*WHNAT_SUPPORT*/
	/*hif*/
	ops->hif_io_forec_read32 = int_pci_io_force_read32;
	ops->hif_io_forec_write32 = int_pci_io_force_write32;
#ifdef WHNAT_SUPPORT
	if (cap & fASIC_CAP_WHNAT) {
		ops->hif_io_read32 = hook_pci_io_read32;
		ops->hif_io_write32 = hook_pci_io_write32;
	} else
#endif
	{
		ops->hif_io_read32 = int_pci_io_read32;
		ops->hif_io_write32 = int_pci_io_write32;
	}
	/*mac*/
	ops->mac_io_read32 = pci_map_io_read32;
	ops->mac_io_write32 = pci_map_io_write32;
	/*phy*/
	ops->phy_io_read32 = int_pci_io_read32;
	ops->phy_io_write32 = int_pci_io_write32;
	/*hw*/
	ops->hw_io_read32 = pci_map_io_read32;
	ops->hw_io_write32 = pci_map_io_write32;
	/*mcu*/
	ops->mcu_io_read32 = CmdIORead32;
	ops->mcu_io_write32 = CmdIOWrite32;
}

/*mstar Platform*/
#if defined(PLATFORM_M_STB)
/*
*
*/
static VOID mstar_pci_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	struct os_cookie *cookie = hc_get_os_cookie(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		pci_read_config_dword(cookie->pci_dev, reg + 0x80000000, val);
	else
		*val = 0;
}

/*
*
*/
static VOID mstar_pci_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	struct os_cookie *cookie = hc_get_os_cookie(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		pci_write_config_dword(cookie->pci_dev, reg + 0x80000000, val);
}

/*
*
*/
static VOID mstar_pci_io_read16(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	struct os_cookie *cookie = hc_get_os_cookie(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		pci_read_config_word(cookie->pci_dev, reg + 0x80000000, val);
	else
		*val = 0;
}

/*
*
*/
static VOID mstar_pci_io_write16(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	struct os_cookie *cookie = hc_get_os_cookie(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		pci_write_config_word(cookie->pci_dev, reg + 0x80000000, val);
}

/*
*
*/
static VOID mstar_pci_io_read8(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	struct os_cookie *cookie = hc_get_os_cookie(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		pci_read_config_byte(cookie->pci_dev, reg + 0x80000000, val);
	else
		*val = 0;
}

/*
*
*/
static VOID mstar_pci_io_write8(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	struct os_cookie *cookie = hc_get_os_cookie(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE)
		pci_write_config_byte(cookie->pci_dev, reg + 0x80000000, val);
}

/*
*
*/
static VOID mstar_pci_io_ops_init(void *hdev_ctrl)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	/*hif*/
	ops->hif_io_read32 = mstar_pci_io_read32;
	ops->hif_io_write32 = mstar_pci_io_write32;
	ops->hif_io_forec_read32 = NULL;
	ops->hif_io_forec_write32 = NULL;
	/*mac*/
	ops->mac_io_read32 = mstar_pci_io_read32;
	ops->mac_io_write32 = mstar_pci_io_write32;
	/*phy*/
	ops->phy_io_read32 = mstar_pci_io_read32;
	ops->phy_io_write32 = mstar_pci_io_write32;
}
#endif /*PLATFORM_M_STB*/

#if defined(INF_TWINPASS) || defined(INF_DANUBE) || defined(INF_AR9) || defined(IKANOS_VX_1X0)

/*
*
*/
static VOID ext_pci_forec_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	*val = readl((void *)((pci_hif->CSRBaseAddress + reg)));
	*val = SWAP32(*((UINT32 *)(val)));
}

/*
*
*/
static VOID ext_pci_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE) {
		*val = readl((void *)((pci_hif->CSRBaseAddress + reg)));
		*val = SWAP32(*((UINT32 *)(val)));
	}
}

/*
*
*/
static VOID ext_pci_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	if (pci_hif->bPCIclkOff == FALSE) {
		val = SWAP32(val);
		writel(val, (pci_hif->CSRBaseAddress + reg));
	}
}

/*
*
*/
static VOID ext_pci_io_ops_init(void *hdev_ctrl)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	/*hif*/
	ops->hif_io_read32 = ext_pci_io_read32;
	ops->hif_io_write32 = ext_pci_io_write32;
	ops->hif_io_forec_read32 = ext_pci_forec_read32;
	ops->hif_io_forec_write32 = NULL;
	/*mac*/
	ops->mac_io_read32 = ext_pci_io_read32;
	ops->mac_io_write32 = ext_pci_io_write32;
	/*phy*/
	ops->phy_io_read32 = ext_pci_io_read32;
	ops->phy_io_write32 = ext_pci_io_write32;
	/*hw*/
	ops->hw_io_read32 = ext_pci_io_read32;
	ops->hw_io_write32 = ext_pci_io_write32;
}
#endif

/*
*
*/
static VOID pci_io_ops_register(void *hdev_ctrl)
{
#if defined(PLATFORM_M_STB)
	mstar_pci_io_ops_init(hdev_ctrl);
#elif defined(INF_TWINPASS) || defined(INF_DANUBE) || defined(INF_AR9) || defined(IKANOS_VX_1X0)
	ext_pci_io_ops_init(hdev_ctrl);
#else
	int_pci_io_ops_init(hdev_ctrl);
#endif
}

/*
*
*/
static INT desc_ring_alloc(VOID *dev, RTMP_DMABUF *pDescRing, INT size)
{
	pDescRing->AllocSize = size;
	RtmpAllocDescBuf(dev,
					 0,
					 pDescRing->AllocSize,
					 FALSE,
					 &pDescRing->AllocVa,
					 &pDescRing->AllocPa);

	if (pDescRing->AllocVa == NULL) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Failed to allocate a big buffer\n"));
		return ERRLOG_OUT_OF_SHARED_MEMORY;
	}

	/* Zero init this memory block*/
	NdisZeroMemory(pDescRing->AllocVa, size);
	return 0;
}

inline VOID *alloc_rx_buf_1k(void *hif_resource)
{
	struct hif_pci_rx_ring *rx_ring = (struct hif_pci_rx_ring *)hif_resource;
	RTMP_DMABUF *dma_buf = &rx_ring->free_buf[rx_ring->free_buf_head];

	INC_INDEX(rx_ring->free_buf_head, rx_ring->free_buf_size);

	return dma_buf->AllocVa;
}

inline VOID free_rx_buf_1k(void *hif_resource)
{
	struct hif_pci_rx_ring *rx_ring = (struct hif_pci_rx_ring *)hif_resource;

	INC_INDEX(rx_ring->free_buf_tail, rx_ring->free_buf_size);
}

inline VOID *alloc_rx_buf_64k(void *hif_resource)
{
	struct hif_pci_rx_ring *rx_ring = (struct hif_pci_rx_ring *)hif_resource;
	RTMP_DMABUF *dma_buf = &rx_ring->free_buf_64k[rx_ring->free_buf_64k_head];

	INC_INDEX(rx_ring->free_buf_64k_head, rx_ring->free_buf_64k_size);

	return dma_buf->AllocVa;
}

inline VOID free_rx_buf_64k(void *hif_resource)
{
	struct hif_pci_rx_ring *rx_ring = (struct hif_pci_rx_ring *)hif_resource;

	INC_INDEX(rx_ring->free_buf_64k_tail, rx_ring->free_buf_64k_size);
}

/*
*
*/
static UINT32 alloc_rx_free_buffer(struct pci_hif_chip *hif, UCHAR resource_idx, UINT32 num)
{
	UINT32 i;
	struct hif_pci_rx_ring *rx_ring = &hif->RxRing[resource_idx];
	RTMP_DMABUF *dma_buf;

	rx_ring->free_buf_head = 0;
	rx_ring->free_buf_tail = 0;
	rx_ring->free_buf_size = (num + 1);

	os_alloc_mem(NULL, (UCHAR **)&rx_ring->free_buf, rx_ring->free_buf_size * sizeof(struct _RTMP_DMABUF));
	NdisZeroMemory(rx_ring->free_buf, rx_ring->free_buf_size * sizeof(struct _RTMP_DMABUF));

	for (i = 0; i < rx_ring->free_buf_size; i++) {
		dma_buf = &rx_ring->free_buf[i];
		dma_buf->AllocSize = RX1_BUFFER_SIZE;
		os_alloc_mem(NULL, (UCHAR **)&dma_buf->AllocVa, dma_buf->AllocSize);
		NdisZeroMemory(dma_buf->AllocVa, dma_buf->AllocSize);
	}

	rx_ring->free_buf_64k_head = 0;
	rx_ring->free_buf_64k_tail = 0;
	rx_ring->free_buf_64k_size = 1;

	os_alloc_mem(NULL, (UCHAR **)&rx_ring->free_buf_64k, rx_ring->free_buf_64k_size * sizeof(struct _RTMP_DMABUF));
	NdisZeroMemory(rx_ring->free_buf_64k, rx_ring->free_buf_64k_size * sizeof(struct _RTMP_DMABUF));

	for (i = 0; i < rx_ring->free_buf_64k_size; i++) {
		dma_buf = &rx_ring->free_buf_64k[i];
		dma_buf->AllocSize = 65536;
		os_alloc_mem(NULL, (UCHAR **)&dma_buf->AllocVa, dma_buf->AllocSize);
		NdisZeroMemory(dma_buf->AllocVa, dma_buf->AllocSize);
	}

	return 0;
}

/*
*
*/
static UINT32 reset_rx_free_buffer(void *hdev_ctrl, UINT8 resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(hif, resource_idx);
	UINT32 num = rx_ring->ring_size;
	UINT32 i;
	RTMP_DMABUF *dma_buf;

	rx_ring->free_buf_head = 0;
	rx_ring->free_buf_tail = 0;
	rx_ring->free_buf_size = (num + 1);

	for (i = 0; i < rx_ring->free_buf_size; i++) {
		dma_buf = &rx_ring->free_buf[i];
		dma_buf->AllocSize = RX1_BUFFER_SIZE;
	}

	rx_ring->free_buf_64k_head = 0;
	rx_ring->free_buf_64k_tail = 0;
	rx_ring->free_buf_64k_size = 1;

	for (i = 0; i < rx_ring->free_buf_64k_size; i++) {
		dma_buf = &rx_ring->free_buf_64k[i];
		dma_buf->AllocSize = 65536;
	}

	return 0;
}

/*
*
*/
static UINT32 release_rx_free_buffer(struct pci_hif_chip *chip_hif, UINT8 resource_idx)
{
	UINT32 i;
	struct hif_pci_rx_ring *rx_ring = &chip_hif->RxRing[resource_idx];
	RTMP_DMABUF *dma_buf;

	for (i = 0; i < rx_ring->free_buf_size; i++) {
		dma_buf = &rx_ring->free_buf[i];
		os_free_mem(dma_buf->AllocVa);
	}

	os_free_mem(rx_ring->free_buf);

	for (i = 0; i < rx_ring->free_buf_64k_size; i++) {
		dma_buf = &rx_ring->free_buf_64k[i];
		os_free_mem(dma_buf->AllocVa);
	}

	os_free_mem(rx_ring->free_buf_64k);

	return 0;
}

/*
*
*/
static INT desc_ring_free(struct pci_hif_chip *chip_hif, RTMP_DMABUF *pDescRing)
{
	if (pDescRing->AllocVa) {
		RtmpFreeDescBuf(chip_hif->pdev,
						pDescRing->AllocSize,
						pDescRing->AllocVa,
						pDescRing->AllocPa);
	}

	NdisZeroMemory(pDescRing, sizeof(RTMP_DMABUF));
	return TRUE;
}

/*
*
*/
static UINT32 pci_get_tx_free_num(struct hif_pci_tx_ring *ring)
{
	UINT32 free_num;
	ULONG flags;
	NDIS_SPIN_LOCK *lock = &ring->tx_lock;

	RTMP_IRQ_LOCK(lock, flags);
	free_num = ((ring->TxSwFreeIdx > ring->TxCpuIdx) ?
		(ring->TxSwFreeIdx - ring->TxCpuIdx - 1) :
		(ring->TxSwFreeIdx + ring->ring_size - ring->TxCpuIdx - 1));
	RTMP_IRQ_UNLOCK(lock, flags);
	return free_num;
}

VOID pci_rx_all(struct _PCI_HIF_T *pci_hif)
{
	struct _RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(pci_hif->net_dev);
	UINT32 i;

	for (i = 0; i < pci_hif->rx_res_num; i++) {
		struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(pci_hif, i);

		pci_hif->dma_done_handle[rx_ring->ring_attr](pAd, rx_ring->resource_idx);
	}
}

/*
*
*/
static BOOLEAN pci_tx_dma_done_handle(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	struct qm_ops *qm_ops = pAd->qm_ops;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);
	UINT free_num;

	hif_free_txd(pAd, resource_idx);

	free_num = pci_get_tx_resource_free_num_nolock(pAd, resource_idx);
	if (free_num >= tx_ring->tx_ring_high_water_mark &&
			pci_get_resource_state(pAd, resource_idx)) {
		pci_set_resource_state(pAd, resource_idx, TX_RING_HIGH);
		qm_ops->schedule_tx_que(pAd, tx_ring->band_idx);
	}

	return FALSE;
}

/*
*
*/
static BOOLEAN pci_cmd_dma_done_handle(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	struct hif_pci_tx_ring *pCtrlRing = pci_get_tx_ring_by_ridx(hif, resource_idx);

	NdisAcquireSpinLock(&pCtrlRing->tx_done_lock);
	HIF_IO_READ32(pAd->hdev_ctrl, pCtrlRing->hw_didx_addr, &pCtrlRing->TxDmaIdx);

	while (pCtrlRing->TxSwFreeIdx != pCtrlRing->TxDmaIdx) {
#ifdef RT_BIG_ENDIAN
		pDestTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
		NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&hw_hdr_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
#endif
		pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket;

		if (pPacket == NULL) {
			INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, pCtrlRing->ring_size);
			continue;
		}

		if (pPacket) {
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RTMPFreeNdisPacket(pAd, pPacket);
		}

		pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket = NULL;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocPa, TXD_SIZE);
		INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, pCtrlRing->ring_size);

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
	}

	NdisReleaseSpinLock(&pCtrlRing->tx_done_lock);

	return FALSE;
}

/*
*
*/
static BOOLEAN pci_fwdl_dma_done_handle(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	struct hif_pci_tx_ring *pRing = pci_get_tx_ring_by_ridx(hif, resource_idx);

	NdisAcquireSpinLock(&pRing->tx_done_lock);
	HIF_IO_READ32(pAd->hdev_ctrl, pRing->hw_didx_addr, &pRing->TxDmaIdx);

	while (pRing->TxSwFreeIdx != pRing->TxDmaIdx) {
#ifdef RT_BIG_ENDIAN
		pDestTxD = (TXD_STRUC *)(pRing->Cell[pRing->TxSwFreeIdx].AllocVa);
		NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&hw_hdr_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (TXD_STRUC *)(pRing->Cell[pRing->TxSwFreeIdx].AllocVa);
#endif
		pPacket = pRing->Cell[pRing->TxSwFreeIdx].pNdisPacket;

		if (pPacket == NULL) {
			INC_RING_INDEX(pRing->TxSwFreeIdx, pRing->ring_size);
			continue;
		}

		if (pPacket) {
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RTMPFreeNdisPacket(pAd, pPacket);
		}

		pRing->Cell[pRing->TxSwFreeIdx].pNdisPacket = NULL;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRing->Cell[pRing->TxSwFreeIdx].AllocPa, TXD_SIZE);
		INC_RING_INDEX(pRing->TxSwFreeIdx, pRing->ring_size);
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
	}

	NdisReleaseSpinLock(&pRing->tx_done_lock);
	return FALSE;
}

VOID dump_rxd(RTMP_ADAPTER *pAd, RXD_STRUC *pRxD)
{
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("RxD:\n"));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tSDPtr0/SDLen0/LastSec0=0x%x/0x%x/0x%x\n",
			 pRxD->SDP0, pRxD->SDL0, pRxD->LS0));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tSDPtr1/SDLen1/LastSec1=0x%x/0x%x/0x%x\n",
			 pRxD->SDP1, pRxD->SDL1, pRxD->LS1));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tDDONE=0x%x\n", pRxD->DDONE));
}

VOID dump_rxd_debug(RTMP_ADAPTER *pAd, RXD_DEBUG_STRUC *pRxD)
{
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("RxD:\n"));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tSDPtr0/SDLen0/LastSec0=0x%x/0x%x/0x%x\n",
			 pRxD->SDP0, pRxD->SDL0, pRxD->LS0));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tQID/DIDX/CIDX/SW_INFO=0x%x/0x%x/0x%x/0x%x\n",
			 pRxD->QID, pRxD->DIDX, pRxD->CIDX, pRxD->SW_INFO));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tDDONE=0x%x\n", pRxD->DDONE));
}

VOID dump_rxd_wed(RTMP_ADAPTER *pAd, RXD_STRUC *pRxD)
{
	UINT32 *dw0 = (UINT32 *)((UCHAR *)pRxD);
	UINT32 *dw1 = (UINT32 *)((UCHAR *)pRxD + 4);
	UINT32 *dw2 = (UINT32 *)((UCHAR *)pRxD + 8);
	UINT32 *dw3 = (UINT32 *)((UCHAR *)pRxD + 12);

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("RxD:\n"));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tRxD DW0: 0x%x\n",
				*dw0));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tRxD DW1: 0x%x\n",
				*dw1));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tRxD DW2: 0x%x\n",
				*dw2));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("\tRxD DW3: 0x%x\n",
				*dw3));
}

static INT32 dump_rx_info(struct _RTMP_ADAPTER *pAd, RTMP_DMACB *rx_cell,
					RXD_DEBUG_STRUC *pRxD, UCHAR resource_idx, PNDIS_PACKET pRxPacket)
{
	INT32 status = NDIS_STATUS_SUCCESS;
	UINT32 rx_pkt_type;
	enum resource_attr res_attr = hif_get_resource_type(pAd->hdev_ctrl, resource_idx);
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(pci_hif, resource_idx);
	ULONG cpu_addr;

	if (res_attr == HIF_RX_DATA) {

		rx_pkt_type = asic_get_packet_type(pAd, GET_OS_PKT_DATAPTR(pRxPacket));

		if ((rx_pkt_type == RMAC_RX_PKT_TYPE_RX_TXS) ||
			(rx_pkt_type == RMAC_RX_PKT_TYPE_RX_TXRXV) ||
			(rx_pkt_type == RMAC_RX_PKT_TYPE_RX_TMR)) {

				if (rx_ring->buf_debug & BUF_RX_RXD) {
					dump_rxd_debug(pAd, pRxD);
				}

				if (rx_ring->buf_debug & BUF_RX_ADDR) {
					/*
					MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,
								("RX Buffer Phy/Vir = 0x%llx/0x%p\n",
								rx_cell->DmaBuf.AllocPa, rx_cell->DmaBuf.AllocVa));
					*/
				}

				if (rx_ring->buf_debug & BUF_RX_PAYLOAD) {
					hex_dump("RX BufferA:", (PUCHAR)GET_OS_PKT_DATAPTR(pRxPacket), GET_OS_PKT_LEN(pRxPacket));
				}

				if (rx_ring->buf_debug & BUF_RX_HW_PAYLOAD) {
					cpu_addr = (ULONG)phys_to_virt(pRxD->SDP0);
					hex_dump("RX BufferB:", (PUCHAR)cpu_addr, pRxD->SDL0);
				}

				status = NDIS_STATUS_FAILURE;
		}
	}

	return status;
}

static PNDIS_PACKET expand_pkt_dynamic_page(
	struct _RTMP_ADAPTER *pAd,
	void *hif_resource,
	PNDIS_PACKET original_pkt,
	UINT32 original_pkt_len,
	UINT32 new_pkt_len,
	UINT *new_skb_len)
{
	PNDIS_PACKET *new_pkt = NULL;
	UINT32 skb_len;

	/*
	 * calculate new skb data size
	 * SKB_BUF_HEADROOM_RSV = NET_SKB_PAD + NET_IP_ALIGN
	 * SKB_BUF_TAILROOM_RSV = sizeof(struct skb_shared_info)
	 * skb_size = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + ext_tail_len) +
	 *		SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV)
	 * if skb_size <= PAGE_SIZE use DEV_ALLOC_FRAG else use kmalloc
	 * to alocate skb data size
	 */
	skb_len = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + new_pkt_len)
			+ SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV);

	if (skb_len <= PAGE_SIZE)
		DEV_ALLOC_FRAG(new_pkt, skb_len);
	else
		os_alloc_mem(pAd, (PUCHAR *)&new_pkt, skb_len);

	*new_skb_len = skb_len;

	if (new_pkt && original_pkt)
		os_move_mem(((PVOID)new_pkt + SKB_BUF_HEADROOM_RSV),
			((PVOID)original_pkt + SKB_BUF_HEADROOM_RSV), original_pkt_len);

	if (original_pkt)
		DEV_FREE_FRAG_BUF(original_pkt);

	if (!new_pkt) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("Extend Rx buffer %d size packet failed! drop pkt.\n",
			(new_pkt_len)));
			return NULL;
	}

	return new_pkt;
}

static PNDIS_PACKET expand_pkt_pre_slab(
	struct _RTMP_ADAPTER *pAd,
	void *hif_resource,
	PNDIS_PACKET original_pkt,
	UINT32 original_pkt_len,
	UINT32 new_pkt_len)
{
	PNDIS_PACKET *new_pkt = NULL;

	new_pkt = alloc_rx_buf_64k(hif_resource);

	if (new_pkt && original_pkt) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s, original_pkt_len = %d\n", __func__, original_pkt_len));
		os_move_mem((PUCHAR)new_pkt, ((PUCHAR)original_pkt), original_pkt_len);
	}

	if (original_pkt)
		free_rx_buf_1k(hif_resource);

	if (!new_pkt) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("Extend Rx buffer %d size packet failed! drop pkt.\n",
			(new_pkt_len)));
			return NULL;
	}

	return new_pkt;
}

static INT rx_scatter_info(
	struct hif_pci_rx_ring *pRxRing,
	struct _RXD_STRUC *pRxD,
	UINT *pPktSize)
{
	UINT LoopCnt;
	INT32 RxCellIdx;
	RTMP_DMACB *pCurRxCell = NULL;
	RXD_STRUC *pCurRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif

	LoopCnt = 0;

	*pPktSize = 0;
	RxCellIdx =  pRxRing->RxSwReadIdx;

	/* walk through rx-ring and find the rx-cell content LS0 to be 1. */
	do {
		pCurRxCell = &pRxRing->Cell[RxCellIdx];
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pCurRxCell->AllocPa, RXD_SIZE);
		/* Point to Rx indexed rx ring descriptor */
#ifdef RT_BIG_ENDIAN
		pDestRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
		NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
		pCurRxD = (RXD_STRUC *)&rx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
#else
		pCurRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
#endif
		if (pCurRxD->DDONE == 0) {
			LoopCnt = 0;
			break;
		}

		*pPktSize += pCurRxD->SDL0;
		LoopCnt++;

		/* find the last pice of rx scattering. */
		if (pCurRxD->LS0 == 1) {
			break;
		}

		INC_RING_INDEX(RxCellIdx, pRxRing->ring_size);
	} while (TRUE);

	return LoopCnt;
}

static INT rx_scatter_info_io(
	struct hif_pci_rx_ring *pRxRing,
	RXD_STRUC *pRxD,
	UINT *pPktSize)
{
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	UINT LoopCnt;
	INT32 RxCellIdx;
	RTMP_DMACB *pCurRxCell = NULL;
	RXD_STRUC *pCurRxD;
	UINT isLsPktFound = FALSE;

	LoopCnt = 0;

	*pPktSize = 0;
	RxCellIdx =  pRxRing->RxSwReadIdx;

	/* walk through rx-ring and find the rx-cell content LS0 to be 1. */
	do {
		if (RxCellIdx == pRxRing->RxDmaIdx) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: RxCellIdx == pRxRing->RxDmaIdx\n", __func__));
			if (pRxD->LS0 == 0) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxD->LS0 == 0\n", __func__));
				LoopCnt = 0;
			}

			break;
		}

		pCurRxCell = &pRxRing->Cell[RxCellIdx];
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pCurRxCell->AllocPa, RXD_SIZE);
#ifdef RT_BIG_ENDIAN
		pDestRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
		/* RxD = *pDestRxD; */
		NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
		pCurRxD = (RXD_STRUC *)&rx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
#else
		/* Point to Rx indexed rx ring descriptor */
		pCurRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
#endif

		if (pCurRxD->DDONE == 0) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pCurRxD->DDONE == 0 \n", __func__));
			break;
		}

		*pPktSize += pCurRxD->SDL0;
		LoopCnt++;
		INC_RING_INDEX(RxCellIdx, pRxRing->ring_size);

		/* find the last pice of rx scattering. */
		if (pCurRxD->LS0 == 1) {
			isLsPktFound = TRUE;
			break;
		}
	} while (TRUE);

	return LoopCnt;
}

static INT rx_scatter_gather_dynamic_page(
	RTMP_ADAPTER *pAd,
	UCHAR resource_idx,
	RTMP_DMACB *pRxCell,
	RXD_STRUC *pRxD,
	UINT scatterCnt,
	UINT gather_size,
	UINT *build_skb_len)
{

	UINT LoopCnt;
	INT32 RxCellIdx;
	UINT32 buf_idx;
	RTMP_DMACB *pCurRxCell = NULL;
	RXD_STRUC *pCurRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif

	PNDIS_PACKET pRxCellPacket;
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(pci_hif, resource_idx);

	if (rx_ring == NULL || pRxCell == NULL || pRxD == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxRing == NULL || pRxCell == NULL\
					|| pRxD == NULL\n", __func__));
		return FALSE;
	}

	/* keep pRxCell value and replace pRxCell->pNdisPacket with expand skb buffer. */
	pRxCellPacket = pRxCell->pNdisPacket;

	if (pRxCellPacket == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxCellPacket == NULL\n", __func__));
		return FALSE;
	}
	buf_idx = pRxD->SDL0;

	pRxCell->pNdisPacket = expand_pkt_dynamic_page(pAd, rx_ring,
					pRxCellPacket, buf_idx, gather_size, build_skb_len);


	buf_idx += (SKB_BUF_HEADROOM_RSV);

	if (pRxCell->pNdisPacket != NULL) {
		pRxCell->DmaBuf.AllocVa = (PVOID)(pRxCell->pNdisPacket)
		+ SKB_BUF_HEADROOM_RSV;
	}

	if (pRxCell->pNdisPacket == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxCell->pNdisPacket == NULL\n", __func__));
		return TRUE;
	}

	RxCellIdx =  rx_ring->RxSwReadIdx;

	for (LoopCnt = 0; LoopCnt < (scatterCnt - 1); LoopCnt++) {
		INC_RING_INDEX(RxCellIdx, rx_ring->ring_size);
		pCurRxCell = &rx_ring->Cell[RxCellIdx];
		/* return tokens of all rx scatter-gather cells. */
		PCI_UNMAP_SINGLE(pAd, pCurRxCell->DmaBuf.AllocPa,
						 pCurRxCell->DmaBuf.AllocSize,
						 RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pCurRxCell->DmaBuf.AllocPa, pCurRxCell->DmaBuf.AllocSize);
		/* Point to Rx indexed rx ring descriptor */
#ifdef RT_BIG_ENDIAN
		pDestRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
		NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
		pCurRxD = (RXD_STRUC *)&rx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
#else
		pCurRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
#endif
		memcpy((pRxCell->pNdisPacket + buf_idx),
		    (VOID *)(pCurRxCell->DmaBuf.AllocVa), pCurRxD->SDL0);

		buf_idx += pCurRxD->SDL0;

		/* update done bit of all rx scatter-gather cells to zero. */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pCurRxD->SDL0 = %d, \n", __func__, pCurRxD->SDL0));

		DEV_FREE_FRAG_BUF(pCurRxCell->pNdisPacket);

		pCurRxCell->pNdisPacket = RTMP_AllocateRxPacketBuffer(rx_ring,
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				pCurRxCell->DmaBuf.AllocSize,
				&pCurRxCell->DmaBuf.AllocVa, &pCurRxCell->DmaBuf.AllocPa);

		pCurRxD->SDP0 = pCurRxCell->DmaBuf.AllocPa;
		pCurRxD->SDL0 = rx_ring->RxBufferSize;
		pCurRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pCurRxD, FALSE, TYPE_RXD);
#endif
	}

	/* update pRxRing->RxSwReadIdx do last cell of rx scatter-gather. */
	rx_ring->RxSwReadIdx = RxCellIdx;
	return TRUE;
}

static INT rx_scatter_gather_dynamic_slab(
	RTMP_ADAPTER *pAd,
	UCHAR resource_idx,
	RTMP_DMACB *pRxCell,
	RXD_STRUC *pRxD,
	UINT scatterCnt,
	UINT gather_size)
{

	UINT LoopCnt;
	INT32 RxCellIdx;
	RTMP_DMACB *pCurRxCell = NULL;
	RXD_STRUC *pCurRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif

	PNDIS_PACKET pRxCellPacket;
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(pci_hif, resource_idx);

	if (rx_ring == NULL || pRxCell == NULL || pRxD == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxRing == NULL || pRxCell == NULL\
					|| pRxD == NULL\n", __func__));
		return FALSE;
	}

	/* keep pRxCell value and replace pRxCell->pNdisPacket with expand skb buffer. */
	pRxCellPacket = pRxCell->pNdisPacket;

	if (pRxCellPacket == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxCellPacket == NULL\n", __func__));
		return FALSE;
	}

	OS_PKT_TAIL_BUF_EXTEND(pRxCellPacket, pRxD->SDL0);

	pRxCell->pNdisPacket = ExpandPacket(NULL, pRxCellPacket, gather_size, 0);

	if (pRxCell->pNdisPacket != NULL)
		pRxCell->DmaBuf.AllocVa = GET_OS_PKT_DATAPTR(pRxCell->pNdisPacket);
	else
		pRxCell->DmaBuf.AllocVa = NULL;

	if (pRxCell->pNdisPacket == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxCell->pNdisPacket == NULL\n", __func__));
		return TRUE;
	}

	RxCellIdx =  rx_ring->RxSwReadIdx;

	for (LoopCnt = 0; LoopCnt < (scatterCnt - 1); LoopCnt++) {
		INC_RING_INDEX(RxCellIdx, rx_ring->ring_size);
		pCurRxCell = &rx_ring->Cell[RxCellIdx];
		/* return tokens of all rx scatter-gather cells. */
		PCI_UNMAP_SINGLE(pAd, pCurRxCell->DmaBuf.AllocPa,
						 pCurRxCell->DmaBuf.AllocSize,
						 RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pCurRxCell->DmaBuf.AllocPa, pCurRxCell->DmaBuf.AllocSize);
		/* Point to Rx indexed rx ring descriptor */
#ifdef RT_BIG_ENDIAN
		pDestRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
		/* RxD = *pDestRxD; */
		NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
		pCurRxD = (RXD_STRUC *)&rx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
#else
		pCurRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
#endif
		memcpy(OS_PKT_TAIL_BUF_EXTEND(pRxCell->pNdisPacket, pCurRxD->SDL0),
			   (VOID *)(pCurRxCell->DmaBuf.AllocVa), pCurRxD->SDL0);

		/* update done bit of all rx scatter-gather cells to zero. */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pCurRxD->SDL0 = %d, \n", __func__, pCurRxD->SDL0));

		DEV_FREE_FRAG_BUF(pCurRxCell->pNdisPacket);

		pCurRxCell->pNdisPacket = RTMP_AllocateRxPacketBuffer(rx_ring,
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				pCurRxCell->DmaBuf.AllocSize,
				&pCurRxCell->DmaBuf.AllocVa, &pCurRxCell->DmaBuf.AllocPa);

		pCurRxD->SDP0 = pCurRxCell->DmaBuf.AllocPa;
		pCurRxD->SDL0 = rx_ring->RxBufferSize;
		pCurRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pCurRxD, FALSE, TYPE_RXD);
#endif
	}

	/* update pRxRing->RxSwReadIdx do last cell of rx scatter-gather. */
	rx_ring->RxSwReadIdx = RxCellIdx;
	return TRUE;
}

static INT rx_scatter_gather_pre_slab(
	RTMP_ADAPTER *pAd,
	UCHAR resource_idx,
	RTMP_DMACB *pRxCell,
	RXD_STRUC *pRxD,
	UINT scatterCnt,
	UINT gather_size)
{
	UINT LoopCnt;
	INT32 RxCellIdx;
	UINT32 buf_idx;
	RTMP_DMACB *pCurRxCell = NULL;
	RXD_STRUC *pCurRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif	
	PNDIS_PACKET pRxCellPacket;
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(pci_hif, resource_idx);

	if (rx_ring == NULL || pRxCell == NULL || pRxD == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxRing == NULL || pRxCell == NULL\
					|| pRxD == NULL\n", __func__));
		return FALSE;
	}

	/* keep pRxCell value and replace pRxCell->pNdisPacket with expand skb buffer. */
	pRxCellPacket = pRxCell->pNdisPacket;

	if (pRxCellPacket == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxCellPacket == NULL\n", __func__));
		return FALSE;
	}

	buf_idx = pRxD->SDL0;

	pRxCell->pNdisPacket = expand_pkt_pre_slab(pAd, rx_ring,
			pRxCellPacket, buf_idx, gather_size);

	if (pRxCell->pNdisPacket != NULL) {
		pRxCell->DmaBuf.AllocVa = (PVOID)(pRxCell->pNdisPacket);
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: pRxCell->pNdisPacket == NULL\n", __func__));
		return TRUE;
	}

	rx_ring->cur_free_buf_len = FREE_BUF_64k;

	RxCellIdx = rx_ring->RxSwReadIdx;

	for (LoopCnt = 0; LoopCnt < (scatterCnt - 1); LoopCnt++) {
		INC_RING_INDEX(RxCellIdx, rx_ring->ring_size);
		pCurRxCell = &rx_ring->Cell[RxCellIdx];
		/* return tokens of all rx scatter-gather cells. */
		PCI_UNMAP_SINGLE(pAd, pCurRxCell->DmaBuf.AllocPa,
		pCurRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pCurRxCell->DmaBuf.AllocPa, pCurRxCell->DmaBuf.AllocSize);
		/* Point to Rx indexed rx ring descriptor */
#ifdef RT_BIG_ENDIAN
		pDestRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
		NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
		pCurRxD = (RXD_STRUC *)&rx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
#else
		pCurRxD = (RXD_STRUC *)pCurRxCell->AllocVa;
#endif
		memcpy((pRxCell->pNdisPacket + buf_idx), (VOID *)(pCurRxCell->DmaBuf.AllocVa), pCurRxD->SDL0);
		buf_idx += pCurRxD->SDL0;
		/* update done bit of all rx scatter-gather cells to zero. */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					("%s: pCurRxD->SDL0 = %d, buf_idx = %d, LoopCnt = %d\n", __func__,
					 pCurRxD->SDL0, buf_idx, LoopCnt));
		free_rx_buf_1k(rx_ring);
		pCurRxCell->pNdisPacket = RTMP_AllocateRxPacketBuffer(rx_ring, ((POS_COOKIE)(pAd->OS_Cookie))->pDev,
													pCurRxCell->DmaBuf.AllocSize,
													&pCurRxCell->DmaBuf.AllocVa, &pCurRxCell->DmaBuf.AllocPa);
		pCurRxD->SDP0 = pCurRxCell->DmaBuf.AllocPa;
		pCurRxD->SDL0 = rx_ring->RxBufferSize;
		pCurRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pCurRxD, TYPE_RXD);
		WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pCurRxD, FALSE, TYPE_RXD);
#endif
	}

	/* update pRxRing->RxSwReadIdx do last cell of rx scatter-gather. */
	rx_ring->RxSwReadIdx = RxCellIdx;
	return TRUE;
}

static VOID build_rx_pkt_skb(PNDIS_PACKET *pkt, VOID *rx_data,
								UINT32 build_skb_len, UINT32 gather_size)
{
	if (build_skb_len <= PAGE_SIZE) {
		DEV_BUILD_SKB(*pkt, rx_data, build_skb_len);
	} else {
		DEV_BUILD_SKB(*pkt, rx_data, 0);
	}

	if (*pkt) {
		DEV_SKB_PTR_ADJUST(*pkt, gather_size, rx_data, 0);
	} else {

		if (build_skb_len <= PAGE_SIZE) {
			DEV_FREE_FRAG_BUF(rx_data);
		} else {
			os_free_mem(rx_data);
		}

		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL,
				DBG_LVL_WARN,
				("%s, build_skb return NULL\n",
				__func__));
	}
}

static PNDIS_PACKET pci_get_pkt_dynamic_page_ddone_token(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN *pbReschedule,
	UINT32 *pRxPending,
	UCHAR resource_idx)
{
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(pci_hif, resource_idx);
	PNDIS_PACKET skb_pkt = NULL, pNewPacket = NULL, cur_pkt = NULL;
	VOID *AllocVa, *cur_alloc_va;
	NDIS_PHYSICAL_ADDRESS AllocPa, cur_alloc_pa;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell = NULL;
#ifdef CONFIG_CSO_SUPPORT
	RX_CSO_STRUCT *prCso = NULL;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	UINT scatterCnt = 1;
	UINT32 gather_size = 0;
	UINT32 build_skb_len = 0;
#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif
	PKT_TOKEN_CB *cb = hc_get_ct_cb(pAd->hdev_ctrl);
	static UINT8 ddone_check;
	BOOLEAN drop_pkt = FALSE;
	pRxCell = &rx_ring->Cell[rx_ring->RxSwReadIdx];

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);
	/* Point to Rx indexed rx ring descriptor */
#ifdef RT_BIG_ENDIAN
	pDestRxD = (RXD_STRUC *)pRxCell->AllocVa;
	NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
	pRxD = (RXD_STRUC *)&rx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
	pRxD = (RXD_STRUC *)pRxCell->AllocVa;
#endif


	if (pRxD->DDONE == 0) {
		if (rx_ring->sw_read_idx_inc != 0) {
			bReschedule = FALSE;
		} else {
			HIF_IO_READ32(pAd->hdev_ctrl, rx_ring->hw_didx_addr, &rx_ring->RxDmaIdx);
			if (rx_ring->RxDmaIdx == rx_ring->RxSwReadIdx) {
				bReschedule = FALSE;
			} else {
				ddone_check++;
				bReschedule = TRUE;
			}
		}

		if (ddone_check < MAX_DDONE_CHECK_TIMES)
			goto done;
	} else {
		ddone_check = 0;
	}

	build_skb_len = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + pRxCell->DmaBuf.AllocSize) +
				SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV);

	scatterCnt = rx_scatter_info(rx_ring, pRxD, &gather_size);

	if (scatterCnt < 1) {
		bReschedule = TRUE;
		goto done;
	}

	pNewPacket = RTMP_AllocateRxPacketBuffer(rx_ring,
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				pRxCell->DmaBuf.AllocSize,
				&AllocVa, &AllocPa);


	if (pNewPacket) {
		UINT32 tkn_rx_id = ((pRxD->SDP1 & RX_TOKEN_ID_MASK) >> RX_TOKEN_ID_SHIFT);

		if (tkn_rx_id >= cb->rx_que.pkt_tkid_cnt) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s(): Invalid tkn_rx_id: %d.\n", __func__, tkn_rx_id));
			goto done;
		}

		/* RXD+RXP buffer lookup according token */
		token_rx_dmad_lookup(&cb->rx_que, tkn_rx_id, &cur_pkt,
								&cur_alloc_va, &cur_alloc_pa);

		PCI_UNMAP_SINGLE(pAd, cur_alloc_pa,
						 pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);

		RTMP_DCACHE_FLUSH(cur_alloc_pa, pRxCell->DmaBuf.AllocSize);

		if (scatterCnt > 1) {
			if (rx_scatter_gather_dynamic_page(pAd, resource_idx, pRxCell,
				pRxD, scatterCnt,
				gather_size, &build_skb_len) == FALSE)	{
				RELEASE_NDIS_PACKET_IRQ(pAd, pNewPacket, NDIS_STATUS_SUCCESS);
				bReschedule = TRUE;
				goto done;
			}
		}

		if (cur_pkt) {
			build_rx_pkt_skb(&skb_pkt, (VOID *)cur_pkt,
									build_skb_len, gather_size);
		} else {
			skb_pkt = NULL;
		}

#ifdef CONFIG_CSO_SUPPORT
		if (skb_pkt && (pChipCap->asic_caps & fASIC_CAP_CSO)) {
			prCso = (RX_CSO_STRUCT *)((UCHAR *)pRxD + sizeof(RXD_STRUC));
			NdisCopyMemory(&(pAd->rCso), prCso, sizeof(RX_CSO_STRUCT));
		}
#endif

#if defined(CONFIG_FAST_NAT_SUPPORT) && defined(WHNAT_SUPPORT)
		if (skb_pkt) {
			UINT32 *dw1 = (UINT32 *)((UCHAR *)pRxD + 4);
			UINT32 *dw3 = (UINT32 *)((UCHAR *)pRxD + sizeof(RXD_STRUC));
			if (*dw3 & RXDMAD_PPE_VLD) {
				struct rxdmad_info info;
				info.pkt = skb_pkt;
				info.ppe_entry = ((*dw3 & RXDMAD_PPE_ENTRY_MASK) >> RXDMAD_PPE_ENTRY_SHIFT);
				info.csrn = ((*dw3 & RXDMAD_CSRN_MASK) >> RXDMAD_CSRN_SHIFT);
				WLAN_HOOK_CALL(WLAN_HOOK_RX, pAd, &info);
				RTMP_SET_PACKET_TYPE(skb_pkt, RX_PPE_VALID);
			}
			drop_pkt = (*dw1 & (RXDMAD_RXD_DROP
								| RXDMAD_TO_HOST_A)) ? 1 : 0;
			/* dump_rxd_wed(pAd, pRxD); */
		}
#endif

		pRxCell->pNdisPacket = (PNDIS_PACKET)pNewPacket;
		pRxCell->DmaBuf.AllocVa = AllocVa;
		pRxCell->DmaBuf.AllocPa = AllocPa;
		pRxCell->DmaBuf.AllocSize = rx_ring->RxBufferSize;

		/* RXD+RXP buffer "update" for RX token */
		token_rx_dmad_update(&cb->rx_que, tkn_rx_id, pNewPacket,
								rx_ring->RxBufferSize, AllocVa, AllocPa);

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);
		pRxD->SDP0 = pRxCell->DmaBuf.AllocPa;
		pRxD->SDL0 = rx_ring->RxBufferSize;

		/*avoid dummy packet received before System Ready*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY) ||
				ddone_check >= MAX_DDONE_CHECK_TIMES || drop_pkt) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): drop rx pkt.\n", __func__));
			ddone_check = 0;

			if (skb_pkt) {
				RELEASE_NDIS_PACKET_IRQ(pAd, skb_pkt, NDIS_STATUS_SUCCESS);
				skb_pkt = NULL;
				bReschedule = TRUE;
			}
		}
	} else {
		bReschedule = TRUE;
		goto done;
	}

	pRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN		
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif


	wmb();
	INC_RING_INDEX(rx_ring->RxSwReadIdx, rx_ring->ring_size);
	rx_ring->sw_read_idx_inc++;
	rx_ring->RxCpuIdx = (rx_ring->RxSwReadIdx == 0)
						? (rx_ring->ring_size - 1) : (rx_ring->RxSwReadIdx - 1);

done:
	*pbReschedule = bReschedule;
	return skb_pkt;
}

static PNDIS_PACKET pci_get_pkt_dynamic_page_ddone(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN *pbReschedule,
	UINT32 *pRxPending,
	UCHAR resource_idx)
{
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(pci_hif, resource_idx);
	PNDIS_PACKET pRxPacket = NULL, pNewPacket = NULL;
	VOID *AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell = NULL;
#ifdef CONFIG_CSO_SUPPORT
	RX_CSO_STRUCT *prCso = NULL;
#endif
#if (defined (CONFIG_CSO_SUPPORT) || defined (RX_RPS_SUPPORT))
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	UINT scatterCnt = 1;
	UINT32 gather_size = 0;
	UINT32 build_skb_len = 0;
#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif
	static UINT8 ddone_check;

	pRxCell = &rx_ring->Cell[rx_ring->RxSwReadIdx];

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);
	/* Point to Rx indexed rx ring descriptor */
#ifdef RT_BIG_ENDIAN
	pDestRxD = (RXD_STRUC *)pRxCell->AllocVa;
	NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
	pRxD = (RXD_STRUC *)&rx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
	pRxD = (RXD_STRUC *)pRxCell->AllocVa;
#endif
	if (pRxD->DDONE == 0) {
		if (rx_ring->sw_read_idx_inc != 0) {
			bReschedule = FALSE;
		} else {
			HIF_IO_READ32(pAd->hdev_ctrl, rx_ring->hw_didx_addr, &rx_ring->RxDmaIdx);
			if (rx_ring->RxDmaIdx == rx_ring->RxSwReadIdx) {
				bReschedule = FALSE;
			} else {
				ddone_check++;
				bReschedule = TRUE;
			}
		}

		if (ddone_check < MAX_DDONE_CHECK_TIMES)
			goto done;
	} else {
		ddone_check = 0;
	}

	build_skb_len = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + pRxCell->DmaBuf.AllocSize) +
				SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV);

	scatterCnt = rx_scatter_info(rx_ring, pRxD, &gather_size);

	if (scatterCnt < 1) {
		bReschedule = TRUE;
		goto done;
	}

	pNewPacket = RTMP_AllocateRxPacketBuffer(rx_ring,
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				pRxCell->DmaBuf.AllocSize,
				&AllocVa, &AllocPa);


	if (pNewPacket) {
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
						 pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);

		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

		if (scatterCnt > 1) {
			if (rx_scatter_gather_dynamic_page(pAd, resource_idx, pRxCell,
				pRxD, scatterCnt,
				gather_size, &build_skb_len) == FALSE)	{
				RELEASE_NDIS_PACKET_IRQ(pAd, pNewPacket, NDIS_STATUS_SUCCESS);
				bReschedule = TRUE;
				goto done;
			}
		}

		if (pRxCell->pNdisPacket) {
			build_rx_pkt_skb(&pRxPacket, (VOID *)pRxCell->pNdisPacket,
									build_skb_len, gather_size);
#ifdef RX_RPS_SUPPORT
			if (pAd->ixia_mode_ctl.kernel_rps_en) {
			if (pChipCap->rx_qm_en) {
				UINT16 wcid = 0;
					struct rxd_grp_0 *rxd_grp0 =
						(struct rxd_grp_0 *) GET_OS_PKT_DATAPTR(pRxPacket);

				if (rxd_grp0->rxd_2 & RXD_NDATA)
					wcid = smp_processor_id()+1;
				else {
					wcid = (rxd_grp0->rxd_1 & RXD_WLAN_IDX_MASK);
					if ((wcid == 0) || (wcid >= MAX_LEN_OF_MAC_TABLE))
						wcid = smp_processor_id()+1;
				}
					RTPKT_TO_OSPKT(pRxPacket)->hash = wcid;
			} else
					RTPKT_TO_OSPKT(pRxPacket)->hash = smp_processor_id()+1;
			}

#endif
		} else {
			pRxPacket = NULL;
		}

#ifdef CONFIG_CSO_SUPPORT
		if (pRxPacket && (pChipCap->asic_caps & fASIC_CAP_CSO)) {
			prCso = (RX_CSO_STRUCT *)((UCHAR *)pRxD + sizeof(RXD_STRUC));
			NdisCopyMemory(&(pAd->rCso), prCso, sizeof(RX_CSO_STRUCT));
		}
#endif
		pRxCell->pNdisPacket = (PNDIS_PACKET)pNewPacket;
		pRxCell->DmaBuf.AllocVa = AllocVa;
		pRxCell->DmaBuf.AllocPa = AllocPa;
		pRxCell->DmaBuf.AllocSize = rx_ring->RxBufferSize;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);
		pRxD->SDP0 = pRxCell->DmaBuf.AllocPa;
		pRxD->SDL0 = rx_ring->RxBufferSize;

		/*avoid dummy packet received before System Ready*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY) ||
						ddone_check >= MAX_DDONE_CHECK_TIMES) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): system is not ready, rx pkt drop it.\n", __func__));
			ddone_check = 0;

			if (pRxPacket) {
				RELEASE_NDIS_PACKET_IRQ(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				pRxPacket = NULL;
				bReschedule = TRUE;
			}
		}
	} else {
		bReschedule = TRUE;
		goto done;
	}

	pRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif

	wmb();
	INC_RING_INDEX(rx_ring->RxSwReadIdx, rx_ring->ring_size);
	rx_ring->sw_read_idx_inc++;
	rx_ring->RxCpuIdx = (rx_ring->RxSwReadIdx == 0)
						? (rx_ring->ring_size - 1) : (rx_ring->RxSwReadIdx - 1);

#ifdef CONFIG_WIFI_PREFETCH_RXDATA
	/* prefetch to enhance throughput */
	prefetch(rx_ring->Cell[rx_ring->RxSwReadIdx].pNdisPacket);
#endif /* CONFIG_WIFI_PREFETCH_RXDATA */

done:
	*pbReschedule = bReschedule;
	return pRxPacket;
}

static PNDIS_PACKET pci_get_pkt_dynamic_page_io(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN *pbReschedule,
	UINT32 *pRxPending,
	UCHAR resource_idx)
{
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(hif, resource_idx);
	PNDIS_PACKET pRxPacket = NULL, pNewPacket = NULL;
	VOID *AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell = NULL;
#ifdef CONFIG_CSO_SUPPORT
	RX_CSO_STRUCT *prCso = NULL;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	UINT scatterCnt = 1;
	UINT32 gather_size = 0;
	UINT32 build_skb_len = 0;
	UINT16 rx_ring_size = rx_ring->ring_size;

#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

	if (*pRxPending == 0) {
		/* Get how may packets had been received */
		HIF_IO_READ32(pAd->hdev_ctrl, rx_ring->hw_didx_addr, &rx_ring->RxDmaIdx);

#ifdef CONFIG_TP_DBG
		tp_dbg->IoReadRx++;
#endif

		if (rx_ring->RxDmaIdx == rx_ring->RxSwReadIdx) {
			bReschedule = FALSE;
			goto done;
		}

		/* get rx pending count */
		if (rx_ring->RxDmaIdx > rx_ring->RxSwReadIdx)
			*pRxPending = rx_ring->RxDmaIdx - rx_ring->RxSwReadIdx;
		else
			*pRxPending = rx_ring->RxDmaIdx + rx_ring_size - rx_ring->RxSwReadIdx;
	}

	pRxCell = &rx_ring->Cell[rx_ring->RxSwReadIdx];
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);
	/* Point to Rx indexed rx ring descriptor */
#ifdef RT_BIG_ENDIAN
	pDestRxD = (RXD_STRUC *)pRxCell->AllocVa;
	NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
	pRxD = (RXD_STRUC *)&rx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
	pRxD = (RXD_STRUC *)pRxCell->AllocVa;
#endif


	if (pRxD->DDONE == 0) {
		*pRxPending = 0;
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, ("DDONE=0!\n"));
		/* DMAIndx had done but DDONE bit not ready */
		bReschedule = TRUE;
		goto done;
	}

	build_skb_len = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + pRxCell->DmaBuf.AllocSize) +
				SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV);

	scatterCnt = rx_scatter_info_io(rx_ring, pRxD, &gather_size);

	if (scatterCnt < 1) {
		bReschedule = TRUE;
		goto done;
	}

	pNewPacket = RTMP_AllocateRxPacketBuffer(rx_ring,
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				pRxCell->DmaBuf.AllocSize,
				&AllocVa, &AllocPa);

	if (pNewPacket) {
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
						 pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

		if (scatterCnt > 1) {
			if (rx_scatter_gather_dynamic_page(pAd, resource_idx, pRxCell,
				pRxD, scatterCnt,
				gather_size, &build_skb_len) == FALSE)	{
				RELEASE_NDIS_PACKET_IRQ(pAd, pNewPacket, NDIS_STATUS_SUCCESS);
				bReschedule = TRUE;
				goto done;
			}
		}

		if (pRxCell->pNdisPacket) {
			build_rx_pkt_skb(&pRxPacket, (VOID *)pRxCell->pNdisPacket,
									build_skb_len, gather_size);
		} else {
			pRxPacket = NULL;
		}

#ifdef CONFIG_CSO_SUPPORT
		if (pRxPacket && (pChipCap->asic_caps & fASIC_CAP_CSO)) {
			prCso = (RX_CSO_STRUCT *)((UCHAR *)pRxD + sizeof(RXD_STRUC));
			NdisCopyMemory(&(pAd->rCso), prCso, sizeof(RX_CSO_STRUCT));
		}
#endif
		pRxCell->pNdisPacket = (PNDIS_PACKET)pNewPacket;
		pRxCell->DmaBuf.AllocVa = AllocVa;
		pRxCell->DmaBuf.AllocPa = AllocPa;
		pRxCell->DmaBuf.AllocSize = rx_ring->RxBufferSize;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);
		pRxD->SDP0 = pRxCell->DmaBuf.AllocPa;
		pRxD->SDL0 = rx_ring->RxBufferSize;

		/*avoid dummy packet received before System Ready*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY)) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): system is not ready, rx pkt drop it.\n", __func__));

			if (pRxPacket) {
				RELEASE_NDIS_PACKET_IRQ(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				pRxPacket = NULL;
				bReschedule = TRUE;
			}
		}
	} else {
		bReschedule = TRUE;
		goto done;
	}

	*pRxPending = *pRxPending - 1;
	pRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif

	wmb();
	INC_RING_INDEX(rx_ring->RxSwReadIdx, rx_ring_size);
	rx_ring->sw_read_idx_inc++;
	rx_ring->RxCpuIdx = (rx_ring->RxSwReadIdx == 0)
						? (rx_ring_size - 1) : (rx_ring->RxSwReadIdx - 1);

#ifdef CONFIG_WIFI_PREFETCH_RXDATA
	/* prefetch to enhance throughput */
	if (*pRxPending > 0)
		prefetch(rx_ring->Cell[rx_ring->RxSwReadIdx].pNdisPacket);
#endif /* CONFIG_WIFI_PREFETCH_RXDATA */

done:
	*pbReschedule = bReschedule;
	return pRxPacket;
}

/*
*
*/
static PNDIS_PACKET pci_get_pkt_dynamic_slab_ddone(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN *pbReschedule,
	UINT32 *pRxPending,
	UCHAR resource_idx)
{
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif

	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(pci_hif, resource_idx);
	PNDIS_PACKET pRxPacket = NULL, pNewPacket = NULL;
	VOID *AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell = NULL;
#ifdef CONFIG_CSO_SUPPORT
	RX_CSO_STRUCT *prCso = NULL;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	UINT scatterCnt = 1;
	UINT32 gather_size = 0;
#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

	pRxCell = &rx_ring->Cell[rx_ring->RxSwReadIdx];

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);
	/* Point to Rx indexed rx ring descriptor */
#ifdef RT_BIG_ENDIAN
	pDestRxD = (RXD_STRUC *)pRxCell->AllocVa;
	NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
	pRxD = (RXD_STRUC *)&rx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
	pRxD = (RXD_STRUC *)pRxCell->AllocVa;
#endif
	if (pRxD->DDONE == 0) {
		bReschedule = FALSE;
		goto done;
	}

	*pRxPending = 1;

	scatterCnt = rx_scatter_info(rx_ring, pRxD, &gather_size);

	if (scatterCnt < 1) {
		bReschedule = TRUE;
		goto done;
	}

	pNewPacket = RTMP_AllocateRxPacketBuffer(rx_ring,
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				pRxCell->DmaBuf.AllocSize,
				&AllocVa, &AllocPa);

	if (pNewPacket) {
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
						 pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

		if (scatterCnt > 1) {
			if (rx_scatter_gather_dynamic_slab(pAd, resource_idx, pRxCell,
				pRxD, scatterCnt,
				gather_size) == FALSE)	{
				RELEASE_NDIS_PACKET_IRQ(pAd, pNewPacket, NDIS_STATUS_SUCCESS);
				bReschedule = TRUE;
				goto done;
			}
		}

		pRxPacket = pRxCell->pNdisPacket;

#ifdef CONFIG_CSO_SUPPORT
		if (pRxPacket && (pChipCap->asic_caps & fASIC_CAP_CSO)) {
			prCso = (RX_CSO_STRUCT *)((UCHAR *)pRxD + sizeof(RXD_STRUC));
			NdisCopyMemory(&(pAd->rCso), prCso, sizeof(RX_CSO_STRUCT));
		}
#endif
		pRxCell->pNdisPacket = (PNDIS_PACKET)pNewPacket;
		pRxCell->DmaBuf.AllocVa = AllocVa;
		pRxCell->DmaBuf.AllocPa = AllocPa;
		pRxCell->DmaBuf.AllocSize = rx_ring->RxBufferSize;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);
		pRxD->SDP0 = pRxCell->DmaBuf.AllocPa;
		pRxD->SDL0 = rx_ring->RxBufferSize;

		/*avoid dummy packet received before System Ready*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY)) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): system is not ready, rx pkt drop it.\n", __func__));

			if (pRxPacket) {
				RELEASE_NDIS_PACKET_IRQ(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				pRxPacket = NULL;
				bReschedule = TRUE;
			}
		}
	} else {
		bReschedule = TRUE;
		goto done;
	}

	pRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif

	wmb();
	INC_RING_INDEX(rx_ring->RxSwReadIdx, rx_ring->ring_size);
	rx_ring->sw_read_idx_inc++;
	rx_ring->RxCpuIdx = (rx_ring->RxSwReadIdx == 0)
						? (rx_ring->ring_size - 1) : (rx_ring->RxSwReadIdx - 1);

#ifdef CONFIG_WIFI_PREFETCH_RXDATA
	/* prefetch to enhance throughput */
	if (*pRxPending > 0)
		prefetch(rx_ring->Cell[rx_ring->RxSwReadIdx].pNdisPacket);
#endif /* CONFIG_WIFI_PREFETCH_RXDATA */

done:
	*pbReschedule = bReschedule;
	return pRxPacket;
}

/*
*
*/
static PNDIS_PACKET pci_get_pkt_dynamic_slab_io(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN *pbReschedule,
	UINT32 *pRxPending,
	UCHAR resource_idx)
{
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif

	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(hif, resource_idx);
	PNDIS_PACKET pRxPacket = NULL, pNewPacket = NULL;
	VOID *AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell = NULL;
#ifdef CONFIG_CSO_SUPPORT
	RX_CSO_STRUCT *prCso = NULL;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	UINT scatterCnt = 1;
	UINT32 gather_size = 0;
	UINT16 rx_ring_size = rx_ring->ring_size;

#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

	if (*pRxPending == 0) {
		/* Get how may packets had been received */
		HIF_IO_READ32(pAd->hdev_ctrl, rx_ring->hw_didx_addr, &rx_ring->RxDmaIdx);

#ifdef CONFIG_TP_DBG
		tp_dbg->IoReadRx++;
#endif

		if (rx_ring->RxDmaIdx == rx_ring->RxSwReadIdx) {
			bReschedule = FALSE;
			goto done;
		}

		/* get rx pending count */
		if (rx_ring->RxDmaIdx > rx_ring->RxSwReadIdx)
			*pRxPending = rx_ring->RxDmaIdx - rx_ring->RxSwReadIdx;
		else
			*pRxPending = rx_ring->RxDmaIdx + rx_ring_size - rx_ring->RxSwReadIdx;
	}

	pRxCell = &rx_ring->Cell[rx_ring->RxSwReadIdx];
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);
	/* Point to Rx indexed rx ring descriptor */
#ifdef RT_BIG_ENDIAN
	pDestRxD = (RXD_STRUC *)pRxCell->AllocVa;
	NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
	pRxD = (RXD_STRUC *)&rx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
	pRxD = (RXD_STRUC *)pRxCell->AllocVa;
#endif
	if (pRxD->DDONE == 0) {
		*pRxPending = 0;
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, ("DDONE=0!\n"));
		/* DMAIndx had done but DDONE bit not ready */
		bReschedule = TRUE;
		goto done;
	}

	scatterCnt = rx_scatter_info_io(rx_ring, pRxD, &gather_size);

	if (scatterCnt < 1) {
		bReschedule = TRUE;
		goto done;
	}

	pNewPacket = RTMP_AllocateRxPacketBuffer(rx_ring,
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				pRxCell->DmaBuf.AllocSize,
				&AllocVa, &AllocPa);

	if (pNewPacket) {
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
						 pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

		if (scatterCnt > 1) {

			if (rx_scatter_gather_dynamic_slab(pAd, resource_idx, pRxCell,
				pRxD, scatterCnt,
				gather_size) == FALSE)	{
				RELEASE_NDIS_PACKET_IRQ(pAd, pNewPacket, NDIS_STATUS_SUCCESS);
				bReschedule = TRUE;
				goto done;
			}
		}

		pRxPacket = pRxCell->pNdisPacket;

#ifdef CONFIG_CSO_SUPPORT
		if (pRxPacket && (pChipCap->asic_caps & fASIC_CAP_CSO)) {
			prCso = (RX_CSO_STRUCT *)((UCHAR *)pRxD + sizeof(RXD_STRUC));
			NdisCopyMemory(&(pAd->rCso), prCso, sizeof(RX_CSO_STRUCT));
		}
#endif
		pRxCell->pNdisPacket = (PNDIS_PACKET)pNewPacket;
		pRxCell->DmaBuf.AllocVa = AllocVa;
		pRxCell->DmaBuf.AllocPa = AllocPa;
		pRxCell->DmaBuf.AllocSize = rx_ring->RxBufferSize;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);
		pRxD->SDP0 = pRxCell->DmaBuf.AllocPa;
		pRxD->SDL0 = rx_ring->RxBufferSize;

		/*avoid dummy packet received before System Ready*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY)) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): system is not ready, rx pkt drop it.\n", __func__));

			if (pRxPacket) {
				RELEASE_NDIS_PACKET_IRQ(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				pRxPacket = NULL;
				bReschedule = TRUE;
			}
		}
	} else {
		bReschedule = TRUE;
		goto done;
	}

	*pRxPending = *pRxPending - 1;
	pRxD->DDONE = 0;
	wmb();
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif

	INC_RING_INDEX(rx_ring->RxSwReadIdx, rx_ring_size);
	rx_ring->sw_read_idx_inc++;
	rx_ring->RxCpuIdx = (rx_ring->RxSwReadIdx == 0)
						? (rx_ring_size - 1) : (rx_ring->RxSwReadIdx - 1);

#ifdef CONFIG_WIFI_PREFETCH_RXDATA
	/* prefetch to enhance throughput */
	if (*pRxPending > 0)
		prefetch(rx_ring->Cell[rx_ring->RxSwReadIdx].pNdisPacket);
#endif /* CONFIG_WIFI_PREFETCH_RXDATA */

done:
	*pbReschedule = bReschedule;
	return pRxPacket;
}

/*
*
*/
static PNDIS_PACKET pci_get_pkt_pre_slab_ddone(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN *pbReschedule,
	UINT32 *pRxPending,
	UCHAR resource_idx)
{
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(pci_hif, resource_idx);
	PNDIS_PACKET pRxPacket = NULL, pNewPacket = NULL;
	VOID *AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell = NULL;
#ifdef CONFIG_CSO_SUPPORT
	RX_CSO_STRUCT *prCso = NULL;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	UINT scatterCnt = 1;
	UINT32 gather_size = 0;
#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif
	static UINT8 ddone_check;

	pRxCell = &rx_ring->Cell[rx_ring->RxSwReadIdx];

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);
	/* Point to Rx indexed rx ring descriptor */
#ifdef RT_BIG_ENDIAN
	pDestRxD = (RXD_STRUC *)pRxCell->AllocVa;
	NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
	pRxD = (RXD_STRUC *)&rx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
	pRxD = (RXD_STRUC *)pRxCell->AllocVa;
#endif
	if (pRxD->DDONE == 0) {
		if (rx_ring->sw_read_idx_inc != 0) {
			bReschedule = FALSE;
		} else {
			HIF_IO_READ32(pAd->hdev_ctrl, rx_ring->hw_didx_addr, &rx_ring->RxDmaIdx);
			if (rx_ring->RxDmaIdx == rx_ring->RxSwReadIdx) {
				bReschedule = FALSE;
			} else {
				ddone_check++;
				bReschedule = TRUE;
			}
		}

		if (ddone_check < MAX_DDONE_CHECK_TIMES)
			goto done;
	} else {
		ddone_check = 0;
	}

	scatterCnt = rx_scatter_info(rx_ring, pRxD, &gather_size);

	if (scatterCnt < 1) {
		bReschedule = TRUE;
		goto done;
	}

	pNewPacket = RTMP_AllocateRxPacketBuffer(rx_ring,
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				pRxCell->DmaBuf.AllocSize,
				&AllocVa, &AllocPa);

	rx_ring->cur_free_buf_len = FREE_BUF_1k;

	if (pNewPacket) {
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
						 pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

		if (scatterCnt > 1) {

			if (rx_scatter_gather_pre_slab(pAd, resource_idx, pRxCell,
				pRxD, scatterCnt,
				gather_size) == FALSE)	{
				free_rx_buf_1k(rx_ring);
				bReschedule = TRUE;
				goto done;
			}
		}

		pRxPacket = pRxCell->pNdisPacket;

		pRxCell->pNdisPacket = (PNDIS_PACKET)pNewPacket;
		pRxCell->DmaBuf.AllocVa = AllocVa;
		pRxCell->DmaBuf.AllocPa = AllocPa;
		pRxCell->DmaBuf.AllocSize = rx_ring->RxBufferSize;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);
		pRxD->SDP0 = pRxCell->DmaBuf.AllocPa;
		pRxD->SDL0 = rx_ring->RxBufferSize;

		if (ddone_check >= MAX_DDONE_CHECK_TIMES) {
			ddone_check = 0;

			if (pRxPacket) {
				pRxPacket = NULL;
				free_rx_buf_1k(rx_ring);
				bReschedule = TRUE;
			}
		}

	} else {
		bReschedule = TRUE;
		goto done;
	}

	pRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif
	wmb();
	INC_RING_INDEX(rx_ring->RxSwReadIdx, rx_ring->ring_size);
	rx_ring->sw_read_idx_inc++;
	rx_ring->RxCpuIdx = (rx_ring->RxSwReadIdx == 0)
						? (rx_ring->ring_size - 1) : (rx_ring->RxSwReadIdx - 1);

done:
	*pbReschedule = bReschedule;
	return pRxPacket;
}

/*
*
*/
static PNDIS_PACKET pci_get_pkt_pre_slab_io(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN *pbReschedule,
	UINT32 *pRxPending,
	UCHAR resource_idx)
{
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif

	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(hif, resource_idx);
	PNDIS_PACKET pRxPacket = NULL, pNewPacket = NULL;
	VOID *AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell = NULL;
#ifdef CONFIG_CSO_SUPPORT
	RX_CSO_STRUCT *prCso = NULL;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	UINT scatterCnt = 1;
	UINT32 gather_size = 0;
	UINT16 rx_ring_size = rx_ring->ring_size;

#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

	if (*pRxPending == 0) {
		/* Get how may packets had been received */
		HIF_IO_READ32(pAd->hdev_ctrl, rx_ring->hw_didx_addr, &rx_ring->RxDmaIdx);

#ifdef CONFIG_TP_DBG
		tp_dbg->IoReadRx1++;
#endif

		if (rx_ring->RxDmaIdx == rx_ring->RxSwReadIdx) {
			bReschedule = FALSE;
			goto done;
		}

		/* get rx pending count */
		if (rx_ring->RxDmaIdx > rx_ring->RxSwReadIdx)
			*pRxPending = rx_ring->RxDmaIdx - rx_ring->RxSwReadIdx;
		else
			*pRxPending = rx_ring->RxDmaIdx + rx_ring_size - rx_ring->RxSwReadIdx;
	}

	pRxCell = &rx_ring->Cell[rx_ring->RxSwReadIdx];
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);
	/* Point to Rx indexed rx ring descriptor */
#ifdef RT_BIG_ENDIAN
		pDestRxD = (RXD_STRUC *)pRxCell->AllocVa;
		NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
		pRxD = (RXD_STRUC *)&rx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else

	pRxD = (RXD_STRUC *)pRxCell->AllocVa;
#endif
	if (pRxD->DDONE == 0) {
		*pRxPending = 0;
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, ("DDONE=0!\n"));
		/* DMAIndx had done but DDONE bit not ready */
		bReschedule = TRUE;
		goto done;
	}

	scatterCnt = rx_scatter_info_io(rx_ring, pRxD, &gather_size);

	if (scatterCnt < 1) {
		bReschedule = TRUE;
		goto done;
	}

	pNewPacket = RTMP_AllocateRxPacketBuffer(rx_ring,
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				pRxCell->DmaBuf.AllocSize,
				&AllocVa, &AllocPa);

	rx_ring->cur_free_buf_len = FREE_BUF_1k;

	if (pNewPacket) {
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
						 pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

		if (scatterCnt > 1) {

			if (rx_scatter_gather_pre_slab(pAd, resource_idx, pRxCell,
				pRxD, scatterCnt,
				gather_size) == FALSE)	{
				free_rx_buf_1k(rx_ring);
				bReschedule = TRUE;
				goto done;
			}
		}

		pRxPacket = pRxCell->pNdisPacket;

		pRxCell->pNdisPacket = (PNDIS_PACKET)pNewPacket;
		pRxCell->DmaBuf.AllocVa = AllocVa;
		pRxCell->DmaBuf.AllocPa = AllocPa;
		pRxCell->DmaBuf.AllocSize = rx_ring->RxBufferSize;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);
		pRxD->SDP0 = pRxCell->DmaBuf.AllocPa;
		pRxD->SDL0 = rx_ring->RxBufferSize;

	} else {
		bReschedule = TRUE;
		goto done;
	}

	*pRxPending = *pRxPending - 1;
	pRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif
	wmb();
	INC_RING_INDEX(rx_ring->RxSwReadIdx, rx_ring_size);
	rx_ring->sw_read_idx_inc++;
	rx_ring->RxCpuIdx = (rx_ring->RxSwReadIdx == 0)
						? (rx_ring_size - 1) : (rx_ring->RxSwReadIdx - 1);

done:
	*pbReschedule = bReschedule;
	return pRxPacket;
}

/*
*
*/
static PNDIS_PACKET pci_get_pkt_dynamic_page_ddone_debug(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN *pbReschedule,
	UINT32 *pRxPending,
	UCHAR resource_idx)
{
	RXD_DEBUG_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(pci_hif, resource_idx);
	PNDIS_PACKET pRxPacket = NULL, pNewPacket = NULL;
	VOID *AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell = NULL;
#ifdef CONFIG_CSO_SUPPORT
	RX_CSO_STRUCT *prCso = NULL;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	UINT scatterCnt = 1;
	UINT32 gather_size = 0;
	UINT32 build_skb_len = 0;
	static UINT64 ring_round;
#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

	pRxCell = &rx_ring->Cell[rx_ring->RxSwReadIdx];

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);
	/* Point to Rx indexed rx ring descriptor */
#ifdef RT_BIG_ENDIAN
	pDestRxD = (RXD_STRUC *)pRxCell->AllocVa;
	NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
	pRxD = (RXD_STRUC *)&rx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
	pRxD = (RXD_DEBUG_STRUC *)pRxCell->AllocVa;
#endif
	if (pRxD->DDONE == 0) {
		bReschedule = FALSE;
		goto done;
	}

	build_skb_len = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + pRxCell->DmaBuf.AllocSize) +
				SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV);

	scatterCnt = rx_scatter_info(rx_ring, (struct _RXD_STRUC *)pRxD, &gather_size);

	if (scatterCnt < 1) {
		bReschedule = TRUE;
		goto done;
	}

	pNewPacket = RTMP_AllocateRxPacketBuffer(rx_ring,
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				pRxCell->DmaBuf.AllocSize,
				&AllocVa, &AllocPa);

	if (pNewPacket) {
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
						 pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

		if (scatterCnt > 1) {
			if (rx_scatter_gather_dynamic_page(pAd, resource_idx, pRxCell,
				(struct _RXD_STRUC *)pRxD, scatterCnt,
				gather_size, &build_skb_len) == FALSE)	{
				RELEASE_NDIS_PACKET_IRQ(pAd, pNewPacket, NDIS_STATUS_SUCCESS);
				bReschedule = TRUE;
				goto done;
			}
		}

		if (pRxCell->pNdisPacket) {
			build_rx_pkt_skb(&pRxPacket, (VOID *)pRxCell->pNdisPacket,
									build_skb_len, gather_size);
		} else {
			pRxPacket = NULL;
		}

#ifdef CONFIG_CSO_SUPPORT
		if (pRxPacket && (pChipCap->asic_caps & fASIC_CAP_CSO)) {
			prCso = (RX_CSO_STRUCT *)((UCHAR *)pRxD + sizeof(RXD_DEBUG_STRUC));
			NdisCopyMemory(&(pAd->rCso), prCso, sizeof(RX_CSO_STRUCT));
		}
#endif

		if (pRxPacket) {
			if (dump_rx_info(pAd, pRxCell, pRxD, resource_idx, pRxPacket) == NDIS_STATUS_FAILURE) {
				RELEASE_NDIS_PACKET_IRQ(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				pRxPacket = NULL;

			}
		}

		pRxCell->pNdisPacket = (PNDIS_PACKET)pNewPacket;
		pRxCell->DmaBuf.AllocVa = AllocVa;
		pRxCell->DmaBuf.AllocPa = AllocPa;
		pRxCell->DmaBuf.AllocSize = rx_ring->RxBufferSize;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);
		pRxD->SDP0 = pRxCell->DmaBuf.AllocPa;
		pRxD->SDL0 = rx_ring->RxBufferSize;

		/*avoid dummy packet received before System Ready*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY)) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							("%s(): system is not ready, rx pkt drop it.\n", __func__));

			if (pRxPacket) {
				RELEASE_NDIS_PACKET_IRQ(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				pRxPacket = NULL;
				bReschedule = TRUE;
			}
		}
	} else {
		bReschedule = TRUE;
		goto done;
	}

	pRxD->DIDX = 0;
	pRxD->QID = 0;
	pRxD->LS1 = 0;
	pRxD->BURST = 0;
	pRxD->CIDX = rx_ring->RxSwReadIdx;
	INC_RING_INDEX(rx_ring->RxSwReadIdx, rx_ring->ring_size);
	rx_ring->sw_read_idx_inc++;
	rx_ring->RxCpuIdx = (rx_ring->RxSwReadIdx == 0)
						? (rx_ring->ring_size - 1) : (rx_ring->RxSwReadIdx - 1);

	if (rx_ring->RxSwReadIdx == 0) {
		ring_round++;
	}

	pRxD->SW_INFO = ring_round;
	pRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif
	wmb();

#ifdef CONFIG_WIFI_PREFETCH_RXDATA
	/* prefetch to enhance throughput */
	if (*pRxPending > 0)
		prefetch(rx_ring->Cell[rx_ring->RxSwReadIdx].pNdisPacket);
#endif /* CONFIG_WIFI_PREFETCH_RXDATA */

done:
	*pbReschedule = bReschedule;
	return pRxPacket;
}

/*
*
*/
static PNDIS_PACKET pci_get_pkt_dynamic_page_io_debug(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN *pbReschedule,
	UINT32 *pRxPending,
	UCHAR resource_idx)
{
	RXD_DEBUG_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif

	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(hif, resource_idx);
	PNDIS_PACKET pRxPacket = NULL, pNewPacket = NULL;
	VOID *AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell = NULL;
#ifdef CONFIG_CSO_SUPPORT
	RX_CSO_STRUCT *prCso = NULL;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	UINT scatterCnt = 1;
	UINT32 gather_size = 0;
	UINT32 build_skb_len = 0;
	UINT16 rx_ring_size = rx_ring->ring_size;
	static UINT64 ring_round;

#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

	if (*pRxPending == 0) {
		/* Get how may packets had been received */
		HIF_IO_READ32(pAd->hdev_ctrl, rx_ring->hw_didx_addr, &rx_ring->RxDmaIdx);

#ifdef CONFIG_TP_DBG
		tp_dbg->IoReadRx++;
#endif

		if (rx_ring->RxDmaIdx == rx_ring->RxSwReadIdx) {
			bReschedule = FALSE;
			goto done;
		}

		/* get rx pending count */
		if (rx_ring->RxDmaIdx > rx_ring->RxSwReadIdx)
			*pRxPending = rx_ring->RxDmaIdx - rx_ring->RxSwReadIdx;
		else
			*pRxPending = rx_ring->RxDmaIdx + rx_ring_size - rx_ring->RxSwReadIdx;
	}

	pRxCell = &rx_ring->Cell[rx_ring->RxSwReadIdx];
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);
	/* Point to Rx indexed rx ring descriptor */
#ifdef RT_BIG_ENDIAN
	pDestRxD = (RXD_STRUC *)pRxCell->AllocVa;
	NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
	pRxD = (RXD_STRUC *)&rx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else

	pRxD = (RXD_DEBUG_STRUC *)pRxCell->AllocVa;
#endif
	if (pRxD->DDONE == 0) {
		*pRxPending = 0;
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, ("DDONE=0!\n"));
		/* DMAIndx had done but DDONE bit not ready */
		bReschedule = TRUE;
		goto done;
	}

	build_skb_len = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + pRxCell->DmaBuf.AllocSize) +
				SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV);

	scatterCnt = rx_scatter_info_io(rx_ring, (struct _RXD_STRUC *)pRxD, &gather_size);

	if (scatterCnt < 1) {
		bReschedule = TRUE;
		goto done;
	}

	pNewPacket = RTMP_AllocateRxPacketBuffer(rx_ring,
				((POS_COOKIE)(pAd->OS_Cookie))->pDev,
				pRxCell->DmaBuf.AllocSize,
				&AllocVa, &AllocPa);

	if (pNewPacket) {
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
						 pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

		if (scatterCnt > 1) {

			if (rx_scatter_gather_dynamic_page(pAd, resource_idx, pRxCell,
				(struct _RXD_STRUC *)pRxD, scatterCnt,
				gather_size, &build_skb_len) == FALSE)	{
				RELEASE_NDIS_PACKET_IRQ(pAd, pNewPacket, NDIS_STATUS_SUCCESS);
				bReschedule = TRUE;
				goto done;
			}
		}

		if (pRxCell->pNdisPacket) {
			build_rx_pkt_skb(&pRxPacket, (VOID *)pRxCell->pNdisPacket,
									build_skb_len, gather_size);
		} else {
			pRxPacket = NULL;
		}

#ifdef CONFIG_CSO_SUPPORT
		if (pRxPacket && (pChipCap->asic_caps & fASIC_CAP_CSO)) {
			prCso = (RX_CSO_STRUCT *)((UCHAR *)pRxD + sizeof(RXD_DEBUG_STRUC));
			NdisCopyMemory(&(pAd->rCso), prCso, sizeof(RX_CSO_STRUCT));
		}
#endif

		if (pRxPacket) {
			if (dump_rx_info(pAd, pRxCell, pRxD, resource_idx, pRxPacket) == NDIS_STATUS_FAILURE) {
				RELEASE_NDIS_PACKET_IRQ(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				pRxPacket = NULL;
			}
		}

		pRxCell->pNdisPacket = (PNDIS_PACKET)pNewPacket;
		pRxCell->DmaBuf.AllocVa = AllocVa;
		pRxCell->DmaBuf.AllocPa = AllocPa;
		pRxCell->DmaBuf.AllocSize = rx_ring->RxBufferSize;
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);
		pRxD->SDP0 = pRxCell->DmaBuf.AllocPa;
		pRxD->SDL0 = rx_ring->RxBufferSize;

		/*avoid dummy packet received before System Ready*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY)) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						("%s(): system is not ready, rx pkt drop it.\n", __func__));

			if (pRxPacket) {
				RELEASE_NDIS_PACKET_IRQ(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				pRxPacket = NULL;
				bReschedule = TRUE;
			}
		}
	} else {
		bReschedule = TRUE;
		goto done;
	}

	*pRxPending = *pRxPending - 1;
	pRxD->DIDX = 0;
	pRxD->QID = 0;
	pRxD->LS1 = 0;
	pRxD->BURST = 0;
	pRxD->CIDX = rx_ring->RxSwReadIdx;
	INC_RING_INDEX(rx_ring->RxSwReadIdx, rx_ring_size);
	rx_ring->sw_read_idx_inc++;
	rx_ring->RxCpuIdx = (rx_ring->RxSwReadIdx == 0)
						? (rx_ring_size - 1) : (rx_ring->RxSwReadIdx - 1);

	if (rx_ring->RxSwReadIdx == 0) {
		ring_round++;
	}

	pRxD->SW_INFO = ring_round;
	pRxD->DDONE = 0;
	
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif
	wmb();

#ifdef CONFIG_WIFI_PREFETCH_RXDATA
	/* prefetch to enhance throughput */
	if (*pRxPending > 0)
		prefetch(rx_ring->Cell[rx_ring->RxSwReadIdx].pNdisPacket);
#endif /* CONFIG_WIFI_PREFETCH_RXDATA */

done:
	*pbReschedule = bReschedule;
	return pRxPacket;
}

/*
*
*/
static BOOLEAN pci_rx_dma_done_handle(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	BOOLEAN bReschedule = FALSE;
	UINT32 RxProcessed, RxPending;
	PNDIS_PACKET pkt = NULL;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(hif, resource_idx);
	NDIS_SPIN_LOCK *lock = &rx_ring->ring_lock;
	RX_BLK rx_blk, *p_rx_blk = NULL;
	UINT16 max_rx_process_cnt = rx_ring->max_rx_process_cnt;

#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

	RxProcessed = RxPending = 0;

	RTMP_SEM_LOCK(lock);

	while (1) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)
			&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE)))
			break;

#ifdef ERR_RECOVERY
		if (IsStopingPdma(&pAd->ErrRecoveryCtl))
			break;
#endif /* ERR_RECOVERY */

		if (RxProcessed++ > max_rx_process_cnt) {
			bReschedule = TRUE;
			break;
		}

		pkt = hif->get_pkt_from_rx_resource[rx_ring->buf_type][rx_ring->get_pkt_method]
					(pAd, &bReschedule, &RxPending, resource_idx);

		if (pkt) {
			os_zero_mem(&rx_blk, sizeof(RX_BLK));
			p_rx_blk = &rx_blk;
			asic_rx_pkt_process(pAd, resource_idx, p_rx_blk, pkt);
		} else {
			break;
		}
	}

	if (rx_ring->sw_read_idx_inc > 0) {
		HIF_IO_WRITE32(pAd->hdev_ctrl, rx_ring->hw_cidx_addr, rx_ring->RxCpuIdx);
		rx_ring->sw_read_idx_inc = 0;
#ifdef CONFIG_TP_DBG
		tp_dbg->IoWriteRx++;
#endif
	}

	RTMP_SEM_UNLOCK(lock);

#ifdef CONFIG_TP_DBG
	RxProcessed--;
	/* 0 <= RxMaxProcessCntA <= 1/4 <= RxMaxProcessCntB <= 1/2 <= RxMaxProcessCntC < 1 == RxMaxProcessCntD */
	if (RxProcessed <= (max_rx_process_cnt >> 2))
		tp_dbg->RxMaxProcessCntA++;
	else if (RxProcessed <= (max_rx_process_cnt >> 1))
		tp_dbg->RxMaxProcessCntB++;
	else if (RxProcessed < max_rx_process_cnt)
		tp_dbg->RxMaxProcessCntC++;
	else
		tp_dbg->RxMaxProcessCntD++;
#endif

	return bReschedule;
}

static BOOLEAN pci_rx_dma_done_rxq_handle(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	BOOLEAN bReschedule = FALSE;
	UINT32 RxProcessed, RxPending;
	PNDIS_PACKET pkt = NULL;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(hif, resource_idx);
	NDIS_SPIN_LOCK *lock = &rx_ring->ring_lock;
	struct qm_ops *qm_ops = pAd->qm_ops;
	struct tm_ops *tm_ops = pAd->tm_qm_ops;
#ifndef RX_RPS_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	UINT16 max_rx_process_cnt = rx_ring->max_rx_process_cnt;

#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

	RxProcessed = RxPending = 0;

	RTMP_SEM_LOCK(lock);
#ifdef KERNEL_RPS_ADJUST
	if (pAd->ixia_mode_ctl.mode_entered)
		max_rx_process_cnt = 512;
#endif

	while (1) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)
			&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE)))
			break;

#ifdef ERR_RECOVERY
		if (IsStopingPdma(&pAd->ErrRecoveryCtl))
			break;
#endif /* ERR_RECOVERY */

		if (RxProcessed++ > max_rx_process_cnt) {
			bReschedule = TRUE;
			break;
		}

		pkt = hif->get_pkt_from_rx_resource[rx_ring->buf_type][rx_ring->get_pkt_method]
					(pAd, &bReschedule, &RxPending, resource_idx);

		if (pkt)
			qm_ops->enq_rx_dataq_pkt(pAd, pkt);
		else
			break;
	}

	if (rx_ring->sw_read_idx_inc > 0) {
		HIF_IO_WRITE32(pAd->hdev_ctrl, rx_ring->hw_cidx_addr, rx_ring->RxCpuIdx);
		rx_ring->sw_read_idx_inc = 0;
#ifdef CONFIG_TP_DBG
		tp_dbg->IoWriteRx++;
#endif
		if (pAd->rx_dequeue_sw_rps_enable)
#ifdef RX_RPS_SUPPORT
		{
			UINT cpu;
			for (cpu = 0; cpu < NR_CPUS; cpu++) {
				if (cpu_online(cpu) && (pAd->rx_que[cpu].Number > 0) && (cpu != smp_processor_id()))
					smp_call_function_single(cpu, (smp_call_func_t)RTMPRxDataDeqOffloadToOtherCPU, pAd, 0);
			}
			if (pAd->rx_que[smp_processor_id()].Number > 0)
				RTMPRxDataDeqOffloadToOtherCPU(pAd);

		}
#else
			smp_call_function_single(cap->RxSwRpsCpu, (smp_call_func_t)RTMPRxDataDeqOffloadToOtherCPU, pAd, 0);
#endif
		else
			tm_ops->schedule_task(pAd, RX_DEQ_TASK, 0);
	}

	RTMP_SEM_UNLOCK(lock);

#ifdef CONFIG_TP_DBG
	RxProcessed--;
	/* 0 <= RxMaxProcessCntA <= 1/4 <= RxMaxProcessCntB <= 1/2 <= RxMaxProcessCntC < 1 == RxMaxProcessCntD */
	if (RxProcessed <= (max_rx_process_cnt >> 2))
		tp_dbg->RxMaxProcessCntA++;
	else if (RxProcessed <= (max_rx_process_cnt >> 1))
		tp_dbg->RxMaxProcessCntB++;
	else if (RxProcessed < max_rx_process_cnt)
		tp_dbg->RxMaxProcessCntC++;
	else
		tp_dbg->RxMaxProcessCntD++;
#endif

	return bReschedule;
}

/*
*
*/
BOOLEAN pci_rx_event_dma_done_handle(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	BOOLEAN bReschedule = FALSE;
	UINT32 RxProcessed, RxPending;
	PNDIS_PACKET pkt = NULL;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(hif, resource_idx);
	NDIS_SPIN_LOCK *lock = &rx_ring->ring_lock;
	RX_BLK rx_blk, *p_rx_blk = NULL;
	UINT16 max_rx_process_cnt = rx_ring->max_rx_process_cnt;

#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

	RxProcessed = RxPending = 0;

	RTMP_SEM_LOCK(lock);

	while (1) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)
			&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE)))
			break;

#ifdef ERR_RECOVERY
		if (IsStopingPdma(&pAd->ErrRecoveryCtl))
			break;
#endif /* ERR_RECOVERY */

		if (RxProcessed++ > max_rx_process_cnt) {
			bReschedule = TRUE;
			break;
		}

		pkt = hif->get_pkt_from_rx_resource[rx_ring->buf_type][rx_ring->get_pkt_method]
					(pAd, &bReschedule, &RxPending, resource_idx);

		if (pkt) {
				os_zero_mem(&rx_blk, sizeof(RX_BLK));
				p_rx_blk = &rx_blk;
				asic_rx_pkt_process(pAd, resource_idx, p_rx_blk, pkt);
		} else {
			break;
		}
	}

	if (rx_ring->sw_read_idx_inc > 0) {
		HIF_IO_WRITE32(pAd->hdev_ctrl, rx_ring->hw_cidx_addr, rx_ring->RxCpuIdx);
		rx_ring->sw_read_idx_inc = 0;
#ifdef CONFIG_TP_DBG
		tp_dbg->IoWriteRx++;
#endif
	}

	RTMP_SEM_UNLOCK(lock);

#ifdef CONFIG_TP_DBG
	RxProcessed--;
	/* 0 <= RxMaxProcessCntA <= 1/4 <= RxMaxProcessCntB <= 1/2 <= RxMaxProcessCntC < 1 == RxMaxProcessCntD */
	if (RxProcessed <= (max_rx_process_cnt >> 2))
		tp_dbg->RxMaxProcessCntA++;
	else if (RxProcessed <= (max_rx_process_cnt >> 1))
		tp_dbg->RxMaxProcessCntB++;
	else if (RxProcessed < max_rx_process_cnt)
		tp_dbg->RxMaxProcessCntC++;
	else
		tp_dbg->RxMaxProcessCntD++;
#endif

	return bReschedule;
}

/*generic pcie hook func*/
/*
*
*/
static VOID pci_tx_dma_done_func(unsigned long data)
{
	unsigned long flags;
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)data;
	struct _PCI_HIF_T *pci_hif = pci_hif_chip->hif;
	struct _RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(pci_hif->net_dev);
	struct pci_task_group *task_group = &pci_hif_chip->task_group;
	struct pci_schedule_task_ops *sched_ops = pci_hif_chip->schedule_task_ops;
	UINT32 int_pending, int_mask;
	struct hif_pci_ring_bh_group *bh_group = &pci_hif_chip->tx_bh_group;
	UINT32 i, ring_num;
	UINT8 *ring_idx;

	int_mask = bh_group->int_mask;
	ring_num = bh_group->ring_num;
	ring_idx = bh_group->ring_idx;

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
		pci_hif_chip->intDisableMask &= ~(int_mask);
		RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
		return;
	}

#ifdef ERR_RECOVERY
	if (IsStopingPdma(&pAd->ErrRecoveryCtl)) {
		RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
		pci_hif_chip->intDisableMask &= ~(int_mask);
		RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
		return;
	}
#endif /* ERR_RECOVERY */

	RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
	int_pending = pci_hif_chip->IntPending;
	pci_hif_chip->IntPending &= ~(int_mask);
	RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);

	for (i = 0; i < ring_num; i++) {
		struct hif_pci_tx_ring *tx_ring;

		tx_ring = &pci_hif_chip->TxRing[ring_idx[i]];
		if (int_pending & tx_ring->hw_int_mask)
			pci_hif->dma_done_handle[tx_ring->ring_attr](pAd, tx_ring->resource_idx);
	}

	RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if (pci_hif_chip->IntPending & int_mask) {
		sched_ops->schedule_tx_dma_done(task_group);
		RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
		return;
	}

	mt_int_enable(pAd, pci_hif_chip, int_mask);
	RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
}

/*
*
*/
static VOID pci_rx_data_done_func(unsigned long data)
{
	unsigned long flags;
	static unsigned long need_reschdule;
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)data;
	struct _PCI_HIF_T *pci_hif = pci_hif_chip->hif;
	struct _RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(pci_hif->net_dev);
	struct pci_task_group *task_group = &pci_hif_chip->task_group;
	struct pci_schedule_task_ops *sched_ops = pci_hif_chip->schedule_task_ops;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;
	UINT32 i, int_mask, ring_num;
	struct hif_pci_ring_bh_group *bh_group = &pci_hif_chip->rx_data_bh_group;
	UINT8 *ring_idx;
	UINT32 int_pending = 0;
#ifdef RX_RPS_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif

	int_mask = bh_group->int_mask;
	ring_num = bh_group->ring_num;
	ring_idx = bh_group->ring_idx;
#ifdef RX_RPS_SUPPORT
	if (cap->rx_qm_en == FALSE)
#endif
	{
	if (ba_ctl->ba_timeout_check) {
		ba_timeout_flush(pAd);
		RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
		if (!((pci_hif_chip->IntPending & int_mask) ||
			(pci_hif_chip->intDisableMask & int_mask))) {
			RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
			return;
		}
		RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
	}
	}

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
		pci_hif_chip->intDisableMask &= ~(int_mask);
		RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
		return;
	}

	RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
	int_pending = pci_hif_chip->IntPending;
	pci_hif_chip->IntPending &= ~(int_mask);
	RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);

	for (i = 0; i < ring_num; i++) {
		struct hif_pci_rx_ring *rx_ring;

		rx_ring = &pci_hif_chip->RxRing[ring_idx[i]];
		if ((int_pending & rx_ring->hw_int_mask) || (test_and_clear_bit(i, &need_reschdule))) {
			if (pci_hif->dma_done_handle[HIF_RX_DATA](pAd, rx_ring->resource_idx))
				set_bit(i, &need_reschdule);
		}
	}

	RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);

	if ((pci_hif_chip->IntPending & int_mask) || need_reschdule) {
		sched_ops->schedule_rx_data_done(task_group);
		RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
		return;
	}

	mt_int_enable(pAd, pci_hif_chip, int_mask);

	RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
}

#ifndef PROPRIETARY_DRIVER_SUPPORT
static INT pci_rx_data_done_poll_func(struct napi_struct *napi, int budget)
{
	INT done = 0;
	unsigned long flags;
	struct pci_task_group *task_group = container_of(napi, struct pci_task_group, rx_data_done_napi_task);
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)(task_group->priv);
	struct _PCI_HIF_T *pci_hif = pci_hif_chip->hif;
	struct _RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(pci_hif->net_dev);
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;
	UINT32 i, int_mask, ring_num;
	struct hif_pci_ring_bh_group *bh_group = &pci_hif_chip->rx_data_bh_group;
	UINT8 *ring_idx;
	UINT32 int_pending = 0;
	UINT32 rx_pending;
	static BOOLEAN need_re_schedule = FALSE;
	PNDIS_PACKET pkt = NULL;
	RX_BLK rx_blk;

	int_mask = bh_group->int_mask;
	ring_num = bh_group->ring_num;
	ring_idx = bh_group->ring_idx;

	if (ba_ctl->ba_timeout_check) {
		ba_timeout_flush(pAd);

		RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
		if (!((pci_hif_chip->IntPending & int_mask) ||
			(pci_hif_chip->intDisableMask & int_mask))) {
			RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
			napi_complete(napi);
			return done;
		}
		RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
	}

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
		pci_hif_chip->intDisableMask &= ~(int_mask);
		RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
		napi_complete(napi);
		return done;
	}


	RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
	int_pending = pci_hif_chip->IntPending;
	pci_hif_chip->IntPending &= ~(int_mask);
	RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);

	for (i = 0; i < ring_num; i++) {
		struct hif_pci_rx_ring *rx_ring;
		NDIS_SPIN_LOCK *lock = NULL;
		BOOLEAN re_schedule = FALSE;

		rx_ring = &pci_hif_chip->RxRing[ring_idx[i]];
		lock = &rx_ring->ring_lock;

		RTMP_SEM_LOCK(lock);
		if ((int_pending & rx_ring->hw_int_mask) || need_re_schedule) {
			do {
				pkt = pci_hif->get_pkt_from_rx_resource[rx_ring->buf_type][rx_ring->get_pkt_method]
								(pAd, &re_schedule,
								&rx_pending, rx_ring->resource_idx);
				if (pkt) {
					os_zero_mem(&rx_blk, sizeof(RX_BLK));
					arch_ops->rx_pkt_process(pAd, rx_ring->resource_idx, &rx_blk, pkt);
				} else {
					break;
				}

				done++;
			} while (done < budget);

			if (rx_ring->sw_read_idx_inc > 0) {
				HIF_IO_WRITE32(pAd->hdev_ctrl, rx_ring->hw_cidx_addr, rx_ring->RxCpuIdx);
				rx_ring->sw_read_idx_inc = 0;
			}

			if (re_schedule)
				need_re_schedule = TRUE;
		}
		RTMP_SEM_UNLOCK(lock);
	}

	if (done < budget) {
		napi_complete(napi);
		RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
		if ((pci_hif_chip->IntPending & int_mask) || need_re_schedule) {
			RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
			napi_reschedule(napi);
		} else {
			mt_int_enable(pAd, pci_hif_chip, int_mask);
			RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
			need_re_schedule = FALSE;
		}
	} else {
		need_re_schedule = TRUE;
	}

	return done;
}
#endif

/*
*
*/
static VOID pci_rx_event_done_func(unsigned long data)
{
	unsigned long flags;
	static unsigned long need_reschdule;
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)data;
	struct _PCI_HIF_T *pci_hif = pci_hif_chip->hif;
	struct _RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(pci_hif->net_dev);
	struct pci_task_group *task_group = &pci_hif_chip->task_group;
	struct pci_schedule_task_ops *sched_ops = pci_hif_chip->schedule_task_ops;
	struct hif_pci_ring_bh_group *bh_group = &pci_hif_chip->rx_event_bh_group;
	UINT32 i, ring_num, int_mask;
	UINT8 *ring_idx;
	UINT32 int_pending = 0;

	int_mask = bh_group->int_mask;
	ring_num = bh_group->ring_num;
	ring_idx = bh_group->ring_idx;

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	/* if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)) */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
		pci_hif_chip->intDisableMask &= ~(int_mask);
		RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
		return;
	}

	RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
	int_pending = pci_hif_chip->IntPending;
	pci_hif_chip->IntPending &= ~(int_mask);
	RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);

	for (i = 0; i < ring_num; i++) {
		struct hif_pci_rx_ring *rx_ring;

		rx_ring = &pci_hif_chip->RxRing[ring_idx[i]];
		if ((int_pending & rx_ring->hw_int_mask) || (test_and_clear_bit(i, &need_reschdule))) {
			if (pci_hif->dma_done_handle[HIF_RX_EVENT](pAd, rx_ring->resource_idx))
				set_bit(i, &need_reschdule);
		}
	}

	RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);

	/* double check to avoid rotting packet  */
	if ((pci_hif_chip->IntPending & int_mask) || need_reschdule) {
		sched_ops->schedule_rx_event_done(task_group);
		RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
		return;
	}

	mt_int_enable(pAd, pci_hif_chip, int_mask);

	RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
}

/*
*
*/
static VOID pci_rx_dly_done_func(unsigned long data)
{
	unsigned long flags;
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)data;
	struct _PCI_HIF_T *pci_hif = pci_hif_chip->hif;
	struct _RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(pci_hif->net_dev);
	UINT32 int_pending = 0;
	static BOOLEAN resch_data = FALSE, resch_event = FALSE;
	struct hif_pci_ring_bh_group *bh_group = &pci_hif_chip->rx_bh_group;
	UINT32 i, ring_num, int_mask;
	struct pci_task_group *task_group = &pci_hif_chip->task_group;
	struct pci_schedule_task_ops *sched_ops = pci_hif_chip->schedule_task_ops;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;
#ifdef CONFIG_TP_DBG
	UINT32 timestamp;
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

	int_mask = bh_group->int_mask;
	ring_num = bh_group->ring_num;

#ifdef CONFIG_TP_DBG
	if (tp_dbg->debug_flag & TP_DEBUG_TIMING) {
		RTMP_IO_READ32(pAd->hdev_ctrl, LPON_FRCR, &timestamp);
		tp_dbg->TRDoneInterval[tp_dbg->TRDoneTimes++ % TP_DBG_TIME_SLOT_NUMS] = (timestamp - tp_dbg->TRDoneTimeStamp);
		tp_dbg->TRDoneTimeStamp = timestamp;
	}
#endif /* CONFIG_TP_DBG */

	if (ba_ctl->ba_timeout_check) {
		ba_timeout_flush(pAd);

		RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
		if (!((pci_hif_chip->IntPending & int_mask) ||
			(pci_hif_chip->intDisableMask & int_mask))) {
			RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
			return;
		}
		RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
	}

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
		pci_hif_chip->intDisableMask &= ~(int_mask | MT_INT_RX_DLY_BIT);
		RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
		return;
	}

	/* renew IntPending if called by ISR */
	RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);
	int_pending = pci_hif_chip->IntPending;
	pci_hif_chip->IntPending &= ~(int_mask | MT_INT_RX_DLY_BIT);
	RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);

	for (i = 0; i < ring_num; i++) {
		struct hif_pci_rx_ring *rx_ring;

		rx_ring = &pci_hif_chip->RxRing[i];

		if (rx_ring->ring_attr == HIF_RX_DATA) {
			if ((int_pending & rx_ring->hw_int_mask) || resch_data) {
				resch_data = pci_hif->dma_done_handle[HIF_RX_DATA](pAd, 0);
				resch_event = pci_hif->dma_done_handle[HIF_RX_EVENT](pAd, 1);
				break;
			}
		} else if (rx_ring->ring_attr == HIF_RX_EVENT) {
			if ((int_pending & rx_ring->hw_int_mask) || resch_event) {
				resch_event = pci_hif->dma_done_handle[HIF_RX_EVENT](pAd, 1);
				resch_data = pci_hif->dma_done_handle[HIF_RX_DATA](pAd, 0);
				break;
			}
		}
	}


	RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, flags);

	/* double check to avoid rotting packet  */
	if ((pci_hif_chip->IntPending & int_mask)
				|| resch_data || resch_event) {
		sched_ops->schedule_rx_dly_done(task_group);
		RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
		return;
	}

	mt_int_enable(pAd, pci_hif_chip, int_mask | MT_INT_RX_DLY_BIT);
	RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, flags);
}

#ifdef ERR_RECOVERY
/*
*
*/
static VOID pci_mac_recovery_func(unsigned long data)
{
	unsigned long Flags;
	UINT32 status;
	UINT32 INT_MCU_CMD = MT_McuCommand;
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)data;
	struct _RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(pci_hif_chip->hif->net_dev);

#if defined(MT7663) || defined(AXE) || defined(MT7915)
	if (IS_MT7663(pAd) || IS_AXE(pAd) || IS_MT7915(pAd))
		INT_MCU_CMD = MT_INT_MCU2HOST_SW_INT_STS_BIT;
#endif

	RTMP_SPIN_LOCK_IRQSAVE(&pci_hif_chip->LockInterrupt, &Flags);
	status = pAd->ErrRecoveryCtl.status;
	pAd->ErrRecoveryCtl.status = 0;
	RTMP_SPIN_UNLOCK_IRQRESTORE(&pci_hif_chip->LockInterrupt, &Flags);

	RTMP_MAC_RECOVERY(pAd, status);

	RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, Flags);
	pci_hif_chip->IntPending &= ~INT_MCU_CMD;
	mt_int_enable(pAd, pci_hif_chip, INT_MCU_CMD);
	RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, Flags);
}
#endif

#ifdef CONFIG_FWOWN_SUPPORT
/*
*
*/
static VOID pci_mac_fw_own_func(unsigned long data)
{
	unsigned long Flags;
	UINT32 INT_FW_CLEAR_OWN = MT_FW_CLEAR_OWN_BIT;
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)data;
	struct _RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(pci_hif_chip->hif->net_dev);

	RTMP_INT_LOCK(&pci_hif_chip->LockInterrupt, Flags);
	mt_int_enable(pAd, pci_hif_chip, INT_FW_CLEAR_OWN);
	RTMP_INT_UNLOCK(&pci_hif_chip->LockInterrupt, Flags);
}
#endif

/*
*
*/
static VOID pci_subsys_int_func(unsigned long data)
{
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)data;
	struct _RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(pci_hif_chip->hif->net_dev);

	chip_subsys_int_handler(pAd, pci_hif_chip);
}

/*
*
*/
static VOID pci_sw_int_func(unsigned long data)
{
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)data;
	struct _RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(pci_hif_chip->hif->net_dev);

	chip_sw_int_handler(pAd, pci_hif_chip);
}

/*
*
*/
VOID pci_free_rx_buf(void *hdev_ctrl, UCHAR resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(hif, resource_idx);

	if (rx_ring->cur_free_buf_len == FREE_BUF_1k) {
		free_rx_buf_1k(rx_ring);
	} else if (rx_ring->cur_free_buf_len == FREE_BUF_64k) {
		free_rx_buf_64k(rx_ring);
	} else {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
			("%s: fail, rx_ring->cur_free_buf_len = %d\n", __func__, rx_ring->cur_free_buf_len));
	}
}

/*
*
*/
static BOOLEAN pci_free_txd(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	struct hif_pci_tx_ring *tx_ring;
	RTMP_DMACB *dma_cb;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pTxD;
	TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	UINT16 tx_ring_size;
#ifdef CONFIG_TP_DBG
	/* struct tp_debug *tp_dbg = &pAd->tr_ctl->tp_dbg;*/
#endif
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	NDIS_SPIN_LOCK *lock;

	tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);
	lock = &tx_ring->tx_done_lock;
	tx_ring_size = tx_ring->ring_size;

	RTMP_SEM_LOCK(lock);
	HIF_IO_READ32(pAd->hdev_ctrl, tx_ring->hw_didx_addr, &tx_ring->TxDmaIdx);

	while (tx_ring->TxSwFreeIdx != tx_ring->TxDmaIdx) {
		dma_cb = &tx_ring->Cell[tx_ring->TxSwFreeIdx];

#ifdef RT_BIG_ENDIAN
		pDestTxD = (TXD_STRUC *)(dma_cb->AllocVa);
		NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&hw_hdr_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

		PCI_UNMAP_SINGLE(pAd, dma_cb->DmaBuf.AllocPa, dma_cb->DmaBuf.AllocSize, RTMP_PCI_DMA_TODEVICE);

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(dma_cb->AllocPa, TXD_SIZE);
		INC_RING_INDEX(tx_ring->TxSwFreeIdx, tx_ring_size);
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
	}
	RTMP_SEM_UNLOCK(lock);

	return FALSE;
}

/*
*
*/
static NDIS_STATUS pci_build_resource_idx_ge(struct _PCI_HIF_T *pci_hif)
{
	UINT32 i, j;
	UINT8 data_ring_idx[5] = {0}; /* total should be 4 AC + 1 ALTX */
	UINT8 data_ring_cnt = 0;
	UINT8 tx_res_num = pci_hif->tx_res_num;

	/* find out tx data ring's resource index */
	for (i = 0; i < tx_res_num; i++) {
		struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(pci_hif, i);

		if (tx_ring->ring_attr == HIF_TX_DATA && data_ring_cnt < 5) {
			data_ring_idx[data_ring_cnt] = tx_ring->resource_idx;
			data_ring_cnt++;
		}
	}

	/* check number of tx ring matches */
	if (data_ring_cnt != 5)
		return NDIS_STATUS_FAILURE;

	/* build swq to SW TxRing mapping */
	for (i = 0; i < DBDC_BAND_NUM; i++) {
		for (j = 0; j < PACKET_TYPE_NUM; j++) {
			switch (j) {
			case TX_DATA:
				pci_hif->swq_to_tx_ring[i][j][0] = data_ring_idx[0];
				pci_hif->swq_to_tx_ring[i][j][1] = data_ring_idx[1];
				pci_hif->swq_to_tx_ring[i][j][2] = data_ring_idx[2];
				pci_hif->swq_to_tx_ring[i][j][3] = data_ring_idx[3];
				break;
			case TX_DATA_HIGH_PRIO:
				pci_hif->swq_to_tx_ring[i][j][0] = data_ring_idx[0];
				pci_hif->swq_to_tx_ring[i][j][1] = data_ring_idx[1];
				pci_hif->swq_to_tx_ring[i][j][2] = data_ring_idx[2];
				pci_hif->swq_to_tx_ring[i][j][3] = data_ring_idx[3];
				break;
			case TX_MGMT:
				pci_hif->swq_to_tx_ring[i][j][0] = data_ring_idx[1];
				pci_hif->swq_to_tx_ring[i][j][1] = data_ring_idx[1];
				pci_hif->swq_to_tx_ring[i][j][2] = data_ring_idx[1];
				pci_hif->swq_to_tx_ring[i][j][3] = data_ring_idx[1];
				break;
			case TX_ALTX:
				pci_hif->swq_to_tx_ring[i][j][0] = data_ring_idx[4];
				pci_hif->swq_to_tx_ring[i][j][1] = data_ring_idx[4];
				pci_hif->swq_to_tx_ring[i][j][2] = data_ring_idx[4];
				pci_hif->swq_to_tx_ring[i][j][3] = data_ring_idx[4];
				break;
			default:
				pci_hif->swq_to_tx_ring[i][j][0] = 0;
				pci_hif->swq_to_tx_ring[i][j][1] = 0;
				pci_hif->swq_to_tx_ring[i][j][2] = 0;
				pci_hif->swq_to_tx_ring[i][j][3] = 0;
				break;
			}
		}
	}

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static NDIS_STATUS pci_build_resource_idx_fp(struct _PCI_HIF_T *pci_hif)
{
	UINT32 i, j;
	UINT8 data_ring_idx[2] = {0}; /* total should be 2 band */
	UINT8 data_ring_cnt = 0;
	UINT8 tx_res_num = pci_hif->tx_res_num;
	struct _RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(pci_hif->net_dev);

	/* find out tx data ring's resource index */
	for (i = 0; i < tx_res_num; i++) {
		struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(pci_hif, i);

		if (tx_ring->ring_attr == HIF_TX_DATA && data_ring_cnt < 2) {
			data_ring_idx[data_ring_cnt] = tx_ring->resource_idx;
			data_ring_cnt++;
		}
	}

	/* chip specific ring assignment */
	chip_hif_pci_data_ring_assign(pAd->hdev_ctrl, data_ring_idx);

	/* check number of tx ring matches */
	if (data_ring_cnt != 2)
		return NDIS_STATUS_FAILURE;

	/* build swq to SW TxRing mapping */
	for (i = 0; i < DBDC_BAND_NUM; i++) {
		for (j = 0; j < PACKET_TYPE_NUM; j++) {
			pci_hif->swq_to_tx_ring[i][j][0] = data_ring_idx[i];
			pci_hif->swq_to_tx_ring[i][j][1] = data_ring_idx[i];
			pci_hif->swq_to_tx_ring[i][j][2] = data_ring_idx[i];
			pci_hif->swq_to_tx_ring[i][j][3] = data_ring_idx[i];
		}
	}

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static INT pci_irq_init(void *hdev_ctrl)
{
	unsigned long irqFlags;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);
	int i;

	RTMP_INT_LOCK(&ad->irq_lock, irqFlags);

	/* traverse each pci_hif_chip */
	for (i = 0; i < hif->pci_hif_chip_num; i++) {
		struct pci_hif_chip *hif_chip = hif->pci_hif_chip[i];

		hif_chip->intDisableMask = 0;
		hif_chip->IntPending = 0;
	}

	RTMP_INT_UNLOCK(&ad->irq_lock, irqFlags);

	/* init sub-layer interrupt enable mask, only for chips after P18 */
	if (ops->irq_init)
		ops->irq_init(ad);

	return FALSE;
}

/*
*
*/
static NDIS_STATUS pci_alloc_ring(struct pci_hif_chip *pci_hif, UINT8 tx_res_num, UINT8 rx_res_num)
{
	INT index;
	struct hif_pci_rx_ring *pRxRing = NULL;
	struct hif_pci_tx_ring *pTxRing = NULL;

	/* allocate pci hif data structure which depends on ChipCap */
	os_alloc_mem(NULL, (UCHAR **)&pTxRing, (tx_res_num)*sizeof(struct hif_pci_tx_ring));

	if (pTxRing == NULL)
		return NDIS_STATUS_FAILURE;

	NdisZeroMemory(pTxRing, (tx_res_num)*sizeof(struct hif_pci_tx_ring));
	pci_hif->TxRing = pTxRing;

	os_alloc_mem(NULL, (UCHAR **)&pRxRing, (rx_res_num)*sizeof(struct hif_pci_rx_ring));

	if (pRxRing == NULL)
		return NDIS_STATUS_FAILURE;

	NdisZeroMemory(pRxRing, (rx_res_num)*sizeof(struct hif_pci_rx_ring));
	pci_hif->RxRing = pRxRing;

	/* spinlock is allocated after data structure is ready */
	for (index = 0; index < tx_res_num; index++) {
		OS_NdisAllocateSpinLock(&pci_hif->TxRing[index].tx_lock);
		OS_NdisAllocateSpinLock(&pci_hif->TxRing[index].tx_done_lock);
	}

	for (index = 0; index < rx_res_num; index++)
		OS_NdisAllocateSpinLock(&pci_hif->RxRing[index].ring_lock);

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static VOID pci_free_ring(struct pci_hif_chip *hif, UINT8 tx_res_num, UINT8 rx_res_num)
{
	INT index;

	/* spinlock must be freed before its data structure is freed */
	for (index = 0; index < tx_res_num; index++)
		OS_NdisFreeSpinLock(&hif->TxRing[index].ring_lock);

	for (index = 0; index < rx_res_num; index++)
		OS_NdisFreeSpinLock(&hif->RxRing[index].ring_lock);

	/* free pci hif data structure which depends on ChipCap */
	os_free_mem(hif->TxRing);
	os_free_mem(hif->RxRing);
}

/*
* pci_data_init: fill in the data struct PCI_HIF_T
*/
static NDIS_STATUS pci_data_init(VOID *hif_ctrl)
{
	struct pci_hif_chip *pci_hif = hif_ctrl;
	const struct hif_pci_tx_ring_desc *tx_ring_layout;
	const struct hif_pci_rx_ring_desc *rx_ring_layout;
	struct hif_pci_ring_bh_group *ring_bh_group;
	UINT32 i;
	UINT8 tx_res_num = pci_hif->tx_res_num;
	UINT8 rx_res_num = pci_hif->rx_res_num;

	pci_alloc_ring(pci_hif, tx_res_num, rx_res_num);

	NdisAllocateSpinLock(NULL, &pci_hif->LockInterrupt);

	/* tx: init pci hif data struct based on hif_pci_ring_desc */
	tx_ring_layout = pci_hif->ring_layout.tx_ring_layout;

	for (i = 0; i < tx_res_num; i++) {
		struct hif_pci_tx_ring_desc ring_desc = tx_ring_layout[i];
		struct hif_pci_tx_ring *tx_ring = &pci_hif->TxRing[i];

		tx_ring->hw_desc_base = ring_desc.hw_desc_base;
		tx_ring->hw_int_mask = ring_desc.hw_int_mask;
		tx_ring->ring_attr = ring_desc.ring_attr;
		tx_ring->ring_size = ring_desc.ring_size;
		tx_ring->band_idx = ring_desc.band_idx;
	}

	/* rx: init pci hif data struct based on hif_pci_ring_desc */
	rx_ring_layout = pci_hif->ring_layout.rx_ring_layout;

	for (i = 0; i < rx_res_num; i++) {
		struct hif_pci_rx_ring_desc ring_desc = rx_ring_layout[i];
		struct hif_pci_rx_ring *rx_ring = &pci_hif->RxRing[i];

		rx_ring->hw_desc_base = ring_desc.hw_desc_base;
		rx_ring->hw_int_mask = ring_desc.hw_int_mask;
		rx_ring->ring_attr = ring_desc.ring_attr;
		rx_ring->event_type = ring_desc.event_type;
		rx_ring->ring_size = ring_desc.ring_size;
		rx_ring->delay_int_en = ring_desc.delay_int_en;
		rx_ring->dl_dly_ctl_tbl = ring_desc.dl_dly_ctl_tbl;
		rx_ring->dl_dly_ctl_tbl_size = ring_desc.dl_dly_ctl_tbl_size;
		rx_ring->ul_dly_ctl_tbl = ring_desc.ul_dly_ctl_tbl;
		rx_ring->ul_dly_ctl_tbl_size = ring_desc.ul_dly_ctl_tbl_size;
		rx_ring->max_rx_process_cnt = ring_desc.max_rx_process_cnt;
		rx_ring->max_sw_read_idx_inc = ring_desc.max_sw_read_idx_inc;
		rx_ring->buf_type = ring_desc.buf_type;
		rx_ring->band_idx = ring_desc.band_idx;
	}

	/* rx buffer allocation type */
	for (i = 0; i < rx_res_num; i++) {
		struct hif_pci_rx_ring *rx_ring = &pci_hif->RxRing[i];

		if (rx_ring->ring_attr == HIF_RX_DATA) {
			rx_ring->get_pkt_method = GET_PKT_DDONE;
			rx_ring->buf_flags = BUF_DEFAULT;
			rx_ring->buf_debug = 0;
		} else if (rx_ring->ring_attr == HIF_RX_EVENT) {
			rx_ring->get_pkt_method = GET_PKT_DDONE;
			rx_ring->buf_flags = BUF_DEFAULT;
			rx_ring->buf_debug = 0;
		} else {
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
						("%s: ring_attr:%d, unknown ring_attr\n",
						__func__, rx_ring->ring_attr));
		}
	}

	/* build ring group for bottom-half's reference */
	ring_bh_group = &pci_hif->tx_bh_group;
	ring_bh_group->int_mask = 0;
	ring_bh_group->ring_num = 0;
	for (i = 0; i < tx_res_num; i++) {
		struct hif_pci_tx_ring *tx_ring = &pci_hif->TxRing[i];

		ring_bh_group->int_mask |= tx_ring->hw_int_mask;
		ring_bh_group->ring_idx[ring_bh_group->ring_num] = i;
		ring_bh_group->ring_num++;
	}

	ring_bh_group = &pci_hif->rx_bh_group;
	ring_bh_group->int_mask = 0;
	ring_bh_group->ring_num = 0;
	for (i = 0; i < rx_res_num; i++) {
		struct hif_pci_rx_ring *rx_ring = &pci_hif->RxRing[i];

		ring_bh_group->int_mask |= rx_ring->hw_int_mask;
		ring_bh_group->ring_idx[ring_bh_group->ring_num] = i;
		ring_bh_group->ring_num++;
	}

	ring_bh_group = &pci_hif->rx_data_bh_group;
	ring_bh_group->int_mask = 0;
	ring_bh_group->ring_num = 0;
	for (i = 0; i < rx_res_num ; i++) {
		struct hif_pci_rx_ring *rx_ring = &pci_hif->RxRing[i];

		if (rx_ring->ring_attr != HIF_RX_DATA)
			continue;

		ring_bh_group->int_mask |= rx_ring->hw_int_mask;
		ring_bh_group->ring_idx[ring_bh_group->ring_num] = i;
		ring_bh_group->ring_num++;
	}

	ring_bh_group = &pci_hif->rx_event_bh_group;
	ring_bh_group->int_mask = 0;
	ring_bh_group->ring_num = 0;
	for (i = 0; i < rx_res_num; i++) {
		struct hif_pci_rx_ring *rx_ring = &pci_hif->RxRing[i];

		if (rx_ring->ring_attr != HIF_RX_EVENT)
			continue;

		ring_bh_group->int_mask |= rx_ring->hw_int_mask;
		ring_bh_group->ring_idx[ring_bh_group->ring_num] = i;
		ring_bh_group->ring_num++;
	}

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static NDIS_STATUS pci_data_exit(void *hif_ctrl)
{
	struct pci_hif_chip *pci_hif = hif_ctrl;
	UINT8 tx_res_num = pci_hif->tx_res_num;
	UINT8 rx_res_num = pci_hif->rx_res_num;

	NdisFreeSpinLock(&pci_hif->LockInterrupt);
	pci_free_ring(pci_hif, tx_res_num, rx_res_num);
	return NDIS_STATUS_SUCCESS;
}


/*
*
*/
static UINT32 pci_get_resouce_type(void *hif, UINT8 resource_idx)
{
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(hif, resource_idx);
	enum resource_attr ring_attr = rx_ring->ring_attr;

	return ring_attr;
}

/*
*
*/
static VOID pci_hif_chip_link_deinit(struct _PCI_HIF_T *hif)
{
	os_free_mem(hif->tx_ring);
	os_free_mem(hif->rx_ring);
	os_free_mem(hif->pci_hif_chip);
}


/*
*
*/
static VOID pci_reset_txrx_ring_mem(void *hdev_ctrl)
{
	int index, j;
	TXD_STRUC *pTxD;
	RTMP_DMACB *dma_cb;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif /* RT_BIG_ENDIAN */
	PNDIS_PACKET pPacket;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	UINT8 num_of_tx_ring = hif->tx_res_num;
	UINT8 num_of_rx_ring = hif->rx_res_num;
	UINT16 tx_ring_size = 0;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);

	/* Free Tx Ring Packet*/
	for (index = 0; index < num_of_tx_ring; index++) {
		struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, index);
		tx_ring_size = tx_ring->ring_size;

		for (j = 0; j < tx_ring_size; j++) {
			dma_cb = &tx_ring->Cell[j];
#ifdef RT_BIG_ENDIAN
			pDestTxD = (TXD_STRUC *)(dma_cb->AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (dma_cb->AllocVa);
#endif /* RT_BIG_ENDIAN */

			if (dma_cb->DmaBuf.AllocSize > 0)
				PCI_UNMAP_SINGLE(ad, dma_cb->DmaBuf.AllocPa, dma_cb->DmaBuf.AllocSize, RTMP_PCI_DMA_TODEVICE);

			pPacket = dma_cb->pNdisPacket;

			if (pPacket) {
				/*the type of packet is hooked at DATA ring hook which should be skb.*/
				if (tx_ring->ring_attr == HIF_TX_DATA) {
					PCI_UNMAP_SINGLE(ad,
							 dma_cb->PacketPa,
							 GET_OS_PKT_LEN(pPacket),
							 RTMP_PCI_DMA_TODEVICE);
				/*the type of packet is hooked at cmd ring hook which is internal pakcet*/
				} else if (tx_ring->ring_attr == HIF_TX_CMD || tx_ring->ring_attr == HIF_TX_CMD_WM) {
					pTxD->DMADONE = 0;
					PCI_UNMAP_SINGLE(ad,
							 pTxD->SDPtr0,
							 pTxD->SDLen0,
							 RTMP_PCI_DMA_TODEVICE);
				} else {
					MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
						("%s: ring_attr:%d, specific handle is needed?\n",
							__func__, tx_ring->ring_attr));
				}

				RELEASE_NDIS_PACKET(ad, pPacket, NDIS_STATUS_SUCCESS);
			}

			/*Always assign pNdisPacket as NULL after clear*/
			dma_cb->pNdisPacket = NULL;
			pPacket = dma_cb->pNextNdisPacket;

			if (pPacket) {
				PCI_UNMAP_SINGLE(ad, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(ad, pPacket, NDIS_STATUS_SUCCESS);
			}

			/*Always assign pNextNdisPacket as NULL after clear*/
			dma_cb->pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}

		NdisZeroMemory(tx_ring->Cell, tx_ring_size * sizeof(RTMP_DMACB));
	}

	for (index = 0; index < num_of_rx_ring; index++) {
		UINT16 RxRingSize;
#ifdef CONFIG_WIFI_BUILD_SKB
		UINT skb_data_size = 0;
#endif
		struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(hif, index);
		enum resource_attr attr = rx_ring->ring_attr;

		RxRingSize = rx_ring->ring_size;

		for (j = RxRingSize - 1; j >= 0; j--) {
			PNDIS_PACKET cur_pkt = NULL;
			VOID *cur_va;
			NDIS_PHYSICAL_ADDRESS cur_pa;
			struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(hdev_ctrl);
			BOOLEAN whnat_en = FALSE;

			dma_cb = &rx_ring->Cell[j];

#ifdef WHNAT_SUPPORT
			whnat_en = ad->CommonCfg.whnat_en;
#endif

			if (whnat_en && (cap->tkn_info.feature & TOKEN_RX)
					&& (attr == HIF_RX_DATA)) {
				cur_va = NULL;
				cur_pkt = NULL;
			} else {
				cur_va = dma_cb->DmaBuf.AllocVa;
				cur_pa = dma_cb->DmaBuf.AllocPa;
				cur_pkt = dma_cb->pNdisPacket;
			}

			if (cur_va && cur_pkt) {
				PCI_UNMAP_SINGLE(ad, cur_pa, dma_cb->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);

				if (attr == HIF_RX_DATA) {
#ifdef CONFIG_WIFI_BUILD_SKB
					skb_data_size = SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + dma_cb->DmaBuf.AllocSize) +
							SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV);

					if (skb_data_size <= PAGE_SIZE) {
						DEV_FREE_FRAG_BUF(cur_pkt);
					} else {
						os_free_mem(cur_pkt);
					}
#else /* CONFIG_WIFI_BUILD_SKB */
					RELEASE_NDIS_PACKET(ad,
							cur_pkt,
							NDIS_STATUS_SUCCESS);
#endif /* CONFIG_WIFI_BUILD_SKB */
				}
			}
		}

		if (attr == HIF_RX_EVENT)
			reset_rx_free_buffer(hdev_ctrl, index);

		NdisZeroMemory(rx_ring->Cell, RxRingSize * sizeof(RTMP_DMACB));

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 3, 0)
		if (rx_ring->buf_type == DYNAMIC_PAGE_ALLOC) {

			if (rx_ring->rx_page.va) {
				struct page *page;

				page = virt_to_page(rx_ring->rx_page.va);
				rx_page_frag_cache_drain(page, rx_ring->rx_page.pagecnt_bias);
				NdisZeroMemory(&rx_ring->rx_page, sizeof(rx_ring->rx_page));
			}
		}
#endif

	}

	if (ad->FragFrame.pFragPacket) {
		RELEASE_NDIS_PACKET(ad, ad->FragFrame.pFragPacket, NDIS_STATUS_SUCCESS);
		ad->FragFrame.pFragPacket = NULL;
	}

	pci_hif_chip_link_deinit(hif);

	NdisFreeSpinLock(&ad->CmdQLock);
}

/*
*
*/
static VOID pci_free_txrx_ring_mem(void *hif_ctrl)
{
	INT num;
	struct pci_hif_chip *hif = hif_ctrl;
	VOID *pdev = hif->pdev;
	UINT8 num_of_tx_ring = hif->tx_res_num;
	UINT8 num_of_rx_ring = hif->rx_res_num;


	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("--> %s\n", __func__));

	for (num = 0; num < num_of_rx_ring; num++) {
		enum resource_attr attr = hif->RxRing[num].ring_attr;

		desc_ring_free(hif, &hif->RxRing[num].desc_ring);

		if (attr == HIF_RX_EVENT)
			release_rx_free_buffer(hif, num);

		os_free_mem(hif->RxRing[num].Cell);
	}

	/* Free 1st TxBufSpace and TxDesc buffer*/
	for (num = 0; num < num_of_tx_ring; num++) {
		if (hif->TxRing[num].buf_space.AllocVa) {
			RTMP_FreeFirstTxBuffer(pdev,
						hif->TxRing[num].buf_space.AllocSize,
						FALSE, hif->TxRing[num].buf_space.AllocVa,
						hif->TxRing[num].buf_space.AllocPa);
		}
		NdisZeroMemory(&hif->TxRing[num].buf_space, sizeof(RTMP_DMABUF));
		desc_ring_free(hif, &hif->TxRing[num].desc_ring);
		os_free_mem(hif->TxRing[num].Cell);
	}

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<-- %s\n", __func__));
}

static NDIS_STATUS pci_hif_chip_link(struct _PCI_HIF_T *pci_hif)
{
	struct pci_hif_chip *hif_chip = pci_hif->main_hif_chip;
	struct _DL_LIST head;
	struct multi_hif_entry *hif_entry;
	UINT32 gid;
	UINT8 i = 0;

	gid = multi_hif_entry_gid_get(hif_chip);
	multi_hif_entry_get_by_gid(gid, &head);
	pci_hif->pci_hif_chip_num = DlListLen(&head);
	os_alloc_mem(NULL, (UCHAR **)&pci_hif->pci_hif_chip,
			pci_hif->pci_hif_chip_num * sizeof(struct pci_hif_chip *));

	if (pci_hif->pci_hif_chip == NULL)
		return NDIS_STATUS_FAILURE;

	/* link _PCI_HIF_T to pci_hif_chip */
	DlListForEach(hif_entry, &head, struct multi_hif_entry, glist) {
		pci_hif->pci_hif_chip[i++] = (struct pci_hif_chip *)&hif_entry->hif_ctrl;
	}

	/* link pci_hif_chip to _PCI_HIF_T */
	for (i = 0; i < pci_hif->pci_hif_chip_num; i++) {
		struct pci_hif_chip *hif_chip_entry;
		hif_chip_entry = pci_hif->pci_hif_chip[i];
		hif_chip_entry->hif = pci_hif;
	}

	return NDIS_STATUS_SUCCESS;
}


static NDIS_STATUS pci_hif_chip_merge_txrx_ring(struct _PCI_HIF_T *pci_hif)
{
	UINT8 i = 0;
	UINT8 j = 0;
	UINT8 tx_ring_ridx = 0;
	UINT8 rx_ring_ridx = 0;
	struct pci_hif_chip *hif_chip = NULL;

	pci_hif->tx_res_num = 0;
	pci_hif->rx_res_num = 0;
	for (i = 0; i < pci_hif->pci_hif_chip_num; i++) {
		pci_hif->tx_res_num += pci_hif->pci_hif_chip[i]->tx_res_num;
		pci_hif->rx_res_num += pci_hif->pci_hif_chip[i]->rx_res_num;
	}

	os_alloc_mem(NULL, (UCHAR **)&pci_hif->tx_ring, pci_hif->tx_res_num * sizeof(struct hif_pci_tx_ring *));
	if (pci_hif->tx_ring == NULL)
		return NDIS_STATUS_FAILURE;
	os_alloc_mem(NULL, (UCHAR **)&pci_hif->rx_ring, pci_hif->rx_res_num * sizeof(struct hif_pci_rx_ring *));
	if (pci_hif->rx_ring == NULL)
		return NDIS_STATUS_FAILURE;

	for (i = 0; i < pci_hif->pci_hif_chip_num; i++) {
		hif_chip = pci_hif->pci_hif_chip[i];
		for (j = 0; j < hif_chip->tx_res_num; j++) {
			hif_chip->TxRing[j].resource_idx = tx_ring_ridx;
			pci_hif->tx_ring[tx_ring_ridx] = &hif_chip->TxRing[j];
			tx_ring_ridx++;
		}

		for (j = 0; j < hif_chip->rx_res_num; j++) {
			hif_chip->RxRing[j].resource_idx = rx_ring_ridx;
			if (hif_chip->RxRing[j].event_type & HOST_MSDU_ID_RPT)
				pci_hif->host_msdu_id_rpt_idx = rx_ring_ridx;
			pci_hif->rx_ring[rx_ring_ridx] = &hif_chip->RxRing[j];
			rx_ring_ridx++;
		}
	}

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static NDIS_STATUS pci_hif_chip_merge(void *hdev_ctrl)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(hdev_ctrl);
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	UINT32 asic_caps = hc_get_asic_cap(hdev_ctrl);
#ifdef WHNAT_SUPPORT
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);
#endif
	int i = 0;
	BOOLEAN whnat_en = FALSE;
#ifdef WHNAT_SUPPORT
	whnat_en = ad->CommonCfg.whnat_en;
#endif

	/*link slave or specific merge depend on chip*/
	chip_hif_chip_match(hdev_ctrl);

	pci_hif_chip_link(pci_hif);

	pci_hif_chip_merge_txrx_ring(pci_hif);

	/* _PCI_HIF_T init */
	pci_hif->dma_done_handle[HIF_TX_DATA] = pci_tx_dma_done_handle;
	pci_hif->dma_done_handle[HIF_TX_CMD] = pci_cmd_dma_done_handle;
	pci_hif->dma_done_handle[HIF_TX_CMD_WM] = pci_cmd_dma_done_handle;
	pci_hif->dma_done_handle[HIF_TX_FWDL] = pci_fwdl_dma_done_handle;

	if (cap->rx_qm_en)
		pci_hif->dma_done_handle[HIF_RX_DATA] = pci_rx_dma_done_rxq_handle;
	else
		pci_hif->dma_done_handle[HIF_RX_DATA] = pci_rx_dma_done_handle;

	pci_hif->dma_done_handle[HIF_RX_EVENT] = pci_rx_event_dma_done_handle;

	if (whnat_en && (cap->tkn_info.feature & TOKEN_RX)) {
		pci_hif->get_pkt_from_rx_resource[DYNAMIC_PAGE_ALLOC][GET_PKT_DDONE] =
							pci_get_pkt_dynamic_page_ddone_token;
	} else {
		pci_hif->get_pkt_from_rx_resource[DYNAMIC_PAGE_ALLOC][GET_PKT_DDONE] =
							pci_get_pkt_dynamic_page_ddone;
	}

	pci_hif->get_pkt_from_rx_resource[DYNAMIC_PAGE_ALLOC][GET_PKT_IO] =
						pci_get_pkt_dynamic_page_io;
	pci_hif->get_pkt_from_rx_resource[DYNAMIC_PAGE_ALLOC_DEBUG][GET_PKT_DDONE] =
						pci_get_pkt_dynamic_page_ddone_debug;
	pci_hif->get_pkt_from_rx_resource[DYNAMIC_PAGE_ALLOC_DEBUG][GET_PKT_IO] =
						pci_get_pkt_dynamic_page_io_debug;
	pci_hif->get_pkt_from_rx_resource[DYNAMIC_SLAB_ALLOC][GET_PKT_DDONE] =
						pci_get_pkt_dynamic_slab_ddone;
	pci_hif->get_pkt_from_rx_resource[DYNAMIC_SLAB_ALLOC][GET_PKT_IO] =
						pci_get_pkt_dynamic_slab_io;
	pci_hif->get_pkt_from_rx_resource[PRE_SLAB_ALLOC][GET_PKT_DDONE] =
						pci_get_pkt_pre_slab_ddone;
	pci_hif->get_pkt_from_rx_resource[PRE_SLAB_ALLOC][GET_PKT_IO] =
						pci_get_pkt_pre_slab_io;

	for (i = 0; i < pci_hif->tx_res_num; i++) {
		struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(pci_hif, i);

		switch (tx_ring->ring_attr) {
		case HIF_TX_CMD:
			/* point pci_hif->cmd_ring to its TxRing instance */
			pci_hif->ctrl_ring = tx_ring;
			break;
		case HIF_TX_FWDL:
			/* point pci_hif->fwdl_ring to its TxRing instance */
			pci_hif->fwdl_ring = tx_ring;
			break;
		default:
			break;
		}
	}

	for (i = 0; i < pci_hif->rx_res_num; i++) {
		struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(pci_hif, i);

		if ((asic_caps & fASIC_CAP_WHNAT) &&
			(rx_ring->ring_attr == HIF_RX_EVENT))
			rx_ring->get_pkt_method = GET_PKT_IO;
	}

	if (IS_GE_QM(cap->qm) && pci_build_resource_idx_ge(pci_hif) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	if (!IS_GE_QM(cap->qm) && pci_build_resource_idx_fp(pci_hif) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static NDIS_STATUS pci_init_txrx_ring_mem(void *hdev_ctrl)
{
	INT num, index;
	ULONG RingBasePaHigh, RingBasePaLow;
	VOID *RingBaseVa;
	struct hif_pci_rx_ring *rx_ring;
	struct hif_pci_tx_ring *tx_ring;
	RTMP_DMABUF *pDmaBuf, *pDescRing;
	RTMP_DMACB *dma_cb;
	PNDIS_PACKET pPacket = NULL;
	TXD_STRUC *pTxD;
	RXD_STRUC *pRxD;
	ULONG ErrorValue = 0;
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	UINT8 num_of_tx_ring = 0;
	UINT8 num_of_rx_ring = 0;
	UINT16 tx_ring_size = 0;
	VOID *pdev = NULL;
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);
	struct pci_hif_chip *hif_chip = hif->main_hif_chip;
	struct pci_hif_chip *slave_hif_chip = NULL;
	PKT_TOKEN_CB *cb = hc_get_ct_cb(hdev_ctrl);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(hdev_ctrl);
	BOOLEAN whnat_en = FALSE;
#ifdef WHNAT_SUPPORT
	whnat_en = ad->CommonCfg.whnat_en;
#endif

	/*merge hif_chip*/
	pci_hif_chip_merge(hdev_ctrl);

	num_of_tx_ring = hif->tx_res_num;
	num_of_rx_ring = hif->rx_res_num;
	pdev = hif_chip->pdev;

	/* Initialize All Tx Ring Descriptors and associated buffer memory*/
	for (num = 0; num < num_of_tx_ring; num++) {
		tx_ring = pci_get_tx_ring_by_ridx(hif, num);
		tx_ring_size = tx_ring->ring_size;
		pDescRing = &tx_ring->desc_ring;

		switch (tx_ring->ring_attr) {
		case (HIF_TX_DATA):
		{
			VOID *BufBaseVa;
			/* linking Tx Ring Descriptor and associated buffer memory */
			/* memory zero the  Tx ring descriptor's memory */
			NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
			/* Save PA & VA for further operation*/
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
			RingBasePaLow = RTMP_GetPhysicalAddressLow(pDescRing->AllocPa);
			RingBaseVa = pDescRing->AllocVa;
			/* Zero init all 1st TXBuf's memory for this TxRing*/
			NdisZeroMemory(tx_ring->buf_space.AllocVa, tx_ring->buf_space.AllocSize);

			/* Save PA & VA for further operation */
			BufBaseVa = tx_ring->buf_space.AllocVa;

			for (index = 0; index < tx_ring_size; index++) {
				dma_cb = &tx_ring->Cell[index];
				dma_cb->pNdisPacket = NULL;
				dma_cb->pNextNdisPacket = NULL;
				/* Init Tx Ring Size, Va, Pa variables*/
				dma_cb->AllocSize = TXD_SIZE;
				dma_cb->AllocVa = RingBaseVa;
				RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
				RTMP_SetPhysicalAddressLow(dma_cb->AllocPa, RingBasePaLow);
				/* Setup Tx Buffer size & address. only 802.11 header will store in this space */
				pDmaBuf = &dma_cb->DmaBuf;
				pDmaBuf->AllocSize = hif_chip->tx_dma_1st_buffer_size;
				pDmaBuf->AllocVa = BufBaseVa;
				/* link the pre-allocated TxBuf to TXD */
				pTxD = (TXD_STRUC *)dma_cb->AllocVa;
				pTxD->DMADONE = 0;
#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);
				RingBasePaLow += TXD_SIZE;
				RingBaseVa = (PUCHAR)RingBaseVa + TXD_SIZE;
				/* advance to next TxBuf address */
				BufBaseVa = (PUCHAR)BufBaseVa + pDmaBuf->AllocSize;
			}

			tx_ring->tx_ring_state = TX_RING_HIGH;
			tx_ring->tx_ring_low_water_mark = 5;
			tx_ring->tx_ring_high_water_mark = tx_ring->tx_ring_low_water_mark + 10;
			tx_ring->tx_ring_full_cnt = 0;
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
				 ("TxRing[%d]: attr:%d, total %d entry initialized\n", num, tx_ring->ring_attr, index));
		}
			break;
		case (HIF_TX_CMD):
		case (HIF_TX_CMD_WM):
		case (HIF_TX_FWDL):
		{
			/* Initialize CTRL Ring and associated buffer memory */
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
			RingBasePaLow = RTMP_GetPhysicalAddressLow(pDescRing->AllocPa);
			RingBaseVa = pDescRing->AllocVa;
			NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);

			for (index = 0; index < tx_ring_size; index++) {
				dma_cb = &tx_ring->Cell[index];
				dma_cb->pNdisPacket = NULL;
				dma_cb->pNextNdisPacket = NULL;
				/* Init Ctrl Ring Size, Va, Pa variables */
				dma_cb->AllocSize = TXD_SIZE;
				dma_cb->AllocVa = RingBaseVa;
				RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
				RTMP_SetPhysicalAddressLow(dma_cb->AllocPa, RingBasePaLow);
				/* Offset to next ring descriptor address */
				RingBasePaLow += TXD_SIZE;
				RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;
				/* link the pre-allocated TxBuf to TXD */
				pTxD = (TXD_STRUC *)dma_cb->AllocVa;
				pTxD->DMADONE = 1;
#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);
				/* no pre-allocated buffer required in CtrlRing for scatter-gather case */
			}
			/* init CTRL ring index pointer */
			tx_ring->TxSwFreeIdx = 0;
			tx_ring->TxCpuIdx = 0;

			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE,
				("ring_attr:%d, total %d entry initialized\n", tx_ring->ring_attr, index));
		}
			break;
		default:
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
				 ("TxRing[%d]: wrong attr:%d\n", num, tx_ring->ring_attr));
			Status = NDIS_STATUS_INVALID_DATA;
			return	Status;
		}
	}

	slave_hif_chip = hif->slave_hif_chip;

	/* Initialize Rx Ring and associated buffer memory */
	for (num = 0; num < num_of_rx_ring; num++) {
		UINT16 RxRingSize;
		UINT16 RxBufferSize = 0;
		enum resource_attr attr;

		rx_ring = pci_get_rx_ring_by_ridx(hif, num);
		pDescRing = &rx_ring->desc_ring;
		attr = rx_ring->ring_attr;
		NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,  ("RX[%d] DESC %p size = %lu\n",
				 num, pDescRing->AllocVa, pDescRing->AllocSize));
		/* Save PA & VA for further operation */
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow(pDescRing->AllocPa);
		RingBaseVa = pDescRing->AllocVa;

		if (attr == HIF_RX_DATA) {
			RxBufferSize = RX_BUFFER_AGGRESIZE;
		} else if (attr == HIF_RX_EVENT) {
			RxBufferSize = RX1_BUFFER_SIZE;
		}

		if (slave_hif_chip &&
			rx_ring->band_idx == BAND1_RX_PCIE0) {
			rx_ring->ring_size = 10;
		} else if (whnat_en && (cap->tkn_info.feature & TOKEN_RX)) {
			if ((rx_ring->band_idx == BAND0_RX_PCIE0) ||
				(rx_ring->band_idx == BAND1_RX_PCIE0))
				rx_ring->ring_size = 0xf8;
		}

		RxRingSize = rx_ring->ring_size;

		/* Linking Rx Ring and associated buffer memory */
		for (index = 0; index < RxRingSize; index++) {
			dma_cb = &rx_ring->Cell[index];
			/* Init RX Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = RXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow(dma_cb->AllocPa, RingBasePaLow);
			/* Offset to next ring descriptor address */
			RingBasePaLow += RXD_SIZE;
			RingBaseVa = (PUCHAR)RingBaseVa + RXD_SIZE;
			/* Setup Rx associated Buffer size & allocate share memory */
			pDmaBuf = &dma_cb->DmaBuf;
			pDmaBuf->AllocSize = RxBufferSize;

			pPacket = RTMP_AllocateRxPacketBuffer(
						rx_ring,
						pdev,
						pDmaBuf->AllocSize,
						&pDmaBuf->AllocVa,
						&pDmaBuf->AllocPa);

			/* keep allocated rx packet */
			dma_cb->pNdisPacket = pPacket;

			/* Error handling*/
			if (pDmaBuf->AllocVa == NULL) {
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Failed to allocate RxRing's 1st buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* Write RxD buffer address & allocated buffer length */
			pRxD = (RXD_STRUC *)dma_cb->AllocVa;
			pRxD->SDP0 = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
			pRxD->DDONE = 0;
			pRxD->SDL0 = RxBufferSize;

			/* rx token initialization */
			if (whnat_en && (cap->tkn_info.feature & TOKEN_RX)
					&& (attr == HIF_RX_DATA)) {
				UINT32 tkn_rx_id;
				tkn_rx_id = token_rx_dmad_init(&cb->rx_que, pPacket,
										pDmaBuf->AllocSize, pDmaBuf->AllocVa, pDmaBuf->AllocPa);
				pRxD->SDP1 |= (tkn_rx_id << RX_TOKEN_ID_SHIFT);
				pRxD->SDL1 |= (1 << TO_HOST_SHIFT);
			}

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#endif
			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pRxD, dma_cb->AllocSize);
		}
	}

	NdisZeroMemory(&ad->FragFrame, sizeof(FRAGMENT_FRAME));
	ad->FragFrame.pFragPacket = RTMP_AllocateFragPacketBuffer(ad, RX_BUFFER_NORMSIZE);

	if (ad->FragFrame.pFragPacket == NULL)
		Status = NDIS_STATUS_RESOURCES;

	for (index = 0; index < num_of_tx_ring; index++) {
		tx_ring = pci_get_tx_ring_by_ridx(hif, index);

		/* Init TX rings index pointer */
		tx_ring->TxSwFreeIdx = 0;
		tx_ring->TxCpuIdx = 0;
	}

	/* Init RX Ring index pointer */
	for (index = 0; index < num_of_rx_ring; index++) {
		UINT16 RxRingSize;
		UINT16 RxBufferSize = 0;
		enum resource_attr attr;

		rx_ring = pci_get_rx_ring_by_ridx(hif, index);

		attr = rx_ring->ring_attr;

		if (attr == HIF_RX_DATA) {
			RxBufferSize = RX_BUFFER_AGGRESIZE;
		} else if (attr == HIF_RX_EVENT) {
			RxBufferSize = RX1_BUFFER_SIZE;
		}

		RxRingSize = rx_ring->ring_size;
		rx_ring->RxSwReadIdx = 0;
		rx_ring->RxCpuIdx = RxRingSize - 1;
		rx_ring->ring_size = RxRingSize;
		rx_ring->RxBufferSize = RxBufferSize;
	}

	ad->PrivateInfo.TxRingFullCnt = 0;
	return Status;
}

/*
*
*/
static NDIS_STATUS pci_alloc_tx_rx_ring_mem(void *hif_ctrl)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	INT num;
	ULONG ErrorValue = 0;
	UINT16 tx_ring_size = 0;
	struct pci_hif_chip *hif = hif_ctrl;
	VOID *pdev = hif->pdev;
	UINT8 num_of_tx_ring = hif->tx_res_num;
	UINT8 num_of_rx_ring = hif->rx_res_num;

	do {
		for (num = 0; num < num_of_tx_ring; num++) {
			struct hif_pci_tx_ring *pTxRing = &hif->TxRing[num];

			tx_ring_size = pTxRing->ring_size;
			/* Allocate Tx ring descriptor's memory (5 TX rings = 4 ACs + 1 HCCA)*/
			desc_ring_alloc(pdev, &pTxRing->desc_ring, tx_ring_size * TXD_SIZE);

			if (pTxRing->desc_ring.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE,
				("TxRing[%d]: Attr:%d, total %d bytes allocated\n",
					num, pTxRing->ring_attr, (INT)pTxRing->desc_ring.AllocSize));
			if (pTxRing->ring_attr == HIF_TX_DATA) {
				/* Allocate all 1st TXBuf's memory for this TxRing */
				pTxRing->buf_space.AllocSize = tx_ring_size * hif->tx_dma_1st_buffer_size;

				RTMP_AllocateFirstTxBuffer(
					pdev,
					num,
					pTxRing->buf_space.AllocSize,
					FALSE,
					&pTxRing->buf_space.AllocVa,
					&pTxRing->buf_space.AllocPa);

				if (pTxRing->buf_space.AllocVa == NULL) {
					ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
					MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
						("Failed to allocate a big buffer\n"));
					Status = NDIS_STATUS_RESOURCES;
					break;
				}
			}

			os_alloc_mem(NULL, (UCHAR **)&pTxRing->Cell, tx_ring_size * sizeof(struct _RTMP_DMACB));
			NdisZeroMemory(pTxRing->Cell, tx_ring_size * sizeof(struct _RTMP_DMACB));
		}

		if (Status == NDIS_STATUS_RESOURCES)
			break;

		/* Alloc RX ring desc memory */
		for (num = 0; num < num_of_rx_ring; num++) {
			UINT16 RxRingSize;
			struct hif_pci_rx_ring *pRxRing = &hif->RxRing[num];
			enum resource_attr attr = pRxRing->ring_attr;

			RxRingSize = pRxRing->ring_size;

			desc_ring_alloc(pdev,
					&hif->RxRing[num].desc_ring,
					RxRingSize * RXD_SIZE);

			if (hif->RxRing[num].desc_ring.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* Allocate RX Free Buf */
			if (attr == HIF_RX_EVENT)
				alloc_rx_free_buffer(hif, num, RxRingSize);

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Rx[%d] Ring: total %d bytes allocated\n",
				num, (INT)hif->RxRing[num].desc_ring.AllocSize));


			os_alloc_mem(NULL, (UCHAR **)&pRxRing->Cell, RxRingSize * sizeof(struct _RTMP_DMACB));
			NdisZeroMemory(pRxRing->Cell, RxRingSize * sizeof(struct _RTMP_DMACB));
		}
	} while (FALSE);

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,
			 ("<-- %s, Status=%x\n", __func__, Status));
	return Status;
}

/*
*
*/
static VOID pci_dma_disable(void *hdev_ctrl)
{
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);

	chip_set_hif_dma(ad, DMA_TX_RX, FALSE);
}

/*
*
*/
static VOID pci_dma_enable(void *hdev_ctrl)
{
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);
#ifdef CONFIG_WIFI_MSI_SUPPORT
	UINT32 val = 0;
#endif

	chip_wait_hif_dma_idle(ad, ALL_DMA, 200, 1000);
	RtmpusecDelay(50);
#ifdef CONFIG_WIFI_MSI_SUPPORT
	RTMP_IO_READ32(ad->hdev_ctrl, (MT_PCIE_MAC_BASE_ADDR + 0x1ac), &val);
	val |= 0x00000002;
	RTMP_IO_WRITE32(ad->hdev_ctrl, (MT_PCIE_MAC_BASE_ADDR + 0x1ac), val);
	RTMP_IO_WRITE32(ad->hdev_ctrl, (MT_PCIE_MAC_BASE_ADDR + 0x18c), 0xff);
#endif
	chip_set_hif_dma(ad, DMA_TX_RX, TRUE);
}

/*
*
*/
static VOID pci_dma_reset(void *hdev_ctrl)
{
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);

	/* disable WPDMA Tx/Rx */
	chip_set_hif_dma(ad, DMA_TX_RX, FALSE);

	/* wait WPDMA idle then reset*/
	if (chip_wait_hif_dma_idle(ad, ALL_DMA, 200, 1000)) {
		chip_reset_hif_dma(ad);
	} else {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<== %s(): WPDMA reset fail! \n", __func__));
	}
}

/*
*
*/
static BOOLEAN pci_poll_txrx_empty(void *hdev_ctrl, UINT8 pcie_port_or_all)
{
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);
	BOOLEAN sts = FALSE;

	RtmpOsMsDelay(100);

	/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
	RTMP_SET_FLAG(ad, fRTMP_ADAPTER_POLL_IDLE);

	sts = chip_wait_hif_dma_idle(ad, pcie_port_or_all, 20000, 50);

	/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
	RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_POLL_IDLE);

	return sts;
}

/*mcu*/
/*
*
*/
static INT32 pci_kick_out_cmd_msg(PRTMP_ADAPTER pAd, struct cmd_msg *msg)
{
	int ret = NDIS_STATUS_SUCCESS;
	unsigned long flags = 0;
	ULONG FreeNum;
	PNDIS_PACKET net_pkt = msg->net_pkt;
	UINT32 SwIdx = 0;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen = 0;
	PACKET_INFO PacketInfo;
	TXD_STRUC *pTxD;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *ring = hif->ctrl_ring;
	NDIS_SPIN_LOCK *lock = &ring->tx_lock;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return NDIS_STATUS_FAILURE;

	FreeNum = pci_get_tx_free_num(ring);
	if (FreeNum < 10) {
		hif->dma_done_handle[ring->ring_attr](pAd, ring->resource_idx);
		FreeNum = pci_get_tx_free_num(ring);
	}

	if (FreeNum == 0) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("%s FreeNum == 0 (TxCpuIdx = %d, TxDmaIdx = %d, TxSwFreeIdx = %d)\n",
				  __func__, ring->TxCpuIdx,
				  ring->TxDmaIdx, ring->TxSwFreeIdx));
		return NDIS_STATUS_FAILURE;
	}

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	RTMP_QueryPacketInfo(net_pkt, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	if (pSrcBufVA == NULL) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
		return NDIS_STATUS_FAILURE;
	}

	SwIdx = ring->TxCpuIdx;
#ifdef RT_BIG_ENDIAN
	pDestTxD  = (TXD_STRUC *)ring->Cell[SwIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#else
	pTxD  = (TXD_STRUC *)ring->Cell[SwIdx].AllocVa;
#endif
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
	ring->Cell[SwIdx].pNdisPacket = net_pkt;
	ring->Cell[SwIdx].pNextNdisPacket = NULL;
	ring->Cell[SwIdx].PacketPa = PCI_MAP_SINGLE(pAd, (pSrcBufVA),
			(SrcBufLen), 0, RTMP_PCI_DMA_TODEVICE);
	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen0 = SrcBufLen;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = ring->Cell[SwIdx].PacketPa;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(SrcBufPA, SrcBufLen);
	RTMP_DCACHE_FLUSH(ring->Cell[SwIdx].AllocPa, TXD_SIZE);
	/* Increase TX_CTX_IDX, but write to register later.*/
	INC_RING_INDEX(ring->TxCpuIdx, ring->ring_size);

	if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg)) {
		AndesQueueTailCmdMsg(&ctl->ackq, msg, wait_ack);
		msg->sending_time_in_jiffies = jiffies;
	} else
		AndesQueueTailCmdMsg(&ctl->tx_doneq, msg, tx_done);

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
		return -1;
	}

	HIF_IO_WRITE32(pAd->hdev_ctrl, ring->hw_cidx_addr, ring->TxCpuIdx);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
	return ret;
}

/*
*
*/
static INT32 pci_kick_out_fwdl_msg(PRTMP_ADAPTER pAd, struct cmd_msg *msg)
{
	int ret = NDIS_STATUS_SUCCESS;
	unsigned long flags = 0;
	ULONG FreeNum;
	PNDIS_PACKET net_pkt = msg->net_pkt;
	UINT32 SwIdx = 0;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen = 0;
	PACKET_INFO PacketInfo;
	TXD_STRUC *pTxD;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	struct hif_pci_tx_ring *pRing;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return -1;

	pRing = hif->fwdl_ring;
	FreeNum = pci_get_tx_free_num(pRing);

	if (FreeNum < 10) {
		hif->dma_done_handle[pRing->ring_attr](pAd, pRing->resource_idx);
		FreeNum = pci_get_tx_free_num(pRing);
	}

	if (FreeNum == 0) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("%s FreeNum == 0 (TxCpuIdx = %d, TxDmaIdx = %d, TxSwFreeIdx = %d)\n",
				  __func__, pRing->TxCpuIdx, pRing->TxDmaIdx, pRing->TxSwFreeIdx));
		return NDIS_STATUS_FAILURE;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&pRing->tx_lock, &flags);
	RTMP_QueryPacketInfo(net_pkt, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	if (pSrcBufVA == NULL) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pRing->tx_lock, &flags);
		return NDIS_STATUS_FAILURE;
	}

	SwIdx = pRing->TxCpuIdx;
#ifdef RT_BIG_ENDIAN
	pDestTxD  = (TXD_STRUC *)pRing->Cell[SwIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#else
	pTxD  = (TXD_STRUC *)pRing->Cell[SwIdx].AllocVa;
#endif
#ifdef RT_BIG_ENDIAN
	/* RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD); */
#endif /* RT_BIG_ENDIAN */
	pRing->Cell[SwIdx].pNdisPacket = net_pkt;
	pRing->Cell[SwIdx].pNextNdisPacket = NULL;
	pRing->Cell[SwIdx].PacketPa = PCI_MAP_SINGLE(pAd, (pSrcBufVA),
								  (SrcBufLen), 0, RTMP_PCI_DMA_TODEVICE);
	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen0 = SrcBufLen;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = pRing->Cell[SwIdx].PacketPa;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRing->Cell[SwIdx].PacketPa, SrcBufLen);
	RTMP_DCACHE_FLUSH(pRing->Cell[SwIdx].AllocPa, TXD_SIZE);
	/* Increase TX_CTX_IDX, but write to register later.*/
	INC_RING_INDEX(pRing->TxCpuIdx, pRing->ring_size);

	if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
		AndesQueueTailCmdMsg(&ctl->ackq, msg, wait_ack);
	else
		AndesQueueTailCmdMsg(&ctl->tx_doneq, msg, tx_done);

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pRing->tx_lock, &flags);
		return -1;
	}

	HIF_IO_WRITE32(pAd->hdev_ctrl, pRing->hw_cidx_addr, pRing->TxCpuIdx);
	RTMP_SPIN_UNLOCK_IRQRESTORE(&pRing->tx_lock, &flags);
	return ret;
}

/*
*
*/
static VOID pci_kick_out_data_tx(RTMP_ADAPTER *pAd, TX_BLK *tx_blk, UCHAR resource_idx)
{
	ULONG flags;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);
	NDIS_SPIN_LOCK *lock = &tx_ring->tx_lock;

	RTMP_IRQ_LOCK(lock, flags);
#if defined(CTXD_SCATTER_AND_GATHER) || defined(CTXD_MEM_CPY)
	if (hif->main_hif_chip->max_ctxd_agg_num > 1)
		asic_write_last_tx_resource(pAd, resource_idx);
#endif

	HIF_IO_WRITE32(pAd->hdev_ctrl, tx_ring->hw_cidx_addr, tx_ring->TxCpuIdx);
	RTMP_IRQ_UNLOCK(lock, flags);
}

/*
*
*/
static UCHAR *pci_get_tx_buf(void *hdev_ctrl, struct _TX_BLK *tx_blk, UCHAR resource_idx, UCHAR frame_type)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);
	USHORT tx_cpu_idx = tx_ring->TxCpuIdx;
	UINT32 offset = 0;

	if (frame_type != TX_FRAG_FRAME) {
#ifdef CTXD_MEM_CPY

		if (hif->main_hif_chip->max_ctxd_agg_num > 1)
			offset = tx_ring->cur_txd_cnt * hif->main_hif_chip->ctxd_size_unit;
#endif
		return (UCHAR *)tx_ring->Cell[tx_cpu_idx].DmaBuf.AllocVa + offset;
	} else
		return (UCHAR *)tx_blk->HeaderBuffer;
}

/*
*
*/
static UINT32 pci_get_tx_resource_free_num(void *hdev_ctrl, UINT8 resource_idx)
{
	UINT32 free_num;
	ULONG flags;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);
	UINT16 tx_ring_size = tx_ring->ring_size;
	NDIS_SPIN_LOCK *lock = &tx_ring->tx_lock;

	RTMP_IRQ_LOCK(lock, flags);
	free_num = tx_ring->TxSwFreeIdx > tx_ring->TxCpuIdx ?
			   tx_ring->TxSwFreeIdx - tx_ring->TxCpuIdx - 1 :
			   tx_ring->TxSwFreeIdx + tx_ring_size - tx_ring->TxCpuIdx - 1;
	RTMP_IRQ_UNLOCK(lock, flags);
	return free_num;
}

/*
*
*/
static VOID pci_rx_event_process(RTMP_ADAPTER *ad, struct cmd_msg *rx_msg)
{
	switch (rx_msg->state) {
	case rx_receive_fail:
		break;
	default:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("unknown msg state(%d)\n", rx_msg->state));
		break;
	}
}

/*
*
*/
static VOID pci_mcu_init(void *hdev_ctrl)
{
	/*no pcie hif specific */
}

/*
*
*/
static VOID pci_mcu_exit(void *hdev_ctrl)
{
	/*no pcie hif specific */
}

/*
*
*/
static VOID pci_mcu_fw_init(void *hdev_ctrl)
{
	struct _RTMP_ADAPTER *pAd = hc_get_hdev_privdata(hdev_ctrl);
	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	Ctl->fwdl_ctrl.stage = FWDL_STAGE_FW_NOT_DL;

	/* Enable Interrupt*/
	chip_interrupt_enable(pAd);
	hif_dma_enable(pAd->hdev_ctrl);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
}

/*
*
*/
static VOID pci_mcu_fw_exit(void *hdev_ctrl)
{
	struct _RTMP_ADAPTER *pAd = hc_get_hdev_privdata(hdev_ctrl);

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	hif_dma_disable(pAd->hdev_ctrl);
	chip_interrupt_disable(pAd);
}

/*
*
*/
static INT pci_sys_init(void *hdev_ctrl)
{
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);

	pci_irq_init(hdev_ctrl);
	chip_init_hif_dma(ad);
	asic_init_txrx_ring(ad);
	chip_init_dmasch(ad);

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static UINT32 pci_get_resource_idx(void *hdev_ctrl, UINT8 band_idx, enum PACKET_TYPE pkt_type, UCHAR q_idx)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);

	return pci_hif->swq_to_tx_ring[band_idx][pkt_type][q_idx];
}

static INT tasklet_schedule_tx_done(struct pci_task_group *task_group)
{
	RTMP_OS_TASKLET_SCHE(&task_group->tx_dma_done_task);
	return NDIS_STATUS_SUCCESS;
}

static INT tasklet_schedule_rx_data_done(struct pci_task_group *task_group)
{
	RTMP_OS_TASKLET_SCHE(&task_group->rx_data_done_task);
	return NDIS_STATUS_SUCCESS;
}

static INT tasklet_schedule_rx_event_done(struct pci_task_group *task_group)
{
	RTMP_OS_TASKLET_SCHE(&task_group->rx_event_done_task);
	return NDIS_STATUS_SUCCESS;
}
static INT tasklet_schedule_rx_dly_done(struct pci_task_group *task_group)
{
	RTMP_OS_TASKLET_SCHE(&task_group->rx_dly_done_task);
	return NDIS_STATUS_SUCCESS;
}

#ifndef PROPRIETARY_DRIVER_SUPPORT
static INT napi_schedule_rx_data_done(struct pci_task_group *task_group)
{
	napi_schedule(&task_group->rx_data_done_napi_task);
	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef ERR_RECOVERY
static INT tasklet_schedule_mac_recovery(struct pci_task_group *task_group)
{
	RTMP_OS_TASKLET_SCHE(&task_group->mac_error_recovey_task);
	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef CONFIG_FWOWN_SUPPORT
static INT tasklet_schedule_mac_fw_own(struct pci_task_group *task_group)
{
	RTMP_OS_TASKLET_SCHE(&task_group->mac_fw_own_task);
	return NDIS_STATUS_SUCCESS;
}
#endif

static INT tasklet_schedule_subsys_int(struct pci_task_group *task_group)
{
	RTMP_OS_TASKLET_SCHE(&task_group->subsys_int_task);
	return NDIS_STATUS_SUCCESS;
}

static INT tasklet_schedule_sw_int(struct pci_task_group *task_group)
{
	RTMP_OS_TASKLET_SCHE(&task_group->sw_int_task);
	return NDIS_STATUS_SUCCESS;
}

static struct pci_schedule_task_ops tasklet_schedule_ops = {
	.schedule_tx_dma_done = tasklet_schedule_tx_done,
	.schedule_rx_data_done = tasklet_schedule_rx_data_done,
	.schedule_rx_event_done = tasklet_schedule_rx_event_done,
	.schedule_rx_dly_done = tasklet_schedule_rx_dly_done,
#ifdef ERR_RECOVERY
	.schedule_mac_recovery = tasklet_schedule_mac_recovery,
#endif
#ifdef CONFIG_FWOWN_SUPPORT
	.schedule_mac_fw_own = tasklet_schedule_mac_fw_own,
#endif
	.schedule_subsys_int = tasklet_schedule_subsys_int,
	.schedule_sw_int = tasklet_schedule_sw_int,
};

#ifndef PROPRIETARY_DRIVER_SUPPORT
static struct pci_schedule_task_ops tasklet_napi_schedule_ops = {
	.schedule_tx_dma_done = tasklet_schedule_tx_done,
	.schedule_rx_data_done = napi_schedule_rx_data_done,
	.schedule_rx_event_done = tasklet_schedule_rx_event_done,
	.schedule_rx_dly_done = tasklet_schedule_rx_dly_done,
#ifdef ERR_RECOVERY
	.schedule_mac_recovery = tasklet_schedule_mac_recovery,
#endif
#ifdef CONFIG_FWOWN_SUPPORT
	.schedule_mac_fw_own = tasklet_schedule_mac_fw_own,
#endif
	.schedule_subsys_int = tasklet_schedule_subsys_int,
	.schedule_sw_int = tasklet_schedule_sw_int,
};
#endif

/*
 *
 */
static NDIS_STATUS pci_init_task_group(void *hdev_ctrl)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(hdev_ctrl);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	struct pci_hif_chip *hif_chip;
	struct pci_task_group *task_group;
	UINT8 i;

	for (i = 0; i < hif->pci_hif_chip_num; i++) {
		hif_chip = hif->pci_hif_chip[i];
		task_group = &hif_chip->task_group;

		if (cap->hif_tm == TASKLET_METHOD) {
			hif_chip->schedule_task_ops = &tasklet_schedule_ops;
			RTMP_OS_TASKLET_INIT(NULL, &task_group->tx_dma_done_task, pci_tx_dma_done_func, (unsigned long)hif_chip);
			RTMP_OS_TASKLET_INIT(NULL, &task_group->rx_data_done_task, pci_rx_data_done_func, (unsigned long)hif_chip);
			RTMP_OS_TASKLET_INIT(NULL, &task_group->rx_event_done_task, pci_rx_event_done_func, (unsigned long)hif_chip);
			RTMP_OS_TASKLET_INIT(NULL, &task_group->rx_dly_done_task, pci_rx_dly_done_func, (unsigned long)hif_chip);
#ifdef ERR_RECOVERY
			RTMP_OS_TASKLET_INIT(NULL, &task_group->mac_error_recovey_task, pci_mac_recovery_func, (unsigned long)hif_chip);
#endif
#ifdef CONFIG_FWOWN_SUPPORT
			RTMP_OS_TASKLET_INIT(NULL, &task_group->mac_fw_own_task, pci_mac_fw_own_func, (unsigned long)hif_chip);
#endif
			RTMP_OS_TASKLET_INIT(NULL, &task_group->subsys_int_task, pci_subsys_int_func, (unsigned long)hif_chip);
			RTMP_OS_TASKLET_INIT(NULL, &task_group->sw_int_task, pci_sw_int_func, (unsigned long)hif_chip);
		}
#ifndef PROPRIETARY_DRIVER_SUPPORT
		else if (cap->hif_tm == TASKLET_NAPI_METHOD) {
			struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);

			hif_chip->schedule_task_ops = &tasklet_napi_schedule_ops;
			init_dummy_netdev(&task_group->napi_dev);
			task_group->priv = (VOID *)hif_chip;
			netif_napi_add(&task_group->napi_dev, &task_group->rx_data_done_napi_task, pci_rx_data_done_poll_func, NAPI_POLL_WEIGHT);
			ad->tr_ctl.napi = (VOID *)&task_group->rx_data_done_napi_task;
			napi_enable(&task_group->rx_data_done_napi_task);
			RTMP_OS_TASKLET_INIT(NULL, &task_group->rx_event_done_task, pci_rx_event_done_func, (unsigned long)hif_chip);
			RTMP_OS_TASKLET_INIT(NULL, &task_group->rx_dly_done_task, pci_rx_dly_done_func, (unsigned long)hif_chip);
			RTMP_OS_TASKLET_INIT(NULL, &task_group->tx_dma_done_task, pci_tx_dma_done_func, (unsigned long)hif_chip);
#ifdef ERR_RECOVERY
			RTMP_OS_TASKLET_INIT(NULL, &task_group->mac_error_recovey_task, pci_mac_recovery_func, (unsigned long)hif_chip);
#endif
#ifdef CONFIG_FWOWN_SUPPORT
			RTMP_OS_TASKLET_INIT(NULL, &task_group->mac_fw_own_task, pci_mac_fw_own_func, (unsigned long)hif_chip);
#endif
			RTMP_OS_TASKLET_INIT(NULL, &task_group->subsys_int_task, pci_subsys_int_func, (unsigned long)hif_chip);
			RTMP_OS_TASKLET_INIT(NULL, &task_group->sw_int_task, pci_sw_int_func, (unsigned long)hif_chip);
		}
#endif
	}

	return NDIS_STATUS_SUCCESS;
}

/*
 *
 */
static NDIS_STATUS pci_reset_task_group(void *hdev_ctrl)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(hdev_ctrl);
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	struct pci_hif_chip *hif_chip;
	struct pci_task_group *task_group;
	UINT8 i;

	for (i = 0; i < hif->pci_hif_chip_num; i++) {
		hif_chip = hif->pci_hif_chip[i];
		task_group = &hif_chip->task_group;

		if (cap->hif_tm == TASKLET_METHOD) {
			RTMP_OS_TASKLET_KILL(&task_group->tx_dma_done_task);
			RTMP_OS_TASKLET_KILL(&task_group->rx_data_done_task);
			RTMP_OS_TASKLET_KILL(&task_group->rx_event_done_task);
			RTMP_OS_TASKLET_KILL(&task_group->rx_dly_done_task);
#ifdef ERR_RECOVERY
			RTMP_OS_TASKLET_KILL(&task_group->mac_error_recovey_task);
#endif
#ifdef CONFIG_FWOWN_SUPPORT
			RTMP_OS_TASKLET_KILL(&task_group->mac_fw_own_task);
#endif
			RTMP_OS_TASKLET_KILL(&task_group->subsys_int_task);
			RTMP_OS_TASKLET_KILL(&task_group->sw_int_task);
		}
#ifndef PROPRIETARY_DRIVER_SUPPORT
		else if (cap->hif_tm == TASKLET_NAPI_METHOD) {
			RTMP_OS_TASKLET_KILL(&task_group->tx_dma_done_task);
			netif_napi_del(&task_group->rx_data_done_napi_task);
			RTMP_OS_TASKLET_KILL(&task_group->rx_event_done_task);
			RTMP_OS_TASKLET_KILL(&task_group->rx_dly_done_task);
#ifdef ERR_RECOVERY
			RTMP_OS_TASKLET_KILL(&task_group->mac_error_recovey_task);
#endif
#ifdef CONFIG_FWOWN_SUPPORT
			RTMP_OS_TASKLET_KILL(&task_group->mac_fw_own_task);
#endif
			RTMP_OS_TASKLET_KILL(&task_group->subsys_int_task);
			RTMP_OS_TASKLET_KILL(&task_group->sw_int_task);
		}
#endif
	}

	return NDIS_STATUS_SUCCESS;
}

/*
 *
 */
static NDIS_STATUS pci_register_irq(void *hdev_ctrl)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	struct pci_hif_chip *hif_chip;
	struct device *dev = NULL;
	int i, ret;

	for (i = 0; i < hif->pci_hif_chip_num; i++) {
		hif_chip = hif->pci_hif_chip[i];
		dev = (struct device *)(hif_chip->pdev);
		ret = RtmpOSIRQRequest(hif_chip->irq, dev_name(dev), rt2860_interrupt, hif_chip);
		if (ret)
			return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

/*
 *
 */
static NDIS_STATUS pci_free_irq_local(void *hdev_ctrl)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	struct pci_hif_chip *hif_chip;
	UINT32 i;

	for (i = 0; i < hif->pci_hif_chip_num; i++) {
		hif_chip = hif->pci_hif_chip[i];
		RtmpOSIRQRelease(hif_chip->irq, hif_chip);
	}

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
VOID pci_handle_irq(void *hif_chip)
{
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)hif_chip;

	pci_hif_chip->isr(pci_hif_chip);
}

/*
*
*/
static VOID pci_hif_init(void *hdev_ctrl)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);

	NdisAllocateSpinLock(NULL, &hif->io_remap_lock);
	hif->bPCIclkOff = FALSE;
}

/*
*
*/
static VOID pci_hif_exit(void *hdev_ctrl)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);

	if (hif) {
		NdisFreeSpinLock(&hif->io_remap_lock);
	}
}

/*
*
*/
static VOID pci_mcu_unlink_ackq(struct cmd_msg *msg)
{
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	DL_LIST *list = &ctl->ackq;

	AndesUnlinkCmdMsg(msg, list);
}

/*
*
*/
static UINT8 pci_get_tx_res_num(VOID *hif_ctrl)
{
	return ((struct _PCI_HIF_T *)hif_ctrl)->tx_res_num;
}

/*
*
*/
static UINT8 pci_get_rx_res_num(VOID *hif_ctrl)
{
	return ((struct _PCI_HIF_T *)hif_ctrl)->rx_res_num;
}

/*register func.*/
/*
*
*/
static VOID pci_ops_register(void *hdev_ctrl)
{
	struct hif_ops *ops = hc_get_hif_ops(hdev_ctrl);

	/*register hif ops*/
	ops->get_resource_type = pci_get_resouce_type;
	ops->free_txd = pci_free_txd;
	ops->free_rx_buf = pci_free_rx_buf;
	ops->get_tx_buf = pci_get_tx_buf;
	ops->init_txrx_mem = pci_init_txrx_ring_mem;
	ops->reset_txrx_mem = pci_reset_txrx_ring_mem;
	ops->dma_reset = pci_dma_reset;
	ops->dma_enable = pci_dma_enable;
	ops->dma_disable = pci_dma_disable;
	ops->poll_txrx_empty = pci_poll_txrx_empty;
	ops->get_tx_resource_free_num = pci_get_tx_resource_free_num;
	ops->get_resource_idx = pci_get_resource_idx;
	ops->init_task_group = pci_init_task_group;
	ops->reset_task_group = pci_reset_task_group;
	ops->register_irq = pci_register_irq;
	ops->free_irq = pci_free_irq_local;
	/*mcu related*/
	ops->kick_out_cmd_msg = pci_kick_out_cmd_msg;
	ops->kick_out_fwdl_msg = pci_kick_out_fwdl_msg;
	ops->kickout_data_tx = pci_kick_out_data_tx;
	ops->kickout_nullframe_tx = MiniportMMRequest;
	ops->rx_event_process = pci_rx_event_process;
	ops->mcu_init = pci_mcu_init;
	ops->mcu_exit = pci_mcu_exit;
	ops->mcu_fw_init = pci_mcu_fw_init;
	ops->mcu_fw_exit = pci_mcu_fw_exit;
	ops->mcu_unlink_ackq = pci_mcu_unlink_ackq;
	/*core init free*/
	ops->sys_init = pci_sys_init;
#ifdef CONFIG_STA_SUPPORT
	ops->ps_poll_enq = EnqueuePsPoll;
	ops->sta_sleep_auto_wakeup = RT28xxPciStaAsicSleepAutoWakeup;
	ops->sta_wakeup = RT28xxPciStaAsicWakeup;
#endif /*CONFIG_STA_SUPPORT*/
	ops->cmd_thread = RTPCICmdThread;
	ops->get_tx_res_num = pci_get_tx_res_num;
	ops->get_rx_res_num = pci_get_rx_res_num;
}

/*export function for other module*/
/*
*
*/
VOID pci_core_ops_register(void *hdev_ctrl)
{
	pci_hif_init(hdev_ctrl);
	pci_io_ops_register(hdev_ctrl);
	pci_ops_register(hdev_ctrl);
}

/*
*
*/
VOID pci_core_ops_unregister(void *hdev_ctrl)
{
	pci_hif_exit(hdev_ctrl);
}

/*
*
*/
inline UINT32 pci_get_tx_resource_free_num_nolock(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	UINT32 free_num;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);
	UINT16 tx_ring_size = tx_ring->ring_size;

	free_num = tx_ring->TxSwFreeIdx > tx_ring->TxCpuIdx ?
			   tx_ring->TxSwFreeIdx - tx_ring->TxCpuIdx - 1 :
			   tx_ring->TxSwFreeIdx + tx_ring_size - tx_ring->TxCpuIdx - 1;

	return free_num;
}

/*
*
*/
inline BOOLEAN pci_is_tx_resource_empty(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);

	return ((tx_ring->TxDmaIdx == tx_ring->TxCpuIdx)	? 1 : 0);
}

/*
*
*/
inline UINT32 pci_get_rx_resource_pending_num(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	UINT32 num;
	ULONG flags;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = pci_get_rx_ring_by_ridx(hif, resource_idx);
	NDIS_SPIN_LOCK *lock = &rx_ring->ring_lock;

	RTMP_IRQ_LOCK(lock, flags);
	num = ((rx_ring->RxDmaIdx > rx_ring->RxSwReadIdx) ?
	(rx_ring->RxDmaIdx - rx_ring->RxSwReadIdx) :
	(rx_ring->RxDmaIdx + rx_ring->ring_size - rx_ring->RxSwReadIdx));

	RTMP_IRQ_UNLOCK(lock, flags);
	return num;
}

/*
*
*/
inline VOID pci_inc_resource_full_cnt(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);

	tx_ring->tx_ring_full_cnt++;
}


inline VOID pci_dec_resource_full_cnt(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);

	tx_ring->tx_ring_full_cnt--;
}
/*
*
*/
inline BOOLEAN pci_get_resource_state(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);

	return test_bit(0, &tx_ring->tx_ring_state);
}

/*
*
*/
inline INT pci_set_resource_state(RTMP_ADAPTER *pAd, UINT8 resource_idx, BOOLEAN state)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);

	if (state == TX_RING_LOW)
		set_bit(0, &tx_ring->tx_ring_state);
	else
		clear_bit(0, &tx_ring->tx_ring_state);

	return NDIS_STATUS_SUCCESS;
}

UINT32 pci_dynamic_dly_int_adjust(VOID *hdev_ctrl, UINT32 tx_tp_mbps, UINT32 rx_tp_mbps)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(hdev_ctrl);
	UINT8 num_of_rx_ring = hif->rx_res_num;
	struct dly_ctl_cfg *cfg, *sel_cfg;
	struct hif_pci_rx_ring *rx_ring = NULL;
	UINT32 i, num;

	if ((tx_tp_mbps + rx_tp_mbps) == 0)
		return NDIS_STATUS_SUCCESS;

	/* WPDMA/WFDMA TX */

	/* WPDMA/WFDMA RX */
	for (num = 0; num < num_of_rx_ring; num++) {
		rx_ring = pci_get_rx_ring_by_ridx(hif, num);

		if (rx_ring->delay_int_en) {
			sel_cfg = NULL;
			if (((tx_tp_mbps * 100) / (tx_tp_mbps + rx_tp_mbps))
								> TX_MODE_RATIO_THRESHOLD) {
				for (i = 0; i < rx_ring->dl_dly_ctl_tbl_size; i++) {
					cfg = rx_ring->dl_dly_ctl_tbl + i;
					if (tx_tp_mbps > cfg->avg_tp) {
						sel_cfg = cfg;
					} else {
						break;
					}
				}
				if (sel_cfg)
					chip_cfg_dly_int(hdev_ctrl, rx_ring->hw_desc_base,
										sel_cfg->dly_number, sel_cfg->dly_time);
			} else if (((rx_tp_mbps * 100) / (tx_tp_mbps + rx_tp_mbps))
								> RX_MODE_RATIO_THRESHOLD) {
				for (i = 0; i < rx_ring->ul_dly_ctl_tbl_size; i++) {
					cfg = rx_ring->ul_dly_ctl_tbl + i;
					if (rx_tp_mbps > cfg->avg_tp) {
						sel_cfg = cfg;
					} else {
						break;
					}
				}
				if (sel_cfg)
					chip_cfg_dly_int(hdev_ctrl, rx_ring->hw_desc_base,
										sel_cfg->dly_number, sel_cfg->dly_time);
			}
		}
	}

	return NDIS_STATUS_RESOURCES;
}
#ifdef RX_RPS_SUPPORT
VOID change_rx_tasklet_method(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	pAd->rx_dequeue_sw_rps_enable = enable;
	cap->rx_qm_en = enable;

	if (pAd->rx_dequeue_sw_rps_enable)
		pci_hif->dma_done_handle[HIF_RX_DATA] = pci_rx_dma_done_rxq_handle;
	else
		pci_hif->dma_done_handle[HIF_RX_DATA] = pci_rx_dma_done_handle;

	if ((pAd->mcli_ctl[DBDC_BAND0].debug_on & MCLI_DEBUG_RPS_CFG_MODE)
#ifdef DBDC_MODE
		|| (pAd->mcli_ctl[DBDC_BAND1].debug_on & MCLI_DEBUG_RPS_CFG_MODE)
#endif
		)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("change rx_qm enable:%u\n", cap->rx_qm_en));


}

VOID change_rx_qm_cpumap(RTMP_ADAPTER *pAd, UINT32 mask)
{
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	int i;

	chip_cap->RxSwRpsNum = 0;
	chip_cap->RxSwRpsCpu = mask;

	for (i = 0; i < NR_CPUS; i++)
		chip_cap->RxSwRpsCpuMap[i] = NR_CPUS;

	for (i = 0; i < NR_CPUS; i++) {
		if ((1 << i) & chip_cap->RxSwRpsCpu) {
			chip_cap->RxSwRpsCpuMap[chip_cap->RxSwRpsNum] = i;
			chip_cap->RxSwRpsNum++;
		}
	}

	if ((pAd->mcli_ctl[DBDC_BAND0].debug_on & MCLI_DEBUG_RPS_CFG_MODE)
#ifdef DBDC_MODE
	|| (pAd->mcli_ctl[DBDC_BAND1].debug_on & MCLI_DEBUG_RPS_CFG_MODE)
#endif
		) {
		if (NR_CPUS == 4)
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cpumap[%d %d %d %d],num:%d\n",
				chip_cap->RxSwRpsCpuMap[3],
				chip_cap->RxSwRpsCpuMap[2],
				chip_cap->RxSwRpsCpuMap[1],
				chip_cap->RxSwRpsCpuMap[0],
				chip_cap->RxSwRpsNum));
	}
}

#endif
/*
*
*/
NDIS_STATUS pci_hif_chip_init(VOID **hif, struct pci_hif_chip_cfg *cfg)
{
	struct pci_hif_chip *chip_hif;
	if (hif_ctrl_init((VOID **)&chip_hif, RTMP_DEV_INF_PCIE) != NDIS_STATUS_SUCCESS)
		goto err;

	chip_hif->CSRBaseAddress = (PUCHAR) cfg->csr_addr;
	hif_chip_init(chip_hif, cfg->device_id);
	chip_hif->pdev = cfg->device;
#ifdef CONFIG_WIFI_MSI_SUPPORT
	chip_hif->is_msi = cfg->msi_en;
#endif
	chip_hif->irq = cfg->irq;

	if (pci_data_init(chip_hif) != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Hif data struct init fail \n"));
		goto err_data;
	}

	if (pci_alloc_tx_rx_ring_mem(chip_hif) != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Failed to allocate memory - TxRxRing\n"));
		goto err_txrx;
	}

	*hif = chip_hif;
	return NDIS_STATUS_SUCCESS;
err_txrx:
	pci_data_exit(chip_hif);
err_data:
	hif_ctrl_exit(chip_hif);
err:
	return NDIS_STATUS_RESOURCES;
}

/*
*
*/
VOID pci_hif_chip_exit(struct pci_hif_chip *chip_hif)
{
	pci_free_txrx_ring_mem(chip_hif);
	pci_data_exit(chip_hif);
	hif_ctrl_exit(chip_hif);
}

