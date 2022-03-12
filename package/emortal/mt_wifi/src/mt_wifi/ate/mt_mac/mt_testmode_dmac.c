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
	mt_testmode_dmac.c

*/

#ifdef COMPOS_TESTMODE_WIN
#include "config.h"
#else
#include "rt_config.h"
#endif

INT32 mtd_ate_mac_cr_restore(RTMP_ADAPTER *pAd)
{
#if defined(MT7615) || defined(MT7622) || defined(MT7663) || defined(MT7626)
	if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7663(pAd) || IS_MT7626(pAd)) {
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
		UINT i;
		/* Data frame protection CR recover */
		MtTestModeRestoreCr(pAd, AGG_PCR);
		/* RTS threshold CR recover */
		MtTestModeRestoreCr(pAd, AGG_PCR1);
#ifdef DBDC_MODE
		if (IS_ATE_DBDC(pAd)) {
			MtTestModeRestoreCr(pAd, AGG_PCR2);
		}
#endif /* DBDC_MODE */
		/* BA related CR recover */
		MtTestModeRestoreCr(pAd, AGG_AWSCR0);
		MtTestModeRestoreCr(pAd, AGG_AWSCR1);
		/* Station pause CR recover */
		MtTestModeRestoreCr(pAd, STATION_PAUSE0);
		MtTestModeRestoreCr(pAd, STATION_PAUSE1);
		MtTestModeRestoreCr(pAd, STATION_PAUSE2);
		MtTestModeRestoreCr(pAd, STATION_PAUSE3);
		/* Enable HW BAR feature */
		MtTestModeRestoreCr(pAd, AGG_MRCR);
		/* IPG related CR recover */
		MtTestModeRestoreCr(pAd, TMAC_TRCR0);
		MtTestModeRestoreCr(pAd, TMAC_ICR_BAND_0);
		MtTestModeRestoreCr(pAd, ARB_DRNGR0);
		MtTestModeRestoreCr(pAd, ARB_DRNGR1);

		/* WMM related CR back up */
		for (i = 0; i < cap->qos.WmmHwNum; i++) {
			switch (i) {
			case 0:
				MtTestModeRestoreCr(pAd, AGG_AALCR0);
				MtTestModeRestoreCr(pAd, ARB_WMMAC01);
				break;

			case 1:
				MtTestModeRestoreCr(pAd, AGG_AALCR1);
				MtTestModeRestoreCr(pAd, ARB_WMMAC11);
				break;

			case 2:
				MtTestModeRestoreCr(pAd, AGG_AALCR2);
				MtTestModeRestoreCr(pAd, ARB_WMMAC21);
				break;

			case 3:
				MtTestModeRestoreCr(pAd, AGG_AALCR3);
				MtTestModeRestoreCr(pAd, ARB_WMMAC31);
				break;

			default:
				break;
			}
		}

		MtTestModeRestoreCr(pAd, ARB_WMMALTX0);

		if (!IS_MT7615(pAd)) {
			/* VHT20 MCS9 Support on LDPC Mode backup*/
			MtTestModeRestoreCr(pAd, WF_WTBLOFF_TOP_LUECR_ADDR);
		}

#ifdef DBDC_MODE
		if (IS_ATE_DBDC(pAd)) {
			MtTestModeRestoreCr(pAd, TMAC_TRCR1);
			MtTestModeRestoreCr(pAd, TMAC_ICR_BAND_1);
			MtTestModeRestoreCr(pAd, ARB_WMMALTX1);
		}
#endif /* DBDC_MODE */
	}
#endif /* defined(MT7615) || defined(MT7622) */

	return 0;
}

INT32 mtd_ate_mac_cr_backup_and_set(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;

	NdisZeroMemory(&ATECtrl->bk_cr, sizeof(struct _TESTMODE_BK_CR)*MAX_TEST_BKCR_NUM);
#if defined(MT7615) || defined(MT7622) || defined(MT7663) || defined(MT7626)
	/* TODO: check if following operation also need to do for other chips */
	if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_MT7663(pAd) || IS_MT7626(pAd)) {
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
		UINT i;
		UINT32 val = 0;
		/* Enable data frame protection for test mode */
		MtTestModeBkCr(pAd, AGG_PCR, TEST_MAC_BKCR);
		MAC_IO_READ32(pAd->hdev_ctrl, AGG_PCR, &val);
		val = 0x80008;
		MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_PCR, val);
		/* RTS threshold need to change to 1 for test mode */
		MtTestModeBkCr(pAd, AGG_PCR1, TEST_MAC_BKCR);
		MAC_IO_READ32(pAd->hdev_ctrl, AGG_PCR1, &val);
		/* val &= 0x0FFFFFFF; */
		/* val |= 0x10000000; */
		/* Setting RTS threshold to max value to aviod send RTS in test mode */
		val = 0xFE0FFFFF;
		MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_PCR1, val);
