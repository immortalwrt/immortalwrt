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
#include "framework_dvt.h"
#include "mcu/andes_core.h"
#include "mcu/mt_cmd.h"

#define DVT_MODNAME "FRAMEWORK"

static VOID dvt_notify_init(struct _RTMP_ADAPTER *ad);

/*module init function section*/
VOID wrap_dvt_init(struct dvt_framework *dvt_ctrl);
VOID apps_dvt_init(struct dvt_framework *dvt_ctrl);
VOID lp_dvt_init(struct dvt_framework *dvt_ctrl);
VOID txcmdsu_dvt_init(struct dvt_framework *dvt_ctrl);
VOID mucop_dvt_init(struct dvt_framework *dvt_ctrl);

/*dvt framework interal use*/
/*
*
*/
static INT dvt_framework_init(struct _RTMP_ADAPTER *ad)
{
	struct dvt_framework *dvt_ctrl;

	os_alloc_mem(ad, (UCHAR **)&ad->dvt_ctrl, sizeof(struct dvt_framework));

	if (!ad->dvt_ctrl) {
		printk("%s(): allocate fail!\n", __func__);
		return DVT_STATUS_RESOUCE_ERROR;
	}

	dvt_ctrl = ad->dvt_ctrl;
	os_zero_mem(dvt_ctrl, sizeof(struct dvt_framework));
	/*initial framework related part*/
	dvt_ctrl->ad = ad;
	DlListInit(&dvt_ctrl->dvt_head);
	dvt_notify_init(dvt_ctrl->ad);
	/*start init dvt features*/
	wrap_dvt_init(dvt_ctrl);
	apps_dvt_init(dvt_ctrl);
	lp_dvt_init(dvt_ctrl);
	txcmdsu_dvt_init(dvt_ctrl);
	mucop_dvt_init(dvt_ctrl);

	return 0;
}

/*
*
*/
VOID dvt_framework_exit(struct _RTMP_ADAPTER *ad, struct dvt_framework *dvt_ctrl)
{
	DlListInit(&dvt_ctrl->dvt_head);
	/*start to exit feature dvt*/
	os_free_mem(dvt_ctrl);
	ad->dvt_ctrl = NULL;
}

/*
*
*/
INT dvt_feature_search(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	struct dvt_framework *dvt_ctrl;
	struct dvt_feature_entry *entry;
	char *cur = arg;
	char *feature;
	char *test_case;
	INT test_id;
	struct os_cookie *obj = (struct os_cookie *) ad->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(ad, obj->ioctl_if, obj->ioctl_if_type);

	if (!ad->dvt_ctrl)
		dvt_framework_init(ad);

	dvt_ctrl = ad->dvt_ctrl;

	if (!dvt_ctrl)
		return 0;

	feature = strsep((char **)&cur, "-");
	test_case = strsep((char **)&cur, "-");
	test_id = simple_strtol(test_case, NULL, 10);
	DVT_LOG("feature,%s&test_id,%d", feature, test_id);
	DlListForEach(entry, &dvt_ctrl->dvt_head, struct dvt_feature_entry, list) {
		if (!strncmp(feature, entry->feature_name, strlen(feature))) {
			if (test_id > entry->dvt_cnt)
				break;
			DVT_LOG("search,ok&dvt_cnt,%d", entry->dvt_cnt);
			return entry->dvt_table[test_id - 1](ad, wdev, cur);
		} else {
			DVT_LOG("search,fail!");
		}
	}
	return 0;
}

/*dvt module usage only*/
/*
*
*/
INT dvt_feature_register(struct dvt_framework *dvt_ctrl, struct dvt_feature_entry *entry)
{
	DlListAdd(&dvt_ctrl->dvt_head, &entry->list);
	return 0;
}

/*
*
*/
VOID dvt_feature_unregister(struct dvt_feature_entry *entry)
{
	DlListDel(&entry->list);
}

