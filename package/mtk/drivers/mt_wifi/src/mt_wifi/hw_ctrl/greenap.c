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
/*
 ***************************************************************************
 ***************************************************************************

	Module Name:
	greenap.c

	Abstract:
	Ralink Wireless driver green ap related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "rt_config.h"

#ifdef GREENAP_SUPPORT
/*
 *
 */
VOID greenap_init(RTMP_ADAPTER *ad)
{
	struct greenap_ctrl *greenap = &ad->ApCfg.greenap;

	if (!greenap)
		return;

	greenap->cap = FALSE;
	greenap->allow = FALSE;
	RTMP_SPIN_LOCK(&greenap->lock);
	greenap->suspend = 0;
	RTMP_SPIN_UNLOCK(&greenap->lock);
}

/*
 *  update greenap on/off immediately
 */
static VOID greenap_update(
	RTMP_ADAPTER *ad,
	BOOLEAN previous_greenap_active,
	BOOLEAN greenap_enter,
	UINT8 band_idx)
{
	struct greenap_on_off_ctrl greenap_on_off = {0};

	greenap_on_off.band_idx = band_idx;

	if (previous_greenap_active) {
		if (greenap_enter) {
			/* Do nothing */
		} else {
			/* Exit GreenAP */
			RTMP_CHIP_DISABLE_GREENAP(ad, &greenap_on_off);
		}
	} else {
		if (greenap_enter) {
			/* Enter GreenAP */
			RTMP_CHIP_ENABLE_GREENAP(ad, &greenap_on_off);
		} else {
			/* Do nothing */
		}
	}
}

/*
 * green ap check rule:
 *   1. peer STA need 1 rx stream, check BF status
 *      a. AP_eBF=1 && STA_eBF=1: green ap off
 *      b. AP_iBF=1: green ap off
 *      c. else case: green ap on
 *   2. peer STA need > 1 rx stream, green ap off
 *   3. only single band AP mode or dbdc AP+AP mode support green ap
 *
 */
#ifdef TXBF_SUPPORT
/*
 *
 */
static BOOLEAN greenap_ap_ebf_cap(RTMP_ADAPTER *ad)
{
	return ad->CommonCfg.ETxBfEnCond ? TRUE : FALSE;
}

/*
 *
 */
static BOOLEAN greenap_ap_ibf_cap(RTMP_ADAPTER *ad)
{
	return ad->CommonCfg.RegTransmitSetting.field.ITxBfEn ? TRUE : FALSE;
}

/*
 *
 */
static BOOLEAN greenap_sta_ebf_cap(MAC_TABLE_ENTRY *entry)
{
	return IS_ETXBF_SUP(entry->rStaRecBf.u1TxBfCap) ? TRUE : FALSE;
}

/*
 *
 */
static BOOLEAN greenap_bf_check(
	RTMP_ADAPTER *ad,
	MAC_TABLE_ENTRY *entry)
{
	BOOLEAN enetr_greenap = TRUE;

	if ((greenap_ap_ebf_cap(ad) && greenap_sta_ebf_cap(entry)) ||
		 greenap_ap_ibf_cap(ad))
		enetr_greenap = FALSE;
	else
		enetr_greenap = TRUE;

	return enetr_greenap;
}
#endif /* TXBF_SUPPORT */

/*
 * B/G mode
 *
 */
static BOOLEAN greenap_rule_1(
	RTMP_ADAPTER *ad,
	MAC_TABLE_ENTRY *entry)
{
	BOOLEAN enetr_greenap;

	if ((entry->MaxHTPhyMode.field.MODE == MODE_CCK) ||
		(entry->MaxHTPhyMode.field.MODE == MODE_OFDM)) {
#ifdef TXBF_SUPPORT
		enetr_greenap = greenap_bf_check(ad, entry) ? TRUE : FALSE;
#else
		enetr_greenap = TRUE;
#endif /* TXBF_SUPPORT */

	} else
		enetr_greenap = FALSE;

	MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "enetr_greenap(%d),AP_e_i_BF(%d,%d),STA_eBF(%d)B/G(%d,%d)\n",
			  enetr_greenap,
#ifdef TXBF_SUPPORT
			  greenap_ap_ebf_cap(ad),
			  greenap_ap_ibf_cap(ad),
			  greenap_sta_ebf_cap(entry),
#else
			  0,
			  0,
			  0,
#endif /* TXBF_SUPPORT */
			  (entry->MaxHTPhyMode.field.MODE == MODE_CCK) ? TRUE : FALSE,
			  (entry->MaxHTPhyMode.field.MODE == MODE_OFDM) ? TRUE : FALSE);
	return enetr_greenap;
}

