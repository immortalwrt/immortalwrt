#include "rt_config.h"

struct capi_set_cmd ap_set_wireless_capi[] = {
	{"mcs_fixedrate", ap_set_wireless_fixed_mcs},
	{"ofdma", ap_set_wireless_ofdma_direction},
	{"ppdu_tx_type", ap_set_wireless_ppdu_tx_type},
	{"mumimo", ap_set_wireless_mumimo},
	{"num_users_ofdma", ap_set_wireless_nusers_ofdma},
	{"non_tx_bss_idx", ap_set_wireless_non_txbss_idx},
	{"mu_edca_override", ap_set_wireless_mu_edca_override},
	{"act_ind_unsolicit_probe_rsp", ap_set_wireless_act_ind_unsolicited_probe_rsp},
	{"ap6g_discovery_apply", ap_set_wireless_discovery_apply},
	{"non_support", ap_set_wireless_non_support} /* End of ap_set_wireless */
};

struct capi_set_cmd ap_set_rfeature_capi[] = {
	{"ack_type", ap_set_rfeature_ack_type},
	{"ack_policy", ap_set_rfeature_ack_policy},
	{"ppdu_tx_type", ap_set_rfeature_ppdu_tx_type},
	{"trig_type", ap_set_rfeature_trig_type},
	{"he_ltf", ap_set_rfeature_he_ltf},
	{"he_gi", ap_set_rfeature_he_gi},
	{"trig_txbf", ap_set_rfeature_trig_txbf},
	{"disable_trig_type", ap_set_rfeature_dis_trig_type},
	{"ofdma", ap_set_rfeature_ofdma_direction},
	{"chnum_band", ap_set_rfeature_chnum_band},
	{"tx_bandwidth", ap_set_rfeature_tx_bandwidth},
	{"ignore_nav", ap_set_rfeature_ignore_nav},
	{"unsolicit_probe_rsp", ap_set_rfeature_unsolicited_probe_rsp},
	{"fils_discovery", ap_set_rfeature_fils_discovery},
	{"qos_null_injector", ap_set_rfeature_qos_null_injector},
	{"bss_max_idle_ie", ap_set_rfeature_bss_max_idle_ie},
	{"bss_max_idle_period", ap_set_rfeature_bss_max_idle_period},
	{"bss_max_idle_option", ap_set_rfeature_bss_max_idle_option},
	/* STA MU EDCA */
	{"muedca_ecwmin_vo", ap_set_rfeature_mu_edca_ecw_min_vo},
	{"muedca_ecwmin_vi", ap_set_rfeature_mu_edca_ecw_min_vi},
	{"muedca_ecwmin_be", ap_set_rfeature_mu_edca_ecw_min_be},
	{"muedca_ecwmin_bk", ap_set_rfeature_mu_edca_ecw_min_bk},
	{"muedca_ecwmax_vo", ap_set_rfeature_mu_edca_ecw_max_vo},
	{"muedca_ecwmax_vi", ap_set_rfeature_mu_edca_ecw_max_vi},
	{"muedca_ecwmax_be", ap_set_rfeature_mu_edca_ecw_max_be},
	{"muedca_ecwmax_bk", ap_set_rfeature_mu_edca_ecw_max_bk},
	{"muedca_aifsn_vo", ap_set_rfeature_mu_edca_aifsn_vo},
	{"muedca_aifsn_vi", ap_set_rfeature_mu_edca_aifsn_vi},
	{"muedca_aifsn_be", ap_set_rfeature_mu_edca_aifsn_be},
	{"muedca_aifsn_bk", ap_set_rfeature_mu_edca_aifsn_bk},
	{"muedca_timer_vo", ap_set_rfeature_mu_edca_timer_vo},
	{"muedca_timer_vi", ap_set_rfeature_mu_edca_timer_vi},
	{"muedca_timer_be", ap_set_rfeature_mu_edca_timer_be},
	{"muedca_timer_bk", ap_set_rfeature_mu_edca_timer_bk},
	/* STA WMM PE */
	{"wmmpe_ecwmin_vo", ap_set_rfeature_wmm_pe_ecw_min_vo},
	{"wmmpe_ecwmin_vi", ap_set_rfeature_wmm_pe_ecw_min_vi},
	{"wmmpe_ecwmin_be", ap_set_rfeature_wmm_pe_ecw_min_be},
	{"wmmpe_ecwmin_bk", ap_set_rfeature_wmm_pe_ecw_min_bk},
	{"wmmpe_ecwmax_vo", ap_set_rfeature_wmm_pe_ecw_max_vo},
	{"wmmpe_ecwmax_vi", ap_set_rfeature_wmm_pe_ecw_max_vi},
	{"wmmpe_ecwmax_be", ap_set_rfeature_wmm_pe_ecw_max_be},
	{"wmmpe_ecwmax_bk", ap_set_rfeature_wmm_pe_ecw_max_bk},
	{"wmmpe_aifsn_vo", ap_set_rfeature_wmm_pe_aifsn_vo},
	{"wmmpe_aifsn_vi", ap_set_rfeature_wmm_pe_aifsn_vi},
	{"wmmpe_aifsn_be", ap_set_rfeature_wmm_pe_aifsn_be},
	{"wmmpe_aifsn_bk", ap_set_rfeature_wmm_pe_aifsn_bk},
	{"wmmpe_txop_vo", ap_set_rfeature_wmm_pe_txop_vo},
	{"wmmpe_txop_vi", ap_set_rfeature_wmm_pe_txop_vi},
	{"wmmpe_txop_be", ap_set_rfeature_wmm_pe_txop_be},
	{"wmmpe_txop_bk", ap_set_rfeature_wmm_pe_txop_bk},
	/* AP WMM PE */
	{"ap_wmmpe_txop_vo", ap_set_rfeature_ap_wmm_pe_txop_vo},
	{"ap_wmmpe_txop_vi", ap_set_rfeature_ap_wmm_pe_txop_vi},
	{"ap_wmmpe_txop_be", ap_set_rfeature_ap_wmm_pe_txop_be},
	{"ap_wmmpe_txop_bk", ap_set_rfeature_ap_wmm_pe_txop_bk},

	{"non_support", ap_set_rfeature_non_support} /* End of ap_set_rfeature */
};

struct capi_set_cmd dev_send_frame_capi[] = {
	{"smpdu", dev_send_frame_smpdu},
	{"unicast_deauth", dev_send_frame_uncast_deauth},
	{"non_support", dev_send_frame_non_support} /* End of dev_send_frame */
};

INT32 dev_send_frame(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	UINT32 i;
	UCHAR *capi_s = NULL;
	INT32 ret = STATUS_SUCCESS;

	capi_s = strsep(&arg, "-");
	MTWF_PRINT("func:%s, capi: %s, arg: %s\n", __func__, capi_s, arg);
	if ((capi_s != NULL) && (strlen(capi_s) > 0)) {
		for (i = 0; i < (sizeof(dev_send_frame_capi)/sizeof(struct capi_set_cmd)); i++) {
			if (strcmp((dev_send_frame_capi + i)->capi, capi_s) == 0) {
				MTWF_PRINT("func:%s, [capi match], arg: %s\n", __func__, arg);
				(dev_send_frame_capi + i)->set_func(ad, arg);
				break;
			}
		}
	}

	return ret;
}

