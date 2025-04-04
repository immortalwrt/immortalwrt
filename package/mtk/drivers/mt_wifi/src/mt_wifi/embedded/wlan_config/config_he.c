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

static VOID he_cfg_mu_edca_init(struct mu_edca_cfg *mu_edca)
{
	/*AC_BK*/
	mu_edca->mu_ac_rec[ACI_AC_BK].acm = 0;
	mu_edca->mu_ac_rec[ACI_AC_BK].aifsn = 15;
	mu_edca->mu_ac_rec[ACI_AC_BK].ecw_min = 10;
	mu_edca->mu_ac_rec[ACI_AC_BK].ecw_max = 10;
	mu_edca->mu_ac_rec[ACI_AC_BK].mu_edca_timer = 255;
	/*AC_BE*/
	mu_edca->mu_ac_rec[ACI_AC_BE].acm = 0;
	mu_edca->mu_ac_rec[ACI_AC_BE].aifsn = 15;
	mu_edca->mu_ac_rec[ACI_AC_BE].ecw_min = 10;
	mu_edca->mu_ac_rec[ACI_AC_BE].ecw_max = 10;
	mu_edca->mu_ac_rec[ACI_AC_BE].mu_edca_timer = 255;
	/*AC_VI*/
	mu_edca->mu_ac_rec[ACI_AC_VI].acm = 0;
	mu_edca->mu_ac_rec[ACI_AC_VI].aifsn = 15;
	mu_edca->mu_ac_rec[ACI_AC_VI].ecw_min = 10;
	mu_edca->mu_ac_rec[ACI_AC_VI].ecw_max = 10;
	mu_edca->mu_ac_rec[ACI_AC_VI].mu_edca_timer = 255;
	/*AC_VO*/
	mu_edca->mu_ac_rec[ACI_AC_VI].acm = 0;
	mu_edca->mu_ac_rec[ACI_AC_VO].aifsn = 15;
	mu_edca->mu_ac_rec[ACI_AC_VO].ecw_min = 10;
	mu_edca->mu_ac_rec[ACI_AC_VO].ecw_max = 10;
	mu_edca->mu_ac_rec[ACI_AC_VO].mu_edca_timer = 255;
}

VOID he_cfg_init(struct he_cfg *obj)
{
	obj->bw = HE_BW_80;
	obj->tx_stbc = 1;
	obj->rx_stbc = 1;
	obj->ldpc = 1;
	obj->he_vhtop = 0;
	obj->he6g_op = 0;
	obj->tx_nss = 2;
	obj->rx_nss = 2;
	obj->txop_duration = DISABLE_TXOP_DURATION_RTS_THRESHOLD;
	obj->twt_support = 0;
	obj->ppdu_tx_type = 0xff;
	obj->ofdma_usr_num = 0;
	obj->non_tx_bss_idx = 0;
	obj->ofdma_dir = 0;
	obj->gi = GI_AUTO;
	he_cfg_mu_edca_init(&obj->mu_edca_param_set);
}

VOID he_cfg_exit(struct he_cfg *obj)
{
	os_zero_mem(obj, sizeof(struct he_cfg));
}

/*
 * SET function
 */
VOID wlan_config_set_he_bw(struct wifi_dev *wdev, UINT8 he_bw)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->he_conf.bw = he_bw;
}

VOID wlan_config_set_he_gi(struct wifi_dev *wdev, UINT8 he_gi)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->he_conf.gi = he_gi;
	wdev->HEPhyMode.field.ShortGI = he_gi;
}

VOID wlan_config_set_he_txop_dur_rts_thld(struct wifi_dev *wdev, UINT32 txop_dur_thld)
{
	struct wlan_config *cfg = NULL;

	cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->he_conf.txop_duration = txop_dur_thld;
}

VOID wlan_config_set_he_ldpc(struct wifi_dev *wdev, UINT8 he_ldpc)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->he_conf.ldpc = he_ldpc;
}

VOID wlan_config_set_he_vhtop_present(struct wifi_dev *wdev, UINT8 vhtop_en)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->he_conf.he_vhtop = vhtop_en;
}

VOID wlan_config_set_he_tx_nss(struct wifi_dev *wdev, UINT8 tx_nss)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->he_conf.tx_nss = tx_nss;
}

VOID wlan_config_set_he_rx_nss(struct wifi_dev *wdev, UINT8 rx_nss)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->he_conf.rx_nss = rx_nss;
}