/*
*
*/
static VOID dvt_ut_wmcu_resp(struct cmd_msg *msg, char *data, UINT16 len)
{
	CMD_SYSDVT_CTRL_EXT_T *dvt_res = (CMD_SYSDVT_CTRL_EXT_T *) data;
	UINT16 head_len = sizeof(CMD_SYSDVT_CTRL_EXT_T);

	DVT_LOG("feature,%d&type,%d&len,%d", dvt_res->u4FeatureIdx, dvt_res->u4Type, len);

	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, data + head_len, len - head_len);
}

/*
*
*/
INT32 dvt_ut_wmcu_send(struct _RTMP_ADAPTER *ad, struct dvt_wmcu_request *request)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	UCHAR *data;
	CMD_SYSDVT_CTRL_EXT_T *dvt_req;
	UINT16 cmd_len = sizeof(CMD_SYSDVT_CTRL_EXT_T) + request->len;
	UINT16 res_len = sizeof(CMD_SYSDVT_CTRL_EXT_T) + request->resp_len;
	struct _CMD_ATTRIBUTE attr = {0};

	DVT_LOG("id,%d&type,%d", request->feature_id, request->type);
	DVT_LOG("len,%d&payload,%p", request->len, request->payload);
	DVT_LOG("resp_len,%d&resp_payload,%p", request->resp_len, request->resp);
	os_alloc_mem(ad, &data, cmd_len);

	if (!data) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	dvt_req = (CMD_SYSDVT_CTRL_EXT_T *)data;
	msg = AndesAllocCmdMsg(ad, cmd_len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SYSDVT_TEST);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 10000);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, res_len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, request->resp);
	if (request->resp_handle)
		SET_CMD_ATTR_RSP_HANDLER(attr, request->resp_handle);
	else
		SET_CMD_ATTR_RSP_HANDLER(attr, dvt_ut_wmcu_resp);
	AndesInitCmdMsg(msg, attr);
	dvt_req->u4FeatureIdx = cpu2le32(request->feature_id);
	dvt_req->u4Type = cpu2le32(request->type);
	dvt_req->u4Lth = cpu2le32(request->len);
	dvt_req->u2CmdLen = cpu2le16(cmd_len);
	dvt_req->ucCmdVer = 0;
	os_move_mem(dvt_req->u1cBuffer, request->payload, request->len);

	AndesAppendCmdMsg(msg, (char *)dvt_req, cmd_len);
	ret = chip_cmd_tx(ad, msg);
error:
	DVT_LOG("ret,%d", ret);
	if (ret != NDIS_STATUS_SUCCESS)
		return DVT_STATUS_REQ_MCPU_FAIL;
	return DVT_STATUS_OK;
}

/*
*
*/
INT32 dvt_txcmd_wmcu_send(struct _RTMP_ADAPTER *ad, struct dvt_wmcu_request *request)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	UCHAR *data;
	CMD_SYSDVT_CTRL_EXT_T *dvt_req;
	UINT16 cmd_len = sizeof(CMD_SYSDVT_CTRL_EXT_T) + max(request->resp_len, request->len);
	struct _CMD_ATTRIBUTE attr = {0};

	DVT_LOG("id,%d&type,%d", request->feature_id, request->type);
	DVT_LOG("len,%d&payload,%p", request->len, request->payload);
	DVT_LOG("resp_len,%d&resp_payload,%p", request->resp_len, request->resp);
	os_alloc_mem(ad, &data, cmd_len);

	if (!data) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	dvt_req = (CMD_SYSDVT_CTRL_EXT_T *)data;
	msg = AndesAllocCmdMsg(ad, cmd_len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_DBG_TXCMD);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 10000);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, request->resp_len + sizeof(CMD_SYSDVT_CTRL_EXT_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, request->resp);
	if (request->resp_handle)
		SET_CMD_ATTR_RSP_HANDLER(attr, request->resp_handle);
	else
		SET_CMD_ATTR_RSP_HANDLER(attr, dvt_ut_wmcu_resp);
	AndesInitCmdMsg(msg, attr);
	dvt_req->u4FeatureIdx = cpu2le32(request->feature_id);
	dvt_req->u4Type = cpu2le32(request->type);
	dvt_req->u4Lth = cpu2le32(request->len);
	dvt_req->u2CmdLen = cpu2le16(cmd_len);
	dvt_req->ucCmdVer = 0;
	os_move_mem(dvt_req->u1cBuffer, request->payload, request->len);

	AndesAppendCmdMsg(msg, (char *)dvt_req, cmd_len);
	ret = chip_cmd_tx(ad, msg);
