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
/****************************************************************************
 ***************************************************************************

	Module Name:
	dabs_qos.c
*/

#include "rt_config.h"
#include "mcu/mt_cmd.h"

#ifdef DABS_QOS

#include <net/ip.h>
#include "ra_ac_q_mgmt.h"

static const UINT8 ac_queue_to_up[WMM_NUM_OF_AC] = {
	1 /* AC_BK */, 0 /* AC_BE */, 5 /* AC_VI */, 7 /* AC_VO */
};
const UINT8  up_to_ac_mapping[NUM_OF_8021D_COS] = {
	WMM_AC_BE /* UP=0 */,   WMM_AC_BK /* UP=1 */,   WMM_AC_BK /* UP=2 */,   WMM_AC_BE /* UP=3 */,
	WMM_AC_VI /* UP=4 */,   WMM_AC_VI /* UP=5 */,   WMM_AC_VO /* UP=6 */,   WMM_AC_VO /* UP=7 */
};

struct qos_param_rec qos_param_table[MAX_QOS_PARAM_TBL];

NDIS_SPIN_LOCK qos_param_table_lock;
#define RTMP_SET_PACKET_QOS_IDX(_p, _idx) (*(UINT8 *)&PACKET_CB(_p, 32) = _idx)
#define RTMP_GET_PACKET_QOS_IDX(_p)                (*(UINT8 *)&PACKET_CB(_p, 32))
UINT32	get_min_delaybound_by_ac(unsigned short wlan_idx, unsigned short priority)
{
	unsigned short idx;
	unsigned short the_idx = MAX_QOS_PARAM_TBL;
	struct qos_param_rec *prec;
	unsigned short tmp_delayreq = 0xFFFF;

	for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
		prec = &qos_param_table[idx];
		if (prec->in_use == FALSE)
			continue;
		if (up_to_ac_mapping[prec->priority] != up_to_ac_mapping[priority])
			continue;
		if (prec->wlan_idx != wlan_idx) {
			continue;
		}
			if (prec->delay_req < tmp_delayreq) {
			 tmp_delayreq = prec->delay_req;
			 the_idx = idx;
			}
	}

	return the_idx;
}

unsigned short search_qos_param_tbl_idx_by_5_tuple(PRTMP_ADAPTER pAd, struct sk_buff *skb)
{
	struct iphdr *hdr = (struct iphdr *)(skb->data + MAT_ETHER_HDR_LEN);
	unsigned short idx;
	unsigned short the_idx = MAX_QOS_PARAM_TBL;
	struct qos_param_rec *prec;

	for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
		prec = &qos_param_table[idx];

		if ((prec->ip_src == 0) || (prec->ip_dest == 0))
			continue;
		if ((prec->ip_src != hdr->saddr) || (prec->ip_dest != hdr->daddr))
			continue;
		if (prec->protocol != hdr->protocol)
			continue;

		if (prec->protocol == IPPROTO_TCP) {
			if ((prec->sport > 0) && (prec->dport > 0)) {
				struct tcphdr *tcph = (struct tcphdr *)((UINT8*)hdr) + 20;
				if ((prec->sport != tcph->source) ||
					(prec->dport != tcph->dest))
					continue;
			}
		}
		else if (prec->protocol == IPPROTO_UDP) {
			if ((prec->sport > 0) && (prec->dport > 0)) {
				struct udphdr *udph = (struct udphdr *)(((UINT8*)hdr) + 20);
				UINT8* pdata = ((UINT8*)hdr) + 20;

				if ((prec->sport != udph->source) ||
					(prec->dport != udph->dest)) {
					if (pAd->dabs_qos_op & DABS_DBG_PRN_LV1)
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"[%d]dest_ip=%u.%u.%u.%u=port=(%u,%u)!=(%u,%u),\
						(%02X:%02X:%02X%02X)\n",
						idx, NIPQUAD(hdr->daddr),
					 	ntohs(prec->sport), ntohs(prec->dport),
						ntohs(udph->source), ntohs(udph->dest),
						pdata[2], pdata[3],pdata[4],pdata[5]);
					continue;
				}	
			}	
		}

		the_idx = idx;
		break;
	}

	return the_idx;
}

unsigned short ioctl_search_qos_param_tbl_idx_by_5_tuple(PRTMP_ADAPTER pAd, VOID *qos_param_rec_pointer, BOOLEAN add)
{
	unsigned short idx;
	unsigned short the_idx = MAX_QOS_PARAM_TBL;
	struct qos_param_rec *prec = NULL;
	struct qos_param_rec_add *qos_param_rec_add = NULL;
	struct qos_param_rec_del *qos_param_rec_del = NULL;

	if (add == TRUE) {
		qos_param_rec_add = (struct qos_param_rec_add *)qos_param_rec_pointer;
		for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
			prec = &qos_param_table[idx];

			if ((prec->ip_src == 0) || (prec->ip_dest == 0))
				continue;
			if ((prec->ip_src != qos_param_rec_add->ip_src) ||
				(prec->ip_dest != qos_param_rec_add->ip_dest))
				continue;
			if (prec->protocol != qos_param_rec_add->protocol)
				continue;
			if ((prec->sport != htons(qos_param_rec_add->sport)) ||
				(prec->dport != htons(qos_param_rec_add->dport)))
				continue;

			the_idx = idx;
			break;
		}
	} else {
		qos_param_rec_del = (struct qos_param_rec_del *)qos_param_rec_pointer;

		for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
			prec = &qos_param_table[idx];

			if ((prec->ip_src == 0) || (prec->ip_dest == 0))
				continue;
			if ((prec->ip_src != qos_param_rec_del->ip_src) ||
				(prec->ip_dest != qos_param_rec_del->ip_dest))
				continue;
			if (prec->protocol != qos_param_rec_del->protocol)
				continue;
			if ((prec->sport != htons(qos_param_rec_del->sport)) ||
				(prec->dport != htons(qos_param_rec_del->dport)))
				continue;

			the_idx = idx;
			break;
		}
	}

	return the_idx;
}

