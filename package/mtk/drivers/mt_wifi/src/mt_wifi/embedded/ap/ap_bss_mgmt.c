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
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/
#include "rt_config.h"
#include "ap_bss_mgmt.h"
#include "action.h"

#ifdef CONFIG_6G_SUPPORT

BSS_MGMT bss_mgmt;

static PBMG_ENTRY get_bss_entry_by_netdev(
	IN PNET_DEV pNetDev
)
{
	PDL_LIST entry_list = NULL;
	PBMG_ENTRY bmg_entry = NULL;

	entry_list = &bss_mgmt.entry_list;

	if (DlListLen(entry_list) == 0)
		return NULL;

	DlListForEach(bmg_entry, entry_list, BMG_ENTRY, List) {
		if (bmg_entry && (bmg_entry->pNetDev == pNetDev))
			return bmg_entry;
	}

	return NULL;
}

static PBMG_ENTRY get_bss_entry_by_ifindex(
	IN UINT32 ifindex
)
{
	PDL_LIST entry_list = NULL;
	PBMG_ENTRY bmg_entry = NULL;

	entry_list = &bss_mgmt.entry_list;

	if (DlListLen(entry_list) == 0)
		return NULL;

	DlListForEach(bmg_entry, entry_list, BMG_ENTRY, List) {
		if (bmg_entry &&
			(RtmpOsGetNetIfIndex(bmg_entry->pNetDev) == ifindex)) {
			return bmg_entry;
		}
	}

	return NULL;
}

static BOOLEAN is_oob_discovery_required(
	IN PBMG_ENTRY	pself_entry,
	IN PBMG_ENTRY	pnhbr_entry
)
{
	USHORT phymode = pnhbr_entry->entry_info.phymode;
	BOOLEAN required = FALSE;
	UCHAR report_en = RNR_REPORTING_NONE;

	if (!pself_entry->valid)
		return required;

	/* only transmitted-bss is capable to bring RNR  */
	if (pnhbr_entry->entry_info.is_trans_bss) {
		if (WMODE_CAP_6G(phymode))
			report_en = pself_entry->repting_rule_6g;
		else if (WMODE_CAP_2G(phymode))
			report_en = pself_entry->repting_rule_2g;
		else if (WMODE_CAP_5G(phymode))
			report_en = pself_entry->repting_rule_5g;

		if ((report_en == RNR_REPORTING_ALL_BSS) ||
			((report_en == RNR_REPORTING_MAIN_BSS) &&
			(pnhbr_entry->entry_info.mbss_grp_idx == MAIN_MBSSID))) {
			required = TRUE;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "\[%02d] neighbor bring RNR required = %d\n",
			 RtmpOsGetNetIfIndex(pnhbr_entry->pNetDev), required);

	return required;
}

static NDIS_STATUS bss_mgmt_con_build_repted_bss_list(
	IN PBMG_ENTRY		pentry
)
{
	PBMG_ENTRY		p_nhbr;
	pbmg_rsp_param	p_rsp_repted_bss = NULL;
	UINT64 repting_bmap = pentry->repting_bmap;
	pbmg_repted_bss		prepted_bss;
	UINT32 ifindex;
	prepted_bss_info bss_info;
	UINT32 max_repted_bss_cnt;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s, [%02d] reported bss bitmap = 0x%llx\n",
			 RtmpOsGetNetIfIndex(pentry->pNetDev), repting_bmap);

	os_alloc_mem(NULL, (UCHAR **)&p_rsp_repted_bss, sizeof(bmg_rsp_param));
	if (p_rsp_repted_bss == NULL) {
		MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "can not allocate bmg_rsp_param value\n");
		return NDIS_STATUS_FAILURE;
	}
	os_zero_mem(p_rsp_repted_bss, sizeof(bmg_rsp_param));
	p_rsp_repted_bss->rsp_id	= BMG_RESP_REPTED_BSS_INFO;
	p_rsp_repted_bss->len		= sizeof(bmg_repted_bss);

	prepted_bss = &(p_rsp_repted_bss->data.repted_bss);
	max_repted_bss_cnt = sizeof(prepted_bss->repted_bss_list)/sizeof(repted_bss_info);

	if (!repting_bmap)
		prepted_bss->repted_bss_cnt = 0;
	else {
		for (ifindex = 0; ifindex < MAX_NET_IF_CNT; ifindex++) {
			if (repting_bmap & ((UINT64)1 << ifindex)) {
				p_nhbr = get_bss_entry_by_ifindex(ifindex);

				if (p_nhbr) {
					/* avoid overflow */
					if (prepted_bss->repted_bss_cnt >= max_repted_bss_cnt)
						continue;

					/* append reported bss info */
					bss_info = (prepted_bss_info)prepted_bss->repted_bss_list +
								prepted_bss->repted_bss_cnt;

					/* fill bss info */
					bss_info->phymode		= p_nhbr->entry_info.phymode;
					bss_info->bss_grp_idx	= p_nhbr->entry_info.mbss_grp_idx;
					bss_info->channel		= p_nhbr->entry_info.channel;
					bss_info->op_class		= p_nhbr->entry_info.op_class;
					bss_info->ssid_len		= p_nhbr->entry_info.ssid_len;
					NdisMoveMemory(bss_info->ssid, p_nhbr->entry_info.ssid, bss_info->ssid_len);
					NdisMoveMemory(bss_info->bssid, p_nhbr->entry_info.bssid, MAC_ADDR_LEN);

					bss_info->bss_feature_set |=
						((p_nhbr->entry_info.is_trans_bss) ? AP_6G_TRANS_BSSID : 0) |
						((p_nhbr->entry_info.is_multi_bss) ? AP_6G_MULTI_BSSID : 0) |
						((p_nhbr->iob_dsc_type == UNSOLICIT_TX_PROBE_RSP) ?
							AP_6G_UNSOL_PROBE_RSP_EN : 0);

					MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "\t[%02d] add reported bss [%02d], feature_set =0x%x\n",
							 RtmpOsGetNetIfIndex(pentry->pNetDev), ifindex,
							 bss_info->bss_feature_set);

					prepted_bss->repted_bss_cnt++;
				}
			}
		}
	}

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "\tTotal Reported BSS Count = %d\n", prepted_bss->repted_bss_cnt);

	/* deliver RNR list to target bss */
	if (pentry->entry_info.bss_mgmt_get_info_rsp) {
		pentry->entry_info.bss_mgmt_get_info_rsp(
				RtmpOsGetNetDevWdev(pentry->pNetDev),
				(UCHAR *)p_rsp_repted_bss,
				sizeof(bmg_rsp_param));
	}

	if (p_rsp_repted_bss)
		os_free_mem(p_rsp_repted_bss);
	return NDIS_STATUS_SUCCESS;
}

