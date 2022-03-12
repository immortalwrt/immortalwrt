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
	multi_hif.c
*/
#include "rt_config.h"
#include "multi_hif.h"
#include "common/link_list.h"

struct multi_hif_ctrl {
	UINT cur_id;
	struct _DL_LIST head;
	NDIS_SPIN_LOCK lock;
};

static struct multi_hif_ctrl mhif_ctrl;

VOID multi_hif_init(VOID)
{
	spin_lock_init((spinlock_t *)(&mhif_ctrl.lock));
	OS_SPIN_LOCK(&mhif_ctrl.lock);
	DlListInit(&mhif_ctrl.head);
	mhif_ctrl.cur_id = 0;
	OS_SPIN_UNLOCK(&mhif_ctrl.lock);
}

VOID multi_hif_exit(VOID)
{
	struct multi_hif_entry *cur, *next;

	OS_SPIN_LOCK(&mhif_ctrl.lock);
	DlListForEachSafe(cur, next, &mhif_ctrl.head, struct multi_hif_entry, list) {
		if (cur) {
			DlListDel(&cur->list);
			os_free_mem(cur);
		}
	}
	OS_SPIN_UNLOCK(&mhif_ctrl.lock);
}

inline NDIS_STATUS multi_hif_acquire_id(struct multi_hif_entry *entry)
{
	if (!entry)
		return NDIS_STATUS_INVALID_DATA;

	OS_SPIN_LOCK(&mhif_ctrl.lock);
	entry->id = mhif_ctrl.cur_id++;
	/*assig gid = id firt, may change after match*/
	entry->gid = entry->id;
	DlListAddTail(&mhif_ctrl.head, &entry->list);
	OS_SPIN_UNLOCK(&mhif_ctrl.lock);

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS multi_hif_entry_alloc(VOID **hif_ctrl, UINT32 size)
{
	UINT32 alloc_size = size + sizeof(struct multi_hif_entry);
	struct multi_hif_entry *hif_entry;

	if (os_alloc_mem(NULL, (UCHAR **) &hif_entry, alloc_size) != NDIS_STATUS_SUCCESS)
		goto err;

	os_zero_mem(hif_entry, alloc_size);
	if (multi_hif_acquire_id(hif_entry) != NDIS_STATUS_SUCCESS)
		goto err1;

	*hif_ctrl = &hif_entry->hif_ctrl[0];
	return NDIS_STATUS_SUCCESS;
err1:
	os_free_mem(hif_entry);
err:
	return NDIS_STATUS_RESOURCES;
}

VOID multi_hif_entry_free(VOID *hif_ctrl)
{
	struct multi_hif_entry *hif_entry;

	hif_entry = container_of(hif_ctrl, struct multi_hif_entry, hif_ctrl);

	OS_SPIN_LOCK(&mhif_ctrl.lock);
	DlListDel(&hif_entry->list);
	OS_SPIN_UNLOCK(&mhif_ctrl.lock);
	os_free_mem(hif_entry);
}

VOID *multi_hif_entry_get_by_id(UINT32 id)
{
	struct multi_hif_entry *hif_entry, *next;

	OS_SPIN_LOCK(&mhif_ctrl.lock);
	DlListForEachSafe(hif_entry, next, &mhif_ctrl.head, struct multi_hif_entry, list) {
		if (hif_entry->id == id) {
			OS_SPIN_UNLOCK(&mhif_ctrl.lock);
			return &hif_entry->hif_ctrl;
		}
	}
	OS_SPIN_UNLOCK(&mhif_ctrl.lock);
	return NULL;
}

VOID multi_hif_entry_get_by_gid(UINT32 gid, struct _DL_LIST *head)
{
	struct multi_hif_entry *hif_entry;

	OS_SPIN_LOCK(&mhif_ctrl.lock);
	DlListInit(head);
	DlListForEach(hif_entry, &mhif_ctrl.head, struct multi_hif_entry, list) {
		if (hif_entry->gid == gid)
			DlListAddTail(head, &hif_entry->glist);
	}
	OS_SPIN_UNLOCK(&mhif_ctrl.lock);
}

UINT32 multi_hif_entry_id_get(VOID *hif_ctrl)
{
	struct multi_hif_entry *hif_entry;

	hif_entry = container_of(hif_ctrl, struct multi_hif_entry, hif_ctrl);

	return hif_entry->id;
}

UINT32 multi_hif_entry_gid_get(VOID *hif_ctrl)
{
	struct multi_hif_entry *hif_entry;

	hif_entry = container_of(hif_ctrl, struct multi_hif_entry, hif_ctrl);

	return hif_entry->gid;
}

VOID multi_hif_entry_gid_set(VOID *hif_ctrl, UINT32 gid)
{
	struct multi_hif_entry *hif_entry;

	hif_entry = container_of(hif_ctrl, struct multi_hif_entry, hif_ctrl);

	hif_entry->gid = gid;
}

VOID multi_hif_entry_rid_set(VOID *hif_ctrl, UINT32 rid)
{
	struct multi_hif_entry *hif_entry;

	hif_entry = container_of(hif_ctrl, struct multi_hif_entry, hif_ctrl);

	hif_entry->rid = rid;
}

UINT32 multi_hif_entry_rid_get(VOID *hif_ctrl)
{
	struct multi_hif_entry *hif_entry;

	hif_entry = container_of(hif_ctrl, struct multi_hif_entry, hif_ctrl);

	return hif_entry->rid;
}
