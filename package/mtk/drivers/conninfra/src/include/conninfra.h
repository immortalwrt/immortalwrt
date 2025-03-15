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

#ifndef _CONNINFRA_H_
#define _CONNINFRA_H_


/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#define AIDE_NUM_MAX 2
/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
enum consys_drv_type {
	CONNDRV_TYPE_BT = 0,
	CONNDRV_TYPE_FM = 1,
	CONNDRV_TYPE_GPS = 2,
	CONNDRV_TYPE_WIFI = 3,
	CONNDRV_TYPE_CONNINFRA = 4,
	CONNDRV_TYPE_MAX
};

/* HW-specific, need sync with FW.  DO NOT MODIFY */
enum sys_spi_subsystem
{
	SYS_SPI_WF1 = 0x00,
	SYS_SPI_WF  = 0x01,
	SYS_SPI_BT  = 0x02,
	SYS_SPI_FM  = 0x03,
	SYS_SPI_GPS = 0x04,
	SYS_SPI_TOP = 0x05,
	SYS_SPI_WF2 = 0x06,
	SYS_SPI_WF3 = 0x07,
	SYS_SPI_2ND_ADIE_WF1 = 0x10,
	SYS_SPI_2ND_ADIE_WF  = 0x11,
	SYS_SPI_2ND_ADIE_BT  = 0x12,
	SYS_SPI_2ND_ADIE_FM  = 0x13,
	SYS_SPI_2ND_ADIE_GPS = 0x14,
	SYS_SPI_2ND_ADIE_TOP = 0x15,
	SYS_SPI_2ND_ADIE_WF2 = 0x16,
	SYS_SPI_2ND_ADIE_WF3 = 0x17,
	SYS_SPI_MAX
};

enum connsys_spi_speed_type {
	CONNSYS_SPI_SPEED_26M,
	CONNSYS_SPI_SPEED_64M,
	CONNSYS_SPI_SPEED_MAX
};

/* Conninfra driver allocate EMI for FW and WFDAM
 * (FW includes: BT, WIFI and their MCU)
 * +-----------+  +
 * |           |  |
 * |    FW     |  |
 * |           |  |
 * +-----------+  v
 * |           |
 * |           | FW_WFDMA
 * |           |  ^
 * |   WFDMA   |  |
 * |           |  |
 * |           |  |
 * +-----------+  +
 *
 * MCIF region is provided by MD
 * +-----------+
 * |           |
 * |           |
 * |   MCIF    |
 * |           |
 * +-----------+
 */
enum connsys_emi_type
{
	CONNSYS_EMI_FW = 0,
	CONNSYS_EMI_MAX,
};

#define CONNINFRA_SPI_OP_FAIL	0x1

#define CONNINFRA_CB_RET_CAL_PASS_POWER_OFF 0x0
#define CONNINFRA_CB_RET_CAL_PASS_POWER_ON  0x2
#define CONNINFRA_CB_RET_CAL_FAIL_POWER_OFF 0x1
#define CONNINFRA_CB_RET_CAL_FAIL_POWER_ON  0x3

#define CONNINFRA_BUS_CLOCK_WPLL	0x1
#define CONNINFRA_BUS_CLOCK_ALL		(CONNINFRA_BUS_CLOCK_WPLL)

/* bus hang error define */
#define CONNINFRA_INFRA_BUS_HANG			0x1
#define CONNINFRA_AP2CONN_RX_SLP_PROT_ERR	0x2
#define CONNINFRA_AP2CONN_TX_SLP_PROT_ERR	0x4
#define CONNINFRA_AP2CONN_CLK_ERR			0x8
#define CONNINFRA_INFRA_BUS_HANG_IRQ		0x10

#define CONNINFRA_ERR_RST_ONGOING			-0x7788
#define CONNINFRA_ERR_WAKEUP_FAIL			-0x5566
/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/* SPI clock switch */
int conninfra_spi_clock_switch(enum connsys_spi_speed_type type);

/* A-die top_ck_en control, only for MT6885 */
int conninfra_adie_top_ck_en_on(enum consys_drv_type type);
int conninfra_adie_top_ck_en_off(enum consys_drv_type type);

/* RFSPI */
int conninfra_spi_read(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int *data);
int conninfra_spi_write(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int data);

/* EMI */
void conninfra_get_emi_phy_addr(enum connsys_emi_type type, phys_addr_t* base, unsigned int *size);

/* power on/off */
int conninfra_pwr_on(enum consys_drv_type drv_type);
int conninfra_pwr_off(enum consys_drv_type drv_type);

/* To setup config relative data, ex: debug flag */
void conninfra_config_setup(void);

/*
 * 0 : NO hang
 * > 0 : HANG!!
 * CONNINFRA_ERR_RST_ONGOING: whole chip reset is ongoing
 */
int conninfra_is_bus_hang(void);

/* chip reset
* return:
*    <0: error
*    =0: triggered
*    =1: ongoing
*/
int conninfra_trigger_whole_chip_rst(enum consys_drv_type drv, char *reason);

int conninfra_debug_dump(void);

struct whole_chip_rst_cb {
	int (*pre_whole_chip_rst)(enum consys_drv_type drv, char *reason);
	int (*post_whole_chip_rst)(void);
};

/* driver state query */

/* VCN control */

/* Thermal */

/* Config */

/* semaphore */

/* calibration */

struct sub_drv_ops_cb {
	/* chip reset */
	struct whole_chip_rst_cb rst_cb;
};

int conninfra_sub_drv_ops_register(enum consys_drv_type drv_type, struct sub_drv_ops_cb *cb);
int conninfra_sub_drv_ops_unregister(enum consys_drv_type drv_type);

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif /* _CONNINFRA_H_ */