error:
	DVT_LOG("ret,%d", ret);
	if (ret != NDIS_STATUS_SUCCESS)
		return DVT_STATUS_REQ_MCPU_FAIL;
	return DVT_STATUS_OK;
}


/*
*
*/
VOID dvt_ut_seudo_sta_security_set(struct _RTMP_ADAPTER *ad, struct dvt_seudo_sta *sta)
{
	struct wifi_dev *wdev;
	struct _MAC_TABLE_ENTRY *entry = sta->mac_entry;

	if (!entry)
		return;

	if (!VALID_UCAST_ENTRY_WCID(ad, entry->wcid))
		return;

	wdev = entry->wdev;

#ifdef CONFIG_AP_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_AP || wdev->wdev_type == WDEV_TYPE_WDS) {
		struct _SECURITY_CONFIG *sec_cfg = NULL;
		ASIC_SEC_INFO info;

		sec_cfg = &wdev->SecConfig;

		if (sec_cfg->AKMMap == 0x0)
			SET_AKM_OPEN(sec_cfg->AKMMap);

		if (sec_cfg->PairwiseCipher == 0x0)
			SET_CIPHER_NONE(sec_cfg->PairwiseCipher);

		/* Set key material to Asic */
		os_zero_mem(&info, sizeof(ASIC_SEC_INFO));
		info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
		info.Direction = SEC_ASIC_KEY_BOTH;
		info.Wcid = entry->wcid;
		info.BssIndex = wdev->bss_info_argument.ucBssIndex;
		info.Cipher = sec_cfg->PairwiseCipher;
		info.KeyIdx = sec_cfg->PairwiseKeyId;
		os_move_mem(&info.PeerAddr[0], entry->Addr, MAC_ADDR_LEN);

		/* When WEP, TKIP or AES is enabled, set group key info to Asic */
		if (IS_CIPHER_WEP(sec_cfg->PairwiseCipher)) {
			os_move_mem(&info.Key, &sec_cfg->WepKey[info.KeyIdx], sizeof(SEC_KEY_INFO));
			HW_ADDREMOVE_KEYTABLE(ad, &info);
		} else if (IS_CIPHER_TKIP(sec_cfg->PairwiseCipher)
				   || IS_CIPHER_CCMP128(sec_cfg->PairwiseCipher)
				   || IS_CIPHER_CCMP256(sec_cfg->PairwiseCipher)
				   || IS_CIPHER_GCMP128(sec_cfg->PairwiseCipher)
				   || IS_CIPHER_GCMP256(sec_cfg->PairwiseCipher)) {
			/* Calculate Key */
			SetWPAPSKKey(ad, sec_cfg->PSK, strlen(sec_cfg->PSK), (PUCHAR) RALINK_PASSPHRASE, sizeof(RALINK_PASSPHRASE), sec_cfg->PMK);
			os_move_mem(info.Key.Key, sec_cfg->PMK, LEN_PMK);

			if (IS_CIPHER_TKIP(sec_cfg->PairwiseCipher)) {
				/*WDS: RxMic/TxMic use the same value */
				os_move_mem(&info.Key.Key[LEN_TK + LEN_TKIP_MIC], &info.Key.Key[LEN_TK], LEN_TKIP_MIC);
			}

			WPAInstallKey(ad, &info, TRUE, TRUE);
		}
	}
#endif /*CONFIG_AP_SUPPORT*/
}

