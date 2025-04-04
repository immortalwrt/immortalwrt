/*
 * Copyright (c) [2020], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/
/*
 ***************************************************************************
 ***************************************************************************

	Module Name:
	rtmp_chip.h

	Abstract:
	Ralink Wireless Chip related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef	__RTMP_CHIP_H__
#define	__RTMP_CHIP_H__

#include "rtmp_type.h"
#include "eeprom.h"
#include "tx_power.h"
#include "hif/hif.h"
#include "mac/mac.h"
#include "mcu/mcu.h"
#ifdef MT7915_MT7916_COEXIST_COMPATIBLE
#include "chip/common.h"
#else
#if defined(MT7986) || defined(MT7916) || defined(MT7981)
#include "chip/common.h"
#endif
#endif












#ifdef MT7986
#include "chip/mt7986.h"
#endif

#ifdef MT7916
#include "chip/mt7916.h"
#endif

#ifdef MT7981
#include "chip/mt7981.h"
#endif

#include "hw_ctrl/hw_init.h"

#define IS_MT7650(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76500000)
#define IS_MT7630(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76300000)

#define IS_MT7662(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76620000)
#define IS_MT7612(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76120000)
#define IS_MT7602(_pAd)		(((_pAd)->ChipID & 0xffff0000) == 0x76020000)
#define REV_MT76x2E3        0x0022
#define REV_MT76x2E4        0x0033

#define IS_MT76xx(_pAd)		(IS_MT76x0(_pAd))


#define IS_MT7603(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007603)
#define IS_MT7603E(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007603) && IS_PCIE_INF(_pAd))
#define IS_MT7603U(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007603) && IS_USB_INF(_pAd) && (_pAd->AntMode == 0))
#define IS_MT76031U(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007603) && IS_USB_INF(_pAd) && (_pAd->AntMode == 1))

#define MT7603E1 0x0000
#define MT7603E2 0x0010

/*
#define IS_MT7636(_pAd)     ((((_pAd)->ChipID & 0x0000ffff) == 0x00007606)||\
							(((_pAd)->ChipID & 0x0000ffff) == 0x00007636) || \
							(((_pAd)->ChipID & 0x0000ffff) == 0x00007611))
*/

/*	MT76x6 series: 7636/7606/7611
	1. For definition/compiler option, use MT7636 for all 7636/7603/7611
	2. For run time case (specific H/W), please use IS_MT7636(), IS_MT7606(), MT7611()
	3. For run time case (all H/Ws), please use IS_MT76x6()
	4. For different interface, please use IS_MT7636U(), IS_MT7636S() */
#define IS_MT76x6(_pAd)     ((((_pAd)->ChipID & 0x0000ffff) == 0x00007606) || \
							 (((_pAd)->ChipID & 0x0000ffff) == 0x00007636) || \
							 (((_pAd)->ChipID & 0x0000ffff) == 0x00007611))

#define IS_MT7636U(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007606) && IS_USB_INF(_pAd))
#define IS_MT7636S(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007606) && IS_SDIO_INF(_pAd))

#define IS_MT7636(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007636)
#define IS_MT7606(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007606)
#define IS_MT7611(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007611)

/* MT7636 Hardeware Version */
#define MT76x6E1 0x0000		/* MT7636 E1	0x8A00 */
#define MT76x6E2 0x0001		/* MT7636 E2 0x8A01 */
#define MT76x6E3 0x0010		/* MT7636 E3 0x8A10 */

/* MT7636 Firmware Version */
#define MT76x6_FW_VER_E1		0x00		/* MT7636 FW Ver E1	0x8A00, use E1 rom patch */
#define MT76x6_FW_VER_E3		0x10		/* MT7636 FW Ver E3	0x8A10, use E3 rom patch */

#define IS_MT76x6_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT76x6_FW_VER_E1)
#define IS_MT76x6_FW_VER_E3(_pAd)		(((_pAd)->FWVersion & 0xff) == MT76x6_FW_VER_E3)


