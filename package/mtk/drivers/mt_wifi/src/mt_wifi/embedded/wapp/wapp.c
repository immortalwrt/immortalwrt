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
****************************************************************************

	Module Name:
	wapp.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef WAPP_SUPPORT
#include "rt_config.h"

#include "wapp/wapp_cmm_type.h"
#define BEACON_REPORT_MAX_OPT_SUB_IE_LEN 250
#define BEACON_REPORT_MAX_IE_LEN (26 + BEACON_REPORT_MAX_OPT_SUB_IE_LEN)

#ifdef DOT11_HE_AX
UINT8 peer_max_bw_cap(INT8 ch_width_set);
#endif /*DOT11_HE_AX*/

UCHAR ESPI_AC_BE_DEFAULT[3] = {0xF8, 0xFF, 0x00};
UCHAR ESPI_AC_BK_DEFAULT[3] = {0xF9, 0xFF, 0x00};
UCHAR ESPI_AC_VO_DEFAULT[3] = {0xFA, 0xFF, 0x00};
UCHAR ESPI_AC_VI_DEFAULT[3] = {0xFB, 0xFF, 0x00};

#define CLI_REQ_MIN_INTERVAL	5 /* sec */

#ifdef CONFIG_MAP_SUPPORT
VOID wapp_send_cac_stop(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 ifindex,
	IN UCHAR channel,
	IN UCHAR ret)
{
	struct wapp_event event;
	wapp_cac_info *cac_info;

	event.event_id = WAPP_CAC_STOP;
	event.ifindex = ifindex;
	cac_info = &event.data.cac_info;
	cac_info->channel = channel;
	cac_info->ret = ret;
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
}

#endif
UINT8 get_channel_utilization(PRTMP_ADAPTER pAd, u32 ifindex)
{
	UCHAR	i, Channel;
	UINT16 	l;
	UINT32  ObssAirTime[DBDC_BAND_NUM] = {0};
	UINT32  MyTxAirTime[DBDC_BAND_NUM] = {0};
	UINT32  MyRxAirTime[DBDC_BAND_NUM] = {0};
	UCHAR ObssAirOccupyPercentage[DBDC_BAND_NUM] = {0};
	UCHAR MyAirOccupyPercentage[DBDC_BAND_NUM] = {0};
	UINT32  res;
	struct wifi_dev *wdev = NULL;

	for (l = 0; l < WDEV_NUM_MAX; l++) {
		if (pAd->wdev_list[l] != NULL) {
			wdev = pAd->wdev_list[l];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == ifindex)) {
				break;
			}
		}
	}

	if (wdev == NULL) {
		return 0;
	}

	Channel = wdev->channel;
	i = HcGetBandByWdev(wdev);
	ObssAirTime[i] = Get_OBSS_AirTime(pAd, i);
	MyTxAirTime[i] = Get_My_Tx_AirTime(pAd, i);
	MyRxAirTime[i] = Get_My_Rx_AirTime(pAd, i);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG,
						"One sec values Channel %d, ObssAirTime[i] %d, MyTxAirTime[i] %d, MyRxAirTime[i] %d\n",
						Channel, ObssAirTime[i], MyTxAirTime[i], MyRxAirTime[i]);
	if (ObssAirTime[i] != 0)
		ObssAirOccupyPercentage[i] = (ObssAirTime[i]*100)/ONE_SEC_2_US;

	if (MyTxAirTime[i] != 0 || MyRxAirTime[i] != 0)
		MyAirOccupyPercentage[i] = ((MyTxAirTime[i] + MyRxAirTime[i]) * 100)/ONE_SEC_2_US;

	res = (MyAirOccupyPercentage[i] + ObssAirOccupyPercentage[i]);
	/* convert to a scale of 255 */
	res *= 255;
	res = (res / 100);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG, "final ch util res %d\n", res);
	return res;
}
VOID wext_send_wapp_qry_rsp2(
	PNET_DEV pNetDev,
	struct wapp_event2 *event)
{

	UINT buflen = sizeof(struct wapp_event2);

	event->len = buflen - sizeof(event->len) - sizeof(event->event_id);

	RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM,
			OID_WAPP_EVENT2, NULL, (PUCHAR)event, sizeof(struct wapp_event2));
}

VOID wext_send_wapp_qry_rsp(
	PNET_DEV pNetDev,
	struct wapp_event *event)
{

	UINT buflen = sizeof(struct wapp_event);

	event->len = buflen - sizeof(event->len) - sizeof(event->event_id);

	RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM,
			OID_WAPP_EVENT, NULL, (PUCHAR)event, sizeof(struct wapp_event));
}

INT wapp_send_wdev_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wapp_dev_info *dev_info;

	event.event_id = WAPP_DEV_QUERY_RSP;
	event.ifindex = req->data.ifindex;
	dev_info = &event.data.dev_info;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex)) {
				dev_info->ifindex = event.ifindex;
				dev_info->dev_type = wdev->wdev_type;
				COPY_MAC_ADDR(dev_info->mac_addr, wdev->if_addr);
				NdisCopyMemory(dev_info->ifname, RtmpOsGetNetDevName(wdev->if_dev), IFNAMSIZ);
				dev_info->radio_id = HcGetBandByWdev(wdev);
				dev_info->adpt_id = (uintptr_t) pAd;
				dev_info->wireless_mode = wdev->PhyMode;
				dev_info->dev_active = HcIsRadioAcq(wdev);
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
				if (IS_MAP_TURNKEY_ENABLE(pAd) &&
					wdev->wdev_type == WDEV_TYPE_STA) {
					if (wdev->func_idx >= MAX_APCLI_NUM)
						continue;
					SetApCliEnableByWdev(pAd, wdev, FALSE);
				}
			}
		}
	}
	return 0;
}

INT wapp_send_wdev_ht_cap_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_ht_cap *ht_cap;

	event.event_id = WAPP_HT_CAP_QUERY_RSP;
	event.ifindex = req->data.ifindex;

	ht_cap = &event.data.ht_cap;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex)) {
				ht_cap->tx_stream = wlan_config_get_tx_stream(wdev);
				ht_cap->rx_stream = wlan_config_get_rx_stream(wdev);
				ht_cap->sgi_20 = (wlan_config_get_ht_gi(wdev) == GI_400) ? \
								 1:0;
				ht_cap->sgi_40 = (wlan_config_get_ht_gi(wdev) == GI_400) ? \
								 1:0;
				ht_cap->ht_40 = (wlan_operate_get_ht_bw(wdev) == BW_40) ? \
								 1:0;
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}
	return 0;
}

INT wapp_send_wdev_vht_cap_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_vht_cap *vht_cap;
	VHT_CAP_INFO drv_vht_cap;
	VHT_OP_IE drv_vht_op;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	NdisZeroMemory(&drv_vht_op, sizeof(VHT_OP_IE));
	NdisCopyMemory(&drv_vht_cap, &pAd->CommonCfg.vht_cap_ie.vht_cap, sizeof(VHT_CAP_INFO));

	event.event_id = WAPP_VHT_CAP_QUERY_RSP;
	event.ifindex = req->data.ifindex;

	vht_cap = &event.data.vht_cap;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex)) {
				mt_WrapSetVHTETxBFCap(pAd, wdev, &drv_vht_cap);

				drv_vht_op.basic_mcs_set.mcs_ss1 = VHT_MCS_CAP_NA;
				drv_vht_op.basic_mcs_set.mcs_ss2 = VHT_MCS_CAP_NA;
				drv_vht_op.basic_mcs_set.mcs_ss3 = VHT_MCS_CAP_NA;
				drv_vht_op.basic_mcs_set.mcs_ss4 = VHT_MCS_CAP_NA;
				drv_vht_op.basic_mcs_set.mcs_ss5 = VHT_MCS_CAP_NA;
				drv_vht_op.basic_mcs_set.mcs_ss6 = VHT_MCS_CAP_NA;
				drv_vht_op.basic_mcs_set.mcs_ss7 = VHT_MCS_CAP_NA;
				drv_vht_op.basic_mcs_set.mcs_ss8 = VHT_MCS_CAP_NA;

				/* BW 80M or 40M is the default setting*/
				switch	(wlan_operate_get_rx_stream(wdev)) {
				case 4:
					drv_vht_op.basic_mcs_set.mcs_ss4 = cap->mcs_nss.max_vht_mcs;
					/* fall through */
				case 3:
					drv_vht_op.basic_mcs_set.mcs_ss3 = cap->mcs_nss.max_vht_mcs;
					/* fall through */
				case 2:
					drv_vht_op.basic_mcs_set.mcs_ss2 = cap->mcs_nss.max_vht_mcs;
					/* fall through */
				case 1:
					drv_vht_op.basic_mcs_set.mcs_ss1 = cap->mcs_nss.max_vht_mcs;
					break;
				}

				/* BW 20M only */
				if (wlan_config_get_vht_bw(wdev) == VHT_BW_2040
					&& wlan_config_get_ht_bw(wdev) == HT_BW_20
					&& cap->mcs_nss.max_vht_mcs == VHT_MCS_CAP_9) {

					switch	(wlan_operate_get_rx_stream(wdev)) {
					case 4:
						drv_vht_op.basic_mcs_set.mcs_ss4 = VHT_MCS_CAP_8;
						/* fall through */
					case 3:
						drv_vht_op.basic_mcs_set.mcs_ss3 = VHT_MCS_CAP_9;
						/* fall through */
					case 2:
						drv_vht_op.basic_mcs_set.mcs_ss2 = VHT_MCS_CAP_8;
						/* fall through */
					case 1:
						drv_vht_op.basic_mcs_set.mcs_ss1 = VHT_MCS_CAP_8;
						break;
					}

				}

				/* BW 160M or 80M+80M */
				if ((wlan_config_get_vht_bw(wdev) == VHT_BW_160 || wlan_config_get_vht_bw(wdev) == VHT_BW_8080)
					&& wlan_config_get_ht_bw(wdev) == HT_BW_40
					&& cap->mcs_nss.max_vht_mcs == VHT_MCS_CAP_9) {
					switch	(wlan_operate_get_rx_stream(wdev)) {
					case 4:
						drv_vht_op.basic_mcs_set.mcs_ss4 = VHT_MCS_CAP_9;
						/* fall through */
					case 3:
						drv_vht_op.basic_mcs_set.mcs_ss3 = VHT_MCS_CAP_8;
						/* fall through */
					case 2:
						drv_vht_op.basic_mcs_set.mcs_ss2 = VHT_MCS_CAP_9;
						/* fall through */
					case 1:
						drv_vht_op.basic_mcs_set.mcs_ss1 = VHT_MCS_CAP_9;
						break;
					}
				}

				NdisMoveMemory(vht_cap->sup_tx_mcs,
								&drv_vht_op.basic_mcs_set,
								sizeof(vht_cap->sup_tx_mcs));
				NdisMoveMemory(vht_cap->sup_rx_mcs,
								&drv_vht_op.basic_mcs_set,
								sizeof(vht_cap->sup_rx_mcs));
				vht_cap->tx_stream = wlan_config_get_tx_stream(wdev);
				vht_cap->rx_stream = wlan_config_get_tx_stream(wdev);
				vht_cap->sgi_80 = (wlan_config_get_ht_gi(wdev) == GI_400) ? \
								 1:0;
				vht_cap->sgi_160 = (wlan_config_get_ht_gi(wdev) == GI_400) ? \
								 1:0;
				vht_cap->vht_160 = (wlan_operate_get_vht_bw(wdev) == VHT_BW_160) ? \
								 1:0;
				vht_cap->vht_8080 = (wlan_operate_get_vht_bw(wdev) == VHT_BW_8080) ? \
								 1:0;
				vht_cap->su_bf = (drv_vht_cap.bfer_cap_su) ? \
								 1:0;
				vht_cap->mu_bf = (drv_vht_cap.bfer_cap_mu) ? \
								 1:0;
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}
	return 0;
}

INT wapp_send_wdev_misc_cap_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_misc_cap *misc_cap;

	event.event_id = WAPP_MISC_CAP_QUERY_RSP;
	event.ifindex = 0;
	misc_cap = &event.data.misc_cap;
	NdisZeroMemory(misc_cap, sizeof(wdev_misc_cap));

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
				event.ifindex = req->data.ifindex;
				misc_cap->max_num_of_cli = 64;
				misc_cap->max_num_of_bss = MAX_MBSSID_NUM(pAd);
				misc_cap->num_of_bss = pAd->ApCfg.BssidNum;
#ifdef CONFIG_MAP_SUPPORT
				misc_cap->max_num_of_block_cli = BLOCK_LIST_NUM;
#endif
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}
	return 0;
}
#ifdef DOT11_HE_AX
u16 get_nss_from_he_chwidth(MAC_TABLE_ENTRY *mac_entry, u8 bw)
{
	int i = 0;
	switch (bw) {
	case HE_BW_20:
	case HE_BW_2040:
	case HE_BW_80: /*For Bw less than equal to 80*/
		if (mac_entry->cap.ch_bw.he_ch_width & (SUPP_40M_CW_IN_24G_BAND | SUPP_40M_80M_CW_IN_5G_BAND))
			for (i = 0; i < DOT11AX_MAX_STREAM; i++) {
				if (mac_entry->cap.rate.he80_rx_nss_mcs[i] == 3)
					break;
			}
		break;
	case HE_BW_160: /*For Bw 160 support*/
		if (mac_entry->cap.ch_bw.he_ch_width & SUPP_160M_CW_IN_5G_BAND)
			for (i = 0; i < DOT11AX_MAX_STREAM; i++) {
				if (mac_entry->cap.rate.he160_rx_nss_mcs[i] == 3)
					break;
			}
		break;
	case HE_BW_8080: /*For Bw 8080 support*/
		if (mac_entry->cap.ch_bw.he_ch_width & SUPP_160M_8080M_CW_IN_5G_BAND)
			for (i = 0; i < DOT11AX_MAX_STREAM; i++) {
				if (mac_entry->cap.rate.he8080_rx_nss_mcs[i] == 3)
					break;
			}
		break;
	default:
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"bw is inwalid!!\n");
		break;
	}
	return i;
}

u16 he_bw_conv(u16 he_bw)
{
	switch (he_bw) {
	case HE_BW_20:
		return BW_20;
	case HE_BW_2040:
		return BW_40;
	case HE_BW_80:
		return BW_80;
	case HE_BW_160:
		return BW_160;
	case HE_BW_8080:
		return BW_8080;
	default:
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"return default he_bw: BW_20\n");
		return BW_20;
	}
}
#endif /*DOT11_HE_AX*/