INT32 set_ap_wireless(struct _RTMP_ADAPTER *ad, RTMP_STRING *capi, VOID *param)
{
	UINT32 i;
	INT32 ret = STATUS_SUCCESS;

	for (i = 0; i < (sizeof(ap_set_wireless_capi)/sizeof(struct capi_set_cmd)); i++) {
		if (strcmp((ap_set_wireless_capi + i)->capi, capi) == 0) {
			(ap_set_wireless_capi + i)->set_func(ad, param);
			break;
		}
	}

	return ret;
}

INT32 set_ap_rfeatures(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	UINT32 i;
	UCHAR *capi_s = NULL;
	INT32 ret = STATUS_SUCCESS;
	UINT16 param;

	capi_s = strsep(&arg, "-");
	MTWF_PRINT("func:%s, capi: %s, arg: %s\n", __func__, capi_s, arg);
	if ((capi_s != NULL) && (strlen(capi_s) > 0)) {
		for (i = 0; i < (sizeof(ap_set_rfeature_capi)/sizeof(struct capi_set_cmd)); i++) {
			if (strcmp((ap_set_rfeature_capi + i)->capi, capi_s) == 0) {
				param = (UINT16)os_str_tol(arg, 0, 10);
				MTWF_PRINT("func:%s, [capi match], arg: %d\n", __func__, param);
				(ap_set_rfeature_capi + i)->set_func(ad, &param);
				break;
			}
		}
	}

	return ret;
}

static struct _MAC_TABLE_ENTRY *
get_mac_entry_by_mac_addr(struct _RTMP_ADAPTER *ad, RTMP_STRING *str)
{
	UCHAR mac_addr[MAC_ADDR_LEN];
	CHAR *token;
	INT i = 0;

	token = rstrtok(str, ":");

	while (token != NULL) {
		AtoH(token, (char *) &mac_addr[i], 1);
		i++;
		if (i >= MAC_ADDR_LEN)
			break;
		token = rstrtok(NULL, ":");
	}

	return MacTableLookup(ad, mac_addr);
}

/* dev_send_frame */
VOID dev_send_frame_smpdu(VOID *ad, VOID *arg)
{
#ifdef DOT11_HE_AX
	UINT16 wcid = WCID_INVALID;
	UINT ArgIdx;
	RTMP_STRING *thisChar;
	UINT8 txopDur = 0, interval = 32;
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)ad;
	RTMP_STRING *argument = (RTMP_STRING *)arg;

	ArgIdx = 0;

	while ((thisChar = strsep((char **)&argument, "!")) != NULL) {
		switch (ArgIdx) {
		case 0:	/* Target Peer's MAC Addr */
			if (strlen(thisChar) == 17) {
				MAC_TABLE_ENTRY *pEntry = get_mac_entry_by_mac_addr(pAd, thisChar);

				if (pEntry != NULL)
					wcid = pEntry->wcid;
			}

			if (!IS_WCID_VALID(pAd, wcid)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, "unknown sta of wcid(%d)\n", wcid);
				return;
			}
			break;

		case 1: /* frame Duration. */
			txopDur = (UINT8) os_str_tol(thisChar, 0, 10);
			break;

		case 2: /* frame Interval. */
			interval = (UINT8) os_str_tol(thisChar, 0, 10);
			break;
		}
		ArgIdx++;
	}
	if (ArgIdx != 3) {
		return;
	}

#ifdef CFG_SUPPORT_FALCON_MURU
	/* In band CMD to inform FW*/
	set_muru_cert_send_frame_ctrl(pAd, txopDur, wcid, interval);
#endif /* CFG_SUPPORT_FALCON_MURU */

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_DEBUG, "wcid = %d txop %d interval %d\n", wcid, txopDur, interval);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_DEBUG, "\n");
#endif /* DOT11_HE_AX */
}

VOID dev_send_frame_uncast_deauth(VOID *ad, VOID *arg)
{
	struct _RTMP_ADAPTER *rad = (struct _RTMP_ADAPTER *) ad;
	RTMP_STRING *str = (RTMP_STRING *) arg;
	RTMP_STRING *this_char;
	INT protect;
	RTMP_STRING *mac = (RTMP_STRING *) strsep((char **) &str, "-");
	struct _MAC_TABLE_ENTRY *entry = get_mac_entry_by_mac_addr(rad, mac);

	this_char = strsep((char **) &str, "-");
	if (this_char == NULL) {
		MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid input arg!\n");
		return;
	}
	protect = os_str_tol(this_char, 0, 10);

	if (!entry)
		return;

	ap_send_unicast_deauth(rad, entry->wdev, entry, (bool)protect);
}


VOID dev_send_frame_non_support(VOID *ad, VOID *arg)
{
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_WARN,
			"Not Support !\n");
	return;
}

/* ap_set_wireless */
VOID ap_set_wireless_fixed_mcs(VOID *ad, VOID *param)
{
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	struct _MAC_TABLE_ENTRY *entry = (struct _MAC_TABLE_ENTRY *)param;
	struct wifi_dev *wdev = NULL;
	UINT8 mcs = CAPI_MCS_AUTO;
	UINT32 wcid;

	if (!entry)
		return;
	wcid = entry->wcid;
	wdev = entry->wdev;
	if (!wdev)
		return;

	mcs = wlan_config_get_fixed_mcs(wdev);

	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%u\n", mcs);

	if (mcs < CAPI_MCS_AUTO)
		snd_ra_fw_cmd(RA_PARAM_MCS_UPDATE, adapt, wcid, &mcs);

	return;
}

VOID ap_set_wireless_ofdma_direction(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	struct wifi_dev *wdev = (struct wifi_dev *)param;
	UINT8 ofdma_dir = CAPI_OFDMA_AUTO;

	if (!wdev)
		return;
	ofdma_dir = wlan_config_get_ofdma_direction(wdev);

	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%u\n", ofdma_dir);
	if (ofdma_dir == CAPI_OFDMA_AUTO)
		return;
#ifdef CFG_SUPPORT_FALCON_MURU
	else {
		SetMuruProtFrameThr(adapt, "9999");

		if (ofdma_dir == CAPI_OFDMA_DL_20n80)
			SetMuru20MDynAlgo(adapt, "1");
	}
#endif /*CFG_SUPPORT_FALCON_MURU*/
#endif /*DOT11_HE_AX*/
	return;
}

VOID ap_set_wireless_ppdu_tx_type(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	struct wifi_dev *wdev = (struct wifi_dev *)param;
	UINT8 ppdu_type;

	if (!wdev)
		return;
	ppdu_type = wlan_config_get_ppdu_tx_type(wdev);
#ifdef CFG_SUPPORT_FALCON_MURU
	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"val:%d\n", ppdu_type);
	if (ppdu_type == CAPI_LEGACY)
		return;
	else if (ppdu_type == CAPI_SU) {
		char muru_config0[] = "dl_comm_user_cnt:0";
		/*iwpriv ra0 set set_muru_sutx=1*/
		SetMuruSuTx(adapt, "1");
		set_muru_manual_config(adapt, muru_config0);
		set_muru_manual_config(adapt, "update");
	} else if (ppdu_type == CAPI_MU) {
		char muru_config2[] = "dl_comm_user_cnt:2";
		/*iwpriv ra0 set set_muru_sutx=0*/
		SetMuruSuTx(adapt, "0");
		/*iwpriv ra0 set set_muru_manual_config=dl_comm_user_cnt:2*/
		set_muru_manual_config(adapt, muru_config2);
		/*iwpriv ra0 set set_muru_manual_config=update*/
		set_muru_manual_config(adapt, "update");
	} else {
		return;
	}
#endif /*CFG_SUPPORT_FALCON_MURU*/
#endif /*DOT11_HE_AX*/
	return;
}

VOID ap_set_wireless_mumimo(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	struct wifi_dev *wdev = (struct wifi_dev *)param;
	char str[20] = {0}, mu_fixed_rate_en[] = "1";
	char mimo_param[] = "2-122-0-1-0-1-2-2-2";/* <2Usr>-<RU61>-<3.2GI>-<HEFB>-<DL>-<WCID>-<MCS2> */

	UINT8 mu_dl_mimo;
	UINT8 ppdu_type;
	int ret;

	if (!wdev)
		return;
	mu_dl_mimo = wlan_config_get_mu_dl_mimo(wdev);
	ppdu_type = wlan_config_get_ppdu_tx_type(wdev);
#ifdef CFG_SUPPORT_FALCON_MURU
	if (mu_dl_mimo && (ppdu_type == CAPI_MU)) {
		ret = snprintf(str, sizeof(str), "%s", "dl_init");
		if (os_snprintf_error(sizeof(str), ret)) {
			MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
				"str snprintf error!\n");
			return;
		}
	} else
		return;

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"cmd:%s\n", str);
	set_muru_manual_config(adapt, str);
	set_muru_manual_config(adapt, "update");

	/* Set default value as SUBAR. */
	set_muru_mudl_ack_policy(adapt, MURU_CMD_MUDL_ACK_POLICY_SU_BAR);

	if ((wlan_config_get_ht_bw(wdev) == BW_40)
#ifdef DOT11_VHT_AC
			&& (wlan_config_get_vht_bw(wdev) == VHT_BW_80)
#endif /* DOT11_VHT_AC */
		) {
			ret = snprintf(mimo_param, sizeof(mimo_param), "%s", "2-134-0-1-0-1-2-2-2");/* <2Usr>-<RU67>-<3.2GI>-<HEFB>-<DL>-<WCID>-<MCS2> */
			if (os_snprintf_error(sizeof(mimo_param), ret)) {
				MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
					"mimo_param snprintf error!\n");
				return;
			}
		}
#ifdef DOT11_HE_AX
	else if (wlan_config_get_he_bw(wdev) == HE_BW_160) {
		ret = snprintf(mimo_param, sizeof(mimo_param), "%s", "2-137-0-1-0-1-2-2-2");/* <2Usr>-<RU67>-<3.2GI>-<HEFB>-<DL>-<WCID>-<MCS2> */
		if (os_snprintf_error(sizeof(mimo_param), ret)) {
			MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
				"mimo_param 2 snprintf error!\n");
			return;
		}
	}
#endif /* DOT11_HE_AX */

	/* Fix MU rate as MCS2. */
	SetMuMimoFixedRate(adapt, mu_fixed_rate_en);
	SetMuMiMoFixedGroupRateProc(adapt, mimo_param);
	/* Force MUCOP to MU */
	SetMuMimoForceMUEnable(adapt, MUMIMO_FORCE_ENABLE_MU);
#endif /*CFG_SUPPORT_FALCON_MURU*/
#endif /*DOT11_HE_AX*/
	return;
}

VOID ap_set_wireless_nusers_ofdma(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	struct wifi_dev *wdev = (struct wifi_dev *)param;
	char str[20];
	UINT8 nuser = 0;
	int ret;

	if (!wdev)
		return;
	nuser = wlan_config_get_ofdma_user_cnt(wdev);
#ifdef CFG_SUPPORT_FALCON_MURU
	if (nuser < 2)
		return;
	SetMuruSuTx(adapt, "0");
	ret = snprintf(str, sizeof(str), "%s:%d", "dl_comm_user_cnt", nuser);
	if (os_snprintf_error(sizeof(str), ret)) {
		MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
			"str 2 snprintf error!\n");
		return;
	}
	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"cmd:%s\n", str);
	/* Set default value as SUBAR. */
	set_muru_mudl_ack_policy(adapt, MURU_CMD_MUDL_ACK_POLICY_SU_BAR);
	set_muru_manual_config(adapt, str);
	set_muru_manual_config(adapt, "update");
#endif /*CFG_SUPPORT_FALCON_MURU*/
#endif /*DOT11_HE_AX*/
	return;
}

VOID ap_set_wireless_mu_edca_override(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	struct wifi_dev *wdev = (struct wifi_dev *)param;
	UINT8 capi_override = 0;

	MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

	if (!wdev)
		return;
	capi_override = wlan_config_get_mu_edca_override(wdev);
#ifdef CFG_SUPPORT_FALCON_MURU
	set_muru_cert_muedca_override(adapt, capi_override);
#endif /*CFG_SUPPORT_FALCON_MURU*/
#endif /*DOT11_HE_AX*/
	return;
}

/*
 * This api is including:
 * unsolicited_probe_rsp / cadence_unsolicited_probe_rsp / fils_discovery
 */
VOID ap_set_wireless_discovery_apply(VOID *ad, VOID *param)
{
#if defined(CONFIG_AP_SUPPORT) && defined(DOT11_HE_AX) && defined(CONFIG_6G_SUPPORT)
	struct wifi_dev *wdev = (struct wifi_dev *)param;
	UINT8 unsolicit_type = 0, tu = 20, unsolicit_txmode = 0;

	if (!wdev)
		return;

	if (WMODE_CAP_6G(wdev->PhyMode)) {
		tu = wlan_config_get_unsolicit_tx_tu(wdev);
		unsolicit_type = wlan_config_get_unsolicit_tx_type(wdev);
		unsolicit_txmode = wlan_config_get_unsolicit_tx_mode(wdev);

		in_band_discovery_update(wdev, unsolicit_type, tu, unsolicit_txmode, TRUE);
	}
#endif /* defined(CONFIG_AP_SUPPORT) && defined(DOT11_HE_AX) && defined(CONFIG_6G_SUPPORT) */
	return;
}

VOID ap_set_wireless_act_ind_unsolicited_probe_rsp(VOID *ad, VOID *param)
{
#if defined(CONFIG_AP_SUPPORT) && defined(DOT11_HE_AX) && defined(CONFIG_6G_SUPPORT)
	struct wifi_dev *wdev = (struct wifi_dev *)param;
	UINT8 rnr_in_probe_2g = wlan_config_get_rnr_in_probe_rsp(wdev, RFIC_24GHZ);
	UINT8 rnr_in_probe_5g = wlan_config_get_rnr_in_probe_rsp(wdev, RFIC_5GHZ);
	UINT8 rnr_in_probe_6g = wlan_config_get_rnr_in_probe_rsp(wdev, RFIC_6GHZ);

	/* RNR in Unsolicited Probe Rsp */
	out_band_discovery_update(wdev, rnr_in_probe_2g, rnr_in_probe_5g, rnr_in_probe_6g);
#endif /* defined(CONFIG_AP_SUPPORT) && defined(DOT11_HE_AX) && defined(CONFIG_6G_SUPPORT) */
	return;
}

VOID ap_set_wireless_non_txbss_idx(VOID *ad, VOID *param)
{
	return;
}

VOID ap_set_wireless_non_support(VOID *ad, VOID *param)
{
}

/* export APIs: ap_set_wireless_sta */
VOID ap_set_wireless_sta_configs(
		struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry)
{
	set_ap_wireless(ad, "mcs_fixedrate", entry);
}

