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

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"

INT tx_temp_dbg;
#define MDSM_NORMAL_TX_POWER							0x00
#define MDSM_DROP_TX_POWER_BY_6dBm						0x01
#define MDSM_DROP_TX_POWER_BY_12dBm					0x02
#define MDSM_ADD_TX_POWER_BY_6dBm						0x03
#define MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK		0x03


/*
	==========================================================================
	Description:
		Gives CCK TX rate 2 more dB TX power.
		This routine works only in LINK UP in INFRASTRUCTURE mode.

		calculate desired Tx power in RF R3.Tx0~5,	should consider -
		0. if current radio is a noisy environment (pAd->DrsCounters.fNoisyEnvironment)
		1. TxPowerPercentage
		2. auto calibration based on TSSI feedback
		3. extra 2 db for CCK
		4. -10 db upon very-short distance (AvgRSSI >= -40db) to AP

	NOTE: Since this routine requires the value of (pAd->DrsCounters.fNoisyEnvironment),
		it should be called AFTER MlmeDynamicTxRatSwitching()
	==========================================================================
 */
VOID AsicAdjustTxPower(RTMP_ADAPTER *pAd)
{
	INT i, j;
	CHAR Value;
	CHAR Rssi = -127;
	CHAR DeltaPwr = 0;
	CHAR TxAgcCompensate = 0;
	CHAR DeltaPowerByBbpR1 = 0;
	CHAR TotalDeltaPower = 0; /* (non-positive number) including the transmit power controlled by the MAC and the BBP R1 */
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC = {0};
#ifdef SINGLE_SKU
	CHAR TotalDeltaPowerOri = 0;
	UCHAR SingleSKUBbpR1Offset = 0;
	ULONG SingleSKUTotalDeltaPwr[MAX_TXPOWER_ARRAY_SIZE] = {0};
#endif /* SINGLE_SKU */
	UCHAR Channel = HcGetRadioChannel(pAd);
#ifdef CONFIG_STA_SUPPORT

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
		return;

	/* TODO: shiang-7603 */
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " Not finish for HIF_MT yet!\n");
		return;
	}

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
	for (i = 0; i < pAd->MSTANum; i++)
		if ((pAd->StaCfg[i].wdev.DevInfo.WdevActive && pAd->StaCfg[i].PwrMgmt.bDoze) &&
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
			return;
#endif /* CONFIG_STA_SUPPORT || APCLI_SUPPORT */

#ifdef RTMP_MAC_PCI
{
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	if	((pci_hif->bPCIclkOff == TRUE) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
		return;
}
#endif /* RTMP_MAC_PCI */

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (INFRA_ON(&pAd->StaCfg[0])) {
			Rssi = RTMPMaxRssi(pAd,
							   pAd->StaCfg[0].RssiSample.AvgRssi[0],
							   pAd->StaCfg[0].RssiSample.AvgRssi[1],
							   pAd->StaCfg[0].RssiSample.AvgRssi[2]);
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	/* Get Tx rate offset table which from EEPROM 0xDEh ~ 0xEFh */
	RTMP_CHIP_ASIC_TX_POWER_OFFSET_GET(pAd, (PULONG)&CfgOfTxPwrCtrlOverMAC);
	/* Get temperature compensation delta power value */
	RTMP_CHIP_ASIC_AUTO_AGC_OFFSET_GET(
		pAd, &DeltaPwr, &TotalDeltaPower, &TxAgcCompensate, &DeltaPowerByBbpR1, Channel);
	MTWF_DBG(NULL, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s(): DeltaPwr=%d, TotalDeltaPower=%d, TxAgcCompensate=%d, DeltaPowerByBbpR1=%d\n",
			  __func__,
			  DeltaPwr,
			  TotalDeltaPower,
			  TxAgcCompensate,
			  DeltaPowerByBbpR1);
	/* Get delta power based on the percentage specified from UI */
	AsicPercentageDeltaPower(pAd, Rssi, &DeltaPwr,  &DeltaPowerByBbpR1);
	/* The transmit power controlled by the BBP */
	TotalDeltaPower += DeltaPowerByBbpR1;
	/* The transmit power controlled by the MAC */
	TotalDeltaPower += DeltaPwr;
#ifdef SINGLE_SKU

	if (pAd->CommonCfg.bSKUMode == TRUE) {
		/* Re calculate delta power while enabling Single SKU */
		GetSingleSkuDeltaPower(pAd, &TotalDeltaPower, (PULONG)&SingleSKUTotalDeltaPwr, &SingleSKUBbpR1Offset);
		TotalDeltaPowerOri = TotalDeltaPower;
	} else
#endif /* SINGLE_SKU */
	{
		AsicCompensatePowerViaBBP(pAd, &TotalDeltaPower);
	}

	/* Power will be updated each 4 sec. */
	if (pAd->Mlme.OneSecPeriodicRound % 4 == 0) {
		/* Set new Tx power for different Tx rates */
		for (i = 0; i < CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++) {
			TX_POWER_CONTROL_OVER_MAC_ENTRY *pTxPwrEntry;
			ULONG reg_val;
			pTxPwrEntry = &CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i];
			reg_val = pTxPwrEntry->RegisterValue;

			if (reg_val != 0xffffffff) {
				for (j = 0; j < 8; j++) {
					CHAR _upbound, _lowbound, t_pwr;
					BOOLEAN _bValid;
					_lowbound = 0;
					_bValid = TRUE;
					Value = (CHAR)((reg_val >> j * 4) & 0x0F);
#ifdef SINGLE_SKU

					if (pAd->CommonCfg.bSKUMode == TRUE) {
						TotalDeltaPower = SingleSKUBbpR1Offset + TotalDeltaPowerOri - (CHAR)((SingleSKUTotalDeltaPwr[i] >> j * 4) & 0x0F);
						MTWF_DBG(NULL, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
								 "%s: BbpR1Offset(%d) + TX ALC(%d) - SingleSKU[%d/%d](%d) = TotalDeltaPower(%d)\n",
								  __func__, SingleSKUBbpR1Offset,
								  TotalDeltaPowerOri, i, j,
								  (CHAR)((SingleSKUTotalDeltaPwr[i] >> j * 4) & 0x0F),
								  TotalDeltaPower);
					}

#endif /* SINGLE_SKU */
					_upbound = 0xc;

					if (_bValid) {
						t_pwr = Value + TotalDeltaPower;

						if (t_pwr < _lowbound)
							Value = _lowbound;
						else if (t_pwr > _upbound)
							Value = _upbound;
						else
							Value = t_pwr;
					}

					/* Fill new value into the corresponding MAC offset */
#ifdef E3_DBG_FALLBACK
					pTxPwrEntry->RegisterValue = (reg_val & ~(0x0000000F << j * 4)) | (Value << j * 4);
#else
					reg_val = (reg_val & ~(0x0000000F << j * 4)) | (Value << j * 4);
#endif /* E3_DBG_FALLBACK */
				}

#ifndef E3_DBG_FALLBACK
				pTxPwrEntry->RegisterValue = reg_val;
#endif /* E3_DBG_FALLBACK */
				RTMP_IO_WRITE32(pAd->hdev_ctrl, pTxPwrEntry->MACRegisterOffset, pTxPwrEntry->RegisterValue);
			}
		}

		/* Extra set MAC registers to compensate Tx power if any */
		RTMP_CHIP_ASIC_EXTRA_POWER_OVER_MAC(pAd);
	}

}


#ifdef SINGLE_SKU
VOID GetSingleSkuDeltaPower(
	IN RTMP_ADAPTER *pAd,
	IN CHAR *pTotalDeltaPower,
	INOUT ULONG *pSingleSKUTotalDeltaPwr,
	INOUT UCHAR *pSingleSKUBbpR1Offset)
{
	INT		i, j;
	CHAR	Value;
	CHAR	MinValue = 127;
	UCHAR	BbpR1 = 0;
	UCHAR	TxPwrInEEPROM = 0xFF, CountryTxPwr = 0xFF, criterion;
	UCHAR	AdjustMaxTxPwr[(MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS * 8)];
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC = {0};
	UCHAR Channel = HcGetRadioChannel(pAd);
	USHORT PhyMode = HcGetRadioPhyMode(pAd);
	struct wifi_dev *wdev = get_default_wdev(pAd);
	UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
	/* Get TX rate offset table which from EEPROM 0xDEh ~ 0xEFh */
	RTMP_CHIP_ASIC_TX_POWER_OFFSET_GET(pAd, (PULONG)&CfgOfTxPwrCtrlOverMAC);

	/* Handle regulatory max. TX power constraint */
	if (Channel > 14) {
		TxPwrInEEPROM = ((pAd->CommonCfg.DefineMaxTxPwr & 0xFF00) >> 8); /* 5G band */
	} else {
		TxPwrInEEPROM = (pAd->CommonCfg.DefineMaxTxPwr & 0x00FF); /* 2.4G band */
	}

	CountryTxPwr = GetCuntryMaxTxPwr(pAd, PhyMode, wdev, op_ht_bw);
	/* Use OFDM 6M as the criterion */
	criterion = (UCHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue & 0x000F0000) >> 16);
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: criterion=%d, TxPwrInEEPROM=%d, CountryTxPwr=%d\n",
			 __func__, criterion, TxPwrInEEPROM, CountryTxPwr);

	/* Adjust max. TX power according to the relationship of TX power in EEPROM */
	for (i = 0; i < CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++) {
		if (i == 0) {
			for (j = 0; j < 8; j++) {
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j * 4) & 0x0F);

				if (j < 4) {
					AdjustMaxTxPwr[i * 8 + j] = TxPwrInEEPROM + (Value - criterion) + 4; /* CCK has 4dBm larger than OFDM */
				} else
					AdjustMaxTxPwr[i * 8 + j] = TxPwrInEEPROM + (Value - criterion);

				MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: offset = 0x%04X, i/j=%d/%d, (Default)Value=%d, %d\n",
						 __func__,
						 CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset,
						 i,
						 j,
						 Value,
						 AdjustMaxTxPwr[i * 8 + j]);
			}
		} else {
			for (j = 0; j < 8; j++) {
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j * 4) & 0x0F);
				AdjustMaxTxPwr[i * 8 + j] = TxPwrInEEPROM + (Value - criterion);
				MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: offset = 0x%04X, i/j=%d/%d, (Default)Value=%d, %d\n",
						 __func__,
						 CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset,
						 i,
						 j,
						 Value,
						 AdjustMaxTxPwr[i * 8 + j]);
			}
		}
	}

	/* Adjust TX power according to the relationship */
	for (i = 0; i < CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++) {
		if (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue != 0xffffffff) {
			for (j = 0; j < 8; j++) {
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j * 4) & 0x0F);

				/* The TX power is larger than the regulatory, the power should be restrained */
				if (AdjustMaxTxPwr[i * 8 + j] > CountryTxPwr) {
					Value = (AdjustMaxTxPwr[i * 8 + j] - CountryTxPwr);

					if (Value > 0xF) {
						/* The output power is larger than Country Regulatory over 15dBm, the origianl design has overflow case */
						MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Value overflow - %d\n", Value);
					}

					*(pSingleSKUTotalDeltaPwr + i) = (*(pSingleSKUTotalDeltaPwr + i) & ~(0x0000000F << j * 4)) | (Value << j * 4);
					MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: offset = 0x%04X, i/j=%d/%d, (Exceed)Value=%d, %d\n",
							 __func__,
							 CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset,
							 i,
							 j,
							 Value,
							 AdjustMaxTxPwr[i * 8 + j]);
				} else {
					MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: offset = 0x%04X, i/j=%d/%d, Value=%d, %d, no change\n",
							 __func__,
							 CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset,
							 i,
							 j,
							 Value,
							 AdjustMaxTxPwr[i * 8 + j]);
				}
			}
		}
	}

	/* Calculate the min. TX power */
	for (i = 0; i < CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++) {
		if (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue != 0xffffffff) {
			for (j = 0; j < 8; j++) {
				CHAR PwrChange;
				/*
				   After Single SKU, each data rate offset power value is saved in TotalDeltaPwr[].
				   PwrChange will add SingleSKUDeltaPwr and TotalDeltaPwr[] for each data rate to calculate
				   the final adjust output power value which is saved in MAC Reg. and BBP_R1.
				*/
				/*
				   Value / TxPwr[] is get from eeprom 0xDEh ~ 0xEFh and increase or decrease the
				   20/40 Bandwidth Delta Value in eeprom 0x50h.
				*/
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j * 4) & 0x0F); /* 0 ~ 15 */

				/* Fix the corner case of Single SKU read eeprom offset 0xF0h ~ 0xFEh which for BBP Instruction configuration */
				if (Value == 0xF)
					continue;

				/* Value_offset is current Pwr comapre with Country Regulation and need adjust delta value */
				PwrChange = (CHAR)((*(pSingleSKUTotalDeltaPwr + i) >> j * 4) & 0x0F); /* 0 ~ 15 */
				PwrChange -= *pTotalDeltaPower;
				Value -= PwrChange;

				if (MinValue > Value)
					MinValue = Value;
			}
		}
	}


	/* Depend on the min. TX power to adjust and prevent the value of MAC_TX_PWR_CFG less than 0 */
	if ((MinValue < 0) && (MinValue >= -6)) {
		BbpR1 |= MDSM_DROP_TX_POWER_BY_6dBm;
		*pSingleSKUBbpR1Offset = 6;
	} else if ((MinValue < -6) && (MinValue >= -12)) {
		BbpR1 |= MDSM_DROP_TX_POWER_BY_12dBm;
		*pSingleSKUBbpR1Offset = 12;
	} else if (MinValue < -12) {
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: ASIC limit..\n", __func__));
		BbpR1 |= MDSM_DROP_TX_POWER_BY_12dBm;
		*pSingleSKUBbpR1Offset = 12;
	}

