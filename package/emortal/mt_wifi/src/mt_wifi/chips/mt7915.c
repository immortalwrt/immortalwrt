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
	mt7915.c
*/

#include "rt_config.h"
#include "chip/mt7915_cr.h"
#include "mcu/mt7915_firmware.h"
#include "mcu/mt7915_firmware_e2.h"
#include "mcu/mt7915_WA_firmware.h"
#ifdef NEED_ROM_PATCH
#include "mcu/mt7915_rom_patch_e1.h"
#include "mcu/mt7915_rom_patch_e2.h"
#endif /* NEED_ROM_PATCH */
#include "mac/mac_mt/fmac/mt_fmac.h"

/* iPAiLNA shall always be included as default */
#include "eeprom/mt7915_e2p_iPAiLNA.h"

#if defined(CONFIG_FIRST_IF_EPAELNA) || defined(CONFIG_SECOND_IF_EPAELNA) || defined(CONFIG_THIRD_IF_EPAELNA)
#include "eeprom/mt7915_e2p_ePAeLNA.h"
#endif
#if defined(CONFIG_FIRST_IF_IPAELNA) || defined(CONFIG_SECOND_IF_IPAELNA) || defined(CONFIG_THIRD_IF_IPAELNA)
#include "eeprom/mt7915_e2p_iPAeLNA.h"
#endif
#if defined(CONFIG_FIRST_IF_EPAILNA) || defined(CONFIG_SECOND_IF_EPAILNA) || defined(CONFIG_THIRD_IF_EPAILNA)
#include "eeprom/mt7915_e2p_ePAiLNA.h"
#endif

typedef enum _RXV_CONTENT_CAT {
	RXV_CONTENT_CMN1 = 0,
	RXV_CONTENT_USR1,
	RXV_CONTENT_USR2,
	RXV_CONTENT_CMN2,
	RXV_CONTENT_NUM
} RXV_CONTENT_CAT, *P_RXV_CONTENT_CAT;

typedef enum _RXV_DUMP_LIST_TYPE {
	RXV_DUMP_LIST_TYPE_CONTENT = 0,
	RXV_DUMP_LIST_TYPE_BASIC_ENTRY,
	RXV_DUMP_LIST_TYPE_ENTRY,
	RXV_DUMP_LIST_TYPE_NUM
} RXV_DUMP_LIST_TYPE, *P_RXV_DUMP_LIST_TYPE;

INT8 i1RxFeLossComp[RAM_BAND_NUM][MAX_ANTENNA_NUM];
/*
 * Used for DNL Calibration happen in MP-Line state
 * 3 Channel for G-band L/M/H channel
 * 16 Channel for A-band L/H channel
 */
UINT16 MT7915_DNL_CAL_GBAND_BW20_FREQ[] = {2412, 2442, 2472};
UINT16 MT7915_DNL_CAL_ABAND_BW20_FREQ[] = {
	4960, 5060,/* Group 1 */
	5180, 5240,/* Group 2 */
	5260, 5320,/* Group 3 */
	5340, 5480,/* Group 4 */
	5500, 5560,/* Group 5 */
	5580, 5640,/* Group 6 */
	5660, 5720,/* Group 7 */
	5785, 5845 /* Group 8 */};
UINT16 MT7915_DNL_CAL_BW20_FREQ[] = {
	2412, 2442, 2472,
	4960, 5060, 5180, 5240, 5260, 5320, 5340, 5480, 5500, 5560, 5580, 5640, 660, 5720, 5785, 5845};

UINT16 MT7915_DNL_CAL_GBAND_BW20_CH[] = {1, 7, 13};
UINT16 MT7915_DNL_CAL_ABAND_BW20_CH[] = {
	192,  12,  36,  48,  52,  64,  68,  96, 100, 112, 116, 128, 132, 144, 157, 169};
UINT16 MT7915_DNL_CAL_BW20_CH[] = {
	  1,   7,  13, 192,  12,  36,  48,  52,  64,  68,  96, 100, 112, 116, 128, 132, 144, 157, 169};

UINT16 MT7915_DNL_CAL_GBAND_BW20_SIZE = (sizeof(MT7915_DNL_CAL_GBAND_BW20_FREQ) / sizeof(UINT16));
UINT16 MT7915_DNL_CAL_ABAND_BW20_SIZE = (sizeof(MT7915_DNL_CAL_ABAND_BW20_FREQ) / sizeof(UINT16));
UINT16 MT7915_DNL_CAL_BW20_FREQ_SIZE  = (sizeof(MT7915_DNL_CAL_BW20_FREQ) / sizeof(UINT16));

UINT16 MT7915_DNL_CAL_GBAND_BW20_CH_SIZE = (sizeof(MT7915_DNL_CAL_GBAND_BW20_CH) / sizeof(UINT16));
UINT16 MT7915_DNL_CAL_ABAND_BW20_CH_SIZE = (sizeof(MT7915_DNL_CAL_ABAND_BW20_CH) / sizeof(UINT16));
UINT16 MT7915_DNL_CAL_B20_CH_SIZE        = (sizeof(MT7915_DNL_CAL_BW20_CH) / sizeof(UINT16));

const struct hif_pci_tx_ring_desc tx_ring_layout[] = {
	{
	 .hw_desc_base = MT_DMA1_T18_RING_BASE,
	 .hw_int_mask = MT_INT_DMA1_T18_DONE,
	 .ring_size = 2048,
	 .ring_attr = HIF_TX_DATA,
	 .band_idx = BAND0,
	 .ring_info = "band0 TXD"
	},
	{
	 .hw_desc_base = MT_DMA1_T19_RING_BASE,
	 .hw_int_mask = MT_INT_DMA1_T19_DONE,
	 .ring_size = 2048,
	 .ring_attr = HIF_TX_DATA,
	 .band_idx = BAND1,
	 .ring_info = "band1 TXD"
	},
	{
	 .hw_desc_base = MT_DMA1_T16_RING_BASE,
	 .hw_int_mask = MT_INT_DMA1_T16_DONE,
	 .ring_size = 128,
	 .ring_attr = HIF_TX_FWDL,
	 .ring_info = "FWDL"
	},
	{
	 .hw_desc_base = MT_DMA1_T17_RING_BASE,
	 .hw_int_mask = MT_INT_DMA1_T17_DONE,
	 .ring_size = 256,
	 .ring_attr = HIF_TX_CMD_WM,
	 .ring_info = "cmd to WM"
	},
	{
	 .hw_desc_base = MT_DMA1_T20_RING_BASE,
	 .hw_int_mask = MT_INT_DMA1_T20_DONE,
	 .ring_size = 256,
	 .ring_attr = HIF_TX_CMD,
	 .ring_info = "cmd to WA"
	}
};
#define TX_RING_NUM	ARRAY_SIZE(tx_ring_layout)

static struct dly_ctl_cfg dly_ctl_ul_tbl_hostdma0_r0[] = {
	{
	 .avg_tp = 0,
	 .dly_number = 1,
	 .dly_time = 28
	},
	{
	 .avg_tp = 300,
	 .dly_number = 24,
	 .dly_time = 20
	},
	{
	 .avg_tp = 500,
	 .dly_number = 48,
	 .dly_time = 27
	}
};

static struct dly_ctl_cfg dly_ctl_dl_tbl_hostdma0_r0[] = {
	{
	 .avg_tp = 0,
	 .dly_number = 1,
	 .dly_time = 28
	},
};

static struct dly_ctl_cfg dly_ctl_ul_tbl_hostdma0_r1[] = {
	{
	 .avg_tp = 0,
	 .dly_number = 1,
	 .dly_time = 28
	},
	{
	 .avg_tp = 300,
	 .dly_number = 24,
	 .dly_time = 20
	},
	{
	 .avg_tp = 500,
	 .dly_number = 48,
	 .dly_time = 27
	}
};

static struct dly_ctl_cfg dly_ctl_dl_tbl_hostdma0_r1[] = {
	{
	 .avg_tp = 0,
	 .dly_number = 1,
	 .dly_time = 28
	},
};

static struct dly_ctl_cfg dly_ctl_ul_tbl_hostdma1_r1[] = {
	{
	 .avg_tp = 0,
	 .dly_number = 1,
	 .dly_time = 28
	},
	{
	 .avg_tp = 300,
	 .dly_number = 8,
	 .dly_time = 7
	},
	{
	 .avg_tp = 500,
	 .dly_number = 16,
	 .dly_time = 8
	}
};

static struct dly_ctl_cfg dly_ctl_dl_tbl_hostdma1_r1[] = {
	{
	 .avg_tp = 0,
	 .dly_number = 1,
	 .dly_time = 28
	},
};

static struct dly_ctl_cfg dly_ctl_ul_tbl_hostdma1_r2[] = {
	{
	 .avg_tp = 0,
	 .dly_number = 1,
	 .dly_time = 28
	},
	{
	 .avg_tp = 300,
	 .dly_number = 8,
	 .dly_time = 7
	},
	{
	 .avg_tp = 500,
	 .dly_number = 16,
	 .dly_time = 8
	}
};

static struct dly_ctl_cfg dly_ctl_dl_tbl_hostdma1_r2[] = {
	{
	 .avg_tp = 0,
	 .dly_number = 1,
	 .dly_time = 28
	},
};

const struct hif_pci_rx_ring_desc rx_ring_layout[] = {
	{
	 .hw_desc_base = MT_DMA0_R0_RING_BASE,
	 .hw_int_mask = MT_INT_DMA0_R0_DONE,
	 .ring_size = 1536,
	 .ring_attr = HIF_RX_DATA,
	 .delay_int_en = TRUE,
	 .dl_dly_ctl_tbl = dly_ctl_dl_tbl_hostdma0_r0,
	 .dl_dly_ctl_tbl_size = sizeof(dly_ctl_dl_tbl_hostdma0_r0)
							/ sizeof(dly_ctl_dl_tbl_hostdma0_r0[0]),
	 .ul_dly_ctl_tbl = dly_ctl_ul_tbl_hostdma0_r0,
	 .ul_dly_ctl_tbl_size = sizeof(dly_ctl_ul_tbl_hostdma0_r0)
							/ sizeof(dly_ctl_ul_tbl_hostdma0_r0[0]),
	 .max_rx_process_cnt = 128,
	 .max_sw_read_idx_inc = 128,
	 .buf_type = DYNAMIC_PAGE_ALLOC,
	 .band_idx = BAND0_RX_PCIE0,
	 .ring_info = "band0 RX data"
	},
	{
	 .hw_desc_base = MT_DMA0_R1_RING_BASE,
	 .hw_int_mask = MT_INT_DMA0_R1_DONE,
	 .ring_size = 1536,
	 .ring_attr = HIF_RX_DATA,
	 .delay_int_en = TRUE,
	 .dl_dly_ctl_tbl = dly_ctl_dl_tbl_hostdma0_r1,
	 .dl_dly_ctl_tbl_size = sizeof(dly_ctl_dl_tbl_hostdma0_r1)
							/ sizeof(dly_ctl_dl_tbl_hostdma0_r1[0]),
	 .ul_dly_ctl_tbl = dly_ctl_ul_tbl_hostdma0_r1,
	 .ul_dly_ctl_tbl_size = sizeof(dly_ctl_ul_tbl_hostdma0_r1)
							/ sizeof(dly_ctl_ul_tbl_hostdma0_r1[0]),
	 .max_rx_process_cnt = 128,
	 .max_sw_read_idx_inc = 128,
	 .buf_type = DYNAMIC_PAGE_ALLOC,
	 .band_idx = BAND1_RX_PCIE0,
	 .ring_info = "band1 RX data"
	},
	{
	 .hw_desc_base = MT_DMA1_R0_RING_BASE,
	 .hw_int_mask = MT_INT_DMA1_R0_DONE,
	 .ring_size = 512,
	 .ring_attr = HIF_RX_EVENT,
	 .max_rx_process_cnt = 128,
	 .max_sw_read_idx_inc = 128,
	 .buf_type = PRE_SLAB_ALLOC,
	 .ring_info = "event from WM"
	},
	{
	 .hw_desc_base = MT_DMA1_R1_RING_BASE,
	 .hw_int_mask = MT_INT_DMA1_R1_DONE,
	 .ring_size = 1024,
	 .ring_attr = HIF_RX_EVENT,
	 .event_type = HOST_MSDU_ID_RPT,
	 .delay_int_en = TRUE,
	 .dl_dly_ctl_tbl = dly_ctl_dl_tbl_hostdma1_r1,
	 .dl_dly_ctl_tbl_size = sizeof(dly_ctl_dl_tbl_hostdma1_r1)
							/ sizeof(dly_ctl_dl_tbl_hostdma1_r1[0]),
	 .ul_dly_ctl_tbl = dly_ctl_ul_tbl_hostdma1_r1,
	 .ul_dly_ctl_tbl_size = sizeof(dly_ctl_ul_tbl_hostdma1_r1)
							/ sizeof(dly_ctl_ul_tbl_hostdma1_r1[0]),
	 .max_rx_process_cnt = 256,
	 .max_sw_read_idx_inc = 256,
	 .buf_type = PRE_SLAB_ALLOC,
	 .ring_info = "event from WA band0"
	},
	{
	 .hw_desc_base = MT_DMA1_R2_RING_BASE,
	 .hw_int_mask = MT_INT_DMA1_R2_DONE,
	 .ring_size = 512,
	 .ring_attr = HIF_RX_EVENT,
	 .delay_int_en = TRUE,
	 .dl_dly_ctl_tbl = dly_ctl_dl_tbl_hostdma1_r2,
	 .dl_dly_ctl_tbl_size = sizeof(dly_ctl_dl_tbl_hostdma1_r2)
							/ sizeof(dly_ctl_dl_tbl_hostdma1_r2[0]),
	 .ul_dly_ctl_tbl = dly_ctl_ul_tbl_hostdma1_r2,
	 .ul_dly_ctl_tbl_size = sizeof(dly_ctl_ul_tbl_hostdma1_r2)
							/ sizeof(dly_ctl_ul_tbl_hostdma1_r2[0]),
	 .max_rx_process_cnt = 128,
	 .max_sw_read_idx_inc = 128,
	 .buf_type = PRE_SLAB_ALLOC,
	 .ring_info = "event from WA band1"
	}
};
#define RX_RING_NUM	ARRAY_SIZE(rx_ring_layout)

struct hif_pci_tx_ring_desc tx_ring_layout_pcie1[] = {
	{.hw_desc_base = MT_PCI1_DMA1_T19_RING_BASE,
	 .hw_int_mask = MT_INT1_DMA1_T19_DONE,
	 .ring_size = 2048,
	 .ring_attr = HIF_TX_DATA,
	 .ring_info = "band1 TXD"},
};

#define TX_RING_NUM_PCIE1	ARRAY_SIZE(tx_ring_layout_pcie1)

static struct dly_ctl_cfg dly_ctl_ul_tbl_hostdma0_r1_pcie1[] = {
	{
	 .avg_tp = 0,
	 .dly_number = 1,
	 .dly_time = 28
	},
	{
	 .avg_tp = 300,
	 .dly_number = 24,
	 .dly_time = 20
	},
	{
	 .avg_tp = 500,
	 .dly_number = 48,
	 .dly_time = 27
	}
};

static struct dly_ctl_cfg dly_ctl_dl_tbl_hostdma0_r1_pcie1[] = {
	{
	 .avg_tp = 0,
	 .dly_number = 1,
	 .dly_time = 28
	},
};

static struct dly_ctl_cfg dly_ctl_ul_tbl_hostdma1_r2_pcie1[] = {
	{
	 .avg_tp = 0,
	 .dly_number = 1,
	 .dly_time = 28
	},
	{
	 .avg_tp = 300,
	 .dly_number = 8,
	 .dly_time = 7
	},
	{
	 .avg_tp = 500,
	 .dly_number = 16,
	 .dly_time = 8
	}
};

static struct dly_ctl_cfg dly_ctl_dl_tbl_hostdma1_r2_pcie1[] = {
	{
	 .avg_tp = 0,
	 .dly_number = 1,
	 .dly_time = 28
	},
};

const struct hif_pci_rx_ring_desc rx_ring_layout_pcie1[] = {
	{
	 .hw_desc_base = MT_PCI1_DMA0_R1_RING_BASE,
	 .hw_int_mask = MT_INT1_DMA0_R1_DONE,
	 .ring_size = 1536,
	 .ring_attr = HIF_RX_DATA,
	 .delay_int_en = TRUE,
	 .dl_dly_ctl_tbl = dly_ctl_dl_tbl_hostdma0_r1_pcie1,
	 .dl_dly_ctl_tbl_size = sizeof(dly_ctl_dl_tbl_hostdma0_r1_pcie1)
							/ sizeof(dly_ctl_dl_tbl_hostdma0_r1_pcie1[0]),
	 .ul_dly_ctl_tbl = dly_ctl_ul_tbl_hostdma0_r1_pcie1,
	 .ul_dly_ctl_tbl_size = sizeof(dly_ctl_ul_tbl_hostdma0_r1_pcie1)
							/ sizeof(dly_ctl_ul_tbl_hostdma0_r1_pcie1[0]),
	 .max_rx_process_cnt = 128,
	 .max_sw_read_idx_inc = 128,
	 .buf_type = DYNAMIC_PAGE_ALLOC,
	 .band_idx = BAND1_RX_PCIE1,
	 .ring_info = "band1 data"
	},
	{
	 .hw_desc_base = MT_PCI1_DMA1_R2_RING_BASE,
	 .hw_int_mask = MT_INT1_DMA1_R2_DONE,
	 .ring_size = 1024,
	 .ring_attr = HIF_RX_EVENT,
	 .delay_int_en = TRUE,
	 .dl_dly_ctl_tbl = dly_ctl_dl_tbl_hostdma1_r2_pcie1,
	 .dl_dly_ctl_tbl_size = sizeof(dly_ctl_dl_tbl_hostdma1_r2_pcie1)
							/ sizeof(dly_ctl_dl_tbl_hostdma1_r2_pcie1[0]),
	 .ul_dly_ctl_tbl = dly_ctl_ul_tbl_hostdma1_r2_pcie1,
	 .ul_dly_ctl_tbl_size = sizeof(dly_ctl_ul_tbl_hostdma1_r2_pcie1)
							/ sizeof(dly_ctl_ul_tbl_hostdma1_r2_pcie1[0]),
	 .max_rx_process_cnt = 128,
	 .max_sw_read_idx_inc = 128,
	 .buf_type = PRE_SLAB_ALLOC,
	 .ring_info = "event from WA band1"
	}
};
#define RX_RING_NUM_PCIE1	ARRAY_SIZE(rx_ring_layout_pcie1)

static struct rtmp_spe_map mt7915_spe_map[] = {
	/* All */
	{0x0, 0},
	{0xf, 0},
	/* 1 Ant */
	{0x1, 0},	/* Tx0 */
	{0x2, 1},	/* Tx1 */
	{0x4, 3},	/* Tx2 */
	{0x8, 9},	/* Tx3 */
	/* 2 Ant */
	{0x3, 0},
	{0x5, 2},
	{0x9, 8},
	{0x6, 4},
	{0xa, 6},
	{0xc, 16},
	/* 3 Ant */
	{0x7, 0},	/* 0_1_2 */
	{0xb, 10},	/* 0_1_3 */
	{0xd, 12},	/* 0_2_3 */
	{0xe, 18},	/* 1_2_3 */
};

#ifdef PRE_CAL_MT7915_SUPPORT
UINT16 MT7915_DPD_FLATNESS_ABAND_BW20_FREQ[] = {
	5180, 5200, 5220, 5240, 5260, 5280, 5300, 5320,
	5500, 5520, 5540, 5560, 5580, 5600, 5620, 5640,
	5660, 5680, 5700, 5745, 5765, 5785, 5805, 5825};
UINT16 MT7915_DPD_FLATNESS_GBAND_BW20_FREQ[] = {2422, 2442, 2462};
UINT16 MT7915_DPD_FLATNESS_BW20_FREQ[] = {
	5180, 5200, 5220, 5240, 5260, 5280, 5300, 5320,
	5500, 5520, 5540, 5560, 5580, 5600, 5620, 5640,
	5660, 5680, 5700, 5745, 5765, 5785, 5805, 5825, 2422, 2442, 2462};

UINT16 MT7915_DPD_FLATNESS_ABAND_BW20_CH[] = {
	 36,  40,  44,  48,  52,  56,  60,  64,
	100, 104, 108, 112, 116, 120, 124, 128,
	132, 136, 140, 149, 153, 157, 161, 165};
UINT16 MT7915_DPD_FLATNESS_GBAND_BW20_CH[] = {3, 7, 11};
UINT16 MT7915_DPD_FLATNESS_BW20_CH[] = {
	 36,  40,  44,  48,  52,  56,  60,  64,
	100, 104, 108, 112, 116, 120, 124, 128,
	132, 136, 140, 149, 153, 157, 161, 165, 3, 7, 11};

UINT16 MT7915_DPD_FLATNESS_ABAND_BW20_SIZE = (sizeof(MT7915_DPD_FLATNESS_ABAND_BW20_FREQ) / sizeof(UINT16));
UINT16 MT7915_DPD_FLATNESS_GBAND_BW20_SIZE = (sizeof(MT7915_DPD_FLATNESS_GBAND_BW20_FREQ) / sizeof(UINT16));
UINT16 MT7915_DPD_FLATNESS_BW20_FREQ_SIZE  = (sizeof(MT7915_DPD_FLATNESS_BW20_FREQ) / sizeof(UINT16));

UINT16 MT7915_DPD_FLATNESS_ABAND_BW20_CH_SIZE = (sizeof(MT7915_DPD_FLATNESS_ABAND_BW20_CH) / sizeof(UINT16));
UINT16 MT7915_DPD_FLATNESS_GBAND_BW20_CH_SIZE = (sizeof(MT7915_DPD_FLATNESS_GBAND_BW20_CH) / sizeof(UINT16));
UINT16 MT7915_DPD_FLATNESS_B20_CH_SIZE        = (sizeof(MT7915_DPD_FLATNESS_BW20_CH) / sizeof(UINT16));
#endif /* PRE_CAL_MT7915_SUPPORT */


/* SKU Table definitions */
const CHAR cSkuParseTypeName[SINGLE_SKU_TYPE_PARSE_NUM_V1][8] = {"CCK", "OFDM", "VHT20", "VHT40", "VHT80", "VHT160", "RU26", "RU52", "RU106", "RU242", "RU484", "RU996", "RU996X2"};
const CHAR cBackoffParseTypeName[BACKOFF_TYPE_PARSE_NUM_V1][14] = {"BFOFF_CCK", "BF_OFF_OFDM", "BF_ON_OFDM", "BF_OFF_VHT20", "BF_ON_VHT20", "BF_OFF_VHT40", "BF_ON_VHT40", "BF_OFF_VHT80", "BF_ON_VHT80", "BF_OFF_VHT160", "BF_ON_VHT160", "BF_OFF_RU26", "BF_ON_RU26", "BF_OFF_RU52", "BF_ON_RU52", "BF_OFF_RU106", "BF_ON_RU106"};

const UINT8 SINGLE_SKU_PARSE_TABLE_LENGTH[SINGLE_SKU_TYPE_PARSE_NUM_V1] = {
	SINGLE_SKU_PARSE_TABLE_CCK_LENGTH_V1,
	SINGLE_SKU_PARSE_TABLE_OFDM_LENGTH_V1,
	SINGLE_SKU_PARSE_TABLE_HTVHT20_LENGTH_V1,
	SINGLE_SKU_PARSE_TABLE_HTVHT40_LENGTH_V1,
	SINGLE_SKU_PARSE_TABLE_VHT80_LENGTH_V1,
	SINGLE_SKU_PARSE_TABLE_VHT160_LENGTH_V1,
	SINGLE_SKU_PARSE_TABLE_RU26_LENGTH_V1,
	SINGLE_SKU_PARSE_TABLE_RU52_LENGTH_V1,
	SINGLE_SKU_PARSE_TABLE_RU106_LENGTH_V1,
	SINGLE_SKU_PARSE_TABLE_RU242_LENGTH_V1,
	SINGLE_SKU_PARSE_TABLE_RU484_LENGTH_V1,
	SINGLE_SKU_PARSE_TABLE_RU996_LENGTH_V1,
	SINGLE_SKU_PARSE_TABLE_RU996X2_LENGTH_V1
};

UINT8 MT7915_SINGLE_SKU_FILL_TABLE_LENGTH[] = {
	SINGLE_SKU_FILL_TABLE_CCK_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_OFDM_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_HT20_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_HT40_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_VHT20_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_VHT40_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_VHT80_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_VHT160_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_RU26_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_RU52_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_RU106_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_RU242_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_RU484_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_RU996_LENGTH_V1,
	SINGLE_SKU_FILL_TABLE_RU996X2_LENGTH_V1
};

const UINT8 BACKOFF_TABLE_BF_LENGTH[BACKOFF_TYPE_PARSE_NUM_V1] = {
	BACKOFF_TABLE_BF_OFF_CCK_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_OFDM_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_OFDM_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_VHT20_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_VHT20_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_VHT40_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_VHT40_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_VHT80_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_VHT80_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_VHT160_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_VHT160_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_RU26_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_RU26_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_RU52_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_RU52_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_RU106_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_RU106_LENGTH_V1
};

const UINT8 BACKOFF_FILL_TABLE_BF_LENGTH[BACKOFF_PARAM_NUM_V1] = {
	BACKOFF_TABLE_BF_OFF_CCK_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_OFDM_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_OFDM_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_VHT20_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_VHT20_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_VHT40_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_VHT40_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_VHT20_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_VHT20_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_VHT40_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_VHT40_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_VHT80_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_VHT80_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_VHT160_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_VHT160_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_RU26_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_RU26_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_RU52_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_RU52_LENGTH_V1,
	BACKOFF_TABLE_BF_OFF_RU106_LENGTH_V1,
	BACKOFF_TABLE_BF_ON_RU106_LENGTH_V1
};

const UINT32 hif_ownsership_cr[2][OWNERSHIP_CR_TYPE_NUM] = {
	{CONN_HOST_CSR_TOP_WF_BAND0_LPCTL_ADDR, CONN_HOST_CSR_TOP_WF_BAND0_IRQ_STAT_ADDR},
	{CONN_HOST_CSR_TOP_WF_BAND1_LPCTL_ADDR, CONN_HOST_CSR_TOP_WF_BAND1_IRQ_STAT_ADDR},
};

#ifdef PRE_CAL_MT7915_SUPPORT
static void mt7195_find_both_central_for_bw160(
	MT_SWITCH_CHANNEL_CFG SwChCfg,
	UCHAR                 *CentPrim80,
	UCHAR                 *CentSec80
	)
{
	if (SwChCfg.Bw == BW_160) {
		switch (SwChCfg.CentralChannel) {
		case 50:
		case 114:
			if (SwChCfg.ControlChannel < SwChCfg.CentralChannel) {
				*CentPrim80 = SwChCfg.CentralChannel - 8;
				*CentSec80  = SwChCfg.CentralChannel + 8;
			} else {
				*CentPrim80 = SwChCfg.CentralChannel + 8;
				*CentSec80  = SwChCfg.CentralChannel - 8;
			}

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s() : BW_160 CtrlCh [%d] CentCh [%d]\n	=> PrimCentral [%d] , SecCentral [%d]\n",
				 __func__, SwChCfg.ControlChannel, SwChCfg.CentralChannel, *CentPrim80, *CentSec80));

			break;

		default:
			*CentPrim80 = 199;
			*CentSec80 = 199;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s() : ERROR!!!! unknown bw160 central %d !!!! shall do online K\n"
					  , __func__, SwChCfg.CentralChannel));
		}
	} else {
		UINT abs_cent1 = ABS(SwChCfg.ControlChannel, SwChCfg.CentralChannel);
		UINT abs_cent2 = ABS(SwChCfg.ControlChannel, SwChCfg.ControlChannel2);

		if (abs_cent1 < abs_cent2) {
			/* prim 80 is CentralChannel */
			*CentPrim80 = SwChCfg.CentralChannel;
			*CentSec80  = SwChCfg.ControlChannel2;
		} else {
			/* prim 80 is ControlChannel2 */
			*CentPrim80 = SwChCfg.ControlChannel2;
			*CentSec80  = SwChCfg.CentralChannel;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s() : BW_8080 CtrlCh [%d] CtrlCh2 [%d]\n	 CentCh [%d]  [ABS %d , %d] => prim80 [%d] sec80 [%d]\n",
				__func__, SwChCfg.ControlChannel, SwChCfg.ControlChannel2, SwChCfg.CentralChannel,
				abs_cent1, abs_cent2, *CentPrim80, *CentSec80));
	}
}

void mt7915_apply_dpd_flatness_data(
	RTMP_ADAPTER          *pAd,
	MT_SWITCH_CHANNEL_CFG SwChCfg,
	UINT16                BW160Central,
	BOOLEAN               bSecBw80
	)
{
	USHORT			doCal1 = 0;
	UINT8			i = 0;
	UINT8			Band = 0;
	UINT16			CentralFreq = 0;
	UINT8			sendNum = 0;
	UINT8			ofst = 0;
	RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);

	chip_ops->eeread(pAd, PRECAL_INDICATION_BYTE, &doCal1);

	if (((doCal1 & (1 << DPD5G_PRECAL_INDN_BIT)) != 0) &&
		((doCal1 & (1 << DPD2G_PRECAL_INDN_BIT)) != 0)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: DPD Pre-Cal finished, load DPD Pre-Cal data\n", __func__));
		if (SwChCfg.CentralChannel == 14) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s: CH 14 don't need DPD , return!!!\n", __func__));
			return;
		} else if (SwChCfg.CentralChannel < 14) {
			Band = GBAND;

			if (SwChCfg.CentralChannel >= 1 && SwChCfg.CentralChannel <= 4)
				CentralFreq = 2422;
			else if (SwChCfg.CentralChannel >= 5 && SwChCfg.CentralChannel <= 9)
				CentralFreq = 2442;
			else if (SwChCfg.CentralChannel >= 10 && SwChCfg.CentralChannel <= 13)
				CentralFreq = 2462;
			else
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s: can't find cent freq for CH %d , should not happen!!!\n",
						  __func__, SwChCfg.CentralChannel));
		} else {
			Band = ABAND;

			/*
			* by the rule DE suggests,
			* 1.  BW20 directly apply , illegal channel online cal.
			* 2.  BW40/80/160 add center frequency by 10 MHz to find a nearest calibrated BW20 CH
			* 3.  if center freq + 10MHz = different group , then use center freq -10 MHz to apply
			*/

			if (SwChCfg.Bw == BW_20) {
				CentralFreq = SwChCfg.CentralChannel * 5 + 5000;
			} else if (SwChCfg.Bw == BW_160 || SwChCfg.Bw == BW_8080) {
				UINT32 Central = BW160Central * 5 + 5000;
				UINT32 CentralAdd10M = (BW160Central + 2) * 5 + 5000;

				if (ChannelFreqToGroup(Central) != ChannelFreqToGroup(CentralAdd10M)) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("==== Different Group Central %d @ group %d Central+10 @ group %d !!\n"
							  , Central, ChannelFreqToGroup(Central), ChannelFreqToGroup(CentralAdd10M)));
					CentralFreq = (BW160Central - 2) * 5 + 5000;
				} else
					CentralFreq = (BW160Central + 2) * 5 + 5000;
			} else {
				UINT32 Central = SwChCfg.CentralChannel * 5 + 5000;
				UINT32 CentralMinus10M = (SwChCfg.CentralChannel - 2) * 5 + 5000;

				if (ChannelFreqToGroup(Central) != ChannelFreqToGroup(CentralMinus10M)) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("==== Different Group Central %d @ group %d Central-10 @ group %d !!\n"
						, Central, ChannelFreqToGroup(Central), ChannelFreqToGroup(CentralMinus10M)));
					CentralFreq = (SwChCfg.CentralChannel + 2) * 5 + 5000;
				} else
					CentralFreq = (SwChCfg.CentralChannel - 2) * 5 + 5000;
			}
		}
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			("%s: eeprom 0x%2x bit 0 is 0, do runtime cal\n",
			__func__, PRECAL_INDICATION_BYTE));
		return;
	}

	/* Find if CentralFreq is exist in DPD+Flatness pre-k table */
	for (i = 0; i < MT7915_DPD_FLATNESS_BW20_FREQ_SIZE; i++) {
		if (MT7915_DPD_FLATNESS_BW20_FREQ[i] == CentralFreq) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s: %d is in DPD-Flatness cal table, index = %d\n",
				__func__, CentralFreq, i));
			break;
		}
	}

	if (i == MT7915_DPD_FLATNESS_BW20_FREQ_SIZE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: Unexpected freq (%d)\n", __func__, CentralFreq));
		return;
	}

	sendNum = DPD_FLATNESS_CAL_SIZE / PRE_CAL_SET_MAX_LENGTH;
	while (ofst < sendNum) {
		MtCmdSetDpdFlatnessCal_7915(pAd, i * sendNum + ofst, PRE_CAL_SET_MAX_LENGTH, bSecBw80);
		ofst++;
	}
}

void mt7915_ch_bw_check_for_pre_k(RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg)
{
	/* If not flash mode or BIN mode, not support pre-k */
	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Currently not in FLASH or BIN MODE,return.\n", __func__));
		return;
	}

	if ((SwChCfg.Bw == BW_160) || (SwChCfg.Bw == BW_8080)) {
		UCHAR CentPrim80 = 0, CentSec80 = 0;

		mt7195_find_both_central_for_bw160(SwChCfg, &CentPrim80, &CentSec80);
		mt7915_apply_dpd_flatness_data(pAd, SwChCfg, CentPrim80, FALSE);
		mt7915_apply_dpd_flatness_data(pAd, SwChCfg, CentSec80, TRUE);
	} else {
		mt7915_apply_dpd_flatness_data(pAd, SwChCfg, 0, FALSE);
	}
}
#endif /* PRE_CAL_MT7915_SUPPORT */

static void switch_channel(RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	cap->channelbw = SwChCfg.Bw;
	/* update power limit table */
	MtPwrLimitTblChProc(pAd, SwChCfg.BandIdx, SwChCfg.Channel_Band, SwChCfg.ControlChannel, SwChCfg.CentralChannel);

#ifdef PRE_CAL_MT7915_SUPPORT
    mt7915_ch_bw_check_for_pre_k(pAd, SwChCfg);
#endif

#ifdef DBDC_MODE
	/* error handling for wrong configs */
	if (pAd->CommonCfg.dbdc_mode == 1) {
		if ((SwChCfg.BandIdx == BAND0) && (SwChCfg.Channel_Band >= ch_band_a)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("\x1b[41m%s(): ERROR band %d only supports G band\x1b[m\n",
					__func__,
					SwChCfg.BandIdx));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("\x1b[41m%s(): ERROR band %d, channel band %d, channel %d\x1b[m\n",
					__func__,
					SwChCfg.BandIdx,
					SwChCfg.Channel_Band,
					SwChCfg.ControlChannel));
			return;
		} else if ((SwChCfg.BandIdx == BAND1) && (SwChCfg.Channel_Band == ch_band_g)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("\x1b[41m%s(): ERROR band %d only supports A band\x1b[m\n",
					__func__,
					SwChCfg.BandIdx));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("\x1b[41m%s(): ERROR band %d, channel band %d, channel %d\x1b[m\n",
					__func__,
					SwChCfg.BandIdx,
					SwChCfg.Channel_Band,
					SwChCfg.ControlChannel));
			return;
		}
	}
#endif

	MtCmdChannelSwitch(pAd, SwChCfg);
	MtCmdSetTxRxPath(pAd, SwChCfg);
	pAd->LatchRfRegs.Channel = SwChCfg.CentralChannel;
}

#ifdef NEW_SET_RX_STREAM
static INT set_RxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx)
{
	UINT32 path = 0;
	UINT i;

	if (StreamNums > 4) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():illegal StreamNums(%d)\n",
				  __func__, StreamNums));
		StreamNums = 4;
	}

	for (i = 0; i < StreamNums; i++)
		path |= 1 << i;

	return MtCmdSetRxPath(pAd, path, BandIdx);
}
#endif

static inline VOID bufferModeDataFill(RTMP_ADAPTER *pAd, union _EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd, UINT16 addr)
{
	UINT32 i = pCmd->v2.ucCount;

	pCmd->v2.BinContent[i] = pAd->EEPROMImage[addr];
	pCmd->v2.ucCount++;
}


static VOID bufferModeCmdFill(RTMP_ADAPTER *pAd, union _EXT_CMD_EFUSE_BUFFER_MODE_T *cmd, UINT16 ctrl_msg)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 addr = 0, page_size = 0x400;
	UINT8 total_page = 0, page_idx = 0, format = 0;

	format = ((ctrl_msg & 0xff00) >> 8);

	cmd->v2.ucSourceMode = (ctrl_msg & 0xff);
	cmd->v2.ucContentFormat = format;
	cmd->v2.ucCount = 0;

	if (ctrl_msg & EEPROM_MODE_BUFFER) {
		total_page = (format & BUFFER_BIN_TOTAL_PAGE_MASK) >> BUFFER_BIN_TOTAL_PAGE_SHIFT;
		page_idx = (format & BUFFER_BIN_PAGE_INDEX_MASK) >> BUFFER_BIN_PAGE_INDEX_SHIFT;

		if (page_idx == total_page) {
			if ((cap->EFUSE_BUFFER_CONTENT_SIZE % 0x400) != 0)
				page_size = (cap->EFUSE_BUFFER_CONTENT_SIZE % 0x400);
		}

		for (addr = 0; addr < page_size; addr++)
			bufferModeDataFill(pAd, cmd, (addr + (page_idx*0x400)));
	}

	cmd->v2.ucCount = cpu2le16(cmd->v2.ucCount);
}

#ifdef CAL_FREE_IC_SUPPORT
static struct _kfree_def ddie_def[] = {
	{43, {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
	      0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x20, 0x21, 0x22, 0x23, 0x24,
	      0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e,
	      0x52, 0x70, 0x71, 0x72, 0x76, 0xa8, 0xa9, 0xaa, 0xab, 0xac,
	      0xad, 0xae, 0xaf}
	}
};

static struct _kfree_def adie_def[] = {
	{55, {0x7cd, 0x7cf, 0x7d1, 0x7d3, 0x802, 0x803, 0x804, 0x805, 0x806, 0x808,
	      0x80a, 0x80b, 0x80c, 0x80d, 0x80e, 0x810, 0x812, 0x813, 0x814, 0x815,
	      0x816, 0x818, 0x81a, 0x81b, 0x81c, 0x81d, 0x81e, 0x820, 0x822, 0x823,
	      0x824, 0x825, 0x826, 0x827, 0x828, 0x829, 0x82f, 0x8d0, 0x8d1, 0x8d7,
	      0x8d8, 0x8fa, 0x9a1, 0x9a5, 0x9a6, 0x9a8, 0x9aa, 0x9b0, 0x9b1, 0x9b2,
	      0x9b3, 0x9b4, 0x9b5, 0x9b6, 0x9b7}
	},
	{59, {0x7cd, 0x7cf, 0x7d1, 0x7d3, 0x802, 0x803, 0x804, 0x805, 0x806, 0x808,
	      0x80a, 0x80b, 0x80c, 0x80d, 0x80e, 0x810, 0x812, 0x813, 0x814, 0x815,
	      0x816, 0x818, 0x81a, 0x81b, 0x81c, 0x81d, 0x81e, 0x820, 0x822, 0x823,
	      0x824, 0x825, 0x826, 0x827, 0x828, 0x829, 0x82f, 0x8c0, 0x8c1, 0x8c2,
	      0x8c3, 0x8d0, 0x8d1, 0x8d7, 0x8d8, 0x8fa, 0x9a1, 0x9a5, 0x9a6, 0x9a8,
	      0x9aa, 0x9b0, 0x9b1, 0x9b2, 0x9b3, 0x9b4, 0x9b5, 0x9b6, 0x9b7}
	}
};

static BOOLEAN is_ical_ignorable(UINT16 offset)
{
	BOOLEAN ret = FALSE;

	if (offset > 0xa7 && offset < 0xb0)
		ret = TRUE;
	else if (offset > 0x9af && offset < 0x9b8)
		ret = TRUE;

	return ret;
}

static inline BOOLEAN check_valid(RTMP_ADAPTER *pAd, UINT16 Offset)
{
	UINT16 Value = 0;
	BOOLEAN NotValid, ret = FALSE;

	if ((Offset % 2))
		Offset -= 1;

	NotValid = rtmp_ee_efuse_read16(pAd, Offset, &Value);

	if (NotValid)
		ret = FALSE;
	else
		ret = TRUE;

	return ret;
}

static BOOLEAN is_cal_free_ic(RTMP_ADAPTER *pAd)
{
	UINT8 kfree_cnt = 0, loop = 0;
	UINT16 Value = 0;
	BOOLEAN NotValid, ret = FALSE;
	struct _kfree_def *kfree_def = NULL;

	/* D-Die version */
	NotValid = rtmp_ee_efuse_read16(pAd, 0x50, &Value);

	if (NotValid || ((Value & 0xff) == 0x00)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\x1b[43m%s iCal for D-Die is invalid, merge dismissed!\x1b[m\n",
			  __func__));
		goto err_out;
	} else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\x1b[32m[d-die version:%d]\x1b[m\n", (Value & 0xff)));

	if ((Value & 0xff) > ARRAY_SIZE(ddie_def)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\x1b[41mUnknown iCal D-die version=%d, merge dismissed!\x1b[m\n", (Value & 0xff)));
		goto err_out;
	}
	kfree_def = &ddie_def[(Value & 0xff)-1];
	kfree_cnt = kfree_def->count;

	for (loop = 0; loop < kfree_cnt; loop++) {
		if ((check_valid(pAd, kfree_def->offsets[loop]) == FALSE)
			&& (is_ical_ignorable(kfree_def->offsets[loop]) == FALSE)) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\x1b[41m%s 0x%x invalid, iCal merge dismissed!\x1b[m\n",
				  __func__, kfree_def->offsets[loop]));
			goto err_out;
		}
	}
	/* A-Die version */
	NotValid = rtmp_ee_efuse_read16(pAd, 0x9a0, &Value);

	if (NotValid || ((Value & 0xff) == 0x00)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\x1b[43m%s iCal for A-Die is invalid, merge dismissed!\x1b[m\n",
			  __func__));
		goto err_out;
	} else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\x1b[32m[a-die version:%d]\x1b[m\n", (Value & 0xff)));

	if ((Value & 0xff) > ARRAY_SIZE(adie_def)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\x1b[41mUnknown iCal A-die version=%d, merge dismessed!\x1b[m\n", (Value & 0xff)));
		goto err_out;
	}

	kfree_def = &adie_def[(Value & 0xff)-1];
	kfree_cnt = kfree_def->count;

	for (loop = 0; loop < kfree_cnt; loop++)
		if (check_valid(pAd, kfree_def->offsets[loop]) == FALSE) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\x1b[41m%s 0x%x invalid, iCal merge dismissed!\x1b[m\n",
				  __func__, kfree_def->offsets[loop]));
			goto err_out;
		}

	ret = TRUE;

err_out:
	return ret;
}

static inline VOID cal_free_data_get_from_addr(RTMP_ADAPTER *ad, UINT16 Offset)
{
	UINT16 value;

	if ((Offset % 2) != 0) {
		rtmp_ee_efuse_read16(ad, Offset - 1, &value);
		ad->EEPROMImage[Offset] = ((value >> 8) & 0xFF);
	} else {
		rtmp_ee_efuse_read16(ad, Offset, &value);
		ad->EEPROMImage[Offset] = (value & 0xFF);
	}
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 ("\t\t%s Replace: 0x%x=0x%x\n", __func__,
		  Offset, ad->EEPROMImage[Offset]));
}

static VOID cal_free_data_get(RTMP_ADAPTER *ad)

{
	UINT8 kfree_cnt = 0, loop = 0;
	struct _kfree_def *kfree_def = NULL;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

	/* D-Die version */
	cal_free_data_get_from_addr(ad, 0x50);
	kfree_def = &ddie_def[ad->EEPROMImage[0x50]-1];
	kfree_cnt = kfree_def->count;
	for (loop = 0; loop < kfree_cnt; loop++)
		cal_free_data_get_from_addr(ad, kfree_def->offsets[loop]);

	/* A-Die version */
	cal_free_data_get_from_addr(ad, 0x9a0);
	kfree_def = &adie_def[ad->EEPROMImage[0x9a0]-1];
	kfree_cnt = kfree_def->count;
	for (loop = 0; loop < kfree_cnt; loop++) {
		cal_free_data_get_from_addr(ad, kfree_def->offsets[loop]);

		/* workaround to apply XO trim to 0x77 as well due to implementation */
		if (kfree_def->offsets[loop] == 0x9a1) {
			ad->EEPROMImage[0x77] = ad->EEPROMImage[0x9a1];
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("\t\t%s Replace: 0x77=0x%x\n", __func__,
				  ad->EEPROMImage[0x9a1]));
		}
	}
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
}
#endif /* CAL_FREE_IC_SUPPORT */

#ifndef MAC_INIT_OFFLOAD
#endif /* MAC_INIT_OFFLOAD */

static VOID init_mac_cr(RTMP_ADAPTER *pAd)
{
	UCHAR bw_signal = 0;
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[MAIN_MBSSID].wdev;
	}
#endif
	if (wdev)
		bw_signal = wlan_config_get_vht_bw_sig(wdev);


	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __func__));
#ifndef MAC_INIT_OFFLOAD
	/* Set TxFreeEvent packet only go through CR4 */
	HW_IO_READ32(pAd->hdev_ctrl, PLE_HIF_REPORT, &mac_val);
	mac_val |= 0x1;
	HW_IO_WRITE32(pAd->hdev_ctrl, PLE_HIF_REPORT, mac_val);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s(): Set TxRxEventPkt path 0x%0x = 0x%08x\n",
			  __func__, PLE_HIF_REPORT, mac_val));
	HW_IO_READ32(pAd->hdev_ctrl, PP_PAGECTL_2, &mac_val);
	mac_val &= ~(PAGECTL_2_CUT_PG_CNT_MASK);
	mac_val |= 0x30;
	HW_IO_WRITE32(pAd->hdev_ctrl, PP_PAGECTL_2, mac_val);
#if defined(COMPOS_WIN) || defined(COMPOS_TESTMODE_WIN)
#else
	/* TxS Setting */
	InitTxSTypeTable(pAd);
#endif
	MtAsicSetTxSClassifyFilter(pAd, TXS2HOST, TXS2H_QID1, TXS2HOST_AGGNUMS, 0x00, 0);
#endif /*MAC_INIT_OFFLOAD*/

	/* RTS Bandwidth Signaling Setting */
	if (bw_signal > BW_SIGNALING_DISABLE)
		AsicSetRtsSignalTA(pAd, bw_signal);

}

static VOID BBPInit(RTMP_ADAPTER *pAd)
{
	BOOLEAN band_vld[2];
	INT idx, cbw[2] = {0};
	INT cent_ch[2] = {0}, prim_ch[2] = {0}, prim_ch_idx[2] = {0};
	INT band[2] = {0};
	INT txStream[2] = {0};
	UCHAR use_bands;

	band_vld[0] = TRUE;
	cbw[0] = RF_BW_20;
	cent_ch[0] = 1;
	prim_ch[0] = 1;
	band[0] = BAND_24G;
	txStream[0] = 2;
#ifdef DOT11_VHT_AC
	prim_ch_idx[0] = vht_prim_ch_idx(cent_ch[0], prim_ch[0], cbw[0]);
#endif /* DOT11_VHT_AC */

	band_vld[1] = FALSE;
	use_bands = 1;
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s():BBP Initialization.....\n", __func__));

	for (idx = 0; idx < 2; idx++) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tBand %d: valid=%d, Band=%d, CBW=%d, CentCh/PrimCh=%d/%d, prim_ch_idx=%d, txStream=%d\n",
				  idx, band_vld[idx], band[idx], cbw[idx], cent_ch[idx], prim_ch[idx],
				  prim_ch_idx[idx], txStream[idx]));
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() todo\n", __func__));
}


static void init_rf_cr(RTMP_ADAPTER *ad)
{
}

static void antenna_default_reset(
	struct _RTMP_ADAPTER *pAd,
	EEPROM_ANTENNA_STRUC *pAntenna)
{
#ifdef DBDC_MODE
	USHORT value = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 max_nss = cap->mcs_nss.max_nss;
#else
#ifdef TXBF_SUPPORT
	USHORT value = 0;
#endif
#endif

	/* TODO: shiang-MT7615 */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() todo\n", __func__));
	pAntenna->word = 0;
	pAd->RfIcType = RFIC_7915;

#ifndef TXBF_SUPPORT
	pAntenna->field.TxPath = 4;
	pAntenna->field.RxPath = 4;
#else
	RT28xx_EEPROM_READ16(pAd, EEPROM_ANTENNA_CFG_OFFSET, value);
	pAntenna->field.TxPath = (value & TX_MASK) >> TX_OFFSET;
	pAntenna->field.RxPath = (value & RX_MASK) >> RX_OFFSET;

	if ((pAntenna->field.TxPath == 0) || (pAntenna->field.TxPath > 4))
		pAntenna->field.TxPath = 4;

	if ((pAntenna->field.RxPath == 0) || (pAntenna->field.RxPath > 4))
		pAntenna->field.RxPath = 4;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() value = 0x%x, TxPath = %d, RxPath = %d\n",
		__func__, value, pAntenna->field.TxPath, pAntenna->field.RxPath));
#endif /* TXBF_SUPPORT */

#ifdef DBDC_MODE

	if (max_nss == 4) {
		RT28xx_EEPROM_READ16(pAd, EEPROM_DBDC_ANTENNA_CFG_OFFSET, value);
		value &= 0xFF;
		pAd->dbdc_band0_rx_path = (value & DBDC_BAND0_RX_MASK) >> DBDC_BAND0_RX_OFFSET;
		pAd->dbdc_band0_tx_path = (value & DBDC_BAND0_TX_MASK) >> DBDC_BAND0_TX_OFFSET;
		pAd->dbdc_band1_rx_path = (value & DBDC_BAND1_RX_MASK) >> DBDC_BAND1_RX_OFFSET;
		pAd->dbdc_band1_tx_path = (value & DBDC_BAND1_TX_MASK) >> DBDC_BAND1_TX_OFFSET;

		if ((pAd->dbdc_band0_rx_path == 0) || (pAd->dbdc_band0_rx_path > 2))
			pAd->dbdc_band0_rx_path = 2;

		if ((pAd->dbdc_band0_tx_path == 0) || (pAd->dbdc_band0_tx_path > 2))
			pAd->dbdc_band0_tx_path = 2;

		if ((pAd->dbdc_band1_rx_path == 0) || (pAd->dbdc_band1_rx_path > 2))
			pAd->dbdc_band1_rx_path = 2;

		if ((pAd->dbdc_band1_tx_path == 0) || (pAd->dbdc_band1_tx_path > 2))
			pAd->dbdc_band1_tx_path = 2;

	} else {
		pAd->dbdc_band0_rx_path = 1;
		pAd->dbdc_band0_tx_path = 1;
		pAd->dbdc_band1_rx_path = 1;
		pAd->dbdc_band1_tx_path = 1;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): DBDC BAND0 TxPath = %d, RxPath = %d\n",
		__func__, pAd->dbdc_band0_tx_path, pAd->dbdc_band0_rx_path));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): DBDC BAND1 TxPath = %d, RxPath = %d\n",
		__func__, pAd->dbdc_band1_tx_path, pAd->dbdc_band1_rx_path));
#endif
}


static VOID fw_prepare(RTMP_ADAPTER *pAd)
{
	struct fwdl_ctrl *ctrl = &pAd->MCUCtrl.fwdl_ctrl;

#ifdef NEED_ROM_PATCH
	if (IS_MT7915_FW_VER_E1(pAd)) {
		ctrl->patch_profile[WM_CPU].source.header_ptr = mt7915_rom_patch_e1;
		ctrl->patch_profile[WM_CPU].source.header_len = sizeof(mt7915_rom_patch_e1);
		ctrl->patch_profile[WM_CPU].source.bin_name = MT7915_ROM_PATCH_BIN_FILE_NAME_E1;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():using E1 ROM patch\n", __func__));
	} else if (IS_MT7915_FW_VER_E2(pAd)) {
		ctrl->patch_profile[WM_CPU].source.header_ptr = mt7915_rom_patch_e2;
		ctrl->patch_profile[WM_CPU].source.header_len = sizeof(mt7915_rom_patch_e2);
		ctrl->patch_profile[WM_CPU].source.bin_name = MT7915_ROM_PATCH_BIN_FILE_NAME_E2;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():using E2 ROM patch\n", __func__));
	} else {
		/* Use E2 rom patch as default */
		ctrl->patch_profile[WM_CPU].source.header_ptr = mt7915_rom_patch_e2;
		ctrl->patch_profile[WM_CPU].source.header_len = sizeof(mt7915_rom_patch_e2);
		ctrl->patch_profile[WM_CPU].source.bin_name = MT7915_ROM_PATCH_BIN_FILE_NAME_E2;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Rom patch is not E1 or E2, so using the default E2 ROM patch\n", __func__));

	}
#endif /* NEED_ROM_PATCH */
	if (IS_MT7915_FW_VER_E1(pAd)) {
		ctrl->fw_profile[WM_CPU].source.header_ptr = MT7915_FirmwareImage_E1;
		ctrl->fw_profile[WM_CPU].source.header_len = sizeof(MT7915_FirmwareImage_E1);
		ctrl->fw_profile[WM_CPU].source.bin_name = MT7915_BIN_FILE_NAME_E1;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():using E1 RAM\n", __func__));
	} else if (IS_MT7915_FW_VER_E2(pAd)) {
		ctrl->fw_profile[WM_CPU].source.header_ptr = MT7915_FirmwareImage_E2;
		ctrl->fw_profile[WM_CPU].source.header_len = sizeof(MT7915_FirmwareImage_E2);
		ctrl->fw_profile[WM_CPU].source.bin_name = MT7915_BIN_FILE_NAME_E2;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():using E2 RAM\n", __func__));
	} else {
		ctrl->fw_profile[WM_CPU].source.header_ptr = MT7915_FirmwareImage_E2;
		ctrl->fw_profile[WM_CPU].source.header_len = sizeof(MT7915_FirmwareImage_E2);
		ctrl->fw_profile[WM_CPU].source.bin_name = MT7915_BIN_FILE_NAME_E2;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():RAM is not E1 or E2, so using the default E2 RAM\n", __func__));
	}
	ctrl->fw_profile[WA_CPU].source.header_ptr = MT7915_WA_FirmwareImage;
	ctrl->fw_profile[WA_CPU].source.header_len = sizeof(MT7915_WA_FirmwareImage);
	ctrl->fw_profile[WA_CPU].source.bin_name = MT7915_WA_BIN_FILE_NAME;
}

static INT32 mt7915_kick_out_fwdl_msg(PRTMP_ADAPTER pAd, struct cmd_msg *msg)
{
	if (pAd->MCUCtrl.fwdl_ctrl.stage == FWDL_STAGE_SCATTER)
		return hif_kick_out_fwdl_msg(pAd, msg);
	else
		return hif_kick_out_cmd_msg(pAd, msg);
}

static UINT32 mt7915_get_wf_path_comb(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	BOOLEAN dbdc_mode_en,
	UINT8 *path,
	UINT8 *path_len)
{
	UINT8 i = 0;
	UINT8 wf_path_begin[2] = {0, 2};

	/* sanity check for null pointer */
	if (!path)
		return FALSE;

	/* sanity check for null pointer */
	if (!path_len)
		return FALSE;

	if (dbdc_mode_en) {
		*path_len = 2;
		for (i = 0; i < *path_len; i++)
			*(path + i) = wf_path_begin[band_idx] + i;
	} else {
		*path_len = 4;
		for (i = 0; i < *path_len; i++)
			*(path + i) = i;
	}

	if (*path_len > MAX_ANTENNA_NUM)
		return FALSE;

	return FALSE;
}

static UINT32 mt7915_get_rx_stat_band(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	UINT8 blk_idx,
	TEST_RX_STAT_BAND_INFO * rx_band_info)
{
	TESTMODE_STATISTIC_INFO_BAND stat_info_band;

	os_zero_mem(&stat_info_band, sizeof(TESTMODE_STATISTIC_INFO_BAND));
	mt_cmd_get_rx_stat_band(pAd, band_idx, &stat_info_band);

	rx_band_info->mac_rx_fcs_err_cnt = stat_info_band.mac_rx_fcs_err_cnt;
	rx_band_info->mac_rx_mdrdy_cnt = stat_info_band.mac_rx_mdrdy_cnt;
	rx_band_info->mac_rx_len_mismatch = stat_info_band.mac_rx_len_mismatch;
	rx_band_info->mac_rx_fcs_ok_cnt = stat_info_band.mac_rx_fcs_ok_cnt;
	rx_band_info->phy_rx_fcs_err_cnt_cck = stat_info_band.phy_rx_fcs_err_cnt_cck;
	rx_band_info->phy_rx_fcs_err_cnt_ofdm = stat_info_band.phy_rx_fcs_err_cnt_ofdm;
	rx_band_info->phy_rx_pd_cck = stat_info_band.phy_rx_pd_cck;
	rx_band_info->phy_rx_pd_ofdm = stat_info_band.phy_rx_pd_ofdm;
	rx_band_info->phy_rx_sig_err_cck = stat_info_band.phy_rx_sig_err_cck;
	rx_band_info->phy_rx_sfd_err_cck = stat_info_band.phy_rx_sfd_err_cck;
	rx_band_info->phy_rx_sig_err_ofdm = stat_info_band.phy_rx_sig_err_ofdm;
	rx_band_info->phy_rx_tag_err_ofdm = stat_info_band.phy_rx_tag_err_ofdm;
	rx_band_info->phy_rx_mdrdy_cnt_cck = stat_info_band.phy_rx_mdrdy_cnt_cck;
	rx_band_info->phy_rx_mdrdy_cnt_ofdm = stat_info_band.phy_rx_mdrdy_cnt_ofdm;
	return FALSE;
}

static UINT32 mt7915_get_rx_stat_path(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	UINT8 blk_idx,
	TEST_RX_STAT_PATH_INFO * rx_path_info
)
{
	TESTMODE_STATISTIC_INFO_PATH stat_info_path;
	UINT8 path_idx = 0;

	os_zero_mem(&stat_info_path, sizeof(TESTMODE_STATISTIC_INFO_PATH));
	mt_cmd_get_rx_stat_path(pAd, blk_idx, &stat_info_path);

	/* from fw command report (query bbp cr) */
	rx_path_info->inst_ib_rssi = stat_info_path.inst_ib_rssi;
	rx_path_info->inst_wb_rssi = stat_info_path.inst_wb_rssi;

	/* path index handle for rxv rcpi format (band0: rx0/rx1, band1: rx0/rx1) */
	if (band_idx == 1)
		path_idx = blk_idx - 2;
	else
		path_idx = blk_idx;

	/* Parsing from RXV */
	rx_path_info->rcpi = pAd->rx_stat_rxv[band_idx].RCPI[path_idx];
	rx_path_info->rssi = pAd->rx_stat_rxv[band_idx].RSSI[path_idx];
	rx_path_info->fagc_ib_rssi = pAd->rx_stat_rxv[band_idx].FAGC_RSSI_IB[path_idx];
	rx_path_info->fagc_wb_rssi = pAd->rx_stat_rxv[band_idx].FAGC_RSSI_WB[path_idx];
	return FALSE;
}

static UINT32 mt7915_get_rx_stat_user(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	UINT8 blk_idx,
	TEST_RX_STAT_USER_INFO * rx_user_info
)
{
	INT64 freq_offset_trans;
	UINT8 bw = 20;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->channelbw == BW_20)
		bw = 20;
	else if (cap->channelbw == BW_40)
		bw = 40;
	else if (cap->channelbw == BW_80)
		bw = 80;
	else if (cap->channelbw == BW_160)
		bw = 160;
	else if (cap->channelbw == BW_10)
		bw = 10;
	else if (cap->channelbw == BW_5)
		bw = 5;
	else if (cap->channelbw == BW_8080)
		bw = 80;

	/* Transform freq offset */
	freq_offset_trans = (INT64) pAd->rx_stat_rxv[band_idx].FreqOffsetFromRx[blk_idx];
	freq_offset_trans = (freq_offset_trans * bw * 1000000) >> 24;
	if (pAd->rx_stat_rxv[band_idx].FreqOffsetFromRx[blk_idx] > (1 << 19))
			freq_offset_trans -= (bw * 1000000) >> 4;

	/* Parsing from RXV */
	rx_user_info->freq_offset_from_rx = (INT32)freq_offset_trans;
	rx_user_info->snr = pAd->rx_stat_rxv[band_idx].SNR[blk_idx];
	rx_user_info->fcs_error_cnt = pAd->rx_stat_rxv[band_idx].fcs_error_cnt[blk_idx];
	return FALSE;
}

static UINT32 mt7915_get_rx_stat_comm(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	UINT8 blk_idx,
	TEST_RX_STAT_COMM_INFO * rx_comm_info
)
{
	TESTMODE_STATISTIC_INFO_COMM stat_info_comm;

	os_zero_mem(&stat_info_comm, sizeof(TESTMODE_STATISTIC_INFO_COMM));
	mt_cmd_get_rx_stat_comm(pAd, &stat_info_comm);

	rx_comm_info->rx_fifo_full = stat_info_comm.mac_rx_fifo_full;
	rx_comm_info->aci_hit_low = stat_info_comm.aci_hit_low;
	rx_comm_info->aci_hit_high = stat_info_comm.aci_hit_high;
	return FALSE;
}

static UINT32 mt7915_get_rx_stat(
	struct _RTMP_ADAPTER *pAd,
	UCHAR band_idx,
	P_TESTMODE_STATISTIC_INFO ptest_mode_stat_info)
{
	if (mt_cmd_get_rx_stat(pAd, band_idx,
			ptest_mode_stat_info) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	return FALSE;
}

static BOOLEAN mt7915_check_RF_lock_down(RTMP_ADAPTER *pAd)
{
	BOOL RFlockDown = FALSE;

	if (pAd->EEPROMImage[RF_LOCKDOWN_EEPROME_OFFSET] & RF_LOCKDOWN_EEPROME_MASK)
		RFlockDown = TRUE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: RFlockDown Enable: %d\n", __func__, RFlockDown));

	return RFlockDown;
}

static UINT32 mt7915_get_sku_tbl_idx(RTMP_ADAPTER *pAd, UINT8 *sku_tbl_idx)
{
	UINT8 tbl_idx = 0;
	BOOLEAN rf_lock = FALSE;

	/* read profile for table index */
	tbl_idx = pAd->CommonCfg.SKUTableIdx;

	/* check rf lockdown status*/
	rf_lock = chip_check_rf_lock_down(pAd);

	/* update sku table index */
	if (rf_lock)
		tbl_idx = (pAd->EEPROMImage[RF_LOCKDOWN_EEPROME_SKU_TBL_OFFSET] & RF_LOCKDOWN_EEPROME_SKU_TBL_MASK)
			>> RF_LOCKDOWN_EEPROME_SKU_TBL_SHIFT;

	/* update sku table index */
	*sku_tbl_idx = tbl_idx;

	return 0;
}

static BOOLEAN mt7915_Config_Effuse_Country(RTMP_ADAPTER *pAd)
{
	UCHAR   Buffer0, Buffer1;
	UCHAR   CountryCode[2];

	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Country Region 2G */
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Buffer0 = pAd->EEPROMImage[COUNTRY_REGION_2G_EEPROME_OFFSET];

	/* Check the RF lock status */
	if (Buffer0 != 0xFF) {
		/* Check Validation bit for content */
		if (((Buffer0) & (COUNTRY_REGION_VALIDATION_MASK)) >> (COUNTRY_REGION_VALIDATION_OFFSET))
			pAd->CommonCfg.CountryRegion = ((Buffer0) & (COUNTRY_REGION_CONTENT_MASK));
	}

	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Country Region 5G */
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	Buffer1 = pAd->EEPROMImage[COUNTRY_REGION_5G_EEPROME_OFFSET];

	/* Check the RF lock status */
	if (Buffer1 != 0xFF) {
		/* Check Validation bit for content */
		if (((Buffer1) & (COUNTRY_REGION_VALIDATION_MASK)) >> (COUNTRY_REGION_VALIDATION_OFFSET))
			pAd->CommonCfg.CountryRegionForABand = ((Buffer1) & (COUNTRY_REGION_CONTENT_MASK));
	}

	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Country Code */
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	CountryCode[0] = pAd->EEPROMImage[COUNTRY_CODE_BYTE0_EEPROME_OFFSET];
	CountryCode[1] = pAd->EEPROMImage[COUNTRY_CODE_BYTE1_EEPROME_OFFSET];

	/* Check the RF lock status */
	if ((CountryCode[0] != 0xFF) && (CountryCode[1] != 0xFF)) {
		/* Check Validation for content */
		if ((CountryCode[0] != 0x00) && (CountryCode[1] != 0x00)) {
			pAd->CommonCfg.CountryCode[0] = CountryCode[0];
			pAd->CommonCfg.CountryCode[1] = CountryCode[1];
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pAd->CommonCfg.CountryCode[0]: 0x%x, %c ",
					 pAd->CommonCfg.CountryCode[0], pAd->CommonCfg.CountryCode[0]));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pAd->CommonCfg.CountryCode[1]: 0x%x, %c ",
					 pAd->CommonCfg.CountryCode[1], pAd->CommonCfg.CountryCode[1]));
		}
	}

	return TRUE;
}

#ifndef ANTENNA_DIVERSITY_SUPPORT
static UINT32 mt7915_rxv_entry_hdr_parser(struct _RTMP_ADAPTER *pAd, VOID *rxv_entry, RXV_ENTRY_HDR *rxv_entry_hdr)
{
	UINT32 *ptr = NULL;

	if (!rxv_entry_hdr)
		goto error0;

	if (!rxv_entry)
		goto error1;

	ptr = (UINT32 *) rxv_entry;

	/* rxv pkt header parsing */
	rxv_entry_hdr->rx_sta_airtime = (*(ptr) & BITS(0, 15)) >> 0;
	rxv_entry_hdr->rx_sta_cnt = (*(ptr) & BITS(16, 22)) >> 16;
	rxv_entry_hdr->rxv_sn = (*(ptr + 1) & BITS(16, 23)) >> 16;
	rxv_entry_hdr->band_idx = (*(ptr + 1) & BIT(24)) >> 24;
	rxv_entry_hdr->rx_airtime_cal = (*(ptr + 1) & BIT(25)) >> 25;
	rxv_entry_hdr->tr = (*(ptr + 1) & BIT(26)) >> 26;
	rxv_entry_hdr->trig_mpdu = (*(ptr + 1) & BIT(27)) >> 27;

	rxv_entry_hdr->time_stamp = 0;
	rxv_entry_hdr->time_stamp |= ((*(ptr) & BITS(2, 10)) >> 2) << 0;
	rxv_entry_hdr->time_stamp |= ((*(ptr + 1) & BITS(0, 15)) >> 0) << 9;
	rxv_entry_hdr->time_stamp |= ((*(ptr + 1) & BITS(28, 31)) >> 28) << 25;

	return 0;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): null pointer for rxv pkt header.\n", __func__));
	return 1;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): null pointer for rxv pkt.\n", __func__));
	return 1;
}

static UINT32 mt7915_rxv_pkt_hdr_parser(struct _RTMP_ADAPTER *pAd, VOID *rxv_pkt, RXV_PKT_HDR *rxv_pkt_hdr)
{
	UINT32 *ptr = NULL;

	if (!rxv_pkt_hdr)
		goto error0;

	if (!rxv_pkt)
		goto error1;

	ptr = (UINT32 *) rxv_pkt;

	/* rxv pkt header parsing */
	rxv_pkt_hdr->rx_byte_cnt = (*(ptr) & BITS(0, 15)) >> 0;
	rxv_pkt_hdr->rxv_cnt = (*(ptr) & BITS(16, 20)) >> 16;
	rxv_pkt_hdr->pkt_type = (*(ptr) & BITS(29, 31)) >> 29;
#ifdef RT_BIG_ENDIAN
	if ((rxv_pkt_hdr->rx_byte_cnt - 4) > 0)
		MTMacInfoEndianChange(pAd, ptr, TYPE_RMACINFO, rxv_pkt_hdr->rx_byte_cnt);
#endif

	rxv_pkt_hdr->pse_fid = (*(ptr + 1) & BITS(16, 27)) >> 16;

	return 0;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): null pointer for rxv pkt header.\n", __func__));
	return 1;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): null pointer for rxv pkt.\n", __func__));
	return 1;
}
#endif

static UINT32 mt7915_rxv_get_byte_cnt(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	UINT32 *byte_cnt)
{
	PUINT32 ptr = NULL;

	/* read rxv pakcet raw data */

	/* sanity check for null pointer */
	if (!ptr)
		return FALSE;

	/* read byte count */
	*byte_cnt = ((*ptr) & BITS(0, 15)) >> 0;

	return FALSE;
}

static UINT32 mt7915_rxv_get_content(
	struct _RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	PVOID *content)
{
	PUINT32 ptr = NULL;

	/* read rxv pakcet raw data */

	/* sanity check for null pointer */
	if (!ptr)
		goto error;

	return FALSE;

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("(rxv) null pointer for rxv data\n"));
	return TRUE;
}

static UINT32 mt7915_rxv_raw_data_show(struct _RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	PUINT32 ptr = NULL, ptr2 = NULL;
	UINT16 dw_cnt = 0, dw_idx = 0, rxv_cnt = 0, rxv_byte_cnt = 0;
	UINT8 dw_line_cnt = 4;

	/* read rxv pakcet raw data */
	ptr = (PUINT32)(pAd->rxv_raw_data.rxv_pkt);

	/* sanity check for null pointer */
	if (!ptr) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): null pointer for rxv raw dara.\n", __func__));
		return FALSE;
	}

	ptr2 = ptr;
	rxv_cnt = ((*ptr2) & BITS(16, 20)) >> 16;
	rxv_byte_cnt = ((*ptr2) & BITS(0, 15)) >> 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: rxv_cnt: %d, rxv_byte_cnt: %d\n", __func__,
		rxv_cnt, rxv_byte_cnt));

	dw_cnt = rxv_byte_cnt >> 2;
	if (rxv_byte_cnt % 4 != 0)
		dw_cnt++;

	for (dw_idx = 0, ptr2 = ptr; dw_idx < dw_cnt; dw_idx++, ptr2++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("[0x%04X]:%08X  ", dw_idx, *ptr2));

		if ((dw_idx % dw_line_cnt) == (dw_line_cnt - 1))
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	return TRUE;
}

static UINT32 mt7915_rxv_info_show(struct _RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	return FALSE;

}

static UINT32 mt7915_rxv_stat_reset(struct _RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	/* free memory for rxv pkt */
	os_zero_mem(pAd->rxv_raw_data.rxv_pkt, pAd->rxv_raw_data.rxv_byte_cnt);
	os_free_mem(pAd->rxv_raw_data.rxv_pkt);
	pAd->rxv_raw_data.rxv_pkt = NULL;
	pAd->rxv_raw_data.rxv_byte_cnt = 0;

	/* reset rxv count */
	pAd->rx_stat_rxv[band_idx].rxv_cnt = 0;

	/* TODO: reset rssi */

	return 0;
}

static UINT32 mt7915_rxv_cap_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (IS_MT7915_FW_VER_E1(pAd)) {
		cap->rxv_pkt_hdr_dw_num = E1_RXV_PACKET_HEADER_DW_NUM;
		cap->rxv_entry_hdr_dw_num = E1_RXV_ENTRY_HEADER_DW_NUM;
		cap->rxv_cmn1_dw_num = E1_CMN_RXV1_DW_NUM;
		cap->rxv_cmn2_dw_num = E1_CMN_RXV2_DW_NUM;
		cap->rxv_usr1_dw_num = E1_USR_RXV1_DW_NUM;
		cap->rxv_usr2_dw_num = E1_USR_RXV2_DW_NUM;
	} else if (IS_MT7915_FW_VER_E2(pAd)) {
		cap->rxv_pkt_hdr_dw_num = E2_RXV_PACKET_HEADER_DW_NUM;
		cap->rxv_entry_hdr_dw_num = E2_RXV_ENTRY_HEADER_DW_NUM;
		cap->rxv_cmn1_dw_num = E2_CMN_RXV1_DW_NUM;
		cap->rxv_cmn2_dw_num = E2_CMN_RXV2_DW_NUM;
		cap->rxv_usr1_dw_num = E2_USR_RXV1_DW_NUM;
		cap->rxv_usr2_dw_num = E2_USR_RXV2_DW_NUM;
	} else {
		cap->rxv_pkt_hdr_dw_num = E1_RXV_PACKET_HEADER_DW_NUM;
		cap->rxv_entry_hdr_dw_num = E1_RXV_ENTRY_HEADER_DW_NUM;
		cap->rxv_cmn1_dw_num = E1_CMN_RXV1_DW_NUM;
		cap->rxv_cmn2_dw_num = E1_CMN_RXV2_DW_NUM;
		cap->rxv_usr1_dw_num = E1_USR_RXV1_DW_NUM;
		cap->rxv_usr2_dw_num = E1_USR_RXV2_DW_NUM;
	}

	return 0;
}

static UINT32 mt7915_rxv_entry_parse(struct _RTMP_ADAPTER *pAd, VOID *Data)
{
	RX_STATISTIC_RXV *rx_stat;
	INT32 foe = 0;
	UINT32 i = 0;
	PUINT32 pbuf = (PUINT32)Data, pbuf2 = (PUINT32)Data, pbuf3 = (PUINT32)Data;
	UINT8 sta_cnt = 0, user_idx, band_idx;
	UINT32 rxv_entry_dw_cnt = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* read sta count */
	sta_cnt = (*pbuf & BITS(16, 22)) >> 16;

	/* decide band index */
	band_idx = (*(pbuf + 1) & BIT(24)) >> 24;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): sta_cnt: %d, band_idx: %d\n",
			__func__, sta_cnt, band_idx));

	/* sanity check for sta count */
	if (sta_cnt > MAX_USER_NUM)
		goto error1;

	/* sanity check for band index */
	if (band_idx >= DBDC_BAND_NUM)
		goto error2;

	rx_stat = pAd->rx_stat_rxv + band_idx;

	/* rxv raw data log (header) */
	for (i = 0, pbuf2 = pbuf3; i < cap->rxv_entry_hdr_dw_num; i++, pbuf2++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("HEADER(%d): 0x%x\n", i, *pbuf2));
	}
	pbuf3 += cap->rxv_entry_hdr_dw_num;

	/* rxv raw data log (CMN1) */
	for (i = 0, pbuf2 = pbuf3; i < cap->rxv_cmn1_dw_num; i++, pbuf2++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("CMN1(%d): 0x%x\n", i, *pbuf2));
	}
	pbuf3 += cap->rxv_cmn1_dw_num;

	for (user_idx = 0; user_idx < sta_cnt; user_idx++) {
		/* rxv raw data log (USR1) */
		for (i = 0, pbuf2 = pbuf3; i < cap->rxv_usr1_dw_num * sta_cnt; i++, pbuf2++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("user_idx: %d, USR1(%d): 0x%x\n", user_idx, i, *pbuf2));
		}
		pbuf3 += cap->rxv_usr1_dw_num;
	}

	for (user_idx = 0, pbuf2 = pbuf3; user_idx < sta_cnt; user_idx++) {
		/* rxv raw data log (USR2) */
		for (i = 0, pbuf2 = pbuf3; i < cap->rxv_usr2_dw_num * sta_cnt; i++, pbuf2++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("user_idx: %d, USR2(%d): 0x%x\n", user_idx, i, *pbuf2));
		}
		pbuf3 += cap->rxv_usr2_dw_num;
	}

	/* rxv raw data log (CMN2) */
	for (i = 0, pbuf2 = pbuf3; i < cap->rxv_cmn2_dw_num; i++, pbuf2++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("CMN2(%d): 0x%x\n", i, *pbuf2));
	}
	pbuf3 += cap->rxv_cmn2_dw_num;

	/* pointer increment (entry header) */
	pbuf += cap->rxv_entry_hdr_dw_num;

	/* rxv raw data parsing (CMN1) */
	/* pfd */
	pbuf2 = pbuf;
	pbuf2 += RXV_DDW0_L;
	rx_stat->pfd = (*pbuf2 & BITS(4, 7)) >> 4;

	/* vht_group_id */
	pbuf2 = pbuf;
	pbuf2 += RXV_DDW1_L;
	rx_stat->vht_gid = (*pbuf2 & BITS(22, 27)) >> 22;

	/* rcpi */
	pbuf2 = pbuf;
	pbuf2 += RXV_DDW3_L;
	rx_stat->RCPI[0] = (*pbuf2 & BITS(0, 7)) >> 0;
	rx_stat->RCPI[1] = (*pbuf2 & BITS(8, 15)) >> 8;
	rx_stat->RCPI[2] = (*pbuf2 & BITS(16, 23)) >> 16;
	rx_stat->RCPI[3] = (*pbuf2 & BITS(24, 31)) >> 24;

	for (i = WF0; i < WF_NUM; i++) {
		rx_stat->RCPI[i] = rx_stat->RCPI[i] + i1RxFeLossComp[band_idx][i];
	}

	/* rssi */
	for (i = WF0; i < WF_NUM; i++) {
		if (rx_stat->RCPI[i] == 0xFF) {
			rx_stat->RCPI[i] = 0;
			rx_stat->RSSI[i] = -127;
		} else {
			rx_stat->RSSI[i] = (rx_stat->RCPI[i] - 220)/2;
		}
	}

	/* fagc rssi (in-band) */
	pbuf2 = pbuf;
	pbuf2 += RXV_DDW3_H;
	rx_stat->FAGC_RSSI_IB[0] = (*pbuf2 & BITS(0, 7)) >> 0;
	rx_stat->FAGC_RSSI_IB[1] = (*pbuf2 & BITS(8, 15)) >> 8;
	rx_stat->FAGC_RSSI_IB[2] = (*pbuf2 & BITS(16, 23)) >> 16;
	rx_stat->FAGC_RSSI_IB[3] = (*pbuf2 & BITS(24, 31)) >> 24;
	for (i = WF0; i < WF_NUM; i++) {
		if (rx_stat->FAGC_RSSI_IB[i] >= 128)
			rx_stat->FAGC_RSSI_IB[i] -= 256;
	}

	/* fagc rssi (wide-band) */
	pbuf2 = pbuf;
	pbuf2 += RXV_DDW4_L;
	rx_stat->FAGC_RSSI_WB[0] = (*pbuf2 & BITS(5, 12)) >> 5;
	rx_stat->FAGC_RSSI_WB[1] = (*pbuf2 & BITS(14, 21)) >> 14;
	rx_stat->FAGC_RSSI_WB[2] = (*pbuf2 & BITS(23, 30)) >> 23;
	pbuf2 = pbuf;
	pbuf2 += RXV_DDW4_H;
	rx_stat->FAGC_RSSI_WB[3] = (*pbuf2 & BITS(0, 7)) >> 0;
	for (i = WF0; i < WF_NUM; i++) {
		if (rx_stat->FAGC_RSSI_WB[i] >= 128)
			rx_stat->FAGC_RSSI_WB[i] -= 256;
	}

	for (i = WF0; i < WF_NUM; i++) {
		rx_stat->FAGC_RSSI_IB[i] = rx_stat->FAGC_RSSI_IB[i] + i1RxFeLossComp[band_idx][i];
		rx_stat->FAGC_RSSI_WB[i] = rx_stat->FAGC_RSSI_WB[i] + i1RxFeLossComp[band_idx][i];
	}

	/* pointer increment (CMN1) */
	pbuf += cap->rxv_cmn1_dw_num;

	/* rxv raw data parsing (USR1) */
	for (user_idx = 0; user_idx < sta_cnt; user_idx++) {

		/* rx_vld_ind */
		pbuf2 = pbuf;
		pbuf2 += RXV_DDW0_H;
		rx_stat->rx_vld_ind[user_idx] = (*pbuf2 & BIT(31)) >> 31;

		/* pointer increment (USR1) */
		pbuf += cap->rxv_usr1_dw_num;
	}

	/* rxv raw data parsing (USR2) */
	for (user_idx = 0; user_idx < sta_cnt; user_idx++) {
		/* foe */
		pbuf2 = pbuf;
		pbuf2 += RXV_DDW0_L;
		foe = 0;
		foe |= (*pbuf2 & BITS(19, 31)) >> 19;
		pbuf2 = pbuf;
		pbuf2 += RXV_DDW0_H;
		foe |= (((*pbuf2 & BITS(0, 6)) >> 0) << (31-19+1));
		rx_stat->FreqOffsetFromRx[user_idx] = foe;

		/* snr */
		pbuf2 = pbuf;
		pbuf2 += RXV_DDW0_L;
		rx_stat->SNR[user_idx] = (*pbuf2 & BITS(13, 18)) >> 13;
		rx_stat->SNR[user_idx] -= 16;

		/* fcs error */
		pbuf2 = pbuf;
		pbuf2 += RXV_DDW0_H;
		rx_stat->fcs_error[user_idx] = (*pbuf2 & BIT(13)) >> 13;
		if (rx_stat->fcs_error[user_idx])
			rx_stat->fcs_error_cnt[user_idx]++;

		/* pointer increment (USR2) */
		pbuf += cap->rxv_usr2_dw_num;
	}

	/* rxv raw data parsing (CMN2) */

	/* pointer increment (CMN2) */
	pbuf += cap->rxv_cmn2_dw_num;

	/* u4RxMuPktCount */
	for (user_idx = 0; user_idx < sta_cnt; user_idx++) {
		if (rx_stat->pfd == TXMODE_VHT && !rx_stat->fcs_error[user_idx] &&
			(rx_stat->vht_gid != 0 && rx_stat->vht_gid != 63)) {
				rx_stat->rx_mu_ok_cnt[user_idx]++;
#ifdef CFG_SUPPORT_MU_MIMO
				if (user_idx == 0)
					pAd->u4RxMuPktCount++;
#endif
		} else if (rx_stat->pfd == TXMODE_HE_MU && !rx_stat->fcs_error[user_idx]) {
				rx_stat->rx_mu_ok_cnt[user_idx]++;
#ifdef CFG_SUPPORT_MU_MIMO
				if (user_idx == 0)
					pAd->u4RxMuPktCount++;
#endif
		}
	}

	/* update rxv count */
	pAd->rx_stat_rxv[band_idx].rxv_cnt++;

	/* compute memory buffer size rxv entry raw data */
	rxv_entry_dw_cnt += cap->rxv_entry_hdr_dw_num;
	rxv_entry_dw_cnt += cap->rxv_cmn1_dw_num + cap->rxv_cmn2_dw_num;
	rxv_entry_dw_cnt += (cap->rxv_usr1_dw_num + cap->rxv_usr2_dw_num) * sta_cnt;

	for (user_idx = 0; user_idx < sta_cnt; user_idx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("\t user_idx: %d, FreqOffsetFromRx = %d \n",
			user_idx, rx_stat->FreqOffsetFromRx[user_idx]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("\t user_idx: %d, fcs_error: %d, fcs error cnt: %d \n",
			user_idx, rx_stat->fcs_error[user_idx], rx_stat->fcs_error_cnt[user_idx]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("\t user_idx: %d, rx_vld_ind: %d, fcs_error: %d, rx mu ok cnt: %d \n",
			user_idx, rx_stat->rx_vld_ind[user_idx], rx_stat->fcs_error[user_idx],
			rx_stat->rx_mu_ok_cnt[user_idx]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("\t user_idx: %d,  SNR: %d \n",
			user_idx, rx_stat->SNR[user_idx]));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("\t pfd(txmode): %d, vht_gid: %d \n",
		rx_stat->pfd, rx_stat->vht_gid));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("\t RCPI: (%d,%d,%d,%d) \n",
		rx_stat->RCPI[0], rx_stat->RCPI[1], rx_stat->RCPI[2], rx_stat->RCPI[3]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("\t RSSI: (%d,%d,%d,%d) \n",
		rx_stat->RSSI[0], rx_stat->RSSI[1], rx_stat->RSSI[2], rx_stat->RSSI[3]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("\t FAGC_RSSI_IB: (%d,%d,%d,%d) \n",
		rx_stat->FAGC_RSSI_IB[0], rx_stat->FAGC_RSSI_IB[1],
		rx_stat->FAGC_RSSI_IB[2], rx_stat->FAGC_RSSI_IB[3]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("\t FAGC_RSSI_WB: (%d,%d,%d,%d) \n",
		rx_stat->FAGC_RSSI_WB[0], rx_stat->FAGC_RSSI_WB[1],
		rx_stat->FAGC_RSSI_WB[2], rx_stat->FAGC_RSSI_WB[3]));

	return 0;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s(): sta count is invalid(%d).\n", __func__, sta_cnt));
	return 1;

error2:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s(): band index is invalid(%d).\n", __func__, band_idx));
	return 1;
}

static UINT32 mt7915_rxv_content_len(struct _RTMP_ADAPTER *pAd, UINT8 type_mask, UINT8 rxv_sta_cnt, UINT16 *len)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (!len)
		return 1;

	/* rxv content length init */
	*len = 0;

	/*  update rxv content length (cmn1) */
	if (type_mask & BIT(RXV_CONTENT_CMN1))
		*len += cap->rxv_cmn1_dw_num;

	/*  update rxv content length (cmn2) */
	if (type_mask & BIT(RXV_CONTENT_CMN2))
		*len += cap->rxv_cmn2_dw_num;

	/*  update rxv content length (usr1) */
	if (type_mask & BIT(RXV_CONTENT_USR1))
		*len += (cap->rxv_usr1_dw_num * rxv_sta_cnt);

	/*  update rxv content length (usr2) */
	if (type_mask & BIT(RXV_CONTENT_USR2))
		*len += (cap->rxv_usr2_dw_num * rxv_sta_cnt);

	return 0;
}

static UINT32 mt7915_rxv_dump_start(struct _RTMP_ADAPTER *pAd)
{
	if (!pAd->rxv_dump_ctrl.alloc)
		goto error0;

	pAd->rxv_dump_ctrl.enable = true;
	return 0;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): dump list not alloc for start dump action.\n", __func__));
	return 1;
}

static UINT32 mt7915_rxv_dump_stop(struct _RTMP_ADAPTER *pAd)
{
	pAd->rxv_dump_ctrl.enable = false;
	return 0;
}

static UINT32 mt7915_rxv_dump_link_list_instl(
	struct _RTMP_ADAPTER *pAd, DL_LIST *list,
	VOID *ptr, RXV_DUMP_LIST_TYPE type)
{
	UINT32 ret = 0;
	VOID *ptr_tmp = NULL;
	UINT8 len = 0;
	RXV_DUMP_ENTRY_CONTENT *rxv_dump_entry_content = NULL;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry = NULL;
	RXV_DUMP_ENTRY *rxv_dump_entry = NULL;

	if (!ptr)
		goto error1;

	switch (type) {
	case RXV_DUMP_LIST_TYPE_CONTENT:
		len = sizeof(RXV_DUMP_ENTRY_CONTENT);
		break;
	case RXV_DUMP_LIST_TYPE_BASIC_ENTRY:
		len = sizeof(RXV_DUMP_BASIC_ENTRY);
		break;
	case RXV_DUMP_LIST_TYPE_ENTRY:
		len = sizeof(RXV_DUMP_ENTRY);
		break;
	default:
		goto error2;
	}

	ret = os_alloc_mem(pAd, (UINT8 **)&ptr_tmp, len);
	if (ret)
		goto error0;

	/* clear allocated memory for rxv dump entry content */
	os_zero_mem(ptr_tmp, len);
	/* update rxv dump entry content */
	os_move_mem(ptr_tmp, ptr, len);

	switch (type) {
	case RXV_DUMP_LIST_TYPE_CONTENT:
		rxv_dump_entry_content = (RXV_DUMP_ENTRY_CONTENT *) ptr_tmp;
		/* add rxv dump entry entity  to link list  */
		DlListAddTail(list, &rxv_dump_entry_content->list);
		break;
	case RXV_DUMP_LIST_TYPE_BASIC_ENTRY:
		rxv_dump_basic_entry = (RXV_DUMP_BASIC_ENTRY *) ptr_tmp;
		/* add rxv dump entry entity to link list  */
		DlListAddTail(list, &rxv_dump_basic_entry->list);
		break;
	case RXV_DUMP_LIST_TYPE_ENTRY:
		rxv_dump_entry = (RXV_DUMP_ENTRY *) ptr_tmp;
		/* add rxv dump entry entity to link list */
		DlListAddTail(list, &rxv_dump_entry->list);
		break;
	default:
		goto error2;
	}

	return 0;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): allocate memory fail.\n", __func__));
	return 1;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for data entry.\n", __func__));
	return 1;

error2:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): invalid type for rxv dump link list.\n", __func__));
	return 1;
}

static UINT32 mt7915_rxv_dump_link_list_remv(struct _RTMP_ADAPTER *pAd, VOID *ptr, RXV_DUMP_LIST_TYPE type)
{
	RXV_DUMP_ENTRY_CONTENT *rxv_dump_entry_content = NULL;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry = NULL;
	RXV_DUMP_ENTRY *rxv_dump_entry = NULL;

	if (!ptr)
		goto error1;

	switch (type) {
	case RXV_DUMP_LIST_TYPE_CONTENT:
		rxv_dump_entry_content = (RXV_DUMP_ENTRY_CONTENT *) ptr;
		/* delete link for this entry */
		DlListDel(&rxv_dump_entry_content->list);
		/* free memory for rxv content */
		if (rxv_dump_entry_content->content) {
			os_free_mem(rxv_dump_entry_content->content);
			rxv_dump_entry_content->content = NULL;
		}
		/* free memory for rxv dump entry content */
		if (rxv_dump_entry_content) {
			os_free_mem(rxv_dump_entry_content);
			/* reset to null pointer */
			rxv_dump_entry_content = NULL;
		}
		break;
	case RXV_DUMP_LIST_TYPE_BASIC_ENTRY:
		rxv_dump_basic_entry = (RXV_DUMP_BASIC_ENTRY *) ptr;
		/* delete link for this entry */
		DlListDel(&rxv_dump_basic_entry->list);
		/* free memory for rxv dump basic entry */
		if (rxv_dump_basic_entry) {
			os_free_mem(rxv_dump_basic_entry);
			/* reset to null pointer */
			rxv_dump_basic_entry = NULL;
		}
		break;
	case RXV_DUMP_LIST_TYPE_ENTRY:
		rxv_dump_entry = (RXV_DUMP_ENTRY *) ptr;
		/* delete link for this entry */
		DlListDel(&rxv_dump_entry->list);
		/* free memory for rxv dump entry */
		if (rxv_dump_entry) {
			os_free_mem(rxv_dump_entry);
			/* reset to null pointer */
			rxv_dump_entry = NULL;
		}
		break;
	default:
		goto error2;
	}

	return 0;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for data entry.\n", __func__));
	return 1;

error2:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): invalid type for rxv dump link list.\n", __func__));
	return 1;
}

static UINT32 mt7915_rxv_dump_buf_alloc(struct _RTMP_ADAPTER *pAd, UINT8 type_mask)
{
	INT32 ret = 0;
	UINT8 dump_entry_num = 10, entry_idx = 0, type_idx = 0, type_num = 0;
	RXV_DUMP_ENTRY rxv_dump_entry;
	DL_LIST *rxv_dump_entry_list = NULL;

	if (pAd->rxv_dump_ctrl.enable)
		goto error2;

	if (pAd->rxv_dump_ctrl.rxv_dump_entry_list)
		goto error1;

	ret = os_alloc_mem(pAd, (UINT8 **)&pAd->rxv_dump_ctrl.rxv_dump_entry_list, sizeof(DL_LIST));
	if (ret)
		goto error3;

	rxv_dump_entry_list = pAd->rxv_dump_ctrl.rxv_dump_entry_list;
	DlListInit(rxv_dump_entry_list);

	/* decide number of type */
	for (type_idx = 0, type_num = 0; type_idx < RXV_CONTENT_NUM; type_idx++) {
		if (type_mask & BIT(type_idx))
			type_num++;
	}

	/* for loop link list all rxv dump entry */
	for (entry_idx = 0; entry_idx < dump_entry_num; entry_idx++) {
		/* clear temp buffer */
		os_zero_mem(&rxv_dump_entry, sizeof(RXV_DUMP_ENTRY));
		/* config entry index and type num */
		rxv_dump_entry.entry_idx = entry_idx;
		rxv_dump_entry.type_num = type_num;

		ret = os_alloc_mem(pAd, (UINT8 **)&rxv_dump_entry.rxv_dump_basic_entry_list, sizeof(DL_LIST));
		if (ret)
			goto error3;
		DlListInit(rxv_dump_entry.rxv_dump_basic_entry_list);

		/* allocate entry and link to list */
		ret = mt7915_rxv_dump_link_list_instl(pAd,
			rxv_dump_entry_list, &rxv_dump_entry, RXV_DUMP_LIST_TYPE_ENTRY);
		if (ret) {
			os_free_mem(rxv_dump_entry.rxv_dump_basic_entry_list);
			rxv_dump_entry.rxv_dump_basic_entry_list = NULL;
			goto error0;
		}
	}

	/* config rxv dump control */
	pAd->rxv_dump_ctrl.type_mask = type_mask;
	pAd->rxv_dump_ctrl.type_num = type_num;
	pAd->rxv_dump_ctrl.ring_idx = 0;
	pAd->rxv_dump_ctrl.dump_entry_total_num = DlListLen(rxv_dump_entry_list);
	pAd->rxv_dump_ctrl.valid_entry_num = 0;
	pAd->rxv_dump_ctrl.alloc = true;

	return 0;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): link list install fail.\n", __func__));
	return 1;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): no need to alloc buf for nonempty list.\n", __func__));
	return 1;

error2:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): cannot alloc buf when enable dump process.\n", __func__));
	return 1;

error3:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): allocate memory fail.\n", __func__));
	return 1;
}

static UINT32 mt7915_rxv_dump_buf_clear(struct _RTMP_ADAPTER *pAd)
{
	UINT32 ret = 0;
	RXV_DUMP_ENTRY_CONTENT *rxv_dump_entry_content = NULL, *rxv_dump_entry_content_tmp = NULL;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry = NULL, *rxv_dump_basic_entry_tmp = NULL;
	RXV_DUMP_ENTRY *rxv_dump_entry = NULL, *rxv_dump_entry_tmp = NULL;
	DL_LIST *rxv_dump_basic_entry_list = NULL, *data_list = NULL;
	UINT32 dl_len = 0;

	if (pAd->rxv_dump_ctrl.enable)
		goto error2;

	/* loop for rxv dump entry list */
	if (!pAd->rxv_dump_ctrl.rxv_dump_entry_list)
		goto error3;

	dl_len = DlListLen(pAd->rxv_dump_ctrl.rxv_dump_entry_list);
	if (dl_len == 0)
		goto error1;

	DlListForEachSafe(rxv_dump_entry, rxv_dump_entry_tmp,
		pAd->rxv_dump_ctrl.rxv_dump_entry_list, RXV_DUMP_ENTRY, list) {
		rxv_dump_basic_entry_list = rxv_dump_entry->rxv_dump_basic_entry_list;
		if (!rxv_dump_basic_entry_list)
			continue;

		dl_len = DlListLen(rxv_dump_basic_entry_list);
		if (dl_len == 0)
			continue;

		/* loop for rxv dump basic entry list */
		DlListForEachSafe(rxv_dump_basic_entry, rxv_dump_basic_entry_tmp,
			rxv_dump_basic_entry_list, RXV_DUMP_BASIC_ENTRY, list) {
			data_list = rxv_dump_basic_entry->data_list;
			if (!data_list)
				continue;

			dl_len = DlListLen(data_list);
			if (dl_len == 0)
				continue;

			/* loop for rxv dump entry content list */
			DlListForEachSafe(rxv_dump_entry_content, rxv_dump_entry_content_tmp,
				data_list, RXV_DUMP_ENTRY_CONTENT, list) {
				ret = mt7915_rxv_dump_link_list_remv(pAd,
					rxv_dump_entry_content, RXV_DUMP_LIST_TYPE_CONTENT);
				if (ret)
					goto error0;
			}

			os_free_mem(data_list);
			/* reset to null pointer */
			data_list = NULL;

			ret = mt7915_rxv_dump_link_list_remv(pAd, rxv_dump_basic_entry, RXV_DUMP_LIST_TYPE_BASIC_ENTRY);
			if (ret)
				goto error0;
		}

		os_free_mem(rxv_dump_basic_entry_list);
		/* reset to null pointer */
		rxv_dump_basic_entry_list = NULL;

		ret = mt7915_rxv_dump_link_list_remv(pAd, rxv_dump_entry, RXV_DUMP_LIST_TYPE_ENTRY);
		if (ret)
			goto error0;
	}

	os_free_mem(pAd->rxv_dump_ctrl.rxv_dump_entry_list);
	/* reset to null pointer */
	pAd->rxv_dump_ctrl.rxv_dump_entry_list = NULL;

	/* config rxv dump control */
	pAd->rxv_dump_ctrl.type_mask = 0;
	pAd->rxv_dump_ctrl.type_num = 0;
	pAd->rxv_dump_ctrl.ring_idx = 0;
	pAd->rxv_dump_ctrl.dump_entry_total_num = 0;
	pAd->rxv_dump_ctrl.valid_entry_num = 0;
	pAd->rxv_dump_ctrl.alloc = false;

	return 0;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): link list remove fail.\n", __func__));
	return 1;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): no need to clear empty list.\n", __func__));
	return 1;

error2:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): cannot alloc buf when enable dump process.\n", __func__));
	return 1;

error3:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for dump entry list.\n", __func__));
	return 1;
}

#ifndef ANTENNA_DIVERSITY_SUPPORT
static UINT32 mt7915_rxv_dump_update(struct _RTMP_ADAPTER *pAd, VOID *rxv_pkt, UINT8 rxv_cnt)
{
	UINT32 ret = 0;
	UINT8 rxv_idx = 0, type_idx = 0, type_mask = 0, type_num = 0, user_num = 0, user_idx = 0;
	UINT8 user_num_tbl[RXV_CONTENT_NUM] = {1, 0, 0, 1};
	UINT32 *ptr = NULL;
	UINT16 len = 0, len_byte = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	RXV_DUMP_ENTRY_CONTENT rxv_dump_entry_content;
	RXV_DUMP_ENTRY_CONTENT *rxv_dump_entry_content_ptr = NULL, *rxv_dump_entry_content_ptr2 = NULL;
	RXV_DUMP_BASIC_ENTRY rxv_dump_basic_entry;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry_ptr = NULL, *rxv_dump_basic_entry_ptr2 = NULL;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry_curr = NULL;
	RXV_DUMP_ENTRY *rxv_dump_entry = NULL, *rxv_dump_entry_tmp = NULL, *rxv_dump_entry_curr = NULL;
	DL_LIST *rxv_dump_entry_list = NULL, *rxv_dump_basic_entry_list = NULL, *data_list = NULL;
	RXV_ENTRY_HDR rxv_entry_hdr;
	UINT8 rx_sta_cnt = 0;
	UINT32 dl_len = 0;

	if (!rxv_pkt)
		goto error0;

	if (!pAd->rxv_dump_ctrl.enable)
		return 0;

	ptr = (UINT32 *) rxv_pkt;

	/* rxv pkt header */
	ptr += cap->rxv_pkt_hdr_dw_num;

	type_mask = pAd->rxv_dump_ctrl.type_mask;
	type_num = pAd->rxv_dump_ctrl.type_num;

	rxv_dump_entry_list = pAd->rxv_dump_ctrl.rxv_dump_entry_list;
	if (!rxv_dump_entry_list)
		goto error5;

	dl_len = DlListLen(rxv_dump_entry_list);
	if (dl_len == 0)
		goto error6;

	os_zero_mem(&rxv_entry_hdr, sizeof(RXV_ENTRY_HDR));
	for (rxv_idx = 0; rxv_idx < rxv_cnt; rxv_idx++) {
		/* read entry pointer from link list */
		DlListForEachSafe(rxv_dump_entry, rxv_dump_entry_tmp, rxv_dump_entry_list, RXV_DUMP_ENTRY, list) {
			if (rxv_dump_entry->entry_idx == pAd->rxv_dump_ctrl.ring_idx) {
				rxv_dump_entry_curr = rxv_dump_entry;
				break;
			}
		}

		/* sanity check for null pointer */
		if (!rxv_dump_entry_curr)
			goto error2;

		/* rxv entry header parsing */
		mt7915_rxv_entry_hdr_parser(pAd, ptr, &rxv_entry_hdr);
		rx_sta_cnt = rxv_entry_hdr.rx_sta_cnt;
		user_num_tbl[RXV_CONTENT_USR1] = user_num_tbl[RXV_CONTENT_USR2] = rx_sta_cnt;
		/* pointer increment for rxv entry header */
		ptr += cap->rxv_entry_hdr_dw_num;

		rxv_dump_basic_entry_list = rxv_dump_entry_curr->rxv_dump_basic_entry_list;
		if (!rxv_dump_basic_entry_list)
			goto error7;
		dl_len = DlListLen(rxv_dump_basic_entry_list);
		if (dl_len != 0) {
			/* loop for rxv dump basic entry list */
			DlListForEachSafe(rxv_dump_basic_entry_ptr, rxv_dump_basic_entry_ptr2,
				rxv_dump_basic_entry_list, RXV_DUMP_BASIC_ENTRY, list) {
				data_list = rxv_dump_basic_entry_ptr->data_list;
				if (!data_list)
					continue;
				dl_len = DlListLen(data_list);
				if (dl_len == 0)
					continue;

				/* loop for rxv dump entry content list */
				DlListForEachSafe(rxv_dump_entry_content_ptr, rxv_dump_entry_content_ptr2,
					data_list, RXV_DUMP_ENTRY_CONTENT, list) {
					ret = mt7915_rxv_dump_link_list_remv(pAd,
						rxv_dump_entry_content_ptr, RXV_DUMP_LIST_TYPE_CONTENT);
					if (ret)
						goto error4;
				}

				os_free_mem(data_list);
				/* reset to null pointer */
				data_list = NULL;

				ret = mt7915_rxv_dump_link_list_remv(pAd,
					rxv_dump_basic_entry_ptr, RXV_DUMP_LIST_TYPE_BASIC_ENTRY);
				if (ret)
					goto error4;
			}

			os_free_mem(rxv_dump_basic_entry_list);
			/* reset to null pointer */
			rxv_dump_basic_entry_list = NULL;
		}

		/* allocate memory for basic dump entry list */
		ret = os_alloc_mem(pAd, (UINT8 **)&rxv_dump_entry_curr->rxv_dump_basic_entry_list, sizeof(DL_LIST));
		if (ret)
			goto error3;
		rxv_dump_basic_entry_list = rxv_dump_entry_curr->rxv_dump_basic_entry_list;
		DlListInit(rxv_dump_basic_entry_list);

		for (type_idx = 0; type_idx < RXV_CONTENT_NUM; type_idx++) {
			mt7915_rxv_content_len(pAd, BIT(type_idx), 1, &len);
			len_byte = len << 2;

			if (type_mask & BIT(type_idx)) {
				os_zero_mem(&rxv_dump_basic_entry, sizeof(RXV_DUMP_BASIC_ENTRY));
				/* config basic entry parameter */
				rxv_dump_basic_entry.type_idx = type_idx;
				rxv_dump_basic_entry.len = len_byte;
				user_num = user_num_tbl[type_idx];
				rxv_dump_basic_entry.usr_num = user_num;
				rxv_dump_basic_entry.data_list = NULL;
				ret = os_alloc_mem(pAd, (UINT8 **)&rxv_dump_basic_entry.data_list, sizeof(DL_LIST));
				if (ret)
					goto error3;
				DlListInit(rxv_dump_basic_entry.data_list);
				/* install basic entry to list */
				ret = mt7915_rxv_dump_link_list_instl(pAd, rxv_dump_basic_entry_list,
						&rxv_dump_basic_entry, RXV_DUMP_LIST_TYPE_BASIC_ENTRY);
				if (ret) {
					os_free_mem(rxv_dump_basic_entry.data_list);
					rxv_dump_basic_entry.data_list = NULL;
					goto error3;
				}

				/* read basic entry pointer from link list */
				DlListForEachSafe(rxv_dump_basic_entry_ptr, rxv_dump_basic_entry_ptr2,
					rxv_dump_basic_entry_list, RXV_DUMP_BASIC_ENTRY, list) {
					if (rxv_dump_basic_entry_ptr->type_idx == type_idx) {
						rxv_dump_basic_entry_curr = rxv_dump_basic_entry_ptr;
						break;
					}
				}

				/* rxv content copy process */
				if (rxv_dump_basic_entry_curr) {
					data_list = rxv_dump_basic_entry_curr->data_list;
					if (data_list) {
						for (user_idx = 0; user_idx < user_num; user_idx++) {
							os_zero_mem(&rxv_dump_entry_content, sizeof(RXV_DUMP_ENTRY_CONTENT));
							/* config basic entry parameter */
							rxv_dump_entry_content.user_idx = user_idx;
							rxv_dump_entry_content.len = len_byte;
							rxv_dump_entry_content.content = NULL;
							ret = os_alloc_mem(pAd, (UINT8 **)&rxv_dump_entry_content.content, len_byte);
							if (ret)
								goto error1;

							/* copy rxv content to buffer */
							os_move_mem(rxv_dump_entry_content.content, ptr, len_byte);

							/* install basic entry to list */
							ret = mt7915_rxv_dump_link_list_instl(pAd, data_list,
									&rxv_dump_entry_content, RXV_DUMP_LIST_TYPE_CONTENT);
							if (ret) {
								os_free_mem(rxv_dump_entry_content.content);
								rxv_dump_entry_content.content = NULL;
								goto error3;
							}

							/* pointer increment for rxv entry content */
							ptr += len;
						}
					}
				}
			} else {
				/* pointer increment for rxv entry content */
				ptr += len * user_num_tbl[type_idx];
			}
		}

		/* increment rxv dump control ring index */
		pAd->rxv_dump_ctrl.ring_idx++;
		/* overflow handle for rxv dump control ring index */
		if (pAd->rxv_dump_ctrl.ring_idx >= pAd->rxv_dump_ctrl.dump_entry_total_num)
			pAd->rxv_dump_ctrl.ring_idx -= pAd->rxv_dump_ctrl.dump_entry_total_num;

		if (pAd->rxv_dump_ctrl.valid_entry_num < pAd->rxv_dump_ctrl.dump_entry_total_num)
			pAd->rxv_dump_ctrl.valid_entry_num++;
	}

	return 0;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for rxv pkt.\n", __func__));
	return 1;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): allocate memory fail.\n", __func__));
	return 1;

error2:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for current rxv dump entry.\n", __func__));
	return 1;

error3:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): invalid type for rxv dump link list.\n", __func__));
	return 1;

error4:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): remove dump basic entry list fail.\n", __func__));
	return 1;

error5:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for dump entry list.\n", __func__));
	return 1;

error6:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): empty list for dump entry.\n", __func__));
	return 1;

error7:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for basic dump entry list.\n", __func__));
	return 1;
}
#endif

static UINT32 mt7915_rxv_dump_show_list(struct _RTMP_ADAPTER *pAd)
{
	DL_LIST *rxv_dump_entry_list = NULL, *rxv_dump_basic_entry_list = NULL, *data_list = NULL;
	RXV_DUMP_ENTRY_CONTENT *rxv_dump_entry_content = NULL, *rxv_dump_entry_content_tmp = NULL;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry = NULL, *rxv_dump_basic_entry_tmp = NULL;
	RXV_DUMP_ENTRY *rxv_dump_entry = NULL, *rxv_dump_entry_tmp = NULL;
	UINT32 *ptr = NULL;
	UINT8 dw_idx = 0, dw_num = 0;
	UINT32 dl_len = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): enable: %d, type_mask: 0x%x, type_num: %d\n", __func__,
		pAd->rxv_dump_ctrl.enable,
		pAd->rxv_dump_ctrl.type_mask,
		pAd->rxv_dump_ctrl.type_num));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): ring_idx: %d, dump_entry_total_num: %d, valid_entry_num: %d\n", __func__,
		pAd->rxv_dump_ctrl.ring_idx,
		pAd->rxv_dump_ctrl.dump_entry_total_num,
		pAd->rxv_dump_ctrl.valid_entry_num));

	if (pAd->rxv_dump_ctrl.enable)
		goto error2;

	rxv_dump_entry_list = pAd->rxv_dump_ctrl.rxv_dump_entry_list;
	if (!rxv_dump_entry_list)
		goto error1;

	dl_len = DlListLen(rxv_dump_entry_list);
	if (dl_len == 0)
		goto error0;

	/* list loop for dump entry */
	DlListForEachSafe(rxv_dump_entry, rxv_dump_entry_tmp,
		rxv_dump_entry_list, RXV_DUMP_ENTRY, list) {

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("    entry_idx: %d, type_num: %d\n",
			rxv_dump_entry->entry_idx,
			rxv_dump_entry->type_num));

		rxv_dump_basic_entry_list = rxv_dump_entry->rxv_dump_basic_entry_list;
		if (!rxv_dump_basic_entry_list)
			continue;
		dl_len = DlListLen(rxv_dump_basic_entry_list);
		if (dl_len == 0)
			continue;

		/* list loop for dump basic entry */
		DlListForEachSafe(rxv_dump_basic_entry, rxv_dump_basic_entry_tmp,
			rxv_dump_basic_entry_list, RXV_DUMP_BASIC_ENTRY, list) {

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("        type_idx: %d, len: %d, usr_num: %d\n",
				rxv_dump_basic_entry->type_idx,
				rxv_dump_basic_entry->len,
				rxv_dump_basic_entry->usr_num));

			data_list = rxv_dump_basic_entry->data_list;
			if (!data_list)
				continue;
			dl_len = DlListLen(data_list);
			if (dl_len == 0)
				continue;

			/* list loop for dump entry content */
			DlListForEachSafe(rxv_dump_entry_content, rxv_dump_entry_content_tmp,
				data_list, RXV_DUMP_ENTRY_CONTENT, list) {

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("            user_idx: %d, len: %d\n",
					rxv_dump_entry_content->user_idx,
					rxv_dump_entry_content->len));

				ptr = (UINT32 *) rxv_dump_entry_content->content;
				dw_num = rxv_dump_entry_content->len >> 2;

				if (!ptr)
					continue;

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("                "));
				for (dw_idx = 0; dw_idx < dw_num; dw_idx++, ptr++) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("(DW%02d):%08X  ", dw_idx, *ptr));
					if (dw_idx % 4 == 3) {
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("\n"));
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("                "));
					}
				}
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\n"));
			}
		}
	}

	return 0;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): empty dump entry list.\n", __func__));
	return 1;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for dump entry list.\n", __func__));
	return 1;

error2:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): cannot access list in rxv dumping process.\n", __func__));
	return 1;
}

static UINT32 mt7915_rxv_dump_user_content_compose(struct _RTMP_ADAPTER *pAd, DL_LIST *data_list, UINT8 user_idx,
		VOID *user_content, UINT32 *len)
{
	RXV_DUMP_ENTRY_CONTENT *rxv_dump_entry_content = NULL, *rxv_dump_entry_content_tmp = NULL;
	RXV_DUMP_ENTRY_CONTENT *rxv_dump_entry_content_curr = NULL;
	UINT8 *ptr = NULL;
	UINT32 usr_idx = 0, raw_data_len = 0;

	if (!data_list)
		goto error0;

	if (!user_content)
		goto error1;

	if (!len)
		goto error2;

	ptr = (UINT8 *) user_content;

	/* read entry content pointer from link list */
	DlListForEachSafe(rxv_dump_entry_content, rxv_dump_entry_content_tmp, data_list, RXV_DUMP_ENTRY_CONTENT, list) {
		if (rxv_dump_entry_content->user_idx == user_idx) {
			rxv_dump_entry_content_curr = rxv_dump_entry_content;
			break;
		}
	}

	/* update user content param */
	usr_idx = rxv_dump_entry_content->user_idx;
	raw_data_len = rxv_dump_entry_content->len;

	/* init length */
	*len = 0;

	/* compose user content */
	os_move_mem(ptr, &usr_idx, sizeof(usr_idx));
	*len += sizeof(usr_idx);
	ptr += sizeof(usr_idx);
	os_move_mem(ptr, &raw_data_len, sizeof(raw_data_len));
	*len += sizeof(raw_data_len);
	ptr += sizeof(raw_data_len);
	os_move_mem(ptr, rxv_dump_entry_content->content, raw_data_len);
	*len += raw_data_len;

	return 0;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for data list.\n", __func__));
	return 1;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for user content.\n", __func__));
	return 1;

error2:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for buffer of length.\n", __func__));
	return 1;
}

static UINT32 mt7915_rxv_dump_type_detail_content_compose(
	struct _RTMP_ADAPTER *pAd,
	UINT8 usr_num,
	DL_LIST *data_list,
	VOID *type_detail_content,
	UINT32 *len)
{
	UINT8 *ptr = NULL, *ptr2 = NULL;
	UINT32 usr_cnt = 0, usr_content_len = 0, usr_content_len_sum = 0;
	UINT8 user_idx = 0;

	if (!type_detail_content)
		goto error0;

	if (!len)
		goto error1;

	ptr = (UINT8 *) type_detail_content;

	/* update type detail content param */
	usr_cnt = (UINT32) usr_num;

	/* init length */
	*len = 0;

	/* compose user content */
	os_move_mem(ptr, &usr_cnt, sizeof(usr_cnt));
	*len += sizeof(usr_cnt);
	ptr += sizeof(usr_cnt);

	/* keep pointer to update user content length */
	ptr2 = ptr;
	*len += sizeof(usr_content_len);
	ptr += sizeof(usr_content_len);

	for (user_idx = 0; user_idx < usr_cnt; user_idx++) {
		mt7915_rxv_dump_user_content_compose(pAd, data_list, user_idx, ptr, &usr_content_len);
		*len += usr_content_len;
		ptr += usr_content_len;
		usr_content_len_sum += usr_content_len;
	}

	os_move_mem(ptr2, &usr_content_len_sum, sizeof(usr_content_len_sum));

	return 0;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for type detail content.\n", __func__));
	return 1;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for buffer of length.\n", __func__));
	return 1;
}

static UINT32 mt7915_rxv_dump_type_content_compose(
	struct _RTMP_ADAPTER *pAd,
	DL_LIST *rxv_dump_basic_entry_list,
	UINT8 type_idx,
	VOID *type_content,
	UINT32 *len)
{
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry = NULL, *rxv_dump_basic_entry_tmp = NULL;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry_curr = NULL;
	DL_LIST *data_list = NULL;
	UINT8 *ptr = NULL, *ptr2 = NULL;
	UINT32 param_type_idx = 0, type_detail_content_len = 0;
	UINT8 usr_num = 0;

	if (!rxv_dump_basic_entry_list)
		goto error0;

	if (!type_content)
		goto error1;

	if (!len)
		goto error2;

	ptr = (UINT8 *) type_content;

	/* read entry content pointer from link list */
	DlListForEachSafe(rxv_dump_basic_entry, rxv_dump_basic_entry_tmp,
		rxv_dump_basic_entry_list, RXV_DUMP_BASIC_ENTRY, list) {
		if (rxv_dump_basic_entry->type_idx == type_idx) {
			rxv_dump_basic_entry_curr = rxv_dump_basic_entry;
			break;
		}
	}

	/* update type detail content param */
	if (rxv_dump_basic_entry_curr) {
		param_type_idx = rxv_dump_basic_entry_curr->type_idx;
		usr_num = rxv_dump_basic_entry_curr->usr_num;
		data_list = rxv_dump_basic_entry_curr->data_list;
	}

	/* init length */
	*len = 0;

	/* compose user content */
	os_move_mem(ptr, &param_type_idx, sizeof(param_type_idx));
	*len += sizeof(param_type_idx);
	ptr += sizeof(param_type_idx);

	/* keep pointer to update user content length */
	ptr2 = ptr;
	*len += sizeof(type_detail_content_len);
	ptr += sizeof(type_detail_content_len);

	mt7915_rxv_dump_type_detail_content_compose(pAd,
		usr_num, data_list, ptr, &type_detail_content_len);
	*len += type_detail_content_len;

	os_move_mem(ptr2, &type_detail_content_len, sizeof(type_detail_content_len));

	return 0;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for badic entry list.\n", __func__));
	return 1;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for type content.\n", __func__));
	return 1;

error2:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for buffer of length.\n", __func__));
	return 1;
}

static UINT32 mt7915_rxv_dump_rxv_content_compose(
	struct _RTMP_ADAPTER *pAd,
	UINT8 entry_idx,
	VOID *rxv_content,
	UINT32 *len)
{
	RXV_DUMP_ENTRY *rxv_dump_entry = NULL, *rxv_dump_entry_tmp = NULL;
	RXV_DUMP_ENTRY *rxv_dump_entry_curr = NULL;
	RXV_DUMP_BASIC_ENTRY *rxv_dump_basic_entry = NULL, *rxv_dump_basic_entry_tmp = NULL;
	DL_LIST *rxv_dump_basic_entry_list = NULL, *rxv_dump_entry_list = NULL;
	UINT8 *ptr = NULL;
	UINT32 type_cnt = 0, type_content_len = 0;
	UINT8 type_idx = 0;

	if (!rxv_content)
		goto error1;

	if (!len)
		goto error2;

	ptr = (UINT8 *) rxv_content;

	rxv_dump_entry_list = pAd->rxv_dump_ctrl.rxv_dump_entry_list;
	if (!rxv_dump_entry_list)
		goto error0;

	/* read entry content pointer from link list */
	DlListForEachSafe(rxv_dump_entry, rxv_dump_entry_tmp,
		rxv_dump_entry_list, RXV_DUMP_ENTRY, list) {
		if (rxv_dump_entry->entry_idx == entry_idx) {
			rxv_dump_entry_curr = rxv_dump_entry;
			break;
		}
	}

	/* update type detail content param */
	if (rxv_dump_entry_curr) {
		type_cnt = rxv_dump_entry_curr->type_num;
		rxv_dump_basic_entry_list = rxv_dump_entry_curr->rxv_dump_basic_entry_list;
	}

	if (!rxv_dump_basic_entry_list)
		goto error0;

	/* init length */
	*len = 0;

	/* compose user content */
	os_move_mem(ptr, &type_cnt, sizeof(type_cnt));
	*len += sizeof(type_cnt);
	ptr += sizeof(type_cnt);

	/* recursive update parameters for different types */
	DlListForEachSafe(rxv_dump_basic_entry, rxv_dump_basic_entry_tmp,
		rxv_dump_basic_entry_list, RXV_DUMP_BASIC_ENTRY, list) {
			type_idx = rxv_dump_basic_entry->type_idx;
			mt7915_rxv_dump_type_content_compose(pAd, rxv_dump_basic_entry_list,
				type_idx, ptr, &type_content_len);
			*len += type_content_len;
			ptr += type_content_len;
	}

	return 0;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for entry list.\n", __func__));
	return 1;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for rxv content.\n", __func__));
	return 1;

error2:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): null pointer for buffer of length.\n", __func__));
	return 1;
}

static UINT32 mt7915_rxv_dump_show_rpt(struct _RTMP_ADAPTER *pAd)
{
	UINT32 ret = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32 *ptr = NULL, *ptr2 = NULL;
	UINT16 buf_len = 0;
	UINT8 cmn1_len = 0, cmn2_len = 0, usr1_len = 0, usr2_len = 0;
	UINT32 len = 0, dw_cnt = 0, dw_idx = 0;
	UINT8 valid_entry_num = 0, ring_idx = 0, dump_entry_total_num = 0;
	CHAR idx = 0;
	UINT8 dw_line_cnt = 4, entry_idx = 0;

	/* compute max buffer size */
	cmn1_len = (1 + 1) + (1 + 1) + (1 + 1) + cap->rxv_cmn1_dw_num;
	cmn2_len = (1 + 1) + (1 + 1) + (1 + 1) + cap->rxv_cmn2_dw_num;
	usr1_len = (1 + 1) + (1 + 1) + MAX_USER_NUM * ((1 + 1) + cap->rxv_usr1_dw_num);
	usr2_len = (1 + 1) + (1 + 1) + MAX_USER_NUM * ((1 + 1) + cap->rxv_usr2_dw_num);

	buf_len = (cmn1_len + cmn2_len + usr1_len + usr2_len) << 2;
	ret = os_alloc_mem(pAd, (UINT8 **)&ptr, buf_len);
	if (ret)
		goto error1;

	valid_entry_num = pAd->rxv_dump_ctrl.valid_entry_num;
	ring_idx = pAd->rxv_dump_ctrl.ring_idx;
	dump_entry_total_num = pAd->rxv_dump_ctrl.dump_entry_total_num;

	ptr2 = ptr;
	for (idx = ring_idx - valid_entry_num; idx < ring_idx; idx++) {
		if (idx < 0)
			entry_idx = idx + dump_entry_total_num;
		else
			entry_idx = idx;

		mt7915_rxv_dump_rxv_content_compose(pAd, entry_idx, ptr, &len);

		dw_cnt = len >> 2;
		if (len % 4 != 0)
			dw_cnt += 1;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("entry_idx: %d\n", entry_idx));
		for (dw_idx = 0; dw_idx < dw_cnt; dw_idx++, ptr2++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("DW[%04d]: 0x%08X  ", dw_idx, *ptr2));
			if ((dw_idx % dw_line_cnt) == (dw_line_cnt - 1))
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		}
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

		os_zero_mem(ptr, buf_len);
		ptr2 = ptr;
	}

	os_free_mem(ptr);

	return 0;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): allocate memory fail.\n", __func__));
	return 1;
}

#ifndef ANTENNA_DIVERSITY_SUPPORT
static UINT32 mt7915_rxv_data_dump(struct _RTMP_ADAPTER *pAd, VOID *Data, UINT32 rxv_cnt, UINT32 rxv_byte_cnt)
{
	if (!pAd->rxv_raw_data.rxv_pkt) {
		/* allocate memory for rxv pkt */
		os_alloc_mem(pAd, (UINT8 **)&pAd->rxv_raw_data.rxv_pkt, rxv_byte_cnt);
		pAd->rxv_raw_data.rxv_byte_cnt = rxv_byte_cnt;
	} else {
		if (rxv_byte_cnt != pAd->rxv_raw_data.rxv_byte_cnt) {
			/* free memory for rxv pkt */
			os_zero_mem(pAd->rxv_raw_data.rxv_pkt, pAd->rxv_raw_data.rxv_byte_cnt);
			os_free_mem(pAd->rxv_raw_data.rxv_pkt);
			pAd->rxv_raw_data.rxv_pkt = NULL;
			pAd->rxv_raw_data.rxv_byte_cnt = 0;

			/* re-allocate memory for rxv pkt */
			os_alloc_mem(pAd, (UINT8 **)&pAd->rxv_raw_data.rxv_pkt, rxv_byte_cnt);
			pAd->rxv_raw_data.rxv_byte_cnt = rxv_byte_cnt;
		}
	}

	/* copy rxv packet content to buffer */
	os_move_mem(pAd->rxv_raw_data.rxv_pkt, Data, rxv_byte_cnt);

	/* testmode rxv dump control */
	mt7915_rxv_dump_update(pAd, Data, rxv_cnt);

	return 0;
}
#endif

#ifdef ANTENNA_DIVERSITY_SUPPORT
static UINT32 mt7915_rxv_packet_parse(struct _RTMP_ADAPTER *pAd, VOID *Data)
{
	PUINT32 pbuf = (PUINT32)Data, pbuf2 = NULL, pbuf_for_RSSI = NULL;
	UINT32 byte_cnt_sum = 0;
	UINT8 i, sta_cnt = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 rxv_entry_dw_cnt = 0;
	UINT16 rx_byte_cnt = 0;
	UINT8 rxv_cnt = 0;
	UINT8 rx_rate = 0, rx_nsts = 0, rx_mode = MODE_HE_SU, dbw = 0, band_idx = 0;
	UINT32 Value = 0, rx_rate_cur = 0;

	/* rxv pkt header parsing */
	rx_byte_cnt = (*(pbuf) & BITS(0, 15)) >> 0;
	rxv_cnt = (*(pbuf) & BITS(16, 20)) >> 16;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("RxV Report: RxvCnt: %d, ByteCnt: %d\n",
		 rxv_cnt, rx_byte_cnt));
	/* packet header processing */
	pbuf += cap->rxv_pkt_hdr_dw_num;
	byte_cnt_sum += cap->rxv_pkt_hdr_dw_num << 2;

	for (i = 0, pbuf2 = pbuf; i < rxv_cnt; i++) {
		/*  rxv Hdr section, read sta count */
		sta_cnt = ((*pbuf2) & BITS(16, 22)) >> 16;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): sta_cnt: %d\n", __func__, sta_cnt));

		/* sanity check for sta count */
		if (sta_cnt > MAX_USER_NUM) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s(): sta count is invalid(%d).\n", __func__, sta_cnt));
			return 1;
		}

		/* decide band index */
		band_idx = (*(pbuf + 1) & BIT(24)) >> 24;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s(): sta_cnt: %d, band_idx: %d\n",
				__func__, sta_cnt, band_idx));

		pbuf2 += cap->rxv_entry_hdr_dw_num;
		/* C_B section*/
		dbw = (*pbuf2 & BITS(8, 10)) >> 8;
		rx_mode = (*pbuf2 & BITS(4, 7)) >> 4;
		/* rcpi */
		pbuf_for_RSSI = pbuf2;
		pbuf_for_RSSI += RXV_DDW3_L;
		pAd->RCPI[0] = (*pbuf_for_RSSI & BITS(0, 7)) >> 0;
		pAd->RCPI[1] = (*pbuf_for_RSSI & BITS(8, 15)) >> 8;
		pAd->RCPI[2] = (*pbuf_for_RSSI & BITS(16, 23)) >> 16;
		pAd->RCPI[3] = (*pbuf_for_RSSI & BITS(24, 31)) >> 24;
		/* rssi */
		for (i = WF0; i < WF_NUM; i++) {
			if (pAd->RCPI[i] == 0xFF) {
				pAd->RCPI[i] = 0;
				pAd->RSSI[i] = -127;
			} else {
				pAd->RSSI[i] = (pAd->RCPI[i] - 220)/2;
			}
		}
		pbuf2 += cap->rxv_cmn1_dw_num;
		/* P_B section*/
		/* get phy rate from P_B, only check user1*/
		rx_rate = (*pbuf2  & BITS(0, 6)) >> 0;
		rx_nsts = (*pbuf2  & BITS(7, 9)) >> 7;

		switch (rx_mode) {
			case MODE_HE:
			case MODE_HE_SU:
			case MODE_HE_EXT_SU:
			case MODE_HE_TRIG:
			case MODE_HE_MU:
				rx_rate = (rx_rate & BITS(0, 3));
				get_rate_he(rx_rate, dbw, rx_nsts+1, 0, (ULONG *) &rx_rate_cur);
				if (pAd->rec_start_rx_rate_flag[band_idx]) {
					pAd->rate_rec_sum[band_idx] += rx_rate_cur;
					pAd->rate_rec_num[band_idx]++;
					if (pAd->rate_rec_num[band_idx] % pAd->cn_rate_read_interval == 0) {
						pAd->phy_rate_average[band_idx] = (pAd->rate_rec_num[band_idx] != 0) ?
							(pAd->rate_rec_sum[band_idx] / pAd->rate_rec_num[band_idx]) : 0;
						pAd->rec_start_rx_rate_flag[band_idx] = FALSE;
					}
				}
				break;
			default:
				break;
		}

		pbuf2 += (cap->rxv_usr1_dw_num * sta_cnt);
		/* P_A section*/
		pbuf2 += (cap->rxv_usr2_dw_num * sta_cnt);
		/* C_A section*/

		/* get CN from DDW10_L, only check user1*/
		pbuf2 += RXV_DDW10_L;
		Value = (*pbuf2 & BITS(0, 7));
		if (pAd->rec_start_cn_flag[band_idx]) {
			pAd->cn_rec_sum[band_idx] += Value;
			pAd->cn_rec_num[band_idx]++;
			if (pAd->cn_rec_num[band_idx] % pAd->cn_rate_read_interval == 0) {
				pAd->cn_average[band_idx] = (pAd->cn_rec_num[band_idx] != 0) ?
					(pAd->cn_rec_sum[band_idx] / pAd->cn_rec_num[band_idx]) : 0;
				pAd->rec_start_cn_flag[band_idx] = FALSE;
			}
		}

		rxv_entry_dw_cnt = (cap->rxv_entry_hdr_dw_num + cap->rxv_cmn1_dw_num +
			sta_cnt*(cap->rxv_usr1_dw_num + cap->rxv_usr2_dw_num) + cap->rxv_cmn2_dw_num);

		/* pointer update for next rxv entry */
		pbuf += rxv_entry_dw_cnt;
		byte_cnt_sum += (rxv_entry_dw_cnt << 2);
	}

	/* sanity check for byte count */
	if (byte_cnt_sum != rx_byte_cnt) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("RxV Report: byte_cnt_sum: %d, rxv_byte_cnt: %d\n",
				byte_cnt_sum, rx_byte_cnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Received byte count not equal to rxv_entry byte count required!\n"));
		return 1;
	}

	return 0;
}
#else
static UINT32 mt7915_rxv_packet_parse(struct _RTMP_ADAPTER *pAd, VOID *Data)
{
	PUINT32 pbuf = (PUINT32)Data, pbuf2 = NULL;
	UINT32 byte_cnt_sum = 0;
	UINT8 i, sta_cnt = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 rxv_entry_dw_cnt = 0;
	RXV_PKT_HDR rxv_pkt_hdr;

	/* rxv pkt header parsing */
	os_zero_mem(&rxv_pkt_hdr, sizeof(RXV_PKT_HDR));
	mt7915_rxv_pkt_hdr_parser(pAd, pbuf, &rxv_pkt_hdr);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("RxV Report: RxvCnt: %d, ByteCnt: %d\n",
		 rxv_pkt_hdr.rxv_cnt, rxv_pkt_hdr.rx_byte_cnt));

	mt7915_rxv_data_dump(pAd, Data, rxv_pkt_hdr.rxv_cnt, rxv_pkt_hdr.rx_byte_cnt);

	/* packet header processing */
	pbuf += cap->rxv_pkt_hdr_dw_num;
	byte_cnt_sum += cap->rxv_pkt_hdr_dw_num << 2;

	for (i = 0, pbuf2 = pbuf; i < rxv_pkt_hdr.rxv_cnt; i++) {
		/* read sta count */
		sta_cnt = ((*pbuf2) & BITS(16, 22)) >> 16;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): sta_cnt: %d\n", __func__, sta_cnt));

		/* sanity check for sta count */
		if (sta_cnt > MAX_USER_NUM) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s(): sta count is invalid(%d).\n", __func__, sta_cnt));
			return 1;
		}

		/* parsing rxv entry */
		chip_parse_rxv_entry(pAd, pbuf2);

		/* update rxv entry dw count */
		rxv_entry_dw_cnt = (cap->rxv_entry_hdr_dw_num + cap->rxv_cmn1_dw_num +
			sta_cnt*(cap->rxv_usr1_dw_num + cap->rxv_usr2_dw_num) + cap->rxv_cmn2_dw_num);

		/* pointer update for next rxv entry */
		pbuf2 += rxv_entry_dw_cnt;
		byte_cnt_sum += (rxv_entry_dw_cnt << 2);
	}

	/* sanity check for byte count */
	if (byte_cnt_sum != rxv_pkt_hdr.rx_byte_cnt) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("RxV Report: byte_cnt_sum: %d, rxv_byte_cnt: %d\n",
				byte_cnt_sum, rxv_pkt_hdr.rx_byte_cnt));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Received byte count not equal to rxv_entry byte count required!\n"));
		return 1;
	}

	return 0;
}
#endif

static VOID mt7915_set_mgmt_pkt_txpwr_prctg(RTMP_ADAPTER *pAd,	struct wifi_dev *wdev, UINT8 prctg)
{
	INT8  power_dropLevel = 0;


	if (!wdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: wdev NULL\n", __func__));
		return;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: [TxdPwrOffset]: %d\n", __func__, wdev->mgmt_txd_txpwr_offset));


	if ((prctg > 90) && (prctg < 100))
		power_dropLevel = 0;
	else if ((prctg > 60) && (prctg <= 90))  /* reduce Pwr for 1 dB. */
		power_dropLevel = -2;
	else if ((prctg > 30) && (prctg <= 60))  /* reduce Pwr for 3 dB. */
		power_dropLevel = -6;
	else if ((prctg > 15) && (prctg <= 30))  /* reduce Pwr for 6 dB. */
		power_dropLevel = -12;
	else if ((prctg > 9) && (prctg <= 15))   /* reduce Pwr for 9 dB. */
		power_dropLevel = -18;
	else if ((prctg > 0) && (prctg <= 9))   /* reduce Pwr for 12 dB. */
		power_dropLevel = -24;


	wdev->mgmt_txd_txpwr_offset = (UINT8)power_dropLevel;

#ifdef CONFIG_AP_SUPPORT
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_DISABLE_TX);
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_ENABLE_TX);
#endif /* CONFIG_AP_SUPPORT */

	return;

}
static VOID mt7915_rssi_get(RTMP_ADAPTER *pAd, UINT16 Wcid, CHAR *RssiSet)
{
	RSSI_REPORT rssi_rpt;

	NdisZeroMemory(&rssi_rpt, sizeof(RSSI_REPORT));
	/* fw command to query rcpi/rssi */
	MtCmdGetRssi(pAd, Wcid, &rssi_rpt);

	RssiSet[0] = rssi_rpt.rssi[0];
	RssiSet[1] = rssi_rpt.rssi[1];
	RssiSet[2] = rssi_rpt.rssi[2];
	RssiSet[3] = rssi_rpt.rssi[3];

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s(): wcid: %d, rssi: %d, %d, %d, %d \n", __func__, Wcid,
		 RssiSet[0], RssiSet[1], RssiSet[2], RssiSet[3]));
}

static VOID mt7915_cninfo_get(RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT16 *pCnInfo)
{
	UINT16 cninfo = 0;

	MtCmdGetCnInfo(pAd, ucBandIdx, &cninfo);

	(*pCnInfo) = cninfo;
}


VOID mt7915_update_mib_bucket(RTMP_ADAPTER *pAd)
{
	UCHAR i = 0;
	UCHAR curr_idx = 0;
	P_MT_MIB_COUNTER_STAT _prPrevMibCnt;
	UCHAR concurrent_bands = HcGetAmountOfBand(pAd);
	RTMP_MIB_PAIR Reg[6];

	NdisZeroMemory(&Reg, sizeof(Reg));
	pAd->MsMibBucket.CurIdx++;
	if (pAd->MsMibBucket.CurIdx >= 2)
		pAd->MsMibBucket.CurIdx = 0;

	_prPrevMibCnt = &pAd->_rPrevMibCnt;
	curr_idx = pAd->MsMibBucket.CurIdx;

	for (i = 0; i < concurrent_bands; i++) {
		if (pAd->MsMibBucket.Enabled == TRUE) {
			Reg[0].Counter = RMAC_CNT_OBSS_AIRTIME;/* RMAC.AIRTIME14 OBSS Air time */
			Reg[1].Counter = MIB_CNT_TX_DUR_CNT;/* M0SDR36 TX Air time */
			Reg[2].Counter = MIB_CNT_RX_DUR_CNT;/* M0SDR37 RX Air time */
			Reg[3].Counter = RMAC_CNT_NONWIFI_AIRTIME;/* RMAC.AIRTIME13 Non Wifi Air time */
			Reg[4].Counter = MIB_CNT_CCA_NAV_TX_TIME;/* M0SDR9 Channel Busy Time */
			Reg[5].Counter = MIB_CNT_P_CCA_TIME;/* M0SDR16 Primary Channel Busy Time */

			MtCmdMultipleMibRegAccessRead(pAd, i, Reg, 6);

			pAd->MsMibBucket.OBSSAirtime[i][curr_idx] =
			(UINT32)(Reg[0].Value - _prPrevMibCnt->ObssAirtimeAcc[i]);
			pAd->MsMibBucket.MyTxAirtime[i][curr_idx] =
			(UINT32)(Reg[1].Value - _prPrevMibCnt->MyTxAirtimeAcc[i]);
			pAd->MsMibBucket.MyRxAirtime[i][curr_idx] =
			(UINT32)(Reg[2].Value - _prPrevMibCnt->MyRxAirtimeAcc[i]);
			pAd->MsMibBucket.EDCCAtime[i][curr_idx] =
			(UINT32)(Reg[3].Value - _prPrevMibCnt->EdccaAirtimeAcc[i]);
			pAd->MsMibBucket.ChannelBusyTimeCcaNavTx[i][curr_idx] =
			(UINT32)(Reg[4].Value - _prPrevMibCnt->CcaNavTxTimeAcc[i]);
			pAd->MsMibBucket.ChannelBusyTime[i][curr_idx] =
			(UINT32)(Reg[5].Value - _prPrevMibCnt->PCcaTimeAcc[i]);

			_prPrevMibCnt->ObssAirtimeAcc[i] = Reg[0].Value;
			_prPrevMibCnt->MyTxAirtimeAcc[i] = Reg[1].Value;
			_prPrevMibCnt->MyRxAirtimeAcc[i] = Reg[2].Value;
			_prPrevMibCnt->EdccaAirtimeAcc[i] = Reg[3].Value;
			_prPrevMibCnt->CcaNavTxTimeAcc[i] = Reg[4].Value;
			_prPrevMibCnt->PCcaTimeAcc[i] = Reg[5].Value;
		}
	}
}

VOID mt7915_ctrl_rxv_group(RTMP_ADAPTER *ad, UINT8 band_idx, UINT8 group, BOOLEAN enable)
{
	UINT32 cr_addr = BN0_WF_DMA_TOP_DCR0_RXD_GROUP_EN_ADDR+(band_idx*0x10000);
	UINT32 cr_mask = 0xffffffff, cr_value = 0;

	switch (group) {
	case FMAC_RXV_GROUP1:
		cr_mask = (0x1 << BN0_WF_DMA_TOP_DCR0_RXD_GROUP_EN_SHFT);
		break;
	case FMAC_RXV_GROUP2:
		cr_mask = (0x2 << BN0_WF_DMA_TOP_DCR0_RXD_GROUP_EN_SHFT);
		break;
	case FMAC_RXV_GROUP3:
		cr_mask = (0x4 << BN0_WF_DMA_TOP_DCR0_RXD_GROUP_EN_SHFT);
		break;
	case FMAC_RXV_GROUP5:
		cr_mask = (0x8 << BN0_WF_DMA_TOP_DCR0_RXD_GROUP_EN_SHFT);
		break;
	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s(): Unknown group(%d), ignored!\n", __func__, group));
		break;
	}

	if (~cr_mask != 0) {
		MAC_IO_READ32(ad->hdev_ctrl, cr_addr, &cr_value);
		if (enable)
			MAC_IO_WRITE32(ad->hdev_ctrl, cr_addr, (cr_value | cr_mask));
		else
			MAC_IO_WRITE32(ad->hdev_ctrl, cr_addr, (cr_value & ~cr_mask));
	}
}

extern RTMP_STRING *get_dev_eeprom_binary(VOID *pvAd);

UCHAR *mt7915_get_default_bin_image(RTMP_ADAPTER *ad)
{
#ifdef MULTI_INF_SUPPORT
	if (multi_inf_get_idx(ad) == 0) {
#if defined(CONFIG_FIRST_IF_EPAELNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 1st ePAeLNA default bin.\n"));
		return MT7915_E2PImage_ePAeLNA;
#elif defined(CONFIG_FIRST_IF_IPAELNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 1st iPAeLNA default bin.\n"));
		return MT7915_E2PImage_iPAeLNA;
#elif defined(CONFIG_FIRST_IF_EPAILNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 1st ePAiLNA default bin.\n"));
		return MT7915_E2PImage_ePAiLNA;
#else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 1st iPAiLNA default bin.\n"));
		return MT7915_E2PImage_iPAiLNA;
#endif
	}

#if defined(CONFIG_RT_SECOND_CARD)
	else if (multi_inf_get_idx(ad) == 1) {
#if defined(CONFIG_SECOND_IF_EPAELNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 2nd ePAeLNA default bin.\n"));
		return MT7915_E2PImage_ePAeLNA;
#elif defined(CONFIG_SECOND_IF_IPAELNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 2nd iPAeLNA default bin.\n"));
		return MT7915_E2PImage_iPAeLNA;
#elif defined(CONFIG_SECOND_IF_EPAILNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 2nd ePAiLNA default bin.\n"));
		return MT7915_E2PImage_ePAiLNA;
#else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 2nd iPAiLNA default bin.\n"));
		return MT7915_E2PImage_iPAiLNA;
#endif
	}

#endif /* CONFIG_RT_SECOND_CARD */
#if defined(CONFIG_RT_THIRD_CARD)
	else if (multi_inf_get_idx(ad) == 2) {
#if defined(CONFIG_THIRD_IF_EPAELNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 3rd ePAeLNA default bin.\n"));
		return MT7915_E2PImage_ePAeLNA;
#elif defined(CONFIG_THIRD_IF_IPAELNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 3rd iPAeLNA default bin.\n"));
		return MT7915_E2PImage_iPAeLNA;
#elif defined(CONFIG_THIRD_IF_EPAILNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 3rd ePAiLNA default bin.\n"));
		return MT7915_E2PImage_ePAiLNA;
#else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 3rd iPAiLNA default bin.\n"));
		return MT7915_E2PImage_iPAiLNA;
#endif
	}

#endif /* CONFIG_RT_THIRD_CARD */
	else
#endif /* MULTI_INF_SUPPORT */
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Use the default iPAiLNA bin image!\n"));
		return MT7915_E2PImage_iPAiLNA;
	}

	return NULL;
}


INT32 mt7915_get_default_bin_image_file(RTMP_ADAPTER *ad, RTMP_STRING *path)
{
	if (strlen(get_dev_eeprom_binary(ad)) > 0)
		snprintf(path, 100, "/lib/firmware/%s", get_dev_eeprom_binary(ad));
	else
		strncat(path, "/lib/firmware/e2p", strlen("/lib/firmware/e2p"));

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Use default BIN from:%s.\n", path));

	return 0;
}

INT32 mt7915_get_prek_image_file(RTMP_ADAPTER *ad, RTMP_STRING *path)
{
	if (strlen(get_dev_eeprom_binary(ad)) > 0)
		snprintf(path, 100, "/lib/firmware/%s", get_dev_eeprom_binary(ad));
	else
		strncat(path, "/lib/firmware/e2p", strlen("/lib/firmware/e2p"));

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Use PreCal BIN from:%s.\n", path));

	return 0;
}

UINT32 mt7915_get_efuse_free_blk_num(RTMP_ADAPTER *ad, UINT8 blk_section)
{
	UINT32 ret = 0;
	struct _EXT_CMD_EFUSE_FREE_BLOCK_T cmd;
	struct _EXT_EVENT_EFUSE_FREE_BLOCK_V1_T rsp;

	memset(&cmd, 0, sizeof(cmd));
	memset(&rsp, 0, sizeof(rsp));
	cmd.ucVersion = 1;
	cmd.ucDieIndex = blk_section;
	if (MtCmdEfuseFreeBlockCount(ad,
				     (PVOID)&cmd,
				     (PVOID)&rsp) == NDIS_STATUS_SUCCESS)
		ret = rsp.ucFreeBlockNum;
	else
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("(%s) cmd failed!\n", __func__));

	return ret;
}

#ifdef LED_CONTROL_SUPPORT
VOID mt7915_wps_led_init(RTMP_ADAPTER *pAd)
{
	UINT32 IdMac, macVal = 0;

	if (IS_MT7915(pAd)) {
#ifdef CONFIG_COLGIN_MT6890
		/*for colgin setting, use gpio38 as wps led*/
		/*set pinmux to GPIO*/
		IdMac = 0x70000000 + 0x5068;
		RTMP_IO_READ32(pAd->hdev_ctrl, IdMac, &macVal);
		macVal &= ~(BIT31 | BIT30 | BIT29 | BIT28);
		macVal |= 0x9 << 28;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, IdMac, macVal);

		/*set enable output*/
		IdMac = 0x70000000 + 0x40C0;
		RTMP_IO_READ32(pAd->hdev_ctrl, IdMac, &macVal);
		macVal |= BIT6;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, IdMac, macVal);

		/*Default turn off: GPIO => 0 high active*/
		IdMac = 0x70000000 + 0x40B0;
		RTMP_IO_READ32(pAd->hdev_ctrl, IdMac, &macVal);
		macVal &= ~(BIT6);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, IdMac, macVal);
#else
		/*for non-colgin setting, use gpio20 as wps led*/
		/*set pinmux to GPIO*/
		IdMac = 0x70000000 + 0x5058;
		RTMP_IO_READ32(pAd->hdev_ctrl, IdMac, &macVal);
		macVal &= ~(BIT19 | BIT18 | BIT17 | BIT16);
		macVal |= 0x9 << 16;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, IdMac, macVal);

		/*set enable output*/
		IdMac = 0x70000000 + 0x4030;
		RTMP_IO_READ32(pAd->hdev_ctrl, IdMac, &macVal);
		macVal |= BIT20;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, IdMac, macVal);

		/*Default turn off: GPIO => 1, low active*/
		IdMac = 0x70000000 + 0x4020;
		RTMP_IO_READ32(pAd->hdev_ctrl, IdMac, &macVal);
		macVal |= BIT20;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, IdMac, macVal);
#endif
		}
		/*for others init, TBD*/
}

UCHAR mt7915_wps_led_control(RTMP_ADAPTER *pAd, UCHAR flag)
{
	UINT32 macVal = 0, IdMac;
	UCHAR ret, reverse_status;

#ifdef CONFIG_COLGIN_MT6890
	IdMac = 0x70000000 + 0x40B0;
	RTMP_IO_READ32(pAd->hdev_ctrl, IdMac, &macVal); /*read current led status*/
	ret = (macVal & BIT6) ? LED_OFF : LED_ON;
	reverse_status = (ret == LED_OFF) ? LED_ON : LED_OFF;/*reverse led status*/

	if (flag == LED_STATUS)
		return ret;
	else if (flag == LED_ON || ((flag == LED_REVERSE) && (reverse_status == LED_ON))) {
		macVal &= ~(BIT6);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, IdMac, macVal);
		ret = LED_ON;
	}
	else if (flag == LED_OFF || ((flag == LED_REVERSE) && (reverse_status == LED_OFF))) {
		macVal |= BIT6;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, IdMac, macVal);
		ret = LED_OFF;
	}
#else
	IdMac = 0x70000000 + 0x4020;
	RTMP_IO_READ32(pAd->hdev_ctrl, IdMac, &macVal); /*read current led status*/
	ret = (macVal & BIT20) ? LED_OFF : LED_ON;
	reverse_status = (ret == LED_OFF) ? LED_ON : LED_OFF;/*reverse led status*/

	if (flag == LED_STATUS)
		return ret;
	else if (flag == LED_ON || ((flag == LED_REVERSE) && (reverse_status == LED_ON))) {
		macVal &= ~(BIT20);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, IdMac, macVal);
		ret = LED_ON;
	}
	else if (flag == LED_OFF || ((flag == LED_REVERSE) && (reverse_status == LED_OFF))) {
		macVal |= BIT20;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, IdMac, macVal);
		ret = LED_OFF;
	}
#endif
	return ret;
}
#endif /*LED_CONTROL_SUPPORT*/

static VOID fwdl_datapath_setup(RTMP_ADAPTER *pAd, BOOLEAN init)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (init == TRUE) {
		/* swap ctrl_ring to WM */
		if (IS_PCI_INF(pAd)) {
			struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
			UINT32 i;

			for (i = 0; i < hif->tx_res_num; i++) {
				struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, i);
				if (tx_ring->ring_attr == HIF_TX_CMD_WM) {
					hif->ctrl_ring = tx_ring;
					break;
				}
			}
		}
		ops->kick_out_cmd_msg = mt7915_kick_out_fwdl_msg;
	} else {
		/* swap ctrl_ring to WA */
		if (IS_PCI_INF(pAd)) {
			struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
			UINT32 i;

			for (i = 0; i < hif->tx_res_num; i++) {
				struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, i);
				if (tx_ring->ring_attr == HIF_TX_CMD) {
					hif->ctrl_ring = tx_ring;
					break;
				}
			}
		}
		ops->kick_out_cmd_msg = hif_kick_out_cmd_msg;
	}
}

#ifdef CONFIG_STA_SUPPORT
static VOID init_dev_nick_name(RTMP_ADAPTER *ad)
{
	snprintf((RTMP_STRING *) ad->nickname, sizeof(ad->nickname), "mt7915_sta");
}
#endif /* CONFIG_STA_SUPPORT */


#ifdef TXBF_SUPPORT
static VOID chip_dump_pfmu_tag1(union txbf_pfmu_tag1 *pfmu_tag1)
{
	MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
		"============================= TxBf profile Tage1 Info ========================================\n"
		"Row data0 = 0x%08x, Row data1 = 0x%08x, Row data2 = 0x%08x, Row data3 = 0x%08x\n"
		"Row data4 = 0x%08x, Row data5 = 0x%08x, Row data6 = 0x%08x\n"
		"\n"
		"PFMU ID = %d		Invalid status = %d\n"
		"iBf/eBf = %d\n"
		"\n"
		"DBW   = %d\n"
		"SU/MU = %d\n"
		"RMSD  = %d\n"
		"nrow=%d, ncol=%d, ng=%d, LM=%d, CodeBook=%d MobCalEn=%d\n"
		"RU start = %d, RU end =%d\n"
		"\n"
		"Mem Col1 = %d, Mem Row1 = %d, Mem Col2 = %d, Mem Row2 = %d\n"
		"Mem Col3 = %d, Mem Row3 = %d, Mem Col4 = %d, Mem Row4 = %d\n"
		"\n"
		"STS0_SNR =0x%02x, STS1_SNR=0x%02x, STS2_SNR=0x%02x, STS3_SNR=0x%02x\n"
		"STS4_SNR =0x%02x, STS5_SNR=0x%02x, STS6_SNR=0x%02x, STS7_SNR=0x%02x\n"
		"==============================================================================================\n",
		pfmu_tag1->raw_data[0], pfmu_tag1->raw_data[1], pfmu_tag1->raw_data[2], pfmu_tag1->raw_data[3],
		pfmu_tag1->raw_data[4], pfmu_tag1->raw_data[5], pfmu_tag1->raw_data[6],
		pfmu_tag1->field.profile_id, pfmu_tag1->field.invalid_prof,
		pfmu_tag1->field.txbf,
		pfmu_tag1->field.dbw,
		pfmu_tag1->field.su_mu,
		pfmu_tag1->field.rmsd,
		pfmu_tag1->field.nrow, pfmu_tag1->field.ncol, pfmu_tag1->field.ngroup, pfmu_tag1->field.lm,
		pfmu_tag1->field.codebook, pfmu_tag1->field.mob_cal_en,
		pfmu_tag1->field.ru_start_id, pfmu_tag1->field.ru_end_id,
		pfmu_tag1->field.mem_addr1_col_id, pfmu_tag1->field.mem_addr1_row_id,
		pfmu_tag1->field.mem_addr2_col_id, pfmu_tag1->field.mem_addr2_row_id,
		pfmu_tag1->field.mem_addr3_col_id, pfmu_tag1->field.mem_addr3_row_id,
		pfmu_tag1->field.mem_addr4_col_id, pfmu_tag1->field.mem_addr4_row_id,
		pfmu_tag1->field.snr_sts0, pfmu_tag1->field.snr_sts1,
		pfmu_tag1->field.snr_sts2, pfmu_tag1->field.snr_sts3,
		pfmu_tag1->field.snr_sts4, pfmu_tag1->field.snr_sts5,
		pfmu_tag1->field.snr_sts6, pfmu_tag1->field.snr_sts7));
}

static VOID chip_dump_pfmu_tag2(union txbf_pfmu_tag2 *pfmu_tag2)
{
	MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
		"============================= TxBf profile Tage2 Info ========================================\n"
		"Row data0 = 0x%08x, Row data1 = 0x%08x, Row data2 = 0x%08x, Row data3 = 0x%08x\n"
		"Row data4 = 0x%08x, Row data5 = 0x%08x, Row data6 = 0x%08x\n"
		"\n"
		"Smart antenna ID = 0x%x,  SE index = %d\n"
		"RMSD threshold = %d\n"
		"Timeout = 0x%x\n"
		"Desired BW = %d, Desired Ncol = %d, Desired Nrow = %d\n"
		"Desired RU Allocation = %d\n"
		"Mobility DeltaT = %d, Mobility LQ = %d\n"
		"==============================================================================================\n",
		pfmu_tag2->raw_data[0], pfmu_tag2->raw_data[1], pfmu_tag2->raw_data[2], pfmu_tag2->raw_data[3],
		pfmu_tag2->raw_data[4], pfmu_tag2->raw_data[5], pfmu_tag2->raw_data[6],
		pfmu_tag2->field.smart_ant, pfmu_tag2->field.se_idx,
		pfmu_tag2->field.rmsd_thd,
		pfmu_tag2->field.ibf_timeout, pfmu_tag2->field.ibf_dbw,
		pfmu_tag2->field.ibf_ncol, pfmu_tag2->field.ibf_nrow,
		pfmu_tag2->field.ibf_ru,
		pfmu_tag2->field.mob_delta_t, pfmu_tag2->field.mob_lq_result));
}

static VOID txbf_dump_tag(struct _RTMP_ADAPTER *pAd, BOOLEAN fgBFer, PUCHAR pBuf)
{
	struct txbf_pfmu_tags_info *tags_info = &pAd->pfmu_tags_info;
	union txbf_pfmu_tag1 *pfmu_tag1;
	union txbf_pfmu_tag2 *pfmu_tag2;

	pfmu_tag1 = (union txbf_pfmu_tag1 *) pBuf;
	pfmu_tag2 = (union txbf_pfmu_tag2 *) (pBuf + sizeof(*pfmu_tag1));
#ifdef RT_BIG_ENDIAN
	RTMPEndianChange((char *)pfmu_tag1, sizeof(*pfmu_tag1));
	RTMPEndianChange((char *)pfmu_tag2, sizeof(*pfmu_tag2));
#endif

	/* cache tag1 */
	tags_info->pfmu_idx = pfmu_tag1->field.profile_id;
	tags_info->ebf = pfmu_tag1->field.txbf;
	tags_info->dbw = pfmu_tag1->field.dbw;
	tags_info->lm = pfmu_tag1->field.lm;
	tags_info->su_mu = pfmu_tag1->field.su_mu;
	tags_info->nr = pfmu_tag1->field.nrow;
	tags_info->nc = pfmu_tag1->field.ncol;
	tags_info->codebook = pfmu_tag1->field.codebook;
	tags_info->ng = pfmu_tag1->field.ngroup;
	tags_info->invalid_prof = pfmu_tag1->field.invalid_prof;
	tags_info->rmsd = pfmu_tag1->field.rmsd;

	tags_info->mem_col0 = pfmu_tag1->field.mem_addr1_col_id;
	tags_info->mem_row0 = pfmu_tag1->field.mem_addr1_row_id;
	tags_info->mem_col1 = pfmu_tag1->field.mem_addr2_col_id;
	tags_info->mem_row1 = pfmu_tag1->field.mem_addr2_row_id;
	tags_info->mem_col2 = pfmu_tag1->field.mem_addr3_col_id;
	tags_info->mem_row2 = pfmu_tag1->field.mem_addr3_row_id;
	tags_info->mem_col3 = pfmu_tag1->field.mem_addr4_col_id;
	tags_info->mem_row3 = pfmu_tag1->field.mem_addr4_row_id;

	tags_info->ru_start = pfmu_tag1->field.ru_start_id;
	tags_info->ru_end = pfmu_tag1->field.ru_end_id;
	tags_info->mobility_cal_en = pfmu_tag1->field.mob_cal_en;
	tags_info->snr_sts0 = pfmu_tag1->field.snr_sts0;
	tags_info->snr_sts1 = pfmu_tag1->field.snr_sts1;
	tags_info->snr_sts2 = pfmu_tag1->field.snr_sts2;
	tags_info->snr_sts3 = pfmu_tag1->field.snr_sts3;
	tags_info->snr_sts4 = pfmu_tag1->field.snr_sts4;
	tags_info->snr_sts5 = pfmu_tag1->field.snr_sts5;
	tags_info->snr_sts6 = pfmu_tag1->field.snr_sts6;
	tags_info->snr_sts7 = pfmu_tag1->field.snr_sts7;

	/* cache tag2 */
	tags_info->smart_ant = pfmu_tag2->field.smart_ant;
	tags_info->se_idx = pfmu_tag2->field.se_idx;
	tags_info->rmsd_threshold = pfmu_tag2->field.rmsd_thd;
	tags_info->ibf_timeout = pfmu_tag2->field.ibf_timeout;
	tags_info->ibf_desired_dbw = pfmu_tag2->field.ibf_dbw;
	tags_info->ibf_desired_ncol = pfmu_tag2->field.ibf_ncol;
	tags_info->ibf_desired_nrow = pfmu_tag2->field.ibf_nrow;
	tags_info->ibf_desired_ru_alloc = pfmu_tag2->field.ibf_ru;

	chip_dump_pfmu_tag1(pfmu_tag1);

	if (fgBFer == TRUE)
		chip_dump_pfmu_tag2(pfmu_tag2);
}

static void setETxBFCap(
	IN  RTMP_ADAPTER	  *pAd,
	IN  TXBF_STATUS_INFO * pTxBfInfo)
{
	HT_BF_CAP *pTxBFCap = pTxBfInfo->pHtTxBFCap;

	if (pTxBfInfo->cmmCfgETxBfEnCond > 0) {
		switch (pTxBfInfo->cmmCfgETxBfEnCond) {
		case SUBF_ALL:
		default:
			pTxBFCap->RxNDPCapable		 = TRUE;
			pTxBFCap->TxNDPCapable		 = (pTxBfInfo->ucRxPathNum > 1) ? TRUE : FALSE;
			pTxBFCap->ExpNoComSteerCapable = FALSE;
			pTxBFCap->ExpComSteerCapable   = TRUE;/* !pTxBfInfo->cmmCfgETxBfNoncompress; */
			pTxBFCap->ExpNoComBF		   = 0; /* HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->ExpComBF			 =
				HT_ExBF_FB_CAP_IMMEDIATE;/* pTxBfInfo->cmmCfgETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->MinGrouping		  = 3;
			pTxBFCap->NoComSteerBFAntSup   = 0;
			pTxBFCap->ComSteerBFAntSup	 = 3;
			pTxBFCap->TxSoundCapable	   = FALSE;  /* Support staggered sounding frames */
			pTxBFCap->ChanEstimation	   = pTxBfInfo->ucRxPathNum - 1;
			break;

		case SUBF_BFER:
			pTxBFCap->RxNDPCapable		 = FALSE;
			pTxBFCap->TxNDPCapable		 = (pTxBfInfo->ucRxPathNum > 1) ? TRUE : FALSE;
			pTxBFCap->ExpNoComSteerCapable = FALSE;
			pTxBFCap->ExpComSteerCapable   = TRUE;/* !pTxBfInfo->cmmCfgETxBfNoncompress; */
			pTxBFCap->ExpNoComBF		   = 0; /* HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->ExpComBF			 =
				HT_ExBF_FB_CAP_IMMEDIATE;/* pTxBfInfo->cmmCfgETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->MinGrouping		  = 3;
			pTxBFCap->NoComSteerBFAntSup   = 0;
			pTxBFCap->ComSteerBFAntSup	 = 3;
			pTxBFCap->TxSoundCapable	   = FALSE;  /* Support staggered sounding frames */
			pTxBFCap->ChanEstimation	   = pTxBfInfo->ucRxPathNum - 1;
			break;

		case SUBF_BFEE:
			pTxBFCap->RxNDPCapable		 = TRUE;
			pTxBFCap->TxNDPCapable		 = FALSE;
			pTxBFCap->ExpNoComSteerCapable = FALSE;
			pTxBFCap->ExpComSteerCapable   = TRUE;/* !pTxBfInfo->cmmCfgETxBfNoncompress; */
			pTxBFCap->ExpNoComBF		   = 0; /* HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->ExpComBF			 =
				HT_ExBF_FB_CAP_IMMEDIATE;/* pTxBfInfo->cmmCfgETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE; */
			pTxBFCap->MinGrouping		  = 3;
			pTxBFCap->NoComSteerBFAntSup   = 0;
			pTxBFCap->ComSteerBFAntSup	 = 3;
			pTxBFCap->TxSoundCapable	   = FALSE;  /* Support staggered sounding frames */
			pTxBFCap->ChanEstimation	   = pTxBfInfo->ucRxPathNum - 1;
			break;
		}
	} else
		memset(pTxBFCap, 0, sizeof(*pTxBFCap));
}

static INT chip_set_txbf_pfmu_tag(struct hdev_ctrl *ctrl, enum txbf_pfmu_tag idx, UINT32 val)
{
	struct _RTMP_ADAPTER *pAd = hc_get_hdev_privdata(ctrl);
	struct txbf_pfmu_tags_info *tags_info = &pAd->pfmu_tags_info;

	switch (idx) {
	case TAG1_PFMU_ID:
		tags_info->pfmu_idx = val;
		break;
	case TAG1_IEBF:
		tags_info->ebf = val;
		break;
	case TAG1_DBW:
		tags_info->dbw = val;
		break;
	case TAG1_SU_MU:
		tags_info->su_mu = val;
		break;
	case TAG1_INVALID:
		tags_info->invalid_prof = val;
		break;
	case TAG1_MEM_ROW0:
		tags_info->mem_row0 = val;
		break;
	case TAG1_MEM_ROW1:
		tags_info->mem_row1 = val;
		break;
	case TAG1_MEM_ROW2:
		tags_info->mem_row2 = val;
		break;
	case TAG1_MEM_ROW3:
		tags_info->mem_row3 = val;
		break;
	case TAG1_MEM_COL0:
		tags_info->mem_col0 = val;
		break;
	case TAG1_MEM_COL1:
		tags_info->mem_col1 = val;
		break;
	case TAG1_MEM_COL2:
		tags_info->mem_col2 = val;
		break;
	case TAG1_MEM_COL3:
		tags_info->mem_col3 = val;
		break;
	case TAG1_RMSD:
		tags_info->rmsd = val;
		break;
	case TAG1_NR:
		tags_info->nr = val;
		break;
	case TAG1_NC:
		tags_info->nc = val;
		break;
	case TAG1_NG:
		tags_info->ng = val;
		break;
	case TAG1_LM:
		tags_info->lm = val;
		break;
	case TAG1_CODEBOOK:
		tags_info->codebook = val;
		break;
	case TAG1_HTC:
		tags_info->htc = val;
		break;
	case TAG1_RU_START:
		tags_info->ru_start = val;
		break;
	case TAG1_RU_END:
		tags_info->ru_end = val;
		break;
	case TAG1_MOB_CAL_EN:
		tags_info->mobility_cal_en = val;
		break;
	case TAG1_SNR_STS0:
		tags_info->snr_sts0 = val;
		break;
	case TAG1_SNR_STS1:
		tags_info->snr_sts1 = val;
		break;
	case TAG1_SNR_STS2:
		tags_info->snr_sts2 = val;
		break;
	case TAG1_SNR_STS3:
		tags_info->snr_sts3 = val;
		break;
	case TAG1_SNR_STS4:
		tags_info->snr_sts4 = val;
		break;
	case TAG1_SNR_STS5:
		tags_info->snr_sts5 = val;
		break;
	case TAG1_SNR_STS6:
		tags_info->snr_sts6 = val;
		break;
	case TAG1_SNR_STS7:
		tags_info->snr_sts7 =  val;
		break;
	case TAG2_SE_ID:
		tags_info->se_idx = val;
		break;
	case TAG2_SMART_ANT:
		tags_info->smart_ant = val;
		break;
	case TAG2_RMSD_THRESHOLD:
		tags_info->rmsd_threshold = val;
		break;
	case TAG2_IBF_TIMEOUT:
		tags_info->ibf_timeout = val;
		break;
	case TAG2_IBF_DBW:
		tags_info->ibf_desired_dbw = val;
		break;
	case TAG2_IBF_NROW:
		tags_info->ibf_desired_nrow = val;
		break;
	case TAG2_IBF_NCOL:
		tags_info->ibf_desired_ncol = val;
		break;
	case TAG2_IBF_RU_ALLOC:
		tags_info->ibf_desired_ru_alloc = val;
		break;
	default:
		break;
	}

	return TRUE;
}

static INT chip_write_txbf_pfmu_tag(struct hdev_ctrl *ctrl, UINT8 pf_idx)
{
	struct _RTMP_ADAPTER *pAd = hc_get_hdev_privdata(ctrl);
	struct txbf_pfmu_tags_info *tags_info = &pAd->pfmu_tags_info;
	union txbf_pfmu_tag1 pfmu_tag1;
	union txbf_pfmu_tag2 pfmu_tag2;

	os_zero_mem(&pfmu_tag1, sizeof(pfmu_tag1));
	os_zero_mem(&pfmu_tag2, sizeof(pfmu_tag2));

	/* prepare tag1 */
	pfmu_tag1.field.profile_id = tags_info->pfmu_idx;
	pfmu_tag1.field.txbf = tags_info->ebf;
	pfmu_tag1.field.dbw = tags_info->dbw;
	pfmu_tag1.field.lm = tags_info->lm;
	pfmu_tag1.field.su_mu = tags_info->su_mu;
	pfmu_tag1.field.nrow = tags_info->nr;
	pfmu_tag1.field.ncol = tags_info->nc;
	pfmu_tag1.field.codebook = tags_info->codebook;
	pfmu_tag1.field.ngroup = tags_info->ng;
	pfmu_tag1.field.invalid_prof = tags_info->invalid_prof;
	pfmu_tag1.field.rmsd = tags_info->rmsd;

	pfmu_tag1.field.mem_addr1_col_id = tags_info->mem_col0;
	pfmu_tag1.field.mem_addr1_row_id = tags_info->mem_row0;
	pfmu_tag1.field.mem_addr2_col_id = tags_info->mem_col1;
	pfmu_tag1.field.mem_addr2_row_id = tags_info->mem_row1;
	pfmu_tag1.field.mem_addr3_col_id = tags_info->mem_col2;
	pfmu_tag1.field.mem_addr3_row_id = tags_info->mem_row2;
	pfmu_tag1.field.mem_addr4_col_id = tags_info->mem_col3;
	pfmu_tag1.field.mem_addr4_row_id = tags_info->mem_row3;

	pfmu_tag1.field.ru_start_id = tags_info->ru_start;
	pfmu_tag1.field.ru_end_id = tags_info->ru_end;
	pfmu_tag1.field.mob_cal_en = tags_info->mobility_cal_en;
	pfmu_tag1.field.snr_sts0 = tags_info->snr_sts0;
	pfmu_tag1.field.snr_sts1 = tags_info->snr_sts1;
	pfmu_tag1.field.snr_sts2 = tags_info->snr_sts2;
	pfmu_tag1.field.snr_sts3 = tags_info->snr_sts3;
	pfmu_tag1.field.snr_sts4 = tags_info->snr_sts4;
	pfmu_tag1.field.snr_sts5 = tags_info->snr_sts5;
	pfmu_tag1.field.snr_sts6 = tags_info->snr_sts6;
	pfmu_tag1.field.snr_sts7 = tags_info->snr_sts7;

	/* prepare tag2 */
	pfmu_tag2.field.smart_ant = tags_info->smart_ant;
	pfmu_tag2.field.se_idx = tags_info->se_idx;
	pfmu_tag2.field.rmsd_thd = tags_info->rmsd_threshold;
	pfmu_tag2.field.ibf_timeout = tags_info->ibf_timeout;
	pfmu_tag2.field.ibf_dbw = tags_info->ibf_desired_dbw;
	pfmu_tag2.field.ibf_ncol = tags_info->ibf_desired_ncol;
	pfmu_tag2.field.ibf_nrow = tags_info->ibf_desired_nrow;
	pfmu_tag2.field.ibf_ru = tags_info->ibf_desired_ru_alloc;

	if (CmdETxBfPfmuProfileTagWrite(pAd,
		(PUCHAR)(&pfmu_tag1), (PUCHAR)(&pfmu_tag2),
		sizeof(pfmu_tag1), sizeof(pfmu_tag2), pf_idx) == STATUS_TRUE) {
		/* dump tag1 */
		chip_dump_pfmu_tag1(&pfmu_tag1);
		/* dump tag2 */
		chip_dump_pfmu_tag2(&pfmu_tag2);
		return 1;
	}

	return 0;
}

VOID txbf_show_pfmu_data(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		subCarrIdx,
	IN PUCHAR		pBuf)
{
	union txbf_bfer_pfmu_data pfmu_data, *pfmu_data_start;
	POS_COOKIE pObj = NULL;
	struct wifi_dev *wdev = NULL;
	UINT8 he_bw, tx_nss;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	he_bw = wlan_config_get_he_bw(wdev);
	tx_nss = wlan_config_get_tx_stream(wdev);

	pfmu_data_start = (union txbf_bfer_pfmu_data *) pBuf;
#ifdef RT_BIG_ENDIAN
	RTMPEndianChange((UCHAR *)pfmu_data_start, sizeof(pfmu_data));
#endif
	NdisCopyMemory(&pfmu_data, pfmu_data_start, sizeof(pfmu_data));

	if (tx_nss > 4) {
		MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
			"================ TxBf profile Data - Subcarrier Idx = %d(0x%03x) =================\n"
			"=============================== Low Seg Angles ==================================\n"
			"Psi31 = 0x%02x, Phi21 = 0x%03x, Psi21 = 0x%02x, Phi11 = 0x%03x\n"
			"Psi51 = 0x%02x, Phi41 = 0x%03x, Psi41 = 0x%02x, Phi31 = 0x%03x\n"
			"Psi71 = 0x%02x, Phi61 = 0x%03x, Psi61 = 0x%02x, Phi51 = 0x%03x\n"
			"Psi32 = 0x%02x, Phi22 = 0x%03x, Psi81 = 0x%02x, Phi71 = 0x%03x\n"
			"Psi52 = 0x%02x, Phi42 = 0x%03x, Psi42 = 0x%02x, Phi32 = 0x%03x\n"
			"Psi72 = 0x%02x, Phi62 = 0x%03x, Psi62 = 0x%02x, Phi52 = 0x%03x\n"
			"Psi43 = 0x%02x, Phi33 = 0x%03x, Psi82 = 0x%02x, Phi72 = 0x%03x\n"
			"Psi63 = 0x%02x, Phi53 = 0x%03x, Psi53 = 0x%02x, Phi43 = 0x%03x\n"
			"Psi83 = 0x%02x, Phi73 = 0x%03x, Psi73 = 0x%02x, Phi63 = 0x%03x\n"
			"Psi64 = 0x%02x, Phi54 = 0x%03x, Psi54 = 0x%02x, Phi44 = 0x%03x\n"
			"Psi84 = 0x%02x, Phi74 = 0x%03x, Psi74 = 0x%02x, Phi64 = 0x%03x\n"
			"Psi75 = 0x%02x, Phi65 = 0x%03x, Psi65 = 0x%02x, Phi55 = 0x%03x\n"
			"Psi76 = 0x%02x, Phi66 = 0x%03x, Psi85 = 0x%02x, Phi75 = 0x%03x\n"
			"Psi87 = 0x%02x, Phi77 = 0x%03x, Psi86 = 0x%02x, Phi76 = 0x%03x\n",
			subCarrIdx, subCarrIdx,
			pfmu_data.field.rLowSegAng.field.psi31, pfmu_data.field.rLowSegAng.field.phi21,
			pfmu_data.field.rLowSegAng.field.psi21, pfmu_data.field.rLowSegAng.field.phi11,
			pfmu_data.field.rLowSegAng.field.psi51, pfmu_data.field.rLowSegAng.field.phi41,
			pfmu_data.field.rLowSegAng.field.psi41, pfmu_data.field.rLowSegAng.field.phi31,
			pfmu_data.field.rLowSegAng.field.psi71, pfmu_data.field.rLowSegAng.field.phi61,
			pfmu_data.field.rLowSegAng.field.psi61, pfmu_data.field.rLowSegAng.field.phi51,
			pfmu_data.field.rLowSegAng.field.psi32, pfmu_data.field.rLowSegAng.field.phi22,
			pfmu_data.field.rLowSegAng.field.psi81, pfmu_data.field.rLowSegAng.field.phi71,
			pfmu_data.field.rLowSegAng.field.psi52, pfmu_data.field.rLowSegAng.field.phi42,
			pfmu_data.field.rLowSegAng.field.psi42, pfmu_data.field.rLowSegAng.field.phi32,
			pfmu_data.field.rLowSegAng.field.psi72, pfmu_data.field.rLowSegAng.field.phi62,
			pfmu_data.field.rLowSegAng.field.psi62, pfmu_data.field.rLowSegAng.field.phi52,
			pfmu_data.field.rLowSegAng.field.psi43, pfmu_data.field.rLowSegAng.field.phi33,
			pfmu_data.field.rLowSegAng.field.psi82, pfmu_data.field.rLowSegAng.field.phi72,
			pfmu_data.field.rLowSegAng.field.psi63, pfmu_data.field.rLowSegAng.field.phi53,
			pfmu_data.field.rLowSegAng.field.psi53, pfmu_data.field.rLowSegAng.field.phi43,
			pfmu_data.field.rLowSegAng.field.psi83, pfmu_data.field.rLowSegAng.field.phi73,
			pfmu_data.field.rLowSegAng.field.psi73, pfmu_data.field.rLowSegAng.field.phi63,
			pfmu_data.field.rLowSegAng.field.psi64, pfmu_data.field.rLowSegAng.field.phi54,
			pfmu_data.field.rLowSegAng.field.psi54, pfmu_data.field.rLowSegAng.field.phi44,
			pfmu_data.field.rLowSegAng.field.psi84, pfmu_data.field.rLowSegAng.field.phi74,
			pfmu_data.field.rLowSegAng.field.psi74, pfmu_data.field.rLowSegAng.field.phi64,
			pfmu_data.field.rLowSegAng.field.psi75, pfmu_data.field.rLowSegAng.field.phi65,
			pfmu_data.field.rLowSegAng.field.psi65, pfmu_data.field.rLowSegAng.field.phi55,
			pfmu_data.field.rLowSegAng.field.psi76, pfmu_data.field.rLowSegAng.field.phi66,
			pfmu_data.field.rLowSegAng.field.psi85, pfmu_data.field.rLowSegAng.field.phi75,
			pfmu_data.field.rLowSegAng.field.psi87, pfmu_data.field.rLowSegAng.field.phi77,
			pfmu_data.field.rLowSegAng.field.psi86, pfmu_data.field.rLowSegAng.field.phi76));

		if (he_bw == HE_BW_160 || he_bw == HE_BW_8080) {
			MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
				"=============================== High Seg Angles =================================\n"
				"Psi31 = 0x%02x, Phi21 = 0x%03x, Psi21 = 0x%02x, Phi11 = 0x%03x\n"
				"Psi51 = 0x%02x, Phi41 = 0x%03x, Psi41 = 0x%02x, Phi31 = 0x%03x\n"
				"Psi71 = 0x%02x, Phi61 = 0x%03x, Psi61 = 0x%02x, Phi51 = 0x%03x\n"
				"Psi32 = 0x%02x, Phi22 = 0x%03x, Psi81 = 0x%02x, Phi71 = 0x%03x\n"
				"Psi52 = 0x%02x, Phi42 = 0x%03x, Psi42 = 0x%02x, Phi32 = 0x%03x\n"
				"Psi72 = 0x%02x, Phi62 = 0x%03x, Psi62 = 0x%02x, Phi52 = 0x%03x\n"
				"Psi43 = 0x%02x, Phi33 = 0x%03x, Psi82 = 0x%02x, Phi72 = 0x%03x\n"
				"Psi63 = 0x%02x, Phi53 = 0x%03x, Psi53 = 0x%02x, Phi43 = 0x%03x\n"
				"Psi83 = 0x%02x, Phi73 = 0x%03x, Psi73 = 0x%02x, Phi63 = 0x%03x\n"
				"Psi64 = 0x%02x, Phi54 = 0x%03x, Psi54 = 0x%02x, Phi44 = 0x%03x\n"
				"Psi84 = 0x%02x, Phi74 = 0x%03x, Psi74 = 0x%02x, Phi64 = 0x%03x\n"
				"Psi75 = 0x%02x, Phi65 = 0x%03x, Psi65 = 0x%02x, Phi55 = 0x%03x\n"
				"Psi76 = 0x%02x, Phi66 = 0x%03x, Psi85 = 0x%02x, Phi75 = 0x%03x\n"
				"Psi87 = 0x%02x, Phi77 = 0x%03x, Psi86 = 0x%02x, Phi76 = 0x%03x\n",
				pfmu_data.field.rHighSegAng.field.psi31, pfmu_data.field.rHighSegAng.field.phi21,
				pfmu_data.field.rHighSegAng.field.psi21, pfmu_data.field.rHighSegAng.field.phi11,
				pfmu_data.field.rHighSegAng.field.psi51, pfmu_data.field.rHighSegAng.field.phi41,
				pfmu_data.field.rHighSegAng.field.psi41, pfmu_data.field.rHighSegAng.field.phi31,
				pfmu_data.field.rHighSegAng.field.psi71, pfmu_data.field.rHighSegAng.field.phi61,
				pfmu_data.field.rHighSegAng.field.psi61, pfmu_data.field.rHighSegAng.field.phi51,
				pfmu_data.field.rHighSegAng.field.psi32, pfmu_data.field.rHighSegAng.field.phi22,
				pfmu_data.field.rHighSegAng.field.psi81, pfmu_data.field.rHighSegAng.field.phi71,
				pfmu_data.field.rHighSegAng.field.psi52, pfmu_data.field.rHighSegAng.field.phi42,
				pfmu_data.field.rHighSegAng.field.psi42, pfmu_data.field.rHighSegAng.field.phi32,
				pfmu_data.field.rHighSegAng.field.psi72, pfmu_data.field.rHighSegAng.field.phi62,
				pfmu_data.field.rHighSegAng.field.psi62, pfmu_data.field.rHighSegAng.field.phi52,
				pfmu_data.field.rHighSegAng.field.psi43, pfmu_data.field.rHighSegAng.field.phi33,
				pfmu_data.field.rHighSegAng.field.psi82, pfmu_data.field.rHighSegAng.field.phi72,
				pfmu_data.field.rHighSegAng.field.psi63, pfmu_data.field.rHighSegAng.field.phi53,
				pfmu_data.field.rHighSegAng.field.psi53, pfmu_data.field.rHighSegAng.field.phi43,
				pfmu_data.field.rHighSegAng.field.psi83, pfmu_data.field.rHighSegAng.field.phi73,
				pfmu_data.field.rHighSegAng.field.psi73, pfmu_data.field.rHighSegAng.field.phi63,
				pfmu_data.field.rHighSegAng.field.psi64, pfmu_data.field.rHighSegAng.field.phi54,
				pfmu_data.field.rHighSegAng.field.psi54, pfmu_data.field.rHighSegAng.field.phi44,
				pfmu_data.field.rHighSegAng.field.psi84, pfmu_data.field.rHighSegAng.field.phi74,
				pfmu_data.field.rHighSegAng.field.psi74, pfmu_data.field.rHighSegAng.field.phi64,
				pfmu_data.field.rHighSegAng.field.psi75, pfmu_data.field.rHighSegAng.field.phi65,
				pfmu_data.field.rHighSegAng.field.psi65, pfmu_data.field.rHighSegAng.field.phi55,
				pfmu_data.field.rHighSegAng.field.psi76, pfmu_data.field.rHighSegAng.field.phi66,
				pfmu_data.field.rHighSegAng.field.psi85, pfmu_data.field.rHighSegAng.field.phi75,
				pfmu_data.field.rHighSegAng.field.psi87, pfmu_data.field.rHighSegAng.field.phi77,
				pfmu_data.field.rHighSegAng.field.psi86, pfmu_data.field.rHighSegAng.field.phi76));
		}

		MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
			"================================= Low Seg SNRs ===================================\n"
			"SNR00 = 0x%01x, SNR01 = 0x%01x, SNR02 = 0x%01x, SNR03 = 0x%01x\n"
			"SNR04 = 0x%01x, SNR05 = 0x%01x, SNR06 = 0x%01x, SNR07 = 0x%01x\n",
			pfmu_data.field.rLowSegSnr.field.dsnr00, pfmu_data.field.rLowSegSnr.field.dsnr01,
			pfmu_data.field.rLowSegSnr.field.dsnr02, pfmu_data.field.rLowSegSnr.field.dsnr03,
			pfmu_data.field.rLowSegSnr.field.dsnr04, pfmu_data.field.rLowSegSnr.field.dsnr05,
			pfmu_data.field.rLowSegSnr.field.dsnr06, pfmu_data.field.rLowSegSnr.field.dsnr07));

		if (he_bw == HE_BW_160 || he_bw == HE_BW_8080) {
			MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
				"================================ High Seg SNRs ===================================\n"
				"SNR00 = 0x%01x, SNR01 = 0x%01x, SNR02 = 0x%01x, SNR03 = 0x%01x\n"
				"SNR04 = 0x%01x, SNR05 = 0x%01x, SNR06 = 0x%01x, SNR07 = 0x%01x\n",
				pfmu_data.field.rHighSegSnr.field.dsnr00, pfmu_data.field.rHighSegSnr.field.dsnr01,
				pfmu_data.field.rHighSegSnr.field.dsnr02, pfmu_data.field.rHighSegSnr.field.dsnr03,
				pfmu_data.field.rHighSegSnr.field.dsnr04, pfmu_data.field.rHighSegSnr.field.dsnr05,
				pfmu_data.field.rHighSegSnr.field.dsnr06, pfmu_data.field.rHighSegSnr.field.dsnr07));
		}

		MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
			"==================================================================================\n"));
	} else {
		MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
			"================= TxBf profile Data - Subcarrier Idx = %d(0x%03x) ===================\n"
			"================================= Low Seg Angles ====================================\n"
			"Psi41 = 0x%02x, Phi31 = 0x%03x, Psi31 = 0x%02x, Phi21 = 0x%03x Psi21 = 0x%02x, Phi11 = 0x%03x,\n"
			"Psi43 = 0x%02x, Phi33 = 0x%03x, Psi42 = 0x%02x, Phi32 = 0x%03x Psi32 = 0x%02x, Phi22 = 0x%03x,\n",
			subCarrIdx, subCarrIdx,
			pfmu_data.field.rLowSegAng.field.psi41, pfmu_data.field.rLowSegAng.field.phi31,
			pfmu_data.field.rLowSegAng.field.psi31, pfmu_data.field.rLowSegAng.field.phi21,
			pfmu_data.field.rLowSegAng.field.psi21, pfmu_data.field.rLowSegAng.field.phi11,
			pfmu_data.field.rLowSegAng.field.psi43, pfmu_data.field.rLowSegAng.field.phi33,
			pfmu_data.field.rLowSegAng.field.psi42, pfmu_data.field.rLowSegAng.field.phi32,
			pfmu_data.field.rLowSegAng.field.psi32, pfmu_data.field.rLowSegAng.field.phi22));

		if (he_bw == HE_BW_160 || he_bw == HE_BW_8080) {
			MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
				"================================== High Seg Angles ====================================\n"
				"Psi41 = 0x%02x, Phi31 = 0x%03x, Psi31 = 0x%02x, Phi21 = 0x%03x Psi21 = 0x%02x, Phi11 = 0x%03x,\n"
				"Psi43 = 0x%02x, Phi33 = 0x%03x, Psi42 = 0x%02x, Phi32 = 0x%03x Psi32 = 0x%02x, Phi22 = 0x%03x,\n",
				pfmu_data.field.rHighSegAng.field.psi41, pfmu_data.field.rHighSegAng.field.phi31,
				pfmu_data.field.rHighSegAng.field.psi31, pfmu_data.field.rHighSegAng.field.phi21,
				pfmu_data.field.rHighSegAng.field.psi21, pfmu_data.field.rHighSegAng.field.phi11,
				pfmu_data.field.rHighSegAng.field.psi43, pfmu_data.field.rHighSegAng.field.phi33,
				pfmu_data.field.rHighSegAng.field.psi42, pfmu_data.field.rHighSegAng.field.phi32,
				pfmu_data.field.rHighSegAng.field.psi32, pfmu_data.field.rHighSegAng.field.phi22));
		}

		MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
			"================================= Low Seg SNRs ===================================\n"
			"SNR00 = 0x%01x, SNR01 = 0x%01x, SNR02 = 0x%01x, SNR03 = 0x%01x\n",
			pfmu_data.field.rLowSegSnr.field.dsnr00, pfmu_data.field.rLowSegSnr.field.dsnr01,
			pfmu_data.field.rLowSegSnr.field.dsnr02, pfmu_data.field.rLowSegSnr.field.dsnr03));

		if (he_bw == HE_BW_160 || he_bw == HE_BW_8080) {
			MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
				"================================ High Seg SNRs ===================================\n"
				"SNR00 = 0x%01x, SNR01 = 0x%01x, SNR02 = 0x%01x, SNR03 = 0x%01x\n",
				pfmu_data.field.rHighSegSnr.field.dsnr00, pfmu_data.field.rHighSegSnr.field.dsnr01,
				pfmu_data.field.rHighSegSnr.field.dsnr02, pfmu_data.field.rHighSegSnr.field.dsnr03));
		}

		MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
			"==================================================================================\n"));
	}
}

static INT chip_set_txbf_angle(struct hdev_ctrl *ctrl, UINT32 bfer, UINT32 nc, UINT32 *angle)
{
	struct _RTMP_ADAPTER *pAd = hc_get_hdev_privdata(ctrl);
	union txbf_bfer_pfmu_data *pfmu_data = (union txbf_bfer_pfmu_data *)pAd->pfmu_data_raw;
	union txbf_low_seg_angle *LowSegAng = &pfmu_data->field.rLowSegAng;

	switch (nc) {
	case 0: /* n x 1 */
		LowSegAng->field.phi11 = angle[0];
		LowSegAng->field.psi21 = angle[1];
		LowSegAng->field.phi21 = angle[2];
		LowSegAng->field.psi31 = angle[3];
		LowSegAng->field.phi31 = angle[4];
		LowSegAng->field.psi41 = angle[5];
		LowSegAng->field.phi41 = angle[6];
		LowSegAng->field.psi51 = angle[7];
		LowSegAng->field.phi51 = angle[8];
		LowSegAng->field.psi61 = angle[9];
		LowSegAng->field.phi61 = angle[10];
		LowSegAng->field.psi71 = angle[11];
		LowSegAng->field.phi71 = angle[12];
		LowSegAng->field.psi81 = angle[13];
		break;
	case 1: /* n x 2 */
		LowSegAng->field.phi22 = angle[0];
		LowSegAng->field.psi32 = angle[1];
		LowSegAng->field.phi32 = angle[2];
		LowSegAng->field.psi42 = angle[3];
		LowSegAng->field.phi42 = angle[4];
		LowSegAng->field.psi52 = angle[5];
		LowSegAng->field.phi52 = angle[6];
		LowSegAng->field.psi62 = angle[7];
		LowSegAng->field.phi62 = angle[8];
		LowSegAng->field.psi72 = angle[9];
		LowSegAng->field.phi72 = angle[10];
		LowSegAng->field.psi82 = angle[11];
		break;
	case 2: /* n x 3 */
		LowSegAng->field.phi33 = angle[0];
		LowSegAng->field.psi43 = angle[1];
		LowSegAng->field.phi43 = angle[2];
		LowSegAng->field.psi53 = angle[3];
		LowSegAng->field.phi53 = angle[4];
		LowSegAng->field.psi63 = angle[5];
		LowSegAng->field.phi63 = angle[6];
		LowSegAng->field.psi73 = angle[7];
		LowSegAng->field.phi73 = angle[8];
		LowSegAng->field.psi83 = angle[9];
		break;
	case 3: /* n x 4 */
		LowSegAng->field.phi44 = angle[0];
		LowSegAng->field.psi54 = angle[1];
		LowSegAng->field.phi54 = angle[2];
		LowSegAng->field.psi64 = angle[3];
		LowSegAng->field.phi64 = angle[4];
		LowSegAng->field.psi74 = angle[5];
		LowSegAng->field.phi74 = angle[6];
		LowSegAng->field.psi84 = angle[7];
		break;
	case 4: /* n x 5 */
		LowSegAng->field.phi55 = angle[0];
		LowSegAng->field.psi65 = angle[1];
		LowSegAng->field.phi65 = angle[2];
		LowSegAng->field.psi75 = angle[3];
		LowSegAng->field.phi75 = angle[4];
		LowSegAng->field.psi85 = angle[5];
		break;
	case 5: /* n x 6 */
		LowSegAng->field.phi66 = angle[0];
		LowSegAng->field.psi76 = angle[1];
		LowSegAng->field.phi76 = angle[2];
		LowSegAng->field.psi86 = angle[3];
		break;
	case 6: /* n x 7 */
	case 7: /* n x 8 */
		LowSegAng->field.phi77 = angle[0];
		LowSegAng->field.psi87 = angle[1];
		break;
	default:
		break;
	}

	return TRUE;
}

static INT chip_set_txbf_dsnr(struct hdev_ctrl *ctrl, UINT32 bfer, UINT32 *dsnr)
{
	struct _RTMP_ADAPTER *pAd = hc_get_hdev_privdata(ctrl);
	union txbf_bfer_pfmu_data *pfmu_data = (union txbf_bfer_pfmu_data *)pAd->pfmu_data_raw;
	union txbf_bfer_low_seg_dsnr *LowSegSnr = &pfmu_data->field.rLowSegSnr;

	LowSegSnr->field.dsnr00 = dsnr[0];
	LowSegSnr->field.dsnr01 = dsnr[1];
	LowSegSnr->field.dsnr02 = dsnr[2];
	LowSegSnr->field.dsnr03 = dsnr[3];
	LowSegSnr->field.dsnr04 = dsnr[4];
	LowSegSnr->field.dsnr05 = dsnr[5];
	LowSegSnr->field.dsnr06 = dsnr[6];
	LowSegSnr->field.dsnr07 = dsnr[7];

	return TRUE;
}

static INT chip_write_txbf_pfmu_data(struct hdev_ctrl *ctrl, UINT8 pf_idx, UINT16 subc_idx, BOOLEAN bfer)
{
	struct _RTMP_ADAPTER *pAd = hc_get_hdev_privdata(ctrl);
	union txbf_bfer_pfmu_data *pfmu_data = (union txbf_bfer_pfmu_data *)pAd->pfmu_data_raw;
	BOOLEAN fgStatus = FALSE;

	if (CmdETxBfPfmuFullDimDataWrite(pAd, pf_idx, subc_idx, bfer,
		(PUCHAR)pfmu_data, sizeof(*pfmu_data)) == STATUS_TRUE)
		fgStatus = TRUE;

	os_zero_mem((pAd->pfmu_data_raw), sizeof(pAd->pfmu_data_raw));

	return fgStatus;
}

static INT chip_write_txbf_profile_data(struct _RTMP_ADAPTER *pAd, PUSHORT Input)
{
	BOOLEAN fgStatus = FALSE;
	UCHAR profileIdx;
	USHORT subcarrierIdx;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT32 angle[14];
	UINT32 bfer = 1;
	UINT32 dsnr[8];
	UINT8 nc = 0;

	profileIdx = Input[0];
	subcarrierIdx = Input[1];

	os_zero_mem((pAd->pfmu_data_raw), sizeof(pAd->pfmu_data_raw));
	os_zero_mem(angle, sizeof(angle));
	os_zero_mem(dsnr, sizeof(dsnr));

	angle[0] = Input[2];
	angle[1] = Input[3];
	angle[2] = Input[4];
	angle[3] = Input[5];
	angle[4] = Input[6];
	angle[5] = Input[7];

	if (ops->set_txbf_angle)
		fgStatus = ops->set_txbf_angle(pAd->hdev_ctrl, bfer, nc, angle);

	if (fgStatus == FALSE) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("set_txbf_angle failed for nc:%d\n", nc));
		return fgStatus;
	}

	nc = 1;
	os_zero_mem(angle, sizeof(angle));
	angle[0] = Input[8];
	angle[1] = Input[9];
	angle[2] = Input[10];
	angle[3] = Input[11];

	if (ops->set_txbf_angle)
		fgStatus = ops->set_txbf_angle(pAd->hdev_ctrl, bfer, nc, angle);

	if (fgStatus == FALSE) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("set_txbf_angle failed for nc:%d\n", nc));
		return fgStatus;
	}

	nc = 2;
	os_zero_mem(angle, sizeof(angle));
	angle[0] = Input[12];
	angle[1] = Input[13];

	if (ops->set_txbf_angle)
		fgStatus = ops->set_txbf_angle(pAd->hdev_ctrl, bfer, nc, angle);

	if (fgStatus == FALSE) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("set_txbf_angle failed for nc:%d\n", nc));
		return fgStatus;
	}

	dsnr[0] = Input[14];
	dsnr[1] = Input[15];
	dsnr[2] = Input[16];
	dsnr[3] = Input[17];

	if (ops->set_txbf_dsnr)
		fgStatus = ops->set_txbf_dsnr(pAd->hdev_ctrl, bfer, dsnr);

	if (fgStatus == FALSE) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("set_txbf_dsnr failed\n"));
		return fgStatus;
	}

	if (ops->write_txbf_pfmu_data)
		fgStatus = ops->write_txbf_pfmu_data(pAd->hdev_ctrl, profileIdx, subcarrierIdx, bfer);

	if (fgStatus == FALSE) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("write_txbf_pfmu_data failed for pfmu_id:%u and subcarrier:%u\n",
			profileIdx, subcarrierIdx));
		return fgStatus;
	}

	return fgStatus;
}

#ifdef VHT_TXBF_SUPPORT
static void setVHTETxBFCap(
	IN  RTMP_ADAPTER *pAd,
	IN  TXBF_STATUS_INFO * pTxBfInfo)
{
	VHT_CAP_INFO *pTxBFCap = pTxBfInfo->pVhtTxBFCap;

	/*
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: cmmCfgETxBfEnCond = %d\n", __FUNCTION__, (UCHAR)pTxBfInfo->cmmCfgETxBfEnCond));
	*/

	if (pTxBfInfo->cmmCfgETxBfEnCond > 0) {
		switch (pTxBfInfo->cmmCfgETxBfEnCond) {
		case SUBF_ALL:
		default:
			pTxBFCap->bfee_cap_su	   = 1;
			pTxBFCap->bfer_cap_su	   = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
#ifdef CFG_SUPPORT_MU_MIMO

			switch (pAd->CommonCfg.MUTxRxEnable) {
			case MUBF_OFF:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_BFER:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
				break;

			case MUBF_BFEE:
				pTxBFCap->bfee_cap_mu = 1;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_ALL:
				pTxBFCap->bfee_cap_mu = 1;
				pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
				break;

			default:
				MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: set wrong parameters\n", __func__));
				break;
			}

#else
			pTxBFCap->bfee_cap_mu = 0;
			pTxBFCap->bfer_cap_mu = 0;
#endif /* CFG_SUPPORT_MU_MIMO */
			if (pTxBFCap->bfee_sts_cap)
				pTxBFCap->bfee_sts_cap	= (pTxBFCap->bfee_sts_cap < 3) ? pTxBFCap->bfee_sts_cap : 3;
			else
				pTxBFCap->bfee_sts_cap	  = 3;
			pTxBFCap->num_snd_dimension = pTxBfInfo->ucTxPathNum - 1;
			break;

		case SUBF_BFER:
			pTxBFCap->bfee_cap_su	   = 0;
			pTxBFCap->bfer_cap_su	   = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
#ifdef CFG_SUPPORT_MU_MIMO

			switch (pAd->CommonCfg.MUTxRxEnable) {
			case MUBF_OFF:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_BFER:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
				break;

			case MUBF_BFEE:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_ALL:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
				break;

			default:
				MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: set wrong parameters\n", __func__));
				break;
			}

#else
			pTxBFCap->bfee_cap_mu = 0;
			pTxBFCap->bfer_cap_mu = 0;
#endif /* CFG_SUPPORT_MU_MIMO */
			pTxBFCap->bfee_sts_cap	  = 0;
			pTxBFCap->num_snd_dimension = pTxBfInfo->ucTxPathNum - 1;
			break;

		case SUBF_BFEE:
			pTxBFCap->bfee_cap_su	   = 1;
			pTxBFCap->bfer_cap_su	   = 0;
#ifdef CFG_SUPPORT_MU_MIMO

			switch (pAd->CommonCfg.MUTxRxEnable) {
			case MUBF_OFF:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_BFER:
				pTxBFCap->bfee_cap_mu = 0;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_BFEE:
				pTxBFCap->bfee_cap_mu = 1;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			case MUBF_ALL:
				pTxBFCap->bfee_cap_mu = 1;
				pTxBFCap->bfer_cap_mu = 0;
				break;

			default:
				MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: set wrong parameters\n", __func__));
				break;
			}

#else
			pTxBFCap->bfee_cap_mu = 0;
			pTxBFCap->bfer_cap_mu = 0;
#endif /* CFG_SUPPORT_MU_MIMO */
			if (pTxBFCap->bfee_sts_cap)
				pTxBFCap->bfee_sts_cap	= (pTxBFCap->bfee_sts_cap < 3) ? pTxBFCap->bfee_sts_cap : 3;
			else
				pTxBFCap->bfee_sts_cap	  = 3;
			pTxBFCap->num_snd_dimension = pTxBfInfo->ucTxPathNum - 1;
			break;
		}
	} else {
		pTxBFCap->num_snd_dimension = 0;
		pTxBFCap->bfee_cap_mu	   = 0;
		pTxBFCap->bfee_cap_su	   = 0;
		pTxBFCap->bfer_cap_mu	   = 0;
		pTxBFCap->bfer_cap_su	   = 0;
		pTxBFCap->bfee_sts_cap	  = 0;
	}
}

#endif /* VHT_TXBF_SUPPORT */

#ifdef HE_TXBF_SUPPORT
void get_he_etxbf_cap(
	struct wifi_dev *wdev,
	TXBF_STATUS_INFO * txbf_status)
{
	struct he_bf_info *he_bf_struct = txbf_status->he_bf_info;
	struct mcs_nss_caps *mcs_nss = wlan_config_get_mcs_nss_caps(wdev);

	if (txbf_status->cmmCfgETxBfEnCond > 0) {
		switch (txbf_status->cmmCfgETxBfEnCond) {
		case SUBF_ALL:

			if (wdev->wdev_type == WDEV_TYPE_AP) {
				he_bf_struct->bf_cap |= (txbf_status->ucTxPathNum > 1) ? HE_SU_BFER : 0;
				he_bf_struct->bf_cap |= (txbf_status->ucTxPathNum > 1) ? HE_MU_BFER : 0;
			}

			he_bf_struct->bf_cap |= HE_SU_BFEE;

			if (he_bf_struct->bf_cap & HE_SU_BFEE) {
				/* Harrier E1/E2 don't support Ng16 due to Spec D4.0 change subcarrier index. */
				/* he_bf_struct->bf_cap |= HE_BFEE_NG_16_SU_FEEDBACK; */
				/* he_bf_struct->bf_cap |= HE_BFEE_NG_16_MU_FEEDBACK; */
				he_bf_struct->bf_cap |= HE_BFEE_CODEBOOK_SU_FEEDBACK;
				he_bf_struct->bf_cap |= HE_BFEE_CODEBOOK_MU_FEEDBACK;
				he_bf_struct->bfee_sts_gt_bw80 = 3;
				he_bf_struct->bfee_sts_le_eq_bw80 = 3;

				/*
				* Our max cap is 4x3, (Nr, Nc) = (3, 2).
				* We should boundle the value to our antenna quantity.
				* Antenna 4	3   2   1
				* Nc	  3	3   2   1
				*/
				he_bf_struct->bfee_max_nc = min(txbf_status->ucTxPathNum - 1, 2);
			}

			if (he_bf_struct->bf_cap & HE_SU_BFER) {
				/*
				* HE dintinguished Nr to '<=BW80' and '>BW80' in its protocol.
				* This fit our HW architecture.
				* Our current HW's Nsts of '<=BW80' is 4 and Nsts of '>BW80' is 2.
				*/
				he_bf_struct->snd_dim_gt_bw80 = max((UINT8)(mcs_nss->bw160_max_nss - 1), (UINT8)1);
				he_bf_struct->snd_dim_le_eq_bw80 = wlan_config_get_tx_stream(wdev) - 1;

				he_bf_struct->bf_cap |= HE_TRIG_SU_BFEE_FEEDBACK;
				he_bf_struct->bf_cap |= HE_TRIG_MU_BFEE_FEEDBACK;
			}

			break;

		case SUBF_BFER:

			if (wdev->wdev_type == WDEV_TYPE_AP) {
				he_bf_struct->bf_cap |= (txbf_status->ucTxPathNum > 1) ? HE_SU_BFER : 0;
				he_bf_struct->bf_cap |= (txbf_status->ucTxPathNum > 1) ? HE_MU_BFER : 0;
			}

			if (he_bf_struct->bf_cap & HE_SU_BFER) {
				/*
				* HE dintinguished Nr to '<=BW80' and '>BW80' in its protocol.
				* This fit our HW architecture.
				* Our current HW's Nsts of '<=BW80' is 4 and Nsts of '>BW80' is 2.
				*/
				he_bf_struct->snd_dim_gt_bw80 = max((UINT8)(mcs_nss->bw160_max_nss - 1), (UINT8)1);
				he_bf_struct->snd_dim_le_eq_bw80 = wlan_config_get_tx_stream(wdev) - 1;

				he_bf_struct->bf_cap |= HE_TRIG_SU_BFEE_FEEDBACK;
				he_bf_struct->bf_cap |= HE_TRIG_MU_BFEE_FEEDBACK;
			}

			break;

		case SUBF_BFEE:
			he_bf_struct->bf_cap |= HE_SU_BFEE;

			if (he_bf_struct->bf_cap & HE_SU_BFEE) {
				/* Harrier E1/E2 don't support Ng16 due to Spec D4.0 change subcarrier index. */
				/* he_bf_struct->bf_cap |= HE_BFEE_NG_16_SU_FEEDBACK; */
				/* he_bf_struct->bf_cap |= HE_BFEE_NG_16_MU_FEEDBACK; */
				he_bf_struct->bf_cap |= HE_BFEE_CODEBOOK_SU_FEEDBACK;
				he_bf_struct->bf_cap |= HE_BFEE_CODEBOOK_MU_FEEDBACK;
				he_bf_struct->bfee_sts_gt_bw80 = 3;
				he_bf_struct->bfee_sts_le_eq_bw80 = 3;
				he_bf_struct->bfee_max_nc = min(txbf_status->ucTxPathNum - 1, 2);
				he_bf_struct->bf_cap |= HE_TRIG_SU_BFEE_FEEDBACK;
				he_bf_struct->bf_cap |= HE_TRIG_MU_BFEE_FEEDBACK;
			}
			break;

		default:
			MTWF_LOG(DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: set wrong parameters\n", __func__));
			break;
		}
	}
}
#endif /* HE_TXBF_SUPPORT */

#endif /* TXBF_SUPPORT */

/*
*
*/
#define DEFAULT_RID 0
inline UINT32 get_rid_value(struct pci_hif_chip *pci_hif)
{
	UINT32 rid;

	rid = readl((void *)(pci_hif->CSRBaseAddress +  WF_WFDMA_EXT_WRAP_CSR_PCI_BASE(WF_WFDMA_EXT_WRAP_CSR_PCIE_RECOGNITION_ID_PCIE_RECOG_ID_ADDR)));

	return ((rid & WF_WFDMA_EXT_WRAP_CSR_PCIE_RECOGNITION_ID_PCIE_RECOG_ID_MASK) >>
		WF_WFDMA_EXT_WRAP_CSR_PCIE_RECOGNITION_ID_PCIE_RECOG_ID_SHFT);
}

inline VOID set_rid_value(struct pci_hif_chip *pci_hif, UINT32 rid)
{
	UINT32 value = WF_WFDMA_EXT_WRAP_CSR_PCIE_RECOGNITION_ID_PCIE_RECOG_SEM_MASK | rid;

	writel(value, (void *)(pci_hif->CSRBaseAddress + WF_WFDMA_EXT_WRAP_CSR_PCI_BASE(WF_WFDMA_EXT_WRAP_CSR_PCIE_RECOGNITION_ID_ADDR)));
}

static INT hif_init_WPDMA(RTMP_ADAPTER *pAd)
{
	UINT32 val = 0;
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct pci_hif_chip *master_hif_chip = pci_hif->main_hif_chip;
#ifdef WHNAT_SUPPORT
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif

	/* configure global setting */
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_ADDR, &val);
	/* host DMA1(to WMCPU/WACPU) need to omit tx/rx info */
	val |= WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_OMIT_TX_INFO_MASK;
	val |= WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_OMIT_RX_INFO_MASK;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_ADDR, val);

	/* configure prefetch setting */
#ifdef WFDMA_PREFETCH_MANUAL_MODE
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, &val);
	/* disable prefetch offset calculation auto-mode */
	val &= ~WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_CSR_DISP_BASE_PTR_CHAIN_EN_MASK;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, val);

	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_ADDR, &val);
	/* disable prefetch offset calculation auto-mode */
	val &= ~WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_CSR_DISP_BASE_PTR_CHAIN_EN_MASK;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_ADDR, val);
#endif

#ifdef WHNAT_SUPPORT
	if (pAd->CommonCfg.whnat_en && (cap->tkn_info.feature & TOKEN_RX)) {
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT0_ADDR, &val);
		val |= (1 << WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT0_CSR_RX_WB_KEEP_RSVD_SHFT);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT0_ADDR, val);
	}
#endif

	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_EXT_CTRL_ADDR, 0x00000004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING1_EXT_CTRL_ADDR, 0x00400004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_EXT_CTRL_ADDR, 0x00800000);

	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_TX_RING0_EXT_CTRL_ADDR, 0x00800004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_TX_RING1_EXT_CTRL_ADDR, 0x00c00004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_TX_RING2_EXT_CTRL_ADDR, 0x01000004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_TX_RING3_EXT_CTRL_ADDR, 0x01400004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_TX_RING4_EXT_CTRL_ADDR, 0x01800004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_TX_RING5_EXT_CTRL_ADDR, 0x01c00004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_TX_RING6_EXT_CTRL_ADDR, 0x02000004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_TX_RING7_EXT_CTRL_ADDR, 0x02400004);

	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_TX_RING16_EXT_CTRL_ADDR, 0x02800004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_TX_RING17_EXT_CTRL_ADDR, 0x02c00004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_TX_RING18_EXT_CTRL_ADDR, 0x03000004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_TX_RING19_EXT_CTRL_ADDR, 0x03400004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_TX_RING20_EXT_CTRL_ADDR, 0x03800004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_TX_RING21_EXT_CTRL_ADDR, 0x03c00000);

	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_RX_RING0_EXT_CTRL_ADDR, 0x03c00004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_RX_RING1_EXT_CTRL_ADDR, 0x04000004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_RX_RING2_EXT_CTRL_ADDR, 0x04400004);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_RX_RING3_EXT_CTRL_ADDR, 0x04800000);

	/* reset dma idx */
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_RST_DTX_PTR_ADDR, 0xFFFFFFFF);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_RST_DTX_PTR_ADDR, 0xFFFFFFFF);

	/* configure delay interrupt */
	if (IS_ASIC_CAP(pAd, fASIC_CAP_DLY_INT_PER_RING)) {
		/* enable hif_dma0 pri 1(rx data) */
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_ADDR,
			(1	<< WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_PRI1_DLY_INT_EN_SHFT) ||
			(1	<< WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PINT_SHFT) ||
			(0x1c << WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PTIME_SHFT));
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG0_ADDR, 0);
	}

	/* configure PCIe 1 host dma if it's in use, check if recognition ID is written */
	if (get_rid_value(master_hif_chip) != DEFAULT_RID) {
		/* configure global setting */
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_ADDR, &val);
		/* host DMA1(to WMCPU/WACPU) need to omit tx/rx info */
		val |= WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_OMIT_TX_INFO_MASK;
		val |= WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_OMIT_RX_INFO_MASK;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_ADDR, val);

		/* configure prefetch setting */
#ifdef WFDMA_PREFETCH_MANUAL_MODE
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_ADDR, &val);
		/* disable prefetch offset calculation auto-mode */
		val &= ~WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_CSR_DISP_BASE_PTR_CHAIN_EN_MASK;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_ADDR, val);

		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_ADDR, &val);
		/* disable prefetch offset calculation auto-mode */
		val &= ~WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_CSR_DISP_BASE_PTR_CHAIN_EN_MASK;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_ADDR, val);
#endif

		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING1_EXT_CTRL_ADDR, 0x04800004);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RX_RING2_EXT_CTRL_ADDR, 0x04c00000);

		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_TX_RING19_EXT_CTRL_ADDR, 0x04c00004);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, (WF_WFDMA_HOST_DMA1_PCIE1_BASE + 0x610), 0x05000000);

		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_RX_RING2_EXT_CTRL_ADDR, 0x05000004);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_RX_RING3_EXT_CTRL_ADDR, 0x05400000);

		/* reset dma idx */
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_RST_DTX_PTR_ADDR, 0xFFFFFFFF);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_RST_DTX_PTR_ADDR, 0xFFFFFFFF);

		/* enable PCIe 1 TXP cut through */
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pdma_per_band_ADDR, &val);
		val |= WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pdma_per_band_MASK;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pdma_per_band_ADDR, val);
	}

#ifdef WFDMA_WED_COMPATIBLE
		/* configure WED ring alias */
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_ADDR, &val);
		val |= WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_host_wed_en_MASK;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_ADDR, val);

		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_EXT_WRAP_CSR_WED_RING_CONTROL_IDX_ADDR, &val);
		val &= ~WF_WFDMA_EXT_WRAP_CSR_WED_RING_CONTROL_IDX_WED_TX_RING0_CONTROL_IDX_MASK;
		val |= 18 << WF_WFDMA_EXT_WRAP_CSR_WED_RING_CONTROL_IDX_WED_TX_RING0_CONTROL_IDX_SHFT;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_EXT_WRAP_CSR_WED_RING_CONTROL_IDX_ADDR, val);

		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_EXT_WRAP_CSR_WED_RING_CONTROL_IDX_ADDR, &val);
		val &= ~WF_WFDMA_EXT_WRAP_CSR_WED_RING_CONTROL_IDX_WED_TX_RING1_CONTROL_IDX_MASK;
		val |= 19 << WF_WFDMA_EXT_WRAP_CSR_WED_RING_CONTROL_IDX_WED_TX_RING1_CONTROL_IDX_SHFT;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_EXT_WRAP_CSR_WED_RING_CONTROL_IDX_ADDR, val);

		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_EXT_WRAP_CSR_WED_RING_CONTROL_IDX_ADDR, &val);
		val &= ~WF_WFDMA_EXT_WRAP_CSR_WED_RING_CONTROL_IDX_WED_RX_RING1_CONTROL_IDX_MASK;
		val |= 0x1 << WF_WFDMA_EXT_WRAP_CSR_WED_RING_CONTROL_IDX_WED_RX_RING1_CONTROL_IDX_SHFT;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_EXT_WRAP_CSR_WED_RING_CONTROL_IDX_ADDR, val);
#endif
#ifdef CTXD_SCATTER_AND_GATHER
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_EXT1_ADDR, &val);
	val |= 0x10000000;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_EXT1_ADDR, val);
#endif
#ifdef CONFIG_COLGIN_MT6890
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH10_ADDR, &val);
	val &= ~(WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH10_RX_DMAD_TH0_MASK |
		WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH10_RX_DMAD_TH1_MASK);
	val |= 0x4;
	val |= 0x4 << WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH10_RX_DMAD_TH1_SHFT;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_PAUSE_RX_Q_TH10_ADDR, val);
#endif
	return TRUE;
}

static INT hif_set_WPDMA(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable)
{
	UINT32 val0 = 0, val1 = 0;
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct pci_hif_chip *master_hif_chip = pci_hif->main_hif_chip;
#ifdef WHNAT_SUPPORT
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif

	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, &val0);
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_ADDR, &val1);

	switch (TxRx) {
	case DMA_TX:
		if (enable == TRUE) {
			val0 |= WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_MASK;
			val1 |= WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_TX_DMA_EN_MASK;
		} else {
			val0 &= ~(WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_MASK);
			val1 &= ~(WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_TX_DMA_EN_MASK);
		}

		break;

	case DMA_RX:
		if (enable == TRUE) {

#ifdef WHNAT_SUPPORT
			if (pAd->CommonCfg.whnat_en && (cap->tkn_info.feature & TOKEN_RX))
				val0 &= ~WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK;
			else
				val0 |= WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK;
#else
			val0 |= WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK;
#endif

			val1 |= WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_RX_DMA_EN_MASK;
		} else {
			val0 &= ~(WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK);
			val1 &= ~(WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_RX_DMA_EN_MASK);
		}

		break;

	case DMA_TX_RX:
		if (enable == TRUE) {
			val0 |= WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_MASK;
			val1 |= WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_TX_DMA_EN_MASK;

#ifdef WHNAT_SUPPORT
			if (pAd->CommonCfg.whnat_en && (cap->tkn_info.feature & TOKEN_RX))
				val0 &= ~(WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK);
			else
				val0 |= WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK;
#else
			val0 |= WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK;
#endif
			val1 |= WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_RX_DMA_EN_MASK;
		} else {
			val0 &= ~(WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_EN_MASK);
			val1 &= ~(WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_TX_DMA_EN_MASK);
			val0 &= ~(WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_EN_MASK);
			val1 &= ~(WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_RX_DMA_EN_MASK);
		}

		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown path (%d\n", __func__, TxRx));
		break;
	}

	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, val0);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_ADDR, val1);

	/* configure PCIe 1 host dma if it's in use, check if recognition ID is written */
	if (get_rid_value(master_hif_chip) != DEFAULT_RID) {
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_ADDR, &val0);
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_ADDR, &val1);

		switch (TxRx) {
		case DMA_TX:
			if (enable == TRUE) {
				val0 |= WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_MASK;
				val1 |= WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_MASK;
			} else {
				val0 &= ~(WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_MASK);
				val1 &= ~(WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_MASK);
			}

			break;

		case DMA_RX:
			if (enable == TRUE) {
				val0 |= WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_MASK;
				val1 |= WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_MASK;
			} else {
				val0 &= ~(WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_MASK);
				val1 &= ~(WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_MASK);
			}

			break;

		case DMA_TX_RX:
			if (enable == TRUE) {
				val0 |= WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_MASK;
				val1 |= WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_MASK;
				val0 |= WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_MASK;
				val1 |= WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_MASK;
			} else {
				val0 &= ~(WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_MASK);
				val1 &= ~(WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_TX_DMA_EN_MASK);
				val0 &= ~(WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_MASK);
				val1 &= ~(WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_RX_DMA_EN_MASK);
			}

			break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown path (%d\n", __func__, TxRx));
			break;
		}

		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_GLO_CFG_ADDR, val0);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_GLO_CFG_ADDR, val1);
	}

#define WPDMA_DISABLE -1

	if (!enable)
		TxRx = WPDMA_DISABLE;

	WLAN_HOOK_CALL(WLAN_HOOK_DMA_SET, pAd, &TxRx);
	return TRUE;
}

static BOOLEAN hif_wait_WPDMA_idle(struct _RTMP_ADAPTER *pAd, UINT8 pcie_port_or_all, INT round, INT wait_us)
{
	INT i = 0;
	UINT32 val = 0;
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct pci_hif_chip *master_hif_chip = pci_hif->main_hif_chip;
	BOOLEAN idle_0 = TRUE;
	BOOLEAN idle_1 = TRUE;
	UINT32 band_idx_map = 0;
	UINT8 bandidx = 0;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
		return FALSE;

	if (pcie_port_or_all == ALL_DMA) {
		for (bandidx = 0; bandidx < HcGetAmountOfBand(pAd) ; bandidx++)
			band_idx_map |= (1 << bandidx);
	} else {
		band_idx_map |= (1 << pcie_port_or_all);
	}

	if (band_idx_map & (1 << 0)) {
		MTWF_LOG(DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): check band(0)\n", __func__));

		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_ADDR, &val);
		val |= (WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_conn_hif_rxfifo_busy_enable_MASK |
			WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_conn_hif_txfifo0_busy_enable_MASK |
			WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_conn_hif_txfifo1_busy_enable_MASK);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_ADDR, val);

		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_CONN_HIF_BUSY_ENA_ADDR, &val);
		val |= (WF_WFDMA_HOST_DMA1_CONN_HIF_BUSY_ENA_conn_hif_rxfifo_busy_enable_MASK |
			WF_WFDMA_HOST_DMA1_CONN_HIF_BUSY_ENA_conn_hif_txfifo0_busy_enable_MASK |
			WF_WFDMA_HOST_DMA1_CONN_HIF_BUSY_ENA_conn_hif_txfifo1_busy_enable_MASK);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_CONN_HIF_BUSY_ENA_ADDR, val);

		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_ADDR, &val);
		val &= ~(WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_rxfifo_busy_enable_MASK |
			WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_txfifo0_busy_enable_MASK |
			WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_txfifo1_busy_enable_MASK);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_ADDR, val);

		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_BUSY_ENA_ADDR, &val);
		val &= ~(WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_rxfifo_busy_enable_MASK |
			WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_txfifo0_busy_enable_MASK |
			WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_txfifo1_busy_enable_MASK);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_BUSY_ENA_ADDR, val);

		i = 0;
		idle_0 = FALSE;
		do {
			RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HIF_MISC_ADDR, &val);

			if ((val & WF_WFDMA_EXT_WRAP_CSR_WFDMA_HIF_MISC_HIF_BUSY_MASK) == 0) {
				idle_0 = TRUE;
				break;
			}

			RtmpusecDelay(wait_us);
		} while ((i++) < round);
	}

	/* configure PCIe 1 host dma if it's in use, check if recognition ID is written */
	if ((get_rid_value(master_hif_chip) != DEFAULT_RID) && (band_idx_map & (1 << 1))) {
		MTWF_LOG(DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): check band(1)\n", __func__));

		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_ADDR, &val);
		val |= (WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_conn_hif_rxfifo_busy_enable_MASK |
			WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_conn_hif_txfifo0_busy_enable_MASK |
			WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_conn_hif_txfifo1_busy_enable_MASK);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_CONN_HIF_BUSY_ENA_ADDR, val);

		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_CONN_HIF_BUSY_ENA_ADDR, &val);
		val |= (WF_WFDMA_HOST_DMA1_CONN_HIF_BUSY_ENA_conn_hif_rxfifo_busy_enable_MASK |
			WF_WFDMA_HOST_DMA1_CONN_HIF_BUSY_ENA_conn_hif_txfifo0_busy_enable_MASK |
			WF_WFDMA_HOST_DMA1_CONN_HIF_BUSY_ENA_conn_hif_txfifo1_busy_enable_MASK);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_CONN_HIF_BUSY_ENA_ADDR, val);

		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_ADDR, &val);
		val |= WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_rxfifo_busy_enable_MASK;
		val &= ~(WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_txfifo0_busy_enable_MASK |
			WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_txfifo1_busy_enable_MASK);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_BUSY_ENA_ADDR, val);

		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_BUSY_ENA_ADDR, &val);
		val |= (WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_rxfifo_busy_enable_MASK |
			WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_txfifo0_busy_enable_MASK |
			WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_BUSY_ENA_conn_hif_txfifo1_busy_enable_MASK);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_BUSY_ENA_ADDR, val);

		i = 0;
		idle_1 = FALSE;
		do {
			RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_EXT_WRAP_CSR_WFDMA_HIF_MISC_ADDR, &val);

			if ((val & WF_WFDMA_EXT_WRAP_CSR_WFDMA_HIF_MISC_HIF_BUSY_MASK) == 0) {
				idle_1 = TRUE;
				break;
			}

			RtmpusecDelay(wait_us);
		} while ((i++) < round);
	}

	return (idle_0 & idle_1);
}

static BOOLEAN hif_reset_WPDMA(RTMP_ADAPTER *pAd)
{
	UINT32 value = 0;
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct pci_hif_chip *master_hif_chip = pci_hif->main_hif_chip;

	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_CONN_HIF_RST_ADDR, &value);
	value &= ~(WF_WFDMA_HOST_DMA1_CONN_HIF_RST_conn_hif_logic_rst_n_MASK | WF_WFDMA_HOST_DMA1_CONN_HIF_RST_dmashdl_all_rst_n_MASK);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_CONN_HIF_RST_ADDR, value);
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_CONN_HIF_RST_ADDR, &value);
	value |= (WF_WFDMA_HOST_DMA1_CONN_HIF_RST_conn_hif_logic_rst_n_MASK | WF_WFDMA_HOST_DMA1_CONN_HIF_RST_dmashdl_all_rst_n_MASK);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_CONN_HIF_RST_ADDR, value);

	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_CONN_HIF_RST_ADDR, &value);
	value &= ~(WF_WFDMA_HOST_DMA0_CONN_HIF_RST_conn_hif_logic_rst_n_MASK | WF_WFDMA_HOST_DMA0_CONN_HIF_RST_dmashdl_all_rst_n_MASK);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_CONN_HIF_RST_ADDR, value);
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_CONN_HIF_RST_ADDR, &value);
	value |= (WF_WFDMA_HOST_DMA0_CONN_HIF_RST_conn_hif_logic_rst_n_MASK | WF_WFDMA_HOST_DMA0_CONN_HIF_RST_dmashdl_all_rst_n_MASK);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_CONN_HIF_RST_ADDR, value);

	/* configure PCIe 1 host dma if it's in use, check if recognition ID is written */
	if (get_rid_value(master_hif_chip) != DEFAULT_RID) {
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_RST_ADDR, &value);
		value &= ~(WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_RST_conn_hif_logic_rst_n_MASK | WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_RST_dmashdl_all_rst_n_MASK);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_RST_ADDR, value);
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_RST_ADDR, &value);
		value |= (WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_RST_conn_hif_logic_rst_n_MASK | WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_RST_dmashdl_all_rst_n_MASK);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_CONN_HIF_RST_ADDR, value);

		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_RST_ADDR, &value);
		value &= ~(WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_RST_conn_hif_logic_rst_n_MASK | WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_RST_dmashdl_all_rst_n_MASK);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_RST_ADDR, value);
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_RST_ADDR, &value);
		value |= (WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_RST_conn_hif_logic_rst_n_MASK | WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_RST_dmashdl_all_rst_n_MASK);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_CONN_HIF_RST_ADDR, value);
	}

	return TRUE;
}
static INT32 get_fw_sync_value(RTMP_ADAPTER *pAd)
{
	UINT32 value = 0;

	RTMP_IO_READ32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_CONN_ON_MISC_drv_fw_stat_sync_ADDR, &value);
	value = (value & CONN_HOST_CSR_TOP_CONN_ON_MISC_drv_fw_stat_sync_MASK) >> CONN_HOST_CSR_TOP_CONN_ON_MISC_drv_fw_stat_sync_SHFT;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: current sync CR = 0x%x\n", __func__, value));
	return value;
}

#ifdef CONFIG_FWOWN_SUPPORT


#ifdef RTMP_MAC_PCI
static VOID pci_fw_own_by_port(RTMP_ADAPTER *ad, UINT8 port_idx)
{
	UINT32 cr_addr = 0;
	UINT32 cr_val = 0;

	cr_addr = hif_ownsership_cr[port_idx][OWNERSHIP_CR_TYPE_OWN];
	cr_val = HOST_SET_FW_OWN_MASK;
	RTMP_IO_WRITE32(ad->hdev_ctrl, cr_addr, cr_val);
	if (port_idx == 0)
		ad->bDrvOwn = FALSE;
	else
		ad->bDrvOwn1 = FALSE;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fw own to p(%d)\n", __func__, port_idx));
}

static INT32 pci_driver_own_by_port(RTMP_ADAPTER *ad, UINT8 port_idx)
{
#define MAX_RETRY_CNT 4
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT32 retrycnt = 0;
	UINT32 counter = 0;
	UINT32 cr_addr = 0;
	UINT32 cr_val = 0;
	UINT8 drv_own = 0;
	UINT8 drv_own_from = 0;

#ifdef CONFIG_COLGIN_MT6890
	UINT32 test_cr_addr = 0;
	UINT32 test_cr_val = 0;
	UINT32 test_i = 0;

    test_cr_addr = CONN_HOST_CSR_TOP_WM_MCU_PC_DBG_ADDR;
    while (test_i < (FW_OWN_POLLING_COUNTER/1000))
    {
        test_i++;
        RTMP_IO_READ32(ad->hdev_ctrl, test_cr_addr, &test_cr_val);
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
            ("%s: Before: %d, p=%d drv_own: %d, fw own, from(%d), test_cr_addr=0x%x, test_cr_val=0x%x.\n",
            __func__, test_i, port_idx, drv_own, drv_own_from, test_cr_addr, test_cr_val));
    }
#endif
	do {
		retrycnt++;

		if (port_idx == 0)
			drv_own = ad->bDrvOwn;
		else
			drv_own = ad->bDrvOwn1;

		if (drv_own == TRUE) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Return since p=%d already in driver own\n", __func__, port_idx));
			return ret;
		}

		/* write cr to get driver own */
		cr_addr = hif_ownsership_cr[port_idx][OWNERSHIP_CR_TYPE_OWN];
		cr_val = HOST_CLR_FW_OWN_MASK;
		RTMP_IO_WRITE32(ad->hdev_ctrl, cr_addr, cr_val);

		/* poll driver own status */
		counter = 0;
		while (counter < FW_OWN_POLLING_COUNTER) {
			RtmpusecDelay(1000);
#ifdef CONFIG_COLGIN_MT6890
            if ((ad->bIsLowPower == TRUE) && (0 == (counter%10))) {
                cr_addr = hif_ownsership_cr[port_idx][OWNERSHIP_CR_TYPE_OWN];
                RTMP_IO_READ32(ad->hdev_ctrl, cr_addr, &cr_val);
                if (!(cr_val & HOST_FW_OWN_SYNC_MASK)) {
                    /* Clear IRQ */
                    cr_addr = hif_ownsership_cr[port_idx][OWNERSHIP_CR_TYPE_OWN_INT_STS];
                    RTMP_IO_WRITE32(ad->hdev_ctrl, cr_addr, HOST_FW_OWN_CLR_STAT_MASK);

                    drv_own = TRUE;
                    drv_own_from = DRIVER_OWN_POLLING_MODE;
                    if (port_idx == 0)
                        ad->bDrvOwn = TRUE;
                    else
                        ad->bDrvOwn1 = TRUE;

                    break;
                }
            }
#endif
			if (port_idx == 0)
				drv_own = ad->bDrvOwn;
			else
				drv_own = ad->bDrvOwn1;

			if (drv_own == TRUE) {
				drv_own_from = DRIVER_OWN_INTERRUPT_MODE;
				break;
			}

			counter++;
		};

		if (counter == FW_OWN_POLLING_COUNTER) {
			cr_addr = hif_ownsership_cr[port_idx][OWNERSHIP_CR_TYPE_OWN];
			RTMP_IO_READ32(ad->hdev_ctrl, cr_addr, &cr_val);

			if (!(cr_val & HOST_FW_OWN_SYNC_MASK)) {
				drv_own = TRUE;
				drv_own_from = DRIVER_OWN_POLLING_MODE;
				if (port_idx == 0)
					ad->bDrvOwn = TRUE;
				else
					ad->bDrvOwn1 = TRUE;
			}
		}

		if (drv_own)
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: success to clear p=%d fw own, from(%d): 1 is interrupt mode, 2 is polling mode.\n", __func__, port_idx, drv_own_from));
		else {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fail to clear p=%d fw_own\n", __func__, port_idx));

			if (retrycnt >= MAX_RETRY_CNT)
				ret = NDIS_STATUS_FAILURE;
		}
	} while (drv_own == FALSE && retrycnt < MAX_RETRY_CNT);

	return ret;
}

static BOOLEAN pci_fw_own_sts_by_port(RTMP_ADAPTER *ad, UINT8 port_idx)
{
	BOOLEAN fw_own_sts = FALSE;

	if (port_idx == 0)
		fw_own_sts = (ad->bDrvOwn ? FALSE : TRUE);
	else
		fw_own_sts = (ad->bDrvOwn1 ? FALSE : TRUE);

	return fw_own_sts;
}
#endif /* RTMP_MAC_PCI */

static VOID fw_own(RTMP_ADAPTER *ad)
{
#ifdef RTMP_MAC_PCI
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(ad->hdev_ctrl);
	struct pci_hif_chip *slave_hif_chip = pci_hif->slave_hif_chip;

	if (IS_PCI_INF(ad)) {
		pci_fw_own_by_port(ad, 0);
		if (slave_hif_chip)
			pci_fw_own_by_port(ad, 1);
		else
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: port=1 is not enabled\n", __func__));
		return;
	}
#endif /*RTMP_MAC_PCI*/
}

static INT32 driver_own(RTMP_ADAPTER *ad)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
#ifdef RTMP_MAC_PCI
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(ad->hdev_ctrl);
	struct pci_hif_chip *slave_hif_chip = pci_hif->slave_hif_chip;

	if (IS_PCI_INF(ad)) {
		ret = pci_driver_own_by_port(ad, 0);
		if (ret != NDIS_STATUS_SUCCESS)
			return ret;

		if (slave_hif_chip) {
			ret = pci_driver_own_by_port(ad, 1);
			if (ret != NDIS_STATUS_SUCCESS)
				return ret;
		} else {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: port=1 is not enabled\n", __func__));
		}
	}
#endif /*RTMP_MAC_PCI*/

	return ret;
}

static BOOLEAN fw_own_sts(RTMP_ADAPTER *ad)
{
	BOOLEAN fw_own_sts = TRUE;
#ifdef RTMP_MAC_PCI
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(ad->hdev_ctrl);
	struct pci_hif_chip *slave_hif_chip = pci_hif->slave_hif_chip;

	if (IS_PCI_INF(ad)) {
		fw_own_sts &= pci_fw_own_sts_by_port(ad, 0);
		if (slave_hif_chip)
			fw_own_sts &= pci_fw_own_sts_by_port(ad, 1);
	}
#endif /*RTMP_MAC_PCI*/

	return fw_own_sts;
}
#endif /* CONFIG_FWOWN_SUPPORT */

static VOID mt7915_isr(struct pci_hif_chip *hif_chip)
{
	UINT32 IntSource = 0x00000000L;
	struct _PCI_HIF_T *pci_hif = hif_chip->hif;
	struct _RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(pci_hif->net_dev);
#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif
	struct pci_task_group *task_group = &hif_chip->task_group;
	struct pci_schedule_task_ops *sched_ops = hif_chip->schedule_task_ops;
	unsigned long flags = 0;

	pci_hif->bPCIclkOff = FALSE;
	/* hook function for wed extend isr handler */
	WLAN_HOOK_CALL(WLAN_HOOK_ISR, pAd, NULL);
	HIF_IO_READ32(pAd->hdev_ctrl, MT_INT_SOURCE_CSR, &IntSource);

	/*E1 work around for wfdma wed*/
#ifdef WFDMA_WED_COMPATIBLE
	if (MTK_REV_ET(pAd, MT7915, MT7915E1))
		IntSource |= MT_INT_DMA1_T20_DONE;
#endif /*WFDMA_WED_COMPATIBLE*/

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT_SOURCE_CSR, IntSource);
		return;
	}

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
		UINT32 reg;

		/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
		reg = IntSource & (MT_INT_CMD | MT_INT_RX | MT_INT_RxCoherent);

		if (!reg) {
			HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT_SOURCE_CSR, IntSource);
			return;
		}
	}

	/* Do nothing if NIC doesn't exist */
	if (IntSource == 0xffffffff) {
		RTMP_SET_FLAG(pAd, (fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_HALT_IN_PROGRESS));
		HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT_SOURCE_CSR, IntSource);
		return;
	}

	if (IntSource & MT_INT_TxCoherent)
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, (">>>TxCoherent<<<\n"));

	if (IntSource & MT_INT_RxCoherent)
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, (">>>RxCoherent<<<\n"));

	RTMP_INT_LOCK(&hif_chip->LockInterrupt, flags);

	if (IntSource & MT_INT_TX_DONE) {
		sched_ops->schedule_tx_dma_done(task_group);
		hif_chip->IntPending |= (IntSource & MT_INT_TX_DONE);
#ifdef CONFIG_TP_DBG
		tp_dbg->IsrTxCnt++;
#endif
	}

	if (IntSource & MT_INT_RX_DATA) {
		sched_ops->schedule_rx_data_done(task_group);
		hif_chip->IntPending |= (IntSource & MT_INT_RX_DATA);
#ifdef CONFIG_TP_DBG
		tp_dbg->IsrRxCnt++;
#endif
	}

	if (IntSource & MT_INT_RX_CMD) {
		sched_ops->schedule_rx_event_done(task_group);
		hif_chip->IntPending |= (IntSource & MT_INT_RX_CMD);

		if (MTK_REV_ET(pAd, MT7915, MT7915E1)) {
			if (pci_hif->slave_hif_chip) {
				struct pci_hif_chip *slave_hif_chip = pci_hif->slave_hif_chip;
				struct pci_task_group *slave_task_group = &slave_hif_chip->task_group;
				struct pci_schedule_task_ops *slave_sched_ops = slave_hif_chip->schedule_task_ops;
				unsigned long flag = 0;
				UINT32 int_src = 0;

				HIF_IO_READ32(pAd->hdev_ctrl,
						WF_WFDMA_HOST_DMA1_PCIE1_PCI_BASE(WF_WFDMA_HOST_DMA1_PCIE1_HOST_INT_STA_ADDR), &int_src);

				/* bit 7 is tx ring 19 */
				if (int_src & WF_WFDMA_HOST_DMA1_PCIE1_HOST_INT_STA_tx_done_int_sts_19_MASK) {
					HIF_IO_WRITE32(pAd->hdev_ctrl,
						WF_WFDMA_HOST_DMA1_PCIE1_PCI_BASE(WF_WFDMA_HOST_DMA1_PCIE1_HOST_INT_STA_ADDR),
						WF_WFDMA_HOST_DMA1_PCIE1_HOST_INT_STA_tx_done_int_sts_19_MASK);
					slave_sched_ops->schedule_tx_dma_done(slave_task_group);
					RTMP_INT_LOCK(&slave_hif_chip->LockInterrupt, flag);
					slave_hif_chip->IntPending |= MT_INT1_TX_DONE;
					RTMP_INT_UNLOCK(&slave_hif_chip->LockInterrupt, flag);
				}
			}
		}

#ifdef CONFIG_TP_DBG
		tp_dbg->IsrRx1Cnt++;
#endif
	}

	if (IntSource & MT_INT_SUBSYS_INT_STS) {
		sched_ops->schedule_subsys_int(task_group);
		hif_chip->IntPending |= MT_INT_SUBSYS_INT_STS;
	}

	if (IntSource & MT_INT_MCU2HOST_SW_INT_STS) {
		sched_ops->schedule_sw_int(task_group);
		hif_chip->IntPending |= MT_INT_MCU2HOST_SW_INT_STS;
	}

	HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT_SOURCE_CSR, IntSource);
#ifdef CONFIG_WIFI_MSI_SUPPORT
	RTMP_IO_WRITE32(pAd->hdev_ctrl, (MT_PCIE_MAC_BASE_ADDR + 0x18c), 0x01);
#endif

	mt_int_disable(pAd, hif_chip, IntSource);
	RTMP_INT_UNLOCK(&hif_chip->LockInterrupt, flags);
}

static VOID mt7916_isr(struct pci_hif_chip *hif_chip)
{
	UINT32 IntSource = 0x00000000L;
	struct _PCI_HIF_T *pci_hif = hif_chip->hif;
	struct _RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(pci_hif->net_dev);
#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif
	unsigned long flags = 0;
	struct pci_task_group *task_group = &hif_chip->task_group;
	struct pci_schedule_task_ops *sched_ops = hif_chip->schedule_task_ops;

	pci_hif->bPCIclkOff = FALSE;
	HIF_IO_READ32(pAd->hdev_ctrl, MT_INT1_SOURCE_CSR, &IntSource);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT1_SOURCE_CSR, IntSource);
		return;
	}

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
		UINT32 reg;

		/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
		reg = IntSource & (MT_INT1_RX | MT_INT1_RxCoherent);

		if (!reg) {
			HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT1_SOURCE_CSR, IntSource);
			return;
		}
	}

	/* Do nothing if NIC doesn't exist */
	if (IntSource == 0xffffffff) {
		RTMP_SET_FLAG(pAd, (fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_HALT_IN_PROGRESS));
		HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT1_SOURCE_CSR, IntSource);
		return;
	}

	if (IntSource & MT_INT1_TxCoherent)
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, (">>>TxCoherent<<<\n"));

	if (IntSource & MT_INT1_RxCoherent)
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, (">>>RxCoherent<<<\n"));

	RTMP_INT_LOCK(&hif_chip->LockInterrupt, flags);
	if (IntSource & MT_INT1_TX_DONE) {
		sched_ops->schedule_tx_dma_done(task_group);
		hif_chip->IntPending |= MT_INT1_TX_DONE;
#ifdef CONFIG_TP_DBG
		tp_dbg->IsrTxCnt++;
#endif
	}

	if (IntSource & MT_INT1_RX_DATA) {
		sched_ops->schedule_rx_data_done(task_group);
		hif_chip->IntPending |= MT_INT1_RX_DATA;
#ifdef CONFIG_TP_DBG
		tp_dbg->IsrRxCnt++;
#endif
	}

	if (IntSource & MT_INT1_RX_CMD) {
		sched_ops->schedule_rx_event_done(task_group);
		hif_chip->IntPending |= MT_INT1_RX_CMD;
#ifdef CONFIG_TP_DBG
		tp_dbg->IsrRx1Cnt++;
#endif
	}

	HIF_IO_WRITE32(pAd->hdev_ctrl, MT_INT1_SOURCE_CSR, IntSource);
	mt_int_disable(pAd, hif_chip, IntSource);
	RTMP_INT_UNLOCK(&hif_chip->LockInterrupt, flags);
}

static INT32 hif_cfg_dly_int(VOID *hdev_ctrl, UINT32 idx, UINT16 dly_number, UINT16 dly_time)
{
	UINT32 reg_val = 0;

	switch (idx) {
	case MT_DMA0_R0_RING_BASE:
		RTMP_IO_READ32(hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_ADDR, &reg_val);
		reg_val &= ~WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_PRI0_MAX_PTIME_MASK;
		reg_val |= (dly_time << WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_PRI0_MAX_PTIME_SHFT);
		reg_val &= ~WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_PRI0_MAX_PINT_MASK;
		reg_val |= (dly_number << WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_PRI0_MAX_PINT_SHFT);
		reg_val |= (1 << WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_PRI0_DLY_INT_EN_SHFT);
		RTMP_IO_WRITE32(hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_ADDR, reg_val);
		break;
	case MT_DMA0_R1_RING_BASE:
		RTMP_IO_READ32(hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_ADDR, &reg_val);
		reg_val &= ~WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PTIME_MASK;
		reg_val |= (dly_time << WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PTIME_SHFT);
		reg_val &= ~WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PINT_MASK;
		reg_val |= (dly_number << WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PINT_SHFT);
		reg_val |= (1 << WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_PRI1_DLY_INT_EN_SHFT);
		RTMP_IO_WRITE32(hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_PRI_DLY_INT_CFG0_ADDR, reg_val);
		break;
	case MT_DMA1_T18_RING_BASE:
		RTMP_IO_READ32(hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG0_ADDR, &reg_val);
		reg_val &= ~WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG0_PRI0_MAX_PTIME_MASK;
		reg_val |= (dly_time << WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG0_PRI0_MAX_PTIME_SHFT);
		reg_val &= ~WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG0_PRI0_MAX_PINT_MASK;
		reg_val |= (dly_number << WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG0_PRI0_MAX_PINT_SHFT);
		reg_val |= (1 << WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG0_PRI0_DLY_INT_EN_SHFT);
		RTMP_IO_WRITE32(hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG0_ADDR, reg_val);
		break;
	case MT_DMA1_T19_RING_BASE:
		RTMP_IO_READ32(hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG0_ADDR, &reg_val);
		reg_val &= ~WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PTIME_MASK;
		reg_val |= (dly_time << WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PTIME_SHFT);
		reg_val &= ~WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PINT_MASK;
		reg_val |= (dly_number << WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PINT_SHFT);
		reg_val |= (1 << WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG0_PRI1_DLY_INT_EN_SHFT);
		RTMP_IO_WRITE32(hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG0_ADDR, reg_val);
		break;
	case MT_DMA1_R0_RING_BASE:
		RTMP_IO_READ32(hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG1_ADDR, &reg_val);
		reg_val &= ~WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG1_PRI0_MAX_PTIME_MASK;
		reg_val |= (dly_time << WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG1_PRI0_MAX_PTIME_SHFT);
		reg_val &= ~WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG1_PRI0_MAX_PINT_MASK;
		reg_val |= (dly_number << WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG1_PRI0_MAX_PINT_SHFT);
		reg_val |= (1 << WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG1_PRI0_DLY_INT_EN_SHFT);
		RTMP_IO_WRITE32(hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG1_ADDR, reg_val);
		break;
	case MT_DMA1_R1_RING_BASE:
		RTMP_IO_READ32(hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG1_ADDR, &reg_val);
		reg_val &= ~WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG1_PRI1_MAX_PTIME_MASK;
		reg_val |= (dly_time << WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG1_PRI1_MAX_PTIME_SHFT);
		reg_val &= ~WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG1_PRI1_MAX_PINT_MASK;
		reg_val |= (dly_number << WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG1_PRI1_MAX_PINT_SHFT);
		reg_val |= (1 << WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG1_PRI1_DLY_INT_EN_SHFT);
		RTMP_IO_WRITE32(hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG1_ADDR, reg_val);
		break;
	case MT_DMA1_R2_RING_BASE:
		RTMP_IO_READ32(hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG2_ADDR, &reg_val);
		reg_val &= ~WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG2_PRI0_MAX_PTIME_MASK;
		reg_val |= (dly_time << WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG2_PRI0_MAX_PTIME_SHFT);
		reg_val &= ~WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG2_PRI0_MAX_PINT_MASK;
		reg_val |= (dly_number << WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG2_PRI0_MAX_PINT_SHFT);
		reg_val |= (1 << WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG2_PRI0_DLY_INT_EN_SHFT);
		RTMP_IO_WRITE32(hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_PRI_DLY_INT_CFG2_ADDR, reg_val);
		break;
	case MT_PCI1_DMA0_R1_RING_BASE:
		RTMP_IO_READ32(hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_PRI_DLY_INT_CFG0_ADDR, &reg_val);
		reg_val &= ~WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PTIME_MASK;
		reg_val |= (dly_time << WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PTIME_SHFT);
		reg_val &= ~WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PINT_MASK;
		reg_val |= (dly_number << WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PINT_SHFT);
		reg_val |= (1 << WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_PRI_DLY_INT_CFG0_PRI1_DLY_INT_EN_SHFT);
		RTMP_IO_WRITE32(hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_PRI_DLY_INT_CFG0_ADDR, reg_val);
		break;
	case MT_PCI1_DMA1_T19_RING_BASE:
		RTMP_IO_READ32(hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_PRI_DLY_INT_CFG0_ADDR, &reg_val);
		reg_val &= ~WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PTIME_MASK;
		reg_val |= (dly_time << WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PTIME_SHFT);
		reg_val &= ~WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PINT_MASK;
		reg_val |= (dly_number << WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_PRI_DLY_INT_CFG0_PRI1_MAX_PINT_SHFT);
		reg_val |= (1 << WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_PRI_DLY_INT_CFG0_PRI1_DLY_INT_EN_SHFT);
		RTMP_IO_WRITE32(hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_PRI_DLY_INT_CFG0_ADDR, reg_val);
		break;
	case MT_PCI1_DMA1_R2_RING_BASE:
		RTMP_IO_READ32(hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_PRI_DLY_INT_CFG0_ADDR, &reg_val);
		reg_val &= ~WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_PRI_DLY_INT_CFG0_PRI0_MAX_PTIME_MASK;
		reg_val |= (dly_time << WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_PRI_DLY_INT_CFG0_PRI0_MAX_PTIME_SHFT);
		reg_val &= ~WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_PRI_DLY_INT_CFG0_PRI0_MAX_PINT_MASK;
		reg_val |= (dly_number << WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_PRI_DLY_INT_CFG0_PRI0_MAX_PINT_SHFT);
		reg_val |= (1 << WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_PRI_DLY_INT_CFG0_PRI0_DLY_INT_EN_SHFT);
		RTMP_IO_WRITE32(hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_PRI_DLY_INT_CFG0_ADDR, reg_val);
		break;
	default:
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
						("Unknown rx ring delay int(0x%x) setting\n", idx));
		break;
	}

	return NDIS_STATUS_SUCCESS;
}

/*
*	init sub-layer interrupt enable mask
*/
static VOID chip_irq_init(RTMP_ADAPTER *pAd)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct pci_hif_chip *master_hif_chip = pci_hif->main_hif_chip;
	UINT32 reg_val = 0;
#ifdef CONFIG_FWOWN_SUPPORT
#ifdef RTMP_MAC_PCI
	/*
	 * Ownership interrupt setting
	 */
	/* CONN_HOST_CSR_TOP */
	RTMP_IO_READ32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_WF_BAND0_IRQ_ENA_conn_hif_on_wf_b0_irq_ena_ADDR, &reg_val);
	reg_val	|= (1 << CONN_HOST_CSR_TOP_WF_BAND0_IRQ_ENA_conn_hif_on_wf_b0_irq_ena_SHFT);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_WF_BAND0_IRQ_ENA_conn_hif_on_wf_b0_irq_ena_ADDR, reg_val);

	RTMP_IO_READ32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_WF_BAND1_IRQ_ENA_conn_hif_on_wf_b1_irq_ena_ADDR, &reg_val);
	reg_val	|= (1 << CONN_HOST_CSR_TOP_WF_BAND1_IRQ_ENA_conn_hif_on_wf_b1_irq_ena_SHFT);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, CONN_HOST_CSR_TOP_WF_BAND1_IRQ_ENA_conn_hif_on_wf_b1_irq_ena_ADDR, reg_val);

	/*
	 * SUBSYS2HOST interrupt setting
	 */
	/* 1. WFDMA_HOST_INT_ENA bit28, handled in mt7915_hif_ctrl_chip_init */
	/* 2. HOST_INT_ENA bit28 */
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_HOST_INT_ENA_subsys_int_ena_ADDR, &reg_val);
	reg_val	|= (1 << WF_WFDMA_HOST_DMA1_HOST_INT_ENA_subsys_int_ena_SHFT);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_HOST_INT_ENA_subsys_int_ena_ADDR, reg_val);
	/* 3. SUBSYS2HOST_INT_ENA bit8  */
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_SUBSYS2HOST_INT_ENA_conn_hif_on_host_int_ena_ADDR, &reg_val);
	reg_val	|= (1 << WF_WFDMA_HOST_DMA1_SUBSYS2HOST_INT_ENA_conn_hif_on_host_int_ena_SHFT);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_SUBSYS2HOST_INT_ENA_conn_hif_on_host_int_ena_ADDR, reg_val);

	/*
	 * MCU2HOST SWI setting
	 */
	/* 1. WFDMA_HOST_INT_ENA bit29, handled in mt7915_hif_ctrl_chip_init */
	/* 2. HOST_INT_ENA bit29 */
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_HOST_INT_ENA_mcu2host_sw_int_ena_ADDR, &reg_val);
	reg_val	|= (1 << WF_WFDMA_HOST_DMA1_HOST_INT_ENA_mcu2host_sw_int_ena_SHFT);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_HOST_INT_ENA_mcu2host_sw_int_ena_ADDR, reg_val);
	/* 3. MCU2HOST_SW_INT_ENA */
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_MCU2HOST_SW_INT_ENA_ADDR, &reg_val);
	reg_val	|= (MT_SW_INT_DRV_OWN | MT_SW_INT_DRV_OWN1);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_MCU2HOST_SW_INT_ENA_ADDR, reg_val);
#endif /* RTMP_MAC_PCI */
#endif /* CONFIG_FWOWN_SUPPORT */

	/* priority interrupt setting */
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_INT_RX_PRI_SEL_ADDR, &reg_val);
	reg_val |= WF_WFDMA_HOST_DMA0_WPDMA_INT_RX_PRI_SEL_WPDMA_INT_RX_RING0_PRI_SEL_MASK |
			   WF_WFDMA_HOST_DMA0_WPDMA_INT_RX_PRI_SEL_WPDMA_INT_RX_RING1_PRI_SEL_MASK;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_WPDMA_INT_RX_PRI_SEL_ADDR, reg_val);
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_INT_RX_PRI_SEL_ADDR, &reg_val);
	reg_val |= WF_WFDMA_HOST_DMA1_WPDMA_INT_RX_PRI_SEL_WPDMA_INT_RX_RING0_PRI_SEL_MASK |
			   WF_WFDMA_HOST_DMA1_WPDMA_INT_RX_PRI_SEL_WPDMA_INT_RX_RING1_PRI_SEL_MASK |
			   WF_WFDMA_HOST_DMA1_WPDMA_INT_RX_PRI_SEL_WPDMA_INT_RX_RING2_PRI_SEL_MASK;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_INT_RX_PRI_SEL_ADDR, reg_val);
	RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_INT_TX_PRI_SEL_ADDR, &reg_val);
	reg_val |= WF_WFDMA_HOST_DMA1_WPDMA_INT_TX_PRI_SEL_WPDMA_INT_TX_RING18_PRI_SEL_MASK |
			   WF_WFDMA_HOST_DMA1_WPDMA_INT_TX_PRI_SEL_WPDMA_INT_TX_RING19_PRI_SEL_MASK;
	RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_WPDMA_INT_TX_PRI_SEL_ADDR, reg_val);

	if (get_rid_value(master_hif_chip) != DEFAULT_RID) {
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_INT_RX_PRI_SEL_ADDR, &reg_val);
		reg_val |= WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_INT_RX_PRI_SEL_WPDMA_INT_RX_RING1_PRI_SEL_MASK;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA0_PCIE1_WPDMA_INT_RX_PRI_SEL_ADDR, reg_val);
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_INT_RX_PRI_SEL_ADDR, &reg_val);
		reg_val |= WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_INT_RX_PRI_SEL_WPDMA_INT_RX_RING2_PRI_SEL_MASK;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_INT_RX_PRI_SEL_ADDR, reg_val);
		RTMP_IO_READ32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_INT_TX_PRI_SEL_ADDR, &reg_val);
		reg_val |= WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_INT_TX_PRI_SEL_WPDMA_INT_TX_RING19_PRI_SEL_MASK;
		RTMP_IO_WRITE32(pAd->hdev_ctrl, WF_WFDMA_HOST_DMA1_PCIE1_WPDMA_INT_TX_PRI_SEL_ADDR, reg_val);
	}
}

static VOID pci_io_remap_l1_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	UINT32 backup_val = 0, tmp_val;

	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L1_REMAP_ADDR, &backup_val);
	tmp_val = (backup_val & ~HIF_ADDR_L1_REMAP_MASK);
	tmp_val |= GET_L1_REMAP_BASE(reg) << HIF_ADDR_L1_REMAP_SHFT;
	HIF_IO_WRITE32(hdev_ctrl, HIF_ADDR_L1_REMAP_ADDR, tmp_val);
	/* use read to push write */
	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L1_REMAP_ADDR, &backup_val);
	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L1_REMAP_BASE_ADDR + GET_L1_REMAP_OFFSET(reg), val);
}

static VOID pci_io_remap_l1_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	UINT32 backup_val = 0, tmp_val;

	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L1_REMAP_ADDR, &backup_val);
	tmp_val = (backup_val & ~HIF_ADDR_L1_REMAP_MASK);
	tmp_val |= GET_L1_REMAP_BASE(reg) << HIF_ADDR_L1_REMAP_SHFT;
	HIF_IO_WRITE32(hdev_ctrl, HIF_ADDR_L1_REMAP_ADDR, tmp_val);
	/* use read to push write */
	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L1_REMAP_ADDR, &backup_val);
	HIF_IO_WRITE32(hdev_ctrl, HIF_ADDR_L1_REMAP_BASE_ADDR + GET_L1_REMAP_OFFSET(reg), val);
}

static VOID pci_io_remap_l2_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	UINT32 backup_val = 0, tmp_val = 0;

	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L2_REMAP_ADDR, &backup_val);
	tmp_val = (backup_val & ~HIF_ADDR_L2_REMAP_MASK);
	tmp_val |= GET_L2_REMAP_BASE(reg) << HIF_ADDR_L2_REMAP_SHFT;
	HIF_IO_WRITE32(hdev_ctrl, HIF_ADDR_L2_REMAP_ADDR, tmp_val);
	/* use read to push write */
	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L2_REMAP_ADDR, &backup_val);
	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L2_REMAP_BASE_ADDR + GET_L2_REMAP_OFFSET(reg), val);
}

static VOID pci_io_remap_l2_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	UINT32 backup_val = 0, tmp_val = 0;

	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L2_REMAP_ADDR, &backup_val);
	tmp_val = (backup_val & ~HIF_ADDR_L2_REMAP_MASK);
	tmp_val |= GET_L2_REMAP_BASE(reg) << HIF_ADDR_L2_REMAP_SHFT;
	HIF_IO_WRITE32(hdev_ctrl, HIF_ADDR_L2_REMAP_ADDR, tmp_val);
	/* use read to push write */
	HIF_IO_READ32(hdev_ctrl, HIF_ADDR_L2_REMAP_ADDR, &backup_val);
	HIF_IO_WRITE32(hdev_ctrl, HIF_ADDR_L2_REMAP_BASE_ADDR + GET_L2_REMAP_OFFSET(reg), val);
}

static BOOLEAN pci_io_remap_is_l1_remap(UINT32 *reg)
{
	/* physical addr shall use layer 1 remap */
	if (IS_PHY_ADDR(*reg))
		return true;

	/* mcu view addr are handled depends on domain */
	/* CBTOP: MCU view addr equals to physical addr, already handled */

	/* CONN_INFRA: covert to phyiscal addr and use layer 1 remap */
	if (IS_CONN_INFRA_MCU_ADDR(*reg)) {
		(*reg) -= CONN_INFRA_MCU_TO_PHY_ADDR_OFFSET;
		return true;
	}

	/* WFSYS: shall use layer 2 remap */
	return false;
}
static VOID pci_io_remap_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	if (pci_io_remap_is_l1_remap(&reg))
		pci_io_remap_l1_read32(hdev_ctrl, reg, val);
	else
		pci_io_remap_l2_read32(hdev_ctrl, reg, val);
}

static VOID pci_io_remap_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	if (pci_io_remap_is_l1_remap(&reg))
		pci_io_remap_l1_write32(hdev_ctrl, reg, val);
	else
		pci_io_remap_l2_write32(hdev_ctrl, reg, val);
}

static VOID chip_cap_mcs_nss_init(struct mcs_nss_caps *mcs_nss)
{
#ifdef G_BAND_256QAM
	mcs_nss->g_band_256_qam = TRUE;
#endif
	mcs_nss->max_nss = 4;
	mcs_nss->bw160_max_nss = 2;
	mcs_nss->max_vht_mcs = VHT_MCS_CAP_9;
#ifdef DOT11_HE_AX
	mcs_nss->max_24g_ru_num = 2;
	mcs_nss->max_5g_ru_num = 3;
	if (mcs_nss->bw160_max_nss)
		mcs_nss->max_5g_ru_num = RU_IDX_ALLOC_NUM;
#endif
}

/*
*
*/
static VOID pci_interrupt_disable(struct _RTMP_ADAPTER *ad)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(ad->hdev_ctrl);
	struct pci_hif_chip *master_hif_chip = pci_hif->main_hif_chip;

	HIF_IO_WRITE32(ad->hdev_ctrl, MT_INT_MASK_CSR, 0);
	RTMP_IO_WRITE32(ad->hdev_ctrl, MT_PCIE_MAC_INT_ENABLE_ADDR, 0x0);
	/* configure PCIe 1 if it's in use, check if recognition ID is written */
	if (get_rid_value(master_hif_chip) != DEFAULT_RID) {
		HIF_IO_WRITE32(ad->hdev_ctrl, MT_INT1_MASK_CSR, 0);
		RTMP_IO_WRITE32(ad->hdev_ctrl, MT_PCIE_MAC1_INT_ENABLE_ADDR, 0x0);
	}

	RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
}

/*
*
*/
static VOID pci_interrupt_enable(struct _RTMP_ADAPTER *ad)
{
	unsigned long flags;
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(ad->hdev_ctrl);
	struct pci_hif_chip *master_hif_chip = pci_hif->main_hif_chip;
	int i = 0;

	RTMP_INT_LOCK(&ad->irq_lock, flags);
	/* clear garbage ints */
	HIF_IO_WRITE32(ad->hdev_ctrl, MT_INT_SOURCE_CSR, 0xffffffff);
	HIF_IO_WRITE32(ad->hdev_ctrl, MT_INT_MASK_CSR, 0);
	/* configure PCIe 1 if it's in use, check if recognition ID is written */
	if (get_rid_value(master_hif_chip) != DEFAULT_RID) {
		HIF_IO_WRITE32(ad->hdev_ctrl, MT_INT1_SOURCE_CSR, 0xffffffff);
		HIF_IO_WRITE32(ad->hdev_ctrl, MT_INT1_MASK_CSR, 0);
	}

	/* traverse each pci_hif_chip */
	for (i = 0; i < pci_hif->pci_hif_chip_num; i++) {
		UINT32 val = 0;
		struct pci_hif_chip *hif_chip = pci_hif->pci_hif_chip[i];

		HIF_IO_READ32(ad->hdev_ctrl, hif_chip->int_ena_reg_addr, &val);
		HIF_IO_WRITE32(ad->hdev_ctrl, hif_chip->int_ena_reg_addr, val | hif_chip->int_enable_mask);
	}

	/* Master Switch of PCIE Interrupt Enable */
	RTMP_IO_WRITE32(ad->hdev_ctrl, MT_PCIE_MAC_INT_ENABLE_ADDR, 0x000000ff);
	/* configure PCIe 1 if it's in use, check if recognition ID is written */
	if (get_rid_value(master_hif_chip) != DEFAULT_RID) {
		RTMP_IO_WRITE32(ad->hdev_ctrl, MT_PCIE_MAC1_INT_ENABLE_ADDR, 0x000000ff);
	}

	RTMP_SET_FLAG(ad, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
	RTMP_INT_UNLOCK(&ad->irq_lock, flags);
}

static INT pci_trigger_int_to_mcu(RTMP_ADAPTER *ad, UINT32 status)
{
#ifdef RTMP_MAC_PCI
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_WARN, ("%s::status=0x%x\n",
			 __func__, status));
	MAC_IO_WRITE32(ad->hdev_ctrl,
		MT_MCU_INT_EVENT,
		status);
#endif /* RTMP_MAC_PCI */
	return TRUE;
}

static VOID pci_subsys_int_handler(RTMP_ADAPTER *ad, void *hif_chip_ptr)
{
	UINT32 cr_addr = 0;
	UINT32 cr_val = 0;
	unsigned long flags;
	struct pci_hif_chip *hif_chip = (struct pci_hif_chip *)hif_chip_ptr;

	if (!(hif_chip->IntPending & MT_INT_SUBSYS_INT_STS))
		return;

#ifdef CONFIG_FWOWN_SUPPORT
	/* read conn_host_csr to check CLR OWN come from which pcie port */
	cr_addr = hif_ownsership_cr[DBDC_BAND0][OWNERSHIP_CR_TYPE_OWN_INT_STS];
	RTMP_IO_READ32(ad->hdev_ctrl, cr_addr, &cr_val);

	if (cr_val & HOST_FW_OWN_CLR_STAT_MASK) {
		ad->bDrvOwn = TRUE;
		cr_val = HOST_FW_OWN_CLR_STAT_MASK;
		RTMP_IO_WRITE32(ad->hdev_ctrl, cr_addr, cr_val);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("%s::DriverOwn\n", __func__));
	}

	cr_addr = hif_ownsership_cr[DBDC_BAND1][OWNERSHIP_CR_TYPE_OWN_INT_STS];
	RTMP_IO_READ32(ad->hdev_ctrl, cr_addr, &cr_val);

	if (cr_val & HOST_FW_OWN_CLR_STAT_MASK) {
		ad->bDrvOwn1 = TRUE;
		cr_val = HOST_FW_OWN_CLR_STAT_MASK;
		RTMP_IO_WRITE32(ad->hdev_ctrl, cr_addr, cr_val);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("%s::DriverOwn1\n", __func__));
	}
#endif /* CONFIG_FWOWN_SUPPORT */

	RTMP_INT_LOCK(&hif_chip->LockInterrupt, flags);
	hif_chip->IntPending &= ~MT_INT_SUBSYS_INT_STS;
	mt_int_enable(ad, hif_chip, MT_INT_SUBSYS_INT_STS);
	RTMP_INT_UNLOCK(&hif_chip->LockInterrupt, flags);
}

static VOID pci_sw_int_handler(RTMP_ADAPTER *ad, void *hif_chip_ptr)
{
	UINT32 sw_int_sta = 0;
	unsigned long flags;
	struct pci_hif_chip *hif_chip = (struct pci_hif_chip *)hif_chip_ptr;
	struct pci_task_group *task_group = &hif_chip->task_group;
	struct pci_schedule_task_ops *sched_ops = hif_chip->schedule_task_ops;
	BOOLEAN en_sw_int = TRUE;

	if (!(hif_chip->IntPending & MT_INT_MCU2HOST_SW_INT_STS))
		return;

#ifdef RTMP_MAC_PCI
	RTMP_IO_READ32(ad->hdev_ctrl, WF_WFDMA_HOST_DMA1_MCU2HOST_SW_INT_STA_ADDR, &sw_int_sta);
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_WARN, ("%s::sw_int_sta=0x%x\n", __func__, sw_int_sta));

#ifdef CONFIG_FWOWN_SUPPORT
	if (sw_int_sta & MT_SW_INT_DRV_OWN) {
		RTMP_IO_WRITE32(ad->hdev_ctrl, WF_WFDMA_HOST_DMA1_MCU2HOST_SW_INT_STA_ADDR, MT_SW_INT_DRV_OWN);
		pci_driver_own_by_port(ad, 0);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("%s::SWI DriverOwn\n", __func__));
	}

	if (sw_int_sta & MT_SW_INT_DRV_OWN1) {
		RTMP_IO_WRITE32(ad->hdev_ctrl, WF_WFDMA_HOST_DMA1_MCU2HOST_SW_INT_STA_ADDR, MT_SW_INT_DRV_OWN1);
		pci_driver_own_by_port(ad, 1);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("%s::SWI DriverOwn1\n", __func__));
	}
#endif /* CONFIG_FWOWN_SUPPORT */

#ifdef ERR_RECOVERY
	if (sw_int_sta & MT7663_ERROR_DETECT_MASK) {
		/* updated ErrRecovery Status. */
		RTMP_SPIN_LOCK_IRQSAVE(&hif_chip->LockInterrupt, &flags);
		ad->ErrRecoveryCtl.status = sw_int_sta;
		RTMP_SPIN_UNLOCK_IRQRESTORE(&hif_chip->LockInterrupt, &flags);

		/* Clear SW INT status*/
		RTMP_IO_WRITE32(ad->hdev_ctrl, WF_WFDMA_HOST_DMA1_MCU2HOST_SW_INT_STA_ADDR, sw_int_sta);

		/* Trigger error recovery process with fw reload. */
		sched_ops->schedule_mac_recovery(task_group);

		/* dump hw error once */
		chip_dump_ser_stat(ad,
			(sw_int_sta & ERROR_DETECT_STOP_PDMA) ? DBG_LVL_ERROR : DBG_LVL_OFF);

		en_sw_int = FALSE;
	}
#endif /* ERR_RECOVERY */

#endif /* RTMP_MAC_PCI */

	/* re-enable SW_INT, SER will do in recovey_task */
	if (en_sw_int) {
		RTMP_INT_LOCK(&hif_chip->LockInterrupt, flags);
		hif_chip->IntPending &= ~MT_INT_MCU2HOST_SW_INT_STS;
		mt_int_enable(ad, hif_chip, MT_INT_MCU2HOST_SW_INT_STS);
		RTMP_INT_UNLOCK(&hif_chip->LockInterrupt, flags);
	}
}

/*
*
*/
static VOID mt7915_hif_chip_match(VOID *hdev_ctrl)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	struct pci_hif_chip *master_hif_chip = pci_hif->main_hif_chip;
	struct pci_hif_chip *slave_hif_chip;
	UINT32 cap = hc_get_asic_cap(hdev_ctrl);
	/*should get from Recognation CR*/
	UINT32 rid;
	UINT32 gid;

	/*set master gid*/
	gid = multi_hif_entry_id_get(master_hif_chip);
	multi_hif_entry_gid_set(master_hif_chip, gid);

	if (cap & fASIC_CAP_TWO_PCIE) {
		rid = multi_hif_entry_rid_get(master_hif_chip);
		if (rid != DEFAULT_RID) {
			rid--;
			slave_hif_chip = multi_hif_entry_get_by_id(rid);
			if (slave_hif_chip) {
				set_rid_value(slave_hif_chip, ++rid);
				pci_hif->slave_hif_chip = slave_hif_chip;
			}
		} else {
			rid = get_rid_value(master_hif_chip);
			multi_hif_entry_rid_set(master_hif_chip, rid);

			if (rid != DEFAULT_RID) {
				/*get slave and set gid
				*due to RID start from 1
				*always minius 1 for mapping to id of multi_hif_entry
				*/
				rid--;
				slave_hif_chip = multi_hif_entry_get_by_id(rid);

				if (slave_hif_chip) {
					multi_hif_entry_gid_set(slave_hif_chip, gid);
					multi_hif_entry_rid_set(slave_hif_chip, ++rid);
					pci_hif->slave_hif_chip = slave_hif_chip;
				}
			}
		}
	}
}

VOID mt7915_hif_pci_data_ring_assign(VOID *hdev_ctrl, UINT8 *resrc_idx)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
	UINT8 i, bn0_resrc_idx = 0, bn1_resrc_idx = 0;
	BOOLEAN is_2pci = FALSE;

	/* check if pcie 1 BN1 TXD ring exist */
	for (i = 0; i < pci_hif->tx_res_num; i++) {
		struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(pci_hif, i);

		if (tx_ring->hw_desc_base == MT_PCI1_DMA1_T19_RING_BASE) {
			is_2pci = TRUE;
			bn1_resrc_idx = tx_ring->resource_idx;
			break;
		}
	}

	if (is_2pci == FALSE)
		return;

	/* get pcie 0 BN0 TXD ring */
	for (i = 0; i < pci_hif->tx_res_num; i++) {
		struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(pci_hif, i);

		if (tx_ring->hw_desc_base == MT_DMA1_T18_RING_BASE) {
			bn0_resrc_idx = tx_ring->resource_idx;
			break;
		}
	}

	resrc_idx[0] = bn0_resrc_idx;
	resrc_idx[1] = bn1_resrc_idx;
}

#ifdef ERR_RECOVERY
#define WF_SW_DEF_HWITS_WAR_EN_MASK				 WF_SW_DEF_CR_RSVD_DBG_0


/* 7915 E1 HWITS */
#define WF_SW_DEF_HWITS00009492_DOUB_RES_CNT		WF_SW_DEF_CR_RSVD_DBG_1
#define WF_SW_DEF_HWITS00012176_RX_ABORT_B0_CNT	 WF_SW_DEF_CR_RSVD_DBG_2
#define WF_SW_DEF_HWITS00012176_RX_ABORT_B1_CNT	 WF_SW_DEF_CR_RSVD_DBG_3
#define WF_SW_DEF_HWITS00012120_4B_ALIGN_CNT		WF_SW_DEF_CR_RSVD_DBG_4
#define WF_SW_DEF_HWITS00012017_RXIOC_B0_CNT		WF_SW_DEF_CR_RSVD_DBG_5
#define WF_SW_DEF_HWITS00012017_RXIOC_B1_CNT		WF_SW_DEF_CR_RSVD_DBG_6

#define WF_SW_DEF_HWITS00012120_4B_ALIGN_ERR_FLAG   WF_SW_DEF_CR_RSVD_DBG_7
#define WF_SW_DEF_HWITS00012017_RXIOC_LAST_FID	  WF_SW_DEF_CR_RSVD_DBG_8
#define WF_SW_DEF_HWITS00012017_RXIOC_LAST_RLBUF	WF_SW_DEF_CR_RSVD_DBG_9

#define WAR_HWITS00009492_EN						BIT(0)
#define WAR_HWITS00012176_EN						BIT(1)
#define WAR_HWITS00012120_EN						BIT(2)
#define WAR_HWITS00012017_EN						BIT(3)

/* 7915 E2 HWITS */
#define WF_SW_DEF_HWITS00014809_RX_ABORT_B0_CNT     WF_SW_DEF_CR_RSVD_DBG_1
#define WF_SW_DEF_HWITS00014809_RX_ABORT_B1_CNT     WF_SW_DEF_CR_RSVD_DBG_2
#define WF_SW_DEF_HWITS00016717_RX_ABORT_B0_CNT     WF_SW_DEF_CR_RSVD_DBG_3
#define WF_SW_DEF_HWITS00016717_RX_ABORT_B1_CNT     WF_SW_DEF_CR_RSVD_DBG_4
#define WF_SW_DEF_HWITS00018677_L1_RCOVERY_B0_CNT     WF_SW_DEF_CR_RSVD_DBG_8
#define WF_SW_DEF_HWITS00018677_L1_RCOVERY_B1_CNT     WF_SW_DEF_CR_RSVD_DBG_9

#define WAR_HWITS00014809_EN               BIT(0)
#define WAR_HWITS00016717_EN               BIT(1)
#define WAR_HWITS00018677_EN               BIT(2)

static VOID mt7915_dump_ser_stat(RTMP_ADAPTER *pAd, UINT8 dump_lvl)
{
	UINT32 reg_tmp_val = 0;
	struct ser_dump_list {
		char *name;
		UINT32 reg;
	} cr_list[] = {
		{"SER_STATUS       ", WF_SW_DEF_CR_SER_STATUS_ADDR},
		{"SER_PLE_ERR      ", WF_SW_DEF_CR_PLE_STATUS_ADDR},
		{"SER_PLE_ERR_1    ", WF_SW_DEF_CR_PLE1_STATUS_ADDR},
		{"SER_PLE_ERR_AMSDU", WF_SW_DEF_CR_PLE_AMSDU_STATUS_ADDR},
		{"SER_PSE_ERR      ", WF_SW_DEF_CR_PSE_STATUS_ADDR},
		{"SER_PSE_ERR_1    ", WF_SW_DEF_CR_PSE1_STATUS_ADDR},
		{"SER_LMAC_WISR6_B0", WF_SW_DEF_CR_LAMC_WISR6_BN0_STATUS_ADDR},
		{"SER_LMAC_WISR6_B1", WF_SW_DEF_CR_LAMC_WISR6_BN1_STATUS_ADDR},
		{"SER_LMAC_WISR7_B0", WF_SW_DEF_CR_LAMC_WISR7_BN0_STATUS_ADDR},
		{"SER_LMAC_WISR7_B1", WF_SW_DEF_CR_LAMC_WISR7_BN1_STATUS_ADDR}
	};
	UINT i, cr_list_num = ARRAY_SIZE(cr_list);
	UINT war_list_num;
	UINT32 en_mask = 0;
	struct war_dump_list {
		char *name;
		UINT32 en_bit;
		UINT32 reg;
	} *war_list;


	struct war_dump_list war_list_e1[] = {
		{"HWITS00009492_DOUB_RES CNT", WAR_HWITS00009492_EN, WF_SW_DEF_HWITS00009492_DOUB_RES_CNT},
		{"HWITS00012176_TRB_PTR0 CNT", WAR_HWITS00012176_EN, WF_SW_DEF_HWITS00012176_RX_ABORT_B0_CNT},
		{"HWITS00012176_TRB_PTR1 CNT", WAR_HWITS00012176_EN, WF_SW_DEF_HWITS00012176_RX_ABORT_B1_CNT},
		{"HWITS00012120_4B_ALIGN CNT", WAR_HWITS00012120_EN, WF_SW_DEF_HWITS00012120_4B_ALIGN_CNT},
		{"             LAST ERR FLAG", WAR_HWITS00012120_EN, WF_SW_DEF_HWITS00012120_4B_ALIGN_ERR_FLAG},
		{"HWITS00012017_RXIOC_B0 CNT", WAR_HWITS00012017_EN, WF_SW_DEF_HWITS00012017_RXIOC_B0_CNT},
		{"HWITS00012017_RXIOC_B1 CNT", WAR_HWITS00012017_EN, WF_SW_DEF_HWITS00012017_RXIOC_B1_CNT},
		{"            LAST ERR FID  ", WAR_HWITS00012017_EN, WF_SW_DEF_HWITS00012017_RXIOC_LAST_FID},
		{"            LAST ERR RLBUF", WAR_HWITS00012017_EN, WF_SW_DEF_HWITS00012017_RXIOC_LAST_RLBUF},
	};

	struct war_dump_list war_list_e2[] = {
		{"HWITS00014809_RX_ABORT_B0 CNT", WAR_HWITS00014809_EN, WF_SW_DEF_HWITS00014809_RX_ABORT_B0_CNT},
		{"HWITS00014809_RX_ABORT_B1 CNT", WAR_HWITS00014809_EN, WF_SW_DEF_HWITS00014809_RX_ABORT_B1_CNT},
		{"HWITS00016717_RX_ABORT_B0 CNT", WAR_HWITS00016717_EN, WF_SW_DEF_HWITS00016717_RX_ABORT_B0_CNT},
		{"HWITS00016717_RX_ABORT_B1 CNT", WAR_HWITS00016717_EN, WF_SW_DEF_HWITS00016717_RX_ABORT_B1_CNT},
		{"HWITS00018677_L1_RECOVERY_B0 CNT", WAR_HWITS00018677_EN, WF_SW_DEF_HWITS00018677_L1_RCOVERY_B0_CNT},
		{"HWITS00018677_L1_RECOVERY_B1 CNT", WAR_HWITS00018677_EN, WF_SW_DEF_HWITS00018677_L1_RCOVERY_B1_CNT},
	};

	if (MTK_REV_ET(pAd, MT7915, MT7915E1)) {
		war_list = war_list_e1;
		war_list_num = ARRAY_SIZE(war_list_e1);
	} else {
		war_list = war_list_e2;
		war_list_num = ARRAY_SIZE(war_list_e2);
	}

	/* SER_STATUS */
	MAC_IO_READ32(pAd->hdev_ctrl, cr_list[0].reg, &reg_tmp_val);
	MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_WARN,
			("%s,::E  R , %s = 0x%08X\n", __func__, cr_list[0].name, reg_tmp_val));

	/* dump level >= DBG_LVL_ERROR(1) */
	if (dump_lvl >= DBG_LVL_ERROR) {
		/* ser_dump_list */
		for (i = 1; i < cr_list_num; i++) {
			MAC_IO_READ32(pAd->hdev_ctrl, cr_list[i].reg, &reg_tmp_val);
			if (reg_tmp_val != 0) {
				MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
						("%s,::E  R , %s = 0x%08X\n", __func__, cr_list[i].name, reg_tmp_val));
			}
		}
	}

	/* dump level >= DBG_LVL_TRACE(3) */
	if (dump_lvl >= DBG_LVL_TRACE) {
		/* war_list */
		MAC_IO_READ32(pAd->hdev_ctrl, WF_SW_DEF_HWITS_WAR_EN_MASK, &en_mask);
		MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				("%s,::E  R , HWITS_WAR_EN_MASK = 0x%08X\n", __func__, en_mask));

		for (i = 0; i < war_list_num; i++) {
			MAC_IO_READ32(pAd->hdev_ctrl, war_list[i].reg, &reg_tmp_val);
				MTWF_LOG(DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
						("\t(WAR_%s) %s=%d\n", (en_mask & war_list[i].en_bit) ? "ENA" : "DIS",
						war_list[i].name, reg_tmp_val));
		}

		/* Add the EDCCA Time for Debug */
		Show_MibBucket_Proc(pAd, "");
	}
}

#ifdef CONFIG_TX_DELAY
static VOID mt7915_tx_deley_parm_init(UCHAR tx_delay_mode, struct tx_delay_control *tx_delay_ctl)
{
	if (tx_delay_mode == TX_DELAY_SW_MODE) {
		tx_delay_ctl->min_tx_delay_en_tp = MIN_AGG_EN_TP;
		tx_delay_ctl->max_tx_delay_en_tp = MAX_AGG_EN_TP;
		tx_delay_ctl->que_agg_timeout_value = QUE_AGG_TIMEOUT;
		tx_delay_ctl->min_pkt_len = MIN_AGG_PKT_LEN;
		tx_delay_ctl->max_pkt_len = MAX_AGG_PKT_LEN;
		tx_delay_ctl->tx_process_batch_cnt = TX_BATCH_CNT;
	} else if (tx_delay_mode == TX_DELAY_HW_MODE) {
		tx_delay_ctl->que_agg_timeout_value = HW_QUE_AGG_TIMEOUT;
		tx_delay_ctl->tx_process_batch_cnt = HW_TX_BATCH_CNT;
	}
}
#endif

#ifdef MT7915_E1_WORKAROUND
#ifdef WFDMA_WED_COMPATIBLE
/* E1 workaround for WED interrupt bit [28][29] not working, do status polling */
static VOID mt7915_sw_int_polling(RTMP_ADAPTER *ad)
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(ad->hdev_ctrl);
	struct pci_hif_chip *hif_chip = pci_hif->main_hif_chip;

	UINT32 IntSource;
	unsigned long flags = 0;

	RTMP_IO_READ32(ad->hdev_ctrl, MT_INT_SOURCE_CSR, &IntSource);

	/* Do nothing if NIC doesn't exist */
	if (IntSource == 0xffffffff)
		return;

	if (IntSource & MT_INT_SUBSYS_INT_STS) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_WARN,
			("%s, SUBSYS_INT polled (0x%x)\n", __func__, IntSource));

		RTMP_INT_LOCK(&hif_chip->LockInterrupt, flags);
		hif_chip->IntPending |= MT_INT_SUBSYS_INT_STS;
		RTMP_INT_UNLOCK(&hif_chip->LockInterrupt, flags);
		pci_subsys_int_handler(ad, hif_chip);
	}

	if (IntSource & MT_INT_MCU2HOST_SW_INT_STS) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_WARN,
			("%s, SW_INT polled (0x%x)\n", __func__, IntSource));

		/* indicate SW_INT to SER handle */
		RTMP_INT_LOCK(&hif_chip->LockInterrupt, flags);
		hif_chip->IntPending |= MT_INT_MCU2HOST_SW_INT_STS;
		RTMP_INT_UNLOCK(&hif_chip->LockInterrupt, flags);
		pci_sw_int_handler(ad, hif_chip);
	}
}
#endif
#endif

#endif
#define LMAC_TXD_MAX_SIZE (sizeof(struct txd_l))
#define CT_PARSE_PAYLOAD_LEN 72

#ifdef CFG_SUPPORT_FALCON_MURU
static UINT_8 muru_io_r_u8(struct _RTMP_ADAPTER *pAd, UINT_32 addr)
{
	UINT32 addr_4b = (addr) & 0xFFFFFFFC;
	UINT32 pos = (addr) & 0x3;
	UINT32 value = 0;

	RTMP_IO_READ32(pAd->hdev_ctrl, addr_4b, &value);

	return (UINT_8)(value >> (pos << 3));
}

static UINT_16 muru_io_r_u16(struct _RTMP_ADAPTER *pAd, UINT_32 addr)
{
	UINT32 addr_4b = (addr) & 0xFFFFFFFC;
	UINT32 pos = (addr) & 0x3;
	UINT32 value = 0;

	RTMP_IO_READ32(pAd->hdev_ctrl, addr_4b, &value);

	return (UINT_16)(value >> (pos << 3));
}

static UINT_32 muru_io_r_u32(struct _RTMP_ADAPTER *pAd, UINT_32 addr)
{
	UINT32 value = 0;

	RTMP_IO_READ32(pAd->hdev_ctrl, addr, &value);

	return value;
}


static INT_16 muru_io_r_i16(struct _RTMP_ADAPTER *pAd, UINT_32 addr)
{
	UINT32 addr_4b = (addr) & 0xFFFFFFFC;
	UINT32 pos = (addr) & 0x3;
	UINT32 value = 0;

	RTMP_IO_READ32(pAd->hdev_ctrl, addr_4b, &value);

	return (INT_16)(value >> (pos << 3));
}


INT SyncMuruSram(struct _RTMP_ADAPTER *pAd)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_DO_SRAM_SYNC;
	struct _CMD_ATTRIBUTE attr = {0};

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd));

	if (!msg) {
		Ret = 0;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesSendCmdMsg(pAd, msg);
error:
	return Ret;
}

VOID SyncMuruSramCheckAddr(struct _RTMP_ADAPTER *pAd, UINT32 Addr)
{
	if (Addr >= 0xE0000000) {
		SyncMuruSram(pAd);
		RtmpusecDelay(500);
	}
}

VOID set_muru_data(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_SET_GLO_ADDR;
	struct _CMD_ATTRIBUTE attr = {0};

	RTMP_STRING *macptr = NULL;
	UINT_32 Addr = 0;
	UINT_32 Value = 0;
	UINT_8 numofparam = 0, i = 0;
	CMD_MURU_SET_GLOBAL_ADDR rSetParam;

	if (arg == NULL)
		return;

	numofparam = delimitcnt(arg, "-") + 1;
	if (numofparam > 2) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Num of Input Parameters Wrong\n"));
		return;
	}

	for (i = 0, macptr = rstrtok(arg, "-"); macptr; macptr = rstrtok(NULL, "-"), i++) {

		if (i == 0)
			Addr = (UINT_32)os_str_tol(macptr, 0, 16);

		if (i == 1)
			Value = (UINT_32)os_str_tol(macptr, 0, 16);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Address = 0x%08X Value = %d\n", Addr, Value));

	os_zero_mem(&rSetParam, sizeof(CMD_MURU_SET_GLOBAL_ADDR));
	rSetParam.u4Addr  = Addr;
	rSetParam.u4Value = Value;

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(CMD_MURU_SET_GLOBAL_ADDR));

	if (!msg)
		goto error;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	rSetParam.u4Addr  = cpu2le32(rSetParam.u4Addr);
	rSetParam.u4Value = cpu2le32(rSetParam.u4Value);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&rSetParam, sizeof(CMD_MURU_SET_GLOBAL_ADDR));
	AndesSendCmdMsg(pAd, msg);
error:
	return;
}

static VOID show_muru_local_muru_para(struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_LOCAL_DATA_T, rMuruPara) */
	UINT_32 offset;
	UINT_32 addr;
	UINT_32 addr_u1TxCmdQLen, addr_ai1ManTargetRssi;
	BOOLEAN err = pAd->CommonCfg.rGloInfo.rLocalDataMuruPara.fgError;
	UINT_8  i = 0;

	if (!err) {
		/* rMuruPara */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rMuruPara (0x%08X)\n", subbase));

		offset = OFFSET_OF(MURU_PARA_T, fgPingPongAlgo);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgPingPongAlgo = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, fgSu);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgSu = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, fg256BitMap);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fg256BitMap = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, fgUlBsrp);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgUlBsrp = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, fgTxcmdsnd);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgTxcmdsnd = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, fgTpc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgTpc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, fgTpcManualMode);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgTpcManualMode = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, u2fixedTPNum);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2fixedTPNum = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, u1UlMpduCntPolicy);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1UlMpduCntPolicy = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, u1DelayPolicy);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1DelayPolicy = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, rTpcManPara);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) rTpcManPara\n", addr));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-|-(0x%08X) ai1ManTargetRssi\n", addr));

		addr_ai1ManTargetRssi = addr;
		for (i = 0; i < MAX_USER_IN_PPDU; i++) {
			addr = addr_ai1ManTargetRssi + sizeof(INT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-|-|-(0x%08X) ai1ManTargetRssi[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(MURU_PARA_T, fgTpcOptMode);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgTpcOptMode = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, u1TxCmdQLen);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TxCmdQLen\n", addr));

		addr_u1TxCmdQLen = addr;
		for (i = 0; i < MAX_DATA_AC_NUM; i++) {
			addr = addr_u1TxCmdQLen + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-|-(0x%08X) u1TxCmdQLen[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(MURU_PARA_T, fgTBSuAdaptiveLSIGLen);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgTBSuAdaptiveLSIGLen = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, fgSRState);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgSRState = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, u1TypeCDelayReq);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TypeCDelayReq = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, u4BsrTruncateThr);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4BsrTruncateThr = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, u2MaxStaCntLimit);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2MaxStaCntLimit = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, fgPreGrp);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgPreGrp = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, fgTxopBurst);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgTxopBurst = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, i2PsdDiffThr);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) i2PsdDiffThr = %d\n", addr, muru_io_r_i16(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, u1SplPriority);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SplPriority = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, u1DlSolictAckPolicy);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1DlSolictAckPolicy = %d\n", addr, muru_io_r_u8(pAd, addr)));
	}
}

static VOID show_muru_local_qlen_info(struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_LOCAL_DATA_T, rQlenInfo) */
	UINT_32 offset;
	UINT_32 addr;
	UINT_32 addr_au4DLQlen, addr_au4ULQlen;
	BOOLEAN err = pAd->CommonCfg.rGloInfo.rLocalDataQlenInfo.fgError;
	UINT_8   i = 0;

	if (!err) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rQlenInfo (0x%08X)\n", subbase));

		offset = OFFSET_OF(MURU_QLEN_INFO_T, au4DLQlen);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au4DLQlen\n", addr));

		addr_au4DLQlen = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_au4DLQlen + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-|-(0x%08X) au4DLQlen[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr)));
		}

		offset = OFFSET_OF(MURU_QLEN_INFO_T, au4ULQlen);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au4ULQlen\n", addr));

		addr_au4ULQlen = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_au4ULQlen + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-|-(0x%08X) au4ULQlen[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr)));
		}

		offset = OFFSET_OF(MURU_QLEN_INFO_T, u4TotDLQlenAllAc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4TotDLQlenAllAc = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_QLEN_INFO_T, u4TotULQlenAllAc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4TotULQlenAllAc = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_QLEN_INFO_T, u4BsrTruncateThr);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4BsrTruncateThr = %d\n", addr, muru_io_r_u32(pAd, addr)));
	}
}

static VOID show_muru_local_bsrp_ctrl(struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_LOCAL_DATA_T, rExt_Cmd_Bsrp_Ctrl)*/
	UINT_32 offset;
	UINT_32 addr;
	BOOLEAN err = pAd->CommonCfg.rGloInfo.rLocalDataBsrpCtrl.fgError;

	if (!err) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rExt_Cmd_Bsrp_Ctrl (0x%08X)\n", subbase));

		offset = OFFSET_OF(CMD_MURU_BSRP_CTRL, u1TriggerFlow);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TriggerFlow = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(CMD_MURU_BSRP_CTRL, u2BsrpInterval);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2BsrpInterval = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(CMD_MURU_BSRP_CTRL, u2BsrpRuAlloc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2BsrpRuAlloc = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(CMD_MURU_BSRP_CTRL, u4TriggerType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4TriggerType = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(CMD_MURU_BSRP_CTRL, fgExtCmdBsrp);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgExtCmdBsrp = %d\n", addr, muru_io_r_u8(pAd, addr)));
	}
}

static VOID show_muru_local_txcmd_ctrl(struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_LOCAL_DATA_T, rMuru_TxCmd_Ctrl)*/
	UINT_32 offset;
	UINT_32 addr;
	BOOLEAN err = pAd->CommonCfg.rGloInfo.rLocalDataTxCmdCtrl.fgError;

	if (!err) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rMuru_TxCmd_Ctrl (0x%08X)\n", subbase));

		offset = OFFSET_OF(MURU_TXCMD_CTRL_T, fgGlobalPreLoad);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgGlobalPreLoad = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TXCMD_CTRL_T, i2PuPreGrpMaxPsd_dBm);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) i2PuPreGrpMaxPsd_dBm = %d\n", addr, muru_io_r_i16(pAd, addr)));
	}
}

static VOID show_muru_local_data(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_32	base = pAd->CommonCfg.rGloInfo.rLocalData.u4Addr;
	UINT_32 offset;
	UINT_32 addr;
	UINT_8  i = 0;
	RTMP_STRING *macptr = NULL;
	CHAR InputStr[3][25] = {0};
	UINT_8  numofparam = 0;
	BOOLEAN err = FALSE;
	BOOLEAN muruparam = FALSE, qleninfo = FALSE, bsrpctrl = FALSE, txcmdctrl = FALSE;

	if (arg == NULL)
		return;

	numofparam = delimitcnt(arg, "-") + 1;
	if (numofparam >= 4) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Num of Input Parameters Wrong\n"));
		return;
	}

	for (i = 0, macptr = rstrtok(arg, "-"); macptr; macptr = rstrtok(NULL, "-"), i++) {
		if (strlen(macptr) <= 25)
			NdisMoveMemory(InputStr[i], macptr, strlen(macptr));

		if (!(NdisCmpMemory(InputStr[i], "all", strlen("all"))))
			muruparam = qleninfo = bsrpctrl = txcmdctrl = TRUE;

		if (!(NdisCmpMemory(InputStr[i], "muruparam", strlen("muruparam"))))
			muruparam = TRUE;

		if (!(NdisCmpMemory(InputStr[i], "qleninfo", strlen("qleninfo"))))
			qleninfo = TRUE;

		if (!(NdisCmpMemory(InputStr[i], "bsrpctrl", strlen("bsrpctrl"))))
			bsrpctrl = TRUE;

		if (!(NdisCmpMemory(InputStr[i], "txcmdctrl", strlen("txcmdctrl"))))
			txcmdctrl = TRUE;
	}

	SyncMuruSramCheckAddr(pAd, base);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("_rMuru_Local_Data (0x%08X)\n", base));

	err = pAd->CommonCfg.rGloInfo.rLocalDataMuruPara.fgError;

	if (muruparam == TRUE)
		show_muru_local_muru_para(pAd, pAd->CommonCfg.rGloInfo.rLocalDataMuruPara.u4Addr);

	if (!err) {
		offset = OFFSET_OF(MURU_LOCAL_DATA_T, eDuSchWeight);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-(0x%08X) eDuSchWeight = %d\n", addr, muru_io_r_u32(pAd, addr)));
	}

	if (qleninfo == TRUE)
		show_muru_local_qlen_info(pAd, pAd->CommonCfg.rGloInfo.rLocalDataQlenInfo.u4Addr);

	if (!err) {
		offset = OFFSET_OF(MURU_LOCAL_DATA_T, u2MuruSplHeadWlanId);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-(0x%08X) u2MuruSplHeadWlanId = %d\n", addr, muru_io_r_u16(pAd, addr)));
	}

	if (bsrpctrl == TRUE)
		show_muru_local_bsrp_ctrl(pAd, pAd->CommonCfg.rGloInfo.rLocalDataBsrpCtrl.u4Addr);

	if (txcmdctrl == TRUE)
		show_muru_local_txcmd_ctrl(pAd, pAd->CommonCfg.rGloInfo.rLocalDataTxCmdCtrl.u4Addr);

	if (!err) {
		offset = OFFSET_OF(MURU_LOCAL_DATA_T, fgMumUl);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-(0x%08X) fgMumUl = %d\n", addr, muru_io_r_u8(pAd, addr)));
	}
}

static VOID show_muru_txinfo_global_data(struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_TX_INFO_T, rGlobalData) */
	UINT_32 offset;
	UINT_32 addr;
	BOOLEAN err = pAd->CommonCfg.rGloInfo.rMuruTxInfoGlobalData.fgError;

	if (!err) {
		/* rGlobalData */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rGlobalData (0x%08X)\n", subbase));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1TxdNum);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TxdNum = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1Qid);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Qid = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1TxcmdType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TxcmdType = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgSpl);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgSpl = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1PresentSpTblIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1PresentSpTblIdx = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgTv);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgTv = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgDbdcIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgDbdcIdx = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgPreload);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgPreload = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgTxop);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgTxop = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1OwnMac);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1OwnMac = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgIgnoreBw);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgIgnoreBw = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgSmartAnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgSmartAnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PARA_T, fgSRState);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgSRState = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1AggPolicy);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AggPolicy = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1Bandwidth);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Bandwidth = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u4AntId);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4AntId = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1SerialId);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SerialId = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, u1SpeIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SpeIdx = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_GLOBAL_INFO_T, fgOptionalBackoff);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgOptionalBackoff = %d\n", addr, muru_io_r_u8(pAd, addr)));
	}
}

static VOID show_muru_txinfo_protect_data(struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_TX_INFO_T, rProtectData) */
	UINT_32 offset;
	UINT_32 addr, addr_rProtRuInfo;
	UINT_32 subbase_rProtRuInfo;
	BOOLEAN err = pAd->CommonCfg.rGloInfo.rMuruTxInfoProtectData.fgError;
	UINT_8  i = 0;
	UINT_8  StaCnt = 0;

	if (!err) {
		/* rProtectData */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rProtectData (0x%08X)\n", subbase));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, u1Protect);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Protect = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, u1StaCnt);
		addr = subbase + offset;
		StaCnt = muru_io_r_u8(pAd, addr);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1StaCnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, fgCascadeIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgCascadeIdx = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, fgCsRequired);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgCsRequired = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, u1TfPad);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TfPad = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, u1Rate);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Rate = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, u1TxMode);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TxMode = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, u1Nsts);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Nsts = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, fgCoding);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgCoding = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, fgDoppler);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgDoppler = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PROTECT_INFO_T, rProtRuInfo);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) rProtRuInfo\n", addr));

		addr_rProtRuInfo = addr;
		for (i = 0; i < StaCnt; i++) {
			subbase_rProtRuInfo = addr_rProtRuInfo + sizeof(PROT_RU_INFO_T) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-(0x%08X) rProtRuInfo[%d]\n", subbase_rProtRuInfo, i));

			offset = OFFSET_OF(PROT_RU_INFO_T, u2Aid);
			addr = subbase_rProtRuInfo + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u2Aid = %d\n", addr, muru_io_r_u16(pAd, addr)));

			offset = OFFSET_OF(PROT_RU_INFO_T, u1RuAlloc);
			addr = subbase_rProtRuInfo + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1RuAlloc = %d\n", addr, muru_io_r_u8(pAd, addr)));
		}
	}
}

static VOID show_muru_txinfo_tx_data(struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_TX_INFO_T, rSxnTxData) */
	UINT_32 offset;
	UINT_32 addr, addr_aucRuAlloc;
	UINT_32 subbase_arTxcmdUser, addr_arTxcmdUser;
	BOOLEAN err = pAd->CommonCfg.rGloInfo.rMuruTxInfoSxnTxData.fgError;
	UINT_8  i = 0;
	UINT_8  StaCnt = 0;

	if (!err) {
		/* rSxnTxData */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rSxnTxData (0x%08X)\n", subbase));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1Rxv);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Rxv = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgRsp);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgRsp = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgPsIgnore);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgPsIgnore = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1SigBCh1StaCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SigBCh1StaCnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1SigBCh2StaCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SigBCh2StaCnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1StaCnt);
		addr = subbase + offset;
		StaCnt = muru_io_r_u8(pAd, addr);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1StaCnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1SigBSym);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SigBSym = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1SigBMcs);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SigBMcs = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgRa);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgRa = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgSigBDcm);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgSigBDcm = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgSigBCompress);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgSigBCompress = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1LtfSym);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1LtfSym = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1Gi);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Gi = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgStbc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgStbc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgCmdPower);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgCmdPower = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u2MuPpduDur);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2MuPpduDur = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1TxPower);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TxPower = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, aucRuAlloc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) aucRuAlloc\n", addr));

		addr_aucRuAlloc = addr;
		for (i = 0; i < 8; i++) {
			addr = addr_aucRuAlloc + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-|-(0x%08X) aucRuAlloc[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(MURU_TX_DATA_T, fgDoppler);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgDoppler = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1PrimaryUserIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1PrimaryUserIdx = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1Ltf);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Ltf = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1TfPad);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TfPad = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1Mu0UserPosition);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Mu0UserPosition = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1Mu1UserPosition);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Mu1UserPosition = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1Mu2UserPosition);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Mu2UserPosition = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1Mu3UserPosition);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Mu3UserPosition = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1MuGroupId);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1MuGroupId = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgRu26dSigBCh1);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgRu26dSigBCh1 = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgRu26uSigBCh2);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgRu26uSigBCh2 = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1TxMode);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TxMode = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, fgDynamicBw);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgDynamicBw = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1PreamblePuncture);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1PreamblePuncture = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u1MuUser);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1MuUser = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u2ProtectionDuration);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2ProtectionDuration = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, u2ResponseDuration);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2ResponseDuration = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_DATA_T, arTxcmdUser);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) arTxcmdUser\n", addr));

		addr_arTxcmdUser = addr;
		for (i = 0; i < StaCnt; i++) {
			subbase_arTxcmdUser = addr_arTxcmdUser + sizeof(MURU_USER_INFO_T) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-(0x%08X) arTxcmdUser[%d]\n", subbase_arTxcmdUser, i));

			offset = OFFSET_OF(MURU_USER_INFO_T, u2TxPowerAlpha);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u2TxPowerAlpha = %d\n", addr, muru_io_r_u16(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgCoding);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgCoding = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u2WlanId);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u2WlanId = %d\n", addr, muru_io_r_u16(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1MuMimoGroup);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1MuMimoGroup = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1MuMimoSpatial);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1MuMimoSpatial = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1StartStream);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1StartStream = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgMultiTid);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgMultiTid = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgRuAllocBn);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgRuAllocBn = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1RuAlloc);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1RuAlloc = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1AckGroup);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1AckGroup = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgSuBar);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgSuBar = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgMuBar);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgMuBar = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgCbSta);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgCbSta = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgAggOld);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgAggOld = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgPreload);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgPreload = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1Rate);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1Rate = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1Nsts);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1Nsts = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1LpCtrl);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1LpCtrl = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgContentCh);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgContentCh = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1AckPol);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1AckPol = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u2SrRate);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u2SrRate = %d\n", addr, muru_io_r_u16(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u2RuRatio);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u2RuRatio = %d\n", addr, muru_io_r_u16(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgSplPrimaryUser);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgSplPrimaryUser = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1AcSeq);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1AcSeq = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1AcNum);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1AcNum = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u2BarRuRatio);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u2BarRuRatio = %d\n", addr, muru_io_r_u16(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u2LSigLen);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u2LSigLen = %d\n", addr, muru_io_r_u16(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1Bw);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1Bw = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1Ac0Ratio);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1Ac0Ratio = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1Ac1Ratio);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1Ac1Ratio = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1Ac2Ratio);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1Ac2Ratio = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1Ac3Ratio);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1Ac3Ratio = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1BarRate);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1BarRate = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1BarMode);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1BarMode = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1BarNsts);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1BarNsts = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1BaType);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1BaType = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgCsRequired);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgCsRequired = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1LtfType);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1LtfType = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1LtfSym);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1LtfSym = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgStbc);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgStbc = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgLdpcExtraSym);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgLdpcExtraSym = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1PktExt);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1PktExt = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgCoding2);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgCoding2 = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgDcm);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgDcm = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgBarAckPol);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgBarAckPol = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgAckRuAllocBn);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgAckRuAllocBn = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1AckRuAlloc);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1AckRuAlloc = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1AckMcs);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1AckMcs = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1SsAlloc);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1SsAlloc = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1TargetRssi);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1TargetRssi = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgDoppler);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgDoppler = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, fgBf);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgBf = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u1TidInfo);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1TidInfo = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_INFO_T, u2SpatialReuse);
			addr = subbase_arTxcmdUser + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u2SpatialReuse = %d\n", addr, muru_io_r_u16(pAd, addr)));
		}
	}
}

static VOID show_muru_txinfo_tx_trig_data(struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_TX_INFO_T, rSxnTrigData) */
	UINT_32 offset;
	UINT_32 addr, addr_au1RuAlloc;
	UINT_32 subbase_rTxcmdUserAck, addr_rTxcmdUserAck;
	BOOLEAN err = pAd->CommonCfg.rGloInfo.rMuruTxInfoSxnTrigData.fgError;
	UINT_8  i = 0;
	UINT_8  StaCnt = 0;

	if (!err) {
		/* rSxnTrigData */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rSxnTrigData (0x%08X)\n", subbase));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1Rxv);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Rxv = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1StaCnt);
		addr = subbase + offset;
		StaCnt = muru_io_r_u8(pAd, addr);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1StaCnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1BaPol);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1BaPol = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgPriOrder);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgPriOrder = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1SplAc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SplAc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1PreambPunc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1PreambPunc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1AckTxMode);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AckTxMode = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1TrigType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TrigType = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u4RxHetbCfg1);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4RxHetbCfg1 = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u4RxHetbCfg2);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4RxHetbCfg2 = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1TfPad);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TfPad = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u2LSigLen);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2LSigLen = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1SigBCh1StaCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SigBCh1StaCnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1SigBSym);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SigBSym = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1SigBMcs);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SigBMcs = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgSigBDcm);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgSigBDcm = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgSigBCompress);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgSigBCompress = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1LtfSym);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1LtfSym = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1Gi);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Gi = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgStbc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgStbc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgDoppler);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgDoppler = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgCmdPower);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgCmdPower = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1SigBCh2StaCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SigBCh2StaCnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u2MuPpduDur);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2MuPpduDur = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1Ltf);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Ltf = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgRu26dSigBCh1);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgRu26dSigBCh1 = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, fgRu26uSigBCh2);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgRu26uSigBCh2 = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, au1RuAlloc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au1RuAlloc\n", addr));

		addr_au1RuAlloc = addr;
		for (i = 0; i < 8; i++) {
			addr = addr_au1RuAlloc + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-|-(0x%08X) au1RuAlloc[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1AckTxPower);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AckTxPower = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1SsnUser);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SsnUser = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u1MuUser);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1MuUser = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, u2MsduId);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2MsduId = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_TX_TRIG_DATA_T, rTxcmdUserAck);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) rTxcmdUserAck\n", addr));

		addr_rTxcmdUserAck = addr;
		for (i = 0; i < StaCnt; i++) {
			subbase_rTxcmdUserAck = addr_rTxcmdUserAck + sizeof(MURU_USER_ACK_INFO_T) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-(0x%08X) rTxcmdUserAck[%d]\n", subbase_rTxcmdUserAck, i));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u2StaId);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u2StaId = %d\n", addr, muru_io_r_u16(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u2AckTxPowerAlpha);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u2AckTxPowerAlpha = %d\n", addr, muru_io_r_u16(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, fgCoding);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgCoding = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, fgContentCh);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgContentCh = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u2WlanId);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u2WlanId = %d\n", addr, muru_io_r_u16(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, fgRuAllocBn);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgRuAllocBn = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u1RuAlloc);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1RuAlloc = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u1Rate);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1Rate = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u1Nsts);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1Nsts = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u1RuAllNss);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1RuAllNss = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u2RuRatio);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u2RuRatio = %d\n", addr, muru_io_r_u16(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, fgSfEnable);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgSfEnable = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, u1Ac);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1Ac = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_USER_ACK_INFO_T, fgSplPrimaryUser);
			addr = subbase_rTxcmdUserAck + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) fgSplPrimaryUser = %d\n", addr, muru_io_r_u8(pAd, addr)));
		}
	}
}

static VOID show_muru_tx_info(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_32	base = pAd->CommonCfg.rGloInfo.rMuruTxInfo.u4Addr;
	UINT_8  i = 0;
	RTMP_STRING *macptr = NULL;
	CHAR InputStr[3][25] = {0};
	UINT_8  numofparam = 0;
	BOOLEAN Globaldata = FALSE, ProtectData = FALSE, SxnTxData = FALSE, SxnTrigData = FALSE;

	if (arg == NULL)
		return;

	numofparam = delimitcnt(arg, "-") + 1;
	if (numofparam >= 4) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Num of Input Parameters Wrong\n"));
		return;
	}

	for (i = 0, macptr = rstrtok(arg, "-"); macptr; macptr = rstrtok(NULL, "-"), i++) {
		if (strlen(macptr) <= 25)
			NdisMoveMemory(InputStr[i], macptr, strlen(macptr));

		if (!(NdisCmpMemory(InputStr[i], "all", strlen("all"))))
			Globaldata = ProtectData = SxnTxData = SxnTrigData = TRUE;

		if (!(NdisCmpMemory(InputStr[i], "Globaldata", strlen("Globaldata"))))
			Globaldata = TRUE;

		if (!(NdisCmpMemory(InputStr[i], "ProtectData", strlen("ProtectData"))))
			ProtectData = TRUE;

		if (!(NdisCmpMemory(InputStr[i], "SxnTxData", strlen("SxnTxData"))))
			SxnTxData = TRUE;

		if (!(NdisCmpMemory(InputStr[i], "SxnTrigData", strlen("SxnTrigData"))))
			SxnTrigData = TRUE;
	}

	SyncMuruSramCheckAddr(pAd, base);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("_rMuru_Tx_Info; (0x%08X)\n", base));

	if (Globaldata == TRUE)
		show_muru_txinfo_global_data(pAd, pAd->CommonCfg.rGloInfo.rMuruTxInfoGlobalData.u4Addr);

	if (ProtectData == TRUE)
		show_muru_txinfo_protect_data(pAd, pAd->CommonCfg.rGloInfo.rMuruTxInfoProtectData.u4Addr);

	if (SxnTxData == TRUE)
		show_muru_txinfo_tx_data(pAd, pAd->CommonCfg.rGloInfo.rMuruTxInfoSxnTxData.u4Addr);

	if (SxnTrigData == TRUE)
		show_muru_txinfo_tx_trig_data(pAd, pAd->CommonCfg.rGloInfo.rMuruTxInfoSxnTrigData.u4Addr);
}

static VOID show_ShareData_Share_Data(struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_SHARE_DATA_T, _rMuru_Share_Data) */
	UINT_32 offset;
	UINT_32 addr, addr_r;
	UINT_8 i;
	BOOLEAN err = pAd->CommonCfg.rGloInfo.rShareData.fgError;

	if (!err) {
		/* _rMuru_Share_Data */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-_rMuru_Share_Data (0x%08X)\n", subbase));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1PrimaryAc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1PrimaryAc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1PrimaryStaIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1PrimaryStaIdx = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1Qid);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Qid = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2MuRuMaxSplCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2MuRuMaxSplCnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1MaxStaCntInPpdu);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1MaxStaCntInPpdu = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2TypeAStaCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2TypeAStaCnt = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2TypeBStaCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2TypeBStaCnt = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2TypeCStaCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2TypeCStaCnt = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, eBandIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) eBandIdx = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1GlobalBw);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1GlobalBw = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgBsrpBandRequest);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgBsrpBandRequest\n", addr));

		addr_r = addr;
		for (i = 0; i < RAM_BAND_NUM; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) fgBsrpBandRequest[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(MURU_SHARE_DATA_T, eLastBsrpBandTx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) eLastBsrpBandTx = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1PuBw);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1PuBw = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, eTxCmdTye);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) eTxCmdTye = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, ePuRuBuftype);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) ePuRuBuftype = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgUplink);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgUplink = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgUlSnd);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgUlSnd = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, eSchtype);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) eSchtype = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1LastBSRPStaIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1LastBSRPStaIdx = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u4MaxRuAlgoTimeOut);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4MaxRuAlgoTimeOut = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1PpduDurBias);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1PpduDurBias = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1PreGrp);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1PreGrp = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgTxopBurst);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgTxopBurst = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgOptionalBackoff);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgOptionalBackoff = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, i2PsdDiffThr);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) i2PsdDiffThr = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgExp);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgExp = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1Pdc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Pdc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgProt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgProt = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u4ProtFrameThr);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4ProtFrameThr = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1ProtRuAlloc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1ProtRuAlloc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgFixedRate);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgFixedRate = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1TxDataSec_Bw);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TxDataSec_Bw = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u4TxDataSec_MuPpduDur);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4TxDataSec_MuPpduDur = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1TrigSec_BA_Policy);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TrigSec_BA_Policy = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1TrigSec_Global_BA_BW);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TrigSec_Global_BA_BW = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u4TrigSec_Global_BA_Dur);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4TrigSec_Global_BA_Dur = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, eTonePlanPolicy);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) eTonePlanPolicy = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1FixedMcs);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1FixedMcs = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1FixedNss);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1FixedNss = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1FixedBaMcs);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1FixedBaMcs = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1FixedBaNss);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1FixedBaNss = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u4PpduDuration);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4PpduDuration = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, fgUlMuBa);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgUlMuBa = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2UlAvgMpduCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2UlAvgMpduCnt = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u4UlAvgMpduSize);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4UlAvgMpduSize = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1MaxMuNum);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1MaxMuNum = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1TypeA_SwPdaPolicy);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TypeA_SwPdaPolicy = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1TypeB_SwPdaPolicy);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TypeB_SwPdaPolicy = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2MpduByte);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2MpduByte = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2QidNeedsDlSplTrigger);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2QidNeedsDlSplTrigger = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u2NonBsrpCount);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2NonBsrpCount = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1SplBackupSeq);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SplBackupSeq = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u4AcBitmapPreviousBsrp);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4AcBitmapPreviousBsrp = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1TriggerTypeOfBsrpTimer);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TriggerTypeOfBsrpTimer = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1DisableBsrpTimer);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1DisableBsrpTimer = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_SHARE_DATA_T, u1DisableULData);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1DisableULData = %d\n", addr, muru_io_r_u8(pAd, addr)));
	}
}

static VOID show_ShareData_RuAlloc_Data(struct _RTMP_ADAPTER *pAd, UINT32 base)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_SHARE_DATA_T, rRuAllocData) */
	UINT_32 offset;
	UINT_32 addr;
	BOOLEAN err = pAd->CommonCfg.rGloInfo.rShareDataRuAllocData.fgError;

	if (!err) {
		/* rRuAllocData */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rRuAllocData (0x%08X)\n", subbase));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, eBand);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) eBand = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1AggPol);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AggPol = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1Ac);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Ac = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, txCmdType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) txCmdType = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1SerialId);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SerialId = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1SpeIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SpeIdx = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1SigbSym);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SigbSym = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1LtfSym);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1LtfSym = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1SigbMcs);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SigbMcs = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1GiType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1GiType = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1LtfType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1LtfType = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1StaCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1StaCnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, eTxMode);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) eTxMode = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1AckGiType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AckGiType = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1AckLtfType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AckLtfType = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1AckMaxNss);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AckMaxNss = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TxPwr_dBm);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TxPwr_dBm = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1Bw);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Bw = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1PrimaryUserIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1PrimaryUserIdx = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4MuPpduDuration);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4MuPpduDuration = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4MaxBaMuPpduDur);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4MaxBaMuPpduDur = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4MaxBaDurForLSig);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4MaxBaDurForLSig = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1GrpId);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1GrpId = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TrigBaPL);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TrigBaPL = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TfType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TfType = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TrigSplAc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TrigSplAc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TrigAckBw);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TrigAckBw = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TrigAckTxPwr);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TrigAckTxPwr = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TrigAckTxMode);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TrigAckTxMode = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4LSigLength);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4LSigLength = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, ucTfPe);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) ucTfPe = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1TotMumGrpCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TotMumGrpCnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, eSchType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) eSchType = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1OperateBw);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1OperateBw = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1HavmDLULIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1HavmDLULIdx = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u1SplStaCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SplStaCnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u2TonePlanIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2TonePlanIdx = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u2TypeAStaCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2TypeAStaCnt = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u2TypeBStaCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2TypeBStaCnt = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4MaxHeadTime);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4MaxHeadTime = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4MaxScore);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4MaxScore = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4SuScore);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4SuScore = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4MuScore);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4MuScore = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4TotBitsOfThisTP);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4TotBitsOfThisTP = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4PpduTxDur);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4PpduTxDur = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(MURU_ALLOC_DATA_INFO_T, u4MuPpduUtilization);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4MuPpduUtilization = %d\n", addr, muru_io_r_u32(pAd, addr)));
	}
}

static VOID show_ShareData_User_Info(struct _RTMP_ADAPTER *pAd, UINT32 base, UINT16 ArrayIdx)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_SHARE_DATA_T, userInfo) */
	UINT_32 offset;
	UINT_32 addr, addr_u1AcRatio;
	BOOLEAN err = pAd->CommonCfg.rGloInfo.rShareDataUserInfo.fgError;
	UINT_8  i = 0;

	subbase = subbase + sizeof(PER_USER_INFO) * ArrayIdx;

	if (!err) {
		/* userInfo */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-userInfo (0x%08X)\n", subbase));

		offset = OFFSET_OF(PER_USER_INFO, u2WlanId);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2WlanId = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, fgUserPreLoad);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgUserPreLoad = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1MuMimoGrp);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1MuMimoGrp = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1RuAlloc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1RuAlloc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1RuTreeMapArrayIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1RuTreeMapArrayIdx = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1RuMapArrayIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1RuMapArrayIdx = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, fgRuAllocBn);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgRuAllocBn = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1MuMimoSpatial);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1MuMimoSpatial = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1StartStream);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1StartStream = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1RateMode);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1RateMode = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1Nss);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Nss = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1StartSpatialStream);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1StartSpatialStream = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1Mcs);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Mcs = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1Gi);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Gi = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, fgLdpc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgLdpc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u2WeightFactor);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2WeightFactor = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1SrMcs);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SrMcs = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1UpperMCS);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1UpperMCS = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, fgDcm);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgDcm = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u2RuRatio);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2RuRatio = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1RuAllNss);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1RuAllNss = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, fgAggOld);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgAggOld = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, fgCB);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgCB = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1AckBw);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AckBw = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1AcSeq);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AcSeq = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1AcNum);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AcNum = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u2BarRuRatio);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2BarRuRatio = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1AcRatio);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AcRatio\n", addr));

		addr_u1AcRatio = addr;
		for (i = 0; i < 4; i++) {
			addr = addr_u1AcRatio + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-|-(0x%08X) u1AcRatio[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(PER_USER_INFO, u2MumGrpIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2MumGrpIdx = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u2MumGrpStaCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2MumGrpStaCnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1LtfType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1LtfType = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, fgSplPrimaryUser);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgSplPrimaryUser = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1BfType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1BfType = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1AckPol);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AckPol = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1AckGrp);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AckGrp = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, fgSuBar);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgSuBar = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, fgMuBar);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgMuBar = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1BarRate);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1BarRate = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1BarMode);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1BarMode = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1BarNsts);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1BarNsts = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1BaType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1BaType = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u4BaMuPpduDur);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4BaMuPpduDur = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u4BaLSigDur);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4BaLSigDur = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, fgBaDcm);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgBaDcm = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, fgBaStbc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgBaStbc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1AckRuAlloc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AckRuAlloc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, fgAckRuAllocBn);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgAckRuAllocBn = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1AckMcs);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AckMcs = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1AckNss);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AckNss = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, fgAckLdpc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgAckLdpc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1BarAckPol);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1BarAckPol = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1SsAaloc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SsAaloc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1TargetRssi);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TargetRssi = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u1TidInfo);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TidInfo = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u2EffSnr);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2EffSnr = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u2TxPwrAlpha_dB);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2TxPwrAlpha_dB = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u4RuScore);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4RuScore = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, u4StaMuPpduDur);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4StaMuPpduDur = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(PER_USER_INFO, fgLargeRu);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgLargeRu = %d\n", addr, muru_io_r_u8(pAd, addr)));
	}
}

static VOID show_ShareData_StaRu_Record(struct _RTMP_ADAPTER *pAd, UINT32 base, UINT16 ArrayIdx)
{
	UINT_32 subbase = base; /* OFFSET_OF(MURU_SHARE_DATA_T, arStaRuRecord) */
	UINT_32 offset;
	UINT_32 addr;
	UINT_32 addr_r;
	UINT_8 i;
	UINT_32 addr_subbase;
	BOOLEAN err = pAd->CommonCfg.rGloInfo.rShareDataStaRuRecord.fgError;

	subbase = subbase + sizeof(STA_MURU_RECORD_T) * ArrayIdx;

	if (!err) {
		/* arStaRuRecord */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-arStaRuRecord (0x%08X)\n", subbase));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u2WlanId);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2WlanId = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u2StaIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2StaIdx = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1StaRecCapMode);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1StaRecCapMode = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1CapBw);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1CapBw = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1TxBw);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TxBw = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1BandIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1BandIdx = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1MumCapBitmap);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1MumCapBitmap = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1BfType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1BfType = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1Mcs);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Mcs = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1Nss);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Nss = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1Gi);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Gi = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1Ecc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Ecc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1HeLtf);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1HeLtf = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1Stbc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Stbc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1Priority);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1Priority = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, afgNonEmptyState);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) afgNonEmptyState\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) afgNonEmptyState[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1DlDepCmd);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1DlDepCmd\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) u1DlDepCmd[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1UlDepCmd);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1UlDepCmd\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) u1UlDepCmd[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1QoSRecIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1QoSRecIdx\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) u1QoSRecIdx[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1BsrpPeriod);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1BsrpPeriod = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1BsrpMaxPeriod);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1BsrpMaxPeriod = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, au1DlQuantum);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au1DlQuantum\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au1DlQuantum[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au1UlQuantum);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au1UlQuantum\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au1UlQuantum[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1DelayWeight);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1DelayWeight = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1HeSndPeriod);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1HeSndPeriod = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1HeSndMaxPeriod);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1HeSndMaxPeriod = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, fgRtsForMuPpduRetry);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgRtsForMuPpduRetry\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) fgRtsForMuPpduRetry[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1LastTxMcs);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1LastTxMcs = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, i1UlPwrHeadroom_dB);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) i1UlPwrHeadroom_dB = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1MinTxPwrNonHitCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1MinTxPwrNonHitCnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, afgCanNotAgg);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) afgCanNotAgg\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) afgCanNotAgg[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1McsSmallRU);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1McsSmallRU = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1LastRuBitmapIdx);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1LastRuBitmapIdx = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, i2MinRssi_dBm);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) i2MinRssi_dBm = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, i2LastPerUserRssi_dBm);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) i2LastPerUserRssi_dBm = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, i2PreGrpMaxPsd_dBm);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) i2PreGrpMaxPsd_dBm = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, i2RssiOffset_dB);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) i2RssiOffset_dB = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1RxRptLastBw);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1RxRptLastBw = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1RxRptLastTxMcs);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1RxRptLastTxMcs = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, i2RxRptLastPerUserRssi_dBm);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) i2RxRptLastPerUserRssi_dBm = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2MpduCntInPpdu);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au2MpduCntInPpdu\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au2MpduCntInPpdu[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2RxAvgMpduSize);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au2RxAvgMpduSize\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au2RxAvgMpduSize[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2CurrMsn);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au2CurrMsn\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au2CurrMsn[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2TxTotCnt);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au2TxTotCnt\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au2TxTotCnt[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2BaWin);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au2BaWin\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au2BaWin[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2NewMpduCntInPpdu);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au2NewMpduCntInPpdu\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au2NewMpduCntInPpdu[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2RxPer);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au2RxPer\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au2RxPer[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2HeadPktLen);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au2HeadPktLen\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au2HeadPktLen[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2UlHeadPktDelay);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au2UlHeadPktDelay\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au2UlHeadPktDelay[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au2RxAvgLongTermMpduSize);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au2RxAvgLongTermMpduSize\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au2RxAvgLongTermMpduSize[%d] = %d\n", addr, i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u2WeightFactor);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2WeightFactor = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1DataTxMode);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1DataTxMode = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1PpduType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1PpduType = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1DL_LTPpduType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1DL_LTPpduType\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) u1DL_LTPpduType[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1UL_LTPpduType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1UL_LTPpduType\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) u1UL_LTPpduType[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1DL_LTTrafficType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1DL_LTTrafficType\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) u1DL_LTTrafficType[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1UL_LTTrafficType);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1UL_LTTrafficType\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) u1UL_LTTrafficType[%d] = %d\n", addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1HeStaStae);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1HeStaStae = %d\n", addr, muru_io_r_u8(pAd, addr)));

		/*Start of Bitwise fields*/
		addr = addr + 1;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TurnCont = %s\n", addr,
				((muru_io_r_u8(pAd, addr) & 0x01) != 0) ? "TRUE" : "FALSE"));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgLastRxFrmTrig = %s\n", addr,
				((muru_io_r_u8(pAd, addr) & 0x02) != 0) ? "TRUE" : "FALSE"));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1InValidBsrCnt = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x1c)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1InValidByteCnt = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0xe0)));

		addr = addr + 1;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1UlTypeACnt = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x0f)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1UlNonTypeACnt = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0xf0)));

		addr = addr + 1;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1HasRxData = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x01)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1NoRxDataStopBsrp = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x02)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgBsrpCandidate = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x04)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgBsrpTriggerCurPPDU = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x08)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgIsTpcInfoValid = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x10)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgRxRptIsTpcInfoValid = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x20)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgIsTriggerred = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x40)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgTcp = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x80)));

		addr = addr + 1;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgHeSndCandidate = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x01)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgHeSndTriggerCurPPDU = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x02)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgUlSuSnd = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x04)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgSrAbortBit = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x08)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgDepCmd = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x10)));

		addr = addr + 1;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgMinTxPwrFlag = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x01)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgHaveHitMinTxPwrFg = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x02)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgAssocOk = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x04)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgNeedBroadcastRU = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x08)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgHasRuAssign = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x10)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgTcpHighTraf = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x20)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgTee = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x40)));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgRee = %d\n", addr, (muru_io_r_u8(pAd, addr) & 0x80)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1RxRate);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1RxRate = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1RxMode);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1RxMode = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1RxNsts);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1RxNsts = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1RxGi);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1RxGi = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1RxStbc);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1RxStbc = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1RxCoding);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1RxCoding = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1RxBW);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1RxBW = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u2MuRtsInactiveTimer);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2MuRtsInactiveTimer = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, eBand);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) eBand = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, prRetryForPktInfo);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) prRetryForPktInfo = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4DlTotQlenBytes);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au4DlTotQlenBytes\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au4DlTotQlenBytes[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4UlTotQlenBytes);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au4UlTotQlenBytes\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au4UlTotQlenBytes[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u4UlTotAllQlenBytes);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4UlTotAllQlenBytes = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4HeadPktTime);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au4HeadPktTime\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au4HeadPktTime[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4ByesInPpdu);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au4ByesInPpdu\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au4ByesInPpdu[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4UlSchTimeStamp);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au4UlSchTimeStamp\n", addr));

		addr_r = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au4UlSchTimeStamp[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u4RuScore);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4RuScore = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u4StaMuPpduDur);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4StaMuPpduDur = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, prRuCandidate);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) prRuCandidate = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4TidQueueSizeBytes);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au4TidQueueSizeBytes\n", addr));

		addr_r = addr;
		for (i = 0; i < MAX_TID_NUM; i++) {
			addr = addr_r + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| | |-(0x%08X) au4TidQueueSizeBytes[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4TotBytesTxInService);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au4TotBytesTxInService\n", addr));

		addr_subbase = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_subbase + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-|-(0x%08X) au4TotBytesTxInService[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4ServiceBytesRxPerSecond);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au4ServiceBytesRxPerSecond\n", addr));

		addr_subbase = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_subbase + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-|-(0x%08X) au4ServiceBytesRxPerSecond[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4TotBytesRxInService);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au4TotBytesRxInService\n", addr));

		addr_subbase = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_subbase + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-|-(0x%08X) au4TotBytesRxInService[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4AvgTxPpduDur);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au4AvgTxPpduDur\n", addr));

		addr_subbase = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_subbase + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-|-(0x%08X) au4AvgTxPpduDur[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, au4AvgRxPpduDur);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) au4AvgRxPpduDur\n", addr));

		addr_subbase = addr;
		for (i = 0; i < MURU_AC_NUM_MAX; i++) {
			addr = addr_subbase + sizeof(UINT_32) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-|-(0x%08X) au4AvgRxPpduDur[%d] = %d\n", addr, i, muru_io_r_u32(pAd, addr)));
		}

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1MuRtsFailStatusBuf);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1MuRtsFailStatusBuf = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(STA_MURU_RECORD_T, u1MuRtsFailScore);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1MuRtsFailScore = %d\n", addr, muru_io_r_u8(pAd, addr)));
	}
}

static VOID show_muru_shared_data(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_32	base = pAd->CommonCfg.rGloInfo.rShareData.u4Addr;
	UINT_8  i = 0;
	RTMP_STRING *macptr = NULL;
	CHAR InputStr[25] = {0};
	UINT_8  numofparam = 0;
	BOOLEAN ShareData = FALSE, RuAllocData = FALSE, UserInfo = FALSE, StaRuRecord = FALSE;
	UINT_16 ArrayIdx = 0;

	if (arg == NULL)
		return;

	numofparam = delimitcnt(arg, "-") + 1;
	if (numofparam >= 3) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Num of Input Parameters Wrong\n"));
		return;
	}

	for (i = 0, macptr = rstrtok(arg, "-"); macptr; macptr = rstrtok(NULL, "-"), i++) {

		if (i == 0) {
			if (strlen(macptr) <= sizeof(InputStr))
				NdisMoveMemory(InputStr, macptr, strlen(macptr));

			if (!(NdisCmpMemory(InputStr, "ShareData", strlen("ShareData"))))
				ShareData = TRUE;

			if (!(NdisCmpMemory(InputStr, "RuAllocData", strlen("RuAllocData"))))
				RuAllocData = TRUE;

			if (!(NdisCmpMemory(InputStr, "UserInfo", strlen("UserInfo"))))
				UserInfo = TRUE;

			if (!(NdisCmpMemory(InputStr, "StaRuRecord", strlen("StaRuRecord"))))
				StaRuRecord = TRUE;
		}

		if ((i == 1) && ((UserInfo == TRUE) || (StaRuRecord == TRUE)))
			ArrayIdx = (UINT16)os_str_tol(macptr, 0, 10);
	}

	SyncMuruSramCheckAddr(pAd, base);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("_rMuru_Shared_Data; (0x%08X)\n", base));

	if (ShareData == TRUE)
		show_ShareData_Share_Data(pAd, pAd->CommonCfg.rGloInfo.rShareData.u4Addr);

	if (RuAllocData == TRUE)
		show_ShareData_RuAlloc_Data(pAd, pAd->CommonCfg.rGloInfo.rShareDataRuAllocData.u4Addr);

	if (UserInfo == TRUE)
		show_ShareData_User_Info(pAd, pAd->CommonCfg.rGloInfo.rShareDataUserInfo.u4Addr, ArrayIdx);

	if (StaRuRecord == TRUE)
		show_ShareData_StaRu_Record(pAd, pAd->CommonCfg.rGloInfo.rShareDataStaRuRecord.u4Addr, ArrayIdx);
}

static VOID show_muru_mancfg_ctrl(struct _RTMP_ADAPTER *pAd)
{
	UINT_32 subbase, subbase_r;
	UINT_32 offset;
	UINT_32 addr;
	BOOLEAN err;
	UINT_8  i = 0;

	subbase = pAd->CommonCfg.rGloInfo.rMuruExtCmdManCfgInf.u4Addr;
	err     = pAd->CommonCfg.rGloInfo.rMuruExtCmdManCfgInf.fgError;
	SyncMuruSramCheckAddr(pAd, subbase);
	if (!err) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rMuruExtCmdManCfgInf (0x%08X)\n", subbase));
		for (i = 0; i < 2; i++) {
			subbase += sizeof(CMD_MURU_MANCFG_INTERFACER) * i;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("| |-rMuruExtCmdManCfgInf[%d] (0x%08X)\n", i, subbase));

			offset = OFFSET_OF(CMD_MURU_MANCFG_INTERFACER, u4ManCfgBmpCmm);
			addr = subbase + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-(0x%08X) u4ManCfgBmpCmm = %d\n", addr, muru_io_r_u32(pAd, addr)));

			offset = OFFSET_OF(CMD_MURU_MANCFG_INTERFACER, u4ManCfgBmpDl);
			addr = subbase + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-(0x%08X) u4ManCfgBmpDl = %d\n", addr, muru_io_r_u32(pAd, addr)));

			offset = OFFSET_OF(CMD_MURU_MANCFG_INTERFACER, u4ManCfgBmpUl);
			addr = subbase + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-(0x%08X) u4ManCfgBmpUl = %d\n", addr, muru_io_r_u32(pAd, addr)));

			offset = OFFSET_OF(CMD_MURU_MANCFG_INTERFACER, rCfgCmm);
			addr = subbase + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-(0x%08X) rCfgCmm\n", addr));

			subbase_r = addr;

			offset = OFFSET_OF(MURU_CMM_MANUAL_CONFIG, u1PpduFmt);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1PpduFmt = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_CMM_MANUAL_CONFIG, u1SchType);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1SchType = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_CMM_MANUAL_CONFIG, u1Band);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1Band = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_CMM_MANUAL_CONFIG, u1WmmSet);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1WmmSet = %d\n", addr, muru_io_r_u8(pAd, addr)));

			/* MURU_DL_MANUAL_CONFIG */
			offset = OFFSET_OF(CMD_MURU_MANCFG_INTERFACER, rCfgDl);
			addr = subbase + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-(0x%08X) rCfgDl\n", addr));

			subbase_r = addr;

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1UserCnt);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1UserCnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1TxMode);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1TxMode = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1Bw);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1Bw = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1GI);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1GI = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1Ltf);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1Ltf = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1SigBMcs);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1SigBMcs = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1SigBDcm);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1SigBDcm = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_DL_MANUAL_CONFIG, u1SigBCmprs);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1SigBCmprs = %d\n", addr, muru_io_r_u8(pAd, addr)));

			/* MURU_DL_MANUAL_CONFIG */
			offset = OFFSET_OF(CMD_MURU_MANCFG_INTERFACER, rCfgUl);
			addr = subbase + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-(0x%08X) rCfgUl\n", addr));

			subbase_r = addr;

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u1UserCnt);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1UserCnt = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u1TrigType);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1TrigType = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u2TrigCnt);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u2TrigCnt = %d\n", addr, muru_io_r_u16(pAd, addr)));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u2TrigIntv);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u2TrigIntv = %d\n", addr, muru_io_r_u16(pAd, addr)));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u1UlBw);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1UlBw = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u1UlGiLtf);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1UlGiLtf = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u2UlLength);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u2UlLength = %d\n", addr, muru_io_r_u16(pAd, addr)));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u1TfPad);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1TfPad = %d\n", addr, muru_io_r_u8(pAd, addr)));

			offset = OFFSET_OF(MURU_UL_MANUAL_CONFIG, u1BaType);
			addr = subbase_r + offset;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("| |-|-|-(0x%08X) u1BaType = %d\n", addr, muru_io_r_u8(pAd, addr)));
		}
	}

	subbase = pAd->CommonCfg.rGloInfo.rMuTxPktCnt.u4Addr;
	err     = pAd->CommonCfg.rGloInfo.rMuTxPktCnt.fgError;
	SyncMuruSramCheckAddr(pAd, subbase);
	if (!err) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rMuTxPktCnt (0x%08X)\n", subbase));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4MuTxPktCnt[0] = %d\n", subbase, muru_io_r_u32(pAd, subbase)));

		subbase = subbase + 4;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4MuTxPktCnt[1] = %d\n", subbase, muru_io_r_u32(pAd, subbase)));
	}

	subbase = pAd->CommonCfg.rGloInfo.rMuTxPktCntDwn.u4Addr;
	err     = pAd->CommonCfg.rGloInfo.rMuTxPktCntDwn.fgError;
	SyncMuruSramCheckAddr(pAd, subbase);
	if (!err) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rMuTxPktCntDwn (0x%08X)\n", subbase));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4MuTxPktCntDwn[0] = %d\n", subbase, muru_io_r_u32(pAd, subbase)));

		subbase = subbase + 4;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4MuTxPktCntDwn[1] = %d\n", subbase, muru_io_r_u32(pAd, subbase)));
	}

	subbase = pAd->CommonCfg.rGloInfo.rAggPolicy.u4Addr;
	err     = pAd->CommonCfg.rGloInfo.rAggPolicy.fgError;
	SyncMuruSramCheckAddr(pAd, subbase);
	if (!err) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rAggPolicy (0x%08X)\n", subbase));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1AggPolicy = %d\n", subbase, muru_io_r_u8(pAd, subbase)));
	}

	subbase = pAd->CommonCfg.rGloInfo.rDurationComp.u4Addr;
	err     = pAd->CommonCfg.rGloInfo.rDurationComp.fgError;
	SyncMuruSramCheckAddr(pAd, subbase);
	if (!err) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rDurationComp (0x%08X)\n", subbase));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1DurationComp = %d\n", subbase, muru_io_r_u8(pAd, subbase)));
	}
}

static VOID show_muru_stacapinfo_ctrl(struct _RTMP_ADAPTER *pAd, UINT_16 WlanIdx)
{
	UINT_32 subbase, subbase_r;
	UINT_32 offset;
	UINT_32 addr;
	BOOLEAN err;

	subbase = pAd->CommonCfg.rGloInfo.rMuruStaCapInfo.u4Addr;
	err     = pAd->CommonCfg.rGloInfo.rMuruStaCapInfo.fgError;
	subbase = subbase + sizeof(MURU_PURE_STACAP_INFO) * WlanIdx;
	SyncMuruSramCheckAddr(pAd, subbase);

	if (!err) {
		offset = OFFSET_OF(MURU_PURE_STACAP_INFO, rDlOfdma);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-(0x%08X) rDlOfdma\n", addr));

		subbase_r = addr;

		offset = OFFSET_OF(MURU_STA_DL_OFDMA, u1PhyPunRx);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) u1PhyPunRx = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_STA_DL_OFDMA, u120MIn40M2G);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) u120MIn40M2G = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_STA_DL_OFDMA, u120MIn160M);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) u120MIn160M = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_STA_DL_OFDMA, u180MIn160M);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) u180MIn160M = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_STA_DL_OFDMA, u1Lt16SigB);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) u1Lt16SigB = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_STA_DL_OFDMA, u1RxSUCompSigB);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) u1RxSUCompSigB = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_STA_DL_OFDMA, u1RxSUNonCompSigB);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) u1RxSUNonCompSigB = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PURE_STACAP_INFO, rUlOfdma);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-(0x%08X) rUlOfdma\n", addr));

		subbase_r = addr;

		offset = OFFSET_OF(MURU_STA_UL_OFDMA, u1TrigFrmPad);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) u1TrigFrmPad = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_STA_UL_OFDMA, u1MuCascading);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) u1MuCascading = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_STA_UL_OFDMA, u1UoRa);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) u1UoRa = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_STA_UL_OFDMA, u12x996Tone);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) u12x996Tone = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_STA_UL_OFDMA, u1RxTrgFrmBy11ac);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) u1RxTrgFrmBy11ac = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PURE_STACAP_INFO, rDlMimo);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-(0x%08X) rDlMimo\n", addr));

		subbase_r = addr;

		offset = OFFSET_OF(MURU_STA_DL_MIMO, fgVhtMuBfee);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) fgVhtMuBfee = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_STA_DL_MIMO, fgParBWDlMimo);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) fgParBWDlMimo = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_PURE_STACAP_INFO, rUlMimo);
		addr = subbase + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-(0x%08X) rUlMimo\n", addr));

		subbase_r = addr;

		offset = OFFSET_OF(MURU_STA_UL_MIMO, fgFullUlMimo);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) fgFullUlMimo = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(MURU_STA_UL_MIMO, fgParUlMimo);
		addr = subbase_r + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-|-(0x%08X) fgParUlMimo = %d\n", addr, muru_io_r_u8(pAd, addr)));
	}
}

static VOID show_muru_mancfg_data(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_8  i = 0;
	RTMP_STRING *macptr = NULL;
	CHAR InputStr[25] = {0};
	UINT_8  numofparam = 0;
	BOOLEAN ManCfg = FALSE;

	if (arg == NULL)
		return;

	numofparam = delimitcnt(arg, "-") + 1;
	if (numofparam >= 2) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Num of Input Parameters Wrong\n"));
		return;
	}

	for (i = 0, macptr = rstrtok(arg, "-"); macptr; macptr = rstrtok(NULL, "-"), i++) {
		if (strlen(macptr) <= sizeof(InputStr))
			NdisMoveMemory(InputStr, macptr, strlen(macptr));

		if (!(NdisCmpMemory(InputStr, "all", strlen("all"))))
			ManCfg = TRUE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("_rMuru_ManCfg_Data\n"));
	if (ManCfg == TRUE)
		show_muru_mancfg_ctrl(pAd);
}

static VOID show_muru_stacap_info(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_8  i = 0;
	RTMP_STRING *macptr = NULL;
	CHAR InputStr[25] = {0};
	UINT_8  numofparam = 0;
	UINT_16 WlanIdx = 0;

	if (arg == NULL)
		return;

	numofparam = delimitcnt(arg, "-") + 1;
	if (numofparam >= 2) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Num of Input Parameters Wrong\n"));
		return;
	}

	for (i = 0, macptr = rstrtok(arg, "-"); macptr; macptr = rstrtok(NULL, "-"), i++) {
		if (strlen(macptr) <= 25)
			NdisMoveMemory(InputStr, macptr, strlen(macptr));

		WlanIdx = (UINT16)os_str_tol(macptr, 0, 10);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("_Muru_StaCap_Info\n"));
	show_muru_stacapinfo_ctrl(pAd, WlanIdx);
}

static VOID show_muru_mum_group_tbl_entry(struct _RTMP_ADAPTER *pAd, UINT_16 GroupIdx)
{
	UINT_32 subbase;
	UINT_32 addr;
	BOOLEAN err;
	UINT_32 DW_Value = 0;
	P_MURU_MUM_GROUP_TBL_ENTRY_DW0 pDW0;
	P_MURU_MUM_GROUP_TBL_ENTRY_DW1 pDW1;
	P_MURU_MUM_GROUP_TBL_ENTRY_DW2 pDW2;
	P_MURU_MUM_GROUP_TBL_ENTRY_DW3 pDW3;

	subbase = pAd->CommonCfg.rGloInfo.rMuruMumGrpTable.u4Addr;
	err     = pAd->CommonCfg.rGloInfo.rMuruMumGrpTable.fgError;

	subbase = subbase + sizeof(MURU_MUM_GROUP_TBL_ENTRY_T) * GroupIdx;
	SyncMuruSramCheckAddr(pAd, subbase);
	if (!err) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("MURU MUM GROUP TABLE ENTRY: GROUP IDX = %d\n", GroupIdx));
		/* DW0 */
		addr = subbase;
		DW_Value = muru_io_r_u32(pAd, addr);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-DW0 (Addr: 0x%08X)(Value: 0x%08X)\n", addr, DW_Value));
		pDW0 = (P_MURU_MUM_GROUP_TBL_ENTRY_DW0)&DW_Value;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1NumUser    = %u\n", pDW0->rField.u1NumUser));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1DlGi       = %u\n", pDW0->rField.u1DlGi));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1UlGi       = %u\n", pDW0->rField.u1UlGi));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1Ax         = %u\n", pDW0->rField.u1Ax));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1PFIDUser0  = %u\n", pDW0->rField.u1PFIDUser0));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1PFIDUser1  = %u\n", pDW0->rField.u1PFIDUser1));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1PFIDUser2  = %u\n", pDW0->rField.u1PFIDUser2));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1PFIDUser3  = %u\n", pDW0->rField.u1PFIDUser3));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1DlVld      = %u\n", pDW0->rField.u1DlVld));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1UlVld      = %u\n", pDW0->rField.u1UlVld));

		/* DW1 */
		addr = subbase + 4;
		DW_Value = muru_io_r_u32(pAd, addr);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-DW1 (Addr: 0x%08X)(Value: 0x%08X)\n", addr, DW_Value));
		pDW1 = (P_MURU_MUM_GROUP_TBL_ENTRY_DW1)&DW_Value;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1RuAlloc    = %u\n", pDW1->rField.u1RuAlloc));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1NssUser0   = %u\n", pDW1->rField.u1NssUser0));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1NssUser1   = %u\n", pDW1->rField.u1NssUser1));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1NssUser2   = %u\n", pDW1->rField.u1NssUser2));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1NssUser3   = %u\n", pDW1->rField.u1NssUser3));

		/* DW2 */
		addr = subbase + 8;
		DW_Value = muru_io_r_u32(pAd, addr);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-DW2 (Addr: 0x%08X)(Value: 0x%08X)\n", addr, DW_Value));
		pDW2 = (P_MURU_MUM_GROUP_TBL_ENTRY_DW2)&DW_Value;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1DlMcsUser0 = %u\n", pDW2->rField.u1DlMcsUser0));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1DlMcsUser1 = %u\n", pDW2->rField.u1DlMcsUser1));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1DlMcsUser2 = %u\n", pDW2->rField.u1DlMcsUser2));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1DlMcsUser3 = %u\n", pDW2->rField.u1DlMcsUser3));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1DlWfUser0  = %u\n", pDW2->rField.u1DlWfUser0));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1DlWfUser1  = %u\n", pDW2->rField.u1DlWfUser1));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1DlWfUser2  = %u\n", pDW2->rField.u1DlWfUser2));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1DlWfUser3  = %u\n", pDW2->rField.u1DlWfUser3));

		/* DW3 */
		addr = subbase + 12;
		DW_Value = muru_io_r_u32(pAd, addr);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-DW3 (Addr: 0x%08X)(Value: 0x%08X)\n", addr, DW_Value));
		pDW3 = (P_MURU_MUM_GROUP_TBL_ENTRY_DW3)&DW_Value;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1UlMcsUser0 = %u\n", pDW3->rField.u1UlMcsUser0));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1UlMcsUser1 = %u\n", pDW3->rField.u1UlMcsUser1));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1UlMcsUser2 = %u\n", pDW3->rField.u1UlMcsUser2));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1UlMcsUser3 = %u\n", pDW3->rField.u1UlMcsUser3));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1UlWfUser0  = %u\n", pDW3->rField.u1UlWfUser0));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1UlWfUser1  = %u\n", pDW3->rField.u1UlWfUser1));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1UlWfUser2  = %u\n", pDW3->rField.u1UlWfUser2));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-u1UlWfUser3  = %u\n", pDW3->rField.u1UlWfUser3));
	}
}

static VOID show_mumimo_group_entry_tbl(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_8  i = 0;
	RTMP_STRING *macptr = NULL;
	CHAR InputStr[25] = {0};
	UINT_8  numofparam = 0;
	UINT_16 GroupIdx = 0;

	if (arg == NULL)
		return;

	numofparam = delimitcnt(arg, "-") + 1;
	if (numofparam >= 2) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Num of Input Parameters Wrong\n"));
		return;
	}

	for (i = 0, macptr = rstrtok(arg, "-"); macptr; macptr = rstrtok(NULL, "-"), i++) {
		if (strlen(macptr) <= sizeof(InputStr))
			NdisMoveMemory(InputStr, macptr, strlen(macptr));
		GroupIdx = (UINT16)os_str_tol(macptr, 0, 10);
	}

	if (GroupIdx >= 512) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Group Entry Idx is Wrong\n"));
		return;
	}
	show_muru_mum_group_tbl_entry(pAd, GroupIdx);
}

static VOID show_candidate_list(struct _RTMP_ADAPTER *pAd, UINT_8 counter, UINT32 addr)
{
	UINT_8 i, j, value, bit;
	UINT_32 addr_3, addr_4;
	RTMP_STRING * group[3] = {" 8: ", "36: ", "71: "};
	UINT_8 u1CnBits, u1MaxBits;

	for (i = 0; i < counter; i++) {
		addr_3 = addr + sizeof(UINT_8) * i * MAX_CAP_MUM_GRP_BLOCK;

		if (i == MUM_GRP_CN_2)
			u1CnBits = MAX_CAP_MUM_GRP_BLOCK - 1;
		else if (i == MUM_GRP_CN_3)
			u1CnBits = MAX_CAP_MUM_GRP_BLOCK;
		else if (i == MUM_GRP_CN_4)
			u1CnBits = 1;

		u1MaxBits = 8;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s", group[i]));

		for (j = 0; j < u1CnBits; j++) {
			addr_4 = addr_3 + sizeof(UINT_8) * j;
			value = muru_io_r_u8(pAd, addr_4);

			if ((i == MUM_GRP_CN_2) && (j == u1CnBits - 1))
				u1MaxBits = 4;
			else if ((i == MUM_GRP_CN_3) && (j == u1CnBits - 1))
				u1MaxBits = 3;
			else if ((i == MUM_GRP_CN_4) && (j == u1CnBits - 1))
				u1MaxBits = 5;

			for (bit = 0; bit < u1MaxBits; bit++) {
				if ((value & BIT(bit)) != 0) {
					if (i == 0)
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" %d, ", 1));
					else if (i == 1)
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m %d\x1b[0m, ", 1));
					else if (i == 2)
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[32m %d\x1b[0m, ", 1));
				} else {
					if (i == 0)
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" %d, ", 0));
					else if (i == 1)
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m %d\x1b[0m, ", 0));
					else if (i == 2)
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[32m %d\x1b[0m, ", 0));
				}
			}
		}
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}
}

static VOID show_mumimo_algorithm_monitor1(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT_32 base, base_starec, offset, offset_starec, subbase_starec;
	UINT_8 i, j, k, l = 0;
	BOOLEAN err, err_starec, err_mu_tx;
	UINT_32 addr, addr_, addr1, addr_2, addr_starec;
	UINT_32 u4SuccessCount[MURU_MAX_GROUP_CN][MURU_MUM_MAX_PFID_NUM];
	UINT_32 u4TotalCount[MURU_MAX_GROUP_CN][MURU_MUM_MAX_PFID_NUM];
	UINT_32 base_mu_tx, subbase;
	UINT_8  PfiIdx[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	UINT_16 Muru_mum_usr_mgmt[RAM_BAND_NUM][MURU_MUM_MAX_PFID_NUM];
	RTMP_STRING *group_cap[5] = {"VHT_CAP", "HE_DLFBMUM_CAP", "HE_DLPBMUM_CAP", "HE_ULFBMUM_CAP", "HE_ULPBMUM_CAP"};
	UINT_16 WlanIdx[8];
	UINT_8  MumCapBitmap[MURU_MUM_MAX_PFID_NUM] = {0};
	UINT_8 counter;

	base = pAd->CommonCfg.rGloInfo.rMuruMumCtrl.u4Addr;
	err  = pAd->CommonCfg.rGloInfo.rMuruMumCtrl.fgError;

	base_starec = pAd->CommonCfg.rGloInfo.rShareDataStaRuRecord.u4Addr;
	err_starec = pAd->CommonCfg.rGloInfo.rShareDataStaRuRecord.fgError;

	base_mu_tx = pAd->CommonCfg.rGloInfo.rMuruTxStatInfo.u4Addr;
	err_mu_tx = pAd->CommonCfg.rGloInfo.rMuruTxStatInfo.fgError;

	SyncMuruSramCheckAddr(pAd, base);
	SyncMuruSramCheckAddr(pAd, base_starec);
	SyncMuruSramCheckAddr(pAd, base_mu_tx);

	counter = MUM_GRP_USR_NUM_4;
	memset(Muru_mum_usr_mgmt, 0, sizeof(UINT_16) * RAM_BAND_NUM * MURU_MUM_MAX_PFID_NUM);
	memset(u4SuccessCount, 0, sizeof(UINT_32) * MURU_MAX_GROUP_CN * MURU_MUM_MAX_PFID_NUM);
	memset(u4TotalCount, 0, sizeof(UINT_32) * MURU_MAX_GROUP_CN * MURU_MUM_MAX_PFID_NUM);

	if (err)
		return;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("|-(0x%08X)MURU MU ALGORITHM MONITOR:\n", base));

	offset = OFFSET_OF(MURU_MUM_CTRL_PARA_T, au2MuProfileIdxToWlanIdx);
	addr_ = base + offset;

	for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
		addr = addr_ + sizeof(UINT_16) * i;
		WlanIdx[i] = muru_io_r_u16(pAd, addr);
		if (WlanIdx[i] > STA_REC_NUM)
			WlanIdx[i] = 0;
	}

	offset = OFFSET_OF(MURU_MUM_CTRL_PARA_T, arMuUserMgmt);
	addr_ = base + offset;

	for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++) {
		addr_2 = addr_ + sizeof(UINT_16) * j * RAM_BAND_NUM;
		if (WlanIdx[j] != 0) {
			for (i = 0; i < RAM_BAND_NUM; i++) {
				addr = addr_2 + sizeof(UINT_16) * i;
				Muru_mum_usr_mgmt[i][j] = muru_io_r_u16(pAd, addr);
			}
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMURU MUM PFID IDX \n\t"));
	for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
		if (i == MURU_MUM_MAX_PFID_NUM - 1)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d", PfiIdx[i]));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d,", PfiIdx[i]));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tWLAN IDX\n"));

	for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
		if (i == MURU_MUM_MAX_PFID_NUM - 1)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d", WlanIdx[i]));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d,", WlanIdx[i]));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n MURU MUM USER GROUP COUNT \n"));

	for (i = 0; i < RAM_BAND_NUM; i++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBAND%d USER GROUP COUNT\n\t", i));

		for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++) {
			if (j == MURU_MUM_MAX_PFID_NUM - 1)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d", Muru_mum_usr_mgmt[i][j]));
			else
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d,", Muru_mum_usr_mgmt[i][j]));
		}
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\n"));
	}

	if (!err_starec) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PFID  VHT_CAP  HE_DLFBMUM_CAP  HE_DLPBMUM_CAP  HE_ULFBMUM_CAP  HE_ULPBMUM_CAP\n"));
		for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
			if (WlanIdx[i] != 0) {
				subbase_starec = base_starec + sizeof(STA_MURU_RECORD_T) * WlanIdx[i];

				offset_starec = OFFSET_OF(STA_MURU_RECORD_T, u1MumCapBitmap);
				addr_starec = subbase_starec + offset_starec;
				MumCapBitmap[i] = muru_io_r_u8(pAd, addr_starec);

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d", i));

				for (j = 0; j < MU_MAX_GRP_USR_CAP; j++) {
					if ((MumCapBitmap[i] & BIT(j)) != 0)
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d", 1));
					else
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%d", 0));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" \t"));
				}
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
			}
		}
	}

	offset = OFFSET_OF(MURU_MUM_CTRL_PARA_T, arB0PfidGrpBitmap);
	addr_ = base + offset;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	if (pAd->CommonCfg.dbdc_mode != 0)
		counter = MUM_GRP_USR_NUM_2;

	for (k = 0; k < MURU_MUM_MAX_PFID_NUM; k++) {
		addr_2 = addr_ + sizeof(UINT_8) * k * l * MAX_CAP_MUM_GRP_BLOCK * MUM_GRP_USR_NUM_4;
		if (WlanIdx[k] == 0)
			continue;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMURU MUM PFID IDX : %x\n", k));

		for (l = 0; l < MU_MAX_GRP_USR_CAP; l++) {
			if ((MumCapBitmap[k] & BIT(l)) != 0) {
				addr = addr_2 + sizeof(UINT_8) * l * MAX_CAP_MUM_GRP_BLOCK * MUM_GRP_USR_NUM_4;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%s:\n", group_cap[l]));

				show_candidate_list(pAd, counter, addr);

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
			}
		}
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}

	if (pAd->CommonCfg.dbdc_mode == 1) {
		offset = OFFSET_OF(MURU_MUM_CTRL_PARA_T, arB1PfidGrpBitmap);
		addr_ = base + offset;

		for (k = 0; k < MURU_MUM_MAX_PFID_NUM; k++) {
			addr_2 = addr_ + sizeof(UINT_8) * k * l * MAX_CAP_MUM_GRP_BLOCK * MUM_GRP_USR_NUM_4;
			if (WlanIdx[k] == 0)
				continue;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tMURU MUM PFID IDX : %x\n", k));

			for (l = 0; l < MU_MAX_GRP_USR_CAP; l++) {
				if ((MumCapBitmap[k] & BIT(l)) != 0) {
					addr = addr_2 + sizeof(UINT_8) * l * MAX_CAP_MUM_GRP_BLOCK * MUM_GRP_USR_NUM_4;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t%s:\n", group_cap[l]));

					show_candidate_list(pAd, counter, addr);

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
				}
			}
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\n"));
	}

	if (err_mu_tx)
		return;

	subbase = base_mu_tx;

	offset = OFFSET_OF(MU_TX_STAT_INFO_T, u4SuccessCnt);
	addr_ = subbase + offset;

	for (j = 0; j < MURU_MAX_GROUP_CN; j++) {
		addr1 = addr_ + j * sizeof(UINT_32) * MURU_MUM_MAX_PFID_NUM;
		for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
			addr = addr1 + i * sizeof(UINT_32);
			u4SuccessCount[j][i] = muru_io_r_u32(pAd, addr);
		}
	}

	offset = OFFSET_OF(MU_TX_STAT_INFO_T, u4TotalCnt);
	addr_ = subbase + offset;

	for (j = 0; j < MURU_MAX_GROUP_CN; j++) {
		addr1 = addr_ + j * sizeof(UINT_32) * MURU_MUM_MAX_PFID_NUM;
		for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
			addr = addr1 + i * sizeof(UINT_32);
			u4TotalCount[j][i] = muru_io_r_u32(pAd, addr);
		}
	}

	for (i = 0; i < MURU_MAX_GROUP_CN; i++)	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n \n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("CN%u DL TX SUCCESS CNT : \n", i+2));

		for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++)	{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t %u", u4SuccessCount[i][j]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\nCN%u DL TX TOTAL CNT : \n", i+2));

		for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++)	{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t %u", u4TotalCount[i][j]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\nCN%u DL TX PER : \n", i+2));

		for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++)	{
			if (u4TotalCount[i][j] == 0)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t %u", u4TotalCount[i][j]));
			else
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t %u", ((u4TotalCount[i][j] - u4SuccessCount[i][j]) * 100) / u4TotalCount[i][j]));
		}
	}

	subbase = base_mu_tx + sizeof(MU_TX_STAT_INFO_T);

	offset = OFFSET_OF(MU_TX_STAT_INFO_T, u4SuccessCnt);
	addr_ = subbase + offset;

	for (j = 0; j < MURU_MAX_GROUP_CN; j++) {
		addr1 = addr_ + j * sizeof(UINT_32) * MURU_MUM_MAX_PFID_NUM;
		for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
			addr = addr1 + i * sizeof(UINT_32);
			u4SuccessCount[j][i] = muru_io_r_u32(pAd, addr);
		}
	}

	offset = OFFSET_OF(MU_TX_STAT_INFO_T, u4TotalCnt);
	addr_ = subbase + offset;

	for (j = 0; j < MURU_MAX_GROUP_CN; j++)	{
		addr1 = addr_ + j * sizeof(UINT_32) * MURU_MUM_MAX_PFID_NUM;
		for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++)	{
			addr = addr1 + i * sizeof(UINT_32);
			u4TotalCount[j][i] = muru_io_r_u32(pAd, addr);
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	for (i = 0; i < MURU_MAX_GROUP_CN; i++)	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n \n"));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("CN%u UL TX SUCCESS CNT : \n", i+2));

		for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++)	{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t %u", u4SuccessCount[i][j]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\nCN%u UL TX TOTAL CNT : \n", i+2));

		for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\t %u", u4TotalCount[i][j]));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\nCN%u UL TX PER : \n", i+2));

		for (j = 0; j < MURU_MUM_MAX_PFID_NUM; j++) {
			if (u4TotalCount[i][j] == 0)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t %u", u4TotalCount[i][j]));
			else
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\t %u", ((u4TotalCount[i][j] - u4SuccessCount[i][j]) * 100) / u4TotalCount[i][j]));
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\n"));
}

static VOID show_mumimo_groupidcli(struct _RTMP_ADAPTER *pAd)
{
	UINT_32 base, base_grptbl, subbase_grptbl, offset, base1;
	UINT_8 u1NumUsr, u1Gid = 0, i;
	BOOLEAN err, err_grp_tbl, err1;
	UINT_32 addr, addr1, addr2;
	UINT_16 WlanIdx[8], u2HwGrpIdx;
	UINT_32 DW0_Value = 0, DW1_Value = 0;
	P_MURU_MUM_GROUP_TBL_ENTRY_DW0 pDW0;
	P_MURU_MUM_GROUP_TBL_ENTRY_DW1 pDW1;

	base = pAd->CommonCfg.rGloInfo.rMuruMumCtrl.u4Addr;
	err  = pAd->CommonCfg.rGloInfo.rMuruMumCtrl.fgError;
	base_grptbl = pAd->CommonCfg.rGloInfo.rMuruMumGrpTable.u4Addr;
	err_grp_tbl = pAd->CommonCfg.rGloInfo.rMuruMumGrpTable.fgError;
	base1 = pAd->CommonCfg.rGloInfo.rCn4GidLookupTable.u4Addr;
	err1  = pAd->CommonCfg.rGloInfo.rCn4GidLookupTable.fgError;

	SyncMuruSramCheckAddr(pAd, base);
	SyncMuruSramCheckAddr(pAd, base_grptbl);
	SyncMuruSramCheckAddr(pAd, base1);

	if (err || err_grp_tbl || err1)
		return;
	offset = OFFSET_OF(MURU_MUM_CTRL_PARA_T, au2MuProfileIdxToWlanIdx);
	addr = base + offset;

	for (i = 0; i < MURU_MUM_MAX_PFID_NUM; i++) {
		addr1 = addr + sizeof(UINT_16) * i;
		WlanIdx[i] = muru_io_r_u16(pAd, addr);
		if (WlanIdx[i] > STA_REC_NUM)
			WlanIdx[i] = 0;
	}

	offset = OFFSET_OF(MURU_MUM_CTRL_PARA_T, u2LatestMuTxGrpIdx);
	addr = base + offset;
	u2HwGrpIdx = muru_io_r_u16(pAd, addr);

	subbase_grptbl = base_grptbl + sizeof(MURU_MUM_GROUP_TBL_ENTRY_T) * u2HwGrpIdx;
	SyncMuruSramCheckAddr(pAd, subbase_grptbl);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("|-(0x%08X)MURU MU ALGORITHM MONITOR:\n", base));

	addr = subbase_grptbl;
	DW0_Value = muru_io_r_u32(pAd, addr);
	addr1 = subbase_grptbl + 4;
	DW1_Value = muru_io_r_u32(pAd, addr1);

	pDW0 = (P_MURU_MUM_GROUP_TBL_ENTRY_DW0)&DW0_Value;
	pDW1 = (P_MURU_MUM_GROUP_TBL_ENTRY_DW1)&DW1_Value;

	u1NumUsr = pDW0->rField.u1NumUser;

	if (u1NumUsr != 3)
		u1Gid = pDW0->rField.u1PFIDUser1;
	else {
		for (i = 0; i < MUM_VHT_4MU_GRP_NUM; i++) {
			addr2  = base1 + sizeof(UINT_8) * i * 6;

			if (muru_io_r_u8(pAd, addr2) == pDW0->rField.u1PFIDUser0 &&
				muru_io_r_u8(pAd, (addr2 + 1)) == pDW0->rField.u1PFIDUser1 &&
				muru_io_r_u8(pAd, (addr2 + 2)) == pDW0->rField.u1PFIDUser2 &&
				muru_io_r_u8(pAd, (addr2 + 3)) == pDW0->rField.u1PFIDUser3) {
					u1Gid = muru_io_r_u8(pAd, (addr2 + 4));
			    }
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n GroupID = %d", u1Gid));
	if (u1NumUsr >= 0)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("| |-u1PFIDUser0  = %u WlanIdx = %u Mustream = %u\n", pDW0->rField.u1PFIDUser0, WlanIdx[pDW0->rField.u1PFIDUser0], pDW1->rField.u1NssUser0 + 1));
	if (u1NumUsr >= 1)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("| |-u1PFIDUser1  = %u WlanIdx = %u Mustream = %u\n", pDW0->rField.u1PFIDUser1, WlanIdx[pDW0->rField.u1PFIDUser1], pDW1->rField.u1NssUser1 + 1));
	if (u1NumUsr >= 2)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("| |-u1PFIDUser2  = %u WlanIdx = %u Mustream = %u\n", pDW0->rField.u1PFIDUser2, WlanIdx[pDW0->rField.u1PFIDUser2], pDW1->rField.u1NssUser2 + 1));
	if (u1NumUsr == 3)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("| |-u1PFIDUser3  = %u WlanIdx = %u Mustream = %u\n", pDW0->rField.u1PFIDUser3, WlanIdx[pDW0->rField.u1PFIDUser3], pDW1->rField.u1NssUser3 + 1));
}

static VOID show_mumimo_algorithm_monitor(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;

	pch = strsep(&arg, "");

	if (pch == NULL)
		return;

	switch (*pch) {
	case '1':
			show_mumimo_algorithm_monitor1(pAd, arg);
			break;

	case '4':
			show_mumimo_groupidcli(pAd);
			break;

	default:
			break;
	}
}

static VOID show_muru_txc_tx_stats(struct _RTMP_ADAPTER *pAd, VOID *pData)
{
	P_EVENT_MURU_TXCMD_TX_STATS pTxCTxStats = (P_EVENT_MURU_TXCMD_TX_STATS)pData;
	P_MURU_TXCMD_DL_TX_STATS pDlTxStats;
	P_MURU_TXCMD_UL_TX_TRIG_STATS pTxTrigUlStats;
	UINT32 u4TotalVhtCount, u4TotalVhtMuCount;
	UINT32 u4TotalHeMuCount, u4TotalHeMuTrigCount;
	UINT32 u4HeDlOfdm2to4RuCount, u4HeDlOfdmGtr5RuCount;
	UINT32 u4TotalHeDlOfdmCount, u4TotalDlHeMuCount;
	UINT32 u4HeUlOfdm2to4RuCount, u4HeUlOfdmGtr5RuCount;
	UINT32 u4TotalHeTrigCount, u4TotalHeUlOfdmCount, u4TotalPpduCount, u4TotalTrigPpduCount;
	UINT16 u2TxModeCck, u2TxModeOfdm, u2TxModeHtMix, u2TxModeHeGf;
	UINT16 u2TxModeVht, u2TxModeHeSu, u2TxModeHeExt, u2TxModeHeMu;
	UINT16 u2StaVht2Mu, u2StaVht3Mu, u2StaVht4Mu, u2StaHe2Mu, u2StaHe3Mu, u2StaHe4Mu;
	UINT16 u2SubModeVhtSuCnt, u2SubModeVhtMuMimoCnt, u2SubModeHeOfdmCnt, u2SubModeHeMumimoCnt;
	UINT16 u2StaOfdm2Ru, u2StaOfdm3Ru, u2StaOfdm4Ru, u2StaOfdm5to8Ru, u2StaOfdm9to16Ru, u2StaOfdmGtr16Ru;
	UINT16 u2TrigOfdmSu, u2TrigOfdm2Ru, u2TrigOfdm3Ru, u2TrigOfdm4Ru, u2TrigOfdm5to8Ru, u2TrigOfdm9to16Ru, u2TrigOfdmGtr16Ru;
	UINT16 u2Trig2MuCnt, u2Trig3MuCnt, u2Trig4MuCnt, u2HeTrigCount, u2HeTrigMuCnt, u2HeTrigOfdmCnt;

	pDlTxStats = (P_MURU_TXCMD_DL_TX_STATS)(&(pTxCTxStats->EventTxDlStats));
	pTxTrigUlStats = (P_MURU_TXCMD_UL_TX_TRIG_STATS)(&(pTxCTxStats->EventTxTrigUlStats));

	u4TotalVhtMuCount = pDlTxStats->u4TxCmdTxModeVht2MuCnt + pDlTxStats->u4TxCmdTxModeVht3MuCnt + pDlTxStats->u4TxCmdTxModeVht4MuCnt;
	u4TotalVhtCount = u4TotalVhtMuCount + pDlTxStats->u4TxCmdTxModeVhtSuCnt;
	u4TotalHeMuCount = pDlTxStats->u4TxCmdTxModeHeMu2MuCnt + pDlTxStats->u4TxCmdTxModeHeMu3MuCnt + pDlTxStats->u4TxCmdTxModeHeMu4MuCnt;
	u4TotalHeMuTrigCount = pTxTrigUlStats->u4TxCmdTxModeHeTrig2MuCnt + pTxTrigUlStats->u4TxCmdTxModeHeTrig3MuCnt + pTxTrigUlStats->u4TxCmdTxModeHeTrig4MuCnt;

	u4HeDlOfdm2to4RuCount = pDlTxStats->u4TxCmdTxModeHeMu2RuCnt + pDlTxStats->u4TxCmdTxModeHeMu3RuCnt + pDlTxStats->u4TxCmdTxModeHeMu4RuCnt;
	u4HeDlOfdmGtr5RuCount = pDlTxStats->u4TxCmdTxModeHeMu5to8RuCnt + pDlTxStats->u4TxCmdTxModeHeMu9to16RuCnt + pDlTxStats->u4TxCmdTxModeHeMuGtr16RuCnt;
	u4TotalHeDlOfdmCount = u4HeDlOfdm2to4RuCount + u4HeDlOfdmGtr5RuCount;
	u4TotalDlHeMuCount = u4TotalHeDlOfdmCount + u4TotalHeMuCount;

	u4HeUlOfdm2to4RuCount = pTxTrigUlStats->u4TxCmdTxModeHeTrig2RuCnt + pTxTrigUlStats->u4TxCmdTxModeHeTrig3RuCnt + pTxTrigUlStats->u4TxCmdTxModeHeTrig4RuCnt;
	u4HeUlOfdmGtr5RuCount = pTxTrigUlStats->u4TxCmdTxModeHeTrig5to8RuCnt + pTxTrigUlStats->u4TxCmdTxModeHeTrig9to16RuCnt + pTxTrigUlStats->u4TxCmdTxModeHeTrigGtr16RuCnt;
	u4TotalHeUlOfdmCount = u4HeUlOfdm2to4RuCount + u4HeUlOfdmGtr5RuCount + pTxTrigUlStats->u4TxCmdTxModeHeTrigSuCnt;
	u4TotalHeTrigCount = u4TotalHeUlOfdmCount + u4TotalHeMuTrigCount;


	u4TotalPpduCount = u4TotalVhtCount + pDlTxStats->u4TxCmdTxModeCckCnt + pDlTxStats->u4TxCmdTxModeOfdmCnt + pDlTxStats->u4TxCmdTxModeHtMmCnt;
	u4TotalPpduCount = u4TotalPpduCount + pDlTxStats->u4TxCmdTxModeHtGfCnt + pDlTxStats->u4TxCmdTxModeHeSuCnt + pDlTxStats->u4TxCmdTxModeHeExtSuCnt;
	u4TotalPpduCount = u4TotalPpduCount + u4TotalDlHeMuCount;

	if (u4TotalPpduCount != 0) {

		u2TxModeCck = ((pDlTxStats->u4TxCmdTxModeCckCnt * 100) / u4TotalPpduCount);
		u2TxModeOfdm = ((pDlTxStats->u4TxCmdTxModeOfdmCnt * 100) / u4TotalPpduCount);
		u2TxModeHtMix = ((pDlTxStats->u4TxCmdTxModeHtMmCnt * 100) / u4TotalPpduCount);
		u2TxModeHeGf = ((pDlTxStats->u4TxCmdTxModeHtGfCnt * 100) / u4TotalPpduCount);
		u2TxModeVht = ((u4TotalVhtCount * 100) / u4TotalPpduCount);
		u2TxModeHeSu = ((pDlTxStats->u4TxCmdTxModeHeSuCnt * 100) / u4TotalPpduCount);
		u2TxModeHeExt = ((pDlTxStats->u4TxCmdTxModeHeExtSuCnt * 100) / u4TotalPpduCount);
		u2TxModeHeMu = ((u4TotalDlHeMuCount * 100) / u4TotalPpduCount);

		u2SubModeVhtSuCnt = ((pDlTxStats->u4TxCmdTxModeVhtSuCnt * 100) / u4TotalPpduCount);
		u2SubModeVhtMuMimoCnt = ((u4TotalVhtMuCount * 100) / u4TotalPpduCount);
		u2SubModeHeOfdmCnt = ((u4TotalHeDlOfdmCount * 100) / u4TotalPpduCount);
		u2SubModeHeMumimoCnt = ((u4TotalHeMuCount * 100) / u4TotalPpduCount);

		u2StaVht2Mu = ((pDlTxStats->u4TxCmdTxModeVht2MuCnt * 100) / u4TotalPpduCount);
		u2StaVht3Mu = ((pDlTxStats->u4TxCmdTxModeVht3MuCnt * 100) / u4TotalPpduCount);
		u2StaVht4Mu = ((pDlTxStats->u4TxCmdTxModeVht4MuCnt * 100) / u4TotalPpduCount);

		u2StaHe2Mu = ((pDlTxStats->u4TxCmdTxModeHeMu2MuCnt * 100) / u4TotalPpduCount);
		u2StaHe3Mu = ((pDlTxStats->u4TxCmdTxModeHeMu3MuCnt * 100) / u4TotalPpduCount);
		u2StaHe4Mu = ((pDlTxStats->u4TxCmdTxModeHeMu4MuCnt * 100) / u4TotalPpduCount);

		u2StaOfdm2Ru = ((pDlTxStats->u4TxCmdTxModeHeMu2RuCnt * 100) / u4TotalPpduCount);
		u2StaOfdm3Ru = ((pDlTxStats->u4TxCmdTxModeHeMu3RuCnt * 100) / u4TotalPpduCount);
		u2StaOfdm4Ru = ((pDlTxStats->u4TxCmdTxModeHeMu4RuCnt * 100) / u4TotalPpduCount);
		u2StaOfdm5to8Ru = ((pDlTxStats->u4TxCmdTxModeHeMu5to8RuCnt * 100) / u4TotalPpduCount);
		u2StaOfdm9to16Ru = ((pDlTxStats->u4TxCmdTxModeHeMu9to16RuCnt * 100) / u4TotalPpduCount);
		u2StaOfdmGtr16Ru = ((pDlTxStats->u4TxCmdTxModeHeMuGtr16RuCnt * 100) / u4TotalPpduCount);

	}

	else {
		u2TxModeCck = 0;
		u2TxModeOfdm = 0;
		u2TxModeHtMix = 0;
		u2TxModeHeGf = 0;
		u2TxModeVht = 0;
		u2TxModeHeSu = 0;
		u2TxModeHeExt = 0;
		u2TxModeHeMu = 0;

		u2SubModeVhtSuCnt = 0;
		u2SubModeVhtMuMimoCnt = 0;
		u2SubModeHeOfdmCnt = 0;
		u2SubModeHeMumimoCnt = 0;

		u2StaVht2Mu = 0;
		u2StaVht3Mu = 0;
		u2StaVht4Mu = 0;

		u2StaHe2Mu = 0;
		u2StaHe3Mu = 0;
		u2StaHe4Mu = 0;

		u2StaOfdm2Ru = 0;
		u2StaOfdm3Ru = 0;
		u2StaOfdm4Ru = 0;
		u2StaOfdm5to8Ru = 0;
		u2StaOfdm9to16Ru = 0;
		u2StaOfdmGtr16Ru = 0;
	}

	u4TotalTrigPpduCount = u4TotalHeTrigCount;

	if (u4TotalTrigPpduCount != 0) {
		u2TrigOfdmSu = ((pTxTrigUlStats->u4TxCmdTxModeHeTrigSuCnt * 100) / u4TotalTrigPpduCount);
		u2TrigOfdm2Ru = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig2RuCnt * 100) / u4TotalTrigPpduCount);
		u2TrigOfdm3Ru = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig3RuCnt * 100) / u4TotalTrigPpduCount);
		u2TrigOfdm4Ru = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig4RuCnt * 100) / u4TotalTrigPpduCount);
		u2TrigOfdm5to8Ru = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig5to8RuCnt * 100) / u4TotalTrigPpduCount);
		u2TrigOfdm9to16Ru = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig9to16RuCnt * 100) / u4TotalTrigPpduCount);
		u2TrigOfdmGtr16Ru = ((pTxTrigUlStats->u4TxCmdTxModeHeTrigGtr16RuCnt * 100) / u4TotalTrigPpduCount);

		u2Trig2MuCnt = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig2MuCnt * 100) / u4TotalTrigPpduCount);
		u2Trig3MuCnt = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig3MuCnt * 100) / u4TotalTrigPpduCount);
		u2Trig4MuCnt = ((pTxTrigUlStats->u4TxCmdTxModeHeTrig4MuCnt * 100) / u4TotalTrigPpduCount);

		u2HeTrigCount = ((u4TotalHeTrigCount * 100) / u4TotalTrigPpduCount);
		u2HeTrigOfdmCnt = ((u4TotalHeUlOfdmCount * 100) / u4TotalTrigPpduCount);
		u2HeTrigMuCnt = ((u4TotalHeMuTrigCount * 100) / u4TotalTrigPpduCount);
	}

	else {
		u2TrigOfdmSu = 0;
		u2TrigOfdm2Ru = 0;
		u2TrigOfdm3Ru = 0;
		u2TrigOfdm4Ru = 0;
		u2TrigOfdm5to8Ru = 0;
		u2TrigOfdm9to16Ru = 0;
		u2TrigOfdmGtr16Ru = 0;

		u2Trig2MuCnt = 0;
		u2Trig3MuCnt = 0;
		u2Trig4MuCnt = 0;

		u2HeTrigCount = 0;
		u2HeTrigOfdmCnt = 0;
		u2HeTrigMuCnt = 0;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Downlink:\ttotal\t\ttx_mode\t\tsub_mode\tstacnt\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("         \tcount\t\tratio  \t\tratio   \tratio\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=====================================================================\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CCK:        \t%u   \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeCckCnt, u2TxModeCck));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("OFDM:       \t%u   \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeOfdmCnt, u2TxModeOfdm));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HT_MIX:     \t%u   \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHtMmCnt, u2TxModeHtMix));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HT_GF:      \t%u   \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHtGfCnt, u2TxModeHeGf));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("VHT:        \t%u   \t\t%u%%\n", u4TotalVhtCount, u2TxModeVht));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  SU:       \t%u   \t         \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeVhtSuCnt, u2SubModeVhtSuCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  MUMIMO:   \t%u   \t         \t\t%u%%\n", u4TotalVhtMuCount, u2SubModeVhtMuMimoCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	   2MU:   \t%u \t         \t\t        \t%u%%\n", pDlTxStats->u4TxCmdTxModeVht2MuCnt, u2StaVht2Mu));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	   3MU:      \t%u\t        \t\t        \t%u%%\n", pDlTxStats->u4TxCmdTxModeVht3MuCnt, u2StaVht3Mu));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	   4MU:        \t%u        \t\t        \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeVht4MuCnt, u2StaVht4Mu));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HE_SU:      \t%u   \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeSuCnt, u2TxModeHeSu));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HE_EXT:     \t%u   \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeExtSuCnt, u2TxModeHeExt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HE_MU:      \t%u   \t\t%u%%\n", u4TotalDlHeMuCount, u2TxModeHeMu));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  OFDMA:    \t%u   \t         \t\t%u%%\n", u4TotalHeDlOfdmCount, u4TotalHeMuCount));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("      2RU:  \t%u   \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu2RuCnt, u2StaOfdm2Ru));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("      3RU:  \t%u   \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu3RuCnt, u2StaOfdm3Ru));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("      4RU:  \t%u   \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu4RuCnt, u2StaOfdm4Ru));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("    5-8RU:  \t%u   \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu5to8RuCnt, u2StaOfdm5to8Ru));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   9-16RU:  \t%u   \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu9to16RuCnt, u2StaOfdm9to16Ru));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("    >16RU:  \t%u   \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMuGtr16RuCnt, u2StaOfdmGtr16Ru));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" MUMIMO:    \t%u   \t         \t\t%u%%\n", u4TotalHeMuCount, u2SubModeHeMumimoCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	   2MU:   \t%u \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu2MuCnt, u2StaHe2Mu));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	   3MU:   \t%u \t         \t\t       \t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu3MuCnt, u2StaHe3Mu));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	   4MU:        \t%u \t     \t\t       \t\t\t%u%%\n", pDlTxStats->u4TxCmdTxModeHeMu4MuCnt, u2StaHe4Mu));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));


	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Uplink:  \ttotal\t\ttx_mode\t\tsub_mode\tstacnt\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("         \tcount\t\tratio  \t\tratio   \tratio\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=====================================================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HE_TRIG     \t%u   \t\t%u%%\n", u4TotalHeTrigCount, u2HeTrigCount));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  OFDMA:    \t%u   \t         \t\t%u%%\n", u4TotalHeUlOfdmCount, u2HeTrigOfdmCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("       SU:  \t%u   \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrigSuCnt, u2TrigOfdmSu));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("      2RU:  \t%u   \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig2RuCnt, u2TrigOfdm2Ru));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("      3RU:  \t%u   \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig3RuCnt, u2TrigOfdm3Ru));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("      4RU:  \t%u   \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig4RuCnt, u2TrigOfdm4Ru));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("    5-8RU:  \t%u   \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig5to8RuCnt, u2TrigOfdm5to8Ru));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   9-16RU:  \t%u   \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig9to16RuCnt, u2TrigOfdm9to16Ru));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("    >16RU:  \t%u   \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrigGtr16RuCnt, u2TrigOfdmGtr16Ru));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" MUMIMO:    \t%u   \t         \t\t%u%%\n", u4TotalHeMuTrigCount, u2HeTrigMuCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	   2MU:   \t%u \t         \t\t        \t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig2MuCnt, u2Trig2MuCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	   3MU:      \t%u \t       \t\t        \t\t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig3MuCnt, u2Trig3MuCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	   4MU:        \t%u \t     \t\t        \t\t%u%%\n", pTxTrigUlStats->u4TxCmdTxModeHeTrig4MuCnt, u2Trig4MuCnt));
}

static VOID check_muru_glo(struct _RTMP_ADAPTER *pAd, VOID *pData)
{
	P_DRV_MURU_GLO pDrvGlo = &pAd->CommonCfg.rGloInfo;
	P_EVENT_MURU_GLO pFwGlo = (P_EVENT_MURU_GLO)pData;
	UINT_32 DriverSize = 0;

	pDrvGlo->rLocalData.u4Addr = pFwGlo->rLocalData.u4Addr;
	pDrvGlo->rLocalData.fgError = (pFwGlo->rLocalData.u4Size != sizeof(MURU_LOCAL_DATA_T))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MURU_LOCAL_DATA_T"));
	if (pDrvGlo->rLocalData.fgError) {
		DriverSize = sizeof(MURU_LOCAL_DATA_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rLocalData.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rLocalDataMuruPara.u4Addr = pFwGlo->rLocalDataMuruPara.u4Addr;
	pDrvGlo->rLocalDataMuruPara.fgError = (pFwGlo->rLocalDataMuruPara.u4Size != sizeof(MURU_PARA_T))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-MURU_PARA_T"));
	if (pDrvGlo->rLocalDataMuruPara.fgError) {
		DriverSize = sizeof(MURU_PARA_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rLocalDataMuruPara.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rLocalDataQlenInfo.u4Addr = pFwGlo->rLocalDataQlenInfo.u4Addr;
	pDrvGlo->rLocalDataQlenInfo.fgError = (pFwGlo->rLocalDataQlenInfo.u4Size != sizeof(MURU_QLEN_INFO_T))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-MURU_QLEN_INFO_T"));
	if (pDrvGlo->rLocalDataQlenInfo.fgError) {
		DriverSize = sizeof(MURU_QLEN_INFO_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rLocalDataQlenInfo.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rLocalDataBsrpCtrl.u4Addr = pFwGlo->rLocalDataBsrpCtrl.u4Addr;
	pDrvGlo->rLocalDataBsrpCtrl.fgError = (pFwGlo->rLocalDataBsrpCtrl.u4Size != sizeof(CMD_MURU_BSRP_CTRL))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-CMD_MURU_BSRP_CTRL"));
	if (pDrvGlo->rLocalDataBsrpCtrl.fgError) {
		DriverSize = sizeof(CMD_MURU_BSRP_CTRL);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rLocalDataBsrpCtrl.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rLocalDataTxCmdCtrl.u4Addr = pFwGlo->rLocalDataTxCmdCtrl.u4Addr;
	pDrvGlo->rLocalDataTxCmdCtrl.fgError = (pFwGlo->rLocalDataTxCmdCtrl.u4Size != sizeof(MURU_TXCMD_CTRL_T))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-MURU_TXCMD_CTRL_T"));
	if (pDrvGlo->rLocalDataTxCmdCtrl.fgError) {
		DriverSize = sizeof(MURU_TXCMD_CTRL_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rLocalDataTxCmdCtrl.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rMuruTxInfo.u4Addr = pFwGlo->rMuruTxInfo.u4Addr;
	pDrvGlo->rMuruTxInfo.fgError = (pFwGlo->rMuruTxInfo.u4Size != sizeof(MURU_TX_INFO_T))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MURU_TX_INFO_T"));
	if (pDrvGlo->rMuruTxInfo.fgError) {
		DriverSize = sizeof(MURU_TX_INFO_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruTxInfo.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rMuruTxInfoGlobalData.u4Addr = pFwGlo->rMuruTxInfoGlobalData.u4Addr;
	pDrvGlo->rMuruTxInfoGlobalData.fgError = (pFwGlo->rMuruTxInfoGlobalData.u4Size != sizeof(MURU_GLOBAL_INFO_T))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-MURU_GLOBAL_INFO_T"));
	if (pDrvGlo->rMuruTxInfoGlobalData.fgError) {
		DriverSize = sizeof(MURU_GLOBAL_INFO_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruTxInfoGlobalData.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rMuruTxInfoProtectData.u4Addr = pFwGlo->rMuruTxInfoProtectData.u4Addr;
	pDrvGlo->rMuruTxInfoProtectData.fgError = (pFwGlo->rMuruTxInfoProtectData.u4Size != sizeof(MURU_PROTECT_INFO_T))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-MURU_PROTECT_INFO_T"));
	if (pDrvGlo->rMuruTxInfoProtectData.fgError) {
		DriverSize = sizeof(MURU_PROTECT_INFO_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruTxInfoProtectData.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rMuruTxInfoSxnTxData.u4Addr = pFwGlo->rMuruTxInfoSxnTxData.u4Addr;
	pDrvGlo->rMuruTxInfoSxnTxData.fgError = (pFwGlo->rMuruTxInfoSxnTxData.u4Size != sizeof(MURU_TX_DATA_T))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-MURU_TX_DATA_T"));
	if (pDrvGlo->rMuruTxInfoSxnTxData.fgError) {
		DriverSize = sizeof(MURU_TX_DATA_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruTxInfoSxnTxData.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rMuruTxInfoSxnTrigData.u4Addr = pFwGlo->rMuruTxInfoSxnTrigData.u4Addr;
	pDrvGlo->rMuruTxInfoSxnTrigData.fgError = (pFwGlo->rMuruTxInfoSxnTrigData.u4Size != sizeof(MURU_TX_TRIG_DATA_T))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-MURU_TX_TRIG_DATA_T"));
	if (pDrvGlo->rMuruTxInfoSxnTrigData.fgError) {
		DriverSize = sizeof(MURU_TX_TRIG_DATA_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruTxInfoSxnTrigData.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rShareData.u4Addr = pFwGlo->rShareData.u4Addr;
	pDrvGlo->rShareData.fgError = (pFwGlo->rShareData.u4Size != sizeof(MURU_SHARE_DATA_T))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MURU_SHARE_DATA_T"));
	if (pDrvGlo->rShareData.fgError) {
		DriverSize = sizeof(MURU_SHARE_DATA_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rShareData.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rShareDataRuAllocData.u4Addr = pFwGlo->rShareDataRuAllocData.u4Addr;
	pDrvGlo->rShareDataRuAllocData.fgError = (pFwGlo->rShareDataRuAllocData.u4Size != sizeof(MURU_ALLOC_DATA_INFO_T))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-MURU_ALLOC_DATA_INFO_T"));
	if (pDrvGlo->rShareDataRuAllocData.fgError) {
		DriverSize = sizeof(MURU_ALLOC_DATA_INFO_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rShareDataRuAllocData.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rShareDataUserInfo.u4Addr = pFwGlo->rShareDataUserInfo.u4Addr;
	pDrvGlo->rShareDataUserInfo.fgError = (pFwGlo->rShareDataUserInfo.u4Size != sizeof(PER_USER_INFO))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-PER_USER_INFO"));
	if (pDrvGlo->rShareDataUserInfo.fgError) {
		DriverSize = sizeof(PER_USER_INFO);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rShareDataUserInfo.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rShareDataStaRuRecord.u4Addr = pFwGlo->rShareDataStaRuRecord.u4Addr;
	pDrvGlo->rShareDataStaRuRecord.fgError = (pFwGlo->rShareDataStaRuRecord.u4Size != sizeof(STA_MURU_RECORD_T))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-STA_MURU_RECORD_T"));
	if (pDrvGlo->rShareDataStaRuRecord.fgError) {
		DriverSize = sizeof(STA_MURU_RECORD_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rShareDataStaRuRecord.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MURU_MAN_CFG_DATA\n"));
	pDrvGlo->rMuruExtCmdManCfgInf.u4Addr = pFwGlo->rMuruExtCmdManCfgInf.u4Addr;
	pDrvGlo->rMuruExtCmdManCfgInf.fgError = (pFwGlo->rMuruExtCmdManCfgInf.u4Size != sizeof(CMD_MURU_MANCFG_INTERFACER))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-CMD_MURU_MANCFG_INTERFACER"));
	if (pDrvGlo->rMuruExtCmdManCfgInf.fgError) {
		DriverSize = sizeof(CMD_MURU_MANCFG_INTERFACER);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruExtCmdManCfgInf.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rMuTxPktCnt.u4Addr = pFwGlo->rMuTxPktCnt.u4Addr;
	pDrvGlo->rMuTxPktCnt.fgError = (pFwGlo->rMuTxPktCnt.u4Size != sizeof(UINT_32))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rMuTxPktCnt"));
	if (pDrvGlo->rMuTxPktCnt.fgError) {
		DriverSize = sizeof(UINT_32);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuTxPktCnt.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rMuTxPktCntDwn.u4Addr = pFwGlo->rMuTxPktCntDwn.u4Addr;
	pDrvGlo->rMuTxPktCntDwn.fgError = (pFwGlo->rMuTxPktCntDwn.u4Size != sizeof(UINT_32))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rMuTxPktCntDwn"));
	if (pDrvGlo->rMuTxPktCntDwn.fgError) {
		DriverSize = sizeof(UINT_32);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuTxPktCntDwn.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rAggPolicy.u4Addr = pFwGlo->rAggPolicy.u4Addr;
	pDrvGlo->rAggPolicy.fgError = (pFwGlo->rAggPolicy.u4Size != sizeof(UINT_8))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rAggPolicy"));
	if (pDrvGlo->rAggPolicy.fgError) {
		DriverSize = sizeof(UINT_8);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rAggPolicy.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rDurationComp.u4Addr = pFwGlo->rDurationComp.u4Addr;
	pDrvGlo->rDurationComp.fgError = (pFwGlo->rDurationComp.u4Size != sizeof(UINT_8))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-rDurationComp"));
	if (pDrvGlo->rDurationComp.fgError) {
		DriverSize = sizeof(UINT_8);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rDurationComp.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MURU_MUMIMO_DATA\n"));
	pDrvGlo->rMuruMumGrpTable.u4Addr = pFwGlo->rMuruMumGrpTable.u4Addr;
	pDrvGlo->rMuruMumGrpTable.fgError = (pFwGlo->rMuruMumGrpTable.u4Size != sizeof(MURU_MUM_GROUP_TBL_ENTRY_T))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-MURU_MUM_GROUP_TBL_ENTRY"));
	if (pDrvGlo->rMuruMumGrpTable.fgError) {
		DriverSize = sizeof(MURU_MUM_GROUP_TBL_ENTRY_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruMumGrpTable.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rMuruMumCtrl.u4Addr = pFwGlo->rMuruMumCtrl.u4Addr;
	pDrvGlo->rMuruMumCtrl.fgError = (pFwGlo->rMuruMumCtrl.u4Size != sizeof(MURU_MUM_CTRL_PARA_T))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-MURU_MU_ALGORITHM_MONITOR"));
	if (pDrvGlo->rMuruMumCtrl.fgError) {
		DriverSize = sizeof(MURU_MUM_CTRL_PARA_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruMumCtrl.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rMuruStaCapInfo.u4Addr = pFwGlo->rMuruStaCapInfo.u4Addr;
	pDrvGlo->rMuruStaCapInfo.fgError = (pFwGlo->rMuruStaCapInfo.u4Size != sizeof(MURU_PURE_STACAP_INFO))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-MURU_STACAP_INFO"));
	if (pDrvGlo->rMuruStaCapInfo.fgError) {
		DriverSize = sizeof(MURU_PURE_STACAP_INFO);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruStaCapInfo.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rMuruTxStatInfo.u4Addr = pFwGlo->rMuruTxStatInfo.u4Addr;
	pDrvGlo->rMuruTxStatInfo.fgError = (pFwGlo->rMuruTxStatInfo.u4Size != sizeof(MU_TX_STAT_INFO_LINK_T))?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-MURU_TXSTAT_INFO"));
	if (pDrvGlo->rMuruTxStatInfo.fgError) {
		DriverSize = sizeof(MU_TX_STAT_INFO_LINK_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rMuruTxStatInfo.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

	pDrvGlo->rCn4GidLookupTable.u4Addr = pFwGlo->rCn4GidLookupTable.u4Addr;
	DriverSize = (MUM_VHT_4MU_GRP_NUM * 6);
	pDrvGlo->rCn4GidLookupTable.fgError = (pFwGlo->rCn4GidLookupTable.u4Size != DriverSize)?1:0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("|-MURU_TXSTAT_INFO"));
	if (pDrvGlo->rCn4GidLookupTable.fgError) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Drive Size = %d, FW Size = %d", DriverSize, pFwGlo->rCn4GidLookupTable.u4Size));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
}

static INT32 set_mumimo_fixed_rate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_MU_MIMO_CTRL;
	UINT16 subcmd = MU_MIMO_SET_FIXED_RATE;
	UINT16 value = 0;

	if (arg != NULL)
		value = os_str_tol(arg, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(subcmd) + sizeof(value));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	subcmd = cpu2le16(subcmd);
	value = cpu2le16(value);

#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&subcmd, sizeof(subcmd));
	AndesAppendCmdMsg(msg, (char *)&value, sizeof(value));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

static INT32 set_mumimo_fixed_group_rate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	INT32 Ret = TRUE;
	UINT_8 ucNNS_MCS = 0;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_MU_MIMO_CTRL;
	UINT32 subcmd = MU_MIMO_SET_FIXED_GROUP_RATE;
	CMD_MURU_MUM_SET_GROUP_TBL_ENTRY param = {0};

	pch = strsep(&arg, "-");
	/*Get NumUsr*/
	if (pch != NULL)
		param.u1NumUser = os_str_tol(pch, 0, 10) - 1;
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	/*Get Rualloc*/
	if (pch != NULL)
		param.u1RuAlloc = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	/*Get GuardInterval*/
	if (pch != NULL)
		param.u1GI = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	/*Get Capability*/
	if (pch != NULL)
		param.u1Capability = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}
	/*Get DL /UL*/
	pch = strsep(&arg, "-");

	if (pch != NULL)
		param.u1Dl_Ul = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	if (param.u1NumUser == 0) {
		pch = strsep(&arg, "-");

		if (pch != NULL)
			param.u2WlidUser0 = os_str_tol(pch, 0, 10);
		else {
			Ret = 0;
			goto error;
		}

		pch = strsep(&arg, "-");

		if (pch != NULL) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns0 = (ucNNS_MCS > 11);
			if (param.u1Dl_Ul != 1)
				param.u1DlMcsUser0 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
			else
				param.u1UlMcsUser0 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		if (param.u1Dl_Ul == 2) {
			UINT_8 ucNNS_MCS = 0;

			pch = strsep(&arg, "");
			if (pch == NULL) {
				Ret = 0;
				goto error;
			}
			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns0 = (ucNNS_MCS > 11);
			param.u1UlMcsUser0 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		}
	}

	if (param.u1NumUser >= 1) {
		pch = strsep(&arg, "-");

		if (pch != NULL) {
			param.u2WlidUser0 = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
			param.u2WlidUser0 = cpu2le16(param.u2WlidUser0);
#endif
		}
		else {
			Ret = 0;
			goto error;
		}

		pch = strsep(&arg, "-");

		if (pch != NULL) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns0 = (ucNNS_MCS > 11);
			if (param.u1Dl_Ul != 1)
				param.u1DlMcsUser0 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
			else
				param.u1UlMcsUser0 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		pch = strsep(&arg, "-");

		if ((param.u1Dl_Ul == 2) && (pch != NULL)) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns0 = (ucNNS_MCS > 11);
			param.u1UlMcsUser0 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
			pch = strsep(&arg, "-");
			} else if (pch == NULL) {
				Ret = 0;
				goto error;
			}

		if (pch != NULL) {
			param.u2WlidUser1 = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
			param.u2WlidUser1 = cpu2le16(param.u2WlidUser1);
#endif
		}
		else {
			Ret = 0;
			goto error;
		}

		if ((param.u1NumUser == 1) && (param.u1Dl_Ul != 2))
			pch = strsep(&arg, "");
		else
			pch = strsep(&arg, "-");

		if (pch != NULL) {
			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns1 = (ucNNS_MCS > 11);
			if (param.u1Dl_Ul != 1)
				param.u1DlMcsUser1 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
			else
				param.u1UlMcsUser1 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		if ((param.u1NumUser == 1) && (param.u1Dl_Ul == 2))
			pch = strsep(&arg, "");
		else if ((param.u1NumUser != 1) && (param.u1Dl_Ul == 2))
			pch = strsep(&arg, "-");

		if ((param.u1Dl_Ul == 2) && (pch != NULL)) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns1 = (ucNNS_MCS > 11);
			param.u1UlMcsUser1 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
			} else if (pch == NULL) {
				Ret = 0;
				goto error;
			}
	}

	if (param.u1NumUser >= 2) {
		pch = strsep(&arg, "-");

		if (pch != NULL) {
			param.u2WlidUser2 = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
			param.u2WlidUser2 = cpu2le16(param.u2WlidUser2);
#endif
		}
		else {
			Ret = 0;
			goto error;
		}

		if ((param.u1NumUser == 2) && (param.u1Dl_Ul != 2))
			pch = strsep(&arg, "");
		else
			pch = strsep(&arg, "-");

		if (pch != NULL) {
			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns2 = (ucNNS_MCS > 11);
			if (param.u1Dl_Ul != 1)
				param.u1DlMcsUser2 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
			else
				param.u1UlMcsUser2 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		if ((param.u1NumUser == 2) && (param.u1Dl_Ul == 2))
			pch = strsep(&arg, "");
		else if ((param.u1NumUser != 2) && (param.u1Dl_Ul == 2))
			pch = strsep(&arg, "-");

		if ((param.u1Dl_Ul == 2) && (pch != NULL)) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns2 = (ucNNS_MCS > 11);
			param.u1UlMcsUser2 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		} else if (pch == NULL) {
				Ret = 0;
				goto error;
		}
	}

	if (param.u1NumUser >= 3) {
		pch = strsep(&arg, "-");

		if (pch != NULL) {
			param.u2WlidUser3 = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
			param.u2WlidUser3 = cpu2le16(param.u2WlidUser3);
#endif
		}
		else {
			Ret = 0;
			goto error;
		}

		if (param.u1Dl_Ul != 2)
			pch = strsep(&arg, "");
		else
			pch = strsep(&arg, "-");

		if (pch != NULL) {
			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns3 = (ucNNS_MCS > 11);
			if (param.u1Dl_Ul != 1)
				param.u1DlMcsUser3 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
			else
				param.u1UlMcsUser3 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		if (param.u1Dl_Ul == 2)
			pch = strsep(&arg, "");

		if ((param.u1Dl_Ul == 2) && (pch != NULL)) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			param.u1Ns3 = (ucNNS_MCS > 11);
			param.u1UlMcsUser3 = (ucNNS_MCS > 11) ? (ucNNS_MCS - 11) : ucNNS_MCS;
		} else if (pch == NULL) {
				Ret = 0;
				goto error;
		}
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(subcmd) + sizeof(param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	subcmd = cpu2le32(subcmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&subcmd, sizeof(subcmd));
	AndesAppendCmdMsg(msg, (char *)&param, sizeof(param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}


static INT32 set_mumimo_force_mu_enable(RTMP_ADAPTER *pAd, BOOLEAN fgForceMu)
{
	INT32 Ret = TRUE;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = MURU_MU_MIMO_CTRL;
	UINT16 subcmd = MU_MIMO_SET_FORCE_MU;

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(subcmd) + sizeof(fgForceMu));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MURU_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&subcmd, sizeof(subcmd));
	AndesAppendCmdMsg(msg, (char *)&fgForceMu, sizeof(fgForceMu));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}
#endif

#ifdef SMART_CARRIER_SENSE_SUPPORT
#ifdef SCS_FW_OFFLOAD
static VOID check_scs_glo(struct _RTMP_ADAPTER *pAd, VOID *pData)
{
	P_DRV_SCS_GLO pDrvGlo = &pAd->CommonCfg.rScsGloInfo;
	P_EVENT_SCS_GLO pFwGlo = (P_EVENT_SCS_GLO)pData;
	UINT_32 DriverSize = 0, j;

	for (j = 0; j < 2; j++) {
		pDrvGlo->rscsband[j].u4Addr = pFwGlo->rscsband[j].u4Addr;
		pDrvGlo->rscsband[j].fgError =
		(pFwGlo->rscsband[j].u4Size != sizeof(SMART_CARRIER_SENSE_CTRL_GEN2_T))?1:0;

		if (pDrvGlo->rscsband[j].fgError) {
		DriverSize = sizeof(SMART_CARRIER_SENSE_CTRL_GEN2_T);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("\x1b[31m is not synced.\x1b[0m"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("Drive Size = %d, FW Size = %d",
			DriverSize, pFwGlo->rscsband[j].u4Size));
		}
	}
}

static VOID show_scs_info(struct _RTMP_ADAPTER *pAd)
{
	UINT_32 base;
	UINT_32 offset;
	UINT_32 addr, addr_r;
	UINT_8 i, j;
	BOOLEAN err;

	for (j = 0; j < 2; j++) {
		base = pAd->CommonCfg.rScsGloInfo.rscsband[j].u4Addr;
		err = pAd->CommonCfg.rScsGloInfo.rscsband[j].fgError;

		SyncMuruSramCheckAddr(pAd, base);
		if (err)
			continue;
		/* SCS_Show_Info */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("********** Band %d  Information *********\n", j));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("|-rscsband%d (0x%08X)\n", j, base));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u1SCSMinRssi);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1SCSMinRssi = %d\n",
			addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u4OneSecTxByteCount);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4OneSecTxByteCount = %d\n",
			addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u4OneSecRxByteCount);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4OneSecRxByteCount = %d\n",
			addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2CckPdBlkTh);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2CckPdBlkTh = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2OfdmPdBlkTh);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2OfdmPdBlkTh = %d\n", addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2SCSMinRssiTolerance);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2SCSMinRssiTolerance = %d\n",
			addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2CckPdThrMax);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2CckPdThrMax = %d\n",
			addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2OfdmPdThrMax);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2OfdmPdThrMax = %d\n",
			addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2CckPdThrMin);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2CckPdThrMin = %d\n",
			addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2OfdmPdThrMin);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2OfdmPdThrMin = %d\n",
			addr, muru_io_r_u16(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2IniAvgTput);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2IniAvgTput\n", addr));

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| | |-(0x%08X) u2IniAvgTput[%d] = %d\n",
			addr, i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2LastTputDiff);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2LastTputDiff\n", addr));

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| | |-(0x%08X) u2LastTputDiff[%d] = %d\n", addr,
			 i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2LastAvgTput);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2LastAvgTput\n", addr));

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| | |-(0x%08X) u2LastAvgTput[%d] = %d\n",
			addr, i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2LastMaxTput);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2LastMaxTput\n", addr));

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| | |-(0x%08X) u2LastMaxTput[%d] = %d\n",
			addr, i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2LastMinTput);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2LastMinTput\n", addr));

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| | |-(0x%08X) u2LastMinTput[%d] = %d\n",
			addr, i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u1LastTputIdx);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1LastTputIdx\n", addr));

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| | |-(0x%08X) u1LastTputIdx[%d] = %d\n",
			addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, fgLastTputDone);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgLastTputDone\n", addr));

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| | |-(0x%08X) fgLastTputDone[%d] = %d\n",
			addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u2CurAvgTput);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u2CurAvgTput\n", addr));

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_16) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| | |-(0x%08X) u2CurAvgTput[%d] = %d\n",
			addr, i, muru_io_r_u16(pAd, addr)));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u1CurTputIdx);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1CurTputIdx\n", addr));

		addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| | |-(0x%08X) u1CurTputIdx[%d] = %d\n",
			addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u1TputPeriodScaleBit);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1TputPeriodScaleBit\n", addr));
			addr_r = addr;
		for (i = 0; i < SCS_STA_NUM; i++) {
			addr = addr_r + sizeof(UINT_8) * i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| | |-(0x%08X) u1TputPeriodScaleBit[%d] = %d\n",
			addr, i, muru_io_r_u8(pAd, addr)));
		}

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u1ChannelBusyTh);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1ChannelBusyTh = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, fgChBusy);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgChBusy = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u1MyTxRxTh);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u1MyTxRxTh = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, fgPDreset);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) fgPDreset = %d\n", addr, muru_io_r_u8(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u4ChannelBusyTime);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4ChannelBusyTime = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u4MyTxAirtime);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4MyTxAirtime = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u4MyRxAirtime);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4MyRxAirtime = %d\n", addr, muru_io_r_u32(pAd, addr)));

		offset = OFFSET_OF(SMART_CARRIER_SENSE_CTRL_GEN2_T, u4OBSSAirtime);
		addr = base + offset;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("| |-(0x%08X) u4OBSSAirtime = %d\n", addr, muru_io_r_u32(pAd, addr)));
	}
}

#endif /*SCS_FW_OFFLOAD*/
#endif /*SMART_CARRIER_SENSE_SUPPORT*/

static UINT8 g_hwcfg_dump[1400];
INT32 show_hwcfg_info(RTMP_ADAPTER *pAd)
{
	UINT16 dump_offset = 0x0;
	UINT16 dump_size = 1200;
	UINT8 *dump_content = &g_hwcfg_dump[0];

	int i, j;

	for (j = 0; j < 3 ; j++) {

		dump_offset = j * 1200;
		if (j == 2)
			dump_size = 0xE00 - (2 * 1200);

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\x1b[1;33m %s: eeprom_type=%d, offset=%d, size=%d\x1b[m \n",
				 __func__, pAd->eeprom_type, dump_offset, dump_size));

		MtCmdHwcfgGet(pAd, dump_offset, dump_size, dump_content);

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump\n\r"));

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%*s ", 9, ""));
		for (i = 0; i < 16; i++) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x ", i));
		}
		for (i = 0; i < dump_size; i++) {
			if ((i%16) == 0) {
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\r"));
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%08x: ", i + (j * 1200)));
			}
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x ", dump_content[i]));
		}
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n\r"));
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\x1b[1;33m %s: End \x1b[m \n", __func__));
	return TRUE;
}

#ifdef VLAN_SUPPORT
#define WF_MDP_TOP_DCR1_ADDR	(0x820cd004)
static inline BOOLEAN is_eco_vlan_tci_enable(struct _RTMP_ADAPTER *ad)
{
	UINT32 value;

	MAC_IO_READ32(ad->hdev_ctrl, WF_MDP_TOP_DCR1_ADDR, &value);
	return (value & BIT16) ? TRUE : FALSE;
}
#endif

VOID mt7915_update_chip_cap(RTMP_ADAPTER *ad)
{
#ifdef VLAN_SUPPORT
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(ctrl);

	if (is_eco_vlan_tci_enable(ad))
		chip_cap->vlan_rx_tag_mode = VLAN_RX_TAG_HW_MODE;
#endif
}

static VOID mt7915_chipCap_init(struct _RTMP_ADAPTER *pAd, RTMP_CHIP_CAP *chip_cap)
{
	chip_cap->TXWISize = LMAC_TXD_MAX_SIZE;
	chip_cap->RXWISize = 28;
	chip_cap->tx_hw_hdr_len = chip_cap->TXWISize;
	chip_cap->rx_hw_hdr_len = 0x60;
	chip_cap->tx_ring_size = 1024;
	chip_cap->tkn_info.feature = TOKEN_TX;
	chip_cap->tkn_info.token_tx_cnt = 8192;
	chip_cap->tkn_info.band0_token_cnt = 2048;
	chip_cap->tkn_info.low_water_mark = 5;
	chip_cap->tkn_info.hw_tx_token_cnt = 7168;
	chip_cap->tkn_info.token_rx_cnt = 15360;
	chip_cap->multi_token_ques_per_band = TRUE;

	chip_cap->asic_caps = (fASIC_CAP_PMF_ENC | fASIC_CAP_MCS_LUT
					| fASIC_CAP_CT | fASIC_CAP_HW_DAMSDU | fASIC_CAP_MCU_OFFLOAD | fASIC_CAP_WMM_PKTDETECT_OFFLOAD);
	chip_cap_mcs_nss_init(&chip_cap->mcs_nss);
#ifdef HDR_TRANS_TX_SUPPORT
	chip_cap->asic_caps |= fASIC_CAP_TX_HDR_TRANS;
#endif
#ifdef HDR_TRANS_RX_SUPPORT
	chip_cap->asic_caps |= fASIC_CAP_RX_HDR_TRANS;
#endif
#ifdef CONFIG_CSO_SUPPORT
	chip_cap->asic_caps |= fASIC_CAP_CSO;
#endif
#ifdef RX_SCATTER
	chip_cap->asic_caps |= fASIC_CAP_RX_DMA_SCATTER;
#endif
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	chip_cap->asic_caps |= fASIC_CAP_TWT;
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
#ifdef HW_TX_AMSDU_SUPPORT
	chip_cap->asic_caps |= fASIC_CAP_HW_TX_AMSDU;
#endif /* HW_TX_AMSDU_SUPPORT */
#ifdef DBDC_MODE
	chip_cap->asic_caps |= fASIC_CAP_DBDC;
	chip_cap->asic_caps |= fASIC_CAP_SEPARATE_DBDC;
#endif
#ifdef WHNAT_SUPPORT
	chip_cap->asic_caps |= fASIC_CAP_WHNAT;
#endif

	chip_cap->asic_caps |= fASIC_CAP_DLY_INT_PER_RING;
	chip_cap->asic_caps |= fASIC_CAP_TWO_PCIE;
	chip_cap->asic_caps |= fASIC_CAP_TXCMD;
	chip_cap->asic_caps |= fASIC_CAP_FW_RESTART_POLLING_MODE;
	chip_cap->asic_caps |= fASIC_CAP_ADV_SECURITY;
	chip_cap->phy_caps = (fPHY_CAP_24G | fPHY_CAP_5G | fPHY_CAP_6G |
			fPHY_CAP_HT | fPHY_CAP_VHT | fPHY_CAP_HE | \
			fPHY_CAP_TXBF | fPHY_CAP_LDPC | \
			fPHY_CAP_BW40 | fPHY_CAP_BW80 | fPHY_CAP_BW160C | fPHY_CAP_BW160NC | \
			fPHY_CAP_BW20_242TONE | fPHY_CAP_HE_SR | \
			fPHY_CAP_HE_PPE_EXIST);

	chip_cap->phy_caps |= (fPHY_CAP_DL_MUMIMO | fPHY_CAP_UL_MUMIMO |
		fPHY_CAP_HE_DL_MUOFDMA | fPHY_CAP_HE_UL_MUOFDMA);

	chip_cap->hw_ops_ver = HWCTRL_OP_TYPE_V2;
	chip_cap->hw_protect_update_ver = HWCTRL_PROT_UPDATE_METHOD_V2;
	chip_cap->hif_type = HIF_MT;
	chip_cap->mac_type = MAC_MT;
	chip_cap->MCUType = ANDES | CR4;
	chip_cap->rf_type = RF_MT;
	chip_cap->MaxNumOfRfId = 127;
	chip_cap->MaxNumOfBbpId = 200;
	chip_cap->ProbeRspTimes = 2;
	chip_cap->FlgIsHwWapiSup = TRUE;
	chip_cap->FlgIsHwAntennaDiversitySup = FALSE;
#ifdef STREAM_MODE_SUPPORT
	chip_cap->FlgHwStreamMode = FALSE;
#endif
#ifdef TXBF_SUPPORT
	chip_cap->FlgHwTxBfCap = TXBF_HW_CAP | TXBF_HW_2BF;
#endif
	chip_cap->SnrFormula = SNR_FORMULA4;
#ifdef RTMP_EFUSE_SUPPORT
	chip_cap->EFUSE_USAGE_MAP_START = 0x1e0;
	chip_cap->EFUSE_USAGE_MAP_END = 0x1ff;
	chip_cap->EFUSE_USAGE_MAP_SIZE = 30;
	chip_cap->EFUSE_RESERVED_SIZE = 29;
#endif
	chip_cap->efuse_content_start = 0x0;
	chip_cap->efuse_content_end = 0xdff;
	chip_cap->EEPROM_DEFAULT_BIN = mt7915_get_default_bin_image(pAd);
	chip_cap->EEPROM_DEFAULT_BIN_SIZE = sizeof(MT7915_E2PImage_iPAiLNA);
	chip_cap->EFUSE_BUFFER_CONTENT_SIZE = sizeof(MT7915_E2PImage_iPAiLNA);
#ifdef CARRIER_DETECTION_SUPPORT
	chip_cap->carrier_func = TONE_RADAR_V2;
#endif

#ifdef NEW_MBSSID_MODE
#ifdef ENHANCE_NEW_MBSSID_MODE
	chip_cap->MBSSIDMode = MBSSID_MODE4;
#else
	chip_cap->MBSSIDMode = MBSSID_MODE1;
#endif /* ENHANCE_NEW_MBSSID_MODE */
#else
	chip_cap->MBSSIDMode = MBSSID_MODE0;
#endif /* NEW_MBSSID_MODE */
#ifdef DOT11W_PMF_SUPPORT
	chip_cap->FlgPMFEncrtptMode = PMF_ENCRYPT_MODE_2;
#endif /* DOT11W_PMF_SUPPORT */
#ifdef CONFIG_ANDES_SUPPORT
#ifdef NEED_ROM_PATCH
	chip_cap->need_load_patch = BIT(WM_CPU);
#else
	chip_cap->need_load_patch = 0;
#endif
	chip_cap->need_load_fw = BIT(WM_CPU) | BIT(WA_CPU);
	chip_cap->load_patch_flow = PATCH_FLOW_V1;
	chip_cap->load_fw_flow = FW_FLOW_V1;
	chip_cap->patch_format = PATCH_FORMAT_V2;
	chip_cap->fw_format = FW_FORMAT_V3;
	chip_cap->load_patch_method = BIT(HEADER_METHOD);
	chip_cap->load_fw_method = BIT(HEADER_METHOD);
	chip_cap->rom_patch_offset = MT7915_ROM_PATCH_START_ADDRESS;
#endif
#ifdef UNIFY_FW_CMD
	chip_cap->cmd_header_len = sizeof(FW_TXD) + sizeof(TMAC_TXD_L);
#else
	chip_cap->cmd_header_len = 12; /* sizeof(FW_TXD) */
#endif
	chip_cap->cmd_padding_len = 0;
	/* ppdu_caps */
	chip_cap->ppdu.TxAggLimit = 64;
	chip_cap->ppdu.non_he_tx_ba_wsize = BA_WIN_SZ_64;
	chip_cap->ppdu.non_he_rx_ba_wsize = BA_WIN_SZ_64;
	chip_cap->ppdu.he_tx_ba_wsize = BA_WIN_SZ_256;
	chip_cap->ppdu.he_rx_ba_wsize = BA_WIN_SZ_256;
	chip_cap->ppdu.max_amsdu_len = MPDU_7991_OCTETS;
	chip_cap->ppdu.ht_max_ampdu_len_exp = 3;
#ifdef DOT11_VHT_AC
	chip_cap->ppdu.max_mpdu_len = MPDU_7991_OCTETS;
	chip_cap->ppdu.vht_max_ampdu_len_exp = 7;
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
	chip_cap->ppdu.he_max_ampdu_len_exp = 3;
	chip_cap->ppdu.max_agg_tid_num = 1;
	chip_cap->ppdu.default_pe_duration = 4;/*unit:4us*/
	chip_cap->ppdu.er_su_dis = 1;/*disable ER_SU*/
	chip_cap->ppdu.trig_mac_pad_dur = PADDING_16US;/*unit:8us*/
#endif
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	chip_cap->fgRateAdaptFWOffload = TRUE;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	chip_cap->qos.WmmHwNum = 4;
	chip_cap->PDA_PORT = 0xf800;
	chip_cap->ppdu.tx_amsdu_support = TRUE;
	chip_cap->ppdu.rx_amsdu_in_ampdu_support = TRUE;
	chip_cap->APPSMode = APPS_MODE2;
	chip_cap->CtParseLen = CT_PARSE_PAYLOAD_LEN ;
	chip_cap->qm = FAST_PATH_QM;
#ifdef CONFIG_SOC_MT7621
	chip_cap->rx_qm_en = FALSE;
#endif
	chip_cap->rx_qm = GENERIC_QM;
	chip_cap->qm_tm = TASKLET_METHOD;
	chip_cap->hif_tm = TASKLET_METHOD;

	if (chip_cap->rx_qm == GENERIC_QM &&
		chip_cap->qm_tm == TASKLET_METHOD) {
		chip_cap->RxSwRpsEnable = TRUE;
		chip_cap->RxSwRpsTpThreshold = 600;
		chip_cap->sw_rps_tp_thd_dl = 1500;
		chip_cap->RxSwRpsCpu = 2;
	}

	chip_cap->qos.wmm_detect_method = WMM_DETECT_METHOD1;
	chip_cap->hw_max_amsdu_nums = 8;
	chip_cap->amsdu_txdcmp = 0xFF7FFFFF;
	chip_cap->band_cnt = 2;
	chip_cap->txd_type = TXD_V2;
	chip_cap->OmacNums = 5;
	chip_cap->BssNums = 4;
#if defined(CONFIG_COLGIN_MT6890)
	chip_cap->BcnMaxNum = 8;
#elif defined(CUSTOMER_VENDOR_IE_SUPPORT)
	chip_cap->BcnMaxNum = 16; /* for 1.5k bcn, at most 16 limited by fw */
	chip_cap->BcnMaxLength = 1500; /* limited by harrier fw */
#else
	chip_cap->BcnMaxNum = 32;
#endif

	chip_cap->ExtMbssOmacStartIdx = 0x10;
	chip_cap->RepeaterStartIdx = 0x20;
#ifdef AIR_MONITOR
	chip_cap->MaxRepeaterNum = 16 * chip_cap->band_cnt;
#else
	chip_cap->MaxRepeaterNum = 16 * chip_cap->band_cnt;
#endif /* AIR_MONITOR */
#ifdef BCN_OFFLOAD_SUPPORT
	chip_cap->fgBcnOffloadSupport = TRUE;
	chip_cap->fgIsNeedPretbttIntEvent = FALSE;
#endif
	chip_cap->tx_delay_support = TRUE;
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	chip_cap->twt_hw_num = TWT_HW_AGRT_MAX_NUM;
#endif /* WIFI_TWT_SUPPORT */
	chip_cap->mu_edca_timer = 255;
#endif /* DOT11_HE_AX */
	if (MT7915_MT_WTBL_SIZE <= MAX_LEN_OF_MAC_TABLE)
		chip_cap->wtbl_max_entries = MT7915_MT_WTBL_SIZE;
	else
		chip_cap->wtbl_max_entries = MAX_LEN_OF_MAC_TABLE;
	chip_cap->wtbl_no_matched = 0x3ff;	/* report [9:0] all 1s if no entry was matched */
	chip_cap->single_sku_type_parse_num = SINGLE_SKU_TYPE_PARSE_NUM_V1;
	chip_cap->single_sku_para_parse_num = SINGLE_SKU_PARAM_PARSE_NUM_V1;
	chip_cap->single_sku_type_num = SINGLE_SKU_TYPE_NUM_V1;
	chip_cap->single_sku_para_num = SINGLE_SKU_PARAM_NUM_V1;
	chip_cap->backoff_type_parse_num = BACKOFF_TYPE_PARSE_NUM_V1;
	chip_cap->backoff_para_parse_num = BACKOFF_PARAM_PARSE_NUM_V1;
	chip_cap->backoff_type_num = BACKOFF_TYPE_NUM_V1;
	chip_cap->backoff_para_num = BACKOFF_PARAM_NUM_V1;
	chip_cap->single_sku_fill_tbl_cck = SINGLE_SKU_FILL_TABLE_CCK_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_ofdm = SINGLE_SKU_FILL_TABLE_OFDM_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_ht20 = SINGLE_SKU_FILL_TABLE_HT20_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_ht40 = SINGLE_SKU_FILL_TABLE_HT40_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_vht20 = SINGLE_SKU_FILL_TABLE_VHT20_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_vht40 = SINGLE_SKU_FILL_TABLE_VHT40_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_vht80 = SINGLE_SKU_FILL_TABLE_VHT80_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_vht160 = SINGLE_SKU_FILL_TABLE_VHT160_LENGTH_V1;
	chip_cap->single_sku_parse_tbl_htvht40 = SINGLE_SKU_PARSE_TABLE_HTVHT40_LENGTH_V1;
	chip_cap->backoff_tbl_bfon_ht40 = BACKOFF_TABLE_BF_ON_HT40_LENGTH_V1;
	chip_cap->single_sku_fill_tbl_length = MT7915_SINGLE_SKU_FILL_TABLE_LENGTH;
	chip_cap->txpower_type = TX_POWER_TYPE_V1;
	chip_cap->single_sku_tbl_type_ht40 = SINGLE_SKU_TABLE_HT40;
	chip_cap->backoff_tbl_bf_on_type_ht40 = BACKOFF_TABLE_BF_ON_HT40;
	chip_cap->spe_map_list.spe_map = mt7915_spe_map;
	chip_cap->spe_map_list.size = ARRAY_SIZE(mt7915_spe_map);
	chip_cap->channelbw = BW_20;
	chip_cap->mgmt_ctrl_frm_hw_htc_disable = TRUE;
	chip_cap->peak_txop = TXOP_BB;
#ifdef BCN_PROTECTION_SUPPORT
	chip_cap->bcn_prot_sup = BCN_PROT_EN_SW_MODE;
#endif
#ifdef VLAN_SUPPORT
	chip_cap->vlan_rx_tag_mode = VLAN_RX_TAG_SW_MODE;
#endif
#ifdef PRE_CAL_MT7915_SUPPORT
	/* DW0 : Used for save total pre-cal size
	 * DW1 : reserved
	 * DW2 : reserved
	 * DW3 : reserved
	  */
	chip_cap->prek_ee_info.info_size = 0x10;
	chip_cap->prek_ee_info.cal_size = PRE_CAL_TOTAL_SIZE;
#endif
#ifdef OCE_SUPPORT
       chip_cap->FdFrameFwOffloadEnabled = TRUE;
#endif /* OCE_SUPPORT */
}

static VOID mt7915_chipCap_post_init(struct _RTMP_ADAPTER *pAd, RTMP_CHIP_CAP *chip_cap)
{
	if (MTK_REV_ET(pAd, MT7915, MT7915E1))
		chip_cap->txd_flow_ctl = TRUE;

	chip_cap->asic_caps |= fASIC_CAP_ADDBA_HW_SSN;

	MAC_IO_READ32(pAd->hdev_ctrl, 0x70010210, &chip_cap->hw_version);
}

static struct _RTMP_CHIP_OP mt7915_chip_op = { 0 };

static VOID mt7915_chipOp_init(struct _RTMP_ADAPTER *pAd, RTMP_CHIP_OP *chip_op)
{
	chip_op->AsicRfInit = init_rf_cr;
	chip_op->AsicBbpInit = BBPInit;
	chip_op->AsicMacInit = init_mac_cr;
	chip_op->AsicReverseRfFromSleepMode = NULL;
	chip_op->AsicHaltAction = NULL;
	/* BBP adjust */
	chip_op->ChipBBPAdjust = NULL;
	chip_op->ChipSwitchChannel = switch_channel;

#ifdef SMART_CARRIER_SENSE_SUPPORT
#ifdef SCS_FW_OFFLOAD
	chip_op->SmartCarrierSense = SmartCarrierSense_Gen6;
	chip_op->ChipSetSCS = SetSCS;
#endif /* SCS_FW_OFFLOAD */
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#ifdef NEW_SET_RX_STREAM
	chip_op->ChipSetRxStream = set_RxStream;
#endif
	chip_op->AsicAntennaDefaultReset = antenna_default_reset;
#ifdef CONFIG_STA_SUPPORT
	chip_op->NetDevNickNameInit = init_dev_nick_name;
#endif
#ifdef CAL_FREE_IC_SUPPORT
	chip_op->is_cal_free_ic = is_cal_free_ic;
	chip_op->cal_free_data_get = cal_free_data_get;
#endif /* CAL_FREE_IC_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
	chip_op->ToneRadarProgram = ToneRadarProgram_v2;
#endif
	chip_op->DisableTxRx = NULL; /* 302 */
#ifdef RTMP_PCI_SUPPORT
	chip_op->interrupt_enable = pci_interrupt_enable;
	chip_op->interrupt_disable = pci_interrupt_disable;
	chip_op->trigger_int_to_mcu = pci_trigger_int_to_mcu;
	chip_op->subsys_int_handler = pci_subsys_int_handler;
	chip_op->sw_int_handler = pci_sw_int_handler;
#endif
	chip_op->get_sku_tbl_idx = mt7915_get_sku_tbl_idx;
	chip_op->check_RF_lock_down = mt7915_check_RF_lock_down;
	chip_op->Config_Effuse_Country = mt7915_Config_Effuse_Country;
#ifdef MT_WOW_SUPPORT
	chip_op->AsicWOWEnable = MT76xxAndesWOWEnable;
	chip_op->AsicWOWDisable = MT76xxAndesWOWDisable;
	/* chip_op->.AsicWOWInit = MT76xxAndesWOWInit, */
#endif /* MT_WOW_SUPPORT */
	chip_op->show_pwr_info = NULL;
	chip_op->bufferModeCmdFill = bufferModeCmdFill;
	chip_op->MtCmdTx = MtCmdSendMsg;
	chip_op->prepare_fwdl_img = fw_prepare;
	chip_op->fwdl_datapath_setup = fwdl_datapath_setup;
	chip_op->HeraStbcPriorityCtrl   = CmdHeraStbcPriorityCtrl;
#ifdef TXBF_SUPPORT
	chip_op->iBFPhaseComp			= mt7915_iBFPhaseComp;
	chip_op->iBFPhaseCalInit		= mt7915_iBFPhaseCalInit;
	chip_op->iBFPhaseFreeMem		= mt7915_iBFPhaseFreeMem;
	chip_op->iBFPhaseCalE2PUpdate	= mt7915_iBFPhaseCalE2PUpdate;
	chip_op->iBFPhaseCalReport		= mt7915_iBFPhaseCalReport;
	chip_op->iBfCaleBfPfmuMemAlloc	= mt7915_eBFPfmuMemAlloc;
	chip_op->iBfCaliBfPfmuMemAlloc	= mt7915_iBFPfmuMemAlloc;
	chip_op->set_manual_assoc		= NULL;
	chip_op->set_cmm_starec			= NULL;
	chip_op->TxBFInit				= mt_WrapTxBFInit;
	chip_op->ClientSupportsETxBF	= mt_WrapClientSupportsETxBF;
	chip_op->setETxBFCap			= setETxBFCap;
	chip_op->BfStaRecUpdate			= mt_AsicBfStaRecUpdate;
	chip_op->BfeeStaRecUpdate		= mt_AsicBfeeStaRecUpdate;
	chip_op->BfStaRecRelease		= mt_AsicBfStaRecRelease;
	chip_op->BfPfmuMemAlloc			= CmdPfmuMemAlloc;
	chip_op->BfPfmuMemRelease		= CmdPfmuMemRelease;
	chip_op->TxBfTxApplyCtrl		= CmdTxBfTxApplyCtrl;
	chip_op->BfApClientCluster		= CmdTxBfApClientCluster;
	chip_op->BfHwEnStatusUpdate		= CmdTxBfHwEnableStatusUpdate;
	chip_op->BfModuleEnCtrl			= CmdTxBfModuleEnCtrl;
	chip_op->BfCfgBfPhy				= CmdETxBfCfgBfPhy;
	chip_op->dump_pfmu_tag			= txbf_dump_tag;
	chip_op->set_txbf_pfmu_tag		= chip_set_txbf_pfmu_tag;
	chip_op->write_txbf_pfmu_tag	= chip_write_txbf_pfmu_tag;
	chip_op->dump_pfmu_data			= txbf_show_pfmu_data;
	chip_op->write_txbf_profile_data  = chip_write_txbf_profile_data;
	chip_op->set_txbf_angle			= chip_set_txbf_angle;
	chip_op->set_txbf_dsnr			= chip_set_txbf_dsnr;
	chip_op->write_txbf_pfmu_data	= chip_write_txbf_pfmu_data;
#ifdef CONFIG_STA_SUPPORT
	chip_op->archSetAid				= NULL;
#endif /* CONFIG_STA_SUPPORT */
#ifdef VHT_TXBF_SUPPORT
	chip_op->ClientSupportsVhtETxBF	 = mt_WrapClientSupportsVhtETxBF;
	chip_op->setVHTETxBFCap			 = setVHTETxBFCap;
#endif /* VHT_TXBF_SUPPORT */
#ifdef HE_TXBF_SUPPORT
	chip_op->get_he_etxbf_cap = get_he_etxbf_cap;
#endif /* HE_TXBF_SUPPORT */
#endif /* TXBF_SUPPORT */
#ifdef GREENAP_SUPPORT
	chip_op->EnableAPMIMOPS = enable_greenap;
	chip_op->DisableAPMIMOPS = disable_greenap;
#endif /* GREENAP_SUPPORT */
#ifdef INTERNAL_CAPTURE_SUPPORT
	chip_op->ICapStart = MtCmdRfTestICapStart;
	chip_op->ICapStatus = MtCmdRfTestSolicitICapStatus;
	chip_op->ICapCmdUnSolicitRawDataProc = NULL;
	chip_op->ICapCmdSolicitRawDataProc = MtCmdRfTestSolicitICapRawDataProc;
	chip_op->ICapGetIQData = NULL;
	chip_op->ICapEventRawDataHandler = NULL;
#endif /* INTERNAL_CAPTURE_SUPPORT */
#ifdef WIFI_SPECTRUM_SUPPORT
	chip_op->SpectrumStart = MtCmdWifiSpectrumStart;
	chip_op->SpectrumStatus = MtCmdWifiSpectrumSolicitCapStatus;
	chip_op->SpectrumCmdRawDataProc = MtCmdWifiSpectrumUnSolicitRawDataProc;
	chip_op->SpectrumEventRawDataHandler = ExtEventWifiSpectrumUnSolicitIQDataHandler;
#endif /* WIFI_SPECTRUM_SUPPORT */
	chip_op->dma_shdl_init = NULL;
	chip_op->irq_init = chip_irq_init;
	chip_op->hif_init_dma = hif_init_WPDMA;
	chip_op->hif_set_dma = hif_set_WPDMA;
	chip_op->hif_wait_dma_idle = hif_wait_WPDMA_idle;
	chip_op->hif_reset_dma = hif_reset_WPDMA;
	chip_op->hif_cfg_dly_int = hif_cfg_dly_int;
	chip_op->get_fw_sync_value = get_fw_sync_value;
	chip_op->read_chl_pwr = NULL;
	chip_op->get_wf_path_comb = mt7915_get_wf_path_comb;
	chip_op->get_rx_stat_band = mt7915_get_rx_stat_band;
	chip_op->get_rx_stat_path = mt7915_get_rx_stat_path;
	chip_op->get_rx_stat_user = mt7915_get_rx_stat_user;
	chip_op->get_rx_stat_comm = mt7915_get_rx_stat_comm;
	chip_op->get_rx_stat = mt7915_get_rx_stat;
	chip_op->rxv_get_byte_cnt = mt7915_rxv_get_byte_cnt;
	chip_op->rxv_get_content = mt7915_rxv_get_content;
	chip_op->rxv_packet_parse = mt7915_rxv_packet_parse;
	chip_op->rxv_entry_parse = mt7915_rxv_entry_parse;
	chip_op->rxv_info_show = mt7915_rxv_info_show;
	chip_op->rxv_raw_data_show = mt7915_rxv_raw_data_show;
	chip_op->rxv_stat_reset = mt7915_rxv_stat_reset;
	chip_op->rxv_cap_init = mt7915_rxv_cap_init;
	chip_op->rxv_dump_start = mt7915_rxv_dump_start;
	chip_op->rxv_dump_stop = mt7915_rxv_dump_stop;
	chip_op->rxv_dump_buf_alloc = mt7915_rxv_dump_buf_alloc;
	chip_op->rxv_dump_buf_clear = mt7915_rxv_dump_buf_clear;
	chip_op->rxv_dump_show_list = mt7915_rxv_dump_show_list;
	chip_op->rxv_dump_show_rpt = mt7915_rxv_dump_show_rpt;
	chip_op->rxv_dump_rxv_content_compose = mt7915_rxv_dump_rxv_content_compose;
	chip_op->rxv_content_len = mt7915_rxv_content_len;
	chip_op->txs_handler = mtf_txs_handler;
#ifdef CONFIG_FWOWN_SUPPORT
	chip_op->driver_own = driver_own;
	chip_op->fw_own = fw_own;
	chip_op->fw_own_sts = fw_own_sts;
#endif
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	chip_op->twt_agrt_update = rtmp_twt_agrt_update;
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
	chip_op->get_tid_sn = NULL;
	chip_op->hif_io_remap_read32 = pci_io_remap_read32;
	chip_op->hif_io_remap_write32 = pci_io_remap_write32;
	chip_op->hif_chip_match = mt7915_hif_chip_match;
	/* finetune to make band1 TXD go though ring0 then TXD/TXP do not content PCIe1 Bus
	   chip_op->hif_pci_data_ring_assign = mt7915_hif_pci_data_ring_assign; */
	chip_op->hif_pci_data_ring_assign = NULL;
	chip_op->fill_key_install_cmd = fill_key_install_cmd_v2;
#ifdef CONFIG_TX_DELAY
	chip_op->tx_deley_parm_init = mt7915_tx_deley_parm_init;
#endif
#ifdef BACKGROUND_SCAN_SUPPORT
	chip_op->set_off_ch_scan = mt_off_ch_scan_dedicated;
	chip_op->bgnd_scan_cr_init = bgnd_scan_ipi_cr_init;
#endif
#ifdef ERR_RECOVERY
	chip_op->dump_ser_stat = mt7915_dump_ser_stat;
#endif
#ifdef CFG_SUPPORT_FALCON_MURU
	chip_op->check_muru_glo = check_muru_glo;
	chip_op->show_muru_local_data = show_muru_local_data;
	chip_op->show_muru_tx_info    = show_muru_tx_info;
	chip_op->show_muru_shared_data = show_muru_shared_data;
	chip_op->show_muru_mancfg_data = show_muru_mancfg_data;
	chip_op->show_muru_stacap_info = show_muru_stacap_info;
	chip_op->show_mumimo_algorithm_monitor = show_mumimo_algorithm_monitor;
	chip_op->set_mumimo_fixed_rate = set_mumimo_fixed_rate;
	chip_op->set_mumimo_fixed_group_rate  = set_mumimo_fixed_group_rate;
	chip_op->set_mumimo_force_mu_enable = set_mumimo_force_mu_enable;
	chip_op->show_mumimo_group_entry_tbl = show_mumimo_group_entry_tbl;
	chip_op->set_muru_data = set_muru_data;
	chip_op->show_muru_txc_tx_stats = show_muru_txc_tx_stats;
#endif

#ifdef SMART_CARRIER_SENSE_SUPPORT
#ifdef SCS_FW_OFFLOAD
	chip_op->check_scs_glo = check_scs_glo;
	chip_op->show_scs_info = show_scs_info;
#endif /*SCS_FW_OFFLOAD*/
#endif /*SMART_CARRIER_SENSE_SUPPORT*/
	chip_op->show_hwcfg_info = show_hwcfg_info;

#if defined(CONFIG_ATE)
	chip_op->backup_reg_before_ate = mtf_ate_mac_cr_backup_and_set;
	chip_op->restore_reg_after_ate = mtf_ate_mac_cr_restore;
	chip_op->restore_reg_during_ate = mtf_ate_ipg_cr_restore;
	chip_op->set_ifs = mtf_ate_set_ifs_cr;
	chip_op->set_ba_limit = mtf_ate_ampdu_ba_limit;
	chip_op->pause_ac_queue = mtf_ate_set_sta_pause_cr;
#endif
	chip_op->rssi_get = mt7915_rssi_get;
	chip_op->cninfo_get = mt7915_cninfo_get;
	chip_op->txpower_show_info = mtf_txpower_show_info;
	chip_op->update_mib_bucket = mt7915_update_mib_bucket;
	chip_op->ctrl_rxv_group = mt7915_ctrl_rxv_group;
	chip_op->get_bin_image_file = mt7915_get_default_bin_image_file;
#if defined(PRE_CAL_TRX_SET1_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) || defined(RLM_CAL_CACHE_SUPPORT)
	chip_op->get_prek_image_file = mt7915_get_prek_image_file;
#endif
	chip_op->set_mgmt_pkt_txpwr_prctg = mt7915_set_mgmt_pkt_txpwr_prctg;
#ifdef RTMP_EFUSE_SUPPORT
	chip_op->get_efuse_free_blk_bnum = mt7915_get_efuse_free_blk_num;
#endif
#ifdef LED_CONTROL_SUPPORT
	chip_op->wps_led_init = mt7915_wps_led_init;
	chip_op->wps_led_control = mt7915_wps_led_control;
#endif /*LED_CONTROL_SUPPORT*/
	chip_op->update_chip_cap = mt7915_update_chip_cap;
}

static VOID mt7915_archOp_init(RTMP_ADAPTER *ad, RTMP_ARCH_OP *arch_ops)
{
	arch_ops->archGetChBusyCnt = MtAsicGetChBusyCntByFw;
	arch_ops->archGetCCACnt = MtAsicGetCCACnt;
	arch_ops->archRcpiReset = MtfAsicRcpiReset;
	arch_ops->show_mac_info = mtf_show_mac_info;
	arch_ops->get_wtbl_entry234 = mtf_get_wtbl_entry234;
	arch_ops->init_wtbl = mtf_init_wtbl;
#ifdef CONFIG_STA_SUPPORT
	arch_ops->archEnableIbssSync = NULL;
#endif /* CONFIG_STA_SUPPORT */
	arch_ops->archAutoFallbackInit = NULL;
	arch_ops->archUpdateProtect = asic_wrap_protinfo_in_bssinfo;
	arch_ops->archUpdateRtsThld = MtAsicUpdateRtsThldByFw;
	arch_ops->archSwitchChannel = MtfAsicSwitchChannel;
	arch_ops->archSetRDG = NULL;
	arch_ops->archSetDevMac = MtAsicSetDevMacByFw;
	arch_ops->archSetBssid = MtAsicSetBssidByFw;
	arch_ops->archSetStaRec = MtAsicSetStaRecByFw;
	arch_ops->archUpdateStaRecBa = MtAsicUpdateStaRecBaByFw;
	arch_ops->asic_rts_on_off = mtf_asic_rts_on_off;
	arch_ops->asic_set_agglimit = mtf_asic_set_agglimit;
	arch_ops->asic_set_rts_retrylimit = mtf_asic_set_rts_retrylimit;
#ifdef CONFIG_AP_SUPPORT
	arch_ops->archSetWdevIfAddr = MtAsicSetWdevIfAddr;
	arch_ops->archSetMbssHwCRSetting = MtfDmacSetMbssHwCRSetting;
	arch_ops->archSetExtTTTTHwCRSetting = MtfDmacSetExtTTTTHwCRSetting;
	arch_ops->archSetExtMbssEnableCR = MtfDmacSetExtMbssEnableCR;
#endif /* CONFIG_AP_SUPPORT */
	arch_ops->archDelWcidTab = MtAsicDelWcidTabByFw;
#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
	arch_ops->archSetWcid4Addr_HdrTrans = MtAsicSetWcid4Addr_HdrTransByFw;
#endif
	arch_ops->archAddRemoveKeyTab = MtAsicAddRemoveKeyTabByFw;
#ifdef BCN_OFFLOAD_SUPPORT
	arch_ops->archEnableBeacon = NULL;
	arch_ops->archDisableBeacon = NULL;
#ifdef DOT11V_MBSSID_SUPPORT
	arch_ops->archUpdateBeacon = MtUpdateBcnToMcuV2;
#else
	arch_ops->archUpdateBeacon = MtUpdateBcnToMcu;
#endif
#else
	arch_ops->archEnableBeacon = MtfDmacAsicEnableBeacon;
	arch_ops->archDisableBeacon = MtfDmacAsicDisableBeacon;
#endif
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	arch_ops->archSetReptFuncEnable = MtAsicSetReptFuncEnableByFw;
	arch_ops->archInsertRepeaterEntry = MtAsicInsertRepeaterEntryByFw;
	arch_ops->archRemoveRepeaterEntry = MtAsicRemoveRepeaterEntryByFw;
	arch_ops->archInsertRepeaterRootEntry = MtAsicInsertRepeaterRootEntryByFw;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */
	arch_ops->archGetTsfTime = MtfAsicGetTsfTimeByDriver;
	arch_ops->archSetPreTbtt = NULL;
	arch_ops->archSetGPTimer = MtfAsicSetGPTimer;
	arch_ops->archDisableSync = NULL;
	arch_ops->archSetSyncModeAndEnable = NULL;
	arch_ops->archSetWmmParam = MtAsicSetWmmParam;
	arch_ops->archGetWmmParam = MtfAsicGetWmmParam;
	arch_ops->archSetEdcaParm = MtAsicSetEdcaParm;
	arch_ops->archSetSlotTime = MtAsicSetSlotTime;
	arch_ops->archGetTxTsc = MtAsicGetTxTscByFw;
	arch_ops->archSetBW = MtfAsicSetBW;
	arch_ops->archGetRxStat = MtfAsicGetRxStat;
	arch_ops->archSetTmrCal = MtfSetTmrCal;
	arch_ops->archGetFwSyncValue = MtfAsicGetFwSyncValue;
	arch_ops->archInitMac = MtfAsicInitMac;
	arch_ops->archGetHwQFromAc = mtf_get_hwq_from_ac;
	arch_ops->write_tmac_info_beacon = mtf_write_tmac_info_beacon;
	arch_ops->get_nsts_by_mcs = mtf_get_nsts_by_mcs;
	arch_ops->tx_rate_to_tmi_rate = mtf_tx_rate_to_tmi_rate;
	arch_ops->update_raw_counters = mtf_update_raw_counters;
	arch_ops->update_mib_bucket = mtf_update_mib_bucket;
#ifdef MAC_INIT_OFFLOAD
	arch_ops->archSetMacTxRx = MtAsicSetMacTxRxByFw;
	arch_ops->archSetMacMaxLen = NULL;
	arch_ops->archSetTxStream = NULL;
	arch_ops->archSetRxFilter = NULL;/* MtAsicSetRxFilter; */
	arch_ops->archSetRxvFilter = MtAsicSetRxvFilter;
#else
	arch_ops->archSetMacTxRx = MtfAsicSetMacTxRx;
	arch_ops->archSetTxStream = MtfAsicSetTxStream;
	arch_ops->archSetRxFilter = MtfAsicSetRxFilter;
	arch_ops->archTxCntUpdate = MtfAsicTxCntUpdate;
	arch_ops->archTxCntUpdate = MtfAsicTxCapAndRateTableUpdate;
#endif /*MAC_INIT_OFFLOAD*/
#ifdef DOT11_VHT_AC
	arch_ops->archSetRtsSignalTA = mtf_asic_set_rts_signal_ta;
#endif /*  DOT11_VHT_AC */
	arch_ops->archTOPInit = MtfAsicTOPInit;
	arch_ops->archSetTmrCR = MtSetTmrCRByFw;
	arch_ops->archUpdateRxWCIDTable = MtAsicUpdateRxWCIDTableByFw;
#ifdef TXBF_SUPPORT
	arch_ops->archUpdateClientBfCap = mt_AsicClientBfCap;
#endif /* TXBF_SUPPORT */
	arch_ops->archUpdateBASession = MtAsicUpdateBASessionOffloadByFw;
	arch_ops->archSetSMPS = MtAsicSetSMPSByFw;
#ifdef DBDC_MODE
	arch_ops->archSetDbdcCtrl = NULL; /* MtAsicSetDbdcCtrlByFw; */
	arch_ops->archGetDbdcCtrl = NULL; /* MtAsicGetDbdcCtrlByFw; */
#endif /*DBDC_MODE*/
	arch_ops->archRxHeaderTransCtl = MtAsicRxHeaderTransCtl;
	arch_ops->archRxHeaderTaranBLCtl = MtAsicRxHeaderTaranBLCtl;
#ifdef VLAN_SUPPORT
	arch_ops->update_vlan_id = mt_asic_update_vlan_id_by_fw;
	arch_ops->update_vlan_priority = mt_asic_update_vlan_priority_by_fw;
#endif
	arch_ops->rx_pkt_process = mt_rx_pkt_process;
	arch_ops->rx_event_handler = mtf_rx_event_handler;
	arch_ops->fill_cmd_header = mtf_fill_cmd_header;
	arch_ops->trans_rxd_into_rxblk = mtf_trans_rxd_into_rxblk;
#ifdef IGMP_SNOOP_SUPPORT
	arch_ops->archMcastEntryInsert = CmdMcastEntryInsert;
	arch_ops->archMcastEntryDelete = CmdMcastEntryDelete;
#ifdef IGMP_TVM_SUPPORT
	arch_ops->archMcastConfigAgeout = CmdSetMcastEntryAgeOut;
	arch_ops->archMcastGetMcastTable = CmdGetMcastEntryTable;
#endif /* IGMP_TVM_SUPPORT */
#endif

#ifdef WFDMA_WED_COMPATIBLE
	arch_ops->asic_wa_update = mtf_wa_cpu_update;
#endif
	arch_ops->write_txp_info = mtf_write_txp_info_by_wa;
	arch_ops->write_tmac_info_fixed_rate = mtf_write_tmac_info_fixed_rate;
	arch_ops->write_tmac_info = mtf_write_tmac_info;
	arch_ops->dump_tmac_info = mtf_dump_tmac_info;
	arch_ops->dump_rmac_info = mtf_dump_rmac_info;
	arch_ops->dump_rx_info = mtf_dump_rxinfo;
	arch_ops->dump_rmac_info_for_icverr = mtf_dump_rmac_info_for_ICVERR;
	arch_ops->dump_txs = mtf_dump_txs;
	arch_ops->dump_dmac_amsdu_info = mtf_dump_dmac_amsdu_info;
	arch_ops->get_packet_type = mtf_get_packet_type;
	arch_ops->txdone_handle = mtf_txdone_handle;
	arch_ops->rxv_handler = mtf_rxv_handler;
	arch_ops->check_hw_resource = mt_ct_check_hw_resource;
	arch_ops->get_hw_resource_state = mt_ct_get_hw_resource_state;
	arch_ops->hw_tx = mt_ct_hw_tx;
	arch_ops->mlme_hw_tx = mt_ct_mlme_hw_tx;
	arch_ops->dump_wtbl_info = mtf_dump_wtbl_info;
	arch_ops->dump_wtbl_base_info = mtf_dump_wtbl_base_info;
	/*hif related*/
#ifdef CTXD_SCATTER_AND_GATHER
	arch_ops->write_tx_resource = mtd_pci_write_tx_resource;
#elif defined(CTXD_MEM_CPY)
	arch_ops->write_tx_resource = mtd_pci_write_tx_resource_for_ctxd;
#else
	arch_ops->write_tx_resource = mtd_pci_write_tx_resource;
#endif
#ifdef CTXD_SCATTER_AND_GATHER
	arch_ops->write_last_tx_resource = mtd_pci_write_last_tx_resource_last_sec;
#elif defined(CTXD_MEM_CPY)
	arch_ops->write_last_tx_resource = mtd_pci_write_last_tx_resource;
#endif
	arch_ops->write_frag_tx_resource = pci_write_frag_tx_resource;
	arch_ops->init_txrx_ring = mtd_asic_init_txrx_ring;

	arch_ops->arch_calculate_ecc = mtf_calculate_ecc;
#ifdef TX_POWER_CONTROL_SUPPORT
	arch_ops->arch_txpower_boost = mtf_txpower_boost;
	arch_ops->arch_txpower_boost_ctrl = mtf_txpower_boost_ctrl;
	arch_ops->arch_txpower_boost_rate_type = mtf_txpower_boost_rate_type;
	arch_ops->arch_txpower_boost_power_cat_type = mtf_txpower_boost_power_cat_type;
	arch_ops->arch_txpower_boost_info_V0 = NULL;
	arch_ops->arch_txpower_boost_info_V1 = mtf_txpower_boost_info;
	arch_ops->arch_txpower_boost_profile = mtf_txpower_boost_profile;
#endif
	arch_ops->arch_txpower_all_rate_info = mtf_txpower_all_rate_info;
#ifdef SINGLE_SKU_V2
	arch_ops->arch_txpower_sku_cfg_para = mtf_txpower_sku_cfg_para;
#endif
	arch_ops->arch_get_bcn_tx_cnt = mtf_get_mib_bcn_tx_cnt;
#ifdef AIR_MONITOR
	arch_ops->arch_set_air_mon_enable = mtf_set_air_monitor_enable;
	arch_ops->arch_set_air_mon_rule = mtf_set_air_monitor_rule;
	arch_ops->arch_set_air_mon_idx = mtf_set_air_monitor_idx;
#endif

}

static VOID mt7915_chipOp_post_init(struct _RTMP_ADAPTER *pAd, RTMP_CHIP_OP *chip_op)
{
#ifdef MT7915_E1_WORKAROUND
	if (MTK_REV_ET(pAd, MT7915, MT7915E1)) {
#ifdef WFDMA_WED_COMPATIBLE
		chip_op->sw_int_polling = mt7915_sw_int_polling;
#endif
	}
#endif
}

static VOID mt7915_hif_ctrl_init(struct _RTMP_ADAPTER *pAd)
{
}

INT32 mt7915_get_chip_info(RTMP_ADAPTER *pAd)
{
	UINT32 value = 0;

	MAC_IO_READ32(pAd->hdev_ctrl, MT_CBTOP_HW_VER, &value);
	pAd->HWVersion = value;
	MAC_IO_READ32(pAd->hdev_ctrl, MT_CBTOP_FW_VER, &value);
	pAd->FWVersion = value;
	MAC_IO_READ32(pAd->hdev_ctrl, MT_CBTOP_HW_CODE, &value);
	pAd->ChipID = value;
	MAC_IO_READ32(pAd->hdev_ctrl, MT_CBTOP_HW_BND, &value);
	pAd->hw_bound = value;

	return TRUE;
}

VOID mt7915_init(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(ctrl);
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(ctrl);
	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ctrl);
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(ctrl);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __func__));
	mt7915_chipCap_init(pAd, pChipCap);
	mt7915_chipOp_init(pAd, &mt7915_chip_op);
	hc_register_chip_ops(ctrl, &mt7915_chip_op);
	mt7915_archOp_init(pAd, arch_ops);
	mt7915_chip_dbg_init(chip_dbg);
	mt7915_hif_ctrl_init(pAd);
	mt_phy_probe(pAd);
	mt7915_get_chip_info(pAd);
	mt7915_chipCap_post_init(pAd, pChipCap);
	mt7915_chipOp_post_init(pAd, chip_ops);
	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_AGBS);

	/* For calibration log buffer size limitation issue */
	pAd->fgQAtoolBatchDumpSupport = TRUE;

	pAd->cp_have_cr4 = TRUE;

#ifdef CONFIG_AP_SUPPORT
	/*VOW CR Address offset - Gen_FALCON*/
	pAd->vow_gen.VOW_GEN = VOW_GEN_FALCON;
#endif /* #ifdef CONFIG_AP_SUPPORT */

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<--%s()\n", __func__));
}

VOID mt7915_hif_ctrl_chip_init(VOID *hif_chip)
{
	struct pci_hif_chip *hif = hif_chip;

	hif->int_ena_reg_addr = MT_INT_MASK_CSR;
	hif->int_enable_mask = MT_INT_CoherentInt | MT_INT_DMA1_T16_DONE |
						MT_INT_DMA1_T17_DONE | MT_INT_DMA1_T20_DONE | MT_INT_RX;
#if defined(ERR_RECOVERY) || defined(CONFIG_FWOWN_SUPPORT)
	hif->int_enable_mask |= MT_INT_MCU2HOST_SW_INT_STS | MT_INT_SUBSYS_INT_STS;
#endif /* ERR_RECOVERY || CONFIG_FWOWN_SUPPORT */

	hif->ring_layout.tx_ring_layout = tx_ring_layout;
	hif->ring_layout.rx_ring_layout = rx_ring_layout;

	hif->tx_res_num = TX_RING_NUM;
	hif->rx_res_num = RX_RING_NUM;

	hif->isr = mt7915_isr;

#if defined(CTXD_SCATTER_AND_GATHER) || defined(CTXD_MEM_CPY)
	hif->max_ctxd_agg_num = 4; /* should also consider array size of ctxd_num in tr_counter */
#endif
#ifdef CTXD_MEM_CPY
	hif->ctxd_size_unit = LMAC_TXD_MAX_SIZE + sizeof(CR4_TXP_MSDU_INFO) + CT_PARSE_PAYLOAD_LEN;
	hif->ct_partial_payload_offset = LMAC_TXD_MAX_SIZE + sizeof(CR4_TXP_MSDU_INFO);

	if (hif->max_ctxd_agg_num > 1)
		hif->tx_dma_1st_buffer_size = hif->max_ctxd_agg_num * hif->ctxd_size_unit;
	else
#endif
		hif->tx_dma_1st_buffer_size = 192; /* TX_DMA_1ST_BUFFER_SIZE */
}

static VOID mt7916_hif_ctrl_init(struct pci_hif_chip *hif_chip)
{
	struct pci_hif_chip *hif = hif_chip;

	hif->int_ena_reg_addr = MT_INT1_MASK_CSR;
	hif->int_enable_mask = MT_INT1_CoherentInt | MT_INT1_TX_DONE | MT_INT1_RX;

	hif->ring_layout.tx_ring_layout = tx_ring_layout_pcie1;
	hif->ring_layout.rx_ring_layout = rx_ring_layout_pcie1;

	hif->tx_res_num = TX_RING_NUM_PCIE1;
	hif->rx_res_num = RX_RING_NUM_PCIE1;

	hif->isr = mt7916_isr;

#if defined(CTXD_SCATTER_AND_GATHER) || defined(CTXD_MEM_CPY)
	hif->max_ctxd_agg_num = 4;
#endif
#ifdef CTXD_MEM_CPY
	hif->ctxd_size_unit = LMAC_TXD_MAX_SIZE + sizeof(CR4_TXP_MSDU_INFO) + CT_PARSE_PAYLOAD_LEN;
	hif->ct_partial_payload_offset = LMAC_TXD_MAX_SIZE + sizeof(CR4_TXP_MSDU_INFO);

	if (hif->max_ctxd_agg_num > 1)
		hif->tx_dma_1st_buffer_size = hif->max_ctxd_agg_num * hif->ctxd_size_unit;
	else
#endif
		hif->tx_dma_1st_buffer_size = 192; /* TX_DMA_1ST_BUFFER_SIZE */
}

VOID mt7916_hif_init(VOID *chip_hif)
{
	UINT32 hif_id;
	struct pci_hif_chip *pci_hif = (struct pci_hif_chip *)chip_hif;

	mt7916_hif_ctrl_init(pci_hif);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<--%s()\n", __func__));
	hif_id = multi_hif_entry_id_get(pci_hif);
	/*write recognitation id for master/slave match
	*due to default RID is 0
	*start from 1 for RID assignment
	*/
	set_rid_value(pci_hif, hif_id + 1);
}