#ifdef CONFIG_MAP_SUPPORT
INT wapp_fill_client_info_new(
	PRTMP_ADAPTER pAd,
	wapp_client_info *cli_info,
	MAC_TABLE_ENTRY *mac_entry)
{
	STA_TR_ENTRY *tr_entry;
	ULONG DataRate = 0, DataRate_r = 0;
	HTTRANSMIT_SETTING HTPhyMode;
	HETRANSMIT_SETTING HEPhyMode;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#ifdef DOT11_HE_AX
	UINT8 he_dcm = 0, he_mcs = 0, he_nss = 0;
#endif
	USHORT PhyMode;
#ifdef MAP_R3
	int i = 0;
	struct ppdu_caps *ppdu = NULL;
#endif

	tr_entry = &tr_ctl->tr_entry[mac_entry->wcid];

	COPY_MAC_ADDR(cli_info->mac_addr, mac_entry->Addr);

	/*
		wapp_send_cli_query_rsp() will called by user space daemon, and the caller have an for loop
		this lock may suffer system performance.
	*/
	NdisAcquireSpinLock(&pAd->MacTabLock);
	if (mac_entry->wdev) {
		COPY_MAC_ADDR(cli_info->bssid, mac_entry->wdev->bssid);
		PhyMode = mac_entry->wdev->PhyMode;
#ifdef MAP_R3
		ppdu = (struct ppdu_caps *)wlan_config_get_ppdu_caps(mac_entry->wdev);
#endif
	}
	else
		NdisZeroMemory(cli_info->bssid, MAC_ADDR_LEN);
	NdisReleaseSpinLock(&pAd->MacTabLock);

	if (tr_entry)
		cli_info->sta_status = (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ? WAPP_STA_CONNECTED : WAPP_STA_DISCONNECTED;
	else
		cli_info->sta_status =  WAPP_STA_DISCONNECTED;

	cli_info->assoc_time = mac_entry->StaConnectTime;
	cli_info->assoc_req_len = mac_entry->assoc_req_len;
	HEPhyMode.word = (USHORT)mac_entry->map_LastTxRate;
	HTPhyMode.word = (USHORT)mac_entry->map_LastTxRate;
#ifdef MAP_R2
	cli_info->IsReassoc = mac_entry->IsReassocSta;
#endif

#ifdef DOT11_HE_AX
		if (HEPhyMode.field.MODE == MODE_HE_SU_REMAPPING) {
			he_mcs = HEPhyMode.field.MCS & 0xf;
			he_dcm = HEPhyMode.field.MCS  & 0x10 ? 1 : 0;
			he_nss = ((HEPhyMode.field.MCS & (0x3 << 5)) >> 5) + 1;
			get_rate_he(he_mcs, HEPhyMode.field.BW, he_nss, he_dcm, &DataRate);
			cli_info->downlink = (u16) DataRate;
		} else
#endif
		{
			getRate(HTPhyMode, &DataRate);
			cli_info->downlink = (u16) DataRate;
		}
	/* Though NSS1VHT20MCS9 and NSS2VHT20MCS9 rates are not specified in
	* IEEE802.11, we do use them */
	if ((HTPhyMode.field.MODE == MODE_VHT) && (HTPhyMode.field.BW == BW_20) &&
		((HTPhyMode.field.MCS & 0xf) == 9)) {
		u8 vht_nss = ((HTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1;
		if (vht_nss == 1)
			cli_info->downlink = HTPhyMode.field.ShortGI ? 96 : 86;
		else if (vht_nss == 2)
			cli_info->downlink = HTPhyMode.field.ShortGI ? 192 : 173;
	}

	HTPhyMode.word = (USHORT) mac_entry->map_LastRxRate;
	getRate(HTPhyMode, &DataRate_r);
	cli_info->uplink = (u16) DataRate_r;

	/* Though NSS1VHT20MCS9 and NSS2VHT20MCS9 rates are not specified in
	* IEEE802.11, we do use them */
	if ((HTPhyMode.field.MODE == MODE_VHT) && (HTPhyMode.field.BW == BW_20) &&
		((HTPhyMode.field.MCS & 0xf) == 9)) {
		u8 vht_nss = ((HTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1;
		if (vht_nss == 1)
			cli_info->uplink = HTPhyMode.field.ShortGI ? 96 : 86;
		else if (vht_nss == 2)
			cli_info->uplink = HTPhyMode.field.ShortGI ? 192 : 173;
	}

	cli_info->uplink_rssi = RTMPAvgRssi(pAd, &mac_entry->RssiSample);
#ifdef CONFIG_DOT11V_WNM
	cli_info->cli_caps.btm_capable = mac_entry->bBSSMantSTASupport == TRUE ? 1 : 0;
#endif
	cli_info->bLocalSteerDisallow = false;
#ifdef DOT11K_RRM_SUPPORT
	if ((mac_entry->RrmEnCap.field.BeaconPassiveMeasureCap ||
		mac_entry->RrmEnCap.field.BeaconActiveMeasureCap)) {
		cli_info->cli_caps.rrm_capable = 1;
	} else {
		cli_info->cli_caps.rrm_capable = 0;
	}
#endif
#ifdef MBO_SUPPORT
	cli_info->cli_caps.mbo_capable = (mac_entry->bIndicateCDC || mac_entry->bIndicateNPC) ? 1 : 0;
#endif
	/* Phy Caps */
	cli_info->cli_caps.phy_mode = mac_entry->MaxHTPhyMode.field.MODE;
	/*11 AX Support*/
#ifdef DOT11_HE_AX
	if (cli_info->cli_caps.phy_mode == MODE_HE) {
		if (mac_entry->wdev && WMODE_CAP_2G(mac_entry->wdev->PhyMode))
			cli_info->cli_caps.bw = peer_max_bw_cap(mac_entry->cap.ch_bw.he_ch_width & 0x01);
		else
			cli_info->cli_caps.bw = peer_max_bw_cap(mac_entry->cap.ch_bw.he_ch_width & 0x0E);
		cli_info->cli_caps.nss_he.nss_80 = get_nss_from_he_chwidth(mac_entry, HE_BW_80);
		cli_info->cli_caps.nss_he.nss_160 = get_nss_from_he_chwidth(mac_entry, HE_BW_160);
		cli_info->cli_caps.nss_he.nss_8080 = get_nss_from_he_chwidth(mac_entry, HE_BW_8080);
		cli_info->cli_caps.nss = get_nss_from_he_chwidth(mac_entry, cli_info->cli_caps.bw);
		cli_info->cli_caps.bw = he_bw_conv(cli_info->cli_caps.bw);
	} else
#endif /*DOT11_HE_AX*/
	{ /*Non 11 AX Support*/
	cli_info->cli_caps.bw = mac_entry->MaxHTPhyMode.field.BW;
	cli_info->cli_caps.nss = ((mac_entry->MaxHTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1;
	}
	cli_info->bBTMSteerDisallow = false;
	/*traffic stats*/
	cli_info->bytes_sent = mac_entry->TxBytesMAP;
	cli_info->bytes_received = mac_entry->RxBytesMAP;
	cli_info->packets_sent = mac_entry->TxPackets.u.LowPart;
	cli_info->packets_received = mac_entry->RxPackets.u.LowPart;
	cli_info->tx_packets_errors = 0; /* to do */
	cli_info->rx_packets_errors = 0; /* to do */
	cli_info->retransmission_count = 0; /* to do */
	cli_info->link_availability = 50; /* to do */
	cli_info->tx_tp = (u32)(mac_entry->AvgTxBytes);
	cli_info->rx_tp = (u32)(mac_entry->AvgRxBytes);

#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd) &&
		IS_ENTRY_A4(mac_entry)) {
		cli_info->is_APCLI = 1;
	} else
#endif
		cli_info->is_APCLI = 0;

#ifdef MAP_R3
	cli_info->tid_cnt = MAX_TID;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WF6:Driver:Setting TID value for %02x:%02x:%02x:%02x:%02x:%02x\n",
		PRINT_MAC(cli_info->mac_addr));
	if (ppdu) {
		for (i = 0; i < MAX_TID; i++) {
			cli_info->status_tlv[i].tid = i;
			if (WMODE_CAP_AX(PhyMode))
				cli_info->status_tlv[i].tid_q_size = ppdu->he_tx_ba_wsize;
			else
				cli_info->status_tlv[i].tid_q_size = ppdu->non_he_tx_ba_wsize;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WF6:Driver:tid_cnt %x, tid %x, tid_q_size %x\n",
				cli_info->tid_cnt, cli_info->status_tlv[i].tid, cli_info->status_tlv[i].tid_q_size);
		}
	}
#endif


	return 0;
}

#endif


INT wapp_fill_client_info(
	PRTMP_ADAPTER pAd,
	wapp_client_info *cli_info,
	MAC_TABLE_ENTRY *mac_entry)
{
	STA_TR_ENTRY *tr_entry;

	ULONG DataRate = 0, DataRate_r = 0;
	HTTRANSMIT_SETTING HTPhyMode;
	EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
	HTTRANSMIT_SETTING LastTxRate;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#ifdef DOT11_HE_AX
	UINT8 he_dcm = 0, he_mcs = 0;

#endif
	USHORT PhyMode;
#ifdef MAP_R3
	int i = 0;
	struct ppdu_caps *ppdu = NULL;
#endif
	os_zero_mem(&rTxStatResult, sizeof(EXT_EVENT_TX_STATISTIC_RESULT_T));
	MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, mac_entry->wcid, &rTxStatResult);
	LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
	LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
	LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
	LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
	LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

#ifdef DOT11_HE_AX
	if (LastTxRate.field.MODE == MODE_HE_SU_REMAPPING) {
		he_mcs = rTxStatResult.rEntryTxRate.MCS & 0xf;
		he_dcm = rTxStatResult.rEntryTxRate.MCS & 0x10 ? 1 : 0;
		get_rate_he(he_mcs, rTxStatResult.rEntryTxRate.BW, rTxStatResult.rEntryTxRate.VhtNss, he_dcm, &DataRate);
	} else
#endif
	if (LastTxRate.field.MODE == MODE_VHT)
		LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
	else if (LastTxRate.field.MODE == MODE_OFDM)
		LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
	else
		LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

	mac_entry->LastTxRate = (UINT32)LastTxRate.word;

	tr_entry = &tr_ctl->tr_entry[mac_entry->wcid];

	COPY_MAC_ADDR(cli_info->mac_addr, mac_entry->Addr);

	NdisAcquireSpinLock(&pAd->MacTabLock);
	if (mac_entry->wdev) {
		COPY_MAC_ADDR(cli_info->bssid, mac_entry->wdev->bssid);
		PhyMode = mac_entry->wdev->PhyMode;
#ifdef MAP_R3
		ppdu = (struct ppdu_caps *)wlan_config_get_ppdu_caps(mac_entry->wdev);
#endif
	} else
		NdisZeroMemory(cli_info->bssid, MAC_ADDR_LEN);
	NdisReleaseSpinLock(&pAd->MacTabLock);

	if (tr_entry)
		cli_info->sta_status = (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ? WAPP_STA_CONNECTED : WAPP_STA_DISCONNECTED;
	else
		cli_info->sta_status = WAPP_STA_DISCONNECTED;

	cli_info->assoc_time = mac_entry->StaConnectTime;
#ifdef CONFIG_MAP_SUPPORT
	cli_info->assoc_req_len = mac_entry->assoc_req_len;
#ifdef MAP_R2
	cli_info->IsReassoc = mac_entry->IsReassocSta;
#endif
#endif /* MAP_SUPPORT */
	HTPhyMode.word = (USHORT)mac_entry->LastTxRate;
#ifdef DOT11_HE_AX
		if (LastTxRate.field.MODE == MODE_HE_SU_REMAPPING) {
			cli_info->downlink = (u16) DataRate;
		} else
#endif
		{
			getRate(HTPhyMode, &DataRate);
			cli_info->downlink = (u16) DataRate;
		}
	/* Though NSS1VHT20MCS9 and NSS2VHT20MCS9 rates are not specified in
	 * IEEE802.11, we do use them */
	if ((HTPhyMode.field.MODE == MODE_VHT) && (HTPhyMode.field.BW == BW_20) &&
			((HTPhyMode.field.MCS & 0xf) == 9)) {
		u8 vht_nss = ((HTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1;
		if (vht_nss == 1)
			cli_info->downlink = HTPhyMode.field.ShortGI ? 96 : 86;
		else if (vht_nss == 2)
			cli_info->downlink = HTPhyMode.field.ShortGI ? 192 : 173;
	}

	HTPhyMode.word = (USHORT) mac_entry->LastRxRate;
	getRate(HTPhyMode, &DataRate_r);
	cli_info->uplink = (u16) DataRate_r;

	cli_info->uplink_rssi = rtmp_avg_rssi(pAd, &mac_entry->RssiSample);
#ifdef CONFIG_DOT11V_WNM
	cli_info->cli_caps.btm_capable = mac_entry->bBSSMantSTASupport == TRUE ? 1 : 0;
#endif
	cli_info->bLocalSteerDisallow = false;
#ifdef DOT11K_RRM_SUPPORT
	if ((mac_entry->RrmEnCap.field.BeaconPassiveMeasureCap ||
		 mac_entry->RrmEnCap.field.BeaconActiveMeasureCap)) {
		cli_info->cli_caps.rrm_capable = 1;
	} else {
		cli_info->cli_caps.rrm_capable = 0;
	}
#endif
#ifdef MBO_SUPPORT
	cli_info->cli_caps.mbo_capable = (mac_entry->bIndicateCDC || mac_entry->bIndicateNPC) ? 1 : 0;
#endif
	/* Phy Caps */
	cli_info->cli_caps.phy_mode = mac_entry->MaxHTPhyMode.field.MODE;
	/*11 AX Support*/
#ifdef DOT11_HE_AX
	if (cli_info->cli_caps.phy_mode == MODE_HE) {
		if (mac_entry->wdev && WMODE_CAP_2G(PhyMode))
			cli_info->cli_caps.bw = peer_max_bw_cap(mac_entry->cap.ch_bw.he_ch_width & 0x01);
		else
			cli_info->cli_caps.bw = peer_max_bw_cap(mac_entry->cap.ch_bw.he_ch_width & 0x0E);
		cli_info->cli_caps.nss_he.nss_80 = get_nss_from_he_chwidth(mac_entry, HE_BW_80);
		cli_info->cli_caps.nss_he.nss_160 = get_nss_from_he_chwidth(mac_entry, HE_BW_160);
		cli_info->cli_caps.nss_he.nss_8080 = get_nss_from_he_chwidth(mac_entry, HE_BW_8080);
		cli_info->cli_caps.nss = get_nss_from_he_chwidth(mac_entry, cli_info->cli_caps.bw);
		cli_info->cli_caps.bw = he_bw_conv(cli_info->cli_caps.bw);
	} else
#endif /*DOT11_HE_AX*/
	{ /*Non 11 AX Support*/
		cli_info->cli_caps.bw = mac_entry->MaxHTPhyMode.field.BW;
		cli_info->cli_caps.nss = ((mac_entry->MaxHTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1;
	}
	cli_info->bBTMSteerDisallow = false;
	/*traffic stats*/
#ifdef CONFIG_MAP_SUPPORT
	cli_info->bytes_sent = mac_entry->TxBytesMAP;
	cli_info->bytes_received = mac_entry->RxBytesMAP;
#else
	cli_info->bytes_sent = mac_entry->TxBytes;
	cli_info->bytes_received = mac_entry->RxBytes;
#endif
	cli_info->packets_sent = mac_entry->TxPackets.u.LowPart;
	cli_info->packets_received = mac_entry->RxPackets.u.LowPart;
	cli_info->tx_packets_errors = 0; /* to do */
	cli_info->rx_packets_errors = 0; /* to do */
	cli_info->retransmission_count = 0; /* to do */
	cli_info->link_availability = 50; /* to do */
	cli_info->tx_tp = (u32)(mac_entry->AvgTxBytes);
	cli_info->rx_tp = (u32)(mac_entry->AvgRxBytes);
#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd) &&
		IS_ENTRY_A4(mac_entry)) {
		cli_info->is_APCLI = 1;
	} else
#endif
		cli_info->is_APCLI = 0;

#ifdef MAP_R3
	cli_info->tid_cnt = MAX_TID;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WF6:Driver:Setting TID value for %02x:%02x:%02x:%02x:%02x:%02x\n",
		PRINT_MAC(cli_info->mac_addr));
	if (ppdu) {
		for (i = 0; i < MAX_TID; i++) {
			cli_info->status_tlv[i].tid = i;
			if (WMODE_CAP_AX(PhyMode))
				cli_info->status_tlv[i].tid_q_size = ppdu->he_tx_ba_wsize;
			else
				cli_info->status_tlv[i].tid_q_size = ppdu->non_he_tx_ba_wsize;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WF6:Driver:tid_cnt %x, tid %x, tid_q_size %x\n",
				cli_info->tid_cnt, cli_info->status_tlv[i].tid, cli_info->status_tlv[i].tid_q_size);
		}
	}
#endif
	return 0;
}

INT wapp_send_cli_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wapp_event event;
	wapp_client_info *cli_info;
	MAC_TABLE_ENTRY *mac_entry;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);

	event.event_id = WAPP_CLI_QUERY_RSP;
	cli_info = &event.data.cli_info;

	for (i = 0; i < wtbl_max_num; i++) {
		mac_entry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_CLIENT(mac_entry)
			&& NdisCmpMemory(mac_entry->Addr, &req->data.mac_addr, MAC_ADDR_LEN) == 0
			&& mac_entry->wdev->if_dev
			&& req->data.ifindex == RtmpOsGetNetIfIndex(mac_entry->wdev->if_dev)) {
			wapp_fill_client_info_new(pAd, cli_info, mac_entry);
#ifdef MAP_R2
			/*printk("build sta ext params\n");*/
			cli_info->ext_metric_info.sta_info.last_data_dl_rate = mac_entry->LastTxRate;
			cli_info->ext_metric_info.sta_info.last_data_ul_rate = mac_entry->LastRxRate;
			cli_info->ext_metric_info.sta_info.utilization_rx =
				mac_entry->TxRxTime[0][0] + mac_entry->TxRxTime[1][0] +
				mac_entry->TxRxTime[2][0] + mac_entry->TxRxTime[3][0];
			cli_info->ext_metric_info.sta_info.utilization_tx =
				mac_entry->TxRxTime[0][1] + mac_entry->TxRxTime[1][1] +
				mac_entry->TxRxTime[2][1] + mac_entry->TxRxTime[3][1];
#endif
			event.ifindex = req->data.ifindex;
			wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			return 0;
		}
	}

	COPY_MAC_ADDR(cli_info->mac_addr, req->data.mac_addr);
	cli_info->sta_status = WAPP_STA_DISCONNECTED;
	event.ifindex = req->data.ifindex;
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}

INT wapp_send_cli_list_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	struct wapp_event event;

	event.event_id = WAPP_CLI_LIST_QUERY_RSP;
	event.ifindex = req->data.ifindex;
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}


INT wapp_handle_cli_list_query(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wapp_client_info *cli_info;
	MAC_TABLE_ENTRY *mac_entry;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);

	event.event_id = WAPP_CLI_QUERY_RSP;
	cli_info = &event.data.cli_info;

	for (i = 0; i < wtbl_max_num; i++) {
		mac_entry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_CLIENT(mac_entry)) {/* report all entry no matter which wdev it is belonged */
			wdev = mac_entry->wdev;
			if (wdev->if_dev == NULL) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "!!!! wdev->ifdev = NULL\n");
				continue;
			}
#ifdef CONFIG_MAP_SUPPORT
			wapp_fill_client_info_new(pAd, cli_info, mac_entry);
#else
			wapp_fill_client_info(pAd, cli_info, mac_entry);
#endif
#ifdef MAP_R2
			/*printk("MAP R2 wapp_handle_cli_list_query\n");*/
			cli_info->ext_metric_info.sta_info.last_data_dl_rate = cli_info->downlink;
			cli_info->ext_metric_info.sta_info.last_data_ul_rate = cli_info->uplink;
			cli_info->ext_metric_info.sta_info.utilization_rx =
				mac_entry->TxRxTime[0][0] + mac_entry->TxRxTime[1][0] +
				mac_entry->TxRxTime[2][0] + mac_entry->TxRxTime[3][0];
			cli_info->ext_metric_info.sta_info.utilization_tx =
				mac_entry->TxRxTime[0][1] + mac_entry->TxRxTime[1][1] +
				mac_entry->TxRxTime[2][1] + mac_entry->TxRxTime[3][1];
