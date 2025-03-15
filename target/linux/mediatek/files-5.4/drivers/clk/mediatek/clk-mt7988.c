/*
 * Copyright (c) 2022 MediaTek Inc.
 * Author: Xiufeng Li <Xiufeng.Li@mediatek.com>
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

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mfd/syscon.h>

#include "clk-mtk.h"
#include "clk-gate.h"
#include "clk-mux.h"

#include <dt-bindings/clock/mt7988-clk.h>

static DEFINE_SPINLOCK(mt7988_clk_lock);

static const struct mtk_fixed_factor top_divs[] __initconst = {
	FACTOR(CK_TOP_CB_CKSQ_40M, "cb_cksq_40m", "clkxtal", 1, 1),
	FACTOR(CK_TOP_CB_M_416M, "cb_m_416m", "mpll", 1, 1),
	FACTOR(CK_TOP_CB_M_D2, "cb_m_d2", "mpll", 1, 2),
	FACTOR(CK_TOP_M_D3_D2, "m_d3_d2", "mpll", 1, 2),
	FACTOR(CK_TOP_CB_M_D4, "cb_m_d4", "mpll", 1, 4),
	FACTOR(CK_TOP_CB_M_D8, "cb_m_d8", "mpll", 1, 8),
	FACTOR(CK_TOP_M_D8_D2, "m_d8_d2", "mpll", 1, 16),
	FACTOR(CK_TOP_CB_MM_720M, "cb_mm_720m", "mmpll", 1, 1),
	FACTOR(CK_TOP_CB_MM_D2, "cb_mm_d2", "mmpll", 1, 2),
	FACTOR(CK_TOP_CB_MM_D3_D5, "cb_mm_d3_d5", "mmpll", 1, 15),
	FACTOR(CK_TOP_CB_MM_D4, "cb_mm_d4", "mmpll", 1, 4),
	FACTOR(CK_TOP_MM_D6_D2, "mm_d6_d2", "mmpll", 1, 12),
	FACTOR(CK_TOP_CB_MM_D8, "cb_mm_d8", "mmpll", 1, 8),
	FACTOR(CK_TOP_CB_APLL2_196M, "cb_apll2_196m", "apll2", 1, 1),
	FACTOR(CK_TOP_CB_APLL2_D4, "cb_apll2_d4", "apll2", 1, 4),
	FACTOR(CK_TOP_CB_NET1_D4, "cb_net1_d4", "net1pll", 1, 4),
	FACTOR(CK_TOP_CB_NET1_D5, "cb_net1_d5", "net1pll", 1, 5),
	FACTOR(CK_TOP_NET1_D5_D2, "net1_d5_d2", "net1pll", 1, 10),
	FACTOR(CK_TOP_NET1_D5_D4, "net1_d5_d4", "net1pll", 1, 20),
	FACTOR(CK_TOP_CB_NET1_D8, "cb_net1_d8", "net1pll", 1, 8),
	FACTOR(CK_TOP_NET1_D8_D2, "net1_d8_d2", "net1pll", 1, 16),
	FACTOR(CK_TOP_NET1_D8_D4, "net1_d8_d4", "net1pll", 1, 32),
	FACTOR(CK_TOP_NET1_D8_D8, "net1_d8_d8", "net1pll", 1, 64),
	FACTOR(CK_TOP_NET1_D8_D16, "net1_d8_d16", "net1pll", 1, 128),
	FACTOR(CK_TOP_CB_NET2_800M, "cb_net2_800m", "net2pll", 1, 1),
	FACTOR(CK_TOP_CB_NET2_D2, "cb_net2_d2", "net2pll", 1, 2),
	FACTOR(CK_TOP_CB_NET2_D4, "cb_net2_d4", "net2pll", 1, 4),
	FACTOR(CK_TOP_NET2_D4_D4, "net2_d4_d4", "net2pll", 1, 16),
	FACTOR(CK_TOP_NET2_D4_D8, "net2_d4_d8", "net2pll", 1, 32),
	FACTOR(CK_TOP_CB_NET2_D6, "cb_net2_d6", "net2pll", 1, 6),
	FACTOR(CK_TOP_CB_NET2_D8, "cb_net2_d8", "net2pll", 1, 8),
	FACTOR(CK_TOP_CB_WEDMCU_208M, "cb_wedmcu_208m", "wedmcupll", 1, 1),
	FACTOR(CK_TOP_CB_SGM_325M, "cb_sgm_325m", "sgmpll", 1, 1),
	FACTOR(CK_TOP_CB_NETSYS_850M, "cb_netsys_850m", "netsyspll", 1, 1),
	FACTOR(CK_TOP_CB_MSDC_400M, "cb_msdc_400m", "msdcpll", 1, 1),
	FACTOR(CK_TOP_CKSQ_40M_D2, "cksq_40m_d2", "cb_cksq_40m", 1, 2),
	FACTOR(CK_TOP_CB_RTC_32K, "cb_rtc_32k", "cb_cksq_40m", 1, 1250),
	FACTOR(CK_TOP_CB_RTC_32P7K, "cb_rtc_32p7k", "cb_cksq_40m", 1, 1220),
	FACTOR(CK_TOP_INFRA_F32K, "csw_infra_f32k", "cb_rtc_32p7k", 1, 1),
	FACTOR(CK_TOP_CKSQ_SRC, "cksq_src", "clkxtal", 1, 1),
	FACTOR(CK_TOP_NETSYS_2X, "netsys_2x", "netsys_2x_sel", 1, 1),
	FACTOR(CK_TOP_NETSYS_GSW, "netsys_gsw", "netsys_gsw_sel", 1, 1),
	FACTOR(CK_TOP_NETSYS_WED_MCU, "netsys_wed_mcu", "netsys_mcu_sel", 1, 1),
	FACTOR(CK_TOP_EIP197, "eip197", "eip197_sel", 1, 1),
	FACTOR(CK_TOP_EMMC_250M, "emmc_250m", "emmc_250m_sel", 1, 1),
	FACTOR(CK_TOP_EMMC_400M, "emmc_400m", "emmc_400m_sel", 1, 1),
	FACTOR(CK_TOP_SPI, "spi", "spi_sel", 1, 1),
	FACTOR(CK_TOP_SPIM_MST, "spim_mst", "spim_mst_sel", 1, 1),
	FACTOR(CK_TOP_NFI1X, "nfi1x", "nfi1x_sel", 1, 1),
	FACTOR(CK_TOP_SPINFI_BCK, "spinfi_bck", "spinfi_sel", 1, 1),
	FACTOR(CK_TOP_I2C_BCK, "i2c_bck", "i2c_sel", 1, 1),
	FACTOR(CK_TOP_USB_SYS, "usb_sys", "usb_sys_sel", 1, 1),
	FACTOR(CK_TOP_USB_SYS_P1, "usb_sys_p1", "usb_sys_p1_sel", 1, 1),
	FACTOR(CK_TOP_USB_XHCI, "usb_xhci", "usb_xhci_sel", 1, 1),
	FACTOR(CK_TOP_USB_XHCI_P1, "usb_xhci_p1", "usb_xhci_p1_sel", 1, 1),
	FACTOR(CK_TOP_USB_FRMCNT, "usb_frmcnt", "usb_frmcnt_sel", 1, 1),
	FACTOR(CK_TOP_USB_FRMCNT_P1, "usb_frmcnt_p1", "usb_frmcnt_p1_sel", 1,
	       1),
	FACTOR(CK_TOP_AUD, "aud", "aud_sel", 1, 1),
	FACTOR(CK_TOP_A1SYS, "a1sys", "a1sys_sel", 1, 1),
	FACTOR(CK_TOP_AUD_L, "aud_l", "aud_l_sel", 1, 1),
	FACTOR(CK_TOP_A_TUNER, "a_tuner", "a_tuner_sel", 1, 1),
	FACTOR(CK_TOP_SYSAXI, "sysaxi", "sysaxi_sel", 1, 1),
	FACTOR(CK_TOP_INFRA_F26M, "csw_infra_f26m", "csw_infra_f26m_sel", 1, 1),
	FACTOR(CK_TOP_USB_REF, "usb_ref", "cksq_src", 1, 1),
	FACTOR(CK_TOP_USB_CK_P1, "usb_ck_p1", "cksq_src", 1, 1),
};

static const struct mtk_fixed_factor infra_divs[] __initconst = {
	FACTOR(CK_INFRA_CK_F26M, "infra_ck_f26m", "csw_infra_f26m_sel", 1, 1),
	FACTOR(CK_INFRA_PWM_O, "infra_pwm_o", "pwm_sel", 1, 1),
	FACTOR(CK_INFRA_PCIE_OCC_P0, "infra_pcie_ck_occ_p0", "pextp_tl_ck_sel",
	       1, 1),
	FACTOR(CK_INFRA_PCIE_OCC_P1, "infra_pcie_ck_occ_p1",
	       "pextp_tl_ck_p1_sel", 1, 1),
	FACTOR(CK_INFRA_PCIE_OCC_P2, "infra_pcie_ck_occ_p2",
	       "pextp_tl_ck_p2_sel", 1, 1),
	FACTOR(CK_INFRA_PCIE_OCC_P3, "infra_pcie_ck_occ_p3",
	       "pextp_tl_ck_p3_sel", 1, 1),
	FACTOR(CK_INFRA_133M_HCK, "infra_133m_hck", "sysaxi", 1, 1),
	FACTOR(CK_INFRA_133M_PHCK, "infra_133m_phck", "infra_133m_hck", 1, 1),
	FACTOR(CK_INFRA_66M_PHCK, "infra_66m_phck", "infra_133m_hck", 1, 1),
	FACTOR(CK_INFRA_FAUD_L_O, "infra_faud_l_o", "aud_l", 1, 1),
	FACTOR(CK_INFRA_FAUD_AUD_O, "infra_faud_aud_o", "a1sys", 1, 1),
	FACTOR(CK_INFRA_FAUD_EG2_O, "infra_faud_eg2_o", "a_tuner", 1, 1),
	FACTOR(CK_INFRA_I2C_O, "infra_i2c_o", "i2c_bck", 1, 1),
	FACTOR(CK_INFRA_UART_O0, "infra_uart_o0", "uart_sel", 1, 1),
	FACTOR(CK_INFRA_UART_O1, "infra_uart_o1", "uart_sel", 1, 1),
	FACTOR(CK_INFRA_UART_O2, "infra_uart_o2", "uart_sel", 1, 1),
	FACTOR(CK_INFRA_NFI_O, "infra_nfi_o", "nfi1x", 1, 1),
	FACTOR(CK_INFRA_SPINFI_O, "infra_spinfi_o", "spinfi_bck", 1, 1),
	FACTOR(CK_INFRA_SPI0_O, "infra_spi0_o", "spi", 1, 1),
	FACTOR(CK_INFRA_SPI1_O, "infra_spi1_o", "spim_mst", 1, 1),
	FACTOR(CK_INFRA_LB_MUX_FRTC, "infra_lb_mux_frtc", "infra_frtc", 1, 1),
	FACTOR(CK_INFRA_FRTC, "infra_frtc", "cb_rtc_32k", 1, 1),
	FACTOR(CK_INFRA_FMSDC400_O, "infra_fmsdc400_o", "emmc_400m", 1, 1),
	FACTOR(CK_INFRA_FMSDC2_HCK_OCC, "infra_fmsdc2_hck_occ", "emmc_250m", 1,
	       1),
	FACTOR(CK_INFRA_PERI_133M, "infra_peri_133m", "sysaxi", 1, 1),
	FACTOR(CK_INFRA_USB_O, "infra_usb_o", "usb_ref", 1, 1),
	FACTOR(CK_INFRA_USB_O_P1, "infra_usb_o_p1", "usb_ck_p1", 1, 1),
	FACTOR(CK_INFRA_USB_FRMCNT_O, "infra_usb_frmcnt_o", "usb_frmcnt", 1, 1),
	FACTOR(CK_INFRA_USB_FRMCNT_O_P1, "infra_usb_frmcnt_o_p1",
	       "usb_frmcnt_p1", 1, 1),
	FACTOR(CK_INFRA_USB_XHCI_O, "infra_usb_xhci_o", "usb_xhci", 1, 1),
	FACTOR(CK_INFRA_USB_XHCI_O_P1, "infra_usb_xhci_o_p1", "usb_xhci_p1", 1,
	       1),
	FACTOR(CK_INFRA_USB_PIPE_O, "infra_usb_pipe_o", "clkxtal", 1, 1),
	FACTOR(CK_INFRA_USB_PIPE_O_P1, "infra_usb_pipe_o_p1", "clkxtal", 1, 1),
	FACTOR(CK_INFRA_USB_UTMI_O, "infra_usb_utmi_o", "clkxtal", 1, 1),
	FACTOR(CK_INFRA_USB_UTMI_O_P1, "infra_usb_utmi_o_p1", "clkxtal", 1, 1),
	FACTOR(CK_INFRA_PCIE_PIPE_OCC_P0, "infra_pcie_pipe_ck_occ_p0",
	       "clkxtal", 1, 1),
	FACTOR(CK_INFRA_PCIE_PIPE_OCC_P1, "infra_pcie_pipe_ck_occ_p1",
	       "clkxtal", 1, 1),
	FACTOR(CK_INFRA_PCIE_PIPE_OCC_P2, "infra_pcie_pipe_ck_occ_p2",
	       "clkxtal", 1, 1),
	FACTOR(CK_INFRA_PCIE_PIPE_OCC_P3, "infra_pcie_pipe_ck_occ_p3",
	       "clkxtal", 1, 1),
	FACTOR(CK_INFRA_F26M_O0, "infra_f26m_o0", "csw_infra_f26m", 1, 1),
	FACTOR(CK_INFRA_F26M_O1, "infra_f26m_o1", "csw_infra_f26m", 1, 1),
	FACTOR(CK_INFRA_133M_MCK, "infra_133m_mck", "sysaxi", 1, 1),
	FACTOR(CK_INFRA_66M_MCK, "infra_66m_mck", "sysaxi", 1, 1),
	FACTOR(CK_INFRA_PERI_66M_O, "infra_peri_66m_o", "sysaxi", 1, 1),
	FACTOR(CK_INFRA_USB_SYS_O, "infra_usb_sys_o", "usb_sys", 1, 1),
	FACTOR(CK_INFRA_USB_SYS_O_P1, "infra_usb_sys_o_p1", "usb_sys_p1", 1, 1),
};

static const char *const mcu_bus_div_parents[] = { "cb_cksq_40m", "ccipll2_b",
						   "cb_net1_d4" };

static const char *const mcu_arm_div_parents[] = { "cb_cksq_40m", "arm_b",
						   "cb_net1_d4" };

static struct mtk_composite mcu_muxes[] = {
	/* bus_pll_divider_cfg */
	MUX_GATE_FLAGS(CK_MCU_BUS_DIV_SEL, "mcu_bus_div_sel",
		       mcu_bus_div_parents, 0x7C0, 9, 2, -1, CLK_IS_CRITICAL),
	/* mp2_pll_divider_cfg */
	MUX_GATE_FLAGS(CK_MCU_ARM_DIV_SEL, "mcu_arm_div_sel",
		       mcu_arm_div_parents, 0x7A8, 9, 2, -1, CLK_IS_CRITICAL),
};

