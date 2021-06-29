// SPDX-License-Identifier : GPL-2.0
/*
 * PCIe RC driver for Allwinner DW PCIe controllers
 *
 * Copyright (C) 2018 Icenowy Zheng <icenowy@aosc.io>
 *
 * Copyright (C) 2016 Allwinner Co., Ltd.
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/reset.h>
#include <linux/resource.h>
#include <linux/signal.h>
#include <linux/types.h>

#include "pcie-designware.h"

#define PCIE_RC_LCR				0x7c
#define USER_DEFINED_REGISTER_LIST		0x1000
#define PCIE_LTSSM_ENABLE			0x1000
#define PCIE_INT_PENDING			0x1018
#define PCIE_ADDR_PAGE_CFG			0x1020
#define PCIE_AWMISC_INF0_CTRL			0x1030
#define PCIE_ARMISC_INF0_CTRL			0x1034
#define PCIE_LINK_STATUS			0x143C
#define PCIE_PHY_CFG				0x1800
#define PCIE_RC_LCR_MAX_LINK_SPEEDS_GEN1	0x1
#define PCIE_RC_LCR_MAX_LINK_SPEEDS_GEN2	0x2
#define PCIE_RC_LCR_MAX_LINK_SPEEDS_MASK	0xf
#define SYS_CLK					0
#define PAD_CLK					1
#define RDLH_LINK_UP				BIT(1)
#define SMLH_LINK_UP				BIT(0)
#define PCIE_LINK_TRAINING			BIT(0)
#define PCIE_LINK_UP_MASK			(0x3 << 16)
#define LINK_WAIT_MAX_RETRIES			10
#define LINK_WAIT_USLEEP_MIN			90000
#define LINK_WAIT_USLEEP_MAX			100000

#define PERST_DELAY_US				1000

#define PCIE_BAR_NUM				6
#define PCIE_MEM_FLAGS				0x4
#define PCIE_IO_FLAGS				0x1
#define PCIE_BAR_REG				0x4
#define HIGH16_MASK				GENMASK(31, 16)
#define LOW16_MASK				GENMASK(15, 0)

#define to_sunxi_pcie(x)			dev_get_drvdata((x)->dev)

struct sunxi_pcie {
	struct dw_pcie		*pci;
	int			link_irq;
	int			msi_irq;
	int			speed_gen;
	int			io_voltage;
	struct clk		*pcie_ref;
	struct clk		*pcie_axi;
	struct clk		*pcie_aux;
	struct clk		*pcie_bus;
	struct reset_control	*pcie_power_rst;
	struct reset_control	*pcie_bus_rst;
	struct regulator	*reg_vcc;
	struct regulator	*reg_vdd;
	struct regulator	*reg_slot;
	struct gpio_desc	*perst_gpio;
};

static void sunxi_pcie_perst_gpio_assert(struct sunxi_pcie *pcie)
{
	gpiod_set_value_cansleep(pcie->perst_gpio, 1);
	usleep_range(PERST_DELAY_US, PERST_DELAY_US + 500);
}

static void sunxi_pcie_perst_gpio_deassert(struct sunxi_pcie *pcie)
{
	gpiod_set_value_cansleep(pcie->perst_gpio, 0);
	usleep_range(PERST_DELAY_US, PERST_DELAY_US + 500);
}

static void sunxi_pcie_clk_select(struct dw_pcie *pci, int status)
{
	u32 val;

	val = dw_pcie_readl_dbi(pci, PCIE_PHY_CFG);
	val &= ~(0x1 << 31);
	val |= status << 31;
	dw_pcie_writel_dbi(pci, PCIE_PHY_CFG, val);
}

static void sunxi_pcie_ltssm_enable(struct dw_pcie *pci)
{
	u32 val;

	val = dw_pcie_readl_dbi(pci, PCIE_LTSSM_ENABLE);
	val |= PCIE_LINK_TRAINING;
	dw_pcie_writel_dbi(pci, PCIE_LTSSM_ENABLE, val);
}

static void sunxi_pcie_irqpending(struct dw_pcie *pci)
{
	u32 val;

	val = dw_pcie_readl_dbi(pci, PCIE_INT_PENDING);
	val &= ~(0x3<<16);
	dw_pcie_writel_dbi(pci, PCIE_INT_PENDING, val);
}

static void sunxi_pcie_phy_cfg(struct dw_pcie *pci, int enable)
{
	u32 val;

	val = dw_pcie_readl_dbi(pci, PCIE_PHY_CFG);
	if (enable)
		val |= 0x1<<0;
	else
		val &= ~(0x1<<0);
	dw_pcie_writel_dbi(pci, PCIE_PHY_CFG, val);
}

static void sunxi_pcie_irqmask(struct dw_pcie *pci)
{
	u32 val;

	val = dw_pcie_readl_dbi(pci, PCIE_INT_PENDING);
	val |= 0x3<<16;
	dw_pcie_writel_dbi(pci, PCIE_INT_PENDING, val);
}

static void sunxi_pcie_ltssm_disable(struct dw_pcie *pci)
{
	u32 val;

	val = dw_pcie_readl_dbi(pci, PCIE_LTSSM_ENABLE);
	val &= ~PCIE_LINK_TRAINING;
	dw_pcie_writel_dbi(pci, PCIE_LTSSM_ENABLE, val);
}

static int sunxi_pcie_wait_for_speed_change(struct dw_pcie *pci)
{
	u32 tmp;
	unsigned int retries;

	for (retries = 0; retries < 200; retries++) {
		tmp = dw_pcie_readl_dbi(pci, PCIE_LINK_WIDTH_SPEED_CONTROL);
		/* Test if the speed change finished. */
		if (!(tmp & PORT_LOGIC_SPEED_CHANGE))
			return 0;
		usleep_range(100, 1000);
	}

	dev_err(pci->dev, "Speed change timeout\n");
	return -EINVAL;
}

