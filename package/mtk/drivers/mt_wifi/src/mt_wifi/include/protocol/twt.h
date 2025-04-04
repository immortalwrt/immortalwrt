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
    twt.h

    Abstract:
    twt spec. related

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------

*/

#ifndef _TWT_H_
#define _TWT_H_

#include "twt_ctrl.h"

#ifdef MT_MAC

/* TWT definitions for protocol */
#define TWT_PROFILE_SUPPORT_DISABLE         0
#define TWT_PROFILE_SUPPORT_ITWT            1
#define TWT_PROFILE_SUPPORT_BTWT            2
#define TWT_PROFILE_SUPPORT_BTWT_ITWT       3
#define TWT_PROFILE_SUPPORT_TYPE_NUM        4

/* TWT Element (802.11ax D6.0) */

/* Tsf from which request */
#define TSF_FROM_SETUP_CMD_REQUEST  0
#define TSF_FROM_SETUP_CMD_SUGGEST  1
#define TSF_FROM_SETUP_CMD_DEMAND   2

/* TWT flow id max num */
#define TWT_FLOW_ID_MAX_NUM			8

/* bTWT max btwt_id num */
#define TWT_BTWT_ID_NUM             32  /* 0~31 */

/* TWT element */
#define TWT_REQ_TYPE_TWT_REQUEST                        BIT(0)
#define TWT_REQ_TYPE_TWT_SETUP_COMMAND                  BITS(1, 3)
#define TWT_REQ_TYPE_TWT_TRIGGER                        BIT(4)
#define TWT_REQ_TYPE_TWT_IMPLICIT_LAST_BCAST_PARAM      BIT(5)
#define TWT_REQ_TYPE_TWT_FLOWTYPE                       BIT(6)
#define TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER                BITS(7, 9)
#define TWT_REQ_TYPE_BTWT_RECOMMENDATION                BITS(7, 9)
#define TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP                BITS(10, 14)
#define TWT_REQ_TYPE_TWT_PROTECTION                     BIT(15)

#define TWT_REQ_TYPE_TWT_REQUEST_OFFSET                    0
#define TWT_REQ_TYPE_TWT_SETUP_COMMAND_OFFSET              1
#define TWT_REQ_TYPE_TWT_TRIGGER_OFFSET                    4
#define TWT_REQ_TYPE_TWT_IMPLICIT_LAST_BCAST_PARAM_OFFSET  5
#define TWT_REQ_TYPE_TWT_FLOWTYPE_OFFSET                   6
#define TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER_OFFSET            7
#define TWT_REQ_TYPE_TWT_BTWT_RECOMMENDATION_OFFSET        7
#define TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP_OFFSET            10
#define TWT_REQ_TYPE_TWT_PROTECTION_OFFSET                 15

#define TWT_SETUP_CMD_REQUEST                       0
#define TWT_SETUP_CMD_SUGGEST                       1
#define TWT_SETUP_CMD_DEMAND                        2
#define TWT_SETUP_CMD_GROUPING                      3
#define TWT_SETUP_CMD_ACCEPT                        4
#define TWT_SETUP_CMD_ALTERNATE                     5
#define TWT_SETUP_CMD_DICTATE                       6
#define TWT_SETUP_CMD_REJECT                        7

/* TWT Flow Field in teardown frame */
#define TWT_TEARDOWN_FLOW_ID                        BITS(0, 2)

/* TWT Information Field */
#define TWT_INFO_FLOW_ID                            BITS(0, 2)
#define TWT_INFO_RESP_REQUESTED                     BIT(3)
#define TWT_INFO_NEXT_TWT_REQ                       BIT(4)
#define TWT_INFO_NEXT_TWT_SUBFIELD_SIZE             BITS(5, 6)
#define TWT_INFO_ALL_TWT                            BIT(7)