/*	MT7637 series: TBD */
#define IS_MT7637(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007637)
#define IS_MT7637U(_pAd)	(IS_MT7637(_pAd) && IS_USB_INF(_pAd))
#define IS_MT7637S(_pAd)	(IS_MT7637(_pAd) && IS_SDIO_INF(_pAd))
#define IS_MT7637E(_pAd)	(IS_MT7637(_pAd) && IS_PCIE_INF(_pAd))


/* MT7637 Hardeware Version */
#define MT7637E1 0x0000		/* MT7637 E1	0x8A00 */
#define MT7637E2 0x0010		/* MT7637 E2 0x8A10 */

/* MT7637 Firmware Version */
#define MT7637_FW_VER_E1		0x00		/* MT7637 FW Ver E1	0x8A00, use E1 rom patch */
#define MT7637_FW_VER_E2		0x10		/* MT7637 FW Ver E2	0x8A10, use E2 rom patch */

#define IS_MT7637_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7637_FW_VER_E1)
#define IS_MT7637_FW_VER_E2(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7637_FW_VER_E2)

/* MT7615 Firmware Version */
#define MT7615_FW_VER_E1		0x00		/* MT7615 FW Ver E1	0x8A00, use E1 RAM IMAGE */
#define MT7615_FW_VER_E3		0x10		/* MT7615 FW Ver E3	0x8A10, use E3 RAM IMAGE */

#define IS_MT7615_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7615_FW_VER_E1)
#define IS_MT7615_FW_VER_E3(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7615_FW_VER_E3)

/* MT7622 Firmware Version */
#define MT7622_FW_VER_E1		0x00		/* MT7622 FW Ver E1	0x8A00, use E1 RAM IMAGE */
#define MT7622_FW_VER_E2		0x10		/* MT7622 FW Ver E2	0x8A10, use E2 RAM IMAGE */

#define IS_MT7622_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7622_FW_VER_E1)
#define IS_MT7622_FW_VER_E2(_pAd)		(((_pAd)->FWVersion & 0xff) >= MT7622_FW_VER_E2)

/* P18 Firmware Version */
#define P18_FW_VER_E1		0x00		/* P18 FW Ver E1	0x8A00, use E1 RAM IMAGE */

#define IS_P18_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == P18_FW_VER_E1)

/* MT7663 Firmware Version */
#define MT7663_FW_VER_E1		0x00		/* MT7663 FW Ver E1	0x8A00, use E1 RAM IMAGE */
#define MT7663_FW_VER_E2		0x01		/* MT7663 FW Ver E2	0x8A01, use E2 RAM IMAGE */

#define IS_MT7663_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7663_FW_VER_E1)
#define IS_MT7663_FW_VER_E2(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7663_FW_VER_E2)

/* AXE Firmware Version */
#define AXE_FW_VER_E1		0x00		/* AXE FW Ver E1	0x8A00, use E1 RAM IMAGE */

#define IS_AXE_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == AXE_FW_VER_E1)

/* MT7626 Firmware Version */
#define MT7626_FW_VER_E1		0x00		/* MT7626 FW Ver E1	0x8A00, use E1 RAM IMAGE */

#define IS_MT7626_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7626_FW_VER_E1)

#define MT7626_FW_VER_E2		0x01		/* MT7626 FW Ver E1	0x8A01, use E1 RAM IMAGE */

#define IS_MT7626_FW_VER_E2(_pAd)		(((_pAd)->FWVersion & 0xff) >= MT7626_FW_VER_E2)


/* MT7915 Firmware Version */
#define MT7915_FW_VER_E1		0x00		/* MT7915 FW Ver E1	0x8A00 */
#define MT7915_FW_VER_E2		0x01		/* MT7915 FW Ver E2	0x8A01 */


#define IS_MT7915_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7915_FW_VER_E1)
#define IS_MT7915_FW_VER_E2(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7915_FW_VER_E2)

/* MT7986 Firmware Version */
#define MT7986_FW_VER_E1		0x00		/* MT7986 FW Ver E1	0x8A00 */

/* MT7916 Firmware Version */
#define MT7916_FW_VER_E1		0x00		/* MT7916 FW Ver E1	0x8A00 */

#define IS_MT7986_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7986_FW_VER_E1)