#ifdef WIFI_TWT_SUPPORT
VOID wlan_config_set_he_twt_support(struct wifi_dev *wdev, UINT8 twt_support)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->he_conf.twt_support = twt_support;
}

VOID wlan_config_set_he_twt_info_frame(struct wifi_dev *wdev, UINT8 twt_info_frame)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->he_conf.twt_info_frame = twt_info_frame;
}

#endif /* WIFI_TWT_SUPPORT */

VOID wlan_config_set_mu_dl_ofdma(struct wifi_dev *wdev, UINT8 enable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->phy_conf.mu_dl_ofdma = enable;
}

VOID wlan_config_set_mu_ul_ofdma(struct wifi_dev *wdev, UINT8 enable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->phy_conf.mu_ul_ofdma = enable;
}

VOID wlan_config_set_mu_dl_mimo(struct wifi_dev *wdev, UINT8 enable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->phy_conf.mu_dl_mimo = enable;
}

VOID wlan_config_set_mu_ul_mimo(struct wifi_dev *wdev, UINT8 enable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->phy_conf.mu_ul_mimo = enable;
}

VOID wlan_config_set_ul_mu_data_disable_rx(struct wifi_dev *wdev, UINT8 enable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->phy_conf.ul_mu_data_disable_rx = enable;
}

VOID wlan_config_set_er_su_rx_disable(struct wifi_dev *wdev, UINT8 enable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->phy_conf.er_su_rx_disable = enable;
}

VOID wlan_config_set_ppdu_tx_type(struct wifi_dev *wdev, UINT8 ppdu_tx_type)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->he_conf.ppdu_tx_type = ppdu_tx_type;
}

VOID wlan_config_set_ofdma_user_cnt(struct wifi_dev *wdev, UINT8 user_cnt)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->he_conf.ofdma_usr_num = user_cnt;
}

VOID wlan_config_set_mu_edca_override(struct wifi_dev *wdev, BOOLEAN mu_edca_override)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->he_conf.mu_edca_override = mu_edca_override;
}


VOID wlan_config_set_non_tx_bss_idx(struct wifi_dev *wdev, UINT8 non_tx_bss_idx)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->he_conf.non_tx_bss_idx = non_tx_bss_idx;
}

VOID wlan_config_set_ofdma_direction(struct wifi_dev *wdev, UINT8 ofdma_dir)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->he_conf.ofdma_dir = ofdma_dir;
}

VOID wlan_config_set_he6g_op_present(struct wifi_dev *wdev, UINT8 he6gop_en)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->he_conf.he6g_op = he6gop_en;
}

VOID wlan_config_set_unsolicit_tx_by_cfg(struct wifi_dev *wdev, UINT8 by_cfg)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->he_conf.ap6g.unsolicit_tx_by_cfg = by_cfg;
}

VOID wlan_config_set_unsolicit_tx_type(struct wifi_dev *wdev, UINT8 tx_type)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->he_conf.ap6g.unsolicit_tx_type = tx_type;
}

VOID wlan_config_set_unsolicit_tx_mode(struct wifi_dev *wdev, UINT8 tx_mode)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->he_conf.ap6g.unsolicit_tx_mode = tx_mode;
}

VOID wlan_config_set_unsolicit_tx_tu(struct wifi_dev *wdev, UINT8 tx_tu)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->he_conf.ap6g.unsolicit_tx_tu = tx_tu;
}

VOID wlan_config_set_qos_tx_tu(struct wifi_dev *wdev, UINT8 tx_tu)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->he_conf.ap6g.qos_tx_tu = tx_tu;
}

VOID wlan_config_set_qos_tx_state(struct wifi_dev *wdev, UINT8 state)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->he_conf.ap6g.qos_tx_state = state;
}

VOID wlan_config_set_rnr_in_probe_rsp(
	struct wifi_dev *wdev, UINT8 rnr_2g, UINT8 rnr_5g, UINT8 rnr_6g
)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->he_conf.ap6g.rnr_in_probe_2g = rnr_2g;
	cfg->he_conf.ap6g.rnr_in_probe_5g = rnr_5g;
	cfg->he_conf.ap6g.rnr_in_probe_6g = rnr_6g;
}

VOID wlan_config_set_he_dyn_smps(struct wifi_dev *wdev, UINT8 enable)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return;
	}

	cfg->he_conf.he_dyn_smps_support = enable;
}

