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
#include "mgmt/be_internal.h"
#include "hdev/hdev.h"

/*
* define  constructor & deconstructor & method
*/
VOID phy_oper_init(struct wifi_dev *wdev, struct phy_op *obj)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#ifdef DBDC_MODE
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	RTMP_CHIP_CAP *pCap = hc_get_chip_cap(ctrl);
#endif
	UINT8 ucTxPath = ad->Antenna.field.TxPath;
	UINT8 ucRxPath = ad->Antenna.field.RxPath;

#ifdef DBDC_MODE
	UINT8 band_idx = HcGetBandByWdev(wdev);
	UINT8 max_nss;

	if (ad->CommonCfg.dbdc_mode) {

		if (band_idx == DBDC_BAND0) {
			max_nss = pCap->mcs_nss.max_nss[0];
			ucTxPath = max_nss < pCap->mcs_nss.max_path[DBDC_BAND0][MAX_PATH_TX]
						? max_nss : pCap->mcs_nss.max_path[DBDC_BAND0][MAX_PATH_TX];
			ucRxPath = max_nss < pCap->mcs_nss.max_path[DBDC_BAND0][MAX_PATH_RX]
						? max_nss : pCap->mcs_nss.max_path[DBDC_BAND0][MAX_PATH_RX];
		} else {
			max_nss = pCap->mcs_nss.max_nss[1];
			ucTxPath = max_nss < pCap->mcs_nss.max_path[DBDC_BAND1][MAX_PATH_TX]
						? max_nss : pCap->mcs_nss.max_path[DBDC_BAND1][MAX_PATH_TX];
			ucRxPath = max_nss < pCap->mcs_nss.max_path[DBDC_BAND1][MAX_PATH_RX]
						? max_nss : pCap->mcs_nss.max_path[DBDC_BAND1][MAX_PATH_RX];
		}
		MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Swap TX/RX Stream number to (%d,%d) since DBDC_MODE EN\n",
				 ucTxPath, ucRxPath);

	}
#endif

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		UINT8 BandIdx = HcGetBandByWdev(wdev);
		if (ad->bAntennaSetAPEnable[BandIdx]) {
			ucTxPath = ad->TxStream[BandIdx];
			ucRxPath = ad->RxStream[BandIdx];
		}
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

	obj->wdev_bw = BW_20;
	obj->tx_stream = wlan_config_get_tx_stream(wdev);
	obj->rx_stream = wlan_config_get_rx_stream(wdev);

	MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "obj->tx_stream = %d, obj->rx_stream = %d\n",
			 obj->tx_stream, obj->rx_stream);

	if ((obj->tx_stream == 0) || (obj->tx_stream > ucTxPath))
		obj->tx_stream = ucTxPath;

	if ((obj->rx_stream == 0) || (obj->rx_stream > ucRxPath))
		obj->rx_stream = ucRxPath;

#ifdef CONFIG_ATE
	if (!ATE_ON(ad)) {
		/* Set to new T/RX */
		wlan_config_set_tx_stream(wdev, obj->tx_stream);
		wlan_config_set_rx_stream(wdev, obj->rx_stream);

		MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "normal mode - set to new T/RX\n");
	}
#endif /* CONFIG_ATE */
	MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Operate TxStream = %d, RxStream = %d\n",
			  obj->tx_stream, obj->rx_stream);

}

VOID phy_oper_exit(struct phy_op *obj)
{
	os_zero_mem(obj, sizeof(*obj));
}

/*
* phy_freq internal related function
*/
static UCHAR phy_bw_adjust(UCHAR ht_bw, UCHAR vht_bw)
{
	UCHAR wdev_bw;

	if (ht_bw == HT_BW_40)
		ht_bw = HT_BW_40;
	else
		ht_bw = HT_BW_20;

	if (ht_bw == HT_BW_20)
		wdev_bw = BW_20;
	else {
#ifdef DOT11_VHT_AC
		if (vht_bw == VHT_BW_80)
			wdev_bw = BW_80;
		else if (vht_bw == VHT_BW_160)
			wdev_bw = BW_160;
		else if (vht_bw == VHT_BW_8080)
			wdev_bw = BW_8080;
		else
#endif /* DOT11_VHT_AC */
			wdev_bw = BW_40;
	}

	return wdev_bw;
}

