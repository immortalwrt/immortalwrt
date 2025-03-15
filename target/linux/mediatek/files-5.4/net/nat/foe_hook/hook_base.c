/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2019 MediaTek Inc.
 * Author: Harry Huang <harry.huang@mediatek.com>
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <net/ra_nat.h>
#define PURPOSE "FAST_NAT_SUPPORT"

int (*ra_sw_nat_hook_rx)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_rx);

int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_tx);
