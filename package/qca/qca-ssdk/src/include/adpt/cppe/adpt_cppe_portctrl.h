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
#ifndef _ADPT_CPPE_PORTCTRLH_
#define _ADPT_CPPE_PORTCTRLH_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#define CPPE_PORT3_PCS_SEL_PCS0_CHANNEL2 0
#define CPPE_PORT3_PCS_SEL_PCS0_CHANNEL4 1
#define CPPE_PORT4_PCS_SEL_PCS0_CHANNEL3 0
#define CPPE_PORT4_PCS_SEL_PCS0_SGMIIPLUS 1
#define CPPE_PORT5_PCS_SEL_PCS0_CHANNEL4 0
#define CPPE_PORT5_PCS_SEL_PCS1_CHANNEL0 1
#define CPPE_PORT5_GMAC_SEL_GMAC 0
#define CPPE_PORT5_GMAC_SEL_XGMAC 1

sw_error_t
_adpt_cppe_port_mux_mac_set(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t port_type);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif
