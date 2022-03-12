/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	operation_jedi.c
*/
#include "rt_config.h"
#include "operation.h"

s_int32 mt_op_set_tr_mac(
	struct test_wlan_info *winfos,
	s_int32 op_type, boolean enable, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_ATE_TEST_MODE_T param;
#endif

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

#ifdef CONFIG_HW_HAL_OFFLOAD
	sys_ad_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = 1;
	param.ucAteIdx = EXT_ATE_SET_TRX;
	param.Data.rAteSetTrx.ucType = op_type;
	param.Data.rAteSetTrx.ucEnable = enable;
	param.Data.rAteSetTrx.ucBand = band_idx;
	/*
	 * Make sure FW command configuration completed
	 * for store tx packet in PLE first
	 * Use aucReserved[1] for ucATEIdx extension feasibility
	 */
	param.aucReserved[1] = INIT_CMD_SET_AND_WAIT_RETRY_RSP;
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: op_type=%d, enable=%u, band_idx=%u\n",
		__func__, op_type, enable, band_idx));

	ret = MtCmdATETest(ad, &param);
#else
	ret = MtAsicSetMacTxRx(ad, op_type, enable, band_idx);
#endif

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_tx_stream(
	struct test_wlan_info *winfos,
	u_int32 stream_nums, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_ATE_TEST_MODE_T param;
#endif

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

#ifdef CONFIG_HW_HAL_OFFLOAD
	sys_ad_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = 1;
	param.ucAteIdx = EXT_ATE_SET_TX_STREAM;
	param.Data.rAteSetTxStream.ucStreamNum = stream_nums;
	param.Data.rAteSetTxStream.ucBand = band_idx;
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: stream_nums=%u, band_idx=%u\n",
		__func__, stream_nums, band_idx));

	ret =  MtCmdATETest(ad, &param);
#else
	ret = MtAsicSetTxStream(ad, stream_nums, band_idx);
#endif

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_tx_path(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_set_rx_path(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}


s_int32 mt_op_set_rx_filter(
	struct test_wlan_info *winfos,
	struct rx_filter_ctrl rx_filter)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
#ifdef CONFIG_HW_HAL_OFFLOAD
	MT_RX_FILTER_CTRL_T filter;
	struct _EXT_CMD_ATE_TEST_MODE_T param;
#endif

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

#ifdef CONFIG_HW_HAL_OFFLOAD
	sys_ad_zero_mem(&filter, sizeof(MT_RX_FILTER_CTRL_T));
	sys_ad_move_mem(&filter, &rx_filter, sizeof(MT_RX_FILTER_CTRL_T));
	sys_ad_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = 1;
	param.ucAteIdx = EXT_ATE_SET_RX_FILTER;
	param.Data.rAteSetRxFilter.ucBand = filter.u1BandIdx;

	if (filter.bPromiscuous)
		param.Data.rAteSetRxFilter.ucPromiscuousMode = 1;
	else {
		param.Data.rAteSetRxFilter.ucReportEn =
					(u_char)filter.bFrameReport;
		param.Data.rAteSetRxFilter.u4FilterMask =
					cpu2le32(filter.filterMask);
	}
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx=%u\n", __func__, filter.u1BandIdx));

	ret =  MtCmdATETest(ad, &param);
#else
	ret = MtAsicSetRxFilter(ad, filter);
#endif

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_clean_persta_txq(
	struct test_wlan_info *winfos,
	boolean sta_pause_enable,
	void *virtual_wtbl,
	u_char omac_idx,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#ifdef CONFIG_HW_HAL_OFFLOAD
	RTMP_ADAPTER *ad = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct _EXT_CMD_ATE_TEST_MODE_T param;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	entry = (struct _MAC_TABLE_ENTRY *)virtual_wtbl;
	sys_ad_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = 1;
	param.ucAteIdx = EXT_ATE_SET_CLEAN_PERSTA_TXQUEUE;
	param.Data.rAteSetCleanPerStaTxQueue.fgStaPauseEnable =
							sta_pause_enable;
	/* Give a same STA ID */
	param.Data.rAteSetCleanPerStaTxQueue.ucStaID = entry->wcid;
	param.Data.rAteSetCleanPerStaTxQueue.ucBand = band_idx;
	/* use omac index*/
	param.Data.rAteSetCleanPerStaTxQueue.aucReserved[0] = omac_idx;
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: wcid[%d], pause=%u, band_idx=%u, reserved[0]=%u\n",
		__func__, entry->wcid, sta_pause_enable, band_idx,
		param.Data.rAteSetCleanPerStaTxQueue.aucReserved[0]));

	ret =  MtCmdATETest(ad, &param);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
#else
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: function does't support yet.\n", __func__));
#endif

	return ret;
}

s_int32 mt_op_set_cfg_on_off(
	struct test_wlan_info *winfos,
	u_int8 type, u_int8 enable, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#ifdef CONFIG_HW_HAL_OFFLOAD
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: type=%u, enable=%u, band_idx=%u\n",
		__func__, type, enable, band_idx));

	ret = MtCmdCfgOnOff(ad, type, enable, (u_int8)band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
#endif

	return ret;
}

s_int32 mt_op_log_on_off(
	struct test_wlan_info *winfos,
	struct test_log_dump_cb *log_cb,
	u_int32 log_type,
	u_int32 log_ctrl,
	u_int32 log_size)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	if (log_cb == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	switch (log_ctrl) {
	case TEST_LOG_ON:
		/* init log */
		log_cb->len = log_size;
		log_cb->first_en = TRUE;

		if (!log_cb->entry) {
			sys_ad_zero_mem(log_cb, sizeof(*log_cb));
			ret = sys_ad_alloc_mem(
				(PUCHAR *)&log_cb->entry,
				log_size * sizeof(struct test_log_dump_entry));

			if (ret) {
				SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
					("%s: allocated memory fail! size %u\n",
					__func__, log_size));
				return SERV_STATUS_HAL_OP_FAIL;
			}

			sys_ad_zero_mem(log_cb->entry,
				log_size * sizeof(struct test_log_dump_entry));

			SERV_OS_ALLOCATE_SPIN_LOCK(&log_cb->lock);
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
				("%s: init log cb size %u, log_cb->len:%u\n",
				__func__, log_size, log_cb->len));
		}

		break;

	case TEST_LOG_OFF:
		break;

	default:
		goto err0;
	}

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: log_type:0x%x, log_ctrl:0x%x, log_size:0x%x\n",
			 __func__, log_type, log_ctrl, log_size));

	return ret;

err0:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s log type %d not supported\n",
		__func__, log_type));
	return SERV_STATUS_HAL_OP_FAIL;
}


s_int32 mt_op_set_antenna_port(
	struct test_wlan_info *winfos,
	u_int8 rf_mode_mask, u_int8 rf_port_mask, u_int8 ant_port_mask)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#ifdef CONFIG_HW_HAL_OFFLOAD
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: rf_mode_mask=%u, rf_port_mask=%u, ant_port_mask=%u\n",
		__func__, rf_mode_mask, rf_port_mask, ant_port_mask));

	ret = MtCmdSetAntennaPort(
		ad, rf_mode_mask, rf_port_mask, ant_port_mask);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
#endif

	return ret;
}

s_int32 mt_op_set_slot_time(
	struct test_wlan_info *winfos,
	u_int8 slot_time, u_int8 sifs_time, u_int8 rifs_time,
	u_int16 eifs_time, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdATESetSlotTime(ad, slot_time, sifs_time,
				  rifs_time, eifs_time, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_power_drop_level(
	struct test_wlan_info *winfos,
	u_int8 pwr_drop_level, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#ifdef CONFIG_HW_HAL_OFFLOAD
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: pwr_drop_level=%u, band_idx=%u\n",
		__func__, pwr_drop_level, band_idx));

	ret = MtCmdATESetPowerDropLevel(ad, pwr_drop_level, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
#endif /* CONFIG_HW_HAL_OFFLOAD */

	return ret;
}

s_int32 mt_op_set_rx_filter_pkt_len(
	struct test_wlan_info *winfos,
	u_int8 enable, u_char band_idx, u_int32 rx_pkt_len)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#ifdef CONFIG_HW_HAL_OFFLOAD
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: enable=%u, band_idx=%u, rx_pkt_len=%u\n",
		__func__, enable, band_idx, rx_pkt_len));

	ret =  MtCmdRxFilterPktLen(ad, enable, band_idx, rx_pkt_len);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
#endif

	return ret;
}

