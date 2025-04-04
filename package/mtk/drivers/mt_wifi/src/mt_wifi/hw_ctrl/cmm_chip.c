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
	cmm_chip.c
*/

#include "rt_config.h"
#include "hdev/hdev.h"

#ifdef TXBF_SUPPORT
VOID chip_tx_bf_init(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *pEntry, struct _IE_lists *ie_list, BOOLEAN supportsETxBF)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->TxBFInit)
		ops->TxBFInit(ad, pEntry, ie_list, supportsETxBF);
}
#endif /*TXBF_SUPPORT*/

UINT32 chip_get_sku_tbl_idx(RTMP_ADAPTER *ad, UINT8 *sku_tbl_idx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->get_sku_tbl_idx)
		return ops->get_sku_tbl_idx(ad, sku_tbl_idx);
	return FALSE;
}

BOOLEAN chip_check_rf_lock_down(RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->check_RF_lock_down)
		return ops->check_RF_lock_down(ad);
	return FALSE;
}

INT32 chip_cmd_tx(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->MtCmdTx)
		return ops->MtCmdTx(ad, msg);
	return 0;
}

BOOLEAN chip_eeprom_read16(struct _RTMP_ADAPTER *ad, UINT32 offset, USHORT *value)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->eeread)
		return ops->eeread(ad, offset, value);
	else
		return FALSE;
}

BOOLEAN chip_eeprom_read_with_range(struct _RTMP_ADAPTER *ad, UINT32 start, UINT32 length, UCHAR *pbuf)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->eeread_range)
		return ops->eeread_range(ad, start, length, pbuf);
	else
		return FALSE;
}

VOID chip_fw_init(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->FwInit && (IS_HIF_TYPE(ad, HIF_MT)))
		ops->FwInit(ad);
}

VOID chip_get_sta_per(struct _RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, PUINT8 u1PER)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* per sta get PER */
	if (ops->sta_per_get)
		ops->sta_per_get(pAd, u2WlanIdx, u1PER);
}

INT32 chip_ra_init(
	struct _RTMP_ADAPTER *ad,
	struct _MAC_TABLE_ENTRY *pEntry)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->RAInit)
		return ops->RAInit(ad, pEntry);
	return 0;
}

VOID chip_get_rssi(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, CHAR *RssiSet)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rssi_get)
		ops->rssi_get(pAd, Wcid, RssiSet);
}

VOID chip_get_cninfo(struct _RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT16 *pCnInfo)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/*cninfo stat parsing */
	if (ops->cninfo_get)
		ops->cninfo_get(pAd, ucBandIdx, pCnInfo);
}

VOID chip_set_mgmt_pkt_txpwr(struct _RTMP_ADAPTER *pAd,	struct wifi_dev *wdev, UINT8 prctg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->set_mgmt_pkt_txpwr_prctg)
		ops->set_mgmt_pkt_txpwr_prctg(pAd, wdev, prctg);
}
VOID chip_show_rxv_info(struct _RTMP_ADAPTER *ad, UINT8 band_idx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_info_show)
		ops->rxv_info_show(ad, band_idx);
}

INT32 chip_get_wf_path_comb(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	BOOLEAN dbdc_mode_en,
	UINT8 *path,
	UINT8 *path_len)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* wf path combination */
	if (ops->get_wf_path_comb)
		return ops->get_wf_path_comb(ad, band_idx, dbdc_mode_en, path, path_len);
	return 0;
}

INT32 chip_get_rx_stat_band(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	UINT8 blk_idx,
	P_TEST_RX_STAT_BAND_INFO prx_band)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->get_rx_stat_band)
		return ops->get_rx_stat_band(ad, band_idx, blk_idx, prx_band);
	return 0;
}

INT32 chip_get_rx_stat_path(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	UINT8 blk_idx,
	P_TEST_RX_STAT_PATH_INFO prx_path)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->get_rx_stat_path)
		return ops->get_rx_stat_path(ad, band_idx, blk_idx, prx_path);
	return 0;
}

