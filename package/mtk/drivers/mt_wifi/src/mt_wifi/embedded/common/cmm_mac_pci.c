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
 ***************************************************************************/



#ifdef RTMP_MAC_PCI
#include	"rt_config.h"

#ifdef CONFIG_WIFI_MSI_SUPPORT
#ifdef MT7916
#include "chip/mt7916_cr.h"
#endif
#endif

#if (KERNEL_VERSION(5, 4, 0) < LINUX_VERSION_CODE)
#define mmiowb()		do { } while (0)
#endif
/*
*
*/
VOID dump_txd(RTMP_ADAPTER *pAd, TXD_STRUC *pTxD)
{
	MTWF_PRINT("TxD:\n");
	MTWF_PRINT("\tSDPtr0=0x%x\n", pTxD->SDPtr0);
	MTWF_PRINT("\tSDLen0=0x%x\n", pTxD->SDLen0);
	MTWF_PRINT("\tLastSec0=0x%x\n", pTxD->LastSec0);
	MTWF_PRINT("\tSDPtr1=0x%x\n", pTxD->SDPtr1);
	MTWF_PRINT("\tSDLen1=0x%x\n", pTxD->SDLen1);
	MTWF_PRINT("\tLastSec1=0x%x\n", pTxD->LastSec1);
	MTWF_PRINT("\tDMADONE=0x%x\n", pTxD->DMADONE);
	MTWF_PRINT("\tBurst=0x%x\n", pTxD->Burst);
}

/***************************************************************************
  *
  *	register related procedures.
  *
  **************************************************************************/

#ifdef CONFIG_STA_SUPPORT

VOID RT28xxPciStaAsicWakeup(RTMP_ADAPTER *pAd, BOOLEAN bFromTx, PSTA_ADMIN_CONFIG pStaCfg)
{
	if (pStaCfg)
		RTMPOffloadPm(pAd, pStaCfg, PM4, EXIT_PM_STATE);
}


VOID RT28xxPciStaAsicSleepAutoWakeup(
	RTMP_ADAPTER *pAd,
	PSTA_ADMIN_CONFIG pStaCfg)
{
	if (pStaCfg)
		RTMPOffloadPm(pAd, pStaCfg, PM4, ENTER_PM_STATE);
}

#endif /* CONFIG_STA_SUPPORT */


/*
	==========================================================================
	Description:
		This routine sends command to firmware and turn our chip to wake up mode from power save mode.
		Both RadioOn and .11 power save function needs to call this routine.
	Input:
		Level = GUIRADIO_OFF : call this function is from Radio Off to Radio On.  Need to restore PCI host value.
		Level = other value : normal wake up function.

	==========================================================================
 */
BOOLEAN RT28xxPciAsicRadioOn(RTMP_ADAPTER *pAd, UCHAR Level)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	if (pAd->OpMode == OPMODE_AP && Level == DOT11POWERSAVE)
		return FALSE;

	/* 2. Send wake up command.*/
	AsicSendCommandToMcu(pAd, 0x31, PowerWakeCID, 0x00, 0x02, FALSE);
	pci_hif->bPCIclkOff = FALSE;
	chip_interrupt_enable(pAd);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);

	if (Level == GUI_IDLE_POWER_SAVE) {
		/*2009/06/09: AP and stations need call the following function*/
		/* this if defined is equivalent to ifndef RTMP_RBUS_SUPPORT */
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT) || defined(RTMP_SDIO_SUPPORT)
		/* add by johnli, RF power sequence setup, load RF normal operation-mode setup*/
		RTMP_CHIP_OP *pChipOps = hc_get_chip_ops(pAd->hdev_ctrl);

		if (!IS_RBUS_INF(pAd) && pChipOps->AsicReverseRfFromSleepMode)
			pChipOps->AsicReverseRfFromSleepMode(pAd, FALSE);
		else
#endif /* defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT) || defined(RTMP_SDIO_SUPPORT) */
		{
			/* In Radio Off, we turn off RF clk, So now need to call ASICSwitchChannel again.*/
			hc_reset_radio(pAd);
		}
	}

	return TRUE;
}


/*
	==========================================================================
	Description:
		This routine sends command to firmware and turn our chip to power save mode.
		Both RadioOff and .11 power save function needs to call this routine.
	Input:
		Level = GUIRADIO_OFF  : GUI Radio Off mode
		Level = DOT11POWERSAVE  : 802.11 power save mode
		Level = RTMP_HALT  : When Disable device.

	==========================================================================
 */
BOOLEAN RT28xxPciAsicRadioOff(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Level,
	IN USHORT TbttNumToNextWakeUp)
{
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "Not support for HIF_MT yet!\n");
		return FALSE;
	}

	return TRUE;
}


/*
========================================================================
Routine Description:
	Get a pci map buffer.

Arguments:
	pAd				- WLAN control block pointer
	*ptr			- Virtual address or TX control block
	size			- buffer size
	sd_idx			- 1: the ptr is TX control block
	direction		- RTMP_PCI_DMA_TODEVICE or RTMP_PCI_DMA_FROMDEVICE

Return Value:
	the PCI map buffer

Note:
========================================================================
*/
ra_dma_addr_t RtmpDrvPciMapSingle(
	IN RTMP_ADAPTER *pAd,
	IN VOID *ptr,
	IN size_t size,
	IN INT sd_idx,
	IN INT direction)
{
	ra_dma_addr_t SrcBufPA = 0;
	struct device *pdev  = ((POS_COOKIE)(pAd->OS_Cookie))->pDev;
	UCHAR ret = 0;

	if (sd_idx == 1) {
		TX_BLK *pTxBlk = (TX_BLK *)(ptr);

		if (pTxBlk->SrcBufLen) {
			SrcBufPA = PCI_MAP_SINGLE_DEV(pAd, pTxBlk->pSrcBufData, pTxBlk->SrcBufLen, 0, direction);
			pTxBlk->SrcBufPA = SrcBufPA;
		 } else {
			return (ra_dma_addr_t)0x0;
		}
	} else
		SrcBufPA = PCI_MAP_SINGLE_DEV(pAd, ptr, size, 0, direction);

	ret = dma_mapping_error(pdev, SrcBufPA);

	if (ret) {
		MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "dma mapping error,ret=%d\n", ret);
		return (ra_dma_addr_t)0x0;
	} else
		return SrcBufPA;
}


