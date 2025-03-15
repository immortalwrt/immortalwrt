// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#define pr_fmt(fmt) KBUILD_MODNAME "@(%s:%d) " fmt, __func__, __LINE__

#include <linux/ratelimit.h>
#include "plat_def.h"
#include "consys_reg_util.h"
#include "consys_reg_mng.h"
#include "mt7981_consys_reg.h"
#include "mt7981_consys_reg_offset.h"
#include "mt7981_pos.h"
#include "mt7981.h"
#include "mt7981_emi.h"



/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#define MTD_WIFI_NM						"Factory"
#define EEPROM_CHIPID_OFFSET   			0x0

#define EEPROM_BAND0_STREAM_OFFSET   		0x190
#define EEPROM_BAND0_STREAM_TX_MASK  		0x7
#define EEPROM_BAND0_STREAM_TX_BIT_OFFSET 	0
#define EEPROM_BAND0_STREAM_RX_MASK  		0x7
#define EEPROM_BAND0_STREAM_RX_BIT_OFFSET 	3

#define _TO_STR(_x) #_x
#define TO_STR(_x) _TO_STR(_x)
#define RED(_text)  "\033[1;31m"_text"\033[0m"
#define GRN(_text)  "\033[1;32m"_text"\033[0m"

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
bool one_adie_dbdc = false;
bool adie_7976 = false;
unsigned int adie_cfg_type = ADIE_TYPE_NONE;

struct spi_op {
	unsigned int busy_cr;
	unsigned int polling_bit;
	unsigned int addr_cr;
	unsigned int read_addr_format;
	unsigned int write_addr_format;
	unsigned int write_data_cr;
	unsigned int read_data_cr;
	unsigned int read_data_mask;
};

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
const static char* g_spi_system_name[SYS_SPI_MAX] = {
	"SYS_SPI_WF1",
	"SYS_SPI_WF",
	"SYS_SPI_BT",
	"SYS_SPI_FM",
	"SYS_SPI_GPS",
	"SYS_SPI_TOP",
	"SYS_SPI_WF2",
	"SYS_SPI_WF3",
};

static const struct spi_op spi_op_array[SYS_SPI_MAX] = {
	/* SYS_SPI_WF1 */
	{
		SPI_STA, 1, SPI_WF_ADDR, 0x00001000, 0x00000000,
		SPI_WF_WDAT, SPI_WF_RDAT, 0xFFFFFFFF
	},
	/* SYS_SPI_WF */
	{
		SPI_STA, 1, SPI_WF_ADDR, 0x00003000, 0x00002000,
		SPI_WF_WDAT, SPI_WF_RDAT, 0xFFFFFFFF
	},
	/* SYS_SPI_BT */
	{
		SPI_STA, 2, SPI_BT_ADDR, 0x00005000, 0x00004000,
		SPI_BT_WDAT, SPI_BT_RDAT, 0xFFFFFFFF
	},
	/* SYS_SPI_FM */
	{
		SPI_STA, 3, SPI_FM_ADDR, 0x00007000, 0x00006000,
		SPI_FM_WDAT, SPI_FM_RDAT, 0x0000FFFF
	},
	/* SYS_SPI_GPS */
	{
		SPI_STA, 4, SPI_GPS_ADDR, 0x00009000, 0x00008000,
		SPI_GPS_WDAT, SPI_GPS_RDAT, 0x0000FFFF
	},
	/* SYS_SPI_TOP */
	{
		SPI_STA, 5, SPI_TOP_ADDR, 0x0000B000, 0x0000A000,
		SPI_TOP_WDAT, SPI_TOP_RDAT, 0xFFFFFFFF
	},
	/* SYS_SPI_WF2 */
	{
		SPI_STA, 1, SPI_WF_ADDR, 0x0000D000, 0x0000C000,
		SPI_WF_WDAT, SPI_WF_RDAT, 0xFFFFFFFF
	},
	/* SYS_SPI_WF3 */
	{
		SPI_STA, 1, SPI_WF_ADDR, 0x0000F000, 0x0000E000,
		SPI_WF_WDAT, SPI_WF_RDAT, 0xFFFFFFFF
	},
};

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
bool _is_flash_content_valid(void)
{
	unsigned short eeFlashId = 0;

	FlashRead(MTD_WIFI_NM, (unsigned char*)&eeFlashId, EEPROM_CHIPID_OFFSET, sizeof(eeFlashId));

	if (eeFlashId == consys_soc_chipid_get())
		return true;
	else
		return false;
}

int _consys_check_adie_cfg(void)
{
	int ret = 0;
	unsigned int hw_adie_type = 0;
	unsigned int i = 0;
	bool found = false;

	for (i = 0; i < AIDE_NUM_MAX; i++) {
		if (conn_hw_env[i].valid) {
			hw_adie_type = conn_hw_env[i].adie_id;
			found = true;
			break;
		}
	}

	if (found) {
		printk(GRN("Adie Type: 0x%x"), hw_adie_type);
	} else {
		printk(RED("No Adie found!!!"));
		ret = -1;
	}

	return ret;
}

int consys_plt_hw_init(void)
{
	/* Cheetah only has a-die 7976 and one-adie-dbdc */
	adie_7976 = true;
	one_adie_dbdc = true;
	adie_cfg_type = ADIE_TYPE_ONE;
	pr_info("adie_cfg_type = %d, one_adie_dbdc = %d\n", adie_cfg_type, one_adie_dbdc);
	return 0;
}

int consys_xtal_ctrl_fast_mode(void)
{
	/* Setting fast mode to xtal control */
	CONSYS_SET_BIT(REG_TOP_MISC_ADDR + CONNSYS_MISC, (0x1 << 3));
	return 0;
}

int consys_sw_reset_ctrl(bool bassert)
{
	/* Release CONNSYS software reset */
	if (bassert) {
		CONSYS_REG_WRITE_MASK(
			REG_TOP_RGU_ADDR + TOP_RGU_WDT_SWSYSRST,
			0x88800000, 0xff800000);
	} else {
		/*  de-assert CONNSYS S/W reset */
		CONSYS_REG_WRITE_MASK(
			REG_TOP_RGU_ADDR + TOP_RGU_WDT_SWSYSRST,
			0x88000000, 0xff800000);
	}

	return 0;
}

void consys_set_if_pinmux(bool enable)
{
	if (enable) {
		/*  One_Adie_DB
			set pinmux for the interface between D-die and A-die (Aux1)
			PAD_WF0_TOP_CLK(GPIO43) 0x0350[14:12]
			PAD_WF0_TOP_DATA(GPIO44) 0x0350[18:16]
			PAD_WF0_HB1(GPIO45) 0x0350[22:20]
			PAD_WF0_HB2(GPIO46) 0x0350[26:24]
			PAD_WF0_HB3(GPIO47) 0x0350[30:28]
			PAD_WF0_HB4(GPIO48) 0x0360[2:0]
			PAD_WF0_HB0(GPIO49) 0x0360[6:4]
			PAD_WF0_HB0_B(GPIO50) 0x0360[10:8]
			PAD_WF0_HB5(GPIO51) 0x0360[14:12]
			PAD_WF0_HB6(GPIO52) 0x0360[18:16]
			PAD_WF0_HB7(GPIO53) 0x0360[22:20]
			PAD_WF0_HB8(GPIO54) 0x0360[26:24]
			PAD_WF0_HB9(GPIO55) 0x0360[30:28]
			PAD_WF0_HB10(GPIO56) 0x0370[2:0]
		*/
		CONSYS_REG_WRITE_MASK(REG_GPIO_BASE_ADDR + GPIO_MODE5, 0x11111000, 0x77777000);
		CONSYS_REG_WRITE_MASK(REG_GPIO_BASE_ADDR + GPIO_MODE6, 0x11111111, 0x77777777);
		CONSYS_REG_WRITE_MASK(REG_GPIO_BASE_ADDR + GPIO_MODE7, 0x00000001, 0x00000007);

		/* Set pinmux driving to 4mA */
		CONSYS_REG_WRITE_MASK(REG_IOCFG_TM_ADDR + IOCFG_TM_DRV_CFG0, 0x49249, 0x1FFFFF);
		CONSYS_REG_WRITE_MASK(REG_IOCFG_LT_ADDR + IOCFG_LT_DRV_CFG0, 0x1249240, 0x7FFFFC0);
	}
}

