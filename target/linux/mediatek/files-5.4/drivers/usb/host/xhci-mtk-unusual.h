/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * xhci-mtk-unusuallib.h -- xhci toolkit header file
 *
 * Copyright (C) 2021 Mediatek Inc - http://www.mediatek.com
 *
 * Author: Zhanyong Wang <zhanyong.wang@mediatek.com>
 */

#ifndef __XHCI_MTK_UNUSUAL_H
#define __XHCI_MTK_UNUSUAL_H

#include <dt-bindings/phy/phy.h>

#define HQA_PREFIX_SIZE		4*1024

#define BIT_WIDTH_1		1
#define MSK_WIDTH_1		0x1
#define VAL_MAX_WDITH_1		0x1

#define STRNG_0_WIDTH_1		"0"
#define STRNG_1_WIDTH_1		"1"

#define BIT_WIDTH_2		2
#define MSK_WIDTH_2		0x3
#define VAL_MAX_WDITH_2		0x3
#define STRNG_0_WIDTH_2		"00"
#define STRNG_1_WIDTH_2		"01"
#define STRNG_2_WIDTH_2		"10"
#define STRNG_3_WIDTH_2		"11"


#define BIT_WIDTH_3		3
#define MSK_WIDTH_3		0x7
#define VAL_MAX_WDITH_3		0x7
#define STRNG_0_WIDTH_3		"000"
#define STRNG_1_WIDTH_3		"001"
#define STRNG_2_WIDTH_3		"010"
#define STRNG_3_WIDTH_3		"011"
#define STRNG_4_WIDTH_3		"100"
#define STRNG_5_WIDTH_3		"101"
#define STRNG_6_WIDTH_3		"110"
#define STRNG_7_WIDTH_3		"111"

#define BIT_WIDTH_4		4
#define MSK_WIDTH_4		0xf
#define VAL_MAX_WDITH_4		0xf
#define STRNG_0_WIDTH_4		"0000"
#define STRNG_1_WIDTH_4		"0001"
#define STRNG_2_WIDTH_4		"0010"
#define STRNG_3_WIDTH_4		"0011"
#define STRNG_4_WIDTH_4		"0100"
#define STRNG_5_WIDTH_4		"0101"
#define STRNG_6_WIDTH_4		"0110"
#define STRNG_7_WIDTH_4		"0111"
#define STRNG_8_WIDTH_4		"1000"
#define STRNG_9_WIDTH_4		"1001"
#define STRNG_A_WIDTH_4		"1010"
#define STRNG_B_WIDTH_4		"1011"
#define STRNG_C_WIDTH_4		"1100"
#define STRNG_D_WIDTH_4		"1101"
#define STRNG_E_WIDTH_4		"1110"
#define STRNG_F_WIDTH_4		"1111"

/* specific */
#define NAME_RG_USB20_INTR_EN		"RG_USB20_INTR_EN"
#define USB20_PHY_USBPHYACR0		0x00
#define SHFT_RG_USB20_INTR_EN		5
#define BV_RG_USB20_INTR_EN		BIT(5)

#define NAME_RG_USB20_VRT_VREF_SEL	"RG_USB20_VRT_VREF_SEL"
#define USB20_PHY_USBPHYACR1		0x04
#define SHFT_RG_USB20_VRT_VREF_SEL	12
#define BV_RG_USB20_VRT_VREF_SEL	GENMASK(14, 12)

#define NAME_RG_USB20_TERM_VREF_SEL	"RG_USB20_TERM_VREF_SEL"
#define SHFT_RG_USB20_TERM_VREF_SEL	8
#define BV_RG_USB20_TERM_VREF_SEL       GENMASK(10,  8)

#define NAME_RG_USB20_HSTX_SRCTRL	"RG_USB20_HSTX_SRCTRL"
#define USB20_PHY_USBPHYACR5		0x14
#define SHFT_RG_USB20_HSTX_SRCTRL	12
#define BV_RG_USB20_HSTX_SRCTRL		GENMASK(14, 12)

#define NAME_RG_USB20_DISCTH		"RG_USB20_DISCTH"
#define USB20_PHY_USBPHYACR6		0x18
#define SHFT_RG_USB20_DISCTH		4
#define BV_RG_USB20_DISCTH		GENMASK(8, 4)

