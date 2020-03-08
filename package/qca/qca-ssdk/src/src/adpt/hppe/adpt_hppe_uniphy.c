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


/**
 * @defgroup
 * @{
 */
#include "sw.h"
#include "hppe_uniphy_reg.h"
#include "hppe_uniphy.h"
#include "hppe_init.h"
#include "ssdk_init.h"
#include "ssdk_clk.h"
#include "adpt.h"
#include "hppe_reg_access.h"
#include "hsl_phy.h"

#ifdef HAWKEYE_CHIP

extern void adpt_hppe_gcc_port_speed_clock_set(a_uint32_t dev_id,
				a_uint32_t port_id, fal_port_speed_t phy_speed);

static sw_error_t
__adpt_hppe_uniphy_10g_r_linkup(a_uint32_t dev_id, a_uint32_t uniphy_index)
{
	a_uint32_t reg_value = 0;
	a_uint32_t retries = 100, linkup = 0;

	union sr_xs_pcs_kr_sts1_u sr_xs_pcs_kr_sts1;

	memset(&sr_xs_pcs_kr_sts1, 0, sizeof(sr_xs_pcs_kr_sts1));
	ADPT_DEV_ID_CHECK(dev_id);

	/* wait 10G_R link up  to uniphy */
	while (linkup != UNIPHY_10GR_LINKUP) {
		mdelay(1);
		if (retries-- == 0)
			return SW_TIMEOUT;
		reg_value = 0;
		hppe_sr_xs_pcs_kr_sts1_get(0, uniphy_index, &sr_xs_pcs_kr_sts1);
		reg_value = sr_xs_pcs_kr_sts1.bf.plu;
		linkup = (reg_value & UNIPHY_10GR_LINKUP);
	}

	return SW_OK;
}

static sw_error_t
__adpt_hppe_uniphy_calibration(a_uint32_t dev_id, a_uint32_t uniphy_index)
{
	a_uint32_t reg_value = 0;
	a_uint32_t retries = 100, calibration_done = 0;

	union uniphy_offset_calib_4_u uniphy_offset_calib_4;

	memset(&uniphy_offset_calib_4, 0, sizeof(uniphy_offset_calib_4));
	ADPT_DEV_ID_CHECK(dev_id);

	/* wait calibration done to uniphy */
	while (calibration_done != UNIPHY_CALIBRATION_DONE) {
		mdelay(1);
		if (retries-- == 0)
		{
			printk("uniphy callibration time out!\n");
			return SW_TIMEOUT;
		}
		reg_value = 0;
		hppe_uniphy_offset_calib_4_get(0, uniphy_index, &uniphy_offset_calib_4);
		reg_value = uniphy_offset_calib_4.bf.mmd1_reg_calibration_done_reg;

		calibration_done = (reg_value & UNIPHY_CALIBRATION_DONE);
	}

	return SW_OK;
}

static void
__adpt_hppe_gcc_uniphy_xpcs_reset(a_uint32_t dev_id, a_uint32_t uniphy_index, a_bool_t enable)
{
	enum unphy_rst_type rst_type;
	enum ssdk_rst_action rst_action;

	if (uniphy_index == SSDK_UNIPHY_INSTANCE0)
		rst_type = UNIPHY0_XPCS_RESET_E;
	else if (uniphy_index == SSDK_UNIPHY_INSTANCE1)
		rst_type = UNIPHY1_XPCS_RESET_E;
	else
		rst_type = UNIPHY2_XPCS_RESET_E;

	if (enable == A_TRUE)
		rst_action = SSDK_RESET_ASSERT;
	else
		rst_action = SSDK_RESET_DEASSERT;
	
	ssdk_uniphy_reset(dev_id, rst_type, rst_action);

	return;
}

static void
__adpt_hppe_gcc_uniphy_software_reset(a_uint32_t dev_id, a_uint32_t uniphy_index)
{

	enum unphy_rst_type rst_type;

	if (uniphy_index == SSDK_UNIPHY_INSTANCE0)
		rst_type = UNIPHY0_SOFT_RESET_E;
	else if (uniphy_index == SSDK_UNIPHY_INSTANCE1)
		rst_type = UNIPHY1_SOFT_RESET_E;
	else
		rst_type = UNIPHY2_SOFT_RESET_E;

	ssdk_uniphy_reset(dev_id, rst_type, SSDK_RESET_ASSERT);

	msleep(100);

	ssdk_uniphy_reset(dev_id, rst_type, SSDK_RESET_DEASSERT);

	return;
}

