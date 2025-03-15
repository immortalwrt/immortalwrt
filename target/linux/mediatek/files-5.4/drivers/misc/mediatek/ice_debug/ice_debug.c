/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regmap.h>
#include <linux/clk.h>
#include <linux/of_address.h>
#include <linux/clk.h>
#include <linux/io.h>

static const struct of_device_id mt2701_icedbg_match[] = {
	{.compatible = "mediatek,mt2701-ice_debug", },
	{ }
};
MODULE_DEVICE_TABLE(of, mt2701_icedbg_match);

static int mtk_ice_debug_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct clk *clk_icedbg;

	clk_icedbg = devm_clk_get(&pdev->dev, "ice_dbg");
	if (IS_ERR(clk_icedbg)) {
		dev_err(&pdev->dev, "get ice_dbg clock fail: %ld\n",
				PTR_ERR(clk_icedbg));
		return PTR_ERR(clk_icedbg);
	}

	ret = clk_prepare_enable(clk_icedbg);
	if (ret)
		return ret;

	/*enable CK_TOP_ARM_DB_JTSEL clk*/
	clk_icedbg = devm_clk_get(&pdev->dev, "dbg_jtsel");
	if (IS_ERR(clk_icedbg)) {
		dev_err(&pdev->dev, "get dbg_sel clock fail: %ld\n",
				PTR_ERR(clk_icedbg));
		return PTR_ERR(clk_icedbg);
	}

	ret = clk_prepare_enable(clk_icedbg);
	if (ret)
		return ret;

	return 0;
}

static struct platform_driver mtk_icedbg_driver = {
	.probe = mtk_ice_debug_probe,
	.driver = {
		.name = "mediatek,mt2701-ice_debug",
		.owner = THIS_MODULE,
		.of_match_table = mt2701_icedbg_match,
	},
};

static int __init mtk_ice_debug_init(void)
{
	return platform_driver_register(&mtk_icedbg_driver);
}
module_init(mtk_ice_debug_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MediaTek MT2701 ICE_DEBUG Driver");
MODULE_AUTHOR("Maoguang Meng <maoguang.meng@mediatek.com>");