static const char *const netsys_parents[] = { "cb_cksq_40m", "cb_net2_d2",
					      "cb_mm_d2" };

static const char *const netsys_500m_parents[] = { "cb_cksq_40m", "cb_net1_d5",
						   "net1_d5_d2" };

static const char *const netsys_2x_parents[] = { "cb_cksq_40m", "cb_net2_800m",
						 "cb_mm_720m" };

static const char *const netsys_gsw_parents[] = { "cb_cksq_40m", "cb_net1_d4",
						  "cb_net1_d5" };

static const char *const eth_gmii_parents[] = { "cb_cksq_40m", "net1_d5_d4" };

static const char *const netsys_mcu_parents[] = { "cb_cksq_40m", "cb_net2_800m",
						  "cb_mm_720m",	 "cb_net1_d4",
						  "cb_net1_d5",	 "cb_m_416m" };

static const char *const eip197_parents[] = { "cb_cksq_40m",  "cb_netsys_850m",
					      "cb_net2_800m", "cb_mm_720m",
					      "cb_net1_d4",   "cb_net1_d5" };

static const char *const axi_infra_parents[] = { "cb_cksq_40m", "net1_d8_d2" };

static const char *const uart_parents[] = { "cb_cksq_40m", "cb_m_d8",
					    "m_d8_d2" };

static const char *const emmc_250m_parents[] = { "cb_cksq_40m", "net1_d5_d2",
						 "cb_mm_d4" };

static const char *const emmc_400m_parents[] = { "cb_cksq_40m", "cb_msdc_400m",
						 "cb_mm_d2",	"cb_m_d2",
						 "cb_mm_d4",	"net1_d8_d2" };

static const char *const spi_parents[] = { "cb_cksq_40m", "cb_m_d2",
					   "cb_mm_d4",	  "net1_d8_d2",
					   "cb_net2_d6",  "net1_d5_d4",
					   "cb_m_d4",	  "net1_d8_d4" };

static const char *const nfi1x_parents[] = { "cb_cksq_40m", "cb_mm_d4",
					     "net1_d8_d2",  "cb_net2_d6",
					     "cb_m_d4",	    "cb_mm_d8",
					     "net1_d8_d4",  "cb_m_d8" };

static const char *const spinfi_parents[] = { "cksq_40m_d2", "cb_cksq_40m",
					      "net1_d5_d4",  "cb_m_d4",
					      "cb_mm_d8",    "net1_d8_d4",
					      "mm_d6_d2",    "cb_m_d8" };

static const char *const pwm_parents[] = { "cb_cksq_40m", "net1_d8_d2",
					   "net1_d5_d4",  "cb_m_d4",
					   "m_d8_d2",	  "cb_rtc_32k" };

static const char *const i2c_parents[] = { "cb_cksq_40m", "net1_d5_d4",
					   "cb_m_d4", "net1_d8_d4" };

static const char *const pcie_mbist_250m_parents[] = { "cb_cksq_40m",
						       "net1_d5_d2" };

static const char *const pextp_tl_ck_parents[] = { "cb_cksq_40m", "cb_net2_d6",
						   "cb_mm_d8", "m_d8_d2",
						   "cb_rtc_32k" };

static const char *const usb_frmcnt_parents[] = { "cb_cksq_40m",
						  "cb_mm_d3_d5" };

static const char *const aud_parents[] = { "cb_cksq_40m", "cb_apll2_196m" };

static const char *const a1sys_parents[] = { "cb_cksq_40m", "cb_apll2_d4" };

static const char *const aud_l_parents[] = { "cb_cksq_40m", "cb_apll2_196m",
					     "m_d8_d2" };

static const char *const sspxtp_parents[] = { "cksq_40m_d2", "m_d8_d2" };

static const char *const usxgmii_sbus_0_parents[] = { "cb_cksq_40m",
						      "net1_d8_d4" };

