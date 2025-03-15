/*
 * Copyright (c) 2021 MediaTek Inc.
 * Author: Wenzhen Yu<Yenzhen.Yu@mediatek.com>
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
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/mfd/syscon.h>

#include "clk-mtk.h"
#include "clk-gate.h"
#include "clk-mux.h"

#include <dt-bindings/clock/mt7981-clk.h>

static DEFINE_SPINLOCK(mt7981_clk_lock);

static const struct mtk_fixed_factor infra_divs[] __initconst = {
	FACTOR(CK_INFRA_CK_F26M, "infra_ck_f26m", "csw_f26m_sel", 1, 1),
	FACTOR(CK_INFRA_UART, "infra_uart", "uart_sel", 1, 1),
	FACTOR(CK_INFRA_ISPI0, "infra_ispi0", "spi_sel", 1, 1),
	FACTOR(CK_INFRA_I2C, "infra_i2c", "i2c_sel", 1, 1),
	FACTOR(CK_INFRA_ISPI1, "infra_ispi1", "spim_mst_sel", 1, 1),
	FACTOR(CK_INFRA_PWM, "infra_pwm", "pwm_sel", 1, 1),
	FACTOR(CK_INFRA_66M_MCK, "infra_66m_mck", "sysaxi_sel", 1, 2),
	FACTOR(CK_INFRA_CK_F32K, "infra_ck_f32k", "cb_rtc_32p7k", 1, 1),
	FACTOR(CK_INFRA_PCIE_CK, "infra_pcie", "pextp_tl_ck_sel", 1, 1),
	FACTOR(CK_INFRA_PWM_BCK, "infra_pwm_bck", "infra_pwm_bsel", 1, 1),
	FACTOR(CK_INFRA_PWM_CK1, "infra_pwm_ck1", "infra_pwm1_sel", 1, 1),
	FACTOR(CK_INFRA_PWM_CK2, "infra_pwm_ck2", "infra_pwm2_sel", 1, 1),
	FACTOR(CK_INFRA_133M_HCK, "infra_133m_hck", "sysaxi", 1, 1),
	FACTOR(CK_INFRA_66M_PHCK, "infra_66m_phck", "infra_133m_hck", 1, 1),
	FACTOR(CK_INFRA_FAUD_L_CK, "infra_faud_l", "aud_l", 1, 1),
	FACTOR(CK_INFRA_FAUD_AUD_CK, "infra_faud_aud", "a1sys", 1, 1),
	FACTOR(CK_INFRA_FAUD_EG2_CK, "infra_faud_eg2", "a_tuner", 1, 1),
	FACTOR(CK_INFRA_I2CS_CK, "infra_i2cs", "i2c_bck", 1, 1),
	FACTOR(CK_INFRA_MUX_UART0, "infra_mux_uart0", "infra_uart0_sel", 1, 1),
	FACTOR(CK_INFRA_MUX_UART1, "infra_mux_uart1", "infra_uart1_sel", 1, 1),
	FACTOR(CK_INFRA_MUX_UART2, "infra_mux_uart2", "infra_uart2_sel", 1, 1),
	FACTOR(CK_INFRA_NFI_CK, "infra_nfi", "nfi1x", 1, 1),
	FACTOR(CK_INFRA_SPINFI_CK, "infra_spinfi", "spinfi_bck", 1, 1),
	FACTOR(CK_INFRA_MUX_SPI0, "infra_mux_spi0", "infra_spi0_sel", 1, 1),
	FACTOR(CK_INFRA_MUX_SPI1, "infra_mux_spi1", "infra_spi1_sel", 1, 1),
	FACTOR(CK_INFRA_MUX_SPI2, "infra_mux_spi2", "infra_spi2_sel", 1, 1),
	FACTOR(CK_INFRA_RTC_32K, "infra_rtc_32k", "cb_rtc_32k", 1, 1),
	FACTOR(CK_INFRA_FMSDC_CK, "infra_fmsdc", "emmc_400m", 1, 1),
	FACTOR(CK_INFRA_FMSDC_HCK_CK, "infra_fmsdc_hck", "emmc_208m", 1, 1),
	FACTOR(CK_INFRA_PERI_133M, "infra_peri_133m", "sysaxi", 1, 1),
	FACTOR(CK_INFRA_133M_PHCK, "infra_133m_phck", "sysaxi", 1, 1),
	FACTOR(CK_INFRA_USB_SYS_CK, "infra_usb_sys", "u2u3_sys", 1, 1),
	FACTOR(CK_INFRA_USB_CK, "infra_usb", "u2u3_ref", 1, 1),
	FACTOR(CK_INFRA_USB_XHCI_CK, "infra_usb_xhci", "u2u3_xhci", 1, 1),
	FACTOR(CK_INFRA_PCIE_GFMUX_TL_O_PRE, "infra_pcie_mux", "pextp_tl", 1, 1),
	FACTOR(CK_INFRA_F26M_CK0, "infra_f26m_ck0", "csw_f26m", 1, 1),
	FACTOR(CK_INFRA_133M_MCK, "infra_133m_mck", "sysaxi", 1, 1),
};

static const struct mtk_fixed_factor top_divs[] __initconst = {
	FACTOR(CK_TOP_CB_CKSQ_40M, "cb_cksq_40m", "clkxtal", 1, 1),
	FACTOR(CK_TOP_CB_M_416M, "cb_m_416m", "mpll", 1, 1),
	FACTOR(CK_TOP_CB_M_D2, "cb_m_d2", "mpll", 1, 2),
	FACTOR(CK_TOP_CB_M_D3, "cb_m_d3", "mpll", 1, 3),
	FACTOR(CK_TOP_M_D3_D2, "m_d3_d2", "mpll", 1, 2),
	FACTOR(CK_TOP_CB_M_D4, "cb_m_d4", "mpll", 1, 4),
	FACTOR(CK_TOP_CB_M_D8, "cb_m_d8", "mpll", 1, 8),
	FACTOR(CK_TOP_M_D8_D2, "m_d8_d2", "mpll", 1, 16),
	FACTOR(CK_TOP_CB_MM_720M, "cb_mm_720m", "mmpll", 1, 1),
	FACTOR(CK_TOP_CB_MM_D2, "cb_mm_d2", "mmpll", 1, 2),
	FACTOR(CK_TOP_CB_MM_D3, "cb_mm_d3", "mmpll", 1, 3),
	FACTOR(CK_TOP_CB_MM_D3_D5, "cb_mm_d3_d5", "mmpll", 1, 15),
	FACTOR(CK_TOP_CB_MM_D4, "cb_mm_d4", "mmpll", 1, 4),
	FACTOR(CK_TOP_CB_MM_D6, "cb_mm_d6", "mmpll", 1, 6),
	FACTOR(CK_TOP_MM_D6_D2, "mm_d6_d2", "mmpll", 1, 12),
	FACTOR(CK_TOP_CB_MM_D8, "cb_mm_d8", "mmpll", 1, 8),
	FACTOR(CK_TOP_CB_APLL2_196M, "cb_apll2_196m", "apll2", 1, 1),
	FACTOR(CK_TOP_APLL2_D2, "apll2_d2", "apll2", 1, 2),
	FACTOR(CK_TOP_APLL2_D4, "apll2_d4", "apll2", 1, 4),
	FACTOR(CK_TOP_NET1_2500M, "net1_2500m", "net1pll", 1, 1),
	FACTOR(CK_TOP_CB_NET1_D4, "cb_net1_d4", "net1pll", 1, 4),
	FACTOR(CK_TOP_CB_NET1_D5, "cb_net1_d5", "net1pll", 1, 5),
	FACTOR(CK_TOP_NET1_D5_D2, "net1_d5_d2", "net1pll", 1, 10),
	FACTOR(CK_TOP_NET1_D5_D4, "net1_d5_d4", "net1pll", 1, 20),
	FACTOR(CK_TOP_CB_NET1_D8, "cb_net1_d8", "net1pll", 1, 8),
	FACTOR(CK_TOP_NET1_D8_D2, "net1_d8_d2", "net1pll", 1, 16),
	FACTOR(CK_TOP_NET1_D8_D4, "net1_d8_d4", "net1pll", 1, 32),
	FACTOR(CK_TOP_CB_NET2_800M, "cb_net2_800m", "net2pll", 1, 1),
	FACTOR(CK_TOP_CB_NET2_D2, "cb_net2_d2", "net2pll", 1, 2),
	FACTOR(CK_TOP_CB_NET2_D4, "cb_net2_d4", "net2pll", 1, 4),
	FACTOR(CK_TOP_NET2_D4_D2, "net2_d4_d2", "net2pll", 1, 8),
	FACTOR(CK_TOP_NET2_D4_D4, "net2_d4_d4", "net2pll", 1, 16),
	FACTOR(CK_TOP_CB_NET2_D6, "cb_net2_d6", "net2pll", 1, 6),
	FACTOR(CK_TOP_CB_WEDMCU_208M, "cb_wedmcu_208m", "wedmcupll", 1, 1),
	FACTOR(CK_TOP_CB_SGM_325M, "cb_sgm_325m", "sgmpll", 1, 1),
	FACTOR(CK_TOP_CKSQ_40M_D2, "cksq_40m_d2", "cb_cksq_40m", 1, 2),
	FACTOR(CK_TOP_CB_RTC_32K, "cb_rtc_32k", "cb_cksq_40m", 1, 1250),
	FACTOR(CK_TOP_CB_RTC_32P7K, "cb_rtc_32p7k", "cb_cksq_40m", 1, 1220),
	FACTOR(CK_TOP_USB_TX250M, "usb_tx250m", "cb_cksq_40m", 1, 1),
	FACTOR(CK_TOP_FAUD, "faud", "aud_sel", 1, 1),
	FACTOR(CK_TOP_NFI1X, "nfi1x", "nfi1x_sel", 1, 1),
	FACTOR(CK_TOP_USB_EQ_RX250M, "usb_eq_rx250m", "cb_cksq_40m", 1, 1),
	FACTOR(CK_TOP_USB_CDR_CK, "usb_cdr", "cb_cksq_40m", 1, 1),
	FACTOR(CK_TOP_USB_LN0_CK, "usb_ln0", "cb_cksq_40m", 1, 1),
	FACTOR(CK_TOP_SPINFI_BCK, "spinfi_bck", "spinfi_sel", 1, 1),
	FACTOR(CK_TOP_SPI, "spi", "spi_sel", 1, 1),
	FACTOR(CK_TOP_SPIM_MST, "spim_mst", "spim_mst_sel", 1, 1),
	FACTOR(CK_TOP_UART_BCK, "uart_bck", "uart_sel", 1, 1),
	FACTOR(CK_TOP_PWM_BCK, "pwm_bck", "pwm_sel", 1, 1),
	FACTOR(CK_TOP_I2C_BCK, "i2c_bck", "i2c_sel", 1, 1),
	FACTOR(CK_TOP_PEXTP_TL, "pextp_tl", "pextp_tl_ck_sel", 1, 1),
	FACTOR(CK_TOP_EMMC_208M, "emmc_208m", "emmc_208m_sel", 1, 1),
	FACTOR(CK_TOP_EMMC_400M, "emmc_400m", "emmc_400m_sel", 1, 1),
	FACTOR(CK_TOP_DRAMC_REF, "dramc_ref", "dramc_sel", 1, 1),
	FACTOR(CK_TOP_DRAMC_MD32, "dramc_md32", "dramc_md32_sel", 1, 1),
	FACTOR(CK_TOP_SYSAXI, "sysaxi", "sysaxi_sel", 1, 1),
	FACTOR(CK_TOP_SYSAPB, "sysapb", "sysapb_sel", 1, 1),
	FACTOR(CK_TOP_ARM_DB_MAIN, "arm_db_main", "arm_db_main_sel", 1, 1),
	FACTOR(CK_TOP_AP2CNN_HOST, "ap2cnn_host", "ap2cnn_host_sel", 1, 1),
	FACTOR(CK_TOP_NETSYS, "netsys", "netsys_sel", 1, 1),
	FACTOR(CK_TOP_NETSYS_500M, "netsys_500m", "netsys_500m_sel", 1, 1),
	FACTOR(CK_TOP_NETSYS_WED_MCU, "netsys_wed_mcu", "netsys_mcu_sel", 1, 1),
	FACTOR(CK_TOP_NETSYS_2X, "netsys_2x", "netsys_2x_sel", 1, 1),
	FACTOR(CK_TOP_SGM_325M, "sgm_325m", "sgm_325m_sel", 1, 1),
	FACTOR(CK_TOP_SGM_REG, "sgm_reg", "sgm_reg_sel", 1, 1),
	FACTOR(CK_TOP_F26M, "csw_f26m", "csw_f26m_sel", 1, 1),
	FACTOR(CK_TOP_EIP97B, "eip97b", "eip97b_sel", 1, 1),
	FACTOR(CK_TOP_USB3_PHY, "usb3_phy", "usb3_phy_sel", 1, 1),
	FACTOR(CK_TOP_AUD, "aud", "faud", 1, 1),
	FACTOR(CK_TOP_A1SYS, "a1sys", "a1sys_sel", 1, 1),
	FACTOR(CK_TOP_AUD_L, "aud_l", "aud_l_sel", 1, 1),
	FACTOR(CK_TOP_A_TUNER, "a_tuner", "a_tuner_sel", 1, 1),
	FACTOR(CK_TOP_U2U3_REF, "u2u3_ref", "u2u3_sel", 1, 1),
	FACTOR(CK_TOP_U2U3_SYS, "u2u3_sys", "u2u3_sys_sel", 1, 1),
	FACTOR(CK_TOP_U2U3_XHCI, "u2u3_xhci", "u2u3_xhci_sel", 1, 1),
	FACTOR(CK_TOP_USB_FRMCNT, "usb_frmcnt", "usb_frmcnt_sel", 1, 1),
};

static const char * const nfi1x_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_mm_d4",
	"net1_d8_d2",
	"cb_net2_d6",
	"cb_m_d4",
	"cb_mm_d8",
	"net1_d8_d4",
	"cb_m_d8"
};

static const char * const spinfi_parents[] __initconst = {
	"cksq_40m_d2",
	"cb_cksq_40m",
	"net1_d5_d4",
	"cb_m_d4",
	"cb_mm_d8",
	"net1_d8_d4",
	"mm_d6_d2",
	"cb_m_d8"
};

static const char * const spi_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_m_d2",
	"cb_mm_d4",
	"net1_d8_d2",
	"cb_net2_d6",
	"net1_d5_d4",
	"cb_m_d4",
	"net1_d8_d4"
};

static const char * const uart_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_m_d8",
	"m_d8_d2"
};

static const char * const pwm_parents[] __initconst = {
	"cb_cksq_40m",
	"net1_d8_d2",
	"net1_d5_d4",
	"cb_m_d4",
	"m_d8_d2",
	"cb_rtc_32k"
};

static const char * const i2c_parents[] __initconst = {
	"cb_cksq_40m",
	"net1_d5_d4",
	"cb_m_d4",
	"net1_d8_d4"
};

static const char * const pextp_tl_ck_parents[] __initconst = {
	"cb_cksq_40m",
	"net1_d5_d4",
	"cb_m_d4",
	"cb_rtc_32k"
};

static const char * const emmc_208m_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_m_d2",
	"cb_net2_d4",
	"cb_apll2_196m",
	"cb_mm_d4",
	"net1_d8_d2",
	"cb_mm_d6"
};

static const char * const emmc_400m_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_net2_d2",
	"cb_mm_d2",
	"cb_net2_d2"
};

static const char * const csw_f26m_parents[] __initconst = {
	"cksq_40m_d2",
	"m_d8_d2"
};

static const char * const dramc_md32_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_m_d2",
	"cb_wedmcu_208m"
};

static const char * const sysaxi_parents[] __initconst = {
	"cb_cksq_40m",
	"net1_d8_d2"
};

static const char * const sysapb_parents[] __initconst = {
	"cb_cksq_40m",
	"m_d3_d2"
};

static const char * const arm_db_main_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_net2_d6"
};

static const char * const ap2cnn_host_parents[] __initconst = {
	"cb_cksq_40m",
	"net1_d8_d4"
};

static const char * const netsys_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_mm_d2"
};

static const char * const netsys_500m_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_net1_d5"
};

static const char * const netsys_mcu_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_mm_720m",
	"cb_net1_d4",
	"cb_net1_d5",
	"cb_m_416m"
};

static const char * const netsys_2x_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_net2_800m",
	"cb_mm_720m"
};

static const char * const sgm_325m_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_sgm_325m"
};

static const char * const sgm_reg_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_net2_d4"
};

static const char * const eip97b_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_net1_d5",
	"cb_m_416m",
	"cb_mm_d2",
	"net1_d5_d2"
};

static const char * const aud_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_apll2_196m"
};

static const char * const a1sys_parents[] __initconst = {
	"cb_cksq_40m",
	"apll2_d4"
};

static const char * const aud_l_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_apll2_196m",
	"m_d8_d2"
};

static const char * const a_tuner_parents[] __initconst = {
	"cb_cksq_40m",
	"apll2_d4",
	"m_d8_d2"
};

static const char * const u2u3_parents[] __initconst = {
	"cb_cksq_40m",
	"m_d8_d2"
};

static const char * const u2u3_sys_parents[] __initconst = {
	"cb_cksq_40m",
	"net1_d5_d4"
};

static const char * const usb_frmcnt_parents[] __initconst = {
	"cb_cksq_40m",
	"cb_mm_d3_d5"
};

static const struct mtk_mux top_muxes[] = {
	/* CLK_CFG_0 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_NFI1X_SEL, "nfi1x_sel",
	    nfi1x_parents, 0x000, 0x004, 0x008, 0, 3, 7, 0x1C0, 0),
	MUX_GATE_CLR_SET_UPD(CK_TOP_SPINFI_SEL, "spinfi_sel",
	    spinfi_parents, 0x000, 0x004, 0x008, 8, 3, 15, 0x1C0, 1),
	MUX_GATE_CLR_SET_UPD(CK_TOP_SPI_SEL, "spi_sel",
	    spi_parents, 0x000, 0x004, 0x008, 16, 3, 23, 0x1C0, 2),
	MUX_GATE_CLR_SET_UPD(CK_TOP_SPIM_MST_SEL, "spim_mst_sel",
	    spi_parents, 0x000, 0x004, 0x008, 24, 3, 31, 0x1C0, 3),
	/* CLK_CFG_1 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_UART_SEL, "uart_sel",
	    uart_parents, 0x010, 0x014, 0x018, 0, 2, 7, 0x1C0, 4),
	MUX_GATE_CLR_SET_UPD(CK_TOP_PWM_SEL, "pwm_sel",
	    pwm_parents, 0x010, 0x014, 0x018, 8, 3, 15, 0x1C0, 5),
	MUX_GATE_CLR_SET_UPD(CK_TOP_I2C_SEL, "i2c_sel",
	    i2c_parents, 0x010, 0x014, 0x018, 16, 2, 23, 0x1C0, 6),
	MUX_GATE_CLR_SET_UPD(CK_TOP_PEXTP_TL_SEL, "pextp_tl_ck_sel",
	    pextp_tl_ck_parents, 0x010, 0x014, 0x018, 24, 2, 31, 0x1C0, 7),
	/* CLK_CFG_2 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_EMMC_208M_SEL, "emmc_208m_sel",
	    emmc_208m_parents, 0x020, 0x024, 0x028, 0, 3, 7, 0x1C0, 8),
	MUX_GATE_CLR_SET_UPD(CK_TOP_EMMC_400M_SEL, "emmc_400m_sel",
	    emmc_400m_parents, 0x020, 0x024, 0x028, 8, 2, 15, 0x1C0, 9),
	MUX_GATE_CLR_SET_UPD(CK_TOP_F26M_SEL, "csw_f26m_sel",
	    csw_f26m_parents, 0x020, 0x024, 0x028, 16, 1, 23, 0x1C0, 10),
	MUX_GATE_CLR_SET_UPD(CK_TOP_DRAMC_SEL, "dramc_sel",
	    csw_f26m_parents, 0x020, 0x024, 0x028, 24, 1, 31, 0x1C0, 11),
	/* CLK_CFG_3 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_DRAMC_MD32_SEL, "dramc_md32_sel",
	    dramc_md32_parents, 0x030, 0x034, 0x038, 0, 2, 7, 0x1C0, 12),
	MUX_GATE_CLR_SET_UPD(CK_TOP_SYSAXI_SEL, "sysaxi_sel",
	    sysaxi_parents, 0x030, 0x034, 0x038, 8, 1, 15, 0x1C0, 13),
	MUX_GATE_CLR_SET_UPD(CK_TOP_SYSAPB_SEL, "sysapb_sel",
	    sysapb_parents, 0x030, 0x034, 0x038, 16, 1, 23, 0x1C0, 14),
	MUX_GATE_CLR_SET_UPD(CK_TOP_ARM_DB_MAIN_SEL, "arm_db_main_sel",
	    arm_db_main_parents, 0x030, 0x034, 0x038, 24, 1, 31, 0x1C0, 15),
	/* CLK_CFG_4 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_AP2CNN_HOST_SEL, "ap2cnn_host_sel",
	    ap2cnn_host_parents, 0x040, 0x044, 0x048, 0, 1, 7, 0x1C0, 16),
	MUX_GATE_CLR_SET_UPD(CK_TOP_NETSYS_SEL, "netsys_sel",
	    netsys_parents, 0x040, 0x044, 0x048, 8, 1, 15, 0x1C0, 17),
	MUX_GATE_CLR_SET_UPD(CK_TOP_NETSYS_500M_SEL, "netsys_500m_sel",
	    netsys_500m_parents, 0x040, 0x044, 0x048, 16, 1, 23, 0x1C0, 18),
	MUX_GATE_CLR_SET_UPD(CK_TOP_NETSYS_MCU_SEL, "netsys_mcu_sel",
	    netsys_mcu_parents, 0x040, 0x044, 0x048, 24, 3, 31, 0x1C0, 19),
	/* CLK_CFG_5 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_NETSYS_2X_SEL, "netsys_2x_sel",
	    netsys_2x_parents, 0x050, 0x054, 0x058, 0, 2, 7, 0x1C0, 20),
	MUX_GATE_CLR_SET_UPD(CK_TOP_SGM_325M_SEL, "sgm_325m_sel",
	    sgm_325m_parents, 0x050, 0x054, 0x058, 8, 1, 15, 0x1C0, 21),
	MUX_GATE_CLR_SET_UPD(CK_TOP_SGM_REG_SEL, "sgm_reg_sel",
	    sgm_reg_parents, 0x050, 0x054, 0x058, 16, 1, 23, 0x1C0, 22),
	MUX_GATE_CLR_SET_UPD(CK_TOP_EIP97B_SEL, "eip97b_sel",
	    eip97b_parents, 0x050, 0x054, 0x058, 24, 3, 31, 0x1C0, 23),
	/* CLK_CFG_6 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_USB3_PHY_SEL, "usb3_phy_sel",
	    csw_f26m_parents, 0x060, 0x064, 0x068, 0, 1, 7, 0x1C0, 24),
	MUX_GATE_CLR_SET_UPD(CK_TOP_AUD_SEL, "aud_sel",
	    aud_parents, 0x060, 0x064, 0x068, 8, 1, 15, 0x1C0, 25),
	MUX_GATE_CLR_SET_UPD(CK_TOP_A1SYS_SEL, "a1sys_sel",
	    a1sys_parents, 0x060, 0x064, 0x068, 16, 1, 23, 0x1C0, 26),
	MUX_GATE_CLR_SET_UPD(CK_TOP_AUD_L_SEL, "aud_l_sel",
	    aud_l_parents, 0x060, 0x064, 0x068, 24, 2, 31, 0x1C0, 27),
	/* CLK_CFG_7 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_A_TUNER_SEL, "a_tuner_sel",
	    a_tuner_parents, 0x070, 0x074, 0x078, 0, 2, 7, 0x1C0, 28),
	MUX_GATE_CLR_SET_UPD(CK_TOP_U2U3_SEL, "u2u3_sel",
	    u2u3_parents, 0x070, 0x074, 0x078, 8, 1, 15, 0x1C0, 29),
	MUX_GATE_CLR_SET_UPD(CK_TOP_U2U3_SYS_SEL, "u2u3_sys_sel",
	    u2u3_sys_parents, 0x070, 0x074, 0x078, 16, 1, 23, 0x1C0, 30),
	MUX_GATE_CLR_SET_UPD(CK_TOP_U2U3_XHCI_SEL, "u2u3_xhci_sel",
	    u2u3_sys_parents, 0x070, 0x074, 0x078, 24, 1, 31, 0x1C4, 0),
	/* CLK_CFG_8 */
	MUX_GATE_CLR_SET_UPD(CK_TOP_USB_FRMCNT_SEL, "usb_frmcnt_sel",
	    usb_frmcnt_parents, 0x080, 0x084, 0x088, 0, 1, 7, 0x1C4, 1),
};

