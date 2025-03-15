// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020 Collabora Ltd.
 */
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/mfd/syscon.h>
#include <linux/of_clk.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_domain.h>
#include <linux/regmap.h>
#include <linux/soc/mediatek/infracfg.h>
#include <linux/regulator/consumer.h>

#include "mt7988-pm-domains.h"

#define MTK_POLL_DELAY_US 30
#define MTK_POLL_TIMEOUT  USEC_PER_SEC

struct scpsys_domain {
	struct generic_pm_domain genpd;
	const struct scpsys_domain_data *data;
	struct scpsys *scpsys;
	int num_clks;
	struct clk_bulk_data *clks;
	int num_subsys_clks;
	struct clk_bulk_data *subsys_clks;
	struct regmap *infracfg;
	struct regulator *supply;
};

struct scpsys {
	struct device *dev;
	struct regmap *base;
	const struct scpsys_soc_data *soc_data;
	struct genpd_onecell_data pd_data;
	struct generic_pm_domain *domains[];
};

static inline int mtk_regmap_set_bits(struct regmap *map, unsigned int reg,
				      unsigned int bits)
{
	return regmap_update_bits_base(map, reg, bits, bits, NULL, false,
				       false);
}

static inline int mtk_regmap_clear_bits(struct regmap *map, unsigned int reg,
					unsigned int bits)
{
	return regmap_update_bits_base(map, reg, bits, 0, NULL, false, false);
}

#define to_scpsys_domain(gpd) container_of(gpd, struct scpsys_domain, genpd)

static bool scpsys_domain_is_on(struct scpsys_domain *pd)
{
	struct scpsys *scpsys = pd->scpsys;
	u32 status = 0, status2 = 0;

	regmap_read(scpsys->base, pd->data->pwr_sta_offs, &status);
	status &= pd->data->sta_mask;

	regmap_read(scpsys->base, pd->data->pwr_sta_2nd_offs, &status2);
	status2 &= pd->data->sta_2nd_mask;

	/* A domain is on when both status bits are set. */
	return status && status2;
}

static int scpsys_sram_enable(struct scpsys_domain *pd)
{
	u32 pdn_ack = pd->data->sram_pdn_ack_bit;
	u32 pdn_2nd_ack = pd->data->sram_2nd_pdn_ack_bit;
	struct scpsys *scpsys = pd->scpsys;
	unsigned int tmp = 0;
	int ret;

	if (pd->data->sram_pdn_bit) {
		mtk_regmap_clear_bits(scpsys->base, pd->data->sram_ctrl_offs,
				      pd->data->sram_pdn_bit);

		/* Either wait until SRAM_PDN_ACK all 1 or 0 */
		ret = regmap_read_poll_timeout(scpsys->base,
					       pd->data->sram_ctrl_offs, tmp,
					       (tmp & pdn_ack) == 0,
					       MTK_POLL_DELAY_US,
					       MTK_POLL_TIMEOUT);
		if (ret < 0)
			return ret;
	}
	if (pd->data->sram_2nd_pdn_bit) {
		/* sram pdn 2nd for special mtcmos */
		mtk_regmap_clear_bits(scpsys->base,
				      pd->data->sram_2nd_ctrl_offs,
				      pd->data->sram_2nd_pdn_bit);

		ret = regmap_read_poll_timeout(scpsys->base,
					       pd->data->sram_2nd_ctrl_offs,
					       tmp, (tmp & pdn_2nd_ack) == 0,
					       MTK_POLL_DELAY_US,
					       MTK_POLL_TIMEOUT);
		if (ret < 0)
			return ret;
	}
	if (pd->data->sram_clk_iso_bit) {
		mtk_regmap_clear_bits(scpsys->base, pd->data->sram_ctrl_offs,
				      pd->data->sram_clk_iso_bit);
	}
	if (pd->data->sram_clk_dis_bit) {
		mtk_regmap_clear_bits(scpsys->base, pd->data->sram_ctrl_offs,
				      pd->data->sram_clk_dis_bit);
	}
	if (pd->data->sram_2nd_clk_iso_bit) {
		mtk_regmap_clear_bits(scpsys->base,
				      pd->data->sram_2nd_ctrl_offs,
				      pd->data->sram_2nd_clk_iso_bit);
	}
	if (pd->data->sram_2nd_clk_dis_bit) {
		mtk_regmap_clear_bits(scpsys->base,
				      pd->data->sram_2nd_ctrl_offs,
				      pd->data->sram_2nd_clk_dis_bit);
	}

	return 0;
}