static VOID phy_ht_vht_bw_adjust(UCHAR bw, UCHAR *ht_bw, UCHAR *vht_bw)
{
	if (bw < BW_40)
		*ht_bw = HT_BW_20;
	else
		*ht_bw = HT_BW_40;

#ifdef DOT11_VHT_AC
	*vht_bw = rf_bw_2_vht_bw(bw);
#endif /*DOT11_VHT_AC*/
	return;
}

static BOOLEAN cal_cent_ch_ate(UCHAR prim_ch, CHAR ext_cha, UCHAR *cen_ch)
{
	*cen_ch = prim_ch - ext_cha;

	return TRUE;
}

static BOOLEAN phy_freq_adjust(struct wifi_dev *wdev, struct freq_cfg *cfg, struct freq_oper *op)
{
	UCHAR reg_cap_bw;

	/*initial to legacy setting*/
	if (cfg->prim_ch == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s : no prim_ch value for adjust!\n", __func__));
		return FALSE;
	}

	op->ht_bw = HT_BW_20;
	op->ext_cha = EXTCHA_NONE;
#ifdef DOT11_VHT_AC
	op->vht_bw = VHT_BW_2040;
#endif /*DOT11_VHT_AC*/
	op->ch_band = cfg->ch_band;
	op->prim_ch = cfg->prim_ch;
	op->cen_ch_2 = 0;
	op->cen_ch_1 = op->prim_ch;
#ifdef DOT11_HE_AX
	op->ap_bw = cfg->ap_bw;
	op->ap_cen_ch = cfg->ap_cen_ch;
#endif	/* DOT11_HE_AX */
	op->rx_stream = cfg->rx_stream;
#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode)) {
		op->ht_bw = cfg->ht_bw;
		op->ext_cha = cfg->ext_cha;
		if (!is_testmode_wdev(wdev->wdev_type))
			ht_ext_cha_adjust(wdev->sys_handle, op->prim_ch, &op->ht_bw, &op->ext_cha, wdev);
	}

#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode) || WMODE_CAP_6G(wdev->PhyMode))
		op->vht_bw = (op->ht_bw >= HT_BW_40) ? cfg->vht_bw : VHT_BW_2040;

#endif /*DOT11_VHT_AC*/
	op->bw = phy_bw_adjust(op->ht_bw, op->vht_bw);

	/*check region capability*/
	if (!is_testmode_wdev(wdev->wdev_type)) {
		reg_cap_bw = get_channel_bw_cap(wdev, op->prim_ch);

		if (op->bw > reg_cap_bw) {
			if (!(op->bw == BW_8080 && (reg_cap_bw == BW_80 || reg_cap_bw == BW_160))) {
				/* if bw capability of primary channel is lower than .dat bw config, bw should follow reg_cap_bw*/
				op->bw = reg_cap_bw;
				phy_ht_vht_bw_adjust(op->bw, &op->ht_bw, &op->vht_bw);
			}
		}
	}

	/*central ch*/
	if (op->bw == BW_40) {
		if (is_testmode_wdev(wdev->wdev_type)) {
			cal_cent_ch_ate(op->prim_ch, op->ext_cha, &op->cen_ch_1);
		} else {
			if (cal_ht_cent_ch(op->prim_ch, op->bw, op->ext_cha, &op->cen_ch_1) != TRUE) {
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
						"%s : buggy here.\n", __func__);
				return FALSE;
			}
		}
	}
#ifdef DOT11_VHT_AC
	else if (op->bw > BW_40) {
		if (is_testmode_wdev(wdev->wdev_type))
			cal_cent_ch_ate(op->prim_ch, op->ext_cha, &op->cen_ch_1);
		else
			op->cen_ch_1 = vht_cent_ch_freq(op->prim_ch, op->vht_bw, op->ch_band);
	}

	if (op->bw == BW_8080)
		op->cen_ch_2 = cfg->cen_ch_2;

#endif /*DOT11_VHT_AC*/
#endif /*DOT11_N_SUPPORT*/
	return TRUE;
}