static const char * const infra_uart0_parents[] __initconst = {
	"infra_ck_f26m",
	"infra_uart"
};

static const char * const infra_spi0_parents[] __initconst = {
	"infra_i2c",
	"infra_ispi0"
};

static const char * const infra_spi1_parents[] __initconst = {
	"infra_i2c",
	"infra_ispi1"
};

static const char * const infra_pwm1_parents[] __initconst = {
	"infra_pwm"
};

static const char * const infra_pwm_bsel_parents[] __initconst = {
	"infra_ck_f32k",
	"infra_ck_f26m",
	"infra_66m_mck",
	"infra_pwm"
};

static const char * const infra_pcie_parents[] __initconst = {
	"infra_ck_f32k",
	"infra_ck_f26m",
	"cb_cksq_40m",
	"infra_pcie"
};

static const struct mtk_mux infra_muxes[] = {
	/* MODULE_CLK_SEL_0 */
	MUX_GATE_CLR_SET_UPD(CK_INFRA_UART0_SEL, "infra_uart0_sel",
	    infra_uart0_parents, 0x0018, 0x0010, 0x0014, 0, 1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_UART1_SEL, "infra_uart1_sel",
	    infra_uart0_parents, 0x0018, 0x0010, 0x0014, 1, 1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_UART2_SEL, "infra_uart2_sel",
	    infra_uart0_parents, 0x0018, 0x0010, 0x0014, 2, 1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_SPI0_SEL, "infra_spi0_sel",
	    infra_spi0_parents, 0x0018, 0x0010, 0x0014, 4, 1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_SPI1_SEL, "infra_spi1_sel",
	    infra_spi1_parents, 0x0018, 0x0010, 0x0014, 5, 1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_SPI2_SEL, "infra_spi2_sel",
	    infra_spi0_parents, 0x0018, 0x0010, 0x0014, 6, 1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PWM1_SEL, "infra_pwm1_sel",
	    infra_pwm1_parents, 0x0018, 0x0010, 0x0014, 9, 1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PWM2_SEL, "infra_pwm2_sel",
	    infra_pwm1_parents, 0x0018, 0x0010, 0x0014, 11, 1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PWM3_SEL, "infra_pwm3_sel",
	    infra_pwm1_parents, 0x0018, 0x0010, 0x0014, 15, 1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PWM_BSEL, "infra_pwm_bsel",
	    infra_pwm_bsel_parents, 0x0018, 0x0010, 0x0014, 13, 2, -1, -1, -1),
	/* MODULE_CLK_SEL_1 */
	MUX_GATE_CLR_SET_UPD(CK_INFRA_PCIE_SEL, "infra_pcie_sel",
	    infra_pcie_parents, 0x0028, 0x0020, 0x0024, 0, 2, -1, -1, -1),
};

