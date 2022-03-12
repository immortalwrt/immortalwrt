#include "rt_config.h"
#include "hw_ctrl.h"

/* For wifi and md coex in colgin project*/
#ifdef WIFI_MD_COEX_SUPPORT
/*---------------------------------------------------------------------*/
/* WIFI and MD Coexistence Realize                                     */
/*---------------------------------------------------------------------*/

/* realize the wifi md coex_tx event func*/
int wifi_md_coex_tx_event(struct notifier_block *nb, unsigned long event, void *msg)
{
	struct _COEX_APCCCI2FW_CMD *p_coex_apccci2fw_cmd;
	struct _RTMP_ADAPTER *pAd;

	switch (event) {
	case APCCCI_DRIVER_FW:
	{
		if (msg == NULL) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s:received NULL data in notifier chain!!", __func__));
			return NOTIFY_DONE;
		}
		/* get the structure address*/
		p_coex_apccci2fw_cmd = CONTAINER_OF(nb, struct _COEX_APCCCI2FW_CMD, coex_apccci2fw_notifier);
		pAd = p_coex_apccci2fw_cmd->priv;
		/* send cmd to FW*/
		HW_WIFI_COEX_APCCCI2FW(pAd, msg);
	}
		break;
	default:
		break;
	}
	return NOTIFY_DONE;
}

/* send idx */
int send_idx_to_wifi_md_coex(struct _RTMP_ADAPTER *pAd)
{
	struct wifi_dev *wdev = NULL;
	struct _EXT_EVENT_FW2APCCCI_T *fw2apccci_msg = &pAd->fw2apccci_msg;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif
	if (!wdev) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: wdev == NULL\n", __func__));
		return FALSE;
	}

	if (pAd->CommonCfg.dbdc_mode) {
		fw2apccci_msg->dtb_idx = DTB_IDX_DBDC;
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: dtb_idx = DBDC\n", __func__));
	} else if (wdev->channel >= 36) {
		fw2apccci_msg->dtb_idx = DTB_IDX_5G;
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: dtb_idx = 5g\n", __func__));
	} else {
		fw2apccci_msg->dtb_idx = DTB_IDX_2G;
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: dtb_idx = 2.4g\n", __func__));
	}

	call_wifi_md_coex_notifier(REGISTER_WIFI_MD_DTB, fw2apccci_msg);
	return TRUE;
}

/* register */
int register_wifi_md_coex(struct _RTMP_ADAPTER *pAd)
{
	pAd->coex_apccci2fw_cmd.coex_apccci2fw_notifier.notifier_call = wifi_md_coex_tx_event;
	pAd->coex_apccci2fw_cmd.priv = pAd;
	return register_wifi_md_coex_notifier(&pAd->coex_apccci2fw_cmd.coex_apccci2fw_notifier);
}

/* unregister */
int unregister_wifi_md_coex(struct _RTMP_ADAPTER *pAd)
{
	int err = 0;

	err = unregister_wifi_md_coex_notifier(&pAd->coex_apccci2fw_cmd.coex_apccci2fw_notifier);
	pAd->coex_apccci2fw_cmd.coex_apccci2fw_notifier.notifier_call = NULL;
	pAd->coex_apccci2fw_cmd.priv = NULL;
	return err;
}
#endif /* WIFI_MD_COEX_SUPPORT */

