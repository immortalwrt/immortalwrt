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
 ****************************************************************************

	Module Name:
	cmm_cfg.c

	Abstract:
	Ralink WiFi Driver configuration related subroutines

	Revision History:
	Who		  When		  What
	---------	----------	----------------------------------------------
*/

#include "rt_config.h"
#ifdef TR181_SUPPORT
#include "hdev/hdev_basic.h"
#endif /*TR181_SUPPORT*/
#ifdef MAP_R2
#include "map.h"
#endif

static BOOLEAN RT_isLegalCmdBeforeInfUp(RTMP_STRING *SetCmd);
RTMP_STRING *wdev_type2str(int type);
#ifdef MGMT_TXPWR_CTRL
BOOLEAN g_fgCommand;
#endif
INT ComputeChecksum(UINT PIN)
{
	INT digit_s;
	UINT accum = 0;
	INT ret;

	PIN *= 10;
	accum += 3 * ((PIN / 10000000) % 10);
	accum += 1 * ((PIN / 1000000) % 10);
	accum += 3 * ((PIN / 100000) % 10);
	accum += 1 * ((PIN / 10000) % 10);
	accum += 3 * ((PIN / 1000) % 10);
	accum += 1 * ((PIN / 100) % 10);
	accum += 3 * ((PIN / 10) % 10);
	digit_s = (accum % 10);
	ret = ((10 - digit_s) % 10);
	return ret;
} /* ComputeChecksum*/

UINT GenerateWpsPinCode(
	IN	PRTMP_ADAPTER	pAd,
	IN  BOOLEAN		 bFromApcli,
	IN	UCHAR			apidx)
{
	UCHAR	macAddr[MAC_ADDR_LEN];
	UINT	iPin;
	UINT	checksum;
#ifdef WIDI_SUPPORT
	BOOLEAN bGenWidiPIN = FALSE;
#endif /* WIDI_SUPPORT */
	NdisZeroMemory(macAddr, MAC_ADDR_LEN);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

		if (bFromApcli)
			NdisMoveMemory(&macAddr[0], pAd->StaCfg[apidx].wdev.if_addr, MAC_ADDR_LEN);
		else
#endif /* APCLI_SUPPORT */
			NdisMoveMemory(&macAddr[0], pAd->ApCfg.MBSSID[apidx].wdev.if_addr, MAC_ADDR_LEN);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	NdisMoveMemory(&macAddr[0], pAd->StaCfg[0].wdev.if_addr, MAC_ADDR_LEN);
#endif /* CONFIG_STA_SUPPORT */
#ifdef P2P_SUPPORT

	if (apidx >= MIN_NET_DEVICE_FOR_P2P_GO)
		NdisMoveMemory(&macAddr[0], pAd->P2PCurrentAddress, MAC_ADDR_LEN);

	if (bFromApcli) {
		APCLI_MR_APIDX_SANITY_CHECK(apidx);
		NdisMoveMemory(&macAddr[0], pAd->StaCfg[apidx].wdev.if_addr, MAC_ADDR_LEN);
	}

#endif /* P2P_SUPPORT */
	iPin = macAddr[0] * 256 * 256 + macAddr[4] * 256 + macAddr[5];
	iPin = iPin % 10000000;
#ifdef WIDI_SUPPORT
#ifdef P2P_SUPPORT

	if (pAd->P2pCfg.bWIDI &&
	    ((apidx >= MIN_NET_DEVICE_FOR_P2P_GO) || bFromApcli))
		bGenWidiPIN = TRUE;
	else
#endif /* P2P_SUPPORT */
		if (pAd->StaCfg[0].bWIDI && (pAd->OpMode == OPMODE_STA))
			bGenWidiPIN = TRUE;

	if (bGenWidiPIN)
		iPin = ((iPin / 1000) * 1000);

#endif /* WIDI_SUPPORT */
	checksum = ComputeChecksum(iPin);
	iPin = iPin * 10 + checksum;
	return iPin;
}

static char *phy_mode_str[] = {"CCK", "OFDM", "HT_MM", "HT_GF", "VHT", "HE",
				"HE5G", "HE2G", "HE_SU", "HE_EXT_SU", "HE_TRIG", "HE_MU"};
char *get_phymode_str(int Mode)
{
	if (Mode >= MODE_CCK && Mode <= MODE_HE_MU)
		return phy_mode_str[Mode];
	else
		return "N/A";
}


/*
	==========================================================================
	Description:
	Set Country Region to pAd->CommonCfg.CountryRegion.
	This command will not work, if the field of CountryRegion in eeprom is programmed.

	Return:
	TRUE if all parameters are OK, FALSE otherwise
	==========================================================================
*/
INT RT_CfgSetCountryRegion(RTMP_ADAPTER *pAd, RTMP_STRING *arg, INT band)
{
	LONG region;
	UCHAR *pCountryRegion;

	region = os_str_tol(arg, 0, 10);

	if (band == BAND_24G)
		pCountryRegion = &pAd->CommonCfg.CountryRegion;
	else
		pCountryRegion = &pAd->CommonCfg.CountryRegionForABand;

	/*
		   1. If this value is set before interface up, do not reject this value.
		   2. Country can be set only when EEPROM not programmed
	*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS) && (*pCountryRegion & EEPROM_IS_PROGRAMMED)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("CfgSetCountryRegion():CountryRegion in eeprom was programmed\n"));
		return FALSE;
	}

	if ((region >= 0) &&
	    (((band == BAND_24G) && ((region <= REGION_MAXIMUM_BG_BAND) ||
				     (region == REGION_31_BG_BAND) || (region == REGION_32_BG_BAND) || (region == REGION_33_BG_BAND))) ||
	     ((band == BAND_5G) && (region <= REGION_MAXIMUM_A_BAND)))
	   )
		*pCountryRegion = (UCHAR) region;
	else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CfgSetCountryRegion():region(%ld) out of range!\n", region));
		return FALSE;
	}

	return TRUE;
}

static USHORT CFG_WMODE_MAP[] = {
	PHY_11BG_MIXED, (WMODE_B | WMODE_G), /* 0 => B/G mixed */
	PHY_11B, (WMODE_B), /* 1 => B only */
	PHY_11A, (WMODE_A), /* 2 => A only */
	PHY_11ABG_MIXED, (WMODE_A | WMODE_B | WMODE_G), /* 3 => A/B/G mixed */
	PHY_11G, WMODE_G, /* 4 => G only */
	PHY_11ABGN_MIXED, (WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN), /* 5 => A/B/G/GN/AN mixed */
	PHY_11N_2_4G, (WMODE_GN), /* 6 => N in 2.4G band only */
	PHY_11GN_MIXED, (WMODE_G | WMODE_GN), /* 7 => G/GN, i.e., no CCK mode */
	PHY_11AN_MIXED, (WMODE_A | WMODE_AN), /* 8 => A/N in 5 band */
	PHY_11BGN_MIXED, (WMODE_B | WMODE_G | WMODE_GN), /* 9 => B/G/GN mode*/
	PHY_11AGN_MIXED, (WMODE_G | WMODE_GN | WMODE_A | WMODE_AN), /* 10 => A/AN/G/GN mode, not support B mode */
	PHY_11N_5G, (WMODE_AN), /* 11 => only N in 5G band */
#ifdef DOT11_VHT_AC
	PHY_11VHT_N_ABG_MIXED, (WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN | WMODE_AC), /* 12 => B/G/GN/A/AN/AC mixed*/
	PHY_11VHT_N_AG_MIXED, (WMODE_G | WMODE_GN | WMODE_A | WMODE_AN | WMODE_AC), /* 13 => G/GN/A/AN/AC mixed, no B mode */
	PHY_11VHT_N_A_MIXED, (WMODE_A | WMODE_AN | WMODE_AC), /* 14 => A/AC/AN mixed */
	PHY_11VHT_N_MIXED, (WMODE_AN | WMODE_AC), /* 15 => AC/AN mixed, but no A mode */
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
	PHY_11AX_24G, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G),
	PHY_11AX_5G, (WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G),
	PHY_11AX_6G, (WMODE_AX_6G),
	PHY_11AX_24G_6G, (WMODE_G | WMODE_GN | WMODE_AX_24G | WMODE_AX_6G),
	PHY_11AX_5G_6G, (WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_AX_6G),
	PHY_11AX_24G_5G_6G, (PHY_11AX_24G | PHY_11AX_5G | WMODE_AX_6G),
#endif /*DOT11_HE_AX*/
	PHY_MODE_MAX, WMODE_INVALID /* default phy mode if not match */
};

static RTMP_STRING *BAND_STR[] = {"Invalid", "2.4G", "5G", "2.4G/5G"};
static RTMP_STRING *WMODE_STR[] = {"", "A", "B", "G", "gN", "aN", "AC", "ax_24g", "ax_5g", "ax_6g"};

UCHAR *wmode_2_str(USHORT wmode)
{
	UCHAR *str;
	INT idx, pos, max_len;

	max_len = WMODE_COMP * 3;

	if (os_alloc_mem(NULL, &str, max_len) == NDIS_STATUS_SUCCESS) {
		NdisZeroMemory(str, max_len);
		pos = 0;

		for (idx = 0; idx < WMODE_COMP; idx++) {
			if (wmode & (1 << idx)) {
				if ((strlen(str) +  strlen(WMODE_STR[idx + 1])) >= (max_len - 1))
					break;

				if (strlen(str)) {
					NdisMoveMemory(&str[pos], "/", 1);
					pos++;
				}

				NdisMoveMemory(&str[pos], WMODE_STR[idx + 1], strlen(WMODE_STR[idx + 1]));
				pos += strlen(WMODE_STR[idx + 1]);
			}

			if (strlen(str) >= max_len)
				break;
		}

		return str;
	} else
		return NULL;
}

RT_802_11_PHY_MODE wmode_2_cfgmode(USHORT wmode)
{
	INT i, mode_cnt = sizeof(CFG_WMODE_MAP) / (sizeof(USHORT) * 2);

	for (i = 0; i < mode_cnt; i++) {
		if (CFG_WMODE_MAP[i * 2 + 1] == wmode)
			return CFG_WMODE_MAP[i * 2];
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Cannot get cfgmode by wmode(%x)\n",
			__func__, wmode));
	return 0;
}

USHORT cfgmode_2_wmode(UCHAR cfg_mode)
{
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cfg_mode=%d\n", cfg_mode));

	if (cfg_mode >= PHY_MODE_MAX)
		cfg_mode =  PHY_MODE_MAX;

	return CFG_WMODE_MAP[cfg_mode * 2 + 1];
}

BOOLEAN wmode_valid_and_correct(RTMP_ADAPTER *pAd, USHORT *wmode)
{
	BOOLEAN ret = TRUE;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (*wmode == WMODE_INVALID)
		*wmode = (WMODE_B | WMODE_G | WMODE_GN |
				WMODE_A | WMODE_AN | WMODE_AC |
				WMODE_AX_5G | WMODE_AX_24G);

	while (1) {
		if (WMODE_CAP_5G(*wmode) && (!PHY_CAP_5G(cap->phy_caps)))
			*wmode = *wmode & ~(WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G);
		else if (WMODE_CAP_2G(*wmode) && (!PHY_CAP_2G(cap->phy_caps)))
			*wmode = *wmode & ~(WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G);
		else if (WMODE_CAP_N(*wmode) && ((!PHY_CAP_N(cap->phy_caps)) ||
					RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N)))
			*wmode = *wmode & ~(WMODE_GN | WMODE_AN);
		else if (WMODE_CAP_AC(*wmode) && (!PHY_CAP_AC(cap->phy_caps)))
			*wmode = *wmode & ~(WMODE_AC);
		else if (WMODE_CAP_AX(*wmode) && (!PHY_CAP_AX(cap->phy_caps)))
			*wmode = *wmode & ~(WMODE_AX_24G | WMODE_AX_5G);

		if (*wmode == 0) {
			*wmode = (WMODE_B | WMODE_G | WMODE_GN |
					WMODE_A | WMODE_AN | WMODE_AC |
					WMODE_AX_24G | WMODE_AX_5G);
			break;
		} else
			break;
	}

	return ret;
}

BOOLEAN wmode_band_equal(USHORT smode, USHORT tmode)
{
	BOOLEAN eq = FALSE;
	UCHAR *str1, *str2;

	if ((WMODE_CAP_5G(smode) == WMODE_CAP_5G(tmode)) &&
	    (WMODE_CAP_2G(smode) == WMODE_CAP_2G(tmode)) &&
	    (WMODE_CAP_6G(smode) == WMODE_CAP_6G(tmode)))
		eq = TRUE;

	str1 = wmode_2_str(smode);
	str2 = wmode_2_str(tmode);

	if (str1)
		os_free_mem(str1);

	if (str2)
		os_free_mem(str2);

	return eq;
}

UCHAR wmode_2_rfic(USHORT PhyMode)
{
	UCHAR rf_mode = 0;

	if (WMODE_CAP_2G(PhyMode))
		rf_mode |= RFIC_24GHZ;

	if (WMODE_CAP_5G(PhyMode))
		rf_mode |= RFIC_5GHZ;

	if (WMODE_CAP_6G(PhyMode))
		rf_mode |= RFIC_6GHZ;

	return rf_mode;
}

/*N9 CMD BW value*/
void bw_2_str(UCHAR bw, CHAR *bw_str)
{
	switch (bw) {
	case BW_20:
		snprintf(bw_str, strlen(bw_str), "%s", "20");
		break;

	case BW_40:
		snprintf(bw_str, strlen(bw_str), "%s", "20/40");
		break;

	case BW_80:
		snprintf(bw_str, strlen(bw_str), "%s", "20/40/80");
		break;

	case BW_8080:
		snprintf(bw_str, strlen(bw_str), "%s", "20/40/80/160NC");
		break;

	case BW_160:
		snprintf(bw_str, strlen(bw_str), "%s", "20/40/80/160C");
		break;

	case BW_10:
		snprintf(bw_str, strlen(bw_str), "%s", "10");
		break;

	case BW_5:
		snprintf(bw_str, strlen(bw_str), "%s", "5");
		break;

	default:
		snprintf(bw_str, strlen(bw_str), "%s", "Invaild");
		break;
	}
}

void extcha_2_str(UCHAR extcha, CHAR *ec_str)
{
	switch (extcha) {
	case EXTCHA_NONE:
		snprintf(ec_str, strlen(ec_str), "%s", "NONE");
		break;

	case EXTCHA_ABOVE:
		snprintf(ec_str, strlen(ec_str), "%s", "ABOVE");
		break;

	case EXTCHA_BELOW:
		snprintf(ec_str, strlen(ec_str), "%s", "BELOW");
		break;

	case EXTCHA_NOASSIGN:
		snprintf(ec_str, strlen(ec_str), "%s", "Not assignment");
		break;

	default:
		snprintf(ec_str, strlen(ec_str), "%s", "Invaild");
		break;
	}
}

/*
	==========================================================================
	Description:
	Set Wireless Mode
	Return:
	TRUE if all parameters are OK, FALSE otherwise
	==========================================================================
*/

INT RT_CfgSetWirelessMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg, struct wifi_dev *wdev)
{
	LONG cfg_mode;
	USHORT wmode;
	UCHAR *mode_str;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	cfg_mode = os_str_tol(arg, 0, 10);
	/* check if chip support 5G band when WirelessMode is 5G band */
	wmode = cfgmode_2_wmode((UCHAR)cfg_mode);

	if (!wmode_valid_and_correct(pAd, &wmode)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s(): Invalid wireless mode(%ld, wmode=0x%x), ChipCap(%s)\n",
			  __func__, cfg_mode, wmode,
			  BAND_STR[pChipCap->phy_caps & 0x3]));
		return FALSE;
	}

#ifdef DOT11_VHT_AC
#if defined(MT76x2) || defined(MT7637)

	if (pChipCap->ac_off_mode && WMODE_CAP_AC(wmode)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("it doesn't support VHT AC!\n"));
		wmode &= ~(WMODE_AC);
	}

#endif /* MT76x2 */
#endif /* DOT11_VHT_AC */

	if (wmode_band_equal(wdev->PhyMode, wmode) == TRUE)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wmode_band_equal(): Band Equal!\n"));
	else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wmode_band_equal(): Band Not Equal!\n"));

	wdev->PhyMode = wmode;
	pAd->CommonCfg.cfg_wmode = wmode;
	mode_str = wmode_2_str(wmode);

	if (mode_str) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Set WMODE=%s(0x%x)\n",
				__func__, mode_str, wmode));
		os_free_mem(mode_str);
	}

	return TRUE;
}

/* maybe can be moved to GPL code, ap_mbss.c, but the code will be open */
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT

static BOOLEAN wmode_valid(RTMP_ADAPTER *pAd, enum WIFI_MODE wmode)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if ((WMODE_CAP_5G(wmode) && (!PHY_CAP_5G(cap->phy_caps))) ||
	    (WMODE_CAP_2G(wmode) && (!PHY_CAP_2G(cap->phy_caps))) ||
	    (WMODE_CAP_N(wmode) && RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N))
	   )
		return FALSE;
	else
		return TRUE;
}

/*
	==========================================================================
	Description:
	Set Wireless Mode for MBSS
	Return:
	TRUE if all parameters are OK, FALSE otherwise
	==========================================================================
*/
INT RT_CfgSetMbssWirelessMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT cfg_mode;
	UCHAR wmode;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	cfg_mode = os_str_tol(arg, 0, 10);
	wmode = cfgmode_2_wmode((UCHAR)cfg_mode);

	if ((wmode == WMODE_INVALID) || (!wmode_valid(pAd, wmode))) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s(): Invalid wireless mode(%d, wmode=0x%x), ChipCap(%s)\n",
			  __func__, cfg_mode, wmode,
			  BAND_STR[pChipCap->phy_caps & 0x3]));
		return FALSE;
	}

	if (WMODE_CAP_5G(wmode) && WMODE_CAP_2G(wmode)) {
		if (pAd->CommonCfg.dbdc_mode == 1) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("AP cannot support 2.4G/5G band mxied mode!\n"));
			return FALSE;
		}
	}

#ifdef DOT11_VHT_AC
#if defined(MT76x2) || defined(MT7637)

	if (pChipCap->ac_off_mode && WMODE_CAP_AC(wmode)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("it doesn't support VHT AC!\n"));
		wmode &= ~(WMODE_AC);
	}

#endif /* MT76x2 */
#endif /* DOT11_VHT_AC */
	pAd->CommonCfg.cfg_wmode = wmode;
	return TRUE;
}
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

static BOOLEAN RT_isLegalCmdBeforeInfUp(RTMP_STRING *SetCmd)
{
	BOOLEAN TestFlag;

	TestFlag =	!strcmp(SetCmd, "Debug") ||
#ifdef CONFIG_APSTA_MIXED_SUPPORT
			!strcmp(SetCmd, "OpMode") ||
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef EXT_BUILD_CHANNEL_LIST
			!strcmp(SetCmd, "CountryCode") ||
			!strcmp(SetCmd, "DfsType") ||
			!strcmp(SetCmd, "ChannelListAdd") ||
			!strcmp(SetCmd, "ChannelListShow") ||
			!strcmp(SetCmd, "ChannelListDel") ||
#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef SINGLE_SKU
			!strcmp(SetCmd, "ModuleTxpower") ||
#endif /* SINGLE_SKU */
			FALSE; /* default */
	return TestFlag;
}

INT RT_CfgSetShortSlot(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG ShortSlot;
	struct wifi_dev *wdev;
	UINT8 i;

	ShortSlot = os_str_tol(arg, 0, 10);
	if (ShortSlot < 0 || ShortSlot > 1)
		return FALSE; /*Invalid argument */

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		wdev = &pAd->ApCfg.MBSSID[i].wdev;
		wdev->bUseShortSlotTime = (ShortSlot == 1) ? TRUE : FALSE;
	}

	return TRUE;
}

/*
	==========================================================================
	Description:
	Set WEP KEY base on KeyIdx
	Return:
	TRUE if all parameters are OK, FALSE otherwise
	==========================================================================
*/
INT	RT_CfgSetWepKey(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *keyString,
	IN	CIPHER_KEY * pSharedKey,
	IN	INT				keyIdx)
{
	INT				KeyLen;
	INT				i;
	/*UCHAR			CipherAlg = CIPHER_NONE;*/
	BOOLEAN			bKeyIsHex = FALSE;
	/* TODO: Shall we do memset for the original key info??*/
	memset(pSharedKey, 0, sizeof(CIPHER_KEY));
	KeyLen = strlen(keyString);

	switch (KeyLen) {
	case 5: /*wep 40 Ascii type*/
	case 13: /*wep 104 Ascii type*/
#ifdef MT_MAC
	case 16: /*wep 128 Ascii type*/
#endif
		bKeyIsHex = FALSE;
		pSharedKey->KeyLen = KeyLen;
		NdisMoveMemory(pSharedKey->Key, keyString, KeyLen);
		break;

	case 10: /*wep 40 Hex type*/
	case 26: /*wep 104 Hex type*/
#ifdef MT_MAC
	case 32: /*wep 128 Hex type*/
#endif
		for (i = 0; i < KeyLen; i++) {
			if (!isxdigit(*(keyString + i)))
				return FALSE;  /*Not Hex value;*/
		}

		bKeyIsHex = TRUE;
		pSharedKey->KeyLen = KeyLen / 2;
		AtoH(keyString, pSharedKey->Key, pSharedKey->KeyLen);
		break;

	default: /*Invalid argument */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetWepKey(keyIdx=%d):Invalid argument (arg=%s)\n", keyIdx,
				keyString));
		return FALSE;
	}

	pSharedKey->CipherAlg = ((KeyLen % 5) ? CIPHER_WEP128 : CIPHER_WEP64);
#ifdef MT_MAC

	if (KeyLen == 32)
		pSharedKey->CipherAlg = CIPHER_WEP152;

#endif
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetWepKey:(KeyIdx=%d,type=%s, Alg=%s)\n",
			keyIdx, (bKeyIsHex == FALSE ? "Ascii" : "Hex"), CipherName[pSharedKey->CipherAlg]));
	return TRUE;
}

INT	RT_CfgSetFixedTxPhyMode(RTMP_STRING *arg)
{
	INT fix_tx_mode = FIXED_TXMODE_HT;
	ULONG value;

	if (rtstrcasecmp(arg, "OFDM") == TRUE)
		fix_tx_mode = FIXED_TXMODE_OFDM;
	else if (rtstrcasecmp(arg, "CCK") == TRUE)
		fix_tx_mode = FIXED_TXMODE_CCK;
	else if (rtstrcasecmp(arg, "HT") == TRUE)
		fix_tx_mode = FIXED_TXMODE_HT;
	else if (rtstrcasecmp(arg, "VHT") == TRUE)
		fix_tx_mode = FIXED_TXMODE_VHT;
	else {
		value = os_str_tol(arg, 0, 10);

		switch (value) {
		case FIXED_TXMODE_CCK:
		case FIXED_TXMODE_OFDM:
		case FIXED_TXMODE_HT:
		case FIXED_TXMODE_VHT:
			fix_tx_mode = value;
			break;
		default:
			fix_tx_mode = FIXED_TXMODE_HT;
		}
	}

	return fix_tx_mode;
}

INT	RT_CfgSetMacAddress(RTMP_ADAPTER *pAd, RTMP_STRING *arg, UCHAR idx, INT opmode)
{
	INT	i, mac_len;
	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	mac_len = strlen(arg);

	if (mac_len != 17) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : invalid length (%d)\n", __func__, mac_len));
		return FALSE;
	}

	if (strcmp(arg, "00:00:00:00:00:00") == 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : invalid mac setting\n", __func__));
		return FALSE;
	}

	if (opmode == OPMODE_AP) {
		if (idx == 0) {
			for (i = 0; i < MAC_ADDR_LEN; i++) {
				AtoH(arg, &pAd->CurrentAddress[i], 1);
				arg = arg + 3;
			}

			pAd->bLocalAdminMAC = TRUE;
		}

#ifdef MT_MAC
#ifdef MBSS_SUPPORT
		else {
			for (i = 0; i < MAC_ADDR_LEN; i++) {
				AtoH(arg, &pAd->ExtendMBssAddr[idx - 1][i], 1);
				arg = arg + 3;
			}

			/* TODO: Carter, is the below code still has its meaning? */
			pAd->bLocalAdminExtendMBssMAC = TRUE;
		}
#endif /* MBSS_SUPPORT */

#ifdef CONFIG_APSTA_MIXED_SUPPORT
	} else if (opmode == OPMODE_STA) {
		for (i = 0; i < MAC_ADDR_LEN; i++) {
			AtoH(arg, &pAd->ApcliAddr[idx][i], 1);
			arg = arg + 3;
		}
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#endif /* MT_MAC */
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: idx(%d) non-supported opmode(%d)\n", __func__, idx, opmode));
	}

	return TRUE;
}

INT	RT_CfgSetTxMCSProc(RTMP_STRING *arg, BOOLEAN *pAutoRate)
{
	INT	Value = os_str_tol(arg, 0, 10);
	INT	TxMcs;

	if ((Value >= 0 && Value <= 23) || (Value == 32)) { /* 3*3*/
		TxMcs = Value;
		*pAutoRate = FALSE;
	} else {
		TxMcs = MCS_AUTO;
		*pAutoRate = TRUE;
	}

	return TxMcs;
}

INT	RT_CfgSetAutoFallBack(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR AutoFallBack = (UCHAR)os_str_tol(arg, 0, 10);

	if (AutoFallBack)
		AutoFallBack = TRUE;
	else
		AutoFallBack = FALSE;

	AsicSetAutoFallBack(pAd, (AutoFallBack) ? TRUE : FALSE);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetAutoFallBack::(AutoFallBack=%d)\n", AutoFallBack));
	return TRUE;
}

#ifdef WSC_INCLUDED
INT	RT_CfgSetWscPinCode(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *pPinCodeStr,
	OUT PWSC_CTRL   pWscControl)
{
	UINT pinCode;

	pinCode = (UINT) os_str_tol(pPinCodeStr, 0, 10); /* When PinCode is 03571361, return value is 3571361.*/

	if (strlen(pPinCodeStr) == 4) {
		pWscControl->WscEnrolleePinCode = pinCode;
		pWscControl->WscEnrolleePinCodeLen = 4;
	} else if (ValidateChecksum(pinCode)) {
		pWscControl->WscEnrolleePinCode = pinCode;
		pWscControl->WscEnrolleePinCodeLen = 8;
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RT_CfgSetWscPinCode(): invalid Wsc PinCode (%d)\n", pinCode));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RT_CfgSetWscPinCode():Wsc PinCode=%d\n", pinCode));
	return TRUE;
}
#endif /* WSC_INCLUDED */

/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWNAME.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT RtmpIoctl_rt_ioctl_giwname(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID *pData,
	IN	ULONG					Data)
{
#ifdef P2P_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
#endif /* P2P_SUPPORT */
	UCHAR CurOpMode = OPMODE_AP;

	if (CurOpMode == OPMODE_AP) {
#ifdef P2P_SUPPORT

		if (pObj->ioctl_if_type == INT_P2P) {
			if (P2P_CLI_ON(pAd))
				strcpy(pData, "Ralink P2P Cli");
			else if (P2P_GO_ON(pAd))
				strcpy(pData, "Ralink P2P GO");
			else
				strcpy(pData, "Ralink P2P");
		} else
#endif /* P2P_SUPPORT */
			strlcpy(pData, "RTWIFI SoftAP", strlen("RTWIFI SoftAP") + 1);
	}

	return NDIS_STATUS_SUCCESS;
}

VOID rtmp_chip_prepare(RTMP_ADAPTER *pAd)
{
}

extern struct wifi_dev_ops ap_wdev_ops;
extern struct wifi_dev_ops sta_wdev_ops;

static VOID rtmp_netdev_set(RTMP_ADAPTER *pAd, PNET_DEV net_dev)
{
	struct wifi_dev *wdev = NULL;
	INT32 ret = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* set main net_dev */
	pAd->net_dev = net_dev;
#ifdef CONFIG_AP_SUPPORT

	if (pAd->OpMode == OPMODE_AP) {
		BSS_STRUCT *pMbss;

		pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
		ASSERT(pMbss);
		wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
		RTMP_OS_NETDEV_SET_WDEV(net_dev, wdev);
		ret = wdev_init(pAd, wdev, WDEV_TYPE_AP, net_dev, MAIN_MBSSID, (VOID *)pMbss, (VOID *)pAd);

		if (!ret) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
					RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
			RtmpOSNetDevFree(pAd->net_dev);
			return;
		}

		ret = wdev_ops_register(wdev, WDEV_TYPE_AP, &ap_wdev_ops,
					cap->qos.wmm_detect_method);

		if (!ret) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("register wdev_ops %s failed, free net device!\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
			RtmpOSNetDevFree(pAd->net_dev);
			return;
		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (pAd->OpMode == OPMODE_STA) {
		wdev = &pAd->StaCfg[MAIN_MSTA_ID].wdev;
		RTMP_OS_NETDEV_SET_WDEV(net_dev, wdev);
		ret = wdev_init(pAd, wdev, WDEV_TYPE_STA, net_dev, MAIN_MSTA_ID, (VOID *)&pAd->StaCfg[MAIN_MSTA_ID], (VOID *)pAd);

		if (!ret) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
					RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
			RtmpOSNetDevFree(pAd->net_dev);
			return;
		}

		ret = wdev_ops_register(wdev, WDEV_TYPE_STA, &sta_wdev_ops,
					cap->qos.wmm_detect_method);

		if (!ret) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("register wdev_ops %s failed, free net device!\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
			RtmpOSNetDevFree(pAd->net_dev);
			return;
		}
	}

#endif /* CONFIG_STA_SUPPORT */
}

INT RTMP_COM_IoctlHandle(
	IN	VOID					*pAdSrc,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT Status = NDIS_STATUS_SUCCESS, i;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
		wdev = &pStaCfg->wdev;
	}
#endif
	pObj = pObj; /* avoid compile warning */

	switch (cmd) {
	case CMD_RTPRIV_IOCTL_NETDEV_GET:
		/* get main net_dev */
	{
		VOID **ppNetDev = (VOID **)pData;
		*ppNetDev = (VOID *)(pAd->net_dev);
	}
	break;
#ifdef APCLI_CFG80211_SUPPORT
	case CMD_RTPRIV_IOCTL_APCLI_NETDEV_GET:
		/*get apcli net dev for CFG80211 mode */
	{
		VOID **ppNetDev = (VOID **)pData;
		*ppNetDev = (VOID *)(pAd->StaCfg[0].wdev.if_dev);
	}
	break;
#endif /* APCLI_CFG80211_SUPPORT */

	case CMD_RTPRIV_IOCTL_NETDEV_SET: {
		rtmp_netdev_set(pAd, pData);
		break;
	}

	case CMD_RTPRIV_IOCTL_OPMODE_GET:
		/* get Operation Mode */
		*(UINT32 *)pData = pAd->OpMode;
		break;

	case CMD_RTPRIV_IOCTL_TASK_LIST_GET:
		/* get all Tasks */
	{
		RT_CMD_WAIT_QUEUE_LIST *pList = (RT_CMD_WAIT_QUEUE_LIST *)pData;

		pList->pMlmeTask = &pAd->mlmeTask;
#ifdef RTMP_TIMER_TASK_SUPPORT
		if (IS_ASIC_CAP(pAd, fASIC_CAP_MGMT_TIMER_TASK))
			pList->pTimerTask = &pAd->timerTask;
#endif /* RTMP_TIMER_TASK_SUPPORT */
		pList->pCmdQTask = &pAd->cmdQTask;
#ifdef WSC_INCLUDED
		pList->pWscTask = &pAd->wscTask;
#endif /* WSC_INCLUDED */
	}
	break;
	case CMD_RTPRIV_IOCTL_NIC_NOT_EXIST:
		/* set driver state to fRTMP_ADAPTER_NIC_NOT_EXIST */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
		break;

	case CMD_RTPRIV_IOCTL_MCU_SLEEP_CLEAR:
		RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
		break;
#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

	case CMD_RTPRIV_IOCTL_USB_DEV_GET:
		/* get USB DEV */
	{
		VOID **ppUsb_Dev = (VOID **)pData;
		*ppUsb_Dev = (VOID *)(pObj->pUsb_Dev);
	}
	break;

	case CMD_RTPRIV_IOCTL_USB_INTF_GET:
		/* get USB INTF */
	{
		VOID **ppINTF = (VOID **)pData;
		*ppINTF = (VOID *)(pObj->intf);
	}
	break;

	case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_SET:
		/* set driver state to fRTMP_ADAPTER_SUSPEND */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_CLEAR:
		/* clear driver state to fRTMP_ADAPTER_SUSPEND */
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
		RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_SEND_DISSASSOCIATE:

		/* clear driver state to fRTMP_ADAPTER_SUSPEND */
		if (INFRA_ON(pStaCfg) &&
		    (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))) {
			MLME_DISASSOC_REQ_STRUCT	DisReq;
			MLME_QUEUE_ELEM *MsgElem;

			os_alloc_mem(NULL, (UCHAR **)&MsgElem, sizeof(MLME_QUEUE_ELEM));

			if (MsgElem) {
				COPY_MAC_ADDR(DisReq.Addr, pAd->CommonCfg.Bssid);
				DisReq.Reason =  REASON_DEAUTH_STA_LEAVING;
				MsgElem->Machine = ASSOC_FSM;
				MsgElem->MsgType = ASSOC_FSM_MLME_DISASSOC_REQ;
				MsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
				NdisMoveMemory(MsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));
				/* Prevent to connect AP again in STAMlmePeriodicExec*/
				pAd->MlmeAux.AutoReconnectSsidLen = 32;
				NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
				pStaCfg->CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;
				MlmeDisassocReqAction(pAd, MsgElem);
				os_free_mem(MsgElem);
			}

			/*				RtmpusecDelay(1000);*/
			RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CGIWAP, -1, NULL, NULL, 0);
		}

		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_TEST:
		/* test driver state to fRTMP_ADAPTER_SUSPEND */
		*(UCHAR *)pData = RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_IDLE_RADIO_OFF_TEST:
		/* test driver state to fRTMP_ADAPTER_IDLE_RADIO_OFF */
		*(UCHAR *)pData = RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_OFF:
		ASIC_RADIO_OFF(pAd, SUSPEND_RADIO_OFF);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_ON:
		ASIC_RADIO_ON(pAd, RESUME_RADIO_ON);
		break;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_STATUS:
		*(UCHAR *)pData = (UCHAR)pAd->WOW_Cfg.bEnable;
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_ENABLE:
		ASIC_WOW_ENABLE(pAd, pStaCfg);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_DISABLE:
		ASIC_WOW_DISABLE(pAd, pStaCfg);
		break;
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */
#endif /* CONFIG_PM */
#ifdef CONFIG_STA_SUPPORT
	case CMD_RTPRIV_IOCTL_AP_BSSID_GET:
		if (pStaCfg && pStaCfg->wdev.PortSecured == WPA_802_1X_PORT_NOT_SECURED)
			NdisCopyMemory(pData, pStaCfg->MlmeAux.Bssid, 6);
		else
			return NDIS_STATUS_FAILURE;

		break;
#endif
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

	case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_SET:
		/* set driver state to fRTMP_ADAPTER_SUSPEND */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_CLEAR:
		/* clear driver state to fRTMP_ADAPTER_SUSPEND */
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		break;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_OFF:
		ASIC_RADIO_OFF(pAd, SUSPEND_RADIO_OFF);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_ON:
		ASIC_RADIO_ON(pAd, RESUME_RADIO_ON);
		break;
#endif /* CONFIG_STA_SUPPORT */

	case CMD_RTPRIV_IOCTL_SANITY_CHECK:

		/* sanity check before IOCTL */
		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS))
#ifdef IFUP_IN_PROBE
		    || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
		    || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
#endif /* IFUP_IN_PROBE */
		   ) {
			if (pData == NULL ||	RT_isLegalCmdBeforeInfUp((RTMP_STRING *) pData) == FALSE)
				return NDIS_STATUS_FAILURE;
		}

		break;

	case CMD_RTPRIV_IOCTL_SIOCGIWFREQ:
		/* get channel number */
#ifdef WDS_SUPPORT
		if (pObj->ioctl_if_type == INT_WDS)
			wdev = get_wdev_by_idx(pAd, (INT)Data + MIN_NET_DEVICE_FOR_WDS);
		else
#endif
#ifdef APCLI_SUPPORT
			if (pObj->ioctl_if_type == INT_APCLI)
				wdev = get_wdev_by_idx(pAd, (INT)Data + MIN_NET_DEVICE_FOR_APCLI);
			else
#endif
				wdev = get_wdev_by_idx(pAd, (INT)Data);

		if (wdev)
			*(ULONG *)pData = wdev->channel;

		break;
#ifdef SNIFFER_SUPPORT

	case CMD_RTPRIV_IOCTL_SNIFF_INIT:
		Monitor_Init(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_SNIFF_OPEN:
		if (Monitor_Open(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_SNIFF_CLOSE:
		if (Monitor_Close(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_SNIFF_REMOVE:
		Monitor_Remove(pAd);
		break;
#endif /*SNIFFER_SUPPORT*/
#ifdef P2P_SUPPORT

	case CMD_RTPRIV_IOCTL_P2P_INIT:
		P2pInit(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_P2P_REMOVE:
		P2P_Remove(pAd);
		break;

	case CMD_RTPRIV_IOCTL_P2P_OPEN_PRE:
		if (P2P_OpenPre(pData) != 0)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_P2P_OPEN_POST:
		if (P2P_OpenPost(pData) != 0)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_P2P_CLOSE:
		P2P_Close(pData);
		break;
#endif /* P2P_SUPPORT */

	case CMD_RTPRIV_IOCTL_BEACON_UPDATE:
		/* update all beacon contents */
#ifdef CONFIG_AP_SUPPORT
		/* TODO: Carter, the oid seems been obsoleted. */
		UpdateBeaconHandler(
			pAd,
			get_default_wdev(pAd),
			BCN_UPDATE_ALL_AP_RENEW);
#endif /* CONFIG_AP_SUPPORT */
		break;

	case CMD_RTPRIV_IOCTL_RXPATH_GET:
		/* get the number of rx path */
		*(ULONG *)pData = pAd->Antenna.field.RxPath;
		break;

	case CMD_RTPRIV_IOCTL_CHAN_LIST_NUM_GET:
		if (wdev)
			BandIdx = HcGetBandByWdev(wdev);
		else {
			BandIdx = BAND0;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[CMD_RTPRIV_IOCTL_CHAN_LIST_NUM_GET] wdev = NULL\n"));
		}
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		*(ULONG *)pData = pChCtrl->ChListNum;
		break;

	case CMD_RTPRIV_IOCTL_CHAN_LIST_GET: {
		UINT32 i;
		UCHAR *pChannel = (UCHAR *)pData;
		if (wdev)
			BandIdx = HcGetBandByWdev(wdev);
		else {
			BandIdx = BAND0;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[CMD_RTPRIV_IOCTL_CHAN_LIST_GET] wdev = NULL\n"));
		}
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

		for (i = 1; i <= pChCtrl->ChListNum; i++) {
			*pChannel = pChCtrl->ChList[i - 1].Channel;
			pChannel++;
		}
	}
	break;

	case CMD_RTPRIV_IOCTL_FREQ_LIST_GET: {
		UINT32 i;
		UINT32 *pFreq = (UINT32 *)pData;
		UINT32 m;
		if (wdev)
			BandIdx = HcGetBandByWdev(wdev);
		else {
			BandIdx = BAND0;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[CMD_RTPRIV_IOCTL_FREQ_LIST_GET] wdev = NULL\n"));
		}
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

		for (i = 1; i <= pChCtrl->ChListNum; i++) {
			m = 2412000;
			MAP_CHANNEL_ID_TO_KHZ(pChCtrl->ChList[i - 1].Channel, m);
			(*pFreq) = m;
			pFreq++;
		}
	}
	break;
#ifdef EXT_BUILD_CHANNEL_LIST

	case CMD_RTPRIV_SET_PRECONFIG_VALUE:
		/* Set some preconfigured value before interface up*/
		pAd->CommonCfg.DfsType = MAX_RD_REGION;
		break;
#endif /* EXT_BUILD_CHANNEL_LIST */

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
	case CMD_RTPRIV_IOCTL_PCI_SUSPEND:
	case CMD_RTPRIV_IOCTL_RBUS_SUSPEND:
#ifdef CONFIG_STA_SUPPORT
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

#ifdef MT_WOW_SUPPORT
		if ((pAd->WOW_Cfg.bEnable == TRUE) && INFRA_ON(pStaCfg)) {
			BOOLEAN InfraAP_BW = FALSE;
			UCHAR BwFallBack = 0;

			if (pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40)
				InfraAP_BW = TRUE;
			else
				InfraAP_BW = FALSE;

			AdjustChannelRelatedValue(pAd,
						  &BwFallBack,
						  BSS0,
						  InfraAP_BW,
						  pStaCfg->MlmeAux.Channel,
						  pStaCfg->MlmeAux.CentralChannel,
						  wdev);
			pAd->WOW_Cfg.bWoWRunning = TRUE;
			ASIC_WOW_ENABLE(pAd, pStaCfg);
			AsicExtPmStateCtrl(pAd, pStaCfg, PM4, ENTER_PM_STATE);
		}
#endif /*MT_WOW_SUPPORT */

#ifdef MT_WOW_SUPPORT
		if (!pAd->WOW_Cfg.bWoWRunning)
#endif /* MT_WOW_SUPPORT */
		{
			MlmeRadioOff(pAd, wdev);
		}

		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
		chip_interrupt_disable(pAd);
		chip_set_hif_dma(pAd, DMA_TX_RX, 0);
#ifdef CONFIG_FWOWN_SUPPORT
		FwOwn(pAd);
#endif /* CONFIG_FWOWN_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
		break;

	case CMD_RTPRIV_IOCTL_PCI_RESUME:
	case CMD_RTPRIV_IOCTL_RBUS_RESUME:
#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_FWOWN_SUPPORT
		DriverOwn(pAd);
#endif /* CONFIG_FWOWN_SUPPORT */
		chip_set_hif_dma(pAd, DMA_TX_RX, 1);
		chip_interrupt_enable(pAd);
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

#ifdef MT_WOW_SUPPORT
		if (!pAd->WOW_Cfg.bWoWRunning)
#endif /* MT_WOW_SUPPORT */
		{
			MlmeRadioOn(pAd, wdev);
		}

#ifdef MT_WOW_SUPPORT
		if (pAd->WOW_Cfg.bWoWRunning == TRUE) {
			AsicExtPmStateCtrl(pAd, pStaCfg, PM4, EXIT_PM_STATE);
			ASIC_WOW_DISABLE(pAd, pStaCfg);
			pAd->WOW_Cfg.bWoWRunning = FALSE;
		}
#endif /* MT_WOW_SUPPORT */

		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

		/* Delete all BA sessions after resume */
		if (pStaCfg)
			ba_session_tear_down_all(pAd, pStaCfg->PwrMgmt.wcid, FALSE);

#ifdef HOST_RESUME_DONE_ACK_SUPPORT
		rtmp_host_resume_done_ack(pAd);
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
		break;
#endif /* RTMP_PCI_SUPPORT || RTMP_RBUS_SUPPORT */

#if defined(RTMP_PCI_SUPPORT)
	case CMD_RTPRIV_IOCTL_PCI_CSR_SET:
	{
		struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

		pci_hif->CSRBaseAddress = (PUCHAR)Data;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("PCI CSRBaseAddress =0x%lx, csr_addr=0x%lx!\n",
				 (ULONG)pci_hif->CSRBaseAddress, (ULONG)Data));
		break;
	}
	case CMD_RTPRIV_IOCTL_PCIE_INIT:
		RTMPInitPCIeDevice(pData, pAd);
		break;
#endif /* RTMP_PCI_SUPPORT */


	case CMD_RTPRIV_IOCTL_CHIP_PREPARE: {
		rtmp_chip_prepare(pAd);
	}
	break;
#ifdef RT_CFG80211_SUPPORT

	case CMD_RTPRIV_IOCTL_CFG80211_CFG_START:
		if (wdev)
			RT_CFG80211_REINIT(pAd, wdev);

#ifndef DISABLE_HOSTAPD_BEACON
		RT_CFG80211_CRDA_REG_RULE_APPLY(pAd);
#endif

		break;
#endif /* RT_CFG80211_SUPPORT */
#ifdef INF_PPA_SUPPORT

	case CMD_RTPRIV_IOCTL_INF_PPA_INIT:
		os_alloc_mem(NULL, (UCHAR **)&(pAd->pDirectpathCb), sizeof(PPA_DIRECTPATH_CB));
		break;

	case CMD_RTPRIV_IOCTL_INF_PPA_EXIT:
		if (ppa_hook_directpath_register_dev_fn && (pAd->PPAEnable == TRUE)) {
			UINT status;

			status = ppa_hook_directpath_register_dev_fn(&pAd->g_if_id, pAd->net_dev, NULL, 0);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Unregister PPA::status=%d, if_id=%d\n", status, pAd->g_if_id));
		}

		os_free_mem(pAd->pDirectpathCb);
		break;
#endif /* INF_PPA_SUPPORT*/

	case CMD_RTPRIV_IOCTL_VIRTUAL_INF_UP:
		/* interface up */
	{
		RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s -> CMD_RTPRIV_IOCTL_VIRTUAL_INF_UP\n", __func__));
		pInfConf->virtual_if_up_handler(pInfConf->operation_dev_p);
	}
	break;

	case CMD_RTPRIV_IOCTL_VIRTUAL_INF_DOWN:
		/* interface down */
	{
		RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s -> CMD_RTPRIV_IOCTL_VIRTUAL_INF_DOWN\n", __func__));
		VIRTUAL_IF_DEC(pAd);
		Status = pInfConf->virtual_if_down_handler(pInfConf->operation_dev_p);
	}
	break;

	case CMD_RTPRIV_IOCTL_VIRTUAL_INF_INIT:
		/* init at first interface up */
	{
		RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s -> CMD_RTPRIV_IOCTL_VIRTUAL_INF_INIT\n", __func__));
		pInfConf->virtual_if_init_handler(pInfConf->operation_dev_p);
	}
	break;

	case CMD_RTPRIV_IOCTL_VIRTUAL_INF_DEINIT:
		/* deinit at last interface down */
	{
		RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s -> CMD_RTPRIV_IOCTL_VIRTUAL_INF_DEINIT\n", __func__));
		pInfConf->virtual_if_deinit_handler(pInfConf->operation_dev_p);
	}
	break;

	case CMD_RTPRIV_IOCTL_VIRTUAL_INF_GET:
		/* get virtual interface number */
		*(ULONG *)pData = VIRTUAL_IF_NUM(pAd);
		break;

	case CMD_RTPRIV_IOCTL_INF_TYPE_GET:
		/* get current interface type */
		*(ULONG *)pData = pAd->infType;
		break;

	case CMD_RTPRIV_IOCTL_INF_STATS_GET:
		/* get statistics */
	{
		RT_CMD_STATS *pStats = (RT_CMD_STATS *)pData;

		pStats->pStats = pAd->stats;

		if (pAd->OpMode == OPMODE_STA) {
			pStats->rx_packets = pAd->WlanCounters[0].ReceivedFragmentCount.QuadPart;
			pStats->tx_packets = pAd->WlanCounters[0].TransmittedFragmentCount.QuadPart;
			pStats->rx_bytes = pAd->RalinkCounters.ReceivedByteCount;
			pStats->tx_bytes = pAd->RalinkCounters.TransmittedByteCount;
			pStats->rx_errors = pAd->Counters8023.RxErrors;
			pStats->tx_errors = pAd->Counters8023.TxErrors;
			pStats->multicast = pAd->WlanCounters[0].MulticastReceivedFrameCount.QuadPart;   /* multicast packets received*/
			pStats->collisions = 0;  /* Collision packets*/
			pStats->rx_over_errors = pAd->Counters8023.RxNoBuffer;				   /* receiver ring buff overflow*/
			pStats->rx_crc_errors = 0;/*pAd->WlanCounters[0].FCSErrorCount;	  recved pkt with crc error*/
			pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
			pStats->rx_fifo_errors = pAd->Counters8023.RxNoBuffer;				   /* recv'r fifo overrun*/
		}

#ifdef CONFIG_AP_SUPPORT
		else if (pAd->OpMode == OPMODE_AP) {
			INT index;
			BOOLEAN found_it = FALSE;
			INT stat_db_source;

			for (index = 0; index < MAX_MBSSID_NUM(pAd); index++) {
				if (pAd->ApCfg.MBSSID[index].wdev.if_dev == (PNET_DEV)(pStats->pNetDev)) {
					found_it = TRUE;
					stat_db_source = 0;
					break;
				}
			}
#ifdef CONFIG_STA_SUPPORT
			if (found_it == FALSE) {
				index = 0;
				if (pAd->StaCfg[index].wdev.if_dev == (PNET_DEV)(pStats->pNetDev)) {
					found_it = TRUE;
					stat_db_source = 1;
				}
			}
#endif /* CONFIG_STA_SUPPORT */
#ifdef APCLI_SUPPORT

			if (found_it == FALSE) {
				for (index = 0; index < pAd->ApCfg.ApCliNum; index++) {
					if (pAd->StaCfg[index].wdev.if_dev == (PNET_DEV)(pStats->pNetDev)) {
						found_it = TRUE;
						stat_db_source = 1;
						break;
					}
				}
			}

#endif

			if (found_it == FALSE) {
				/* reset counters */
				pStats->rx_packets = 0;
				pStats->tx_packets = 0;
				pStats->rx_bytes = 0;
				pStats->tx_bytes = 0;
				pStats->rx_errors = 0;
				pStats->tx_errors = 0;
				pStats->multicast = 0;   /* multicast packets received*/
				pStats->collisions = 0;  /* Collision packets*/
				pStats->rx_over_errors = 0; /* receiver ring buff overflow*/
				pStats->rx_crc_errors = 0; /* recved pkt with crc error*/
				pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
				pStats->rx_fifo_errors = 0; /* recv'r fifo overrun*/
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CMD_RTPRIV_IOCTL_INF_STATS_GET: can not find mbss I/F\n"));
				return NDIS_STATUS_FAILURE;
			}

			if (stat_db_source == 0) {
				pStats->rx_packets = pAd->ApCfg.MBSSID[index].RxCount;
				pStats->tx_packets = pAd->ApCfg.MBSSID[index].TxCount;
				pStats->rx_bytes = pAd->ApCfg.MBSSID[index].ReceivedByteCount;
				pStats->tx_bytes = pAd->ApCfg.MBSSID[index].TransmittedByteCount;
				pStats->rx_errors = pAd->ApCfg.MBSSID[index].RxErrorCount;
				pStats->tx_errors = pAd->ApCfg.MBSSID[index].TxErrorCount;
				pStats->multicast = pAd->ApCfg.MBSSID[index].mcPktsRx; /* multicast packets received */
				pStats->collisions = 0;  /* Collision packets*/
				pStats->rx_over_errors = 0;				   /* receiver ring buff overflow*/
				pStats->rx_crc_errors = 0;/* recved pkt with crc error*/
				pStats->rx_frame_errors = 0;          /* recv'd frame alignment error*/
				pStats->rx_fifo_errors = 0;                   /* recv'r fifo overrun*/
			} else if (stat_db_source == 1) {
#ifdef CONFIG_STA_SUPPORT
				pStats->rx_packets = pAd->StaCfg[index].StaStatistic.RxCount;
				pStats->tx_packets = pAd->StaCfg[index].StaStatistic.TxCount;
				pStats->rx_bytes = pAd->StaCfg[index].StaStatistic.ReceivedByteCount;
				pStats->tx_bytes = pAd->StaCfg[index].StaStatistic.TransmittedByteCount;
				pStats->rx_errors = pAd->StaCfg[index].StaStatistic.RxErrorCount;
				pStats->tx_errors = pAd->StaCfg[index].StaStatistic.TxErrorCount;
				pStats->multicast = pAd->StaCfg[index].StaStatistic.mcPktsRx; /* multicast packets received */
				pStats->collisions = 0; /* Collision packets*/
				pStats->rx_over_errors = 0; /* receiver ring buff overflow*/
				pStats->rx_crc_errors = 0;/* recved pkt with crc error*/
				pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
				pStats->rx_fifo_errors = 0;
#endif /* CONFIG_STA_SUPPORT */
#ifdef APCLI_SUPPORT
				pStats->rx_packets = pAd->StaCfg[index].StaStatistic.RxCount;
				pStats->tx_packets = pAd->StaCfg[index].StaStatistic.TxCount;
				pStats->rx_bytes = pAd->StaCfg[index].StaStatistic.ReceivedByteCount;
				pStats->tx_bytes = pAd->StaCfg[index].StaStatistic.TransmittedByteCount;
				pStats->rx_errors = pAd->StaCfg[index].StaStatistic.RxErrorCount;
				pStats->tx_errors = pAd->StaCfg[index].StaStatistic.TxErrorCount;
				pStats->multicast = pAd->StaCfg[index].StaStatistic.mcPktsRx; /* multicast packets received */
				pStats->collisions = 0; /* Collision packets*/
				pStats->rx_over_errors = 0; /* receiver ring buff overflow*/
				pStats->rx_crc_errors = 0;/* recved pkt with crc error*/
				pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
				pStats->rx_fifo_errors = 0; /* recv'r fifo overrun*/
#endif
			}

		}

#endif
	}
	break;

	case CMD_RTPRIV_IOCTL_INF_IW_STATUS_GET:

		/* get wireless statistics */
	{
		UCHAR CurOpMode = OPMODE_AP;
#ifdef CONFIG_AP_SUPPORT
		PMAC_TABLE_ENTRY pMacEntry = NULL;
#endif /* CONFIG_AP_SUPPORT */
		RT_CMD_IW_STATS *pStats = (RT_CMD_IW_STATS *)pData;

		pStats->qual = 0;
		pStats->level = 0;
		pStats->noise = 0;
		pStats->pStats = pAd->iw_stats;
#ifdef CONFIG_STA_SUPPORT

		if (pAd->OpMode == OPMODE_STA) {
			CurOpMode = OPMODE_STA;
#ifdef P2P_SUPPORT

			if (pStats->priv_flags == INT_P2P)
				CurOpMode = OPMODE_AP;

#endif /* P2P_SUPPORT */
		}

#endif /* CONFIG_STA_SUPPORT */

		/*check if the interface is down*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS))
			return NDIS_STATUS_FAILURE;

#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == OPMODE_AP) {
#ifdef APCLI_SUPPORT

			if ((pStats->priv_flags == INT_APCLI)
#ifdef P2P_SUPPORT
			    || (P2P_CLI_ON(pAd))
#endif /* P2P_SUPPORT */
			   ) {
				INT ApCliIdx = ApCliIfLookUp(pAd, (PUCHAR)pStats->dev_addr);

				if ((ApCliIdx >= 0) && IS_WCID_VALID(pAd, pAd->StaCfg[ApCliIdx].MacTabWCID))
					pMacEntry = &pAd->MacTab.Content[pAd->StaCfg[ApCliIdx].MacTabWCID];
			} else
#endif /* APCLI_SUPPORT */
			{
				/*
					only AP client support wireless stats function.
					return NULL pointer for all other cases.
				*/
				pMacEntry = NULL;
			}
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

		if (CurOpMode == OPMODE_STA)
			pStats->qual = ((pStaCfg->ChannelQuality * 12) / 10 + 10);

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == OPMODE_AP) {
			if (pMacEntry != NULL)
				pStats->qual = ((pMacEntry->ChannelQuality * 12) / 10 + 10);
			else
				pStats->qual = ((pAd->Mlme.ChannelQuality * 12) / 10 + 10);
		}

#endif /* CONFIG_AP_SUPPORT */

		if (pStats->qual > 100)
			pStats->qual = 100;

#ifdef CONFIG_STA_SUPPORT

		if (CurOpMode == OPMODE_STA) {
			pStats->level =
				RTMPMaxRssi(pAd, pStaCfg->RssiSample.AvgRssi[0],
					    pStaCfg->RssiSample.AvgRssi[1],
					    pStaCfg->RssiSample.AvgRssi[2]);
			pStats->noise = RTMPMaxRssi(pAd, pStaCfg->RssiSample.AvgRssi[0],
						    pStaCfg->RssiSample.AvgRssi[1],
						    pStaCfg->RssiSample.AvgRssi[2]) -
					RTMPMinSnr(pAd, pAd->StaCfg[0].RssiSample.AvgSnr[0],
						   pStaCfg->RssiSample.AvgSnr[1]);
		}

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == OPMODE_AP) {
			if (pMacEntry != NULL)
				pStats->level =
					RTMPMaxRssi(pAd, pMacEntry->RssiSample.AvgRssi[0],
						    pMacEntry->RssiSample.AvgRssi[1],
						    pMacEntry->RssiSample.AvgRssi[2]);

#ifdef P2P_APCLI_SUPPORT
			else
				pStats->level =
					RTMPMaxRssi(pAd, pAd->StaCfg[0].RssiSample.AvgRssi[0],
						    pAd->StaCfg[0].RssiSample.AvgRssi[1],
						    pAd->StaCfg[0].RssiSample.AvgRssi[2]);

#endif /* P2P_APCLI_SUPPORT */
			pStats->noise = RTMPMaxRssi(pAd, pAd->ApCfg.RssiSample.AvgRssi[0],
						    pAd->ApCfg.RssiSample.AvgRssi[1],
						    pAd->ApCfg.RssiSample.AvgRssi[2]) -
					RTMPMinSnr(pAd, pAd->ApCfg.RssiSample.AvgSnr[0],
						   pAd->ApCfg.RssiSample.AvgSnr[1]);
		}

#endif /* CONFIG_AP_SUPPORT */

	}
	break;

	case CMD_RTPRIV_IOCTL_INF_MAIN_CREATE:
		*(VOID **)pData = RtmpPhyNetDevMainCreate(pAd);
		break;

	case CMD_RTPRIV_IOCTL_INF_MAIN_ID_GET:
		*(ULONG *)pData = INT_MAIN;
		break;

	case CMD_RTPRIV_IOCTL_INF_MAIN_CHECK:
		if (Data != INT_MAIN)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_INF_P2P_CHECK:
		if (Data != INT_P2P)
			return NDIS_STATUS_FAILURE;

		break;
#ifdef WDS_SUPPORT

	case CMD_RTPRIV_IOCTL_WDS_INIT:
		WDS_Init(pAd, (UCHAR)Data, pData);
		break;

	case CMD_RTPRIV_IOCTL_WDS_REMOVE:
		WDS_Remove(pAd);
		break;

	case CMD_RTPRIV_IOCTL_WDS_STATS_GET:
		if (Data == INT_WDS) {
			if (WDS_StatsGet(pAd, pData) != TRUE)
				return NDIS_STATUS_FAILURE;
		} else
			return NDIS_STATUS_FAILURE;

		break;
#endif /* WDS_SUPPORT */

#ifdef CONFIG_ATE
#ifdef CONFIG_QA

	/* Note: temp for wlan_service/original coexistence */
#ifdef CONFIG_WLAN_SERVICE
	case (CMD_RTPRIV_IOCTL_COMMON)CMD_RTPRIV_IOCTL_ATE: {
		struct hqa_frame *hqa_frame;
		struct hqa_frame_ctrl local_hqa;

		os_alloc_mem_suspend(pAd, (UCHAR **)&hqa_frame, sizeof(*hqa_frame));

		if (!hqa_frame) {
			Status = -ENOMEM;
			break;
		}

		NdisZeroMemory(hqa_frame, sizeof(*hqa_frame));
		Status = copy_from_user((PUCHAR)hqa_frame, wrq->u.data.pointer, wrq->u.data.length);

		if (Status)	{
			Status = -EFAULT;
			goto IOCTL_TEST_ERROR;
		}

		/* 0 means eth type hqa frame*/
		local_hqa.type = 0;
		local_hqa.hqa_frame_comm.hqa_frame_eth = hqa_frame;
		Status = mt_agent_hqa_cmd_handler(&pAd->serv, (struct hqa_frame_ctrl *)&local_hqa);

		/* Update cmd_frame length */
		wrq->u.data.length = sizeof((hqa_frame)->magic_no) + sizeof((hqa_frame)->type)
					+ sizeof((hqa_frame)->id) + sizeof((hqa_frame)->length)
					+ sizeof((hqa_frame)->sequence) + OS_NTOHS((hqa_frame)->length);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("wrq->u.data.length=%d, usr_addr=%p, hqa_addr=%p\n",
			wrq->u.data.length, wrq->u.data.pointer, hqa_frame));

		/* Feedback as soon as we can to avoid QA timeout */
		if (copy_to_user(wrq->u.data.pointer, (UCHAR *)hqa_frame, wrq->u.data.length)) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("copy_to_user() fail in %s\n", __func__));
			Status = -EFAULT;
			break;
		}

		/* TODO: Check sanity */
IOCTL_TEST_ERROR:
		os_free_mem(hqa_frame);
	}
	break;
#else
	case (CMD_RTPRIV_IOCTL_COMMON)CMD_RTPRIV_IOCTL_ATE: {
		struct _HQA_CMD_FRAME *HqaCmdFrame;

		os_alloc_mem_suspend(pAd, (UCHAR **)&HqaCmdFrame, sizeof(*HqaCmdFrame));

		if (!HqaCmdFrame) {
			Status = -ENOMEM;
			break;
		}

		NdisZeroMemory(HqaCmdFrame, sizeof(*HqaCmdFrame));
		Status = copy_from_user((PUCHAR)HqaCmdFrame, wrq->u.data.pointer, wrq->u.data.length);

		if (Status)	{
			Status = -EFAULT;
			goto IOCTL_ATE_ERROR;
		}

		Status = HQA_CMDHandler(pAd, wrq, HqaCmdFrame);
		/* TODO: Check sanity */
IOCTL_ATE_ERROR:
		os_free_mem(HqaCmdFrame);
	}
	break;
#endif /* CONFIG_WLAN_SERVICE */
#endif /* CONFIG_QA */
#endif /* CONFIG_ATE */

	case CMD_RTPRIV_IOCTL_MAC_ADDR_GET: {
		UCHAR mac_addr[MAC_ADDR_LEN];
		USHORT Addr01 = 0, Addr23 = 0, Addr45 = 0;

		RT28xx_EEPROM_READ16(pAd, 0x04, Addr01);
		RT28xx_EEPROM_READ16(pAd, 0x06, Addr23);
		RT28xx_EEPROM_READ16(pAd, 0x08, Addr45);
		mac_addr[0] = (UCHAR)(Addr01 & 0xff);
		mac_addr[1] = (UCHAR)(Addr01 >> 8);
		mac_addr[2] = (UCHAR)(Addr23 & 0xff);
		mac_addr[3] = (UCHAR)(Addr23 >> 8);
		mac_addr[4] = (UCHAR)(Addr45 & 0xff);
		mac_addr[5] = (UCHAR)(Addr45 >> 8);

		for (i = 0; i < 6; i++)
			*(UCHAR *)(pData + i) = mac_addr[i];

		break;
	}

#ifdef CONFIG_AP_SUPPORT

	case CMD_RTPRIV_IOCTL_SIOCGIWRATE:
		/* handle for SIOCGIWRATEQ */
	{
		RT_CMD_IOCTL_RATE *pRate = (RT_CMD_IOCTL_RATE *)pData;
		HTTRANSMIT_SETTING HtPhyMode;
#ifdef DOT11_HE_AX
		HE_TRANSMIT_SETTING HePhyMode;
#endif
		UINT8 BW;
		UINT8 Antenna = 0;
		USHORT MCS;
		struct wifi_dev	*wdev = NULL;
#ifdef APCLI_SUPPORT
		MAC_TABLE_ENTRY	*pEntry = NULL;
#endif /* APCLI_SUPPORT */
		UINT ifIndex = pObj->ioctl_if;

#ifdef APCLI_SUPPORT
		if (pRate->priv_flags == INT_APCLI) {
			if (ifIndex < pAd->MSTANum) {
				pEntry = MacTableLookup2(pAd, pAd->StaCfg[ifIndex].wdev.bssid,
							&pAd->StaCfg[ifIndex].wdev);

				if (!pEntry) {/* show maximum capability */
					HtPhyMode = pAd->StaCfg[ifIndex].wdev.HTPhyMode;
				} else {
					HtPhyMode = pEntry->HTPhyMode;

				if (HtPhyMode.field.MODE == MODE_VHT)
					Antenna = (HtPhyMode.field.MCS >> 4) + 1;
				}

				wdev = &pAd->StaCfg[ifIndex].wdev;
			}
			else {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s() invalid sta entry id(%d)\n", __func__, ifIndex));
				return NDIS_STATUS_FAILURE;
			}

		} else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
		if (pRate->priv_flags == INT_WDS) {
			if (ifIndex < MAX_WDS_ENTRY) {
				HtPhyMode = pAd->WdsTab.WdsEntry[ifIndex].wdev.HTPhyMode;
				wdev = &pAd->WdsTab.WdsEntry[ifIndex].wdev;
			}
			else {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s() invalid wds entry id(%d)\n", __func__, ifIndex));
				return NDIS_STATUS_FAILURE;
			}
		} else
#endif /* WDS_SUPPORT */
		{
			if (VALID_MBSS(pAd, ifIndex)) {
				HtPhyMode = pAd->ApCfg.MBSSID[ifIndex].wdev.HTPhyMode;
				wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			}
			else {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s() invalid mbss entry id(%d)\n", __func__, ifIndex));
				return NDIS_STATUS_FAILURE;
			}
		}
		if (!wdev->DevInfo.WdevActive)
			break;
		MCS = HtPhyMode.field.MCS;
#ifdef DOT11_N_SUPPORT
		if ((HtPhyMode.field.MODE == MODE_HTMIX)
		    || (HtPhyMode.field.MODE == MODE_HTGREENFIELD)) {
			Antenna = (HtPhyMode.field.MCS >> 3) + 1;
			MCS = MCS & 0xffff;
		}

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
		if (HtPhyMode.field.MODE >= MODE_VHT)
			MCS = MCS & 0xf;

		if (HtPhyMode.field.MODE >= MODE_VHT) {
			BW = wlan_operate_get_vht_bw(wdev);
			if (wlan_operate_get_ht_bw(wdev)) {
				if (BW == 0) /* VHT40 */
					BW = 1;
				else if (BW == 1) /* VHT80 */
					BW = 2;
				else if (BW >= 2) /* VHT80-80,VHT160 */
					BW = 3;
			}
		} else
#endif /*DOT11_VHT_AC*/
			if (HtPhyMode.field.MODE >= MODE_HTMIX)
				BW = wlan_operate_get_ht_bw(wdev);
			else
				BW = HtPhyMode.field.BW;
		if (Antenna == 0)
			Antenna = wlan_config_get_tx_stream(wdev);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HtPhyMode.field.MODE=%d\n\r", HtPhyMode.field.MODE));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HtPhyMode.field.ShortGI=%d\n\r", HtPhyMode.field.ShortGI));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HtPhyMode.field.BW=%d\n\r", HtPhyMode.field.BW));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HtPhyMode.field.MCS=%d\n\r", MCS));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BW=%d\n\r", BW));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Antenna=%d\n\r", Antenna));

#ifdef DOT11_HE_AX
		if (WMODE_CAP_AX(wdev->PhyMode)) {
			if (VALID_MBSS(pAd, ifIndex))
				HePhyMode = pAd->ApCfg.MBSSID[ifIndex].wdev.HEPhyMode;
			else {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s() invalid mbss id(%d)\n", __func__, ifIndex));
				return NDIS_STATUS_FAILURE;
			}
			Antenna = wlan_config_get_he_tx_nss(wdev);
			BW =  wlan_config_get_he_bw(wdev);
			MCS = HePhyMode.field.MCS;

			RtmpDrvMaxRateGet(pAd, HePhyMode.field.MODE, HePhyMode.field.ShortGI,
				  BW, MCS,
				  Antenna,
				  (UINT32 *)&pRate->BitRate);
			break;
		}
#endif /*DOT11_HE_AX*/
		RtmpDrvMaxRateGet(pAd, HtPhyMode.field.MODE, HtPhyMode.field.ShortGI,
				  BW, MCS,
				  Antenna,
				  (UINT32 *)&pRate->BitRate);
	}
	break;
#endif /* CONFIG_AP_SUPPORT */

	case CMD_RTPRIV_IOCTL_SIOCGIWNAME:
		RtmpIoctl_rt_ioctl_giwname(pAd, pData, 0);
		break;
#ifdef CONFIG_CSO_SUPPORT

	case CMD_RTPRIV_IOCTL_ADAPTER_CSO_SUPPORT_TEST:
		*(UCHAR *)pData = (pAd->MoreFlags & fASIC_CAP_CSO) ? 1 : 0;
		break;
#endif /* CONFIG_CSO_SUPPORT */
#ifdef PROFILE_PATH_DYNAMIC

	case CMD_RTPRIV_IOCTL_PROFILEPATH_SET:
		pAd->profilePath = (CHAR *)Data;
		break;
#endif /* PROFILE_PATH_DYNAMIC */
	}

#ifdef RT_CFG80211_SUPPORT

	if ((cmd >= CMD_RTPRIV_IOCTL_80211_START) &&
	    (cmd <= CMD_RTPRIV_IOCTL_80211_END))
		Status = CFG80211DRV_IoctlHandle(pAd, wrq, cmd, subcmd, pData, Data);

#endif /* RT_CFG80211_SUPPORT */

	if (cmd >= CMD_RTPRIV_IOCTL_80211_COM_LATEST_ONE)
		return NDIS_STATUS_FAILURE;

	return Status;
}

/*
	==========================================================================
	Description:
	Issue a site survey command to driver
	Arguments:
		pAdapter					Pointer to our adapter
		wrq						 Pointer to the ioctl argument

	Return Value:
	None

	Note:
	Usage:
		   1.) iwpriv ra0 set site_survey
	==========================================================================
*/
INT Set_SiteSurvey_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	NDIS_802_11_SSID Ssid;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT ifIndex = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	BOOLEAN ap_scan = TRUE; /* snowpin for ap/sta */

	/* check if the interface is down */
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("INFO::Network is down!\n"));
		return -ENETDOWN;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

		if (pObj->ioctl_if_type == INT_APCLI)
			wdev = &pAd->StaCfg[ifIndex].wdev;
		else
#endif
#ifdef CONFIG_STA_SUPPORT /* snowpin for ap/sta ++ */
			if (pObj->ioctl_if_type == INT_APCLI) {
				ap_scan = FALSE;
				if (ifIndex < MAX_APCLI_NUM)
					wdev = &pAd->StaCfg[ifIndex].wdev;
				else {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s() invalid apcli entry id(%d)\n", __func__, ifIndex));
					return -EINVAL;
				}
			} else
#endif /* CONFIG_STA_SUPPORT */ /* snowpin for ap/sta -- */
				if (pObj->ioctl_if_type == INT_MBSSID) {
					if (VALID_MBSS(pAd, ifIndex))
						wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
					else {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("%s() invalid mbss entry id(%d)\n", __func__, ifIndex));
						return -EINVAL;
					}
				}
				else
					wdev = &pAd->ApCfg.MBSSID[0].wdev;

		ASSERT(wdev);
		if (!wdev) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: wdev is null!\n", __func__));
			return -EINVAL;
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (MONITOR_ON(pAd)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!!! Driver is in Monitor Mode now !!!\n"));
			return -EINVAL;
		}
		ap_scan = FALSE; /* snowpin for ap/sta */
	}
#endif /* CONFIG_STA_SUPPORT // */
	NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
	if (ap_scan) /* snopwin for ap/sta */
	{
		if ((strlen(arg) > 0) && (strlen(arg) <= MAX_LEN_OF_SSID)) {
			NdisMoveMemory(Ssid.Ssid, arg, strlen(arg));
			Ssid.SsidLength = strlen(arg);
			ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, wdev);
		} else {
			Ssid.SsidLength = 0;
			ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_PASSIVE, FALSE, wdev);
		}

		return TRUE;
	}

#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT // */
#ifdef CONFIG_STA_SUPPORT
	/* snowpin for ap/sta IF_DEV_CONFIG_OPMODE_ON_STA(pAd) */
	if (!ap_scan) {
		Ssid.SsidLength = 0;

		if (ifIndex < pAd->MSTANum) {
			if ((arg != NULL) &&
			    (strlen(arg) <= MAX_LEN_OF_SSID)) {
				RTMPMoveMemory(Ssid.Ssid, arg, strlen(arg));
				Ssid.SsidLength = strlen(arg);
			}

			pAd->StaCfg[ifIndex].bSkipAutoScanConn = TRUE;
			wdev = &pAd->StaCfg[ifIndex].wdev;
			if (wdev == NULL) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s() wdev is NULL\n", __func__));
				return -EINVAL;
			}
			StaSiteSurvey(pAd, &Ssid, SCAN_ACTIVE, wdev);
		}
		else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s() invalid sta entry id(%d)\n", __func__, ifIndex));
			return -EINVAL;
		}
	}
#endif /* CONFIG_STA_SUPPORT // */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_SiteSurvey_Proc\n"));
	return TRUE;
}

#ifdef AP_SCAN_SUPPORT
/*
    ==========================================================================
    Description:
	Issue a Clear site survey command to driver
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 set ClearSiteSurvey=1
    ==========================================================================
*/
INT Set_ClearSiteSurvey_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 flag;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	   ifIndex;
	struct wifi_dev *wdev = NULL;
	SCAN_CTRL *ScanCtrl = NULL;

	flag = simple_strtol(arg, 0, 10);

	if (strlen(arg) > 1 || flag != 1) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Wrong argument type/Value\n"));
		return FALSE;
	}

	/* check if the interface is down */
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("INFO::Network is down!\n"));
		return -ENETDOWN;
	}

	/* Still scanning, Don't clear the scan Table */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Scan in Progress!\n", __func__));
		return -EINVAL;
	}

	ifIndex = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI) {
		if (ifIndex < MAX_APCLI_NUM) {
			wdev = &pAd->StaCfg[ifIndex].wdev;
			ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s() invalid apcli entry id(%d)\n", __func__, ifIndex));
			return -EINVAL;
		}
	} else
#endif/*APCLI_SUPPORT*/
		if (pObj->ioctl_if_type == INT_MBSSID) {
			if (VALID_MBSS(pAd, ifIndex)) {
				wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
				ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s() invalid mbss entry id(%d)\n", __func__, ifIndex));
				return -EINVAL;
			}
		}

	if (!ScanCtrl) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s() ScanCtrl==NULL!\n", __func__));
		return -EINVAL;
	}

	/* Don't clear the scan table if we are doing partial scan */
	if ((ScanCtrl->PartialScan.bScanning == TRUE && ScanCtrl->PartialScan.LastScanChannel == 0) ||
		ScanCtrl->PartialScan.bScanning == FALSE) {
			BssTableInit(&pAd->ScanTab);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Clear the Scan table\n"));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_ClearSiteSurvey_Proc\n"));
	return TRUE;
}
#endif /* AP_SCAN_SUPPORT */



INT	Set_Antenna_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ANT_DIVERSITY_TYPE UsedAnt;
	INT i;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==> Set_Antenna_Proc *******************\n"));

	for (i = 0; i < strlen(arg); i++)
		if (!isdigit(arg[i]))
			return -EINVAL;

	UsedAnt = os_str_tol(arg, 0, 10);

	switch (UsedAnt) {

		/* 2: Fix in the PHY Antenna CON1*/
	case ANT_FIX_ANT0:
		AsicSetRxAnt(pAd, 0);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<== Set_Antenna_Proc(Fix in Ant CON1), (%d,%d)\n",
				pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt));
		break;

		/* 3: Fix in the PHY Antenna CON2*/
	case ANT_FIX_ANT1:
		AsicSetRxAnt(pAd, 1);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<== %s(Fix in Ant CON2), (%d,%d)\n",
				__func__, pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt));
		break;

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("<== %s(N/A cmd: %d), (%d,%d)\n", __func__, UsedAnt,
				pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt));
		break;
	}

	return TRUE;
}

#ifdef HW_TX_RATE_LOOKUP_SUPPORT
INT Set_HwTxRateLookUp_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Enable;
	UINT32 MacReg;

	Enable = os_str_tol(arg, 0, 10);
	RTMP_IO_READ32(pAd->hdev_ctrl, TX_FBK_LIMIT, &MacReg);

	if (Enable) {
		MacReg |= 0x00040000;
		pAd->bUseHwTxLURate = TRUE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>UseHwTxLURate (ON)\n"));
	} else {
		MacReg &= (~0x00040000);
		pAd->bUseHwTxLURate = FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>UseHwTxLURate (OFF)\n"));
	}

	RTMP_IO_WRITE32(pAd->hdev_ctrl, TX_FBK_LIMIT, MacReg);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("UseHwTxLURate = %d\n", pAd->bUseHwTxLURate));
	return TRUE;
}
#endif /* HW_TX_RATE_LOOKUP_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
#ifdef MULTI_MAC_ADDR_EXT_SUPPORT
INT Set_EnMultiMacAddrExt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Enable = os_str_tol(arg, 0, 10);
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev;
	UCHAR band_idx = 0;

	switch (pObj->ioctl_if_type) {
	case INT_MAIN:
	case INT_MBSSID:
		wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
		band_idx = HcGetBandInfoByChannel(pAd, wdev->channel);
		break;

	case INT_APCLI:
		wdev = &pAd->StaCfg[pObj->ioctl_if].wdev;
		band_idx = HcGetBandInfoByChannel(pAd, wdev->channel);

		break;
	default:
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s)No Support This Type\n", __func__));
		return FALSE;

	}

	pAd->bUseMultiMacAddrExt = (Enable ? TRUE : FALSE);
	AsicSetReptFuncEnable(pAd, pAd->bUseMultiMacAddrExt, band_idx);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("UseMultiMacAddrExt = %d, UseMultiMacAddrExt(%s)\n",
			pAd->bUseMultiMacAddrExt, (Enable ? "ON" : "OFF")));
	return TRUE;
}

INT	Set_MultiMacAddrExt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR tempMAC[6], idx;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	ULONG offset, Addr;
	INT i;

	if (strlen(arg) <
	    19) /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.*/
		return FALSE;

	token = strchr(arg, DASH);

	if ((token != NULL) && (strlen(token) > 1)) {
		idx = (UCHAR) os_str_tol((token + 1), 0, 10);

		if (idx > 15)
			return FALSE;

		*token = '\0';

		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++) {
			if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
				return FALSE;

			AtoH(token, (&tempMAC[i]), 1);
		}

		if (i != 6)
			return FALSE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x\n",
				tempMAC[0], tempMAC[1], tempMAC[2], tempMAC[3], tempMAC[4], tempMAC[5], idx));
		offset = 0x1480 + (HW_WCID_ENTRY_SIZE * idx);
		Addr = tempMAC[0] + (tempMAC[1] << 8) + (tempMAC[2] << 16) + (tempMAC[3] << 24);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, offset, Addr);
		Addr = tempMAC[4] + (tempMAC[5] << 8);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, offset + 4, Addr);
		return TRUE;
	}

	return FALSE;
}
#endif /* MULTI_MAC_ADDR_EXT_SUPPORT */
#endif /* MAC_REPEATER_SUPPORT */

INT set_tssi_enable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 tssi_enable = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	tssi_enable = os_str_tol(arg, 0, 10);

	if (tssi_enable == 1) {
		cap->tssi_enable = TRUE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("turn on TSSI mechanism\n"));
	} else if (tssi_enable == 0) {
		cap->tssi_enable = FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("turn off TSS mechanism\n"));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("illegal param(%u)\n", tssi_enable));
		return FALSE;
	}

	return TRUE;
}

INT	Set_RadioOn_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR radio;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = NULL;
#endif

	if (!wdev) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: wdev is NULL\n", __func__));
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
		if (!pMbss) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: pMbss is NULL\n", __func__));
			return FALSE;
		}
	}
#endif

	radio = os_str_tol(arg, 0, 10);

	if (!wdev->if_up_down_state) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("==>Set_RadioOn_Proc (%s) but IF is done, ignore!!! (wdev_idx %d)\n",
			  radio ? "ON" : "OFF", wdev->wdev_idx));
		return TRUE;
	}

	if (radio == !IsHcRadioCurStatOffByChannel(pAd, wdev->channel)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("==>Set_RadioOn_Proc (%s) equal to current state, ignore!!! (wdev_idx %d)\n",
			  radio ? "ON" : "OFF", wdev->wdev_idx));
		return TRUE;
	}

	if (radio) {
		MlmeRadioOn(pAd, wdev);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_RadioOn_Proc (ON)\n"));
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
#ifdef FT_R1KH_KEEP
			pAd->ApCfg.FtTab.FT_RadioOff = FALSE;
#endif /* FT_R1KH_KEEP */
		}
#endif
		wdev->radio_off_req = FALSE;
	} else {
		wdev->radio_off_req = TRUE;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef FT_R1KH_KEEP
			pAd->ApCfg.FtTab.FT_RadioOff = TRUE;
#endif /* FT_R1KH_KEEP */
			APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
		}
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			MSTAStop(pAd, wdev);
		}
#endif
		if (!hwctrl_cmd_q_empty(pAd)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s(OFF),cmd q not empty!\n", __func__));
		}

		MlmeRadioOff(pAd, wdev);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_RadioOn_Proc (OFF)\n"));
	}

	return TRUE;
}

#ifdef OFFCHANNEL_SCAN_FEATURE
INT Set_ScanResults_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT32 ch_index = 0;

	ch_index = Channel2Index(pAd, pAd->ChannelInfo.ChannelNo, pAd->ChannelInfo.bandidx);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Channel : %d NF value : %ddb \tBusyTime : %dusec \tChannelBusyTime : %dusec\n",
				pAd->ChannelInfo.ChannelNo, pAd->ChannelInfo.AvgNF,
				pAd->ChannelInfo.chanbusytime[ch_index],
				pAd->ChannelInfo.FalseCCA[ch_index]));

	pAd->ChannelInfo.bandidx = 0;
	pAd->ChannelInfo.ChannelNo = 0;
	pAd->ChannelInfo.AvgNF = 0;
	pAd->ChannelInfo.chanbusytime[ch_index] = 0;
	pAd->ChannelInfo.FalseCCA[ch_index] = 0;
	return TRUE;
}

INT Set_ApScan_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UINT channel = 0;
	UINT timeout = 0;
	UINT i = 0, j = 0, count = 0;
	CHAR scantype[8] = {0};
	CHAR temp[33];
	UINT ifIndex;
	struct wifi_dev *wdev = NULL;
	SCAN_CTRL *ScanCtrl = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	/* check if the interface is down */
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("INFO::Network is down!\n"));
		return -ENETDOWN;
	}

	while (arg[j] != '\0') {
		temp[i] = arg[j++];
		if (temp[i] == ':') {
			switch (++count) {
			case 1:
				temp[i] = '\0';
				if ((strlen(temp) != 0) && (strlen(temp) <= 7)) {
					strncpy(scantype, temp, strlen(temp));
					if (strcmp(scantype, "active") && strcmp(scantype, "passive")) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("wrong scan type argument\n"));
						return FALSE;
					}
				} else {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("wrong scan type argument\n"));
					return FALSE;
				}
				i = 0;
				temp[i] = arg[j++];
				break;
			case 2:
				temp[i] = '\0';
				if ((strlen(temp) != 0) && (strlen(temp) <= 3)) {
					channel = simple_strtol(temp, 0, 10);
					if (!ChannelSanity(pAd, channel)) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("wrong channel number\n"));
						return FALSE;
					}
				} else {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("wrong channel number\n"));
					return FALSE;
				}
				i = 0;
				temp[i] = arg[j++];
				break;
			default:
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("wrong number of arguments\n"));
				return FALSE;
			}
		} else if (arg[j] == '\0') {
			temp[i+1] = '\0';
			if ((strlen(temp) != 0) && (strlen(temp) <= 10) && (simple_strtol(temp, 0, 10) < 0xffffffff)) {
				timeout = simple_strtol(temp, 0, 10);
			} else if (strlen(temp)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("wrong Timeout value\n"));
				return FALSE;
			}
		}
		i++;
	}

	ifIndex = pObj->ioctl_if;
	if (pObj->ioctl_if_type == INT_MBSSID) {
		if (VALID_MBSS(pAd, ifIndex)) {
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s() invalid mbss entry id(%d)\n", __func__, ifIndex));
			return FALSE;
		}
	}
	else
		wdev = &pAd->ApCfg.MBSSID[0].wdev;
	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
	/* Make compatible with application path */
	ScanCtrl->Num_Of_Channels = 1;
	ScanCtrl->ScanTime[0] = 0;
	ScanCtrl->CurrentGivenChan_Index = 0;
	ScanCtrl->state = OFFCHANNEL_SCAN_START;
	if (!strcmp(scantype, "passive"))
		ApSiteSurveyNew_by_wdev(pAd, channel, timeout, SCAN_PASSIVE, FALSE, wdev);
	else if (!strcmp(scantype, "active"))
		ApSiteSurveyNew_by_wdev(pAd, channel, timeout, SCAN_ACTIVE, FALSE, wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_ApScan_Proc\n"));
	return TRUE;
}
#endif
#ifdef NEW_SET_RX_STREAM
INT	Set_RxStream_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT RxStream;

	RxStream = os_str_tol(arg, 0, 10);
	AsicSetRxStream(pAd, RxStream, 0);
	return TRUE;
}
#endif

INT	Set_Lp_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR lp_enable;
	struct wifi_dev *wdev;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT ifIndex = pObj->ioctl_if;

#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMBSS = NULL;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s() invalid mbss id(%d)\n", __func__, ifIndex));
			return FALSE;
		}
		pMBSS = &pAd->ApCfg.MBSSID[ifIndex];
		wdev = &pMBSS->wdev;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (ifIndex < pAd->MSTANum)
			wdev = &pAd->StaCfg[ifIndex].wdev;
		else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s() invalid sta entry id(%d)\n", __func__, ifIndex));
			return FALSE;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	lp_enable = os_str_tol(arg, 0, 10);

	if (lp_enable) {
		MlmeLpEnter(pAd);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_Lp_Proc (Enetr)\n"));
	} else {
		MlmeLpExit(pAd);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			APStartUp(pAd, pMBSS, AP_BSS_OPER_BY_RF);
		}
#endif /* CONFIG_AP_SUPPORT */
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>Set_Lp_Proc (Exit)\n"));
	}

	return TRUE;
}

#ifdef MT_MAC
INT setTmrVerProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	CHAR ver;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	ver = (CHAR)os_str_tol(arg, 0, 10);

	if ((ver < TMR_VER_1_0) || (ver > TMR_VER_2_0)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: wrong setting %d, remain default %d!!\n",
			  __func__, ver, pChipCap->TmrHwVer));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%s: ver = %d, pChipCap->TmrHwVer = %d\n",
		  __func__, ver, pChipCap->TmrHwVer));

	return TRUE;
}

INT setTmrEnableProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	LONG enable;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	enable = os_str_tol(arg, 0, 10);

	if ((enable < TMR_DISABLE) || (enable > TMR_RESPONDER)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: enable is incorrect!!\n", __func__));
		return FALSE;
	}

	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: chipcap is not HIF_MT\n", __func__));
		return FALSE;
	}

	TmrCtrl(pAd, (UCHAR)enable, cap->TmrHwVer);
	return TRUE;
}

#ifndef COMPOS_TESTMODE_WIN
INT SetTmrCalProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	struct os_cookie *obj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, obj->ioctl_if, obj->ioctl_if_type);
	UCHAR TmrType = os_str_tol(arg, 0, 10);
	UCHAR Channel = HcGetRadioChannel(pAd);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%s(): TMR Calibration, TmrType: %d\n", __func__, TmrType));

	AsicSetTmrCal(pAd, TmrType, Channel, wlan_operate_get_bw(wdev));
	return TRUE;
}
#endif




INT set_cr4_query(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 option = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 (":%s: arg = %s\n", __func__, arg));

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	option = os_str_toul(arg, 0, 16);
	MtCmdCr4Query(pAd, option, 0, 0);
	return TRUE;
}

INT set_cr4_set(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 arg0 = 0;
	UINT32 arg1 = 0;
	UINT32 arg2 = 0;
	RTMP_STRING *arg0_ptr = NULL;
	RTMP_STRING *arg1_ptr = NULL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 (":%s: arg = %s\n", __func__, arg));
	arg0_ptr = strsep(&arg, ":");
	arg1_ptr = strsep(&arg, ":");

	if (arg0_ptr == NULL || arg1_ptr == NULL || arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	arg0 = os_str_toul(arg0_ptr, 0, 16);
	arg1 = os_str_toul(arg1_ptr, 0, 16);
	arg2 = os_str_toul(arg, 0, 16);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%s: arg0 = 0x%x, arg1 = 0x%x, arg2 = 0x%x\n",
		  __func__, arg0, arg1, arg2));
	MtCmdCr4Set(pAd, arg0, arg1, arg2);
	return TRUE;
}

INT set_cr4_capability(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 option = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 (":%s: arg = %s\n", __func__, arg));

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	option = os_str_toul(arg, 0, 16);
	MtCmdCr4Capability(pAd, option);
	return TRUE;
}

INT set_cr4_debug(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 option = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 (":%s: arg = %s\n", __func__, arg));

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	option = os_str_toul(arg, 0, 16);
	MtCmdCr4Debug(pAd, option);
	return TRUE;
}


INT set_re_calibration(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 BandIdx = 0;
	UINT32 CalItem = 0;
	UINT32 CalItemIdx = 0;
	RTMP_STRING *pBandIdx  = NULL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 (":%s: arg = %s\n", __func__, arg));
	pBandIdx = strsep(&arg, ":");

	if (pBandIdx == NULL || arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	BandIdx = os_str_toul(pBandIdx, 0, 10);
	CalItem = os_str_toul(arg, 0, 10);

	if (BandIdx > 1) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: Unknown BandIdx = %d\n", __func__, BandIdx));
		return FALSE;
	}

	if ((CalItem > 12) || (CalItem == 3) || (CalItem == 4)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: Unknown CalItem = %d\n", __func__, CalItem));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 (":%s: BandIdx: %d, CalItem: %d\n", __func__, BandIdx, CalItem));

	switch (CalItem) {
	case 0:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: RC_CAL\n", __func__));
		CalItemIdx = RC_CAL;
		break;

	case 1:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: RX_RSSI_DCOC_CAL\n", __func__));
		CalItemIdx = RX_RSSI_DCOC_CAL;
		break;

	case 2:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: RX_DCOC_CAL\n", __func__));
		CalItemIdx = RX_DCOC_CAL;
		break;

	case 5:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: RX_FIIQ_CAL\n", __func__));
		CalItemIdx = RX_FIIQ_CAL;
		break;

	case 6:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: RX_FDIQ_CAL\n", __func__));
		CalItemIdx = RX_FDIQ_CAL;
		break;

	case 7:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: TX_DPD_LINK\n", __func__));
		CalItemIdx = TX_DPD_LINK;
		break;

	case 8:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: TX_LPFG\n", __func__));
		CalItemIdx = TX_LPFG;
		break;

	case 9:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: TX_DCIQC\n", __func__));
		CalItemIdx = TX_DCIQC;
		break;

	case 10:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: TX_IQM\n", __func__));
		CalItemIdx = TX_IQM;
		break;

	case 11:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: TX_PGA\n", __func__));
		CalItemIdx = TX_PGA;
		break;

	case 12:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: CAL_ALL\n", __func__));
		CalItemIdx = CAL_ALL;
		break;

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: Unknown CalItem = %d\n", __func__, CalItem));
		break;
	}

	MtCmdDoCalibration(pAd, RE_CALIBRATION, CalItemIdx, BandIdx);
	return TRUE;
}

/*
 * iwpriv ra0 set ThermalMode=mode:action
 * mode   = 1: Thermal re-cal
 *        = 2: Dynamic G0
 *        = 3: Apply High Rate DPD table (Note: This mode only disable/enable)
 * action = 0: Disable
 *        = 1: Enable
 *        = 2: Trigger
 */
INT set_thermal_dbg_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#define THERMAL_MODE_RECAL      1
#define THERMAL_MODE_DYNAMIC_G0 2
#define THERMAL_MODE_HRATE_DPD  3
#define THERMAL_MODE_NT_LT      4

#define THERMAL_ACTION_DISABLE  0
#define THERMAL_ACTION_ENABLE   1
#define THERMAL_ACTION_TRIGGER  2
#define THERMAL_ACTION_NT2LT_TRIGGER 2
#define THERMAL_ACTION_LT2NT_TRIGGER 3

	UINT8 mode = 0;
	UINT8 action = 0;
	RTMP_STRING *cmd_str  = NULL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (":%s: arg = %s\n", __FUNCTION__, arg));

	cmd_str = strsep(&arg, ":");

	if (cmd_str == NULL || arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	mode = os_str_toul(cmd_str, 0, 10);
	action = os_str_toul(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		(":%s: mode(%d), action(%d))\n", __FUNCTION__, mode, action));

	switch (mode) {
	case THERMAL_MODE_RECAL:
	{
		if (action > THERMAL_ACTION_TRIGGER) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(":%s: Unknown action = %d (0: thermal recal OFF; 1: thermal recal ON; 2: trigger thermal recal)\n",
				__FUNCTION__, action));

			return FALSE;
		}

		if ((action == THERMAL_ACTION_TRIGGER) && (pAd->CommonCfg.ThermalRecalMode == THERMAL_ACTION_DISABLE)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(":%s: Can't trigger recal in Thermal recal off mode\n", __FUNCTION__));

			return FALSE;
		}

		pAd->CommonCfg.ThermalRecalMode = action;
		MtCmdThermalMode(pAd, mode, action);
	}
	break;

	case THERMAL_MODE_DYNAMIC_G0:
	{
		if (action > THERMAL_ACTION_TRIGGER) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(":%s: Unknown action = %d (0: Dynamic G0 OFF; 1: Dynamic G0 ON; 2: trigger Dynamic G0)\n",
				__FUNCTION__, action));

			return FALSE;
		}

		if ((action == THERMAL_ACTION_TRIGGER) && (pAd->CommonCfg.ThermalDynamicG0Action == THERMAL_ACTION_DISABLE)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(":%s: Can't trigger G0 in Dynamic G0 off mode\n", __FUNCTION__));

			return FALSE;
		}

		pAd->CommonCfg.ThermalDynamicG0Action = action;
		MtCmdThermalMode(pAd, mode, action);
	}
	break;

	case THERMAL_MODE_HRATE_DPD:
	{
		if (action > THERMAL_ACTION_ENABLE) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(":%s: Unknown action = %d (0: H-rate DPD OFF; 1: H-rate DPD ON;)\n",
				__FUNCTION__, action));

			return FALSE;
		}

		pAd->CommonCfg.ThermalHRateDpdMode = action;
		MtCmdThermalMode(pAd, mode, action);
	}
	break;

	case THERMAL_MODE_NT_LT:
	{
		if (action > THERMAL_ACTION_LT2NT_TRIGGER) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(":%s: Unknown action = %d (NTLT 0:OFF; 1:ON; 2:trigger NT2LT; 3:trigger LT2NT)\n",
				__FUNCTION__, action));

			return FALSE;
		}

		if ((pAd->CommonCfg.ThermalNtLtAction == THERMAL_ACTION_DISABLE) &&
			 ((action == THERMAL_ACTION_NT2LT_TRIGGER) ||
			  (action == THERMAL_ACTION_LT2NT_TRIGGER))) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				(":%s: Can't trigger NT2LT/LT2NT in NTLT off mode\n", __FUNCTION__));

			return FALSE;
		}

		pAd->CommonCfg.ThermalNtLtAction = action;
		MtCmdThermalMode(pAd, mode, action);
	}
	break;


	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: Unknown mode(%d) (1: Thermal re-cal; 2: Dynamic G0) )\n", __FUNCTION__, mode));
		break;
	}

#undef THERMAL_ACTION_DISABLE
#undef THERMAL_ACTION_ENABLE
#undef THERMAL_ACTION_TRIGGER
#undef THERMAL_ACTION_NT2LT_TRIGGER
#undef THERMAL_ACTION_LT2NT_TRIGGER

#undef THERMAL_MODE_DYNAMIC_G0
#undef THERMAL_MODE_RECAL
#undef THERMAL_MODE_HRATE_DPD
#undef THERMAL_MODE_NT_LT


	return TRUE;
}


INT set_fw_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 McuDest = 0;
	UINT32 LogType = 0;
	BOOLEAN invalid = FALSE;
	UINT8 bitmap;
	RTMP_STRING *pMcuDest  = NULL;
	RTMP_STRING *Dest[]  = {
		"HOST2N9(WM)",
		"HOST2CR4(WA)",
		"HOST2WO"
	};
	RTMP_STRING *info[] = {
		"Print MCU Log to UART",
		"Send MCU log by Event",
#ifdef FW_LOG_DUMP
		"Send MCU log to EMI(currently not support)",
		"Send MCU log by Event to Storage",
		"Send MCU log by Event to Ethernet"
#endif
	};

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		(":%s: arg = %s\n", __func__, arg));
	pMcuDest = strsep(&arg, ":");

	if (pMcuDest == NULL || arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	McuDest = os_str_toul(pMcuDest, 0, 10);
	LogType = os_str_toul(arg, 0, 10);

	if (McuDest < 3 && McuDest >= 0) {
		if ((LogType & ~SUPPORTED_FW_LOG_TYPE) != 0)
			invalid = TRUE;
#ifdef FW_LOG_DUMP
		else
			pAd->fw_log_ctrl.wmcpu_log_type = LogType;
#endif /* FW_LOG_DUMP */
	} else if ((McuDest > 2) || ((McuDest < 0)))
		invalid = TRUE;

	if (invalid) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: Unknown Mcu Dest = %d, Log Type = %x\n",
			  __func__, McuDest, LogType));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		(":%s: Mcu Dest = %s \n", __func__, Dest[McuDest]));

	if (LogType == 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("Log Type = Disable MCU Log Message\n"));
	} else {
		for (bitmap = 0; bitmap < FW_LOG_TYPE_COUNT; bitmap++)
			if (LogType & (1 << bitmap))
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Log Type = %s\n", info[bitmap]));
	}
	MtCmdFwLog2Host(pAd, McuDest, LogType);
	return TRUE;
}

INT set_fw_dbg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_STRING *dbg_lvl_str = NULL;
	UINT8 dbg_lvl;
	UINT32 dbg_module_idx;

	if (arg == NULL || strlen(arg) == 0)
		goto error;

	dbg_lvl_str = strsep(&arg, ":");
	dbg_lvl = os_str_toul(dbg_lvl_str, 0, 10);

	if (arg == NULL || strlen(arg) == 0) {
		/* imply all modules */
		dbg_module_idx = 0xffffffff;
	} else {
		dbg_module_idx = os_str_toul(arg, 0, 10);
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%s: set fw debug level to 0x%x for module index 0x%x\n", __func__, dbg_lvl, dbg_module_idx));

	MtCmdFwDbgCtrl(pAd, dbg_lvl, dbg_module_idx);
	return TRUE;

error:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Usage: fw_dbg=[debug level bitmap]:[debug module index]\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tdebug level bitmap:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t DBG_CLASS_ERROR (1 << 0)\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t DBG_CLASS_WARN  (1 << 1)\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t DBG_CLASS_STATE (1 << 2)\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t DBG_CLASS_INFO  (1 << 3)\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t DBG_CLASS_LOUD  (1 << 4)\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tdebug module index:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\t If not specified, means all modules.\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Example:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t iwpriv ra0 set fw_dbg=3 (turn on ERROR and WARN for all modules\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t iwpriv ra0 set fw_dbg=1:0 (turn on ERROR for module 0\n"));

	return FALSE;
}

INT set_isr_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#define ISR_CMD_CMD_ID_OFFSET 24
#define ISR_CMD_SUBCMD_ID_OFFSET 0
#define ISR_CMD_CMD_ID_MASK 0x7f
#define ISR_CMD_SUBCMD_ID_MASK 0xffffff
	UINT32 cmd = 0;
	UINT32 sub_cmd = 0;
	RTMP_STRING *cmd_str  = NULL;
	UINT32 tmp = 0;
	UINT32 tmp2;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 (":%s: arg = %s\n", __func__, arg));
	cmd_str = strsep(&arg, ":");

	if (cmd_str == NULL || arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	cmd = os_str_toul(cmd_str, 0, 10);
	sub_cmd = os_str_toul(arg, 0, 10);

	cmd &= ISR_CMD_CMD_ID_MASK;
	sub_cmd &= ISR_CMD_SUBCMD_ID_MASK;

	HW_IO_READ32(pAd->hdev_ctrl, PLE_TO_N9_INT, &tmp);
	tmp2 = (tmp ^ PLE_TO_N9_INT_TOGGLE_MASK) & PLE_TO_N9_INT_TOGGLE_MASK;

	tmp2 |= (cmd << ISR_CMD_CMD_ID_OFFSET);
	tmp2 |= (sub_cmd << ISR_CMD_SUBCMD_ID_OFFSET);

	HW_IO_WRITE32(pAd->hdev_ctrl, PLE_TO_N9_INT, tmp2);

	return TRUE;
}

UINT16 txop0;
UINT16 txop60 = 0x60;
UINT16 txop80 = 0x80;
UINT16 txopfe = 0xfe;
INT set_txop_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_STRING *str  = NULL;
	RTMP_STRING *str2  = NULL;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 (":%s: current setting txop 0=%x, txop 60=%x, txop 80=%x, txop fe=%x\n",
		  __func__, txop0, txop60, txop80, txopfe));

	str = strsep(&arg, ":");
	if (str == NULL || arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (":%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	do {
		str2 = strsep(&arg, ":");
		if (str2 == NULL)
			str2 = arg;

		switch (os_str_toul(str, 0, 16)) {
		case 0x0:
			txop0 = os_str_toul(str2, 0, 16);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: txop0 change to %x\n", __func__, txop0));
			break;
		case 0x60:
			txop60 = os_str_toul(str2, 0, 16);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: txop60 change to %x\n", __func__, txop60));
			break;
		case 0x80:
			txop80 = os_str_toul(str2, 0, 16);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: txop80 change to %x\n", __func__, txop80));
			break;
		case 0xfe:
			txopfe = os_str_toul(str2, 0, 16);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: txop60 change to %x\n", __func__, txopfe));
			break;
		default:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 (":%s: not support txop%lx\n", __func__, os_str_toul(str, 0, 16)));
			break;
		}

		str = strsep(&arg, ":");
	} while (str != NULL);

	return TRUE;
}

INT set_rts_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#define RTS_NUM_DIS_VALUE 0xff
#define RTS_LEN_DIS_VALUE 0xffffff
#define RTS_NUM_EN_VALUE 0x4
#define RTS_LEN_EN_VALUE 0x92b

	UINT32 rts_mode;
	RTMP_STRING *str  = NULL;
	UCHAR bandidx = 0;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	if (arg == NULL || strlen(arg) == 0)
		goto invalidparameter;

	str = strsep(&arg, ":");

	if (str == NULL || arg == NULL)
		goto invalidparameter;

	bandidx = os_str_toul(str, 0, 10);

	if (bandidx > 1)
		goto invalidparameter;

	rts_mode = os_str_toul(arg, 0, 10);

	if (rts_mode == 0) {
		asic_rts_on_off_detail(pAd, bandidx, RTS_NUM_EN_VALUE, RTS_LEN_EN_VALUE, FALSE);
		pAd->mcli_ctl[bandidx].c2s_only = FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("enable rts\n"));
	} else if (rts_mode == 1) {
		asic_rts_on_off_detail(pAd, bandidx, RTS_NUM_DIS_VALUE, RTS_LEN_DIS_VALUE, FALSE);
		pAd->mcli_ctl[bandidx].c2s_only = TRUE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("disable rts\n"));
	} else
		goto invalidparameter;

	return TRUE;

invalidparameter:
	if (bandidx < DBDC_BAND_NUM) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("rts is %s, asic_rts_on_off = %p\n",
		 (pAd->mcli_ctl[bandidx].c2s_only) ? "off" : "on", arch_ops->asic_rts_on_off));
	}
	else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("invalid bandidx = %d, asic_rts_on_off = %p\n",
		 bandidx, arch_ops->asic_rts_on_off));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("format: [bandidx]:[mode]\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("[bandidx]: 0/1\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("[mode]: 0 = enable rts , 1 = disable rts,"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("2 = disable dynamic rts on/off alg, 3 = enable dynamic rts on/off alg\n"));

	return TRUE;
}

/* SER user command */
#define SER_USER_CMD_DISABLE		 0
#define SER_USER_CMD_ENABLE		  1

#define SER_USER_CMD_ENABLE_MASK_TRACKING_ONLY	  (200)
#define SER_USER_CMD_ENABLE_MASK_L1_RECOVER_ONLY	(201)
#define SER_USER_CMD_ENABLE_MASK_L2_RECOVER_ONLY	(202)
#define SER_USER_CMD_ENABLE_MASK_L3_RX_ABORT_ONLY   (203)
#define SER_USER_CMD_ENABLE_MASK_L3_TX_ABORT_ONLY   (204)
#define SER_USER_CMD_ENABLE_MASK_L3_TX_DISABLE_ONLY (205)
#define SER_USER_CMD_ENABLE_MASK_L3_BFRECOVER_ONLY  (206)
#define SER_USER_CMD_ENABLE_MASK_RECOVER_ALL		(207)

/* Use a magic number to prevent human mistake */
#define SER_USER_CMD_L1_RECOVER		  995

#define SER_USER_CMD_L2_BN0_RECOVER	  (300)
#define SER_USER_CMD_L2_BN1_RECOVER	  (301)
#define SER_USER_CMD_L3_RX0_ABORT		(302)
#define SER_USER_CMD_L3_RX1_ABORT		(303)
#define SER_USER_CMD_L3_TX0_ABORT		(304)
#define SER_USER_CMD_L3_TX1_ABORT		(305)
#define SER_USER_CMD_L3_TX0_DISABLE	  (306)
#define SER_USER_CMD_L3_TX1_DISABLE	  (307)
#define SER_USER_CMD_L3_BF_RECOVER	   (308)


#ifdef RTMP_PCI_SUPPORT
INT set_rxd_debug(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 hw_rxd = 0;
	UINT32 sw_addr = 0;
	UINT32 sw_payload = 0;
	UINT32 hw_payload = 0;
	UINT32 idx;
	ULONG flags;
	INT32 ret = 0;
	UINT8 num_of_rx_ring = hif_get_rx_res_num(pAd->hdev_ctrl);
	PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = NULL;
	NDIS_SPIN_LOCK *lock;

	if (arg == NULL)
		return TRUE;

	ret = sscanf(arg, "%d-%d-%d-%d", &hw_rxd, &sw_addr,
								&sw_payload, &hw_payload);

	if (ret != 4) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Format: iwpriv ra0 set rxd_debug=[hw_rxd]-[sw_addr]-[sw_payload]-[hw_payload]\n"));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("hw_rxd=%d, sw_addr=%d, sw_payload=%d, hw_payload=%d\n",
				  hw_rxd, sw_addr, sw_payload, hw_payload));
	}

	for (idx = 0; idx < num_of_rx_ring; idx++) {
			rx_ring = pci_get_rx_ring_by_ridx(hif, idx);
			lock = &rx_ring->ring_lock;
			RTMP_IRQ_LOCK(lock, flags);
			rx_ring->buf_debug = ((hw_rxd << 0) | (sw_addr << 1)
									|(sw_payload << 2) | (hw_payload << 3));
			RTMP_IRQ_UNLOCK(lock, flags);
	}

	return TRUE;
}
#endif

INT set_ser(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 cmdId = 0;

	if (arg == NULL)
		goto ser_usage;

	cmdId = os_str_toul(arg, 0, 10);

	switch (cmdId) {
	case SER_USER_CMD_DISABLE:
		CmdExtSER(pAd, SER_ACTION_SET, SER_SET_DISABLE, 0);
		break;

	case SER_USER_CMD_ENABLE:
		CmdExtSER(pAd, SER_ACTION_SET, SER_SET_ENABLE, 0);
		break;

	case SER_USER_CMD_ENABLE_MASK_TRACKING_ONLY:
		CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING, 0);
		break;

	case SER_USER_CMD_ENABLE_MASK_L1_RECOVER_ONLY:
		CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L1_RECOVER, 0);
		break;

	case SER_USER_CMD_ENABLE_MASK_L2_RECOVER_ONLY:
		CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L2_RECOVER, 0);
		break;

	case SER_USER_CMD_ENABLE_MASK_L3_RX_ABORT_ONLY:
		CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_RX_ABORT, 0);
		break;

	case SER_USER_CMD_ENABLE_MASK_L3_TX_ABORT_ONLY:
		CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_TX_ABORT, 0);
		break;

	case SER_USER_CMD_ENABLE_MASK_L3_TX_DISABLE_ONLY:
		CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_TX_DISABLE, 0);
		break;

	case SER_USER_CMD_ENABLE_MASK_L3_BFRECOVER_ONLY:
		CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_BF_RECOVER, 0);
		break;

	case SER_USER_CMD_ENABLE_MASK_RECOVER_ALL:
		CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK,
			  (SER_ENABLE_TRACKING |
			   SER_ENABLE_L1_RECOVER | SER_ENABLE_L2_RECOVER |
			   SER_ENABLE_L3_RX_ABORT | SER_ENABLE_L3_TX_ABORT |
			   SER_ENABLE_L3_TX_DISABLE | SER_ENABLE_L3_BF_RECOVER), 0);
		break;

	case SER_USER_CMD_L1_RECOVER:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L1_RECOVER, 0);
		break;

	case SER_USER_CMD_L2_BN0_RECOVER:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L2_RECOVER, DBDC_BAND0);
		break;

	case SER_USER_CMD_L2_BN1_RECOVER:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L2_RECOVER, DBDC_BAND1);
		break;

	case SER_USER_CMD_L3_RX0_ABORT:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_RX_ABORT, DBDC_BAND0);
		break;

	case SER_USER_CMD_L3_RX1_ABORT:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_RX_ABORT, DBDC_BAND1);
		break;

	case SER_USER_CMD_L3_TX0_ABORT:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_TX_ABORT, DBDC_BAND0);
		break;

	case SER_USER_CMD_L3_TX1_ABORT:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_TX_ABORT, DBDC_BAND1);
		break;

	case SER_USER_CMD_L3_TX0_DISABLE:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_TX_DISABLE, DBDC_BAND0);
		break;

	case SER_USER_CMD_L3_TX1_DISABLE:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_TX_DISABLE, DBDC_BAND1);
		break;

	case SER_USER_CMD_L3_BF_RECOVER:
		CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_BF_RECOVER, 0);
		break;

	default:
		goto ser_usage;
	}

	return TRUE;
ser_usage:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("iwpriv rax set ser=[command ID]\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("[command ID]\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Disable SER\n", SER_USER_CMD_DISABLE));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Enable SER\n", SER_USER_CMD_ENABLE));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Level 1 recover\n", SER_USER_CMD_L1_RECOVER));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Level 2 BN0 recover\n", SER_USER_CMD_L2_BN0_RECOVER));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Level 2 BN1 recover\n", SER_USER_CMD_L2_BN1_RECOVER));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Level 3 BN0 rx abort\n", SER_USER_CMD_L3_RX0_ABORT));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Level 3 BN1 rx abort\n", SER_USER_CMD_L3_RX1_ABORT));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Level 3 BN0 tx abort\n", SER_USER_CMD_L3_TX0_ABORT));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Level 3 BN1 tx abort\n", SER_USER_CMD_L3_TX1_ABORT));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Level 3 BN0 tx disable\n", SER_USER_CMD_L3_TX0_DISABLE));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Level 3 BN1 tx disable\n", SER_USER_CMD_L3_TX1_DISABLE));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Level 3 BF recover\n", SER_USER_CMD_L3_BF_RECOVER));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : HW ERROR Tracking only, no recover\n", SER_USER_CMD_ENABLE_MASK_TRACKING_ONLY));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Set L1 recover only\n", SER_USER_CMD_ENABLE_MASK_L1_RECOVER_ONLY));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Set L2 recover only\n", SER_USER_CMD_ENABLE_MASK_L2_RECOVER_ONLY));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Set L3 rx abort only\n", SER_USER_CMD_ENABLE_MASK_L3_RX_ABORT_ONLY));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Set L3 tx abort only\n", SER_USER_CMD_ENABLE_MASK_L3_TX_ABORT_ONLY));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Set L3 tx disable only\n", SER_USER_CMD_ENABLE_MASK_L3_TX_DISABLE_ONLY));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Set L3 BF recover only\n", SER_USER_CMD_ENABLE_MASK_L3_BFRECOVER_ONLY));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%d : Set All recover enable\n", SER_USER_CMD_ENABLE_MASK_RECOVER_ALL));

	return TRUE;
}

INT32 set_fw_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	CHAR *value;
	UCHAR *Input;
	UCHAR ExtendID;
	INT i, len;
	BOOLEAN fgStatus = FALSE;
	/* get ExtID */
	value = rstrtok(Arg, ":");
	if (value == NULL)
		return fgStatus;
	AtoH(value, &ExtendID, 1);
	/* get cmd raw data */
	value += 3;
	len = strlen(value) >> 1;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=======Set_FwCmd==========\n"));
	os_alloc_mem(pAd, (UCHAR **)&Input, len);

	for (i = 0; i < len; i++)
		AtoH(value + i * 2, &Input[i], 1);

	/* print cmd raw data */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("EID= 0x%x, CMD[%d] = ", ExtendID, len));

	for (i = 0; i < len; i++)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0x%x ", Input[i]));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	/* send cmd to fw */
	MtCmdSendRaw(pAd, ExtendID, Input, len, CMD_SET);
	os_free_mem((PVOID)Input);
	return fgStatus;
}

INT32 get_fw_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	CHAR *value;
	UCHAR *Input;
	UCHAR ExtendID;
	INT i, len;
	BOOLEAN fgStatus = FALSE;
	/* get ExtID */
	value = rstrtok(Arg, ":");
	if (value == NULL)
		return fgStatus;
	AtoH(value, &ExtendID, 1);
	/* get cmd raw data */
	value += 3;
	len = strlen(value) >> 1;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=======Get_FwCmd==========\n"));
	os_alloc_mem(pAd, (UCHAR **)&Input, len);

	for (i = 0; i < len; i++)
		AtoH(value + i * 2, &Input[i], 1);

	/* print cmd raw data */
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("EID= 0x%x, CMD[%d] = ", ExtendID, len));

	for (i = 0; i < len; i++)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0x%x ", Input[i]));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	/* send cmd to fw */
	MtCmdSendRaw(pAd, ExtendID, Input, len, CMD_QUERY);
	os_free_mem((PVOID)Input);
	return fgStatus;
}

#ifdef FW_DUMP_SUPPORT
INT set_fwdump_max_size(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->fw_dump_max_size = os_str_tol(arg, 0, 10);
	return TRUE;
}

INT set_fwdump_path(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_OS_FWDUMP_SETPATH(pAd, arg);
	return TRUE;
}

INT fwdump_print(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 x;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: len = %d\n", __func__, pAd->fw_dump_size));

	for (x = 0; x < pAd->fw_dump_size; x++) {
		if (x % 16 == 0)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0x%04x : ", x));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x ", ((unsigned char)pAd->fw_dump_buffer[x])));

		if (x % 16 == 15)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	return TRUE;
}
#endif

#endif

INT set_thermal_protection_criteria_proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING		*arg)
{
	CHAR	 *value;
	UINT8	 ucParamIdx;
	BOOLEAN  fgHighEn = FALSE, fgLowEn = FALSE, fgRFOffEn = FALSE;
	CHAR	 cHighTempTh = 0, cLowTempTh = 0, cRFOffTh = 0;
	UINT32   u4RechkTimer = 0;
	UINT8    ucType = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand;

	/* sanity check for input parameter*/
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: No parameters!!\n", __func__));
		goto error;
	}

	/* Parsing input parameter */
	for (ucParamIdx = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), ucParamIdx++) {
		switch (ucParamIdx) {
		case 0:
			fgHighEn = simple_strtol(value, 0, 10); /* 1-bit format */
			break;

		case 1:
			fgLowEn = simple_strtol(value, 0, 10); /* 1-bit format */
			break;

		case 2:
			fgRFOffEn = simple_strtol(value, 0, 10); /* 1-bit format */
			break;

		case 3:
			cHighTempTh = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 4:
			cLowTempTh = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 5:
			cRFOffTh = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 6:
			u4RechkTimer = simple_strtol(value, 0, 10); /* 4-bit format */
			break;

		case 7:
			ucType = simple_strtol(value, 0, 10); /* 1-bit format */
			break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Invalid Parameter Format!!\n", __func__));
			break;
		}
	}

	if (ucParamIdx != 8) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Wrong parameter format!!\n", __func__));
		goto error;
	}

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wdev is invalid\n", __func__));
		return FALSE;
	}

	ucBand = HcGetBandByWdev(wdev);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%s: fgHighEn: %d, fgLowEn: %d, fgRFOffEn: %d, cHighTempTh: %d, cLowTempTh: %d, cRFOffTh: %d\n", __func__, fgHighEn, fgLowEn, fgRFOffEn, cHighTempTh, cLowTempTh, cRFOffTh));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%s: u4RechkTimer: %d, ucType: %s\n", __func__, u4RechkTimer, ucType ? "Duty Cycle" : "TxStream"));

#ifdef MT_MAC
	AsicThermalProtect(pAd, ucBand, fgHighEn, cHighTempTh, fgLowEn, cLowTempTh, u4RechkTimer, fgRFOffEn, cRFOffTh, ucType);
#endif /* MT_MAC */

	return TRUE;

error:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 (KYEL "iwpriv <interface> set tpc=fgHighEn:fgLowEn:fgRFOffEn:cHighTempTh:cLowTempTh:cRFOffTh:u4RechkTimer:ucType\n" KNRM));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("	fgHighEn:	(1-bit format) High Temperature Protect Trigger Enable\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("	fgLowEn:	 (1-bit format) Low Temperature Protect Trigger Enable\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("	fgRFOffEn:   (1-bit format) RF off Protect Trigger Enable\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("	cHighTempTh: (3-bit format) High Temperature Protect Trigger point\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("	cLowTempTh:  (3-bit format) Low Temperature Protect Trigger point\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("	cRFOffTh:	(3-bit format) RF off Protect Trigger point\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("	u4RechkTimer:(4-bit format) Thermal Protect Recheck period\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("	ucType:	  (1-bit format) Thermal Protect Type (0: TxStream, 1: Duty Cycle)\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 (KGRN "Ex: iwpriv ra0 set tpc=1:1:1:080:070:110:0060:1\n" KNRM));

	return FALSE;
}

INT set_thermal_protection_admin_ctrl_duty_proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING		*arg)
{
	CHAR   *value;
	UINT8  ucParamIdx;
	UINT32 u4Lv0Duty = 0, u4Lv1Duty = 0, u4Lv2Duty = 0, u4Lv3Duty = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand;

	/* sanity check for input parameter*/
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: No parameters!!\n", __func__));
		goto error0;
	}

	/* Parsing input parameter */
	for (ucParamIdx = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), ucParamIdx++) {
		switch (ucParamIdx) {
		case 0:
			u4Lv0Duty = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 1:
			u4Lv1Duty = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 2:
			u4Lv2Duty = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 3:
			u4Lv3Duty = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Invalid Parameter Format!!\n", __func__));
			break;
		}
	}

	if (ucParamIdx != 4) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Wrong parameter format!!\n", __func__));
		goto error0;
	}

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wdev is invalid\n", __func__));
		return FALSE;
	}

	ucBand = HcGetBandByWdev(wdev);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%s: ucBand:%d, u4Lv0Duty:%d, u4Lv1Duty:%d, u4Lv2Duty:%d, u4Lv3Duty:%d\n", __func__,
		  ucBand, u4Lv0Duty, u4Lv1Duty, u4Lv2Duty, u4Lv3Duty));

	/* Parameter sanity check */
	if (u4Lv0Duty > 100) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Parameters error! Lv0Duty > 100\n"));
		goto error1;
	}

	if (u4Lv1Duty > u4Lv0Duty) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Parameters error! Lv1Duty > Lv0Duty"));
		goto error1;
	}

	if (u4Lv2Duty > u4Lv1Duty) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Parameters error! Lv2Duty > Lv1Duty"));
		goto error1;
	}

	if (u4Lv3Duty > u4Lv2Duty) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Parameters error! Lv3Duty > Lv2Duty"));
		goto error1;
	}

	AsicThermalProtectAdmitDuty(pAd, ucBand, u4Lv0Duty, u4Lv1Duty, u4Lv2Duty, u4Lv3Duty);

	return TRUE;

error0:

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 (KYEL "iwpriv <interface> set tpc_duty=Lv0Duty:Lv1Duty:Lv2Duty:Lv3Duty\n" KNRM));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("	Lv0Duty: (3-bit format) Level 0 Protect Duty cycle\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("	Lv1Duty: (3-bit format) Level 1 Protect Duty cycle\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("	Lv2Duty: (3-bit format) Level 2 Protect Duty cycle\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("	Lv3Duty: (3-bit format) Level 3 Protect Duty cycle\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 (KGRN "Ex: iwpriv ra0 set tpc_duty=100:080:050:030\n" KNRM));

	return FALSE;

error1:
	return FALSE;
}


INT get_thermal_protection_admin_ctrl_duty_proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING		 *arg)
{
#if defined(MT7615) || defined(MT7622)
	return TRUE;
#else
	return AsicThermalProtectAdmitDutyInfo(pAd);
#endif /* defined(MT7615) || defined(MT7622) */
}

#ifdef CONFIG_DVT_MODE
#define STOP_DP_OUT	0x50029080
INT16 i2SetDvt(RTMP_ADAPTER *pAd, RTMP_STRING *pArg)
{
	/*
	test item=0: normal mode
	test item=1: test tx endpint, param1=
	*/
	INT16	i2ParameterNumber = 0;
	UCHAR	ucTestItem = 0;
	UCHAR	ucTestParam1 = 0;
	UCHAR	ucTestParam2 = 0;
	UINT32	u4Value;

	if (pArg) {
		i2ParameterNumber = sscanf(pArg, "%d,%d,%d", &(ucTestItem), &(ucTestParam1), &(ucTestParam2));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: i2ParameterNumber(%d), ucTestItem(%d), ucTestParam1(%d), ucTestParam2(%d)\n", __func__, i2ParameterNumber,
			  ucTestItem, ucTestParam1, ucTestParam2));
		pAd->rDvtCtrl.ucTestItem = ucTestItem;
		pAd->rDvtCtrl.ucTestParam1 = ucTestParam1;
		pAd->rDvtCtrl.ucTestParam2 = ucTestParam2;

		/* Tx Queue Mode*/
		if (pAd->rDvtCtrl.ucTestItem == 1) {
			pAd->rDvtCtrl.ucTxQMode = pAd->rDvtCtrl.ucTestItem;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucTxQMode(%d)\n", __func__, pAd->rDvtCtrl.ucTxQMode));
		} else if (pAd->rDvtCtrl.ucTestItem == 2) {
			pAd->rDvtCtrl.ucTxQMode = pAd->rDvtCtrl.ucTestItem;
			pAd->rDvtCtrl.ucQueIdx = pAd->rDvtCtrl.ucTestParam1;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucTxQMode(%d), ucQueIdx(%d)\n", __func__,
					pAd->rDvtCtrl.ucTxQMode, pAd->rDvtCtrl.ucQueIdx));
		}
		/* UDMA Drop CR Access */
		else if (pAd->rDvtCtrl.ucTestItem == 3 && pAd->rDvtCtrl.ucTestParam1 == 0) {
			RTMP_IO_READ32(pAd->hdev_ctrl, STOP_DP_OUT, &u4Value);
			u4Value &= ~(BIT25 | BIT24 | BIT23 | BIT22 | BIT21 | BIT20);
			RTMP_IO_WRITE32(pAd->hdev_ctrl, STOP_DP_OUT, u4Value);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Set Drop=0\n", __func__));
		} else if (pAd->rDvtCtrl.ucTestItem == 3 && pAd->rDvtCtrl.ucTestParam1 == 1) {
			RTMP_IO_READ32(pAd->hdev_ctrl, STOP_DP_OUT, &u4Value);
			u4Value |= (BIT25 | BIT24 | BIT23 | BIT22 | BIT21 | BIT20);
			RTMP_IO_WRITE32(pAd->hdev_ctrl, STOP_DP_OUT, u4Value);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Set Drop=1\n", __func__));
		} else if (pAd->rDvtCtrl.ucTestItem == 4 && pAd->rDvtCtrl.ucTestParam1 == 0) {
			RTMP_IO_READ32(pAd->hdev_ctrl, STOP_DP_OUT, &u4Value);
			u4Value &= ~(BIT9 | BIT8 | BIT7 | BIT6 | BIT5 | BIT4);
			RTMP_IO_WRITE32(pAd->hdev_ctrl, STOP_DP_OUT, u4Value);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Set Stop=0\n", __func__));
		} else if (pAd->rDvtCtrl.ucTestItem == 4 && pAd->rDvtCtrl.ucTestParam1 == 1) {
			RTMP_IO_READ32(pAd->hdev_ctrl, STOP_DP_OUT, &u4Value);
			u4Value |= (BIT9 | BIT8 | BIT7 | BIT6 | BIT5 | BIT4);
			RTMP_IO_WRITE32(pAd->hdev_ctrl, STOP_DP_OUT, u4Value);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Set Stop=1\n", __func__));
		}
		/* ACQTxNumber */
		else if ((pAd->rDvtCtrl.ucTestItem == 5) && (pAd->rDvtCtrl.ucTestParam1 == 0)) {
			UCHAR ucIdx = 0;

			for (ucIdx = 0; ucIdx < 5; ucIdx++)
				pAd->rDvtCtrl.au4ACQTxNum[ucIdx] = 0;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Reset au4ACQTxNum EP4_AC0(%d), EP5_AC1(%d), EP6_AC2(%d), EP7_AC3(%d), EP9_AC0(%d)\n",
				  __func__,
				  pAd->rDvtCtrl.au4ACQTxNum[0],
				  pAd->rDvtCtrl.au4ACQTxNum[1],
				  pAd->rDvtCtrl.au4ACQTxNum[2],
				  pAd->rDvtCtrl.au4ACQTxNum[3],
				  pAd->rDvtCtrl.au4ACQTxNum[4]));
		} else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: i2ParameterNumber(%d), ucTestItem(%d), parameters error\n",
					__func__, i2ParameterNumber, ucTestItem));
	}

	return TRUE;
}
#endif /* CONFIG_DVT_MODE */

#ifdef MT_MAC
VOID StatRateToString(RTMP_ADAPTER *pAd, CHAR *Output, UCHAR TxRx, UINT32 RawData)
{
	extern UCHAR tmi_rate_map_ofdm[];
	extern UCHAR tmi_rate_map_cck_lp[];
	extern UCHAR tmi_rate_map_cck_sp[];
	UCHAR phy_mode, rate, bw, preamble, sgi, vht_nss;
	UCHAR phy_idx, bw_idx;
	//CHAR * phyMode[8] = {"CCK", "OFDM", "MM", "GF", "VHT", "HE_SU", "HE_EXT_SU", "HE_MU"};
	CHAR *FecCoding[2] = {"BCC", "LDPC"};
	CHAR *bwMode[4] = {"BW20", "BW40", "BW80", "BW160/8080"};

	phy_mode = (RawData >> 13) & 0x7;
	rate = RawData & 0x3F;
	bw = (RawData >> 7) & 0x3;
	sgi = (RawData >> 9) & 0x1;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
		preamble = SHORT_PREAMBLE;
	else
		preamble = LONG_PREAMBLE;

	phy_idx = phy_mode;

	if (bw == BW_20)
		bw_idx = 0;
	else if (bw == BW_40)
		bw_idx = 1;
	else if (bw == BW_80)
		bw_idx = 2;
	else if (bw == BW_160)
		bw_idx = 3;

	if (TxRx == 0)
		sprintf(Output + strlen(Output), "Last TX Rate                    = ");
	else
		sprintf(Output + strlen(Output), "Last RX Rate                    = ");

	if (phy_mode == MODE_CCK) {
		if (TxRx == 0) {
			if (preamble)
				rate = tmi_rate_map_cck_lp[rate];
			else
				rate = tmi_rate_map_cck_sp[rate];
		}

		if (rate == TMI_TX_RATE_CCK_1M_LP)
			sprintf(Output + strlen(Output), "1M LP, ");
		else if (rate == TMI_TX_RATE_CCK_2M_LP)
			sprintf(Output + strlen(Output), "2M LP, ");
		else if (rate == TMI_TX_RATE_CCK_5M_LP)
			sprintf(Output + strlen(Output), "5M LP, ");
		else if (rate == TMI_TX_RATE_CCK_11M_LP)
			sprintf(Output + strlen(Output), "11M LP, ");
		else if (rate == TMI_TX_RATE_CCK_2M_SP)
			sprintf(Output + strlen(Output), "2M SP, ");
		else if (rate == TMI_TX_RATE_CCK_5M_SP)
			sprintf(Output + strlen(Output), "5M SP, ");
		else if (rate == TMI_TX_RATE_CCK_11M_SP)
			sprintf(Output + strlen(Output), "11M SP, ");
		else
			sprintf(Output + strlen(Output), "unkonw, ");
	} else if (phy_mode == MODE_OFDM) {
		if (TxRx == 0)
			rate = tmi_rate_map_ofdm[rate];

		if (rate == TMI_TX_RATE_OFDM_6M)
			sprintf(Output + strlen(Output), "6M, ");
		else if (rate == TMI_TX_RATE_OFDM_9M)
			sprintf(Output + strlen(Output), "9M, ");
		else if (rate == TMI_TX_RATE_OFDM_12M)
			sprintf(Output + strlen(Output), "12M, ");
		else if (rate == TMI_TX_RATE_OFDM_18M)
			sprintf(Output + strlen(Output), "18M, ");
		else if (rate == TMI_TX_RATE_OFDM_24M)
			sprintf(Output + strlen(Output), "24M, ");
		else if (rate == TMI_TX_RATE_OFDM_36M)
			sprintf(Output + strlen(Output), "36M, ");
		else if (rate == TMI_TX_RATE_OFDM_48M)
			sprintf(Output + strlen(Output), "48M, ");
		else if (rate == TMI_TX_RATE_OFDM_54M)
			sprintf(Output + strlen(Output), "54M, ");
		else
			sprintf(Output + strlen(Output), "unkonw, ");
	} else if (phy_mode >= MODE_VHT) {
		vht_nss = ((rate & (0x3 << 4)) >> 4) + 1;
		rate = rate & 0xF;
		sprintf(Output + strlen(Output), "NSS%d_MCS%d, ", vht_nss, rate);
	} else
		sprintf(Output + strlen(Output), "MCS%d, ", rate);

	sprintf(Output + strlen(Output), "%s, ", bwMode[bw_idx]);
	sprintf(Output + strlen(Output), "%cGI, ", sgi ? 'S' : 'L');
	sprintf(Output + strlen(Output), "%s%s %s\n",
		get_phymode_str(phy_mode),
		((RawData >> 10) & 0x1) ? ", STBC" : " ",
		FecCoding[((RawData >> 6) & 0x1)]);
}

VOID StatMt7915RxRateToString(RTMP_ADAPTER *pAd, CHAR *Output, UINT32 RawData)
{
	UCHAR phy_mode, guard_interval, rate, bandwidth, stbc, coding, nsts;
	//CHAR *phyMode[12] = {"CCK", "OFDM", "HT_MM", "HT_GF", "VHT", "HE", "HE5G", "HE2G", "HE_SU", "HE_EXT_SU", "HE_TRIG", "HE_MU"};
	CHAR *FecCoding[2] = {"BCC", "LDPC"};
	CHAR *bwMode[4] = {"BW20", "BW40", "BW80", "BW160/8080"};
	CHAR *HeGi[4] = {"0.8us", "1.6us", "3.2us", " "};
	UINT32 msg_len = 2048;

	/* RawData        Mode [19:16]     GI [15:14]     Rate [13:8]     BW [7:5]     STBC [4]     Coding [3]     Nsts [2:0]  */
	phy_mode = (RawData >> 16) & 0xF;
	guard_interval = (RawData >> 14) & 0x3;
	rate = (RawData >> 8) & 0x3F;
	bandwidth = (RawData >> 5) & 0x7;
	stbc = (RawData >> 4) & 0x1;
	coding = (RawData >> 3) & 0x1;
	nsts = (RawData) & 0x7;

	snprintf(Output + strlen(Output), msg_len - strlen(Output),
		"%s", "Last RX Rate                    = ");

	if (phy_mode > MODE_HE_MU) {
		snprintf(Output + strlen(Output), msg_len - strlen(Output),
			"Undefined for %d\n", phy_mode);
		return;
	}

	if (phy_mode == MODE_CCK) {
		rate = rate & 0x7;
		if (rate == TMI_TX_RATE_CCK_1M_LP)
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "1M LP, ");
		else if (rate == TMI_TX_RATE_CCK_2M_LP)
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "2M LP, ");
		else if (rate == TMI_TX_RATE_CCK_5M_LP)
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "5M LP, ");
		else if (rate == TMI_TX_RATE_CCK_11M_LP)
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "11M LP, ");
		else if (rate == TMI_TX_RATE_CCK_2M_SP)
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "2M SP, ");
		else if (rate == TMI_TX_RATE_CCK_5M_SP)
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "5M SP, ");
		else if (rate == TMI_TX_RATE_CCK_11M_SP)
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "11M SP, ");
		else
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "unknown, ");
	} else if (phy_mode == MODE_OFDM) {
		rate = rate & 0xF;
		if (rate == TMI_TX_RATE_OFDM_6M)
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "6M, ");
		else if (rate == TMI_TX_RATE_OFDM_9M)
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "9M, ");
		else if (rate == TMI_TX_RATE_OFDM_12M)
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "12M, ");
		else if (rate == TMI_TX_RATE_OFDM_18M)
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "18M, ");
		else if (rate == TMI_TX_RATE_OFDM_24M)
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "24M, ");
		else if (rate == TMI_TX_RATE_OFDM_36M)
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "36M, ");
		else if (rate == TMI_TX_RATE_OFDM_48M)
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "48M, ");
		else if (rate == TMI_TX_RATE_OFDM_54M)
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "54M, ");
		else
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "unknown, ");
	} else if (phy_mode == MODE_HTMIX || phy_mode == MODE_HTGREENFIELD) {
		snprintf(Output + strlen(Output), msg_len - strlen(Output), "MCS%d, ", rate);
	} else {
		rate = rate & 0xF;
		if (((phy_mode == MODE_VHT) && (rate > 9)) || ((phy_mode >= MODE_HE) && (rate > 11)))
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "Incorrect NSS%d_MCS%d, ", ((nsts + 1) / (stbc + 1)), rate);
		else
			snprintf(Output + strlen(Output), msg_len - strlen(Output), "NSS%d_MCS%d, ", ((nsts + 1) / (stbc + 1)), rate);
	}

	snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s, ", bwMode[bandwidth]);

	if (phy_mode == MODE_HTMIX || phy_mode == MODE_HTGREENFIELD || phy_mode == MODE_VHT)
		snprintf(Output + strlen(Output), msg_len - strlen(Output),"%cGI, ", (guard_interval & 0x1) ? 'S' : 'L');
	if (phy_mode >= MODE_HE)
		snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s GI, ", HeGi[guard_interval]);

	snprintf(Output + strlen(Output), msg_len - strlen(Output),"%s", get_phymode_str(phy_mode));

	if (phy_mode >= MODE_HTMIX) {
		snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", (stbc) ? ", STBC, " : ", ");
		snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", FecCoding[coding]);
	}

	snprintf(Output + strlen(Output), msg_len - strlen(Output), "%s", "\n");
}

INT Set_themal_sensor(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* 0: get temperature; 1: get adc */
	UINT32 value;
	UINT32 Sensor = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand;

	value = os_str_tol(arg, 0, 10);

	if ((value == 0) || (value == 1)) {
		if (!wdev) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wdev is invalid\n", __func__));
			return FALSE;
		}

		ucBand = HcGetBandByWdev(wdev);
		MtCmdGetThermalSensorResult(pAd, value, ucBand, &Sensor);
		switch (value) {
		case 0:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TMP Value = %d [0x%x]\n", __func__, (INT32)Sensor, (INT32)Sensor));
			break;
		case 1:
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ADC Value = %d [0x%x]\n", __func__, Sensor, Sensor));
			break;
		}
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: 0: get temperature; 1: get adc\n", __func__));

	return TRUE;
}


INT set_manual_rdg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32   ret = 0;
	UINT32   init, resp, txop, wcid, band;

	ret = sscanf(arg, "%u-%u-%u-%u-%u",
		     &(init), &(resp), &(txop), &(wcid), &(band));

	if (ret != 5) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Format Error!! should be: iwpriv ra0 set manual_rdg=[init]-[resp]-[txop]-[wcid]-[band]\n"));
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\n>> Initiator=%x, Responder=%x, Txop=0x%x, Wcid=%u, BandIdx=%x\n",
			  init, resp, txop, wcid, band));

		AsicSetRDG(pAd, wcid, band, init, resp);
	}

	return TRUE;
}
#endif /* MT_MAC */

#ifdef SCS_FW_OFFLOAD

static VOID ScsEventCallback(struct cmd_msg *msg, char *rsp_payload, UINT16 rsp_payload_len);

static VOID scs_show_info_callback(struct cmd_msg *msg, char *rsp_payload,	UINT16 rsp_payload_len);

/*----------------------------------------------------------------------------*/
/*!
* @brief [CMD] Monitor the MU-RGA Group State
*
* @param pucParam
*
* @return status
*/
/*----------------------------------------------------------------------------*/
static VOID scs_show_info_callback(
	struct cmd_msg *msg,
	char *rsp_payload,
	UINT16 rsp_payload_len)
{
	PSCS_SHOW_INFO_T pData = NULL;
	UINT_8 i;
	if (rsp_payload == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: error !! rsp_payload is null!!\n", __func__));
		return;
	}

	pData = (PSCS_SHOW_INFO_T) rsp_payload;

	for (i = 0; i < DBDC_BAND_NUM; i++) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("************** Bnad%d  Information*************\n", i));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("SCSTrafficThreshold =%u\n", pData->rScsInfo.au4SCSTrafficThreshold[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("CckFalseCcaUpBound =%d\n", pData->rScsInfo.ai4CckFalseCcaUpBound[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("CckFalseCcaLowBound =%d\n", pData->rScsInfo.ai4CckFalseCcaLowBound[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("CckFalseCcaCount =%d\n", pData->rScsInfo.ai4CckFalseCcaCount[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OFDMFalseCcaCount =%d\n", pData->rScsInfo.ai4OFDMFalseCcaCount[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OneSecTxByteCount =%u\n", pData->rScsInfo.au4OneSecTxByteCount[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OneSecRxByteCount =%u\n", pData->rScsInfo.au4OneSecRxByteCount[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("RtsCount =%u\n", pData->rScsInfo.au4RtsCount[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("RtsRtyCount =%u\n", pData->rScsInfo.au4RtsRtyCount[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OfdmFalseCcaUpBound =%u\n", (UINT_32)pData->rScsInfo.au2OfdmFalseCcaUpBound[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OfdmFalseCcaLowBound =%u\n", (UINT_32)pData->rScsInfo.au2OfdmFalseCcaLowBound[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("CckFixedRssiBound =%u\n", (UINT_32)pData->rScsInfo.au2CckFixedRssiBound[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OfdmFixedRssiBound =%u\n", (UINT_32)pData->rScsInfo.au2OfdmFixedRssiBound[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OfdmPdBlkTh =%u\n", (UINT_32)pData->rScsInfo.au2OfdmPdBlkTh[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OfdmPdBlkTh =%u\n", (UINT_32)pData->rScsInfo.au2OfdmPdBlkTh[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("SCSMinRssi =%d\n", (INT_32)pData->rScsInfo.ai1SCSMinRssi[i]));


			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("SCSStatus =%u\n", (UINT_32)pData->rScsInfo.au1SCSStatus[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("CckPdBlkTh =%u\n", (UINT_32)pData->rScsInfo.au1CckPdBlkTh[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("SCSThTolerance =%u\n", (UINT_32)pData->rScsInfo.au1SCSThTolerance[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("SCSMinRssiTolerance =%u\n", (UINT_32)pData->rScsInfo.au1SCSMinRssiTolerance[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("SCSEnable =%u\n", (UINT_32)pData->rScsInfo.au1SCSEnable[i]));

			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("OfdmPdSupport =%u\n", (UINT_32)pData->rScsInfo.afgOfdmPdSupport[i]));

	}

}

/*
	==========================================================================
	Description:
		MU_RGA Event Handler

	Parameters:
		Standard MU-RGA  Paramter

	==========================================================================
 */
static VOID ScsEventCallback(
	struct cmd_msg *msg,
	char *rsp_payload,
	UINT16 rsp_payload_len)
{

	UINT32 u4EventId = (*(UINT32 *) rsp_payload);
	char *pData = (rsp_payload);
	UINT16 len = (rsp_payload_len);
#ifdef RT_BIG_ENDIAN
	u4EventId = cpu2le32(u4EventId);
#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: u4EventId = %u, len = %u\n", __func__, u4EventId, len));

	switch (u4EventId) {
	case SCS_SHOW_INFO:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: SCS_SHOW_INFO\n", __func__));
		scs_show_info_callback(msg, pData, len);
		break;

	default:
		break;
	}
}

/* ==========================================================================
Description:
	Send SCS data to FW

Parameters:
	Standard SCS  Paramters

==========================================================================
 */
INT SendSCSDataProc(RTMP_ADAPTER *pAd)
{
	INT32 Ret = TRUE;
	UINT_8 u1BandIndex = 0;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = SCS_EVENT_SEND_DATA;
	CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW Param;
	PSMART_CARRIER_SENSE_CTRL    pSCSCtrl;

	pSCSCtrl = &pAd->SCSCtrl;

	if (pSCSCtrl != NULL) {
		for (u1BandIndex = 0; u1BandIndex < DBDC_BAND_NUM; u1BandIndex++) {
			if (pSCSCtrl->SCSEnable[u1BandIndex] == SCS_ENABLE) {
				Param.OneSecRxByteCount[u1BandIndex] = pSCSCtrl->OneSecRxByteCount[u1BandIndex];
				Param.OneSecTxByteCount[u1BandIndex] = pSCSCtrl->OneSecTxByteCount[u1BandIndex];
				Param.SCSMinRssi[u1BandIndex] = pSCSCtrl->SCSMinRssi[u1BandIndex];			}
		}
	} else {
		Ret = FALSE;
		goto error;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(Param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	for (u1BandIndex = 0; u1BandIndex < DBDC_BAND_NUM; u1BandIndex++) {
		Param.OneSecRxByteCount[u1BandIndex]
			= cpu2le32(Param.OneSecRxByteCount[u1BandIndex]);
		Param.OneSecTxByteCount[u1BandIndex]
			= cpu2le32(Param.OneSecTxByteCount[u1BandIndex]);
	}

#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&Param, sizeof(Param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT SendSCSDataProc_CONNAC3(RTMP_ADAPTER *pAd, UINT8 u1BandIdx)
{
	PSMART_CARRIER_SENSE_CTRL	pSCSCtrl = &pAd->SCSCtrl;
	UINT16	eTput;
	UINT8	ActiveSTA = 0;
	UINT16	j = 0;
	BOOL	fgRxOnly;
	BOOL	PDreset = FALSE;
	/* prepare command message */
	INT32 Ret = TRUE;
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = SCS_EVENT_SEND_DATA;
	CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2 Param;

	if (pSCSCtrl == NULL) {
		Ret = FALSE;
		goto error;
	}
	ActiveSTA = 0;
	eTput = 0;
	fgRxOnly = FALSE;
	PDreset = FALSE;
	if (pSCSCtrl->SCSEnable[u1BandIdx] == SCS_ENABLE) {
		/* Algorithm is not work in RX_only */
		/* TX byte <10% TR ratio*/
		/* Note : Need to calculate : pSCSCtrl->OneSecTxByteCount, pSCSCtrl->OneSecRxByteCount*/
		if ((pSCSCtrl->OneSecTxByteCount[u1BandIdx]) * 9 <  pSCSCtrl->OneSecRxByteCount[u1BandIdx]) {
			pSCSCtrl->PDreset[u1BandIdx] = TRUE;
			fgRxOnly = TRUE;
		} else {
			ActiveSTA = 0;
			pSCSCtrl->PDreset[u1BandIdx] = TRUE;
			fgRxOnly = FALSE;
			/*Part 1 : Tx Success Byte Cnt*/
			for (j = 1; VALID_UCAST_ENTRY_WCID(pAd, j); j++) {
				PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[j];
				if ((pEntry->wdev == NULL) || HcGetBandByWdev(pEntry->wdev) != u1BandIdx)
					continue;
				if ((IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) || IS_ENTRY_WDS(pEntry) || IS_ENTRY_APCLI(pEntry)) {

					/* Mutiple Active STA Support */
					/* Sum all Active STA T-PUT - Tput > 2Mbps */
					if (pEntry->OneSecTxBytes + pEntry->OneSecRxBytes > 250000) {
						eTput += (pEntry->OneSecTxBytes + pEntry->OneSecRxBytes) >> 17;
						pSCSCtrl->PDreset[u1BandIdx] = FALSE;
						ActiveSTA++;
					}

				}
			}
		}
	}
	Param.u1BandIdx = u1BandIdx;
	Param.ActiveSTA = ActiveSTA;
	Param.eTput = eTput;
	Param.fgRxOnly = fgRxOnly;
	Param.PDreset = pSCSCtrl->PDreset[u1BandIdx];
	Param.SCSMinRssi = pSCSCtrl->SCSMinRssi[u1BandIdx];

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("[CMD to FW] Band%u: ActiveSTA %u, eTput %u, fgRxOnly %u, PDreset %u, SCSMinRssi %d, OneSecTxByteCount %u, OneSecRxByteCount %u\n",
			Param.u1BandIdx,
			Param.ActiveSTA,
			Param.eTput,
			Param.fgRxOnly,
			Param.PDreset,
			Param.SCSMinRssi,
			pSCSCtrl->OneSecTxByteCount[u1BandIdx],
			pSCSCtrl->OneSecRxByteCount[u1BandIdx]
		));

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(Param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	Param.eTput = cpu2le16(Param.eTput);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&Param, sizeof(Param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT Show_SCS_FW_Offload_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	UINT32 cmd = SCS_SHOW_INFO;
	struct _CMD_ATTRIBUTE attr = {0};
	SCS_SHOW_INFO_T stat_result = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(stat_result));
	if (!msg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Msg allocation failed\n", __func__));
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(stat_result));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, &stat_result);
	SET_CMD_ATTR_RSP_HANDLER(attr, ScsEventCallback);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));

	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s:(Ret = %d\n", __func__, Ret));

	return Ret;
}

#endif /* SCS_FW_OFFLOAD */

#ifdef SMART_CARRIER_SENSE_SUPPORT
#ifdef SCS_FW_OFFLOAD
INT Set_SCSDefaultEnable(RTMP_ADAPTER *pAd, UINT8 u1BandIdx, UINT8 u1ScsEnable)
{
	INT32 Ret = TRUE;

	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = SCS_EVENT_SCS_ENABLE;
	CMD_SMART_CARRIER_ENABLE Param = {0};

	Param.BandIdx = u1BandIdx;
	Param.SCSEnable = u1ScsEnable;

	/* Match the Enable value of SCS_ENBALE in FW*/
	if (IS_MT7915(pAd))
		Param.SCSEnable = Param.SCSEnable + 1;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): BandIdx=%d, SCSEnable=%d\n", __func__, u1BandIdx, u1ScsEnable));
	if (u1ScsEnable == SCS_DISABLE) {
		pAd->SCSCtrl.SCSEnable[u1BandIdx] = SCS_DISABLE;
		pAd->SCSCtrl.SCSStatus[u1BandIdx] = PD_BLOCKING_OFF;
	} else if (u1ScsEnable == SCS_ENABLE) {
		pAd->SCSCtrl.SCSEnable[u1BandIdx] = SCS_ENABLE;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(Param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&Param, sizeof(Param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));

	return Ret;
}
#endif /*SCS_FW_OFFLOAD*/

INT Set_SCSEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef SCS_FW_OFFLOAD

	INT32 Ret = TRUE;
	struct wifi_dev *wdev;

	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = SCS_EVENT_SCS_ENABLE;
	CMD_SMART_CARRIER_ENABLE Param = {0};
	UINT_32 BandIdx, SCSEnable;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	SCSEnable = os_str_tol(arg, 0, 10);

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);

	Param.BandIdx = (UINT_8) BandIdx;
	Param.SCSEnable = (UINT_8) SCSEnable;

	/* Match the Enable value of SCS_ENBALE in FW*/
	if (IS_MT7915(pAd))
		Param.SCSEnable = Param.SCSEnable + 1;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): BandIdx=%d, SCSEnable=%d\n", __func__, BandIdx, SCSEnable));
	if (SCSEnable == SCS_DISABLE) {
		pAd->SCSCtrl.SCSEnable[BandIdx] = SCS_DISABLE;
		pAd->SCSCtrl.SCSStatus[BandIdx] = PD_BLOCKING_OFF;
	} else if (SCSEnable == SCS_ENABLE) {
		pAd->SCSCtrl.SCSEnable[BandIdx] = SCS_ENABLE;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(Param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&Param, sizeof(Param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;

	/* MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()  BandIdx=%d, u1SCSMinRssiTolerance=%d */
	/* , u1SCSThTolerance=%d, u4SCSTrafficThreshold=%d\n", __func__, */
	/* BandIdx, u1SCSMinRssiTolerance, u1SCSThTolerance, u4SCSTrafficThreshold)); */
	return TRUE;

#else
	UINT32  SCSEnable;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	UCHAR BandIdx = 0;
	struct wifi_dev *wdev;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);
	SCSEnable = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()  BandIdx=%d, SCSEnable=%d\n", __func__, BandIdx, SCSEnable));
	RTMP_CHIP_ASIC_SET_SCS(pAd, BandIdx, SCSEnable);
	return TRUE;
#endif
}

INT Set_SCSPdThrRange_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

#ifdef SCS_FW_OFFLOAD

	INT32 Ret = TRUE;
	UINT32 u4SubCmd = SCS_EVENT_SET_PD_THR_RANGE;
	INT8 recv = 0;
	CMD_SET_SCS_PD_THR_RANGE Param = {0};
	UINT32 u4CckPdUpperBound, u4CckPdLowerBound, u4OfdmPdUpperBound, u4OfdmPdLowerBound;

	UINT_32 BandIdx;
	struct wifi_dev *wdev;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);

	Param.u1BandIdx = (UINT_8) BandIdx;

	if (!IS_MT7915(pAd)) {
			Ret = FALSE;
			goto error;
	}

	if (arg) {
		recv = sscanf(arg, "%u-%u-%u-%u",
					&(u4CckPdUpperBound), &(u4CckPdLowerBound), &(u4OfdmPdUpperBound), &(u4OfdmPdLowerBound));

		if (recv != 4) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Format Error! Please enter in the following format\n"
						"CCK_Pd_Upper-CCK_Pd_Lower-OFDM_Pd_Upper-OFDM_Pd_Lower\n"));
			return TRUE;
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s():CCK_Pd_Upper = %u\nCCK_Pd_Lower = %u\nOFDM_Pd_Upper = %u\nOFDM_Pd_Lower = %u\n",
					__func__, u4CckPdUpperBound, u4CckPdLowerBound, u4OfdmPdUpperBound, u4OfdmPdLowerBound));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:", __func__));
	}

	else
		return FALSE;

	Param.u2CckPdThrMax = (UINT16) u4CckPdUpperBound;
	Param.u2OfdmPdThrMax = (UINT16) u4OfdmPdUpperBound;
	Param.u2CckPdThrMin = (UINT16) u4CckPdLowerBound;
	Param.u2OfdmPdThrMin = (UINT16) u4OfdmPdLowerBound;

	msg = AndesAllocCmdMsg(pAd, sizeof(u4SubCmd) + sizeof(Param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	u4SubCmd = cpu2le32(u4SubCmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&u4SubCmd, sizeof(u4SubCmd));
	AndesAppendCmdMsg(msg, (char *)&Param, sizeof(Param));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
#endif
	return TRUE;
}

INT Set_SCSCfg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef SCS_FW_OFFLOAD

	INT32  Ret = TRUE;
	INT32  Recv = 0;
	INT_32 BandIdx = 0;
	INT_32 SCSThTolerance;
	INT_32 SCSMinRssiTolerance;
	INT_32 OfdmFalseCcaUpBound;
	INT_32 OfdmFalseCcaLowBound;
	INT_32 CckFixedRssiBound;
	INT_32 OfdmPdSupport;
	INT_32 OfdmFixedRssiBound;
	INT_32 SCSTrafficThreshold;
	INT_32 CckFalseCcaUpBound;
	INT_32 CckFalseCcaLowBound;

	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = SCS_EVENT_CONFIG_SCS;
	CMD_SMART_CARRIER_SENSE_CONFIG Param = {0};

	struct wifi_dev *wdev;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	BandIdx = HcGetBandByWdev(wdev);

	Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", &(SCSMinRssiTolerance), &(SCSThTolerance), &(SCSTrafficThreshold),
					  &(OfdmPdSupport), &(CckFalseCcaUpBound), &(CckFalseCcaLowBound), &(OfdmFalseCcaUpBound), &(OfdmFalseCcaLowBound), &(CckFixedRssiBound), &(OfdmFixedRssiBound));

		if ((Recv != 10) || (BandIdx > DBDC_BAND_NUM - 1)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Max supported bands = %d (Supported Band Number is out of range)\n", DBDC_BAND_NUM));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Format Error!\n"));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("iwpriv ra0 set SCSCfg=[MinRssiTolerance]-[ThTolerance]-[TrafficThreshold]-[OfdmSupport]-[CckUpBoundary]-[CckLowBoundary]-[OfdmUpBoundary]-[OfdmLowBoundary]-[FixedCckBoundary]-[FixedOfdmBoundary]"));
		} else {
			Param.u1BandIdx = (UINT_8)BandIdx;
			Param.u1SCSMinRssiTolerance = (UINT_8)SCSMinRssiTolerance;
			Param.u1SCSThTolerance = (UINT_8)SCSThTolerance;
			Param.u4SCSTrafficThreshold = (UINT_32)SCSTrafficThreshold;
			Param.fgOfdmPdSupport = (BOOL)OfdmPdSupport;
			Param.i4CckFalseCcaUpBound = (INT_32)CckFalseCcaUpBound;
			Param.i4CckFalseCcaLowBound = (INT_32)CckFalseCcaLowBound;
			Param.u2OfdmFalseCcaUpBound = (UINT_16)OfdmFalseCcaUpBound;
			Param.u2OfdmFalseCcaLowBound = (UINT_16)OfdmFalseCcaLowBound;
			Param.u2CckFixedRssiBound = (UINT_16)CckFixedRssiBound;
			Param.u2OfdmFixedRssiBound = (UINT_16)OfdmFixedRssiBound;
		}

	msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(Param));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
	cmd = cpu2le32(cmd);
	Param.u4SCSTrafficThreshold = cpu2le32(Param.u4SCSTrafficThreshold);
	Param.i4CckFalseCcaUpBound = cpu2le32(Param.i4CckFalseCcaUpBound);
	Param.i4CckFalseCcaLowBound = cpu2le32(Param.i4CckFalseCcaLowBound);
	Param.u2OfdmFalseCcaUpBound = cpu2le16(Param.u2OfdmFalseCcaUpBound);
	Param.u2OfdmFalseCcaLowBound = cpu2le16(Param.u2OfdmFalseCcaLowBound);
	Param.u2CckFixedRssiBound = cpu2le16(Param.u2CckFixedRssiBound);
	Param.u2OfdmFixedRssiBound = cpu2le16(Param.u2OfdmFixedRssiBound);
#endif
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
	AndesAppendCmdMsg(msg, (char *)&Param, sizeof(Param));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;

	/* MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()  BandIdx=%d, u1SCSMinRssiTolerance=%d */
	/* , u1SCSThTolerance=%d, u4SCSTrafficThreshold=%d\n", __func__, */
	/* BandIdx, u1SCSMinRssiTolerance, u1SCSThTolerance, u4SCSTrafficThreshold)); */
	return TRUE;

#else
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	UCHAR BandIdx = 0;
	INT32 SCSMinRssiTolerance = 0;
	INT32 SCSThTolerance = 0;
	UINT32 SCSTrafficThreshold = 0;
	UINT32 OfdmPdSupport = 0;
	INT32   Recv = 0;
	struct wifi_dev *wdev;
	UINT32 CckUpBond = 0, CckLowBond = 0, OfdmUpBond = 0, OfdmLowBond = 0, CckFixedBond = 0, OfdmFixedBond = 0;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	BandIdx = HcGetBandByWdev(wdev);

	Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", &(SCSMinRssiTolerance), &(SCSThTolerance), &(SCSTrafficThreshold),
		      &(OfdmPdSupport), &(CckUpBond), &(CckLowBond), &(OfdmUpBond), &(OfdmLowBond), &(CckFixedBond), &(OfdmFixedBond));

	if (Recv != 10) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error!\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("iwpriv ra0 set SCSCfg=[MinRssiTolerance]-[ThTolerance]-[TrafficThreshold]-[OfdmSupport]-[CckUpBoundary]-[CckLowBoundary]-[OfdmUpBoundary]-[OfdmLowBoundary]-[FixedCckBoundary]-[FixedOfdmBoundary]"));
	} else {
		pAd->SCSCtrl.SCSMinRssiTolerance[BandIdx] = SCSMinRssiTolerance;
		pAd->SCSCtrl.SCSThTolerance[BandIdx] = SCSThTolerance;
		pAd->SCSCtrl.SCSTrafficThreshold[BandIdx] = SCSTrafficThreshold;
		pAd->SCSCtrl.OfdmPdSupport[BandIdx] = (UCHAR) OfdmPdSupport;
		pAd->SCSCtrl.CckFalseCcaUpBond[BandIdx] = (UINT16)CckUpBond;
		pAd->SCSCtrl.CckFalseCcaLowBond[BandIdx] = (UINT16)CckLowBond;
		pAd->SCSCtrl.OfdmFalseCcaUpBond[BandIdx] = (UINT16)OfdmUpBond;
		pAd->SCSCtrl.OfdmFalseCcaLowBond[BandIdx] = (UINT16)OfdmLowBond;
		pAd->SCSCtrl.CckFixedRssiBond[BandIdx] = (INT32)CckFixedBond;
		pAd->SCSCtrl.OfdmFixedRssiBond[BandIdx] = (INT32)OfdmFixedBond;
	}

	/* MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()  BandIdx=%d, SCSMinRssiTolerance=%d */
	/* , SCSThTolerance=%d, SCSTrafficThreshold=%d\n", __func__, */
	/* BandIdx, SCSMinRssiTolerance, SCSThTolerance, SCSTrafficThreshold)); */
	return TRUE;
#endif
}

INT Set_SCSPd_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef SCS_FW_OFFLOAD

	INT32	Recv = 0;
	INT32	Ret = TRUE;
	INT32	BandIdx = 0;
	INT32	CckPdBlkTh = 0;
	INT32	OfdmPdBlkTh = 0;
	/* prepare command message */
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	UINT32 cmd = SCS_EVENT_SET_MANUAL_PD_TH;
	CMD_SMART_CARRIER_SENSE_CTRL_PD_TH Param = {0};
	struct wifi_dev *wdev;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	BandIdx = HcGetBandByWdev(wdev);

	Recv = sscanf(arg, "%d-%d-%d",	&(BandIdx), &(CckPdBlkTh), &(OfdmPdBlkTh));
	if (Recv != 3 || (CckPdBlkTh < 30 || CckPdBlkTh > 110)
			|| (OfdmPdBlkTh < 30 || OfdmPdBlkTh > 98) || (BandIdx > DBDC_BAND_NUM - 1)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("Format Error or Out of range\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("iwpriv ra0 set SCSCfg=[CckPdBlkTh]-[OfdmPdBlkTh]\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("CckPdBlkTh  Range: 30~110 dBm (Represents a negative number)\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("OfdmPdBlkTh Range: 30~98	dBm (Represents a negative number)\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("Max supported bands = %d (Supported Band Number is out of range)\n", DBDC_BAND_NUM));
		goto error;
	} else {
		Param.BandIdx = (UINT_8)BandIdx;
		Param.SCSEnable = SCS_MANUAL;
		Param.CckPdBlkTh = ((CckPdBlkTh * (-1)) + 256);
		Param.OfdmPdBlkTh = ((OfdmPdBlkTh * (-2)) + 512);
#ifdef RT_BIG_ENDIAN
		Param.OfdmPdBlkTh = cpu2le16(Param.OfdmPdBlkTh);
#endif

		msg = AndesAllocCmdMsg(pAd, sizeof(cmd) + sizeof(Param));

		if (!msg) {
			Ret = FALSE;
			goto error;
		}

		SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
		SET_CMD_ATTR_TYPE(attr, EXT_CID);
		SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SCS_FEATURE_CTRL);
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
		SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
		SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
		AndesInitCmdMsg(msg, attr);
#ifdef RT_BIG_ENDIAN
		cmd = cpu2le32(cmd);
#endif
		AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));
		AndesAppendCmdMsg(msg, (char *)&Param, sizeof(Param));
		AndesSendCmdMsg(pAd, msg);
	}
		error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;

#else
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	UCHAR BandIdx = 0;
	INT32	CckPdBlkTh = 0;
	INT32	OfdmPdBlkTh = 0;
	INT32   Recv = 0;
	UINT32 CrValue;
	struct wifi_dev *wdev;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);

	Recv = sscanf(arg, "%d-%d", &(CckPdBlkTh), &(OfdmPdBlkTh));
	if (Recv != 2 || (CckPdBlkTh < 30 || CckPdBlkTh > 110) || (OfdmPdBlkTh < 30 || OfdmPdBlkTh > 98)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error or Out of range\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("iwpriv ra0 set SCSCfg=[CckPdBlkTh]-[OfdmPdBlkTh]\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CckPdBlkTh  Range: 30~110 dBm (Represents a negative number)\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("OfdmPdBlkTh Range: 30~98  dBm (Represents a negative number)\n"));
	} else {
		pAd->SCSCtrl.SCSEnable[BandIdx] = SCS_MANUAL;
		pAd->SCSCtrl.CckPdBlkTh[BandIdx] = ((CckPdBlkTh * (-1)) + 256);
		pAd->SCSCtrl.OfdmPdBlkTh[BandIdx] = ((OfdmPdBlkTh * (-2)) + 512);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Band%d CckPdBlkTh  = -%ddBm (%d)\n",
				BandIdx, CckPdBlkTh, pAd->SCSCtrl.CckPdBlkTh[BandIdx]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Band%d OfdmPdBlkTh = -%ddBm (%d)\n",
				BandIdx, OfdmPdBlkTh, pAd->SCSCtrl.OfdmPdBlkTh[BandIdx]));
		HW_IO_READ32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR, &CrValue);
		CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);  /* OFDM PD BLOCKING TH */
		CrValue |= (pAd->SCSCtrl.OfdmPdBlkTh[BandIdx] << PdBlkOfmdThOffset);
		HW_IO_WRITE32(pAd->hdev_ctrl, PHY_MIN_PRI_PWR, CrValue);
		HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7, &CrValue);
		CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset); /* Bit[8:1] */
		CrValue |= (pAd->SCSCtrl.CckPdBlkTh[BandIdx] << PdBlkCckThOffset);
		HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_7, CrValue);
		HW_IO_READ32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8, &CrValue);
		CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset); /* Bit[31:24] */
		CrValue |= (pAd->SCSCtrl.CckPdBlkTh[BandIdx] << PdBlkCck1RThOffset);
		HW_IO_WRITE32(pAd->hdev_ctrl, PHY_RXTD_CCKPD_8, CrValue);
	}
	return TRUE;
#endif
}

#endif /* SMART_CARRIER_SENSE_SUPPORT */

INT32 EnableNF(RTMP_ADAPTER *pAd, UINT8 isEnable, UINT8 u1TimeOut, UINT8 u1Count)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EXT_CMD_ENABLE_NOISE_FLOOR_T NfParam = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	if (!IS_MT7915(pAd)) {
		Ret = FALSE;
		goto error;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:", __func__));

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(NfParam));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_ENABLE_NOISEFLOOR);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

	NfParam.fgEnable = (BOOLEAN)isEnable;
	NfParam.u1TimeOut = u1TimeOut;
	NfParam.u1Count = u1Count;

#ifdef RT_BIG_ENDIAN
	NfParam = cpu2le32(NfParam);
#endif

	AndesAppendCmdMsg(msg, (char *)&NfParam, sizeof(NfParam));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}

INT SetEnableNf(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	UINT8 u1Enable, u1TimeOut, u1Count;
	INT status = TRUE;

	pch = strsep(&arg, "-");

	if (pch != NULL)
		u1Enable = os_str_tol(pch, 0, 10);
	else
		return FALSE;

	pch = strsep(&arg, "-");
	/*Get Timer Timeout*/
	if (pch != NULL)
		u1TimeOut = os_str_tol(pch, 0, 10);
	else
		return FALSE;

	pch = strsep(&arg, "");
	/*Get Time Count*/
	if (pch != NULL)
		u1Count = os_str_tol(pch, 0, 10);
	else
		return FALSE;

	if ((u1TimeOut != 0) && (u1Count != 0) && ((u1TimeOut % 100) == 0))
		EnableNF(pAd, u1Enable, u1TimeOut, u1Count);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\nTimeOut shoud be a multiple of 100ms\n"));
		return FALSE;
	}
	return status;
}

#ifdef WIFI_MD_COEX_SUPPORT
INT SendApccci2fwMsg(RTMP_ADAPTER *pAd, struct _MT_WIFI_COEX_APCCCI2FW *apccci2fw_msg)
{
	INT32	Ret = TRUE;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};

	if (!IS_MT7915(pAd)) {
		Ret = FALSE;
		goto error;
	}

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, apccci2fw_msg->len);

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_APCCCI_MSG);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

#ifdef RT_BIG_ENDIAN
	apccci2fw_msg = cpu2le32(apccci2fw_msg);
#endif

	AndesAppendCmdMsg(msg, (char *)apccci2fw_msg->data, apccci2fw_msg->len);
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_COEX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(Ret = %d_\n", __func__, Ret));
	return Ret;
}
#endif /* WIFI_MD_COEX_SUPPORT */

INT set_support_rate_table_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
	UINT8 i = 0;
	CHAR *value = 0;
	UINT8 tx_mode = 0;
	UINT8 tx_nss = 0;
	UINT8 tx_bw = 0;
	UINT16 mcs_cap = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: no parameter.\n", __func__));
		goto error0;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			tx_mode = simple_strtol(value, 0, 10);
			break;

		case 1:
			tx_nss = simple_strtol(value, 0, 10);
			break;

		case 2:
			tx_bw = simple_strtol(value, 0, 10);
			break;

		case 3:
			/* input with hex format */
			mcs_cap = simple_strtol(value, 0, 16);
			break;

		default: {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: invalid parameter format.\n", __func__));
			goto error0;
		}
		}
	}

	status = support_rate_table_ctrl(pAd, tx_mode, tx_nss, tx_bw, &mcs_cap);

	return status;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("----------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("argument format:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("	[tx_mode]:[tx_nss]:[tx_bw]:[mcs_cap]:[set]\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("----------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("tx_mode: 0/1/2/4/8 for cck/ofdm/ht/vht/he\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("tx_nss: 0~3 for 1ss/2ss/3ss/4ss\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("tx_bw: 0~3 for bw20/40/80/160\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("mcs_cap: hex format representation\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("	ex: 88 for supprt mcs7 and mcs3.\n"));
	return FALSE;
}

INT set_support_rate_table_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	return TRUE;
}

# define RA_DBG_SUPPORT_PARAM_NUM    20
INT set_ra_dbg_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
	UINT8 i = 0;
	CHAR *value = 0;
	UINT8 param_num;
	UINT32 param[RA_DBG_SUPPORT_PARAM_NUM] = {0};

	/* sanity check for input parameter format */
	if (!arg)
		goto error0;

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		/* parsing for number of parameters */
		if (i == 0) {
			param_num = simple_strtol(value, 0, 10);

			if (param_num > RA_DBG_SUPPORT_PARAM_NUM)
				goto error2;
		}

		/* error handle */
		if (i > RA_DBG_SUPPORT_PARAM_NUM)
			goto error1;

		/* parsing for parameters */
		if (i > 0)
			param[i-1] = simple_strtol(value, 0, 10);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: param_num: %d \n", __func__, param_num));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: param:", __func__));

	for (i = 0; i < RA_DBG_SUPPORT_PARAM_NUM; i++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			(" %d", param[i]));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\n"));

	status = ra_dbg_ctrl(pAd, param_num, param);

	return status;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: no parameter.\n", __func__));
	return FALSE;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: invalid parameter format (param num not larger than %d) \n", __func__, RA_DBG_SUPPORT_PARAM_NUM));
	return FALSE;

error2:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: param num not larger than %d \n", __func__, RA_DBG_SUPPORT_PARAM_NUM));
	return FALSE;
}

INT SetSKUCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR	*value = 0;
	BOOLEAN tx_pwr_sku_en = FALSE;
	INT	 status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_ATE
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
#ifdef DBDC_MODE
	struct _BAND_INFO *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR	   apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucBandIdx = %d\n", __func__, ucBandIdx));

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!!\n",
				__func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			tx_pwr_sku_en = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((tx_pwr_sku_en != FALSE) && (tx_pwr_sku_en != TRUE)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxPowerSKUEn: %d\n", __func__, tx_pwr_sku_en));
	/* Update Profile Info for SKU */
#ifdef SINGLE_SKU_V2
	pAd->CommonCfg.SKUenable[ucBandIdx] = tx_pwr_sku_en;
#endif /* SINGLE_SKU_V2 */

#ifdef CONFIG_ATE
	/* Update SKU Status in ATECTRL Structure */
	if (ucBandIdx == BAND0)
		ATECtrl->tx_pwr_sku_en = tx_pwr_sku_en;
#ifdef DBDC_MODE
	else if (ucBandIdx == BAND1)
		Info->tx_pwr_sku_en = tx_pwr_sku_en;
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */

#ifdef SINGLE_SKU_V2
	return TxPowerSKUCtrl(pAd, tx_pwr_sku_en, ucBandIdx);
#else
	return TRUE;
#endif /* SINGLE_SKU_V2 */
}

INT SetPercentageCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = 0;
	BOOLEAN fgTxPowerPercentEn = FALSE;
	INT	 status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_ATE
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
#ifdef DBDC_MODE
	struct _BAND_INFO *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucBandIdx = %d\n", __func__, ucBandIdx));

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!!\n",
				__func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgTxPowerPercentEn = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((fgTxPowerPercentEn != FALSE) && (fgTxPowerPercentEn != TRUE)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxPowerPercentEn = %d\n", __func__, fgTxPowerPercentEn));

	/* Update Profile Info for Power Percentage */
	pAd->CommonCfg.PERCENTAGEenable[ucBandIdx] = fgTxPowerPercentEn;

#ifdef CONFIG_ATE
	/* Update Power Percentage Status in ATECTRL Structure */
	if (ucBandIdx == BAND0)
		ATECtrl->tx_pwr_percentage_en = fgTxPowerPercentEn;
#ifdef DBDC_MODE
	else if (ucBandIdx == BAND1)
		Info->tx_pwr_percentage_en = fgTxPowerPercentEn;
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */

	return TxPowerPercentCtrl(pAd, fgTxPowerPercentEn, ucBandIdx);
}

INT SetPowerDropCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR	*value = 0;
	UINT8	ucPowerDrop = 0;
	INT	 status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef MGMT_TXPWR_CTRL
	BSS_STRUCT *pMbss = NULL;
#endif
#ifdef CONFIG_ATE
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
#ifdef DBDC_MODE
	struct _BAND_INFO *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */

	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			ucPowerDrop = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucPowerDrop = %d\n", __func__, ucPowerDrop));

	/* Update Profile Info for Power Percentage Drop Value */
	pAd->CommonCfg.ucTxPowerPercentage[ucBandIdx] = ucPowerDrop;

#ifdef CONFIG_ATE
	/* Update Power Percentage Drop Value Status in ATECTRL Structure */
	if (ucBandIdx == BAND0)
		ATECtrl->tx_pwr_percentage_level = ucPowerDrop;
#ifdef DBDC_MODE
	else if (ucBandIdx == BAND1)
		Info->tx_pwr_percentage_level = ucPowerDrop;
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */
#ifdef MGMT_TXPWR_CTRL
	status = TxPowerDropCtrl(pAd, ucPowerDrop, ucBandIdx);
	if(!status) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ERROR: TxPowerDropCtrl() FAILED!\n "));
		return FALSE;
	}

	for(i=0; i < pAd->ApCfg.BssidNum; i++)
	{
		pMbss = &pAd->ApCfg.MBSSID[i];
		wdev = &pMbss->wdev;
		if(HcGetBandByWdev(wdev)!= ucBandIdx)
			continue;

		/*clear old data for iwpriv set channel*/
		wdev->bPwrCtrlEn = FALSE;
		wdev->TxPwrDelta = 0;
		/* Get EPA info by Tx Power info cmd*/
		pAd->ApCfg.MgmtTxPwr[ucBandIdx] = 0;
		MtCmdTxPwrShowInfo(pAd, TXPOWER_ALL_RATE_POWER_INFO, ucBandIdx);

		/* Update beacon/probe TxPwr wrt profile param */
		if (wdev->MgmtTxPwr) {
			/* wait until TX Pwr event rx*/
			RtmpusecDelay(50);
			update_mgmt_frame_power(pAd, wdev);
		}

	}
	return status;
#else
	return TxPowerDropCtrl(pAd, ucPowerDrop, ucBandIdx);
#endif
}

INT SetDecreasePwrCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
	BOOLEAN  fgStatus = FALSE;
	INT8  cPowerDropLevel = 0;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */

	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	cPowerDropLevel = simple_strtol(arg, 0, 10);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: cPowerDropLevel (0.5 dBm) = %d\n", __func__, cPowerDropLevel));

	if (MtCmdTxPowerDropCtrl(pAd, cPowerDropLevel, ucBandIdx) == 0)
		fgStatus = TRUE;

	return fgStatus;
}

INT SetBfBackoffCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR	*value = 0;
	BOOLEAN fgTxBFBackoffEn = FALSE;
	INT	 status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_ATE
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
#ifdef DBDC_MODE
	struct _BAND_INFO *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucBandIdx = %d\n", __func__, ucBandIdx));

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgTxBFBackoffEn = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((fgTxBFBackoffEn != FALSE) && (fgTxBFBackoffEn != TRUE)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxBFBackoffEn = %d\n", __func__, fgTxBFBackoffEn));

	/* Update Profile Info for Power Percentage */
	pAd->CommonCfg.BFBACKOFFenable[ucBandIdx] = fgTxBFBackoffEn;

#ifdef CONFIG_ATE
	/* Update Power Percentage Drop Value Status in ATECTRL Structure */
	if (ucBandIdx == BAND0)
		ATECtrl->tx_pwr_backoff_en = fgTxBFBackoffEn;
#ifdef DBDC_MODE
	else if (ucBandIdx == BAND1)
		Info->tx_pwr_backoff_en = fgTxBFBackoffEn;
#endif /* DBDC_MODE */
#endif /* CONFIG_ATE */

#ifdef SINGLE_SKU_V2
	return TxPowerBfBackoffCtrl(pAd, fgTxBFBackoffEn, ucBandIdx);
#else
	return TRUE;
#endif /* SINGLE_SKU_V2 */
}

INT SetThermoCompCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR	*value = 0;
	BOOLEAN fgThermoCompEn = 0;
	INT	 status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucBandIdx = %d\n", __func__, ucBandIdx));

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!!\n",
				__func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgThermoCompEn = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((fgThermoCompEn != FALSE) && (fgThermoCompEn != TRUE)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgThermoCompEn = %d\n", __func__, fgThermoCompEn));
	return ThermoCompCtrl(pAd, fgThermoCompEn, ucBandIdx);
}

INT SetCCKTxStream(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR    *value = 0;
	UINT8   u1CCKTxStream = 0;
	INT     status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;

#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	   apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	ucBandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	wdev = &pAd->StaCfg[0].wdev;
	ucBandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_STA_SUPPORT */

	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			u1CCKTxStream = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	/* sanity check for input parameter range */
	if (u1CCKTxStream >= WF_NUM || !u1CCKTxStream) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("EX. iwpriv <interface> set CCKTxStream=1 (1~4)\n"));
		return FALSE;
	}

	/* Update Profile Info for Power Percentage Drop Value */
	pAd->CommonCfg.CCKTxStream[ucBandIdx] = u1CCKTxStream;

	return TxCCKStreamCtrl(pAd, pAd->CommonCfg.CCKTxStream[ucBandIdx], ucBandIdx);
}

INT SetRfTxAnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	INT     status = TRUE;
	CHAR	*value = 0;
	UINT8   ucTxAntIdx = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 2) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format with 2-digit.\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			ucTxAntIdx = simple_strtol(value, 0, 10);
			break;
		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
			break;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucTxAntIdx: 0x%x\n", __func__, ucTxAntIdx));
	return TxPowerRfTxAnt(pAd, ucTxAntIdx);
}

INT SetTxPowerInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = 0;
	UCHAR   ucTxPowerInfoCatg = 0;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR  IfIdx = 0;
	IfIdx = pObj->ioctl_if;

	/* Grab Band index */
	if (pObj->ioctl_if_type == INT_MBSSID || pObj->ioctl_if_type == INT_MAIN) {
#ifdef CONFIG_AP_SUPPORT
		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: AP Band = %d !!\n", __func__, ucBandIdx));

#endif
	}
#ifdef CONFIG_STA_SUPPORT
	else if (pObj->ioctl_if_type == INT_APCLI) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: STA Band = %d !!\n", __func__, ucBandIdx));
	} else if (pObj->ioctl_if_type == INT_MSTA) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: STA Band = %d !!\n", __func__, ucBandIdx));
		}
#endif
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[SetTxPowerInfo]: pObj->ioctl_if_type = %d!!\n"
, pObj->ioctl_if_type));
		return FALSE;
	}

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Invalid Band Index!!\n", __func__));
		goto error;
	}

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		goto error;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			ucTxPowerInfoCatg = simple_strtol(value, 0, 10);
			break;

		default: {
			break;
		}
		}
	}

	/* Sanity check for parameter range */
	if (ucTxPowerInfoCatg >= POWER_INFO_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: parameter out of range!!\n", __func__));
		goto error;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BandIdx: %d, ucTxPowerInfoCatg: %d\n", __func__, ucBandIdx, ucTxPowerInfoCatg));
#ifdef MGMT_TXPWR_CTRL
	g_fgCommand = TRUE;
#endif
	return TxPowerShowInfo(pAd, ucTxPowerInfoCatg, ucBandIdx);

error:

#if defined(MT7615) || defined(MT7622)
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("iwpriv <interface> set TxPowerInfo=[param1]\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	param1: Tx Power Info Category (0~3)\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("			0: Tx Power Basic Info\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("			1: Thermal Sensor Basic Info\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("			2: Backup Power Table\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("			3: Thermal Compensation Table\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("			4: TXV/BBP Power Value (per packet)\n"));
#else
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("iwpriv <interface> set TxPowerInfo=[param1]\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("	param1: Tx Power Info Category (0~3)\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("			0: Tx Power Basic Info\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("			1: Backup Power Table\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("			2: Tx Power Rate Power Info\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("			3: Thermal Compensation Table\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("			4: TXV/BBP Power Value (per packet)\n"));
#endif /* defined(MT7615) || defined(MT7622) */

	return FALSE;
}

INT SetTOAECtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	 *value = 0;
	UCHAR   TOAECtrl = 0;
	INT	 status = TRUE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			TOAECtrl = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((TOAECtrl != FALSE) && (TOAECtrl != TRUE)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please input 1(Enable) or 0(Disable)!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TOAECtrl = %d\n", __func__, TOAECtrl));
	return TOAECtrlCmd(pAd, TOAECtrl);
}

#define SKU_FAIL 1
INT SetSKUInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
#ifdef SINGLE_SKU_V2
#if defined(MT7615) || defined(MT7622)
	/* print out Sku table info */
	MtShowSkuTable(pAd, DBG_LVL_OFF);
#else
	/* print out Sku table info */
	if (NDIS_STATUS_SUCCESS != MtShowPwrLimitTable(pAd, POWER_LIMIT_TABLE_TYPE_SKU, DBG_LVL_OFF))
		status = SKU_FAIL;
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* SINGLE_SKU_V2 */
	return status;
}

INT SetBFBackoffInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
#ifdef SINGLE_SKU_V2
#if defined(MT7615) || defined(MT7622)
	UINT8 i;
	BACKOFF_POWER *ch, *ch_temp;

	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BF Backof table index: %d\n", pAd->CommonCfg.SKUTableIdx));
	DlListForEachSafe(ch, ch_temp, &pAd->PwrLimitBackoffList, BACKOFF_POWER, List) {
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start ch = %d, ch->num = %d\n", ch->StartChannel, ch->num));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Band: %d\n", ch->band));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel: "));

		for (i = 0; i < ch->num; i++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d ", ch->Channel[i]));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Max Power: "));

		for (i = 0; i < 3; i++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d ", ch->PwrMax[i]));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}
#else
	/* print out Backoff table info */
	if (NDIS_STATUS_SUCCESS != MtShowPwrLimitTable(pAd, POWER_LIMIT_TABLE_TYPE_BACKOFF, DBG_LVL_OFF))
		status = SKU_FAIL;
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* SINGLE_SKU_V2 */
	return status;
}

#define MT7915_TXD_TX_PWR_OFFSET_VALUE_MIN       -32
#define MT7915_TXD_TX_PWR_OFFSET_VALUE_MAX        31
#define TXD_TX_PWR_OFFSET_VALUE_MIN              -16
#define TXD_TX_PWR_OFFSET_VALUE_MAX               15

INT set_mgmt_txpwr_offset(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev;
	INT8 txpwr_offset = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT8 txd_txpwr_min, txd_txpwr_max;
#ifdef CONFIG_AP_SUPPORT
	UCHAR apidx = pObj->ioctl_if;
#endif

	if (pObj->ioctl_if_type == INT_MBSSID || pObj->ioctl_if_type == INT_MAIN) {
#ifdef CONFIG_AP_SUPPORT
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#endif
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: pObj->ioctl_if_type = %d!!\n", __func__, pObj->ioctl_if_type));
		return FALSE;
	}

	if (arg != NULL) {
		txpwr_offset = os_str_tol(arg, 0, 10);
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Invalid parameters!!\n", __func__));
		return FALSE;
	}

	if (IS_MT7915(pAd)) {
		txd_txpwr_min = MT7915_TXD_TX_PWR_OFFSET_VALUE_MIN;
		txd_txpwr_max = MT7915_TXD_TX_PWR_OFFSET_VALUE_MAX;
	} else {
		txd_txpwr_min = TXD_TX_PWR_OFFSET_VALUE_MIN;
		txd_txpwr_max = TXD_TX_PWR_OFFSET_VALUE_MAX;
	}

	if ((txpwr_offset < txd_txpwr_min) || (txpwr_offset > txd_txpwr_max)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: [TxdPwrOffset] range [%d,%d]\n", __func__, txd_txpwr_min, txd_txpwr_max));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: [TxdPwrOffset]: %u\n", __func__, (UINT8)txpwr_offset));

	wdev->mgmt_txd_txpwr_offset = (UINT8)txpwr_offset;

#ifdef CONFIG_AP_SUPPORT
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_DISABLE_TX);
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_ENABLE_TX);
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}

INT show_BSSEdca_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ap_idx)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s() invalid mbss id(%d)\n", __func__, ap_idx));
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}
	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pBssEdca is NULL\n"));
		return FALSE;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("   BSSEdca:     AIFSN    CWmin   CWmax    TXOP(us)  ACM\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_BE	  %2d	  %2d	  %2d	   %4d	   %d\n",
							 pBssEdca->Aifsn[0],
							 pBssEdca->Cwmin[0],
							 pBssEdca->Cwmax[0],
							 pBssEdca->Txop[0],
							 pBssEdca->bACM[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_BK	  %2d	  %2d	  %2d	   %4d	   %d\n",
							 pBssEdca->Aifsn[1],
							 pBssEdca->Cwmin[1],
							 pBssEdca->Cwmax[1],
							 pBssEdca->Txop[1],
							 pBssEdca->bACM[1]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_VI	  %2d	  %2d	  %2d	   %4d	   %d\n",
							 pBssEdca->Aifsn[2],
							 pBssEdca->Cwmin[2],
							 pBssEdca->Cwmax[2],
							 pBssEdca->Txop[2],
							 pBssEdca->bACM[2]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_VO	  %2d	  %2d	  %2d	   %4d	   %d\n",
							 pBssEdca->Aifsn[3],
							 pBssEdca->Cwmin[3],
							 pBssEdca->Cwmax[3],
							 pBssEdca->Txop[3],
							 pBssEdca->bACM[3]));

	return TRUE;
}

INT show_APEdca_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ap_idx)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s() invalid mbss id(%d)\n", __func__, ap_idx));
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("   APEdca:     AIFSN     CWmin   CWmax    TXOP(us)  ACM\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_BE	%2d	  %2d	  %2d	   %4d	     %d\n",
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[0],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[0],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[0],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[0],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].bACM[0]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_BK	%2d	  %2d	  %2d	   %4d	     %d\n",
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[1],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[1],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[1],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[1],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].bACM[1]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_VI	%2d	  %2d	  %2d	   %4d	     %d\n",
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[2],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[2],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[2],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[2],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].bACM[2]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("	   AC_VO	%2d	  %2d	  %2d	   %4d	     %d\n",
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[3],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[3],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[3],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[3],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].bACM[3]));

	return TRUE;
}

#ifdef AMPDU_CONF_SUPPORT
#define MAX_MPDU_COUNT_FOR_AGG   256
INT Set_AMPDU_MPDU_Count(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 count = 0;
	EXT_CMD_CFG_SET_AGG_AC_LIMIT_T ExtCmdAggAcLimitCfg = {0};
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			    ("%s: Incoorect BSS!!\n", __func__));
		return FALSE;
	}
	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			    ("%s: No parameters!!\n", __func__));
		return FALSE;
	}
	count = os_str_tol(arg, 0, 10);
	if (count > MAX_MPDU_COUNT_FOR_AGG) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			    ("%s: Max MPDU in AMPDU support = 256!!\n", __func__));
		return FALSE;
	}

	ExtCmdAggAcLimitCfg.ucWmmIdx = HcGetWmmIdx(pAd, wdev);
	ExtCmdAggAcLimitCfg.ucAggLimit = count;
	ExtCmdAggAcLimitCfg.ucAc = QID_AC_BK;
	CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_AGG_AC_LIMT_FEATURE, &ExtCmdAggAcLimitCfg);
	ExtCmdAggAcLimitCfg.ucAc = QID_AC_BE;
	CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_AGG_AC_LIMT_FEATURE, &ExtCmdAggAcLimitCfg);
	ExtCmdAggAcLimitCfg.ucAc = QID_AC_VI;
	CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_AGG_AC_LIMT_FEATURE, &ExtCmdAggAcLimitCfg);
	ExtCmdAggAcLimitCfg.ucAc = QID_AC_VO;
	CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_AGG_AC_LIMT_FEATURE, &ExtCmdAggAcLimitCfg);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		    ("Number of MPDUs in AMPDU set = %d!!\n", count));
	return TRUE;
}
#define AMPDU_MAX_REMAIN_TX_COUNT 0x1F
INT Show_AMPDU_Retry_Count(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	UCHAR count = 0;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#endif /* CONFIG_AP_SUPPORT */
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Incorrect BSS!! \n", __func__));
		return FALSE;
	}
	count = wdev->bAMPDURetrynum;

	if (count == AMPDU_MAX_REMAIN_TX_COUNT)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Unlimited retry!\n", __func__));
	else if (count == 0)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Auto retry control!\n", __func__));
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Tx retry count = %d!\n", __func__, count - 1));
	return TRUE;
}

INT Set_AMPDU_Retry_Count(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 count = 0;
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#endif /* CONFIG_AP_SUPPORT */
	if (!wdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Incorrect BSS!! \n", __func__));
		return FALSE;
	}
	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}
	count = os_str_tol(arg, 0, 10);
	if (count > AMPDU_MAX_REMAIN_TX_COUNT) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Error, Retry range is: [0-1F]!\n"));
		return FALSE;
	}
	wdev->bAMPDURetrynum = count;
	MtCmdCr4Set(pAd, WA_SET_AMPDU_RETRY_COUNT, apidx, count);
	if (count == AMPDU_MAX_REMAIN_TX_COUNT)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Unlimited retry!\n", __func__));
	else if (count == 0)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Auto retry control!\n", __func__));
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s(): Set Tx retry count = %d!\n", __func__, count - 1));
	return TRUE;
}
#endif


#ifdef WIFI_GPIO_CTRL
INT set_gpio_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *pch = NULL;
	UINT8 gpio_idx;
	BOOLEAN gpio_en = FALSE;

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		gpio_idx = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: No parameters for gpio_idx!!\n", __func__));
		return FALSE;
	}

	if (gpio_idx < GPIO_INDEX_MIN_VAL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: gpio_idx value should not be less than %d!\n",
				__func__, GPIO_INDEX_MIN_VAL));
		return FALSE;
	}

	if (gpio_idx > GPIO_INDEX_MAX_VAL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: gpio_idx value should not be greater than %d!\n",
				__func__, GPIO_INDEX_MAX_VAL));
		return FALSE;
	}

	pch = arg;
	if (pch != NULL)
		gpio_en = (BOOLEAN) os_str_toul(pch, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: No parameters for gpio_en!!\n", __func__));
		return FALSE;
	}

	if (gpio_en)
		gpio_en = TRUE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: gpio_num:%u Enable:%u\n",
			__func__, gpio_idx, gpio_en));

	return SetGpioCtrl(pAd, gpio_idx, gpio_en);
}

INT set_gpio_value(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *pch = NULL;
	UINT8 gpio_idx, gpio_val;

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Invalid parameters\n", __func__));
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		gpio_idx = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: No parameters for gpio_idx!!\n", __func__));
		return FALSE;
	}

	if (gpio_idx < GPIO_INDEX_MIN_VAL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: gpio_idx value should not be less than %d!\n",
				__func__, GPIO_INDEX_MIN_VAL));
		return FALSE;
	}

	if (gpio_idx > GPIO_INDEX_MAX_VAL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: gpio_idx value should not be greater than %d!\n",
				__func__, GPIO_INDEX_MAX_VAL));
		return FALSE;
	}

	pch = arg;
	if (pch != NULL)
		gpio_val = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: No parameters for gpio_val!!\n", __func__));
		return FALSE;
	}

	if (gpio_val)
		gpio_val = 1;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: gpio_num:%u Value:%u\n",
			__func__, gpio_idx, gpio_val));

	return SetGpioValue(pAd, gpio_idx, gpio_val);
}
#endif /* WIFI_GPIO_CTRL */

INT SetRxvEnCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
	UINT8 i;
	CHAR *value = 0;
	BOOLEAN fgRxvEnCtrl = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgRxvEnCtrl = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	RxvEnCtrl(pAd, fgRxvEnCtrl);

	return status;
}

INT SetRxvRuCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
	UINT8 i;
	CHAR *value = 0;
	UINT8 u1RxvRuCtrl = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			u1RxvRuCtrl = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	RxvRuCtrl(pAd, u1RxvRuCtrl);

	return status;
}

INT SetRxvInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
	UINT8 band_idx = 0;
	UINT8 i;
	CHAR *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: no parameter.\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":");
		value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			/* 2 symbol representation */
			band_idx = os_str_tol(value, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s(): number of parameters exceed expectation.\n",
				__func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): band_idx: %d\n", __func__, band_idx));

	/* show rxv info */
	chip_show_rxv_info(pAd, band_idx);

	return status;
}

INT SetRxvRawDump(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
	UINT8 band_idx = 0;
	UINT8 i;
	CHAR *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: no parameter.\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":");
		value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			/* 2 symbol representation */
			band_idx = os_str_tol(value, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s(): number of parameters exceed expectation.\n", __func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): band_idx: %d\n", __func__, band_idx));

	/* rxv raw data dump */
	chip_dump_rxv_raw_data(pAd, band_idx);

	return status;
}

INT SetRxvStatReset(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
	UINT8 band_idx = 0;
	UINT8 i;
	CHAR *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: no parameter.\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			/* 2 symbol representation */
			band_idx = os_str_tol(value, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s(): number of parameters exceed expectation.\n", __func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): band_idx: %d\n", __func__, band_idx));

	/* rxv raw data dump */
	chip_reset_rxv_stat(pAd, band_idx);

	return status;
}

INT SetRxvLogCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 ctrl = 0, type_mask = 0;
	UINT8 i;
	CHAR *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: no parameter.\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			ctrl = os_str_tol(value, 0, 10);
			break;
		case 1:
			type_mask = os_str_tol(value, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s(): number of parameters exceed expectation.\n", __func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): ctrl: %d, type_mask: %d\n", __func__, ctrl, type_mask));

	/* rxv log handle */
	switch (ctrl) {
	case RXV_DUMP_START:
		chip_rxv_dump_start(pAd);
		break;
	case RXV_DUMP_STOP:
		chip_rxv_dump_stop(pAd);
		break;
	case RXV_DUMP_ALLOC:
		chip_rxv_dump_buf_alloc(pAd, type_mask);
		break;
	case RXV_DUMP_CLEAR:
		chip_rxv_dump_buf_clear(pAd);
		break;
	case RXV_DUMP_REPORT:
		break;
	default:
		goto error0;
	}

	return TRUE;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): invalid ctrl param(%d)\n", __func__, ctrl));
	return FALSE;
}

INT SetRxvListInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* rxv list info handle */
	chip_rxv_dump_show_list(pAd);

	return TRUE;
}

INT SetRxvRptInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* rxv list info handle */
	chip_rxv_dump_show_rpt(pAd);

	return TRUE;
}

#ifdef LINK_TEST_SUPPORT
INT SetLinkTestRxParamCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 25) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Wrong parameter format!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":");
		value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			/* 2 symbol representation */
			pAd->ucRssiSigniTh = simple_strtol(value, 0, 10);
			break;
		case 1:
			/* 3 symbol representation */
			pAd->cWBRssiTh = simple_strtol(value, 0, 10);
			break;
		case 2:
			/* 3 symbol representation */
			pAd->cIBRssiTh = simple_strtol(value, 0, 10);
			break;
		case 3:
			/* 2 symbol representation */
			pAd->c8RxCountTh = simple_strtol(value, 0, 10);
			break;
		case 4:
			/* 3 symbol representation */
			pAd->ucTimeOutTh = simple_strtol(value, 0, 10);
			break;
		case 5:
			/* 3 symbol representation */
			pAd->cNrRssiTh = simple_strtol(value, 0, 10);
			break;
		case 6:
			/* 3 symbol representation */
			pAd->cChgTestPathTh = simple_strtol(value, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): ucRssiSigniTh: %d, cWBRssiTh: %d, cIBRssiTh: %d\n", __func__,
			pAd->ucRssiSigniTh, pAd->cWBRssiTh, pAd->cIBRssiTh));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): c8RxCountTh: %lld, ucTimeOutTh: %d\n", __func__,
			pAd->c8RxCountTh, pAd->ucTimeOutTh));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): cNrRssiTh: %d, cChgTestPathTh: %d\n", __func__,
			pAd->cNrRssiTh, pAd->cChgTestPathTh));

	return TRUE;
}

INT SetLinkTestModeCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			pAd->fgTxSpurEn = simple_strtol(value, 0, 10);
			break;
		case 1:
			pAd->fgRxSensitEn = simple_strtol(value, 0, 10);
			break;
		case 2:
			pAd->fgACREn = simple_strtol(value, 0, 10);
			break;
		case 3:
			pAd->fgTxSpeEn = simple_strtol(value, 0, 10);
			break;
		case 4:
			pAd->fgRxRcpiEn = simple_strtol(value, 0, 10);
			break;
		case 5:
			pAd->cMaxInRssiTh = simple_strtol(value, 0, 10);
			break;
		case 6:
			pAd->ucACRConfidenceCntTh = simple_strtol(value, 0, 10);
			break;
		case 7:
			pAd->ucMaxInConfidenceCntTh = simple_strtol(value, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgTxSpurEn: %d, fgRxSensitEn: %d, fgACREn: %d, fgTxSpeEn: %d\n", __func__,
			pAd->fgTxSpurEn, pAd->fgRxSensitEn, pAd->fgACREn, pAd->fgTxSpeEn));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgRxRcpiEn: %d, cMaxInRssiTh: %d, ucACRConfidenceCntTh: %d, ucMaxInConfidenceCntTh: %d\n", __func__,
			pAd->fgRxRcpiEn, pAd->cMaxInRssiTh, pAd->ucACRConfidenceCntTh, pAd->ucMaxInConfidenceCntTh));

	return TRUE;
}

INT SetLinkTestPowerUpTblCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value;
	UINT8	ucTxPwrUpCat = 0;
	UINT8	ucTxPwrUpRate = 0;
	UINT8	ucTxPwrUpValue = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			ucTxPwrUpCat = simple_strtol(value, 0, 10);
			break;
		case 1:
			ucTxPwrUpRate = simple_strtol(value, 0, 10);
			break;
		case 2:
			ucTxPwrUpValue = simple_strtol(value, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucTxPwrUpCat: %d, ucTxPwrUpRate: %d, ucTxPwrUpValue: %d\n", __func__,
			ucTxPwrUpCat, ucTxPwrUpRate, ucTxPwrUpValue));

	/* update Tx Power Up Table */
	pAd->ucTxPwrUpTbl[ucTxPwrUpCat][ucTxPwrUpRate] = ucTxPwrUpValue;

	/* sync Tx Power up table to Firmware */
	MtCmdLinkTestTxPwrUpTblCtrl(pAd, ucTxPwrUpCat, pAd->ucTxPwrUpTbl[ucTxPwrUpCat]);

	return TRUE;
}

INT SetLinkTestPowerUpTblInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  ucRateIdx, ucCatIdx;
	CHAR   cRateInfo[CMW_POWER_UP_RATE_NUM][12] = {"CCK_1M2M", "CCK_5M11M", "OFDM_6M9M", "OFDM_12M18M", "OFDM_24M36M", "OFDM_48M", "OFDM_54M",
						       "HT20_MCS0", "HT20_MCS12", "HT20_MCS34", "HT20_MCS5", "HT20_MCS6", "HT20_MCS7"
						      };

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("						Link Test Power Up Table\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("				2G(S)		5G(S)		2G(D)		5G(D)\n"));

	for (ucRateIdx = 0; ucRateIdx < CMW_POWER_UP_RATE_NUM; ucRateIdx++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:  ", cRateInfo[ucRateIdx]));
		for (ucCatIdx = 0; ucCatIdx < CMW_POWER_UP_CATEGORY_NUM; ucCatIdx++)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		%d", pAd->ucTxPwrUpTbl[ucCatIdx][ucRateIdx]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
	}

	return TRUE;
}

INT SetLinkTestInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 u4value;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("								 Link Status\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n\n"));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Link Up Done: %d\n", pAd->fgCmwLinkDone));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ChannelBand: %dG(Band0) %dG(Band1)\n", pAd->ucCmwChannelBand[BAND0] ? 2 : 5, pAd->ucCmwChannelBand[BAND1] ? 2 : 5));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CSD State: %d(Band0) %d (Band1)\n", pAd->ucTxCsdState[BAND0], pAd->ucTxCsdState[BAND1]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Power Up State: %d(Band0) %d(Band1)\n", pAd->ucTxPwrBoostState[BAND0], pAd->ucTxPwrBoostState[BAND1]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RxStream Specific Rx Path Index: %d(Band0) %d(Band1)\n", pAd->ucRxStreamState[BAND0], pAd->ucRxStreamState[BAND1]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Rx Filter State: %d(Band0) %d(Band1)\n", pAd->ucRxFilterstate[BAND0], pAd->ucRxFilterstate[BAND1]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ACR Confidence count and Threshoild: %d(Band0) %d(Band1) %d(Threshold)\n", pAd->ucRxFilterConfidenceCnt[BAND0], pAd->ucRxFilterConfidenceCnt[BAND1], pAd->ucACRConfidenceCntTh));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MaxIn Confidence count and Threshoild: %d(Band0) %d(Band1) %d(Threshold)\n", pAd->ucRxFilterConfidenceCnt[BAND0], pAd->ucRxFilterConfidenceCnt[BAND1], pAd->ucMaxInConfidenceCntTh));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("								Tx Antenna Status\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n\n"));

	/* Read RF CR */
	MtCmdRFRegAccessRead(pAd, (UINT32)WF0, (UINT32)0x48, (UINT32 *)&u4value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WF%d 0x%04x 0x%08x\n", WF0, 0x48, u4value));

	MtCmdRFRegAccessRead(pAd, (UINT32)WF1, (UINT32)0x48, (UINT32 *)&u4value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WF%d 0x%04x 0x%08x\n", WF1, 0x48, u4value));

	MtCmdRFRegAccessRead(pAd, (UINT32)WF2, (UINT32)0x48, (UINT32 *)&u4value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WF%d 0x%04x 0x%08x\n", WF2, 0x48, u4value));

	MtCmdRFRegAccessRead(pAd, (UINT32)WF3, (UINT32)0x48, (UINT32 *)&u4value);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WF%d 0x%04x 0x%08x\n", WF3, 0x48, u4value));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================\n"));

	return TRUE;
}
#endif /* LINK_TEST_SUPPORT */

#ifdef ETSI_RX_BLOCKER_SUPPORT
/* Set fix WBRSSI/IBRSSI pattern */
INT SetFixWbIbRssiCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL,   DBG_SUBCAT_ALL,   DBG_LVL_ERROR,	("%s: No parameters!!\n",  __func__));
		return  FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL,   DBG_SUBCAT_ALL,   DBG_LVL_ERROR,	("%s: Wrong parameter format!!\n",  __func__));
		return  FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
			/* 1 symbol representation */
		case 0:
			pAd->fgFixWbIBRssiEn = simple_strtol(value, 0, 10);
			break;
		default: {
			MTWF_LOG(DBG_CAT_ALL,   DBG_SUBCAT_ALL,   DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n",  __func__));
			return  FALSE;
		}
		}
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Enable CR for RX_BLOCKER: fgFixWbIBRssiEn: %d\n", __func__,
			pAd->fgFixWbIBRssiEn));
	return TRUE;
}
/* Set RSSI threshold */
INT SetRssiThCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 15) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
			/* 3 symbol representation */
		case 0:
			pAd->c1RWbRssiHTh = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 1:
			pAd->c1RWbRssiLTh = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 2:
			pAd->c1RIbRssiLTh = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 3:
			pAd->c1WBRssiTh4R = simple_strtol(value, 0, 10);
			break;

		default: {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: c1RWbRssiHTh: %d, c1RWbRssiLTh: %d, c1RIbRssiLTh: %d,c1WBRssiTh4R: %d\n", __FUNCTION__,
			pAd->c1RWbRssiHTh, pAd->c1RWbRssiLTh, pAd->c1RIbRssiLTh, pAd->c1WBRssiTh4R));

	return TRUE;
}

/* Set fgAdaptRxBlock */
INT SetAdaptRxBlockCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
			/* 3 symbol representation */
		case 0:
			pAd->fgAdaptRxBlock = simple_strtol(value, 0, 10);
			break;
		default: {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgAdaptRxBlock: %d\n", __func__,
			pAd->fgAdaptRxBlock));

	return TRUE;
}

/* Set check counter threshold */
INT SetCheckThCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 11) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
			/* 2 symbol representation */
		case 0:
			pAd->u1CheckTime = simple_strtol(value, 0, 10);
			break;
			/* 2 symbol representation */
		case 1:
			pAd->u1To1RCheckCnt = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 2:
			pAd->u2To1RvaildCntTH = simple_strtol(value, 0, 10);
			break;
		case 3:
			pAd->u2To4RvaildCntTH = simple_strtol(value, 0, 10);
			break;
		default: {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: u1CheckTime: %d, u1To1RCheckCnt: %d, u2To1RvaildCntTH: %d, u2To4RvaildCntTH: %d\n", __func__,
			pAd->u1CheckTime, pAd->u1To1RCheckCnt, pAd->u2To1RvaildCntTH, pAd->u2To4RvaildCntTH));
	return TRUE;

}
/* Set WBRSSI CR */
INT SetWbRssiDirectCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 15) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
			/* 3 symbol representation */
		case 0:
			pAd->c1WbRssiWF0 = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 1:
			pAd->c1WbRssiWF1 = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 2:
			pAd->c1WbRssiWF2 = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 3:
			pAd->c1WbRssiWF3 = simple_strtol(value, 0, 10);
			break;
		default: {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
		}
		}
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: c1WbRssiWF0: %d, c1WbRssiWF1: %d, c1WbRssiWF2: %d, c1WbRssiWF3: %d\n", __func__, pAd->c1WbRssiWF0, pAd->c1WbRssiWF1, pAd->c1WbRssiWF2, pAd->c1WbRssiWF3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	return TRUE;
}

/* Set IBRSSI CR */
INT SetIbRssiDirectCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 15) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
			/* 3 symbol representation */
		case 0:
			pAd->c1IbRssiWF0 = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 1:
			pAd->c1IbRssiWF1 = simple_strtol(value, 0, 10);
			break;
		case 2:
			pAd->c1IbRssiWF2 = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 3:
			pAd->c1IbRssiWF3 = simple_strtol(value, 0, 10);
			break;
		default: {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Number of parameters exceed expectation !!\n", __func__));
			return FALSE;
		}
		}
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: c1IbRssiWF0: %d, c1IbRssiWF1: %d, c1WbRssiWF2: %d, c1IbRssiWF3: %d\n", __func__,
			pAd->c1IbRssiWF0, pAd->c1IbRssiWF1, pAd->c1IbRssiWF2, pAd->c1IbRssiWF3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------\n"));
	return TRUE;
}
#endif /* end ETSI_RX_BLOCKER_SUPPORT */

INT SetMuTxPower(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	  status = TRUE;
	UINT8	i;
	CHAR	 *value = 0;
	BOOLEAN  fgMuTxPwrManEn = FALSE;
	CHAR     cMuTxPwr = 0;
	UINT8    u1BandIdx = 0;
	struct  wifi_dev *wdev;
#if defined(CONFIG_AP_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
#endif /* CONFIG_AP_SUPPORT */
#if defined(CONFIG_AP_SUPPORT)
	UCHAR	  apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	if ((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) {
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		u1BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI) {
		wdev = &pAd->StaCfg[0].wdev;
		u1BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucBandIdx = %d\n", __func__, u1BandIdx));

	/* sanity check for Band index */
	if (u1BandIdx >= DBDC_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgMuTxPwrManEn = os_str_tol(value, 0, 10);
			break;

		case 1:
			cMuTxPwr = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgMuTxPwrManEn: %d, cMuTxPwr: %d\n", __func__,
			fgMuTxPwrManEn, cMuTxPwr));

	return MuPwrCtrlCmd(pAd, fgMuTxPwrManEn, cMuTxPwr, u1BandIdx);
}

INT SetBFNDPATxDCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	  status = TRUE;
	UINT8	i;
	CHAR	 *value = 0;
	BOOLEAN  fgNDPA_ManualMode = FALSE;
	UINT8	ucNDPA_TxMode = 0;
	UINT8	ucNDPA_Rate = 0;
	UINT8	ucNDPA_BW = 0;
	UINT8	ucNDPA_PowerOffset = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgNDPA_ManualMode = os_str_tol(value, 0, 10);
			break;

		case 1:
			ucNDPA_TxMode = os_str_tol(value, 0, 10);
			break;

		case 2:
			ucNDPA_Rate = os_str_tol(value, 0, 10);
			break;

		case 3:
			ucNDPA_BW = os_str_tol(value, 0, 10);
			break;

		case 4:
			ucNDPA_PowerOffset = os_str_tol(value, 0, 10); /* negative value need to use 2's complement */
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("%s: fgNDPA_ManualMode = %d, ucNDPA_TxMode = %d, ucNDPA_Rate = %d, ucNDPA_BW = %d, ucNDPA_PowerOffset = %d\n",
		  __func__, fgNDPA_ManualMode, ucNDPA_TxMode, ucNDPA_Rate, ucNDPA_BW, ucNDPA_PowerOffset));
	return BFNDPATxDCtrlCmd(pAd, fgNDPA_ManualMode, ucNDPA_TxMode, ucNDPA_Rate, ucNDPA_BW, ucNDPA_PowerOffset);
}

INT SetTxPowerCompInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	 status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	   apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */
	UINT8  ucPowerTableIdx;
	CHAR   *STRING[SKU_TABLE_SIZE] = {
		"CCK_1M2M   ", "CCK5M11M   ", "OFDM6M9M   ", "OFDM12M18M ",	"OFDM24M36M ", "OFDM48M	", "OFDM54M	",
		"HT20M0	 ", "HT20M32	", "HT20M1M2   ", "HT20M3M4   ", "HT20M5	 ", "HT20M6	 ", "HT20M7	 ",
		"HT40M0	 ", "HT40M32	", "HT40M1M2   ", "HT40M3M4   ",	"HT40M5	 ", "HT40M6	 ", "HT40M7	 ",
		"VHT20M0	", "VHT20M1M2  ", "VHT20M1M2  ", "VHT20M5M6  ", "VHT20M7	", "VHT20M8	", "VHT20M9	",
		"VHT40M0	", "VHT40M1M2  ", "VHT40M3M4  ", "VHT40M5M6  ", "VHT40M7	", "VHT40M8	", "VHT40M9	",
		"VHT80M0	", "VHT80M1M2  ", "VHT80M3M4  ", "VHT80M5M6  ", "VHT80M7	", "VHT80M8	", "VHT80M9	",
		"VHT160M0   ", "VHT160M1M2 ", "VHT160M3M4 ", "VHT160M5M6 ", "VHT160M7   ", "VHT160M8   ", "VHT160M9   "
	};

#ifdef CONFIG_AP_SUPPORT
	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	ucBandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	wdev = &pAd->StaCfg[0].wdev;
	ucBandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_STA_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ucBandIdx = %d\n", __func__, ucBandIdx));

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("=============================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("                       Tx Power Compenstation Info                 \n"));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("=============================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 (KGRN "         Band%d        (1SS, 2SS, 3SS, 4SS)                    \n" KNRM, ucBandIdx));

	for (ucPowerTableIdx = 0; ucPowerTableIdx < SKU_TABLE_SIZE; ucPowerTableIdx++) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         %s : %3d, %3d, %3d, %3d                               \n",
			  &STRING[ucPowerTableIdx][0],
			  pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucPowerTableIdx][0],
			  pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucPowerTableIdx][1],
			  pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucPowerTableIdx][2],
			  pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucPowerTableIdx][3]));
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("-----------------------------------------------------------------------------\n"));

	return status;
}

/* Manually setting Tx power on a,g,n*/
INT SetTxPwrManualCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR		*value = 0;
	UCHAR		ucBandIdx = 0;
	struct		wifi_dev *wdev;
	INT32		Ret;
	UINT8		ParamIdx;
	INT_8		cTxPower = 0;
	UINT8		ucPhyMode = 0;
	UINT8		ucTxRate = 0;
	UINT8		ucBW = 0;

#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	   apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

	/* obtain Band index */
#ifdef CONFIG_AP_SUPPORT
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	ucBandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	wdev = &pAd->StaCfg[0].wdev;
	ucBandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_STA_SUPPORT */

	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No Parameters !! \n", __FUNCTION__));
		goto err1;
	}

	/* Sanity check for input parameter format */
	if (strlen(arg) != 11) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong Parameter Format !!\n", __FUNCTION__));
		goto err1;
	}

	/* Parsing input parameter */
	for (ParamIdx = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), ParamIdx++) {
		switch (ParamIdx) {
		case 0:
			ucPhyMode = simple_strtol(value, 0, 10); /* 2-bit format */
			break;
		case 1:
			ucTxRate = simple_strtol(value, 0, 10);  /* 2-bit format */
			break;
		case 2:
			ucBW = simple_strtol(value, 0, 10);      /* 2-bit format */
			break;
		case 3:
			cTxPower = simple_strtol(value, 0, 10);  /* 2-bit format */
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Set Too Much Parameters !!\n", __func__));
			goto err1;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: Band(%d), TxMode(%d), MCS(%d), BW(%d), TxPower(%d)\n",
		__func__, ucBandIdx, ucPhyMode, ucTxRate, ucBW, cTxPower));

	/* Command Handler for Force Power Control */
	Ret = TxPowerManualCtrl(pAd, ucBandIdx, cTxPower, ucPhyMode, ucTxRate, ucBW);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
	err1:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "Please input parameter via format \"Phymode:TxRate:BW:TxPower\"\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "Phymode:\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "    (2-digit) 0: CCK, 1: OFDM, 2: HT, 3: VHT, 4: HESU\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "TxRate:\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "    (2-digit) CCK: 00~03, OFDM: 00~07, HT: 00~07, VHT: 00~09, HESU:00~11 \n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "BW:\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "    (2-digit) 0: BW20, 1: BW40, 2: BW80, 3:BW160\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "    (2-digit) 0: HE26, 1: HE52, 2: HE106, 3:HE242, 4:HE484, 5:HE996, 6:HE996X2\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "TxPower:\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "    (2-digit) absolute Tx power (unit: 0.5dB)\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KRED "Ex: iwpriv ra0 set TxPwrManualSet=02:00:00:16\n" KNRM));
		return FALSE;
}

INT SetTpcManCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = NULL;
	BOOLEAN fgTpcManual = FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgTpcManual = (BOOLEAN)os_str_tol(value, 0, 10);
			break;

		default: {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			return FALSE;
		}
		}
	}
	return TpcManCtrl(pAd, fgTpcManual);
}

INT SetTpcEnable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = NULL;
	BOOLEAN fgTpcEnable = FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgTpcEnable = (BOOLEAN)os_str_tol(value, 0, 10);
			break;

		default: {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			return FALSE;
		}
		}
	}
	return TpcEnableCfg(pAd, fgTpcEnable);
}

INT SetTpcWlanIdCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = NULL;
	BOOLEAN	fgUplink = FALSE;
	UINT8	u1EntryIdx = 0;
	UINT16	u2WlanId = 0;
	UINT8	u1DlTxType = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgUplink = (BOOLEAN)os_str_tol(value, 0, 10);
			break;
		case 1:
			u1EntryIdx = os_str_tol(value, 0, 10);
			if (u1EntryIdx < 0 || u1EntryIdx > 31) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Set wrong parameters! Entry index should be between 0~31 .\n", __func__));
				return FALSE;
			}
			break;
		case 2:
			u2WlanId = os_str_tol(value, 0, 10);
			break;
		case 3:
			u1DlTxType = os_str_tol(value, 0, 10);
			if (u1DlTxType != 0 && u1DlTxType != 1) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Set wrong parameters! Down-link Tx Type 0: MU-MIMO, 1: OFDMA. \n", __func__));
				return FALSE;
			}
			break;
		default: {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			return FALSE;
		}
		}
	}
	return TpcWlanIdCtrl(pAd, fgUplink, u1EntryIdx, u2WlanId, u1DlTxType);
}

INT SetTpcUlAlgoCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	UINT8	u1para[4] = {0};
	CHAR	*value = NULL;
	UINT8	u1TpcCmd = 0;
	UINT8	u1ApTxPwr = 0;
	UINT8	u1EntryIdx = 0;
	UINT8	u1TargetRssi = 0;
	UINT8	u1UPH = 0;
	BOOLEAN	fgMinPwrFlag = FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++)
		u1para[i] = os_str_tol(value, 0, 10);

	u1TpcCmd = u1para[0];
	switch (u1TpcCmd) {
	case 0:
		if (u1para[1] < 0 || u1para[1] > 60) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Set wrong parameters! Ap Tx Power should be between 0~60. \n", __func__));
			return FALSE;
		}
		u1ApTxPwr = u1para[1];
		break;
	case 1:
		u1EntryIdx = u1para[1];
		if (u1EntryIdx < 0 || u1EntryIdx > 31) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Set wrong parameters! Entry index should be between 0~31 .\n", __func__));
			return FALSE;
		}
		u1TargetRssi = u1para[2];
		if (u1TargetRssi < 0 || u1TargetRssi > 90) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Set wrong parameters! Target TSSI be between 0~90. \n", __func__));
			return FALSE;
		}
		break;
	case 2:
		u1EntryIdx = u1para[1];
		if (u1EntryIdx < 0 || u1EntryIdx > 31) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Set wrong parameters! Entry index should be between 0~31 .\n", __func__));
			return FALSE;
		}
		u1UPH = u1para[2];
		if (u1UPH < 0 || u1UPH > 31) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Set wrong parameters! Uplink Power Headroom should be between 0~31.\n", __func__));
			return FALSE;
		}
		fgMinPwrFlag = u1para[3];
		break;
	default: {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: set wrong parameters\n", __func__));
		return FALSE;
	}
	}
	return TpcUlAlgoCtrl(pAd, u1TpcCmd, u1ApTxPwr, u1EntryIdx, u1TargetRssi, u1UPH, fgMinPwrFlag);
}

INT SetTpcDlAlgoCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	INT16	para[4];
	CHAR	*value = NULL;
	UINT8	u1TpcCmd = 0;
	BOOLEAN fgCmdCtrl = FALSE;
	UINT8	u1DlTxType = 0;
	CHAR	DlTxPwr = 0;
	UINT8	u1EntryIdx = 0;
	INT16	DlTxpwrAlpha = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++)
		para[i] = os_str_tol(value, 0, 10);

	u1TpcCmd = (UINT8)para[0];
	switch (u1TpcCmd) {
	case 0:
		if (para[1] != 0 && para[1] != 1) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Set wrong parameters! Enable command control enable=1, disable=0.\n", __func__));
			return FALSE;
		}
		fgCmdCtrl = (BOOLEAN)para[1];
		break;
	case 1:
		if (para[1] != 0 && para[1] != 1) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Set wrong parameters! Down-link Tx Type 0: MU-MIMO, 1: OFDMA. \n", __func__));
			return FALSE;
		}
		u1DlTxType = (UINT8)para[1];
		if (para[2] < -128 || para[2] > 127) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Set wrong parameters! Down-link Tx Power should between -127~128. \n", __func__));
			return FALSE;
		}
		DlTxPwr = (CHAR)para[2];
		break;
	case 2:
		if (para[1] < 0 || para[1] > 31) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Set wrong parameters! Entry index should be between 0~31 .\n", __func__));
			return FALSE;
		}
		u1EntryIdx = (UINT8)para[1];
		if (para[2] != 0 && para[2] != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Set wrong parameters! Down-link Tx Type 0: MU-MIMO, 1: OFDMA. \n", __func__));
			return FALSE;
		}
		u1DlTxType = (UINT8)para[2];
		if (para[3] < -256 || para[3] > 255) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Set wrong parameters! Down-link Tx Alpha Power should between -256~255. \n", __func__));
			return FALSE;
		}
		DlTxpwrAlpha = para[3];
		break;
	default: {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: set wrong parameters\n", __func__));
		return FALSE;
	}
	}
	return TpcDlAlgoCtrl(pAd, u1TpcCmd, fgCmdCtrl, u1DlTxType, DlTxPwr, u1EntryIdx, DlTxpwrAlpha);
}

INT SetTpcManTblInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = NULL;
	BOOLEAN	fgUplink = FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgUplink = (BOOLEAN)os_str_tol(value, 0, 10);
			break;

		default: {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			return FALSE;
		}
		}
	}
	return TpcManTblInfo(pAd, fgUplink);
}

INT SetTpcAlgoUlUtCfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	INT16	i2para[4] = {0};
	CHAR	*value = NULL;
	UINT8	u1EntryIdx = 0;
	UINT8	u1VarType = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++)
		i2para[i] = os_str_tol(value, 0, 10);

	u1EntryIdx = (UINT8)i2para[0];
	u1VarType = (UINT8)i2para[1];

	if (u1EntryIdx < 0 || u1EntryIdx > 31) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: Set wrong parameters! Entry index should be between 0~31 .\n", __func__));
		return FALSE;
	}
	return TpcUlUtVarCfg(pAd, u1EntryIdx, u1VarType, i2para[2]);
}

INT SetTpcAlgoUlUtGo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = NULL;
	BOOLEAN fgTpcUtGo = FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}
	/*  Note:  u1VarType:
	ENUM {
	(i2) LastRssi_dBm = 0,
	(i2) LastPowerHeadroom_dB  = 1,
	(fg) MinTxPwr =2,
	(fg) HaveHitMinTxPwrFg  = 3,
	(i2) MinRssi_dBm =4,
	(i2) RssiOffset_dB = 5,
	(u2) LastRuAllocIdx =6,
	(u1) LastTxMcs =7,
	(u2) CurrRuAllocIdx =8,
	(u1) CurrTxMcs =9,
	(u1) TotTrigUser =10 } */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgTpcUtGo = (BOOLEAN)os_str_tol(value, 0, 10);
			break;

		default: {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			return FALSE;
		}
		}
	}
	return TpcAlgoUtGo(pAd, fgTpcUtGo);
}

INT SetThermalManCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 u1BandIdx = 0;
	BOOLEAN fgManualMode = FALSE;
	UINT8 u1ThermalAdc = 0;
	INT i4status = TRUE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters !!\n", __func__));
		return FALSE;
	}

	/* sanity check for input parameter format */
	if (strlen(arg) != 7) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format !!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":"); pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			u1BandIdx = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 1:
			/* 1 symbol representation */
			fgManualMode = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 2:
			/* 3 symbol representation */
			u1ThermalAdc = os_str_tol(pi1Buffer, 0, 10);
			break;

		default: {
			i4status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameters !!\n", __func__));
			break;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): u1BandIdx: %d, fgManualMode: %d, u1ThermalAdc: %d\n", __func__, u1BandIdx, fgManualMode, u1ThermalAdc));

	/* handler */
	i4status = ThermalManCtrl(pAd, u1BandIdx, fgManualMode, u1ThermalAdc);

	return i4status;
}

INT SetThermalTaskInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i4Status = TRUE;
#if defined(MT7615) || defined(MT7622)
	return i4Status;
#else
	UINT8 u1BandIdx = 0;
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;

		/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters !!\n", __func__));
		return FALSE;
	}

	/* sanity check for input parameter format */
	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format !!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":"); pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			u1BandIdx = os_str_tol(pi1Buffer, 0, 10);
			break;

		default: {
			i4Status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameters !!\n", __func__));
			break;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): u1BandIdx: %d\n", __func__, u1BandIdx));

	/* handler */
	i4Status = ThermalBasicInfo(pAd, u1BandIdx);

	return i4Status;
#endif /* defined(MT7615) || defined(MT7622) */
}

INT SetThermalTaskCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 u1BandIdx = 0;
	BOOLEAN fgTrigEn = FALSE;
	UINT8 u1Thres = 0;
	INT i4status = TRUE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters !!\n", __func__));
		return FALSE;
	}

	/* sanity check for input parameter format */
	if (strlen(arg) != 7) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format !!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":"); pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			u1BandIdx = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 1:
			/* 3 symbol representation */
			u1Thres = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 2:
			/* 1 symbol representation */
			fgTrigEn = os_str_tol(pi1Buffer, 0, 10);
			break;

		default: {
			i4status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameters !!\n", __func__));
			break;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): u1BandIdx: %d, fgTrigEn: %d, u1Thres: %d\n", __func__, u1BandIdx, fgTrigEn, u1Thres));

	/* handler */
	i4status = ThermalTaskCtrl(pAd, u1BandIdx, fgTrigEn, u1Thres);

	return i4status;
}

INT SetThermalProtectEnable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 band_idx = 0, protection_type = 0, trigger_type = 0;
	INT32 trigger_temp = 0, restore_temp = 0;
	UINT16 recheck_time = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: No parameters !!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":");
		pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			band_idx = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 1:
			/* 1 symbol representation */
			protection_type = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 2:
			/* 1 symbol representation */
			trigger_type = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 3:
			/* 3 symbol representation */
			trigger_temp = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 4:
			/* 3 symbol representation */
			restore_temp = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 5:
			/* 4 symbol representation */
			recheck_time = os_str_tol(pi1Buffer, 0, 10);
			break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: wrong param format.\n", __func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): band_idx: %d, protection_type: %d, trigger_type: %d\n",
		__func__, band_idx, protection_type, trigger_type));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): trigger_temp: %d, restore_temp: %d, recheck_time: %d\n",
		__func__, trigger_temp, restore_temp, recheck_time));

	/* handler */
	MtCmdThermalProtectEnable(pAd, band_idx, protection_type,
		trigger_type, trigger_temp, restore_temp, recheck_time);

	return TRUE;
}

INT SetThermalProtectDisable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 band_idx = 0, protection_type = 0, trigger_type = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: No parameters !!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":");
		pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			band_idx = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 1:
			/* 1 symbol representation */
			protection_type = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 2:
			/* 1 symbol representation */
			trigger_type = os_str_tol(pi1Buffer, 0, 10);
			break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: wrong param format.\n", __func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): band_idx: %d, protection_type: %d, trigger_type: %d\n",
		__func__, band_idx, protection_type, trigger_type));

	/* handler */
	MtCmdThermalProtectDisable(pAd, band_idx,
		protection_type, trigger_type);

	return TRUE;
}

INT SetThermalProtectDutyCfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 band_idx = 0, level_idx = 0,  duty = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: No parameters !!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":");
		pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			band_idx = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 1:
			/* 1 symbol representation */
			level_idx = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 2:
			/* 1 symbol representation */
			duty = os_str_tol(pi1Buffer, 0, 10);
			break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: wrong param format.\n", __func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): band_idx: %d, level_idx: %d, duty: %d\n",
		__func__, band_idx, level_idx, duty));

	/* handler */
	MtCmdThermalProtectDutyCfg(pAd, band_idx, level_idx, duty);

	return TRUE;
}

INT SetThermalProtectInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 band_idx = 0;
	struct THERMAL_PROTECT_MECH_INFO info_buf;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: No parameters !!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":");
		pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			band_idx = os_str_tol(pi1Buffer, 0, 10);
			break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: wrong param format.\n", __func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): band_idx: %d\n",
		__func__, band_idx));

	/* handler */
	MtCmdThermalProtectInfo(pAd, band_idx, &info_buf);

	return TRUE;
}

INT SetThermalProtectDutyInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 band_idx = 0;
	struct THERMAL_PROTECT_DUTY_INFO info_buf;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: No parameters !!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":");
		pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			band_idx = os_str_tol(pi1Buffer, 0, 10);
			break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: wrong param format.\n", __func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): band_idx: %d\n",
		__func__, band_idx));

	/* handler */
	MtCmdThermalProtectDutyInfo(pAd, band_idx, &info_buf);

	return TRUE;
}

INT SetThermalProtectStateAct(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 band_idx = 0, protect_type = 0;
	UINT8 trig_type = 0, state = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: No parameters !!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":");
		pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			band_idx = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 1:
			/* 1 symbol representation */
			protect_type = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 2:
			/* 1 symbol representation */
			trig_type = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 3:
			/* 1 symbol representation */
			state = os_str_tol(pi1Buffer, 0, 10);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: wrong param format.\n", __func__));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): band_idx: %d, protect_type: %d\n",
		__func__, band_idx, protect_type));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): trig_type: %d, state: %d\n",
		__func__, trig_type, state));

	/* handler */
	MtCmdThermalProtectStateAct(pAd, band_idx,
		protect_type, trig_type, state);

	return TRUE;
}

#ifdef TX_POWER_CONTROL_SUPPORT
INT SetTxPowerBoostCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = 0;
	UINT8	u1PwrUpCat = 0, u1LenVal, u1PwrBoost;
	CHAR	cPwrUpValue[POWER_UP_CATEGORY_RATE_NUM] = {0};
	UINT8	ucBandIdx = 0;
	UINT8	u1Bw, u1PhyMode;

	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	value = strsep(&arg, ":");
	if (value == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters: Please enter band index\n", __func__));
		return -1;
	}
	ucBandIdx = simple_strtol(value, 0, 10);

	/* sanity check for Band index */
	if (ucBandIdx >= DBDC_BAND_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters: Band Index\n", __func__));
		return FALSE;
	}

	value = strsep(&arg, ":");
	if (value == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters: Please enter PhyMode\n", __func__));
		return -1;
	}
	u1PhyMode = simple_strtol(value, 0, 10);

	value = strsep(&arg, ":");
	if (value == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters: Please enter bandwidth\n", __func__));
		return -1;
	}
	u1Bw = simple_strtol(value, 0, 10);

	if (arch_ops && arch_ops->arch_txpower_boost_power_cat_type) {
		if (!arch_ops->arch_txpower_boost_power_cat_type(pAd, u1PhyMode, u1Bw, &u1PwrUpCat))
			return FALSE;
	}

	value = strsep(&arg, ":");
	if (value == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters: Please enter length of power boost values\n", __func__));
		return -1;
	}
	u1LenVal  = simple_strtol(value, 0, 10);

	for (i = 0; i < u1LenVal; i++) {
		if ((i + 1) == u1LenVal)
			value = strsep(&arg, "");
		else
			value = strsep(&arg, ":");

		if (value == NULL) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters: Please enter Power boost value \n", __func__));
			return -1;
		}

		u1PwrBoost = simple_strtol(value, 0, 10);

		if (u1PwrBoost < cap->single_sku_fill_tbl_length[u1PwrUpCat])
			cPwrUpValue[i] = u1PwrBoost;
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters: Power boost value should not be larger than Max value %d\n", __func__, cap->single_sku_fill_tbl_length[u1PwrUpCat]));
			return -1;
		}

	}

	if (arch_ops && arch_ops->arch_txpower_boost_ctrl)
		arch_ops->arch_txpower_boost_ctrl(pAd, ucBandIdx, u1PwrUpCat, cPwrUpValue);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: cPwrUpValue: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
			 __func__, cPwrUpValue[0], cPwrUpValue[1],
			 cPwrUpValue[2], cPwrUpValue[3], cPwrUpValue[4],
			 cPwrUpValue[5], cPwrUpValue[6], cPwrUpValue[7],
			 cPwrUpValue[8], cPwrUpValue[9], cPwrUpValue[10],
			 cPwrUpValue[11]));

	return TxPwrUpCtrl(pAd, ucBandIdx, u1PwrUpCat, &cPwrUpValue[0], POWER_UP_CATEGORY_RATE_NUM);
}
#endif /* TX_POWER_CONTROL_SUPPORT */

INT SetCalFreeApply(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	 status = TRUE;
	UINT8   i;
	CHAR	*value = 0;
	UCHAR   CalFreeApply = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!!\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like X (X = 0,1)!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			CalFreeApply = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: CalFreeApply = %d\n", __func__, CalFreeApply));
	/* Configure to Global pAd structure */
	pAd->fgCalFreeApply = CalFreeApply;
	return status;
}

INT SetWriteEffuseRFpara(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
	UCHAR   block[EFUSE_BLOCK_SIZE] = "";
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	USHORT  length = cap->EEPROM_DEFAULT_BIN_SIZE;
	UCHAR   *ptr = pAd->EEPROMImage;
	UCHAR   index, i;
	USHORT  offset = 0;
	UINT	isVaild = 0;
	BOOL	NeedWrite;
	BOOL	WriteStatus;

	/* Only Write to Effuse when RF is not lock down */
	if (!chip_check_rf_lock_down(pAd)) {
		/* Write to Effuse block by block */
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		for (offset = 0; offset < length; offset += EFUSE_BLOCK_SIZE) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("offset 0x%04x:\n", offset));
			NeedWrite = FALSE;
			MtCmdEfuseAccessRead(pAd, offset, &block[0], &isVaild);

			/* Check the Needed contents are different and update the buffer content for write back to Effuse */
			for (index = 0; index < EFUSE_BLOCK_SIZE; index++) {
				/* Obtain the status of this effuse column need to write or not */
				WriteStatus = ops->write_RF_lock_parameter(pAd, offset + index);
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Effuse[0x%04x]: Write(%d)\n", offset + index, WriteStatus));

				if ((block[index] != ptr[index]) && (WriteStatus == TRUE))
					NeedWrite = TRUE;
				else
					continue;

				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("index 0x%04x: ", offset + index));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("orignal block value=0x%04x, write value=0x%04x\n", block[index],
						ptr[index]));

				if (WriteStatus == TRUE)
					block[index] = ptr[index];
			}

			/* RF Lock Protection */
			if (offset == RF_LOCKDOWN_EEPROME_BLOCK_OFFSET) {
				block[RF_LOCKDOWN_EEPROME_COLUMN_OFFSET] |= RF_LOCKDOWN_EEPROME_MASK;
				NeedWrite = TRUE;
			}

			/* Only write to Effuse when Needed contents are different in Effuse and Flash */
			if (NeedWrite == TRUE) {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("write block content: "));

				for (i = 0; i < EFUSE_BLOCK_SIZE; i++)
					MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%x ", (UINT)block[i]));

				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
				MtCmdEfuseAccessWrite(pAd, offset, &block[0]);
			}

			ptr += EFUSE_BLOCK_SIZE;
		}
	} else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RF is lock now. Cannot write back to Effuse!!\n"));

	return status;
}

INT SetRFBackup(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	 status = TRUE;
	CHAR	*value = 0;
	UCHAR   Param  = 0;
	UCHAR   block[EFUSE_BLOCK_SIZE] = "";
	UCHAR   i;
	USHORT  offset = RF_LOCKDOWN_EEPROME_BLOCK_OFFSET;
	UINT	isVaild = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!!\n", __func__));
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			Param = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __func__));
			break;
		}
		}
	}

	if (Param == RF_VALIDATION_NUMBER) {
		/* Check RF lock down status */
		if (chip_check_rf_lock_down(pAd)) {
			/* Read Effuse Contents of Block Address 0x120 */
			MtCmdEfuseAccessRead(pAd, offset, &block[0], &isVaild);
			/* Configue Block 0x12C Content (Unlock RF lock) */
			block[RF_LOCKDOWN_EEPROME_COLUMN_OFFSET] &= (~(RF_LOCKDOWN_EEPROME_MASK));
			/* Write to Effuse */
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("write block content: "));

			for (i = 0; i < EFUSE_BLOCK_SIZE; i++)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%x ", (UINT)block[i]));

			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
			MtCmdEfuseAccessWrite(pAd, offset, &block[0]);
		} else
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("No need to unlock!!\n"));
	}

	return status;
}

INT set_hnat_register(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 reg_en;
	INT idx;
	struct wifi_dev *wdev;

	reg_en = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Device Instance\n"));

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\tWDEV %02d:", idx));

		if (pAd->wdev_list[idx]) {
			wdev = pAd->wdev_list[idx];
			if (wdev->if_up_down_state) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\n\t\tName:%s\n",
					RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev)));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\t\tWdev(list) Idx:%d\n", wdev->wdev_idx));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\t\t Idx:%d\n", RtmpOsGetNetIfIndex(wdev->if_dev)));
#if defined(CONFIG_FAST_NAT_SUPPORT)

			if (ppe_dev_unregister_hook != NULL &&
				ppe_dev_register_hook != NULL) {
					if (reg_en)
						ppe_dev_register_hook(wdev->if_dev);
					else
						ppe_dev_unregister_hook(wdev->if_dev);
				}
#endif /*CONFIG_FAST_NAT_SUPPORT*/
			}
		} else
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\n"));
	}

	return TRUE;
}

INT Set_MibBucket_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR  MibBucketEnable;
	UCHAR	   concurrent_bands = HcGetAmountOfBand(pAd);
	UCHAR i = 0;

	MibBucketEnable = os_str_tol(arg, 0, 10);

	/* MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()  BandIdx=%d, MibBucket Enable=%d\n", __func__, BandIdx, MibBucketEnable)); */
	for (i = 0; i < concurrent_bands; i++)
		pAd->OneSecMibBucket.Enabled[i] = MibBucketEnable;

	pAd->MsMibBucket.Enabled = MibBucketEnable;
	return TRUE;
}

#ifdef PKT_BUDGET_CTRL_SUPPORT
INT Set_PBC_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	 *value = 0;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Set PBC Up bound:\n"));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		pAd->pbc_bound[i] = os_str_tol(value, 0, 10);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%d: %d\n", i, pAd->pbc_bound[i]));
	}

	return TRUE;
}
#endif /*PKT_BUDGET_CTRL_SUPPORT*/

void hc_show_radio_info(struct _RTMP_ADAPTER *ad);
void hc_show_hdev_obj(struct wifi_dev *wdev);

/*dump radio information*/
INT show_radio_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	CHAR str[32] = "";
#ifdef DOT11_N_SUPPORT
	CHAR str2[32] = "";
#endif /*DOT11_N_SUPPORT*/
	CHAR *pstr = NULL;
	UCHAR i;
#ifdef TR181_SUPPORT
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
	ULONG TNow;
	UINT32	Time, TimeDelta;
#endif /*TR181_SUPPORT*/

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==========BBP radio information==========\n"));
#ifdef DBDC_MODE
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DBDCEn\t: %s\n",
			(pAd->CommonCfg.dbdc_mode) ? "Enable" : "Disable"));
#endif /*DBDC_MODE*/
#ifdef TR181_SUPPORT
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("------------Band0---------- \n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel Busy Time(11k scale for last 100ms):%d\n", pAd->Ch_BusyTime_11k[DBDC_BAND0]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel Busy Time:%d, MeasurementDur:%d usec\n",
						pAd->Ch_BusyTime[DBDC_BAND0], pAd->ChannelStats.MeasurementDuration));
	NdisGetSystemUpTime(&TNow);
	Time = jiffies_to_usecs(TNow);
	TimeDelta = Time - ctrl->rdev[DBDC_BAND0].pRadioCtrl->CurChannelUpTime;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel:%d Channel Up time:%u usec\n", ctrl->rdev[DBDC_BAND0].pRadioCtrl->Channel, TimeDelta));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("TotalChannelChangeCount:%d\n", (ctrl->rdev[DBDC_BAND0].pRadioCtrl->TotalChannelChangeCount +
																	pAd->ApBootACSChannelChangePerBandCount[DBDC_BAND0])));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ManualChannelChangeCount:%d\n", ctrl->rdev[DBDC_BAND0].pRadioCtrl->ManualChannelChangeCount));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("DFSTriggeredChannelChangeCount:%d\n",
									ctrl->rdev[DBDC_BAND0].pRadioCtrl->DFSTriggeredChannelChangeCount));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApBootACSChannelChangeCount:%d\n", pAd->ApBootACSChannelChangePerBandCount[DBDC_BAND0]));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ForceACSChannelChangeCount:%d\n",
									ctrl->rdev[DBDC_BAND0].pRadioCtrl->ForceACSChannelChangeCount));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RefreshACSChannelChangeCount:%d\n",
									ctrl->rdev[DBDC_BAND0].pRadioCtrl->RefreshACSChannelChangeCount));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------- \n"));
#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("------------Band1---------- \n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel Busy Time(11k scale for last 100ms):%d\n", pAd->Ch_BusyTime_11k[DBDC_BAND1]));
		TimeDelta = Time - ctrl->rdev[DBDC_BAND1].pRadioCtrl->CurChannelUpTime;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel:%d Channel Up time:%u usec\n", ctrl->rdev[DBDC_BAND1].pRadioCtrl->Channel, TimeDelta));

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("TotalChannelChangeCount:%d\n", (ctrl->rdev[DBDC_BAND1].pRadioCtrl->TotalChannelChangeCount +
																	pAd->ApBootACSChannelChangePerBandCount[DBDC_BAND1])));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ManualChannelChangeCount:%d\n", ctrl->rdev[DBDC_BAND1].pRadioCtrl->ManualChannelChangeCount));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("DFSTriggeredChannelChangeCount:%d\n",
									ctrl->rdev[DBDC_BAND1].pRadioCtrl->DFSTriggeredChannelChangeCount));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApBootACSChannelChangeCount:%d\n", pAd->ApBootACSChannelChangePerBandCount[DBDC_BAND1]));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ForceACSChannelChangeCount:%d\n",
									ctrl->rdev[DBDC_BAND1].pRadioCtrl->ForceACSChannelChangeCount));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RefreshACSChannelChangeCount:%d\n",
									ctrl->rdev[DBDC_BAND1].pRadioCtrl->RefreshACSChannelChangeCount));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------- \n"));
	}
#endif /*DBDC_MODE*/
#endif /*TR181_SUPPORT*/

	/*show radio info per band*/
	hc_show_radio_info(pAd);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("##########WDEV radio information##########\n"));

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev) {
			UCHAR cfg_ext_cha = wlan_config_get_ext_cha(wdev);
			UCHAR op_ext_cha = wlan_operate_get_ext_cha(wdev);

			pstr = wdev_type2str(wdev->wdev_type);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("==========wdev(%d)==========\n", i));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("type\t: %s\n", pstr));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("fun_idx\t: %d\n", wdev->func_idx));
			pstr = wmode_2_str(wdev->PhyMode);

			if (pstr != NULL) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wmode\t: %s\n", pstr));
				os_free_mem(pstr);
			}

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("channel\t: %d\n", wlan_operate_get_prim_ch(wdev)));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("ch band\t: %d\n", wlan_operate_get_ch_band(wdev)));
#ifdef DOT11_N_SUPPORT

			if (WMODE_CAP_N(wdev->PhyMode)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cen_ch1\t: %d\n", wlan_operate_get_cen_ch_1(wdev)));
				bw_2_str(wlan_config_get_ht_bw(wdev), str);
				bw_2_str(wlan_operate_get_ht_bw(wdev), str2);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ht_bw\t: (%s,%s)\n", str, str2));
				extcha_2_str(cfg_ext_cha, str);
				extcha_2_str(op_ext_cha, str2);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ext_ch\t: (%s,%s)\n", str, str2));
			}

#ifdef DOT11_VHT_AC

			if (WMODE_CAP_AC(wdev->PhyMode)) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cen_ch2\t: (%d,%d)\n",
						wlan_config_get_cen_ch_2(wdev), wlan_operate_get_cen_ch_2(wdev)));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("vht_bw\t: (%d,%d)\n",
						wlan_config_get_vht_bw(wdev), wlan_operate_get_vht_bw(wdev)));
			}

#endif /*DOT11_VHT_AC*/
#endif /*DOT11_N_SUPPORT*/
			bw_2_str(wlan_operate_get_bw(wdev), str);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bw\t: %s\n", str));
			/*hdev related*/
			hc_show_hdev_obj(wdev);
		}
	}

	return TRUE;
}

INT set_tx_amsdu(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ctrl);
	UINT32 wcid, type, amsdu_fix_num, len;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(ctrl);

	if (arg == NULL || strlen(arg) == 0)
		goto error;

	if (sscanf(arg, "%d-%d-%d-%d", &wcid, &type, &amsdu_fix_num, &len) != 4)
		goto error;

	if (type < 0 || type >= 2) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wrong type(%d), please input 0: TX_SW_AMSDU, 1: TX_HW_AMSDU\n", type));
		return FALSE;
	}

	if ((type == TX_HW_AMSDU) && !IS_ASIC_CAP(pAd, fASIC_CAP_HW_TX_AMSDU)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wrong type(%d), chip do not have TX_HW_AMSDU cap, please input 0: TX_SW_AMSDU\n", type));
		return FALSE;
	}

	if (amsdu_fix_num < 0 || amsdu_fix_num > cap->hw_max_amsdu_nums) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("amsdu num is out of range(%d), chip cap(%d)\n", amsdu_fix_num, cap->hw_max_amsdu_nums));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("wcid = %d, type: %d, nums  = %d, len = %d(unit: 256bytes)\n", wcid, type, amsdu_fix_num, len));

	if (type == TX_SW_AMSDU) {
		/* SW AMSDU */
		tr_ctl->amsdu_type = TX_SW_AMSDU;
		tr_ctl->amsdu_fix_num = amsdu_fix_num;

		if (tr_ctl->amsdu_fix_num > 0)
			tr_ctl->amsdu_fix = TRUE;
		else
			tr_ctl->amsdu_fix = FALSE;

	} else if (type == TX_HW_AMSDU) {
		/* HW AMSDU */
		tr_ctl->amsdu_type = TX_HW_AMSDU;

		if (chip_dbg->set_hw_amsdu)
			chip_dbg->set_hw_amsdu(pAd, wcid, amsdu_fix_num, len);
	}

	return TRUE;

error:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("please input wcid-type-amsdu_nums-len (type: 0: TX_SW_AMSDU, 1:TX_HW_AMSDU), len unit 256b\n"));
	return TRUE;
}
INT set_rx_amsdu(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	UINT32 wcid, type;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(ctrl);

	if (arg == NULL || strlen(arg) == 0)
		goto error;

	if (sscanf(arg, "%d-%d", &wcid, &type) != 2)
		goto error;

	if (type < 0 || type >= 2) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wrong type(%d), please input 0: RX_SW_AMSDU, 1: RX_HW_AMSDU\n", type));
		return FALSE;
	}

	if ((type == RX_HW_AMSDU) && !IS_ASIC_CAP(pAd, fASIC_CAP_RX_HDR_TRANS)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wrong type(%d), chip do not have RX_HW_AMSDU cap, please input 0: RX_SW_AMSDU\n", type));
		return FALSE;
	}

	if (type == RX_SW_AMSDU) {
		/* SW AMSDU */
		tr_ctl->damsdu_type = RX_SW_AMSDU;

		if (chip_dbg->set_header_translation)
			chip_dbg->set_header_translation(pAd, wcid, FALSE);

	} else if (type == RX_HW_AMSDU) {
		/* HW AMSDU */
		tr_ctl->damsdu_type = RX_HW_AMSDU;

		if (chip_dbg->set_header_translation)
			chip_dbg->set_header_translation(pAd, wcid, TRUE);
	}

	return TRUE;

error:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("please input wcid-type (type: 0: RX_SW_AMSDU, 1:RX_HW_AMSDU)\n"));
	return TRUE;
}

INT set_ba_dbg(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UINT32 record_basic, record_mac, dump_within, dump_surpass, dump_old, dump_dup, dump_stepone, dump_bar;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct ba_control *ba_ctl = &tr_ctl->ba_ctl;

	if (arg == NULL || strlen(arg) == 0)
		goto error;

	if (sscanf(arg, "%d:%d:%d:%d:%d:%d:%d:%d", &record_basic, &record_mac,
			&dump_within, &dump_surpass, &dump_old, &dump_dup, &dump_stepone, &dump_bar) != 8)
		goto error;

	if (record_basic)
		ba_ctl->dbg_flag |= SN_RECORD_BASIC;
	if (record_mac)
		ba_ctl->dbg_flag |= SN_RECORD_MAC;
	if (dump_within)
		ba_ctl->dbg_flag |= SN_DUMP_WITHIN;
	if (dump_surpass)
		ba_ctl->dbg_flag |= SN_DUMP_SURPASS;
	if (dump_old)
		ba_ctl->dbg_flag |= SN_DUMP_OLD;
	if (dump_dup)
		ba_ctl->dbg_flag |= SN_DUMP_DUP;
	if (dump_stepone)
		ba_ctl->dbg_flag |= SN_DUMP_STEPONE;
	if (dump_bar)
		ba_ctl->dbg_flag |= SN_DUMP_BAR;

	return TRUE;

error:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("please input record_basic:record_mac:dump_within:dump_surpass:dump_old:dump_dup:dump_stepone:dump_bar\n"));
	return TRUE;
}

INT set_tx_deq_cpu(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	RTMP_STRING *str;
	UINT8 band;
	UINT32 cpu;

	if (arg != NULL && strlen(arg)) {
		str = strsep(&arg, ":");
		band = os_str_tol(str, 0, 10);
	} else {
		goto err;
	}

	if (arg != NULL && strlen(arg)) {
		cpu = os_str_tol(arg, 0, 10);
	} else {
		goto err;
	}

	if ((band > 1) || (cpu > 3))
		goto err;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("band %d cpu = %u\n", band, cpu));

	return TRUE;

err:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("invalid input, should be band:cpu\n"));
	return TRUE;
}

INT set_tx_max_cnt(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	struct fp_qm *qm = (struct fp_qm *)pAd->qm;
	UINT32 tx_max_cnt = 1024;

	if (arg == NULL || strlen(arg) == 0)
		goto err;

	if (sscanf(arg, "%d", &tx_max_cnt) != 1)
		goto err;

	qm->max_tx_process_cnt = tx_max_cnt;

	return TRUE;

err:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("invalid input, should be tx_max_cnt\n"));
	return TRUE;
}

INT set_rx_max_cnt(
	PRTMP_ADAPTER pAd,
	char *arg)
{
#ifdef RTMP_MAC_PCI
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = NULL;
	RTMP_STRING *str;
	UINT8 resrc_idx;
	UINT32 max_cnt;

	if (arg != NULL && strlen(arg)) {
		str = strsep(&arg, ":");
		resrc_idx = os_str_tol(str, 0, 10);
	} else {
		goto err;
	}

	if (arg != NULL && strlen(arg)) {
		max_cnt = os_str_tol(arg, 0, 10);
	} else {
		goto err;
	}

	if (resrc_idx > hif->rx_res_num)
		goto err;

	rx_ring = pci_get_rx_ring_by_ridx(hif, resrc_idx);

	rx_ring->max_rx_process_cnt = max_cnt;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("rx ring %d max count = %u\n", resrc_idx, max_cnt));

	return TRUE;

err:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("invalid input, should be resource_idx:max_rx_process_cnt\n"));
#endif /*RTMP_MAC_PCI*/
	return TRUE;
}

#ifdef CUT_THROUGH
INT set_token_setting(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	INT32 qidx, option, sub_option, value;
	INT32 ret;

	if (arg == NULL || strlen(arg) == 0)
		goto err;

	if (sscanf(arg, "%d-%d-%d-%d", &qidx, &option, &sub_option, &value) != 3)
		goto err;

	ret = token_tx_setting(pAd, qidx, option, sub_option, value);

	if (!ret)
		return TRUE;
err:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("invalid input, should be [qidx]-[option]-[sub_option]-[value]\n"));

	return TRUE;
}
#endif

INT set_rx_cnt_io_thd(
	PRTMP_ADAPTER pAd,
	char *arg)
{
#ifdef RTMP_MAC_PCI
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = NULL;
	RTMP_STRING *str;
	UINT8 resrc_idx;
	UINT32 max_cnt;

	if (arg != NULL && strlen(arg)) {
		str = strsep(&arg, ":");
		resrc_idx = os_str_tol(str, 0, 10);
	} else {
		goto err;
	}

	if (arg != NULL && strlen(arg))
		max_cnt = os_str_tol(arg, 0, 10);
	else
		goto err;

	if (resrc_idx > hif->rx_res_num)
		goto err;

	rx_ring = pci_get_rx_ring_by_ridx(hif, resrc_idx);

	rx_ring->max_sw_read_idx_inc = max_cnt;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("rx ring %d rx count threshold for io = %u\n", resrc_idx, max_cnt));

	return TRUE;

err:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("invalid input, should be resource_idx:rx_cnt_io_thd\n"));
#endif /*RTMP_MAC_PCI*/
	return TRUE;
}

INT set_rx_dly_ctl(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	return TRUE;
}

INT set_tx_dly_ctl(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	return TRUE;
}

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
void set_pcie_aspm_dym_ctrl_cap(
	PRTMP_ADAPTER pAd,
	BOOLEAN flag_pcie_aspm_dym_ctrl)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (flag_pcie_aspm_dym_ctrl) {
		pChipCap->asic_caps |= fASIC_CAP_PCIE_ASPM_DYM_CTRL;
	} else {
		pChipCap->asic_caps &= ~fASIC_CAP_PCIE_ASPM_DYM_CTRL;
	}
}

BOOLEAN get_pcie_aspm_dym_ctrl_cap(
	PRTMP_ADAPTER pAd)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	return ((pChipCap->asic_caps & fASIC_CAP_PCIE_ASPM_DYM_CTRL) ? TRUE : FALSE);
}
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#if defined(MT7615) || defined(MT7622) || defined(MT7663)
INT SetRxvRecordEn(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 rv;
	UINT32 Enable, Mode, Group_0, Group_1, Group_2, band_idx, wcid, FcsErrorEn;

	if (arg) {
		UINT32 value;

		rv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%s",
			    &Enable, &Mode, &wcid, &band_idx, &Group_0, &Group_1, &Group_2, &FcsErrorEn, &pAd->RxvFilePath[0]);

		if (rv == 9) {
			if (Enable) {
				PRxVBQElmt ptmprxvbqelmt = pAd->prxvbstartelmt;

				/* Set CSD-Debug set*/
				if (!SetCsdDebugSet(pAd, band_idx, Group_0, Group_1, Group_2))
					return TRUE;

				/* Allocate RxVB buffer to record RxV/RxBlk  */
				if (pAd->prxvbstartelmt == NULL) {
					os_alloc_mem(pAd, (UCHAR **)&pAd->prxvbstartelmt, sizeof(RxVBQElmt) * MAX_RXVB_BUFFER_IN_NORMAL);
					if (pAd->prxvbstartelmt == NULL) {
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Can't allocate memory\n", __func__));
						return TRUE;
					}
					NdisZeroMemory(pAd->prxvbstartelmt, sizeof(RxVBQElmt) * MAX_RXVB_BUFFER_IN_NORMAL);
				}
				ptmprxvbqelmt = (pAd->prxvbstartelmt + MAX_RXVB_BUFFER_IN_NORMAL - 1);
				ptmprxvbqelmt->isEnd = TRUE;

				/* Disable Rx airtime report, it will affect DW10 in RXV. */
				MAC_IO_READ32(pAd->hdev_ctrl, RMAC_AIRTIME0, &pAd->u4RMACAirtime0ori);
				value = pAd->u4RMACAirtime0ori;
				value = value & (~RX_AIRTIME_EN);
				MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_AIRTIME0, value);

				/* RxV Enable*/
				MAC_IO_READ32(pAd->hdev_ctrl, ARB_RQCR, &pAd->u4RQCRori);
				value = pAd->u4RQCRori;
				value = (value | ARB_RQCR_RXV_T_EN | ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_START | ARB_RQCR_RX_START);
				MAC_IO_WRITE32(pAd->hdev_ctrl, ARB_RQCR, value);

				/* Set RFCR about FCS error frame should be drop or not. */
				MAC_IO_READ32(pAd->hdev_ctrl, RMAC_RFCR_BAND_0, &pAd->u4RFCRBand0ori);
				value = pAd->u4RFCRBand0ori;
				if (FcsErrorEn == FALSE)
					value |= DROP_FCS_ERROR_FRAME;
				else
					value &= 0xFFFFFFFD;

				MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_RFCR_BAND_0, value);
				if (pAd->CommonCfg.dbdc_mode) {
					MAC_IO_READ32(pAd->hdev_ctrl, RMAC_RFCR_BAND_1, &pAd->u4RFCRBand1ori);
					value = pAd->u4RFCRBand1ori;
					if (FcsErrorEn == FALSE)
						value |= DROP_FCS_ERROR_FRAME;
					else
						value &= 0xFFFFFFFD;
					MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_RFCR_BAND_1, value);
				}

				/*Set BN0RFCR0 to re-direct all RXD to host.*/
				MAC_IO_READ32(pAd->hdev_ctrl, DMA_BN0RCFR0, &pAd->u4BN0RFCR0ori);
				value = pAd->u4BN0RFCR0ori;
				value &= 0xF0000000;
				MAC_IO_WRITE32(pAd->hdev_ctrl, DMA_BN0RCFR0, value);

				/* Save configuration to global variable */
				pAd->prxvbcurelmt = pAd->prxvbstartelmt;
				pAd->ucRxvRecordEn = Enable;
				pAd->ucRxvMode = Mode;
				pAd->u4RxvCurCnt = 0;
				pAd->ucRxvWcid = wcid;
				pAd->ucFirstWrite = TRUE;
				pAd->ucRecordStart = TRUE;
				pAd->ucRxvG0 = Group_0;
				pAd->ucRxvG1 = Group_1;
				pAd->ucRxvG2 = Group_2;
				pAd->ucRxvBandIdx = band_idx;

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Set RXVEn %d\nMode %d\nWcid %d\nBand %d\nG0 %d\nG1 %d\nG2 %d\nFilePath %s\n",
					  Enable, Mode, wcid, band_idx, Group_0, Group_1, Group_2, pAd->RxvFilePath));
			} else {
				/* Save configuration to global variable */
				pAd->ucRxvRecordEn = Enable;

				/* Restore original CR setting and disable RX vector.*/
				MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_AIRTIME0, pAd->u4RMACAirtime0ori);
				MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_RFCR_BAND_0, pAd->u4RFCRBand0ori);
				MAC_IO_WRITE32(pAd->hdev_ctrl, DMA_BN0RCFR0, pAd->u4BN0RFCR0ori);
				MAC_IO_WRITE32(pAd->hdev_ctrl, ARB_RQCR, pAd->u4RQCRori);

				/* Release all remaining elements which in RxvQ and RxblkQ. */
				pAd->ucRxvCurSN = 0;
				pAd->prxvbcurelmt = NULL;
				if (pAd->prxvbstartelmt != NULL) {
					os_free_mem(pAd->prxvbstartelmt);
					pAd->prxvbstartelmt = NULL;
				}

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Set RXVEn %d\nWrite RXV Done.\nFilePath %s.\n", Enable, pAd->RxvFilePath));
			}
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Format Error! [Enable]-[Mode]-[WCID]-[Band]-[G0]-[G1]-[G2]-[FcsErEn]-[FilePath]\n"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Mode 0: Record RXV to Ethernet.\n"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Mode 1: Record RXV to External Storage.\n"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("WCID: only record wanted WCID of RXV, set 0 to record all WCID of RXV.\n"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Band: 0 for band0, 1 for band1.\n"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("G0/G1/G2: CSD-Debug Group0~Group2 for after payload of RXV.\n"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("FcsErEn: RMAC will receive FCS error packets.\n"));
		}
	}
	return TRUE;
}

BOOLEAN SetCsdDebugSet(RTMP_ADAPTER *pAd, UINT8 band_idx, UINT8 g0_value, UINT8 g1_value, UINT8 g2_value)
{
	UINT8 index;
	UINT32 cr_addr, cr_value, g0_offset, g0_mask, g1_offset, g1_mask, g2_offset, g2_mask;

	/* MT7615(g0,g1,g2), MT7622(g0,g1,g2), MT7663(g0,g1,g2) */
	UINT32 arOffset[3][3] = { {0, 4, 8}, {0, 2, 7}, {9, 11, 16} };
	UINT32 arMask[3][3] = { {0x00000003, 0x000000F0, 0x00000F00},
		{0x00000003, 0x0000007C, 0x00000F80},
		{0x00000600, 0x0000F800, 0x001F0000}
	};

	/* MT7615(Band0, Band1), MT7622(Band0, Band1), MT7663(Band0, Band1) */
	UINT32 arCrAddr[3][BAND_NUM] = { {(WF_PHY_BASE + 0x066C), (WF_PHY_BASE + 0x086C)},
		{(WF_PHY_BASE + 0x066C), (WF_PHY_BASE + 0x086C)},
		{(WF_PHY_BASE + 0x04B0), (WF_PHY_BASE + 0x14B0)}
	};

	if (band_idx > BAND1) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Wrong Band index: %u.\n", band_idx));
		return FALSE;
	}

	if (IS_MT7615(pAd))
		index = 0;
	else if (IS_MT7622(pAd))
		index = 1;
	else if (IS_MT7663(pAd))
		index = 2;
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("This tool only support MT7615, MT7622, MT7663 currently.\n"));
		return FALSE;
	}

	cr_addr = arCrAddr[index][band_idx];
	g0_offset = arOffset[index][0];
	g1_offset = arOffset[index][1];
	g2_offset = arOffset[index][2];
	g0_mask = arMask[index][0];
	g1_mask = arMask[index][1];
	g2_mask = arMask[index][2];

	MAC_IO_READ32(pAd->hdev_ctrl, cr_addr, &cr_value);
	cr_value &= ~(g0_mask | g1_mask | g2_mask);
	cr_value |= ((g0_value << g0_offset) | (g1_value << g1_offset) | (g2_value << g2_offset));
	MAC_IO_WRITE32(pAd->hdev_ctrl, cr_addr, cr_value);

	return TRUE;
}

INT SetRxRateRecordEn(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 rv;
	UINT32 en;

	if (arg) {
		rv = sscanf(arg, "%d", &en);

		if (rv == 1) {
			pAd->ucRxRateRecordEn = en;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set RateRecordEn to %d\n", pAd->ucRxRateRecordEn));
		} else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Format Error! [Enable:1/Disable:0]\n"));
	}
	return TRUE;
}

INT ShowRxRateHistogram(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 i, j;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RX Rate Historgram :\n"));

	for (i = 0; i < 4; i++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CCK MCS%u=%u\n", i, pAd->arRateCCK[i]));
		pAd->arRateCCK[i] = 0;
	}
	for (i = 0; i < 8; i++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("OFDM MCS%u=%u\n", i, pAd->arRateOFDM[i]));
		pAd->arRateOFDM[i] = 0;
	}
	for (i = 0; i < 4; i++)
		for (j = 0; j < 8; j++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("HT MCS%u=%u\n", 8 * i + j, pAd->arRateHT[i][j]));
			pAd->arRateHT[i][j] = 0;
		}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("HT MCS%u=%u\n", 24 + j, pAd->arRateHT[3][j]));
	pAd->arRateHT[3][j] = 0;

	for (i = 0; i < 4; i++)
		for (j = 0; j < 10; j++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("VHT %uSS,MCS%u=%u\n", (i + 1), j, pAd->arRateVHT[i][j]));
			pAd->arRateVHT[i][j] = 0;
		}
	return TRUE;
}
#endif /* #if defined(MT7615) || defined(MT7622) || defined(MT7663) */
INT SetLoadFwMethod(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT Enable;/* enable load from bin */

	Enable = os_str_tol(arg, 0, 10);
	if (!Enable) {
		pChipCap->load_fw_method = BIT(HEADER_METHOD);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Load from header\n"));
	} else {
		pChipCap->load_fw_method = BIT(BIN_METHOD);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("Load from bin\n"));
	}
	return TRUE;
}

#ifdef KERNEL_RPS_ADJUST
/* multi client config  */
INT set_mcli_cfg(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 rv, cmd, op, op2, op3, op4, band_idx;
	RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef RX_RPS_SUPPORT
	UINT8 cpu = 0;
#endif
	if (arg) {
		rv = sscanf(arg, "%u-%u-%u-%u-%u-%u", &cmd, &band_idx, &op, &op2, &op3, &op4);

		if (rv <= 0)
			return FALSE;

		band_idx &= 0x01;

		switch (cmd) {
		case MCLI_RPS_ADJUST_ENABLE:
			pAd->mcli_ctl[band_idx].kernel_rps_adjust_enable = (op > 0);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: kernel_rps_adj[%u]=%u\n", __func__, band_idx,
				pAd->mcli_ctl[band_idx].kernel_rps_adjust_enable));
			break;
		case MCLI_FORCE_AGGLIMIT:
			pAd->mcli_ctl[band_idx].force_agglimit = op % 65;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s:band%u agglimit=%u\n", __func__, band_idx,
				pAd->mcli_ctl[band_idx].force_agglimit));
			break;
		case MCLI_FORCE_TX_PROCESS_CNT:
			pAd->mcli_ctl[band_idx].force_tx_process_cnt = op;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s:band%u tx_process_cnt=%u\n", __func__, band_idx,
				pAd->mcli_ctl[band_idx].force_tx_process_cnt));
			break;
		case MCLI_SHOW_INFO:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rx_qm_en:%u\n", chip_cap->rx_qm_en));
			for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("band%u:\n", band_idx));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("mcli_tx:%u\n", pAd->txop_ctl[band_idx].last_client_num));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("far:%u\n", pAd->mcli_ctl[band_idx].last_large_rssi_gap_num));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("mcli_rx:%u\n", pAd->txop_ctl[band_idx].last_rx_client_num));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("mcli_tcp:%u\n", pAd->txop_ctl[band_idx].last_tcp_nums));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("pkt_avg_len:%u\n", pAd->mcli_ctl[band_idx].pkt_avg_len));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("pkt_rx_avg_len:%u\n", pAd->mcli_ctl[band_idx].pkt_rx_avg_len));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("proc_rps mode:%u\n", pAd->mcli_ctl[band_idx].proc_rps_mode));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("rps_state_flag:0x%08X\n", pAd->mcli_ctl[band_idx].rps_state_flag));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("force_agglimit:%u\n",
					pAd->mcli_ctl[band_idx].force_agglimit));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cur_agglimit:%u\n",
					pAd->mcli_ctl[band_idx].cur_agglimit));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cur_txop:%u\n",
					pAd->mcli_ctl[band_idx].cur_txop));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("force_tx_process_cnt:%u\n",
					pAd->mcli_ctl[band_idx].force_tx_process_cnt));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bidir:%u\n",
					pAd->mcli_ctl[band_idx].is_bidir));

			}
#ifdef RX_RPS_SUPPORT
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cpumap[%d %d %d %d],num:%d\n",
				chip_cap->RxSwRpsCpuMap[3],
				chip_cap->RxSwRpsCpuMap[2],
				chip_cap->RxSwRpsCpuMap[1],
				chip_cap->RxSwRpsCpuMap[0], chip_cap->RxSwRpsNum));
#endif
			break;
		case MCLI_DEBUG_ON:
			pAd->mcli_ctl[band_idx].debug_on = op;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: debug_on[%u]=0x%x\n", __func__, band_idx, pAd->mcli_ctl[band_idx].debug_on));
			break;
#ifdef RX_RPS_SUPPORT
		case MCLI_RX_RPS_ENABLE:
			change_rx_tasklet_method(pAd, (op > 0));
			chip_cap->RxSwRpsNum = 3;
			chip_cap->RxSwRpsCpuMap[0] = 0;
			chip_cap->RxSwRpsCpuMap[1] = 2;
			chip_cap->RxSwRpsCpuMap[2] = 3;
			chip_cap->RxSwRpsCpuMap[3] = NR_CPUS;
			chip_cap->rx_qm_en = (op > 0);
			break;
		case MCLI_RX_RPS_CPUMAP:
			chip_cap->RxSwRpsNum = 0;
			chip_cap->RxSwRpsCpu = op;
			for (cpu = 0; cpu < NR_CPUS; cpu++)
				chip_cap->RxSwRpsCpuMap[cpu] = NR_CPUS;

			for (cpu = 0; cpu < NR_CPUS; cpu++) {
				if ((1 << cpu) & chip_cap->RxSwRpsCpu) {
					chip_cap->RxSwRpsCpuMap[chip_cap->RxSwRpsNum] = cpu;
					chip_cap->RxSwRpsNum++;
				}
			}
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("cpumap[%d %d %d %d],num:%d\n",
				chip_cap->RxSwRpsCpuMap[3],
				chip_cap->RxSwRpsCpuMap[2],
				chip_cap->RxSwRpsCpuMap[1],
				chip_cap->RxSwRpsCpuMap[0], chip_cap->RxSwRpsNum));
			break;
#endif
		case MCLI_FORCE_RPS_CFG:
			if (band_idx < DBDC_BAND_NUM) {
				pAd->mcli_ctl[band_idx].force_rps_cfg = TRUE;
				snprintf(pAd->mcli_ctl[band_idx].force_proc_rps[ETH0_RPS_FILE],
					sizeof(pAd->mcli_ctl[band_idx].force_proc_rps[ETH0_RPS_FILE]),"%d", op);
				snprintf(pAd->mcli_ctl[band_idx].force_proc_rps[BAND0_RPS_FILE],
					sizeof(pAd->mcli_ctl[band_idx].force_proc_rps[BAND0_RPS_FILE]), "%d", op2);
				snprintf(pAd->mcli_ctl[band_idx].force_proc_rps[BAND1_RPS_FILE],
				 	sizeof(pAd->mcli_ctl[band_idx].force_proc_rps[BAND1_RPS_FILE]) ,"%d", op3);
				snprintf(pAd->mcli_ctl[band_idx].force_proc_rps[TXFREE_IRQ_FILE],
					sizeof(pAd->mcli_ctl[band_idx].force_proc_rps[TXFREE_IRQ_FILE]), "%d", op4);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: proc_rps [%s %s %s %s]\n", __func__,
					pAd->mcli_ctl[band_idx].force_proc_rps[ETH0_RPS_FILE],
					pAd->mcli_ctl[band_idx].force_proc_rps[BAND0_RPS_FILE],
					pAd->mcli_ctl[band_idx].force_proc_rps[BAND1_RPS_FILE],
					pAd->mcli_ctl[band_idx].force_proc_rps[TXFREE_IRQ_FILE]));
			}
			break;
		case MCLI_CLI_NUMS_EAP_TH:
			pAd->multi_cli_nums_eap_th = op;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: eap_cli_nums=%u\n", __func__, pAd->multi_cli_nums_eap_th));
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: cmd:%d, op:%d\n", __func__, cmd, op));
			break;
		}
	} else
		return FALSE;

	return TRUE;
}
#endif

INT set_fwcmd_timeout_print_cnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->FwCmdTimeoutPrintCnt = (UINT16)os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s(): count = %d %s\n", __func__, pAd->FwCmdTimeoutPrintCnt,
			 (pAd->FwCmdTimeoutPrintCnt == 0) ? "(unlimited)" : ""));

	return TRUE;
}

INT show_fwcmd_timeout_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 print_cnt = 0;
	UINT16 i = 0, start_idx;
	UINT16 rec_idx;
	P_FWCMD_TIMEOUT_RECORD pToRec = NULL;
	ULONG Now32;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%s:\n", __func__));

	NdisGetSystemUpTime(&Now32);
	print_cnt = (pAd->FwCmdTimeoutCnt < FW_CMD_TO_RECORD_CNT) ?
				 pAd->FwCmdTimeoutCnt : FW_CMD_TO_RECORD_CNT;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\t- TimeoutCnt(%d), PrintCnt(%d), RecordCnt(%d)\n",
			 pAd->FwCmdTimeoutCnt, pAd->FwCmdTimeoutPrintCnt, FW_CMD_TO_RECORD_CNT));

	if (print_cnt) {
		start_idx = (pAd->FwCmdTimeoutCnt - 1) % FW_CMD_TO_RECORD_CNT ;
		for (i = 0; i < print_cnt; i++) {
			rec_idx = (start_idx + FW_CMD_TO_RECORD_CNT - i) % FW_CMD_TO_RECORD_CNT;
			pToRec = &pAd->FwCmdTimeoutRecord[rec_idx];
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("\[%02d/%02d] (%ld ms ago) FWCmdTimeout: cmd(%x), ext_cmd(%x), seq(%d), state(%d)\n",
					 i, rec_idx, ((Now32 - pToRec->timestamp) * 1000 / OS_HZ),
					  pToRec->type, pToRec->ext_type, pToRec->seq, pToRec->state));
		}
	}

	return TRUE;
}

INT Set_BSSAifsn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ap_idx)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s() invalid mbss id(%d)\n", __func__, ap_idx));
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}

	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pBssEdca is NULL\n"));
		return FALSE;
	}

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] < 2) || (value_temp[i] > 15)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("BSSAifsn[%d]=%d is invalid, BSSAifsn should be configured from 2 to 15\n",
				i, value_temp[i]));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure BSSAifsn\n"));

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pBssEdca->Aifsn[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSSAifsn[%d]=%d\n", i, pBssEdca->Aifsn[i]));
	}

	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	return TRUE;
}

INT Set_BSSCwmin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ap_idx)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s() invalid mbss id(%d)\n", __func__, ap_idx));
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}

	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pBssEdca is NULL\n"));
		return FALSE;
	}

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] > 15)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("BSSCwmin[%d]=%d is invalid, BSSCwmin should not be over 15\n", i, value_temp[i]));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure BSSCwmin\n"));
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pBssEdca->Cwmin[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSSCwmin[%d]=%d\n", i, pBssEdca->Cwmin[i]));
	}

	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	return TRUE;
}

INT Set_BSSCwmax_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ap_idx)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s() invalid mbss id(%d)\n", __func__, ap_idx));
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}

	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pBssEdca is NULL\n"));
		return FALSE;
	}

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] > 15)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("BSSCwmax[%d]=%d is invalid, BSSCwmax should not be over 15\n", i, value_temp[i]));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure BSSCwmax\n"));
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pBssEdca->Cwmax[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSSCwmax[%d]=%d\n", i, pBssEdca->Cwmax[i]));
	}

	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	return TRUE;
}

INT Set_BSSTxop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
	USHORT value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ap_idx)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s() invalid mbss id(%d)\n", __func__, ap_idx));
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}

	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pBssEdca is NULL\n"));
		return FALSE;
	}

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (USHORT)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure BSSTxop\n"));
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pBssEdca->Txop[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BSSTxop[%d]=%d\n", i, pBssEdca->Txop[i]));
	}

	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	return TRUE;
}

UCHAR ac_queue[] = {
	TxQ_IDX_AC1, /* ACI:0 AC_BE */
	TxQ_IDX_AC0, /* ACI:1 AC_BK */
	TxQ_IDX_AC2, /* ACI:2 AC_VI */
	TxQ_IDX_AC3, /* ACI:3 AC_VO */
};

INT Set_APAifsn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = TRUE;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ap_idx)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s() invalid mbss id(%d)\n", __func__, ap_idx));
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = HcGetEdca(pAd, wdev);

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] < 1) || (value_temp[i] > 15)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("APAifsn[%d]=%d is invalid, APAifsn should be configured from 1 to 15\n",
				i, value_temp[i]));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure APAifsn\n"));
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[i] = value_temp[i];
		if (wmm_edca_param)
			wmm_edca_param->Aifsn[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("APAifsn[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[i]));
		ac_index = ac_queue[i];
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_AIFSN, value_temp[i]) == NDIS_STATUS_FAILURE) {
			success = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("fails to set APAifsn[%d]\n", i));
		 }
	}

	return success;
}

INT Set_APCwmin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = TRUE;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ap_idx)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s() invalid mbss id(%d)\n", __func__, ap_idx));
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = HcGetEdca(pAd, wdev);

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] > 16)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("APCwmin[%d]=%d is invalid, APCwmin should not be over 16\n", i, value_temp[i]));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure APCwmin\n"));
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[i] = value_temp[i];
		if (wmm_edca_param)
			wmm_edca_param->Cwmin[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("APCwmin[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[i]));
		ac_index = ac_queue[i];
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_CWMIN, value_temp[i]) == NDIS_STATUS_FAILURE) {
			success = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("fails to set APCwmin[%d]\n", i));
		 }
	}

	return success;
}

INT Set_APCwmax_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = TRUE;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ap_idx)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s() invalid mbss id(%d)\n", __func__, ap_idx));
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = HcGetEdca(pAd, wdev);

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] > 16)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("APCwmax[%d]=%d is invalid, APCwmax should not be over 16\n", i, value_temp[i]));
			return FALSE;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure APCwmax\n"));
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[i] = value_temp[i];
		if (wmm_edca_param)
			wmm_edca_param->Cwmax[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("APCwmax[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[i]));
		ac_index = ac_queue[i];
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_CWMAX, value_temp[i]) == NDIS_STATUS_FAILURE) {
			success = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("fails to set APCwmax[%d]\n", i));
		 }
	}

	return success;
}

INT Set_APTxop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = TRUE;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
	USHORT value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ap_idx)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("%s() invalid mbss id(%d)\n", __func__, ap_idx));
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("wdev is NULL\n"));
		return FALSE;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = HcGetEdca(pAd, wdev);

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (USHORT)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("invalid format, parameters should be configured as xx:xx:xx:xx\n"));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("start to configure APTxop\n"));
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[i] = value_temp[i];
		if (wmm_edca_param)
			wmm_edca_param->Txop[i] = value_temp[i];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("APTxop[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[i]));
		ac_index = ac_queue[i];
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_TXOP, value_temp[i]) == NDIS_STATUS_FAILURE) {
			success = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("fails to set APTxop[%d]\n", i));
		 }
	}

	return success;
}

#ifdef MGMT_TXPWR_CTRL
/* Tx pwr offset in 0.5db unit*/
#define TX_PWR_OFFSET_WTBL_MIN -31
#define TX_PWR_OFFSET_WTBL_MAX 0

INT set_mgmt_frame_power(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT16  value;
	CHAR   target_pwr = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	RTMP_STRING *tmp;
	struct wifi_dev *wdev;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (!arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Invalid argument\n"));
		return FALSE;
	}

	if (*arg == '.') { /* handle .5 arg value*/
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("unable to update tx pwr %s\n", arg));
		return FALSE;
	}

	tmp = rstrtok(arg, ".");

	if (!tmp) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Please enter valid tx pwr\n"));
		return FALSE;
	}

	value = os_str_tol(tmp, 0, 10);
	target_pwr = value*2; /* convert to  0.5db scale*/
	tmp = rstrtok(NULL, ".");

	if (tmp) {
		value = os_str_tol(tmp, 0, 10);
		if (value == 5)
			target_pwr += 1;
		else if (target_pwr == 0) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("unable to update tx pwr\n"));
			return FALSE;
		}
	}

	wdev->MgmtTxPwrBak = wdev->MgmtTxPwr;
	wdev->MgmtTxPwr = target_pwr;
	update_mgmt_frame_power(pAd, wdev);
	return TRUE;
}

INT update_mgmt_frame_power(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	CHAR   target_pwr, delta_pwr;
	UINT8  OpMode, BandIdx;

	BandIdx = HcGetBandByWdev(wdev);
	OpMode = wdev->rate.MlmeTransmit.field.MODE;

	target_pwr = wdev->MgmtTxPwr;

	if (target_pwr == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("[%s] disable mgmt pwr ctrl\n", __func__));
		wdev->bPwrCtrlEn = FALSE;
		wdev->TxPwrDelta = 0;
		wtbl_update_pwr_offset(pAd, wdev);
		goto exit;
	}

	if (pAd->ApCfg.MgmtTxPwr[BandIdx] == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] Mgmt Tx base pwr zero apidx:%d band-idx:%d\n",
		__func__, wdev->func_idx, BandIdx));
		MtCmdTxPwrShowInfo(pAd, TXPOWER_ALL_RATE_POWER_INFO, BandIdx);

	}

	delta_pwr = target_pwr - (pAd->ApCfg.MgmtTxPwr[BandIdx] + pAd->ApCfg.EpaFeGain[BandIdx]);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s] Target_Pwr=%d Base_Pwr=%d delta_Pwr=%d!!!\n",
			__func__, target_pwr/2, (pAd->ApCfg.MgmtTxPwr[BandIdx] + pAd->ApCfg.EpaFeGain[BandIdx])/2, delta_pwr/2));

	if (delta_pwr >= TX_PWR_OFFSET_WTBL_MIN && delta_pwr <= TX_PWR_OFFSET_WTBL_MAX) {
		/* update wtbl tx pwr offset*/
		wdev->bPwrCtrlEn = TRUE;
		wdev->TxPwrDelta = delta_pwr;
		wtbl_update_pwr_offset(pAd, wdev);
		wdev->MgmtTxPwrBak = wdev->MgmtTxPwr;
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] unable to adjust target pwr \n", __func__));
		wdev->MgmtTxPwr = wdev->MgmtTxPwrBak;

		if (wdev->bPwrCtrlEn)
			return FALSE;
	}

exit:

	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_DISABLE_TX);
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_ENABLE_TX);

	return TRUE;
}

INT show_mgmt_frame_power(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8	Tx_Pwr, BandIdx;
	struct wifi_dev	*wdev;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);

	Tx_Pwr = pAd->ApCfg.MgmtTxPwr[BandIdx] + pAd->ApCfg.EpaFeGain[BandIdx] + (wdev->TxPwrDelta);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Tx_Pwr = %d.%d db\n",
		__func__, Tx_Pwr/2, ((Tx_Pwr%2 == 0) ? 0:5)));

	return TRUE;
}
#endif
#ifdef PKTLOSS_CHK
extern BOOLEAN pktloss_chk_func(PRTMP_ADAPTER pAd,
				PUINT_8 aucPktContent,
				UINT_8 ip_offset,
				UINT_8 check_point,
				BOOLEAN drop);
extern void pktloss_chk_dump(PRTMP_ADAPTER pAd, BOOLEAN dump_all);

INT set_pktloss_chk(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 rv, cmd, param = 0;
	if (!arg)
		return FALSE;

	rv = sscanf(arg, "%d-%d", &cmd, &param);
	if ((rv <= 0) || (cmd >= PKTLOSS_CHK_SET_MAX))
		return FALSE;

	MtCmdFwLog2Host(pAd, 1, 2);
	if (cmd == PKTLOSS_CHK_SET_IPERF_CASE) {
		/* iperf */
		os_zero_mem(&pAd->pktloss_chk, sizeof(pktloss_check_var_type));
		pAd->pktloss_chk.pktloss_chk_handler = pktloss_chk_func;
		pAd->pktloss_chk.src_ip_mask = 0xFFFFFFFF;
		pAd->pktloss_chk.dest_ip_mask = 0xFFFFFFFF;
		pAd->pktloss_chk.dest_port = param;
		pAd->pktloss_chk.byte_offset = 0;
		pAd->pktloss_chk.is_seq_signed = 1;
		pAd->pktloss_chk.is_seq_cross_zero = 1;
		pAd->pktloss_chk.seq_mask = 0xFFFFFFFF;
		pAd->pktloss_chk.ctrl_flag = (1<<PKTLOSS_CHK_BY_SEQ);
		pAd->pktloss_chk.ts_threshold = 20000/(1000000/HZ);
		pAd->pktloss_chk.proto = 17;
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_RESET, 0);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_SRC_IP, 0xFFFFFFFF);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_DEST_IP, 0xFFFFFFFF);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_PORT, param);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_OFFSET, 0);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_IS_SEQ_SIGNED, 1);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_IS_SEQ_CROSS_ZERO, 1);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_SEQ_MASK, 0xFFFFFFFF);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_CTRL_FLAG,
			(1<<PKTLOSS_CHK_BY_SEQ));
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_TS_THRESHOLD, 20000/30.5);

		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_ENABLE, 1);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_DUMP_SHORT, 0);
		pktloss_chk_dump(pAd, TRUE);
		pAd->pktloss_chk.enable = TRUE;
	} else if (cmd == PKTLOSS_CHK_SET_RTP_CASE) {
		/* RTP */
		os_zero_mem(&pAd->pktloss_chk, sizeof(pktloss_check_var_type));
		pAd->pktloss_chk.pktloss_chk_handler = pktloss_chk_func;
		pAd->pktloss_chk.src_ip_mask = 0xFFFFFFFF;
		pAd->pktloss_chk.dest_ip_mask = 0xFFFFFFFF;
		pAd->pktloss_chk.dest_port = param;
		pAd->pktloss_chk.byte_offset = 0;
		pAd->pktloss_chk.is_seq_signed = 0;
		pAd->pktloss_chk.is_seq_cross_zero = 1;
		pAd->pktloss_chk.seq_mask = 0x0FFFF;
		pAd->pktloss_chk.ctrl_flag = (1<<PKTLOSS_CHK_BY_SEQ)|(1<<PKTLOSS_CHK_BY_TS);
		pAd->pktloss_chk.ts_threshold = 20000/(1000000/HZ);
		pAd->pktloss_chk.proto = 17;
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_RESET, 0);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_SRC_IP, 0xFFFFFFFF);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_DEST_IP, 0xFFFFFFFF);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_PORT, param);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_OFFSET, 0);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_IS_SEQ_SIGNED, 0);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_IS_SEQ_CROSS_ZERO, 1);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_SEQ_MASK, 0xFFFF);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_CTRL_FLAG,
			(1<<PKTLOSS_CHK_BY_SEQ)|(1<<PKTLOSS_CHK_BY_TS));
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_TS_THRESHOLD, 20000/30.5);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_ENABLE, 1);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_DUMP_SHORT, 0);
		pktloss_chk_dump(pAd, TRUE);
		pAd->pktloss_chk.enable = TRUE;
	} else {
		switch (cmd) {
		case PKTLOSS_CHK_SET_SRC_IP:
			pAd->pktloss_chk.src_ip_mask = param;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] src_ip_mask:%u\n",
				__func__, pAd->pktloss_chk.src_ip_mask));
			break;
		case PKTLOSS_CHK_SET_DEST_IP:
			pAd->pktloss_chk.dest_ip_mask = param;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] dest_ip_mask:%u\n",
				__func__, pAd->pktloss_chk.dest_ip_mask));
			break;
		case PKTLOSS_CHK_SET_PORT:
			pAd->pktloss_chk.dest_port = param & 0x0FFFF;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] dest_port:%u\n",
				__func__, pAd->pktloss_chk.dest_port));
			break;
		case PKTLOSS_CHK_SET_OFFSET:
			pAd->pktloss_chk.byte_offset = param;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] byte_offset:%u\n",
				__func__, pAd->pktloss_chk.byte_offset));
			break;
		case PKTLOSS_CHK_SET_IS_SEQ_SIGNED:
			pAd->pktloss_chk.is_seq_signed = (param > 0) ? 1 : 0;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] is_seq_signed:%u\n",
				__func__, pAd->pktloss_chk.is_seq_signed));
			break;
		case PKTLOSS_CHK_SET_IS_SEQ_CROSS_ZERO:
			pAd->pktloss_chk.is_seq_cross_zero = (param > 0) ? 1 : 0;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] is_seq_cross_zero:%u\n",
				__func__, pAd->pktloss_chk.is_seq_cross_zero));
			break;
		case PKTLOSS_CHK_SET_SEQ_MASK:
			pAd->pktloss_chk.seq_mask = param;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] seq_mask:0x%08X\n",
				__func__, pAd->pktloss_chk.seq_mask));
			break;
		case PKTLOSS_CHK_SET_CTRL_FLAG:
			pAd->pktloss_chk.ctrl_flag = param;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] ctrl_flag:0x%08X\n",
				 __func__, pAd->pktloss_chk.ctrl_flag));
			break;
		case PKTLOSS_CHK_SET_TS_THRESHOLD:
			pAd->pktloss_chk.ts_threshold = param/(1000000/HZ);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] threshold:%d\n",
				__func__, pAd->pktloss_chk.ts_threshold));
			param = (param << 1)/60;
			break;
		case PKTLOSS_CHK_SET_ENABLE:
			pAd->pktloss_chk.enable = (param > 0) ? TRUE : FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] enable:%u\n",
				__func__, pAd->pktloss_chk.enable));
			break;
		case PKTLOSS_CHK_SET_RESET:
			pAd->pktloss_chk.enable = FALSE;
			os_zero_mem(&pAd->pktloss_chk, sizeof(pktloss_check_var_type));
			pAd->pktloss_chk.proto = 17;
			pAd->pktloss_chk.seq_mask = 0xFFFFFFFF;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] reset\n",
				__func__));
			break;
		case PKTLOSS_CHK_SET_DUMP_SHORT:
			pktloss_chk_dump(pAd, FALSE);
			break;
		case PKTLOSS_CHK_SET_HEX_DUMP:
			pAd->pktloss_chk.ctrl_flag |= (1 << PKTLOSS_CHK_HEX_DUMP);
			break;
		case PKTLOSS_CHK_SET_CONTINUE_HEX_DUMP:
			pAd->pktloss_chk.ctrl_flag |= (1 << PKTLOSS_CHK_CONTINUE_HEX_DUMP);
			break;
		case PKTLOSS_CHK_SET_TXS_LOG_ENABLE:
			pAd->pktloss_chk.txs_log_enable = (param > 0) ? TRUE : FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] txs_log_enable:%u\n",
				__func__, pAd->pktloss_chk.txs_log_enable));
			break;
		default:
			break;
		}
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, cmd, param);
	}
	MtCmdFwLog2Host(pAd, 1, 0);

	return TRUE;
}
#endif

#ifdef ANTENNA_CONTROL_SUPPORT
INT Antenna_Control_Init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UINT8 BandIdx = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct mcs_nss_caps *nss_cap = &cap->mcs_nss;

	BandIdx = HcGetBandByWdev(wdev);
	if (pAd->TxStream[BandIdx] && pAd->RxStream[BandIdx]) {
		wlan_config_set_tx_stream(wdev, min(pAd->TxStream[BandIdx], nss_cap->max_nss));
		wlan_config_set_rx_stream(wdev, min(pAd->RxStream[BandIdx], nss_cap->max_nss));
		wlan_operate_set_tx_stream(wdev, min(pAd->TxStream[BandIdx], nss_cap->max_nss));
		wlan_operate_set_rx_stream(wdev, min(pAd->RxStream[BandIdx], nss_cap->max_nss));
#ifdef DOT11_HE_AX
		wlan_config_set_he_rx_nss(wdev, min(pAd->RxStream[BandIdx], nss_cap->max_nss));
		wlan_config_set_he_tx_nss(wdev, min(pAd->TxStream[BandIdx], nss_cap->max_nss));
#endif
		SetCommonHtVht(pAd, wdev);
	}
	return TRUE;
}

INT Set_Antenna_Control_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx = 0, Txstream = 0, Rxstream = 0;
	struct wifi_dev *wdev;
	BSS_STRUCT *pMbss = NULL;

	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct mcs_nss_caps *nss_cap = &cap->mcs_nss;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Incoorect BSS!! \n", __FUNCTION__));
		return FALSE;
	}

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
		return FALSE;
	}

	if (0 == simple_strtol(&arg[0], 0, 10)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Set default Antenna number!!\n", __func__));
		Txstream = Rxstream = nss_cap->max_nss;
		if (pAd->CommonCfg.dbdc_mode)
			Txstream = Rxstream = (nss_cap->max_nss/2);
		goto set_default;
	}

	if (strlen(arg) != 4) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use input format like XTXR (X = 1,2) !! \n", __FUNCTION__));
		return FALSE;
	}

	if (((arg[1] == 'T') || (arg[1] == 't')) && ((arg[3] == 'R') || (arg[3] == 'r'))) {
		Txstream = simple_strtol(&arg[0], 0, 10);
		Rxstream = simple_strtol(&arg[2], 0, 10);

		if (Txstream != Rxstream) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("Wrong Input: Tx & Rx Antenna number different!\n"));
			return FALSE;
		}
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
			UINT dbdc_tx_max_nss;
			UINT dbdc_rx_max_nss;

			if (BandIdx == DBDC_BAND1) {
				dbdc_tx_max_nss = pAd->dbdc_band1_tx_path;
				dbdc_rx_max_nss = pAd->dbdc_band1_rx_path;
			} else {
				dbdc_tx_max_nss = pAd->dbdc_band0_tx_path;
				dbdc_rx_max_nss = pAd->dbdc_band0_rx_path;
			}

			if ((Txstream > dbdc_tx_max_nss) || (Rxstream > dbdc_rx_max_nss)) {

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("Wrong Input: BandIdx = %d, more than max DBDC ant cap (%d)!!",
							BandIdx, dbdc_tx_max_nss));
				return FALSE;
			}
		} else
#endif
		{
			if (Txstream > nss_cap->max_nss) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("Wrong Input: BandIdx = %d, more than max ant cap (%d)!!\n",
						BandIdx, nss_cap->max_nss));
				return FALSE;
			}
		}

set_default:
		if (BandIdx == DBDC_BAND0) {
			pAd->TxStream[BandIdx] = Txstream;
			pAd->RxStream[BandIdx] = Rxstream;
			pAd->bAntennaSetAPEnable[BandIdx] = 1;
		}
#ifdef DBDC_MODE
		else {
			if (pAd->CommonCfg.dbdc_mode &&
				(BandIdx == DBDC_BAND1)) {
				pAd->TxStream[BandIdx] = Txstream;
				pAd->RxStream[BandIdx] = Rxstream;
				pAd->bAntennaSetAPEnable[BandIdx] = 1;
			}
		}
#endif
		wlan_config_set_tx_stream(wdev, min(Txstream, nss_cap->max_nss));
		wlan_config_set_rx_stream(wdev, min(Rxstream, nss_cap->max_nss));
		wlan_operate_set_tx_stream(wdev, min(Txstream, nss_cap->max_nss));
		wlan_operate_set_rx_stream(wdev, min(Rxstream, nss_cap->max_nss));
#ifdef DOT11_HE_AX
		wlan_config_set_he_tx_nss(wdev, min(Txstream, nss_cap->max_nss));
		wlan_config_set_he_rx_nss(wdev, min(Rxstream, nss_cap->max_nss));
#endif /* DOT11_HE_AX */

		SetCommonHtVht(pAd, wdev);
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Wrong Params, xTxR (x <= %d)!!\n", nss_cap->max_nss));
		return FALSE;
	}
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (pMbss != NULL) {
			APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
			APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
			return TRUE;
		} else
			return FALSE;
	}
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}

INT Show_Antenna_Control_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx = 0;
	struct wifi_dev *wdev;

#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);
#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(TxStream = %d, RxStream = %d)\n",
		 (pAd->TxStream[BandIdx]) ? pAd->TxStream[BandIdx] : wlan_operate_get_tx_stream(wdev),
		 (pAd->RxStream[BandIdx]) ? pAd->RxStream[BandIdx] : wlan_operate_get_rx_stream(wdev)));

	return TRUE;
}
#endif /* ANTENNA_CONTROL_SUPPORT */

#ifdef MAP_R2
INT show_traffic_separation_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	unsigned short vid = 0;
	int i = 0;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("ifname\town role\tpvid\tpcp\tSSID\tfh_vid\tvid_num\tall vid\n"));
	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		wdev = &pAd->ApCfg.MBSSID[i].wdev;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s\t%02x\t%d\t%d\t%s\t%d\t%d ",
				 wdev->if_dev->name, wdev->MAPCfg.DevOwnRole, wdev->MAPCfg.primary_vid,
				 wdev->MAPCfg.primary_pcp, pAd->ApCfg.MBSSID[i].Ssid, wdev->MAPCfg.fh_vid,
				 wdev->MAPCfg.vid_num));

		for (vid = 1; vid < 4095; vid++) {
			if (is_vid_configed(vid, wdev->MAPCfg.vids)) {
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%d ", vid));
			}
		}

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n\ttransparent vlan:"));
		for (vid = 1; vid < 4095; vid++) {
			if (is_vid_configed(vid, wdev->MAPCfg.bitmap_trans_vlan)) {
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 (" %d", vid));
			}
		}

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n"));
	}

	for (i = 0; i < pAd->MSTANum; i++) {
		wdev = &pAd->StaCfg[i].wdev;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s\t%02x\t%d\t%d\t%s\t%d\t%d ",
				 wdev->if_dev->name, wdev->MAPCfg.DevOwnRole, wdev->MAPCfg.primary_vid,
				 wdev->MAPCfg.primary_pcp, "N/A", wdev->MAPCfg.fh_vid,
				 wdev->MAPCfg.vid_num));
		for (vid = 1; vid < 4095; vid++) {
			if (is_vid_configed(vid, wdev->MAPCfg.vids)) {
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%d ", vid));
			}
		}
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 ("\n"));
	}

	return TRUE;
}
#endif

#ifdef IWCOMMAND_CFG80211_SUPPORT
INT set_apmacaddress(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	RT_CfgSetMacAddress(pAd, arg, apidx, OPMODE_AP);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ApMacAddress%d = %02x:%02x:%02x:%02x:%02x:%02x\n",
			 apidx, PRINT_MAC(pAd->ExtendMBssAddr[apidx-1])));
	return TRUE;
}

INT set_apclimacaddress(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	RT_CfgSetMacAddress(pAd, arg, ifIndex, OPMODE_STA);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ApCliMacAddress%d = %02x:%02x:%02x:%02x:%02x:%02x\n",
			ifIndex, PRINT_MAC(pAd->ApcliAddr[ifIndex])));

	COPY_MAC_ADDR(&pAd->StaCfg[ifIndex].wdev.if_addr, pAd->ApcliAddr[ifIndex]);
	return TRUE;
}
#endif /* IWCOMMAND_CFG80211_SUPPORT */

#ifdef ANDLINK_FEATURE_SUPPORT
INT set_andlink_en_porc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pobj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pobj->ioctl_if;
	INT andlink_enable = 0;
	INT band_idx = 0;
	struct wifi_dev *wdev = NULL;

	if ((pobj->ioctl_if_type == INT_MAIN) || (pobj->ioctl_if_type == INT_MBSSID)) {
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): invalid mbss id(%d)\n", __func__, ifIndex));
			return FALSE;
		}
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		band_idx = HcGetBandByWdev(wdev);

		andlink_enable = os_str_tol(arg, 0, 10);

		if (!andlink_enable)
			andlink_enable = FALSE;
		else
			andlink_enable = TRUE;

		if (pAd->CommonCfg.andlink_enable[band_idx] == andlink_enable) {
			/* No need to do anything, current and previos values are same */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("[%s](%d): current andlink_enable is %d in band %d\n",
				__func__, __LINE__, andlink_enable, band_idx));
			return TRUE;
		}

		pAd->CommonCfg.andlink_enable[band_idx] = andlink_enable;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("[%s](%d):andlink_enable is %d in band %d success! \n",
				__func__, __LINE__, andlink_enable, band_idx));
		return TRUE;
	}

	return FALSE;
}

INT set_andlink_ip_hostname_en_porc(RTMP_ADAPTER *pAd, RTMP_STRING *arg) {
	POS_COOKIE pobj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pobj->ioctl_if;
	INT andlink_ip_hostname_en = 0;
	INT band_idx = 0;
	struct wifi_dev *wdev = NULL;

	if ((pobj->ioctl_if_type == INT_MAIN) || (pobj->ioctl_if_type == INT_MBSSID)) {
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): invalid mbss id(%d)\n", __func__, ifIndex));
			return FALSE;
		}
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		band_idx = HcGetBandByWdev(wdev);

		andlink_ip_hostname_en = os_str_tol(arg, 0, 10);

		if (!andlink_ip_hostname_en)
			andlink_ip_hostname_en = FALSE;
		else
			andlink_ip_hostname_en = TRUE;

		if (pAd->CommonCfg.andlink_ip_hostname_en[band_idx] == andlink_ip_hostname_en) {
			/* No need to do anything, current and previos values are same */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("[%s](%d): current andlink_enable is %d in band %d\n",
				__func__, __LINE__, andlink_ip_hostname_en, band_idx));
			return TRUE;
		}

		pAd->CommonCfg.andlink_ip_hostname_en[band_idx] = andlink_ip_hostname_en;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("[%s](%d):andlink_ip_hostname_en is %d in band %d success! \n",
					__func__, __LINE__, andlink_ip_hostname_en, band_idx));
		return TRUE;
	}

	return FALSE;
}
#endif/*ANDLINK_FEATURE_SUPPORT*/


#ifdef ACK_CTS_TIMEOUT_SUPPORT
static INT set_ack_timeout_cr(RTMP_ADAPTER *pAd, UINT32 reg_addr, UINT32 timeout)
{
	UINT32 mac_val = 0;
	if (NULL == pAd || NULL == pAd->hdev_ctrl) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: invalid null input: pAd=%p, pAd->hdev_ctrl=%p;!!\n",
			__func__, pAd, pAd->hdev_ctrl));
		return FALSE;
	}

	MAC_IO_READ32(pAd->hdev_ctrl, reg_addr, &mac_val);
	mac_val &= ~MAX_ACK_TIMEOUT;
	mac_val |= (timeout & 0xFFFF);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("reg_addr = %x, mac_val= %x!!\n",
		reg_addr, mac_val));
	MAC_IO_WRITE32(pAd->hdev_ctrl, reg_addr, mac_val);
	return TRUE;
}

INT set_ack_timeout_mode_byband(
	RTMP_ADAPTER *pAd,
	UINT32 timeout,
	UINT32 bandidx,
	ACK_TIMEOUT_MODE_T ackmode)
{

	if ((timeout > MAX_ACK_TIMEOUT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("CTS/ACK Timeout Range should between [0xFFFF:0]!!\n"));
		return FALSE;
	}

	if (pAd->CommonCfg.ack_cts_enable[bandidx] == FALSE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s](%d):ERROR! BAND%u, ack_cts_enable=%u, CTS/ACK FEATURE is not enable!!\n",
			__func__, __LINE__, bandidx, pAd->CommonCfg.ack_cts_enable[bandidx]));
		return FALSE;
	}

	switch (bandidx) {
		case BAND0: {
			/*set BAND0 CCK OFDM OFDMA TIMEOUT*/
			switch (ackmode) {
				case CCK_TIME_OUT: {
					set_ack_timeout_cr(pAd, BN0_WF_TMAC_TOP_CDTR_ADDR, timeout);
					break;
				}
				case OFDM_TIME_OUT: {
					set_ack_timeout_cr(pAd, BN0_WF_TMAC_TOP_ODTR_ADDR, timeout);
					break;
				}
				case OFDMA_TIME_OUT: {
					set_ack_timeout_cr(pAd, BN0_WF_TMAC_TOP_OMDTR_ADDR, timeout);
					break;
				}
				case ACK_ALL_TIME_OUT: {
					set_ack_timeout_cr(pAd, BN0_WF_TMAC_TOP_CDTR_ADDR, timeout);
					set_ack_timeout_cr(pAd, BN0_WF_TMAC_TOP_ODTR_ADDR, timeout);
					set_ack_timeout_cr(pAd, BN0_WF_TMAC_TOP_OMDTR_ADDR, timeout);
					break;
				}
				default: {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: invalid ackmode ,  ackmode=%d!!\n",
					__FUNCTION__, ackmode));
					return FALSE;
				}
			}
			break;
		}
		case BAND1: {
			/*set BAND1 CCK OFDM OFDMA TIMEOUT*/
			switch (ackmode) {
				case CCK_TIME_OUT: {
					set_ack_timeout_cr(pAd, BN1_WF_TMAC_TOP_CDTR_ADDR, timeout);
					break;
				}
				case OFDM_TIME_OUT: {
					set_ack_timeout_cr(pAd, BN1_WF_TMAC_TOP_ODTR_ADDR, timeout);
					break;
				}
				case OFDMA_TIME_OUT: {
					set_ack_timeout_cr(pAd, BN1_WF_TMAC_TOP_OMDTR_ADDR, timeout);
					break;
				}
				case ACK_ALL_TIME_OUT: {
					set_ack_timeout_cr(pAd, BN1_WF_TMAC_TOP_CDTR_ADDR, timeout);
					set_ack_timeout_cr(pAd, BN1_WF_TMAC_TOP_ODTR_ADDR, timeout);
					set_ack_timeout_cr(pAd, BN1_WF_TMAC_TOP_OMDTR_ADDR, timeout);
					break;
				}
				default: {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: invalid ackmode,  ackmode=%d!!\n",
					__FUNCTION__, ackmode));
					return FALSE;
				}
			}
			break;
		}
		default: {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: invalid BAND idx,  band_id=%d!!\n",
			__FUNCTION__, bandidx));
			return FALSE;
		}
	}
	return TRUE;
}

INT32 set_cck_ofdm_ofdma_tout (RTMP_ADAPTER *pAd, UINT32 timeout, ACK_TIMEOUT_MODE_T ack_mode)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR band_idx = 0;

	if (NULL == pAd || NULL == pObj) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: invalid null input: pAd=%p; pObj=%p!!\n",
			__FUNCTION__, pAd, pObj));
		return FALSE;
	}

	if (((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN))
		&& (pObj->ioctl_if < MAX_BEACON_NUM)) {
			wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: invalid interafce! or interface idx: %d!!\n",
			__FUNCTION__, pObj->ioctl_if));
		return FALSE;
	}

	/*GET BANDINDX*/
	band_idx = HcGetBandByWdev(wdev);

	if (FALSE == set_ack_timeout_mode_byband(pAd, timeout, band_idx, ack_mode)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s]:SET CTS/ACK Timeout Fail!!\n", __func__));
		return FALSE;
	}
	return TRUE;
}


INT32 set_datcfg_ack_cts_timeout (RTMP_ADAPTER *pAd)
{
	UCHAR idx = 0;
	UINT32 value = 0;

	if (NULL == pAd) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: invalid null input: pAd=%p;!!\n",
			__func__, pAd));
		return FALSE;
	}

	for (idx = 0; idx < DBDC_BAND_NUM; idx++) {
		if (pAd->CommonCfg.distance[idx] > 0) {
			value = pAd->CommonCfg.distance[idx];
			value = value*2/LIGHT_SPEED;

			if (TRUE != set_ack_timeout_mode_byband(pAd,
				value, idx, ACK_ALL_TIME_OUT)) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("[%s](%d): DAT config band(%u) Distance Fail!\n",
				__func__, __LINE__, idx));
			}
		} else {
			if (pAd->CommonCfg.cck_timeout[idx] > 0) {
				value = pAd->CommonCfg.cck_timeout[idx];
				if (TRUE != set_ack_timeout_mode_byband(pAd,
					value, idx, CCK_TIME_OUT)) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("[%s](%d): DAT config band(%u) cck_timeout Fail!\n",
					__func__, __LINE__, idx));
				}
			}

			if (pAd->CommonCfg.ofdm_timeout[idx] > 0) {
				value = pAd->CommonCfg.ofdm_timeout[idx];

				if (TRUE != set_ack_timeout_mode_byband(pAd,
					value, idx, OFDM_TIME_OUT)) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("[%s](%d): DAT config band(%u) ofdm_timeout Fail!\n",
					__func__, __LINE__, idx));
				}
			}

			if (pAd->CommonCfg.ofdma_timeout[idx] > 0) {
				value = pAd->CommonCfg.ofdma_timeout[idx];

				if (TRUE != set_ack_timeout_mode_byband(pAd,
					value, idx, OFDMA_TIME_OUT)) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("[%s](%d): DAT config band(%u) ofdma_timeout Fail!\n",
					__func__, __LINE__, idx));
				}
			}
		}
	}

	return TRUE;

}


INT set_dst2acktimeout_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 bTimeout = 0, distance = 0;

	if (arg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s]: No parameters!!\n", __FUNCTION__));
		return FALSE;
	}

	/* Get distance and store in wdev*/
	distance = os_str_tol(arg, 0, 10);

	/* Calculate, timeout=((distance/speed of light)*2) */
	bTimeout = distance/LIGHT_SPEED;
	bTimeout *= 2;

	if ((bTimeout <= 0) || (bTimeout > MAX_ACK_TIMEOUT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s]: CTS/ACK Timeout Range should between [0xFFFF:0)!!\n", __func__));
		return FALSE;
	}

	if (FALSE == set_cck_ofdm_ofdma_tout(pAd, bTimeout, ACK_ALL_TIME_OUT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s]:SET CTS/ACK Timeout Fail!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("[%s]:SET CTS/ACK Timeout SUCCESS!!\n", __func__));
	return TRUE;
}


INT set_ackcts_timeout_enable_porc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	UCHAR band_idx = 0;
	UCHAR enable = 0;

	if (NULL == pAd || NULL == pObj) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: invalid null input: pAd=%p; pObj=%p!!\n",
			__FUNCTION__, pAd, pObj));
		return FALSE;
	}

	if (((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN))
		&& (pObj->ioctl_if < MAX_BEACON_NUM)) {
			wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: invalid interafce! or interface idx: %d!!\n",
			__FUNCTION__, pObj->ioctl_if));
		return FALSE;
	}

	/*GET BANDINDX*/
	band_idx = HcGetBandByWdev(wdev);
	enable = os_str_tol(arg, 0, 10);

	pAd->CommonCfg.ack_cts_enable[band_idx] = (enable == 0 ? FALSE : TRUE);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("ACK_CTS_TOUT_ENABLE: %s\n", enable == 0 ? "DISABLE" : "ENABLE"));
	return TRUE;
}

INT set_cck_ack_timeout_porc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 bTimeout = 0;

	if (NULL == pAd) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s]: invalid null input: pAd=%p!!\n",
			__FUNCTION__, pAd));
		return FALSE;
	}

	bTimeout = os_str_tol(arg, 0, 10);

	if ((bTimeout <= 0) || (bTimeout > MAX_ACK_TIMEOUT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s]: CTS/ACK Timeout Range [0xFFFF:0)!!\n", __func__));
		return FALSE;
	}

	if (FALSE == set_cck_ofdm_ofdma_tout(pAd, bTimeout, CCK_TIME_OUT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s]: SET CTS/ACK Timeout Fail!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("[%s]:SET CCK CTS/ACK Timeout SUCCESS!!\n", __func__));
	return TRUE;
}

INT set_ofdm_ack_timeout_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 bTimeout = 0;

	if (NULL == pAd) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s]: invalid null input: pAd=%p!!\n",
			__FUNCTION__, pAd));
		return FALSE;
	}

	bTimeout = os_str_tol(arg, 0, 10);

	if ((bTimeout <= 0) || (bTimeout > MAX_ACK_TIMEOUT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s] CTS/ACK Timeout Range [0xFFFF:0)!!\n", __func__));
		return FALSE;
	}

	if (FALSE == set_cck_ofdm_ofdma_tout(pAd, bTimeout, OFDM_TIME_OUT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s]SET CTS/ACK Timeout Fail!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("[%s]:SET OFDM CTS/ACK Timeout SUCCESS!!\n", __func__));
	return TRUE;
}

INT set_ofdma_ack_timeout_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32  bTimeout = 0;

	if (NULL == pAd) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s]: invalid null input: pAd=%p!!\n",
			__FUNCTION__, pAd));
		return FALSE;
	}

	bTimeout = os_str_tol(arg, 0, 10);

	if ((bTimeout <= 0) || (bTimeout > MAX_ACK_TIMEOUT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s]CTS/ACK Timeout Range [0xFFFF:0)!!\n", __func__));
		return FALSE;
	}

	if (FALSE == set_cck_ofdm_ofdma_tout(pAd, bTimeout, OFDMA_TIME_OUT)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s]SET CTS/ACK Timeout Fail!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("[%s]:SET OFDMA CTS/ACK Timeout SUCCESS!!\n", __func__));
	return TRUE;
}


#endif/*ACK_CTS_TIMEOUT_SUPPORT*/

#ifdef MLME_MULTI_QUEUE_SUPPORT
INT set_mlme_queue_ration(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_STRING *str1  = NULL;
	RTMP_STRING *str2  = NULL;
	UCHAR hp_q_ration = RATION_OF_MLME_HP_QUEUE;
	UCHAR np_q_ration = RATION_OF_MLME_QUEUE;
	UCHAR lp_q_ration = RATION_OF_MLME_LP_QUEUE;
	if (arg == NULL || strlen(arg) == 0)
		goto error;
	str1 = strsep(&arg, "-");
	str2 = strsep(&arg, "-");
	if (str1 == NULL || str2 == NULL ||  arg == NULL)
		goto error;
	hp_q_ration = os_str_tol(str1, 0, 10);
	np_q_ration = os_str_tol(str2, 0, 10);
	lp_q_ration = os_str_tol(arg, 0, 10);
	pAd->Mlme.HPQueue.Ration = hp_q_ration;
	pAd->Mlme.Queue.Ration = np_q_ration;
	pAd->Mlme.LPQueue.Ration = lp_q_ration;
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s(): [hp_q_ration]-[np_q_ration]-[lp_q_ration] = %d-%d-%d\n",
		__func__, hp_q_ration, np_q_ration, lp_q_ration));
	return TRUE;
error:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): Invalid parameter, format: [hp_q_ration]-[np_q_ration]-[lp_q_ration] \n",__func__));
	return FALSE;
}
#endif /*MLME_MULTI_QUEUE_SUPPORT*/

#ifdef CFG_SUPPORT_CSI
bool wlanPushCSIData(RTMP_ADAPTER *pAd, struct CSI_DATA_T *prCSIData)
{
	struct CSI_INFO_T *prCSIInfo = &(pAd->rCSIInfo);

	NdisAcquireSpinLock(&prCSIInfo->CSIBufferLock);

	/* Put the CSI data into CSI event queue */
	if (prCSIInfo->u4CSIBufferUsed != 0) {
		prCSIInfo->u4CSIBufferTail++;
		prCSIInfo->u4CSIBufferTail %= CSI_RING_SIZE;
	}

	os_move_mem(&(prCSIInfo->arCSIBuffer[prCSIInfo->u4CSIBufferTail]),
		prCSIData, sizeof(struct CSI_DATA_T));

	if (prCSIInfo->u4CSIBufferUsed < CSI_RING_SIZE) {
		prCSIInfo->u4CSIBufferUsed++;
	} else {
		/*
		 * While new CSI event comes and the ring buffer is
		 * already full, the new coming CSI event will
		 * overwrite the oldest one in the ring buffer.
		 * Thus, the Head pointer which points to * the
		 * oldest CSI event in the buffer should be moved too.
		 */
		prCSIInfo->u4CSIBufferHead++;
		prCSIInfo->u4CSIBufferHead %= CSI_RING_SIZE;
	}

	NdisReleaseSpinLock(&prCSIInfo->CSIBufferLock);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
	("%s: CSIBufferUsed=%d,CSIBufferHead=%d,CSIBufferTail=%d\n", __func__,
		prCSIInfo->u4CSIBufferUsed, prCSIInfo->u4CSIBufferHead, prCSIInfo->u4CSIBufferTail));
	return TRUE;
}

bool wlanPopCSIData(RTMP_ADAPTER *pAd, struct CSI_DATA_T *prCSIData)
{
	struct CSI_INFO_T *prCSIInfo = &pAd->rCSIInfo;

	NdisAcquireSpinLock(&prCSIInfo->CSIBufferLock);

	/*No CSI data in the ring buffer*/
	if (prCSIInfo->u4CSIBufferUsed == 0) {
		NdisReleaseSpinLock(&prCSIInfo->CSIBufferLock);
		return FALSE;
	}

	os_move_mem(prCSIData,
		&(prCSIInfo->arCSIBuffer[prCSIInfo->u4CSIBufferHead]),
		sizeof(struct CSI_DATA_T));

	prCSIInfo->u4CSIBufferUsed--;
	if (prCSIInfo->u4CSIBufferUsed != 0) {
		prCSIInfo->u4CSIBufferHead++;
		prCSIInfo->u4CSIBufferHead %= CSI_RING_SIZE;
	}
	NdisReleaseSpinLock(&prCSIInfo->CSIBufferLock);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
	("%s: CSIBufferUsed=%d,CSIBufferHead=%d,CSIBufferTail=%d\n", __func__,
		prCSIInfo->u4CSIBufferUsed, prCSIInfo->u4CSIBufferHead, prCSIInfo->u4CSIBufferTail));

	return TRUE;
}

/*
*CSI TONE MASK
*this function mask(clear) the null tone && pilot tones
*for example, bw20 has  64 tones in total.however there
*are some null tone && pilot tones we do not need for the
*feature of channel state information(CSI).
*so we need to clear these tone.
*/
VOID wlanApplyCSIToneMask(
	UINT_8 ucRxMode,
	UINT_8 ucCBW,
	UINT_8 ucDBW,
	UINT_8 ucPrimaryChIdx,
	INT_16 *ai2IData,
	INT_16 *ai2QData)
{
	UINT_8 ucSize = sizeof(INT_16);

#define ZERO(index) \
{ ai2IData[index] = 0; ai2QData[index] = 0; }

#define ZERO_RANGE(start, end) \
{\
	os_zero_mem(&ai2IData[start], ucSize * (end - start + 1));\
	os_zero_mem(&ai2QData[start], ucSize * (end - start + 1));\
}
	/*Mask the NULL TONE*/
	if (ucRxMode == RX_VT_LEGACY_OFDM) {
		if (ucCBW == RX_VT_FR_MODE_20) {
			ZERO(0);
			ZERO_RANGE(27, 37);
		} else if (ucCBW == RX_VT_FR_MODE_40) {
			if (ucDBW == RX_VT_FR_MODE_40) {
				ZERO(32); ZERO(96);
				ZERO_RANGE(0, 5);
				ZERO_RANGE(59, 69);
				ZERO_RANGE(123, 127);
			} else if (ucDBW == RX_VT_FR_MODE_20) {
				if (ucPrimaryChIdx == 0) {
					ZERO(96);
					ZERO_RANGE(0, 69);
					ZERO_RANGE(123, 127);
				} else {
					ZERO(32);
					ZERO_RANGE(0, 5);
					ZERO_RANGE(59, 127);
				}
			}
		} else if (ucCBW == RX_VT_FR_MODE_80) {
			if (ucDBW == RX_VT_FR_MODE_80) {
				ZERO(32); ZERO(96);
				ZERO(160); ZERO(224);
				ZERO_RANGE(0, 5);
				ZERO_RANGE(59, 69);
				ZERO_RANGE(123, 133);
				ZERO_RANGE(187, 197);
				ZERO_RANGE(251, 255);
			} else if (ucDBW == RX_VT_FR_MODE_40) {
				if (ucPrimaryChIdx <= 1) {
					ZERO(160); ZERO(224);
					ZERO_RANGE(0, 133);
					ZERO_RANGE(187, 197);
					ZERO_RANGE(251, 255);
				} else {
					ZERO(32); ZERO(96);
					ZERO_RANGE(0, 5);
					ZERO_RANGE(59, 69);
					ZERO_RANGE(123, 255);
				}
			} else if (ucDBW == RX_VT_FR_MODE_20) {
				if (ucPrimaryChIdx == 0) {
					ZERO(160);
					ZERO_RANGE(0, 133);
					ZERO_RANGE(187, 255);
				} else if (ucPrimaryChIdx == 1) {
					ZERO(224);
					ZERO_RANGE(0, 197);
					ZERO_RANGE(251, 255);
				} else if (ucPrimaryChIdx == 2) {
					ZERO(32);
					ZERO_RANGE(0, 5);
					ZERO_RANGE(59, 255);
				} else {
					ZERO(96);
					ZERO_RANGE(0, 69);
					ZERO_RANGE(123, 255);
				}
			}
		}
	} else if (ucRxMode == RX_VT_MIXED_MODE ||
		ucRxMode == RX_VT_GREEN_MODE ||
		ucRxMode == RX_VT_VHT_MODE) {
		if (ucCBW == RX_VT_FR_MODE_20) {
			ZERO(0);
			ZERO_RANGE(29, 35);
		} else if (ucCBW == RX_VT_FR_MODE_40) {
			if (ucDBW == RX_VT_FR_MODE_40) {
				ZERO(0); ZERO(1); ZERO(127);
				ZERO_RANGE(59, 69);
			} else if (ucDBW == RX_VT_FR_MODE_20) {
				if (ucPrimaryChIdx == 0) {
					ZERO(96);
					ZERO_RANGE(0, 67);
					ZERO_RANGE(125, 127);
				} else {
					ZERO(32);
					ZERO_RANGE(0, 3);
					ZERO_RANGE(61, 127);
				}
			}
		} else if (ucCBW == RX_VT_FR_MODE_80) {
			if (ucDBW == RX_VT_FR_MODE_80) {
				ZERO(0); ZERO(1); ZERO(255);
				ZERO_RANGE(123, 133);
			} else if (ucDBW == RX_VT_FR_MODE_40) {
				if (ucPrimaryChIdx <= 1) {
					ZERO_RANGE(0, 133);
					ZERO_RANGE(191, 193);
					ZERO_RANGE(251, 255);
				} else {
					ZERO_RANGE(0, 5);
					ZERO_RANGE(63, 65);
					ZERO_RANGE(123, 127);
				}
			} else if (ucDBW == RX_VT_FR_MODE_20) {
				if (ucPrimaryChIdx == 0) {
					ZERO(160);
					ZERO_RANGE(0, 131);
					ZERO_RANGE(189, 255);
				} else if (ucPrimaryChIdx == 1) {
					ZERO(224);
					ZERO_RANGE(0, 195);
					ZERO_RANGE(253, 255);
				} else if (ucPrimaryChIdx == 2) {
					ZERO(32);
					ZERO_RANGE(0, 3);
					ZERO_RANGE(61, 255);
				} else {
					ZERO(96);
					ZERO_RANGE(0, 67);
					ZERO_RANGE(125, 255);
				}
			}
		}
	} else if (ucRxMode == RX_VT_HE_MODE) {		/*11ax support*/
		if (ucCBW == RX_VT_FR_MODE_20) {
			ZERO(0);
			ZERO_RANGE(31, 33);
		} else if (ucCBW == RX_VT_FR_MODE_40) {
			if (ucDBW == RX_VT_FR_MODE_40) {
				ZERO(0);
				ZERO_RANGE(62, 66);
			} else if (ucDBW == RX_VT_FR_MODE_20) {
				if (ucPrimaryChIdx == 0) {
					ZERO(96); ZERO(127);
					ZERO_RANGE(0, 65);
				} else {
					ZERO(0);
					ZERO(1); ZERO(32);
					ZERO_RANGE(63, 127);
				}
			}
		} else if (ucCBW == RX_VT_FR_MODE_80) {
			if (ucDBW == RX_VT_FR_MODE_80) {
				ZERO(0);
				ZERO_RANGE(126, 130);
			} else if (ucDBW == RX_VT_FR_MODE_40) {
				if (ucPrimaryChIdx <= 1) {
					ZERO_RANGE(0, 130);
					ZERO(192);
					ZERO_RANGE(254, 255);
				} else {
					ZERO_RANGE(0, 2);
					ZERO(64);
					ZERO_RANGE(126, 255);
				}
			} else if (ucDBW == RX_VT_FR_MODE_20) {
				if (ucPrimaryChIdx == 0) {
					ZERO_RANGE(0, 129);
					ZERO(160);
					ZERO_RANGE(191, 255);
				} else if (ucPrimaryChIdx == 1) {
					ZERO_RANGE(0, 193);
					ZERO(224);
					ZERO(255);
				} else if (ucPrimaryChIdx == 2) {
					ZERO_RANGE(0, 1);
					ZERO(32);
					ZERO_RANGE(63, 255);
				} else {
					ZERO_RANGE(0, 65);
					ZERO(96);
					ZERO_RANGE(127, 255);
				}
			}
		}
	}

	/*Mask the VHT Pilots*/
	if (ucRxMode == RX_VT_VHT_MODE) {
		if (ucCBW == RX_VT_FR_MODE_20) {
			ZERO(7); ZERO(21); ZERO(43); ZERO(57);
		} else if (ucCBW == RX_VT_FR_MODE_40) {
			if (ucDBW == RX_VT_FR_MODE_40) {
				ZERO(11); ZERO(25); ZERO(53);
				ZERO(75); ZERO(103); ZERO(117);
			} else if (ucDBW == RX_VT_FR_MODE_20) {
				if (ucPrimaryChIdx == 0) {
					ZERO(75); ZERO(89);
					ZERO(103); ZERO(117);
				} else {
					ZERO(11); ZERO(25);
					ZERO(39); ZERO(53);
				}
			}
		} else if (ucCBW == RX_VT_FR_MODE_80) {
			if (ucDBW == RX_VT_FR_MODE_80) {
				ZERO(11); ZERO(39); ZERO(75); ZERO(103);
				ZERO(153); ZERO(181); ZERO(217); ZERO(245);
			} else if (ucDBW == RX_VT_FR_MODE_40) {
				if (ucPrimaryChIdx <= 1) {
					ZERO(139); ZERO(167); ZERO(181);
					ZERO(203); ZERO(217); ZERO(245);
				} else {
					ZERO(11); ZERO(39); ZERO(53);
					ZERO(75); ZERO(89); ZERO(117);
				}
			} else if (ucDBW == RX_VT_FR_MODE_20) {
				if (ucPrimaryChIdx == 0) {
					ZERO(139); ZERO(153);
					ZERO(167); ZERO(181);
				} else if (ucPrimaryChIdx == 1) {
					ZERO(203); ZERO(217);
					ZERO(231); ZERO(245);
				} else if (ucPrimaryChIdx == 2) {
					ZERO(11); ZERO(25);
					ZERO(39); ZERO(53);
				} else {
					ZERO(75); ZERO(89);
					ZERO(103); ZERO(117);
				}
			}
		}
	} else if (ucRxMode == RX_VT_HE_MODE) {			/*11ax support*/
		if (ucCBW == RX_VT_FR_MODE_20) {
			ZERO(12); ZERO(29);
			ZERO(52); ZERO(35);
		} else if (ucCBW == RX_VT_FR_MODE_40) {
			if (ucDBW == RX_VT_FR_MODE_40) {
				ZERO(9); ZERO(26);
				ZERO(36); ZERO(53);
				ZERO(119); ZERO(102);
				ZERO(92); ZERO(75);
			} else if (ucDBW == RX_VT_FR_MODE_20) {
				if (ucPrimaryChIdx == 0) {
					ZERO(67); ZERO(84);
					ZERO(108); ZERO(125);
				} else {
					ZERO(3); ZERO(20);
					ZERO(61); ZERO(44);
				}
			}
		} else if (ucCBW == RX_VT_FR_MODE_80) {
			if (ucDBW == RX_VT_FR_MODE_80) {
				ZERO(6); ZERO(23);
				ZERO(100); ZERO(117);
				ZERO(250); ZERO(233);
				ZERO(156); ZERO(139);
			} else if (ucDBW == RX_VT_FR_MODE_40) {
				if (ucPrimaryChIdx <= 1) {
					ZERO(201); ZERO(218);
					ZERO(228); ZERO(245);
					ZERO(183); ZERO(166);
					ZERO(156); ZERO(139);
				} else {
					ZERO(73); ZERO(90);
					ZERO(100); ZERO(117);
					ZERO(55); ZERO(38);
					ZERO(28); ZERO(11);
				}
			} else if (ucDBW == RX_VT_FR_MODE_20) {
				if (ucPrimaryChIdx == 0) {
					ZERO(172); ZERO(189);
					ZERO(148); ZERO(131);
				} else if (ucPrimaryChIdx == 1) {
					ZERO(236); ZERO(253);
					ZERO(212); ZERO(195);
				} else if (ucPrimaryChIdx == 2) {
					ZERO(44); ZERO(61);
					ZERO(20); ZERO(3);
				} else {
					ZERO(108); ZERO(125);
					ZERO(84); ZERO(67);
				}
			}
		}
		}
}
/*
*CSI TONE SHIFT
*for example, statiosn may use 20MHz to send data
*even if it's channel bandwidth is 40MHz.
*in this way, there are many zero tone(have been masked) is the buffer.
*some customers would like to arrange the valid tones in the buffer.
*/
VOID wlanShiftCSI(
	UINT_8 ucRxMode,
	UINT_8 ucCBW,
	UINT_8 ucDBW,
	UINT_8 ucPrimaryChIdx,
	INT_16 *ai2IData,
	INT_16 *ai2QData,
	INT_16 *ai2ShiftIData,
	INT_16 *ai2ShiftQData)
{
	UINT_8 ucSize = sizeof(INT_16);
#define COPY_RANGE(dest, start, end) \
{\
	os_move_mem(&ai2ShiftIData[dest], \
		&ai2IData[start], ucSize * (end - start + 1)); \
	os_move_mem(&ai2ShiftQData[dest], \
		&ai2QData[start], ucSize * (end - start + 1)); \
}

#define COPY(dest, src) \
{ ai2ShiftIData[dest] = ai2IData[src]; ai2ShiftQData[dest] = ai2QData[src]; }

	if (ucRxMode == RX_VT_LEGACY_OFDM) {
		if (ucCBW == RX_VT_FR_MODE_20) {
			COPY_RANGE(0, 0, 63);
		} else if (ucCBW == RX_VT_FR_MODE_40) {
			if (ucDBW == RX_VT_FR_MODE_40) {
				COPY_RANGE(0, 0, 127);
			} else if (ucDBW == RX_VT_FR_MODE_20) {
				if (ucPrimaryChIdx == 0) {
					COPY(0, 96);
					COPY_RANGE(38, 70, 95);
					COPY_RANGE(1, 97, 122);
				} else {
					COPY(0, 32);
					COPY_RANGE(38, 6, 31);
					COPY_RANGE(1, 33, 58);
				}
			}
		} else if (ucCBW == RX_VT_FR_MODE_80) {
			if (ucDBW == RX_VT_FR_MODE_80) {
				COPY_RANGE(0, 0, 255);
			} else if (ucDBW == RX_VT_FR_MODE_40) {
				if (ucPrimaryChIdx <= 1) {
					COPY(0, 192);
					COPY_RANGE(2, 198, 250);
					COPY_RANGE(74, 134, 186);
				} else {
					COPY(0, 64);
					COPY_RANGE(2, 70, 122);
					COPY_RANGE(74, 6, 58);
				}
			} else if (ucDBW == RX_VT_FR_MODE_20) {
				if (ucPrimaryChIdx == 0) {
					COPY(0, 160);
					COPY_RANGE(1, 161, 186);
					COPY_RANGE(38, 134, 159);
				} else if (ucPrimaryChIdx == 1) {
					COPY(0, 224);
					COPY_RANGE(1, 225, 250);
					COPY_RANGE(38, 198, 223);
				} else if (ucPrimaryChIdx == 2) {
					COPY(0, 32);
					COPY_RANGE(1, 33, 58);
					COPY_RANGE(38, 6, 31);
				} else {
					COPY(0, 96);
					COPY_RANGE(1, 97, 122);
					COPY_RANGE(38, 70, 95);
				}
			}
		}
	} else if (ucRxMode == RX_VT_MIXED_MODE ||
		ucRxMode == RX_VT_GREEN_MODE ||
		ucRxMode == RX_VT_VHT_MODE) {
		if (ucCBW == RX_VT_FR_MODE_20) {
			COPY_RANGE(0, 0, 63);
		} else if (ucCBW == RX_VT_FR_MODE_40) {
			if (ucDBW == RX_VT_FR_MODE_40) {
				COPY_RANGE(0, 0, 127);
			} else if (ucDBW == RX_VT_FR_MODE_20) {
				if (ucPrimaryChIdx == 0) {
					COPY(0, 96);
					COPY_RANGE(36, 68, 95);
					COPY_RANGE(1, 97, 124);
				} else {
					COPY(0, 32);
					COPY_RANGE(36, 4, 31);
					COPY_RANGE(1, 33, 60);
				}
			}
		} else if (ucCBW == RX_VT_FR_MODE_80) {
			if (ucDBW == RX_VT_FR_MODE_80) {
				COPY_RANGE(0, 0, 255);
			} else if (ucDBW == RX_VT_FR_MODE_40) {
				if (ucPrimaryChIdx <= 1) {
					COPY(0, 192);
					COPY_RANGE(2, 194, 250);
					COPY_RANGE(70, 134, 190);
				} else {
					COPY(0, 64);
					COPY_RANGE(2, 66, 122);
					COPY_RANGE(70, 6, 62);
				}
			} else if (ucDBW == RX_VT_FR_MODE_20) {
				if (ucPrimaryChIdx == 0) {
					COPY(0, 160);
					COPY_RANGE(1, 161, 188);
					COPY_RANGE(36, 132, 159);
				} else if (ucPrimaryChIdx == 1) {
					COPY(0, 224);
					COPY_RANGE(1, 225, 252);
					COPY_RANGE(36, 196, 223);
				} else if (ucPrimaryChIdx == 2) {
					COPY(0, 32);
					COPY_RANGE(1, 33, 60);
					COPY_RANGE(36, 4, 31);
				} else {
					COPY(0, 96);
					COPY_RANGE(1, 97, 124);
					COPY_RANGE(36, 68, 95);
				}
			}
		}
	}
}
#endif

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
INT show_Customer_Vie_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    INT i, len = 0, ret, apidx;
    UCHAR *tmp, *buf = NULL;
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
    struct customer_vendor_ie *ap_vendor_ie;
    struct wifi_dev *wdev = NULL;
   
    /* check if the interface is down */
    if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("INFO::Network is down!\n"));
        return FALSE;
    }

    apidx = pObj->ioctl_if;
    wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
    ap_vendor_ie = &pAd->ApCfg.MBSSID[apidx].ap_vendor_ie;

    RTMP_SPIN_LOCK(&ap_vendor_ie->vendor_ie_lock);
    tmp = ap_vendor_ie->pointer;
    if (NULL != tmp) 
    {
        len = ap_vendor_ie->length;
        ret = os_alloc_mem(NULL, &buf, len);
        if (ret == NDIS_STATUS_SUCCESS) 
        {
            memcpy(buf, tmp, len);
        }
        else
        {
            RTMP_SPIN_UNLOCK(&ap_vendor_ie->vendor_ie_lock);
            return FALSE;
        }
    }
    RTMP_SPIN_UNLOCK(&ap_vendor_ie->vendor_ie_lock);

    if (buf != NULL)
    {
        for (i=0; i<len; i++)
        {
            printk("%02X", tmp[i]);
        }

        if (i > 0)
        {
            printk("\n");
        }
        os_free_mem(buf);
    }
	return TRUE;
}

static char uitl_charToHex(UCHAR c)
{
    if (c >= '0' && c <= '9')
    {
        return (c - '0');
    }

    if (c >= 'a' && c <= 'f')
    {
        return (c - 'a' + 10);
    }

    if (c >= 'A' && c <= 'F')
    {
        return (c - 'A' + 10);
    }

    return -1;
}
static int uitl_strToHex(UCHAR *str, UCHAR *pszHex, int len)
{
    char high = 0;
    char low  = 0;
    int i = 0;
    int strlength = strlen(str);
    char tmp = 0;

    if((strlength % 2) != 0)
        return -1;

    while(strlength > 0)
    {
        high = uitl_charToHex(*(str+i));
        low  = uitl_charToHex(*(str+i+1));
        tmp = (high << 4) + low;

        if((i/2) >= len)
            break;

        pszHex[i/2] = tmp;
        i = i + 2;
        strlength = strlength - 2;
    }

    return 0;
}

INT Set_Customer_Vie_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    INT apidx, len, ret;
    UCHAR vie_len = 0, *buf = NULL, *tmp = NULL;
    char high, low;
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
    struct customer_vendor_ie *ap_vendor_ie;
    struct wifi_dev *wdev = NULL;

    apidx = pObj->ioctl_if;
    wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

    /* check if the interface is down */
    if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;
    }

    buf = arg;
    len = strlen(buf);
    if (len < 10)/*5x2 dd+len+oui*/
    {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s():: Invalid vie len\n", __func__));
        return -EINVAL;
    }
    if (0 != strncasecmp(buf, "dd", 2))
    {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s():: Invalid vie\n", __func__));
        return -EINVAL;
    }

    high = uitl_charToHex(*(buf+2));
    low  = uitl_charToHex(*(buf+3));
    vie_len = (high << 4) + low;
    if ((vie_len + 2) != (len>>1))
    {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s():: Invalid vie len\n", __func__));
        return -EINVAL;
    }

    ret = os_alloc_mem(NULL, (UCHAR **)&tmp, len);
    if (ret == NDIS_STATUS_SUCCESS) 
    {
        os_zero_mem(tmp, len);
        if (-1 == uitl_strToHex(buf, tmp, len))
        {
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s():: set vie fail\n", __func__));
            os_free_mem(tmp);
            return -EINVAL;
        }

        ap_vendor_ie = &pAd->ApCfg.MBSSID[apidx].ap_vendor_ie;
        RTMP_SPIN_LOCK(&ap_vendor_ie->vendor_ie_lock);
        if (ap_vendor_ie->pointer != NULL)
        {
            os_free_mem(ap_vendor_ie->pointer);
        }
        ap_vendor_ie->pointer = tmp;
        ap_vendor_ie->length = len/2;
        RTMP_SPIN_UNLOCK(&ap_vendor_ie->vendor_ie_lock);

        /* start sending BEACON out */
        UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
    }

    return TRUE;
}
#endif
