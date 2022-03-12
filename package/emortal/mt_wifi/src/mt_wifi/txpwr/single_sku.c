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
	cmm_single_sku.c
*/

/*******************************************************************************
 *    INCLUDED COMMON FILES
 ******************************************************************************/

#include "rt_config.h"

/*******************************************************************************
 *    INCLUDED EXTERNAL FILES
 ******************************************************************************/


/*******************************************************************************
 *    INCLUDED INTERNAL FILES
 ******************************************************************************/
#if defined(MT7615) || defined(MT7622)
#include	"txpwr/SKUTable_1.h"
#include	"txpwr/SKUTable_2.h"
#include	"txpwr/SKUTable_3.h"
#include	"txpwr/SKUTable_4.h"
#include	"txpwr/SKUTable_5.h"
#include	"txpwr/SKUTable_6.h"
#include	"txpwr/SKUTable_7.h"
#include	"txpwr/SKUTable_8.h"
#include	"txpwr/SKUTable_9.h"
#include	"txpwr/SKUTable_10.h"
#include	"txpwr/SKUTable_11.h"
#include	"txpwr/SKUTable_12.h"
#include	"txpwr/SKUTable_13.h"
#include	"txpwr/SKUTable_14.h"
#include	"txpwr/SKUTable_15.h"
#include	"txpwr/SKUTable_16.h"
#include	"txpwr/SKUTable_17.h"
#include	"txpwr/SKUTable_18.h"
#include	"txpwr/SKUTable_19.h"
#include	"txpwr/SKUTable_20.h"
#include	"txpwr/BFBackoffTable_1.h"
#include	"txpwr/BFBackoffTable_2.h"
#include	"txpwr/BFBackoffTable_3.h"
#include	"txpwr/BFBackoffTable_4.h"
#include	"txpwr/BFBackoffTable_5.h"
#include	"txpwr/BFBackoffTable_6.h"
#include	"txpwr/BFBackoffTable_7.h"
#include	"txpwr/BFBackoffTable_8.h"
#include	"txpwr/BFBackoffTable_9.h"
#include	"txpwr/BFBackoffTable_10.h"
#include	"txpwr/BFBackoffTable_11.h"
#include	"txpwr/BFBackoffTable_12.h"
#include	"txpwr/BFBackoffTable_13.h"
#include	"txpwr/BFBackoffTable_14.h"
#include	"txpwr/BFBackoffTable_15.h"
#include	"txpwr/BFBackoffTable_16.h"
#include	"txpwr/BFBackoffTable_17.h"
#include	"txpwr/BFBackoffTable_18.h"
#include	"txpwr/BFBackoffTable_19.h"
#include	"txpwr/BFBackoffTable_20.h"
#endif
#if defined(AXE) || defined(MT7915)
#include 	"txpwr/PowerLimit_mt7915.h"
#else
#include 	"txpwr/PowerLimit.h"
#endif

/*******************************************************************************
 *   PRIVATE DEFINITIONS
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE TYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE DATA
 ******************************************************************************/


/*******************************************************************************
 *    PUBLIC DATA
 ******************************************************************************/


/*******************************************************************************
 *    EXTERNAL DATA
 ******************************************************************************/

extern RTMP_STRING *__rstrtok;

/*******************************************************************************
 *    EXTERNAL FUNCTION PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/

#if defined(MT7615) || defined(MT7622)
INT MtSingleSkuLoadParam(RTMP_ADAPTER *pAd)
{
	CHAR *buffer;
	CHAR *readline, *token;
	CHAR *ptr;
	INT index, i;
	CH_POWER_V0 *StartCh = NULL;
	UINT8 band = 0;
	UINT8 channel, *temp;
	CH_POWER_V0 *pwr = NULL;

	/* Link list Init */
	DlListInit(&pAd->PwrLimitSkuList);
	/* allocate memory for buffer SKU value */
	os_alloc_mem(pAd, (UCHAR **)&buffer, MAX_POWER_LIMIT_BUFFER_SIZE);

	if (!buffer)
		return FALSE;

	pAd->CommonCfg.SKUTableIdx = pAd->EEPROMImage[SINGLE_SKU_TABLE_EFFUSE_ADDRESS] & BITS(0, 6);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (KBLU "%s: SKU Table index = %d \n" KNRM, __func__,
			 pAd->CommonCfg.SKUTableIdx));
	/* card information file exists so reading the card information */
	os_zero_mem(buffer, MAX_POWER_LIMIT_BUFFER_SIZE);

	switch (pAd->CommonCfg.SKUTableIdx) {
	case SKUTABLE_1:
		os_move_mem(buffer, SKUvalue_1, sizeof(SKUvalue_1));
		break;

	case SKUTABLE_2:
		os_move_mem(buffer, SKUvalue_2, sizeof(SKUvalue_2));
		break;

	case SKUTABLE_3:
		os_move_mem(buffer, SKUvalue_3, sizeof(SKUvalue_3));
		break;

	case SKUTABLE_4:
		os_move_mem(buffer, SKUvalue_4, sizeof(SKUvalue_4));
		break;

	case SKUTABLE_5:
		os_move_mem(buffer, SKUvalue_5, sizeof(SKUvalue_5));
		break;

	case SKUTABLE_6:
		os_move_mem(buffer, SKUvalue_6, sizeof(SKUvalue_6));
		break;

	case SKUTABLE_7:
		os_move_mem(buffer, SKUvalue_7, sizeof(SKUvalue_7));
		break;

	case SKUTABLE_8:
		os_move_mem(buffer, SKUvalue_8, sizeof(SKUvalue_8));
		break;

	case SKUTABLE_9:
		os_move_mem(buffer, SKUvalue_9, sizeof(SKUvalue_9));
		break;

	case SKUTABLE_10:
		os_move_mem(buffer, SKUvalue_10, sizeof(SKUvalue_10));
		break;

	case SKUTABLE_11:
		os_move_mem(buffer, SKUvalue_11, sizeof(SKUvalue_11));
		break;

	case SKUTABLE_12:
		os_move_mem(buffer, SKUvalue_12, sizeof(SKUvalue_12));
		break;

	case SKUTABLE_13:
		os_move_mem(buffer, SKUvalue_13, sizeof(SKUvalue_13));
		break;

	case SKUTABLE_14:
		os_move_mem(buffer, SKUvalue_14, sizeof(SKUvalue_14));
		break;

	case SKUTABLE_15:
		os_move_mem(buffer, SKUvalue_15, sizeof(SKUvalue_15));
		break;

	case SKUTABLE_16:
		os_move_mem(buffer, SKUvalue_16, sizeof(SKUvalue_16));
		break;

	case SKUTABLE_17:
		os_move_mem(buffer, SKUvalue_17, sizeof(SKUvalue_17));
		break;

	case SKUTABLE_18:
		os_move_mem(buffer, SKUvalue_18, sizeof(SKUvalue_18));
		break;

	case SKUTABLE_19:
		os_move_mem(buffer, SKUvalue_19, sizeof(SKUvalue_19));
		break;

	case SKUTABLE_20:
		os_move_mem(buffer, SKUvalue_20, sizeof(SKUvalue_20));;
		break;

	default:
		os_move_mem(buffer, SKUvalue_20, sizeof(SKUvalue_20));
		break;
	}

	for (readline = ptr = buffer, index = 0; (ptr = os_str_chr(readline, '\t')) != NULL; readline = ptr + 1, index++)
	{
		*ptr = '\0';

		if (readline[0] == '!')
			continue;

		/* Band Info Parsing */
		if (!strncmp(readline, "Band: ", 6)) {
			token = rstrtok(readline + 6, " ");

			/* sanity check for non-Null pointer */
			if (!token)
				continue;

			band = (UINT8)os_str_tol(token, 0, 10);

			if (band == 2)
				band = 0;
			else if (band == 5)
				band = 1;

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("band = %d\n", band));
		}

		/* Rate Info Parsing for each channel */
		if (!strncmp(readline, "Ch", 2)) {
			/* Dynamic allocate memory for parsing structure */
			os_alloc_mem(pAd, (UINT8 **)&pwr, sizeof(*pwr));
			/* set default value to 0 for parsing structure */
			os_zero_mem(pwr, sizeof(*pwr));
			token = rstrtok(readline + 2, " ");

			/* sanity check for non-Null pointer */
			if (!token) {
				/* free memory buffer before escape this loop */
				os_free_mem(pwr);
				/* escape this loop for Null pointer */
				continue;
			}

			channel = (UINT8)os_str_tol(token, 0, 10);
			pwr->StartChannel = channel;
			pwr->band = band;

			/* Rate Info Parsing (CCK) */
			if (band == 0) {
				for (i = 0; i < SINGLE_SKU_TABLE_CCK_LENGTH; i++) {
					token = rstrtok(NULL, " ");

					/* sanity check for non-Null pointer */
					if (!token)
						break;

					/* config CCK Power Limit */
					MtPowerLimitFormatTrans(pAd, pwr->u1PwrLimitCCK + i, token);
				}
			}

			/* Rate Info Parsing (OFDM) */
			for (i = 0; i < SINGLE_SKU_TABLE_OFDM_LENGTH; i++) {
				token = rstrtok(NULL, " ");

				/* sanity check for non-Null pointer */
				if (!token)
					break;

				/* config ofdm Power Limit */
				MtPowerLimitFormatTrans(pAd, pwr->u1PwrLimitOFDM + i, token);
			}

#ifdef DOT11_VHT_AC

			/* Rate Info Parsing (VHT20) */
			for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
				token = rstrtok(NULL, " ");

				/* sanity check for non-Null pointer */
				if (!token)
					break;

				/* config vht20 Power Limit */
				MtPowerLimitFormatTrans(pAd, pwr->u1PwrLimitVHT20 + i, token);
			}

			/* Rate Info Parsing (VHT40) */
			for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
				token = rstrtok(NULL, " ");

				/* sanity check for non-Null pointer */
				if (!token)
					break;

				/* config vht40 Power Limit */
				MtPowerLimitFormatTrans(pAd, pwr->u1PwrLimitVHT40 + i, token);
			}

			/* if (pwr->StartChannel > 14) */
			if (band == 1) {
				/* Rate Info Parsing (VHT80) */
				for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
					token = rstrtok(NULL, " ");

					/* sanity check for non-Null pointer */
					if (!token)
						break;

					/* config vht80 Power Limit */
					MtPowerLimitFormatTrans(pAd, pwr->u1PwrLimitVHT80 + i, token);
				}

				/* Rate Info Parsing (VHT160) */
				for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
					token = rstrtok(NULL, " ");

					/* sanity check for non-Null pointer */
					if (!token)
						break;

					/* config vht160 Power Limit */
					MtPowerLimitFormatTrans(pAd, pwr->u1PwrLimitVHT160 + i, token);
				}
			}