#define TWT_INFO_FLOW_ID_OFFSET                     0
#define TWT_INFO_RESP_REQUESTED_OFFSET              3
#define TWT_INFO_NEXT_TWT_REQ_OFFSET                4
#define TWT_INFO_NEXT_TWT_SUBFIELD_SIZE_OFFSET      5
#define TWT_INFO_ALL_TWT_OFFSET                     7

#define NEXT_TWT_SUBFIELD_ZERO_BIT                  0
#define NEXT_TWT_SUBFIELD_32_BITS                   1
#define NEXT_TWT_SUBFIELD_48_BITS                   2
#define NEXT_TWT_SUBFIELD_64_BITS                   3

/* TWT teardown */
#define TEARDOWM_FRAME_NEGO_TYPE                    BITS(5, 6)
#define TEARDOWM_FRAME_NEGO_TYPE_OFFSET             5

/* TWT teardown (bTWT) */
#define BTWT_FLOW_BTWT_ID                           BITS(0, 4)
#define BTWT_FLOW_NEGO_TYPE                         BITS(5, 6)
#define BTWT_FLOW_TEARDOWN_ALL_TWT                  BIT(7)
#define BTWT_FLOW_BTWT_ID_OFFSET                    0
#define BTWT_FLOW_NEGO_TYPE_OFFSET                  5
#define BTWT_FLOW_TEARDOWN_ALL_TWT_OFFSET           7

/* TWT element -> control */
#define TWT_CTRL_NDP_PAGING_INDICATOR               BIT(0)
#define TWT_CTRL_RESPONDER_PM_MODE                  BIT(1)
#define TWT_CTRL_NEGO_TYPE                          BITS(2, 3)
#define TWT_CTRL_INFO_FRM_DIS                       BIT(4)
#define TWT_CTRL_WAKE_DUR_UNIT                      BIT(5)

#define TWT_CTRL_NDP_PAGING_INDICATOR_OFFSET        0
#define TWT_CTRL_RESPONDER_PM_MODE_OFFSET           1
#define TWT_CTRL_NEGO_TYPE_OFFSET                   2
#define TWT_CTRL_INFO_FRM_DIS_OFFSET                4
#define TWT_CTRL_WAKE_DUR_UNIT_OFFSET               5

#define TWT_CTRL_NEGO_TYPE_ITWT                     0
#define TWT_CTRL_NEGO_TYPE_NEXT_WAKE_TBTT           1
#define TWT_CTRL_NEGO_TYPE_BTWT_ANNOUNCE            2
#define TWT_CTRL_NEGO_TYPE_BTWT_MBR_MGMT            3

/* TWT element -> bTWT */
#define BTWT_INFO_BTWT_ID                           BITS(3, 7)
#define BTWT_INFO_BTWT_P                            BITS(8, 15)
#define BTWT_INFO_BTWT_ID_OFFSET                    3
#define BTWT_INFO_BTWT_P_OFFSET                     8


#ifdef APCLI_SUPPORT
/* TWT related definitions */
#define TWT_AGRT_MAX_NUM        16
#define TWT_GRP_MAX_NUM         8
#define TWT_GRP_MAX_MEMBER_CNT  8
#endif /* APCLI_SUPPORT */

/* TWT tsf type to CMD */
#define CMD_TST_TYPE_SCHEDULE        0 /* peer STA seed: suggest/request */
#define CMD_TSF_TYPE_REQUESTER       1 /* peer STA send: demand */
#define CMD_TSF_TYPE_TWT_INFO        2 /* peer STA sned twt info. frame */
#define CMD_TSF_TYPE_NA              3 /* used in teardown */
#define CMD_TSF_TYPE_BTWT            4 /* used in bTWT acquire btwt element */


struct GNU_PACKED itwt_ie {
	UINT8 elem_id;
	UINT8 len;
	UINT8 control;
	UINT16 req_type;
	UINT32 target_wake_time[2];
	/* twt group assignment: not used */
	UINT8 duration;
	UINT16 mantissa;
	/* broadcast twt id: not used */
	UINT8 channel;
	/* ndp paging: not used */
};

