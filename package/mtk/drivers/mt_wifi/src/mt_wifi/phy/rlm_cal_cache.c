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
	rlm_cal_cache.c
*/

/*#include "rtmp_type.h" */
/*#include "os/rt_linux.h" */
/*#include "common/mt_os_util.h" */

#include "rt_config.h"
#include "phy/rlm_cal_cache.h"


INT rlmCalCacheDone(VOID *rlmCalCache)
{
	return RLM_CAL_CACHE_IS_DONE(rlmCalCache);
}

INT rlmCalCacheInit(VOID *pAd, VOID **ppRlmCalCache)
{
	if (os_alloc_mem(pAd, (UCHAR **)ppRlmCalCache, sizeof(RLM_CAL_CACHE)) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		    "fail to alloca rlmCalCache size=%zu\n", sizeof(RLM_CAL_CACHE));
		return FALSE;
	}
	os_zero_mem(*ppRlmCalCache, sizeof(RLM_CAL_CACHE));

	return TRUE;
}

INT rlmCalCacheDeinit(VOID **ppRlmCalCache)
{
	if (*ppRlmCalCache == NULL)
		return FALSE;

	os_free_mem(*ppRlmCalCache);
	*ppRlmCalCache = NULL;
	return TRUE;
}

INT RlmCalCacheTxLpfInfo(VOID *rlmCalCtrl, UINT8 *Data, UINT32 Length)
{
	P_TXLPF_CAL_INFO_T ptxLpfCalInfo;
	UINT32 i;
	if (Length != sizeof(TXLPF_CAL_INFO_T)) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"P_TXLPF_CAL_INFO_T: length mismatch=%d\n", Length);

		return FALSE;
	}
	//hex_dump("P_TXLPF_CAL_INFO", Data, Length);

	if (rlmCalCtrl == NULL)
		return FALSE;
	ptxLpfCalInfo = (P_TXLPF_CAL_INFO_T)Data;
	ptxLpfCalInfo->u2BitMap = le2cpu16(ptxLpfCalInfo->u2BitMap);
	for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM; i++)
		ptxLpfCalInfo->au4Data[i] = le2cpu32(ptxLpfCalInfo->au4Data[i]);

	os_move_mem((PVOID)&RLM_CAL_CACHE_TXLPF_CAL_INFO(rlmCalCtrl), Data, Length);
	RLM_CAL_CACHE_TXLPF_CAL_DONE(rlmCalCtrl);

	return TRUE;
}

INT RlmCalCacheTxIqInfo(VOID *rlmCalCtrl, UINT8 *Data, UINT32 Length)
{
	P_TXIQ_CAL_INFO_T ptxIqCalInfo;
	UINT32 i;
	if (Length != sizeof(TXIQ_CAL_INFO_T)) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"P_TXIQ_CAL_INFO_T: length mismatch=%d\n", Length);

		return FALSE;
	}
	//hex_dump("P_TXIQ_CAL_INFO", Data, Length);

	if (rlmCalCtrl == NULL)
		return FALSE;
	ptxIqCalInfo = (P_TXIQ_CAL_INFO_T)Data;
	ptxIqCalInfo->u2BitMap = le2cpu16(ptxIqCalInfo->u2BitMap);
	for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM*6; i++)
		ptxIqCalInfo->au4Data[i] = le2cpu32(ptxIqCalInfo->au4Data[i]);

	os_move_mem((PVOID)&RLM_CAL_CACHE_TXIQ_CAL_INFO(rlmCalCtrl), Data, Length);
	RLM_CAL_CACHE_TXIQ_CAL_DONE(rlmCalCtrl);

	return TRUE;
}

