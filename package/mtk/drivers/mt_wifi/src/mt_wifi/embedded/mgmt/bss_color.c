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
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR,
				"wdev(idx:%d) can't acquire BSS color\n", wdev->wdev_idx);
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO, "wdev(idx:%d) acquire BSS color %d\n", wdev->wdev_idx, *color);
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
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO,
		"BSSID(%x,%x,%x,%x,%x,%x),BssIdx=%d,color(%d),disable(%d)\n",
		PRINT_MAC(bss.Bssid),
		bss.ucBssIndex,
		new_color->color,
		new_color->disabled);
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

VOID set_bss_color_disabled(struct wifi_dev *wdev, UINT8 OP_MODE, BOOLEAN disabled)
{
	struct wifi_dev *curr_wdev = NULL;
	struct _BSS_INFO_ARGUMENT_T *curr_bssinfo = NULL;
	struct bss_color_ctrl *curr_bss_color = NULL;
	UINT8 i = 0, band_idx = 0;
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *) wdev->sys_handle;


	if (!pAd) {

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR, "pAd is NULL!!!\n");
		return;
	}

	if (OP_MODE == BSS_COLOR_OPMODE_BAND) {
		band_idx = HcGetBandByWdev(wdev);

		/*Trigger collision for all AP's on this band */
		for (i = 0; i < WDEV_NUM_MAX; i++) {
			curr_wdev = pAd->wdev_list[i];
			if (curr_wdev && (band_idx == HcGetBandByWdev(curr_wdev))
				&& (WDEV_BSS_STATE(curr_wdev) == BSS_READY) && (curr_wdev->wdev_type == WDEV_TYPE_AP)) {

				curr_bssinfo = &curr_wdev->bss_info_argument;
				curr_bss_color = &curr_bssinfo->bss_color;
				curr_bss_color->disabled = disabled;


				/* update state on firmware side */
				deliver_bss_color(pAd, curr_bssinfo, curr_bss_color);

				/* update wlan_operation state */
				wlan_operate_set_he_bss_color(curr_wdev, curr_bss_color->color, curr_bss_color->disabled);


				/* update the Beacon content */
				UpdateBeaconHandler(pAd, curr_wdev, BCN_UPDATE_IE_CHG);


			}

		}
	} else if (OP_MODE == BSS_COLOR_OPMODE_SINGLE) {
		curr_bssinfo = &wdev->bss_info_argument;
		curr_bss_color = &curr_bssinfo->bss_color;
		curr_bss_color->disabled = disabled;


		/* update state on firmware side */
		deliver_bss_color(pAd, curr_bssinfo, curr_bss_color);

		/* update wlan_operation state */
		wlan_operate_set_he_bss_color(wdev, curr_bss_color->color, curr_bss_color->disabled);


		/* update the Beacon content */
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	}

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

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_DEBUG,
			"receive BSS color info, disabled = %d, color = %d\n",disabled, color);
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
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO, "collision is alleviated\n");
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


static void bss_color_trigger_collision(struct wifi_dev *wdev, UINT8 OP_MODE)
{
	struct wifi_dev *curr_wdev = NULL;
	struct _BSS_INFO_ARGUMENT_T *bssinfo = NULL;
	struct bss_color_ctrl *bss_color = NULL;
	struct _RTMP_ADAPTER *pAd = NULL;
#ifdef CONFIG_AP_SUPPORT
	UINT8 i = 0, band_idx = 0;
#endif

	if (WDEV_BSS_STATE(wdev) < BSS_READY) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR, "wdev(%d) bss not ready!!!\n",
				 wdev->wdev_idx);
		return;
	}

	pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (!pAd) {

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR, "pAd is NULL!!!\n");
		return;
	}