static NDIS_STATUS bss_mgmt_con_renew_repted_bss_list(
	IN PBMG_ENTRY pentry,
	IN BOOLEAN bforce
)
{
	UINT32 ifindex = RtmpOsGetNetIfIndex(pentry->pNetDev);
	PDL_LIST entry_list = &bss_mgmt.entry_list;
	PBMG_ENTRY p_nhbr = NULL;
	UINT64 self_bmap_backup = pentry->repting_bmap;
	UINT64 self_bmap_tmp = pentry->repting_bmap;
	UINT64 nhbr_bmap_backup;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "===> %s: [%02d], force = %d\n",
			 __func__, ifindex, bforce);

	/* traverse BSSs that need reporting/reported RNR */
	DlListForEach(p_nhbr, entry_list, BMG_ENTRY, List) {
		if (p_nhbr && (p_nhbr != pentry)) {
			UINT32 nifindex = RtmpOsGetNetIfIndex(p_nhbr->pNetDev);

			/* RNR to neighbor */
			nhbr_bmap_backup = p_nhbr->repting_bmap;
			if (is_oob_discovery_required(pentry, p_nhbr))
				p_nhbr->repting_bmap |= ((UINT64)1 << ifindex);
			else
				p_nhbr->repting_bmap &= ~((UINT64)1 << ifindex);

			/* bitmap changed */
			if ((p_nhbr->repting_bmap != nhbr_bmap_backup) || bforce) {
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "\t%s:%d N[%02d] %s RNR_S[%02d], bitmap = 0x%llx -> 0x%llx\n",
						 __func__, __LINE__, nifindex,
						 (p_nhbr->repting_bmap & ((UINT64)1 << ifindex)) ? "+" : "-",
						 ifindex, nhbr_bmap_backup, p_nhbr->repting_bmap);
				bss_mgmt_con_build_repted_bss_list(p_nhbr);
			}
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"\t%s:%d N[%02d] %s RNR_S[%02d], bitmap = 0x%llx -> 0x%llx\n",
				__func__, __LINE__, nifindex,
				(p_nhbr->repting_bmap & ((UINT64)1 << ifindex)) ? "+" : "-",
				ifindex, nhbr_bmap_backup, p_nhbr->repting_bmap);

			/* RNR to self */
			self_bmap_tmp = pentry->repting_bmap;
			if (is_oob_discovery_required(p_nhbr, pentry))
				pentry->repting_bmap |= ((UINT64)1 << nifindex);
			else
				pentry->repting_bmap &= ~((UINT64)1 << nifindex);

			/* bitmap changed */
			if (pentry->repting_bmap != self_bmap_tmp) {
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "\t%s:%d S[%02d] %s RNR_N[%02d], bitmap = 0x%llx -> 0x%llx\n",
						 __func__, __LINE__, RtmpOsGetNetIfIndex(pentry->pNetDev),
						 (pentry->repting_bmap & ((UINT64)1 << nifindex)) ? "+" : "-",
						 nifindex, self_bmap_tmp, pentry->repting_bmap);
			}
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "\t%s:%d S[%02d] %s RNR_N[%02d], bitmap = 0x%llx -> 0x%llx\n",
						 __func__, __LINE__, RtmpOsGetNetIfIndex(pentry->pNetDev),
						 (pentry->repting_bmap & ((UINT64)1 << nifindex)) ? "+" : "-",
						 nifindex, self_bmap_tmp, pentry->repting_bmap);
		}
	}

	if (pentry->valid && pentry->pNetDev &&
		((pentry->repting_bmap ^ self_bmap_backup) || bforce))	/* self bitmap changed */
		bss_mgmt_con_build_repted_bss_list(pentry);

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "<=== %s:\n", __func__);

	return NDIS_STATUS_SUCCESS;
}

