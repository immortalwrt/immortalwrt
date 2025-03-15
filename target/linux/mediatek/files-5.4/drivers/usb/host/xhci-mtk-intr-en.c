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

static ssize_t RG_USB20_INTR_EN_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	struct device_node  *node = dev->of_node;
	ssize_t cnt = 0;
	void __iomem *addr;
	u32 val;
	u32 i;
	int ports;
	char str[32];
	int index = 0;
	u32 io, length;
	int ret;

	ports = mtk->num_u3_ports + mtk->num_u2_ports;
	cnt += sprintf(buf + cnt, " RG_USB20_INTR_EN usage:\n");
	cnt += sprintf(buf + cnt,
                "   echo u2p index 1b0 > RG_USB20_INTR_EN\n");
	if (mtk->num_u3_ports + 1 != ports)
		cnt += sprintf(buf + cnt, "	parameter: u2p: %i ~ %i\n",
					mtk->num_u3_ports + 1, ports);
	else
		cnt += sprintf(buf + cnt, "	parameter: u2p: %i\n",
					mtk->num_u3_ports + 1);

	if (mtk->num_u2_ports > 1)
		cnt += sprintf(buf + cnt, "	parameter: index: 0 ~ %i\n",
			       mtk->num_u2_ports);
	else
		cnt += sprintf(buf + cnt, "	parameter: index: 0\n");

	cnt += sprintf(buf + cnt, " e.g.: echo 2 0 1b1 > RG_USB20_INTR_EN\n");
	cnt += sprintf(buf + cnt,
		"  port2 binding phy 0, enable 1b'1 as INTR_EN\n");

	cnt += sprintf(buf + cnt,
			"\n=========current HQA setting check=========\n");
	for (i = 1; i <= ports; i++) {
		addr = &xhci->op_regs->port_status_base +
			NUM_PORT_REGS * ((i - 1) & 0xff);
		val = readl(addr);
		if (i <= mtk->num_u3_ports) {
			cnt += sprintf(buf + cnt,
				       "USB30 Port%i: 0x%08X\n", i, val);
		} else {
			cnt += sprintf(buf + cnt,
				       "USB20 Port%i: 0x%08X\n", i, val);

			ret = query_phy_addr(node,
					 &index, &io, &length, PHY_TYPE_USB2);
			if (ret && ret != -EACCES) {
				if (ret == -EPERM)
					cnt += sprintf(buf + cnt,
					"USB20 Port%i (Phy%i: absent)\n",
					i, index);
				else
					cnt += sprintf(buf + cnt,
					"USB20 Port%i (Phy%i) failure %i\n",
						 i, index, ret);
				continue;
			}

			cnt += sprintf(buf + cnt,
				"USB20 Port%i (Phy%i:%sable): 0x%08X 0x%08X\n",
				i, index, ret ? " dis" : " en", io, length);

			addr   = ioremap_nocache(io, length);
			addr  += (length != 0x100) ? 0x300 : 0;

			HQA_INFORMACTION_COLLECTS();

			iounmap(addr);
			index ++;
		}
	}

	if (mtk->hqa_pos) {
		cnt += sprintf(buf + cnt, "%s", mtk->hqa_buf);
		mtk->hqa_pos = 0;
	}

	return cnt;
}

static ssize_t RG_USB20_INTR_EN_store(struct device *dev,
                        struct device_attribute *attr,
                        const char *buf, size_t n)
{
	u32 val;
	u32 io;
	u32 length;
	int ports;
	int words;
	int port;
	int index;
	int ret;
	char *str = NULL;
	void __iomem *addr;
	struct xhci_hcd_mtk *mtk  = dev_get_drvdata(dev);
	struct device_node  *node = dev->of_node;

	ports = mtk->num_u3_ports + mtk->num_u2_ports;
	mtk->hqa_pos = 0;

	memset(mtk->hqa_buf, 0, mtk->hqa_size);

	str = kzalloc(n, GFP_ATOMIC);

	hqa_info(mtk, "RG_USB20_INTR_EN(%lu): %s\n", n, buf);

	words = sscanf(buf, "%i %i 1b%1[0,1]", &port, &index, str);
	if ((words != 3) ||
	    (port < mtk->num_u3_ports || port > ports)) {
		hqa_info(mtk, "Check params(%i):\" %i %i %s\", Please!\n",
			words, port, index, str);

		ret = -EINVAL;
		goto error;
	}

	hqa_info(mtk, " params: %i %i %s\n",
		port, index, str);

	ret = query_phy_addr(node, &index, &io, &length, PHY_TYPE_USB2);
	if (ret && ret != -EACCES)
		goto error;

	io += (length != 0x100) ? 0x300 : 0;
	io += USB20_PHY_USBPHYACR0;

	addr = ioremap_nocache(io, 4);
	val = binary_write_width1(addr, SHFT_RG_USB20_INTR_EN, str);
	hqa_info(mtk, "Port%i(Phy%i)[0x%08X]: 0x%08X but 0x%08X\n",
		port, index, io, val, readl(addr));

	iounmap(addr);
	ret = n;

error:
	kfree(str);
	return ret;
}
DEVICE_ATTR_RW(RG_USB20_INTR_EN);
