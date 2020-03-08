/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
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
#include "hppe_global_reg.h"
#include "hppe_global.h"
#include "adpt.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "ssdk_init.h"
#include "ssdk_dts.h"
#include "adpt_cppe_portctrl.h"

sw_error_t
_adpt_cppe_port_mux_mac_set(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t port_type)
{
	sw_error_t rv = SW_OK;
	a_uint32_t mode0, mode1;
	union cppe_port_mux_ctrl_u cppe_port_mux_ctrl;

	memset(&cppe_port_mux_ctrl, 0, sizeof(cppe_port_mux_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);

	rv = cppe_port_mux_ctrl_get(dev_id, &cppe_port_mux_ctrl);

	mode0 = ssdk_dt_global_get_mac_mode(dev_id, SSDK_UNIPHY_INSTANCE0);
	mode1 = ssdk_dt_global_get_mac_mode(dev_id, SSDK_UNIPHY_INSTANCE1);

	if ((mode0 == PORT_WRAPPER_PSGMII) && (mode1 == PORT_WRAPPER_MAX)) {
		cppe_port_mux_ctrl.bf.port3_pcs_sel =
			CPPE_PORT3_PCS_SEL_PCS0_CHANNEL2;
		cppe_port_mux_ctrl.bf.port4_pcs_sel =
			CPPE_PORT4_PCS_SEL_PCS0_CHANNEL3;
		cppe_port_mux_ctrl.bf.port5_pcs_sel =
			CPPE_PORT5_PCS_SEL_PCS0_CHANNEL4;
		cppe_port_mux_ctrl.bf.port5_gmac_sel =
			CPPE_PORT5_GMAC_SEL_GMAC;
	} else if (((mode0 == PORT_WRAPPER_PSGMII) || (mode0 == PORT_WRAPPER_QSGMII))
			&& (mode1 == PORT_WRAPPER_USXGMII)) {
		cppe_port_mux_ctrl.bf.port3_pcs_sel =
			CPPE_PORT3_PCS_SEL_PCS0_CHANNEL2;
		cppe_port_mux_ctrl.bf.port4_pcs_sel =
			CPPE_PORT4_PCS_SEL_PCS0_CHANNEL3;
		cppe_port_mux_ctrl.bf.port5_pcs_sel =
			CPPE_PORT5_PCS_SEL_PCS1_CHANNEL0;
		cppe_port_mux_ctrl.bf.port5_gmac_sel =
			CPPE_PORT5_GMAC_SEL_XGMAC;
	} else {
		return SW_OK;
	}

	rv = cppe_port_mux_ctrl_set(dev_id, &cppe_port_mux_ctrl);

	return rv;
}

/**
 * @}
 */
