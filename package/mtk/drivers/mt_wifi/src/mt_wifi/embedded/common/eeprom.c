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
	eeprom.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/
#include "rt_config.h"


#ifdef CONFIG_RT_FIRST_CARD
#define FIRST_EEPROM_FILE_PATH	"/etc_ro/Wireless/RT2860/MT7615_EEPROM_1.bin"
#endif

#ifdef CONFIG_RT_SECOND_CARD
#define SECOND_EEPROM_FILE_PATH	"/etc_ro/Wireless/iNIC/MT7615_EEPROM_2.bin"
#endif

#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
#ifdef CONFIG_RT_FIRST_CARD
#define FIRST_CAL_BIN_FILE_PATH	    "/etc_ro/Wireless/RT2860AP/CALIBRATION_DATA_1.bin"
#endif /* CONFIG_RT_FIRST_CARD */

#ifdef CONFIG_RT_SECOND_CARD
#define SECOND_CAL_BIN_FILE_PATH	"/etc_ro/Wireless/iNIC/CALIBRATION_DATA_2.bin"
#endif /* CONFIG_RT_SECOND_CARD */
#endif /* CAL_BIN_FILE_SUPPORT */

struct chip_map {
	UINT32 ChipVersion;
	RTMP_STRING *name;
};

struct chip_map RTMP_CHIP_E2P_FILE_TABLE[] = {
	{0x3071,	"RT3092_PCIe_LNA_2T2R_ALC_V1_2.bin"},
	{0x3090,	"RT3092_PCIe_LNA_2T2R_ALC_V1_2.bin"},
	{0x3593,	"HMC_RT3593_PCIe_3T3R_V1_3.bin"},
	{0x5392,	"RT5392_PCIe_2T2R_ALC_V1_4.bin"},
	{0x5592,	"RT5592_PCIe_2T2R_V1_7.bin"},
	{0,}
};


struct chip_map chip_card_id_map[] = {
	{7620, ""},
};


INT rtmp_read_rssi_langain_from_eeprom(RTMP_ADAPTER *pAd)
{
	INT i;
	UINT16 value = 0;
	/* Get RSSI Offset on EEPROM 0x9Ah & 0x9Ch.*/
	/* The valid value are (-10 ~ 10) */
	/* */
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_BG_OFFSET, value);
		pAd->BGRssiOffset[0] = value & 0x00ff;
		pAd->BGRssiOffset[1] = (value >> 8);
	}
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_BG_OFFSET + 2, value);
		{
			pAd->BGRssiOffset[2] = value & 0x00ff;
			pAd->ALNAGain1 = (value >> 8);
		}
	}
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_LNA_OFFSET, value);
		pAd->BLNAGain = value & 0x00ff;
		/* External LNA gain for 5GHz Band(CH36~CH64) */
		pAd->ALNAGain0 = (value >> 8);
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_A_OFFSET, value);
		pAd->ARssiOffset[0] = value & 0x00ff;
		pAd->ARssiOffset[1] = (value >> 8);
	}
	{
		RT28xx_EEPROM_READ16(pAd, (EEPROM_RSSI_A_OFFSET + 2), value);
		{
			pAd->ARssiOffset[2] = value & 0x00ff;
			pAd->ALNAGain2 = (value >> 8);
		}
	}

	if (((UCHAR)pAd->ALNAGain1 == 0xFF) || (pAd->ALNAGain1 == 0x00))
		pAd->ALNAGain1 = pAd->ALNAGain0;

	if (((UCHAR)pAd->ALNAGain2 == 0xFF) || (pAd->ALNAGain2 == 0x00))
		pAd->ALNAGain2 = pAd->ALNAGain0;

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ALNAGain0 = %d, ALNAGain1 = %d, ALNAGain2 = %d\n",
			 pAd->ALNAGain0, pAd->ALNAGain1, pAd->ALNAGain2);

	/* Validate 11a/b/g RSSI 0/1/2 offset.*/
	for (i = 0; i < 3; i++) {
		if ((pAd->BGRssiOffset[i] < -10) || (pAd->BGRssiOffset[i] > 10))
			pAd->BGRssiOffset[i] = 0;

		if ((pAd->ARssiOffset[i] < -10) || (pAd->ARssiOffset[i] > 10))
			pAd->ARssiOffset[i] = 0;
	}

	return TRUE;
}


/*
	CountryRegion byte offset (38h)
*/
INT rtmp_read_country_region_from_eeporm(RTMP_ADAPTER *pAd)
{
	UINT16 value, value2;
	{
		value = pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] >> 8;		/* 2.4G band*/
		value2 = pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] & 0x00FF;	/* 5G band*/
	}

	if ((value <= REGION_MAXIMUM_BG_BAND) || (value == REGION_31_BG_BAND) || (value == REGION_32_BG_BAND) ||
		(value == REGION_33_BG_BAND))
		pAd->CommonCfg.CountryRegion = ((UCHAR) value) | EEPROM_IS_PROGRAMMED;

	if (value2 <= REGION_MAXIMUM_A_BAND)
		pAd->CommonCfg.CountryRegionForABand = ((UCHAR) value2) | EEPROM_IS_PROGRAMMED;

	return TRUE;
}


/*
	Read frequency offset setting from EEPROM which used for RF
*/
INT rtmp_read_freq_offset_from_eeprom(RTMP_ADAPTER *pAd)
{
	UINT16 value = 0;
	RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, value);
	{
		if ((value & 0x00FF) != 0x00FF)
			pAd->RfFreqOffset = (ULONG) (value & 0x00FF);
		else
			pAd->RfFreqOffset = 0;
	}
#ifdef RTMP_RBUS_SUPPORT

	if (pAd->infType == RTMP_DEV_INF_RBUS) {
		if (pAd->RfFreqDelta & 0x10)
			pAd->RfFreqOffset = (pAd->RfFreqOffset >= pAd->RfFreqDelta) ? (pAd->RfFreqOffset - (pAd->RfFreqDelta & 0xf)) : 0;
		else
			pAd->RfFreqOffset = ((pAd->RfFreqOffset + pAd->RfFreqDelta) < 0x40) ? (pAd->RfFreqOffset +
								(pAd->RfFreqDelta & 0xf)) : 0x3f;
	}

#endif /* RTMP_RBUS_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "E2PROM: RF FreqOffset=0x%x\n", pAd->RfFreqOffset);
	return TRUE;
}


INT rtmp_read_txpwr_from_eeprom(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* if not return early. cause fail at emulation.*/
	/* Init the channel number for TX channel power*/
	if (ops->read_chl_pwr)
		ops->read_chl_pwr(pAd);

	RTMPReadTxPwrPerRate(pAd);
#ifdef SINGLE_SKU
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_DEFINE_MAX_TXPWR, pAd->CommonCfg.DefineMaxTxPwr);
	}

	/*
		Some dongle has old EEPROM value, use ModuleTxpower for saving correct value fo DefineMaxTxPwr.
		ModuleTxpower will override DefineMaxTxPwr (value from EEPROM) if ModuleTxpower is not zero.
	*/
	if (pAd->CommonCfg.ModuleTxpower > 0)
		pAd->CommonCfg.DefineMaxTxPwr = pAd->CommonCfg.ModuleTxpower;

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TX Power set for SINGLE SKU MODE is : 0x%04x\n",
			 pAd->CommonCfg.DefineMaxTxPwr);
	pAd->CommonCfg.bSKUMode = FALSE;

	if ((pAd->CommonCfg.DefineMaxTxPwr & 0xFF) <= 0x50) {
		if ((pAd->CommonCfg.AntGain > 0) && (pAd->CommonCfg.BandedgeDelta >= 0))
			pAd->CommonCfg.bSKUMode = TRUE;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Single SKU Mode is %s\n",
			 pAd->CommonCfg.bSKUMode ? "Enable" : "Disable");
#endif /* SINGLE_SKU */
#ifdef SINGLE_SKU_V2
	InitSkuRateDiffTable(pAd);
#endif /* SINGLE_SKU_V2 */
	return TRUE;
}


/*
	========================================================================

	Routine Description:
		Read initial parameters from EEPROM

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
INT NICReadEEPROMParameters(RTMP_ADAPTER *pAd, RTMP_STRING *mac_addr)
{
	USHORT i, value = 0;
	EEPROM_VERSION_STRUC Version = {0};
	EEPROM_ANTENNA_STRUC Antenna;
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;
	USHORT  Addr01 = 0, Addr23 = 0, Addr45 = 0;
#ifdef CONFIG_6G_SUPPORT
#ifdef DBDC_MODE
	PUSHORT EeprAntCfg = NULL;
#endif /* DBDC_MODE */
#endif /* CONFIG_6G_SUPPORT */
#ifdef PRE_CAL_TRX_SET2_SUPPORT
	UINT16 PreCalSize;
#endif /* PRE_CAL_TRX_SET2_SUPPORT */
#if defined(PRE_CAL_TRX_SET2_SUPPORT) || defined(PRE_CAL_MT7622_SUPPORT) || \
	defined(PRE_CAL_MT7626_SUPPORT) || defined(PRE_CAL_MT7915_SUPPORT) || \
	defined(PRE_CAL_MT7986_SUPPORT) || defined(PRE_CAL_MT7981_SUPPORT) || \
	defined(PRE_CAL_MT7916_SUPPORT)
	UINT16 DoPreCal = 0;
#endif /* PRE_CAL_TRX_SET2_SUPPORT */
#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
	UINT16 DoPATrim = 0;
#endif /* CAL_BIN_FILE_SUPPORT */
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

#ifdef PRE_CAL_MT7622_SUPPORT
	UINT32 ch;
#endif /* PRE_CAL_MT7622_SUPPORT */

#if defined(PRE_CAL_MT7626_SUPPORT) || defined(PRE_CAL_MT7915_SUPPORT) || \
	defined(PRE_CAL_MT7986_SUPPORT) || defined(PRE_CAL_MT7981_SUPPORT) || \
	defined(PRE_CAL_MT7916_SUPPORT)
	UINT32 setPreCalTotal = 0;
	USHORT setPreCalIdx = 0;
	UINT32 sendLength = 0;
#endif

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");

	if (ops->eeinit) {
#if defined(WCX_SUPPORT)
		/* do nothing */
#else
#ifdef CAL_FREE_IC_SUPPORT
		BOOLEAN bCalFree = FALSE;
#endif
		/* If we are run in Multicard mode */
		ops->eeinit(pAd);
#ifdef CAL_FREE_IC_SUPPORT
		RTMP_CAL_FREE_IC_CHECK(pAd, bCalFree);
		if (bCalFree) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Cal Free IC!!\n");
			RTMP_CAL_FREE_DATA_GET(pAd);
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Non Cal Free IC!!\n");
		}
#endif	/* CAL_FREE_IC_SUPPORT */
#endif /*WCX_SUPPORT */

		/* Merge RF parameters in Effuse to E2p buffer */
		if (ops->merge_RF_lock_parameter != NULL)
			ops->merge_RF_lock_parameter(pAd);

		/* Replace Country code and Country Region in Profile by Effuse content */
		if (ops->Config_Effuse_Country != NULL)
			ops->Config_Effuse_Country(pAd);

	}

