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

/*
* private structure definition to prevent direct access
*/
static VOID wlan_config_init(struct wlan_config *obj)
{
	phy_cfg_init(&obj->phy_conf);
	ht_cfg_init(&obj->ht_conf);
#ifdef DOT11_VHT_AC
	vht_cfg_init(&obj->vht_conf);
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
	he_cfg_init(&obj->he_conf);
#endif
}

static VOID wlan_config_exit(struct wlan_config *obj)
{
	phy_cfg_exit(&obj->phy_conf);
	ht_cfg_exit(&obj->ht_conf);
#ifdef DOT11_VHT_AC
	vht_cfg_exit(&obj->vht_conf);
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
	he_cfg_exit(&obj->he_conf);
#endif
}

enum ASIC_CAP wlan_config_get_asic_caps(struct wifi_dev *wdev)
{
	VOID *hdev_ctrl = hc_get_hdev_ctrl(wdev);
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(hdev_ctrl);

	return chip_cap->asic_caps;
}

enum PHY_CAP wlan_config_get_phy_caps(struct wifi_dev *wdev)
{
	VOID *hdev_ctrl = hc_get_hdev_ctrl(wdev);
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(hdev_ctrl);

	return chip_cap->phy_caps;
}

VOID *wlan_config_get_ppdu_caps(struct wifi_dev *wdev)
{
	VOID *hdev_ctrl = hc_get_hdev_ctrl(wdev);
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(hdev_ctrl);

	return &chip_cap->ppdu;
}

VOID *wlan_config_get_mcs_nss_caps(struct wifi_dev *wdev)
{
	VOID *hdev_ctrl = hc_get_hdev_ctrl(wdev);
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(hdev_ctrl);

	return &chip_cap->mcs_nss;
}

VOID *wlan_config_get_qos_caps(struct wifi_dev *wdev)
{
	VOID *hdev_ctrl = hc_get_hdev_ctrl(wdev);
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(hdev_ctrl);

	return &chip_cap->qos;
}

VOID *wlan_config_get_chip_caps(struct wifi_dev *wdev)
{
	VOID *hdev_ctrl = hc_get_hdev_ctrl(wdev);

	return hc_get_chip_cap(hdev_ctrl);
}

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
BOOLEAN wlan_config_get_asic_twt_caps(struct wifi_dev *wdev)
{
	VOID *hdev_ctrl = hc_get_hdev_ctrl(wdev);
	UINT_32 asic_cap = hc_get_asic_cap(hdev_ctrl);

	return (asic_cap & fASIC_CAP_TWT) ? TRUE : FALSE;
}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

static INT32 wpf_config_acquire(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	int i;
	struct wpf_ctrl *ctrl = &ad->wpf;
	struct wpf_data *pf;

	if (wdev->wpf_cfg || wdev->wpf_op)
		return 0;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		pf = &ctrl->pf[i];

		if (pf->dev == NULL) {
			wdev->wpf_cfg = pf->conf;
			wdev->wpf_op = pf->oper;
			pf->dev = wdev;
			return 0;
		}
	}

	return -1;
}

static INT32 wpf_config_release(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	int i;
	struct wpf_ctrl *ctrl = &ad->wpf;
	struct wpf_data *pf;

	if ((wdev->wpf_cfg == NULL) && (wdev->wpf_op == NULL))
		return 0;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		pf = &ctrl->pf[i];

		if (pf->dev == wdev) {
			pf->dev = NULL;
			break;
		}
	}

	wdev->wpf_cfg = NULL;
	wdev->wpf_op = NULL;
	return 0;
}

VOID wpf_config_init(struct _RTMP_ADAPTER *ad)
{
	int i;
	struct wpf_ctrl *ctrl = &ad->wpf;
	struct wpf_data *pf;

	os_zero_mem(ctrl->pf, sizeof(ctrl->pf));

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		pf = &ctrl->pf[i];
		pf->idx = i;
		os_alloc_mem(NULL, (UCHAR **)&pf->conf, sizeof(struct wlan_config));

		if (pf->conf) {
			os_zero_mem(pf->conf, sizeof(struct wlan_config));
			wlan_config_init(pf->conf);
		}

		os_alloc_mem(NULL, (UCHAR **)&pf->oper, sizeof(struct wlan_operate));

		if (pf->oper)
			os_zero_mem(pf->oper, sizeof(struct wlan_operate));
	}
}