int consys_tx_rx_bus_slp_prot_ctrl(bool enable)
{
	int check;

	if (enable) {
		/* conn2ap/ap2conn slpprot disable */
		/* Turn off AP2CONN AHB RX bus sleep protect */
		CONSYS_REG_WRITE_MASK(REG_INFRACFG_AO_ADDR + AP2CONN_GALS_SLPPROT, 0x0, 0x10000);
		CONSYS_REG_BIT_POLLING(REG_INFRACFG_AO_ADDR + AP2CONN_GALS_SLPPROT,
								24, 0x0, 100, 500, check);
		if (check != 0)
			pr_err("Polling AP2CONN AHB RX bus sleep protect turn off fail! CR Value = 0x%08x\n",
					CONSYS_REG_READ(REG_INFRACFG_AO_ADDR + AP2CONN_GALS_SLPPROT));

		/* Turn off AP2CONN AHB TX bus sleep protect */
		CONSYS_REG_WRITE_MASK(REG_INFRACFG_AO_ADDR + AP2CONN_GALS_SLPPROT, 0x0, 0x1);
		CONSYS_REG_BIT_POLLING(REG_INFRACFG_AO_ADDR + AP2CONN_GALS_SLPPROT,
								4, 0x0, 100, 500, check);
		if (check != 0)
			pr_err("Polling AP2CONN AHB TX bus sleep protect turn off fail! CR Value = 0x%08x\n",
					CONSYS_REG_READ(REG_INFRACFG_AO_ADDR + AP2CONN_GALS_SLPPROT));

		/* Turn off CONN2AP AXI RX bus sleep protect */
		CONSYS_REG_WRITE_MASK(REG_INFRACFG_AO_ADDR + CONN2AP_GALS_SLPPROT, 0x0, 0x10000);
		/* Turn off CONN2AP AXI TX bus sleep protect */
		CONSYS_REG_WRITE_MASK(REG_INFRACFG_AO_ADDR + CONN2AP_GALS_SLPPROT, 0x0, 0x1);

		/* Wait 900us (apply this for CONNSYS XO clock ready) */
		udelay(900);
	} else {
		/* Turn on AP2CONN AHB TX bus sleep protect */
		CONSYS_REG_WRITE_MASK(REG_INFRACFG_AO_ADDR + AP2CONN_GALS_SLPPROT, 0x1, 0x1);
		CONSYS_REG_BIT_POLLING(REG_INFRACFG_AO_ADDR + AP2CONN_GALS_SLPPROT,
							4, 0x1, 100, 500, check);
		if (check != 1)
			pr_err("Polling AP2CONN AHB TX bus sleep protect turn on fail! CR Value = 0x%08x\n",
					CONSYS_REG_READ(REG_INFRACFG_AO_ADDR + AP2CONN_GALS_SLPPROT));

		/* Turn on AP2CONN AHB RX bus sleep protec */
		CONSYS_REG_WRITE_MASK(REG_INFRACFG_AO_ADDR + AP2CONN_GALS_SLPPROT, 0x1, 0x10000);
		CONSYS_REG_BIT_POLLING(REG_INFRACFG_AO_ADDR + AP2CONN_GALS_SLPPROT,
								24, 0x1, 100, 500, check);
		if (check !=1)
			pr_err("Polling AP2CONN AHB RX bus sleep protect turn on fail! CR Value = 0x%08x\n",
					CONSYS_REG_READ(REG_INFRACFG_AO_ADDR + AP2CONN_GALS_SLPPROT));

		/* Turn on CONN2AP AXI TX bus sleep protect */
		CONSYS_REG_WRITE_MASK(REG_INFRACFG_AO_ADDR + CONN2AP_GALS_SLPPROT, 0x1, 0x1);
		CONSYS_REG_BIT_POLLING(REG_INFRACFG_AO_ADDR + CONN2AP_GALS_SLPPROT,
								4, 0x1, 100, 500, check);
		if (check != 1)
			pr_err("Polling CONN2AP AXI TX bus sleep protect turn on fail! CR Value = 0x%08x\n",
					CONSYS_REG_READ(REG_INFRACFG_AO_ADDR + CONN2AP_GALS_SLPPROT));

		/* Turn on CONN2AP AXI RX bus sleep protect */
		CONSYS_REG_WRITE_MASK(REG_INFRACFG_AO_ADDR + CONN2AP_GALS_SLPPROT, 0x1, 0x10000);
		CONSYS_REG_BIT_POLLING(REG_INFRACFG_AO_ADDR + CONN2AP_GALS_SLPPROT,
								24, 0x1, 100, 500, check);
		if (check != 1)
			pr_err("Polling CONN2AP AXI RX bus sleep protect turn on fail! CR Value = 0x%08x\n",
					CONSYS_REG_READ(REG_INFRACFG_AO_ADDR + CONN2AP_GALS_SLPPROT));

		/* wait 1us*/
		udelay(1);
	}

	return 0;
}

int _consys_polling_chipid_int(unsigned int retry, unsigned int sleep_ms)
{
	unsigned int count = retry + 1;
	unsigned int consys_hw_ver = consys_get_hw_ver();
	unsigned int hw_ver = 0;

	while (--count > 0) {
		hw_ver = CONSYS_REG_READ(REG_CONN_INFRA_CFG_ADDR + CONN_INFRA_CFG_IP_VERSION);
		if ((hw_ver >= consys_hw_ver) && (hw_ver != 0xdeadfeed))
			break;
		msleep(sleep_ms);
	}

	if (count == 0) {
		pr_err("Read CONNSYS HW IP version fail. Expect 0x%x but get 0x%x\n", consys_hw_ver, hw_ver);
		return -1;
	} else {
		pr_info("Read CONNSYS HW IP version successfully! (0x%08x)\n", hw_ver);
	}

	return 0;
}

int consys_polling_chipid(void)
{
	return _consys_polling_chipid_int(10, 1);
}

int consys_bus_clock_ctrl(enum consys_drv_type drv_type, unsigned int bus_clock)
{
	/* Cheetah doesn't need to do anything according to DE's excel */
	return 0;
}

int consys_emi_set_remapping_reg(void)
{
	struct consys_emi_addr_info *addr_info = emi_mng_get_phy_addr();

	/* 0x1806_01C4[19:0], ap_emi_base[19:0] = TBD (related to emi)
	   0x1806_01D4[19:0], wf_ap_peri_base[19:0] = 0x0_1100 (un-related to emi)
	*/
	if (addr_info->emi_ap_phy_base != 0)
		CONSYS_REG_WRITE_OFFSET_RANGE(REG_CONN_HOST_CSR_TOP_ADDR + CONN2AP_REMAP_MCU_EMI,
									addr_info->emi_ap_phy_base, 0, 16, 20);

	CONSYS_REG_WRITE_OFFSET_RANGE(REG_CONN_HOST_CSR_TOP_ADDR + CONN2AP_REMAP_WF_PERI,
									0x300D0000, 0, 16, 20);

	CONSYS_REG_WRITE_OFFSET_RANGE(REG_CONN_HOST_CSR_TOP_ADDR + CONN2AP_RSVD_PERI_REGION1,
									0x11F20000, 0, 16, 20);

	return 0;
}

int consys_emi_set_region_protection(void)
{
	struct consys_emi_addr_info *addr_info = emi_mng_get_phy_addr();

	/* set infra top emi address range */
	if (addr_info->emi_ap_phy_base != 0) {
		CONSYS_REG_WRITE(REG_CONN_INFRA_BUS_CR_ADDR + CONN2AP_EMI_PATH_ADDR_START,
						addr_info->emi_ap_phy_base);

		if (addr_info->emi_ap_phy_size != 0)
			CONSYS_REG_WRITE(REG_CONN_INFRA_BUS_CR_ADDR + CONN2AP_EMI_PATH_ADDR_END,
							addr_info->emi_ap_phy_base + addr_info->emi_ap_phy_size);
	}

	return 0;
}

int connsys_d_die_cfg(void)
{
	unsigned int efuse;

	efuse = CONSYS_REG_READ(REG_CONN_INFRA_CFG_ADDR + EFUSE);
	pr_info("D-die efuse: 0x%08x\n", efuse);

	return 0;
}

int connsys_conninfra_sysram_hw_ctrl(void)
{
	/* conn_infra sysram hw control setting -> disable hw power down */
	CONSYS_REG_WRITE(REG_CONN_INFRA_RGU_ADDR + SYSRAM_HWCTL_PDN, 0x0);

	/* conn_infra sysram hw control setting -> enable hw sleep */
	CONSYS_REG_WRITE(REG_CONN_INFRA_RGU_ADDR + SYSRAM_HWCTL_SLP, 0x1);

	return 0;
}

int connsys_spi_master_cfg(void)
{
	/* wt_slp CR for A-die ck_en/wake_en control */
	/*
										RFSPI #0			RFSPI #1
		WF_CK_ADDR						0x18005070[11:0]	0x18085070[11:0]	0xA04
		WF_B1_CK_ADDR					0x18005070[27:16]	0x18085070[27:16]	0xAF4
		WF_WAKE_ADDR					0x18005074[11:0]	0x18085074[11:0]	0x090
		WF_B1_WAKE_ADDR 				0x18005074[27:16]	0x18085074[27:16]	0x0A0
		WF_ZPS_ADDR 					0x18005078[11:0]	0x18085078[11:0]	0x08C
		WF_B1_ZPS_ADDR					0x18005078[27:16]	0x18085078[27:16]	0x09C
		TOP_CK_ADDR 					0x18005084[11:0]	0x18085084[11:0]	0xA00
		WF_B0_CMD_ADDR					0x1800508c[11:0]	0x1808508c[11:0]	0x0F0
		WF_B1_CMD_ADDR					0x18005090[11:0]	0x18085090[11:0]	0x0F4
	*/
	CONSYS_REG_WRITE_MASK(REG_CONN_WT_SLP_CTL_REG_ADDR + WB_WF_CK_ADDR, 0xAF40A04, 0xFFF0FFF);
	CONSYS_REG_WRITE_MASK(REG_CONN_WT_SLP_CTL_REG_ADDR + WB_WF_WAKE_ADDR, 0x0A00090, 0xFFF0FFF);
	CONSYS_REG_WRITE_MASK(REG_CONN_WT_SLP_CTL_REG_ADDR + WB_WF_ZPS_ADDR, 0x09C008C, 0xFFF0FFF);
	CONSYS_REG_WRITE_MASK(REG_CONN_WT_SLP_CTL_REG_ADDR + WB_TOP_CK_ADDR, 0xA00, 0xFFF);
	CONSYS_REG_WRITE_MASK(REG_CONN_WT_SLP_CTL_REG_ADDR + WB_WF_B0_CMD_ADDR, 0x0F0, 0xFFF);
	CONSYS_REG_WRITE_MASK(REG_CONN_WT_SLP_CTL_REG_ADDR + WB_WF_B1_CMD_ADDR, 0x0F4, 0xFFF);

	/* Cheetah doesn't need to configure RFSPI#1 */

	return 0;
}

static int consys_spi_read_nolock(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int *data)
{
	int check = 0;
	unsigned long rf_spi_addr = 0;
	const struct spi_op *op = NULL;
	unsigned char adie_idx = ((subsystem & 0xF0) >> 4); //0: one adie, 1: two adie
	unsigned char subsystem_idx = (subsystem & 0xF);

	if (!data) {
		pr_err("invalid data ptr\n");
		return CONNINFRA_SPI_OP_FAIL;
	}

	op = &spi_op_array[subsystem_idx];
	if (adie_idx != 0)
		rf_spi_addr = REG_INST2_CONN_RF_SPI_MST_REG_ADDR;
	else
		rf_spi_addr = REG_CONN_RF_SPI_MST_REG_ADDR;

	/* Read action:
	 * 1. Polling busy_cr[polling_bit] should be 0
	 * 2. Write addr_cr with data being {read_addr_format | addr[11:0]}
	 * 3. Trigger SPI by writing write_data_cr as 0
	 * 4. Polling busy_cr[polling_bit] as 0
	 * 5. Read data_cr[data_mask]
	 */

	CONSYS_REG_BIT_POLLING(rf_spi_addr + op->busy_cr, op->polling_bit, 0, 100, 500, check);
	if (check != 0) {
		pr_err("[%d][STEP1] polling 0x%08lx bit %d fail. Value=0x%08x\n",
				subsystem, rf_spi_addr + op->busy_cr, op->polling_bit,
				CONSYS_REG_READ(rf_spi_addr + op->busy_cr));
		return CONNINFRA_SPI_OP_FAIL;
	}

	CONSYS_REG_WRITE(rf_spi_addr + op->addr_cr, (op->read_addr_format | addr));
	CONSYS_REG_WRITE(rf_spi_addr + op->write_data_cr, 0);

	CONSYS_REG_BIT_POLLING(rf_spi_addr + op->busy_cr, op->polling_bit, 0, 100, 500, check);
	if (check != 0) {
		pr_err("[%d][STEP4] polling 0x%08lx bit %d fail. Value=0x%08x\n",
				subsystem, rf_spi_addr + op->busy_cr,
				op->polling_bit, CONSYS_REG_READ(rf_spi_addr + op->busy_cr));
		return CONNINFRA_SPI_OP_FAIL;
	}

	check = CONSYS_REG_READ_BIT(rf_spi_addr + op->read_data_cr, op->read_data_mask);
	*data = check;

	return 0;
}