#ifdef DOT11_HE_AX
/*
*
*/
static INT dvt_ut_seudo_sta_he_template(struct _RTMP_ADAPTER *ad, struct dvt_seudo_sta *sta)
{
	struct _MAC_TABLE_ENTRY *peer = sta->mac_entry;
	INT i = 0;

	/*init he_phy_cap/he_gi/he_stbc*/
	peer->cap.he_phy_cap = 0;
	peer->cap.he_gi = 0;
	peer->cap.stbc.he_stbc = 0;
	peer->cap.he_bf.bf_cap = 0;
	peer->cap.he_phy_cap |= HE_DUAL_BAND;
	peer->cap.modes |= (HE_24G_SUPPORT | HE_5G_SUPPORT);
	peer->cap.ch_bw.he_ch_width = SUPP_40M_CW_IN_24G_BAND | SUPP_40M_80M_CW_IN_5G_BAND;
	peer->cap.he_mac_cap |= HE_AMSDU_IN_ACK_EN_AMPDU;

	/*phy cap*/
	peer->cap.ch_bw.he_bw20_242tone = 1;
	peer->cap.he_phy_cap |= HE_DEV_CLASS_A;
	peer->cap.he_phy_cap |= HE_LDPC;
	peer->cap.he_phy_cap |= HE_DCM_MAX_NSS_TX;
	peer->cap.he_phy_cap |= HE_DCM_MAX_NSS_RX;
	peer->cap.punc_preamble_rx = 0;
	/*
	peer->cap.ampdu.max_he_ampdu_len_exp = ;
	peer->cap.dcm_max_constellation_tx = ;
	*/
	peer->cap.he_mac_cap |= HE_HTC;
	peer->cap.he_mac_cap |= HE_BQR;
	peer->cap.he_mac_cap |= HE_BSR;
	peer->cap.he_mac_cap |= HE_OM_CTRL;
	/*peer->cap.tf_mac_pad_duration*/
	/*peer->cap.dcm_max_constellation_rx*/
	peer->cap.he_phy_cap |= HE_TRIG_CQI_FEEDBACK;
	peer->cap.he_phy_cap |= HE_PARTIAL_BW_ER;
	/*max nss mcs*/
	for (i = 0; i < DOT11AX_MAX_STREAM; i++) {
		peer->cap.rate.he80_rx_nss_mcs[i] = 2;
		/*
		peer->cap.rate.he160_rx_nss_mcs;
		peer->cap.rate.he8080_rx_nss_mcs;
		*/
	}
	return DVT_STATUS_OK;
}

INT dvt_enable_sta_he_test(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	UINT Enable;

	Enable = simple_strtol(arg, 0, 10);
	if (!Enable)
		ad->bStaHeTest = FALSE;
	else
		ad->bStaHeTest = TRUE;
	DVT_LOG("ad->bStaHeTest=%d", ad->bStaHeTest);
	return TRUE;
}
#endif /*DOT11_HE_AX*/