unsigned short search_free_qos_param_tbl_idx(PRTMP_ADAPTER pAd)
{
	unsigned short idx;
	unsigned short the_idx = MAX_QOS_PARAM_TBL;
	struct qos_param_rec *prec;

	for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
		prec = &qos_param_table[idx];

		if (prec->in_use == TRUE)
			continue;

		the_idx = idx;
		break;
	}

	return the_idx;
}


unsigned short search_qos_param_tbl_idx_by_wlan_idx(
	unsigned short wlan_idx,
	unsigned short proto,
	unsigned short dport)
{
	unsigned short idx;
	unsigned short the_idx = MAX_QOS_PARAM_TBL;
	struct qos_param_rec *prec;

	for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
		prec = &qos_param_table[idx];
		if (prec->valid == FALSE)
			continue;

		if ((prec->protocol != proto) && (prec->protocol != 0))
			continue;

		if ((prec->dport != dport) && (prec->dport != 0))
			continue;

		if (prec->wlan_idx == wlan_idx) {
			the_idx = idx;
			break;
		}
	}

	return the_idx;
}
bool set_qos_param_tbl_wlan_idx_by_idx(unsigned short idx, unsigned short wlan_idx)
{
	bool ret = true;
	struct qos_param_rec *ptarget_qos_param;

	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	ptarget_qos_param = &qos_param_table[idx];
	ptarget_qos_param->wlan_idx = wlan_idx;
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	return ret;
}

bool set_qos_param_tbl_ipaddr_by_idx(
	unsigned short idx, 
	struct qos_param_rec *pqos_param
)
{
	bool ret = true;
	struct qos_param_rec *ptarget_qos_param;
	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	ptarget_qos_param = &qos_param_table[idx];

	ptarget_qos_param->ip_src = pqos_param->ip_src;
	ptarget_qos_param->ip_dest = pqos_param->ip_dest;
	ptarget_qos_param->protocol = pqos_param->protocol;
	ptarget_qos_param->sport = pqos_param->sport;
	ptarget_qos_param->dport = pqos_param->dport;
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	return ret;
}

bool set_qos_param_tbl_qos_by_idx(unsigned short idx, struct qos_param_rec *pqos_param)
{
	bool ret = true;
	struct qos_param_rec *ptarget_qos_param;
	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	ptarget_qos_param = &qos_param_table[idx];

	ptarget_qos_param->priority = ac_queue_to_up[pqos_param->priority];
	ptarget_qos_param->force_ac = pqos_param->force_ac;
	ptarget_qos_param->delay_bound = pqos_param->delay_bound;
	ptarget_qos_param->delay_req = pqos_param->delay_req;
	ptarget_qos_param->delay_weight = pqos_param->delay_weight;
	ptarget_qos_param->bw_req = pqos_param->bw_req;
	ptarget_qos_param->data_rate = pqos_param->data_rate;
	ptarget_qos_param->dir = pqos_param->dir;
	ptarget_qos_param->drop_thres = pqos_param->drop_thres;
	ptarget_qos_param->app_type = pqos_param->app_type;
/*
	if (ptarget_qos_param->delay_req <= 10)
		ptarget_qos_param->priority = ac_queue_to_up[3];
	else if (ptarget_qos_param->delay_req <= 50)
		ptarget_qos_param->priority = ac_queue_to_up[2];
	else if (ptarget_qos_param->delay_req <= 500)
		ptarget_qos_param->priority = ac_queue_to_up[1];
	else
		ptarget_qos_param->priority = ac_queue_to_up[0];
*/	
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Set qos: prio:%u,force_ac:%u,delay_bound:%u,delay_req:%u,weight=%u,data_rate:%u,dir:%u,dropth:%u,app_type:%u\n",
		ptarget_qos_param->priority,
		ptarget_qos_param->force_ac,
		ptarget_qos_param->delay_bound,
		ptarget_qos_param->delay_req,
		ptarget_qos_param->delay_weight,
		ptarget_qos_param->data_rate,
		ptarget_qos_param->dir,
		ptarget_qos_param->drop_thres,
		ptarget_qos_param->app_type
		);
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	return ret;
}

bool set_qos_param_tbl_by_idx(unsigned short idx, struct qos_param_rec *pqos_param)
{
	bool ret = true;

	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	memcpy(&qos_param_table[idx], pqos_param, sizeof(struct qos_param_rec));
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	return ret;
}
unsigned short delete_qos_param_tbl_by_wlan_idx(
	PRTMP_ADAPTER pAd, unsigned short wlan_idx, struct wifi_dev *wdev)
{
	unsigned short idx;
	struct qos_param_rec *prec;
#ifdef QOS_R1
#ifdef MSCS_PROPRIETARY
	struct wapp_vend_spec_classifier_para_report vend_spec_classifier_param;
	MAC_TABLE_ENTRY *pEntry = NULL;
#endif
#endif
	for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
		prec = &qos_param_table[idx];
		if (prec->in_use == FALSE)
			continue;
		if (prec->wlan_idx != wlan_idx)
			continue;
#ifdef QOS_R1
#ifdef MSCS_PROPRIETARY
		os_zero_mem(&vend_spec_classifier_param, sizeof(struct wapp_vend_spec_classifier_para_report));
		pEntry = &pAd->MacTab.Content[wlan_idx];
		vend_spec_classifier_param.id = idx;
		COPY_MAC_ADDR(vend_spec_classifier_param.sta_mac, (UCHAR *)pEntry->Addr);
		vend_spec_classifier_param.destIp.ipv4 = prec->ip_dest;
		vend_spec_classifier_param.srcIp.ipv4 = prec->ip_src;
		vend_spec_classifier_param.destPort = prec->sport;
		vend_spec_classifier_param.srcPort = prec->dport;
		vend_spec_classifier_param.protocol = prec->protocol;
		vend_spec_classifier_param.requet_type = SCS_REQ_TYPE_REMOVE;
		wext_send_vendor_spec_tclas_elment(wdev, (UCHAR *)&vend_spec_classifier_param,
			sizeof(vend_spec_classifier_param));
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "removed, id:%d, mac:%pM\n",
						idx, pEntry->Addr);
#endif
#endif
		delete_qos_param(pAd, idx);
	}
	return TRUE;
}
bool update_qos_param(PRTMP_ADAPTER pAd, UINT32 idx, struct qos_param_rec_add *qos_param_rec_add)
{
	bool ret = TRUE;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct qos_param_rec *pqos_param;
	UINT32 min_delaybound_idx = 0;

	pEntry = MacTableLookup(pAd, qos_param_rec_add->dest_mac);
	if (pEntry == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"pEntry == NULL!!, (DA = %02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(qos_param_rec_add->dest_mac));
		return FALSE;
	}

	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	pqos_param = &qos_param_table[idx];
	memset(pqos_param, 0, sizeof(struct qos_param_rec));
	pqos_param->wlan_idx = pEntry->wcid;
	pqos_param->ip_src = qos_param_rec_add->ip_src;
	pqos_param->ip_dest = qos_param_rec_add->ip_dest;
	pqos_param->sport = htons(qos_param_rec_add->sport);
	pqos_param->dport = htons(qos_param_rec_add->dport);
	pqos_param->protocol = qos_param_rec_add->protocol;
	pqos_param->priority = qos_param_rec_add->priority;
	pqos_param->force_ac = qos_param_rec_add->force_ac;
	pqos_param->delay_bound = qos_param_rec_add->delay_bound;
	pqos_param->delay_req = qos_param_rec_add->delay_req;
	pqos_param->delay_weight = qos_param_rec_add->delay_weight;
	pqos_param->bw_req = qos_param_rec_add->bw_req;
	pqos_param->data_rate = qos_param_rec_add->data_rate;
	pqos_param->dir = qos_param_rec_add->dir;
	pqos_param->drop_thres = qos_param_rec_add->drop_thres;
	pqos_param->app_type = qos_param_rec_add->app_type;
	pqos_param->in_use = TRUE;
	pqos_param->valid = TRUE;
	min_delaybound_idx = get_min_delaybound_by_ac(pEntry->wcid, qos_param_rec_add->priority);
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
	if (min_delaybound_idx == idx) {
		if (HW_UPDATE_QOS_PARAM(pAd, idx, TRUE) != NDIS_STATUS_SUCCESS)	{
			disable_qos_param_tbl_by_idx(idx);
			ret = FALSE;
		}
	}
	return ret;
}

bool delete_qos_param(PRTMP_ADAPTER pAd, UINT32 idx)
{
	bool ret = TRUE;
	UINT32 min_delaybound_idx = 0;
	struct qos_param_rec *pqos_param = NULL;
	unsigned short wlan_idx;
	unsigned short priority;

	if(idx >= MAX_QOS_PARAM_TBL)
		return FALSE;

	pqos_param = &qos_param_table[idx];
	wlan_idx = pqos_param->wlan_idx;
	priority = pqos_param->priority;
	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	pqos_param->in_use = FALSE;
	min_delaybound_idx = get_min_delaybound_by_ac(wlan_idx, priority);
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"priority = %d ,min_delaybound_idx=%d!!\n", priority, min_delaybound_idx);

	if (min_delaybound_idx == MAX_QOS_PARAM_TBL) {
		if (HW_UPDATE_QOS_PARAM(pAd, idx, FALSE) != NDIS_STATUS_SUCCESS)
			ret = FALSE;
	} else if (pqos_param->delay_req < qos_param_table[min_delaybound_idx].delay_req) {
		if (HW_UPDATE_QOS_PARAM(pAd, min_delaybound_idx, TRUE) != NDIS_STATUS_SUCCESS)
			ret = FALSE;
	}
		OS_SPIN_LOCK_BH(&qos_param_table_lock);
		memset(pqos_param, 0, sizeof(struct qos_param_rec));
		OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
	return ret;
}

bool enable_qos_param_tbl_by_idx(unsigned short idx)
{
	bool ret = true;

	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	qos_param_table[idx].valid = true;
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	return ret;
}

bool disable_qos_param_tbl_by_idx(unsigned short idx)
{
	bool ret = true;

	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	qos_param_table[idx].valid = false;
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	return ret;
}

bool set_qos_param_to_fw(PRTMP_ADAPTER pAd, struct qos_param_set_del *pqos_param_set_del, BOOLEAN set_del)
{
	bool ret = true;
	MURU_QOS_SETTING qos_setting = {0};
	USHORT idx = pqos_param_set_del->idx;
	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	memcpy(&qos_setting, &pqos_param_set_del->qos_setting, sizeof(MURU_QOS_SETTING));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "idx(%d)delay_req%d\n", idx, qos_setting.u2DelayReq);
	/*qos_setting.u2WlanIdx = pqos_param->wlan_idx;
	qos_setting.u1AC = up_to_ac_mapping[pqos_param->priority];
	qos_setting.u1ForceAC = pqos_param->force_ac;
	qos_setting.u2DelayBound = pqos_param->delay_bound;
	qos_setting.u2DelayReq = pqos_param->delay_req;
	qos_setting.u1DelayWeight = pqos_param->delay_weight;
	qos_setting.u2DataRate = pqos_param->data_rate;
	qos_setting.u2BWReq = pqos_param->bw_req;
	qos_setting.u1Dir = pqos_param->dir;
	qos_setting.u2DropThres = pqos_param->drop_thres;
	qos_setting.u1Idx = idx;*/

	if (set_del == TRUE) {
		if (SendQoSCmd(pAd, QOS_CMD_PARAM_SETTING, &qos_setting) == false) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"SendQoSCmd fail!!!\n");
			ret = false;
		}
	} else {
		if (SendQoSCmd(pAd, QOS_CMD_PARAM_DELETE, &qos_setting) == false) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"SendQoSCmd fail!!!\n");
			ret = false;
		}
	}

	return ret;
}

INT set_dabs_qos_param(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 cmd = 3, rv, ret = 1, idx = 0;
	UINT32 param[13] = {0};
	UINT32 ip_src[4] = {0}, ip_dest[4] = {0};
	MURU_QOS_SETTING qos_setting = {0};
	struct qos_param_rec target_qos_param, *pqos_param;
	if (arg) {
		rv = sscanf(arg, "%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u", &cmd, &param[0], &param[1], &param[2],
			&param[3], &param[4], &param[5], &param[6], &param[7], &param[8], &param[9], &param[10],
			&param[11]);

		if (rv == 0) {
			ret = 0;
			goto error;
		}
		switch (cmd) {
		case 0:
			rv = sscanf(arg, "%u-%u-%u.%u.%u.%u-%u.%u.%u.%u-%u-%u-%u",
				&cmd, &param[0],
				&ip_src[0], &ip_src[1], &ip_src[2], &ip_src[3],
				&ip_dest[0], &ip_dest[1], &ip_dest[2], &ip_dest[3],
				&param[3], &param[4], &param[5]);
			if (rv == 0) {
				ret = 0;
				goto error;
			}

			param[1] = (ip_src[3]<<24) | (ip_src[2] << 16) | (ip_src[1] << 8)
				| (ip_src[0] << 0);
			param[2] = (ip_dest[3]<<24) | (ip_dest[2] << 16) | (ip_dest[1] << 8)
				| (ip_dest[0] << 0);

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"ip_src=%u.%u.%u.%u, ip_dest=%u.%u.%u.%u\n",
				NIPQUAD(param[1]), NIPQUAD(param[2]));

			target_qos_param.ip_src = param[1];
			target_qos_param.ip_dest = param[2];
			target_qos_param.protocol = param[3];
			target_qos_param.sport = htons(param[4]);
			target_qos_param.dport = htons(param[5]);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"ip_src=%u.%u.%u.%u, ip_dest=%u.%u.%u.%u,proto=%u,sp=%u,dp=%u\n",
				NIPQUAD(target_qos_param.ip_src),
				NIPQUAD(target_qos_param.ip_dest),
				target_qos_param.protocol,
				ntohs(target_qos_param.sport),
				ntohs(target_qos_param.dport));


			set_qos_param_tbl_ipaddr_by_idx(param[0], &target_qos_param);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"set qos_param ipaddr for idx:%u\n", param[0]);
			break;
		case 1:
			target_qos_param.priority = param[2]; /* up to ac by set_qos_param_tbl_qos_by_idx */
			target_qos_param.delay_bound = param[3];
			target_qos_param.delay_req = param[4];
			target_qos_param.delay_weight = param[5];
			target_qos_param.data_rate = param[6];
			target_qos_param.bw_req = param[7];
			target_qos_param.dir = param[8];
			target_qos_param.drop_thres = param[9];
			target_qos_param.app_type = param[10];
			target_qos_param.force_ac = param[11];
			set_qos_param_tbl_qos_by_idx(param[0], &target_qos_param);
			if (param[1] > 0)
				set_qos_param_tbl_wlan_idx_by_idx(param[0], param[1]);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"set qos_param qos related for idx:%u\n", param[0]);
			if ((target_qos_param.delay_req > 0) && (target_qos_param.delay_req < 65535)) {
				if (pAd->pbc_bound[DBDC_BAND0][PBC_AC_BK] == PBC_WMM_UP_DEFAULT_BK)
				{
					pAd->pbc_bound[DBDC_BAND0][PBC_AC_VO] = 1200;
					pAd->pbc_bound[DBDC_BAND0][PBC_AC_VI] = 4000;
					pAd->pbc_bound[DBDC_BAND0][PBC_AC_BE] = 1200;
					pAd->pbc_bound[DBDC_BAND0][PBC_AC_BK] = 1000;

					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"%s: set pbc_ubound=[%u,%u,%u,%u]\n", __func__,
						pAd->pbc_bound[DBDC_BAND0][PBC_AC_BK], pAd->pbc_bound[DBDC_BAND0][PBC_AC_BE],
						pAd->pbc_bound[DBDC_BAND0][PBC_AC_VI], pAd->pbc_bound[DBDC_BAND0][PBC_AC_VO]);
				}
