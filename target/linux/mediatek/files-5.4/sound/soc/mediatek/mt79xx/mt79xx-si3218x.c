// SPDX-License-Identifier: GPL-2.0
/*
 * mt79xx-si3218x.c  --  MT79xx WM8960 ALSA SoC machine driver
 *
 * Copyright (c) 2021 MediaTek Inc.
 * Author: Vic Wu <vic.wu@mediatek.com>
 */

#include <linux/module.h>
#include <sound/soc.h>

#include "mt79xx-afe-clk.h"
#include "mt79xx-afe-common.h"
#include "mt79xx-reg.h"
#include "../common/mtk-afe-platform-driver.h"

enum {
	HOPPING_CLK = 0,
	APLL_CLK = 1,
};

enum {
	I2S = 0,
	PCMA = 4,
	PCMB,
};

enum {
	ETDM_IN5 = 2,
	ETDM_OUT5 = 10,
};

enum {
	AFE_FS_8K = 0,
	AFE_FS_11K = 1,
	AFE_FS_12K = 2,
	AFE_FS_16K = 4,
	AFE_FS_22K = 5,
	AFE_FS_24K = 6,
	AFE_FS_32K = 8,
	AFE_FS_44K = 9,
	AFE_FS_48K = 10,
	AFE_FS_88K = 13,
	AFE_FS_96K = 14,
	AFE_FS_176K = 17,
	AFE_FS_192K = 18,
};

enum {
	ETDM_FS_8K = 0,
	ETDM_FS_12K = 1,
	ETDM_FS_16K = 2,
	ETDM_FS_24K = 3,
	ETDM_FS_32K = 4,
	ETDM_FS_48K = 5,
	ETDM_FS_96K = 7,
	ETDM_FS_192K = 9,
	ETDM_FS_11K = 16,
	ETDM_FS_22K = 17,
	ETDM_FS_44K = 18,
	ETDM_FS_88K = 19,
	ETDM_FS_176K = 20,
};

struct mt79xx_si3218x_priv {
	struct device_node *platform_node;
	struct device_node *codec_node;
};