static BOOLEAN is_ap_2g_5g_exist(void)
{
	PBMG_ENTRY bmg_entry = NULL;
	PDL_LIST entry_list = &bss_mgmt.entry_list;
	USHORT phymode = 0;
	UCHAR ap_2g5g_cnt = 0, ap_6g_cnt = 0;

	/* traverse BSSs that is 6 GHz only or multi-band  */
	DlListForEach(bmg_entry, entry_list, BMG_ENTRY, List) {
		if (bmg_entry && bmg_entry->valid) {
			phymode = bmg_entry->entry_info.phymode;
			if (WMODE_CAP_6G(phymode))
				ap_6g_cnt++;
			else if (WMODE_CAP_2G(phymode) || WMODE_CAP_5G(phymode))
				ap_2g5g_cnt++;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"\t2g5g_cnt(%d), 6g_cnt(%d)\n", ap_2g5g_cnt, ap_6g_cnt);

	return ap_2g5g_cnt ? TRUE : FALSE;
}

static BOOLEAN bss_mgmt_con_renew_6g_iob_rule(
	IN PBMG_ENTRY pentry
)
{
	PBMG_ENTRY bmg_entry = NULL;
	UINT32 ifindex = RtmpOsGetNetIfIndex(pentry->pNetDev);
	PDL_LIST entry_list = &bss_mgmt.entry_list;
	USHORT phymode = 0;
	bool is_2g5g_exist = is_ap_2g_5g_exist();
	bool iob_type_chg = false;
	pbmg_rsp_param	p_rsp_iob_chg = NULL;

	if (pentry->valid) {
		if (pentry->iob_dsc_by_cfg) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "===> %s: [%02d] ignored by cfg\n",
				__func__, ifindex);
			return false;
		}

		if (!pentry->entry_info.is_trans_bss) {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "===> %s: [%02d] ignored by ntx\n",
				__func__, ifindex);
			return false;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "===> %s: [%02d], 2g5g_exist(%d)\n",
			 __func__, ifindex, is_2g5g_exist);

	/*
	 * 1. AP is operating in 6 GHz only
	 *     - allowed to transmit either UPR or FD frames
	 *
	 * 2. AP is operating in multiple bands (including 6 GHz)
	 *     - Disable transmission of UPR and FD frames
	 */
	DlListForEach(bmg_entry, entry_list, BMG_ENTRY, List) {
		if (bmg_entry && bmg_entry->valid) {
			if (bmg_entry->iob_dsc_by_cfg) {
				MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"\t[%02d] iob dsc ignored by cfg\n",
					RtmpOsGetNetIfIndex(bmg_entry->pNetDev));
				continue;
			}

			phymode = bmg_entry->entry_info.phymode;
			if (WMODE_CAP_6G(phymode) && (bmg_entry->entry_info.is_trans_bss)) {
				if (!is_2g5g_exist) {
					if (bmg_entry->iob_dsc_type == UNSOLICIT_TX_DISABLE) {
						bmg_entry->iob_dsc_type = UNSOLICIT_TX_PROBE_RSP;
						iob_type_chg = true;
						MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"\t[%02d] iob dsc enabled(%d)\n",
								RtmpOsGetNetIfIndex(bmg_entry->pNetDev),
								bmg_entry->iob_dsc_type);
					}
				} else {
					if (bmg_entry->iob_dsc_type != UNSOLICIT_TX_DISABLE) {
						bmg_entry->iob_dsc_type = UNSOLICIT_TX_DISABLE;
						iob_type_chg = true;
						MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"\t[%02d] iob dsc disabled\n",
								RtmpOsGetNetIfIndex(bmg_entry->pNetDev));
					}
				}

				/* write back iob type */
				if (iob_type_chg) {
					os_alloc_mem(NULL, (UCHAR **)&p_rsp_iob_chg, sizeof(bmg_rsp_param));
					if (p_rsp_iob_chg == NULL) {
						MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								 "can not allocate p_rsp_iob_chg buf\n");
						return NDIS_STATUS_FAILURE;
					}

					os_zero_mem(p_rsp_iob_chg, sizeof(bmg_rsp_param));
					p_rsp_iob_chg->rsp_id		= BMG_RESP_IOB_TYPE_CHG;
					p_rsp_iob_chg->len			= sizeof(UINT32);
					p_rsp_iob_chg->data.value	= bmg_entry->iob_dsc_type;

					if (bmg_entry->entry_info.bss_mgmt_get_info_rsp) {
						bmg_entry->entry_info.bss_mgmt_get_info_rsp(
								RtmpOsGetNetDevWdev(bmg_entry->pNetDev),
								(UCHAR *)p_rsp_iob_chg,
								sizeof(bmg_rsp_param));
					}

					if (p_rsp_iob_chg)
						os_free_mem(p_rsp_iob_chg);

				}
			}
		}
	}

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"<=== %s: [%02d]\n", __func__, ifindex);

	return iob_type_chg;
}

static NDIS_STATUS bss_mgmt_con_set_discovery_rule(
	IN PNET_DEV pNetDev,
	IN pbmg_discov_rule discov_rule
)
{
	PBMG_ENTRY bmg_entry = get_bss_entry_by_netdev(pNetDev);
	BOOLEAN rule_change = FALSE;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "===> %s: [%02d]\n",
			 __func__, RtmpOsGetNetIfIndex(pNetDev));

	if (bmg_entry) {
		if ((bmg_entry->iob_dsc_type != discov_rule->iob_type) ||
			(bmg_entry->repting_rule_2g != discov_rule->oob_repting_2g) ||
			(bmg_entry->repting_rule_5g != discov_rule->oob_repting_5g) ||
			(bmg_entry->repting_rule_6g != discov_rule->oob_repting_6g))
			rule_change = TRUE;

		bmg_entry->iob_dsc_type		= discov_rule->iob_type;
		bmg_entry->iob_dsc_interval	= discov_rule->iob_interval;
		bmg_entry->iob_dsc_txmode	= discov_rule->iob_txmode;
		bmg_entry->iob_dsc_by_cfg	= discov_rule->iob_by_cfg;
		bmg_entry->repting_rule_2g	= discov_rule->oob_repting_2g;
		bmg_entry->repting_rule_5g	= discov_rule->oob_repting_5g;
		bmg_entry->repting_rule_6g	= discov_rule->oob_repting_6g;

		bss_mgmt_con_renew_repted_bss_list(bmg_entry, rule_change);
	} else {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "entry not found!!\n");
		goto exit;
	}


	/* check 6G AP allowed to tx UPR or FD frames */
	if (bss_mgmt_con_renew_6g_iob_rule(bmg_entry))
		bss_mgmt_con_renew_repted_bss_list(bmg_entry, TRUE);


	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "<=== %s:\n", __func__);