#endif
				event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
#ifdef MBO_SUPPORT
			if (IS_MBO_ENABLE(wdev)) {
				if (mac_entry->bIndicateNPC && mac_entry->bindicate_NPC_event){
					MboIndicateStaInfoToDaemon(pAd,
											&mac_entry->MboStaInfoNPC,
											MBO_MSG_STA_PREF_UPDATE);
					 mac_entry->bindicate_NPC_event = FALSE;
				}
				if (mac_entry->bIndicateCDC && mac_entry->bindicate_CDC_event){
					MboIndicateStaInfoToDaemon(pAd,
											&mac_entry->MboStaInfoCDC,
											MBO_MSG_CDC_UPDATE);
					mac_entry->bindicate_CDC_event = FALSE;
				}
			}
#endif /* MBO_SUPPORT */
		}
	}

	wapp_send_cli_list_query_rsp(pAd, req);
	return 0;
}

/* client assoc */
INT wapp_send_cli_join_event(
	PRTMP_ADAPTER pAd,
	MAC_TABLE_ENTRY *mac_entry)
{
	struct wifi_dev *wdev;
	struct wapp_event event;
	wapp_client_info *cli_info;

	if (mac_entry) {
		wdev = mac_entry->wdev;
		if (wdev->if_dev) {
			event.event_id = WAPP_CLI_JOIN_EVENT;
			event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
			cli_info = &event.data.cli_info;
			wapp_fill_client_info_new(pAd, cli_info, mac_entry);
			wext_send_wapp_qry_rsp(pAd->net_dev, &event);
#ifdef MBO_SUPPORT
		if (IS_MBO_ENABLE(wdev)) {
			if (mac_entry->bIndicateNPC && mac_entry->bindicate_NPC_event){
				MboIndicateStaInfoToDaemon(pAd, &mac_entry->MboStaInfoNPC, MBO_MSG_STA_PREF_UPDATE);
				mac_entry->bindicate_NPC_event = FALSE;
			}
			if (mac_entry->bIndicateCDC && mac_entry->bindicate_CDC_event){
				MboIndicateStaInfoToDaemon(pAd, &mac_entry->MboStaInfoCDC, MBO_MSG_CDC_UPDATE);
				mac_entry->bindicate_CDC_event	= FALSE;
			}
		}
#endif /* MBO_SUPPORT */
		}
	}

	return 0;
}

/* client disaccos */
INT wapp_send_cli_leave_event(
	PRTMP_ADAPTER pAd,
	UINT32 ifindex,
	UCHAR *mac_addr,
	MAC_TABLE_ENTRY *mac_entry)
{
	struct wapp_event2 event;
	wapp_client_info *cli_info;

	event.event_id = WAPP_CLI_LEAVE_EVENT;
	event.ifindex = ifindex;
	cli_info = &event.data.cli_info;
#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd) &&
		IS_ENTRY_A4(mac_entry)) {
		cli_info->is_APCLI = 1;
	} else
#endif
		cli_info->is_APCLI = 0;
	COPY_MAC_ADDR(cli_info->mac_addr, mac_addr);
	cli_info->disassoc_reason = mac_entry->DisconnectReason;
	wext_send_wapp_qry_rsp2(pAd->net_dev, &event);

	return 0;
}

#ifdef MAP_R2

INT wapp_send_sta_disassoc_stats_event(
	PRTMP_ADAPTER pAd,
	MAC_TABLE_ENTRY *pEntry,
	USHORT reason)
{
	struct wapp_event event;
	wapp_client_info *cli_info;

	if (pEntry->wdev->if_dev) {
		event.event_id = WAPP_STA_DISASSOC_EVENT;
		cli_info = &event.data.cli_info;

		wapp_fill_client_info_new(pAd, cli_info, pEntry);
		cli_info->disassoc_reason = reason;
		/*printk("disassoc stats evt: reason code: %d\n", cli_info->disassoc_reason);*/
		event.ifindex = pEntry->wdev->if_dev->ifindex;
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
	return 0;
}

void wapp_handle_sta_disassoc(PRTMP_ADAPTER pAd, UINT16 wcid, UINT16 Reason)
{
	STA_TR_ENTRY *tr_entry	= &pAd->MacTab.tr_entry[wcid];
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[wcid];

	/*printk(" MAP_R2 %s: %d\n",__func__, Reason);*/
	if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
		pEntry->DisconnectReason = Reason;
	else
		wapp_send_sta_connect_rejected(pAd, pEntry->wdev, pEntry->Addr,
					pEntry->bssid,
					WAPP_ASSOC, Reason, 0, Reason);
}

#endif


INT wapp_send_apcli_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
#ifdef APCLI_SUPPORT
	u8 i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wapp_client_info *cli_info;
	MAC_TABLE_ENTRY *mac_entry;
	PSTA_ADMIN_CONFIG apcli_entry;

	event.event_id = WAPP_APCLI_QUERY_RSP;
	event.ifindex = req->data.ifindex;
	cli_info = &event.data.cli_info;

	for (i = 0; i < MAX_MULTI_STA; i++) {
		apcli_entry = &pAd->StaCfg[i];
		wdev = &apcli_entry->wdev;
		if ((apcli_entry->ApcliInfStat.Valid == TRUE)
			&& wdev->if_dev
			&& (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
			mac_entry = &pAd->MacTab.Content[apcli_entry->MacTabWCID];
			if (IS_ENTRY_PEER_AP(mac_entry)) {
				wapp_fill_client_info_new(pAd, cli_info, mac_entry);
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
				break;
			}
		}
	}
#endif

	return 0;
}


INT wapp_send_cli_probe_event(
	PRTMP_ADAPTER pAd,
	UINT32 ifindex,
	UCHAR *mac_addr,
	MLME_QUEUE_ELEM *elem)
{
	struct wapp_event event;
	wapp_probe_info *probe_info;
	struct raw_rssi_info *raw_rssi;
	RSSI_SAMPLE rssi;

	raw_rssi = &elem->rssi_info;

	rssi.AvgRssi[0] = raw_rssi->raw_rssi[0];
	rssi.AvgRssi[1] = raw_rssi->raw_rssi[1];
	rssi.AvgRssi[2] = raw_rssi->raw_rssi[2];
	rssi.AvgRssi[3] = raw_rssi->raw_rssi[3];

	event.event_id = WAPP_CLI_PROBE_EVENT;
	event.ifindex = ifindex;
	probe_info = &event.data.probe_info;
	COPY_MAC_ADDR(probe_info->mac_addr, mac_addr);
	probe_info->channel = elem->Channel;
	probe_info->rssi = rtmp_avg_rssi(pAd, &rssi);
	if (elem->MsgLen <= PREQ_IE_LEN) {
		NdisCopyMemory(probe_info->preq, elem->Msg, elem->MsgLen);
		probe_info->preq_len = elem->MsgLen;
	} else {
		NdisCopyMemory(probe_info->preq, elem->Msg, PREQ_IE_LEN);
		probe_info->preq_len = PREQ_IE_LEN;
	}

	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}

VOID wapp_send_bcn_report(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PUCHAR report,
	IN ULONG report_len)
{
	struct wifi_dev *wdev;
	struct wapp_event *event = NULL;
	UCHAR count = 0, i = 0;
	PEID_STRUCT eid_ptr = NULL;
	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		os_alloc_mem(pAd, (UCHAR **)&event, sizeof(*event));
		if (!event)
			return;
		NdisZeroMemory(event, sizeof(*event));
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(%s) Sta Addr = "MACSTR"\n",
				__func__, MAC2STR(pEntry->Addr));
		wdev = pEntry->wdev;
		if (wdev->if_dev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "!!!! wdev->ifdev = NULL\n");
			os_free_mem(event);
			return;
		}
		event->event_id = WAPP_RCEV_BCN_REPORT;
		event->ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		eid_ptr = (PEID_STRUCT) report;
		COPY_MAC_ADDR(event->data.bcn_rpt_info.sta_addr, pEntry->Addr);

		if (report_len > 0) {
			if (report_len % BCN_RPT_LEN)
				count = report_len/BCN_RPT_LEN + 1;
			else
				count = report_len/BCN_RPT_LEN;
			for (i = 0 ; i < count ; i++) {
				if (i == (count - 1)) {
					event->data.bcn_rpt_info.bcn_rpt_len = report_len;
					event->data.bcn_rpt_info.last_fragment = 1;
					NdisCopyMemory(event->data.bcn_rpt_info.bcn_rpt, &report[i*BCN_RPT_LEN], report_len);
				} else {
					event->data.bcn_rpt_info.bcn_rpt_len = BCN_RPT_LEN;
					event->data.bcn_rpt_info.last_fragment = 0;
					NdisCopyMemory(event->data.bcn_rpt_info.bcn_rpt, &report[i*BCN_RPT_LEN], BCN_RPT_LEN);
					report_len -= BCN_RPT_LEN;
				}
				wext_send_wapp_qry_rsp(pAd->net_dev, event);
			}
		} else {
			event->data.bcn_rpt_info.bcn_rpt_len = report_len;
			event->data.bcn_rpt_info.last_fragment = 1;
			wext_send_wapp_qry_rsp(pAd->net_dev, event);
		}
		os_free_mem(event);
	}
}


VOID wapp_send_bcn_report_complete(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{
	struct wifi_dev *wdev;
	struct wapp_event event;

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(%s) Sta Addr = "MACSTR"\n",
				__func__, MAC2STR(pEntry->Addr));
		wdev = pEntry->wdev;
		if (wdev->if_dev) {
			event.event_id = WAPP_RCEV_BCN_REPORT_COMPLETE;
			event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
			COPY_MAC_ADDR(event.data.bcn_rpt_info.sta_addr, pEntry->Addr);
			wext_send_wapp_qry_rsp(pAd->net_dev, &event);
		}
	}
}

#ifdef AIR_MONITOR
VOID wapp_send_air_mnt_rssi(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PMNT_STA_ENTRY pMntEntry)
{
	struct wifi_dev *wdev;
	struct wapp_event event;

	if (pEntry && IS_ENTRY_MONITOR(pEntry)) {
		wdev = pEntry->wdev;
		if (wdev->if_dev) {
			event.event_id = WAPP_RCEV_MONITOR_INFO;
			event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
			COPY_MAC_ADDR(event.data.mnt_info.sta_addr, pMntEntry->addr);
			event.data.mnt_info.rssi = pMntEntry->RssiSample.AvgRssi[0];
			wext_send_wapp_qry_rsp(pAd->net_dev, &event);
		}
	}
}
#endif

#ifdef CONFIG_MAP_SUPPORT
VOID wapp_send_cac_period_event(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 ifindex,
	IN UCHAR channel,
	IN UCHAR cac_enable,
	IN USHORT cac_time)
{
	struct wapp_event event;
	wapp_cac_info *cac_info = NULL;
	event.event_id = WAPP_CAC_PERIOD_EVENT;
	event.ifindex = ifindex;
	cac_info = &event.data.cac_info;
	cac_info->channel = channel;
	cac_info->ret = cac_enable;
	cac_info->cac_timer = cac_time;
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
}
#endif

#ifdef DPP_R2_SUPPORT
VOID RTMPIoctlGetCCEResults(
	IN	PRTMP_ADAPTER	pAdapter, struct wapp_req *req) {
	struct wapp_event event;
	BSS_ENTRY *bss;
	struct cce_vendor_ie_result result;
	int i = 0, index = 0, j = 0;
	UINT16 l = 0;
	BSS_TABLE *ScanTab = NULL;
	struct wifi_dev *wdev = NULL;
	u8 dup = 0;
#ifdef MAP_R3_6E_SUPPORT
	u8 band_6g = 0;
	int k = 0;
#endif

	for (l = 0; l < WDEV_NUM_MAX; l++) {
		if (pAdapter->wdev_list[l] != NULL) {
			wdev = pAdapter->wdev_list[l];
			if (wdev && wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex))
				break;
		}
	}

	if (wdev == NULL) {
		MTWF_DBG(pAdapter, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"wdev is null\n");
		return;
	}

	ScanTab = get_scan_tab_by_wdev(pAdapter, wdev);

	NdisZeroMemory(&result, sizeof(struct cce_vendor_ie_result));

	if (ScanTab->BssNr == 0) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s: no bss present in scan tab\n", __func__);
		/*return;*/
	}

#ifdef MAP_R3_6E_SUPPORT
	if (WMODE_CAP_6G(wdev->PhyMode)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		" DPP %s: 6G BAND wdev found\n", __func__);
		band_6g = 1;
	}
#endif

	for (i = 0; i < ScanTab->BssNr; i++) {
		dup = 0;
		bss = &ScanTab->BssEntry[i];

		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		" DPP %s: ABOVE CCE res_channl[%d] = %d, cce_flag:%d, cce_channel:%d\n"
		, __func__, i, result.cce_ch[i], bss->cce_vendor_ie_found, bss->Channel);

		if (bss->cce_vendor_ie_found == TRUE) {
			/*check if the channel is already in the list*/
			for (j = 0; j < index; j++) {
				if (result.cce_ch[j] == bss->Channel
#ifdef MAP_R3_6E_SUPPORT
					|| (band_6g && (result.rnr_6e_ch[j] == bss->Channel))
#endif
				) {
					dup = 1;
					break;
				}
			}
			if (dup == 1)
				continue;

#ifdef MAP_R3_6E_SUPPORT
			if (!band_6g)
#endif
				result.cce_ch[index] = bss->Channel;

#ifdef MAP_R3_6E_SUPPORT
			else if (bss->Channel != 0) {
				/* Appending 6G Band Scan channels in rnr array*/
				result.rnr_6e_ch[k] = bss->Channel;
				k = k + 1;
				result.rnr_6e_num = k;
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					" DPP %s: PSC SCAN channel%d,\n", __func__, result.rnr_6e_ch[index]);
			}
#endif
#ifdef MAP_R3_6E_SUPPORT
			if (bss->rnr_info.channel != 0 && (!band_6g)) {
				/* Appending RNR Information when scannig for 2G,5G band*/
				dup = 0;
				for (j = 0; j < k; j++) {
					if (result.rnr_6e_ch[j] == bss->Channel) {
						dup = 1;
						break;
					}
				}

				if (dup == 0) {
					result.rnr_6e_ch[k] = bss->rnr_info.channel;
					k = k + 1;
					result.rnr_6e_num = k;
				}
			}

			if (!band_6g) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					" DPP %s: bss->rnr_info.channel%d,\n", __func__, bss->rnr_info.channel);
#endif
				result.num++;
#ifdef MAP_R3_6E_SUPPORT
			}
#endif
			index++;
		}
	}

	for (i = 0; i < result.num; i++)
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"%s: result.num = %d, channl[%d] = %d\n", __func__, result.num, i, result.cce_ch[i]);

	event.event_id = WAPP_DPP_CCE_RSP;
	event.ifindex = req->data.ifindex;
	NdisCopyMemory(&event.data.cce_ie_result, &result, sizeof(struct cce_vendor_ie_result));
	wext_send_wapp_qry_rsp(pAdapter->net_dev, &event);

	return;
}

#endif

VOID wapp_send_csa_event(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 ifindex,
	IN UCHAR new_channel)
{
	struct wapp_event event;
	wapp_csa_info *csa_info;

	event.event_id = WAPP_CSA_EVENT;
	event.ifindex = ifindex;
	csa_info = &event.data.csa_info;
	csa_info->new_channel = new_channel;
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
}

VOID wapp_send_cli_active_change(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN STA_STATUS stat)
{
	struct wifi_dev *wdev;
	struct wapp_event event;

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		wdev = pEntry->wdev;
		if (wdev->if_dev) {
			event.event_id = WAPP_CLI_ACTIVE_CHANGE;
			event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
			event.data.cli_info.status = stat;
			COPY_MAC_ADDR(event.data.cli_info.mac_addr, pEntry->Addr);
			wext_send_wapp_qry_rsp(pAd->net_dev, &event);
		}
	}
}

VOID setChannelList(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	wdev_chn_info *chn_list)
{
	int i = 0;
#ifdef CONFIG_MAP_SUPPORT
	UCHAR band_idx;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	band_idx = HcGetBandByWdev(wdev);
#endif
	for (i = 0; i < pAd->ChannelListNum && i < (sizeof(chn_list->ch_list)/(sizeof(struct chnList)));
		i++) {
		chn_list->ch_list[i].channel =  pAd->ChannelList[i].Channel;

		/* Set Preference & reason */
		if (pAd->ChannelList[i].DfsReq) {
#ifdef CONFIG_MAP_SUPPORT /* TODO: move to MAP */
			chn_list->ch_list[i].pref |= (OP_DISALLOWED_DUE_TO_DFS | NON_PREF);
			if ((pAd->CommonCfg.RDDurRegion == CE) &&
				DfsCacRestrictBand(pAd, pDfsParam->band_bw[band_idx], pAd->ChannelList[i].Channel,0)) {
				chn_list->ch_list[i].cac_timer = 605;
			} else
				chn_list->ch_list[i].cac_timer = 65;
#endif /* CONFIG_MAP_SUPPORT */
		}
		if (pAd->ChannelList[i].Channel == wlan_operate_get_cen_ch_1(wdev)) {
#ifdef CONFIG_MAP_SUPPORT /* TODO: move to MAP */
			chn_list->ch_list[i].pref |= PREF_SCORE_14;
#endif /* CONFIG_MAP_SUPPORT */
		}
	}

}