s_int32 mt_op_get_antswap_capability(
	struct test_wlan_info *winfos,
	u_int32 *antswap_support)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_set_antswap(
	struct test_wlan_info *winfos,
	u_int32 ant)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_get_thermal_value(
	struct test_wlan_info *winfos,
	struct test_configuration *test_configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_set_freq_offset(
	struct test_wlan_info *winfos,
	u_int32 freq_offset, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

#ifdef CONFIG_HW_HAL_OFFLOAD
	ret = MtCmdSetFreqOffset(ad, freq_offset, band_idx);
#else
	ret = MtAsicSetRfFreqOffset(ad, freq_offset);
#endif

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_phy_counter(
	struct test_wlan_info *winfos,
	s_int32 control, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdSetPhyCounter(ad, control, (u_int8) band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_rxv_index(
	struct test_wlan_info *winfos,
	u_int8 group_1, u_int8 group_2, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdSetRxvIndex(ad, group_1, group_2, (u_int8) band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_fagc_path(
	struct test_wlan_info *winfos,
	u_int8 path, u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdSetFAGCPath(ad, path, (u_int8) band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_fw_mode(
	struct test_wlan_info *winfos, u_char fw_mode)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdATEModeCtrl(ad, fw_mode);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_rf_test_mode(
	struct test_wlan_info *winfos,
	u_int32 op_mode, u_int8 icap_len, u_int16 rsp_len)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdRfTestSwitchMode(ad, op_mode, icap_len, rsp_len);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_test_mode_start(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_set_test_mode_abort(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_backup_and_set_cr(
	struct test_wlan_info *winfos,
	struct test_bk_cr *bks,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = mt_test_mac_backup_and_set_cr(winfos, bks, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SET_MAC;

	return ret;
}

s_int32 mt_op_restore_cr(
	struct test_wlan_info *winfos,
	struct test_bk_cr *bks,
	u_char band_idx,
	u_char option)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = mt_test_mac_restore_cr(winfos, bks, band_idx, option);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SET_MAC;

	return ret;
}

s_int32 mt_op_set_ampdu_ba_limit(
	struct test_wlan_info *winfos,
	u_int8 wmm_idx,
	u_int8 agg_limit)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = mt_test_mac_set_ampdu_ba_limit(winfos, wmm_idx, agg_limit);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SET_MAC;

	return ret;
}

s_int32 mt_op_set_sta_pause_cr(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = mt_test_mac_set_sta_pause_cr(winfos);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SET_MAC;

	return ret;
}

s_int32 mt_op_set_ifs_cr(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = mt_test_mac_set_ifs_cr(winfos, configs, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SET_MAC;

	return ret;
}

s_int32 mt_op_write_mac_bbp_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_read_bulk_mac_bbp_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_read_bulk_rf_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_write_bulk_rf_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_read_bulk_eeprom(
	struct test_wlan_info *winfos,
	struct test_eeprom *eprms)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_start_tx(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_stop_tx(
	struct test_wlan_info *winfos,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_start_rx(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_stop_rx(
	struct test_wlan_info *winfos,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_set_channel(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_OSAL_NET_INVALID_PAD;

	ret = net_ad_update_wdev(band_idx, winfos, configs);
	if (ret)
		goto error;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx: %d, bw: %d, ch:%d",
		__func__, band_idx, configs->bw, configs->channel));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("ctrl_ch: %d, cntl_ch2: %d, pri_sel: %d\n",
		configs->ctrl_ch, configs->channel_2nd, configs->pri_sel));

	return ret;

error:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: set channel fail, ", __func__));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("control channel: %d|%d\n", configs->ctrl_ch,
		configs->channel));
	return SERV_STATUS_OSAL_NET_FAIL_SET_CHANNEL;
}

s_int32 mt_op_set_tx_content(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_set_preamble(
	struct test_wlan_info *winfos,
	u_char mode)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_set_system_bw(
	struct test_wlan_info *winfos,
	u_char sys_bw)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_set_per_pkt_bw(
	struct test_wlan_info *winfos,
	u_char per_pkt_bw)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_reset_txrx_counter(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RX_STATISTIC_RXV *rx_stat;
	u_int32 control = 0, user_idx = 0, band_idx = 0;
	struct _RTMP_CHIP_DBG *chip_dbg = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: reset txrx counter\n", __func__));

	chip_dbg = hc_get_chip_dbg(ad->hdev_ctrl);

	for (band_idx = TEST_DBDC_BAND0;
			band_idx < TEST_DBDC_BAND_NUM; band_idx++) {
		control = 0;
		ret = MtCmdSetPhyCounter(ad, control, band_idx);
		if (ret)
			goto error;

		control = 1;
		ret = MtCmdSetPhyCounter(ad, control, band_idx);
		if (ret)
			goto error;

		/* reset rx stat fcs error count */
		rx_stat = ad->rx_stat_rxv + band_idx;
		for (user_idx = 0; user_idx < TEST_USER_NUM; user_idx++) {
			rx_stat->fcs_error_cnt[user_idx] = 0;
			rx_stat->FreqOffsetFromRx[user_idx] = 0;
			rx_stat->SNR[user_idx] = 0;
		}

		if (chip_dbg) {
			chip_dbg->get_tx_mibinfo(ad, band_idx,
						MODE_CCK,
						BW_20);
			chip_dbg->get_tx_mibinfo(ad, band_idx,
						MODE_CCK,
						BW_40);
			chip_dbg->get_tx_mibinfo(ad, band_idx,
						MODE_CCK,
						BW_80);
			chip_dbg->get_tx_mibinfo(ad, band_idx,
						MODE_CCK,
						BW_160);
			chip_dbg->get_tx_mibinfo(ad, band_idx,
						MODE_HE_MU,
						BW_20);
		}
	}

	return ret;

error:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: reset tx rx phy counter fail(0x%08x).\n", __func__, ret));
	return ret;
}

s_int32 mt_op_set_rx_vector_idx(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 group1,
	u_int32 group2)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_set_fagc_rssi_path(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 fagc_path)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_get_rx_stat_leg(
	struct test_wlan_info *winfos,
	struct test_rx_stat_leg *rx_stat)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	TESTMODE_STATISTIC_INFO st;
	RX_STATISTIC_RXV *rx_stat_rxv;
	u_char band_idx, band_num, user_idx;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	if (IS_TEST_DBDC(winfos)) {
		band_num = 2;
		rx_stat_rxv = ad->rx_stat_rxv + 0;
		rx_stat->fagc_ib_rssi[0] = rx_stat_rxv->FAGC_RSSI_IB[0];
		rx_stat->fagc_ib_rssi[1] = rx_stat_rxv->FAGC_RSSI_IB[1];
		rx_stat->fagc_wb_rssi[0] = rx_stat_rxv->FAGC_RSSI_WB[0];
		rx_stat->fagc_wb_rssi[1] = rx_stat_rxv->FAGC_RSSI_WB[1];
		rx_stat->rcpi0 = rx_stat_rxv->RCPI[0];
		rx_stat->rcpi1 = rx_stat_rxv->RCPI[1];
		rx_stat->rssi0 = rx_stat_rxv->RSSI[0];
		rx_stat->rssi1 = rx_stat_rxv->RSSI[1];

		rx_stat_rxv = ad->rx_stat_rxv + 1;
		rx_stat->fagc_ib_rssi[2] = rx_stat_rxv->FAGC_RSSI_IB[0];
		rx_stat->fagc_ib_rssi[3] = rx_stat_rxv->FAGC_RSSI_IB[1];
		rx_stat->fagc_wb_rssi[2] = rx_stat_rxv->FAGC_RSSI_WB[0];
		rx_stat->fagc_wb_rssi[3] = rx_stat_rxv->FAGC_RSSI_WB[1];
		rx_stat->rcpi2 = rx_stat_rxv->RCPI[0];
		rx_stat->rcpi3 = rx_stat_rxv->RCPI[1];
		rx_stat->rssi2 = rx_stat_rxv->RSSI[0];
		rx_stat->rssi3 = rx_stat_rxv->RSSI[1];
	} else {
		band_num = 1;
		rx_stat_rxv = ad->rx_stat_rxv + 0;
		rx_stat->fagc_ib_rssi[0] = rx_stat_rxv->FAGC_RSSI_IB[0];
		rx_stat->fagc_ib_rssi[1] = rx_stat_rxv->FAGC_RSSI_IB[1];
		rx_stat->fagc_ib_rssi[2] = rx_stat_rxv->FAGC_RSSI_IB[2];
		rx_stat->fagc_ib_rssi[3] = rx_stat_rxv->FAGC_RSSI_IB[3];
		rx_stat->fagc_wb_rssi[0] = rx_stat_rxv->FAGC_RSSI_WB[0];
		rx_stat->fagc_wb_rssi[1] = rx_stat_rxv->FAGC_RSSI_WB[1];
		rx_stat->fagc_wb_rssi[2] = rx_stat_rxv->FAGC_RSSI_WB[2];
		rx_stat->fagc_wb_rssi[3] = rx_stat_rxv->FAGC_RSSI_WB[3];
		rx_stat->rcpi0 = rx_stat_rxv->RCPI[0];
		rx_stat->rcpi1 = rx_stat_rxv->RCPI[1];
		rx_stat->rcpi2 = rx_stat_rxv->RCPI[2];
		rx_stat->rcpi3 = rx_stat_rxv->RCPI[3];
		rx_stat->rssi0 = rx_stat_rxv->RSSI[0];
		rx_stat->rssi1 = rx_stat_rxv->RSSI[1];
		rx_stat->rssi2 = rx_stat_rxv->RSSI[2];
		rx_stat->rssi3 = rx_stat_rxv->RSSI[3];
	}

	for (band_idx = 0; band_idx < band_num; band_idx++) {
		/* read statistic from firmware */
		chip_get_rx_stat(ad, band_idx, &st);

		/* Copy statistic info */
		switch (band_idx) {
		case 0:
			/* MAC COUNT */
			rx_stat->mac_rx_fcs_ok_cnt =
				st.mac_rx_fcs_ok_cnt;
			rx_stat->mac_rx_fcs_err_cnt =
				st.mac_rx_fcs_err_cnt;
			rx_stat->mac_rx_len_mismatch =
				st.mac_rx_len_mismatch;
			rx_stat->rx_fifo_full =
				st.mac_rx_fifo_full;
			rx_stat->mac_rx_mdrdy_cnt =
				st.mac_rx_mdrdy_cnt;

			/* PHY COUNT */
			rx_stat->phy_rx_pd_cck =
				st.phy_rx_pd_cck;
			rx_stat->phy_rx_pd_ofdm =
				st.phy_rx_pd_ofdm;
			rx_stat->phy_rx_sig_err_cck =
				st.phy_rx_sig_err_cck;
			rx_stat->phy_rx_sfd_err_cck =
				st.phy_rx_sfd_err_cck;
			rx_stat->phy_rx_sig_err_ofdm =
				st.phy_rx_sig_err_ofdm;
			rx_stat->phy_rx_tag_err_ofdm =
				st.phy_rx_tag_err_ofdm;
			rx_stat->phy_rx_mdrdy_cnt_cck =
				st.phy_rx_mdrdy_cnt_cck;
			rx_stat->phy_rx_mdrdy_cnt_ofdm =
				st.phy_rx_mdrdy_cnt_ofdm;
			rx_stat->phy_rx_fcs_err_cnt_cck =
				st.phy_rx_fcs_err_cnt_cck;
			rx_stat->phy_rx_fcs_err_cnt_ofdm =
				st.phy_rx_fcs_err_cnt_ofdm;
			break;

		case 1:
			/* MAC COUNT */
			rx_stat->mac_rx_fcs_err_cnt_band1 =
				st.mac_rx_fcs_err_cnt;
			rx_stat->mac_rx_len_mismatch_band1 =
				st.mac_rx_len_mismatch;
			rx_stat->rx_fifo_full_band1 =
				st.mac_rx_fifo_full;
			rx_stat->mac_rx_mdrdy_cnt_band1 =
				st.mac_rx_mdrdy_cnt;

			/* PHY COUNT */
			rx_stat->phy_rx_pd_cck_band1 =
				st.phy_rx_pd_cck;
			rx_stat->phy_rx_pd_ofdm_band1 =
				st.phy_rx_pd_ofdm;
			rx_stat->phy_rx_sig_err_cck_band1 =
				st.phy_rx_sig_err_cck;
			rx_stat->phy_rx_sfd_err_cck_band1 =
				st.phy_rx_sfd_err_cck;
			rx_stat->phy_rx_sig_err_ofdm_band1 =
				st.phy_rx_sig_err_ofdm;
			rx_stat->phy_rx_tag_err_ofdm_band1 =
				st.phy_rx_tag_err_ofdm;
			rx_stat->phy_rx_mdrdy_cnt_cck_band1 =
				st.phy_rx_mdrdy_cnt_cck;
			rx_stat->phy_rx_mdrdy_cnt_ofdm_band1 =
				st.phy_rx_mdrdy_cnt_ofdm;
			break;

		default:
			ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
		}
	}

	rx_stat->freq_offset_rx =
		rx_stat_rxv->FreqOffsetFromRx[0];

	for (user_idx = 0; user_idx < TEST_USER_NUM; user_idx++) {
		rx_stat->user_rx_freq_offset[user_idx] =
			rx_stat_rxv->FreqOffsetFromRx[user_idx];
		rx_stat->user_snr[user_idx] =
			(u_int32)rx_stat_rxv->SNR[user_idx];
		rx_stat->fcs_error_cnt[user_idx] =
			rx_stat_rxv->fcs_error_cnt[user_idx];
	}

	return ret;
}

s_int32 mt_op_get_rxv_dump_action(
	struct test_wlan_info *winfos,
	u_int32 action,
	u_int32 type_mask)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	if (action == TEST_RXV_DUMP_STOP)
		chip_rxv_dump_stop(ad);
	else if (action == TEST_RXV_DUMP_CLEAR_BUFFER)
		chip_rxv_dump_buf_clear(ad);
	else {
		chip_rxv_dump_buf_alloc(ad, type_mask);
		chip_rxv_dump_start(ad);
	}

	return ret;
}

s_int32 mt_op_get_rxv_dump_ring_attr(
	struct test_wlan_info *winfos,
	struct rxv_dump_ring_attr *attr)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RXV_DUMP_CTRL *ctrl = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ctrl = &ad->rxv_dump_ctrl;
	attr->type_mask = ctrl->type_mask;
	attr->valid_entry_num = ctrl->valid_entry_num;
	attr->ring_idx = ctrl->ring_idx;
	attr->dump_entry_total_num = ctrl->dump_entry_total_num;

	return ret;
}

s_int32 mt_op_get_rxv_content_len(
	struct test_wlan_info *winfos,
	u_int8 type_mask,
	u_int8 rxv_sta_cnt,
	u_int16 *len)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	chip_rxv_content_len(ad,
		type_mask, rxv_sta_cnt, len);

	return ret;
}

s_int32 mt_op_get_rxv_dump_rxv_content(
	struct test_wlan_info *winfos,
	u_int8 entry_idx,
	u_int32 *content_len,
	void *rxv_content)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	chip_rxv_dump_rxv_content_compose(ad,
		entry_idx, rxv_content, content_len);

	return ret;
}

s_int32 mt_op_dbdc_tx_tone(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_int8 tx_tone_en = 0, ant_idx = 0, tone_type = 0;
	u_int8 tone_freq = 0;
	s_int32 dc_offset_I = 0, dc_offset_Q = 0, ch_band = 0;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	tx_tone_en = (u_int8) configs->tx_tone_en;
	ant_idx = (u_int8) configs->ant_idx;
	tone_type = (u_int8) configs->tone_type;
	tone_freq = (u_int8) configs->tone_freq;
	dc_offset_I = (s_int32) configs->dc_offset_I;
	dc_offset_Q = (s_int32) configs->dc_offset_Q;
	ch_band = (s_int32) configs->ch_band;

	ret = MtCmdTxTone(ad, band_idx, tx_tone_en,
		ant_idx, tone_type, tone_freq,
		dc_offset_I, dc_offset_Q, ch_band);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_dbdc_tx_tone_pwr(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_int8 ant_idx = 0;
	u_int32 digi_pwr = 0;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ant_idx = (u_int8) configs->ant_idx;
	digi_pwr = configs->digi_pwr;

	ret = MtCmdTxTonePower(ad, 0x12, digi_pwr, ant_idx, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_dbdc_continuous_tx(
	struct test_wlan_info *winfos,
	u_char band_idx,
	struct test_configuration *configs)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_int8 tx_tone_en = 0;
	u_int32 ant_mask = 0, tx_mode = 0, bw = 0;
	u_int32 pri_sel = 0, rate = 0, channel = 0;
	u_int32 tx_fd_mode = 0;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	tx_tone_en = configs->tx_tone_en;
	ant_mask = configs->ant_mask;
	tx_mode = configs->tx_mode;
	bw = configs->bw;
	pri_sel = configs->pri_sel;
	rate = configs->rate;
	channel = configs->channel;
	tx_fd_mode = configs->tx_fd_mode;

	ret = MtCmdTxContinous(ad, tx_mode, bw,
		pri_sel, channel, rate, ant_mask,
		tx_fd_mode, band_idx, tx_tone_en);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_get_tx_info(
	struct test_wlan_info *winfos,
	struct test_configuration *test_configs_band0,
	struct test_configuration *test_configs_band1)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 mib_counter = 0;
	RTMP_ADAPTER *ad = NULL;
	struct _RTMP_CHIP_DBG *chip_dbg = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	chip_dbg = hc_get_chip_dbg(ad->hdev_ctrl);

	if (chip_dbg) {
		u_int8 dbw = test_configs_band0->per_pkt_bw;

		if (dbw >= TEST_BW_160C)
			dbw = BAND_WIDTH_160;
		mib_counter = chip_dbg->get_tx_mibinfo(ad, BAND0,
					test_configs_band0->tx_mode,
					dbw);

		test_configs_band0->tx_stat.tx_done_cnt += mib_counter;

		dbw = test_configs_band1->per_pkt_bw;
		if (dbw >= TEST_BW_160C)
			dbw = BAND_WIDTH_160;
		mib_counter = chip_dbg->get_tx_mibinfo(ad, BAND1,
					test_configs_band1->tx_mode,
					dbw);
		test_configs_band1->tx_stat.tx_done_cnt += mib_counter;
	} else
		ret = SERV_STATUS_HAL_OP_FAIL;

	return ret;
}

s_int32 mt_op_get_rx_statistics_all(
	struct test_wlan_info *winfos,
	struct hqa_comm_rx_stat *hqa_rx_stat)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_calibration_test_mode(
	struct test_wlan_info *winfos,
	u_char mode)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	u_int8 icap_len = 0;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdRfTestSwitchMode(ad, mode,
		icap_len, RF_TEST_DEFAULT_RESP_LEN);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_icap_start(
	struct test_wlan_info *winfos,
	u_int8 *data)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;
#ifdef INTERNAL_CAPTURE_SUPPORT
	RBIST_CAP_START_T *prICapInfo = (RBIST_CAP_START_T *)data;
#endif/* INTERNAL_CAPTURE_SUPPORT */

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

#ifdef INTERNAL_CAPTURE_SUPPORT
	if (ops->ICapStart != NULL)
		ret = ops->ICapStart(ad, (u_int8 *)prICapInfo);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s : The function is not hooked !!\n", __func__));
	}
#endif/* INTERNAL_CAPTURE_SUPPORT */

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_get_icap_status(
	struct test_wlan_info *winfos,
	s_int32 *icap_stat)
{
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

#ifdef INTERNAL_CAPTURE_SUPPORT
	if (ops->ICapStatus != NULL)
		*icap_stat = ops->ICapStatus(ad);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s : The function is not hooked !!\n", __func__));
		return  SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
	}
#endif/* INTERNAL_CAPTURE_SUPPORT */

	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_get_icap_max_data_len(
	struct test_wlan_info *winfos,
	u_long *max_data_len)
{
	*max_data_len = (ICAP_EVENT_DATA_SAMPLE * sizeof(INT32));
	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_get_icap_data(
	struct test_wlan_info *winfos,
	s_int32 *icap_cnt,
	s_int32 *icap_data,
	u_int32 wf_num,
	u_int32 iq_type)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

#ifdef INTERNAL_CAPTURE_SUPPORT
	if (ops->ICapGetIQData != NULL)
		ret = ops->ICapGetIQData(ad
				, icap_data, icap_cnt, iq_type, wf_num);
	else if (ops->ICapCmdSolicitRawDataProc != NULL)
		ret = ops->ICapCmdSolicitRawDataProc(ad
				, icap_data, icap_cnt, iq_type, wf_num);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s : The function is not hooked !!\n", __func__));
	}
#endif/* INTERNAL_CAPTURE_SUPPORT */

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_do_cal_item(
	struct test_wlan_info *winfos,
	u_int32 item,
	u_char band_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	if (item == 0) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: Wrong input [%d] ! Check !\n", __func__, item));
		return SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
	}

	if (IS_ATE_DBDC(ad) == FALSE) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: MIMO item[0x%x]\n", __func__, item));
	} else {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: DBDC item[0x%x]\n", __func__, item));
	}

	if (item == (1<<25)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: 2G DNL+TSSI\n", __func__));
		ret = mt_op_set_test_mode_dnlk_2g(winfos);
	} else if (item == (1<<26)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: 5G DNL+TSSI\n", __func__));
		ret = mt_op_set_test_mode_dnlk_5g(winfos);
	} else {
		ret = MtCmdDoCalibration(ad, 0x1, item, band_idx);
	}

	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_band_mode(
	struct test_wlan_info *winfos,
	struct test_band_state *band_state)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_get_chipid(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	return ret;
}

s_int32 mt_op_mps_set_seq_data(
	struct test_wlan_info *winfos,
	u_int32 len,
	struct test_mps_setting *mps_setting)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	return ret;
}

s_int32 mt_op_get_tx_pwr(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx,
	u_char channel,
	u_char ant_idx,
	u_int32 *power)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	EXT_EVENT_ID_GET_TX_POWER_T txpwr_result;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdGetTxPower(ad, band_idx, channel, ant_idx, &txpwr_result);
	*power = (u_int32)txpwr_result.i1TargetPower;
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_tx_pwr(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx,
	struct test_txpwr_param *pwr_param)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	struct test_txpwr_cfg txpwr_cfg;
	ATE_TXPOWER txpwr;

	/* sanity check for null pointer */
	if (!pwr_param)
		return SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	txpwr_cfg.ant_idx = pwr_param->ant_idx;
	txpwr_cfg.txpwr = configs->tx_pwr[pwr_param->ant_idx];
	txpwr_cfg.channel = configs->channel;
	txpwr_cfg.band_idx = band_idx;
	txpwr_cfg.ch_band = configs->ch_band;

	sys_ad_zero_mem(&txpwr, sizeof(ATE_TXPOWER));
	sys_ad_move_mem(&txpwr, &txpwr_cfg, sizeof(ATE_TXPOWER));
	ret = MtCmdSetTxPowerCtrl(ad, txpwr);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_get_freq_offset(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 *freq_offset)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdGetFreqOffset(ad, band_idx, freq_offset);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_get_cfg_on_off(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 type,
	u_int32 *result)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdGetCfgOnOff(ad, type, band_idx, result);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_get_tx_tone_pwr(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 ant_idx,
	u_int32 *power)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdRfTestGetTxTonePower(ad, power, ant_idx, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_get_recal_cnt(
	struct test_wlan_info *winfos,
	u_int32 *recal_cnt,
	u_int32 *recal_dw_num)
{
	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_get_recal_content(
	struct test_wlan_info *winfos,
	u_int32 *content)
{
	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_get_rxv_cnt(
	struct test_wlan_info *winfos,
	u_int32 *rxv_cnt,
	u_int32 *rxv_dw_num)
{
	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_get_rxv_content(
	struct test_wlan_info *winfos,
	u_int32 dw_cnt,
	u_int32 *content)
{
	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_set_cal_bypass(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int32 cal_item)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdDoCalibration(ad, CALIBRATION_BYPASS, cal_item, band_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_dpd(
	struct test_wlan_info *winfos,
	u_int32 on_off,
	u_int32 wf_sel)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtAsicSetDPD(ad, on_off, wf_sel);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_tssi(
	struct test_wlan_info *winfos,
	u_int32 on_off,
	u_int32 wf_sel)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtAsicSetTSSI(ad, on_off, wf_sel);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_get_thermal_val(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx,
	u_int32 *value)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	/* 0: get temperature; 1: get adc */
	ret = MtCmdGetThermalSensorResult(ad, 0,
		band_idx, value);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_set_rdd_test(
	struct test_wlan_info *winfos,
	u_int32 rdd_idx,
	u_int32 rdd_sel,
	u_int32 enable)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: rdd_idx: %d, rdd_sel: %d, enable:%d\n",
		__func__, rdd_idx, rdd_sel, enable));

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = MtCmdSetRDDTestExt(ad, rdd_idx, rdd_sel, enable);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_evt_rf_test_cb(
	struct test_wlan_info *winfos,
	struct test_log_dump_cb *test_log_dump,
	u_int32 en_log,
	u_int8 *data,
	u_int32 length)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	ret = net_ad_rf_test_cb(winfos, test_log_dump, en_log, data, length);

	return ret;
}

s_int32 mt_op_set_off_ch_scan(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx,
	struct test_off_ch_param *param)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	EXT_CMD_OFF_CH_SCAN_CTRL_T ext_cmd_param;
	u_char ch = 0;
	u_char work_tx_strm_pth = 0, work_rx_strm_pth = 0, off_ch_idx = 0;
	u_char ch_ext_index = 0;
	u_char ch_ext_above[] = {
	36, 44, 52, 60,
	100, 108, 116, 124,
	132, 140, 149, 157, 0
	};
	u_char ch_ext_below[] = {
	40, 48, 56, 64,
	104, 112, 120, 128,
	136, 144, 153, 161, 0
	};
	u_char prim_ch[off_ch_ch_idx_num] = {0, 0};
	u_char bw[off_ch_ch_idx_num] = {0, 0};
	u_char cen_ch[off_ch_ch_idx_num] = {0, 0};

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s ", __func__));

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	work_tx_strm_pth = configs->tx_ant;
	work_rx_strm_pth = configs->rx_strm_pth;
	prim_ch[off_ch_wrk_ch_idx] = configs->channel;
	bw[off_ch_wrk_ch_idx] = configs->bw;
	prim_ch[off_ch_mntr_ch_idx] = param->mntr_ch;
	bw[off_ch_mntr_ch_idx] = param->mntr_bw;

	for (off_ch_idx = 0; off_ch_idx < off_ch_ch_idx_num; off_ch_idx++) {
		ch = prim_ch[off_ch_idx];

		/* Initialize index */
		ch_ext_index = 0;

		switch (bw[off_ch_idx]) {
		case TEST_BW_20:
			break;

		case TEST_BW_40:
			while (ch_ext_above[ch_ext_index] != 0) {
				if (ch == ch_ext_above[ch_ext_index])
					ch = ch + 2;
				else if (ch == ch_ext_below[ch_ext_index])
					ch = ch - 2;

				ch_ext_index++;
			}
			break;

		case TEST_BW_80:
		case TEST_BW_160NC:
			ch = vht_cent_ch_freq(ch, VHT_BW_80, CMD_CH_BAND_5G);
			break;

		case TEST_BW_160C:
			ch = vht_cent_ch_freq(ch, VHT_BW_160, CMD_CH_BAND_5G);
			break;

		default:
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
				("%s: off_ch_idx %d, BW is invalid %d\n",
				__func__, off_ch_idx, bw[off_ch_idx]));
			ret = NDIS_STATUS_FAILURE;
			goto err0;
		}

		cen_ch[off_ch_idx] = ch;
	}

	/* Initialize */
	sys_ad_zero_mem(&ext_cmd_param, sizeof(ext_cmd_param));

	/* Fill in ext_cmd_param */
	ext_cmd_param.mntr_prim_ch = param->mntr_ch;
	ext_cmd_param.mntr_cntrl_ch = cen_ch[off_ch_mntr_ch_idx];
	ext_cmd_param.mntr_bw = bw[off_ch_mntr_ch_idx];
	ext_cmd_param.mntr_tx_strm_pth = param->mntr_tx_rx_pth;
	ext_cmd_param.mntr_rx_strm_pth = param->mntr_tx_rx_pth;

	ext_cmd_param.work_prim_ch = prim_ch[off_ch_wrk_ch_idx];
	ext_cmd_param.work_cntrl_ch = cen_ch[off_ch_wrk_ch_idx];
	ext_cmd_param.work_bw = bw[off_ch_wrk_ch_idx];
	ext_cmd_param.work_tx_strm_pth = work_tx_strm_pth;
	ext_cmd_param.work_rx_strm_pth = work_rx_strm_pth;

	ext_cmd_param.dbdc_idx = param->dbdc_idx;
	ext_cmd_param.scan_mode = param->scan_mode;
	ext_cmd_param.is_aband = param->is_aband;
	ext_cmd_param.off_ch_scn_type = off_ch_scan_simple_rx;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: mntr_ch:%d mntr_bw:%d mntr_central_ch:%d\n",
		__func__, ext_cmd_param.mntr_prim_ch,
		ext_cmd_param.mntr_bw, ext_cmd_param.mntr_cntrl_ch));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: work_prim_ch:%d work_bw:%d work_central_ch:%d\n",
		__func__, ext_cmd_param.work_prim_ch,
		ext_cmd_param.work_bw, ext_cmd_param.work_cntrl_ch));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: scan_mode:%d dbdc_idx:%d is_aband:%d\n",
		__func__, ext_cmd_param.scan_mode,
		ext_cmd_param.dbdc_idx, ext_cmd_param.is_aband));

	ret = mt_cmd_off_ch_scan(ad, &ext_cmd_param);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;

