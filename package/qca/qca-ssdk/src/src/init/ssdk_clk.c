/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "sw.h"
#include "ssdk_init.h"
#include "ssdk_plat.h"
#include "ssdk_clk.h"
#include "fal.h"
#include <linux/kconfig.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
#include <linux/of.h>
#include <linux/reset.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#endif

#ifdef HAWKEYE_CHIP
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
struct device_node *clock_node = NULL;
static struct clk *uniphy_port_clks[UNIPHYT_CLK_MAX] = {0};

struct device_node *rst_node = NULL;
struct reset_control *uniphy_rsts[UNIPHY_RST_MAX] = {0};

/* below 3 routines to be used as common */
void ssdk_clock_rate_set_and_enable(
	struct device_node *node, a_uint8_t* clock_id, a_uint32_t rate)
{
	struct clk *clk;

	clk = of_clk_get_by_name(node, clock_id);
	if (!IS_ERR(clk)) {
		if (rate)
			clk_set_rate(clk, rate);

		clk_prepare_enable(clk);
	}
}

void ssdk_gcc_reset(struct reset_control *rst, a_uint32_t action)
{
	if (action == SSDK_RESET_ASSERT)
		reset_control_assert(rst);
	else
		reset_control_deassert(rst);

}
#endif

void ssdk_uniphy_reset(
	a_uint32_t dev_id,
	enum unphy_rst_type rst_type,
	a_uint32_t action)
{
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
	struct reset_control *rst;

	rst = uniphy_rsts[rst_type];
	if (IS_ERR(rst)) {
		SSDK_ERROR("reset(%d) nof exist!\n", rst_type);
		return;
	}

	ssdk_gcc_reset(rst, action);
#endif

}

void ssdk_uniphy_clock_rate_set(
	a_uint32_t dev_id,
	enum unphy_clk_type clock_type,
	a_uint32_t rate)
{
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
	struct clk *uniphy_clk;

	uniphy_clk = uniphy_port_clks[clock_type];
	if (!IS_ERR(uniphy_clk)) {
		if (rate)
			if (clk_set_rate(uniphy_clk, rate))
				SSDK_INFO("%d set rate=%d fail\n", clock_type, rate);
	} else
		SSDK_INFO("%d set rate %x fail!\n", clock_type, rate);
#endif

}

void ssdk_uniphy_clock_enable(
	a_uint32_t dev_id,
	enum unphy_clk_type clock_type,
	a_bool_t enable)
{
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
	struct clk *uniphy_clk;

	uniphy_clk = uniphy_port_clks[clock_type];
	if (!IS_ERR(uniphy_clk)) {
		if (enable) {
			if (clk_prepare_enable(uniphy_clk))
				SSDK_ERROR("clock enable fail!\n");
		} else
			clk_disable_unprepare(uniphy_clk);
	} else {
		SSDK_ERROR("clock_type= %d enable=%d not find\n",
				clock_type, enable);
	}
#endif

}

/* below special for ppe */
#if defined(HPPE)

#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
struct clk_uniphy {
	struct clk_hw hw;
	u8 uniphy_index;
	u8 dir;
	unsigned long rate;
};

#define to_clk_uniphy(_hw) container_of(_hw, struct clk_uniphy, hw)

static unsigned long
uniphy_clks_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct clk_uniphy *uniphy = to_clk_uniphy(hw);

	return uniphy->rate;
}

static int
uniphy_clks_determine_rate(struct clk_hw *hw, struct clk_rate_request *req)
{
	/* add logic for checking the current mode */
	if (req->rate <= UNIPHY_CLK_RATE_125M)
		req->rate = UNIPHY_CLK_RATE_125M;
	else
		req->rate = UNIPHY_CLK_RATE_312M;

	return 0;
}

static int
uniphy_clks_set_rate(struct clk_hw *hw, unsigned long rate,
		     unsigned long parent_rate)
{
	struct clk_uniphy *uniphy = to_clk_uniphy(hw);

	if (rate != UNIPHY_CLK_RATE_125M && rate != UNIPHY_CLK_RATE_312M)
		return -1;

	uniphy->rate = rate;

	return 0;
}