#define IS_MT7916_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7916_FW_VER_E1)

/* MT7981 Firmware Version */
#define MT7981_FW_VER_E1		0x00		/* MT7981 FW Ver E1	0x8A00 */

#define IS_MT7981_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7981_FW_VER_E1)

#define IS_MT7628(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007628)

#define MT7628E1 0x0000
#define MT7628E2 0x0010

#define IS_MT7615(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007615)

#define MT7615E1 0x0000       /* MT7615 E1 0x8A00 */
#define MT7615E2 0x0001       /* MT7615 E2 0x8A01 */
#define MT7615E3 0x0010       /* MT7615 E3 0x8A10 */

#define IS_MT7622(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007622)

#define IS_P18(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00006632) /* 0x6632 is for FPGA release v10 */
#define IS_MT7663(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007663)

#define IS_AXE(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00006867)
#define IS_MT7626(_pAd)		((((_pAd)->ChipID & 0x0000ffff) == 0x00007626))
#define IS_MT7915(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007915)

#define IS_MT7915_6E(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007915) \
							&& ((_pAd)->Wifi6gCap == 1))

#define IS_MT7986(_pAd)		((((_pAd)->ChipID & 0x0000ffff) == 0x00007986) || \
							(((_pAd)->ChipID & 0x0000ffff) == 0x00000789))

#define IS_MT7916(_pAd)		((((_pAd)->ChipID & 0x0000ffff) == 0x00007906))

#define IS_MT7981(_pAd)		((((_pAd)->ChipID & 0x0000ffff) == 0x00007981) || \
							(((_pAd)->ChipID & 0x0000ffff) == 0x00000790))

#define MT7915E1 0x0000
#define MT7915E2 0x0010

#define MT7916E1 0x0000
#define MT7916E2 0x0002

#define MT7986E1 0x0000
#define MT7986E2 0x0001

#define IS_SUPPORT_ATE_TX_BY_TIME(_pAd) \
	(IS_MT7622(_pAd) || IS_MT7615(_pAd) || IS_MT7663(_pAd) || IS_P18(_pAd) || \
	 IS_AXE(_pAd) || IS_MT7915(_pAd) || IS_MT7986(_pAd) || IS_MT7916(_pAd) || \
	 IS_MT7981(_pAd))

/* RT3592BC8 (WiFi + BT) */
#define RT_REV_LT(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->MACVersion & 0x0000FFFF) < (_rev)))

#define RT_REV_GTE(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->MACVersion & 0x0000FFFF) >= (_rev)))

#define MT_REV_LT(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->ChipID & 0x0000FFFF) < (_rev)))

#define MT_REV_GTE(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->ChipID & 0x0000FFFF) >= (_rev)))

#define MT_REV_ET(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->ChipID & 0x0000FFFF) == (_rev)))

#define MTK_REV_LT(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->HWVersion & 0x000000ff) < (_rev)))

#define MTK_REV_GTE(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->HWVersion & 0x000000ff) >= (_rev)))

#define MTK_REV_ET(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->HWVersion & 0x000000ff) == (_rev)))

/* Dual-band NIC (RF/BBP/MAC are in the same chip.) */

#define IS_RT_NEW_DUAL_BAND_NIC(_pAd) ((FALSE))

#define CCA_AVG_MAX_COUNT	5

/* ------------------------------------------------------ */
/* PCI registers - base address 0x0000 */
/* ------------------------------------------------------ */
#define CHIP_PCI_CFG		0x0000
#define CHIP_PCI_EECTRL		0x0004
#define CHIP_PCI_MCUCTRL	0x0008

#define OPT_14			0x114

#define RETRY_LIMIT		10

/* ------------------------------------------------------ */
/* BBP & RF	definition */
/* ------------------------------------------------------ */
#define	BUSY		                1
#define	IDLE		                0

/*------------------------------------------------------------------------- */
/* EEPROM definition */
/*------------------------------------------------------------------------- */
#define EEDO                        0x08
#define EEDI                        0x04
#define EECS                        0x02
#define EESK                        0x01
#define EERL                        0x80