/*
*
*/
INT dvt_ut_seudo_sta_connect(struct _RTMP_ADAPTER *ad, struct dvt_seudo_sta *sta)
{
	struct _MAC_TABLE_ENTRY *entry;
	struct _STA_TR_ENTRY *tr_entry;
	struct tx_rx_ctl *tr_ctl = &ad->tr_ctl;
	struct wifi_dev *wdev = sta->wdev;
	HTTRANSMIT_SETTING ht_phy_mode;
	USHORT phy_mode = sta->phy_mode;
	HT_CAPABILITY_IE *ht_cap;
	BOOLEAN has_ht_cap = FALSE;
	BOOLEAN has_vht_cap = FALSE;
	struct legacy_rate *rate = &wdev->rate.legacy_rate;
	INT result = DVT_STATUS_OK;

	/* allocate one MAC entry */
	entry = MacTableInsertEntry(ad, sta->addr, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

	if (entry) {
		tr_entry = &tr_ctl->tr_entry[entry->wcid];
		tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
		tr_entry->OmacIdx = wdev->OmacIdx;
		sta->mac_entry = entry;
		DVT_LOG("wcid,%d&omac,%d&phymode,%d", entry->wcid, wdev->OmacIdx, phy_mode);

		/* specific Max Tx Rate for STA link. */
		os_zero_mem(&ht_phy_mode, sizeof(HTTRANSMIT_SETTING));

		if (WMODE_EQUAL(phy_mode, WMODE_B)) {
			ht_phy_mode.field.MODE = MODE_CCK;
			ht_phy_mode.field.MCS = 3;
			entry->RateLen = 4;
			DVT_LOG("MODE_CCK");
		} else if (WMODE_EQUAL(phy_mode, WMODE_GN | WMODE_AN)) {
			ht_phy_mode.field.MODE = MODE_HTGREENFIELD;
			ht_phy_mode.field.MCS = 7;
			ht_phy_mode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
			ht_phy_mode.field.BW = wdev->HTPhyMode.field.BW;
			ht_phy_mode.field.STBC = wdev->HTPhyMode.field.STBC;
			entry->RateLen = 12;
			DVT_LOG("MODE_HTGREENFIELD");
#ifdef DOT11_HE_AX
		} else if (WMODE_CAP_AX(phy_mode)) {
			/* to check */
			ht_phy_mode.field.MODE = MODE_HE;
			ht_phy_mode.field.MCS = 11;
			ht_phy_mode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
			ht_phy_mode.field.BW = wdev->HTPhyMode.field.BW;
			ht_phy_mode.field.STBC = wdev->HTPhyMode.field.STBC;
			entry->RateLen = 12;
			dvt_ut_seudo_sta_he_template(ad, sta);
			DVT_LOG("MODE_HE");
#endif /*DOT11_HE_AX*/
		} else if (WMODE_CAP_AC(sta->phy_mode)) {
			ht_phy_mode.field.MODE = MODE_VHT;
			ht_phy_mode.field.MCS = 9;
			ht_phy_mode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
			ht_phy_mode.field.BW = wdev->HTPhyMode.field.BW;
			ht_phy_mode.field.STBC = wdev->HTPhyMode.field.STBC;
			entry->RateLen = 12;
			DVT_LOG("MODE_VHT");
		} else if (WMODE_CAP_N(phy_mode)) {
			ht_phy_mode.field.MODE = MODE_HTMIX;
			ht_phy_mode.field.MCS = 7;
			ht_phy_mode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
			ht_phy_mode.field.BW = wdev->HTPhyMode.field.BW;
			ht_phy_mode.field.STBC = wdev->HTPhyMode.field.STBC;
			entry->RateLen = 12;
			DVT_LOG("MODE_HTMIX");
		} else {
			ht_phy_mode.field.MODE = MODE_OFDM;
			ht_phy_mode.field.MCS = 7;
			entry->RateLen = 8;
			DVT_LOG("MODE_OFDM");
		}

		entry->MaxHTPhyMode.word = ht_phy_mode.word;
		entry->MinHTPhyMode.word = wdev->MinHTPhyMode.word;
		entry->HTPhyMode.word = entry->MaxHTPhyMode.word;

#ifdef DOT11_N_SUPPORT
		if (WMODE_CAP_N(phy_mode)) {
			if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO) {
				DVT_LOG("Desired MCS,%d", wdev->DesiredTransmitSetting.field.MCS);
				set_ht_fixed_mcs(entry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);
			}

			entry->MmpsMode = MMPS_DISABLE;
			ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
			NdisMoveMemory(&entry->HTCapability, ht_cap, sizeof(HT_CAPABILITY_IE));
			has_ht_cap = TRUE;

			ht_mode_adjust(ad, entry, &entry->HTCapability);
			set_sta_ht_cap(ad, entry, &entry->HTCapability);

			if (sta->cap_flag & DVT_STA_WMM_CAP)
				CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_WMM_CAPABLE);

			DVT_LOG("MaxRAmpduFactor,%d&MpduDensity,%d",
				entry->HTCapability.HtCapParm.MaxRAmpduFactor,
				entry->HTCapability.HtCapParm.MpduDensity);
		} else {
			os_zero_mem(&entry->HTCapability, sizeof(HT_CAPABILITY_IE));
		}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
		if (WMODE_CAP_AC(phy_mode) && WMODE_CAP_5G(wdev->PhyMode)) {
			VHT_CAP_IE vht_cap;

			vht_mode_adjust(ad, entry, &vht_cap, NULL, NULL);
			dot11_vht_mcs_to_internal_mcs(ad, wdev, &vht_cap, &entry->MaxHTPhyMode);
			set_vht_cap(ad, entry, &vht_cap);
			DVT_LOG("PhyCap&Mode,%s&BW,%s&MC,0x%x&Word,0x%x",
					 get_phymode_str(entry->MaxHTPhyMode.field.MODE),
					 get_bw_str(entry->MaxHTPhyMode.field.BW),
					 entry->MaxHTPhyMode.field.MCS,
					 entry->MaxHTPhyMode.word);
			os_move_mem(&entry->vht_cap_ie, &vht_cap, sizeof(VHT_CAP_IE));
			has_vht_cap = TRUE;
			assoc_vht_info_debugshow(ad, entry, &vht_cap, NULL);
		} else {
			os_zero_mem(&entry->vht_cap_ie, sizeof(VHT_CAP_IE));
		}
#endif /* DOT11_VHT_AC */

		RTMPSetSupportMCS(ad,
						  OPMODE_AP,
						  entry,
						  rate,
#ifdef DOT11_VHT_AC
						  has_vht_cap,
						  &entry->vht_cap_ie,
#endif
						  &entry->HTCapability,
						  has_ht_cap);

		/* for now, we set this by default! */
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_RALINK_CHIPSET);

		if (wdev->bAutoTxRateSwitch == FALSE) {
			entry->HTPhyMode.field.MCS = wdev->DesiredTransmitSetting.field.MCS;
			entry->bAutoTxRateSwitch = FALSE;
			/* If the legacy mode is set, overwrite the transmit setting of this entry. */
			RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, entry);
		} else {
			entry->bAutoTxRateSwitch = TRUE;
		}

		entry->func_tb_idx = wdev->func_idx;
		entry->wdev = wdev;
		entry->sta_force_keep = TRUE;
		entry->Sst = SST_ASSOC;
		/* update per wdev bw */
		wlan_operate_set_ht_bw(wdev, wdev->MaxHTPhyMode.field.BW, wlan_operate_get_ext_cha(wdev));

		if (wdev_do_conn_act(wdev, entry) != TRUE) {
			DVT_LOG("connect fail\n");
			result = DVT_STATUS_CONN_FAIL;
		}

		dvt_ut_seudo_sta_security_set(ad, sta);
		MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CONNECTION_NOT_READY);

		DVT_LOG("connect ok&wcid,%d", entry->wcid);
	} else {
		DVT_LOG("can't find available mac entry!");
		result = DVT_STATUS_CONN_FAIL;
	}
	return result;
}