static const struct clk_ops clk_uniphy_ops = {
	.recalc_rate = uniphy_clks_recalc_rate,
	.determine_rate = uniphy_clks_determine_rate,
	.set_rate = uniphy_clks_set_rate,
};

static struct clk_uniphy uniphy0_gcc_rx_clk = {
                .hw.init = &(struct clk_init_data){
                        .name = "uniphy0_gcc_rx_clk",
                        .ops = &clk_uniphy_ops,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
			.flags = CLK_IS_ROOT,
#endif
                },
		.uniphy_index = 0,
		.dir = UNIPHY_RX,
		.rate = UNIPHY_DEFAULT_RATE,
};

static struct clk_uniphy uniphy0_gcc_tx_clk = {
                .hw.init = &(struct clk_init_data){
                        .name = "uniphy0_gcc_tx_clk",
                        .ops = &clk_uniphy_ops,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
			.flags = CLK_IS_ROOT,
#endif
                },
		.uniphy_index = 0,
		.dir = UNIPHY_TX,
		.rate = UNIPHY_DEFAULT_RATE,
};

static struct clk_uniphy uniphy1_gcc_rx_clk = {
                .hw.init = &(struct clk_init_data){
                        .name = "uniphy1_gcc_rx_clk",
                        .ops = &clk_uniphy_ops,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
			.flags = CLK_IS_ROOT,
#endif
                },
		.uniphy_index = 1,
		.dir = UNIPHY_RX,
		.rate = UNIPHY_DEFAULT_RATE,
};

static struct clk_uniphy uniphy1_gcc_tx_clk = {
                .hw.init = &(struct clk_init_data){
                        .name = "uniphy1_gcc_tx_clk",
                        .ops = &clk_uniphy_ops,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
			.flags = CLK_IS_ROOT,
#endif
                },
		.uniphy_index = 1,
		.dir = UNIPHY_TX,
		.rate = UNIPHY_DEFAULT_RATE,
};

static struct clk_uniphy uniphy2_gcc_rx_clk = {
                .hw.init = &(struct clk_init_data){
                        .name = "uniphy2_gcc_rx_clk",
                        .ops = &clk_uniphy_ops,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
			.flags = CLK_IS_ROOT,
#endif
                },
		.uniphy_index = 2,
		.dir = UNIPHY_RX,
		.rate = UNIPHY_DEFAULT_RATE,
};

static struct clk_uniphy uniphy2_gcc_tx_clk = {
                .hw.init = &(struct clk_init_data){
                        .name = "uniphy2_gcc_tx_clk",
                        .ops = &clk_uniphy_ops,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
			.flags = CLK_IS_ROOT,
#endif
                },
		.uniphy_index = 2,
		.dir = UNIPHY_TX,
		.rate = UNIPHY_DEFAULT_RATE,
};

static struct clk_hw *uniphy_raw_clks[SSDK_MAX_UNIPHY_INSTANCE * 2] = {
	&uniphy0_gcc_rx_clk.hw, &uniphy0_gcc_tx_clk.hw,
	&uniphy1_gcc_rx_clk.hw, &uniphy1_gcc_tx_clk.hw,
	&uniphy2_gcc_rx_clk.hw, &uniphy2_gcc_tx_clk.hw,
};

static char *ppe_clk_ids[UNIPHYT_CLK_MAX] = {
	NSS_PORT1_RX_CLK,
	NSS_PORT1_TX_CLK,
	NSS_PORT2_RX_CLK,
	NSS_PORT2_TX_CLK,
	NSS_PORT3_RX_CLK,
	NSS_PORT3_TX_CLK,
	NSS_PORT4_RX_CLK,
	NSS_PORT4_TX_CLK,
	NSS_PORT5_RX_CLK,
	NSS_PORT5_TX_CLK,
	NSS_PORT6_RX_CLK,
	NSS_PORT6_TX_CLK,
	UNIPHY0_PORT1_RX_CLK,
	UNIPHY0_PORT1_TX_CLK,
	UNIPHY0_PORT2_RX_CLK,
	UNIPHY0_PORT2_TX_CLK,
	UNIPHY0_PORT3_RX_CLK,
	UNIPHY0_PORT3_TX_CLK,
	UNIPHY0_PORT4_RX_CLK,
	UNIPHY0_PORT4_TX_CLK,
	UNIPHY0_PORT5_RX_CLK,
	UNIPHY0_PORT5_TX_CLK,
	UNIPHY1_PORT5_RX_CLK,
	UNIPHY1_PORT5_TX_CLK,
	UNIPHY2_PORT6_RX_CLK,
	UNIPHY2_PORT6_TX_CLK,
	PORT5_RX_SRC,
	PORT5_TX_SRC
};

