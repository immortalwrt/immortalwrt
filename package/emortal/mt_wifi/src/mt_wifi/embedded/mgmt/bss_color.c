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

#ifdef DOT11_HE_AX

#include "rt_config.h"


#ifdef CONFIG_AP_SUPPORT
static BOOLEAN bss_color_acquire(struct wifi_dev *wdev, UINT8 *color)
{
	if (hc_bcolor_acquire(wdev, color) == FALSE) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR,
				("wdev(idx:%d) can't acquire BSS color\n", wdev->wdev_idx));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_TRACE,
			("wdev(idx:%d) acquire BSS color %d\n", wdev->wdev_idx, *color));
	return TRUE;
}
#endif

static void deliver_bss_color(
		struct _RTMP_ADAPTER *ad,
		struct _BSS_INFO_ARGUMENT_T *bss_info,
		struct bss_color_ctrl *bss_color)
{
	struct _BSS_INFO_ARGUMENT_T bss;
	struct bss_color_ctrl *new_color = &bss.bss_color;

	os_zero_mem(&bss, sizeof(bss));
	bss.ucBssIndex = bss_info->ucBssIndex;
	memcpy(bss.Bssid, bss_info->Bssid, MAC_ADDR_LEN);
	bss.u4BssInfoFeature = BSS_INFO_BSS_COLOR_FEATURE;
	new_color->disabled = bss_color->disabled;
	new_color->color = bss_color->color;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_TRACE,
		("%s:BSSID(%x,%x,%x,%x,%x,%x),BssIdx=%d,color(%d),disable(%d)\n",
		__func__,
		PRINT_MAC(bss.Bssid),
		bss.ucBssIndex,
		new_color->color,
		new_color->disabled));
	HW_UPDATE_BSSINFO(ad, &bss);
}

static void bss_color_ageout(struct wifi_dev *wdev, UINT8 sec)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _BSS_INFO_ARGUMENT_T *bssinfo = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = &bssinfo->bss_color;
#ifdef CONFIG_AP_SUPPORT
	ULONG current_time;
#endif
	UINT_8 idx;
	struct wifi_dev *tmp_wdev;

#ifdef CONFIG_AP_SUPPORT
	/* determine collision from OBSS AP still exists */
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		NdisGetSystemUpTime(&current_time);
		if (RTMP_TIME_AFTER(current_time, bss_color->collision_time + (sec * OS_HZ))) {
			bss_color->collision_time = 0;
			bss_color->collision_detected = FALSE;
		}
	}
#endif

	/* age out the hw resource table entry */
	hc_bcolor_ageout(wdev, sec);

	/* occupy the used BSS color again */
	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		tmp_wdev = ad->wdev_list[idx];
		if (tmp_wdev &&
				(tmp_wdev->wdev_type == WDEV_TYPE_AP || tmp_wdev->wdev_type == WDEV_TYPE_STA)) {
			bssinfo = &tmp_wdev->bss_info_argument;
			bss_color = &bssinfo->bss_color;
			if (bss_color->color)
				hc_bcolor_occupy(wdev, bss_color->color);
		}
	}
}

BOOLEAN get_bss_color_disabled(struct wifi_dev *wdev)
{
	struct _BSS_INFO_ARGUMENT_T *bss_info = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = (struct bss_color_ctrl *)&bss_info->bss_color;

	return bss_color->disabled;
}

BOOLEAN get_bss_color_collision(struct wifi_dev *wdev)
{
	struct _BSS_INFO_ARGUMENT_T *bss_info = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = (struct bss_color_ctrl *)&bss_info->bss_color;

	return bss_color->collision_detected;
}

UINT8 get_bss_color(struct wifi_dev *wdev)
{
	struct _BSS_INFO_ARGUMENT_T *bss_info = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = (struct bss_color_ctrl *)&bss_info->bss_color;

	return bss_color->color;
}

/* should be used on STA mode only */
void set_bss_color_info(struct wifi_dev *wdev, BOOLEAN disabled, UINT8 color)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _BSS_INFO_ARGUMENT_T *bss_info = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = (struct bss_color_ctrl *)&bss_info->bss_color;

	if (wdev->wdev_type != WDEV_TYPE_STA)
		return;

	if (WDEV_BSS_STATE(wdev) != BSS_READY)
		return;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO,
			("%s: receive BSS color info, disabled = %d, color = %d\n",
			 __func__, disabled, color));
	if ((bss_color->disabled == disabled) && (bss_color->color == color))
		return;