#ifdef PRE_CAL_MT7622_SUPPORT
	if (IS_MT7622(pAd)) {
		/* Check DoPreCal bits */
		RT28xx_EEPROM_READ16(pAd, 0x52, DoPreCal);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\x1b[34m EEPROM 0x52 %x\x1b[m\n", DoPreCal);
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			if (DoPreCal & (1 << 3)) {
				MtCmdSetTxLpfCal_7622(pAd);
				MtCmdSetTxDcIqCal_7622(pAd);
				for (ch = 1; ch < 14; ch++)
					MtCmdSetTxDpdCal_7622(pAd, ch);
			}
		}
	}
#endif /*PRE_CAL_MT7622_SUPPORT*/
#ifdef PRE_CAL_MT7626_SUPPORT
	if (IS_MT7626(pAd)) {
		/* Check DoPreCal bits */
		RT28xx_EEPROM_READ16(pAd, 0x32, DoPreCal);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\x1b[34m EEPROM 0x32 %x\x1b[m\n", DoPreCal);
		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			if (DoPreCal & (1 << 0)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Group Pre-Cal finish !\n");
				os_move_mem(&(pAd->PreCalOfst), pAd->PreCalImageInfo, 4);
				setPreCalTotal = pAd->PreCalOfst;
				while (setPreCalTotal > 0) {
					if (setPreCalTotal >= PRE_CAL_SET_MAX_LENGTH)
						MtCmdSetGroupPreCal_7626(pAd, setPreCalIdx, PRE_CAL_SET_MAX_LENGTH);
					else
						MtCmdSetGroupPreCal_7626(pAd, setPreCalIdx, setPreCalTotal);

					setPreCalIdx = setPreCalIdx + 1;
					if (setPreCalTotal >= PRE_CAL_SET_MAX_LENGTH)
						setPreCalTotal = setPreCalTotal - PRE_CAL_SET_MAX_LENGTH;
					else
						setPreCalTotal = 0;
				}
			}
			if (DoPreCal & (1 << 1)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DPD 5G Pre-Cal finish !\n");
			}
			if (DoPreCal & (1 << 2)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DPD 2G Pre-Cal finish !\n");
			}
		}
	}
#endif /* PRE_CAL_MT7626_SUPPORT */
#ifdef PRE_CAL_MT7915_SUPPORT
	if (IS_MT7915(pAd)) {
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

		/* Check DoPreCal bits */
		RT28xx_EEPROM_READ16(pAd, MT7915_PRECAL_INDICATION_BYTE, DoPreCal);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\x1b[34m EEPROM 0x%2x %x\x1b[m\n", MT7915_PRECAL_INDICATION_BYTE, DoPreCal);
		if (pAd->E2pAccessMode == E2P_FLASH_MODE ||
			pAd->E2pAccessMode == E2P_BIN_MODE) {
			if (DoPreCal & (1 << GROUP_PRECAL_INDN_BIT)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Group Pre-Cal finished, load Group Pre-Cal data\n");
				setPreCalTotal = cap->prek_ee_info.pre_cal_total_size;
				while (setPreCalTotal > 0) {
					sendLength = (setPreCalTotal >= PRE_CAL_SET_MAX_LENGTH) ?
								PRE_CAL_SET_MAX_LENGTH : setPreCalTotal;
					MtCmdSetGroupPreCal_7915(pAd, setPreCalIdx, sendLength);
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"idx=%d, sendLength=%d, remainedLength=%d\n",
						setPreCalIdx, sendLength, setPreCalTotal);
					setPreCalIdx++;
					setPreCalTotal -= sendLength;
				}
			}
			if (DoPreCal & (1 << DPD5G_PRECAL_INDN_BIT)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DPD 5G Pre-Cal finished!\n");
			}
			if (DoPreCal & (1 << DPD2G_PRECAL_INDN_BIT)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DPD 2G Pre-Cal finished!\n");
			}
		}
	}
#endif /* PRE_CAL_MT7915_SUPPORT */

#ifdef PRE_CAL_MT7986_SUPPORT
	if (IS_MT7986(pAd)) {
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

		/* Check DoPreCal bits */
		RT28xx_EEPROM_READ16(pAd, PRECAL_INDICATION_BYTE, DoPreCal);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\x1b[34m EEPROM 0x%2x %x\x1b[m\n", PRECAL_INDICATION_BYTE, DoPreCal);
		if (pAd->E2pAccessMode == E2P_FLASH_MODE ||
			pAd->E2pAccessMode == E2P_BIN_MODE) {
			if (DoPreCal & (1 << GROUP_PRECAL_INDN_BIT)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Group Pre-Cal finished, load Group Pre-Cal data\n");
				setPreCalTotal = cap->prek_ee_info.pre_cal_total_size;
				while (setPreCalTotal > 0) {
					sendLength = (setPreCalTotal >= PRE_CAL_SET_MAX_LENGTH) ?
								PRE_CAL_SET_MAX_LENGTH : setPreCalTotal;
					MtCmdSetGroupPreCal_7986(pAd, setPreCalIdx, sendLength);
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"idx=%d, sendLength=%d, remainedLength=%d\n",
						setPreCalIdx, sendLength, setPreCalTotal);
					setPreCalIdx++;
					setPreCalTotal -= sendLength;
				}
			}
			if (DoPreCal & (1 << DPD5G_PRECAL_INDN_BIT)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DPD 5G Pre-Cal finished!\n");
			}
			if (DoPreCal & (1 << DPD2G_PRECAL_INDN_BIT)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DPD 2G Pre-Cal finished!\n");
			}
		}
	}
#endif /* PRE_CAL_MT7986_SUPPORT */

#ifdef PRE_CAL_MT7981_SUPPORT
	if (IS_MT7981(pAd)) {
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

		/* Check DoPreCal bits */
		RT28xx_EEPROM_READ16(pAd, PRECAL_INDICATION_BYTE, DoPreCal);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\x1b[34m EEPROM 0x%2x %x\x1b[m\n", PRECAL_INDICATION_BYTE, DoPreCal);
		if (pAd->E2pAccessMode == E2P_FLASH_MODE ||
			pAd->E2pAccessMode == E2P_BIN_MODE) {
			if (DoPreCal & (1 << GROUP_PRECAL_INDN_BIT)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Group Pre-Cal finished, load Group Pre-Cal data\n");
				setPreCalTotal = cap->prek_ee_info.pre_cal_total_size;
				while (setPreCalTotal > 0) {
					sendLength = (setPreCalTotal >= PRE_CAL_SET_MAX_LENGTH) ?
								PRE_CAL_SET_MAX_LENGTH : setPreCalTotal;
					MtCmdSetGroupPreCal_7981(pAd, setPreCalIdx, sendLength);
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"idx=%d, sendLength=%d, remainedLength=%d\n",
						setPreCalIdx, sendLength, setPreCalTotal);
					setPreCalIdx++;
					setPreCalTotal -= sendLength;
				}
			}
			if (DoPreCal & (1 << DPD5G_PRECAL_INDN_BIT)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DPD 5G Pre-Cal finished!\n");
			}
			if (DoPreCal & (1 << DPD2G_PRECAL_INDN_BIT)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DPD 2G Pre-Cal finished!\n");
			}
		}
	}
#endif /* PRE_CAL_MT7981_SUPPORT */

#ifdef PRE_CAL_MT7916_SUPPORT
	if (IS_MT7916(pAd)) {
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

		/* Check DoPreCal bits */
		RT28xx_EEPROM_READ16(pAd, PRECAL_INDICATION_BYTE, DoPreCal);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\x1b[34m EEPROM 0x%2x %x\x1b[m\n", PRECAL_INDICATION_BYTE, DoPreCal);
		if (pAd->E2pAccessMode == E2P_FLASH_MODE ||
			pAd->E2pAccessMode == E2P_BIN_MODE) {
			if (DoPreCal & (1 << GROUP_PRECAL_INDN_BIT)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Group Pre-Cal finished, load Group Pre-Cal data\n");
				setPreCalTotal = cap->prek_ee_info.pre_cal_total_size;
				while (setPreCalTotal > 0) {
					sendLength = (setPreCalTotal >= PRE_CAL_SET_MAX_LENGTH) ?
								PRE_CAL_SET_MAX_LENGTH : setPreCalTotal;
					MtCmdSetGroupPreCal_7916(pAd, setPreCalIdx, sendLength);
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"idx=%d, sendLength=%d, remainedLength=%d\n",
						setPreCalIdx, sendLength, setPreCalTotal);
					setPreCalIdx++;
					setPreCalTotal -= sendLength;
				}
			}
			if (DoPreCal & (1 << DPD5G_PRECAL_INDN_BIT)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DPD 5G Pre-Cal finished!\n");
			}
			if (DoPreCal & (1 << DPD2G_PRECAL_INDN_BIT)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"DPD 2G Pre-Cal finished!\n");
			}
		}
	}
#endif /* PRE_CAL_MT7986_SUPPORT */

#ifdef PRE_CAL_TRX_SET2_SUPPORT

	PreCalSize = PRE_CAL_SIZE_ONE_CARD;
#ifdef CONFIG_RALINK_MT7621
	/* Litmit PreCalSize to 12k for MT7622 + MT7615 + MT7615 */
#ifdef MULTI_INF_SUPPORT
	if (multi_inf_get_count() >= 2)
		PreCalSize = PRE_CAL_SIZE_DUAL_CARD;
#endif /*MULTI_INF_SUPPORT*/
#endif /*CONFIG_RALINK_MT7621*/

	/* Check DoPreCal bits */
	RT28xx_EEPROM_READ16(pAd, 0x52, DoPreCal);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\x1b[34m EEPROM 0x52 %x\x1b[m\n", DoPreCal);

	/* Pre Cal only supports Bin or Flash Mode */
	if (pAd->E2pAccessMode == E2P_FLASH_MODE || pAd->E2pAccessMode == E2P_BIN_MODE) {
	/* Restore when RLM cache is empty */
		if (!rlmCalCacheDone(pAd->rlmCalCache) && (DoPreCal & (1 << 2))) {
			INT32 ret = 0;
			INT32 ret_cal_data = NDIS_STATUS_SUCCESS;
			ret = os_alloc_mem(pAd, &pAd->PreCalReStoreBuffer, PreCalSize);/* Allocate 16K buffer*/

			if (ret != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "\x1b[42m Not enough memory for pre-cal restored buffer!!\x1b[m\n");
			} else {
				/* Read pre-cal data from flash and store to pre-cal buffer*/
#ifdef RTMP_FLASH_SUPPORT
				if (pAd->E2pAccessMode == E2P_FLASH_MODE)
					RtmpFlashRead(pAd->hdev_ctrl, pAd->PreCalReStoreBuffer,
							get_dev_eeprom_offset(pAd) + PRECALPART_OFFSET, PreCalSize);
#endif /* RTMP_FLASH_SUPPORT */
				if (pAd->E2pAccessMode == E2P_BIN_MODE) {
					ret_cal_data = rtmp_cal_load_from_bin(pAd, pAd->PreCalReStoreBuffer,
							get_dev_eeprom_offset(pAd) + PRECALPART_OFFSET, PreCalSize);

					if (ret_cal_data != NDIS_STATUS_SUCCESS) {
						/* Erase DoPreCal bit */
						DoPreCal &= ~(1 << 2);
						RT28xx_EEPROM_WRITE16(pAd, 0x52, DoPreCal);

						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Reset EEPROM 0x52[2] bit, Let FW do On-line Calibration\n");
					}
				}

				if (ret_cal_data == NDIS_STATUS_SUCCESS)
					ret = MtCmdPreCalReStoreProc(pAd, (INT32 *)pAd->PreCalReStoreBuffer);
			}
		}
	} else {
		/* Force Erase DoPreCal bit for any mode other than Bin & Flash*/
		DoPreCal &= ~(1 << 2);
		RT28xx_EEPROM_WRITE16(pAd, 0x52, DoPreCal);
	}
#endif /* PRE_CAL_TRX_SET2_SUPPORT */

#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
	if (IS_MT7615(pAd)) {
		/* Check DoPATrim bits */
		RT28xx_EEPROM_READ16(pAd, 0x52, DoPATrim);

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"\x1b[34m EEPROM 0x52 %x\x1b[m\n", DoPATrim);

		/* Restore PA data when EEPROM 0x52[3]=1 */
		if (DoPATrim & (1 << 3)) {
			INT32 Status = NDIS_STATUS_FAILURE;

			Status = MtCmdCalReStoreFromFileProc(pAd, CAL_RESTORE_PA_TRIM);

			if (Status != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"\x1b[41m Fail to restore PA data!!\x1b[m\n");
			}
		}
	}
#endif /* CAL_BIN_FILE_SUPPORT */


#ifdef MT_DFS_SUPPORT	/*Dynamically enable or disable DFS calibration in firmware. Must be performed before power on calibration*/
	DfsSetCalibration(pAd, pAd->CommonCfg.DfsParameter.DisableDfsCal);
#endif
#ifdef MT_MAC
	/*Send EEprom parameter to FW*/
#ifdef CONFIG_ATE

	if (!ATE_ON(pAd))
#endif
	{
#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT
		rtmp_eeprom_info_extract(pAd);
#endif /*#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT*/
#ifdef CONFIG_6G_SUPPORT
#ifdef DBDC_MODE
		/* Change eeprom band selection value by wirelessmode for FW */
		if (ops->eep_set_band_sel)
			ops->eep_set_band_sel(pAd, &EeprAntCfg);
#endif /* DBDC_MODE */
#endif /* CONFIG_6G_SUPPORT */
		/*configure PA/LNA setting by efuse*/
		chip_pa_lna_set(pAd, pAd->EEPROMImage);
		/*configure TSSI sestting by efuse*/
		chip_tssi_set(pAd, pAd->EEPROMImage);
		MtCmdEfusBufferModeSet(pAd, pAd->eeprom_type);

#ifdef CONFIG_6G_SUPPORT
#ifdef DBDC_MODE
		/* Recover eeprom band selection value after fw updated*/
		if (ops->eep_set_band_sel && EeprAntCfg != NULL)
			ops->eep_set_band_sel(pAd, &EeprAntCfg);
#endif /* DBDC_MODE */
#endif /* CONFIG_6G_SUPPORT */
	}

#ifdef COEX_SUPPORT
	if (IS_MT7622(pAd))
		mt7622_antenna_sel_cfg(pAd);
#endif

#endif /* MT_MAC */
	/* Read MAC setting from EEPROM and record as permanent MAC address */
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Initialize MAC Address from E2PROM\n");
	RT28xx_EEPROM_READ16(pAd, 0x04, Addr01);
	RT28xx_EEPROM_READ16(pAd, 0x06, Addr23);
	RT28xx_EEPROM_READ16(pAd, 0x08, Addr45);
	pAd->PermanentAddress[0] = (UCHAR)(Addr01 & 0xff);
	pAd->PermanentAddress[1] = (UCHAR)(Addr01 >> 8);
	pAd->PermanentAddress[2] = (UCHAR)(Addr23 & 0xff);
	pAd->PermanentAddress[3] = (UCHAR)(Addr23 >> 8);
	pAd->PermanentAddress[4] = (UCHAR)(Addr45 & 0xff);
	pAd->PermanentAddress[5] = (UCHAR)(Addr45 >> 8);

	/*more conveninet to test mbssid, so ap's bssid &0xf1*/
	if (pAd->PermanentAddress[0] == 0xff)
		pAd->PermanentAddress[0] = RandomByte(pAd) & 0xf8;

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "E2PROM MAC: ="MACSTR"\n",
			 MAC2STR(pAd->PermanentAddress));

	/* Assign the actually working MAC Address */
	if (pAd->bLocalAdminMAC) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Use the MAC address what is assigned from Configuration file(.dat).\n");
#if defined(BB_SOC) && !defined(NEW_MBSSID_MODE)
		/* BBUPrepareMAC(pAd, pAd->CurrentAddress); */
		COPY_MAC_ADDR(pAd->PermanentAddress, pAd->CurrentAddress);
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"now bb MainSsid mac "MACSTR"\n", MAC2STR(pAd->CurrentAddress));
#endif
	} else if (mac_addr &&
			   strlen((RTMP_STRING *)mac_addr) == 17 &&
			   (strcmp(mac_addr, "00:00:00:00:00:00") != 0)) {
		INT j;
		RTMP_STRING *macptr;
		macptr = (RTMP_STRING *) mac_addr;

		for (j = 0; j < MAC_ADDR_LEN; j++) {
			AtoH(macptr, &pAd->CurrentAddress[j], 1);
			macptr = macptr + 3;
		}

		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Use the MAC address what is assigned from Moudle Parameter.\n");
	} else {
		COPY_MAC_ADDR(pAd->CurrentAddress, pAd->PermanentAddress);
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Use the MAC address what is assigned from EEPROM.\n");
	}

	/* if E2PROM version mismatch with driver's expectation, then skip*/
	/* all subsequent E2RPOM retieval and set a system error bit to notify GUI*/
	RT28xx_EEPROM_READ16(pAd, EEPROM_VERSION_OFFSET, Version.word);
	pAd->EepromVersion = Version.field.Version + Version.field.FaeReleaseNumber * 256;
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "E2PROM: Version = %d, FAE release #%d\n", Version.field.Version,
			 Version.field.FaeReleaseNumber);
	/* Read BBP default value from EEPROM and store to array(EEPROMDefaultValue) in pAd */
	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC1_OFFSET, value);
	pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET] = value;
	/* EEPROM offset 0x36 - NIC Configuration 1 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC2_OFFSET, value);
	pAd->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET] = value;
	NicConfig2.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET];
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_COUNTRY_REGION, value);	/* Country Region*/
		pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] = value;
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Country Region from e2p = %x\n", value);
	}

	for (i = 0; i < 8; i++) {
#if defined(RLT_MAC) || defined(MT_MAC)

		/* TODO: shiang-MT7603, replace below chip based check to this one!! */
		if (IS_HIF_TYPE(pAd, HIF_RLT) || IS_HIF_TYPE(pAd, HIF_MT))
			break;

#endif /* defined(RLT_MAC) || defined(MT_MAC) */
		RT28xx_EEPROM_READ16(pAd, EEPROM_BBP_BASE_OFFSET + i * 2, value);
		pAd->EEPROMDefaultValue[i + EEPROM_BBP_ARRAY_OFFSET] = value;
	}

	/* We have to parse NIC configuration 0 at here. */
	/* If TSSI did not have preloaded value, it should reset the TxAutoAgc to false */
	/* Therefore, we have to read TxAutoAgc control beforehand. */
	/* Read Tx AGC control bit */
	Antenna.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET];

	/* TODO: shiang, why we only check oxff00?? */
	if (((Antenna.word & 0xFF00) == 0xFF00))
		/*	if (Antenna.word == 0xFFFF)*/
		RTMP_CHIP_ANTENNA_INFO_DEFAULT_RESET(pAd, &Antenna);

		RTMP_CHIP_ANTENNA_INFO_DEFAULT_RESET(pAd, &Antenna);

