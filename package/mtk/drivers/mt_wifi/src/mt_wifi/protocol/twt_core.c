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
	if (peer_id_grp_id & TWT_GROUP_ID_BIT)
		return TRUE;
	else
		return FALSE;
}

#if (MTK_TWT_GROUP_EN == 1)
static UINT8 twt_grp_tbl_idx(UINT16 peer_id_grp_id)
{
	return ((UINT8)(peer_id_grp_id & (~TWT_GROUP_ID_BIT)));
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
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return FALSE;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(curr_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				return FALSE;
			}

			if (curr_twt_node->peer_id_grp_id == wcid) {
				wcid_in_individual = TRUE;
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

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
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return FALSE;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(curr_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
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
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

	return wcid_in_group;
}

#define ITWT_AGRT_PARA_BITMAP_CHECK (TWT_AGRT_PARA_BITMAP_WAKE_DUR_UINT | \
					TWT_AGRT_PARA_BITMAP_IS_TRIGGER)
static BOOLEAN twt_itwt_agrt_exist(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN struct itwt_ie *twt_ie_in,
	OUT struct twt_link_node **found_twt_node)
{
	BOOLEAN twt_agrt_exist = FALSE;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	UINT8 sch_link_idx = 0;
	UINT8 exponent = 0;
	UINT8 wake_dur_unit = 0;
	UINT8 agrt_para_bitmap = 0;
	BOOLEAN trigger = 0;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return FALSE;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;
	/* get incoming twt request info */
	wake_dur_unit = GET_TWT_CTRL_WAKE_DUR_UNIT(twt_ie_in->control);
	trigger = GET_TWT_RT_TRIGGER(twt_ie_in->req_type);
	exponent = GET_TWT_RT_WAKE_INTVAL_EXP(twt_ie_in->req_type);

	if (wake_dur_unit)
		agrt_para_bitmap |= TWT_AGRT_PARA_BITMAP_WAKE_DUR_UINT;
	if (trigger)
		agrt_para_bitmap |= TWT_AGRT_PARA_BITMAP_IS_TRIGGER;

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(curr_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				return FALSE;
			}

			/* TWT parameter is defined in ln39, page 186, IEEE P802.11ax/D7.0,
			 * Check related info to find out duplicated twt request.
			 */
			if ((curr_twt_node->peer_id_grp_id == wcid) &&
				((curr_twt_node->agrt_para_bitmap & ITWT_AGRT_PARA_BITMAP_CHECK) == agrt_para_bitmap) &&
				(curr_twt_node->agrt_sp_duration == twt_ie_in->duration) &&
				(curr_twt_node->agrt_sp_wake_intvl_mantissa == twt_ie_in->mantissa) &&
				(curr_twt_node->agrt_sp_wake_intvl_exponent == exponent) &&
				(curr_twt_node->twt_channel == twt_ie_in->channel)) {
				twt_agrt_exist = TRUE;
				*found_twt_node = curr_twt_node;
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
					"Duplicated twt mode is found\n");
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

	return twt_agrt_exist;
}


static UINT32 twt_align_duration(UINT32 sp_duration, UINT32 alignment)
{
	UINT32 sp_duration_alignment = 0;
	UINT32 m = sp_duration % alignment;

#if (TWT_TSF_ALIGNMENT_EN == 1)
	if (m == 0)
		sp_duration_alignment = sp_duration;
	else
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
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=%p, twt_node=%p, NULL, please check\n",
			wdev, twt_node);
		return;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);

	if ((twt_node->type == TWT_TYPE_INDIVIDUAL) &&
		(twt_node->tsf_type == TSF_FROM_SETUP_CMD_DEMAND)) { /* iTWT: demand */
		/* insert to unschedule link list */
		DlListAddTail(&twt_ctl->twt_link[USCH_LINK], &twt_node->list);
	} else { /* iTWT: suggest/resuest, bTWT */
		/* insert to schedule link list */
		sp_duration = twt_node->agrt_sp_duration << 8;
		if (DlListLen(&twt_ctl->twt_link[SCH_LINK]) == 0) {
			/* insert as the 1st node */
			twt_node->schedule_sp_start_tsf = 0;
			DlListAddTail(&twt_ctl->twt_link[SCH_LINK], &twt_node->list);
		} else if (DlListLen(&twt_ctl->twt_link[SCH_LINK]) == 1) {
			curr_twt_node = DlListFirst(&twt_ctl->twt_link[SCH_LINK], struct twt_link_node, list);

			if (!curr_twt_node) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"NULL Pointer!\n");
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
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL or ptwt_link_entry=NULL, please check\n");
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


static BOOLEAN twt_check_interval(
	UINT32 twt_mantissa,
	UINT32 twt_exp,
	UINT32 peer_mantissa,
	UINT32 peer_exp)
{
	UINT64 twt_interval = 0;
	UINT64 peer_interval = 0;

	twt_interval = twt_mantissa;
	twt_interval <<= twt_exp;

	peer_interval = peer_mantissa;
	peer_interval <<= peer_exp;

	return (twt_interval == peer_interval) ? TRUE : FALSE;
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
	if (IS_BTWT_ID(curr_twt_node)) {
		twt_agrt->persistence = curr_twt_node->persistence;
		twt_agrt->ntbtt_before_reject = NTBBT_BEFORE_REJECT;
	}

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
		twt_agrt->persistence = 0;
		twt_agrt->ntbtt_before_reject = 0;
	} else if (cmd_tsf_type == CMD_TSF_TYPE_BTWT) {
		twt_agrt->agrt_sp_start_tsf_low = (UINT32)(curr_twt_node->schedule_sp_start_tsf & 0xffffffff);
		twt_agrt->agrt_sp_start_tsf_high = (UINT32)(curr_twt_node->schedule_sp_start_tsf >> 32);
	}

	twt_agrt->is_role_ap = curr_twt_node->is_role_ap;
	twt_agrt->grp_member_cnt = curr_twt_node->grp_member_cnt;
	if (twt_agrt->grp_member_cnt) {
		for (i = 0; i < TWT_HW_GRP_MAX_MEMBER_CNT; i++)
			twt_agrt->sta_list[i] = curr_twt_node->sta_list[i];
	}

	MTWF_PRINT("agrt_tbl_idx(%d)\n", twt_agrt->agrt_tbl_idx);
	MTWF_PRINT("agrt_ctrl_flag(%d)\n", twt_agrt->agrt_ctrl_flag);
	MTWF_PRINT("own_mac_idx(%d)\n", twt_agrt->own_mac_idx);
	MTWF_PRINT("flow_id(%d)\n", twt_agrt->flow_id);
	MTWF_PRINT("peer_id_grp_id(0x%.4x)\n", twt_agrt->peer_id_grp_id);
	MTWF_PRINT("agrt_sp_duration(%d)\n", twt_agrt->agrt_sp_duration);
	MTWF_PRINT("bss_idx(%d)\n", twt_agrt->bss_idx);
	MTWF_PRINT("agrt_sp_start_tsf_low(0x%.8x)\n", twt_agrt->agrt_sp_start_tsf_low);
	MTWF_PRINT("agrt_sp_start_tsf_high(0x%.8x)\n", twt_agrt->agrt_sp_start_tsf_high);
	MTWF_PRINT("agrt_sp_wake_intvl_mantissa(%d)\n", twt_agrt->agrt_sp_wake_intvl_mantissa);
	MTWF_PRINT("agrt_sp_wake_intvl_exponent(%d)\n", twt_agrt->agrt_sp_wake_intvl_exponent);
	MTWF_PRINT("is_role_ap(%d)\n", twt_agrt->is_role_ap);
	MTWF_PRINT("agrt_para_bitmap(0x%x)\n",  twt_agrt->agrt_para_bitmap);
	MTWF_PRINT("grp_member_cnt(%d)\n",  twt_agrt->grp_member_cnt);
	MTWF_PRINT("sta_list[%d,%d,%d,%d,%d,%d,%d,%d]\n",
		twt_agrt->sta_list[0],
		twt_agrt->sta_list[1],
		twt_agrt->sta_list[2],
		twt_agrt->sta_list[3],
		twt_agrt->sta_list[4],
		twt_agrt->sta_list[5],
		twt_agrt->sta_list[6],
		twt_agrt->sta_list[7]);
	MTWF_PRINT("persistence(%d)\n", twt_agrt->persistence);
	MTWF_PRINT("ntbtt_before_reject(%d)\n", twt_agrt->ntbtt_before_reject);
}

static VOID twt_tbl_free(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN struct itwt_ie *twt_ie_in,
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

	UINT16 control = twt_ie_in->control;
	UINT8 twt_info_frame_dis = 0;
	UINT8 wake_dur_unit = 0;
	UINT16 req_type = twt_ie_in->req_type;
	UINT8 ie_steup_cmd = 0;
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
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	entry = &ad->MacTab.Content[wcid];
	if (twt_is_sp_duration_tolerance(ad, twt_ie_in) == FALSE) {
		*setup_cmd = TWT_SETUP_CMD_DICTATE;
		return;
	}

	*setup_cmd = TWT_SETUP_CMD_ACCEPT;
	ctrl = hc_get_hdev_ctrl(wdev);
	obj = wdev->pHObj;

	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	/* get peer twt control filed */
	twt_info_frame_dis = GET_TWT_CTRL_INFO_FRM_DIS(control);
	wake_dur_unit = GET_TWT_CTRL_WAKE_DUR_UNIT(control);

	/* get peer twt request type */
	ie_steup_cmd = GET_TWT_RT_SETUP_CMD(req_type);
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

	/* optimize twt parameters */
	twt_para_optimize(&mantissa, &exponent, &duration);

	/* fill twt agrt parameters */
	/* update entry */
	entry->twt_flow_id_bitmap |= (1 << flow_identifier);

	/* update node */
	twt_node = twt_ctrl_acquire_twt_node(ctrl, agrt_type);
	if (!twt_node) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" twt_ctrl_acquire_twt_node: fail!\n");
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
	twt_node->twt_info_frame_dis = twt_info_frame_dis;
	twt_node->twt_channel = channel;

	CLR_AGRT_PARA_BITMAP(twt_node);
	if (wake_dur_unit)
		SET_AGRT_PARA_BITMAP(twt_node, TWT_AGRT_PARA_BITMAP_WAKE_DUR_UINT);
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

	if (ie_steup_cmd == TWT_SETUP_CMD_REQUEST)
		twt_node->tsf_type = TSF_FROM_SETUP_CMD_REQUEST;
	else if (ie_steup_cmd == TWT_SETUP_CMD_SUGGEST)
		twt_node->tsf_type = TSF_FROM_SETUP_CMD_SUGGEST;
	else if (ie_steup_cmd == TWT_SETUP_CMD_DEMAND)
		twt_node->tsf_type = TSF_FROM_SETUP_CMD_DEMAND;
	else
		twt_node->tsf_type = TSF_FROM_SETUP_CMD_REQUEST;

	twt_link_insert_node(wdev, twt_node);

	twt_interval = ((UINT64)(twt_node->agrt_sp_wake_intvl_mantissa)) << twt_node->agrt_sp_wake_intvl_exponent;

	/* handle tsf */
	if ((ie_steup_cmd == TWT_SETUP_CMD_REQUEST) || (ie_steup_cmd == TWT_SETUP_CMD_SUGGEST)) {
		twt_get_current_tsf(wdev, current_tsf);
		twt_current_tsf = current_tsf[0] + (((UINT64)current_tsf[1]) << 32);
		temp = twt_current_tsf - twt_node->schedule_sp_start_tsf;
		twt_mod = mod_64bit(temp, twt_interval);
		twt_node->schedule_sp_start_tsf_abs = (twt_current_tsf + (twt_interval - twt_mod));
		twt_assigned_tsf = twt_node->schedule_sp_start_tsf_abs;
	} else if (ie_steup_cmd == TWT_SETUP_CMD_DEMAND) {
		twt_assigned_tsf = twt_node->agrt_sp_start_tsf;
	}

	if (entry->twt_interval_max < twt_interval) {
		UINT64 temp2;
		temp = twt_interval;
		/* Converting twt_interval from microseconds to seconds. */
		temp2 = mod_64bit(temp, 1000000);
		entry->twt_interval_max = temp;
	}

	os_move_mem(tsf, &twt_assigned_tsf, sizeof(twt_assigned_tsf));

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
	    "steup_cmd=%d,%s current_tsf(%llu) and assigned_tsf(%llu),wcid=%d,agrt_type=%d,tbl_i(%d)\n",
	    ie_steup_cmd,
	    (ie_steup_cmd == TWT_SETUP_CMD_DEMAND) ? "STA" : "AP",
	    twt_current_tsf,
	    twt_assigned_tsf,
	    twt_node->peer_id_grp_id,
	    agrt_type,
	    twt_node->agrt_tbl_idx);

	/* twt h/w control new*/
	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	twt_agrt_cmd_set(&twt_agrt,
		twt_node,
		TWT_AGRT_CTRL_ADD,
		(twt_node->tsf_type == TSF_FROM_SETUP_CMD_DEMAND) ? CMD_TSF_TYPE_REQUESTER : CMD_TST_TYPE_SCHEDULE);
	mt_asic_twt_agrt_update(wdev, twt_agrt);
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
}

