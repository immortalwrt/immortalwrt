/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2019, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************
*/

#include "rt_config.h"
#include "ap_bss_mnger.h"
#include "action.h"

#ifdef CONFIG_6G_SUPPORT

struct bss_mnger bssmnger;

static bool is_module_device(struct bmg_entry *bmgentry)
{
	struct radio_info *radioinfo = NULL;
	UINT8 ret = 0, bitmap = bssmnger.kernel_band_bitmap;

	if (bmgentry == NULL)
		goto end;

	radioinfo = &bmgentry->reginfo.radioinfo;
	if (radioinfo == NULL)
		goto end;

	ret += (bitmap & (WMODE_2G_ONLY(radioinfo->phymode) << 0));
	ret += (bitmap & (WMODE_5G_ONLY(radioinfo->phymode) << 1));
	ret += (bitmap & (WMODE_6G_ONLY(radioinfo->phymode) << 2));

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"bitmap:0x%x (kernel_band_bitmap:0x%x), ret:%d\n",
		bitmap, bssmnger.kernel_band_bitmap, ret);

end:
	return (ret ? TRUE : FALSE);
}

static struct wifi_dev *get_wdev_by_devinfo(struct netdev_info *devinfo)
{
	struct wifi_dev *wdev = NULL;
	struct net_device *netdev = NULL;

	netdev = __dev_get_by_index(&init_net, devinfo->ifindex);
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		":%s: netdev:%p\n", __func__, netdev);
	if (netdev == NULL)
		return NULL;

	wdev = RtmpOsGetNetDevWdev(netdev);
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		":%s: ifindex:%d, wdev:%p\n", __func__, devinfo->ifindex, wdev);

	return wdev;
}

struct wifi_dev *get_6G_wdev_by_bssmnger(void)
{
	struct wifi_dev *wdev = NULL;
	DL_LIST *entry_list = NULL;
	struct bmg_entry *bmgentry = NULL;
	struct radio_info *radioinfo = NULL;
	struct netdev_info *devinfo = NULL;

	entry_list = &bssmnger.entry_list;
	if (entry_list == NULL)
		goto end;
	else if (DlListLen(entry_list) == 0)
		goto end;

	DlListForEach(bmgentry, entry_list, struct bmg_entry, list) {
		if (bmgentry == NULL)
			break;
		if (!is_module_device(bmgentry)) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"This bmgentry does't exist in this module\n");
			continue;
		}
		radioinfo = &bmgentry->reginfo.radioinfo;
		devinfo = &bmgentry->devinfo;
		if (radioinfo == NULL || devinfo == NULL)
			continue;
		if (WMODE_CAP_6G(radioinfo->phymode)) {
			wdev = get_wdev_by_devinfo(devinfo);
			break;
		}
	}

end:
	return wdev;
}

struct wifi_dev *get_5G_wdev_by_bssmnger(void)
{
	struct wifi_dev *wdev = NULL;
	DL_LIST *entry_list = NULL;
	struct bmg_entry *bmgentry = NULL;
	struct radio_info *radioinfo = NULL;
	struct netdev_info *devinfo = NULL;

	entry_list = &bssmnger.entry_list;
	if (entry_list == NULL)
		goto end;
	else if (DlListLen(entry_list) == 0)
		goto end;

	DlListForEach(bmgentry, entry_list, struct bmg_entry, list) {
		if (bmgentry == NULL)
			break;
		if (!is_module_device(bmgentry)) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"This bmgentry does't exist in this module\n");
			continue;
		}
		radioinfo = &bmgentry->reginfo.radioinfo;
		devinfo = &bmgentry->devinfo;
		if (radioinfo == NULL || devinfo == NULL)
			continue;
		if (!WMODE_CAP_6G(radioinfo->phymode) &&
			radioinfo->channel > 14) {
			wdev = get_wdev_by_devinfo(devinfo);
			break;
		}
	}

end:
	return wdev;
}

struct wifi_dev *get_2G_wdev_by_bssmnger(void)
{
	struct wifi_dev *wdev = NULL;
	DL_LIST *entry_list = NULL;
	struct bmg_entry *bmgentry = NULL;
	struct radio_info *radioinfo = NULL;
	struct netdev_info *devinfo = NULL;

	entry_list = &bssmnger.entry_list;
	if (entry_list == NULL)
		goto end;
	else if (DlListLen(entry_list) == 0)
		goto end;

	DlListForEach(bmgentry, entry_list, struct bmg_entry, list) {
		if (bmgentry == NULL)
			break;
		if (!is_module_device(bmgentry)) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"This bmgentry does't exist in this module\n");
			continue;
		}
		radioinfo = &bmgentry->reginfo.radioinfo;
		devinfo = &bmgentry->devinfo;
		if (radioinfo == NULL || devinfo == NULL)
			continue;
		if (WMODE_CAP_6G(radioinfo->phymode) &&
			radioinfo->channel <= 14) {
			wdev = get_wdev_by_devinfo(devinfo);
			break;
		}
	}

end:
	return wdev;
}

static NDIS_STATUS get_iob_info_from_driver(struct wifi_dev *wdev, struct bmg_entry *bmgentry)
{
	int ret = NDIS_STATUS_SUCCESS;
	struct iob_info *iobinfo = NULL;

	iobinfo = &bmgentry->iobinfo;
	if (iobinfo == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: iobinfo is NULL!! (ret:%d)\n", __func__, ret);
		goto err;
	}

	iobinfo->iob_dsc_type = wlan_config_get_unsolicit_tx_type(wdev);
	iobinfo->iob_dsc_interval = wlan_config_get_unsolicit_tx_tu(wdev);
	iobinfo->iob_dsc_txmode = wlan_config_get_unsolicit_tx_mode(wdev);
	iobinfo->iob_dsc_by_cfg = wlan_config_get_unsolicit_tx_by_cfg(wdev);
err:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s, type:%d, interval:%d, txmode:%d, bycfg:%d\n",
		(ret == NDIS_STATUS_SUCCESS ? "succ":"fail"),
		iobinfo->iob_dsc_type, iobinfo->iob_dsc_interval,
		iobinfo->iob_dsc_txmode, iobinfo->iob_dsc_by_cfg);
	return ret;
}

static NDIS_STATUS get_oob_info_from_driver(struct wifi_dev *wdev, struct bmg_entry *bmgentry)
{
	int ret = NDIS_STATUS_SUCCESS;
	struct oob_info *oobinfo = NULL;

	oobinfo = &bmgentry->oobinfo;
	if (oobinfo == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: oobinfo is NULL!! (ret:%d)\n", __func__, ret);
		goto err;
	}

	oobinfo->repting_rule_2g = wlan_config_get_rnr_in_probe_rsp(wdev, RFIC_24GHZ);
	oobinfo->repting_rule_5g = wlan_config_get_rnr_in_probe_rsp(wdev, RFIC_5GHZ);
	oobinfo->repting_rule_6g = wlan_config_get_rnr_in_probe_rsp(wdev, RFIC_6GHZ);

err:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s, repting_rule_2g:%d, repting_rule_5g:%d, repting_rule_6g:%d\n",
		(ret == NDIS_STATUS_SUCCESS ? "succ":"fail"),
		oobinfo->repting_rule_2g, oobinfo->repting_rule_5g, oobinfo->repting_rule_6g);
	return ret;
}

static NDIS_STATUS get_radio_info_from_driver(struct wifi_dev *wdev, struct radio_info *radioinfo)
{
	int ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_ADAPTER *pAd = NULL;

	pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (pAd == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: pAd is NULL!! (ret:%d)\n", __func__, ret);
		goto err;
	}

	radioinfo->channel = wdev->channel;
	NdisMoveMemory(radioinfo->bssid, wdev->bssid, MAC_ADDR_LEN);
	radioinfo->opclass = get_regulatory_class(pAd, wdev->channel, wdev->PhyMode, wdev);
	radioinfo->phymode = wdev->PhyMode;
err:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s, channel:%d, opclass:%d, phymode:%d, bssid: %02x:%02x:%02x:%02x:%02x:%02x\n",
		(ret == NDIS_STATUS_SUCCESS ? "succ":"fail"),
		radioinfo->channel, radioinfo->opclass, radioinfo->phymode,
		PRINT_MAC(radioinfo->bssid));
	return ret;
}

static NDIS_STATUS get_bss_info_from_driver(struct wifi_dev *wdev, struct bss_info *bssinfo)
{
	int ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_ADAPTER *pAd = NULL;
	BSS_STRUCT *pMbss = NULL;

	pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (pAd == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: pAd is NULL!! (ret:%d)\n", __func__, ret);
		goto err;
	}
	pMbss = wdev->func_dev;
	if (pMbss == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: pMbss is NULL!! (ret:%d)\n", __func__, ret);
		goto err;
	}

	bssinfo->bandidx = HcGetBandByWdev(wdev);
	bssinfo->is_hide_ssid = pMbss->bHideSsid;
	bssinfo->is_trans_bss = TRUE;
	bssinfo->is_multi_bss = FALSE;
	bssinfo->mbss_grp_idx = pMbss->mbss_grp_idx;
	NdisMoveMemory(bssinfo->ssid, pMbss->Ssid, (MAX_LEN_OF_SSID+1));
	bssinfo->ssid_len = pMbss->SsidLen;

#ifdef DOT11V_MBSSID_SUPPORT
	if (bssinfo->mbss_grp_idx != MAIN_MBSSID && IS_BSSID_11V_NON_TRANS(pAd, pMbss, bssinfo->bandidx))
		bssinfo->is_trans_bss = FALSE;
	if (IS_BSSID_11V_ENABLED(pAd, bssinfo->bandidx) && (pAd->ApCfg.BssidNumPerBand[bssinfo->bandidx] > 1))
		bssinfo->is_multi_bss = TRUE;
#endif /* DOT11V_MBSSID_SUPPORT */

err:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s\n", ret == NDIS_STATUS_SUCCESS ? "succ":"fail");
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"bandidx:%d, is_hide_ssid:%d, is_trans_bss:%d, is_multi_bss:%d\n",
		bssinfo->bandidx, bssinfo->is_hide_ssid, bssinfo->is_trans_bss, bssinfo->is_multi_bss);
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"mbss_grp_idx:%d, ssid:%s, ssid_len:%d\n",
		bssinfo->mbss_grp_idx, bssinfo->ssid, bssinfo->ssid_len);
	return ret;
}