static struct mtk_composite top_aud_divs[] = {
	DIV_GATE(CK_TOP_AUD_I2S_M, "aud_i2s_m", "aud",
		0x0420, 0, 0x0420, 8, 8),
};

static const struct mtk_gate_regs infra0_cg_regs = {
	.set_ofs = 0x40,
	.clr_ofs = 0x44,
	.sta_ofs = 0x48,
};

static const struct mtk_gate_regs infra1_cg_regs = {
	.set_ofs = 0x50,
	.clr_ofs = 0x54,
	.sta_ofs = 0x58,
};

static const struct mtk_gate_regs infra2_cg_regs = {
	.set_ofs = 0x60,
	.clr_ofs = 0x64,
	.sta_ofs = 0x68,
};

#define GATE_INFRA0(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &infra0_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_setclr,	\
	}

#define GATE_INFRA1(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &infra1_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_setclr,	\
	}

#define GATE_INFRA2(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &infra2_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_setclr,	\
	}

static const struct mtk_gate infra_clks[] __initconst = {
	/* INFRA0 */
	GATE_INFRA0(CK_INFRA_GPT_STA, "infra_gpt_sta", "infra_66m_mck", 0),
	GATE_INFRA0(CK_INFRA_PWM_HCK, "infra_pwm_hck", "infra_66m_mck", 1),
	GATE_INFRA0(CK_INFRA_PWM_STA, "infra_pwm_sta", "infra_pwm_bck", 2),
	GATE_INFRA0(CK_INFRA_PWM1_CK, "infra_pwm1", "infra_pwm_ck1", 3),
	GATE_INFRA0(CK_INFRA_PWM2_CK, "infra_pwm2", "infra_pwm_ck2", 4),
	GATE_INFRA0(CK_INFRA_CQ_DMA_CK, "infra_cq_dma", "infra_133m_hck", 6),
	GATE_INFRA0(CK_INFRA_AUD_BUS_CK, "infra_aud_bus", "infra_66m_phck", 8),
	GATE_INFRA0(CK_INFRA_AUD_26M_CK, "infra_aud_26m", "infra_ck_f26m", 9),
	GATE_INFRA0(CK_INFRA_AUD_L_CK, "infra_aud_l", "infra_faud_l", 10),
	GATE_INFRA0(CK_INFRA_AUD_AUD_CK, "infra_aud_aud", "infra_faud_aud", 11),
	GATE_INFRA0(CK_INFRA_AUD_EG2_CK, "infra_aud_eg2", "infra_faud_eg2", 13),
	GATE_INFRA0(CK_INFRA_DRAMC_26M_CK, "infra_dramc_26m", "infra_ck_f26m", 14),
	GATE_INFRA0(CK_INFRA_DBG_CK, "infra_dbg", "infra_66m_mck", 15),
	GATE_INFRA0(CK_INFRA_AP_DMA_CK, "infra_ap_dma", "infra_66m_mck", 16),
	GATE_INFRA0(CK_INFRA_SEJ_CK, "infra_sej", "infra_66m_mck", 24),
	GATE_INFRA0(CK_INFRA_SEJ_13M_CK, "infra_sej_13m", "infra_ck_f26m", 25),
	GATE_INFRA0(CK_INFRA_PWM3_CK, "infra_pwm3", "infra_pwm3_sel", 27),
	/* INFRA1 */
	GATE_INFRA1(CK_INFRA_THERM_CK, "infra_therm", "infra_ck_f26m", 0),
	GATE_INFRA1(CK_INFRA_I2CO_CK, "infra_i2co", "infra_i2cs", 1),
	GATE_INFRA1(CK_INFRA_UART0_CK, "infra_uart0", "infra_mux_uart0", 2),
	GATE_INFRA1(CK_INFRA_UART1_CK, "infra_uart1", "infra_mux_uart1", 3),
	GATE_INFRA1(CK_INFRA_UART2_CK, "infra_uart2", "infra_mux_uart2", 4),
	GATE_INFRA1(CK_INFRA_SPI2_CK, "infra_spi2", "infra_mux_spi2", 6),
	GATE_INFRA1(CK_INFRA_SPI2_HCK_CK, "infra_spi2_hck", "infra_66m_mck", 7),
	GATE_INFRA1(CK_INFRA_NFI1_CK, "infra_nfi1", "infra_nfi", 8),
	GATE_INFRA1(CK_INFRA_SPINFI1_CK, "infra_spinfi1", "infra_spinfi", 9),
	GATE_INFRA1(CK_INFRA_NFI_HCK_CK, "infra_nfi_hck", "infra_66m_mck", 10),
	GATE_INFRA1(CK_INFRA_SPI0_CK, "infra_spi0", "infra_mux_spi0", 11),
	GATE_INFRA1(CK_INFRA_SPI1_CK, "infra_spi1", "infra_mux_spi1", 12),
	GATE_INFRA1(CK_INFRA_SPI0_HCK_CK, "infra_spi0_hck", "infra_66m_mck", 13),
	GATE_INFRA1(CK_INFRA_SPI1_HCK_CK, "infra_spi1_hck", "infra_66m_mck", 14),
	GATE_INFRA1(CK_INFRA_FRTC_CK, "infra_frtc", "infra_rtc_32k", 15),
	GATE_INFRA1(CK_INFRA_MSDC_CK, "infra_msdc", "infra_fmsdc", 16),
	GATE_INFRA1(CK_INFRA_MSDC_HCK_CK, "infra_msdc_hck", "infra_fmsdc_hck", 17),
	GATE_INFRA1(CK_INFRA_MSDC_133M_CK, "infra_msdc_133m", "infra_peri_133m", 18),
	GATE_INFRA1(CK_INFRA_MSDC_66M_CK, "infra_msdc_66m", "infra_66m_phck", 19),
	GATE_INFRA1(CK_INFRA_ADC_26M_CK, "infra_adc_26m", "csw_f26m", 20),
	GATE_INFRA1(CK_INFRA_ADC_FRC_CK, "infra_adc_frc", "csw_f26m", 21),
	GATE_INFRA1(CK_INFRA_FBIST2FPC_CK, "infra_fbist2fpc", "infra_nfi", 23),
	GATE_INFRA1(CK_INFRA_I2C_MCK_CK, "infra_i2c_mck", "infra_133m_mck", 25),
	GATE_INFRA1(CK_INFRA_I2C_PCK_CK, "infra_i2c_pck", "infra_66m_mck", 26),
	/* INFRA2 */
	GATE_INFRA2(CK_INFRA_IUSB_133_CK, "infra_iusb_133", "infra_133m_phck", 0),
	GATE_INFRA2(CK_INFRA_IUSB_66M_CK, "infra_iusb_66m", "infra_66m_phck", 1),
	GATE_INFRA2(CK_INFRA_IUSB_SYS_CK, "infra_iusb_sys", "infra_usb_sys", 2),
	GATE_INFRA2(CK_INFRA_IUSB_CK, "infra_iusb", "infra_usb", 3),
	GATE_INFRA2(CK_INFRA_IPCIE_CK, "infra_ipcie", "infra_pcie_mux", 12),
	GATE_INFRA2(CK_INFRA_IPCIE_PIPE_CK, "infra_ipcie_pipe", "cb_cksq_40m", 13),
	GATE_INFRA2(CK_INFRA_IPCIER_CK, "infra_ipcier", "infra_f26m_ck0", 14),
	GATE_INFRA2(CK_INFRA_IPCIEB_CK, "infra_ipcieb", "infra_133m_phck", 15),
};