static const char *const sgm_0_parents[] = { "cb_cksq_40m", "cb_sgm_325m" };

static const char *const sysapb_parents[] = { "cb_cksq_40m", "m_d3_d2" };

static const char *const eth_refck_50m_parents[] = { "cb_cksq_40m",
						     "net2_d4_d4" };

static const char *const eth_sys_200m_parents[] = { "cb_cksq_40m",
						    "cb_net2_d4" };

static const char *const eth_xgmii_parents[] = { "cksq_40m_d2", "net1_d8_d8",
						 "net1_d8_d16" };

static const char *const bus_tops_parents[] = { "cb_cksq_40m", "cb_net1_d5",
						"cb_net2_d2" };

static const char *const npu_tops_parents[] = { "cb_cksq_40m", "cb_net2_800m" };

static const char *const dramc_md32_parents[] = { "cb_cksq_40m", "cb_m_d2",
						  "cb_wedmcu_208m" };

static const char *const da_xtp_glb_p0_parents[] = { "cb_cksq_40m",
						     "cb_net2_d8" };

static const char *const mcusys_backup_625m_parents[] = { "cb_cksq_40m",
							  "cb_net1_d4" };

static const char *const macsec_parents[] = { "cb_cksq_40m", "cb_sgm_325m",
					      "cb_net1_d8" };

static const char *const netsys_tops_400m_parents[] = { "cb_cksq_40m",
							"cb_net2_d2" };

static const char *const eth_mii_parents[] = { "cksq_40m_d2", "net2_d4_d8" };