static int consys_spi_write_nolock(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int data)
{
	int check = 0;
	unsigned long rf_spi_addr = 0;
	const struct spi_op *op = NULL;
	unsigned char adie_idx = ((subsystem & 0xF0) >> 4); //0: one adie, 1: two adie
	unsigned char subsystem_idx = (subsystem & 0xF);

	op = &spi_op_array[subsystem_idx];
	if (adie_idx != 0)
		rf_spi_addr = REG_INST2_CONN_RF_SPI_MST_REG_ADDR;
	else
		rf_spi_addr = REG_CONN_RF_SPI_MST_REG_ADDR;

	/* Write action:
	 * 1. Wait busy_cr[polling_bit] as 0
	 * 2. Write addr_cr with data being {write_addr_format | addr[11:0]
	 * 3. Write write_data_cr ad data
	 * 4. Wait busy_cr[polling_bit] as 0
	 */

	CONSYS_REG_BIT_POLLING(rf_spi_addr + op->busy_cr, op->polling_bit, 0, 100, 500, check);
	if (check != 0) {
		pr_err("[%d][STEP1] polling 0x%08lx bit %d fail. Value=0x%08x\n",
			subsystem, rf_spi_addr + op->busy_cr,
			op->polling_bit, CONSYS_REG_READ(rf_spi_addr + op->busy_cr));
		return CONNINFRA_SPI_OP_FAIL;
	}

	CONSYS_REG_WRITE(rf_spi_addr + op->addr_cr, (op->write_addr_format | addr));
	CONSYS_REG_WRITE(rf_spi_addr + op->write_data_cr, data);

	check = 0;
	CONSYS_REG_BIT_POLLING(rf_spi_addr + op->busy_cr, op->polling_bit, 0, 100, 500, check);
	if (check != 0) {
		pr_err("[%d][STEP4] polling 0x%08lx bit %d fail. Value=0x%08x\n",
			subsystem, rf_spi_addr + op->busy_cr,
			op->polling_bit, CONSYS_REG_READ(rf_spi_addr + op->busy_cr));
		return CONNINFRA_SPI_OP_FAIL;
	}

	pr_info("addr=0x%04x, val=0x%08x\n", addr, data);

	return 0;
}

static int consys_sema_acquire(enum conn_semaphore_type index)
{
	if (CONSYS_REG_READ_BIT((REG_CONN_SEMAPHORE_ADDR + CONN_SEMA00_M2_OWN_STA + index*4), 0x1) == 0x1) {
		return CONN_SEMA_GET_SUCCESS;
	} else {
		return CONN_SEMA_GET_FAIL;
	}
}

int consys_sema_acquire_timeout(unsigned int index, unsigned int usec)
{
	int i;

	if (index >= CONN_SEMA_NUM_MAX) {
		pr_err("wrong index: %d\n", index);
		return CONN_SEMA_GET_FAIL;
	}

	for (i = 0; i < usec; i++) {
		if (consys_sema_acquire(index) == CONN_SEMA_GET_SUCCESS) {
			return CONN_SEMA_GET_SUCCESS;
		}
		udelay(1);
	}
	pr_err("Get semaphore 0x%x timeout, dump status:\n", index);
	pr_err("M0:[0x%x] M1:[0x%x] M2:[0x%x] M3:[0x%x] M4:[0x%x] M5:[0x%x] M6:[0x%x] M7:[0x%x]\n",
			CONSYS_REG_READ(REG_CONN_SEMAPHORE_ADDR + CONN_SEMA_OWN_BY_M0_STA_REP),
			CONSYS_REG_READ(REG_CONN_SEMAPHORE_ADDR + CONN_SEMA_OWN_BY_M1_STA_REP),
			CONSYS_REG_READ(REG_CONN_SEMAPHORE_ADDR + CONN_SEMA_OWN_BY_M2_STA_REP),
			CONSYS_REG_READ(REG_CONN_SEMAPHORE_ADDR + CONN_SEMA_OWN_BY_M3_STA_REP),
			CONSYS_REG_READ(REG_CONN_SEMAPHORE_ADDR + CONN_SEMA_OWN_BY_M4_STA_REP),
			CONSYS_REG_READ(REG_CONN_SEMAPHORE_ADDR + CONN_SEMA_OWN_BY_M5_STA_REP),
			CONSYS_REG_READ(REG_CONN_SEMAPHORE_ADDR + CONN_SEMA_OWN_BY_M6_STA_REP),
			CONSYS_REG_READ(REG_CONN_SEMAPHORE_ADDR + CONN_SEMA_OWN_BY_M7_STA_REP));

	return CONN_SEMA_GET_FAIL;
}

void consys_sema_release(unsigned int index)
{
	if (index >= CONN_SEMA_NUM_MAX) {
		pr_err("wrong index: %d\n", index);
		return;
	}

	CONSYS_REG_WRITE((REG_CONN_SEMAPHORE_ADDR + CONN_SEMA00_M2_OWN_REL + index*4), 0x1);
}

int consys_spi_read(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int *data)
{
	int ret;

	/* Get semaphore before read */
	if (consys_sema_acquire_timeout(CONN_SEMA_RFSPI_INDEX, CONN_SEMA_TIMEOUT) == CONN_SEMA_GET_FAIL) {
		pr_err("[SPI READ] Require semaphore fail\n");
		return CONNINFRA_SPI_OP_FAIL;
	}

	ret = consys_spi_read_nolock(subsystem, addr, data);

	consys_sema_release(CONN_SEMA_RFSPI_INDEX);

	return ret;
}

int consys_spi_write(enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int data)
{
	int ret;

	/* Get semaphore before read */
	if (consys_sema_acquire_timeout(CONN_SEMA_RFSPI_INDEX, CONN_SEMA_TIMEOUT) == CONN_SEMA_GET_FAIL) {
		pr_err("[SPI WRITE] Require semaphore fail\n");
		return CONNINFRA_SPI_OP_FAIL;
	}

	ret = consys_spi_write_nolock(subsystem, addr, data);

	consys_sema_release(CONN_SEMA_RFSPI_INDEX);
	return ret;
}

static void consys_spi_write_offset_range_nolock(
	enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int value,
	unsigned int reg_offset, unsigned int value_offset, unsigned int size)
{
	unsigned int data = 0, data2;
	unsigned int reg_mask;
	int ret;

	pr_info("[%s] addr=0x%04x value=0x%08x reg_offset=%d value_offset=%d size=%d\n",
			g_spi_system_name[subsystem], addr, value, reg_offset, value_offset, size);

	value = (value >> value_offset);
	value = GET_BIT_RANGE(value, size, 0);
	value = (value << reg_offset);
	ret = consys_spi_read_nolock(subsystem, addr, &data);
	if (ret) {
		pr_err("[%s] Get 0x%08x error, ret=%d\n",
				g_spi_system_name[subsystem], addr, ret);
		return;
	}

	reg_mask = GENMASK(reg_offset + size - 1, reg_offset);
	data2 = data & (~reg_mask);
	data2 = (data2 | value);
	consys_spi_write_nolock(subsystem, addr, data2);

	pr_info("[%s] Write CR:0x%08x from 0x%08x to 0x%08x\n",
			g_spi_system_name[subsystem], addr, data, data2);
}

int consys_spi_write_offset_range(
	enum sys_spi_subsystem subsystem, unsigned int addr, unsigned int value,
	unsigned int reg_offset, unsigned int value_offset, unsigned int size)
{
	if (consys_sema_acquire_timeout(CONN_SEMA_RFSPI_INDEX, CONN_SEMA_TIMEOUT) == CONN_SEMA_GET_FAIL) {
		pr_err("[SPI READ] Require semaphore fail\n");
		return CONNINFRA_SPI_OP_FAIL;
	}
	consys_spi_write_offset_range_nolock(subsystem, addr, value, reg_offset, value_offset, size);

	consys_sema_release(CONN_SEMA_RFSPI_INDEX);

	return 0;
}