#define EEPROM_WRITE_OPCODE         0x05
#define EEPROM_READ_OPCODE          0x06
#define EEPROM_EWDS_OPCODE          0x10
#define EEPROM_EWEN_OPCODE          0x13

#define NUM_EEPROM_BBP_PARMS		19	/* Include NIC Config 0, 1, CR, TX ALC step, BBPs */
#define NUM_EEPROM_TX_G_PARMS		7

#define VALID_EEPROM_VERSION        1
#define EEPROM_VERSION_OFFSET       0x02
#define EEPROM_NIC1_OFFSET          0x34	/* The address is from NIC config 0, not BBP register ID */
#define EEPROM_NIC2_OFFSET          0x36	/* The address is from NIC config 1, not BBP register ID */

#define EEPROM_COUNTRY_REGION			0x38
#define COUNTRY_REGION_A_BAND_MASK (0xff)
#define COUNTRY_REGION_G_BAND (0xff << 8)

#define EEPROM_DEFINE_MAX_TXPWR			0x4e
#define MAX_EIRP_TX_PWR_G_BAND_MASK (0xff)
#define MAX_EIRP_TX_PWR_A_BAND_MASK (0xff << 8)

#define EEPROM_FREQ_OFFSET			0x3a
#define FREQ_OFFSET_MASK (0x7f)
#define FREQ_OFFSET_DIP (1 << 7)
#define LED_MODE_MASK (0xff << 8)

#define EEPROM_LEDAG_CONF_OFFSET	0x3c
#define EEPROM_LEDACT_CONF_OFFSET	0x3e
#define EEPROM_LED_POLARITY_OFFSET	0x40

#define EEPROM_LNA_OFFSET			0x44
#define LNA_GAIN_G_BAND_MASK (0x7f)
#define LNA_GAIN_G_BAND_EN (1 << 7)
#define LNA_GAIN_A_BAND_CH36_64_MASK (0x7f << 8)
#define LNA_GAIN_A_BAND_CH36_64_EN (1 << 15)

#define EEPROM_RSSI_BG_OFFSET			0x46
#define RSSI0_OFFSET_G_BAND_MASK (0x3f)
#define RSSI0_OFFSET_G_BAND_SIGN (1 << 6)
#define RSSI0_OFFSET_G_BAND_EN (1 << 7)
#define RSSI1_OFFSET_G_BAND_MASK (0x3f << 8)
#define RSSI1_OFFSET_G_BAND_SIGN (1 << 14)
#define RSSI1_OFFSET_G_BAND_EN (1 << 15)

#define EEPROM_TXMIXER_GAIN_2_4G		0x48
#define LNA_GAIN_A_BAND_CH100_128_MASK (0x7f << 8)
#define LNA_GAIN_A_BAND_CH100_128_EN (1 << 15)

#define EEPROM_RSSI_A_OFFSET			0x4a
#define RSSI0_OFFSET_A_BAND_MASK (0x3f)
#define RSSI0_OFFSET_A_BAND_SIGN (1 << 6)
#define RSSI0_OFFSET_A_BANE_EN (1 << 7)
#define RSSI1_OFFSET_A_BAND_MASK (0x3f << 8)
#define RSSI1_OFFSET_A_BAND_SIGN (1 << 14)
#define RSSI1_OFFSET_A_BAND_EN (1 << 15)

#define EEPROM_TXMIXER_GAIN_5G			0x4c
#define LNA_GAIN_A_BAND_CH132_165_MASK (0x7f << 8)
#define LNA_GAIN_A_BAND_CH132_165_EN (1 << 15)

#define EEPROM_TXPOWER_DELTA			0x50	/* 20MHZ AND 40 MHZ use different power. This is delta in 40MHZ. */

#define EEPROM_G_TX_PWR_OFFSET			0x52
#define EEPROM_G_TX2_PWR_OFFSET			0x60

#define EEPROM_G_TSSI_BOUND1			0x6e
#define EEPROM_G_TSSI_BOUND2			0x70
#define EEPROM_G_TSSI_BOUND3			0x72
#define EEPROM_G_TSSI_BOUND4			0x74
#define EEPROM_G_TSSI_BOUND5			0x76

