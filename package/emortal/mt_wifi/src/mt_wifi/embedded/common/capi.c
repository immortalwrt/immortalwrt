#include "rt_config.h"

struct capi_set_cmd ap_set_wireless_capi[] = {
	{"mcs_fixedrate", ap_set_wireless_fixed_mcs},
	{"ofdma", ap_set_wireless_ofdma_direction},
	{"ppdu_tx_type", ap_set_wireless_ppdu_tx_type},
	{"mumimo", ap_set_wireless_mumimo},
	{"num_users_ofdma", ap_set_wireless_nusers_ofdma},
	{"non_tx_bss_idx", ap_set_wireless_non_txbss_idx},
	{"mu_edca_override", ap_set_wireless_mu_edca_override},
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
	{"non_support", ap_set_rfeature_non_support} /* End of ap_set_rfeature */
};

struct capi_set_cmd dev_send_frame_capi[] = {
	{"smpdu", dev_send_frame_smpdu},
	{"non_support", dev_send_frame_non_support} /* End of dev_send_frame */
};

INT32 dev_send_frame(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	UINT32 i;
	UCHAR *capi_s = NULL;
	INT32 ret = STATUS_SUCCESS;

	capi_s = strsep(&arg, "-");
	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, capi: %s, arg: %s\n", __func__, capi_s, arg));
	if ((capi_s != NULL) && (strlen(capi_s) > 0)) {
		for (i = 0; i < (sizeof(dev_send_frame_capi)/sizeof(struct capi_set_cmd)); i++) {
			if (strcmp((dev_send_frame_capi + i)->capi, capi_s) == 0) {
				MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
						("func:%s, [capi match], arg: %s\n", __func__, arg));
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
	UINT8 param;

	capi_s = strsep(&arg, "-");
	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, capi: %s, arg: %s\n", __func__, capi_s, arg));
	if ((capi_s != NULL) && (strlen(capi_s) > 0)) {
		for (i = 0; i < (sizeof(ap_set_rfeature_capi)/sizeof(struct capi_set_cmd)); i++) {
			if (strcmp((ap_set_rfeature_capi + i)->capi, capi_s) == 0) {
				param = (UINT8)os_str_tol(arg, 0, 10);
				MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
						("func:%s, [capi match], arg: %d\n", __func__, param));
				(ap_set_rfeature_capi + i)->set_func(ad, &param);
				break;
			}
		}
	}

	return ret;
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
				UCHAR mac_addr[MAC_ADDR_LEN];
				CHAR *token;
				MAC_TABLE_ENTRY *pEntry = NULL;
				INT i = 0;

				token = rstrtok(thisChar, ":");

				while (token != NULL) {
					AtoH(token, (char *) &mac_addr[i], 1);
					i++;
					if (i >= MAC_ADDR_LEN)
						break;
					token = rstrtok(NULL, ":");
				}

				pEntry = MacTableLookup(pAd, mac_addr);

				if (pEntry != NULL)
					wcid = pEntry->wcid;
			}

			if (!IS_WCID_VALID(pAd, wcid)) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("%s: unknow sta of wcid(%d)\n", __func__, wcid));
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

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("%s::wcid = %d txop %d interval %d\n", __func__, wcid, txopDur, interval));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("\n"));
#endif /* DOT11_HE_AX */
}

VOID dev_send_frame_non_support(VOID *ad, VOID *arg)
{
	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s\n", __func__));
	return;
}

/* ap_set_wireless */
VOID ap_set_wireless_fixed_mcs(VOID *ad, VOID *param)
{
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	struct _MAC_TABLE_ENTRY *entry = (struct _MAC_TABLE_ENTRY *)param;
	struct wifi_dev *wdev = NULL;
	UINT8 mcs = CAPI_MCS_AUTO;
	UINT32 wcid = 1;

	if (!entry)
		return;
	wcid = entry->wcid;
	wdev = entry->wdev;
	if (!wdev)
		return;

	mcs = wlan_config_get_fixed_mcs(wdev);

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_TRACE,
			("func:%s, arg:%u\n", __func__, mcs));

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

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_TRACE,
			("func:%s, arg:%u\n", __func__, ofdma_dir));
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
	UINT8 ppdu_type = CAPI_LEGACY;

	if (!wdev)
		return;
	ppdu_type = wlan_config_get_ppdu_tx_type(wdev);
#ifdef CFG_SUPPORT_FALCON_MURU
	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_WARN,
			("func:%s, val:%d\n", __func__, ppdu_type));
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

	UINT8 mu_dl_mimo = 0;
	UINT8 ppdu_type = CAPI_LEGACY;

	if (!wdev)
		return;
	mu_dl_mimo = wlan_config_get_mu_dl_mimo(wdev);
	ppdu_type = wlan_config_get_ppdu_tx_type(wdev);