#ifndef E3_DBG_FALLBACK
	else {
		BbpR1 &= ~MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK;
		*pSingleSKUBbpR1Offset = 0;
	}

#endif /* E3_DBG_FALLBACK */
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: <After BBP R1> TotalDeltaPower = %d dBm, BbpR1 = 0x%02X\n",
			 __func__, *pTotalDeltaPower, BbpR1);
}
#endif /* SINGLE_SKU */


VOID AsicPercentageDeltaPower(
	IN		PRTMP_ADAPTER		pAd,
	IN		CHAR				Rssi,
	INOUT	PCHAR				pDeltaPwr,
	INOUT	PCHAR				pDeltaPowerByBbpR1)
{
	/*
		Calculate delta power based on the percentage specified from UI.
		E2PROM setting is calibrated for maximum TX power (i.e. 100%).
		We lower TX power here according to the percentage specified from UI.
	*/
	if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] >= 100) { /* AUTO TX POWER control */
#ifdef CONFIG_STA_SUPPORT
		if ((pAd->OpMode == OPMODE_STA)
#ifdef P2P_SUPPORT
			&& (!P2P_GO_ON(pAd))
#endif /* P2P_SUPPORT */
		   ) {
			/* To patch high power issue with some APs, like Belkin N1.*/
			if (Rssi > -35)
				*pDeltaPwr -= 12;
			else if (Rssi > -40)
				*pDeltaPwr -= 6;
			else
				;
		}

#endif /* CONFIG_STA_SUPPORT */
	} else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 90) /* 91 ~ 100% & AUTO, treat as 100% in terms of mW */
		*pDeltaPwr -= 0;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 60) /* 61 ~ 90%, treat as 75% in terms of mW		 DeltaPwr -= 1; */
		*pDeltaPwr -= 1;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 30) /* 31 ~ 60%, treat as 50% in terms of mW		 DeltaPwr -= 3; */
		*pDeltaPwr -= 3;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 15) { /* 16 ~ 30%, treat as 25% in terms of mW		 DeltaPwr -= 6; */
		*pDeltaPowerByBbpR1 -= 6; /* -6 dBm */
	} else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] >
			   9) { /* 10 ~ 15%, treat as 12.5% in terms of mW		 DeltaPwr -= 9; */
		*pDeltaPowerByBbpR1 -= 6; /* -6 dBm */
		*pDeltaPwr -= 3;
	} else { /* 0 ~ 9 %, treat as MIN(~3%) in terms of mW		 DeltaPwr -= 12; */
		*pDeltaPowerByBbpR1 -= 12; /* -12 dBm */
	}
}