struct GNU_PACKED itwt_para_set {
	UINT16 req_type;
	UINT32 target_wake_time[2];
	/* twt group assignment: not used */
	UINT8 duration;
	UINT16 mantissa;
	/* broadcast twt id: not used */
	UINT8 channel;
	/* ndp paging: not used */
};

struct GNU_PACKED btwt_para_set {
	UINT16 req_type;
	UINT16 target_wake_time;
	UINT8 duration;
	UINT16 mantissa;
	UINT16 btwt_info;
};

struct GNU_PACKED btwt_ie {
	UINT8 elem_id;
	UINT8 len;
	UINT8 control;
	struct btwt_para_set btwt_para[TWT_HW_BTWT_MAX_NUM];
};


/* TWT setup frame action filed format */
#define CATE_S1G_ACTION_TWT_SETUP 6

struct GNU_PACKED frame_twt_setup {
	struct _HEADER_802_11 hdr;
	UINT8 category;
	UINT8 s1g_action;
	UINT8 token;
	UINT8 elem_id;
	UINT8 len;
	UINT8 control;
	UINT8 oct[0];
};

struct GNU_PACKED frame_itwt_setup {
	struct _HEADER_802_11 hdr;
	UINT8 category;
	UINT8 s1g_action;
	UINT8 token;
	struct itwt_ie twt_ie;
};

struct GNU_PACKED frame_btwt_setup {
	struct _HEADER_802_11 hdr;
	UINT8 category;
	UINT8 s1g_action;
	UINT8 token;
	UINT8 elem_id;
	UINT8 len;
	UINT8 control;
	struct btwt_para_set btwt_para[0];
};

/* TWT teardown frame action filed format */
#define CATE_S1G_ACTION_TWT_TEARDOWN 7
struct GNU_PACKED frame_twt_teardown {
	struct _HEADER_802_11 hdr;
	UINT8 category;
	UINT8 s1g_action;
	UINT8 twt_flow;
};

struct GNU_PACKED frame_itwt_teardown {
	struct _HEADER_802_11 hdr;
	UINT8 category;
	UINT8 s1g_action;
	UINT8 twt_flow_id;
};

struct GNU_PACKED frame_btwt_teardown {
	struct _HEADER_802_11 hdr;
	UINT8 category;
	UINT8 s1g_action;
	UINT8 twt_flow;
};
/* TWT information frame action filed format */
#define CATE_S1G_ACTION_TWT_INFO 11

struct GNU_PACKED frame_twt_information {
	struct _HEADER_802_11 hdr;
	UINT8 category;
	UINT8 s1g_action;
	UINT8 twt_info;
	UINT32 next_twt[2];
};

struct GNU_PACKED twt_resume_info{
	UINT8 bssinfo_idx;
	UINT16 wcid;
	UINT8 flow_id;
	UINT8 idle;
	UINT8 reserved[3];
};

/* MAcro to get TWT protocol support status */
#define TWT_SUPPORT_ITWT(profile_value) \
	(((profile_value) & TWT_PROFILE_SUPPORT_ITWT) ? TRUE : FALSE)

#define TWT_SUPPORT_BTWT(profile_value) \
	(((profile_value) & TWT_PROFILE_SUPPORT_BTWT) ? TRUE : FALSE)

#define TWT_SUPPORT_BTWT_OR_ITWT(profile_value) \
	(((profile_value) & (TWT_PROFILE_SUPPORT_BTWT | TWT_PROFILE_SUPPORT_ITWT)) ? TRUE : FALSE)


/* Macro to set TWT element -> control field */
#define SET_TWT_CTRL_NDP_PAGING_INDICATOR(ndp_paging) \
	(((ndp_paging) << TWT_CTRL_NDP_PAGING_INDICATOR_OFFSET) & \
		TWT_CTRL_NDP_PAGING_INDICATOR)

#define SET_TWT_CTRL_RESPONDER_PM_MODE(rsp_pm) \
	(((rsp_pm) << TWT_CTRL_RESPONDER_PM_MODE_OFFSET) & \
		TWT_CTRL_RESPONDER_PM_MODE)