#ifdef CFG_SUPPORT_FALCON_MURU
	if (mu_dl_mimo && (ppdu_type == CAPI_MU))
		snprintf(str, sizeof(str), "%s", "dl_init");
	else
		return;

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, cmd:%s\n", __func__, str));
	set_muru_manual_config(adapt, str);
	set_muru_manual_config(adapt, "update");

	/* Set default value as SUBAR. */
	set_muru_mudl_ack_policy(adapt, MURU_CMD_MUDL_ACK_POLICY_SU_BAR);

	if ((wlan_config_get_ht_bw(wdev) == BW_40)
#ifdef DOT11_VHT_AC
			&& (wlan_config_get_vht_bw(wdev) == VHT_BW_80)
#endif /* DOT11_VHT_AC */
		)
		snprintf(mimo_param, sizeof(mimo_param), "%s", "2-134-0-1-0-1-2-2-2");/* <2Usr>-<RU67>-<3.2GI>-<HEFB>-<DL>-<WCID>-<MCS2> */

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

	if (!wdev)
		return;
	nuser = wlan_config_get_ofdma_user_cnt(wdev);
#ifdef CFG_SUPPORT_FALCON_MURU
	if (nuser < 2)
		return;
	SetMuruSuTx(adapt, "0");
	snprintf(str, sizeof(str), "%s:%d", "dl_comm_user_cnt", nuser);
	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, cmd:%s\n", __func__, str));
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

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("func:%s\n", __func__));

	if (!wdev)
		return;
	capi_override = wlan_config_get_mu_edca_override(wdev);
#ifdef CFG_SUPPORT_FALCON_MURU
	set_muru_cert_muedca_override(adapt, capi_override);
#endif /*CFG_SUPPORT_FALCON_MURU*/
#endif /*DOT11_HE_AX*/
	return;
}

VOID ap_set_wireless_non_txbss_idx(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
#endif /*DOT11_HE_AX*/
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
#endif /*DOT11_HE_AX*/
}


/* ap_set_rfeature */
VOID ap_set_rfeature_ack_type(VOID *ad, VOID *param)
{
	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%d\n", __func__, *((UINT8 *)param)));
	return;
}