#ifdef DBDC_MODE
				if (pAd->pbc_bound[DBDC_BAND1][PBC_AC_BK] == PBC_WMM_UP_DEFAULT_BK) {
					pAd->pbc_bound[DBDC_BAND1][PBC_AC_VO] = 1200;
					pAd->pbc_bound[DBDC_BAND1][PBC_AC_VI] = 4000;
					pAd->pbc_bound[DBDC_BAND1][PBC_AC_BE] = 1200;
					pAd->pbc_bound[DBDC_BAND1][PBC_AC_BK] = 1000;

					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"%s: set pbc_ubound=[%u,%u,%u,%u]\n", __func__,
						pAd->pbc_bound[DBDC_BAND1][PBC_AC_BK], pAd->pbc_bound[DBDC_BAND1][PBC_AC_BE],
						pAd->pbc_bound[DBDC_BAND1][PBC_AC_VI], pAd->pbc_bound[DBDC_BAND1][PBC_AC_VO]);
				}

#endif
			}
			break;
		case 2:
			set_qos_param_tbl_wlan_idx_by_idx(param[0], param[1]);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"set qos_param wlan:%u related for idx:%u\n", param[1], param[0]);
			break;
		case 3:
			if (param[1] != 0) {
				enable_qos_param_tbl_by_idx(param[0]);
				if (HW_UPDATE_QOS_PARAM(pAd, param[0], TRUE) == FALSE) {
					ret = 0;
					disable_qos_param_tbl_by_idx(param[0]);
				}
			} else {
				HW_UPDATE_QOS_PARAM(pAd, param[0], FALSE);

				disable_qos_param_tbl_by_idx(param[0]);
			}
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"qos_param tbl[%d] valid:%u\n", param[0],param[1]);
			break;
		case 4:
			for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
				pqos_param = &qos_param_table[idx];
				if (pqos_param->ip_dest == 0)
					continue;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
"dump qos_param[%d]: ip_src=%u.%u.%u.%u,ip_dest=%u.%u.%u.%u,proto=%u,sport=%u,dport=%u\n",
				idx,
			NIPQUAD(pqos_param->ip_src),
			NIPQUAD(pqos_param->ip_dest),
			pqos_param->protocol,
			ntohs(pqos_param->sport),
			ntohs(pqos_param->dport)
			);

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"dump qos_param[%d]: valid:%u,prio:%u,force_ac:%u,delay_bound:%u,delay_req:%u,delay_weight:%u,data_rate:%u,bw_req:%u,dir=%u,dropth=%u,app_type:%u,in_use=%u\n",
			param[0],
			pqos_param->valid,
			pqos_param->priority,
			pqos_param->force_ac,
			pqos_param->delay_bound,
			pqos_param->delay_req,
			pqos_param->delay_weight,
			pqos_param->data_rate,
			pqos_param->bw_req,
			pqos_param->dir,
			pqos_param->drop_thres,
			pqos_param->app_type,
			pqos_param->in_use
			);
			}	
			SendQoSCmd(pAd, QOS_CMD_PARAM_DUMP, &qos_setting);
			break;
		case 5:
			pAd->dabs_qos_op = param[0];
			if (pAd->dabs_qos_op & DABS_DBG_DLY_TIME)
				net_enable_timestamp();
			else
	                        net_disable_timestamp();
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"dabs_qos_op:%u\n", param[0]);
			break;
		case 6:
			ret = SendQoSCmd(pAd, QOS_CMD_PARAM_RESET, &qos_setting);
			OS_SPIN_LOCK_BH(&qos_param_table_lock);
			memset(&qos_param_table[0], 0, sizeof(struct qos_param_rec)*MAX_QOS_PARAM_TBL);
			OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
			pAd->pbc_bound[DBDC_BAND0][PBC_AC_BE] = PBC_WMM_UP_DEFAULT_BE_BAND0;
			pAd->pbc_bound[DBDC_BAND0][PBC_AC_BK] = PBC_WMM_UP_DEFAULT_BK_BAND0;
			pAd->pbc_bound[DBDC_BAND0][PBC_AC_VO] = PBC_WMM_UP_DEFAULT_VO_BAND0;
			pAd->pbc_bound[DBDC_BAND0][PBC_AC_VI] = PBC_WMM_UP_DEFAULT_VI_BAND0;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s: clean up table ret=%u\n", __func__, ret);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s: reset pbc_ubound=[%u,%u,%u,%u]\n", __func__,
				pAd->pbc_bound[DBDC_BAND0][PBC_AC_BK], pAd->pbc_bound[DBDC_BAND0][PBC_AC_BE],
				pAd->pbc_bound[DBDC_BAND0][PBC_AC_VI], pAd->pbc_bound[DBDC_BAND0][PBC_AC_VO]);