exit:
	return NDIS_STATUS_SUCCESS;
}

static NDIS_STATUS bss_mgmt_con_dev_reg(
	IN PNET_DEV pNetDev,
	IN PBMG_DEV_REG_INFO reg_info
)
{
	PDL_LIST entry_list = NULL;
	PBMG_ENTRY bmg_entry = NULL;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "===> [%02d]\n",
			 RtmpOsGetNetIfIndex(pNetDev));

	if (get_bss_entry_by_netdev(pNetDev)) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "  entry exist!!\n");
		goto exit;
	}

	os_alloc_mem(NULL, (UCHAR **)&bmg_entry, sizeof(BMG_ENTRY));

	entry_list = &bss_mgmt.entry_list;

	if (bmg_entry) {
		NdisZeroMemory(bmg_entry, sizeof(BMG_ENTRY));

		bmg_entry->pNetDev = pNetDev;
		bmg_entry->valid = TRUE;
		NdisMoveMemory(&bmg_entry->entry_info, reg_info, sizeof(BMG_DEV_REG_INFO));

		RTMP_SEM_LOCK(&bss_mgmt.lock);
		DlListAddTail(entry_list, &bmg_entry->List);

		bss_mgmt.dev_cnt++;
		RTMP_SEM_UNLOCK(&bss_mgmt.lock);

		bss_mgmt_con_renew_repted_bss_list(bmg_entry, FALSE);
	} else {
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "entry alloc failed\n");
	}

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "<=== \n");

exit:
	return NDIS_STATUS_SUCCESS;
}

static NDIS_STATUS bss_mgmt_con_dev_dereg(
	IN PNET_DEV pNetDev
)
{
	PBMG_ENTRY bmg_entry = NULL;
	PDL_LIST entry_list = NULL;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "===> [%02d]\n",
			 RtmpOsGetNetIfIndex(pNetDev));

	entry_list = &bss_mgmt.entry_list;
	RTMP_SEM_LOCK(&bss_mgmt.lock);
	DlListForEach(bmg_entry, entry_list, BMG_ENTRY, List) {
		if (bmg_entry && (bmg_entry->pNetDev == pNetDev)) {
			bmg_entry->valid = FALSE;

			/* clear self reporting rule and delivering */
			bmg_entry->repting_rule_2g = RNR_REPORTING_NONE;
			bmg_entry->repting_rule_5g = RNR_REPORTING_NONE;
			bmg_entry->repting_rule_6g = RNR_REPORTING_NONE;
			bss_mgmt_con_renew_repted_bss_list(bmg_entry, FALSE);

			DlListDel(&bmg_entry->List);
			bss_mgmt.dev_cnt--;
			os_free_mem(bmg_entry);
			bmg_entry = NULL;
			break;
		}
	}
	RTMP_SEM_UNLOCK(&bss_mgmt.lock);

	/* check 6G AP allowed to tx UPR or FD frames */
	if (bmg_entry) {
		if (bss_mgmt_con_renew_6g_iob_rule(bmg_entry))
			bss_mgmt_con_renew_repted_bss_list(bmg_entry, TRUE);
	}
	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "<=== \n");

	return NDIS_STATUS_SUCCESS;
}

static NDIS_STATUS bss_mgmt_con_init(VOID)
{
	PDL_LIST entry_list = NULL;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "inited = %d\n",
		bss_mgmt.inited);

	entry_list = &bss_mgmt.entry_list;

	if (!bss_mgmt.inited) {
		os_zero_mem(&bss_mgmt, sizeof(BSS_MGMT));

		NdisAllocateSpinLock(NULL, &bss_mgmt.lock);
		DlListInit(entry_list);

		bss_mgmt.inited = TRUE;
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, " init\n");
	} else {
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, " inited\n");
	}

	return NDIS_STATUS_SUCCESS;
}

static NDIS_STATUS bss_mgmt_con_deinit(VOID)
{
	PBMG_ENTRY bmg_entry = NULL, bmg_entry_tmp = NULL;
	PDL_LIST entry_list = NULL;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "dev_cnt = %d\n", bss_mgmt.dev_cnt);

	if (bss_mgmt.inited) {
		entry_list = &bss_mgmt.entry_list;
		DlListForEachSafe(bmg_entry, bmg_entry_tmp, entry_list, BMG_ENTRY, List) {
			if (bmg_entry)
				bss_mgmt_con_dev_dereg(bmg_entry->pNetDev);
		}

		if (bss_mgmt.dev_cnt == 0) {
			NdisFreeSpinLock(&bss_mgmt.lock);
			bss_mgmt.inited = FALSE;
			MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, " de-init\n");
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "de-inited\n");
	}

	return NDIS_STATUS_SUCCESS;
}