INT wapp_send_chn_list_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_chn_info *chn_list;

	event.event_id = WAPP_CHN_LIST_RSP;
	event.ifindex = req->data.ifindex;
	chn_list = &event.data.chn_list;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex)) {
				chn_list->band = wdev->PhyMode;
				chn_list->op_ch = wlan_operate_get_prim_ch(wdev);
				chn_list->op_class = get_regulatory_class(pAd, wdev->channel, wdev->PhyMode, wdev);
				chn_list->ch_list_num = pAd->ChannelListNum;
				chn_list->dl_mcs = wdev->HTPhyMode.field.MCS;
				setChannelList(pAd, wdev, chn_list);
#ifdef CONFIG_MAP_SUPPORT /* TODO: move to MAP */
				chn_list->non_op_chn_num = getNonOpChnNum(pAd, wdev, chn_list->op_class);
				setNonOpChnList(pAd,
								wdev,
								chn_list->non_op_ch_list,
								chn_list->op_class,
								chn_list->non_op_chn_num);
#endif /* CONFIG_MAP_SUPPORT */
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}
	return 0;
}

INT wapp_send_op_class_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
#ifndef MAP_6E_SUPPORT
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_op_class_info *op_class;

	NdisZeroMemory((void *)&event, sizeof(struct wapp_event));
	event.event_id = WAPP_OP_CLASS_RSP;
	event.ifindex = req->data.ifindex;
	op_class = &event.data.op_class;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex)) {
#ifdef CONFIG_MAP_SUPPORT
				op_class->num_of_op_class = map_set_op_class_info(pAd, wdev, op_class);
#endif /* CONFIG_MAP_SUPPORT */
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}
	return 0;
#else
	return 0;
#endif
}

#ifdef DPP_SUPPORT
void wext_send_dpp_frame_tx_status(PRTMP_ADAPTER pAd, struct wifi_dev *wdev,
				BOOLEAN tx_error, UINT16 seq_no)
{
	struct wapp_event event;
	struct wapp_dpp_frm_tx_status *req_data;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"wdev is null\n");
		return;
	}
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"%s: sending tx status to wapp status=%d\n", __func__, tx_error);
	if (wdev->if_dev) {
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		req_data = (struct wapp_dpp_frm_tx_status *)&event.data;
		req_data->tx_success = tx_error;
		req_data->seq_no = seq_no;
		event.event_id = WAPP_DPP_ACTION_FRAME_STATUS;

		RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM,
				OID_WAPP_EVENT, NULL, (PUCHAR)&event, sizeof(struct wapp_event));
	}
}

void cache_dpp_frame_rx_event(struct wifi_dev *wdev, const char *peer_mac_addr, UINT channel,
					const char *frm, UINT16 frm_len, BOOL is_gas, UINT32 frm_count)
{
	char *buf;
	struct wapp_event *event;
	struct wapp_dpp_action_frame *req_data;
	UINT16 buflen = 0;
	struct dpp_frame_list *dpp_frame;

	if (!wdev || !wdev->if_dev)
		return;
	buflen = sizeof(*event) + frm_len;
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	event = (struct wapp_event *)buf;
	event->ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	event->event_id = WAPP_DPP_ACTION_FRAME_RECEIVED;

	req_data = (struct wapp_dpp_action_frame *)&(event->data.frame);
	NdisCopyMemory(req_data->src, peer_mac_addr, 6);
	req_data->frm_len = frm_len;
	req_data->chan = channel;
	req_data->is_gas = is_gas;
	req_data->wapp_dpp_frame_id_no = frm_count;
	NdisCopyMemory(req_data->frm, frm, frm_len);
	MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s, storing event with frame id %d\n", __func__, req_data->wapp_dpp_frame_id_no);
	MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"%s, source mac address for dpp frame :%02x:%02x:%02x:%02x:%02x:%02x\n",
			 __func__,  PRINT_MAC(req_data->src));
	os_alloc_mem(NULL, (UCHAR **)&dpp_frame, sizeof(struct dpp_frame_list));
	NdisZeroMemory(dpp_frame, sizeof(struct dpp_frame_list));

	dpp_frame->dpp_frame_event = event;
	DlListAddTail(&wdev->dpp_frame_event_list, &dpp_frame->List);

}

void wext_send_dpp_frame_rx_event(struct wifi_dev *wdev, UINT32 frm_count)
{
	struct wapp_event *event;
	UINT16 buflen = 0;
	char *buf = NULL;

	if (!wdev || !wdev->if_dev)
		return;
	buflen = sizeof(*event);
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);
	event = (struct wapp_event *)buf;
	event->ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	event->event_id = WAPP_DPP_ACTION_FRAME_RECEIVED;
	event->data.wapp_dpp_frame_id_no = frm_count;
	MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s, sending event to wapp with frame id %d\n", __func__, event->data.wapp_dpp_frame_id_no);

	RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM,
					OID_WAPP_EVENT, NULL, (PUCHAR)buf, 100);
	os_free_mem(buf);

}

void wext_send_dpp_action_frame(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, const char *peer_mac_addr, UINT channel,
							  const char *frm, UINT16 frm_len, BOOL is_gas)
{
	/* MAKING EVENT TO BE STORED IN LIST WITH FRAME*/
	cache_dpp_frame_rx_event(wdev, peer_mac_addr, channel, frm, frm_len, is_gas, pAd->dpp_rx_frm_counter);
	/*MAKING EVENT TO BE SENT TO WAPP  */
	wext_send_dpp_frame_rx_event(wdev, pAd->dpp_rx_frm_counter);
	pAd->dpp_rx_frm_counter++;
}
#endif /* DPP_SUPPORT */

#ifdef MAP_R3
void wext_send_sta_info(PRTMP_ADAPTER pAd, struct wifi_dev *wdev,
				MAC_TABLE_ENTRY *pEntry)
{
	struct wapp_event event;
	struct wapp_sta_info *req_data;

	if (!wdev || !wdev->if_dev) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"wdev is null\n");
		return;
	}
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"sending sta info to wapp\n");
	event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	req_data = (struct wapp_sta_info *)&event.data;
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		NdisCopyMemory(req_data->src, pEntry->Addr, MAC_ADDR_LEN);
		req_data->SsidLen = pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen;
		NdisCopyMemory(req_data->ssid,
			pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid, (MAX_LEN_OF_SSID+1));
	} else {
		NdisCopyMemory(req_data->src, pEntry->wdev->if_addr, MAC_ADDR_LEN);
		if (pEntry->func_tb_idx < MAX_MULTI_STA) {
			req_data->SsidLen = pAd->StaCfg[pEntry->func_tb_idx].SsidLen;
			NdisCopyMemory(req_data->ssid,
				pAd->StaCfg[pEntry->func_tb_idx].Ssid, MAX_LEN_OF_SSID);
		}
	}
	/* if (!IS_AKM_DPP(pEntry->SecConfig.AKMMap)) */
	NdisCopyMemory(req_data->passphrase, pEntry->wdev->SecConfig.PSK, LEN_PSK);
	req_data->pmk_len = pEntry->SecConfig.pmk_len;
	NdisCopyMemory(req_data->pmk, pEntry->SecConfig.PMK, pEntry->SecConfig.pmk_len);
	req_data->ptk_len = pEntry->SecConfig.ptk_len;
	NdisCopyMemory(req_data->ptk, pEntry->SecConfig.PTK, pEntry->SecConfig.ptk_len);
	event.event_id = WAPP_STA_INFO;

	RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM,
					OID_WAPP_EVENT, NULL, (PUCHAR)&event, sizeof(struct wapp_event));
}

void wext_send_dpp_uri_info(PRTMP_ADAPTER pAd, struct wifi_dev *wdev,
				PWSC_CTRL pWscControl)
{
	struct wapp_event event;
	struct wapp_uri_info *req_data;

	if (!wdev || !wdev->if_dev) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"wdev is null\n");
		return;
	}
	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"sending URI info to wapp\n");
	event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	req_data = (struct wapp_uri_info *)&event.data;

	NdisCopyMemory(req_data->src_mac, pWscControl->WscPeerInfo.WscPeerMAC, MAC_ADDR_LEN);
	req_data->uri_len = (u8)pWscControl->rcvd_uri_len;
	NdisCopyMemory(req_data->rcvd_uri, pWscControl->rcvd_dpp_uri, req_data->uri_len);
	req_data->rcvd_uri[req_data->uri_len] = '\0';
	event.event_id = WAPP_R3_DPP_URI_INFO;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Sta Addr = %02x:%02x:%02x:%02x:%02x:%02x wdev:%s URI to wapp:%s with len:%u\n",
				PRINT_MAC(req_data->src_mac), RtmpOsGetNetDevName(wdev->if_dev),
				req_data->rcvd_uri, req_data->uri_len);
	RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM,
					OID_WAPP_EVENT, NULL, (PUCHAR)&event, sizeof(struct wapp_event));
}
#endif /* MAP_R3 */

INT wapp_send_bss_info_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_bss_info *bss_info;

	NdisZeroMemory(&event, sizeof(event));
	event.event_id = WAPP_BSS_INFO_RSP;
	event.ifindex = req->data.ifindex;
	bss_info = &event.data.bss_info;


	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex)) {
				if (i >= MAX_MBSSID_NUM(pAd)
					|| i >= MAX_BEACON_NUM
					|| wdev->wdev_type != WDEV_TYPE_AP) {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"wdev %d is not an AP\n", i);
					break;
				}
				bss_info->SsidLen = pAd->ApCfg.MBSSID[i].SsidLen;
				NdisMoveMemory(bss_info->ssid, pAd->ApCfg.MBSSID[i].Ssid, (MAX_LEN_OF_SSID+1));
				NdisMoveMemory(bss_info->bssid, wdev->bssid, MAC_ADDR_LEN);
				NdisMoveMemory(bss_info->if_addr, wdev->if_addr, MAC_ADDR_LEN);
#ifdef CONFIG_MAP_SUPPORT
				bss_info->map_role = wdev->MAPCfg.DevOwnRole;
				bss_info->auth_mode = pAd->ApCfg.MBSSID[i].wdev.SecConfig.AKMMap;
				bss_info->enc_type = pAd->ApCfg.MBSSID[i].wdev.SecConfig.PairwiseCipher;
#ifdef WSC_AP_SUPPORT
				bss_info->key_len = strlen(pAd->ApCfg.MBSSID[i].wdev.WscControl.WpaPsk);
				NdisMoveMemory(bss_info->key,
					pAd->ApCfg.MBSSID[i].wdev.WscControl.WpaPsk, bss_info->key_len);
#else
				bss_info->key_len = strlen(pAd->ApCfg.MBSSID[i].PSK);
				NdisMoveMemory(bss_info->key, pAd->ApCfg.MBSSID[i].PSK, bss_info->key_len);
#endif
				bss_info->hidden_ssid = pAd->ApCfg.MBSSID[i].bHideSsid;
#endif
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}

	return 0;
}


INT wapp_send_ap_metric_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_ap_metric *ap_metric;
	BSS_STRUCT *mbss;

	event.event_id = WAPP_AP_METRIC_RSP;
	event.ifindex = req->data.ifindex;
	ap_metric = &event.data.ap_metrics;
	NdisZeroMemory(ap_metric, sizeof(wdev_ap_metric));

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex)) {
				mbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
				NdisMoveMemory(ap_metric->bssid, wdev->bssid, MAC_ADDR_LEN);
				ap_metric->cu = get_channel_utilization(pAd, event.ifindex);
				NdisCopyMemory(ap_metric->ESPI_AC[ESPI_BE], mbss->ESPI_AC_BE, sizeof(mbss->ESPI_AC_BE));
				NdisCopyMemory(ap_metric->ESPI_AC[ESPI_BK], mbss->ESPI_AC_BK, sizeof(mbss->ESPI_AC_BK));
				NdisCopyMemory(ap_metric->ESPI_AC[ESPI_VO], mbss->ESPI_AC_VO, sizeof(mbss->ESPI_AC_VO));
				NdisCopyMemory(ap_metric->ESPI_AC[ESPI_VI], mbss->ESPI_AC_VI, sizeof(mbss->ESPI_AC_VI));
#ifdef MAP_R2
				/*TODO: take care of WHNAT*/
				ap_metric->ext_ap_metric.bc_rx =
					mbss->bcBytesRx == 0?0:((mbss->bcBytesRx/1024) == 0?1:mbss->bcBytesRx/1024);
				ap_metric->ext_ap_metric.bc_tx =
					mbss->bcBytesTx == 0?0:((mbss->bcBytesTx/1024) == 0?1:mbss->bcBytesTx/1024);
				ap_metric->ext_ap_metric.mc_rx =
					mbss->mcBytesRx == 0?0:((mbss->mcBytesRx/1024) == 0?1:mbss->mcBytesRx/1024);
				ap_metric->ext_ap_metric.mc_tx =
					mbss->mcBytesTx == 0?0:((mbss->mcBytesTx/1024) == 0?1:mbss->mcBytesTx/1024);
				ap_metric->ext_ap_metric.uc_rx =
					mbss->ucBytesRx == 0?0:((mbss->ucBytesRx/1024) == 0?1:mbss->ucBytesRx/1024);
				ap_metric->ext_ap_metric.uc_tx =
					mbss->ucBytesTx == 0?0:((mbss->ucBytesTx/1024) == 0?1:mbss->ucBytesTx/1024);
#endif
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}

	return 0;
}

#ifdef MAP_R2
INT wapp_send_radio_metric_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_radio_metric *radio_metric;
	int band_idx = 0;

	event.event_id = WAPP_RADIO_METRIC_RSP;
	event.ifindex = req->data.ifindex;
	radio_metric = &event.data.radio_metrics;
	NdisZeroMemory(radio_metric, sizeof(wdev_radio_metric));
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex)) {
				band_idx = HcGetBandByWdev(wdev);
				/*TODO: Raghav: NF logic in Harrier*/
				radio_metric->cu_noise =  0;/*pAd->Avg_NF[band_idx];*/
				radio_metric->cu_tx = (Get_My_Tx_AirTime(pAd, band_idx)*255)/ONE_SEC_2_US;
				radio_metric->cu_rx = (Get_My_Rx_AirTime(pAd, band_idx)*255)/ONE_SEC_2_US;
				radio_metric->cu_other = (Get_OBSS_AirTime(pAd, band_idx)*255)/ONE_SEC_2_US;
				radio_metric->edcca = (Get_EDCCA_Time(pAd, band_idx)*255)/ONE_SEC_2_US;;
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}

	return 0;
}
VOID Update_Mib_Bucket_for_map(RTMP_ADAPTER *pAd)
{
	UCHAR   i = 0, j = 0;
	UCHAR concurrent_bands = HcGetAmountOfBand(pAd);

	for (i = 0 ; i < concurrent_bands ; i++) {
		if (pAd->OneSecMibBucket.Enabled[i] == TRUE) {
			pAd->OneSecMibBucket.ChannelBusyTimeCcaNavTx[i] = 0;
			pAd->OneSecMibBucket.ChannelBusyTime[i] = 0;
			pAd->OneSecMibBucket.OBSSAirtime[i] = 0;
			pAd->OneSecMibBucket.MyTxAirtime[i] = 0;
			pAd->OneSecMibBucket.MyRxAirtime[i] = 0;
			pAd->OneSecMibBucket.EDCCAtime[i] =  0;
			pAd->OneSecMibBucket.MdrdyCount[i] = 0;
			pAd->OneSecMibBucket.PdCount[i] = 0;
			pAd->OneSecMibBucket.WtblRxTime[i] = 0;
			for (j = 0 ; j < 2 ; j++) {
				pAd->OneSecMibBucket.ChannelBusyTimeCcaNavTx[i] += pAd->MsMibBucket.ChannelBusyTimeCcaNavTx[i][j];
				pAd->OneSecMibBucket.ChannelBusyTime[i] += pAd->MsMibBucket.ChannelBusyTime[i][j];
				pAd->OneSecMibBucket.OBSSAirtime[i] += pAd->MsMibBucket.OBSSAirtime[i][j];
				pAd->OneSecMibBucket.MyTxAirtime[i] += pAd->MsMibBucket.MyTxAirtime[i][j];
				pAd->OneSecMibBucket.MyRxAirtime[i] += pAd->MsMibBucket.MyRxAirtime[i][j];
				pAd->OneSecMibBucket.EDCCAtime[i] += pAd->MsMibBucket.EDCCAtime[i][j];
				pAd->OneSecMibBucket.MdrdyCount[i] += pAd->MsMibBucket.MdrdyCount[i][j];
				pAd->OneSecMibBucket.PdCount[i] += pAd->MsMibBucket.PdCount[i][j];
				pAd->OneSecMibBucket.WtblRxTime[i] += pAd->MsMibBucket.WtblRxTime[i][j];
			}
		}
	}
}

#endif

INT wapp_send_ch_util_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	struct wapp_event event;
	event.event_id = WAPP_CH_UTIL_QUERY_RSP;
	event.ifindex = req->data.ifindex;
	event.data.ch_util = get_channel_utilization(pAd, event.ifindex);
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}