VOID wpf_config_exit(struct _RTMP_ADAPTER *ad)
{
	int i;
	struct wpf_ctrl *ctrl = &ad->wpf;
	struct wpf_data *pf;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		pf = &ctrl->pf[i];
		pf->idx = i;

		if (pf->conf) {
			wlan_config_exit(pf->conf);
			os_free_mem(pf->conf);
		}

		pf->conf = NULL;

		if (pf->oper)
			os_free_mem(pf->oper);

		pf->oper = NULL;
	}

	os_zero_mem(ctrl->pf, sizeof(ctrl->pf));
}

/*
* assign order: ( MBSS | STA ) >  WDS  > APCLI > P2P > MESH > SERVICE > ATE
*/
VOID wpf_init(struct _RTMP_ADAPTER *ad)
{
	struct wifi_dev *wdev;
	int i;
	/*do not change order*/
#ifdef CONFIG_AP_SUPPORT
	/* snowpin for ap/sta IF_DEV_CONFIG_OPMODE_ON_AP(ad) */
	{
		for (i = 0; i < MAX_MBSSID_NUM(ad); i++) {
			wdev = &ad->ApCfg.MBSSID[i].wdev;

			if (wpf_config_acquire(ad, wdev) < 0) {
				MTWF_DBG(ad, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "[ERROR] wdev_cfg is full!\n");
				return;
			}
		}

#ifdef WDS_SUPPORT

		for (i = 0; i < MAX_WDS_ENTRY; i++) {
			wdev = &ad->WdsTab.WdsEntry[i].wdev;

			if (wpf_config_acquire(ad, wdev) < 0) {
				MTWF_DBG(ad, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "[ERROR] wdev_cfg is full!\n");
				return;
			}
		}

#endif /*WDS_SUPPORT*/
#ifdef APCLI_SUPPORT

		for (i = 0; i < MAX_APCLI_NUM; i++) {
			wdev = &ad->StaCfg[i].wdev;

			if (wpf_config_acquire(ad, wdev) < 0) {
				MTWF_DBG(ad, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "[ERROR] wdev_cfg is full!\n");
				return;
			}
		}

#endif /*APCLI_SUPPORT*/
#ifdef SNIFFER_SUPPORT
		for (i = 0; i < MONITOR_MAX_DEV_NUM; i++) {
			wdev = &ad->monitor_ctrl.wdev;
			if (wpf_config_acquire(ad, wdev) < 0) {
				MTWF_DBG(ad, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "[ERROR] wdev_cfg is full!\n");
				return;
			}
		}
#endif

	}
#endif /*CONFIG_AP_SUPPORT*/
#ifdef CONFIG_STA_SUPPORT
	/* snowpin for ap/sta IF_DEV_CONFIG_OPMODE_ON_STA(ad) */
	{
		for (i = 0; i < MAX_MULTI_STA; i++) {
			wdev = &ad->StaCfg[i].wdev;

			if (wpf_config_acquire(ad, wdev) < 0) {
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "[ERROR] wdev_cfg is full!\n");
				return;
			}
		}
	}
#endif /*CONFIG_STA_SUPPORT*/

#ifdef CONFIG_WLAN_SERVICE
	{
		UCHAR band_idx, wmm_idx = 0;

		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s: acquiring SERVICE wdev_cfg!\n", __func__);

		for (band_idx = DBDC_BAND0;
			band_idx < DBDC_BAND_NUM; band_idx++) {
			for (wmm_idx = 0 ; wmm_idx < 2 ; wmm_idx++) {
				wdev = &ad->ate_wdev[band_idx][wmm_idx];

				if (wpf_config_acquire(ad, wdev) < 0)
					MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"[ERROR] wdev_cfg is full!\n");
				else
					MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"%s:acquiring DBDC/wdev_cfg for band%d!\n",
						__func__, band_idx);
			}
		}
	}
#else
#ifdef CONFIG_ATE
	{
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: Acquiring DBDC/ATE wdev_cfg!\n", __func__);

		for (i = 0; i < 2; i++) {
			if (ad->ATECtrl.wdev[i] == NULL) {
				os_alloc_mem(ad, (PUCHAR *)&ad->ATECtrl.wdev[i], sizeof(struct wifi_dev));
				wdev = ad->ATECtrl.wdev[i];

				if (wdev != NULL) {
					NdisZeroMemory(wdev, sizeof(struct wifi_dev));

					if (wpf_config_acquire(ad, wdev) < 0)
							MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[ERROR] wdev_cfg is full!\n");
					else
						MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
								 "%s:acquiring DBDC/wdev_cfg[%d] for band%d!\n", __func__, i, TESTMODE_BAND0);
				} else {
					MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
										 "[ERROR] ATE DBDC/wdev[%d] band%d allocate memory failed!\n",
										  i, TESTMODE_BAND0);
				}
			}