#ifdef DBDC_MODE
			pAd->pbc_bound[DBDC_BAND1][PBC_AC_BE] = PBC_WMM_UP_DEFAULT_BE;
			pAd->pbc_bound[DBDC_BAND1][PBC_AC_BK] = PBC_WMM_UP_DEFAULT_BK;
			pAd->pbc_bound[DBDC_BAND1][PBC_AC_VO] = PBC_WMM_UP_DEFAULT_VO;
			pAd->pbc_bound[DBDC_BAND1][PBC_AC_VI] = PBC_WMM_UP_DEFAULT_VI;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s: clean up table ret=%u\n", __func__, ret);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s: reset pbc_ubound=[%u,%u,%u,%u]\n", __func__,
				pAd->pbc_bound[DBDC_BAND1][PBC_AC_BK], pAd->pbc_bound[DBDC_BAND1][PBC_AC_BE],
				pAd->pbc_bound[DBDC_BAND1][PBC_AC_VI], pAd->pbc_bound[DBDC_BAND1][PBC_AC_VO]);

#endif
			break;
		case 7:
			for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
				pqos_param = &qos_param_table[idx];
				if (pqos_param->tot_pkt_cnt > 0) {
					unsigned int avg_dly;
					avg_dly = div64_ul(pqos_param->tot_pkt_dly, pqos_param->tot_pkt_cnt);	
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"qos[%d]:dly_req:%u,avg_dly:%u(max=%u)[%llu/%u]\n", idx,
						pqos_param->delay_req,
						avg_dly,
						pqos_param->max_pkt_dly,
						pqos_param->tot_pkt_dly,
						pqos_param->tot_pkt_cnt);
					pqos_param->tot_pkt_dly = 0;
					pqos_param->tot_pkt_cnt = 0;
					pqos_param->max_pkt_dly = 0;
				}
			}
			SendQoSCmd(pAd, QOS_CMD_RESULT_DUMP, &qos_setting);
			break;
		case 8:
			{
				UINT_32 au4Keybitmap[4] = {0};
				EVENT_FAST_PATH_T event_fastpath;

				NdisZeroMemory(&event_fastpath, sizeof(EVENT_FAST_PATH_T));

				FastPathCheckMIC(pAd, FAST_PATH_CMD_CAL_MIC_TEST_MODE, 0, 0, MIC_AP, 0,
						au4Keybitmap, &event_fastpath);
			}
			break;
		default:
			ret = 0;
			goto error;
		}
	}

error:

	if (ret == 0) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"iwpriv ra0 set dabs_qos=0-[idx]-[ip_src]-[ip_dest]-[proto]-[sport]-[dport] (set ip by idx)\n");
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"iwpriv ra0 set dabs_qos=1-[idx]-[wlanid]-[AC]-[DlyBound]-[DlyReq]-[DlyWt]-[Rate]-[BWReq]-[Dir]-[DropTh]-[app]-[ForceAC] (set qos by idx)\n");
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"iwpriv ra0 set dabs_qos=4 (dump table)\n");
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"iwpriv ra0 set dabs_qos=5-[dabs_qos_op(bit2:] (set dabs operation)\n");
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"iwpriv ra0 set dabs_qos=6 (clean up table)\n");
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"iwpriv ra0 set dabs_qos=7 (dump host delay)\n");
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "\n");


	return ret;
}

void dabs_host_delay(PRTMP_ADAPTER pAd,  NDIS_PACKET *pkt)
{
	UINT16 qos_idx;

	if (pkt == NULL)
		return;

	qos_idx = RTMP_GET_PACKET_QOS_IDX(pkt);

	if (qos_idx < MAX_QOS_PARAM_TBL) {
		struct qos_param_rec *prec;
		prec = &qos_param_table[qos_idx];
		if (prec->valid == TRUE) {
			UINT32 delta_us = 0;
#ifdef PROPRIETARY_DRIVER_SUPPORT
			struct timespec64 kts64 = {0};
			ktime_t kts, delta_kt;

			ktime_get_real_ts64(&kts64);
			kts = timespec64_to_ktime(kts64);
			delta_kt = ktime_sub(kts, RTPKT_TO_OSPKT(pkt)->tstamp);
#else
			ktime_t delta_kt = net_timedelta(RTPKT_TO_OSPKT(pkt)->tstamp);
#endif
			delta_us = ktime_to_us(delta_kt);

			prec->tot_pkt_cnt++;
			prec->tot_pkt_dly += delta_us;

			if (delta_us > prec->max_pkt_dly) {
				prec->max_pkt_dly = delta_us;
			}

			if (pAd->dabs_qos_op & DABS_DBG_PRN_LV3) {
				prec->some_pkt_dly += delta_us;
				if ((prec->tot_pkt_cnt % 1000) == 999) {
					prec->avg_pkt_dly =  prec->some_pkt_dly/1000;
					prec->some_pkt_dly = 0;
				}
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"qos[%d] deltaus=%u\n",qos_idx, delta_us);
			}
		}
	}

	return;
}


