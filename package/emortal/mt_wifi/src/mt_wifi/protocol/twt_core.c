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
    twt_core.c

    Abstract:
    Support twt mlme

    Who             When            What
    --------------  ----------      --------------------------------------------

*/

#include "rt_config.h"
#include "hdev/hdev.h"

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT

static BOOLEAN twt_is_grpid(UINT16 peer_id_grp_id)
{
	if (peer_id_grp_id & TWT_PEER_GROUP_ID_BIT)
		return TRUE;
	else
		return FALSE;
}

#if (MTK_TWT_GROUP_EN == 1)
static UINT8 twt_grp_tbl_idx(UINT16 peer_id_grp_id)
{
	return ((UINT8)(peer_id_grp_id & (~TWT_PEER_GROUP_ID_BIT)));
}
#endif /* MTK_TWT_GROUP_EN */

static BOOLEAN twt_agrt_in_individual(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid)
{
	BOOLEAN wcid_in_individual = FALSE;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	UINT8 sch_link_idx = 0;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return FALSE;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(curr_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			if (!curr_twt_node) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					("%s: twt_entry=NULL, please check\n", __func__));
				return FALSE;
			}

			if (curr_twt_node->peer_id_grp_id == wcid) {
				wcid_in_individual = TRUE;
				break;
			}
		}
	}

	return wcid_in_individual;
}

static BOOLEAN twt_agrt_in_group(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid)
{
	BOOLEAN wcid_in_group = FALSE;
	UINT8 i = 0;
	UINT8 sch_link_idx = 0;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return FALSE;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(curr_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			if (!curr_twt_node) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					("%s: twt_entry=NULL, please check\n", __func__));
				return FALSE;
			}

			if (twt_is_grpid(curr_twt_node->peer_id_grp_id)) {
				for (i = 0; i < TWT_HW_GRP_MAX_MEMBER_CNT; i++) {
					if (curr_twt_node->sta_list[i] == wcid) {
						wcid_in_group = TRUE;
						break;
					}
				}
			}
			if (wcid_in_group)
				break;
		}
	}

	return wcid_in_group;
}

static BOOLEAN twt_agrt_exist(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN UINT8 flow_id)
{
	BOOLEAN twt_agrt_exist = FALSE;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	UINT8 sch_link_idx = 0;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return FALSE;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(curr_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			if (!curr_twt_node) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					("%s: twt_entry=NULL, please check\n", __func__));
				return FALSE;
			}

			if ((curr_twt_node->peer_id_grp_id == wcid) &&
			    (curr_twt_node->flow_id == flow_id)) {
				twt_agrt_exist = TRUE;
				break;
			}
		}
	}

	return twt_agrt_exist;
}


static UINT32 twt_align_duration(UINT32 sp_duration, UINT32 alignment)
{
	UINT32 sp_duration_alignment = 0;
	UINT32 m = sp_duration % alignment;

#if (TWT_TSF_ALIGNMENT_EN == 1)
	sp_duration_alignment = sp_duration + (alignment - m);
#else
	sp_duration_alignment = sp_duration;
#endif /* TWT_TSF_ALIGNMENT_EN */

	return sp_duration_alignment;
}

/*
    target wake time:
    1. Decide by AP when CMD=Request/Suggest and use force=FALSE
    2. Decide by STA when CMD=Demand and use force=TRUE
*/
static VOID twt_link_insert_node(
	IN struct wifi_dev *wdev,
	IN struct twt_link_node *twt_node)
{
	BOOLEAN found = FALSE;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *next_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct twt_link_node *head_twt_node = NULL;
	UINT16 sp_duration = 0;

	if (!wdev || !twt_node) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=%p, twt_node=%p, NULL, please check\n",
			__func__, wdev, twt_node));
		return;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);

	if (twt_node->tsf_type == TSF_FROM_SETUP_CMD_DEMAND) {
		/* insert to unschedule link list */
		DlListAddTail(&twt_ctl->twt_link[USCH_LINK], &twt_node->list);
	} else {
		/* insert to schedule link list */
		sp_duration = twt_node->agrt_sp_duration << 8;
		if (DlListLen(&twt_ctl->twt_link[SCH_LINK]) == 0) {
			/* insert as the 1st node */
			twt_node->schedule_sp_start_tsf = 0;
			DlListAddTail(&twt_ctl->twt_link[SCH_LINK], &twt_node->list);
		} else if (DlListLen(&twt_ctl->twt_link[SCH_LINK]) == 1) {
			curr_twt_node = DlListFirst(&twt_ctl->twt_link[SCH_LINK], struct twt_link_node, list);

			if (!curr_twt_node) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				("%s:NULL Pointer!\n", __func__));
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				return;
			}

			if (curr_twt_node->schedule_sp_start_tsf >= sp_duration) {
				/* insert before 1st node */
				twt_node->schedule_sp_start_tsf = 0;
				DlListAddTail(&curr_twt_node->list, &twt_node->list);
			} else {
				/* insert after 1st node */
				twt_node->schedule_sp_start_tsf = curr_twt_node->schedule_sp_start_tsf +
					twt_align_duration((curr_twt_node->agrt_sp_duration) << 8, TWT_TSF_ALIGNMNET_UINT);
				DlListAdd(&curr_twt_node->list, &twt_node->list);
			}
		} else {
			/* insert at proper place */
			head_twt_node = DlListFirst(&twt_ctl->twt_link[SCH_LINK], struct twt_link_node, list);
			DlListForEachSafe(temp_twt_node, next_twt_node, &twt_ctl->twt_link[SCH_LINK], struct twt_link_node, list) {
			    curr_twt_node = temp_twt_node;
				/* space check before 1st node */
				if (curr_twt_node == head_twt_node) {
					if (curr_twt_node->schedule_sp_start_tsf >= sp_duration) {
						/* insert before head */
						twt_node->schedule_sp_start_tsf = 0;
						DlListAddTail(&curr_twt_node->list, &twt_node->list);
						found = TRUE;
						break;
					}
				}
				/* space check after 1st node if current node is not the last node */
				if ((&curr_twt_node->list)->Next != (&twt_ctl->twt_link[SCH_LINK])) {
					if (next_twt_node->schedule_sp_start_tsf -
						(curr_twt_node->schedule_sp_start_tsf +
						twt_align_duration(curr_twt_node->agrt_sp_duration << 8, TWT_TSF_ALIGNMNET_UINT)) >= sp_duration) {
						twt_node->schedule_sp_start_tsf = curr_twt_node->schedule_sp_start_tsf +
							twt_align_duration(curr_twt_node->agrt_sp_duration << 8, TWT_TSF_ALIGNMNET_UINT);
						DlListAdd(&curr_twt_node->list, &twt_node->list);
						found = TRUE;
						break;
					}
				}
			}
			/* insert as the tail node */
			if (!found) {
				twt_node->schedule_sp_start_tsf = curr_twt_node->schedule_sp_start_tsf +
					twt_align_duration(curr_twt_node->agrt_sp_duration << 8, TWT_TSF_ALIGNMNET_UINT);
				DlListAddTail(&twt_ctl->twt_link[SCH_LINK], &twt_node->list);
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
}

static VOID twt_link_remove_node(
	IN struct wifi_dev *wdev,
	IN struct twt_link_node *twt_node)
{
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;

	if (!wdev || !twt_node) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL or ptwt_link_entry=NULL, please check\n", __func__));
		return;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	DlListDel(&twt_node->list);
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
}

/* normalize grp_grade to 0~100 */
static UINT8 twt_grp_grade(
	UINT16 mantissa,
	UINT8 exponent,
	UINT8 sp)
{
	UINT8 i = 0;
	ULONG result = 1;

	for (i = 0; i < exponent; i++)
		result *= 2;

	result = (sp * 256 * MAX_GRP_GRADE) / (mantissa * result);

	return (UINT8)result;
}

static VOID twt_para_optimize(
	INOUT PUINT16 mantissa,
	INOUT PUINT8 exponent,
	INOUT PUINT8 sp)
{
#if (TWT_PARA_OPTIMIZE_EN == 1)
	UINT8 i = 0;
	UINT16 result = 1;

	for (i = 0; i < 16; i++) {
		result *= 2;
		if (result >= *mantissa) {
			*mantissa = result;
			break;
		}
	}

	result = 1;
	for (i = 0; i < 8; i++) {
		result *= 2;
		if (result >= *sp) {
			if (i == 7)
				result -= 1;
			*sp = (UINT8)result;
			break;
		}
	}
#endif /* TWT_PARA_OPTIMIZE_EN */
}

static UINT8 twt_get_free_flow_id(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid)
{
	UINT8 flow_id = 0;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct _RTMP_ADAPTER *ad = NULL;
	UINT8 i = 0;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	entry = &ad->MacTab.Content[wcid];

	for (i = 0; i < TWT_FLOW_ID_MAX_NUM; i++) {
		if ((entry->twt_flow_id_bitmap & (1 << i)) == 0) {
			flow_id = i;
			break;
		}
	}

	return flow_id;
}

static VOID twt_agrt_cmd_set(
	OUT struct twt_agrt_para *twt_agrt,
	IN struct twt_link_node *curr_twt_node,
	IN UINT8 agrt_ctrl_flag,
	IN UINT8 cmd_tsf_type)
{
	UINT8 i = 0;

	twt_agrt->agrt_tbl_idx = curr_twt_node->agrt_tbl_idx;
	twt_agrt->agrt_ctrl_flag = agrt_ctrl_flag;
	twt_agrt->own_mac_idx = curr_twt_node->own_mac_idx;
	twt_agrt->flow_id = curr_twt_node->flow_id;
	twt_agrt->peer_id_grp_id = curr_twt_node->peer_id_grp_id;
	twt_agrt->agrt_sp_duration = curr_twt_node->agrt_sp_duration;
	twt_agrt->bss_idx = curr_twt_node->bss_idx;
	twt_agrt->agrt_sp_wake_intvl_mantissa = curr_twt_node->agrt_sp_wake_intvl_mantissa;
	twt_agrt->agrt_sp_wake_intvl_exponent = curr_twt_node->agrt_sp_wake_intvl_exponent;
	twt_agrt->agrt_para_bitmap = curr_twt_node->agrt_para_bitmap;

	if (cmd_tsf_type == CMD_TST_TYPE_SCHEDULE) {
		twt_agrt->agrt_sp_start_tsf_low = (UINT32)(curr_twt_node->schedule_sp_start_tsf_abs & 0xffffffff);
		twt_agrt->agrt_sp_start_tsf_high = (UINT32)(curr_twt_node->schedule_sp_start_tsf_abs >> 32);
	} else if (cmd_tsf_type == CMD_TSF_TYPE_REQUESTER) {
		twt_agrt->agrt_sp_start_tsf_low = (UINT32)(curr_twt_node->agrt_sp_start_tsf & 0xffffffff);
		twt_agrt->agrt_sp_start_tsf_high = (UINT32)(curr_twt_node->agrt_sp_start_tsf >> 32);
	} else if (cmd_tsf_type == CMD_TSF_TYPE_TWT_INFO) {
		twt_agrt->agrt_sp_start_tsf_low = (UINT32)(curr_twt_node->agrt_sp_info_tsf & 0xffffffff);
		twt_agrt->agrt_sp_start_tsf_high = (UINT32)(curr_twt_node->agrt_sp_info_tsf >> 32);
	} else if (cmd_tsf_type == CMD_TSF_TYPE_NA) {
		twt_agrt->agrt_sp_start_tsf_low = 0;
		twt_agrt->agrt_sp_start_tsf_high = 0;
		twt_agrt->agrt_sp_duration = 0;
		twt_agrt->agrt_sp_wake_intvl_mantissa = 0;
		twt_agrt->agrt_sp_wake_intvl_exponent = 0;
		twt_agrt->agrt_para_bitmap = 0;
	}

	twt_agrt->is_role_ap = curr_twt_node->is_role_ap;
	twt_agrt->grp_member_cnt = curr_twt_node->grp_member_cnt;
	if (twt_agrt->grp_member_cnt) {
		for (i = 0; i < TWT_HW_GRP_MAX_MEMBER_CNT; i++)
			twt_agrt->sta_list[i] = curr_twt_node->sta_list[i];
	}

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("\t%s:agrt_tbl_idx(%d)\n", __func__, twt_agrt->agrt_tbl_idx));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("\t%s:agrt_ctrl_flag(%d)\n", __func__, twt_agrt->agrt_ctrl_flag));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("\t%s:own_mac_idx(%d)\n", __func__, twt_agrt->own_mac_idx));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("\t%s:flow_id(%d)\n", __func__, twt_agrt->flow_id));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("\t%s:peer_id_grp_id(%d)\n", __func__, twt_agrt->peer_id_grp_id));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("\t%s:agrt_sp_duration(%d)\n", __func__, twt_agrt->agrt_sp_duration));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("\t%s:bss_idx(%d)\n", __func__, twt_agrt->bss_idx));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("\t%s:agrt_sp_start_tsf_low(0x%.8x)\n", __func__, twt_agrt->agrt_sp_start_tsf_low));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("\t%s:agrt_sp_start_tsf_high(0x%.8x)\n", __func__, twt_agrt->agrt_sp_start_tsf_high));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("\t%s:agrt_sp_wake_intvl_mantissa(%d)\n", __func__, twt_agrt->agrt_sp_wake_intvl_mantissa));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("\t%s:agrt_sp_wake_intvl_exponent(%d)\n", __func__, twt_agrt->agrt_sp_wake_intvl_exponent));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("\t%s:is_role_ap(%d)\n", __func__, twt_agrt->is_role_ap));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("\t%s:agrt_para_bitmap(0x%x)\n", __func__, twt_agrt->agrt_para_bitmap));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("\t%s:grp_member_cnt(%d)\n", __func__, twt_agrt->grp_member_cnt));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("\t%s:sta_list[%d,%d,%d,%d,%d,%d,%d,%d]\n",
		__func__,
		twt_agrt->sta_list[0],
		twt_agrt->sta_list[1],
		twt_agrt->sta_list[2],
		twt_agrt->sta_list[3],
		twt_agrt->sta_list[4],
		twt_agrt->sta_list[5],
		twt_agrt->sta_list[6],
		twt_agrt->sta_list[7]));
}