/*
*
*/
VOID dvt_ut_seudo_sta_disconnect(struct _RTMP_ADAPTER *ad, struct dvt_seudo_sta *sta)
{
	if (sta->mac_entry)
		mac_entry_delete(ad, sta->mac_entry);
}

/*
*
*/
VOID dvt_ut_seudo_sta_template_get(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct dvt_seudo_sta *sta)
{
	os_zero_mem(sta, sizeof(*sta));
	sta->addr[0] = 0x00;
	sta->addr[1] = 0x0C;
	sta->addr[2] = 0x43;
	sta->addr[3] = 0x15;
	sta->addr[4] = 0x13;
	sta->addr[5] = 0x14;
	sta->cap_flag &= DVT_STA_WMM_CAP;
	sta->wdev = wdev;
	sta->phy_mode = (WMODE_G | WMODE_A | WMODE_GN | WMODE_AN | WMODE_AC | WMODE_AX_24G | WMODE_AX_5G);
	sta->phy_mode &= sta->wdev->PhyMode;
}

/*
*
*/
inline struct dvt_notify_event *dvt_ut_notify_get_event(struct _RTMP_ADAPTER *ad, UINT signal)
{
	struct dvt_framework *dvt_ctrl = ad->dvt_ctrl;

	if (signal >= DVT_NOTIFY_EVENT_MAX)
		return NULL;

	return &dvt_ctrl->notify_table[signal];
}

