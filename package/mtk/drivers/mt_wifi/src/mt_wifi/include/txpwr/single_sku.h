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
	single_sku.h
*/

#ifndef __CMM_SINGLE_SKU_H__
#define __CMM_SINGLE_SKU_H__

/*******************************************************************************
 *	INCLUDED FILES
 ******************************************************************************/

/*******************************************************************************
 *	DEFINITIONS
 ******************************************************************************/

/** buffer size allocated for power limit table */
/* use Sku_sizeof[sku_tbl_idx] & Backoff_sizeof[sku_tbl_idx]  for new chips */
#define MAX_POWER_LIMIT_BUFFER_SIZE    42000

/* Debug log color */
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define CH_G_BAND    0
#define CH_A_BAND    1
#define CH_6G_BAND   2

#define	SINGLE_SKU_TABLE_LENGTH		(SINGLE_SKU_TABLE_CCK_LENGTH+SINGLE_SKU_TABLE_OFDM_LENGTH+(SINGLE_SKU_TABLE_HT_LENGTH*2)+SINGLE_SKU_TABLE_VHT_LENGTH)

#define SINGLE_SKU_TABLE_EFFUSE_ADDRESS 0x12C

#define SINGLE_SKU_TABLE_CCK_LENGTH		   2
#define SINGLE_SKU_TABLE_OFDM_LENGTH		  5
#define SINGLE_SKU_TABLE_HT_LENGTH			16
#define SINGLE_SKU_TABLE_VHT_LENGTH		   7	 /* VHT80 MCS 0 ~ 9 */
#define TABLE_PARSE_TYPE_NUM				  2

/* 0: None, 1: CH_G_BAND, 2: CH_A_BAND, 3: CH_G_BAND and CH_A_BAND */
#define TABLE_NO_PARSE						0
#define TABLE_PARSE_G_BAND					BIT(0)
#define TABLE_PARSE_A_BAND					BIT(1)
#define TABLE_PARSE_G_A_BAND				  BITS(0, 1)

#define SINGLE_SKU_TABLE_TX_OFFSET_NUM  3
#define SINGLE_SKU_TABLE_NSS_OFFSET_NUM 4

#define SKUTABLE_1					  1
#define SKUTABLE_2					  2
#define SKUTABLE_3					  3
#define SKUTABLE_4					  4
#define SKUTABLE_5					  5
#define SKUTABLE_6					  6
#define SKUTABLE_7					  7
#define SKUTABLE_8					  8
#define SKUTABLE_9					  9
#define SKUTABLE_10					10
#define SKUTABLE_11					11
#define SKUTABLE_12					12
#define SKUTABLE_13					13
#define SKUTABLE_14					14
#define SKUTABLE_15					15
#define SKUTABLE_16					16
#define SKUTABLE_17					17
#define SKUTABLE_18					18
#define SKUTABLE_19					19
#define SKUTABLE_20					20
#define TABLE_SIZE					 20

#define VHT20_OFFSET					0
#define VHT40_OFFSET					7
#define VHT80_OFFSET				   14
#define VHT160C_OFFSET				 21

/* PHY Mode */
#define SKU_CCK_OFFSET				  0
#define SKU_OFDM_OFFSET				 2
#define SKU_HT_OFFSET				   7
#define SKU_VHT_OFFSET				 21

/* MCS Rate */
#define SKU_CCK_RATE_M01				0
#define SKU_CCK_RATE_M23				1

#define SKU_OFDM_RATE_M01			   0
#define SKU_OFDM_RATE_M23			   1
#define SKU_OFDM_RATE_M45			   2
#define SKU_OFDM_RATE_M6				3
#define SKU_OFDM_RATE_M7				4

#define SKU_HT_RATE_M0				  0
#define SKU_HT_RATE_M32				 1
#define SKU_HT_RATE_M12				 2
#define SKU_HT_RATE_M34				 3
#define SKU_HT_RATE_M5				  4
#define SKU_HT_RATE_M6				  5
#define SKU_HT_RATE_M7				  6

#define SKU_VHT_RATE_M0				 0
#define SKU_VHT_RATE_M12				1
#define SKU_VHT_RATE_M34				2
#define SKU_VHT_RATE_M56				3
#define SKU_VHT_RATE_M7				 4
#define SKU_VHT_RATE_M8				 5
#define SKU_VHT_RATE_M9				 6