err0:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: invalid parameters\n", __func__));
	return ret;
}

s_int32 mt_op_get_rdd_cnt(
	struct test_log_dump_cb *log_cb,
	u_int32 *rdd_cnt,
	u_int32 *rdd_dw_num)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s:\n", __func__));

	/* radar pulse number */
	*rdd_cnt = log_cb->idx;
	/* RDD buffer size */
	*rdd_dw_num = log_cb->len;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: radar pulse number is %d, RDD buffer size is %d\n",
		__func__, log_cb->idx, log_cb->len));

	return ret;
}

s_int32 mt_op_get_rdd_content(
	struct test_log_dump_cb *log_cb,
	u_int32 *content,
	u_int32 *total_cnt)
{
	static u_int32 idx;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 end = 0, remindIdx = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s:\n", __func__));

	if (log_cb == NULL) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: log_cb == NULL\n", __func__));
		return SERV_STATUS_HAL_OP_INVALID_PAD;
	}

	if (!log_cb->entry) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: log_cb->entry == NULL\n", __func__));
		return SERV_STATUS_HAL_OP_INVALID_PAD;
	}

	if (log_cb->len == 0) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: RDD buffer size is empty\n", __func__));
		return SERV_STATUS_HAL_OP_INVALID_PAD;
	}

	/* Prepare for RDD dump */
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("[RDD DUMP START][HQA_GetDumpRDD]\n"));

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: radar pulse number is %d, RDD buffer size is %d\n",
		__func__, log_cb->idx, log_cb->len));

	if (log_cb->first_en == TRUE) {
		/*
		 * Reset idx - 1. HQA RDD dump (re-)enable
		 */
		idx = 0;
	}

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("idx: %d\n", idx));
	/* If log_cb->idx greater than log_cb->len(RDDBufferSize),
	 * it will re-count from 0
	 */
	remindIdx = (idx > log_cb->idx) ?
		((log_cb->idx + log_cb->len) - idx) :
		(log_cb->idx - idx);

	end = (remindIdx > TEST_RDD_DUMP_SIZE) ?
		((idx + TEST_RDD_DUMP_SIZE) % (log_cb->len)) :
		((idx + remindIdx) % (log_cb->len));

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("remindIdx: %d, end: %d\n", remindIdx, end));

	do {
		idx = (idx % (log_cb->len));
		if (log_cb->entry[idx].un_dumped) {
			struct test_rdd_log *result =
				&log_cb->entry[idx].log.rdd;
			/* 1 pulse: 64 bits */
			UINT32 *pulse =
				(UINT32 *)result->buffer;

			/* To sperate the interrupts of radar signals */
			if (!result->by_pass) {
				*content = result->prefix;
				content++;
				*content = result->cnt;
				content++;
				*total_cnt = *total_cnt + 2;
			}

			log_cb->entry[idx].un_dumped = FALSE;

			*content = pulse[0];
			content++;
			*content = pulse[1];
			content++;
			*total_cnt = *total_cnt + 2;
			SERV_LOG(SERV_DBG_CAT_ALL, SERV_DBG_LVL_TRACE,
				("[RDD]0x%08x %08x\n", pulse[0], pulse[1]));
		}

		INC_RING_INDEX(idx, log_cb->len);
	} while (idx != end);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("[After RDD dumping] idx: %d, end: %d, total_cnt: %d\n",
		idx, end, *total_cnt));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("[%s] idx = %d, log_cb->idx = %d\n",
		__func__, idx, log_cb->idx));

	if (idx == log_cb->idx) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("[RDD DUMP END]\n"));
		idx = 0;
		log_cb->idx = 0;
	}

	SERV_OS_SEM_LOCK(&log_cb->lock);
	log_cb->is_dumping = FALSE;
	SERV_OS_SEM_UNLOCK(&log_cb->lock);

	log_cb->first_en = FALSE;

	return ret;
}