/*
 * HT mode
 *
 */
static BOOLEAN greenap_rule_2(
	RTMP_ADAPTER *ad,
	MAC_TABLE_ENTRY *entry)
{
	BOOLEAN enetr_greenap;

	if ((entry->MaxHTPhyMode.field.MODE == MODE_HTMIX) ||
		(entry->MaxHTPhyMode.field.MODE == MODE_HTGREENFIELD)) {
		if (entry->SupportHTMCS <= ((1 << MCS_8) - 1)) {
#ifdef TXBF_SUPPORT
			enetr_greenap = greenap_bf_check(ad, entry) ? TRUE : FALSE;
#else
			enetr_greenap = TRUE;
#endif /* TXBF_SUPPORT */
		} else
			enetr_greenap = FALSE;
	} else
		enetr_greenap = FALSE;

	MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "enetr_greenap(%d),AP_e_i_BF(%d,%d),STA_eBF(%d)HT(%d)MSC(0x%x)\n",
			  enetr_greenap,
#ifdef TXBF_SUPPORT
			  greenap_ap_ebf_cap(ad),
			  greenap_ap_ibf_cap(ad),
			  greenap_sta_ebf_cap(entry),
#else
			  0,
			  0,
			  0,
#endif /* TXBF_SUPPORT */
			  ((entry->MaxHTPhyMode.field.MODE == MODE_HTMIX) ||
			   (entry->MaxHTPhyMode.field.MODE == MODE_HTGREENFIELD)) ? TRUE : FALSE,
			  ((entry->MaxHTPhyMode.field.MODE == MODE_HTMIX) ||
			   (entry->MaxHTPhyMode.field.MODE == MODE_HTGREENFIELD)) ? entry->SupportHTMCS : 0);
	return enetr_greenap;
}

/*
 * VHT mode
 *
 */
static BOOLEAN greenap_rule_3(
	RTMP_ADAPTER *ad,
	MAC_TABLE_ENTRY *entry)
{
	BOOLEAN enetr_greenap;

#ifdef DOT11_VHT_AC
	if (entry->MaxHTPhyMode.field.MODE == MODE_VHT) {
		if ((entry->SupportVHTMCS1SS != 0)
			&& (entry->SupportVHTMCS2SS == 0)
			&& (entry->SupportVHTMCS3SS == 0)
			&& (entry->SupportVHTMCS4SS == 0)) {
#ifdef TXBF_SUPPORT
			enetr_greenap = greenap_bf_check(ad, entry) ? TRUE : FALSE;
#else
			enetr_greenap = TRUE;
#endif /* TXBF_SUPPORT */

			if ((entry->vht_cap_ie.vht_cap.ch_width == 1) || (entry->vht_cap_ie.vht_cap.ch_width == 2))
				enetr_greenap = FALSE;

		} else
			enetr_greenap = FALSE;
	} else
		enetr_greenap = FALSE;
#else
	enetr_greenap = FALSE;
#endif /* DOT11_VHT_AC */

	MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "enetr_greenap(%d),AP_e_i_BF(%d,%d),STA_eBF(%d)VHT(%d)MCS(0x%x,0x%x,0x%x,0x%x)\n",
			  enetr_greenap,
#ifdef TXBF_SUPPORT
			  greenap_ap_ebf_cap(ad),
			  greenap_ap_ibf_cap(ad),
			  greenap_sta_ebf_cap(entry),
#else
			  0,
			  0,
			  0,
#endif /* TXBF_SUPPORT */
			  (entry->MaxHTPhyMode.field.MODE == MODE_VHT) ? TRUE : FALSE,
#ifdef DOT11_VHT_AC
			  entry->SupportVHTMCS1SS,
			  entry->SupportVHTMCS2SS,
			  entry->SupportVHTMCS3SS,
			  entry->SupportVHTMCS4SS
#else
			  0,
			  0,
			  0,
			  0
#endif /* DOT11_VHT_AC */
			 );
	return enetr_greenap;
}