static NDIS_STATUS get_sec_info_from_driver(struct wifi_dev *wdev, struct sec_info *secinfo)
{
	int ret = NDIS_STATUS_SUCCESS;
	SECURITY_CONFIG *pSecConfig = NULL;

	pSecConfig = &wdev->SecConfig;
	if (pSecConfig == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: pSecConfig is NULL!! (ret:%d)\n", __func__, ret);
		goto err;
	}

	secinfo->auth_mode = GET_SEC_AKM(pSecConfig);
	secinfo->PairwiseCipher = GET_PAIRWISE_CIPHER(pSecConfig);
	secinfo->GroupCipher = GET_GROUP_CIPHER(pSecConfig);

err:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s: auth_mode:%x, PairwiseChipher:%x, GroupCipher:%x\n",
		(ret == NDIS_STATUS_SUCCESS ? "succ":"fail"),
		secinfo->auth_mode, secinfo->PairwiseCipher, secinfo->GroupCipher);
	return ret;
}

static NDIS_STATUS get_reg_info_from_driver(struct wifi_dev *wdev, struct reg_info *reginfo)
{
	int ret = NDIS_STATUS_SUCCESS;
	struct radio_info *radioinfo;
	struct bss_info *bssinfo;
	struct sec_info *secinfo;

	radioinfo = &reginfo->radioinfo;
	bssinfo = &reginfo->bssinfo;
	secinfo = &reginfo->secinfo;

	if (radioinfo == NULL || bssinfo == NULL || secinfo == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: radioinfo or bssinfo or secinfo is NULL!! (ret:%d)\n",
			__func__, ret);
		goto err;
	}

	ret = get_radio_info_from_driver(wdev, radioinfo);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: get_radio_info_from_driver fail!! (ret:%d)\n", __func__, ret);
	}

	ret = get_bss_info_from_driver(wdev, bssinfo);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: get_bss_info_from_driver fail!! (ret:%d)\n", __func__, ret);
	}

	ret = get_sec_info_from_driver(wdev, secinfo);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: get_sec_info fail!! (ret:%d)\n", __func__, ret);
	}

err:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s\n", (ret == NDIS_STATUS_SUCCESS ? "succ":"fail"));
	return ret;
}

static NDIS_STATUS get_module_info_from_driver(struct wifi_dev *wdev, struct module_info *modinfo)
{
	int ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_ADAPTER *pAd = NULL;

	pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pAd is NULL\n");
		goto end;
	}

	modinfo->chip_id = pAd->ChipID;
end:
	return ret;

}

static NDIS_STATUS get_netdev_info_from_driver(struct wifi_dev *wdev, struct netdev_info *devinfo)
{
	int ret = NDIS_STATUS_SUCCESS;
	struct net_device *netdev = NULL;

	netdev = wdev->if_dev;
	if (netdev == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: netdev is NULL!! (ret:%d)\n", __func__, ret);
		goto err;
	}

	devinfo->ifindex = RtmpOsGetNetIfIndex(netdev);
	NdisCopyMemory(devinfo->ifname, RtmpOsGetNetDevName(netdev), IFNAMSIZ);

err:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s, ifindex:%d, ifname:%s\n",
		ret == NDIS_STATUS_SUCCESS ? "succ":"fail",
		devinfo->ifindex, devinfo->ifname);
	return ret;
}

static bool is_same_netdevinfo(struct netdev_info *devinfo1, struct netdev_info *devinfo2)
{
	bool ret = TRUE;

	if (devinfo1 == NULL || devinfo2 == NULL) {
		ret = FALSE;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"devinfo1 or devinfo2 is NULL\n");
		goto end;
	}

	if (devinfo1->ifindex == devinfo2->ifindex
		&& NdisEqualMemory(devinfo1->ifname, devinfo2->ifname, IFNAMSIZ)) {
		ret = TRUE;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"the same\n");
	} else {
		ret = FALSE;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"not the same\n");
	}
end:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"ret: %s\n", ret ? "the same" : "not the same");
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"debug devinfo1:%p, ifindex:%d, ifname:%s\n",
		devinfo1, devinfo1->ifindex, devinfo1->ifname);
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"debug devinfo2:%p, ifindex:%d, ifname:%s\n",
		devinfo2, devinfo2->ifindex, devinfo2->ifname);
	return ret;
}

static struct bmg_entry *get_bmgentry_by_ifindex(UINT32 ifindex)
{
	DL_LIST *entry_list = NULL;
	struct bmg_entry *bmgentry = NULL;
	struct netdev_info *curr_devinfo = NULL;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "--> ifindex %d\n", ifindex);

	entry_list = &bssmnger.entry_list;
	if (entry_list == NULL)
		goto end;

	DlListForEach(bmgentry, entry_list, struct bmg_entry, list) {
		if (bmgentry == NULL) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				":%s: bmgentry is NULL", __func__);
			break;
		} else if (!bmgentry->valid) {
			continue;
		}
		curr_devinfo = &bmgentry->devinfo;
		if (curr_devinfo->ifindex == ifindex)
			return bmgentry;
	}

end:
	return NULL;
}

static struct bmg_entry *get_bmgentry_by_devinfo(struct netdev_info *devinfo)
{
	DL_LIST *entry_list = NULL;
	struct bmg_entry *bmgentry = NULL;
	struct netdev_info *curr_devinfo = NULL;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"--> ifindex:%d, ifname:%s\n", devinfo->ifindex, devinfo->ifname);

	entry_list = &bssmnger.entry_list;
	if (entry_list == NULL)
		goto end;

	DlListForEach(bmgentry, entry_list, struct bmg_entry, list) {
		if (bmgentry == NULL) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				":%s: bmgentry is NULL", __func__);
			break;
		} else if (!bmgentry->valid) {
			continue;
		}
		curr_devinfo = &bmgentry->devinfo;
		if (is_same_netdevinfo(curr_devinfo, devinfo))
			return bmgentry;
	}

end:
	return NULL;
}

static struct bmg_entry *get_bmgentry_by_wdev(struct wifi_dev *wdev)
{
	struct bmg_entry *bmgentry = NULL;
	struct netdev_info *devinfo = NULL;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "--> wdev(%d)\n", wdev->wdev_idx);

	os_alloc_mem(NULL, (UCHAR **)&devinfo, sizeof(struct netdev_info));
	if (get_netdev_info_from_driver(wdev, devinfo) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				":%s: get_netdev_info_from_driver fail", __func__);
		goto end;
	}

	bmgentry = get_bmgentry_by_devinfo(devinfo);
	if (bmgentry == NULL)
		goto end;

end:
	os_free_mem(devinfo);
	return bmgentry;
}

static bool is_oob_discovery_required(struct bmg_entry *self_entry, struct bmg_entry *nbr_entry)
{
	bool required = FALSE;
	struct reg_info *self_reginfo = NULL;
	struct oob_info *self_oobinfo = NULL;
	struct reg_info *nbr_reginfo = NULL;
	struct radio_info *nbr_radioinfo = NULL;
	struct bss_info *nbr_bssinfo = NULL;
	UINT8 report_en = RNR_REPORTING_NONE;

	if (!self_entry->valid) {
		required = FALSE;
		goto end;
	}

	self_reginfo = &self_entry->reginfo;
	if (self_reginfo == NULL) {
		required = FALSE;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: self_reginfo is NULL!! (required:%d)\n", __func__, required);
		goto end;
	}
	self_oobinfo = &self_entry->oobinfo;
	if (self_oobinfo == NULL) {
		required = FALSE;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: self_oobinfo is NULL!! (required:%d)\n", __func__, required);
		goto end;
	}

	nbr_reginfo = &nbr_entry->reginfo;
	if (nbr_reginfo == NULL) {
		required = FALSE;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: nbr_reginfo is NULL!! (required:%d)\n", __func__, required);
		goto end;
	}
	nbr_radioinfo = &nbr_reginfo->radioinfo;
	if (nbr_radioinfo == NULL) {
		required = FALSE;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: nbr_radioinfo is NULL!! (required:%d)\n", __func__, required);
		goto end;
	}
	nbr_bssinfo = &nbr_reginfo->bssinfo;
	if (nbr_bssinfo == NULL) {
		required = FALSE;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: nbr_bssinfo is NULL!! (required:%d)\n", __func__, required);
		goto end;
	}

	/* only transmitted-bss is capable to bring RNR  */
	if (!nbr_bssinfo->is_trans_bss) {
		required = FALSE;
		goto end;
	}

	if (WMODE_CAP_6G(nbr_radioinfo->phymode))
		report_en = self_oobinfo->repting_rule_6g;
	else if (WMODE_CAP_2G(nbr_radioinfo->phymode))
		report_en = self_oobinfo->repting_rule_2g;
	else if (WMODE_CAP_5G(nbr_radioinfo->phymode))
		report_en = self_oobinfo->repting_rule_5g;

	if ((report_en == RNR_REPORTING_ALL_BSS ||
		report_en == RNR_REPORTING_MAIN_BSS) &&
		nbr_bssinfo->mbss_grp_idx == MAIN_MBSSID) {
		required = TRUE;
	}

end:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Self:repting_rule_2g:%d, repting_rule_5g:%d repting_rule_6g:%d\n",
		self_oobinfo->repting_rule_2g, self_oobinfo->repting_rule_5g,
		self_oobinfo->repting_rule_6g);
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Self: report_en:%d, nbor phymode:%d, nbor mbss_grp_idx:%d\n",
		report_en, nbr_radioinfo->phymode, nbr_bssinfo->mbss_grp_idx);
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"required:%d, ifname:%s\n", required, nbr_entry->devinfo.ifname);
	return required;
}