static int mt79xx_si3218x_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_component *component =
		snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
	struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);

	/* enable clk */
	mt79xx_afe_enable_clock(afe);
	regmap_update_bits(afe->regmap, AUDIO_TOP_CON2, CLK_OUT5_PDN_MASK,
			   0);
	regmap_update_bits(afe->regmap, AUDIO_TOP_CON2, CLK_IN5_PDN_MASK,
			   0);
	regmap_update_bits(afe->regmap, AUDIO_TOP_CON4, 0x3fff, 0);
	regmap_update_bits(afe->regmap, AUDIO_ENGEN_CON0, AUD_APLL2_EN_MASK,
			   AUD_APLL2_EN);
	regmap_update_bits(afe->regmap, AUDIO_ENGEN_CON0, AUD_26M_EN_MASK,
			   AUD_26M_EN);

	/* set ETDM_IN5_CON0 */
	regmap_update_bits(afe->regmap, ETDM_IN5_CON0, ETDM_SYNC_MASK,
			   ETDM_SYNC);
	regmap_update_bits(afe->regmap, ETDM_IN5_CON0, ETDM_FMT_MASK,
			   ETDM_FMT(PCMA));
	regmap_update_bits(afe->regmap, ETDM_IN5_CON0, ETDM_BIT_LEN_MASK,
			   ETDM_BIT_LEN(16));
	regmap_update_bits(afe->regmap, ETDM_IN5_CON0, ETDM_WRD_LEN_MASK,
			   ETDM_WRD_LEN(16));
	regmap_update_bits(afe->regmap, ETDM_IN5_CON0, ETDM_CH_NUM_MASK,
			   ETDM_CH_NUM(4));
	regmap_update_bits(afe->regmap, ETDM_IN5_CON0, RELATCH_SRC_MASK,
			   RELATCH_SRC(APLL_CLK));

	/* set ETDM_IN5_CON2 */
	regmap_update_bits(afe->regmap, ETDM_IN5_CON2, IN_CLK_SRC_MASK,
			   IN_CLK_SRC(APLL_CLK));

	/* set ETDM_IN5_CON3 */
	regmap_update_bits(afe->regmap, ETDM_IN5_CON3, IN_SEL_FS_MASK,
			   IN_SEL_FS(ETDM_FS_16K));

	/* set ETDM_IN5_CON4 */
	regmap_update_bits(afe->regmap, ETDM_IN5_CON4, IN_CLK_INV_MASK,
			   IN_CLK_INV);
	regmap_update_bits(afe->regmap, ETDM_IN5_CON4, IN_RELATCH_MASK,
			   IN_RELATCH(AFE_FS_16K));

	/* set ETDM_OUT5_CON0 */
	regmap_update_bits(afe->regmap, ETDM_OUT5_CON0, ETDM_FMT_MASK,
			   ETDM_FMT(PCMA));
	regmap_update_bits(afe->regmap, ETDM_OUT5_CON0, ETDM_BIT_LEN_MASK,
			   ETDM_BIT_LEN(16));
	regmap_update_bits(afe->regmap, ETDM_OUT5_CON0, ETDM_WRD_LEN_MASK,
			   ETDM_WRD_LEN(16));
	regmap_update_bits(afe->regmap, ETDM_OUT5_CON0, ETDM_CH_NUM_MASK,
			   ETDM_CH_NUM(4));
	regmap_update_bits(afe->regmap, ETDM_OUT5_CON0, RELATCH_SRC_MASK,
			   RELATCH_SRC(APLL_CLK));

	/* set ETDM_OUT5_CON4 */
	regmap_update_bits(afe->regmap, ETDM_OUT5_CON4, OUT_SEL_FS_MASK,
			   OUT_SEL_FS(ETDM_FS_16K));
	regmap_update_bits(afe->regmap, ETDM_OUT5_CON4, OUT_CLK_SRC_MASK,
			   OUT_CLK_SRC(APLL_CLK));
	regmap_update_bits(afe->regmap, ETDM_OUT5_CON4, OUT_RELATCH_MASK,
			   OUT_RELATCH(AFE_FS_16K));

	/* set ETDM_OUT5_CON5 */
	regmap_update_bits(afe->regmap, ETDM_OUT5_CON5, OUT_CLK_INV_MASK,
			   OUT_CLK_INV);
	regmap_update_bits(afe->regmap, ETDM_OUT5_CON5, ETDM_CLK_DIV_MASK,
			   ETDM_CLK_DIV);

	/* set external loopback */
	regmap_update_bits(afe->regmap, ETDM_4_7_COWORK_CON0, OUT_SEL_MASK,
			   OUT_SEL(ETDM_IN5));

	/* enable ETDM */
	regmap_update_bits(afe->regmap, ETDM_IN5_CON0, ETDM_EN_MASK,
			   ETDM_EN);
	regmap_update_bits(afe->regmap, ETDM_OUT5_CON0, ETDM_EN_MASK,
			   ETDM_EN);

	return 0;
}

SND_SOC_DAILINK_DEFS(playback,
	DAILINK_COMP_ARRAY(COMP_CPU("DL1")),
	DAILINK_COMP_ARRAY(COMP_DUMMY()),
	DAILINK_COMP_ARRAY(COMP_EMPTY()));

SND_SOC_DAILINK_DEFS(capture,
	DAILINK_COMP_ARRAY(COMP_CPU("UL1")),
	DAILINK_COMP_ARRAY(COMP_DUMMY()),
	DAILINK_COMP_ARRAY(COMP_EMPTY()));

SND_SOC_DAILINK_DEFS(codec,
	DAILINK_COMP_ARRAY(COMP_CPU("ETDM")),
	DAILINK_COMP_ARRAY(COMP_CODEC(NULL, "proslic_spi-aif")),
	DAILINK_COMP_ARRAY(COMP_EMPTY()));

