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

*/

#ifdef DOT11_HE_AX

#include "rt_config.h"
#include "hdev/hdev.h"

void bss_color_table_init(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	struct bss_color_table *table = &pResource->color_tbl[0];
	UINT8 index;

	os_zero_mem(table, sizeof(struct bss_color_table) * DBDC_BAND_NUM);
	for (index = 0; index < DBDC_BAND_NUM; index++)
		NdisAllocateSpinLock(NULL, &pResource->color_tbl[index].bss_color_lock);
}

void bss_color_table_deinit(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	struct bss_color_table *table;
	UINT8 index;

	for (index = 0; index < DBDC_BAND_NUM; index++) {
		table = &pResource->color_tbl[index];
		NdisFreeSpinLock(&table->bss_color_lock);
		os_zero_mem(table, sizeof(struct bss_color_table));
	}
}

UINT8 bcolor_acquire_entry(struct hdev_ctrl *ctrl, struct hdev_obj *obj)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	struct bss_color_table *table;
	ULONG current_time;
	UINT8 new_color;
	UCHAR band_idx, start_idx, stop_idx;

	band_idx = RcGetBandIdx(obj->rdev);
	table = &pResource->color_tbl[band_idx];

	NdisGetSystemUpTime(&current_time);
	start_idx = current_time % BSS_COLOR_VALUE_MAX;
	stop_idx = start_idx + BSS_COLOR_VALUE_MAX;
	NdisAcquireSpinLock(&table->bss_color_lock);

	while (start_idx < stop_idx) {
		new_color = (start_idx < BSS_COLOR_VALUE_MAX) ? start_idx : (start_idx % BSS_COLOR_VALUE_MAX);
		if (table->last_detected_time[new_color] == 0) {
			table->last_detected_time[new_color] = current_time;
			break;
		}
		start_idx++;
	}

	NdisReleaseSpinLock(&table->bss_color_lock);

	return (start_idx == stop_idx) ? 0 : (new_color + 1);
}

void bcolor_release_entry(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UINT8 color)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	struct bss_color_table *table;
	UCHAR band_idx;

	band_idx = RcGetBandIdx(obj->rdev);
	table = &pResource->color_tbl[band_idx];
	NdisAcquireSpinLock(&table->bss_color_lock);
	table->last_detected_time[color - 1] = 0;
	NdisReleaseSpinLock(&table->bss_color_lock);
}

void bcolor_occupy_entry(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UINT8 color)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	struct bss_color_table *table;
	ULONG current_time;
	UCHAR band_idx;

	band_idx = RcGetBandIdx(obj->rdev);
	table = &pResource->color_tbl[band_idx];
	NdisGetSystemUpTime(&current_time);
	NdisAcquireSpinLock(&table->bss_color_lock);
	table->last_detected_time[color - 1] = current_time;
	NdisReleaseSpinLock(&table->bss_color_lock);
}

BOOLEAN bcolor_entry_is_occupied(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UINT8 color)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	struct bss_color_table *table;
	UCHAR band_idx;
	BOOLEAN ret;

	band_idx = RcGetBandIdx(obj->rdev);
	table = &pResource->color_tbl[band_idx];
	NdisAcquireSpinLock(&table->bss_color_lock);

	if (table->last_detected_time[color - 1])
		ret = TRUE;
	else
		ret = FALSE;

	NdisReleaseSpinLock(&table->bss_color_lock);
	return ret;
}

void bcolor_entry_ageout(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UINT8 sec)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	struct bss_color_table *table;
	UCHAR band_idx, i;
	ULONG current_time;

	band_idx = RcGetBandIdx(obj->rdev);
	table = &pResource->color_tbl[band_idx];
	NdisGetSystemUpTime(&current_time);
	NdisAcquireSpinLock(&table->bss_color_lock);

	for (i = 0; i < BSS_COLOR_VALUE_MAX; i++) {
		if (RTMP_TIME_AFTER(current_time, table->last_detected_time[i] + (sec * OS_HZ)))
			table->last_detected_time[i] = 0;
	}

	NdisReleaseSpinLock(&table->bss_color_lock);
}

void bcolor_get_bitmap(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UINT8 *bitmap)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	struct bss_color_table *table;
	UCHAR band_idx, i;
	UINT8 maps[8];

	band_idx = RcGetBandIdx(obj->rdev);
	table = &pResource->color_tbl[band_idx];

	os_zero_mem(maps, sizeof(maps));
	NdisAcquireSpinLock(&table->bss_color_lock);

	for (i = 0; i < BSS_COLOR_VALUE_MAX; i++) {
		if (table->last_detected_time[i])
			maps[(i + 1) / 8] |= 1 << ((i + 1) % 8);
	}
	NdisReleaseSpinLock(&table->bss_color_lock);
	os_move_mem(bitmap, maps, sizeof(maps));
}

void bcolor_update_by_bitmap(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UINT8 *bitmap)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	struct bss_color_table *table;
	UCHAR band_idx, i;
	ULONG current_time;
	UINT8 maps[8];

	band_idx = RcGetBandIdx(obj->rdev);
	table = &pResource->color_tbl[band_idx];
	NdisGetSystemUpTime(&current_time);

	os_move_mem(maps, bitmap, sizeof(maps));
	NdisAcquireSpinLock(&table->bss_color_lock);

	for (i = 0; i < BSS_COLOR_VALUE_MAX; i++) {
		if (maps[(i + 1) / 8] & (1 << ((i+1) % 8)))
			table->last_detected_time[i] = current_time;
	}

	NdisReleaseSpinLock(&table->bss_color_lock);
}
#endif
