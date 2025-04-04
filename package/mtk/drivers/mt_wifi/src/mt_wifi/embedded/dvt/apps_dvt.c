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

#define DVT_MODNAME "APPS"

#define DVT_APPS_WTBL_DEBUG_ACT_QUERY	(QUERY_WTBL - 1)

static INT apps_dvt_1_3_basic_test(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, RTMP_STRING *arg)
{
	struct _STA_TR_ENTRY *tr_entry;
	union _wtbl_debug_u debug_u;
	UCHAR peer_psm_status = 0;
#ifdef CONFIG_STA_SUPPORT
	struct os_cookie *os_cookie = (struct os_cookie *)ad->OS_Cookie;
	struct _STA_ADMIN_CONFIG  *sta_cfg = &ad->StaCfg[os_cookie->ioctl_if];
#endif
	UCHAR wtbl_debug_cmd_id = WTBL_PEER_PS;
	UCHAR wtbl_debug_act = DVT_APPS_WTBL_DEBUG_ACT_QUERY;
	NDIS_PACKET *rx_packet = NULL;

	NdisZeroMemory(&debug_u, sizeof(union _wtbl_debug_u));

	DVT_LOG("arg,%s", arg);
	if (strcmp("ap", arg) == 0) {
		ad->veri_ctrl.verify_mode_on = VERIFY_ON;

		/*Step 1: wait for STA connection*/
		tr_entry = dvt_ut_notify_wait(ad, DVT_NOTIFY_WSYS_CONNECT_EVENT);
		ad->veri_ctrl.dump_rx_debug = 1;
		DVT_LOG("STA connected addr:"MACSTR"\n", MAC2STR(tr_entry->Addr));

		/*Step 2: check STA's wtbl content*/
		mt_wtbltlv_debug(ad, tr_entry->wcid, wtbl_debug_cmd_id, wtbl_debug_act, &debug_u);
		peer_psm_status = debug_u.wtbl_peer_ps_t.ucPsm;
		if (peer_psm_status == 1) {
			DVT_LOG("STA psm in WTBL is PM mode, out of expectation\n");
			ad->veri_ctrl.verify_mode_on = VERIFY_OFF;
			goto end;
		} else {
			DVT_LOG("Connected STA psm:%d\n", peer_psm_status);
		}
		/*Step 3: wait for STA's packet, check packet format, psm bit info.*/
		rx_packet = dvt_ut_notify_wait(ad, DVT_NOTIFY_TRAFFIC_RX_DATA_EVENT);

		/*Step 4: check STA's wtbl content*/
		mt_wtbltlv_debug(ad, tr_entry->wcid, wtbl_debug_cmd_id, wtbl_debug_act, &debug_u);
		peer_psm_status = debug_u.wtbl_peer_ps_t.ucPsm;
		if (peer_psm_status == 0) {
			DVT_LOG("Step 4: STA psm in WTBL is Active, out of expectation\n");
			ad->veri_ctrl.verify_mode_on = VERIFY_OFF;
			goto end;
		} else {
			DVT_LOG("Connected STA psm:%d, Pass!\n", peer_psm_status);
		}

		/*Step 5: wait for STA's packet, check packet format, psm bit info.*/
		rx_packet = dvt_ut_notify_wait(ad, DVT_NOTIFY_TRAFFIC_RX_DATA_EVENT);

		/*Step 6: check STA's wtbl content*/
		mt_wtbltlv_debug(ad, tr_entry->wcid, wtbl_debug_cmd_id, wtbl_debug_act, &debug_u);
		peer_psm_status = debug_u.wtbl_peer_ps_t.ucPsm;
		if (peer_psm_status == 1) {
			DVT_LOG("Step 6: STA psm in WTBL is PM mode, out of expectation\n");
			ad->veri_ctrl.verify_mode_on = VERIFY_OFF;
			goto end;
		} else {
			DVT_LOG("Connected STA psm:%d, Pass!\n", peer_psm_status);
		}

		ad->veri_ctrl.verify_mode_on = VERIFY_OFF;
	} else if (strcmp("sta", arg) == 0) {
#ifdef CONFIG_STA_SUPPORT
		UINT32 pkt_ctrl_map_input = 0;
		struct veri_designated_ctrl assign_ctrl_input;
		struct veri_app_head_input head_input, *phead_input = &head_input;

		if (INFRA_ON(sta_cfg))
			DVT_LOG("STA connected addr:"MACSTR"\n", MAC2STR(sta_cfg->wdev.bssid));
		else {
			DVT_LOG("STA is not connected to AP\n");
			goto end;
		}

		NdisZeroMemory(&head_input, sizeof(struct veri_app_head_input));

		phead_input->veri_pkt_type = FC_TYPE_DATA;
		phead_input->veri_pkt_subtype = SUBTYPE_QOS_NULL;
		COPY_MAC_ADDR(phead_input->addr1, sta_cfg->wdev.bssid);
		COPY_MAC_ADDR(phead_input->addr2, sta_cfg->wdev.if_addr);
		COPY_MAC_ADDR(phead_input->addr3, sta_cfg->wdev.bssid);
		prepare_veri_pkt_head(ad, &head_input);
		SET_VERI_PKT_CTRL_IDX(pkt_ctrl_map_input, VERI_PM_CTRL_BY_SW);
		prepare_veri_pkt_ctrl_en(ad, pkt_ctrl_map_input);
		NdisZeroMemory(&assign_ctrl_input, sizeof(struct veri_designated_ctrl));
		assign_ctrl_input.assigned_pm = 1;
		prepare_veri_pkt_ctrl_assign(ad, &assign_ctrl_input);

		DVT_LOG("STA triggers PM = 1 QoS_Null\n");
		send_veri_pkt(ad, NULL);

		msleep(2000);

		NdisZeroMemory(&assign_ctrl_input, sizeof(struct veri_designated_ctrl));
		assign_ctrl_input.assigned_pm = 0;
		prepare_veri_pkt_ctrl_assign(ad, &assign_ctrl_input);

		DVT_LOG("STA triggers PM = 0 QoS_Null\n");
		send_veri_pkt(ad, NULL);
#endif
	} else
		goto end;

	return DVT_STATUS_OK;

end:
	return DVT_STATUS_FAIL;
}

static dvt_fun apps_dvt_table[] = {
	apps_dvt_1_3_basic_test
};

static struct dvt_feature_entry apps_dvt = {
	.feature_name = "apps",
	.dvt_cnt = sizeof(apps_dvt_table)/sizeof(dvt_fun),
	.dvt_table = apps_dvt_table,
};

VOID apps_dvt_init(struct dvt_framework *dvt_ctrl)
{
	dvt_feature_register(dvt_ctrl, &apps_dvt);
	DVT_LOG("apps dvt init,ok");
}