void dabs_active_qos_by_ipaddr(PRTMP_ADAPTER pAd,  NDIS_PACKET *pkt)
{
	UINT32 idx = MAX_QOS_PARAM_TBL;
	UINT8 *pData = GET_OS_PKT_DATAPTR(pkt);
	UINT32 ip_dest, ip_src, proto, sport = 0,dport = 0;

	pData += (MAT_ETHER_HDR_LEN);
	ip_src = *((UINT32*)(pData + 12));
	ip_dest = *((UINT32*)(pData + 16));
	proto = *((UINT8*)(pData + 9));

	if ((ip_src != 0) && (ip_dest != 0) && ((ip_dest & 0x0FF) != 0x0FF)) {
		if (RTMP_GET_PACKET_WCID(pkt) < (MAX_LEN_OF_MAC_TABLE - 16)) {
			idx = search_qos_param_tbl_idx_by_5_tuple(pAd, RTPKT_TO_OSPKT(pkt));
			if (pAd->dabs_qos_op & DABS_DBG_PRN_LV3) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WCID%u,hit=%u src=%u.%u.%u.%u,dest=%u.%u.%u.%u, proto=%u,sp=%u,dp=%u\n",
					RTMP_GET_PACKET_WCID(pkt), idx,
					NIPQUAD(ip_src), NIPQUAD(ip_dest), proto, sport, dport);
			}
		}
	}

	RTMP_SET_PACKET_QOS_IDX(pkt, MAX_QOS_PARAM_TBL);

	if (idx < MAX_QOS_PARAM_TBL) {
		struct qos_param_rec *prec;
		prec = &qos_param_table[idx];

		if (prec->valid == FALSE) {
			set_qos_param_tbl_wlan_idx_by_idx(idx, RTMP_GET_PACKET_WCID(pkt));
			if (pAd->dabs_qos_op & DABS_DBG_PRN_LV1) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Set QTBL for STA%u,idx=%u,src=%u.%u.%u.%u,dest=%u.%u.%u.%u, proto=%u,sp=%u,dp=%u\n",
					RTMP_GET_PACKET_WCID(pkt), idx,
					NIPQUAD(ip_src), NIPQUAD(ip_dest), proto, htons(sport), htons(dport));
			}
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
			pAd->vow_sta_ac[RTMP_GET_PACKET_WCID(pkt)] = up_to_ac_mapping[prec->priority];
#endif
			enable_qos_param_tbl_by_idx(idx);

			if (pAd->dabs_qos_op & DABS_SET_QOS_PARAM) {
				if (HW_UPDATE_QOS_PARAM(pAd, idx, TRUE) == FALSE)
					disable_qos_param_tbl_by_idx(idx);
			}
		} else {
			RTMP_SET_PACKET_QOS_IDX(pkt, idx);
		}
	}

	return;
}
INT Show_qos_dabs_table_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	struct qos_param_rec *pqos_param;
	UCHAR ip_dest[4];
	UCHAR ip_src[4];
	PMAC_TABLE_ENTRY pEntry;

	for (i = 0; i < MAX_QOS_PARAM_TBL; i++) {
		pqos_param = &qos_param_table[i];
		pEntry = &pAd->MacTab.Content[pqos_param->wlan_idx];
		if (pqos_param->in_use) {
			memcpy(ip_dest, &pqos_param->ip_dest, 4);
			memcpy(ip_src, &pqos_param->ip_src, 4);
			MTWF_PRINT(
				"[%d]sta_mac:" MACSTR "\ndescip: %d.%d.%d.%d, sourceip: %d.%d.%d.%d\n",
				i, MAC2STR(pEntry->Addr), ip_dest[0], ip_dest[1], ip_dest[2], ip_dest[3],
				ip_src[0], ip_src[1], ip_src[2], ip_src[3]);
			MTWF_PRINT("descport:%d, srcport:%d, UP=%d, delay_req=%d\n",
					pqos_param->dport, pqos_param->sport,
					 pqos_param->priority, pqos_param->delay_req);

		}
	}
	return TRUE;
}