static VOID twt_tbl_full(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN struct itwt_ie *twt_ie_in,
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
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
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

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
    /* twt tempo is exactly the same then ACCEPT */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				return;
			}
			twt_interval_old = ((UINT64)(curr_twt_node->agrt_sp_wake_intvl_mantissa)) << curr_twt_node->agrt_sp_wake_intvl_exponent;

			/* check if exactly the same twt tempo exist */
			if (twt_is_grpid(curr_twt_node->peer_id_grp_id) &&
				(curr_twt_node->grp_member_cnt != TWT_HW_GRP_MAX_MEMBER_CNT) &&
				(curr_twt_node->agrt_sp_duration == duration) &&
				(twt_interval_old == twt_interval_new) &&
				(curr_twt_node->agrt_para_bitmap == agrt_para_bitmap)) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					" twt tempo the same with tbl_i=%d \n", curr_twt_node->agrt_tbl_idx);
				the_same_twt_tempo = TRUE;
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

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
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				" sta_list[%d]=%d\n", i, twt_agrt.sta_list[i]);
		}

		mt_asic_twt_agrt_update(wdev, twt_agrt);

		return;
	}

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	/* twt tempo is similar check */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(curr_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
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
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

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
	IN struct itwt_ie *twt_ie)
{
	UINT8 len = 0;
	BOOLEAN implicit = 0;

	if (twt_ie->elem_id != IE_TWT) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" elem_id!=%d incorrect, please check\n",IE_TWT);
		return FALSE;
	}

	len = sizeof(struct itwt_ie) - 2;
	if (twt_ie->len != len) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"twt ie len!=%d incorrect, please check\n", len);
		return FALSE;
	}

	implicit = GET_TWT_RT_IMPLICIT_LAST(twt_ie->req_type);
	if (implicit != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"implicit!=TRUE incorrect, please check\n");
		return FALSE;
	}

	if (twt_ie->channel != 0) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" channel!=0 incorrect, please check\n");
		return FALSE;
	}
	if (!parse_twt_ie(twt_ie)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" twt ie check fail\n");
		return FALSE;
	}

	return TRUE;
}

/* peer agrt request invoked by assoc. request or action frame phase */
static VOID twt_agrt_request(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN struct itwt_ie *twt_ie_in,
	OUT BOOLEAN *tbl_free,
	OUT PUINT8 agrt_tbl_idx,
	OUT PUINT8 setup_cmd,
	OUT PUINT32 tsf,
	OUT PUINT8 flow_id)
{
	struct hdev_ctrl *ctrl = hc_get_hdev_ctrl(wdev);
	UINT8 i_tbl_free_cnt = 0;
	UINT8 g_tbl_free_cnt = 0;
	struct twt_link_node *found_twt_node = NULL;

	if (twt_itwt_agrt_exist(wdev, wcid, twt_ie_in, &found_twt_node)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" twt_agrt with wcid=%d, flow_id=%d exist, Accept\n",
			wcid,
			found_twt_node->flow_id);
		*tbl_free = TRUE;
		*setup_cmd = TWT_SETUP_CMD_ACCEPT;
		*flow_id = found_twt_node->flow_id;
		*agrt_tbl_idx = found_twt_node->agrt_tbl_idx;
		return;
	}
	twt_ctrl_get_free_twt_node_num(ctrl, &i_tbl_free_cnt, &g_tbl_free_cnt);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"i_tbl_free_cnt(%d),g_tbl_free_cnt(%d)\n", i_tbl_free_cnt, g_tbl_free_cnt);

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
	IN struct itwt_ie *twt_ie_in,
	OUT struct itwt_ie *twt_ie_out)
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
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	if (!twt_setup_frame_sanity(twt_ie_in))
		setup_cmd = TWT_SETUP_CMD_REJECT;

	/*** twt agrt request and come out setup_cmd (accept, reject, alternate, dictate) ***/
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
		if (!twt_node) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_node is NULL!\n");
			return;
		}
		trigger = GET_AGRT_PARA_BITMAP(twt_node, TWT_AGRT_PARA_BITMAP_IS_TRIGGER);
		implicit = TRUE;
		flow_type = GET_AGRT_PARA_BITMAP(twt_node, TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE);
		exponent = twt_node->agrt_sp_wake_intvl_exponent;
		protection = GET_AGRT_PARA_BITMAP(twt_node, TWT_AGRT_PARA_BITMAP_IS_PROTECT);
		duration = twt_node->agrt_sp_duration;
		mantissa = twt_node->agrt_sp_wake_intvl_mantissa;
		channel = 0;
	} else if (((setup_cmd == TWT_SETUP_CMD_DICTATE) && tbl_free)) {
		struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
		struct _RTMP_CHIP_CAP *pChipCap = NULL;
		if (!ad) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"ad is NULL!\n");
			return;
		}
		pChipCap = hc_get_chip_cap(ad->hdev_ctrl);
		trigger = GET_TWT_RT_TRIGGER(req_type);
		implicit = GET_TWT_RT_IMPLICIT_LAST(req_type);
		flow_type = GET_TWT_RT_FLOW_TYPE(req_type);
		exponent = GET_TWT_RT_WAKE_INTVAL_EXP(req_type);
		protection = GET_TWT_RT_PROTECTION(req_type);
		duration = pChipCap->twt_sp_duration_min_num;
		mantissa = twt_ie_in->mantissa;
		channel = twt_ie_in->channel;
	} else {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"incorrect, setup_cmd=%d, tbl_i=%d, tbl_free:%d, please check\n",
			setup_cmd, agrt_tbl_idx, tbl_free);
		return;
	}

	/* control */
	os_zero_mem(twt_ie_out, sizeof(struct itwt_ie));
	twt_ie_out->elem_id = IE_TWT;
	twt_ie_out->len = sizeof(struct itwt_ie) - 2;

	twt_ie_out->control &= ~TWT_CTRL_NDP_PAGING_INDICATOR;
	twt_ie_out->control &= ~TWT_CTRL_RESPONDER_PM_MODE;
	twt_ie_out->control |= SET_TWT_CTRL_NEGO_TYPE(GET_TWT_CTRL_NEGO_TYPE(twt_ie_in->control));
	/*twt_ie_out->control |= SET_TWT_CTRL_INFO_FRM_DIS(GET_TWT_CTRL_INFO_FRM_DIS(twt_ie_in->control));*/
	twt_ie_out->control |= SET_TWT_CTRL_INFO_FRM_DIS(twt_get_twt_info_frame_support(wdev) ? 0 : 1);
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

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		" reply wcid=%d, f_id=%d, setup_cmd=%d, tbl_i=%d, tbl_free=%d\n", wcid, flow_identifier, setup_cmd, agrt_tbl_idx, tbl_free);
}

static VOID twt_teardown_request(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN UINT8 twt_flow_id)
{
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
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	entry = &ad->MacTab.Content[wcid];

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
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
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					" twt_entry=NULL, please check\n");
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
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

	if (found) {
		if (entry->twt_flow_id_bitmap == 0)
			entry->twt_interval_max = 0;

		NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
		twt_agrt_cmd_set(&twt_agrt, curr_twt_node, TWT_AGRT_CTRL_DELETE, CMD_TSF_TYPE_NA);
		mt_asic_twt_agrt_update(wdev, twt_agrt);
		NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
		if (process_last_wcid) {
			twt_link_remove_node(wdev, curr_twt_node);
			twt_ctrl_release_twt_node(ctrl, curr_twt_node);
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" process=FALSE, wcid=%d,flow_id=%d\n",  wcid, twt_flow_id);
	}
}

static BOOLEAN twt_handle_one_peer_with_btwt_id_leave(
	IN struct wifi_dev *wdev,
	IN struct _MAC_TABLE_ENTRY *entry,
	IN UINT8 btwt_id,
	OUT struct twt_agrt_para *twt_agrt)
{
	UINT8 sch_link_idx = 0;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct _RTMP_ADAPTER *ad = NULL;
	UINT8 band = 0;
	BOOLEAN found = FALSE;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s: wdev=NULL, please check\n", __func__);
		return found;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	band = HcGetBandByWdev(wdev);

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	/*
	 * btwt_id = remove wcid belongs to btwt_id
	 * teardown_all_twt = remove wcid belongs to twt_id=0~31
	 */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"%s: twt_entry=NULL, please check\n", __func__);
				return found;
			}

			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band) &&
				(GET_BTWT_ID(curr_twt_node) == btwt_id) &&
				twt_check_btwt_member(curr_twt_node->sta_list, entry->wcid)) {
				twt_remove_btwt_member(curr_twt_node, entry->wcid);
				twt_agrt_cmd_set(twt_agrt, curr_twt_node, TWT_AGRT_CTRL_MODIFY, CMD_TSF_TYPE_BTWT);
				found = TRUE;
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

	return found;
}

static VOID twt_handle_all_peers_in_btwt_id_leave(
	IN struct wifi_dev *wdev,
	IN UINT8 btwt_id,
	OUT UINT16 *sta_list,
	OUT UINT8 *sta_cnt,
	OUT struct twt_agrt_para *twt_agrt)
{
	UINT8 sch_link_idx = 0;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct _RTMP_ADAPTER *ad = NULL;
	UINT8 band = 0;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	band = HcGetBandByWdev(wdev);

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	/*
	 * btwt_id = remove wcid belongs to btwt_id
	 * teardown_all_twt = remove wcid belongs to twt_id=0~31
	 */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					" twt_entry=NULL, please check\n");
				return;
			}

			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band) &&
				(GET_BTWT_ID(curr_twt_node) == btwt_id) &&
				(curr_twt_node->grp_member_cnt > 0)) {
				os_move_mem(sta_list, curr_twt_node->sta_list, sizeof(UINT16) * TWT_HW_GRP_MAX_MEMBER_CNT);
				os_zero_mem(curr_twt_node->sta_list, sizeof(UINT16) * TWT_HW_GRP_MAX_MEMBER_CNT);
				*sta_cnt = curr_twt_node->grp_member_cnt;
				curr_twt_node->grp_member_cnt = 0;
				twt_agrt_cmd_set(twt_agrt, curr_twt_node, TWT_AGRT_CTRL_MODIFY, CMD_TSF_TYPE_BTWT);
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
}