VOID wlan_config_set_he_mu_edca(
	struct wifi_dev *wdev, UINT8 param, UINT8 aci, UINT8 value)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
	struct _EDCA_PARM *edca_param = NULL;
	struct mu_edca_cfg *mu_edca = NULL;
	BOOLEAN updated_cnt = FALSE;	/* Update Count incremented if any of the AC parameters changes */

	if (cfg && (aci < ACI_AC_NUM)) {
		edca_param = wlan_config_get_ht_edca(wdev);
		mu_edca = wlan_config_get_he_mu_edca(wdev);

		switch (param) {
		case MU_EDCA_ECW_MIN:
			if (value > 15) {
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"invalid ecw_min[%d]=%d, should not over 15\n", aci, value);
				goto err;
			}

			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"configure ecw_min[%d]=%d\n", aci, value);

			if (mu_edca->mu_ac_rec[aci].ecw_min != value) {
				mu_edca->mu_ac_rec[aci].ecw_min = value;
				updated_cnt = TRUE;
			}
			break;

		case MU_EDCA_ECW_MAX:
			if (value > 15) {
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"invalid ecw_max[%d]=%d, should not over 15\n", aci, value);
				goto err;
			}

			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"configure ecw_max[%d]=%d\n", aci, value);

			if (mu_edca->mu_ac_rec[aci].ecw_max != value) {
				mu_edca->mu_ac_rec[aci].ecw_max = value;
				updated_cnt = TRUE;
			}
			break;

		case MU_EDCA_AIFSN:
			if ((value < 2) || (value > 15)) {
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"invalid aifsn[%d]=%d, should 2 to 15\n", aci, value);
				goto err;
			}

			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"configure aifsn[%d]=%d\n", aci, value);

			if (mu_edca->mu_ac_rec[aci].aifsn != value) {
				mu_edca->mu_ac_rec[aci].aifsn = value;
				updated_cnt = TRUE;
			}
			break;

		case MU_EDCA_TIMER:
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"configure timer[%d]=%d\n", aci, value);

			/*in units of 8us */
			if (mu_edca->mu_ac_rec[aci].mu_edca_timer != value) {
				mu_edca->mu_ac_rec[aci].mu_edca_timer = value;
				updated_cnt = TRUE;
			}
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"invalid param(%d)\n", param);
			break;
		}
	}

	if (updated_cnt) {
		edca_param->EdcaUpdateCount++;
		UpdateBeaconHandler(wdev->sys_handle, wdev, BCN_UPDATE_IE_CHG);
	}

	return;

err:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid input!!\n");
}

/*
 * GET function
 */
UINT8 wlan_config_get_he_bw(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	if (cfg->ht_conf.ht_bw == HT_BW_20)
		cfg->he_conf.bw = HE_BW_20;
	if (cfg->ht_conf.ht_bw == HT_BW_40) {
		switch (cfg->vht_conf.vht_bw) {
		case VHT_BW_2040:
			cfg->he_conf.bw = HE_BW_2040;
			break;
		case VHT_BW_80:
			cfg->he_conf.bw = HE_BW_80;
			break;
		case VHT_BW_160:
			cfg->he_conf.bw = HE_BW_160;
			break;
		case VHT_BW_8080:
			cfg->he_conf.bw = HE_BW_8080;
			break;
		default:
			break;
		}
	}
	if ((cfg->he_conf.bw > HE_BW_2040) && WMODE_CAP_AX_2G(wdev->PhyMode))
		cfg->he_conf.bw = HE_BW_2040;

	return cfg->he_conf.bw;
}

UINT8 wlan_config_get_he_vhtop_present(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
	UINT8 vhtop_present;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	vhtop_present = cfg->he_conf.he_vhtop;

	return vhtop_present;
}

UINT8 wlan_config_get_he6g_op_present(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
	UINT8 he6gop_en;

	he6gop_en = cfg->he_conf.he6g_op;

	return he6gop_en;
}

UINT8 wlan_config_get_he_tx_stbc(struct wifi_dev *wdev)
{
	UINT8 tx_stbc;
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,"cfg NULL\n");
		return 0;
	}

	tx_stbc = cfg->he_conf.tx_stbc;

	return tx_stbc;
}