/*******************************************************************************
 *	MACRO
 ******************************************************************************/

#define SINGLE_SKU_TABLE_FILE_NAME	"/etc/wireless/mediatek/mt7615e-sku.dat"
#define BF_SKU_TABLE_FILE_NAME		"/etc/wireless/mediatek/mt7615e-sku-bf.dat"

/*******************************************************************************
 *	TYPES
 ******************************************************************************/

typedef enum _POWER_LIMIT_TABLE {
	POWER_LIMIT_TABLE_TYPE_SKU = 0,
	POWER_LIMIT_TABLE_TYPE_BACKOFF,
	POWER_LIMIT_TABLE_TYPE_NUM
} POWER_LIMIT_TABLE, *P_POWER_LIMIT_TABLE;

#ifdef TPC_SUPPORT
typedef enum _POWER_LIMIT_TABLE_MODE {
	POWER_LIMIT_TX_MODE_CCK = 0,
	POWER_LIMIT_TX_MODE_OFDM,
	POWER_LIMIT_TX_MODE_HTVHT20,
	POWER_LIMIT_TX_MODE_HTVHT40,
	POWER_LIMIT_TX_MODE_HTVHT80,
	POWER_LIMIT_TX_MODE_HTVHT160,
	POWER_LIMIT_TX_MODE_RU26,
	POWER_LIMIT_TX_MODE_RU52,
	POWER_LIMIT_TX_MODE_RU106,
	POWER_LIMIT_TX_MODE_RU242,
	POWER_LIMIT_TX_MODE_RU484,
	POWER_LIMIT_TX_MODE_RU996,
	POWER_LIMIT_TX_MODE_RU996X2,
	POWER_LIMIT_TX_MODE_NUM
} POWER_LIMIT_TABLE_MODE, *P_POWER_LIMIT_TABLE_MODE;

typedef enum _POWER_LIMIT_TABLE_CCK_T {
	POWER_LIMIT_CCK_1M = 0,
	POWER_LIMIT_CCK_2M,
	POWER_LIMIT_CCK_5M,
	POWER_LIMIT_CCK_11M,
	POWER_LIMIT_CCK_NUM
} POWER_LIMIT_TABLE_CCK_T, *P_POWER_LIMIT_TABLE_CCK_T;

typedef enum _POWER_LIMIT_TABLE_OFDM_T {
	POWER_LIMIT_OFDM_6M = 0,
	POWER_LIMIT_OFDM_9M,
	POWER_LIMIT_OFDM_12M,
	POWER_LIMIT_OFDM_18M,
	POWER_LIMIT_OFDM_24M,
	POWER_LIMIT_OFDM_36M,
	POWER_LIMIT_OFDM_48M,
	POWER_LIMIT_OFDM_54M,
	POWER_LIMIT_OFDM_NUM
} POWER_LIMIT_TABLE_OFDM_T, POWER_LIMIT_TABLE_OFDM_T;

UINT8 GetSkuTxPwr(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	USHORT FCSubType);

#endif /* TPC_SUPPORT */

/* TODO: shiang-usw, need to re-organize these for MT7610/MT7601/MT7620!! */

typedef struct _CH_POWER_V0 {
	DL_LIST  List;
	UINT8  StartChannel;
	UINT8  num;
	UINT8  *Channel;
	UINT8  band;
	UINT8  u1PwrLimitCCK[SINGLE_SKU_TABLE_CCK_LENGTH];
	UINT8  u1PwrLimitOFDM[SINGLE_SKU_TABLE_OFDM_LENGTH];
	UINT8  u1PwrLimitHT20[SINGLE_SKU_TABLE_HT_LENGTH];
	UINT8  u1PwrLimitHT40[SINGLE_SKU_TABLE_HT_LENGTH];
	UINT8  u1PwrLimitVHT20[SINGLE_SKU_TABLE_VHT_LENGTH];
	UINT8  u1PwrLimitVHT40[SINGLE_SKU_TABLE_VHT_LENGTH];
	UINT8  u1PwrLimitVHT80[SINGLE_SKU_TABLE_VHT_LENGTH];
	UINT8  u1PwrLimitVHT160[SINGLE_SKU_TABLE_VHT_LENGTH];
	UINT8  u1PwrLimitTxStreamDelta[SINGLE_SKU_TABLE_TX_OFFSET_NUM];
	UINT8  u1PwrLimitTxNSSDelta[SINGLE_SKU_TABLE_NSS_OFFSET_NUM];
} CH_POWER_V0, *P_CH_POWER_V0;