s_int32 mt_op_set_muru_manual(
	void *virtual_device,
	struct test_wlan_info *winfos,
	struct test_configuration *configs)
{
#if defined(CFG_SUPPORT_FALCON_MURU)
	RTMP_ADAPTER *ad = NULL;
	u_int8 ru_seq = 0, wmm_idx = 0;
	CMD_MURU_MANCFG_INTERFACER MuruManCfg;
	struct _MAC_TABLE_ENTRY *mac_tbl_entry = NULL;
	struct test_tx_stack *stack = &configs->stack;
	struct test_ru_info *ru_info = &configs->ru_info_list[0];
	u_int8 spe_idx = 0;
	u_int32 ant_sel = configs->tx_ant;

	if (ant_sel & TEST_ANT_USER_DEF) {
		ant_sel &= ~TEST_ANT_USER_DEF;
		spe_idx = ant_sel;
	} else {
		net_ad_get_speidx(winfos, ant_sel, &spe_idx);
	}

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	mac_tbl_entry = (struct _MAC_TABLE_ENTRY *)stack->virtual_wtbl[0];

	sys_ad_zero_mem(&MuruManCfg, sizeof(MuruManCfg));

	MuruManCfg.rCfgCmm.u1PpduFmt = MURU_PPDU_HE_MU;
	MuruManCfg.u4ManCfgBmpCmm = MURU_FIXED_CMM_PPDU_FMT;
	net_ad_get_band_idx(virtual_device, &MuruManCfg.rCfgCmm.u1Band);
	MuruManCfg.u4ManCfgBmpCmm |= MURU_FIXED_CMM_BAND;
	net_ad_get_wmm_idx(virtual_device, &wmm_idx);
	MuruManCfg.rCfgCmm.u1WmmSet = wmm_idx;
	MuruManCfg.u4ManCfgBmpCmm |= MURU_FIXED_CMM_WMM_SET;
	MuruManCfg.rCfgCmm.u1SpeIdx = spe_idx;
	MuruManCfg.u4ManCfgBmpCmm |= MURU_FIXED_CMM_SPE_IDX;
	if (configs->per_pkt_bw > TEST_BW_80)
		MuruManCfg.rCfgDl.u1Bw = 0x3;	/* 0x3 imply 80+80/160 */
	else
		MuruManCfg.rCfgDl.u1Bw = configs->per_pkt_bw;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_BW;
	MuruManCfg.rCfgDl.u1SigBMcs = (configs->mcs & 0xf);
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_SIGB_MCS;
	MuruManCfg.rCfgDl.u1SigBDcm = ((configs->mcs & BIT5) ? 0x1 : 0);
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_SIGB_DCM;
	MuruManCfg.rCfgDl.u1TxMode = configs->tx_mode;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_TX_MODE;
	MuruManCfg.rCfgDl.u1UserCnt = stack->index;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_TOTAL_USER_CNT;
	sys_ad_move_mem(&MuruManCfg.rCfgDl.au1RU,
					&configs->ru_alloc,
					sizeof(configs->ru_alloc));
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_TONE_PLAN;
	MuruManCfg.rCfgDl.u1GI = configs->stack.tx_info[0].gi;
	MuruManCfg.rCfgDl.u1Ltf = configs->stack.tx_info[0].ltf;
	MuruManCfg.u4ManCfgBmpDl |= (MURU_FIXED_GI | MURU_FIXED_LTF);
	for (ru_seq = 0 ; ru_seq < MAX_MULTI_TX_STA ; ru_seq++) {
		u_int8 seg = 0, alloc = 0;
		MURU_DL_USER_INFO *user_info = NULL;
		struct phy_params *phy_param = NULL;

		if (ru_info[ru_seq].valid) {
			user_info = &MuruManCfg.rCfgDl.arUserInfoDl[ru_seq];
			seg = (ru_info[ru_seq].ru_index & 0x1);
			alloc = (ru_info[ru_seq].ru_index >> 1);
			phy_param = &mac_tbl_entry[ru_seq].phy_param;

			user_info->u2WlanIdx = mac_tbl_entry[ru_seq].wcid;
			user_info->u1RuAllocBn = seg;
			user_info->u1RuAllocIdx = alloc;
			user_info->u1Mcs = (phy_param->rate & 0xf);
			user_info->u2TxPwrAlpha =
					(ru_info[ru_seq].alpha & 0xffff);

			if (phy_param->rate & BIT5)	/* DCM required */
				user_info->u1Mcs |= BIT4;

			user_info->u1Nss = phy_param->vht_nss-1;
			user_info->u1Ldpc = phy_param->ldpc;

			if ((ru_info[ru_seq].ru_index >> 1) == 18) {
				u_int8 *au1C26 = NULL;

				au1C26 = MuruManCfg.rCfgDl.au1C26;
				au1C26[(ru_info[ru_seq].ru_index & 1)] = 1;
			}
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
				("%s: Add user[%d] wcid:%d, ru index:%d,\n",
				__func__, ru_seq, user_info->u2WlanIdx,
				user_info->u1RuAllocIdx));
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
				("\t\tsegment:%d, mcs:%d\n",
				user_info->u1RuAllocBn,
				user_info->u1Mcs));
		}
	}
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_USER_WLAN_ID;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_USER_COD;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_USER_MCS;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_USER_NSS;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_USER_RU_ALLOC;
	MuruManCfg.u4ManCfgBmpDl |= MURU_FIXED_USER_PWR_ALPHA;

	wifi_test_muru_set_manual_config(ad, &MuruManCfg);