static int scpsys_sram_disable(struct scpsys_domain *pd)
{
	u32 pdn_ack = pd->data->sram_pdn_ack_bit;
	u32 pdn_2nd_ack = pd->data->sram_2nd_pdn_ack_bit;
	struct scpsys *scpsys = pd->scpsys;
	unsigned int tmp = 0;
	int ret;

	if (pd->data->sram_2nd_clk_dis_bit) {
		mtk_regmap_set_bits(scpsys->base, pd->data->sram_2nd_ctrl_offs,
				    pd->data->sram_2nd_clk_dis_bit);
	}
	if (pd->data->sram_clk_iso_bit) {
		mtk_regmap_set_bits(scpsys->base, pd->data->sram_ctrl_offs,
				    pd->data->sram_clk_iso_bit);
		udelay(1);
	}
	if (pd->data->sram_pdn_bit) {
		mtk_regmap_set_bits(scpsys->base, pd->data->sram_ctrl_offs,
				    pd->data->sram_pdn_bit);
	}

	if (pd->data->sram_2nd_clk_iso_bit) {
		mtk_regmap_set_bits(scpsys->base, pd->data->sram_2nd_ctrl_offs,
				    pd->data->sram_2nd_clk_iso_bit);
		udelay(1);
	}
	if (pd->data->sram_2nd_pdn_bit) {
		mtk_regmap_set_bits(scpsys->base, pd->data->sram_2nd_ctrl_offs,
				    pd->data->sram_2nd_pdn_bit);
	}

	return 0;
}

static int scpsys_regulator_get(struct generic_pm_domain *genpd)
{
	struct scpsys_domain *pd =
		container_of(genpd, struct scpsys_domain, genpd);
	struct device_node *node;
	struct device_node *root;

	if (MTK_SCPD_CAPS(pd, MTK_SCPD_DOMAIN_SUPPLY) && !pd->supply) {
		root = pd->scpsys->dev->of_node;
		node = of_find_node_by_name(root, genpd->name);
		if (node) {
			pd->scpsys->dev->of_node = node;
			pd->supply =
				devm_regulator_get(pd->scpsys->dev, "domain");
			pd->scpsys->dev->of_node = root;
			if (IS_ERR(pd->supply))
				return -EINVAL;
		}
	}

	return 0;
}

static int scpsys_regulator_enable(struct generic_pm_domain *genpd)
{
	struct scpsys_domain *pd =
		container_of(genpd, struct scpsys_domain, genpd);
	int ret = scpsys_regulator_get(genpd);

	if (ret)
		return ret;

	return pd->supply ? regulator_enable(pd->supply) : 0;
}

static int scpsys_regulator_disable(struct generic_pm_domain *genpd)
{
	struct scpsys_domain *pd =
		container_of(genpd, struct scpsys_domain, genpd);

	return pd->supply ? regulator_disable(pd->supply) : 0;
}

