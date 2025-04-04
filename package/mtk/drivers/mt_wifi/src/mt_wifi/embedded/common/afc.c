/*
 * Copyright (c) [2022], MediaTek Inc. All rights reserved.
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
	afc.c
*/
#include "rt_config.h"

#ifdef CONFIG_6G_AFC_SUPPORT
/*******************************************************************************
 *    MACRO
 ******************************************************************************/

/*******************************************************************************
 *    DEFINITIONS
 ******************************************************************************/

#define AFC_PWR_LIMIT_TBL_PATH            "/etc/wireless/mediatek/AFCPwrLimitTbl.dat"
#define AFC_6G_FIRST_CHANNEL_INDEX        1
#define AFC_6G_LAST_CHANNEL_INDEX         233
#define AFC_6G_CHANNEL_FREQ_GAP           4

/* (((AFC_6G_LAST_CHANNEL_INDEX - AFC_6G_FIRST_CHANNEL_INDEX)/AFC_6G_CHANNEL_FREQ_GAP) + 1) */
#define AFC_6G_CHANNEL_20MHZ_NUM          59
#define AFC_6G_PWR_LIMIT_TBL_COL          8
#define AFC_6G_PWR_LIMIT_TBL_ROW          AFC_6G_CHANNEL_20MHZ_NUM

/* number of channels in unii-5 6G band */
#define AFC_20MHZ_CHAN_UNII_5             24
#define AFC_40MHZ_CHAN_UNII_5             12
#define AFC_80MHZ_CHAN_UNII_5             6
#define AFC_160MHZ_CHAN_UNII_5            3

/* number of channels in unii-7 6G band */
#define AFC_20MHZ_CHAN_UNII_7             17
#define AFC_40MHZ_CHAN_UNII_7             8
#define AFC_80MHZ_CHAN_UNII_7             3
#define AFC_160MHZ_CHAN_UNII_7            1

#define AFC_UNII_5_20MHZ_FIRST_CHAN_IDX   1
#define AFC_UNII_5_20MHZ_LAST_CHAN_IDX    93
#define AFC_UNII_5_40MHZ_FIRST_CHAN_IDX   1
#define AFC_UNII_5_40MHZ_LAST_CHAN_IDX    93
#define AFC_UNII_5_80MHZ_FIRST_CHAN_IDX   1
#define AFC_UNII_5_80MHZ_LAST_CHAN_IDX    93
#define AFC_UNII_5_160MHZ_FIRST_CHAN_IDX  1
#define AFC_UNII_5_160MHZ_LAST_CHAN_IDX   93
#define AFC_UNII_7_20MHZ_FIRST_CHAN_IDX   117
#define AFC_UNII_7_20MHZ_LAST_CHAN_IDX    181
#define AFC_UNII_7_40MHZ_FIRST_CHAN_IDX   121
#define AFC_UNII_7_40MHZ_LAST_CHAN_IDX    181
#define AFC_UNII_7_80MHZ_FIRST_CHAN_IDX   129
#define AFC_UNII_7_80MHZ_LAST_CHAN_IDX    173
#define AFC_UNII_7_160MHZ_FIRST_CHAN_IDX  129
#define AFC_UNII_7_160MHZ_LAST_CHAN_IDX   157

#define AFC_UNII_5_20MHZ_FIRST_ROW_IDX    0    /* Chan 1 */
#define AFC_UNII_5_20MHZ_LAST_ROW_IDX     23   /* Chan 93 */
#define AFC_UNII_5_40MHZ_FIRST_ROW_IDX    0    /* Chan 1 */
#define AFC_UNII_5_40MHZ_LAST_ROW_IDX     23   /* Chan 93 */
#define AFC_UNII_5_80MHZ_FIRST_ROW_IDX    0    /* Chan 1 */
#define AFC_UNII_5_80MHZ_LAST_ROW_IDX     23   /* Chan 93 */
#define AFC_UNII_5_160MHZ_FIRST_ROW_IDX   0    /* Chan 1 */
#define AFC_UNII_5_160MHZ_LAST_ROW_IDX    23   /* Chan 93 */
#define AFC_UNII_7_20MHZ_FIRST_ROW_IDX    29   /* Chan 117 */
#define AFC_UNII_7_20MHZ_LAST_ROW_IDX     45   /* Chan 181 */
#define AFC_UNII_7_40MHZ_FIRST_ROW_IDX    30   /* Chan 121 */
#define AFC_UNII_7_40MHZ_LAST_ROW_IDX     45   /* Chan 181 */
#define AFC_UNII_7_80MHZ_FIRST_ROW_IDX    32   /* Chan 129 */
#define AFC_UNII_7_80MHZ_LAST_ROW_IDX     43   /* Chan 173 */
#define AFC_UNII_7_160MHZ_FIRST_ROW_IDX   32   /* Chan 129 */
#define AFC_UNII_7_160MHZ_LAST_ROW_IDX    39   /* Chan 157 */

#define NON_AFC_CHANNEL                   126
#define AFC_TXPWR_VAL_INVALID             127
#define AFC_TXPWR_20MHZ_DELTA_PSD         13   /* 10log(20) */
#define AFC_TXPWR_DOUBLE_IN_DB            3    /* 10log(2) */

#define AFC_NUM_20MHZ_IN_20MHZ            1
#define AFC_NUM_20MHZ_IN_40MHZ            2
#define AFC_NUM_20MHZ_IN_80MHZ            4
#define AFC_NUM_20MHZ_IN_160MHZ           8
#define AFC_NUM_20MHZ_IN_320MHZ           16

#define AFC_RU26_TXPWR_OFFSET_WITH_20MHZ  10
#define AFC_RU52_TXPWR_OFFSET_WITH_20MHZ  7
#define AFC_RU106_TXPWR_OFFSET_WITH_20MHZ 4

/*******************************************************************************
 *    PRIVATE TYPES
 ******************************************************************************/

enum AFC_BW {
	AFC_BW_20,
	AFC_BW_40,
	AFC_BW_80,
	AFC_BW_160,
	AFC_BW_NUM
};

enum AFC_SET_PARAM_REF {
	AFC_RESET_AFC_PARAMS,
	AFC_SET_EIRP_PARAM,
	AFC_SET_PSD_PARAM,
	AFC_SET_EXPIRY_PARAM,
	AFC_SET_RESP_PARAM,
	AFC_CFG_PWR_LMT_TBL,
	AFC_UPD_FW_TX_PWR,
	AFC_SET_PARAM_NUM
};