#ifdef CONFIG_AP_SUPPORT
	/*Trigger collision for all AP's on this band */
	if (OP_MODE == BSS_COLOR_OPMODE_BAND) {

		band_idx = HcGetBandByWdev(wdev);

		for (i = 0; i < WDEV_NUM_MAX; i++) {
			curr_wdev = pAd->wdev_list[i];
			if (curr_wdev && (band_idx == HcGetBandByWdev(curr_wdev))
				&& (WDEV_BSS_STATE(curr_wdev) == BSS_READY) && (curr_wdev->wdev_type == WDEV_TYPE_AP)) {

				bssinfo = &curr_wdev->bss_info_argument;
				bss_color = &bssinfo->bss_color;

				if (bss_color->collision_detected == FALSE)
					pAd->ApCfg.bss_color_cfg.rem_ap_bss_color_change_cnt[band_idx]++;


				/* since timeout handler will use the age out mechanism to
				 * determine the collision still exists, we need to keep
				 * updating the collision time here
				 */
				NdisGetSystemUpTime(&bss_color->collision_time);
				if (bss_color->collision_detected == FALSE) {
					bss_color->collision_detected = TRUE;
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO, "collision is detected\n");
					if (!bss_color->u.ap_ctrl.trigger_timer_running) {
						RTMPSetTimer(&bss_color->u.ap_ctrl.trigger_timer,
								bss_color->u.ap_ctrl.ap_collision_period * 1000);
						bss_color->u.ap_ctrl.trigger_timer_running = TRUE;
						MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO, "timer is triggered\n");
					}
				}


			}

		}
	} else
#endif
	if (OP_MODE == BSS_COLOR_OPMODE_SINGLE) {
		bssinfo = &wdev->bss_info_argument;
		bss_color = &bssinfo->bss_color;

#ifdef CONFIG_AP_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_AP) {

			/* since timeout handler will use the age out mechanism to
			 * determine the collision still exists, we need to keep
			 * updating the collision time here
			 */
			NdisGetSystemUpTime(&bss_color->collision_time);
			if (bss_color->collision_detected == FALSE) {
				bss_color->collision_detected = TRUE;
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO,
					"collision is detected\n");
				if (!bss_color->u.ap_ctrl.trigger_timer_running) {
					RTMPSetTimer(&bss_color->u.ap_ctrl.trigger_timer,
							bss_color->u.ap_ctrl.ap_collision_period * 1000);
					bss_color->u.ap_ctrl.trigger_timer_running = TRUE;
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO, "timer is triggered\n");
				}
			}
		}
#endif

#ifdef CONFIG_STA_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_STA) {
			if (bss_color->collision_detected == FALSE) {
				bss_color->collision_detected = TRUE;
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO,
					"collision is detected\n");
				if (!bss_color->u.sta_ctrl.notify_timer_running) {
					RTMPSetTimer(&bss_color->u.sta_ctrl.notify_timer,
							bss_color->u.sta_ctrl.sta_collision_period * 1000);
					bss_color->u.sta_ctrl.notify_timer_running = TRUE;
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO, "timer is triggered\n");
				}
			}
		}
#endif
	}
}



/* when AP receives event report for BSS color collision from associated STAs,
 * it parses the bitmap and updates the existing BSS color and then triggers
 * collision mechanism
 */
void bss_color_parse_collision_report(struct wifi_dev *wdev, UINT8 *bitmap)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#ifdef CONFIG_AP_SUPPORT
	UINT8 band_idx = 0, i = 0;
	struct wifi_dev *wdev_temp = NULL;
	struct _BSS_INFO_ARGUMENT_T *bssinfo_temp = NULL;
	struct bss_color_ctrl *bss_color_temp = NULL;
#endif


	if (!ad) {

		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR, "pAd is NULL!!!\n");
		return;
	}


	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO,
			"0x%02x %02x %02x %02x %02x %02x %02x %02x\n",
			 bitmap[7], bitmap[6], bitmap[5], bitmap[4],
			 bitmap[3], bitmap[2], bitmap[1], bitmap[0]);