#ifdef WSC_INCLUDED

	/* WSC hardware push button function 0811 */
	if ((pAd->MACVersion == 0x28600100) || (pAd->MACVersion == 0x28700100))
		WSC_HDR_BTN_MR_HDR_SUPPORT_SET(pAd, NicConfig2.field.EnableWPSPBC);

#endif /* WSC_INCLUDED */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (NicConfig2.word == 0xffff)
			NicConfig2.word = 0;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if ((NicConfig2.word & 0x00ff) == 0xff)
			NicConfig2.word &= 0xff00;

		if ((NicConfig2.word >> 8) == 0xff)
			NicConfig2.word &= 0x00ff;
	}
#endif /* CONFIG_STA_SUPPORT */

	/* Save value for future using */
	pAd->NicConfig2.word = NicConfig2.word;
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RxPath = %d, TxPath = %d, RfIcType = %d\n",
			 Antenna.field.RxPath, Antenna.field.TxPath,
			 Antenna.field.RfIcType);
	/* Save the antenna for future use*/
	pAd->Antenna.word = Antenna.word;
	/* Set the RfICType here, then we can initialize RFIC related operation callbacks*/
	pAd->Mlme.RealRxPath = (UCHAR) Antenna.field.RxPath;
	pAd->RfIcType = (UCHAR) Antenna.field.RfIcType;
#ifdef MT7986
	if (IS_MT7986(pAd))
		pAd->RfIcType = RFIC_7986;
#endif
#ifdef MT7916
	if (IS_MT7916(pAd))
		pAd->RfIcType = RFIC_7916;
#endif
#ifdef MT7981
	if (IS_MT7981(pAd))
		pAd->RfIcType = RFIC_7981;
#endif


#ifdef LED_CONTROL_SUPPORT
	rtmp_read_led_setting_from_eeprom(pAd);
#endif /* LED_CONTROL_SUPPORT */
	rtmp_read_txpwr_from_eeprom(pAd);
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pAd->Antenna.field.BoardType = %d\n",
			 pAd->Antenna.field.BoardType);
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--\n");
	return TRUE;
}

#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT
void rtmp_eeprom_info_extract(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (chip_ops->eeprom_extract) {
		chip_ops->eeprom_extract(pAd, (VOID *)pAd->EEPROMImage);
	}
}