static NDIS_STATUS bss_mgmt_con_info_show(
	IN PNET_DEV pNetDev
)
{
	PDL_LIST entry_list = NULL;
	PBMG_ENTRY bmg_entry = NULL;
	PBMG_DEV_REG_INFO reg_info = NULL;
	UCHAR *str = NULL;
	INT i;
	static const char *iob_type_str[] = {"Off", "UnsolProbeRsp", "FilsDiscovery", "QosNull"};
	static const char *iob_mode_str[] = {"Non-HT", "Non-HT-Dup", "HE-SU"};
	static const char *oob_rule_str[] = {"Off", "MainBSS", "AllBSS"};

	entry_list = &bss_mgmt.entry_list;
	MTWF_PRINT("%s: dev_cnt = %d, list_len = %d\n",
			 __func__, bss_mgmt.dev_cnt, DlListLen(entry_list));

	DlListForEach(bmg_entry, entry_list, BMG_ENTRY, List) {
		if (bmg_entry) {
			reg_info = &bmg_entry->entry_info;
			MTWF_PRINT(" - [%02d] %s (%s)\n",
					 RtmpOsGetNetIfIndex(bmg_entry->pNetDev),
					 RtmpOsGetNetDevName(bmg_entry->pNetDev),
					 bmg_entry->valid ? "valid" : "invalid");
			MTWF_PRINT("\t\tNetDev:%p, GrpIdx:%d, BcnTx:%d, Mbss:%d\n",
					 bmg_entry->pNetDev,
					 reg_info->mbss_grp_idx,
					 reg_info->is_trans_bss,
					 reg_info->is_multi_bss);
			MTWF_PRINT("\t\tSSID:%s (%08x)\t(%02x-%02x-%02x-%02x-%02x-%02x)\n",
					 reg_info->ssid,
					 (UINT32)Crcbitbybitfast(reg_info->ssid, reg_info->ssid_len),
					 PRINT_MAC(reg_info->bssid));

			str = wmode_2_str(reg_info->phymode);
			if (str) {
				MTWF_PRINT("\t\tPhymode:%s (0x%x), Ch=%3d, OpClass=%d\n",
						 str, reg_info->phymode, reg_info->channel, reg_info->op_class);
				os_free_mem(str);
			}

			MTWF_PRINT("\t\tAuthMode:%s, Cipher(P:%s/G:%s)\n",
					 GetAuthModeStr(reg_info->auth_mode),
					 GetEncryModeStr(reg_info->PairwiseCipher),
					 GetEncryModeStr(reg_info->GroupCipher));

			/* iut-of-band discovery config */
			MTWF_PRINT("\t\tIoB Config:\n");
			MTWF_PRINT("\t\t- Type: %s%s ",
					 iob_type_str[bmg_entry->iob_dsc_type],
					 bmg_entry->iob_dsc_by_cfg ? "ByCfg" : "");

			if (bmg_entry->iob_dsc_type) {
				MTWF_PRINT("(Interval:%dms, Mode:%s)\n",
						 bmg_entry->iob_dsc_interval,
						 iob_mode_str[bmg_entry->iob_dsc_txmode]);
			} else
				MTWF_PRINT("\n");

			/* out-of-band discovery config */
			MTWF_PRINT("\t\tOoB Config:\n");
			MTWF_PRINT("\t\t- Reporting rule 2G:%s, 5G:%s, 6G:%s\n",
					 oob_rule_str[bmg_entry->repting_rule_2g],
					 oob_rule_str[bmg_entry->repting_rule_5g],
					 oob_rule_str[bmg_entry->repting_rule_6g]);

			MTWF_PRINT("\t\t- Reported APs (Ryan): repting_bmap:%llx\n", bmg_entry->repting_bmap);
			if (bmg_entry->repting_bmap) {
				MTWF_PRINT("\t\t- Reported APs: ");
				for (i = 0; i < MAX_NET_IF_CNT; i++) {
					if (bmg_entry->repting_bmap & ((UINT64)1 << i))
						MTWF_PRINT("[%02d] repting_bmap:%llx", i, bmg_entry->repting_bmap);
				}
				MTWF_PRINT("\n");
			}
		}
	}

	return NDIS_STATUS_SUCCESS;
}

/* set info to bss manager */
static NDIS_STATUS bss_mgmt_con_set_info(
	IN PNET_DEV				pNetDev,
	IN PBMG_SET_PARAM		info
)
{
	INT ret = NDIS_STATUS_SUCCESS;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: NetDev[%d], Cmd(%d), Len(%d)\n",
			 __func__, (pNetDev != NULL) ? RtmpOsGetNetIfIndex(pNetDev) : -1,
			 info->cmd_id, info->len);

	switch (info->cmd_id) {
	case BMG_SET_INIT:
		bss_mgmt_con_init();
		break;
	case BMG_SET_DEINIT:
		bss_mgmt_con_deinit();
		break;
	case BMG_SET_DEV_REG:
		if (info->len >= sizeof(BMG_DEV_REG_INFO)) {
			bss_mgmt_con_dev_reg(pNetDev, &info->data.dev_reg_info);
		} else {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "data length not enough (%u < %u)\n", info->len, (UINT)sizeof(BMG_DEV_REG_INFO));
			ret = NDIS_STATUS_FAILURE;
		}
		break;
	case BMG_SET_DEV_DEREG:
		bss_mgmt_con_dev_dereg(pNetDev);
		break;
	case BMG_SET_DISCOV_RULE:
		if (info->len >= sizeof(bmg_discov_rule)) {
			bss_mgmt_con_set_discovery_rule(pNetDev, &info->data.discov_rule);
		} else {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "data length not enough (%u < %u)\n", info->len, (UINT)sizeof(bmg_discov_rule));
			ret = NDIS_STATUS_FAILURE;
		}
		break;
	case BMG_GET_2G_5G_EXIST:
		info->data.data = is_ap_2g_5g_exist() ? TRUE : FALSE;
		break;
	case BMG_SET_SHOW_INFO:
		bss_mgmt_con_info_show(pNetDev);
		break;
	default:
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid bss_mgmt set id=%d\n", info->cmd_id);
		ret = NDIS_STATUS_FAILURE;
		break;
	}

	return ret;
}

