/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Debug addons for NAND Mapped-block Management (NMBM)
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _NMBM_DEBUG_H_
#define _NMBM_DEBUG_H_

#define nmbm_mark_block_color_normal(ni, start_ba, end_ba)
#define nmbm_mark_block_color_bad(ni, ba)
#define nmbm_mark_block_color_mgmt(ni, start_ba, end_ba)
#define nmbm_mark_block_color_signature(ni, ba)
#define nmbm_mark_block_color_info_table(ni, start_ba, end_ba)
#define nmbm_mark_block_color_mapped(ni, ba)

#endif /* _NMBM_DEBUG_H_ */
