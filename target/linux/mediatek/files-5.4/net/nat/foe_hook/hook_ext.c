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

struct net_device	*dst_port[MAX_IF_NUM];
EXPORT_SYMBOL(dst_port);
u8 dst_port_type[MAX_IF_NUM];
EXPORT_SYMBOL(dst_port_type);

struct foe_entry *ppe_virt_foe_base_tmp;
EXPORT_SYMBOL(ppe_virt_foe_base_tmp);
struct foe_entry *ppe1_virt_foe_base_tmp;
EXPORT_SYMBOL(ppe1_virt_foe_base_tmp);

int (*ppe_hook_rx_wifi)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(ppe_hook_rx_wifi);
int (*ppe_hook_tx_wifi)(struct sk_buff *skb, int gmac_no) = NULL;
EXPORT_SYMBOL(ppe_hook_tx_wifi);

int (*ppe_hook_rx_modem)(struct sk_buff *skb, u32 cpu_reason, u32 foe_entry_num) = NULL;
EXPORT_SYMBOL(ppe_hook_rx_modem);
int (*ppe_hook_tx_modem)(struct sk_buff *skb, u32 net_type, u32 channel_id) = NULL;
EXPORT_SYMBOL(ppe_hook_tx_modem);

int (*ppe_hook_rx_eth)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(ppe_hook_rx_eth);
int (*ppe_hook_tx_eth)(struct sk_buff *skb, int gmac_no) = NULL;
EXPORT_SYMBOL(ppe_hook_tx_eth);

int (*ppe_hook_rx_ext)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(ppe_hook_rx_ext);
int (*ppe_hook_tx_ext)(struct sk_buff *skb, int gmac_no) = NULL;
EXPORT_SYMBOL(ppe_hook_tx_ext);

void (*ppe_dev_register_hook)(struct net_device *dev) = NULL;
EXPORT_SYMBOL(ppe_dev_register_hook);
void (*ppe_dev_unregister_hook)(struct net_device *dev) = NULL;
EXPORT_SYMBOL(ppe_dev_unregister_hook);

void  hwnat_magic_tag_set_zero(struct sk_buff *skb)
{
	if ((FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_PCI) ||
	    (FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_WLAN) ||
	    (FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_GE)) {
		if (IS_SPACE_AVAILABLE_HEAD(skb))
			FOE_MAGIC_TAG_HEAD(skb) = 0;
	}
	if ((FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_PCI) ||
	    (FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_WLAN) ||
	    (FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_GE)) {
		if (IS_SPACE_AVAILABLE_TAIL(skb))
			FOE_MAGIC_TAG_TAIL(skb) = 0;
	}
}
EXPORT_SYMBOL(hwnat_magic_tag_set_zero);

void hwnat_check_magic_tag(struct sk_buff *skb)
{
	if (IS_SPACE_AVAILABLE_HEAD(skb)) {
		FOE_MAGIC_TAG_HEAD(skb) = 0;
		FOE_AI_HEAD(skb) = UN_HIT;
	}
	if (IS_SPACE_AVAILABLE_TAIL(skb)) {
		FOE_MAGIC_TAG_TAIL(skb) = 0;
		FOE_AI_TAIL(skb) = UN_HIT;
	}
}
EXPORT_SYMBOL(hwnat_check_magic_tag);

void hwnat_set_headroom_zero(struct sk_buff *skb)
{
	if (skb->cloned != 1) {
		if (IS_MAGIC_TAG_PROTECT_VALID_HEAD(skb) ||
		    (FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_PPE)) {
			if (IS_SPACE_AVAILABLE_HEAD(skb))
				memset(FOE_INFO_START_ADDR_HEAD(skb), 0,
				       FOE_INFO_LEN);
		}
	}
}
EXPORT_SYMBOL(hwnat_set_headroom_zero);

void hwnat_set_tailroom_zero(struct sk_buff *skb)
{
	if (skb->cloned != 1) {
		if (IS_MAGIC_TAG_PROTECT_VALID_TAIL(skb) ||
		    (FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_PPE)) {
			if (IS_SPACE_AVAILABLE_TAIL(skb))
				memset(FOE_INFO_START_ADDR_TAIL(skb), 0,
				       FOE_INFO_LEN);
		}
	}
}
EXPORT_SYMBOL(hwnat_set_tailroom_zero);

void hwnat_copy_headroom(u8 *data, struct sk_buff *skb)
{
	memcpy(data, skb->head, FOE_INFO_LEN);
}
EXPORT_SYMBOL(hwnat_copy_headroom);

void hwnat_copy_tailroom(u8 *data, int size, struct sk_buff *skb)
{
	memcpy((data + size - FOE_INFO_LEN),
	       (skb_end_pointer(skb) - FOE_INFO_LEN),
	       FOE_INFO_LEN);
}
EXPORT_SYMBOL(hwnat_copy_tailroom);

void hwnat_setup_dma_ops(struct device *dev, bool coherent)
{
	arch_setup_dma_ops(dev, 0, 0, NULL, coherent);
}
EXPORT_SYMBOL(hwnat_setup_dma_ops);