static struct mtk_mux top_muxes[] = {
	/* CLK_CFG_0 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_NETSYS_SEL, "netsys_sel", netsys_parents,
			     0x000, 0x004, 0x008, 0, 2, 7, 0x1C0, 0),
	MUX_GATE_CLR_SET_UPD(CK_TOP_NETSYS_500M_SEL, "netsys_500m_sel",
			     netsys_500m_parents, 0x000, 0x004, 0x008, 8, 2, 15,
			     0x1C0, 1),
	MUX_GATE_CLR_SET_UPD(CK_TOP_NETSYS_2X_SEL, "netsys_2x_sel",
			     netsys_2x_parents, 0x000, 0x004, 0x008, 16, 2, 23,
			     0x1C0, 2),
	MUX_GATE_CLR_SET_UPD(CK_TOP_NETSYS_GSW_SEL, "netsys_gsw_sel",
			     netsys_gsw_parents, 0x000, 0x004, 0x008, 24, 2, 31,
			     0x1C0, 3),
	/* CLK_CFG_1 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_ETH_GMII_SEL, "eth_gmii_sel",
			     eth_gmii_parents, 0x010, 0x014, 0x018, 0, 1, 7,
			     0x1C0, 4),
	MUX_GATE_CLR_SET_UPD(CK_TOP_NETSYS_MCU_SEL, "netsys_mcu_sel",
			     netsys_mcu_parents, 0x010, 0x014, 0x018, 8, 3, 15,
			     0x1C0, 5),
	MUX_GATE_CLR_SET_UPD(CK_TOP_NETSYS_PAO_2X_SEL, "netsys_pao_2x_sel",
			     netsys_mcu_parents, 0x010, 0x014, 0x018, 16, 3, 23,
			     0x1C0, 6),
	MUX_GATE_CLR_SET_UPD(CK_TOP_EIP197_SEL, "eip197_sel", eip197_parents,
			     0x010, 0x014, 0x018, 24, 3, 31, 0x1C0, 7),
	/* CLK_CFG_2 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_AXI_INFRA_SEL, "axi_infra_sel",
			     axi_infra_parents, 0x020, 0x024, 0x028, 0, 1, 7,
			     0x1C0, 8),
	MUX_GATE_CLR_SET_UPD(CK_TOP_UART_SEL, "uart_sel", uart_parents, 0x020,
			     0x024, 0x028, 8, 2, 15, 0x1C0, 9),
	MUX_GATE_CLR_SET_UPD(CK_TOP_EMMC_250M_SEL, "emmc_250m_sel",
			     emmc_250m_parents, 0x020, 0x024, 0x028, 16, 2, 23,
			     0x1C0, 10),
	MUX_GATE_CLR_SET_UPD(CK_TOP_EMMC_400M_SEL, "emmc_400m_sel",
			     emmc_400m_parents, 0x020, 0x024, 0x028, 24, 3, 31,
			     0x1C0, 11),
	/* CLK_CFG_3 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_SPI_SEL, "spi_sel", spi_parents, 0x030,
			     0x034, 0x038, 0, 3, 7, 0x1C0, 12),
	MUX_GATE_CLR_SET_UPD(CK_TOP_SPIM_MST_SEL, "spim_mst_sel", spi_parents,
			     0x030, 0x034, 0x038, 8, 3, 15, 0x1C0, 13),
	MUX_GATE_CLR_SET_UPD(CK_TOP_NFI1X_SEL, "nfi1x_sel", nfi1x_parents,
			     0x030, 0x034, 0x038, 16, 3, 23, 0x1C0, 14),
	MUX_GATE_CLR_SET_UPD(CK_TOP_SPINFI_SEL, "spinfi_sel", spinfi_parents,
			     0x030, 0x034, 0x038, 24, 3, 31, 0x1C0, 15),
	/* CLK_CFG_4 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_PWM_SEL, "pwm_sel", pwm_parents, 0x040,
			     0x044, 0x048, 0, 3, 7, 0x1C0, 16),
	MUX_GATE_CLR_SET_UPD(CK_TOP_I2C_SEL, "i2c_sel", i2c_parents, 0x040,
			     0x044, 0x048, 8, 2, 15, 0x1C0, 17),
	MUX_GATE_CLR_SET_UPD(CK_TOP_PCIE_MBIST_250M_SEL, "pcie_mbist_250m_sel",
			     pcie_mbist_250m_parents, 0x040, 0x044, 0x048, 16,
			     1, 23, 0x1C0, 18),
	MUX_GATE_CLR_SET_UPD(CK_TOP_PEXTP_TL_SEL, "pextp_tl_ck_sel",
			     pextp_tl_ck_parents, 0x040, 0x044, 0x048, 24, 3,
			     31, 0x1C0, 19),
	/* CLK_CFG_5 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_PEXTP_TL_P1_SEL, "pextp_tl_ck_p1_sel",
			     pextp_tl_ck_parents, 0x050, 0x054, 0x058, 0, 3, 7,
			     0x1C0, 20),
	MUX_GATE_CLR_SET_UPD(CK_TOP_PEXTP_TL_P2_SEL, "pextp_tl_ck_p2_sel",
			     pextp_tl_ck_parents, 0x050, 0x054, 0x058, 8, 3, 15,
			     0x1C0, 21),
	MUX_GATE_CLR_SET_UPD(CK_TOP_PEXTP_TL_P3_SEL, "pextp_tl_ck_p3_sel",
			     pextp_tl_ck_parents, 0x050, 0x054, 0x058, 16, 3,
			     23, 0x1C0, 22),
	MUX_GATE_CLR_SET_UPD(CK_TOP_USB_SYS_SEL, "usb_sys_sel",
			     eth_gmii_parents, 0x050, 0x054, 0x058, 24, 1, 31,
			     0x1C0, 23),
	/* CLK_CFG_6 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_USB_SYS_P1_SEL, "usb_sys_p1_sel",
			     eth_gmii_parents, 0x060, 0x064, 0x068, 0, 1, 7,
			     0x1C0, 24),
	MUX_GATE_CLR_SET_UPD(CK_TOP_USB_XHCI_SEL, "usb_xhci_sel",
			     eth_gmii_parents, 0x060, 0x064, 0x068, 8, 1, 15,
			     0x1C0, 25),
	MUX_GATE_CLR_SET_UPD(CK_TOP_USB_XHCI_P1_SEL, "usb_xhci_p1_sel",
			     eth_gmii_parents, 0x060, 0x064, 0x068, 16, 1, 23,
			     0x1C0, 26),
	MUX_GATE_CLR_SET_UPD(CK_TOP_USB_FRMCNT_SEL, "usb_frmcnt_sel",
			     usb_frmcnt_parents, 0x060, 0x064, 0x068, 24, 1, 31,
			     0x1C0, 27),
	/* CLK_CFG_7 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_USB_FRMCNT_P1_SEL, "usb_frmcnt_p1_sel",
			     usb_frmcnt_parents, 0x070, 0x074, 0x078, 0, 1, 7,
			     0x1C0, 28),
	MUX_GATE_CLR_SET_UPD(CK_TOP_AUD_SEL, "aud_sel", aud_parents, 0x070,
			     0x074, 0x078, 8, 1, 15, 0x1C0, 29),
	MUX_GATE_CLR_SET_UPD(CK_TOP_A1SYS_SEL, "a1sys_sel", a1sys_parents,
			     0x070, 0x074, 0x078, 16, 1, 23, 0x1C0, 30),
	MUX_GATE_CLR_SET_UPD(CK_TOP_AUD_L_SEL, "aud_l_sel", aud_l_parents,
			     0x070, 0x074, 0x078, 24, 2, 31, 0x1C4, 0),
	/* CLK_CFG_8 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_A_TUNER_SEL, "a_tuner_sel", a1sys_parents,
			     0x080, 0x084, 0x088, 0, 1, 7, 0x1C4, 1),
	MUX_GATE_CLR_SET_UPD(CK_TOP_SSPXTP_SEL, "sspxtp_sel", sspxtp_parents,
			     0x080, 0x084, 0x088, 8, 1, 15, 0x1C4, 2),
	MUX_GATE_CLR_SET_UPD(CK_TOP_USB_PHY_SEL, "usb_phy_sel", sspxtp_parents,
			     0x080, 0x084, 0x088, 16, 1, 23, 0x1C4, 3),
	MUX_GATE_CLR_SET_UPD(CK_TOP_USXGMII_SBUS_0_SEL, "usxgmii_sbus_0_sel",
			     usxgmii_sbus_0_parents, 0x080, 0x084, 0x088, 24, 1,
			     31, 0x1C4, 4),
	/* CLK_CFG_9 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_USXGMII_SBUS_1_SEL, "usxgmii_sbus_1_sel",
			     usxgmii_sbus_0_parents, 0x090, 0x094, 0x098, 0, 1,
			     7, 0x1C4, 5),
	MUX_GATE_CLR_SET_UPD(CK_TOP_SGM_0_SEL, "sgm_0_sel", sgm_0_parents,
			     0x090, 0x094, 0x098, 8, 1, 15, 0x1C4, 6),
	MUX_GATE_CLR_SET_UPD(CK_TOP_SGM_SBUS_0_SEL, "sgm_sbus_0_sel",
			     usxgmii_sbus_0_parents, 0x090, 0x094, 0x098, 16, 1,
			     23, 0x1C4, 7),
	MUX_GATE_CLR_SET_UPD(CK_TOP_SGM_1_SEL, "sgm_1_sel", sgm_0_parents,
			     0x090, 0x094, 0x098, 24, 1, 31, 0x1C4, 8),
	/* CLK_CFG_10 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_SGM_SBUS_1_SEL, "sgm_sbus_1_sel",
			     usxgmii_sbus_0_parents, 0x0A0, 0x0A4, 0x0A8, 0, 1,
			     7, 0x1C4, 9),
	MUX_GATE_CLR_SET_UPD(CK_TOP_XFI_PHY_0_XTAL_SEL, "xfi_phy_0_xtal_sel",
			     sspxtp_parents, 0x0A0, 0x0A4, 0x0A8, 8, 1, 15,
			     0x1C4, 10),
	MUX_GATE_CLR_SET_UPD(CK_TOP_XFI_PHY_1_XTAL_SEL, "xfi_phy_1_xtal_sel",
			     sspxtp_parents, 0x0A0, 0x0A4, 0x0A8, 16, 1, 23,
			     0x1C4, 11),
	MUX_GATE_CLR_SET_UPD(CK_TOP_SYSAXI_SEL, "sysaxi_sel", axi_infra_parents,
			     0x0A0, 0x0A4, 0x0A8, 24, 1, 31, 0x1C4, 12),
	/* CLK_CFG_11 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_SYSAPB_SEL, "sysapb_sel", sysapb_parents,
			     0x0B0, 0x0B4, 0x0B8, 0, 1, 7, 0x1C4, 13),
	MUX_GATE_CLR_SET_UPD(CK_TOP_ETH_REFCK_50M_SEL, "eth_refck_50m_sel",
			     eth_refck_50m_parents, 0x0B0, 0x0B4, 0x0B8, 8, 1,
			     15, 0x1C4, 14),
	MUX_GATE_CLR_SET_UPD(CK_TOP_ETH_SYS_200M_SEL, "eth_sys_200m_sel",
			     eth_sys_200m_parents, 0x0B0, 0x0B4, 0x0B8, 16, 1,
			     23, 0x1C4, 15),
	MUX_GATE_CLR_SET_UPD(CK_TOP_ETH_SYS_SEL, "eth_sys_sel",
			     pcie_mbist_250m_parents, 0x0B0, 0x0B4, 0x0B8, 24,
			     1, 31, 0x1C4, 16),
	/* CLK_CFG_12 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_ETH_XGMII_SEL, "eth_xgmii_sel",
			     eth_xgmii_parents, 0x0C0, 0x0C4, 0x0C8, 0, 2, 7,
			     0x1C4, 17),
	MUX_GATE_CLR_SET_UPD(CK_TOP_BUS_TOPS_SEL, "bus_tops_sel",
			     bus_tops_parents, 0x0C0, 0x0C4, 0x0C8, 8, 2, 15,
			     0x1C4, 18),
	MUX_GATE_CLR_SET_UPD(CK_TOP_NPU_TOPS_SEL, "npu_tops_sel",
			     npu_tops_parents, 0x0C0, 0x0C4, 0x0C8, 16, 1, 23,
			     0x1C4, 19),
	MUX_GATE_CLR_SET_UPD(CK_TOP_DRAMC_SEL, "dramc_sel", sspxtp_parents,
			     0x0C0, 0x0C4, 0x0C8, 24, 1, 31, 0x1C4, 20),
	/* CLK_CFG_13 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_DRAMC_MD32_SEL, "dramc_md32_sel",
			     dramc_md32_parents, 0x0D0, 0x0D4, 0x0D8, 0, 2, 7,
			     0x1C4, 21),
	MUX_GATE_CLR_SET_UPD(CK_TOP_INFRA_F26M_SEL, "csw_infra_f26m_sel",
			     sspxtp_parents, 0x0D0, 0x0D4, 0x0D8, 8, 1, 15,
			     0x1C4, 22),
	MUX_GATE_CLR_SET_UPD(CK_TOP_PEXTP_P0_SEL, "pextp_p0_sel",
			     sspxtp_parents, 0x0D0, 0x0D4, 0x0D8, 16, 1, 23,
			     0x1C4, 23),
	MUX_GATE_CLR_SET_UPD(CK_TOP_PEXTP_P1_SEL, "pextp_p1_sel",
			     sspxtp_parents, 0x0D0, 0x0D4, 0x0D8, 24, 1, 31,
			     0x1C4, 24),
	/* CLK_CFG_14 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_PEXTP_P2_SEL, "pextp_p2_sel",
			     sspxtp_parents, 0x0E0, 0x0E4, 0x0E8, 0, 1, 7,
			     0x1C4, 25),
	MUX_GATE_CLR_SET_UPD(CK_TOP_PEXTP_P3_SEL, "pextp_p3_sel",
			     sspxtp_parents, 0x0E0, 0x0E4, 0x0E8, 8, 1, 15,
			     0x1C4, 26),
	MUX_GATE_CLR_SET_UPD(CK_TOP_DA_XTP_GLB_P0_SEL, "da_xtp_glb_p0_sel",
			     da_xtp_glb_p0_parents, 0x0E0, 0x0E4, 0x0E8, 16, 1,
			     23, 0x1C4, 27),
	MUX_GATE_CLR_SET_UPD(CK_TOP_DA_XTP_GLB_P1_SEL, "da_xtp_glb_p1_sel",
			     da_xtp_glb_p0_parents, 0x0E0, 0x0E4, 0x0E8, 24, 1,
			     31, 0x1C4, 28),
	/* CLK_CFG_15 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_DA_XTP_GLB_P2_SEL, "da_xtp_glb_p2_sel",
			     da_xtp_glb_p0_parents, 0x0F0, 0x0F4, 0x0F8, 0, 1,
			     7, 0x1C4, 29),
	MUX_GATE_CLR_SET_UPD(CK_TOP_DA_XTP_GLB_P3_SEL, "da_xtp_glb_p3_sel",
			     da_xtp_glb_p0_parents, 0x0F0, 0x0F4, 0x0F8, 8, 1,
			     15, 0x1C4, 30),
	MUX_GATE_CLR_SET_UPD(CK_TOP_CKM_SEL, "ckm_sel", sspxtp_parents, 0x0F0,
			     0x0F4, 0x0F8, 16, 1, 23, 0x1C8, 0),
	MUX_GATE_CLR_SET_UPD(CK_TOP_DA_SELM_XTAL_SEL, "da_selm_xtal_sel",
			     sspxtp_parents, 0x0F0, 0x0F4, 0x0F8, 24, 1, 31,
			     0x1C8, 1),
	/* CLK_CFG_16 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_PEXTP_SEL, "pextp_sel", sspxtp_parents,
			     0x0100, 0x104, 0x108, 0, 1, 7, 0x1C8, 2),
	MUX_GATE_CLR_SET_UPD(CK_TOP_TOPS_P2_26M_SEL, "tops_p2_26m_sel",
			     sspxtp_parents, 0x0100, 0x104, 0x108, 8, 1, 15,
			     0x1C8, 3),
	MUX_GATE_CLR_SET_UPD(CK_TOP_MCUSYS_BACKUP_625M_SEL,
			     "mcusys_backup_625m_sel",
			     mcusys_backup_625m_parents, 0x0100, 0x104, 0x108,
			     16, 1, 23, 0x1C8, 4),
	MUX_GATE_CLR_SET_UPD(CK_TOP_NETSYS_SYNC_250M_SEL,
			     "netsys_sync_250m_sel", pcie_mbist_250m_parents,
			     0x0100, 0x104, 0x108, 24, 1, 31, 0x1C8, 5),
	/* CLK_CFG_17 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_MACSEC_SEL, "macsec_sel", macsec_parents,
			     0x0110, 0x114, 0x118, 0, 2, 7, 0x1C8, 6),
	MUX_GATE_CLR_SET_UPD(CK_TOP_NETSYS_TOPS_400M_SEL,
			     "netsys_tops_400m_sel", netsys_tops_400m_parents,
			     0x0110, 0x114, 0x118, 8, 1, 15, 0x1C8, 7),
	MUX_GATE_CLR_SET_UPD(CK_TOP_NETSYS_PPEFB_250M_SEL,
			     "netsys_ppefb_250m_sel", pcie_mbist_250m_parents,
			     0x0110, 0x114, 0x118, 16, 1, 23, 0x1C8, 8),
	MUX_GATE_CLR_SET_UPD(CK_TOP_NETSYS_WARP_SEL, "netsys_warp_sel",
			     netsys_parents, 0x0110, 0x114, 0x118, 24, 2, 31,
			     0x1C8, 9),
	/* CLK_CFG_18 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_ETH_MII_SEL, "eth_mii_sel", eth_mii_parents,
			     0x0120, 0x124, 0x128, 0, 1, 7, 0x1C8, 10),
	MUX_GATE_CLR_SET_UPD(CK_TOP_CK_NPU_SEL_CM_TOPS_SEL,
			     "ck_npu_sel_cm_tops_sel", netsys_2x_parents,
			     0x0120, 0x124, 0x128, 8, 2, 15, 0x1C8, 11),
};

static const char *const infra_mux_uart0_parents[] __initconst = {
	"infra_ck_f26m", "infra_uart_o0"
};

static const char *const infra_mux_uart1_parents[] __initconst = {
	"infra_ck_f26m", "infra_uart_o1"
};

static const char *const infra_mux_uart2_parents[] __initconst = {
	"infra_ck_f26m", "infra_uart_o2"
};

static const char *const infra_mux_spi0_parents[] __initconst = {
	"infra_i2c_o", "infra_spi0_o"
};

static const char *const infra_mux_spi1_parents[] __initconst = {
	"infra_i2c_o", "infra_spi1_o"
};

static const char *const infra_pwm_bck_parents[] __initconst = {
	"csw_infra_f32k", "infra_ck_f26m", "infra_66m_mck", "infra_pwm_o"
};

static const char *const infra_pcie_gfmux_tl_ck_o_p0_parents[] __initconst = {
	"csw_infra_f32k", "infra_ck_f26m", "infra_ck_f26m",
	"infra_pcie_ck_occ_p0"
};

static const char *const infra_pcie_gfmux_tl_ck_o_p1_parents[] __initconst = {
	"csw_infra_f32k", "infra_ck_f26m", "infra_ck_f26m",
	"infra_pcie_ck_occ_p1"
};

static const char *const infra_pcie_gfmux_tl_ck_o_p2_parents[] __initconst = {
	"csw_infra_f32k", "infra_ck_f26m", "infra_ck_f26m",
	"infra_pcie_ck_occ_p2"
};

static const char *const infra_pcie_gfmux_tl_ck_o_p3_parents[] __initconst = {
	"csw_infra_f32k", "infra_ck_f26m", "infra_ck_f26m",
	"infra_pcie_ck_occ_p3"
};

static const struct mtk_mux infra_muxes[] = {
	/* MODULE_CLK_SEL_0 */
	MUX_GATE_CLR_SET_UPD(CK_INFRA_MUX_UART0_SEL, "infra_mux_uart0_sel",
			     infra_mux_uart0_parents, 0x0018, 0x0010, 0x0014, 0,
			     1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_MUX_UART1_SEL, "infra_mux_uart1_sel",
			     infra_mux_uart1_parents, 0x0018, 0x0010, 0x0014, 1,
			     1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_MUX_UART2_SEL, "infra_mux_uart2_sel",
			     infra_mux_uart2_parents, 0x0018, 0x0010, 0x0014, 2,
			     1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_MUX_SPI0_SEL, "infra_mux_spi0_sel",
			     infra_mux_spi0_parents, 0x0018, 0x0010, 0x0014, 4,
			     1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_MUX_SPI1_SEL, "infra_mux_spi1_sel",
			     infra_mux_spi1_parents, 0x0018, 0x0010, 0x0014, 5,
			     1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_MUX_SPI2_SEL, "infra_mux_spi2_sel",
			     infra_mux_spi0_parents, 0x0018, 0x0010, 0x0014, 6,
			     1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PWM_SEL, "infra_pwm_sel",
			     infra_pwm_bck_parents, 0x0018, 0x0010, 0x0014, 14,
			     2, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PWM_CK1_SEL, "infra_pwm_ck1_sel",
			     infra_pwm_bck_parents, 0x0018, 0x0010, 0x0014, 16,
			     2, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PWM_CK2_SEL, "infra_pwm_ck2_sel",
			     infra_pwm_bck_parents, 0x0018, 0x0010, 0x0014, 18,
			     2, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PWM_CK3_SEL, "infra_pwm_ck3_sel",
			     infra_pwm_bck_parents, 0x0018, 0x0010, 0x0014, 20,
			     2, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PWM_CK4_SEL, "infra_pwm_ck4_sel",
			     infra_pwm_bck_parents, 0x0018, 0x0010, 0x0014, 22,
			     2, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PWM_CK5_SEL, "infra_pwm_ck5_sel",
			     infra_pwm_bck_parents, 0x0018, 0x0010, 0x0014, 24,
			     2, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PWM_CK6_SEL, "infra_pwm_ck6_sel",
			     infra_pwm_bck_parents, 0x0018, 0x0010, 0x0014, 26,
			     2, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PWM_CK7_SEL, "infra_pwm_ck7_sel",
			     infra_pwm_bck_parents, 0x0018, 0x0010, 0x0014, 28,
			     2, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PWM_CK8_SEL, "infra_pwm_ck8_sel",
			     infra_pwm_bck_parents, 0x0018, 0x0010, 0x0014, 30,
			     2, -1, -1, -1),
	/* MODULE_CLK_SEL_1 */
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PCIE_GFMUX_TL_O_P0_SEL,
			     "infra_pcie_gfmux_tl_o_p0_sel",
			     infra_pcie_gfmux_tl_ck_o_p0_parents, 0x0028,
			     0x0020, 0x0024, 0, 2, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PCIE_GFMUX_TL_O_P1_SEL,
			     "infra_pcie_gfmux_tl_o_p1_sel",
			     infra_pcie_gfmux_tl_ck_o_p1_parents, 0x0028,
			     0x0020, 0x0024, 2, 2, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PCIE_GFMUX_TL_O_P2_SEL,
			     "infra_pcie_gfmux_tl_o_p2_sel",
			     infra_pcie_gfmux_tl_ck_o_p2_parents, 0x0028,
			     0x0020, 0x0024, 4, 2, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PCIE_GFMUX_TL_O_P3_SEL,
			     "infra_pcie_gfmux_tl_o_p3_sel",
			     infra_pcie_gfmux_tl_ck_o_p3_parents, 0x0028,
			     0x0020, 0x0024, 6, 2, -1, -1, -1),
};