NDIS_STATUS bss_mgmt_rsp_handler(
	IN struct wifi_dev		*wdev,
	IN UCHAR				*rsp,
	IN UINT16				rsp_len
)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	INT ret = NDIS_STATUS_SUCCESS;
	pbmg_rsp_param rsp_param = (pbmg_rsp_param)rsp;

	MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s: wdev(%d), rsp_id(%d), rsp_len(%d)\n",
			 wdev->wdev_idx, rsp_param->rsp_id, rsp_param->len);

	switch (rsp_param->rsp_id) {
	case BMG_RESP_REPTED_BSS_INFO:
	{
		pdiscov_oob dsc_oob = &wdev->ap6g_cfg.dsc_oob;

		if (dsc_oob->repted_bss_list) {
			RTMP_SEM_LOCK(&dsc_oob->list_lock);
			dsc_oob->repted_bss_cnt = rsp_param->data.repted_bss.repted_bss_cnt;
			if (dsc_oob->repted_bss_cnt > MAX_REPTED_BSS_CNT)	/* avoid overflow */
				dsc_oob->repted_bss_cnt = MAX_REPTED_BSS_CNT
			NdisMoveMemory(dsc_oob->repted_bss_list,
						   rsp_param->data.repted_bss.repted_bss_list,
						   dsc_oob->repted_bss_cnt * sizeof(repted_bss_info));

			MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "wdev(%d), reported bss count(%d), list_prt %p\n",
					 wdev->wdev_idx, dsc_oob->repted_bss_cnt, dsc_oob->repted_bss_list);

			if (dsc_oob->repted_bss_cnt) {
				int i;

				for (i = 0; i < dsc_oob->repted_bss_cnt; i++) {
					MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "repted_bss %d:\n", i);
					hex_dump_with_cat_and_lvl("HEX:", (char *)(dsc_oob->repted_bss_list + i),
											sizeof(repted_bss_info),
											DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR);
				}
			}
			RTMP_SEM_UNLOCK(&dsc_oob->list_lock);
		} else {
			dsc_oob->repted_bss_cnt = 0;
			MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "wdev(%d), reported bss count(%d), list_prt = NULL!\n", wdev->wdev_idx, dsc_oob->repted_bss_cnt);
		}

		/* sync RNR IEs to beacon frame */
		UpdateBeaconHandler(ad, wdev, BCN_UPDATE_IE_CHG);
		break;
	}
	case BMG_RESP_IOB_TYPE_CHG:
	{
		UINT8 new_iob_type = (UINT8)rsp_param->data.value;
		UINT8 iob_type = wlan_config_get_unsolicit_tx_type(wdev);

		if (new_iob_type != iob_type) {
			wlan_config_set_unsolicit_tx_type(wdev, new_iob_type);
			MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "%s: wdev(%d), iob type changed %d -> %d\n", __func__,
					 wdev->wdev_idx, iob_type, new_iob_type);
		}
		break;
	}
	default:
		MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid rsp_id=%d\n", rsp_param->rsp_id);
		ret = NDIS_STATUS_FAILURE;
		break;
	}

	return ret;
}

ULONG ap_6g_build_unsol_bc_probe_rsp(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT8 *f_buf)
{
	HEADER_802_11 hdr;
	ULONG frame_len = 0;
	LARGE_INTEGER FakeTimestamp;
	BSS_STRUCT *mbss = wdev->func_dev;
	struct legacy_rate *rate = &wdev->rate.legacy_rate;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s, wdev(%d) f_buf = 0x%p\n",
			 __func__, wdev->wdev_idx, f_buf);

	MgtMacHeaderInit(pAd, &hdr, SUBTYPE_PROBE_RSP, 0, BROADCAST_ADDR,
						 wdev->if_addr, wdev->bssid);
	MakeOutgoingFrame(f_buf,					  &frame_len,
					  sizeof(HEADER_802_11),	  &hdr,
					  TIMESTAMP_LEN,			  &FakeTimestamp,
					  2,						  &pAd->CommonCfg.BeaconPeriod,
					  2,						  &mbss->CapabilityInfo,
					  1,						  &SsidIe,
					  1,						  &mbss->SsidLen,
					  mbss->SsidLen,			  mbss->Ssid,
					  END_OF_ARGS);

	frame_len += build_support_rate_ie(wdev, rate->sup_rate, rate->sup_rate_len, f_buf + frame_len);

#ifdef CONFIG_HOTSPOT_R2
	if ((mbss->HotSpotCtrl.HotSpotEnable == 0) && (mbss->HotSpotCtrl.bASANEnable == 1) &&
		(IS_AKM_WPA2_Entry(wdev))) {
		/* replace RSN IE with OSEN IE if it's OSEN wdev */
		ULONG temp_len = 0;
		UCHAR RSNIe = IE_WPA;
		extern UCHAR			OSEN_IE[];
		extern UCHAR			OSEN_IELEN;

		MakeOutgoingFrame(f_buf + frame_len,			&temp_len,
						  1,							&RSNIe,
						  1,							&OSEN_IELEN,
						  OSEN_IELEN,					OSEN_IE,
						  END_OF_ARGS);
		frame_len += temp_len;
	} else
#endif /* CONFIG_HOTSPOT_R2 */

	ComposeBcnPktTail(pAd, wdev, &frame_len, f_buf);

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s, Build BC_PROBE_RSP, Len = %ld\n",
			 __func__, frame_len);

	return frame_len;
}

ULONG ap_6g_build_fils_discovery
(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT8 *f_buf)
{
	ULONG frame_len = 0;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev(%d) f_buf = 0x%p\n",
			 wdev->wdev_idx, f_buf);

	frame_len = build_fils_discovery_action(wdev, f_buf);

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Build FILS_DISCOVERY, Len = %ld\n",
			 frame_len);

	return frame_len;
}

ULONG ap_6g_build_qos_null_injector(struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev, UINT8 *f_buf)
{
	ULONG frame_len = 0;

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev(%d) f_buf = 0x%p\n",
			 wdev->wdev_idx, f_buf);

	frame_len = build_qos_null_injector(wdev, f_buf);

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Build QOS_NULL, Len = %ld\n",
			 frame_len);

	return frame_len;
}