INT RlmCalCacheTxDcInfo(VOID *rlmCalCtrl, UINT8 *Data, UINT32 Length)
{
	P_TXDC_CAL_INFO_T ptxDcCalInfo;
	UINT32 i;
	if (Length != sizeof(TXDC_CAL_INFO_T)) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"P_TXDC_CAL_INFO_T: length mismatch=%d\n", Length);

		return FALSE;
	}
	//hex_dump("P_TXDC_CAL_INFO", Data, Length);

	if (rlmCalCtrl == NULL)
		return TRUE;
	ptxDcCalInfo = (P_TXDC_CAL_INFO_T)Data;
	ptxDcCalInfo->u2BitMap = le2cpu16(ptxDcCalInfo->u2BitMap);
	for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM*6; i++)
		ptxDcCalInfo->au4Data[i] = le2cpu32(ptxDcCalInfo->au4Data[i]);

	os_move_mem((PVOID)&RLM_CAL_CACHE_TXDC_CAL_INFO(rlmCalCtrl), Data, Length);
	RLM_CAL_CACHE_TXDC_CAL_DONE(rlmCalCtrl);

	return TRUE;
}

INT RlmCalCacheRxFiInfo(VOID *rlmCalCtrl, UINT8 *Data, UINT32 Length)
{
	P_RXFI_CAL_INFO_T rxfi_event;
	UINT32 i;

	if (Data == NULL)
		return FALSE;

	if (Length != sizeof(RXFI_CAL_INFO_T)) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"P_RXFI_CAL_INFO_T: length mismatch=%d\n", Length);

		return FALSE;
	}

	rxfi_event = (P_RXFI_CAL_INFO_T)Data;

	//hex_dump("P_RXFI_CAL_INFO", Data, Length);

	if (rlmCalCtrl == NULL)
		return FALSE;
	/*os_move_mem((PVOID)RLM_CAL_CACHE_RXFI_CAL_INFO(rlmCalCtrl).au4Data,
		rxfi_event->au4Data, sizeof(rxfi_event->au4Data));*/
	rxfi_event->u2BitMap = le2cpu16(rxfi_event->u2BitMap);
	for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM*4; i++)
		rxfi_event->au4Data[i] = le2cpu32(rxfi_event->au4Data[i]);

    os_move_mem((PVOID)&RLM_CAL_CACHE_RXFI_CAL_INFO(rlmCalCtrl),
        Data, Length);

	RLM_CAL_CACHE_RXFI_CAL_DONE(rlmCalCtrl);

	return TRUE;

}

INT RlmCalCacheRxFdInfo(VOID *rlmCalCtrl, UINT8 *Data, UINT32 Length)
{
	P_RXFD_CAL_INFO_T rxfd_event;
	UINT32 ch_group_id, i;

	if (Data == NULL)
		return FALSE;

	if (Length != sizeof(RXFD_CAL_INFO_T)) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"P_RXFD_CAL_INFO_T: length mismatch=%d\n", Length);

		return FALSE;
	}

	rxfd_event = (P_RXFD_CAL_INFO_T)Data;
	rxfd_event->u4ChGroupId = le2cpu32(rxfd_event->u4ChGroupId);
	ch_group_id = rxfd_event->u4ChGroupId;
	rxfd_event->u2BitMap = le2cpu16(rxfd_event->u2BitMap);
	for (i = 0;
	i < (SCN_NUM*RX_SWAGC_LNA_NUM)+(SCN_NUM*RX_FDIQ_LPF_GAIN_NUM*RX_FDIQ_TABLE_SIZE*3);
	i++)
		rxfd_event->au4Data[i] = le2cpu32(rxfd_event->au4Data[i]);

	//hex_dump("P_RXFD_CAL_INFO", Data, Length);

	if (rlmCalCtrl == NULL)
		return FALSE;

	if (rxfd_event->u4ChGroupId >= CHANNEL_GROUP_NUM)
		return FALSE;

	/*os_move_mem((PVOID)(RLM_CAL_CACHE_RXFD_CAL_INFO(rlmCalCtrl, rxfd_event->u4ChGroupId).au4Data),
		rxfd_event->au4Data, sizeof(rxfd_event->au4Data));*/
    os_move_mem((PVOID)(&RLM_CAL_CACHE_RXFD_CAL_INFO(rlmCalCtrl, rxfd_event->u4ChGroupId)),
        Data, Length);

	RLM_CAL_CACHE_RXFD_CAL_DONE(rlmCalCtrl, rxfd_event->u4ChGroupId);

	return TRUE;

}