static void ssdk_ppe_uniphy_clock_init(void)
{
	a_uint32_t i;
	struct clk *clk;

	for (i = 0; i < SSDK_MAX_UNIPHY_INSTANCE * 2; i++) {
		clk = clk_register(NULL, uniphy_raw_clks[i]);
		if (IS_ERR(clk))
			SSDK_ERROR("Clk register %d fail!\n", i);
	}

	for (i = NSS_PORT1_RX_CLK_E; i < UNIPHYT_CLK_MAX; i++)
		uniphy_port_clks[i] = of_clk_get_by_name(clock_node,
							ppe_clk_ids[i]);

	/* enable uniphy and mac clock */
	for (i = NSS_PORT1_RX_CLK_E; i < PORT5_RX_SRC_E; i++)
		ssdk_uniphy_clock_enable(0, i, A_TRUE);
}

static void ssdk_ppe_fixed_clock_init(void)
{
	/* AHB and sys clk */
	ssdk_clock_rate_set_and_enable(clock_node, CMN_AHB_CLK, 0);
	ssdk_clock_rate_set_and_enable(clock_node, CMN_SYS_CLK, 0);
	ssdk_clock_rate_set_and_enable(clock_node, UNIPHY0_AHB_CLK,
					UNIPHY_AHB_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node, UNIPHY0_SYS_CLK,
					UNIPHY_SYS_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node, UNIPHY1_AHB_CLK,
					UNIPHY_AHB_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node, UNIPHY1_SYS_CLK,
					UNIPHY_SYS_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node, UNIPHY2_AHB_CLK,
					UNIPHY_AHB_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node, UNIPHY2_SYS_CLK,
					UNIPHY_SYS_CLK_RATE);

	/* ppe related fixed clock init */
	ssdk_clock_rate_set_and_enable(clock_node,
					PORT1_MAC_CLK, PPE_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					PORT2_MAC_CLK, PPE_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					PORT3_MAC_CLK, PPE_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					PORT4_MAC_CLK, PPE_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					PORT5_MAC_CLK, PPE_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					PORT6_MAC_CLK, PPE_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					NSS_PPE_CLK, PPE_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					NSS_PPE_CFG_CLK, PPE_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					NSSNOC_PPE_CLK, PPE_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					NSSNOC_PPE_CFG_CLK, PPE_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					NSS_EDMA_CLK, PPE_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					NSS_EDMA_CFG_CLK, PPE_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					NSS_PPE_IPE_CLK, PPE_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					NSS_PPE_BTQ_CLK, PPE_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					MDIO_AHB_CLK, MDIO_AHB_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					NSSNOC_CLK, NSS_NOC_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					NSSNOC_SNOC_CLK, NSSNOC_SNOC_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					MEM_NOC_NSSAXI_CLK, NSS_AXI_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					CRYPTO_PPE_CLK, PPE_CLK_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					NSS_IMEM_CLK, NSS_IMEM_RATE);
	ssdk_clock_rate_set_and_enable(clock_node,
					NSS_PTP_REF_CLK, PTP_REF_RARE);
}

#define CMN_BLK_ADDR	0x0009B780
#define CMN_BLK_SIZE	0x100
static void ssdk_ppe_cmnblk_init(void)
{
	void __iomem *gcc_pll_base = NULL;
	a_uint32_t reg_val;

	gcc_pll_base = ioremap_nocache(CMN_BLK_ADDR, CMN_BLK_SIZE);
	if (!gcc_pll_base) {
		SSDK_ERROR("can't map gcc pll address!\n");
		return;
	}
	reg_val = readl(gcc_pll_base + 4);
	reg_val = (reg_val & 0xfffffff0) | 0x7;
	writel(reg_val, gcc_pll_base + 0x4);
	reg_val = readl(gcc_pll_base);
	reg_val = reg_val | 0x40;
	writel(reg_val, gcc_pll_base);
	msleep(1);
	reg_val = reg_val & (~0x40);
	writel(reg_val, gcc_pll_base);
	msleep(1);
	writel(0xbf, gcc_pll_base);
	msleep(1);
	writel(0xff, gcc_pll_base);
	msleep(1);

	iounmap(gcc_pll_base);
}
#endif

