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
#if defined(AXE) || defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
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

NDIS_STATUS MtPwrLimitLoadParamHandle(RTMP_ADAPTER *pAd, UINT8 u1Type)
{
	PCHAR pi1Buffer = NULL;
	PDL_LIST pList = NULL;

	/* get pointer of link list address */
	MtPwrGetPwrLimitInstance(pAd, u1Type, POWER_LIMIT_LINK_LIST, (PVOID *)&pList);

	/* sanity check for null pointer */
	if (!pList)
		goto error4;

	/* Link list Init */
	DlListInit(pList);


	/* update buffer with power limit table content */
	if (NDIS_STATUS_SUCCESS != MtReadPwrLimitTable(pAd, &pi1Buffer, u1Type))
		goto error1;

	/* parsing sku table contents from buffer */
	if (NDIS_STATUS_SUCCESS != MtParsePwrLimitTable(pAd, &pi1Buffer, u1Type))
		goto error2;

	/* enable flag for Read Power limit table pass */
		pAd->fgPwrLimitRead[u1Type] = TRUE;

	/* print out power limit table info */
	if (NDIS_STATUS_SUCCESS != MtShowPwrLimitTable(pAd, u1Type, DBG_LVL_DEBUG))
		goto error3;

	/* free allocated memory */
	os_free_mem(pi1Buffer);
	return NDIS_STATUS_SUCCESS;

error1:
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Read Power Table Error!!\n");
	/* free allocated memory */
	os_free_mem(pi1Buffer);
	return NDIS_STATUS_FAILURE;

error2:
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Parse Power Table Error!!\n");
	/* free allocated memory */
	os_free_mem(pi1Buffer);
	return NDIS_STATUS_FAILURE;

error3:
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Show Power Table Error!!\n");
	/* free allocated memory */
	os_free_mem(pi1Buffer);
	return NDIS_STATUS_FAILURE;

error4:
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pointer for link list!!\n");
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
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pointer for link list!!\n");
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtParsePwrLimitTable(RTMP_ADAPTER *pAd, PCHAR *pi1Buffer, UINT8 u1Type)
{
	UINT8 u1ChBand = CH_G_BAND;
	PCHAR pcReadline, pcToken, pcptr;
	UINT8 u1Channel = 0;
	UINT8 *prTempChList;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 u1PwrLimitParamNum[TABLE_PARSE_TYPE_NUM] = {pChipCap->single_sku_para_parse_num, pChipCap->backoff_para_parse_num};
	P_CH_POWER_V1 prTbl = NULL, prStartCh = NULL;

	/* sanity check for null pointer */
	if (!(*pi1Buffer))
		goto error;

	for (pcReadline = pcptr = (*pi1Buffer); (pcptr = os_str_chr(pcReadline, '\t')) != NULL; pcReadline = pcptr + 1) {
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
				MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "ChBand: CH_G_BAND\n");
				break;
			case 5:
				u1ChBand = CH_A_BAND;
				MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "ChBand: CH_A_BAND\n");
				break;
			case 6:
				u1ChBand = CH_6G_BAND;
				MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "ChBand: CH_6G_BAND\n");
				break;
			default:
				MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s - u1ChBand: %d\n", __func__, u1ChBand);
				break;
			}
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
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pointer when parsing power limit table !!\n");
	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS MtReadPwrLimitTable(RTMP_ADAPTER *pAd, PCHAR *pi1Buffer, UINT8 u1Type)
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

	UINT32 Sku_sizeof[TABLE_SIZE] = {sizeof(Sku_01), sizeof(Sku_02), sizeof(Sku_03), sizeof(Sku_04), sizeof(Sku_05),
		sizeof(Sku_06), sizeof(Sku_07), sizeof(Sku_08), sizeof(Sku_09), sizeof(Sku_10),
		sizeof(Sku_11), sizeof(Sku_12), sizeof(Sku_13), sizeof(Sku_14), sizeof(Sku_15),
		sizeof(Sku_16), sizeof(Sku_17), sizeof(Sku_18), sizeof(Sku_19), sizeof(Sku_20)};

	UINT32 Backoff_sizeof[TABLE_SIZE] = {sizeof(Backoff_01), sizeof(Backoff_02), sizeof(Backoff_03), sizeof(Backoff_04), sizeof(Backoff_05),
		sizeof(Backoff_06), sizeof(Backoff_07), sizeof(Backoff_08), sizeof(Backoff_09), sizeof(Backoff_10),
		sizeof(Backoff_11), sizeof(Backoff_12), sizeof(Backoff_13), sizeof(Backoff_14), sizeof(Backoff_15),
		sizeof(Backoff_16), sizeof(Backoff_17), sizeof(Backoff_18), sizeof(Backoff_19), sizeof(Backoff_20)};

	/* query sku table index */
	chip_get_sku_tbl_idx(pAd, &sku_tbl_idx);
	if (sku_tbl_idx >= TABLE_SIZE)
		return NDIS_STATUS_FAILURE;

	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		KBLU " sku table idx: %d\n" KNRM, sku_tbl_idx);

	/* update buffer with sku table content */
	if (POWER_LIMIT_TABLE_TYPE_SKU == u1Type) {
		/* allocate memory for buffer power limit value , caller need to free the memoey */
		os_alloc_mem(pAd, (UINT8 **)pi1Buffer, Sku_sizeof[sku_tbl_idx]);
		os_move_mem((*pi1Buffer), pcptrSkuTbl[sku_tbl_idx], Sku_sizeof[sku_tbl_idx]);
	} else if (POWER_LIMIT_TABLE_TYPE_BACKOFF == u1Type) {
		/* allocate memory for buffer power limit value , caller need to free the memoey */
		os_alloc_mem(pAd, (UINT8 **)pi1Buffer, Backoff_sizeof[sku_tbl_idx]);
		os_move_mem((*pi1Buffer), pcptrBackoffTbl[sku_tbl_idx], Backoff_sizeof[sku_tbl_idx]);
	}

	/* sanity check for null pointer */
	if (!(*pi1Buffer))
		goto error1;

	return NDIS_STATUS_SUCCESS;