/*****************************************************************************
* FUNCTION
*  connsys_a_die_efuse_read
* DESCRIPTION
*  Read a-die efuse
* PARAMETERS
*  efuse_addr: read address
* RETURNS
*  int
*	0: fail, efuse is invalid
*	1: success, efuse is valid
*****************************************************************************/
static int connsys_a_die_efuse_read_nolock(
	enum sys_spi_subsystem subsystem, unsigned int efuse_ctrl, unsigned int efuse_addr,
	unsigned int *data0, unsigned int *data1, unsigned int *data2, unsigned int *data3)
{
	int ret = 0;
	int retry = 0;
	int ret0, ret1, ret2, ret3;
	unsigned int efuse_block_sel;

	if (data0 == NULL || data1 == NULL || data2 == NULL || data3 == NULL) {
		pr_err("invalid parameter (%p, %p, %p, %p)\n",
				data0, data1, data2, data3);
		return 0;
	}

	switch (efuse_ctrl) {
		case ATOP_EFUSE_CTRL_1:
			efuse_block_sel = 0x1;
			break;

		case ATOP_EFUSE_CTRL_2:
			efuse_block_sel = 0x2;
			break;

		case ATOP_EFUSE_CTRL_3:
			efuse_block_sel = 0x4;
			break;

		case ATOP_EFUSE_CTRL_4:
			efuse_block_sel = 0x8;
			break;

		default:
			pr_err("Not support for efuse block No. = %d\n", efuse_ctrl);
			return 0;
			break;
	}

	/* select Efuse block */
	consys_spi_write_nolock(subsystem, ATOP_RG_EFUSE_CFG5, efuse_block_sel);

	/* Efuse control clear, clear Status /trigger
	 * Address: ATOP EFUSE_CTRL_write_efsrom_kick_and_read_kick_busy_flag (0x108[30])
	 * Data: 1'b0
	 * Action: TOPSPI_WR
	 */
	consys_spi_read_nolock(subsystem, efuse_ctrl, &ret);
	ret &= ~(0x1 << 30);
	consys_spi_write_nolock(subsystem, efuse_ctrl, ret);

	/* Efuse Read 1st 16byte
	 * Address:
	 *    ATOP EFUSE_CTRL_efsrom_mode (0x108[7:6]) = 2'b00
	 *    ATOP EFUSE_CTRL_efsrom_ain (0x108[25:16]) = efuse_addr (0)
	 *    ATOP EFUSE_CTRL_write_efsrom_kick_and_read_kick_busy_flag (0x108[30]) = 1'b1
	 * Action: TOPSPI_WR
	 */
	consys_spi_read_nolock(subsystem, efuse_ctrl, &ret);
	ret &= ~(0x43FF00C0);
	ret |= (0x1 << 30);
	ret |= ((efuse_addr << 16) & 0x3FF0000);
	consys_spi_write_nolock(subsystem, efuse_ctrl, ret);

	/* Polling EFUSE busy = low
	 * (each polling interval is "30us" and polling timeout is 2ms)
	 * Address:
	 *    ATOP EFUSE_CTRL_write_efsrom_kick_and_read_kick_busy_flag (0x108[30]) = 1'b0
	 * Action: TOPSPI_Polling
	 */
	consys_spi_read_nolock(subsystem, efuse_ctrl, &ret);
	while ((ret & (0x1 << 30)) != 0 && retry < 70) {
		retry++;
		udelay(30);
		consys_spi_read_nolock(subsystem, efuse_ctrl, &ret);
	}
	if ((ret & (0x1 << 30)) != 0) {
		pr_err("EFUSE busy, retry failed(%d)\n", retry);
	}

	/* Check efuse_valid & return
	 * Address: ATOP EFUSE_CTRL_csri_efsrom_dout_vld_sync_1_ (0x108[29])
	 * Action: TOPSPI_RD
	 */
	/* if (efuse_valid == 1'b1)
	 *     Read Efuse Data to global var
	 */
	consys_spi_read_nolock(subsystem, efuse_ctrl, &ret);
	if (((ret & (0x1 << 29)) >> 29) == 1) {
		ret0 = consys_spi_read_nolock(subsystem, ATOP_EFUSE_RDATA0, data0);
		ret1 = consys_spi_read_nolock(subsystem, ATOP_EFUSE_RDATA1, data1);
		ret2 = consys_spi_read_nolock(subsystem, ATOP_EFUSE_RDATA2, data2);
		ret3 = consys_spi_read_nolock(subsystem, ATOP_EFUSE_RDATA3, data3);

		pr_info("efuse = [0x%08x, 0x%08x, 0x%08x, 0x%08x]\n", *data0, *data1, *data2, *data3);
		if (ret0 || ret1 || ret2 || ret3)
			pr_err("efuse read error: [%d, %d, %d, %d]\n", ret0, ret1, ret2, ret3);
		ret = 1;
	} else {
		pr_err("EFUSE is invalid\n");
		ret = 0;
	}

	return ret;
}

static int _connsys_a_die_thermal_cal(enum sys_spi_subsystem subsystem)
{
	int efuse_valid = 0;
	unsigned int efuse0 = 0, efuse1 = 0, efuse2 = 0, efuse3 = 0;

	/* thernal efuse data in 7976&7975 in EFUSE2 */
	efuse_valid = connsys_a_die_efuse_read_nolock(subsystem, ATOP_EFUSE_CTRL_2, ATOP_THADC_ANALOG,
													&efuse0, &efuse1, &efuse2, &efuse3);
	//if (efuse_valid) {
		if ((efuse0 & (0x1 << 7))) {
			consys_spi_write_offset_range_nolock(subsystem, ATOP_RG_TOP_THADC_BG, efuse0, 12, 3, 4);
			consys_spi_write_offset_range_nolock(subsystem, ATOP_RG_TOP_THADC_00, efuse0, 23, 0, 3);
		}
	//}

	efuse_valid = connsys_a_die_efuse_read_nolock(subsystem, ATOP_EFUSE_CTRL_2, ATOP_THADC_SLOP,
													&efuse0, &efuse1, &efuse2, &efuse3);
	//if (efuse_valid) {
		if((efuse0 & (0x1 << 7))) {
			consys_spi_write_offset_range_nolock(subsystem, ATOP_RG_TOP_THADC_00, efuse0, 26, 5, 2);
		}
	//}

	return 0;
}

static int _connsys_a_die_xtal_trim_7976(enum sys_spi_subsystem subsystem)
{
	unsigned int efuse0 = 0, efuse1 = 0, efuse2 = 0, efuse3 = 0;
	int c1c2_trim_result_ax_80m = 0, c1c2_trim_result_ax_40m = 0;
	unsigned int cbtop_strap_rdata = 0, xtal_strap_mode = 0, adie_rdata = 0, value = 0;

	connsys_a_die_efuse_read_nolock(subsystem, ATOP_EFUSE_CTRL_2, ATOP_XTAL_TRIM_FLOW,
									&efuse0, &efuse1, &efuse2, &efuse3);
	if ((efuse0 & (0x1 < 1))) {
		/* C1C2 80M AX */
		connsys_a_die_efuse_read_nolock(subsystem, ATOP_EFUSE_CTRL_2, ATOP_XTAL_CR_C1_SEL_AXM_80M_OSC,
										&efuse0, &efuse1, &efuse2, &efuse3);
		if ((efuse0 & (0x1 < 7)) == 0) {
			c1c2_trim_result_ax_80m = 64;
		} else {
			c1c2_trim_result_ax_80m = (efuse0 & 0x7F);
			connsys_a_die_efuse_read_nolock(subsystem, ATOP_EFUSE_CTRL_2, ATOP_XTAL_CR_C1_SEL_AXM_TRIM1_80M_OSC,
										&efuse0, &efuse1, &efuse2, &efuse3);
			if ((efuse0 & (0x1 < 7)) == 1) {
				if ((efuse0 & (0x1 < 6)) == 0) {
					c1c2_trim_result_ax_80m
= c1c2_trim_result_ax_80m + (efuse0 & 0x3F);
				} else {
					c1c2_trim_result_ax_80m = c1c2_trim_result_ax_80m - (efuse0 & 0x3F);
				}

				if (c1c2_trim_result_ax_80m > 127)
					c1c2_trim_result_ax_80m = 127;
				else if (c1c2_trim_result_ax_80m < 0)
					c1c2_trim_result_ax_80m = 0;
			}
		}

		/* C1C2 40M AX */
		connsys_a_die_efuse_read_nolock(subsystem, ATOP_EFUSE_CTRL_2, ATOP_XTAL_CR_C1_SEL_AXM_40M_OSC,
										&efuse0, &efuse1, &efuse2, &efuse3);
		if ((efuse0 & (0x1 < 7)) == 0) {
			c1c2_trim_result_ax_40m = 64;
		} else {
			c1c2_trim_result_ax_40m = (efuse0 & 0x7F);
			connsys_a_die_efuse_read_nolock(subsystem, ATOP_EFUSE_CTRL_2, ATOP_XTAL_CR_C1_SEL_AXM_TRIM1_40M_OSC,
										&efuse0, &efuse1, &efuse2, &efuse3);
			if ((efuse0 & (0x1 < 7)) == 1) {
				if ((efuse0 & (0x1 < 6)) == 0) {
					c1c2_trim_result_ax_40m
= c1c2_trim_result_ax_40m + (efuse0 & 0x3F);
				} else {
					c1c2_trim_result_ax_40m = c1c2_trim_result_ax_40m - (efuse0 & 0x3F);
				}

				if (c1c2_trim_result_ax_40m > 127)
					c1c2_trim_result_ax_40m = 127;
				else if (c1c2_trim_result_ax_40m < 0)
					c1c2_trim_result_ax_40m = 0;
			}
		}

		/* Update trim value to C1 and C2 */
		consys_spi_read_nolock(subsystem, ATOP_RG_STRAP_PIN_IN, &cbtop_strap_rdata);
		xtal_strap_mode = ((cbtop_strap_rdata & 0x70) >> 4);
		if ((xtal_strap_mode == 0x0) || (xtal_strap_mode == 0x2)) { //80m osc
			/* C1 */
			consys_spi_read_nolock(subsystem, 0x654, &adie_rdata);
			value = (adie_rdata & 0xFFFFFF) | ((c1c2_trim_result_ax_80m & 0xFF) << 24);
			consys_spi_write_nolock(subsystem, 0x654, value);

			/* C2 */
			consys_spi_read_nolock(subsystem, 0x658, &adie_rdata);
			value = (adie_rdata & 0xFFFFFF) | ((c1c2_trim_result_ax_80m & 0xFF) << 24);
			consys_spi_write_nolock(subsystem, 0x658, value);
		} else if ((xtal_strap_mode == 0x3) || (xtal_strap_mode == 0x4) || (xtal_strap_mode == 0x6)) { //40m osc
			/* C1 */
			consys_spi_read_nolock(subsystem, 0x654, &adie_rdata);
			value = (adie_rdata & 0xFF00FFFF) | ((c1c2_trim_result_ax_40m & 0xFF) << 16);
			consys_spi_write_nolock(subsystem, 0x654, value);

			/* C2 */
			consys_spi_read_nolock(subsystem, 0x658, &adie_rdata);
			value = (adie_rdata & 0xFF00FFFF) | ((c1c2_trim_result_ax_40m & 0xFF) << 16);
			consys_spi_write_nolock(subsystem, 0x658, value);
		}
	}

	return 0;
}