#endif /* DOT11_VHT_AC */

			/* Tx Stream offset Info Parsing */
			for (i = 0; i < SINGLE_SKU_TABLE_TX_OFFSET_NUM; i++) {
				token = rstrtok(NULL, " ");

				/* sanity check for non-Null pointer */
				if (!token)
					break;

				/* config Tx stream offset */
				MtPowerLimitFormatTrans(pAd, pwr->u1PwrLimitTxStreamDelta + i, token);
			}

			/* Tx Spatial Stream offset Info Parsing */
			for (i = 0; i < SINGLE_SKU_TABLE_NSS_OFFSET_NUM; i++) {
				token = rstrtok(NULL, " ");

				/* sanity check for non-Null pointer */
				if (!token)
					break;

				/* config Tx spatial stream offset */
				MtPowerLimitFormatTrans(pAd, pwr->u1PwrLimitTxNSSDelta + i, token);
			}

			/* Create New Data Structure to simpilify the SKU table (Represent together for channels with same rate Info, band Info, Tx Stream offset Info, Tx Spatial stream offset Info) */
			if (!StartCh) {
				/* (Begining) assign new pointer head to SKU table contents for this channel */
				StartCh = pwr;
				/* add tail for Link list */
				DlListAddTail(&pAd->PwrLimitSkuList, &pwr->List);
			} else {
				BOOLEAN fgSameCont = TRUE;

				/* if (pwr->StartChannel <= 14) */
				if (band == 0) {
					for (i = 0; i < SINGLE_SKU_TABLE_CCK_LENGTH; i++) {
						if (StartCh->u1PwrLimitCCK[i] != pwr->u1PwrLimitCCK[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					for (i = 0; i < SINGLE_SKU_TABLE_OFDM_LENGTH; i++) {
						if (StartCh->u1PwrLimitOFDM[i] != pwr->u1PwrLimitOFDM[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
						if (StartCh->u1PwrLimitVHT20[i] != pwr->u1PwrLimitVHT20[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
						if (StartCh->u1PwrLimitVHT40[i] != pwr->u1PwrLimitVHT40[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
						if (StartCh->u1PwrLimitVHT80[i] != pwr->u1PwrLimitVHT80[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++) {
						if (StartCh->u1PwrLimitVHT160[i] != pwr->u1PwrLimitVHT160[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					for (i = 0; i < SINGLE_SKU_TABLE_TX_OFFSET_NUM; i++) {
						if (StartCh->u1PwrLimitTxStreamDelta[i] != pwr->u1PwrLimitTxStreamDelta[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					for (i = 0; i < SINGLE_SKU_TABLE_NSS_OFFSET_NUM; i++) {
						if (StartCh->u1PwrLimitTxNSSDelta[i] != pwr->u1PwrLimitTxNSSDelta[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					if (StartCh->band != pwr->band)
						fgSameCont = FALSE;
				}

				/* check similarity of SKU table content for different channel */
				if (fgSameCont)
					os_free_mem(pwr);
				else {
					/* Assign new pointer head to SKU table contents for this channel */
					StartCh = pwr;
					/* add tail for Link list */
					DlListAddTail(&pAd->PwrLimitSkuList, &StartCh->List);
				}
			}

			/* Increment total channel counts for channels with same SKU table contents */
			StartCh->num++;
			/* allocate memory for channel list with same SKU table contents */
			os_alloc_mem(pAd, (PUINT8 *)&temp, StartCh->num);

			/* backup non-empty channel list to temp buffer */
			if (NULL != StartCh->Channel) {
				/* copy channel list to temp buffer */
				os_move_mem(temp, StartCh->Channel, StartCh->num - 1);
				/* free memory for channel list used before assign pointer of temp memory buffer */
				os_free_mem(StartCh->Channel);
			}

			/* assign pointer of temp memory buffer */
			StartCh->Channel = temp;
			/* update latest channel number to channel list */
			StartCh->Channel[StartCh->num - 1] = channel;
		}
	}

	/* print out Sku table info */
	MtShowSkuTable(pAd, DBG_LVL_INFO);

	os_free_mem(buffer);
	return TRUE;
}

VOID MtSingleSkuUnloadParam(RTMP_ADAPTER *pAd)
{
	CH_POWER_V0 *ch, *ch_temp;
	DlListForEachSafe(ch, ch_temp, &pAd->PwrLimitSkuList, CH_POWER_V0, List) {
		DlListDel(&ch->List);

		/* free memory for channel list with same table contents */
		os_free_mem(ch->Channel);

		/* free memory for table contents*/
		os_free_mem(ch);
	}
}

INT MtBfBackOffLoadParam(RTMP_ADAPTER *pAd)
{
	CHAR *buffer;
	CHAR *readline, *token;
	CHAR *ptr;
	INT index, i;
	BACKOFF_POWER *StartCh = NULL;
	UINT8 band = 0;
	UINT8 channel, *temp;
	BACKOFF_POWER *pwr = NULL;
	BACKOFF_POWER *ch, *ch_temp;

	DlListInit(&pAd->PwrLimitBackoffList);
	/* init*/
	os_alloc_mem(pAd, (UCHAR **)&buffer, MAX_POWER_LIMIT_BUFFER_SIZE);

	if (buffer == NULL)
		return FALSE;

	pAd->CommonCfg.SKUTableIdx = pAd->EEPROMImage[SINGLE_SKU_TABLE_EFFUSE_ADDRESS] & BITS(0, 6);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BFBackoff Table index = %d \n", __func__,
			 pAd->CommonCfg.SKUTableIdx));
	/* card information file exists so reading the card information */
	os_zero_mem(buffer, MAX_POWER_LIMIT_BUFFER_SIZE);

	switch (pAd->CommonCfg.SKUTableIdx) {
	case SKUTABLE_1:
		os_move_mem(buffer, BFBackoffvalue_1, sizeof(BFBackoffvalue_1));
		break;

	case SKUTABLE_2:
		os_move_mem(buffer, BFBackoffvalue_2, sizeof(BFBackoffvalue_2));
		break;

	case SKUTABLE_3:
		os_move_mem(buffer, BFBackoffvalue_3, sizeof(BFBackoffvalue_3));
		break;

	case SKUTABLE_4:
		os_move_mem(buffer, BFBackoffvalue_4, sizeof(BFBackoffvalue_4));
		break;

	case SKUTABLE_5:
		os_move_mem(buffer, BFBackoffvalue_5, sizeof(BFBackoffvalue_5));
		break;

	case SKUTABLE_6:
		os_move_mem(buffer, BFBackoffvalue_6, sizeof(BFBackoffvalue_6));
		break;

	case SKUTABLE_7:
		os_move_mem(buffer, BFBackoffvalue_7, sizeof(BFBackoffvalue_7));
		break;

	case SKUTABLE_8:
		os_move_mem(buffer, BFBackoffvalue_8, sizeof(BFBackoffvalue_8));
		break;

	case SKUTABLE_9:
		os_move_mem(buffer, BFBackoffvalue_9, sizeof(BFBackoffvalue_9));
		break;

	case SKUTABLE_10:
		os_move_mem(buffer, BFBackoffvalue_10, sizeof(BFBackoffvalue_10));
		break;

	case SKUTABLE_11:
		os_move_mem(buffer, BFBackoffvalue_11, sizeof(BFBackoffvalue_11));
		break;

	case SKUTABLE_12:
		os_move_mem(buffer, BFBackoffvalue_12, sizeof(BFBackoffvalue_12));
		break;

	case SKUTABLE_13:
		os_move_mem(buffer, BFBackoffvalue_13, sizeof(BFBackoffvalue_13));
		break;

	case SKUTABLE_14:
		os_move_mem(buffer, BFBackoffvalue_14, sizeof(BFBackoffvalue_14));
		break;

	case SKUTABLE_15:
		os_move_mem(buffer, BFBackoffvalue_15, sizeof(BFBackoffvalue_15));
		break;

	case SKUTABLE_16:
		os_move_mem(buffer, BFBackoffvalue_16, sizeof(BFBackoffvalue_16));
		break;

	case SKUTABLE_17:
		os_move_mem(buffer, BFBackoffvalue_17, sizeof(BFBackoffvalue_17));
		break;

	case SKUTABLE_18:
		os_move_mem(buffer, BFBackoffvalue_18, sizeof(BFBackoffvalue_18));
		break;

	case SKUTABLE_19:
		os_move_mem(buffer, BFBackoffvalue_19, sizeof(BFBackoffvalue_19));
		break;

	case SKUTABLE_20:
		os_move_mem(buffer, BFBackoffvalue_20, sizeof(BFBackoffvalue_20));
		break;

	default:
		os_move_mem(buffer, SKUvalue_20, sizeof(SKUvalue_20));
		break;
	}

	for (readline = ptr = buffer, index = 0; (ptr = os_str_chr(readline, '\t')) != NULL; readline = ptr + 1, index++)
	{
		*ptr = '\0';

		if (readline[0] == '!')
			continue;

		/* Band Info Parsing */
		if (!strncmp(readline, "Band: ", 6)) {
			token = rstrtok(readline + 6, " ");

			/* sanity check for non-Null pointer */
			if (!token)
				continue;

			band = (UINT8)os_str_tol(token, 0, 10);

			if (band == 2)
				band = 0;
			else if (band == 5)
				band = 1;

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("band = %d\n", band));
		}

		/* BF Backoff Info Parsing for each channel */
		if (!strncmp(readline, "Ch", 2)) {
			/* Dynamic allocate memory for parsing structure */
			os_alloc_mem(pAd, (UINT8 **)&pwr, sizeof(*pwr));
			/* set default value to 0 for parsing structure */
			os_zero_mem(pwr, sizeof(*pwr));
			token = rstrtok(readline + 2, " ");

			/* sanity check for non-Null pointer */
			if (!token) {
				/* free memory buffer before escape this loop */
				os_free_mem(pwr);
				/* escape this loop for Null pointer */
				continue;
			}

			channel = (UINT8)os_str_tol(token, 0, 10);
			pwr->StartChannel = channel;
			pwr->band = band;

			/* BF Backoff Info Parsing */
			for (i = 0; i < 3; i++) {
				token = rstrtok(NULL, " ");

				/* sanity check for non-Null pointer */
				if (!token)
					break;

				/* config bf power Limit */
				MtPowerLimitFormatTrans(pAd, pwr->PwrMax + i, token);
			}

			/* Create New Data Structure to simpilify the SKU table (Represent together for channels with same BF Backoff Info) */
			if (!StartCh) {
				/* (Begining) assign new pointer head to SKU table contents for this channel */
				StartCh = pwr;
				/* add tail for Link list */
				DlListAddTail(&pAd->PwrLimitBackoffList, &pwr->List);
			} else {
				BOOLEAN fgSameCont = TRUE;

				if (fgSameCont) {
					for (i = 0; i < 3; i++) {
						if (StartCh->PwrMax[i] != pwr->PwrMax[i]) {
							fgSameCont = FALSE;
							break;
						}
					}
				}

				if (fgSameCont) {
					if (StartCh->band != pwr->band)
						fgSameCont = FALSE;
				}

				/* check similarity of SKU table content for different channel */
				if (fgSameCont)
					os_free_mem(pwr);
				else {
					/* Assign new pointer head to SKU table contents for this channel */
					StartCh = pwr;
					/* add tail for Link list */
					DlListAddTail(&pAd->PwrLimitBackoffList, &StartCh->List);
				}
			}

			/* Increment total channel counts for channels with same SKU table contents */
			StartCh->num++;
			/* allocate memory for channel list with same SKU table contents */
			os_alloc_mem(pAd, (PUINT8 *)&temp, StartCh->num);

			/* backup non-empty channel list to temp buffer */
			if (StartCh->Channel != NULL) {
				/* copy channel list to temp buffer */
				os_move_mem(temp, StartCh->Channel, StartCh->num - 1);
				/* free memory for channel list used before assign pointer of temp memory buffer */
				os_free_mem(StartCh->Channel);
			}

			/* assign pointer of temp memory buffer */
			StartCh->Channel = temp;
			/* update latest channel number to channel list */
			StartCh->Channel[StartCh->num - 1] = channel;
		}
	}

	DlListForEachSafe(ch, ch_temp, &pAd->PwrLimitBackoffList, BACKOFF_POWER, List) {
		int i;
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("start ch = %d, ch->num = %d\n", ch->StartChannel, ch->num));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Band: %d \n", ch->band));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Channel: "));

		for (i = 0; i < ch->num; i++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%d ", ch->Channel[i]));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\n"));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Max Power: "));

		for (i = 0; i < 3; i++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%d ", ch->PwrMax[i]));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\n"));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("-----------------------------------------------------------------\n"));
	}

	os_free_mem(buffer);
	return TRUE;
}

VOID MtBfBackOffUnloadParam(RTMP_ADAPTER *pAd)
{
	BACKOFF_POWER *ch, *ch_temp;
	DlListForEachSafe(ch, ch_temp, &pAd->PwrLimitBackoffList, BACKOFF_POWER, List) {
		DlListDel(&ch->List);

		/* free memory for channel list with same table contents */
		os_free_mem(ch->Channel);

		/* free memory for table contents*/
		os_free_mem(ch);
	}
}

VOID MtFillSkuParam(RTMP_ADAPTER *pAd, UINT8 channel, UINT8 Band, UINT8 TxStream, UINT8 *txPowerSku)
{
	CH_POWER_V0 *ch, *ch_temp;
	UINT8 start_ch;
	UINT8 i, j;
	UINT8 TxOffset = 0;
	UINT8 band_local = 0;

	/* -----------------------------------------------------------------------------------------------------------------------*/
	/* This part is due to MtCmdChannelSwitch is not ready for 802.11j and variable channel_band is always 0				  */
	/* -----------------------------------------------------------------------------------------------------------------------*/

	if (channel >= 16) /* must be 5G */
		band_local = 1;
	else if ((channel <= 14) && (channel >= 8)) /* depends on "channel_band" in MtCmdChannelSwitch */
		band_local = Band;
	else if (channel <= 8) /* must be 2.4G */
		band_local = 0;

	DlListForEachSafe(ch, ch_temp, &pAd->PwrLimitSkuList, CH_POWER_V0, List) {
		start_ch = ch->StartChannel;
		/* if (channel >= start_ch) */
		/* { */
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: channel = %d, start_ch = %d , Band = %d\n", __func__, channel,
				 start_ch, Band));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: ch->num = %d\n", __func__, ch->num));

		for (j = 0; j < ch->num; j++) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: In for loop, channel = %d, ch->Channel[%d] = %d\n", __func__,
					 channel, j, ch->Channel[j]));

			if (Band == ch->band) {
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: Band check, ch->Channel[%d] = %d\n", __func__, j,
						 ch->Channel[j]));

				if (channel == ch->Channel[j]) {
					for (i = 0; i < SINGLE_SKU_TABLE_CCK_LENGTH; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("CCK[%d]: 0x%x\n", i, ch->u1PwrLimitCCK[i]));

					for (i = 0; i < SINGLE_SKU_TABLE_OFDM_LENGTH; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("OFDM[%d]: 0x%x\n", i, ch->u1PwrLimitOFDM[i]));

					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("VHT20[%d]: 0x%x\n", i, ch->u1PwrLimitVHT20[i]));

					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("VHT40[%d]: 0x%x\n", i, ch->u1PwrLimitVHT40[i]));

					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("VHT80[%d]: 0x%x\n", i, ch->u1PwrLimitVHT80[i]));

					for (i = 0; i < SINGLE_SKU_TABLE_VHT_LENGTH; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("VHT160[%d]: 0x%x\n", i, ch->u1PwrLimitVHT160[i]));

					for (i = 0; i < SINGLE_SKU_TABLE_TX_OFFSET_NUM; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("TxStreamDelta(%dT): 0x%x (ref to 4T)\n", (3 - i),
								 ch->u1PwrLimitTxStreamDelta[i]));

					for (i = 0; i < SINGLE_SKU_TABLE_TX_OFFSET_NUM; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("TxNSSDelta(%dSS): 0x%x (ref to 4SS)\n", i, ch->u1PwrLimitTxNSSDelta[i]));

					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("TxStream = %d\n", TxStream));

					/* check the TxStream 1T/2T/3T/4T*/
					if (TxStream == 1) {
						TxOffset = ch->u1PwrLimitTxStreamDelta[2];
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ch->u1PwrLimitTxStreamDelta[2] = %d\n", ch->u1PwrLimitTxStreamDelta[2]));
					} else if (TxStream == 2) {
						TxOffset = ch->u1PwrLimitTxStreamDelta[1];
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ch->u1PwrLimitTxStreamDelta[1] = %d\n", ch->u1PwrLimitTxStreamDelta[1]));
					} else if (TxStream == 3) {
						TxOffset = ch->u1PwrLimitTxStreamDelta[0];
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ch->u1PwrLimitTxStreamDelta[0] = %d\n", ch->u1PwrLimitTxStreamDelta[0]));
					} else if (TxStream == 4)
						TxOffset = 0;
					else
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("The TxStream value is invalid.\n"));

					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("TxOffset = %d\n", TxOffset));
					/* Fill in the SKU table for destination channel*/
					txPowerSku[SKU_CCK_1_2]	   = ch->u1PwrLimitCCK[0]	?  (ch->u1PwrLimitCCK[0]	+ TxOffset) : 0x3F;
					txPowerSku[SKU_CCK_55_11]	   = ch->u1PwrLimitCCK[1]	?  (ch->u1PwrLimitCCK[1]	+ TxOffset) : 0x3F;
					txPowerSku[SKU_OFDM_6_9]	   = ch->u1PwrLimitOFDM[0]   ?  (ch->u1PwrLimitOFDM[0]   + TxOffset) : 0x3F;
					txPowerSku[SKU_OFDM_12_18]	 = ch->u1PwrLimitOFDM[1]   ?  (ch->u1PwrLimitOFDM[1]   + TxOffset) : 0x3F;
					txPowerSku[SKU_OFDM_24_36]	 = ch->u1PwrLimitOFDM[2]   ?  (ch->u1PwrLimitOFDM[2]   + TxOffset) : 0x3F;
					txPowerSku[SKU_OFDM_48]		= ch->u1PwrLimitOFDM[3]   ?  (ch->u1PwrLimitOFDM[3]   + TxOffset) : 0x3F;
					txPowerSku[SKU_OFDM_54]		= ch->u1PwrLimitOFDM[4]   ?  (ch->u1PwrLimitOFDM[4]   + TxOffset) : 0x3F;
					txPowerSku[SKU_HT20_0_8]	   = ch->u1PwrLimitVHT20[0]  ?  (ch->u1PwrLimitVHT20[0]  + TxOffset) : 0x3F;
					/*MCS32 is a special rate will chose the max power, normally will be OFDM 6M */
					txPowerSku[SKU_HT20_32]	   =  ch->u1PwrLimitOFDM[0]  ?  (ch->u1PwrLimitOFDM[0]   + TxOffset) : 0x3F;
					txPowerSku[SKU_HT20_1_2_9_10]  = ch->u1PwrLimitVHT20[1]  ?  (ch->u1PwrLimitVHT20[1]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT20_3_4_11_12] = ch->u1PwrLimitVHT20[2]  ?  (ch->u1PwrLimitVHT20[2]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT20_5_13]	   = ch->u1PwrLimitVHT20[3]  ?  (ch->u1PwrLimitVHT20[3]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT20_6_14]	   = ch->u1PwrLimitVHT20[3]  ?  (ch->u1PwrLimitVHT20[3]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT20_7_15]	   = ch->u1PwrLimitVHT20[4]  ?  (ch->u1PwrLimitVHT20[4]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT40_0_8]	   = ch->u1PwrLimitVHT40[0]  ?  (ch->u1PwrLimitVHT40[0]  + TxOffset) : 0x3F;
					/*MCS32 is a special rate will chose the max power, normally will be OFDM 6M */
					txPowerSku[SKU_HT40_32]	   =  ch->u1PwrLimitOFDM[0]  ?  (ch->u1PwrLimitOFDM[0]   + TxOffset) : 0x3F;
					txPowerSku[SKU_HT40_1_2_9_10]  = ch->u1PwrLimitVHT40[1]  ?  (ch->u1PwrLimitVHT40[1]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT40_3_4_11_12] = ch->u1PwrLimitVHT40[2]  ?  (ch->u1PwrLimitVHT40[2]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT40_5_13]	   = ch->u1PwrLimitVHT40[3]  ?  (ch->u1PwrLimitVHT40[3]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT40_6_14]	   = ch->u1PwrLimitVHT40[3]  ?  (ch->u1PwrLimitVHT40[3]  + TxOffset) : 0x3F;
					txPowerSku[SKU_HT40_7_15]	   = ch->u1PwrLimitVHT40[4]  ?  (ch->u1PwrLimitVHT40[4]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT20_0]	   = ch->u1PwrLimitVHT20[0]  ?  (ch->u1PwrLimitVHT20[0]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT20_1_2]	   = ch->u1PwrLimitVHT20[1]  ?  (ch->u1PwrLimitVHT20[1]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT20_3_4]	   = ch->u1PwrLimitVHT20[2]  ?  (ch->u1PwrLimitVHT20[2]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT20_5_6]	   = ch->u1PwrLimitVHT20[3]  ?  (ch->u1PwrLimitVHT20[3]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT20_7]	   = ch->u1PwrLimitVHT20[4]  ?  (ch->u1PwrLimitVHT20[4]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT20_8]	   = ch->u1PwrLimitVHT20[5]  ?  (ch->u1PwrLimitVHT20[5]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT20_9]	   = ch->u1PwrLimitVHT20[6]  ?  (ch->u1PwrLimitVHT20[6]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT40_0]	   = ch->u1PwrLimitVHT40[0]  ?  (ch->u1PwrLimitVHT40[0]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT40_1_2]	   = ch->u1PwrLimitVHT40[1]  ?  (ch->u1PwrLimitVHT40[1]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT40_3_4]	   = ch->u1PwrLimitVHT40[2]  ?  (ch->u1PwrLimitVHT40[2]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT40_5_6]	   = ch->u1PwrLimitVHT40[3]  ?  (ch->u1PwrLimitVHT40[3]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT40_7]	   = ch->u1PwrLimitVHT40[4]  ?  (ch->u1PwrLimitVHT40[4]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT40_8]	   = ch->u1PwrLimitVHT40[5]  ?  (ch->u1PwrLimitVHT40[5]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT40_9]	   = ch->u1PwrLimitVHT40[6]  ?  (ch->u1PwrLimitVHT40[6]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT80_0]	   = ch->u1PwrLimitVHT80[0]  ?  (ch->u1PwrLimitVHT80[0]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT80_1_2]	   = ch->u1PwrLimitVHT80[1]  ?  (ch->u1PwrLimitVHT80[1]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT80_3_4]	   = ch->u1PwrLimitVHT80[2]  ?  (ch->u1PwrLimitVHT80[2]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT80_5_6]	   = ch->u1PwrLimitVHT80[3]  ?  (ch->u1PwrLimitVHT80[3]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT80_7]	   = ch->u1PwrLimitVHT80[4]  ?  (ch->u1PwrLimitVHT80[4]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT80_8]	   = ch->u1PwrLimitVHT80[5]  ?  (ch->u1PwrLimitVHT80[5]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT80_9]	   = ch->u1PwrLimitVHT80[6]  ?  (ch->u1PwrLimitVHT80[6]  + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT160_0]	   = ch->u1PwrLimitVHT160[0] ?  (ch->u1PwrLimitVHT160[0] + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT160_1_2]	   = ch->u1PwrLimitVHT160[1] ?  (ch->u1PwrLimitVHT160[1] + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT160_3_4]	   = ch->u1PwrLimitVHT160[2] ?  (ch->u1PwrLimitVHT160[2] + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT160_5_6]	   = ch->u1PwrLimitVHT160[3] ?  (ch->u1PwrLimitVHT160[3] + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT160_7]	   = ch->u1PwrLimitVHT160[4] ?  (ch->u1PwrLimitVHT160[4] + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT160_8]	   = ch->u1PwrLimitVHT160[5] ?  (ch->u1PwrLimitVHT160[5] + TxOffset) : 0x3F;
					txPowerSku[SKU_VHT160_9]	   = ch->u1PwrLimitVHT160[6] ?  (ch->u1PwrLimitVHT160[6] + TxOffset) : 0x3F;
					txPowerSku[SKU_1SS_Delta]	   = ch->u1PwrLimitTxNSSDelta[0] ?  ch->u1PwrLimitTxNSSDelta[0] : 0x0;
					txPowerSku[SKU_2SS_Delta]	   = ch->u1PwrLimitTxNSSDelta[1] ?  ch->u1PwrLimitTxNSSDelta[1] : 0x0;
					txPowerSku[SKU_3SS_Delta]	   = ch->u1PwrLimitTxNSSDelta[2] ?  ch->u1PwrLimitTxNSSDelta[2] : 0x0;
					txPowerSku[SKU_4SS_Delta]	   = ch->u1PwrLimitTxNSSDelta[3] ?  ch->u1PwrLimitTxNSSDelta[3] : 0x0;

					for (i = 0; i < SKU_TOTAL_SIZE; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("txPowerSku[%d]: 0x%x\n", i, txPowerSku[i]));

					break;
				}
			}
		}

		/* } */
	}
}