NDIS_STATUS rtmp_eeprom_Sys_update(
	RTMP_ADAPTER * pAd,
	UCHAR GetOrSet,
	UCHAR ColumnIdx,
	UCHAR *value
)
{
	PUINT8 pu1Param = NULL;

	/* get pointer to system module */
	pu1Param = (PUINT8)(pAd->EfuseInfoAll.prSys);

	/*Sanity check*/
	if (!pu1Param)
		return NDIS_STATUS_FAILURE;

	/* Set/Get Efuse Info value */
	if (GetOrSet == ENUM_EFUSE_INFO_SET)
		*(pu1Param + ColumnIdx) = *value;
	else
		*value = *(pu1Param + ColumnIdx);
	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS rtmp_eeprom_WfCmm_update(
	RTMP_ADAPTER * pAd,
	UCHAR eBand,
	ENUM_EFUSE_COMMON_CATOGORY eCmmType,
	UCHAR GetOrSet,
	UCHAR ChGroup,
	UCHAR ColumnIdx,
	UCHAR *value
)
{
	PUINT8 pu1Param = NULL;
	BOOLEAN  fgGBand;

								/* BAND_5G						BAND_2G4 */
	UINT_8 u1StartAdd[EFUSE_COMMON_CATOGORY_NUM][BAND_NUM] = {{EFUSE_INFO_5G_COMMON_TEMP_TSSI_OFFSET, EFUSE_INFO_2G4_COMMON_TEMP_TSSI_OFFSET},
								  {EFUSE_INFO_5G_COMMON_DYNAMIC_IPA_SWITCH_OFFSET, EFUSE_INFO_2G4_COMMON_DYNAMIC_IPA_SWITCH_OFFSET},
								  {EFUSE_INFO_5G_COMMON_ELNA_OFFSET, EFUSE_INFO_2G4_COMMON_ELNA_OFFSET},
								  {EFUSE_INFO_5G_COMMON_THERMO_COMP_OFFSET, EFUSE_INFO_2G4_COMMON_THERMO_COMP_OFFSET},
								  {EFUSE_INFO_5G_COMMON_EPA_DPD_OFFSET, EFUSE_INFO_2G4_COMMON_EPA_DPD_OFFSET}
								};
								/* BAND_5G		BAND_2G4 */
	UINT_8 u1GroupNum[EFUSE_COMMON_CATOGORY_NUM][BAND_NUM] = {{8, 1},
								  {SINGLE_GROUP, SINGLE_GROUP},
								  {3, 1},
								  {SINGLE_GROUP, SINGLE_GROUP},
								  {SINGLE_GROUP, SINGLE_GROUP}
								};
								 /* BAND_5G						BAND_2G4 */
	UINT_8 u1ColumnNum[EFUSE_COMMON_CATOGORY_NUM][BAND_NUM] = {{EFUSE_COMMON_TEMP_TSSI_CATEGORY_ITEM_NUM, EFUSE_COMMON_TEMP_TSSI_CATEGORY_ITEM_NUM},
								   {EFUSE_COMMON_DYNAMIC_IPA_SWITCH_CATEGORY_ITEM_NUM, EFUSE_COMMON_DYNAMIC_IPA_SWITCH_CATEGORY_ITEM_NUM},
								   {EFUSE_5G_COMMON_ELNA_CATEGORY_ITEM_NUM, EFUSE_2G4_COMMON_ELNA_CATEGORY_ITEM_NUM},
								   {EFUSE_COMMON_THERMO_COMP_CATEGORY_ITEM_NUM, EFUSE_COMMON_THERMO_COMP_CATEGORY_ITEM_NUM},
								   {EFUSE_COMMON_EPA_DPD_CATEGORY_ITEM_NUM, EFUSE_COMMON_EPA_DPD_CATEGORY_ITEM_NUM}
								};

	/* check Channel Band */
	fgGBand = (eBand == BAND_24G) ? (TRUE) : (FALSE);

	/* get pointer to common module */
	pu1Param = (eBand == BAND_24G) ? ((PUINT8)pAd->EfuseInfoAll.pr2G4Cmm) : ((PUINT8)pAd->EfuseInfoAll.pr5GCmm);

	/*Sanity check*/
	if (!pu1Param)
		return NDIS_STATUS_FAILURE;
	if (GetOrSet >= ENUM_EFUSE_INFO_MODE_NUM)
		return NDIS_STATUS_FAILURE;

	if (eCmmType >= EFUSE_COMMON_CATOGORY_NUM)
		return NDIS_STATUS_FAILURE;

	if (ChGroup >= u1GroupNum[eCmmType][fgGBand])
		return NDIS_STATUS_FAILURE;

	if (ColumnIdx >= u1ColumnNum[eCmmType][fgGBand])
		return NDIS_STATUS_FAILURE;

	/* get pointer to Common Specific Category */
	pu1Param += u1StartAdd[eCmmType][fgGBand];

	/* get pointer to Common Specific channel group
	If no group info, please fill CH_GROUP_NOT_CARE for ChGroup*/
	if (ChGroup == CH_GROUP_NOT_CARE)
		;
	else
		pu1Param += (ChGroup * u1ColumnNum[eCmmType][fgGBand]);

	/* Set/Get Efuse Info value */
	if (GetOrSet == ENUM_EFUSE_INFO_SET)
		*(pu1Param + ColumnIdx) = *value;
	else
		*value = *(pu1Param + ColumnIdx);
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS rtmp_eeprom_WfPath_update(
	RTMP_ADAPTER * pAd,
	UCHAR eBand,
	ENUM_EFUSE_WF_PATH_CATOGORY eWFPathType,
	UCHAR AntIdx,
	UCHAR GetOrSet,
	UCHAR ChGroup,
	UCHAR ColumnIdx,
	UCHAR *value
)
{
	PUINT8 pu1Param = NULL;
	BOOLEAN  fgGBand;

	UINT_8 u1StartAdd[EFUSE_WF_PATH_CATOGORY_NUM][BAND_NUM] = {{EFUSE_INFO_5G_WF_PATH_FE_LOSS_OFFSET, EFUSE_INFO_2G4_WF_PATH_FE_LOSS_OFFSET},
								   {EFUSE_INFO_5G_WF_PATH_TSSI_OFFSET, EFUSE_INFO_2G4_WF_PATH_TSSI_OFFSET},
								   {EFUSE_INFO_5G_WF_PATH_PHY_POWER_OFFSET, EFUSE_INFO_2G4_WF_PATH_PHY_POWER_OFFSET},
								   {EFUSE_INFO_5G_WF_PATH_TX_POWER_OFFSET_OFFSET, EFUSE_INFO_2G4_WF_PATH_TX_POWER_OFFSET_OFFSET},
								   {EFUSE_INFO_5G_WF_PATH_DPD_G0_OFFSET, EFUSE_INFO_2G4_WF_PATH_DPD_G0_OFFSET},
								   {EFUSE_INFO_5G_WF_PATH_LNA_GAIN_OFFSET, EFUSE_INFO_2G4_WF_PATH_LNA_GAIN_OFFSET}
								};
								 /* BAND_5G		 BAND_2G4 */
	UINT_8 u1GroupNum[EFUSE_WF_PATH_CATOGORY_NUM][BAND_NUM] = {{3, 1},
								   {8, 1},
								   {SINGLE_GROUP, SINGLE_GROUP},
								   {8, 1},
								   {4, 1},
								   {8, 1}
								};
								  /* BAND_5G						 BAND_2G4 */
	UINT_8 u1ColumnNum[EFUSE_WF_PATH_CATOGORY_NUM][BAND_NUM] = {{EFUSE_5G_WF_PATH_FE_LOSS_CATEGORY_ITEM_NUM, EFUSE_2G4_WF_PATH_FE_LOSS_CATEGORY_ITEM_NUM},
								    {EFUSE_WF_PATH_TSSI_CATEGORY_ITEM_NUM, EFUSE_WF_PATH_TSSI_CATEGORY_ITEM_NUM},
								    {EFUSE_WF_PATH_PHY_POWER_CATEGORY_ITEM_NUM, EFUSE_WF_PATH_PHY_POWER_CATEGORY_ITEM_NUM},
								    {EFUSE_5G_WF_PATH_TX_POWER_OFFSET_CATEGORY_ITEM_NUM, EFUSE_2G4_WF_PATH_TX_POWER_OFFSET_CATEGORY_ITEM_NUM},
								    {EFUSE_WF_PATH_DPD_G0_CATEGORY_ITEM_NUM, EFUSE_WF_PATH_DPD_G0_CATEGORY_ITEM_NUM},
								    {EFUSE_WF_PATH_LNA_GAIN_CATEGORY_ITEM_NUM, EFUSE_WF_PATH_LNA_GAIN_CATEGORY_ITEM_NUM}
								};

	/* check Channel Band */
	fgGBand = (eBand == BAND_24G) ? (TRUE) : (FALSE);

	/*Sanity check*/
	if (GetOrSet >= ENUM_EFUSE_INFO_MODE_NUM)
		return NDIS_STATUS_FAILURE;
	if (eWFPathType >= EFUSE_WF_PATH_CATOGORY_NUM)
		return NDIS_STATUS_FAILURE;
	if (AntIdx >= MAX_ANTENNA_NUM)
		return NDIS_STATUS_FAILURE;
	if (ChGroup >= u1GroupNum[eWFPathType][fgGBand])
		return NDIS_STATUS_FAILURE;
	if (ColumnIdx >= u1ColumnNum[eWFPathType][fgGBand])
		return NDIS_STATUS_FAILURE;

	/* get pointer to common module */
	pu1Param = (eBand == BAND_24G) ? ((PUINT8)pAd->EfuseInfoAll.pr2G4WFPath[AntIdx]) : ((PUINT8)pAd->EfuseInfoAll.pr5GWFPath[AntIdx]);

	if (!pu1Param)
		return NDIS_STATUS_FAILURE;

	/* get pointer to Common Specific Category */
	pu1Param += u1StartAdd[eWFPathType][fgGBand];

	/* get pointer to Common Specific channel group
	If no group info, please fill CH_GROUP_NOT_CARE for ChGroup*/
	if (ChGroup == CH_GROUP_NOT_CARE)
		;
	else
		pu1Param += (ChGroup * u1ColumnNum[eWFPathType][fgGBand]);

	/* Set/Get Efuse Info value */
	if (GetOrSet == ENUM_EFUSE_INFO_SET)
		*(pu1Param + ColumnIdx) = *value;
	else
		*value = *(pu1Param + ColumnIdx);

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS rtmp_eeprom_TxPwr_update(
	RTMP_ADAPTER * pAd,
	UCHAR eBand,
	EFUSE_INFO_TX_POWER_CATEGORY eTxPwrParamCat,
	UCHAR GetOrSet,
	UCHAR ColumnIdx,
	UCHAR *value
)
{
	PUINT8 pu1Param = NULL;
	BOOLEAN  fgGBand;
	UCHAR ColumnNum = 0;

	/* get pointer to common module */
	switch (eTxPwrParamCat) {
	case CATEGORY_NTX_POWER_OFFSET:
		pu1Param = (PUINT8)(pAd->EfuseInfoAll.prNTxPowerOffset);
		ColumnNum = EFUSE_NTX_POWER_OFFSET_ITEM_NUM;
		break;
	case CATEGORY_TX_POWER_RATE:
		fgGBand = (eBand == BAND_24G) ? (TRUE) : (FALSE);
		pu1Param = (eBand == BAND_24G) ? ((PUINT8)(pAd->EfuseInfoAll.pr2G4TxPower)) : ((PUINT8)(pAd->EfuseInfoAll.pr5GTxPower));
		if (fgGBand == TRUE)
			ColumnNum = EFUSE_2G4_TXPOWER_RATE_POWER_ITEM_NUM;
		else
			ColumnNum = EFUSE_5G_TXPOWER_RATE_POWER_ITEM_NUM;
		break;
	default:
		break;
	}

	/*Sanity check*/
	if (!pu1Param)
		return NDIS_STATUS_FAILURE;
	if (GetOrSet >= ENUM_EFUSE_INFO_MODE_NUM)
		return NDIS_STATUS_FAILURE;
	if (ColumnIdx >= ColumnNum)
		return NDIS_STATUS_FAILURE;

	/* Set/Get Efuse Info value */
	if (GetOrSet == ENUM_EFUSE_INFO_SET)
		*(pu1Param + ColumnIdx) = *value;
	else
		*value = *(pu1Param + ColumnIdx);

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS rtmp_eeprom_IBF_update(
	RTMP_ADAPTER * pAd,
	UCHAR eBand,
	UCHAR GetOrSet,
	UCHAR ColumnIdx,
	UCHAR *value
)
{
	PUINT8 pu1Param = NULL;
	UCHAR ColumnNum;
	BOOLEAN  fgGBand;
	/* get pointer to common module */
	pu1Param = (eBand == BAND_24G) ? ((PUINT8)(pAd->EfuseInfoAll.pr2G4IBfParam)) : ((PUINT8)(pAd->EfuseInfoAll.pr5GIBfParam));
	fgGBand = (eBand == BAND_24G) ? (TRUE) : (FALSE);
	if (fgGBand == TRUE)
		ColumnNum = IBF_PARAM_NUM * G_BAND_CH_GROUP_NUM;
	else
		ColumnNum = IBF_PARAM_NUM * A_BAND_CH_GROUP_NUM;

	/*Sanity check*/
	if (!pu1Param)
		return NDIS_STATUS_FAILURE;
	if (GetOrSet >= ENUM_EFUSE_INFO_MODE_NUM)
		return NDIS_STATUS_FAILURE;
	if (ColumnIdx >= ColumnNum)
		return NDIS_STATUS_FAILURE;

	/* Set/Get Efuse Info value */
	if (GetOrSet == ENUM_EFUSE_INFO_SET)
		*(pu1Param + ColumnIdx) = *value;
	else
		*value = *(pu1Param + ColumnIdx);

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS rtmp_eeprom_DelayComp_update(
	struct _RTMP_ADAPTER *pAd,
	UCHAR eBand,
	UCHAR GetOrSet,
	EFUSE_INFO_DELAY_COMP_CATEGORY eDelayCompParamCat,
	UCHAR ColumnIdx,
	UCHAR *value
)
{
	PUINT8 pu1Param = NULL;

	/* get pointer to common module */
	switch (eDelayCompParamCat) {
	case CATEGORY_DEALY_COMP_CTRL:
		pu1Param = (PUINT8)(pAd->EfuseInfoAll.prDelayCompCtrl);
		break;
	case CATEGORY_RX_LATENCY:
		pu1Param = (eBand == BAND_24G) ? ((PUINT8)(pAd->EfuseInfoAll.pr2G4RxLatency)) : ((PUINT8)(pAd->EfuseInfoAll.pr5GRxLatency));
		break;
	default:
		break;
	}

	/* sanity check for pointer null status */
	if (!pu1Param)
		return NDIS_STATUS_FAILURE;

	/* Set/Get Efuse Info value */
	if (GetOrSet == ENUM_EFUSE_INFO_SET)
		*(pu1Param + ColumnIdx) = *value;
	else
		*value = *(pu1Param + ColumnIdx);

	return NDIS_STATUS_SUCCESS;
}

INT32 show_UpdateEfuse_Example(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR value = 0xB3;
	int status;
	/*Example for set 2.4G Dymaic iPA switch : PA CCK Threshold (chip out) for HPA: 0xB6*/
	MTWF_PRINT("\x1b[1;33m Before change: EEPROM[0x%x] is 0x%x \x1b[m \n",
		 0xB6, pAd->EEPROMImage[0xB6]);
	status = rtmp_eeprom_WfCmm_update(pAd, BAND_24G,
		 EFUSE_COMMON_CATEGORY_DYNAMIC_IPA_SWITCH, ENUM_EFUSE_INFO_SET,
		 CH_GROUP_NOT_CARE,
		 EFUSE_COMMON_DYNAMIC_IPA_SWITCH_CATEGORY_HPA_CCK_THRESHOLD,
		 &value);

	MTWF_PRINT("\x1b[1;33m After change: EEPROM[0x%x] is 0x%x \x1b[m \n",
		 0xB6, pAd->EEPROMImage[0xB6]);
	return TRUE;
}
#endif /*#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT*/

void rtmp_eeprom_of_platform(RTMP_ADAPTER *pAd)
{
	UCHAR e2p_dafault;
	e2p_dafault = E2P_NONE;
#ifdef CONFIG_RT_FIRST_CARD_EEPROM

	if (pAd->dev_idx == 0) {
		if (RTMPEqualMemory("efuse", CONFIG_RT_FIRST_CARD_EEPROM, 5))
			e2p_dafault = E2P_EFUSE_MODE;

		if (RTMPEqualMemory("prom", CONFIG_RT_FIRST_CARD_EEPROM, 4))
			e2p_dafault = E2P_EEPROM_MODE;

		if (RTMPEqualMemory("flash", CONFIG_RT_FIRST_CARD_EEPROM, 5))
			e2p_dafault = E2P_FLASH_MODE;

		pAd->E2pAccessMode = e2p_dafault;
	}

#endif /* CONFIG_RT_FIRST_CARD_EEPROM */
#ifdef CONFIG_RT_SECOND_CARD_EEPROM

	if (pAd->dev_idx == 1) {
		if (RTMPEqualMemory("efuse", CONFIG_RT_SECOND_CARD_EEPROM, 5))
			e2p_dafault = E2P_EFUSE_MODE;

		if (RTMPEqualMemory("prom", CONFIG_RT_SECOND_CARD_EEPROM, 4))
			e2p_dafault = E2P_EEPROM_MODE;

		if (RTMPEqualMemory("flash", CONFIG_RT_SECOND_CARD_EEPROM, 5))
			e2p_dafault = E2P_FLASH_MODE;

		pAd->E2pAccessMode = e2p_dafault;
	}

#endif /* CONFIG_RT_SECOND_CARD_EEPROM */
#ifdef CONFIG_RT_THIRD_CARD_EEPROM

	if (pAd->dev_idx == 2) {
		if (RTMPEqualMemory("efuse", CONFIG_RT_THIRD_CARD_EEPROM, 5))
			e2p_dafault = E2P_EFUSE_MODE;

		if (RTMPEqualMemory("prom", CONFIG_RT_THIRD_CARD_EEPROM, 4))
			e2p_dafault = E2P_EEPROM_MODE;

		if (RTMPEqualMemory("flash", CONFIG_RT_THIRD_CARD_EEPROM, 5))
			e2p_dafault = E2P_FLASH_MODE;

		pAd->E2pAccessMode = e2p_dafault;
	}

#endif /* CONFIG_RT_THIRD_CARD_EEPROM */
}

UCHAR RtmpEepromGetDefault(RTMP_ADAPTER *pAd)
{
	UCHAR e2p_dafault = 0;
#ifdef RTMP_FLASH_SUPPORT

	if (pAd->infType == RTMP_DEV_INF_RBUS)
		e2p_dafault = E2P_FLASH_MODE;
	else
#endif /* RTMP_FLASH_SUPPORT */
	{
#ifdef RTMP_EFUSE_SUPPORT

		if (pAd->bUseEfuse)
			e2p_dafault = E2P_EFUSE_MODE;
		else
#endif /* RTMP_EFUSE_SUPPORT */
			e2p_dafault = E2P_EEPROM_MODE;
	}

	/* SDIO load from NVRAM(BIN FILE) */
	if (pAd->infType == RTMP_DEV_INF_SDIO)
		e2p_dafault = E2P_BIN_MODE;

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "e2p_dafault=%d\n", e2p_dafault);
	return e2p_dafault;
}


static NDIS_STATUS rtmp_ee_bin_init(PRTMP_ADAPTER pAd)
{
	rtmp_ee_load_from_bin(pAd);
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	{
		os_alloc_mem(pAd, &pAd->CalDCOCImage, DCOC_IMAGE_SIZE);
		os_alloc_mem(pAd, &pAd->CalDPDAPart1Image, TXDPD_IMAGE1_SIZE);
		os_alloc_mem(pAd, &pAd->CalDPDAPart2Image, TXDPD_IMAGE2_SIZE);
		NdisZeroMemory(pAd->CalDCOCImage, DCOC_IMAGE_SIZE);
		NdisZeroMemory(pAd->CalDPDAPart1Image, TXDPD_IMAGE1_SIZE);
		NdisZeroMemory(pAd->CalDPDAPart2Image, TXDPD_IMAGE2_SIZE);

		if (NDIS_STATUS_SUCCESS == rtmp_cal_load_from_bin(pAd, pAd->CalDCOCImage, 0, DCOC_IMAGE_SIZE))
			pAd->bDCOCReloaded = TRUE;
		else
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"bDCOCReloaded = false.\n");

		if ((NDIS_STATUS_SUCCESS ==
				rtmp_cal_load_from_bin(pAd, pAd->CalDPDAPart1Image, DPDPART1_OFFSET, TXDPD_IMAGE1_SIZE)) &&
			(NDIS_STATUS_SUCCESS ==
				rtmp_cal_load_from_bin(pAd, pAd->CalDPDAPart2Image, DPDPART2_OFFSET, TXDPD_IMAGE2_SIZE)))
			pAd->bDPDReloaded = TRUE;
		else
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"bDPDReloaded = false.\n");
	}
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
#if defined(PRE_CAL_MT7915_SUPPORT) || defined(PRE_CAL_MT7986_SUPPORT) || defined(PRE_CAL_MT7916_SUPPORT) || defined(PRE_CAL_MT7981_SUPPORT)
	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
		USHORT prek_ee_offset = cap->EEPROM_DEFAULT_BIN_SIZE + cap->prek_ee_info.info_size;
		UINT32 dpdk_ee_offset = prek_ee_offset + cap->prek_ee_info.pre_cal_total_size;

		pAd->PreCalImageInfo = pAd->EEPROMImage + cap->EEPROM_DEFAULT_BIN_SIZE;
		pAd->PreCalImage     = pAd->EEPROMImage + prek_ee_offset;
		pAd->TxDPDImage      = pAd->EEPROMImage + dpdk_ee_offset;
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"\x1b[42m[EEPROMImage - PreCalImageInfo - PreCalImage - TxDPDImage]\x1b[m\n"
				"\x1b[42m[0x%p - 0x%p - 0x%p - 0x%p]\x1b[m\n",
				pAd->EEPROMImage, pAd->PreCalImageInfo, pAd->PreCalImage, pAd->TxDPDImage);
	}
#endif
#if defined(MT7986) || defined(MT7916) || defined(MT7915) || defined(MT7981)
	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
		USHORT rxgaincal_offset = cap->rxgaincal_ofst;
		pAd->RXGainCal       = pAd->EEPROMImage + rxgaincal_offset;
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"\x1b[42m[EEPROMImage - RXGainCal]\x1b[m\n"
				"\x1b[42m[0x%p - 0x%p]\x1b[m\n",
				pAd->EEPROMImage, pAd->RXGainCal);
	}