static sw_error_t
__adpt_hppe_uniphy_usxgmii_mode_set(a_uint32_t dev_id, a_uint32_t uniphy_index)
{
	sw_error_t rv = SW_OK;

	union uniphy_mode_ctrl_u uniphy_mode_ctrl;
	union vr_xs_pcs_dig_ctrl1_u vr_xs_pcs_dig_ctrl1;
	union vr_mii_an_ctrl_u vr_mii_an_ctrl;
	union sr_mii_ctrl_u sr_mii_ctrl;

	memset(&uniphy_mode_ctrl, 0, sizeof(uniphy_mode_ctrl));
	memset(&vr_xs_pcs_dig_ctrl1, 0, sizeof(vr_xs_pcs_dig_ctrl1));
	memset(&vr_mii_an_ctrl, 0, sizeof(vr_mii_an_ctrl));
	memset(&sr_mii_ctrl, 0, sizeof(sr_mii_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);

	hppe_uniphy_reg_set(dev_id, UNIPHY_MISC2_REG_OFFSET,
		uniphy_index, UNIPHY_MISC2_REG_VALUE);
	/*reset uniphy*/
	hppe_uniphy_reg_set(dev_id, UNIPHY_PLL_RESET_REG_OFFSET,
		uniphy_index, UNIPHY_PLL_RESET_REG_VALUE);
	msleep(500);
	hppe_uniphy_reg_set(dev_id, UNIPHY_PLL_RESET_REG_OFFSET,
		uniphy_index, UNIPHY_PLL_RESET_REG_DEFAULT_VALUE);
	msleep(500);

	/* disable instance clock */
	qca_gcc_uniphy_port_clock_set(dev_id, uniphy_index,
			1, A_FALSE);

	/* keep xpcs to reset status */
	__adpt_hppe_gcc_uniphy_xpcs_reset(dev_id, uniphy_index, A_TRUE);

	/* configure uniphy to usxgmii mode */
	hppe_uniphy_mode_ctrl_get(0, uniphy_index, &uniphy_mode_ctrl);
	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_psgmii_qsgmii = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_qsgmii_sgmii = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_sg_mode = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_sgplus_mode = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_xpcs_mode = 1;
	hppe_uniphy_mode_ctrl_set(0, uniphy_index, &uniphy_mode_ctrl);

	/* configure uniphy usxgmii gcc software reset */
	__adpt_hppe_gcc_uniphy_software_reset(dev_id, uniphy_index);

	msleep(100);

	/* wait calibration done to uniphy */
	__adpt_hppe_uniphy_calibration(dev_id, uniphy_index);

	/* enable instance clock */
	qca_gcc_uniphy_port_clock_set(dev_id, uniphy_index,
			1, A_TRUE);

	/* release xpcs reset status */
	__adpt_hppe_gcc_uniphy_xpcs_reset(dev_id, uniphy_index, A_FALSE);

	/* wait 10g base_r link up */
	__adpt_hppe_uniphy_10g_r_linkup(dev_id, uniphy_index);

	/* enable uniphy usxgmii */
	hppe_vr_xs_pcs_dig_ctrl1_get(0, uniphy_index, &vr_xs_pcs_dig_ctrl1);
	vr_xs_pcs_dig_ctrl1.bf.usxg_en = 1;
	hppe_vr_xs_pcs_dig_ctrl1_set(0, uniphy_index, &vr_xs_pcs_dig_ctrl1);

	/* enable uniphy autoneg complete interrupt and 10M/100M 8-bits MII width */
	hppe_vr_mii_an_ctrl_get(0, uniphy_index, &vr_mii_an_ctrl);
	vr_mii_an_ctrl.bf.mii_an_intr_en = 1;
	vr_mii_an_ctrl.bf.mii_ctrl = 1;
	hppe_vr_mii_an_ctrl_set(0, uniphy_index, &vr_mii_an_ctrl);

	/* enable uniphy autoneg ability and usxgmii 10g speed and full duplex */
	hppe_sr_mii_ctrl_get(0, uniphy_index, &sr_mii_ctrl);
	sr_mii_ctrl.bf.an_enable = 1;
	sr_mii_ctrl.bf.ss5 = 0;
	sr_mii_ctrl.bf.ss6 = 1;
	sr_mii_ctrl.bf.ss13 = 1;
	sr_mii_ctrl.bf.duplex_mode = 1;
	hppe_sr_mii_ctrl_set(0, uniphy_index, &sr_mii_ctrl);

	/*This hardcode will move to aq phy driver init when it's available*/
	if (uniphy_index == SSDK_UNIPHY_INSTANCE2)
		qca_ar8327_phy_write(0, 7, (1<<30) | (4<<16) | 0xc441, 8);
	return rv;
}

static sw_error_t
__adpt_hppe_uniphy_10g_r_mode_set(a_uint32_t dev_id, a_uint32_t uniphy_index)
{
	a_uint32_t port_id = 0;
	sw_error_t rv = SW_OK;

	union uniphy_mode_ctrl_u uniphy_mode_ctrl;
	union uniphy_instance_link_detect_u uniphy_instance_link_detect;

	memset(&uniphy_mode_ctrl, 0, sizeof(uniphy_mode_ctrl));
	memset(&uniphy_instance_link_detect, 0, sizeof(uniphy_instance_link_detect));
	ADPT_DEV_ID_CHECK(dev_id);

	/* keep xpcs to reset status */
	__adpt_hppe_gcc_uniphy_xpcs_reset(dev_id, uniphy_index, A_TRUE);

	/* disable instance clock */
	qca_gcc_uniphy_port_clock_set(dev_id, uniphy_index,
				1, A_FALSE);

	/* configure uniphy to 10g_r mode */
	hppe_uniphy_mode_ctrl_get(0, uniphy_index, &uniphy_mode_ctrl);

	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_psgmii_qsgmii = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_qsgmii_sgmii = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_sg_mode = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_sgplus_mode = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_xpcs_mode = 1;

	hppe_uniphy_mode_ctrl_set(0, uniphy_index, &uniphy_mode_ctrl);

	hppe_uniphy_instance_link_detect_get(dev_id, uniphy_index, &uniphy_instance_link_detect);
	uniphy_instance_link_detect.bf.detect_los_from_sfp = UNIPHY_10GR_LINK_LOSS;
	hppe_uniphy_instance_link_detect_set(dev_id, uniphy_index, &uniphy_instance_link_detect);

	/* configure uniphy gcc software reset */
	__adpt_hppe_gcc_uniphy_software_reset(dev_id, uniphy_index);

	/* wait uniphy calibration done */
	rv = __adpt_hppe_uniphy_calibration(dev_id, uniphy_index);

	/* configure gcc speed clock to 10g r mode*/
	if (uniphy_index == SSDK_UNIPHY_INSTANCE1)
		port_id = HPPE_MUX_PORT1;
	else if (uniphy_index == SSDK_UNIPHY_INSTANCE2)
		port_id = HPPE_MUX_PORT2;
	adpt_hppe_gcc_port_speed_clock_set(dev_id, port_id, FAL_SPEED_10000);

	/* enable instance clock */
	qca_gcc_uniphy_port_clock_set(dev_id, uniphy_index,
			1, A_TRUE);

	/* release xpcs reset status */
	__adpt_hppe_gcc_uniphy_xpcs_reset(dev_id, uniphy_index, A_FALSE);

	return rv;
}

static sw_error_t
__adpt_hppe_uniphy_sgmiiplus_mode_set(a_uint32_t dev_id, a_uint32_t uniphy_index)
{
	sw_error_t rv = SW_OK;

	union uniphy_mode_ctrl_u uniphy_mode_ctrl;

	memset(&uniphy_mode_ctrl, 0, sizeof(uniphy_mode_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);

	hppe_uniphy_reg_set(dev_id, UNIPHY_MISC2_REG_OFFSET,
		uniphy_index, UNIPHY_MISC2_REG_SGMII_PLUS_MODE);
	/*reset uniphy*/
	hppe_uniphy_reg_set(dev_id, UNIPHY_PLL_RESET_REG_OFFSET,
		uniphy_index, UNIPHY_PLL_RESET_REG_VALUE);
	msleep(500);
	hppe_uniphy_reg_set(dev_id, UNIPHY_PLL_RESET_REG_OFFSET,
		uniphy_index, UNIPHY_PLL_RESET_REG_DEFAULT_VALUE);
	msleep(500);

	/* keep xpcs to reset status */
	__adpt_hppe_gcc_uniphy_xpcs_reset(dev_id, uniphy_index, A_TRUE);

	/* disable instance clock */
	qca_gcc_uniphy_port_clock_set(dev_id, uniphy_index,
				1, A_FALSE);

	/* configure uniphy to Athr mode and sgmiiplus mode */
	hppe_uniphy_mode_ctrl_get(0, uniphy_index, &uniphy_mode_ctrl);

	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_athr_csco_mode_25m = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_psgmii_qsgmii = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_qsgmii_sgmii = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_sg_mode = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_sgplus_mode = 1;
	uniphy_mode_ctrl.bf.newaddedfromhere_xpcs_mode = 0;

	hppe_uniphy_mode_ctrl_set(0, uniphy_index, &uniphy_mode_ctrl);

	/* configure uniphy gcc software reset */
	__adpt_hppe_gcc_uniphy_software_reset(dev_id, uniphy_index);

	/* wait uniphy calibration done */
	rv = __adpt_hppe_uniphy_calibration(dev_id, uniphy_index);

	/* enable instance clock */
	qca_gcc_uniphy_port_clock_set(dev_id, uniphy_index,
				1, A_TRUE);
	return rv;
}

static sw_error_t
__adpt_hppe_uniphy_sgmii_mode_set(a_uint32_t dev_id, a_uint32_t uniphy_index, a_uint32_t channel)
{
	a_uint32_t i, max_port;
	sw_error_t rv = SW_OK;

	union uniphy_mode_ctrl_u uniphy_mode_ctrl;

	memset(&uniphy_mode_ctrl, 0, sizeof(uniphy_mode_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);

	/*set the PHY mode to SGMII*/
	hppe_uniphy_reg_set(dev_id, UNIPHY_MISC2_REG_OFFSET,
		uniphy_index, UNIPHY_MISC2_REG_SGMII_MODE);
	/*reset uniphy*/
	hppe_uniphy_reg_set(dev_id, UNIPHY_PLL_RESET_REG_OFFSET,
		uniphy_index, UNIPHY_PLL_RESET_REG_VALUE);
	msleep(500);
	hppe_uniphy_reg_set(dev_id, UNIPHY_PLL_RESET_REG_OFFSET,
		uniphy_index, UNIPHY_PLL_RESET_REG_DEFAULT_VALUE);
	msleep(500);

	/* keep xpcs to reset status */
	__adpt_hppe_gcc_uniphy_xpcs_reset(dev_id, uniphy_index, A_TRUE);

	/* disable instance clock */
	if (uniphy_index == SSDK_UNIPHY_INSTANCE0)
		max_port = 5;
	else
		max_port = 1;

	for (i = 1; i <= max_port; i++)
	{
		qca_gcc_uniphy_port_clock_set(dev_id, uniphy_index,
			i, A_FALSE);
	}

	/* configure uniphy to Athr mode and sgmii mode */
	hppe_uniphy_mode_ctrl_get(0, uniphy_index, &uniphy_mode_ctrl);
	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_athr_csco_mode_25m = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_psgmii_qsgmii = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_qsgmii_sgmii = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_sgplus_mode = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_xpcs_mode = 0;
	if (uniphy_index == SSDK_UNIPHY_INSTANCE0) {
		uniphy_mode_ctrl.bf.newaddedfromhere_sg_mode = 0;
		/* select channel as a sgmii interface */
		if (channel == 0)
		{
			uniphy_mode_ctrl.bf.newaddedfromhere_ch1_ch0_sgmii = 0;
			uniphy_mode_ctrl.bf.newaddedfromhere_ch4_ch1_0_sgmii = 0;
		}
		else if (channel == 1)
		{
			uniphy_mode_ctrl.bf.newaddedfromhere_ch1_ch0_sgmii = 1;
			uniphy_mode_ctrl.bf.newaddedfromhere_ch4_ch1_0_sgmii = 0;
		}
		else if (channel == 4)
		{
			uniphy_mode_ctrl.bf.newaddedfromhere_ch1_ch0_sgmii = 0;
			uniphy_mode_ctrl.bf.newaddedfromhere_ch4_ch1_0_sgmii = 1;
		}
	}
	else
	{
		uniphy_mode_ctrl.bf.newaddedfromhere_sg_mode = 1;
	}

	hppe_uniphy_mode_ctrl_set(0, uniphy_index, &uniphy_mode_ctrl);

	/* configure uniphy gcc software reset */
	__adpt_hppe_gcc_uniphy_software_reset(dev_id, uniphy_index);

	/* wait uniphy calibration done */
	rv = __adpt_hppe_uniphy_calibration(dev_id, uniphy_index);

	/* enable instance clock */
	if (uniphy_index == SSDK_UNIPHY_INSTANCE0)
		max_port = 5;
	else
		max_port = 1;

	for (i = 1; i <= max_port; i++)
	{
		qca_gcc_uniphy_port_clock_set(dev_id, uniphy_index,
			i, A_TRUE);
	}

	return rv;
}

static sw_error_t
__adpt_hppe_uniphy_qsgmii_mode_set(a_uint32_t dev_id, a_uint32_t uniphy_index)
{
	a_uint32_t i;
	sw_error_t rv = SW_OK;

	union uniphy_mode_ctrl_u uniphy_mode_ctrl;

	memset(&uniphy_mode_ctrl, 0, sizeof(uniphy_mode_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);

	/* configure malibu phy to qsgmii mode*/
	hsl_ssdk_phy_mode_set(dev_id, PORT_QSGMII);

	/* keep xpcs to reset status */
	__adpt_hppe_gcc_uniphy_xpcs_reset(dev_id, uniphy_index, A_TRUE);

	/* disable instance0 clock */
	for (i = 1; i < 6; i++)
	{
		qca_gcc_uniphy_port_clock_set(dev_id, uniphy_index,
			i, A_FALSE);
	}

	/* configure uniphy to Athr mode and qsgmii mode */
	hppe_uniphy_mode_ctrl_get(0, uniphy_index, &uniphy_mode_ctrl);
	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_athr_csco_mode_25m = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_psgmii_qsgmii = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_qsgmii_sgmii = 1;
	uniphy_mode_ctrl.bf.newaddedfromhere_sg_mode = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_sgplus_mode = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_xpcs_mode = 0;
	hppe_uniphy_mode_ctrl_set(0, uniphy_index, &uniphy_mode_ctrl);

	/* configure uniphy gcc software reset */
	__adpt_hppe_gcc_uniphy_software_reset(dev_id, uniphy_index);

	/* wait uniphy calibration done */
	rv = __adpt_hppe_uniphy_calibration(dev_id, uniphy_index);

	hsl_ssdk_phy_serdes_reset(dev_id);

	/* enable instance0 clock */
	for (i = 1; i < 6; i++)
	{
		qca_gcc_uniphy_port_clock_set(dev_id, uniphy_index,
			i, A_TRUE);
	}

	return rv;
}

static sw_error_t
__adpt_hppe_uniphy_psgmii_mode_set(a_uint32_t dev_id, a_uint32_t uniphy_index)
{
	a_uint32_t i;
	sw_error_t rv = SW_OK;

	union uniphy_mode_ctrl_u uniphy_mode_ctrl;

	memset(&uniphy_mode_ctrl, 0, sizeof(uniphy_mode_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);

	/* keep xpcs to reset status */
	__adpt_hppe_gcc_uniphy_xpcs_reset(dev_id, uniphy_index, A_TRUE);

	/* disable instance0 clock */
	for (i = 1; i < 6; i++)
	{
		qca_gcc_uniphy_port_clock_set(dev_id, uniphy_index,
			i, A_FALSE);
	}

	/* configure uniphy to Athr mode and psgmii mode */
	hppe_uniphy_mode_ctrl_get(0, uniphy_index, &uniphy_mode_ctrl);
	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_athr_csco_mode_25m = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_psgmii_qsgmii = 1;
	uniphy_mode_ctrl.bf.newaddedfromhere_ch0_qsgmii_sgmii = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_sg_mode = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_sgplus_mode = 0;
	uniphy_mode_ctrl.bf.newaddedfromhere_xpcs_mode = 0;
	hppe_uniphy_mode_ctrl_set(0, uniphy_index, &uniphy_mode_ctrl);

	/* configure uniphy gcc software reset */
	__adpt_hppe_gcc_uniphy_software_reset(dev_id, uniphy_index);

	/* wait uniphy calibration done */
	rv = __adpt_hppe_uniphy_calibration(dev_id, uniphy_index);

	hsl_ssdk_phy_serdes_reset(dev_id);

	/* enable instance0 clock */
	for (i = 1; i < 6; i++)
	{
		qca_gcc_uniphy_port_clock_set(dev_id, uniphy_index,
			i, A_TRUE);
	}

	return rv;
}

sw_error_t
adpt_hppe_uniphy_mode_set(a_uint32_t dev_id, a_uint32_t index, a_uint32_t mode)
{
	sw_error_t rv = SW_OK;
	a_uint32_t clock = UNIPHY_CLK_RATE_125M;

	if (mode == PORT_WRAPPER_MAX) {
		ssdk_uniphy_raw_clock_reset(index);
		return SW_OK;
	}

	switch(mode) {
		case PORT_WRAPPER_PSGMII:
		case PORT_WRAPPER_PSGMII_FIBER:
			rv = __adpt_hppe_uniphy_psgmii_mode_set(dev_id, index);
			break;

		case PORT_WRAPPER_QSGMII:
			rv = __adpt_hppe_uniphy_qsgmii_mode_set(dev_id, index);
			break;

		case PORT_WRAPPER_SGMII0_RGMII4:
		case PORT_WRAPPER_SGMII_CHANNEL0:
			rv = __adpt_hppe_uniphy_sgmii_mode_set(dev_id, index, 0);
			break;

		case PORT_WRAPPER_SGMII1_RGMII4:
		case PORT_WRAPPER_SGMII_CHANNEL1:
			rv = __adpt_hppe_uniphy_sgmii_mode_set(dev_id, index, 1);
			break;

		case PORT_WRAPPER_SGMII4_RGMII4:
		case PORT_WRAPPER_SGMII_CHANNEL4:
			rv = __adpt_hppe_uniphy_sgmii_mode_set(dev_id, index, 4);
			break;

		case PORT_WRAPPER_SGMII_PLUS:
			rv = __adpt_hppe_uniphy_sgmiiplus_mode_set(dev_id, index);
			clock = UNIPHY_CLK_RATE_312M;
			break;

		case PORT_WRAPPER_10GBASE_R:
			rv = __adpt_hppe_uniphy_10g_r_mode_set(dev_id, index);
			clock = UNIPHY_CLK_RATE_312M;
			break;

		case PORT_WRAPPER_USXGMII:
			rv = __adpt_hppe_uniphy_usxgmii_mode_set(dev_id, index);
			clock = UNIPHY_CLK_RATE_312M;
			break;

		default:
			rv = SW_FAIL;
	}
	if (SW_OK == rv) {
		ssdk_uniphy_raw_clock_set(index, UNIPHY_RX, clock);
		ssdk_uniphy_raw_clock_set(index, UNIPHY_TX, clock);
	}
	return rv;
}
sw_error_t adpt_hppe_uniphy_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	if(p_adpt_api == NULL)
		return SW_FAIL;

	p_adpt_api->adpt_uniphy_mode_set = adpt_hppe_uniphy_mode_set;

	return SW_OK;
}
#endif

/**
 * @}
 */
