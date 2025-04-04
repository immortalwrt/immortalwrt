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
#include "wlan_config/config_internal.h"


/*
* define  constructor & deconstructor & method
*/
/*
*basic phy related
*/
VOID phy_cfg_init(struct phy_cfg *obj)
{
#ifdef TXBF_SUPPORT
	obj->ETxBfEnCond = SUBF_OFF;
	obj->ITxBfEn = SUBF_OFF;
#endif /* TXBF_SUPPORT */
	obj->mu_dl_ofdma = 0;
	obj->mu_ul_ofdma = 0;
	obj->mu_dl_mimo = 0;
	obj->mu_ul_mimo = 0;
	obj->fixed_mcs = 0xff;
	obj->ul_mu_data_disable_rx = 1;
	obj->er_su_rx_disable = 0;
}

VOID phy_cfg_exit(struct phy_cfg *obj)
{
	os_zero_mem(obj, sizeof(struct phy_cfg));
}

/*
* Operater loader
*/

UCHAR chip_get_max_nss(struct _RTMP_ADAPTER *ad, UINT8 bandIdx)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	struct mcs_nss_caps *nss_cap = &cap->mcs_nss;

	return nss_cap->max_nss[bandIdx];
}

/*
* export function
*/
/*
* configure functio
*/
VOID wlan_config_set_ch_band(struct wifi_dev *wdev, USHORT wmode)
{
	UINT8 ch_band = CMD_CH_BAND_24G;

	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cfg NULL\n");
		return;
	}

	/* do not change sequence due to 6GHz might include AC/GN then confused */
	if (WMODE_CAP_6G(wmode))
		ch_band = CMD_CH_BAND_6G;
	else if WMODE_CAP_5G(wmode)
		ch_band = CMD_CH_BAND_5G;

	cfg->phy_conf.ch_band = ch_band;
}

VOID wlan_config_set_ch_band_all(struct wpf_ctrl *ctrl, USHORT wmode)
{
	struct wlan_config *cfg;
	unsigned int i;
	UINT8 ch_band = CMD_CH_BAND_24G;

	/* do not change sequence due to 6GHz might include AC/GN then confused */
	if (WMODE_CAP_6G(wmode))
		ch_band = CMD_CH_BAND_6G;
	else if WMODE_CAP_5G(wmode)
		ch_band = CMD_CH_BAND_5G;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		cfg = (struct wlan_config *)ctrl->pf[i].conf;

		if (!cfg) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				" cfg NULL\n");
			return;
		}

		cfg->phy_conf.ch_band = ch_band;
	}
}

VOID wlan_config_set_cen_ch_2(struct wifi_dev *wdev, UCHAR cen_ch_2)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return;
	}

	cfg->phy_conf.cen_ch_2 = cen_ch_2;
}

VOID wlan_config_set_cen_ch_2_all(struct wpf_ctrl *ctrl, UCHAR cen_ch_2)
{
	struct wlan_config *cfg;
	unsigned int i;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		cfg = (struct wlan_config *)ctrl->pf[i].conf;

		if (!cfg) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				" cfg NULL\n");
			return;
		}

		cfg->phy_conf.cen_ch_2 = cen_ch_2;
	}
}

#ifdef DOT11_HE_AX
VOID wlan_config_set_ap_bw(struct wifi_dev *wdev, UCHAR ap_bw)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return;
	}

	cfg->phy_conf.ap_bw = ap_bw;
}

VOID wlan_config_set_ap_cen(struct wifi_dev *wdev, UCHAR ap_cen_ch)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return;
	}

	cfg->phy_conf.ap_cen_ch = ap_cen_ch;
}
#endif	/* DOT11_HE_AX */

/*
*Set
*/
VOID wlan_config_set_ack_policy(struct wifi_dev *wdev,UCHAR *policy)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	UCHAR i=0;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return;
	}

	for(i = 0; i < WMM_NUM_OF_AC ; i++){
		cfg->phy_conf.ack_policy[i] = policy[i];
	}
}

VOID wlan_config_set_ack_policy_all(struct wpf_ctrl *ctrl,UCHAR *policy)
{
	struct wlan_config *cfg;
	UCHAR i;
	UCHAR j;
	for(i=0;i<WDEV_NUM_MAX;i++){
		cfg = (struct wlan_config*)ctrl->pf[i].conf;

		if (!cfg) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				" cfg NULL\n");
			return;
		}

		for(j = 0; j < WMM_NUM_OF_AC ; j++){
			cfg->phy_conf.ack_policy[j] = policy[j];
		}
	}
}

VOID wlan_config_set_tx_stream(struct wifi_dev *wdev, UINT8 tx_stream)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cfg NULL\n");
		return;
	}

	cfg->phy_conf.tx_stream = tx_stream;
}

VOID wlan_config_set_rx_stream(struct wifi_dev *wdev, UINT8 rx_stream)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return;
	}

	cfg->phy_conf.rx_stream = rx_stream;
}

#ifdef TXBF_SUPPORT
VOID wlan_config_set_etxbf(struct wifi_dev *wdev, UCHAR ETxBfEnCond)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return;
	}

	cfg->phy_conf.ETxBfEnCond = ETxBfEnCond;

}

VOID wlan_config_set_itxbf(struct wifi_dev *wdev, UCHAR ITxBfEn)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return;
	}

	cfg->phy_conf.ITxBfEn = ITxBfEn;

}
#endif /* TXBF_SUPPORT */

VOID wlan_config_set_fixed_mcs(struct wifi_dev *wdev, UINT8 fixed_mcs)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return;
	}

	cfg->phy_conf.fixed_mcs = fixed_mcs;
}

/*
*Get
*/
UCHAR wlan_config_get_ch_band(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.ch_band;
}

UCHAR wlan_config_get_cen_ch_2(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.cen_ch_2;
}

UCHAR wlan_config_get_ack_policy(struct wifi_dev *wdev, UCHAR ac_id)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.ack_policy[ac_id];
}

UINT8 wlan_config_get_tx_stream(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.tx_stream;
}

UINT8 wlan_config_get_rx_stream(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.rx_stream;
}

#ifdef TXBF_SUPPORT
UCHAR wlan_config_get_etxbf(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.ETxBfEnCond;
}

UCHAR wlan_config_get_itxbf(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.ITxBfEn;
}
#endif /* TXBF_SUPPORT */

UINT8 wlan_config_get_fixed_mcs(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			" cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.fixed_mcs;
}

enum PHY_CAP wlan_cofig_get_phy_caps(struct wifi_dev *wdev)
{
	VOID *hdev_ctrl = hc_get_hdev_ctrl(wdev);
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(hdev_ctrl);

	return chip_cap->phy_caps;
}