static int is_iob_rule_changed(struct wifi_dev *wdev, struct iob_info *iobinfo)
{
	int change_cnt = 0;

	change_cnt += iobinfo->iob_dsc_type != wlan_config_get_unsolicit_tx_type(wdev);
	change_cnt += iobinfo->iob_dsc_interval != wlan_config_get_unsolicit_tx_tu(wdev);
	change_cnt += iobinfo->iob_dsc_txmode != wlan_config_get_unsolicit_tx_mode(wdev);
	change_cnt += iobinfo->iob_dsc_by_cfg != wlan_config_get_unsolicit_tx_by_cfg(wdev);

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"iob rule %s(change_cnt:%d)\n",	change_cnt ? "change" : "not change", change_cnt);
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"bssmnger: type:%d, interval:%d, txmode:%d, bycfg:%d\n",
		iobinfo->iob_dsc_type, iobinfo->iob_dsc_interval,
		iobinfo->iob_dsc_txmode, iobinfo->iob_dsc_by_cfg);
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"driver: type:%d, interval:%d, txmode:%d, bycfg:%d\n",
		wlan_config_get_unsolicit_tx_type(wdev),
		wlan_config_get_unsolicit_tx_tu(wdev),
		wlan_config_get_unsolicit_tx_mode(wdev),
		wlan_config_get_unsolicit_tx_by_cfg(wdev));
	return change_cnt;
}

static int is_oob_rule_changed(struct wifi_dev *wdev, struct oob_info *oobinfo)
{
	int change_cnt = 0;

	change_cnt += oobinfo->repting_rule_2g != wlan_config_get_rnr_in_probe_rsp(wdev, RFIC_24GHZ);
	change_cnt += oobinfo->repting_rule_5g != wlan_config_get_rnr_in_probe_rsp(wdev, RFIC_5GHZ);
	change_cnt += oobinfo->repting_rule_6g != wlan_config_get_rnr_in_probe_rsp(wdev, RFIC_6GHZ);

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"oob rule %s(change_cnt:%d)\n",	change_cnt ? "change" : "not change", change_cnt);
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"bssmnger: rule2g:%d, rule5g:%d, rule6g:%d\n",
		oobinfo->repting_rule_2g, oobinfo->repting_rule_5g, oobinfo->repting_rule_6g);
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"driver: rule2g:%d, rule5g:%d, rule6g:%d\n",
		wlan_config_get_rnr_in_probe_rsp(wdev, RFIC_24GHZ),
		wlan_config_get_rnr_in_probe_rsp(wdev, RFIC_5GHZ),
		wlan_config_get_rnr_in_probe_rsp(wdev, RFIC_6GHZ));
	return change_cnt;
}

static int is_dicovery_rule_changed(struct wifi_dev *wdev, struct bmg_entry *bmgentry)
{
	bool change_cnt = 0;
	struct iob_info *iobinfo = NULL;
	struct oob_info *oobinfo = NULL;

	iobinfo = &bmgentry->iobinfo;
	oobinfo = &bmgentry->oobinfo;
	if (iobinfo == NULL || oobinfo == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: iobinfo or oobinfo is NULL\n", __func__);
		goto end;
	}
	// Ryan: the change cnt mismatch
	change_cnt += is_iob_rule_changed(wdev, iobinfo);
	change_cnt += is_oob_rule_changed(wdev, oobinfo);

end:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"discovery rule %s(change_cnt:%d)\n",
		change_cnt ? "change" : "not change", change_cnt);
	return change_cnt;
}

int bssmnger_is_entry_exist_2g_5g(void)
{
	int ap_non_6g_entry_cnt = 0, ap_6g_entry_cnt = 0;
	DL_LIST *entry_list = NULL;
	struct bmg_entry *bmgentry = NULL;
	struct radio_info *radioinfo = NULL;

	entry_list = &bssmnger.entry_list;
	DlListForEach(bmgentry, entry_list, struct bmg_entry, list) {
		if (bmgentry == NULL)
			break;
		else if (!bmgentry->valid)
			continue;

		radioinfo = &bmgentry->reginfo.radioinfo;
		if (radioinfo == NULL) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"radioinfo is NULL!! (non_6g_entry_cnt:%d)", ap_non_6g_entry_cnt);
			continue;
		}

		if (WMODE_CAP_6G(radioinfo->phymode))
			ap_6g_entry_cnt++;
		else if (WMODE_CAP_2G(radioinfo->phymode) || WMODE_CAP_5G(radioinfo->phymode))
			ap_non_6g_entry_cnt++;
		else {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"unknown phmode(phymode:%d) entry", radioinfo->phymode);
			continue;
		}
	}
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"is entry lists %s 2g or 5g (non_6g_entry_cnt:%d, 6g_entry_cnt:%d)\n",
		ap_non_6g_entry_cnt ? "exist" : "didn't exist",
		ap_non_6g_entry_cnt, ap_6g_entry_cnt);
	return ap_non_6g_entry_cnt;
}

static NDIS_STATUS bssmnger_set_reported_bss_info(
	struct bmg_entry *bmgentry, struct _repted_bss_info *reported_bssinfo)
{
	int ret = NDIS_STATUS_SUCCESS;
	struct reg_info *reginfo = NULL;
	struct radio_info *radioinfo = NULL;
	struct bss_info *bssinfo = NULL;
	struct iob_info *iobinfo = NULL;

	reginfo = &bmgentry->reginfo;
	iobinfo = &bmgentry->iobinfo;
	radioinfo = &reginfo->radioinfo;
	bssinfo = &reginfo->bssinfo;
	if (reginfo == NULL || iobinfo == NULL || radioinfo == NULL || bssinfo == NULL || reported_bssinfo == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto err;
	}

	/* fill reported bss info by radio info */
	reported_bssinfo->phymode = radioinfo->phymode;
	reported_bssinfo->channel = radioinfo->channel;
	reported_bssinfo->op_class = radioinfo->opclass;
	NdisMoveMemory(reported_bssinfo->bssid, radioinfo->bssid, MAC_ADDR_LEN);
	/* fill reported bss info by bss info */
	reported_bssinfo->bss_grp_idx = bssinfo->mbss_grp_idx;
	reported_bssinfo->ssid_len = bssinfo->ssid_len;
	NdisMoveMemory(reported_bssinfo->ssid, bssinfo->ssid, bssinfo->ssid_len);
	/* fil bss_feature_set by bssinfo and iob info, need to reset first*/
	reported_bssinfo->bss_feature_set = 0;
	reported_bssinfo->bss_feature_set |=
		(bssinfo->is_trans_bss ? AP_6G_TRANS_BSSID : 0) |
		(bssinfo->is_multi_bss ? AP_6G_MULTI_BSSID : 0) |
		((iobinfo->iob_dsc_type == UNSOLICIT_TX_PROBE_RSP) ?
			AP_6G_UNSOL_PROBE_RSP_EN : 0);

err:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"phymode:%d, channel:%d, op_class:%d, bssid: %02x:%02x:%02x:%02x:%02x:%02x\n",
		reported_bssinfo->phymode, reported_bssinfo->channel,
		reported_bssinfo->op_class, PRINT_MAC(reported_bssinfo->bssid));
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"bss_grp_idx:%d, ssid_len:%d, ssid:%s\n", reported_bssinfo->bss_grp_idx,
		reported_bssinfo->ssid_len, reported_bssinfo->ssid);
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"bss_feature_set:0x%x\n", reported_bssinfo->bss_feature_set);
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "set reported bssinfo %s.\n",
		ret == NDIS_STATUS_SUCCESS ? "succ":"fail");
	return ret;
}

static void bssmnger_set_iob_type_to_driver(struct bmg_entry *bmgentry)
{
	struct netdev_info *devinfo = NULL;
	struct wifi_dev *wdev = NULL;
	struct iob_info *new_iobinfo = NULL;
	UINT8 old_iob_type = 0;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");

	devinfo = &bmgentry->devinfo;
	new_iobinfo = &bmgentry->iobinfo;
	if (devinfo == NULL || new_iobinfo == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: devinfo or iobinfo is NULL", __func__);
		goto err;
	}
	wdev = get_wdev_by_devinfo(devinfo);
	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: devinfo or iobinfo is NULL", __func__);
		goto err;
	}

	old_iob_type = wlan_config_get_unsolicit_tx_type(wdev);
	if (new_iobinfo->iob_dsc_type != old_iob_type) {
		wlan_config_set_unsolicit_tx_type(wdev, new_iobinfo->iob_dsc_type);
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Change: set iob_type: (n)%d ==> (o)%d to driver\n",
			new_iobinfo->iob_dsc_type, old_iob_type);
	}
err:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--\n");
}

bool bssmnger_update_6g_iob_rule(struct bmg_entry *target_bmgentry)
{
	bool iob_type_change = FALSE;
	DL_LIST *entry_list = NULL;
	struct bmg_entry *curr_bmgentry = NULL;
	struct netdev_info *target_devinfo = NULL;
	struct iob_info *target_iobinfo = NULL, *curr_iobinfo = NULL;
	struct bss_info *target_bssinfo = NULL, *curr_bssinfo = NULL;
	struct radio_info *curr_radioinfo = NULL;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");

	entry_list = &bssmnger.entry_list;
	target_devinfo = &target_bmgentry->devinfo;
	target_iobinfo = &target_bmgentry->iobinfo;
	target_bssinfo = &target_bmgentry->reginfo.bssinfo;
	if (entry_list == NULL || target_devinfo == NULL || target_iobinfo == NULL
		|| target_bssinfo == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: entry_list or target_bmgentry's infos. is NULL\n", __func__);
		goto end;
	} else if (target_bmgentry->valid &&
		(target_iobinfo->iob_dsc_by_cfg || !target_bssinfo->is_trans_bss)) {
		iob_type_change = FALSE;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"===> %s: [%02d] ignored by cfg or ntx bss\n",
			__func__, target_devinfo->ifindex);
		goto end;
	}

	/*
	 * 1. AP is operating in 6 Ghz only
	 *        - Allowed to transmit either UPR or FD Frames
	 *
	 * 2. AP is operating in multiple bands (including 6 GHz)
	 *        - Disable transmission of UPR and FD frames
	 */

	RTMP_SEM_LOCK(&bssmnger.lock);
	DlListForEach(curr_bmgentry, entry_list, struct bmg_entry, list) {
		if (curr_bmgentry == NULL) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				":%s: curr_bmgentry is NULL\n", __func__);
			break;
		}
		if (!is_module_device(curr_bmgentry)) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"This bmgentry does't exist in this module\n");
			continue;
		}

		curr_iobinfo = &curr_bmgentry->iobinfo;
		curr_radioinfo = &curr_bmgentry->reginfo.radioinfo;
		curr_bssinfo = &curr_bmgentry->reginfo.bssinfo;
		if (curr_iobinfo == NULL || curr_radioinfo == NULL || curr_bssinfo == NULL) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				":%s: curr_bmgentry's infos. is NULL\n", __func__);
			break;
		} else if (!curr_bmgentry->valid)
			continue;
		else if (!(WMODE_CAP_6G(curr_radioinfo->phymode) && curr_bssinfo->is_trans_bss))
			continue;
		else if (curr_iobinfo->iob_dsc_by_cfg)
			continue;

		if (!bssmnger_is_entry_exist_2g_5g()) {
			if (curr_iobinfo->iob_dsc_type == UNSOLICIT_TX_DISABLE) {
				curr_iobinfo->iob_dsc_type = UNSOLICIT_TX_PROBE_RSP;
				iob_type_change = TRUE;
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"<- A, iob dsc enable: curr iob_type (%d) change (%d)\n",
						curr_iobinfo->iob_dsc_type, iob_type_change);
			}
		} else if (curr_iobinfo->iob_dsc_type != UNSOLICIT_TX_DISABLE) {
			curr_iobinfo->iob_dsc_type = UNSOLICIT_TX_DISABLE;
			iob_type_change = TRUE;
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"<- iob dsc disable: curr iob_type (%d) change (%d)\n",
				curr_iobinfo->iob_dsc_type, iob_type_change);
		} else
			iob_type_change = FALSE;

		if (!iob_type_change)
			continue;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"<- iob_type change (%d)\n", iob_type_change);
		bssmnger_set_iob_type_to_driver(curr_bmgentry);
	}
	RTMP_SEM_UNLOCK(&bssmnger.lock);

end:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--\n");
	return iob_type_change;
}

NDIS_STATUS bssmnger_build_reported_bss_list_wdev(
	struct wifi_dev *wdev, struct bmg_entry *bmgentry)
{
	int ret = NDIS_STATUS_SUCCESS, i = 0;
	struct _RTMP_ADAPTER *pAd = NULL;
	struct bmg_entry *nbor_bmgentry = NULL;
	struct oob_info *oobinfo = NULL;
	struct _repted_bss_info *reported_bssinfo = NULL;
	struct cfg_discov_out_of_band *cfg_dsc_oob = NULL;
	UINT8 reported_bss_cnt_tmp = 0;
	UINT32 ifindex = 0;
	UINT64 reported_bmap = 0;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");

	if (wdev == NULL || bmgentry == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev or bmg entry is NULL\n");
		goto end;
	}

	oobinfo = &bmgentry->oobinfo;
	if (oobinfo == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"oobinfo is NULL\n");
		goto end;
	}
	reported_bmap = oobinfo->repting_bmap;
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		" [%02d] reported bss bitmap = 0x%llx\n",
		bmgentry->devinfo.ifindex, reported_bmap);


	pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	cfg_dsc_oob = &wdev->ap6g_cfg.dsc_oob;

	if (pAd == NULL || cfg_dsc_oob == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pAd or cfg_dsc_oob is NULL\n");
		goto end;
	} else if (!cfg_dsc_oob->repted_bss_list) {
		ret = NDIS_STATUS_FAILURE;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"repted_bss_list is NULL\n");
		goto end;
	}

	if (!reported_bmap) {
		reported_bss_cnt_tmp = 0;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"repting_bmap false (repting_bmap:%d)\n", reported_bmap);
	} else {
		if (cfg_dsc_oob->repted_bss_list) {
			RTMP_SEM_LOCK(&cfg_dsc_oob->list_lock);
			for (ifindex = 0; ifindex < MAX_NET_IF_CNT; ifindex++) {
				if (!(reported_bmap & ((UINT64)1 << ifindex)))
					continue;
				nbor_bmgentry = get_bmgentry_by_ifindex(ifindex);
				if (nbor_bmgentry == NULL)
					continue;
				/* repted_bss_list only allocate MAX_REPTED_BSS_CNT entries */
				if (reported_bss_cnt_tmp >= MAX_REPTED_BSS_CNT)
					continue;

				reported_bssinfo =
					(struct _repted_bss_info *)cfg_dsc_oob->repted_bss_list +
						reported_bss_cnt_tmp;

				ret = bssmnger_set_reported_bss_info(nbor_bmgentry, reported_bssinfo);
				if (ret != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"bssmnger_set_reported_bss_info is NULL (ifindex %d)\n", ifindex);
				}
				reported_bss_cnt_tmp++;
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"\tadd reported bss [%02d], feature_set =0x%x, bss_cnt:%d\n",
					ifindex, reported_bssinfo->bss_feature_set, reported_bss_cnt_tmp);
			}
			RTMP_SEM_UNLOCK(&cfg_dsc_oob->list_lock);
		} else {
			reported_bss_cnt_tmp = 0;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"wdev(%d), reported bss count(%d), list_prt = NULL!\n",
				wdev->wdev_idx, cfg_dsc_oob->repted_bss_cnt);
		}
	}

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\tTotal Reported BSS Count: %d\n", reported_bss_cnt_tmp);

	RTMP_SEM_LOCK(&cfg_dsc_oob->list_lock);
	cfg_dsc_oob->repted_bss_cnt = reported_bss_cnt_tmp;
	if (cfg_dsc_oob->repted_bss_cnt) {
		for (i = 0; i < cfg_dsc_oob->repted_bss_cnt; i++) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"repted_bss %d:\n", i);
			hex_dump_with_cat_and_lvl("HEX:",
				(char *)(cfg_dsc_oob->repted_bss_list + i),
				sizeof(struct _repted_bss_info),
				DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO);
		}
	}
	RTMP_SEM_UNLOCK(&cfg_dsc_oob->list_lock);

	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
end:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--\n");
	return ret;
}