#ifdef CONFIG_AP_SUPPORT
	band_idx = HcGetBandByWdev(wdev);

	if (IS_BSS_COLOR_MANUAL_ACTIVE(ad, band_idx)) {

		/*Trigger collision for all AP's on this band */
		for (i = 0; i < WDEV_NUM_MAX; i++) {
			wdev_temp = ad->wdev_list[i];
			if (wdev_temp && (band_idx == HcGetBandByWdev(wdev_temp))
				&& (WDEV_BSS_STATE(wdev_temp) == BSS_READY) && (wdev_temp->wdev_type == WDEV_TYPE_AP)) {

				bssinfo_temp = &wdev_temp->bss_info_argument;
				bss_color_temp = &bssinfo_temp->bss_color;

				hc_bcolor_update_by_bitmap(wdev, bitmap);
				bss_color_trigger_collision(wdev, BSS_COLOR_OPMODE_BAND);

			}

		}
	} else
#endif
	{
		hc_bcolor_update_by_bitmap(wdev, bitmap);
		bss_color_trigger_collision(wdev, BSS_COLOR_OPMODE_SINGLE);
	}
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
#ifdef CONFIG_AP_SUPPORT
	UINT8 band_idx = 0;
	RTMP_ADAPTER *pAd = NULL;
#endif

#ifdef CONFIG_AP_SUPPORT
	ASSERT(wdev->sys_handle);

	pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	if (!pAd) {

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR, "pAd is NULL!!!\n");
		return;
	}
	band_idx = HcGetBandByWdev(wdev);
#endif
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_DEBUG,
			"received BSS color disabled = %d, color = %d\n", disabled, color);

	if (disabled == FALSE) {
		if (color == bss_color->color) {
#ifdef CONFIG_AP_SUPPORT
			if (IS_BSS_COLOR_MANUAL_ACTIVE(pAd, band_idx)) {
				bss_color_trigger_collision(wdev, BSS_COLOR_OPMODE_BAND);
			} else
#endif
			bss_color_trigger_collision(wdev, BSS_COLOR_OPMODE_SINGLE);
		}
		else
			hc_bcolor_occupy(wdev, color);
	}
}

void bss_color_event_handler(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _BSS_INFO_ARGUMENT_T *bss_info = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = &bss_info->bss_color;
#ifdef CONFIG_AP_SUPPORT
	UINT8 band_idx = 0;
#endif
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO,
			"start to use the new BSS color\n");

	if (!ad) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR, "pAd is NULL!!!\n");
		return;
	}

	/* start to use new BSS Color */


#ifdef CONFIG_AP_SUPPORT
	band_idx = HcGetBandByWdev(wdev);
	if (IS_BSS_COLOR_MANUAL_ACTIVE(ad, band_idx)) {
		if (ad->ApCfg.bss_color_cfg.rem_ap_bss_color_change_cnt[band_idx] > 0)
			ad->ApCfg.bss_color_cfg.rem_ap_bss_color_change_cnt[band_idx]--;

	} else
#endif
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
#ifdef CONFIG_AP_SUPPORT
	if (IS_BSS_COLOR_MANUAL_ACTIVE(ad, band_idx) &&
			ad->ApCfg.bss_color_cfg.rem_ap_bss_color_change_cnt[band_idx] == 0) {
		set_bss_color_disabled(wdev, BSS_COLOR_OPMODE_BAND, FALSE);
		ad->ApCfg.bss_color_cfg.bss_color_next[band_idx] = 0;
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO,
					"BSS color change completed for all MBSS\n");
			return;
	}
#endif

}

void bss_color_init(struct wifi_dev *wdev, struct _BSS_INFO_ARGUMENT_T *bssinfo)
{
	struct bss_color_ctrl *bss_color = &bssinfo->bss_color;
#ifdef CONFIG_AP_SUPPORT
	UINT8 band_idx = 0;
	RTMP_ADAPTER *pAd = NULL;
#endif

#ifdef CONFIG_AP_SUPPORT
	ASSERT(wdev->sys_handle);

	pAd = (RTMP_ADAPTER *)wdev->sys_handle;

	if (!pAd) {

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR, "pAd is NULL!!!\n");
		return;
	}
#endif

	os_zero_mem(bss_color, sizeof(struct bss_color_ctrl));