static int sunxi_pcie_link_up_status(struct dw_pcie *pci)
{
	u32 rc;
	int ret;

	rc = dw_pcie_readl_dbi(pci, PCIE_LINK_STATUS);
	if ((rc & RDLH_LINK_UP) && (rc & SMLH_LINK_UP))
		ret = 1;
	else
		ret = 0;

	return ret;
}

static int sunxi_pcie_speed_change(struct dw_pcie *pci, int gen)
{
	int val;
	int ret;

	if (gen == 2) {
		val = dw_pcie_readl_dbi(pci, PCIE_RC_LCR);
		val &= ~PCIE_RC_LCR_MAX_LINK_SPEEDS_MASK;
		val |= PCIE_RC_LCR_MAX_LINK_SPEEDS_GEN2;
		dw_pcie_writel_dbi(pci, PCIE_RC_LCR, val);

		/*
		 * Start Directed Speed Change so the best possible
		 * speed both link partners support can be negotiated.
		 */
		val = dw_pcie_readl_dbi(pci, PCIE_LINK_WIDTH_SPEED_CONTROL);
		val |= PORT_LOGIC_SPEED_CHANGE;
		dw_pcie_writel_dbi(pci, PCIE_LINK_WIDTH_SPEED_CONTROL, val);
		ret = sunxi_pcie_wait_for_speed_change(pci);

		if (!ret)
			dev_info(pci->dev, "PCI-e speed of Gen%d\n", gen);
		else
			dev_info(pci->dev, "PCI-e speed of Gen1\n");
	} else {
		dev_info(pci->dev, "PCI-e speed of Gen1\n");
	}

	return 0;
}

static int sunxi_pcie_establish_link(struct dw_pcie *pci)
{
	struct sunxi_pcie *pcie = to_sunxi_pcie(pci);

	if (dw_pcie_link_up(pci)) {
		dev_err(pcie->pci->dev, "link is already up\n");
		return 0;
	}

	sunxi_pcie_ltssm_enable(pci);
	dw_pcie_wait_for_link(pci);

	return 0;
}