static struct mtk_composite top_aud_divs[] = {
	DIV_GATE(CK_TOP_AUD_I2S_M, "aud_i2s_m", "aud",
		0x0420, 0, 0x0420, 8, 8),
};

static const struct mtk_gate_regs infra0_cg_regs = {
	.set_ofs = 0x10,
	.clr_ofs = 0x14,
	.sta_ofs = 0x18,
};

static const struct mtk_gate_regs infra1_cg_regs = {
	.set_ofs = 0x40,
	.clr_ofs = 0x44,
	.sta_ofs = 0x48,
};

static const struct mtk_gate_regs infra2_cg_regs = {
	.set_ofs = 0x50,
	.clr_ofs = 0x54,
	.sta_ofs = 0x58,
};

static const struct mtk_gate_regs infra3_cg_regs = {
	.set_ofs = 0x60,
	.clr_ofs = 0x64,
	.sta_ofs = 0x68,
};

#define GATE_INFRA0(_id, _name, _parent, _shift)                               \
	{                                                                      \
		.id = _id, .name = _name, .parent_name = _parent,              \
		.regs = &infra0_cg_regs, .shift = _shift,                      \
		.ops = &mtk_clk_gate_ops_setclr,                               \
	}

#define GATE_INFRA1(_id, _name, _parent, _shift)                               \
	{                                                                      \
		.id = _id, .name = _name, .parent_name = _parent,              \
		.regs = &infra1_cg_regs, .shift = _shift,                      \
		.ops = &mtk_clk_gate_ops_setclr,                               \
	}

#define GATE_INFRA2(_id, _name, _parent, _shift)                               \
	{                                                                      \
		.id = _id, .name = _name, .parent_name = _parent,              \
		.regs = &infra2_cg_regs, .shift = _shift,                      \
		.ops = &mtk_clk_gate_ops_setclr,                               \
	}

#define GATE_INFRA3(_id, _name, _parent, _shift)                               \
	{                                                                      \
		.id = _id, .name = _name, .parent_name = _parent,              \
		.regs = &infra3_cg_regs, .shift = _shift,                      \
		.ops = &mtk_clk_gate_ops_setclr,                               \
	}

