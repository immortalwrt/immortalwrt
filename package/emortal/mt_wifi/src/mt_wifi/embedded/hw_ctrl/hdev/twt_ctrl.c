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

#define TWT_STATE_SW_NONE_OCCUPIED	0
#define TWT_STATE_SW_OCCUPIED		1


/* TWT control*/

/*
*
*/
VOID twt_ctrl_init(struct hdev_ctrl *ctrl)
{
	struct _HD_RESOURCE_CFG *resource = &ctrl->HwResourceCfg;
	struct twt_ctrl *twt_ctl = &resource->twt_ctl;
	struct twt_link_node *twt_node = NULL;
	UINT8 i = 0;
	UINT8 idx = 0;
	UINT8 sch_link_idx = 0;

	os_zero_mem(twt_ctl, sizeof(struct twt_ctrl));
	NdisAllocateSpinLock(NULL, &twt_ctl->twt_rec_lock);

	/* twt node init. */
	for (i = 0; i < (TWT_HW_AGRT_MAX_NUM - TWT_HW_GRP_MAX_NUM); i++) {
		idx = i;
		twt_node = &twt_ctl->twt_node[idx];
		twt_node->agrt_tbl_idx = idx;
		twt_node->state = TWT_STATE_SW_NONE_OCCUPIED;
		twt_node->type = TWT_TYPE_INDIVIDUAL;
		twt_ctl->max_twt_node_num++;
		twt_ctl->free_twt_node_num_individual++;
	}

#if (TWT_HW_GRP_MAX_NUM > 0)
	for (i = 0; i < TWT_HW_GRP_MAX_NUM; i++) {
		idx = i + TWT_HW_AGRT_MAX_NUM - TWT_HW_GRP_MAX_NUM;
		twt_node = &twt_ctl->twt_node[idx];
		twt_node->agrt_tbl_idx = idx;
		twt_node->state = TWT_STATE_SW_NONE_OCCUPIED;
		twt_node->type = TWT_TYPE_GROUP;
		twt_ctl->max_twt_node_num++;
		twt_ctl->free_twt_node_num_group++;
	}
#endif
	/* twt sch/usch link init. */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++)
		DlListInit(&twt_ctl->twt_link[sch_link_idx]);
}

/*
*
*/
VOID twt_ctrl_exit(struct hdev_ctrl *ctrl)
{
	struct _HD_RESOURCE_CFG *resource = &ctrl->HwResourceCfg;
	struct twt_ctrl *twt_ctl = &resource->twt_ctl;
	UINT8 sch_link_idx = 0;

	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++)
		DlListInit(&twt_ctl->twt_link[sch_link_idx]);

	NdisFreeSpinLock(&twt_ctl->twt_rec_lock);
	os_zero_mem(twt_ctl, sizeof(struct twt_ctrl));
}

/*
*
*/
struct twt_link_node *twt_ctrl_acquire_twt_node(struct hdev_ctrl *ctrl, struct hdev_obj *obj, BOOLEAN type)
{
	struct _HD_RESOURCE_CFG *resource = &ctrl->HwResourceCfg;
	struct twt_ctrl *twt_ctl = &resource->twt_ctl;
	struct twt_link_node *twt_node = NULL;
	UINT8 i;

	if (obj == NULL || resource == NULL || twt_ctl == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: unexpected NULL, please check\n", __func__));
		return NULL;
	}

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);

	for (i = 0; i < twt_ctl->max_twt_node_num; i++) {
		twt_node = &twt_ctl->twt_node[i];

		if (twt_node == NULL) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				("%s: unexpected NULL, please check\n", __func__));
			NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
			return NULL;
		}

		if (twt_node->type != type)
			continue;

		if (twt_node->state == TWT_STATE_SW_OCCUPIED)
			continue;

		twt_node->state = TWT_STATE_SW_OCCUPIED;

		if (type == TWT_TYPE_INDIVIDUAL)
			twt_ctl->free_twt_node_num_individual--;
		else
			twt_ctl->free_twt_node_num_group--;

		NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

		return twt_node;
	}

	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

	return NULL;
}