#endif /* defined(MT7986) || defined(MT7916) || defined(MT7915) || defined(MT7981) */
	return NDIS_STATUS_SUCCESS;
}

extern UINT32 get_dev_eeprom_size(VOID *ad);
extern UINT32 get_dev_eeprom_offset(VOID *ad);

static NDIS_STATUS rtmp_ee_fr_host(
	struct _RTMP_ADAPTER *ad,
	VOID *cmd,
	UINT8 cmd_seq,
	UINT8 cmd_total)
{
	INT ret = NDIS_STATUS_FAILURE;
	UINT16 ctrl_msg = 0;
#ifdef CONFIG_MT7976_SUPPORT
	EEPROM_PWR_ON_MODE_T mode;
#endif /* CONFIG_MT7976_SUPPORT */
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	ctrl_msg = (EEPROM_MODE_BUFFER << SOURCE_MODE_SHIFT);

#ifdef CONFIG_MT7976_SUPPORT
	/* only MT7915 need to overwrite the mode to 2G + 6G */
	if (IS_MT7915(ad)) {
		mode = get_power_on_cal_mode(ad);
		ctrl_msg |= (mode << PWR_ON_CAL_SEL_SHIFT);
	}
#endif /* CONFIG_MT7976_SUPPORT */


#if defined(MT7663) || defined(AXE) || defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	/* only new chips support multiple fw cmds to cascade EEPROM data */
	if (IS_MT7663(ad) || IS_AXE(ad) || IS_MT7915(ad) || IS_MT7986(ad) || IS_MT7916(ad) || IS_MT7981(ad)) {
		ctrl_msg |= (CONTENT_FORMAT_WHOLE_CONTENT << 8);
		ctrl_msg |= ((((cmd_total-1) << BUFFER_BIN_TOTAL_PAGE_SHIFT) & BUFFER_BIN_TOTAL_PAGE_MASK) << 8);
		ctrl_msg |= (((cmd_seq << BUFFER_BIN_PAGE_INDEX_SHIFT) & BUFFER_BIN_PAGE_INDEX_MASK) << 8);
	}
#endif

	if (ops->bufferModeCmdFill) {
		ops->bufferModeCmdFill(ad, cmd, ctrl_msg);
		ret = NDIS_STATUS_SUCCESS;
	} else {
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "invalid ops for filling cmd, dismissed!\n");
	}

	return ret;
}