static const struct mtk_gate infra_clks[] __initconst = {
	/* INFRA0 */
	GATE_INFRA0(CK_INFRA_PCIE_PERI_26M_CK_P0,
		    "infra_pcie_peri_ck_26m_ck_p0", "infra_f26m_o0", 7),
	GATE_INFRA0(CK_INFRA_PCIE_PERI_26M_CK_P1,
		    "infra_pcie_peri_ck_26m_ck_p1", "infra_f26m_o0", 8),
	GATE_INFRA0(CK_INFRA_PCIE_PERI_26M_CK_P2,
		    "infra_pcie_peri_ck_26m_ck_p2", "infra_f26m_o0", 9),
	GATE_INFRA0(CK_INFRA_PCIE_PERI_26M_CK_P3,
		    "infra_pcie_peri_ck_26m_ck_p3", "infra_f26m_o0", 10),
	/* INFRA1 */
	GATE_INFRA1(CK_INFRA_66M_GPT_BCK, "infra_hf_66m_gpt_bck",
		    "infra_66m_mck", 0),
	GATE_INFRA1(CK_INFRA_66M_PWM_HCK, "infra_hf_66m_pwm_hck",
		    "infra_66m_mck", 1),
	GATE_INFRA1(CK_INFRA_66M_PWM_BCK, "infra_hf_66m_pwm_bck",
		    "infra_pwm_sel", 2),
	GATE_INFRA1(CK_INFRA_66M_PWM_CK1, "infra_hf_66m_pwm_ck1",
		    "infra_pwm_ck1_sel", 3),
	GATE_INFRA1(CK_INFRA_66M_PWM_CK2, "infra_hf_66m_pwm_ck2",
		    "infra_pwm_ck2_sel", 4),
	GATE_INFRA1(CK_INFRA_66M_PWM_CK3, "infra_hf_66m_pwm_ck3",
		    "infra_pwm_ck3_sel", 5),
	GATE_INFRA1(CK_INFRA_66M_PWM_CK4, "infra_hf_66m_pwm_ck4",
		    "infra_pwm_ck4_sel", 6),
	GATE_INFRA1(CK_INFRA_66M_PWM_CK5, "infra_hf_66m_pwm_ck5",
		    "infra_pwm_ck5_sel", 7),
	GATE_INFRA1(CK_INFRA_66M_PWM_CK6, "infra_hf_66m_pwm_ck6",
		    "infra_pwm_ck6_sel", 8),
	GATE_INFRA1(CK_INFRA_66M_PWM_CK7, "infra_hf_66m_pwm_ck7",
		    "infra_pwm_ck7_sel", 9),
	GATE_INFRA1(CK_INFRA_66M_PWM_CK8, "infra_hf_66m_pwm_ck8",
		    "infra_pwm_ck8_sel", 10),
	GATE_INFRA1(CK_INFRA_133M_CQDMA_BCK, "infra_hf_133m_cqdma_bck",
		    "infra_133m_mck", 12),
	GATE_INFRA1(CK_INFRA_66M_AUD_SLV_BCK, "infra_66m_aud_slv_bck",
		    "infra_66m_phck", 13),
	GATE_INFRA1(CK_INFRA_AUD_26M, "infra_f_faud_26m", "infra_ck_f26m", 14),
	GATE_INFRA1(CK_INFRA_AUD_L, "infra_f_faud_l", "infra_faud_l_o", 15),
	GATE_INFRA1(CK_INFRA_AUD_AUD, "infra_f_aud_aud", "infra_faud_aud_o",
		    16),
	GATE_INFRA1(CK_INFRA_AUD_EG2, "infra_f_faud_eg2", "infra_faud_eg2_o",
		    18),
	GATE_INFRA1(CK_INFRA_DRAMC_F26M, "infra_dramc_f26m", "infra_ck_f26m",
		    19),
	GATE_INFRA1(CK_INFRA_133M_DBG_ACKM, "infra_hf_133m_dbg_ackm",
		    "infra_133m_mck", 20),
	GATE_INFRA1(CK_INFRA_66M_AP_DMA_BCK, "infra_66m_ap_dma_bck",
		    "infra_66m_mck", 21),
	GATE_INFRA1(CK_INFRA_66M_SEJ_BCK, "infra_hf_66m_sej_bck",
		    "infra_66m_mck", 29),
	GATE_INFRA1(CK_INFRA_PRE_CK_SEJ_F13M, "infra_pre_ck_sej_f13m",
		    "infra_ck_f26m", 30),
	GATE_INFRA1(CK_INFRA_66M_TRNG, "infra_hf_66m_trng", "infra_peri_66m_o",
		    31),
	/* INFRA2 */
	GATE_INFRA2(CK_INFRA_26M_THERM_SYSTEM, "infra_hf_26m_therm_system",
		    "infra_ck_f26m", 0),
	GATE_INFRA2(CK_INFRA_I2C_BCK, "infra_i2c_bck", "infra_i2c_o", 1),
	GATE_INFRA2(CK_INFRA_66M_UART0_PCK, "infra_hf_66m_uart0_pck",
		    "infra_66m_mck", 3),
	GATE_INFRA2(CK_INFRA_66M_UART1_PCK, "infra_hf_66m_uart1_pck",
		    "infra_66m_mck", 4),
	GATE_INFRA2(CK_INFRA_66M_UART2_PCK, "infra_hf_66m_uart2_pck",
		    "infra_66m_mck", 5),
	GATE_INFRA2(CK_INFRA_52M_UART0_CK, "infra_f_52m_uart0",
		    "infra_mux_uart0_sel", 3),
	GATE_INFRA2(CK_INFRA_52M_UART1_CK, "infra_f_52m_uart1",
		    "infra_mux_uart1_sel", 4),
	GATE_INFRA2(CK_INFRA_52M_UART2_CK, "infra_f_52m_uart2",
		    "infra_mux_uart2_sel", 5),
	GATE_INFRA2(CK_INFRA_NFI, "infra_f_fnfi", "infra_nfi_o", 9),
	GATE_INFRA2(CK_INFRA_SPINFI, "infra_f_fspinfi", "infra_spinfi_o", 10),
	GATE_INFRA2(CK_INFRA_66M_NFI_HCK, "infra_hf_66m_nfi_hck",
		    "infra_66m_mck", 11),
	GATE_INFRA2(CK_INFRA_104M_SPI0, "infra_hf_104m_spi0",
		    "infra_mux_spi0_sel", 12),
	GATE_INFRA2(CK_INFRA_104M_SPI1, "infra_hf_104m_spi1",
		    "infra_mux_spi1_sel", 13),
	GATE_INFRA2(CK_INFRA_104M_SPI2_BCK, "infra_hf_104m_spi2_bck",
		    "infra_mux_spi2_sel", 14),
	GATE_INFRA2(CK_INFRA_66M_SPI0_HCK, "infra_hf_66m_spi0_hck",
		    "infra_66m_mck", 15),
	GATE_INFRA2(CK_INFRA_66M_SPI1_HCK, "infra_hf_66m_spi1_hck",
		    "infra_66m_mck", 16),
	GATE_INFRA2(CK_INFRA_66M_SPI2_HCK, "infra_hf_66m_spi2_hck",
		    "infra_66m_mck", 17),
	GATE_INFRA2(CK_INFRA_66M_FLASHIF_AXI, "infra_hf_66m_flashif_axi",
		    "infra_66m_mck", 18),
	GATE_INFRA2(CK_INFRA_RTC, "infra_f_frtc", "infra_lb_mux_frtc", 19),
	GATE_INFRA2(CK_INFRA_26M_ADC_BCK, "infra_f_26m_adc_bck",
		    "infra_f26m_o1", 20),
	GATE_INFRA2(CK_INFRA_RC_ADC, "infra_f_frc_adc", "infra_f_26m_adc_bck",
		    21),
	GATE_INFRA2(CK_INFRA_MSDC400, "infra_f_fmsdc400", "infra_fmsdc400_o",
		    22),
	GATE_INFRA2(CK_INFRA_MSDC2_HCK, "infra_f_fmsdc2_hck",
		    "infra_fmsdc2_hck_occ", 23),
	GATE_INFRA2(CK_INFRA_133M_MSDC_0_HCK, "infra_hf_133m_msdc_0_hck",
		    "infra_peri_133m", 24),
	GATE_INFRA2(CK_INFRA_66M_MSDC_0_HCK, "infra_66m_msdc_0_hck",
		    "infra_66m_phck", 25),
	GATE_INFRA2(CK_INFRA_133M_CPUM_BCK, "infra_hf_133m_cpum_bck",
		    "infra_133m_mck", 26),
	GATE_INFRA2(CK_INFRA_BIST2FPC, "infra_hf_fbist2fpc", "infra_nfi_o", 27),
	GATE_INFRA2(CK_INFRA_I2C_X16W_MCK_CK_P1, "infra_hf_i2c_x16w_mck_ck_p1",
		    "infra_133m_mck", 29),
	GATE_INFRA2(CK_INFRA_I2C_X16W_PCK_CK_P1, "infra_hf_i2c_x16w_pck_ck_p1",
		    "infra_66m_phck", 31),
	/* INFRA3 */
	GATE_INFRA3(CK_INFRA_133M_USB_HCK, "infra_133m_usb_hck",
		    "infra_133m_phck", 0),
	GATE_INFRA3(CK_INFRA_133M_USB_HCK_CK_P1, "infra_133m_usb_hck_ck_p1",
		    "infra_133m_phck", 1),
	GATE_INFRA3(CK_INFRA_66M_USB_HCK, "infra_66m_usb_hck", "infra_66m_phck",
		    2),
	GATE_INFRA3(CK_INFRA_66M_USB_HCK_CK_P1, "infra_66m_usb_hck_ck_p1",
		    "infra_66m_phck", 3),
	GATE_INFRA3(CK_INFRA_USB_SYS, "infra_usb_sys", "infra_usb_sys_o", 4),
	GATE_INFRA3(CK_INFRA_USB_SYS_CK_P1, "infra_usb_sys_ck_p1",
		    "infra_usb_sys_o_p1", 5),
	GATE_INFRA3(CK_INFRA_USB_REF, "infra_usb_ref", "infra_usb_o", 6),
	GATE_INFRA3(CK_INFRA_USB_CK_P1, "infra_usb_ck_p1", "infra_usb_o_p1", 7),
	GATE_INFRA3(CK_INFRA_USB_FRMCNT, "infra_usb_frmcnt",
		    "infra_usb_frmcnt_o", 8),
	GATE_INFRA3(CK_INFRA_USB_FRMCNT_CK_P1, "infra_usb_frmcnt_ck_p1",
		    "infra_usb_frmcnt_o_p1", 9),
	GATE_INFRA3(CK_INFRA_USB_PIPE, "infra_usb_pipe", "infra_usb_pipe_o",
		    10),
	GATE_INFRA3(CK_INFRA_USB_PIPE_CK_P1, "infra_usb_pipe_ck_p1",
		    "infra_usb_pipe_o_p1", 11),
	GATE_INFRA3(CK_INFRA_USB_UTMI, "infra_usb_utmi", "infra_usb_utmi_o",
		    12),
	GATE_INFRA3(CK_INFRA_USB_UTMI_CK_P1, "infra_usb_utmi_ck_p1",
		    "infra_usb_utmi_o_p1", 13),
	GATE_INFRA3(CK_INFRA_USB_XHCI, "infra_usb_xhci", "infra_usb_xhci_o",
		    14),
	GATE_INFRA3(CK_INFRA_USB_XHCI_CK_P1, "infra_usb_xhci_ck_p1",
		    "infra_usb_xhci_o_p1", 15),
	GATE_INFRA3(CK_INFRA_PCIE_GFMUX_TL_P0, "infra_pcie_gfmux_tl_ck_p0",
		    "infra_pcie_gfmux_tl_o_p0_sel", 20),
	GATE_INFRA3(CK_INFRA_PCIE_GFMUX_TL_P1, "infra_pcie_gfmux_tl_ck_p1",
		    "infra_pcie_gfmux_tl_o_p1_sel", 21),
	GATE_INFRA3(CK_INFRA_PCIE_GFMUX_TL_P2, "infra_pcie_gfmux_tl_ck_p2",
		    "infra_pcie_gfmux_tl_o_p2_sel", 22),
	GATE_INFRA3(CK_INFRA_PCIE_GFMUX_TL_P3, "infra_pcie_gfmux_tl_ck_p3",
		    "infra_pcie_gfmux_tl_o_p3_sel", 23),
	GATE_INFRA3(CK_INFRA_PCIE_PIPE_P0, "infra_pcie_pipe_ck_p0",
		    "infra_pcie_pipe_ck_occ_p0", 24),
	GATE_INFRA3(CK_INFRA_PCIE_PIPE_P1, "infra_pcie_pipe_ck_p1",
		    "infra_pcie_pipe_ck_occ_p1", 25),
	GATE_INFRA3(CK_INFRA_PCIE_PIPE_P2, "infra_pcie_pipe_ck_p2",
		    "infra_pcie_pipe_ck_occ_p2", 26),
	GATE_INFRA3(CK_INFRA_PCIE_PIPE_P3, "infra_pcie_pipe_ck_p3",
		    "infra_pcie_pipe_ck_occ_p3", 27),
	GATE_INFRA3(CK_INFRA_133M_PCIE_CK_P0, "infra_133m_pcie_ck_p0",
		    "infra_133m_phck", 28),
	GATE_INFRA3(CK_INFRA_133M_PCIE_CK_P1, "infra_133m_pcie_ck_p1",
		    "infra_133m_phck", 29),
	GATE_INFRA3(CK_INFRA_133M_PCIE_CK_P2, "infra_133m_pcie_ck_p2",
		    "infra_133m_phck", 30),
	GATE_INFRA3(CK_INFRA_133M_PCIE_CK_P3, "infra_133m_pcie_ck_p3",
		    "infra_133m_phck", 31),
};

