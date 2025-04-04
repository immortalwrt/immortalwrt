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
	rt_led.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#include	"rt_config.h"

#ifdef LED_CONTROL_SUPPORT
/* LED operation should modify based on platform, should fix later */
#ifdef MT7915_MT7916_COEXIST_COMPATIBLE

#if defined(MT7916)
static LED_INIT_TABLE led_init_table_mt7916[LED_MAX_NUM] = {
	/*DEFAULT*/
	{LED_WLAN_2G_init, LED_IDX_0, GPIO(14), HW_LED},
	{LED_WLAN_5G_init, LED_IDX_1, GPIO(15), HW_LED},
	{LED_WLAN_WPS_init, LED_IDX_2, GPIO(16), HW_LED}
};
#endif

#else /* MT7915_MT7916_COEXIST_COMPATIBLE */

#if defined(MT7916)
static LED_INIT_TABLE led_init_table[LED_MAX_NUM] = {
	/*DEFAULT*/
	{LED_WLAN_2G_init, LED_IDX_0, GPIO(14), HW_LED},
	{LED_WLAN_5G_init, LED_IDX_1, GPIO(15), HW_LED},
	{LED_WLAN_WPS_init, LED_IDX_2, GPIO(16), HW_LED}
};
#endif
#endif /* MT7915_MT7916_COEXIST_COMPATIBLE */