/* export APIs: ap_set_wireless_bss */
VOID ap_set_wireless_bss_configs(
		struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
#ifdef DOT11_HE_AX
	set_ap_wireless(ad, "ppdu_tx_type", wdev);
	set_ap_wireless(ad, "ofdma", wdev);
	set_ap_wireless(ad, "mumimo", wdev);
	set_ap_wireless(ad, "num_users_ofdma", wdev);
	set_ap_wireless(ad, "non_tx_bss_idx", wdev);
	set_ap_wireless(ad, "mu_edca_override", wdev);
	set_ap_wireless(ad, "act_ind_unsolicit_probe_rsp", wdev);
	set_ap_wireless(ad, "ap6g_discovery_apply", wdev);
#endif /*DOT11_HE_AX*/
}


/* ap_set_rfeature */
VOID ap_set_rfeature_ack_type(VOID *ad, VOID *param)
{
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_WARN,
			"Not Support ! arg:%d\n", *((UINT8 *)param));
	return;
}

VOID ap_set_rfeature_ack_policy(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT8 ack_policy = *((UINT8 *)param);

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%d\n", ack_policy);
#ifdef CFG_SUPPORT_FALCON_MURU
	if ((ack_policy == ACK_POLICY_NORMAL_ACK_IMPLICIT_BA_REQ)
		|| (ack_policy == ACK_POLICY_BA)) {
		/* Set default value as SUBAR. */
		set_muru_mudl_ack_policy(adapt, MURU_CMD_MUDL_ACK_POLICY_SU_BAR);
	} else
		set_muru_mudl_ack_policy(adapt, ack_policy);

#endif /*CFG_SUPPORT_FALCON_MURU*/
#endif /*DOT11_HE_AX*/
	return;
}

VOID ap_set_rfeature_ppdu_tx_type(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT8 ppdu_type = *((UINT8 *)param);

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%d\n", ppdu_type);
#ifdef CFG_SUPPORT_FALCON_MURU
	if (ppdu_type == CAPI_LEGACY)
		return;
	else if (ppdu_type == CAPI_SU) {
		char muru_config0[] = "dl_comm_user_cnt:0";
		/*iwpriv ra0 set set_muru_sutx=1*/
		SetMuruSuTx(adapt, "1");
		set_muru_manual_config(adapt, muru_config0);
		set_muru_manual_config(adapt, "update");
	} else if (ppdu_type == CAPI_MU) {
		char muru_config2[] = "dl_comm_user_cnt:2";
		/*iwpriv ra0 set set_muru_sutx=0*/
		SetMuruSuTx(adapt, "0");
		/*iwpriv ra0 set set_muru_manual_config=dl_comm_user_cnt:2*/
		set_muru_manual_config(adapt, muru_config2);
		/*iwpriv ra0 set set_muru_manual_config=update*/
		set_muru_manual_config(adapt, "update");
	} else {
		return;
	}
#endif /*CFG_SUPPORT_FALCON_MURU*/
#endif /*DOT11_HE_AX*/
	return;
}

VOID ap_set_rfeature_trig_type(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT8 trig_type = *((UINT8 *)param);

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%d\n", trig_type);
#ifdef CFG_SUPPORT_FALCON_MURU
	/* EXT_CID, EXT_CMD_ID_MURU_CTRL, MURU_SET_TRIG_TYPE */
	set_muru_trig_type(adapt, trig_type);
	if ((trig_type == CAPI_BASIC)) {
		char mu_ru_bsrp[] = "1-0-5-67-0";
		/*iwpriv ra0 set set_muru_bsrp_ctrl=1-0-5-67-0;*/
		SetMuruBsrpCtrl(adapt, mu_ru_bsrp); /* set timer 5 ms */
	} else if (trig_type == CAPI_BSRP) {
		char mu_ru_bsrp[] = "1-0-5-67-4";
		/*iwpriv ra0 set set_muru_bsrp_ctrl=1-0-5-67-4;*/
		SetMuruBsrpCtrl(adapt, mu_ru_bsrp); /* set timer 5 ms */
	} else if (trig_type == CAPI_MU_BAR) {
		set_muru_mudl_ack_policy(adapt, MURU_CMD_MUDL_ACK_POLICY_MU_BAR);
	}
#ifdef TXBF_SUPPORT
	else if (trig_type == CAPI_BRP) {
		char bfrp[] = "01:00:00:1B";

		Set_TxBfTxSndInfo((struct _RTMP_ADAPTER *)ad, bfrp);
	}
#endif /* TXBF_SUPPORT */
#endif /*CFG_SUPPORT_FALCON_MURU*/
#endif /*DOT11_HE_AX*/
	return;
}

VOID ap_set_rfeature_he_ltf(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT32 wcid = CAPI_ALL_STA, startWcid = 0;
	UINT8 he_ltf[CAPI_HE_LTF_NUM] = {
		(LTF_1x << BW20_HE_LTF_SHIFT)|(LTF_1x << BW40_HE_LTF_SHIFT)|(LTF_1x << BW80_HE_LTF_SHIFT)|(LTF_1x << BW160_HE_LTF_SHIFT),
		(LTF_2x << BW20_HE_LTF_SHIFT)|(LTF_2x << BW40_HE_LTF_SHIFT)|(LTF_2x << BW80_HE_LTF_SHIFT)|(LTF_2x << BW160_HE_LTF_SHIFT),
		(LTF_4x << BW20_HE_LTF_SHIFT)|(LTF_4x << BW40_HE_LTF_SHIFT)|(LTF_4x << BW80_HE_LTF_SHIFT)|(LTF_4x << BW160_HE_LTF_SHIFT),
	};
	UINT8 ltf = *((UINT8 *)param);
	MAC_TABLE_ENTRY *pEntry = NULL;
	MAC_TABLE *pMacTable = NULL;
	UINT16 max_sta_num = 0;

	if (ltf >= CAPI_HE_LTF_NUM)
		return;

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%d ra_setting=0x%08x\n", ltf, he_ltf[ltf]);

	pMacTable = &adapt->MacTab;
	max_sta_num = HcGetMaxStaNum(adapt);
	{
		/* loop for all link-up station */
		for (wcid = startWcid; VALID_UCAST_ENTRY_WCID(adapt, wcid); wcid++) {
			pEntry = &pMacTable->Content[wcid];

			if (wcid > max_sta_num)
				break;

			if (IS_ENTRY_NONE(pEntry) || IS_ENTRY_MCAST(pEntry))
				continue;

			if (pEntry && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)))
				snd_ra_fw_cmd(RA_PARAM_HELTF_UPDATE, adapt, wcid, &he_ltf[ltf]);
		}
	}

#endif /*DOT11_HE_AX*/
	return;
}