static const struct mtk_gate_regs sgmii0_cg_regs = {
	.set_ofs = 0xE4,
	.clr_ofs = 0xE4,
	.sta_ofs = 0xE4,
};

#define GATE_SGMII0(_id, _name, _parent, _shift)                               \
	{                                                                      \
		.id = _id, .name = _name, .parent_name = _parent,              \
		.regs = &sgmii0_cg_regs, .shift = _shift,                      \
		.ops = &mtk_clk_gate_ops_no_setclr_inv,                        \
	}

static const struct mtk_gate sgmii0_clks[] __initconst = {
	GATE_SGMII0(CK_SGM0_TX_EN, "sgm0_tx_en", "clkxtal", 2),
	GATE_SGMII0(CK_SGM0_RX_EN, "sgm0_rx_en", "clkxtal", 3),
};

static const struct mtk_gate_regs sgmii1_cg_regs = {
	.set_ofs = 0xE4,
	.clr_ofs = 0xE4,
	.sta_ofs = 0xE4,
};

#define GATE_SGMII1(_id, _name, _parent, _shift)                               \
	{                                                                      \
		.id = _id, .name = _name, .parent_name = _parent,              \
		.regs = &sgmii1_cg_regs, .shift = _shift,                      \
		.ops = &mtk_clk_gate_ops_no_setclr_inv,                        \
	}

static const struct mtk_gate sgmii1_clks[] __initconst = {
	GATE_SGMII1(CK_SGM1_TX_EN, "sgm1_tx_en", "clkxtal", 2),
	GATE_SGMII1(CK_SGM1_RX_EN, "sgm1_rx_en", "clkxtal", 3),
};

static const struct mtk_gate_regs ethdma_cg_regs = {
	.set_ofs = 0x30,
	.clr_ofs = 0x30,
	.sta_ofs = 0x30,
};

#define GATE_ETHDMA(_id, _name, _parent, _shift)                               \
	{                                                                      \
		.id = _id, .name = _name, .parent_name = _parent,              \
		.regs = &ethdma_cg_regs, .shift = _shift,                      \
		.ops = &mtk_clk_gate_ops_no_setclr_inv,                        \
	}

static const struct mtk_gate ethdma_clks[] __initconst = {
	GATE_ETHDMA(CK_ETHDMA_XGP1_EN, "ethdma_xgp1_en", "clkxtal", 0),
	GATE_ETHDMA(CK_ETHDMA_XGP2_EN, "ethdma_xgp2_en", "clkxtal", 1),
	GATE_ETHDMA(CK_ETHDMA_XGP3_EN, "ethdma_xgp3_en", "clkxtal", 2),
	GATE_ETHDMA(CK_ETHDMA_FE_EN, "ethdma_fe_en", "netsys_2x", 6),
	GATE_ETHDMA(CK_ETHDMA_GP2_EN, "ethdma_gp2_en", "clkxtal", 7),
	GATE_ETHDMA(CK_ETHDMA_GP1_EN, "ethdma_gp1_en", "clkxtal", 8),
	GATE_ETHDMA(CK_ETHDMA_GP3_EN, "ethdma_gp3_en", "clkxtal", 10),
	GATE_ETHDMA(CK_ETHDMA_ESW_EN, "ethdma_esw_en", "netsys_gsw", 16),
	GATE_ETHDMA(CK_ETHDMA_CRYPT0_EN, "ethdma_crypt0_en", "eip197", 29),
};

static const struct mtk_gate_regs ethwarp_cg_regs = {
	.set_ofs = 0x14,
	.clr_ofs = 0x14,
	.sta_ofs = 0x14,
};

#define GATE_ETHWARP(_id, _name, _parent, _shift)                              \
	{                                                                      \
		.id = _id, .name = _name, .parent_name = _parent,              \
		.regs = &ethwarp_cg_regs, .shift = _shift,                     \
		.ops = &mtk_clk_gate_ops_no_setclr_inv,                        \
	}

static const struct mtk_gate ethwarp_clks[] __initconst = {
	GATE_ETHWARP(CK_ETHWARP_WOCPU2_EN, "ethwarp_wocpu2_en",
		     "netsys_wed_mcu", 13),
	GATE_ETHWARP(CK_ETHWARP_WOCPU1_EN, "ethwarp_wocpu1_en",
		     "netsys_wed_mcu", 14),
	GATE_ETHWARP(CK_ETHWARP_WOCPU0_EN, "ethwarp_wocpu0_en",
		     "netsys_wed_mcu", 15),
};

#define MT7988_PLL_FMAX	     (2500UL * MHZ)
#define MT7988_PCW_CHG_SHIFT 2

#define PLL_B(_id, _name, _reg, _pwr_reg, _en_mask, _flags, _rst_bar_mask,     \
	      _pcwbits, _pd_reg, _pd_shift, _tuner_reg, _tuner_en_reg,         \
	      _tuner_en_bit, _pcw_reg, _pcw_shift, _pcw_chg_reg, _div_table,   \
	      _parent_name)                                                    \
	{                                                                      \
		.id = _id, .name = _name, .reg = _reg, .pwr_reg = _pwr_reg,    \
		.en_mask = _en_mask, .flags = _flags,                          \
		.rst_bar_mask = BIT(_rst_bar_mask), .fmax = MT7988_PLL_FMAX,   \
		.pcwbits = _pcwbits, .pd_reg = _pd_reg, .pd_shift = _pd_shift, \
		.tuner_reg = _tuner_reg, .tuner_en_reg = _tuner_en_reg,        \
		.tuner_en_bit = _tuner_en_bit, .pcw_reg = _pcw_reg,            \
		.pcw_shift = _pcw_shift, .pcw_chg_reg = _pcw_chg_reg,          \
		.pcw_chg_shift = MT7988_PCW_CHG_SHIFT,                         \
		.div_table = _div_table, .parent_name = _parent_name,          \
	}

#define PLL(_id, _name, _reg, _pwr_reg, _en_mask, _flags, _rst_bar_mask,       \
	    _pcwbits, _pd_reg, _pd_shift, _tuner_reg, _tuner_en_reg,           \
	    _tuner_en_bit, _pcw_reg, _pcw_shift, _pcw_chg_reg, _parent_name)   \
	PLL_B(_id, _name, _reg, _pwr_reg, _en_mask, _flags, _rst_bar_mask,     \
	      _pcwbits, _pd_reg, _pd_shift, _tuner_reg, _tuner_en_reg,         \
	      _tuner_en_bit, _pcw_reg, _pcw_shift, _pcw_chg_reg, NULL,         \
	      _parent_name)

