#ifdef WIFI_SYS_FW_V2

#include "rt_config.h"
#include  "hw_ctrl.h"
#include "hw_ctrl_basic.h"

static NTSTATUS hw_ctrl_flow_v2_open(struct WIFI_SYS_CTRL *wsys)
{
	struct wifi_dev *wdev = wsys->wdev;
	struct _RTMP_ADAPTER *ad = (PRTMP_ADAPTER)wdev->sys_handle;
	struct _DEV_INFO_CTRL_T *devinfo =  &wsys->DevInfoCtrl;

	MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "wdev_idx=%d\n", wsys->wdev->wdev_idx);
	if (devinfo->EnableFeature) {
		AsicDevInfoUpdate(
			ad,
			devinfo->OwnMacIdx,
			devinfo->OwnMacAddr,
			devinfo->BandIdx,
			devinfo->WdevActive,
			devinfo->EnableFeature
		);
		/*update devinfo to wdev*/
		wifi_sys_update_devinfo(ad, wdev, devinfo);
	}

	wifi_sys_op_unlock(wdev);

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static NTSTATUS hw_ctrl_flow_v2_close(struct WIFI_SYS_CTRL *wsys)
{
	struct wifi_dev *wdev = wsys->wdev;
	struct _RTMP_ADAPTER *ad = (PRTMP_ADAPTER)wdev->sys_handle;
	struct _DEV_INFO_CTRL_T *devinfo =  &wsys->DevInfoCtrl;

	MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "wdev_idx=%d\n", wsys->wdev->wdev_idx);
	if (devinfo->EnableFeature) {
		AsicDevInfoUpdate(
			ad,
			devinfo->OwnMacIdx,
			devinfo->OwnMacAddr,
			devinfo->BandIdx,
			devinfo->WdevActive,
			devinfo->EnableFeature
		);
		/*update devinfo to wdev*/
		wifi_sys_update_devinfo(ad, wdev, devinfo);
	}

	wifi_sys_op_unlock(wdev);

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static NTSTATUS hw_ctrl_flow_v2_link_up(struct WIFI_SYS_CTRL *wsys)
{
	struct wifi_dev *wdev = wsys->wdev;
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	struct _STA_REC_CTRL_T *sta_rec = &wsys->StaRecCtrl;
	struct _BSS_INFO_ARGUMENT_T *bss = &wsys->BssInfoCtrl;
	UINT16 txop_level = TXOP_0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "wdev_idx=%d\n", wsys->wdev->wdev_idx);
	if (bss->u4BssInfoFeature) {
		AsicBssInfoUpdate(ad, &wsys->BssInfoCtrl);
		HcSetEdca(wdev);
		/*update bssinfo tp wdev*/
		wifi_sys_update_bssinfo(ad, wdev, bss);
	}

	if (sta_rec->EnableFeature) {
		AsicStaRecUpdate(ad, sta_rec);
		/*update starec to tr_entry*/
		wifi_sys_update_starec(ad, sta_rec);
	}

	if (ad->CommonCfg.bEnableTxBurst) {
		txop_level = cap->peak_txop;

		if (ad->CommonCfg.bRdg)
			txop_level = cap->peak_txop;
	} else {
		txop_level = TXOP_0;
	}

	hw_set_tx_burst(ad, wdev, AC_BE, PRIO_DEFAULT, txop_level, 1);
#ifdef CONFIG_AP_SUPPORT

	if (WDEV_WITH_BCN_ABILITY(wdev)) {
		UpdateBeaconHandler(
			ad,
			wdev,
			BCN_UPDATE_INIT);
	}