#endif	/* CFG_SUPPORT_FALCON_MURU */

	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_set_tam_arb(
	struct test_wlan_info *winfos,
	u_int8 arb_op_mode)
{
#if defined(CFG_SUPPORT_FALCON_MURU)
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	wifi_test_muru_set_arb_op_mode(ad, arb_op_mode);
#endif /* CFG_SUPPORT_FALCON_MURU */

	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_set_mu_count(
	struct test_wlan_info *winfos,
	void *virtual_device,
	u_int32 count)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#if defined(CFG_SUPPORT_FALCON_MURU)
	RTMP_ADAPTER *ad = NULL;
	CMD_MURU_SET_MU_TX_PKT_CNT SetMuTxPktCnt;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL) {
		ret = SERV_STATUS_HAL_OP_INVALID_PAD;
		goto err_out;
	}

	ret = net_ad_get_band_idx(virtual_device, &SetMuTxPktCnt.u1BandIdx);
	if (ret)
		goto err_out;
	/* u4MuTxPktCnt set to 0 as continuously */
	if (count != 0x8fffffff)
		SetMuTxPktCnt.u4MuTxPktCnt = count;
	else
		SetMuTxPktCnt.u4MuTxPktCnt = 0;

	if (set_muru_mu_tx_pkt_cnt(ad, &SetMuTxPktCnt) == FALSE) {
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
		goto err_out;
	}