static const struct mtk_pll_data plls[] = {
	PLL(CK_APMIXED_NETSYSPLL, "netsyspll", 0x0104, 0x0110, 0x00000001, 0, 0,
	    32, 0x0104, 4, 0, 0, 0, 0x0108, 0, 0x0104, "clkxtal"),
	PLL(CK_APMIXED_MPLL, "mpll", 0x0114, 0x0120, 0xff000001, HAVE_RST_BAR,
	    23, 32, 0x0114, 4, 0, 0, 0, 0x0118, 0, 0x0114, "clkxtal"),
	PLL(CK_APMIXED_MMPLL, "mmpll", 0x0124, 0x0130, 0xff000001, HAVE_RST_BAR,
	    23, 32, 0x0124, 4, 0, 0, 0, 0x0128, 0, 0x0124, "clkxtal"),
	PLL(CK_APMIXED_APLL2, "apll2", 0x0134, 0x0140, 0x00000001, 0, 0, 32,
	    0x0134, 4, 0x0704, 0x0700, 1, 0x0138, 0, 0x0134, "clkxtal"),
	PLL(CK_APMIXED_NET1PLL, "net1pll", 0x0144, 0x0150, 0xff000001,
	    HAVE_RST_BAR, 23, 32, 0x0144, 4, 0, 0, 0, 0x0148, 0, 0x0144,
	    "clkxtal"),
	PLL(CK_APMIXED_NET2PLL, "net2pll", 0x0154, 0x0160, 0xff000001,
	    HAVE_RST_BAR, 23, 32, 0x0154, 4, 0, 0, 0, 0x0158, 0, 0x0154,
	    "clkxtal"),
	PLL(CK_APMIXED_WEDMCUPLL, "wedmcupll", 0x0164, 0x0170, 0x00000001, 0, 0,
	    32, 0x0164, 4, 0, 0, 0, 0x0168, 0, 0x0164, "clkxtal"),
	PLL(CK_APMIXED_SGMPLL, "sgmpll", 0x0174, 0x0180, 0x00000001, 0, 0, 32,
	    0x0174, 4, 0, 0, 0, 0x0178, 0, 0x0174, "clkxtal"),
	PLL(CK_APMIXED_ARM_B, "arm_b", 0x0204, 0x0210, 0xff000001, HAVE_RST_BAR,
	    23, 32, 0x0204, 4, 0, 0, 0, 0x0208, 0, 0x0204, "clkxtal"),
	PLL(CK_APMIXED_CCIPLL2_B, "ccipll2_b", 0x0214, 0x0220, 0xff000001,
	    HAVE_RST_BAR, 23, 32, 0x0214, 4, 0, 0, 0, 0x0218, 0, 0x0214,
	    "clkxtal"),
	PLL(CK_APMIXED_USXGMIIPLL, "usxgmiipll", 0x0304, 0x0310, 0xff000001,
	    HAVE_RST_BAR, 23, 32, 0x0304, 4, 0, 0, 0, 0x0308, 0, 0x0304,
	    "clkxtal"),
	PLL(CK_APMIXED_MSDCPLL, "msdcpll", 0x0314, 0x0320, 0x00000001, 0, 0, 32,
	    0x0314, 4, 0, 0, 0, 0x0318, 0, 0x0314, "clkxtal"),
};

static struct clk_onecell_data *mt7988_top_clk_data __initdata;
static struct clk_onecell_data *mt7988_pll_clk_data __initdata;

static void __init mtk_clk_enable_critical(void)
{
	if (!mt7988_top_clk_data || !mt7988_pll_clk_data)
		return;

	clk_prepare_enable(mt7988_pll_clk_data->clks[CK_APMIXED_ARM_B]);
	clk_prepare_enable(mt7988_top_clk_data->clks[CK_TOP_SYSAXI_SEL]);
	clk_prepare_enable(mt7988_top_clk_data->clks[CK_TOP_SYSAPB_SEL]);
	clk_prepare_enable(mt7988_top_clk_data->clks[CK_TOP_DRAMC_SEL]);
	clk_prepare_enable(mt7988_top_clk_data->clks[CK_TOP_DRAMC_MD32_SEL]);
	clk_prepare_enable(mt7988_top_clk_data->clks[CK_TOP_INFRA_F26M_SEL]);
}

static void __init mtk_infracfg_init(struct device_node *node)
{
	int r;

	mt7988_top_clk_data = mtk_alloc_clk_data(CLK_INFRA_NR_CLK);

	mtk_clk_register_factors(infra_divs, ARRAY_SIZE(infra_divs),
				 mt7988_top_clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get,
				mt7988_top_clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);

	mtk_clk_enable_critical();
}
CLK_OF_DECLARE(mtk_infracfg, "mediatek,mt7988-infracfg", mtk_infracfg_init);

static void __init mtk_topckgen_init(struct device_node *node)
{
	int r;
	void __iomem *base;

	base = of_iomap(node, 0);
	if (!base) {
		pr_err("%s(): ioremap failed\n", __func__);
		return;
	}

	mt7988_top_clk_data = mtk_alloc_clk_data(CLK_TOP_NR_CLK);

	mtk_clk_register_factors(top_divs, ARRAY_SIZE(top_divs),
				 mt7988_top_clk_data);
	mtk_clk_register_muxes(top_muxes, ARRAY_SIZE(top_muxes), node,
			       &mt7988_clk_lock, mt7988_top_clk_data);
	mtk_clk_register_composites(top_aud_divs, ARRAY_SIZE(top_aud_divs),
		base, &mt7988_clk_lock, mt7988_top_clk_data);
	r = of_clk_add_provider(node, of_clk_src_onecell_get,
				mt7988_top_clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);

	mtk_clk_enable_critical();
}
CLK_OF_DECLARE(mtk_topckgen, "mediatek,mt7988-topckgen", mtk_topckgen_init);

static void __init mtk_infracfg_ao_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	int r;
	void __iomem *base;

	base = of_iomap(node, 0);
	if (!base) {
		pr_err("%s(): ioremap failed\n", __func__);
		return;
	}

	clk_data = mtk_alloc_clk_data(CLK_INFRA_AO_NR_CLK);

	mtk_clk_register_muxes(infra_muxes, ARRAY_SIZE(infra_muxes), node,
			       &mt7988_clk_lock, clk_data);
	mtk_clk_register_gates(node, infra_clks, ARRAY_SIZE(infra_clks),
			       clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);
}
CLK_OF_DECLARE(mtk_infracfg_ao, "mediatek,mt7988-infracfg_ao",
	       mtk_infracfg_ao_init);

static void __init mtk_apmixedsys_init(struct device_node *node)
{
	int r;

	mt7988_pll_clk_data = mtk_alloc_clk_data(CLK_APMIXED_NR_CLK);

	mtk_clk_register_plls(node, plls, ARRAY_SIZE(plls),
			      mt7988_pll_clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get,
				mt7988_pll_clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);

	mtk_clk_enable_critical();
}
CLK_OF_DECLARE(mtk_apmixedsys, "mediatek,mt7988-apmixedsys",
	       mtk_apmixedsys_init);

static void __init mtk_mcusys_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	int r;
	void __iomem *base;

	base = of_iomap(node, 0);
	if (!base) {
		pr_err("%s(): ioremap failed\n", __func__);
		return;
	}

	clk_data = mtk_alloc_clk_data(CLK_MCU_NR_CLK);
	mtk_clk_register_composites(mcu_muxes, ARRAY_SIZE(mcu_muxes), base,
				    &mt7988_clk_lock, clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);
}
CLK_OF_DECLARE(mtk_mcusys, "mediatek,mt7988-mcusys", mtk_mcusys_init);

static void __init mtk_sgmiisys_0_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	int r;

	clk_data = mtk_alloc_clk_data(CLK_SGMII0_NR_CLK);

	mtk_clk_register_gates(node, sgmii0_clks, ARRAY_SIZE(sgmii0_clks),
			       clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);
}
CLK_OF_DECLARE(mtk_sgmiisys_0, "mediatek,mt7988-sgmiisys_0",
	       mtk_sgmiisys_0_init);

static void __init mtk_sgmiisys_1_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	int r;

	clk_data = mtk_alloc_clk_data(CLK_SGMII1_NR_CLK);

	mtk_clk_register_gates(node, sgmii1_clks, ARRAY_SIZE(sgmii1_clks),
			       clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);
}
CLK_OF_DECLARE(mtk_sgmiisys_1, "mediatek,mt7988-sgmiisys_1",
	       mtk_sgmiisys_1_init);

static void __init mtk_ethdma_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	int r;

	clk_data = mtk_alloc_clk_data(CLK_ETHDMA_NR_CLK);

	mtk_clk_register_gates(node, ethdma_clks, ARRAY_SIZE(ethdma_clks),
			       clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);
}
CLK_OF_DECLARE(mtk_ethdma, "mediatek,mt7988-ethsys", mtk_ethdma_init);

static void __init mtk_ethwarp_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	int r;

	clk_data = mtk_alloc_clk_data(CLK_ETHWARP_NR_CLK);

	mtk_clk_register_gates(node, ethwarp_clks, ARRAY_SIZE(ethwarp_clks),
			       clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);
}
CLK_OF_DECLARE(mtk_ethwarp, "mediatek,mt7988-ethwarp", mtk_ethwarp_init);