VOID MtFillBackoffParam(RTMP_ADAPTER *pAd, UINT8 channel, UINT8 Band, UINT8 *BFPowerBackOff)
{
	BACKOFF_POWER *ch, *ch_temp;
	UINT8 start_ch;
	UINT8 i, j;
	UINT8 band_local = 0;

	/* -----------------------------------------------------------------------------------------------------------------------*/
	/* This part is due to MtCmdChannelSwitch is not ready for 802.11j and variable channel_band is always 0				  */
	/* -----------------------------------------------------------------------------------------------------------------------*/

	if (channel >= 16) /* must be 5G */
		band_local = 1;
	else if ((channel <= 14) && (channel >= 8)) /* depends on "channel_band" in MtCmdChannelSwitch */
		band_local = Band;
	else if (channel <= 8) /* must be 2.4G */
		band_local = 0;

	DlListForEachSafe(ch, ch_temp, &pAd->PwrLimitBackoffList, BACKOFF_POWER, List) {
		start_ch = ch->StartChannel;
		/* if (channel >= start_ch) */
		/* { */
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: channel = %d, start_ch = %d , Band = %d\n", __func__, channel,
				 start_ch, Band));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: ch->num = %d\n", __func__, ch->num));

		for (j = 0; j < ch->num; j++) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: In for loop, channel = %d, ch->Channel[%d] = %d\n", __func__,
					 channel, j, ch->Channel[j]));

			if (Band == ch->band) {
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: Band check, ch->Channel[%d] = %d\n", __func__, j,
						 ch->Channel[j]));

				if (channel == ch->Channel[j]) {
					for (i = 0; i < 3; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Max Power[%d]: 0x%x\n", i, ch->PwrMax[i]));

					/* Fill in the SKU table for destination channel*/
					BFPowerBackOff[0] = ch->PwrMax[0] ? (ch->PwrMax[0]) : 0x3F;
					BFPowerBackOff[1] = ch->PwrMax[1] ? (ch->PwrMax[1]) : 0x3F;
					BFPowerBackOff[2] = ch->PwrMax[2] ? (ch->PwrMax[2]) : 0x3F;

					for (i = 0; i < 3; i++)
						MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("BFPowerBackOff[%d]: 0x%x\n", i, BFPowerBackOff[i]));

					break;
				}
			}
		}

		/* } */
	}
}