/*
 * HE mode
 *
 */
static BOOLEAN greenap_rule_4(
	RTMP_ADAPTER *ad,
	MAC_TABLE_ENTRY *entry)
{
	BOOLEAN enetr_greenap;

#ifdef DOT11_HE_AX
	if (entry->MaxHTPhyMode.field.MODE == MODE_HE) {
		if ((entry->cap.rate.he80_tx_nss_mcs[0] != 3)
			&& (entry->cap.rate.he80_tx_nss_mcs[1] == 3)
			&& (entry->cap.rate.he80_tx_nss_mcs[2] == 3)
			&& (entry->cap.rate.he80_tx_nss_mcs[3] == 3)
			&& !(wlan_config_get_mu_ul_mimo(entry->wdev) && (entry->cap.he_phy_cap & HE_FULL_BW_UL_MU_MIMO))) {
#ifdef TXBF_SUPPORT
			enetr_greenap = greenap_bf_check(ad, entry) ? TRUE : FALSE;
#else
			enetr_greenap = TRUE;
#endif /* TXBF_SUPPORT */
		} else
			enetr_greenap = FALSE;
	} else
		enetr_greenap = FALSE;
#else
	enetr_greenap = FALSE;
#endif /* DOT11_HE_AX */

	MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "enetr_greenap(%d),AP_e_i_BF(%d,%d),STA_eBF(%d)HE(%d)MCS(0x%x,0x%x,0x%x,0x%x),mu_ul_mimo:%x,peer_mu_ul_mimo:%x\n",
			  enetr_greenap,
#ifdef TXBF_SUPPORT
			  greenap_ap_ebf_cap(ad),
			  greenap_ap_ibf_cap(ad),
			  greenap_sta_ebf_cap(entry),
#else
			  0,
			  0,
			  0,
#endif /* TXBF_SUPPORT */
			  (entry->MaxHTPhyMode.field.MODE == MODE_HE) ? TRUE : FALSE,
#ifdef DOT11_HE_AX
			  entry->cap.rate.he80_tx_nss_mcs[0],
			  entry->cap.rate.he80_tx_nss_mcs[1],
			  entry->cap.rate.he80_tx_nss_mcs[2],
			  entry->cap.rate.he80_tx_nss_mcs[3],
			  wlan_config_get_mu_ul_mimo(entry->wdev),
			  (entry->cap.he_phy_cap & HE_FULL_BW_UL_MU_MIMO)
#else
			  0,
			  0,
			  0,
			  0,
			  0,
			  0
#endif /* DOT11_HE_AX */
		 );
	return enetr_greenap;
}

/*
 *
 */
static BOOLEAN greenap_rule_check(
	RTMP_ADAPTER *ad,
	MAC_TABLE_ENTRY *entry)
{
	BOOLEAN enetr_greenap = FALSE;

	if (entry == NULL)
		return FALSE;

	enetr_greenap |= greenap_rule_1(ad, entry);
	enetr_greenap |= greenap_rule_2(ad, entry);
	enetr_greenap |= greenap_rule_3(ad, entry);
	enetr_greenap |= greenap_rule_4(ad, entry);
	return enetr_greenap;
}

/*
 *
 */
VOID greenap_set_capability(RTMP_ADAPTER *ad, BOOLEAN greenap_cap)
{
	struct greenap_ctrl *greenap = &ad->ApCfg.greenap;

	if (!greenap)
		return;

	greenap->cap = greenap_cap;
}

/*
 *
 */
BOOLEAN greenap_get_capability(RTMP_ADAPTER *ad)
{
	struct greenap_ctrl *greenap = &ad->ApCfg.greenap;

	if (!greenap)
		return FALSE;
	else
		return greenap->cap;
}