#ifdef CONFIG_AP_SUPPORT
	band_idx = HcGetBandByWdev(wdev);

	if (wdev->wdev_type == WDEV_TYPE_AP) {

		if (pAd->ApCfg.bss_color_cfg.bss_color_enable[band_idx] == BSS_COLOR_DISABLE) {
			bss_color->disabled = TRUE;
			wlan_operate_set_he_bss_color(wdev, 0, TRUE);
			return;
		} else if (pAd->ApCfg.bss_color_cfg.bss_color_enable[band_idx] == BSS_COLOR_VALUE_AUTO_INIT) {
			if (bss_color_acquire(wdev, &bss_color->color) == FALSE) {
				wlan_operate_set_he_bss_color(wdev, 0, TRUE);
				return;
			}
		} else if (pAd->ApCfg.bss_color_cfg.bss_color_enable[band_idx] == BSS_COLOR_VALUE_AUTO_SEL_ALL_MBSS) {
			if (bss_color_acquire(wdev, &bss_color->color) == FALSE) {
				wlan_operate_set_he_bss_color(wdev, 0, TRUE);
				return;
			}
			pAd->ApCfg.bss_color_cfg.bss_color_enable[band_idx] = bss_color->color;

		} else {
			bss_color->color = pAd->ApCfg.bss_color_cfg.bss_color_enable[band_idx];
			bss_color->disabled = FALSE;
			hc_bcolor_occupy(wdev, bss_color->color);
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

void bss_color_for_all_wdev(struct wifi_dev *wdev, UINT8 *bitmap)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _BSS_INFO_ARGUMENT_T *bssinfo = &wdev->bss_info_argument;
	struct bss_color_ctrl *bss_color = &bssinfo->bss_color;
	struct wifi_dev *tmp_wdev;
	UINT_8 idx, band_idx;
	UINT8 maps[8];

	band_idx = HcGetBandByWdev(wdev);

	os_zero_mem(maps, sizeof(maps));

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (!ad->wdev_list[idx])
			continue;

		tmp_wdev = ad->wdev_list[idx];

		if (band_idx != HcGetBandByWdev(tmp_wdev))
			continue;

		if (tmp_wdev->wdev_type != WDEV_TYPE_AP)
			continue;

		bssinfo = &tmp_wdev->bss_info_argument;
		bss_color = &bssinfo->bss_color;

		if (bss_color->color >= BSS_COLOR_VALUE_MIN && bss_color->color <= BSS_COLOR_VALUE_MAX)
			maps[(bss_color->color) / 8] |= 1 << ((bss_color->color) % 8);
	}

	os_move_mem(bitmap, maps, sizeof(maps));
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
	UINT8 next_color = 0, band_idx = 0;


	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO, "enter\n");
	band_idx = HcGetBandByWdev(wdev);

	bss_color_ageout(wdev, BSS_COLOR_AGEOUT_TIME_DEFAULT);
	if (bss_color->collision_detected) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO,
				"trigger BSS color change procedure\n");

		bss_color->disabled = TRUE;

		/* update state on firmware side */
		deliver_bss_color(ad, bss_info, bss_color);

		/* update wlan_operation state */
		wlan_operate_set_he_bss_color(wdev, bss_color->color, TRUE);


		/*If manual bss color configured update the configured bss color */
		if (IS_BSS_COLOR_MANUAL_ACTIVE(ad, band_idx)) {

			if (!ad->ApCfg.bss_color_cfg.bss_color_next[band_idx]) {
				if (bss_color_acquire(wdev, &next_color) == FALSE) {
							/* error handling to add */
							bss_color->u.ap_ctrl.trigger_timer_running = FALSE;
							return;
				}
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO, "Got new color after collision:%d\n", next_color);
				ad->ApCfg.bss_color_cfg.bss_color_next[band_idx] = next_color;
				ad->ApCfg.bss_color_cfg.bss_color_enable[band_idx] = next_color;
				bss_color->next_color = next_color;
			} else {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO, "Update color to MBSS(%d):%d\n",
						wdev->wdev_idx, ad->ApCfg.bss_color_cfg.bss_color_next[band_idx]);
				bss_color->next_color = ad->ApCfg.bss_color_cfg.bss_color_next[band_idx];
			}
		} else 	{
		/* acquire next BSS color which is used in BSS Color Change
		 * Announcement IE and update it to wlan_opertaion module
		 */
		if (bss_color_acquire(wdev, &next_color) == FALSE) {
			/* error handling to add */
			bss_color->u.ap_ctrl.trigger_timer_running = FALSE;
			return;
		}
		bss_color->next_color = next_color;
		}


		wlan_operate_set_he_bss_next_color(wdev, bss_color->next_color, bss_color->u.ap_ctrl.bcc_count);

		/* update the Beacon content */
		UpdateBeaconHandler(ad, wdev, BCN_UPDATE_IE_CHG);