err_out:
#endif /* CFG_SUPPORT_FALCON_MURU */
	return ret;
}

s_int32 mt_op_trigger_mu_counting(
	struct test_wlan_info *winfos,
	void *virtual_device,
	boolean enable)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#if defined(CFG_SUPPORT_FALCON_MURU)
	RTMP_ADAPTER *ad = NULL;
	CMD_MURU_SET_MU_TX_PKT_CNT SetMuTxPktCnt;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL) {
		ret = SERV_STATUS_HAL_OP_INVALID_PAD;
		goto err_out;
	}

	sys_ad_zero_mem(&SetMuTxPktCnt, sizeof(SetMuTxPktCnt));

	ret = net_ad_get_band_idx(virtual_device, &SetMuTxPktCnt.u1BandIdx);
	if (ret)
		goto err_out;

	SetMuTxPktCnt.u1MuTxEn = enable;
	if (set_muru_mu_tx_pkt_en(ad, &SetMuTxPktCnt) == FALSE) {
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
		goto err_out;
	}

err_out:
#endif /* CFG_SUPPORT_FALCON_MURU */
	return ret;
}

s_int32 mt_op_get_rx_stat_band(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int8 blk_idx,
	struct test_rx_stat_band_info *rx_st_band)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	TEST_RX_STAT_BAND_INFO st;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = chip_get_rx_stat_band(ad, band_idx, blk_idx, &st);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	rx_st_band->mac_rx_fcs_err_cnt = st.mac_rx_fcs_err_cnt;
	rx_st_band->mac_rx_mdrdy_cnt = st.mac_rx_mdrdy_cnt;
	rx_st_band->mac_rx_len_mismatch = st.mac_rx_len_mismatch;
	rx_st_band->mac_rx_fcs_ok_cnt = st.mac_rx_fcs_ok_cnt;
	rx_st_band->phy_rx_fcs_err_cnt_cck = st.phy_rx_fcs_err_cnt_cck;
	rx_st_band->phy_rx_fcs_err_cnt_ofdm = st.phy_rx_fcs_err_cnt_ofdm;
	rx_st_band->phy_rx_pd_cck = st.phy_rx_pd_cck;
	rx_st_band->phy_rx_pd_ofdm = st.phy_rx_pd_ofdm;
	rx_st_band->phy_rx_sig_err_cck = st.phy_rx_sig_err_cck;
	rx_st_band->phy_rx_sfd_err_cck = st.phy_rx_sfd_err_cck;
	rx_st_band->phy_rx_sig_err_ofdm = st.phy_rx_sig_err_ofdm;
	rx_st_band->phy_rx_tag_err_ofdm = st.phy_rx_tag_err_ofdm;
	rx_st_band->phy_rx_mdrdy_cnt_cck = st.phy_rx_mdrdy_cnt_cck;
	rx_st_band->phy_rx_mdrdy_cnt_ofdm = st.phy_rx_mdrdy_cnt_ofdm;

	return ret;
}

s_int32 mt_op_get_rx_stat_path(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int8 blk_idx,
	struct test_rx_stat_path_info *rx_st_path)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	TEST_RX_STAT_PATH_INFO st;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = chip_get_rx_stat_path(ad, band_idx, blk_idx, &st);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	rx_st_path->rcpi = st.rcpi;
	rx_st_path->rssi = st.rssi;
	rx_st_path->fagc_ib_rssi = st.fagc_ib_rssi;
	rx_st_path->fagc_wb_rssi = st.fagc_wb_rssi;
	rx_st_path->inst_ib_rssi = st.inst_ib_rssi;
	rx_st_path->inst_wb_rssi = st.inst_wb_rssi;

	return ret;
}

s_int32 mt_op_get_rx_stat_user(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int8 blk_idx,
	struct test_rx_stat_user_info *rx_st_user)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	TEST_RX_STAT_USER_INFO st;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = chip_get_rx_stat_user(ad, band_idx, blk_idx, &st);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	rx_st_user->freq_offset_from_rx = st.freq_offset_from_rx;
	rx_st_user->snr = st.snr;
	rx_st_user->fcs_error_cnt = st.fcs_error_cnt;

	return ret;
}

s_int32 mt_op_get_rx_stat_comm(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int8 blk_idx,
	struct test_rx_stat_comm_info *rx_st_comm)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	TEST_RX_STAT_COMM_INFO st;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = chip_get_rx_stat_comm(ad, band_idx, blk_idx, &st);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	rx_st_comm->rx_fifo_full = st.rx_fifo_full;
	rx_st_comm->aci_hit_low = st.aci_hit_low;
	rx_st_comm->aci_hit_high = st.aci_hit_high;
	rx_st_comm->mu_pkt_count = st.mu_pkt_count;
	rx_st_comm->sig_mcs = st.sig_mcs;
	rx_st_comm->sinr = st.sinr;
	rx_st_comm->driver_rx_count = st.driver_rx_count;

	return ret;
}

s_int32 mt_op_set_rx_user_idx(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int16 user_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = mt_cmd_set_rx_stat_user_idx(ad, band_idx, user_idx);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_get_wf_path_comb(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	boolean dbdc_mode_en,
	u_int8 *path,
	u_int8 *path_len)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ret = chip_get_wf_path_comb(ad, band_idx, dbdc_mode_en, path, path_len);
	if (ret)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;

	return ret;
}

