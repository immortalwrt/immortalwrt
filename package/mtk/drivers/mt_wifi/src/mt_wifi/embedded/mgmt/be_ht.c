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

/*
*ht phy info related
*/
VOID ht_oper_init(struct wifi_dev *wdev, struct ht_op *obj)
{
	/*initial ht_phy_info value*/
	obj->ht_bw = wlan_operate_get_ht_bw(wdev);
	obj->ext_cha = wlan_operate_get_ext_cha(wdev);

	obj->ht_ldpc = wlan_config_get_ht_ldpc(wdev);
	obj->ht_stbc = wlan_config_get_ht_stbc(wdev);
	obj->ht_gi = wlan_config_get_ht_gi(wdev);
	obj->frag_thld = wlan_config_get_frag_thld(wdev);
	obj->len_thld = wlan_config_get_rts_len_thld(wdev);
	obj->pkt_thld = wlan_config_get_rts_pkt_thld(wdev);
	/* frag threshold */
	wlan_operate_set_frag_thld(wdev, obj->frag_thld);
	/* rts threshold */
	wlan_operate_set_rts_len_thld(wdev, obj->len_thld);
	wlan_operate_set_rts_pkt_thld(wdev, obj->pkt_thld);
}

VOID ht_oper_exit(struct ht_op *obj)
{
	os_zero_mem(obj, sizeof(*obj));
}

/*
* ht operate related
*/
VOID ht_op_status_init(struct wifi_dev *wdev, struct ht_op_status *obj)
{
	wlan_operate_update_ht_cap(wdev);
}

VOID ht_op_status_exit(struct ht_op_status *obj)
{
	os_zero_mem(obj, sizeof(struct ht_op_status));
}


/*
* Configure loader
*/
static VOID ht_oper_set_ext_cha(struct wifi_dev *wdev, UCHAR ext_cha)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct wlan_operate *op;
	struct freq_cfg cfg;
	struct wifi_dev *twdev;
	UCHAR i;

	os_zero_mem(&cfg, sizeof(cfg));
	/*update extcha since radio is changed*/
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		twdev = ad->wdev_list[i];

		if (!twdev)
			continue;

		if (twdev->channel != wdev->channel)
			continue;

		if (wlan_operate_get_state(twdev) != WLAN_OPER_STATE_VALID)
			continue;

		op = (struct wlan_operate *)twdev->wpf_op;

		if ((op->ht_oper.ext_cha != EXTCHA_NONE) && (op->ht_oper.ext_cha != ext_cha)) {
			phy_freq_get_cfg(twdev, &cfg);
			cfg.ext_cha = ext_cha;
			operate_loader_phy(twdev, &cfg);
		}
	}
}

/*
*
Operate loader
*/
VOID operate_loader_trx_stream(struct wifi_dev *wdev, struct wlan_operate *op, UINT8 tx_stream, UINT8 rx_stream)
{
	UINT8 cur_op_rx_stream = rx_stream;

#ifdef DBDC_MODE
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (pAd->CommonCfg.dbdc_mode) {
		UINT8 band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0) {
			if (cur_op_rx_stream > pAd->dbdc_band0_rx_path)
				cur_op_rx_stream = pAd->dbdc_band0_rx_path;
		} else {
			if (cur_op_rx_stream > pAd->dbdc_band1_rx_path)
				cur_op_rx_stream = pAd->dbdc_band1_rx_path;
		}
	}
#endif

	memset(op->ht_status.ht_cap_ie.MCSSet, 0, sizeof(op->ht_status.ht_cap_ie.MCSSet));
	switch (cur_op_rx_stream) {
	case 4:
		op->ht_status.ht_cap_ie.MCSSet[3] =  0xff;
		/* fall through */
	case 3:
		op->ht_status.ht_cap_ie.MCSSet[2] =  0xff;
		/* fall through */
	case 2:
		op->ht_status.ht_cap_ie.MCSSet[1] =  0xff;
		/* fall through */
	case 1:

	default:
		op->ht_status.ht_cap_ie.MCSSet[0] =  0xff;
	}
}

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
VOID operate_loader_eap_trx_stream(struct wifi_dev *wdev, struct wlan_operate *op, UINT8 tx_stream, UINT8 rx_stream)
{
	UINT8 cur_op_rx_stream = rx_stream;

	if (wdev->eap.eap_htsuprate_en != TRUE)
		return;

	memset(op->ht_status.ht_cap_ie.MCSSet, 0, sizeof(op->ht_status.ht_cap_ie.MCSSet));
	switch (cur_op_rx_stream) {
	case 4:
		op->ht_status.ht_cap_ie.MCSSet[3] = wdev->eap.eapmcsset[3];
		/* fall through */
	case 3:
		op->ht_status.ht_cap_ie.MCSSet[2] = wdev->eap.eapmcsset[2];
		/* fall through */
	case 2:
		op->ht_status.ht_cap_ie.MCSSet[1] = wdev->eap.eapmcsset[1];
		/* fall through */
	case 1:
		op->ht_status.ht_cap_ie.MCSSet[0] = wdev->eap.eapmcsset[0];
		break;
	}
}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