#ifdef DBDC_MODE
			if (ad->ATECtrl.band_ext[0].wdev[i] == NULL) {
				os_alloc_mem(ad, (PUCHAR *)&ad->ATECtrl.band_ext[0].wdev[i], sizeof(struct wifi_dev));
				wdev = ad->ATECtrl.band_ext[0].wdev[i];

				if (wdev != NULL) {
					NdisZeroMemory(wdev, sizeof(struct wifi_dev));

					if (wpf_config_acquire(ad, wdev) < 0)
							MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[ERROR] wdev_cfg is full!\n");
					else
						MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
										 "%s:acquiring DBDC/wdev_cfg[%d] for band%d wdev!\n", __func__, i, TESTMODE_BAND1);
				} else {
					MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
										 "[ERROR] ATE DBDC/wdev[%d] band%d allocate memory failed!\n",
										  i, TESTMODE_BAND1);
				}
			}
#endif	/* DBDC_MODE */
		}
	}
#endif /* CONFIG_ATE */
#endif /* CONFIG_WLAN_SERVICE */
/*Don't alloc ad->ATECtrl.wdev because in ATEInit ad->ATECtrl.wdev point to the same memory as test_configs->wdev*/

}

VOID wpf_exit(struct _RTMP_ADAPTER *ad)
{
	struct wifi_dev *wdev;
	int i;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(ad) {
		for (i = 0; i < MAX_MBSSID_NUM(ad); i++) {
			wdev = &ad->ApCfg.MBSSID[i].wdev;

			if (wpf_config_release(ad, wdev) < 0) {
				MTWF_DBG(ad, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "[ERROR] wdev_cfg is full!\n");
				return;
			}
		}

#ifdef WDS_SUPPORT

		for (i = 0; i < MAX_WDS_ENTRY; i++) {
			wdev = &ad->WdsTab.WdsEntry[i].wdev;

			if (wpf_config_release(ad, wdev) < 0) {
				MTWF_DBG(ad, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "[ERROR] wdev_cfg is full!\n");
				return;
			}
		}

#endif /*WDS_SUPPORT*/
#ifdef APCLI_SUPPORT

		for (i = 0; i < MAX_APCLI_NUM; i++) {
			wdev = &ad->StaCfg[i].wdev;

			if (wpf_config_release(ad, wdev) < 0) {
				MTWF_DBG(ad, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "[ERROR] wdev_cfg is full!\n");
				return;
			}
		}

#endif /*APCLI_SUPPORT*/
	}
#endif /*CONFIG_AP_SUPPORT*/
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(ad) {
		for (i = 0; i < MAX_MULTI_STA; i++) {
			wdev = &ad->StaCfg[i].wdev;

			if (wpf_config_release(ad, wdev) < 0) {
				MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "[ERROR] wdev_cfg is full!\n");
				return;
			}
		}
	}
#endif /*CONFIG_STA_SUPPORT*/

#ifdef CONFIG_WLAN_SERVICE
	{
		UCHAR band_idx, wmm_idx;

		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s: releasing SERVICE wdev_cfg!\n", __func__);

		for (band_idx = DBDC_BAND0;
			band_idx < DBDC_BAND_NUM; band_idx++) {
			for (wmm_idx = 0 ; wmm_idx < 2 ; wmm_idx++) {
				wdev = &ad->ate_wdev[band_idx][wmm_idx];

				if (wpf_config_release(ad, wdev) < 0)
					MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"[ERROR] wdev_cfg is full!\n");
			}
		}
	}

#else
#ifdef CONFIG_ATE
	{
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: Releasing DBDC/ATE wdev_cfg!\n", __func__);

		for (i = 0; i < 2; i++) {
			wdev = (struct wifi_dev *)ad->ATECtrl.wdev[i];

			if (wdev != NULL) {
				if (wpf_config_release(ad, wdev) < 0)
					MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[ERROR] wdev_cfg is full!\n");

				os_free_mem(ad->ATECtrl.wdev[i]);
				ad->ATECtrl.wdev[i] = NULL;
			}

#ifdef DBDC_MODE
			wdev = (struct wifi_dev *)ad->ATECtrl.band_ext[0].wdev[i];

			if (wdev != NULL) {
				if (wpf_config_release(ad, wdev) < 0)
					MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[ERROR] wdev_cfg is full!\n");

				os_free_mem(ad->ATECtrl.band_ext[0].wdev[i]);
				ad->ATECtrl.band_ext[0].wdev[i] = NULL;
			}
#endif /* DBDC_MODE */
		}
	}
#endif /* CONFIG_ATE */
#endif /* CONFIG_WLAN_SERVICE */

}