static const struct mtk_gate_regs sgmii0_cg_regs = {
	.set_ofs = 0xE4,
	.clr_ofs = 0xE4,
	.sta_ofs = 0xE4,
};

#define GATE_SGMII0(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &sgmii0_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_no_setclr_inv,	\
	}

static const struct mtk_gate sgmii0_clks[] __initconst = {
	GATE_SGMII0(CK_SGM0_TX_EN, "sgm0_tx_en", "usb_tx250m", 2),
	GATE_SGMII0(CK_SGM0_RX_EN, "sgm0_rx_en", "usb_eq_rx250m", 3),
	GATE_SGMII0(CK_SGM0_CK0_EN, "sgm0_ck0_en", "usb_ln0", 4),
	GATE_SGMII0(CK_SGM0_CDR_CK0_EN, "sgm0_cdr_ck0_en", "usb_cdr", 5),
};

static const struct mtk_gate_regs sgmii1_cg_regs = {
	.set_ofs = 0xE4,
	.clr_ofs = 0xE4,
	.sta_ofs = 0xE4,
};

#define GATE_SGMII1(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &sgmii1_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_no_setclr_inv,	\
	}

static const struct mtk_gate sgmii1_clks[] __initconst = {
	GATE_SGMII1(CK_SGM1_TX_EN, "sgm1_tx_en", "usb_tx250m", 2),
	GATE_SGMII1(CK_SGM1_RX_EN, "sgm1_rx_en", "usb_eq_rx250m", 3),
	GATE_SGMII1(CK_SGM1_CK1_EN, "sgm1_ck1_en", "usb_ln0", 4),
	GATE_SGMII1(CK_SGM1_CDR_CK1_EN, "sgm1_cdr_ck1_en", "usb_cdr", 5),
};

