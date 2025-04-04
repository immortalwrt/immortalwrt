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
    twt_ctrl.c

    Abstract:
    Support twt hardware control

    Who             When            What
    --------------  ----------      --------------------------------------------

*/

#include "rt_config.h"
#include "hdev/hdev.h"

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT


/* TWT control*/

/*
*
*/
struct twt_link_node *twt_ctrl_get_twt_node_by_index(
	struct hdev_ctrl *ctrl,
	UINT8 agrt_tbl_idx)
{
	struct _HD_RESOURCE_CFG *resource = &ctrl->HwResourceCfg;
	struct twt_ctrl *twt_ctl = &resource->twt_ctl;

	if ((twt_ctl == NULL) ||
		(twt_ctl->initd == FALSE) ||
		(agrt_tbl_idx >= twt_ctl->max_twt_node_num))
		return NULL;

#ifdef DYNAMIC_TWT_NODE_SUPPORT
	return twt_ctl->twt_node[agrt_tbl_idx];
#else
	return &twt_ctl->twt_node[agrt_tbl_idx];
#endif /* DYNAMIC_TWT_NODE_SUPPORT */
}

/*
*
*/
VOID twt_ctrl_init(struct hdev_ctrl *ctrl)
{
	struct _HD_RESOURCE_CFG *resource = &ctrl->HwResourceCfg;
	struct twt_ctrl *twt_ctl = &resource->twt_ctl;
	UINT8 i = 0;
	UINT8 sch_link_idx = 0;
#ifdef DYNAMIC_TWT_NODE_SUPPORT
	struct _RTMP_ADAPTER *pAd = ctrl->priv;
	struct _RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#else
	UINT8 idx = 0;
	struct twt_link_node *twt_node = NULL;
#endif /* DYNAMIC_TWT_NODE_SUPPORT */

	os_zero_mem(twt_ctl, sizeof(struct twt_ctrl));
	NdisAllocateSpinLock(NULL, &twt_ctl->twt_rec_lock);

#ifdef DYNAMIC_TWT_NODE_SUPPORT
	os_alloc_mem(pAd, (UCHAR **)&twt_ctl->twt_node,
				(sizeof(struct twt_link_node *)*pChipCap->twt_hw_num));
	if (twt_ctl->twt_node != NULL) {
		twt_ctl->max_twt_node_num = pChipCap->twt_hw_num;
		twt_ctl->free_twt_node_num_individual = pChipCap->twt_individual_max_num;
		twt_ctl->free_twt_node_num_btwt = pChipCap->twt_btwt_max_num;
		twt_ctl->free_twt_node_num_group = pChipCap->twt_group_max_num;

		/* twt node init. */
		for (i = 0; i < twt_ctl->max_twt_node_num; i++)
			twt_ctl->twt_node[i] = (struct twt_link_node *)NULL;
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"unexpected NULL, please check\n");
		return;
	}
#else
	/* twt node init. */
	for (i = 0; i < (TWT_HW_AGRT_MAX_NUM - TWT_HW_BTWT_MAX_NUM); i++) {
		idx = i;
		twt_node = &twt_ctl->twt_node[idx];
		twt_node->agrt_tbl_idx = idx;
		twt_node->state = TWT_STATE_SW_NONE_OCCUPIED;
		twt_node->type = TWT_TYPE_INDIVIDUAL;
		twt_ctl->max_twt_node_num++;
		twt_ctl->free_twt_node_num_individual++;
	}

#if (TWT_HW_BTWT_MAX_NUM > 0)/*FixCoverity,TWT_HW_BTWT_MAX_NUM=0 now*/
	for (i = 0; i < TWT_HW_BTWT_MAX_NUM; i++) {
		idx = i + TWT_HW_AGRT_MAX_NUM - TWT_HW_BTWT_MAX_NUM;
		twt_node = &twt_ctl->twt_node[idx];
		twt_node->agrt_tbl_idx = idx;
		twt_node->state = TWT_STATE_SW_NONE_OCCUPIED;
		twt_node->type = TWT_TYPE_BTWT;
		twt_ctl->max_twt_node_num++;
		twt_ctl->free_twt_node_num_btwt++;
	}