int write_reg(RTMP_ADAPTER *ad, UINT32 base, UINT16 offset, UINT32 value)
{
	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(ad, HIF_MT)) {
		MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "Not support for HIF_MT yet!\n");
		return FALSE;
	}

	if (base == 0x40)
		RTMP_IO_WRITE32(ad->hdev_ctrl, 0x10000 + offset, value);
	else if (base == 0x41)
		RTMP_IO_WRITE32(ad->hdev_ctrl, offset, value);
	else
		MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "illegal base = %x\n", base);

	return 0;
}


int read_reg(RTMP_ADAPTER *ad, UINT32 base, UINT16 offset, UINT32 *value)
{
	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(ad, HIF_MT)) {
		MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "Not support for HIF_MT yet!\n");
		return FALSE;
	}

	if (base == 0x40)
		RTMP_IO_READ32(ad->hdev_ctrl, 0x10000 + offset, value);
	else if (base == 0x41)
		RTMP_IO_READ32(ad->hdev_ctrl, offset, value);
	else
		MTWF_DBG(ad, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "illegal base = %x\n", base);

	return 0;
}

#ifdef CUT_THROUGH
INT mt_ct_check_hw_resource(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);
	PKT_TOKEN_CB *cb = hif->PktTokenCb;
	UINT8 band_idx = HcGetBandByWdev(wdev);
	struct token_tx_pkt_queue *que = NULL;

	que = token_tx_get_queue_by_band(cb, band_idx);

	if (pci_get_tx_resource_free_num_nolock(pAd, resource_idx)
					< tx_ring->tx_ring_low_water_mark) {
		pci_inc_resource_full_cnt(pAd, resource_idx);
		pci_set_resource_state(pAd, resource_idx, TX_RING_LOW);
		return ERROR_NO_RING;
	}

	if (token_tx_get_free_cnt(que)
					<  token_tx_get_lwmark(que)) {
		token_tx_inc_full_cnt(que);
		token_tx_set_state(que, TX_TOKEN_LOW);
		return ERROR_NO_TOKEN;
	}

	if (token_tx_get_used_cnt_per_band(que, band_idx)
					>  token_tx_get_hwmark_per_band(que, band_idx)) {
		token_tx_inc_full_cnt_per_band(que, band_idx);
		return ERROR_NO_TOKEN;
	}

	return NDIS_STATUS_SUCCESS;
}

INT32 mt_ct_get_hw_resource_state(RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
									UINT32 pkt_type, UCHAR q_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	UCHAR resource_idx = hif_get_resource_idx(pAd->hdev_ctrl, wdev, pkt_type, q_idx);
	PKT_TOKEN_CB *cb = hif->PktTokenCb;
	UINT8 band_idx = HcGetBandByWdev(wdev);
	struct token_tx_pkt_queue *que = NULL;

	que = token_tx_get_queue_by_band(cb, band_idx);

	return (pci_get_resource_state(pAd, resource_idx)
				|| token_tx_get_state(que));
}

INT mt_ct_hw_tx(RTMP_ADAPTER *pAd, TX_BLK *tx_blk)
{
	USHORT free_cnt = 1;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (!TX_BLK_TEST_FLAG(tx_blk, fTX_MCU_OFFLOAD))
		tx_bytes_calculate(pAd, tx_blk);

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS)) {

		if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
			(tx_blk->amsdu_state == TX_AMSDU_ID_LAST))
			asic_write_tmac_info(pAd, &tx_blk->HeaderBuf[0], tx_blk);

		ret = asic_write_txp_info(pAd, &tx_blk->HeaderBuf[cap->tx_hw_hdr_len], tx_blk);

		if (ret != NDIS_STATUS_SUCCESS)
			return ret;

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
		/* if needr_to_drop_counter > 0 then skip to update PDMA descritpor */
		if (pAd->vow_need_drop_cnt[tx_blk->Wcid]) {
			return ret;
		}