INT32 chip_get_rx_stat_user(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	UINT8 blk_idx,
	P_TEST_RX_STAT_USER_INFO prx_user)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->get_rx_stat_user)
		return ops->get_rx_stat_user(ad, band_idx, blk_idx, prx_user);
	return 0;
}

INT32 chip_get_rx_stat_comm(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	UINT8 blk_idx,
	P_TEST_RX_STAT_COMM_INFO prx_comm)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->get_rx_stat_comm)
		return ops->get_rx_stat_comm(ad, band_idx, blk_idx, prx_comm);
	return 0;
}

VOID chip_get_rx_stat(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	P_TESTMODE_STATISTIC_INFO ptest_mode_stat_info)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* query rx stat */
	if (ops->get_rx_stat)
		ops->get_rx_stat(ad, band_idx, ptest_mode_stat_info);
}

VOID chip_get_rxv_cnt(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	UINT32 *byte_cnt)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_get_byte_cnt)
		ops->rxv_get_byte_cnt(ad, band_idx, byte_cnt);
}

VOID chip_get_rxv_content(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	PVOID *content)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_get_content)
		ops->rxv_get_content(ad, band_idx, content);
}

VOID chip_dump_rxv_raw_data(struct _RTMP_ADAPTER *ad, UINT8 band_idx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_raw_data_show)
		ops->rxv_raw_data_show(ad, band_idx);
}

VOID chip_reset_rxv_stat(struct _RTMP_ADAPTER *ad, UINT8 band_idx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_stat_reset)
		ops->rxv_stat_reset(ad, band_idx);
}

VOID chip_parse_rxv_packet(struct _RTMP_ADAPTER *ad, UINT32 Type, struct _RX_BLK *RxBlk, UCHAR *Data)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_packet_parse)
		ops->rxv_packet_parse(ad, Data);
}

VOID chip_parse_rxv_entry(struct _RTMP_ADAPTER *ad, VOID *Data)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_entry_parse)
		ops->rxv_entry_parse(ad, Data);
}

VOID chip_rxv_dump_start(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_dump_start)
		ops->rxv_dump_start(ad);
}

VOID chip_rxv_dump_stop(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_dump_stop)
		ops->rxv_dump_stop(ad);
}

VOID chip_rxv_dump_buf_alloc(struct _RTMP_ADAPTER *ad, UINT8 type_mask)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_dump_buf_alloc)
		ops->rxv_dump_buf_alloc(ad, type_mask);
}

VOID chip_rxv_dump_buf_clear(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_dump_buf_clear)
		ops->rxv_dump_buf_clear(ad);
}

VOID chip_rxv_dump_show_list(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_dump_show_list)
		ops->rxv_dump_show_list(ad);
}

VOID chip_rxv_dump_show_rpt(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_dump_show_rpt)
		ops->rxv_dump_show_rpt(ad);
}

VOID chip_rxv_dump_rxv_content_compose(struct _RTMP_ADAPTER *ad, UINT8 entry_idx, VOID *rxv_content, UINT32 *len)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_dump_rxv_content_compose)
		ops->rxv_dump_rxv_content_compose(ad, entry_idx, rxv_content, len);
}

VOID chip_rxv_content_len(struct _RTMP_ADAPTER *ad, UINT8 type_mask, UINT8 rxv_sta_cnt, UINT16 *len)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_content_len)
		ops->rxv_content_len(ad, type_mask, rxv_sta_cnt, len);
}

INT32 chip_txs_handler(RTMP_ADAPTER *ad, VOID *rx_packet)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->txs_handler)
		ops->txs_handler(ad, rx_packet);

	return FALSE;
}

INT chip_show_pwr_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->show_pwr_info)
		ops->show_pwr_info(pAd);

	return 0;
}

INT chip_update_mib_bucket(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->update_mib_bucket)
		ops->update_mib_bucket(pAd);

	return 0;
}

#ifdef OFFCHANNEL_ZERO_LOSS
INT chip_read_channel_stat_registers(RTMP_ADAPTER *pAd, UINT8 ucBandIdx, void *ChStat)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->read_channel_stat_registers)
		ops->read_channel_stat_registers(pAd, ucBandIdx, ChStat);

	return 0;
}
#endif