void twt_dump_btwt_setup_frame(
	struct frame_btwt_setup *btwt_frame_in,
	struct frame_btwt_setup *btwt_frame_out)
{
	struct btwt_para_set *btwt_para_in = NULL;
	struct btwt_para_set *btwt_para_out = NULL;

	if (!btwt_frame_in || !btwt_frame_out)
		return;

	btwt_para_in = &btwt_frame_in->btwt_para[0];
	btwt_para_out = &btwt_frame_out->btwt_para[0];

	if (!btwt_para_in || !btwt_para_out)
		return;

	/* in twt setup frame */
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"In(twt setup frame):\nA1=%pM,A2=%pM,A3=%pM\n",
		btwt_frame_in->hdr.Addr1, btwt_frame_in->hdr.Addr2, btwt_frame_in->hdr.Addr3);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"Cat(%d),S1G(%d),Token(%d),ElemID(%d),Len(%d)\n",
		btwt_frame_in->category,
		btwt_frame_in->s1g_action,
		btwt_frame_in->token,
		btwt_frame_in->elem_id,
		btwt_frame_in->len);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"NDP(%d),PM(%d),NT(%d),TWTInfoDis(%d),WakeDurUnit(%d)\n",
		(UINT8)GET_TWT_CTRL_NDP_PAGING_INDICATOR(btwt_frame_in->control),
		(UINT8)GET_TWT_CTRL_RESPONDER_PM_MODE(btwt_frame_in->control),
		(UINT8)GET_TWT_CTRL_NEGO_TYPE(btwt_frame_in->control),
		(UINT8)GET_TWT_CTRL_INFO_FRM_DIS(btwt_frame_in->control),
		(UINT8)GET_TWT_CTRL_WAKE_DUR_UNIT(btwt_frame_in->control));

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"TR(%d),SC(%d),T(%d),LBPS(%d),FT(%d),BTR(%d),TWIE(%d)\n",
		(UINT16)GET_TWT_RT_REQUEST(btwt_para_in->req_type),
		(UINT16)GET_TWT_RT_SETUP_CMD(btwt_para_in->req_type),
		(UINT16)GET_TWT_RT_TRIGGER(btwt_para_in->req_type),
		(UINT16)GET_TWT_RT_IMPLICIT_LAST(btwt_para_in->req_type),
		(UINT16)GET_TWT_RT_FLOW_TYPE(btwt_para_in->req_type),
		(UINT16)GET_TWT_RT_BTWT_REC(btwt_para_in->req_type),
		(UINT16)GET_TWT_RT_WAKE_INTVAL_EXP(btwt_para_in->req_type));
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"TWT(0x%x),Duration(%d),Mantissa(%d)\n",
		btwt_para_in->target_wake_time,
		btwt_para_in->duration,
		btwt_para_in->mantissa);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"bTWTId(%d),Persistence(%d)\n",
		(UINT16)GET_BTWT_INFO_BTWT_ID(btwt_para_in->btwt_info),
		(UINT16)GET_BTWT_INFO_BTWT_P(btwt_para_in->btwt_info));

	/* out twt setup frame */
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"Out(twt setup frame):\nA1=%pM,A2=%pM,A3=%pM\n",
		btwt_frame_out->hdr.Addr1, btwt_frame_out->hdr.Addr2, btwt_frame_out->hdr.Addr3);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"Cat(%d),S1G(%d),Token(%d),ElemID(%d),Len(%d)\n",
		btwt_frame_out->category,
		btwt_frame_out->s1g_action,
		btwt_frame_out->token,
		btwt_frame_out->elem_id,
		btwt_frame_out->len);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"NDP(%d),PM(%d),NT(%d),TWTInfoDis(%d),WakeDurUnit(%d)\n",
		(UINT8)GET_TWT_CTRL_NDP_PAGING_INDICATOR(btwt_frame_out->control),
		(UINT8)GET_TWT_CTRL_RESPONDER_PM_MODE(btwt_frame_out->control),
		(UINT8)GET_TWT_CTRL_NEGO_TYPE(btwt_frame_out->control),
		(UINT8)GET_TWT_CTRL_INFO_FRM_DIS(btwt_frame_out->control),
		(UINT8)GET_TWT_CTRL_WAKE_DUR_UNIT(btwt_frame_out->control));

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"TR(%d),SC(%d),T(%d),LBPS(%d),FT(%d),BTR(%d),TWIE(%d)\n",
		(UINT16)GET_TWT_RT_REQUEST(btwt_para_out->req_type),
		(UINT16)GET_TWT_RT_SETUP_CMD(btwt_para_out->req_type),
		(UINT16)GET_TWT_RT_TRIGGER(btwt_para_out->req_type),
		(UINT16)GET_TWT_RT_IMPLICIT_LAST(btwt_para_out->req_type),
		(UINT16)GET_TWT_RT_FLOW_TYPE(btwt_para_out->req_type),
		(UINT16)GET_TWT_RT_BTWT_REC(btwt_para_out->req_type),
		(UINT16)GET_TWT_RT_WAKE_INTVAL_EXP(btwt_para_out->req_type));
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"TWT(0x%x),Duration(%d),Mantissa(%d)\n",
		btwt_para_out->target_wake_time,
		btwt_para_out->duration,
		btwt_para_out->mantissa);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"bTWTId(%d),Persistence(%d)\n",
		(UINT16)GET_BTWT_INFO_BTWT_ID(btwt_para_out->btwt_info),
		(UINT16)GET_BTWT_INFO_BTWT_P(btwt_para_out->btwt_info));
}



void twt_dump_btwt_mlme_teardown_frame(
	struct frame_btwt_teardown *mlme_teardown_frame)
{
	if (!mlme_teardown_frame)
		return;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"mlme_teardown_frame:\nA1=%pM,A2=%pM,A3=%pM\n",
		mlme_teardown_frame->hdr.Addr1, mlme_teardown_frame->hdr.Addr2, mlme_teardown_frame->hdr.Addr3);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"Cat(%d),S1G(%d),bTWTID(%d),NT(%d),AllTWT(%d)\n",
		mlme_teardown_frame->category,
		mlme_teardown_frame->s1g_action,
		(UINT8)GET_BTWT_FLOW_BTWT_ID(mlme_teardown_frame->twt_flow),
		(UINT8)GET_BTWT_FLOW_NEGO_TYPE(mlme_teardown_frame->twt_flow),
		(UINT8)GET_BTWT_FLOW_TEARDOWN_ALL_TWT(mlme_teardown_frame->twt_flow));
}

/* TWT action frame state machine management (for peer STA role) */
VOID peer_twt_setup_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct wifi_dev *wdev = elem->wdev;
	UINT8 nego_type = TWT_CTRL_NEGO_TYPE_ITWT;
	struct frame_twt_setup *frame = (struct frame_twt_setup *)&elem->Msg;

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	nego_type = GET_TWT_CTRL_NEGO_TYPE(frame->control);

	if (nego_type == TWT_CTRL_NEGO_TYPE_ITWT) {
		PUCHAR out_buffer = NULL;
		ULONG frame_len = 0;
		struct frame_itwt_setup *frame_in = (struct frame_itwt_setup *)&elem->Msg;
		struct frame_itwt_setup frame_out;
		struct itwt_ie *twt_ie_in = &frame_in->twt_ie;
		struct itwt_ie *twt_ie_out = &frame_out.twt_ie;

#ifdef APCLI_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA) {
		struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
		UCHAR ucTWTFlowId;
		struct itwt_ie *prtwt_ie;

		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "-->\n");
		apcli_entry = &ad->StaCfg[wdev->func_idx];
		prtwt_ie = &(frame_in->twt_ie);

		if (!prtwt_ie) {
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" prtwt_ie == NULL!!!!\n");
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
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				" allocate memory failed, please check\n");
				return;
			}

			/* fill action frame content */
			os_zero_mem(&frame_out, sizeof(struct frame_itwt_setup));
			ActHeaderInit(ad, &frame_out.hdr, frame_in->hdr.Addr2, wdev->if_addr, wdev->bssid);
			frame_out.category = CATEGORY_S1G;
			frame_out.s1g_action = CATE_S1G_ACTION_TWT_SETUP;
			frame_out.token = frame_in->token;
			twt_build_twt_ie(wdev, elem->Wcid, twt_ie_in, twt_ie_out);

			/* send action frame to peer sta */
			MakeOutgoingFrame(out_buffer, &frame_len,
				sizeof(struct frame_itwt_setup), &frame_out,
				END_OF_ARGS);

			MiniportMMRequest(ad, QID_AC_BE, out_buffer, frame_len);

			os_free_mem(out_buffer);
		}
	} else if (nego_type == TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) {
		if (wdev->wdev_type == WDEV_TYPE_STA) {
		} else {
			struct frame_btwt_setup *btwt_frame_in = (struct frame_btwt_setup *)&elem->Msg;
			struct btwt_para_set *btwt_para_in = NULL;
			UINT8 btwt_element_num = 0;
			UINT8 btwt_frame_out_len = 0;
			PUCHAR out_buffer = NULL;
			struct frame_btwt_setup *btwt_frame_out = NULL;
			struct btwt_para_set *btwt_para_out = NULL;
			BOOLEAN last_element = TRUE;

			if (!TWT_SUPPORT_BTWT(wlan_config_get_he_twt_support(wdev))) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"wdev(type=%d,fun_idx=%d,wdev_idx=%d) not support btwt,return\n",
					wdev->wdev_type, wdev->func_idx, wdev->wdev_idx);
				return;
			}

			if (MlmeAllocateMemory(ad, &out_buffer) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"allocate memory failed, please check\n");
				return;
			}

			btwt_frame_out = (struct frame_btwt_setup *)out_buffer;
			btwt_para_out = &btwt_frame_out->btwt_para[0];
			btwt_para_in = &btwt_frame_in->btwt_para[0];

			/* Handle join request and fill action frame content */
			do {
				os_zero_mem(btwt_para_out, sizeof(struct btwt_para_set));
				btwt_para_out->req_type |= SET_TWT_RT_SETUP_CMD(TWT_SETUP_CMD_REJECT);
				twt_handle_peer_join_btwt_id(wdev, elem->Wcid, btwt_para_in, btwt_para_out);
				btwt_element_num++;
				last_element = GET_TWT_RT_IMPLICIT_LAST(btwt_para_in->req_type) ? TRUE : FALSE;
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					"last_element=%d,twt_setup_cmd=%d\n",
					last_element, (UINT16)GET_TWT_RT_SETUP_CMD(btwt_para_out->req_type));
				btwt_para_in++;
				btwt_para_out++;
			} while (!last_element);

			ActHeaderInit(ad, &btwt_frame_out->hdr, btwt_frame_in->hdr.Addr2, wdev->if_addr, wdev->bssid);
			btwt_frame_out->category = CATEGORY_S1G;
			btwt_frame_out->s1g_action = CATE_S1G_ACTION_TWT_SETUP;
			btwt_frame_out->token = btwt_frame_in->token;
			btwt_frame_out->elem_id = IE_TWT;
			btwt_frame_out->len = 1 + sizeof(struct btwt_para_set) * btwt_element_num; /* 1=control */
			btwt_frame_out->control = btwt_frame_in->control;
			btwt_frame_out_len = sizeof(struct frame_btwt_setup) +
				sizeof(struct btwt_para_set) * btwt_element_num; /* control is counted in frame_btwt_setup */
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"%s:btwt_frame_out_len=%d,btwt_element_num=%d\n",
				__func__, btwt_frame_out_len, btwt_element_num);

			twt_dump_btwt_setup_frame(btwt_frame_in, btwt_frame_out);

			/* Send action frame to peer sta */
			MiniportMMRequest(ad, QID_AC_BE, (PUCHAR)btwt_frame_out, btwt_frame_out_len);
			MlmeFreeMemory(out_buffer);
		}
	}
}