static VOID twt_tbl_free(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN struct twt_ie *twt_ie_in,
	IN UINT8 agrt_type,
	OUT PUINT8 agrt_tbl_idx,
	OUT PUINT8 setup_cmd,
	OUT PUINT32 tsf,
	OUT PUINT8 flow_id)
{
	/*
	* 1. peer flow id < 8
	* 2. peer is not in the group
	* 3. acauire agrt_tbl
	* 4. check database and assign proper tsf position
	* 5. reply action frame
	* 6. cmd-event
	*/
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct _RTMP_ADAPTER *ad = NULL;
	struct hdev_ctrl *ctrl = NULL;
	struct hdev_obj *obj = NULL;

	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *twt_node = NULL;
	struct twt_agrt_para twt_agrt = {0};

	UINT16 req_type = twt_ie_in->req_type;
	UINT8 steup_cmd = 0;
	BOOLEAN trigger = 0;
	BOOLEAN implicit = 0;
	BOOLEAN flow_type = 0;
	UINT8 flow_identifier = 0;
	UINT8 exponent = 0;
	BOOLEAN protection = 0;
	UINT8 duration = 0;
	UINT16 mantissa = 0;
	UINT8 channel = 0;

	UINT32 current_tsf[2];
	UINT64 twt_interval = 0;
	UINT64 twt_mod = 0;
	UINT64 twt_current_tsf = 0;
	UINT64 twt_assigned_tsf = 0;
	UINT64 agrt_tsf = 0;
	UINT64 temp = 0;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	entry = &ad->MacTab.Content[wcid];

	*setup_cmd = TWT_SETUP_CMD_ACCEPT;
	ctrl = hc_get_hdev_ctrl(wdev);
	obj = wdev->pHObj;

	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	/* get peer twt request type */
	steup_cmd = GET_TWT_RT_SETUP_CMD(req_type);
	trigger = GET_TWT_RT_TRIGGER(req_type);
	implicit = GET_TWT_RT_IMPLICIT_LAST(req_type);
	flow_type = GET_TWT_RT_FLOW_TYPE(req_type);
	flow_identifier = twt_get_free_flow_id(wdev, wcid);
	*flow_id = flow_identifier;
	exponent = GET_TWT_RT_WAKE_INTVAL_EXP(req_type);
	protection = GET_TWT_RT_PROTECTION(req_type);
	duration = twt_ie_in->duration;
	mantissa = twt_ie_in->mantissa;
	channel = twt_ie_in->channel;
	agrt_tsf = twt_ie_in->target_wake_time[1];
	agrt_tsf = (agrt_tsf << 32) + twt_ie_in->target_wake_time[0];

	if ((agrt_type == TWT_TYPE_INDIVIDUAL) &&
		((entry->twt_flow_id_bitmap == 0xff) ||
		twt_agrt_in_group(wdev, wcid))) {
		*setup_cmd = TWT_SETUP_CMD_REJECT;
		return;
	}

	if ((agrt_type == TWT_TYPE_GROUP) &&
		((entry->twt_flow_id_bitmap == 0xff) ||
		twt_agrt_in_individual(wdev, wcid))) {
		*setup_cmd = TWT_SETUP_CMD_REJECT;
		return;
	}

	if (twt_agrt_exist(wdev, wcid, flow_identifier)) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: twt_agrt with wcid=%d, flow_id=%d exist, REJECT\n",
			__func__,
			wcid,
			flow_identifier));
		*setup_cmd = TWT_SETUP_CMD_REJECT;
		return;
	}

	/* optimize twt parameters */
	twt_para_optimize(&mantissa, &exponent, &duration);

	/* fill twt agrt parameters */
	/* update entry */
	entry->twt_flow_id_bitmap |= (1 << flow_identifier);

	/* update node */
	twt_node = twt_ctrl_acquire_twt_node(ctrl, obj, agrt_type);
	if (!twt_node) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: twt_ctrl_acquire_twt_node: fail!\n", __func__));
		return;
	}
	*agrt_tbl_idx = twt_node->agrt_tbl_idx;
	twt_node->own_mac_idx = obj->OmacIdx;
	twt_node->flow_id = flow_identifier;

	if (agrt_type == TWT_TYPE_INDIVIDUAL)
		twt_node->peer_id_grp_id = wcid;

	twt_node->agrt_sp_duration = duration;
	twt_node->bss_idx = wdev->bss_info_argument.ucBssIndex;
	twt_node->agrt_sp_wake_intvl_mantissa = mantissa;
	twt_node->agrt_sp_wake_intvl_exponent = exponent;
	twt_node->agrt_sp_start_tsf = agrt_tsf;
	twt_node->is_role_ap = TWT_ROLE_AP;

	CLR_AGRT_PARA_BITMAP(twt_node);
	if (trigger)
		SET_AGRT_PARA_BITMAP(twt_node, TWT_AGRT_PARA_BITMAP_IS_TRIGGER);
	if (!flow_type)
		SET_AGRT_PARA_BITMAP(twt_node, TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE);
	if (protection)
		SET_AGRT_PARA_BITMAP(twt_node, TWT_AGRT_PARA_BITMAP_IS_PROTECT);
	if (agrt_type == TWT_TYPE_GROUP) {
		twt_node->grp_member_cnt = 1;
		twt_node->sta_list[0] = wcid;
		twt_node->grp_grade = twt_grp_grade(mantissa, exponent, duration);
	}

	if (steup_cmd == TWT_SETUP_CMD_REQUEST)
		twt_node->tsf_type = TSF_FROM_SETUP_CMD_REQUEST;
	else if (steup_cmd == TWT_SETUP_CMD_SUGGEST)
		twt_node->tsf_type = TSF_FROM_SETUP_CMD_SUGGEST;
	else if (steup_cmd == TWT_SETUP_CMD_DEMAND)
		twt_node->tsf_type = TSF_FROM_SETUP_CMD_DEMAND;
	else
		twt_node->tsf_type = TSF_FROM_SETUP_CMD_REQUEST;

	twt_link_insert_node(wdev, twt_node);

	/* handle tsf */
	if ((steup_cmd == TWT_SETUP_CMD_REQUEST) || (steup_cmd == TWT_SETUP_CMD_SUGGEST)) {
		twt_get_current_tsf(wdev, current_tsf);
		twt_current_tsf = current_tsf[0] + (((UINT64)current_tsf[1]) << 32);
		twt_interval = ((UINT64)(twt_node->agrt_sp_wake_intvl_mantissa)) << twt_node->agrt_sp_wake_intvl_exponent;
		temp = twt_current_tsf - twt_node->schedule_sp_start_tsf;
		twt_mod = mod_64bit(temp, twt_interval);
		twt_node->schedule_sp_start_tsf_abs = (twt_current_tsf + (twt_interval - twt_mod));
		twt_assigned_tsf = twt_node->schedule_sp_start_tsf_abs;
	} else if (steup_cmd == TWT_SETUP_CMD_DEMAND) {
		twt_assigned_tsf = twt_node->agrt_sp_start_tsf;
	}
	os_move_mem(tsf, &twt_assigned_tsf, sizeof(twt_assigned_tsf));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
	    ("%s:steup_cmd=%d,%s current_tsf(%llu) and assigned_tsf(%llu),wcid=%d,agrt_type=%d,tbl_i(%d)\n",
	    __func__,
	    steup_cmd,
	    (steup_cmd == TWT_SETUP_CMD_DEMAND) ? "STA" : "AP",
	    twt_current_tsf,
	    twt_assigned_tsf,
	    twt_node->peer_id_grp_id,
	    agrt_type,
	    twt_node->agrt_tbl_idx));

	/* twt h/w control new*/
	twt_agrt_cmd_set(&twt_agrt,
		twt_node,
		TWT_AGRT_CTRL_ADD,
		(twt_node->tsf_type == TSF_FROM_SETUP_CMD_DEMAND) ? CMD_TSF_TYPE_REQUESTER : CMD_TST_TYPE_SCHEDULE);
	mt_asic_twt_agrt_update(wdev, twt_agrt);
}