static const struct mtk_gate_regs eth_cg_regs = {
	.set_ofs = 0x30,
	.clr_ofs = 0x30,
	.sta_ofs = 0x30,
};

#define GATE_ETH(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &eth_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_no_setclr_inv,	\
	}

static const struct mtk_gate eth_clks[] __initconst = {
	GATE_ETH(CK_ETH_FE_EN, "eth_fe_en", "netsys_2x", 6),
	GATE_ETH(CK_ETH_GP2_EN, "eth_gp2_en", "sgm_325m", 7),
	GATE_ETH(CK_ETH_GP1_EN, "eth_gp1_en", "sgm_325m", 8),
	GATE_ETH(CK_ETH_WOCPU0_EN, "eth_wocpu0_en", "netsys_wed_mcu", 15),
};

#define MT7981_PLL_FMAX		(2500UL * MHZ)

#define CON0_MT7981_RST_BAR	BIT(27)

#define PLL_B(_id, _name, _reg, _pwr_reg, _en_mask, _flags, _pcwbits,	\
			_pd_reg, _pd_shift, _tuner_reg, _pcw_reg,	\
			_pcw_shift, _div_table, _parent_name) {		\
		.id = _id,						\
		.name = _name,						\
		.reg = _reg,						\
		.pwr_reg = _pwr_reg,					\
		.en_mask = _en_mask,					\
		.flags = _flags,					\
		.rst_bar_mask = CON0_MT7981_RST_BAR,			\
		.fmax = MT7981_PLL_FMAX,				\
		.pcwbits = _pcwbits,					\
		.pd_reg = _pd_reg,					\
		.pd_shift = _pd_shift,					\
		.tuner_reg = _tuner_reg,				\
		.pcw_reg = _pcw_reg,					\
		.pcw_shift = _pcw_shift,				\
		.div_table = _div_table,				\
		.parent_name = _parent_name,				\
	}