#endif
#endif /* DYNAMIC_TWT_NODE_SUPPORT */

	/* twt sch/usch link init. */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++)
		DlListInit(&twt_ctl->twt_link[sch_link_idx]);

	twt_ctl->initd = TRUE;
}

/*
*
*/
VOID twt_ctrl_exit(struct hdev_ctrl *ctrl)
{
	struct _HD_RESOURCE_CFG *resource = &ctrl->HwResourceCfg;
	struct twt_ctrl *twt_ctl = &resource->twt_ctl;
	UINT8 sch_link_idx = 0;
#ifdef DYNAMIC_TWT_NODE_SUPPORT
	UINT8 i = 0;
	struct twt_link_node *twt_node = NULL;
#endif /* DYNAMIC_TWT_NODE_SUPPORT */

	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++)
		DlListInit(&twt_ctl->twt_link[sch_link_idx]);

	NdisFreeSpinLock(&twt_ctl->twt_rec_lock);
#ifdef DYNAMIC_TWT_NODE_SUPPORT
	if (twt_ctl->twt_node != NULL) {
		for (i = 0; i < twt_ctl->max_twt_node_num; i++) {
			twt_node = twt_ctl->twt_node[i];
			if (twt_node != NULL)
				os_free_mem(twt_node);
		}
		os_free_mem(twt_ctl->twt_node);
	}
#endif /* DYNAMIC_TWT_NODE_SUPPORT */
	os_zero_mem(twt_ctl, sizeof(struct twt_ctrl));
}

/*
*
*/
struct twt_link_node *twt_ctrl_acquire_twt_node(struct hdev_ctrl *ctrl, BOOLEAN type)
{
	struct _RTMP_ADAPTER *pAd = ctrl->priv;
	struct _HD_RESOURCE_CFG *resource = &ctrl->HwResourceCfg;
	struct twt_ctrl *twt_ctl = &resource->twt_ctl;
	struct twt_link_node *twt_node = NULL;
	UINT8 i;

	if (resource == NULL || twt_ctl == NULL || !twt_ctl->initd) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"unexpected NULL, please check\n");
		return NULL;
	}

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
#ifdef DYNAMIC_TWT_NODE_SUPPORT
	if (((type == TWT_TYPE_INDIVIDUAL) && (twt_ctl->free_twt_node_num_individual == 0)) ||
		((type == TWT_TYPE_BTWT) && (twt_ctl->free_twt_node_num_btwt == 0)) ||
		((type == TWT_TYPE_GROUP) && (twt_ctl->free_twt_node_num_group == 0))) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"unexpected NULL, due to free poll is out of resource! (type:%d)\n", type);
		NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
		return NULL;
	}

	for (i = 0; i < twt_ctl->max_twt_node_num; i++) {
		if (twt_ctl->twt_node[i] != NULL)
			continue;

		os_alloc_mem(pAd, (UCHAR **)&twt_ctl->twt_node[i], sizeof(struct twt_link_node));
		if (twt_ctl->twt_node[i] == NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"unexpected NULL, due to alloc memory fail!\n");
			NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
			return NULL;
		}
		twt_node = twt_ctl->twt_node[i];
		twt_node->state = TWT_STATE_SW_OCCUPIED;
		twt_node->type = type;
		twt_node->agrt_tbl_idx = i;
		if (type == TWT_TYPE_INDIVIDUAL)
			twt_ctl->free_twt_node_num_individual--;
		else if (type == TWT_TYPE_GROUP)
			twt_ctl->free_twt_node_num_group--;
		else if (type == TWT_TYPE_BTWT)
			twt_ctl->free_twt_node_num_btwt--;
		NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
		return twt_node;
	}
