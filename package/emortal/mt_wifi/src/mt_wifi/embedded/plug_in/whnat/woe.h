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

	Module Name: wifi_offload
	whnat.h
*/

#ifndef _WHNAT_H
#define _WHNAT_H

#include "wdma.h"
#include "wed.h"
#include "woe_hif.h"
#include "woe_basic.h"

struct whnat_cfg {
	char cr_mirror_en;
	char hw_tx_en;
};

struct whnat_entry {
	unsigned char idx;
	unsigned char slot_id;
	struct wifi_entry wifi;
	struct platform_driver pdriver;
	struct wdma_entry wdma;
	struct wed_entry wed;
	struct platform_dev *pdev;
	struct whnat_cfg cfg;
	void *proc;
	void *proc_stat;
	void *proc_cr;
	void *proc_cfg;
	void *proc_tx;
	void *proc_rx;
	void *proc_ctrl;
};


struct whnat_ctrl {
	unsigned char whnat_num;
	struct whnat_entry *entry;
	struct whnat_hif_cfg hif_cfg;
	unsigned int whnat_driver_idx;
	void *proc;
	void *proc_trace;
};

struct whnat_wifi_cr_map {
	unsigned int wifi_cr;
	unsigned int whnat_cr;
	char whnat_type;
};

void whnat_proc_handle(struct whnat_entry *entry);
void whnat_dump_cfg(struct whnat_entry *whnat);
void whnat_dump_txinfo(struct whnat_entry *whnat);
void whnat_dump_rxinfo(struct whnat_entry *whnat);


struct whnat_ctrl *whnat_ctrl_get(void);
struct whnat_entry *whnat_entry_search(void *cookie);
struct whnat_entry *whnat_entry_search_by_hw_ctrl(void *hw_ctrl);


int whnat_entry_proc_init(struct whnat_ctrl *whnat_ctrl, struct whnat_entry *whnat);
void whnat_entry_proc_exit(struct whnat_ctrl *whnat_ctrl, struct whnat_entry *whnat);
int whnat_ctrl_proc_init(struct whnat_ctrl *whnat_ctrl);
void whnat_ctrl_proc_exit(struct whnat_ctrl *whnat_ctrl);
int wed_entry_proc_init(struct whnat_entry *whnat, struct wed_entry *wed);
void wed_entry_proc_exit(struct whnat_entry *whnat, struct wed_entry *wed);
int wdma_entry_proc_init(struct whnat_entry *whnat, struct wdma_entry *wdma);
void wdma_entry_proc_exit(struct whnat_entry *whnat, struct wdma_entry *wdma);





#endif /*_WHNAT_H*/