#ifdef CONFIG_STA_SUPPORT
	/* clear collision situation if it is triggered earilier */
	if ((bss_color->collision_detected == TRUE) && (bss_color->disabled != disabled)) {
		BOOLEAN cancelled;
		bss_color->collision_detected = FALSE;
		if (bss_color->u.sta_ctrl.notify_timer_running) {
			RTMPCancelTimer(&bss_color->u.sta_ctrl.notify_timer, &cancelled);
			bss_color->u.sta_ctrl.notify_timer_running = FALSE;
		}
		bss_color->u.sta_ctrl.collision_notify_times = 0;
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_TRACE, ("collision is alleviated\n"));
	}
#endif	/* CONFIG_STA_SUPPORT */

	bss_color->disabled = disabled;
	bss_color->color = color;

	hc_bcolor_occupy(wdev, bss_color->color);

	/* update state on firmware side */
	deliver_bss_color(ad, bss_info, bss_color);
}

void get_bss_color_bitmap(struct wifi_dev *wdev, UINT8 *bitmap)
{
	bss_color_ageout(wdev, BSS_COLOR_AGEOUT_TIME_DEFAULT);
	hc_bcolor_get_bitmap(wdev, bitmap);
}

static void bss_color_trigger_collision(struct wifi_dev *wdev, struct bss_color_ctrl *bss_color)
{
#ifdef CONFIG_AP_SUPPORT
	if (WDEV_BSS_STATE(wdev) < BSS_READY) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR, ("%s: wdev(%d) bss not ready!!!\n",
				 __func__, wdev->wdev_idx));
		return;
	}

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		/* since timeout handler will use the age out mechanism to
		 * determine the collision still exists, we need to keep
		 * updating the collision time here
		 */
		NdisGetSystemUpTime(&bss_color->collision_time);
		if (bss_color->collision_detected == FALSE) {
			bss_color->collision_detected = TRUE;
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_TRACE, ("collision is detected\n"));
			if (!bss_color->u.ap_ctrl.trigger_timer_running) {
				RTMPSetTimer(&bss_color->u.ap_ctrl.trigger_timer,
						bss_color->u.ap_ctrl.ap_collision_period * 1000);
				bss_color->u.ap_ctrl.trigger_timer_running = TRUE;
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_TRACE, ("timer is triggered\n"));
			}
		}
	}
#endif

#ifdef CONFIG_STA_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA) {
		if (bss_color->collision_detected == FALSE) {
			bss_color->collision_detected = TRUE;
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_TRACE, ("collision is detected\n"));
			if (!bss_color->u.sta_ctrl.notify_timer_running) {
				RTMPSetTimer(&bss_color->u.sta_ctrl.notify_timer,
						bss_color->u.sta_ctrl.sta_collision_period * 1000);
				bss_color->u.sta_ctrl.notify_timer_running = TRUE;
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_TRACE, ("timer is triggered\n"));
			}
		}
	}
#endif
}

/* when AP receives event report for BSS color collision from associated STAs,
 * it parses the bitmap and updates the existing BSS color and then triggers
 * collision mechanism
 */
void bss_color_parse_collision_report(struct wifi_dev *wdev, UINT8 *bitmap)
{
	struct _BSS_INFO_ARGUMENT_T *bssinfo = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = &bssinfo->bss_color;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_TRACE,
			("%s: 0x%02x %02x %02x %02x %02x %02x %02x %02x\n", __func__,
			 bitmap[7], bitmap[6], bitmap[5], bitmap[4],
			 bitmap[3], bitmap[2], bitmap[1], bitmap[0]));
	hc_bcolor_update_by_bitmap(wdev, bitmap);
	bss_color_trigger_collision(wdev, bss_color);
}

/* when AP receives event report for BSS color in use from not-associated STAs,
 * the event report field contains the BSS color in use by reporting STA
 */
void bss_color_parse_inuse_report(struct wifi_dev *wdev, UINT8 *color)
{
	hc_bcolor_occupy(wdev, *color);
}

/* when AP receives Beacons from Overlapping BSS, or STA receives Beacons from
 * other APs, it needs to determine if there is BSS color collision
 */
void bss_color_collision_detect(struct wifi_dev *wdev, BOOLEAN disabled, UINT8 color)
{
	struct _BSS_INFO_ARGUMENT_T *bssinfo = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = &bssinfo->bss_color;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO,
			("%s: received BSS color disabled = %d, color = %d\n", __func__, disabled, color));

	if (disabled == FALSE) {
		if (color == bss_color->color)
			bss_color_trigger_collision(wdev, bss_color);
		else
			hc_bcolor_occupy(wdev, color);
	}
}

