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
    protection.h

    Abstract:
    Generic 802.11 Legacy/HT/nonHT Protection Mechanism

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Hugo        2016-0505     create

*/

#ifndef __PROTECTION_H__
#define __PROTECTION_H__

struct _MAC_TABLE_ENTRY;
struct _BSS_STRUCT;

enum ht_protection {
	NON_PROTECT = 0,
	NONMEMBER_PROTECT = 1,
	BW20_PROTECT = 2,
	NONHT_MM_PROTECT = 3
};

enum peer_state {
	PEER_JOIN = 0,
	PEER_LEAVE
};

enum protection {
	/* 11n */
	NO_PROTECTION, /*0*/
	NON_MEMBER_PROTECT, /*1*/
	HT20_PROTECT, /*2*/
	NON_HT_MIXMODE_PROTECT, /*3*/
	_NOT_DEFINE_HT_PROTECT,
	/* b/g */
	ERP,
	/* vendor */
	LONG_NAV_PROTECT,
	RDG_PROTECT = LONG_NAV_PROTECT,
	GREEN_FIELD_PROTECT,
	RIFS_PROTECT,
	RDG,
	FORCE_RTS_PROTECT,
	_NOT_DEFINE_VENDOR_PROTECT,
	_END_PROTECT
};

enum prot_service_type {
	PROT_PROTOCOL,
	PROT_RTS_THLD,
#ifdef DOT11_HE_AX
	PROT_TXOP_DUR_BASE,
#endif
	PROT_MAX_TYPE
};

#ifdef DOT11_HE_AX
#define MAX_TXOP_DURATION_RTS_THRESHOLD	1023
#define DISABLE_TXOP_DURATION_RTS_THRESHOLD	MAX_TXOP_DURATION_RTS_THRESHOLD
#endif

#define SET_PROTECT(x)  (1 << (x))

#ifdef CONFIG_AP_SUPPORT

UINT16 nonerp_sta_num(struct _MAC_TABLE_ENTRY *peer, UCHAR peer_state);
UCHAR nonerp_protection(struct _BSS_STRUCT *bss_struct);

#endif

#endif /* __PROTECTION_H__ */