#define EEPROM_A_TX_PWR_OFFSET			0x78
#define EEPROM_A_TX2_PWR_OFFSET			0xa6


enum FREQ_CAL_INIT_MODE {
	FREQ_CAL_INIT_MODE0,
	FREQ_CAL_INIT_MODE1,
	FREQ_CAL_INIT_MODE2,
	FREQ_CAL_INIT_UNKNOW,
};

enum FREQ_CAL_MODE {
	FREQ_CAL_MODE0,
	FREQ_CAL_MODE1,
	FREQ_CAL_MODE2,
};

enum RXWI_FRQ_OFFSET_FIELD {
	RXWI_FRQ_OFFSET_FIELD0, /* SNR1 */
	RXWI_FRQ_OFFSET_FIELD1, /* Frequency Offset */
};

#define EEPROM_A_TSSI_BOUND1		0xd4
#define EEPROM_A_TSSI_BOUND2		0xd6
#define EEPROM_A_TSSI_BOUND3		0xd8
#define EEPROM_A_TSSI_BOUND4		0xda
#define EEPROM_A_TSSI_BOUND5		0xdc

#define EEPROM_TXPOWER_BYRATE			0xde	/* 20MHZ power. */
#define EEPROM_TXPOWER_BYRATE_20MHZ_2_4G	0xde	/* 20MHZ 2.4G tx power. */
#define EEPROM_TXPOWER_BYRATE_40MHZ_2_4G	0xee	/* 40MHZ 2.4G tx power. */
#define EEPROM_TXPOWER_BYRATE_20MHZ_5G		0xfa	/* 20MHZ 5G tx power. */
#define EEPROM_TXPOWER_BYRATE_40MHZ_5G		0x10a	/* 40MHZ 5G tx power. */

#define EEPROM_BBP_BASE_OFFSET			0xf0	/* The address is from NIC config 0, not BBP register ID */

/* */
/* Bit mask for the Tx ALC and the Tx fine power control */
/* */
#define GET_TX_ALC_BIT_MASK					0x1F	/* Valid: 0~31, and in 0.5dB step */
#define GET_TX_FINE_POWER_CTRL_BIT_MASK	0xE0	/* Valid: 0~4, and in 0.1dB step */
#define NUMBER_OF_BITS_FOR_TX_ALC			5	/* The length, in bit, of the Tx ALC field */


/* TSSI gain and TSSI attenuation */

#define EEPROM_TSSI_GAIN_AND_ATTENUATION	0x76

/*#define EEPROM_Japan_TX_PWR_OFFSET      0x90 // 802.11j */
/*#define EEPROM_Japan_TX2_PWR_OFFSET      0xbe */
/*#define EEPROM_TSSI_REF_OFFSET	0x54 */
/*#define EEPROM_TSSI_DELTA_OFFSET	0x24 */
/*#define EEPROM_CCK_TX_PWR_OFFSET  0x62 */
/*#define EEPROM_CALIBRATE_OFFSET	0x7c */

#define EEPROM_NIC_CFG1_OFFSET		0
#define EEPROM_NIC_CFG2_OFFSET		1
#define EEPROM_NIC_CFG3_OFFSET		2
#define EEPROM_COUNTRY_REG_OFFSET	3
#define EEPROM_BBP_ARRAY_OFFSET		4

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_ANTENNA_STRUC {
	struct {
		USHORT RssiIndicationMode:1;	/* RSSI indication mode */
		USHORT Rsv:1;
		USHORT BoardType:2;		/* 0: mini card; 1: USB pen */
		USHORT RfIcType:4;		/* see E2PROM document */
		USHORT TxPath:4;		/* 1: 1T, 2: 2T, 3: 3T */
		USHORT RxPath:4;		/* 1: 1R, 2: 2R, 3: 3R */
	} field;
	USHORT word;
} EEPROM_ANTENNA_STRUC, *PEEPROM_ANTENNA_STRUC;
#else
typedef union _EEPROM_ANTENNA_STRUC {
	struct {
		USHORT RxPath:4;		/* 1: 1R, 2: 2R, 3: 3R */
		USHORT TxPath:4;		/* 1: 1T, 2: 2T, 3: 3T */
		USHORT RfIcType:4;		/* see E2PROM document */
		USHORT BoardType:2;		/* 0: mini card; 1: USB pen */
		USHORT Rsv:1;
		USHORT RssiIndicationMode:1;	/* RSSI indication mode */
	} field;
	USHORT word;
} EEPROM_ANTENNA_STRUC, *PEEPROM_ANTENNA_STRUC;
#endif