#endif /*CONFIG_AP_SUPPORT*/

	wifi_sys_op_unlock(wdev);

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static NTSTATUS hw_ctrl_flow_v2_link_down(struct WIFI_SYS_CTRL *wsys)
{
	struct wifi_dev *wdev = wsys->wdev;
	struct _RTMP_ADAPTER *ad = (PRTMP_ADAPTER)wdev->sys_handle;
	struct _STA_REC_CTRL_T *sta_rec = &wsys->StaRecCtrl;
	struct _BSS_INFO_ARGUMENT_T *bss = &wsys->BssInfoCtrl;
	struct wifi_dev *pwdev = NULL;
	UINT32 i = 0, ActBssidNum = 0;
	MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "wdev_idx=%d\n", wsys->wdev->wdev_idx);

	if (sta_rec->EnableFeature) {
		AsicStaRecUpdate(ad, sta_rec);
		/*update starec to tr_entry*/
		wifi_sys_update_starec(ad, sta_rec);
	}

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		pwdev = ad->wdev_list[i];

		if (pwdev == NULL)
			continue;

		if (pwdev->if_up_down_state &&
			(pwdev->wdev_type == WDEV_TYPE_AP ||
			pwdev->wdev_type == WDEV_TYPE_WDS) &&
			(pwdev->PhyMode == wdev->PhyMode)) {
			ActBssidNum++;
		}
	}

	if (!wsys->skip_set_txop) {
		if (ActBssidNum == 1)
			hw_set_tx_burst(ad, wdev, AC_BE, PRIO_DEFAULT, TXOP_0, 0);
		else
			wdev->prio_bitmap &= ~(1 << PRIO_DEFAULT);
	}

	if (bss->u4BssInfoFeature) {
		AsicBssInfoUpdate(ad, &wsys->BssInfoCtrl);
		/*update bssinfo to wdev*/
		wifi_sys_update_bssinfo(ad, wdev, bss);
	}

	wifi_sys_op_unlock(wdev);

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static NTSTATUS hw_ctrl_flow_v2_disconnt_act(struct WIFI_SYS_CTRL *wsys)
{
	struct wifi_dev *wdev = wsys->wdev;
	struct _RTMP_ADAPTER *ad = (PRTMP_ADAPTER)wdev->sys_handle;
	struct _STA_REC_CTRL_T *sta_rec = &wsys->StaRecCtrl;

	MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "wdev_idx=%d\n", wsys->wdev->wdev_idx);
	/*release ucast wcid on hw*/
	if (sta_rec->EnableFeature)
		AsicStaRecUpdate(ad, sta_rec);

	if (!wsys->skip_set_txop)
		hw_set_tx_burst(ad, wdev, AC_BE, PRIO_DEFAULT, TXOP_0, 0);

	/*update starec to tr_entry*/
	wifi_sys_update_starec(ad, sta_rec);

	switch (wdev->wdev_type) {
#ifdef CONFIG_AP_SUPPORT

	case WDEV_TYPE_AP: {
		ADD_HT_INFO_IE *addht = wlan_operate_get_addht(wdev);
		/* back to default protection */
		wdev->protection = 0;
		addht->AddHtInfo2.OperaionMode = 0;
		UpdateBeaconHandler(ad, wdev, BCN_UPDATE_IE_CHG);
	}
	break;
#endif /*CONFIG_AP_SUPPORT*/
	}

	wifi_sys_op_unlock(wdev);

	return NDIS_STATUS_SUCCESS;
}


