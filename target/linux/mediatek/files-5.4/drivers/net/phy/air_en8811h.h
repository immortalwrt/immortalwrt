/* SPDX-License-Identifier: GPL-2.0 */
/*************************************************
 * FILE NAME:  air_en8811h.h
 * PURPOSE:
 *      EN8811H PHY Driver for Linux
 * NOTES:
 *
 *  Copyright (C) 2023 Airoha Technology Corp.
 *************************************************/
#ifndef __EN8811H_H
#define __EN8811H_H

#define EN8811H_MD32_DM             "EthMD32.dm.bin"
#define EN8811H_MD32_DSP            "EthMD32.DSP.bin"
#define EN8811H_IVY		            "ivypram.bin"

#define EN8811H_PHY_ID1             0x03a2
#define EN8811H_PHY_ID2             0xa411
#define EN8811H_PHY_ID              ((EN8811H_PHY_ID1 << 16) | EN8811H_PHY_ID2)
#define EN8811H_PHY_READY           0x02
#define EN8811H_PHY_IVY_READY		0xABC
#define MAX_RETRY                   25

#define EN8811H_TX_POL_NORMAL   0x1
#define EN8811H_TX_POL_REVERSE  0x0

#define EN8811H_RX_POL_NORMAL   (0x0 << 1)
#define EN8811H_RX_POL_REVERSE  (0x1 << 1)


/***************************************************************
 * The following led_cfg example is for reference only.
 * LED0 Link 2500/Blink 2500 TxRx   (GPIO5)    <-> BASE_T_LED0,
 * LED1 Link 1000/Blink 1000 TxRx   (GPIO4)    <-> BASE_T_LED1,
 * LED2 Link 100 /Blink 100  TxRx   (GPIO3)    <-> BASE_T_LED2,
 ***************************************************************/
/* User-defined.B */
#define AIR_LED0_ON     (LED_ON_EVT_LINK_2500M)
#define AIR_LED0_BLK    (LED_BLK_EVT_2500M_TX_ACT | LED_BLK_EVT_2500M_RX_ACT)
#define AIR_LED1_ON     (LED_ON_EVT_LINK_1000M)
#define AIR_LED1_BLK    (LED_BLK_EVT_1000M_TX_ACT | LED_BLK_EVT_1000M_RX_ACT)
#define AIR_LED2_ON     (LED_ON_EVT_LINK_100M)
#define AIR_LED2_BLK    (LED_BLK_EVT_100M_TX_ACT | LED_BLK_EVT_100M_RX_ACT)
/* User-defined.E */

/* CL45 MDIO control */
#define MII_MMD_ACC_CTL_REG         0x0d
#define MII_MMD_ADDR_DATA_REG       0x0e
#define MMD_OP_MODE_DATA            BIT(14)

#define EN8811H_DRIVER_VERSION      "v1.3.0"

#define LED_ON_CTRL(i)              (0x024 + ((i)*2))
#define LED_ON_EN                   (1 << 15)
#define LED_ON_POL                  (1 << 14)
#define LED_ON_EVT_MASK             (0x1ff)
/* LED ON Event Option.B */
#define LED_ON_EVT_LINK_2500M       (1 << 8)
#define LED_ON_EVT_FORCE            (1 << 6)
#define LED_ON_EVT_LINK_DOWN        (1 << 3)
#define LED_ON_EVT_LINK_100M        (1 << 1)
#define LED_ON_EVT_LINK_1000M       (1 << 0)
/* LED ON Event Option.E */

#define LED_BLK_CTRL(i)             (0x025 + ((i)*2))
#define LED_BLK_EVT_MASK            (0xfff)
/* LED Blinking Event Option.B*/
#define LED_BLK_EVT_2500M_RX_ACT    (1 << 11)
#define LED_BLK_EVT_2500M_TX_ACT    (1 << 10)
#define LED_BLK_EVT_FORCE           (1 << 9)
#define LED_BLK_EVT_100M_RX_ACT     (1 << 3)
#define LED_BLK_EVT_100M_TX_ACT     (1 << 2)
#define LED_BLK_EVT_1000M_RX_ACT    (1 << 1)
#define LED_BLK_EVT_1000M_TX_ACT    (1 << 0)
/* LED Blinking Event Option.E*/
#define EN8811H_LED_COUNT           3