static VOID phy_freq_update(struct wifi_dev *wdev, struct freq_oper *oper)
{
	struct wlan_operate *op = (struct wlan_operate *)wdev->wpf_op;
#ifdef CONFIG_MAP_SUPPORT
	struct _RTMP_ADAPTER *ad = NULL;
#endif
	op->phy_oper.ch_band = oper->ch_band;
	op->phy_oper.prim_ch = oper->prim_ch;
	operate_loader_prim_ch(op);
	op->phy_oper.cen_ch_1 = oper->cen_ch_1;
	op->phy_oper.cen_ch_2 = oper->cen_ch_2;
#ifdef CONFIG_MAP_SUPPORT
	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (IS_MAP_TURNKEY_ENABLE(ad)) {
		if (oper->bw != op->phy_oper.wdev_bw ||
			oper->ht_bw != op->ht_oper.ht_bw) {
			wdev->map_indicate_channel_change = 1;
		}
	}
#endif
	op->phy_oper.wdev_bw = oper->bw;
	op->ht_oper.ht_bw = oper->ht_bw;
	operate_loader_ht_bw(op);
	op->ht_oper.ext_cha = oper->ext_cha;
	operate_loader_ext_cha(op);
#ifdef DOT11_VHT_AC
	op->vht_oper.vht_bw = oper->vht_bw;
	operate_loader_vht_bw(op);
#endif /*DOT11_VHT_AC*/
}

static VOID phy_freq_get_max(struct wifi_dev *wdev, struct freq_oper *result)
{
	struct wlan_operate *op = (struct wlan_operate *)wdev->wpf_op;

	if (op->phy_oper.prim_ch != result->prim_ch)
		return;

	/*bw*/
	if (op->phy_oper.wdev_bw > result->bw) {
		result->bw = op->phy_oper.wdev_bw;
		result->cen_ch_1 = op->phy_oper.cen_ch_1;
		result->cen_ch_2 = op->phy_oper.cen_ch_2;
		result->ext_cha = op->ht_oper.ext_cha;
	}
}

static VOID phy_freq_decision(struct wifi_dev *wdev, struct freq_oper *want, struct freq_oper *result)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct wifi_dev *cur_wdev;
	UCHAR i;
	/*basic setting*/
	os_move_mem(result, want, sizeof(struct freq_oper));

	/*check max capability for each operating*/
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		cur_wdev = ad->wdev_list[i];

		if (cur_wdev &&
			wlan_operate_get_state(cur_wdev) &&
			wmode_band_equal(wdev->PhyMode, cur_wdev->PhyMode))
			phy_freq_get_max(cur_wdev, result);
	}
}

BOOLEAN phy_get_freq_adjust(struct wifi_dev *wdev, struct freq_cfg *cfg, struct freq_oper *op)
{
	return phy_freq_adjust(wdev, cfg, op);
}

/*
* Utility
*/
VOID phy_freq_get_cfg(struct wifi_dev *wdev, struct freq_cfg *fcfg)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	UCHAR BandIdx = HcGetBandByWdev(wdev);

	if (!cfg) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: cfg NULL\n", __func__));
		return;
	}

	os_zero_mem(fcfg, sizeof(struct freq_cfg));
	fcfg->prim_ch = wdev->channel;
	fcfg->ch_band = cfg->phy_conf.ch_band;
	fcfg->cen_ch_2 = cfg->phy_conf.cen_ch_2;

	if (
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		IS_APCLI_SYNC_PEER_DEAUTH_ENBL(pAd, HcGetBandByWdev(wdev)) ||
#endif
		(pAd->CommonCfg.bBssCoexEnable &&
		pAd->CommonCfg.BssCoexScanLastResult[BandIdx].bNeedFallBack)) {
		fcfg->ht_bw = wlan_operate_get_ht_bw(wdev);
		fcfg->ext_cha = wlan_operate_get_ext_cha(wdev);
		fcfg->vht_bw = wlan_operate_get_vht_bw(wdev);
	} else {
		fcfg->ht_bw = cfg->ht_conf.ht_bw;
		fcfg->ext_cha = cfg->ht_conf.ext_cha;
		fcfg->vht_bw = cfg->vht_conf.vht_bw;
	}

#ifdef DOT11_HE_AX
	fcfg->ap_bw = cfg->phy_conf.ap_bw;
	fcfg->ap_cen_ch = cfg->phy_conf.ap_cen_ch;
#endif	/* DOT11_HE_AX */
	fcfg->rx_stream = cfg->phy_conf.rx_stream;
}

/*
* Configure loader
*/

/*
* Operater loader
*/
VOID operate_loader_prim_ch(struct wlan_operate *op)
{
	UCHAR prim_ch = op->phy_oper.prim_ch;

	op->ht_status.addht.ControlChan = prim_ch;
}