NDIS_STATUS bssmnger_build_reported_bss_list(struct bmg_entry *bmgentry)
{
	int ret = NDIS_STATUS_SUCCESS, i = 0;
	struct _RTMP_ADAPTER *pAd = NULL;
	struct wifi_dev *wdev = NULL;
	struct bmg_entry *nbor_bmgentry = NULL;
	struct oob_info *oobinfo = NULL;
	struct _repted_bss_info *reported_bssinfo = NULL;
	struct cfg_discov_out_of_band *cfg_dsc_oob = NULL;
	UINT8 reported_bss_cnt_tmp = 0;
	UINT32 ifindex = 0;
	UINT64 reported_bmap = 0;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");
	oobinfo = &bmgentry->oobinfo;
	if (oobinfo == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"oobinfo is NULL\n");
		goto end;
	}
	reported_bmap = oobinfo->repting_bmap;
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		" [%02d] reported bss bitmap = 0x%llx\n",
		bmgentry->devinfo.ifindex, reported_bmap);

	wdev = get_wdev_by_devinfo(&bmgentry->devinfo);
	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev (%d, %s) is NULL\n", bmgentry->devinfo.ifindex,
			bmgentry->devinfo.ifname);
		goto end;
	}

	pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	cfg_dsc_oob = &wdev->ap6g_cfg.dsc_oob;

	if (pAd == NULL || cfg_dsc_oob == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev or pAd or cfg_dsc_oob is NULL\n");
		goto end;
	} else if (!cfg_dsc_oob->repted_bss_list) {
		ret = NDIS_STATUS_FAILURE;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"repted_bss_list is NULL\n");
		goto end;
	}

	if (!reported_bmap) {
		reported_bss_cnt_tmp = 0;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"repting_bmap false (repting_bmap:%d)\n", reported_bmap);
	} else {
		if (cfg_dsc_oob->repted_bss_list) {
			RTMP_SEM_LOCK(&cfg_dsc_oob->list_lock);
			for (ifindex = 0; ifindex < MAX_NET_IF_CNT; ifindex++) {
				if (!(reported_bmap & ((UINT64)1 << ifindex)))
					continue;
				nbor_bmgentry = get_bmgentry_by_ifindex(ifindex);
				if (nbor_bmgentry == NULL)
					continue;
				/* repted_bss_list only allocate MAX_REPTED_BSS_CNT entries */
				if (reported_bss_cnt_tmp >= MAX_REPTED_BSS_CNT)
					continue;

				reported_bssinfo =
					(struct _repted_bss_info *)cfg_dsc_oob->repted_bss_list +
						reported_bss_cnt_tmp;

				ret = bssmnger_set_reported_bss_info(nbor_bmgentry, reported_bssinfo);
				if (ret != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"bssmnger_set_reported_bss_info is NULL\n");
				}
				reported_bss_cnt_tmp++;
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"\tadd reported bss [%02d], feature_set =0x%x, bss_cnt:%d\n",
					ifindex, reported_bssinfo->bss_feature_set, reported_bss_cnt_tmp);
			}
			RTMP_SEM_UNLOCK(&cfg_dsc_oob->list_lock);
		} else {
			reported_bss_cnt_tmp = 0;
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"wdev(%d), reported bss count(%d), list_prt = NULL!\n",
				wdev->wdev_idx, cfg_dsc_oob->repted_bss_cnt);
		}
	}

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\tTotal Reported BSS Count: %d\n", reported_bss_cnt_tmp);

	RTMP_SEM_LOCK(&cfg_dsc_oob->list_lock);
	cfg_dsc_oob->repted_bss_cnt = reported_bss_cnt_tmp;
	if (cfg_dsc_oob->repted_bss_cnt) {
		for (i = 0; i < cfg_dsc_oob->repted_bss_cnt; i++) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"repted_bss %d:\n", i);
			hex_dump_with_cat_and_lvl("HEX:",
				(char *)(cfg_dsc_oob->repted_bss_list + i),
				sizeof(struct _repted_bss_info),
				DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO);
		}
	}
	RTMP_SEM_UNLOCK(&cfg_dsc_oob->list_lock);

	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
end:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--\n");
	return ret;
}

NDIS_STATUS bssmnger_update_reported_bss_list(
	struct bmg_entry *curr_bmgentry, bool rule_option)
{
	int ret = NDIS_STATUS_SUCCESS;
	DL_LIST *entry_list = NULL;
	struct bmg_entry *nbor_bmgentry = NULL;
	struct netdev_info *curr_devinfo = NULL, *nbor_devinfo = NULL;
	struct oob_info *curr_oobinfo = NULL, *nbor_oobinfo = NULL;
	UINT64 curr_bmap_backup = 0, curr_bmap_tmp = 0, nbor_bmap_backup = 0;

	entry_list = &bssmnger.entry_list;
	if (entry_list == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: entry_list is NULL\n", __func__);
		ret = NDIS_STATUS_FAILURE;
		goto err;
	}

	curr_devinfo = &curr_bmgentry->devinfo;
	curr_oobinfo = &curr_bmgentry->oobinfo;
	if (curr_devinfo == NULL || curr_oobinfo == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: curr_devinfo or curr_oobinfo is NULL\n", __func__);
		ret = NDIS_STATUS_FAILURE;
		goto err;
	}
	curr_bmap_backup = curr_bmap_tmp = curr_oobinfo->repting_bmap;

	RTMP_SEM_LOCK(&bssmnger.lock);
	DlListForEach(nbor_bmgentry, entry_list, struct bmg_entry, list) {
		if (nbor_bmgentry && nbor_bmgentry != curr_bmgentry) {
			if (!is_module_device(nbor_bmgentry)) {
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"This bmgentry does't exist in this module\n");
				continue;
			}
			nbor_devinfo = &nbor_bmgentry->devinfo;
			nbor_oobinfo = &nbor_bmgentry->oobinfo;
			if (nbor_devinfo == NULL || nbor_oobinfo == NULL) {
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					":%s: nbor_devinfo or nbor_oobinfo is NULL\n", __func__);
				break;
			}
			/* RNR to neighbor */
			nbor_bmap_backup = nbor_oobinfo->repting_bmap;
			if (is_oob_discovery_required(curr_bmgentry, nbor_bmgentry))
				nbor_oobinfo->repting_bmap |= ((UINT64)1 << curr_devinfo->ifindex);
			else
				nbor_oobinfo->repting_bmap &= ~((UINT64)1 << curr_devinfo->ifindex);

			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"<- RNR2N: nbor_oobinfo->repting_bmap:0x%llx (orig: bitmap:0x%llx)\n",
				nbor_oobinfo->repting_bmap, nbor_bmap_backup);

			/* bitmap change */
			if ((nbor_oobinfo->repting_bmap != nbor_bmap_backup) || rule_option) {
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"\tChange: N[%02d] %s RNR_S[%02d], bitmap = 0x%llx -> 0x%llx\n",
					nbor_devinfo->ifindex,
					(nbor_oobinfo->repting_bmap & ((UINT64)1 << curr_devinfo->ifindex)) ? "+" : "-",
					curr_devinfo->ifindex, nbor_bmap_backup, nbor_oobinfo->repting_bmap);
				bssmnger_build_reported_bss_list(nbor_bmgentry);
			}

			/* RNR to self */
			curr_bmap_tmp = curr_oobinfo->repting_bmap;
			if (is_oob_discovery_required(nbor_bmgentry, curr_bmgentry))
				curr_oobinfo->repting_bmap |= ((UINT64)1 << nbor_devinfo->ifindex);
			else
				curr_oobinfo->repting_bmap &= ~((UINT64)1 << nbor_devinfo->ifindex);

			if (curr_oobinfo->repting_bmap != curr_bmap_tmp) {
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"\tS[%02d] %s RNR_N[%02d], bitmap = 0x%llx -> 0x%llx\n",
					curr_devinfo->ifindex,
					(curr_oobinfo->repting_bmap & ((UINT64)1 << nbor_devinfo->ifindex)) ? "+" : "-",
					nbor_devinfo->ifindex, curr_bmap_tmp, curr_oobinfo->repting_bmap);
			}

			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"<- RNR2S: curr_oobinfo->repting_bmap:0x%llx (orig: bitmap:0x%llx)\n",
				curr_oobinfo->repting_bmap, curr_bmap_tmp);
		}
	}
	RTMP_SEM_UNLOCK(&bssmnger.lock);

	if (curr_bmgentry->valid && (curr_oobinfo->repting_bmap ^ curr_bmap_backup || rule_option))
		bssmnger_build_reported_bss_list(curr_bmgentry);

err:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<-- (ret:%d)\n", ret);
	return ret;
}

NDIS_STATUS bssmnger_update_discovery_rule(struct wifi_dev *wdev)
{
	int ret = NDIS_STATUS_SUCCESS;
	bool is_rule_changed = FALSE;
	struct bmg_entry *bmgentry = NULL;
	struct iob_info *iobinfo = NULL;
	struct oob_info *oobinfo = NULL;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");

	bmgentry = get_bmgentry_by_wdev(wdev);
	if (bmgentry == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: bmgentry is NULL!! (ret:%d)\n", __func__, ret);
		goto err;
	}

	if (is_dicovery_rule_changed(wdev, bmgentry))
		is_rule_changed = TRUE;

	/* get the iobinfo from wdev */
	ret = get_iob_info_from_driver(wdev, bmgentry);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: get_iob_info_from_driver fail!! (ret:%d)\n", __func__, ret);
	}

	/* get the oobinfo from wdev */
	ret = get_oob_info_from_driver(wdev, bmgentry);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: get_oob_info_from_driver fail!! (ret:%d)\n", __func__, ret);
	}

	iobinfo = &bmgentry->iobinfo;
	oobinfo = &bmgentry->oobinfo;
	MTWF_PRINT("%s: IoB (%d/%d/%d%s), OoB Reported rules (%d/%d/%d)\n", __func__,
		iobinfo->iob_dsc_type, iobinfo->iob_dsc_interval, iobinfo->iob_dsc_txmode,
		iobinfo->iob_dsc_by_cfg ? " bycfg":"",
		oobinfo->repting_rule_2g, oobinfo->repting_rule_5g, oobinfo->repting_rule_6g);

	ret = bssmnger_update_reported_bss_list(bmgentry, is_rule_changed);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: bssmnger_update_reported_bss_list fail!! (ret:%d)\n", __func__, ret);
		goto err;
	}

	/* check 6G AP allowed to tx UPR or FD frames */
	if (bssmnger_update_6g_iob_rule(bmgentry)) {
		ret = bssmnger_update_reported_bss_list(bmgentry, TRUE);
		if (ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"bssmnger_update_reported_bss_list fail!! (ret:%d)\n", ret);
			goto err;
		}
	}
err:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<-- (ret:%d)\n");
	return ret;
}

NDIS_STATUS bssmnger_update_radio_info(struct wifi_dev *wdev)
{
	int ret = NDIS_STATUS_SUCCESS;
	struct bmg_entry *bmgentry = NULL;
	struct radio_info *radioinfo = NULL;

	bmgentry = get_bmgentry_by_wdev(wdev);
	if (bmgentry == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: bmgentry is NULL!! (ret:%d)\n", __func__, ret);
		goto err;
	}

	radioinfo = &bmgentry->reginfo.radioinfo;
	ret = get_radio_info_from_driver(wdev, radioinfo);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: get_radio_info_from_driver fail!! (ret:%d)\n", __func__, ret);
		goto err;
	}

	ret = bssmnger_update_reported_bss_list(bmgentry, TRUE);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: bssmnger_update_reported_bss_list fail!! (ret:%d)\n", __func__, ret);
	}

err:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s\n", ret == NDIS_STATUS_SUCCESS ? "succ":"fail");
	return ret;

}

static NDIS_STATUS bssmnger_init_band_bitmap_from_driver(struct wifi_dev *wdev)
{
	int ret = NDIS_STATUS_SUCCESS;

	/*
	 * Need not to reset the kernel_band_bitmap when bsslist de-reg.
	 * This bit map shows the capability, which support interfaces in
	 * this kernel module.
	 */

	bssmnger.kernel_band_bitmap |= (WMODE_2G_ONLY(wdev->PhyMode) << 0);
	bssmnger.kernel_band_bitmap |= (WMODE_5G_ONLY(wdev->PhyMode) << 1);
	bssmnger.kernel_band_bitmap |= (WMODE_6G_ONLY(wdev->PhyMode) << 2);

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"phymode:%d, band_bmap:0x%x\n", wdev->PhyMode, bssmnger.kernel_band_bitmap);
	return ret;
}

NDIS_STATUS bssmnger_reg_bmg_entry(struct wifi_dev *wdev)
{
	int ret = NDIS_STATUS_SUCCESS;
	DL_LIST *entry_list = NULL;
	struct bmg_entry *bmgentry = NULL;
	struct netdev_info *devinfo = NULL;
	struct module_info *modinfo = NULL;
	struct reg_info *reginfo = NULL;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");

	entry_list = &bssmnger.entry_list;
	if (get_bmgentry_by_wdev(wdev)) {
		ret = NDIS_STATUS_FAILURE;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: entry(wdev:%d) existed!! (ret:%d)\n", __func__, wdev->wdev_idx, ret);
		goto err1;
	} else if (entry_list == NULL) {
		ret = NDIS_STATUS_FAILURE;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: entry_list is NULL!! (ret:%d)\n", __func__, ret);
		goto err1;
	}

	/* alloc bmgentry's space */
	os_alloc_mem(NULL, (UCHAR **)&bmgentry, sizeof(struct bmg_entry));
	if (bmgentry == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: bmgentry is NULL!! (ret:%d)\n", __func__, ret);
		goto err2;
	}
	memset(bmgentry, 0x00, sizeof(struct bmg_entry));
	bmgentry->valid = TRUE;

	modinfo = &bmgentry->modinfo;
	devinfo = &bmgentry->devinfo;
	reginfo = &bmgentry->reginfo;
	if (devinfo == NULL || reginfo == NULL || modinfo == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: devinfo or reginfo is NULL!! (ret:%d)\n", __func__, ret);
		goto err2;
	}

	ret = get_module_info_from_driver(wdev, modinfo);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: get module_info fail!! (ret:%d)\n", __func__, ret);
	}

	ret = get_netdev_info_from_driver(wdev, devinfo);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: get netdev_info fail!! (ret:%d)\n", __func__, ret);
	}

	ret = get_reg_info_from_driver(wdev, reginfo);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: get get_reg_info_from_driver fail!! (ret:%d)\n", __func__, ret);
	}

	ret = bssmnger_init_band_bitmap_from_driver(wdev);
	if (ret != NDIS_STATUS_SUCCESS) {
		ret = NDIS_STATUS_FAILURE;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: bssmnger_update_reported_bss_list fail!! (ret:%d)\n", __func__, ret);
		goto err2;
	}

	RTMP_SEM_LOCK(&bssmnger.lock);
	DlListAddTail(entry_list, &bmgentry->list);
	bssmnger.dev_cnt++;
	RTMP_SEM_UNLOCK(&bssmnger.lock);

	ret = bssmnger_update_reported_bss_list(bmgentry, FALSE);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: bssmnger_update_reported_bss_list fail!! (ret:%d)\n", __func__, ret);
	}

	ret = bssmnger_update_discovery_rule(wdev);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: bssmnger_update_discovery_rule fail!! (ret:%d)\n", __func__, ret);
	}

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"<-- reg %s, valid:%d, devCnt:%d, band_bitmap:0x%x\n",
		ret == NDIS_STATUS_SUCCESS ? "succ":"fail",
		bmgentry->valid, bssmnger.dev_cnt, bssmnger.kernel_band_bitmap);

#ifdef BSSMGR_CROSS_MODULE_SUPPORT
	ret = indicate_bssentry_to_wappd(wdev, BSSMNGER_MSG_REG_BMGENTRY);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"indicate_bssentry_to_wappd fail (ret:%d)\n", ret);
		goto err2;
	}
#endif /* BSSMGR_CROSS_MODULE_SUPPORT */
err1:
	return ret;
err2:
	if (bmgentry) {
		/* Clear the reg information including sec_info */
		memset(bmgentry, 0x00, sizeof(struct bmg_entry));
		os_free_mem(bmgentry);
	}
	return ret;
}

NDIS_STATUS bssmnger_dereg_bmg_entry(struct wifi_dev *wdev)
{
	int ret = NDIS_STATUS_SUCCESS;
	DL_LIST *entry_list = NULL;
	struct bmg_entry *bmgentry = NULL;
	struct netdev_info *entry_devinfo, devinfo;
	struct oob_info *entry_oobinfo;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");

	entry_list = &bssmnger.entry_list;
	if (entry_list == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: entry_list is NULL!! (ret:%d)\n", __func__, ret);
		goto err;
	}

	bmgentry = get_bmgentry_by_wdev(wdev);
	if (bmgentry == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: bmgentry is NULL!! (ret:%d)\n", __func__, ret);
		goto err;
	}

	entry_oobinfo = &bmgentry->oobinfo;
	bmgentry->valid = FALSE;

	/* clear self reporting rule and delivering */
	entry_oobinfo->repting_rule_2g = RNR_REPORTING_NONE;
	entry_oobinfo->repting_rule_5g = RNR_REPORTING_NONE;
	entry_oobinfo->repting_rule_6g = RNR_REPORTING_NONE;

	ret = bssmnger_update_reported_bss_list(bmgentry, FALSE);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: bssmnger_update_reported_bss_list fail!! (ret:%d)\n", __func__, ret);
	}

	RTMP_SEM_LOCK(&bssmnger.lock);
	DlListDel(&bmgentry->list);
	bssmnger.dev_cnt--;
	RTMP_SEM_UNLOCK(&bssmnger.lock);

	/* Clear the reg information including security information */
	memset(bmgentry, 0x00, sizeof(struct bmg_entry));
	os_free_mem(bmgentry);
	bmgentry = NULL;

#ifdef BSSMGR_CROSS_MODULE_SUPPORT
	ret = indicate_bssentry_to_wappd(wdev, BSSMNGER_MSG_DEREG_BMGENTRY);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"indicate_bssentry_to_wappd fail (ret:%d)\n", ret);
	}
#endif /* BSSMGR_CROSS_MODULE_SUPPORT */

err:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"<-- dereg %s, devCnt:%d, bitmap:0x%x\n", ret == NDIS_STATUS_SUCCESS ? "done":"fail",
		bssmnger.dev_cnt, bssmnger.kernel_band_bitmap);
	return ret;
}

struct bmg_entry *get_bmg_entry_by_ifname(char *ifname)
{

	DL_LIST *entry_list = NULL;
	struct bmg_entry *bmgentry = NULL;
	struct netdev_info *devinfo = NULL;

	entry_list = &bssmnger.entry_list;
	DlListForEach(bmgentry, entry_list, struct bmg_entry, list) {
		if (bmgentry) {
			devinfo = &bmgentry->devinfo;
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s cmp [%02d] %s, bmgentry: %p\n", ifname,
				devinfo->ifindex, devinfo->ifname, bmgentry);

			if (NdisEqualMemory(devinfo->ifname, ifname, IFNAMSIZ)) {
				return bmgentry;
			}
		}
	}
	return NULL;
}