NDIS_STATUS ap_6g_build_discovery_frame(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev)
{
	pdiscov_iob dsc_iob = &wdev->ap6g_cfg.dsc_iob;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	PUCHAR pkt = NULL;
	ULONG frame_len = 0;
	UCHAR type, sub_type;
	HTTRANSMIT_SETTING TransmitSet = {.word = 0};   /* MGMT frame PHY rate setting when operatin at HT rate. */
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
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "discovery frame disabled (%d)\n", iob_type);
		break;
	}

	if (frame_len) {
		/* fixed-rate option */
		if (iob_mode == UNSOLICIT_TXMODE_NON_HT_DUP) {
			TransmitSet.field.BW		= BW_80;
			TransmitSet.field.MODE	= MODE_OFDM;
			TransmitSet.field.MCS	= MCS_RATE_6;
		} else {
			TransmitSet.field.BW		= BW_20;
			TransmitSet.field.MODE	= MODE_OFDM;
			TransmitSet.field.MCS	= MCS_RATE_6;
		}

		write_tmac_info_offload_pkt(pAd, wdev,
									type, sub_type,
									dsc_iob->pkt_buf,
									&TransmitSet,
									frame_len);
	}
	dsc_iob->pkt_len = frame_len;

	RTMP_SEM_UNLOCK(&dsc_iob->pkt_lock);

	MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Type(%d), Len(%ld), mode:%d, mcs:%d\n",
			 iob_type, frame_len, TransmitSet.field.MODE, TransmitSet.field.MCS);

	return NDIS_STATUS_SUCCESS;
}

struct wifi_dev *get_5G_wdev_by_bss_mgmt(void)
{
	PDL_LIST entry_list = NULL;
	PBMG_ENTRY bmg_entry = NULL;

	entry_list = &bss_mgmt.entry_list;

	if (DlListLen(entry_list) == 0)
		return NULL;

	DlListForEach(bmg_entry, entry_list, BMG_ENTRY, List) {
		if (!WMODE_CAP_6G(bmg_entry->entry_info.phymode) &&
			(bmg_entry->entry_info.channel > 14)) {
			return RtmpOsGetNetDevWdev(bmg_entry->pNetDev);
		}
	}
	return NULL;
}

struct wifi_dev *get_2G_wdev_by_bss_mgmt(void)
{
	PDL_LIST entry_list = NULL;
	PBMG_ENTRY bmg_entry = NULL;

	entry_list = &bss_mgmt.entry_list;

	if (DlListLen(entry_list) == 0)
		return NULL;

	DlListForEach(bmg_entry, entry_list, BMG_ENTRY, List) {
		if (!WMODE_CAP_6G(bmg_entry->entry_info.phymode) &&
			(bmg_entry->entry_info.channel <= 14)) {
			return RtmpOsGetNetDevWdev(bmg_entry->pNetDev);
		}
	}

	return NULL;
}

struct wifi_dev *get_6G_wdev_by_bss_mgmt(void)
{
	PDL_LIST entry_list = NULL;
	PBMG_ENTRY bmg_entry = NULL;

	entry_list = &bss_mgmt.entry_list;

	if (DlListLen(entry_list) == 0)
		return NULL;

	DlListForEach(bmg_entry, entry_list, BMG_ENTRY, List) {
		if (WMODE_CAP_6G(bmg_entry->entry_info.phymode))
			return RtmpOsGetNetDevWdev(bmg_entry->pNetDev);
	}

	return NULL;
}

NDIS_STATUS bss_mgmt_init(VOID)
{
	INT ret = NDIS_STATUS_SUCCESS;
	BMG_SET_PARAM bmg_info;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "\n");

	bmg_info.cmd_id		= BMG_SET_INIT;
	bmg_info.len		= 0;

	/* pass info to bss manager */
	bss_mgmt_con_set_info(NULL, &bmg_info);

	return ret;
}

NDIS_STATUS bss_mgmt_deinit(VOID)
{
	INT ret = NDIS_STATUS_SUCCESS;
	BMG_SET_PARAM bmg_info;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "\n");

	bmg_info.cmd_id		= BMG_SET_DEINIT;
	bmg_info.len		= 0;

	/* pass info to bss manager */
	bss_mgmt_con_set_info(NULL, &bmg_info);

	return ret;
}

NDIS_STATUS bss_mgmt_fill_bss_info(
	IN struct wifi_dev		*wdev,
	OUT PBMG_DEV_REG_INFO	reg_info
)
{
	struct _RTMP_ADAPTER *ad = NULL;
	BSS_STRUCT *pMbss = NULL;
	SECURITY_CONFIG *pSecConfig = NULL;
	UCHAR band_idx = 0;

	if (wdev) {
		ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
		pSecConfig = &wdev->SecConfig;
		if (wdev->wdev_type == WDEV_TYPE_AP)
			pMbss = wdev->func_dev;
		else {
			MTWF_DBG(ad, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"pMbss is NULL pointer\n");
			return NDIS_STATUS_FAILURE;
		}
	} else {
		return NDIS_STATUS_FAILURE;
	}
	band_idx = HcGetBandByWdev(wdev);

	reg_info->phymode			= wdev->PhyMode;
	reg_info->mbss_grp_idx		= pMbss->mbss_grp_idx;

	reg_info->channel			= wdev->channel;

	reg_info->is_trans_bss		= TRUE;
	reg_info->is_multi_bss		= FALSE;
#ifdef DOT11V_MBSSID_SUPPORT
	if (reg_info->mbss_grp_idx != MAIN_MBSSID) {
		if (IS_BSSID_11V_NON_TRANS(ad, pMbss, band_idx))
			reg_info->is_trans_bss	= FALSE;
	}

	if (IS_BSSID_11V_ENABLED(ad, band_idx) && (ad->ApCfg.BssidNumPerBand[band_idx] > 1))
		reg_info->is_multi_bss	= TRUE;
#endif

	reg_info->op_class = get_regulatory_class(ad, wdev->channel, wdev->PhyMode, wdev);

	/* ssid bssid */
	reg_info->ssid_len			= pMbss->SsidLen;
	reg_info->is_hide_ssid		= pMbss->bHideSsid;
	NdisMoveMemory(reg_info->ssid, pMbss->Ssid, (MAX_LEN_OF_SSID + 1));
	NdisMoveMemory(reg_info->bssid, wdev->bssid, MAC_ADDR_LEN);

	/* security */
	reg_info->auth_mode			= GET_SEC_AKM(pSecConfig);
	reg_info->PairwiseCipher	= GET_PAIRWISE_CIPHER(pSecConfig);
	reg_info->GroupCipher		= GET_GROUP_CIPHER(pSecConfig);

	/* response handler */
	BSS_MGMT_SET_RSP_HANDLER(reg_info, bss_mgmt_rsp_handler);

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS bss_mgmt_dev_reg(
	IN struct wifi_dev	*wdev
)
{
	INT ret = NDIS_STATUS_SUCCESS;
	PNET_DEV pNetDev = NULL;
	BMG_SET_PARAM bmg_info;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP)
			pNetDev = wdev->if_dev;
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev(%d)\n",
					 wdev->wdev_idx);
	}

	bmg_info.cmd_id		= BMG_SET_DEV_REG;
	bmg_info.len		= sizeof(BMG_DEV_REG_INFO);

	bss_mgmt_fill_bss_info(wdev, &bmg_info.data.dev_reg_info);

	/* register to bss manager */
	bss_mgmt_con_set_info(pNetDev, &bmg_info);

	/* sync iob/oob discovery rules */
	bss_mgmt_set_discovery_rules(wdev);

	return ret;
}