/* rxv double data word low/high cycle */
#define RXV_DDW0_L     0
#define RXV_DDW0_H     1
#define RXV_DDW1_L     2
#define RXV_DDW1_H     3
#define RXV_DDW2_L     4
#define RXV_DDW2_H     5
#define RXV_DDW3_L     6
#define RXV_DDW3_H     7
#define RXV_DDW4_L     8
#define RXV_DDW4_H     9
#define RXV_DDW5_L     10
#define RXV_DDW5_H     11
#define RXV_DDW6_L     12
#define RXV_DDW6_H     13
#define RXV_DDW7_L     14
#define RXV_DDW7_H     15
#define RXV_DDW8_L     16
#define RXV_DDW8_H     17
#define RXV_DDW9_L     18
#define RXV_DDW9_H     19
#define RXV_DDW10_L    20
#define RXV_DDW10_H    21

#define MUM_VHT_4MU_GRP_NUM 5

#define TSSI_CAL_2G_SIZE  (9 * 4)
#define TSSI_CAL_5G_SIZE  (48 * 4)
#define DNL_CAL_SIZE      (158 * 4)
#define RXGAIN_CAL_SIZE      (120 * 4)

#define TSSI_CAL_2G_EEPROM_OFST  0x308
#define TSSI_CAL_5G_EEPROM_OFST  0x380
#define TX_DNL_CAL_EEPROM_OFST   0xAFC

/* RF LOCKDOWN */
#define COUNTRY_CODE_BYTE0_EEPROME_OFFSET   0x5C
#define COUNTRY_CODE_BYTE1_EEPROME_OFFSET   0x5D
#define COUNTRY_REGION_2G_EEPROME_OFFSET    0x5E
#define COUNTRY_REGION_5G_EEPROME_OFFSET    0x5F
#define COUNTRY_REGION_VALIDATION_MASK      BIT(7)
#define COUNTRY_REGION_VALIDATION_OFFSET    7
#define COUNTRY_REGION_CONTENT_MASK         BITS(0, 6)
#define RF_LOCKDOWN_EEPROME_BLOCK_OFFSET    0x50
#define RF_LOCKDOWN_EEPROME_COLUMN_OFFSET   0xB
#define RF_LOCKDOWN_EEPROME_OFFSET          0x5B
#define RF_LOCKDOWN_EEPROME_MASK            BIT(7)
#define RF_LOCKDOWN_EEPROME_SHIFT           7
#define RF_LOCKDOWN_EEPROME_SKU_TBL_OFFSET  0x5B
#define RF_LOCKDOWN_EEPROME_SKU_TBL_MASK    BITS(0, 6)
#define RF_LOCKDOWN_EEPROME_SKU_TBL_SHIFT   0
#define RF_VALIDATION_NUMBER                10