VOID AsicCompensatePowerViaBBP(RTMP_ADAPTER *pAd, CHAR *pTotalDeltaPower)
{
	UCHAR mdsm_drop_pwr;
	MTWF_DBG(NULL, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: <Before> TotalDeltaPower = %d dBm\n", __func__,
			 *pTotalDeltaPower);

	if (*pTotalDeltaPower <= -12) {
		*pTotalDeltaPower += 12;
		mdsm_drop_pwr = MDSM_DROP_TX_POWER_BY_12dBm;
	} else if ((*pTotalDeltaPower <= -6) && (*pTotalDeltaPower > -12)) {
		*pTotalDeltaPower += 6;
		mdsm_drop_pwr = MDSM_DROP_TX_POWER_BY_6dBm;
	} else {
		/* Control the the transmit power by using the MAC only */
		mdsm_drop_pwr = MDSM_NORMAL_TX_POWER;
	}

	MTWF_DBG(NULL, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s: Drop the BBP transmit power by %d dBm!\n",
			 __func__,
			 (mdsm_drop_pwr == MDSM_DROP_TX_POWER_BY_12dBm ? 12 : \
			  (mdsm_drop_pwr == MDSM_DROP_TX_POWER_BY_6dBm ? 6 : 0)));
}


/*
	========================================================================

	Routine Description:
		Read initial Tx power per MCS and BW from EEPROM

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID RTMPReadTxPwrPerRate(RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "(%d): Don't Support this now!\n", __LINE__);
	return;
}


/*
	========================================================================

	Routine Description:
		Read initial channel power parameters from EEPROM

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID RTMPReadChannelPwr(RTMP_ADAPTER *pAd)
{
	UINT32					i, choffset;
	EEPROM_TX_PWR_STRUC	    Power = {0};
	EEPROM_TX_PWR_STRUC	    Power2 = {0};
#if (defined(RT30xx) && defined(RTMP_MAC_PCI)) || defined(RT3593)
	UCHAR Tx0ALC = 0, Tx1ALC = 0, Tx0FinePowerCtrl = 0, Tx1FinePowerCtrl = 0;
#endif /* (defined(RT30xx) && defined(RTMP_MAC_PCI)) || defined(RT3593) */

	/* Read Tx power value for all channels*/
	/* Value from 1 - 0x7f. Default value is 24.*/
	/* Power value : 2.4G 0x00 (0) ~ 0x1F (31)*/
	/*             : 5.5G 0xF9 (-7) ~ 0x0F (15)*/

	/* 0. 11b/g, ch1 - ch 14*/
	for (i = 0; i < 7; i++) {
		/* Default routine. RT3070 and RT3370 run here. */
		RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET + i * 2, Power.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX2_PWR_OFFSET + i * 2, Power2.word);
		pAd->TxPower[i * 2].Channel = i * 2 + 1;
		pAd->TxPower[i * 2 + 1].Channel = i * 2 + 2;
		pAd->TxPower[i * 2].Power = Power.field.Byte0;

		if ((Power.field.Byte0 > 31) || (Power.field.Byte0 < 0))
			pAd->TxPower[i * 2].Power = DEFAULT_RF_TX_POWER;

		pAd->TxPower[i * 2 + 1].Power = Power.field.Byte1;

		if ((Power.field.Byte1 > 31) || (Power.field.Byte1 < 0))
			pAd->TxPower[i * 2 + 1].Power = DEFAULT_RF_TX_POWER;

		if ((Power2.field.Byte0 > 31) || (Power2.field.Byte0 < 0))
			pAd->TxPower[i * 2].Power2 = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 > 31) || (Power2.field.Byte1 < 0))
			pAd->TxPower[i * 2 + 1].Power2 = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2 + 1].Power2 = Power2.field.Byte1;
	}

	{
		/* 1. U-NII lower/middle band: 36, 38, 40; 44, 46, 48; 52, 54, 56; 60, 62, 64 (including central frequency in BW 40MHz)*/
		/* 1.1 Fill up channel*/
		choffset = 14;

		for (i = 0; i < 4; i++) {
			pAd->TxPower[3 * i + choffset + 0].Channel	= 36 + i * 8 + 0;
			pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Channel	= 36 + i * 8 + 2;
			pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Channel	= 36 + i * 8 + 4;
			pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
		}

		/* 1.2 Fill up power*/
		for (i = 0; i < 6; i++) {
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + i * 2, Power2.word);

			if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

			if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;

			if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

			if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;
		}

		/* 2. HipperLAN 2 100, 102 ,104; 108, 110, 112; 116, 118, 120; 124, 126, 128; 132, 134, 136; 140 (including central frequency in BW 40MHz)*/
		/* 2.1 Fill up channel*/
		choffset = 14 + 12;

		for (i = 0; i < 5; i++) {
			pAd->TxPower[3 * i + choffset + 0].Channel	= 100 + i * 8 + 0;
			pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Channel	= 100 + i * 8 + 2;
			pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Channel	= 100 + i * 8 + 4;
			pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
		}

		pAd->TxPower[3 * 5 + choffset + 0].Channel		= 140;
		pAd->TxPower[3 * 5 + choffset + 0].Power		= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 5 + choffset + 0].Power2		= DEFAULT_RF_TX_POWER;

		/* 2.2 Fill up power*/
		for (i = 0; i < 8; i++) {
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word);

			if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

			if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;

			if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

			if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;
		}

		/* 3. U-NII upper band: 149, 151, 153; 157, 159, 161; 165, 167, 169; 171, 173 (including central frequency in BW 40MHz)*/
		/* 3.1 Fill up channel*/
		choffset = 14 + 12 + 16;

		/*for (i = 0; i < 2; i++)*/
		for (i = 0; i < 3; i++) {
			pAd->TxPower[3 * i + choffset + 0].Channel	= 149 + i * 8 + 0;
			pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Channel	= 149 + i * 8 + 2;
			pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Channel	= 149 + i * 8 + 4;
			pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
		}

		pAd->TxPower[3 * 3 + choffset + 0].Channel		= 171;
		pAd->TxPower[3 * 3 + choffset + 0].Power		= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 3 + choffset + 0].Power2		= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 3 + choffset + 1].Channel		= 173;
		pAd->TxPower[3 * 3 + choffset + 1].Power		= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 3 + choffset + 1].Power2		= DEFAULT_RF_TX_POWER;

		/* 3.2 Fill up power*/
		/*for (i = 0; i < 4; i++)*/
		for (i = 0; i < 6; i++) {
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word);

			if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

			if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;

			if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

			if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;
		}
	}

	/* 4. Print and Debug*/
	/*choffset = 14 + 12 + 16 + 7;*/
	choffset = 14 + 12 + 16 + 11;
}