static VOID peer_twt_teardown_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct wifi_dev *wdev = elem->wdev;
	UINT16 wcid = elem->Wcid;
	UINT8 nego_type = TWT_CTRL_NEGO_TYPE_ITWT;
	struct frame_twt_teardown *frame = (struct frame_twt_teardown *)&elem->Msg;

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	nego_type = GET_TEARDWON_FRAME_NEGO_TYPE(frame->twt_flow);

	if (nego_type == TWT_CTRL_NEGO_TYPE_ITWT) {
		struct frame_itwt_teardown *frame_in = NULL;
		frame_in = (struct frame_itwt_teardown *)&elem->Msg;

		/* handle twt_entry, twt_link_entry, twt cmd-event */
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"wcid=%d,flow_id=%d\n", wcid, frame_in->twt_flow_id);

#ifdef APCLI_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_STA) {
			twtReqFsmRunEventRxTeardown(
				ad, wdev, frame_in->twt_flow_id);
		} else
#endif /* APCLI_SUPPORT */
		twt_teardown_request(wdev, wcid, frame_in->twt_flow_id);
	} else if (nego_type == TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) {
		struct frame_btwt_teardown *btwt_tardown_frame = (struct frame_btwt_teardown *)&elem->Msg;

		twt_handle_peer_leave_btwt_id(wdev,
			elem->Wcid,
			GET_BTWT_FLOW_BTWT_ID(btwt_tardown_frame->twt_flow));
	}
}

/* handle peer twt information frame request */
BOOLEAN twt_get_twt_info_frame_support(
	struct wifi_dev *wdev)
{
	return wlan_config_get_he_twt_info_frame(wdev) ? TRUE : FALSE;
}

VOID peer_twt_info_frame_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
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
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	if (!twt_get_twt_info_frame_support(wdev)) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"Not support\n");
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

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"wcid(%d),fid(%d),subfield_size(%d),all_twt(%d),next_twt(msb:0x%.8x,lsb:0x%.8x)\n",
		wcid, twt_flow_id, next_twt_subfield_size, all_twt, frame_in->next_twt[1], frame_in->next_twt[0]);

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	/* find the twt agrt with flow id */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL,please check\n");
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
				curr_twt_node->suspend = TRUE;
				CLR_AGRT_PARA_BITMAP(curr_twt_node);
				if (all_twt)
					SET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_ALL_TWT);

				if (next_twt_subfield_size) {
					/* suspend_resume */
					tsf_64 = tsf[1];
					tsf_64 = (tsf_64 << 32) + tsf[0];

					if (next_twt_subfield_size == 1) {
						tsf_64 = tsf_64 & 0x00000000ffffffff;
						SET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_NEXT_TWT_32_BITS);
					} else if (next_twt_subfield_size == 2) {
						tsf_64 = tsf_64 & 0x0000ffffffffffff;
						SET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_NEXT_TWT_48_BITS);
					} else if (next_twt_subfield_size == 3) {
						SET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_NEXT_TWT_64_BITS);
					}

					curr_twt_node->agrt_sp_info_tsf = tsf_64;
					agrt_ctrl_flag = TWT_AGRT_CTRL_SUSPEND_RESUME;
				} else {
					/* suspend */
					curr_twt_node->agrt_sp_info_tsf = 0;
					agrt_ctrl_flag = TWT_AGRT_CTRL_SUSPEND;
				}

				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					"found wcid=%d,fid=%d agrt with all_twt(%d)\n",
					wcid, twt_flow_id, all_twt);

				twt_agrt_cmd_set(&twt_agrt, curr_twt_node, agrt_ctrl_flag, CMD_TSF_TYPE_TWT_INFO);
				mt_asic_twt_agrt_update(wdev, twt_agrt);
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

	if (found_num == 0) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"fail to find wcid=%d,fid=%d agrt with all_twt(%d)\n",
			 wcid, twt_flow_id, all_twt);
	}
}

VOID twt_get_resume_event(
	IN struct wifi_dev *wdev,
	IN struct twt_resume_info *resume_info)
{
	struct _RTMP_ADAPTER *ad = NULL;

	if (!wdev)
		return;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);

	if (!ad)
		return;
	if (!resume_info)
		return;
	if (!VALID_UCAST_ENTRY_WCID(ad, resume_info->wcid))
		return;

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"bss=%d,wcid=%d,flow_id=%d,idle=%d\n",
		resume_info->bssinfo_idx,
		resume_info->wcid,
		resume_info->flow_id,
		resume_info->idle);

	MlmeEnqueueWithWdev(ad,
		ACTION_STATE_MACHINE,
		MT2_MLME_TWT_RESUME_INFO,
		sizeof(struct twt_resume_info),
		(PVOID)resume_info,
		0,
		wdev);
	RTMP_MLME_HANDLER(ad);
}

VOID mlme_twt_resume_info_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct wifi_dev *wdev = elem->wdev;
	UINT8 sch_link_idx = 0;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct twt_resume_info *resume_info = NULL;

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	if (!twt_get_twt_info_frame_support(wdev)) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"Not support\n");
		return;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;
	resume_info = (struct twt_resume_info *)&elem->Msg;

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	/* update suspend status */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL,please check\n");
				return;
			}

			if ((curr_twt_node->type == TWT_TYPE_INDIVIDUAL) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->bss_idx == resume_info->bssinfo_idx) &&
				(curr_twt_node->peer_id_grp_id == resume_info->wcid) &&
				(curr_twt_node->flow_id == resume_info->flow_id)) {

				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					"bss=%d,wcid=%d,flow_id=%d,suspend=%d->0\n",
					resume_info->bssinfo_idx,
					resume_info->wcid,
					resume_info->flow_id,
					curr_twt_node->suspend);

				curr_twt_node->suspend = FALSE;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
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
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			" Non-HE STA, MaxCap=%s reject\n",
			get_phymode_str(entry->MaxHTPhyMode.field.MODE));
		return;
	}

	if (htc_len) {
		NdisMoveMemory((void *)(elem->Msg+LENGTH_802_11),
			(void *)(elem->Msg+LENGTH_802_11+htc_len),
			(elem->MsgLen-htc_len));
	}

	/* Get S1G Action */
	action = elem->Msg[LENGTH_802_11 + 1];
	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		" get twt action=%d\n", action);

	switch (action) {
	case CATE_S1G_ACTION_TWT_SETUP:
		peer_twt_setup_action(ad, elem);
		break;

	case CATE_S1G_ACTION_TWT_TEARDOWN:
		peer_twt_teardown_action(ad, elem);
		break;

	case CATE_S1G_ACTION_TWT_INFO:
		peer_twt_info_frame_action(ad, elem);
		break;
	default:
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" unexpected action=%d, please check\n", action);
		break;
	}
}

/* TWT action frame trigger (for AP role) */
VOID twt_teardown_itwt(
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
	msg.nego_type = TWT_CTRL_NEGO_TYPE_ITWT;
	msg.wcid = wcid;
	os_move_mem(&msg.peer_addr[0], entry->Addr, MAC_ADDR_LEN);
	msg.twt_flow_id = twt_flow_id;

	/* enqueue message */
	MlmeEnqueueWithWdev(ad,
		ACTION_STATE_MACHINE,
		MT2_MLME_TWT_TEARDOWN_TWT,
		sizeof(struct mlme_twt_tear_down_req_struct),
		(PVOID)&msg,
		0,
		wdev);
	RTMP_MLME_HANDLER(ad);

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"wcid(%d),flow_id(%d)\n",
		wcid,
		twt_flow_id);
}

static VOID twt_remove_btwt_ie(
	struct wifi_dev *wdev,
	UINT8 btwt_id)
{
	UINT8 sch_link_idx = 0;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct _RTMP_ADAPTER *ad = NULL;
	UINT8 band = 0;
	BOOLEAN found = FALSE;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s: wdev=NULL, please check\n", __func__);
		return;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	band = HcGetBandByWdev(wdev);

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"%s: twt_entry=NULL, please check\n", __func__);
				return;
			}

			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band) &&
				(GET_BTWT_ID(curr_twt_node) == btwt_id)) {
				curr_twt_node->present = FALSE;
				found = TRUE;
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

	if (found)
		UpdateBeaconHandler(ad, wdev, BCN_UPDATE_BTWT_IE);
}