#ifdef CONFIG_ANDES_SUPPORT
INT LED_Array[16][16] = {
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ LED_SOLID_OFF,   LED_TX_BLINKING,    LED_SOLID_OFF,    LED_TX_BLINKING,   LED_SOLID_OFF,  -1,  LED_SOLID_ON, -1,   LED_BLINKING_170MS_ON_170MS_OFF, -1, -1, LED_WPS_5S_ON_3S_OFF_THEN_BLINKING, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{  3,  2,   -1,   -1,   -1, -1, 16,  1,  5, -1, -1, 17, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ -1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{  1,  2,    1,   -1,   -1, -1,  3, -1,  6, -1, -1,  0, -1, -1, -1, -1},
	{  1,  2,    1,   -1,   -1, -1, -1,  1,  4, -1, -1, 18, -1, -1, -1, -1}
};
#endif /* CONFIG_ANDES_SUPPORT */

/*
	========================================================================

	Routine Description:
		Set LED Status

	Arguments:
		pAd						Pointer to our adapter
		Status					LED Status

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPSetLEDStatus(RTMP_ADAPTER *pAd, UCHAR Status, UCHAR BandIdx)
{
	/*ULONG			data; */
	UCHAR			LinkStatus = 0;
	UCHAR			LedMode;
	UCHAR			MCUCmd = 0;
	BOOLEAN		bIgnored = FALSE;
	/* #ifdef MT76x0 */
	INT				LED_CMD = -1;
	/* #endif MT76x0 */
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_CPE_SUPPORT
	UCHAR i = 0, is5G = 0;
	UCHAR reverse_polarity = 0;
	struct wifi_dev *wdev_tmp = NULL;
#endif
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
	PWSC_CTRL		pWscControl = NULL;
#ifdef CONFIG_AP_SUPPORT
	pWscControl = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.WscControl;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	pWscControl = &pAd->StaCfg[0].wdev.WscControl;
#endif /* CONFIG_STA_SUPPORT */
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef CONFIG_ATE

	/*
		In ATE mode of RT2860 AP/STA, we have erased 8051 firmware.
		So LED mode is not supported when ATE is running.
	*/
	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */
	LedMode = LED_MODE(pAd);

	/* #ifdef MT76x0 */
	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT7615(pAd) || IS_MT7626(pAd) || IS_MT7915(pAd) ||
		IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		LedMode = 1;
#if defined(WSC_INCLUDED) && defined(WSC_LED_SUPPORT)

		if (Status > LED_WPS_SUCCESS)
#else
		if (Status > LED_POWER_UP)
#endif /* defined(WSC_INCLUDED) && defined(WSC_LED_SUPPORT) */
			return;
		else
			LED_CMD = LED_Array[LedMode][Status];
	}

	/* #endif MT76x0 */
	switch (Status) {
	case LED_LINK_DOWN:
		LinkStatus = LINK_STATUS_LINK_DOWN;
		pAd->LedCntl.LedIndicatorStrength = 0;
		MCUCmd = MCU_SET_LED_MODE;
		break;

	case LED_LINK_UP:
		wdev = get_default_wdev(pAd);

		if (!wdev) {
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"LED_WPS_IN_PROCESS\n");
			return;
		}

		if (WMODE_CAP_5G(wdev->PhyMode))
			LinkStatus = LINK_STATUS_ABAND_LINK_UP;
		else
			LinkStatus = LINK_STATUS_GBAND_LINK_UP;

		MCUCmd = MCU_SET_LED_MODE;
		break;

	case LED_RADIO_ON:
		LinkStatus = LINK_STATUS_RADIO_ON;
		MCUCmd = MCU_SET_LED_MODE;
		break;

	case LED_HALT:
		LedMode = 0; /* Driver sets MAC register and MAC controls LED */
		/* fallthrough */

	case LED_RADIO_OFF:
		LinkStatus = LINK_STATUS_RADIO_OFF;
		MCUCmd = MCU_SET_LED_MODE;
		break;

	case LED_WPS:
		LinkStatus = LINK_STATUS_WPS;
		MCUCmd = MCU_SET_LED_MODE;
		break;

	case LED_ON_SITE_SURVEY:
		LinkStatus = LINK_STATUS_ON_SITE_SURVEY;
		MCUCmd = MCU_SET_LED_MODE;
		break;

	case LED_POWER_UP:
		LinkStatus = LINK_STATUS_POWER_UP;
		MCUCmd = MCU_SET_LED_MODE;
		break;
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT

	case LED_WPS_IN_PROCESS:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_WPS_IN_PROCESS;
			MCUCmd = MCU_SET_WPS_LED_MODE;
			pWscControl->WscLEDMode = LED_WPS_IN_PROCESS;
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: LED_WPS_IN_PROCESS\n", __func__);
		} else
			bIgnored = TRUE;

		break;

	case LED_WPS_ERROR:
		if (WscSupportWPSLEDMode(pAd)) {
			/* In the case of LED mode 9, the error LED should be turned on only after WPS walk time expiration. */
			if ((pWscControl->bWPSWalkTimeExpiration == FALSE) &&
				(LED_MODE(pAd) == WPS_LED_MODE_9)) {
				/* do nothing. */
			} else {
				LinkStatus = LINK_STATUS_WPS_ERROR;
				MCUCmd = MCU_SET_WPS_LED_MODE;
			}

			pWscControl->WscLEDMode = LED_WPS_ERROR;
			pWscControl->WscLastWarningLEDMode = LED_WPS_ERROR;
		} else
			bIgnored = TRUE;

		break;

	case LED_WPS_SESSION_OVERLAP_DETECTED:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_WPS_SESSION_OVERLAP_DETECTED;
			MCUCmd = MCU_SET_WPS_LED_MODE;
			pWscControl->WscLEDMode = LED_WPS_SESSION_OVERLAP_DETECTED;
			pWscControl->WscLastWarningLEDMode = LED_WPS_SESSION_OVERLAP_DETECTED;
		} else
			bIgnored = TRUE;

		break;

	case LED_WPS_SUCCESS:
		if (WscSupportWPSLEDMode(pAd)) {
			if ((LED_MODE(pAd) == WPS_LED_MODE_7) ||
				(LED_MODE(pAd) == WPS_LED_MODE_11) ||
				(LED_MODE(pAd) == WPS_LED_MODE_12)
			   ) {
				/* In the WPS LED mode 7, 11 and 12, the blue LED would last 300 seconds regardless of the AP's security settings. */
				LinkStatus = LINK_STATUS_WPS_SUCCESS_WITH_SECURITY;
				MCUCmd = MCU_SET_WPS_LED_MODE;
				pWscControl->WscLEDMode = LED_WPS_SUCCESS;
				/* Turn off the WPS successful LED pattern after 300 seconds. */
				RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
			} else if (LED_MODE(pAd) == WPS_LED_MODE_8) { /* The WPS LED mode 8 */
				if (WscAPHasSecuritySetting(pAd, pWscControl)) { /* The WPS AP has the security setting. */
					LinkStatus = LINK_STATUS_WPS_SUCCESS_WITH_SECURITY;
					MCUCmd = MCU_SET_WPS_LED_MODE;
					pWscControl->WscLEDMode = LED_WPS_SUCCESS;
					/* Turn off the WPS successful LED pattern after 300 seconds. */
					RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
				} else { /* The WPS AP does not have the secuirty setting. */
					LinkStatus = LINK_STATUS_WPS_SUCCESS_WITHOUT_SECURITY;
					MCUCmd = MCU_SET_WPS_LED_MODE;
					pWscControl->WscLEDMode = LED_WPS_SUCCESS;
					/* Turn off the WPS successful LED pattern after 300 seconds. */
					RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
				}
			} else if (LED_MODE(pAd) == WPS_LED_MODE_9) { /* The WPS LED mode 9. */
				/* Always turn on the WPS blue LED for 300 seconds. */
				LinkStatus = LINK_STATUS_WPS_BLUE_LED;
				MCUCmd = MCU_SET_WPS_LED_MODE;
				pWscControl->WscLEDMode = LED_WPS_SUCCESS;
				/* Turn off the WPS successful LED pattern after 300 seconds. */
				RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_SUCCESSFUL_LED_PATTERN_TIMEOUT);
			} else {
				MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: LED_WPS_SUCCESS (Incorrect LED mode = %d)\n",
						 __func__, LED_MODE(pAd));
				ASSERT(FALSE);
			}
		} else
			bIgnored = TRUE;

		break;

	case LED_WPS_TURN_LED_OFF:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_WPS_TURN_LED_OFF;
			MCUCmd = MCU_SET_WPS_LED_MODE;
			pWscControl->WscLEDMode = LED_WPS_TURN_LED_OFF;
		} else
			bIgnored = TRUE;

		break;

	case LED_WPS_TURN_ON_BLUE_LED:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_WPS_BLUE_LED;
			MCUCmd = MCU_SET_WPS_LED_MODE;
			pWscControl->WscLEDMode = LED_WPS_SUCCESS;
		} else
			bIgnored = TRUE;

		break;

	case LED_NORMAL_CONNECTION_WITHOUT_SECURITY:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_NORMAL_CONNECTION_WITHOUT_SECURITY;
			MCUCmd = MCU_SET_WPS_LED_MODE;
			pWscControl->WscLEDMode = LED_WPS_SUCCESS;
		} else
			bIgnored = TRUE;

		break;

	case LED_NORMAL_CONNECTION_WITH_SECURITY:
		if (WscSupportWPSLEDMode(pAd)) {
			LinkStatus = LINK_STATUS_NORMAL_CONNECTION_WITH_SECURITY;
			MCUCmd = MCU_SET_WPS_LED_MODE;
			pWscControl->WscLEDMode = LED_WPS_SUCCESS;
		} else
			bIgnored = TRUE;

		break;

	/*WPS LED Mode 10 */
	case LED_WPS_MODE10_TURN_ON:
		if (WscSupportWPSLEDMode10(pAd)) {
			LinkStatus = LINK_STATUS_WPS_MODE10_TURN_ON;
			MCUCmd = MCU_SET_WPS_LED_MODE;
		} else
			bIgnored = TRUE;

		break;

	case LED_WPS_MODE10_FLASH:
		if (WscSupportWPSLEDMode10(pAd)) {
			LinkStatus = LINK_STATUS_WPS_MODE10_FLASH;
			MCUCmd = MCU_SET_WPS_LED_MODE;
		} else
			bIgnored = TRUE;

		break;

	case LED_WPS_MODE10_TURN_OFF:
		if (WscSupportWPSLEDMode10(pAd)) {
			LinkStatus = LINK_STATUS_WPS_MODE10_TURN_OFF;
			MCUCmd = MCU_SET_WPS_LED_MODE;
		} else
			bIgnored = TRUE;

		break;
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("RTMPSetLED::Unknown Status 0x%x\n", Status));
		break;
	}

	if (IS_MT7615(pAd) || IS_MT7663(pAd) || IS_MT7626(pAd) || IS_MT7915(pAd) ||
		IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
#ifdef CONFIG_CPE_SUPPORT
		for (i = 0; i < WDEV_NUM_MAX; i++) {
			wdev_tmp = pAd->wdev_list[i];
			if (wdev_tmp) {
				if (WMODE_CAP_5G(wdev_tmp->PhyMode))
					is5G = 1;
				break;
			}
		}
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RTMPSetLED::is5G=%d\n", is5G);
#endif
		if ((Status == LED_RADIO_OFF) || (Status == LED_LINK_DOWN)) {
			UCHAR i = 0;
			UCHAR wdev_same_band_cnt = 0;
			struct wifi_dev *wdev_temp = NULL;

			for (i = 0; i < WDEV_NUM_MAX; i++) {
				wdev_temp = pAd->wdev_list[i];
				if (wdev_temp && wdev_temp->if_up_down_state) {
					if (HcGetBandByWdev(wdev_temp) == BandIdx) {
						wdev_same_band_cnt++;
						break;
					}
				}
			}
#ifdef CONFIG_CPE_SUPPORT
#ifdef MT6990
			if (IS_MT7915(pAd))
				reverse_polarity = LED_REVERSE_POLARITY_ENABLE;
			else
				reverse_polarity = LED_REVERSE_POLARITY_DISABLE;
#else
			reverse_polarity = LED_REVERSE_POLARITY_DISABLE;
#endif
			if (wdev_same_band_cnt == 0) {
				if (pAd->CommonCfg.dbdc_mode)
					/*if all interface in the band has been down.turn led off*/
					AndesLedEnhanceOP(pAd, BandIdx, LED_TX_OVER_BLINK_DISABLE,
					reverse_polarity, BandIdx, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
				else {
					if (is5G)
						AndesLedEnhanceOP(pAd, 1, LED_TX_OVER_BLINK_DISABLE,
						reverse_polarity, 0, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
					else
						AndesLedEnhanceOP(pAd, 0, LED_TX_OVER_BLINK_DISABLE,
						reverse_polarity, 0, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
				}
			}
#else
			if ((Status == LED_RADIO_OFF) && (wdev_same_band_cnt == 0)) { /* Interface down case */
				AndesLedEnhanceOP(pAd, BandIdx, LED_TX_OVER_BLINK_DISABLE, LED_REVERSE_POLARITY_ENABLE,
					BandIdx, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
			} else if ((Status == LED_LINK_DOWN) && (wdev_same_band_cnt == 1)) { /* Radio On/Off case */
				AndesLedEnhanceOP(pAd, BandIdx, LED_TX_OVER_BLINK_DISABLE, LED_REVERSE_POLARITY_ENABLE,
						BandIdx, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
			}
#endif
		} else {
			if (IS_MT7626(pAd))
				AndesLedEnhanceOP(pAd, BandIdx, LED_TX_OVER_BLINK_DISABLE, LED_REVERSE_POLARITY_DISABLE, BandIdx, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
			else if (IS_MT7663(pAd))
				AndesLedEnhanceOP(pAd, LED_ID_WPS, LED_TX_OVER_BLINK_DISABLE, LED_REVERSE_POLARITY_DISABLE, LED_BAND_0, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
			else if (IS_MT7915(pAd)) {
#ifdef CONFIG_CPE_SUPPORT
#ifdef MT6990
				reverse_polarity = LED_REVERSE_POLARITY_DISABLE;
#else
				reverse_polarity = LED_REVERSE_POLARITY_ENABLE;
#endif
				if (Status == LED_LINK_UP || Status == LED_RADIO_ON) {
					if (pAd->CommonCfg.dbdc_mode)
						AndesLedEnhanceOP(pAd, BandIdx, LED_TX_OVER_BLINK_DISABLE,
						reverse_polarity, BandIdx, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
					else {
						if (is5G)
							AndesLedEnhanceOP(pAd, 1, LED_TX_OVER_BLINK_DISABLE,
							reverse_polarity, 0, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
						else
							AndesLedEnhanceOP(pAd, 0, LED_TX_OVER_BLINK_DISABLE,
							reverse_polarity, 0, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
					}
				}
#endif
			} else if (IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
				if ((Status == LED_LINK_UP) || (Status == LED_RADIO_ON)) {
					AndesLedEnhanceOP(pAd, BandIdx, LED_TX_OVER_BLINK_DISABLE, LED_REVERSE_POLARITY_DISABLE,
										BandIdx, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
				}
			} else
				AndesLedEnhanceOP(pAd, LED_ID_WLAN_OD, LED_TX_OVER_BLINK_DISABLE, LED_REVERSE_POLARITY_DISABLE, LED_BAND_0, LED_TX_DATA_ONLY, 0, 0, LED_CMD);
		}
	} else {
		AndesLedEnhanceOP(pAd, LED_ID_WLAN_OD, LED_TX_OVER_BLINK_DISABLE, LED_REVERSE_POLARITY_DISABLE,
			0, LED_ALL_TX_FRAMES, 0, 0, LED_CMD);
	}
	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT7615(pAd) || IS_MT7663(pAd) || IS_MT7626(pAd)) {
		AndesLedEnhanceOP(pAd, LED_ID_WLAN_OD, LED_TX_OVER_BLINK_DISABLE, LED_REVERSE_POLARITY_DISABLE,
		0, LED_ALL_TX_FRAMES, 0, 0, LED_CMD);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: LED_CMD:0x%x, LED Mode:0x%x, LinkStatus:0x%x\n", __func__, LED_CMD, LedMode, LinkStatus);
	} else if (MCUCmd) {
		AsicSendCommandToMcu(pAd, MCUCmd, 0xff, LedMode, LinkStatus, FALSE);
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: MCUCmd:0x%x, LED Mode:0x%x, LinkStatus:0x%x\n", __func__, MCUCmd, LedMode, LinkStatus);
	}

	/*
		Keep LED status for LED SiteSurvey mode.
		After SiteSurvey, we will set the LED mode to previous status.
	*/
	if ((Status != LED_ON_SITE_SURVEY) && (Status != LED_POWER_UP) && (bIgnored == FALSE))
		pAd->LedCntl.LedStatus = Status;
}


/*
	========================================================================

	Routine Description:
		Set LED Signal Stregth

	Arguments:
		pAd						Pointer to our adapter
		Dbm						Signal Stregth

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		Can be run on any IRQL level.

		According to Microsoft Zero Config Wireless Signal Stregth definition as belows.
		<= -90  No Signal
		<= -81  Very Low
		<= -71  Low
		<= -67  Good
		<= -57  Very Good
		 > -57  Excellent
	========================================================================
*/
VOID RTMPSetSignalLED(RTMP_ADAPTER *pAd, NDIS_802_11_RSSI Dbm)
{
	UCHAR		nLed = 0;

	if (pAd->LedCntl.MCULedCntl.field.LedMode == LED_MODE_SIGNAL_STREGTH) {
		if (Dbm <= -90)
			nLed = 0;
		else if (Dbm <= -81)
			nLed = 1;
		else if (Dbm <= -71)
			nLed = 3;
		else if (Dbm <= -67)
			nLed = 7;
		else if (Dbm <= -57)
			nLed = 15;
		else
			nLed = 31;

		/* */
		/* Update Signal Stregth to firmware if changed. */
		/* */
		if (pAd->LedCntl.LedIndicatorStrength != nLed) {
			AsicSendCommandToMcu(pAd, MCU_SET_LED_GPIO_SIGNAL_CFG, 0xff, nLed, pAd->LedCntl.MCULedCntl.field.Polarity, FALSE);
			pAd->LedCntl.LedIndicatorStrength = nLed;
		}
	}
}


#ifdef WSC_STA_SUPPORT
#ifdef WSC_LED_SUPPORT
/*
	LED indication for normal connection start.
*/
VOID LEDConnectionStart(RTMP_ADAPTER *pAd)
{
	/* LED indication. */
	/*if (pAd->StaCfg[0].WscControl.bWPSSession == FALSE) */
	/*if (pAd->StaCfg[0].WscControl.WscConfMode != WSC_DISABLE && pAd->StaCfg[0].WscControl.bWscTrigger) */
	if (pAd->StaCfg[0].wdev.WscControl.WscConfMode == WSC_DISABLE) {
		if (LED_MODE(pAd) == WPS_LED_MODE_9) { /* LED mode 9. */
			UCHAR WPSLEDStatus = 0;

			/* The AP uses OPEN-NONE. */
			if ((IS_AKM_OPEN(pAd->StaCfg[0].wdev.SecConfig.AKMMap))
				&& (IS_CIPHER_NONE(pAd->StaCfg[0].wdev.SecConfig.PairwiseCipher)))
				WPSLEDStatus = LED_WPS_TURN_LED_OFF;
			else /* The AP uses an encryption algorithm. */
				WPSLEDStatus = LED_WPS_IN_PROCESS;

			RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(&pAd->StaCfg[0].wdev));
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: %d\n", __func__, WPSLEDStatus);
		}
	}
}


/*
	LED indication for normal connection completion.
*/
VOID LEDConnectionCompletion(RTMP_ADAPTER *pAd, BOOLEAN bSuccess)
{
	/* LED indication. */
	/*if (pAd->StaCfg[0].WscControl.bWPSSession == FALSE) */
	if (pAd->StaCfg[0].wdev.WscControl.WscConfMode == WSC_DISABLE) {
		if (LED_MODE(pAd) == WPS_LED_MODE_9) { /* LED mode 9. */
			UCHAR WPSLEDStatus = 0;

			if (bSuccess == TRUE) { /* Successful connenction. */
				/* The AP uses OPEN-NONE. */
				if ((IS_AKM_OPEN(pAd->StaCfg[0].wdev.SecConfig.AKMMap))
					&& (IS_CIPHER_NONE(pAd->StaCfg[0].wdev.SecConfig.PairwiseCipher)))
					WPSLEDStatus = LED_NORMAL_CONNECTION_WITHOUT_SECURITY;
				else /* The AP uses an encryption algorithm. */
					WPSLEDStatus = LED_NORMAL_CONNECTION_WITH_SECURITY;
			} else /* Connection failure. */
				WPSLEDStatus = LED_WPS_TURN_LED_OFF;

			RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(&pAd->StaCfg[0].wdev));
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: %d\n", __func__, WPSLEDStatus);
		}
	}
}
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_STA_SUPPORT */


void rtmp_read_led_setting_from_eeprom(RTMP_ADAPTER *pAd)
{
	USHORT Value = 0;
	PLED_CONTROL pLedCntl = &pAd->LedCntl;
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, Value);
		pLedCntl->MCULedCntl.word = (Value >> 8);
		RT28xx_EEPROM_READ16(pAd, EEPROM_LEDAG_CONF_OFFSET, Value);
		pLedCntl->LedAGCfg = Value;
		RT28xx_EEPROM_READ16(pAd, EEPROM_LEDACT_CONF_OFFSET, Value);
		pLedCntl->LedACTCfg = Value;
		RT28xx_EEPROM_READ16(pAd, EEPROM_LED_POLARITY_OFFSET, Value);
		pLedCntl->LedPolarity = Value;
	}
}


void RTMPStartLEDMode(IN RTMP_ADAPTER *pAd)
{
}


void RTMPInitLEDMode(IN RTMP_ADAPTER *pAd)
{
	PLED_CONTROL pLedCntl = &pAd->LedCntl;
#if defined(MT7915) || defined(MT7916)
	int i;
	PLED_INIT_TABLE pled_table;

#ifdef MT7915_MT7916_COEXIST_COMPATIBLE

#if defined(MT7916)
	if (IS_MT7916(pAd)) {
		pAd->LedCntl.Led_Init_Ops = led_init_table_mt7916;
	}
#endif
#else
	pAd->LedCntl.Led_Init_Ops = led_init_table;
#endif
	pled_table = pAd->LedCntl.Led_Init_Ops;
#endif

	if (pLedCntl->MCULedCntl.word == 0xFF) {
		pLedCntl->MCULedCntl.word = 0x01;
		pLedCntl->LedAGCfg = 0x5555;
		pLedCntl->LedACTCfg = 0x2221;
#ifdef RTMP_MAC_PCI
		pLedCntl->LedPolarity = 0xA9F8;
#endif /* RTMP_MAC_PCI */
	}

	AsicSendCommandToMcu(pAd, MCU_SET_LED_AG_CFG, 0xff, (UCHAR)pLedCntl->LedAGCfg, (UCHAR)(pLedCntl->LedAGCfg >> 8), FALSE);
	AsicSendCommandToMcu(pAd, MCU_SET_LED_ACT_CFG, 0xff, (UCHAR)pLedCntl->LedACTCfg, (UCHAR)(pLedCntl->LedACTCfg >> 8), FALSE);
	AsicSendCommandToMcu(pAd, MCU_SET_LED_POLARITY, 0xff, (UCHAR)pLedCntl->LedPolarity, (UCHAR)(pLedCntl->LedPolarity >> 8), FALSE);
	AsicSendCommandToMcu(pAd, MCU_SET_LED_GPIO_SIGNAL_CFG, 0xff, 0, pLedCntl->MCULedCntl.field.Polarity, FALSE);
	pAd->LedCntl.LedIndicatorStrength = 0xFF;
	RTMPSetSignalLED(pAd, -100);	/* Force signal strength Led to be turned off, before link up */
	RTMPStartLEDMode(pAd);

#if defined(MT7915) || defined(MT7916)
	/*led init setting*/
	for (i = 0; i < LED_MAX_NUM; i++) {
		if (pled_table[i].gpio_inti_func)
			pled_table[i].gpio_inti_func(pAd, i);
	}
#endif
}


inline void RTMPExitLEDMode(IN RTMP_ADAPTER *pAd)
{
	UCHAR BandIdx;

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++)
		RTMPSetLED(pAd, LED_RADIO_OFF, BandIdx);

#ifdef CONFIG_CPE_SUPPORT
	wps_led_control(pAd, 1);/*turn OFF wps led  HIGH ACTIVE*/
#endif
	return;
}

void wps_led_control(struct _RTMP_ADAPTER *pAd, UCHAR flag)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->wps_led_control)
		ops->wps_led_control(pAd, flag);
	else
		AsicNotSupportFunc(pAd, __func__);
}

#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
INT rtmp_control_led_cmd(
	struct _RTMP_ADAPTER *pAd,
	UCHAR led_idx,
	UCHAR tx_over_blink,
	UCHAR reverse_polarity,
	UCHAR band,
	UCHAR blink_mode,
	UCHAR off_time,
	UCHAR on_time,
	UCHAR led_control_mode){

	INT ret = 0;
	UINT8 finish = 0;
	BOOLEAN Cancelled;

	/*only these mode need timer, so they are handled here. Other modes are still handled in AndesLedEnhanceOP*/
	switch (led_control_mode) {
	/*WPS*/
	case LED_WPS_3_BLINKING_PER_SECOND_FOR_4_SECONDS:
		if(led_idx < 2){
			ret = AndesLedEnhanceOP(pAd, led_idx, tx_over_blink, reverse_polarity, band,
				blink_mode, off_time, on_time, LED_BLINKING_170MS_ON_170MS_OFF);
		}
		pAd->LedCntl.LEDActionType = LED_MODE16_ACTION_1;
		pAd->LedCntl.LEDIndex = led_idx;
		RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 4000);
		finish = 1;
		break;
	case LED_WPS_5S_ON_3S_OFF_THEN_BLINKING:
		if(led_idx < 2){
			ret = AndesLedEnhanceOP(pAd, led_idx, tx_over_blink, reverse_polarity, band,
				blink_mode, off_time, on_time, LED_SOLID_ON);
		}
		pAd->LedCntl.LEDActionType = LED_MODE17_ACTION_1;
		pAd->LedCntl.LEDIndex = led_idx;
		RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 5000);
		finish = 1;
		break;
	case LED_WPS_5S_ON:
		if(led_idx < 2){
			ret = AndesLedEnhanceOP(pAd, led_idx, tx_over_blink, reverse_polarity, band,
				blink_mode, off_time, on_time, LED_SOLID_ON);
		}
		pAd->LedCntl.LEDActionType = LED_MODE18_ACTION_1;
		pAd->LedCntl.LEDIndex = led_idx;
		RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 5000);
		finish = 1;
		break;
	case LED_SOLID_ON:
		if(led_idx==2){
			if(pAd->LedCntl.LEDControlTimerRunning == true){
				RTMPCancelTimer(&pAd->LedCntl.LEDControlTimer,&Cancelled);
				pAd->LedCntl.LEDControlTimerRunning = false;
			}
			if (reverse_polarity)
				wps_led_control(pAd, LED_OFF);
			else
				wps_led_control(pAd, LED_ON);
		}
		break;
	case LED_SOLID_OFF:
		if (led_idx == 2) {
			if (pAd->LedCntl.LEDControlTimerRunning == true) {
				RTMPCancelTimer(&pAd->LedCntl.LEDControlTimer,&Cancelled);
				pAd->LedCntl.LEDControlTimerRunning = false;
			}
			if (reverse_polarity)
				wps_led_control(pAd, LED_ON);
			else
				wps_led_control(pAd, LED_OFF);
		}
		break;
	case LED_BLINKING_500MS_ON_500MS_OFF:
		if(led_idx == 2){
			wps_led_control(pAd, LED_ON);
			pAd->LedCntl.LEDIndex = led_idx;
			pAd->LedCntl.LEDActionType = LED_BLINK_ACTION_500;
			pAd->LedCntl.LEDControlTimerRunning = true;
			RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 500);
		}
		break;
	case LED_BLINKING_1000MS_ON_1000MS_OFF:
		if(led_idx == 2){
			wps_led_control(pAd, LED_ON);
			pAd->LedCntl.LEDIndex = led_idx;
			pAd->LedCntl.LEDActionType = LED_BLINK_ACTION_1000;
			pAd->LedCntl.LEDControlTimerRunning = true;
			RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 1000);
		}
		break;
	case LED_BLINKING_2000MS_ON_2000MS_OFF:
		if(led_idx == 2){
			wps_led_control(pAd, LED_ON);
			pAd->LedCntl.LEDIndex = led_idx;
			pAd->LedCntl.LEDActionType = LED_BLINK_ACTION_2000;
			pAd->LedCntl.LEDControlTimerRunning = true;
			RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 2000);
		}
		break;
	default:
		break;
	}
	if (!finish && (led_idx != 2))
		ret = AndesLedEnhanceOP(pAd, led_idx, tx_over_blink, reverse_polarity, band,
		blink_mode, off_time, on_time, led_control_mode);
	return ret;
}