VOID ap_set_rfeature_he_gi(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT32 wcid = CAPI_ALL_STA, startWcid = 0;
	UINT8 he_gi[CAPI_HE_GI_NUM] = {
		((GI_08_US << BW20_HE_GI_SHIFT)|(GI_08_US << BW40_HE_GI_SHIFT)|(GI_08_US << BW80_HE_GI_SHIFT)|(GI_08_US << BW160_HE_GI_SHIFT)),
		((GI_16_US << BW20_HE_GI_SHIFT)|(GI_16_US << BW40_HE_GI_SHIFT)|(GI_16_US << BW80_HE_GI_SHIFT)|(GI_16_US << BW160_HE_GI_SHIFT)),
		((GI_32_US << BW20_HE_GI_SHIFT)|(GI_32_US << BW40_HE_GI_SHIFT)|(GI_32_US << BW80_HE_GI_SHIFT)|(GI_32_US << BW160_HE_GI_SHIFT)),
	};
	UINT8 gi = *((UINT8 *)param);
	MAC_TABLE_ENTRY *pEntry = NULL;
	MAC_TABLE *pMacTable = NULL;
	UINT16 max_sta_num = 0;

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%d\n", gi);

	pMacTable = &adapt->MacTab;
	max_sta_num = HcGetMaxStaNum(adapt);
	{
		/* loop for all link-up station */
		for (wcid = startWcid; VALID_UCAST_ENTRY_WCID(adapt, wcid); wcid++) {
			pEntry = &pMacTable->Content[wcid];

			if (wcid > max_sta_num)
				break;

			if (IS_ENTRY_NONE(pEntry) || IS_ENTRY_MCAST(pEntry))
				continue;

			if (pEntry && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)))
				snd_ra_fw_cmd(RA_PARAM_GI_UPDATE, adapt, wcid, &he_gi[gi]);
		}
	}
#endif /*DOT11_HE_AX*/
	return;
}

#if defined(DOT11_HE_AX) && defined(FIXED_HE_GI_SUPPORT)
VOID ap_set_he_fixed_gi_ltf_by_wcid_or_bss(RTMP_ADAPTER *pAd, UINT8 gi, UINT8 mode, UINT32 wcid, struct wifi_dev *wdev)
{
	UINT32 u_wcid = CAPI_ALL_STA;
	UINT32 startWcid = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
	MAC_TABLE *pMacTable = NULL;
	struct wifi_dev *entry_wdev = NULL;
	UINT16 max_sta_num = 0;
	UINT8 ltf = gi;
	UINT8 he_gi[CAPI_HE_GI_NUM] = {
		((GI_08_US << BW20_HE_GI_SHIFT)|(GI_08_US << BW40_HE_GI_SHIFT)|(GI_08_US << BW80_HE_GI_SHIFT)|(GI_08_US << BW160_HE_GI_SHIFT)),
		((GI_16_US << BW20_HE_GI_SHIFT)|(GI_16_US << BW40_HE_GI_SHIFT)|(GI_16_US << BW80_HE_GI_SHIFT)|(GI_16_US << BW160_HE_GI_SHIFT)),
		((GI_32_US << BW20_HE_GI_SHIFT)|(GI_32_US << BW40_HE_GI_SHIFT)|(GI_32_US << BW80_HE_GI_SHIFT)|(GI_32_US << BW160_HE_GI_SHIFT)),
	};
	UINT8 he_ltf[CAPI_HE_LTF_NUM] = {
		(LTF_2x << BW20_HE_LTF_SHIFT)|(LTF_2x << BW40_HE_LTF_SHIFT)|(LTF_2x << BW80_HE_LTF_SHIFT)|(LTF_2x << BW160_HE_LTF_SHIFT),
		(LTF_2x << BW20_HE_LTF_SHIFT)|(LTF_2x << BW40_HE_LTF_SHIFT)|(LTF_2x << BW80_HE_LTF_SHIFT)|(LTF_2x << BW160_HE_LTF_SHIFT),
		(LTF_4x << BW20_HE_LTF_SHIFT)|(LTF_4x << BW40_HE_LTF_SHIFT)|(LTF_4x << BW80_HE_LTF_SHIFT)|(LTF_4x << BW160_HE_LTF_SHIFT),
	};
	PCHAR gi_ltf_info[3] = {
	"GI_08_US + LTF_2x",
	"GI_16_US + LTF_2x",
	"GI_32_US + LTF_4x"
	};

	MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO, "Enter.\n");
	if (!pAd)
		return;
	if (gi >= GI_MASK) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR, "Invalid gi value(%d)!!!.\n", gi);
		return;
	}

	switch (mode) {
		case GI_BY_WCID:
			if (VALID_UCAST_ENTRY_WCID(pAd, wcid)) {
				snd_ra_fw_cmd(RA_PARAM_GI_UPDATE, pAd, wcid, &he_gi[gi]);
				snd_ra_fw_cmd(RA_PARAM_HELTF_UPDATE, pAd, wcid, &he_ltf[ltf]);
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
					"Set %s by wcid success.\n", gi_ltf_info[gi]);
			}
			else
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO, "Invalid wcid value(%d)!!!.\n", wcid));
			break;
		case GI_BY_BSS:
			if (!wdev) {
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR, "Invalid wdev!!!.\n");
				return;
			}
			pMacTable = &pAd->MacTab;
			max_sta_num = HcGetMaxStaNum(pAd);
			for (u_wcid = startWcid; VALID_UCAST_ENTRY_WCID(pAd, u_wcid); u_wcid++) {
				pEntry = &pMacTable->Content[u_wcid];
				if (u_wcid > max_sta_num)
					break;
				if (!(pEntry && (IS_ENTRY_CLIENT(pEntry))))
					continue;
				entry_wdev = pEntry->wdev;
				if (!(entry_wdev && (entry_wdev == wdev)))
					continue;
				snd_ra_fw_cmd(RA_PARAM_GI_UPDATE, pAd, u_wcid, &he_gi[gi]);
				snd_ra_fw_cmd(RA_PARAM_HELTF_UPDATE, pAd, wcid, &he_ltf[ltf]);
			}
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
					"Set %s by BSS success.\n", gi_ltf_info[gi]);
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR, "Wrong gi mode(%d)!!!.\n", mode);
			break;
	}
}
#endif

VOID ap_set_rfeature_trig_txbf(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_WARN,
			"Not Support ! arg:%s\n", (UCHAR *)param);
#endif /*DOT11_HE_AX*/

	return;
}

VOID ap_set_rfeature_dis_trig_type(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT8 dis_trig = *((UINT8 *)param);

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%u\n", dis_trig);
#ifdef CFG_SUPPORT_FALCON_MURU
	if ((dis_trig == CAPI_BASIC) || (dis_trig == CAPI_BSRP)) {
		char mu_ru_bsrp[] = "0-0-5-67-0";
		/* Disable continous trigger frame or  BSRP */
		SetMuruBsrpCtrl(adapt, mu_ru_bsrp);
	}
#endif /*CFG_SUPPORT_FALCON_MURU*/
#endif /*DOT11_HE_AX*/
	return;
}

VOID ap_set_rfeature_ofdma_direction(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT8 ofdma_dir = *((UINT8 *)param);

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%u\n", ofdma_dir);
#ifdef CFG_SUPPORT_FALCON_MURU
	if (ofdma_dir == CAPI_OFDMA_DL_20n80)
		SetMuru20MDynAlgo(adapt, "1");
#endif /*CFG_SUPPORT_FALCON_MURU*/
#endif /*DOT11_HE_AX*/
	return;
}

VOID ap_set_rfeature_chnum_band(VOID *ad, VOID *param)
{
	return;
}

VOID ap_set_rfeature_tx_bandwidth(VOID *ad, VOID *param)
{
	return;
}