enum AFC_TXPWR_TBL_BW {
	AFC_TXPWR_TBL_BW20,
	AFC_TXPWR_TBL_BW40,
	AFC_TXPWR_TBL_BW80,
	AFC_TXPWR_TBL_BW160,
	AFC_TXPWR_TBL_BW320,
	AFC_TXPWR_TBL_RU26,
	AFC_TXPWR_TBL_RU52,
	AFC_TXPWR_TBL_RU106,
	AFC_TXPWR_TBL_BW_NUM
};

UINT8 max_afc_chan_idx[BAND_6G_UNII_NUM][AFC_BW_NUM] = {
		{AFC_20MHZ_CHAN_UNII_5, AFC_40MHZ_CHAN_UNII_5,
			AFC_80MHZ_CHAN_UNII_5, AFC_160MHZ_CHAN_UNII_5},
		{AFC_20MHZ_CHAN_UNII_7, AFC_40MHZ_CHAN_UNII_7,
			AFC_80MHZ_CHAN_UNII_7, AFC_160MHZ_CHAN_UNII_7}
	};


INT8 g_afc_channel_pwr[AFC_6G_PWR_LIMIT_TBL_COL][AFC_6G_PWR_LIMIT_TBL_ROW];

/*******************************************************************************
 *    PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/

static void afc_update_pwr_limit_table(struct _RTMP_ADAPTER *pAd);

/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/

static UINT_8 afc_get_num20MHz_in_bw(UINT_8 afc_bw)
{
	UINT_8 num20 = 0;

	if (afc_bw > AFC_TXPWR_TBL_BW320)
		return 0;

	switch (afc_bw) {
	case AFC_TXPWR_TBL_BW20:
		num20 = AFC_NUM_20MHZ_IN_20MHZ;
		break;

	case AFC_TXPWR_TBL_BW40:
		num20 = AFC_NUM_20MHZ_IN_40MHZ;
		break;

	case AFC_TXPWR_TBL_BW80:
		num20 = AFC_NUM_20MHZ_IN_80MHZ;
		break;

	case AFC_TXPWR_TBL_BW160:
		num20 = AFC_NUM_20MHZ_IN_160MHZ;
		break;

	case AFC_TXPWR_TBL_BW320:
		num20 = AFC_NUM_20MHZ_IN_320MHZ;
		break;

	default:
		break;
	}

	return num20;
}

static VOID afc_get_eirp_val_per_bw(
	struct _RTMP_ADAPTER *pAd, UINT_8 subband, UINT_8 afc_bw, INT8 **eirp
)
{
	struct AFC_TX_PWR_INFO *pwr_info = &pAd->afc_response_data.afc_txpwr_info[subband];

	switch (afc_bw) {

	case AFC_TXPWR_TBL_BW20:
		*eirp = (INT8 *)pwr_info->max_eirp_bw20;
		break;

	case AFC_TXPWR_TBL_BW40:
		*eirp = (INT8 *)pwr_info->max_eirp_bw40;
		break;

	case AFC_TXPWR_TBL_BW80:
		*eirp = (INT8 *)pwr_info->max_eirp_bw80;
		break;

	case AFC_TXPWR_TBL_BW160:
		*eirp = (INT8 *)pwr_info->max_eirp_bw160;
		break;

	default:
		break;
	}
}

static VOID afc_table_get_offset_params(
	UINT_8 subband, UINT_8 afc_bw,
	UINT_8 *row_offset, UINT_8 *psd_max, UINT_8 *bw_offset
)
{
	switch (afc_bw) {

	case AFC_TXPWR_TBL_BW20:
		if (subband == BAND_6G_UNII_5) {
			*row_offset = AFC_UNII_5_20MHZ_FIRST_ROW_IDX;
			*psd_max = AFC_20MHZ_CHAN_UNII_5;
		} else {
			*row_offset = AFC_UNII_7_20MHZ_FIRST_ROW_IDX;
			*psd_max = AFC_20MHZ_CHAN_UNII_7;
		}
		*bw_offset = AFC_NUM_20MHZ_IN_20MHZ;
		break;

	case AFC_TXPWR_TBL_BW40:
		if (subband == BAND_6G_UNII_5) {
			*row_offset = AFC_UNII_5_40MHZ_FIRST_ROW_IDX;
			*psd_max = AFC_40MHZ_CHAN_UNII_5;
		} else {
			*row_offset = AFC_UNII_7_40MHZ_FIRST_ROW_IDX;
			*psd_max = AFC_40MHZ_CHAN_UNII_7;
		}
		*bw_offset = AFC_NUM_20MHZ_IN_40MHZ;
		break;

	case AFC_TXPWR_TBL_BW80:
		if (subband == BAND_6G_UNII_5) {
			*row_offset = AFC_UNII_5_80MHZ_FIRST_ROW_IDX;
			*psd_max = AFC_80MHZ_CHAN_UNII_5;
		} else {
			*row_offset = AFC_UNII_7_80MHZ_FIRST_ROW_IDX;
			*psd_max = AFC_80MHZ_CHAN_UNII_7;
		}
		*bw_offset = AFC_NUM_20MHZ_IN_80MHZ;
		break;

	case AFC_TXPWR_TBL_BW160:
		if (subband == BAND_6G_UNII_5) {
			*row_offset = AFC_UNII_5_160MHZ_FIRST_ROW_IDX;
			*psd_max = AFC_160MHZ_CHAN_UNII_5;
		} else {
			*row_offset = AFC_UNII_7_160MHZ_FIRST_ROW_IDX;
			*psd_max = AFC_160MHZ_CHAN_UNII_7;
		}
		*bw_offset = AFC_NUM_20MHZ_IN_160MHZ;
		break;

	default:
		break;
	}
}

static INT8 afc_table_get_ru_txpwr_offset_from_20mhz(UINT_8 ColIndex)
{
	INT8 offset;

	switch (ColIndex) {
	case AFC_TXPWR_TBL_RU26:
		offset = AFC_RU26_TXPWR_OFFSET_WITH_20MHZ;
		break;

	case AFC_TXPWR_TBL_RU52:
		offset = AFC_RU52_TXPWR_OFFSET_WITH_20MHZ;
		break;

	case AFC_TXPWR_TBL_RU106:
		offset = AFC_RU106_TXPWR_OFFSET_WITH_20MHZ;
		break;

	default:
		break;
	}

	return offset;
}

static VOID afc_table_update_non_afc_channels(struct _RTMP_ADAPTER *pAd)
{
	UINT_8 RowIndex, Index1;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s\n", __func__);

	/* Update 20 MHz non-AFC channel lists */
	for (RowIndex = 0; RowIndex < AFC_6G_PWR_LIMIT_TBL_ROW; RowIndex++) {
		Index1 = (RowIndex * AFC_6G_CHANNEL_FREQ_GAP) + 1;

		if (Index1 >= AFC_UNII_5_20MHZ_FIRST_CHAN_IDX && Index1 <= AFC_UNII_5_20MHZ_LAST_CHAN_IDX)
			continue;

		if (Index1 >= AFC_UNII_7_20MHZ_FIRST_CHAN_IDX && Index1 <= AFC_UNII_7_20MHZ_LAST_CHAN_IDX)
			continue;

		g_afc_channel_pwr[AFC_TXPWR_TBL_BW20][RowIndex] = NON_AFC_CHANNEL;
	}

	/* Update 40 MHz non-AFC channel lists */
	for (RowIndex = 0; RowIndex < AFC_6G_PWR_LIMIT_TBL_ROW; RowIndex++) {
		Index1 = (RowIndex * AFC_6G_CHANNEL_FREQ_GAP) + 1;

		if (Index1 >= AFC_UNII_5_40MHZ_FIRST_CHAN_IDX && Index1 <= AFC_UNII_5_40MHZ_LAST_CHAN_IDX)
			continue;

		if (Index1 >= AFC_UNII_7_40MHZ_FIRST_CHAN_IDX && Index1 <= AFC_UNII_7_40MHZ_LAST_CHAN_IDX)
			continue;

		g_afc_channel_pwr[AFC_TXPWR_TBL_BW40][RowIndex] = NON_AFC_CHANNEL;
	}

	/* Update 80 MHz non-AFC channel lists */
	for (RowIndex = 0; RowIndex < AFC_6G_PWR_LIMIT_TBL_ROW; RowIndex++) {
		Index1 = (RowIndex * AFC_6G_CHANNEL_FREQ_GAP) + 1;

		if (Index1 >= AFC_UNII_5_80MHZ_FIRST_CHAN_IDX && Index1 <= AFC_UNII_5_80MHZ_LAST_CHAN_IDX)
			continue;

		if (Index1 >= AFC_UNII_7_80MHZ_FIRST_CHAN_IDX && Index1 <= AFC_UNII_7_80MHZ_LAST_CHAN_IDX)
			continue;

		g_afc_channel_pwr[AFC_TXPWR_TBL_BW80][RowIndex] = NON_AFC_CHANNEL;
	}

	/* Update 160 MHz non-AFC channel lists */
	for (RowIndex = 0; RowIndex < AFC_6G_PWR_LIMIT_TBL_ROW; RowIndex++) {
		Index1 = (RowIndex * AFC_6G_CHANNEL_FREQ_GAP) + 1;

		if (Index1 >= AFC_UNII_5_160MHZ_FIRST_CHAN_IDX && Index1 <= AFC_UNII_5_160MHZ_LAST_CHAN_IDX)
			continue;

		if (Index1 >= AFC_UNII_7_160MHZ_FIRST_CHAN_IDX && Index1 <= AFC_UNII_7_160MHZ_LAST_CHAN_IDX)
			continue;

		g_afc_channel_pwr[AFC_TXPWR_TBL_BW160][RowIndex] = NON_AFC_CHANNEL;
	}

	/* TODO: Update 320 MHz non-AFC channel lists */
	for (RowIndex = 0; RowIndex < AFC_6G_PWR_LIMIT_TBL_ROW; RowIndex++)
		g_afc_channel_pwr[AFC_TXPWR_TBL_BW320][RowIndex] = NON_AFC_CHANNEL;
}