VOID mlme_twt_teradown_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct mlme_twt_tear_down_req_struct *msg = NULL;
	struct frame_twt_teardown frame_out;
	struct wifi_dev *wdev = NULL;
	PUCHAR out_buffer = NULL;
	ULONG frame_len = 0;
	UINT8 nego_type = 0;
	UINT8 i = 0, j = 0;
	UINT16 wcid = 0;
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;

	/* get an unused nonpaged memory */
	status = os_alloc_mem(ad, &out_buffer, MAX_MGMT_PKT_LEN);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"allocate memory failed, please check\n");
		return;
	}

	msg = (struct mlme_twt_tear_down_req_struct *)&elem->Msg;
	wdev = msg->wdev;
	nego_type = msg->nego_type;

	if (nego_type == TWT_CTRL_NEGO_TYPE_ITWT) {
		struct frame_itwt_teardown *itwt_frame = (struct frame_itwt_teardown *)&frame_out;
		/* handle twt_node, twt_link_lsit, twt cmd-event */
		twt_teardown_request(wdev, msg->wcid, msg->twt_flow_id);

		/* send action frame to peer sta */
		os_zero_mem(itwt_frame, sizeof(struct frame_itwt_teardown));
		ActHeaderInit(ad, &itwt_frame->hdr, msg->peer_addr, wdev->if_addr, wdev->bssid);
		itwt_frame->category = CATEGORY_S1G;
		itwt_frame->s1g_action = CATE_S1G_ACTION_TWT_TEARDOWN;
		itwt_frame->twt_flow_id = msg->twt_flow_id;

		MakeOutgoingFrame(out_buffer, &frame_len,
			sizeof(struct frame_itwt_teardown), itwt_frame,
			END_OF_ARGS);

		MiniportMMRequest(ad,
			(MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[QID_AC_VO]),
			out_buffer, frame_len);

		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"wcid(%d), flow_id(%d)\n",
			msg->wcid,
			msg->twt_flow_id);
	} else if (nego_type == TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) {
		struct frame_btwt_teardown *btwt_frame = (struct frame_btwt_teardown *)&frame_out;
		UINT16 sta_list[TWT_HW_GRP_MAX_MEMBER_CNT] = {0};
		UINT8 sta_cnt = 0;
		UINT8 btwt_id = msg->btwt_id;
		BOOLEAN teardown_all_twt = msg->teardown_all_twt;
		struct _MAC_TABLE_ENTRY *entry = NULL;
		struct twt_agrt_para twt_agrt = {0};
		struct wifi_dev_ops *ops;
		BOOLEAN action_frame_send = FALSE;

		if (!MAC_ADDR_EQUAL(msg->peer_addr, ZERO_MAC_ADDR)) {
			ops = wdev->wdev_ops;
			ops->mac_entry_lookup(ad, msg->peer_addr, wdev, &entry);

			if (!entry) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"%s: Fail to find pEntry with %pM, return\n",
					__func__, msg->peer_addr);
				os_free_mem(out_buffer);
				return;
			}

			/* teardown with action frame only btwt_id > 0 */
			for (i = 1; i < TWT_BTWT_ID_NUM; i++) {
				if (!teardown_all_twt && btwt_id != i)
					continue;

				/* send action frame to peer sta */
				if (!VALID_UCAST_ENTRY_WCID(ad, entry->wcid))
					continue;

				if (!GET_PEER_JOIN_BTWT_ID(entry, i))
					continue;

				SET_PEER_LEAVE_BTWT_ID(entry, i);

				if (!action_frame_send) {
					os_zero_mem(btwt_frame, sizeof(struct frame_btwt_teardown));
					ActHeaderInit(ad, &btwt_frame->hdr, entry->Addr, wdev->if_addr, wdev->bssid);
					btwt_frame->category = CATEGORY_S1G;
					btwt_frame->s1g_action = CATE_S1G_ACTION_TWT_TEARDOWN;
					btwt_frame->twt_flow |= SET_BTWT_FLOW_BTWT_ID(teardown_all_twt ? 0 : i) |
						SET_BTWT_FLOW_NEGO_TYPE(TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) |
						SET_BTWT_FLOW_TEARDOWN_ALL_TWT(teardown_all_twt);

					MakeOutgoingFrame(out_buffer, &frame_len,
						sizeof(struct frame_btwt_teardown), btwt_frame,
						END_OF_ARGS);

					MiniportMMRequest(ad,
						(MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[QID_AC_VO]),
						out_buffer, frame_len);

					twt_dump_btwt_mlme_teardown_frame((struct frame_btwt_teardown *)out_buffer);

					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"%s(RA):send teardown action frame btwt_id=%d,teardown_all_twt=%d\n",
						__func__, i, teardown_all_twt);
					action_frame_send = TRUE;
				}

				/* twt cmd-event(modify) */
				if (twt_handle_one_peer_with_btwt_id_leave(wdev, entry, i, &twt_agrt))
					mt_asic_twt_agrt_update(wdev, twt_agrt);
			}
		} else {
			/* teardown with action frame only btwt_id > 0 */
			for (i = 1; i < TWT_BTWT_ID_NUM; i++) {
				/* AP teardown specific btwt_id (not btwt_id_0) */
				if (!teardown_all_twt && btwt_id != i)
					continue;

				os_zero_mem(sta_list, sizeof(sta_list));
				os_zero_mem(&twt_agrt, sizeof(struct twt_agrt_para));
				sta_cnt = 0;

				/* handle twt_node, twt_link_lsit */
				twt_handle_all_peers_in_btwt_id_leave(wdev, i, sta_list, &sta_cnt, &twt_agrt);

				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					"%s:btwt_id=%d,teardown_all_twt=%d,sta_cnt=%d,sta_list[%d,%d,%d,%d,%d,%d,%d,%d],sts=%s\n",
					__func__, i, teardown_all_twt, sta_cnt,
					sta_list[0], sta_list[1], sta_list[2], sta_list[3], sta_list[4], sta_list[5], sta_list[6], sta_list[7],
					sta_cnt ? "DONE" : "NA");

				if (sta_cnt > 0) {
					/* send action frame to peer sta */
					for (j = 0; j < sta_cnt; j++) {
						wcid = sta_list[j];

						if (!VALID_UCAST_ENTRY_WCID(ad, wcid))
							continue;

						entry = &ad->MacTab.Content[wcid];
						if (!GET_PEER_JOIN_BTWT_ID(entry, i))
							continue;

						SET_PEER_LEAVE_BTWT_ID(entry, btwt_id);
						if (teardown_all_twt)
							SET_PEER_LEAVE_ALL_BTWT(entry);

						os_zero_mem(btwt_frame, sizeof(struct frame_btwt_teardown));
						ActHeaderInit(ad, &btwt_frame->hdr, entry->Addr, wdev->if_addr, wdev->bssid);
						btwt_frame->category = CATEGORY_S1G;
						btwt_frame->s1g_action = CATE_S1G_ACTION_TWT_TEARDOWN;
						btwt_frame->twt_flow |= SET_BTWT_FLOW_BTWT_ID(i) |
							SET_BTWT_FLOW_NEGO_TYPE(TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) |
							SET_BTWT_FLOW_TEARDOWN_ALL_TWT(teardown_all_twt);

						MakeOutgoingFrame(out_buffer, &frame_len,
							sizeof(struct frame_btwt_teardown), btwt_frame,
							END_OF_ARGS);

						MiniportMMRequest(ad,
							(MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[QID_AC_VO]),
							out_buffer, frame_len);

						twt_dump_btwt_mlme_teardown_frame((struct frame_btwt_teardown *)out_buffer);

						MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
							"%s(ID):send teardown action frame btwt_id=%d,wcid=%d,teardown_all_twt=%d\n",
							__func__, i, wcid, teardown_all_twt);
					}
					/* twt cmd-event(modify) */
					mt_asic_twt_agrt_update(wdev, twt_agrt);
				}
			}
		}
	}

	os_free_mem(out_buffer);
}

/* AP wants to exclude btwt_id ie in BCN/ProbeRsp */
VOID twt_remove_btwt_resouce(
	struct wifi_dev *wdev,
	UINT8 btwt_id)
{
	struct frame_twt_teardown frame_out;
	struct frame_btwt_teardown *btwt_frame = (struct frame_btwt_teardown *)&frame_out;
	PUCHAR out_buffer = NULL;
	ULONG frame_len = 0;
	UINT8 i = btwt_id, j = 0;
	UINT16 wcid = 0;
	UINT16 sta_list[TWT_HW_GRP_MAX_MEMBER_CNT] = {0};
	UINT8 sta_cnt = 0;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct twt_agrt_para twt_agrt = {0};
	NDIS_STATUS status = NDIS_STATUS_FAILURE;

	/* get an unused nonpaged memory */
	status = os_alloc_mem(ad, &out_buffer, MAX_MGMT_PKT_LEN);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s: allocate memory failed, please check\n", __func__);
		return;
	}

	os_zero_mem(sta_list, sizeof(sta_list));
	os_zero_mem(&twt_agrt, sizeof(struct twt_agrt_para));
	sta_cnt = 0;

	/* handle twt_node, twt_link_lsit */
	twt_handle_all_peers_in_btwt_id_leave(wdev, i, sta_list, &sta_cnt, &twt_agrt);

	/* remove btwt ie from BCN/ProbeRsp */
	twt_remove_btwt_ie(wdev, i);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"%s:btwt_id=%d,sta_cnt=%d,sta_list[%d,%d,%d,%d,%d,%d,%d,%d],sts=%s\n",
		__func__, i, sta_cnt,
		sta_list[0], sta_list[1], sta_list[2], sta_list[3], sta_list[4], sta_list[5], sta_list[6], sta_list[7],
		sta_cnt ? "DONE" : "NA");

	/* only btwt_id > 0 apply teadrdown action frame */
	if ((sta_cnt > 0) && (i > 0)) {
		/* send action frame to peer sta */
		for (j = 0; j < sta_cnt; j++) {
			wcid = sta_list[j];

			if (!VALID_UCAST_ENTRY_WCID(ad, wcid))
				continue;

			entry = &ad->MacTab.Content[wcid];
			if (!GET_PEER_JOIN_BTWT_ID(entry, i))
				continue;

			SET_PEER_LEAVE_BTWT_ID(entry, btwt_id);

			os_zero_mem(btwt_frame, sizeof(struct frame_btwt_teardown));
			ActHeaderInit(ad, &btwt_frame->hdr, entry->Addr, wdev->if_addr, wdev->bssid);
			btwt_frame->category = CATEGORY_S1G;
			btwt_frame->s1g_action = CATE_S1G_ACTION_TWT_TEARDOWN;
			btwt_frame->twt_flow |= SET_BTWT_FLOW_BTWT_ID(i) |
				SET_BTWT_FLOW_NEGO_TYPE(TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) |
				SET_BTWT_FLOW_TEARDOWN_ALL_TWT(0);

			MakeOutgoingFrame(out_buffer, &frame_len,
				sizeof(struct frame_btwt_teardown), btwt_frame,
				END_OF_ARGS);

			MiniportMMRequest(ad,
				(MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[QID_AC_VO]),
				out_buffer, frame_len);

			twt_dump_btwt_mlme_teardown_frame((struct frame_btwt_teardown *)out_buffer);

			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"%s:OK to remove wcid=%d from btwt_id=%d\n",
				__func__, wcid, i);
		}
	}
	/* twt cmd-event(modify) */
	if (sta_cnt > 0)
		mt_asic_twt_agrt_update(wdev, twt_agrt);

	/* remove btwt node in linklist and twt cmd-event(del) */
	twt_release_btwt_node(wdev, i);

	os_free_mem(out_buffer);
}

/* Peer STA link down twt management */
VOID twt_resource_release_at_link_down(
	IN struct _RTMP_ADAPTER *ad,
	IN UINT16 wcid)
{
	struct wifi_dev *wdev = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	UINT8 i = 0;

	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" ad=NULL, please check\n");
		return;
	}

	entry = &ad->MacTab.Content[wcid];
	if (!entry || IS_ENTRY_NONE(entry))
		return;

	wdev = entry->wdev;
	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}
	/* handle twt_node, twt_link_list, twt cmd-event */
	if (entry->twt_flow_id_bitmap != 0) {
		for (i = 0; i < TWT_FLOW_ID_MAX_NUM; i++) {
			if (entry->twt_flow_id_bitmap & (1 << i)) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					" wcid(%d), flow_id(%d)\n", wcid, i);
				twt_teardown_request(wdev, wcid, i);
			}
		}
	}

	if (entry->twt_btwt_id_bitmap != 0) {
		for (i = 0; i < TWT_BTWT_ID_NUM; i++) {
			if (entry->twt_btwt_id_bitmap & (1 << i)) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					" wcid(%d) in btwt_id(%d)\n",
					 wcid, i);
				twt_handle_peer_leave_btwt_id(wdev, wcid, i);
			}
		}
	}
}

VOID twt_dump_resource(
	IN struct wifi_dev *wdev)
{
	struct hdev_ctrl *ctrl = NULL;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	twt_ctrl_resource_status_dump(ctrl);
}

VOID twt_get_current_tsf(
	struct wifi_dev *wdev,
	PUINT32 current_tsf)
{
	if (HW_GET_TSF(wdev, current_tsf) != NDIS_STATUS_SUCCESS)
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s: fail\n", __func__);
}

/* bTWT*/
VOID mlme_twt_handle_btwt_join_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem)
{
	struct mlme_twt_join_btwt_req_struct *msg = NULL;
	struct wifi_dev *wdev = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	UINT8 sch_link_idx = 0;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct twt_agrt_para twt_agrt = {0};
	UINT8 band = 0;

	msg = (struct mlme_twt_join_btwt_req_struct *)&elem->Msg;
	wdev = msg->wdev;

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	if (!ad)
		return;

	if (!VALID_UCAST_ENTRY_WCID(ad, msg->wcid))
		return;

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;
	band = HcGetBandByWdev(wdev);
	entry = &ad->MacTab.Content[msg->wcid];

	/* i have btwt_id=0 and let peer STA with btwt cap. join btwt id=0 */
	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					" twt_entry=NULL, please check\n");
				return;
			}
			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band)) {
				if (GET_BTWT_ID(curr_twt_node) == msg->btwt_id) {
					if (twt_add_btwt_member(curr_twt_node, msg->wcid)) {
						SET_PEER_JOIN_BTWT_ID(entry, msg->btwt_id);
						twt_agrt_cmd_set(&twt_agrt,
							curr_twt_node,
							TWT_AGRT_CTRL_MODIFY,
							CMD_TSF_TYPE_BTWT);

						mt_asic_twt_agrt_update(wdev, twt_agrt);

						MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
							"OK to add wcid=%d to btwt_id=%d\n",
							msg->wcid, (UINT16)GET_BTWT_ID(curr_twt_node));
					}

					break;
				}
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
}