VOID rlmCalCacheStatus(VOID *rlmCalCache)
{
	UINT idx;

	MTWF_PRINT("TxLPF Cal Done::(%c)\n",
				 RLM_CAL_CACHE_IS_TXLPF_CAL_DONE(rlmCalCache) ? 'T':'F');

	MTWF_PRINT("TxIQ Cal Done::(%c)\n",
				 RLM_CAL_CACHE_IS_TXIQ_CAL_DONE(rlmCalCache) ? 'T':'F');

	MTWF_PRINT("TxDC Cal Done::(%c)\n",
				 RLM_CAL_CACHE_IS_TXDC_CAL_DONE(rlmCalCache) ? 'T':'F');

	MTWF_PRINT("RxFI Cal Done::(%c)\n",
				 RLM_CAL_CACHE_IS_RXFI_CAL_DONE(rlmCalCache) ? 'T':'F');

	for (idx = 0; idx < 9; idx++) {
		MTWF_PRINT("RxFD(%d) Cal Done::(%c)\n", idx,
			RLM_CAL_CACHE_IS_RXFD_CAL_DONE(rlmCalCache, idx) ? 'T':'F');
	}

	/* return; */
}

VOID rlmCalCacheDump(VOID *rlmCalCache)
{
	UINT idx;

	if (RLM_CAL_CACHE_IS_TXLPF_CAL_DONE(rlmCalCache))
		hex_dump("TxLpf", (UCHAR *)(RLM_CAL_CACHE_TXLPF_CAL_INFO(rlmCalCache).au4Data),
			sizeof(RLM_CAL_CACHE_TXLPF_CAL_INFO(rlmCalCache).au4Data));

	if (RLM_CAL_CACHE_IS_TXIQ_CAL_DONE(rlmCalCache))
		hex_dump("TxIq", (UCHAR *)(RLM_CAL_CACHE_TXIQ_CAL_INFO(rlmCalCache).au4Data),
			sizeof(RLM_CAL_CACHE_TXIQ_CAL_INFO(rlmCalCache).au4Data));

	if (RLM_CAL_CACHE_IS_TXDC_CAL_DONE(rlmCalCache))
		hex_dump("TxDc", (UCHAR *)(RLM_CAL_CACHE_TXDC_CAL_INFO(rlmCalCache).au4Data),
			sizeof(RLM_CAL_CACHE_TXDC_CAL_INFO(rlmCalCache).au4Data));

	if (RLM_CAL_CACHE_IS_RXFI_CAL_DONE(rlmCalCache))
		hex_dump("RxFi", (UCHAR *)(RLM_CAL_CACHE_RXFI_CAL_INFO(rlmCalCache).au4Data),
			sizeof(RLM_CAL_CACHE_RXFI_CAL_INFO(rlmCalCache).au4Data));

	for (idx = 0; idx < 9; idx++) {
		if (RLM_CAL_CACHE_IS_RXFD_CAL_DONE(rlmCalCache, idx)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"RxFD(%d)\n", idx);
			hex_dump("RxFd", (UCHAR *)(RLM_CAL_CACHE_RXFD_CAL_INFO(rlmCalCache, idx).au4Data),
				sizeof(RLM_CAL_CACHE_RXFD_CAL_INFO(rlmCalCache, idx).au4Data));
		}
	}

	/*return;*/
}
#ifdef PRE_CAL_MT7622_SUPPORT
INT TxLpfCalInfoAlloc_7622(RTMP_ADAPTER *pAd, VOID **pptr)
{
	P_TXLPF_CAL_INFO_MT7622_T pTxLpfCalInfo;
	if (IS_MT7622(pAd)) {
		if (os_alloc_mem(pAd, (UCHAR **)&pTxLpfCalInfo, sizeof(TXLPF_CAL_INFO_MT7622_T)) != NDIS_STATUS_SUCCESS)
			return 0;

		*pptr = pTxLpfCalInfo;
		os_zero_mem(pTxLpfCalInfo, sizeof(TXLPF_CAL_INFO_MT7622_T));
		os_move_mem(pTxLpfCalInfo->au4Data, pAd->CalTXLPFGImage, CAL_TXLPFG_SIZE);
	}
	return sizeof(TXLPF_CAL_INFO_MT7622_T);
}

