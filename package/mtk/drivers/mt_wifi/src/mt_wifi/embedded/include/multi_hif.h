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
	multi_hif.h
*/

#ifndef __MULTI_HIF_H__
#define __MULTI_HIF_H__

struct multi_hif_entry {
	struct _DL_LIST list;
	struct _DL_LIST glist;
	UINT32 id;
	UINT32 gid;
	UINT32 rid;
	UINT8 hif_ctrl[0] ____cacheline_aligned;
};

VOID multi_hif_init(VOID);
VOID multi_hif_exit(VOID);
VOID multi_hif_entry_free(VOID *hif_ctrl);
VOID multi_hif_entry_gid_set(VOID *hif_ctrl, UINT32 gid);
UINT32 multi_hif_entry_gid_get(VOID *hif_ctrl);
VOID *multi_hif_entry_get_by_id(UINT32 id);
UINT32 multi_hif_entry_id_get(VOID *hif_ctrl);
NDIS_STATUS multi_hif_entry_alloc(VOID **hif_ctrl, UINT32 size);
VOID multi_hif_entry_get_by_gid(UINT32 gid, struct _DL_LIST *head);
VOID multi_hif_entry_rid_set(VOID *hif_ctrl, UINT32 rid);
UINT32 multi_hif_entry_rid_get(VOID *hif_ctrl);

#endif /*__MULTI_HIF_H__*/