VOID twt_peer_join_btwt_id_0(
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *enrty)
{
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *ad = NULL;
	BTWT_BUF_STRUCT *btwt = NULL;
	UINT8 band = 0;
	struct mlme_twt_join_btwt_req_struct msg = {0};

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (!ad)
		return;

	if (!VALID_UCAST_ENTRY_WCID(ad, enrty->wcid))
		return;

	if (wlan_config_get_asic_twt_caps(wdev) &&
		TWT_SUPPORT_BTWT(wlan_config_get_he_twt_support(wdev)) &&
		IS_STA_SUPPORT_BTWT(enrty)) {
		band = HcGetBandByWdev(wdev);
		btwt = &ad->ApCfg.btwt[band];

		if (btwt->support_btwt_id_0) {
			msg.wdev = wdev;
			msg.btwt_id = 0;
			msg.wcid = enrty->wcid;
			MlmeEnqueueWithWdev(ad,
				ACTION_STATE_MACHINE,
				MT2_MLME_TWT_JOIN_BTWT,
				sizeof(struct mlme_twt_join_btwt_req_struct),
				(PVOID)&msg,
				0,
				wdev);
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}

VOID twt_tardown_btwt(
	struct wifi_dev *wdev,
	UCHAR *peer_addr,
	UINT8 nego_type,
	UINT8 btwt_id,
	BOOLEAN teardown_all_twt)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct mlme_twt_tear_down_req_struct msg = {0};

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (!ad)
		return;

	if (nego_type == TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT) {
		if (wlan_config_get_asic_twt_caps(wdev) &&
			TWT_SUPPORT_BTWT(wlan_config_get_he_twt_support(wdev))) {
			msg.wdev = wdev;
			if (peer_addr != NULL)
				COPY_MAC_ADDR(&msg.peer_addr, peer_addr);
			msg.nego_type = nego_type;
			msg.btwt_id = btwt_id;
			msg.teardown_all_twt = teardown_all_twt;
			MlmeEnqueueWithWdev(ad,
				ACTION_STATE_MACHINE,
				MT2_MLME_TWT_TEARDOWN_TWT,
				sizeof(struct mlme_twt_tear_down_req_struct),
				(PVOID)&msg,
				0,
				wdev);
		}
	}
}

static VOID twt_build_btwt_ie(
	struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = NULL;
	UINT8 sch_link_idx = 0;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	UINT8 btwt_element_num = 0;
	struct btwt_ie *btwt_element = NULL;
	struct btwt_para_set *btwt_para = NULL;
	UINT8 band = 0;
	BTWT_BUF_STRUCT *btwt = NULL;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s: wdev=NULL, please check\n", __func__);
		return;
	}

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (!ad) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s: ad=NULL, please check\n", __func__);
		return;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	NdisAcquireSpinLock(&ad->ApCfg.btwt_ie_lock);
	band = HcGetBandByWdev(wdev);
	btwt = &ad->ApCfg.btwt[band];
	os_zero_mem(btwt, sizeof(BTWT_BUF_STRUCT));
	btwt_element = (struct btwt_ie *)&btwt->btwt_element;

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"%s: twt_entry=NULL, please check\n", __func__);
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				NdisReleaseSpinLock(&ad->ApCfg.btwt_ie_lock);
				return;
			}

			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band) &&
				curr_twt_node->present) {
				btwt_para = &btwt_element->btwt_para[btwt_element_num];
				/* request type */
				btwt_para->req_type |= SET_TWT_RT_REQUEST(0) |
					SET_TWT_RT_SETUP_CMD(curr_twt_node->twt_setup_cmd) |
					SET_TWT_RT_TRIGGER(GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_TRIGGER)) |
					SET_TWT_RT_IMPLICIT_LAST(0) |
					SET_TWT_RT_FLOW_TYPE(GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE) ? 0 : 1) |
					SET_TWT_RT_BTWT_REC(curr_twt_node->btwt_recommendation) |
					SET_TWT_RT_WAKE_INTVAL_EXP(curr_twt_node->agrt_sp_wake_intvl_exponent) |
					SET_TWT_RT_PROTECTION(GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_PROTECT));
				/* target wake  time: unit=TU */
				btwt_para->target_wake_time = (UINT16)(curr_twt_node->schedule_sp_start_tsf >> 10);
				btwt->schedule_sp_start_tsf[btwt_element_num] = (UINT32)(curr_twt_node->schedule_sp_start_tsf);
				/* nominal minimum twt wake duration */
				btwt_para->duration = curr_twt_node->agrt_sp_duration;
				/* twt wake inetrval mantissa */
				btwt_para->mantissa = curr_twt_node->agrt_sp_wake_intvl_mantissa;
				/* broadcast twt info */
				btwt_para->btwt_info |= SET_BTWT_INFO_BTWT_ID(GET_BTWT_ID(curr_twt_node)) |
					SET_BTWT_INFO_BTWT_P(curr_twt_node->persistence);

				if (GET_BTWT_ID(curr_twt_node) == 0)
					btwt->support_btwt_id_0 = TRUE;

				btwt_element_num++;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

	if (btwt_element_num != 0) {
		btwt_element->elem_id = IE_TWT;
		/*1=control*/
		btwt_element->len = 1 + sizeof(struct btwt_para_set) * btwt_element_num;
		/* control */
		btwt_element->control |=  SET_TWT_CTRL_NEGO_TYPE(TWT_CTRL_NEGO_TYPE_BTWT_ANNOUNCE) |
		SET_TWT_CTRL_INFO_FRM_DIS(curr_twt_node->twt_info_frame_dis) |
		SET_TWT_CTRL_WAKE_DUR_UNIT(curr_twt_node->wake_dur_unit);
		/* assign last element bit */
		btwt_para = &btwt_element->btwt_para[btwt_element_num - 1];
		btwt_para->req_type |= SET_TWT_RT_IMPLICIT_LAST(1);

		btwt->btwt_element_exist = TRUE;
		btwt->btwt_element_num = btwt_element_num;
		btwt->btwt_bcn_offset = 0;
		btwt->btwt_probe_rsp_offset = 0;
	}

	NdisReleaseSpinLock(&ad->ApCfg.btwt_ie_lock);
}

VOID twt_update_btwt_twt(
	struct wifi_dev *wdev,
	PUINT32 current_tsf)
{
	struct _RTMP_ADAPTER *ad = NULL;
	BTWT_BUF_STRUCT *btwt = NULL;
	struct btwt_ie *btwt_element = NULL;
	UINT8 band = 0;
	UINT8 i = 0;
	struct btwt_para_set *btwt_para = NULL;
	UINT64 twt_current_tsf = 0;
	UINT16 mantissa = 0;
	UINT8 exponent = 0;
	UINT64 twt_interval = 0;
	UINT64 twt_mod = 0;
	UINT64 temp = 0;
	UINT64 future_target_wake_time = 0;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s: wdev=NULL, error\n", __func__);
		return;
	}

	if (current_tsf[0] == 0 && current_tsf[1] == 0) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s: tsf=0, error\n", __func__);
		return;
	}

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	band = HcGetBandByWdev(wdev);
	btwt = &ad->ApCfg.btwt[band];
	btwt_element = (struct btwt_ie *)&btwt->btwt_element;
	twt_current_tsf = current_tsf[0] + (((UINT64)current_tsf[1]) << 32);

	for (i = 0; i < btwt->btwt_element_num; i++) {
		btwt_para = &btwt_element->btwt_para[i];
		mantissa = btwt_para->mantissa;
		exponent = (UINT8)GET_TWT_RT_WAKE_INTVAL_EXP(btwt_para->req_type);
		twt_interval = ((UINT64)mantissa) << exponent;
		temp = twt_current_tsf - btwt->schedule_sp_start_tsf[i];
		twt_mod = mod_64bit(temp, twt_interval);
		future_target_wake_time = twt_current_tsf + (twt_interval - twt_mod);
		btwt_para->target_wake_time = (UINT16)((future_target_wake_time >> 10) & 0xffff);
	}
}

VOID twt_acquire_btwt_node(
	struct wifi_dev *wdev,
	struct twt_ctrl_btwt *btwt_ctrl_para)
{
	UINT8 sch_link_idx = 0;
	BOOLEAN found = FALSE;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct twt_agrt_para twt_agrt = {0};
	struct twt_link_node *twt_node = NULL;
	struct _RTMP_ADAPTER *ad = NULL;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);

	if (!TWT_SUPPORT_BTWT(wlan_config_get_he_twt_support(wdev))) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev(type=%d,fun_idx=%d,wdev_idx=%d) not support btwt,return\n",
			wdev->wdev_type, wdev->func_idx, wdev->wdev_idx);
		return;
	}

	if (wdev->btwt_id & (1 << btwt_ctrl_para->btwt_id)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s:btwt_id=%d existed, please release it brefore new acquire\n",
			__func__, btwt_ctrl_para->btwt_id);
		return;
	}

	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	/* reject duplicate band/btwt_id request */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				return;
			}

			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == btwt_ctrl_para->band) &&
				(GET_BTWT_ID(curr_twt_node) == btwt_ctrl_para->btwt_id)) {
				found = TRUE;
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

	if (!found) {
		twt_node = twt_ctrl_acquire_twt_node(ctrl, TWT_TYPE_BTWT);
		if (twt_node) {
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"%s:Success.Done added btwt element\n", __func__);
			twt_node->band = btwt_ctrl_para->band;
			SET_BTWT_ID(twt_node, btwt_ctrl_para->btwt_id);
			twt_node->agrt_sp_duration = btwt_ctrl_para->agrt_sp_duration;
			twt_node->agrt_sp_wake_intvl_mantissa = btwt_ctrl_para->agrt_sp_wake_intvl_mantissa;
			twt_node->agrt_sp_wake_intvl_exponent = btwt_ctrl_para->agrt_sp_wake_intvl_exponent;
			twt_node->is_role_ap = TWT_ROLE_AP;
			twt_node->agrt_para_bitmap = btwt_ctrl_para->agrt_para_bitmap;
			twt_node->twt_setup_cmd	= btwt_ctrl_para->twt_setup_cmd;
			twt_node->persistence = btwt_ctrl_para->persistence;
			twt_node->twt_info_frame_dis = btwt_ctrl_para->twt_info_frame_dis;
			twt_node->wake_dur_unit = btwt_ctrl_para->wake_dur_unit;
			twt_node->btwt_recommendation = btwt_ctrl_para->btwt_recommendation;
			twt_node->present = TRUE;

			twt_link_insert_node(wdev, twt_node);
			NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
			twt_agrt_cmd_set(&twt_agrt,
				twt_node,
				TWT_AGRT_CTRL_ADD,
				CMD_TSF_TYPE_BTWT);

			mt_asic_twt_agrt_update(wdev, twt_agrt);
			NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
			wdev->btwt_id |= (1 << btwt_ctrl_para->btwt_id);
		} else {
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"Fail.No free btwt resource available!\n");
		}
	}

	/* build btwt element for bcn/probe_rsp */
	twt_build_btwt_ie(wdev);
}