static VOID twt_tbl_full(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN struct twt_ie *twt_ie_in,
	OUT PUINT8 agrt_tbl_idx,
	OUT PUINT8 setup_cmd,
	OUT PUINT32 tsf,
	OUT PUINT8 flow_id)
{
#if (MTK_TWT_GROUP_EN == 1)
	/*
	* 1. exactly the same case: find twt node with grp_member_cnt<8 && sp/interval,announcement/trigger the same
	* 2. similar case: find twt node with grp_member_cnt 8 && grp_grade diff = min
	* 3. assign this peer sta to twt grp[index]
	*/
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct _RTMP_ADAPTER *ad = NULL;
	UINT16 req_type = twt_ie_in->req_type;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	UINT8 grp_grade = 0;
	UINT8 exponent = 0;
	UINT8 duration = 0;
	UINT16 mantissa = 0;
	UINT8 diff_grade[TWT_HW_GRP_MAX_NUM];
	UINT8 offset = TWT_HW_AGRT_MAX_NUM - TWT_HW_GRP_MAX_NUM;
	UINT8 i = 0;
	UINT8 index = 0;
	UINT8 min_diff = MAX_GRP_GRADE;
	UINT8 sch_link_idx = 0;
	BOOLEAN the_same_twt_tempo = FALSE;
	BOOLEAN trigger = 0;
	BOOLEAN flow_type = 0;
	UINT8 flow_identifier = 0;
	BOOLEAN protection = 0;
	UINT8 agrt_para_bitmap = 0;

	UINT32 current_tsf[2];
	UINT64 twt_interval = 0;
	UINT64 twt_mod = 0;
	UINT64 twt_current_tsf = 0;
	UINT64 twt_assigned_tsf = 0;
	UINT64 temp = 0;
	UINT64 twt_interval_old = 0;
	UINT64 twt_interval_new = 0;
	UINT8 all_grp_with_full_member_cnt = 0;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	trigger = GET_TWT_RT_TRIGGER(req_type);
	flow_type = GET_TWT_RT_FLOW_TYPE(req_type);
	flow_identifier = twt_get_free_flow_id(wdev, wcid);
	*flow_id = flow_identifier;
	exponent = GET_TWT_RT_WAKE_INTVAL_EXP(req_type);
	protection = GET_TWT_RT_PROTECTION(req_type);

	if (trigger)
		agrt_para_bitmap |= TWT_AGRT_PARA_BITMAP_IS_TRIGGER;
	if (!flow_type)
		agrt_para_bitmap |= TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE;
	if (protection)
		agrt_para_bitmap |= TWT_AGRT_PARA_BITMAP_IS_PROTECT;

	duration = twt_ie_in->duration;
	mantissa = twt_ie_in->mantissa;

	grp_grade = twt_grp_grade(mantissa, exponent, duration);
	twt_interval_new = ((UINT64)mantissa) << exponent;

    /* twt tempo is exactly the same then ACCEPT */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
		    curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					("%s: twt_entry=NULL, please check\n", __func__));
				return;
			}
			twt_interval_old = ((UINT64)(curr_twt_node->agrt_sp_wake_intvl_mantissa)) << curr_twt_node->agrt_sp_wake_intvl_exponent;

			/* check if exactly the same twt tempo exist */
			if (twt_is_grpid(curr_twt_node->peer_id_grp_id) &&
				(curr_twt_node->grp_member_cnt != TWT_HW_GRP_MAX_MEMBER_CNT) &&
				(curr_twt_node->agrt_sp_duration == duration) &&
				(twt_interval_old == twt_interval_new) &&
				(curr_twt_node->agrt_para_bitmap == agrt_para_bitmap)) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
					("%s: twt tempo the same with tbl_i=%d \n", __func__, curr_twt_node->agrt_tbl_idx));
				the_same_twt_tempo = TRUE;
				break;
			}
		}
	}

	if (the_same_twt_tempo) {
		struct twt_agrt_para twt_agrt = {0};

		/* action frame setup cmd update */
		*agrt_tbl_idx = curr_twt_node->agrt_tbl_idx;
		*setup_cmd = TWT_SETUP_CMD_ACCEPT;

		/* handle tsf */
		twt_get_current_tsf(wdev, current_tsf);
		twt_current_tsf = current_tsf[0] + (((UINT64)current_tsf[1]) << 32);
		twt_interval = ((UINT64)(curr_twt_node->agrt_sp_wake_intvl_mantissa)) << curr_twt_node->agrt_sp_wake_intvl_exponent;
		temp = twt_current_tsf - curr_twt_node->schedule_sp_start_tsf;
		twt_mod = mod_64bit(temp, twt_interval);
		twt_assigned_tsf = twt_current_tsf + (twt_interval - twt_mod) + twt_interval;
		tsf[0] = (UINT32)(twt_assigned_tsf & 0xffffffff);
		tsf[1] = (UINT32)(twt_assigned_tsf >> 32);

		/* update entry */
		ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
		entry = &ad->MacTab.Content[wcid];
		entry->twt_flow_id_bitmap |= (1 << flow_identifier);

		/* update twt node */
		curr_twt_node->grp_member_cnt++;
		for (i = 0; i < curr_twt_node->grp_member_cnt; i++) {
			if (curr_twt_node->sta_list[i] == 0) {
				curr_twt_node->sta_list[i] = wcid;
				break;
			}
		}

		/* send modified cmd to h/w */
		twt_agrt.agrt_tbl_idx = curr_twt_node->agrt_tbl_idx;
		twt_agrt.agrt_ctrl_flag = TWT_AGRT_CTRL_MODIFY;
		twt_agrt.own_mac_idx = curr_twt_node->own_mac_idx;
		twt_agrt.flow_id = curr_twt_node->flow_id;
		twt_agrt.peer_id_grp_id = curr_twt_node->peer_id_grp_id;
		twt_agrt.agrt_sp_duration = curr_twt_node->agrt_sp_duration;
		twt_agrt.bss_idx = curr_twt_node->bss_idx;
		twt_agrt.agrt_sp_start_tsf_low = tsf[0];
		twt_agrt.agrt_sp_start_tsf_high = tsf[1];
		twt_agrt.agrt_sp_wake_intvl_mantissa = curr_twt_node->agrt_sp_wake_intvl_mantissa;
		twt_agrt.agrt_sp_wake_intvl_exponent = curr_twt_node->agrt_sp_wake_intvl_exponent;
		twt_agrt.is_role_ap = TWT_ROLE_AP;
		twt_agrt.agrt_para_bitmap = curr_twt_node->agrt_para_bitmap;
		twt_agrt.grp_member_cnt = curr_twt_node->grp_member_cnt;
		for (i = 0; i < curr_twt_node->grp_member_cnt; i++) {
			twt_agrt.sta_list[i] = curr_twt_node->sta_list[i];
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
				("%s: sta_list[%d]=%d\n", __func__, i, twt_agrt.sta_list[i]));
		}

		mt_asic_twt_agrt_update(wdev, twt_agrt);

		return;
	}

	/* twt tempo is similar check */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(curr_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			if (!curr_twt_node) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					("%s: twt_entry=NULL, please check\n", __func__));
				return;
			}

			/* get grade difference for similar tempo twt check */
			if (twt_is_grpid(curr_twt_node->peer_id_grp_id)) {
				diff_grade[twt_grp_tbl_idx(curr_twt_node->peer_id_grp_id)] =
					(grp_grade >= curr_twt_node->grp_grade) ? (grp_grade - curr_twt_node->grp_grade) : (curr_twt_node->grp_grade - grp_grade);
			}

			if (curr_twt_node->grp_member_cnt == TWT_HW_GRP_MAX_MEMBER_CNT)
				all_grp_with_full_member_cnt++;
		}
	}

	if (all_grp_with_full_member_cnt == TWT_HW_GRP_MAX_NUM) {
		/* agrt twt (i+g) are fully ocupied and do REJECT reply */
		*agrt_tbl_idx = index;
		*setup_cmd = TWT_SETUP_CMD_REJECT;
	} else {
		/* find a similar tempo agrt twt and do ALTERNATE reply */
		for (i = 0; i < TWT_HW_GRP_MAX_NUM; i++) {
			if (diff_grade[i] <= min_diff) {
				min_diff = diff_grade[i];
				index = i;
			}
		}
		*agrt_tbl_idx = index + offset;
		*setup_cmd = TWT_SETUP_CMD_ALTERNATE;
	}
#else
	*setup_cmd = TWT_SETUP_CMD_REJECT;
#endif /* MTK_TWT_GROUP_EN */
}

static BOOLEAN twt_setup_frame_sanity(
	IN struct twt_ie *twt_ie)
{
	UINT8 len = 0;
	BOOLEAN implicit = 0;

	if (twt_ie->elem_id != IE_TWT) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: elem_id!=%d incorrect, please check\n", __func__, IE_TWT));
		return FALSE;
	}

	len = sizeof(struct twt_ie) - 2;
	if (twt_ie->len != len) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: twt ie len!=%d incorrect, please check\n", __func__, len));
		return FALSE;
	}

	implicit = GET_TWT_RT_IMPLICIT_LAST(twt_ie->req_type);
	if (implicit != TRUE) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: implicit!=TRUE incorrect, please check\n", __func__));
		return FALSE;
	}

	if (twt_ie->channel != 0) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: channel!=0 incorrect, please check\n", __func__));
		return FALSE;
	}

	return TRUE;
}

