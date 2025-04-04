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
 /***************************************************************************
 ***************************************************************************

    Module Name:
    protection.c

    Abstract:
    For general 802.11 Legacy/HT/nonHT protection mechanism

    Who             When            What
    --------------  ----------      ----------------------------------------------
    Hugo            2016-0505       created
*/

#include "rt_config.h"

#ifdef CONFIG_AP_SUPPORT

UINT16 nonerp_sta_num(struct _MAC_TABLE_ENTRY *peer, UCHAR peer_state)
{
	if ((peer->MaxHTPhyMode.field.MODE == MODE_CCK) && (peer->Sst == SST_ASSOC)) {
		if (peer_state == PEER_JOIN) {
			peer->pMbss->conn_sta.nonerp_sta_cnt++;
		}
		if ((peer_state == PEER_LEAVE) && (peer->pMbss->conn_sta.nonerp_sta_cnt > 0)) {
			peer->pMbss->conn_sta.nonerp_sta_cnt--;
		}
	}

	return peer->pMbss->conn_sta.nonerp_sta_cnt;
}

UCHAR nonerp_protection(struct _BSS_STRUCT *bss_struct)
{
	UCHAR use_prot = 0;

	if (bss_struct && (bss_struct->conn_sta.nonerp_sta_cnt > 0)) {
		use_prot = 1;
	}

	return use_prot;
}

#endif