static int sunxi_pcie_reset_deassert(struct sunxi_pcie *pcie)
{
	int ret;

	ret = reset_control_deassert(pcie->pcie_bus_rst);
	if (ret) {
		dev_err(pcie->pci->dev, "unable to deassert bus reset\n");
		return ret;
	}
	usleep_range(1000, 2000);

	if (pcie->io_voltage < 2000000)
		sunxi_pcie_phy_cfg(pcie->pci, 1);
	else
		sunxi_pcie_phy_cfg(pcie->pci, 0);

	usleep_range(1000, 2000);
	ret = reset_control_deassert(pcie->pcie_power_rst);
	if (ret) {
		dev_err(pcie->pci->dev, "unable to deassert power reset\n");
		goto bus_reset_assert;
	}

	return 0;

bus_reset_assert:
	reset_control_assert(pcie->pcie_bus_rst);
	return ret;
}

static int sunxi_pcie_clk_setup(struct sunxi_pcie *pcie)
{
	int ret;

	ret = clk_prepare_enable(pcie->pcie_ref);
	if (ret) {
		dev_err(pcie->pci->dev, "unable to enable pcie_ref clock\n");
		return ret;
	}

	ret = clk_prepare_enable(pcie->pcie_axi);
	if (ret) {
		dev_err(pcie->pci->dev, "unable to enable pcie_axi clock\n");
		goto disable_ref;
	}

	ret = clk_prepare_enable(pcie->pcie_aux);
	if (ret) {
		dev_err(pcie->pci->dev, "unable to enable pcie_aux clock\n");
		goto disable_axi;
	}

	ret = clk_prepare_enable(pcie->pcie_bus);
	if (ret) {
		dev_err(pcie->pci->dev, "unable to enable pcie_bus clock\n");
		goto disable_aux;
	}

	return 0;

disable_aux:
	clk_disable_unprepare(pcie->pcie_aux);
disable_axi:
	clk_disable_unprepare(pcie->pcie_axi);
disable_ref:
	clk_disable_unprepare(pcie->pcie_ref);

	return ret;
}

static int sunxi_pcie_regulator_enable(struct sunxi_pcie *pcie)
{
	int ret;

	ret = regulator_enable(pcie->reg_vcc);
	if (ret)
		return ret;

	pcie->io_voltage = regulator_get_voltage(pcie->reg_vcc);
	if (pcie->io_voltage < 0) {
		ret = pcie->io_voltage;
		goto disable_vcc;
	}

	ret = regulator_enable(pcie->reg_vdd);
	if (ret)
		goto disable_vcc;

	ret = regulator_enable(pcie->reg_slot);
	if (ret)
		goto disable_vdd;

	return 0;

disable_vdd:
	regulator_disable(pcie->reg_vdd);
disable_vcc:
	regulator_disable(pcie->reg_vcc);
	
	return ret;
}

static irqreturn_t sunxi_pcie_msi_irq_handler(int irq, void *arg)
{
	struct sunxi_pcie *pcie = (struct sunxi_pcie *)arg;

	return dw_handle_msi_irq(&pcie->pci->pp);
}

static irqreturn_t sunxi_pcie_linkup_handler(int irq, void *arg)
{
	struct sunxi_pcie *pcie = (struct sunxi_pcie *)arg;

	sunxi_pcie_irqpending(pcie->pci);

	return IRQ_HANDLED;
}

static int sunxi_pcie_get_clk_rst(struct platform_device *pdev,
				  struct sunxi_pcie *pcie)
{
	struct device *dev = &pdev->dev;

	pcie->pcie_ref = devm_clk_get(dev, "ref");
	if (IS_ERR(pcie->pcie_ref)) {
		dev_err(dev, "failed to get pcie ref clk\n");
		return PTR_ERR(pcie->pcie_ref);
	}

	pcie->pcie_axi = devm_clk_get(dev, "axi");
	if (IS_ERR(pcie->pcie_axi)) {
		dev_err(dev, "failed to get pcie axi clk\n");
		return PTR_ERR(pcie->pcie_axi);
	}

	pcie->pcie_aux = devm_clk_get(dev, "aux");
	if (IS_ERR(pcie->pcie_aux)) {
		dev_err(dev, "failed to get pcie aux clk\n");
		return PTR_ERR(pcie->pcie_aux);
	}