INT TxDcIqCalInfoAlloc_7622(RTMP_ADAPTER *pAd, VOID **pptr)
{
	P_TXDCIQ_CAL_INFO_T pTxDcIqCalInfo;

	if (IS_MT7622(pAd)) {
	if (os_alloc_mem(pAd, (UCHAR **)&pTxDcIqCalInfo, sizeof(TXDCIQ_CAL_INFO_T)) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pTxDcIqCalInfo;
	os_zero_mem(pTxDcIqCalInfo, sizeof(TXDCIQ_CAL_INFO_T));
	os_move_mem(pTxDcIqCalInfo->au4Data, pAd->CalTXDCIQImage, CAL_TXDCIQ_SIZE);
	}
	return sizeof(TXDCIQ_CAL_INFO_T);
}

INT TxDpdCalInfoAlloc_7622(RTMP_ADAPTER *pAd, VOID **pptr, UINT32 chan)
{
	P_TXDPD_CAL_INFO_T pTxDpdCalInfo;

	if (os_alloc_mem(pAd, (UCHAR **)&pTxDpdCalInfo, sizeof(TXDPD_CAL_INFO_T)) != NDIS_STATUS_SUCCESS)
		return 0;

	if (IS_MT7622(pAd)) {
	*pptr = pTxDpdCalInfo;
	os_zero_mem(pTxDpdCalInfo, sizeof(TXDPD_CAL_INFO_T));
	pTxDpdCalInfo->u4Chan = chan;
	os_move_mem(pTxDpdCalInfo->au4Data, pAd->CalTXDPDImage + CAL_TXDPD_PERCHAN_SIZE * (chan-1),
		CAL_TXDPD_PERCHAN_SIZE);
	}
	return sizeof(TXDPD_CAL_INFO_T);
}
#endif /* PRE_CAL_MT7622_SUPPORT */

#ifdef PRE_CAL_MT7626_SUPPORT
INT GroupPreCalInfoAlloc_7626(RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length)
{
	UINT32 i = 0;
	P_PRE_CAL_INFO_MT7626_T pGroupPreCalInfo;

	if (os_alloc_mem(pAd, (UCHAR **)&pGroupPreCalInfo, sizeof(PRE_CAL_INFO_MT7626_T)) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pGroupPreCalInfo;
	os_zero_mem(pGroupPreCalInfo, sizeof(PRE_CAL_INFO_MT7626_T));

	pGroupPreCalInfo->ucIndex  = idx;
	pGroupPreCalInfo->u4Length = length;

	/* Due to PreCalImage is UCHAR, then we should multiple 4 to become UINT32 */
	os_move_mem(pGroupPreCalInfo->au4Data, pAd->PreCalImage + (idx * PRE_CAL_SET_MAX_LENGTH), length);
	return sizeof(PRE_CAL_INFO_MT7626_T);
}

INT DpdFlatnessCalInfoAlloc_7626(RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length)
{
	UINT32 i = 0;
	P_PRE_CAL_INFO_MT7626_T pDpdFlatnessCalInfo;

	if (os_alloc_mem(pAd, (UCHAR **)&pDpdFlatnessCalInfo, sizeof(PRE_CAL_INFO_MT7626_T)) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pDpdFlatnessCalInfo;
	os_zero_mem(pDpdFlatnessCalInfo, sizeof(PRE_CAL_INFO_MT7626_T));

	pDpdFlatnessCalInfo->ucIndex  = idx;
	pDpdFlatnessCalInfo->u4Length = length;

	/* Due to PreCalImage is UCHAR, then we should multiple 4 to become UINT32 */
    if (length == DPD_FLATNESS_2G_CAL_SIZE) {
		pDpdFlatnessCalInfo->ucBand = GBAND;
		os_move_mem(pDpdFlatnessCalInfo->au4Data,
			pAd->TxDPDImage + ((DPD_FLATNESS_5G_CHAN_NUM * DPD_FLATNESS_5G_CAL_SIZE) + ((idx-DPD_FLATNESS_5G_CHAN_NUM) * length)),
			length);
	} else {
		pDpdFlatnessCalInfo->ucBand = ABAND;
		os_move_mem(pDpdFlatnessCalInfo->au4Data, pAd->TxDPDImage + (idx * length), length);
	}
	return sizeof(PRE_CAL_INFO_MT7626_T);
}
#endif /* PRE_CAL_MT7626_SUPPORT */

#ifdef PRE_CAL_MT7915_SUPPORT
INT GroupPreCalInfoAlloc_7915(RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length)
{
	P_PRE_CAL_INFO_MT7915_T pGroupPreCalInfo;
	UINT32 event_len = sizeof(PRE_CAL_INFO_MT7915_T) + length;

	if (os_alloc_mem(pAd, (UCHAR **)&pGroupPreCalInfo, event_len) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pGroupPreCalInfo;
	os_zero_mem(pGroupPreCalInfo, event_len);

	pGroupPreCalInfo->ucIndex  = idx;
	pGroupPreCalInfo->u4Length = length;
#ifdef RT_BIG_ENDIAN
	pGroupPreCalInfo->u4Length = cpu2le32(pGroupPreCalInfo->u4Length);
#endif
	os_move_mem(pGroupPreCalInfo->au4Data, pAd->PreCalImage + (idx * PRE_CAL_SET_MAX_LENGTH), length);
	return event_len;
}

INT DpdFlatnessCalInfoAlloc_7915(
	RTMP_ADAPTER *pAd,
	VOID         **pptr,
	UINT16       idx,
	UINT32       length,
	BOOLEAN      bSecBw80
	)
{
	P_PRE_CAL_INFO_MT7915_T pDpdFlatnessCalInfo;
	UINT32 event_len = sizeof(PRE_CAL_INFO_MT7915_T) + length;

	if (os_alloc_mem(pAd, (UCHAR **)&pDpdFlatnessCalInfo, event_len) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pDpdFlatnessCalInfo;
	os_zero_mem(pDpdFlatnessCalInfo, event_len);

	pDpdFlatnessCalInfo->ucIndex  = idx;
	pDpdFlatnessCalInfo->u4Length = length;
#ifdef RT_BIG_ENDIAN
	pDpdFlatnessCalInfo->u4Length = cpu2le32(pDpdFlatnessCalInfo->u4Length);
#endif
	pDpdFlatnessCalInfo->ucSecBw80 = bSecBw80;
	os_move_mem(pDpdFlatnessCalInfo->au4Data, pAd->TxDPDImage + (idx * PRE_CAL_SET_MAX_LENGTH), length);


	return event_len;
}
#endif /* PRE_CAL_MT7915_SUPPORT */

#ifdef PRE_CAL_MT7986_SUPPORT
INT GroupPreCalInfoAlloc_7986(RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length)
{
	P_PRE_CAL_INFO_MT7986_T pGroupPreCalInfo;
	UINT32 event_len = sizeof(PRE_CAL_INFO_MT7986_T) + length;

	if (os_alloc_mem(pAd, (UCHAR **)&pGroupPreCalInfo, event_len) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pGroupPreCalInfo;
	os_zero_mem(pGroupPreCalInfo, event_len);

	pGroupPreCalInfo->ucIndex  = idx;
	pGroupPreCalInfo->u4Length = length;

	os_move_mem(pGroupPreCalInfo->au4Data, pAd->PreCalImage + (idx * PRE_CAL_SET_MAX_LENGTH), length);
	return event_len;
}

INT DpdFlatnessCalInfoAlloc_7986(
	RTMP_ADAPTER *pAd,
	VOID         **pptr,
	UINT16       idx,
	UINT32       length,
	UINT32       eeprom_offset
	)
{
	P_PRE_CAL_INFO_MT7986_T pDpdFlatnessCalInfo;
	UINT32 event_len = sizeof(PRE_CAL_INFO_MT7986_T) + length;

	if (os_alloc_mem(pAd, (UCHAR **)&pDpdFlatnessCalInfo, event_len) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pDpdFlatnessCalInfo;
	os_zero_mem(pDpdFlatnessCalInfo, event_len);

	pDpdFlatnessCalInfo->ucIndex  = idx;
	pDpdFlatnessCalInfo->u4Length = length;
#ifdef RT_BIG_ENDIAN
	pDpdFlatnessCalInfo->u4Length = cpu2le32(pDpdFlatnessCalInfo->u4Length);
#endif
	////pDpdFlatnessCalInfo->ucSecBw80 = bSecBw80;
	os_move_mem(pDpdFlatnessCalInfo->au4Data, pAd->TxDPDImage + eeprom_offset + (idx * PRE_CAL_SET_MAX_LENGTH), length);

	return event_len;
}
#endif /* PRE_CAL_MT7986_SUPPORT */

#ifdef PRE_CAL_MT7916_SUPPORT
INT GroupPreCalInfoAlloc_7916(RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length)
{
	P_PRE_CAL_INFO_MT7916_T pGroupPreCalInfo;
	UINT32 event_len = sizeof(PRE_CAL_INFO_MT7916_T) + length;

	if (os_alloc_mem(pAd, (UCHAR **)&pGroupPreCalInfo, event_len) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pGroupPreCalInfo;
	os_zero_mem(pGroupPreCalInfo, event_len);

	pGroupPreCalInfo->ucIndex  = idx;
	pGroupPreCalInfo->u4Length = length;

	os_move_mem(pGroupPreCalInfo->au4Data, pAd->PreCalImage + (idx * PRE_CAL_SET_MAX_LENGTH), length);
	return event_len;
}

INT DpdFlatnessCalInfoAlloc_7916(
	RTMP_ADAPTER *pAd,
	VOID         **pptr,
	UINT16       idx,
	UINT32       length,
	UINT32       eeprom_ofst
	)
{
	P_PRE_CAL_INFO_MT7916_T pDpdFlatnessCalInfo;
	UINT32 event_len = sizeof(PRE_CAL_INFO_MT7916_T) + length;

	if (os_alloc_mem(pAd, (UCHAR **)&pDpdFlatnessCalInfo, event_len) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pDpdFlatnessCalInfo;
	os_zero_mem(pDpdFlatnessCalInfo, event_len);

	pDpdFlatnessCalInfo->ucIndex  = idx;
	pDpdFlatnessCalInfo->u4Length = length;
#ifdef RT_BIG_ENDIAN
	pDpdFlatnessCalInfo->u4Length = cpu2le32(pDpdFlatnessCalInfo->u4Length);
#endif
	////pDpdFlatnessCalInfo->ucSecBw80 = bSecBw80;
	os_move_mem(pDpdFlatnessCalInfo->au4Data, pAd->TxDPDImage + eeprom_ofst + (idx * PRE_CAL_SET_MAX_LENGTH), length);

	return event_len;
}
#endif /* PRE_CAL_MT7916_SUPPORT */

#ifdef PRE_CAL_MT7981_SUPPORT
INT GroupPreCalInfoAlloc_7981(RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length)
{
	P_PRE_CAL_INFO_MT7981_T pGroupPreCalInfo;
	UINT32 event_len = sizeof(PRE_CAL_INFO_MT7981_T) + length;

	if (os_alloc_mem(pAd, (UCHAR **)&pGroupPreCalInfo, event_len) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pGroupPreCalInfo;
	os_zero_mem(pGroupPreCalInfo, event_len);

	pGroupPreCalInfo->ucIndex  = idx;
	pGroupPreCalInfo->u4Length = length;

	os_move_mem(pGroupPreCalInfo->au4Data, pAd->PreCalImage + (idx * PRE_CAL_SET_MAX_LENGTH), length);
	return event_len;
}

INT DpdFlatnessCalInfoAlloc_7981(
	RTMP_ADAPTER *pAd,
	VOID         **pptr,
	UINT16       idx,
	UINT32       length,
	UINT32       eeprom_offset
	)
{
	P_PRE_CAL_INFO_MT7981_T pDpdFlatnessCalInfo;
	UINT32 event_len = sizeof(PRE_CAL_INFO_MT7981_T) + length;

	if (os_alloc_mem(pAd, (UCHAR **)&pDpdFlatnessCalInfo, event_len) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pDpdFlatnessCalInfo;
	os_zero_mem(pDpdFlatnessCalInfo, event_len);

	pDpdFlatnessCalInfo->ucIndex  = idx;
	pDpdFlatnessCalInfo->u4Length = length;
#ifdef RT_BIG_ENDIAN
	pDpdFlatnessCalInfo->u4Length = cpu2le32(pDpdFlatnessCalInfo->u4Length);
#endif
	////pDpdFlatnessCalInfo->ucSecBw80 = bSecBw80;
	os_move_mem(pDpdFlatnessCalInfo->au4Data, pAd->TxDPDImage + eeprom_offset + (idx * PRE_CAL_SET_MAX_LENGTH), length);

	return event_len;
}
#endif /* PRE_CAL_MT7981_SUPPORT */

INT TxLpfCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr)
{
	P_TXLPF_CAL_INFO_T pTxLpfCalInfo;

	if (!RLM_CAL_CACHE_IS_TXLPF_CAL_DONE(rlmCalCache))
		return 0;

	if (os_alloc_mem(pAd, (UCHAR **)&pTxLpfCalInfo, sizeof(TXLPF_CAL_INFO_T)) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pTxLpfCalInfo;
	os_zero_mem(pTxLpfCalInfo, sizeof(TXLPF_CAL_INFO_T));
	/*os_move_mem(pTxLpfCalInfo->au4Data, RLM_CAL_CACHE_TXLPF_CAL_INFO(rlmCalCache).au4Data,
		sizeof(RLM_CAL_CACHE_TXLPF_CAL_INFO(rlmCalCache).au4Data));*/
	os_move_mem(pTxLpfCalInfo, &RLM_CAL_CACHE_TXLPF_CAL_INFO(rlmCalCache), sizeof(TXLPF_CAL_INFO_T));
	return sizeof(TXLPF_CAL_INFO_T);
}

INT TxIqCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr)
{
	P_TXIQ_CAL_INFO_T pTxIqCalInfo;

	if (!RLM_CAL_CACHE_IS_TXIQ_CAL_DONE(rlmCalCache))
		return 0;

	if (os_alloc_mem(pAd, (UCHAR **)&pTxIqCalInfo, sizeof(TXIQ_CAL_INFO_T)) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pTxIqCalInfo;
	os_zero_mem(pTxIqCalInfo, sizeof(TXIQ_CAL_INFO_T));
	/*os_move_mem(pTxIqCalInfo->au4Data, &(RLM_CAL_CACHE_TXIQ_CAL_INFO(rlmCalCache).au4Data),
		sizeof(RLM_CAL_CACHE_TXIQ_CAL_INFO(rlmCalCache).au4Data));*/
	os_move_mem(pTxIqCalInfo, &RLM_CAL_CACHE_TXIQ_CAL_INFO(rlmCalCache), sizeof(TXIQ_CAL_INFO_T));

	return sizeof(TXIQ_CAL_INFO_T);
}

INT TxDcCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr)
{
	P_TXDC_CAL_INFO_T pTxDcCalInfo;

	if (!RLM_CAL_CACHE_IS_TXDC_CAL_DONE(rlmCalCache))
		return 0;

	if (os_alloc_mem(pAd, (UCHAR **)&pTxDcCalInfo, sizeof(TXDC_CAL_INFO_T)) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pTxDcCalInfo;
	os_zero_mem(pTxDcCalInfo, sizeof(TXDC_CAL_INFO_T));
	/*os_move_mem(pTxDcCalInfo->au4Data, RLM_CAL_CACHE_TXDC_CAL_INFO(rlmCalCache).au4Data,
		sizeof(RLM_CAL_CACHE_TXDC_CAL_INFO(rlmCalCache).au4Data));*/
	os_move_mem(pTxDcCalInfo, &RLM_CAL_CACHE_TXDC_CAL_INFO(rlmCalCache), sizeof(TXDC_CAL_INFO_T));

	return sizeof(TXDC_CAL_INFO_T);
}

INT RxFiCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr)
{
	P_RXFI_CAL_INFO_T pRxFiCalInfo = NULL;

	if (!RLM_CAL_CACHE_IS_RXFI_CAL_DONE(rlmCalCache))
		return 0;

	if (os_alloc_mem(pAd, (UCHAR **)&pRxFiCalInfo, sizeof(RXFI_CAL_INFO_T)) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pRxFiCalInfo;
	os_zero_mem(pRxFiCalInfo, sizeof(RXFI_CAL_INFO_T));
	/*os_move_mem(pRxFiCalInfo->au4Data, RLM_CAL_CACHE_RXFI_CAL_INFO(rlmCalCache).au4Data,
		sizeof(RLM_CAL_CACHE_RXFI_CAL_INFO(rlmCalCache).au4Data));*/
	os_move_mem(pRxFiCalInfo, &RLM_CAL_CACHE_RXFI_CAL_INFO(rlmCalCache), sizeof(RXFI_CAL_INFO_T));

	return sizeof(RXFI_CAL_INFO_T);
}

INT RxFdCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr, UINT32 chGroup)
{
	P_RXFD_CAL_INFO_T pRxFdCalInfo;

	if (!RLM_CAL_CACHE_IS_RXFD_CAL_DONE(rlmCalCache, chGroup))
		return 0;

	if (os_alloc_mem(pAd, (UCHAR **)&pRxFdCalInfo, sizeof(RXFD_CAL_INFO_T)) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pRxFdCalInfo;
	os_zero_mem(pRxFdCalInfo, sizeof(RXFD_CAL_INFO_T));
		pRxFdCalInfo->u4ChGroupId = chGroup;
	/*os_move_mem(pRxFdCalInfo->au4Data, RLM_CAL_CACHE_RXFD_CAL_INFO(rlmCalCache, chGroup).au4Data,
		sizeof(RLM_CAL_CACHE_RXFD_CAL_INFO(rlmCalCache, chGroup).au4Data));*/
	os_move_mem(pRxFdCalInfo, &RLM_CAL_CACHE_RXFD_CAL_INFO(rlmCalCache, chGroup), sizeof(RXFD_CAL_INFO_T));

	return sizeof(RXFD_CAL_INFO_T);
}

INT RlmPorCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr)
{
	P_RLM_POR_CAL_INFO_T pRlmPorCalInfo = NULL;

	if (os_alloc_mem(pAd, (UCHAR **)&pRlmPorCalInfo, sizeof(RLM_POR_CAL_INFO_T)) != NDIS_STATUS_SUCCESS)
		return 0;

	*pptr = pRlmPorCalInfo;
	os_zero_mem(pRlmPorCalInfo, sizeof(RLM_POR_CAL_INFO_T));
	pRlmPorCalInfo->ucRlmPorCal = 1;

	return sizeof(RLM_POR_CAL_INFO_T);
}