/*
*
*/
static NTSTATUS hw_ctrl_flow_v2_connt_act(struct WIFI_SYS_CTRL *wsys)
{
	struct wifi_dev *wdev = wsys->wdev;
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	PEER_LINKUP_HWCTRL *lu_ctrl = (PEER_LINKUP_HWCTRL *)wsys->priv;
	UINT16 txop_level = TXOP_0;
	struct _STA_REC_CTRL_T *sta_rec = &wsys->StaRecCtrl;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "wdev_idx=%d\n", wsys->wdev->wdev_idx);
	/*
	* check whether add new starec and update para to fw:
	* 1)if certification enable, directly do AsicStaRecUpdate for TGax test;
	* 2)if certification disable, we should check starec exist or not.
	*/
	if (!ad->CommonCfg.wifi_cert
		&& get_starec_by_wcid(ad, sta_rec->WlanIdx)) {
		/* reset psm to active */
		AsicSetWcidPsm(ad, sta_rec->WlanIdx, PWR_ACTIVE);
		goto end;
	}

	if (sta_rec->EnableFeature)
		AsicStaRecUpdate(ad, sta_rec);

	/*update sta_rec to tr_entry*/
	wifi_sys_update_starec(ad, sta_rec);

	if (ad->CommonCfg.bEnableTxBurst) {
		txop_level = cap->peak_txop;

		if (ad->CommonCfg.bRdg)
			txop_level = cap->peak_txop;
	} else
		txop_level = TXOP_0;

	hw_set_tx_burst(ad, wdev, AC_BE, PRIO_DEFAULT, txop_level, 1);

	if (wdev->wdev_type == WDEV_TYPE_AP) {
#ifdef DOT11_N_SUPPORT

		if (lu_ctrl->bRdgCap) {
		}

#endif
	}

end:
	wifi_sys_op_unlock(wdev);

	if (lu_ctrl)
		os_free_mem(lu_ctrl);

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static NTSTATUS hw_ctrl_flow_v2_peer_update(struct WIFI_SYS_CTRL *wsys)
{
	struct wifi_dev *wdev = wsys->wdev;
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	struct _STA_REC_CTRL_T *sta_rec = &wsys->StaRecCtrl;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	UINT32 featues = 0;

	MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wdev_idx=%d\n", wsys->wdev->wdev_idx);
	/*update ra rate*/
	if ((sta_rec->EnableFeature & STA_REC_RA_UPDATE_FEATURE) && wsys->priv) {
		AsicRaParamStaRecUpdate(ad,
								sta_rec->WlanIdx,
								(CMD_STAREC_AUTO_RATE_UPDATE_T *)wsys->priv,
								STA_REC_RA_UPDATE_FEATURE);

		if (wsys->priv)
			os_free_mem(wsys->priv);

		return NDIS_STATUS_SUCCESS;
	}

	/*update ra reldated setting, can't change the order*/
	if (sta_rec->EnableFeature & STA_REC_RA_COMMON_INFO_FEATURE) {
		featues = sta_rec->EnableFeature;
		sta_rec->EnableFeature = STA_REC_RA_COMMON_INFO_FEATURE;
		AsicStaRecUpdate(ad, sta_rec);
		sta_rec->EnableFeature = featues & (~STA_REC_RA_COMMON_INFO_FEATURE);
	}

	if (sta_rec->EnableFeature & STA_REC_RA_FEATURE) {
		featues = sta_rec->EnableFeature;
		sta_rec->EnableFeature = STA_REC_RA_FEATURE;
		AsicStaRecUpdate(ad, sta_rec);
		sta_rec->EnableFeature = featues & (~STA_REC_RA_FEATURE);
	}

#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/

	/*normal update*/
	if (sta_rec->EnableFeature) {
		AsicStaRecUpdate(ad, sta_rec);
		/*update starec to tr_entry*/
		wifi_sys_update_starec_info(ad, sta_rec);
	}

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
VOID hw_ctrl_ops_v2_register(struct _HWCTRL_OP *hwctrl_ops)
{
	hwctrl_ops->wifi_sys_open = hw_ctrl_flow_v2_open;
	hwctrl_ops->wifi_sys_close = hw_ctrl_flow_v2_close;
	hwctrl_ops->wifi_sys_link_up = hw_ctrl_flow_v2_link_up;
	hwctrl_ops->wifi_sys_link_down = hw_ctrl_flow_v2_link_down;
	hwctrl_ops->wifi_sys_connt_act = hw_ctrl_flow_v2_connt_act;
	hwctrl_ops->wifi_sys_disconnt_act = hw_ctrl_flow_v2_disconnt_act;
	hwctrl_ops->wifi_sys_peer_update = hw_ctrl_flow_v2_peer_update;
}

#endif /*WIFI_SYS_FW_V2*/