/*
*
*/
VOID operate_loader_phy(struct wifi_dev *wdev, struct freq_cfg *cfg)
{
	struct freq_oper oper_dev;
	struct freq_oper oper_radio;
	struct radio_res res;
	UCHAR i = 0;
	UCHAR band_idx = 0;
	struct wifi_dev *tdev = NULL;
	struct _RTMP_ADAPTER *ad = NULL;
#ifdef DFS_ADJ_BW_ZERO_WAIT
	UINT8 WdevIdx = 0;
#endif

	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				 "wdev NULL!");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			 "oper_cfg: prim_ch(%d), ht_bw(%d), extcha(%d), vht_bw(%d), cen_ch_2(%d), PhyMode=%d!\n",
			  cfg->prim_ch,
			  cfg->ht_bw,
			  cfg->ext_cha,
			  cfg->vht_bw,
			  cfg->cen_ch_2,
			  wdev->PhyMode);
	os_zero_mem(&oper_dev, sizeof(oper_dev));
	if (!phy_freq_adjust(wdev, cfg, &oper_dev)) {
		MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				 "phy_freq_adjust failed!");
		return;
	}

	MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			 "oper_dev after adjust: bw(%d), prim_ch(%d), cen_ch_1(%d), cen_ch_2(%d),ext_cha(%d)!\n",
			  oper_dev.bw,
			  oper_dev.prim_ch,
			  oper_dev.cen_ch_1,
			  oper_dev.cen_ch_2,
			  oper_dev.ext_cha);
	phy_freq_update(wdev, &oper_dev);

#ifdef DFS_ADJ_BW_ZERO_WAIT
	if (ad->CommonCfg.DfsParameter.BW160ZeroWaitSupport == TRUE) {
		band_idx = HcGetBandByWdev(wdev);
		for (WdevIdx = 0; WdevIdx < WDEV_NUM_MAX; WdevIdx++) {
			tdev = ad->wdev_list[WdevIdx];
				if (tdev && HcIsRadioAcq(tdev) && (band_idx == HcGetBandByWdev(tdev)))
				phy_freq_update(tdev, &oper_dev);
		}
	}
#endif
	/*get last radio result for hdev check and update*/
	phy_freq_decision(wdev, &oper_dev, &oper_radio);
	MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			 "oper_radio after decision: bw(%d), prim_ch(%d), cen_ch_1(%d), cen_ch_2(%d)!\n",
			  oper_radio.bw,
			  oper_radio.prim_ch,
			  oper_radio.cen_ch_1,
			  oper_radio.cen_ch_2);
	/*acquire radio resouce*/
	res.reason = REASON_NORMAL_SW;
	res.oper = &oper_radio;

#ifdef CONFIG_AP_SUPPORT
#ifdef MT_DFS_SUPPORT
	/* Perform CAC only for DFS Channel */
	if (DfsRadarChannelCheck(ad, wdev, oper_radio.cen_ch_2, oper_radio.bw))
		DfsCacNormalStart(ad, wdev, RD_SILENCE_MODE);
#endif
#endif

	if (hc_radio_res_request(wdev, &res) != TRUE) {
		/*can't get radio resource, update operating to radio setting*/
		MTWF_DBG(ad, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
				 "oper_dev request radio fail! bw(%d), prim_ch(%d), cen_ch_1(%d), cen_ch_2(%d)!\n",
				  oper_dev.bw,
				  oper_dev.prim_ch,
				  oper_dev.cen_ch_1,
				  oper_dev.cen_ch_2);
		/*Even if the interface is down, the relevant configuration
		  should be updated and take effect when it is up*/
		phy_freq_update(wdev, &oper_dev);
		return;
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef MT_DFS_SUPPORT
	DfsCacNormalStart(ad, wdev, RD_NORMAL_MODE);

	/* Perform CAC & Radar Detect only for DFS Channel */
	if (DfsRadarChannelCheck(ad, wdev, oper_radio.cen_ch_2, oper_radio.bw)) {
		WrapDfsRadarDetectStart(ad, wdev);
	}
#endif
#endif

	wdev_sync_prim_ch(ad, wdev);
	band_idx = HcGetBandByWdev(wdev);
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = ad->wdev_list[i];
		if (tdev && (band_idx == HcGetBandByWdev(tdev))) {
			if (tdev == wdev)
				continue;
			phy_freq_update(tdev, &oper_dev);
		}
	}
}


/*
* export function
*/

/*
* operation function
*/
UCHAR wlan_operate_get_bw(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: op NULL\n", __func__));
		return 0;
	}

	return op->phy_oper.wdev_bw;
}

