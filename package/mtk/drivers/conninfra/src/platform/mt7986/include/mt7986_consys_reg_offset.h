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
/*! \file
*    \brief  Declaration of library functions
*
*    Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/

#ifndef _PLATFORM_MT7986_CONSYS_REG_OFFSET_H_
#define _PLATFORM_MT7986_CONSYS_REG_OFFSET_H_

/**********************************************************************/
/* Base: TOP_MISC (0x11D1_0000) */
/**********************************************************************/
#define CONNSYS_MISC			0x0114
#define TOP_MISC_RSRV_ALL1_3 	0x021C


/**********************************************************************/
/* Base: TOP RGU (0x1001_C000) */
/**********************************************************************/
#define TOP_RGU_WDT_SWSYSRST	0x0018

/**********************************************************************/
/* Base: GPIO (0x1001_F000) */
/**********************************************************************/
#define GPIO_MODE9				0x0390
#define GPIO_MODE10				0x03A0
#define GPIO_MODE11				0x03B0
#define GPIO_MODE12				0x03C0

/**********************************************************************/
/* Base: IOCFG_TR (0x11F0_0000) */
/**********************************************************************/
#define IOCFG_TR_DRV_CFG0		0x0000
#define IOCFG_TR_DRV_CFG1		0x0010

/**********************************************************************/
/* Base: IOCFG_TL (0x11F1_0000) */
/**********************************************************************/
#define IOCFG_TL_DRV_CFG0		0x0000
#define IOCFG_TL_DRV_CFG1		0x0010

/**********************************************************************/
/* Base: INFRACFG_AO (0x1000_3000) */
/**********************************************************************/
#define CONN2AP_GALS_SLPPROT	0x00D0
#define AP2CONN_GALS_SLPPROT	0x00D4

/**********************************************************************/
/* Base: CONN_INFRA_CFG (0x1800_1000) */
/**********************************************************************/
#define CONN_INFRA_CFG_IP_VERSION	0x0000
#define EFUSE 						0x0020
#define ADIE_CTL					0x0030
#define CONN_INFRA_CFG_PWRCTRL0		0x0200
#define CONN_INFRA_CFG_RC_CTL_0		0x0380
#define OSC_CTL_0					0x0300
#define EMI_CTL_WF					0x0414
#define CONN_INFRA_WF_SLP_CTRL		0x0540
#define CONN_INFRA_WF_SLP_STATUS	0x0544

/**********************************************************************/
/* Base: CONN_INFRA_SYSRAM (0x1805_0000) */
/**********************************************************************/
#define SYSRAM_BASE_ADDR		0x0000

/**********************************************************************/
/* Base: CONN_INFRA_CLKGEN_ON_TOP (0x1800_9000) */
/**********************************************************************/
#define CKGEN_BUS_WPLL_DIV_1	0x0008
#define CKGEN_BUS_WPLL_DIV_2	0x000C
#define CKGEN_RFSPI_WPLL_DIV	0x0040
#define CKGEN_BUS				0x0A00

/**********************************************************************/
/* Base: CONN_HOST_CSR_TOP (0x1806_0000) */
/**********************************************************************/
#define CONN_INFRA_WAKEPU_TOP		0x01A0
#define CONN_INFRA_WAKEPU_WF		0x01A4
#define CONN2AP_REMAP_MCU_EMI 		0x01C4
#define CONN2AP_REMAP_WF_PERI 		0x01D4
#define DBG_DUMMY_3 				0x02CC

/**********************************************************************/
/* Base: CONN_INFRA_BUS_CR (0x1800_E000) */
/**********************************************************************/
#define CONN_INFRA_BUS_OFF_TIMEOUT_CTRL		0x0300
#define CONN_INFRA_BUS_ON_TIMEOUT_CTRL		0x031C
#define CONN2AP_EMI_PATH_ADDR_START 		0x0360
#define CONN2AP_EMI_PATH_ADDR_END 			0x0364

/**********************************************************************/
/* Base: CONN_INFRA_RGU (0x1800_0000) */
/**********************************************************************/
#define WFSYS_ON_TOP_PWR_CTL	0x0010
#define BGFYS_ON_TOP_PWR_CTL	0x0020
#define SYSRAM_HWCTL_PDN 		0x0050
#define SYSRAM_HWCTL_SLP 		0x0054
#define WFSYS_CPU_SW_RST_B		0x0120

/**********************************************************************/
/* Base: CONN_WT_SLP_CTL_REG 		(0x1800_5000) */
/* Base: INST2_CONN_WT_SLP_CTL_REG  (0x1808_5000) */
/**********************************************************************/
#define WB_WF_CK_ADDR		0x0070
#define WB_WF_WAKE_ADDR		0x0074
#define WB_WF_ZPS_ADDR		0x0078
#define WB_TOP_CK_ADDR		0x0084
#define WB_WF_B0_CMD_ADDR	0x008C
#define WB_WF_B1_CMD_ADDR	0x0090
#define WB_SLP_TOP_CK_0		0x0120
#define WB_SLP_TOP_CK_1		0x0124

/**********************************************************************/
/* Base: CONN_RF_SPI_MST_REG (0x1800_4000) */
/* Base: INST2_CONN_RF_SPI_MST_REG (0x1808_4000) */
/**********************************************************************/
#define SPI_STA			0x0000
#define SPI_WF_ADDR		0x0010
#define SPI_WF_WDAT		0x0014
#define SPI_WF_RDAT		0x0018
#define SPI_BT_ADDR		0x0020
#define SPI_BT_WDAT		0x0024
#define SPI_BT_RDAT		0x0028
#define SPI_FM_ADDR		0x0030
#define SPI_FM_WDAT		0x0034
#define SPI_FM_RDAT		0x0038
#define SPI_GPS_ADDR	0x0040
#define SPI_GPS_WDAT	0x0044
#define SPI_GPS_RDAT	0x0048
#define SPI_TOP_ADDR	0x0050
#define SPI_TOP_WDAT	0x0054
#define SPI_TOP_RDAT	0x0058