static int scpsys_power_on(struct generic_pm_domain *genpd)
{
	struct scpsys_domain *pd =
		container_of(genpd, struct scpsys_domain, genpd);
	struct scpsys *scpsys = pd->scpsys;
	bool tmp;
	int ret;

	ret = scpsys_regulator_enable(genpd);
	if (ret)
		return ret;

	ret = clk_bulk_prepare_enable(pd->num_clks, pd->clks);
	if (ret)
		goto err_pwr_ack;

	/* subsys power on */
	mtk_regmap_set_bits(scpsys->base, pd->data->pwr_on_offs,
			    pd->data->pwr_on_bit);
	mtk_regmap_set_bits(scpsys->base, pd->data->pwr_on_2nd_offs,
			    pd->data->pwr_on_2nd_bit);

	/* wait until PWR_ACK = 1 */
	ret = readx_poll_timeout(scpsys_domain_is_on, pd, tmp, tmp,
				 MTK_POLL_DELAY_US, MTK_POLL_TIMEOUT);
	if (ret < 0)
		goto err_pwr_ack;
	udelay(30);

	if (pd->data->pwr_clamp_bit) {
		mtk_regmap_clear_bits(scpsys->base, pd->data->pwr_on_offs,
				      pd->data->pwr_clamp_bit);
		udelay(30);
	}

	if (pd->data->pwr_rst_bit)
		mtk_regmap_set_bits(scpsys->base, pd->data->pwr_on_offs,
				    pd->data->pwr_rst_bit);

	ret = clk_bulk_prepare_enable(pd->num_subsys_clks, pd->subsys_clks);
	if (ret)
		goto err_disable_subsys_clks;

	ret = scpsys_sram_enable(pd);
	if (ret < 0)
		goto err_disable_sram;

	return 0;

err_disable_sram:
	scpsys_sram_disable(pd);
err_disable_subsys_clks:
	clk_bulk_disable_unprepare(pd->num_subsys_clks, pd->subsys_clks);
err_pwr_ack:
	clk_bulk_disable_unprepare(pd->num_clks, pd->clks);
err_reg:
	scpsys_regulator_disable(genpd);
	return ret;
}

static int scpsys_power_off(struct generic_pm_domain *genpd)
{
	struct scpsys_domain *pd =
		container_of(genpd, struct scpsys_domain, genpd);
	struct scpsys *scpsys = pd->scpsys;
	bool tmp;
	int ret;

	ret = scpsys_sram_disable(pd);
	if (ret < 0)
		return ret;

	clk_bulk_disable_unprepare(pd->num_subsys_clks, pd->subsys_clks);

	if (pd->data->pwr_clamp_bit) {
		mtk_regmap_set_bits(scpsys->base, pd->data->pwr_on_offs,
				    pd->data->pwr_clamp_bit);
		udelay(30);
	}
	if (pd->data->pwr_rst_bit)
		mtk_regmap_clear_bits(scpsys->base, pd->data->pwr_on_offs,
				      pd->data->pwr_rst_bit);

	mtk_regmap_clear_bits(scpsys->base, pd->data->pwr_on_offs,
			      pd->data->pwr_on_bit);
	mtk_regmap_clear_bits(scpsys->base, pd->data->pwr_on_2nd_offs,
			      pd->data->pwr_on_2nd_bit);

	clk_bulk_disable_unprepare(pd->num_clks, pd->clks);
	scpsys_regulator_disable(genpd);

	return 0;
}

static struct generic_pm_domain *scpsys_add_one_domain(struct scpsys *scpsys,
						       struct device_node *node)
{
	const struct scpsys_domain_data *domain_data;
	struct scpsys_domain *pd;
	struct property *prop;
	const char *clk_name;
	int i, ret, num_clks;
	struct clk *clk;
	struct device_node *smi_node;
	struct device_node *larb_node;
	int clk_ind = 0;
	u32 id;

	ret = of_property_read_u32(node, "reg", &id);
	if (ret) {
		dev_err(scpsys->dev,
			"%pOF: failed to retrieve domain id from reg: %d\n",
			node, ret);
		return ERR_PTR(-EINVAL);
	}

	if (id >= scpsys->soc_data->num_domains) {
		dev_err(scpsys->dev, "%pOF: invalid domain id %d\n", node, id);
		return ERR_PTR(-EINVAL);
	}

	domain_data = &scpsys->soc_data->domains_data[id];
	if (domain_data->sta_mask == 0) {
		dev_err(scpsys->dev, "%pOF: undefined domain id %d\n", node,
			id);
		return ERR_PTR(-EINVAL);
	}

	pd = devm_kzalloc(scpsys->dev, sizeof(*pd), GFP_KERNEL);
	if (!pd)
		return ERR_PTR(-ENOMEM);

	pd->data = domain_data;
	pd->scpsys = scpsys;