#define PLL(_id, _name, _reg, _pwr_reg, _en_mask, _flags, _pcwbits,	\
			_pd_reg, _pd_shift, _tuner_reg, _pcw_reg,	\
			_pcw_shift, _parent_name)				\
		PLL_B(_id, _name, _reg, _pwr_reg, _en_mask, _flags, _pcwbits, \
			_pd_reg, _pd_shift, _tuner_reg, _pcw_reg, _pcw_shift, \
			NULL, _parent_name)

static const struct mtk_pll_data plls[] = {
	PLL(CK_APMIXED_ARMPLL, "armpll", 0x0200, 0x020C, 0x00000001,
	    0, 32, 0x0200, 4, 0, 0x0204, 0, "clkxtal"),
	PLL(CK_APMIXED_NET2PLL, "net2pll", 0x0210, 0x021C, 0x00000001,
	    0, 32, 0x0210, 4, 0, 0x0214, 0, "clkxtal"),
	PLL(CK_APMIXED_MMPLL, "mmpll", 0x0220, 0x022C, 0x00000001,
	    0, 32, 0x0220, 4, 0, 0x0224, 0, "clkxtal"),
	PLL(CK_APMIXED_SGMPLL, "sgmpll", 0x0230, 0x023C, 0x00000001,
	    0, 32, 0x0230, 4, 0, 0x0234, 0, "clkxtal"),
	PLL(CK_APMIXED_WEDMCUPLL, "wedmcupll", 0x0240, 0x024C, 0x00000001,
	    0, 32, 0x0240, 4, 0, 0x0244, 0, "clkxtal"),
	PLL(CK_APMIXED_NET1PLL, "net1pll", 0x0250, 0x025C, 0x00000001,
	    0, 32, 0x0250, 4, 0, 0x0254, 0, "clkxtal"),
	PLL(CK_APMIXED_MPLL, "mpll", 0x0260, 0x0270, 0x00000001,
	    0, 32, 0x0260, 4, 0, 0x0264, 0, "clkxtal"),
	PLL(CK_APMIXED_APLL2, "apll2", 0x0278, 0x0288, 0x00000001,
	    0, 32, 0x0278, 4, 0, 0x027C, 0, "clkxtal"),
};