/* peer agrt request invoked by assoc. request or action frame phase */
static VOID twt_agrt_request(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN struct twt_ie *twt_ie_in,
	OUT BOOLEAN *tbl_free,
	OUT PUINT8 agrt_tbl_idx,
	OUT PUINT8 setup_cmd,
	OUT PUINT32 tsf,
	OUT PUINT8 flow_id)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);
	UINT8 i_tbl_free_cnt = 0;
	UINT8 g_tbl_free_cnt = 0;

	twt_ctrl_get_free_twt_node_num(ctrl, &i_tbl_free_cnt, &g_tbl_free_cnt);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("%s:i_tbl_free_cnt(%d),g_tbl_free_cnt(%d)\n", __func__, i_tbl_free_cnt, g_tbl_free_cnt));

	if (i_tbl_free_cnt != 0) {
		/* assign i twt */
		twt_tbl_free(wdev, wcid, twt_ie_in, TWT_TYPE_INDIVIDUAL, agrt_tbl_idx, setup_cmd, tsf, flow_id);
		*tbl_free = TRUE;
	} else if ((i_tbl_free_cnt == 0) && (g_tbl_free_cnt != 0)) {
		/* assign g twt */
		twt_tbl_free(wdev, wcid, twt_ie_in, TWT_TYPE_GROUP, agrt_tbl_idx, setup_cmd, tsf, flow_id);
		*tbl_free = TRUE;
	} else if ((i_tbl_free_cnt == 0) && (g_tbl_free_cnt == 0)) {
		/* check exactly the same or similar tempo twt */
		twt_tbl_full(wdev, wcid, twt_ie_in, agrt_tbl_idx, setup_cmd, tsf, flow_id);
		*tbl_free = FALSE;
	}
}

static VOID twt_build_twt_ie(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN struct twt_ie *twt_ie_in,
	OUT struct twt_ie *twt_ie_out)
{
	UINT16 req_type = 0;
	UINT8 setup_cmd = TWT_SETUP_CMD_ACCEPT;
	BOOLEAN trigger = 0;
	BOOLEAN implicit = 0;
	BOOLEAN flow_type = 0;
	UINT8 flow_identifier = 0;
	UINT8 exponent = 0;
	BOOLEAN protection = 0;
	UINT8 agrt_tbl_idx = TWT_HW_AGRT_MAX_NUM;
	UINT8 duration = 0;
	UINT16 mantissa = 0;
	UINT8 channel = 0;
	BOOLEAN tbl_free = TRUE;
	UINT32 tsf[2] = {0};

	struct hdev_ctrl *ctrl = NULL;
	struct twt_link_node *twt_node = NULL;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return;
	}

	if (!twt_setup_frame_sanity(twt_ie_in))
		setup_cmd = TWT_SETUP_CMD_REJECT;

	/*** twt agrt request and come out setup_cmd (accept, reject, alternate) ***/
	if (setup_cmd != TWT_SETUP_CMD_REJECT)
		twt_agrt_request(wdev, wcid, twt_ie_in, &tbl_free, &agrt_tbl_idx, &setup_cmd, tsf, &flow_identifier);

	/*** twt_ie_out assignment ***/
	req_type = twt_ie_in->req_type;

	if (((setup_cmd == TWT_SETUP_CMD_ACCEPT) &&	tbl_free) ||
		(setup_cmd == TWT_SETUP_CMD_REJECT)) {
		/* ACCEPT: i or g free case, REJECT: i+g are fully occupied */
		trigger = GET_TWT_RT_TRIGGER(req_type);
		implicit = GET_TWT_RT_IMPLICIT_LAST(req_type);
		flow_type = GET_TWT_RT_FLOW_TYPE(req_type);
		exponent = GET_TWT_RT_WAKE_INTVAL_EXP(req_type);
		protection = GET_TWT_RT_PROTECTION(req_type);
		duration = twt_ie_in->duration;
		mantissa = twt_ie_in->mantissa;
		channel = twt_ie_in->channel;
	} else if (((setup_cmd == TWT_SETUP_CMD_ACCEPT) ||
		(setup_cmd == TWT_SETUP_CMD_ALTERNATE)) &&
		!tbl_free) {
		/* ACCEPT: twt tempo is exactly the same */
		/* ALTERNATE: find similar twt tempo and apply grp twt parameters */
		ctrl = hc_get_hdev_ctrl(wdev);
		twt_node = twt_ctrl_get_twt_node_by_index(ctrl, agrt_tbl_idx);
		trigger = (twt_node->agrt_para_bitmap & TWT_AGRT_PARA_BITMAP_IS_TRIGGER) ? TRUE : FALSE;
		implicit = TRUE;
		flow_type = (twt_node->agrt_para_bitmap & TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE) ? FALSE : TRUE;
		exponent = twt_node->agrt_sp_wake_intvl_exponent;
		protection = (twt_node->agrt_para_bitmap & TWT_AGRT_PARA_BITMAP_IS_PROTECT) ? TRUE : FALSE;
		duration = twt_node->agrt_sp_duration;
		mantissa = twt_node->agrt_sp_wake_intvl_mantissa;
		channel = 0;
	} else {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: incorrect, setup_cmd=%d, tbl_i=%d, tbl_free:%d, please check\n",
			__func__, setup_cmd, agrt_tbl_idx, tbl_free));
		return;
	}

	/* control */
	os_zero_mem(twt_ie_out, sizeof(struct twt_ie));
	twt_ie_out->elem_id = IE_TWT;
	twt_ie_out->len = sizeof(struct twt_ie) - 2;

	twt_ie_out->control &= ~TWT_CTRL_NDP_PAGING_INDICATOR;
	twt_ie_out->control &= ~TWT_CTRL_RESPONDER_PM_MODE;
	twt_ie_out->control |= SET_TWT_CTRL_NEGO_TYPE(GET_TWT_CTRL_NEGO_TYPE(twt_ie_in->control));
	twt_ie_out->control |= SET_TWT_CTRL_INFO_FRM_DIS(GET_TWT_CTRL_INFO_FRM_DIS(twt_ie_in->control));
	twt_ie_out->control |= SET_TWT_CTRL_WAKE_DUR_UNIT(GET_TWT_CTRL_WAKE_DUR_UNIT(twt_ie_in->control));

	/* request type */
	twt_ie_out->req_type &= ~TWT_REQ_TYPE_TWT_REQUEST;
	twt_ie_out->req_type |= SET_TWT_RT_SETUP_CMD(setup_cmd);
	twt_ie_out->req_type |= SET_TWT_RT_TRIGGER(trigger);
	twt_ie_out->req_type |= SET_TWT_RT_IMPLICIT_LAST(implicit);
	twt_ie_out->req_type |= SET_TWT_RT_FLOW_TYPE(flow_type);
	twt_ie_out->req_type |= SET_TWT_RT_FLOW_ID(flow_identifier);
	twt_ie_out->req_type |= SET_TWT_RT_WAKE_INTVAL_EXP(exponent);
	twt_ie_out->req_type |= SET_TWT_RT_PROTECTION(protection);

	/* target wake time */
	twt_ie_out->target_wake_time[0] = cpu2le32(tsf[0]);
	twt_ie_out->target_wake_time[1] = cpu2le32(tsf[1]);

	/* duration */
	twt_ie_out->duration = duration;

	/* matissa */
	twt_ie_out->mantissa = mantissa;

	/* channel */
	twt_ie_out->channel = channel;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("%s: reply wcid=%d, f_id=%d, setup_cmd=%d, tbl_i=%d, tbl_free=%d\n",
		__func__, wcid, flow_identifier, setup_cmd, agrt_tbl_idx, tbl_free));
}

VOID parse_twt_ie(
	IN struct _EID_STRUCT *ie_head,
	IN VOID *ie_list)
{
	struct _IE_lists *ie = (struct _IE_lists *)ie_list;

	os_move_mem(&ie->twt_ie, ie_head, sizeof(struct twt_ie));
}

PUINT8 build_twt_ie(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	OUT PUINT8 f_buf,
	IN PVOID ie_list)
{
	struct _IE_lists *ie = (struct _IE_lists *)ie_list;
	struct twt_ie *twt_ie_in = &ie->twt_ie;
	struct twt_ie *twt_ie_out = (struct twt_ie *)f_buf;
	PUINT8 pos = f_buf;

	twt_build_twt_ie(wdev, wcid, twt_ie_in, twt_ie_out);

	pos += sizeof(struct twt_ie);

	return pos;
}

static VOID twt_teardown_request(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN UINT8 twt_flow_id)
{
	struct hdev_obj *obj = NULL;
	UINT8 sch_link_idx = 0;
	BOOLEAN found = FALSE;
	BOOLEAN process_last_wcid = FALSE;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct twt_agrt_para twt_agrt = {0};
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct _RTMP_ADAPTER *ad = NULL;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;
	obj = wdev->pHObj;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	entry = &ad->MacTab.Content[wcid];

	/*
	 * find twt database for peer wcid + twt flow id
	 * delete: in individial
	 * delete: in grp and grp_mem_cnt=1
	 * modify: in grp & grp_member>1
	 */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
		    curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					("%s: twt_entry=NULL, please check\n", __func__));
				return;
			}

			if ((curr_twt_node->peer_id_grp_id == wcid) &&
				(curr_twt_node->flow_id == twt_flow_id)) {
				entry->twt_flow_id_bitmap &= ~(1 << twt_flow_id);
				found = TRUE;
				process_last_wcid = TRUE;
				break;
			}
		}
	}

	if (found) {
		twt_agrt_cmd_set(&twt_agrt, curr_twt_node, TWT_AGRT_CTRL_DELETE, CMD_TSF_TYPE_NA);
		mt_asic_twt_agrt_update(wdev, twt_agrt);
		if (process_last_wcid) {
			twt_link_remove_node(wdev, curr_twt_node);
			twt_ctrl_release_twt_node(ctrl, obj, curr_twt_node);
		}
	} else {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: process=FALSE, wcid=%d,flow_id=%d\n", __func__, wcid, twt_flow_id));
	}
}

/* TWT action frame state machine management (for peer STA role) */
static VOID peer_twt_setup_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct wifi_dev *wdev = elem->wdev;
	PUCHAR out_buffer = NULL;
	ULONG frame_len = 0;

	struct frame_twt_setup *frame_in = (struct frame_twt_setup *)&elem->Msg;
	struct frame_twt_setup frame_out;
	struct twt_ie *twt_ie_in = &frame_in->twt_ie;
	struct twt_ie *twt_ie_out = &frame_out.twt_ie;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return;
	}

#ifdef APCLI_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA) {
		struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
		UCHAR ucTWTFlowId;
		struct twt_ie *prtwt_ie;

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR, ("-->%s\n", __func__));
		apcli_entry = &ad->StaCfg[wdev->func_idx];
		prtwt_ie = &(frame_in->twt_ie);

		if (!prtwt_ie) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: prtwt_ie == NULL!!!!\n", __func__));
			return;
		}
		ucTWTFlowId = twtGetRxSetupFlowId(prtwt_ie);
		twtParseTWTElement(prtwt_ie, &(apcli_entry->arTWTFlow[ucTWTFlowId].rTWTPeerParams));
		twtReqFsmRunEventRxSetup(ad, wdev, ucTWTFlowId);
	} else
