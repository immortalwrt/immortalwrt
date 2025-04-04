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
	ra_ac_q_mgmt.h
*/

#include    "rt_config.h"

#ifndef _DABS_QOS_H_
#define _DABS_QOS_H_

#ifdef DABS_QOS
#define NO_MIC 0
#define MIC_AP 1
#define MIC_STA 2

typedef enum _FAST_PATH_CMD_T {
	FAST_PATH_CMD_GET_CAP = 1,
	FAST_PATH_CMD_CAL_MIC = 2,
	FAST_PATH_CMD_CAL_MIC_TEST_MODE = 3
} FAST_PATH_CMD;
extern const UINT8  up_to_ac_mapping[8];
#define DABS_DBG_PRN_LV1	(1 << 0)
#define DABS_DBG_PRN_LV2	(1 << 1)
#define DABS_SET_QOS_PARAM	(1 << 2)
#define DABS_DBG_DLY_TIME	(1 << 3)
#define DABS_DBG_PRN_LV3	(1 << 4)


#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#define NUM_OF_8021D_COS        8

#define MAX_QOS_PARAM_TBL 128

enum app_type_def {
	GAMING = 0,
	VR,
	CLOUD_VR,
	CLOUD_GAMING,
	VOIP,
	VIDEO_CONFERENCE,
	VIDEO_STREAM,
	IPTV,
	UNKONWN,
	MAX_APP_TYPE
};

struct qos_param_rec {
	unsigned short wlan_idx;
	unsigned int ip_src;
	unsigned int ip_dest;
	unsigned short sport;
	unsigned short dport;
	unsigned short protocol;
	unsigned char priority;
	unsigned char force_ac;
	unsigned short delay_bound; /* drop packet with delay above max delay bound */
	unsigned short delay_req; /* for SmartQoS scheduler use */
	unsigned char delay_weight;
	unsigned int data_rate; /* application data rate */
	unsigned short bw_req;
	unsigned char dir;	/* DL or UL */
	unsigned short drop_thres;
	enum app_type_def app_type;
	bool valid;
	unsigned long long tot_pkt_dly;	/* usec */
	unsigned int tot_pkt_cnt;
	unsigned int some_pkt_dly;
	unsigned int avg_pkt_dly;
	unsigned int max_pkt_dly;
	bool in_use;
};

struct qos_param_rec_add {
	unsigned char dest_mac[6];
	unsigned int ip_src;
	unsigned int ip_dest;
	unsigned short sport;
	unsigned short dport;
	unsigned short protocol;
	unsigned char priority;
	unsigned char force_ac;
	unsigned short delay_bound;
	unsigned short delay_req;
	unsigned char delay_weight;
	unsigned int data_rate;
	unsigned short bw_req;
	unsigned char dir;
	unsigned short drop_thres;
	enum app_type_def app_type;
};

struct qos_param_rec_del {
	unsigned int ip_src;
	unsigned int ip_dest;
	unsigned short sport;
	unsigned short dport;
	unsigned short protocol;
};
#ifdef DABS_QOS
struct qos_param_set_del {
	UINT32 idx;
	BOOLEAN sel_del;
	MURU_QOS_SETTING qos_setting;
};
#endif
INT set_dabs_qos_param(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
unsigned short search_qos_param_tbl_idx_by_5_tuple(PRTMP_ADAPTER pAd, struct sk_buff *skb);
unsigned short ioctl_search_qos_param_tbl_idx_by_5_tuple(PRTMP_ADAPTER pAd, VOID *qos_param_rec_pointer, BOOLEAN add);

unsigned short search_free_qos_param_tbl_idx(PRTMP_ADAPTER pAd);
unsigned short search_qos_param_tbl_idx_by_wlan_idx(unsigned short wlan_idx,
	unsigned short proto, unsigned short dport);
extern struct qos_param_rec qos_param_table[MAX_QOS_PARAM_TBL];
bool set_qos_param_tbl_wlan_idx_by_idx(unsigned short idx, unsigned short wlan_idx);
bool set_qos_param_to_fw(PRTMP_ADAPTER pAd, struct qos_param_set_del *pqos_param_set_del, BOOLEAN set_del);
bool update_qos_param(PRTMP_ADAPTER pAd, UINT32 idx, struct qos_param_rec_add *qos_param_rec_add);
bool delete_qos_param(PRTMP_ADAPTER pAd, UINT32 idx);
bool enable_qos_param_tbl_by_idx(unsigned short idx);
bool disable_qos_param_tbl_by_idx(unsigned short idx);
void dabs_host_delay(PRTMP_ADAPTER pAd, NDIS_PACKET *pkt);
void dabs_active_qos_by_ipaddr(PRTMP_ADAPTER pAd, NDIS_PACKET *pkt);
INT Show_qos_dabs_table_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID ExtEventFastPathRptHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
extern NDIS_SPIN_LOCK qos_param_table_lock;
unsigned short delete_qos_param_tbl_by_wlan_idx(PRTMP_ADAPTER pAd, unsigned short wlan_idx, struct wifi_dev *wdev);
BOOLEAN FastPathCheckMIC(RTMP_ADAPTER *pAd, UINT_8 ucOpMode, UINT_16 u2WlanId,
	UINT_16 u2RandomNum, UINT_8 mic_type, UINT_16 u2mic, UINT_32 *au4Keybitmap, EVENT_FAST_PATH_T *event_fastpath);
#endif /* DABS_QOS */
#endif /* _DABS_QOS_H_ */