/** Tx Power Rate Column Offset */
#define TXPOWER_RATE_CCK_OFFSET     (0)
#define TXPOWER_RATE_OFDM_OFFSET    (TXPOWER_RATE_CCK_OFFSET  + MODULATION_SYSTEM_CCK_NUM)
#define TXPOWER_RATE_HT20_OFFSET    (TXPOWER_RATE_OFDM_OFFSET + MODULATION_SYSTEM_OFDM_NUM)
#define TXPOWER_RATE_HT40_OFFSET    (TXPOWER_RATE_HT20_OFFSET + MODULATION_SYSTEM_HT20_NUM)
#define TXPOWER_RATE_VHT20_OFFSET   (TXPOWER_RATE_HT40_OFFSET + MODULATION_SYSTEM_HT40_NUM)
#define TXPOWER_RATE_VHT40_OFFSET   (TXPOWER_RATE_VHT20_OFFSET + MODULATION_SYSTEM_VHT20_NUM)
#define TXPOWER_RATE_VHT80_OFFSET   (TXPOWER_RATE_VHT40_OFFSET + MODULATION_SYSTEM_VHT40_NUM)
#define TXPOWER_RATE_VHT160_OFFSET  (TXPOWER_RATE_VHT80_OFFSET + MODULATION_SYSTEM_VHT80_NUM)
#define TXPOWER_RATE_LEG_RATE_NUM   (TXPOWER_RATE_VHT160_OFFSET + MODULATION_SYSTEM_VHT160_NUM)
#define TXPOWER_RATE_HE26_OFFSET    (TXPOWER_RATE_LEG_RATE_NUM)
#define TXPOWER_RATE_HE52_OFFSET    (TXPOWER_RATE_HE26_OFFSET + MODULATION_SYSTEM_HE26_NUM)
#define TXPOWER_RATE_HE106_OFFSET   (TXPOWER_RATE_HE52_OFFSET + MODULATION_SYSTEM_HE52_NUM)
#define TXPOWER_RATE_HE242_OFFSET   (TXPOWER_RATE_HE106_OFFSET + MODULATION_SYSTEM_HE106_NUM)
#define TXPOWER_RATE_HE484_OFFSET   (TXPOWER_RATE_HE242_OFFSET + MODULATION_SYSTEM_HE242_NUM)
#define TXPOWER_RATE_HE996_OFFSET   (TXPOWER_RATE_HE484_OFFSET + MODULATION_SYSTEM_HE484_NUM)
#define TXPOWER_RATE_HE996X2_OFFSET (TXPOWER_RATE_HE996_OFFSET + MODULATION_SYSTEM_HE996_NUM)
#define TXPOWER_RATE_NUM            (TXPOWER_RATE_HE996X2_OFFSET + MODULATION_SYSTEM_HE996X2_NUM)


#if defined(PRE_CAL_MT7986_SUPPORT) || defined(PRE_CAL_MT7916_SUPPORT) || \
	defined(PRE_CAL_MT7981_SUPPORT) || defined(PRE_CAL_MT7915_SUPPORT)

#ifdef RTMP_FLASH_SUPPORT
enum PRECAL_INDICATION_BIT_OFFSET {
	GROUP_PRECAL_INDN_BIT = 0,
	DPD5G_PRECAL_INDN_BIT,
	DPD2G_PRECAL_INDN_BIT,
	DPD6G_PRECAL_INDN_BIT,
};
#endif

enum {
    GBAND = 0,
    ABAND = 1,
};

/* Length limitation from Host to Firmware */
#define PRE_CAL_SET_MAX_CR_NUM   256
#define PRE_CAL_SET_MAX_LENGTH   (PRE_CAL_SET_MAX_CR_NUM * 4)

#endif

#if defined(CONFIG_WLAN_SERVICE)
enum {
	RX_FE_LOSS_2G = 0,
	RX_FE_LOSS_5G_LB,
	RX_FE_LOSS_5G_MB,
	RX_FE_LOSS_5G_HB,
	RX_FE_LOSS_6G_LB1 = 4,
	RX_FE_LOSS_6G_LB2,
	RX_FE_LOSS_6G_HB1,
	RX_FE_LOSS_5G_HB2,
	RX_FE_LOSS_MAX_NUM,
};

struct _rx_fe_loss_ee_def {
	UINT16 ee_of[RX_FE_LOSS_MAX_NUM * 4];
};
#endif

struct BUS_DEBUG_INFO {
	PCHAR  reg_name;
	UINT32 reg_address;
};



struct RF_BANK_OFFSET {
	UINT8 RFBankIndex;
	UINT16 RFStart;
	UINT16 RFEnd;
};

struct RF_INDEX_OFFSET {
	UINT8 RFIndex;
	UINT16 RFStart;
	UINT16 RFEnd;
};


struct _RSSI_SAMPLE;
union _EXT_CMD_EFUSE_BUFFER_MODE_T;

int RtmpChipOpsHook(VOID *pCB);