static VOID afc_table_update_bw20_txpwr_with_psd(struct _RTMP_ADAPTER *pAd)
{
	UINT_8 PsdIndex, RowIndex;
	struct AFC_TX_PWR_INFO *pwr_info;

	pwr_info = &pAd->afc_response_data.afc_txpwr_info[BAND_6G_UNII_5];
	for (PsdIndex = 0; PsdIndex < AFC_20MHZ_CHAN_UNII_5; PsdIndex++) {
		RowIndex = PsdIndex + AFC_UNII_5_20MHZ_FIRST_ROW_IDX;

		if (pwr_info->max_psd_bw20[PsdIndex] != AFC_TXPWR_VAL_INVALID)
			g_afc_channel_pwr[AFC_TXPWR_TBL_BW20][RowIndex] = pwr_info->max_psd_bw20[PsdIndex] + AFC_TXPWR_20MHZ_DELTA_PSD;
		else
			g_afc_channel_pwr[AFC_TXPWR_TBL_BW20][RowIndex] = pwr_info->max_psd_bw20[PsdIndex];
	}

	pwr_info = &pAd->afc_response_data.afc_txpwr_info[BAND_6G_UNII_7];
	for (PsdIndex = 0; PsdIndex < AFC_20MHZ_CHAN_UNII_7; PsdIndex++) {
		RowIndex = PsdIndex + AFC_UNII_7_20MHZ_FIRST_ROW_IDX;

		if (pwr_info->max_psd_bw20[PsdIndex] != AFC_TXPWR_VAL_INVALID)
			g_afc_channel_pwr[AFC_TXPWR_TBL_BW20][RowIndex] = pwr_info->max_psd_bw20[PsdIndex] + AFC_TXPWR_20MHZ_DELTA_PSD;
		else
			g_afc_channel_pwr[AFC_TXPWR_TBL_BW20][RowIndex] = pwr_info->max_psd_bw20[PsdIndex];
	}
}

static VOID afc_table_update_txpwr_with_psd(struct _RTMP_ADAPTER *pAd)
{
	UINT_8 RowIndex, ColIndex;

	afc_table_update_bw20_txpwr_with_psd(pAd);

	for (ColIndex = AFC_TXPWR_TBL_BW40; ColIndex <= AFC_TXPWR_TBL_BW320; ColIndex++) {
		for (RowIndex = 0; RowIndex < AFC_6G_PWR_LIMIT_TBL_ROW; RowIndex++) {
			if (g_afc_channel_pwr[ColIndex][RowIndex] == NON_AFC_CHANNEL)
				continue;

			if (g_afc_channel_pwr[AFC_TXPWR_TBL_BW20][RowIndex] != AFC_TXPWR_VAL_INVALID)
				g_afc_channel_pwr[ColIndex][RowIndex] = g_afc_channel_pwr[AFC_TXPWR_TBL_BW20][RowIndex] + (AFC_TXPWR_DOUBLE_IN_DB * ColIndex);
			else
				g_afc_channel_pwr[ColIndex][RowIndex] = g_afc_channel_pwr[AFC_TXPWR_TBL_BW20][RowIndex];
		}
	}
}