static int _connsys_a_die_sw_cntl(enum sys_spi_subsystem subsystem, unsigned char adie_idx)
{
	if (conn_hw_env[adie_idx].valid && (conn_hw_env[adie_idx].adie_id == 0x7976)) {
		if ((conn_hw_env[adie_idx].adie_hw_version == 0x8A00)
			 || (conn_hw_env[adie_idx].adie_hw_version == 0x8A10)
			 || (conn_hw_env[adie_idx].adie_hw_version == 0x8B00)){
			consys_spi_write_nolock(subsystem, ATOP_RG_TOP_THADC_00, 0x4A563B00);
			consys_spi_write_nolock(subsystem, ATOP_RG_XO_01, 0x1D59080F);
			consys_spi_write_nolock(subsystem, ATOP_RG_XO_03, 0x34C00FE0);
		} else {
			consys_spi_write_nolock(subsystem, ATOP_RG_TOP_THADC_00, 0x4A563B00);
			consys_spi_write_nolock(subsystem, ATOP_RG_XO_01, 0x1959C80F);
			consys_spi_write_nolock(subsystem, ATOP_RG_XO_03, 0x34D00FE0);
		}
	}

	return 0;
}

int _connsys_a_die_cfg_7976(unsigned char adie_idx)
{
	int check;
	unsigned int adie_chip_id = 0x0;
	unsigned char subsystem = 0;

	if (adie_idx == 1)
		subsystem = SYS_SPI_2ND_ADIE_TOP;
	else
		subsystem = SYS_SPI_TOP;

	/* release D Die to A Die Digital reset_b */
	if (adie_idx == 1)
		CONSYS_SET_BIT(REG_CONN_INFRA_CFG_ADDR + ADIE_CTL, 0x4);
	else
		CONSYS_SET_BIT(REG_CONN_INFRA_CFG_ADDR + ADIE_CTL, 0x1);

	/* Get semaphore once */
	if (consys_sema_acquire_timeout(CONN_SEMA_RFSPI_INDEX, CONN_SEMA_TIMEOUT) == CONN_SEMA_GET_FAIL) {
		pr_err("Require semaphore fail\n");
		return -1;
	}

	/* read a-die ID */
	check = consys_spi_read_nolock(subsystem, ATOP_CHIP_ID, &adie_chip_id);
	if (check) {
		/* Release semaphore */
		consys_sema_release(CONN_SEMA_RFSPI_INDEX);
		pr_err("Get ATOP_CHIP_ID fail, check=%d\n", check);
		return -1;
	}

	if (adie_idx < AIDE_NUM_MAX && ((adie_chip_id & 0xFFFF0000) != 0)) {
		conn_hw_env[adie_idx].valid = true;
		conn_hw_env[adie_idx].adie_hw_version = (adie_chip_id & 0xFFFF);
		conn_hw_env[adie_idx].adie_id = ((adie_chip_id & 0xFFFF0000) >> 16);
		conn_hw_env[adie_idx].is_rc_mode = 0;

		pr_info("adie_idx[%d], A-die CHIP ID = 0x%x, HW Version = 0x%x\n",
			adie_idx, conn_hw_env[adie_idx].adie_id, conn_hw_env[adie_idx].adie_hw_version);
	}

	/* enable TOPDIG CK */
	check = consys_spi_write_nolock(subsystem, ATOP_TOP_CLK_EN, 0xFFFFFFFF);

	/* config WRI CK select */
	if (one_adie_dbdc)
		check = consys_spi_write_nolock(subsystem, ATOP_RG_WRI_CK_SELECT, 0x1C);

	/* Thermal Cal (TOP) */
	_connsys_a_die_thermal_cal(subsystem);

	/* XTAL TRIM */
	_connsys_a_die_xtal_trim_7976(subsystem);

	/* SW control part  */
	_connsys_a_die_sw_cntl(subsystem, adie_idx);

	/* Release semaphore */
	consys_sema_release(CONN_SEMA_RFSPI_INDEX);

	return 0;
}

static int _connsys_a_die_xtal_trim_7975(enum sys_spi_subsystem subsystem)
{
	unsigned int efuse0 = 0, efuse1 = 0, efuse2 = 0, efuse3 = 0;
	unsigned int trim_result = 0, value = 0;
	int ret = 0;

	ret = connsys_a_die_efuse_read_nolock(subsystem, ATOP_EFUSE_CTRL_2, ATOP_7975_XTAL_TRIM_FLOW,
										  &efuse0, &efuse1, &efuse2, &efuse3);
	if (((efuse0 & 0x1) == 0) || (ret == 0))
		return 0;

	connsys_a_die_efuse_read_nolock(subsystem, ATOP_EFUSE_CTRL_2, ATOP_7975_XTAL_CALIBRATION,
									&efuse0, &efuse1, &efuse2, &efuse3);
	if ((efuse0 & (0x1 << 7))) {
		trim_result = (efuse0 & 0x7F);
		trim_result = (trim_result & 0x7F);
	}

	connsys_a_die_efuse_read_nolock(subsystem, ATOP_EFUSE_CTRL_2, ATOP_7975_XTAL_TRIM2_COMPENSATION,
									&efuse0, &efuse1, &efuse2, &efuse3);
	if ((efuse0 & (0x1 << 7))){
		if ((efuse0 & (0x1 << 6)))
			trim_result -= (efuse0 & 0x3F);
		else
			trim_result += (efuse0 & 0x3F);
		trim_result = (trim_result & 0x7F);
	}

	connsys_a_die_efuse_read_nolock(subsystem, ATOP_EFUSE_CTRL_2, ATOP_7975_XTAL_TRIM3_COMPENSATION,
									&efuse0, &efuse1, &efuse2, &efuse3);
	if ((efuse0 & (0x1 << 7))){
		if ((efuse0 & (0x1 << 6)))
			trim_result -= (efuse0 & 0x3F);
		else
			trim_result += (efuse0 & 0x3F);
		trim_result = (trim_result & 0x7F);
	}

	connsys_a_die_efuse_read_nolock(subsystem, ATOP_EFUSE_CTRL_2, ATOP_7975_XTAL_TRIM4_COMPENSATION,
									&efuse0, &efuse1, &efuse2, &efuse3);
	if ((efuse0 & (0x1 << 7))){
		if ((efuse0 & (0x1 << 6)))
			trim_result -= (efuse0 & 0x3F);
		else
			trim_result += (efuse0 & 0x3F);
		trim_result = (trim_result & 0x7F);
	}

	/* Update Trim Value to C1 and C2*/
	/* Step 1 */
	consys_spi_read_nolock(subsystem, ATOP_7975_CR_C1_C2_A94, &value);
	value = ((value & 0xf8080fff) | ((trim_result << 20) | (trim_result << 12)));
	consys_spi_write_nolock(subsystem, ATOP_7975_CR_C1_C2_A94, value);

	/* Step 2 */
	consys_spi_read_nolock(subsystem, ATOP_7975_CR_C1_C2_A18, &value);
	if(value & (1<<29)){
		consys_spi_read_nolock(subsystem, ATOP_7975_CR_C1_C2_A84, &value);
		value = (value & 0x7fffffff);
		consys_spi_write_nolock(subsystem, ATOP_7975_CR_C1_C2_A84, value);
	}

	/* Step 3 */
	consys_spi_read_nolock(subsystem, ATOP_7975_CR_C1_C2_AA4, &value);
	value = ((value & 0xfffeffff) | 0x10000);
	consys_spi_write_nolock(subsystem, ATOP_7975_CR_C1_C2_AA4, value);

	return 0;
}


static int _connsys_a_die_form_patch_7975(enum sys_spi_subsystem subsystem)
{
	pr_info("Form 7975 adie Patch\n");

	/* disable CAL LDO and fine tune RFDIG LDO, 20191218 */
    consys_spi_write_nolock(subsystem, 0x348, 0x00000002);

	/* disable CAL LDO and fine tune RFDIG LDO, 20191218 */
    consys_spi_write_nolock(subsystem, 0x378, 0x00000002);

	/* disable CAL LDO and fine tune RFDIG LDO, 20191218 */
    consys_spi_write_nolock(subsystem, 0x3A8, 0x00000002);

	/* disable CAL LDO and fine tune RFDIG LDO, 20191218 */
    consys_spi_write_nolock(subsystem, 0x3D8, 0x00000002);

	/* set CKA driving and filter */
    consys_spi_write_nolock(subsystem, 0xA1C, 0x30000AAA);

	/* set CKB LDO to 1.4V */
    consys_spi_write_nolock(subsystem, 0xA84, 0x8470008A);

	/* turn on SX0 LTBUF */
    consys_spi_write_nolock(subsystem, 0x074, 0x00000007);

	/* CK_BUF_SW_EN=1 (all buf in manual mode.) */
    consys_spi_write_nolock(subsystem, 0xAA4, 0x01001FC0);

	/* BT mode/WF normal mode 32'h=00000005 */
    consys_spi_write_nolock(subsystem, 0x070, 0x00000005);

	/* BG thermal sensor offset update */
    consys_spi_write_nolock(subsystem, 0x344, 0x00000088);

	/* BG thermal sensor offset update */
    consys_spi_write_nolock(subsystem, 0x374, 0x00000088);

	/* BG thermal sensor offset update */
    consys_spi_write_nolock(subsystem, 0x3A4, 0x00000088);

	/* BG thermal sensor offset update */
    consys_spi_write_nolock(subsystem, 0x3D4, 0x00000088);

	/* set WCON VDD IPTAT to "0000" */
    consys_spi_write_nolock(subsystem, 0xA80, 0x44D07000);

	/* change back LTBUF SX3 drving to default value, 20191113 */
    consys_spi_write_nolock(subsystem, 0xA88, 0x3900AAAA);

	/* SM input cap off */
    consys_spi_write_nolock(subsystem, 0x2C4, 0x00000000);


	return 0;
}