void bss_color_event_handler(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _BSS_INFO_ARGUMENT_T *bss_info = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = &bss_info->bss_color;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_TRACE,
			("%s: start to use the new BSS color\n", __func__));
	/* start to use new BSS Color */
	bss_color->disabled = FALSE;
	bss_color->color = bss_color->next_color;
	bss_color->next_color = 0;
	bss_color->collision_detected = FALSE;

	/* update state on firmware side */
	deliver_bss_color(ad, bss_info, bss_color);

	/* update wlan_operation state */
	wlan_operate_set_he_bss_color(wdev, bss_color->color, bss_color->disabled);

	/* reset parameters in BSS Color Change Announcement IE */
	wlan_operate_set_he_bss_next_color(wdev, bss_color->next_color, 0/* countdown */);

	/* update the Beacon content */
	UpdateBeaconHandler(ad, wdev, BCN_UPDATE_IE_CHG);
}

void bss_color_init(struct wifi_dev *wdev, struct _BSS_INFO_ARGUMENT_T *bssinfo)
{
	struct bss_color_ctrl *bss_color = &bssinfo->bss_color;

	os_zero_mem(bss_color, sizeof(struct bss_color_ctrl));

#ifdef CONFIG_AP_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		if (bss_color_acquire(wdev, &bss_color->color) == FALSE) {
			wlan_operate_set_he_bss_color(wdev, 0, TRUE);
			return;
		}

		/* update wlan_operation state */
		wlan_operate_set_he_bss_color(wdev, bss_color->color, bss_color->disabled);

		bss_color->u.ap_ctrl.bcc_count = BSS_COLOR_CA_COUNT_DEFAULT;
		bss_color->u.ap_ctrl.ap_collision_period = BSS_COLOR_COLLISION_AP_PERIOD;
	}
#endif

#ifdef CONFIG_STA_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA)
		bss_color->u.sta_ctrl.sta_collision_period = BSS_COLOR_COLLISION_STA_PERIOD;
#endif

#ifdef CONFIG_ATE
	if (is_testmode_wdev(wdev->wdev_type)) {
		/* fix me, 0x2a for both HETB TX & HE SU TX to make PAPR meet certification */
		bss_color->color = 0x2e;
		bss_color->disabled = FALSE;
	}
#endif
}

void bss_color_timer_init(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _BSS_INFO_ARGUMENT_T *bssinfo = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = &bssinfo->bss_color;

#ifdef CONFIG_AP_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		RTMPInitTimer(ad, &bss_color->u.ap_ctrl.trigger_timer,
				GET_TIMER_FUNCTION(trigger_timer_callback), wdev, FALSE);
	}
#endif

#ifdef CONFIG_STA_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA) {
		RTMPInitTimer(ad, &bss_color->u.sta_ctrl.notify_timer,
				GET_TIMER_FUNCTION(notify_timer_callback), wdev, FALSE);
	}
#endif
}

void bss_color_timer_release(struct wifi_dev *wdev)
{
	struct _BSS_INFO_ARGUMENT_T *bssinfo = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = &bssinfo->bss_color;
	BOOLEAN cancelled;

#ifdef CONFIG_AP_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_AP)
		RTMPReleaseTimer(&bss_color->u.ap_ctrl.trigger_timer, &cancelled);
#endif

#ifdef CONFIG_STA_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA)
		RTMPReleaseTimer(&bss_color->u.sta_ctrl.notify_timer, &cancelled);
#endif
}

#ifdef CONFIG_AP_SUPPORT
void trigger_timer_callback(
		IN PVOID SystemSpecific1,
		IN PVOID FunctionContext,
		IN PVOID SystemSpecific2,
		IN PVOID SystemSpecific3)
{
	struct wifi_dev *wdev = (struct wifi_dev *)FunctionContext;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _BSS_INFO_ARGUMENT_T *bss_info = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = (struct bss_color_ctrl *)&bss_info->bss_color;
	UINT8 next_color = 0;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_TRACE, ("%s: enter\n", __func__));

	bss_color_ageout(wdev, BSS_COLOR_AGEOUT_TIME_DEFAULT);
	if (bss_color->collision_detected) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_TRACE,
				("%s: trigger BSS color change procedure\n", __func__));

		bss_color->disabled = TRUE;

		/* update state on firmware side */
		deliver_bss_color(ad, bss_info, bss_color);

		/* update wlan_operation state */
		wlan_operate_set_he_bss_color(wdev, bss_color->color, TRUE);

		/* acquire next BSS color which is used in BSS Color Change
		 * Announcement IE and update it to wlan_opertaion module
		 */
		if (bss_color_acquire(wdev, &next_color) == FALSE) {
			/* error handling to add */
			bss_color->u.ap_ctrl.trigger_timer_running = FALSE;
			return;
		}
		bss_color->next_color = next_color;
		wlan_operate_set_he_bss_next_color(wdev, next_color, bss_color->u.ap_ctrl.bcc_count);

		/* update the Beacon content */
		UpdateBeaconHandler(ad, wdev, BCN_UPDATE_IE_CHG);
	}
	bss_color->u.ap_ctrl.trigger_timer_running = FALSE;
}
#endif