#define SET_TWT_CTRL_NEGO_TYPE(nego_type) \
	(((nego_type) << TWT_CTRL_NEGO_TYPE_OFFSET) & \
		TWT_CTRL_NEGO_TYPE)

#define SET_TWT_CTRL_INFO_FRM_DIS(info_frm_dis) \
	(((info_frm_dis) << TWT_CTRL_INFO_FRM_DIS_OFFSET) & \
		TWT_CTRL_INFO_FRM_DIS)

#define SET_TWT_CTRL_WAKE_DUR_UNIT(wake_dur_unit) \
	(((wake_dur_unit) << TWT_CTRL_WAKE_DUR_UNIT_OFFSET) & \
		TWT_CTRL_WAKE_DUR_UNIT)

/* Macro to get TWT element -> control field */
#define GET_TWT_CTRL_NDP_PAGING_INDICATOR(control) \
	(((control) & TWT_CTRL_NDP_PAGING_INDICATOR) >> TWT_CTRL_NDP_PAGING_INDICATOR_OFFSET)

#define GET_TWT_CTRL_RESPONDER_PM_MODE(control) \
	(((control) & TWT_CTRL_RESPONDER_PM_MODE) >> TWT_CTRL_RESPONDER_PM_MODE_OFFSET)

#define GET_TWT_CTRL_NEGO_TYPE(control) \
	(((control) & TWT_CTRL_NEGO_TYPE) >> TWT_CTRL_NEGO_TYPE_OFFSET)

#define GET_TWT_CTRL_INFO_FRM_DIS(control) \
	(((control) & TWT_CTRL_INFO_FRM_DIS) >> TWT_CTRL_INFO_FRM_DIS_OFFSET)

#define GET_TWT_CTRL_WAKE_DUR_UNIT(control) \
	(((control) & TWT_CTRL_WAKE_DUR_UNIT) >> TWT_CTRL_WAKE_DUR_UNIT_OFFSET)

/* Macros to set twt request type field */
#define SET_TWT_RT_REQUEST(fgReq) \
	(((fgReq) << TWT_REQ_TYPE_TWT_REQUEST_OFFSET) & \
		TWT_REQ_TYPE_TWT_REQUEST)

#define SET_TWT_RT_SETUP_CMD(ucSetupCmd) \
	(((ucSetupCmd) << TWT_REQ_TYPE_TWT_SETUP_COMMAND_OFFSET) & \
		TWT_REQ_TYPE_TWT_SETUP_COMMAND)

#define SET_TWT_RT_TRIGGER(fgTrigger) \
	(((fgTrigger) << TWT_REQ_TYPE_TWT_TRIGGER_OFFSET) & TWT_REQ_TYPE_TWT_TRIGGER)

#define SET_TWT_RT_IMPLICIT_LAST(value) \
	(((value) << TWT_REQ_TYPE_TWT_IMPLICIT_LAST_BCAST_PARAM_OFFSET) & \
		TWT_REQ_TYPE_TWT_IMPLICIT_LAST_BCAST_PARAM)

#define SET_TWT_RT_FLOW_TYPE(fgUnannounced) \
	(((fgUnannounced) << TWT_REQ_TYPE_TWT_FLOWTYPE_OFFSET) & \
		TWT_REQ_TYPE_TWT_FLOWTYPE)

#define SET_TWT_RT_FLOW_ID(ucTWTFlowId) \
	(((ucTWTFlowId) << TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER_OFFSET) & \
		TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER)

#define SET_TWT_RT_BTWT_REC(ucTWTFlowId) \
	(((ucTWTFlowId) << TWT_REQ_TYPE_TWT_BTWT_RECOMMENDATION_OFFSET) & \
		TWT_REQ_TYPE_BTWT_RECOMMENDATION)

#define SET_TWT_RT_WAKE_INTVAL_EXP(ucWakeIntvlExponent) \
	(((ucWakeIntvlExponent) << TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP_OFFSET) & \
		TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP)