VOID LEDControlTimer(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;

	switch (pAd->LedCntl.LEDActionType) { /* Last Control LED state. */
	/*Turn off LED*/
	case LED_MODE16_ACTION_1:
		AndesLedEnhanceOP(pAd, pAd->LedCntl.LEDIndex, 0, 0, 0, 0, 0, 0, 1);
		pAd->LedCntl.LEDActionType = LED_CONTROL_SUCCESS;
		break;
	/* Turn off LED for next 3 secs */
	case LED_MODE17_ACTION_1:
		AndesLedEnhanceOP(pAd, pAd->LedCntl.LEDIndex, 0, 0, 0, 0, 0, 0, 1);
		pAd->LedCntl.LEDActionType = LED_MODE17_ACTION_2;
		RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 3000);
		break;
	/* Change to TX Blink mode */
	case LED_MODE17_ACTION_2:
		AndesLedEnhanceOP(pAd, pAd->LedCntl.LEDIndex, 0, 0, 0, 2, 0, 0, 2);
		pAd->LedCntl.LEDActionType = LED_CONTROL_SUCCESS;
		break;
	/* Turn off LED */
	case LED_MODE18_ACTION_1:
		AndesLedEnhanceOP(pAd, pAd->LedCntl.LEDIndex, 0, 0, 0, 0, 0, 0, 1);
		pAd->LedCntl.LEDActionType = LED_CONTROL_SUCCESS;
		break;
	case LED_BLINK_ACTION_500:
		wps_led_control(pAd, LED_REVERSE);
		RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 500);
		break;
	case LED_BLINK_ACTION_1000:
		wps_led_control(pAd, LED_REVERSE);
		RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 1000);
		break;
	case LED_BLINK_ACTION_2000:
		wps_led_control(pAd, LED_REVERSE);
		RTMPSetTimer(&pAd->LedCntl.LEDControlTimer, 2000);
		break;
	default:
		/* do nothing. */
		break;
	}
}
#endif