#ifdef DBDC_MODE
		/* RTS threshold disable for band1 */
		if (IS_ATE_DBDC(pAd)) {
			MtTestModeBkCr(pAd, AGG_PCR2, TEST_MAC_BKCR);
			MAC_IO_READ32(pAd->hdev_ctrl, AGG_PCR2, &val);
			val = 0xFE0FFFFF;
			MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_PCR2, val);
		}
#endif /* DBDC_MODE */
		/* BA related CR backup */
		MtTestModeBkCr(pAd, AGG_AWSCR0, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, AGG_AWSCR1, TEST_MAC_BKCR);
		/* Station pause CR backup for TX after reset WTBL */
		MtTestModeBkCr(pAd, STATION_PAUSE0, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, STATION_PAUSE1, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, STATION_PAUSE2, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, STATION_PAUSE3, TEST_MAC_BKCR);
		/* HW BAR feature */
		MtTestModeBkCr(pAd, AGG_MRCR, TEST_MAC_BKCR);
		MAC_IO_READ32(pAd->hdev_ctrl, AGG_MRCR, &val);
		val &= ~BAR_TX_CNT_LIMIT_MASK;
		val |= BAR_TX_CNT_LIMIT(0);
		MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_MRCR, val);
		/* IPG related CR back up */
		MtTestModeBkCr(pAd, TMAC_TRCR0, TEST_MAC_BKCR);
		MtTestModeBkCr(pAd, TMAC_ICR_BAND_0, TEST_MAC_BKCR);    /* IFS CR, for SIFS/SLOT time control */
		MtTestModeBkCr(pAd, ARB_DRNGR0, TEST_MAC_BKCR);         /* For fixing backoff random number */
		MtTestModeBkCr(pAd, ARB_DRNGR1, TEST_MAC_BKCR);

		/* WMM related CR back up */
		for (i = 0; i < cap->qos.WmmHwNum; i++) {
			switch (i) {
			case 0:
				MtTestModeBkCr(pAd, AGG_AALCR0, TEST_MAC_BKCR);
				MtTestModeBkCr(pAd, ARB_WMMAC01, TEST_MAC_BKCR);
				break;

			case 1:
				MtTestModeBkCr(pAd, AGG_AALCR1, TEST_MAC_BKCR);
				MtTestModeBkCr(pAd, ARB_WMMAC11, TEST_MAC_BKCR);
				break;

			case 2:
				MtTestModeBkCr(pAd, AGG_AALCR2, TEST_MAC_BKCR);
				MtTestModeBkCr(pAd, ARB_WMMAC21, TEST_MAC_BKCR);
				break;

			case 3:
				MtTestModeBkCr(pAd, AGG_AALCR3, TEST_MAC_BKCR);
				MtTestModeBkCr(pAd, ARB_WMMAC31, TEST_MAC_BKCR);
				break;

			default:
				break;
			}
		}

		MtTestModeBkCr(pAd, ARB_WMMALTX0, TEST_MAC_BKCR);

		if (!IS_MT7615(pAd)) {
			/* VHT20 MCS9 Support on LDPC Mode */
			MtTestModeBkCr(pAd, WF_WTBLOFF_TOP_LUECR_ADDR, TEST_MAC_BKCR);
			MAC_IO_READ32(pAd->hdev_ctrl, WF_WTBLOFF_TOP_LUECR_ADDR, &val);
			val = 0xFFFFFFFF;
			MAC_IO_WRITE32(pAd->hdev_ctrl, WF_WTBLOFF_TOP_LUECR_ADDR, val);
		}