#define SET_TWT_RT_PROTECTION(fgProtect) \
	(((fgProtect) << TWT_REQ_TYPE_TWT_PROTECTION_OFFSET) & \
		TWT_REQ_TYPE_TWT_PROTECTION)

/* Macros for getting request type bit fields in TWT IE */
#define GET_TWT_RT_REQUEST(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_REQUEST) >> \
		TWT_REQ_TYPE_TWT_REQUEST_OFFSET)

#define GET_TWT_RT_SETUP_CMD(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_SETUP_COMMAND) >> \
		TWT_REQ_TYPE_TWT_SETUP_COMMAND_OFFSET)

#define GET_TWT_RT_TRIGGER(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_TRIGGER) >> TWT_REQ_TYPE_TWT_TRIGGER_OFFSET)

#define GET_TWT_RT_IMPLICIT_LAST(value) \
	(((value) & TWT_REQ_TYPE_TWT_IMPLICIT_LAST_BCAST_PARAM) >> \
		TWT_REQ_TYPE_TWT_IMPLICIT_LAST_BCAST_PARAM_OFFSET)

#define GET_TWT_RT_FLOW_TYPE(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_FLOWTYPE) >> TWT_REQ_TYPE_TWT_FLOWTYPE_OFFSET)

#define GET_TWT_RT_FLOW_ID(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER) >> \
		TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER_OFFSET)

#define GET_TWT_RT_BTWT_REC(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_BTWT_RECOMMENDATION) >> \
		TWT_REQ_TYPE_TWT_BTWT_RECOMMENDATION_OFFSET)

#define GET_TWT_RT_WAKE_INTVAL_EXP(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP) >> \
		TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP_OFFSET)

#define GET_TWT_RT_PROTECTION(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_PROTECTION) >> \
		TWT_REQ_TYPE_TWT_PROTECTION_OFFSET)

/* Macros to set TWT info field */
#define SET_TWT_INFO_FLOW_ID(ucNextTWTCtrl, _id) \
	(ucNextTWTCtrl |= (((_id) << TWT_INFO_FLOW_ID_OFFSET) & TWT_INFO_FLOW_ID))

#define SET_TWT_INFO_RESP_REQUESTED(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) << TWT_INFO_RESP_REQUESTED_OFFSET) & \
	TWT_INFO_RESP_REQUESTED)

#define SET_TWT_INFO_NEXT_TWT_REQ(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) << TWT_INFO_NEXT_TWT_REQ_OFFSET) & \
	TWT_INFO_NEXT_TWT_REQ)

#define SET_TWT_INFO_NEXT_TWT_SUBFIELD_SIZE(ucNextTWTCtrl, _subfield_size) \
	(ucNextTWTCtrl |= (((_subfield_size) << TWT_INFO_NEXT_TWT_SUBFIELD_SIZE_OFFSET) & \
	TWT_INFO_NEXT_TWT_SUBFIELD_SIZE))

#define SET_TWT_INFO_ALL_TWT(ucNextTWTCtrl, _all_twt) \
	(ucNextTWTCtrl |= (((_all_twt) << TWT_INFO_ALL_TWT_OFFSET) & \
	TWT_INFO_ALL_TWT))

/* Macros to get TWT info field */
#define GET_TWT_INFO_FLOW_ID(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) & TWT_INFO_FLOW_ID) >> TWT_INFO_FLOW_ID_OFFSET)

#define GET_TWT_INFO_RESP_REQUESTED(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) & TWT_INFO_RESP_REQUESTED) >> \
	TWT_INFO_RESP_REQUESTED_OFFSET)

#define GET_TWT_INFO_NEXT_TWT_REQ(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) & TWT_INFO_NEXT_TWT_REQ) >> \
	TWT_INFO_NEXT_TWT_REQ_OFFSET)

#define GET_TWT_INFO_NEXT_TWT_SUBFIELD_SIZE(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) & TWT_INFO_NEXT_TWT_SUBFIELD_SIZE) >> \
	TWT_INFO_NEXT_TWT_SUBFIELD_SIZE_OFFSET)