static struct clk_onecell_data *mt7981_top_clk_data __initdata;
static struct clk_onecell_data *mt7981_pll_clk_data __initdata;

static void __init mtk_clk_enable_critical(void)
{
	if (!mt7981_top_clk_data || !mt7981_pll_clk_data)
		return;

	clk_prepare_enable(mt7981_pll_clk_data->clks[CK_APMIXED_ARMPLL]);
	clk_prepare_enable(mt7981_top_clk_data->clks[CK_TOP_SYSAXI_SEL]);
	clk_prepare_enable(mt7981_top_clk_data->clks[CK_TOP_SYSAPB_SEL]);
	clk_prepare_enable(mt7981_top_clk_data->clks[CK_TOP_DRAMC_SEL]);
	clk_prepare_enable(mt7981_top_clk_data->clks[CK_TOP_DRAMC_MD32_SEL]);
	clk_prepare_enable(mt7981_top_clk_data->clks[CK_TOP_F26M_SEL]);
}

static void __init mtk_infracfg_init(struct device_node *node)
{
	int r;


	mt7981_top_clk_data = mtk_alloc_clk_data(CLK_INFRA_NR_CLK);

	mtk_clk_register_factors(infra_divs, ARRAY_SIZE(infra_divs), mt7981_top_clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, mt7981_top_clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
			__func__, r);

	mtk_clk_enable_critical();
}
CLK_OF_DECLARE(mtk_infracfg, "mediatek,mt7981-infracfg", mtk_infracfg_init);