#endif /* LED_CONTROL_SUPPORT */


INT RTMPSetLED(RTMP_ADAPTER *pAd, UCHAR Status, UCHAR BandIdx)
{
#ifdef RTMP_MAC_PCI

	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
#ifdef LED_CONTROL_SUPPORT
		RTMPSetLEDStatus(pAd, Status, BandIdx);
#endif /* LED_CONTROL_SUPPORT */
	}

#endif /* RTMP_MAC_PCI */
	return TRUE;
}

void LED_WLAN_2G_init(RTMP_ADAPTER *pAd, UINT8 led_index)
{
	PLED_INIT_TABLE pled_table = pAd->LedCntl.Led_Init_Ops;

	RTMP_LED_GPIO_MAP(pAd, led_index, pled_table[led_index].map_idx, pled_table[led_index].control_type);
	/*for others init, TBD*/
}

void LED_WLAN_5G_init(RTMP_ADAPTER *pAd, UINT8 led_index)
{
	PLED_INIT_TABLE pled_table = pAd->LedCntl.Led_Init_Ops;

	RTMP_LED_GPIO_MAP(pAd, led_index, pled_table[led_index].map_idx, pled_table[led_index].control_type);
	/*for others init, TBD*/
}

void LED_WLAN_WPS_init(RTMP_ADAPTER *pAd, UINT8 led_index)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->wps_led_init)
		ops->wps_led_init(pAd);
	else
		AsicNotSupportFunc(pAd, __func__);
}