#define LED_BCR                     (0x021)
#define LED_BCR_EXT_CTRL            (1 << 15)
#define LED_BCR_CLK_EN              (1 << 3)
#define LED_BCR_TIME_TEST           (1 << 2)
#define LED_BCR_MODE_MASK           (3)
#define LED_BCR_MODE_DISABLE        (0)

#define LED_ON_DUR                  (0x022)
#define LED_ON_DUR_MASK             (0xffff)

#define LED_BLK_DUR                 (0x023)
#define LED_BLK_DUR_MASK            (0xffff)

#define UNIT_LED_BLINK_DURATION     780

#define GET_BIT(val, bit) ((val & BIT(bit)) >> bit)

#define INVALID_DATA                0xffff
#define PBUS_INVALID_DATA           0xffffffff

/* MII Registers */
#define AIR_AUX_CTRL_STATUS		0x1d
#define AIR_AUX_CTRL_STATUS_SPEED_MASK	GENMASK(4, 2)
#define AIR_AUX_CTRL_STATUS_SPEED_100		0x4
#define AIR_AUX_CTRL_STATUS_SPEED_1000	0x8
#define AIR_AUX_CTRL_STATUS_SPEED_2500	0xc

/* Registers on BUCKPBUS */
#define EN8811H_2P5G_LPA		0x3b30
#define EN8811H_2P5G_LPA_2P5G			BIT(0)

#define EN8811H_FW_CTRL_1		0x0f0018
#define   EN8811H_FW_CTRL_1_START		0x0
#define   EN8811H_FW_CTRL_1_FINISH		0x1
#define EN8811H_FW_CTRL_2		0x800000
#define EN8811H_FW_CTRL_2_LOADING		BIT(11)
#define EN8811H_LOOP      0x800

#define NUM_ASI_REGS       5
struct air_cable_test_rsl {
	int          status[4];
	unsigned int length[4];
};

struct en8811h_priv {
	struct dentry       *debugfs_root;
	unsigned int        dm_crc32;
	unsigned int        dsp_crc32;
	unsigned int        ivy_crc32;
	int                 pol;
	int                 surge;
	int                 cko;
	struct kobject      *cable_kobj;
	int                 running_status;
	int                 pair[4];
	int                 an;
	int                 link;
	int                 speed;
	int                 duplex;
	int                 pause;
	int                 asym_pause;
	u16                 on_crtl[3];
	u16                 blk_crtl[3];
	u32                 firmware_version;
	bool                mcu_needs_restart;
	bool                mcu_load;
	int                 debug;
	int                 phy_handle;
	int                 init_stage;
	int                 need_an;
	int                 count;
};

struct air_base_t_led_cfg {
	u16 en;
	u16 gpio;
	u16 pol;
	u16 on_cfg;
	u16 blk_cfg;
};

enum air_init_stage {
	AIR_INIT_START,
	AIR_INIT_CONFIG,
	AIR_INIT_FW_LOADING,
	AIR_INIT_FW_READY,
	AIR_INIT_SUCESS,
	AIR_INIT_FW_FAIL,
	AIR_INIT_FAIL,
	AIR_INIT_LAST
};

enum air_led_gpio {
	AIR_LED2_GPIO3 = 3,
	AIR_LED1_GPIO4,
	AIR_LED0_GPIO5,
	AIR_LED_LAST
};

enum air_base_t_led {
	AIR_BASE_T_LED0,
	AIR_BASE_T_LED1,
	AIR_BASE_T_LED2,
	AIR_BASE_T_LED3
};

enum air_led_blk_dur {
	AIR_LED_BLK_DUR_32M,
	AIR_LED_BLK_DUR_64M,
	AIR_LED_BLK_DUR_128M,
	AIR_LED_BLK_DUR_256M,
	AIR_LED_BLK_DUR_512M,
	AIR_LED_BLK_DUR_1024M,
	AIR_LED_BLK_DUR_LAST
};

enum air_led_polarity {
	AIR_ACTIVE_LOW,
	AIR_ACTIVE_HIGH,
};
enum air_led_mode {
	AIR_LED_MODE_DISABLE,
	AIR_LED_MODE_USER_DEFINE,
	AIR_LED_MODE_LAST
};

#endif /* End of __EN8811H_H */