#endif /* APCLI_SUPPORT */
	{
		/* get an unused nonpaged memory */
		if (os_alloc_mem(ad, &out_buffer, MAX_MGMT_PKT_LEN) != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: allocate memory failed, please check\n", __func__));
			return;
		}

		/* fill action frame content */
		os_zero_mem(&frame_out, sizeof(struct frame_twt_setup));
		ActHeaderInit(ad, &frame_out.hdr, frame_in->hdr.Addr2, wdev->if_addr, wdev->bssid);
		frame_out.category = CATEGORY_S1G;
		frame_out.s1g_action = CATE_S1G_ACTION_TWT_SETUP;
		frame_out.token = frame_in->token;
		twt_build_twt_ie(wdev, elem->Wcid, twt_ie_in, twt_ie_out);

		/* send action frame to peer sta */
		MakeOutgoingFrame(out_buffer, &frame_len,
			sizeof(struct frame_twt_setup), &frame_out,
			END_OF_ARGS);

		MiniportMMRequest(ad, QID_AC_BE, out_buffer, frame_len);

		os_free_mem(out_buffer);
	}
}

static VOID peer_twt_teardown_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct wifi_dev *wdev = elem->wdev;
	UINT16 wcid = elem->Wcid;
	struct frame_teardown *frame_in = NULL;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return;
	}

	frame_in = (struct frame_teardown *)&elem->Msg;

	/* handle twt_entry, twt_link_entry, twt cmd-event */
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
	    ("%s: wcid=%d,flow_id=%d\n", __func__, wcid, frame_in->twt_flow_id));

#ifdef APCLI_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA) {
		twtReqFsmRunEventRxTeardown(
			ad, wdev, frame_in->twt_flow_id);
	} else
#endif /* APCLI_SUPPORT */
	twt_teardown_request(wdev, wcid, frame_in->twt_flow_id);
}

VOID peer_twt_info_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
#if (TWT_IFNO_FRAME_EN == 1)
	struct wifi_dev *wdev = elem->wdev;
	UINT16 wcid = elem->Wcid;
	BOOLEAN found = FALSE;
	UINT8 found_num = 0;
	UINT8 sch_link_idx = 0;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct frame_twt_information *frame_in = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct twt_agrt_para twt_agrt = {0};
	UINT8 agrt_ctrl_flag = 0;
	UINT8 twt_flow_id = 0;
	UINT8 next_twt_subfield_size = 0;
	UINT8 all_twt = 0;
	UINT32 tsf[2] = {0};
	UINT64 tsf_64 = 0;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;
	frame_in = (struct frame_twt_information *)&elem->Msg;

    /* get twt inforamtion parameters */
	twt_flow_id = GET_TWT_INFO_FLOW_ID(frame_in->twt_info);
	next_twt_subfield_size = GET_TWT_INFO_NEXT_TWT_SUBFIELD_SIZE(frame_in->twt_info);
	if (next_twt_subfield_size)
		os_move_mem(&tsf, &frame_in->next_twt, sizeof(tsf));
	all_twt = GET_TWT_INFO_ALL_TWT(frame_in->twt_info);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("%s:wcid(%d),fid(%d),subfield_size(%d),all_twt(%d),next_twt(msb:0x%.8x,lsb:0x%.8x)\n",
		__func__, wcid, twt_flow_id, next_twt_subfield_size, all_twt, frame_in->next_twt[1], frame_in->next_twt[0]));

	/* find the twt agrt with flow id */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
		    curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					("%s:twt_entry=NULL,please check\n", __func__));
				return;
			}

			found = FALSE;
			if (all_twt) {
				if (curr_twt_node->peer_id_grp_id == wcid) {
					found_num++;
					found = TRUE;
				}
			} else {
				if ((curr_twt_node->peer_id_grp_id == wcid) &&
					(curr_twt_node->flow_id == twt_flow_id)) {
					found_num++;
					found = TRUE;
				}
			}
			/* handle suspend or suspend_resume */
			if (found) {
				curr_twt_node->is_role_ap = TWT_ROLE_AP;
				curr_twt_node->suspend = TRUE;

				CLR_AGRT_PARA_BITMAP(curr_twt_node);
				if (all_twt)
					SET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_ALL_TWT);

				if (next_twt_subfield_size) {
					/* suspend_resume */
					/* TODO: need FW event to update suspend from TRUE to FALSE */
					tsf_64 = tsf[1];
					tsf_64 = (tsf_64 << 32) + tsf[0];
					curr_twt_node->agrt_sp_info_tsf = tsf_64;
					if (next_twt_subfield_size == 1)
						SET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_NEXT_TWT_32_BITSIS);
					else if (next_twt_subfield_size == 2)
						SET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_NEXT_TWT_48_BITSIS);
					else if (next_twt_subfield_size == 3)
						SET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_NEXT_TWT_64_BITSIS);
					agrt_ctrl_flag = TWT_AGRT_CTRL_SUSPEND_RESUME;
				} else {
					/* suspend */
					curr_twt_node->agrt_sp_info_tsf = 0;
					agrt_ctrl_flag = TWT_AGRT_CTRL_SUSPEND;
				}

				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
					("%s:found wcid=%d,fid=%d agrt with all_twt(%d)\n",
					__func__, wcid, twt_flow_id, all_twt));

				twt_agrt_cmd_set(&twt_agrt, curr_twt_node, agrt_ctrl_flag, CMD_TSF_TYPE_TWT_INFO);
				mt_asic_twt_agrt_update(wdev, twt_agrt);
			}
		}
	}

	if (found_num == 0) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s:fail to find wcid=%d,fid=%d agrt with all_twt(%d)\n",
			__func__, wcid, twt_flow_id, all_twt));
	}
#else
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("%s:Not support\n", __func__));
#endif /* TWT_IFNO_FRAME_EN */
}

VOID peer_twt_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	PFRAME_802_11 frame = (PFRAME_802_11)&elem->Msg;
	/* Bypass HTC len 4 bytes */
	UINT8 htc_len = (frame->Hdr.FC.Order) ? 4 : 0;
	UINT8 action = 0;
	struct _MAC_TABLE_ENTRY *entry = NULL;

	if (!VALID_UCAST_ENTRY_WCID(ad, elem->Wcid))
		return;

	entry = &ad->MacTab.Content[elem->Wcid];
	if (!entry || !entry->wdev)
		return;

	if (IS_HE_STA(entry->cap.modes) ? FALSE : TRUE) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
				 ("%s: Non-HE STA, MaxCap=%s reject\n",
				 __func__,
				 get_phymode_str(entry->MaxHTPhyMode.field.MODE)));
		return;
	}

	if (htc_len) {
		NdisMoveMemory((void *)(elem->Msg+LENGTH_802_11),
			(void *)(elem->Msg+LENGTH_802_11+htc_len),
			(elem->MsgLen-htc_len));
	}

	/* Get S1G Action */
	action = elem->Msg[LENGTH_802_11 + 1];
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
			 ("%s: get twt action=%d\n", __func__, action));

	switch (action) {
	case CATE_S1G_ACTION_TWT_SETUP:
		peer_twt_setup_action(ad, elem);
		break;

	case CATE_S1G_ACTION_TWT_TEARDOWN:
		peer_twt_teardown_action(ad, elem);
		break;

	case CATE_S1G_ACTION_TWT_INFO:
		peer_twt_info_action(ad, elem);
		break;
	default:
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				 ("%s: unexpected action=%d, please check\n", __func__, action));
		break;
	}
}

/* TWT action frame trigger (for AP role) */
VOID twt_tear_down(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN UINT8 twt_flow_id)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct mlme_twt_tear_down_req_struct msg = {0};
	struct _MAC_TABLE_ENTRY *entry = NULL;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	entry = &ad->MacTab.Content[wcid];

	/* assign message */
	msg.wdev = wdev;
	msg.wcid = wcid;
	os_move_mem(&msg.peer_addr[0], entry->Addr, MAC_ADDR_LEN);
	msg.twt_flow_id = twt_flow_id;

	/* enqueue message */
	MlmeEnqueueWithWdev(ad,
		ACTION_STATE_MACHINE,
		MT2_MLME_S1G_CATE_TWT_TEARDOWN,
		sizeof(struct mlme_twt_tear_down_req_struct),
		(PVOID)&msg,
		0,
		wdev);
	RTMP_MLME_HANDLER(ad);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("%s: wcid(%d),flow_id(%d)\n",
		__func__,
		wcid,
		twt_flow_id));
}

VOID mlme_twt_teradown_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct mlme_twt_tear_down_req_struct *msg = NULL;
	struct frame_teardown frame_out;
	struct wifi_dev *wdev = NULL;
	PUCHAR out_buffer = NULL;
	ULONG frame_len = 0;
	NDIS_STATUS status = NDIS_STATUS_FAILURE;

	/* get an unused nonpaged memory */
	status = os_alloc_mem(ad, &out_buffer, MAX_MGMT_PKT_LEN);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
		("%s: allocate memory failed, please check\n", __func__));
		return;
	}

	msg = (struct mlme_twt_tear_down_req_struct *)&elem->Msg;
	wdev = msg->wdev;

	/* handle twt_node, twt_link_lsit, twt cmd-event */
	twt_teardown_request(wdev, msg->wcid, msg->twt_flow_id);

	/* send action frame to peer sta */
	os_zero_mem(&frame_out, sizeof(struct frame_teardown));
	ActHeaderInit(ad, &frame_out.hdr, msg->peer_addr, wdev->if_addr, wdev->bssid);
	frame_out.category = CATEGORY_S1G;
	frame_out.s1g_action = CATE_S1G_ACTION_TWT_TEARDOWN;
	frame_out.twt_flow_id = msg->twt_flow_id;

	MakeOutgoingFrame(out_buffer,
		&frame_len,
		sizeof(struct frame_teardown),
		&frame_out,
		END_OF_ARGS);
	MiniportMMRequest(ad,
		(MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[QID_AC_VO]),
		out_buffer, frame_len);

	os_free_mem(out_buffer);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("%s: wcid(%d), flow_id(%d)\n",
		__func__,
		msg->wcid,
		msg->twt_flow_id));
}

/* Peer STA link down twt management */
VOID twt_resource_release_at_link_down(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid)
{
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct _RTMP_ADAPTER *ad = NULL;
	UINT8 i = 0;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	entry = &ad->MacTab.Content[wcid];

	/* handle twt_node, twt_link_list, twt cmd-event */
	if (entry->twt_flow_id_bitmap != 0) {
		for (i = 0; i < TWT_FLOW_ID_MAX_NUM; i++) {
			if (entry->twt_flow_id_bitmap & (1 << i)) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
					("%s: wcid(%d), flow_id(%d)\n",
					__func__, wcid, i));
				twt_teardown_request(wdev, wcid, i);
			}
		}
	}
}

VOID twt_resource_dump(
	IN struct wifi_dev *wdev)
{
	struct hdev_ctrl *ctrl = NULL;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	twt_ctrl_resource_status_dump(ctrl);
}