UINT8 wlan_config_get_he_rx_stbc(struct wifi_dev *wdev)
{
	UINT8 rx_stbc;
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	rx_stbc = cfg->he_conf.rx_stbc;

	return rx_stbc;
}

UINT8 wlan_config_get_he_ldpc(struct wifi_dev *wdev)
{
	UINT8 ldpc;
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	ldpc = cfg->he_conf.ldpc;

	return ldpc;
}

UINT8 wlan_config_get_he_tx_nss(struct wifi_dev *wdev)
{
	UINT8 tx_nss;
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	tx_nss = cfg->he_conf.tx_nss;

	return tx_nss;
}

UINT8 wlan_config_get_he_rx_nss(struct wifi_dev *wdev)
{
	UINT8 rx_nss;
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	rx_nss = cfg->he_conf.rx_nss;

	return rx_nss;
}

UINT16 wlan_config_get_he_txop_dur_rts_thld(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	/* unit 32us, 1023 indicate disable */
	return cfg->he_conf.txop_duration;
}

#ifdef WIFI_TWT_SUPPORT
UINT8 wlan_config_get_he_twt_support(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->he_conf.twt_support;
}

UINT8 wlan_config_get_he_twt_info_frame(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->he_conf.twt_info_frame;
}
#endif /* WIFI_TWT_SUPPORT */

UINT8 wlan_config_get_ppdu_tx_type(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->he_conf.ppdu_tx_type;
}


UINT8 wlan_config_get_ofdma_user_cnt(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->he_conf.ofdma_usr_num;
}

UINT8 wlan_config_get_mu_edca_override(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	return cfg->he_conf.mu_edca_override;
}

UINT8 wlan_config_get_non_tx_bss_idx(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->he_conf.non_tx_bss_idx;
}

UINT8 wlan_config_get_ofdma_direction(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->he_conf.ofdma_dir;
}

struct mu_edca_cfg *wlan_config_get_he_mu_edca(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (cfg)
		return &cfg->he_conf.mu_edca_param_set;
	else
		return NULL;
}

UINT8 wlan_config_get_mu_dl_ofdma(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.mu_dl_ofdma;
}

UINT8 wlan_config_get_mu_ul_ofdma(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.mu_ul_ofdma;
}

UINT8 wlan_config_get_mu_dl_mimo(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.mu_dl_mimo;
}

UINT8 wlan_config_get_mu_ul_mimo(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.mu_ul_mimo;
}

UINT8 wlan_config_get_ul_mu_data_disable_rx(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.ul_mu_data_disable_rx;
}

UINT8 wlan_config_get_er_su_rx_disable(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (!cfg) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cfg NULL\n");
		return 0;
	}

	return cfg->phy_conf.er_su_rx_disable;
}

UINT8 wlan_config_get_unsolicit_tx_tu(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	return cfg->he_conf.ap6g.unsolicit_tx_tu;
}

UINT8 wlan_config_get_unsolicit_tx_mode(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	return cfg->he_conf.ap6g.unsolicit_tx_mode;
}

UINT8 wlan_config_get_unsolicit_tx_by_cfg(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	return cfg->he_conf.ap6g.unsolicit_tx_by_cfg;
}

UINT8 wlan_config_get_unsolicit_tx_type(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	return cfg->he_conf.ap6g.unsolicit_tx_type;
}

UINT8 wlan_config_get_qos_tx_state(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	return cfg->he_conf.ap6g.qos_tx_state;
}

UINT8 wlan_config_get_qos_tx_tu(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	return cfg->he_conf.ap6g.qos_tx_tu;
}

UINT8 wlan_config_get_rnr_in_probe_rsp(struct wifi_dev *wdev, UCHAR rf_mode)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
	UINT8 rnr_in_probe;

	if (rf_mode == RFIC_6GHZ)
		rnr_in_probe = cfg->he_conf.ap6g.rnr_in_probe_6g;
	else if (rf_mode == RFIC_5GHZ)
		rnr_in_probe = cfg->he_conf.ap6g.rnr_in_probe_5g;
	else
		rnr_in_probe = cfg->he_conf.ap6g.rnr_in_probe_2g;

	return rnr_in_probe;
}

UCHAR wlan_config_get_he_gi(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (cfg)
		return cfg->he_conf.gi;
	else
		return 0;
}

UINT8 wlan_config_get_he_dyn_smps(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (cfg)
		return cfg->he_conf.he_dyn_smps_support;
	else
		return 0;
}