INT wapp_send_ap_config_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_ap_config *ap_conf;

	event.event_id = WAPP_AP_CONFIG_RSP;
	event.ifindex = req->data.ifindex;
	ap_conf = &event.data.ap_conf;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex)) {
#ifdef CONFIG_MAP_SUPPORT /* TODO: move to MAP */
				ap_conf->sta_report_on_cop = wdev->MAPCfg.bUnAssocStaLinkMetricRptOpBss;
				ap_conf->sta_report_not_cop = wdev->MAPCfg.bUnAssocStaLinkMetricRptNonOpBss;
				ap_conf->rssi_steer = wdev->MAPCfg.bAgentInitRssiSteering;
#endif /* CONFIG_MAP_SUPPORT */
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}
	return 0;
}
#ifdef WPS_UNCONFIG_FEATURE_SUPPORT
INT wapp_send_wps_config(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	IN  PWSC_CREDENTIAL pCredential)
{

	struct wapp_event event;
	u8 index = 0;
	u8 pf1_num = 0, pf2_num = 0;

	pf1_num = multi_profile_get_pf1_num(ad);
	pf2_num = multi_profile_get_pf2_num(ad);

	if (wdev == NULL) {
		MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error :: Wapp_Send_wps_config --> wdev NULL");
		return 0;
	}
	if (wdev->if_dev) {
		event.event_id = WAPP_CONFIG_WPS_EVENT;
		event.ifindex = wdev->if_dev->ifindex;

		NdisZeroMemory(event.data.wps_conf_info.SSID, MAX_LEN_OF_SSID);
		NdisMoveMemory(event.data.wps_conf_info.SSID, pCredential->SSID.Ssid, pCredential->SSID.SsidLength);


		NdisMoveMemory(event.data.wps_conf_info.MacAddr, pCredential->MacAddr, MAC_ADDR_LEN);

		RTMPZeroMemory(event.data.wps_conf_info.Key, 64);
		RTMPMoveMemory(event.data.wps_conf_info.Key, pCredential->Key, pCredential->KeyLength);

		event.data.wps_conf_info.KeyLength = pCredential->KeyLength;
		event.data.wps_conf_info.AuthType = pCredential->AuthType;
		event.data.wps_conf_info.EncrType = pCredential->EncrType;
		event.data.wps_conf_info.bss_role = pCredential->bss_role;
		event.data.wps_conf_info.channel  = wdev->channel;

		if (pf1_num && pf2_num) {
			if (wdev->wdev_idx >= pf1_num)
				index = wdev->wdev_idx - pf1_num;
			else
				index = wdev->wdev_idx;
		} else {
			index = wdev->wdev_idx;
		}

		event.data.wps_conf_info.index = index;
		MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"SSID = %s, psk = %s, len = %d", event.data.wps_conf_info.SSID, event.data.wps_conf_info.Key,
			event.data.wps_conf_info.KeyLength);
		MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Auth = %d, EncyType = %d, Bss_role = %d",
			event.data.wps_conf_info.AuthType, event.data.wps_conf_info.EncrType,
			event.data.wps_conf_info.bss_role);
		MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MAC :: 02x%x:02x%x:02x%x:02x%x:02x%x:02x%x",
			event.data.wps_conf_info.MacAddr[0], event.data.wps_conf_info.MacAddr[1],
			event.data.wps_conf_info.MacAddr[2], event.data.wps_conf_info.MacAddr[3],
			event.data.wps_conf_info.MacAddr[4], event.data.wps_conf_info.MacAddr[5]);
		MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "channel == %d, index = %d\n", wdev->channel, index);

		wext_send_wapp_qry_rsp(ad->net_dev, &event);
	}
	return 0;
}
#endif
INT wapp_send_bss_state_change(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	UINT8 bss_state)
{
	struct wapp_event event;

	if (wdev->if_dev) {
		event.event_id = WAPP_BSS_STATE_CHANGE;
		event.ifindex = wdev->if_dev->ifindex;
		event.data.bss_state_info.interface_index = event.ifindex;
		event.data.bss_state_info.bss_state = bss_state;
		wext_send_wapp_qry_rsp(ad->net_dev, &event);
	}
	return 0;
}

INT wapp_send_ch_change_rsp(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UINT8 ControlChannel)
{
	struct wapp_event event;

	if (wdev->if_dev) {
		event.event_id = WAPP_CH_CHANGE;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		event.data.ch_change_info.interface_index = event.ifindex;
		event.data.ch_change_info.new_ch = (u_int8_t)ControlChannel;
		event.data.ch_change_info.op_class =
			get_regulatory_class(pAd, ControlChannel, wdev->PhyMode, wdev);
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
	return 0;
}
#ifdef MAP_R3
INT wapp_send_ch_change_rsp_map_r3(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UINT8 ControlChannel)
{
	struct wapp_event event;

	if (wdev->if_dev) {
		event.event_id = WAPP_CH_CHANGE_R3;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		event.data.ch_change_info.interface_index = event.ifindex;
		event.data.ch_change_info.new_ch = (u_int8_t)ControlChannel;
		event.data.ch_change_info.op_class =																		get_regulatory_class(pAd, ControlChannel, wdev->PhyMode, wdev);
			wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
	return 0;
}
#endif
/*
*	INT wapp_send_txpower_change_rsp(
*	PRTMP_ADAPTER pAd,
*	)
*	{
*		struct wapp_event event;
*		event.event_id = WAPP_TX_POWER_CHANGE;
*		event.ifindex = ;
*		event.data.txpwr_change_info.interface_index =event.ifindex;
*		event.data.txpwr_change_info.new_tx_pwr =;
*		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
*		return 0;
*	}
*/

INT wapp_send_apcli_association_change(
	UINT8 apcli_assoc_state,
	PRTMP_ADAPTER pAd,
	PSTA_ADMIN_CONFIG pApCliEntry)
{
	struct wapp_event event;

	if (pApCliEntry->wdev.if_dev) {
		event.event_id = WAPP_APCLI_ASSOC_STATE_CHANGE;
		event.ifindex = pApCliEntry->wdev.if_dev->ifindex;
		event.data.apcli_association_info.interface_index = event.ifindex;
		event.data.apcli_association_info.apcli_assoc_state = apcli_assoc_state;
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd))
			event.data.apcli_association_info.PeerMAPEnable = pApCliEntry->PeerMAPEnable;
#endif
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
	return 0;
}

#ifdef CONVERTER_MODE_SWITCH_SUPPORT
INT wapp_send_apcli_association_change_vendor10(
	UINT8 apcli_assoc_state,
	PRTMP_ADAPTER pAd,
	PSTA_ADMIN_CONFIG pApCliEntry)
{
	struct wapp_event event;

	event.event_id = WAPP_APCLI_ASSOC_STATE_CHANGE_VENDOR10;
	event.ifindex = pApCliEntry->wdev.if_dev->ifindex;
	event.data.apcli_association_info.interface_index = event.ifindex;
	event.data.apcli_association_info.apcli_assoc_state = apcli_assoc_state;
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */

INT wapp_send_bssload_crossing(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	UCHAR bssload_high_thrd,
	UCHAR bssload_low_thrd,
	UCHAR bssload)
{
	struct wapp_event event;

	if (wdev->if_dev) {
		event.event_id = WAPP_BSSLOAD_CROSSING;
		event.ifindex = wdev->if_dev->ifindex;
		event.data.bssload_crossing_info.interface_index = event.ifindex;
		event.data.bssload_crossing_info.bssload = bssload;
		event.data.bssload_crossing_info.bssload_high_thrd = bssload_high_thrd;
		event.data.bssload_crossing_info.bssload_low_thrd = bssload_low_thrd;
		wext_send_wapp_qry_rsp(ad->net_dev, &event);
	}
	return 0;
}

INT wapp_send_sta_connect_rejected(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	UCHAR *sta_mac_addr,
	UCHAR *bssid,
	UINT8 connect_stage,
	UINT16 reason,
	USHORT status_code,
	USHORT reason_code)
{
	struct wapp_event event;

	if (wdev->if_dev) {
		event.event_id = WAPP_STA_CNNCT_REJ;
		event.ifindex = wdev->if_dev->ifindex;
		event.data.sta_cnnct_rej_info.interface_index = event.ifindex;
		os_move_mem(event.data.sta_cnnct_rej_info.bssid, bssid, MAC_ADDR_LEN);
		os_move_mem(event.data.sta_cnnct_rej_info.sta_mac, sta_mac_addr, MAC_ADDR_LEN);
		event.data.sta_cnnct_rej_info.cnnct_fail.connect_stage = connect_stage;
		event.data.sta_cnnct_rej_info.cnnct_fail.reason = reason;
#ifdef MAP_R2
		event.data.sta_cnnct_rej_info.assoc_status_code = status_code;
		event.data.sta_cnnct_rej_info.assoc_reason_code = reason_code;
		/*printk("### %d %s status_code = %d\n", __LINE__, __func__, status_code);*/
#endif
		wext_send_wapp_qry_rsp(ad->net_dev, &event);
	}
	return TRUE;
}

INT wapp_bss_start(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i, j;
	struct wifi_dev *wdev, *wdev_active;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
				for (j = 0; j < WDEV_NUM_MAX; j++) {
					wdev_active = pAd->wdev_list[j];
					if (wdev_active && HcIsRadioAcq(wdev_active) &&
						(HcGetBandByWdev(wdev) == HcGetBandByWdev(wdev_active))) {
						wdev->channel = wdev_active->channel;
						break;
					}
				}
#ifdef CONFIG_MAP_SUPPORT
				if (pAd->ApCfg.MBSSID[wdev->func_idx].is_bss_stop_by_map)
					pAd->ApCfg.MBSSID[wdev->func_idx].is_bss_stop_by_map = FALSE;
#endif
				APStartUpByBss(pAd, &pAd->ApCfg.MBSSID[wdev->func_idx]);
				wdev_if_up_down(pAd, wdev, TRUE);
				break;
			}
		}
	}
	return 0;
}

INT wapp_bss_stop(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
#ifdef CONFIG_MAP_SUPPORT
	BSS_STRUCT *pMbss;
#endif
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"%s():req->data.ifindex = %d\n", __func__, req->data.ifindex);
#ifdef CONFIG_MAP_SUPPORT
				if (IS_MAP_ENABLE(pAd)) {

					pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
					pMbss->wdev.avoid_apcli_linkdown = TRUE;
				}
#endif
#ifdef CONFIG_MAP_SUPPORT
				if (!IS_MAP_ENABLE(pAd))
#endif
					wdev_if_up_down(pAd, wdev, FALSE);
#ifdef CONFIG_MAP_SUPPORT
				pAd->ApCfg.MBSSID[wdev->func_idx].is_bss_stop_by_map = TRUE;
#endif
				APStopByBss(pAd, &pAd->ApCfg.MBSSID[wdev->func_idx]);
#ifdef CONFIG_MAP_SUPPORT
				if (IS_MAP_ENABLE(pAd))
					pMbss->wdev.avoid_apcli_linkdown = FALSE;
#endif
			}
		}
	}
	return 0;
}

INT wapp_bss_load_thrd_set(
	struct _RTMP_ADAPTER *ad,
	struct wapp_req *req)
{
	int i = 0;
	UINT8 band_id;
	UINT8 high_thrd;
	UINT8 low_thrd;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (ad->wdev_list[i] != NULL) {
			wdev = ad->wdev_list[i];
			if (wdev->if_dev && wdev->if_dev->ifindex == req->data.ifindex) {
				band_id = HcGetBandByWdev(wdev);
				high_thrd = req->data.bssload_thrd.high_bssload_thrd;
				low_thrd = req->data.bssload_thrd.low_bssload_thrd;
				ad->bss_load_info.high_thrd[band_id] = high_thrd;
				ad->bss_load_info.low_thrd[band_id] = low_thrd;
				return 0;
			}
		}
	}

	MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"wdev not found\n");
	return 1;
}

/*
 *==========================================================================
 *	Description:
 *		Periodically check the BSS load. Once the load status changes into an abnormal situation (above the
 *		high threshold/below the low threshold), the WAPP event will be sent. Notice that the event won't
 *		be sent if the load status stays in the same abnormal situation.
 *==========================================================================
 */
VOID wapp_bss_load_check(
	struct _RTMP_ADAPTER *ad)
{
	int i;
	UINT8 band_id;
	UINT8 new_load;
	UINT8 new_status;
	UINT8 current_status;
	UINT8 high_thrd;
	UINT8 low_thrd;
	ULONG up_time;
	struct wifi_dev *wdev;

	NdisGetSystemUpTime(&up_time);
#ifdef AP_QLOAD_SUPPORT
	QBSS_LoadUpdate(ad, up_time);
#endif /* AP_QLOAD_SUPPORT */
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = ad->wdev_list[i];

		if ((wdev != NULL) && (wdev->wdev_type == WDEV_TYPE_AP)) {
			if (wdev->if_dev == NULL) {
				MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "!!!!  wdev->ifdev = NULL\n");
				continue;
			}
			band_id = HcGetBandByWdev(wdev);
			high_thrd = ad->bss_load_info.high_thrd[band_id];
			low_thrd = ad->bss_load_info.low_thrd[band_id];
			current_status = ad->bss_load_info.current_status[band_id];
			new_load = get_channel_utilization(ad, wdev->if_dev->ifindex);

			if (new_load >= high_thrd)
				new_status = WAPP_BSSLOAD_HIGH;
			else if (new_load <= low_thrd)
				new_status = WAPP_BSSLOAD_LOW;
			else
				new_status = WAPP_BSSLOAD_NORMAL;

			/*The status changes & the new status is an abnormal situation*/
			if (new_status != current_status && new_status != WAPP_BSSLOAD_NORMAL)
				wapp_send_bssload_crossing(ad, wdev, high_thrd, low_thrd, new_load);

			ad->bss_load_info.current_load[band_id] = new_load;
			ad->bss_load_info.current_status[band_id] = new_status;
		}
	}
}

INT wapp_config_ap_setting(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
#ifdef CONFIG_MAP_SUPPORT
				wdev->MAPCfg.bUnAssocStaLinkMetricRptOpBss = req->data.ap_conf.sta_report_on_cop;
				wdev->MAPCfg.bUnAssocStaLinkMetricRptNonOpBss = req->data.ap_conf.sta_report_not_cop;
				wdev->MAPCfg.bAgentInitRssiSteering = req->data.ap_conf.rssi_steer;
#endif
			}
		}
	}
	return 0;
}

/* set Tx Power Perventage */
INT wapp_set_tx_power_prctg(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	UINT8 prctg = (UINT8) req->data.value;
	UCHAR Band_Idx = 0;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
				Band_Idx = HcGetBandByWdev(wdev);
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(%s) ifindex = %u, prctg = %u\n",
						__func__, req->data.ifindex, prctg);
				if (prctg > 0) {
					TxPowerPercentCtrl(pAd, TRUE, Band_Idx);
					TxPowerDropCtrl(pAd, prctg, Band_Idx);
					chip_set_mgmt_pkt_txpwr(pAd, wdev, prctg);
				} else {
					TxPowerPercentCtrl(pAd, FALSE, Band_Idx);
				}
			}
		}
	}
	return 0;
}

INT wapp_set_steer_policy(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
#ifdef CONFIG_MAP_SUPPORT
				pAd->ApCfg.SteerPolicy.steer_policy = req->data.str_policy.steer_policy;
				pAd->ApCfg.SteerPolicy.cu_thr = req->data.str_policy.cu_thr;
				pAd->ApCfg.SteerPolicy.rcpi_thr = req->data.str_policy.rcpi_thr;
#endif
			}
		}
	}
	return 0;
}

INT wapp_send_bssload_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;

	event.event_id = WAPP_BSSLOAD_RSP;
	event.ifindex = req->data.ifindex;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex)) {
				event.data.bssload_info.sta_cnt = MacTableAssocStaNumGet(pAd);
				event.data.bssload_info.ch_util = get_channel_utilization(pAd, event.ifindex);
				event.data.bssload_info.AvalAdmCap = (0x7a12); /* 0x7a12 * 32us = 1 second */
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}

	return 0;
}

INT wapp_send_he_cap_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{

	return 0;
}

INT wapp_send_apcli_rssi_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	struct wapp_event event;
	PMAC_TABLE_ENTRY mac_entry;
	wapp_apcli_association_info *apcli_info;

	event.event_id = WAPP_APCLI_RSSI_RSP;
	event.ifindex = req->data.ifindex;
	apcli_info = &event.data.apcli_association_info;

	mac_entry = MacTableLookup(pAd, req->data.mac_addr);
	if (mac_entry && mac_entry->wdev->if_dev && IS_ENTRY_PEER_AP(mac_entry)) {
		if (req->data.ifindex == RtmpOsGetNetIfIndex(mac_entry->wdev->if_dev)) {
			apcli_info->rssi = RTMPAvgRssi(pAd, &mac_entry->RssiSample);
			wext_send_wapp_qry_rsp(pAd->net_dev, &event);
		}
	}

	return 0;
}

INT wapp_send_sta_rssi_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	struct wapp_event event;
	PMAC_TABLE_ENTRY mac_entry;
	wapp_client_info *cli_info;

	event.event_id = WAPP_STA_RSSI_RSP;
	event.ifindex = req->data.ifindex;
	cli_info = &event.data.cli_info;

	mac_entry = MacTableLookup(pAd, req->data.mac_addr);
	if (mac_entry && mac_entry->wdev->if_dev && IS_ENTRY_CLIENT(mac_entry)) {
		if (req->data.ifindex == RtmpOsGetNetIfIndex(mac_entry->wdev->if_dev)) {
			COPY_MAC_ADDR(cli_info->mac_addr, mac_entry->Addr);
			cli_info->uplink_rssi = RTMPAvgRssi(pAd, &mac_entry->RssiSample);
			wext_send_wapp_qry_rsp(pAd->net_dev, &event);
		}
	}

	return 0;
}

/* set SRG Bitmap  */
INT wapp_set_srg_bitmap(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
#ifdef MAP_R3
	INT i;
	struct wifi_dev *wdev;
	UCHAR Band_Idx = 0;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
				Band_Idx = HcGetBandByWdev(wdev);
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"(%s) ifindex = %u\n", __func__, req->data.ifindex);
				SrMeshSrgBitMapControl(pAd, TRUE, (PUINT_8)&req->data.bm_info);
			}
		}
	}