VOID operate_loader_ht_bw(struct wlan_operate *op)
{
	op->ht_status.addht.AddHtInfo.RecomWidth = op->ht_oper.ht_bw;
}

VOID operate_loader_ext_cha(struct wlan_operate *op)
{
	op->ht_status.addht.AddHtInfo.ExtChanOffset = op->ht_oper.ext_cha;
}

VOID operate_loader_ht_stbc(struct wlan_operate *op, UCHAR tx_nsts, UCHAR rx_nsts, UCHAR ht_stbc)
{
	UCHAR tx_stbc = STBC_NONE, rx_stbc = STBC_NONE;

	if (ht_stbc == STBC_USE) {
		if (tx_nsts > 1)
			tx_stbc = STBC_USE;
		if (rx_nsts >= 1)
			rx_stbc = RXSTBC_ONE; /*current hw only support rx 1ss STBC */
		if ((tx_stbc == STBC_NONE)
				&& (rx_stbc == STBC_NONE))
			ht_stbc = STBC_NONE;
	}
	op->ht_oper.ht_stbc = ht_stbc;
	op->ht_status.ht_cap_ie.HtCapInfo.TxSTBC = tx_stbc;
	op->ht_status.ht_cap_ie.HtCapInfo.RxSTBC = rx_stbc;
}

VOID operate_loader_ht_ldpc(struct wlan_operate *op, UCHAR ht_ldpc)
{
	op->ht_oper.ht_ldpc = ht_ldpc;
	op->ht_status.ht_cap_ie.HtCapInfo.ht_rx_ldpc = ht_ldpc;
}

VOID operate_loader_ht_gi(struct wlan_operate *op, UCHAR ht_bw, UCHAR ht_gi)
{
	UCHAR ht20_gi = GI_800, ht40_gi = GI_800;

	if (ht_gi == GI_400) {
		ht20_gi = GI_400;
		if (ht_bw == HT_BW_40)
			ht40_gi = GI_400;
	}
	op->ht_oper.ht_gi = ht_gi;
	op->ht_status.ht_cap_ie.HtCapInfo.ShortGIfor20 = ht20_gi;
	op->ht_status.ht_cap_ie.HtCapInfo.ShortGIfor40 = ht40_gi;
}

VOID operate_loader_greenfield(struct wlan_operate *op, UCHAR ht_gf)
{
	/*TODO: should check if STA is nonHT */
	op->ht_status.ht_cap_ie.HtCapInfo.GF = ht_gf;
}

VOID operate_loader_max_amsdu_len(struct wlan_operate *op, UCHAR len)
{
	if (len > MPDU_7991_OCTETS)
		len = MPDU_7991_OCTETS;
	op->ht_status.ht_cap_ie.HtCapInfo.AMsduSize = len;
}

VOID operate_loader_min_mpdu_start_space(struct wlan_operate *op, UCHAR min_start_space)
{
	op->ht_status.ht_cap_ie.HtCapParm.MpduDensity = min_start_space;
}

VOID operate_loader_ht_max_ampdu_len_exp(struct wlan_operate *op, UCHAR exp_factor)
{
	/* recv. cap. max_ampdu_len = 2^(13+exp) - 1, from 0 to 3 */
	op->ht_status.ht_cap_ie.HtCapParm.MaxRAmpduFactor = exp_factor;
}

VOID operate_loader_support_ch_width_set(struct wlan_operate *op, UCHAR ch_width_set)
{
	op->ht_status.ht_cap_ie.HtCapInfo.ChannelWidth = ch_width_set;
}

VOID operate_loader_smps(struct wlan_operate *op, UCHAR smps)
{
	/* TODO: need to confirm MMPS equal to SMPS? */
	op->ht_status.ht_cap_ie.HtCapInfo.MimoPs = smps;
}

VOID operate_loader_cckin40(struct wlan_operate *op, BOOL is_2g, UCHAR ht_bw)
{
	UCHAR cckin40 = 0;

	if (is_2g && (ht_bw == HT_BW_40))
		cckin40 = 1;
	op->ht_status.ht_cap_ie.HtCapInfo.CCKmodein40 = cckin40;
}