static
void ssdk_uniphy1_clock_source_set(void)
{
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
	clk_set_parent(uniphy_port_clks[PORT5_RX_SRC_E],
			uniphy_raw_clks[2]->clk);
	clk_set_parent(uniphy_port_clks[PORT5_TX_SRC_E],
			uniphy_raw_clks[3]->clk);
#endif
}

void ssdk_ppe_clock_init(void)
{
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
	clock_node = of_find_node_by_name(NULL, "ess-switch");

	ssdk_ppe_fixed_clock_init();
	/*fixme for cmn clock init*/
	ssdk_ppe_cmnblk_init();
	ssdk_ppe_uniphy_clock_init();
#endif
	SSDK_INFO("ppe and uniphy clock init successfully!\n");
}

void ssdk_uniphy_raw_clock_reset(a_uint8_t uniphy_index)
{
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
	a_uint32_t id;

	if (uniphy_index >= SSDK_MAX_UNIPHY_INSTANCE)
		return;

	id = uniphy_index*2;
	if (clk_set_rate(uniphy_raw_clks[id]->clk, UNIPHY_DEFAULT_RATE))
		SSDK_ERROR("set rate for %d fail!\n", id);
	if (clk_set_rate(uniphy_raw_clks[id+1]->clk, UNIPHY_DEFAULT_RATE))
		SSDK_ERROR("set rate for %d fail!\n", id+1);
#endif
}

void ssdk_uniphy_raw_clock_set(
	a_uint8_t uniphy_index,
	a_uint8_t direction,
	a_uint32_t clock)
{
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
	a_uint32_t old_clock, id;

	if ((uniphy_index >= SSDK_MAX_UNIPHY_INSTANCE) ||
	     (direction > UNIPHY_TX) ||
	     (clock != UNIPHY_CLK_RATE_125M &&
	      clock != UNIPHY_CLK_RATE_312M))
		return;

	id = uniphy_index*2 + direction;
	old_clock = clk_get_rate(uniphy_raw_clks[id]->clk);

	if (clock != old_clock) {
		if (uniphy_index == SSDK_UNIPHY_INSTANCE1) {
			if (UNIPHY_RX == direction)
				ssdk_uniphy_clock_rate_set(0,
						NSS_PORT5_RX_CLK_E,
						NSS_PORT5_DFLT_RATE);
			else
				ssdk_uniphy_clock_rate_set(0,
						NSS_PORT5_TX_CLK_E,
						NSS_PORT5_DFLT_RATE);
		}
		if (clk_set_rate(uniphy_raw_clks[id]->clk, clock))
			SSDK_ERROR("set rate: %d fail!\n", clock);
	}

	if (uniphy_index == SSDK_UNIPHY_INSTANCE1) {
		if (clk_set_parent(uniphy_port_clks[PORT5_RX_SRC_E + direction],
				uniphy_raw_clks[id]->clk))
			SSDK_ERROR("set parent fail!\n");
	}
#endif
}