/*
*
*/
VOID *dvt_ut_notify_wait(struct _RTMP_ADAPTER *ad, UINT signal)
{
	struct dvt_notify_event *event = dvt_ut_notify_get_event(ad, signal);

	if (!event)
		return NULL;

	event->is_wait = TRUE;
	RTMP_OS_INIT_COMPLETION(&event->done);

	DVT_LOG("signal,%d&wait,ok", signal);
	if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&event->done, RTMPMsecsToJiffies(100000))) {
		DVT_LOG("signal,%d&wait,fail", signal);
	}
	DVT_LOG("signal,%d&wait,ok", signal);
	return event->out_data;
}

/*
*
*/
VOID *dvt_ut_notify_wait_threshold(struct _RTMP_ADAPTER *ad, UINT signal, UINT sta_id, ULONG threshold)
{
	struct dvt_notify_event *event = dvt_ut_notify_get_event(ad, signal);

	if (!event)
		return NULL;

	event->is_wait = TRUE;
	event->threshold = threshold;
	event->sta_id = sta_id;
	RTMP_OS_INIT_COMPLETION(&event->done);
	DVT_LOG("signal,%d&staid,%d, threshold,%lu", signal, sta_id, threshold);

	if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&event->done, RTMPMsecsToJiffies(100000))) {
		DVT_LOG("signal,%d&wait,fail", signal);
	}
	DVT_LOG("signal,%d&wait,ok", signal);
	return event->out_data;
}

/*
*
*/
static INT dvt_ut_notify_complete(struct _RTMP_ADAPTER *ad, UINT signal, VOID *data)
{
	struct dvt_notify_event *event = dvt_ut_notify_get_event(ad, signal);

	if (!event)
		return DVT_STATUS_FAIL;

	if (event->is_wait) {
		event->out_data = data;
		event->is_wait = FALSE;
		RTMP_OS_COMPLETE(&event->done);
	}
	return DVT_STATUS_OK;
}

/*
*
*/
static INT dvt_ut_notify_tput_complete(struct _RTMP_ADAPTER *ad, UINT signal, VOID *data)
{
	struct _MAC_TABLE_ENTRY *entry = data;
	struct dvt_notify_event *event = dvt_ut_notify_get_event(ad, signal);

	if (!event)
		return DVT_STATUS_FAIL;

	if (event->is_wait) {
		if (entry->wcid == event->sta_id && entry->one_sec_tx_pkts > event->threshold) {
			event->out_data = data;
			event->is_wait = FALSE;
			event->is_check = FALSE;
			event->threshold = 0;
			event->sta_id = 0;
			RTMP_OS_COMPLETE(&event->done);
		}
	}
	return DVT_STATUS_OK;
}