#if defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
		/* Notify SR module */
		if (IS_MAP_ENABLE(ad) && IS_MAP_R3_ENABLE(ad))
			SrMeshSelfSrgBMChangeEvent(ad, wdev, FALSE);
#endif
	}
	bss_color->u.ap_ctrl.trigger_timer_running = FALSE;
}
#endif

#ifdef CONFIG_STA_SUPPORT
static void send_event_report(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
#ifdef CONFIG_DOT11V_WNM
	struct _MLME_WNM_EVT_REPORT_STRUCT event_report;
	UINT8 bitmap[8] = {0};

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

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_INFO, "enter\n");
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
	UINT_8 idx = 0, bitmap[8] = {0};
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
			MTWF_PRINT("wdev_idx type dis color next collision running bitmap\n");
			MTWF_PRINT("-------- ---- --- ----- ---- --------- ------- ------------------\n");

			MTWF_PRINT("%8d %4d %3d %5d %4d %9d %7d 0x%02x%02x%02x%02x%02x%02x%02x%02x\n",
					 idx, wdev->wdev_type,
					 bss_color->disabled, bss_color->color,
					 bss_color->next_color,
					 bss_color->collision_detected, timer_running,
					 bitmap[7], bitmap[6], bitmap[5], bitmap[4],
					 bitmap[3], bitmap[2], bitmap[1], bitmap[0]);
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
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_ERROR, "invalid wdev\n");
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
		MTWF_DBG(ad, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "BSS color is not applied for this wdev\n");
	}
}
#ifdef CONFIG_AP_SUPPORT
NDIS_STATUS bss_color_profile_enable(IN struct _RTMP_ADAPTER *pAd, IN RTMP_STRING * buffer)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT8 u1BandIdx, u1BandNum, value = 0;
	RTMP_STRING *ptr;


#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode)
		u1BandNum = 2;
	else
		u1BandNum = 1;
#else
		u1BandNum = 1;
#endif /* DBDC_MODE */


	for (u1BandIdx = BAND0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), u1BandIdx++) {

		if (u1BandIdx >= u1BandNum)
			return NDIS_STATUS_INVALID_DATA;

		value = simple_strtol(ptr, 0, 10);

		if (value == BSS_COLOR_DISABLE)
			pAd->ApCfg.bss_color_cfg.bss_color_enable[u1BandIdx] = BSS_COLOR_DISABLE;
		else if (value == BSS_COLOR_VALUE_AUTO_INIT)
			pAd->ApCfg.bss_color_cfg.bss_color_enable[u1BandIdx] = BSS_COLOR_VALUE_AUTO_INIT;
		else if (value == BSS_COLOR_VALUE_AUTO_SEL_ALL_MBSS)
			pAd->ApCfg.bss_color_cfg.bss_color_enable[u1BandIdx] = BSS_COLOR_VALUE_AUTO_SEL_ALL_MBSS;
		else {
			if (value >= BSS_COLOR_VALUE_MIN && value <= BSS_COLOR_VALUE_MAX)
				pAd->ApCfg.bss_color_cfg.bss_color_enable[u1BandIdx] = value;
			else {
				pAd->ApCfg.bss_color_cfg.bss_color_enable[u1BandIdx] = BSS_COLOR_VALUE_AUTO_INIT;
				MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Manual configured BSS color value %d is not in range 1 >= bss color <=63 \n", value);
			}
		}

	}
	return Status;
}
#endif /*CONFIG_AP_SUPPORT*/
#endif