static INT8 afc_table_check_min_txpwr(
	UINT_8 start_index, UINT_8 afc_bw, UINT_8 uniband)
{
	UINT_8 RowIndex, MaxIndex;
	INT8 min_txpwr = AFC_TXPWR_VAL_INVALID;

	if (afc_bw > AFC_TXPWR_TBL_BW320)
		return AFC_TXPWR_VAL_INVALID;

	MaxIndex = start_index + afc_get_num20MHz_in_bw(afc_bw) - 1;

	for (RowIndex = start_index; RowIndex <= MaxIndex; RowIndex++) {
		if (g_afc_channel_pwr[afc_bw][RowIndex] == AFC_TXPWR_VAL_INVALID) {
			min_txpwr = AFC_TXPWR_VAL_INVALID;
			break;
		}

		min_txpwr = min(min_txpwr, g_afc_channel_pwr[afc_bw][RowIndex]);
	}

	return min_txpwr;
}

static VOID afc_table_assign_txpwr_by_index(
	struct _RTMP_ADAPTER *pAd, UINT_8 start_index, UINT_8 afc_bw, INT8 txpwr)
{
	UINT_8 RowIndex, MaxIndex;

	if (afc_bw > AFC_TXPWR_TBL_BW320)
		return;

	MaxIndex = start_index + afc_get_num20MHz_in_bw(afc_bw) - 1;

	for (RowIndex = start_index; RowIndex <= MaxIndex; RowIndex++)
		g_afc_channel_pwr[afc_bw][RowIndex] = txpwr;
}

static VOID afc_table_update_txpwr_with_eirp(struct _RTMP_ADAPTER *pAd)
{
	UINT_8 subband, BwIndex, PsdIndex, RowIndex, PsdMax = 0, RowOffset = 0, BwOffset = 0;
	INT8 txpwr;
	INT8 *eirp;

	for (subband = BAND_6G_UNII_5; subband < BAND_6G_UNII_NUM; subband++) {
		for (BwIndex = AFC_TXPWR_TBL_BW20; BwIndex < AFC_TXPWR_TBL_BW320; BwIndex++) {
			afc_table_get_offset_params(subband, BwIndex, &RowOffset, &PsdMax, &BwOffset);
			afc_get_eirp_val_per_bw(pAd, subband, BwIndex, &eirp);

			for (PsdIndex = 0; PsdIndex < PsdMax; PsdIndex++) {
				RowIndex = (PsdIndex * BwOffset) + RowOffset;
				txpwr = afc_table_check_min_txpwr(RowIndex, BwIndex, subband);

				if ((txpwr != AFC_TXPWR_VAL_INVALID) && (*(eirp + PsdIndex) != AFC_TXPWR_VAL_INVALID))
					txpwr = min(txpwr, *(eirp + PsdIndex));
				else
					txpwr = AFC_TXPWR_VAL_INVALID;

				afc_table_assign_txpwr_by_index(pAd, RowIndex, BwIndex, txpwr);
			}
		}
	}
}

static VOID afc_table_update_ru_txpwr(struct _RTMP_ADAPTER *pAd)
{
	UINT_8 RowIndex, ColIndex;
	INT8 txpwr_offset = 0;

	for (ColIndex = AFC_TXPWR_TBL_RU26; ColIndex < AFC_TXPWR_TBL_BW_NUM; ColIndex++) {
		txpwr_offset = afc_table_get_ru_txpwr_offset_from_20mhz(ColIndex);
		for (RowIndex = 0; RowIndex < AFC_6G_PWR_LIMIT_TBL_ROW; RowIndex++) {
			if (g_afc_channel_pwr[AFC_TXPWR_TBL_BW20][RowIndex] < NON_AFC_CHANNEL)
				g_afc_channel_pwr[ColIndex][RowIndex] = g_afc_channel_pwr[AFC_TXPWR_TBL_BW20][RowIndex] - txpwr_offset;
			else
				g_afc_channel_pwr[ColIndex][RowIndex] = g_afc_channel_pwr[AFC_TXPWR_TBL_BW20][RowIndex];
		}
	}
}

static VOID afc_table_update_unallowed_afc_channels(struct _RTMP_ADAPTER *pAd)
{
	UINT_8 subband, BwIndex, PsdIndex, RowIndex, PsdMax = 0, RowOffset = 0, BwOffset = 0;
	INT8 txpwr;
	INT8 *eirp;

	for (subband = BAND_6G_UNII_5; subband < BAND_6G_UNII_NUM; subband++) {
		for (BwIndex = AFC_TXPWR_TBL_BW40; BwIndex < AFC_TXPWR_TBL_BW320; BwIndex++) {
			afc_table_get_offset_params(subband, BwIndex, &RowOffset, &PsdMax, &BwOffset);

			for (PsdIndex = 0; PsdIndex < PsdMax; PsdIndex++) {
				RowIndex = (PsdIndex * BwOffset) + RowOffset;
				txpwr = afc_table_check_min_txpwr(RowIndex, (BwIndex - 1), subband);
				if (txpwr != AFC_TXPWR_VAL_INVALID)
					txpwr = afc_table_check_min_txpwr(RowIndex + (BwOffset >> 1),
									(BwIndex - 1), subband);

				if (txpwr == AFC_TXPWR_VAL_INVALID)
					afc_table_assign_txpwr_by_index(pAd, RowIndex, BwIndex, txpwr);
			}
		}
	}
}

static VOID afc_table_txpwr_print(struct _RTMP_ADAPTER *pAd)
{
	static char *col_str[AFC_6G_PWR_LIMIT_TBL_COL] = {
		"BW20", "BW40", "BW80", "BW160", "BW320", "RU26", "RU52", "RU106"};
	INT RowIndex, Index1;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Band:6G\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
			 col_str[0], col_str[1], col_str[2], col_str[3],
			 col_str[4], col_str[5], col_str[6], col_str[7]);

	for (RowIndex = 0; RowIndex < AFC_6G_PWR_LIMIT_TBL_ROW; RowIndex++) {
		Index1 = (RowIndex * AFC_6G_CHANNEL_FREQ_GAP) + 1;

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Ch%d  \t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n",
				 Index1, g_afc_channel_pwr[AFC_TXPWR_TBL_BW20][RowIndex],
				 g_afc_channel_pwr[AFC_TXPWR_TBL_BW40][RowIndex],
				 g_afc_channel_pwr[AFC_TXPWR_TBL_BW80][RowIndex],
				 g_afc_channel_pwr[AFC_TXPWR_TBL_BW160][RowIndex],
				 g_afc_channel_pwr[AFC_TXPWR_TBL_BW320][RowIndex],
				 g_afc_channel_pwr[AFC_TXPWR_TBL_RU26][RowIndex],
				 g_afc_channel_pwr[AFC_TXPWR_TBL_RU52][RowIndex],
				 g_afc_channel_pwr[AFC_TXPWR_TBL_RU106][RowIndex]);
	}
}