	pcie->pcie_bus = devm_clk_get(dev, "bus");
	if (IS_ERR(pcie->pcie_bus)) {
		dev_err(dev, "failed to get pcie bus clk\n");
		return PTR_ERR(pcie->pcie_bus);
	}

	pcie->pcie_bus_rst = devm_reset_control_get(dev, "bus");
	if (IS_ERR(pcie->pcie_bus_rst)) {
		dev_err(dev, "failed to get pcie bus reset\n");
		return PTR_ERR(pcie->pcie_bus_rst);
	}

	pcie->pcie_power_rst = devm_reset_control_get(dev, "power");
	if (IS_ERR(pcie->pcie_power_rst)) {
		dev_err(dev, "failed to get pcie power reset\n");
		return PTR_ERR(pcie->pcie_power_rst);
	}

	return 0;
}

static int sunxi_pcie_get_regulator(struct platform_device *pdev,
				    struct sunxi_pcie *pcie)
{
	struct device *dev = &pdev->dev;

	pcie->reg_vcc = devm_regulator_get(dev, "vcc");
	if (IS_ERR(pcie->reg_vcc)) {
		dev_err(dev, "failed to get pcie vcc supply\n");
		return PTR_ERR(pcie->reg_vcc);
	}

	pcie->reg_vdd = devm_regulator_get(dev, "vdd");
	if (IS_ERR(pcie->reg_vdd)) {
		dev_err(dev, "failed to get pcie vdd supply\n");
		return PTR_ERR(pcie->reg_vdd);
	}

	pcie->reg_slot = devm_regulator_get(dev, "slot");
	if (IS_ERR(pcie->reg_slot)) {
		dev_err(dev, "failed to get pcie slot supply\n");
		return PTR_ERR(pcie->reg_slot);
	}

	return 0;
}

static int sunxi_pcie_host_init(struct pcie_port *pp)
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct sunxi_pcie *pcie = to_sunxi_pcie(pci);

	sunxi_pcie_ltssm_disable(pci);
	sunxi_pcie_clk_select(pci, PAD_CLK);

	dw_pcie_setup_rc(pp);

	dw_pcie_prog_outbound_atu(pci, PCIE_ATU_REGION_INDEX0,
				  PCIE_ATU_TYPE_MEM, pp->mem_base,
				  pp->mem_bus_addr, pp->mem_size);
	if (pci->num_viewport > 2)
		dw_pcie_prog_outbound_atu(pci, PCIE_ATU_REGION_INDEX2,
					  PCIE_ATU_TYPE_IO, pp->io_base,
					  pp->io_bus_addr, pp->io_size);

	sunxi_pcie_establish_link(pci);
	if (IS_ENABLED(CONFIG_PCI_MSI))
		dw_pcie_msi_init(pp);

	sunxi_pcie_speed_change(pci, pcie->speed_gen);

	return 0;
}

static const struct dw_pcie_host_ops sunxi_pcie_host_ops = {
	.host_init = sunxi_pcie_host_init,
};

static const struct dw_pcie_ops sunxi_pcie_ops = {
	.link_up = sunxi_pcie_link_up_status,
};

static int sunxi_add_pcie_port(struct sunxi_pcie *pcie,
			       struct platform_device *pdev)
{
	int ret;
	struct pcie_port *pp = &pcie->pci->pp;

	if (IS_ENABLED(CONFIG_PCI_MSI)) {
		pp->msi_irq = platform_get_irq_byname(pdev, "msi");
		if (pp->msi_irq < 0)
			return pp->msi_irq;

		ret = devm_request_irq(&pdev->dev, pp->msi_irq,
					sunxi_pcie_msi_irq_handler,
					IRQF_SHARED, "sunxi-pcie-msi", pcie);
		if (ret) {
			dev_err(&pdev->dev, "failed to request MSI IRQ\n");
			return ret;
		}
	}

	pp->root_bus_nr = -1;
	pp->ops = &sunxi_pcie_host_ops;

	ret = dw_pcie_host_init(pp);
	if (ret) {
		dev_err(&pdev->dev, "failed to initialize host\n");
		return ret;
	}