#ifdef RTMP_EFUSE_SUPPORT
static NDIS_STATUS rtmp_ee_fr_mcu(
	struct _RTMP_ADAPTER *ad,
	VOID *cmd,
	UINT8 cmd_seq,
	UINT8 cmd_total)
{
	INT ret = NDIS_STATUS_FAILURE;
	UINT16 ctrl_msg = 0;
#ifdef CONFIG_MT7976_SUPPORT
	EEPROM_PWR_ON_MODE_T mode;
#endif /* CONFIG_MT7976_SUPPORT */
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);
	ctrl_msg = (EEPROM_MODE_EFUSE << SOURCE_MODE_SHIFT);

#ifdef CONFIG_MT7976_SUPPORT
	/* only MT7915 need to overwrite the mode to 2G + 6G */
	if (IS_MT7915(ad)) {
		mode = get_power_on_cal_mode(ad);
		ctrl_msg |= (mode << PWR_ON_CAL_SEL_SHIFT);
	}
#endif /* CONFIG_MT7976_SUPPORT */

#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT
	ctrl_msg |= (CONTENT_FORMAT_WHOLE_CONTENT << 8);
#endif

	if (ops->bufferModeCmdFill) {
		ops->bufferModeCmdFill(ad, cmd, ctrl_msg);
		ret = NDIS_STATUS_SUCCESS;
	} else {
		MTWF_DBG(ad, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "invalid ops for filling cmd, dismissed!\n");
	}

	return ret;
}
#endif /* RTMP_EFUSE_SUPPORT */

INT RtmpChipOpsEepromHook(RTMP_ADAPTER *pAd, INT infType, INT forceMode)
{
	RTMP_CHIP_OP *pChipOps = hc_get_chip_ops(pAd->hdev_ctrl);
	UCHAR e2p_type;
#ifdef RTMP_PCI_SUPPORT
	UINT32 val = 0;
#endif /* RTMP_PCI_SUPPORT */
	UCHAR e2p_default = 0;
	EEPROM_CONTROL *pE2pCtrl = &pAd->E2pCtrl;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
		return -1;

#ifdef RTMP_EFUSE_SUPPORT
	efuse_probe(pAd);
#endif /* RTMP_EFUSE_SUPPORT */

	/* rtmp_eeprom_of_platform(pAd);  //for MT7615, only use E2pAccessMode parameter to get eeprom type */

	if (forceMode != E2P_NONE && forceMode < NUM_OF_E2P_MODE) {
		e2p_type = forceMode;
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "forceMode: %d , infType: %d\n",
				 e2p_type, infType);
	} else {
		e2p_type = pAd->E2pAccessMode;
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "e2p_type=%d, inf_Type=%d\n",
				 e2p_type, infType);
		e2p_default = RtmpEepromGetDefault(pAd);
		/* If e2p_type is out of range, get the default mode */
		e2p_type = ((e2p_type != 0) && (e2p_type < NUM_OF_E2P_MODE)) ? e2p_type : e2p_default;

		if (pAd->E2pAccessMode == E2P_NONE)
			pAd->E2pAccessMode = e2p_default;

		if (infType == RTMP_DEV_INF_RBUS) {
			e2p_type = E2P_FLASH_MODE;
		}

#ifdef RTMP_EFUSE_SUPPORT

		if (e2p_type != E2P_EFUSE_MODE)
			pAd->bUseEfuse = FALSE;

#endif /* RTMP_EFUSE_SUPPORT */
#if defined(MT7636_FPGA) || defined(MT7637_FPGA) || defined(MT7915_FPGA) || defined(MT7986_FPGA) || defined(MT7981_FPGA)
		e2p_type = E2P_BIN_MODE;
#endif /* RTMP_EFUSE_SUPPORT */
	}

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "E2P type(%d), E2pAccessMode = %d, E2P default = %d\n",
			 e2p_type, pAd->E2pAccessMode, e2p_default);
	pAd->eeprom_type = (e2p_type == E2P_EFUSE_MODE)  ? EEPROM_EFUSE : EEPROM_FLASH;
	pE2pCtrl->e2pCurMode = e2p_type;

	if (pAd->EEPROMImage == NULL)
		os_alloc_mem(pAd, &pAd->EEPROMImage,
				(get_dev_eeprom_size(pAd) > cap->EFUSE_BUFFER_CONTENT_SIZE) ?
				 get_dev_eeprom_size(pAd) : cap->EFUSE_BUFFER_CONTENT_SIZE);

	switch (e2p_type) {
	case E2P_EEPROM_MODE:
		break;

	case E2P_BIN_MODE: {
		pChipOps->eeinit = rtmp_ee_bin_init;
		pChipOps->eeread = rtmp_ee_bin_read16;
		pChipOps->eewrite = rtmp_ee_bin_write16;
		pChipOps->eeread_range = rtmp_ee_bin_read_with_range;
		pChipOps->eewrite_range = rtmp_ee_bin_write_with_range;
		pChipOps->ee_gen_cmd = rtmp_ee_fr_host;
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "NVM is BIN mode\n");
		return 0;
	}

#ifdef RTMP_FLASH_SUPPORT

	case E2P_FLASH_MODE: {
		pChipOps->eeinit = rtmp_nv_init;
		pChipOps->eeread = rtmp_ee_flash_read;
		pChipOps->eewrite = rtmp_ee_flash_write;
		pChipOps->eeread_range = rtmp_ee_flash_read_with_range;
		pChipOps->eewrite_range = rtmp_ee_flash_write_with_range;
		pChipOps->ee_gen_cmd = rtmp_ee_fr_host;

		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 "NVM is FLASH mode. dev_idx [%d] FLASH OFFSET [0x%X]\n", pAd->dev_idx, get_dev_eeprom_offset(pAd));
		return 0;
	}

#endif /* RTMP_FLASH_SUPPORT */
#ifdef RTMP_EFUSE_SUPPORT
	case E2P_EFUSE_MODE:
		if (pAd->bUseEfuse) {
			pChipOps->eeinit = eFuse_init;
			pChipOps->eeread = rtmp_ee_efuse_read16;
			pChipOps->eewrite = rtmp_ee_efuse_write16;
			pChipOps->ee_gen_cmd = rtmp_ee_fr_mcu;
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "NVM is EFUSE mode\n");
			return 0;
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "hook efuse mode failed\n");
			break;
		}

#endif /* RTMP_EFUSE_SUPPORT */

	default:
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Do not support E2P type(%d), change to BIN mode\n",
				 e2p_type);
		pChipOps->eeinit = rtmp_ee_bin_init;
		pChipOps->eeread = rtmp_ee_bin_read16;
		pChipOps->eewrite = rtmp_ee_bin_write16;
		pChipOps->ee_gen_cmd = rtmp_ee_fr_host;
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "NVM is BIN mode\n");
		return 0;
	}

	/* Hook functions based on interface types for EEPROM */
	switch (infType) {
#ifdef RTMP_PCI_SUPPORT

	case RTMP_DEV_INF_PCI:
	case RTMP_DEV_INF_PCIE:
		RTMP_IO_READ32(pAd->hdev_ctrl, E2PROM_CSR, &val);

		if ((val & 0x30) == 0)
			pAd->EEPROMAddressNum = 6; /* 93C46 */
		else
			pAd->EEPROMAddressNum = 8; /* 93C66 or 93C86 */

		pChipOps->eeinit = NULL;
		pChipOps->eeread = rtmp_ee_prom_read16;
		pChipOps->eewrite = rtmp_ee_prom_write16;
		pChipOps->ee_gen_cmd = NULL;
		break;
#endif /* RTMP_PCI_SUPPORT */

	default:
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "hook failed\n");
		break;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "NVM is EEPROM mode\n");
	return 0;
}


BOOLEAN rtmp_get_default_bin_file_by_chip(
	IN PRTMP_ADAPTER pAd,
	IN UINT32	ChipVersion,
	OUT RTMP_STRING **pBinFileName)
{
	BOOLEAN found = FALSE;
	INT i;

	for (i = 0; RTMP_CHIP_E2P_FILE_TABLE[i].ChipVersion != 0; i++) {
		if (RTMP_CHIP_E2P_FILE_TABLE[i].ChipVersion == ChipVersion) {
			*pBinFileName = RTMP_CHIP_E2P_FILE_TABLE[i].name;
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "Found E2P bin file name:%s\n", *pBinFileName);
			found = TRUE;
			break;
		}
	}

	if (found == TRUE)
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Found E2P bin file name=%s\n", *pBinFileName);
	else
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "E2P bin file name not found\n");

	return found;
}


BOOLEAN rtmp_ee_bin_read16(RTMP_ADAPTER *pAd, UINT32 Offset, UINT16 *pValue)
{
	BOOLEAN IsEmpty = 0;
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Read from EEPROM buffer\n");
	NdisMoveMemory(pValue, &(pAd->EEPROMImage[Offset]), 2);
	*pValue = le2cpu16(*pValue);

	if ((*pValue == 0xffff) || (*pValue == 0x0000))
		IsEmpty = 1;

	return IsEmpty;
}