typedef struct _CH_POWER_V1 {
	DL_LIST  List;
	UINT8  u1StartChannel;
	UINT8  u1ChNum;
	PUINT8 pu1ChList;
	UINT8  u1ChBand;
	PUINT8 pu1PwrLimit;
} CH_POWER_V1, *P_CH_POWER_V1;

typedef enum _ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE {
	POWER_LIMIT_LINK_LIST = 0,
	POWER_LIMIT_RAW_DATA_LENGTH,
	POWER_LIMIT_RAW_DATA_OFFSET,
	POWER_LIMIT_DATA_LENGTH,
	POWER_LIMIT_CH_BAND_NEED_PARSE_BITFIELD,
	POWER_LIMIT_PARAMETER_INSTANCE_TYPE_NUM
} ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE, *P_ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE;

/*******************************************************************************
 *	GLOBAL VARIABLES
 ******************************************************************************/


/*******************************************************************************
 *	FUNCTION PROTOTYPES
 ******************************************************************************/
NDIS_STATUS
MtPwrLimitLoadParamHandle(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1Type
	);

NDIS_STATUS
MtPwrLimitUnloadParamHandle(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1Type
	);

NDIS_STATUS
MtParsePwrLimitTable(
	struct _RTMP_ADAPTER *pAd,
	PCHAR *pi1Buffer,
	UINT8 u1Type
	);

NDIS_STATUS
MtReadPwrLimitTable(
	struct _RTMP_ADAPTER *pAd,
	PCHAR *pi1Buffer,
	UINT8 u1Type
	);

NDIS_STATUS
MtPwrLimitParse(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pi1PwrLimitNewCh,
	UINT8 u1ChBand,
	UINT8 u1Type
	);

NDIS_STATUS
MtPwrLimitSimilarCheck(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pi1PwrLimitStartCh,
	PUINT8 pi1PwrLimitNewCh,
	BOOLEAN *pfgSameContent,
	UINT8 u1ChBand,
	UINT8 u1Type
	);

NDIS_STATUS
MtShowPwrLimitTable(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1Type,
	UINT8 u1DebugLevel
	);

VOID
MtPwrLimitTblChProc(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT8 u1ChannelBand,
	UINT8 u1ControlChannel,
	UINT8 u1CentralChannel
	);

NDIS_STATUS
MtPwrGetPwrLimitInstanceSku(
	struct _RTMP_ADAPTER *pAd,
	ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE eInstanceIdx,
	PVOID * ppvBuffer
	);

NDIS_STATUS
MtPwrGetPwrLimitInstanceBackoff(
	struct _RTMP_ADAPTER *pAd,
	ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE eInstanceIdx,
	PVOID *ppvBuffer);

NDIS_STATUS
MtPwrGetPwrLimitInstance(
	struct _RTMP_ADAPTER *pAd,
	POWER_LIMIT_TABLE u1Type,
	ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE eInstanceIdx,
	PVOID *ppvBuffer
	);

NDIS_STATUS
MtPowerLimitFormatTrans(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pu1Value,
	PCHAR pcRawData
	);

CHAR
SKUTxPwrOffsetGet(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucBandIdx,
	UINT8 ucBW,
	UINT8 ucPhymode,
	UINT8 ucMCS,
	UINT8 ucNss,
	BOOLEAN fgSE
	);

NDIS_STATUS
MtPwrFillLimitParam(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ChBand,
	UINT8 u1ControlChannel,
	UINT8 u1CentralChannel,
	VOID  *pi1PwrLimitParam,
	UINT8 u1Type
	);

NDIS_STATUS
MtPowerLimitFormatTrans(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pu1Value,
	PCHAR pcRawData
	);

#endif /*__CMM_SINGLE_SKU_H__*/