#endif
	return 0;
}

/* Update Topology  */
INT wapp_srg_topology_update(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
#ifdef MAP_R3
	INT i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"(%s) ifindex = %u Band:%u\n",
					__func__, req->data.ifindex, req->data.band_index);
				SrMeshTopologyUpdate(pAd, (PUINT_8)&req->data.topology_update, req->data.band_index);
			}
		}
	}
#endif
	return 0;
}

/* set SRG Uplink Traffic Status  */
INT wapp_set_srg_uplink_traffic_status(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
#ifdef MAP_R3
	INT i;
	struct wifi_dev *wdev;
	UINT8 status = (UINT8) req->data.value;
	UCHAR Band_Idx = 0;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
				Band_Idx = HcGetBandByWdev(wdev);
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"(%s) ifindex = %u, status = %u\n",
					__func__, req->data.ifindex, status);
				SrSetUplinkTrafficStatus(pAd, Band_Idx, status);
			}
		}
	}
#endif
	return 0;
}

/* set SR Mesh link Sta Threshold  */
INT wapp_set_meshsrlinkstath(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
#ifdef MAP_R3
	SRMeshLinkSTAThreshold(pAd, req->data.sta_threshold_update.wdev_idx, req->data.sta_threshold_update.rssi);
#endif
	return 0;
}

/* set SR Mesh BH SRG BitMap  */
INT wapp_set_meshbhsrgbitmap(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
#ifdef MAP_R3
	INT i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"(%s) ifindex = %u\n", __func__, req->data.ifindex);
				SrMultiAPBhMeshSrgBitmap(pAd, TRUE, (PUINT_8)&req->data.bm_info);
			}
		}
	}
#endif
	return 0;
}

/* set SR Mesh FH SRG BitMap  */
INT wapp_set_meshfhsrgbitmap(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
#ifdef MAP_R3
	INT i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"(%s) ifindex = %u\n", __func__, req->data.ifindex);
				SrMultiAPFhMeshSrgBitmap(pAd, TRUE, (PUINT_8)&req->data.bm_info);
			}
		}
	}
#endif
	return 0;
}

/* set SR Mesh BH DL Thresh  */
INT wapp_set_meshbhobsspdth(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
#ifdef MAP_R3
	INT i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"(%s) ifindex = %u\n", __func__, req->data.ifindex);
				SrBHDownMeshSRThreshold(pAd, (INT_8)&req->data.value);
			}
		}
	}
#endif
	return 0;
}

/* set SR Mesh FH DL Thresh  */
INT wapp_set_meshfhobsspdth(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
#ifdef MAP_R3
	INT i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"(%s) ifindex = %u\n", __func__, req->data.ifindex);
				SrFHDownMeshSRThreshold(pAd, (INT_8)&req->data.value);
			}
		}
	}
#endif
	return 0;
}

/* set SR Link forbid SR  */
INT wapp_set_linkforbidsr(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
#ifdef MAP_R3
	INT i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"(%s) ifindex = %u\n", __func__, req->data.ifindex);
				SrMeshForbidSrBssid(pAd, (UINT_8)&req->data.value);
			}
		}
	}
#endif
	return 0;
}

/* set SRG STA Mode Report  */
INT wapp_send_sta_mode_rpt_cmd(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	UINT8 status = (UINT8) req->data.value;
	UCHAR Band_Idx = 0;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)) {
				Band_Idx = HcGetBandByWdev(wdev);
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"(%s) ifindex = %u, status = %u\n",
					__func__, req->data.ifindex, status);
				SrSetMeshRemoteStaModeRpt(pAd, Band_Idx, status);
			}
		}
	}

	return 0;
}

INT wapp_send_wsc_scan_complete_notification(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	struct wapp_event event;
	WSC_CTRL *pWscControl = &wdev->WscControl;

	if (wdev->if_dev) {
		event.len = 0;
		event.event_id = WAPP_WSC_SCAN_COMP_NOTIF;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		event.data.wsc_scan_info.bss_count =
			pWscControl->WscPBCBssCount;
		if (pWscControl->WscPBCBssCount == 1) {
			NdisCopyMemory(event.data.wsc_scan_info.Uuid,
				pWscControl->WscPeerUuid, sizeof(pWscControl->WscPeerUuid));
		}
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
	return 0;
}

INT wapp_send_wsc_eapol_start_notification(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	struct wapp_event event;

	if (wdev->if_dev) {
		event.len = 0;
		event.event_id = WAPP_WSC_EAPOL_START_NOTIF;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
	return 0;
}

INT wapp_send_wsc_eapol_complete_notif(
	PRTMP_ADAPTER pAd,
	PWSC_CTRL pWscControl)
{
	struct wapp_event event;
	struct wapp_bhsta_info *bsta_info = &event.data.bhsta_info;
	struct wifi_dev *wdev = (struct wifi_dev *)pWscControl->wdev;

	if (wdev->if_dev) {
		event.event_id = WAPP_WSC_EAPOL_COMPLETE_NOTIF;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		bsta_info->peer_map_enable = 0;

		if (IS_MAP_ENABLE(pAd) &&
			(pWscControl->RegData.PeerInfo.map_DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA))) {
			bsta_info->peer_map_enable = 1;
		}

		COPY_MAC_ADDR(bsta_info->connected_bssid, wdev->bssid);
		COPY_MAC_ADDR(bsta_info->mac_addr, pWscControl->EntryAddr);
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
	return 0;
}
#ifdef CONFIG_MAP_SUPPORT
INT wapp_send_scan_complete_notification(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	struct wapp_event event;

	if (wdev->if_dev) {
		event.len = 0;
		event.event_id = WAPP_SCAN_COMPLETE_NOTIF;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
	return 0;
}
#endif /* CONFIG_MAP_SUPPORT */

#ifdef CONFIG_CPE_SUPPORT
INT lppe_send_scan_complete_notification(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	struct wapp_event event;
	UINT buflen = sizeof(struct wapp_event);

	if (wdev->if_dev) {
		event.len = 0;
		event.event_id = LPPE_SCAN_COMPLETE_NOTIF;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		event.len = buflen - sizeof(event.len) - sizeof(event.event_id);
		NdisCopyMemory(event.data.ifname, RtmpOsGetNetDevName(wdev->if_dev), IFNAMSIZ);

		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM,
			OID_LPPE_EVENT, NULL, (PUCHAR)&event, sizeof(struct wapp_event));
	}
	return 0;
}
#endif

#ifdef A4_CONN
/* client disaccos */
INT wapp_send_a4_entry_missing(
	PRTMP_ADAPTER pAd,
	UINT32 ifindex,
	UCHAR *ip)
{
	struct wapp_event event;
	UCHAR *a4_missing_entry_ip;

	event.event_id = WAPP_A4_ENTRY_MISSING_NOTIF;
	event.ifindex = ifindex;
	a4_missing_entry_ip = (UCHAR *)&event.data.a4_missing_entry_ip;

	NdisCopyMemory(a4_missing_entry_ip, ip, 4);
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);

	return 0;
}
#endif
/* client disaccos */
INT wapp_send_radar_detect_notif(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	unsigned char channel,
	unsigned char bw,
	unsigned char ch_status
	)
{
	struct wapp_event event;

	if (wdev->if_dev) {
		event.event_id = WAPP_RADAR_DETECT_NOTIF;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		event.data.radar_notif.channel = channel;
		event.data.radar_notif.status = ch_status;
		event.data.radar_notif.bw = bw;
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
	return 0;
}

#ifdef WIFI_MD_COEX_SUPPORT
INT wapp_send_lte_safe_chn_event(
	PRTMP_ADAPTER pAd, UINT32 *safe_chn_bitmask)
{
	struct wapp_event event;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"safe_chn_bitmask:%x-%x-%x\n", safe_chn_bitmask[0],
		safe_chn_bitmask[1], safe_chn_bitmask[2]);

	event.event_id = WAPP_UNSAFE_CHANNEL_EVENT;
	NdisCopyMemory(event.data.unsafe_ch_notif.ch_bitmap, safe_chn_bitmask,
		sizeof(UINT32) * 3);
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}

INT wapp_send_band_status_event(
	PRTMP_ADAPTER pAd, struct wifi_dev *wdev, BOOLEAN status)
{
	struct wapp_event event;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"update band status[%d]", status);
	if (wdev->if_dev) {
		event.event_id = WAPP_BAND_STATUS_CHANGE_EVENT;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		event.data.band_status.status = status;
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
	return 0;
}
#endif

#ifdef CONFIG_MAP_SUPPORT
#ifdef MAP_R3
INT wapp_send_trigger_reconfig(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	struct wapp_event event;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	if (wdev->if_dev) {
		event.event_id = WAPP_R3_RECONFIG_TRIGGER;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
	return 0;
}
#endif
#endif

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
INT wapp_send_uplink_traffic_event(
	PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UINT8 u1UlStatus)
{

	struct wapp_event event;
	if (wdev->if_dev) {
		event.event_id = WAPP_UPLINK_TRAFFIC_EVENT;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		event.data.mesh_sr_info.ul_traffic_status = u1UlStatus;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s: uplink traffic event, ifindex:%u status:%u\n",
				__func__, event.ifindex, u1UlStatus);

		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
	return 0;
}

INT wapp_send_sr_self_srg_bm_event(
	IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev, IN PUINT8 prSrgBitmap)
{
	struct wapp_event event;
	if (wdev->if_dev) {
		event.event_id = WAPP_SELF_SRG_BITMAP_EVENT;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		NdisCopyMemory(&(event.data.mesh_sr_info.bm_info), prSrgBitmap,
				sizeof(struct wapp_srg_bitmap));

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s: ifindex:%u Color:[63_32][%u]-[31_0][%u] PBssid::[63_32][%u]-[31_0][%u]\n",
				__func__, event.ifindex,
				event.data.mesh_sr_info.bm_info.color_63_32,
				event.data.mesh_sr_info.bm_info.color_31_0,
				event.data.mesh_sr_info.bm_info.bssid_63_32,
				event.data.mesh_sr_info.bm_info.bssid_31_0);

		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
	return 0;
}
#endif /* defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) */

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
INT wapp_send_sta_mode_rpt_event(
	PRTMP_ADAPTER pAd, struct wifi_dev *wdev, BOOLEAN IsStaAllHe)
{
	struct wapp_event event;
	if (wdev->if_dev) {
		event.event_id = WAPP_STA_MODE_RPT_EVENT;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		event.data.mesh_sr_info.ul_traffic_status = IsStaAllHe;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s: STA Mode Rpt event, ifindex:%u IsStaAllHe:%u\n",
				__func__, event.ifindex, IsStaAllHe);

		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
	return 0;
}
#endif /* defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3) */

VOID RTMPIoctlGetScanResults(
	IN	PRTMP_ADAPTER	pAdapter, struct wapp_req *req)
{
	struct wapp_event *event;
	INT last_bss_cnt = 0;
	RTMP_STRING *msg;
	INT		i = 0;
	/*INT			WaitCnt = 0;*/
	UINT32		bss_start_idx;
	BSS_ENTRY *bss;
	struct scan_bss_info *pBss;
	wdev_ht_cap *ht_cap;
	wdev_vht_cap *vht_cap;
	wdev_he_cap *he_cap;
	char *mcsptr = NULL;
	UINT8 he_ch_width = HE_BW_80;
	UINT16	l;
	INT custom_event_length;
	UINT32 TotalLen;
	INT count = 0, max_bss;
	BSS_TABLE *ScanTab = NULL;
	struct wifi_dev *wdev = NULL;

	for (l = 0; l < WDEV_NUM_MAX; l++) {
		if (pAdapter->wdev_list[l] != NULL) {
			wdev = pAdapter->wdev_list[l];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex))
				break;
		}
	}

	if (wdev == NULL)
		return;

	ScanTab = get_scan_tab_by_wdev(pAdapter, wdev);
#ifndef IWEVCUSTOM_PAYLOD_MAX_LEN
#define IWEVCUSTOM_PAYLOD_MAX_LEN 220
#endif
	custom_event_length = IWEVCUSTOM_PAYLOD_MAX_LEN;

	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "RTMPIoctlGetSiteSurvey - enter\n");
	/*
	 * At the moment there is a single scan tab across the pAd
	 * that means if scan id or if_index has changed, it doesn't
	 * make sense of continuing the last get scan command.
	 */
	if ((pAdapter->last_scan_req.scan_id != req->data.value) ||
	    (pAdapter->last_scan_req.if_index != req->data.ifindex)) {
		pAdapter->last_scan_req.scan_id = req->data.value;
		pAdapter->last_scan_req.if_index = req->data.ifindex;
		/* Storing the first bss nr in the bs_nr */
		pAdapter->last_scan_req.bss_nr = ScanTab->BssNr;
	} else
		last_bss_cnt = pAdapter->last_scan_req.last_bss_cnt;

	if (ScanTab->BssNr == 0) {
		MTWF_DBG(pAdapter, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"no bss present in scan tab\n");
	}
	max_bss = custom_event_length / sizeof(struct scan_bss_info);

	bss_start_idx = last_bss_cnt;

	TotalLen = (sizeof(CHAR) * 4) + ((sizeof(struct scan_bss_info)) * max_bss) + (sizeof(UINT32) * 2);
	if ((max_bss > 0) && (TotalLen > custom_event_length))
		max_bss--;

	if (max_bss == 0) {
		MTWF_DBG(pAdapter, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"bss size is too big for wireless send event\n");
		return;
	}
	TotalLen = (sizeof(CHAR) * 4) + ((sizeof(struct scan_bss_info)) * max_bss) + (sizeof(UINT32) * 2);
	os_alloc_mem(NULL, (PUCHAR *)&msg, TotalLen);

	if (msg == NULL) {
		MTWF_DBG(pAdapter, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"RTMPIoctlGetSiteSurvey - msg memory alloc fail.\n");
		return;
	}

	NdisZeroMemory(msg, TotalLen);

	event = (struct wapp_event *)msg;
	event->event_id = WAPP_SCAN_RESULT_RSP;
	event->ifindex = req->data.ifindex;

	if (ScanTab->BssNr == 0) {
		/* Send the event as this is possible scenario during in DFS channe */
		goto send_event;
	}

	if (bss_start_idx > (ScanTab->BssNr - 1)) {
		event->data.scan_info.bss_count = 0;
		goto send_event;
	}

	for (i = bss_start_idx; i < ScanTab->BssNr && count < max_bss; i++) {
		bss = &ScanTab->BssEntry[i];
		pBss = &event->data.scan_info.bss[count];

		ht_cap = &pBss->ht_cap;
		vht_cap = &pBss->vht_cap;
		he_cap = &pBss->he_cap;
		if (bss->Channel == 0) {
			i++;
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"break from here bss Channel is 0 i %d \n", i);
			break;
		}
		pBss->Channel = bss->Channel;
		pBss->CentralChannel = bss->CentralChannel;
		pBss->Rssi = bss->Rssi;
		ht_cap->tx_stream = 0; /* TODO */
		ht_cap->rx_stream = 0; /* TODO */
		ht_cap->sgi_20 = bss->HtCapability.HtCapInfo.ShortGIfor20;
		ht_cap->sgi_40 = bss->HtCapability.HtCapInfo.ShortGIfor40;
		ht_cap->ht_40 = bss->HtCapability.HtCapInfo.ChannelWidth;

		NdisMoveMemory(vht_cap->sup_tx_mcs,
						&bss->vht_cap_ie.mcs_set.tx_mcs_map,
						sizeof(vht_cap->sup_tx_mcs));
		NdisMoveMemory(vht_cap->sup_rx_mcs,
						&bss->vht_cap_ie.mcs_set.rx_mcs_map,
						sizeof(vht_cap->sup_rx_mcs));
		if (bss->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss4 != VHT_MCS_CAP_NA)
			vht_cap->rx_stream = 4;
		else if (bss->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss3 != VHT_MCS_CAP_NA)
			vht_cap->rx_stream = 3;
		else if (bss->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 != VHT_MCS_CAP_NA)
			vht_cap->rx_stream = 2;
		else
			vht_cap->rx_stream = 1;

		if (bss->vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss4 != VHT_MCS_CAP_NA)
			vht_cap->tx_stream = 4;
		else if (bss->vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss3 != VHT_MCS_CAP_NA)
			vht_cap->tx_stream = 3;
		else if (bss->vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss2 != VHT_MCS_CAP_NA)
			vht_cap->tx_stream = 2;
		else
			vht_cap->tx_stream = 1;
		vht_cap->sgi_80 = bss->vht_cap_ie.vht_cap.sgi_80M;
		vht_cap->sgi_160 = bss->vht_cap_ie.vht_cap.sgi_160M;
		vht_cap->vht_160 = bss->vht_op_ie.vht_op_info.ch_width;
		vht_cap->vht_8080 = 0; /* TODO */
		vht_cap->su_bf = bss->vht_cap_ie.vht_cap.bfee_cap_su;
		vht_cap->mu_bf = bss->vht_cap_ie.vht_cap.bfee_cap_mu;
#ifdef DOT11_HE_AX
		he_ch_width = peer_max_bw_cap(GET_DOT11AX_CH_WIDTH(bss->he_caps.phy_cap.phy_capinfo_1));
		mcsptr = he_cap->he_mcs;
		NdisCopyMemory(mcsptr, &bss->he_caps.txrx_mcs_nss, sizeof(struct he_txrx_mcs_nss));
		if (he_ch_width == HE_BW_160) {
			he_cap->he_160 = 1;
			mcsptr += sizeof(struct he_txrx_mcs_nss);
			NdisCopyMemory(mcsptr, &bss->he_mcs_nss_160, sizeof(struct he_txrx_mcs_nss));
		} else
			he_cap->he_160 = 0;
