/*
 * Copyright (c) 2017 MediaTek Inc.
 * Author: Star Chang <star.chang@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_pci.h>
#include <linux/of_platform.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/resource.h>
#include <linux/types.h>
#include <linux/pinctrl/consumer.h>


/*platform device & platform driver match name*/
#define OF_RBUS_NAME "mediatek,wbsys"
#define OF_PIO_NAME "mediatek,mt7622-pctl-a-syscfg"

#define RBUS_VENDOR_ID_OFFSET 0
#define RBUS_CHIP_ID_OFFSET 2
#define RBUS_BAR_OFFSET 0x10
#define RBUS_DEFAULT_CHIP_ID 0x7622
#define RBUS_DEFAULT_VEND_ID 0x14c3
#define RBUS_TSSI_CTRL_OFFSET 0x34
#define RBUS_TSSI_CTRL_MASK 0x1
#define RBUS_PA_LNA_CTRL_OFFSET 0x38
#define RBUS_PA_LNA_CTRL_MASK 0x3

#define GPIO_G2_MISC_OFFSET 0x00000AF0
#define GPIO_G2_MISC_MASK 0xffffff00

static char rbus_string[] = "rbus";
unsigned int dev_second_irq = 0;
unsigned int multi_intr_2nd = 0;
unsigned int multi_intr_3rd = 0;
unsigned int multi_intr_4th = 0;
EXPORT_SYMBOL(dev_second_irq);
EXPORT_SYMBOL(multi_intr_2nd);
EXPORT_SYMBOL(multi_intr_3rd);
EXPORT_SYMBOL(multi_intr_4th);

static const struct of_device_id rbus_of_ids[] = {
	{   .compatible = OF_RBUS_NAME, },
	{ },
};

struct rbus_dev {
	char name[36];
	struct device *dev;
	struct resource *res;
	struct list_head resources;
	unsigned int base_addr;
	unsigned int irq;
	unsigned int chip_id;
	unsigned int vend_id;
};

enum {
	TSSI_MODE_DIS=0,
	TSSI_MODE_EN=1
};

enum {
	IPA_ILNA_MODE=0,
	IPA_ELNA_MODE=1,
	EPA_ELNA_MODE=2,
	EPA_ILNA_MODE=3
};

#define RBUS_IO_READ32(_A, _R, _pV) (*(_pV) = readl((void *)(_A + _R)))
#define RBUS_IO_WRITE32(_A, _R, _V)	 writel(_V, (void *)(_A + _R))