VOID chip_arch_set_aid(struct _RTMP_ADAPTER *ad, USHORT aid, UINT8 OmacIdx)
{
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);
		if (ops->archSetAid)
			ops->archSetAid(ad, aid, OmacIdx);
#endif
}

VOID AsicSetRxAnt(RTMP_ADAPTER *ad, UCHAR Ant)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->SetRxAnt)
		ops->SetRxAnt(ad, Ant);
}

#ifdef MICROWAVE_OVEN_SUPPORT
VOID AsicMeasureFalseCCA(RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->AsicMeasureFalseCCA)
		ops->AsicMeasureFalseCCAad;
}

VOID AsicMitigateMicrowave(RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->AsicMitigateMicrowave)
		ops->AsicMitigateMicrowave(ad);
}
#endif /* MICROWAVE_OVEN_SUPPORT */


VOID AsicBbpInitFromEEPROM(RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BbpInitFromEEPROM)
		ops->BbpInitFromEEPROM(ad);
}


#if defined(MT_MAC) && defined(TXBF_SUPPORT)
INT32 AsicBfStaRecUpdate(
	RTMP_ADAPTER *ad,
	UCHAR        ucPhyMode,
	UCHAR        ucBssIdx,
	UINT16       u2WlanIdx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfStaRecUpdate) {
		return ops->BfStaRecUpdate(
				   ad,
				   ucPhyMode,
				   ucBssIdx,
				   u2WlanIdx);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicBfeeStaRecUpdate(
	RTMP_ADAPTER * ad,
	UCHAR        u1PhyMode,
	UCHAR        u1BssIdx,
	UINT16       u2WlanIdx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfeeStaRecUpdate) {
		return ops->BfeeStaRecUpdate(
				   ad,
				   u1PhyMode,
				   u1BssIdx,
				   u2WlanIdx);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicBfStaRecRelease(
	RTMP_ADAPTER *ad,
	UCHAR        ucBssIdx,
	UINT16       u2WlanIdx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfStaRecRelease) {
		return ops->BfStaRecRelease(
				   ad,
				   ucBssIdx,
				   u2WlanIdx);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicBfPfmuMemAlloc(
	RTMP_ADAPTER *ad,
	UCHAR ucSu_Mu,
	UCHAR ucWlanId)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfPfmuMemAlloc) {
		return ops->BfPfmuMemAlloc(
				   ad,
				   ucSu_Mu,
				   ucWlanId);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicBfPfmuMemRelease(
	RTMP_ADAPTER *ad,
	UCHAR ucWlanId)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfPfmuMemRelease) {
		return ops->BfPfmuMemRelease(
				   ad,
				   ucWlanId);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicTxBfTxApplyCtrl(
	RTMP_ADAPTER *ad,
	UCHAR   ucWlanId,
	BOOLEAN fgETxBf,
	BOOLEAN fgITxBf,
	BOOLEAN fgMuTxBf,
	BOOLEAN fgPhaseCali)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->TxBfTxApplyCtrl) {
		return ops->TxBfTxApplyCtrl(
				   ad,
				   ucWlanId,
				   fgETxBf,
				   fgITxBf,
				   fgMuTxBf,
				   fgPhaseCali);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicTxBfeeHwCtrl(
	RTMP_ADAPTER *ad,
	BOOLEAN   fgBfeeHwCtrl)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfeeHwCtrl) {
		return ops->BfeeHwCtrl(
				   ad,
				   fgBfeeHwCtrl);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicTxBfApClientCluster(
	RTMP_ADAPTER *ad,
	UCHAR   ucWlanId,
	UCHAR   ucCmmWlanId)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfApClientCluster) {
		return ops->BfApClientCluster(
				   ad,
				   ucWlanId,
				   ucCmmWlanId);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicTxBfReptClonedStaToNormalSta(
	RTMP_ADAPTER *ad,
	UCHAR   ucWlanId,
	UCHAR   ucCliIdx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfReptClonedStaToNormalSta) {
		return ops->BfReptClonedStaToNormalSta(
				   ad,
				   ucWlanId,
				   ucCliIdx);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicTxBfHwEnStatusUpdate(
	RTMP_ADAPTER *ad,
	BOOLEAN   fgETxBf,
	BOOLEAN   fgITxBf)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfHwEnStatusUpdate) {
		return ops->BfHwEnStatusUpdate(
				   ad,
				   fgETxBf,
				   fgITxBf);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicTxBfModuleEnCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BfNum,
	UINT8 u1BfBitmap,
	UINT8 u1BfSelBand[])
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->BfModuleEnCtrl) {
		return ops->BfModuleEnCtrl(
					pAd,
					u1BfNum,
					u1BfBitmap,
					u1BfSelBand);
	} else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}

INT32 AsicTxBfCfgBfPhy(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pucData)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->BfCfgBfPhy) {
		return ops->BfCfgBfPhy(pAd, pucData);
	} else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}

