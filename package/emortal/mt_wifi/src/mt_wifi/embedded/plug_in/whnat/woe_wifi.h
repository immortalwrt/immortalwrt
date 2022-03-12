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
	woe_wifi.h
*/
#ifndef _WOE_WIFI_H_

#include "woe_client_jedi.h"

/*TX hook structure*/
struct wlan_tx_info {
	UCHAR *pkt;
	UINT32 bssidx;
	UINT32 wcid;
	UINT32 ringidx;
};



struct wifi_entry {
	void *cookie;
	unsigned char slot_id;
	unsigned int irq;
	unsigned int wpdma_base;
	unsigned int tx_ring_num;
	unsigned int tx_ring_len;
	unsigned int tx_token_nums;
	unsigned int sw_tx_token_nums;
	unsigned int *int_mask;
	unsigned long base_addr;
};

/*default SER status*/
#define WIFI_ERR_RECOV_NONE 0x10


void dump_wifi_value(struct wifi_entry *wifi, char *name, unsigned int addr);
/*wifi related hal*/
void wifi_fbuf_init(unsigned char *fbuf, unsigned int pkt_pa, unsigned int tkid);
void wifi_tx_tuple_add(void *woe, unsigned char *tx_info);
void wifi_tx_tuple_reset(void);
char wifi_hw_tx_allow(void *cookie, unsigned char *tx_info);
void wifi_dma_cfg_wrapper(int wifi_cfg, unsigned char *dma_cfg);
void wifi_dump_tx_ring_info(struct wifi_entry *wifi, unsigned char ring_id, unsigned int idx);
void wifi_chip_cr_mirror_set(struct wifi_entry *wifi, unsigned char enable);
void wifi_chip_probe(struct wifi_entry *wifi, unsigned int irq);
void wifi_chip_remove(struct wifi_entry *wifi);
unsigned int wifi_chip_id_get(void *cookie);
unsigned int wifi_whnat_en_get(void *cookie);
void wifi_whnat_en_set(void *cookie, unsigned int en);
unsigned int wifi_ser_status(void *ser_ctrl);
int wifi_slot_get(void *cookie);
void wifi_cap_get(struct wifi_entry *wifi);
unsigned int wifi_wpdma_base_get(void *cookie);
void *wifi_get_hw_ctrl(void *cookie);


#endif /*_WOE_WIFI_H_*/