VOID MtShowSkuTable(RTMP_ADAPTER *pAd, UINT8 u1DebugLevel)
{
	UINT8 u1ColIdx;
	P_CH_POWER_V0 prPwrLimitTbl, prTempPwrLimitTbl;

	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("-----------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("SKU table index: %d \n", pAd->CommonCfg.SKUTableIdx));

	DlListForEachSafe(prPwrLimitTbl, prTempPwrLimitTbl, &pAd->PwrLimitSkuList, CH_POWER_V0, List) {
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("start channel: %d, ChListNum: %d\n", prPwrLimitTbl->StartChannel, prPwrLimitTbl->num));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("Band: %d \n", prPwrLimitTbl->band));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("Channel: "));
		for (u1ColIdx = 0; u1ColIdx < prPwrLimitTbl->num; u1ColIdx++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->Channel[u1ColIdx]));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("CCK: "));
		for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_CCK_LENGTH; u1ColIdx++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitCCK[u1ColIdx]));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("OFDM: "));
		for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_OFDM_LENGTH; u1ColIdx++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitOFDM[u1ColIdx]));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("VHT20: "));
		for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_VHT_LENGTH; u1ColIdx++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitVHT20[u1ColIdx]));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("VHT40: "));
		for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_VHT_LENGTH; u1ColIdx++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitVHT40[u1ColIdx]));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("VHT80: "));
		for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_VHT_LENGTH; u1ColIdx++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitVHT80[u1ColIdx]));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("VHT160: "));
		for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_VHT_LENGTH; u1ColIdx++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitVHT160[u1ColIdx]));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("TxStreamDelta: "));
		for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_TX_OFFSET_NUM; u1ColIdx++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitTxStreamDelta[u1ColIdx]));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("TxNSSDelta: "));
		for (u1ColIdx = 0; u1ColIdx < SINGLE_SKU_TABLE_NSS_OFFSET_NUM; u1ColIdx++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->u1PwrLimitTxNSSDelta[u1ColIdx]));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));
	}

	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("-----------------------------------------------------------------\n"));
}