static VOID afc_table_update_tx_pwr_limits(struct _RTMP_ADAPTER *pAd)
{
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s\n", __func__);

	memset(&g_afc_channel_pwr[0][0], 0, sizeof(g_afc_channel_pwr));

	afc_table_update_non_afc_channels(pAd);

	/* Update 20/40/80/160/320 MHz as per PSD */
	afc_table_update_txpwr_with_psd(pAd);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\n\n%s:: After Updating PSD\n", __func__);
	afc_table_txpwr_print(pAd);

	/* Update 20/40/80/160/320 MHz as per EIRP */
	afc_table_update_txpwr_with_eirp(pAd);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\n\n%s:: After Updating 320 EIRP\n", __func__);
	afc_table_txpwr_print(pAd);

	/* Update RU 26/52/106 as per 20 MHz TxPwr value */
	afc_table_update_ru_txpwr(pAd);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\n\n%s:: After Updating RU\n", __func__);
	afc_table_txpwr_print(pAd);

	/* Update Unavailable Channels */
	afc_table_update_unallowed_afc_channels(pAd);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\n\n%s:: After Updating Unavailable Channels\n", __func__);
	afc_table_txpwr_print(pAd);
}

static void afc_update_pwr_limit_table(struct _RTMP_ADAPTER *pAd)
{
	RTMP_OS_FD fd = NULL;
	RTMP_OS_FS_INFO osFSInfo;
	UCHAR buf[2048];
	UINT32 buf_size = 2048;
	UINT32 write_size;
	CHAR *fname = NULL;
	static char *col_str[AFC_6G_PWR_LIMIT_TBL_COL] = {
		"BW20", "BW40", "BW80", "BW160", "BW320", "RU26", "RU52", "RU106"};
	INT RowIndex, Index1;

	fname = AFC_PWR_LIMIT_TBL_PATH;

	if (!fname) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				" fname is NULL\n");
		return;
	}

	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	fd = RtmpOSFileOpen(fname, O_WRONLY | O_CREAT, 0);
	if (IS_FILE_OPEN_ERR(fd)) {
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"open file fail\n");
		return;
	}

	memset(buf, 0, buf_size);

	if (strlen(buf) < (buf_size - 1))
		snprintf(buf + strlen(buf),
		buf_size - strlen(buf),
		"Band:6G\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
			col_str[0], col_str[1], col_str[2], col_str[3],
			col_str[4], col_str[5], col_str[6], col_str[7]);

	afc_table_update_tx_pwr_limits(pAd);

	for (RowIndex = 0; RowIndex < AFC_6G_PWR_LIMIT_TBL_ROW; RowIndex++) {
		Index1 = (RowIndex * AFC_6G_CHANNEL_FREQ_GAP) + 1;

		if (strlen(buf) < (buf_size - 1))
			snprintf(buf + strlen(buf),
			buf_size - strlen(buf),
			"Ch%d  \t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
				Index1, g_afc_channel_pwr[AFC_TXPWR_TBL_BW20][RowIndex],
				g_afc_channel_pwr[AFC_TXPWR_TBL_BW40][RowIndex],
				g_afc_channel_pwr[AFC_TXPWR_TBL_BW80][RowIndex],
				g_afc_channel_pwr[AFC_TXPWR_TBL_BW160][RowIndex],
				g_afc_channel_pwr[AFC_TXPWR_TBL_BW320][RowIndex],
				g_afc_channel_pwr[AFC_TXPWR_TBL_RU26][RowIndex],
				g_afc_channel_pwr[AFC_TXPWR_TBL_RU52][RowIndex],
				g_afc_channel_pwr[AFC_TXPWR_TBL_RU106][RowIndex]);
	}

	write_size = strlen(buf);
	RtmpOSFileWrite(fd, buf, write_size);
	memset(buf, 0, buf_size);

	RtmpOSFileClose(fd);
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
}

static VOID afc_cmd_set_eirp_from_index(
	struct _RTMP_ADAPTER *pAd, UINT8 unii, UINT8 bw,
	UINT8 index1, UINT8 index2, INT8 value)
{
	struct AFC_TX_PWR_INFO *pwr_info;
	UINT8 index, max_index = 0;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s::unii:%u bw:%u index1:%u index2:%u value:%d\n",
				__func__, unii, bw, index1, index2, value);

	if (unii >= BAND_6G_UNII_NUM) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "%s: Max Unii is %u\n", __func__, BAND_6G_UNII_NUM);
		return;
	}

	if (bw >= AFC_BW_NUM) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "%s: Max BW is %u\n", __func__, AFC_BW_NUM);
		return;
	}

	max_index = max_afc_chan_idx[unii][bw];

	if (index1 >= max_index) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "%s:Index1 %u > max index :%u\n", __func__, index1, max_index);
		return;
	}

	if (index2 >= max_index) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "%s:Index2 %u > max index :%u. Lets consider Index2 %u\n",
					 __func__, index2, max_index, max_index);
		index2 = max_index - 1;
	}

	pwr_info = &pAd->afc_response_data.afc_txpwr_info[unii];

	switch (bw) {
	case AFC_BW_20:
		if (index2) {
			for (index = index1; index <= index2; index++)
				pwr_info->max_eirp_bw20[index] = value;
		} else
			pwr_info->max_eirp_bw20[index1] = value;
		break;

	case AFC_BW_40:
		if (index2) {
			for (index = index1; index <= index2; index++)
				pwr_info->max_eirp_bw40[index] = value;
		} else
			pwr_info->max_eirp_bw40[index1] = value;
		break;

	case AFC_BW_80:
		if (index2) {
			for (index = index1; index <= index2; index++)
				pwr_info->max_eirp_bw80[index] = value;
		} else
			pwr_info->max_eirp_bw80[index1] = value;
		break;

	case AFC_BW_160:
		if (index2) {
			for (index = index1; index <= index2; index++)
				pwr_info->max_eirp_bw160[index] = value;
		} else
			pwr_info->max_eirp_bw160[index1] = value;
		break;

	default:
		break;
	}
}