VOID ap_set_rfeature_ignore_nav(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT8 fgIgnoreNav = *((UINT8 *)param);

#ifdef CFG_SUPPORT_FALCON_MURU
	//char mu_ru_bsrp_dis[] = "0-0-5-67-0";
	char mu_ru_bsrp_enable[] = "1-0-16-67-0";

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
		"arg:%d\n", fgIgnoreNav);

	/* 1. disable BSRP timer before */
	/* iwpriv ra0 set set_muru_bsrp_ctrl=0-0-5-67-5023; */
	//SetMuruBsrpCtrl(adapt, mu_ru_bsrp_dis);

	/* 2. set ignore NAV to transmit pkts */
	set_muru_ignore_nav(adapt, fgIgnoreNav);

	/* 3. set trigger type SW_FID_BASIC_TF */
	/* EXT_CID, EXT_CMD_ID_MURU_CTRL, MURU_SET_TRIG_TYPE */
	set_muru_trig_type(adapt, 0);

	/* 4. enable BSRP timer 16 ms */
	/*iwpriv ra0 set set_muru_bsrp_ctrl=1-0-16-67-5023;*/
	SetMuruBsrpCtrl(adapt, mu_ru_bsrp_enable);
#endif /*CFG_SUPPORT_FALCON_MURU*/
#endif /*DOT11_HE_AX*/
}

VOID ap_set_rfeature_unsolicited_probe_rsp(VOID *ad, VOID *param)
{
#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_6G_SUPPORT)
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT8 en = *((UINT8 *)param);
	UINT32 IdBss;
	BSS_STRUCT *mbss = NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR tx_mode = UNSOLICIT_TXMODE_NON_HT;

	for (IdBss = 0; IdBss < adapt->ApCfg.BssidNum; IdBss++) {
		mbss = &adapt->ApCfg.MBSSID[IdBss];
		wdev = &mbss->wdev;
		if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G) &&
			IS_PHY_CAPS(wlan_config_get_phy_caps(wdev), fPHY_CAP_6G)) {
			tx_mode = wlan_config_get_unsolicit_tx_mode(wdev);
			if (en)
				in_band_discovery_update(wdev, UNSOLICIT_TX_PROBE_RSP, 8, tx_mode, TRUE);
			else
				in_band_discovery_update(wdev, UNSOLICIT_TX_FILS_DISC, 8, tx_mode, TRUE);
		}
	}
#endif /* defined(CONFIG_AP_SUPPORT) && defined(CONFIG_6G_SUPPORT) */
}

VOID ap_set_rfeature_fils_discovery(VOID *ad, VOID *param)
{
#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_6G_SUPPORT)
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT8 en = *((UINT8 *)param);
	UINT32 IdBss;
	BSS_STRUCT *mbss = NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR tx_mode = UNSOLICIT_TXMODE_NON_HT;

	for (IdBss = 0; IdBss < adapt->ApCfg.BssidNum; IdBss++) {
		mbss = &adapt->ApCfg.MBSSID[IdBss];
		wdev = &mbss->wdev;
		if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G) &&
			IS_PHY_CAPS(wlan_config_get_phy_caps(wdev), fPHY_CAP_6G)) {
			tx_mode = wlan_config_get_unsolicit_tx_mode(wdev);
			if (en)
				in_band_discovery_update(wdev, UNSOLICIT_TX_FILS_DISC, 8, tx_mode, TRUE);
			else
				in_band_discovery_update(wdev, UNSOLICIT_TX_PROBE_RSP, 8, tx_mode, TRUE);
		}
	}
#endif /* defined(CONFIG_AP_SUPPORT) && defined(CONFIG_6G_SUPPORT) */
}

VOID ap_set_rfeature_qos_null_injector(VOID *ad, VOID *param)
{
#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_6G_SUPPORT)
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT8 en = *((UINT8 *)param);
	UINT32 IdBss;
	BSS_STRUCT *mbss = NULL;
	struct wifi_dev *wdev = NULL;

	for (IdBss = 0; IdBss < adapt->ApCfg.BssidNum; IdBss++) {
		mbss = &adapt->ApCfg.MBSSID[IdBss];
		wdev = &mbss->wdev;
		if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G) &&
			IS_PHY_CAPS(wlan_config_get_phy_caps(wdev), fPHY_CAP_6G)) {
			qos_injector_update(wdev, 5, en);
		}
	}
#endif /* defined(CONFIG_AP_SUPPORT) && defined(CONFIG_6G_SUPPORT) */
}

VOID ap_set_rfeature_bss_max_idle_ie(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	BSS_STRUCT *pMbss = wdev->func_dev;
	UINT8 value = *((UINT8 *)param);

	if (pMbss) {
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
				"func:%s, arg:%u\n", __func__, value);

		if (value)
			pMbss->max_idle_ie_en = TRUE;
		else
			pMbss->max_idle_ie_en = FALSE;

		UpdateBeaconHandler(ad, wdev, BCN_UPDATE_IE_CHG);
	}
#endif
}

VOID ap_set_rfeature_bss_max_idle_period(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	BSS_STRUCT *pMbss = wdev->func_dev;
	UINT16 value = *((UINT16 *)param);

	if (pMbss) {
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
				"func:%s, arg:%u\n", __func__, value);

		pMbss->max_idle_period = value;
		UpdateBeaconHandler(ad, wdev, BCN_UPDATE_IE_CHG);
	}
#endif
}

VOID ap_set_rfeature_bss_max_idle_option(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	BSS_STRUCT *pMbss = wdev->func_dev;
	UINT8 value = *((UINT8 *)param);

	if (pMbss) {
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
				"func:%s, arg:%u\n", __func__, value);

		pMbss->max_idle_option = value;
		UpdateBeaconHandler(ad, wdev, BCN_UPDATE_IE_CHG);
	}
#endif
}

/* MU EDCA */
VOID ap_set_rfeature_mu_edca_ecw_min_vo(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_ECW_MIN, ACI_AC_VO, value);
#endif
}

VOID ap_set_rfeature_mu_edca_ecw_min_vi(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_ECW_MIN, ACI_AC_VI, value);
#endif
}

VOID ap_set_rfeature_mu_edca_ecw_min_be(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_ECW_MIN, ACI_AC_BE, value);
#endif
}

VOID ap_set_rfeature_mu_edca_ecw_min_bk(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_ECW_MIN, ACI_AC_BK, value);
#endif
}

VOID ap_set_rfeature_mu_edca_ecw_max_vo(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_ECW_MAX, ACI_AC_VO, value);
#endif
}

VOID ap_set_rfeature_mu_edca_ecw_max_vi(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_ECW_MAX, ACI_AC_VI, value);
#endif
}

VOID ap_set_rfeature_mu_edca_ecw_max_be(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_ECW_MAX, ACI_AC_BE, value);
#endif
}

VOID ap_set_rfeature_mu_edca_ecw_max_bk(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_ECW_MAX, ACI_AC_BK, value);
#endif
}

VOID ap_set_rfeature_mu_edca_aifsn_vo(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_AIFSN, ACI_AC_VO, value);
#endif
}

VOID ap_set_rfeature_mu_edca_aifsn_vi(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_AIFSN, ACI_AC_VI, value);
#endif
}

VOID ap_set_rfeature_mu_edca_aifsn_be(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_AIFSN, ACI_AC_BE, value);
#endif
}

VOID ap_set_rfeature_mu_edca_aifsn_bk(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_AIFSN, ACI_AC_BK, value);
#endif
}

VOID ap_set_rfeature_mu_edca_timer_vo(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_TIMER, ACI_AC_VO, value);
#endif
}

VOID ap_set_rfeature_mu_edca_timer_vi(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_TIMER, ACI_AC_VI, value);
#endif
}

VOID ap_set_rfeature_mu_edca_timer_be(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_TIMER, ACI_AC_BE, value);
#endif
}