#else
NDIS_STATUS MtPwrLimitLoadParamHandle(RTMP_ADAPTER *pAd, UINT8 u1Type)
{
	PCHAR pi1Buffer;
	PDL_LIST pList = NULL;

	/* get pointer of link list address */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_LINK_LIST, (PVOID *)&pList);

	/* sanity check for null pointer */
	if (!pList)
		goto error4;

	/* Link list Init */
	DlListInit(pList);

	/* allocate memory for buffer power limit value */
	os_alloc_mem(pAd, (UINT8 **)&pi1Buffer, MAX_POWER_LIMIT_BUFFER_SIZE);

	if (!pi1Buffer)
		return NDIS_STATUS_FAILURE;

	/* update buffer with power limit table content */
	if (NDIS_STATUS_SUCCESS != MtReadPwrLimitTable(pAd, pi1Buffer, u1Type))
		goto error1;

	/* parsing sku table contents from buffer */
	if (NDIS_STATUS_SUCCESS != MtParsePwrLimitTable(pAd, pi1Buffer, u1Type))
		goto error2;

	/* enable flag for Read Power limit table pass */
		pAd->fgPwrLimitRead[u1Type] = TRUE;

	/* print out power limit table info */
	if (NDIS_STATUS_SUCCESS != MtShowPwrLimitTable(pAd, u1Type, DBG_LVL_INFO))
		goto error3;

	/* free allocated memory */
	os_free_mem(pi1Buffer);
	return NDIS_STATUS_SUCCESS;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Read Power Table Error!!\n", __func__));
	/* free allocated memory */
	os_free_mem(pi1Buffer);
	return NDIS_STATUS_FAILURE;

error2:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Parse Power Table Error!!\n", __func__));
	/* free allocated memory */
	os_free_mem(pi1Buffer);
	return NDIS_STATUS_FAILURE;

error3:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Show Power Table Error!!\n", __func__));
	/* free allocated memory */
	os_free_mem(pi1Buffer);
	return NDIS_STATUS_FAILURE;

error4:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for link list!!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtPwrLimitUnloadParamHandle(RTMP_ADAPTER *pAd, UINT8 u1Type)
{
	P_CH_POWER_V1 prPwrLimitTbl, prTempPwrLimitTbl;
	PDL_LIST pList = NULL;

	/* get pointer of link list address */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_LINK_LIST, (PVOID *)&pList);

	/* sanity check for null pointer */
	if (!pList)
		goto error0;

	/* free allocated memory for power limit table */
	if (pAd->fgPwrLimitRead[u1Type]) {
		DlListForEachSafe(prPwrLimitTbl, prTempPwrLimitTbl, pList, CH_POWER_V1, List) {

			/* delete this element link to next element */
			DlListDel(&prPwrLimitTbl->List);

			/* free memory for channel list with same table contents */
			os_free_mem(prPwrLimitTbl->pu1ChList);

			/* free memory for power limit parameters */
			os_free_mem(prPwrLimitTbl->pu1PwrLimit);

			/* free memory for table contents*/
			os_free_mem(prPwrLimitTbl);
		}

		/* disable flag for Read Power limit table pass */
		pAd->fgPwrLimitRead[u1Type] = FALSE;
	}

	return NDIS_STATUS_SUCCESS;

error0:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for link list!!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtParsePwrLimitTable(RTMP_ADAPTER *pAd, PCHAR pi1Buffer, UINT8 u1Type)
{
	UINT8 u1ChBand = CH_G_BAND;
	PCHAR pcReadline, pcToken, pcptr;
	UINT8 u1Channel = 0;
	UINT8 *prTempChList;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 u1PwrLimitParamNum[TABLE_PARSE_TYPE_NUM] = {pChipCap->single_sku_para_parse_num, pChipCap->backoff_para_parse_num};
	P_CH_POWER_V1 prTbl = NULL, prStartCh = NULL;

	/* sanity check for null pointer */
	if (!pi1Buffer)
		goto error;

	for (pcReadline = pcptr = pi1Buffer; (pcptr = os_str_chr(pcReadline, '\t')) != NULL; pcReadline = pcptr + 1) {
		*pcptr = '\0';

		/* Skip Phy mode notation cloumn line */
		if (pcReadline[0] == '#')
			continue;

		/* Channel Band Info Parsing */
		if (!strncmp(pcReadline, "Band: ", 6)) {
			pcToken = rstrtok(pcReadline + 6, " ");

			/* sanity check for non-Null pointer */
			if (!pcToken)
				continue;

			u1ChBand = (UINT8)os_str_tol(pcToken, 0, 10);

			switch (u1ChBand) {
			case 2:
				u1ChBand = CH_G_BAND;
				break;
			case 5:
				u1ChBand = CH_A_BAND;
				break;
			default:
				break;
			}

			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ChBand: %s\n", (CH_G_BAND == u1ChBand) ? "CH_G_BAND" : "CH_A_BAND"));
		}

		/* Rate Info Parsing for each u1Channel */
		if (!strncmp(pcReadline, "Ch", 2)) {
			/* Dynamic allocate memory for parsing structure */
			os_alloc_mem(pAd, (UINT8 **)&prTbl, sizeof(CH_POWER_V1));
			/* set default value to 0 for parsing structure */
			os_zero_mem(prTbl, sizeof(CH_POWER_V1));

			/* Dynamic allocate memory for parsing structure power limit paramters */
			os_alloc_mem(pAd, (UINT8 **)&prTbl->pu1PwrLimit, u1PwrLimitParamNum[u1Type]);
			/* set default value to 0 for parsing structure */
			os_zero_mem(prTbl->pu1PwrLimit, u1PwrLimitParamNum[u1Type]);

			/* pasrsing channel info */
			pcToken = rstrtok(pcReadline + 2, " ");

			/* sanity check for null pointer */
			if (!pcToken) {
				/* free memory buffer of power limit parameters before escape this loop */
				os_free_mem(prTbl->pu1PwrLimit);
				/* free total memory buffer before escape this loop */
				os_free_mem(prTbl);
				/* escape this loop for Null pointer */
				continue;
			}

			u1Channel = (UINT8)os_str_tol(pcToken, 0, 10);
			prTbl->u1StartChannel = u1Channel;
			prTbl->u1ChBand = u1ChBand;

			/* Rate Info Parsing (CCK, OFDM, VHT20/40/80/160) */
			MtPwrLimitParse(pAd, prTbl->pu1PwrLimit, u1ChBand, u1Type);

			/* Create New Data Structure to simpilify the SKU table (Represent together for channels with same Rate Power Limit Info, Band Info) */
			if (!prStartCh) {
				/* (Begining) assign new pointer head to SKU table contents for this u1Channel */
				prStartCh = prTbl;
				/* add tail for Link list */
				if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type)
					DlListAddTail(&pAd->PwrLimitSkuList, &prTbl->List);
				else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type)
					DlListAddTail(&pAd->PwrLimitBackoffList, &prTbl->List);
			} else {
				BOOLEAN fgSameCont = TRUE;

				/* check if different info contents for different channel (CCK, OFDM, VHT20/40/80/160) */
				MtPwrLimitSimilarCheck(pAd, prStartCh->pu1PwrLimit, prTbl->pu1PwrLimit, &fgSameCont, u1ChBand, u1Type);

				/* check if different info contents for different channel (channel band) */
				if (fgSameCont) {
					if (prStartCh->u1ChBand != prTbl->u1ChBand)
						fgSameCont = FALSE;
				}

				/* check similarity of SKU table content for different u1Channel */
				if (fgSameCont) {
					os_free_mem(prTbl->pu1PwrLimit);
					os_free_mem(prTbl);
				} else {
					/* Assign new pointer head to SKU table contents for this u1Channel */
					prStartCh = prTbl;
					/* add tail for Link list */
					if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type)
						DlListAddTail(&pAd->PwrLimitSkuList, &prStartCh->List);
					else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type)
						DlListAddTail(&pAd->PwrLimitBackoffList, &prStartCh->List);
				}
			}

			/* Increment total u1Channel counts for channels with same SKU table contents */
			prStartCh->u1ChNum++;
			/* allocate memory for u1Channel list with same SKU table contents */
			os_alloc_mem(pAd, (PUINT8 *)&prTempChList, prStartCh->u1ChNum);

			/* backup non-empty u1Channel list to prTempChList buffer */
			if (prStartCh->pu1ChList) {
				/* copy u1Channel list to prTempChList buffer */
				os_move_mem(prTempChList, prStartCh->pu1ChList, prStartCh->u1ChNum - 1);
				/* free memory for u1Channel list used before assign pointer of prTempChList memory buffer */
				os_free_mem(prStartCh->pu1ChList);
			}

			/* assign pointer of prTempChList memory buffer */
			prStartCh->pu1ChList = prTempChList;
			/* update latest u1Channel number to u1Channel list */
			prStartCh->pu1ChList[prStartCh->u1ChNum - 1] = u1Channel;
		}
	}

	return NDIS_STATUS_SUCCESS;

error:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer when parsing power limit table !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtReadPwrLimitTable(RTMP_ADAPTER *pAd, PCHAR pi1Buffer, UINT8 u1Type)
{
	UINT8 sku_tbl_idx = 0;
	PUINT8 pcptrSkuTbl[TABLE_SIZE] = {Sku_01, Sku_02, Sku_03, Sku_04, Sku_05,
		Sku_06, Sku_07, Sku_08, Sku_09, Sku_10,
		Sku_11, Sku_12, Sku_13, Sku_14, Sku_15,
		Sku_16, Sku_17, Sku_18, Sku_19, Sku_20};

	PUINT8 pcptrBackoffTbl[TABLE_SIZE] = {Backoff_01, Backoff_02, Backoff_03, Backoff_04, Backoff_05,
		Backoff_06, Backoff_07, Backoff_08, Backoff_09, Backoff_10,
		Backoff_11, Backoff_12, Backoff_13, Backoff_14, Backoff_15,
		Backoff_16, Backoff_17, Backoff_18, Backoff_19, Backoff_20};

	UINT16 Sku_sizeof[TABLE_SIZE] = {sizeof(Sku_01), sizeof(Sku_02), sizeof(Sku_03), sizeof(Sku_04), sizeof(Sku_05),
		sizeof(Sku_06), sizeof(Sku_07), sizeof(Sku_08), sizeof(Sku_09), sizeof(Sku_10),
		sizeof(Sku_11), sizeof(Sku_12), sizeof(Sku_13), sizeof(Sku_14), sizeof(Sku_15),
		sizeof(Sku_16), sizeof(Sku_17), sizeof(Sku_18), sizeof(Sku_19), sizeof(Sku_20)};

	UINT16 Backoff_sizeof[TABLE_SIZE] = {sizeof(Backoff_01), sizeof(Backoff_02), sizeof(Backoff_03), sizeof(Backoff_04), sizeof(Backoff_05),
		sizeof(Backoff_06), sizeof(Backoff_07), sizeof(Backoff_08), sizeof(Backoff_09), sizeof(Backoff_10),
		sizeof(Backoff_11), sizeof(Backoff_12), sizeof(Backoff_13), sizeof(Backoff_14), sizeof(Backoff_15),
		sizeof(Backoff_16), sizeof(Backoff_17), sizeof(Backoff_18), sizeof(Backoff_19), sizeof(Backoff_20)};

	/* sanity check for null pointer */
	if (!pi1Buffer)
		goto error1;

	/* query sku table index */
	chip_get_sku_tbl_idx(pAd, &sku_tbl_idx);

	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		(KBLU "%s: sku table idx: %d\n" KNRM, __func__, sku_tbl_idx));

	/* init bufer for Sku table */
	os_zero_mem(pi1Buffer, MAX_POWER_LIMIT_BUFFER_SIZE);

	/* update buffer with sku table content */
	if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type)
		os_move_mem(pi1Buffer, pcptrSkuTbl[sku_tbl_idx], Sku_sizeof[sku_tbl_idx]);
	else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type)
		os_move_mem(pi1Buffer, pcptrBackoffTbl[sku_tbl_idx], Backoff_sizeof[sku_tbl_idx]);

	return NDIS_STATUS_SUCCESS;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for buffer to read power limit table !!\n", __func__));
	return NDIS_STATUS_FAILURE;

}

NDIS_STATUS MtPwrLimitParse(RTMP_ADAPTER *pAd, PUINT8 pi1PwrLimitNewCh, UINT8 u1ChBand, UINT8 u1Type)
{
	UINT8 u1ColIdx, u1ParamType, u1ParamIdx;
	INT8  *pu1ParamTypeLen = NULL, *pu1ChBandNeedParse = NULL;
	PCHAR pcToken;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 u1TypeParseNum[TABLE_PARSE_TYPE_NUM] = {pChipCap->single_sku_type_parse_num, pChipCap->backoff_type_parse_num};

	/* sanity check for null pointer */
	if (!pi1PwrLimitNewCh)
		goto error0;

	/* update power limit value raw data type number */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_RAW_DATA_LENGTH, (PVOID *)&pu1ParamTypeLen);

	/* sanity check for null pointer */
	if (!pu1ParamTypeLen)
		goto error1;

	/* update power limit value channel band need parsing bit-field */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_CH_BAND_NEED_PARSE_BITFIELD, (PVOID *)&pu1ChBandNeedParse);

	/* sanity check for null pointer */
	if (!pu1ChBandNeedParse)
		goto error1;

	/* check if different info contents for different channel (CCK, OFDM, VHT20/40/80/160, RU26/52/106/242/484/996/996X2) */
	for (u1ParamType = 0, u1ParamIdx = 0; u1ParamType < u1TypeParseNum[u1Type]; u1ParamType++) {
		/* check if need to parse for specific channel band */
		if (*(pu1ChBandNeedParse + u1ParamType) & (u1ChBand + 1)) {
			for (u1ColIdx = 0; u1ColIdx < *(pu1ParamTypeLen + u1ParamType); u1ColIdx++) {
				/* toker update for next character parsing */
				pcToken = rstrtok(NULL, " ");
				if (!pcToken)
					break;
				/* config VHT20 Power Limit */
				MtPowerLimitFormatTrans(pAd, pi1PwrLimitNewCh + u1ColIdx + u1ParamIdx, pcToken);
			}
		}

		/* parameter index increment for different parameter type */
		u1ParamIdx += *(pu1ParamTypeLen + u1ParamType);
	}

	return NDIS_STATUS_SUCCESS;