#endif /* #if defined(VOW_SUPPORT) && defined(VOW_DVT) */

		if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
			(tx_blk->amsdu_state == TX_AMSDU_ID_LAST))
			asic_write_tx_resource(pAd, tx_blk, TRUE, &free_cnt);
	} else {
		INT dot11_meta_hdr_len, tx_hw_hdr_len;
		PNDIS_PACKET pkt;
		NDIS_STATUS status;
		UCHAR *dot11_hdr;
		TX_BLK new_tx_blk, *p_new_tx_blk = &new_tx_blk;

		NdisCopyMemory(p_new_tx_blk, tx_blk, sizeof(*tx_blk));
#ifdef CONFIG_ATE
		if (ATE_ON(pAd))
			dot11_meta_hdr_len = 0;
		else
#endif
			dot11_meta_hdr_len = tx_blk->MpduHeaderLen + tx_blk->HdrPadLen;
		tx_hw_hdr_len = cap->tx_hw_hdr_len;
		dot11_hdr = &tx_blk->HeaderBuf[tx_hw_hdr_len];
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "%s():DataFrm, MpduHdrL=%d,WFHdrL=%d,HdrPadL=%d,HwRsvL=%d, NeedCopyHdrLen=%d\n",
				  __func__, tx_blk->MpduHeaderLen, tx_blk->wifi_hdr_len, tx_blk->HdrPadLen,
				  tx_blk->hw_rsv_len, dot11_meta_hdr_len);
		/* original packet only 802.3 payload, so create a new packet including 802.11 header for cut-through transfer */
		status = RTMPAllocateNdisPacket(pAd, &pkt,
										dot11_hdr, dot11_meta_hdr_len,
										tx_blk->pSrcBufData, tx_blk->SrcBufLen);

		if (status != NDIS_STATUS_SUCCESS)
			return NDIS_STATUS_FAILURE;

		NdisCopyMemory(GET_OS_PKT_CB(pkt),  GET_OS_PKT_CB(tx_blk->pPacket),
			sizeof(GET_OS_PKT_CB(tx_blk->pPacket)));

		RTMP_SET_PACKET_WDEV(pkt, RTMP_GET_PACKET_WDEV(tx_blk->pPacket));
		if ((tx_blk->FragIdx == TX_FRAG_ID_NO) || (tx_blk->FragIdx == TX_FRAG_ID_LAST))
			RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_SUCCESS);

		p_new_tx_blk->HeaderBuf = hif_get_tx_buf(pAd->hdev_ctrl, p_new_tx_blk, tx_blk->resource_idx, tx_blk->TxFrameType);
		if (p_new_tx_blk->HeaderBuf == NULL) {
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
			return NDIS_STATUS_FAILURE;
		}
		p_new_tx_blk->pPacket = pkt;
		p_new_tx_blk->pSrcBufData = GET_OS_PKT_DATAPTR(pkt);
		p_new_tx_blk->SrcBufLen = GET_OS_PKT_LEN(pkt);
		TX_BLK_SET_FLAG(p_new_tx_blk, fTX_CT_WithTxD);

		if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
			(tx_blk->amsdu_state == TX_AMSDU_ID_LAST))
			asic_write_tmac_info(pAd, &p_new_tx_blk->HeaderBuf[0], p_new_tx_blk);

		/* because 802.11 header already in new packet, not in HeaderBuf, so set below parameters to 0 */
		p_new_tx_blk->MpduHeaderLen = 0;
		p_new_tx_blk->wifi_hdr_len = 0;
		p_new_tx_blk->HdrPadLen = 0;
		p_new_tx_blk->hw_rsv_len = 0;

		tx_blk->resource_idx = p_new_tx_blk->resource_idx;
		ret = asic_write_txp_info(pAd, &p_new_tx_blk->HeaderBuf[cap->tx_hw_hdr_len], p_new_tx_blk);

		if (ret != NDIS_STATUS_SUCCESS)
			return ret;

		if ((tx_blk->amsdu_state == TX_AMSDU_ID_NO) ||
			(tx_blk->amsdu_state == TX_AMSDU_ID_LAST))
			asic_write_tx_resource(pAd, p_new_tx_blk, TRUE, &free_cnt);
	}

	return NDIS_STATUS_SUCCESS;
}

INT mt_ct_mlme_hw_tx(RTMP_ADAPTER *pAd, UCHAR *tmac_info, MAC_TX_INFO *info, HTTRANSMIT_SETTING *transmit, TX_BLK *tx_blk)
{
	UINT16 free_cnt = 1;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef RT_BIG_ENDIAN
	PHEADER_802_11 pHeader_802_11;
	pHeader_802_11 = (HEADER_802_11 *)(tx_blk->pSrcBufHeader + cap->tx_hw_hdr_len);
#endif
	asic_write_tmac_info_fixed_rate(pAd, tmac_info, info, transmit);
#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pHeader_802_11, DIR_WRITE, FALSE);
	MTMacInfoEndianChange(pAd, tmac_info, TYPE_TMACINFO, sizeof(TMAC_TXD_L));
#endif
	tx_blk->pSrcBufData = tx_blk->pSrcBufHeader;
	NdisCopyMemory(&tx_blk->HeaderBuf[0], tx_blk->pSrcBufData, cap->tx_hw_hdr_len);
	tx_blk->pSrcBufData += cap->tx_hw_hdr_len;
	tx_blk->SrcBufLen -= cap->tx_hw_hdr_len;
	tx_blk->MpduHeaderLen = 0;
	tx_blk->wifi_hdr_len = 0;
	tx_blk->HdrPadLen = 0;
	tx_blk->hw_rsv_len = 0;

	ret = asic_write_txp_info(pAd, &tx_blk->HeaderBuf[cap->tx_hw_hdr_len], tx_blk);

	if (ret != NDIS_STATUS_SUCCESS)
		return ret;

	asic_write_tx_resource(pAd, tx_blk, TRUE, &free_cnt);
	return NDIS_STATUS_SUCCESS;
}

#endif /*CUT_THROUGH*/

/*
*
*/
static USHORT mt_pci_get_buf_len(RTMP_ADAPTER *pAd, TX_BLK *tx_blk)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT tx_hw_hdr_len = cap->tx_hw_hdr_len;

	return tx_hw_hdr_len + tx_blk->txp_len + tx_blk->MpduHeaderLen + tx_blk->HdrPadLen - tx_blk->hw_rsv_len;
}

