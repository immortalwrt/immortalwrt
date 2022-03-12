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

	Module Name:
	veri_ctl.h

	Abstract:
	for verification mode related control materials.

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Carter Chen 02-April-2018    created

*/

#ifndef __VERI_CTL_H__
#define __VERI_CTL_H__

#define MAX_LEN_OF_VERI_BUF	1024
/*use a trick here, to indentify we will gen 802.3 packet.*/
#define VERI_PKT_DOT_3_TYPE	FC_TYPE_RSVED

struct _RTMP_ADAPTER;
struct wifi_dev;

enum ENUM_VERI_PKT_PREPARE_STATE_T {
	VERI_PKT_PREPARE_INIT,
	VERI_PKT_UPDATE_HEAD,
	VERI_PKT_UPDATE_CTNT,
};

enum ENUM_VERIFY_SWITCH_T {
	VERIFY_OFF = 0,
	VERIFY_ON,
};

#define UPDATE_VERI_PKT_STATE(_state)	(veri_ctrl->veri_pkt_state = _state)
#define CHECK_VERI_PKT_STATE(_state)	(veri_ctrl->veri_pkt_state != _state)

#define CLEAR_VERI_PKT_CTRL_IDX(_map, _idx)	(_map &= ~_idx)
#define SET_VERI_PKT_CTRL_IDX(_map, _idx)	(_map |= _idx)
#define CHECK_VERI_PKT_CTRL_IDX(_map, _idx)	(_map & _idx)

#define VERI_NULL_DATA		(1 << 0)
#define VERI_QOS_DATA		(1 << 1)
#define VERI_PM_CTRL_BY_SW	(1 << 2) /*PM ctrl by sw, need to assign pm bit.*/
#define VERI_SEQ_CTRL_BY_SW	(1 << 3) /*SN_VLD, need to assign seq.*/
#define VERI_DUR_CTRL_BY_SW	(1 << 4)
#define VERI_FM_CTRL		(1 << 5) /*0: TxD fixed rate mode. 1: WTBL rate index fixed rate mode*/
#define VERI_HDR_TRANS		(1 << 6)
#define VERI_NA			(1 << 7) /*no-ack*/
#define VERI_TM			(1 << 8) /*TMR*/
#define VERI_PN			(1 << 9) /*pn ctrl by sw, need to assign pn*/
#define VERI_TXS2M		(1 << 10)
#define VERI_TXS2H		(1 << 11)
#define VERI_LIFETIME_CTRL	(1 << 12)
#define VERI_HTC_CTRL		(1 << 13)

enum {
	VERI_FW_TEST = 1,
	/*END*/
};

struct veri_designated_ctrl {
	UINT32	assigned_pkt_htc;/*htc control field raw data*/
	USHORT  assigned_seq;/*dur in wifi hdr and txd*/
	USHORT	assigned_dur;/*dur in wifi hdr*/
	UCHAR	assigned_pid;/*pid in txd*/
	UCHAR	assigned_pm;/*pm in wifi hdr*/
	UCHAR	assigned_pkt_lifetime;/*life_time in txd, the unit is 8TU.*/
	/*UCHAR	assigned_pn[6];*/
};

struct veri_app_head_input {
	UINT32 veri_pkt_type;
	UINT32 veri_pkt_subtype;
	UCHAR addr1[MAC_ADDR_LEN];/*also could be DA of ether pkt*/
	UCHAR addr2[MAC_ADDR_LEN];/*also could be SA of ether pkt*/
	UCHAR addr3[MAC_ADDR_LEN];/*only 802.11 mgmt/data type use this addr.*/
};

struct veri_ctrl {
	UINT32 veri_pkt_ctrl_map;
	UINT32 veri_pkt_state;
	UINT32 veri_pkt_type;
	UINT32 veri_pkt_subtype;
	UINT32 veri_pkt_length;
	UCHAR addr1[MAC_ADDR_LEN];/*also could be DA of ether pkt*/
	UCHAR addr2[MAC_ADDR_LEN];/*also could be SA of ether pkt*/
	UCHAR addr3[MAC_ADDR_LEN];/*only 802.11 mgmt/data type use this addr.*/
	UCHAR veri_pkt_ctnt[MAX_LEN_OF_VERI_BUF];

	BOOLEAN verify_mode_on;
	BOOLEAN dump_rx_debug;
	BOOLEAN skip_ageout;
	struct veri_designated_ctrl assign_ctrl;
};

struct cmd_veri_set_fw {
	UCHAR sub_cmd;
	UCHAR field;
	UCHAR value;
};

INT prepare_veri_pkt_head(struct _RTMP_ADAPTER *ad, struct veri_app_head_input *head_input);
INT prepare_veri_pkt_ctnt(struct _RTMP_ADAPTER *ad, UCHAR *padded_ctnt, UINT32 ctnt_length);
INT prepare_veri_pkt_ctrl_en(struct _RTMP_ADAPTER *ad, UINT32 pkt_ctrl_map_input);
INT prepare_veri_pkt_ctrl_assign(struct _RTMP_ADAPTER *ad, struct veri_designated_ctrl *assign_ctrl_input);

INT set_veri_mode_enable(struct _RTMP_ADAPTER *ad, char *arg);
INT set_veri_pkt_ctrl_en(struct _RTMP_ADAPTER *ad, char *arg);
INT set_veri_pkt_ctrl_assign(struct _RTMP_ADAPTER *ad, char *arg);
INT set_dump_rx_debug(struct _RTMP_ADAPTER *ad, char *arg);
INT set_skip_ageout(struct _RTMP_ADAPTER *ad, char *arg);
INT set_veri_pkt_head(struct _RTMP_ADAPTER *ad, char *arg);
INT set_veri_pkt_ctnt(struct _RTMP_ADAPTER *ad, char *arg);
INT send_veri_pkt(struct _RTMP_ADAPTER *ad, char *arg);
INT veri_mode_switch(struct _RTMP_ADAPTER *ad, char *arg);
INT ecc_calculate_test(struct _RTMP_ADAPTER *ad, char *arg);

INT32 verify_pkt_tx(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
#endif /* __VERI_CTL_H__ */