#ifdef CONFIG_STA_SUPPORT
static void send_event_report(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
#ifdef CONFIG_DOT11V_WNM
	struct _MLME_WNM_EVT_REPORT_STRUCT event_report;
	UINT8 bitmap[8];

	os_zero_mem(&event_report, sizeof(event_report));
	event_report.wdev = wdev;
	event_report.diag_token = 0;
	event_report.element_id = IE_EVENT_REPORT;
	event_report.token = 0; /* autonomously */
	event_report.type = BSS_COLOR_COLLISION;
	event_report.status = STATUS_SUCCESSFUL;
	get_bss_color_bitmap(wdev, bitmap);
	os_move_mem(&event_report.report, bitmap, sizeof(bitmap));

	MlmeEnqueueWithWdev(ad, ACTION_STATE_MACHINE, MT2_MLME_WNM_EVT_REPORT,
			sizeof(struct _MLME_WNM_EVT_REPORT_STRUCT),
			(PVOID)&event_report, 0, wdev);
	RTMP_MLME_HANDLER(ad);
#endif
}

void notify_timer_callback(
		IN PVOID SystemSpecific1,
		IN PVOID FunctionContext,
		IN PVOID SystemSpecific2,
		IN PVOID SystemSpecific3)
{
	struct wifi_dev *wdev = (struct wifi_dev *)FunctionContext;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _BSS_INFO_ARGUMENT_T *bss_info = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = (struct bss_color_ctrl *)&bss_info->bss_color;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_TRACE, ("%s: enter\n", __func__));
	send_event_report(ad, wdev);

	bss_color->u.sta_ctrl.collision_notify_times++;
	if (bss_color->u.sta_ctrl.collision_notify_times >= BSS_COLOR_NOTIFY_TIMES_DEFAULT) {
		bss_color->u.sta_ctrl.notify_timer_running = FALSE;
	} else {
		RTMPSetTimer(&bss_color->u.sta_ctrl.notify_timer,
				bss_color->u.sta_ctrl.sta_collision_period * 1000);
	}
}
#endif

void show_bss_color_info(struct _RTMP_ADAPTER *ad)
{
	UINT_8 idx = 0, bitmap[8];
	struct wifi_dev *wdev;
	struct _BSS_INFO_ARGUMENT_T *bssinfo;
	struct bss_color_ctrl *bss_color;
	BOOLEAN timer_running = 0;

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (!ad->wdev_list[idx])
			continue;

		wdev = ad->wdev_list[idx];
		bssinfo = &wdev->bss_info_argument;
		bss_color = &bssinfo->bss_color;

		if (wdev->if_up_down_state &&
				((wdev->wdev_type == WDEV_TYPE_AP) ||
				  wdev->wdev_type == WDEV_TYPE_STA)) {
#ifdef CONFIG_AP_SUPPORT
			if (wdev->wdev_type == WDEV_TYPE_AP)
				timer_running = bss_color->u.ap_ctrl.trigger_timer_running;
#endif
#ifdef CONFIG_STA_SUPPORT
			if (wdev->wdev_type == WDEV_TYPE_STA)
				timer_running = bss_color->u.sta_ctrl.notify_timer_running;
#endif

			get_bss_color_bitmap(wdev, bitmap);
			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("wdev_idx type dis color next collision running bitmap\n"));
			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("-------- ---- --- ----- ---- --------- ------- ------------------\n"));

			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%8d %4d %3d %5d %4d %9d %7d 0x%02x%02x%02x%02x%02x%02x%02x%02x\n",
					 idx, wdev->wdev_type,
					 bss_color->disabled, bss_color->color,
					 bss_color->next_color,
					 bss_color->collision_detected, timer_running,
					 bitmap[7], bitmap[6], bitmap[5], bitmap[4],
					 bitmap[3], bitmap[2], bitmap[1], bitmap[0]));
		}
	}
}