/*
*
*/
static USHORT write_first_buf(RTMP_ADAPTER *pAd, TX_BLK *tx_blk, UCHAR *dma_buf)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT tx_hw_hdr_len = cap->tx_hw_hdr_len;
	USHORT first_buf_len;

	first_buf_len =  tx_hw_hdr_len + tx_blk->txp_len + tx_blk->MpduHeaderLen + tx_blk->HdrPadLen - tx_blk->hw_rsv_len;
	NdisMoveMemory(dma_buf, (UCHAR *)(tx_blk->HeaderBuf + tx_blk->hw_rsv_len), first_buf_len);
	return first_buf_len;
}
#ifdef MT_MAC
/*
*
*/
USHORT mt_pci_write_tx_resource(
	RTMP_ADAPTER *pAd,
	TX_BLK *pTxBlk,
	BOOLEAN bIsLast,
	USHORT *FreeNumber)
{
	UCHAR *pDMAHeaderBufVA;
	UINT32 BufBasePaLow;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	UINT16 TmacLen;
	UINT8 *temp;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	struct hif_pci_tx_ring *tx_ring;

	/* get Tx Ring Resource*/
	tx_ring = pci_get_tx_ring_by_ridx(hif, pTxBlk->resource_idx);
	TxIdx = tx_ring->TxCpuIdx;

	pDMAHeaderBufVA = (UCHAR *)tx_ring->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(tx_ring->Cell[TxIdx].DmaBuf.AllocPa);
	tx_ring->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
	tx_ring->Cell[TxIdx].pNextNdisPacket = NULL;
	tx_ring->Cell[TxIdx].PacketPa = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
	/* build Tx Descriptor*/
#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *)tx_ring->Cell[TxIdx].AllocVa;
#else
	pDestTxD = (TXD_STRUC *)tx_ring->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#endif
	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = write_first_buf(pAd, pTxBlk, pDMAHeaderBufVA);
	pTxD->SDPtr1 = tx_ring->Cell[TxIdx].PacketPa;
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (bIsLast && pTxD->SDLen1) ? 1 : 0;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;
	wmb();
#ifdef RT_BIG_ENDIAN
	TmacLen = (cap->tx_hw_hdr_len - pTxBlk->hw_rsv_len);
	MTMacInfoEndianChange(pAd,  (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, TmacLen);
	RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TmacLen), DIR_WRITE, FALSE);
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
	RetTxIdx = TxIdx;
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(tx_ring->Cell[TxIdx].DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	RTMP_DCACHE_FLUSH(tx_ring->Cell[TxIdx].AllocPa, RXD_SIZE);
	/* Update Tx index*/
	tx_ring->TxCpuIdx = TxIdx;
	*FreeNumber -= 1;
	return RetTxIdx;
}

/*
*
*/
USHORT mt_pci_write_multi_tx_resource(
	RTMP_ADAPTER *pAd,
	TX_BLK *pTxBlk,
	UCHAR frameNum,
	USHORT *FreeNumber)
{
	BOOLEAN bIsLast;
	UCHAR *pDMAHeaderBufVA;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	UINT16 TmacLen;
#endif
	UINT32 BufBasePaLow;
	struct hif_pci_tx_ring *tx_ring;
	UINT32 firstDMALen = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	ASSERT((pTxBlk->TxFrameType == TX_AMSDU_FRAME));
	bIsLast = ((frameNum == (pTxBlk->TotalFrameNum - 1)) ? 1 : 0);
	/* get Tx Ring Resource */
	tx_ring = pci_get_tx_ring_by_ridx(hif, pTxBlk->resource_idx);
	tx_ring_size = tx_ring->ring_size;
	TxIdx = tx_ring->TxCpuIdx;
	pDMAHeaderBufVA = (PUCHAR) tx_ring->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(tx_ring->Cell[TxIdx].DmaBuf.AllocPa);

	if (frameNum == 0) {
		/* copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer */
		firstDMALen =  cap->tx_hw_hdr_len - pTxBlk->hw_rsv_len + pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;
		NdisMoveMemory(pDMAHeaderBufVA, pTxBlk->HeaderBuf + pTxBlk->hw_rsv_len, firstDMALen);
	} else {
		firstDMALen = pTxBlk->MpduHeaderLen;
		NdisMoveMemory(pDMAHeaderBufVA, pTxBlk->HeaderBuf, firstDMALen);
	}

	tx_ring->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
	tx_ring->Cell[TxIdx].pNextNdisPacket = NULL;
	tx_ring->Cell[TxIdx].PacketPa = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
	/* build Tx Descriptor */
#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *) tx_ring->Cell[TxIdx].AllocVa;
#else
	pDestTxD = (TXD_STRUC *) tx_ring->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#endif
	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = firstDMALen; /* include padding*/
	pTxD->SDPtr1 = tx_ring->Cell[TxIdx].PacketPa;
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (bIsLast && pTxD->SDLen1) ? 1 : 0;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;
	wmb();
#ifdef RT_BIG_ENDIAN
	TmacLen = (cap->tx_hw_hdr_len - pTxBlk->hw_rsv_len);

	if (frameNum == 0)
		RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TmacLen), DIR_WRITE, FALSE);

	if (frameNum != 0)
		MTMacInfoEndianChange(pAd,  (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, TmacLen);

	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
	RetTxIdx = TxIdx;
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(tx_ring->Cell[TxIdx].DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	RTMP_DCACHE_FLUSH(tx_ring->Cell[TxIdx].AllocPa, RXD_SIZE);
	/* Update Tx index*/
	INC_RING_INDEX(TxIdx, tx_ring_size);
	tx_ring->TxCpuIdx = TxIdx;
	*FreeNumber -= 1;
	return RetTxIdx;
}

/*
*
*/
VOID mt_pci_write_final_tx_resource(
	RTMP_ADAPTER *pAd,
	TX_BLK *pTxBlk,
	USHORT totalMPDUSize,
	USHORT FirstTxIdx)
{
	struct hif_pci_tx_ring *tx_ring;
	UCHAR *tmac_info;
	TMAC_TXD_S *txd_s;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	UINT16 tx_ring_size;
	/* get Tx Ring Resource*/
	tx_ring = pci_get_tx_ring_by_ridx(hif, pTxBlk->resource_idx);
	tx_ring_size = tx_ring->ring_size;

	if (FirstTxIdx >= tx_ring_size)
		return;

	tmac_info = (UCHAR *)tx_ring->Cell[FirstTxIdx].DmaBuf.AllocVa;
	txd_s = (TMAC_TXD_S *)tmac_info;
	txd_s->TxD0.TxByteCount = totalMPDUSize;
#ifdef RT_BIG_ENDIAN
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16  TmacLen = (cap->tx_hw_hdr_len - pTxBlk->hw_rsv_len);

	MTMacInfoEndianChange(pAd, tmac_info, TYPE_TMACINFO, TmacLen);
#endif /* RT_BIG_ENDIAN */
}

static INT mt_asic_cfg_hif_tx_ring(RTMP_ADAPTER *pAd, struct hif_pci_tx_ring *ring, UINT32 phy_addr, UINT32 cnt)
{
	ring->TxSwFreeIdx = 0;
	ring->TxCpuIdx = 0;
	ring->hw_cnt_addr = ring->hw_desc_base + 0x04;
	ring->hw_cidx_addr = ring->hw_desc_base + 0x08;
	ring->hw_didx_addr = ring->hw_desc_base + 0x0c;
	HIF_IO_WRITE32(pAd->hdev_ctrl, ring->hw_desc_base, phy_addr);
	HIF_IO_WRITE32(pAd->hdev_ctrl, ring->hw_cidx_addr, ring->TxCpuIdx);
	HIF_IO_WRITE32(pAd->hdev_ctrl, ring->hw_cnt_addr, ring->ring_size);
	return TRUE;
}


VOID mt_asic_init_txrx_ring(RTMP_ADAPTER *pAd)
{
	UINT32 phy_addr, offset;
	INT i, TxHwRingNum;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	/* Set DMA global configuration except TX_DMA_EN and RX_DMA_EN bits */
	chip_set_hif_dma(pAd, DMA_TX_RX, FALSE);
	chip_wait_hif_dma_idle(pAd, ALL_DMA, 100, 1000);

	TxHwRingNum = hif->tx_res_num;

	for (i = 0; i < TxHwRingNum; i++) {
		struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, i);

		offset = tx_ring->hw_desc_base;
		phy_addr = RTMP_GetPhysicalAddressLow(tx_ring->Cell[0].AllocPa);
		mt_asic_cfg_hif_tx_ring(pAd, tx_ring, phy_addr, tx_ring->ring_size);
	}

	/* Init RX Ring0 Base/Size/Index pointer CSR */
	for (i = 0; i < hif->rx_res_num; i++) {
		struct hif_pci_rx_ring *rx_ring;
		UINT16 RxRingSize;

		rx_ring = pci_get_rx_ring_by_ridx(hif, i);
		RxRingSize = rx_ring->ring_size;
		offset = i * 0x10;
		phy_addr = RTMP_GetPhysicalAddressLow(rx_ring->Cell[0].AllocPa);
		rx_ring->RxSwReadIdx = 0;
		rx_ring->RxCpuIdx = RxRingSize - 1;
		rx_ring->hw_cidx_addr = rx_ring->hw_desc_base + 0x08;
		rx_ring->hw_didx_addr = rx_ring->hw_desc_base + 0x0c;
		rx_ring->hw_cnt_addr = rx_ring->hw_desc_base + 0x04;
		HIF_IO_WRITE32(pAd->hdev_ctrl, rx_ring->hw_desc_base, phy_addr);
		HIF_IO_WRITE32(pAd->hdev_ctrl, rx_ring->hw_cidx_addr, rx_ring->RxCpuIdx);
		HIF_IO_WRITE32(pAd->hdev_ctrl, rx_ring->hw_cnt_addr, RxRingSize);
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->RX_RING%d[0x%x]: Base=0x%x, Cnt=%d\n",
				 i, rx_ring->hw_desc_base, phy_addr, RxRingSize);
	}
}