	num_clks = of_clk_get_parent_count(node);
	if (num_clks > 0) {
		/* Calculate number of subsys_clks */
		of_property_for_each_string(node, "clock-names", prop,
					     clk_name) {
			char *subsys;

			subsys = strchr(clk_name, '-');
			if (subsys)
				pd->num_subsys_clks++;
			else
				pd->num_clks++;
		}

		pd->clks = devm_kcalloc(scpsys->dev, pd->num_clks,
					sizeof(*pd->clks), GFP_KERNEL);
		if (!pd->clks)
			return ERR_PTR(-ENOMEM);

		pd->subsys_clks =
			devm_kcalloc(scpsys->dev, pd->num_subsys_clks,
				     sizeof(*pd->subsys_clks), GFP_KERNEL);
		if (!pd->subsys_clks)
			return ERR_PTR(-ENOMEM);
	}

	for (i = 0; i < pd->num_clks; i++) {
		clk = of_clk_get(node, i);
		if (IS_ERR(clk)) {
			ret = PTR_ERR(clk);
			dev_err(scpsys->dev,
				"%pOF: failed to get clk at index %d: %d\n",
				node, i, ret);
			goto err_put_clocks;
		}

		pd->clks[clk_ind++].clk = clk;
	}

	for (i = 0; i < pd->num_subsys_clks; i++) {
		clk = of_clk_get(node, i + clk_ind);
		if (IS_ERR(clk)) {
			ret = PTR_ERR(clk);
			dev_err(scpsys->dev,
				"%pOF: failed to get clk at index %d: %d\n",
				node, i + clk_ind, ret);
			goto err_put_subsys_clocks;
		}

		pd->subsys_clks[i].clk = clk;
	}

	/*
	 * Initially turn on all domains to make the domains usable
	 * with !CONFIG_PM and to get the hardware in sync with the
	 * software.  The unused domains will be switched off during
	 * late_init time.
	 */
	if (MTK_SCPD_CAPS(pd, MTK_SCPD_KEEP_DEFAULT_OFF)) {
		if (scpsys_domain_is_on(pd))
			dev_warn(
				scpsys->dev,
				"%pOF: A default off power domain has been ON\n",
				node);
	} else {
		ret = scpsys_power_on(&pd->genpd);
		if (ret < 0) {
			dev_err(scpsys->dev,
				"%pOF: failed to power on domain: %d\n", node,
				ret);
			goto err_put_subsys_clocks;
		}
	}

	if (scpsys->domains[id]) {
		ret = -EINVAL;
		dev_err(scpsys->dev,
			"power domain with id %d already exists, check your device-tree\n",
			id);
		goto err_put_subsys_clocks;
	}

	pd->genpd.name = node->name;
	pd->genpd.power_off = scpsys_power_off;
	pd->genpd.power_on = scpsys_power_on;

	if (MTK_SCPD_CAPS(pd, MTK_SCPD_ACTIVE_WAKEUP))
		pd->genpd.flags |= GENPD_FLAG_ACTIVE_WAKEUP;

	if (MTK_SCPD_CAPS(pd, MTK_SCPD_KEEP_DEFAULT_OFF))
		pm_genpd_init(&pd->genpd, NULL, true);
	else
		pm_genpd_init(&pd->genpd, NULL, false);

	scpsys->domains[id] = &pd->genpd;

	return scpsys->pd_data.domains[id];

err_put_subsys_clocks:
	clk_bulk_put(pd->num_subsys_clks, pd->subsys_clks);
err_put_clocks:
	clk_bulk_put(pd->num_clks, pd->clks);
	return ERR_PTR(ret);
}

static int scpsys_add_subdomain(struct scpsys *scpsys,
				struct device_node *parent)
{
	struct generic_pm_domain *child_pd, *parent_pd;
	struct device_node *child;
	int ret;

	for_each_child_of_node(parent, child) {
		u32 id;

		ret = of_property_read_u32(parent, "reg", &id);
		if (ret) {
			dev_err(scpsys->dev,
				"%pOF: failed to get parent domain id\n",
				child);
			goto err_put_node;
		}

		if (!scpsys->pd_data.domains[id]) {
			ret = -EINVAL;
			dev_err(scpsys->dev,
				"power domain with id %d does not exist\n", id);
			goto err_put_node;
		}

		parent_pd = scpsys->pd_data.domains[id];

		child_pd = scpsys_add_one_domain(scpsys, child);
		if (IS_ERR(child_pd)) {
			ret = PTR_ERR(child_pd);
			dev_err(scpsys->dev,
				"%pOF: failed to get child domain id\n", child);
			goto err_put_node;
		}

		ret = pm_genpd_add_subdomain(parent_pd, child_pd);
		if (ret) {
			dev_err(scpsys->dev,
				"failed to add %s subdomain to parent %s\n",
				child_pd->name, parent_pd->name);
			goto err_put_node;
		} else {
			dev_dbg(scpsys->dev, "%s add subdomain: %s\n",
				parent_pd->name, child_pd->name);
		}

		/* recursive call to add all subdomains */
		ret = scpsys_add_subdomain(scpsys, child);
		if (ret)
			goto err_put_node;
	}