/*fake configure space*/
static unsigned char rbus_conf_space[] = {
	0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x10, 0x00, 0x00, 0x80, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x76, 0xc3, 0x14,
	0x00, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x60, 0x61, 0x12, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x05, 0x78, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0xc3, 0x01, 0x08, 0x00, 0x00, 0x00,
	0x10, 0x00, 0x02, 0x00, 0x40, 0x83, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x12, 0x8c, 0x40, 0x01,
	0x43, 0x00, 0x12, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static int
rbus_tssi_config(struct platform_device *pdev, unsigned char mode)
{
	struct device_node *node = NULL;
	unsigned long addr;
	unsigned int value = 0;

	node = of_find_compatible_node(NULL, NULL, OF_PIO_NAME);
	if (!node) {
		dev_err(&pdev->dev, "%s(): can't find node for %s\n", __func__, OF_PIO_NAME);
		return -ENODEV;
	}

	addr = (unsigned long) of_iomap(node, 0);
	RBUS_IO_READ32(addr, GPIO_G2_MISC_OFFSET, &value);

	if (mode == TSSI_MODE_EN) {
		value &= GPIO_G2_MISC_MASK;
		RBUS_IO_WRITE32(addr, GPIO_G2_MISC_OFFSET, value);
	}

	RBUS_IO_READ32(addr, GPIO_G2_MISC_OFFSET, &value);
	return 0;
}

static int
rbus_pa_lan_config(struct platform_device *pdev, unsigned int devfn, unsigned char mode)
{
	struct pinctrl *p;
	struct pinctrl_state *s;
	unsigned char state[32] = "";
	int ret = 0;

	if (mode != IPA_ELNA_MODE && mode != EPA_ELNA_MODE)
		return ret;

	p = devm_pinctrl_get(&pdev->dev);

	if (!p) {
		dev_err(&pdev->dev, "%s(): can't get pinctrl by dev:%p\n", __func__, &pdev->dev);
		return ret;
	}

	strncpy(state, "state_epa", sizeof("state_epa"));

	s = pinctrl_lookup_state(p, state);

	if (!s) {
		dev_err(&pdev->dev, "%s(): can't find pinctrl state: %s\n", __func__, state);
		return ret;
	}

	ret = pinctrl_select_state(p, s);

	if (ret < 0)
		dev_err(&pdev->dev, "%s(): pinctrl select to %s fail!, ret=%d\n", __func__, state, ret);

	return ret;
}

static void
rbus_init_config(struct rbus_dev *rbus)
{
	rbus_conf_space[RBUS_VENDOR_ID_OFFSET] = rbus->vend_id & 0xff;
	rbus_conf_space[RBUS_VENDOR_ID_OFFSET + 1] = (rbus->vend_id >> 8) & 0xff;
	rbus_conf_space[RBUS_CHIP_ID_OFFSET] = rbus->chip_id & 0xff;
	rbus_conf_space[RBUS_CHIP_ID_OFFSET + 1] = (rbus->chip_id >> 8) & 0xff;
	rbus_conf_space[RBUS_BAR_OFFSET + 3] = (rbus->base_addr >> 24) & 0xff;
	rbus_conf_space[RBUS_BAR_OFFSET + 2] = (rbus->base_addr >> 16) & 0xff;
	rbus_conf_space[RBUS_BAR_OFFSET + 1] = (rbus->base_addr >> 8) & 0xff;
}

static int
rbus_read_config(struct pci_bus *bus, unsigned int devfn, int where,
			int size, u32 *value)
{
	u32 *cr;

	if(where >= sizeof(rbus_conf_space))
		return PCIBIOS_BUFFER_TOO_SMALL;

	cr = (u32 *) &rbus_conf_space[where];

	if(devfn == 0)
		*value = *cr;
	return PCIBIOS_SUCCESSFUL;
}

static int
rbus_write_config(struct pci_bus *bus, unsigned int devfn, int where,
			int size, u32 value)
{
	int i;
	struct platform_device *pdev = bus->sysdata;

	if (devfn != 0)
		goto end;

	for (i = 0 ; i < size ; i++) {
		rbus_conf_space[where + i] = (value << (i * 8)) & 0xff;
	}
	/*handle vendor specific action*/
	switch(where) {
	case RBUS_TSSI_CTRL_OFFSET:
		rbus_tssi_config(pdev, (value & RBUS_TSSI_CTRL_MASK));
	break;
	case RBUS_PA_LNA_CTRL_OFFSET:
		rbus_pa_lan_config(pdev, devfn, (value & RBUS_PA_LNA_CTRL_MASK));
	break;
	default:
	break;
	}
end:
	return PCIBIOS_SUCCESSFUL;
}


struct pci_ops rbus_ops = {
	.read  = rbus_read_config,
	.write = rbus_write_config,
};

static int rbus_add_port(struct rbus_dev *rbus,
				   struct platform_device *pdev)
{
	struct pci_bus *bus;
	struct pci_dev *pci;

	bus = pci_scan_root_bus(&pdev->dev, 0, &rbus_ops,
				pdev, &rbus->resources);

	if (!bus)
		return -ENOMEM;

	pci_bus_add_devices(bus);

	pci = pci_scan_single_device(bus, 0);

	if (pci) {
		/*re-assign hw resource*/
		pci->irq = rbus->irq;
		pci->resource[0].start = rbus->res->start;
		pci->resource[0].end = rbus->res->end;
	}
	return 0;
}

static int rbus_add_res(struct rbus_dev *rbus)
{
	struct device *dev = rbus->dev;
	struct platform_device *pdev = to_platform_device(dev);
	struct resource bus_range;

	INIT_LIST_HEAD(&rbus->resources);
	/*resource allocate*/
	rbus->res  = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	rbus->irq = platform_get_irq(pdev, 0);
	rbus->base_addr = (unsigned int)rbus->res->start;
	if (rbus->chip_id == 0x7629)
		dev_second_irq = platform_get_irq(pdev, 1);
	else if (rbus->chip_id == 0x7986) {
		multi_intr_2nd = platform_get_irq(pdev, 1);
		multi_intr_3rd = platform_get_irq(pdev, 2);
		multi_intr_4th = platform_get_irq(pdev, 3);
	}

	pci_add_resource(&rbus->resources, rbus->res);

	bus_range = (struct resource) {
		.name	= "rbus_range",
		.start	= 0,
		.end	= 0xff,
		.flags	= IORESOURCE_BUS,
	};

	pci_add_resource(&rbus->resources, &bus_range);
	return 0;
}

/*
*
*/
static int rbus_probe(struct platform_device *pdev)
{
	struct device_node *node = NULL;
	struct rbus_dev *rbus;

	node = of_find_compatible_node(NULL, NULL, OF_RBUS_NAME);
	if (!node)
		return -ENODEV;

	rbus = devm_kzalloc(&pdev->dev, sizeof(*rbus), GFP_KERNEL);
	if (!rbus)
		return -ENOMEM;

	rbus->dev = &pdev->dev;

	if (of_property_read_u32_index(node, "chip_id", 0, &rbus->chip_id)) {
		rbus->chip_id = RBUS_DEFAULT_CHIP_ID;
	}

	if (of_property_read_u32_index(node, "vend_id", 0, &rbus->vend_id)) {
		rbus->vend_id = RBUS_DEFAULT_VEND_ID;
	}
	/*set priv_data to pdev*/
	snprintf(rbus->name,sizeof(rbus->name),"mediatek-rbus");
	platform_set_drvdata(pdev, rbus);
	rbus_add_res(rbus);
	/*init config, need run before add port*/
	rbus_init_config(rbus);
	/*add pci bus & device*/
	rbus_add_port(rbus, pdev);
	return -ENODEV;
}

/*
*
*/
static int rbus_remove(struct platform_device *pdev)
{
	struct rbus_dev *rbus = platform_get_drvdata(pdev);
	dev_err(&pdev->dev, "remove rbus name: %s\n", rbus->name);
	return 0;
}


/*
* global resource preparing
*/
static struct platform_driver rbus_driver = {
	.probe  = rbus_probe,
	.remove = rbus_remove,
	.driver = {
		.name   = rbus_string,
		.owner  = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = rbus_of_ids,
#endif /*CONFIG_OF*/
	},
};

/* PCIe driver does not allow module unload */
static int __init rbus_init(void)
{
	return platform_driver_probe(&rbus_driver, rbus_probe);
}

subsys_initcall_sync(rbus_init);

MODULE_DESCRIPTION("Mediatek RBUS host controller driver");
MODULE_LICENSE("GPL v2");