UINT8 NICGetBandSupported(struct _RTMP_ADAPTER *pAd);

VOID RtmpChipWriteMemory(
	IN	struct _RTMP_ADAPTER *pAd,
	IN	USHORT			Offset,
	IN	UINT32			Value,
	IN	UINT8			Unit);

VOID RTMPReadChannelPwr(struct _RTMP_ADAPTER *pAd);
VOID RTMPReadTxPwrPerRate(struct _RTMP_ADAPTER *pAd);

INT tx_pwr_comp_init(struct _RTMP_ADAPTER *pAd);
VOID NetDevNickNameInit(IN struct _RTMP_ADAPTER *pAd);

#ifdef GREENAP_SUPPORT
struct greenap_ctrl;
struct greenap_on_off_ctrl;
VOID greenap_init(
	struct _RTMP_ADAPTER *ad);
VOID greenap_show(
	struct _RTMP_ADAPTER *ad);
BOOLEAN greenap_check_when_if_down_up(
	struct _RTMP_ADAPTER *ad);
BOOLEAN greenap_check_when_ap_bss_change(
	struct _RTMP_ADAPTER *ad);
VOID greenap_check_peer_connection_at_link_up_down(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev);
VOID greenap_suspend(
	struct _RTMP_ADAPTER *ad,
	UINT32 reason);
VOID greenap_resume(
	struct _RTMP_ADAPTER *ad,
	UINT32 reason);
VOID greenap_set_capability(
	struct _RTMP_ADAPTER *ad,
	BOOLEAN greenap_cap);
BOOLEAN greenap_get_capability(
	struct _RTMP_ADAPTER *ad);
VOID greenap_proc(
	struct _RTMP_ADAPTER *ad,
	BOOLEAN greenap_cap_on);

VOID enable_greenap(
	struct _RTMP_ADAPTER *ad,
	struct greenap_on_off_ctrl *greenap_on_off);
VOID disable_greenap(
	struct _RTMP_ADAPTER *ad,
	struct greenap_on_off_ctrl *greenap_on_off);
VOID EnableAPMIMOPSv2(struct _RTMP_ADAPTER *ad, struct greenap_on_off_ctrl *greenap_on_off);
VOID DisableAPMIMOPSv2(struct _RTMP_ADAPTER *ad, struct greenap_on_off_ctrl *greenap_on_off);
VOID EnableAPMIMOPSv1(struct _RTMP_ADAPTER *ad, struct greenap_on_off_ctrl *greenap_on_off);
VOID DisableAPMIMOPSv1(struct _RTMP_ADAPTER *ad, struct greenap_on_off_ctrl *greenap_on_off);
#endif /* GREENAP_SUPPORT */

/* global variable */
extern FREQUENCY_ITEM RtmpFreqItems3020[];
extern FREQUENCY_ITEM FreqItems3020_Xtal20M[];
extern UCHAR NUM_OF_3020_CHNL;
extern FREQUENCY_ITEM *FreqItems3020;
extern RTMP_RF_REGS RF2850RegTable[];
extern UCHAR NUM_OF_2850_CHNL;

INT AsicGetMacVersion(struct _RTMP_ADAPTER *pAd);

INT WaitForAsicReady(struct _RTMP_ADAPTER *pAd);

#define ASIC_MAC_TX			1
#define ASIC_MAC_RX			2
#define ASIC_MAC_TXRX		3
#define ASIC_MAC_TXRX_RXV	4
#define ASIC_MAC_RXV		5
#define ASIC_MAC_RX_RXV		6

INT AsicSetMacTxRx(struct _RTMP_ADAPTER *pAd, INT32 txrx, BOOLEAN enable);

INT AsicSetMacWD(struct _RTMP_ADAPTER *pAd);

INT AsicReadAggCnt(struct _RTMP_ADAPTER *pAd, ULONG *aggCnt, int cnt_len);
#ifdef MT_MAC
INT rt28xx_read16(struct _RTMP_ADAPTER *pAd, USHORT, USHORT *);
#endif /* MT_MAC */

#endif /* __RTMP_CHIP_H__ */

