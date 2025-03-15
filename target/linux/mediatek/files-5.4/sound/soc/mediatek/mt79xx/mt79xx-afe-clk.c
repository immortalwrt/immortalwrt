// SPDX-License-Identifier: GPL-2.0
//
// mt79xx-afe-clk.c  --  Mediatek 79xx afe clock ctrl
//
// Copyright (c) 2021 MediaTek Inc.
// Author: Vic Wu <vic.wu@mediatek.com>

#include <linux/clk.h>

#include "mt79xx-afe-common.h"
#include "mt79xx-afe-clk.h"
#include "mt79xx-reg.h"

enum {
	CK_INFRA_AUD_BUS_CK = 0,
	CK_INFRA_AUD_26M_CK,
	CK_INFRA_AUD_L_CK,
	CK_INFRA_AUD_AUD_CK,
	CK_INFRA_AUD_EG2_CK,
	CLK_NUM
};

static const char *aud_clks[CLK_NUM] = {
	[CK_INFRA_AUD_BUS_CK] = "aud_bus_ck",
	[CK_INFRA_AUD_26M_CK] = "aud_26m_ck",
	[CK_INFRA_AUD_L_CK] = "aud_l_ck",
	[CK_INFRA_AUD_AUD_CK] = "aud_aud_ck",
	[CK_INFRA_AUD_EG2_CK] = "aud_eg2_ck",
};

int mt79xx_init_clock(struct mtk_base_afe *afe)
{
	struct mt79xx_afe_private *afe_priv = afe->platform_priv;
	int i;

	afe_priv->clk = devm_kcalloc(afe->dev, CLK_NUM, sizeof(*afe_priv->clk),
				     GFP_KERNEL);
	if (!afe_priv->clk)
		return -ENOMEM;

	for (i = 0; i < CLK_NUM; i++) {
		afe_priv->clk[i] = devm_clk_get(afe->dev, aud_clks[i]);
		if (IS_ERR(afe_priv->clk[i])) {
			dev_err(afe->dev, "%s(), devm_clk_get %s fail,\
				ret %ld\n", __func__, aud_clks[i],
				PTR_ERR(afe_priv->clk[i]));
			return PTR_ERR(afe_priv->clk[i]);
		}
	}

	return 0;
}

int mt79xx_afe_enable_clock(struct mtk_base_afe *afe)
{
	struct mt79xx_afe_private *afe_priv = afe->platform_priv;
	int ret;

	ret = clk_prepare_enable(afe_priv->clk[CK_INFRA_AUD_BUS_CK]);
	if (ret) {
		dev_err(afe->dev, "%s(), clk_prepare_enable %s fail %d\n",
			__func__, aud_clks[CK_INFRA_AUD_BUS_CK], ret);
		goto CK_INFRA_AUD_BUS_CK_ERR;
	}

	ret = clk_prepare_enable(afe_priv->clk[CK_INFRA_AUD_26M_CK]);
	if (ret) {
		dev_err(afe->dev, "%s(), clk_prepare_enable %s fail %d\n",
			__func__, aud_clks[CK_INFRA_AUD_26M_CK], ret);
		goto CK_INFRA_AUD_26M_ERR;
	}

	ret = clk_prepare_enable(afe_priv->clk[CK_INFRA_AUD_L_CK]);
	if (ret) {
		dev_err(afe->dev, "%s(), clk_prepare_enable %s fail %d\n",
			__func__, aud_clks[CK_INFRA_AUD_L_CK], ret);
		goto CK_INFRA_AUD_L_CK_ERR;
	}

	ret = clk_prepare_enable(afe_priv->clk[CK_INFRA_AUD_AUD_CK]);
	if (ret) {
		dev_err(afe->dev, "%s clk_prepare_enable %s fail %d\n",
			__func__, aud_clks[CK_INFRA_AUD_AUD_CK], ret);
		goto CK_INFRA_AUD_AUD_CK_ERR;
	}

	ret = clk_prepare_enable(afe_priv->clk[CK_INFRA_AUD_EG2_CK]);
	if (ret) {
		dev_err(afe->dev, "%s clk_prepare_enable %s fail %d\n",
			__func__, aud_clks[CK_INFRA_AUD_EG2_CK], ret);
		goto CK_INFRA_AUD_EG2_CK_ERR;
	}

	return 0;

CK_INFRA_AUD_EG2_CK_ERR:
	clk_disable_unprepare(afe_priv->clk[CK_INFRA_AUD_AUD_CK]);
CK_INFRA_AUD_AUD_CK_ERR:
	clk_disable_unprepare(afe_priv->clk[CK_INFRA_AUD_L_CK]);
CK_INFRA_AUD_L_CK_ERR:
	clk_disable_unprepare(afe_priv->clk[CK_INFRA_AUD_26M_CK]);
CK_INFRA_AUD_26M_ERR:
	clk_disable_unprepare(afe_priv->clk[CK_INFRA_AUD_BUS_CK]);
CK_INFRA_AUD_BUS_CK_ERR:
	return ret;
}
EXPORT_SYMBOL_GPL(mt79xx_afe_enable_clock);

int mt79xx_afe_disable_clock(struct mtk_base_afe *afe)
{
	struct mt79xx_afe_private *afe_priv = afe->platform_priv;

	clk_disable_unprepare(afe_priv->clk[CK_INFRA_AUD_EG2_CK]);
	clk_disable_unprepare(afe_priv->clk[CK_INFRA_AUD_AUD_CK]);
	clk_disable_unprepare(afe_priv->clk[CK_INFRA_AUD_L_CK]);
	clk_disable_unprepare(afe_priv->clk[CK_INFRA_AUD_26M_CK]);
	clk_disable_unprepare(afe_priv->clk[CK_INFRA_AUD_BUS_CK]);

	return 0;
}
EXPORT_SYMBOL_GPL(mt79xx_afe_disable_clock);