void set_bss_color_dbg(struct _RTMP_ADAPTER *ad, UINT8 wdev_idx, UINT8 action, UINT8 value)
{
	struct wifi_dev *wdev;
	struct _BSS_INFO_ARGUMENT_T *bssinfo;
	struct bss_color_ctrl *bss_color;

	wdev = ad->wdev_list[wdev_idx];
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR, ("invalid wdev\n"));
		return;
	}

	if (wdev->if_up_down_state &&
			(wdev->wdev_type == WDEV_TYPE_AP || wdev->wdev_type == WDEV_TYPE_STA)) {
		bssinfo = &wdev->bss_info_argument;
		bss_color = &bssinfo->bss_color;

		switch (action) {
		case BSS_COLOR_DBG_OCCUPY:
			if (value < BSS_COLOR_VALUE_MIN || value > BSS_COLOR_VALUE_MAX)
				return;
			hc_bcolor_occupy(wdev, value);
			break;

		case BSS_COLOR_DBG_SETPERIOD:
#ifdef CONFIG_AP_SUPPORT
			if (wdev->wdev_type == WDEV_TYPE_AP) {
				if (value == 0)
					return;
				bss_color->u.ap_ctrl.ap_collision_period = value;
			}
#endif
#ifdef CONFIG_STA_SUPPORT
			if (wdev->wdev_type == WDEV_TYPE_STA) {
				if (value == 0)
					return;
				bss_color->u.sta_ctrl.sta_collision_period = value;
			}
#endif
			break;

		case BSS_COLOR_DBG_TRIGGER:
			NdisGetSystemUpTime(&bss_color->collision_time);
			bss_color->collision_detected = TRUE;
#ifdef CONFIG_AP_SUPPORT
			if (wdev->wdev_type == WDEV_TYPE_AP) {
				if (!bss_color->u.ap_ctrl.trigger_timer_running) {
					RTMPSetTimer(&bss_color->u.ap_ctrl.trigger_timer,
							bss_color->u.ap_ctrl.ap_collision_period * 1000);
					bss_color->u.ap_ctrl.trigger_timer_running = TRUE;
				}
			}
#endif
#ifdef CONFIG_STA_SUPPORT
			if (wdev->wdev_type == WDEV_TYPE_STA) {
				if (!bss_color->u.sta_ctrl.notify_timer_running) {
					RTMPSetTimer(&bss_color->u.sta_ctrl.notify_timer,
							bss_color->u.sta_ctrl.sta_collision_period * 1000);
					bss_color->u.sta_ctrl.notify_timer_running = TRUE;
				}
			}
#endif
			break;

		case BSS_COLOR_DBG_CHANGE:
#ifdef CONFIG_AP_SUPPORT
			if (wdev->wdev_type == WDEV_TYPE_AP)
				bss_color_event_handler(wdev);
#endif
			break;

		case BSS_COLOR_DBG_ASSIGN_MANUAL:
#ifdef CONFIG_AP_SUPPORT
			if (wdev->wdev_type != WDEV_TYPE_AP)
				break;

			/* check range */
			if (value < BSS_COLOR_VALUE_MIN || value > BSS_COLOR_VALUE_MAX)
				break;

			/* ignore it if desired color is same as current setting */
			if (bss_color->color == value)
				break;

			/* release previous color and apply new setting */
			hc_bcolor_release(wdev, bss_color->color);
			hc_bcolor_occupy(wdev, value);
			bss_color->color = value;
			bss_color->disabled = FALSE;

			/* update state on firmware side */
			deliver_bss_color(ad, bssinfo, bss_color);

			/* update wlan_operation state */
			wlan_operate_set_he_bss_color(wdev, bss_color->color, bss_color->disabled);

			/* update the Beacon content */
			UpdateBeaconHandler(ad, wdev, BCN_UPDATE_IE_CHG);
#endif
			break;

		case BSS_COLOR_DBG_CHANGE_MANUAL:
#ifdef CONFIG_AP_SUPPORT
			if (wdev->wdev_type != WDEV_TYPE_AP)
				break;

			/* check range */
			if (value < BSS_COLOR_VALUE_MIN || value > BSS_COLOR_VALUE_MAX)
				break;

			/* ignore it if desired color is same as current setting */
			if (bss_color->color == value)
				break;

			bss_color->disabled = TRUE;

			/* update state on firmware side */
			deliver_bss_color(ad, bssinfo, bss_color);

			/* update wlan_operation state */
			wlan_operate_set_he_bss_color(wdev, bss_color->color, TRUE);
			bss_color->next_color = value;
			wlan_operate_set_he_bss_next_color(wdev, value, bss_color->u.ap_ctrl.bcc_count);

			/* update the Beacon content */
			UpdateBeaconHandler(ad, wdev, BCN_UPDATE_IE_CHG);
#endif
			break;
		default:
			break;
		}
	} else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BSS color is not applied for this wdev\n"));
	}
}
#endif