void
qca_gcc_uniphy_port_clock_set(
	a_uint32_t dev_id, a_uint32_t uniphy_index,
	a_uint32_t port_id, a_bool_t enable)
{

	if (uniphy_index == SSDK_UNIPHY_INSTANCE2) {
		ssdk_uniphy_clock_enable(dev_id,
					UNIPHY2_PORT6_RX_CLK_E, enable);
		ssdk_uniphy_clock_enable(dev_id,
					UNIPHY2_PORT6_TX_CLK_E, enable);
	} else if (uniphy_index == SSDK_UNIPHY_INSTANCE1) {
		ssdk_uniphy_clock_enable(dev_id,
					UNIPHY1_PORT5_RX_CLK_E, enable);
		ssdk_uniphy_clock_enable(dev_id,
					UNIPHY1_PORT5_TX_CLK_E, enable);
	} else if (uniphy_index == SSDK_UNIPHY_INSTANCE0) {
		switch (port_id) {
			case SSDK_PHYSICAL_PORT1:
				ssdk_uniphy_clock_enable(dev_id,
							UNIPHY0_PORT1_RX_CLK_E,
							enable);
				ssdk_uniphy_clock_enable(dev_id,
							UNIPHY0_PORT1_TX_CLK_E,
							enable);
				break;
			case SSDK_PHYSICAL_PORT2:
				ssdk_uniphy_clock_enable(dev_id,
							UNIPHY0_PORT2_RX_CLK_E,
							enable);
				ssdk_uniphy_clock_enable(dev_id,
							UNIPHY0_PORT2_TX_CLK_E,
							enable);
				break;
			case SSDK_PHYSICAL_PORT3:
				ssdk_uniphy_clock_enable(dev_id,
							UNIPHY0_PORT3_RX_CLK_E,
							enable);
				ssdk_uniphy_clock_enable(dev_id,
							UNIPHY0_PORT3_TX_CLK_E,
							enable);
				break;
			case SSDK_PHYSICAL_PORT4:
				ssdk_uniphy_clock_enable(dev_id,
							UNIPHY0_PORT4_RX_CLK_E,
							enable);
				ssdk_uniphy_clock_enable(dev_id,
							UNIPHY0_PORT4_TX_CLK_E,
							enable);
				break;
			case SSDK_PHYSICAL_PORT5:
				ssdk_uniphy_clock_enable(dev_id,
							UNIPHY0_PORT5_RX_CLK_E,
							enable);
				ssdk_uniphy_clock_enable(dev_id,
							UNIPHY0_PORT5_TX_CLK_E,
							enable);
				break;
			default:
				break;
		}
	}
}

void
qca_gcc_mac_port_clock_set(
	a_uint32_t dev_id,
	a_uint32_t port_id,
	a_bool_t enable)
{

	switch (port_id) {
		case SSDK_PHYSICAL_PORT1:
			ssdk_uniphy_clock_enable(dev_id,
						NSS_PORT1_RX_CLK_E,
						enable);
			ssdk_uniphy_clock_enable(dev_id,
						NSS_PORT1_TX_CLK_E,
						enable);
			break;
		case SSDK_PHYSICAL_PORT2:
			ssdk_uniphy_clock_enable(dev_id,
						NSS_PORT2_RX_CLK_E,
						enable);
			ssdk_uniphy_clock_enable(dev_id,
						NSS_PORT2_TX_CLK_E,
						enable);
			break;
		case SSDK_PHYSICAL_PORT3:
			ssdk_uniphy_clock_enable(dev_id,
						NSS_PORT3_RX_CLK_E,
						enable);
			ssdk_uniphy_clock_enable(dev_id,
						NSS_PORT3_TX_CLK_E,
						enable);
			break;
		case SSDK_PHYSICAL_PORT4:
			ssdk_uniphy_clock_enable(dev_id,
						NSS_PORT4_RX_CLK_E,
						enable);
			ssdk_uniphy_clock_enable(dev_id,
						NSS_PORT4_TX_CLK_E,
						enable);
			break;
		case SSDK_PHYSICAL_PORT5:
			ssdk_uniphy_clock_enable(dev_id,
						NSS_PORT5_RX_CLK_E,
						enable);
			ssdk_uniphy_clock_enable(dev_id,
						NSS_PORT5_TX_CLK_E,
						enable);
			break;
		case SSDK_PHYSICAL_PORT6:
			ssdk_uniphy_clock_enable(dev_id,
						NSS_PORT6_RX_CLK_E,
						enable);
			ssdk_uniphy_clock_enable(dev_id,
						NSS_PORT6_TX_CLK_E,
						enable);
			break;
		default:
			break;
	}
}

