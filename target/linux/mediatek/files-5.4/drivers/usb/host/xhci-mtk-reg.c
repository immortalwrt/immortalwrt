// SPDX-License-Identifier: GPL-2.0
/*
 * xHCI host controller toolkit driver for intr-en
 *
 * Copyright (C) 2021  MediaTek Inc.
 *
 *  Author: Zhanyong Wang <zhanyong.wang@mediatek.com>
 */


#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include "xhci-mtk.h"
#include "xhci-mtk-test.h"
#include "xhci-mtk-unusual.h"

#define REGS_LIMIT_XHCI 0x1000
#define REGS_LIMIT_MU3D 0x2e00
static ssize_t reg_show(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	ssize_t cnt = 0;

	cnt += sprintf(buf + cnt,
		"SSUSB register operation interface help info.\n"
		"  rx - read xhci  reg: offset [len]\n"
		"  rm - read mu3d  reg: offset [len]\n"
		"  ri - read ippc  reg: offset [len]\n"
		"  rp - read phy   reg: offset [len]\n"
		"  wx - write xhci reg: offset value\n"
		"  wm - write mu3d reg: offset value\n"
		"  wi - write ippc reg: offset value\n"
		"  wp - write phy  reg: offset value\n"
		"  sx - set xhci mac reg bits: offset bit_start mask value\n"
		"  sm - set mu3d mac reg bits: offset bit_start mask value\n"
		"  si - set ippc     reg bits: offset bit_start mask value\n"
		"  sp - set phy      reg bits: offset bit_start mask value\n"
		"  px - print xhci mac reg bits: offset bit_start mask\n"
		"  pm - print mu3d mac reg bits: offset bit_start mask\n"
		"  pi - print ippc     reg bits: offset bit_start mask\n"
		"  pp - print phy      reg bits: offset bit_start mask\n"
		"  NOTE: numbers should be HEX, except bit_star(DEC)\n");

	if (mtk->hqa_pos) {
		cnt += sprintf(buf + cnt, "%s", mtk->hqa_buf);
		mtk->hqa_pos = 0;
	}

	return cnt;
}

/* base address: return value; limit is put into @limit */
static void __iomem *get_reg_base_limit(struct xhci_hcd_mtk *mtk,
					const char *buf, u32 *limit)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	struct platform_device *device = to_platform_device(mtk->dev);
	void __iomem *base = NULL;
	struct device_node  *node = mtk->dev->of_node;
	u32 io     = 0;
	u32 range  = 0;
	u32 len    = 0;
	int index  = 0;
	int ret    = 0;

	switch (buf[1]) {
	case 'x':
		ret = query_reg_addr(device, &io, &range, "mac");
		if (ret) break;

		base = ioremap(io, range);

		xhci_info(xhci, "xhci's reg: [0x%08X ~ 0x%08X]\n",
			  io, io + range);
		hqa_info (mtk,  "xhci's reg: [0x%08X ~ 0x%08X]\n",
			  io, io + range);
		break;
	case 'm':
		if (!mtk->has_ippc)
			device = to_platform_device(device->dev.parent);

		ret = query_reg_addr(device, &io, &range, "mac");
		if (ret) break;

		if (mtk->has_ippc) {
			io   += REGS_LIMIT_XHCI;
			range = REGS_LIMIT_MU3D;
		}

		base = ioremap(io, range);
                xhci_info(xhci, "mu3d's reg: [0x%08X ~ 0x%08X]\n",
			  io, io + range);
                hqa_info (mtk,  "mu3d's reg: [0x%08X ~ 0x%08X]\n",
			  io, io + range);
		break;
	case 'i':
		ret = query_reg_addr(device, &io, &range, "ippc");
		if (ret) break;

		base = ioremap(io, range);
		xhci_info(xhci, "ippc's reg: [0x%08X ~ 0x%08X]\n",
			  io, io + range);
		hqa_info (mtk,  "ippc's reg: [0x%08X ~ 0x%08X]\n",
			  io, io + range);
		break;
	case 'p':
		ret = query_phy_addr(node, &index, &io, &len, PHY_TYPE_USB3);
		if (ret && ret != -EACCES) break;

		range  = io & 0x0000FFFF;
		range += len;

		io &= 0xFFFF0000;

		base = ioremap(io, range);
		xhci_info(xhci, "phy's reg: [0x%08X ~ 0x%08X]\n",
			      io, io + range);
		hqa_info (mtk,  "phy's reg: [0x%08X ~ 0x%08X]\n",
			  io, io + range);
		break;
	default:
		base = NULL;
	}

	*limit = range;

	return base;
}