s_int32 mt_op_hetb_ctrl(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_char ctrl_type,
	boolean enable,
	u_int8 bw,
	u_int8 ltf_gi,
	u_int8 stbc,
	u_int8 pri_ru_idx,
	struct test_ru_info *ru_info)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#if defined(DOT11_HE_AX)
	RTMP_ADAPTER *ad = NULL;
	struct _RTMP_CHIP_DBG *chip_dbg = NULL;
	struct _ATE_RU_STA *ru = (struct _ATE_RU_STA *)ru_info;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	chip_dbg = hc_get_chip_dbg(ad->hdev_ctrl);

	if (ctrl_type == OP_HETB_RX_CFG) {
		u_int64 hetb_rx_csd = 0x240004000060FF;

		if (chip_dbg->ctrl_manual_hetb_rx)
			chip_dbg->ctrl_manual_hetb_rx(ad,
						band_idx,
						enable,
						bw,
						ltf_gi,
						stbc,
						hetb_rx_csd,
						&ru[pri_ru_idx],
						ru);
		else
			ret = SERV_STATUS_HAL_OP_FAIL;
	} else {
		if (chip_dbg->ctrl_manual_hetb_tx)
			chip_dbg->ctrl_manual_hetb_tx(ad,
					band_idx,
					ctrl_type,
					bw,
					ltf_gi,
					stbc,
					ru);
		else
			ret = SERV_STATUS_HAL_OP_FAIL;
	}
#endif	/* DOT11_HE_AX */
	return ret;
}

s_int32 mt_op_set_ru_aid(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_int16 mu_rx_aid)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
#ifdef CONFIG_HW_HAL_OFFLOAD
	RTMP_ADAPTER *ad = NULL;
	struct _EXT_CMD_ATE_TEST_MODE_T param;
	u_int8 testmode_en = 1;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	sys_ad_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = testmode_en;
	param.ucAteIdx = ENUM_ATE_SET_MU_RX_AID;
	param.Data.set_mu_rx_aid.band_idx = band_idx;
	param.Data.set_mu_rx_aid.aid = cpu2le16(mu_rx_aid);

	param.aucReserved[1] = INIT_CMD_SET_AND_WAIT_RETRY_RSP;
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: Set to decode MU accodring to AID:%d\n",
		__func__, param.Data.set_mu_rx_aid.aid));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("\t\t(%d means disable)\n", 0xf800));
	ret = MtCmdATETest(ad, &param);

	if (ret != 0)
		ret = SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD;
#endif

	return ret;
}

s_int32 mt_op_set_mutb_spe(
	struct test_wlan_info *winfos,
	u_char band_idx,
	u_char tx_mode,
	u_int8 spe_idx)
{
	RTMP_ADAPTER *ad = NULL;
	struct _RTMP_CHIP_DBG *chip_dbg = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	chip_dbg = hc_get_chip_dbg(ad->hdev_ctrl);
	if (chip_dbg->chip_ctrl_spe)
		chip_dbg->chip_ctrl_spe(ad, band_idx, tx_mode, spe_idx);

	return SERV_STATUS_SUCCESS;
}

s_int32 mt_op_set_test_mode_dnlk_clean(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	/* Clear DNLK Image */
	ad->DnlCalOfst = 0;
	sys_ad_zero_mem(ad->TxDnlCal, DNL_CAL_SIZE);

	/* Clear TSSI Image */
	ad->TssiCal2GOfst = 0;
	sys_ad_zero_mem(ad->TssiCal2G, TSSI_CAL_2G_SIZE);
	ad->TssiCal5GOfst = 0;
	sys_ad_zero_mem(ad->TssiCal5G, TSSI_CAL_5G_SIZE);

	return ret;
}

s_int32 mt_op_set_test_mode_dnlk_2g(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int8  i = 0;
	MT_SWITCH_CHANNEL_CFG ch_cfg;
	RTMP_ADAPTER *ad = NULL;
	struct _ATE_CTRL *ATECtrl = NULL;
#ifdef DBDC_MODE
	u_int32 band0_tx_path_backup = 0;
	u_int32 band0_rx_path_backup = 0;
#endif

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ATECtrl = &(ad->ATECtrl);

	ad->DnlCalOfst = 0;
	ad->TssiCal2GOfst = 0;

	/* 2.4G DNL Calibration */
	sys_ad_zero_mem(&ch_cfg, sizeof(ch_cfg));

	ATECtrl->op_mode |= fATE_IN_RFTEST;

	/* Execute 2.4G DNL + TSSI Calibration */
	/* If want to debug, we can use only ch6 to verify */
	/* and need pay attention to the index */
	for (i = 0; i < MT7915_DNL_CAL_GBAND_BW20_CH_SIZE; i++) {
		/* set channel command */
		/* per group calibration - set to channel 1, 7, 13, BW20 */
		ch_cfg.Bw = BW_20;
		ch_cfg.CentralChannel = MT7915_DNL_CAL_GBAND_BW20_CH[i];
		ch_cfg.ControlChannel = MT7915_DNL_CAL_GBAND_BW20_CH[i];
		ch_cfg.ControlChannel2 = 0;
		ch_cfg.bScan = 0;
		ch_cfg.BandIdx = 0;
		ch_cfg.bDnlCal = TRUE;

		/* Sw Ch in Test Mode */
		/* T/Rx number bring T/Rx path bit-wise */
		if (IS_ATE_DBDC(ad) == FALSE) {
			ch_cfg.TxStream = 0xF;
			ch_cfg.RxStream = 0xF;
		} else {
			ch_cfg.TxStream = 0x3;
			ch_cfg.RxStream = 0x3;
#ifdef DBDC_MODE
			band0_tx_path_backup = ad->dbdc_band0_tx_path;
			band0_rx_path_backup = ad->dbdc_band0_rx_path;
			ad->dbdc_band0_tx_path = ch_cfg.TxStream;
			ad->dbdc_band0_rx_path = ch_cfg.RxStream;
#endif
		}
		MtCmdChannelSwitch(ad, ch_cfg);
		if (IS_ATE_DBDC(ad)) {
#ifdef DBDC_MODE
			ad->dbdc_band0_tx_path = band0_tx_path_backup;
			ad->dbdc_band0_rx_path = band0_rx_path_backup;
#endif
		}

		/* T/Rx Path in Test Mode */
		/* T/Rx number bring T/Rx path bit-wise */
		if (IS_ATE_DBDC(ad) == FALSE) {
			ch_cfg.TxStream = 0xF;
			ch_cfg.RxStream = 0xF;
		} else {
			ch_cfg.TxStream = 2;
			ch_cfg.RxStream = 0x3;
		}
		MtCmdSetTxRxPath(ad, ch_cfg);
	}

	/* Get DNL Calibration result */
	MtCmdDoCalibration(ad, RE_CALIBRATION, (1<<27), 0);

	/* Get TSSI 2G Calibration result */
	MtCmdDoCalibration(ad, RE_CALIBRATION, (1<<25), 0);

	ATECtrl->op_mode &= ~fATE_IN_RFTEST;

	return ret;
}

s_int32 mt_op_set_test_mode_dnlk_5g(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int8  i = 0;
	MT_SWITCH_CHANNEL_CFG ch_cfg;
	RTMP_ADAPTER *ad = NULL;
	struct _ATE_CTRL *ATECtrl = NULL;
#ifdef DBDC_MODE
	u_int32 band1_tx_path_backup = 0;
	u_int32 band1_rx_path_backup = 0;
#endif

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ATECtrl = &(ad->ATECtrl);

	ad->DnlCalOfst = 0;
	ad->TssiCal5GOfst = 0;

	/* 5G DNL Calibration */
	sys_ad_zero_mem(&ch_cfg, sizeof(ch_cfg));

	ATECtrl->op_mode |= fATE_IN_RFTEST;

	/* Execute 5G DNL + TSSI Calibration */
	/* If want to debug, we can use only ch36 to verify */
	/* and need pay attention to the index */
	for (i = 0; i < MT7915_DNL_CAL_ABAND_BW20_CH_SIZE; i++) {
		/* set channel command */
		/* per group calibration - set to channel 36, 52, BW20 */
		ch_cfg.Bw = BW_20;
		ch_cfg.CentralChannel = MT7915_DNL_CAL_ABAND_BW20_CH[i];
		ch_cfg.ControlChannel = MT7915_DNL_CAL_ABAND_BW20_CH[i];
		ch_cfg.ControlChannel2 = 0;
		ch_cfg.bScan = 0;
		ch_cfg.Channel_Band = 1;
		ch_cfg.bDnlCal = TRUE;

		/* Sw Ch in Test Mode */
		/* T/Rx number bring T/Rx path bit-wise */
		if (IS_ATE_DBDC(ad) == FALSE) {
			ch_cfg.TxStream = 0xF;
			ch_cfg.RxStream = 0xF;
			ch_cfg.BandIdx = 0;
		} else {
			ch_cfg.TxStream = 0xC;
			ch_cfg.RxStream = 0xC;
			ch_cfg.BandIdx = 1;
#ifdef DBDC_MODE
			band1_tx_path_backup = ad->dbdc_band1_tx_path;
			band1_rx_path_backup = ad->dbdc_band1_rx_path;
			ad->dbdc_band1_tx_path = ch_cfg.TxStream;
			ad->dbdc_band1_rx_path = ch_cfg.RxStream;
#endif
		}
		MtCmdChannelSwitch(ad, ch_cfg);
		if (IS_ATE_DBDC(ad)) {
#ifdef DBDC_MODE
			ad->dbdc_band1_tx_path = band1_tx_path_backup;
			ad->dbdc_band1_rx_path = band1_rx_path_backup;
#endif
		}

		/* T/Rx Path in Test Mode */
		/* T/Rx number bring T/Rx path bit-wise */
		if (IS_ATE_DBDC(ad) == FALSE) {
			ch_cfg.TxStream = 0xF;
			ch_cfg.RxStream = 0xF;
		} else {
			ch_cfg.TxStream = 2;
			ch_cfg.RxStream = 0xC;
		}
		MtCmdSetTxRxPath(ad, ch_cfg);
	}

	/* Get DNL Calibration result */
	MtCmdDoCalibration(ad, RE_CALIBRATION, (1<<27), 0);

	/* Get TSSI 5G Calibration result */
	MtCmdDoCalibration(ad, RE_CALIBRATION, (1<<26), 0);

	ATECtrl->op_mode &= ~fATE_IN_RFTEST;

	return ret;
}