int _connsys_a_die_cfg_7975(unsigned char adie_idx)
{
	int check;
	unsigned int adie_chip_id = 0x0;
	unsigned char subsystem = 0;

	if (adie_idx == 1)
		subsystem = SYS_SPI_2ND_ADIE_TOP;
	else
		subsystem = SYS_SPI_TOP;

	/* release D Die to A Die Digital reset_b */
	if (adie_idx == 1)
		CONSYS_SET_BIT(REG_CONN_INFRA_CFG_ADDR + ADIE_CTL, 0x4);
	else
		CONSYS_SET_BIT(REG_CONN_INFRA_CFG_ADDR + ADIE_CTL, 0x1);

	/* Get semaphore once */
	if (consys_sema_acquire_timeout(CONN_SEMA_RFSPI_INDEX, CONN_SEMA_TIMEOUT) == CONN_SEMA_GET_FAIL) {
		pr_err("Require semaphore fail\n");
		return -1;
	}

	/* read a-die ID */
	check = consys_spi_read_nolock(subsystem, ATOP_CHIP_ID, &adie_chip_id);
	if (check) {
		/* Release semaphore */
		consys_sema_release(CONN_SEMA_RFSPI_INDEX);
		pr_err("Get ATOP_CHIP_ID fail, check=%d\n", check);
		return -1;
	}

	if (adie_idx < AIDE_NUM_MAX) {
		conn_hw_env[adie_idx].valid = true;
		conn_hw_env[adie_idx].adie_hw_version = (adie_chip_id & 0xFFFF);
		conn_hw_env[adie_idx].adie_id = ((adie_chip_id & 0xFFFF0000) >> 16);
		conn_hw_env[adie_idx].is_rc_mode = 0;

		pr_info("adie_idx[%d], A-die CHIP ID = 0x%x, HW Version = 0x%x\n",
			adie_idx, conn_hw_env[adie_idx].adie_id, conn_hw_env[adie_idx].adie_hw_version);
	}

	/* enable TOPDIG CK */
	check = consys_spi_write_nolock(subsystem, ATOP_TOP_CLK_EN, 0xFFFFFFFF);

	/* Thermal Cal (TOP) */
	_connsys_a_die_thermal_cal(subsystem);

	/* XTAL TRIM */
	_connsys_a_die_xtal_trim_7975(subsystem);

	/* Form Harrier E2 Patch */
	_connsys_a_die_form_patch_7975(subsystem);

	/* Release semaphore */
	consys_sema_release(CONN_SEMA_RFSPI_INDEX);

	return 0;
}

int connsys_a_die_cfg(void)
{
	int ret;
	memset(&conn_hw_env, 0, sizeof(conn_hw_env));

	if (adie_7976){
		if (adie_cfg_type == ADIE_TYPE_TWO) {
			ret = _connsys_a_die_cfg_7976(0);
			ret = _connsys_a_die_cfg_7976(1);
		} else {
			if (one_adie_dbdc) {
				ret = _connsys_a_die_cfg_7976(0);
			} else {
				ret = _connsys_a_die_cfg_7976(1);
			}
		}
	} else {
		if (adie_cfg_type == ADIE_TYPE_TWO) {
			ret = _connsys_a_die_cfg_7975(0);
			ret = _connsys_a_die_cfg_7975(1);
		} else {
			if (!one_adie_dbdc)
				ret = _connsys_a_die_cfg_7975(1);
		}
	}

	return ret;
}

int _connsys_afe_wbg_cal_7976(unsigned char wbg_idx, unsigned char rfspi_idx)
{
	int check;
	unsigned long afe_ctl_addr = 0;
	unsigned char subsystem = 0;

	if ((wbg_idx == 0) && (rfspi_idx == 0)) {
		afe_ctl_addr = REG_CONN_AFE_CTL_ADDR;
		subsystem = SYS_SPI_TOP;
	} else if ((wbg_idx == 1) && (rfspi_idx == 0)) {
		afe_ctl_addr = REG_CONN_AFE_CTL_2ND_ADDR;
		subsystem = SYS_SPI_TOP;
	} else if ((wbg_idx == 1) && (rfspi_idx == 1)) {
		afe_ctl_addr = REG_CONN_AFE_CTL_2ND_ADDR;
		subsystem = SYS_SPI_2ND_ADIE_TOP;
	} else {
		pr_err("Not support for this combination (wbg_idx=%d, rfspi_idx=%d)\n",
				wbg_idx, rfspi_idx);
		return -1;
	}

	/* Get semaphore once */
	if (consys_sema_acquire_timeout(CONN_SEMA_RFSPI_INDEX, CONN_SEMA_TIMEOUT) == CONN_SEMA_GET_FAIL) {
		pr_err("Require semaphore fail\n");
		return -1;
	}

	/* set WF_PAD to HighZ */
	check = consys_spi_write_nolock(subsystem, ATOP_RG_ENCAL_WBTAC_IF_SW, 0x88888005);

	/* AFE WBG CAL SEQ1 (RC calibration) */
	CONSYS_SET_BIT(afe_ctl_addr + RG_DIG_EN_01, 0x1);
	udelay(60);
	CONSYS_CLR_BIT(afe_ctl_addr + RG_DIG_EN_01, 0x1);

	/* AFE WBG CAL SEQ2 (TX calibration) */
	CONSYS_SET_BIT(afe_ctl_addr + RG_DIG_EN_03, (0x1 << 21));
	udelay(30);
	CONSYS_SET_BIT(afe_ctl_addr + RG_DIG_EN_03, (0x1 << 20));
	udelay(60);
	CONSYS_SET_BIT(afe_ctl_addr + RG_DIG_EN_01, 0x203E0000);
	udelay(800);
	CONSYS_CLR_BIT(afe_ctl_addr + RG_DIG_EN_01, 0x203E0000);
	CONSYS_CLR_BIT(afe_ctl_addr + RG_DIG_EN_03, 0x300000);

	/* disable WF_PAD to HighZ */
	check = consys_spi_write_nolock(subsystem, ATOP_RG_ENCAL_WBTAC_IF_SW, 0x00000005);

	/* Release semaphore */
	consys_sema_release(CONN_SEMA_RFSPI_INDEX);

	return 0;
}

int _connsys_afe_wbg_cal_7975(unsigned char wbg_idx, unsigned char rfspi_idx)
{
	int check;
	unsigned long afe_ctl_addr = 0;
	unsigned char subsystem = 0;

	if ((wbg_idx == 0) && (rfspi_idx == 0)) {
		afe_ctl_addr = REG_CONN_AFE_CTL_ADDR;
		subsystem = SYS_SPI_TOP;
	} else if ((wbg_idx == 1) && (rfspi_idx == 0)) {
		afe_ctl_addr = REG_CONN_AFE_CTL_2ND_ADDR;
		subsystem = SYS_SPI_TOP;
	} else if ((wbg_idx == 1) && (rfspi_idx == 1)) {
		afe_ctl_addr = REG_CONN_AFE_CTL_2ND_ADDR;
		subsystem = SYS_SPI_2ND_ADIE_TOP;
	} else {
		pr_err("Not support for this combination (wbg_idx=%d, rfspi_idx=%d)\n",
				wbg_idx, rfspi_idx);
		return -1;
	}

	/* Get semaphore once */
	if (consys_sema_acquire_timeout(CONN_SEMA_RFSPI_INDEX, CONN_SEMA_TIMEOUT) == CONN_SEMA_GET_FAIL) {
		pr_err("Require semaphore fail\n");
		return -1;
	}

	/* set WF_PAD to HighZ */
	check = consys_spi_write_nolock(subsystem, ATOP_RG_ENCAL_WBTAC_IF_SW, 0x80000000);

	/* AFE WBG CAL SEQ1 (RC calibration) */
	CONSYS_SET_BIT(afe_ctl_addr + RG_DIG_EN_01, 0x1);
	udelay(60);
	CONSYS_CLR_BIT(afe_ctl_addr + RG_DIG_EN_01, 0x1);

	/* AFE WBG CAL SEQ2 (TX calibration) */
	CONSYS_SET_BIT(afe_ctl_addr + RG_DIG_EN_03, (0x1 << 21));
	udelay(30);
	CONSYS_SET_BIT(afe_ctl_addr + RG_DIG_EN_03, (0x1 << 20));
	udelay(60);
	CONSYS_SET_BIT(afe_ctl_addr + RG_DIG_EN_01, 0x3E0000);
	udelay(800);
	CONSYS_CLR_BIT(afe_ctl_addr + RG_DIG_EN_01, 0x3E0000);
	CONSYS_CLR_BIT(afe_ctl_addr + RG_DIG_EN_03, 0x300000);

	/* disable WF_PAD to HighZ */
	check = consys_spi_write_nolock(subsystem, ATOP_RG_ENCAL_WBTAC_IF_SW, 0x00000005);

	/* Release semaphore */
	consys_sema_release(CONN_SEMA_RFSPI_INDEX);

	return 0;
}

int connsys_afe_wbg_cal(void)
{
	int ret;

	if (adie_7976){
		if (adie_cfg_type == ADIE_TYPE_TWO) {
			ret = _connsys_afe_wbg_cal_7976(0, 0);
			ret = _connsys_afe_wbg_cal_7976(1, 1);
		} else {
			if (one_adie_dbdc) {
				ret = _connsys_afe_wbg_cal_7976(0, 0);
			} else {
				ret = _connsys_afe_wbg_cal_7976(1, 1);
			}
		}
	} else {
		if (adie_cfg_type == ADIE_TYPE_TWO) {
			ret = _connsys_afe_wbg_cal_7975(0, 0);
			ret = _connsys_afe_wbg_cal_7975(1, 1);
		} else {
			if (!one_adie_dbdc)
				ret = _connsys_afe_wbg_cal_7975(1, 1);
		}
	}

	return ret;
}