#endif
		pBss->MinSNR = bss->MinSNR;
		pBss->Privacy = bss->Privacy;
		pBss->SsidLen = bss->SsidLen;
		NdisCopyMemory(pBss->Ssid, bss->Ssid, pBss->SsidLen);
		COPY_MAC_ADDR(pBss->Bssid, bss->Bssid);
		pBss->AuthMode = bss->AuthMode;
#ifdef CONFIG_MAP_SUPPORT
		NdisCopyMemory(&pBss->map_info, &bss->map_info, sizeof(struct map_vendor_ie));
		pBss->map_vendor_ie_found = bss->map_vendor_ie_found;
		pBss->AuthMode = WscGetAuthType(bss->AKMMap);
		pBss->EncrypType = WscGetEncryType(bss->PairwiseCipher);
#ifdef MAP_R2
		pBss->QbssLoad.bValid = bss->QbssLoad.bValid;
		pBss->QbssLoad.StaNum = bss->QbssLoad.StaNum;
		pBss->QbssLoad.ChannelUtilization = bss->QbssLoad.ChannelUtilization;
#endif
#ifdef MAP_6E_SUPPORT
		pBss->rnr_6e.channel = bss->rnr_info.channel;	/* 6E channel to be filled*/
		pBss->rnr_6e.op = bss->rnr_info.op;		/*op class for 6E channel*/
#ifdef DPP_R2_SUPPORT
		pBss->rnr_6e.cce_ind = bss->cce_vendor_ie_found;	/* CCE indication on 6G*/
#endif
#endif
#endif
		count++;
		if (pBss->SsidLen)
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"RTMPIoctlGetSiteSurvey - ssid %s\n", pBss->Ssid);
	}
	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"RTMPIoctlGetSiteSurvey - max_bss%d\n len=%u count=%d", max_bss, TotalLen, count);
	event->data.scan_info.bss_count = count;
	pAdapter->last_scan_req.last_bss_cnt = i;
	if (i < pAdapter->last_scan_req.bss_nr)
		event->data.scan_info.more_bss = 1;
	if (ScanTab->BssNr < pAdapter->last_scan_req.bss_nr)
		event->data.scan_info.more_bss = 0;
send_event:
	RtmpOSWrielessEventSend(pAdapter->net_dev, RT_WLAN_EVENT_CUSTOM,
			OID_WAPP_EVENT, NULL, (PUCHAR)event, TotalLen);

	os_free_mem((PUCHAR)msg);
}

#ifdef CONFIG_CPE_SUPPORT
VOID RTMPLppeGetScanResults(
	IN	PRTMP_ADAPTER	pAdapter, struct wapp_req *req)
{
	struct wapp_event *event;
	INT last_bss_cnt = 0;
	RTMP_STRING *msg;
	INT		i = 0;
	/*INT			WaitCnt = 0;*/
	UINT32		bss_start_idx;
	BSS_ENTRY *bss;
	struct scan_bss_info *pBss;
	wdev_ht_cap *ht_cap;
	wdev_vht_cap *vht_cap;
	wdev_he_cap *he_cap;
	char *mcsptr = NULL;
	UINT8 he_ch_width = HE_BW_80;
	UINT16	l;
	INT custom_event_length;
	UINT32 TotalLen;
	INT count = 0, max_bss;
	BSS_TABLE *ScanTab = NULL;
	struct wifi_dev *wdev = NULL;

	for (l = 0; l < WDEV_NUM_MAX; l++) {
		if (pAdapter->wdev_list[l] != NULL) {
			wdev = pAdapter->wdev_list[l];
			if (wdev->if_dev && (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex))
				break;
		}
	}

	if (wdev == NULL || !wdev->if_dev)
		return;

	ScanTab = get_scan_tab_by_wdev(pAdapter, wdev);
#ifndef IWEVCUSTOM_PAYLOD_MAX_LEN
#define IWEVCUSTOM_PAYLOD_MAX_LEN 220
#endif
	custom_event_length = IWEVCUSTOM_PAYLOD_MAX_LEN;
	TotalLen = custom_event_length;

	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "RTMPIoctlGetSiteSurvey - enter\n");
	/*
	 * At the moment there is a single scan tab across the pAd
	 * that means if scan id or if_index has changed, it doesn't
	 * make sense of continuing the last get scan command.
	 */
	if ((pAdapter->last_scan_req_lppe.scan_id != req->data.value) ||
	    (pAdapter->last_scan_req_lppe.if_index != req->data.ifindex)) {
		pAdapter->last_scan_req_lppe.scan_id = req->data.value;
		pAdapter->last_scan_req_lppe.if_index = req->data.ifindex;
	} else
		last_bss_cnt = pAdapter->last_scan_req_lppe.last_bss_cnt;

	if (ScanTab->BssNr == 0) {
		MTWF_DBG(pAdapter, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"no bss present in scan tab\n");
	}
	max_bss = custom_event_length / sizeof(struct scan_bss_info);

	bss_start_idx = last_bss_cnt;

	TotalLen = (sizeof(CHAR) * 4) + ((sizeof(struct scan_bss_info)) * max_bss) + (sizeof(UINT32) * 2);
	if ((max_bss > 0) && (TotalLen > custom_event_length))
		max_bss--;

	if (max_bss == 0) {
		MTWF_DBG(pAdapter, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"bss size is too big for wireless send event\n");
		return;
	}
	TotalLen = (sizeof(CHAR) * 4) + ((sizeof(struct scan_bss_info)) * max_bss) + (sizeof(UINT32) * 2);
	os_alloc_mem(NULL, (PUCHAR *)&msg, TotalLen);

	if (msg == NULL) {
		MTWF_DBG(pAdapter, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"RTMPIoctlGetSiteSurvey - msg memory alloc fail.\n");
		return;
	}

	NdisZeroMemory(msg, TotalLen);

	event = (struct wapp_event *)msg;
	event->event_id = LPPE_SCAN_RESULT_RSP;
	event->ifindex = req->data.ifindex;

	if (ScanTab->BssNr == 0) {
		/* Send the event as this is possible scenario during in DFS channe */
		goto send_event;
	}

	if (bss_start_idx > (ScanTab->BssNr - 1)) {
		event->data.scan_info.bss_count = 0;
		goto send_event;
	}

	for (i = bss_start_idx; i < ScanTab->BssNr && count < max_bss; i++) {
		bss = &ScanTab->BssEntry[i];
		pBss = &event->data.scan_info.bss[count];

		ht_cap = &pBss->ht_cap;
		vht_cap = &pBss->vht_cap;
		he_cap = &pBss->he_cap;
		if (bss->Channel == 0)
			break;
		pBss->Channel = bss->Channel;
		pBss->CentralChannel = bss->CentralChannel;
		pBss->Rssi = bss->Rssi;
		ht_cap->tx_stream = 0; /* TODO */
		ht_cap->rx_stream = 0; /* TODO */
		ht_cap->sgi_20 = bss->HtCapability.HtCapInfo.ShortGIfor20;
		ht_cap->sgi_40 = bss->HtCapability.HtCapInfo.ShortGIfor40;
		ht_cap->ht_40 = bss->HtCapability.HtCapInfo.ChannelWidth;

		NdisMoveMemory(vht_cap->sup_tx_mcs,
						&bss->vht_cap_ie.mcs_set.tx_mcs_map,
						sizeof(vht_cap->sup_tx_mcs));
		NdisMoveMemory(vht_cap->sup_rx_mcs,
						&bss->vht_cap_ie.mcs_set.rx_mcs_map,
						sizeof(vht_cap->sup_rx_mcs));
		if (bss->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss4 != VHT_MCS_CAP_NA)
			vht_cap->rx_stream = 4;
		else if (bss->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss3 != VHT_MCS_CAP_NA)
			vht_cap->rx_stream = 3;
		else if (bss->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 != VHT_MCS_CAP_NA)
			vht_cap->rx_stream = 2;
		else
			vht_cap->rx_stream = 1;

		if (bss->vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss4 != VHT_MCS_CAP_NA)
			vht_cap->tx_stream = 4;
		else if (bss->vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss3 != VHT_MCS_CAP_NA)
			vht_cap->tx_stream = 3;
		else if (bss->vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss2 != VHT_MCS_CAP_NA)
			vht_cap->tx_stream = 2;
		else
			vht_cap->tx_stream = 1;
		vht_cap->sgi_80 = bss->vht_cap_ie.vht_cap.sgi_80M;
		vht_cap->sgi_160 = bss->vht_cap_ie.vht_cap.sgi_160M;
		vht_cap->vht_160 = bss->vht_op_ie.vht_op_info.ch_width;
		vht_cap->vht_8080 = 0; /* TODO */
		vht_cap->su_bf = bss->vht_cap_ie.vht_cap.bfee_cap_su;
		vht_cap->mu_bf = bss->vht_cap_ie.vht_cap.bfee_cap_mu;
#ifdef DOT11_HE_AX
		he_ch_width = peer_max_bw_cap(GET_DOT11AX_CH_WIDTH(bss->he_caps.phy_cap.phy_capinfo_1));
		mcsptr = he_cap->he_mcs;
		NdisCopyMemory(mcsptr, &bss->he_caps.txrx_mcs_nss, sizeof(struct he_txrx_mcs_nss));
		if (he_ch_width == HE_BW_160) {
			he_cap->he_160 = 1;
			mcsptr += sizeof(struct he_txrx_mcs_nss);
			NdisCopyMemory(mcsptr, &bss->he_mcs_nss_160, sizeof(struct he_txrx_mcs_nss));
		} else
			he_cap->he_160 = 0;
#endif
		pBss->MinSNR = bss->MinSNR;
		pBss->Privacy = bss->Privacy;
		pBss->SsidLen = bss->SsidLen;
		NdisCopyMemory(pBss->Ssid, bss->Ssid, pBss->SsidLen);
		COPY_MAC_ADDR(pBss->Bssid, bss->Bssid);
		pBss->AuthMode = bss->AuthMode;
#ifdef CONFIG_MAP_SUPPORT
		NdisCopyMemory(&pBss->map_info, &bss->map_info, sizeof(struct map_vendor_ie));
		pBss->map_vendor_ie_found = bss->map_vendor_ie_found;
		pBss->AuthMode = WscGetAuthType(bss->AKMMap);
		pBss->EncrypType = WscGetEncryType(bss->PairwiseCipher);
#ifdef MAP_R2
		pBss->QbssLoad.bValid = bss->QbssLoad.bValid;
		pBss->QbssLoad.StaNum = bss->QbssLoad.StaNum;
		pBss->QbssLoad.ChannelUtilization = bss->QbssLoad.ChannelUtilization;
#endif
#endif
		count++;
		if (pBss->SsidLen)
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"RTMPIoctlGetSiteSurvey - ssid %s\n", pBss->Ssid);
	}
	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"RTMPLppeGetScanResults - max_bss%d\n len=%u count=%d", max_bss, TotalLen, count);
	event->data.scan_info.bss_count = count;
	pAdapter->last_scan_req_lppe.last_bss_cnt = i;
	if (i < ScanTab->BssNr)
		event->data.scan_info.more_bss = 1;
send_event:
	RtmpOSWrielessEventSend(pAdapter->net_dev, RT_WLAN_EVENT_CUSTOM,
			OID_LPPE_EVENT, NULL, (PUCHAR)event, TotalLen);

	os_free_mem((PUCHAR)msg);
}
#endif

VOID RTMPIoctlSendNullDataFrame(
	IN	PRTMP_ADAPTER	pAdapter, struct wapp_req *req)
{
	INT count, pkt_count;
	MAC_TABLE_ENTRY *pEntry = MacTableLookup(pAdapter, req->data.mac_addr);

	if (!pEntry) {
		MTWF_DBG(pAdapter, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "given station not found\n");
		return;
	}

	pkt_count = req->data.value;

	if (pEntry->PsMode == PWR_SAVE) {
		/* use TIM bit to detect the PS station */
		WLAN_MR_TIM_BIT_SET(pAdapter, pEntry->func_tb_idx, pEntry->Aid);
		OS_WAIT(200);
	} else {
		/* use Null or QoS Null to detect the ACTIVE station */
		BOOLEAN bQosNull = FALSE;

		if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
			bQosNull = TRUE;

		for (count = 0; count < pkt_count; count++) {
			/* TODO status */
			RtmpEnqueueNullFrame(pAdapter, pEntry->Addr, pEntry->CurrTxRate,
							 pEntry->Aid, pEntry->func_tb_idx, bQosNull, TRUE, 0);
		}
	}
}

void wapp_prepare_nop_channel_list(PRTMP_ADAPTER pAd,
	struct nop_channel_list_s *nop_list)
{
#ifdef MT_DFS_SUPPORT
	UINT_8 i;
	UINT_8 band_idx = 0;
	PCHANNEL_CTRL pChCtrl = NULL;
	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

		for (i = 0; i < pChCtrl->ChListNum; i++) {
			if ((pChCtrl->ChList[i].NonOccupancy > 0 || pChCtrl->ChList[i].NOPSaveForClear) && nop_list->channel_count < MAX_NUM_OF_CHANNELS) {
				nop_list->channel_list[nop_list->channel_count] = pChCtrl->ChList[i].Channel;
			nop_list->channel_count++;
			}
		}
	}
#endif /* MT_DFS_SUPPORT */
}
INT	wapp_event_handle(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	switch (req->req_id) {
	case WAPP_DEV_QUERY_REQ:
		wapp_send_wdev_query_rsp(pAd, req);
		break;
	case WAPP_HT_CAP_QUERY_REQ:
		wapp_send_wdev_ht_cap_rsp(pAd, req);
		break;
	case WAPP_VHT_CAP_QUERY_REQ:
		wapp_send_wdev_vht_cap_rsp(pAd, req);
		break;
	case WAPP_MISC_CAP_QUERY_REQ:
		wapp_send_wdev_misc_cap_rsp(pAd, req);
		break;
	case WAPP_CLI_QUERY_REQ:
		wapp_send_cli_query_rsp(pAd, req);
		break;
	case WAPP_CLI_LIST_QUERY_REQ:
		wapp_handle_cli_list_query(pAd, req);
		break;
	case WAPP_CHN_LIST_QUERY_REQ:
		wapp_send_chn_list_query_rsp(pAd, req);
		break;
	case WAPP_OP_CLASS_QUERY_REQ:
		wapp_send_op_class_query_rsp(pAd, req);
		break;
	case WAPP_BSS_INFO_QUERY_REQ:
		wapp_send_bss_info_query_rsp(pAd, req);
		break;
	case WAPP_AP_METRIC_QUERY_REQ:
		wapp_send_ap_metric_query_rsp(pAd, req);
		break;
#ifdef MAP_R2
	case WAPP_RADIO_METRICS_REQ:
		wapp_send_radio_metric_query_rsp(pAd, req);
		break;
#endif
	case WAPP_CH_UTIL_QUERY_REQ:
		wapp_send_ch_util_query_rsp(pAd, req);
		break;
	case WAPP_APCLI_QUERY_REQ:
		wapp_send_apcli_query_rsp(pAd, req);
		break;
	case WAPP_BSS_START_REQ:
		wapp_bss_start(pAd, req);
		break;
	case WAPP_BSS_STOP_REQ:
		wapp_bss_stop(pAd, req);
		break;
	case WAPP_BSS_LOAD_THRD_SET_REQ:
		wapp_bss_load_thrd_set(pAd, req);
		break;
	case WAPP_TXPWR_PRCTG_REQ:
		wapp_set_tx_power_prctg(pAd, req);
		break;
	case WAPP_STEERING_POLICY_SET_REQ:
		wapp_set_steer_policy(pAd, req);
		break;
	case WAPP_AP_CONFIG_SET_REQ:
		wapp_config_ap_setting(pAd, req);
		break;
	case WAPP_BSSLOAD_QUERY_REQ:
		wapp_send_bssload_query_rsp(pAd, req);
		break;
	case WAPP_HECAP_QUERY_REQ:
		wapp_send_he_cap_query_rsp(pAd, req);
		break;
	case WAPP_STA_RSSI_QUERY_REQ:
		wapp_send_sta_rssi_query_rsp(pAd, req);
		break;
	case WAPP_APCLI_RSSI_QUERY_REQ:
		wapp_send_apcli_rssi_query_rsp(pAd, req);
		break;
	case WAPP_GET_SCAN_RESULTS:
		RTMPIoctlGetScanResults(pAd, req);
		break;
	case WAPP_WSC_PBC_EXEC:
		{
			POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
			UCHAR ifIndex = pObj->ioctl_if;
			PWSC_CTRL pWscControl = NULL;

			if (ifIndex >= pAd->ApCfg.BssidNum) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
					"Unknown If index (%d)", ifIndex);
				return NDIS_STATUS_FAILURE;
			}
			pWscControl = &pAd->StaCfg[ifIndex].wdev.WscControl;
			WscPBCExec(pAd, FALSE, pWscControl);
			break;
		}
	case WAPP_SEND_NULL_FRAMES:
		RTMPIoctlSendNullDataFrame(pAd, req);
		break;
	case WAPP_WSC_SET_BH_PROFILE:
		{
			POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
			UCHAR ifIndex = pObj->ioctl_if;
			PWSC_CTRL pWscControl = NULL;
			int i = 0;

			pWscControl = &pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl;
			for (i = 0; i < pWscControl->WscBhProfiles.ProfileCnt; i++) {
				if (MAC_ADDR_EQUAL(pWscControl->WscBhProfiles.Profile[i].MacAddr,
					req->data.bh_wsc_profile.MacAddr)) {
					NdisCopyMemory(&pWscControl->WscBhProfiles.Profile[i],
						&req->data.bh_wsc_profile, sizeof(WSC_CREDENTIAL));
					break;
				}
			}
			if (i == pWscControl->WscBhProfiles.ProfileCnt) {
				NdisCopyMemory(&pWscControl->WscBhProfiles.Profile[i],
					&req->data.bh_wsc_profile, sizeof(WSC_CREDENTIAL));
				pWscControl->WscBhProfiles.ProfileCnt++;
			}
			break;
		}