s_int32 mt_op_group_prek(
	struct test_wlan_info *winfos,
	u_char op)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	if (op < 0 || op >= PREK_GROUP_OP_NUM) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: Invalid op\n", __func__));
		return SERV_STATUS_HAL_OP_FAIL;
	}
#ifdef PRE_CAL_MT7915_SUPPORT
	ret = MtATE_Group_Pre_Cal_Store_Proc_7915(ad, op);
#else
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: PRE_CAL not enable!\n", __func__));
#endif

	/* MtATE_Group_Pre_Cal_Store_Proc retrun 1 if success, so flip the bit */
	ret = ret ^ 1;
	return ret;
}

s_int32 mt_op_dpd_prek(
	struct test_wlan_info *winfos,
	u_char op)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;

	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	if (op < 0 || op >= PREK_DPD_OP_NUM) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: Invalid op\n", __func__));
		return SERV_STATUS_HAL_OP_FAIL;
	}
#ifdef PRE_CAL_MT7915_SUPPORT
	ret = MtATE_DPD_Cal_Store_Proc_7915(ad, op);
#else
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: PRE_CAL not enable!\n", __func__));
#endif
	/* MtATE_DPD_Cal_Store_Proc retrun 1 if success, so flip the bit */
	ret = ret ^ 1;
	return ret;
}


#ifdef TXBF_SUPPORT
s_int32 mt_op_set_ibf_phase_cal_e2p_update(
	struct test_wlan_info *winfos,
	u_char group_idx,
	boolean fgSx2,
	u_char update_type)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->iBFPhaseCalE2PUpdate != NULL) {
		ad->fgCalibrationFail = FALSE; /* Enable EEPROM write */
						/* of calibrated phase */

		/* Bit0   : BW160 ? */
		/* Bit1~3 : reserved */
		/* Bit4~5 : 0(Clean all), 1(Clean 2G iBF E2p only),*/
				/*2(Clean 5G iBF E2p only)*/
		/* Bit6~7 : reserved */
		switch (fgSx2 >> 4) {
		case CLEAN_ALL:
			ad->u1IbfCalPhase2G5GE2pClean = 0; /* Clean all */
			break;
		case CLEAN_2G:
			ad->u1IbfCalPhase2G5GE2pClean = 1; /* Clean 2G */
			break;
		case CLEAN_5G:
			ad->u1IbfCalPhase2G5GE2pClean = 2; /* Clean 5G */
			break;
		default:
			ad->u1IbfCalPhase2G5GE2pClean = 0; /* Clean all */
			break;
		}

		ops->iBFPhaseCalE2PUpdate(ad, group_idx, fgSx2, update_type);
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s : The function is not hooked !!\n", __func__));

		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;
	}

	return ret;
}

s_int32 mt_op_set_ibf_phase_cal_init(
	struct test_wlan_info *winfos)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->iBFPhaseCalInit)
		ops->iBFPhaseCalInit(ad);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}

s_int32 mt_op_set_wite_txbf_pfmu_tag(
	struct test_wlan_info *winfos, u_char prf_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->write_txbf_pfmu_tag)
		ops->write_txbf_pfmu_tag(ad->hdev_ctrl, prf_idx);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_invalid(
	struct test_wlan_info *winfos, boolean fg_invalid)
{
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		return ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG1_INVALID,
					fg_invalid);
	else
		return SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;
}


s_int32 mt_op_set_txbf_pfmu_tag_idx(
	struct test_wlan_info *winfos, u_char pfmu_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG1_PFMU_ID,
					pfmu_idx);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_bf_type(
	struct test_wlan_info *winfos, u_char bf_type)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG1_IEBF,
					bf_type);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_dbw(
	struct test_wlan_info *winfos, u_char dbw)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG1_DBW,
					dbw);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_sumu(
	struct test_wlan_info *winfos, u_char su_mu)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG1_SU_MU,
					su_mu);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_get_wrap_ibf_cal_ibf_mem_alloc(
	struct test_wlan_info *winfos,
	u_char *pfmu_mem_row,
	u_char *pfmu_mem_col)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->iBfCaliBfPfmuMemAlloc)
		ops->iBfCaliBfPfmuMemAlloc(ad, pfmu_mem_row, pfmu_mem_col);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_get_wrap_ibf_cal_ebf_mem_alloc(
	struct test_wlan_info *winfos,
	u_char *pfmu_mem_row,
	u_char *pfmu_mem_col)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->iBfCaleBfPfmuMemAlloc)
		ops->iBfCaleBfPfmuMemAlloc(ad, pfmu_mem_row, pfmu_mem_col);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_mem(
	struct test_wlan_info *winfos,
	u_char *pfmu_mem_row,
	u_char *pfmu_mem_col)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag) {
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_ROW0,
				pfmu_mem_row[0]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_ROW1,
				pfmu_mem_row[1]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_ROW2,
				pfmu_mem_row[2]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_ROW3,
				pfmu_mem_row[3]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_COL0,
				pfmu_mem_col[0]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_COL1,
				pfmu_mem_col[1]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_COL2,
				pfmu_mem_col[2]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_MEM_COL3,
				pfmu_mem_col[3]);
	} else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_matrix(
	struct test_wlan_info *winfos,
	u_char nr, u_char nc, u_char ng, u_char lm, u_char cb, u_char he)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag) {
		ops->set_txbf_pfmu_tag(ad->hdev_ctrl, TAG1_NR, nr);
		ops->set_txbf_pfmu_tag(ad->hdev_ctrl, TAG1_NC, nc);
		ops->set_txbf_pfmu_tag(ad->hdev_ctrl, TAG1_NG, ng);
		ops->set_txbf_pfmu_tag(ad->hdev_ctrl, TAG1_LM, lm);
		ops->set_txbf_pfmu_tag(ad->hdev_ctrl, TAG1_CODEBOOK, cb);
		ops->set_txbf_pfmu_tag(ad->hdev_ctrl, TAG1_HTC, he);
	} else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_snr(
	struct test_wlan_info *winfos,
	u_char *snr_sts)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag) {
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS0,
				snr_sts[0]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS1,
				snr_sts[1]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS2,
				snr_sts[2]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS3,
				snr_sts[3]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS4,
				snr_sts[4]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS5,
				snr_sts[5]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS6,
				snr_sts[6]);
		ops->set_txbf_pfmu_tag(
				ad->hdev_ctrl,
				TAG1_SNR_STS7,
				snr_sts[7]);
	} else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_smart_ant(
	struct test_wlan_info *winfos,
	u_int32 smart_ant)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG2_SMART_ANT,
					smart_ant);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_se_idx(
	struct test_wlan_info *winfos,
	u_char se_idx)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG2_SE_ID,
					se_idx);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_rmsd_thrd(
	struct test_wlan_info *winfos,
	u_char rmsd_thrd)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG2_RMSD_THRESHOLD,
					rmsd_thrd);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_time_out(
	struct test_wlan_info *winfos,
	u_char time_out)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG2_IBF_TIMEOUT,
					time_out);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_desired_bw(
	struct test_wlan_info *winfos,
	u_char desired_bw)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG2_IBF_DBW,
					desired_bw);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_desired_nr(
	struct test_wlan_info *winfos,
	u_char desired_nr)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG2_IBF_NROW,
					desired_nr);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_tag_desired_nc(
	struct test_wlan_info *winfos,
	u_char desired_nc)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_txbf_pfmu_tag)
		ret = ops->set_txbf_pfmu_tag(
					ad->hdev_ctrl,
					TAG2_IBF_NCOL,
					desired_nc);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_txbf_pfmu_data_write(
	struct test_wlan_info *winfos,
	u_int16 *angle_input)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->write_txbf_profile_data)
		ret = ops->write_txbf_profile_data(ad, angle_input);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}


s_int32 mt_op_set_manual_assoc(
	struct test_wlan_info *winfos,
	u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	RTMP_ADAPTER *ad = NULL;
	RTMP_CHIP_OP *ops = NULL;


	/* Get adapter from jedi driver first */
	GET_PAD_FROM_NET_DEV(ad, winfos->net_dev);
	if (ad == NULL)
		return SERV_STATUS_HAL_OP_INVALID_PAD;

	ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->set_manual_assoc)
		ops->set_manual_assoc(ad, arg);
	else
		ret = SERV_STATUS_HAL_OP_INVALID_NULL_POINTER;

	return ret;
}

#endif /* TXBF_SUPPORT */