VOID ap_set_rfeature_ack_policy(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT8 ack_policy = *((UINT8 *)param);

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%d\n", __func__, ack_policy));
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

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%d\n", __func__, ppdu_type));
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

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%d\n", __func__, trig_type));
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

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%d ra_setting=0x%08x\n", __func__, ltf, he_ltf[ltf]));

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

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%d\n", __func__, gi));

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

VOID ap_set_rfeature_trig_txbf(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%s\n", __func__, (UCHAR *)param));
#endif /*DOT11_HE_AX*/

	return;
}

VOID ap_set_rfeature_dis_trig_type(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT8 dis_trig = *((UINT8 *)param);

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%u\n", __func__, dis_trig));
#ifdef CFG_SUPPORT_FALCON_MURU
	if ((dis_trig == CAPI_BASIC) || (dis_trig == CAPI_BSRP)) {
		char mu_ru_bsrp[] = "0-0-5-67-5023";
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

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%u\n", __func__, ofdma_dir));
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
	char mu_ru_bsrp_dis[] = "0-0-5-67-5023";
	char mu_ru_bsrp_enable[] = "1-0-16-67-5023";

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
		("func:%s, arg:%d\n", __func__, fgIgnoreNav));

	/* 1. disable BSRP timer before */
	/* iwpriv ra0 set set_muru_bsrp_ctrl=0-0-5-67-5023; */
	SetMuruBsrpCtrl(adapt, mu_ru_bsrp_dis);

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

VOID ap_set_rfeature_non_support(VOID *ad, VOID *param)
{
	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%s\n", __func__, (UCHAR *)param));
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
	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, capi: %s, arg: %s\n", __func__, capi_s, arg));
	if ((capi_s != NULL) && (strlen(capi_s) > 0)) {
		for (i = 0; i < (sizeof(sta_set_wireless_capi)/sizeof(struct capi_set_cmd)); i++) {
			if (strcmp((sta_set_wireless_capi + i)->capi, capi_s) == 0) {
				param = (UINT16)os_str_tol(arg, 0, 10);
				MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
						("func:%s, [capi match], arg: %d\n", __func__, param));
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
	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, capi: %s, arg: %s\n", __func__, capi_s, arg));
	if ((capi_s != NULL) && (strlen(capi_s) > 0)) {
		for (i = 0; i < (sizeof(sta_set_rfeature_capi)/sizeof(struct capi_set_cmd)); i++) {
			if (strcmp((sta_set_rfeature_capi + i)->capi, capi_s) == 0) {
				param = (UINT8)os_str_tol(arg, 0, 10);
				MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
						("func:%s, [capi match], arg: %d\n", __func__, param));
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

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, set addbareq_bufsize tx/rx win_size = %d\n", __func__, addbareq_bufsize));
	wlan_config_set_ba_txrx_wsize(wdev, addbareq_bufsize, addbareq_bufsize);
	return;
}

VOID sta_set_wireless_addbaresp_bufsize(VOID *ad, VOID *param)
{
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	POS_COOKIE pObj = (POS_COOKIE) adapt->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(adapt, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT16 addbaresp_bufsize = *((UINT16 *)param);

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
				("func:%s,set addbaresp_bufsize tx/rx win_size = %d\n", __func__, addbaresp_bufsize));
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

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%s\n", __func__, (UCHAR *)param));

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

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%s\n", __func__, (UCHAR *)param));

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
	UINT32 wcid = CAPI_APCLI_STA;

	if (!entry)
		return;
	wcid = entry->wcid;
	wdev = entry->wdev;
	if (!wdev)
		return;

	mcs = wlan_config_get_fixed_mcs(wdev);

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_TRACE,
			("func:%s, arg:%u\n", __func__, mcs));

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

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%s\n", __func__, (UCHAR *)param));

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

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%s\n", __func__, (UCHAR *)param));

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

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%s\n", __func__, (UCHAR *)param));

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
	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, to be implemented, arg:%s\n", __func__, (UCHAR *)param));
	return;
}

VOID sta_set_rfeature_ltf(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT32 wcid = CAPI_APCLI_STA;
	UINT8 he_ltf[CAPI_HE_LTF_NUM] = {
		(LTF_1x << BW20_HE_LTF_SHIFT)|(LTF_1x << BW40_HE_LTF_SHIFT)|(LTF_1x << BW80_HE_LTF_SHIFT)|(LTF_1x << BW160_HE_LTF_SHIFT),
		(LTF_2x << BW20_HE_LTF_SHIFT)|(LTF_2x << BW40_HE_LTF_SHIFT)|(LTF_2x << BW80_HE_LTF_SHIFT)|(LTF_2x << BW160_HE_LTF_SHIFT),
		(LTF_4x << BW20_HE_LTF_SHIFT)|(LTF_4x << BW40_HE_LTF_SHIFT)|(LTF_4x << BW80_HE_LTF_SHIFT)|(LTF_4x << BW160_HE_LTF_SHIFT),
	};
	UINT8 ltf = *((UINT8 *)param);

	if (ltf >= CAPI_HE_LTF_NUM)
		return;

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%d ra_setting=0x%08x\n", __func__, ltf, he_ltf[ltf]));

	/* EXT_CMD_STAREC_UPDATE,STA_REC_RA_UPDATE,RA_PARAM_HELTF_UPDATE */
	snd_ra_fw_cmd(RA_PARAM_HELTF_UPDATE, adapt, wcid, &he_ltf[ltf]);
#endif /*DOT11_HE_AX*/
	return;
}

VOID sta_set_rfeature_gi(VOID *ad, VOID *param)
{
#ifdef DOT11_HE_AX
	struct _RTMP_ADAPTER *adapt = (struct _RTMP_ADAPTER *)ad;
	UINT32 wcid = CAPI_APCLI_STA;
	UINT8 he_gi[CAPI_HE_GI_NUM] = {
		((GI_08_US << BW20_HE_GI_SHIFT)|(GI_08_US << BW40_HE_GI_SHIFT)|(GI_08_US << BW80_HE_GI_SHIFT)|(GI_08_US << BW160_HE_GI_SHIFT)),
		((GI_16_US << BW20_HE_GI_SHIFT)|(GI_16_US << BW40_HE_GI_SHIFT)|(GI_16_US << BW80_HE_GI_SHIFT)|(GI_16_US << BW160_HE_GI_SHIFT)),
		((GI_32_US << BW20_HE_GI_SHIFT)|(GI_32_US << BW40_HE_GI_SHIFT)|(GI_32_US << BW80_HE_GI_SHIFT)|(GI_32_US << BW160_HE_GI_SHIFT)),
	};
	UINT8 gi = *((UINT8 *)param);

	MTWF_LOG(DBG_CAT_TEST, CATTEST_RFEATURE, DBG_LVL_OFF,
			("func:%s, arg:%d\n", __func__, gi));

	/* EXT_CMD_STAREC_UPDATE,STA_REC_RA_UPDATE,RA_PARAM_GI_UPDATE */
	snd_ra_fw_cmd(RA_PARAM_GI_UPDATE, adapt, wcid, &he_gi[gi]);
#endif /*DOT11_HE_AX*/
	return;
}
#endif /* defined(APCLI_SUPPORT) || defined(CONFIG_STA_SUPPORT) */
