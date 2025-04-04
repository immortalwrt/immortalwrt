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
	cmm_profile.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#include "rt_config.h"
#include "hdev/hdev.h"

#define ETH_MAC_ADDR_STR_LEN 17  /* in format of xx:xx:xx:xx:xx:xx*/

/* We assume the s1 is a sting, s2 is a memory space with 6 bytes. and content of s1 will be changed.*/
BOOLEAN rtstrmactohex(RTMP_STRING *s1, RTMP_STRING *s2)
{
	int i = 0;
	RTMP_STRING *ptokS = s1, *ptokE;

	if (strlen(s1) != ETH_MAC_ADDR_STR_LEN)
		return FALSE;

	while ((*ptokS) != '\0') {
		ptokE = strchr(ptokS, ':');

		if (ptokE != NULL)
			*ptokE++ = '\0';

		if ((strlen(ptokS) != 2) || (!isxdigit(*ptokS)) || (!isxdigit(*(ptokS + 1))))
			break; /* fail*/

		AtoH(ptokS, (PUCHAR)&s2[i++], 1);
		ptokS = ptokE;

		if (ptokS == NULL)
			break;

		if (i == 6)
			break; /* parsing finished*/
	}

	return (i == 6 ? TRUE : FALSE);
}


#define ASC_LOWER(_x)	((((_x) >= 0x41) && ((_x) <= 0x5a)) ? (_x) + 0x20 : (_x))
/* we assume the s1 and s2 both are strings.*/
BOOLEAN rtstrcasecmp(RTMP_STRING *s1, RTMP_STRING *s2)
{
	RTMP_STRING *p1 = s1, *p2 = s2;
	CHAR c1, c2;

	if (strlen(s1) != strlen(s2))
		return FALSE;

	while (*p1 != '\0') {
		c1 = ASC_LOWER(*p1);
		c2 = ASC_LOWER(*p2);

		if (c1 != c2)
			return FALSE;

		p1++;
		p2++;
	}

	return TRUE;
}


/* we assume the s1 (buffer) and s2 (key) both are strings.*/
RTMP_STRING *rtstrstruncasecmp(RTMP_STRING *s1, RTMP_STRING *s2)
{
	INT l1, l2, i;
	char temp1, temp2;

	l2 = strlen(s2);

	if (!l2)
		return (char *) s1;

	l1 = strlen(s1);

	while (l1 >= l2) {
		l1--;

		for (i = 0; i < l2; i++) {
			temp1 = *(s1 + i);
			temp2 = *(s2 + i);

			if (('a' <= temp1) && (temp1 <= 'z'))
				temp1 = 'A' + (temp1 - 'a');

			if (('a' <= temp2) && (temp2 <= 'z'))
				temp2 = 'A' + (temp2 - 'a');

			if (temp1 != temp2)
				break;
		}

		if (i == l2)
			return (char *) s1;

		s1++;
	}

	return NULL; /* not found*/
}


/**
 * strstr - Find the first substring in a %NUL terminated string
 * @s1: The string to be searched
 * @s2: The string to search for
 */
RTMP_STRING *rtstrstr(const RTMP_STRING *s1, const RTMP_STRING *s2)
{
	INT l1, l2;

	l2 = strlen(s2);

	if (!l2)
		return (RTMP_STRING *)s1;

	l1 = strlen(s1);

	while (l1 >= l2) {
		l1--;

		if (!memcmp(s1, s2, l2))
			return (RTMP_STRING *)s1;

		s1++;
	}

	return NULL;
}

/**
 * rstrtok - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 * * WARNING: strtok is deprecated, use strsep instead. However strsep is not compatible with old architecture.
 */
RTMP_STRING *__rstrtok;
RTMP_STRING *rstrtok(RTMP_STRING *s, const RTMP_STRING *ct)
{
	RTMP_STRING *sbegin, *send;

	sbegin  = s ? s : __rstrtok;

	if (!sbegin)
		return NULL;

	sbegin += strspn(sbegin, ct);

	if (*sbegin == '\0') {
		__rstrtok = NULL;
		return NULL;
	}

	send = strpbrk(sbegin, ct);

	if (send && *send != '\0')
		*send++ = '\0';

	__rstrtok = send;
	return sbegin;
}

/**
 * delimitcnt - return the count of a given delimiter in a given string.
 * @s: The string to be searched.
 * @ct: The delimiter to search for.
 * Notice : We suppose the delimiter is a single-char string(for example : ";").
 */
INT delimitcnt(RTMP_STRING *s, RTMP_STRING *ct)
{
	INT count = 0;
	/* point to the beginning of the line */
	RTMP_STRING *token = s;

	for (;; ) {
		token = strpbrk(token, ct); /* search for delimiters */

		if (token == NULL) {
			/* advanced to the terminating null character */
			break;
		}

		/* skip the delimiter */
		++token;
		/*
		 * Print the found text: use len with %.*s to specify field width.
		 */
		/* accumulate delimiter count */
		++count;
	}

	return count;
}

/*
  * converts the Internet host address from the standard numbers-and-dots notation
  * into binary data.
  * returns nonzero if the address is valid, zero if not.
  */
int rtinet_aton(const RTMP_STRING *cp, unsigned int *addr)
{
	unsigned int	val;
	int	base, n;
	RTMP_STRING c;
	unsigned int    parts[4] = {0};
	unsigned int    *pp = parts;

	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 *	0x=hex, 0=octal, other=decimal.
		 */
		val = 0;
		base = 10;

		if (*cp == '0') {
			if (*++cp == 'x' || *cp == 'X')
				base = 16, cp++;
			else
				base = 8;
		}

		while ((c = *cp) != '\0') {
			if (isdigit((unsigned char) c)) {
				val = (val * base) + (c - '0');
				cp++;
				continue;
			}

			if (base == 16 && isxdigit((unsigned char) c)) {
				val = (val << 4) +
					  (c + 10 - (islower((unsigned char) c) ? 'a' : 'A'));
				cp++;
				continue;
			}

			break;
		}

		if (*cp == '.') {
			/*
			 * Internet format: a.b.c.d a.b.c   (with c treated as 16-bits)
			 * a.b     (with b treated as 24 bits)
			 */
			if (pp >= parts + 3 || val > 0xff)
				return 0;

			*pp++ = val, cp++;
		} else
			break;
	}

	/*
	 * Check for trailing junk.
	 */
	while (*cp)
		if (!isspace((unsigned char) *cp++))
			return 0;

	/*
	 * Concoct the address according to the number of parts specified.
	 */
	n = pp - parts + 1;

	switch (n) {
	case 1:         /* a -- 32 bits */
		break;

	case 2:         /* a.b -- 8.24 bits */
		if (val > 0xffffff)
			return 0;

		val |= parts[0] << 24;
		break;

	case 3:         /* a.b.c -- 8.8.16 bits */
		if (val > 0xffff)
			return 0;

		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:         /* a.b.c.d -- 8.8.8.8 bits */
		if (val > 0xff)
			return 0;

		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}

	*addr = OS_HTONL(val);
	return 1;
}

#if defined(CONFIG_AP_SUPPORT) || defined(CONFIG_STA_SUPPORT)
static UCHAR GetDefaultChannel(USHORT PhyMode)
{
	/*priority must the same as Default PhyMode*/
	if (WMODE_CAP_2G(PhyMode))
		return 1;
	else if (WMODE_CAP_5G(PhyMode))
		return 36;

	return 1;
}
#endif /* defined(CONFIG_AP_SUPPORT) || defined(CONFIG_STA_SUPPORT) */


static VOID RTMPChannelCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UINT32 i;
	CHAR *macptr;
	struct wifi_dev *wdev;
	UCHAR Channel;
#ifdef CONFIG_AP_SUPPORT
		UINT32 j = 0;
#endif

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		Channel = os_str_tol(macptr, 0, 10);
#ifdef CONFIG_AP_SUPPORT

		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (i >= DBDC_BAND_NUM)
				break;

			for (j = 0; j < pAd->ApCfg.BssidNum; j++) {

				wdev = &pAd->ApCfg.MBSSID[j].wdev;
					/*When Enable AutoChannelSelect(ACS), we assume that the configuration parameters(bAutoChannelAtBootup) for ACS are correct*/
					if (pAd->CommonCfg.dbdc_mode) {
						if (Channel > 14) {
							if (pAd->CommonCfg.eDBDC_mode == ENUM_DBDC_5G5G) {/*MT7915A 5G Low + 5G High*/
								if ((Channel <= 100) && (!pAd->ApCfg.bAutoChannelAtBootup[DBDC_BAND0]))
									wdev->channel = Channel;
#ifdef DBDC_MODE
								else if ((Channel > 100) && (!pAd->ApCfg.bAutoChannelAtBootup[DBDC_BAND1]))
									wdev->channel = Channel;
#endif
								else
									MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Warning:ACS Enable, the Channel of config parameter must set 0:\n");
							} else if (WMODE_CAP_5G(wdev->PhyMode)) {
#ifdef DBDC_MODE
									if (!pAd->ApCfg.bAutoChannelAtBootup[DBDC_BAND1]) {
										wdev->channel = Channel;
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
										pAd->CommonCfg.usr_channel = Channel;
#endif
									}
									else if (pAd->ApCfg.bAutoChannelAtBootup[DBDC_BAND1])
										MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Warning:ACS Enable and the 5G Channel of config parameter must set 0:\n");
#endif
							}
						} else if ((0 < Channel) && (Channel <= 14)) {
							/* Handle channel 1 ~ 14 case for 2G/6G */
							if ((i == 0) && WMODE_CAP_2G(wdev->PhyMode)) {
								/* i = 0 for 2G Band. */
								if (!pAd->ApCfg.bAutoChannelAtBootup[DBDC_BAND0])
									wdev->channel = Channel;
								else if (pAd->ApCfg.bAutoChannelAtBootup[DBDC_BAND0])
									MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
										"Warning:ACS Enable and the 2G Channel of config parameter must set 0:\n");
							} else if ((i == 1) && WMODE_CAP_6G(wdev->PhyMode)) {
								/* i = 1 for 6G Band. */
								if (!pAd->ApCfg.bAutoChannelAtBootup[DBDC_BAND1])
									wdev->channel = Channel;
								else if (pAd->ApCfg.bAutoChannelAtBootup[DBDC_BAND1])
									MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
										"Warning:ACS Enable and the 6G Channel of config parameter must set 0:\n");
							}
						} else if ((0 == Channel) && (!wdev->channel)) {
								if (!pAd->ApCfg.bAutoChannelAtBootup[DBDC_BAND0] && WMODE_CAP_2G(wdev->PhyMode)) {
									wdev->channel = GetDefaultChannel(WMODE_B);
									MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Error:ACS Disable, but the 2G Channel of config parameter invalid,so default channel:\n");
#ifdef DBDC_MODE
								} else if (!pAd->ApCfg.bAutoChannelAtBootup[DBDC_BAND1] && WMODE_CAP_5G(wdev->PhyMode)) {
									wdev->channel = GetDefaultChannel(WMODE_A);
									MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Error:ACS Disable, but the 5G Channel of config parameter invalid,so default channel:\n");
#endif
								}
						}
					} else if (0 < Channel) {
							if (!pAd->ApCfg.bAutoChannelAtBootup[DBDC_BAND0]) {
								wdev->channel = Channel;
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
								if (Channel > 14)
									pAd->CommonCfg.usr_channel = Channel;
#endif
							} else
								MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "%s:Warning:ACS Enable and the Channel of config parameter must set 0:\n", __func__);
					} else if (!pAd->ApCfg.bAutoChannelAtBootup[DBDC_BAND0]) {
						wdev->channel = GetDefaultChannel(pAd->ApCfg.MBSSID[0].wdev.PhyMode);
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error:ACS Disable, but the Channel of config parameter invalid,so default channel:\n");
					}

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BssIdx(%d) wdev->channel=%d\n", j, wdev->channel);
			}

#ifdef BW_VENDOR10_CUSTOM_FEATURE
			for (j = 0; j < DBDC_BAND_NUM; j++)
				SET_APCLI_SYNC_PEER_DEAUTH_ENBL(pAd, FALSE, i);
#endif
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Index%d Channel=%d\n", i, Channel);
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (i < MAX_MULTI_STA) {
				wdev = &pAd->StaCfg[i].wdev;
				wdev->channel = Channel;
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
#ifdef DBDC_MODE
	/* assign per-band bssid num */
#ifdef MULTI_PROFILE
	if (is_multi_profile_enable(pAd)) {
		pAd->ApCfg.BssidNumPerBand[DBDC_BAND0] = multi_profile_get_pf1_num(pAd);
		pAd->ApCfg.BssidNumPerBand[DBDC_BAND1] = multi_profile_get_pf2_num(pAd);
	} else
#endif /* MULTI_PROFILE */
	{
		UINT8 PrevBssNumAll =
			pAd->ApCfg.BssidNumPerBand[DBDC_BAND0] + pAd->ApCfg.BssidNumPerBand[DBDC_BAND1];

		/* check by wdev's channel */
		for (i = PrevBssNumAll; i < pAd->ApCfg.BssidNum; i++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			if (wdev->channel == pAd->ApCfg.MBSSID[0].wdev.channel)
				pAd->ApCfg.BssidNumPerBand[DBDC_BAND0]++;
			else
				pAd->ApCfg.BssidNumPerBand[DBDC_BAND1]++;
		}
	}
#else /* DBDC_MODE */
	pAd->ApCfg.BssidNumPerBand[DBDC_BAND0] = pAd->ApCfg.BssidNum;
#endif /* DBDC_MODE */

#endif/*MBSS_SUPPORT*/
#endif /*CONFIG_AP_SUPPORT*/

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (pAd->StaCfg[MAIN_MSTA_ID].wdev.channel == 0)
			pAd->StaCfg[MAIN_MSTA_ID].wdev.channel = GetDefaultChannel(pAd->StaCfg[MAIN_MSTA_ID].wdev.PhyMode);

		for (i = 0; i < MAX_MULTI_STA; i++) {
			wdev = &pAd->StaCfg[i].wdev;

			if (wdev->channel == 0)
				wdev->channel = pAd->StaCfg[MAIN_MSTA_ID].wdev.channel;
		}
	}
#endif /*CONFIG_STA_SUPPORT*/
}

#ifdef WHNAT_SUPPORT
static VOID RTMPWHNATCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{

#ifdef CTXD_SCATTER_AND_GATHER
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct pci_hif_chip *hif_chip = pci_hif->main_hif_chip;
#endif

	pAd->CommonCfg.whnat_en = os_str_tol(Buffer, 0, 10);
	hif_core_ops_register(pAd->hdev_ctrl, pAd->infType);
#ifdef CTXD_SCATTER_AND_GATHER
	if (pAd->CommonCfg.whnat_en)
		hif_chip->max_ctxd_agg_num = 1;
#endif
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WHNAT=%d\n", pAd->CommonCfg.whnat_en);
}
#endif /*WHNAT_SUPPORT*/

#if defined(MT7986)
static VOID RTMPDBDCDualPCIECfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	pAd->CommonCfg.DBDCDualPCIE = os_str_tol(Buffer, 0, 10);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DBDCDualPCIE = %d\n", pAd->CommonCfg.DBDCDualPCIE);
}
#endif /* MT7986 */

#ifdef MULTI_INTR_SUPPORT
static VOID RTMPMultiIntrCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	BOOLEAN pre_condition;
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);

	pAd->CommonCfg.MultiIntr = os_str_tol(Buffer, 0, 10);

	/* The multiple interrupt will be adopted only When WHNAT is off
	 * since the current HWNAT design requires be aware of all interrupt
	 * events through single irq
	 */
#ifdef WHNAT_SUPPORT
	pre_condition = pAd->CommonCfg.whnat_en == 0;
#else
	pre_condition = TRUE;
#endif
	if (!pre_condition)
		pAd->CommonCfg.MultiIntr = 0;

	/* update some hif setting configured during driver probe phase */
	if (chip_ops && chip_ops->post_config_hif)
		chip_ops->post_config_hif(pAd);

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MultiIntr = %d\n", pAd->CommonCfg.MultiIntr);
}
#endif

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
static VOID RTMPOldChannelCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UINT32 i;
	CHAR *macptr;
	struct wifi_dev *wdev;
	UCHAR BackupChannel;

	if ((Buffer == NULL) || (IS_SUPPORT_V10_DFS(pAd) == FALSE))
		return;

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
#ifdef CONFIG_AP_SUPPORT
		/* V10 -- AP Mode Only */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			/* V10 -- Only 5G Mode */
			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			if ((wdev == NULL) || (!WMODE_CAP_5G(wdev->PhyMode)))
				continue;

			/* Extract Channel */
			BackupChannel = os_str_tol(macptr, 0, 10);

			/* Disallow Zero or Invalid Values */
			if ((!BackupChannel) || (DfsV10CheckChnlGrp(pAd, wdev, BackupChannel) == NA_GRP)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Backup Channel=%d\n",
					BackupChannel);
				continue;
			}

			/* Valid Old Channel Processing */
			if (BackupChannel && (!IS_V10_OLD_CHNL_VALID(wdev)))
				SET_V10_OLD_CHNL_VALID(wdev, TRUE);

			/* Update Channel */
			wdev->channel = BackupChannel;
			pAd->CommonCfg.old_channel = wdev->channel;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS%d Channel=%d\n",
				i, wdev->channel);
		}
#endif /* CONFIG_AP_SUPPORT */
	}
}
#endif

static VOID RTMPWirelessModeCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UCHAR i;
	UCHAR cfg_mode, *macptr;
	struct wifi_dev *wdev = NULL;

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		cfg_mode = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
		if (i >= pAd->ApCfg.BssidNum)
			break;

		wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
		wdev->PhyMode = cfgmode_2_wmode(cfg_mode);
		wlan_config_set_ch_band(wdev, wdev->PhyMode);
#ifdef DOT11_HE_AX
		if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G))
			wlan_config_set_he6g_op_present(wdev, 1);
		else
			wlan_config_set_he6g_op_present(wdev, 0);
#endif /* #ifdef DOT11_HE_AX */
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Init: BSS%d PhyMode=%d\n", i, wdev->PhyMode);

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

		if (i < MAX_MULTI_STA) {
			wdev = &pAd->StaCfg[i].wdev;
			wdev->PhyMode = cfgmode_2_wmode(cfg_mode);
			wlan_config_set_ch_band(wdev, wdev->PhyMode);
		}

#endif /*CONFIG_STA_SUPPORT*/
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
		/* for first time, update all phy mode is same as ra0 */
		for (i = 0; i < MAX_APCLI_NUM; i++) {
			pAd->StaCfg[i].wdev.PhyMode = pAd->ApCfg.MBSSID[0].wdev.PhyMode;
			wlan_config_set_ch_band(&pAd->StaCfg[i].wdev,
						pAd->ApCfg.MBSSID[0].wdev.PhyMode);
#ifdef MULTI_PROFILE
			if (is_multi_profile_enable(pAd) && i != 0) {
				UCHAR idx = multi_profile_get_pf1_num(pAd);
				pAd->StaCfg[i].wdev.PhyMode = pAd->ApCfg.MBSSID[idx].wdev.PhyMode;
				wlan_config_set_ch_band(&pAd->StaCfg[i].wdev,
							pAd->ApCfg.MBSSID[idx].wdev.PhyMode);
			}
#endif /* MULTI_PROFILE */
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Final: Apcli%d, PhyMode=%d\n", i, pAd->StaCfg[i].wdev.PhyMode);
		}

#endif /*APCLI_SUPPORT*/

#ifdef MBSS_SUPPORT

	/*Check if any wdev not configure a wireless mode, apply MSSID value to it.*/
	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;

		if (wdev->PhyMode == WMODE_INVALID) {
			wdev->PhyMode = pAd->ApCfg.MBSSID[0].wdev.PhyMode;
			wlan_config_set_ch_band(wdev, wdev->PhyMode);
		}
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Final: BSS%d PhyMode=%d\n", i, wdev->PhyMode);
	}

#endif/*MBSS_SUPPORT*/
#endif /*CONFIG_AP_SUPPORT*/

}

/*
   ==========================================================================
   Description:
       Set EDCCA mode by reading profile settings.
       0:disable, 1: set ED threshold to -49 dB for 2.4G or 5G

    Return:
	None.
   ==========================================================================
*/
VOID edcca_mode_set_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *buffer)
{
	UINT8 band_idx = 0;
	RTMP_STRING *ptr;

	for (band_idx = 0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), band_idx++) {
		if (band_idx < DBDC_BAND_NUM) {
#ifdef DEFAULT_5G_PROFILE
			if (band_idx == 0 && (pAd->CommonCfg.dbdc_mode == 1)) {
				pAd->CommonCfg.u1EDCCAMode[BAND1] = simple_strtol(ptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, " edcca_mode band mode[1]= %d\n",
					pAd->CommonCfg.u1EDCCAMode[BAND1]);
			} else {
				pAd->CommonCfg.u1EDCCAMode[BAND0] = simple_strtol(ptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, " edcca_mode band mode [0]= %d\n",
					pAd->CommonCfg.u1EDCCAMode[BAND0]);
			}

#else
			pAd->CommonCfg.u1EDCCAMode[band_idx] = simple_strtol(ptr, 0, 10);
#endif /* DEFAULT_5G_PROFILE */
		} else
			break;
	}
}
/*
   ==========================================================================
   Description:
       Set SCSEnable by reading profile settings.
       0:disable, 1: Enable

    Return:
	None.
   ==========================================================================
*/
VOID SCS_Enable_set_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *buffer)
{
	UINT8 band_idx = 0;
	RTMP_STRING *ptr;

	for (band_idx = 0, ptr = rstrtok(buffer, ";"); ptr; ptr = rstrtok(NULL, ";"), band_idx++) {
		if (band_idx < DBDC_BAND_NUM) {
			long value = 0;

			if (kstrtol(ptr, 10, &value))
				MTWF_PRINT("%s: read value failed(=%s)\n", __func__, buffer);
			else {
#ifdef DEFAULT_5G_PROFILE
				if (band_idx == 0 && (pAd->CommonCfg.dbdc_mode == 1)) {
					pAd->SCSCtrl.SCSEnable[BAND1] = (UINT_8)value;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"SCSEnable[1]= %d\n", pAd->SCSCtrl.SCSEnable[BAND1]);
				} else {
					pAd->SCSCtrl.SCSEnable[BAND0] = (UINT_8)value;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"SCSEnable [0]= %d\n", pAd->SCSCtrl.SCSEnable[BAND0]);
				}

#else
				pAd->SCSCtrl.SCSEnable[band_idx] = (UINT_8)value;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "SCSEnable [0]= %d\n",
					pAd->SCSCtrl.SCSEnable[band_idx]);

#endif /* DEFAULT_5G_PROFILE */
			}
		} else
			break;
	}
}

/*
    ========================================================================

    Routine Description:
	Find key section for Get key parameter.

    Arguments:
	buffer                      Pointer to the buffer to start find the key section
	section                     the key of the secion to be find

    Return Value:
	NULL                        Fail
	Others                      Success
    ========================================================================
*/
RTMP_STRING *RTMPFindSection(RTMP_STRING *buffer)
{
	RTMP_STRING temp_buf[32];
	RTMP_STRING *ptr, *ret = NULL;

	strncpy(temp_buf, "Default", strlen("Default") + 1);
	ptr = rtstrstr(buffer, temp_buf);

	if (ptr != NULL) {
		ret = ptr + strlen("\n");
		return ret;
	} else
		return NULL;
}

/*
    ========================================================================

    Routine Description:
	Get key parameter.

    Arguments:
	key			Pointer to key string
	dest			Pointer to destination
	destsize		The datasize of the destination
	buffer		Pointer to the buffer to start find the key
	bTrimSpace	Set true if you want to strip the space character of the result pattern

    Return Value:
	TRUE                        Success
	FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
	For SSID and security key related parameters, we SHALL NOT trim the space(' ') character.
    ========================================================================
*/
INT RTMPGetKeyParameter(
	IN RTMP_STRING *key,
	OUT RTMP_STRING *dest,
	IN INT destsize,
	IN RTMP_STRING *buffer,
	IN BOOLEAN bTrimSpace)
{
	RTMP_STRING *pMemBuf, *temp_buf1 = NULL, *temp_buf2 = NULL;
	RTMP_STRING *start_ptr, *end_ptr;
	RTMP_STRING *ptr;
	RTMP_STRING *offset = NULL;
	INT  len, keyLen;

	keyLen = strlen(key);
	os_alloc_mem(NULL, (PUCHAR *)&pMemBuf, MAX_PARAM_BUFFER_SIZE  * 2);

	if (pMemBuf == NULL)
		return FALSE;

	memset(pMemBuf, 0, MAX_PARAM_BUFFER_SIZE * 2);
	temp_buf1 = pMemBuf;
	temp_buf2 = (RTMP_STRING *)(pMemBuf + MAX_PARAM_BUFFER_SIZE);
	/*find section*/
	offset = RTMPFindSection(buffer);

	if (offset == NULL) {
		os_free_mem((PUCHAR)pMemBuf);
		return FALSE;
	}

	strncpy(temp_buf1, "\n", strlen("\n") + 1);
	strncat(temp_buf1, key, strlen(key));
	strncat(temp_buf1, "=", strlen("="));
	/*search key*/
	start_ptr = rtstrstr(offset, temp_buf1);

	if (start_ptr == NULL) {
		os_free_mem((PUCHAR)pMemBuf);
		return FALSE;
	}

	start_ptr += strlen("\n");
	end_ptr = rtstrstr(start_ptr, "\n");

	if (end_ptr == NULL)
		end_ptr = start_ptr + strlen(start_ptr);

	if (end_ptr < start_ptr) {
		os_free_mem((PUCHAR)pMemBuf);
		return FALSE;
	}

	NdisMoveMemory(temp_buf2, start_ptr, end_ptr - start_ptr);
	temp_buf2[end_ptr - start_ptr] = '\0';
	start_ptr = rtstrstr(temp_buf2, "=");

	if (start_ptr == NULL) {
		os_free_mem((PUCHAR)pMemBuf);
		return FALSE;
	}

	ptr = (start_ptr + 1);

	/*trim special characters, i.e.,  TAB or space*/
	while (*start_ptr != 0x00) {
		if (((*ptr == ' ') && bTrimSpace) || (*ptr == '\t'))
			ptr++;
		else
			break;
	}

	len = strlen(start_ptr);
	memset(dest, 0x00, destsize);
	strncpy(dest, ptr, ((len >= destsize) ? destsize : len));
	os_free_mem((PUCHAR)pMemBuf);
	return TRUE;
}

/*
    ========================================================================

    Routine Description:
	Add key parameter.

    Arguments:
	key			Pointer to key string
	value			Pointer to destination
	destsize		The datasize of the destination
	bTrimSpace	Set true if you want to strip the space character of the result pattern

    Return Value:
	TRUE                        Success
	FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
	For SSID and security key related parameters, we SHALL NOT trim the space(' ') character.
    ========================================================================
*/
INT RTMPAddKeyParameter(
	IN RTMP_STRING *key,
	IN CHAR *value,
	IN INT destsize,
	IN RTMP_STRING *buffer)
{
	ULONG str_tem_len = MAX_INI_BUFFER_SIZE - strlen(buffer);
	int ret = snprintf(buffer + strlen(buffer), str_tem_len,
		"%s=%s\n", key, value);
	if (os_snprintf_error(str_tem_len, ret)) {
		MTWF_PRINT("%s: snprintf error!\n", __func__);
		return FALSE;
	}
	return TRUE;
}

/*
    ========================================================================

    Routine Description:
	Set key parameter.

    Arguments:
	key			Pointer to key string
	value			Pointer to destination
	destsize		The datasize of the destination
	bTrimSpace	Set true if you want to strip the space character of the result pattern

    Return Value:
	TRUE                        Success
	FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
	For SSID and security key related parameters, we SHALL NOT trim the space(' ') character.
    ========================================================================
*/
INT RTMPSetKeyParameter(
	IN RTMP_STRING *key,
	OUT CHAR *value,
	IN INT destsize,
	IN RTMP_STRING *buffer,
	IN BOOLEAN bTrimSpace)
{
	RTMP_STRING buf[512] = "", *temp_buf1 = NULL;
	RTMP_STRING *start_ptr;
	RTMP_STRING *end_ptr;
	RTMP_STRING *offset = NULL;
	INT keyLen;
	INT start_len;
	INT end_len;
	INT len;

	keyLen = strlen(key);
	temp_buf1 = buf;

	/*find section*/
	offset = RTMPFindSection(buffer);
	if (offset == NULL)
		return FALSE;

	if (keyLen > (sizeof(buf) - 3)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Input parameter incorrect, keyLen = %d\n", keyLen);
		return FALSE;
	}

	strncpy(temp_buf1, "\n", strlen("\n") + 1);
	strncat(temp_buf1, key, strlen(key));
	strncat(temp_buf1, "=", strlen("="));

	/*search key*/
	start_ptr = rtstrstr(offset, temp_buf1);
	if (start_ptr == NULL) {
		/*can't searched, add directly*/
		RTMPAddKeyParameter(key, value, destsize, buffer);
		return TRUE;
	}

	/*remove original*/
	start_ptr += strlen("\n");
	start_len = strlen(start_ptr);

	end_ptr = rtstrstr(start_ptr, "\n");
	if (end_ptr == NULL)
		end_ptr = start_ptr + start_len;

	if (end_ptr < start_ptr)
		return FALSE;

	/*clear original setting*/
	end_ptr += strlen("\n");
	end_len = strlen(end_ptr);
	os_move_mem(start_ptr, end_ptr, end_len);
	start_ptr += end_len;
	len = start_len - end_len;
	os_zero_mem(start_ptr, len);
	/*fill new field & value*/
	RTMPAddKeyParameter(key, value, destsize, buffer);
	return TRUE;
}

/*
    ========================================================================

    Routine Description:
	Get multiple key parameter.

    Arguments:
	key                         Pointer to key string
	dest                        Pointer to destination
	destsize                    The datasize of the destination
	buffer                      Pointer to the buffer to start find the key

    Return Value:
	TRUE                        Success
	FALSE                       Fail

    Note:
	This routine get the value with the matched key (case case-sensitive)
    ========================================================================
*/
INT RTMPGetKeyParameterWithOffset(
	IN  RTMP_STRING *key,
	OUT RTMP_STRING *dest,
	OUT PUINT end_offset,
	IN  INT     destsize,
	IN  RTMP_STRING *buffer,
	IN	BOOLEAN	bTrimSpace)
{
	RTMP_STRING *temp_buf1 = NULL;
	RTMP_STRING *temp_buf2 = NULL;
	RTMP_STRING *start_ptr;
	RTMP_STRING *end_ptr;
	RTMP_STRING *ptr;
	RTMP_STRING *offset = 0;
	INT  len;

#ifndef DBDC_MODE
	if (*end_offset >= MAX_INI_BUFFER_SIZE)
		return FALSE;
#endif

	os_alloc_mem(NULL, (PUCHAR *)&temp_buf1, MAX_PARAM_BUFFER_SIZE);

	if (temp_buf1 == NULL)
		return FALSE;

	os_alloc_mem(NULL, (PUCHAR *)&temp_buf2, MAX_PARAM_BUFFER_SIZE);

	if (temp_buf2 == NULL) {
		os_free_mem((PUCHAR)temp_buf1);
		return FALSE;
	}

	/*find section		*/
	if (*end_offset == 0) {
		offset = RTMPFindSection(buffer);

		if (offset == NULL) {
			os_free_mem((PUCHAR)temp_buf1);
			os_free_mem((PUCHAR)temp_buf2);
			return FALSE;
		}
	} else
		offset = buffer + (*end_offset);

	strncpy(temp_buf1, "\n", strlen("\n") + 1);
	strncat(temp_buf1, key, strlen(key));
	strncat(temp_buf1, "=", strlen("="));
	/*search key*/
	start_ptr = rtstrstr(offset, temp_buf1);

	if (start_ptr == NULL) {
		os_free_mem((PUCHAR)temp_buf1);
		os_free_mem((PUCHAR)temp_buf2);
		return FALSE;
	}

	start_ptr += strlen("\n");
	end_ptr = rtstrstr(start_ptr, "\n");

	if (end_ptr == NULL)
		end_ptr = start_ptr + strlen(start_ptr);

	if (end_ptr < start_ptr) {
		os_free_mem((PUCHAR)temp_buf1);
		os_free_mem((PUCHAR)temp_buf2);
		return FALSE;
	}

	*end_offset = end_ptr - buffer;
	NdisMoveMemory(temp_buf2, start_ptr, end_ptr - start_ptr);
	temp_buf2[end_ptr - start_ptr] = '\0';
	strncpy(temp_buf1, temp_buf2, strlen(temp_buf2) + 1);
	start_ptr = rtstrstr(temp_buf1, "=");

	if (start_ptr == NULL) {
		os_free_mem((PUCHAR)temp_buf1);
		os_free_mem((PUCHAR)temp_buf2);
		return FALSE;
	}

	strncpy(temp_buf2, start_ptr + 1, strlen(start_ptr + 1) + 1);
	ptr = temp_buf2;

	/*trim space or tab*/
	while (*ptr != 0x00) {
		if ((bTrimSpace && (*ptr == ' ')) || (*ptr == '\t'))
			ptr++;
		else
			break;
	}

	len = strlen(ptr);
	memset(dest, 0x00, destsize);
	strncpy(dest, ptr, len >= destsize ?  destsize : len);
	os_free_mem((PUCHAR)temp_buf1);
	os_free_mem((PUCHAR)temp_buf2);
	return TRUE;
}


#ifdef CONFIG_AP_SUPPORT

#ifdef APCLI_SUPPORT
static void rtmp_read_ap_client_from_file(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *buffer)
{
	RTMP_STRING *macptr = NULL;
	INT			i = 0, j = 0;
	UCHAR		macAddress[MAC_ADDR_LEN];
	PSTA_ADMIN_CONFIG   pApCliEntry = NULL;
	struct wifi_dev *wdev = NULL;
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
	UINT_8 idx = 0;
	u8 max_bss = 0;
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */


	for (i = 0; i < MAX_APCLI_NUM; i++) {

#ifdef DOT11W_PMF_SUPPORT
		pAd->StaCfg[i].wdev.SecConfig.PmfCfg.Desired_MFPC = FALSE;
		pAd->StaCfg[i].wdev.SecConfig.PmfCfg.Desired_MFPR = FALSE;
		pAd->StaCfg[i].wdev.SecConfig.PmfCfg.Desired_PMFSHA256 = FALSE;
#endif /* DOT11W_PMF_SUPPORT */

		pAd->StaCfg[i].BssType = BSS_INFRA;
	}



#ifdef CONVERTER_MODE_SWITCH_SUPPORT
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"Initially Start any BSS or AP with Default settings\n");
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
			pAd->ApCfg.MBSSID[idx].APStartPseduState = AP_STATE_ALWAYS_START_AP_DEFAULT;
		for (idx = 0; idx < MAX_APCLI_NUM; idx++)
			pAd->StaCfg[idx].ApCliMode = APCLI_MODE_ALWAYS_START_AP_DEFAULT;
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */

	/*ApCliEnable*/
	if (RTMPGetKeyParameter("ApCliEnable", tmpbuf, 128, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			pApCliEntry = &pAd->StaCfg[i];

			if ((strncmp(macptr, "0", 1) == 0))
				pApCliEntry->ApcliInfStat.Enable = FALSE;
			else if ((strncmp(macptr, "1", 1) == 0))
				pApCliEntry->ApcliInfStat.Enable = TRUE;
			else {
				pApCliEntry->ApcliInfStat.Enable = FALSE;

#ifdef CONVERTER_MODE_SWITCH_SUPPORT
				if (strncmp(macptr, "2", 1) == 0) {
						pApCliEntry->ApcliInfStat.Enable = TRUE;
						pApCliEntry->ApCliMode = APCLI_MODE_START_AP_AFTER_APCLI_CONNECTION;
						MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"Start any BSS or AP, after ApcliConnection with RootAP\n");
				} else if (strncmp(macptr, "3", 1) == 0) {
						pApCliEntry->ApcliInfStat.Enable = TRUE;
						pApCliEntry->ApCliMode = APCLI_MODE_NEVER_START_AP;
						MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"Do Not start any BSS or AP\n");
				}

				if (i == 0) {
					idx = 0;
					max_bss = multi_profile_get_pf1_num(pAd);
				} else {
					idx = multi_profile_get_pf1_num(pAd);
					max_bss = pAd->ApCfg.BssidNum;
				}
				if (pAd->CommonCfg.dbdc_mode == 0)
					max_bss = pAd->ApCfg.BssidNum;
				for (; idx < max_bss; idx++) {
					/* When ApCliEnable=2: Start AP, when Apcli connect to RootAP */
					/* When ApCliEnable=3: Never start any(BSS) AP */
					if (strncmp(macptr, "2", 1) == 0) {
						pAd->ApCfg.MBSSID[idx].APStartPseduState =
							AP_STATE_START_AFTER_APCLI_CONNECTION;
					} else if (strncmp(macptr, "3", 1) == 0) {
							pAd->ApCfg.MBSSID[idx].APStartPseduState = AP_STATE_NEVER_START_AP;
					}
				}
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */

			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ApCliEntry[%d].Enable=%d\n", i, pApCliEntry->ApcliInfStat.Enable);
		}
	}

	/*ApCliSsid*/
	if (RTMPGetKeyParameter("ApCliSsid", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, FALSE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pApCliEntry = &pAd->StaCfg[i];
			/*Ssid acceptable strlen must be less than 32 and bigger than 0.*/
			pApCliEntry->CfgSsidLen = (UCHAR)strlen(macptr);

			if (pApCliEntry->CfgSsidLen > 32) {
				pApCliEntry->CfgSsidLen = 0;
				continue;
			}

			if (pApCliEntry->CfgSsidLen > 0) {
				memcpy(&pApCliEntry->CfgSsid, macptr, pApCliEntry->CfgSsidLen);
				pApCliEntry->ApcliInfStat.Valid = FALSE;/* it should be set when successfuley association*/
			} else {
				NdisZeroMemory(&(pApCliEntry->CfgSsid), MAX_LEN_OF_SSID);
				continue;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ApCliEntry[%d].CfgSsidLen=%d, CfgSsid=%s\n", i,
					 pApCliEntry->CfgSsidLen, pApCliEntry->CfgSsid);
		}
	}

#ifdef DBDC_MODE

	/*ApCliWirelessMode*/
	if (RTMPGetKeyParameter("ApCliWirelessMode", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			UCHAR cfg_mode;

			cfg_mode = os_str_tol(macptr, 0, 10);
			pApCliEntry = &pAd->StaCfg[i];
			pApCliEntry->wdev.PhyMode = cfgmode_2_wmode(cfg_mode);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ApCliEntry[%d].wdev.PhyMode=%d\n", i,
					 pApCliEntry->wdev.PhyMode);
		}
	}

#ifdef MULTI_PROFILE
	/* sanity check apcli wireless mode setting */
	{
		{
			USHORT max_5G_6G_PhyMode = 0, max_2G_PhyMode = 0;

			INT default_5g_6g_rule = 0;

			if (!WMODE_2G_ONLY(pAd->ApCfg.MBSSID[0].wdev.PhyMode))
				default_5g_6g_rule = 1;

			for (i = 0; i < MAX_APCLI_NUM; i++) {
				UCHAR mbss_idx;
				UCHAR apcli_phy_mode_correct = 0;

				pApCliEntry = &pAd->StaCfg[i];

				/* check if apcli phy mode setting is in one of all mbss phy mode */
				for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
					BSS_STRUCT *pMbss = NULL;
					struct wifi_dev	 *mbss_wdev = NULL;

					pMbss = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, mbss_idx)];
					mbss_wdev = &pMbss->wdev;

					if (!WMODE_2G_ONLY(mbss_wdev->PhyMode))
						max_5G_6G_PhyMode = (max_5G_6G_PhyMode < mbss_wdev->PhyMode) ?
										 mbss_wdev->PhyMode : max_5G_6G_PhyMode;
					else
						max_2G_PhyMode = (max_2G_PhyMode < mbss_wdev->PhyMode) ?
										 mbss_wdev->PhyMode : max_2G_PhyMode;

					if (default_5g_6g_rule == 1) {
						if ((i == 0) &&
							!WMODE_2G_ONLY(mbss_wdev->PhyMode) &&
							(pApCliEntry->wdev.PhyMode ==
							 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, mbss_idx)].wdev.PhyMode))
							apcli_phy_mode_correct = 1;
						else if ((i == 1) &&
								 WMODE_2G_ONLY(mbss_wdev->PhyMode) &&
								 (pApCliEntry->wdev.PhyMode ==
								  pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, mbss_idx)].wdev.PhyMode))
							apcli_phy_mode_correct = 1;
					} else {
						if ((i == 0) &&
							WMODE_2G_ONLY(mbss_wdev->PhyMode) &&
							(pApCliEntry->wdev.PhyMode ==
							 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, mbss_idx)].wdev.PhyMode))
							apcli_phy_mode_correct = 1;
						else if ((i == 1) &&
								 !WMODE_2G_ONLY(mbss_wdev->PhyMode) &&
								 (pApCliEntry->wdev.PhyMode ==
								  pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, mbss_idx)].wdev.PhyMode))
							apcli_phy_mode_correct = 1;
					}
				}

				if (apcli_phy_mode_correct != 1) {
					if (default_5g_6g_rule == 1) {
						if (i == 0)
							pApCliEntry->wdev.PhyMode = max_5G_6G_PhyMode;
						else if (i == 1)
							pApCliEntry->wdev.PhyMode = max_2G_PhyMode;
					} else {
						if (i == 0)
							pApCliEntry->wdev.PhyMode = max_2G_PhyMode;
						else if (i == 1)
							pApCliEntry->wdev.PhyMode = max_5G_6G_PhyMode;
					}
				}

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "Sanity check in DBDC :ApCliEntry[%d].wdev.PhyMode=%d\n",
						  i, pApCliEntry->wdev.PhyMode);
			}
		}
	}
#endif /*MULTI_PROFILE*/
#endif

	/*ApCliBssid*/
	if (RTMPGetKeyParameter("ApCliBssid", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pApCliEntry = &pAd->StaCfg[i];

			if (strlen(macptr) != 17) /*Mac address acceptable format 01:02:03:04:05:06 length 17*/
				continue;

			if (strcmp(macptr, "00:00:00:00:00:00") == 0)
				continue;

			for (j = 0; j < MAC_ADDR_LEN; j++) {
				AtoH(macptr, &macAddress[j], 1);
				macptr = macptr + 3;
			}

			memcpy(pApCliEntry->CfgApCliBssid, &macAddress, MAC_ADDR_LEN);
			pApCliEntry->ApcliInfStat.Valid = FALSE;/* it should be set when successfuley association*/
		}
	}

	/* ApCliTxMode*/
	if (RTMPGetKeyParameter("ApCliTxMode", tmpbuf, 25, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			wdev = &pAd->StaCfg[i].wdev;
			wdev->DesiredTransmitSetting.field.FixedTxMode =
				RT_CfgSetFixedTxPhyMode(macptr);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) Tx Mode = %d\n", i,
					 wdev->DesiredTransmitSetting.field.FixedTxMode);
		}
	}

	/* ApCliTxMcs*/
	if (RTMPGetKeyParameter("ApCliTxMcs", tmpbuf, 50, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			wdev = &pAd->StaCfg[i].wdev;
			wdev->DesiredTransmitSetting.field.MCS =
				RT_CfgSetTxMCSProc(macptr, &wdev->bAutoTxRateSwitch);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) Tx MCS = %s(%d)\n", i,
					 (wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO ? "AUTO" : ""),
					 wdev->DesiredTransmitSetting.field.MCS);
		}
	}

#ifdef WSC_AP_SUPPORT

	/* Wsc4digitPinCode = TRUE use 4-digit Pin code, otherwise 8-digit Pin code */
	if (RTMPGetKeyParameter("ApCli_Wsc4digitPinCode", tmpbuf, 32, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			if (os_str_tol(macptr, 0, 10) != 0)
				pAd->StaCfg[i].wdev.WscControl.WscEnrollee4digitPinCode = TRUE;
			else /* Disable */
				pAd->StaCfg[i].wdev.WscControl.WscEnrollee4digitPinCode = FALSE;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) ApCli_Wsc4digitPinCode=%d\n", i,
					 pAd->StaCfg[i].wdev.WscControl.WscEnrollee4digitPinCode);
		}
	}

#ifdef APCLI_SUPPORT
	/* ApCliWscScanMode */
	if (RTMPGetKeyParameter("ApCliWscScanMode", tmpbuf, 32, buffer, TRUE)) {
		UCHAR Mode;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			Mode = simple_strtol(macptr, 0, 10);
			if (Mode != TRIGGER_PARTIAL_SCAN)
				Mode = TRIGGER_FULL_SCAN;

			pAd->StaCfg[i].wdev.WscControl.WscApCliScanMode = Mode;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"I/F(apcli%d) WscApCliScanMode=%d\n", i, Mode);
		}
	}
#endif /* APCLI_SUPPORT */

#endif /* WSC_AP_SUPPORT */
#ifdef UAPSD_SUPPORT

	/*UAPSDCapable*/
	if (RTMPGetKeyParameter("ApCliUAPSDCapable", tmpbuf, 10, buffer, TRUE) ||
		RTMPGetKeyParameter("ApCliAPSDCapable", tmpbuf, 10, buffer, TRUE)) { /* backward compatible with old SDK */
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;
		pAd->ApCfg.FlgApCliIsUapsdInfoUpdated = TRUE;

		for (i = 0, macptr = rstrtok(tmpbuf, ";");
			 (macptr && i < MAX_APCLI_NUM);
			 macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pApCliEntry = &pAd->StaCfg[i];
			pApCliEntry->wdev.UapsdInfo.bAPSDCapable = \
					(UCHAR) os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ApCliUAPSDCapable[%d]=%d\n", i,
					 pApCliEntry->wdev.UapsdInfo.bAPSDCapable);
		}
	}

#endif /* UAPSD_SUPPORT */

	/* ApCliNum */
	if (RTMPGetKeyParameter("ApCliNum", tmpbuf, 10, buffer, TRUE)) {
		if (os_str_tol(tmpbuf, 0, 10) <= MAX_APCLI_NUM)
			pAd->ApCfg.ApCliNum = os_str_tol(tmpbuf, 0, 10);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli) ApCliNum=%d\n", pAd->ApCfg.ApCliNum);
	}

#if defined(DBDC_MODE)
	if (pAd->CommonCfg.dbdc_mode == TRUE)
		pAd->ApCfg.ApCliNum = 2;
	else
#endif
		pAd->ApCfg.ApCliNum = 1;

#ifdef APCLI_CONNECTION_TRIAL
	pAd->ApCfg.ApCliNum++;

	/* ApCliTrialCh */
	if (RTMPGetKeyParameter("ApCliTrialCh", tmpbuf, 128, buffer, TRUE)) {
		/* last IF is for apcli connection trial */
		pApCliEntry = &pAd->StaCfg[pAd->ApCfg.ApCliNum - 1];
		pApCliEntry->TrialCh = (UCHAR) os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TrialChannel=%d\n", pApCliEntry->TrialCh);
	}

#endif /* APCLI_CONNECTION_TRIAL */
#ifdef DOT11W_PMF_SUPPORT

	/* Protection Management Frame Capable */
	if (RTMPGetKeyParameter("ApCliPMFMFPC", tmpbuf, 32, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0,
			 macptr = rstrtok(tmpbuf, ";");
			 (macptr && i < MAX_APCLI_NUM);
			 macptr = rstrtok(NULL, ";"),
			 i++) {
			POS_COOKIE pObj;
			INT backup_ioctl_if;
			INT backup_ioctl_if_type;

			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pObj = (POS_COOKIE) pAd->OS_Cookie;
			backup_ioctl_if = pObj->ioctl_if;
			backup_ioctl_if_type = pObj->ioctl_if_type;
			pObj->ioctl_if = i;
			pObj->ioctl_if_type = INT_APCLI;
			Set_ApCliPMFMFPC_Proc(pAd, macptr);
			pObj->ioctl_if = backup_ioctl_if;
			pObj->ioctl_if_type = backup_ioctl_if_type;
		}
	}

	/* Protection Management Frame Required */
	if (RTMPGetKeyParameter("ApCliPMFMFPR", tmpbuf, 32, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0,
			 macptr = rstrtok(tmpbuf, ";");
			 (macptr && i < MAX_APCLI_NUM);
			 macptr = rstrtok(NULL, ";"),
			 i++) {
			POS_COOKIE pObj;
			INT backup_ioctl_if;
			INT backup_ioctl_if_type;

			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pObj = (POS_COOKIE) pAd->OS_Cookie;
			backup_ioctl_if = pObj->ioctl_if;
			backup_ioctl_if_type = pObj->ioctl_if_type;
			pObj->ioctl_if = i;
			pObj->ioctl_if_type = INT_APCLI;
			Set_ApCliPMFMFPR_Proc(pAd, macptr);
			pObj->ioctl_if = backup_ioctl_if;
			pObj->ioctl_if_type = backup_ioctl_if_type;
		}
	}

	if (RTMPGetKeyParameter("ApCliPMFSHA256", tmpbuf, 32, buffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;

		for (i = 0,
			 macptr = rstrtok(tmpbuf, ";");
			 (macptr && i < MAX_APCLI_NUM);
			 macptr = rstrtok(NULL, ";"),
			 i++) {
			POS_COOKIE pObj;
			INT backup_ioctl_if;
			INT backup_ioctl_if_type;

			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;

			pObj = (POS_COOKIE) pAd->OS_Cookie;
			backup_ioctl_if = pObj->ioctl_if;
			backup_ioctl_if_type = pObj->ioctl_if_type;
			pObj->ioctl_if = i;
			pObj->ioctl_if_type = INT_APCLI;
			Set_ApCliPMFSHA256_Proc(pAd, macptr);
			pObj->ioctl_if = backup_ioctl_if;
			pObj->ioctl_if_type = backup_ioctl_if_type;
		}
	}

#endif /* DOT11W_PMF_SUPPORT */

#ifdef DOT11_HE_AX
	/* ApCliMuOfdmaDlEnable */
	if (RTMPGetKeyParameter("ApCliMuOfdmaDlEnable", tmpbuf, 128, buffer, FALSE)) {
		UCHAR mu_dl_ofdma = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			mu_dl_ofdma = os_str_tol(macptr, 0, 10);

			if (IF_COMBO_HAVE_AP_STA(pAd)) {
				wdev = &pAd->StaCfg[i].wdev;
			}

			if (wdev) {
				wlan_config_set_mu_dl_ofdma(wdev, mu_dl_ofdma);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"APCLI[%d] ApCliMuOfdmaDlEnable = %d\n", i, mu_dl_ofdma);
			}
		}
	}

	/* ApCliMuOfdmaUlEnable */
	if (RTMPGetKeyParameter("ApCliMuOfdmaUlEnable", tmpbuf, 128, buffer, FALSE)) {
		UCHAR mu_ul_ofdma = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			mu_ul_ofdma = os_str_tol(macptr, 0, 10);

			if (IF_COMBO_HAVE_AP_STA(pAd)) {
				wdev = &pAd->StaCfg[i].wdev;
			}

			if (wdev) {
				wlan_config_set_mu_ul_ofdma(wdev, mu_ul_ofdma);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"APCLI[%d] ApCliMuOfdmaUlEnable = %d\n", i, mu_ul_ofdma);
			}
		}
	}

	/* ApCliMuMimoDlEnable */
	if (RTMPGetKeyParameter("ApCliMuMimoDlEnable", tmpbuf, 128, buffer, FALSE)) {
		UCHAR mu_dl_mimo = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			mu_dl_mimo = os_str_tol(macptr, 0, 10);

			if (IF_COMBO_HAVE_AP_STA(pAd)) {
				wdev = &pAd->StaCfg[i].wdev;
			}

			if (wdev) {
				wlan_config_set_mu_dl_mimo(wdev, mu_dl_mimo);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"APCLI[%d] ApCliMuMimoDlEnable = %d\n", i, mu_dl_mimo);
			}
		}
	}

	/* ApCliMuMimoUlEnable */
	if (RTMPGetKeyParameter("ApCliMuMimoUlEnable", tmpbuf, 128, buffer, FALSE)) {
		UCHAR mu_ul_mimo = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			mu_ul_mimo = os_str_tol(macptr, 0, 10);

			if (IF_COMBO_HAVE_AP_STA(pAd)) {
				wdev = &pAd->StaCfg[i].wdev;
			}

			if (wdev) {
				wlan_config_set_mu_ul_mimo(wdev, mu_ul_mimo);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"APCLI[%d] ApCliMuMimoUlEnable = %d\n", i, mu_ul_mimo);
			}
		}
	}
#endif /* #ifdef DOT11_HE_AX */
}
#endif /* APCLI_SUPPORT */


static void rtmp_read_acl_parms_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING tok_str[32], *macptr;
	INT			i = 0, j = 0, idx;
	UCHAR		macAddress[MAC_ADDR_LEN];
	BOOLEAN		isDuplicate = FALSE;
	int ret;
	memset(macAddress, 0, MAC_ADDR_LEN);

	for (idx = 0; idx < pAd->ApCfg.BssidNum/*MAX_MBSSID_NUM(pAd)*/; idx++) {
		memset(&pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].AccessControlList, 0, sizeof(RT_802_11_ACL));
		/* AccessPolicyX*/
		ret = snprintf(tok_str, sizeof(tok_str), "AccessPolicy%d", idx);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: snprintf error!\n", __func__);
			return;
		}
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 10, buffer, TRUE)) {
			switch (os_str_tol(tmpbuf, 0, 10)) {
			case 1: /* Allow All, and the AccessControlList is positive now.*/
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].AccessControlList.Policy = 1;
				break;

			case 2: /* Reject All, and the AccessControlList is negative now.*/
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].AccessControlList.Policy = 2;
				break;

			case 0: /* Disable, don't care the AccessControlList.*/
			default:
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].AccessControlList.Policy = 0;
				break;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s=%ld\n", tok_str,
					 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].AccessControlList.Policy);
		}

		/* AccessControlListX*/
		ret = snprintf(tok_str, sizeof(tok_str), "AccessControlList%d", idx);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: snprintf error!\n", __func__);
			return;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				if (strlen(macptr) != 17)  /* Mac address acceptable format 01:02:03:04:05:06 length 17*/
					continue;

				ASSERT(pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].AccessControlList.Num <= MAX_NUM_OF_ACL_LIST);

				for (j = 0; j < MAC_ADDR_LEN; j++) {
					AtoH(macptr, &macAddress[j], 1);
					macptr = macptr + 3;
				}

				if (pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].AccessControlList.Num == MAX_NUM_OF_ACL_LIST) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							 "The AccessControlList is full, and no more entry can join the list!\n");
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "The last entry of ACL is "MACSTR"\n", MAC2STR(macAddress));
					break;
				}
				isDuplicate = FALSE;
				for (j = 0; j < pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].AccessControlList.Num; j++) {
					if (memcmp(pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].AccessControlList.Entry[j].Addr, &macAddress, 6) == 0) {
						isDuplicate = TRUE;
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							"You have added an entry before :\n");
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							"The duplicate entry is "MACSTR"\n", MAC2STR(macAddress));
					}
				}
				if (!isDuplicate) {
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].AccessControlList.Num++;
				NdisMoveMemory(pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].AccessControlList.Entry[(pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].AccessControlList.Num - 1)].Addr,
							   macAddress, MAC_ADDR_LEN);
				}
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s=Get %ld Mac Address\n", tok_str,
					 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].AccessControlList.Num);
		}
	}
}

/*
    ========================================================================

    Routine Description:
	In kernel mode read parameters from file

    Arguments:
	src                     the location of the file.
	dest                        put the parameters to the destination.
	Length                  size to read.

    Return Value:
	None

    Note:

    ========================================================================
*/

static void rtmp_read_ap_edca_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr, *edcaptr, tok_str[16];
	INT	i = 0, j = 0;
	EDCA_PARM *pEdca;
	RTMP_STRING *ptmpStr[6];
	struct wifi_dev *wdev = NULL;
	UCHAR ack_policy[WMM_NUM_OF_AC] = {0};
	int ret;

	for (j = 0; j < WMM_NUM; j++) {
		ret = snprintf(tok_str, sizeof(tok_str), "APEdca%d", j);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: snprintf error!\n", __func__);
			return;
		}
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s\n", tok_str);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE)) {
			pEdca = &pAd->CommonCfg.APEdcaParm[j];

			for (i = 0, edcaptr = rstrtok(tmpbuf, ";"); edcaptr; edcaptr = rstrtok(NULL, ";"), i++)
				ptmpStr[i] = edcaptr;

			if (i != 6) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Input parameter incorrect\n");
				return;
			}

			/*APValid*/
			edcaptr = ptmpStr[0];

			if (edcaptr) {
				pEdca->bValid = (UCHAR) os_str_tol(edcaptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Valid=%d\n", pEdca->bValid);
			}

			/*APAifsn*/
			edcaptr = ptmpStr[1];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					pEdca->Aifsn[i] = (UCHAR) os_str_tol(macptr, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APAifsn[%d]=%d\n", i, pEdca->Aifsn[i]);
				}
			}

			/*APCwmin*/
			edcaptr = ptmpStr[2];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					pEdca->Cwmin[i] = (UCHAR) os_str_tol(macptr, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APCwmin[%d]=%d\n", i, pEdca->Cwmin[i]);
				}
			}

			/*APCwmax*/
			edcaptr = ptmpStr[3];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					pEdca->Cwmax[i] = (UCHAR) os_str_tol(macptr, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APCwmax[%d]=%d\n", i, pEdca->Cwmax[i]);
				}
			}

			/*APTxop*/
			edcaptr = ptmpStr[4];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					pEdca->Txop[i] = (USHORT) os_str_tol(macptr, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APTxop[%d]=%d\n", i, pEdca->Txop[i]);
				}
			}

			/*APACM*/
			edcaptr = ptmpStr[5];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					pEdca->bACM[i] = (BOOLEAN) os_str_tol(macptr, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APACM[%d]=%d\n", i, pEdca->bACM[i]);
				}
			}
		}
	}

	/*AckPolicy*/
	for (i = 0 ; i < pAd->ApCfg.BssidNum; i++) {
		ret = snprintf(tok_str, sizeof(tok_str), "APAckPolicy%d", i);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: snprintf error!\n", __func__);
			return;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE)) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;

			for (j = 0, edcaptr = rstrtok(tmpbuf, ";"); edcaptr; edcaptr = rstrtok(NULL, ";"), j++)
				ack_policy[j] = (USHORT) simple_strtol(edcaptr, 0, 10);

			wlan_config_set_ack_policy(wdev, ack_policy);
		}
	}
}

static void rtmp_read_bss_edca_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr, *edcaptr, tok_str[16];
	UINT i = 0, j = 0;
	RTMP_STRING *ptmpStr[6];
	struct _EDCA_PARM *pBssEdca = NULL;
	int ret;

	for (j = 0; j < pAd->ApCfg.BssidNum; j++) {
		ret = snprintf(tok_str, sizeof(tok_str), "BSSEdca%d", j);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: snprintf error!\n", __func__);
			return;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE)) {
			pBssEdca = wlan_config_get_ht_edca(&pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, j)].wdev);

			if (!pBssEdca) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "BSS[%d]: Invalid pBssEdca\n", j);
				return;
			}

			for (i = 0, edcaptr = rstrtok(tmpbuf, ";"); edcaptr; edcaptr = rstrtok(NULL, ";"), i++) {
				if (i >= (sizeof(ptmpStr) / sizeof(ptmpStr[0]))) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "Input parameter[Edca] incorrect\n"
						 );
					return;
				}
				ptmpStr[i] = edcaptr;
			}
			if (i != 5) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Input parameter incorrect\n");
				return;
			}

			/*BSSAifsn*/
			edcaptr = ptmpStr[0];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					if (i >= WMM_NUM_OF_AC) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Input parameter[Aifsn] incorrect\n"
							 );
						return;
					}

					pBssEdca->Aifsn[i] = (UCHAR) simple_strtol(macptr, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSSAifsn[%d]=%d\n", i, pBssEdca->Aifsn[i]);
				}
			}

			/*BSSCwmin*/
			edcaptr = ptmpStr[1];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					if (i >= WMM_NUM_OF_AC) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Input parameter[Cwmin] incorrect\n"
							 );
						return;
					}
					pBssEdca->Cwmin[i] = (UCHAR) simple_strtol(macptr, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSSCwmin[%d]=%d\n", i, pBssEdca->Cwmin[i]);
				}
			}

			/*BSSCwmax*/
			edcaptr = ptmpStr[2];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					if (i >= WMM_NUM_OF_AC) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Input parameter[Cwmax] incorrect\n"
							 );
						return;
					}
					pBssEdca->Cwmax[i] = (UCHAR) simple_strtol(macptr, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSSCwmax[%d]=%d\n", i, pBssEdca->Cwmax[i]);
				}
			}

			/*BSSTxop*/
			edcaptr = ptmpStr[3];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					if (i >= WMM_NUM_OF_AC) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Input parameter[Txop] incorrect\n"
							 );
						return;
					}
					pBssEdca->Txop[i] = (USHORT) simple_strtol(macptr, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSSTxop[%d]=%d\n", i, pBssEdca->Txop[i]);
				}
			}

			/*BSSACM*/
			edcaptr = ptmpStr[4];

			if (edcaptr) {
				for (i = 0, macptr = rstrtok(edcaptr, ","); macptr; macptr = rstrtok(NULL, ","), i++) {
					if (i >= WMM_NUM_OF_AC) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Input parameter[ACM] incorrect\n"
							 );
						return;
					}
					pBssEdca->bACM[i] = (BOOLEAN) simple_strtol(macptr, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSSACM[%d]=%d\n", i, pBssEdca->bACM[i]);
				}
			}
		}
	}
}

static void rtmp_read_ap_wmm_parms_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr;
	INT	i = 0, j = 0;
	struct _EDCA_PARM *pBssEdca = NULL;

	/*WmmCapable*/
	if (RTMPGetKeyParameter("WmmCapable", tmpbuf, 32, buffer, TRUE)) {
		BOOLEAN bEnableWmm = FALSE;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			if (os_str_tol(macptr, 0, 10) != 0) {
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.bWmmCapable = TRUE;
				bEnableWmm = TRUE;
			} else
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.bWmmCapable = FALSE;

			if (bEnableWmm) {
				pAd->CommonCfg.APEdcaParm[0].bValid = TRUE;

				/* Apply BSS[0] setting to all as default */
				if (i == 0)
					wlan_config_set_edca_valid_all(&pAd->wpf, TRUE);
				else
					wlan_config_set_edca_valid(&pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev, TRUE);
			} else {
				pAd->CommonCfg.APEdcaParm[0].bValid = FALSE;

				/* Apply BSS[0] setting to all as default */
				if (i == 0)
					wlan_config_set_edca_valid_all(&pAd->wpf, FALSE);
				else
					wlan_config_set_edca_valid(&pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev, FALSE);
			}

			pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].bWmmCapableOrg = \
												  pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.bWmmCapable;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(ra%d) WmmCapable=%d\n", i,
					 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.bWmmCapable);
		}
	}

	/*New WMM Parameter*/
	rtmp_read_ap_edca_from_file(pAd, tmpbuf, buffer);

	/*DLSCapable*/
	if (RTMPGetKeyParameter("DLSCapable", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			if (os_str_tol(macptr, 0, 10) != 0) /*Enable*/
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].bDLSCapable = TRUE;
			else /*Disable*/
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].bDLSCapable = FALSE;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(ra%d) DLSCapable=%d\n", i,
					 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].bDLSCapable);
		}
	}

	/*APAifsn*/
	if (RTMPGetKeyParameter("APAifsn", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.APEdcaParm[0].Aifsn[i] = (UCHAR) os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APAifsn[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Aifsn[i]);
		}
	}

	/*APCwmin*/
	if (RTMPGetKeyParameter("APCwmin", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.APEdcaParm[0].Cwmin[i] = (UCHAR) os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APCwmin[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Cwmin[i]);
		}
	}

	/*APCwmax*/
	if (RTMPGetKeyParameter("APCwmax", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.APEdcaParm[0].Cwmax[i] = (UCHAR) os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APCwmax[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Cwmax[i]);
		}
	}

	/*APTxop*/
	if (RTMPGetKeyParameter("APTxop", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.APEdcaParm[0].Txop[i] = (USHORT) os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APTxop[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].Txop[i]);
		}
	}

	/*APACM*/
	if (RTMPGetKeyParameter("APACM", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.APEdcaParm[0].bACM[i] = (BOOLEAN) os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APACM[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[0].bACM[i]);
		}
	}

	/* Apply default (BSS) WMM Parameter */
	for (j = 0; j < pAd->ApCfg.BssidNum; j++) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS[%d]:\n", j);
		pBssEdca = wlan_config_get_ht_edca(&pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, j)].wdev);

		if (!pBssEdca)
			continue;

		/*BSSAifsn*/
		if (RTMPGetKeyParameter("BSSAifsn", tmpbuf, 32, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				pBssEdca->Aifsn[i] = (UCHAR) simple_strtol(macptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSSAifsn[%d]=%d\n", i, pBssEdca->Aifsn[i]);
			}
		}

		/*BSSCwmin*/
		if (RTMPGetKeyParameter("BSSCwmin", tmpbuf, 32, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				pBssEdca->Cwmin[i] = (UCHAR) simple_strtol(macptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSSCwmin[%d]=%d\n", i, pBssEdca->Cwmin[i]);
			}
		}

		/*BSSCwmax*/
		if (RTMPGetKeyParameter("BSSCwmax", tmpbuf, 32, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				pBssEdca->Cwmax[i] = (UCHAR) simple_strtol(macptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSSCwmax[%d]=%d\n", i, pBssEdca->Cwmax[i]);
			}
		}

		/*BSSTxop*/
		if (RTMPGetKeyParameter("BSSTxop", tmpbuf, 32, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				pBssEdca->Txop[i] = (USHORT) simple_strtol(macptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSSTxop[%d]=%d\n", i, pBssEdca->Txop[i]);
			}
		}

		/*BSSACM*/
		if (RTMPGetKeyParameter("BSSACM", tmpbuf, 32, buffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				pBssEdca->bACM[i] = (BOOLEAN) simple_strtol(macptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSSACM[%d]=%d\n", i, pBssEdca->bACM[i]);
			}
		}
	}

	/*Apply new (BSS) WMM Parameter*/
	rtmp_read_bss_edca_from_file(pAd, tmpbuf, buffer);

	/*AckPolicy*/
	if (RTMPGetKeyParameter("AckPolicy", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			pAd->CommonCfg.AckPolicy[i] = (UCHAR) os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AckPolicy[%d]=%d\n", i, pAd->CommonCfg.AckPolicy[i]);
		}

		wlan_config_set_ack_policy_all(&pAd->wpf, pAd->CommonCfg.AckPolicy);
	}

#ifdef UAPSD_SUPPORT
	/*UAPSDCapable*/
	if (RTMPGetKeyParameter("UAPSDCapable", tmpbuf, 64, buffer, TRUE) ||
		RTMPGetKeyParameter("APSDCapable", tmpbuf, 64, buffer, TRUE)) { /* backward compatible with old SDK */

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (VALID_MBSS(pAd, i)) {
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.UapsdInfo.bAPSDCapable = \
						(UCHAR) os_str_tol(macptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "UAPSDCapable[%d]=%d\n", i,
						 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.UapsdInfo.bAPSDCapable);
			}
		}

		if (i == 1) {
			/*
				Old format in UAPSD settings: only 1 parameter
				i.e. UAPSD for all BSS is enabled or disabled.
			*/
			for (i = 1; VALID_MBSS(pAd, i); i++) {
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.UapsdInfo.bAPSDCapable =
					pAd->ApCfg.MBSSID[0].wdev.UapsdInfo.bAPSDCapable;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "UAPSDCapable[%d]=%d\n", i,
						 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.UapsdInfo.bAPSDCapable);
			}
		}

#ifdef APCLI_SUPPORT

		if (pAd->ApCfg.FlgApCliIsUapsdInfoUpdated == FALSE) {
			/*
				Backward:
				All UAPSD for AP Client interface is same as MBSS0
				when we can not find "ApCliUAPSDCapable".
				When we find "ApCliUAPSDCapable" hereafter, we will over-write.
			*/
			for (i = 0; i < MAX_APCLI_NUM; i++) {
				pAd->StaCfg[i].wdev.UapsdInfo.bAPSDCapable = \
						pAd->ApCfg.MBSSID[0].wdev.UapsdInfo.bAPSDCapable;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "default ApCliUAPSDCapable[%d]=%d\n",
						 i, pAd->StaCfg[i].wdev.UapsdInfo.bAPSDCapable);
			}
		}

#endif /* APCLI_SUPPORT */
	}

#endif /* UAPSD_SUPPORT */
}

#endif /* CONFIG_AP_SUPPORT */

INT rtmp_band_index_get_by_order(struct _RTMP_ADAPTER *pAd, UCHAR order)
{
	INT ret = DBDC_BAND0;
#ifdef MULTI_PROFILE

	if (is_multi_profile_enable(pAd)) {
#ifdef DEFAULT_5G_PROFILE

		if (order == 0)
			ret = DBDC_BAND1;

		if (order == 1)
			ret = DBDC_BAND0;

#else /*DEFAULT_5G_PROFILE*/

		if (order == 0)
			ret = DBDC_BAND0;

		if (order == 1)
			ret = DBDC_BAND1;

#endif /*DEFAULT_5G_PROFILE*/
	} else {
		if (order == 0)
			ret = DBDC_BAND0;

		if (order == 1)
			ret = DBDC_BAND1;
	}

#else /*MULTI_PROFILE*/

	if (order == 0)
		ret = DBDC_BAND0;

	if (order == 1)
		ret = DBDC_BAND1;

#endif /*MULTI_PROFILE*/
	return ret;
}


static UCHAR band_order_check(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR order)
{
	UCHAR ret = 0;

	if (pAd->CommonCfg.dbdc_mode) {
#ifdef MULTI_PROFILE

		if (is_multi_profile_enable(pAd)) {
#ifdef DEFAULT_5G_PROFILE

			if (((order == 0) && WMODE_CAP_5G(wdev->PhyMode))
				|| ((order == 1) && WMODE_CAP_2G(wdev->PhyMode)))
				ret = 1;

#else /*DEFAULT_5G_PROFILE*/

			if (((order == 0) && WMODE_CAP_2G(wdev->PhyMode))
				|| ((order == 1) && WMODE_CAP_5G(wdev->PhyMode)))
				ret = 1;

#endif /*DEFAULT_5G_PROFILE*/
		} else {
			if (((order == 0) && WMODE_CAP_2G(wdev->PhyMode))
				|| ((order == 1) && WMODE_CAP_5G(wdev->PhyMode)))
				ret = 1;
		}

#else /*MULTI_PROFILE*/

		if (((order == 0) && WMODE_CAP_2G(wdev->PhyMode))
			|| ((order == 1) && WMODE_CAP_5G(wdev->PhyMode)))
			ret = 1;

#endif /*MULTI_PROFILE*/
	} else
		ret = 1;

	return ret;
}

static struct wifi_dev *get_curr_wdev(struct _RTMP_ADAPTER *pAd, UCHAR idx)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (idx < MAX_MBSSID_NUM(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSSID[%d]\n", idx);
			return &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].wdev;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (idx < MAX_MULTI_STA) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "STA[%d]\n", idx);
			return &pAd->StaCfg[idx].wdev;
		}
	}
#endif /*CONFIG_STA_SUPPORT*/
	return NULL;
}

static void read_frag_thld_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i = 0;
	UINT32 frag_thld = 0;
	RTMP_STRING *macptr = NULL;
	struct wifi_dev *wdev = NULL;

	if (RTMPGetKeyParameter("FragThreshold", tmpbuf, PER_BSS_SIZE_5(pAd), buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			frag_thld = os_str_tol(macptr, 0, 10);

			if (frag_thld > MAX_FRAG_THRESHOLD || frag_thld < MIN_FRAG_THRESHOLD)
				frag_thld = MAX_FRAG_THRESHOLD;
			else if (frag_thld % 2 == 1)
				frag_thld -= 1;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "profile: FragThreshold[%d]=%d\n", i, frag_thld);

			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_frag_thld(wdev, frag_thld);
		}
	}
}

static VOID read_rts_pkt_thld_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i = 0;
	UINT32 rts_pkt_thld = 0;
	RTMP_STRING *macptr = NULL;
	struct wifi_dev *wdev = NULL;

	if (RTMPGetKeyParameter("RTSPktThreshold", tmpbuf, PER_BSS_SIZE_5(pAd), buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			rts_pkt_thld = os_str_tol(macptr, 0, 10);

			if ((rts_pkt_thld < 1) || (rts_pkt_thld > MAX_RTS_PKT_THRESHOLD))
				rts_pkt_thld = MAX_RTS_PKT_THRESHOLD;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "profile: RTSPktThreshold[%d]=%d\n", i, rts_pkt_thld);

			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_rts_pkt_thld(wdev, rts_pkt_thld);
		}
	}
}

static VOID read_rts_len_thld_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i = 0;
	UINT32 rts_thld = 0;
	RTMP_STRING *macptr = NULL;
	struct wifi_dev *wdev = NULL;


	if (RTMPGetKeyParameter("RTSThreshold", tmpbuf, 128, buf, FALSE)) {
		if (pAd->CommonCfg.dbdc_mode) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				rts_thld = (UINT32)os_str_tol(macptr, 0, 10);
				if ((rts_thld > MAX_RTS_THRESHOLD) || (rts_thld < 1))
					rts_thld = MAX_RTS_THRESHOLD;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "profile: RTSThreshold[%d]=%d\n", i, rts_thld);
				wdev = get_curr_wdev(pAd, i);
				if (wdev)
					wlan_config_set_rts_len_thld(wdev, rts_thld);
			}
		} else {
#ifdef CONFIG_AP_SUPPORT
			UCHAR mbss_idx = 0;
#endif
			rts_thld = (UINT32)os_str_tol(tmpbuf, 0, 10);
			if ((rts_thld > MAX_RTS_THRESHOLD) || (rts_thld < 1))
				rts_thld = MAX_RTS_THRESHOLD;
#ifdef CONFIG_AP_SUPPORT
			for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
				struct wifi_dev *mbss_wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, mbss_idx)].wdev;
				wlan_config_set_rts_len_thld(mbss_wdev, rts_thld);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"mbss[idx=%d], RTSThreshold[%d]=%d\n", mbss_idx, mbss_idx,
						 rts_thld);
			}
#endif
		}
	}
}

#ifdef CONFIG_STA_SUPPORT
static void rtmp_read_sta_wmm_parms_from_file(IN  PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	RTMP_STRING *macptr;
	UINT i = 0, j = 0;
	BOOLEAN bWmmEnable = FALSE;

	/*WmmCapable*/
	if (RTMPGetKeyParameter("WmmCapable", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= MAX_MULTI_STA) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Input parameter[WmmCapable] incorrect\n");
				return;
			}
			if (os_str_tol(macptr, 0, 10) != 0) { /*Enable*/
				pAd->StaCfg[i].wdev.bWmmCapable = TRUE;
				bWmmEnable = TRUE;
			} else /*Disable*/
				pAd->StaCfg[i].wdev.bWmmCapable = FALSE;

			if (i == 0) {
				/* First setting is also the default value for others */
				for (j = 1; j < MAX_MULTI_STA; j++)
					pAd->StaCfg[j].wdev.bWmmCapable = pAd->StaCfg[i].wdev.bWmmCapable;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WmmCapable=%d\n", pAd->StaCfg[i].wdev.bWmmCapable);
		}
	}

	/*AckPolicy for AC_BK, AC_BE, AC_VI, AC_VO*/
	if (RTMPGetKeyParameter("AckPolicy", tmpbuf, 32, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= WMM_NUM_OF_AC) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Input parameter[WmmCapable] incorrect\n");
				return;
			}
			pAd->CommonCfg.AckPolicy[i] = (UCHAR)os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AckPolicy[%d]=%d\n", i, pAd->CommonCfg.AckPolicy[i]);
		}

		wlan_config_set_ack_policy_all(&pAd->wpf, pAd->CommonCfg.AckPolicy);
	}

#ifdef UAPSD_SUPPORT

	if (bWmmEnable) {
		/*UAPSDCapable*/
		if (RTMPGetKeyParameter("UAPSDCapable", tmpbuf, 10, buffer, TRUE) ||
			RTMPGetKeyParameter("APSDCapable", tmpbuf, 10, buffer, TRUE)) { /* backward compatible with old SDK */
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				if (i >= MAX_MULTI_STA) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "Input parameter[UAPSDCapable] incorrect\n"
						);
					return;
				}
				if (os_str_tol(tmpbuf, 0, 10) != 0) /*Enable*/
					pAd->StaCfg[i].wdev.UapsdInfo.bAPSDCapable = TRUE;
				else
					pAd->StaCfg[i].wdev.UapsdInfo.bAPSDCapable = FALSE;

				if (i == 0) {
					/* First setting is also the default value for others */
					for (j = 1; j < MAX_MULTI_STA; j++)
						pAd->StaCfg[j].wdev.UapsdInfo.bAPSDCapable = pAd->StaCfg[i].wdev.UapsdInfo.bAPSDCapable;
				}

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "UAPSDCapable=%d\n", pAd->StaCfg[i].wdev.UapsdInfo.bAPSDCapable);
			}
		}

		/*MaxSPLength*/
		if (RTMPGetKeyParameter("MaxSPLength", tmpbuf, 10, buffer, TRUE)) {
			pAd->CommonCfg.MaxSPLength = os_str_tol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MaxSPLength=%d\n", pAd->CommonCfg.MaxSPLength);
		}

		/*APSDAC for AC_BE, AC_BK, AC_VI, AC_VO*/
		if (RTMPGetKeyParameter("APSDAC", tmpbuf, 32, buffer, TRUE)) {
			BOOLEAN apsd_ac[WMM_NUM_OF_AC] = { 0 };

			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				if (i >= WMM_NUM_OF_AC) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "Input parameter[APSDAC] incorrect\n"
						);
					return;
				}
				apsd_ac[i] = (BOOLEAN)os_str_tol(macptr, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APSDAC%d  %d\n", i,  apsd_ac[i]);
			}

			pAd->CommonCfg.bAPSDAC_BE = apsd_ac[0];
			pAd->CommonCfg.bAPSDAC_BK = apsd_ac[1];
			pAd->CommonCfg.bAPSDAC_VI = apsd_ac[2];
			pAd->CommonCfg.bAPSDAC_VO = apsd_ac[3];
			pAd->CommonCfg.bACMAPSDTr[0] = apsd_ac[0];
			pAd->CommonCfg.bACMAPSDTr[1] = apsd_ac[1];
			pAd->CommonCfg.bACMAPSDTr[2] = apsd_ac[2];
			pAd->CommonCfg.bACMAPSDTr[3] = apsd_ac[3];
		}
	}

#endif /* UAPSD_SUPPORT */
}

#ifdef XLINK_SUPPORT
static void rtmp_get_psp_xlink_mode_from_file(IN  PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	/* Xlink Mode*/
	if (RTMPGetKeyParameter("PSP_XLINK_MODE", tmpbuf, 32, buffer, TRUE)) {
		if (os_str_tol(tmpbuf, 0, 10) != 0) /* enable*/
			pAd->StaCfg[0].PSPXlink = TRUE;
		else /* disable*/
			pAd->StaCfg[0].PSPXlink = FALSE;

		AsicSetRxFilter(pAd);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PSP_XLINK_MODE=%d\n", pAd->StaCfg[0].PSPXlink);
	}
}
#endif /* XLINK_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef DOT11_VHT_AC
static VOID read_vht_sgi(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;

	if (RTMPGetKeyParameter("VHT_SGI", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				wlan_config_set_vht_sgi(wdev, val);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"VHT: Short GI for 80Mhz/160Mhz  = %s\n",
						 (val == GI_800) ? "Disabled" : "Enable");
			}
		}
	}
}

static VOID read_vht_stbc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;

	if (RTMPGetKeyParameter("VHT_STBC", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_vht_stbc(wdev, val);
		}
	}
}

static VOID read_vht_ldpc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;

	if (RTMPGetKeyParameter("VHT_LDPC", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_vht_ldpc(wdev, val);
		}
	}
}

static VOID read_vht_bw_sig(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;
	UCHAR *bwsig_str[] = {"NONE", "STATIC", "DYNAMIC"};

	if (RTMPGetKeyParameter("VHT_BW_SIGNAL", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);

			if (wdev) {
				if (val > BW_SIGNALING_DYNAMIC)
					val = BW_SIGNALING_DISABLE;
				wlan_config_set_vht_bw_sig(wdev, val);

				/* For MT7615 certi RTS signaling workaround */
				if (IS_MT7615(pAd)) {
					UINT32 value = 0;

					if (val > BW_SIGNALING_DISABLE) {
						MAC_IO_READ32(pAd->hdev_ctrl, TMAC_TCR, &value);
						value |= DCH_DET_DIS;
						MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_TCR, value);

						if (val == BW_SIGNALING_DYNAMIC)
							MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_AALCR0, 0x02020202);
					} else {
						MAC_IO_READ32(pAd->hdev_ctrl, TMAC_TCR, &value);
						value &= (~DCH_DET_DIS);
						MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_TCR, value);


						MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_AALCR0, 0x00000000);
					}
				}

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"VHT: BW SIGNALING = %s\n", bwsig_str[val]);
			}
		}
	}
}

static VOID read_vht_param_from_file(struct _RTMP_ADAPTER *pAd,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;
	long Value;
	UCHAR vht_bw;
	UCHAR cen_ch_2 = 0;
	UCHAR IdBss = 0;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* Channel Width */
	if (RTMPGetKeyParameter("VHT_BW", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			Value = os_str_tol(macptr, 0, 10);

			if (Value <= VHT_BW_8080)
				vht_bw = Value;
			else
				vht_bw = VHT_BW_2040;

			if (vht_bw == VHT_BW_8080 && !IS_PHY_CAPS(pChipCap->phy_caps, fPHY_CAP_BW160NC))
				vht_bw = VHT_BW_80;

			if (pAd->CommonCfg.dbdc_mode && (vht_bw > VHT_BW_80) &&
				!IS_PHY_CAPS(pChipCap->phy_caps, fPHY_CAP_BW160C_STD))
				vht_bw = VHT_BW_80;

			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
#ifdef DFS_ADJ_BW_ZERO_WAIT
				if (pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport == TRUE)
					DfsZeroWaitBW160StateUpdate(pAd, &vht_bw, wdev->channel);
#endif
#if defined(DFS_MT7916_DEDICATED_ZW) || defined(DFS_MT7981_DEDICATED_ZW)
				if (pAd->CommonCfg.DfsParameter.BW160DedicatedSup == TRUE)
				DfsZeroWaitBW160StateUpdate(pAd, &vht_bw, wdev->channel);
#endif
				wlan_config_set_vht_bw(wdev, vht_bw);
			}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
			if (WMODE_CAP_5G(wdev->PhyMode) && IS_SUPPORT_V10_DFS(pAd) && vht_bw == VHT_BW_2040) {
				/* Boot Time HT BW Update when VHT BW if VHT2040 */
				wlan_config_set_ht_bw(wdev, HT_BW_20);
#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
				pAd->CommonCfg.MCastPhyMode.field.BW = HT_BW_20;
				pAd->CommonCfg.MCastPhyMode_5G.field.BW = HT_BW_20;
#else
				pAd->CommonCfg.mcastphymode.field.BW = HT_BW_20;
#endif /* MCAST_VENDOR10_CUSTOM_FEATURE */
#endif /* MCAST_RATE_SPECIFIC */
			}
#endif
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "wdev[%d] VHT: Channel Width = %s MHz\n", i,
				  VhtBw2Str(vht_bw));

			/* for first time, update all same as ra0 */
			if (i == 0) {
				for (IdBss = 1; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
					wdev = get_curr_wdev(pAd, PF_TO_BSS_IDX(pAd, IdBss));
					if (wdev)
						wlan_config_set_vht_bw(wdev, vht_bw);
				}
			}
		}
	}

	/* VHT_SGI */
	read_vht_sgi(pAd, tmpbuf, buf);
	/* VHT_STBC */
	read_vht_stbc(pAd, tmpbuf, buf);
	/* VHT_LDPC */
	read_vht_ldpc(pAd, tmpbuf, buf);
	/* VHT_BW_SIGNAL */
	read_vht_bw_sig(pAd, tmpbuf, buf);

	/* Disallow non-VHT connection */
	if (RTMPGetKeyParameter("VHT_DisallowNonVHT", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value == 0)
			pAd->CommonCfg.bNonVhtDisallow = FALSE;
		else
			pAd->CommonCfg.bNonVhtDisallow = TRUE;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VHT: VHT_DisallowNonVHT = %d\n",
				 pAd->CommonCfg.bNonVhtDisallow);
	}

	/* VHT Secondary80 */
	if (RTMPGetKeyParameter("VHT_Sec80_Channel", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			Value = os_str_tol(tmpbuf, 0, 10);

			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				UCHAR ch_band = wlan_config_get_ch_band(wdev);
				cen_ch_2 = vht_cent_ch_freq((UCHAR)Value, VHT_BW_80, ch_band);
				wlan_config_set_cen_ch_2(wdev, cen_ch_2);
#if (DFS_ZEROWAIT_SUPPORT_8080 == 1)
				wdev->vht_sec_80_channel = Value;
#endif
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "wdev[%d] VHT: Secondary80 = %ld, Center = %d\n", i,
					  Value, cen_ch_2);
			}
	}

	/* 2.4G 256QAM */
	if (RTMPGetKeyParameter("G_BAND_256QAM", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);
		pAd->CommonCfg.g_band_256_qam = (Value) ? TRUE : FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "VHT: G_BAND_256QAM = %ld\n", Value);
	}

	/* Use VHT Rate for 2G Band */
	if (RTMPGetKeyParameter("UseVhtRateFor2g", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);
		pAd->CommonCfg.bUseVhtRateFor2g = (Value) ? TRUE : FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "VHT: UseVhtRateFor2g = %ld\n", Value);
	}
}

#endif /* DOT11_VHT_AC */

#ifdef DOT11_HE_AX
#ifdef FIXED_HE_GI_SUPPORT
static VOID read_he_gi_ltf(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;

	if (RTMPGetKeyParameter("FgiFltf", tmpbuf, PER_BSS_SIZE_8(pAd), buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				set_fgi_and_ltf_profile(pAd, wdev, val);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"WDEV[%d], FgiFltf = %d\n", i, val);
			}
		}
	}
}
#endif

static VOID read_he_ldpc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;

	if (RTMPGetKeyParameter("HeLdpc", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				wlan_config_set_he_ldpc(wdev, val);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"WDEV[%d], HeLdpc = %d\n", i, val);
			}
		}
	}
}

static VOID read_he_txop_dur_rts_thld(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	INT i;
	UINT32 val;
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;

	if (RTMPGetKeyParameter("HE_TXOP_RTS_THLD", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev) {
				wlan_config_set_he_txop_dur_rts_thld(wdev, val);

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"WDEV[%d], HE_TXOP_RTS_THLD = %d\n", i, val);
			}
		}
	}
}

static VOID read_ppdu_tx_type(struct _RTMP_ADAPTER *ad,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UINT8 ppdu_tx_type = 0;
	INT i;

	if (RTMPGetKeyParameter("PPDUTxType", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
				macptr = rstrtok(NULL, ";"), i++) {
			ppdu_tx_type = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(ad, i);
			if (wdev)
				wlan_config_set_ppdu_tx_type(wdev, ppdu_tx_type);
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"WDEV[%d], PPDUTxType = %d\n", i, ppdu_tx_type);
		}
	}
}

static VOID read_num_users_ofdma(struct _RTMP_ADAPTER *ad,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UINT8 user_cnt = 0;
	INT i;

	if (RTMPGetKeyParameter("NumUsersOFDMA", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
				macptr = rstrtok(NULL, ";"), i++) {
			user_cnt = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(ad, i);
			if (wdev)
				wlan_config_set_ofdma_user_cnt(wdev, user_cnt);
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"WDEV[%d], NumUsersOFDMA = %d\n", i, user_cnt);
		}
	}
}

static VOID read_mu_edca_override(struct _RTMP_ADAPTER *ad,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UINT8 input = 0;
	BOOLEAN override = FALSE;
	INT i;

	if (RTMPGetKeyParameter("MuEdcaOverride", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
				macptr = rstrtok(NULL, ";"), i++) {
			input = os_str_tol(macptr, 0, 10);
			override = (input == 0) ? FALSE : TRUE;
			wdev = get_curr_wdev(ad, i);
			if (wdev)
				wlan_config_set_mu_edca_override(wdev, override);
			MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"WDEV[%d], MuEdcaOverride = %d\n", i, override);
		}
	}
}

static VOID read_non_tx_bss_idx(struct _RTMP_ADAPTER *ad,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UINT8 bss_idx = 0;
	INT i;

	if (RTMPGetKeyParameter("NonTxBSSIndex", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
				macptr = rstrtok(NULL, ";"), i++) {
			bss_idx = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(ad, i);
			if (wdev)
				wlan_config_set_non_tx_bss_idx(wdev, bss_idx);
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"NonTxBSSIndex = %d\n", bss_idx);
		}
	}
}

static VOID read_ofdma_direction(struct _RTMP_ADAPTER *ad,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	INT i;
	UINT8 ofdma_dir = 0;

	if (RTMPGetKeyParameter("OFDMA", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
				macptr = rstrtok(NULL, ";"), i++) {
			ofdma_dir = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(ad, i);
			if (wdev)
				wlan_config_set_ofdma_direction(wdev, ofdma_dir);
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"OFDMA = %d\n", ofdma_dir);
		}
	}
}

static VOID read_vht_1024qam_support(struct _RTMP_ADAPTER *ad,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	long Value;

	/* VHT 1024QAM */
	if (RTMPGetKeyParameter("Vht1024QamSupport", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);
		ad->CommonCfg.vht_1024_qam = (Value) ? TRUE : FALSE;
		MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Vht1024QamSupport = %ld\n", Value);
	}
}

static VOID read_he_param_from_file(struct _RTMP_ADAPTER *pAd,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	read_he_ldpc(pAd, tmpbuf, buf);
#if defined(DOT11_HE_AX) && defined(FIXED_HE_GI_SUPPORT)
	read_he_gi_ltf(pAd, tmpbuf, buf);
#endif
	read_he_txop_dur_rts_thld(pAd, tmpbuf, buf);
	read_ppdu_tx_type(pAd, tmpbuf, buf);
	read_num_users_ofdma(pAd, tmpbuf, buf);
	read_mu_edca_override(pAd, tmpbuf, buf);
	read_non_tx_bss_idx(pAd, tmpbuf, buf);
	read_ofdma_direction(pAd, tmpbuf, buf);
	read_vht_1024qam_support(pAd, tmpbuf, buf);
}
#endif


#ifdef DOT11_N_SUPPORT
static VOID read_fixed_mcs(struct _RTMP_ADAPTER *ad,
		RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	INT i = 0;
	UCHAR fixed_mcs;

	if (RTMPGetKeyParameter("FixedMcs", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
				macptr = rstrtok(NULL, ";"), i++) {
			fixed_mcs = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(ad, i);
			if (wdev)
				wlan_config_set_fixed_mcs(wdev, fixed_mcs);
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"WDEV[%d], FixedMcs = %d\n", i, fixed_mcs);
		}
	}
}

static VOID read_min_mpdu_start_space(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	INT i = 0;
	UCHAR mpdu_density = 0;

	if (RTMPGetKeyParameter("HT_MpduDensity", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			mpdu_density = os_str_tol(macptr, 0, 10);
			if (mpdu_density > 7)
				mpdu_density = 4;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"HT: MPDU Density = %d\n", (INT) mpdu_density);

			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_min_mpdu_start_space(wdev, mpdu_density);
		}
	}
}

static VOID read_ht_protect(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht_protect_en = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_PROTECT", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht_protect_en = os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "HT_PROTECT=%s\n", (ht_protect_en) ? "Enable" : "Disable");
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_ht_protect_en(wdev, ht_protect_en);
		}
	}
}

static VOID read_ht_gi(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht_gi;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_GI", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht_gi = os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "HT_GI = %s\n", (ht_gi == GI_400) ? "GI_400" : "GI_800");
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_ht_gi(wdev, ht_gi);
		}
	}
}

static VOID read_40M_intolerant(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht40_intolerant = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_40MHZ_INTOLERANT", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht40_intolerant = os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"HT: 40MHZ INTOLERANT = %d\n", ht40_intolerant);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_40M_intolerant(wdev, ht40_intolerant);
		}
	}
}

static VOID read_ht_ldpc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht_ldpc = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_LDPC", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht_ldpc = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_ht_ldpc(wdev, ht_ldpc);
		}
	}
}

static VOID read_ht_stbc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht_stbc = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_STBC", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht_stbc = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_ht_stbc(wdev, ht_stbc);
		}
	}
}

static VOID read_ht_mode(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ht_mode = 0;
	INT i = 0;

	/* HT Operation Mode : Mixed Mode , Green Field*/
	if (RTMPGetKeyParameter("HT_OpMode", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ht_mode = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_ht_mode(wdev, ht_mode);
		}
	}
}

static VOID read_txrx_stream_num(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct mcs_nss_caps *nss_cap = &cap->mcs_nss;
	RTMP_STRING *macptr = NULL;
	UCHAR tx_stream = 0, rx_stream = 0;
	INT i = 0, j = 0, boundary_idx = MAX_MULTI_STA, start_idx = 0;

#if defined(CONFIG_AP_SUPPORT)
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		boundary_idx = pAd->ApCfg.BssidNum;
#endif

#ifdef MULTI_PROFILE
	if(is_multi_profile_enable(pAd))
		start_idx = multi_profile_get_pf1_num(pAd);
#endif

	if (RTMPGetKeyParameter("HT_TxStream", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			tx_stream = os_str_tol(macptr, 0, 10);

			for (j = i*start_idx; j < boundary_idx; j++) {
				wdev = get_curr_wdev(pAd, j);
				if (wdev) {
					wlan_config_set_tx_stream(wdev, min(tx_stream, nss_cap->max_nss[i]));
#ifdef DOT11_HE_AX
					wlan_config_set_he_tx_nss(wdev, min(tx_stream, nss_cap->max_nss[i]));
#endif
				}
			}
		}
	}

	if (RTMPGetKeyParameter("HT_RxStream", tmpbuf, 128, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			rx_stream = os_str_tol(macptr, 0, 10);

			for (j = i*start_idx; j < boundary_idx; j++) {
				wdev = get_curr_wdev(pAd, j);
				if (wdev) {
					wlan_config_set_rx_stream(wdev, min(rx_stream, nss_cap->max_nss[i]));
#ifdef DOT11_HE_AX
					wlan_config_set_he_rx_nss(wdev, min(rx_stream, nss_cap->max_nss[i]));
#endif
				}
			}
		}
	}
}

static VOID read_amsdu_enable(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR amsdu_enable = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HT_AMSDU", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			amsdu_enable = os_str_tol(macptr, 0, 10);

			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_amsdu_en(wdev, amsdu_enable);
		}
	}
}

static VOID read_amsdu_num(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UINT8 amsdu_num, max_mpdu_len = MPDU_7991_OCTETS;
	INT i = 0;

	if (RTMPGetKeyParameter("AMSDU_NUM", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			amsdu_num = os_str_tol(macptr, 0, 10);
			if (amsdu_num < 3)
				max_mpdu_len = MPDU_3895_OCTETS;
			if (amsdu_num > 4)
				max_mpdu_len = MPDU_11454_OCTETS;

			wdev = get_curr_wdev(pAd, i);
			if (wdev)
				wlan_config_set_vht_max_mpdu_len(wdev, max_mpdu_len);
		}
	}
}

static VOID read_mmps(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR mmps = 0;
	INT i = 0, idx = 0;

	if (RTMPGetKeyParameter("HT_MIMOPSMode", tmpbuf, 25, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i > DBDC_BAND_NUM)
				break;

			mmps = os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "HT: MIMOPS Mode  = %d\n", mmps);
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, idx)].wdev;
					if (band_order_check(pAd, wdev, i))
						wlan_config_set_mmps(wdev, mmps);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				for (idx = 0; idx < MAX_MULTI_STA; idx++) {
					wdev = &pAd->StaCfg[idx].wdev;
					if (band_order_check(pAd, wdev, i))
						wlan_config_set_mmps(wdev, mmps);
				}
			}
#endif /* CONFIG_STA_SUPPORT */
		}
	}
}

static VOID read_ba_related(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UINT16 val = 0;
	UINT32 i = 0;

	if (RTMPGetKeyParameter("HT_BADecline", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);

			if (wdev)
				wlan_config_set_ba_decline(wdev, (UINT8)val);
		}
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"HT: BA Decline  = %s\n", (val == 0) ? "Disable" : "Enable");
	}

	if (RTMPGetKeyParameter("HT_AutoBA", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);

			if (wdev)
				wlan_config_set_ba_enable(wdev, (UINT8)val);
		}
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"HT: Auto BA  = %s\n", (val == 0) ? "Disable" : "Enable");
	}

	if (RTMPGetKeyParameter("HT_BAWinSize", tmpbuf, PER_BSS_SIZE_4(pAd), buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			val = os_str_tol(macptr, 0, 10);
			wdev = get_curr_wdev(pAd, i);

			if (wdev)
				wlan_config_set_ba_txrx_wsize(wdev, val, val);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"HT_BAWinSize: wdev[%d]: (TX=%d, RX=%d)\n", i, val, val);
		}
	}
}

static VOID read_ht_bw(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	UCHAR ht_bw;
#ifdef HIGHPRI_RATE_SPECIFIC
	UINT8 frame_type;
#endif

	if (RTMPGetKeyParameter("HT_BW", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		ht_bw = os_str_tol(tmpbuf, 0, 10);
		wlan_config_set_ht_bw_all(&pAd->wpf, ht_bw);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "HT: Channel Width = %s\n",
				 (ht_bw == HT_BW_40) ? "40 MHz" : "20 MHz");
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			RTMP_STRING *Bufptr;
			struct wifi_dev *wdev;
			INT i;
			UCHAR IdBss = 0;

			for (i = 0, Bufptr = rstrtok(tmpbuf, ";"); (Bufptr && (i < MAX_MBSSID_NUM(pAd)));
				 Bufptr = rstrtok(NULL, ";"), i++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				ht_bw = os_str_tol(Bufptr, 0, 10);
				wlan_config_set_ht_bw(wdev, ht_bw);
#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
				wdev->rate.MCastPhyMode_5G.field.BW = ht_bw;
				wdev->rate.MCastPhyMode.field.BW = ht_bw;
#else
				wdev->rate.mcastphymode.field.BW = ht_bw;
#endif /* #ifdef MCAST_VENDOR10_CUSTOM_FEATURE */
#endif /* MCAST_RATE_SPECIFIC */
#ifdef HIGHPRI_RATE_SPECIFIC
				for (frame_type = HIGHPRI_ARP; frame_type < HIGHPRI_MAX_TYPE; frame_type++) {
					wdev->rate.HighPriPhyMode[frame_type].field.BW = ht_bw;
					wdev->rate.HighPriPhyMode_5G[frame_type].field.BW = ht_bw;
				}
#endif

				/* for first time, update all same as ra0 */
				if (i == 0) {
					for (IdBss = 1; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
						wdev = get_curr_wdev(pAd, PF_TO_BSS_IDX(pAd, IdBss));
						if (wdev != NULL) {
							wlan_config_set_ht_bw(wdev, ht_bw);
#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
							wdev->rate.MCastPhyMode_5G.field.BW = ht_bw;
							wdev->rate.MCastPhyMode.field.BW = ht_bw;
#else
							wdev->rate.mcastphymode.field.BW = ht_bw;
#endif /*MCAST_VENDOR10_CUSTOM_FEATURE */
#endif /* MCAST_RATE_SPECIFIC */
						} else {
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is NULL\n");
						}
					}
				}
			}
		}
#endif /*CONFIG_AP_SUPPORT*/
	}
}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
static VOID RTMPOldBWCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer, BOOLEAN isVHT)
{
	UCHAR bw = 0, curr_bw;
	RTMP_STRING *Bufptr = NULL;
	struct wifi_dev *wdev = NULL;
	INT i = 0;

	if ((Buffer == NULL) || (IS_SUPPORT_V10_DFS(pAd) == FALSE))
		return;

#ifdef CONFIG_AP_SUPPORT
	/* V10 -- AP Mode Only */
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		bw = os_str_tol(Buffer, 0, 10);

		/* Disallow Invalid Values */
		if ((!isVHT && bw > BW_40) || (isVHT && bw > BW_80)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Incorrect BW=%d\n",
				bw);
			return;
		}

		for (i = 0, Bufptr = rstrtok(Buffer, ";"); (Bufptr && (i < MAX_MBSSID_NUM(pAd)));
			Bufptr = rstrtok(NULL, ";"), i++) {
			bw = os_str_tol(Bufptr, 0, 10);

			wdev = get_curr_wdev(pAd, i);
			if (!wdev || !IS_V10_OLD_CHNL_VALID(wdev))
				continue;

			if (isVHT) {
				curr_bw = wlan_config_get_vht_bw(wdev);
				wlan_config_set_vht_bw(wdev, bw);
				if (curr_bw == VHT_BW_160 && bw < VHT_BW_160)
					SET_V10_W56_VHT160_SWITCH(pAd, TRUE);
				else if (curr_bw == VHT_BW_80 && bw < VHT_BW_80)
					SET_V10_W56_VHT80_SWITCH(pAd, TRUE);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"wdev[%d] VHT: Channel Width = %s MHz\n", i, VhtBw2Str(bw));
			} else {
				wlan_config_set_ht_bw(wdev, bw);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "HT: Channel Width = %s\n",
					(bw == HT_BW_40) ? "40 MHz" : "20 MHz");
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}
#endif

static VOID read_ht_param_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	UINT32 Value = 0;
	INT i = 0;
#ifdef CONFIG_AP_SUPPORT
	RTMP_STRING *Bufptr;
#endif /* CONFIG_AP_SUPPORT */

	/* HT_BW */
	read_ht_bw(pAd, tmpbuf, buf);
	/* Tx/Rx Stream */
	read_txrx_stream_num(pAd, tmpbuf, buf);
	/* HT_OpMode */
	read_ht_mode(pAd, tmpbuf, buf);
	/* HT_PROTECT */
	read_ht_protect(pAd, tmpbuf, buf);
	/* HT_GI */
	read_ht_gi(pAd, tmpbuf, buf);
	/* HT_LDPC */
	read_ht_ldpc(pAd, tmpbuf, buf);
	/* HT_STBC */
	read_ht_stbc(pAd, tmpbuf, buf);
	/* MPDU Density*/
	read_min_mpdu_start_space(pAd, tmpbuf, buf);
	/* 40_Mhz_Intolerant*/
	read_40M_intolerant(pAd, tmpbuf, buf);
	/* Tx A-MSUD */
	read_amsdu_enable(pAd, tmpbuf, buf);
	/* A-MSDU Number */
	read_amsdu_num(pAd, tmpbuf, buf);
	/* MMPS */
	read_mmps(pAd, tmpbuf, buf);
	/* Block Ack */
	read_ba_related(pAd, tmpbuf, buf);
	/* Fixed MCS */
	read_fixed_mcs(pAd, tmpbuf, buf);


	/* Reverse Direction Mechanism*/
	if (RTMPGetKeyParameter("HT_RDG", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value != 0 && IS_ASIC_CAP(pAd, fASIC_CAP_RDG))
			pAd->CommonCfg.bRdg = TRUE;
		else
			pAd->CommonCfg.bRdg = FALSE;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "HT: RDG = %s\n", (Value == 0) ? "Disable" : "Enable(+HTC)");
	}

	/* Fixed Tx mode : CCK, OFDM*/
	if (RTMPGetKeyParameter("FixedTxMode", tmpbuf, 25, buf, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, Bufptr = rstrtok(tmpbuf, ";"); (Bufptr && i < MAX_MBSSID_NUM(pAd));
				 Bufptr = rstrtok(NULL, ";"), i++) {
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.DesiredTransmitSetting.field.FixedTxMode =
					RT_CfgSetFixedTxPhyMode(Bufptr);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(IF-ra%d) Fixed Tx Mode = %d\n", i,
						 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.DesiredTransmitSetting.field.FixedTxMode);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			pAd->StaCfg[0].wdev.DesiredTransmitSetting.field.FixedTxMode =
				RT_CfgSetFixedTxPhyMode(tmpbuf);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Fixed Tx Mode = %d\n",
					 pAd->StaCfg[0].wdev.DesiredTransmitSetting.field.FixedTxMode);
		}
#endif /* CONFIG_STA_SUPPORT */
	}


	if (RTMPGetKeyParameter("HT_EXTCHA", tmpbuf, PER_BSS_SIZE_2(pAd), buf, TRUE)) {
		struct wifi_dev *wdev;
		UCHAR ext_cha;
#ifdef CONFIG_STA_SUPPORT
		Value = os_str_tol(tmpbuf, 0, 10);

		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			wdev = &pAd->StaCfg[MAIN_MSTA_ID].wdev;

		if (Value == 0)
			ext_cha = EXTCHA_BELOW;
		else
			ext_cha = EXTCHA_ABOVE;

		wlan_config_set_ext_cha(wdev, ext_cha);

			for (i = 0; i < MAX_MULTI_STA; i++) {
				wdev = &pAd->StaCfg[i].wdev;

				if (wlan_config_get_ext_cha(wdev) == EXTCHA_NOASSIGN)
					wlan_config_set_ext_cha(wdev, ext_cha);
			}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "HT: Ext Channel = %s\n", (Value == 0) ? "BELOW" : "ABOVE");
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

		for (i = 0, Bufptr = rstrtok(tmpbuf, ";"); (Bufptr && (i < MAX_MBSSID_NUM(pAd)));
			 Bufptr = rstrtok(NULL, ";"), i++) {
			Value = os_str_tol(Bufptr, 0, 10);
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;

			if (Value == 0)
				ext_cha = EXTCHA_BELOW;
			else
				ext_cha = EXTCHA_ABOVE;

			wlan_config_set_ext_cha(wdev, ext_cha);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "HT: WDEV[%x] Ext Channel = %s\n", i,
					 (Value == 0) ? "BELOW" : "ABOVE");
		}
		}

		ext_cha = wlan_config_get_ext_cha(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);

		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;

			if (wlan_config_get_ext_cha(wdev) == EXTCHA_NOASSIGN) {
				wlan_config_set_ext_cha(wdev, ext_cha);
			}
		}

#endif /*CONFIG_AP_SUPPORT*/
	}

	/* MSC*/
	if (RTMPGetKeyParameter("HT_MCS", tmpbuf, 50, buf, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0, Bufptr = rstrtok(tmpbuf, ";"); (Bufptr && i < MAX_MBSSID_NUM(pAd));
				 Bufptr = rstrtok(NULL, ";"), i++) {
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;

				Value = os_str_tol(Bufptr, 0, 10);

				if (Value <= MCS_32)
					wdev->DesiredTransmitSetting.field.MCS = Value;
				else
					wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(IF-ra%d) HT: MCS = %s(%d)\n",
						 i, (wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO ? "AUTO" : "Fixed"),
						 wdev->DesiredTransmitSetting.field.MCS);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;

			Value = os_str_tol(tmpbuf, 0, 10);

			if (Value <= MCS_32) {
				wdev->DesiredTransmitSetting.field.MCS  = Value;
#ifdef P2P_SUPPORT
				pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.DesiredTransmitSetting.field.MCS  = Value;
#endif /* P2P_SUPPORT */
				wdev->bAutoTxRateSwitch = FALSE;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "HT: MCS = %d\n", wdev->DesiredTransmitSetting.field.MCS);
			} else {
				wdev->DesiredTransmitSetting.field.MCS  = MCS_AUTO;
#ifdef P2P_SUPPORT
				pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.DesiredTransmitSetting.field.MCS  = MCS_AUTO;
#endif /* P2P_SUPPORT */
				wdev->bAutoTxRateSwitch = TRUE;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "HT: MCS = AUTO\n");
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	}


#ifdef GREENAP_SUPPORT

	/*Green AP*/
	if (RTMPGetKeyParameter("GreenAP", tmpbuf, 10, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value == 0)
			greenap_set_capability(pAd, FALSE);
		else
			greenap_set_capability(pAd, TRUE);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "HT: greenap_cap = %d\n", greenap_get_capability(pAd));
	}

#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT

	/* PcieAspm */
	if (RTMPGetKeyParameter("PcieAspm", tmpbuf, 10, buf, TRUE)) {

		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value == 0)
			set_pcie_aspm_dym_ctrl_cap(pAd, FALSE);
		else
			set_pcie_aspm_dym_ctrl_cap(pAd, TRUE);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"ChipI=%x, Value=%d, pcie_aspm in profile=%d\n",
			pAd->ChipID,
			Value,
			get_pcie_aspm_dym_ctrl_cap(pAd));
	}

#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	if (RTMPGetKeyParameter("TWTSupport", tmpbuf, 128, buf, TRUE)) {
		struct wifi_dev *wdev;
		RTMP_STRING *macptr = NULL;
		UINT i, twt_support[DBDC_BAND_NUM];
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i < DBDC_BAND_NUM) {
				twt_support[i] = os_str_tol(macptr, 0, 10);
#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
					if (i < MAX_MULTI_STA) {
						wdev = &pAd->StaCfg[i].wdev;
						wlan_config_set_he_twt_support(wdev, twt_support[i]);
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"%d:%s TWTsupport = %d\n", i,
							RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev), twt_support[i]);
					}
				}
			}
#endif /* CONFIG_STA_SUPPORT */
		}
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				UCHAR BandIdx = DBDC_BAND0;
				wdev = get_curr_wdev(pAd, i);
				if (!wdev) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Dereference null return value\n");
					return;
				}
				if (pAd->CommonCfg.dbdc_mode) {
					if (WMODE_CAP_5G(wdev->PhyMode))
						BandIdx = DBDC_BAND1;
					else
						BandIdx = DBDC_BAND0;
				}
				wlan_config_set_he_twt_support(wdev, twt_support[BandIdx]);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"%d:%s TWTsupport = %d\n", i,
					RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev), twt_support[BandIdx]);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	if (RTMPGetKeyParameter("TWTInfoFrame", tmpbuf, 128, buf, TRUE)) {
		RTMP_STRING *macptr = NULL;
		UINT i, twt_info_frame[DBDC_BAND_NUM];
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i < DBDC_BAND_NUM) {
				twt_info_frame[i] = os_str_tol(macptr, 0, 10);
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
			}
		}
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			struct wifi_dev *wdev;
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				UCHAR BandIdx = DBDC_BAND0;
				wdev = get_curr_wdev(pAd, i);
				if (wdev) {
					if (pAd->CommonCfg.dbdc_mode) {
						if (WMODE_CAP_5G(wdev->PhyMode))
							BandIdx = DBDC_BAND1;
						else
							BandIdx = DBDC_BAND0;
					}
					wlan_config_set_he_twt_info_frame(wdev, twt_info_frame[BandIdx]);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"%d:%s TWTInfoFrame = %d\n", i,
						RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev), twt_info_frame[BandIdx]);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

	/* HT_DisallowTKIP*/
	if (RTMPGetKeyParameter("HT_DisallowTKIP", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);

		if (Value == 1)
			pAd->CommonCfg.HT_DisallowTKIP = TRUE;
		else
			pAd->CommonCfg.HT_DisallowTKIP = FALSE;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "HT: Disallow TKIP mode = %s\n",
				 (pAd->CommonCfg.HT_DisallowTKIP == TRUE) ? "ON" : "OFF");
	}

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	if (RTMPGetKeyParameter("OBSSScanParam", tmpbuf, 32, buf, TRUE)) {
		int ObssScanValue, idx;
		RTMP_STRING *macptr;
		for (idx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), idx++) {
			ObssScanValue = os_str_tol(macptr, 0, 10);

			switch (idx) {
			case 0:
				if (ObssScanValue < 5 || ObssScanValue > 1000)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Invalid OBSSScanParam for Dot11OBssScanPassiveDwell(%d), should in range 5~1000\n", ObssScanValue);
				else {
					pAd->CommonCfg.Dot11OBssScanPassiveDwell = ObssScanValue;	/* Unit : TU. 5~1000*/
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11OBssScanPassiveDwell=%d\n",
							 ObssScanValue);
				}

				break;

			case 1:
				if (ObssScanValue < 10 || ObssScanValue > 1000)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Invalid OBSSScanParam for Dot11OBssScanActiveDwell(%d), should in range 10~1000\n", ObssScanValue);
				else {
					pAd->CommonCfg.Dot11OBssScanActiveDwell = ObssScanValue;	/* Unit : TU. 10~1000*/
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11OBssScanActiveDwell=%d\n",
							 ObssScanValue);
				}

				break;

			case 2:
				pAd->CommonCfg.Dot11BssWidthTriggerScanInt = ObssScanValue;	/* Unit : Second*/
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11BssWidthTriggerScanInt=%d\n",
						 ObssScanValue);
				break;

			case 3:
				if (ObssScanValue < 200 || ObssScanValue > 10000)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Invalid OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel(%d), should in range 200~10000\n", ObssScanValue);
				else {
					pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = ObssScanValue;	/* Unit : TU. 200~10000*/
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel=%d\n",
							 ObssScanValue);
				}

				break;

			case 4:
				if (ObssScanValue < 20 || ObssScanValue > 10000)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Invalid OBSSScanParam for Dot11OBssScanActiveTotalPerChannel(%d), should in range 20~10000\n", ObssScanValue);
				else {
					pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = ObssScanValue;	/* Unit : TU. 20~10000*/
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11OBssScanActiveTotalPerChannel=%d\n",
							 ObssScanValue);
				}

				break;

			case 5:
				pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = ObssScanValue;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11BssWidthChanTranDelayFactor=%d\n",
						 ObssScanValue);
				break;

			case 6:
				pAd->CommonCfg.Dot11OBssScanActivityThre = ObssScanValue;	/* Unit : percentage*/
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11BssWidthChanTranDelayFactor=%d\n",
						 ObssScanValue);
				break;
			}
		}

		if (idx != 7) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Wrong OBSSScanParamtetrs format in dat file!!!!! Use default value.\n");
			pAd->CommonCfg.Dot11OBssScanPassiveDwell = dot11OBSSScanPassiveDwell;	/* Unit : TU. 5~1000*/
			pAd->CommonCfg.Dot11OBssScanActiveDwell = dot11OBSSScanActiveDwell;	/* Unit : TU. 10~1000*/
			pAd->CommonCfg.Dot11BssWidthTriggerScanInt = dot11BSSWidthTriggerScanInterval;	/* Unit : Second	*/
			pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = dot11OBSSScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000*/
			pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = dot11OBSSScanActiveTotalPerChannel;	/* Unit : TU. 20~10000*/
			pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = dot11BSSWidthChannelTransactionDelayFactor;
			pAd->CommonCfg.Dot11OBssScanActivityThre = dot11BSSScanActivityThreshold;	/* Unit : percentage*/
		}

		pAd->CommonCfg.Dot11BssWidthChanTranDelay = ((UINT32)pAd->CommonCfg.Dot11BssWidthTriggerScanInt *
				(UINT32)pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11BssWidthChanTranDelay=%ld\n",
				 pAd->CommonCfg.Dot11BssWidthChanTranDelay);
	}

	if (RTMPGetKeyParameter("HT_BSSCoexistence", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);
		pAd->CommonCfg.bBssCoexEnable = ((Value == 1) ? TRUE : FALSE);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "HT: 20/40 BssCoexSupport = %s\n",
				 (pAd->CommonCfg.bBssCoexEnable == TRUE) ? "ON" : "OFF");
	}

	if (RTMPGetKeyParameter("HT_BSSCoexApCntThr", tmpbuf, 25, buf, TRUE)) {
		pAd->CommonCfg.BssCoexApCntThr = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "HT: 20/40 BssCoexApCntThr = %d\n",
				 pAd->CommonCfg.BssCoexApCntThr);
	}

#endif /* DOT11N_DRAFT3 */

	if (RTMPGetKeyParameter("BurstMode", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);
		pAd->CommonCfg.bRalinkBurstMode = ((Value == 1) ? 1 : 0);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "HT: RaBurstMode= %d\n", pAd->CommonCfg.bRalinkBurstMode);
	}

#endif /* DOT11_N_SUPPORT */

	if (RTMPGetKeyParameter("TXRX_RXV_ON", tmpbuf, 25, buf, TRUE)) {
		Value = os_str_tol(tmpbuf, 0, 10);
		pAd->CommonCfg.bTXRX_RXV_ON = Value;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TXRX_RXV_ON = %s\n", (Value == 1) ? "ON" : "OFF");
	}
}
#endif /* DOT11_N_SUPPORT */

#ifdef TXBF_SUPPORT
#if defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT)
static VOID read_itxbf(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ITxBfEn = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("ITxBfEn", tmpbuf, 128, buf, FALSE)) {
		/* Reset pAd->CommonCfg.ITxBfEn */
		pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ITxBfEn = os_str_tol(macptr, 0, 10);
			pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn |= ITxBfEn;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"ITxBfEn = %d\n", ITxBfEn);
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSSID[%d]\n", i);
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				}
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef STA_ITXBF_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "STA[%d]\n", i);
					wdev = &pAd->StaCfg[i].wdev;
				}
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				wlan_config_set_itxbf(wdev, ITxBfEn);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"MBSS[%d] ITxBfEn = %d\n", i, ITxBfEn);
			}
		}

		/* If wdev num > ITxBfEn num in profile, set wdev with the final ITxBfEn */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "More BSSID[%d]\n", i);
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_itxbf(wdev, ITxBfEn);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"More MBSS[%d] ITxBfEn = %d\n", i, ITxBfEn);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef STA_ITXBF_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "More STA[%d]\n", i);
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_itxbf(wdev, ITxBfEn);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"More MBSS[%d] ITxBfEn = %d\n", i, ITxBfEn);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Common.ITxBfEn = %d\n",
		pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn);
}
#endif /* defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT) */

static VOID read_etxbf(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR ETxBfEnCond = SUBF_OFF;
	INT i = 0;

	if (RTMPGetKeyParameter("ETxBfEnCond", tmpbuf, PER_BSS_SIZE_2(pAd), buf, FALSE)) {
		/* Reset pAd->CommonCfg.ETxBfEnCond */
		pAd->CommonCfg.ETxBfEnCond = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			ETxBfEnCond = os_str_tol(macptr, 0, 10);
			pAd->CommonCfg.ETxBfEnCond |= ETxBfEnCond;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "ETxBfEnCond = %d\n", ETxBfEnCond);
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSSID[%d]\n", i);
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				}
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "STA[%d]\n", i);
					wdev = &pAd->StaCfg[i].wdev;
				}
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				wlan_config_set_etxbf(wdev, ETxBfEnCond);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"MBSS[%d] ETxBfEnCond = %d\n", i, ETxBfEnCond);
			}
		}

		/* If wdev num > ETxBfEnCond num in profile, set wdev with the final ETxBfEnCond */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "More BSSID[%d]\n", i);
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_etxbf(wdev, ETxBfEnCond);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"More MBSS[%d] ETxBfEnCond = %d\n", i, ETxBfEnCond);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "More STA[%d]\n", i);
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_etxbf(wdev, ETxBfEnCond);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"More MBSS[%d] ETxBfEnCond = %d\n", i, ETxBfEnCond);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}
}

static VOID read_txbf_smth_intl_bypas(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	RTMP_STRING *macptr = NULL;
	UINT i = 0;

	if (RTMPGetKeyParameter("BfSmthIntlBbypass", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= DBDC_BAND_NUM)
				break;

			pAd->CommonCfg.BfSmthIntlBypass[i] = os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "BfSmthIntlBbypass[%d] = %d\n", i, pAd->CommonCfg.BfSmthIntlBypass[i]);
		}
	}
}

static VOID read_hera_stbc_priority(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	RTMP_STRING *macptr = NULL;
	UINT i = 0;

	if (RTMPGetKeyParameter("HeraStbcPriority", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= DBDC_BAND_NUM)
				break;

			pAd->CommonCfg.HeraStbcPriority[i] = os_str_tol(macptr, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "HeraStbcPriority[%d] = %d\n", i, pAd->CommonCfg.HeraStbcPriority[i]);
		}
	}
}


#ifdef DSCP_PRI_SUPPORT
static VOID read_dscp_pri_param(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr = NULL;
	RTMP_STRING tok_str[32];
	UCHAR i = 0, bss_idx = 0;
	int ret;

	if (RTMPGetKeyParameter("DscpPriMapEnable", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
		ULONG enable = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			enable = os_str_tol(macptr, 0, 10);

			if (enable < 2)
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].dscp_pri_map_enable = (UCHAR) (enable);
			else
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"invalid dscp_pri_map_enable value\n");

			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"==> enable:%d\n", pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].dscp_pri_map_enable);
		}
	}

	for (bss_idx = 0; bss_idx < pAd->ApCfg.BssidNum; bss_idx++) {
		ret = snprintf(tok_str, sizeof(tok_str), "DscpPriMapBss%d", bss_idx);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: snprintf error!\n", __func__);
			return;
		}
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 512, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				RTMP_STRING	*this_char;
				UINT8	dscpValue;
				INT8 pri;

				if (i > 63)
					break;

				this_char = strsep((char **)&macptr, ":");
				if (this_char == NULL) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"value not defined for Dscp and Priority\n");
					break;
				}

				dscpValue = simple_strtol(this_char, 0, 10);
				if (dscpValue > 63) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Invalid Dscp Value Valid Value between 0 to 63\n");
					break;
				}
				if (macptr == NULL) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Priority not defined for Dscp %d\n", dscpValue);
					break;
				}
				pri = simple_strtol(macptr, 0, 10);
				if (pri < -1  || pri > 7) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Invalid Priority value Valid value between 0 to 7\n");
					break;
				}
				if (pri == 0)
					pri = 3;
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Setting Pri %d for Dscp=%d\n", pri, dscpValue);
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, bss_idx)].dscp_pri_map[dscpValue] = pri;
			}
		}
	}
}
#endif /*DSCP_PRI_SUPPORT*/

static VOID read_txbf_param_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
#if defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT)
	/* ITxBfEn */
	read_itxbf(pAd, tmpbuf, buf);
#endif /* defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT) */

	/* ETxBfEnCond */
	read_etxbf(pAd, tmpbuf, buf);

	/* BfSmthIntlBbypass*/
	read_txbf_smth_intl_bypas(pAd, tmpbuf, buf);

	/* HeraStbcPriority */
	read_hera_stbc_priority(pAd, tmpbuf, buf);
}
#endif /* TXBF_SUPPORT */

#ifdef DOT11_HE_AX
static VOID read_mu_dl_ofdma(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR mu_dl_ofdma = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("MuOfdmaDlEnable", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			mu_dl_ofdma = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum)
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA)
					wdev = &pAd->StaCfg[i].wdev;
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				wlan_config_set_mu_dl_ofdma(wdev, mu_dl_ofdma);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"MBSS[%d] MuOfdmaDlEnable = %d\n", i, mu_dl_ofdma);
			}
		}

		/* If wdev num > ETxBfEnCond num in profile, set wdev with the final ETxBfEnCond */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_mu_dl_ofdma(wdev, mu_dl_ofdma);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"More MBSS[%d] MuOfdmaDlEnable = %d\n", i, mu_dl_ofdma);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_mu_dl_ofdma(wdev, mu_dl_ofdma);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"More MBSS[%d] MuOfdmaDlEnable = %d\n", i, mu_dl_ofdma);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}
}

static VOID read_mu_ul_ofdma(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR mu_ul_ofdma = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("MuOfdmaUlEnable", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			mu_ul_ofdma = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum)
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA)
					wdev = &pAd->StaCfg[i].wdev;
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				wlan_config_set_mu_ul_ofdma(wdev, mu_ul_ofdma);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"MBSS[%d] MuOfdmaUlEnable = %d\n", i, mu_ul_ofdma);
			}
		}

		/* If wdev num > ETxBfEnCond num in profile, set wdev with the final ETxBfEnCond */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_mu_ul_ofdma(wdev, mu_ul_ofdma);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"More MBSS[%d] MuOfdmaUlEnable = %d\n", i, mu_ul_ofdma);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_mu_ul_ofdma(wdev, mu_ul_ofdma);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"More MBSS[%d] MuOfdmaUlEnable = %d\n", i, mu_ul_ofdma);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}
}

static VOID read_mu_dl_mimo(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	ULONG etxbf_en_cond = 0;
	UCHAR mu_dl_mimo = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("MuMimoDlEnable", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			mu_dl_mimo = os_str_tol(macptr, 0, 10);


#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum)
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA)
					wdev = &pAd->StaCfg[i].wdev;
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				/* DL Mu-Mimo should be disabled if ETxBfEnCond is disabled */
				etxbf_en_cond = wlan_config_get_etxbf(wdev);
				if (etxbf_en_cond == 0)
					mu_dl_mimo = 0;

				wlan_config_set_mu_dl_mimo(wdev, mu_dl_mimo);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"MBSS[%d] MuMimoDlEnable = %d\n", i, mu_dl_mimo);
			}
		}

		/* If wdev num > ETxBfEnCond num in profile, set wdev with the final ETxBfEnCond */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_mu_dl_mimo(wdev, mu_dl_mimo);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"More MBSS[%d] MuMimoDlEnable = %d\n", i, mu_dl_mimo);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_mu_dl_mimo(wdev, mu_dl_mimo);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"More MBSS[%d] MuMimoDlEnable = %d\n", i, mu_dl_mimo);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}
}

static VOID read_mu_ul_mimo(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR mu_ul_mimo = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("MuMimoUlEnable", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			mu_ul_mimo = os_str_tol(macptr, 0, 10);


#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if (i < pAd->ApCfg.BssidNum)
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (i < MAX_MULTI_STA)
					wdev = &pAd->StaCfg[i].wdev;
			}
#endif /*CONFIG_STA_SUPPORT*/
			if (wdev) {
				wlan_config_set_mu_ul_mimo(wdev, mu_ul_mimo);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"MBSS[%d] MuMimoUlEnable = %d\n", i, mu_ul_mimo);
			}
		}

		/* If wdev num > ETxBfEnCond num in profile, set wdev with the final ETxBfEnCond */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (; i < pAd->ApCfg.BssidNum ; i++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				if (wdev) {
					wlan_config_set_mu_ul_mimo(wdev, mu_ul_mimo);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"More MBSS[%d] MuMimoUlEnable = %d\n", i, mu_ul_mimo);
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			for (; i < MAX_MULTI_STA; i++) {
				wdev = &pAd->StaCfg[i].wdev;
				if (wdev) {
					wlan_config_set_mu_ul_mimo(wdev, mu_ul_mimo);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"More MBSS[%d] MuMimoUlEnable = %d\n", i, mu_ul_mimo);
				}
			}
		}
#endif /*CONFIG_STA_SUPPORT*/
	}
}

static VOID read_mu_param_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct _RTMP_CHIP_CAP *chip_cap;
	enum PHY_CAP phy_caps;

	chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	phy_caps = chip_cap->phy_caps;

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_DL_MUOFDMA))
		read_mu_dl_ofdma(pAd, tmpbuf, buf);

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_UL_MUOFDMA))
		read_mu_ul_ofdma(pAd, tmpbuf, buf);

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_DL_MUMIMO))
		read_mu_dl_mimo(pAd, tmpbuf, buf);

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_UL_MUMIMO))
		read_mu_ul_mimo(pAd, tmpbuf, buf);
}

static VOID read_er_su_param_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct _RTMP_CHIP_CAP *chip_cap;
	enum PHY_CAP phy_caps;
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR HeErSuRxDisable = 0;
	INT i = 0;

	chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	phy_caps = chip_cap->phy_caps;

	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_ER_SU)) {
		if (RTMPGetKeyParameter("HeErSuRxDisable", tmpbuf, 128, buf, FALSE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				HeErSuRxDisable = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
					if ((i < pAd->ApCfg.BssidNum) && (i < MAX_BEACON_NUM))
						wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				}
#endif /* CONFIG_AP_SUPPORT */

				if (wdev) {
					wlan_config_set_er_su_rx_disable(wdev, HeErSuRxDisable);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"MBSS[%d] HeErSuRxDisable = %d\n", i, HeErSuRxDisable);
				}
			}
		}
	}
}

static VOID read_he_dyn_smps_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR he_dyn_smps = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HeDynSmps", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			he_dyn_smps = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if ((i < pAd->ApCfg.BssidNum) && (i < MAX_BEACON_NUM))
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */

			if (wdev) {
				wlan_config_set_he_dyn_smps(wdev, he_dyn_smps);
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"MBSS[%d] HeDynSmps = %d\n",
					i, wlan_config_get_he_dyn_smps(wdev));
			}
		}
	}
}

static VOID read_ul_mu_data_param_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	struct wifi_dev *wdev = NULL;
	RTMP_STRING *macptr = NULL;
	UCHAR HeOmiUlMuDataDisableRx = 0;
	INT i = 0;

	if (RTMPGetKeyParameter("HeOmiUlMuDataDisableRx", tmpbuf, 128, buf, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			HeOmiUlMuDataDisableRx = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				if ((i < pAd->ApCfg.BssidNum) && (i < MAX_BEACON_NUM))
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
			}
#endif /* CONFIG_AP_SUPPORT */

			if (wdev) {
				wlan_config_set_ul_mu_data_disable_rx(wdev, HeOmiUlMuDataDisableRx);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"MBSS[%d] HeOmiUlMuDataDisableRx = %d\n", i, HeOmiUlMuDataDisableRx);
			}
		}
	}
}

#endif /* DOT11_HE_AX */

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
static VOID read_eaprate_param_from_file(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buf)
{
	/*MgmRateSet*/
	INT eaprateidx = 0;
	RTMP_STRING *macptr = NULL;
	char		tok_str[128];
	int i = 0;
	int ret;

	for (eaprateidx = 1; eaprateidx < 3; eaprateidx++) {
		/*AccessPolicy0 */
		ret = sprintf(tok_str, "mgmrateset%d", eaprateidx);
		if (os_snprintf_error(sizeof(tok_str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: snprintf error!\n", __func__);
			return;
		}

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buf, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				UINT32 ratetype = 0, phymode = 0, mcs = 0;
				INT i4Recv = 0;
				HTTRANSMIT_SETTING *transmit = NULL;
				struct wifi_dev *wdev = NULL;

				if (i >= pAd->ApCfg.BssidNum)
					break;
				wdev = &pAd->ApCfg.MBSSID[i].wdev;

				if (macptr) {
					i4Recv = sscanf(macptr, "%d-%d-%d", &(ratetype), &(phymode), &(mcs));
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"RateType %d,PhyMode %d,Mcs %d\n", ratetype, phymode, (mcs));

					if (i4Recv != 3) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"Format Error!\n");
						break;
					}

					if (WMODE_CAP_5G(wdev->PhyMode)) {
						if (phymode == MCAST_CCK || phymode == EAP_CCK) {
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							("We could not set CCK mode for Mgm in 5G band!\n"));
							break;
						}
					}

					if (mcs > 15) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"Mcs must be in range of 0 to 15\n");
						return;
					}

					if (ratetype == BCN_TYPE) {
						wdev->eap.eap_bcnrate_en = TRUE;
						transmit = &wdev->eap.bcnphymode;
					} else if (ratetype == MGM_TYPE) {
						wdev->eap.eap_mgmrate_en = TRUE;
						transmit = &wdev->eap.mgmphymode;
					} else {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"ratetype must be BCN_TYPE or MGM_TYPE\n");
						return;
					}

					switch (phymode) {
					case EAP_RATE_DISABLE: /* disable */
						if (WMODE_CAP_2G(wdev->PhyMode)) {
							transmit->field.MODE = MODE_CCK;
							transmit->field.BW =  BW_20;
							transmit->field.MCS = RATE_1;
						} else {
							transmit->field.MODE = MODE_OFDM;
							transmit->field.BW =  BW_20;
							transmit->field.MCS = OfdmRateToRxwiMCS[RATE_6];
						}
						break;
					case EAP_CCK:	/* CCK */
						transmit->field.MODE = MODE_CCK;
						transmit->field.BW =  BW_20;
						if ((transmit->field.MCS > 11) ||
							((transmit->field.MCS > 3) && (transmit->field.MCS < 8)))
							transmit->field.MCS = 3;
						break;
					case EAP_OFDM:	/* OFDM */
						transmit->field.MODE = MODE_OFDM;
						transmit->field.BW =  BW_20;
						if (transmit->field.MCS > 7)
							transmit->field.MCS = 7;
						break;
					default:
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"unknown PhyMode %d.\n", phymode);
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"0:Disable, 1:CCK, 2:OFDM\n");
						break;
					}

					switch (transmit->field.MODE) {
					case MODE_CCK:
						if ((mcs <= 3) || (mcs >= 8 && mcs <= 11))
							transmit->field.MCS = mcs;
						else {
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"MCS must in range of 0 ~ 3 and 8 ~ 11 for CCK Mode.\n");
						}

							break;
					case MODE_OFDM:
						if (mcs > 7) {
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"MCS must in range from 0 to 7 for OFDM Mode.\n");
						} else
							transmit->field.MCS = mcs;
						break;
					default:
						transmit->field.MCS = mcs;
						break;
					}
				} else {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Format Error!\n");
					break;
				}
			}
		}
	}

/* SupRateSet */
	if (RTMPGetKeyParameter("suprateset", tmpbuf, 32, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			INT supratesetbitmap = 0, j = 0;
			UCHAR rate[] = {
				0x82, 0x84, 0x8b, 0x96, 0x8C, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6c};
			struct wifi_dev *wdev;
			struct legacy_rate *eap_legacy_rate;

			if (i >= pAd->ApCfg.BssidNum)
				break;

			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			eap_legacy_rate = &wdev->eap.eap_legacy_rate;

			supratesetbitmap = os_str_tol(macptr, 0, 10);
			wdev->eap.eap_suprate_en = TRUE;
			eap_legacy_rate->sup_rate_len = 0;
			eap_legacy_rate->ext_rate_len = 0;
			for (j = 0; j < MAX_LEN_OF_SUPPORTED_RATES; j++) {
				if (supratesetbitmap & (1 << j)) {
					if (WMODE_EQUAL(wdev->PhyMode, WMODE_B) && WMODE_CAP_2G(wdev->PhyMode)) {
						eap_legacy_rate->sup_rate[eap_legacy_rate->sup_rate_len] = rate[j];
						eap_legacy_rate->sup_rate_len++;
						wdev->eap.eapsupportcckmcs |= (1 << j);
						wdev->eap.eapsupportratemode |= SUPPORT_CCK_MODE;
					} else if (WMODE_CAP_5G(wdev->PhyMode) && (j > 3)) {
						eap_legacy_rate->sup_rate[eap_legacy_rate->sup_rate_len] = rate[j];
						eap_legacy_rate->sup_rate_len++;
						wdev->eap.eapsupportofdmmcs |= (1 << (j - 4));
						wdev->eap.eapsupportratemode |= SUPPORT_OFDM_MODE;
					} else {
						if ((j < 4) || (j == 5) || (j == 7) || (j == 9) || (j == 11)) {
							eap_legacy_rate->sup_rate[eap_legacy_rate->sup_rate_len] = rate[j];
							eap_legacy_rate->sup_rate_len++;
							if (j < 4) {
								wdev->eap.eapsupportcckmcs |= (1 << j);
								wdev->eap.eapsupportratemode |= SUPPORT_CCK_MODE;
							} else {
								wdev->eap.eapsupportofdmmcs |= (1 << (j - 4));
								wdev->eap.eapsupportratemode |= SUPPORT_OFDM_MODE;
							}
						} else {
							eap_legacy_rate->ext_rate[eap_legacy_rate->ext_rate_len] = rate[i] & 0x7f;
							eap_legacy_rate->ext_rate_len++;
							wdev->eap.eapsupportofdmmcs |= (1 << (j - 4));
							wdev->eap.eapsupportratemode |= SUPPORT_OFDM_MODE;
						}
					}
				}
			}
		}
	}
	/* HtSupRateSet */
	if (RTMPGetKeyParameter("htsuprateset", tmpbuf, 32, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			UINT32 htsupratesetbitmap = 0;
			struct wifi_dev *wdev;

			if (i >= pAd->ApCfg.BssidNum)
				break;

			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			htsupratesetbitmap = (UINT32) os_str_tol(macptr, 0, 10);
			wdev->eap.eap_htsuprate_en = TRUE;
			wdev->eap.eapsupporthtmcs = htsupratesetbitmap;
			wdev->eap.eapmcsset[0] = htsupratesetbitmap & 0x000000ff;
			wdev->eap.eapmcsset[1] = (htsupratesetbitmap & 0x0000ff00) >> 8;
			wdev->eap.eapmcsset[2] = (htsupratesetbitmap & 0x00ff0000) >> 16;
			wdev->eap.eapmcsset[3] = (htsupratesetbitmap & 0xff000000) >> 24;
		}
	}
	/* VhtSupRateSet */
	if (RTMPGetKeyParameter("vhtsuprateset", tmpbuf, 32, buf, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			UINT32 vhtsupratesetbitmap = 0;
			struct wifi_dev *wdev;

			if (i >= pAd->ApCfg.BssidNum)
				break;

			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			vhtsupratesetbitmap = (UINT32) os_str_tol(macptr, 0, 10);
			wdev->eap.eap_vhtsuprate_en = TRUE;
			wdev->eap.rx_mcs_map.mcs_ss1 = vhtsupratesetbitmap & 0x0000003;
			wdev->eap.rx_mcs_map.mcs_ss2 = (vhtsupratesetbitmap & 0x0000000c) >> 2;
			wdev->eap.rx_mcs_map.mcs_ss3 = (vhtsupratesetbitmap & 0x00000030) >> 4;
			wdev->eap.rx_mcs_map.mcs_ss4 = (vhtsupratesetbitmap & 0x000000c0) >> 6;
			wdev->eap.rx_mcs_map.mcs_ss5 = (vhtsupratesetbitmap & 0x00000300) >> 8;
			wdev->eap.rx_mcs_map.mcs_ss6 = (vhtsupratesetbitmap & 0x00000c00) >> 10;
			wdev->eap.rx_mcs_map.mcs_ss7 = (vhtsupratesetbitmap & 0x00003000) >> 12;
			wdev->eap.rx_mcs_map.mcs_ss8 = (vhtsupratesetbitmap & 0x0000c000) >> 14;

			wdev->eap.tx_mcs_map.mcs_ss1 = (vhtsupratesetbitmap & 0x00030000) >> 16;
			wdev->eap.tx_mcs_map.mcs_ss2 = (vhtsupratesetbitmap & 0x000c0000) >> 18;
			wdev->eap.tx_mcs_map.mcs_ss3 = (vhtsupratesetbitmap & 0x00300000) >> 20;
			wdev->eap.tx_mcs_map.mcs_ss4 = (vhtsupratesetbitmap & 0x00c00000) >> 22;
			wdev->eap.tx_mcs_map.mcs_ss5 = (vhtsupratesetbitmap & 0x03000000) >> 24;
			wdev->eap.tx_mcs_map.mcs_ss6 = (vhtsupratesetbitmap & 0x0c000000) >> 26;
			wdev->eap.tx_mcs_map.mcs_ss7 = (vhtsupratesetbitmap & 0x30000000) >> 28;
			wdev->eap.tx_mcs_map.mcs_ss8 = (vhtsupratesetbitmap & 0xc0000000) >> 30;
		}
	}
#ifdef DOT11_HE_AX
	if (RTMPGetKeyParameter("hesuprateset", tmpbuf, 32 * 4, buf, TRUE)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "hesuprateset=%s\n", tmpbuf);
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr;
		     macptr = rstrtok(NULL, ";"), i++) {
			struct wifi_dev *wdev;
			UINT32 mcs_set[3];
			INT j;

			if (i >= pAd->ApCfg.BssidNum)
				break;

			wdev = &pAd->ApCfg.MBSSID[i].wdev;

			memset(mcs_set, 0xff, sizeof(mcs_set));
			for (j = 0, macptr = rstrtok(macptr, ",");
			     macptr && (j < ARRAY_SIZE(mcs_set));
			     macptr = rstrtok(NULL, ","), j++) {
				mcs_set[j] = (UINT32)os_str_tol(macptr, 0, 0);
			}
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"mcs_set: 0x%x 0x%x 0x%x\n",
					mcs_set[0], mcs_set[1], mcs_set[2]);

			for (j = 0; j < HE_MAX_SUPPORT_STREAM; j++ ) {
				wdev->eap.rate.he80_rx_nss_mcs[j] = (mcs_set[0] >> (j*2)) & 0x3;
				wdev->eap.rate.he80_tx_nss_mcs[j] = (mcs_set[0] >> (16+j*2)) & 0x3;
				wdev->eap.rate.he160_rx_nss_mcs[j] = (mcs_set[1] >> (j*2)) & 0x3;
				wdev->eap.rate.he160_tx_nss_mcs[j] = (mcs_set[1] >> (16+j*2)) & 0x3;
				wdev->eap.rate.he8080_rx_nss_mcs[j] = (mcs_set[2] >> (j*2)) & 0x3;
				wdev->eap.rate.he8080_tx_nss_mcs[j] = (mcs_set[2] >> (16+j*2)) & 0x3;
			}

			wdev->eap.eap_hesuprate_en = TRUE;
		}
	}
#endif /* DOT11_HE_AX */
}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
void RTMPSetSTASSID(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RTMP_STRING *SSID)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);

	pStaCfg->SsidLen = (UCHAR) strlen(SSID);
	NdisZeroMemory(pStaCfg->Ssid, NDIS_802_11_LENGTH_SSID);
	NdisMoveMemory(pStaCfg->Ssid, SSID, pStaCfg->SsidLen);
	pStaCfg->LastSsidLen = pStaCfg->SsidLen;
	NdisZeroMemory(pStaCfg->LastSsid, NDIS_802_11_LENGTH_SSID);
	NdisMoveMemory(pStaCfg->LastSsid, SSID, pStaCfg->LastSsidLen);
	pStaCfg->MlmeAux.AutoReconnectSsidLen = pStaCfg->SsidLen;
	NdisZeroMemory(pStaCfg->MlmeAux.AutoReconnectSsid, NDIS_802_11_LENGTH_SSID);
	NdisMoveMemory(pStaCfg->MlmeAux.AutoReconnectSsid, SSID, pStaCfg->MlmeAux.AutoReconnectSsidLen);
	pStaCfg->MlmeAux.SsidLen = pStaCfg->SsidLen;
	NdisZeroMemory(pStaCfg->MlmeAux.Ssid, NDIS_802_11_LENGTH_SSID);
	NdisMoveMemory(pStaCfg->MlmeAux.Ssid, SSID, pStaCfg->MlmeAux.SsidLen);
}


void RTMPSetSTAPassPhrase(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RTMP_STRING *PassPh)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	int ret;

	PassPh[strlen(PassPh)] = '\0'; /* make STA can process .$^& for WPAPSK input */

	if ((!IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap))
	   )
		ret = FALSE;
	else
		ret = SetWPAPSKKey(pAd, PassPh, strlen(PassPh), (PUCHAR)pStaCfg->Ssid, pStaCfg->SsidLen, pStaCfg->PMK);

	/*TODO: review it, if the ret is already FALSE. */
	if (IS_AKM_OWE(wdev->SecConfig.AKMMap))
		ret = TRUE;

	if (ret == TRUE) {
		RTMPZeroMemory(pStaCfg->WpaPassPhrase, 64);
		RTMPMoveMemory(pStaCfg->WpaPassPhrase, PassPh, strlen(PassPh));
		pStaCfg->WpaPassPhraseLen = strlen(PassPh);

		if (IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap) ||
		    IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap) ||
		    IS_AKM_OWE(wdev->SecConfig.AKMMap)) {
			/* Start STA supplicant state machine*/
			pStaCfg->WpaState = SS_START;
		} else if IS_AKM_WPANONE(wdev->SecConfig.AKMMap)
			pStaCfg->WpaState = SS_NOTUSE;

#ifdef WSC_STA_SUPPORT
		NdisZeroMemory(pStaCfg->wdev.WscControl.WpaPsk, 64);
		pStaCfg->wdev.WscControl.WpaPskLen = 0;

		if ((strlen(PassPh) >= 8) && (strlen(PassPh) <= 64)) {
			NdisMoveMemory(pStaCfg->wdev.WscControl.WpaPsk, PassPh, strlen(PassPh));
			pStaCfg->wdev.WscControl.WpaPskLen = strlen(PassPh);
		}

#endif /* WSC_STA_SUPPORT */
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WPAPSK=%s\n", PassPh);
	}
}


inline void RTMPSetSTACipherSuites(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, NDIS_802_11_ENCRYPTION_STATUS WepStatus)
{
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	/* Update all wepstatus related*/
	pStaCfg->PairwiseCipher = SecEncryModeOldToNew(WepStatus);
	pStaCfg->GroupCipher = SecEncryModeOldToNew(WepStatus);
}

#ifdef CREDENTIAL_STORE

/*RECOVER THE OLD CONNECT INFO */
NDIS_STATUS RecoverConnectInfo(
	IN  RTMP_ADAPTER *pAd)
{
	INT idx;
	char ssidStr[NDIS_802_11_LENGTH_SSID + 1];

	NdisZeroMemory(&ssidStr[0], sizeof(ssidStr));
	RTMP_SEM_LOCK(&pAd->StaCtIf.Lock);

	if ((pAd->StaCtIf.Changeable == FALSE) || (pAd->StaCtIf.SsidLen > NDIS_802_11_LENGTH_SSID)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, " DRIVER INIT  not need to RecoverConnectInfo()\n");
		RTMP_SEM_UNLOCK(&pAd->StaCtIf.Lock);
		return 0;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->RecoverConnectInfo()\n");
	NdisMoveMemory(ssidStr, pAd->StaCtIf.Ssid, pAd->StaCtIf.SsidLen);
	RTMPSetSTASSID(pAd, &pAd->StaCfg[0].wdev, &ssidStr[0]);
	pAd->StaCfg[0].AuthMode = pAd->StaCtIf.AuthMode;
	pAd->StaCfg[0].WepStatus = pAd->StaCtIf.WepStatus;
#ifdef WPA_SUPPLICANT_SUPPORT
	pAd->StaCfg[0].wdev.IEEE8021X = pAd->StaCtIf.IEEE8021X;
	pAd->StaCfg[0].wpa_supplicant_info.DesireSharedKeyId = pAd->StaCtIf.DefaultKeyId;
#endif /* WPA_SUPPLICANT_SUPPORT // */
	pAd->StaCfg[0].DefaultKeyId = pAd->StaCtIf.DefaultKeyId;
	NdisMoveMemory(pAd->StaCfg[0].PMK, pAd->StaCtIf.PMK, 32);
	RTMPMoveMemory(pAd->StaCfg[0].WpaPassPhrase, pAd->StaCtIf.WpaPassPhrase, pAd->StaCfg[0].WpaPassPhraseLen);
	pAd->StaCfg[0].WpaPassPhraseLen = pAd->StaCtIf.WpaPassPhraseLen;

	for (idx = 0; idx < 4; idx++) {
		NdisMoveMemory(&pAd->SharedKey[BSS0][idx], &pAd->StaCtIf.SharedKey[BSS0][idx], sizeof(CIPHER_KEY));
#ifdef WPA_SUPPLICANT_SUPPORT
		NdisMoveMemory(&pAd->StaCfg[0].wpa_supplicant_info.DesireSharedKey[idx], &pAd->StaCtIf.SharedKey[BSS0][idx],
					   sizeof(CIPHER_KEY));
#endif /* WPA_SUPPLICANT_SUPPORT // */
	}

	if ((pAd->StaCfg[0].AuthMode == Ndis802_11AuthModeWPAPSK) ||
		(pAd->StaCfg[0].AuthMode == Ndis802_11AuthModeWPA2PSK)) {
		/* Start STA supplicant state machine */
		pAd->StaCfg[0].WpaState = SS_START;
	} else if (pAd->StaCfg[0].AuthMode == Ndis802_11AuthModeWPANone)
		pAd->StaCfg[0].WpaState = SS_NOTUSE;

	RTMP_SEM_UNLOCK(&pAd->StaCtIf.Lock);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--RecoverConnectInfo()\n");
	return 0;
}


/*STORE THE CONNECT INFO*/
NDIS_STATUS StoreConnectInfo(
	IN  RTMP_ADAPTER *pAd)
{
	INT idx;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->StoreConnectInfo()\n");
	RTMP_SEM_LOCK(&pAd->StaCtIf.Lock);
	pAd->StaCtIf.Changeable = TRUE;
	pAd->StaCtIf.SsidLen = pAd->CommonCfg.SsidLen;
	NdisMoveMemory(pAd->StaCtIf.Ssid, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen);
	pAd->StaCtIf.AuthMode = pAd->StaCfg[0].AuthMode;
	pAd->StaCtIf.WepStatus = pAd->StaCfg[0].WepStatus;
	pAd->StaCtIf.DefaultKeyId = pAd->StaCfg[0].DefaultKeyId;
#ifdef WPA_SUPPLICANT_SUPPORT
	pAd->StaCtIf.wpa_supplicant_info.DefaultKeyId = pAd->StaCfg[0].DesireSharedKeyId;
	pAd->StaCtIf.IEEE8021X = pAd->StaCfg[0].wdev.IEEE8021X;
#endif /* WPA_SUPPLICANT_SUPPORT // */
	NdisMoveMemory(pAd->StaCtIf.PMK, pAd->StaCfg[0].PMK, 32);
	RTMPMoveMemory(pAd->StaCtIf.WpaPassPhrase, pAd->StaCfg[0].WpaPassPhrase, pAd->StaCfg[0].WpaPassPhraseLen);
	pAd->StaCtIf.WpaPassPhraseLen = pAd->StaCfg[0].WpaPassPhraseLen;

	for (idx = 0; idx < 4; idx++)
		NdisMoveMemory(&pAd->StaCtIf.SharedKey[BSS0][idx], &pAd->SharedKey[BSS0][idx], sizeof(CIPHER_KEY));

	RTMP_SEM_UNLOCK(&pAd->StaCtIf.Lock);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--StoreConnectInfo()\n");
	return 0;
}

#endif /* CREDENTIAL_STORE */

#endif /* CONFIG_STA_SUPPORT */


void RTMPSetCountryCode(RTMP_ADAPTER *pAd, RTMP_STRING *CountryCode)
{
	if (strlen(CountryCode) == 2) {
		NdisMoveMemory(pAd->CommonCfg.CountryCode, CountryCode, 2);
		pAd->CommonCfg.CountryCode[2] = ' ';
#ifdef CONFIG_STA_SUPPORT
#ifdef EXT_BUILD_CHANNEL_LIST
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		NdisMoveMemory(pAd->StaCfg[0].StaOriCountryCode, CountryCode, 2);
#endif /* EXT_BUILD_CHANNEL_LIST */
#endif /* CONFIG_STA_SUPPORT */

		pAd->CommonCfg.bCountryFlag = TRUE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "CountryCode(%s) is invalid, length!=2.\n", CountryCode);
		pAd->CommonCfg.bCountryFlag = FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CountryCode=%s\n", pAd->CommonCfg.CountryCode);
}

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11V_MBSSID_SUPPORT
static VOID rtmp_read_dot11v_mbssid_cfg_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UINT32 i;
	CHAR *macptr;
	BOOLEAN bDot11vMbss;
	UINT32 BssidNumB0 = pAd->ApCfg.BssidNumPerBand[DBDC_BAND0];
#ifdef DBDC_MODE
	UINT32 BssidNumB1 = pAd->ApCfg.BssidNumPerBand[DBDC_BAND1];
#endif
	UCHAR ucDbdcIdx;

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		bDot11vMbss = os_str_tol(macptr, 0, 10);
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (i >= pAd->ApCfg.BssidNum)
				break;
			if (bDot11vMbss) {
				/* build per-band 11v mbssid mapping */
				if (i < BssidNumB0)
					pAd->ApCfg.dot11v_mbssid_bitmap[DBDC_BAND0] |= (1 << i);
#ifdef DBDC_MODE
				else if (i < (BssidNumB0 + BssidNumB1))
					pAd->ApCfg.dot11v_mbssid_bitmap[DBDC_BAND1] |= (1 << (i - BssidNumB0));
#endif
				else
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"ERROR (idx = %d)\n", i);
			}
		}
	}

	/* check if any non-trans bss exist */
	for (ucDbdcIdx = DBDC_BAND0; ucDbdcIdx < DBDC_BAND_NUM; ucDbdcIdx++) {
		if (pAd->ApCfg.dot11v_mbssid_bitmap[ucDbdcIdx])
			pAd->ApCfg.dot11v_mbssid_bitmap[ucDbdcIdx] |= BIT(0);
	}
}
#endif

#ifdef CONFIG_6G_SUPPORT
static VOID rtmp_read_6g_discov_cfg_from_file(RTMP_ADAPTER *pAd, char *tmpbuf, char *pBuffer)
{
	UINT8	apidx;
	UCHAR	iob_type, iob_interval, iob_mode;
	UCHAR	oob_rnr_2g, oob_rnr_5g, oob_rnr_6g;
	BOOLEAN cfg_valid[4] = {FALSE};
	BOOLEAN iob_type_by_cfg = FALSE;

	/* read 6g iob config */
	if (RTMPGetKeyParameter("He6gIobType", tmpbuf, 25, pBuffer, TRUE)) {
		if (strlen(tmpbuf) != 0) {
			iob_type = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			cfg_valid[0] = TRUE;
			iob_type_by_cfg = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "He6gIobType=%d\n", iob_type);
		}
	}

	if (RTMPGetKeyParameter("He6gIobTu", tmpbuf, 25, pBuffer, TRUE)) {
		if (strlen(tmpbuf) != 0) {
			iob_interval = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			cfg_valid[1] = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "He6gIobTu=%d\n", iob_interval);
		}
	} else
		iob_interval = UNSOLICIT_TX_INTERVAL;

	if (RTMPGetKeyParameter("He6gIobMode", tmpbuf, 25, pBuffer, TRUE)) {
		if (strlen(tmpbuf) != 0) {
			iob_mode = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			cfg_valid[2] = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "He6gIobMode=%d\n", iob_mode);
		}
	}

	/* read 6g oob config */
	if (RTMPGetKeyParameter("He6gOob", tmpbuf, 25, pBuffer, TRUE)) {
		if (strlen(tmpbuf) != 0) {
			oob_rnr_2g = oob_rnr_5g = oob_rnr_6g = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			cfg_valid[3] = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "He6gOob=%d\n", oob_rnr_2g);
		}
	}

	/* apply setting to wdev */
	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev;

		if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G)) {
			/* 6g: default config if parameter not found */
			if (!cfg_valid[0]) {
				// if (bss_mgmt_is_2g_5g_exist())
				if (bssmnger_is_entry_exist_2g_5g())
					iob_type =		UNSOLICIT_TX_DISABLE;
				else
					iob_type =		UNSOLICIT_TX_PROBE_RSP;
			}
			if (!cfg_valid[1])
				iob_interval =	UNSOLICIT_TX_INTERVAL;
			if (!cfg_valid[2])
				iob_mode = (pAd->CommonCfg.wifi_cert) ?
							UNSOLICIT_TXMODE_HE_SU : UNSOLICIT_TXMODE_NON_HT;
			if (!cfg_valid[3]) {
				oob_rnr_2g =	RNR_REPORTING_MAIN_BSS;
				oob_rnr_5g =	RNR_REPORTING_MAIN_BSS;
				oob_rnr_6g =	RNR_REPORTING_NONE;	/* No 6G RNR in-band */
			}

			in_band_discovery_update(wdev, iob_type, iob_interval, iob_mode, iob_type_by_cfg);
			out_band_discovery_update(wdev, oob_rnr_2g, oob_rnr_5g, oob_rnr_6g);

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Bss(%d): phymode=%d, iob_type=%d%s, iob_interval=%d, iob_mode=%d, oob_rnr=%d/%d/%d\n",
				apidx, wdev->PhyMode, iob_type,
				iob_type_by_cfg ? "(byCfg)" : "",
				iob_interval, iob_mode,
				oob_rnr_2g, oob_rnr_5g, oob_rnr_6g);
		} else {
			/* 2g/5g: default config */
			in_band_discovery_update(wdev, UNSOLICIT_TX_DISABLE, 0, UNSOLICIT_TXMODE_NON_HT, FALSE);
			out_band_discovery_update(wdev, RNR_REPORTING_NONE, RNR_REPORTING_NONE, RNR_REPORTING_NONE);
		}

		wlan_operate_set_he_6g_qos_state(wdev, false);
		wlan_operate_set_he_6g_qos_tu(wdev, 0);
	}

}
#endif

static VOID RTMPDtimPeriodCfg(RTMP_ADAPTER *pAd, RTMP_STRING *Buffer)
{
	UINT32 i;
	CHAR *macptr;
	BSS_STRUCT *pMbss = NULL;
	UINT8 DtimPeriod;

	for (i = 0, macptr = rstrtok(Buffer, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
		DtimPeriod = os_str_tol(macptr, 0, 10);
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			if (i == 0)
				pAd->ApCfg.DtimPeriod = DtimPeriod;

			pMbss = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)];
			pMbss->DtimPeriod = DtimPeriod;
		}
	}

#ifdef MBSS_SUPPORT
	/* Check if any 0 case, apply MSSID 0 DtimPeriod to it.*/
	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		pMbss = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)];

		if (pMbss->DtimPeriod == 0)
			pMbss->DtimPeriod = pAd->ApCfg.DtimPeriod;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"BSS%d DtimPeriod=%d\n", i, pMbss->DtimPeriod);
	}
#endif/*MBSS_SUPPORT*/

}
#endif /*CONFIG_AP_SUPPORT*/

NDIS_STATUS	RTMPSetPreProfileParameters(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *tmpbuf;
#ifdef SW_CONNECT_SUPPORT
	BOOLEAN bSw = FALSE;
#endif /* SW_CONNECT_SUPPORT */

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (tmpbuf == NULL)
		return NDIS_STATUS_FAILURE;

#ifdef SW_CONNECT_SUPPORT
		if (RTMPGetKeyParameter("SwStaEnable", tmpbuf, 25, pBuffer, TRUE)) {
			/* UserCfgInit() default off , need to set in latter */
			UCHAR On = os_str_tol(tmpbuf, 0, 10);
			bSw = On ?  TRUE : FALSE;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "Pre : SwStaEnable=%d, SKIP WHNAT=%s\n", On, (bSw ? "YES" : "NO"));
		}

#endif /* SW_CONNECT_SUPPORT */

	/*WHNAT*/
#ifdef WHNAT_SUPPORT
#ifdef SW_CONNECT_SUPPORT
	/* S/W connect don't support H/W offload cases */
	if (bSw == TRUE) {
		RTMPWHNATCfg(pAd, "0");
	} else
#endif /* SW_CONNECT_SUPPORT */
	{
		if (RTMPGetKeyParameter("WHNAT", tmpbuf, 10, pBuffer, TRUE))
			RTMPWHNATCfg(pAd, tmpbuf);
	}

	if (RTMPGetKeyParameter("WED_IDX", tmpbuf, 10, pBuffer, FALSE)) {
		pAd->CommonCfg.wed_idx = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "WED_IDX=%d\n", pAd->CommonCfg.wed_idx);
	}

#endif /*WHNAT_SUPPORT*/

#if defined(MT7986)
	if (RTMPGetKeyParameter("DBDCDualPCIE", tmpbuf, 10, pBuffer, TRUE))
		RTMPDBDCDualPCIECfg(pAd, tmpbuf);
#endif /* MT7986 */

#ifdef MULTI_INTR_SUPPORT
	if (RTMPGetKeyParameter("MultiIntr", tmpbuf, 10,  pBuffer, TRUE))
		RTMPMultiIntrCfg(pAd, tmpbuf);
#endif

	os_free_mem(tmpbuf);
	return NDIS_STATUS_SUCCESS;
}

#ifdef MGMT_TXPWR_CTRL
void rtmp_read_mgmt_pwr_parms_from_file(
		IN RTMP_ADAPTER *pAd,
		IN RTMP_STRING *tmpbuf,
		IN RTMP_STRING *pBuffer)
{
	UINT32 i4Recv, value0, value1;
	UINT8 i;
	RTMP_STRING *macptr;

	if (RTMPGetKeyParameter("MgmtTxPwr", tmpbuf, 256, pBuffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;
			i4Recv = sscanf(macptr, "%d.%d", &(value0), &(value1));

			 /* in 0.5 db scale*/
			value0 *= 2;
			if (i4Recv == 2 && value1 == 5)
				value0++;

			pAd->ApCfg.MBSSID[i].wdev.MgmtTxPwr = value0;
			pAd->ApCfg.MBSSID[i].wdev.MgmtTxPwrBak = value0;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(ra%d) MgmtTxPwr=%d\n", i, value0);
		}
	}
}
#endif

#ifdef DATA_TXPWR_CTRL
NDIS_STATUS rtmp_read_data_pwr_parms_from_file(
		IN RTMP_ADAPTER *pAd,
		IN RTMP_STRING *tmpbuf)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT8 u1BandIdx, u1BandNum;
	RTMP_STRING *ptr;

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode)
		u1BandNum = 2;
	else
		u1BandNum = 1;
#else
	u1BandNum = 1;
#endif /* DBDC_MODE */

	for (u1BandIdx = BAND0, ptr = rstrtok(tmpbuf, ";"); ptr; ptr = rstrtok(NULL, ";"), u1BandIdx++) {
		if (u1BandIdx >= u1BandNum)
			return NDIS_STATUS_INVALID_DATA;

		pAd->ApCfg.MinBaseTxPwr[u1BandIdx] = os_str_tol(ptr, 0, 10);
		pAd->ApCfg.MinBaseTxPwr[u1BandIdx] = pAd->ApCfg.MinBaseTxPwr[u1BandIdx] * 2;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MinBaseTxPwr[%d] = %d", u1BandIdx, pAd->ApCfg.MinBaseTxPwr[u1BandIdx]);
	}

	return Status;
}
#endif
#ifdef ANTENNA_CONTROL_SUPPORT
void rtmp_read_ant_ctrl_parms_from_file(
		IN RTMP_ADAPTER *pAd,
		IN RTMP_STRING *tmpbuf,
		IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr;
	struct wifi_dev *wdev;
	UINT32 Txstream = 0, Rxstream = 0;
	UINT8 i, Band_idx = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct mcs_nss_caps *nss_cap = &cap->mcs_nss;

	if (RTMPGetKeyParameter("AntCtrl", tmpbuf, 32, pBuffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			if (wdev == NULL) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Incorrect BSS\n");
				continue;
			}

			/* Updating default band index 0 since causing issue in non dbdc mode */
			Band_idx = DBDC_BAND0;
			if ((pAd->CommonCfg.dbdc_mode) && (WMODE_CAP_5G(wdev->PhyMode)))
				Band_idx = DBDC_BAND1;

			/* Required in multi bss in dbdc mode */
			if ((pAd->CommonCfg.dbdc_mode) && (pAd->TxStream[Band_idx] != 0))
				Band_idx = !Band_idx;

			/* Default value of antenna when input 0 */
			if (0 == os_str_tol(macptr, 0, 10)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Band_idx = %d default Antenna number!!\n", Band_idx);
				goto set_default;
			}

			if (strlen(macptr) != 4) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Please use input format like xTxR (x <= %d)!!\n",
						nss_cap->max_nss[Band_idx]);
				goto set_default;
			}

			if (((macptr[1] == 'T') || (macptr[1] == 't')) && ((macptr[3] == 'R') || (macptr[3] == 'r'))) {
				Txstream = simple_strtol(&macptr[0], 0, 10);
				Rxstream = simple_strtol(&macptr[2], 0, 10);

				if (Txstream != Rxstream) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Band_idx = %d Tx & Rx Antenna number different, Set to Default!!\n",
							Band_idx);
					goto set_default;
				}
#ifdef DBDC_MODE
				if (pAd->CommonCfg.dbdc_mode) {

					if ((Band_idx == DBDC_BAND0 && (Txstream > pAd->dbdc_band0_tx_path))
						|| (Band_idx == DBDC_BAND1 && (Txstream > pAd->dbdc_band1_tx_path))) {

							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"Band_idx=%d Wrong Configuratio Ant number(%d/%d) > MAX TX/RX Support!!\n",
								Band_idx, Txstream, Rxstream);
							goto set_default;
					}
				} else
#endif
				{
					if (Txstream >  pAd->Antenna.field.TxPath) {
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"Band_idx=%d Wrong Configuration Ant number > MAX TX Support == %d!!\n",
								Band_idx, pAd->Antenna.field.TxPath);
						goto set_default;
					}
				}
				pAd->TxStream[Band_idx] = Txstream;
				pAd->RxStream[Band_idx] = Rxstream;
				pAd->bAntennaSetAPEnable[Band_idx] = 1;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					" Band_idx=%d, Tx_Stream=%d, Rx_Stream=%d\n",
						Band_idx, pAd->TxStream[Band_idx], pAd->RxStream[Band_idx]);
				continue;
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Invalid input\n");
				continue;
			}
set_default:
#ifdef DOT11_HE_AX
			if (WMODE_CAP_AX(wdev->PhyMode)) {
				pAd->RxStream[Band_idx] = wlan_config_get_he_rx_nss(wdev);
				pAd->TxStream[Band_idx] = wlan_config_get_he_tx_nss(wdev);
			} else
#endif
			{
#ifdef DBDC_MODE
				if (pAd->CommonCfg.dbdc_mode) {

					if (Band_idx == DBDC_BAND0) {
						pAd->TxStream[Band_idx] = pAd->dbdc_band0_tx_path;
						pAd->RxStream[Band_idx] = pAd->dbdc_band0_rx_path;
					} else if (Band_idx == DBDC_BAND1) {
						pAd->TxStream[Band_idx] = pAd->dbdc_band1_tx_path;
						pAd->RxStream[Band_idx] = pAd->dbdc_band1_rx_path;
					}
				} else
#endif
				{
					pAd->TxStream[DBDC_BAND0] = pAd->Antenna.field.TxPath;
					pAd->RxStream[DBDC_BAND0] = pAd->Antenna.field.RxPath;
				}
			}
			pAd->bAntennaSetAPEnable[Band_idx] = 0;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				" Band_idx=%d, Tx_path=%d, Rx_path=%d\n",
				Band_idx, pAd->TxStream[Band_idx], pAd->RxStream[Band_idx]);
		}
	}
}
#endif /* ANTENNA_CONTROL_SUPPORT */
#ifdef ANDLINK_FEATURE_SUPPORT
void rtmp_read_andlink_parms_from_file(
		IN RTMP_ADAPTER *pAd,
		IN RTMP_STRING *tmpbuf,
		IN RTMP_STRING *pBuffer)
{
	UCHAR bandidx = 0;
	UINT16 tmp_value = 0;
	RTMP_STRING *macptr = NULL;

	/*init the value,default is disable*/
	for (bandidx = 0; bandidx < DBDC_BAND_NUM; bandidx++) {
		pAd->CommonCfg.andlink_enable[bandidx] = FALSE;
		pAd->CommonCfg.andlink_ip_hostname_en[bandidx] = FALSE;
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "andlink_en=%d, in BAND: %d\n",
					pAd->CommonCfg.andlink_enable[bandidx], bandidx);
	/*andlink_en*/
	if (RTMPGetKeyParameter("andlink_en", tmpbuf, 256, pBuffer, FALSE)) {
		tmp_value = 0;
		bandidx = 0;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "andlink_en=%d, in BAND: %d\n",
					pAd->CommonCfg.andlink_enable[bandidx], bandidx);
		for (bandidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), bandidx++) {
			if (bandidx < DBDC_BAND_NUM) {
				tmp_value = (UINT16)os_str_tol(macptr, 0, 10);
				pAd->CommonCfg.andlink_enable[bandidx] = tmp_value > 0 ? TRUE : FALSE;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "andlink_en=%d, in BAND: %d\n",
					pAd->CommonCfg.andlink_enable[bandidx], bandidx);
			}
		}
	}

	/*andlink_ip_hostname_en*/
	if (RTMPGetKeyParameter("andlink_ip_hostname_en", tmpbuf, 256, pBuffer, FALSE)) {
			tmp_value = 0;
			bandidx = 0;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "andlink_ip_hostname_en=%d, in BAND: %d\n",
					pAd->CommonCfg.andlink_ip_hostname_en[bandidx], bandidx);
			for (bandidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), bandidx++) {
				if (bandidx < DBDC_BAND_NUM) {
					tmp_value = (UINT16)os_str_tol(macptr, 0, 10);
					pAd->CommonCfg.andlink_ip_hostname_en[bandidx] = tmp_value > 0 ? TRUE : FALSE;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "andlink_ip_hostname_en=%d, in BAND: %d\n",
					pAd->CommonCfg.andlink_ip_hostname_en[bandidx], bandidx);
				}
			}
		}
}

#endif/*ANDLINK_FEATURE_SUPPORT*/
#ifdef ACK_CTS_TIMEOUT_SUPPORT
void rtmp_read_ackcts_timeout_parms_from_file(
		IN RTMP_ADAPTER *pAd,
		IN RTMP_STRING *tmpbuf,
		IN RTMP_STRING *pBuffer) {

	UCHAR bandidx = 0;
	RTMP_STRING *macptr = NULL;

	/*init the value, then will not set. keep CR default value*/
	for (bandidx = 0; bandidx < DBDC_BAND_NUM; bandidx++) {
		pAd->CommonCfg.ack_cts_enable[bandidx] = FALSE;
		pAd->CommonCfg.cck_timeout[bandidx] = DEFALT_TMAC_CDTR_VALUE;
		pAd->CommonCfg.ofdm_timeout[bandidx] = DEFALT_TMAC_ODTR_VALUE;
		pAd->CommonCfg.ofdma_timeout[bandidx] = DEFALT_TMAC_OMDTR_VALUE;
		pAd->CommonCfg.distance[bandidx] = INVALID_DISTANCE;
	}

	if (RTMPGetKeyParameter("ACK_CTS_TOUT_EN", tmpbuf, 256, pBuffer, FALSE)) {
		UINT16 tmp_value = 0;
		UCHAR bandidx = 0;

		for (bandidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), bandidx++) {
			if (bandidx < DBDC_BAND_NUM) {
				tmp_value = (UINT16)os_str_tol(macptr, 0, 10);
				pAd->CommonCfg.ack_cts_enable[bandidx] = tmp_value > 0 ? TRUE : FALSE;

			}
		}
	}

	if (RTMPGetKeyParameter("CCK_ACK_TOUT", tmpbuf, 256, pBuffer, FALSE)) {
		UINT16 ack_timeout = 0;
		UCHAR bandidx = 0;

		for (bandidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), bandidx++) {
			if (bandidx < DBDC_BAND_NUM) {
				ack_timeout = (UINT16)os_str_tol(macptr, 0, 10);
				pAd->CommonCfg.cck_timeout[bandidx] = ack_timeout;

			}
		}
	}

	if (RTMPGetKeyParameter("OFDM_ACK_TOUT", tmpbuf, 256, pBuffer, FALSE)) {
		UINT16 ack_timeout = 0;
		UCHAR bandidx = 0;

		for (bandidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), bandidx++) {
			if (bandidx < DBDC_BAND_NUM) {
				ack_timeout = (UINT16)os_str_tol(macptr, 0, 10);
				pAd->CommonCfg.ofdm_timeout[bandidx] = ack_timeout;
			}
		}
	}

	if (RTMPGetKeyParameter("OFDMA_ACK_TOUT", tmpbuf, 256, pBuffer, FALSE)) {
		UINT16 ack_timeout = 0;
		UCHAR bandidx = 0;

		for (bandidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), bandidx++) {
			if (bandidx < DBDC_BAND_NUM) {
				ack_timeout = (UINT16)os_str_tol(macptr, 0, 10);
				pAd->CommonCfg.ofdma_timeout[bandidx] = ack_timeout;
			}
		}
	}

	if (RTMPGetKeyParameter("Distance", tmpbuf, 256, pBuffer, FALSE)) {
		UINT16 distance = 0;
		UCHAR bandidx = 0;

		for (bandidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), bandidx++) {
			if (bandidx < DBDC_BAND_NUM) {
				distance = (UINT16)os_str_tol(macptr, 0, 10);
				pAd->CommonCfg.distance[bandidx] = distance;
			}
		}
	}

	return;
}

#endif/*ACK_CTS_TIMEOUT_SUPPORT*/

#ifdef MLME_MULTI_QUEUE_SUPPORT
void rtmp_read_mlme_multiqueue_parms_from_file(
		IN RTMP_ADAPTER *pAd,
		IN RTMP_STRING *tmpbuf,
		IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr = NULL;
	UCHAR idx = 0;
	UCHAR ration_val;
	pAd->Mlme.MultiQEnable = FALSE;
	pAd->Mlme.HPQueue.Ration = RATION_OF_MLME_HP_QUEUE;
	pAd->Mlme.Queue.Ration = RATION_OF_MLME_QUEUE;
	pAd->Mlme.LPQueue.Ration = RATION_OF_MLME_LP_QUEUE;

	if (RTMPGetKeyParameter("MlmeMultiQEnable", tmpbuf, 128, pBuffer, TRUE)) {
		if ((UCHAR) simple_strtol(tmpbuf, 0, 10) != 0)
			pAd->Mlme.MultiQEnable = TRUE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Mlme.MultiQEnable=%d\n", pAd->Mlme.MultiQEnable);
	}

	if (RTMPGetKeyParameter("MlmeMultiQCtrl", tmpbuf, 128, pBuffer, TRUE)) {
		for (idx = 0, macptr = rstrtok(tmpbuf, "-"); macptr; macptr = rstrtok(NULL, "-"), idx++) {
			ration_val = (UINT16)os_str_tol(macptr, 0, 10);
			switch(idx) {
				case 0:
					pAd->Mlme.HPQueue.Ration = ration_val;
					break;
				case 1:
					pAd->Mlme.Queue.Ration = ration_val;
					break;
				case 2:
					pAd->Mlme.LPQueue.Ration = ration_val;
					break;
				default:
					break;

			}
		}
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[hp_q_ration]-[np_q_ration]-[lp_q_ration] = %d-%d-%d\n",
			pAd->Mlme.HPQueue.Ration, pAd->Mlme.Queue.Ration, pAd->Mlme.LPQueue.Ration);
	}
}
#endif

void rtmp_read_quick_channel_switch_parms_from_file(
		IN RTMP_ADAPTER *pAd,
		IN RTMP_STRING *tmpbuf,
		IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr = NULL;
	UINT16 tmp_value = 0;
	INT i = 0;
	struct wifi_dev *wdev = NULL;

	if (RTMPGetKeyParameter("QuickChannelSwitch", tmpbuf, 256, pBuffer, FALSE)) {
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), i++) {
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;

					if (macptr)
						tmp_value = (UINT16)os_str_tol(macptr, 0, 10);

					if (tmp_value > QUICK_CH_SWICH_ENABLE)
						tmp_value = QUICK_CH_SWICH_ENABLE;

					wdev->quick_ch_change = tmp_value;
				}
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), i++) {
					wdev = &pAd->StaCfg[i].wdev;

					if (macptr)
						tmp_value = (UINT16)os_str_tol(macptr, 0, 10);

					if (tmp_value > QUICK_CH_SWICH_ENABLE)
						tmp_value = QUICK_CH_SWICH_ENABLE;

					wdev->quick_ch_change = tmp_value;
				}
			}
#endif /* CONFIG_STA_SUPPORT */
	} else {
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
					wdev->quick_ch_change = QUICK_CH_SWICH_ENABLE; /* Enable quick_ch_change by default */
				}
			}
#endif /* CONFIG_AP_SUPPORT */
	}
}

#ifdef CONFIG_CPE_SUPPORT
void rtmp_read_powerBackoff_parms_from_file(
		IN RTMP_ADAPTER *pAd,
		IN RTMP_STRING *tmpbuf,
		IN RTMP_STRING *pBuffer)
{
	UCHAR bandidx = 0;
	RTMP_STRING *macptr = NULL;

	if (RTMPGetKeyParameter("AmplifierPowerBackoff", tmpbuf, 32, pBuffer, TRUE)) {
		for (bandidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), bandidx++) {
			if (bandidx < DBDC_BAND_NUM)
				pAd->CommonCfg.powerBackoff[bandidx] = (INT8)os_str_tol(macptr, 0, 10);
		}
	}
}
#endif

#ifdef CONFIG_3_WIRE_SUPPORT
void read_3wireFunction_enable(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *pBuffer)
{
	if (RTMPGetKeyParameter("ThreeWireFunctionEnable", tmpbuf, 10, pBuffer, TRUE)) {
		pAd->CommonCfg.ThreeWireFunctionEnable = (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ThreeWireFunctionEnable=%d\n", pAd->CommonCfg.ThreeWireFunctionEnable);
	}
}
#endif

#ifdef WIFI_MD_COEX_SUPPORT
static void rtmp_read_coex_param_from_file(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	MTWF_DBG(pAd, DBG_CAT_COEX, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "\n");

	if (RTMPGetKeyParameter("IdcState", tmpbuf, 32, buffer, TRUE)) {
		UINT_32 idc_state = os_str_tol(tmpbuf, 0, 10);

		if (idc_state != 0 && idc_state != 1) {
			MTWF_DBG(pAd, DBG_CAT_COEX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "input parameter[IdcState=%d] incorrect!\n", idc_state);
			return;
		}

		pAd->idcState = idc_state;
		MTWF_DBG(pAd, DBG_CAT_COEX, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "IDC state set to %s.\n", pAd->idcState?"Enable":"Disable");
	}
}
#endif

void RTMPSetProfileEDCCAThresholdParameters(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *macptr,
	UINT		 i)
{
	CHAR *parser = 0;
	UINT j;
	u_long tmp_thresh = 0;
	int ret = 0;

#ifdef DEFAULT_5G_PROFILE
	for (j = 0, parser = strsep((char **)&macptr, ":"); (j < 3); parser = strsep((char **)&macptr, ":"), j++) {
		if (parser != NULL) {
			ret = kstrtol(parser, 10, &tmp_thresh);
			if (ret == 0)
				pAd->CommonCfg.u1EDCCAThreshold[1-i][j] = ret;
			else
				pAd->CommonCfg.u1EDCCAThreshold[1-i][j] = 0;
			if (pAd->CommonCfg.u1EDCCAThreshold[1-i][j] > 0 && pAd->CommonCfg.u1EDCCAThreshold[1-i][j] < 0x7f) {
				pAd->CommonCfg.u1EDCCAThreshold[1-i][j] = ~pAd->CommonCfg.u1EDCCAThreshold[1-i][j] + 1;
				pAd->CommonCfg.isCust[1-i] = 1;
			}
		} else
			pAd->CommonCfg.u1EDCCAThreshold[1-i][j] = 0x7f;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			" EDCCAthreshold band[%d]= %d by profile\n", i, pAd->CommonCfg.u1EDCCAThreshold[1-i][j]);
	}
#else
	for (j = 0, parser = strsep((char **)&macptr, ":"); (j < 3); parser = strsep((char **)&macptr, ":"), j++) {
		if (parser != NULL) {
			ret = kstrtol(parser, 10, &tmp_thresh);
			if (ret == 0)
				pAd->CommonCfg.u1EDCCAThreshold[i][j] = tmp_thresh;
			else
				pAd->CommonCfg.u1EDCCAThreshold[i][j] = 0;
			if (pAd->CommonCfg.u1EDCCAThreshold[i][j] > 0 && pAd->CommonCfg.u1EDCCAThreshold[i][j] < 0x7f) {
				pAd->CommonCfg.u1EDCCAThreshold[i][j] = ~pAd->CommonCfg.u1EDCCAThreshold[i][j] + 1;
				pAd->CommonCfg.isCust[i] = 1;
			} else
				pAd->CommonCfg.u1EDCCAThreshold[i][j] = 0x7f;
		} else
			pAd->CommonCfg.u1EDCCAThreshold[i][j] = 0x7f;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			" EDCCAthreshold band[%d]= %d by profile\n", i, pAd->CommonCfg.u1EDCCAThreshold[i][j]);
	}
#endif

}

void RTMPSetProfileEDCCACompensationParameters(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *macptr,
	UINT		 i)
{
	CHAR *parser = 0;
	UINT j;
	u_long tmp_thresh = 0;
	int ret = 0;

#ifdef DEFAULT_5G_PROFILE
	for (j = 0, parser = strsep((char **)&macptr, ":"); (j < 4); parser = strsep((char **)&macptr, ":"), j++) {
		if (parser != NULL) {
			ret = kstrtol(parser, 10, &tmp_thresh);
			if (ret == 0)
				pAd->CommonCfg.u1EDCCACBPCpst[1-i][j] = tmp_thresh;
			else
				pAd->CommonCfg.u1EDCCACBPCpst[1-i][j] = 0;
		} else
			pAd->CommonCfg.u1EDCCACBPCpst[1-i][j] = 0;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			" CBP_COMPENSATION band[%d]= %d by profile\n", i, pAd->CommonCfg.u1EDCCACBPCpst[1-i][j]);
	}
#else
	for (j = 0, parser = strsep((char **)&macptr, ":"); (j < 4); parser = strsep((char **)&macptr, ":"), j++) {
		if (parser != NULL) {
			ret = kstrtol(parser, 10, &tmp_thresh);
			if (ret == 0)
				pAd->CommonCfg.u1EDCCACBPCpst[i][j] = tmp_thresh;
			else
				pAd->CommonCfg.u1EDCCACBPCpst[i][j] = 0;
		} else
			pAd->CommonCfg.u1EDCCACBPCpst[i][j] = 0;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			" CBP_COMPENSATION band[%d]= %d by profile\n", i, pAd->CommonCfg.u1EDCCACBPCpst[i][j]);
	}
#endif

}

NDIS_STATUS	RTMPSetProfileParameters(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *tmpbuf;
	RTMP_STRING *macptr = NULL;
	INT		i = 0, retval;
	CHAR	*value = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef TX_POWER_CONTROL_SUPPORT
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	INT		j = 0;
	CHAR	*value2 = 0;
	UINT8	ucPwrBoostReg[2] = {0};
#endif
#ifdef CONFIG_AP_SUPPORT
	RTMP_STRING tok_str[16];
	UCHAR BssidCountSupposed = 0;
	BOOLEAN bSSIDxIsUsed = FALSE;
#endif

	os_alloc_mem(NULL, (UCHAR **)&tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (tmpbuf == NULL)
		return NDIS_STATUS_FAILURE;

	/* If profile parameter is set, channel lists of HW bands need to be reset*/
	hc_init_ChCtrl(pAd);

#ifdef CONFIG_AP_SUPPORT
	/* If profile parameter is set, ACS parameters of HW bands need to be reset*/
	hc_init_ACSChCtrl(pAd);
#endif

	do {
		/* wifi certification */
		if (RTMPGetKeyParameter("WifiCert", tmpbuf, 25, pBuffer, TRUE)) {
			UCHAR wifi_cert = os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.wifi_cert = wifi_cert;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WifiCert=%d\n", wifi_cert);
		}

#ifdef WTBL_TDD_SUPPORT
		if (RTMPGetKeyParameter("WtblTdd", tmpbuf, 25, pBuffer, TRUE)) {
			UCHAR WtblTdd = os_str_tol(tmpbuf, 0, 10);
			pAd->wtblTddInfo.enabled = (WtblTdd ? TRUE : FALSE);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WtblTdd=%d\n", WtblTdd);
		}
#endif /* WTBL_TDD_SUPPORT */

#ifdef SW_CONNECT_SUPPORT
		if (RTMPGetKeyParameter("SwStaEnable", tmpbuf, 25, pBuffer, TRUE)) {
			UCHAR On = os_str_tol(tmpbuf, 0, 10);
			pAd->bSw_sta = On;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "SwStaEnable=%d\n", On);
		}
#endif /* SW_CONNECT_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef MBSS_SUPPORT

			/*BSSIDNum; This must read first of other multiSSID field, so list this field first in configuration file*/
			if (RTMPGetKeyParameter("BssidNum", tmpbuf, 25, pBuffer, TRUE)) {
				pAd->ApCfg.BssidNum = (UCHAR) os_str_tol(tmpbuf, 0, 10);
				if (pAd->ApCfg.BssidNum > MAX_MBSSID_NUM(pAd)) {
					pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "BssidNum=%d(MAX_MBSSID_NUM is %d)\n",
							  pAd->ApCfg.BssidNum, MAX_MBSSID_NUM(pAd));
				} else
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BssidNum=%d\n", pAd->ApCfg.BssidNum);
			}

			if (pAd->FlgMbssInit == FALSE) {
				/* assign default value */
				for (i = 0; i < pAd->ApCfg.BssidNum; i++)
					pAd->ApCfg.Pf2MbssIdxMap[i] = i;
			} else {
				UINT8 DbdcIdx;
				UINT8 PrevBssNumAll = 0;
				UINT8 PrevBssNum[DBDC_BAND_NUM] = {0}, CurrBssNum[DBDC_BAND_NUM] = {0};
				UINT8 BssNumDelta[DBDC_BAND_NUM] = {0};

				/* current bss count */
#ifdef MULTI_PROFILE
				if (is_multi_profile_enable(pAd)) {
					CurrBssNum[DBDC_BAND0] = multi_profile_get_pf1_num(pAd);
					CurrBssNum[DBDC_BAND1] = multi_profile_get_pf2_num(pAd);
				} else
#endif /* MULTI_PROFILE */
					CurrBssNum[DBDC_BAND0] = pAd->ApCfg.BssidNum;

				/* previous bss count */
				for (DbdcIdx = DBDC_BAND0; DbdcIdx < DBDC_BAND_NUM; DbdcIdx++) {
					PrevBssNumAll += pAd->ApCfg.BssidNumPerBand[DbdcIdx];
					PrevBssNum[DbdcIdx] = pAd->ApCfg.BssidNumPerBand[DbdcIdx];
					BssNumDelta[DbdcIdx] = CurrBssNum[DbdcIdx] - PrevBssNum[DbdcIdx];

					if (CurrBssNum[DbdcIdx] != PrevBssNum[DbdcIdx]) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								 "B%d BssidNum change: %d->%d\n",
								 DbdcIdx, PrevBssNum[DbdcIdx], CurrBssNum[DbdcIdx]);
					}
				}

				/* BssidNum change, build new profile_idx to mbss_idx mapping table */
				if ((pAd->ApCfg.BssidNum > PrevBssNumAll) && pAd->CommonCfg.wifi_cert) {
					struct wifi_dev *wdev;
					UCHAR TempVal;

					/*
					 * #1: re-assign index to existed BSS
					 *       in reverse order to prevent index be over-write
					 *       ex, BssidNum (4+8) -> (8+8)
					 */
					for (i = (PrevBssNumAll - 1); i > 0; i--) {
						wdev = &pAd->ApCfg.MBSSID[i].wdev;
						if (wdev->if_dev != NULL) {
							if (i >= PrevBssNum[DBDC_BAND0]) {
								/* Band0 BSS idx no change, extend/exchange Band1 BSS idx */
								TempVal = pAd->ApCfg.Pf2MbssIdxMap[i];
								pAd->ApCfg.Pf2MbssIdxMap[i] = pAd->ApCfg.Pf2MbssIdxMap[i + BssNumDelta[DBDC_BAND0]];
								pAd->ApCfg.Pf2MbssIdxMap[i + BssNumDelta[DBDC_BAND0]] = TempVal;
							}
						}
					}

					/* #2: fill index to new created BSS */
					for (i = FIRST_MBSSID; i < pAd->ApCfg.BssidNum; i++) {
						if (pAd->ApCfg.Pf2MbssIdxMap[i] == 0)
							pAd->ApCfg.Pf2MbssIdxMap[i] = (PrevBssNumAll++);
					}
				}
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Pf2MbssIdxMap:\n");
			for (i = 0; i < pAd->ApCfg.BssidNum; i++)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%2d ", pAd->ApCfg.Pf2MbssIdxMap[i]);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

#else
			pAd->ApCfg.BssidNum = 1;
#endif /* MBSS_SUPPORT */
		}

		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			UCHAR txcmd_mode;
			if (RTMPGetKeyParameter("TxCmdMode", tmpbuf, 25, pBuffer, TRUE)) {
				txcmd_mode = (UCHAR) os_str_tol(tmpbuf, 0, 10);
				hc_set_txcmd_mode(pAd->hdev_ctrl, txcmd_mode);
			}
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef ICAP_FW_SUPPORT
		if (RTMPGetKeyParameter("IsICAPFW", tmpbuf, 25, pBuffer, TRUE)) {
			UCHAR IcapFW = 0;

			IcapFW = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			if (IcapFW) {
				cap->wtbl_max_entries = 30;
				}
		}
#endif /* ICAP_FW_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (RTMPGetKeyParameter("MStaNum", tmpbuf, 25, pBuffer, TRUE)) {
				pAd->MaxMSTANum = (UCHAR) os_str_tol(tmpbuf, 0, 10);

				if (pAd->MaxMSTANum > MAX_MULTI_STA) {
					pAd->MaxMSTANum = MAX_MULTI_STA;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "MStaNum=%d(MAX_MULTI_STA is %d)\n",
							  pAd->MaxMSTANum, MAX_MULTI_STA);
				} else
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MStaNum=%d\n", pAd->MaxMSTANum);
			}
		}
#endif

		/* set file parameter to portcfg*/
		if (RTMPGetKeyParameter("MacAddress", tmpbuf, 25, pBuffer, TRUE)) {
			retval = RT_CfgSetMacAddress(pAd, tmpbuf, 0, OPMODE_AP);

			if (retval)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MacAddress = "MACSTR"\n",
						 MAC2STR(pAd->CurrentAddress));
		}

#ifdef MT_MAC
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef MBSS_SUPPORT

			/* for MT7615, we could assign extend BSSID mac address by ourself. */
			/* extend index starts from 1.*/
			for (i = 1; i < pAd->ApCfg.BssidNum; i++) {
				snprintf(tok_str, sizeof(tok_str), "MacAddress%d", i);

				if (RTMPGetKeyParameter(tok_str, tmpbuf, 25, pBuffer, TRUE)) {
					retval = RT_CfgSetMacAddress(pAd, tmpbuf, i, OPMODE_AP);

					if (retval)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MacAddress%d = "MACSTR"\n",
								 i, MAC2STR(pAd->ExtendMBssAddr[i]));
				}
			}

#endif /* MBSS_SUPPORT */

#ifdef CONFIG_APSTA_MIXED_SUPPORT
			if (RTMPGetKeyParameter("ApcliMacAddress", tmpbuf, 25, pBuffer, TRUE)) {
				retval = RT_CfgSetMacAddress(pAd, tmpbuf, 0, OPMODE_STA);

				if (retval)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ApcliMacAddress = "MACSTR"\n",
							 MAC2STR(pAd->ApcliAddr[0]));
			}
			if (RTMPGetKeyParameter("ApcliMacAddress1", tmpbuf, 25, pBuffer, TRUE)) {
				retval = RT_CfgSetMacAddress(pAd, tmpbuf, 1, OPMODE_STA);

				if (retval)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ApcliMacAddress1 = "MACSTR"\n",
							 MAC2STR(pAd->ApcliAddr[1]));
			}
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

		}
#endif /*CONFIG_AP_SUPPORT*/
#endif /*MT_MAC*/

		/*CountryRegion*/
		if (RTMPGetKeyParameter("CountryRegion", tmpbuf, 25, pBuffer, TRUE)) {
			retval = RT_CfgSetCountryRegion(pAd, tmpbuf, BAND_24G);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CountryRegion=%d\n", pAd->CommonCfg.CountryRegion);
		}

		/*CountryRegionABand*/
		if (RTMPGetKeyParameter("CountryRegionABand", tmpbuf, 25, pBuffer, TRUE)) {
			retval = RT_CfgSetCountryRegion(pAd, tmpbuf, BAND_5G);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CountryRegionABand=%d\n", pAd->CommonCfg.CountryRegionForABand);
		}

#ifdef BB_SOC
#ifdef RTMP_EFUSE_SUPPORT

		/*EfuseBufferMode*/
		if (RTMPGetKeyParameter("EfuseBufferMode", tmpbuf, 25, pBuffer, TRUE)) {
			pAd->E2pAccessMode = ((UCHAR) os_str_tol(tmpbuf, 0, 10) == 1) ? 4 : (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "EfuseBufferMode=%d\n", pAd->E2pAccessMode);
		}

#endif /* RTMP_EFUSE_SUPPORT */
#endif /* BB_SOC */

		/* E2pAccessMode */
		if (RTMPGetKeyParameter("E2pAccessMode", tmpbuf, 25, pBuffer, TRUE)) {
			pAd->E2pAccessMode = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "E2pAccessMode=%d\n", pAd->E2pAccessMode);
		}

#ifdef CAL_FREE_IC_SUPPORT

		/* DisableCalFree */
		if (RTMPGetKeyParameter("DisableCalFree", tmpbuf, 25, pBuffer, TRUE)) {
			UCHAR DisableCalFree = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

			if (DisableCalFree)
				ops->is_cal_free_ic = NULL;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DisableCalFree=%d\n", DisableCalFree);
		}

#endif /* DisableCalFree */

		/*CountryCode*/
		if (pAd->CommonCfg.bCountryFlag == 0) {
			if (RTMPGetKeyParameter("CountryCode", tmpbuf, 25, pBuffer, TRUE))
				RTMPSetCountryCode(pAd, tmpbuf);
		}

#ifdef EXT_BUILD_CHANNEL_LIST

		/*ChannelGeography*/
		if (RTMPGetKeyParameter("ChannelGeography", tmpbuf, 25, pBuffer, TRUE)) {
			UCHAR Geography = (UCHAR) os_str_tol(tmpbuf, 0, 10);

			if (Geography <= BOTH) {
				pAd->CommonCfg.Geography = Geography;
				pAd->CommonCfg.CountryCode[2] =
					(pAd->CommonCfg.Geography == BOTH) ? ' ' : ((pAd->CommonCfg.Geography == IDOR) ? 'I' : 'O');
#ifdef CONFIG_STA_SUPPORT
#ifdef EXT_BUILD_CHANNEL_LIST
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				pAd->StaCfg[0].StaOriGeography = pAd->CommonCfg.Geography;
#endif /* EXT_BUILD_CHANNEL_LIST */
#endif /* CONFIG_STA_SUPPORT */
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ChannelGeography=%d\n", pAd->CommonCfg.Geography);
			}
		} else {
			pAd->CommonCfg.Geography = BOTH;
			pAd->CommonCfg.CountryCode[2] = ' ';
		}

#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/* SSID*/
			if (TRUE) {
				/* PRINT(DBG_LVL_INFO, "pAd->ApCfg.BssidNum=%d\n", pAd->ApCfg.BssidNum); */
				for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					snprintf(tok_str, sizeof(tok_str), "SSID%d", i + 1);
					if (RTMPGetKeyParameter(tok_str, tmpbuf, 33, pBuffer, FALSE)) {
						NdisMoveMemory(pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].Ssid, tmpbuf, strlen(tmpbuf));
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].Ssid[strlen(tmpbuf)] = '\0';
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].SsidLen = strlen((RTMP_STRING *) pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].Ssid);

						if (bSSIDxIsUsed == FALSE)
							bSSIDxIsUsed = TRUE;
#ifdef MASK_PARTIAL_MACADDR  /* Sensitive log */
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SSID[%d]=***, EdcaIdx=%d\n", i,
								 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.EdcaIdx);
#else
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SSID[%d]=%s, EdcaIdx=%d\n", i, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].Ssid,
								 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.EdcaIdx);
#endif
					}
				}

				if (bSSIDxIsUsed == FALSE) {
					if (RTMPGetKeyParameter("SSID", tmpbuf, 256, pBuffer, FALSE)) {
						BssidCountSupposed = delimitcnt(tmpbuf, ";") + 1;

						if (pAd->ApCfg.BssidNum != BssidCountSupposed)
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Your no. of SSIDs( = %d) does not match your BssidNum( = %d)!\n",
									BssidCountSupposed, pAd->ApCfg.BssidNum);

						if (pAd->ApCfg.BssidNum > 1) {
							/* Anyway, we still do the legacy dissection of the whole SSID string.*/
							for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
								int apidx = 0;

								if (i < pAd->ApCfg.BssidNum)
									apidx = i;
								else
									break;

								NdisMoveMemory(pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].Ssid, macptr, strlen(macptr));
								pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].Ssid[strlen(macptr)] = '\0';
								pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].SsidLen = strlen((RTMP_STRING *)pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].Ssid);
								MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SSID[%d]=%s\n", i, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].Ssid);
							}
						} else {
							if ((strlen(tmpbuf) > 0) && (strlen(tmpbuf) <= 32)) {
								NdisMoveMemory(pAd->ApCfg.MBSSID[BSS0].Ssid, tmpbuf, strlen(tmpbuf));
								pAd->ApCfg.MBSSID[BSS0].Ssid[strlen(tmpbuf)] = '\0';
								pAd->ApCfg.MBSSID[BSS0].SsidLen = strlen((RTMP_STRING *) pAd->ApCfg.MBSSID[BSS0].Ssid);
								MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SSID=%s\n", pAd->ApCfg.MBSSID[BSS0].Ssid);
							}
						}
					}
				}

				if (RTMPGetKeyParameter("EdcaIdx", tmpbuf, 256, pBuffer, FALSE)) {
					UCHAR edca_idx = 0;

					for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
						if (i < pAd->ApCfg.BssidNum) {
							edca_idx = os_str_tol(macptr, 0, 10);
							pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.EdcaIdx = edca_idx;
						}
					}
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			/*SSID*/
			if (RTMPGetKeyParameter("SSID", tmpbuf, 256, pBuffer, FALSE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (strlen(macptr) <= 32) {
						RTMPSetSTASSID(pAd, &pAd->StaCfg[i].wdev, macptr);
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SSID=%s)\n", macptr);
					}
				}
			}
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			/*NetworkType*/
			if (RTMPGetKeyParameter("NetworkType", tmpbuf, 25, pBuffer, TRUE)) {
				int ii;

				if (strcmp(tmpbuf, "Adhoc") == 0)
					pAd->StaCfg[0].BssType = BSS_ADHOC;
				else { /*Default Infrastructure mode*/
					for (ii = 0; ii < MAX_MULTI_STA; ii++)
						pAd->StaCfg[ii].BssType = BSS_INFRA;
				}

				/* Reset Ralink supplicant to not use, it will be set to start when UI set PMK key*/
				for (ii = 0; ii < MAX_MULTI_STA; ii++) {
					pAd->StaCfg[ii].WpaState = SS_NOTUSE;
					pAd->StaCfg[ii].bConfigChanged = TRUE;
				}

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(NetworkType=%d)\n", pAd->StaCfg[0].BssType);
			}
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef DBDC_MODE

		/*Note: must be put before WirelessMode/Channel for check phy mode*/
		if (RTMPGetKeyParameter("DBDC_MODE", tmpbuf, 25, pBuffer, TRUE)) {
			ULONG dbdc_mode = os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.dbdc_mode = dbdc_mode > 0 ? TRUE : FALSE;
			pAd->CommonCfg.eDBDC_mode = dbdc_mode;

			/*
				TODO
				For DBDC mode, currently cannot use this wf_fwd function!
			*/
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "DBDC Mode=%d, eDBDC_mode = %d\n",
					  pAd->CommonCfg.dbdc_mode,
					  pAd->CommonCfg.eDBDC_mode);
		}

		if (IS_MT7626(pAd)) {
			pAd->CommonCfg.dbdc_mode = 1;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "MT7626 only support DBDC Mode=%d\n",
					  pAd->CommonCfg.dbdc_mode);
		}
#endif /* DBDC_MODE */

#ifdef MT_DFS_SUPPORT

		if (RTMPGetKeyParameter("DfsCalibration", tmpbuf, 25, pBuffer, TRUE)) {
			UINT_32 DisableDfsCal = os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.DfsParameter.DisableDfsCal = DisableDfsCal;
		}

		if (RTMPGetKeyParameter("DfsEnable", tmpbuf, 25, pBuffer, TRUE)) {
			UINT_32 DfsEnable = os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.DfsParameter.bDfsEnable = DfsEnable;
		}
#ifdef CONFIG_RCSA_SUPPORT
		if (RTMPGetKeyParameter("DfsUseCsaCfg", tmpbuf, 25, pBuffer, TRUE)) {
			UINT_8 UseCsaCfg = os_str_tol(tmpbuf, 0, 10);

			if (UseCsaCfg > 0)
				pAd->CommonCfg.DfsParameter.fUseCsaCfg = TRUE;
			else
				pAd->CommonCfg.DfsParameter.fUseCsaCfg = FALSE;
		}
		if (RTMPGetKeyParameter("DfsRCSAEn", tmpbuf, 25, pBuffer, TRUE)) {
			UINT_8 RCSAEn = os_str_tol(tmpbuf, 0, 10);

			if (RCSAEn > 0)
				pAd->CommonCfg.DfsParameter.bRCSAEn = TRUE;
			else
				pAd->CommonCfg.DfsParameter.bRCSAEn = FALSE;
		}
#endif
#endif

		/*WirelessMode*/
		/*Note: BssidNum must be put before WirelessMode in dat file*/
		if (RTMPGetKeyParameter("WirelessMode", tmpbuf, 128, pBuffer, TRUE))
			RTMPWirelessModeCfg(pAd, tmpbuf);

#ifdef CONFIG_AP_SUPPORT
		/*EDCCA_Mode*/
		if (RTMPGetKeyParameter("EDCCA_Mode", tmpbuf, 10, pBuffer, TRUE))
			edcca_mode_set_cfg(pAd, tmpbuf);

		/*AutoChannelSelect*/
		if (RTMPGetKeyParameter("AutoChannelSelect", tmpbuf, 10, pBuffer, TRUE))
			auto_ch_select_set_cfg(pAd, tmpbuf);
#ifdef CONFIG_6G_SUPPORT
		if (RTMPGetKeyParameter("PSC_ACS", tmpbuf, 10, pBuffer, TRUE))
			auto_ch_select_PSC_cfg(pAd, tmpbuf);
#endif

#endif/* CONFIG_AP_SUPPORT */

		/* Channel Group */
		if (RTMPGetKeyParameter("ChannelGrp", tmpbuf, 25, pBuffer, TRUE))
			MTSetChGrp(pAd, tmpbuf);
		/*SeamlessCSA*/
		if (RTMPGetKeyParameter("SeamlessCSA", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) != 0)  /*Enable*/
				pAd->CommonCfg.CSASupportFor2G = TRUE;
			else /*Disable*/
				pAd->CommonCfg.CSASupportFor2G = FALSE;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"SeamlessCSA=%d\n", pAd->CommonCfg.CSASupportFor2G);
		} else
			pAd->CommonCfg.CSASupportFor2G = FALSE;

		/*Channel*/
		if (RTMPGetKeyParameter("Channel", tmpbuf, 128, pBuffer, TRUE))
			RTMPChannelCfg(pAd, tmpbuf);

		/* EtherTrafficBand */
		if (RTMPGetKeyParameter("EtherTrafficBand", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->CommonCfg.EtherTrafficBand = (UCHAR) os_str_tol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "EtherTrafficBand=%d\n", pAd->CommonCfg.EtherTrafficBand);

			if (pAd->CommonCfg.EtherTrafficBand > EtherTrafficBand5G)
				pAd->CommonCfg.EtherTrafficBand = EtherTrafficBand5G;
		}

		/* Wf_fwd_ */
		if (RTMPGetKeyParameter("WfFwdDisabled", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->CommonCfg.WfFwdDisabled = os_str_tol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WfFwdDisabled=%d\n", pAd->CommonCfg.WfFwdDisabled);
		}

		/*BasicRate*/
		if (RTMPGetKeyParameter("BasicRate", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->CommonCfg.BasicRateBitmap = (ULONG) os_str_tol(tmpbuf, 0, 10);
			pAd->CommonCfg.BasicRateBitmapOld = (ULONG) os_str_tol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BasicRate=%ld\n", pAd->CommonCfg.BasicRateBitmap);
		}

#ifdef GN_MIXMODE_SUPPORT
		/*GNMixMode*/
		if (RTMPGetKeyParameter("GNMixMode", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->CommonCfg.GNMixMode = (ULONG) os_str_tol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"GNMixMode=%d\n", pAd->CommonCfg.GNMixMode);
		}
#endif /*GN_MIXMODE_SUPPORT*/

#ifdef DBDC_ONE_BAND_SUPPORT
		/*DbdcBandSupport::0:both band, 1:2.4G, 2:5G*/
		if (RTMPGetKeyParameter("DbdcBandSupport", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->CommonCfg.DbdcBandSupport = (ULONG) os_str_tol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"DbdcBandSupport=%d\n", pAd->CommonCfg.DbdcBandSupport);
		}
#endif /*DBDC_ONE_BAND_SUPPORT*/

		/*BeaconPeriod*/
		if (RTMPGetKeyParameter("BeaconPeriod", tmpbuf, PER_BSS_SIZE_5(pAd), pBuffer, TRUE)) {
			UCHAR i_counter = 0;
			CHAR *remain_buf;
			RTMP_STRING *pa_buf = tmpbuf;
			USHORT bcn_val;

			do {
				if (i_counter >= DBDC_BAND_NUM)
					break;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"BeaconPeriod:tmpbuf=%s\n", pa_buf);
				bcn_val = (USHORT) os_str_tol(pa_buf, (CHAR **)&remain_buf, 10);
				/* The acceptable is 20~1000 ms. Refer to WiFi test plan. */
				if (bcn_val >= 20 && bcn_val <= 1000)
					pAd->CommonCfg.BeaconPeriod[i_counter] = bcn_val;
				else
					pAd->CommonCfg.BeaconPeriod[i_counter] = 100;	/* Default value*/
				pa_buf = &remain_buf[1];
				i_counter++;
			} while (remain_buf[0] == ';');
#ifdef APCLI_CONNECTION_TRIAL
			for (i_counter = 0; i_counter < DBDC_BAND_NUM; i_counter++)
				pAd->CommonCfg.BeaconPeriod[i_counter] = 200;
#endif /* APCLI_CONNECTION_TRIAL */
			for (i_counter = 0; i_counter < DBDC_BAND_NUM; i_counter++)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"BeaconPeriod[%d]=%d\n", i_counter, pAd->CommonCfg.BeaconPeriod[i_counter]);
		}

#ifdef RTMP_RBUS_SUPPORT

		/*FreqOffsetDelta*/
		if (pAd->infType == RTMP_DEV_INF_RBUS) {
			if (RTMPGetKeyParameter("FreqDelta", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->RfFreqDelta = (USHORT) os_str_tol(tmpbuf, 0, 10);

				if (pAd->RfFreqDelta > 0x20)
					pAd->RfFreqDelta = 0;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "FreqDelta=%d\n", pAd->RfFreqDelta);
			}
		}

#endif /* RTMP_RBUS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef CONFIG_DOT11V_WNM
			WNM_ReadParametersFromFile(pAd, tmpbuf, pBuffer);
#endif

			/*DtimPeriod*/
			if (RTMPGetKeyParameter("DtimPeriod", tmpbuf, 128, pBuffer, TRUE)) {
				/* apply per-BSSs setting */
				RTMPDtimPeriodCfg(pAd, tmpbuf);
			}

#ifdef BAND_STEERING
			/* Read BandSteering profile parameters */
			BndStrgSetProfileParam(pAd, tmpbuf, pBuffer);
#endif /* BAND_STEERING */
#ifdef MBSS_AS_WDS_AP_SUPPORT
			if (RTMPGetKeyParameter("WDSEnable", tmpbuf, 50, pBuffer, TRUE)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_CAT_AP, DBG_LVL_INFO, "WDS=%s\n", tmpbuf);
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					pAd->ApCfg.MBSSID[i].wdev.wds_enable = simple_strtoul(macptr, 0, 10);
				}
			}

			if (RTMPGetKeyParameter("WdsMac", tmpbuf, 50, pBuffer, TRUE)) {
				/*Mac address acceptable format 01:02:03:04:05:06 length 17 */
				if (strlen(tmpbuf) != 17) {
					for (i = 0, value = rstrtok(tmpbuf, ":"); value; value = rstrtok(NULL, ":")) {
						if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
							return FALSE;  /*Invalid */
					AtoH(value, (UCHAR *)&pAd->ApCfg.wds_mac[i++], 1);
						}
				}
			}
#endif
		}
#endif /* CONFIG_AP_SUPPORT */

		/* TxPower */
		if (RTMPGetKeyParameter("TxPower", tmpbuf, 10, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {

#ifdef DEFAULT_5G_PROFILE

					case 0:
						pAd->CommonCfg.ucTxPowerPercentage[BAND1] = simple_strtol(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.ucTxPowerPercentage[BAND0] = simple_strtol(value, 0, 10);
						break;
#else
					case 0:
						pAd->CommonCfg.ucTxPowerPercentage[BAND0] = simple_strtol(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.ucTxPowerPercentage[BAND1] = simple_strtol(value, 0, 10);
						break;
#endif /*DEFAULT_5G_PROFILE*/



					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.ucTxPowerPercentage[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				}

#else

				switch (i) {
				case 0:
					pAd->CommonCfg.ucTxPowerPercentage[BAND0] = simple_strtol(value, 0, 10);
					break;

				default:
					break;
				}

#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[TxPower] BAND0: %d, BAND1: %d\n",
						 pAd->CommonCfg.ucTxPowerPercentage[BAND0], pAd->CommonCfg.ucTxPowerPercentage[BAND1]);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[TxPower] BAND0: %d\n",
						 pAd->CommonCfg.ucTxPowerPercentage[BAND0]);

#else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[TxPower] BAND0: %d\n",
					 pAd->CommonCfg.ucTxPowerPercentage[BAND0]);
#endif /* DBDC_MODE */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				pAd->CommonCfg.ucTxPowerDefault[BAND0] = pAd->CommonCfg.ucTxPowerPercentage[BAND0];
#ifdef DBDC_MODE
				pAd->CommonCfg.ucTxPowerDefault[BAND1] = pAd->CommonCfg.ucTxPowerPercentage[BAND1];
#endif /* DBDC_MODE */
			}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				pAd->CommonCfg.ucTxPowerDefault[BAND0] = pAd->CommonCfg.ucTxPowerPercentage[BAND0];
#ifdef DBDC_MODE
				pAd->CommonCfg.ucTxPowerDefault[BAND1] = pAd->CommonCfg.ucTxPowerPercentage[BAND1];
#endif /* DBDC_MODE */
			}
#endif /* CONFIG_AP_SUPPORT */
		}

#ifdef TX_POWER_CONTROL_SUPPORT
		/* Power Boost (CCK) */
		if (RTMPGetKeyParameter("PowerUpCck", tmpbuf, 32,
					pBuffer, TRUE)) {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
					ptmpStr[i] = value;

				/* sanity check for parameter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
						DBG_LVL_ERROR,
						"[PowerUpCck] Input parameter incorrect!!\n");

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_cck); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_cck)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpCck] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND1].cPowerUpCck[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND1].cPowerUpCck[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_cck); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_cck)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpCck] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpCck[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpCck[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
			} else {
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_cck); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_cck)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpCck] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpCck[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpCck[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_cck); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_cck)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpCck] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpCck[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpCck[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
#endif /* DBDC_MODE */
			}

		/* Power Boost (OFDM) */
		if (RTMPGetKeyParameter("PowerUpOfdm", tmpbuf, 32,
					pBuffer, TRUE)) {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
					ptmpStr[i] = value;

				/* sanity check for parameter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
						DBG_LVL_ERROR,
						"[PowerUpOfdm] Input parameter incorrect!!\n");

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_ofdm); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_ofdm)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpOfdm] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND1].cPowerUpOfdm[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND1].cPowerUpOfdm[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_ofdm); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_ofdm)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpOfdm] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpOfdm[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpOfdm[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
			} else {
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_ofdm); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_ofdm)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpOfdm] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpOfdm[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpOfdm[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_ofdm); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_ofdm)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpOfdm] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpOfdm[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpOfdm[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
#endif /* DBDC_MODE */
		}

		/* Power Boost (HT20) */
		if (RTMPGetKeyParameter("PowerUpHT20", tmpbuf, 32,
					pBuffer, TRUE)) {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
					ptmpStr[i] = value;

				/* sanity check for parameter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
						DBG_LVL_ERROR,
						"[PowerUpHT20] Input parameter incorrect!!\n");

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_ht20); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_ht20)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpHT20] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND1].cPowerUpHt20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND1].cPowerUpHt20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_ht20); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_ht20)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpHT20] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpHt20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpHt20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
			} else {
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_ht20); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_ht20)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpHT20] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpHt20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpHt20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
					}
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_ht20); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_ht20)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpHT20] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpHt20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpHt20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
#endif /* DBDC_MODE */
		}

		/* Power Boost (HT40) */
		if (RTMPGetKeyParameter("PowerUpHT40", tmpbuf, 32,
					pBuffer, TRUE)) {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
					ptmpStr[i] = value;

				/* sanity check for parameter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
						DBG_LVL_ERROR,
						"[PowerUpHT40] Input parameter incorrect!!\n");

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_ht40); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_ht40)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpHT40] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND1].cPowerUpHt40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND1].cPowerUpHt40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_ht40); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_ht40)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpHT40] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpHt40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpHt40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
			} else {
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_ht40); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_ht40)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpHT40] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpHt40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpHt40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
					}
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_ht40); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_ht40)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpHT40] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpHt40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpHt40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
#endif /* DBDC_MODE */
			}

		/* Power Boost (VHT20) */
		if (RTMPGetKeyParameter("PowerUpVHT20", tmpbuf, 32,
					pBuffer, TRUE)) {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
					ptmpStr[i] = value;

				/* sanity check for parameter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
						DBG_LVL_ERROR,
						"[PowerUpVHT20] Input parameter incorrect!!\n");

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_vht20); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht20)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpVHT20] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND1].cPowerUpVht20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND1].cPowerUpVht20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_vht20); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht20)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpVHT20] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpVht20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpVht20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
			} else {
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_vht20); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht20)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpVHT20] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpVht20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpVht20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
					}
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_vht20); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht20)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpVHT20] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpVht20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpVht20[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
#endif /* DBDC_MODE */
		}

		/* Power Boost (VHT40) */
		if (RTMPGetKeyParameter("PowerUpVHT40", tmpbuf, 32,
					pBuffer, TRUE)) {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
					ptmpStr[i] = value;

				/* sanity check for parameter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
						DBG_LVL_ERROR,
						"[PowerUpVHT40] Input parameter incorrect!!\n");

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_vht40); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht40)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpVHT40] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND1].cPowerUpVht40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND1].cPowerUpVht40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_vht40); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht40)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpVHT40] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpVht40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpVht40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
			} else {
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_vht40); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht40)
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PowerUpVHT40] Input parameter incorrect!!\n"));
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpVht40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpVht40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
					}
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_vht40); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht40)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpVHT40] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpVht40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpVht40[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
#endif /* DBDC_MODE */
		}

		/* Power Boost (VHT80) */
		if (RTMPGetKeyParameter("PowerUpVHT80", tmpbuf, 32,
					pBuffer, TRUE)) {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
					ptmpStr[i] = value;

				/* sanity check for parameter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
							DBG_LVL_ERROR,
							"[PowerUpVHT80] Input parameter incorrect!!\n");

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_vht80); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht80)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpVHT80] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND1].cPowerUpVht80[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND1].cPowerUpVht80[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_vht80); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht80)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpVHT80] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpVht80[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpVht80[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
			} else {
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_vht80); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht80)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpVHT80] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpVht80[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpVht80[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
					}
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_vht80); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht80)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpVHT80] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpVht80[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpVht80[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
#endif /* DBDC_MODE */
		}

		/* Power Boost (VHT160) */
		if (RTMPGetKeyParameter("PowerUpVHT160", tmpbuf, 32,
					pBuffer, TRUE)) {
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				RTMP_STRING *ptmpStr[DBDC_BAND_NUM];

				/* parameter parsing (Phase I) */
				for (i = 0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++)
					ptmpStr[i] = value;

				/* sanity check for parameter parsing (Phase I) */
				if (i != DBDC_BAND_NUM)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL,
						DBG_LVL_ERROR,
						"[PowerUpVHT160] Input parameter incorrect!!\n");

				/* Band1 Parameter parsing (Phase II) */
				value = ptmpStr[0];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_vht160); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht160)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpVHT160] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND1].cPowerUpVht160[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND1].cPowerUpVht160[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
				/* Band0 Parameter parsing (Phase II) */
				value = ptmpStr[1];
				for (i = 0, value = rstrtok(value, "-"); (value) && (i < cap->single_sku_fill_tbl_vht160); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht160)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpVHT160] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpVht160[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpVht160[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
				}
			} else {
				/* parameter parsing */
				for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_vht160); value = rstrtok(NULL, "-"), i++) {
					for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
						ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
					if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht160)
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpVHT160] Input parameter incorrect!!\n");
					else {
						if (cap->txpower_type == TX_POWER_TYPE_V0)
							pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpVht160[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
						else if (cap->txpower_type == TX_POWER_TYPE_V1)
							pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpVht160[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					}
					}
			}
#else
			/* parameter parsing */
			for (i = 0, value = rstrtok(tmpbuf, "-"); (value) && (i < cap->single_sku_fill_tbl_vht160); value = rstrtok(NULL, "-"), i++) {
				for (j = 0, value2 = strsep((char **)&value, ":"); (value2) && (j < 2); value2 = strsep((char **)&value, ":"), j++)
					ucPwrBoostReg[j] = simple_strtol(value2, 0, 10);
				if (ucPwrBoostReg[0] >= cap->single_sku_fill_tbl_vht160)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PowerUpVHT160] Input parameter incorrect!!\n");
				else {
					if (cap->txpower_type == TX_POWER_TYPE_V0)
						pAd->CommonCfg.PowerBoostParamV0[BAND0].cPowerUpVht160[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
					else if (cap->txpower_type == TX_POWER_TYPE_V1)
						pAd->CommonCfg.PowerBoostParamV1[BAND0].cPowerUpVht160[ucPwrBoostReg[0]] = ucPwrBoostReg[1];
				}
			}
#endif /* DBDC_MODE */
		}

		if (arch_ops && arch_ops->arch_txpower_boost_profile)
			arch_ops->arch_txpower_boost_profile(pAd, tmpbuf, pBuffer);

#endif /* TX_POWER_CONTROL_SUPPORT */

#ifdef SINGLE_SKU_V2

		/* TxPower SKU */
		if (RTMPGetKeyParameter("SKUenable", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->CommonCfg.SKUenable[BAND0] = simple_strtol(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.SKUenable[BAND1] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.SKUenable[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				}

#else

				switch (i) {
				case 0:
					pAd->CommonCfg.SKUenable[BAND0] = simple_strtol(value, 0, 10);
					break;

				default:
					break;
				}

#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[SKUenable] BAND0: %d, BAND1: %d\n",
						 pAd->CommonCfg.SKUenable[BAND0], pAd->CommonCfg.SKUenable[BAND1]);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[SKUenable] BAND0: %d\n", pAd->CommonCfg.SKUenable[BAND0]);

#else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[SKUenable] BAND0: %d\n", pAd->CommonCfg.SKUenable[BAND0]);
#endif /* DBDC_MODE */
		}

#endif /*SINGLE_SKU_V2 */

		/* TxPower SKU */
		if (RTMPGetKeyParameter("SkuTableIdx", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			pAd->CommonCfg.SKUTableIdx = simple_strtol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[SkuTableIdx]: %d\n", pAd->CommonCfg.SKUTableIdx);
		}

		/* CCKTxStream */
		if (RTMPGetKeyParameter("CCKTxStream", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {

#ifdef DBDC_MODE
				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->CommonCfg.CCKTxStream[BAND0] = simple_strtol(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.CCKTxStream[BAND1] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.CCKTxStream[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				}
#else

				switch (i) {
				case 0:
					pAd->CommonCfg.CCKTxStream[BAND0] = simple_strtol(value, 0, 10);
					break;

				default:
					break;
				}
#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[CCKTxStream] BAND0: %d, BAND1: %d\n",
						 pAd->CommonCfg.CCKTxStream[BAND0], pAd->CommonCfg.CCKTxStream[BAND1]);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[CCKTxStream] BAND0: %d\n", pAd->CommonCfg.CCKTxStream[BAND0]);

#else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[CCKTxStream] BAND0: %d\n", pAd->CommonCfg.CCKTxStream[BAND0]);
#endif /* DBDC_MODE */
		}

		/* TxPower Percentage */
		if (RTMPGetKeyParameter("PERCENTAGEenable", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->CommonCfg.PERCENTAGEenable[BAND0] = simple_strtol(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.PERCENTAGEenable[BAND1] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.PERCENTAGEenable[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				}

#else

				switch (i) {
				case 0:
					pAd->CommonCfg.PERCENTAGEenable[BAND0] = simple_strtol(value, 0, 10);
					break;

				default:
					break;
				}

#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[PERCENTAGEenable] BAND0: %d, BAND1: %d\n",
						 pAd->CommonCfg.PERCENTAGEenable[BAND0], pAd->CommonCfg.PERCENTAGEenable[BAND1]);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[PERCENTAGEenable] BAND0: %d\n",
						 pAd->CommonCfg.PERCENTAGEenable[BAND0]);

#else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[PERCENTAGEenable] BAND0: %d\n",
					 pAd->CommonCfg.PERCENTAGEenable[BAND0]);
#endif /* DBDC_MODE */
		}

		/* TxPower BF Backoff */
		if (RTMPGetKeyParameter("BFBACKOFFenable", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->CommonCfg.BFBACKOFFenable[BAND0] = simple_strtol(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.BFBACKOFFenable[BAND1] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.BFBACKOFFenable[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				}

#else

				switch (i) {
				case 0:
					pAd->CommonCfg.BFBACKOFFenable[BAND0] = simple_strtol(value, 0, 10);
					break;

				default:
					break;
				}

#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[BFBACKOFFenable] BAND0: %d, BAND1: %d\n",
						 pAd->CommonCfg.BFBACKOFFenable[BAND0], pAd->CommonCfg.BFBACKOFFenable[BAND1]);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[BFBACKOFFenable] BAND0: %d\n",
						 pAd->CommonCfg.BFBACKOFFenable[BAND0]);

#else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[BFBACKOFFenable] BAND0: %d\n",
					 pAd->CommonCfg.BFBACKOFFenable[BAND0]);
#endif /* DBDC_MODE */
		}

#ifdef CFG_SUPPORT_FALCON_MURU
		if (RTMPGetKeyParameter("Disable160RuMu", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->CommonCfg.Dis160RuMu[BAND0] = simple_strtoul(value, 0, 16);
						break;

					case 1:
						pAd->CommonCfg.Dis160RuMu[BAND1] = simple_strtoul(value, 0, 16);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.Dis160RuMu[BAND0] = simple_strtoul(value, 0, 16);
						break;

					default:
						break;
					}
				}

#else

				switch (i) {
				case 0:
					pAd->CommonCfg.Dis160RuMu[BAND0] = simple_strtoul(value, 0, 16);
					break;

				default:
					break;
				}

#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[Disable160RuMu] BAND0: %d, BAND1: %d\n",
						 pAd->CommonCfg.Dis160RuMu[BAND0], pAd->CommonCfg.Dis160RuMu[BAND1]);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[Disable160RuMu] BAND0: %d\n", pAd->CommonCfg.Dis160RuMu[BAND0]);

#else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[Disable160RuMu] BAND0: %d\n", pAd->CommonCfg.Dis160RuMu[BAND0]);
#endif /* DBDC_MODE */
		}

		if (RTMPGetKeyParameter("MaxRuOfdma", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->CommonCfg.MaxRuOfdma[BAND0] = simple_strtol(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.MaxRuOfdma[BAND1] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.MaxRuOfdma[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				}

#else

				switch (i) {
				case 0:
					pAd->CommonCfg.MaxRuOfdma[BAND0] = simple_strtol(value, 0, 10);
					break;

				default:
					break;
				}

#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[MaxRuOfdma] BAND0: %d, BAND1: %d\n",
						 pAd->CommonCfg.MaxRuOfdma[BAND0], pAd->CommonCfg.MaxRuOfdma[BAND1]);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[MaxRuOfdma] BAND0: %d\n", pAd->CommonCfg.MaxRuOfdma[BAND0]);

#else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[MaxRuOfdma] BAND0: %d\n", pAd->CommonCfg.MaxRuOfdma[BAND0]);
#endif /* DBDC_MODE */
		}

		if (RTMPGetKeyParameter("MaxDLMuMimo", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->CommonCfg.MaxDLMuMimo[BAND0] = simple_strtol(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.MaxDLMuMimo[BAND1] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.MaxDLMuMimo[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				}

#else

				switch (i) {
				case 0:
					pAd->CommonCfg.MaxDLMuMimo[BAND0] = simple_strtol(value, 0, 10);
					break;

				default:
					break;
				}

#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[MaxDLMuMimo] BAND0: %d, BAND1: %d\n",
						 pAd->CommonCfg.MaxDLMuMimo[BAND0], pAd->CommonCfg.MaxDLMuMimo[BAND1]);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[MaxDLMuMimo] BAND0: %d\n", pAd->CommonCfg.MaxDLMuMimo[BAND0]);

#else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[MaxDLMuMimo] BAND0: %d\n", pAd->CommonCfg.MaxDLMuMimo[BAND0]);
#endif /* DBDC_MODE */
		}

		if (RTMPGetKeyParameter("MaxULMuMimo", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->CommonCfg.MaxULMuMimo[BAND0] = simple_strtol(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.MaxULMuMimo[BAND1] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.MaxULMuMimo[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				}

#else

				switch (i) {
				case 0:
					pAd->CommonCfg.MaxULMuMimo[BAND0] = simple_strtol(value, 0, 10);
					break;

				default:
					break;
				}

#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[MaxULMuMimo] BAND0: %d, BAND1: %d\n",
						 pAd->CommonCfg.MaxULMuMimo[BAND0], pAd->CommonCfg.MaxULMuMimo[BAND1]);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[MaxULMuMimo] BAND0: %d\n", pAd->CommonCfg.MaxULMuMimo[BAND0]);

#else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[MaxULMuMimo] BAND0: %d\n", pAd->CommonCfg.MaxULMuMimo[BAND0]);
#endif /* DBDC_MODE */
		}
#endif /* CFG_SUPPORT_FALCON_MURU */

#ifdef DYNAMIC_WMM_SUPPORT
		if (RTMPGetKeyParameter("DynWmmEnable", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->DynWmmCtrl.DynWmmEnable[BAND0] = simple_strtoul(value, 0, 10);
						break;

					case 1:
						pAd->DynWmmCtrl.DynWmmEnable[BAND1] = simple_strtoul(value, 0, 10);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->DynWmmCtrl.DynWmmEnable[BAND0] = simple_strtoul(value, 0, 10);
						break;

					default:
						break;
					}
				}

#else

				switch (i) {
				case 0:
					pAd->DynWmmCtrl.DynWmmEnable[BAND0] = simple_strtoul(value, 0, 10);
					break;

				default:
					break;
				}

#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"[DynWmmEnable] BAND0: %d, BAND1: %d\n",
					pAd->DynWmmCtrl.DynWmmEnable[BAND0],
					pAd->DynWmmCtrl.DynWmmEnable[BAND1]);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"[DynWmmEnable] BAND0: %d\n",
					pAd->DynWmmCtrl.DynWmmEnable[BAND0]);

#else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"[DynWmmEnable] BAND0: %d\n",
				pAd->DynWmmCtrl.DynWmmEnable[BAND0]);
#endif /* DBDC_MODE */
		}
#endif /* DYNAMIC_WMM_SUPPORT */

#ifdef WIFI_CSI_CN_INFO_SUPPORT
		if (RTMPGetKeyParameter("EnableCNInfo", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->CommonCfg.EnableCNInfo[BAND0] = (UINT8)simple_strtoul(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.EnableCNInfo[BAND1] = (UINT8)simple_strtoul(value, 0, 10);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.EnableCNInfo[BAND0] = (UINT8)simple_strtoul(value, 0, 10);
						break;

					default:
						break;
					}
				}

#else

				switch (i) {
				case 0:
					pAd->CommonCfg.EnableCNInfo[BAND0] = (UINT8)simple_strtoul(value, 0, 10);
					break;

				default:
					break;
				}

#endif /* DBDC_MODE */
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"[EnableCNInfo] BAND0: %d, BAND1: %d\n",
					pAd->CommonCfg.EnableCNInfo[BAND0],
					pAd->CommonCfg.EnableCNInfo[BAND1]);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"[EnableCNInfo] BAND0: %d\n",
					pAd->CommonCfg.EnableCNInfo[BAND0]);

#else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"[EnableCNInfo] BAND0: %d\n",
				pAd->CommonCfg.EnableCNInfo[BAND0]);
#endif /* DBDC_MODE */
		}
#endif /* WIFI_CSI_CN_INFO_SUPPORT */

#ifdef LINK_TEST_SUPPORT

		/* Link Test Support */
		if (RTMPGetKeyParameter("LinkTestSupport", tmpbuf, 32, pBuffer, TRUE)) {
			/* parameter parsing */
			for (i = BAND0, value = rstrtok(tmpbuf, ";"); value; value = rstrtok(NULL, ";"), i++) {
#ifdef DBDC_MODE

				if (pAd->CommonCfg.dbdc_mode) {
					switch (i) {
					case 0:
						pAd->CommonCfg.LinkTestSupportTemp[BAND0] = simple_strtol(value, 0, 10);
						break;

					case 1:
						pAd->CommonCfg.LinkTestSupportTemp[BAND1] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				} else {
					switch (i) {
					case 0:
						pAd->CommonCfg.LinkTestSupportTemp[BAND0] = simple_strtol(value, 0, 10);
						break;

					default:
						break;
					}
				}

#else

				switch (i) {
				case 0:
					pAd->CommonCfg.LinkTestSupportTemp[BAND0] = simple_strtol(value, 0, 10);
					break;

				default:
					break;
				}

#endif /* DBDC_MODE */
			}

			/* LinkTestSupport can be enabled by any profile */

			if (pAd->CommonCfg.LinkTestSupportTemp[BAND0]
#ifdef DBDC_MODE
				|| pAd->CommonCfg.LinkTestSupportTemp[BAND1]
#endif /* DBDC_MODE */
				)
				pAd->CommonCfg.LinkTestSupport = TRUE;
			else
				pAd->CommonCfg.LinkTestSupport = FALSE;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "LinkTestSupport = %d\n", pAd->CommonCfg.LinkTestSupport);
		}

#endif /* LINK_TEST_SUPPORT */
#ifdef RLM_CAL_CACHE_SUPPORT

		/* Calibration Cache Support */
		if (RTMPGetKeyParameter("CalCacheApply", tmpbuf, 32, pBuffer, TRUE)) {
			pAd->CommonCfg.CalCacheApply = (ULONG) simple_strtol(tmpbuf, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CalCacheApply = %d\n", pAd->CommonCfg.CalCacheApply);
		}

#endif /* RLM_CAL_CACHE_SUPPORT */

		/*BGProtection*/
		if (RTMPGetKeyParameter("BGProtection", tmpbuf, 10, pBuffer, TRUE)) {
			/*#if 0	#ifndef WIFI_TEST*/
			/*		pAd->CommonCfg.UseBGProtection = 2; disable b/g protection for throughput test*/
			/*#else*/
			switch (os_str_tol(tmpbuf, 0, 10)) {
			case 1: /*Always On*/
				pAd->CommonCfg.UseBGProtection = 1;
				break;

			case 2: /*Always OFF*/
				pAd->CommonCfg.UseBGProtection = 2;
				break;

			case 0: /*AUTO*/
			default:
				pAd->CommonCfg.UseBGProtection = 0;
				break;
			}

			/*#endif*/
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BGProtection=%ld\n", pAd->CommonCfg.UseBGProtection);
		}

#ifdef CONFIG_AP_SUPPORT

		/*OLBCDetection*/
		if (RTMPGetKeyParameter("DisableOLBC", tmpbuf, 10, pBuffer, TRUE)) {
			switch (os_str_tol(tmpbuf, 0, 10)) {
			case 1: /*disable OLBC Detection*/
				pAd->CommonCfg.DisableOLBCDetect = 1;
				break;

			case 0: /*enable OLBC Detection*/
				pAd->CommonCfg.DisableOLBCDetect = 0;
				break;

			default:
				pAd->CommonCfg.DisableOLBCDetect = 0;
				break;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OLBCDetection=%ld\n", pAd->CommonCfg.DisableOLBCDetect);
		}

#endif /* CONFIG_AP_SUPPORT */

		/*TxPreamble*/
		if (RTMPGetKeyParameter("TxPreamble", tmpbuf, 10, pBuffer, TRUE)) {
			switch (os_str_tol(tmpbuf, 0, 10)) {
			case Rt802_11PreambleShort:
				pAd->CommonCfg.TxPreamble = Rt802_11PreambleShort;
				break;

			case Rt802_11PreambleAuto:
				pAd->CommonCfg.TxPreamble = Rt802_11PreambleAuto;
				break;

			case Rt802_11PreambleLong:
			default:
				pAd->CommonCfg.TxPreamble = Rt802_11PreambleLong;
				break;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TxPreamble=%ld\n", pAd->CommonCfg.TxPreamble);
		}

		/*RTSPktThreshold*/
		read_rts_pkt_thld_from_file(pAd, tmpbuf, pBuffer);
		/*RTSThreshold*/
		read_rts_len_thld_from_file(pAd, tmpbuf, pBuffer);
		/*FragThreshold*/
		read_frag_thld_from_file(pAd, tmpbuf, pBuffer);

#ifdef VLAN_SUPPORT
		/*VLANTag*/
#ifdef CONFIG_AP_SUPPORT
		if (RTMPGetKeyParameter("VLANTag", tmpbuf, 10, pBuffer, TRUE)) {
			BOOLEAN bVlan_tag = FALSE;
			struct wifi_dev *wdev;
			if (simple_strtol(tmpbuf, 0, 10) != 0)
				bVlan_tag = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wdev->bVlan_tag = %d\n", bVlan_tag);
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				wdev = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev;
				wdev->bVLAN_Tag = bVlan_tag;
				if (bVlan_tag == TRUE)
					wdev->VLAN_Policy[TX_VLAN] = VLAN_TX_ALLOW;
			}
#ifdef APCLI_SUPPORT
			if (RTMPGetKeyParameter("STAVLANTag", tmpbuf, 10, pBuffer, TRUE)) {
				BOOLEAN bVlan_tag = FALSE;
				struct wifi_dev *wdev;
				if (simple_strtol(tmpbuf, 0, 10) != 0)
					bVlan_tag = TRUE;

				for (i = 0; i < MAX_APCLI_NUM; i++) {
					wdev = &pAd->StaCfg[i].wdev;
					wdev->bVLAN_Tag = bVlan_tag;
					if (bVlan_tag == TRUE)
						wdev->VLAN_Policy[TX_VLAN] = VLAN_TX_ALLOW;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"SHAILESH: APCLI%d VlanTag=%d\n", i, wdev->bVLAN_Tag);
				}
			}
#endif /*APCLI_SUPPORT*/
		}
		if (RTMPGetKeyParameter("VLANEn", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) != 0)
				pAd->CommonCfg.bEnableVlan = TRUE;
			else
				pAd->CommonCfg.bEnableVlan = FALSE;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VLANEn=%d\n", pAd->CommonCfg.bEnableVlan);
		}
#endif /*CONFIG_AP_SUPPORT*/
#endif /*VLAN_SUPPORT*/

		/*TxBurst*/
		if (RTMPGetKeyParameter("TxBurst", tmpbuf, 10, pBuffer, TRUE)) {
			/*#ifdef WIFI_TEST*/
			/*						pAd->CommonCfg.bEnableTxBurst = FALSE;*/
			/*#else*/
			if (os_str_tol(tmpbuf, 0, 10) != 0) /*Enable*/
				pAd->CommonCfg.bEnableTxBurst = TRUE;
			else /*Disable*/
				pAd->CommonCfg.bEnableTxBurst = FALSE;

			/*#endif*/
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TxBurst=%d\n", pAd->CommonCfg.bEnableTxBurst);
		}

#ifdef AGGREGATION_SUPPORT

		/*PktAggregate*/
		if (RTMPGetKeyParameter("PktAggregate", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) != 0) /*Enable*/
				pAd->CommonCfg.bAggregationCapable = TRUE;
			else /*Disable*/
				pAd->CommonCfg.bAggregationCapable = FALSE;

#ifdef PIGGYBACK_SUPPORT
			pAd->CommonCfg.bPiggyBackCapable = pAd->CommonCfg.bAggregationCapable;
#endif /* PIGGYBACK_SUPPORT */
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PktAggregate=%d\n", pAd->CommonCfg.bAggregationCapable);
		}

#else
		pAd->CommonCfg.bAggregationCapable = FALSE;
		pAd->CommonCfg.bPiggyBackCapable = FALSE;
#endif /* AGGREGATION_SUPPORT */
		/* WmmCapable*/
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		rtmp_read_ap_wmm_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			rtmp_read_sta_wmm_parms_from_file(pAd, tmpbuf, pBuffer);
#ifdef XLINK_SUPPORT
			rtmp_get_psp_xlink_mode_from_file(pAd, tmpbuf, pBuffer);
#endif /* XLINK_SUPPORT */
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/* MbssMaxStaNum*/
			if (RTMPGetKeyParameter("MbssMaxStaNum", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					ApCfg_Set_PerMbssMaxStaNum_Proc(pAd, i, macptr);
				}
			}

			/* BssMaxIdleEn */
			if (RTMPGetKeyParameter("BssMaxIdleEn", tmpbuf, 128, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					if (os_str_tol(macptr, 0, 10) != 0) /*Enable*/
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].max_idle_ie_en = TRUE;
					else /*Disable*/
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].max_idle_ie_en = FALSE;

					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"I/F(ra%d) BssMaxIdleEn=%d\n", i,
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].max_idle_ie_en);
				}
			}

			/* BssMaxIdle */
			if (RTMPGetKeyParameter("BssMaxIdle", tmpbuf, 128, pBuffer, TRUE)) {
				UINT16 idle_period;

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					idle_period = (USHORT) os_str_tol(macptr, 0, 10);
					if ((idle_period < 1) || (idle_period > 0xffff))
						idle_period = MAC_TABLE_AGEOUT_TIME;

					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].max_idle_period = idle_period;

					/* Apply BSS[0] setting to all as default */
					if (i == 0)
						for (j = 1; j < pAd->ApCfg.BssidNum; j++)
							pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, j)].max_idle_period = idle_period;
				}
			}

			/*NoForwarding*/
			if (RTMPGetKeyParameter("NoForwarding", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					if (os_str_tol(macptr, 0, 10) != 0) /*Enable*/
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].IsolateInterStaTraffic = TRUE;
					else /*Disable*/
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].IsolateInterStaTraffic = FALSE;

					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(ra%d) NoForwarding=%ld\n", i,
							 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].IsolateInterStaTraffic);
				}
			}

			/*NoForwardingBTNBSSID*/
			if (RTMPGetKeyParameter("NoForwardingBTNBSSID", tmpbuf, 10, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) != 0) /*Enable*/
					pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = TRUE;
				else /*Disable*/
					pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = FALSE;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "NoForwardingBTNBSSID=%ld\n",
						 pAd->ApCfg.IsolateInterStaTrafficBTNBSSID);
			}

			/*HideSSID*/
			if (RTMPGetKeyParameter("HideSSID", tmpbuf, 100, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					int apidx = i;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					if (os_str_tol(macptr, 0, 10) != 0) { /*Enable*/
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].bHideSsid = TRUE;
#ifdef WSC_V2_SUPPORT
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].wdev.WscControl.WscV2Info.bWpsEnable = FALSE;
#endif /* WSC_V2_SUPPORT */
					} else /*Disable*/
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].bHideSsid = FALSE;

					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(ra%d) HideSSID=%d\n", i,
							 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].bHideSsid);
				}
			}

			/*StationKeepAlive*/
			if (RTMPGetKeyParameter("StationKeepAlive", tmpbuf, PER_BSS_SIZE_2(pAd), pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					int apidx = i;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].StationKeepAliveTime = os_str_tol(macptr, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(ra%d) StationKeepAliveTime=%d\n", i,
							 pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, apidx)].StationKeepAliveTime);
				}
			}

			/*AutoChannelSkipList*/
			if (RTMPGetKeyParameter("AutoChannelSkipList", tmpbuf, 128, pBuffer, FALSE)) {
				pAd->ApCfg.AutoChannelSkipListNum = delimitcnt(tmpbuf, ";") + 1;
				if (delimitcnt(tmpbuf, ";"))
					pAd->ApCfg.AutoChannelSkipListNum =
						delimitcnt(tmpbuf, ";") + 1;
				else
					pAd->ApCfg.AutoChannelSkipListNum = 0;

				/*
				if (pAd->ApCfg.AutoChannelSkipListNum > 10) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "Your no. of AutoChannelSkipList( %d ) is larger than 10 (boundary)\n", pAd->ApCfg.AutoChannelSkipListNum);
					pAd->ApCfg.AutoChannelSkipListNum = 10;
				}*/

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i < pAd->ApCfg.AutoChannelSkipListNum) {
						pAd->ApCfg.AutoChannelSkipList[i] = os_str_tol(macptr, 0, 10);
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, " AutoChannelSkipList[%d]= %d\n", i,
								 pAd->ApCfg.AutoChannelSkipList[i]);
					} else
						break;
				}
			}

			/*AutoChannelSkipList6G*/
			if (RTMPGetKeyParameter("AutoChannelSkipList6G", tmpbuf, 128, pBuffer, FALSE)) {
				if (delimitcnt(tmpbuf, ";")) {
					pAd->ApCfg.AutoChannelSkipListNum6G =
						delimitcnt(tmpbuf, ";") + 1;
					if (pAd->ApCfg.AutoChannelSkipListNum6G > MAX_NUM_OF_CHANNELS) {
						pAd->ApCfg.AutoChannelSkipListNum6G = MAX_NUM_OF_CHANNELS;
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, " Exceed the upper limit, adjust to = %d\n", MAX_NUM_OF_CHANNELS);
						}
				} else
					pAd->ApCfg.AutoChannelSkipListNum6G = 0;
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i < pAd->ApCfg.AutoChannelSkipListNum6G)
						pAd->ApCfg.AutoChannelSkipList6G[i] = os_str_tol(macptr, 0, 10);
					else
						break;
				}
			}

#ifdef BACKGROUND_SCAN_SUPPORT

			if (RTMPGetKeyParameter("DfsZeroWait", tmpbuf, 50, pBuffer, FALSE)) {
				UINT8 DfsZeroWait = os_str_tol(tmpbuf, 0, 10);

				if ((DfsZeroWait == 1)
#ifdef MT_DFS_SUPPORT
					&& IS_SUPPORT_MT_DFS(pAd)
#endif
				   ) {
					pAd->BgndScanCtrl.DfsZeroWaitSupport = TRUE;/*Enable*/
#ifdef MT_DFS_SUPPORT
					UPDATE_MT_ZEROWAIT_DFS_Support(pAd, TRUE);
#endif
				} else {
					pAd->BgndScanCtrl.DfsZeroWaitSupport = FALSE;
#ifdef MT_DFS_SUPPORT
					UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
#endif
				}

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DfsZeroWait Support=%d/%d\n", DfsZeroWait,
						 pAd->BgndScanCtrl.DfsZeroWaitSupport);
			}

#ifdef MT_DFS_SUPPORT
			if (RTMPGetKeyParameter("DfsDedicatedZeroWait", tmpbuf, 25, pBuffer, TRUE)) {
				UCHAR DfsDedicatedZeroWait = (UCHAR) simple_strtol(tmpbuf, 0, 10);

#if defined(MT7981)
			/* support 3 antenna in FW, efuse_max_band1_path == 1 means support 2 antenna in FW */
			if (cap->efuse_max_band1_path == 0 && DfsDedicatedZeroWait != DFS_DEDICATED_ZERO_WAIT_DISABLED
#ifdef DFS_ADJ_BW_ZERO_WAIT
				&& DfsDedicatedZeroWait != DFS_DEDICATED_ZERO_WAIT_BW_160_ENABLE
#endif
				)
				DfsDedicatedZeroWait = DFS_DEDICATED_ZERO_WAIT_DEFAULT_FLOW_ENABLED;
#endif  /* defined(MT7981) */

#if ((RDD_PROJECT_TYPE_2 == 1) || (RDD_2_SUPPORTED == 1))
				switch (DfsDedicatedZeroWait) {
				case DFS_DEDICATED_ZERO_WAIT_DISABLED:
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
					break;

				case DFS_DEDICATED_ZERO_WAIT_ENABLED:
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
#if (defined(DFS_MT7916_DEDICATED_ZW) || defined(DFS_MT7981_DEDICATED_ZW)) && !defined(ZWDFS_AX7800)
					pAd->CommonCfg.DfsParameter.BW160DedicatedSup = TRUE;
#endif
					break;

				case DFS_DEDICATED_ZERO_WAIT_DEFAULT_FLOW_ENABLED:
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = TRUE;
#if (defined(DFS_MT7916_DEDICATED_ZW) || defined(DFS_MT7981_DEDICATED_ZW)) && !defined(ZWDFS_AX7800)
					pAd->CommonCfg.DfsParameter.BW160DedicatedSup = TRUE;
#endif
					break;

				default:
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
					break;
				}

#else
				if (!pAd->CommonCfg.dbdc_mode) {
					switch (DfsDedicatedZeroWait) {
					case DFS_DEDICATED_ZERO_WAIT_DISABLED:
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
						break;

					case DFS_DEDICATED_ZERO_WAIT_ENABLED:
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
						break;

					case DFS_DEDICATED_ZERO_WAIT_DEFAULT_FLOW_ENABLED:
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = TRUE;
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = TRUE;
						break;

					default:
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
						pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
						break;
					}
				} else {
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport = FALSE;
					pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = FALSE;
				}

#endif
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
				pAd->CommonCfg.DfsParameter.bDFSV10Support = TRUE;
#endif
#ifdef DFS_ADJ_BW_ZERO_WAIT
				if ((DfsDedicatedZeroWait == DFS_DEDICATED_ZERO_WAIT_BW_160_ENABLE) &&
					(pAd->CommonCfg.DfsParameter.bDfsEnable))
					pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport = TRUE;
				else
					pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport = FALSE;
#endif
			}
			if (RTMPGetKeyParameter("DfsZeroWaitDefault", tmpbuf, 25, pBuffer, TRUE)) {
				UCHAR DfsZeroWaitDefault = (UCHAR) simple_strtol(tmpbuf, 0, 10);
				pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitDefault = DfsZeroWaitDefault;
			}
			if (RTMPGetKeyParameter("DfsTargetCh", tmpbuf, 25, pBuffer, TRUE)) {
				UCHAR Channel = 0, Bw = 0;
				USHORT CacValue = 0;

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					switch (i) {
					case 0:
						Channel = simple_strtol(macptr, 0, 10);
						break;
					case 1:
						Bw = simple_strtol(macptr, 0, 10);
						break;
					case 2:
						CacValue = simple_strtol(macptr, 0, 10);
						break;
					default:
						break;
					}
				}
				ZeroWait_DFS_Pre_Assign_Next_Target_Channel(pAd, Channel, Bw, CacValue);
			}

			if (RTMPGetKeyParameter("DfsChSelPrefer", tmpbuf, 25, pBuffer, TRUE)) {
				UCHAR Prefer = (UCHAR) simple_strtol(tmpbuf, 0, 10);
				if (Prefer >= RadarDetectSelectNum)
					MTWF_PRINT("Invalid parameter, Please enter 0 or 1 or 2!\n");
				else
					pAd->CommonCfg.DfsParameter.DfsChSelPrefer = Prefer;
			}

#endif
			if (RTMPGetKeyParameter("BgndScanSkipCh", tmpbuf, 50, pBuffer, FALSE)) {
				pAd->BgndScanCtrl.SkipChannelNum = delimitcnt(tmpbuf, ";") + 1;

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i < pAd->BgndScanCtrl.SkipChannelNum) {
						pAd->BgndScanCtrl.SkipChannelList[i] = os_str_tol(macptr, 0, 10);
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, " Background Skip Channel list[%d]= %d\n", i,
								 pAd->BgndScanCtrl.SkipChannelList[i]);
					} else
						break;
				}
			}

#endif /* BACKGROUND_SCAN_SUPPORT */

			if (RTMPGetKeyParameter("EDCCACfgMode", tmpbuf, 10, pBuffer, FALSE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
					pAd->CommonCfg.u1EDCCACfgMode[i] = (UCHAR) simple_strtol(macptr, 0, 10);
			}

			if (RTMPGetKeyParameter("EDCCAEnable", tmpbuf, 10, pBuffer, FALSE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i < DBDC_BAND_NUM) {
#ifdef DEFAULT_5G_PROFILE
						if (i == 0 && (pAd->CommonCfg.dbdc_mode == 1)) {
							pAd->CommonCfg.u1EDCCACtrl[BAND1] = (UCHAR) simple_strtol(macptr, 0, 10);
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								" EDCCA band[1]= %d by profile\n", pAd->CommonCfg.u1EDCCACtrl[BAND1]);
						} else {
							pAd->CommonCfg.u1EDCCACtrl[BAND0] = simple_strtol(macptr, 0, 10);
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								" EDCCA band[0]= %d by profile\n", pAd->CommonCfg.u1EDCCACtrl[BAND0]);
						}

#else
						pAd->CommonCfg.u1EDCCACtrl[i] = simple_strtol(macptr, 0, 10);
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							" EDCCA band[%d]= %d by profile\n", i, pAd->CommonCfg.u1EDCCACtrl[i]);
#endif /* DEFAULT_5G_PROFILE */
					} else
						break;
				}
			}

			if (RTMPGetKeyParameter("EDCCAThreshold", tmpbuf, 32, pBuffer, FALSE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= DBDC_BAND_NUM)
						break;
					RTMPSetProfileEDCCAThresholdParameters(pAd, macptr, i);
				}
			}
			if (RTMPGetKeyParameter("CBP_COMPENSATION", tmpbuf, 32, pBuffer, FALSE)) {

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr && i < DBDC_BAND_NUM; macptr = rstrtok(NULL, ";"), i++) {
					RTMPSetProfileEDCCACompensationParameters(pAd, macptr, i);
				}
			}

			if (RTMPGetKeyParameter("SCSEnable", tmpbuf, 10, pBuffer, TRUE))
				SCS_Enable_set_cfg(pAd, tmpbuf);

#ifdef MT_DFS_SUPPORT

			if (RTMPGetKeyParameter("DfsZeroWaitCacTime", tmpbuf, 50, pBuffer, FALSE)) {
				UINT8 OffChnlCacTime = os_str_tol(tmpbuf, 0, 10);

				pAd->CommonCfg.DfsParameter.DfsZeroWaitCacTime = OffChnlCacTime; /* Unit is minute */
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DfsZeroWaitCacTime=%d/%d\n",
						 OffChnlCacTime,
						 pAd->CommonCfg.DfsParameter.DfsZeroWaitCacTime);
			}

#endif /* MT_DFS_SUPPORT  */
#ifdef AP_SCAN_SUPPORT

			/*ACSCheckTime*/
			if (RTMPGetKeyParameter("ACSCheckTime", tmpbuf, 32, pBuffer, TRUE)) {
				UINT8 i = 0;
				UINT32 time = 0;
				RTMP_STRING *ptr;
				struct wifi_dev *pwdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

				for (i = 0, ptr = rstrtok(tmpbuf, ";"); ptr; ptr = rstrtok(NULL, ";"), i++) {
					if (i >= DBDC_BAND_NUM)
						break;

					time = simple_strtol(ptr, 0, 10);
#if !defined(ACS_CTCC_SUPPORT)
					time = time * 3600;/* Hour to second */
#endif
					if (pAd->CommonCfg.eDBDC_mode == ENUM_DBDC_5G5G) {
						/* 5G + 5G */
						pAd->ApCfg.ACSCheckTime[i] = time;

					} else {
						if (WMODE_CAP_5G(pwdev->PhyMode)) {
							/* 5G + 2G */
							if (i == 0 && (pAd->CommonCfg.dbdc_mode == 1)) {
#ifdef DBDC_MODE
								/* [5G] + 2G */
								pAd->ApCfg.ACSCheckTime[BAND1] = time;
#endif
							} else {
								/* 5G + [2G] */
								pAd->ApCfg.ACSCheckTime[BAND0] = time;
							}
						} else {
							/* 2G + 5G or 2G only */
							pAd->ApCfg.ACSCheckTime[i] = time;
						}
					}
				}
				for (i = 0; i < DBDC_BAND_NUM; i++) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"ACSCheckTime[%d]=%u seconds\n",
					i, pAd->ApCfg.ACSCheckTime[i]);
				}
			}
			/*ACSCheckMinTime*/
			if (RTMPGetKeyParameter("ACSCheckMinTime", tmpbuf, 32, pBuffer, TRUE)) {
				UINT8 i = 0;
				UINT32 time = 0;
				RTMP_STRING *ptr;
				struct wifi_dev *pwdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

				for (i = 0, ptr = rstrtok(tmpbuf, ";"); ptr; ptr = rstrtok(NULL, ";"), i++) {
					if (i >= DBDC_BAND_NUM)
						break;

					time = simple_strtol(ptr, 0, 10);
					time = time * 60;/* minute to second */
					if (pAd->CommonCfg.eDBDC_mode == ENUM_DBDC_5G5G) {
						/* 5G + 5G */
						pAd->ApCfg.ACSCheckTime[i] = time;

					} else {
						if (WMODE_CAP_5G(pwdev->PhyMode)) {
							/* 5G + 2G */
							if (i == 0 && (pAd->CommonCfg.dbdc_mode == 1)) {
#ifdef DBDC_MODE
								/* [5G] + 2G */
								pAd->ApCfg.ACSCheckTime[BAND1] = time;
#endif
							} else {
								/* 5G + [2G] */
								pAd->ApCfg.ACSCheckTime[BAND0] = time;
							}
						} else {
							/* 2G + 5G or 2G only */
							pAd->ApCfg.ACSCheckTime[i] = time;
						}
					}
				}
				for (i = 0; i < DBDC_BAND_NUM; i++) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"ACSCheckTime[%d]=%u seconds\n",
					i, pAd->ApCfg.ACSCheckTime[i]);
				}
			}
#endif /* AP_SCAN_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

		/*ShortSlot*/
		if (RTMPGetKeyParameter("ShortSlot", tmpbuf, 10, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				UCHAR ShortSlot = 0;
				INT status = TRUE;
				if (i < 2) {
					status = RT_CfgSetShortSlot(pAd, macptr, i);
					if (status) {
						ShortSlot = os_str_tol(macptr, 0, 10);
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"band%d ShortSlot=%d\n", i, ShortSlot);
					} else
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"band%d wrong ShortSlot\n", i);
				}
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ShortSlot=%d\n", TRUE);
		}
#ifdef CONFIG_AP_SUPPORT
		if (RTMPGetKeyParameter("SlotTime", tmpbuf, 10, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				UCHAR SlotTime = 0;
				struct wifi_dev *wdev = NULL;
				UCHAR BandIdx;

				if (i >= pAd->ApCfg.BssidNum)
					break;

				wdev = &pAd->ApCfg.MBSSID[i].wdev;
				BandIdx = HcGetBandByWdev(wdev);
				SlotTime = os_str_tol(macptr, 0, 10);
				if ((SlotTime < 9) || (SlotTime > 25))
					SlotTime = 9;

				pAd->CommonCfg.bUseShortSlotTime[BandIdx] = (SlotTime == 9) ? TRUE : FALSE;
				pAd->CommonCfg.SlotTime[BandIdx] = SlotTime;
				if (wdev->channel > 14) {
					pAd->CommonCfg.bUseShortSlotTime[BandIdx] = TRUE;
					pAd->CommonCfg.SlotTime[BandIdx] = 9;
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef TXBF_SUPPORT

		if (cap->FlgHwTxBfCap) {
			/* Set ETxBfEnCond to wdev->wpf_cfg->phy_conf.ETxBfEnCond and pAd->CommonCfg.ETxBfEnCond */
			read_txbf_param_from_file(pAd, tmpbuf, pBuffer);

#if defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT)
			/* ITxBfTimeout */
			if (RTMPGetKeyParameter("ITxBfTimeout", tmpbuf, 32, pBuffer, TRUE)) {
				pAd->CommonCfg.ITxBfTimeout = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ITxBfTimeout = %ld\n", pAd->CommonCfg.ITxBfTimeout);
			}
#endif /* defined(CONFIG_AP_SUPPORT) || defined(STA_ITXBF_SUPPORT) */

			/* ETxBfEnCond */
			if (RTMPGetKeyParameter("ETxBfEnCond", tmpbuf, PER_BSS_SIZE_2(pAd), pBuffer, TRUE)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CommnCfg.ETxBfEnCond = %ld\n", pAd->CommonCfg.ETxBfEnCond);

				if (pAd->CommonCfg.ETxBfEnCond)
					pAd->CommonCfg.RegTransmitSetting.field.TxBF = TRUE;
				else
					pAd->CommonCfg.RegTransmitSetting.field.TxBF = FALSE;

				/* MUTxRxEnable*/
				if (RTMPGetKeyParameter("MUTxRxEnable", tmpbuf, 32, pBuffer, TRUE)) {
					pAd->CommonCfg.MUTxRxEnable = os_str_tol(tmpbuf, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MUTxRxEnable = %ld\n", pAd->CommonCfg.MUTxRxEnable);
				}
			}

#ifdef CFG_SUPPORT_MU_MIMO_RA
				if (RTMPGetKeyParameter("MuEnable7615HwPatch", tmpbuf, 10, pBuffer, TRUE)) {
					UINT8 IsEnable = FALSE;

					IsEnable = simple_strtol(tmpbuf, 0, 10);
					pAd->MuHwSwPatch = IsEnable;
				}
#endif

			/* ETxBfTimeout*/
			if (RTMPGetKeyParameter("ETxBfTimeout", tmpbuf, 32, pBuffer, TRUE)) {
				pAd->CommonCfg.ETxBfTimeout = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ETxBfTimeout = %ld\n", pAd->CommonCfg.ETxBfTimeout);
			}

			/* ETxBfNoncompress*/
			if (RTMPGetKeyParameter("ETxBfNoncompress", tmpbuf, 32, pBuffer, TRUE)) {
				pAd->CommonCfg.ETxBfNoncompress = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ETxBfNoncompress = %d\n", pAd->CommonCfg.ETxBfNoncompress);
			}

			/* ETxBfIncapable */
			if (RTMPGetKeyParameter("ETxBfIncapable", tmpbuf, 32, pBuffer, TRUE)) {
				pAd->CommonCfg.ETxBfIncapable = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ETxBfIncapable = %d\n", pAd->CommonCfg.ETxBfIncapable);
			}
		}

#endif /* TXBF_SUPPORT */

#ifdef DOT11_HE_AX
	read_mu_param_from_file(pAd, tmpbuf, pBuffer);
	read_he_dyn_smps_from_file(pAd, tmpbuf, pBuffer);
	read_ul_mu_data_param_from_file(pAd, tmpbuf, pBuffer);
	read_er_su_param_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT11_HE_AX */

#ifdef STREAM_MODE_SUPPORT

		/* StreamMode*/
		if (cap->FlgHwStreamMode) {
			if (RTMPGetKeyParameter("StreamMode", tmpbuf, 32, pBuffer, TRUE)) {
				pAd->CommonCfg.StreamMode = (os_str_tol(tmpbuf, 0, 10) & 0x03);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "StreamMode= %d\n", pAd->CommonCfg.StreamMode);
			}

			/* StreamModeMac*/
			for (i = 0; i < STREAM_MODE_STA_NUM; i++) {
				RTMP_STRING tok_str[32];

				sprintf(tok_str, "StreamModeMac%d", i);

				if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAM_BUFFER_SIZE, pBuffer, TRUE)) {
					int j;

					if (strlen(tmpbuf) != 17) /*Mac address acceptable format 01:02:03:04:05:06 length 17*/
						continue;

					for (j = 0; j < MAC_ADDR_LEN; j++) {
						AtoH(tmpbuf, &pAd->CommonCfg.StreamModeMac[i][j], 1);
						tmpbuf = tmpbuf + 3;
					}
				}
			}

			if (NdisEqualMemory(ZERO_MAC_ADDR, &pAd->CommonCfg.StreamModeMac[0][0], MAC_ADDR_LEN)) {
				/* set default broadcast mac to entry 0 if user not set it */
				NdisMoveMemory(&pAd->CommonCfg.StreamModeMac[0][0], BROADCAST_ADDR, MAC_ADDR_LEN);
			}
		}

#endif /* STREAM_MODE_SUPPORT */

		/*IEEE80211H*/
		if (RTMPGetKeyParameter("IEEE80211H", tmpbuf, 10, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
				if (os_str_tol(macptr, 0, 10) != 0) /*Enable*/
					pAd->CommonCfg.bIEEE80211H = TRUE;
				else { /*Disable*/
					pAd->CommonCfg.bIEEE80211H = FALSE;
#ifdef BACKGROUND_SCAN_SUPPORT
					pAd->BgndScanCtrl.DfsZeroWaitSupport = FALSE;
#endif
#ifdef MT_DFS_SUPPORT
					pAd->CommonCfg.DfsParameter.bDfsEnable = FALSE;
					UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Disable DFS/Zero wait=%d/%d\n",
							 IS_SUPPORT_MT_DFS(pAd),
							 IS_SUPPORT_MT_ZEROWAIT_DFS(pAd));
#endif
				}

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IEEE80211H=%d\n", pAd->CommonCfg.bIEEE80211H);
			}
		}


		/*RDRegion*/
		if (RTMPGetKeyParameter("RDRegion", tmpbuf, 128, pBuffer, TRUE)) {
			if ((strncmp(tmpbuf, "JAP_W53", 7) == 0) || (strncmp(tmpbuf, "jap_w53", 7) == 0)) {
				pAd->CommonCfg.RDDurRegion = JAP_W53;
				/*pRadarDetect->DfsSessionTime = 15;*/
			} else if ((strncmp(tmpbuf, "JAP_W56", 7) == 0) || (strncmp(tmpbuf, "jap_w56", 7) == 0)) {
				pAd->CommonCfg.RDDurRegion = JAP_W56;
				/*pRadarDetect->DfsSessionTime = 13;*/
			} else if ((strncmp(tmpbuf, "JAP", 3) == 0) || (strncmp(tmpbuf, "jap", 3) == 0)) {
				pAd->CommonCfg.RDDurRegion = JAP;
				/*pRadarDetect->DfsSessionTime = 5;*/
			} else  if ((strncmp(tmpbuf, "FCC", 3) == 0) || (strncmp(tmpbuf, "fcc", 3) == 0)) {
				pAd->CommonCfg.RDDurRegion = FCC;
				/*pRadarDetect->DfsSessionTime = 5;*/
			} else if ((strncmp(tmpbuf, "CE", 2) == 0) || (strncmp(tmpbuf, "ce", 2) == 0)) {
				pAd->CommonCfg.RDDurRegion = CE;
				/*pRadarDetect->DfsSessionTime = 13;*/
			} else if ((strncmp(tmpbuf, "KR", 2) == 0) || (strncmp(tmpbuf, "kr", 2) == 0)) {
				pAd->CommonCfg.RDDurRegion = KR;
				/*pRadarDetect->DfsSessionTime = 13;*/
			} else {
				pAd->CommonCfg.RDDurRegion = CE;
				/*pRadarDetect->DfsSessionTime = 13;*/
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RDRegion=%d\n", pAd->CommonCfg.RDDurRegion);
		} else {
			pAd->CommonCfg.RDDurRegion = CE;
			/*pRadarDetect->DfsSessionTime = 13;*/
		}

#ifdef SYSTEM_LOG_SUPPORT

		/*WirelessEvent*/
		if (RTMPGetKeyParameter("WirelessEvent", tmpbuf, 10, pBuffer, TRUE)) {
			BOOLEAN FlgIsWEntSup = FALSE;

			if (os_str_tol(tmpbuf, 0, 10) != 0)
				FlgIsWEntSup = TRUE;

			RtmpOsWlanEventSet(pAd, &pAd->CommonCfg.bWirelessEvent, FlgIsWEntSup);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WirelessEvent=%d\n", pAd->CommonCfg.bWirelessEvent);
		}

#endif /* SYSTEM_LOG_SUPPORT */
		/*Security Parameters */
		ReadSecurityParameterFromFile(pAd, tmpbuf, pBuffer);
#ifdef MBO_SUPPORT
		ReadMboParameterFromFile(pAd, tmpbuf, pBuffer);
#endif /* MBO_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
		ReadMapParameterFromFile(pAd, tmpbuf, pBuffer);
#endif /* MAP_SUPPORT */
#ifdef QOS_R1
		if (RTMPGetKeyParameter("QoSR1Enable", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) != 0)
				pAd->bQoSR1Enable = os_str_tol(tmpbuf, 0, 10);
			else
				pAd->bQoSR1Enable = 0;	/* disable*/
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"QoSR1Enable=%d\n", pAd->bQoSR1Enable);

#ifdef DSCP_PRI_SUPPORT
			if (IS_QOSR1_ENABLE(pAd))
				QoS_Init_DSCP2UP_Mapping(pAd);
#endif
		}
#ifdef QOS_R2
		if (RTMPGetKeyParameter("ScsEnable", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) != 0)
				pAd->bScsEnable = os_str_tol(tmpbuf, 0, 10);
			else
				pAd->bScsEnable = 0;	/* disable*/
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ScsEnable=%d\n", pAd->bScsEnable);
		}
		if (RTMPGetKeyParameter("QoSMgmtCapa", tmpbuf, 10, pBuffer, TRUE)) {
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.bQoSMCapability
					= os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"%s, QoSMgmtCapa=%d\n",
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.if_dev->name,
				pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.bQoSMCapability);
			}
		}
#endif /* QOS_R2 */

#ifdef MSCS_PROPRIETARY
		if (RTMPGetKeyParameter("DABSgroupkeybitmap", tmpbuf, 16, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 16) != 0) {
				*((ULONG *)&pAd->keybitmap[2]) = (ULONG)os_str_tol(tmpbuf, 0, 16);
				}
				if (pAd->keybitmap[2] == 0 && pAd->keybitmap[3] == 0)
					pAd->keybitmap[2] = GENERAL_KEY;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"DABSkeybitmap=%lu\n", *((ULONG *)&pAd->keybitmap[2]));
		}
		if (RTMPGetKeyParameter("DABSvendorkeybitmap", tmpbuf, 16, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 16) != 0) {
				*((ULONG *)&pAd->keybitmap[0]) = (ULONG)os_str_tol(tmpbuf, 0, 16);
				}
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"DABSkeybitmap=%lu\n", *((ULONG *)&pAd->keybitmap[0]));
		}
#endif	/* MSCS_PROPRIETARY */
#endif /* QOS_R1 */
#ifdef DSCP_PRI_SUPPORT
		/*read custom DSCP mapping,only after QOS DSCP init
		* As QOS DSCP init resets the mapping table*/
		read_dscp_pri_param(pAd, tmpbuf, pBuffer);
#endif /*DSCP_PRI_SUPPORT*/
#ifdef DPP_SUPPORT
		if (RTMPGetKeyParameter("DppEnable", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) != 0)
				pAd->bDppEnable = os_str_tol(tmpbuf, 0, 10);
			else
				pAd->bDppEnable = 0;	/* disable*/
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DppEnable=%d\n", pAd->bDppEnable);
		}
#endif /* DPP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			/*Access Control List*/
			rtmp_read_acl_parms_from_file(pAd, tmpbuf, pBuffer);
#ifdef APCLI_SUPPORT
			rtmp_read_ap_client_from_file(pAd, tmpbuf, pBuffer);
#endif /* APCLI_SUPPORT */
#ifdef IGMP_SNOOP_SUPPORT
			/* Igmp Snooping information*/
			rtmp_read_igmp_snoop_from_file(pAd, tmpbuf, pBuffer);
#endif /* IGMP_SNOOP_SUPPORT */
#ifdef WDS_SUPPORT
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode == FALSE)
				profile_wds_reg(pAd, DBDC_BAND0, pBuffer);
#endif	/* DBDC_MODE */
			rtmp_read_wds_from_file(pAd, tmpbuf, pBuffer);
#endif /* WDS_SUPPORT */
#ifdef IDS_SUPPORT
			rtmp_read_ids_from_file(pAd, tmpbuf, pBuffer);
#endif /* IDS_SUPPORT */
#ifdef MWDS
			rtmp_read_MWDS_from_file(pAd, tmpbuf, pBuffer);
#endif /* MWDS */
#ifdef MAC_REPEATER_SUPPORT

			if (RTMPGetKeyParameter("MACRepeaterEn", tmpbuf, 10, pBuffer, FALSE)) {
				BOOLEAN bEnable = FALSE;
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (os_str_tol(macptr, 0, 10) != 0)
						bEnable = TRUE;
					else
						bEnable = FALSE;

					AsicSetReptFuncEnable(pAd, bEnable, i);
					pAd->ApCfg.bMACRepeaterEn_precfg = bEnable;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MACRepeaterEn=%d Band=%d\n", pAd->ApCfg.bMACRepeaterEn_precfg, i);
				}
				/* Disable DFS zero wait support */
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)

				if (pAd->ApCfg.bMACRepeaterEn_precfg) {
					pAd->BgndScanCtrl.DfsZeroWaitSupport = FALSE;
					UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\x1b[31m:Disable DfsZeroWait\x1b[m\n");
				}

#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */
			}

			if (RTMPGetKeyParameter("MACRepeaterOuiMode", tmpbuf, 10, pBuffer, FALSE)) {
				INT OuiMode = os_str_tol(tmpbuf, 0, 10);

				if (OuiMode == CASUALLY_DEFINE_MAC_ADDR)
					pAd->ApCfg.MACRepeaterOuiMode = CASUALLY_DEFINE_MAC_ADDR;
				else if (OuiMode == VENDOR_DEFINED_MAC_ADDR_OUI)
					pAd->ApCfg.MACRepeaterOuiMode = VENDOR_DEFINED_MAC_ADDR_OUI; /* customer specific */
				else
					pAd->ApCfg.MACRepeaterOuiMode = FOLLOW_CLI_LINK_MAC_ADDR_OUI; /* use Ap-Client first 3 bytes MAC assress (default) */

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MACRepeaterOuiMode=%d\n", pAd->ApCfg.MACRepeaterOuiMode);
			}

#endif /* MAC_REPEATER_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

		if (RTMPGetKeyParameter("SE_OFF", tmpbuf, 25, pBuffer, TRUE)) {
			ULONG SeOff = os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.bSeOff = SeOff > 0 ? TRUE : FALSE;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SE_OFF=%d\n",
					 pAd->CommonCfg.bSeOff);
		}

		if (RTMPGetKeyParameter("AntennaIndex", tmpbuf, 25, pBuffer, TRUE)) {
			ULONG antenna_index = simple_strtol(tmpbuf, 0, 10);

			if (antenna_index > 28)
				antenna_index = 0;

			if (antenna_index == 24 || antenna_index == 25)
				antenna_index = 0;

			pAd->CommonCfg.ucAntennaIndex = antenna_index;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "antenna_index=%d\n",
					 pAd->CommonCfg.ucAntennaIndex);
		}

#ifdef DOT11_N_SUPPORT
		read_ht_param_from_file(pAd, tmpbuf, pBuffer);
#ifdef DOT11_VHT_AC
		read_vht_param_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
		read_he_param_from_file(pAd, tmpbuf, pBuffer);
#endif
#endif /* DOT11_N_SUPPORT */

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		if (RTMPGetKeyParameter("OldChannel_Dev1", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldChannelCfg(pAd, tmpbuf);
		if (RTMPGetKeyParameter("OldChannel_Dev2", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldChannelCfg(pAd, tmpbuf);

		if (RTMPGetKeyParameter("OldHTBW_Dev1", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldBWCfg(pAd, tmpbuf, FALSE);
		if (RTMPGetKeyParameter("OldHTBW_Dev2", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldBWCfg(pAd, tmpbuf, FALSE);

		if (RTMPGetKeyParameter("OldVHTBW_Dev1", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldBWCfg(pAd, tmpbuf, TRUE);
		if (RTMPGetKeyParameter("OldVHTBW_Dev2", tmpbuf, 25, pBuffer, TRUE))
			RTMPOldBWCfg(pAd, tmpbuf, TRUE);

		if (RTMPGetKeyParameter("Ch144Support", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) == 1)
				pAd->CommonCfg.bCh144Enabled = TRUE;
			else
				pAd->CommonCfg.bCh144Enabled = FALSE;
		}
#endif

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef WSC_AP_SUPPORT
			RTMP_STRING tok_str[16] = {0};

			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				snprintf(tok_str, sizeof(tok_str), "WscDefaultSSID%d", i + 1);

				if (RTMPGetKeyParameter(tok_str, tmpbuf, 33, pBuffer, FALSE)) {
					NdisZeroMemory(&pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscDefaultSsid, sizeof(NDIS_802_11_SSID));
					NdisMoveMemory(pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscDefaultSsid.Ssid, tmpbuf, strlen(tmpbuf));
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscDefaultSsid.SsidLength = strlen(tmpbuf);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "WscDefaultSSID[%d]=%s\n", i, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscDefaultSsid.Ssid);
				}
			}

			/*WscConfMode*/
			if (RTMPGetKeyParameter("WscConfMode", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					INT WscConfMode = os_str_tol(macptr, 0, 10);

					if (i >= pAd->ApCfg.BssidNum)
						break;

					if (WscConfMode > 0 && WscConfMode < 8)
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscConfMode = WscConfMode;
					else
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscConfMode = WSC_DISABLE;

					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "I/F(ra%d) WscConfMode=%d\n", i, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscConfMode);
				}
			}

			/*WscConfStatus*/
			if (RTMPGetKeyParameter("WscConfStatus", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscConfStatus = (INT) os_str_tol(macptr, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "I/F(ra%d) WscConfStatus=%d\n", i, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscConfStatus);
				}
			}

			/*WscConfMethods*/
			if (RTMPGetKeyParameter("WscConfMethods", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscConfigMethods = (USHORT)os_str_tol(macptr, 0, 16);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "I/F(ra%d) WscConfMethods=0x%x\n", i, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscConfigMethods);
				}
			}

			/*WscKeyASCII (0:Hex, 1:ASCII(random length), others: ASCII length, default 8)*/
			if (RTMPGetKeyParameter("WscKeyASCII", tmpbuf, 10, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					INT Value;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					Value = (INT) os_str_tol(tmpbuf, 0, 10);

					if (Value == 0 || Value == 1)
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscKeyASCII = Value;
					else if (Value >= 8 && Value <= 63)
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscKeyASCII = Value;
					else
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscKeyASCII = 8;

					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							 "WscKeyASCII=%d\n", pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscKeyASCII);
				}
			}

			if (RTMPGetKeyParameter("WscSecurityMode", tmpbuf, 50, pBuffer, TRUE)) {
				for (i = 0; i < pAd->ApCfg.BssidNum; i++)
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscSecurityMode = WPAPSKTKIP;

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					INT tmpMode = 0;

					if (i >= pAd->ApCfg.BssidNum)
						break;

					tmpMode = (INT) os_str_tol(macptr, 0, 10);

					if (tmpMode <= WPAPSKTKIP)
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscSecurityMode = tmpMode;

					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RTMPSetProfileParameters I/F(ra%d) WscSecurityMode=%d\n",
							 i, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscSecurityMode);
				}
			}

			/* WCNTest*/
			if (RTMPGetKeyParameter("WCNTest", tmpbuf, 10, pBuffer, TRUE)) {
				BOOLEAN	bEn = FALSE;

				if ((strncmp(tmpbuf, "0", 1) == 0))
					bEn = FALSE;
				else
					bEn = TRUE;

				for (i = 0; i < pAd->ApCfg.BssidNum; i++)
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.bWCNTest = bEn;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WCNTest=%d\n", bEn);
			}

			/*WSC UUID Str*/
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				PWSC_CTRL	pWpsCtrl = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl;

				snprintf(tok_str, sizeof(tok_str), "WSC_UUID_Str%d", i + 1);

				if (RTMPGetKeyParameter(tok_str, tmpbuf, 40, pBuffer, FALSE)) {
					NdisMoveMemory(&pWpsCtrl->Wsc_Uuid_Str[0], tmpbuf, strlen(tmpbuf));
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "UUID_Str[%d]=%s\n", i + 1, pWpsCtrl->Wsc_Uuid_Str);
				}
			}

			/*WSC UUID Hex*/
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				PWSC_CTRL	pWpsCtrl = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl;

				snprintf(tok_str, sizeof(tok_str), "WSC_UUID_E%d", i + 1);

				if (RTMPGetKeyParameter(tok_str, tmpbuf, 40, pBuffer, FALSE)) {
					AtoH(tmpbuf, &pWpsCtrl->Wsc_Uuid_E[0], UUID_LEN_HEX);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Wsc_Uuid_E[%d]", i + 1);
					hex_dump("", &pWpsCtrl->Wsc_Uuid_E[0], UUID_LEN_HEX);
				}
			}

			/* WSC AutoTrigger Disable */
			if (RTMPGetKeyParameter("WscAutoTriggerDisable", tmpbuf, 10, pBuffer, TRUE)) {
				BOOLEAN	bEn = FALSE;

				if ((strncmp(tmpbuf, "0", 1) == 0))
					bEn = FALSE;
				else
					bEn = TRUE;

				for (i = 0; i < pAd->ApCfg.BssidNum; i++)
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.bWscAutoTriggerDisable = bEn;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "bWscAutoTriggerDisable=%d\n", bEn);
			}

#endif /* WSC_AP_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
			if (RTMPGetKeyParameter("RoamingEnhance", tmpbuf, 32, pBuffer, TRUE))
				pAd->ApCfg.bRoamingEnhance = (simple_strtol(tmpbuf, 0, 10) > 0)?TRUE:FALSE;
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT

		/*CarrierDetect*/
		if (RTMPGetKeyParameter("CarrierDetect", tmpbuf, 128, pBuffer, TRUE)) {
			if ((strncmp(tmpbuf, "0", 1) == 0))
				pAd->CommonCfg.CarrierDetect.Enable = FALSE;
			else if ((strncmp(tmpbuf, "1", 1) == 0))
				pAd->CommonCfg.CarrierDetect.Enable = TRUE;
			else
				pAd->CommonCfg.CarrierDetect.Enable = FALSE;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CarrierDetect.Enable=%d\n",
					 pAd->CommonCfg.CarrierDetect.Enable);
		} else
			pAd->CommonCfg.CarrierDetect.Enable = FALSE;

#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			/*PSMode*/
			if (RTMPGetKeyParameter("PSMode", tmpbuf, MAX_PARAM_BUFFER_SIZE, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr != NULL) && (i < MAX_MULTI_STA); macptr = rstrtok(NULL, ";"), i++) {
					if (pAd->StaCfg[i].BssType == BSS_INFRA) {
						if ((strcmp(tmpbuf, "MAX_PSP") == 0) || (strcmp(tmpbuf, "max_psp") == 0)) {
							/*
								do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange()
								to exclude certain situations
							*/
							/*	MlmeSetPsm(pAd, PWR_SAVE);*/
							OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

							if (pAd->StaCfg[i].bWindowsACCAMEnable == FALSE)
								pAd->StaCfg[i].WindowsPowerMode = Ndis802_11PowerModeMAX_PSP;

							pAd->StaCfg[i].WindowsBatteryPowerMode = Ndis802_11PowerModeMAX_PSP;
							pAd->StaCfg[i].DefaultListenCount = 5;
						} else if ((strcmp(tmpbuf, "Fast_PSP") == 0) || (strcmp(tmpbuf, "fast_psp") == 0)
								   || (strcmp(tmpbuf, "FAST_PSP") == 0)) {
							/* do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange()*/
							/* to exclude certain situations.*/
							OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

							if (pAd->StaCfg[i].bWindowsACCAMEnable == FALSE)
								pAd->StaCfg[i].WindowsPowerMode = Ndis802_11PowerModeFast_PSP;

							pAd->StaCfg[i].WindowsBatteryPowerMode = Ndis802_11PowerModeFast_PSP;
							pAd->StaCfg[i].DefaultListenCount = 3;
						} else if ((strcmp(tmpbuf, "Legacy_PSP") == 0) || (strcmp(tmpbuf, "legacy_psp") == 0)
								   || (strcmp(tmpbuf, "LEGACY_PSP") == 0)) {
							/* do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange()*/
							/* to exclude certain situations.*/
							OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

							if (pAd->StaCfg[i].bWindowsACCAMEnable == FALSE)
								pAd->StaCfg[i].WindowsPowerMode = Ndis802_11PowerModeLegacy_PSP;

							pAd->StaCfg[i].WindowsBatteryPowerMode = Ndis802_11PowerModeLegacy_PSP;
							pAd->StaCfg[i].DefaultListenCount = 3;
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
							pAd->StaCfg[i].DefaultListenCount = 1;
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
						} else {
							/*Default Ndis802_11PowerModeCAM*/
							/* clear PSM bit immediately*/
							RTMP_SET_PSM_BIT(pAd, &pAd->StaCfg[i], PWR_ACTIVE);
							OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

							if (pAd->StaCfg[i].bWindowsACCAMEnable == FALSE)
								pAd->StaCfg[i].WindowsPowerMode = Ndis802_11PowerModeCAM;

							pAd->StaCfg[i].WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
						}

						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pAd->StaCfg[%d]::PSMode=%ld\n", i,
								 pAd->StaCfg[i].WindowsPowerMode);
					}
				}
			}

			/* AutoRoaming by RSSI*/
			if (RTMPGetKeyParameter("AutoRoaming", tmpbuf, 32, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) == 0)
					pAd->StaCfg[0].bAutoRoaming = FALSE;
				else
					pAd->StaCfg[0].bAutoRoaming = TRUE;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AutoRoaming=%d\n", pAd->StaCfg[0].bAutoRoaming);
			}

			/* RoamThreshold*/
			if (RTMPGetKeyParameter("RoamThreshold", tmpbuf, 32, pBuffer, TRUE)) {
				long lInfo = os_str_tol(tmpbuf, 0, 10);

				if (lInfo > 90 || lInfo < 60)
					pAd->StaCfg[0].dBmToRoam = -70;
				else
					pAd->StaCfg[0].dBmToRoam = (CHAR)(-1) * lInfo;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RoamThreshold=%d  dBm\n", pAd->StaCfg[0].dBmToRoam);
			}


			if (RTMPGetKeyParameter("TGnWifiTest", tmpbuf, 10, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) == 0)
					pAd->StaCfg[0].bTGnWifiTest = FALSE;
				else
					pAd->StaCfg[0].bTGnWifiTest = TRUE;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TGnWifiTest=%d\n", pAd->StaCfg[0].bTGnWifiTest);
			}

#ifdef ETH_CONVERT_SUPPORT

			/* Ethernet Converter Operation Mode.*/
			if (RTMPGetKeyParameter("EthConvertMode", tmpbuf, 32, pBuffer, TRUE)) {
				Set_EthConvertMode_Proc(pAd, tmpbuf);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "EthConvertMode=%d\n", pAd->EthConvert.ECMode);
			}

			/* Ethernet Converter Operation Mode.*/
			if (RTMPGetKeyParameter("EthCloneMac", tmpbuf, 32, pBuffer, TRUE)) {
				Set_EthCloneMac_Proc(pAd, tmpbuf);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "EthCloneMac="MACSTR"\n",
						 MAC2STR(pAd->EthConvert.EthCloneMac));
			}

#endif /* ETH_CONVERT_SUPPORT */

			/* Beacon Lost Time*/
			if (RTMPGetKeyParameter("BeaconLostTime", tmpbuf, 32, pBuffer, TRUE)) {
				ULONG lInfo = (ULONG)os_str_tol(tmpbuf, 0, 10);

				if ((lInfo != 0) && (lInfo <= 60))
					pAd->StaCfg[0].BeaconLostTime = (lInfo * OS_HZ);

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BeaconLostTime=%ld\n", pAd->StaCfg[0].BeaconLostTime);
			}

			/* Auto Connet Setting if no SSID			*/
			if (RTMPGetKeyParameter("AutoConnect", tmpbuf, 32, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) == 0)
					pAd->StaCfg[0].bAutoConnectIfNoSSID = FALSE;
				else
					pAd->StaCfg[0].bAutoConnectIfNoSSID = TRUE;
			}

#ifdef DOT11R_FT_SUPPORT

			/* FtSupport*/
			if (RTMPGetKeyParameter("FtSupport", tmpbuf, 32, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) == 0)
					pAd->StaCfg[0].Dot11RCommInfo.bFtSupport = FALSE;
				else
					pAd->StaCfg[0].Dot11RCommInfo.bFtSupport = TRUE;

				;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "bFtSupport=%d\n", pAd->StaCfg[0].Dot11RCommInfo.bFtSupport);
			}

#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11Z_TDLS_SUPPORT

			if (RTMPGetKeyParameter("TDLSCapable", tmpbuf, 32, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) != 0) /*Enable*/
					pAd->StaCfg[0].TdlsInfo.bTDLSCapable = TRUE;
				else /*Disable*/
					pAd->StaCfg[0].TdlsInfo.bTDLSCapable = FALSE;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "bTDLSCapable=%d\n", pAd->StaCfg[0].TdlsInfo.bTDLSCapable);
			}

#ifdef TDLS_AUTOLINK_SUPPORT

			if (RTMPGetKeyParameter("TDLS_AutoLink", tmpbuf, 32, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) != 0)  /* Enable */
					pAd->StaCfg[0].TdlsInfo.TdlsAutoLink = TRUE;
				else /* Disable */
					pAd->StaCfg[0].TdlsInfo.TdlsAutoLink = FALSE;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TdlsAutoLink=%d\n", pAd->StaCfg[0].TdlsInfo.TdlsAutoLink);
			}

#endif /* TDLS_AUTOLINK_SUPPORT */

			if (RTMPGetKeyParameter("TDLS_SwitchChSupp", tmpbuf, 32, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) != 0)  /* Enable */
					pAd->StaCfg[0].TdlsInfo.TdlsChSwitchSupp = TRUE;
				else /* Disable */
					pAd->StaCfg[0].TdlsInfo.TdlsChSwitchSupp = FALSE;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TdlsSwitchChSupp=%d\n",
						 pAd->StaCfg[0].TdlsInfo.TdlsChSwitchSupp);
			}

			if (RTMPGetKeyParameter("TDLS_PsmSupp", tmpbuf, 32, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) != 0)  /* Enable */
					pAd->StaCfg[0].TdlsInfo.TdlsPsmSupp = TRUE;
				else /* Disable */
					pAd->StaCfg[0].TdlsInfo.TdlsPsmSupp = FALSE;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TdlsPsmSupp=%d\n", pAd->StaCfg[0].TdlsInfo.TdlsPsmSupp);
			}

			/* APSDAC for AC_BE, AC_BK, AC_VI, AC_VO */
			if (RTMPGetKeyParameter("TDLS_APSDAC", tmpbuf, 32, pBuffer, TRUE)) {
				BOOLEAN FlgIsAcUapsdEnabled[4];

				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					FlgIsAcUapsdEnabled[i] = (BOOLEAN)os_str_tol(macptr, 0, 10);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TDLS APSDAC%d  %d\n", i, FlgIsAcUapsdEnabled[i]);
				}

				pAd->CommonCfg.TDLS_bAPSDAC_BE = FlgIsAcUapsdEnabled[0];
				pAd->CommonCfg.TDLS_bAPSDAC_BK = FlgIsAcUapsdEnabled[1];
				pAd->CommonCfg.TDLS_bAPSDAC_VI = FlgIsAcUapsdEnabled[2];
				pAd->CommonCfg.TDLS_bAPSDAC_VO = FlgIsAcUapsdEnabled[3];
			}

			/* MaxSPLength */
			if (RTMPGetKeyParameter("TDLS_MaxSPLength", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->CommonCfg.TDLS_MaxSPLength = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TDLS MaxSPLength=%d\n", pAd->CommonCfg.TDLS_MaxSPLength);
			}

#endif /* DOT11Z_TDLS_SUPPORT */

			/* FastConnect*/
			if (RTMPGetKeyParameter("FastConnect", tmpbuf, 32, pBuffer, TRUE)) {
				if (os_str_tol(tmpbuf, 0, 10) == 0)
					pAd->StaCfg[0].bFastConnect = FALSE;
				else
					pAd->StaCfg[0].bFastConnect = TRUE;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "FastConnect=%d\n", pAd->StaCfg[0].bFastConnect);
			}
		}
#endif /* CONFIG_STA_SUPPORT */
#ifdef BT_APCLI_SUPPORT
		if (RTMPGetKeyParameter("BTApCliAutoBWSupport", tmpbuf, 128, pBuffer, TRUE)) {
			INT BT_APCLI_Auto_BW_Support = 0;

			BT_APCLI_Auto_BW_Support = os_str_tol(tmpbuf, 0, 10);
			pAd->ApCfg.ApCliAutoBWBTSupport = BT_APCLI_Auto_BW_Support;
		}
#endif
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	if (RTMPGetKeyParameter("ApCliSyncAutoBandSupport", tmpbuf, 128, pBuffer, TRUE)) {
		UCHAR policy = 0, value = 0;
		macptr = rstrtok(tmpbuf, ";");
		for (i = 0; i < DBDC_BAND_NUM; i++) {
			for (policy = 0; (policy < BAND_MAX_POLICY && macptr); macptr = rstrtok(NULL, ";"), policy++) {
				value = (UCHAR)os_str_tol(macptr, 0, 10);
				if (value == 0)
					continue;

				switch (policy) {
				case SAME_BAND_SYNC:
					SET_APCLI_SYNC_BAND_VALID(pAd, policy, i);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Policy %d Band Valid %d\n", policy, IS_APCLI_SYNC_BAND_VALID(pAd, policy, i));
					break;

				case DEAUTH_PEERS:
					SET_APCLI_SYNC_PEER_DEAUTH_VALID(pAd, TRUE, i);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Policy %d Deauth Valid %d\n", policy, IS_APCLI_SYNC_PEER_DEAUTH_VALID(pAd, i));
					break;

				case DIFF_BAND_SYNC:
				case BAND_MAX_POLICY:
				default:
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid Band Policy %d\n", policy);
					break;
				}
			}
			if (!pAd->CommonCfg.dbdc_mode)
					break;
		}
	}
	if (RTMPGetKeyParameter("ApCliSyncAutoBWSupport", tmpbuf, 128, pBuffer, TRUE)) {
		UCHAR policy = 0, value = 0, i = 0, j = 0;
		POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
		struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
		macptr = rstrtok(tmpbuf, ";");

		for (i = 0; i < DBDC_BAND_NUM; i++) {
			for (j = 0; j < pAd->ApCfg.BssidNum; j++) {
				wdev = &pAd->ApCfg.MBSSID[i].wdev;

				if (i == HcGetBandByWdev(wdev))
					break;
			}
		for (policy = 0;  (policy < BW_MAX_POLICY && macptr); macptr = rstrtok(NULL, ";"), policy++) {
				if (IS_APCLI_SYNC_BAND_VALID(pAd, SAME_BAND_SYNC, i) || IS_APCLI_SYNC_BAND_VALID(pAd, DIFF_BAND_SYNC, i)) {
					value = (UCHAR)os_str_tol(macptr, 0, 10);
					if (value == 0)
						continue;

					switch (policy) {
					case HT_2040_UP_ENBL:
					case HT_4020_DOWN_ENBL:
						SET_APCLI_AUTO_BW_HT_VALID(pAd, policy, i);
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"Policy %d HT Valid %d\n", policy, IS_SYNC_BW_POLICY_VALID(pAd, TRUE, policy, i));
						break;

					case VHT_80_2040_DOWN_ENBL:
					case VHT_2040_80_UP_ENBL:
						if (wlan_config_get_vht_bw(wdev) > VHT_BW_2040) {
							SET_APCLI_AUTO_BW_VHT_VALID(pAd, (policy-VHT_POLICY_OFFSET), i);
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								"Policy %d VHT Valid %d\n", policy,
								IS_SYNC_BW_POLICY_VALID(pAd, FALSE, (policy-VHT_POLICY_OFFSET), i));
						} else if (policy == VHT_2040_80_UP_ENBL) {
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Policy VHT 20/40->80 InValid Curret Cap %d\n", wlan_config_get_vht_bw(wdev));
						} else if (policy == VHT_80_2040_DOWN_ENBL) {
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Policy VHT 80->20/40 InValid Curret Cap %d\n", wlan_config_get_vht_bw(wdev));
						}
						break;
					case VHT_160_2040_DOWN_ENBL:
					case VHT_160_80_DOWN_ENBL:
					case VHT_2040_160_UP_ENBL:
					case VHT_80_160_UP_ENBL:
					case BW_MAX_POLICY:
					default:
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Invalid BW Policy  %d\n", policy);
						break;
					}
				}
			}
			if (!pAd->CommonCfg.dbdc_mode)
				break;
		}
		if (pAd->CommonCfg.dbdc_mode && IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd, DBDC_BAND0) && IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd, DBDC_BAND1)) {
			/* Disable 40 MHz Intolerance & BSS Coex Enable Fields */
			pAd->CommonCfg.bBssCoexEnable = FALSE;
			pAd->CommonCfg.bForty_Mhz_Intolerant = FALSE;
		} else if (IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd, DBDC_BAND0)) {
			/* Disable 40 MHz Intolerance & BSS Coex Enable Fields */
			pAd->CommonCfg.bBssCoexEnable = FALSE;
			pAd->CommonCfg.bForty_Mhz_Intolerant = FALSE;
			if (pAd->CommonCfg.dbdc_mode) {
				SET_APCLI_SYNC_BAND_FEATURE_DISABLE(pAd, POLICY_DISABLE, DBDC_BAND1);
				SET_APCLI_SYNC_PEER_DEAUTH_VALID(pAd, POLICY_DISABLE, DBDC_BAND1);
			}
		} else if (pAd->CommonCfg.dbdc_mode && IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd, DBDC_BAND1)) {
			/* Disable 40 MHz Intolerance & BSS Coex Enable Fields */
			pAd->CommonCfg.bBssCoexEnable = FALSE;
			pAd->CommonCfg.bForty_Mhz_Intolerant = FALSE;
			if (pAd->CommonCfg.dbdc_mode) {
				SET_APCLI_SYNC_BAND_FEATURE_DISABLE(pAd, POLICY_DISABLE, DBDC_BAND0);
				SET_APCLI_SYNC_PEER_DEAUTH_VALID(pAd, POLICY_DISABLE, DBDC_BAND0);
			}
		} else {
			/* Disable Feature as Policy not updated */
			SET_APCLI_SYNC_BAND_FEATURE_DISABLE(pAd, POLICY_DISABLE, DBDC_BAND0);
			SET_APCLI_SYNC_PEER_DEAUTH_VALID(pAd, POLICY_DISABLE, DBDC_BAND0);
			if (pAd->CommonCfg.dbdc_mode) {
				SET_APCLI_SYNC_BAND_FEATURE_DISABLE(pAd, POLICY_DISABLE, DBDC_BAND1);
				SET_APCLI_SYNC_PEER_DEAUTH_VALID(pAd, POLICY_DISABLE, DBDC_BAND1);
			}
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Disable Auto Band Policy %d \n", IS_V10_AUTO_BAND_FEATURE_ENBL(pAd));
		}
	}
#endif
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
			read_eaprate_param_from_file(pAd, tmpbuf, pBuffer);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

#ifdef MCAST_RATE_SPECIFIC

			/* McastPhyMode*/
			if (RTMPGetKeyParameter("McastPhyMode", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					UCHAR PhyMode = 0;
					HTTRANSMIT_SETTING *transmit;
					struct wifi_dev *wdev = NULL;
					UCHAR ht_bw = 0;

					if (i >= pAd->ApCfg.BssidNum)
						break;
					wdev = &pAd->ApCfg.MBSSID[i].wdev;
					ht_bw = wlan_config_get_ht_bw(wdev);
					PhyMode = os_str_tol(macptr, 0, 10);
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
					transmit  = (wdev->channel > 14) ?
							 (&wdev->rate.MCastPhyMode_5G) : (&wdev->rate.MCastPhyMode);
#else
					transmit  = &wdev->rate.mcastphymode;
#endif
					transmit->field.BW = ht_bw;

					switch (PhyMode) {
					case MCAST_DISABLE: /* disable */
						NdisMoveMemory(transmit,
									   &pAd->MacTab.Content[MCAST_WCID].HTPhyMode, sizeof(HTTRANSMIT_SETTING));

						if (WMODE_CAP_2G(wdev->PhyMode)) {
							transmit->field.MODE = MODE_CCK;
							transmit->field.BW =  BW_20;
							transmit->field.MCS = RATE_1;
						} else {
							transmit->field.MODE = MODE_OFDM;
							transmit->field.BW =  BW_20;
							transmit->field.MCS = OfdmRateToRxwiMCS[RATE_6];
						}

						break;

					case MCAST_CCK:	/* CCK*/
						if (WMODE_CAP_2G(wdev->PhyMode)) {
							transmit->field.MODE = MODE_CCK;
							transmit->field.BW =  BW_20;
						} else {
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Could not set CCK mode for 5G band so set OFDM!\n");
							transmit->field.MODE = MODE_OFDM;
							transmit->field.BW =  BW_20;
							/* pTransmit->field.MCS = OfdmRateToRxwiMCS[RATE_6]; */
						}

						break;

					case MCAST_OFDM:	/* OFDM*/
						transmit->field.MODE = MODE_OFDM;
						transmit->field.BW =  BW_20;
						break;
#ifdef DOT11_N_SUPPORT

					case MCAST_HTMIX:	/* HTMIX*/
						transmit->field.MODE = MODE_HTMIX;
						break;
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC

					case MCAST_VHT: /* VHT */
						transmit->field.MODE = MODE_VHT;
						break;
#endif /* DOT11_VHT_AC */

					default:
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Unknown Multicast PhyMode %d.\n", PhyMode);
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Set the default mode, MCAST_CCK!\n");
						transmit->field.MODE = MODE_CCK;
						transmit->field.BW =  BW_20;
						break;
					}
				}
			}
			/* McastMcs*/
			if (RTMPGetKeyParameter("McastMcs", tmpbuf, 32, pBuffer, TRUE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					HTTRANSMIT_SETTING *transmit;
					UCHAR Mcs = 0;
					struct wifi_dev *wdev;

					if (i >= pAd->ApCfg.BssidNum)
						break;
					wdev = &pAd->ApCfg.MBSSID[i].wdev;
					Mcs = os_str_tol(macptr, 0, 10);
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
					transmit  = (wdev->channel > 14) ?
						(&wdev->rate.MCastPhyMode_5G) : (&wdev->rate.MCastPhyMode);
#else
					transmit  = &wdev->rate.mcastphymode;
#endif
					switch (transmit->field.MODE) {
					case MODE_CCK:
						if (WMODE_CAP_5G(wdev->PhyMode)) {
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Could not set CCK mode for 5G band!\n");
							break;
						}

						if ((Mcs <= 3) || (Mcs >= 8 && Mcs <= 11)) {
							if (pAd->CommonCfg.TxPreamble == Rt802_11PreambleShort && (Mcs <= 3)) {
								if (Mcs > 0)
									Mcs = Mcs - 1;
								else
									MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
											"Invalid MCS(CCK-1M) value when short preamble is enabled!\n");
							}
							transmit->field.MCS = Mcs;
						}
						else
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "MCS must in range of 0 ~ 3 and 8 ~ 11 for CCK Mode.\n");

						break;

					case MODE_OFDM:
						if (Mcs > 7)
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "MCS must in range from 0 to 7 for OFDM Mode.\n");
						else
							transmit->field.MCS = Mcs;

						break;

					default:
						transmit->field.MCS = Mcs;
						break;
					}
				}
			}
#endif /* MCAST_RATE_SPECIFIC */
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef WIDI_SUPPORT
#ifdef CONFIG_STA_SUPPORT

		/* WIDIEnable */
		if (RTMPGetKeyParameter("WIDIEnable", tmpbuf, 10, pBuffer, TRUE)) {
			if (os_str_tol(tmpbuf, 0, 10) == 0)
				pAd->StaCfg[0].bWIDI = FALSE;
			else
				pAd->StaCfg[0].bWIDI = TRUE;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WIDIEnable=%d\n", pAd->StaCfg[0].bWIDI);
		}

#endif /* CONFIG_STA_SUPPORT */
#endif /* WIDI_SUPPORT */
#ifdef WSC_INCLUDED
		rtmp_read_wsc_user_parms_from_file(pAd, tmpbuf, pBuffer);

		/* Wsc4digitPinCode = TRUE use 4-digit Pin code, otherwise 8-digit Pin code */
		if (RTMPGetKeyParameter("Wsc4digitPinCode", tmpbuf, 32, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					if (os_str_tol(macptr, 0, 10) != 0)	/* Enable */
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscEnrollee4digitPinCode = TRUE;
					else /* Disable */
						pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscEnrollee4digitPinCode = FALSE;

					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "I/F(ra%d) Wsc4digitPinCode=%d\n", i, pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscEnrollee4digitPinCode);
				}
			}
#endif /* CONFIG_AP_SUPPORT // */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				if (os_str_tol(tmpbuf, 0, 10) != 0)	/* Enable */
					pAd->StaCfg[0].wdev.WscControl.WscEnrollee4digitPinCode = TRUE;
				else /* Disable */
					pAd->StaCfg[0].wdev.WscControl.WscEnrollee4digitPinCode = FALSE;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "Wsc4digitPinCode=%d\n", pAd->StaCfg[0].wdev.WscControl.WscEnrollee4digitPinCode);
			}
#endif /* CONFIG_STA_SUPPORT // */
		}

		if (RTMPGetKeyParameter("WscVendorPinCode", tmpbuf, 256, pBuffer, TRUE)) {
			PWSC_CTRL pWscContrl = NULL;
			int bSetOk;
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
			{
#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
					pWscContrl = &pAd->ApCfg.MBSSID[i].wdev.WscControl;
				}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
					pWscContrl = &pAd->StaCfg[0].wdev.WscControl;
				}
#endif /* CONFIG_STA_SUPPORT */

				bSetOk = RT_CfgSetWscPinCode(pAd, macptr, pWscContrl);

				if (bSetOk)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WscVendorPinCode= (%d)\n", bSetOk);
				else
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "WscVendorPinCode: invalid pin code(%s)\n",
							macptr);
			}
		}

#ifdef WSC_V2_SUPPORT

		if (RTMPGetKeyParameter("WscV2Support", tmpbuf, 32, pBuffer, TRUE)) {
			UCHAR			bEnable;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
					if (i >= pAd->ApCfg.BssidNum)
						break;

					bEnable = (UCHAR)os_str_tol(macptr, 0, 10);
					pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl.WscV2Info.bEnableWpsV2 = bEnable;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(ra%d) WscV2Support=%d\n", i, bEnable);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				bEnable = (UCHAR)os_str_tol(tmpbuf, 0, 10);
				pAd->StaCfg[0].wdev.WscControl.WscV2Info.bEnableWpsV2 = bEnable;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WscV2Support= (%d)\n", bEnable);
			}
#endif /* CONFIG_STA_SUPPORT */
		}

#endif /* WSC_V2_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT
		FT_rtmp_read_parameters_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT11R_FT_SUPPORT */
#ifdef OCE_SUPPORT
		Oce_read_parameters_from_file(pAd, tmpbuf, pBuffer);
#endif /* OCE_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

		/* EntryLifeCheck is used to check */
		if (RTMPGetKeyParameter("EntryLifeCheck", tmpbuf, 256, pBuffer, TRUE)) {
			long LifeCheckCnt = os_str_tol(tmpbuf, 0, 10);

			if ((LifeCheckCnt <= 65535) && (LifeCheckCnt != 0))
				pAd->ApCfg.EntryLifeCheck = LifeCheckCnt;
			else
				pAd->ApCfg.EntryLifeCheck = MAC_ENTRY_LIFE_CHECK_CNT;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "EntryLifeCheck=%ld\n", pAd->ApCfg.EntryLifeCheck);
		}

#ifdef DOT11K_RRM_SUPPORT
		RRM_ReadParametersFromFile(pAd, tmpbuf, pBuffer);
#endif /* DOT11K_RRM_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef SINGLE_SKU

		if (RTMPGetKeyParameter("AntGain", tmpbuf, 10, pBuffer, TRUE)) {
			UCHAR AntGain = os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.AntGain = AntGain;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AntGain=%d\n", pAd->CommonCfg.AntGain);
		}

		if (RTMPGetKeyParameter("BandedgeDelta", tmpbuf, 10, pBuffer, TRUE)) {
			UCHAR Bandedge = os_str_tol(tmpbuf, 0, 10);

			pAd->CommonCfg.BandedgeDelta = Bandedge;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BandedgeDelta=%d\n", pAd->CommonCfg.BandedgeDelta);
		}

#endif /* SINGLE_SKU */
#ifdef P2P_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			if (RTMPGetKeyParameter("P2P_GOIntent", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->P2pCfg.GoIntentIdx = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "P2P_GOIntent=%d\n", pAd->P2pCfg.GoIntentIdx);
			}

			if (RTMPGetKeyParameter("P2P_DevName", tmpbuf, 32, pBuffer, TRUE)) {
				if ((strlen(tmpbuf) <= 32)
#ifdef WIDI_SUPPORT
					&& (!pAd->P2pCfg.bWIDI)
#endif /* WIDI_SUPPORT */
				   ) {
					pAd->P2pCfg.DeviceNameLen = (UCHAR) strlen(tmpbuf);
					NdisZeroMemory(pAd->P2pCfg.DeviceName, 32);
					NdisMoveMemory(pAd->P2pCfg.DeviceName, tmpbuf, pAd->P2pCfg.DeviceNameLen);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "P2P_DevName=%s\n", tmpbuf);
				}
			}

			if (RTMPGetKeyParameter("P2P_ListChannel", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->P2pCfg.ListenChannel = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "P2P_ListChannel=%d\n", pAd->P2pCfg.ListenChannel);
			}

			if (RTMPGetKeyParameter("P2P_OpChannel", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->P2pCfg.GroupChannel = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "P2P_OpChannel=%d\n", pAd->P2pCfg.GroupChannel);
			}


			if (RTMPGetKeyParameter("P2P_WscCfgMethod", tmpbuf, 10, pBuffer, TRUE)) {
				UCHAR CfgMethod = os_str_tol(tmpbuf, 0, 10);

				if (pAd->P2pCfg.WscMode == WSC_PIN_MODE) {
					if (CfgMethod == P2P_REG_CM_DISPLAY)
						pAd->P2pCfg.DefaultConfigMethod = P2P_REG_CM_DISPLAY;
					else if (CfgMethod == P2P_REG_CM_KEYPAD)
						pAd->P2pCfg.DefaultConfigMethod = P2P_REG_CM_KEYPAD;
				} else if (pAd->P2pCfg.WscMode == WSC_PBC_MODE)
					pAd->P2pCfg.DefaultConfigMethod = P2P_REG_CM_PBC;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "P2P_WscCfgMethod=%d\n", pAd->P2pCfg.DefaultConfigMethod);
			}

			if (RTMPGetKeyParameter("P2P_ExtListenEnable", tmpbuf, 10, pBuffer, TRUE)) {
				UINT32 ExtList = os_str_tol(tmpbuf, 0, 10);

				if (ExtList == 0)
					pAd->P2pCfg.bExtListen = FALSE;
				else if (ExtList == 1)
					pAd->P2pCfg.bExtListen = TRUE;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "P2P_ExtListen=%d\n", pAd->P2pCfg.bExtListen);
			}

			if (RTMPGetKeyParameter("P2P_ExtListenPrd", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->P2pCfg.ExtListenPeriod = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "P2P_ExtListenPrd=%d\n", pAd->P2pCfg.ExtListenPeriod);
			}

			if (RTMPGetKeyParameter("P2P_ExtenListenInv", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->P2pCfg.ExtListenInterval = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "P2P_ExtenListenInv=%d\n", pAd->P2pCfg.ExtListenInterval);
			}

			if (RTMPGetKeyParameter("P2P_IntraBss", tmpbuf, 10, pBuffer, TRUE)) {
				UINT32 IntraBss = os_str_tol(tmpbuf, 0, 10);

				if (IntraBss == 0)
					pAd->P2pCfg.bIntraBss = FALSE;
				else if (IntraBss == 1)
					pAd->P2pCfg.bIntraBss = TRUE;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "P2P_IntraBss=%d\n", pAd->P2pCfg.bIntraBss);
			}

			if (RTMPGetKeyParameter("P2P_NoACnt", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->P2pCfg.GONoASchedule.Count = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "P2P_NoACnt=%d\n", pAd->P2pCfg.GONoASchedule.Count);
			}

			if (RTMPGetKeyParameter("P2P_NoADuration", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->P2pCfg.GONoASchedule.Duration = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "P2P_NoADuration=%ld\n", pAd->P2pCfg.GONoASchedule.Duration);
			}

			if (RTMPGetKeyParameter("P2P_NoAInv", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->P2pCfg.GONoASchedule.Interval = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "P2P_NoAInv=%d\n", pAd->P2pCfg.GONoASchedule.Interval);
			}

			if (RTMPGetKeyParameter("P2P_MaxEntry", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->P2pCfg.p2pMaxEntry = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "P2P_MaxEntry=%d\n", pAd->P2pCfg.p2pMaxEntry);
			}

#ifdef WIDI_SUPPORT

			if (RTMPGetKeyParameter("P2P_L2SD_SCAN_TOGGLE", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->p2p_l2sd_scan_toggle = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "P2P_L2SD_SCAN_TOGGLE=%d\n", pAd->p2p_l2sd_scan_toggle);
			}

			if (RTMPGetKeyParameter("P2P_WIDIEnable", tmpbuf, 10, pBuffer, TRUE)) {
				pAd->P2pCfg.bWIDI = os_str_tol(tmpbuf, 0, 10);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "P2P_WIDIEnable=%d\n", pAd->P2pCfg.bWIDI);
			}

#endif /* WIDI_SUPPORT */
		}
#endif /* P2P_SUPPORT */
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)

		/* set GPIO pin for wake-up signal */
		if (RTMPGetKeyParameter("WOW_GPIO", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_GPIO(pAd, tmpbuf);

		/* set WOW enable/disable */
		if (RTMPGetKeyParameter("WOW_Enable", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Enable(pAd, tmpbuf);

		/* set delay time for WOW really enable */
		if (RTMPGetKeyParameter("WOW_Delay", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Delay(pAd, tmpbuf);

		/* set GPIO pulse hold time */
		if (RTMPGetKeyParameter("WOW_Hold", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Hold(pAd, tmpbuf);

		/* set wakeup signal type */
		if (RTMPGetKeyParameter("WOW_InBand", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_InBand(pAd, tmpbuf);

		/* set wakeup interface */
		if (RTMPGetKeyParameter("WOW_Interface", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_Interface(pAd, tmpbuf);

		/* set if down interface */
		if (RTMPGetKeyParameter("WOW_IfDown_Support", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_IfDown_Support(pAd, tmpbuf);

		/* set GPIO High Low */
		if (RTMPGetKeyParameter("WOW_GPIOHighLow", tmpbuf, 10, pBuffer, TRUE))
			Set_WOW_GPIOHighLow(pAd, tmpbuf);

#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT) */
#ifdef MICROWAVE_OVEN_SUPPORT

		if (RTMPGetKeyParameter("MO_FalseCCATh", tmpbuf, 10, pBuffer, TRUE))
			Set_MO_FalseCCATh_Proc(pAd, tmpbuf);

#endif /* MICROWAVE_OVEN_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#ifdef SNIFFER_SUPPORT

		if (RTMPGetKeyParameter("SnifferType", tmpbuf, 10, pBuffer, TRUE)) {
			pAd->sniffer_ctl.sniffer_type = os_str_tol(tmpbuf, 0, 10);
			set_sniffer_mode(pAd->StaCfg[0].wdev.if_dev, pAd->sniffer_ctl.sniffer_type);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SnifferType = %d\n", pAd->sniffer_ctl.sniffer_type);
		}

#endif /* SNIFFER_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

		if (RTMPGetKeyParameter("PS_RETRIEVE", tmpbuf, 10, pBuffer, TRUE)) {
			long PS_RETRIEVE;

			PS_RETRIEVE = os_str_tol(tmpbuf, 0, 10);
			pAd->bPS_Retrieve = PS_RETRIEVE;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PS_RETRIEVE = %lx\n", PS_RETRIEVE);
		}

#ifdef FW_DUMP_SUPPORT

		if (RTMPGetKeyParameter("FWDump_Path", tmpbuf, 10, pBuffer, TRUE))
			set_fwdump_path(pAd, tmpbuf);

		if (RTMPGetKeyParameter("FWDump_MaxSize", tmpbuf, 10, pBuffer, TRUE))
			set_fwdump_max_size(pAd, tmpbuf);

#endif
#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)

		if (RTMPGetKeyParameter("IcapMode", tmpbuf, 10, pBuffer, TRUE)) {
			UINT8 ICapMode; /* 0 : Normal Mode; 1 : Internal Capture; 2 : Wifi Spectrum */

			ICapMode = simple_strtol(tmpbuf, 0, 10);
			pAd->ICapMode = ICapMode;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ICapMode = %d\n", ICapMode);
		}

#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */
#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		rtmp_read_vow_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* CONFIG_AP_SUPPORT */
#endif /* VOW_SUPPORT */
#ifdef FW_LOG_DUMP
		rtmp_read_fw_log_dump_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* FW_LOG_DUMP */
#ifdef RED_SUPPORT
		rtmp_read_red_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* RED_SUPPORT */
#ifdef FQ_SCH_SUPPORT
		rtmp_read_fq_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* FQ_SCH_SUPPORT */
		rtmp_read_cp_parms_from_file(pAd, tmpbuf, pBuffer);
		rtmp_read_multi_cli_nums_eap_th_parms_from_file(pAd, tmpbuf, pBuffer);
#ifdef MGMT_TXPWR_CTRL
		rtmp_read_mgmt_pwr_parms_from_file(pAd, tmpbuf, pBuffer);
#endif
#ifdef DATA_TXPWR_CTRL
		if (RTMPGetKeyParameter("DataTxPwr", tmpbuf, 32, pBuffer, TRUE))
			rtmp_read_data_pwr_parms_from_file(pAd, tmpbuf);
#endif
#ifdef KERNEL_RPS_ADJUST
		rtmp_read_kernel_rps_parms_from_file(pAd, tmpbuf, pBuffer);
#endif
		rtmp_read_retry_parms_from_file(pAd, tmpbuf, pBuffer);
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11V_MBSSID_SUPPORT
	if (RTMPGetKeyParameter("Dot11vMbssid", tmpbuf, 100, pBuffer, TRUE))
		rtmp_read_dot11v_mbssid_cfg_from_file(pAd, tmpbuf);
#endif /* DOT11V_MBSSID_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_6G_SUPPORT
	rtmp_read_6g_discov_cfg_from_file(pAd, tmpbuf, pBuffer);
#endif
#ifdef CFG_SUPPORT_FALCON_MURU
	if (RTMPGetKeyParameter("TamArbOpMode", tmpbuf, 32, pBuffer, TRUE)) {
		pAd->CommonCfg.TamArbOpMode = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"TamArbOpMode = %ld\n", pAd->CommonCfg.TamArbOpMode);
	}

	if (RTMPGetKeyParameter("HE_PpduFmt", tmpbuf, 32, pBuffer, TRUE)) {
		pAd->CommonCfg.HE_PpduFmt = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"HE_PpduFmt = %ld\n", pAd->CommonCfg.HE_PpduFmt);
	}

	if (RTMPGetKeyParameter("HE_OfdmaSchType", tmpbuf, 32, pBuffer, TRUE)) {
		pAd->CommonCfg.HE_OfdmaSchType = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"HE_OfdmaSchType = %ld\n", pAd->CommonCfg.HE_OfdmaSchType);
	}

	if (RTMPGetKeyParameter("HE_OfdmaUserNum", tmpbuf, 32, pBuffer, TRUE)) {
		pAd->CommonCfg.HE_OfdmaUserNum = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"HE_OfdmaUserNum = %ld\n", pAd->CommonCfg.HE_OfdmaUserNum);
	}

	if (RTMPGetKeyParameter("HE_TrigPadding", tmpbuf, 32, pBuffer, TRUE)) {
		pAd->CommonCfg.HE_TrigPadding = os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"HE_TrigPadding = %ld\n", pAd->CommonCfg.HE_TrigPadding);
	}
#endif
#ifdef CFG_SUPPORT_FALCON_SR
	/* Spatial Reuse  */
	if (RTMPGetKeyParameter("SREnable", tmpbuf, 32, pBuffer, TRUE))
		SrProfileSREnable(pAd, tmpbuf);
	if (RTMPGetKeyParameter("SRMode", tmpbuf, 32, pBuffer, TRUE))
		SrProfileSRMode(pAd, tmpbuf);
	if (RTMPGetKeyParameter("SRSDEnable", tmpbuf, 32, pBuffer, TRUE))
		SrProfileSRSDEnable(pAd, tmpbuf);
	if (RTMPGetKeyParameter("SRDPDEnable", tmpbuf, 32, pBuffer, TRUE))
		SrProfileSRDPDEnable(pAd, tmpbuf);
	if (RTMPGetKeyParameter("SRDropMinMcs", tmpbuf, 32, pBuffer, TRUE))
		SrProfileSRDropMinMCS(pAd, tmpbuf);
	if (RTMPGetKeyParameter("SRDPDThreshold", tmpbuf, 32, pBuffer, TRUE))
		SrProfileSRDPDThreshold(pAd, tmpbuf);
#endif /* CFG_SUPPORT_FALCON_SR */

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_HE_AX
	if (RTMPGetKeyParameter("BSSColorValue", tmpbuf, 32, pBuffer, TRUE))
		bss_color_profile_enable(pAd, tmpbuf);
#endif /* DOT11_HE_AX */
#endif

#ifdef CFG_SUPPORT_FALCON_PP
	/* Preamble puncture */
	if (RTMPGetKeyParameter("PPEnable", tmpbuf, 32, pBuffer, TRUE))
		pp_profile_pp_en(pAd, tmpbuf);
#endif /* CFG_SUPPORT_FALCON_PP */

#ifdef ANTENNA_CONTROL_SUPPORT
		rtmp_read_ant_ctrl_parms_from_file(pAd, tmpbuf, pBuffer);
#endif /* ANTENNA_CONTROL_SUPPORT */
#ifdef ANDLINK_FEATURE_SUPPORT
	rtmp_read_andlink_parms_from_file(pAd, tmpbuf, pBuffer);
#endif/*ANDLINK_FEATURE_SUPPORT*/
#ifdef ACK_CTS_TIMEOUT_SUPPORT
	rtmp_read_ackcts_timeout_parms_from_file(pAd, tmpbuf, pBuffer);
#endif
#ifdef MLME_MULTI_QUEUE_SUPPORT
	rtmp_read_mlme_multiqueue_parms_from_file(pAd, tmpbuf, pBuffer);
#endif
	rtmp_read_quick_channel_switch_parms_from_file(pAd, tmpbuf, pBuffer);
#ifdef CONFIG_CPE_SUPPORT
	rtmp_read_powerBackoff_parms_from_file(pAd, tmpbuf, pBuffer);
#endif
#ifdef CONFIG_3_WIRE_SUPPORT
	read_3wireFunction_enable(pAd, tmpbuf, pBuffer);
#endif
	if (RTMPGetKeyParameter("Wifi6GCap", tmpbuf, 10, pBuffer, TRUE)) {
		/* 0 : not support; 1 : WiFi 6E support */
		pAd->Wifi6gCap = simple_strtol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Wifi6gCap = %d\n", pAd->Wifi6gCap);
	}

#ifdef WIFI_MD_COEX_SUPPORT
	rtmp_read_coex_param_from_file(pAd, tmpbuf, pBuffer);
#endif
	} while (0);

	os_free_mem(tmpbuf);
	return NDIS_STATUS_SUCCESS;
}

#ifdef WSC_INCLUDED
void rtmp_read_wsc_user_parms(
	PWSC_CTRL pWscControl,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	INT		i = 0;
	RTMP_STRING *macptr = NULL;
	ULONG len;
	if (RTMPGetKeyParameter("WscManufacturer", tmpbuf, WSC_MANUFACTURE_LEN, buffer, TRUE))
	{
		len = strlen(tmpbuf);
		if (len > WSC_MANUFACTURE_LEN) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "input parameter[WscManufacturer] incorrect\n");
			return;
		}

		NdisZeroMemory(pWscControl->RegData.SelfInfo.Manufacturer, WSC_MANUFACTURE_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.Manufacturer, tmpbuf, len);

		if (pWscControl->RegData.SelfInfo.Manufacturer[0] != 0x00)
			RTMP_SET_FLAG(pWscControl, 0x01);
	}

	/*WSC_User_ModelName*/
	if (RTMPGetKeyParameter("WscModelName", tmpbuf, WSC_MODELNAME_LEN, buffer, TRUE))
	{
		len = strlen(tmpbuf);
		if (len > WSC_MODELNAME_LEN) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "input parameter[WscModelName] incorrect\n");
			return;
		}

		NdisZeroMemory(pWscControl->RegData.SelfInfo.ModelName, WSC_MODELNAME_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.ModelName, tmpbuf, len);

		if (pWscControl->RegData.SelfInfo.ModelName[0] != 0x00)
			RTMP_SET_FLAG(pWscControl, 0x02);
	}

	/*WSC_User_DeviceName*/
	if (RTMPGetKeyParameter("WscDeviceName", tmpbuf, 256, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
		{
			if (pWscControl->WpsApBand == i)
			{
				len = strlen(macptr);
				if (len > WSC_DEVICENAME_LEN) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "input parameter[WscDeviceName] incorrect\n"
						 );
					return;
				}

				NdisZeroMemory(pWscControl->RegData.SelfInfo.DeviceName, WSC_DEVICENAME_LEN);
				NdisMoveMemory(pWscControl->RegData.SelfInfo.DeviceName, macptr, len);

				if (pWscControl->RegData.SelfInfo.DeviceName[0] != 0x00)
					RTMP_SET_FLAG(pWscControl, 0x04);
			}
		}
	}

	/*WSC_User_ModelNumber*/
	if (RTMPGetKeyParameter("WscModelNumber", tmpbuf, WSC_MODELNUNBER_LEN, buffer, TRUE))
	{
		len = strlen(tmpbuf);
		if (len > WSC_MODELNUNBER_LEN) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "input parameter[WscModelNumber] incorrect\n");
			return;
		}

		NdisZeroMemory(pWscControl->RegData.SelfInfo.ModelNumber, WSC_MODELNUNBER_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.ModelNumber, tmpbuf, len);

		if (pWscControl->RegData.SelfInfo.ModelNumber[0] != 0x00)
			RTMP_SET_FLAG(pWscControl, 0x08);
	}

	/*WSC_User_SerialNumber*/
	if (RTMPGetKeyParameter("WscSerialNumber", tmpbuf, WSC_SERIALNUNBER_LEN, buffer, TRUE))
	{
		len = strlen(tmpbuf);
		if (len > WSC_SERIALNUNBER_LEN) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "input parameter[WscSerialNumber] incorrect\n");
			return;
		}

		NdisZeroMemory(pWscControl->RegData.SelfInfo.SerialNumber, WSC_SERIALNUNBER_LEN);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.SerialNumber, tmpbuf, len);

		if (pWscControl->RegData.SelfInfo.SerialNumber[0] != 0x00)
			RTMP_SET_FLAG(pWscControl, 0x10);
	}
}

void rtmp_read_wsc_user_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	PWSC_CTRL           pWscControl;
#ifdef WSC_AP_SUPPORT
	int i = 0;

	for (i = 0; i < MAX_MBSSID_NUM(pAd); i++) {
		pWscControl = &pAd->ApCfg.MBSSID[PF_TO_BSS_IDX(pAd, i)].wdev.WscControl;
		pWscControl->WpsApBand = PF_TO_BSS_IDX(pAd, i);
		rtmp_read_wsc_user_parms(pWscControl, tmpbuf, buffer);
	}

#ifdef APCLI_SUPPORT
	pWscControl = &pAd->StaCfg[0].wdev.WscControl;
	rtmp_read_wsc_user_parms(pWscControl, tmpbuf, buffer);
#endif /* APCLI_SUPPORT */
#endif /* WSC_AP_SUPPORT */
#ifdef WSC_STA_SUPPORT
	pWscControl = &pAd->StaCfg[0].wdev.WscControl;
	rtmp_read_wsc_user_parms(pWscControl, tmpbuf, buffer);
#endif /* WSC_STA_SUPPORT */
#ifdef P2P_SUPPORT

	if (RTMPGetKeyParameter("WscManufacturer", tmpbuf, WSC_MANUFACTURE_LEN, buffer, TRUE)) {
		NdisMoveMemory(pAd->P2pCfg.Manufacturer, tmpbuf, strlen(tmpbuf));
		pAd->P2pCfg.ManufacturerLen = strlen(tmpbuf);

		if (!pAd->P2pCfg.ManufacturerLen) {
			pAd->P2pCfg.Manufacturer[0] = 0x20;
			pAd->P2pCfg.ManufacturerLen = 0x01;
		}
	}

	/*WSC_User_ModelName*/
	if (RTMPGetKeyParameter("WscModelName", tmpbuf, WSC_MODELNAME_LEN, buffer, TRUE)) {
		NdisMoveMemory(pAd->P2pCfg.ModelName, tmpbuf, strlen(tmpbuf));
		pAd->P2pCfg.ModelNameLen = strlen(tmpbuf);

		if (!pAd->P2pCfg.ModelNameLen) {
			pAd->P2pCfg.ModelName[0] = 0x20;
			pAd->P2pCfg.ModelNameLen = 0x01;
		}
	}

	/* Already has P2P_DevName parameter, skip this*/

	/*WSC_User_ModelNumber*/
	if (RTMPGetKeyParameter("WscModelNumber", tmpbuf, WSC_MODELNUNBER_LEN, buffer, TRUE)) {
		NdisMoveMemory(pAd->P2pCfg.ModelNumber, tmpbuf, strlen(tmpbuf));
		pAd->P2pCfg.ModelNumberLen = strlen(tmpbuf);

		if (!pAd->P2pCfg.ModelNumberLen) {
			pAd->P2pCfg.ModelNumber[0] = 0x20;
			pAd->P2pCfg.ModelNumberLen = 0x01;
		}
	}

	/*WSC_User_SerialNumber*/
	if (RTMPGetKeyParameter("WscSerialNumber", tmpbuf, WSC_SERIALNUNBER_LEN, buffer, TRUE)) {
		NdisMoveMemory(pAd->P2pCfg.SerialNumber, tmpbuf, strlen(tmpbuf));
		pAd->P2pCfg.SerialNumberLen = strlen(tmpbuf);

		if (!pAd->P2pCfg.SerialNumberLen) {
			pAd->P2pCfg.SerialNumber[0] = 0x20;
			pAd->P2pCfg.SerialNumberLen = 0x01;
		}
	}

#endif /* P2P_SUPPORT */
}
#endif/*WSC_INCLUDED*/

#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
void rtmp_read_vow_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	UINT16		i = 0, j = 0, n;
	CHAR		*ptok = NULL;
	CHAR		*macptr;
	CHAR		*tmp = NULL;
	CHAR		*pwatf_string = NULL;
	UINT16 		wtbl_max_num = WTBL_MAX_NUM(pAd);
	LONG 		tok;
	int ret;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "begin -->\n");
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	/* for enable/disable */
	if (RTMPGetKeyParameter("VOW_BW_Ctrl", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		tok = os_str_tol(tmpbuf, 0, 10);
		pAd->vow_cfg.en_bw_ctrl = (tok != 0) ? TRUE : FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_BW_Ctrl --> %d\n",
				 pAd->vow_cfg.en_bw_ctrl);
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Airtime_Fairness_En", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.en_airtime_fairness =  os_str_tol(tmpbuf, 0, 10) != 0 ? TRUE : FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_Airtime_Fairness_En --> %d\n",
				 pAd->vow_cfg.en_airtime_fairness);
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	n = 0;

	if (RTMPGetKeyParameter("VOW_Airtime_Ctrl_En", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_GROUP_TABLE_MAX)
				break;

			if (n >= VOW_MAX_GROUP_NUM)
				break;

			tok = os_str_tol(ptok, 0, 10);

			if ((!(pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG)) || (tok != 0)) {
				pAd->vow_bss_cfg[n].at_on = (tok != 0) ? TRUE : FALSE;
				pAd->vow_bss_cfg[n].group_table_idx = i;
				pAd->bss_group.bw_group_idx[i] = n;

				MTWF_PRINT("BWCG(%d) idx:%u VOW_Airtime_Ctrl_En --> %d\n",
					 n, i, pAd->vow_bss_cfg[n].at_on);
				n++;
			}
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	n = 0;
	if (RTMPGetKeyParameter("VOW_Rate_Ctrl_En", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_GROUP_TABLE_MAX)
				break;

			if (n >= VOW_MAX_GROUP_NUM)
				break;

			tok = os_str_tol(ptok, 0, 10);
			if ((!(pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG)) || (tok != 0)) {
				pAd->vow_bss_cfg[n].bw_on = (tok != 0) ? TRUE : FALSE;
				pAd->vow_bss_cfg[n].group_table_idx = i;
				pAd->bss_group.bw_group_idx[i] = n;
				MTWF_PRINT("BWCG(%d) idx:%u VOW_Rate_Ctrl_En --> %d\n",
					 n, i, pAd->vow_bss_cfg[n].bw_on);
				n++;
			}

		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_RX_En", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_rx_time_cfg.rx_time_en =  os_str_tol(tmpbuf, 0, 10) != 0 ? TRUE : FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_RX_En --> %d\n",
				 pAd->vow_rx_time_cfg.rx_time_en);
	}

	/* for gorup setting */
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Min_Rate", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].min_rate =  (UINT16)os_str_tol(ptok, 0, 10);
			pAd->vow_bss_cfg[i].min_rate_token = vow_convert_rate_token(pAd, VOW_MIN, i);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS(%d) VOW_Group_Min_Rate --> %d\n",
					 i, pAd->vow_bss_cfg[i].min_rate);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Max_Rate", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].max_rate =  (UINT16)os_str_tol(ptok, 0, 10);
			pAd->vow_bss_cfg[i].max_rate_token = vow_convert_rate_token(pAd, VOW_MAX, i);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS(%d) VOW_Group_Max_Rate --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_rate);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Min_Ratio", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].min_airtime_ratio =  (UINT8)os_str_tol(ptok, 0, 10);
			pAd->vow_bss_cfg[i].min_airtime_token = vow_convert_airtime_token(pAd, VOW_MIN, i);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS(%d) VOW_Group_Min_Ratio --> %d\n",
					 i, pAd->vow_bss_cfg[i].min_airtime_ratio);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Max_Ratio", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].max_airtime_ratio =  (UINT8)os_str_tol(ptok, 0, 10);
			pAd->vow_bss_cfg[i].max_airtime_token = vow_convert_airtime_token(pAd, VOW_MAX, i);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS(%d) VOW_Group_Max_Ratio --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_airtime_ratio);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Refill_Period", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.refill_period =  (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_Refill_Period --> %d\n",
				 pAd->vow_cfg.refill_period);
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Min_Rate_Bucket_Size", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].min_ratebucket_size =  (UINT16)os_str_tol(ptok, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS(%d) VOW_Group_Min_Rate_Bucket_Size --> %d\n",
					 i, pAd->vow_bss_cfg[i].min_ratebucket_size);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Max_Rate_Bucket_Size", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].max_ratebucket_size =  (UINT16)os_str_tol(ptok, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS(%d) VOW_Group_Max_Rate_Bucket_Size --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_ratebucket_size);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Min_Airtime_Bucket_Size", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].min_airtimebucket_size =  (UINT8)os_str_tol(ptok, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS(%d) VOW_Group_Min_Airtime_Bucket_Size --> %d\n",
					 i, pAd->vow_bss_cfg[i].min_airtimebucket_size);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Max_Airtime_Bucket_Size", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].max_airtimebucket_size = (UINT8)os_str_tol(ptok, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS(%d) VOW_Group_Max_Airtime_Bucket_Size --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_airtimebucket_size);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Backlog", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].max_backlog_size = (UINT16)os_str_tol(ptok, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS(%d) VOW_Group_Backlog --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_backlog_size);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_Max_Wait_Time", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].max_wait_time = (UINT8)os_str_tol(ptok, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS(%d) VOW_Group_Max_Wait_Time --> %d\n",
					 i, pAd->vow_bss_cfg[i].max_wait_time);
		}
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_DWRR_Quantum", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ";"); ptok; ptok = rstrtok(NULL, ";"), i++) {
			if (i >= VOW_MAX_GROUP_NUM)
				break;

			pAd->vow_bss_cfg[i].dwrr_quantum =  (UINT8)os_str_tol(ptok, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS(%d) VOW_Group_DWRR_Quantum --> %d\n",
					 i, pAd->vow_bss_cfg[i].dwrr_quantum);
		}
	}

	/* for stations */
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Sta_VO_DWRR_Quantum", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0; i < wtbl_max_num; i++)
			pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_VO] = (UINT8)os_str_tol(tmpbuf, 0, 10);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_Sta_VO_DWRR_Quantum --> %d\n",
				 (UINT8)os_str_tol(tmpbuf, 0, 10));
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Sta_VI_DWRR_Quantum", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0; i < wtbl_max_num; i++)
			pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_VI] = (UINT8)os_str_tol(tmpbuf, 0, 10);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_Sta_VI_DWRR_Quantum --> %d\n",
				 (UINT8)os_str_tol(tmpbuf, 0, 10));
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Sta_BE_DWRR_Quantum", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0; i < wtbl_max_num; i++)
			pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_BE] = (UINT8)os_str_tol(tmpbuf, 0, 10);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_Sta_BE_DWRR_Quantum --> %d\n",
				 (UINT8)os_str_tol(tmpbuf, 0, 10));
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Sta_BK_DWRR_Quantum", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0; i < wtbl_max_num; i++)
			pAd->vow_sta_cfg[i].dwrr_quantum[WMM_AC_BK] = (UINT8)os_str_tol(tmpbuf, 0, 10);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_Sta_BK_DWRR_Quantum --> %d\n",
				 (UINT8)os_str_tol(tmpbuf, 0, 10));
	}

	/* for group/stations control */
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_WMM_Search_Rule_Band0", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.dbdc0_search_rule =  os_str_tol(tmpbuf, 0, 10) ? 1 : 0;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_WMM_Search_Rule_Band0 --> %d\n",
				 pAd->vow_cfg.dbdc0_search_rule);
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_WMM_Search_Rule_Band1", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.dbdc1_search_rule =  os_str_tol(tmpbuf, 0, 10) ? 1 : 0;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_WMM_Search_Rule_Band1 --> %d\n",
				 pAd->vow_cfg.dbdc1_search_rule);
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Sta_DWRR_Max_Wait_Time", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.sta_max_wait_time =  (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_Sta_DWRR_Max_Wait_Time --> %d\n",
				 pAd->vow_cfg.sta_max_wait_time);
	}

	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_Group_DWRR_Max_Wait_Time", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.group_max_wait_time =  (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_Group_DWRR_Max_Wait_Time --> %d\n",
				 pAd->vow_cfg.group_max_wait_time);
	}

	/* Weigthed Airtime Fairness - Enable/Disable*/
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("VOW_WATF_Enable", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_watf_en =  (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_WATF_Enable --> %d\n", pAd->vow_watf_en);
	}

	if (pAd->vow_watf_en) {
		/* Weigthed Airtime Fairness - Different DWRR quantum value*/
		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

		if (RTMPGetKeyParameter("VOW_WATF_Q_LV0", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
			pAd->vow_watf_q_lv0 = (UINT8)os_str_tol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[0] = pAd->vow_watf_q_lv0;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_WATF_Q_LV0 --> %d\n", pAd->vow_watf_q_lv0);
		} else
			pAd->vow_watf_q_lv0 = 4;

		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

		if (RTMPGetKeyParameter("VOW_WATF_Q_LV1", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
			pAd->vow_watf_q_lv1 = (UINT8)os_str_tol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[1] = pAd->vow_watf_q_lv1;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_WATF_Q_LV1 --> %d\n", pAd->vow_watf_q_lv1);
		} else
			pAd->vow_watf_q_lv1 = 8;

		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

		if (RTMPGetKeyParameter("VOW_WATF_Q_LV2", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
			pAd->vow_watf_q_lv2 = (UINT8)os_str_tol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[2] = pAd->vow_watf_q_lv2;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_WATF_Q_LV2 --> %d\n", pAd->vow_watf_q_lv2);
		} else
			pAd->vow_watf_q_lv2 = 12;

		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

		if (RTMPGetKeyParameter("VOW_WATF_Q_LV3", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
			pAd->vow_watf_q_lv3 = (UINT8)os_str_tol(tmpbuf, 0, 10);
			pAd->vow_cfg.vow_sta_dwrr_quantum[3] = pAd->vow_watf_q_lv3;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_WATF_Q_LV3 --> %d\n", pAd->vow_watf_q_lv3);
		} else
			pAd->vow_watf_q_lv3 = 16;

		/* Weigthed Airtime Fairness - Different DWRR quantum MAC address list*/
		NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
		os_alloc_mem(NULL, (UCHAR **)&pwatf_string, 32);
		if(pwatf_string == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "alloc failed, pwatf_string = NULL\n");
			return;
		}

		os_alloc_mem(NULL, (UCHAR **)&tmp, 32);
		if (tmp == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "alloc failed, tmp = NULL\n");
			os_free_mem(pwatf_string);
			return;
		}
		os_zero_mem(tmp, 32);
		for (i = 0; i < VOW_WATF_LEVEL_NUM; i++) {
			ret = snprintf(pwatf_string, 32, "VOW_WATF_MAC_LV%d", i);
			if (os_snprintf_error(32, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: snprintf error!\n", __func__);
				os_free_mem(pwatf_string);
				os_free_mem(tmp);
				return;
			}

			if (RTMPGetKeyParameter(pwatf_string, tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (pAd->vow_watf_en)) {
				ULONG str_tem_len;
				for (j = 0, macptr = rstrtok(tmpbuf, ","); macptr; macptr = rstrtok(NULL, ","), j++) {
					if (strlen(macptr) != 17)  /* Mac address acceptable format 01:02:03:04:05:06 length 17*/
						continue;
					str_tem_len = 32 - strlen(tmp);
					ret = snprintf(tmp + strlen(tmp), str_tem_len, "%d-%s", i, macptr);
					if (os_snprintf_error(str_tem_len, ret)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: snprintf error!\n", __func__);
						os_free_mem(pwatf_string);
						os_free_mem(tmp);
						return;
					}
					set_vow_watf_add_entry(pAd, tmp);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%d-%s", i, macptr);
				}
			}
		}

		if (pwatf_string != NULL)
			os_free_mem(pwatf_string);

		if (tmp != NULL)
			os_free_mem(tmp);
	}

	/* fast round robin */
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);
	if (RTMPGetKeyParameter("VOW_STA_FRR_QUANTUM", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_sta_frr_quantum =  (UINT8)simple_strtol(tmpbuf, 0, 10);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"VOW_STA_FRR_QUANTUM --> %d\n", pAd->vow_sta_frr_quantum);
	}

	if (RTMPGetKeyParameter("VOW_Schedule_Type", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		if ((strcmp(tmpbuf, "HW") == 0) || (strcmp(tmpbuf, "Hw") == 0) || (strcmp(tmpbuf, "hw") == 0))
			pAd->vow_sch_cfg.sch_type = (UINT8)VOW_SCH_FOLLOW_HW;
		else
			pAd->vow_sch_cfg.sch_type = (UINT8)VOW_SCH_FOLLOW_ALGO;
		/* use mcli_sch_cfg.sch_type to record default setting */
		pAd->vow_cfg.mcli_sch_cfg.sch_type = pAd->vow_sch_cfg.sch_type;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"VOW_Schedule_Type --> %u\n", pAd->vow_sch_cfg.sch_type);
	}

	if (RTMPGetKeyParameter("VOW_Schedule_Policy", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		if ((strcmp(tmpbuf, "WRR") == 0) || (strcmp(tmpbuf, "Wrr") == 0) || (strcmp(tmpbuf, "wrr") == 0))
			pAd->vow_sch_cfg.sch_policy = (UINT8)VOW_SCH_POL_WRR;
		else
			pAd->vow_sch_cfg.sch_policy = (UINT8)VOW_SCH_POL_SRR;
		/* use mcli_sch_cfg.sch_policy to record default setting */
		pAd->vow_cfg.mcli_sch_cfg.sch_policy = pAd->vow_sch_cfg.sch_policy;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"VOW_Schedule_Policy --> %u\n", pAd->vow_sch_cfg.sch_policy);
	}

	if (RTMPGetKeyParameter("VOW_Mcli_Schedule_Enable", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->vow_cfg.mcli_schedule_en =  (UINT8)os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "VOW_Mcli_Balance_Enable --> %d\n", pAd->vow_cfg.mcli_schedule_en);
	}

}
#endif /* CONFIG_AP_SUPPORT */
#endif  /*  VOW_SUPPORT */

#ifdef RED_SUPPORT
void rtmp_read_red_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "begin -->\n");
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	/* for enable/disable */
	if (RTMPGetKeyParameter("RED_Enable", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->red_en =  os_str_tol(tmpbuf, 0, 10) != 0 ? TRUE : FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RED_Enable --> %d\n", pAd->red_en);
	}
}
#endif  /*  RED_SUPPORT */

#ifdef FQ_SCH_SUPPORT
void rtmp_read_fq_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "begin -->\n");
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	/* for enable/disable */
	if (RTMPGetKeyParameter("FQ_Enable", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		if (!(pAd->fq_ctrl.enable & FQ_READY))
			pAd->fq_ctrl.enable = os_str_tol(tmpbuf, 0, 10) != 0 ? FQ_NEED_ON : 0;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "FQ_Enable --> %d\n", pAd->fq_ctrl.enable);
	}
}
#endif  /* FQ_SCH_SUPPORT */


void rtmp_read_cp_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "begin -->\n");
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	/* for enable/disable */
	if (RTMPGetKeyParameter("CP_SUPPORT", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->cp_support =  os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CP_SUPPORT --> %d\n", pAd->cp_support);
	}
}

void rtmp_read_multi_cli_nums_eap_th_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "begin -->\n");
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	/* for enable/disable */
	if (RTMPGetKeyParameter("MCLI_NUMS_EAP_TH", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->multi_cli_nums_eap_th =  os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MCLI_NUMS_EAP_TH --> %d\n", pAd->multi_cli_nums_eap_th);
	}
}

#ifdef KERNEL_RPS_ADJUST
void rtmp_read_kernel_rps_parms_from_file(IN    PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "begin -->\n");
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	/* for enable/disable */
	if (RTMPGetKeyParameter("KernelRps", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		pAd->ixia_mode_ctl.kernel_rps_en =  os_str_tol(tmpbuf, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "KernelRps --> %d\n",
			pAd->ixia_mode_ctl.kernel_rps_en);
	}
}
#endif
void rtmp_read_retry_parms_from_file(IN	PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	UINT32		rv = 0;
	INT		i = 0, j = 0;
	CHAR		*ptok = NULL;
	CHAR		buf[8] = {0};
	UINT		qidx = TxQ_IDX_AC33 + 1, limit = 32, band_idx = 0;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "begin -->\n");
	NdisZeroMemory(tmpbuf, MAX_PARAM_BUFFER_SIZE);

	if (RTMPGetKeyParameter("RetryLimit", tmpbuf, 128, buffer, TRUE) && (strlen(tmpbuf) > 0)) {
		for (i = 0, ptok = rstrtok(tmpbuf, ":"); ptok; ptok = rstrtok(NULL, ":"), i++) {
			qidx = TxQ_IDX_AC33 + 1;
			limit = 32;
			band_idx = 0;
			memset(buf, 0, 8);

			for (j = 0; j < 8; j++) {
				buf[j] = toupper(*(ptok+j));
			}

			if (strncmp(buf, "RTS", 3) == 0) {
				band_idx = 0;
				rv = sscanf(buf, "RTS%u-%u",&band_idx, &limit);
				if (rv != 2) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Format Error!\n");
					break;
				}
				pAd->rts_retrylimit[band_idx] = limit;
			}

			if (strncmp(buf, "AC", 2) == 0) {
				UINT ac = 0;
				rv = sscanf(buf, "AC%u-%u",&ac, &limit);
				if (rv != 2) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Format Error!\n");
					break;
				}
				qidx = (ac/10)*4 + ac%10;
				pAd->retrylimit[qidx] = limit;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(band=%u qidx=%u,retrylimit=%u)\n", band_idx, qidx, limit);
		}
	}
}

#ifdef SINGLE_SKU_V2
/* TODO: shiang-usw, for MT76x0 series, currently cannot use this function! */
NDIS_STATUS RTMPSetSkuParam(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS ret;
	ret = MtPwrLimitLoadParamHandle(pAd, POWER_LIMIT_TABLE_TYPE_SKU);
	return ret;
}

NDIS_STATUS RTMPSetBackOffParam(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS ret;
	ret = MtPwrLimitLoadParamHandle(pAd, POWER_LIMIT_TABLE_TYPE_BACKOFF);
	return ret;
}

NDIS_STATUS RTMPResetSkuParam(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;

	ret = MtPwrLimitUnloadParamHandle(pAd, POWER_LIMIT_TABLE_TYPE_SKU);
	return ret;
}

NDIS_STATUS RTMPResetBackOffParam(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	ret = MtPwrLimitUnloadParamHandle(pAd, POWER_LIMIT_TABLE_TYPE_BACKOFF);
	return ret;
}


#define	SKU_PHYMODE_CCK_1M_2M				0
#define	SKU_PHYMODE_CCK_5M_11M				1
#define	SKU_PHYMODE_OFDM_6M_9M				2
#define	SKU_PHYMODE_OFDM_12M_18M			3
#define	SKU_PHYMODE_OFDM_24M_36M			4
#define	SKU_PHYMODE_OFDM_48M_54M			5
#define	SKU_PHYMODE_HT_MCS0_MCS1			6
#define	SKU_PHYMODE_HT_MCS2_MCS3			7
#define	SKU_PHYMODE_HT_MCS4_MCS5			8
#define	SKU_PHYMODE_HT_MCS6_MCS7			9
#define	SKU_PHYMODE_HT_MCS8_MCS9			10
#define	SKU_PHYMODE_HT_MCS10_MCS11			11
#define	SKU_PHYMODE_HT_MCS12_MCS13			12
#define	SKU_PHYMODE_HT_MCS14_MCS15			13
#define	SKU_PHYMODE_STBC_MCS0_MCS1			14
#define	SKU_PHYMODE_STBC_MCS2_MCS3			15
#define	SKU_PHYMODE_STBC_MCS4_MCS5			16
#define	SKU_PHYMODE_STBC_MCS6_MCS7			17


VOID InitSkuRateDiffTable(
	IN PRTMP_ADAPTER	pAd)
{
	USHORT		i, value = 0;
	CHAR		BasePwr, Pwr;

	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 4, value);
	BasePwr = (value >> 8) & 0xFF;
	BasePwr = (BasePwr > 0x1F) ? BasePwr - 0x40 : BasePwr;

	for (i = 0; i < 9; i++) {
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i * 2, value);
		Pwr = value & 0xFF;
		Pwr = (Pwr > 0x1F) ? Pwr - 0x40 : Pwr;
		pAd->SingleSkuRatePwrDiff[i * 2] = Pwr - BasePwr;
		Pwr = (value >> 8) & 0xFF;
		Pwr = (Pwr > 0x1F) ? Pwr - 0x40 : Pwr;
		pAd->SingleSkuRatePwrDiff[i * 2 + 1] = Pwr - BasePwr;
	}
}
#endif /* SINGLE_SKU_V2 */

INT32 ralinkrate[] = {
	/* CCK */
	2, 4, 11, 22,
	/* OFDM */
	12, 18, 24, 36, 48, 72, 96, 108,
	/* 20MHz, 800ns GI, MCS: 0 ~ 15 */
	13, 26, 39, 52, 78, 104, 117, 130, 26, 52, 78, 104, 156, 208, 234, 260,
	/* 20MHz, 800ns GI, MCS: 16 ~ 23 */
	39, 78, 117, 156, 234, 312, 351, 390,
	/* 40MHz, 800ns GI, MCS: 0 ~ 15 */
	27, 54, 81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540,
	/* 40MHz, 800ns GI, MCS: 16 ~ 23 */
	81, 162, 243, 324, 486, 648, 729, 810,
	/* 20MHz, 400ns GI, MCS: 0 ~ 15 */
	14, 29, 43, 57, 87, 115, 130, 144, 29, 59, 87, 115, 173, 230, 260, 288,
	/* 20MHz, 400ns GI, MCS: 16 ~ 23 */
	43, 87, 130, 173, 260, 317, 390, 433,
	/* 40MHz, 400ns GI, MCS: 0 ~ 15 */
	30, 60, 90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600,
	/* 40MHz, 400ns GI, MCS: 16 ~ 23 */
	90, 180, 270, 360, 540, 720, 810, 900
};

UINT32 RT_RateSize = sizeof(ralinkrate);