VOID operate_loader_rts_len_thld(struct wlan_operate *op, UINT32 pkt_len)
{
	op->ht_oper.len_thld = pkt_len;
}

VOID operate_loader_frag_thld(struct wlan_operate *op, UINT32 frag_thld)
{
	op->ht_oper.frag_thld = frag_thld;
}

VOID operate_loader_rts_pkt_thld(struct wlan_operate *op, UCHAR pkt_num)
{
	op->ht_oper.pkt_thld = pkt_num;
}
/*
*  export operate function
*/
/*
* Set
*/
INT32 wlan_operate_set_support_ch_width_set(struct wifi_dev *wdev, UCHAR ch_width_set)
{
	struct wlan_operate *op;
	INT32 ret = WLAN_OPER_OK;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): can't find wlan opeate!\n", __func__));
		return ret;
	}

	op = (struct wlan_operate *) wdev->wpf_op;
	if (wdev && wdev->wpf_op) {
		op = (struct wlan_operate *)wdev->wpf_op;
		operate_loader_support_ch_width_set(op, ch_width_set);
	}
	return ret;
}

INT32 wlan_operate_set_ht_bw(struct wifi_dev *wdev, UCHAR ht_bw, UCHAR ext_cha)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;
	UCHAR cap_ht_bw = wlan_config_get_ht_bw(wdev);
	INT32 ret = WLAN_OPER_OK;
	struct freq_cfg cfg;

	if ((ht_bw == op->ht_oper.ht_bw) && (ext_cha == op->ht_oper.ext_cha))
		return ret;

	if (ht_bw > cap_ht_bw) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): new ht_bw:%d > cap_ht_bw: %d, correct to cap_ht_bw\n",
				  __func__,
				  ht_bw,
				  cap_ht_bw
				 ));
		ht_bw = cap_ht_bw;
		ret = WLAN_OPER_FAIL;
	}

	phy_freq_get_cfg(wdev, &cfg);
	cfg.ht_bw = ht_bw;
	cfg.ext_cha = ext_cha;
	operate_loader_phy(wdev, &cfg);

	if (op->ht_oper.ext_cha != EXTCHA_NONE)
		ht_oper_set_ext_cha(wdev, op->ht_oper.ext_cha);

	return ret;
}

INT32 wlan_operate_set_non_gf_sta(struct wifi_dev *wdev, UINT16 non_gf_sta)
{
	struct wlan_operate *op;

	/*due to use in MACTableMaintaince*/
	if (wdev && wdev->wpf_op) {
		op = (struct wlan_operate *)wdev->wpf_op;
		op->ht_status.non_gf_sta = non_gf_sta;
	}

	return WLAN_OPER_OK;
}

INT32 wlan_operate_set_ht_stbc(struct wifi_dev *wdev, UCHAR ht_stbc)
{
	INT32 ret = WLAN_OPER_OK;

	if (wdev)
		wlan_operate_update_ht_stbc(wdev, ht_stbc);
	return ret;
}

INT32 wlan_operate_set_ht_ldpc(struct wifi_dev *wdev, UCHAR ht_ldpc)
{
	struct wlan_operate *op;
	INT32 ret = WLAN_OPER_OK;

	if (wdev && wdev->wpf_op) {
		op = (struct wlan_operate *)wdev->wpf_op;
		operate_loader_ht_ldpc(op, ht_ldpc);
	}
	return ret;
}

INT32 wlan_operate_loader_greenfield(struct wifi_dev *wdev, UCHAR ht_gf)
{
    struct wlan_operate *op;
    INT32 ret = WLAN_OPER_OK;

    if (wdev && wdev->wpf_op) {
		op = (struct wlan_operate *)wdev->wpf_op;
		operate_loader_greenfield(op, ht_gf);
    }
    return ret;
}

INT32 wlan_operate_set_max_amsdu_len(struct wifi_dev *wdev, UCHAR len)
{
	struct wlan_operate *op;
	INT32 ret = WLAN_OPER_OK;

	if (wdev && wdev->wpf_op) {
		op = (struct wlan_operate *)wdev->wpf_op;
		operate_loader_max_amsdu_len(op, len);
	}
	return ret;
}

INT32 wlan_operate_set_min_start_space(struct wifi_dev *wdev, UCHAR mpdu_density)
{
	struct wlan_operate *op;
	INT32 ret = WLAN_OPER_OK;

	if (wdev && wdev->wpf_op) {
		op = (struct wlan_operate *)wdev->wpf_op;
		operate_loader_min_mpdu_start_space(op, mpdu_density);
	}
	return ret;
}