/*
 *
 */
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_WPA3_SUPPORT)
BOOLEAN greenap_get_allow_status(RTMP_ADAPTER *ad)
#else
static BOOLEAN greenap_get_allow_status(RTMP_ADAPTER *ad)
#endif
{
	BOOLEAN ap_mode_if_up = FALSE;
	BOOLEAN non_ap_mode_if_up = FALSE;
	UCHAR i = 0;
	struct wifi_dev *wdev = NULL;
	struct greenap_ctrl *greenap = &ad->ApCfg.greenap;

	if (!greenap)
		return FALSE;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = ad->wdev_list[i];

		if (!wdev)
			continue;

		if ((wdev->wdev_type == WDEV_TYPE_AP) && wdev->if_up_down_state) {
			ap_mode_if_up = TRUE;
			break;
		}
	}

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = ad->wdev_list[i];

		if (wdev == NULL)
			continue;

		if ((wdev->wdev_type != WDEV_TYPE_AP) && wdev->if_up_down_state) {
			non_ap_mode_if_up = TRUE;
			break;
		}
	}

	greenap->allow = (ap_mode_if_up & (!non_ap_mode_if_up));

	MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "ap_mode_if_up(%d), non_ap_mode_if_up(%d) --> greenap_allow(%d)\n",
			  ap_mode_if_up,
			  non_ap_mode_if_up,
			  greenap->allow);

	return greenap->allow;
}

/*
*   For existed connection and update GreenAP state
*/
static VOID greenap_check_peer_connection_status(
	RTMP_ADAPTER *ad,
	UINT8 band_idx,
	BOOLEAN previous_greenap_active,
	BOOLEAN greenap_allow)
{
	BOOLEAN greenap_enter = TRUE;
	UINT16 i = 0;
	MAC_TABLE_ENTRY *entry = NULL;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(ad);

	for (i = 0; i < wtbl_max_num; i++) {
		entry = &ad->MacTab.Content[i];

		if ((IS_ENTRY_CLIENT(entry)) &&
			(entry->ConnectionType == CONNECTION_INFRA_STA) &&
			(HcGetBandByWdev(entry->wdev) == band_idx)) {
			if (greenap_rule_check(ad, entry))
				greenap_enter = TRUE;
			else {
				greenap_enter = FALSE;
				break;
			}
		} else {
			/* Do nothing and continue next entry */
		}
	}

	MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "band(%d), previous_greenap_active(%d),  greenap_allow(%d), greenap_enter(%d)\n",
			  band_idx,
			  previous_greenap_active,
			  greenap_allow,
			  greenap_enter);
	greenap_update(ad, previous_greenap_active, (greenap_allow & greenap_enter), band_idx);
}

/*
*
*/
static BOOLEAN greenap_get_suspend_status(struct greenap_ctrl *greenap)
{
	return (greenap->suspend != 0) ? TRUE : FALSE;
}

/*
 *
 */
VOID greenap_suspend(RTMP_ADAPTER *ad, UINT32 reason)
{
	UCHAR band_idx = 0;
	UCHAR amount_of_band = HcGetAmountOfBand(ad);
	BOOLEAN greenap_cap = greenap_get_capability(ad);
	struct greenap_ctrl *greenap = &ad->ApCfg.greenap;

	if (!greenap)
		return;

	MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "reason(0x%x)\n",
			 greenap_cap ? reason : GREENAP_REASON_NONE);

	if (greenap_cap) {
		RTMP_SPIN_LOCK(&greenap->lock);
		greenap->suspend |= reason;
		MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "greenap->suspend(0x%x)\n",
				  greenap->suspend);

		for (band_idx = 0; band_idx < amount_of_band; band_idx++)
			greenap_update(ad, IsHcGreenAPActiveByBand(ad, band_idx), FALSE, band_idx);

		RTMP_SPIN_UNLOCK(&greenap->lock);
	}
}

/*
 *
 */