static void ssusb_write_reg(struct xhci_hcd_mtk *mtk, const char *buf)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	void __iomem *base;
	u32 offset = 0;
	u32 value = 0;
	u32 old_val = 0;
	u32 limit = 0;
	u32 param;

	param = sscanf(buf, "%*s 0x%x 0x%x", &offset, &value);
	xhci_info(xhci, "params-%d (offset: %#x, value: %#x)\n",
		  param, offset, value);
	hqa_info (mtk,  "params-%d (offset: %#x, value: %#x)\n",
		  param, offset, value);

	base = get_reg_base_limit(mtk, buf, &limit);
	if (!base || (param != 2)) {
		xhci_err(xhci, "params are invalid!\n");
		hqa_info(mtk,  "params are invalid since %p, %u!\n",
			 base, param);
		return;
	}

	offset &= ~0x3;  /* 4-bytes align */
	if (offset >= limit) {
		xhci_err(xhci, "reg's offset overrun!\n");
		hqa_info(mtk,  "reg's offset overrun since %u >= %u!\n",
			 offset, limit);
		return;
	}
	old_val = readl(base + offset);
	writel(value, base + offset);
	xhci_info(xhci, "0x%8.8x : 0x%8.8x --> 0x%8.8x\n", offset, old_val,
		  readl(base + offset));
	hqa_info (mtk,  "0x%8.8x : 0x%8.8x --> 0x%8.8x\n", offset, old_val,
		  readl(base + offset));

	base = (void __iomem *)((unsigned long)base & 0xFFFF0000);
	iounmap(base);
}

static void read_single_reg(struct xhci_hcd_mtk *mtk,
			void __iomem *base, u32 offset, u32 limit)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	u32 value;

	offset &= ~0x3;  /* 4-bytes align */
	if (offset >= limit) {
		xhci_err(xhci, "reg's offset overrun!\n");
		hqa_info(mtk,  "reg's offset overrun since %u >= %u!\n",
			 offset, limit);
		return;
	}
	value = readl(base + offset);
	xhci_err(xhci, "0x%8.8x : 0x%8.8x\n", offset, value);
	hqa_info(mtk,  "0x%8.8x : 0x%8.8x\n", offset, value);
}

static void read_multi_regs(struct xhci_hcd_mtk *mtk,
			    void __iomem *base, u32 offset, u32 len, u32 limit)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	int i;

	/* at least 4 ints */
	offset &= ~0xF;
	len = (len + 0x3) & ~0x3;

	if (offset + len > limit) {
		xhci_err(xhci, "reg's offset overrun!\n");
		hqa_info(mtk,  "reg's offset overrun since %u > %u!\n",
			 offset + len, limit);
		return;
	}

	len >>= 2;
	xhci_info(xhci, "read regs [%#x, %#x)\n", offset, offset + (len << 4));
	hqa_info (mtk,  "read regs [%#x, %#x)\n", offset, offset + (len << 4));
	for (i = 0; i < len; i++) {
		xhci_err(xhci, "0x%8.8x : 0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x\n",
			offset, readl(base + offset),
			readl(base + offset + 0x4),
			readl(base + offset + 0x8),
			readl(base + offset + 0xc));
		hqa_info(mtk,  "0x%8.8x : 0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x\n",
			 offset, readl(base + offset),
			 readl(base + offset + 0x4),
			 readl(base + offset + 0x8),
			 readl(base + offset + 0xc));
		offset += 0x10;
	}
}

static void ssusb_read_regs(struct xhci_hcd_mtk *mtk, const char *buf)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	void __iomem *base;
	u32 offset = 0;
	u32 len = 0;
	u32 limit = 0;
	u32 param;

	param = sscanf(buf, "%*s 0x%x 0x%x", &offset, &len);
	xhci_info(xhci, "params-%d (offset: %#x, len: %#x)\n",
		  param, offset, len);
	hqa_info (mtk,  "params-%d (offset: %#x, len: %#x)\n",
		 param, offset, len);

	base = get_reg_base_limit(mtk, buf, &limit);
	if (!base || !param) {
		xhci_err(xhci, "params are invalid!\n");
		hqa_info(mtk,  "params are invalid since %p, %u!\n",
			 base, param);
		return;
	}

	if (param == 1)
		read_single_reg(mtk, base, offset, limit);
	else
		read_multi_regs(mtk, base, offset, len, limit);

	base = (void __iomem *)((unsigned long)base & 0xFFFF0000);
	iounmap(base);
}