BOOLEAN asic_txbf_bfee_adaption(
	struct _RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->bfee_adaption) {
		return ops->bfee_adaption(pAd);
	} else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}

#endif /* MT_MAC && TXBF_SUPPORT */

INT32 AsicHeraStbcPriorityCtrl(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pucData)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->HeraStbcPriorityCtrl)
		return ops->HeraStbcPriorityCtrl(pAd, pucData);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT32 chip_tssi_set(struct _RTMP_ADAPTER *ad, char *efuse)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->tssi_set)
		return ops->tssi_set(ad, efuse);
	return 0;
}

INT32 chip_pa_lna_set(struct _RTMP_ADAPTER *ad, char *efuse)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->pa_lna_set)
		return ops->pa_lna_set(ad, efuse);

	return 0;
}

UINT16 chip_get_tid_sn(RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR tid)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->get_tid_sn)
		return ops->get_tid_sn(pAd, wcid, tid);
	else
		return 0;
}

INT chip_init_hif_dma(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->hif_init_dma)
		return ops->hif_init_dma(pAd);

	return FALSE;
}

INT chip_set_hif_dma(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->hif_set_dma)
		return ops->hif_set_dma(pAd, TxRx, enable);

	return FALSE;
}


BOOLEAN chip_wait_hif_dma_idle(struct _RTMP_ADAPTER *pAd, UINT8 pcie_port_or_all, INT round, INT wait_us)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->hif_wait_dma_idle)
		return ops->hif_wait_dma_idle(pAd, pcie_port_or_all, round, wait_us);

	return FALSE;
}

BOOLEAN chip_reset_hif_dma(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->hif_reset_dma)
		return ops->hif_reset_dma(pAd);

	return FALSE;
}

INT32 chip_init_dmasch(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->dma_shdl_init)
		return ops->dma_shdl_init(pAd);

	return FALSE;
}

INT32 chip_cfg_dly_int(void *hdev_ctrl, UINT32 idx, UINT16 dly_number, UINT16 dly_time)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);

	if (ops->hif_cfg_dly_int)
		return ops->hif_cfg_dly_int(hdev_ctrl, idx, dly_number, dly_time);

	return FALSE;
}

VOID chip_interrupt_enable(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->interrupt_enable)
		return ops->interrupt_enable(pAd);
}

VOID chip_interrupt_disable(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->interrupt_disable)
		return ops->interrupt_disable(pAd);
}

INT chip_trigger_int_to_mcu(RTMP_ADAPTER *pAd, UINT32 status)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->trigger_int_to_mcu)
		return ops->trigger_int_to_mcu(pAd, status);

	return FALSE;
}

VOID chip_subsys_int_handler(RTMP_ADAPTER *pAd, void *hif_chip)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->subsys_int_handler)
		ops->subsys_int_handler(pAd, hif_chip);
}

VOID chip_sw_int_handler(RTMP_ADAPTER *pAd, void *hif_chip)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->sw_int_handler)
		ops->sw_int_handler(pAd, hif_chip);
}

VOID chip_hif_chip_match(VOID *hdev_ctrl)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);

	if (ops->hif_chip_match)
		ops->hif_chip_match(hdev_ctrl);
}