int _connsys_subsys_pll_initial(unsigned char wbg_idx)
{
	unsigned long afe_ctl_addr = 0;

	if (wbg_idx == 0) {
		afe_ctl_addr = REG_CONN_AFE_CTL_ADDR;
	} else if (wbg_idx == 1) {
		afe_ctl_addr = REG_CONN_AFE_CTL_2ND_ADDR;
	} else {
		pr_err("Not support for this wbg index (wbg_idx=%d)\n", wbg_idx);
		return -1;
	}

	/* SWITCH(xtal_freq)
	   CASE SYS_XTAL_40000K
	*/
	/* set BPLL stable time = 30us (value = 30 * 1000 *1.01 / 25ns) */
	CONSYS_REG_WRITE_RANGE(afe_ctl_addr + RG_PLL_STB_TIME, (0x4BC << 16), 30, 16);
	/* set WPLL stable time = 50us (value = 50 * 1000 *1.01 / 25ns) */
	CONSYS_REG_WRITE_RANGE(afe_ctl_addr + RG_PLL_STB_TIME, 0x7E4, 14, 0);
	/* BT pll_en will turn on BPLL only (may change in different XTAL option) */
	CONSYS_REG_WRITE_RANGE(afe_ctl_addr + RG_DIG_EN_02, (0x1 << 6), 7, 6);
	/* WF pll_en will turn on WPLL only (may change in different XTAL option) */
	CONSYS_REG_WRITE_RANGE(afe_ctl_addr + RG_DIG_EN_02, 0x2, 1, 0);
	/* MCU pll_en will turn on BPLL (may change in different XTAL option) */
	CONSYS_REG_WRITE_RANGE(afe_ctl_addr + RG_DIG_EN_02, (0x1 << 2), 3, 2);
	/* MCU pll_en will turn on BPLL + WPLL (may change in different XTAL option) */
	CONSYS_REG_WRITE_RANGE(afe_ctl_addr + RG_DIG_EN_02, (0x2 << 16), 17, 16);
	/* CONN_INFRA CLKGEN WPLL AND BPLL REQUEST */
	CONSYS_REG_WRITE_RANGE(afe_ctl_addr + RG_DIG_TOP_01, (0x9 << 15), 18, 15);

	return 0;
}

int connsys_subsys_pll_initial(void)
{
	int ret;

	ret = _connsys_subsys_pll_initial(0);

	return ret;
}

int connsys_osc_legacy_mode(void)
{
	/* disable conn_top rc osc_ctrl_top */
	CONSYS_CLR_BIT(REG_CONN_INFRA_CFG_ADDR + CONN_INFRA_CFG_RC_CTL_0, 0x80);
	CONSYS_REG_WRITE_RANGE(REG_CONN_INFRA_CFG_ADDR + OSC_CTL_0, 0x80706, 23, 0);

	return 0;
}

int connsys_top_pwr_ctrl(void)
{
	/* prevent subsys from power on/of in a short time interval */
	CONSYS_CLR_BIT_WITH_KEY(REG_CONN_INFRA_RGU_ADDR + BGFYS_ON_TOP_PWR_CTL, 0x40, 0x42540000);
	CONSYS_CLR_BIT_WITH_KEY(REG_CONN_INFRA_RGU_ADDR + WFSYS_ON_TOP_PWR_CTL, 0x40, 0x57460000);

	return 0;
}

int connsys_conn_infra_bus_timeout(void)
{
	/* set conn_infra_off bus timeout */
	CONSYS_REG_WRITE_RANGE(REG_CONN_INFRA_BUS_CR_ADDR + CONN_INFRA_BUS_OFF_TIMEOUT_CTRL, (0x2 << 7), 14, 7);
	/* enable conn_infra off bus timeout feature */
	CONSYS_REG_WRITE_RANGE(REG_CONN_INFRA_BUS_CR_ADDR + CONN_INFRA_BUS_OFF_TIMEOUT_CTRL, 0xF, 3, 0);

	/* set conn_infra_on bus timeout */
	CONSYS_REG_WRITE_RANGE(REG_CONN_INFRA_BUS_CR_ADDR + CONN_INFRA_BUS_ON_TIMEOUT_CTRL, (0xC << 7), 14, 7);
	/* enable conn_infra_on bus timeout feature */
	CONSYS_REG_WRITE_RANGE(REG_CONN_INFRA_BUS_CR_ADDR + CONN_INFRA_BUS_ON_TIMEOUT_CTRL, 0xF, 3, 0);

	return 0;
}

int connsys_clkgen_wpll_hw_ctrl(void)
{
	/* set hclk_div_1 with wpll div sel */
	CONSYS_REG_WRITE_MASK(REG_CONN_INFRA_CLKGEN_ON_TOP_ADDR + CKGEN_BUS_WPLL_DIV_1, 0x4, 0xFC);

	/* set hclk_div_2 with wpll div sel */
	CONSYS_REG_WRITE_MASK(REG_CONN_INFRA_CLKGEN_ON_TOP_ADDR + CKGEN_BUS_WPLL_DIV_2, 0x4, 0xFC);

#ifndef CONFIG_FPGA_EARLY_PORTING
	/* enable conn_infra bus wpll div_1 */
	CONSYS_SET_BIT(REG_CONN_INFRA_CLKGEN_ON_TOP_ADDR + CKGEN_BUS_WPLL_DIV_1, 0x1);

	/* enable conn_infra bus wpll div_2 */
	CONSYS_SET_BIT(REG_CONN_INFRA_CLKGEN_ON_TOP_ADDR + CKGEN_BUS_WPLL_DIV_2, 0x1);
#endif

#ifndef CONFIG_FPGA_EARLY_PORTING
	/* set rfspi wpll div sel
	   enable rfspis wpll div
	*/
	CONSYS_REG_WRITE_MASK(REG_CONN_INFRA_CLKGEN_ON_TOP_ADDR + CKGEN_RFSPI_WPLL_DIV, 0x21, 0xFD);
#else
	/* set rfspi wpll div sel */
	CONSYS_REG_WRITE_MASK(REG_CONN_INFRA_CLKGEN_ON_TOP_ADDR + CKGEN_RFSPI_WPLL_DIV, 0x20, 0xFC);
#endif

	/* disable conn_infra bus clock sw control  ==> conn_infra bus clock hw control */
	CONSYS_CLR_BIT(REG_CONN_INFRA_CLKGEN_ON_TOP_ADDR + CKGEN_BUS, 0x800000);

	/* Conn_infra HW_CONTROL => conn_infra enter dsleep mode */
	CONSYS_SET_BIT(REG_CONN_INFRA_CFG_ADDR + CONN_INFRA_CFG_PWRCTRL0, 0x1);

	return 0;
}

int consys_conninfra_top_wakeup(void)
{
	/* wake up conn_infra */
	CONSYS_SET_BIT(REG_CONN_HOST_CSR_TOP_ADDR + CONN_INFRA_WAKEPU_TOP, 0x1);

	/* Wait 900us (apply this for CONNSYS XO clock ready) */
	udelay(900);

	/* Check CONNSYS version ID
	 * (polling "10 times" for specific project code and each polling interval is "1ms")
	 */
	if (consys_polling_chipid() != 0) {
		pr_err("Polling chip id fail\n");
		return -1;
	}

	return 0;
}

int consys_conninfra_top_sleep(void)
{
	/* release conn_infra force on */
	CONSYS_CLR_BIT(REG_CONN_HOST_CSR_TOP_ADDR + CONN_INFRA_WAKEPU_TOP, 0x1);

	return 0;
}

int _consys_adie_top_ck_en_on_off_ctrl(unsigned char rfspi_idx, enum consys_drv_type type, unsigned char on)
{
	int check = 0;
	unsigned long slp_ctl_addr = 0;

	if (rfspi_idx == 1)
		slp_ctl_addr = REG_INST2_CONN_WT_SLP_CTL_REG_ADDR;
	else
		slp_ctl_addr = REG_CONN_WT_SLP_CTL_REG_ADDR;

	if (type == CONNDRV_TYPE_CONNINFRA) {
		if (on)
			CONSYS_SET_BIT(slp_ctl_addr + WB_SLP_TOP_CK_0, 0x1);
		else
			CONSYS_CLR_BIT(slp_ctl_addr + WB_SLP_TOP_CK_0, 0x1);
		CONSYS_REG_BIT_POLLING(slp_ctl_addr + WB_SLP_TOP_CK_0, 1, 0, 100, 500, check);
		if (check == -1)
			pr_err("[type=%d][on=%d] op= fail\n", type, on);
	} else if (type == CONNDRV_TYPE_WIFI) {
		if (on)
			CONSYS_SET_BIT(slp_ctl_addr + WB_SLP_TOP_CK_1, 0x1);
		else
			CONSYS_CLR_BIT(slp_ctl_addr + WB_SLP_TOP_CK_1, 0x1);
		CONSYS_REG_BIT_POLLING(slp_ctl_addr + WB_SLP_TOP_CK_1, 1, 0, 100, 500, check);
		if (check == -1)
			pr_err("[type=%d][on=%d] op= fail\n", type, on);
	} else {
		pr_err("Not support for this consys drv type = %d\n", type);
		return -1;
	}

	return 0;
}

int consys_adie_top_ck_en_on_off_ctrl(enum consys_drv_type type, unsigned char on)
{
	int ret;

	if (consys_sema_acquire_timeout(CONN_SEMA_CONN_INFRA_COMMON_SYSRAM_INDEX, CONN_SEMA_TIMEOUT) == CONN_SEMA_GET_FAIL) {
		pr_err("[type=%d] acquire semaphore (%d) timeout\n",
				type, CONN_SEMA_CONN_INFRA_COMMON_SYSRAM_INDEX);
		return -1;
	}

	if (adie_cfg_type == ADIE_TYPE_TWO) {
		ret = _consys_adie_top_ck_en_on_off_ctrl(0, type, on);
		ret = _consys_adie_top_ck_en_on_off_ctrl(1, type, on);
	} else {
		if (one_adie_dbdc) {
			ret = _consys_adie_top_ck_en_on_off_ctrl(0, type, on);
		} else {
			ret = _consys_adie_top_ck_en_on_off_ctrl(1, type, on);
		}
	}

	consys_sema_release(CONN_SEMA_CONN_INFRA_COMMON_SYSRAM_INDEX);

	return ret;
}

int consys_conninfra_wf_wakeup(void)
{
	/* wake up conn_infra */
	CONSYS_SET_BIT(REG_CONN_HOST_CSR_TOP_ADDR + CONN_INFRA_WAKEPU_WF, 0x1);

	/* Wait 900us (apply this for CONNSYS XO clock ready) */
	udelay(900);

	/* Check CONNSYS version ID
	 * (polling "10 times" for specific project code and each polling interval is "1ms")
	 */
	if (consys_polling_chipid() != 0) {
		pr_err("Polling chip id fail\n");
		return -1;
	}

	return 0;
}

int consys_conninfra_wf_sleep(void)
{
	CONSYS_CLR_BIT(REG_CONN_HOST_CSR_TOP_ADDR + CONN_INFRA_WAKEPU_WF, 0x1);

	return 0;
}