/*
*
*/
BOOLEAN twt_ctrl_release_twt_node(struct hdev_ctrl *ctrl, struct hdev_obj *obj, struct twt_link_node *twt_node)
{
	struct _HD_RESOURCE_CFG *resource = &ctrl->HwResourceCfg;
	struct twt_ctrl *twt_ctl = &resource->twt_ctl;
	UINT8 agrt_tbl_idx = 0;
	BOOLEAN type = TWT_TYPE_INDIVIDUAL;
	BOOLEAN status = FALSE;

	if (obj == NULL || resource == NULL || twt_ctl == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: unexpected NULL, please check\n", __func__));
		return FALSE;
	}

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);

	if (twt_node->state == TWT_STATE_SW_NONE_OCCUPIED) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: try to release non-occupied tbl_i=%d, please check\n", __func__, twt_node->agrt_tbl_idx));
		status = FALSE;
	} else {
		type = twt_node->type;
		agrt_tbl_idx = twt_node->agrt_tbl_idx;
		os_zero_mem(twt_node, sizeof(struct twt_link_node));

		if (type == TWT_TYPE_INDIVIDUAL)
			twt_ctl->free_twt_node_num_individual++;
		else
			twt_ctl->free_twt_node_num_group++;

		twt_node->state = TWT_STATE_SW_NONE_OCCUPIED;
		twt_node->agrt_tbl_idx = agrt_tbl_idx;
		twt_node->type = type;
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
			("%s: tbl_i=%d\n", __func__, twt_node->agrt_tbl_idx));
		status = TRUE;
	}

	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

	return status;
}

struct twt_link_node *twt_ctrl_get_twt_node_by_index(struct hdev_ctrl *ctrl, UINT8 agrt_tbl_idx)
{
	struct _HD_RESOURCE_CFG *resource = &ctrl->HwResourceCfg;
	struct twt_ctrl *twt_ctl = &resource->twt_ctl;

	return &twt_ctl->twt_node[agrt_tbl_idx];
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
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: ctrl=NULL, please check\n", __func__));
		return;
	}

	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	/* twt agrt status */
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_OFF,
		("\n*** twt agrt status ***\n"));

	for (i = 0; i < twt_ctl->max_twt_node_num; i++) {
		twt_node = &twt_ctl->twt_node[i];
		if (twt_node->type == TWT_TYPE_INDIVIDUAL) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_OFF,
				("\t**i_idx=%d,p=%p,tbl_i=%d,ste=%d,o_i=%d,b_i=%d,wcid=%d,spd=%d,f_i=%d,sp=%d,m=%d,e=%d,para=0x%x,tsf_sch=(%x,%x),\n",
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
				(UINT32)(twt_node->schedule_sp_start_tsf >> 32)));
		}
	}

	for (i = 0; i < twt_ctl->max_twt_node_num; i++) {
		twt_node = &twt_ctl->twt_node[i];
		if (twt_node->type == TWT_TYPE_GROUP) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_OFF,
				("\t**g_idx=%d,p=%p,tbl_i=%d,ste=%d,o_i=%d,b_i=%d,wcid=%d,spd=%d,f_i=%d,sp=%d,m=%d,e=%d,para=0x%x,tsf_sch=(%x,%x),\n"
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
				twt_node->sta_list[7]));
		}
	}

	twt_ctrl_get_free_twt_node_num(ctrl, &i_tbl_free, &g_tbl_free);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_OFF,
		("\t*total:%d,i_free:%d,g_free:%d,sch_link=%p,usch_link=%p\n",
		twt_ctrl_get_max_twt_node_num(ctrl),
		i_tbl_free,
		g_tbl_free,
		&twt_ctl->twt_link[0],
		&twt_ctl->twt_link[1]));

	/* twt link list status */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_OFF,
			("*** twt_link[%d],len=%d ***\n", sch_link_idx, DlListLen(&twt_ctl->twt_link[sch_link_idx])));
		DlListForEach(curr_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_OFF,
			("\t**twt_node:tbl_i=%d,s=%d,wcid=%d,spd=%d,f_i=%d,tsf_t=%d,tsf_sch=(%x,%x),sp=%d,align=%d,tsf_info=(%x,%x),tsf_wish(%x,%x)\n",
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
				(UINT32)(curr_twt_node->agrt_sp_start_tsf >> 32)));
		}
	}
}

#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