INT tx_pwr_comp_init(RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s()-->\n", __func__);

	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "NotSupportYet!\n");
	return FALSE;

}

INT32 get_low_mid_hi_index(UINT8 channel)
{
	INT32 index = G_BAND_LOW;

	if (channel <= 14) {
		if (channel >= 1 && channel <= 5)
			index = G_BAND_LOW;
		else if (channel >= 6 && channel <= 10)
			index = G_BAND_MID;
		else if (channel >= 11 && channel <= 14)
			index = G_BAND_HI;
		else
			MTWF_DBG(NULL, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "illegal channel(%d)\n", channel);
	} else {
		if (channel >= 184 && channel <= 188)
			index = A_BAND_LOW;
		else if (channel >= 192 && channel <= 196)
			index = A_BAND_HI;
		else if (channel >= 36 && channel <= 42)
			index = A_BAND_LOW;
		else if (channel >= 44 && channel <= 48)
			index = A_BAND_HI;
		else if (channel >= 52 && channel <= 56)
			index = A_BAND_LOW;
		else if (channel >= 58 && channel <= 64)
			index = A_BAND_HI;
		else if (channel >= 98 && channel <= 104)
			index = A_BAND_LOW;
		else if (channel >= 106 && channel <= 114)
			index = A_BAND_HI;
		else if (channel >= 116 && channel <= 128)
			index = A_BAND_LOW;
		else if (channel >= 130 && channel <= 144)
			index = A_BAND_HI;
		else if (channel >= 149 && channel <= 156)
			index = A_BAND_LOW;
		else if (channel >= 157 && channel <= 165)
			index = A_BAND_HI;
		else
			MTWF_DBG(NULL, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "illegal channel(%d)\n", channel);
	}

	return index;
}

