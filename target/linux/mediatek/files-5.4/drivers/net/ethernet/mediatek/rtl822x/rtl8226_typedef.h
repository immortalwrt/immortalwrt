/*
 * Copyright (C) 2019 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : PHY 8226 Driver
 *
 * Feature : PHY 8226 Driver
 *
 */
#ifndef __NIC_RTL8226_TYPEDEF_H__
#define __NIC_RTL8226_TYPEDEF_H__

#include <linux/phy.h>

/* from typedef.h and rtl8156_mmd.h */

#define BIT_0       0x0001
#define BIT_1       0x0002
#define BIT_2       0x0004
#define BIT_3       0x0008
#define BIT_4       0x0010
#define BIT_5       0x0020
#define BIT_6       0x0040
#define BIT_7       0x0080
#define BIT_8       0x0100
#define BIT_9       0x0200
#define BIT_10      0x0400
#define BIT_11      0x0800
#define BIT_12      0x1000
#define BIT_13      0x2000
#define BIT_14      0x4000
#define BIT_15      0x8000

#define SUCCESS     TRUE
#define FAILURE     FALSE



typedef struct {
    struct mtk_eth *eth;
    int addr;
} HANDLE;

#define BOOLEAN         bool
#define BOOL            uint32
#define UINT32          uint32
#define UINT16          uint16
#define UINT8           uint8
#define Sleep(_t)       osal_time_udelay(_t*1000)
#define IN
#define OUT


#define MMD_PMAPMD     1
#define MMD_PCS        3
#define MMD_AN         7
#define MMD_VEND1      30   /* Vendor specific 2 */
#define MMD_VEND2      31   /* Vendor specific 2 */


typedef struct
{
    UINT16 dev;
    UINT16 addr;
    UINT16 value;
} MMD_REG;


#define NO_LINK 0
#define LINK_SPEED_10M 10
#define LINK_SPEED_100M 100
#define LINK_SPEED_500M 500
#define LINK_SPEED_1G 1000
#define LINK_SPEED_2P5G 2500

typedef enum
{
    PHY_CROSSPVER_MODE_AUTO = 0,
    PHY_CROSSPVER_MODE_MDI,
    PHY_CROSSPVER_MODE_MDIX,
    PHY_CROSSPVER_MODE_END
} PHY_CROSSPVER_MODE;

typedef enum
{
    PHY_CROSSPVER_STATUS_MDI = 0,
    PHY_CROSSPVER_STATUS_MDIX,
    PHY_CROSSPVER_STATUS_END
} PHY_CROSSPVER_STATUS;

typedef enum
{
    PHY_AUTO_MODE = 0,
    PHY_SLAVE_MODE,
    PHY_MASTER_MODE,
    PHY_MASTER_SLAVE_END
} PHY_MASTERSLAVE_MODE;

typedef struct
{
    UINT32 Half_10:1;
    UINT32 Full_10:1;

    UINT32 Half_100:1;
    UINT32 Full_100:1;

    UINT32 Full_1000:1;

    UINT32 adv_2_5G:1;

    UINT32 FC:1;
    UINT32 AsyFC:1;
} PHY_LINK_ABILITY;

typedef struct
{
    UINT8 EEE_100:1;
    UINT8 EEE_1000:1;
    UINT8 EEE_2_5G:1;
} PHY_EEE_ENABLE;

typedef struct
{
    UINT8 TX_SWAP:1;
    UINT8 RX_SWAP:1;
} PHY_SERDES_POLARITY_SWAP;

typedef enum
{
	TESTMODE_CHANNEL_NONE = 0,
	TESTMODE_CHANNEL_A,
	TESTMODE_CHANNEL_B,
	TESTMODE_CHANNEL_C,
	TESTMODE_CHANNEL_D,
	TESTMODE_CHANNEL_END
} PHY_TESTMODE_CHANNEL;

typedef struct
{
    UINT32 TM1:1;
    UINT32 TM2:1;
    UINT32 TM3:1;
    UINT32 TM4:1;
    UINT32 TM5:1;
    UINT32 TM6:1;

    UINT32 TONE1:1;
    UINT32 TONE2:1;
    UINT32 TONE3:1;
    UINT32 TONE4:1;
    UINT32 TONE5:1;

    UINT32 TMFINISH:1;

    UINT32 NORMAL:1;
    UINT32 HARMONIC:1;
    UINT32 LINKPLUSE:1;

	PHY_TESTMODE_CHANNEL channel:3;
} PHY_IEEE_TEST_MODE;