INT32 wlan_operate_set_mmps(struct wifi_dev *wdev, UCHAR mmps)
{
	struct wlan_operate *op;
	INT32 ret = WLAN_OPER_OK;

	if (wdev && wdev->wpf_op) {
		op = (struct wlan_operate *)wdev->wpf_op;
		operate_loader_smps(op, mmps);
	}
	return ret;
}

INT32 wlan_operate_set_ht_max_ampdu_len_exp(struct wifi_dev *wdev, UCHAR exp_factor)
{
	struct wlan_operate *op;
	INT32 ret = WLAN_OPER_OK;

	if (wdev && wdev->wpf_op) {
		op = (struct wlan_operate *)wdev->wpf_op;
		operate_loader_ht_max_ampdu_len_exp(op, exp_factor);
	}
	return ret;
}

INT32 wlan_operate_set_ht_delayed_ba(struct wifi_dev *wdev, UCHAR support)
{
	struct wlan_operate *op;
	INT32 ret = WLAN_OPER_OK;

	if (wdev && wdev->wpf_op) {
		op = (struct wlan_operate *)wdev->wpf_op;
		op->ht_status.ht_cap_ie.HtCapInfo.DelayedBA = support;
	}
	return ret;
}

INT32 wlan_operate_set_lsig_txop_protect(struct wifi_dev *wdev, UCHAR support)
{
	struct wlan_operate *op;
	INT32 ret = WLAN_OPER_OK;

	if (wdev && wdev->wpf_op) {
		op = (struct wlan_operate *)wdev->wpf_op;
		op->ht_oper.l_sig_txop = support;
		op->ht_status.ht_cap_ie.HtCapInfo.LSIGTxopProSup = support;
	}
	return ret;
}

INT32 wlan_operate_set_psmp(struct wifi_dev *wdev, UCHAR psmp)
{
	struct wlan_operate *op;
	INT32 ret = WLAN_OPER_OK;

	if (wdev && wdev->wpf_op) {
		op = (struct wlan_operate *)wdev->wpf_op;
		op->ht_status.ht_cap_ie.HtCapInfo.PSMP = psmp;
	}
	return ret;
}

INT32 wlan_operate_set_frag_thld(struct wifi_dev *wdev, UINT32 frag_thld)
{
	struct wlan_operate *op;
	INT32 ret = WLAN_OPER_OK;

	if (wdev && wdev->wpf_op) {
		op = (struct wlan_operate *)wdev->wpf_op;
		operate_loader_frag_thld(op, frag_thld);
	}
	return ret;
}

INT32 wlan_operate_set_rts_pkt_thld(struct wifi_dev *wdev, UCHAR pkt_num)
{
	struct wlan_operate *op;
	struct _RTMP_ADAPTER *ad;
	INT32 ret = WLAN_OPER_OK;

	if (wdev && wdev->wpf_op) {
		op = (struct wlan_operate *)wdev->wpf_op;
		ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
		operate_loader_rts_pkt_thld(op, pkt_num);
		HW_SET_RTS_THLD(ad, wdev, op->ht_oper.pkt_thld, op->ht_oper.len_thld);
	}
	return ret;
}

INT32 wlan_operate_set_rts_len_thld(struct wifi_dev *wdev, UINT32 pkt_len)
{
	struct wlan_operate *op;
	struct _RTMP_ADAPTER *ad;
	INT32 ret = WLAN_OPER_OK;

	if (wdev && wdev->wpf_op) {
		op = (struct wlan_operate *)wdev->wpf_op;
		ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
		operate_loader_rts_len_thld(op, pkt_len);
		HW_SET_RTS_THLD(ad, wdev, op->ht_oper.pkt_thld, op->ht_oper.len_thld);
	}
	return ret;
}

/*
*Get
*/
UCHAR wlan_operate_get_ht_bw(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	return op->ht_oper.ht_bw;
}

UCHAR wlan_operate_get_ht_stbc(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	return op->ht_oper.ht_stbc;
}

UCHAR wlan_operate_get_ht_ldpc(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	return op->ht_oper.ht_ldpc;
}

UCHAR wlan_operate_get_ext_cha(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	return op->ht_oper.ext_cha;
}

struct _ADD_HT_INFO_IE *wlan_operate_get_addht(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	return &op->ht_status.addht;
}