VOID twt_release_btwt_node(
	struct wifi_dev *wdev,
	UINT8 btwt_id)
{
	UINT8 sch_link_idx = 0;
	UINT8 band = 0;
	BOOLEAN found = FALSE;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct twt_agrt_para twt_agrt = {0};

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return;
	}

	if (!TWT_SUPPORT_BTWT(wlan_config_get_he_twt_support(wdev))) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev(type=%d,fun_idx=%d,wdev_idx=%d) not support btwt,return\n",
			wdev->wdev_type, wdev->func_idx, wdev->wdev_idx);
		return;
	}

	if (wdev->btwt_id == 0) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR, "%s:wdev not support btwt and return\n", __func__);
		return;
	}

	band = HcGetBandByWdev(wdev);
	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;

	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	/* find twt database for band+btwt_id */
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					" twt_entry=NULL, please check\n");
				return;
			}

			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band) &&
				(GET_BTWT_ID(curr_twt_node) == btwt_id)) {
				found = TRUE;
				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

	if (found) {
		wdev->btwt_id &= ~(1 << btwt_id);
		NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
		twt_agrt_cmd_set(&twt_agrt, curr_twt_node, TWT_AGRT_CTRL_DELETE, CMD_TSF_TYPE_NA);

		mt_asic_twt_agrt_update(wdev, twt_agrt);
		NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);

		twt_link_remove_node(wdev, curr_twt_node);
		twt_ctrl_release_twt_node(ctrl, curr_twt_node);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"%s:Success.Found band=%d,btwt_id=%d and release btwt resource\n",
			__func__, band, btwt_id);
	} else {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"%s:Fail.Failed to find band=%d,btwt_id=%d\n",
			__func__, band, btwt_id);
	}

	/* build btwt element for bcn/probe_rsp */
	twt_build_btwt_ie(wdev);
}


VOID twt_release_btwt_resource(
	struct wifi_dev *wdev)
{
	UINT8 i = 0;

	if (!wdev)
		return;

	if (!TWT_SUPPORT_BTWT(wlan_config_get_he_twt_support(wdev)))
		return;

	if (wdev->btwt_id == 0)
		return;

	for (i = 0; i < TWT_BTWT_ID_NUM; i++) {
		if (wdev->btwt_id & (1 << i))
			twt_remove_btwt_resouce(wdev, i);
	}
}

UINT8 twt_get_btwt_element_num(
	struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = NULL;
	BTWT_BUF_STRUCT *btwt = NULL;
	UINT8 btwt_element_num = 0;
	UINT8 band = 0;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"%s: wdev=NULL, please check\n", __func__);
		return 0;
	}

	if (wlan_config_get_asic_twt_caps(wdev) &&
		TWT_SUPPORT_BTWT(wlan_config_get_he_twt_support(wdev))) {
		ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
		band = HcGetBandByWdev(wdev);
		btwt = &ad->ApCfg.btwt[band];
		btwt_element_num = (btwt->btwt_element_exist) ? btwt->btwt_element_num : 0;
	} else {
		btwt_element_num = 0;
	}

	return btwt_element_num;
}

BOOLEAN twt_check_btwt_member(
	UINT16 *sta_list,
	UINT16 wcid)
{
	UINT8 i = 0;
	BOOLEAN found = FALSE;

	for (i = 0; i < TWT_HW_GRP_MAX_MEMBER_CNT; i++) {
		if (sta_list[i] == wcid) {
			found = TRUE;
			break;
		}
	}

	return found;
}

BOOLEAN twt_add_btwt_member(
	IN struct twt_link_node *curr_twt_node,
	IN UINT16 wcid)
{
	UINT8 i = 0;
	BOOLEAN done = FALSE;

	for (i = 0 ; i < TWT_HW_GRP_MAX_MEMBER_CNT; i++) {
		if (curr_twt_node->sta_list[i] == 0) {
			curr_twt_node->sta_list[i] = wcid;
			curr_twt_node->grp_member_cnt++;
			done = TRUE;
			break;
		}
	}

	return done;
}

VOID twt_remove_btwt_member(
	IN struct twt_link_node *curr_twt_node,
	IN UINT16 wcid)
{
	UINT8 i = 0;
	UINT8 j = 0;
	UINT16 temp_wcid[TWT_HW_GRP_MAX_MEMBER_CNT] = {0};

	for (i = 0; i < TWT_HW_GRP_MAX_MEMBER_CNT; i++) {
		if (curr_twt_node->sta_list[i] == wcid) {
			curr_twt_node->sta_list[i] = 0;
			curr_twt_node->grp_member_cnt--;
			break;
		}
	}

	for (i = 0; i < TWT_HW_GRP_MAX_MEMBER_CNT; i++) {
		if (curr_twt_node->sta_list[i] != 0)
			temp_wcid[j++] = curr_twt_node->sta_list[i];
	}

	for (i = 0; i < TWT_HW_GRP_MAX_MEMBER_CNT; i++)
		curr_twt_node->sta_list[i] = temp_wcid[i];
}

VOID twt_handle_peer_join_btwt_id(
	struct wifi_dev *wdev,
	UINT16 wcid,
	struct btwt_para_set *btwt_para_in,
	struct btwt_para_set *btwt_para_out)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	UINT8 sch_link_idx = 0;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct twt_agrt_para twt_agrt = {0};
	UINT8 band = 0;

	if (!btwt_para_in)
		return;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);

	if (!ad)
		return;

	if (!VALID_UCAST_ENTRY_WCID(ad, wcid))
		return;

	entry = &ad->MacTab.Content[wcid];
	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;
	band = HcGetBandByWdev(wdev);

	/* i have this btwt_id & have free space available & peer btwt para meet my config */
	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				return;
			}
			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band)) {
				/* step 5 */
				/* e.i */
				if (GET_BTWT_ID(curr_twt_node) != GET_BTWT_INFO_BTWT_ID(btwt_para_in->btwt_info)) {
					continue;
				}
				/* a.i */
				if (GET_TWT_RT_REQUEST(btwt_para_in->req_type) != 1) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						"Error, twt request=1,%d\n", (UINT16)GET_TWT_RT_REQUEST(btwt_para_in->req_type));
					continue;
				}
				/* a.iii */
				if (GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_TRIGGER) != GET_TWT_RT_TRIGGER(btwt_para_in->req_type)) {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						"Error, trigger=%d,%d\n", 
						GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_TRIGGER),
						(UINT16)GET_TWT_RT_REQUEST(btwt_para_in->req_type));
					continue;
				}
				/* a.v */
				if (GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE) != (GET_TWT_RT_FLOW_TYPE(btwt_para_in->req_type) ? 0 : 1)) {
					MTWF_DBG(ad,DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						"Error, announce=%d,%d\n",
						GET_AGRT_PARA_BITMAP(curr_twt_node, TWT_AGRT_PARA_BITMAP_IS_ANNOUNCE),
						(GET_TWT_RT_FLOW_TYPE(btwt_para_in->req_type) ? 1 : 0));
					continue;
				}
				/* a.vi */
				if (curr_twt_node->btwt_recommendation != GET_TWT_RT_BTWT_REC(btwt_para_in->req_type)) {
					MTWF_DBG(ad,DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						"Error, recommendation=%d,%d\n", 
						curr_twt_node->btwt_recommendation,
						(UINT16)GET_TWT_RT_BTWT_REC(btwt_para_in->req_type));
					continue;
				}
				/* a.vii & d */
				if (!twt_check_interval(
					curr_twt_node->agrt_sp_wake_intvl_mantissa,
					curr_twt_node->agrt_sp_wake_intvl_exponent,
					btwt_para_in->mantissa,
					GET_TWT_RT_WAKE_INTVAL_EXP(btwt_para_in->req_type))) {
					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						"%s:Error, man/exp=%d,%d,man/exp=%d,%d\n", __func__,
						curr_twt_node->agrt_sp_wake_intvl_mantissa,
						curr_twt_node->agrt_sp_wake_intvl_exponent,
						btwt_para_in->mantissa,
						(UINT16)GET_TWT_RT_WAKE_INTVAL_EXP(btwt_para_in->req_type));
					continue;
				}
				/* c */
				if (curr_twt_node->agrt_sp_duration != btwt_para_in->duration) {
					MTWF_DBG(ad,DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						"Error, sp_duration=%d,%d\n", 
						curr_twt_node->agrt_sp_duration,
						btwt_para_in->duration);
					continue;
				}

				if (curr_twt_node->grp_member_cnt >= TWT_HW_GRP_MAX_MEMBER_CNT) {
					MTWF_DBG(ad,DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						"Error, grp_member_cnt=%d,%d\n", 
						TWT_HW_GRP_MAX_MEMBER_CNT,
						curr_twt_node->grp_member_cnt);
					continue;
				}

				if (twt_check_btwt_member(curr_twt_node->sta_list, wcid)) {
					MTWF_DBG(ad,DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						"Error, wcid=%d already in btwt_id=%d\n", 
						wcid,
						(UINT16)GET_BTWT_ID(curr_twt_node));
					continue;
				}

				if (twt_add_btwt_member(curr_twt_node, wcid)) {
					/* Handle out frame */
					os_move_mem(btwt_para_out, btwt_para_in, sizeof(struct btwt_para_set));
					/* step 6 */
					/* a.i */
					btwt_para_out->req_type &= ~TWT_REQ_TYPE_TWT_REQUEST;
					/* a.ii */
					btwt_para_out->req_type &= ~TWT_REQ_TYPE_TWT_SETUP_COMMAND;
					btwt_para_out->req_type |= SET_TWT_RT_SETUP_CMD(TWT_SETUP_CMD_ACCEPT);
					/* b */
					btwt_para_out->target_wake_time = (UINT16)(curr_twt_node->schedule_sp_start_tsf >> 10);
					/* e.ii */
					btwt_para_out->btwt_info &= ~BTWT_INFO_BTWT_P;
					btwt_para_out->btwt_info |= SET_BTWT_INFO_BTWT_P(curr_twt_node->persistence);
					/* Handle peer state*/
					SET_PEER_JOIN_BTWT_ID(entry, GET_BTWT_ID(curr_twt_node));
					/* Handle fw cmd */
					twt_agrt_cmd_set(&twt_agrt,
						curr_twt_node,
						TWT_AGRT_CTRL_MODIFY,
						CMD_TSF_TYPE_BTWT);

					mt_asic_twt_agrt_update(wdev, twt_agrt);

					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"OK to add wcid=%d to btwt_id=%d\n",
						wcid, (UINT16)GET_BTWT_ID(curr_twt_node));
				}

				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
}