int consys_conn_wmcpu_sw_reset(bool bassert)
{
	if (bassert)
		CONSYS_CLR_BIT(REG_CONN_INFRA_RGU_ADDR + WFSYS_CPU_SW_RST_B, 0x1);
	else
		CONSYS_SET_BIT(REG_CONN_INFRA_RGU_ADDR + WFSYS_CPU_SW_RST_B, 0x1);

	return 0;
}

int consys_wf_bus_slp_prot_ctrl(bool enable)
{
	/* Turn on/off "conn_infra to wfsys"/wfsys to conn_infra/wfdma2conn" bus sleep protect */

	if (enable)
		CONSYS_SET_BIT(REG_CONN_INFRA_CFG_ADDR + CONN_INFRA_WF_SLP_CTRL, 0x1);
	else
		CONSYS_CLR_BIT(REG_CONN_INFRA_CFG_ADDR + CONN_INFRA_WF_SLP_CTRL, 0x1);

	return 0;
}

int consys_wfsys_top_on_ctrl(bool enable)
{
	int check = 0;

	if (enable) {
		/* turn on wfsys_top_on */
		CONSYS_SET_BIT(REG_CONN_INFRA_RGU_ADDR + WFSYS_ON_TOP_PWR_CTL, 0x57460080);

		/* polling wfsys_rgu_off_hreset_rst_b */
		CONSYS_REG_BIT_POLLING(REG_CONN_HOST_CSR_TOP_ADDR + DBG_DUMMY_3, 30, 1, 100, 500, check);
		if (check == -1)
			pr_err("[%d] polling wfsys_rgu_off_hreset_rst_b fail\n", enable);
	} else {
		/* turn off wfsys_top_on */
		CONSYS_CLR_BIT_WITH_KEY(REG_CONN_INFRA_RGU_ADDR + WFSYS_ON_TOP_PWR_CTL, 0x80, 0x57460000);

		/* polling wfsys_rgu_off_hreset_rst_b */
		CONSYS_REG_BIT_POLLING(REG_CONN_HOST_CSR_TOP_ADDR + DBG_DUMMY_3, 30, 0, 100, 500, check);
		if (check == -1)
			pr_err("[%d] polling wfsys_rgu_off_hreset_rst_b fail\n", enable);
	}

	return 0;
}

int consys_wfsys_bus_slp_prot_check(bool enable)
{
	int check = 0;

	if (enable) {
		/* check "conn_infra to wfsys"/wfsys to conn_infra" bus sleep protect turn off */
		CONSYS_REG_BIT_POLLING(REG_CONN_INFRA_CFG_ADDR + CONN_INFRA_WF_SLP_STATUS, 29, 0, 100, 500, check);
		if (check == -1)
			pr_err("[bit %d] check conn_infra to wfsys or wfsys to conn_infra bus sleep protect turn off fail\n", 29);

		CONSYS_REG_BIT_POLLING(REG_CONN_INFRA_CFG_ADDR + CONN_INFRA_WF_SLP_STATUS, 31, 0, 100, 500, check);
		if (check == -1)
			pr_err("[bit %d] check conn_infra to wfsys or wfsys to conn_infra bus sleep protect turn off fail\n", 31);

		/* check WFDMA2CONN AXI TX bus sleep protect turn off */
		CONSYS_REG_BIT_POLLING(REG_WF_TOP_SLPPROT_ON_ADDR + WF_TOP_SLPPROT_ON_STATUS_READ, 23, 0, 100, 500, check);
		if (check == -1)
			pr_err("check WFDMA2CONN AXI TX bus sleep protect turn off fail\n");

		/* check WFDMA2CONN AXI RX bus sleep protect turn off */
		CONSYS_REG_BIT_POLLING(REG_WF_TOP_SLPPROT_ON_ADDR + WF_TOP_SLPPROT_ON_STATUS_READ, 21, 0, 100, 500, check);
		if (check == -1)
			pr_err("check WFDMA2CONN AXI RX bus sleep protect turn off fail\n");

		/* check WFSYS version ID */
		CONSYS_REG_POLLING_LARGER_OR_EQUAL(REG_WF_TOP_CFG_ADDR + WF_TOP_CFG_IP_VERSION, 0xFFFFFFFF, 0, 0x02060000, 10, 500, check);
		if (check == -1)
			pr_err("check WFSYS version ID fail\n");
	} else {
		/* check WFDMA2CONN AXI RX bus sleep protect turn on */
		CONSYS_REG_BIT_POLLING(REG_CONN_INFRA_CFG_ADDR + CONN_INFRA_WF_SLP_STATUS, 25, 1, 100, 500, check);
		if (check == -1)
			pr_err("check WFDMA2CONN AXI RX bus sleep protect turn on fail\n");

		/* check "conn_infra to wfsys"/wfsys to conn_infra" bus sleep protect turn on */
		CONSYS_REG_BIT_POLLING(REG_CONN_INFRA_CFG_ADDR + CONN_INFRA_WF_SLP_STATUS, 29, 1, 100, 500, check);
		if (check == -1)
			pr_err("[bit %d] check conn_infra to wfsys or wfsys to conn_infra bus sleep protect turn on fail\n", 29);

		CONSYS_REG_BIT_POLLING(REG_CONN_INFRA_CFG_ADDR + CONN_INFRA_WF_SLP_STATUS, 31, 1, 100, 500, check);
		if (check == -1)
			pr_err("[bit %d] check conn_infra to wfsys or wfsys to conn_infra bus sleep protect turn on fail\n", 31);
	}

	return 0;
}

int consys_wfsys_bus_timeout_ctrl(void)
{
	/* set wfsys bus timeout value (ahb apb timeout) */
	CONSYS_REG_WRITE_MASK(REG_WF_MCU_CONFIG_LS_ADDR + BUSHANGCR, 0x1, 0xFF);

	/* enable wfsys bus timeout (ahb apb timeout) */
	CONSYS_SET_BIT(REG_WF_MCU_CONFIG_LS_ADDR + BUSHANGCR, 0x90000000);

	/* set conn2wf remapping window to wf debug_ctrl_ao CR */
	CONSYS_REG_WRITE(REG_WF_MCU_BUS_CR_ADDR + AP2WF_REMAP_1, 0x810F0000);

	/* set wfsys bus timeout value (debug ctrl ao) */
	CONSYS_REG_WRITE_MASK(REG_WF_MCUSYS_INFRA_BUS_FULL_U_DEBUG_CTRL_AO_ADDR + WF_MCUSYS_INFRA_BUS_FULL_U_DEBUG_CTRL_AO_WFMCU_PWA_CTRL0,
							0x03AA0000, 0xFFFF0000);

	/* enable wfsys bus timeout (debug ctrl ao) */
	CONSYS_SET_BIT(REG_WF_MCUSYS_INFRA_BUS_FULL_U_DEBUG_CTRL_AO_ADDR + WF_MCUSYS_INFRA_BUS_FULL_U_DEBUG_CTRL_AO_WFMCU_PWA_CTRL0, 0xC);

	return 0;
}

int consys_wmcpu_idle_loop_check(void)
{
	int check = 0;

	/* check CONNSYS power-on completion */
	CONSYS_REG_POLLING_EQUAL(REG_WF_TOP_CFG_ON_ADDR + ROMCODE_INDEX, 0xFFFFFFFF, 0, 0x1D1E, 5000, 1000, check);
	if (check == -1)
		pr_err("check CONNSYS power-on completion fail\n");

	return 0;
}

void _consys_check_sku_cfg(void)
{
	unsigned int hw_sku_type = 0;

	if (one_adie_dbdc) {
		if (adie_cfg_type == ADIE_TYPE_ONE)
			hw_sku_type = 3000;
	}

	if (hw_sku_type)
		printk(GRN("SKU Type: %d"), hw_sku_type);
	else
		printk(GRN("Unknown SKU Type\n"));
}

int consys_plt_adie_type_cfg(void)
{
	/*
		If One_Adie_DB:
			then TOP_MISC_CR (0x11D1_021C[31:28]) = 0x7 && 0x18050000 = 0x7
	*/

	if (one_adie_dbdc) {
		if (adie_cfg_type == ADIE_TYPE_ONE) {
			CONSYS_REG_WRITE_MASK(REG_TOP_MISC_ADDR + TOP_MISC_RSRV_ALL1_3, 0x70000000, 0xF0000000);
			CONSYS_REG_WRITE(REG_CONN_INFRA_SYSRAM_ADDR + SYSRAM_BASE_ADDR, 0x7);
		}
	}

	if (_consys_check_adie_cfg() == 0)
		_consys_check_sku_cfg();

	return 0;
}

int consys_wpll_ctrl(bool enable)
{
	if (enable) {
		/* turn back wpll setting in conn_afe_ctl by setting wpll initial control to 2'b10 */
		CONSYS_REG_WRITE_MASK(REG_CONN_AFE_CTL_ADDR + RG_DIG_EN_02, 0x20002, 0x30003);
	} else {
		/* Don't need below code check anymore due to new design */
#if 0
		int check = 0;
		/* trun off wpll enable in conn_afe_ctl by setting wpll initial control to 2'b00 */
		CONSYS_REG_WRITE_MASK(REG_CONN_AFE_CTL_ADDR + RG_DIG_EN_02, 0x0, 0x30003);

		/* polling conn_infra bus to non-wpll case */
		CONSYS_REG_POLLING_EQUAL(REG_CONN_INFRA_CLKGEN_ON_TOP_ADDR + CKGEN_BUS, 0x7800, 11, 0x0, 5000, 1000, check);
		if (check == -1)
			pr_err("polling conn_infra bus to non-wpll case fail\n");
#endif
	}
	return 0;
}

int consys_conninfra_wf_req_clr(void)
{
	/* clear wf_emi_req */
	CONSYS_SET_BIT(REG_CONN_INFRA_CFG_ADDR + EMI_CTL_WF, 0x1);
	CONSYS_CLR_BIT(REG_CONN_INFRA_CFG_ADDR + EMI_CTL_WF, 0x1);

	/* clear wf_infra_req */
	CONSYS_SET_BIT(REG_CONN_INFRA_CFG_ADDR + EMI_CTL_WF, 0x20);
	CONSYS_CLR_BIT(REG_CONN_INFRA_CFG_ADDR + EMI_CTL_WF, 0x20);

	return 0;
}