VOID twt_get_current_tsf(
	struct wifi_dev *wdev,
	PUINT32 current_tsf)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	UINT32 high_part = 0;
	UINT32 low_part = 0;

	AsicGetTsfTime(ad, &high_part, &low_part, wdev->OmacIdx);
	current_tsf[0] = low_part;
	current_tsf[1] = high_part;
}

#ifdef APCLI_SUPPORT
VOID twtParseTWTElement(
	struct twt_ie *prTWTIE,
	struct twt_params_t *prTWTParams)
{
	UINT16 u2ReqType;

	u2ReqType = le2cpu16(prTWTIE->req_type);

	prTWTParams->fgReq = GET_TWT_RT_REQUEST(u2ReqType);
	prTWTParams->ucSetupCmd = GET_TWT_RT_SETUP_CMD(u2ReqType);
	prTWTParams->fgTrigger = GET_TWT_RT_TRIGGER(u2ReqType);
	prTWTParams->fgUnannounced = GET_TWT_RT_FLOW_TYPE(u2ReqType);
	prTWTParams->ucWakeIntvalExponent = GET_TWT_RT_WAKE_INTVAL_EXP(u2ReqType);
	prTWTParams->fgProtect = GET_TWT_RT_PROTECTION(u2ReqType);

	prTWTParams->u8TWT = le2cpu32(prTWTIE->target_wake_time[0]) |
		(((UINT64)(le2cpu32(prTWTIE->target_wake_time[1]))) << 32);

	prTWTParams->ucMinWakeDur = prTWTIE->duration;
	prTWTParams->u2WakeIntvalMantiss = le2cpu16(prTWTIE->mantissa);
}

UINT8 twtGetRxSetupFlowId(
	struct twt_ie *prTWTIE)
{
	UINT16 u2ReqType;
	u2ReqType = le2cpu16(prTWTIE->req_type);
	return GET_TWT_RT_FLOW_ID(u2ReqType);
}

VOID twtReqFsmRunEventRxSetup(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId
	)
{

	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;

	apcli_entry = &pAd->StaCfg[wdev->func_idx];
	if (!apcli_entry) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: apcli_entry=NULL, please check\n", __func__));
		return;
	}

	switch (apcli_entry->aeTWTReqState) {
	case TWT_REQ_STATE_WAIT_RSP:
		/* transition to the IDLE state */
		twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_IDLE,
			ucTWTFlowId, NULL);
		break;

	default:
		break;		/* Ignore other cases */
	}

}

VOID twtReqFsmRunEventRxTeardown(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId
)
{
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: apcli_entry=NULL, please check\n", __func__));
		return;
	}

	switch (apcli_entry->aeTWTReqState) {
	case TWT_REQ_STATE_IDLE:
		/* transition to the RX TEARDOWN state */
		twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_RX_TEARDOWN,
			ucTWTFlowId, NULL);
		break;

	default:
		break;		/* Ignore other cases */
	}
}

static VOID twtFillTWTElement(
	struct twt_ie *prTWTBuf,
	UINT8 ucTWTFlowId,
	struct twt_params_t *prTWTParams)
{
	/* Add TWT element */
	prTWTBuf->elem_id = IE_TWT;
	prTWTBuf->len = sizeof(struct twt_ie) - 2;

	/* Request Type */
	prTWTBuf->req_type |= SET_TWT_RT_REQUEST(prTWTParams->fgReq) |
		SET_TWT_RT_SETUP_CMD(prTWTParams->ucSetupCmd) |
		SET_TWT_RT_TRIGGER(prTWTParams->fgTrigger) |
		TWT_REQ_TYPE_TWT_IMPLICIT_LAST_BCAST_PARAM |
		SET_TWT_RT_FLOW_TYPE(prTWTParams->fgUnannounced) |
		SET_TWT_RT_FLOW_ID(ucTWTFlowId) |
		SET_TWT_RT_WAKE_INTVAL_EXP(prTWTParams->ucWakeIntvalExponent) |
		SET_TWT_RT_PROTECTION(prTWTParams->fgProtect);
	prTWTBuf->target_wake_time[0] = (cpu2le64(prTWTParams->u8TWT) & 0xFFFFFFFF);
	prTWTBuf->target_wake_time[1] = (cpu2le64(prTWTParams->u8TWT & 0xffffffff00000000) >> 32);
	prTWTBuf->duration = prTWTParams->ucMinWakeDur;
	prTWTBuf->mantissa =
		cpu2le16(prTWTParams->u2WakeIntvalMantiss);
}

UINT32 twtSendSetupFrame(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId,
	IN struct twt_params_t *prTWTParams)
{
	struct frame_twt_setup frame_out;
	struct twt_ie *twt_ie_out = &frame_out.twt_ie;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	PUCHAR out_buffer = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	ULONG frame_len = 0;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: apcli_entry=NULL, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: pEntry=NULL, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	if (os_alloc_mem(pAd, &out_buffer, MAX_MGMT_PKT_LEN) != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
		("%s: allocate memory failed, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	os_zero_mem(&frame_out, sizeof(struct frame_twt_setup));
	ActHeaderInit(pAd, &frame_out.hdr, pEntry->Addr, wdev->if_addr, wdev->bssid);
	frame_out.category = CATEGORY_S1G;
	frame_out.s1g_action = CATE_S1G_ACTION_TWT_SETUP;

	twtFillTWTElement(twt_ie_out, ucTWTFlowId, prTWTParams);

	/* send action frame to peer sta */
	MakeOutgoingFrame(out_buffer, &frame_len,
		sizeof(struct frame_twt_setup), &frame_out,
		END_OF_ARGS);

	MiniportMMRequest(pAd, QID_AC_BE, out_buffer, frame_len);
	os_free_mem(out_buffer);

	return NDIS_STATUS_SUCCESS;
}

UINT32 twtSendTeardownFrame(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId
	)
{
	struct frame_teardown frame_out;
	PUCHAR out_buffer = NULL;
	ULONG frame_len = 0;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: apcli_entry=NULL, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: pEntry=NULL, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	/* send action frame to peer sta */
	if (os_alloc_mem(pAd, &out_buffer, MAX_MGMT_PKT_LEN) != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
		("%s: allocate memory failed, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	os_zero_mem(&frame_out, sizeof(struct frame_teardown));
	ActHeaderInit(pAd, &frame_out.hdr, pEntry->Addr, wdev->if_addr, wdev->bssid);
	frame_out.category = CATEGORY_S1G;
	frame_out.s1g_action = CATE_S1G_ACTION_TWT_TEARDOWN;
	frame_out.twt_flow_id = ucTWTFlowId;

	MakeOutgoingFrame(out_buffer,
		&frame_len,
		sizeof(struct frame_teardown),
		&frame_out,
		END_OF_ARGS);
	MiniportMMRequest(pAd,
		(MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[QID_AC_VO]),
		out_buffer, frame_len);

	os_free_mem(out_buffer);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
		("%s: wcid(%d), flow_id(%d)\n",
		__func__,
		pEntry->wcid,
		ucTWTFlowId));

	return NDIS_STATUS_SUCCESS;
}

static UINT8 twtPlannerDrvAgrtFind(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId
	)
{
	UINT8 i;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	struct twt_planner_t *prTWTPlanner = NULL;
	struct twt_agrt_t *prTWTAgrt = NULL;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	prTWTPlanner = &(apcli_entry->rTWTPlanner);

	for (i = 0; i < TWT_AGRT_MAX_NUM; i++) {

		prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[i]);

		if (prTWTAgrt->fgValid == TRUE &&
			prTWTAgrt->ucFlowId == ucTWTFlowId &&
			prTWTAgrt->ucBssIdx == wdev->bss_info_argument.ucBssIndex)
			break;
	}

	return i;
}

static UINT32 twtPlannerDrvAgrtDel(
	IN struct twt_planner_t *prTWTPlanner,
	IN UINT8 ucIdx
	)
{
	struct twt_agrt_t *prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[ucIdx]);

	NdisZeroMemory(prTWTAgrt, sizeof(struct twt_agrt_t));

	return NDIS_STATUS_SUCCESS;
}

static UINT32 twtPlannerDelAgrtTbl(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId,
	IN UINT8 fgDelDrvEntry)
{
	UINT8 ucAgrtTblIdx;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	NDIS_STATUS status = NDIS_STATUS_FAILURE;
	struct twt_planner_t *prTWTPlanner = NULL;
	struct twt_agrt_para twt_agrt = {0};
	struct hdev_obj *obj = NULL;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: apcli_entry=NULL, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: pEntry=NULL, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	obj = wdev->pHObj;

	if (obj == NULL)
		return NDIS_STATUS_FAILURE;

	prTWTPlanner = &(apcli_entry->rTWTPlanner);

	/* Find and delete the agreement entry in the driver */
	ucAgrtTblIdx = twtPlannerDrvAgrtFind(pAd, wdev, ucTWTFlowId);

	if (ucAgrtTblIdx >= TWT_AGRT_MAX_NUM)
		return NDIS_STATUS_FAILURE;

	if (fgDelDrvEntry)
		twtPlannerDrvAgrtDel(prTWTPlanner, ucAgrtTblIdx);

	twt_agrt.agrt_tbl_idx = ucAgrtTblIdx;
	twt_agrt.agrt_ctrl_flag = TWT_AGRT_CTRL_DELETE;
	twt_agrt.own_mac_idx = obj->OmacIdx;
	twt_agrt.flow_id = ucTWTFlowId;
	twt_agrt.peer_id_grp_id = pEntry->wcid;
	twt_agrt.bss_idx = wdev->bss_info_argument.ucBssIndex;
	/* apcli */
	twt_agrt.is_role_ap = TWT_ROLE_APCLI;
	mt_asic_twt_agrt_update(wdev, twt_agrt);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_OFF,
		("%s del twt agrt to FW,wcid=%d,flow_id=%d,tbl_idx=%d\n",
		__func__, pEntry->wcid, ucTWTFlowId, ucAgrtTblIdx));

	return status;
}

UINT32 twtPlannerTeardownDone(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId
	)
{
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: apcli_entry=NULL, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: pEntry=NULL, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	/* Delete driver & FW TWT agreement entry */
	twtPlannerDelAgrtTbl(pAd, wdev, ucTWTFlowId, TRUE);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("%s: wcid(%d), flow_id(%d)\n", __func__,
		pEntry->wcid, ucTWTFlowId));

	return NDIS_STATUS_SUCCESS;
}