/**********************************************************************/
/* Base: CONN_SEMAPHORE_BASE (0x1807_0000) */
/**********************************************************************/
#define CONN_SEMA00_M2_OWN_STA			0x2000
#define CONN_SEMA00_M2_OWN_REL			0x2200
#define CONN_SEMA_OWN_BY_M0_STA_REP 	0x0400
#define CONN_SEMA_OWN_BY_M1_STA_REP 	0x1400
#define CONN_SEMA_OWN_BY_M2_STA_REP		0x2400
#define CONN_SEMA_OWN_BY_M3_STA_REP 	0x3400
#define CONN_SEMA_OWN_BY_M4_STA_REP 	0x4400
#define CONN_SEMA_OWN_BY_M5_STA_REP		0x5400
#define CONN_SEMA_OWN_BY_M6_STA_REP		0x6400
#define CONN_SEMA_OWN_BY_M7_STA_REP		0x7400

/**********************************************************************/
/* Base: CONN_AFE_CTL_BASE 		(0x1800_3000) */
/* Base: CONN_AFE_CTL_2ND_BASE 	(0x1808_3000) */
/**********************************************************************/
#define RG_DIG_EN_01			0x0000
#define RG_DIG_EN_02			0x0004
#define RG_DIG_EN_03			0x0008
#define RG_DIG_TOP_01			0x000C
#define RG_PLL_STB_TIME			0x00F4

/**********************************************************************/
/* Base: WF_TOP_SLPPROT_ON_BASE (0x8102_0000 remap to 0x184C_0000) */
/**********************************************************************/
#define WF_TOP_SLPPROT_ON_STATUS_READ 0x300C

/**********************************************************************/
/* Base: WF_TOP_CFG_BASE (0x8002_0000 remap to 0x184B_0000) */
/**********************************************************************/
#define WF_TOP_CFG_IP_VERSION 		  0x0010

/**********************************************************************/
/* Base: WF_MCU_CONFIG_LS_BASE (0x8800_0000 remap to 0x184F_0000) */
/**********************************************************************/
#define BUSHANGCR  0x0440

/**********************************************************************/
/* Base: WF_MCU_BUS_CR_BASE (0x830C_0XXX remap to 0x1840_0XXX) */
/**********************************************************************/
#define AP2WF_REMAP_1  0x0120

/**********************************************************************/
/* Base: WF_MCUSYS_INFRA_BUS_FULL_U_DEBUG_CTRL_AO_BASE (0x810F_0000 remap to 0x1850_0000) */
/**********************************************************************/
#define WF_MCUSYS_INFRA_BUS_FULL_U_DEBUG_CTRL_AO_WFMCU_PWA_CTRL0  0x0000

/**********************************************************************/
/* Base: WF_TOP_CFG_ON_BASE (0x8102_1000 remap to 0x184C_0000) */
/**********************************************************************/
#define ROMCODE_INDEX  0x1604

/**********************************************************************/
/* A-die CR */
/**********************************************************************/
#define ATOP_CHIP_ID								0x02C
#define ATOP_TOP_CLK_EN								0xA00	
#define ATOP_RG_ENCAL_WBTAC_IF_SW					0x070
#define ATOP_RG_WRI_CK_SELECT						0x4AC
#define ATOP_EFUSE_CTRL_1							0x108
#define ATOP_EFUSE_CTRL_2							0x148
#define ATOP_EFUSE_CTRL_3							0x14C
#define ATOP_EFUSE_CTRL_4							0x15C
#define ATOP_EFUSE_RDATA0							0x130
#define ATOP_EFUSE_RDATA1							0x134
#define ATOP_EFUSE_RDATA2							0x138
#define ATOP_EFUSE_RDATA3							0x13C
#define ATOP_RG_EFUSE_CFG5							0x144
#define ATOP_THADC_ANALOG							0x3A6
#define ATOP_THADC_SLOP								0x3A7
#define ATOP_RG_TOP_THADC_BG						0x034
#define ATOP_RG_TOP_THADC_00						0x038

#define ATOP_XTAL_TRIM_FLOW							0x3AC
#define ATOP_XTAL_CR_C1_SEL_AXM_80M_OSC				0x390
#define ATOP_XTAL_CR_C1_SEL_AXM_40M_OSC				0x391
#define ATOP_XTAL_CR_C1_SEL_AXM_TRIM1_80M_OSC		0x398
#define ATOP_XTAL_CR_C1_SEL_AXM_TRIM1_40M_OSC		0x399
#define ATOP_RG_STRAP_PIN_IN						0x4FC
#define ATOP_RG_XO_01								0x65C
#define ATOP_RG_XO_03								0x664


#define ATOP_7975_XTAL_CALIBRATION					0x3A1
#define ATOP_7975_XTAL_TRIM2_COMPENSATION			0x3A2
#define ATOP_7975_XTAL_TRIM3_COMPENSATION			0x3A3
#define ATOP_7975_XTAL_TRIM4_COMPENSATION			0x3A4
#define ATOP_7975_XTAL_TRIM_FLOW					0x3A5
#define ATOP_7975_CR_C1_C2_A94						0xA94
#define ATOP_7975_CR_C1_C2_A18						0xA18
#define ATOP_7975_CR_C1_C2_A84						0xA84
#define ATOP_7975_CR_C1_C2_AA4						0xAA4
#define ATOP_7975_CO_CLK							0xA1C


#endif				/* _PLATFORM_MT7986_CONSYS_REG_OFFSET_H_ */