VOID ap_set_rfeature_mu_edca_timer_bk(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_he_mu_edca(wdev, MU_EDCA_TIMER, ACI_AC_BK, value);
#endif
}

/* WMM PE */
VOID ap_set_rfeature_wmm_pe_ecw_min_vo(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_ECW_MIN, ACI_AC_VO, value);
#endif
}

VOID ap_set_rfeature_wmm_pe_ecw_min_vi(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_ECW_MIN, ACI_AC_VI, value);
#endif
}

VOID ap_set_rfeature_wmm_pe_ecw_min_be(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_ECW_MIN, ACI_AC_BE, value);
#endif
}

VOID ap_set_rfeature_wmm_pe_ecw_min_bk(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_ECW_MIN, ACI_AC_BK, value);
#endif
}

VOID ap_set_rfeature_wmm_pe_ecw_max_vo(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_ECW_MAX, ACI_AC_VO, value);
#endif
}

VOID ap_set_rfeature_wmm_pe_ecw_max_vi(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_ECW_MAX, ACI_AC_VI, value);
#endif
}

VOID ap_set_rfeature_wmm_pe_ecw_max_be(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_ECW_MAX, ACI_AC_BE, value);
#endif
}

VOID ap_set_rfeature_wmm_pe_ecw_max_bk(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_ECW_MAX, ACI_AC_BK, value);
#endif
}

VOID ap_set_rfeature_wmm_pe_aifsn_vo(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_AIFSN, ACI_AC_VO, value);
#endif
}

VOID ap_set_rfeature_wmm_pe_aifsn_vi(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_AIFSN, ACI_AC_VI, value);
#endif
}

VOID ap_set_rfeature_wmm_pe_aifsn_be(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_AIFSN, ACI_AC_BE, value);
#endif
}

VOID ap_set_rfeature_wmm_pe_aifsn_bk(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 value = *((UINT8 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_AIFSN, ACI_AC_BK, value);
#endif
}

VOID ap_set_rfeature_wmm_pe_txop_vo(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT16 value = *((UINT16 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_TXOP, ACI_AC_VO, value);
#endif
}

VOID ap_set_rfeature_wmm_pe_txop_vi(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT16 value = *((UINT16 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_TXOP, ACI_AC_VI, value);
#endif
}

VOID ap_set_rfeature_wmm_pe_txop_be(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT16 value = *((UINT16 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_TXOP, ACI_AC_BE, value);
#endif
}

VOID ap_set_rfeature_wmm_pe_txop_bk(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT16 value = *((UINT16 *)param);

	wlan_config_set_ht_edca(wdev, WMM_PE_TXOP, ACI_AC_BK, value);
#endif
}

#ifdef CONFIG_AP_SUPPORT
static VOID ap_set_rfeature_ap_wmm_pe_txop(
	VOID *ad, struct wifi_dev *wdev, UINT32 aci, UINT16 value)
{
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UCHAR wmm_idx = HcGetWmmIdx(adapt, wdev);
	EDCA_PARM *wmm_edca_param = HcGetEdca(adapt, wdev);
	UCHAR ac_queue[] = {
		TxQ_IDX_AC1, /* ACI:0 AC_BE */
		TxQ_IDX_AC0, /* ACI:1 AC_BK */
		TxQ_IDX_AC2, /* ACI:2 AC_VI */
		TxQ_IDX_AC3, /* ACI:3 AC_VO */
	};
	UCHAR q_idx = ac_queue[aci];

	adapt->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[q_idx] = value;
	if (wmm_edca_param)
		wmm_edca_param->Txop[q_idx] = value;
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"%s: APTxop[%d]=%d\n",
		__func__, q_idx, adapt->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[q_idx]);

	if (AsicSetWmmParam(adapt, wmm_idx, ac_queue[q_idx], WMM_PARAM_TXOP, value) == NDIS_STATUS_FAILURE) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"fails to set APTxop[%d]\n", q_idx);
	}

}
#endif

VOID ap_set_rfeature_ap_wmm_pe_txop_vo(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT16 value = *((UINT16 *)param);

	ap_set_rfeature_ap_wmm_pe_txop(ad, wdev, ACI_AC_VO, value);
#endif
}

VOID ap_set_rfeature_ap_wmm_pe_txop_vi(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT16 value = *((UINT16 *)param);

	ap_set_rfeature_ap_wmm_pe_txop(ad, wdev, ACI_AC_VI, value);
#endif
}

VOID ap_set_rfeature_ap_wmm_pe_txop_be(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT16 value = *((UINT16 *)param);

	ap_set_rfeature_ap_wmm_pe_txop(ad, wdev, ACI_AC_BE, value);
#endif
}

VOID ap_set_rfeature_ap_wmm_pe_txop_bk(VOID *ad, VOID *param)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT16 value = *((UINT16 *)param);

	ap_set_rfeature_ap_wmm_pe_txop(ad, wdev, ACI_AC_BK, value);
#endif
}

VOID ap_set_rfeature_non_support(VOID *ad, VOID *param)
{
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_WARN,
			"Not Support ! arg:%s\n", (UCHAR *)param);
	return;
}


#if defined(APCLI_SUPPORT) || defined(CONFIG_STA_SUPPORT)
struct capi_set_cmd sta_set_wireless_capi[] = {
	{"addbareq_bufsize", sta_set_wireless_addbareq_bufsize},
	{"addbaresp_bufsize", sta_set_wireless_addbaresp_bufsize},
	{"bcc_ldpc", sta_set_wireless_bcc_ldpc},
	{"mcs_fixedrate", sta_set_wireless_fixed_mcs_ie},
	{"rxsp_stream", sta_set_wireless_rxsp_stream},
	{"txsp_stream", sta_set_wireless_txsp_stream},
	{"bandwidth", sta_set_wireless_bandwidth} /* End of sta_set_wireless */
};

struct capi_set_cmd sta_set_rfeature_capi[] = {
	{"txsuppdu", sta_set_rfeature_txsuppdu},
	{"ltf", sta_set_rfeature_ltf},
	{"gi", sta_set_rfeature_gi} /* End of sta_set_rfeature */
};

INT32 set_sta_wireless(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	UINT32 i;
	UCHAR *capi_s = NULL;
	INT32 ret = STATUS_SUCCESS;
	UINT16 param;

	capi_s = strsep(&arg, "-");
	MTWF_PRINT("func:%s, capi: %s, arg: %s\n", __func__, capi_s, arg);
	if ((capi_s != NULL) && (strlen(capi_s) > 0)) {
		for (i = 0; i < (sizeof(sta_set_wireless_capi)/sizeof(struct capi_set_cmd)); i++) {
			if (strcmp((sta_set_wireless_capi + i)->capi, capi_s) == 0) {
				param = (UINT16)os_str_tol(arg, 0, 10);
				MTWF_PRINT("func:%s, [capi match], arg: %d\n", __func__, param);
				(sta_set_wireless_capi + i)->set_func(ad, &param);
				break;
			}
		}
	}

	return ret;
}

INT32 set_sta_rfeatures(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	UINT32 i;
	UCHAR *capi_s = NULL;
	INT32 ret = STATUS_SUCCESS;
	UINT8 param;

	capi_s = strsep(&arg, "-");
	MTWF_PRINT("func:%s, capi: %s, arg: %s\n", __func__, capi_s, arg);
	if ((capi_s != NULL) && (strlen(capi_s) > 0)) {
		for (i = 0; i < (sizeof(sta_set_rfeature_capi)/sizeof(struct capi_set_cmd)); i++) {
			if (strcmp((sta_set_rfeature_capi + i)->capi, capi_s) == 0) {
				param = (UINT8)os_str_tol(arg, 0, 10);
				MTWF_PRINT("func:%s, [capi match], arg: %d\n", __func__, param);
				(sta_set_rfeature_capi + i)->set_func(ad, &param);
				break;
			}
		}
	}

	return ret;
}