VOID twtPlannerSetParams(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN struct twt_ctrl_t rtwtCtrl)
{
	struct twt_ctrl_t rTWTCtrl, *prTWTCtrl = &rTWTCtrl;
	UINT_8 ucBssIdx, ucFlowId;
	NDIS_STATUS status = NDIS_STATUS_FAILURE;
	struct twt_get_tsf_context_t *prGetTsfCtxt = NULL;

	NdisCopyMemory(prTWTCtrl, &rtwtCtrl, sizeof(rTWTCtrl));

	ucBssIdx = prTWTCtrl->ucBssIdx;
	pAd = (struct _RTMP_ADAPTER *)(wdev->sys_handle);

	if (IS_TWT_PARAM_ACTION_ADD_BYPASS(prTWTCtrl->ucCtrlAction) ||
		IS_TWT_PARAM_ACTION_ADD(prTWTCtrl->ucCtrlAction)) {

		status = os_alloc_mem(pAd, (UCHAR **)&prGetTsfCtxt, sizeof(struct twt_get_tsf_context_t));

		if (prGetTsfCtxt == NULL)
			return;

		if (IS_TWT_PARAM_ACTION_ADD_BYPASS(prTWTCtrl->ucCtrlAction))
			prGetTsfCtxt->ucReason = TWT_GET_TSF_FOR_ADD_AGRT_BYPASS;
		else
			prGetTsfCtxt->ucReason = TWT_GET_TSF_FOR_ADD_AGRT;

		prGetTsfCtxt->ucBssIdx = ucBssIdx;
		prGetTsfCtxt->ucTWTFlowId = prTWTCtrl->ucTWTFlowId;
		NdisCopyMemory(&(prGetTsfCtxt->rTWTParams),
				&(prTWTCtrl->rTWTParams),
				sizeof(struct twt_params_t));
		twtPlannerGetCurrentTSF(pAd,
			wdev, prGetTsfCtxt, sizeof(*prGetTsfCtxt));



		if (prGetTsfCtxt)
			os_free_mem(prGetTsfCtxt);

	}

	ucFlowId = prTWTCtrl->ucTWTFlowId;

	switch (prTWTCtrl->ucCtrlAction) {
	case TWT_PARAM_ACTION_DEL:
		if (twtPlannerDrvAgrtFind(pAd,
			wdev, ucFlowId) < TWT_AGRT_MAX_NUM) {
			/* Start the process to tear down this TWT agreement */
			twtReqFsmSendEvent(pAd, wdev,
				ucFlowId, MID_TWT_REQ_FSM_TEARDOWN);
		} else {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				("BSS %u TWT flow %u doesn't exist\n\n", ucBssIdx, ucFlowId));
		}
		break;
	default:
		break;
	}
}

VOID twtPlannerRxNegoResult(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId)
{
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct twt_flow_t *prTWTFlow;
	struct twt_params_t *prTWTResult;
	struct twt_params_t *prTWTParams;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: apcli_entry=NULL, please check\n", __func__));
		return;
	}

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: pEntry=NULL, please check\n", __func__));
		return;
	}

	prTWTFlow = &(apcli_entry->arTWTFlow[ucTWTFlowId]);
	prTWTResult = &(prTWTFlow->rTWTPeerParams);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("%s: ucSetupCmd=%d\n", __func__, prTWTResult->ucSetupCmd));

	switch (prTWTResult->ucSetupCmd) {
	case TWT_SETUP_CMD_ACCEPT:
		/* Update agreement table */
		twtPlannerAddAgrtTbl(pAd, wdev, pEntry,
			prTWTResult, ucTWTFlowId);
		break;

	case TWT_SETUP_CMD_ALTERNATE:
	case TWT_SETUP_CMD_DICTATE:
		/* Use AP's suggestions */
		prTWTParams = &(prTWTFlow->rTWTParams);
		NdisCopyMemory(prTWTParams, prTWTResult, sizeof(struct twt_params_t));
		prTWTParams->ucSetupCmd = TWT_SETUP_CMD_SUGGEST;
		prTWTParams->fgReq = 1;
		twtReqFsmSendEvent(pAd, wdev, ucTWTFlowId, MID_TWT_REQ_FSM_START);
		break;

	case TWT_SETUP_CMD_REJECT:
		/* Clear TWT flow in StaRec */
		break;

	default:
		ASSERT(0);
		break;
	}
}

VOID twtReqFsmSendEvent(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId,
	IN UINT8 eMsgId)
{
	struct msg_twt_fsm_t rTWTReqFsmMsg = {0};

	rTWTReqFsmMsg.eMsgId = eMsgId;
	rTWTReqFsmMsg.wdev = wdev;
	rTWTReqFsmMsg.ucTWTFlowId = ucTWTFlowId;

	MlmeEnqueueWithWdev(pAd,
		ACTION_STATE_MACHINE,
		MT2_MLME_S1G_CATE_TWT_SETUP,
		sizeof(struct msg_twt_fsm_t),
		(PVOID)&rTWTReqFsmMsg,
		0,
		wdev);

	RTMP_MLME_HANDLER(pAd);
}

VOID twtReqFsmSteps(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN enum ENUM_TWT_REQUESTER_STATE_T eNextState,
	IN UINT8 ucTWTFlowId,
	IN void *pParam)
{
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	enum ENUM_TWT_REQUESTER_STATE_T ePreState;
	UINT8 fgIsTransition;
	NDIS_STATUS rStatus = NDIS_STATUS_SUCCESS;

	if (wdev == NULL)
		return;

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	do {
		ePreState = apcli_entry->aeTWTReqState;
		apcli_entry->aeTWTReqState = eNextState;
		fgIsTransition = (UINT8) FALSE;

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
			("twtReqFsmSteps ePreState=%d, eNextState=%d\n",
				ePreState, eNextState));

		switch (apcli_entry->aeTWTReqState) {
		case TWT_REQ_STATE_IDLE:
			/* Notify TWT Planner of the negotiation result */
			if (ePreState == TWT_REQ_STATE_WAIT_RSP) {
				twtReqFsmSendEvent(pAd, wdev,
					ucTWTFlowId, MID_TWT_REQ_IND_RESULT);
				/* TODO: how to handle failures */
			} else if (ePreState == TWT_REQ_STATE_TEARING_DOWN) {
				twtReqFsmSendEvent(pAd, wdev,
					ucTWTFlowId,
					MID_TWT_REQ_IND_TEARDOWN_DONE);
			} else if (ePreState == TWT_REQ_STATE_RESUMING) {
				twtReqFsmSendEvent(pAd, wdev,
					ucTWTFlowId,
					MID_TWT_REQ_IND_RESUME_DONE);
			}
			break;
		case TWT_REQ_STATE_REQTX:
			{
				struct twt_params_t *prTWTParams =
					(struct twt_params_t *)pParam;
				if (unlikely(prTWTParams == NULL)) {
					eNextState = TWT_REQ_STATE_IDLE;
					fgIsTransition = TRUE;
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
								("prTWTParams is NULL, TWT_REQ TX Failure!\n"));
					ASSERT(prTWTParams);
					break;
				}
				rStatus = twtSendSetupFrame(
					pAd, wdev, ucTWTFlowId,
					prTWTParams);
				if (rStatus != NDIS_STATUS_SUCCESS) {
					eNextState = TWT_REQ_STATE_IDLE;
					fgIsTransition = TRUE;
				}
				break;
			}
		case TWT_REQ_STATE_WAIT_RSP:
			break;
		case TWT_REQ_STATE_TEARING_DOWN:
			rStatus = twtSendTeardownFrame(
				pAd, wdev, ucTWTFlowId);
			if (rStatus != NDIS_STATUS_SUCCESS) {
				eNextState = TWT_REQ_STATE_IDLE;
				fgIsTransition = TRUE;
			}
			break;
		case TWT_REQ_STATE_RX_TEARDOWN:
			twtReqFsmSendEvent(pAd, wdev,
				ucTWTFlowId, MID_TWT_REQ_IND_TEARDOWN_DONE);
			break;
		default:
			/* nothing to do */
			break;
		}
	} while (fgIsTransition);
}

UINT32 twtPlannerDrvAgrtInsert(
	IN struct twt_planner_t *prTWTPlanner,
	IN UINT8 ucBssIdx,
	IN UINT8 ucFlowId,
	IN struct twt_params_t *prTWTParams,
	IN UINT8 ucIdx)
{
	struct twt_agrt_t *prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[ucIdx]);

	prTWTAgrt->fgValid = TRUE;
	prTWTAgrt->ucBssIdx = ucBssIdx;
	prTWTAgrt->ucFlowId = ucFlowId;
	prTWTAgrt->ucAgrtTblIdx = ucIdx;
	NdisCopyMemory(&(prTWTAgrt->rTWTAgrt), prTWTParams,
		sizeof(struct twt_params_t));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("%s: ucIdx=%d, ucBssIdx=%d, ucFlowId=%d\n", __func__,
			ucIdx, ucBssIdx, ucFlowId));

	return NDIS_STATUS_SUCCESS;
}

UINT32 twtPlannerDrvAgrtAdd(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucFlowId,
	IN struct twt_params_t *prTWTParams,
	IN UINT8 *pucIdx)
{
	UINT8 ucIdx;
	UINT32 rStatus = NDIS_STATUS_FAILURE;
	struct twt_planner_t *prTWTPlanner = NULL;
	struct twt_agrt_t *prTWTAgrt = NULL;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;

	if (!wdev) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s: wdev=NULL, please check\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];
	prTWTPlanner = &(apcli_entry->rTWTPlanner);

	for (ucIdx = 0; ucIdx < TWT_AGRT_MAX_NUM; ucIdx++) {
		prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[ucIdx]);
		if (prTWTAgrt->fgValid == FALSE)
			break;
	}

	if (ucIdx < TWT_AGRT_MAX_NUM) {
		twtPlannerDrvAgrtInsert(prTWTPlanner, wdev->bss_info_argument.ucBssIndex,
			ucFlowId, prTWTParams, ucIdx);
		*pucIdx = ucIdx;
		rStatus = NDIS_STATUS_SUCCESS;
	}

	return rStatus;
}