#endif /*MT_MAC*/

#ifdef MT_DMAC

/*
*
*/
USHORT mtd_pci_write_tx_resource(
	RTMP_ADAPTER *pAd,
	TX_BLK *pTxBlk,
	BOOLEAN bIsLast,
	USHORT *FreeNumber)
{
	UCHAR *pDMAHeaderBufVA;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
	RTMP_DMACB *dma_cb;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	UINT16 TmacLen;
	UINT8 *temp;
#endif
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring;
#ifdef CTXD_SCATTER_AND_GATHER
	UINT8 max_ctxd_agg_num = hif->main_hif_chip->max_ctxd_agg_num;
#endif

	tx_ring = pci_get_tx_ring_by_ridx(hif, pTxBlk->resource_idx);
	tx_ring_size = tx_ring->ring_size;
	TxIdx = tx_ring->TxCpuIdx;
	dma_cb = &tx_ring->Cell[TxIdx];
	pDMAHeaderBufVA = (UCHAR *)dma_cb->DmaBuf.AllocVa;
	dma_cb->pNextNdisPacket = NULL;
	dma_cb->PacketPa = pTxBlk->SrcBufPA;

	if (pTxBlk->TxFrameType != TX_FRAG_FRAME)
		dma_cb->DmaBuf.AllocSize = mt_pci_get_buf_len(pAd, pTxBlk);
	else
		dma_cb->DmaBuf.AllocSize = write_first_buf(pAd, pTxBlk, pDMAHeaderBufVA);

#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *)dma_cb->AllocVa;
#else
	pDestTxD = (TXD_STRUC *)dma_cb->AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
	TmacLen = (cap->tx_hw_hdr_len - pTxBlk->hw_rsv_len);
#endif

	dma_cb->DmaBuf.AllocPa = PCI_MAP_SINGLE(pAd, pDMAHeaderBufVA, dma_cb->DmaBuf.AllocSize, 0, RTMP_PCI_DMA_TODEVICE);
	pTxD->SDPtr0 = dma_cb->DmaBuf.AllocPa;
	pTxD->SDLen0 = dma_cb->DmaBuf.AllocSize;
	pTxD->SDPtr1 = dma_cb->PacketPa;
	pTxD->SDLen1 = cap->CtParseLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
#ifdef CTXD_SCATTER_AND_GATHER
	if (hif->main_hif_chip->max_ctxd_agg_num > 1) {
		tx_ring->cur_txd_cnt = (tx_ring->cur_txd_cnt + 1) % max_ctxd_agg_num;
		if (tx_ring->cur_txd_cnt == 0) {
			pAd->tr_ctl.tr_cnt.ctxd_num[max_ctxd_agg_num - 1]++;
			pTxD->LastSec1 = 1;
		} else
			pTxD->LastSec1 = 0;
	} else
#endif
		pTxD->LastSec1 = (bIsLast && pTxD->SDLen1) ? 1 : 0;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;
	wmb();

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

	RetTxIdx = TxIdx;

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(dma_cb->DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	RTMP_DCACHE_FLUSH(dma_cb->AllocPa, RXD_SIZE);

	INC_RING_INDEX(TxIdx, tx_ring_size);
	tx_ring->TxCpuIdx = TxIdx;
	*FreeNumber -= 1;
	return RetTxIdx;
}

#ifdef CTXD_MEM_CPY
/*
*
*/
USHORT mtd_pci_write_tx_resource_for_ctxd(
	RTMP_ADAPTER *pAd,
	TX_BLK *pTxBlk,
	BOOLEAN bIsLast,
	USHORT *FreeNumber)
{
	UCHAR *pDMAHeaderBufVA;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
	RTMP_DMACB *dma_cb;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	UINT16 TmacLen;
	UINT8 *temp;
#endif
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 tx_ring_size;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct pci_hif_chip *hif_chip = hif->main_hif_chip;
	struct hif_pci_tx_ring *tx_ring;
	UINT32 offset = 0;

	tx_ring = pci_get_tx_ring_by_ridx(hif, pTxBlk->resource_idx);
	tx_ring_size = tx_ring->ring_size;
	TxIdx = tx_ring->TxCpuIdx;
	dma_cb = &tx_ring->Cell[TxIdx];
	pDMAHeaderBufVA = (UCHAR *)dma_cb->DmaBuf.AllocVa;
	dma_cb->pNextNdisPacket = NULL;

	if (pTxBlk->TxFrameType == TX_FRAG_FRAME) {
		offset = tx_ring->cur_txd_cnt * hif_chip->ctxd_size_unit;
		write_first_buf(pAd, pTxBlk, pDMAHeaderBufVA + offset);
	}

	offset = tx_ring->cur_txd_cnt * hif_chip->ctxd_size_unit + hif_chip->ct_partial_payload_offset;
	NdisCopyMemory(pDMAHeaderBufVA + offset, pTxBlk->pSrcBufData, (pTxBlk->SrcBufLen > cap->CtParseLen) ? cap->CtParseLen : pTxBlk->SrcBufLen);
	tx_ring->cur_txd_cnt++;

#ifdef PKTLOSS_CHK
	if (pAd->pktloss_chk.enable)
		pAd->pktloss_chk.pktloss_chk_handler(pAd, GET_OS_PKT_DATAPTR(pTxBlk->pPacket), MAT_ETHER_HDR_LEN, 1, FALSE);
#endif

	MEM_DBG_PKT_RECORD(pTxBlk->pPacket, 1<<6);

	if (tx_ring->cur_txd_cnt != hif_chip->max_ctxd_agg_num)
		return TxIdx;

#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *)dma_cb->AllocVa;
#else
	pDestTxD = (TXD_STRUC *)dma_cb->AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#endif

	dma_cb->DmaBuf.AllocSize = tx_ring->cur_txd_cnt * hif_chip->ctxd_size_unit;

	dma_cb->DmaBuf.AllocPa = PCI_MAP_SINGLE(pAd, pDMAHeaderBufVA, dma_cb->DmaBuf.AllocSize, 0, RTMP_PCI_DMA_TODEVICE);
	pTxD->SDPtr0 = dma_cb->DmaBuf.AllocPa;
	pTxD->SDLen0 = dma_cb->DmaBuf.AllocSize;
	pTxD->SDPtr1 = 0;
	pTxD->SDLen1 = 0;
	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;
	wmb();

	tx_ring->cur_txd_cnt = 0;

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

	RetTxIdx = TxIdx;

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(dma_cb->DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	RTMP_DCACHE_FLUSH(dma_cb->AllocPa, RXD_SIZE);

	INC_RING_INDEX(TxIdx, tx_ring_size);
	tx_ring->TxCpuIdx = TxIdx;
	*FreeNumber -= 1;
	pAd->tr_ctl.tr_cnt.ctxd_num[hif_chip->max_ctxd_agg_num - 1]++;
	return RetTxIdx;
}

/*
*
*/
VOID mtd_pci_write_last_tx_resource(
	RTMP_ADAPTER *pAd,
	UCHAR resource_idx)
{
	UCHAR *pDMAHeaderBufVA;
	USHORT TxIdx;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif

	RTMP_DMACB *dma_cb;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct pci_hif_chip *hif_chip = hif->main_hif_chip;
	struct hif_pci_tx_ring *tx_ring;
	UINT16 tx_ring_size;

	tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);

	if (tx_ring->cur_txd_cnt == 0)
		return;
	tx_ring_size = tx_ring->ring_size;

	TxIdx = tx_ring->TxCpuIdx;
	dma_cb = &tx_ring->Cell[TxIdx];
	pDMAHeaderBufVA = (UCHAR *)dma_cb->DmaBuf.AllocVa;

#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *)dma_cb->AllocVa;
#else
	pDestTxD = (TXD_STRUC *)dma_cb->AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#endif

	dma_cb->DmaBuf.AllocSize = tx_ring->cur_txd_cnt * hif_chip->ctxd_size_unit;

	dma_cb->DmaBuf.AllocPa = PCI_MAP_SINGLE(pAd, pDMAHeaderBufVA, dma_cb->DmaBuf.AllocSize, 0, RTMP_PCI_DMA_TODEVICE);
	pTxD->SDPtr0 = dma_cb->DmaBuf.AllocPa;
	pTxD->SDLen0 = dma_cb->DmaBuf.AllocSize;
	pTxD->SDPtr1 = 0;
	pTxD->SDLen1 = 0;
	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;
	wmb();
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

	pAd->tr_ctl.tr_cnt.ctxd_num[tx_ring->cur_txd_cnt - 1]++;

	tx_ring->cur_txd_cnt = 0;


	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(dma_cb->DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(dma_cb->AllocPa, RXD_SIZE);

	INC_RING_INDEX(TxIdx, tx_ring_size);
	tx_ring->TxCpuIdx = TxIdx;
}
#endif


/*
*
*/
#ifdef CTXD_SCATTER_AND_GATHER
VOID mtd_pci_write_last_tx_resource_last_sec(
	RTMP_ADAPTER *pAd,
	UCHAR resource_idx)
{
	USHORT TxIdx;
	TXD_STRUC *pTxD;
	RTMP_DMACB *dma_cb;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring;

	tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);

	if (tx_ring->cur_txd_cnt == 0)
		return;

	TxIdx = tx_ring->TxCpuIdx - 1;
	if (TxIdx >= tx_ring->ring_size)
		TxIdx = tx_ring->ring_size - 1;
	dma_cb = &tx_ring->Cell[TxIdx];

	pTxD = (TXD_STRUC *)dma_cb->AllocVa;

	pTxD->LastSec1 = 1;

	pAd->tr_ctl.tr_cnt.ctxd_num[tx_ring->cur_txd_cnt - 1]++;

	tx_ring->cur_txd_cnt = 0;
}
#endif

/*
*
*/
VOID mt_int_disable(RTMP_ADAPTER *pAd, struct pci_hif_chip *hif_chip, unsigned int mode)
{
	UINT32 regValue, reg_addr;
#ifdef CONFIG_WIFI_MSI_SUPPORT
	UINT32 intDisableMask;
#endif

	hif_chip->intDisableMask |= mode;
	reg_addr = hif_chip->int_ena_reg_addr;
	regValue = hif_chip->int_enable_mask & ~(hif_chip->intDisableMask);
#ifdef CONFIG_WIFI_MSI_SUPPORT
	if (hif_chip->is_msi) {
		intDisableMask = hif_chip->int_enable_mask & hif_chip->intDisableMask;
		if (hif_chip->is_main)
			HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT_MASK_CSR_W1C_DMA0_PCIE0, intDisableMask);
		else
			HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT_MASK_CSR_W1C_DMA0_PCIE1, intDisableMask);
	} else