NDIS_STATUS bss_mgmt_dev_dereg(
	IN struct wifi_dev	*wdev
)
{
	INT ret = NDIS_STATUS_SUCCESS;
	PNET_DEV pNetDev = NULL;
	BMG_SET_PARAM bmg_info;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP)
			pNetDev = wdev->if_dev;
		else {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"pNetDev is NULL pointer\n");
			return NDIS_STATUS_FAILURE;
		}
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev(%d)\n",
					 wdev->wdev_idx);
	}

	bmg_info.cmd_id		= BMG_SET_DEV_DEREG;
	bmg_info.len		= 0;

	/* de-register to bss manager */
	bss_mgmt_con_set_info(pNetDev, &bmg_info);

	return ret;
}

NDIS_STATUS bss_mgmt_show_info(VOID)
{
	INT ret = NDIS_STATUS_SUCCESS;
	BMG_SET_PARAM bmg_info;

	bmg_info.cmd_id		= BMG_SET_SHOW_INFO;
	bmg_info.len		= 0;

	/* pass info to bss manager */
	bss_mgmt_con_set_info(NULL, &bmg_info);

	return ret;
}

bool bss_mgmt_is_2g_5g_exist(VOID)
{
	BMG_SET_PARAM bmg_info;

	bmg_info.cmd_id		= BMG_GET_2G_5G_EXIST;
	bmg_info.len		= sizeof(UINT32);

	/* pass info to bss manager */
	bss_mgmt_con_set_info(NULL, &bmg_info);

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"%s: 2g_5g_exist(%d)\n", __func__, bmg_info.data.data);

	return bmg_info.data.data;
}

NDIS_STATUS bss_mgmt_set_discovery_rules(
	IN struct wifi_dev	*wdev
)
{
	INT ret = NDIS_STATUS_SUCCESS;
	PNET_DEV pNetDev = NULL;
	BMG_SET_PARAM bmg_info;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP)
			pNetDev = wdev->if_dev;
		else {
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"pNetDev is NULL pointer\n");
			return NDIS_STATUS_FAILURE;
		}
			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev(%d)\n",
					 wdev->wdev_idx);
	} else {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev is NULL pointer\n");
		return NDIS_STATUS_FAILURE;
	}

	bmg_info.cmd_id		= BMG_SET_DISCOV_RULE;
	bmg_info.len		= sizeof(bmg_discov_rule);

	/* IoB Config */
	bmg_info.data.discov_rule.iob_type		= wlan_config_get_unsolicit_tx_type(wdev);
	bmg_info.data.discov_rule.iob_interval	= wlan_config_get_unsolicit_tx_tu(wdev);
	bmg_info.data.discov_rule.iob_txmode	= wlan_config_get_unsolicit_tx_mode(wdev);
	bmg_info.data.discov_rule.iob_by_cfg	= wlan_config_get_unsolicit_tx_by_cfg(wdev);

	/* OoB Config */
	bmg_info.data.discov_rule.oob_repting_2g = wlan_config_get_rnr_in_probe_rsp(wdev, RFIC_24GHZ);
	bmg_info.data.discov_rule.oob_repting_5g = wlan_config_get_rnr_in_probe_rsp(wdev, RFIC_5GHZ);
	bmg_info.data.discov_rule.oob_repting_6g = wlan_config_get_rnr_in_probe_rsp(wdev, RFIC_6GHZ);


	MTWF_PRINT("%s: IoB (%d/%d/%d%s), OoB Reported rules (%d/%d/%d)\n", __func__,
			 bmg_info.data.discov_rule.iob_type,
			 bmg_info.data.discov_rule.iob_interval,
			 bmg_info.data.discov_rule.iob_txmode,
			 bmg_info.data.discov_rule.iob_by_cfg ? " ByCfg" : "",
			 bmg_info.data.discov_rule.oob_repting_2g,
			 bmg_info.data.discov_rule.oob_repting_5g,
			 bmg_info.data.discov_rule.oob_repting_6g);

	/* pass info to bss manager */
	bss_mgmt_con_set_info(pNetDev, &bmg_info);

	return ret;
}

#endif /* CONFIG_6G_SUPPORT */