#else
	for (i = 0; i < twt_ctl->max_twt_node_num; i++) {
		twt_node = &twt_ctl->twt_node[i];

		if (twt_node->type != type)
			continue;

		if (twt_node->state == TWT_STATE_SW_OCCUPIED)
			continue;

		twt_node->state = TWT_STATE_SW_OCCUPIED;

		if (type == TWT_TYPE_INDIVIDUAL)
			twt_ctl->free_twt_node_num_individual--;
		else if (type == TWT_TYPE_GROUP)
			twt_ctl->free_twt_node_num_group--;
		else if (type == TWT_TYPE_BTWT)
			twt_ctl->free_twt_node_num_btwt--;

		NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
		return twt_node;
	}
#endif /* DYNAMIC_TWT_NODE_SUPPORT */

	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

	return NULL;
}

/*
*
*/
BOOLEAN twt_ctrl_release_twt_node(struct hdev_ctrl *ctrl, struct twt_link_node *twt_node)
{
	struct _RTMP_ADAPTER *pAd = ctrl->priv;
	struct _HD_RESOURCE_CFG *resource = &ctrl->HwResourceCfg;
	struct twt_ctrl *twt_ctl = &resource->twt_ctl;
	UINT8 agrt_tbl_idx = 0;
	BOOLEAN status = FALSE;
#ifndef DYNAMIC_TWT_NODE_SUPPORT
	BOOLEAN type = TWT_TYPE_INDIVIDUAL;
#endif /* DYNAMIC_TWT_NODE_SUPPORT */

	if (resource == NULL || twt_ctl == NULL || !twt_ctl->initd) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"unexpected NULL, please check\n");
		return FALSE;
	}

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);

#ifdef DYNAMIC_TWT_NODE_SUPPORT
	if (twt_node &&
		(twt_node->state == TWT_STATE_SW_OCCUPIED)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"bl_i=%d, type=%d\n", twt_node->agrt_tbl_idx, twt_node->type);

		agrt_tbl_idx = twt_node->agrt_tbl_idx;
		if (agrt_tbl_idx < twt_ctl->max_twt_node_num) {
			if (twt_node->type == TWT_TYPE_INDIVIDUAL)
				twt_ctl->free_twt_node_num_individual++;
			else if (twt_node->type == TWT_TYPE_GROUP)
				twt_ctl->free_twt_node_num_group++;
			else if (twt_node->type == TWT_TYPE_BTWT)
				twt_ctl->free_twt_node_num_btwt++;
			twt_ctl->twt_node[agrt_tbl_idx] = (struct twt_link_node *)NULL;
			os_free_mem(twt_node);
			status = TRUE;
		}
	}
#else
	if (twt_node->state == TWT_STATE_SW_NONE_OCCUPIED) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"try to release non-occupied tbl_i=%d, please check\n", twt_node->agrt_tbl_idx);
		status = FALSE;
	} else {
		type = twt_node->type;
		agrt_tbl_idx = twt_node->agrt_tbl_idx;
		os_zero_mem(twt_node, sizeof(struct twt_link_node));

		if (type == TWT_TYPE_INDIVIDUAL)
			twt_ctl->free_twt_node_num_individual++;
		else if (type == TWT_TYPE_GROUP)
			twt_ctl->free_twt_node_num_group++;
		else if (type == TWT_TYPE_BTWT)
			twt_ctl->free_twt_node_num_btwt++;
		twt_node->state = TWT_STATE_SW_NONE_OCCUPIED;
		twt_node->agrt_tbl_idx = agrt_tbl_idx;
		twt_node->type = type;
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"bl_i=%d\n", twt_node->agrt_tbl_idx);
		status = TRUE;
	}
#endif /* DYNAMIC_TWT_NODE_SUPPORT */

	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

	return status;
}