#ifdef DBDC_MODE
		if (IS_ATE_DBDC(pAd)) {
			MtTestModeBkCr(pAd, TMAC_TRCR1, TEST_MAC_BKCR);
			MtTestModeBkCr(pAd, TMAC_ICR_BAND_1, TEST_MAC_BKCR);
			MtTestModeBkCr(pAd, ARB_WMMALTX1, TEST_MAC_BKCR);
		}
#endif /* DBDC_MODE */
	}
#endif /* defined(MT7615) || defined(MT7622) || defined(MT7663) */
	return 0;
}

UINT32 agg_cnt_array[] = {AGG_AALCR0, AGG_AALCR1, AGG_AALCR2, AGG_AALCR3};
INT32 mtd_ate_ampdu_ba_limit(RTMP_ADAPTER *pAd, UINT8 wmm_idx, UINT8 agg_limit)
{
	UINT32 value;

	if (wmm_idx > 3) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: invalid WmmIdx=%d, set to all!\n", __func__, wmm_idx));
		wmm_idx = 0xFF;
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: WmmIdx=%d\n", __func__, wmm_idx));
	value = ((agg_limit & 0x3F) << 24)
		| ((agg_limit & 0x3F) << 16)
		| ((agg_limit & 0x3F) << 8)
		| ((agg_limit & 0x3F) << 0);

	if (wmm_idx <= 3)
		MAC_IO_WRITE32(pAd->hdev_ctrl, agg_cnt_array[wmm_idx], value);
	else {
		MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_AALCR0, value);
		MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_AALCR1, value);
		MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_AALCR2, value);
		MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_AALCR3, value);
	}

	value = 0x0;
	MAC_IO_WRITE32(pAd->hdev_ctrl, AGG_AWSCR0, value);

	return 0;
}

INT32 mtd_ate_set_sta_pause_cr(RTMP_ADAPTER *pAd, UINT8 ac_idx)
{
	INT32 ret = 0;
	UINT32 value;

	/* Set station pause CRs to 0 for TX after reset WTBL */
	/* The CR meaning in normal mode is that stop to TX packet when STA disconnect */
	value = 0x0;
	MAC_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE0, value);
	MAC_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE1, value);
	MAC_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE2, value);
	MAC_IO_WRITE32(pAd->hdev_ctrl, STATION_PAUSE3, value);

	return ret;
}

INT mtd_ate_set_ifs_cr(RTMP_ADAPTER *pAd, UINT8 band_idx)
{
	INT32 ret = 0;
	struct _ATE_IPG_PARAM *ipg_param = TESTMODE_GET_PADDR(pAd, band_idx, ipg_param);
	UINT16 slot_time, sifs_time;
	UINT32 txv_time = 0, i2t_chk_time = 0, tr2t_chk_time = 0;
	UINT32 value = 0;

	slot_time = ipg_param->slot_time;
	sifs_time = ipg_param->sifs_time;
	/* in uint of ns */
	MAC_IO_READ32(pAd, TMAC_ATCR, &txv_time);
	txv_time *= NORMAL_CLOCK_TIME;
	i2t_chk_time = (UINT32)(slot_time * 1000) - txv_time - BBP_PROCESSING_TIME;
	tr2t_chk_time = (UINT32)(sifs_time * 1000) - txv_time - BBP_PROCESSING_TIME;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: txv_time=%dns, i2t_chk_time=%dns, tr2t_chk_time=%dns\n",
		__func__, txv_time, i2t_chk_time, tr2t_chk_time));
	i2t_chk_time /= NORMAL_CLOCK_TIME;
	tr2t_chk_time /= NORMAL_CLOCK_TIME;

	if (band_idx == TESTMODE_BAND0) {
		MAC_IO_READ32(pAd->hdev_ctrl, TMAC_TRCR0, &value);
		value = (value & 0xFE00FE00)
				| ((i2t_chk_time & 0x1FF) << 16)
				| ((tr2t_chk_time & 0x1FF) << 0);
		MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_TRCR0, value);
	} else if (band_idx == TESTMODE_BAND1) {
		MAC_IO_READ32(pAd->hdev_ctrl, TMAC_TRCR1, &value);
		value = (value & 0xFE00FE00)
				| ((i2t_chk_time & 0x1FF) << 16)
				| ((tr2t_chk_time & 0x1FF) << 0);
		MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_TRCR1, value);
	} else {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Invalid band_idx!!\n", __func__));
		return FALSE;
	}

	return ret;
}
