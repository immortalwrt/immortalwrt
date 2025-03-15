/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Definitions for NAND Mapped-block Management (NMBM)
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _NMBM_PRIVATE_H_
#define _NMBM_PRIVATE_H_

#include <nmbm/nmbm.h>

#define NMBM_MAGIC_SIGNATURE			0x304d4d4e	/* NMM0 */
#define NMBM_MAGIC_INFO_TABLE			0x314d4d4e	/* NMM1 */

#define NMBM_VERSION_MAJOR_S			0
#define NMBM_VERSION_MAJOR_M			0xffff
#define NMBM_VERSION_MINOR_S			16
#define NMBM_VERSION_MINOR_M			0xffff
#define NMBM_VERSION_MAKE(major, minor)		(((major) & NMBM_VERSION_MAJOR_M) | \
						(((minor) & NMBM_VERSION_MINOR_M) << \
						NMBM_VERSION_MINOR_S))
#define NMBM_VERSION_MAJOR_GET(ver)		(((ver) >> NMBM_VERSION_MAJOR_S) & \
						NMBM_VERSION_MAJOR_M)
#define NMBM_VERSION_MINOR_GET(ver)		(((ver) >> NMBM_VERSION_MINOR_S) & \
						NMBM_VERSION_MINOR_M)

typedef uint32_t				nmbm_bitmap_t;
#define NMBM_BITMAP_UNIT_SIZE			(sizeof(nmbm_bitmap_t))
#define NMBM_BITMAP_BITS_PER_BLOCK		2
#define NMBM_BITMAP_BITS_PER_UNIT		(8 * sizeof(nmbm_bitmap_t))
#define NMBM_BITMAP_BLOCKS_PER_UNIT		(NMBM_BITMAP_BITS_PER_UNIT / \
						 NMBM_BITMAP_BITS_PER_BLOCK)

#define NMBM_SPARE_BLOCK_MULTI			1
#define NMBM_SPARE_BLOCK_DIV			2
#define NMBM_SPARE_BLOCK_MIN			2

#define NMBM_MGMT_DIV				16
#define NMBM_MGMT_BLOCKS_MIN			32

#define NMBM_TRY_COUNT				3

#define BLOCK_ST_BAD				0
#define BLOCK_ST_NEED_REMAP			2
#define BLOCK_ST_GOOD				3
#define BLOCK_ST_MASK				3

struct nmbm_header {
	uint32_t magic;
	uint32_t version;
	uint32_t size;
	uint32_t checksum;
};

struct nmbm_signature {
	struct nmbm_header header;
	uint64_t nand_size;
	uint32_t block_size;
	uint32_t page_size;
	uint32_t spare_size;
	uint32_t mgmt_start_pb;
	uint8_t max_try_count;
	uint8_t padding[3];
};

struct nmbm_info_table_header {
	struct nmbm_header header;
	uint32_t write_count;
	uint32_t state_table_off;
	uint32_t mapping_table_off;
	uint32_t padding;
};

struct nmbm_instance {
	struct nmbm_lower_device lower;

	uint32_t rawpage_size;
	uint32_t rawblock_size;
	uint32_t rawchip_size;

	uint32_t writesize_mask;
	uint32_t erasesize_mask;
	uint16_t writesize_shift;
	uint16_t erasesize_shift;

	struct nmbm_signature signature;

	uint8_t *info_table_cache;
	uint32_t info_table_size;
	uint32_t info_table_spare_blocks;
	struct nmbm_info_table_header info_table;

	nmbm_bitmap_t *block_state;
	uint32_t block_state_changed;
	uint32_t state_table_size;

	int32_t *block_mapping;
	uint32_t block_mapping_changed;
	uint32_t mapping_table_size;

	uint8_t *page_cache;

	int protected;

	uint32_t block_count;
	uint32_t data_block_count;

	uint32_t mgmt_start_ba;
	uint32_t main_table_ba;
	uint32_t backup_table_ba;
	uint32_t mapping_blocks_ba;
	uint32_t mapping_blocks_top_ba;
	uint32_t signature_ba;

	enum nmbm_log_category log_display_level;
};

/* Log utilities */
#define nlog_debug(ni, fmt, ...) \
	nmbm_log(ni, NMBM_LOG_DEBUG, fmt, ##__VA_ARGS__)

#define nlog_info(ni, fmt, ...) \
	nmbm_log(ni, NMBM_LOG_INFO, fmt, ##__VA_ARGS__)

#define nlog_warn(ni, fmt, ...) \
	nmbm_log(ni, NMBM_LOG_WARN, fmt, ##__VA_ARGS__)

#define nlog_err(ni, fmt, ...) \
	nmbm_log(ni, NMBM_LOG_ERR, fmt, ##__VA_ARGS__)

#define nlog_emerg(ni, fmt, ...) \
	nmbm_log(ni, NMBM_LOG_EMERG, fmt, ##__VA_ARGS__)

#endif /* _NMBM_PRIVATE_H_ */