static VOID afc_cmd_set_psd_from_index(
	struct _RTMP_ADAPTER *pAd, UINT8 unii, UINT8 index1, UINT8 index2, INT8 value)
{
	struct AFC_TX_PWR_INFO *pwr_info;
	UINT8 index, max_index = 0;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s::unii:%u index1:%u index2:%u value:%d\n",
		__func__, unii, index1, index2, value);

	if (unii >= BAND_6G_UNII_NUM) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "%s: Max Unii is %u\n", __func__, BAND_6G_UNII_NUM);
		return;
	}

	max_index = max_afc_chan_idx[unii][AFC_BW_20];

	if (index1 >= max_index) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "%s:Index1 %u > max index :%u\n", __func__, index1, max_index);
		return;
	}

	if (index2 >= max_index)
		index2 = max_index - 1;

	pwr_info = &pAd->afc_response_data.afc_txpwr_info[unii];

	if (index2) {
		for (index = index1; index <= index2; index++)
			pwr_info->max_psd_bw20[index] = value;
	} else
		pwr_info->max_psd_bw20[index1] = value;
}

INT afc_cmd_set_afc_params(IN struct _RTMP_ADAPTER *pAd, IN RTMP_STRING * arg)
{
	INT32 Ret = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	PCHAR pch = NULL;
	UINT8 paramref, band, bw, index1, index2;
	INT8 value;

	if (wdev != NULL) {
		if (!WMODE_CAP_6G(wdev->PhyMode)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "%s::Params is valid only for 6G Interface\n", __func__);
			Ret = FALSE;
			goto error;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"%s::wdev is NULL\n", __func__);
		Ret = FALSE;
		goto error;
	}

	/* setafc=0 */
	/* setafc=1-UniBand-BW-EirpValue-Index1-Index2 */
	/* setafc=2-UniBand-MaxPsdValue-Index1-Index2 */
	/* setafc=3-Expiry_Value */
	/* setafc=4-Response_Value */
	/* setafc=5 */
	/* setafc=6 */

	pch = strsep(&arg, ":");
	/* Get ParamRef */
	if (pch != NULL)
		paramref = (UINT8)os_str_toul(pch, 0, 10);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 "%s::paramref is NULL\n", __func__);
		Ret = FALSE;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s::paramref = %u\n", __func__, paramref);

	if (paramref >= AFC_SET_PARAM_NUM) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "%s::paramref max val is %u\n", __func__, AFC_SET_PARAM_NUM);
				Ret = FALSE;
				goto error;
	}

	if (paramref == AFC_RESET_AFC_PARAMS) {
		/* Reset all Value */
		os_zero_mem(&(pAd->afc_response_data), sizeof(pAd->afc_response_data));
	} else if (paramref == AFC_SET_EIRP_PARAM || paramref == AFC_SET_PSD_PARAM) {
		pch = strsep(&arg, ":");
		/*Get Band*/
		if (pch != NULL)
			band = (UINT8)os_str_toul(pch, 0, 10);
		else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s::band is NULL\n", __func__);
			Ret = FALSE;
			goto error;
		}

		if (paramref == AFC_SET_EIRP_PARAM) {
			pch = strsep(&arg, ":");
			/*Get BW*/
			if (pch != NULL)
				bw = (UINT8)os_str_toul(pch, 0, 10);
			else {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "%s::bw is NULL\n", __func__);
				Ret = FALSE;
				goto error;
			}
		}

		pch = strsep(&arg, ":");
		/*Get Value*/
		if (pch != NULL)
			value = (UINT8)os_str_tol(pch, 0, 10);
		else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "%s::value is NULL\n", __func__);
			Ret = FALSE;
			goto error;
		}

		pch = strsep(&arg, ":");
		/*Get Index1*/
		if (pch != NULL)
			index1 = (UINT8)os_str_toul(pch, 0, 10);
		else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "%s::index1 is NULL\n", __func__);
			Ret = FALSE;
			goto error;
		}

		pch = strsep(&arg, ":");
		/*Get Index2*/
		if (pch != NULL)
			index2 = (UINT8)os_str_toul(pch, 0, 10);
		else
			index2 = 0;

		if (index2 < index1)
			index2 = 0;

		if (paramref == AFC_SET_EIRP_PARAM)
			afc_cmd_set_eirp_from_index(pAd, band, bw, index1, index2, value);
		else
			afc_cmd_set_psd_from_index(pAd, band, index1, index2, value);
	} else if (paramref == AFC_SET_EXPIRY_PARAM) {
		/*Get Exipry Time*/
		pch = strsep(&arg, ":");
		if (pch != NULL)
			pAd->afc_response_data.expiry_time = (UINT32)os_str_toul(pch, 0, 10);
		else {
			Ret = FALSE;
			goto error;
		}
	} else if (paramref == AFC_SET_RESP_PARAM) {
		/*Get Response Code*/
		pch = strsep(&arg, ":");
		if (pch != NULL)
			pAd->afc_response_data.response_code = (UINT16)os_str_toul(pch, 0, 10);
		else {
			Ret = FALSE;
			goto error;
		}
	} else if (paramref == AFC_CFG_PWR_LMT_TBL) {
		/* Config AFC Power Limit */
		afc_update_pwr_limit_table(pAd);
	} else if (paramref == AFC_UPD_FW_TX_PWR) {
		/* Update TX Power in FW */
		/* TODO *****************/

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
			 "%s::Update FW\n", __func__);
	}

error:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
			 "%s:Ret = %d\n", __func__, Ret);
	return Ret;
}