#ifdef BSSMGR_CROSS_MODULE_SUPPORT
static NDIS_STATUS bmgentrycpy(struct bmg_entry *dst, struct bmg_entry *src)
{
	struct module_info *src_modinfo = NULL, *dst_modinfo = NULL;
	struct netdev_info *src_devinfo = NULL, *dst_devinfo = NULL;
	struct reg_info *src_reginfo, *dst_reginfo = NULL;
	struct iob_info *src_iobinfo = NULL, *dst_iobinfo = NULL;
	struct oob_info *src_oobinfo = NULL, *dst_oobinfo = NULL;

	if (dst == NULL || src == NULL)
		return NDIS_STATUS_INVALID_DATA;

	src_modinfo = &src->modinfo;
	src_devinfo = &src->devinfo;
	src_reginfo = &src->reginfo;
	src_iobinfo = &src->iobinfo;
	src_oobinfo = &src->oobinfo;

	dst_modinfo = &dst->modinfo;
	dst_devinfo = &dst->devinfo;
	dst_reginfo = &dst->reginfo;
	dst_iobinfo = &dst->iobinfo;
	dst_oobinfo = &dst->oobinfo;

	dst->valid = src->valid;
	NdisCopyMemory(dst_modinfo, src_modinfo, sizeof(struct module_info));
	NdisCopyMemory(dst_devinfo, src_devinfo, sizeof(struct netdev_info));
	NdisCopyMemory(dst_reginfo, src_reginfo, sizeof(struct reg_info));
	NdisCopyMemory(dst_iobinfo, src_iobinfo, sizeof(struct iob_info));
	NdisCopyMemory(dst_oobinfo, src_oobinfo, sizeof(struct oob_info));

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS bssmnger_set_event_handle(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR *buf)
{
	INT ret = NDIS_STATUS_SUCCESS;

	struct bmg_entry *bmgentry_src = (struct bmg_entry *)buf;
	DL_LIST *entry_list = NULL;
	struct bmg_entry *bmgentry = NULL, *curr = NULL;
	struct netdev_info *curr_devinfo = NULL, *src_devinfo;

	if (bmgentry_src == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		return ret;
	}

	entry_list = &bssmnger.entry_list;

	RTMP_SEM_LOCK(&bssmnger.lock);
	DlListForEach(curr, entry_list, struct bmg_entry, list) {
		curr_devinfo = &curr->devinfo;
		src_devinfo = &bmgentry_src->devinfo;

		if (is_same_netdevinfo(curr_devinfo, src_devinfo)) {
			/* Update bmg entry*/
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					":%s: Update [%s] bmg entry\n", __func__, curr_devinfo->ifname);
			ret = bmgentrycpy(curr, bmgentry_src);
			RTMP_SEM_UNLOCK(&bssmnger.lock);

			/* Only self dev can update reported bss list */
			if (strcmp(RtmpOsGetNetDevName(wdev->if_dev), curr_devinfo->ifname) == 0)
				bssmnger_build_reported_bss_list_wdev(wdev, curr);

			return ret;
		}
	}
	RTMP_SEM_UNLOCK(&bssmnger.lock);

	os_alloc_mem(NULL, (UINT8 **)&bmgentry, sizeof(struct bmg_entry));
	if (bmgentry == NULL) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				":%s: bmgentry is NULL!! (ret:%d)\n", __func__, ret);
		ret = NDIS_STATUS_INVALID_DATA;
		return ret;
	}
	os_zero_mem(bmgentry, sizeof(struct bmg_entry));

	if (bmgentrycpy(bmgentry, bmgentry_src) == NDIS_STATUS_SUCCESS) {
		entry_list = &bssmnger.entry_list;
		RTMP_SEM_LOCK(&bssmnger.lock);
		DlListAddTail(entry_list, &bmgentry->list);
		bssmnger.dev_cnt++;
		RTMP_SEM_UNLOCK(&bssmnger.lock);

		/* Only self dev can update reported bss list */
		if (strcmp(RtmpOsGetNetDevName(wdev->if_dev), curr_devinfo->ifname) == 0)
			bssmnger_build_reported_bss_list_wdev(wdev, curr);

	} else {
		os_free_mem(bmgentry);
	}

	return ret;
}

NDIS_STATUS bssmnger_get_event_handle(
	PRTMP_ADAPTER pAd,
	UCHAR **buf,
	UCHAR *ifname)
{
	INT ret = NDIS_STATUS_SUCCESS;
	struct bmg_entry *bmgentry = NULL;

	bmgentry = get_bmg_entry_by_ifname(ifname);

	if (bmgentry == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		return ret;
	}

	*buf = (UCHAR *)bmgentry;
	return ret;
}

static void bssmnger_to_wappd_event_handle(
	PNET_DEV pNetDev, enum BSSMNGER_MSG_TYPE type, UINT16 datalen, struct bmg_entry *pEntry)
{
	struct bssmnger_msg *pBssMgmtMsg = NULL;
	UINT16 buflen;
	char *buf;

	buflen = sizeof(struct bssmnger_msg);
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	pBssMgmtMsg = (struct bssmnger_msg *)buf;
	pBssMgmtMsg->ifindex = RtmpOsGetNetIfIndex(pNetDev);
	pBssMgmtMsg->type = type;

	if (pEntry && datalen) {
		pBssMgmtMsg->datalen = datalen;
		NdisCopyMemory(&pBssMgmtMsg->MsgBody, pEntry, datalen);
	}

	RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM,
		OID_802_11_BSS_MGMT_MSG, NULL, (PUCHAR)buf, buflen);

	os_free_mem(buf);
}

NDIS_STATUS indicate_bssentry_to_wappd(struct wifi_dev *wdev, enum BSSMNGER_MSG_TYPE type)
{
	int ret = NDIS_STATUS_SUCCESS;
	struct bmg_entry *bmgentry = NULL;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"bssmnger: msg type: %d\n", type);

	bmgentry = get_bmgentry_by_wdev(wdev);
	if (bmgentry == NULL) {
		ret = NDIS_STATUS_FAILURE;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"bmg entey not found(ret:%d)\n", ret);
		bssmnger_to_wappd_event_handle(wdev->if_dev, type, 0, NULL);
		goto end;
	}

	bssmnger_to_wappd_event_handle(wdev->if_dev, type, sizeof(struct bmg_entry), bmgentry);
end:
	return ret;
}
#endif /* BSSMGR_CROSS_MODULE_SUPPORT */

NDIS_STATUS bssmnger_init(void)
{
	int ret = NDIS_STATUS_SUCCESS;
	DL_LIST *entry_list = NULL;

	if (bssmnger.inited && bssmnger.kernel_band_bitmap)
		goto err;

	entry_list = &bssmnger.entry_list;
	if (entry_list == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: entry is NULL!! (ret:%d)\n", __func__, ret);
		goto err;
	}

	os_zero_mem(&bssmnger, sizeof(struct bss_mnger));
	bssmnger.dev_cnt = 0;
	DlListInit(entry_list);
	NdisAllocateSpinLock(NULL, &bssmnger.lock);
	bssmnger.inited = TRUE;

err:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		":%s: init %s, kernel_band_bmap:0x%x, inited:%d, devCnt:%d\n",
		__func__,  ret == NDIS_STATUS_SUCCESS ? "done":"fail",
		bssmnger.kernel_band_bitmap,
		bssmnger.inited, bssmnger.dev_cnt);
	return ret;
}

NDIS_STATUS bssmnger_deinit(void)
{
	int ret = NDIS_STATUS_SUCCESS;
	DL_LIST *entry_list = NULL;
	struct bmg_entry *bmgentry = NULL, *bmg_entry_tmp = NULL;

	entry_list = &bssmnger.entry_list;
	if (entry_list == NULL) {
		ret = NDIS_STATUS_INVALID_DATA;
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			":%s: entry is NULL!! (ret:%d)\n", __func__, ret);
		goto err;
	}

	RTMP_SEM_LOCK(&bssmnger.lock);
	DlListForEachSafe(bmgentry, bmg_entry_tmp, entry_list, struct bmg_entry, list) {
		if (bmgentry == NULL)
			break;

		DlListDel(&bmgentry->list);
		bssmnger.dev_cnt--;
		/* Clear the reg information including security information */
		memset(bmgentry, 0x00, sizeof(struct bmg_entry));
		os_free_mem(bmgentry);
		bmgentry = NULL;
	}
	RTMP_SEM_UNLOCK(&bssmnger.lock);

	if (bssmnger.dev_cnt == 0) {
		NdisFreeSpinLock(&bssmnger.lock);
		DlListInit(entry_list);
		os_zero_mem(&bssmnger, sizeof(struct bss_mnger));
	}

err:
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		":%s: deinit %s, inited:%d, devCnt:%d\n",
		__func__,  ret == NDIS_STATUS_SUCCESS ? "done":"fail",
		bssmnger.inited, bssmnger.dev_cnt);
	return ret;
}