#define GET_TWT_INFO_ALL_TWT(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) & TWT_INFO_ALL_TWT) >> \
	TWT_INFO_ALL_TWT_OFFSET)

/* teardown(GET) */
#define GET_TEARDWON_FRAME_NEGO_TYPE(twt_flow) \
	(((twt_flow) & TEARDOWM_FRAME_NEGO_TYPE) >> TEARDOWM_FRAME_NEGO_TYPE_OFFSET)

/* bTWT teardown(SET) */
#define SET_BTWT_FLOW_BTWT_ID(btwt_id) \
	(((btwt_id) << BTWT_FLOW_BTWT_ID_OFFSET) & BTWT_FLOW_BTWT_ID)

#define SET_BTWT_FLOW_NEGO_TYPE(nego_type) \
	(((nego_type) << BTWT_FLOW_NEGO_TYPE_OFFSET) & BTWT_FLOW_NEGO_TYPE)

#define SET_BTWT_FLOW_TEARDOWN_ALL_TWT(nego_type) \
	(((nego_type) << BTWT_FLOW_TEARDOWN_ALL_TWT_OFFSET) & BTWT_FLOW_TEARDOWN_ALL_TWT)

/* bTWT teardown(GET) */
#define GET_BTWT_FLOW_BTWT_ID(twt_flow) \
	(((twt_flow) & BTWT_FLOW_BTWT_ID) >> BTWT_FLOW_BTWT_ID_OFFSET)

#define GET_BTWT_FLOW_NEGO_TYPE(twt_flow) \
	(((twt_flow) & BTWT_FLOW_NEGO_TYPE) >> BTWT_FLOW_NEGO_TYPE_OFFSET)

#define GET_BTWT_FLOW_TEARDOWN_ALL_TWT(twt_flow) \
	(((twt_flow) & BTWT_FLOW_TEARDOWN_ALL_TWT) >> BTWT_FLOW_TEARDOWN_ALL_TWT_OFFSET)

/* TWT element -> bTWT */
#define SET_BTWT_INFO_BTWT_ID(btwt_id) \
	(((btwt_id) << BTWT_INFO_BTWT_ID_OFFSET) & \
		BTWT_INFO_BTWT_ID)

#define SET_BTWT_INFO_BTWT_P(p) \
	(((p) << BTWT_INFO_BTWT_P_OFFSET) & \
		BTWT_INFO_BTWT_P)

#define GET_BTWT_INFO_BTWT_ID(btwt_info) \
	(((btwt_info) & BTWT_INFO_BTWT_ID) >> BTWT_INFO_BTWT_ID_OFFSET)

#define GET_BTWT_INFO_BTWT_P(btwt_info) \
	(((btwt_info) & BTWT_INFO_BTWT_P) >> BTWT_INFO_BTWT_P_OFFSET)

/* peer sta join/leave btwt macro */
#define SET_PEER_JOIN_BTWT_ID(entry, btwt_id) \
	((entry->twt_btwt_id_bitmap) |= (1 << btwt_id))

#define SET_PEER_LEAVE_BTWT_ID(entry, btwt_id) \
	((entry->twt_btwt_id_bitmap) &= ~(1 << btwt_id))

#define SET_PEER_LEAVE_ALL_BTWT(entry) \
	(entry->twt_btwt_id_bitmap = 0)

#define GET_PEER_JOIN_ANY_BTWT(entry) \
	((entry->twt_btwt_id_bitmap) ? TRUE : FALSE)

#define GET_PEER_JOIN_BTWT_ID(entry, btwt_id) \
	((entry->twt_btwt_id_bitmap & (1 << btwt_id)) ? TRUE : FALSE)


#ifdef APCLI_SUPPORT
#define TSF_OFFSET_FOR_EMU	   (1 * 1000 * 1000)	/* after 1 sec */
#define TSF_OFFSET_FOR_AGRT_ADD	   (5 * 1000 * 1000)	/* after 5 sec */
#define TSF_OFFSET_FOR_AGRT_RESUME (5 * 1000 * 1000)	/* after 5 sec */