typedef enum
{
    MIS_MATCH_OPEN = 1, // Mis-Match_Open, larger_than_130ohm
    MIS_MATCH_SHORT = 2, // Mis-Match_short, less_than_77ohm
} PHY_RTCT_STATUS_MISMATCH;

typedef struct
{
    BOOL Open;
    BOOL Short;
    PHY_RTCT_STATUS_MISMATCH Mismatch;
} PHY_RTCT_STATUS;

typedef struct
{

    UINT16 linkType;

    UINT32 rxLen;
    UINT32 txLen;

    UINT32 channelALen;
    UINT32 channelBLen;
    UINT32 channelCLen;
    UINT32 channelDLen;

    PHY_RTCT_STATUS channelAStatus;
    PHY_RTCT_STATUS channelBStatus;
    PHY_RTCT_STATUS channelCStatus;
    PHY_RTCT_STATUS channelDStatus;
} PHY_RTCT_RESULT;

typedef enum
{
    PHY_SERDES_MODE_OTHER = 0,
    PHY_SERDES_MODE_SGMII,
    PHY_SERDES_MODE_HiSGMII,
    PHY_SERDES_MODE_2500BASEX,
    PHY_SERDES_MODE_USXGMII,
    PHY_SERDES_MODE_NO_SDS,
    PHY_SERDES_MODE_END
} PHY_SERDES_MODE;

typedef enum
{
    PHY_SERDES_OPTION_2500BASEX_SGMII = 0,
    PHY_SERDES_OPTION_HiSGMII_SGMII,
    PHY_SERDES_OPTION_2500BASEX,
    PHY_SERDES_OPTION_HiSGMII,
    PHY_SERDES_OPTION_OTHER,
} PHY_SERDES_OPTION;

typedef struct
{
    UINT16 MASK15_0;
    UINT16 MASK31_16;
    UINT16 MASK47_32;
    UINT16 MASK63_48;
    UINT16 MASK79_64;
    UINT16 MASK95_80;
    UINT16 MASK111_96;
    UINT16 MASK127_112;
    UINT16 CRC;
} PHY_WAKEUP_FRAME;

typedef struct
{
    UINT16 REG15_0;
    UINT16 REG31_16;
    UINT16 REG47_32;
    UINT16 REG63_48;
} PHY_MULTICAST_REG;

typedef struct
{
    UINT16 ADDR15_0;
    UINT16 ADDR31_16;
    UINT16 ADDR47_32;
} PHY_MAC_ADDRESS;

typedef struct
{
    UINT32 LINKCHG:1;
    UINT32 MAGIC:1;
    UINT32 ARBITRARY:1;
    UINT32 UNICAST:1;
    UINT32 MULTICAST:1;
    UINT32 BROADCAST:1;

    UINT32 FRAME0:1;
    UINT32 FRAME1:1;
    UINT32 FRAME2:1;
    UINT32 FRAME3:1;
    UINT32 FRAME4:1;
    UINT32 FRAME5:1;
    UINT32 FRAME6:1;
    UINT32 FRAME7:1;

    UINT32 MAXPKTLENGTH;
    PHY_MAC_ADDRESS macaddress;
    PHY_MULTICAST_REG multicast;

    PHY_WAKEUP_FRAME wakeframe0;

} PHY_WOL_EVENT;

typedef struct
{
    bool	Enable;
    UINT16	Temperature;
    UINT16	Temperature_threshold;
}PHY_THERMAL_RESULT;


BOOLEAN
MmdPhyRead(
    IN  HANDLE hDevice,
    IN  UINT16 dev,
    IN  UINT16 addr,
    OUT UINT16 *data);

BOOLEAN
MmdPhyWrite(
    IN HANDLE hDevice,
    IN UINT16 dev,
    IN UINT16 addr,
    IN UINT16 data);




#endif /* __NIC_RTL8226_TYPEDEF_H__ */