INT rtmp_ee_bin_write16(
	IN RTMP_ADAPTER	 *pAd,
	IN UINT32			Offset,
	IN USHORT			data)
{
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Write to EEPROM buffer\n");
	data = le2cpu16(data);
	NdisMoveMemory(&(pAd->EEPROMImage[Offset]), &data, 2);
	return 0;
}

BOOLEAN rtmp_ee_bin_read_with_range(PRTMP_ADAPTER pAd, UINT32 start, UINT32 Length, UCHAR *pbuf)
{
	BOOLEAN IsEmpty = 0;
	UINT16  u2Loop;
	UCHAR   ucValue = 0;

	memcpy(pbuf, pAd->EEPROMImage + start, Length);

	for (u2Loop = 0; u2Loop < Length; u2Loop++)
		ucValue |= pbuf[u2Loop];

	if ((ucValue == 0xff) || (ucValue == 0x00))
		IsEmpty = 1;

	return IsEmpty;
}

INT rtmp_ee_bin_write_with_range(PRTMP_ADAPTER pAd, UINT32 start, UINT32 Length, UCHAR *pbuf)
{
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Write data range to EEPROM buffer\n");
	NdisMoveMemory(&(pAd->EEPROMImage[start]), pbuf, Length);

	return 0;
}

INT rtmp_ee_load_from_bin(
	IN PRTMP_ADAPTER	pAd)
{
	CHAR src[100] = {'\0'};
	INT ret_val;
	INT n;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;
	EEPROM_CONTROL *pE2pCtrl = &pAd->E2pCtrl;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	USHORT ChipIdFromE2p;

	pE2pCtrl->e2pCurMode = E2P_BIN_MODE;

	if (ops && ops->get_bin_image_file != NULL) {
		ops->get_bin_image_file(pAd, src, TRUE);
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"bin FileName=%s\n", src);
	} else {
		strncat(src, BIN_FILE_PATH, strlen(BIN_FILE_PATH));
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "FileName=%s\n", src);
	}

	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (strlen(src)) {
		struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);

		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(srcf)) {
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "Error opening %s\n", src);
			goto error;
		} else {
			NdisZeroMemory(pAd->EEPROMImage,
					((get_dev_eeprom_size(pAd) > chip_cap->EEPROM_DEFAULT_BIN_SIZE) ?
					   get_dev_eeprom_size(pAd) : chip_cap->EEPROM_DEFAULT_BIN_SIZE));
			RtmpOSFileSeek(srcf, get_dev_eeprom_offset(pAd));
			ret_val = RtmpOSFileRead(srcf, (RTMP_STRING *)pAd->EEPROMImage,
						 ((get_dev_eeprom_size(pAd) > chip_cap->EEPROM_DEFAULT_BIN_SIZE) ?
						   get_dev_eeprom_size(pAd) : chip_cap->EEPROM_DEFAULT_BIN_SIZE));

			if (ret_val > 0) {
				MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "Load EEPROM Buffer from %s\n", src);
				ret_val = NDIS_STATUS_SUCCESS;
			} else {
				MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "Read file \"%s\" failed(errCode=%d)!\n", src, ret_val);
				goto error;
			}
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Error src or srcf is null\n");
		goto error;
	}

	ret_val = RtmpOSFileClose(srcf);

	if (ret_val)
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Error %d closing %s\n", -ret_val, src);

	pE2pCtrl->e2pSource = E2P_SRC_FROM_BIN;
	n = snprintf(pE2pCtrl->BinSource, sizeof(pE2pCtrl->BinSource), "%s", src);
	if (n < 0 || n >= sizeof(pE2pCtrl->BinSource)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "%s:%d snprintf Error\n", __func__, __LINE__);
	}
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	rtmp_ee_bin_read16(pAd, 0, &ChipIdFromE2p);

#ifdef MT7916
	if (IS_MT7916(pAd)) {
		if ((ChipIdFromE2p == 0x7916))
			return TRUE;
	} else
#endif
	if ((pAd->ChipID & 0x0000ffff) == ChipIdFromE2p)
		return TRUE;

error:
#ifdef CONFIG_CPE_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 	"\n\n\n\n\n\n");
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 	"********************************************************\n");
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 	"********************************************************\n");
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 	"*                                                      *\n");
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 	"*      Notice! Notice! Notice Notice! Notice!          *\n");
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 	"*                                                      *\n");
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 	"*        Did Not Find Correct Bin File in Path:        *\n");
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 	"*                 %-37s*\n", src);
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 	"*       Bin File Mode May Not Work Correctly!!!        *\n");
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 	"*                                                      *\n");
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 	"********************************************************\n");
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"********************************************************\n");
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 	"\n\n\n\n\n");
#endif

	if (cap->EEPROM_DEFAULT_BIN != NULL) {
		NdisMoveMemory(pAd->EEPROMImage, cap->EEPROM_DEFAULT_BIN,
					   ((cap->EEPROM_DEFAULT_BIN_SIZE > MAX_EEPROM_BUFFER_SIZE) ? cap->EEPROM_DEFAULT_BIN_SIZE :
					   MAX_EEPROM_BUFFER_SIZE));
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Load EEPROM Buffer from default BIN.\n");
		pE2pCtrl->e2pSource = E2P_SRC_FROM_BIN;
		n = snprintf(pE2pCtrl->BinSource, sizeof(pE2pCtrl->BinSource), "%s", "Default BIN");
		if (n < 0 || n >= sizeof(pE2pCtrl->BinSource)) {
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "%s:%d snprintf Error\n", __func__, __LINE__);
		}
	}
	RtmpOSFSInfoChange(&osFSInfo, FALSE);

	return FALSE;
}

#if defined(PRE_CAL_TRX_SET1_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) || defined(RLM_CAL_CACHE_SUPPORT)
NDIS_STATUS rtmp_cal_load_from_bin(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR *buf,
	IN ULONG offset,
	IN ULONG len)
{
	RTMP_STRING src[100] = {"\0"};
	INT ret_val;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;
	RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (chip_ops->get_prek_image_file)
		chip_ops->get_prek_image_file(pAd, src);
	else {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown PreK image file\n");
		return NDIS_STATUS_FAILURE;
	}

	/* MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "FileName=%s\n", src); */
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "FileName=%s\n", src);		/* Anjan - debug */
	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (strlen(src)) {
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);

		if (IS_FILE_OPEN_ERR(srcf)) {
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Error opening %s\n", src);
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "\e[0;31m Run RXDCOC/TXDPD/PRECAL cmds to generate caldata files\e[m\n");
			RtmpOSFSInfoChange(&osFSInfo, FALSE);
			return NDIS_STATUS_FAILURE;
		} else {
			RtmpOSFileSeek(srcf, offset);
			ret_val = RtmpOSFileRead(srcf, (RTMP_STRING *)buf, len);

			if (ret_val > 0) {
				MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "Load from %s (Read = %d)\n", src, ret_val);
				ret_val = NDIS_STATUS_SUCCESS;
			} else {
				MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "Read file \"%s\" failed(errCode=%d)!\n", src, ret_val);
			}
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Error src or srcf is null\n");
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "FAIL!!! Load Cal Data from file.\n");
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
		return NDIS_STATUS_FAILURE;
	}

	ret_val = RtmpOSFileClose(srcf);

	if (ret_val)
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Error %d closing %s\n", -ret_val, src);

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS rtmp_cal_write_to_bin(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR *buf,
	IN ULONG offset,
	IN ULONG len)
{
	CHAR *src = NULL;
	INT ret_val;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;
	RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (chip_ops->get_prek_image_file)
		chip_ops->get_prek_image_file(pAd, src);
	else {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown PreK image file\n");
		return NDIS_STATUS_FAILURE;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "FileName=%s\n", src);
	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (strlen(src)) {
		srcf = RtmpOSFileOpen(src, O_WRONLY | O_CREAT, 0);

		if (IS_FILE_OPEN_ERR(srcf)) {
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error opening %s\n", src);
			RtmpOSFSInfoChange(&osFSInfo, FALSE);
			return NDIS_STATUS_FAILURE;
		} else {
			RtmpOSFileSeek(srcf, offset);
			RtmpOSFileWrite(srcf, (RTMP_STRING *)buf, len);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error src or srcf is null\n");
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
		return NDIS_STATUS_FAILURE;
	}

	ret_val = RtmpOSFileClose(srcf);

	if (ret_val)
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error %d closing %s\n", -ret_val, src);

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	return NDIS_STATUS_SUCCESS;
}
#endif	/* PRE_CAL_TRX_SET1_SUPPORT */

#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
INT Cal_Data_Write_To_Bin(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 *Buf,
	IN UINT32 Offset,
	IN UINT32 Len)
{
	INT32 retval, Status = NDIS_STATUS_FAILURE;
	RTMP_STRING pSrc[100] = {"\0"};
	RTMP_OS_FD pSrcf;
	RTMP_OS_FS_INFO osFSInfo;
	RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (IS_MT7615(pAd)) {
		if (chip_ops->get_prek_image_file)
			chip_ops->get_prek_image_file(pAd, pSrc);
		else {
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown PreK image file\n");
			return NDIS_STATUS_FAILURE;
		}

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"\x1b[32m FileName = %s\x1b[m\n", pSrc);

		/* Change limits of authority in order to read/write file */
		RtmpOSFSInfoChange(&osFSInfo, TRUE);

		/* Create file descriptor */
		pSrcf = RtmpOSFileOpen(pSrc, O_WRONLY|O_CREAT, 0);
		if (IS_FILE_OPEN_ERR(pSrcf)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"\x1b[41m Error opening %s\x1b[m\n", pSrc);
			goto error;
		}

		RtmpOSFileSeek(pSrcf, Offset);
		retval = RtmpOSFileWrite(pSrcf, (RTMP_STRING *)Buf, Len);
		if (retval < 0) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"\x1b[41m Fail to write data to %s !!\x1b[m\n", pSrc);
			goto error;
		}

		/* Close file descriptor */
		if (!IS_FILE_OPEN_ERR(pSrcf)) {
			retval = RtmpOSFileClose(pSrcf);
			if (retval) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"\x1b[41m Error %d closing %s\x1b[m\n", -retval, pSrc);
				goto error;
			}
		}

		/* Change limits of authority in order to read/write file */
		RtmpOSFSInfoChange(&osFSInfo, FALSE);

		/* Update status */
		Status = NDIS_STATUS_SUCCESS;

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"\x1b[42m Store data to %s done !!\x1b[m \n", pSrc);

		return Status;

	error:
		/* Close file descriptor */
		if (!IS_FILE_OPEN_ERR(pSrcf)) {
			retval = RtmpOSFileClose(pSrcf);
			if (retval) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"\x1b[41m Error %d closing %s\x1b[m\n", -retval, pSrc);
			}
		}

		/* Change limits of authority in order to read/write file */
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
	}
	return Status;

}