enum ENUM_MID_TWT_REQ_FSM_T {
	MID_TWT_REQ_FSM_START = 0,
	MID_TWT_REQ_FSM_TEARDOWN,
	MID_TWT_REQ_FSM_SUSPEND,
	MID_TWT_REQ_FSM_RESUME,
	MID_TWT_REQ_IND_RESULT,
	MID_TWT_REQ_IND_SUSPEND_DONE,
	MID_TWT_REQ_IND_RESUME_DONE,
	MID_TWT_REQ_IND_TEARDOWN_DONE,
	MID_TWT_REQ_IND_INFOFRM,
	MID_TWT_PARAMS_SET,
	MID_TWT_REQ_FSM_NUM,
};

enum ENUM_TWT_REQUESTER_STATE_T {
	TWT_REQ_STATE_IDLE = 0,
	TWT_REQ_STATE_REQTX,
	TWT_REQ_STATE_WAIT_RSP,
	TWT_REQ_STATE_SUSPENDING,
	TWT_REQ_STATE_SUSPENDED,
	TWT_REQ_STATE_RESUMING,
	TWT_REQ_STATE_TEARING_DOWN,
	TWT_REQ_STATE_RX_TEARDOWN,
	TWT_REQ_STATE_RX_INFOFRM,
	TWT_REQ_STATE_NUM
};

/* Definitions for action control of TWT params */
enum {
	TWT_PARAM_ACTION_NONE = 0,
	TWT_PARAM_ACTION_ADD_BYPASS = 1, /* bypass nego & add an agrt */
	TWT_PARAM_ACTION_DEL_BYPASS = 2, /* bypass proto & del an agrt */
	TWT_PARAM_ACTION_MOD_BYPASS = 3, /* bypass proto & modify an agrt */
	TWT_PARAM_ACTION_ADD = 4,
	TWT_PARAM_ACTION_DEL = 5,
	TWT_PARAM_ACTION_SUSPEND = 6,
	TWT_PARAM_ACTION_RESUME = 7,
	TWT_PARAM_ACTION_MAX
};

enum TWT_GET_TSF_REASON {
	TWT_GET_TSF_FOR_ADD_AGRT_BYPASS = 1,
	TWT_GET_TSF_FOR_ADD_AGRT = 2,
	TWT_GET_TSF_FOR_RESUME_AGRT = 3,
	TWT_GET_TSF_REASON_MAX
};

#define TWT_MAX_FLOW_NUM        8
#define TWT_MAX_WAKE_INTVAL_EXP 0x1F

#define IS_TWT_PARAM_ACTION_ADD_BYPASS(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_ADD_BYPASS)
#define IS_TWT_PARAM_ACTION_DEL_BYPASS(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_DEL_BYPASS)
#define IS_TWT_PARAM_ACTION_MOD_BYPASS(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_MOD_BYPASS)
#define IS_TWT_PARAM_ACTION_ADD(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_ADD)
#define IS_TWT_PARAM_ACTION_DEL(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_DEL)
#define IS_TWT_PARAM_ACTION_SUSPEND(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_SUSPEND)
#define IS_TWT_PARAM_ACTION_RESUME(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_RESUME)
#endif /* APCLI_SUPPORT */

#ifdef APCLI_SUPPORT
struct twt_params_t {
	UINT8 fgReq;
	UINT8 fgTrigger;
	UINT8 fgProtect;
	UINT8 fgUnannounced;
	UINT8 ucSetupCmd;
	UINT8 ucMinWakeDur;
	UINT8 ucWakeIntvalExponent;
	UINT16 u2WakeIntvalMantiss;
	UINT64 u8TWT;
};

struct next_twt_info_t {
	UINT64 u8NextTWT;
	UINT8 ucNextTWTSize;
};