/* peer send action frame to AP to leave btwt_id */
VOID twt_handle_peer_leave_btwt_id(
	struct wifi_dev *wdev,
	UINT16 wcid,
	UINT16 btwt_id)
{
	struct _RTMP_ADAPTER *ad = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	UINT8 sch_link_idx = 0;
	struct hdev_ctrl *ctrl = NULL;
	struct _HD_RESOURCE_CFG *resource = NULL;
	struct twt_ctrl *twt_ctl = NULL;
	struct twt_link_node *curr_twt_node = NULL;
	struct twt_link_node *temp_twt_node = NULL;
	struct twt_agrt_para twt_agrt = {0};
	UINT8 band = 0;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return;
	}

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);

	if (!ad)
		return;

	if (!VALID_UCAST_ENTRY_WCID(ad, wcid))
		return;

	entry = &ad->MacTab.Content[wcid];
	ctrl = hc_get_hdev_ctrl(wdev);
	resource = &ctrl->HwResourceCfg;
	twt_ctl = &resource->twt_ctl;
	band = HcGetBandByWdev(wdev);

	/* i have this btwt_id & have free space available & peer btwt para meet my config */
	NdisAcquireSpinLock(&twt_ctl->twt_rec_lock);
	for (sch_link_idx = 0; sch_link_idx < SCH_LINK_NUM; sch_link_idx++) {
		DlListForEach(temp_twt_node, &twt_ctl->twt_link[sch_link_idx], struct twt_link_node, list) {
			curr_twt_node = temp_twt_node;
			if (!curr_twt_node) {
				NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"twt_entry=NULL, please check\n");
				return;
			}
			if ((curr_twt_node->type == TWT_TYPE_BTWT) &&
				(curr_twt_node->state == TWT_STATE_SW_OCCUPIED) &&
				(curr_twt_node->band == band)) {
				if (GET_BTWT_ID(curr_twt_node) != btwt_id) {
					continue;
				}

				if (twt_check_btwt_member(curr_twt_node->sta_list, wcid)) {
					SET_PEER_LEAVE_BTWT_ID(entry, btwt_id);
					twt_remove_btwt_member(curr_twt_node, wcid);
					twt_agrt_cmd_set(&twt_agrt,
						curr_twt_node,
						TWT_AGRT_CTRL_MODIFY,
						CMD_TSF_TYPE_BTWT);

					mt_asic_twt_agrt_update(wdev, twt_agrt);

					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"OK to remove wcid=%d from btwt_id=%d\n",
						wcid, (UINT16)GET_BTWT_ID(curr_twt_node));
				} else {
					MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"Fail. wcid=%d is not in btwt_id=%d\n",
						wcid, (UINT16)GET_BTWT_ID(curr_twt_node));
				}

				break;
			}
		}
	}
	NdisReleaseSpinLock(&twt_ctl->twt_rec_lock);
}

VOID twt_dump_btwt_elem(
	struct wifi_dev *wdev,
	UINT32 *btwt_id_bitmap)
{
	struct hdev_ctrl *ctrl = NULL;

	ctrl = hc_get_hdev_ctrl(wdev);

	twt_ctrl_btwt_dump(ctrl, btwt_id_bitmap);
}

BOOLEAN twt_is_sp_duration_tolerance(
	IN struct _RTMP_ADAPTER *ad,
	IN struct itwt_ie *twt_ie_in)
{
	struct _RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(ad->hdev_ctrl);

	if (twt_ie_in->duration < pChipCap->twt_sp_duration_min_num)
		return FALSE;

	return TRUE;
}

#ifdef APCLI_SUPPORT
VOID twtParseTWTElement(
	struct itwt_ie *prTWTIE,
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
	struct itwt_ie *prTWTIE)
{
	UINT16 u2ReqType;

	ASSERT(prTWTIE);
	if (!prTWTIE) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"prTWTIE is NULL, please check\n");
		return 0;
	}

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
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" apcli_entry=NULL, please check\n");
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
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" apcli_entry=NULL, please check\n");
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
	struct itwt_ie *prTWTBuf,
	UINT8 ucTWTFlowId,
	struct twt_params_t *prTWTParams)
{
	/* Add TWT element */
	prTWTBuf->elem_id = IE_TWT;
	prTWTBuf->len = sizeof(struct itwt_ie) - 2;

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
	struct frame_itwt_setup frame_out;
	struct itwt_ie *twt_ie_out = &frame_out.twt_ie;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	PUCHAR out_buffer = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	ULONG frame_len = 0;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" apcli_entry=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" pEntry=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	if (os_alloc_mem(pAd, &out_buffer, MAX_MGMT_PKT_LEN) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
		" allocate memory failed, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	os_zero_mem(&frame_out, sizeof(struct frame_itwt_setup));
	ActHeaderInit(pAd, &frame_out.hdr, pEntry->Addr, wdev->if_addr, wdev->bssid);
	frame_out.category = CATEGORY_S1G;
	frame_out.s1g_action = CATE_S1G_ACTION_TWT_SETUP;

	twtFillTWTElement(twt_ie_out, ucTWTFlowId, prTWTParams);

	/* send action frame to peer sta */
	MakeOutgoingFrame(out_buffer, &frame_len,
		sizeof(struct frame_itwt_setup), &frame_out,
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
	struct frame_itwt_teardown frame_out;
	PUCHAR out_buffer = NULL;
	ULONG frame_len = 0;
	struct _STA_ADMIN_CONFIG *apcli_entry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" apcli_entry=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" pEntry=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	/* send action frame to peer sta */
	if (os_alloc_mem(pAd, &out_buffer, MAX_MGMT_PKT_LEN) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd,DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" allocate memory failed, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	os_zero_mem(&frame_out, sizeof(struct frame_itwt_teardown));
	ActHeaderInit(pAd, &frame_out.hdr, pEntry->Addr, wdev->if_addr, wdev->bssid);
	frame_out.category = CATEGORY_S1G;
	frame_out.s1g_action = CATE_S1G_ACTION_TWT_TEARDOWN;
	frame_out.twt_flow_id = ucTWTFlowId;

	MakeOutgoingFrame(out_buffer,
		&frame_len,
		sizeof(struct frame_itwt_teardown),
		&frame_out,
		END_OF_ARGS);
	MiniportMMRequest(pAd,
		(MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[QID_AC_VO]),
		out_buffer, frame_len);

	os_free_mem(out_buffer);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
		" wcid(%d), flow_id(%d)\n",
		pEntry->wcid,
		ucTWTFlowId);

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
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
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
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" apcli_entry=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" pEntry=NULL, please check\n");
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

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"del twt agrt to FW,wcid=%d,flow_id=%d,tbl_idx=%d\n",
		 pEntry->wcid, ucTWTFlowId, ucAgrtTblIdx);

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
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"wdev=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"apcli_entry=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" pEntry=NULL, please check\n");
		return NDIS_STATUS_FAILURE;
	}

	/* Delete driver & FW TWT agreement entry */
	twtPlannerDelAgrtTbl(pAd, wdev, ucTWTFlowId, TRUE);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		" wcid(%d), flow_id(%d)\n", 
		pEntry->wcid, ucTWTFlowId);

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
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"BSS %u TWT flow %u doesn't exist\n\n", ucBssIdx, ucFlowId);
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
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
		return;
	}

	apcli_entry = &pAd->StaCfg[wdev->func_idx];

	if (!apcli_entry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" apcli_entry=NULL, please check\n");
		return;
	}

	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" pEntry=NULL, please check\n");
		return;
	}

	prTWTFlow = &(apcli_entry->arTWTFlow[ucTWTFlowId]);
	prTWTResult = &(prTWTFlow->rTWTPeerParams);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"ucSetupCmd=%d\n", prTWTResult->ucSetupCmd);

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

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"twtReqFsmSteps ePreState=%d, eNextState=%d\n",
			ePreState, eNextState);

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
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						"prTWTParams is NULL, TWT_REQ TX Failure!\n");
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

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		" ucIdx=%d, ucBssIdx=%d, ucFlowId=%d\n",
		ucIdx, ucBssIdx, ucFlowId);

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
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			" wdev=NULL, please check\n");
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
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			" Agreement table is full\n");
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

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		" set twt agrt to FW,wcid=%d,flow_id=%d,tbl_idx=%d\n",
		pEntry->wcid, ucFlowId, ucAgrtTblIdx);

	return NDIS_STATUS_SUCCESS;
}

static struct twt_flow_t *twtPlannerFlowFindById(
	IN struct _STA_ADMIN_CONFIG *prStaCfg,
	IN UINT8 ucFlowId)
{
	struct twt_flow_t *prTWTFlow = NULL;

	ASSERT(prStaCfg);

	if (ucFlowId >= TWT_MAX_FLOW_NUM) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"Invalid TWT flow id %u\n",
			ucFlowId);
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

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		" u8CurTsf=%llu, Reason=%d\n",
		u8CurTsf, prGetTsfCtxt->ucReason);

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
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"NULL Pointer!\n");
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
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"Unknown reason to get TSF %u\n", prGetTsfCtxt->ucReason);
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
	struct frame_itwt_setup frame_setup;
	struct frame_itwt_teardown frame_tear_down;
	struct itwt_ie *twt_ie_out;
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
				NdisCopyMemory(&frame_setup, pHead, sizeof(struct frame_itwt_setup));
				twt_ie_out = &frame_setup.twt_ie;
				ucTWTFlowId = twtGetRxSetupFlowId(twt_ie_out);
				twtReqFsmSteps(pAd, wdev, TWT_REQ_STATE_WAIT_RSP, ucTWTFlowId, NULL);
			}
			break;

		case CATE_S1G_ACTION_TWT_TEARDOWN:
			if (apcli_entry->aeTWTReqState == TWT_REQ_STATE_TEARING_DOWN) {
				NdisCopyMemory(&frame_tear_down, pHead, sizeof(struct frame_itwt_teardown));
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
	MTWF_PRINT("prTWTParams->fgReq = %d\n", prTWTParams->fgReq);
	MTWF_PRINT("prTWTParams->ucSetupCmd = %d\n", prTWTParams->ucSetupCmd);
	MTWF_PRINT("prTWTParams->fgTrigger = %d\n", prTWTParams->fgTrigger);
	MTWF_PRINT("prTWTParams->fgUnannounced = %d\n", prTWTParams->fgUnannounced);
	MTWF_PRINT("prTWTParams->ucWakeIntvalExponent = %d\n", prTWTParams->ucWakeIntvalExponent);
	MTWF_PRINT("prTWTParams->tsf_low = 0x%.8x\n", cpu2le32(prTWTParams->u8TWT & 0xFFFFFFFF));
	MTWF_PRINT("prTWTParams->tsf_high = 0x%.8x\n", cpu2le32((UINT32)(prTWTParams->u8TWT >> 32)));
	MTWF_PRINT("prTWTParams->fgProtect = %d\n", prTWTParams->fgProtect);
	MTWF_PRINT("prTWTParams->ucMinWakeDur = %d\n", prTWTParams->ucMinWakeDur);
	MTWF_PRINT("prTWTParams->u2WakeIntvalMantiss = %d\n", prTWTParams->u2WakeIntvalMantiss);
}

UINT32 twtGetTxTeardownFlowId(
	IN struct frame_itwt_teardown *pframe_tear_down)
{
	UINT8 ucFlowId;

	ucFlowId = (pframe_tear_down->twt_flow_id & TWT_TEARDOWN_FLOW_ID);
	return ucFlowId;
}

BOOLEAN twtPlannerIsRunning(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _STA_ADMIN_CONFIG *prStaCfg
	)
{
	struct twt_planner_t *prTWTPlanner = NULL;
	struct twt_agrt_t *prTWTAgrt = NULL;
	UINT8 ucIdx;
	BOOLEAN bFound = FALSE;

	ASSERT(prStaCfg);
	if (!prStaCfg) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"prStaCfg is NULL, please check\n");
		return bFound;
	}

	prTWTPlanner = &(prStaCfg->rTWTPlanner);

	for (ucIdx = 0; ucIdx < TWT_AGRT_MAX_NUM; ucIdx++) {
		prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[ucIdx]);
		if ((prTWTAgrt != NULL) && (prTWTAgrt->fgValid == TRUE)) {
			bFound = TRUE;
			break;
		}
	}

	return bFound;
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