INT afc_cmd_show_afc_params(IN struct _RTMP_ADAPTER *pAd, IN RTMP_STRING * arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	struct AFC_TX_PWR_INFO *pwr_info;
	UINT8 unii_index, index, max_index = 0;

	if (wdev != NULL) {
		if (!WMODE_CAP_6G(wdev->PhyMode)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "%s::Params is valid only for 6G Interface\n", __func__);
			return FALSE;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"%s::wdev is NULL\n", __func__);
		return FALSE;
	}

	for (unii_index = BAND_6G_UNII_5; unii_index < BAND_6G_UNII_NUM; unii_index++) {
		pwr_info = &pAd->afc_response_data.afc_txpwr_info[unii_index];

		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
			 "UNII_INDEX %u\n", unii_index);

		max_index = max_afc_chan_idx[unii_index][AFC_BW_20];

		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
			 "\nPSD      :");
		for (index = 0; index < max_index; index++)
			MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
				" %d", pwr_info->max_psd_bw20[index]);

		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
			 "\nEIRP 20  :");
		for (index = 0; index < max_index; index++)
			MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
				" %d", pwr_info->max_eirp_bw20[index]);


		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
			 "\nEIRP 40  :");
		max_index = max_afc_chan_idx[unii_index][AFC_BW_40];
		for (index = 0; index < max_index; index++)
			MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
				" %d", pwr_info->max_eirp_bw40[index]);

		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
			 "\nEIRP 80  :");
		max_index = max_afc_chan_idx[unii_index][AFC_BW_80];
		for (index = 0; index < max_index; index++)
			MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
				" %d", pwr_info->max_eirp_bw80[index]);

		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
			 "\nEIRP 160 :");
		max_index = max_afc_chan_idx[unii_index][AFC_BW_160];
		for (index = 0; index < max_index; index++)
			MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
				" %d", pwr_info->max_eirp_bw160[index]);

		MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
						"\n\n");
	}

	MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
		"Expiry Time   : %d\n", pAd->afc_response_data.expiry_time);

	MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
		"Response Code : %d\n\n", pAd->afc_response_data.response_code);

	return TRUE;
}

INT set_afc_event(struct _RTMP_ADAPTER	*pAd, char *arg)
{
	UINT_8 Value = os_str_tol(arg, 0, 10);

	MTWF_PRINT("%s():%d\n", __func__, Value);
	/*iwpriv ra0 set afcevents=0 to trigger AFC_INQ_EVENT event from driver to daemon*/
	if (Value == 0)
		RtmpOSWrielessEventSend(pAd->net_dev,
				RT_WLAN_EVENT_CUSTOM,
				AFC_INQ_EVENT,
				NULL,
				(char *)NULL, 0);
	/*iwpriv ra0 set afcevents=1 to trigger AFC_INQ_EVENT event from driver to daemon*/
	if (Value == 1)
		RtmpOSWrielessEventSend(pAd->net_dev,
				RT_WLAN_EVENT_CUSTOM,
				AFC_STOP_EVENT,
				NULL,
				(char *)NULL, 0);

	return TRUE;
}
INT8 afc_get_psd(CHAR *strPsd)
{
	INT8 val_psd = 0;
	UINT32 fractional_part = 0;
	CHAR *token = NULL;

	token = strsep((char **)&strPsd, ".");
	val_psd = 2 * os_str_tol(token, 0, 10);

	if (strPsd != NULL) {
		fractional_part = os_str_tol(strPsd, 0, 10);
		if (fractional_part > 0) {
			if (val_psd > 0) {
				if (strPsd[0] >= '5')
					val_psd++;
			} else {
				if (strPsd[0] > '5')
					val_psd -= 2;
				else
					val_psd--;
			}
		}
	}

	return val_psd;

}

void afc_update_psd(struct _RTMP_ADAPTER *pAd, UINT16 lowFrequency, UINT16 highFrequency, INT8 i1Psd)
{
	UINT8 channel_idx = 0;
	UINT16 start_freq = 0, last_freq = 0;
	struct AFC_TX_PWR_INFO *pAfcTxPwrInfo = &pAd->afc_response_data.afc_txpwr_info[BAND_6G_UNII_5];

	for (channel_idx = 0; channel_idx < MAX_20MHZ_CHANNEL_IN_6G_UNII; channel_idx++) {
		start_freq = UNII_5_STARTING_FREQ + (channel_idx * BW20_MHZ);
		last_freq = start_freq + BW20_MHZ;

		if ((last_freq > lowFrequency) && (start_freq < highFrequency)) {

			if ((!pAfcTxPwrInfo[BAND_6G_UNII_5].max_psd_bw20[channel_idx]) ||
				(pAfcTxPwrInfo[BAND_6G_UNII_5].max_psd_bw20[channel_idx] > i1Psd))

				pAfcTxPwrInfo[BAND_6G_UNII_5].max_psd_bw20[channel_idx] = i1Psd;
		}
	}

	for (channel_idx = 0; channel_idx < MAX_20MHZ_CHANNEL_IN_6G_UNII; channel_idx++) {
		start_freq = UNII_7_STARTING_FREQ + (channel_idx * BW20_MHZ);
		last_freq = start_freq + BW20_MHZ;

		if ((last_freq > lowFrequency) && (start_freq < highFrequency)) {

			if ((!pAfcTxPwrInfo[BAND_6G_UNII_7].max_psd_bw20[channel_idx]) ||
				(pAfcTxPwrInfo[BAND_6G_UNII_7].max_psd_bw20[channel_idx] > i1Psd))
				pAfcTxPwrInfo[BAND_6G_UNII_7].max_psd_bw20[channel_idx] = i1Psd;
		}
	}

}
void afc_update_eirp(struct _RTMP_ADAPTER *pAd, UINT8 opclass, UINT8 channelNum, INT8 i1Eirp)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;

	struct AFC_TX_PWR_INFO *pAfcTxPwrInfo = &pAd->afc_response_data.afc_txpwr_info[BAND_6G_UNII_5];
	UINT8 channel_idx = 0, channel_set_num = 0, unused_ch = 0;
	PCHAR channel_set, pmax_eirp = NULL;

	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, obj->ioctl_if, obj->ioctl_if_type);
	channel_set = get_channelset_by_reg_class(pAd, opclass, wdev->PhyMode);
	channel_set_num = get_channel_set_num(channel_set);

	for (channel_idx = 0; channel_idx < channel_set_num; channel_idx++) {

		switch (opclass) {
		case OP_CLASS_131:
			if (channel_idx == 0)
				pmax_eirp = &pAfcTxPwrInfo[BAND_6G_UNII_5].max_eirp_bw20[0];
			else if (channel_idx == MAX_20MHZ_CHANNEL_IN_6G_UNII) {
				channel_idx = UNII_7_BW20_START_INX;
				unused_ch = channel_idx;
				pmax_eirp = &pAfcTxPwrInfo[BAND_6G_UNII_7].max_eirp_bw20[0];
			}
			break;
		case OP_CLASS_132:
			if (channel_idx == 0)
				pmax_eirp = &pAfcTxPwrInfo[BAND_6G_UNII_5].max_eirp_bw40[0];
			else if (channel_idx == MAX_40MHZ_CHANNEL_IN_6G_UNII) {
				channel_idx = UNII_7_BW40_START_INX;
				unused_ch = channel_idx;
				pmax_eirp = &pAfcTxPwrInfo[BAND_6G_UNII_7].max_eirp_bw40[0];
			}
			break;
		case OP_CLASS_133:
		case OP_CLASS_135:
			if (channel_idx == 0)
				pmax_eirp = &pAfcTxPwrInfo[BAND_6G_UNII_5].max_eirp_bw80[0];
			else if (channel_idx == MAX_80MHZ_CHANNEL_IN_6G_UNII) {
				channel_idx = UNII_7_BW80_START_INX;
				unused_ch = channel_idx;
				pmax_eirp = &pAfcTxPwrInfo[BAND_6G_UNII_7].max_eirp_bw80[0];
			}
			break;
		case OP_CLASS_134:
			if (channel_idx == 0)
				pmax_eirp = &pAfcTxPwrInfo[BAND_6G_UNII_5].max_eirp_bw160[0];
			else if (channel_idx == MAX_160MHZ_CHANNEL_IN_6G_UNII) {
				channel_idx = UNII_7_BW160_START_INX;
				unused_ch = channel_idx;
				pmax_eirp = &pAfcTxPwrInfo[BAND_6G_UNII_7].max_eirp_bw160[0];
			}
			break;
		case OP_CLASS_136:
		case OP_CLASS_137:
		default:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"global OpClass %d is not Supported\n", opclass);
			break;

		}
		if (((UINT8)channel_set[channel_idx]) == channelNum) {
			pmax_eirp[channel_idx - unused_ch] = i1Eirp;
			break;
		}
	}
}