/*
* bTWT
*/
VOID twt_ctrl_btwt_dump(struct hdev_ctrl *ctrl, UINT32 *btwt_id_bitmap)
{
	struct _HD_RESOURCE_CFG *resource = &ctrl->HwResourceCfg;
	struct twt_ctrl *twt_ctl = &resource->twt_ctl;
	struct twt_link_node *twt_node = NULL;
	UINT8 i = 0;

	MTWF_PRINT("**btwt_element=%d,free btwt resource(%d)\n",
				(TWT_HW_BTWT_MAX_NUM - twt_ctl->free_twt_node_num_btwt),
				twt_ctl->free_twt_node_num_btwt);

	MTWF_PRINT("\n%-5s%-8s%-4s%-4s%-4s%-5s%-7s%-8s%-10s%-11s%-13s%-14s%-4s\n",
			 "band", "btwt_id", "sp", "man", "exp", "trig", "f_type", "protect", "setup_cmd", "peristence", "twt_info_dis", "wake_dur_unit", "rec");

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	for (i = 0; i < twt_ctl->max_twt_node_num; i++) {
		twt_node = twt_ctrl_get_twt_node_by_index(ctrl, i);
		if (twt_node == NULL)
			continue;
		if (twt_node->type != TWT_TYPE_BTWT)
			continue;
		if (twt_node->state != TWT_STATE_SW_OCCUPIED)
			continue;

		(*btwt_id_bitmap) |= (1 << GET_BTWT_ID(twt_node));

		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"%-5d", twt_node->band);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"%-8d", (UINT16)GET_BTWT_ID(twt_node));
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"%-4d", twt_node->agrt_sp_duration);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"%-4d", twt_node->agrt_sp_wake_intvl_mantissa);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"%-4d", twt_node->agrt_sp_wake_intvl_exponent);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"%-5d", GET_AGRT_PARA_BITMAP(twt_node, TWT_AGRT_PARA_BITMAP_IS_TRIGGER));
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"%-7d", GET_AGRT_PARA_BITMAP(twt_node, TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE));
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"%-8d", GET_AGRT_PARA_BITMAP(twt_node, TWT_AGRT_PARA_BITMAP_IS_PROTECT));
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"%-10d", twt_node->twt_setup_cmd);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"%-11d", twt_node->persistence);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"%-13d", twt_node->twt_info_frame_dis);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"%-14d", twt_node->wake_dur_unit);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"%-4d", twt_node->btwt_recommendation);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"\n");
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
}

/*
*
*/
UINT8 twt_ctrl_get_max_twt_node_num(struct hdev_ctrl *ctrl)
{
	struct _HD_RESOURCE_CFG *resource = &ctrl->HwResourceCfg;
	struct twt_ctrl *twt_ctl = &resource->twt_ctl;

	return twt_ctl->max_twt_node_num;
}

/*
*
*/
VOID twt_ctrl_get_free_twt_node_num(struct hdev_ctrl *ctrl, UINT8 *individual_num, UINT8 *group_num)
{
	struct _HD_RESOURCE_CFG *resource = &ctrl->HwResourceCfg;
	struct twt_ctrl *twt_ctl = &resource->twt_ctl;

	*individual_num = twt_ctl->free_twt_node_num_individual;
	*group_num = twt_ctl->free_twt_node_num_group;
}