INT Cal_Data_Load_From_Bin(
		IN PRTMP_ADAPTER pAd,
		IN UINT8 *Buf,
		IN UINT32 Offset,
		IN UINT32 Len)
{
	INT32 retval, Status = NDIS_STATUS_FAILURE;
	RTMP_STRING *pSrc = NULL;
	RTMP_OS_FD pSrcf;
	RTMP_OS_FS_INFO osFSInfo;
	RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (IS_MT7615(pAd)) {
		if (chip_ops->get_prek_image_file)
			chip_ops->get_prek_image_file(pAd, pSrc);
		else {
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown PreK image file\n");
			return NDIS_STATUS_FAILURE;
		}

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"\x1b[32m FileName = %s\x1b[m\n", pSrc);

		/* Change limits of authority in order to read/write file */
		RtmpOSFSInfoChange(&osFSInfo, TRUE);

		/* Create file descriptor */
		pSrcf = RtmpOSFileOpen(pSrc, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(pSrcf)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"\x1b[41m Error opening %s\x1b[m\n", pSrc);
			goto error;
		}

		RtmpOSFileSeek(pSrcf, Offset);
		retval = RtmpOSFileRead(pSrcf, (RTMP_STRING *)Buf, Len);
		if (retval < 0) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"\x1b[41m Fail to load data from %s !!\x1b[m\n", pSrc);
			goto error;
		}

		/* Close file descriptor */
		if (!IS_FILE_OPEN_ERR(pSrcf)) {
			retval = RtmpOSFileClose(pSrcf);
			if (retval) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"\x1b[41m Error %d closing %s\x1b[m\n", -retval, pSrc);
				goto error;
			}
		}

		/* Change limits of authority in order to read/write file */
		RtmpOSFSInfoChange(&osFSInfo, FALSE);

		/* Update status */
		Status = NDIS_STATUS_SUCCESS;

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"\x1b[42m Load data from %s success!!\x1b[m\n", pSrc);

		return Status;

	error:
		/* Close file descriptor */
		if (!IS_FILE_OPEN_ERR(pSrcf)) {
			retval = RtmpOSFileClose(pSrcf);
			if (retval) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"\x1b[41m Error %d closing %s\x1b[m\n", -retval, pSrc);
			}
		}

		/* Change limits of authority in order to read/write file */
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
	}
	return Status;
}
#endif/* CAL_BIN_FILE_SUPPORT */

INT rtmp_ee_write_to_bin(
	IN PRTMP_ADAPTER	pAd)
{
	CHAR src[100] = {'\0'};
	INT ret_val;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;
#ifdef RT_SOC_SUPPORT
#ifdef MULTIPLE_CARD_SUPPORT
	RTMP_STRING bin_file_path[128];
	RTMP_STRING *bin_file_name = NULL;
	UINT32 chip_ver = (pAd->MACVersion >> 16);

	if (rtmp_get_default_bin_file_by_chip(pAd, chip_ver, &bin_file_name) == TRUE) {
		if (pAd->MC_RowID > 0)
			sprintf(bin_file_path, "%s%s", EEPROM_2ND_FILE_DIR, bin_file_name);
		else
			sprintf(bin_file_path, "%s%s", EEPROM_1ST_FILE_DIR, bin_file_name);

		strcpy(src, bin_file_path);
	} else
#endif /* MULTIPLE_CARD_SUPPORT */
#endif /* RT_SOC_SUPPORT */
	{
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops && ops->get_bin_image_file != NULL) {
			ops->get_bin_image_file(pAd, src, TRUE);
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"bin FileName=%s\n", src);
		} else {
			strncat(src, BIN_FILE_PATH, strlen(BIN_FILE_PATH));
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"src FileName=%s\n", src);
		}
	}

	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (strlen(src)) {
		struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);

		srcf = RtmpOSFileOpen(src, O_WRONLY | O_CREAT, 0);
		if (IS_FILE_OPEN_ERR(srcf)) {
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error opening %s\n", src);
			RtmpOSFSInfoChange(&osFSInfo, FALSE);
			return FALSE;
		} else {
			RtmpOSFileSeek(srcf, get_dev_eeprom_offset(pAd));
			RtmpOSFileWrite(srcf, (RTMP_STRING *)pAd->EEPROMImage,
					(get_dev_eeprom_size(pAd) > chip_cap->EEPROM_DEFAULT_BIN_SIZE) ?
						get_dev_eeprom_size(pAd) : chip_cap->EEPROM_DEFAULT_BIN_SIZE);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error src or srcf is null\n");
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
		return FALSE;
	}

	ret_val = RtmpOSFileClose(srcf);

	if (ret_val)
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error %d closing %s\n", -ret_val, src);

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	return TRUE;
}


INT Set_LoadEepromBufferFromBin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG bEnable = os_str_tol(arg, 0, 10);
	INT result;

	if (bEnable < 0)
		return FALSE;
	else {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Load EEPROM buffer from BIN, and change to BIN buffer mode\n");
		result = rtmp_ee_load_from_bin(pAd);

		if (result == FALSE) {
			struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

			if (cap->EEPROM_DEFAULT_BIN != NULL) {
				NdisMoveMemory(pAd->EEPROMImage, cap->EEPROM_DEFAULT_BIN,
							   cap->EEPROM_DEFAULT_BIN_SIZE > MAX_EEPROM_BUFFER_SIZE ? MAX_EEPROM_BUFFER_SIZE :
							   cap->EEPROM_DEFAULT_BIN_SIZE);
				MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Load EEPROM Buffer from default BIN.\n");
			}
		}

		/* Change to BIN eeprom buffer mode */
		RtmpChipOpsEepromHook(pAd, pAd->infType, E2P_BIN_MODE);
		return TRUE;
	}
}


INT Set_EepromBufferWriteBack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT e2p_mode = os_str_tol(arg, 0, 10);

	if (e2p_mode >= NUM_OF_E2P_MODE)
		return FALSE;

	switch (e2p_mode) {
#ifdef RTMP_EFUSE_SUPPORT

	case E2P_EFUSE_MODE:
		MTWF_PRINT("Write EEPROM buffer back to eFuse\n");
		rtmp_ee_write_to_efuse(pAd);
		break;
#endif /* RTMP_EFUSE_SUPPORT */
#ifdef RTMP_FLASH_SUPPORT

	case E2P_FLASH_MODE:
		MTWF_PRINT("Write EEPROM buffer back to Flash\n");
		rtmp_ee_flash_write_all(pAd);
		break;
#endif /* RTMP_FLASH_SUPPORT */

	case E2P_EEPROM_MODE:
		MTWF_PRINT("Write EEPROM buffer back to EEPROM\n");
		rtmp_ee_write_to_prom(pAd);
		break;

	case E2P_BIN_MODE:
		MTWF_PRINT("Write EEPROM buffer back to BIN\n");
		rtmp_ee_write_to_bin(pAd);
		break;

	default:
		MTWF_PRINT("Do not support this EEPROM access mode\n");
		return FALSE;
	}

	return TRUE;
}

INT Set_bufferMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT EepromType = os_str_tol(arg, 0, 10);
#ifdef CONFIG_6G_SUPPORT
#ifdef DBDC_MODE
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	PUSHORT EeprAntCfg = NULL;
			/* Change eeprom band selection value by wirelessmode for FW */
			if (ops->eep_set_band_sel)
				ops->eep_set_band_sel(pAd, &EeprAntCfg);
#endif /* DBDC_MODE */
#endif /* CONFIG_6G_SUPPORT */
	MtCmdEfusBufferModeSet(pAd, EepromType);

#ifdef CONFIG_6G_SUPPORT
#ifdef DBDC_MODE
			/* Recover eeprom band selection value after fw updated*/
			if (ops->eep_set_band_sel && EeprAntCfg != NULL)
				ops->eep_set_band_sel(pAd, &EeprAntCfg);
#endif /* DBDC_MODE */
#endif /* #ifdef CONFIG_6G_SUPPORT */
	return TRUE;
}


static PCHAR e2p_mode[] = {"NONE", "Efuse", "Flash", "Eeprom", "Bin"};
static PCHAR e2p_src[] = {[E2P_SRC_FROM_EFUSE] = "Efuse",
						  [E2P_SRC_FROM_FLASH] = "Flash",
						  [E2P_SRC_FROM_EEPROM] = "Eeprom",
						  [E2P_SRC_FROM_BIN] = "Bin",
						  [E2P_SRC_FROM_FLASH_AND_EFUSE] = "Flash + ical data  from efuse(merge mode)",
						  [E2P_SRC_FROM_BIN_AND_EFUSE] = "Bin + ical data from efuse(merge mode)"
						 };
INT show_e2pinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	EEPROM_CONTROL *pE2pCtrl = &pAd->E2pCtrl;
#ifdef CAL_FREE_IC_SUPPORT
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
#endif /* CAL_FREE_IC_SUPPORT */

	MTWF_PRINT("Default eeprom mode from profile: %s\n", e2p_mode[pAd->E2pAccessMode]);
	MTWF_PRINT("Current mode: %s\n", e2p_mode[pE2pCtrl->e2pCurMode]);
	MTWF_PRINT("E2p source: %s\n", e2p_src[pE2pCtrl->e2pSource]);
#ifdef CAL_FREE_IC_SUPPORT

	if (!(pE2pCtrl->e2pSource & E2P_SRC_FROM_EFUSE)
		&& ops->check_is_cal_free_merge) {
		if (ops->check_is_cal_free_merge(pAd))
			MTWF_PRINT("ical data merge: YES\n");
		else
			MTWF_PRINT("ical data merge: No\n");
	}

#endif /* CAL_FREE_IC_SUPPORT */

	if ((pE2pCtrl->e2pSource & E2P_SRC_FROM_BIN) && pE2pCtrl->BinSource)
		MTWF_PRINT("Bin file Source: %s\n", pE2pCtrl->BinSource);

	return TRUE;
}


#ifdef CAL_FREE_IC_SUPPORT
INT Set_LoadCalFreeData_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT value = os_str_tol(arg, 0, 10);
	EEPROM_CONTROL *pE2pCtrl = &pAd->E2pCtrl;

	if (value == 1) {
		BOOLEAN bCalFree = 0;
		RTMP_CAL_FREE_IC_CHECK(pAd, bCalFree);

		if (bCalFree) {
			RTMP_CAL_FREE_DATA_GET(pAd);
			MTWF_PRINT("Merge successfully");

			if (pE2pCtrl->e2pCurMode == E2P_FLASH_MODE)
				MTWF_PRINT(",plz write back to flash");

			MTWF_PRINT("\n");
		} else
			MTWF_PRINT("Merge fail\n");
	} else if (value == 2) {
		RTMP_CAL_FREE_DATA_GET(pAd);
		MTWF_PRINT("Merge successfully");
	} else
		return FALSE;

	return TRUE;
}

INT Set_CheckCalFree_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->check_is_cal_free_merge) {
		if (ops->check_is_cal_free_merge(pAd))
			MTWF_PRINT("CalFree data has been merged!!\n");
		else
			MTWF_PRINT("CalFree data has not been merged!!\n");
	} else {
		MTWF_PRINT("Not Support CalFree Merge Check!\n");
		return FALSE;
	}

	return TRUE;
}

#endif