error1:
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pointer for buffer to read power limit table !!\n");
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
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pointer for buffer to update power limit table after parsing !!\n");
	return NDIS_STATUS_FAILURE;

error1:
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pointer for parameter related to parse power limit table proc !!\n");
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
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pointer for pointer to power limit table start channel for check !!\n");
	return NDIS_STATUS_FAILURE;

error2:
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pointer for pointer to power limit table current channel for check !!\n");
	return NDIS_STATUS_FAILURE;

error3:
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pointer for parameter related to power limit table proc similar check !!\n");
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
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pointer for parameter related to show power limit table !!\n");
	return NDIS_STATUS_FAILURE;

error1:
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pointer for list of power limit table to show power limit info !!\n");
	return NDIS_STATUS_FAILURE;
}

#ifdef TPC_SUPPORT
static UINT8 getSkuTblIdx(UINT8 TxMode, UINT8 MCSRate)
{
	UINT8 idx = 0;

	if (TxMode == POWER_LIMIT_TX_MODE_CCK) {
		idx = MCSRate;
	} else if (TxMode == POWER_LIMIT_TX_MODE_OFDM) {
		idx = POWER_LIMIT_CCK_NUM + MCSRate;
	} else if (TxMode == POWER_LIMIT_TX_MODE_HTVHT20) {
		idx = POWER_LIMIT_CCK_NUM + POWER_LIMIT_OFDM_NUM + MCSRate;
	} else if (TxMode == POWER_LIMIT_TX_MODE_HTVHT40) {
		if (MCSRate != MCS_32)
			idx = POWER_LIMIT_CCK_NUM + POWER_LIMIT_OFDM_NUM + (TxMode - 2)*12 + MCSRate;
		else
			idx = POWER_LIMIT_CCK_NUM + POWER_LIMIT_OFDM_NUM + (TxMode - 1)*12;
	} else if (TxMode > POWER_LIMIT_TX_MODE_HTVHT40 && TxMode < POWER_LIMIT_TX_MODE_NUM) {
		idx = POWER_LIMIT_CCK_NUM + POWER_LIMIT_OFDM_NUM + (TxMode - 2)*12 + MCSRate + 1;
	}
	return idx;
}