#endif
	HIF_IO_WRITE32(pAd->hdev_ctrl, reg_addr, regValue);
	mmiowb();

	if (regValue == 0)
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
}

/*
*
*/
VOID mt_int_enable(RTMP_ADAPTER *pAd, struct pci_hif_chip *hif_chip, unsigned int mode)
{
	UINT32 regValue, reg_addr;

	hif_chip->intDisableMask &= ~(mode);
	reg_addr = hif_chip->int_ena_reg_addr;
	regValue = hif_chip->int_enable_mask & ~(hif_chip->intDisableMask);
#ifdef CONFIG_WIFI_MSI_SUPPORT
	if (hif_chip->is_msi) {
		if (hif_chip->is_main)
			HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT_MASK_CSR_W1S_DMA0_PCIE0, regValue);
		else
			HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT_MASK_CSR_W1S_DMA0_PCIE1, regValue);
	} else
#endif
	HIF_IO_WRITE32(pAd->hdev_ctrl, reg_addr, regValue);
	mmiowb();

	if (regValue != 0)
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
}

static INT mtd_asic_cfg_hif_tx_ring(RTMP_ADAPTER *pAd, struct hif_pci_tx_ring *ring, UINT32 phy_addr, UINT32 cnt)
{
	UINT16 tx_ring_size = ring->ring_size;

	ring->TxSwFreeIdx = 0;
	ring->TxCpuIdx = 0;
	ring->hw_cnt_addr = ring->hw_desc_base + 0x04;
	ring->hw_cidx_addr = ring->hw_desc_base + 0x08;
	ring->hw_didx_addr = ring->hw_desc_base + 0x0c;
	HIF_IO_WRITE32(pAd->hdev_ctrl, ring->hw_desc_base, phy_addr);
	HIF_IO_WRITE32(pAd->hdev_ctrl, ring->hw_cidx_addr, ring->TxCpuIdx);
	HIF_IO_WRITE32(pAd->hdev_ctrl, ring->hw_cnt_addr, tx_ring_size);
	return TRUE;
}