#define NAME_RG_CHGDT_EN		"RG_CHGDT_EN"
#define USB20_PHY_U2PHYBC12C		0x80
#define SHFT_RG_CHGDT_EN		0
#define BV_RG_CHGDT_EN			BIT(0)

#define NAME_RG_USB20_PHY_REV		"RG_USB20_PHY_REV"
/* #define USB20_PHY_USBPHYACR6		0x18 */
#define SHFT_RG_USB20_PHY_REV		30
#define BV_RG_USB20_PHY_REV		GENMASK(31, 30)

#define ECHO_HQA(reg, _bd, _bw)  do {\
	val = usb20hqa_read(addr + (reg), \
		 SHFT_##_bd, \
		 BV_##_bd); \
	val = bin2str(val, BIT_WIDTH_##_bw, str); \
	cnt += sprintf(buf + cnt, "	%-22s = %ib%s\n", \
			NAME_##_bd, _bw, str); } while(0)


#ifdef CONFIG_USB_XHCI_MTK_DEBUGFS
static inline u32 usb20hqa_write(u32 __iomem *addr,
				u32 shift, u32 mask, u32 value)
{
	u32 val;

	val  = readl(addr);
	val &= ~((mask) << shift);
	val |=  (((value) & (mask)) << shift);
	writel(val, addr);

	return val;
}
static inline u32 usb20hqa_read(u32 __iomem *addr, u32 shift, u32 mask)
{
	u32 val;

	val   = readl(addr);
	val  &= mask;
	val >>=  shift;

	return val;
}

u32 binary_write_width1(u32 __iomem *addr,
				u32 shift, const char *buf);
u32 binary_write_width2(u32 __iomem *addr,
				u32 shift, const char *buf);
u32 binary_write_width3(u32 __iomem *addr,
				u32 shift, const char *buf);
u32 binary_write_width4(u32 __iomem *addr,
				u32 shift, const char *buf);
u32 bin2str(u32 value, u32 width, char *buffer);
int query_phy_addr(struct device_node *np, int *start,
				u32 *addr, u32 *length, int type);
int query_reg_addr(struct platform_device *pdev, u32 *addr,
				u32 *length, const char* name);

static inline int remaining(struct xhci_hcd_mtk *mtk)
{
	u32 surplus = 0;
	if (mtk && mtk->hqa_pos < mtk->hqa_size)
		surplus = mtk->hqa_size - mtk->hqa_pos;

	return surplus;
}

#define hqa_info(mtk, fmt, args...)  \
	(mtk)->hqa_pos += snprintf((mtk)->hqa_buf + (mtk)->hqa_pos, \
		remaining(mtk), fmt, ## args)

#define DEVICE_ATTR_DECLARED(_name) \
		extern struct device_attribute dev_attr_##_name;
#define UNUSUAL_DEVICE_ATTR(_name)  &dev_attr_##_name
#else
static inline u32 usb20hqa_write(u32 __iomem *addr,
					u32 shift, u32 mask, u32 value)
{
	return 0;
}
static inline u32 usb20hqa_read(u32 __iomem *addr, u32 shift, u32 mask)
{
	return 0;
}
static inline u32 binary_write_width1(u32 __iomem *addr,
					u32 shift, const char *buf)
{
	return 0;
};
static inline u32 binary_write_width2(u32 __iomem *addr,
					u32 shift, const char *buf)
{
	return 0;
};
static inline u32 binary_write_width3(u32 __iomem *addr,
					u32 shift, const char *buf)
{
	return 0;
};
static inline u32 binary_write_width4(u32 __iomem *addr,
					u32 shift, const char *buf)
{
	return 0;
};
static inline u32 bin2str(u32 value, u32 width, char *buffer)
{
	return 0;
};
static inline int query_phy_addr(struct device_node *np, int *start,
					u32 *addr, u32 *length, int type)
{
	return -EPERM;
}
static inline int query_reg_addr(struct platform_device *pdev, u32 *addr,
					u32 *length, const char* name)
{
	return -EPERM;
}
static inline int remaining(int wrote)
{
	return 0;
}
#define hqa_info(mtk, fmt, args...)
#define DEVICE_ATTR_DECLARED(...)
#endif

#include "unusual-declaration.h"

#endif /* __XHCI_MTK_UNUSUAL_H */