/* sta_set_wireless */
VOID sta_set_wireless_addbareq_bufsize(VOID *ad, VOID *param)
{
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT16 addbareq_bufsize = *((UINT16 *)param);

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"set addbareq_bufsize tx/rx win_size = %d\n", addbareq_bufsize);
	wlan_config_set_ba_txrx_wsize(wdev, addbareq_bufsize, addbareq_bufsize);
	return;
}

VOID sta_set_wireless_addbaresp_bufsize(VOID *ad, VOID *param)
{
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT16 addbaresp_bufsize = *((UINT16 *)param);

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
				"set addbaresp_bufsize tx/rx win_size = %d\n", addbaresp_bufsize);

	wlan_config_set_ba_txrx_wsize(wdev, addbaresp_bufsize, addbaresp_bufsize);
	return;

}

VOID sta_set_wireless_bcc_ldpc(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 he_ldpc = *((UINT8 *)param);

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO, "arg:%s\n", (UCHAR *)param);

	wlan_config_set_he_ldpc(wdev, he_ldpc);
#endif /*DOT11_HE_AX*/
	return;
}

VOID sta_set_wireless_fixed_mcs_ie(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 fixed_mcs = *((UINT8 *)param);

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%s\n", (UCHAR *)param);

	wlan_config_set_fixed_mcs(wdev, fixed_mcs);
#endif /*DOT11_HE_AX*/
	return;
}

VOID sta_set_wireless_fixed_mcs_run_time(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	struct _MAC_TABLE_ENTRY *entry = (struct _MAC_TABLE_ENTRY *)param;
	struct wifi_dev *wdev = NULL;
	UINT8 mcs = CAPI_MCS_AUTO;
	UINT32 wcid;

	if (!entry)
		return;
	wcid = entry->wcid;
	wdev = entry->wdev;
	if (!wdev)
		return;

	mcs = wlan_config_get_fixed_mcs(wdev);

	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%u\n", mcs);

	if (mcs < CAPI_MCS_AUTO)
		snd_ra_fw_cmd(RA_PARAM_MCS_UPDATE, adapt, wcid, &mcs);
#endif /*DOT11_HE_AX*/
	return;
}

VOID sta_set_wireless_rxsp_stream(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 rx_nss = *((UINT8 *)param);

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%s\n", (UCHAR *)param);

	wlan_config_set_rx_stream(wdev, rx_nss);
#endif /*DOT11_HE_AX*/
	return;
}

VOID sta_set_wireless_txsp_stream(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 tx_nss = *((UINT8 *)param);

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%s\n", (UCHAR *)param);

	wlan_config_set_tx_stream(wdev, tx_nss);
#endif /*DOT11_HE_AX*/
	return;
}

VOID sta_set_wireless_bandwidth(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 bandwidth = *((UINT8 *)param);

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%s\n", (UCHAR *)param);

	wlan_config_set_he_bw(wdev, bandwidth);
#endif /*DOT11_HE_AX*/
	return;
}

VOID sta_set_wireless_sta_configs(
		struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry)
{
	sta_set_wireless_fixed_mcs_run_time(ad, (VOID *)entry);
}

/* sta_set_rfeature */
VOID sta_set_rfeature_txsuppdu(VOID *ad, VOID *param)
{
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_WARN,
			"to be implemented, arg:%s\n", (UCHAR *)param);
	return;
}

VOID sta_set_rfeature_ltf(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT32 wcid = 0;
	UINT8 he_ltf[CAPI_HE_LTF_NUM] = {
		(LTF_1x << BW20_HE_LTF_SHIFT)|(LTF_1x << BW40_HE_LTF_SHIFT)|(LTF_1x << BW80_HE_LTF_SHIFT)|(LTF_1x << BW160_HE_LTF_SHIFT),
		(LTF_2x << BW20_HE_LTF_SHIFT)|(LTF_2x << BW40_HE_LTF_SHIFT)|(LTF_2x << BW80_HE_LTF_SHIFT)|(LTF_2x << BW160_HE_LTF_SHIFT),
		(LTF_4x << BW20_HE_LTF_SHIFT)|(LTF_4x << BW40_HE_LTF_SHIFT)|(LTF_4x << BW80_HE_LTF_SHIFT)|(LTF_4x << BW160_HE_LTF_SHIFT),
	};
	UINT8 ltf = *((UINT8 *)param);

	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	MAC_TABLE_ENTRY *pEntry = NULL;

	pEntry = GetAssociatedAPByWdev(adapt, wdev);
	if (IS_ENTRY_NONE(pEntry)) {
		MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
				"pEntry not found\n");
		return;
	}
	wcid = pEntry->wcid;

	if (ltf >= CAPI_HE_LTF_NUM)
		return;

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%d ra_setting=0x%08x\n", ltf, he_ltf[ltf]);

	/* EXT_CMD_STAREC_UPDATE,STA_REC_RA_UPDATE,RA_PARAM_HELTF_UPDATE */
	snd_ra_fw_cmd(RA_PARAM_HELTF_UPDATE, adapt, wcid, &he_ltf[ltf]);
#endif /*DOT11_HE_AX*/
	return;
}

VOID sta_set_rfeature_gi(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT32 wcid = 0;
	UINT8 he_gi[CAPI_HE_GI_NUM] = {
		((GI_08_US << BW20_HE_GI_SHIFT)|(GI_08_US << BW40_HE_GI_SHIFT)|(GI_08_US << BW80_HE_GI_SHIFT)|(GI_08_US << BW160_HE_GI_SHIFT)),
		((GI_16_US << BW20_HE_GI_SHIFT)|(GI_16_US << BW40_HE_GI_SHIFT)|(GI_16_US << BW80_HE_GI_SHIFT)|(GI_16_US << BW160_HE_GI_SHIFT)),
		((GI_32_US << BW20_HE_GI_SHIFT)|(GI_32_US << BW40_HE_GI_SHIFT)|(GI_32_US << BW80_HE_GI_SHIFT)|(GI_32_US << BW160_HE_GI_SHIFT)),
	};
	UINT8 gi = *((UINT8 *)param);

	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	MAC_TABLE_ENTRY *pEntry = NULL;

	pEntry = GetAssociatedAPByWdev(adapt, wdev);
	if (IS_ENTRY_NONE(pEntry)) {
		MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_ERROR,
				"pEntry not found\n");
		MTWF_PRINT("func:%s pEntry NULL\n", __func__);
		return;
	}
	wcid = pEntry->wcid;

	if (gi >= CAPI_HE_GI_NUM)
		return;

	MTWF_DBG(adapt, DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_INFO,
			"arg:%d ra_setting=0x%08x\n", gi, he_gi[gi]);

	/* EXT_CMD_STAREC_UPDATE,STA_REC_RA_UPDATE,RA_PARAM_GI_UPDATE */
	snd_ra_fw_cmd(RA_PARAM_GI_UPDATE, adapt, wcid, &he_gi[gi]);
#endif /*DOT11_HE_AX*/
	return;
}
#endif /* defined(APCLI_SUPPORT) || defined(CONFIG_STA_SUPPORT) */