UINT32 twtPlannerAddAgrtTbl(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN struct twt_params_t *prTWTParams,
	IN UINT8 ucFlowId)
{
	UINT8 ucAgrtTblIdx;
	UINT32 rWlanStatus = NDIS_STATUS_SUCCESS;
	struct twt_agrt_para twt_agrt = {0};
	struct hdev_obj *obj = NULL;

	twtPlannerDbgPrintVal(pAd, prTWTParams);

	if (pEntry == NULL)
		return NDIS_STATUS_FAILURE;

	if (wdev == NULL)
		return NDIS_STATUS_FAILURE;

	rWlanStatus = twtPlannerDrvAgrtAdd(pAd, wdev,
		ucFlowId, prTWTParams, &ucAgrtTblIdx);

	if (rWlanStatus) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
				("%s: Agreement table is full\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	obj = wdev->pHObj;

	if (obj == NULL)
		return NDIS_STATUS_FAILURE;

	twt_agrt.agrt_tbl_idx = ucAgrtTblIdx;
	twt_agrt.agrt_ctrl_flag = TWT_AGRT_CTRL_ADD;
	twt_agrt.own_mac_idx = obj->OmacIdx;
	twt_agrt.flow_id = ucFlowId;
	twt_agrt.peer_id_grp_id = pEntry->wcid;
	twt_agrt.agrt_sp_duration = prTWTParams->ucMinWakeDur;
	twt_agrt.bss_idx = wdev->bss_info_argument.ucBssIndex;
	twt_agrt.agrt_sp_start_tsf_low = prTWTParams->u8TWT & 0xFFFFFFFF;
	twt_agrt.agrt_sp_start_tsf_high = (UINT32)(prTWTParams->u8TWT >> 32);
	twt_agrt.agrt_sp_wake_intvl_mantissa = prTWTParams->u2WakeIntvalMantiss;
	twt_agrt.agrt_sp_wake_intvl_exponent = prTWTParams->ucWakeIntvalExponent;
	/* TODO: aplci might need to use ap role */
	twt_agrt.is_role_ap = TWT_ROLE_APCLI;
	twt_agrt.agrt_para_bitmap =
		((prTWTParams->fgProtect << TWT_AGRT_PARA_BITMAP_PROTECT_OFFSET) |
		((!prTWTParams->fgUnannounced) << TWT_AGRT_PARA_BITMAP_ANNCE_OFFSET) |
		(prTWTParams->fgTrigger << TWT_AGRT_PARA_BITMAP_TRIGGER_OFFSET));

	twt_agrt.grp_member_cnt = 0;

	mt_asic_twt_agrt_update(wdev, twt_agrt);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_OFF,
		("%s: set twt agrt to FW,wcid=%d,flow_id=%d,tbl_idx=%d\n",
		__func__, pEntry->wcid, ucFlowId, ucAgrtTblIdx));

	return NDIS_STATUS_SUCCESS;
}

static struct twt_flow_t *twtPlannerFlowFindById(
	IN struct _STA_ADMIN_CONFIG *prStaCfg,
	IN UINT8 ucFlowId)
{
	struct twt_flow_t *prTWTFlow = NULL;

	ASSERT(prStaCfg);

	if (ucFlowId >= TWT_MAX_FLOW_NUM) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				("Invalid TWT flow id %u\n",
				ucFlowId));
		return NULL;
	}

	prTWTFlow = &(prStaCfg->arTWTFlow[ucFlowId]);

	return prTWTFlow;
}

UINT32 twtPlannerGetCurrentTSF(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN struct twt_get_tsf_context_t *prGetTsfCtxt,
	IN UINT32 u4SetBufferLen)
{
	UINT32 current_tsf[2] = {0};
	UINT64 u8CurTsf;
	UINT8 ucBssIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;

	if (wdev == NULL)
		return NDIS_STATUS_FAILURE;

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (apcli_entry == NULL)
		return NDIS_STATUS_FAILURE;

	twt_get_current_tsf(wdev, current_tsf);

	if (current_tsf[0] == 0 && current_tsf[1] == 0)
		return NDIS_STATUS_FAILURE;

	u8CurTsf = le2cpu32(current_tsf[0]) |
		(((UINT64)(le2cpu32(current_tsf[1]))) << 32);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("%s: u8CurTsf=%llu, Reason=%d\n", __func__,
		u8CurTsf, prGetTsfCtxt->ucReason));

	ucBssIdx = prGetTsfCtxt->ucBssIdx;

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (pEntry == NULL)
		return NDIS_STATUS_FAILURE;

	switch (prGetTsfCtxt->ucReason) {
	case TWT_GET_TSF_FOR_ADD_AGRT_BYPASS:
		prGetTsfCtxt->rTWTParams.u8TWT = u8CurTsf + TSF_OFFSET_FOR_EMU;
		twtPlannerAddAgrtTbl(pAd, wdev,
			pEntry, &(prGetTsfCtxt->rTWTParams),
			prGetTsfCtxt->ucTWTFlowId);
		break;

	case TWT_GET_TSF_FOR_ADD_AGRT:
	{
		struct twt_params_t *prTWTParams;
		struct twt_flow_t *prTWTFlow = twtPlannerFlowFindById(apcli_entry,
			prGetTsfCtxt->ucTWTFlowId);

		if (!prTWTFlow)	{
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("%s:NULL Pointer!\n", __func__));
			return NDIS_STATUS_FAILURE;
		}


		prGetTsfCtxt->rTWTParams.u8TWT =
			u8CurTsf + TSF_OFFSET_FOR_AGRT_ADD;
		prTWTParams = &(prTWTFlow->rTWTParams);
		NdisCopyMemory(prTWTParams, &(prGetTsfCtxt->rTWTParams),
			sizeof(struct twt_params_t));
		/* Start the process to nego for a new agreement */
		twtReqFsmSendEvent(pAd,
			wdev, prGetTsfCtxt->ucTWTFlowId, MID_TWT_REQ_FSM_START);

		break;
	}
	default:
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			("Unknown reason to get TSF %u\n", prGetTsfCtxt->ucReason));
		break;
	}

	return NDIS_STATUS_SUCCESS;
}

VOID twtTxDoneCheckSetupFrame(
	IN struct _RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pkt)
{
	HEADER_802_11 *pHead;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	UCHAR wdev_idx = 0;
	PFRAME_ACTION_HDR pActHdr;
	struct wifi_dev *wdev = NULL;
	struct frame_twt_setup frame_setup;
	struct frame_teardown frame_tear_down;
	struct twt_ie *twt_ie_out;
	UCHAR ucTWTFlowId;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;

	wdev_idx = RTMP_GET_PACKET_WDEV(pkt);
	wdev = pAd->wdev_list[wdev_idx];

	if (wdev == NULL || (wdev->wdev_type != WDEV_TYPE_STA))
		return;

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	pHead = (PHEADER_802_11)(GET_OS_PKT_DATAPTR(pkt) + tx_hw_hdr_len);

	if (pHead->FC.Type == FC_TYPE_MGMT && pHead->FC.SubType == SUBTYPE_ACTION) {
		pActHdr = (PFRAME_ACTION_HDR)pHead;

		if (pActHdr->Category != CATEGORY_S1G)
			return;

		switch (pActHdr->Action) {
		case CATE_S1G_ACTION_TWT_SETUP:
			if (apcli_entry->aeTWTReqState == TWT_REQ_STATE_REQTX) {
				NdisCopyMemory(&frame_setup, pHead, sizeof(struct frame_twt_setup));
				twt_ie_out = &frame_setup.twt_ie;
				ucTWTFlowId = twtGetRxSetupFlowId(twt_ie_out);
				twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_WAIT_RSP, ucTWTFlowId, NULL);
			}
			break;

		case CATE_S1G_ACTION_TWT_TEARDOWN:
			if (apcli_entry->aeTWTReqState == TWT_REQ_STATE_TEARING_DOWN) {
				NdisCopyMemory(&frame_tear_down, pHead, sizeof(struct frame_teardown));
				ucTWTFlowId = twtGetTxTeardownFlowId(&frame_tear_down);
				twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_IDLE, ucTWTFlowId, NULL);
			}
			break;

		default:
			break;
		} /* End of switch */
	}
}

VOID twtPlannerDbgPrintVal(
	IN struct _RTMP_ADAPTER *ad,
	IN struct twt_params_t *prTWTParams)
{
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("prTWTParams->fgReq = %d\n", prTWTParams->fgReq));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("prTWTParams->ucSetupCmd = %d\n", prTWTParams->ucSetupCmd));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("prTWTParams->fgTrigger = %d\n", prTWTParams->fgTrigger));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("prTWTParams->fgUnannounced = %d\n", prTWTParams->fgUnannounced));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("prTWTParams->ucWakeIntvalExponent = %d\n", prTWTParams->ucWakeIntvalExponent));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("prTWTParams->tsf_low = 0x%.8x\n", cpu2le32(prTWTParams->u8TWT & 0xFFFFFFFF)));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("prTWTParams->tsf_high = 0x%.8x\n", cpu2le32((UINT32)(prTWTParams->u8TWT >> 32))));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("prTWTParams->fgProtect = %d\n", prTWTParams->fgProtect));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("prTWTParams->ucMinWakeDur = %d\n", prTWTParams->ucMinWakeDur));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_TRACE,
		("prTWTParams->u2WakeIntvalMantiss = %d\n", prTWTParams->u2WakeIntvalMantiss));
}

UINT32 twtGetTxTeardownFlowId(
	IN struct frame_teardown *pframe_tear_down)
{
	UINT8 ucFlowId;

	ucFlowId = (pframe_tear_down->twt_flow_id & TWT_TEARDOWN_FLOW_ID);
	return ucFlowId;
}

VOID twtMlmeSetupAction(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct msg_twt_fsm_t *msg = NULL;
	struct wifi_dev *wdev = NULL;
	UINT8 ucFlowId;
	UCHAR eMsgId;
	struct twt_flow_t *prTWTFlow = NULL;
	STA_ADMIN_CONFIG *apcli_entry = NULL;
	struct twt_params_t *prTWTParams;

	msg = (struct msg_twt_fsm_t *)&elem->Msg;
	wdev = msg->wdev;
	ucFlowId = msg->ucTWTFlowId;
	eMsgId = msg->eMsgId;

	if (wdev == NULL)
		return;

	apcli_entry = &pAd->StaCfg[wdev->func_idx];
	prTWTFlow = twtPlannerFlowFindById(apcli_entry, ucFlowId);

	if (prTWTFlow == NULL)
		return;

	prTWTParams = &(prTWTFlow->rTWTParams);

	switch (eMsgId) {
	case MID_TWT_PARAMS_SET:
		twtPlannerSetParams(pAd, wdev, msg->rtwtCtrl);
		break;
	case MID_TWT_REQ_FSM_START:
		twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_REQTX, ucFlowId, prTWTParams);
		twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_WAIT_RSP, ucFlowId, prTWTParams);
		break;
	case MID_TWT_REQ_FSM_TEARDOWN:
		twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_TEARING_DOWN, ucFlowId, prTWTParams);
		twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_IDLE, ucFlowId, prTWTParams);
		break;
	case MID_TWT_REQ_IND_TEARDOWN_DONE:
		twtPlannerTeardownDone(pAd, wdev, ucFlowId);
		break;
	case MID_TWT_REQ_IND_RESULT:
		twtPlannerRxNegoResult(pAd, wdev, ucFlowId);
		break;
	default:
		/* nothing to do */
		break;
	}
}
#endif /* APCLI_SUPPORT */
#endif /* WIFI_TWT_SUPPORT */
#endif /* #ifdef DOT11_HE_AX */