struct twt_ctrl_t {
	UINT8 ucBssIdx;
	UINT8 ucCtrlAction;
	UINT8 ucTWTFlowId;
	struct twt_params_t rTWTParams;
};

struct twt_get_tsf_context_t {
	enum TWT_GET_TSF_REASON ucReason;
	UINT8 ucBssIdx;
	UINT8 ucTWTFlowId;
	struct twt_params_t rTWTParams;
};

struct twt_flow_t {
	struct twt_params_t rTWTParams;
	struct twt_params_t rTWTPeerParams;
	UINT64 u8NextTWT;
};

struct twt_agrt_t {
	UINT8 fgValid;
	UINT8 ucAgrtTblIdx;
	UINT8 ucBssIdx;
	UINT8 ucFlowId;
	struct twt_params_t rTWTAgrt;
};

struct twt_planner_t {
	struct twt_agrt_t arTWTAgrtTbl[TWT_AGRT_MAX_NUM];
};

/* 11ax TWT Information frame format */
struct GNU_PACKED frame_twt_info {
	/* MAC header */
	UINT16 u2FrameCtrl;	/* Frame Control */
	UINT16 u2Duration;	/* Duration */
	UINT8 aucDestAddr[MAC_ADDR_LEN];	/* DA */
	UINT8 aucSrcAddr[MAC_ADDR_LEN];	/* SA */
	UINT8 aucBSSID[MAC_ADDR_LEN];	/* BSSID */
	UINT16 u2SeqCtrl;	/* Sequence Control */
	/* TWT Information frame body */
	UINT8 ucCategory;	/* Category */
	UINT8 ucAction;	/* Action Value */
	UINT8 ucNextTWTCtrl;
	UINT8 aucNextTWT[0];
};

struct GNU_PACKED msg_twt_fsm_t {
	UCHAR eMsgId;	/* Must be the first member */
	struct wifi_dev *wdev;
	UINT8 ucTWTFlowId;
	struct twt_ctrl_t rtwtCtrl;
};

/*
 * Important: Used for Communication between Host and WM-CPU,
 * should be packed and DW-aligned and in little-endian format
 */
struct GNU_PACKED cmd_twt_agrt_update_t {
	/* DW0 */
	UINT8 ucAgrtTblIdx;
	UINT8 ucAgrtCtrlFlag;
	UINT8 ucOwnMacId;
	UINT8 ucFlowId;		/* It is set to 0xff when peerGrpId is a group ID */
	/* DW1 */
	UINT16 u2PeerIdGrpId;	/* Specify the peer ID (MSB=0) or group ID (MSB=1)
				 * (10 bits for StaIdx, MSB to identify if it is for groupId)
				 */
	UINT8  ucAgrtSpDuration;	/* Same as SPEC definition. 8 bits, in unit of 256 us */
	UINT8  ucBssIndex;		/* So that we know which BSS TSF should be used for this AGRT */
	/* DW2, DW3, DW4 */
	UINT32 u4AgrtSpStartTsfLow;
	UINT32 u4AgrtSpStartTsfHigh;
	UINT16 u2AgrtSpWakeIntvlMantissa;
	UINT8  ucAgrtSpWakeIntvlExponent;
	UINT8  ucIsRoleAp;		/* 1: AP, 0: STA */
	/* DW5 */
	UINT8  ucAgrtParaBitmap;	/* For Bitmap definition, please refer to
					 * TWT_AGRT_PARA_BITMAP_IS_TRIGGER and etc
					 */
	UINT8  ucReserved_a;
	UINT16 u2Reserved_b;		/* Following field is valid ONLY when peerIdGrpId is a group ID */
	/* DW6 */
	UINT8  ucGrpMemberCnt;
	UINT8  ucReserved_c;
	UINT16 u2Reserved_d;
	/* DW7 ~ DW10 */
	UINT16 au2StaList[TWT_GRP_MAX_MEMBER_CNT];
};
#endif /* APCLI_SUPPORT */
#endif /* MT_MAC */

#endif /* _TWT_H_ */