static struct snd_soc_dai_link mt79xx_si3218x_dai_links[] = {
	/* FE */
	{
		.name = "si3218x-playback",
		.stream_name = "si3218x-playback",
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			    SND_SOC_DPCM_TRIGGER_POST},
		.dynamic = 1,
		.dpcm_playback = 1,
		SND_SOC_DAILINK_REG(playback),
	},
	{
		.name = "si3218x-capture",
		.stream_name = "si3218x-capture",
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			    SND_SOC_DPCM_TRIGGER_POST},
		.dynamic = 1,
		.dpcm_capture = 1,
		SND_SOC_DAILINK_REG(capture),
	},
	/* BE */
	{
		.name = "si3218x-codec",
		.no_pcm = 1,
		.dai_fmt = SND_SOC_DAIFMT_DSP_A |
			SND_SOC_DAIFMT_IB_NF |
			SND_SOC_DAIFMT_CBS_CFS,
		.init = mt79xx_si3218x_init,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		SND_SOC_DAILINK_REG(codec),
	},
};

static struct snd_soc_card mt79xx_si3218x_card = {
	.name = "mt79xx-si3218x",
	.owner = THIS_MODULE,
	.dai_link = mt79xx_si3218x_dai_links,
	.num_links = ARRAY_SIZE(mt79xx_si3218x_dai_links),
};

static int mt79xx_si3218x_machine_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &mt79xx_si3218x_card;
	struct snd_soc_dai_link *dai_link;
	struct mt79xx_si3218x_priv *priv;
	int ret, i;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->platform_node = of_parse_phandle(pdev->dev.of_node,
					       "mediatek,platform", 0);
	if (!priv->platform_node) {
		dev_err(&pdev->dev, "Property 'platform' missing or invalid\n");
		return -EINVAL;
	}

	for_each_card_prelinks(card, i, dai_link) {
		if (dai_link->platforms->name)
			continue;
		dai_link->platforms->of_node = priv->platform_node;
	}

	card->dev = &pdev->dev;

	priv->codec_node = of_parse_phandle(pdev->dev.of_node,
					    "mediatek,ext-codec", 0);
	if (!priv->codec_node) {
		dev_err(&pdev->dev,
			"Property 'audio-codec' missing or invalid\n");
		of_node_put(priv->platform_node);
		return -EINVAL;
	}

	for_each_card_prelinks(card, i, dai_link) {
		if (dai_link->codecs->name)
			continue;
		dai_link->codecs->of_node = priv->codec_node;
	}

	ret = devm_snd_soc_register_card(&pdev->dev, card);
	if (ret) {
		dev_err(&pdev->dev, "%s snd_soc_register_card fail %d\n",
			__func__, ret);
		of_node_put(priv->codec_node);
		of_node_put(priv->platform_node);
	}

	return ret;
}

static int mt79xx_si3218x_machine_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	struct mt79xx_si3218x_priv *priv = snd_soc_card_get_drvdata(card);

	of_node_put(priv->codec_node);
	of_node_put(priv->platform_node);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id mt79xx_si3218x_machine_dt_match[] = {
	{.compatible = "mediatek,mt79xx-si3218x-machine",},
	{}
};
#endif

static struct platform_driver mt79xx_si3218x_machine = {
	.driver = {
		.name = "mt79xx-si3218x",
#ifdef CONFIG_OF
		.of_match_table = mt79xx_si3218x_machine_dt_match,
#endif
	},
	.probe = mt79xx_si3218x_machine_probe,
	.remove = mt79xx_si3218x_machine_remove,
};

module_platform_driver(mt79xx_si3218x_machine);

/* Module information */
MODULE_DESCRIPTION("MT79xx SI3218x ALSA SoC machine driver");
MODULE_AUTHOR("Vic Wu <vic.wu@mediatek.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("mt79xx si3218x soc card");