error0:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for buffer to update power limit table after parsing !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for parameter related to parse power limit table proc !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtPwrLimitSimilarCheck(RTMP_ADAPTER *pAd, PUINT8 pi1PwrLimitStartCh, PUINT8 pi1PwrLimitNewCh, BOOLEAN *pfgSameContent, UINT8 u1ChBand, UINT8 u1Type)
{
	UINT8 u1ColIdx, u1ParamType, u1ParamIdx;
	INT8  *pu1ParamTypeLen = NULL, *pu1ChBandNeedParse = NULL;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 u1TypeParseNum[TABLE_PARSE_TYPE_NUM] = {pChipCap->single_sku_type_parse_num, pChipCap->backoff_type_parse_num};

	/* sanity check for null pointer */
	if (!pi1PwrLimitStartCh)
		goto error1;

	/* sanity check for null pointer */
	if (!pi1PwrLimitNewCh)
		goto error2;

	/* update power limit value raw data type number */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_RAW_DATA_LENGTH, (PVOID *)&pu1ParamTypeLen);

	/* sanity check for null pointer */
	if (!pu1ParamTypeLen)
		goto error3;

	/* update power limit value channel band need parsing bit-field */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_CH_BAND_NEED_PARSE_BITFIELD, (PVOID *)&pu1ChBandNeedParse);

	/* sanity check for null pointer */
	if (!pu1ChBandNeedParse)
		goto error3;

	/* same content flag init */
	*pfgSameContent = TRUE;

	/* check if different info contents for different channel (CCK, OFDM, VHT20/40/80/160, RU26/52/106/242/484/996/996X2) */
	for (u1ParamType = 0, u1ParamIdx = 0; u1ParamType < u1TypeParseNum[u1Type]; u1ParamType++) {
		/* check if need to parse for specific channel band */
		if (*(pu1ChBandNeedParse + u1ParamType) & (u1ChBand + 1)) {
			for (u1ColIdx = 0; u1ColIdx < *(pu1ParamTypeLen + u1ParamType); u1ColIdx++) {
				if (*(pi1PwrLimitStartCh + u1ColIdx + u1ParamIdx) != *(pi1PwrLimitNewCh + u1ColIdx + u1ParamIdx)) {
					*pfgSameContent = FALSE;
					return NDIS_STATUS_SUCCESS;
				}
			}
		}

		/* parameter index increment for different parameter type */
		u1ParamIdx += *(pu1ParamTypeLen + u1ParamType);
	}

	return NDIS_STATUS_SUCCESS;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for pointer to power limit table start channel for check !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error2:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for pointer to power limit table current channel for check !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error3:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for parameter related to power limit table proc similar check !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtShowPwrLimitTable(RTMP_ADAPTER *pAd, UINT8 u1Type, UINT8 u1DebugLevel)
{
	PDL_LIST pList = NULL;
	UINT8 u1ColIdx, u1ParamType, u1ParamIdx;
	UINT8 *pu1ParamTypeLen = NULL;
	P_CH_POWER_V1 prPwrLimitTbl, prTempPwrLimitTbl;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 sku_tbl_idx = 0;
	UINT8 u1TypeParseNum[TABLE_PARSE_TYPE_NUM] = {pChipCap->single_sku_type_parse_num, pChipCap->backoff_type_parse_num};

	/* update power limit value raw data type number */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_RAW_DATA_LENGTH, (PVOID *)&pu1ParamTypeLen);

	/* sanity check for null pointer */
	if (!pu1ParamTypeLen)
		goto error0;

	/* update pointer of link list address */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_LINK_LIST, (PVOID *)&pList);

	/* sanity check for null pointer */
	if (!pList)
		goto error1;

	/* query sku table index */
	chip_get_sku_tbl_idx(pAd, &sku_tbl_idx);

	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("-----------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("SKU table index: %d \n", sku_tbl_idx));

	DlListForEachSafe(prPwrLimitTbl, prTempPwrLimitTbl, pList, CH_POWER_V1, List) {
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("start channel: %d, ChListNum: %d\n", prPwrLimitTbl->u1StartChannel, prPwrLimitTbl->u1ChNum));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("Band: %d \n", prPwrLimitTbl->u1ChBand));

		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("Channel: "));
		for (u1ColIdx = 0; u1ColIdx < prPwrLimitTbl->u1ChNum; u1ColIdx++)
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", prPwrLimitTbl->pu1ChList[u1ColIdx]));
		MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

		/* check if different info contents for different channel (CCK, OFDM, VHT20/40/80/160, RU26/52/106/242/484/996/996X2) */
		for (u1ParamType = 0, u1ParamIdx = 0; u1ParamType < u1TypeParseNum[u1Type]; u1ParamType++) {

			if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%s: ", cSkuParseTypeName[u1ParamType]));
			else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%s: ", cBackoffParseTypeName[u1ParamType]));

			for (u1ColIdx = 0; u1ColIdx < *(pu1ParamTypeLen + u1ParamType); u1ColIdx++)
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("%d ", *(prPwrLimitTbl->pu1PwrLimit + u1ColIdx + u1ParamIdx)));
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("\n"));

			/* parameter index increment for different parameter type */
			u1ParamIdx += *(pu1ParamTypeLen + u1ParamType);
		}
	}

	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, u1DebugLevel, ("-----------------------------------------------------------------\n"));

	return NDIS_STATUS_SUCCESS;

error0:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: null pointer for parameter related to show power limit table !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: null pointer for list of power limit table to show power limit info !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}
#endif /* defined(MT7615) || defined(MT7622) */