VOID greenap_resume(RTMP_ADAPTER *ad, UINT32 reason)
{
	UCHAR band_idx = 0;
	UCHAR amount_of_band = HcGetAmountOfBand(ad);
	BOOLEAN greenap_cap = greenap_get_capability(ad);
	struct greenap_ctrl *greenap = &ad->ApCfg.greenap;

	if (!greenap)
		return;

	MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "reason(0x%x)\n",
			  greenap_cap ? reason : GREENAP_REASON_NONE);
	/* During scan, ifreface might be add or delete, check allow here again */
	/* Check all entry if meet GreenAP mode */
	if (greenap_cap) {
		RTMP_SPIN_LOCK(&greenap->lock);
		for (band_idx = 0; band_idx < amount_of_band; band_idx++) {
			greenap_check_peer_connection_status(
				ad,
				band_idx,
				IsHcGreenAPActiveByBand(ad, band_idx),
				greenap_get_allow_status(ad));
		}

		greenap->suspend &= ~reason;
		MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "greenap->suspend(0x%x)\n",
				  greenap->suspend);
		RTMP_SPIN_UNLOCK(&greenap->lock);
	}
}

/*
 *
 */
static BOOLEAN greenap_check_for_all_bands(RTMP_ADAPTER *ad)
{
	UCHAR band_idx = 0;
	UCHAR amount_of_band = HcGetAmountOfBand(ad);
	struct greenap_ctrl *greenap = &ad->ApCfg.greenap;

	if (!greenap)
		return FALSE;

	if (greenap_get_capability(ad)) {
		RTMP_SPIN_LOCK(&greenap->lock);

		if (greenap_get_suspend_status(greenap)) {
			RTMP_SPIN_UNLOCK(&greenap->lock);
			return TRUE;
		}

		MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

		for (band_idx = 0; band_idx < amount_of_band; band_idx++) {
			greenap_check_peer_connection_status(
				ad,
				band_idx,
				IsHcGreenAPActiveByBand(ad, band_idx),
				greenap_get_allow_status(ad));
		}

		RTMP_SPIN_UNLOCK(&greenap->lock);
	}

	return TRUE;
}

/*
 *
 */
BOOLEAN greenap_check_when_if_down_up(RTMP_ADAPTER *ad)
{
	MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"greenap_cap=%d\n",
		greenap_get_capability(ad) ? TRUE : FALSE);

	return greenap_check_for_all_bands(ad);
}

/*
 *
 */
BOOLEAN greenap_check_when_ap_bss_change(RTMP_ADAPTER *ad)
{
	MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"greenap_cap=%d\n",
		greenap_get_capability(ad) ? TRUE : FALSE);

	return greenap_check_for_all_bands(ad);
}

/*
 *
 */
VOID greenap_check_peer_connection_at_link_up_down(
	RTMP_ADAPTER *ad,
	struct wifi_dev *wdev)
{
	UCHAR   band_idx = HcGetBandByWdev(wdev);
	BOOLEAN previous_greenap_active = IsHcGreenAPActiveByWdev(wdev);
	BOOLEAN greenap_cap = greenap_get_capability(ad);
	struct greenap_ctrl *greenap = &ad->ApCfg.greenap;

	if (!greenap)
		return;

	MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"greenap_cap=%d\n",
		greenap_cap ? TRUE : FALSE);

	if (greenap_cap) {
		RTMP_SPIN_LOCK(&greenap->lock);

		if (greenap_get_suspend_status(greenap)) {
			RTMP_SPIN_UNLOCK(&greenap->lock);
			return;
		}

		greenap_check_peer_connection_status(
			ad,
			band_idx,
			previous_greenap_active,
			greenap_get_allow_status(ad));
		RTMP_SPIN_UNLOCK(&greenap->lock);
	}
}

/*
 *
 */
VOID greenap_proc(RTMP_ADAPTER *ad, BOOLEAN greenap_cap_on)
{
	UCHAR band_idx = 0;
	UCHAR amount_of_band = HcGetAmountOfBand(ad);
	struct greenap_ctrl *greenap = &ad->ApCfg.greenap;

	if (!greenap)
		return;

	RTMP_SPIN_LOCK(&greenap->lock);

	if (greenap_get_suspend_status(greenap)) {
		RTMP_SPIN_UNLOCK(&greenap->lock);
		return;
	}

	if (!greenap_cap_on) {
		greenap_set_capability(ad, FALSE);

		for (band_idx = 0; band_idx < amount_of_band; band_idx++)
			greenap_update(ad, IsHcGreenAPActiveByBand(ad, band_idx), FALSE, band_idx);
	} else if (greenap_cap_on) {
		greenap_set_capability(ad, TRUE);
		/* use case: No GreeAP on --> sta connected --> GreenAP on */
		for (band_idx = 0; band_idx < amount_of_band; band_idx++) {
			greenap_check_peer_connection_status(
				ad,
				band_idx,
				IsHcGreenAPActiveByBand(ad, band_idx),
				greenap_get_allow_status(ad));
		}
	}

	RTMP_SPIN_UNLOCK(&greenap->lock);
}