/*
*
*/
VOID twt_ctrl_resource_status_dump(struct hdev_ctrl *ctrl)
{
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *twt_node = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	UINT8 i = 0;
	UINT8 i_tbl_free = 0;
	UINT8 g_tbl_free = 0;
	UINT8 sch_link_idx = 0;

	if (ctrl == NULL) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "ctrl=NULL, please check\n");
		return;
	}

	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	/* twt agrt status */
	MTWF_PRINT("\n*** twt agrt status ***\n");

	for (i = 0; i < twt_ctl->max_twt_node_num; i++) {
		twt_node = twt_ctrl_get_twt_node_by_index(ctrl, i);
		if (twt_node == NULL)
			continue;

		if (twt_node->type == TWT_TYPE_INDIVIDUAL) {
			MTWF_PRINT("\t**i_idx=%d,p=%p,tbl_i=%d,ste=%d,o_i=%d,b_i=%d,wcid=%d,spd=%d,f_i=%d,sp=%d,m=%d,e=%d,para=0x%x,tsf_sch=(%x,%x),\n",
				i,
				twt_node,
				twt_node->agrt_tbl_idx,
				twt_node->state,
				twt_node->own_mac_idx,
				twt_node->bss_idx,
				twt_node->peer_id_grp_id,
				twt_node->suspend,
				twt_node->flow_id,
				twt_node->agrt_sp_duration,
				twt_node->agrt_sp_wake_intvl_mantissa,
				twt_node->agrt_sp_wake_intvl_exponent,
				twt_node->agrt_para_bitmap,
				(UINT32)(twt_node->schedule_sp_start_tsf & 0xffffffff),
				(UINT32)(twt_node->schedule_sp_start_tsf >> 32));
		}
	}

	for (i = 0; i < twt_ctl->max_twt_node_num; i++) {
		twt_node = twt_ctrl_get_twt_node_by_index(ctrl, i);
		if (twt_node == NULL)
			continue;

		if (twt_node->type == TWT_TYPE_BTWT) {
			MTWF_PRINT("\t**g_idx=%d,p=%p,tbl_i=%d,ste=%d,o_i=%d,b_i=%d,wcid=%d,spd=%d,f_i=%d,sp=%d,m=%d,e=%d,para=0x%x,tsf_sch=(%x,%x),\n"
				"\t  grp_grade=%d,grp_cnt=%d,[%d,%d,%d,%d,%d,%d,%d,%d]\n",
				i,
				twt_node,
				twt_node->agrt_tbl_idx,
				twt_node->state,
				twt_node->own_mac_idx,
				twt_node->bss_idx,
				twt_node->peer_id_grp_id,
				twt_node->suspend,
				twt_node->flow_id,
				twt_node->agrt_sp_duration,
				twt_node->agrt_sp_wake_intvl_mantissa,
				twt_node->agrt_sp_wake_intvl_exponent,
				twt_node->agrt_para_bitmap,
				(UINT32)(twt_node->schedule_sp_start_tsf & 0xffffffff),
				(UINT32)(twt_node->schedule_sp_start_tsf >> 32),
				twt_node->grp_grade,
				twt_node->grp_member_cnt,
				twt_node->sta_list[0],
				twt_node->sta_list[1],
				twt_node->sta_list[2],
				twt_node->sta_list[3],
				twt_node->sta_list[4],
				twt_node->sta_list[5],
				twt_node->sta_list[6],
				twt_node->sta_list[7]);
		}
	}

	twt_ctrl_get_free_twt_node_num(ctrl, &i_tbl_free, &g_tbl_free);
	MTWF_PRINT("\t*total:%d,i_free:%d,g_free:%d,sch_link=%p,usch_link=%p\n",
		twt_ctrl_get_max_twt_node_num(ctrl),
		i_tbl_free,
		g_tbl_free,
		&twt_ctl->twt_link[0],
		&twt_ctl->twt_link[1]);

	/* twt link list status */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		MTWF_PRINT("*** twt_link[%d],len=%d ***\n", sch_link_idx, DlListLen(&twt_ctl->twt_link[sch_link_idx]));
		DlListForEach(curr_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			MTWF_PRINT("\t**twt_node:tbl_i=%d,s=%d,wcid=%d,spd=%d,f_i=%d,tsf_t=%d,tsf_sch=(%x,%x),sp=%d,align=%d,tsf_info=(%x,%x),tsf_wish(%x,%x)\n",
				curr_twt_node->agrt_tbl_idx,
				curr_twt_node->state,
				curr_twt_node->peer_id_grp_id,
				curr_twt_node->suspend,
				curr_twt_node->flow_id,
				curr_twt_node->tsf_type,
				(UINT32)(curr_twt_node->schedule_sp_start_tsf & 0xffffffff),
				(UINT32)(curr_twt_node->schedule_sp_start_tsf >> 32),
				curr_twt_node->agrt_sp_duration,
				(UINT32)(curr_twt_node->schedule_sp_start_tsf & 0xffffffff) % TWT_TSF_ALIGNMNET_UINT,
				(UINT32)(curr_twt_node->agrt_sp_info_tsf & 0xffffffff),
				(UINT32)(curr_twt_node->agrt_sp_info_tsf >> 32),
				(UINT32)(curr_twt_node->agrt_sp_start_tsf & 0xffffffff),
				(UINT32)(curr_twt_node->agrt_sp_start_tsf >> 32));
		}
	}
}

#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