void
ssdk_port_speed_clock_set(
	a_uint32_t dev_id,
	a_uint32_t port_id,
	a_uint32_t rate)
{
	a_uint32_t mode = 0;

               switch (port_id ) {
		case SSDK_PHYSICAL_PORT1:
			ssdk_uniphy_clock_rate_set(dev_id,
					NSS_PORT1_RX_CLK_E, rate);
			ssdk_uniphy_clock_rate_set(dev_id,
					NSS_PORT1_TX_CLK_E, rate);
			break;
		case SSDK_PHYSICAL_PORT2:
			ssdk_uniphy_clock_rate_set(dev_id,
					NSS_PORT2_RX_CLK_E, rate);
			ssdk_uniphy_clock_rate_set(dev_id,
					NSS_PORT2_TX_CLK_E, rate);
			break;
		case SSDK_PHYSICAL_PORT3:
			ssdk_uniphy_clock_rate_set(dev_id,
					NSS_PORT3_RX_CLK_E, rate);
			ssdk_uniphy_clock_rate_set(dev_id,
					NSS_PORT3_TX_CLK_E, rate);
			break;
		case SSDK_PHYSICAL_PORT4:
			ssdk_uniphy_clock_rate_set(dev_id,
					NSS_PORT4_RX_CLK_E, rate);
			ssdk_uniphy_clock_rate_set(dev_id,
					NSS_PORT4_TX_CLK_E, rate);
			break;
		case SSDK_PHYSICAL_PORT5:
			ssdk_uniphy_clock_rate_set(dev_id,
					NSS_PORT5_RX_CLK_E, rate);
			ssdk_uniphy_clock_rate_set(dev_id,
					NSS_PORT5_TX_CLK_E, rate);
			mode = ssdk_dt_global_get_mac_mode(dev_id, SSDK_UNIPHY_INSTANCE1);
			if (mode != PORT_INTERFACE_MODE_MAX)
				ssdk_uniphy1_clock_source_set();
			break;
		case SSDK_PHYSICAL_PORT6:
			ssdk_uniphy_clock_rate_set(dev_id,
					NSS_PORT6_RX_CLK_E, rate);
			ssdk_uniphy_clock_rate_set(dev_id,
					NSS_PORT6_TX_CLK_E, rate);
			break;
		default:
			break;
	}
}

#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
static char *ppe_rst_ids[UNIPHY_RST_MAX] = {
	UNIPHY0_SOFT_RESET_ID,
	UNIPHY0_XPCS_RESET_ID,
	UNIPHY1_SOFT_RESET_ID,
	UNIPHY1_XPCS_RESET_ID,
	UNIPHY2_SOFT_RESET_ID,
	UNIPHY2_XPCS_RESET_ID
};
#endif

void ssdk_ppe_reset_init(void)
{
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0))
	struct reset_control *rst;
	a_uint32_t i;

	rst_node = of_find_node_by_name(NULL, "ess-switch");
	rst = of_reset_control_get(rst_node, PPE_RESET_ID);
	if (IS_ERR(rst)) {
		SSDK_ERROR("%s not exist!\n", PPE_RESET_ID);
		return;
	}

	ssdk_gcc_reset(rst, SSDK_RESET_ASSERT);
	msleep(100);
	ssdk_gcc_reset(rst, SSDK_RESET_DEASSERT);
	msleep(100);
	SSDK_INFO("ppe reset successfully!\n");

	for (i = UNIPHY0_SOFT_RESET_E; i < UNIPHY_RST_MAX; i++)
		uniphy_rsts[i] = of_reset_control_get(rst_node,
							ppe_rst_ids[i]);
#endif
}
#endif
#endif

#ifndef HAWKEYE_CHIP
#if defined(HPPE)
sw_error_t
qca_cppe_fpga_xgmac_clock_enable(a_uint32_t dev_id)
{
	void __iomem *cppe_xgmac_clock_base;

	cppe_xgmac_clock_base = ioremap_nocache(CPPE_XGMAC_CLK_REG,
		CPPE_XGMAC_CLK_SIZE);
	if (!cppe_xgmac_clock_base) {
		SSDK_INFO("can't get cppe xgmac clock address!\n");
		return -1;
	}
	/* RUMI specific clock configuration for enabling XGMAC */
	writel(CPPE_XGMAC_CLK_ENABLE, cppe_xgmac_clock_base + 0);
	iounmap(cppe_xgmac_clock_base);
	SSDK_INFO("set cppe clock to enable XGMAC successfully!\n");

	msleep(100);

	return SW_OK;
}
#endif
#endif