/*
*
*/
VOID mtd_asic_init_txrx_ring(RTMP_ADAPTER *pAd)
{
	UINT32 phy_addr, offset;
	INT i, TxHwRingNum;
	UINT16 tx_ring_size = 0;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	/* Set DMA global configuration except TX_DMA_EN and RX_DMA_EN bits */
	chip_set_hif_dma(pAd, DMA_TX_RX, FALSE);
	chip_wait_hif_dma_idle(pAd, ALL_DMA, 100, 1000);

	TxHwRingNum = hif->tx_res_num;


	for (i = 0; i < TxHwRingNum; i++) {
		struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, i);

		offset = tx_ring->hw_desc_base;
		tx_ring_size = tx_ring->ring_size;
		phy_addr = RTMP_GetPhysicalAddressLow(tx_ring->Cell[0].AllocPa);
		mtd_asic_cfg_hif_tx_ring(pAd, tx_ring, phy_addr, tx_ring_size);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"-->TX_RING_%d[0x%x]: Attr:%d, Base=0x%x, Cnt=%d!\n",
				 i, tx_ring->hw_desc_base, tx_ring->ring_attr, phy_addr, tx_ring_size);
	}

	/* Init RX Ring0 Base/Size/Index pointer CSR */
	for (i = 0; i < hif->rx_res_num; i++) {
		UINT16 RxRingSize;
		struct hif_pci_rx_ring *rx_ring;

		rx_ring = pci_get_rx_ring_by_ridx(hif, i);
		RxRingSize = rx_ring->ring_size;
		offset = i * 0x10;
		phy_addr = RTMP_GetPhysicalAddressLow(rx_ring->Cell[0].AllocPa);
		rx_ring->RxSwReadIdx = 0;
		rx_ring->RxCpuIdx = RxRingSize - 1;
		rx_ring->hw_cidx_addr = rx_ring->hw_desc_base + 0x08;
		rx_ring->hw_didx_addr = rx_ring->hw_desc_base + 0x0c;
		rx_ring->hw_cnt_addr = rx_ring->hw_desc_base + 0x04;
		HIF_IO_WRITE32(pAd->hdev_ctrl, rx_ring->hw_desc_base, phy_addr);
		HIF_IO_WRITE32(pAd->hdev_ctrl, rx_ring->hw_cidx_addr, rx_ring->RxCpuIdx);
		HIF_IO_WRITE32(pAd->hdev_ctrl, rx_ring->hw_cnt_addr, RxRingSize);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "-->RX_RING_%d[0x%x]: Base=0x%x, Cnt=%d\n",
				 i, rx_ring->hw_desc_base, phy_addr, RxRingSize);
	}
}