UINT8 GetSkuTxPwr(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, USHORT FCSubType)
{
	UINT8 u1ChListIdx;
	P_CH_POWER_V1 prPwrLimitTbl, prTempPwrLimitTbl;
	PDL_LIST pList = &(pAd->PwrLimitSkuList);
	INT8 pwr = 0;
	UINT8 ChBand = 0;
	BOOLEAN IsFound = FALSE;
	UCHAR bandIdx = HcGetBandByWdev(wdev);
	HTTRANSMIT_SETTING *transmit;
	UINT8 skuTblIdx;

	if (!pList) {
		/* return max Tx Pwr, if can't read from PowerLimitTable */
		pwr = GetMaxTxPwr(pAd);
		return pwr;
	}

	/* return already stored value for current channel */
	if (pAd->TxPwrParsedChannel[bandIdx] == wdev->channel) {
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
		if ((FCSubType == SUBTYPE_BEACON) && (wdev->eap.eap_bcnrate_en)) {
			if (pAd->SkuBcnTxPwr[bandIdx]) {
				pwr = pAd->SkuBcnTxPwr[bandIdx];
				goto Done;
			}
		} else if (pAd->SkuMgmtTxPwr[bandIdx])
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
		{
			pwr = pAd->SkuMgmtTxPwr[bandIdx];
			goto Done;
		}
	} else {
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
		pAd->SkuBcnTxPwr[bandIdx] = 0;
#endif
		pAd->SkuMgmtTxPwr[bandIdx] = 0;
	}

	/* Assign channel Band */
	if (WMODE_CAP_2G(wdev->PhyMode))
		ChBand = 0;
	else if (WMODE_CAP_5G(wdev->PhyMode))
		ChBand = 1;
	else if (WMODE_CAP_6G(wdev->PhyMode))
		ChBand = 2;

	transmit = &wdev->rate.MlmeTransmit;
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	if ((FCSubType == SUBTYPE_BEACON) && (wdev->eap.eap_bcnrate_en))
		transmit = &wdev->eap.bcnphymode;
	else if (wdev->eap.eap_mgmrate_en)
		transmit = &wdev->eap.mgmphymode;
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

	skuTblIdx = getSkuTblIdx(transmit->field.MODE, transmit->field.MCS);
	DlListForEachSafe(prPwrLimitTbl, prTempPwrLimitTbl, pList, CH_POWER_V1, List) {
		/* check correct chBand */
		if ((prPwrLimitTbl->u1ChBand == ChBand) && (IsFound == FALSE)) {
			/* search for specific channel */
			for (u1ChListIdx = 0; u1ChListIdx < prPwrLimitTbl->u1ChNum; u1ChListIdx++) {
				/* check Channel Band and Channel */
				if (prPwrLimitTbl->pu1ChList[u1ChListIdx] == wdev->channel) {
					pwr = (*(prPwrLimitTbl->pu1PwrLimit + skuTblIdx))/2;
					MTWF_DBG(pAd, DBG_SUBCAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"ch:%d, ChBand:%d, power: %d\n", prPwrLimitTbl->pu1ChList[u1ChListIdx],
						prPwrLimitTbl->u1ChBand, pwr);
					IsFound = TRUE;
					break;
				}
			}
		}
	}
	pAd->TxPwrParsedChannel[bandIdx] = wdev->channel;
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	if ((FCSubType == SUBTYPE_BEACON) && (wdev->eap.eap_bcnrate_en))
		pAd->SkuBcnTxPwr[bandIdx] = pwr;
	else
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
	pAd->SkuMgmtTxPwr[bandIdx] = pwr;

Done:
	if (pAd->CommonCfg.ucTxPowerPercentage[bandIdx]) {
		INT8 cPowerDropLevel = 0;
		UINT8 ucPowerDrop = pAd->CommonCfg.ucTxPowerPercentage[bandIdx];

		/* Tx Power Drop value */
		if ((ucPowerDrop > 90) && (ucPowerDrop < 100))
			cPowerDropLevel = 0;
		else if ((ucPowerDrop > 60) && (ucPowerDrop <= 90))  /* reduce Pwr for 1 dB. */
			cPowerDropLevel = 1;
		else if ((ucPowerDrop > 30) && (ucPowerDrop <= 60))  /* reduce Pwr for 3 dB. */
			cPowerDropLevel = 3;
		else if ((ucPowerDrop > 15) && (ucPowerDrop <= 30))  /* reduce Pwr for 6 dB. */
			cPowerDropLevel = 6;
		else if ((ucPowerDrop > 9) && (ucPowerDrop <= 15))   /* reduce Pwr for 9 dB. */
			cPowerDropLevel = 9;
		else if ((ucPowerDrop > 0) && (ucPowerDrop <= 9))   /* reduce Pwr for 12 dB. */
			cPowerDropLevel = 12;

		pwr -= cPowerDropLevel;
	}

	MTWF_DBG(pAd, DBG_SUBCAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"Tx Power = %d!!!\n\n", pwr);

	return pwr;
}
#endif /* TPC_SUPPORT */


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
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid instance for sku !!\n");
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
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid instance for backoff !!\n");
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
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid instance type !!\n");
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
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pointer for buffer to update transform result !!\n");
	return NDIS_STATUS_FAILURE;

error2:
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pointer for raw data buffer !!\n");
	return NDIS_STATUS_FAILURE;

error3:
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"null pointer for integer value parsing !!\n");
	return NDIS_STATUS_FAILURE;

error4:
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"null pointer for decimal value parsing !!\n");
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
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 KBLU "%s: ucBW: %d, ucPhymode: %d, ucMCS: %d, ucNss: %d, fgSPE: %d !!!\n" KNRM, __func__, ucBW, ucPhymode, ucMCS,
			  ucNss, fgSE);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, KBLU "%s: cPowerOffset: 0x%x (%d) !!!\n" KNRM, __func__,
			 cPowerOffset, cPowerOffset);

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
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pointer for buffer to fill power limit table !!\n");
	return NDIS_STATUS_FAILURE;

error1:
	MTWF_DBG(pAd, DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "null pointer for parameter related to fill power limit table proc !!\n");
	return NDIS_STATUS_FAILURE;
}