static void __init mtk_topckgen_init(struct device_node *node)
{
	int r;
	void __iomem *base;

	base = of_iomap(node, 0);
	if (!base) {
		pr_err("%s(): ioremap failed\n", __func__);
		return;
	}

	mt7981_top_clk_data = mtk_alloc_clk_data(CLK_TOP_NR_CLK);

	mtk_clk_register_factors(top_divs, ARRAY_SIZE(top_divs), mt7981_top_clk_data);
	mtk_clk_register_muxes(top_muxes, ARRAY_SIZE(top_muxes), node, &mt7981_clk_lock, mt7981_top_clk_data);
	mtk_clk_register_composites(top_aud_divs, ARRAY_SIZE(top_aud_divs),
		base, &mt7981_clk_lock, mt7981_top_clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, mt7981_top_clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
			__func__, r);

	mtk_clk_enable_critical();
}
CLK_OF_DECLARE(mtk_topckgen, "mediatek,mt7981-topckgen", mtk_topckgen_init);

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

	mtk_clk_register_muxes(infra_muxes, ARRAY_SIZE(infra_muxes), node, &mt7981_clk_lock, clk_data);
	mtk_clk_register_gates(node, infra_clks, ARRAY_SIZE(infra_clks), clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
			__func__, r);
}
CLK_OF_DECLARE(mtk_infracfg_ao, "mediatek,mt7981-infracfg_ao", mtk_infracfg_ao_init);

static void __init mtk_apmixedsys_init(struct device_node *node)
{
	int r;

	mt7981_pll_clk_data = mtk_alloc_clk_data(CLK_APMIXED_NR_CLK);

	mtk_clk_register_plls(node, plls, ARRAY_SIZE(plls), mt7981_pll_clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, mt7981_pll_clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
			__func__, r);

	mtk_clk_enable_critical();
}
CLK_OF_DECLARE(mtk_apmixedsys, "mediatek,mt7981-apmixedsys", mtk_apmixedsys_init);

static void __init mtk_sgmiisys_0_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	int r;

	clk_data = mtk_alloc_clk_data(CLK_SGMII0_NR_CLK);

	mtk_clk_register_gates(node, sgmii0_clks, ARRAY_SIZE(sgmii0_clks), clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
			__func__, r);
}
CLK_OF_DECLARE(mtk_sgmiisys_0, "mediatek,mt7981-sgmiisys_0", mtk_sgmiisys_0_init);

static void __init mtk_sgmiisys_1_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	int r;

	clk_data = mtk_alloc_clk_data(CLK_SGMII1_NR_CLK);

	mtk_clk_register_gates(node, sgmii1_clks, ARRAY_SIZE(sgmii1_clks), clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
			__func__, r);
}
CLK_OF_DECLARE(mtk_sgmiisys_1, "mediatek,mt7981-sgmiisys_1", mtk_sgmiisys_1_init);

static void __init mtk_ethsys_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	int r;

	clk_data = mtk_alloc_clk_data(CLK_ETH_NR_CLK);

	mtk_clk_register_gates(node, eth_clks, ARRAY_SIZE(eth_clks), clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
			__func__, r);
}
CLK_OF_DECLARE(mtk_ethsys, "mediatek,mt7981-ethsys", mtk_ethsys_init);