void bssmnger_show_bsslist_info(void)
{
	DL_LIST *entry_list = NULL;
	struct bmg_entry *bmgentry = NULL;
	struct module_info *modinfo = NULL;
	struct netdev_info *devinfo = NULL;
	struct reg_info *reginfo = NULL;
	struct iob_info *iobinfo = NULL;
	struct oob_info *oobinfo = NULL;
	struct radio_info *radioinfo = NULL;
	struct bss_info *bssinfo = NULL;
	struct sec_info *secinfo = NULL;
	int i = 0;

	const char *iob_type_str[] = {"Off", "UnsolProbeRsp", "FilsDiscovery", "QosNull"};
	const char *iob_mode_str[] = {"Non-HT", "Non-HT-Dup", "HE-SU"};
	const char *oob_rule_str[] = {"Off", "MainBSS", "AllBSS"};
	entry_list = &bssmnger.entry_list;

	MTWF_PRINT("dev_cnt = %d, list_len = %d, kernel_band_bitmap:0x%x\n",
			bssmnger.dev_cnt, DlListLen(entry_list),
			bssmnger.kernel_band_bitmap);

	RTMP_SEM_LOCK(&bssmnger.lock);
	DlListForEach(bmgentry, entry_list, struct bmg_entry, list) {
		if (bmgentry == NULL) {
			MTWF_PRINT("%s: bmg_entry did not exist\n", __func__);
			break;
		}

		modinfo = &bmgentry->modinfo;
		devinfo = &bmgentry->devinfo;
		reginfo = &bmgentry->reginfo;
		iobinfo = &bmgentry->iobinfo;
		oobinfo = &bmgentry->oobinfo;
		radioinfo = &reginfo->radioinfo;
		bssinfo = &reginfo->bssinfo;
		secinfo = &reginfo->secinfo;

		MTWF_PRINT(" - [%02d] %s [mt%x] (%s)\n",
			devinfo->ifindex, devinfo->ifname,
			modinfo->chip_id,
			bmgentry->valid ? "valid" : "invalid");

		MTWF_PRINT("\t\tGrpIdx:%d, BcnTx:%d, Mbss:%d\n",
			bssinfo->mbss_grp_idx, bssinfo->is_trans_bss, bssinfo->is_multi_bss);

		MTWF_PRINT("\t\tSSID:%s (%08x)\t(%02x:%02x:%02x:%02x:%02x:%02x)\n",
			bssinfo->ssid, (UINT32)Crcbitbybitfast(bssinfo->ssid, bssinfo->ssid_len),
			PRINT_MAC(radioinfo->bssid));

		MTWF_PRINT("\t\tPhymode:%s (0x%x), Ch=%3d, OpClass=%d\n",
			wmode_2_str(radioinfo->phymode), radioinfo->phymode, radioinfo->channel, radioinfo->opclass);

		MTWF_PRINT("\t\tAuthMode:%s, Cipher(P:%s/G:%s)\n",
			GetAuthModeStr(secinfo->auth_mode), GetEncryModeStr(secinfo->PairwiseCipher),
			GetEncryModeStr(secinfo->GroupCipher));

		MTWF_PRINT("\t\tIoB Config:\n");
		MTWF_PRINT("\t\t- Type: %s%s ",
			iob_type_str[iobinfo->iob_dsc_type],
			iobinfo->iob_dsc_by_cfg ? "ByCfg" : "");

		if (iobinfo->iob_dsc_type) {
			MTWF_PRINT("(Interval:%dms, Mode:%s)\n",
				iobinfo->iob_dsc_interval,
				iob_mode_str[iobinfo->iob_dsc_txmode]);
		} else
			MTWF_PRINT("\n");

		MTWF_PRINT("\t\tOoB Config:\n");
		MTWF_PRINT("\t\t- Reporting rule 2G:%s, 5G:%s, 6G:%s\n",
			oob_rule_str[oobinfo->repting_rule_2g],
			oob_rule_str[oobinfo->repting_rule_5g],
			oob_rule_str[oobinfo->repting_rule_6g]);

		if (oobinfo->repting_bmap) {
			MTWF_PRINT("\t\t- Reported APs: \n");
			MTWF_PRINT("\t\t- Repting_bmap:%llx ", oobinfo->repting_bmap);
			for (i = 0; i < MAX_NET_IF_CNT; i++) {
				if (oobinfo->repting_bmap & ((UINT64)1 << i))
					MTWF_PRINT("[%02d] ", i);
			}
			MTWF_PRINT("\n");
		}
	}
	RTMP_SEM_UNLOCK(&bssmnger.lock);
}

ULONG ap_6g_build_unsol_bc_probe_rsp(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 *f_buf)
{
	HEADER_802_11 hdr;
	ULONG frame_len = 0;
	LARGE_INTEGER FakeTimestamp;
	BSS_STRUCT *mbss = wdev->func_dev;
	struct legacy_rate *rate = &wdev->rate.legacy_rate;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s, wdev(%d) f_buf = 0x%p\n",
		__func__, wdev->wdev_idx, f_buf);

	MgtMacHeaderInit(pAd, &hdr, SUBTYPE_PROBE_RSP, 0, BROADCAST_ADDR,
						 wdev->if_addr, wdev->bssid);
	MakeOutgoingFrame(f_buf, &frame_len, sizeof(HEADER_802_11), &hdr, TIMESTAMP_LEN, &FakeTimestamp,
		2, &pAd->CommonCfg.BeaconPeriod,
		2, &mbss->CapabilityInfo,
		1, &SsidIe,
		1, &mbss->SsidLen,
		mbss->SsidLen, mbss->Ssid,
		END_OF_ARGS);

	frame_len += build_support_rate_ie(wdev, rate->sup_rate, rate->sup_rate_len, f_buf + frame_len);

#ifdef CONFIG_HOTSPOT_R2
	if ((mbss->HotSpotCtrl.HotSpotEnable == 0) && (mbss->HotSpotCtrl.bASANEnable == 1) &&
		(IS_AKM_WPA2_Entry(wdev))) {
		/* replace RSN IE with OSEN IE if it's OSEN wdev */
		ULONG temp_len = 0;
		UCHAR RSNIe = IE_WPA;
		extern UCHAR OSEN_IE[];
		extern UCHAR OSEN_IELEN;

		MakeOutgoingFrame(f_buf + frame_len, &temp_len,
			1, &RSNIe,
			1, &OSEN_IELEN,
			OSEN_IELEN, OSEN_IE,
			END_OF_ARGS);
		frame_len += temp_len;
	}
#endif /* CONFIG_HOTSPOT_R2 */

	ComposeBcnPktTail(pAd, wdev, &frame_len, f_buf);

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s, Build BC_PROBE_RSP, Len = %ld\n", __func__, frame_len);

	return frame_len;
}

ULONG ap_6g_build_fils_discovery(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 *f_buf)
{
	ULONG frame_len = 0;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"wdev(%d) f_buf = 0x%p\n", wdev->wdev_idx, f_buf);

	frame_len = build_fils_discovery_action(wdev, f_buf);

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Build FILS_DISCOVERY, Len = %ld\n", frame_len);

	return frame_len;
}

ULONG ap_6g_build_qos_null_injector(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 *f_buf)
{
	ULONG frame_len = 0;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"wdev(%d) f_buf = 0x%p\n", wdev->wdev_idx, f_buf);

	frame_len = build_qos_null_injector(wdev, f_buf);

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Build QOS_NULL, Len = %ld\n", frame_len);

	return frame_len;
}

NDIS_STATUS ap_6g_build_discovery_frame(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	pdiscov_iob dsc_iob = &wdev->ap6g_cfg.dsc_iob;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	PUCHAR pkt = NULL;
	ULONG frame_len = 0;
	UCHAR type, sub_type;
	/* MGMT frame PHY rate setting when operatin at HT rate. */
	HTTRANSMIT_SETTING TransmitSet = {.word = 0};
	UCHAR iob_type = wlan_config_get_unsolicit_tx_type(wdev);
	UCHAR iob_mode = wlan_config_get_unsolicit_tx_mode(wdev);

	if (!dsc_iob->pkt_buf) {
		MTWF_DBG(pAd, DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "pkt_buf = NULL!\n");
		return NDIS_STATUS_FAILURE;
	}

	pkt = (UCHAR *)(dsc_iob->pkt_buf + tx_hw_hdr_len);

	RTMP_SEM_LOCK(&dsc_iob->pkt_lock);

	switch (iob_type) {
	case UNSOLICIT_TX_PROBE_RSP:
		type =		FC_TYPE_MGMT;
		sub_type =	SUBTYPE_PROBE_RSP;
		frame_len =	ap_6g_build_unsol_bc_probe_rsp(pAd, wdev, pkt);
		break;
	case UNSOLICIT_TX_FILS_DISC:
		type =		FC_TYPE_MGMT;
		sub_type =	SUBTYPE_ACTION;
		frame_len =	ap_6g_build_fils_discovery(pAd, wdev, pkt);
		break;
	default:
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"discovery frame disabled (%d)\n", iob_type);
		break;
	}

	if (frame_len) {
		/* fixed-rate option */
		if (iob_mode == UNSOLICIT_TXMODE_NON_HT_DUP) {
			TransmitSet.field.BW = BW_80;
			TransmitSet.field.MODE = MODE_OFDM;
			TransmitSet.field.MCS = MCS_RATE_6;
		/* HWITS00026034(B): temporary disable HE mode (HE-PE issue) */
		// } else if (iob_mode == UNSOLICIT_TXMODE_HE_SU) {
		//	TransmitSet.field.MODE	= MODE_HE;	/* temporarily */
		//	TransmitSet.field.MCS	= MCS_0;
		/* HWITS00026034(E): temporary disable HE mode (HE-PE issue) */
		} else {
			TransmitSet.field.BW = BW_20;
			TransmitSet.field.MODE = MODE_OFDM;
			TransmitSet.field.MCS = MCS_RATE_6;
		}

		write_tmac_info_offload_pkt(pAd, wdev, type, sub_type,
			dsc_iob->pkt_buf, &TransmitSet, frame_len);
	}
	dsc_iob->pkt_len = frame_len;

	RTMP_SEM_UNLOCK(&dsc_iob->pkt_lock);

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Type(%d), Len(%ld), mode:%d, mcs:%d\n",
		iob_type, frame_len, TransmitSet.field.MODE, TransmitSet.field.MCS);

	return NDIS_STATUS_SUCCESS;
}

#endif /* CONFIG_6G_SUPPORT */