#ifdef CONFIG_MAP_SUPPORT
	case WAPP_SET_SCAN_BH_SSIDS:
		{
			struct wifi_dev *wdev;
			POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
			UCHAR ifIndex = pObj->ioctl_if;

#ifdef APCLI_SUPPORT
			if (pObj->ioctl_if_type == INT_APCLI) {
			if (ifIndex >= pAd->ApCfg.ApCliNum) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "break if index is wrong\n");
				break;
			}
			wdev = &pAd->StaCfg[ifIndex].wdev;
			} else
#endif
			{
				if (pObj->ioctl_if_type == INT_MBSSID)
					wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
				else
					wdev = &pAd->ApCfg.MBSSID[0].wdev;
			}
			NdisZeroMemory(&wdev->MAPCfg.scan_bh_ssids,
				sizeof(wdev->MAPCfg.scan_bh_ssids));
			NdisCopyMemory(&wdev->MAPCfg.scan_bh_ssids,
				&req->data.scan_bh_ssids, sizeof(wdev->MAPCfg.scan_bh_ssids));
			break;
		}
	case WAPP_SET_AVOID_SCAN_CAC:
		pAd->bMAPAvoidScanDuringCac = (UINT8) req->data.value;
		break;
#endif /* CONFIG_MAP_SUPPORT */
#ifdef DPP_R2_SUPPORT
		case WAPP_GET_CCE_RESULT:
			RTMPIoctlGetCCEResults(pAd, req);
			break;
#endif

	case WAPP_SET_SRG_BITMAP:
		wapp_set_srg_bitmap(pAd, req);
		break;

	case WAPP_SET_TOPOLOGY_UPDATE:
		wapp_srg_topology_update(pAd, req);
		break;

	case WAPP_SET_SRG_UPLINK_STATUS:
		wapp_set_srg_uplink_traffic_status(pAd, req);
		break;

	case WAPP_SET_SR_MESH_STA_TH:
		wapp_set_meshsrlinkstath(pAd, req);
		break;

	case WAPP_SET_SR_MESH_BH_BITMAP:
		wapp_set_meshbhsrgbitmap(pAd, req);
		break;

	case WAPP_SET_SR_MESH_FH_BITMAP:
		wapp_set_meshfhsrgbitmap(pAd, req);
		break;

	case WAPP_SET_SR_MESH_BH_DL_THR:
		wapp_set_meshbhobsspdth(pAd, req);
		break;

	case WAPP_SET_SR_MESH_FH_DL_THR:
		wapp_set_meshfhobsspdth(pAd, req);
		break;

	case WAPP_SET_SR_LINK_FORBID_SR:
		wapp_set_linkforbidsr(pAd, req);
		break;

	case WAPP_SET_SRG_STA_MODE_RPT:
		wapp_send_sta_mode_rpt_cmd(pAd, req);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "invalid wapp req id=%d\n", req->req_id);
		break;
	}
	return 0;
}

INT set_wapp_param(
	IN PRTMP_ADAPTER pAd,
	UINT32 Param,
	UINT32 Value)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl;
	PGAS_CTRL pGASCtrl;

#ifdef CONFIG_HOTSPOT_R2
	PHOTSPOT_CTRL pHSCtrl;
	struct wifi_dev *wdev;
	pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;
	wdev = &pAd->ApCfg.MBSSID[APIndex].wdev;
#endif /* CONFIG_HOTSPOT_R2 */

	pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
	pGASCtrl = &pAd->ApCfg.MBSSID[APIndex].GASCtrl;

	switch (Param) {
	case PARAM_WNM_BSS_TRANSITION_MANAGEMENT:
		pWNMCtrl->WNMBTMEnable = Value;
		break;

	case PARAM_EXTERNAL_ANQP_SERVER_TEST:
		pGASCtrl->ExternalANQPServerTest = Value;
		break;

	case PARAM_GAS_COME_BACK_DELAY:
		pGASCtrl->cb_delay = Value;
		break;

	case PARAM_MMPDU_SIZE:
		pGASCtrl->MMPDUSize = Value;
		break;

	case PARAM_WNM_NOTIFICATION:
		pWNMCtrl->WNMNotifyEnable = Value;
		break;
#ifdef CONFIG_HOTSPOT_R2

	case PARAM_QOSMAP:
		pHSCtrl->QosMapEnable = Value;
		hotspot_update_bssflag(pAd, fgQosMapEnable, Value, pHSCtrl);
		break;
	case PARAM_DGAF_DISABLED:
		pHSCtrl->DGAFDisable = Value;
		hotspot_update_bssflag(pAd, fgDGAFDisable, Value, pHSCtrl);
		break;

	case PARAM_PROXY_ARP:
		pWNMCtrl->ProxyARPEnable = Value;
		hotspot_update_bssflag(pAd, fgProxyArpEnable, Value, pHSCtrl);
		break;

	case PARAM_L2_FILTER:
		pHSCtrl->L2Filter = Value;
		break;

	case PARAM_ICMPV4_DENY:
		pHSCtrl->ICMPv4Deny = Value;
		break;

#endif /* CONFIG_HOTSPOT_R2 */
	default:
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown Parameter:%d\n", Param);
		break;
	}
#ifdef CONFIG_HOTSPOT_R2
	/* for 7615 offload to CR4 */
	hotspot_update_bss_info_to_cr4(pAd, APIndex, wdev->bss_info_argument.ucBssIndex);
#endif /* CONFIG_HOTSPOT_R2 */

	return 0;
}

static INT set_wapp_cmm_ie(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 EID,
	IN RTMP_STRING *IE,
	IN UINT32 IELen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl =  &pAd->ApCfg.MBSSID[apidx].WNMCtrl;
	PGAS_CTRL pGasCtrl = &pAd->ApCfg.MBSSID[apidx].GASCtrl;
#ifdef CONFIG_HOTSPOT_R2
	PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[apidx].HotSpotCtrl;
#endif /* CONFIG_HOTSPOT_R2 */

	INT32 Ret;
	PUCHAR tmp_buf_ptr = NULL;

	Ret = os_alloc_mem(NULL, &tmp_buf_ptr, IELen);
	if (Ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Not enough memory\n");
		return FALSE;
	}
	NdisMoveMemory(tmp_buf_ptr, IE, IELen);

	switch (EID) {
	case IE_INTERWORKING:
		RTMP_SEM_LOCK(&pGasCtrl->IeLock);
		if (pGasCtrl->InterWorkingIE)
			os_free_mem(pGasCtrl->InterWorkingIE);
		pGasCtrl->InterWorkingIE = tmp_buf_ptr;
		pGasCtrl->InterWorkingIELen = IELen;
		RTMP_SEM_UNLOCK(&pGasCtrl->IeLock);
#ifdef CONFIG_HOTSPOT_R2
		pHSCtrl->AccessNetWorkType  = (*(IE + 2)) & 0x0F;
		if (IELen > 3) {
			pHSCtrl->IsHessid = TRUE;

			if (IELen == 7)
				NdisMoveMemory(pHSCtrl->Hessid, IE + 3, MAC_ADDR_LEN);
			else
				NdisMoveMemory(pHSCtrl->Hessid, IE + 5, MAC_ADDR_LEN);
		}
#endif /* CONFIG_HOTSPOT_R2 */
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set Interworking IE\n");
		break;
	case IE_ADVERTISEMENT_PROTO:
		RTMP_SEM_LOCK(&pGasCtrl->IeLock);
		if (pGasCtrl->AdvertisementProtoIE)
			os_free_mem(pGasCtrl->AdvertisementProtoIE);
		pGasCtrl->AdvertisementProtoIE = tmp_buf_ptr;
		pGasCtrl->AdvertisementProtoIELen = IELen;
		RTMP_SEM_UNLOCK(&pGasCtrl->IeLock);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set Advertisement Protocol IE\n");
		break;

	case IE_TIME_ADVERTISEMENT:
		RTMP_SEM_LOCK(&pWNMCtrl->IeLock);
		if (pWNMCtrl->TimeadvertisementIE)
			os_free_mem(pWNMCtrl->TimeadvertisementIE);
		pWNMCtrl->TimeadvertisementIE = tmp_buf_ptr;
		pWNMCtrl->TimeadvertisementIELen = IELen;
		RTMP_SEM_UNLOCK(&pWNMCtrl->IeLock);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set Time Advertisement IE\n");
		break;

	case IE_TIME_ZONE:
		RTMP_SEM_LOCK(&pWNMCtrl->IeLock);
		if (pWNMCtrl->TimezoneIE)
			os_free_mem(pWNMCtrl->TimezoneIE);
		pWNMCtrl->TimezoneIE = tmp_buf_ptr;
		pWNMCtrl->TimezoneIELen = IELen;
		RTMP_SEM_UNLOCK(&pWNMCtrl->IeLock);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set Time Zone IE\n");
		break;

#ifdef CONFIG_HOTSPOT_R2
	case IE_QOS_MAP_SET: {
		int tmp = 0;
		char *pos = (char *)(IE + 2);

		RTMP_SEM_LOCK(&pHSCtrl->IeLock);
		if (pHSCtrl->QosMapSetIE)
			os_free_mem(pHSCtrl->QosMapSetIE);
		pHSCtrl->QosMapSetIE = tmp_buf_ptr;
		pHSCtrl->QosMapSetIELen = IELen;
		RTMP_SEM_UNLOCK(&pHSCtrl->IeLock);

		for (tmp = 0; tmp < (IELen - 16 - 2) / 2; tmp++) {
			pHSCtrl->DscpException[tmp] = *pos & 0xff;
			pHSCtrl->DscpException[tmp] |= (*(pos + 1) & 0xff) << 8;
			pos += 2;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"DscpException[%d]:0x%x\n", tmp, pHSCtrl->DscpException[tmp]);
		}

		for (tmp = 0; tmp < 8; tmp++) {
			pHSCtrl->DscpRange[tmp] = *pos & 0xff;
			pHSCtrl->DscpRange[tmp] |= (*(pos + 1) & 0xff) << 8;
			pos += 2;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"DscpRange[%d]:0x%x\n", tmp, pHSCtrl->DscpRange[tmp]);
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("=========================================== Set Qos MAP Set IE\n"));
		break;
	}

	case IE_ROAMING_CONSORTIUM:
		RTMP_SEM_LOCK(&pHSCtrl->IeLock);
		if (pHSCtrl->RoamingConsortiumIE)
			os_free_mem(pHSCtrl->RoamingConsortiumIE);
		pHSCtrl->RoamingConsortiumIE = tmp_buf_ptr;
		pHSCtrl->RoamingConsortiumIELen = IELen;
		RTMP_SEM_UNLOCK(&pHSCtrl->IeLock);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set Roaming Consortium IE\n");
		break;
#endif /* CONFIG_HOTSPOT_R2 */

#ifdef QOS_R1
	case IE_QOS_R1_MAP_SET:
		if (IS_QOSR1_ENABLE(pAd))
			QoS_MapIE_Config(pAd, apidx, IE, IELen);

		os_free_mem(tmp_buf_ptr);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"-->Set Qos R1/R2 MAP Set IE\n");
		break;
#endif /* QOS_R1 */

	default:
		os_free_mem(tmp_buf_ptr);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown IE(EID = %d)\n", EID);
		break;
	}
	/* Update Beacon Template */
	if (pAd->ApCfg.MBSSID[apidx].wdev.bAllowBeaconing) {
		UpdateBeaconHandler(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, BCN_UPDATE_IE_CHG);
	}

	return TRUE;
}

#ifdef DPP_R2_SUPPORT
static void Set_DPP_VENDOR_SPECIFIC_IE(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 OUIType,
	IN RTMP_STRING *IE,
	IN UINT32 IELen,
	IN UCHAR apidx)
{
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set DPP IE\n");
	os_zero_mem(wdev->DPPCfg.cce_ie_buf, 6);
	wdev->DPPCfg.cce_ie_len = IELen;
	NdisMoveMemory(wdev->DPPCfg.cce_ie_buf, IE, wdev->DPPCfg.cce_ie_len);
	if (wdev->bAllowBeaconing)
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
}
#endif


static void Set_MTK_VENDOR_SPECIFIC_IE(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 OUIType,
	IN RTMP_STRING * IE,
	IN UINT32 IELen,
	IN UCHAR ApIdx)
{
#ifdef CONFIG_MAP_SUPPORT
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[ApIdx].wdev;

	if (!IS_MAP_TURNKEY_ENABLE(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "MAP turnkey is not enabled, skipping vendor ie settings\n");
		return;
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set MTK IE\n");
	os_zero_mem(wdev->MAPCfg.vendor_ie_buf, VENDOR_SPECIFIC_LEN);
	wdev->MAPCfg.vendor_ie_len = IELen;
	NdisMoveMemory(wdev->MAPCfg.vendor_ie_buf, IE, wdev->MAPCfg.vendor_ie_len);
	if (wdev->bAllowBeaconing)
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
#endif
}


static INT Set_AP_VENDOR_SPECIFIC_IE(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 OUIType,
	IN RTMP_STRING *IE,
	IN UINT32 IELen)
{
#ifdef CONFIG_HOTSPOT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[apidx].HotSpotCtrl;
#endif
	INT32 Ret;
	PUCHAR tmp_buf_ptr = NULL;

	Ret = os_alloc_mem(NULL, &tmp_buf_ptr, IELen);
	if (Ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Not enough memory\n");
		return FALSE;
	}
	NdisMoveMemory(tmp_buf_ptr, IE, IELen);

	switch (OUIType) {
#ifdef CONFIG_HOTSPOT

	case OUI_P2P:
		RTMP_SEM_LOCK(&pHSCtrl->IeLock);
		if (pHSCtrl->P2PIE)
			os_free_mem(pHSCtrl->P2PIE);
		pHSCtrl->P2PIE = tmp_buf_ptr;
		pHSCtrl->P2PIELen = IELen;
		RTMP_SEM_UNLOCK(&pHSCtrl->IeLock);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set P2P IE\n");
		break;

	case OUI_HS2_INDICATION:
		RTMP_SEM_LOCK(&pHSCtrl->IeLock);
		if (pHSCtrl->HSIndicationIE)
			os_free_mem(pHSCtrl->HSIndicationIE);
		pHSCtrl->HSIndicationIE = tmp_buf_ptr;
		pHSCtrl->HSIndicationIELen = IELen;
		RTMP_SEM_UNLOCK(&pHSCtrl->IeLock);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set HS2.0 Indication IE\n");
		break;
#endif

	default:
		os_free_mem(tmp_buf_ptr);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown OUIType = %d\n", OUIType);
		break;
	}

	return TRUE;
}

INT wapp_set_ap_ie(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *IE,
	IN UINT32 IELen,
	IN UCHAR ApIdx)
{
	UINT8 EID;
	UINT8 OUIType;

	EID = *IE;

	switch (EID) {
	case IE_INTERWORKING:
	case IE_ADVERTISEMENT_PROTO:
	case IE_TIME_ADVERTISEMENT:
	case IE_TIME_ZONE:
	case IE_QOS_MAP_SET:
	case IE_ROAMING_CONSORTIUM:
#ifdef QOS_R1
	case IE_QOS_R1_MAP_SET:
#endif
		set_wapp_cmm_ie(pAd, EID, IE, IELen);
		break;

	case IE_VENDOR_SPECIFIC:
		OUIType = *(IE + 5);
		if (NdisEqualMemory(&IE[2], &MTK_OUI[0], 3))
			Set_MTK_VENDOR_SPECIFIC_IE(pAd, OUIType, IE, IELen, ApIdx);
#ifdef DPP_R2_SUPPORT
		else if (NdisEqualMemory(&IE[2], &DPP_OUI[0], 3) &&
					(OUIType == WFA_DPP_CCE_OUITYPE))
			Set_DPP_VENDOR_SPECIFIC_IE(pAd, OUIType, IE, IELen, ApIdx);
#endif
		else
			Set_AP_VENDOR_SPECIFIC_IE(pAd, OUIType, IE, IELen);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown IE(EID = %d)\n", EID);
		break;
	}

	return TRUE;
}

BOOLEAN wapp_init(
	PRTMP_ADAPTER pAd,
	BSS_STRUCT *pMbss
)
{
	NdisCopyMemory(pMbss->ESPI_AC_BE, ESPI_AC_BE_DEFAULT, sizeof(pMbss->ESPI_AC_BE));
	NdisCopyMemory(pMbss->ESPI_AC_BK, ESPI_AC_BK_DEFAULT, sizeof(pMbss->ESPI_AC_BK));
	NdisCopyMemory(pMbss->ESPI_AC_VO, ESPI_AC_VO_DEFAULT, sizeof(pMbss->ESPI_AC_VO));
	NdisCopyMemory(pMbss->ESPI_AC_VI, ESPI_AC_VI_DEFAULT, sizeof(pMbss->ESPI_AC_VI));
	return 0;
}

#endif /* WAPP_SUPPORT */