/*
 *
 */
VOID enable_greenap(RTMP_ADAPTER *ad, struct greenap_on_off_ctrl *greenap_on_off)
{
	struct greenap_on_off_ctrl *greenap = (struct greenap_on_off_ctrl *)greenap_on_off;

	MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx(%d), enable_greenap\n", greenap->band_idx);
	HcSetGreenAPActiveByBand(ad, greenap->band_idx, TRUE);
	RTMP_GREENAP_ON_OFF_CTRL(ad, greenap->band_idx, TRUE);
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	mt_asic_pcie_aspm_dym_ctrl(ad, greenap->band_idx, TRUE, FALSE);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
}

/*
*
*/
VOID disable_greenap(RTMP_ADAPTER *ad, struct greenap_on_off_ctrl *greenap_on_off)
{
	struct greenap_on_off_ctrl *greenap = (struct greenap_on_off_ctrl *)greenap_on_off;

	MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx(%d), disable_greenap\n", greenap->band_idx);
	HcSetGreenAPActiveByBand(ad, greenap->band_idx, FALSE);
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	mt_asic_pcie_aspm_dym_ctrl(ad, greenap->band_idx, FALSE, FALSE);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
	RTMP_GREENAP_ON_OFF_CTRL(ad, greenap->band_idx, FALSE);
}

/*
 *
 */
VOID greenap_show(RTMP_ADAPTER *ad)
{
	UINT32  value = 0;
	UCHAR i = 0;
	struct wifi_dev *wdev = NULL;
	UCHAR band_idx = 0;
	UCHAR amount_of_band = HcGetAmountOfBand(ad);
	struct greenap_ctrl *greenap = &ad->ApCfg.greenap;

	if (!greenap)
		return;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = ad->wdev_list[i];

		if (!wdev)
			continue;

		MTWF_DBG(ad, DBG_CAT_HW, CATHW_GREENAP, DBG_LVL_INFO,
				 "\tband(%d), wdev[%d], type(0x%x), up(%d)\n",
				  HcGetBandByWdev(wdev),
				  i,
				  wdev->wdev_type,
				  wdev->if_up_down_state);
	}

	MTWF_DBG(ad, DBG_CAT_HW, CATHW_GREENAP, DBG_LVL_INFO,
			 "\tGREENAP::greenap_cap(%d), greenap_allow(%d), dbdc_mode(%d)\n",
			  greenap_get_capability(ad),
			  greenap_get_allow_status(ad),
			  ad->CommonCfg.dbdc_mode);

	for (band_idx = 0; band_idx < amount_of_band; band_idx++) {
		MAC_IO_READ32(ad->hdev_ctrl, ((band_idx == 0) ? RMAC_RMCR : RMAC_RMCR_BAND_1), &value);
		MTWF_DBG(ad, DBG_CAT_HW, CATHW_GREENAP, DBG_LVL_INFO,
				 "\tGREENAP::band_idx(%d), greenap_active(%d)\n",
				  band_idx,
				  IsHcGreenAPActiveByBand(ad, band_idx));
	}
}

VOID EnableAPMIMOPSv2(RTMP_ADAPTER *ad, struct greenap_on_off_ctrl *greenap_on_off)
{
	struct greenap_on_off_ctrl *greenap = (struct greenap_on_off_ctrl *)greenap_on_off;

	bbp_set_mmps(ad, greenap->reduce_core_power);
	HcSetGreenAPActiveByBand(ad, 0, TRUE);
	MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "EnableAPMIMOPSNew, 30xx changes the # of antenna to 1\n");
}