/*
* wifi_system related notify
*/
static INT dvt_notify_wbsys_handler(struct notify_entry *ne, INT event_id, VOID *data)
{
	INT ret = NOTIFY_STAT_OK;
	struct wsys_notify_info *info = data;
	struct wifi_dev *wdev = info->wdev;
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	UINT signal;

	DVT_LOG("event_id,%d&wdev,%d", event_id, wdev->wdev_idx);

	switch (event_id) {
	case WSYS_NOTIFY_CONNT_ACT:
		/*response a STA_TR_ENTRY for checking*/
		dvt_ut_notify_complete(ad, DVT_NOTIFY_WSYS_CONNECT_EVENT, info->v);
		break;
	case WSYS_NOTIFY_DISCONNT_ACT:
		/*response a STA_TR_ENTRY for checking*/
		dvt_ut_notify_complete(ad, DVT_NOTIFY_WSYS_DISCONN_EVENT, info->v);
		break;
	case WSYS_NOTIFY_CLOSE:
		/*response a wdev for checking*/
		dvt_ut_notify_complete(ad, DVT_NOTIFY_WSYS_CLOSE_EVENT, info->wdev);
		break;
	case WSYS_NOTIFY_OPEN:
		/*response a wdev for checking*/
		dvt_ut_notify_complete(ad, DVT_NOTIFY_WSYS_OPEN_EVENT, info->wdev);
		break;
	case WSYS_NOTIFY_LINKUP:
		/*response a wdev for checking*/
		dvt_ut_notify_complete(ad, DVT_NOTIFY_WSYS_LINKUP_EVENT, info->wdev);
		break;
	case WSYS_NOTIFY_LINKDOWN:
		/*response a wdev for checking*/
		dvt_ut_notify_complete(ad, DVT_NOTIFY_WSYS_LINKDOWN_EVENT, info->wdev);
		break;
	case WSYS_NOTIFY_STA_UPDATE:
		/*response a STA_TR_ENTRY for checking*/
		dvt_ut_notify_complete(ad, DVT_NOTIFY_WSYS_STAUPDATE_EVENT, info->v);
		break;
	default:
		signal = DVT_NOTIFY_EVENT_MAX;
		break;
	}
	return ret;
}

struct notify_entry dvt_wsys_ne = {
	.notify_call = dvt_notify_wbsys_handler,
	.priority = WSYS_NOTIFY_PRIORITY_DVT,
	.priv = NULL,
};

/*
* traffic detection related notify
*/
static INT dvt_notify_traffic_handler(struct notify_entry *ne, INT event_id, VOID *data)
{
	INT ret = NOTIFY_STAT_OK;
	struct traffic_notify_info *info = data;
	struct _RTMP_ADAPTER *ad = info->ad;
	UINT signal;

	switch (event_id) {
	case TRAFFIC_NOTIFY_RX_DATA:
		dvt_ut_notify_complete(ad, DVT_NOTIFY_TRAFFIC_RX_DATA_EVENT, info->v);
		break;
	case TRAFFIC_NOTIFY_RX_EVENT:
		dvt_ut_notify_complete(ad, DVT_NOTIFY_TRAFFIC_RX_CMD_EVENT, info->v);
		break;
	case TRAFFIC_NOTIFY_RX_TMR:
		dvt_ut_notify_complete(ad, DVT_NOTIFY_TRAFFIC_RX_TMR_EVENT, info->v);
		break;
	case TRAFFIC_NOTIFY_RX_TXRXV:
		dvt_ut_notify_complete(ad, DVT_NOTIFY_TRAFFIC_RX_TRXV_EVENT, info->v);
		break;
	case TRAFFIC_NOTIFY_RX_TXRX_NOTIFY:
		dvt_ut_notify_complete(ad, DVT_NOTIFY_TRAFFIC_RX_TRX_FREE_EVENT, info->v);
		break;
	case TRAFFIC_NOTIFY_RX_TXS:
		dvt_ut_notify_complete(ad, DVT_NOTIFY_TRAFFIC_RX_TXS_EVENT, info->v);
		break;
	case TRAFFIC_NOTIFY_TPUT_DETECT:
		dvt_ut_notify_tput_complete(ad, DVT_NOTIFY_TRAFFIC_TPUT_DETECT_EVENT, info->v);
		break;
	case TRAFFIC_NOTIFY_WMM_DETECT:
		dvt_ut_notify_complete(ad, DVT_NOTIFY_TRAFFIC_WMM_DETECT_EVENT, info->v);
		break;
	default:
		signal = DVT_NOTIFY_EVENT_MAX;
		break;
	}
	return ret;
}

struct notify_entry dvt_traffic_ne = {
	.notify_call = dvt_notify_traffic_handler,
	.priority = TRAFFIC_NOTIFY_PRIORITY_DVT,
	.priv = NULL,
};

/*
*
*/
static VOID dvt_notify_init(struct _RTMP_ADAPTER *ad)
{
	register_wsys_notifier(&ad->WifiSysInfo, &dvt_wsys_ne);
	register_traffic_notifier(ad, &dvt_traffic_ne);
}