NDIS_STATUS MtPwrGetPwrLimitInstanceSku(RTMP_ADAPTER *pAd, ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE eInstanceIdx, PVOID *ppvBuffer)
{
	switch (eInstanceIdx) {
	case POWER_LIMIT_LINK_LIST:
		*ppvBuffer = &(pAd->PwrLimitSkuList);
		break;
	case POWER_LIMIT_RAW_DATA_LENGTH:
		*ppvBuffer = pAd->u1SkuParamLen;
		break;
	case POWER_LIMIT_RAW_DATA_OFFSET:
		*ppvBuffer = pAd->u1SkuParamTransOffset;
		break;
	case POWER_LIMIT_DATA_LENGTH:
		*ppvBuffer = pAd->u1SkuFillParamLen;
		break;
	case POWER_LIMIT_CH_BAND_NEED_PARSE_BITFIELD:
		*ppvBuffer = pAd->u1SkuChBandNeedParse;
		break;
	default:
		goto error;
	}

	return NDIS_STATUS_SUCCESS;

error:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: invalid instance for sku !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtPwrGetPwrLimitInstanceBackoff(RTMP_ADAPTER *pAd, ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE eInstanceIdx, PVOID *ppvBuffer)
{
	switch (eInstanceIdx) {
	case POWER_LIMIT_LINK_LIST:
		*ppvBuffer = &(pAd->PwrLimitBackoffList);
		break;
	case POWER_LIMIT_RAW_DATA_LENGTH:
		*ppvBuffer = pAd->u1BackoffParamLen;
		break;
	case POWER_LIMIT_RAW_DATA_OFFSET:
		*ppvBuffer = pAd->u1BackoffParamTransOffset;
		break;
	case POWER_LIMIT_DATA_LENGTH:
		*ppvBuffer = pAd->u1BackoffFillParamLen;
		break;
	case POWER_LIMIT_CH_BAND_NEED_PARSE_BITFIELD:
		*ppvBuffer = pAd->u1BackoffChBandNeedParse;
		break;
	default:
		goto error;
	}

	return NDIS_STATUS_SUCCESS;

error:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: invalid instance for backoff !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtPwrGetPwrLimitInstance(RTMP_ADAPTER *pAd, POWER_LIMIT_TABLE u1Type, ENUM_POWER_LIMIT_PARAMETER_INSTANCE_TYPE eInstanceIdx, PVOID *ppvBuffer)
{
	/* get pointer of link list address */
	switch (u1Type) {
	case POWER_LIMIT_TABLE_TYPE_SKU:
		MtPwrGetPwrLimitInstanceSku(pAd, eInstanceIdx, ppvBuffer);
		break;
	case POWER_LIMIT_TABLE_TYPE_BACKOFF:
		MtPwrGetPwrLimitInstanceBackoff(pAd, eInstanceIdx, ppvBuffer);
		break;
	default:
		goto error;
	}

	return NDIS_STATUS_SUCCESS;

error:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: invalid instance type !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtPowerLimitFormatTrans(RTMP_ADAPTER *pAd, PUINT8 pu1Value, PCHAR pcRawData)
{
	CHAR *cBuffer = NULL;
	CHAR *cToken = NULL;
	UINT8 u1NonInteValue = 0;

	/* sanity check for null pointer */
	if (!pu1Value)
		goto error1;

	/* sanity check for null poitner */
	if (!pcRawData)
		goto error2;

	/* neglect multiple spaces for content parsing */
	pcRawData += strspn(pcRawData, " ");

	/* decimal point existence check */
	if (!strchr(pcRawData, '.'))
		*pu1Value = (UINT8)os_str_tol(pcRawData, 0, 10) * 2;
	else {
		/* backup pointer to string of parser function */
		cBuffer = __rstrtok;

		/* parse integer part */
		cToken = rstrtok(pcRawData, ".");

		/* sanity check for null pointer */
		if (!cToken)
			goto error3;

		/* transform integer part unit to (0.5) */
		*pu1Value = (UINT8)os_str_tol(cToken, 0, 10) * 2;

		/* parse non-integer part */
		cToken = rstrtok(NULL, ".");

		/* sanity check for null pointer */
		if (!cToken)
			goto error4;

		/* get non-integer part */
		u1NonInteValue = (UINT8)os_str_tol(cToken, 0, 10);

		/* increment for non-zero non-integer part */
		if (u1NonInteValue >= 5)
			(*pu1Value) += 1;

		/* backup pointer to string of parser function */
		__rstrtok = cBuffer;
	}

	return NDIS_STATUS_SUCCESS;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: null pointer for buffer to update transform result !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error2:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: null pointer for raw data buffer !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error3:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: null pointer for integer value parsing !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error4:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: null pointer for decimal value parsing !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

CHAR SKUTxPwrOffsetGet(RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT8 ucBW, UINT8 ucPhymode, UINT8 ucMCS, UINT8 ucNss, BOOLEAN fgSE)
{
	CHAR   cPowerOffset = 0;
	UINT8  ucRateOffset = 0;
	UINT8  ucNSS = 1;
	UINT8  BW_OFFSET[4] = {VHT20_OFFSET, VHT40_OFFSET, VHT80_OFFSET, VHT160C_OFFSET};
#ifdef CONFIG_ATE
	struct	_ATE_CTRL	*ATECtrl = &(pAd->ATECtrl);
#endif

	/* Compute MCS rate and Nss for HT mode */
	if ((ucPhymode == MODE_HTMIX) || (ucPhymode == MODE_HTGREENFIELD)) {
		ucNss = (ucMCS >> 3) + 1;
		ucMCS &= 0x7;
	}

	switch (ucPhymode) {
	case MODE_CCK:
		ucRateOffset = SKU_CCK_OFFSET;

		switch (ucMCS) {
		case MCS_0:
		case MCS_1:
			ucRateOffset = SKU_CCK_RATE_M01;
			break;

		case MCS_2:
		case MCS_3:
			ucRateOffset = SKU_CCK_RATE_M23;
			break;

		default:
			break;
		}

		break;

	case MODE_OFDM:
		ucRateOffset = SKU_OFDM_OFFSET;

		switch (ucMCS) {
		case MCS_0:
		case MCS_1:
			ucRateOffset = SKU_OFDM_RATE_M01;
			break;

		case MCS_2:
		case MCS_3:
			ucRateOffset = SKU_OFDM_RATE_M23;
			break;

		case MCS_4:
		case MCS_5:
			ucRateOffset = SKU_OFDM_RATE_M45;
			break;

		case MCS_6:
			ucRateOffset = SKU_OFDM_RATE_M6;
			break;

		case MCS_7:
			ucRateOffset = SKU_OFDM_RATE_M7;
			break;

		default:
			break;
		}

		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		ucRateOffset = SKU_HT_OFFSET + BW_OFFSET[ucBW];

		switch (ucMCS) {
		case MCS_0:
			ucRateOffset += SKU_HT_RATE_M0;
			break;

		case MCS_1:
		case MCS_2:
			ucRateOffset += SKU_HT_RATE_M12;
			break;

		case MCS_3:
		case MCS_4:
			ucRateOffset += SKU_HT_RATE_M34;
			break;

		case MCS_5:
			ucRateOffset += SKU_HT_RATE_M5;
			break;

		case MCS_6:
			ucRateOffset += SKU_HT_RATE_M6;
			break;

		case MCS_7:
			ucRateOffset += SKU_HT_RATE_M7;
			break;
		}

		break;

	case MODE_VHT:
		ucRateOffset = SKU_VHT_OFFSET + BW_OFFSET[ucBW];

		switch (ucMCS) {
		case MCS_0:
			ucRateOffset += SKU_VHT_RATE_M0;
			break;

		case MCS_1:
		case MCS_2:
			ucRateOffset += SKU_VHT_RATE_M12;
			break;

		case MCS_3:
		case MCS_4:
			ucRateOffset += SKU_VHT_RATE_M34;
			break;

		case MCS_5:
		case MCS_6:
			ucRateOffset += SKU_VHT_RATE_M56;
			break;

		case MCS_7:
			ucRateOffset += SKU_VHT_RATE_M7;
			break;

		case MCS_8:
			ucRateOffset += SKU_VHT_RATE_M8;
			break;

		case MCS_9:
			ucRateOffset += SKU_VHT_RATE_M9;
			break;

		default:
			break;
		}

		break;
	}

	/* Update Power offset by look up Tx Power Compensation Table */
	cPowerOffset = (fgSE) ? (pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucRateOffset][ucNSS - 1]) : (pAd->CommonCfg.cTxPowerCompBackup[ucBandIdx][ucRateOffset][3]);

	/* Debug log for SKU Power offset to compensate */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 (KBLU "%s: ucBW: %d, ucPhymode: %d, ucMCS: %d, ucNss: %d, fgSPE: %d !!!\n" KNRM, __func__, ucBW, ucPhymode, ucMCS,
			  ucNss, fgSE));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (KBLU "%s: cPowerOffset: 0x%x (%d) !!!\n" KNRM, __func__,
			 cPowerOffset, cPowerOffset));

#ifdef CONFIG_ATE
	/* Check if Single SKU is disabled */
	if (!ATECtrl->tx_pwr_sku_en)
		cPowerOffset = 0;
#endif
	return cPowerOffset;
}

VOID MtPwrLimitTblChProc(RTMP_ADAPTER *pAd, UINT8 u1BandIdx, UINT8 u1ChannelBand, UINT8 u1ControlChannel, UINT8 u1CentralChannel)
{
	UINT8 u1Type;

	for (u1Type = POWER_LIMIT_TABLE_TYPE_SKU; u1Type < POWER_LIMIT_TABLE_TYPE_NUM; u1Type++) {
		if (pAd->fgPwrLimitRead[u1Type])
			MtCmdPwrLimitTblUpdate(pAd, u1BandIdx, u1Type, u1ChannelBand, u1ControlChannel, u1CentralChannel);
	}
}

NDIS_STATUS MtPwrFillLimitParam(RTMP_ADAPTER *pAd, UINT8 ChBand, UINT8 u1ControlChannel,
				UINT8 u1CentralChannel, VOID *pi1PwrLimitParam, UINT8 u1Type)
{
	UINT8 u1RateIdx, u1FillParamType, u1ParseParamType, u1ParamIdx, u1ParamIdx2, u1ChListIdx;
	PUINT8 pu1FillParamTypeLen = NULL;
	PUINT8 pu1RawDataIdxOffset = NULL;
	P_CH_POWER_V1 prPwrLimitTbl, prTempPwrLimitTbl;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 u1TypeFillNum[TABLE_PARSE_TYPE_NUM] = {pChipCap->single_sku_type_num, pChipCap->backoff_type_num};
	UINT8 u1PwrLimitChannel;
	PDL_LIST pList = NULL;

	/* sanity check for null pointer */
	if (!pi1PwrLimitParam)
		goto error0;

	/* update power limit value data length */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_DATA_LENGTH, (PVOID *)&pu1FillParamTypeLen);

	/* sanity check for null pointer */
	if (!pu1FillParamTypeLen)
		goto error1;

	/* update power limit value raw data offset */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_RAW_DATA_OFFSET, (PVOID *)&pu1RawDataIdxOffset);

	/* update power limit link list */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_LINK_LIST, (PVOID *)&pList);

	/* sanity check for null pointer */
	if (!pu1RawDataIdxOffset)
		goto error1;

	if (u1ControlChannel >= 16) /* must be 5G */
		ChBand = 1;
	else if (u1ControlChannel <= 8) /* must be 2.4G */
		ChBand = 0;

	/* Check G-Band/A-Band and power limit channel */
	u1PwrLimitChannel = (ChBand) ? (u1ControlChannel) : (u1CentralChannel);

	DlListForEachSafe(prPwrLimitTbl, prTempPwrLimitTbl, pList, CH_POWER_V1, List) {
		/* search for specific channel */
		for (u1ChListIdx = 0; u1ChListIdx < prPwrLimitTbl->u1ChNum; u1ChListIdx++) {
			/* check Channel Band and Channel */
			if ((ChBand == prPwrLimitTbl->u1ChBand) && (u1PwrLimitChannel == prPwrLimitTbl->pu1ChList[u1ChListIdx])) {
				/* update sku parameter for cck, ofdm, ht20/40, vht20/40/80/160 to buffer */
				for (u1FillParamType = 0, u1ParseParamType = 0, u1ParamIdx = 0, u1ParamIdx2 = 0; u1FillParamType < u1TypeFillNum[u1Type]; u1FillParamType++, u1ParseParamType++) {
					/* raw data index increment for different parameter type */
					u1ParamIdx2 = *(pu1RawDataIdxOffset + u1ParseParamType);

					for (u1RateIdx = 0; u1RateIdx < *(pu1FillParamTypeLen + u1FillParamType); u1RateIdx++) {

						/* init power limit value */
						*((INT8 *)pi1PwrLimitParam + u1ParamIdx + u1RateIdx) = (0x3F);

						/* special case handler for ht40 mcs32 */
						if ((u1Type == POWER_LIMIT_TABLE_TYPE_SKU) &&
							(u1FillParamType == pChipCap->single_sku_tbl_type_ht40) &&
							(u1RateIdx == (pChipCap->single_sku_fill_tbl_ht40 - 1))) {
							if (prPwrLimitTbl->pu1PwrLimit + u1ParamIdx2 + (pChipCap->single_sku_parse_tbl_htvht40 - 1))
								*((INT8 *)pi1PwrLimitParam + u1ParamIdx + u1RateIdx) = *(prPwrLimitTbl->pu1PwrLimit + u1ParamIdx2 + (pChipCap->single_sku_parse_tbl_htvht40 - 1));
							u1ParseParamType = u1ParseParamType - 2;
						} else if ((u1Type == POWER_LIMIT_TABLE_TYPE_BACKOFF) &&
							(u1FillParamType == pChipCap->backoff_tbl_bf_on_type_ht40) &&
							(u1RateIdx == (pChipCap->backoff_tbl_bfon_ht40 - 1))) {
							if (prPwrLimitTbl->pu1PwrLimit + u1ParamIdx2 + u1RateIdx)
								*((INT8 *)pi1PwrLimitParam + u1ParamIdx + u1RateIdx) = *(prPwrLimitTbl->pu1PwrLimit + u1ParamIdx2 + u1RateIdx);
							u1ParseParamType = u1ParseParamType - 4;
						} else {
							if (prPwrLimitTbl->pu1PwrLimit + u1ParamIdx2 + u1RateIdx)
								*((INT8 *)pi1PwrLimitParam + u1ParamIdx + u1RateIdx) = *(prPwrLimitTbl->pu1PwrLimit + u1ParamIdx2 + u1RateIdx);
						}
					}

					/* data index increment for different parameter type */
					u1ParamIdx += *(pu1FillParamTypeLen + u1FillParamType);
				}

				/* stop channel list search loop */
				break;
			}
		}
	}

	return NDIS_STATUS_SUCCESS;

error0:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for buffer to fill power limit table !!\n", __func__));
	return NDIS_STATUS_FAILURE;

error1:
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: null pointer for parameter related to fill power limit table proc !!\n", __func__));
	return NDIS_STATUS_FAILURE;
}