VOID DisableAPMIMOPSv2(RTMP_ADAPTER *ad, struct greenap_on_off_ctrl *greenap_on_off)
{
	bbp_set_mmps(ad, FALSE);
	HcSetGreenAPActiveByBand(ad, 0, FALSE);
	MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "DisableAPMIMOPSNew, 30xx reserve only one antenna\n");
}

VOID EnableAPMIMOPSv1(
	IN RTMP_ADAPTER *ad,
	IN struct greenap_on_off_ctrl *greenap_on_off)
{
	ULONG TxPinCfg = 0x00050F0A;/*Gary 2007/08/09 0x050A0A*/
	UINT8 TxPath = ad->Antenna.field.TxPath;
	UINT8 RxPath = ad->Antenna.field.RxPath;
	struct wifi_dev *wdev = get_default_wdev(ad);

	if (wdev == NULL) {
		MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "get_default_wdev fail!!!\n");
		return;
	}

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		UINT8 BandIdx = HcGetBandByWdev(wdev);
		if (ad->bAntennaSetAPEnable[BandIdx]) {
			TxPath = ad->TxStream[BandIdx];
			RxPath = ad->RxStream[BandIdx];
		}
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

	if (wdev->channel > 14)
		TxPinCfg = 0x00050F05;

	TxPinCfg &= 0xFFFFFFF3;
	TxPinCfg &= 0xFFFFF3FF;
	HcSetGreenAPActiveByBand(ad, 0, TRUE);
	MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Run with BW_20\n");
	/* Set BBP registers to BW20 */
	wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);

	/* RF Bandwidth related registers would be set in AsicSwitchChannel() */
	if (TxPath > 1 || RxPath > 1) {
		/*Tx/Rx Stream*/
		bbp_set_txdac(ad, 0);
		bbp_set_rxpath(ad, 1);
		/* Need to check in RT chip */
		/*RTMP_IO_WRITE32(ad->hdev_ctrl, TX_PIN_CFG, TxPinCfg);*/
	}

	MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "EnableAPMIMOPS, 305x/28xx changes the # of antenna to 1\n");
}

VOID DisableAPMIMOPSv1(
	IN PRTMP_ADAPTER		ad,
	IN struct greenap_on_off_ctrl *greenap_on_off)
{
	UCHAR ext_cha = 0;
	UCHAR ht_bw = 0;
	UINT8 TxPath = ad->Antenna.field.TxPath;
	UINT8 RxPath = ad->Antenna.field.RxPath;
	struct wifi_dev *wdev = get_default_wdev(ad);

	if (wdev == NULL) {
		MTWF_DBG(ad, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"get_default_wdev fail!!!\n");
		return;
	}
	ext_cha = wlan_operate_get_ext_cha(wdev);
	ht_bw = wlan_config_get_ht_bw(wdev);

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		UINT8 BandIdx = HcGetBandByWdev(wdev);
		if (ad->bAntennaSetAPEnable[BandIdx]) {
			TxPath = ad->TxStream[BandIdx];
			RxPath = ad->RxStream[BandIdx];
		}
	}
#endif /* ANTENNA_CONTROL_SUPPORT */
	HcSetGreenAPActiveByBand(ad, 0, FALSE);

	if ((ht_bw == BW_40) && (wdev->channel != 14)) {
		MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Run with BW_40\n");

		/* Set CentralChannel to work for BW40 */
		if (ext_cha == EXTCHA_ABOVE) {
			ext_cha = EXTCHA_ABOVE;
		} else if ((wdev->channel > 2) && (ext_cha == EXTCHA_BELOW)) {
			ext_cha = EXTCHA_BELOW;
		}

		wlan_operate_set_prim_ch(wdev, wdev->channel);
		wlan_operate_set_ht_bw(wdev, ht_bw, ext_cha);
	}

	/*Tx Stream*/
	if (WMODE_CAP_N(wdev->PhyMode) && (TxPath == 2))
		bbp_set_txdac(ad, 2);
	else
		bbp_set_txdac(ad, 0);

	/*Rx Stream*/
	bbp_set_rxpath(ad, RxPath);
	MTWF_DBG(NULL, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "DisableAPMIMOPS, 305x/28xx reserve only one antenna\n");
}
#endif /* GREENAP_SUPPORT */