void afc_update_params_from_response(struct _RTMP_ADAPTER	*pAd,
	UINT8 *buf_data, UINT32 buf_len)
{
	UINT8 idx = 0;
	UINT16 lowFrequency = 0, highFrequency = 0;
	INT8  i1Psd = 0, i1Eirp = 0;
	UINT8 opclass = 0, channelNum = 0;
	CHAR *strPsd = NULL, *strEirp = NULL;

	if (buf_data[idx] ==  ASI_RESPONSE) {
		idx++;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Length of ASI response = %d\n", *(UINT16 *)(buf_data + idx));
		idx += TLV_HEADER;
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid data Received\n");
		return;
	}

	memset(pAd->afc_response_data.afc_txpwr_info, AFC_TXPWR_VAL_INVALID,
						sizeof(struct AFC_TX_PWR_INFO) * BAND_6G_UNII_NUM);
	while (1) {
		if (idx >= buf_len)
			return;

		switch (buf_data[idx]) {

		case FREQ_INFO:
			idx = idx + TLV_HEADER;
			lowFrequency = *(UINT16 *)(buf_data + idx);
			highFrequency = *(UINT16 *)(buf_data + idx + sizeof(UINT16));
			strPsd = &buf_data[idx + (2 * sizeof(UINT16))];
			i1Psd = afc_get_psd(strPsd);

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"frequency Info: low: %d, high: %d, Max Psd = %d\n",
					lowFrequency, highFrequency, i1Psd);

			afc_update_psd(pAd, lowFrequency, highFrequency, i1Psd);
			idx = idx + buf_data[idx - 1];
			break;
		case CHANNELS_ALLOWED:
			idx = idx + TLV_HEADER;
			break;
		case OPER_CLASS:
			idx = idx + TLV_HEADER;
			opclass = buf_data[idx];
			idx = idx + sizeof(UINT8);
			break;
		case CHANNEL_LIST:
			idx = idx + TLV_HEADER;
			channelNum = buf_data[idx];
			strEirp = &buf_data[idx + sizeof(UINT8)];

			i1Eirp = afc_get_psd(strEirp);

			afc_update_eirp(pAd, opclass, channelNum, i1Eirp);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"\nOpClass : %d, Channel Num : %d Max Eirp : %d\n",
					opclass, channelNum, i1Eirp);

			idx = idx + buf_data[idx - 1];
			break;
		case EXPIRY_TIME:
			idx = idx + TLV_HEADER;
			pAd->afc_response_data.expiry_time = *(UINT32 *)&buf_data[idx];
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Expiry Time : %d\n", pAd->afc_response_data.expiry_time);

			idx = idx + buf_data[idx - 1];
			break;
		case RESPONSE:
			idx = idx + TLV_HEADER;
			pAd->afc_response_data.response_code = *(UINT16 *)(buf_data + idx);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"AFC System Response code : %d\n", pAd->afc_response_data.response_code);

			idx = idx + buf_data[idx - 1];
		default:
			return;
		}
	}
}

int afc_daemon_response(struct _RTMP_ADAPTER *pAd, struct __RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	struct afc_response *afc_resp = NULL;
	int Status;

	os_alloc_mem(pAd, (UCHAR **)&afc_resp, wrq->u.data.length);
	if (afc_resp == NULL) {
		Status = -ENOMEM;
		return Status;
	}
	Status = copy_from_user(afc_resp, wrq->u.data.pointer, wrq->u.data.length);

	if (Status == NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Response status %d: buffer length = %d\n", afc_resp->status, wrq->u.data.length);

		afc_update_params_from_response(pAd, afc_resp->data, wrq->u.data.length - AFC_STATUS_LEN);
	} else
		Status = -EFAULT;
	os_free_mem(afc_resp);
	return Status;
}

int afc_daemon_channel_info(struct _RTMP_ADAPTER	*pAd, struct __RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	struct afc_device_info afc_data;
	int Status;

	/*TODO:need to get info from driver currently hard coded the info for debugging purpose.  */
	strcpy(afc_data.regionCode, "FCC");
	afc_data.glblOperClassNum = 3;
	afc_data.glblOperClass[0] = OP_CLASS_133;
	afc_data.glblOperClass[1] = OP_CLASS_134;
	afc_data.glblOperClass[2] = OP_CLASS_135;
	afc_data.lowFrequency = 5925;
	afc_data.highFrequency = 7125;
	afc_data.minDesiredPower = 24;
	/*************************************************************************/
	wrq->u.data.length = sizeof(afc_data);
	Status = copy_to_user(wrq->u.data.pointer, &afc_data, wrq->u.data.length);
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"regionCode is %s\n", afc_data.regionCode);
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"glblOperClassNum is %d\n", afc_data.glblOperClassNum);
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"glblOperClass is %d\n", afc_data.glblOperClass[0]);
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"glblOperClass is %d\n", afc_data.glblOperClass[1]);
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"glblOperClass is %d\n", afc_data.glblOperClass[2]);
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"lowFrequency is %d\n", afc_data.lowFrequency);
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"highFrequency is %d\n", afc_data.highFrequency);
	MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"minDesiredPower is %d\n", afc_data.minDesiredPower);
	return Status;
}

#endif /* CONFIG_6G_AFC_SUPPORT */