UCHAR wlan_operate_get_prim_ch(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: op NULL\n", __func__));
		return 0;
	}

	return op->phy_oper.prim_ch;
}

UCHAR wlan_operate_get_ch_band(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: op NULL\n", __func__));
		return 0;
	}

	return op->phy_oper.ch_band;
}

INT32 wlan_operate_set_ch_band(struct wifi_dev *wdev, UCHAR ch_band)
{
	struct freq_cfg cfg;

	os_zero_mem(&cfg, sizeof(cfg));
	phy_freq_get_cfg(wdev, &cfg);
	cfg.ch_band = ch_band;
	operate_loader_phy(wdev, &cfg);
	return WLAN_OPER_OK;
}

INT32 wlan_operate_set_prim_ch(struct wifi_dev *wdev, UCHAR prim_ch)
{
	struct freq_cfg cfg;

	os_zero_mem(&cfg, sizeof(cfg));
	phy_freq_get_cfg(wdev, &cfg);
	cfg.prim_ch = prim_ch;
	operate_loader_phy(wdev, &cfg);
	return WLAN_OPER_OK;
}

INT32 wlan_operate_set_phy(struct wifi_dev *wdev, struct freq_cfg *cfg)
{
	operate_loader_phy(wdev, cfg);
	return WLAN_OPER_OK;
}


INT32 wlan_operate_set_tx_stream(struct wifi_dev *wdev, UINT8 tx_stream)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: op NULL\n", __func__));
		return 0;
	}

	op->phy_oper.tx_stream = tx_stream;
	return WLAN_OPER_OK;
}


INT32 wlan_operate_set_rx_stream(struct wifi_dev *wdev, UINT8 rx_stream)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: op NULL\n", __func__));
		return 0;
	}

	op->phy_oper.rx_stream = rx_stream;
	return WLAN_OPER_OK;
}


UCHAR wlan_operate_get_cen_ch_2(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: op NULL\n", __func__));
		return 0;
	}

	return op->phy_oper.cen_ch_2;
}

INT32 wlan_operate_set_cen_ch_2(struct wifi_dev *wdev, UCHAR cen_ch_2)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;
	struct freq_cfg cfg;

	os_zero_mem(&cfg, sizeof(cfg));

	if (!op) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: op NULL\n", __func__));
		return 0;
	}

	if (op->phy_oper.cen_ch_2 == cen_ch_2)
		return WLAN_OPER_OK;

	phy_freq_get_cfg(wdev, &cfg);
	cfg.cen_ch_2 = cen_ch_2;
	operate_loader_phy(wdev, &cfg);
	return WLAN_OPER_OK;
}

UCHAR wlan_operate_get_cen_ch_1(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: op NULL\n", __func__));
		return 0;
	}

	return op->phy_oper.cen_ch_1;
}

UINT8 wlan_operate_get_tx_stream(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: op NULL\n", __func__));
		return 0;
	}

	return op->phy_oper.tx_stream;
}


UINT8 wlan_operate_get_rx_stream(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	if (!op) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: op NULL\n", __func__));
		return 0;
	}

	return op->phy_oper.rx_stream;
}


BOOLEAN wlan_operate_scan(struct wifi_dev *wdev, UCHAR prim_ch)
{
	struct radio_res radio, *res = &radio;
	struct freq_oper oper;
	BOOLEAN ret;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	os_zero_mem(&oper, sizeof(oper));
	res->oper = &oper;

	/* do not change sequence due to 6GHz might include AC/GN then confused */
	if (WMODE_CAP_6G(wdev->PhyMode))
		oper.ch_band = CMD_CH_BAND_6G;
	else if (WMODE_CAP_5G(wdev->PhyMode))
		oper.ch_band = CMD_CH_BAND_5G;
	else
		oper.ch_band = CMD_CH_BAND_24G;

	oper.bw = BW_20;
	oper.cen_ch_1 = prim_ch;
	oper.ext_cha = EXTCHA_NONE;
	oper.ht_bw = HT_BW_20;
#ifdef DOT11_VHT_AC
	oper.cen_ch_2 = 0;
	oper.vht_bw = VHT_BW_2040;
#endif /*DOT11_VHT_AC*/
	oper.prim_ch = prim_ch;
	res->reason = REASON_NORMAL_SCAN;
	ad->oper_ch = wlan_operate_get_prim_ch(wdev);
	ret = hc_radio_res_request(wdev, res);
	return ret;
}