static void ssusb_set_reg_bits(struct xhci_hcd_mtk *mtk, const char *buf)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	void __iomem *base;
	u32 offset = 0;
	u32 bit_start = 0;
	u32 mask = 0;
	u32 value = 0;
	u32 old_val = 0;
	u32 new_val = 0;
	u32 limit = 0;
	u32 param;

	param = sscanf(buf, "%*s 0x%x %d 0x%x 0x%x",
		       &offset, &bit_start, &mask, &value);
	xhci_info(xhci, "params-%d (offset:%#x,bit_start:%d,mask:%#x,value:%#x)\n",
		  param, offset, bit_start, mask, value);
	hqa_info(mtk,  "params-%d (offset:%#x,bit_start:%d,mask:%#x,value:%#x)\n",
		 param, offset, bit_start, mask, value);

	base = get_reg_base_limit(mtk, buf, &limit);
	if (!base || (param != 4) || (bit_start > 31)) {
		xhci_err(xhci, "params are invalid!\n");
		hqa_info(mtk,  "params are invalid since %p, %u, %u\n",
			 base, param, bit_start);
		return;
	}

	offset &= ~0x3;  /* 4-bytes align */
	if (offset >= limit) {
		xhci_err(xhci, "reg's offset overrun!\n");
		hqa_info(mtk,  "reg's offset overrun since %u >= %u!\n",
			 offset, limit);
		return;
	}
	old_val = readl(base + offset);
	new_val = old_val;
	new_val &= ~(mask << bit_start);
	new_val |= (value << bit_start);
	writel(new_val, base + offset);
	xhci_info(xhci, "0x%8.8x : 0x%8.8x --> 0x%8.8x\n", offset, old_val,
		  readl(base + offset));
	hqa_info (mtk,  "0x%8.8x : 0x%8.8x --> 0x%8.8x\n", offset, old_val,
		 readl(base + offset));

	base = (void __iomem *)((unsigned long)base & 0xFFFF0000);
	iounmap(base);
}

static void ssusb_print_reg_bits(struct xhci_hcd_mtk *mtk, const char *buf)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	void __iomem *base;
	u32 offset = 0;
	u32 bit_start = 0;
	u32 mask = 0;
	u32 old_val = 0;
	u32 new_val = 0;
	u32 limit = 0;
	u32 param;

	param = sscanf(buf, "%*s 0x%x %d 0x%x", &offset, &bit_start, &mask);
	xhci_info(xhci, "params-%d (offset: %#x, bit_start: %d, mask: %#x)\n",
		param, offset, bit_start, mask);
	hqa_info (mtk,  "params-%d (offset: %#x, bit_start: %d, mask: %#x)\n",
		param, offset, bit_start, mask);

	base = get_reg_base_limit(mtk, buf, &limit);
	if (!base || (param != 3) || (bit_start > 31)) {
		xhci_err(xhci, "params are invalid!\n");
		hqa_info(mtk,  "params are invalid since %p, %u, %u\n",
			 base, param, bit_start);
		return;
	}

	offset &= ~0x3;  /* 4-bytes align */
	if (offset >= limit) {
		xhci_err(xhci, "reg's offset overrun!\n");
		hqa_info(mtk,  "reg's offset overrun since %u >= %u!\n",
			 offset, limit);
		return;
	}

	old_val = readl(base + offset);
	new_val = old_val;
	new_val >>= bit_start;
	new_val &= mask;
	xhci_info(xhci, "0x%8.8x : 0x%8.8x (0x%x)\n", offset, old_val, new_val);
	hqa_info (mtk,  "0x%8.8x : 0x%8.8x (0x%x)\n", offset, old_val, new_val);

	base = (void __iomem *)((unsigned long)base & 0xFFFF0000);
	iounmap(base);
}

static ssize_t
reg_store(struct device *dev, struct device_attribute *attr,
	  const char *buf, size_t n)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);

	xhci_info(xhci, "cmd:%s\n", buf);
	hqa_info (mtk,  "cmd:%s\n", buf);

	switch (buf[0]) {
	case 'w':
		ssusb_write_reg(mtk, buf);
		break;
	case 'r':
		ssusb_read_regs(mtk, buf);
		break;
	case 's':
		ssusb_set_reg_bits(mtk, buf);
		break;
	case 'p':
		ssusb_print_reg_bits(mtk, buf);
		break;
	default:
		xhci_err(xhci, "No such cmd\n");
		hqa_info(mtk,  "No such cmd\n");
	}

	return n;
}
DEVICE_ATTR_RW(reg);
