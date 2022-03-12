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

	Module Name:
	misc_app.h

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs

	Carter Chen	2019-May-15		misc applications definitions.
*/

#ifndef _MGMT_ENTRYTB_H_
#define _MGMT_ENTRYTB_H_

#include "rtmp_type.h"
#include "rtmp_def.h"

#define INVALID_AID	2008 /*According SPEC, aid is in the range of 1 to 2007.*/

struct _RTMP_CHIP_CAP;
struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _RTMP_CHIP_DBG;

struct _aid_info {
	UINT32 *aid_bitmap;
	UINT16 aid_allocate_from_idx;
	UINT16 max_aid;
};

/* static size about MacTab in compiler time */
#if defined(MT7915)                      /* MT7915 */
#if defined(CONFIG_COLGIN_MT6890)
#define MAX_LEN_OF_MAC_TABLE    128
#else
#define MAX_LEN_OF_MAC_TABLE    288
#endif
#elif defined(MT7626)                    /* MT7626 */
#define MAX_LEN_OF_MAC_TABLE    144
#elif defined(MT7663)                    /* MT7663 */
#define MAX_LEN_OF_MAC_TABLE    136
#else                                    /* else */
#define MAX_LEN_OF_MAC_TABLE    128
#endif

#define IS_WCID_VALID(_pAd, _wcid)		((_wcid) < WTBL_MAX_NUM(_pAd))

#define MAX_LEN_OF_TR_TABLE				(MAX_LEN_OF_MAC_TABLE)
#define IS_TR_WCID_VALID(_pAd, _wcid)	(IS_WCID_VALID(_pAd, _wcid) && ((_wcid) < MAX_LEN_OF_TR_TABLE))

typedef UINT32 (*entrytb_traversal_func)(struct _MAC_TABLE_ENTRY *entry, void *cookie);

void entrytb_aid_bitmap_init(struct _RTMP_CHIP_CAP *cap, struct _aid_info *aid_info);
void entrytb_aid_bitmap_free(struct _aid_info *aid_info);
void entrytb_aid_bitmap_reserve(struct _aid_info *aid_info, UINT16 aid_order_reserved);
UINT16 entrytb_aid_aquire(struct _aid_info *aid_info);

INT show_aid_info(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

typedef struct _entrytb_aid_search_t {
	UINT16 aid_search;/*the AID that would like to search in the entrytb*/
	struct _MAC_TABLE_ENTRY *entry;/*the returned entry once the AID matched*/
} entrytb_aid_search_t;

UINT32 traversal_func_find_entry_by_aid(struct _MAC_TABLE_ENTRY *entry, void *cookie);
UINT32 traversal_func_dump_entry_rate_by_aid(struct _MAC_TABLE_ENTRY *entry, void *cookie);
UINT32 traversal_func_dump_entry_psm_by_aid(struct _MAC_TABLE_ENTRY *entry, void *cookie);
typedef struct _entrytb_bss_idx_search_t {
	UINT32 bss_idx;/*the BSS IDX that would like to search in the entrytb*/
	UINT32 need_print_field_name;
} entrytb_bss_idx_search_t;
UINT32 traversal_func_dump_entry_associated_to_bss(struct _MAC_TABLE_ENTRY *entry, void *cookie);

UINT32 entrytb_traversal(struct _RTMP_ADAPTER *ad, entrytb_traversal_func func, void *cookie);


#endif /*_MGMT_ENTRYTB_H_*/