VOID ExtEventFastPathRptHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	INT32 i;
	P_EXT_EVENT_FASTPATH_RPT_T prTxRptEvt;
	P_FASTPATH_RPT_T prTxRpt  = NULL;
	UINT32 *RecInUseBitmap;
	INT32 FW_QOS_PARAM_TBL = 32;
	UINT drop_percentage = 0, max_drop_percentage = 0;
	UINT TxCnt = 0, DropCnt = 0;

	prTxRptEvt = (P_EXT_EVENT_FASTPATH_RPT_T) Data;
	RecInUseBitmap = &prTxRptEvt->RecInUseBitmap[0];
	prTxRpt = (P_FASTPATH_RPT_T)(Data + sizeof(EXT_EVENT_FASTPATH_RPT_T));

	if (pAd->dabs_qos_op & DABS_DBG_PRN_LV2)
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"FPDBG:TokenU:[%u,%u],PageU:%u\n",
			prTxRptEvt->u2FreeTokenUnderflowCnt[0],
			prTxRptEvt->u2FreeTokenUnderflowCnt[1],
			prTxRptEvt->u2FreePageUnderflowCnt);


	for (i = 0; i < FW_QOS_PARAM_TBL; i++) {

		if ((RecInUseBitmap[i >> RED_INUSE_BITSHIFT] & (1 << (i & RED_INUSE_BITMASK))) == 0)
			continue;

		if (pAd->dabs_qos_op & DABS_DBG_PRN_LV2)
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"FPDBG:QTBL[%u],TxDrop:%u,TxCnt:%u,TxDly:%u\n",
				i, prTxRpt->u4DropCnt,
				prTxRpt->u4TxCnt, prTxRpt->u4TxDly);
		if (prTxRpt->u4TxCnt == 0)
			continue;
		TxCnt = prTxRpt->u4TxCnt;
		DropCnt = prTxRpt->u4DropCnt;
		drop_percentage = DropCnt * 100 / TxCnt;
		if (drop_percentage > max_drop_percentage)
		max_drop_percentage = drop_percentage;

		prTxRpt++;
	}
	if (max_drop_percentage > pAd->dabs_drop_threashold)
		pAd->mscs_req_reject = TRUE;
	else
		pAd->mscs_req_reject = FALSE;
}
static VOID MtCmdFastPathCalMICRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EVENT_FAST_PATH_T pEventResult = (P_EVENT_FAST_PATH_T)Data;
	P_EVENT_FAST_PATH_T pFastpath = (P_EVENT_FAST_PATH_T)msg->attr.rsp.wb_buf_in_calbk;

#ifdef RT_BIG_ENDIAN
#endif
	pFastpath->u2Mic = pEventResult->u2Mic;
	pFastpath->u4KeybitmapMatchStatus = pEventResult->u4KeybitmapMatchStatus;
	pFastpath->ucKeynum = pEventResult->ucKeynum;
	memcpy(pFastpath->cap.u4Keybitmap, pEventResult->cap.u4Keybitmap, 16);
	pFastpath->cap.ucSupportFastPath = pEventResult->cap.ucSupportFastPath;
	pFastpath->cap.ucVersion = pEventResult->cap.ucVersion;
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ucKeynum:%d, u2Mic (%x)\n",
			 pFastpath->ucKeynum, pFastpath->u2Mic);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MatchStatus:%x, ExtCmd (0x%02x)\n",
			 pFastpath->u4KeybitmapMatchStatus, msg->attr.ext_type);
	return;
}

BOOLEAN FastPathCheckMIC(RTMP_ADAPTER *pAd, UINT_8 ucOpMode, UINT_16 u2WlanId,
	UINT_16 u2RandomNum, UINT_8 mic_type, UINT_16 u2mic, UINT_32 *au4Keybitmap, EVENT_FAST_PATH_T *event_fastpath)
{
	int ret = NDIS_STATUS_SUCCESS;
	CMD_FAST_PATH_T fastpath_cmd;
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
    MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[u2WlanId];

	memset(&fastpath_cmd, 0, sizeof(CMD_FAST_PATH_T));
	fastpath_cmd.ucOpMode = ucOpMode;
	fastpath_cmd.u2CmdLen = sizeof(CMD_FAST_PATH_T);
	if (au4Keybitmap) {
    fastpath_cmd.u4Keybitmap[0] = au4Keybitmap[0];
    fastpath_cmd.u4Keybitmap[1] = au4Keybitmap[1];
    fastpath_cmd.u4Keybitmap[2] = au4Keybitmap[2];
    fastpath_cmd.u4Keybitmap[3] = au4Keybitmap[3];
	}
	fastpath_cmd.u2Mic = u2mic;
    fastpath_cmd.u2WlanId = u2WlanId;
	if (ucOpMode == FAST_PATH_CMD_CAL_MIC) {
	 if (mic_type == MIC_STA)
			NdisCopyMemory(&fastpath_cmd.aucOwnMac[0],  pEntry->Addr, 6);
		else if (mic_type == MIC_AP)
			NdisCopyMemory(&fastpath_cmd.aucOwnMac[0],  pEntry->wdev->if_addr, 6);
	}
	if (ucOpMode == FAST_PATH_CMD_CAL_MIC_TEST_MODE)
		NdisZeroMemory(&fastpath_cmd.aucOwnMac[0], 6);

    fastpath_cmd.u2RandomNum = u2RandomNum;

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_FAST_PATH_T));

	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "msg alloc fail!!!\n");
		ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_FAST_PATH_CAL_MIC);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_FAST_PATH_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, event_fastpath);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdFastPathCalMICRsp);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	fastpath_cmd = cpu2le32(fastpath_cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&fastpath_cmd, sizeof(CMD_FAST_PATH_T));
	ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "(ret = %d)(op:%u)\n", ret, ucOpMode);

	return ret;
}
#endif /* DABS_QOS */