#endif /*MT_DMAC*/

/*
*
*/
USHORT	pci_write_frag_tx_resource(
	RTMP_ADAPTER *pAd,
	TX_BLK *pTxBlk,
	UCHAR fragNum,
	USHORT *FreeNumber)
{
	UCHAR *pDMAHeaderBufVA;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
	RTMP_DMACB *dma_cb;
	UINT16 tx_ring_size;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	UINT16 TmacLen;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	struct hif_pci_tx_ring *tx_ring;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	tx_ring = pci_get_tx_ring_by_ridx(hif, pTxBlk->resource_idx);
	tx_ring_size = tx_ring->ring_size;
	TxIdx = tx_ring->TxCpuIdx;
	dma_cb = &tx_ring->Cell[TxIdx];
	pDMAHeaderBufVA = (PUCHAR)dma_cb->DmaBuf.AllocVa;
#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *)dma_cb->AllocVa;
#else
	pDestTxD = (TXD_STRUC *)dma_cb->AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
	TmacLen = (cap->tx_hw_hdr_len - pTxBlk->hw_rsv_len);
	MTMacInfoEndianChange(pAd,  (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, TmacLen);
	RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TmacLen), DIR_WRITE, FALSE);
#endif

	if (fragNum == pTxBlk->TotalFragNum) {
		dma_cb->pNdisPacket = pTxBlk->pPacket;
		dma_cb->pNextNdisPacket = NULL;
	}

	if (IS_ASIC_CAP(pAd, fASIC_CAP_CT))
		dma_cb->PacketPa = pTxBlk->SrcBufPA;
	else
		dma_cb->PacketPa = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);

	dma_cb->DmaBuf.AllocSize = mt_pci_get_buf_len(pAd, pTxBlk);
	dma_cb->DmaBuf.AllocPa = PCI_MAP_SINGLE(pAd, pDMAHeaderBufVA, dma_cb->DmaBuf.AllocSize, 0, RTMP_PCI_DMA_TODEVICE);

	pTxD->SDPtr0 = dma_cb->DmaBuf.AllocPa;
	pTxD->SDLen0 = dma_cb->DmaBuf.AllocSize;
	pTxD->SDPtr1 = dma_cb->PacketPa;
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (pTxD->SDLen1 ? 1 : 0);
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;
	wmb();
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
	RetTxIdx = TxIdx;
	pTxBlk->Priv += pTxBlk->SrcBufLen;
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(dma_cb->DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	RTMP_DCACHE_FLUSH(dma_cb->AllocPa, RXD_SIZE);
	/* Update Tx index */
	INC_RING_INDEX(TxIdx, tx_ring_size);
	tx_ring->TxCpuIdx = TxIdx;
	*FreeNumber -= 1;
	return RetTxIdx;
}

/*
*
*/
VOID dumpTxRing(RTMP_ADAPTER *pAd, INT ring_idx)
{
	RTMP_DMABUF *pDescRing;
	struct hif_pci_tx_ring *tx_ring;
	TXD_STRUC *pTxD;
	int index;
	UINT8 num_of_tx_ring = hif_get_tx_res_num(pAd->hdev_ctrl);
	UINT16 tx_ring_size;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	ASSERT(ring_idx < num_of_tx_ring);
	tx_ring = pci_get_tx_ring_by_ridx(hif, ring_idx);
	pDescRing = (RTMP_DMABUF *)tx_ring->desc_ring.AllocVa;

	tx_ring_size = tx_ring->ring_size;
	for (index = 0; index < tx_ring_size; index++) {
		pTxD = (TXD_STRUC *)tx_ring->Cell[index].AllocVa;
		hex_dump("Dump TxDesc", (UCHAR *)pTxD, sizeof(TXD_STRUC));
		dump_txd(pAd, pTxD);
	}
}

/*
*
*/
VOID dumpRxRing(RTMP_ADAPTER *pAd, INT ring_idx)
{
	RTMP_DMABUF *pDescRing;
	struct hif_pci_rx_ring *rx_ring;
	RXD_STRUC *pRxD;
	int index;
	UINT16 RxRingSize;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	rx_ring = pci_get_rx_ring_by_ridx(hif, 0);
	pDescRing = (RTMP_DMABUF *)rx_ring->desc_ring.AllocVa;
	RxRingSize = rx_ring->ring_size;

	for (index = 0; index < RxRingSize; index++) {
		pRxD = (RXD_STRUC *)rx_ring->Cell[index].AllocVa;
		hex_dump("Dump RxDesc", (UCHAR *)pRxD, sizeof(RXD_STRUC));
		dump_rxd(pAd, pRxD);
	}
}

#endif /* RTMP_MAC_PCI */