	return 0;
}

static int sunxi_pcie_probe(struct platform_device *pdev)
{
	struct sunxi_pcie *sunxi_pcie;
	struct dw_pcie *pci;
	struct resource *dbi_res;
	int ret;

	sunxi_pcie = devm_kzalloc(&pdev->dev, sizeof(*sunxi_pcie),
				  GFP_KERNEL);
	if (!sunxi_pcie)
		return -ENOMEM;

	pci = devm_kzalloc(&pdev->dev, sizeof(*pci), GFP_KERNEL);
	if (!pci)
		return -ENOMEM;

	sunxi_pcie->pci = pci;

	pci->dev = &pdev->dev;
	pci->ops = &sunxi_pcie_ops;

	platform_set_drvdata(pdev, sunxi_pcie);

	dbi_res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi");
	if (!dbi_res)
		return -ENODEV;

	pci->dbi_base = devm_ioremap_resource(&pdev->dev, dbi_res);
	if (IS_ERR(pci->dbi_base))
		return PTR_ERR(pci->dbi_base);

	ret = sunxi_pcie_get_clk_rst(pdev, sunxi_pcie);
	if (ret)
		return ret;

	ret = sunxi_pcie_get_regulator(pdev, sunxi_pcie);
	if (ret)
		return ret;

	sunxi_pcie->perst_gpio = devm_gpiod_get_optional(&pdev->dev, "perst", GPIOD_OUT_LOW);
	if (IS_ERR(sunxi_pcie->perst_gpio))
		return PTR_ERR(sunxi_pcie->perst_gpio);

	ret = of_property_read_u32(pdev->dev.of_node, "max-link-speed", &sunxi_pcie->speed_gen);
	if (ret) {
		dev_info(&pdev->dev, "No speed generation info specified, fallback to Gen1\n");
		sunxi_pcie->speed_gen = 0x1;
	}

	ret = sunxi_pcie_regulator_enable(sunxi_pcie);
	if (ret)
		return ret;

	ret = sunxi_pcie_clk_setup(sunxi_pcie);
	if (ret)
		return ret;

	ret = sunxi_pcie_reset_deassert(sunxi_pcie);
	if (ret)
		return ret;

	sunxi_pcie_perst_gpio_deassert(sunxi_pcie);

	sunxi_pcie_irqmask(pci);
	sunxi_pcie->link_irq = platform_get_irq_byname(pdev, "linkup");
	ret = devm_request_irq(&pdev->dev, sunxi_pcie->link_irq,
			       sunxi_pcie_linkup_handler,
			       IRQF_SHARED, "sunxi-pcie-linkup", sunxi_pcie);
	if (ret) {
		dev_err(&pdev->dev, "failed to request linkup IRQ\n");
		return ret;
	}

	ret = sunxi_add_pcie_port(sunxi_pcie, pdev);
	if (ret < 0)
		return ret;

	return 0;
}

static int sunxi_pcie_remove(struct platform_device *pdev)
{
	struct sunxi_pcie *pcie = platform_get_drvdata(pdev);

	sunxi_pcie_perst_gpio_assert(pcie);
	reset_control_assert(pcie->pcie_bus_rst);
	reset_control_assert(pcie->pcie_power_rst);
	clk_disable_unprepare(pcie->pcie_ref);
	clk_disable_unprepare(pcie->pcie_axi);
	clk_disable_unprepare(pcie->pcie_aux);
	clk_disable_unprepare(pcie->pcie_bus);

	return 0;
}

static const struct of_device_id sunxi_pcie_of_match[] = {
	{ .compatible = "allwinner,sun50i-h6-pcie", },
	{},
};
MODULE_DEVICE_TABLE(of, sunxi_pcie_of_match);

static struct platform_driver sunxi_pcie_driver = {
	.driver = {
		.name = "sunxi-pcie",
		.suppress_bind_attrs = true,
		.of_match_table = sunxi_pcie_of_match,
	},
	.remove = sunxi_pcie_remove,
	.probe	= sunxi_pcie_probe,
};
builtin_platform_driver(sunxi_pcie_driver);