VOID chip_hif_pci_data_ring_assign(VOID *hdev_ctrl, UINT8 *resrc_idx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);

	if (ops->hif_pci_data_ring_assign)
		ops->hif_pci_data_ring_assign(hdev_ctrl, resrc_idx);
}

VOID chip_hif_pci_slave_chip_defer_create(VOID *hdev_ctrl)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);

	if (ops->hif_pci_slave_chip_defer_create)
		ops->hif_pci_slave_chip_defer_create(hdev_ctrl);
}

INT32 chip_fill_key_install_cmd(
	VOID *hdev_ctrl,
	struct _ASIC_SEC_INFO *asic_sec_info,
	UCHAR is_sta_rec_update, /* TRUE: sta_rec, FALSE: wtbl */
	VOID **wtbl_security_key,
	UINT32 *cmd_len)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);

	/* only fill_key_install_cmd_v2 support bigtk */
	if (ops->fill_key_install_cmd)
		return ops->fill_key_install_cmd(asic_sec_info, is_sta_rec_update, wtbl_security_key, cmd_len);

	return NDIS_STATUS_FAILURE;
}

#ifdef WIFI_UNIFIED_COMMAND
INT32 chip_fill_key_install_uni_cmd(
	VOID *hdev_ctrl,
	struct _ASIC_SEC_INFO *asic_sec_info,
	UCHAR is_sta_rec_update, /* TRUE: sta_rec, FALSE: wtbl */
	VOID *wtbl_security_key,
	UINT32 *cmd_len)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);

	/* only fill_key_install_cmd_v2 support bigtk */
	if (ops->fill_key_install_uni_cmd)
		return ops->fill_key_install_uni_cmd(asic_sec_info, is_sta_rec_update, wtbl_security_key, cmd_len);

	return NDIS_STATUS_FAILURE;
}

INT32 chip_fill_key_install_uni_cmd_dynsize_check(
	VOID *hdev_ctrl,
	struct _ASIC_SEC_INFO *asic_sec_info,
	UINT32 *cmd_len)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);

	/* only fill_key_install_cmd_v2 support bigtk */
	if (ops->fill_key_install_uni_cmd_dynsize_check)
		return ops->fill_key_install_uni_cmd_dynsize_check(asic_sec_info, cmd_len);

	return NDIS_STATUS_FAILURE;
}
#endif /* WIFI_UNIFIED_COMMAND*/

#ifdef CONFIG_TX_DELAY
VOID chip_tx_deley_parm_init(
	VOID *hdev_ctrl,
	UCHAR tx_delay_mode,
	struct tx_delay_control *tx_delay_ctl)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);

	if (ops->tx_deley_parm_init)
		ops->tx_deley_parm_init(tx_delay_mode, tx_delay_ctl);
}
#endif

#ifdef ERR_RECOVERY
VOID chip_dump_ser_stat(RTMP_ADAPTER *pAd, UINT8 dump_lvl)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->dump_ser_stat)
		ops->dump_ser_stat(pAd, dump_lvl);
}

#ifdef MT7915_E1_WORKAROUND
#ifdef WFDMA_WED_COMPATIBLE
VOID chip_sw_int_polling(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->sw_int_polling)
		ops->sw_int_polling(pAd);
}
#endif
#endif
#endif

VOID chip_update_chip_cap(
	struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->update_chip_cap)
		return ops->update_chip_cap(ad);
}

UINT32 chip_get_sub_chipid(
	struct _RTMP_ADAPTER *ad,
	UINT32 *sub_chipid)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->get_sub_chipid == NULL)
		MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, " get_sub_chipid: NULL\n");


	if (ops->get_sub_chipid)
		return ops->get_sub_chipid(ad, sub_chipid);

	MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, " get_sub_chipid: %04x\n", *sub_chipid);

	*sub_chipid = 0;
	return FALSE;
}

INT ChkExceptionType(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->chk_exception_type)
		return chip_dbg->chk_exception_type(pAd);

	return FALSE;
}

VOID chip_rx_ics_handler(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucPktType,
	UINT8 *pucData,
	UINT16 u2Length)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rx_ics_handler)
		ops->rx_ics_handler(pAd, ucPktType, pucData, u2Length);
}