UINT16 wlan_operate_get_non_gf_sta(struct wifi_dev *wdev)
{
	struct wlan_operate *op;

	if (wdev && wdev->wpf_op) {
		op = (struct wlan_operate *)wdev->wpf_op;
		return op->ht_status.non_gf_sta;
	}

	return 0;
}

UINT32 wlan_operate_get_frag_thld(struct wifi_dev *wdev)
{
	struct wlan_operate *op;
	if (!wdev)
		return DEFAULT_FRAG_THLD;
	op = (struct wlan_operate *) wdev->wpf_op;
	if (!op)
		return DEFAULT_FRAG_THLD;

	return op->ht_oper.frag_thld;
}
EXPORT_SYMBOL(wlan_operate_get_frag_thld);

UCHAR wlan_operate_get_rts_pkt_thld(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	return op->ht_oper.pkt_thld;
}

UINT32 wlan_operate_get_rts_len_thld(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	return op->ht_oper.len_thld;
}

VOID *wlan_operate_get_ht_cap(struct wifi_dev *wdev)
{
	struct wlan_operate *op = NULL;
	if (wdev == NULL)/*ALPS05330340*/
		return NULL;
	op = (struct wlan_operate *)wdev->wpf_op;
	if (op)
		return &op->ht_status.ht_cap_ie;

	return NULL;
}
/*
* Update
*/
VOID wlan_operate_update_ht_stbc(struct wifi_dev *wdev, UCHAR use_stbc)
{
	struct wlan_operate *op = (struct wlan_operate *)wdev->wpf_op;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	UCHAR tx_nsts = ad->Antenna.field.TxPath;
	UCHAR rx_nsts = ad->Antenna.field.RxPath;

#ifdef DBDC_MODE
	if (ad->CommonCfg.dbdc_mode) {
		UINT8 band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0) {
			tx_nsts = ad->dbdc_band0_tx_path;
			rx_nsts = ad->dbdc_band0_rx_path;
		} else {
			tx_nsts = ad->dbdc_band1_tx_path;
			rx_nsts = ad->dbdc_band1_rx_path;
		}
	}
#endif

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		UINT8 BandIdx = HcGetBandByWdev(wdev);
		if (ad->bAntennaSetAPEnable[BandIdx]) {
			tx_nsts = ad->TxStream[BandIdx];
			rx_nsts = ad->RxStream[BandIdx];
		}
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

	if (op)
		operate_loader_ht_stbc(op, tx_nsts, rx_nsts, use_stbc);
}

VOID wlan_operate_update_ht_cap(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
	struct wlan_operate *op = (struct wlan_operate *)wdev->wpf_op;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
#ifdef TXBF_SUPPORT
	HT_CAPABILITY_IE *ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev); /* Get wdev's HT TxBF Cap pointer */
#endif

	if (cfg && op) {
		/* set from .dat */
		operate_loader_ht_ldpc(op, cfg->ht_conf.ht_ldpc);
		operate_loader_support_ch_width_set(op, cfg->ht_conf.ht_bw);
		operate_loader_ht_gi(op, cfg->ht_conf.ht_bw, cfg->ht_conf.ht_gi);
		operate_loader_greenfield(op, cfg->ht_conf.gf_support);
		operate_loader_min_mpdu_start_space(op, cfg->ht_conf.min_mpdu_start_space);
		operate_loader_smps(op, cfg->ht_conf.mmps);
		wlan_operate_update_ht_stbc(wdev, cfg->ht_conf.ht_stbc);
		operate_loader_trx_stream(wdev, op, cfg->phy_conf.tx_stream, cfg->phy_conf.rx_stream);
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
		operate_loader_eap_trx_stream(wdev, op, cfg->phy_conf.tx_stream, cfg->phy_conf.rx_stream);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */


		/* set from chip cap */
		operate_loader_max_amsdu_len(op, cap->ppdu.max_amsdu_len);
		operate_loader_ht_max_ampdu_len_exp(op, cap->ppdu.ht_max_ampdu_len_exp);
		/* set from fix */
		wlan_operate_set_ht_delayed_ba(wdev, 0);
		wlan_operate_set_lsig_txop_protect(wdev, 0);
		wlan_operate_set_psmp(wdev, 0);
		operate_loader_cckin40(op, WMODE_CAP_2G(wdev->PhyMode), cfg->ht_conf.ht_bw);
#ifdef TXBF_SUPPORT
		if (cap->FlgHwTxBfCap) {
			/* build HT TxBF Cap for wdev*/
			mt_WrapSetETxBFCap(ad, wdev, &ht_cap->TxBFCap);
		}
#endif /* TXBF_SUPPORT */
	}
}
