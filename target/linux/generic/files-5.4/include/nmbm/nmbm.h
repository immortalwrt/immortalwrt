/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Definitions for NAND Mapped-block Management (NMBM)
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _NMBM_H_
#define _NMBM_H_

#include <nmbm/nmbm-os.h>

enum nmbm_log_category {
	NMBM_LOG_DEBUG,
	NMBM_LOG_INFO,
	NMBM_LOG_WARN,
	NMBM_LOG_ERR,
	NMBM_LOG_EMERG,

	__NMBM_LOG_MAX
};

enum nmbm_oob_mode {
	NMBM_MODE_PLACE_OOB,
	NMBM_MODE_AUTO_OOB,
	NMBM_MODE_RAW,

	__NMBM_MODE_MAX
};

struct nmbm_lower_device {
	uint32_t max_ratio;
	uint32_t max_reserved_blocks;
	int flags;

	uint64_t size;
	uint32_t erasesize;
	uint32_t writesize;
	uint32_t oobsize;
	uint32_t oobavail;

	void *arg;
	int (*reset_chip)(void *arg);

	/*
	 * read_page:
	 *    return 0 if succeeds
	 *    return positive number for ecc error
	 *    return negative number for other errors
	 */
	int (*read_page)(void *arg, uint64_t addr, void *buf, void *oob, enum nmbm_oob_mode mode);
	int (*write_page)(void *arg, uint64_t addr, const void *buf, const void *oob, enum nmbm_oob_mode mode);
	int (*erase_block)(void *arg, uint64_t addr);

	int (*is_bad_block)(void *arg, uint64_t addr);
	int (*mark_bad_block)(void *arg, uint64_t addr);

	/* OS-dependent logging function */
	void (*logprint)(void *arg, enum nmbm_log_category level, const char *fmt, va_list ap);
};

struct nmbm_instance;

/* Create NMBM if management area not found, or not complete */
#define NMBM_F_CREATE			0x01

/* Empty page is also protected by ECC, and bitflip(s) can be corrected */
#define NMBM_F_EMPTY_PAGE_ECC_OK	0x02

/* Do not write anything back to flash */
#define NMBM_F_READ_ONLY		0x04

size_t nmbm_calc_structure_size(struct nmbm_lower_device *nld);
int nmbm_attach(struct nmbm_lower_device *nld, struct nmbm_instance *ni);
int nmbm_detach(struct nmbm_instance *ni);

enum nmbm_log_category nmbm_set_log_level(struct nmbm_instance *ni,
					  enum nmbm_log_category level);

int nmbm_erase_block_range(struct nmbm_instance *ni, uint64_t addr,
			   uint64_t size, uint64_t *failed_addr);
int nmbm_read_single_page(struct nmbm_instance *ni, uint64_t addr, void *data,
			  void *oob, enum nmbm_oob_mode mode);
int nmbm_read_range(struct nmbm_instance *ni, uint64_t addr, size_t size,
		    void *data, enum nmbm_oob_mode mode, size_t *retlen);
int nmbm_write_single_page(struct nmbm_instance *ni, uint64_t addr,
			   const void *data, const void *oob,
			   enum nmbm_oob_mode mode);
int nmbm_write_range(struct nmbm_instance *ni, uint64_t addr, size_t size,
		     const void *data, enum nmbm_oob_mode mode,
		     size_t *retlen);

int nmbm_check_bad_block(struct nmbm_instance *ni, uint64_t addr);
int nmbm_mark_bad_block(struct nmbm_instance *ni, uint64_t addr);

uint64_t nmbm_get_avail_size(struct nmbm_instance *ni);

int nmbm_get_lower_device(struct nmbm_instance *ni, struct nmbm_lower_device *nld);

#endif /* _NMBM_H_ */