	return 0;

err_put_node:
	of_node_put(child);
	return ret;
}

static void scpsys_remove_one_domain(struct scpsys_domain *pd)
{
	int ret;

	if (scpsys_domain_is_on(pd))
		scpsys_power_off(&pd->genpd);

	/*
	 * We're in the error cleanup already, so we only complain,
	 * but won't emit another error on top of the original one.
	 */
	ret = pm_genpd_remove(&pd->genpd);
	if (ret < 0)
		dev_err(pd->scpsys->dev,
			"failed to remove domain '%s' : %d - state may be inconsistent\n",
			pd->genpd.name, ret);

	clk_bulk_put(pd->num_clks, pd->clks);
	clk_bulk_put(pd->num_subsys_clks, pd->subsys_clks);
}

static void scpsys_domain_cleanup(struct scpsys *scpsys)
{
	struct generic_pm_domain *genpd;
	struct scpsys_domain *pd;
	int i;

	for (i = scpsys->pd_data.num_domains - 1; i >= 0; i--) {
		genpd = scpsys->pd_data.domains[i];
		if (genpd) {
			pd = to_scpsys_domain(genpd);
			scpsys_remove_one_domain(pd);
		}
	}
}

static const struct of_device_id scpsys_of_match[] = {
	{
		.compatible = "mediatek,mt7988-power-controller",
		.data = &mt7988_scpsys_data,
	},
	{}
};

static int scpsys_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	const struct scpsys_soc_data *soc;
	struct device_node *node;
	struct scpsys *scpsys;
	struct resource *res;
	int ret;

	soc = of_device_get_match_data(&pdev->dev);
	if (!soc) {
		dev_err(&pdev->dev, "no power controller data\n");
		return -EINVAL;
	}

	scpsys = devm_kzalloc(dev,
			      struct_size(scpsys, domains, soc->num_domains),
			      GFP_KERNEL);
	if (!scpsys)
		return -ENOMEM;

	scpsys->dev = dev;
	scpsys->soc_data = soc;

	scpsys->pd_data.domains = scpsys->domains;
	scpsys->pd_data.num_domains = soc->num_domains;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	scpsys->base = syscon_node_to_regmap(np);
	if (IS_ERR(scpsys->base)) {
		dev_err(dev, "no regmap available\n");
		return PTR_ERR(scpsys->base);
	}

	ret = -ENODEV;
	for_each_available_child_of_node(np, node) {
		struct generic_pm_domain *domain;

		domain = scpsys_add_one_domain(scpsys, node);
		if (IS_ERR(domain)) {
			ret = PTR_ERR(domain);
			of_node_put(node);
			goto err_cleanup_domains;
		}

		ret = scpsys_add_subdomain(scpsys, node);
		if (ret) {
			of_node_put(node);
			goto err_cleanup_domains;
		}
	}

	if (ret) {
		dev_dbg(dev, "no power domains present\n");
		return ret;
	}

	ret = of_genpd_add_provider_onecell(np, &scpsys->pd_data);
	if (ret) {
		dev_err(dev, "failed to add provider: %d\n", ret);
		goto err_cleanup_domains;
	}

	return 0;

err_cleanup_domains:
	scpsys_domain_cleanup(scpsys);
	return ret;
}

static struct platform_driver scpsys_pm_domain_driver = {
	.probe = scpsys_probe,
	.driver = {
		.name = "mtk-power-controller",
		.suppress_bind_attrs = true,
		.of_match_table = scpsys_of_match,
	},
};
builtin_platform_driver(scpsys_pm_domain_driver);
