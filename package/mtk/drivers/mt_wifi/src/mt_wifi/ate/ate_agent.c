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
 ***************************************************************************

	Module Name:
	ate_agent.c
*/

#include "rt_config.h"

#define MCAST_WCID_TO_REMOVE 0	/* Pat: TODO */

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)
#define ATE_ANT_USER_SEL 0x80000000
#endif

/*  CCK Mode */
static CHAR CCKRateTable[] = {0, 1, 2, 3, 8, 9, 10, 11, -1};

/*  OFDM Mode */
static CHAR OFDMRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, -1};

/*  HT Mixed Mode */
static CHAR HTMIXRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
				24, 25, 26, 27, 28, 29, 30, 31, 32, -1};

/* VHT Mode */
static CHAR VHTRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1};

/* HE SU Mode */
static CHAR HERateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 32, 33, -1};

/* HE ER Mode */
static CHAR HEERRateTable[] = {0, 1, 2, 16, 32, 33, 48, -1};

UINT_8  Addr1[6] = {0x00, 0x11, 0x11, 0x11, 0x11, 0x11};
UINT_8  Addr2[6] = {0x00, 0x22, 0x22, 0x22, 0x22, 0x22};
UINT_8  Addr3[6] = {0x00, 0x22, 0x22, 0x22, 0x22, 0x22};

#define QOS_NO_ACK 0x20

#if defined(TXBF_SUPPORT) && defined(MT_MAC)
UCHAR TemplateFrame[32] = {0x88, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0x00, 0xAA, 0xBB, 0x12, 0x34, 0x56,
				0x00, 0x11, 0x22, 0xAA, 0xBB, 0xCC, 0x00, 0x00,
				0x00, QOS_NO_ACK, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				};

extern UINT8 BF_ON_certification;
extern UINT8 g_EBF_certification;
#else
static UCHAR TemplateFrame[32] = {0x08, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
					0xFF, 0xFF, 0x00, 0xAA, 0xBB, 0x12, 0x34, 0x56,
					0x00,	0x11, 0x22, 0xAA, 0xBB, 0xCC, 0x00, 0x00,
					0x00, QOS_NO_ACK, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
					};
#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */

static UCHAR tx_mode_map[MODE_VHT_MIMO+1][15] = {
	{"CCK"},
	{"OFDM"},
	{"HT-Mix"},
	{"HT-GreenField"},
	{"VHT"},
	{"HE"},
	{""},
	{""},
	{"HE-SU"},
	{"HE-ER"},
	{"HE-TB"},
	{"HE-MU"},
	{"VHT-MIMO"},
};

INT32 SetTxStop(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetRxStop(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


#ifdef DBG
VOID ATE_QA_Statistics(RTMP_ADAPTER *pAd, RXWI_STRUC *pRxWI, RXINFO_STRUC *pRxInfo, PHEADER_802_11 pHeader)
{
}


INT32 SetEERead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetEEWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetBBPRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetBBPWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetRFWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}
#endif /* DBG */


VOID EEReadAll(PRTMP_ADAPTER pAd, UINT16 *Data, UINT32 size)
{
	UINT32 Offset = 0;
	UINT16 Value = 0;

	for (Offset = 0; Offset < (size >> 1);) {
		RT28xx_EEPROM_READ16(pAd, (Offset << 1), Value);
		Data[Offset] = Value;
		Offset++;
	}
}

INT32 SetATEDaByWtblTlv(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UINT16 wcid = 1;
	UINT8 *addr = NULL;
	INT32 Octet;
	UINT32 rv;
	RTMP_STRING *mac_tok = NULL, *dash_ptr = NULL, *mac_str = NULL;
	/* Tag = 0, Generic */
	CMD_WTBL_GENERIC_T rWtblGeneric = {0};

	MTWF_PRINT("Da = %s\n", Arg);

	dash_ptr = strstr(Arg, "-");
	if (dash_ptr) {
		mac_str = dash_ptr+1;
		*dash_ptr = '\0';
		rv = sscanf(Arg, "%d", &Octet);
		if (rv != 1) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Format Error!\n");
			return FALSE;
		}
		wcid = Octet;
	} else
		mac_str = Arg;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(mac_str) != 17)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr1[wcid-1]);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr3[wcid-1]);
#endif /* CONFIG_STA_SUPPORT */

	if (addr == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "addr is NULL\n");
		return FALSE;
	}

	for (Octet = 0, mac_tok = rstrtok(mac_str, ":"); mac_tok; mac_tok = rstrtok(NULL, ":")) {
		/* sanity check */
		if ((strlen(mac_tok) != 2) || (!isxdigit(*mac_tok)) || (!isxdigit(*(mac_tok + 1))))
			return FALSE;

		AtoH(mac_tok, &addr[Octet++], 1);
	}

	/* sanity check */
	if (Octet != MAC_ADDR_LEN)
		return FALSE;

#ifdef MT_MAC
	/* WIndex = 1, WTBL1: PeerAddress */
	rWtblGeneric.u2Tag = WTBL_GENERIC;
	rWtblGeneric.u2Length = sizeof(CMD_WTBL_GENERIC_T);
	NdisMoveMemory(rWtblGeneric.aucPeerAddress, addr, MAC_ADDR_LEN);
	CmdExtWtblUpdate(pAd, wcid /**/, SET_WTBL, (PUCHAR)&rWtblGeneric, sizeof(CMD_WTBL_GENERIC_T));
#endif
	MTWF_PRINT("%s: (DA = %02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(addr));

	return TRUE;
}

INT32 SetATEQid(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	USHORT q_idx;

	q_idx = simple_strtol(Arg, 0, 10);
	TESTMODE_SET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), ac_idx, q_idx);
	MTWF_PRINT("%s: QID:%u\n", __func__, q_idx);
	return TRUE;
}

INT32 SetATETxSEnable(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UINT32 param = 0;

	MTWF_PRINT("%s: Parm = %s\n", __func__, Arg);
	param = simple_strtol(Arg, 0, 10);
	ATECtrl->txs_enable = param;

	return TRUE;
}

INT32 SetATERxFilter(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	const INT param_num = 3;
	MT_RX_FILTER_CTRL_T rx_filter;
	UINT32 *input = NULL;
	CHAR *value;
	INT i;

	MTWF_PRINT("%s: Parm = %s\n", __func__, arg);

	os_alloc_mem(pAd, (UCHAR **)&input, param_num);
	if (input == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Failed to allocate memory !\n");
		return FALSE;
	}

	for (i = 0; i < param_num; i++)
		input[i] = 0;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if (i == param_num)
			break;

		input[i++] = simple_strtol(value, 0, 16);
	}

	os_zero_mem(&rx_filter, sizeof(rx_filter));
	rx_filter.bPromiscuous = input[0];
	rx_filter.bFrameReport = input[1];
	rx_filter.filterMask = input[2];
	rx_filter.u1BandIdx = control_band_idx;
	MtATESetRxFilter(pAd, rx_filter);
	MTWF_PRINT("%s: Promiscuous:%x, FrameReport:%x, filterMask:%x\n", __func__,
		rx_filter.bPromiscuous, rx_filter.bFrameReport, rx_filter.filterMask);
	os_free_mem(input);

	return TRUE;
}

INT32 SetATERxStream(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT32 param = 0;

	MTWF_PRINT("%s: Parm = %s\n", __func__, Arg);

	param = simple_strtol(Arg, 0, 10);
	Ret = MtATESetRxPath(pAd, param, control_band_idx);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}

INT32 SetATETxStream(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT32 param = 0;

	MTWF_PRINT("%s: Parm = %s\n", __func__, Arg);

	param = simple_strtol(Arg, 0, 10);
	Ret = MtATESetTxStream(pAd, param, control_band_idx);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}

INT32 SetATEMACTRx(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	const INT param_num = 3;
	UINT32 *input;
	CHAR *value;
	INT i;

	MTWF_PRINT("%s: Parm = %s\n", __func__, arg);

	os_alloc_mem(pAd, (UCHAR **)&input, param_num);
	if (input == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Failed to allocate memory !\n");
		return FALSE;
	}

	for (i = 0; i < param_num; i++)
		input[i] = 0;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if (i == param_num)
			break;

		input[i++] = simple_strtol(value, 0, 16);
	}

	Ret = MtATESetMacTxRx(pAd, input[0], input[1], control_band_idx);
	os_free_mem(input);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}

INT32 SetATEMPSStart(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = 0;
	UINT enable;

	MTWF_PRINT("%s: Parm = %s\n", __func__, arg);

	enable = simple_strtol(arg, 0, 10);

	if (enable)
		ret = ATEOp->MPSTxStart(pAd);
	else
		ret = ATEOp->MPSTxStop(pAd);

	if (!ret)
		return TRUE;
	else
		return FALSE;
}

INT32 SetATEMPSDump(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT ret = 0;
	UINT32 band_idx = simple_strtol(arg, 0, 10);

	ret = MT_SetATEMPSDump(pAd, band_idx);

	if (!ret)
		return TRUE;

	return FALSE;
}

static INT32 SetATEMPSParam(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg, UINT type)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Ret = 0;
	INT num_items = 0;
	CHAR *value;
	RTMP_STRING *tmp = arg;
	UINT32 *mps_setting = NULL;
	INT i;

	MTWF_PRINT("%s: Parm = %s\n", __func__, arg);

	value = rstrtok(tmp, ":");

	if (!value)
		goto err0;

	MTWF_PRINT("value:%s, arg:%s, tmp:%s\n", value, arg, tmp);
	num_items = simple_strtol(value, 0, 10);

	if (!num_items)
		goto err0;

	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32) * (num_items));

	if (Ret)
		goto err1;

	for (i = 0, value = rstrtok(NULL, ":"); value; value = rstrtok(NULL, ":")) {
		if (i == num_items)
			break;

		mps_setting[i++] = simple_strtol(value, 0, 10);
	}

	if (i != num_items)
		goto err2;

	ATEOp->MPSSetParm(pAd, type, num_items, mps_setting);

	if (mps_setting)
		os_free_mem(mps_setting);

	return TRUE;
err2:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"Number of items %d is not matched with number of params %d\n", num_items, i);

	if (mps_setting)
		os_free_mem(mps_setting);

err1:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"Mem allocate fail\n");
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"[%u]Format: num_itmes:param1:param2:...\n", type);
	return FALSE;
}

INT32 SetATEMPSPhyMode(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_PHYMODE);
	return ret;
}

INT32 SetATEMPSRate(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_RATE);
	return ret;
}


INT32 SetATEMPSPath(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_PATH);
	return ret;
}

INT32 SetATEMPSPayloadLen(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_PAYLOAD_LEN);
	return ret;
}

INT32 SetATEMPSPktCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_TX_COUNT);
	return ret;
}

INT32 SetATEMPSPwr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_PWR_GAIN);
	return ret;
}

INT32 SetATEMPSNss(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_NSS);
	return ret;
}

INT32 SetATEMPSPktBw(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;

	ret = SetATEMPSParam(pAd, arg, MPS_PKT_BW);
	return ret;
}

INT32 SetATELOGDump(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_LOG_DUMP_CB *log_cb;
	UINT32 log_type;

	log_type = simple_strtol(Arg, 0, 10);

	if (log_type >= ATE_LOG_TYPE_NUM || log_type < 1)
		return FALSE;

	log_cb = &ATECtrl->log_dump[log_type - 1];
	MT_ATEDumpLog(pAd, log_cb, log_type);
	MTWF_PRINT("%s: log_type:%08x, driver:%08x\n",
		__func__, log_type, ATECtrl->en_log);
	return TRUE;
}

INT32 SetATELOGEnable(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 log_type;
	INT32 Ret = 0;

	log_type = simple_strtol(Arg, 0, 10);
	Ret = ATEOp->LogOnOff(pAd, log_type, TRUE, 2000);
	MTWF_PRINT("%s: log_type:%08x, driver:%08x\n",
		__func__, log_type, ATECtrl->en_log);
	return TRUE;
}

INT32 SetATELOGDisable(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 log_type;
	INT32 Ret = 0;

	log_type = simple_strtol(Arg, 0, 10);
	Ret = ATEOp->LogOnOff(pAd, log_type, FALSE, 0);
	MTWF_PRINT("%s: log_type:%08x, driver:%08x\n",
		__func__, log_type, ATECtrl->en_log);
	return TRUE;
}

INT32 SetATEDeqCnt(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#ifdef ATE_TXTHREAD
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT deq_cnt;

	deq_cnt = simple_strtol(Arg, 0, 10);
	ATECtrl->deq_cnt = deq_cnt;
	MTWF_PRINT("%s: deq_cnt:%d\n", __func__, deq_cnt);
#endif
	return TRUE;
}

INT32 SetATEDa(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	return SetATEDaByWtblTlv(pAd, Arg);
}

INT32 SetATESa(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	RTMP_STRING *Value;
	INT32 Octet, sta_idx = 0;
#ifndef CONFIG_WLAN_SERVICE
	struct wifi_dev *wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), wdev[0]);
#else
	struct wifi_dev *wdev = &pAd->ate_wdev[TESTMODE_GET_BAND_IDX(pAd)][0];
#endif /* CONFIG_WLAN_SERVICE */
	UCHAR BandIdx, *addr = NULL;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(Arg) != 17)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr3[0]);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr2[0]);
#endif /* CONFIG_STA_SUPPORT */

	if (addr == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"addr is NULL\n");
		return FALSE;
	}

	for (Octet = 0, Value = rstrtok(Arg, ":"); Value; Value = rstrtok(NULL, ":")) {
		/* sanity check */
		if ((strlen(Value) != 2) || (!isxdigit(*Value)) || (!isxdigit(*(Value + 1))))
			return FALSE;

		AtoH(Value, &addr[Octet++], 1);
	}

	/* sanity check */
	if (Octet != MAC_ADDR_LEN)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		for (sta_idx = 1; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
			UCHAR *tmp_addr = NULL;

			tmp_addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr3[sta_idx]);
			os_move_mem(tmp_addr, addr, MAC_ADDR_LEN);
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		for (sta_idx = 1; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
			UCHAR *tmp_addr = NULL;

			tmp_addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr2[sta_idx]);
			os_move_mem(tmp_addr, addr, MAC_ADDR_LEN);
		}
	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef MT_MAC
	/* Set the specific MAC to ASIC */
	/* TODO: Hanmin H/W HAL offload, below code is replaced by new code above, right PIC needs further check */
	BandIdx = HcGetBandByWdev(wdev);
	AsicDevInfoUpdate(
		pAd,
		HcGetOmacIdx(pAd, wdev),
		addr,
		BandIdx,
		TRUE,
		DEVINFO_ACTIVE_FEATURE);
#endif
	MTWF_PRINT("%s: (SA = %02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(addr));



	return TRUE;
}


INT32 SetATEBssid(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	RTMP_STRING *Value;
	INT32 Octet, sta_idx = 0;
	UCHAR *addr = NULL;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(Arg) != 17)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr2[0]);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr1[0]);
#endif /* CONFIG_STA_SUPPORT */

	if (addr == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"addr is NULL\n");
		return FALSE;
	}

	for (Octet = 0, Value = rstrtok(Arg, ":"); Value; Value = rstrtok(NULL, ":")) {
		/* sanity check */
		if ((strlen(Value) != 2) || (!isxdigit(*Value)) || (!isxdigit(*(Value + 1))))
			return FALSE;

		AtoH(Value, &addr[Octet++], 1);
	}

	/* sanity check */
	if (Octet != MAC_ADDR_LEN)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		for (sta_idx = 1; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
			UCHAR *tmp_addr = NULL;

			tmp_addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr2[sta_idx]);
			os_move_mem(tmp_addr, addr, MAC_ADDR_LEN);
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		for (sta_idx = 1; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
			UCHAR *tmp_addr = NULL;

			tmp_addr = TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), addr1[sta_idx]);
			os_move_mem(tmp_addr, addr, MAC_ADDR_LEN);
		}
	}
#endif /* CONFIG_STA_SUPPORT */


	MTWF_PRINT("%s: (BSSID = %02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(addr));


	return TRUE;
}


INT32 SetATEInitChan(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	return TRUE;
}


INT32 SetADCDump(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}

INT32 SetATERxUser(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UCHAR band_idx = 0;
	UINT16 user_idx = 0;
	INT32 Ret = 0;

	UINT8 i;
	CHAR *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			band_idx = simple_strtol(value, 0, 10);
			break;
		case 1:
			user_idx = simple_strtol(value, 0, 16);
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Number of parameters exceed expectation !!\n");
			return FALSE;
		}
	}

	MTWF_PRINT("%s(): band_idx: %d, user_idx: %d\n", __func__, band_idx, user_idx);

	TESTMODE_SET_PARAM(pAd, band_idx, user_idx, user_idx);

	Ret = ATEOp->SetRxUserIdx(pAd, band_idx, user_idx);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}

INT32 SetATETxPower0(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_set_ext("ATETXPOW0", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	ATE_TXPOWER TxPower;
	CHAR Power;
	INT32 Ret = 0;

	MTWF_PRINT("%s: Power0 = %s\n", __func__, Arg);
	Power = simple_strtol(Arg, 0, 10);
	ATECtrl->TxPower0 = Power;
	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Power = Power;
	TxPower.Dbdc_idx = TESTMODE_GET_BAND_IDX(pAd);
	Ret = ATEOp->SetTxPower0(pAd, TxPower);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
#endif
}


INT32 SetATETxPower1(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	ATE_TXPOWER TxPower;
	CHAR Power;
	INT32 Ret = 0;

	MTWF_PRINT("%s: Power1 = %s\n", __func__, Arg);
	Power = simple_strtol(Arg, 0, 10);
	ATECtrl->TxPower1 = Power;
	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Power = Power;
	TxPower.Dbdc_idx = TESTMODE_GET_BAND_IDX(pAd);
	Ret = ATEOp->SetTxPower1(pAd, TxPower);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATETxPower2(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	ATE_TXPOWER TxPower;
	CHAR Power;
	INT32 Ret = 0;

	MTWF_PRINT("%s: Power2 = %s\n", __func__, Arg);
	Power = simple_strtol(Arg, 0, 10);
	ATECtrl->TxPower2 = Power;
	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Power = Power;
	TxPower.Dbdc_idx = TESTMODE_GET_BAND_IDX(pAd);
	Ret = ATEOp->SetTxPower2(pAd, TxPower);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATETxPower3(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	ATE_TXPOWER TxPower;
	CHAR Power;
	INT32 Ret = 0;

	MTWF_PRINT("%s: Power3 = %s\n", __func__, Arg);
	Power = simple_strtol(Arg, 0, 10);
	ATECtrl->TxPower3 = Power;
	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Power = Power;
	TxPower.Dbdc_idx = TESTMODE_GET_BAND_IDX(pAd);
	Ret = ATEOp->SetTxPower3(pAd, TxPower);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}

INT32 SetATEForceTxPower(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Ret;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT8 ParamIdx;
	CHAR  *value = 0;
	INT_8 cTxPower = 0;
	UINT8 ucPhyMode = 0;
	UINT8 ucTxRate = 0;
	UINT8 ucBW = 0;

	/* Sanity check for input parameter */
	if (!Arg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"No Parameters !! \n");
		goto err1;
	}

	/* Sanity check for input parameter format */
	if (strlen(Arg) != 11) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Wrong Parameter Format !!\n");
		goto err1;
	}

	/* Parsing input parameter */
	for (ParamIdx = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":"), ParamIdx++) {
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
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Set Too Much Parameters !!\n");
			goto err1;
		}
	}

	MTWF_PRINT("%s: Band(%d), TxMode(%d), MCS(%d), BW(%d), TxPower(%d)\n",
		__func__, control_band_idx, ucPhyMode, ucTxRate, ucBW, cTxPower);

	/* Command Handler for Force Power Control */
	Ret = ATEOp->SetTxForceTxPower(pAd, cTxPower, ucPhyMode, ucTxRate, ucBW);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
	err1:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, KGRN "Please input parameter via format \"Phymode:TxRate:BW:TxPower\"\n" KNRM);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, KGRN "Phymode:\n" KNRM);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, KGRN "    (2-digit) 0: CCK, 1: OFDM, 2: HT, 3: VHT, 4: HESU\n" KNRM);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, KGRN "TxRate:\n" KNRM);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, KGRN "    (2-digit) CCK: 00~03, OFDM: 00~07, HT: 00~07, VHT: 00~09, HESU:00~11 \n" KNRM);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, KGRN "BW:\n" KNRM);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, KGRN "    (2-digit) 0: BW20, 1: BW40, 2: BW80, 3:BW160\n" KNRM);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, KGRN "    (2-digit) 0: HE26, 1: HE52, 2: HE106, 3:HE242, 4:HE484, 5:HE996, 6:HE996X2\n" KNRM);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, KGRN "TxPower:\n" KNRM);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, KGRN "    (2-digit) absolute Tx power (unit: 0.5dB)\n" KNRM);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, KRED "Ex: iwpriv ra0 set ATEFORCETXPOWER=02:00:00:16\n" KNRM);
		return FALSE;

}

INT32 SetATETxPowerEvaluation(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetATETxAntenna(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_set_ext("ATETXANT", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Ret = 0;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT32 Ant = 1;
	const INT idx_num = 2;
	UINT32 param[idx_num];
	UINT8 loop_index = 0;
	CHAR *value;
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)
	UINT32 mode = 0;
#endif

	/* Sanity check for input parameter */
	if (Arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"No parameters!!\n");
		goto err0;
	}

	/* TX path setting */
	if (!strchr(Arg, ':')) {
		MTWF_PRINT("%s: Ant = %scontrol_band_idx = %d\n",
			__func__, Arg, control_band_idx);
		Ant = simple_strtol(Arg, 0, 10);
	} else {
		MTWF_PRINT("%s: Mode:Value = %s, control_band_idx = %d\n",
			__func__, Arg, control_band_idx);

		for (loop_index = 0; loop_index < idx_num; loop_index++)
			param[loop_index] = 0;

		for (loop_index = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":")) {
			if (loop_index == idx_num)
				break;

			param[loop_index] = simple_strtol(value, 0, 10);
			loop_index++;
		}

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)
		mode = param[0];

		if (mode == ANT_MODE_SPE_IDX)
			Ant = param[1] | ATE_ANT_USER_SEL;
		else
			Ant = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_ant);

#else
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"No need to set Spe_idx.\n");

		goto err0;
#endif /* defined(MT7615) || defined(MT7622) */
	}

	Ret = ATEOp->SetTxAntenna(pAd, Ant);

	if (!Ret)
		return TRUE;

err0:
	return FALSE;
#endif
}


INT32 SetATERxAntenna(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_set_ext("ATERXANT", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	CHAR Ant;

	/* Sanity check for input parameter */
	if (Arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"No parameters!!\n");
		goto err0;
	}

	MTWF_PRINT("%s: Ant = %s\n", __func__, Arg);
	Ant = simple_strtol(Arg, 0, 10);
	Ret = ATEOp->SetRxAntenna(pAd, Ant);

	if (!Ret)
		return TRUE;

err0:
	return FALSE;
#endif
}


INT32 Default_Set_ATE_TX_FREQ_OFFSET_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	return TRUE;
}


INT32 SetATETxFreqOffset(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	UINT32 FreqOffset;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_PRINT("%s: FreqOffset = %s\n", __func__, Arg);
	FreqOffset = simple_strtol(Arg, 0, 10);
	Ret = ATEOp->SetTxFreqOffset(pAd, FreqOffset);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 GetATETxFreqOffset(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 FreqOffsetRxv = 0;
	RX_STATISTIC_RXV *rx_stat_rxv = &pAd->rx_stat_rxv[DBDC_BAND0];

	FreqOffsetRxv = rx_stat_rxv->FreqOffsetFromRx[0];

	MTWF_PRINT("%s: FreqOffset = %d\n", __func__, FreqOffsetRxv);

		return TRUE;
}



INT32 Default_Set_ATE_TX_BW_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetATETxLength(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT32 TxLength;

	MTWF_PRINT("%s: TxLength = %s, control_band_idx = %d\n", __func__, Arg, control_band_idx);

	TxLength = simple_strtol(Arg, 0, 10);
	TESTMODE_SET_PARAM(pAd, control_band_idx, tx_len, TxLength);
	return TRUE;
}


INT32 SetATETxCount(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT32 TxCount = 0;

	MTWF_PRINT("%s: TxCount = %s, control_band_idx = %d\n", __func__, Arg, control_band_idx);

	TxCount = simple_strtol(Arg, 0, 10);

	if (TxCount == 0)
		TxCount = 0xFFFFFFFF;

	TESTMODE_SET_PARAM(pAd, control_band_idx, ATE_TX_CNT, TxCount);
	return TRUE;
}


INT32 CheckMCSValid(PRTMP_ADAPTER pAd, UCHAR tx_mode, UCHAR Mcs)
{
	int Index;
	PCHAR pRateTab = NULL;

	switch (tx_mode) {
	case MODE_CCK:
		pRateTab = CCKRateTable;
		break;

	case MODE_OFDM:
		pRateTab = OFDMRateTable;
		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		pRateTab = HTMIXRateTable;
		break;

	case MODE_VHT:
		pRateTab = VHTRateTable;
		break;

	case MODE_HE_SU:
		pRateTab = HERateTable;
		break;

	case MODE_HE_EXT_SU:
		pRateTab = HEERRateTable;
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Unrecognizable Tx Mode\n");
		return -1;
	}

	Index = 0;

	while (pRateTab[Index] != -1) {
		if (pRateTab[Index] == Mcs)
			return 0;

		Index++;
	}

	return -1;
}


INT32 SetATETxMcs(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_set_dw("ATETXMCS", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	INT32 Ret = 0;
	UCHAR Mcs, tx_mode = 0;
	UINT8 control_band_idx = TESTMODE_GET_BAND_IDX(pAd);

	MTWF_PRINT("%s: Mcs = %s, control_band_idx = %d\n", __func__, Arg, control_band_idx);

	tx_mode = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_mode);
	Mcs = simple_strtol(Arg, 0, 10);
	Ret = CheckMCSValid(pAd, tx_mode, Mcs);

	if (Ret != -1) {
		UCHAR tmp_mcs = TESTMODE_GET_PARAM(pAd, control_band_idx, mcs);

		tmp_mcs = tmp_mcs & 0x80;	/* b'8 is DCM enabled or not, keep it */
		tmp_mcs |= (Mcs & 0x7f);

		TESTMODE_SET_PARAM(pAd, control_band_idx, mcs, tmp_mcs);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Out of range, refer to rate table.\n");
		goto err0;
	}

	return TRUE;
err0:
	return FALSE;
#endif
}


INT32 SetATETxNss(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_set_dw("ATETXNSS", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UCHAR Nss = 0;

	MTWF_PRINT("%s: Nss = %s, control_band_idx = %d\n", __func__, Arg, control_band_idx);

	Nss = simple_strtol(Arg, 0, 10);
	TESTMODE_SET_PARAM(pAd, control_band_idx, nss, Nss);
	return TRUE;
#endif
}


INT32 SetATETxLdpc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UCHAR Ldpc;

	MTWF_PRINT("%s: Ldpc = %s, control_band_idx = %d\n", __func__, Arg, control_band_idx);

	Ldpc = simple_strtol(Arg, 0, 10);

	if (Ldpc > 1) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Out of range (%d)\n", Ldpc);
		return FALSE;
	}

	TESTMODE_SET_PARAM(pAd, control_band_idx, ldpc, Ldpc);
	return TRUE;
}


INT32 SetATETxStbc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_set_dw("ATETXSTBC", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UCHAR Stbc;

	MTWF_PRINT("%s: Stbc = %s, control_band_idx = %d\n", __func__, Arg, control_band_idx);

	Stbc = simple_strtol(Arg, 0, 10);

	if (Stbc > 1) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Out of range (%d)\n", Stbc);
		return FALSE;
	}

	TESTMODE_SET_PARAM(pAd, control_band_idx, stbc, Stbc);
	return TRUE;
#endif
}


INT32 SetATETxMode(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_set_dw("ATETXMODE", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UCHAR tx_mode;

	MTWF_PRINT("%s: TxMode = %s, control_band_idx = %d\n", __func__, Arg, control_band_idx);

	tx_mode = simple_strtol(Arg, 0, 10);
	TESTMODE_SET_PARAM(pAd, control_band_idx, tx_mode, tx_mode);
	MTWF_PRINT("%s: TxMode = %x, control_band_idx:%u\n", __func__, tx_mode, control_band_idx);
	return TRUE;
#endif	/* CONFIG_WLAN_SERVICE */
}


INT32 SetATETxGi(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_set_dw("ATETXGI", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	INT32 ret = FALSE;
	UCHAR tx_mode = MODE_CCK;
	UCHAR sgi;
	UINT8 control_band_idx = TESTMODE_GET_BAND_IDX(pAd);

	MTWF_PRINT("%s: Sgi = %s, control_band_idx = %d\n", __func__, Arg, control_band_idx);

	sgi = simple_strtol(Arg, 0, 10);

	tx_mode = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_mode);

	if (tx_mode == MODE_HE_TRIG) {
		if (sgi > 2)
			goto err_out;
	} else if (tx_mode > MODE_VHT) {
		if (sgi > 3)
			goto err_out;
	} else {
		if (sgi > 1)
			goto err_out;
	}

	TESTMODE_SET_PARAM(pAd, control_band_idx, sgi, sgi);
	ret = TRUE;

err_out:
	if (!ret)
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Out of range (%d), ignored!\n", sgi);
	return ret;
#endif
}


INT32 set_ate_max_pe(RTMP_ADAPTER *ad, RTMP_STRING *Arg)
{
	UCHAR max_pe = 0;

	MTWF_PRINT("%s: Max Packet Extension = %s, control_band_idx = %d\n", __func__,
		Arg, TESTMODE_GET_BAND_IDX(ad));

	max_pe = simple_strtol(Arg, 0, 10);

	if (max_pe > 2) {
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%d is invalid (0: 0us, 1:8 us, 2:16 us)\n", max_pe);
		return FALSE;
	}

	TESTMODE_SET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), max_pkt_ext, max_pe);
	return TRUE;
}

#if defined(DOT11_HE_AX)
INT32 set_ate_ru_info(RTMP_ADAPTER *ad, RTMP_STRING *Arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_set_ext("ATERUINFO", &ad->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	INT32 ret = FALSE;
	struct _ATE_CTRL *ate_ctrl = &(ad->ATECtrl);
	struct _ATE_OPERATION *ate_op = ate_ctrl->ATEOp;
	UCHAR band_idx = TESTMODE_GET_BAND_IDX(ad);

	if (strlen(Arg) > 0) {
		MTWF_PRINT("%s: RU info = %s, control_band_idx = %d\n", __func__, Arg, band_idx);

		if (ate_op->set_ru_info)
			ate_op->set_ru_info(ad, band_idx, Arg);
		else
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"handler not registered, dismissed!\n");
	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid string\n");
	}

	return ret;
#endif
}

INT32 set_ate_ru_rx_aid(RTMP_ADAPTER *ad, RTMP_STRING *Arg)
{
	INT32 ret = TRUE, aid = 0;
	UINT32 rv;

	if (strlen(Arg) > 0) {
		rv = sscanf(Arg, "%d", &aid);
		if (rv != 1) {
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Format Error!\n");
			return FALSE;
		}
		MTWF_PRINT("%s: RX MU PPDU Aid = %s, control_band_idx = %d\n", __func__, Arg, TESTMODE_GET_BAND_IDX(ad));

		TESTMODE_SET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), mu_rx_aid, aid);
	} else {
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid string\n");
	}

	return ret;
}

INT32 set_ate_tx_policy(RTMP_ADAPTER *ad, RTMP_STRING *Arg)
{
	INT32 ret = TRUE;
	UINT32 tx_mode = 0, tx_method = 0,  rv;
	if (strlen(Arg) > 0) {
		rv = sscanf(Arg, "%d:%d", &tx_mode, &tx_method);
		if (rv != 2) {
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Format Error!\n");
			return FALSE;
		}
		if (tx_mode >= TX_MODE_MAX)
			MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"[band%d] Unknown TX mode(%d).\n", TESTMODE_GET_BAND_IDX(ad), tx_mode);
		else if (tx_method > 1)
			MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"[band%d] Unknown TX policy(%d, 0:TXD ; 1:TXC).\n", TESTMODE_GET_BAND_IDX(ad), tx_method);
		else {
			TESTMODE_SET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_method[tx_mode], tx_method);
			MTWF_PRINT("%s: [band%d] Set %s as %s mode.\n", __func__, TESTMODE_GET_BAND_IDX(ad), tx_mode_map[tx_mode],
					(TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_method[tx_mode]) ? "TXC" : "TXD"));
		}
	} else {
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid string\n");
	}

	return ret;
}
#endif


INT32 set_ate_retry(RTMP_ADAPTER *ad, RTMP_STRING *Arg)
{
	INT32 ret = TRUE;
	UINT32 retry = 0, rv;

	if (strlen(Arg) > 0) {
		rv = sscanf(Arg, "%d", &retry);
		if (rv != 1) {
			MTWF_DBG(ad, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Format Error!\n");
			return FALSE;
		}
		MTWF_PRINT("%s: TX PPDU Retry = %s, control_band_idx = %d\n", __func__, Arg, TESTMODE_GET_BAND_IDX(ad));

		TESTMODE_SET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), retry, retry);
	} else {
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid string\n");
	}

	return ret;
}


INT32 SetATERxFer(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	return TRUE;
}


INT32 SetATETempSensor(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}


INT32 SetATEReadRF(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;

	Ret = ShowAllRF(pAd);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATELoadE2p(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UINT32 Ret = 0;
	RTMP_STRING *Src = EEPROM_BIN_FILE_NAME;
	RTMP_OS_FD Srcf;
	INT32 Retval;
	USHORT *WriteEEPROM = NULL;
	INT32 FileLength = 0;
	UINT32 Value = (UINT32)simple_strtol(Arg, 0, 10);
	RTMP_OS_FS_INFO	OsFSInfo;

	MTWF_PRINT("===> %s (value=%d)\n\n", __func__, Value);
#ifdef RTMP_RBUS_SUPPORT
	if (IS_RBUS_INF(pAd))
		Src = EEPROM_DEFAULT_FILE_PATH;
#endif /* RTMP_RBUS_SUPPORT */

	Ret = os_alloc_mem(pAd, (PUCHAR *)&WriteEEPROM, EEPROM_SIZE);	/* TODO verify */

	if (Ret == NDIS_STATUS_FAILURE)
		return Ret;

	if (Value > 0) {
		/* zero the e2p buffer */
		NdisZeroMemory((PUCHAR)WriteEEPROM, EEPROM_SIZE);
		RtmpOSFSInfoChange(&OsFSInfo, TRUE);

		do {
			/* open the bin file */
			Srcf = RtmpOSFileOpen(Src, O_RDONLY, 0);

			if (IS_FILE_OPEN_ERR(Srcf)) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error opening file %s\n", Src);
				break;
			}

			/* read the firmware from the file *.bin */
			FileLength = RtmpOSFileRead(Srcf, (RTMP_STRING *)WriteEEPROM, EEPROM_SIZE);

			if (FileLength != EEPROM_SIZE) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"error file length (=%d) in e2p.bin\n", FileLength);
				break;
			}

			/* write the content of .bin file to EEPROM */
#if defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT)

			if (IS_PCI_INF(pAd)) {
				UINT16 Index = 0;
				UINT16 Value = 0;
				INT32 E2pSize = 512;/* == 0x200 for PCI interface */
				UINT16 TempData = 0;

				for (Index = 0 ; Index < (E2pSize >> 1); Index++) {
					/* "value" is especially for some compilers... */
					TempData = le2cpu16(WriteEEPROM[Index]);
					Value = TempData;
					RT28xx_EEPROM_WRITE16(pAd, (Index << 1), Value);
				}
			}

#else
			/* rt_ee_write_all(pAd, WriteEEPROM); */
#endif /* defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT) */
			Ret = TRUE;
			break;
		} while (TRUE);

		/* close firmware file */
		if (IS_FILE_OPEN_ERR(Srcf))
			;
		else {
			Retval = RtmpOSFileClose(Srcf);

			if (Retval) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"--> Error %d closing %s\n", -Retval, Src);
			}
		}

		/* restore */
		RtmpOSFSInfoChange(&OsFSInfo, FALSE);
	}

	os_free_mem(WriteEEPROM);
	MTWF_PRINT("<=== %s (Ret=%d)\n", __func__, Ret);
	return Ret;
}


#ifdef RTMP_EFUSE_SUPPORT
INT32 SetATELoadE2pFromBuf(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	BOOLEAN Ret = FALSE;
	UINT32 Value = (UINT32)simple_strtol(Arg, 0, 10);

	MTWF_PRINT("===> %s (Value=%d)\n\n", __func__, Value);

	if (Value > 0) {
#if defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT)

		if (IS_PCI_INF(pAd)) {
			UINT16 Index = 0;
			UINT16 Value = 0;
			INT32 E2PSize = 512;/* == 0x200 for PCI interface */
			UINT16 TempData = 0;

			for (Index = 0; Index < (E2PSize >> 1); Index++) {
				/* "value" is especially for some compilers... */
				TempData = le2cpu16(pAd->EEPROMImage[Index]);
				Value = TempData;
				RT28xx_EEPROM_WRITE16(pAd, (Index << 1), Value);
			}
		}

#else
		/* rt_ee_write_all(pAd, pAd->EEPROMImage); */
#endif /* defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT) */
		Ret = TRUE;
	}

	MTWF_PRINT("<=== %s (Ret=%d)\n", __func__, Ret);
	return Ret;
}
#endif /* RTMP_EFUSE_SUPPORT */


INT32 SetATEReadE2p(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 ret;
	UINT16 *Buffer = NULL;
	UINT16 size = EEPROM_SIZE;
	int i;

#if defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT)
	if (pAd->E2pAccessMode == E2P_FLASH_MODE)
		size = get_dev_eeprom_size(pAd);
#endif

	ret = os_alloc_mem(pAd, (PUCHAR *)&Buffer, size);

	if (ret == NDIS_STATUS_FAILURE)
		return ret;

	EEReadAll(pAd, (UINT16 *)Buffer, size);

	for (i = 0; i < (size >> 1); i++) {
		MTWF_PRINT("%4.4x ", *Buffer);

		if (((i + 1) % 16) == 0)
			MTWF_PRINT("\n");

		Buffer++;
	}

	os_free_mem(Buffer);
	return TRUE;
}


INT32 SetATEAutoAlc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	return TRUE;
}


INT32 SetATEIpg(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#if !defined(CONFIG_WLAN_SERVICE)
	INT32 ret = 0;
#endif
	UINT32 ipg = 0;

	/* Sanity check for input parameter */
	if (Arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"No parameters!!\n");
		goto err0;
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "IPG = %s\n", Arg);
	MTWF_PRINT("%s: IPG = %s\n", __func__, Arg);

	ipg = simple_strtol(Arg, 0, 10);

	TESTMODE_SET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), ipg_param.ipg, ipg);
#if !defined(CONFIG_WLAN_SERVICE)
	{
		struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
		struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

		ret = ATEOp->SetIPG(pAd, ipg);

		if (ret)
			goto err0;
	}
#endif
	return TRUE;
err0:
	return FALSE;
}


INT32 SetATEPayload(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_set_dw("ATEPAYLOAD", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	RTMP_STRING *Value;

	Value = Arg;

	/* only one octet acceptable */
	if (strlen(Value) != 2)
		return FALSE;

	AtoH(Value, &(ATECtrl->payload[0]), 1);
	MTWF_PRINT("Set_ATE_Payload_Proc (repeated pattern = 0x%2x)\n", ATECtrl->payload[0]);
	return TRUE;
#endif
}


INT32 SetATEFixedPayload(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_set_dw("ATEFIXEDPAYLOAD", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	UINT32 Value;
	UINT8 control_band_idx = TESTMODE_GET_BAND_IDX(pAd);

	/* only one octet acceptable */
	Value = simple_strtol(Arg, 0, 10);

	if (Value == 0)
		TESTMODE_SET_PARAM(pAd, control_band_idx, fixed_payload, 2);
	else
		TESTMODE_SET_PARAM(pAd, control_band_idx, fixed_payload, 1);

	MTWF_PRINT("%s: (Fixed Payload  = %u)\n", __func__, TESTMODE_GET_PARAM(pAd, control_band_idx, fixed_payload));
	return TRUE;
#endif
}


INT32 SetATETtr(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	return TRUE;
}


INT32 SetATEShow(RTMP_ADAPTER *ad, RTMP_STRING *Arg)
{
	struct _ATE_CTRL *ATECtrl = &ad->ATECtrl;
#ifdef DBDC_MODE
	struct _BAND_INFO *Info = NULL;
#endif /* DBDC_MODE */
	RTMP_STRING *Mode_String = NULL;
	RTMP_STRING *TxMode_String = NULL;
	UCHAR         control_band_idx = TESTMODE_GET_BAND_IDX(ad);
	UINT8         loop_index;
	INT           status = TRUE;
	CHAR          *value = 0;
	UCHAR         ExtendInfo = 0;

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Configure Input Parameter */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/

	/* sanity check for input parameter*/
	if (Arg == NULL) {
		MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "No parameters!!\n");
		MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Please use parameter 0 for Summary INFO, 1 for Detail INFO!!\n");
		return FALSE;
	}

	/* Parsing input parameter */
	for (loop_index = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":"), loop_index++) {
		switch (loop_index) {
		case 0:
			ExtendInfo = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Set wrong parameters\n");
			break;
		}
		}
	}

	MTWF_PRINT("%s: ExtendInfo = %d\n", __func__, ExtendInfo);

	MTWF_PRINT("%s: control_band_idx = %d !!!!!\n", __func__, control_band_idx);

	/* initialize pointer to structure of parameters of Band1 */
	MTWF_PRINT("%s: band[%d] ATE Mode = 0x%x !!!!!\n", __func__, TESTMODE_GET_BAND_IDX(ad),
			 TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), op_mode));

	/* check the ATE mode */
	switch (TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), op_mode)) {
	case (fATE_IDLE):
		Mode_String = "ATESTART";
		break;

	case (fATE_EXIT):
		Mode_String = "ATESTOP";
		break;

	case ((fATE_TX_ENABLE)|(fATE_TXCONT_ENABLE)):
		Mode_String = "TXCONT";
		break;

	case ((fATE_TX_ENABLE)|(fATE_TXCARR_ENABLE)):
		Mode_String = "TXCARR";
		break;

	case ((fATE_TX_ENABLE)|(fATE_TXCARRSUPP_ENABLE)):
		Mode_String = "TXCARS";
		break;

	case (fATE_TX_ENABLE):
		Mode_String = "TXFRAME";
		break;

	case (fATE_RX_ENABLE):
		Mode_String = "RXFRAME";
		break;

	default: {
		Mode_String = "Unknown ATE mode";
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ERROR! Unknown ATE mode(0x%x)!\n", TESTMODE_GET_PARAM(ad, TESTMODE_BAND0, op_mode));
		break;
		}
	}

	TxMode_String = "Unknown phy mode";
	if (TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_mode) < ARRAY_SIZE(tx_mode_map))
		TxMode_String = tx_mode_map[TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_mode)];

	/* Generic information */
	MTWF_PRINT("=============================================\n");
	MTWF_PRINT("\t\tBand %d Generic INFO\n", control_band_idx);
	MTWF_PRINT("=============================================\n");

	{
		UCHAR *tx_method = TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_method);
		UCHAR *addr1 = TESTMODE_GET_PADDR(ad, TESTMODE_GET_BAND_IDX(ad), addr1[0][0]);
		UCHAR *addr2 = TESTMODE_GET_PADDR(ad, TESTMODE_GET_BAND_IDX(ad), addr2[0][0]);
		UCHAR *addr3 = TESTMODE_GET_PADDR(ad, TESTMODE_GET_BAND_IDX(ad), addr3[0][0]);

		MTWF_PRINT("ATE Mode = %s\n", Mode_String);
		MTWF_PRINT("ATE Tx Methods:\n");
		for (loop_index = 0 ; loop_index < TX_MODE_MAX ; loop_index++) {

			if (strlen(tx_mode_map[loop_index])) {
				MTWF_PRINT("\t%s = %s\n", tx_mode_map[loop_index], tx_method[loop_index]?"TXC":"TXD");
			}
		}
		MTWF_PRINT("TxAntennaSel = 0x%x\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_ant));
		MTWF_PRINT("RxAntennaSel = 0x%x\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), rx_ant));
		MTWF_PRINT("BBPCurrentBW = %u\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), bw));
		MTWF_PRINT("GI = %u\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), sgi));
		MTWF_PRINT("MCS = %u\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), mcs));
		MTWF_PRINT("TxMode = %s\n", TxMode_String);
		MTWF_PRINT("Addr1 = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(addr1));
		MTWF_PRINT("Addr2 = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(addr2));
		MTWF_PRINT("Addr3 = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(addr3));
		MTWF_PRINT("Channel = %u\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), channel));
#ifdef DOT11_VHT_AC
		MTWF_PRINT("Channel_2nd = %u\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), channel_2nd));

#endif /* DOT11_VHT_AC */
		MTWF_PRINT("Ch_Band = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ch_band));
		MTWF_PRINT("Control Channel = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ctrl_ch));
		MTWF_PRINT("TxLength = %u\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_len));
		MTWF_PRINT("Header Length = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), hdr_len));
		MTWF_PRINT("Payload Length = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), pl_len));
		MTWF_PRINT("IPG = %dus\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ipg_param.ipg));
		MTWF_PRINT("Duty Cycle = %d%%\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), duty_cycle));
		MTWF_PRINT("Pkt Tx Time = %dus\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), tx_time_param.pkt_tx_time));
		MTWF_PRINT("Payload Pattern = 0x%02x\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), payload[0]));
		MTWF_PRINT("RFFreqOffset = %u\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), rf_freq_offset));
		MTWF_PRINT("SKB Allocate = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), is_alloc_skb));
		MTWF_PRINT("wdev_idx = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), wdev_idx));
		MTWF_PRINT("QID = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ac_idx));
		MTWF_PRINT("PriSel = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), pri_sel));
		MTWF_PRINT("Nss = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), nss));
		MTWF_PRINT("PerPktBW = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), per_pkt_bw));
		MTWF_PRINT("PrimaryBWSel = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ch_offset));
		MTWF_PRINT("STBC = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), stbc));
		MTWF_PRINT("LDPC = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ldpc));
		MTWF_PRINT("Preamble = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), preamble));
		MTWF_PRINT("FixedPayload = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), fixed_payload));
		MTWF_PRINT("Thermal Value = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), thermal_val));
	}

	if (ExtendInfo) {
		/* TX information */
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("\t\tTX INFO\n");
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("Sequence = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), seq));
		MTWF_PRINT("TxDoneCount = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ATE_TXDONE_CNT));
		MTWF_PRINT("TxedCount = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ATE_TXED_CNT));

		if (control_band_idx == 0) {
			/* RX information */
			MTWF_PRINT("=============================================\n");
			MTWF_PRINT("\t\tRX INFO\n");
			MTWF_PRINT("=============================================\n");
			MTWF_PRINT("RxTotalCnt = %d\n", ATECtrl->rx_stat.RxTotalCnt[control_band_idx]);
			MTWF_PRINT("RxMacMdrdyCount = %d\n", ATECtrl->rx_stat.RxMacMdrdyCount);
			MTWF_PRINT("RxMacFCSErrCount = %d\n", ATECtrl->rx_stat.RxMacFCSErrCount);
		}
#ifdef DBDC_MODE
		else {
			if (Info != NULL) {
				/* RX information */
				MTWF_PRINT("=============================================\n");
				MTWF_PRINT("\t\tRX INFO\n");
				MTWF_PRINT("=============================================\n");
				MTWF_PRINT("RxTotalCnt = %d\n", ATECtrl->rx_stat.RxTotalCnt[control_band_idx]);
				MTWF_PRINT("RxMacMdrdyCount = %d\n", ATECtrl->rx_stat.RxMacMdrdyCount_band1);
				MTWF_PRINT("RxMacFCSErrCount = %d\n", ATECtrl->rx_stat.RxMacFCSErrCount_band1);
			}
		}
#endif /* DBDC_MODE */
		/* TX power information */
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("\t\tTx Power INFO\n");
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("TxPower0 = %d\n", ATECtrl->TxPower0);
		MTWF_PRINT("TxPower1 = %d\n", ATECtrl->TxPower1);
		MTWF_PRINT("TxPower2 = %d\n", ATECtrl->TxPower2);
		MTWF_PRINT("TxPower3 = %d\n", ATECtrl->TxPower3);
#ifdef ATE_TXTHREAD
		/* TX thread information */
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("\t\tATE TX Thread INFO\n");
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("Current_Init_Thread = %d\n", ATECtrl->current_init_thread);
		MTWF_PRINT("Dequeue Count = %d\n", ATECtrl->deq_cnt);
#endif /* ATE_TXTHREAD */
#ifdef TXBF_SUPPORT
		/* BF related information */
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("\t\tBF Band %d INFO\n", control_band_idx);
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("fgEBfEverEnabled = %d\n", TESTMODE_GET_PARAM(ad, control_band_idx, fgEBfEverEnabled));
		MTWF_PRINT("TXBF INFO Length = %d\n", ATECtrl->txbf_info_len);

		MTWF_PRINT("ETXBF = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ebf));
		MTWF_PRINT("ITXBF = %d\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), ibf));
#endif /* TXBF_SUPPORT */
		/* MU related information */
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("\t\tMU INFO\n");
		MTWF_PRINT("=============================================\n");
		MTWF_PRINT("MU Enable = %d\n", ATECtrl->mu_enable);
		MTWF_PRINT("MU Users = %d\n", ATECtrl->mu_usrs);
		MTWF_PRINT("wcid_ref = %d\n", ATECtrl->wcid_ref);

	}

	return TRUE;
}

INT32 ShowATERUInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_show("ATERUINFO", &pAd->serv) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	return FALSE;
#endif
}

INT32 ShowATETxDoneInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_show("ATETXDONE", &pAd->serv) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	return FALSE;
#endif
}

INT32 set_ate_duty_cycle(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Ret = 0;
	UINT32 duty_cycle = 0;

	/* Sanity check for input parameter */
	if (Arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"No parameters!!\n");
		goto err0;
	}

	MTWF_PRINT("%s: Duty cycle=%s%%\n", __func__, Arg);

	duty_cycle = simple_strtol(Arg, 0, 10);

	if (duty_cycle > 100) /*duty_cycle must >= 0*/
		goto err1;

	Ret = ATEOp->SetDutyCycle(pAd, duty_cycle);

	if (!Ret)
		return TRUE;

err1:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"Unexpected input!!\n");
err0:
	return FALSE;
}


INT32 set_ate_pkt_tx_time(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Ret = 0;
	UINT32 pkt_tx_time = 0;

	/* Sanity check for input parameter */
	if (Arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"No parameters!!\n");
		goto err0;
	}

	MTWF_PRINT("%s: Pkt Tx time=%sus\n", __func__, Arg);

	pkt_tx_time = simple_strtol(Arg, 0, 10);

	Ret = ATEOp->SetPktTxTime(pAd, pkt_tx_time);

	if (!Ret)
		return TRUE;

err0:
	return FALSE;
}


INT32 set_ate_control_band_idx(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_set_ext("ATECTRLBANDIDX", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 ret = 0;
	UCHAR control_band_idx;

	/* Sanity check for input parameter */
	if (Arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"No parameters!!\n");
		goto err;
	}

	MTWF_PRINT("%s: Control band_idx=%s\n", __func__, Arg);

	control_band_idx = simple_strtol(Arg, 0, 10);
	ATECtrl->control_band_idx = control_band_idx;

	if (!ret)
		return TRUE;

err:
	return FALSE;
#endif
}

INT32 set_ate_show_rx_stat(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 ret = 0;
	UCHAR i = 0;
	UCHAR band_idx;
	RX_STAT_CNT *rx_stat_cnt;
	RX_STATISTIC_RXV *rx_stat_rxv;
	TEST_RX_STAT_BAND_INFO rx_stat_band_info;
	TEST_RX_STAT_COMM_INFO rx_stat_comm_info;

	/* sanity check for input parameter */
	if (!Arg)
		goto error1;

	/* parameter parsing */
	band_idx = simple_strtol(Arg, 0, 10);

	if (band_idx >= DBDC_BAND_NUM)
		goto error2;

	MTWF_PRINT("%s: band_idx: %d\n", __func__, band_idx);

	NdisZeroMemory(&rx_stat_band_info, sizeof(rx_stat_band_info));
	NdisZeroMemory(&rx_stat_comm_info, sizeof(rx_stat_comm_info));

	rx_stat_cnt = pAd->rx_stat_cnt + band_idx;
	rx_stat_rxv = pAd->rx_stat_rxv + band_idx;

	MTWF_PRINT("rcpi: ");
	for (i = 0; i < 4; i++) {
		MTWF_PRINT("%d ", rx_stat_rxv->RCPI[i]);
	}
	MTWF_PRINT("\n");

	MTWF_PRINT("rssi: ");
	for (i = 0; i < 4; i++) {
		MTWF_PRINT("%d ", rx_stat_rxv->RSSI[i]);
	}
	MTWF_PRINT("\n");

	MTWF_PRINT("fagc rssi ib: ");
	for (i = 0; i < 4; i++) {
		MTWF_PRINT("%d ", rx_stat_rxv->FAGC_RSSI_IB[i]);
	}
	MTWF_PRINT("\n");

	MTWF_PRINT("fagc rssi wb: ");
	for (i = 0; i < 4; i++) {
		MTWF_PRINT("%d ", rx_stat_rxv->FAGC_RSSI_WB[i]);
	}
	MTWF_PRINT("\n");

	/* read statistic from firmware */
	chip_get_rx_stat_band(pAd, band_idx, 0, &rx_stat_band_info);
	chip_get_rx_stat_comm(pAd, band_idx, 0, &rx_stat_comm_info);

	/* update cumulated rx stat count */
	rx_stat_cnt->all_mac_rx_mdrdy_cnt += rx_stat_band_info.mac_rx_mdrdy_cnt;
	rx_stat_cnt->all_mac_rx_fcs_err_cnt += rx_stat_band_info.mac_rx_fcs_err_cnt;
	rx_stat_cnt->all_mac_rx_len_mismatch += rx_stat_band_info.mac_rx_len_mismatch;
	rx_stat_cnt->all_mac_rx_fifo_full += rx_stat_comm_info.rx_fifo_full;
	rx_stat_cnt->all_mac_rx_ok_cnt += rx_stat_band_info.mac_rx_fcs_ok_cnt;

	if (rx_stat_cnt->all_mac_rx_mdrdy_cnt == 0)
		rx_stat_cnt->all_per = 0;
	else
		rx_stat_cnt->all_per = (rx_stat_cnt->all_mac_rx_mdrdy_cnt - rx_stat_cnt->all_mac_rx_ok_cnt) * 100 / rx_stat_cnt->all_mac_rx_mdrdy_cnt;

	MTWF_PRINT("mac_rx_fcs_err_cnt: %d\n", rx_stat_band_info.mac_rx_fcs_err_cnt);
	MTWF_PRINT("mac_rx_len_mismatch: %d\n", rx_stat_band_info.mac_rx_len_mismatch);
	MTWF_PRINT("mac_rx_fifo_full: %d\n", rx_stat_comm_info.rx_fifo_full);
	MTWF_PRINT("mac_rx_mdrdy_cnt: %d\n", rx_stat_band_info.mac_rx_mdrdy_cnt);
	MTWF_PRINT("phy_rx_pd_cck: %d\n", rx_stat_band_info.phy_rx_pd_cck);
	MTWF_PRINT("phy_rx_pd_ofdm: %d\n", rx_stat_band_info.phy_rx_pd_ofdm);
	MTWF_PRINT("phy_rx_sig_err_cck: %d\n", rx_stat_band_info.phy_rx_sig_err_cck);
	MTWF_PRINT("phy_rx_sfd_err_cck: %d\n", rx_stat_band_info.phy_rx_sfd_err_cck);
	MTWF_PRINT("phy_rx_sig_err_ofdm: %d\n", rx_stat_band_info.phy_rx_sig_err_ofdm);
	MTWF_PRINT("phy_rx_tag_err_ofdm: %d\n", rx_stat_band_info.phy_rx_tag_err_ofdm);
	MTWF_PRINT("phy_rx_mdrdy_cnt_cck: %d\n", rx_stat_band_info.phy_rx_mdrdy_cnt_cck);
	MTWF_PRINT("phy_rx_mdrdy_cnt_ofdm: %d\n", rx_stat_band_info.phy_rx_mdrdy_cnt_ofdm);

	MTWF_PRINT("all_mac_rx_mdrdy_cnt: %d\n", rx_stat_cnt->all_mac_rx_mdrdy_cnt);
	MTWF_PRINT("all_mac_rx_fcs_err_cnt: %d\n", rx_stat_cnt->all_mac_rx_fcs_err_cnt);
	MTWF_PRINT("all_mac_rx_len_mismatch : %d\n", rx_stat_cnt->all_mac_rx_len_mismatch);
	MTWF_PRINT("all_mac_rx_fifo_full : %d\n", rx_stat_cnt->all_mac_rx_fifo_full);
	MTWF_PRINT("all_mac_rx_ok_cnt : %d\n", rx_stat_cnt->all_mac_rx_ok_cnt);
	MTWF_PRINT("all_per : %d\n", rx_stat_cnt->all_per);
	if (!ret)
		return TRUE;

error1:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"invalid argument (no argument)\n");
	return FALSE;

error2:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"invalid band index(%d).\n", band_idx);
	return FALSE;
}

INT32 set_ate_rx_stat_reset(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 ret = 0;
	UCHAR i = 0;
	UCHAR band_idx;
	RX_STAT_CNT *rx_stat_cnt;
	RX_STATISTIC_RXV *rx_stat_rxv;
	TEST_RX_STAT_BAND_INFO rx_stat_band_info;
	TEST_RX_STAT_COMM_INFO rx_stat_comm_info;
	UINT32 control = 0;

	/* sanity check for input parameter */
	if (!Arg)
		goto error1;

	/* parameter parsing */
	band_idx = simple_strtol(Arg, 0, 10);

	if (band_idx >= DBDC_BAND_NUM)
		goto error2;

	MTWF_PRINT("%s: band_idx: %d\n", __func__, band_idx);

	rx_stat_cnt = pAd->rx_stat_cnt + band_idx;
	rx_stat_rxv = pAd->rx_stat_rxv + band_idx;

	/* read statistic from firmware for clear mib counter */
	chip_get_rx_stat_band(pAd, band_idx, 0, &rx_stat_band_info);
	chip_get_rx_stat_comm(pAd, band_idx, 0, &rx_stat_comm_info);

	/* reset phy counter */
	control = 0;
	ret = MtCmdSetPhyCounter(pAd, control, band_idx);
	if (ret)
		goto error3;

	control = 1;
	ret = MtCmdSetPhyCounter(pAd, control, band_idx);
	if (ret)
		goto error3;

	/*reset rcpi/rssi */
	for (i = 0; i < 4; i++) {
		rx_stat_rxv->RCPI[i] = 0;
		rx_stat_rxv->RSSI[i] = -110;
		rx_stat_rxv->FAGC_RSSI_IB[i] = -110;
		rx_stat_rxv->FAGC_RSSI_WB[i] = -110;
	}

	/* reset cumulated rx stat count */
	rx_stat_cnt->all_mac_rx_mdrdy_cnt = 0;
	rx_stat_cnt->all_mac_rx_fcs_err_cnt = 0;
	rx_stat_cnt->all_mac_rx_len_mismatch = 0;
	rx_stat_cnt->all_mac_rx_fifo_full = 0;
	rx_stat_cnt->all_mac_rx_ok_cnt = 0;
	rx_stat_cnt->all_per = 0;

	return TRUE;

error1:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"invalid argument (no argument)\n");
	return FALSE;

error2:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"invalid band index(%d).\n", band_idx);
	return FALSE;

error3:
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"error status(%d) for fw command.\n", ret);
	return FALSE;
}

#if defined(TXBF_SUPPORT) && defined(MT_MAC)
INT SetATEApplyStaToMacTblEntry(RTMP_ADAPTER *pAd)
{
	P_MANUAL_CONN pManual_cfg = &pAd->AteManualConnInfo;
	UINT16 WCID = pManual_cfg->wtbl_idx;
	PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[WCID];

	/* Currently, for MU-MIMO, we only care the VHT/HT Cap Info and VHT MCS set */
	os_move_mem(&pEntry->vht_cap_ie.vht_cap, &pManual_cfg->vht_cap_info, sizeof(pEntry->vht_cap_ie.vht_cap));
	os_move_mem(&pEntry->HTCapability.HtCapInfo, &pManual_cfg->ht_cap_info, sizeof(pEntry->HTCapability.HtCapInfo));
	os_move_mem(&pEntry->vht_cap_ie.mcs_set, &pManual_cfg->vht_mcs_set, sizeof(pEntry->vht_cap_ie.mcs_set));
	return TRUE;
}


INT SetATEApplyStaToAsic(RTMP_ADAPTER *pAd)
{
	P_MANUAL_CONN manual_cfg = &pAd->AteManualConnInfo;
	UINT16 WCID = manual_cfg->wtbl_idx;
	UCHAR *pAddr = &manual_cfg->peer_mac[0];
	MT_WCID_TABLE_INFO_T WtblInfo;
	/* MAC_TABLE_ENTRY *mac_entry = NULL; */
	struct rtmp_mac_ctrl *wtbl_ctrl = &pAd->mac_ctrl;

	if (wtbl_ctrl->wtbl_entry_cnt[0] > 0)
		WCID = (wtbl_ctrl->wtbl_entry_cnt[0] > WCID ? WCID : MCAST_WCID_TO_REMOVE);
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"PSE not init yet!\n");
		return FALSE;
	}

	os_zero_mem(&WtblInfo, sizeof(MT_WCID_TABLE_INFO_T));
	WtblInfo.Wcid = WCID;
	os_move_mem(&WtblInfo.Addr[0], &pAddr[0], 6);
	/* TODO: shiang-MT7615, risk here!!! */
	/* if (WCID < WTBL_MAX_NUM(pAd)) */
	/* mac_entry = &pAd->MacTab.Content[WCID]; */

	if (WCID == MCAST_WCID_TO_REMOVE || WCID == WTBL_MAX_NUM(pAd)) {
		WtblInfo.MacAddrIdx = 0xe;
		WtblInfo.WcidType = MT_WCID_TYPE_BMCAST;
		WtblInfo.CipherSuit = WTBL_CIPHER_NONE;
	} else {
		/* if (!mac_entry) { */
		/* ("%s(): mac_entry is NULL!\n", __FUNCTION__)); */
		/* return; */
		/* } */
		if (pAd->AteManualConnInfo.peer_op_type == OPMODE_AP)
			WtblInfo.WcidType = MT_WCID_TYPE_AP;
		else
			WtblInfo.WcidType = MT_WCID_TYPE_CLI;

		WtblInfo.MacAddrIdx = manual_cfg->ownmac_idx; /* mac_entry->wdev->OmacIdx; */
		/* WtblInfo.Aid = manual_cfg->wtbl_idx; //mac_entry->Aid; */
		WtblInfo.CipherSuit = WTBL_CIPHER_NONE;
		/* if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_WMM_CAPABLE)) */
		WtblInfo.SupportQoS = TRUE;

		if (WMODE_CAP_N(manual_cfg->peer_phy_mode)) {
			WtblInfo.SupportHT = TRUE;
			/* if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_RDG_CAPABLE)) */
			{
				WtblInfo.SupportRDG = TRUE;
			}
			WtblInfo.SmpsMode = 0; /* mac_entry->MmpsMode ; */
			WtblInfo.MpduDensity = 0; /* mac_entry->MpduDensity; */
			WtblInfo.MaxRAmpduFactor = 3; /* mac_entry->MaxRAmpduFactor; */
#ifdef DOT11_VHT_AC

			if (WMODE_CAP_AC(manual_cfg->peer_phy_mode))
				WtblInfo.SupportVHT = TRUE;

#endif /* DOT11_VHT_AC */
		}
	}

	WtblInfo.Aid     = manual_cfg->aid;
	WtblInfo.PfmuId  = manual_cfg->pfmuId;
	WtblInfo.spe_idx = manual_cfg->spe_idx;
	/*
	 *	WtblInfo.rca2    = manual_cfg->rca2;
	 *	WtblInfo.rv      = manual_cfg->rv;
	 */
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"Update WTBL table, WCID=%d, Addr=%02x:%02x:%02x:%02x:%02x:%02x, WtblInfo.MacAddrIdx=%d\n",
		WCID, PRINT_MAC(pAddr), WtblInfo.MacAddrIdx);
	AsicUpdateRxWCIDTableDetail(pAd, WtblInfo);
#ifdef MANUAL_MU

	if (WMODE_CAP_N(manual_cfg->peer_phy_mode)) {
		INT tid;

		for (tid = 0; tid < 4; tid++) {
			AsicUpdateBASession(pAd,
				WtblInfo.Wcid,
				0, 0, 64, TRUE, BA_SESSION_ORI, FALSE);
		}
	}

	asic_dump_wtbl_info(pAd, WtblInfo.Wcid);
#endif /* MANUAL_MU */
	return TRUE;
}


static INT ATEMacStr2Hex(RTMP_STRING *arg, UINT8 *mac)
{
	INT i;
	RTMP_STRING *token, sepValue[] = ":";

	if (arg == NULL)
		return FALSE;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17*/
	if (strlen(arg) < 17)
		return FALSE;

	for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++) {
		if (i > 6)
			break;

		if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
			return FALSE;

		AtoH(token, (&mac[i]), 1);
	}

	if (i != 6)
		return FALSE;

	MTWF_PRINT("\n%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return TRUE;
}


INT ATEManualParsingParam(RTMP_ADAPTER *pAd, RTMP_STRING *type, RTMP_STRING *val)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 mac[MAC_ADDR_LEN] = {0};
	INT op_type = 0;
	INT wtbl_idx = 1;
	INT own_mac_idx = 0;
	INT phy_mode = 0;
	INT bw = BW_20;
	INT nss = 1;
	INT maxrate_mode = MODE_CCK;
	INT maxrate_mcs = 0;
	INT pfmuId = 0, speIdx = 24;
	INT aid = 0;
	UINT8 rca2 = 0, rv = 0;
	UINT8 fgIsSuBFee = 0;
	UINT8 fgIsMuBFee = 0;
	UINT8 fgIsSGIFor20 = 0;
	UINT8 fgIsSGIFor40 = 0;
	UINT8 fgIsSGIFor80 = 0;
	UINT8 fgIsSGIFor160 = 0;
	UINT8 bFeeNsts = 0;
	UINT8 mcsSupport = 0;

	if ((!type) || (!val))
		return FALSE;

	if (!wdev)
		return FALSE;

	/* mac:xx:xx:xx:xx:xx:xx */
	if (strcmp("mac", type) == 0) {
		if (ATEMacStr2Hex(val, &mac[0]) == FALSE) {
			NdisZeroMemory(&mac[0], MAC_ADDR_LEN);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid MAC address(%s), use default\n", (val == NULL ? "" : val));
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid MAC address(%s), use default\n", (val == NULL ? "" : val));
		}

		NdisMoveMemory(&pAd->AteManualConnInfo.peer_mac[0], mac, MAC_ADDR_LEN);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MAC=%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}

	/* type:ap/sta */
	if (strcmp("type", type) == 0) {
		if (strcmp(val, "ap") == 0)
			op_type = OPMODE_AP;
		else if (strcmp(val, "sta") == 0)
			op_type = OPMODE_STA;
		else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid type(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.peer_op_type = op_type;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"TYPE=%d\n", op_type);
	}

	/* wtbl:1~127 */
	if (strcmp("wtbl", type) == 0) {
		if (strlen(val)) {
			wtbl_idx = simple_strtol(val, 0, 10);

			if (wtbl_idx <= 0 || wtbl_idx > 127) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid wtbl idx(%s), use default\n", (val == NULL ? "" : val));
				wtbl_idx = 1;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid wtbl idx(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.wtbl_idx = wtbl_idx;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"WTBL_IDX=%d\n", wtbl_idx);
	}

	/* ownmac:0~4, 0x10~0x1f */
	if (strcmp("ownmac", type)  == 0) {
		if (strlen(val)) {
			own_mac_idx = simple_strtol(val, 0, 10);

			if (!((own_mac_idx >= 0 && own_mac_idx <= 4) || (own_mac_idx >= 0x10 && own_mac_idx <= 0x1f))) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid OwnMac idx(%s), use default\n", (val == NULL ? "" : val));
				own_mac_idx = 1;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid wtbl idx(%s), use default\n", (val == NULL ? "" : val));
		}
		pAd->AteManualConnInfo.ownmac_idx = own_mac_idx;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"OWN_MAC_IDX=%d\n", own_mac_idx);

	}

	/* pfmuId: */
	if (strcmp("pfmuId", type)  == 0) {
		if (strlen(val)) {
			pfmuId = simple_strtol(val, 0, 10);

			if (!(pfmuId >= 0x00 || pfmuId <= 0x3f)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid PFMU idx(%s), use default\n", (val == NULL ? "" : val));
				pfmuId = 0;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid PFMU idx(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.pfmuId = pfmuId;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"PFMU_IDX=%d\n", pfmuId);
	}

	/* aid: */
	if (strcmp("aid", type) == 0) {
		if (strlen(val)) {
			aid = simple_strtol(val, 0, 10);

			if (!(aid >= 0x00 || aid <= 2007)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid aid(%s), use default\n", (val == NULL ? "" : val));
				aid = 0;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid aid(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.aid = aid;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"AID =%d\n", aid);
	}

	/* spe-idx: */
	if (strcmp("speIdx", type)  == 0) {
		if (strlen(val)) {
			speIdx = simple_strtol(val, 0, 10);

			if (!(speIdx >= 0 || speIdx <= 30)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid SPE idx(%s), use default\n", (val == NULL ? "" : val));
				speIdx = 24;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid SPE idx(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.spe_idx = speIdx;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"SPE_IDX=%d\n", speIdx);
	}

	if (strcmp("mubfee", type) == 0) {
		if (strlen(val)) {
			fgIsMuBFee = simple_strtol(val, 0, 10);

			if (!(fgIsMuBFee == 0 || fgIsMuBFee == 1)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid mubfee(%s), use default\n", (val == NULL ? "" : val));
				fgIsMuBFee = 0;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid mubfee(%s), use default\n", (val == NULL ? "" : val));
		}

		if (fgIsMuBFee)
			pAd->AteManualConnInfo.vht_cap_info.bfee_cap_mu = fgIsMuBFee;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"mubfee =%d\n", fgIsMuBFee);
	}

	if (strcmp("sgi160", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor160 = simple_strtol(val, 0, 10);

			if (!(fgIsSGIFor160 == 0 || fgIsSGIFor160 == 1)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid sgi160(%s), use default\n", (val == NULL ? "" : val));
				fgIsSGIFor160 = 0;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid sgi160(%s), use default\n", (val == NULL ? "" : val));
		}

		if (fgIsSGIFor160)
			pAd->AteManualConnInfo.vht_cap_info.sgi_160M = fgIsSGIFor160;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"sgi160 =%d\n", fgIsSGIFor160);
	}

	if (strcmp("sgi80", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor80 = simple_strtol(val, 0, 10);

			if (!(fgIsSGIFor80 == 0 || fgIsSGIFor80 == 1)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid sgi80(%s), use default\n", (val == NULL ? "" : val));
				fgIsSGIFor80 = 0;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid sgi80(%s), use default\n", (val == NULL ? "" : val));
		}

		if (fgIsSGIFor80)
			pAd->AteManualConnInfo.vht_cap_info.sgi_80M = fgIsSGIFor80;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"sgi80 =%d\n", fgIsSGIFor80);
	}

	if (strcmp("sgi40", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor40 = simple_strtol(val, 0, 10);

			if (!(fgIsSGIFor40 == 0 || fgIsSGIFor40 == 1)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid sgi40(%s), use default\n", (val == NULL ? "" : val));
				fgIsSGIFor40 = 0;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid sgi40(%s), use default\n", (val == NULL ? "" : val));
		}

		if (fgIsSGIFor40)
			pAd->AteManualConnInfo.ht_cap_info.ShortGIfor40 = fgIsSGIFor40;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"sgi40 =%d\n", fgIsSGIFor40);
	}

	if (strcmp("sgi20", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor20 = simple_strtol(val, 0, 10);

			if (!(fgIsSGIFor20 == 0 || fgIsSGIFor20 == 1)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid sgi20(%s), use default\n", (val == NULL ? "" : val));
				fgIsSGIFor20 = 0;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid sgi20(%s), use default\n", (val == NULL ? "" : val));
		}

		if (fgIsSGIFor20)
			pAd->AteManualConnInfo.ht_cap_info.ShortGIfor20 = fgIsSGIFor20;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"sgi20 =%d\n", fgIsSGIFor20);
	}

	if (strcmp("rxmcsnss1", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);

			if (mcsSupport > 3) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid rxmcsnss1(%s), use default\n", (val == NULL ? "" : val));
				mcsSupport = 3;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid rxmcsnss1(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.vht_mcs_set.rx_mcs_map.mcs_ss1 = mcsSupport;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"rxmcsnss1 =%d\n", mcsSupport);
	}

	if (strcmp("rxmcsnss2", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);

			if (mcsSupport > 3) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid rxmcsnss2(%s), use default\n", (val == NULL ? "" : val));
				mcsSupport = 3;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid rxmcsnss2(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.vht_mcs_set.rx_mcs_map.mcs_ss2 = mcsSupport;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"rxmcsnss2 =%d\n", mcsSupport);
	}

	if (strcmp("rxmcsnss3", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);

			if (mcsSupport > 3) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid rxmcsnss3(%s), use default\n", (val == NULL ? "" : val));
				mcsSupport = 3;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Invalid rxmcsnss3(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.vht_mcs_set.rx_mcs_map.mcs_ss3 = mcsSupport;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"rxMcsNSS3 =%d\n", mcsSupport);
	}

	if (strcmp("rxmcsnss4", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);

			if (mcsSupport > 3) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid rxmcsnss4(%s), use default\n", (val == NULL ? "" : val));
				mcsSupport = 3;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid rxmcsnss4(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.vht_mcs_set.rx_mcs_map.mcs_ss4 = mcsSupport;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"rxmcsnss4 =%d\n", mcsSupport);
	}

	if (strcmp("subfee", type) == 0) {
		if (strlen(val)) {
			fgIsSuBFee = simple_strtol(val, 0, 10);

			if (!(fgIsSuBFee == 0 || fgIsSuBFee == 1)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid subfee(%s), use default\n", (val == NULL ? "" : val));
				fgIsSuBFee = 0;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid subfee(%s), use default\n", (val == NULL ? "" : val));
		}

		if (fgIsSuBFee)
			pAd->AteManualConnInfo.vht_cap_info.bfee_cap_su = fgIsSuBFee;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"subfee =%d\n", fgIsSuBFee);
	}

	if (strcmp("bfeensts", type) == 0) {
		if (strlen(val)) {
			bFeeNsts = simple_strtol(val, 0, 10);

			if (bFeeNsts > 4) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid bfeensts(%s), use default\n", (val == NULL ? "" : val));
				bFeeNsts = 4;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid bfeensts(%s), use default\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.vht_cap_info.bfee_sts_cap = bFeeNsts;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"bfeensts =%d\n", bFeeNsts);
	}

	/* mode:a/bg/n/ac */
	if (strcmp("mode", type) == 0) {
		RTMP_STRING *tok;

		tok = val;

		while (strlen(tok)) {
			if (*tok == 'b') {
				phy_mode |= WMODE_B;
				tok++;
			} else if (*tok == 'g') {
				if ((*(tok + 1) == 'n') && (strlen(tok) >= 2)) {
					phy_mode |= WMODE_GN;
					tok += 2;
				} else {
					phy_mode |= WMODE_G;
					tok += 1;
				}
			} else if (*tok == 'a') {
				if ((*(tok + 1) == 'n') && (strlen(tok) >= 2)) {
					phy_mode |= WMODE_AN;
					tok += 2;
				} else if ((*(tok + 1) == 'c') && (strlen(tok) >= 2)) {
					phy_mode |= WMODE_AC;
					tok += 2;
				} else {
					phy_mode |= WMODE_A;
					tok += 1;
				}
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid phy_mode %c\n", *tok);
				tok++;
			}
		}

		pAd->AteManualConnInfo.peer_phy_mode = phy_mode;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"phy_mode=%s, convert to PhyMode= 0x%x\n", (val == NULL ? "" : val), phy_mode);
	}

	/* bw:20/40/80/160 */
	if (strcmp("bw", type) == 0) {
		if (strlen(val)) {
			bw = simple_strtol(val, 0, 10);

			switch (bw) {
			case 20:
				bw = BW_20;
				break;

			case 40:
				bw = BW_40;
				break;

			case 80:
				bw = BW_80;
				break;

			case 160:
				bw = BW_160;
				break;

			default:
				bw = BW_20;
				break;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid BW string(%s), use default!\n", (val == NULL ? "" : val));
		}

		pAd->AteManualConnInfo.peer_bw = bw;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BW=%d\n", bw);
	}

	if (strcmp("nss", type) == 0) {
		if (strlen(val)) {
			UINT8 ucTxPath = pAd->Antenna.field.TxPath;

			nss = simple_strtol(val, 0, 10);

#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				UINT8 band_idx = HcGetBandByWdev(wdev);

				if (band_idx == DBDC_BAND0)
					ucTxPath = pAd->dbdc_band0_tx_path;
				else
					ucTxPath = pAd->dbdc_band1_tx_path;
			}
#endif
			if (nss > ucTxPath) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid NSS string(%s), use default!\n", (val == NULL ? "" : val));
				nss = 1;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid NSS setting, use default!\n");
		}

		pAd->AteManualConnInfo.peer_nss = nss;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"NSS=%d\n", nss);
	}

	/* rca2 = 0/1 */
	if (strcmp("rca2", type) == 0) {
		if (strlen(val)) {
			rca2 = simple_strtol(val, 0, 10);

			if (rca2 > 1) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid RCA2 string(%s), use default!\n", (val == NULL ? "" : val));
				rca2 = 0;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid RCA2 setting, use default!\n");
		}

		pAd->AteManualConnInfo.rca2 = rca2;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"RCA2=%d\n", rca2);
	}

	/* rv = 0/1 */
	if (strcmp("rv", type) == 0) {
		if (strlen(val)) {
			rv = simple_strtol(val, 0, 10);

			if (rv > 1) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid RV string(%s), use default!\n", (val == NULL ? "" : val));
				rv = 0;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid RV setting, use default!\n");
		}

		pAd->AteManualConnInfo.rv = rv;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"RV=%d\n", rv);
	}

	/* maxrate:cck/ofdm/htmix/htgf/vht/_0~32 */
	if (strcmp("maxrate", type) == 0) {
		RTMP_STRING *tok;

		if (strlen(val)) {
			tok = rtstrchr(val, '_');

			if (tok && strlen(tok) > 1) {
				*tok = 0;
				tok++;
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Invalid maxmcs setting(%s), use default!\n", (val == NULL ? "" : val));
				goto maxrate_final;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid maxmcs setting(%s), use default!\n", (val == NULL ? "" : val));
			goto maxrate_final;
		}

		if (strlen(tok)) {
			maxrate_mcs = simple_strtol(tok, 0, 10);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"input MCS string(%s) =%d\n", tok, maxrate_mcs);

		}

		if (strcmp(val, "cck") == 0) {
			maxrate_mode = MODE_CCK;

			if (maxrate_mcs > 4)
				maxrate_mcs = 3;
		} else if (strcmp(val, "ofdm") == 0) {
			maxrate_mode = MODE_OFDM;

			if (maxrate_mcs > 7)
				maxrate_mcs = 7;
		} else if (strcmp(val, "htmix") == 0) {
			maxrate_mode = MODE_HTMIX;

			if (maxrate_mcs > 32)
				maxrate_mcs = 32;
		} else if (strcmp(val, "htgf") == 0) {
			maxrate_mode = MODE_HTGREENFIELD;

			if (maxrate_mcs > 32)
				maxrate_mcs = 32;
		} else if (strcmp(val, "vht") == 0) {
			maxrate_mode = MODE_VHT;

			if (maxrate_mcs > 9)
				maxrate_mcs = 9;
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Invalid RateMode string(%s), use default!\n", val);
			maxrate_mode = MODE_CCK;
			maxrate_mcs = 0;
		}

maxrate_final:
		pAd->AteManualConnInfo.peer_maxrate_mode = maxrate_mode;
		pAd->AteManualConnInfo.peer_maxrate_mcs = maxrate_mcs;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MAXRATE=>MODE=%d,MCS=%d\n", maxrate_mode, maxrate_mcs);
	}

	return TRUE;
}


/*
 *	Assoc Parameters:
 *		mac:xx:xx:xx:xx:xx:xx-type:ap/sta-mode:a/b/g/gn/an/ac-bw:20/40/80/160-nss:1/2/3/4-pfmuId:xx-aid:xx-maxrate:
 *
 *	@jeffrey: For MU-MIMO, we need to configure the HT/VHP cap info to emulate different STAs (# of STA >= 2)  which
 *		  supports different Tx and Rx dimension for early algorithm verification
 */
INT SetATEAssocProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	char sep_type = '-', sep_val = ':';
	RTMP_STRING *tok, *param_str, *param_type, *param_val;
	INT stat;
	char ucNsts;
	UINT_32 rate[8];
	RA_PHY_CFG_T TxPhyCfg;
	RTMP_STRING rate_str[64];
	struct wifi_dev *wdev;
	int ret;

	NdisZeroMemory(&pAd->AteManualConnInfo, sizeof(MANUAL_CONN));
	tok = arg;

	while (tok) {
		if (strlen(tok)) {
			param_str = tok;
			tok = rtstrchr(tok, sep_type);

			if (tok) {
				*tok = 0;
				tok++;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"param_str=%s\n", param_str);

			if (strlen(param_str)) {
				param_type = param_str;
				param_val = rtstrchr(param_str, sep_val);

				if (param_val) {
					*param_val = 0;
					param_val++;
				}

				if (strlen(param_type) && param_val && strlen(param_val)) {
					stat = ATEManualParsingParam(pAd, param_type, param_val);

					if (stat == FALSE)
						goto err_dump_usage;
				}
			}
		} else
			break;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "User manual configured peer STA info:\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\tMAC=>0x%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pAd->AteManualConnInfo.peer_mac));
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\tBAND=>%d\n", pAd->AteManualConnInfo.peer_band);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\tOwnMacIdx=>%d\n", pAd->AteManualConnInfo.ownmac_idx);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\tWTBL_Idx=>%d\n", pAd->AteManualConnInfo.wtbl_idx);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\tOperationType=>%d\n", pAd->AteManualConnInfo.peer_op_type);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\tPhyMode=>%d\n", pAd->AteManualConnInfo.peer_phy_mode);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\tBandWidth=>%d\n", pAd->AteManualConnInfo.peer_bw);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\tNSS=>%d\n", pAd->AteManualConnInfo.peer_nss);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\tPfmuId=>%d\n", pAd->AteManualConnInfo.pfmuId);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\tAid=>%d\n", pAd->AteManualConnInfo.aid);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\tSpe_idx=>%d\n", pAd->AteManualConnInfo.spe_idx);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\tMaxRate_Mode=>%d\n", pAd->AteManualConnInfo.peer_maxrate_mode);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\tMaxRate_MCS=>%d\n", pAd->AteManualConnInfo.peer_maxrate_mcs);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Now apply it to hardware!\n");
	/* This applied the manual config info into the mac table entry, including the HT/VHT cap, VHT MCS set */
	SetATEApplyStaToMacTblEntry(pAd);
	/* Fixed rate configuration */
	NdisZeroMemory(&rate_str[0], sizeof(rate_str));
	ret = snprintf(rate_str, sizeof(rate_str), "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d",
			pAd->AteManualConnInfo.wtbl_idx,
			pAd->AteManualConnInfo.peer_maxrate_mode,
			pAd->AteManualConnInfo.peer_bw,
			pAd->AteManualConnInfo.peer_maxrate_mcs,
			pAd->AteManualConnInfo.peer_nss,
			0, 0, 0, 0, 0);
	if (os_snprintf_error(sizeof(rate_str), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"rate_str snprintf error!\n");
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\tSet fixed RateInfo string as %s\n", rate_str);
	/* Set_Fixed_Rate_Proc(pAd, rate_str); */
	ucNsts = asic_get_nsts_by_mcs(pAd, pAd->AteManualConnInfo.peer_maxrate_mode,
							 pAd->AteManualConnInfo.peer_maxrate_mcs,
							 FALSE,
							 pAd->AteManualConnInfo.peer_nss);
	rate[0] = asic_tx_rate_to_tmi_rate(pAd, pAd->AteManualConnInfo.peer_maxrate_mode,
								  pAd->AteManualConnInfo.peer_maxrate_mcs,
								  ucNsts,
								  FALSE,
								  0);
	rate[0] &= 0xfff;
	rate[1] = rate[2] = rate[3] = rate[4] = rate[5] = rate[6] = rate[7] = rate[0];
	os_zero_mem(&TxPhyCfg, sizeof(TxPhyCfg));
	TxPhyCfg.BW      = pAd->AteManualConnInfo.peer_bw;
	TxPhyCfg.ShortGI = FALSE;
	/* TxPhyCfg.ldpc  = HT_LDPC | VHT_LDPC; */
	TxPhyCfg.ldpc    = 0;
	AsicTxCapAndRateTableUpdate(pAd,
								  pAd->AteManualConnInfo.wtbl_idx,
								  &TxPhyCfg,
								  rate,
								  FALSE);
	/* WTBL configuration */
	SetATEApplyStaToAsic(pAd);
	wdev = wdev_search_by_omac_idx(pAd, pAd->AteManualConnInfo.ownmac_idx);
	if (wdev == NULL) {
		MTWF_PRINT("can't find wdev!!\n");
		return FALSE;
	}
	mt_ate_store_tx_info(pAd,
						 HcGetBandByWdev(wdev),
						 wdev_search_by_omac_idx(pAd, pAd->AteManualConnInfo.ownmac_idx),
						 pAd->AteManualConnInfo.peer_mac, &pAd->MacTab.Content[pAd->AteManualConnInfo.wtbl_idx], NULL);
	/* dump WTBL again */
	/* dump_wtbl_info(pAd, pAd->AteManualConnInfo.wtbl_idx); */
	return TRUE;
err_dump_usage:
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Parameter Usage:\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\tiwpriv ra0 set assoc=[mac:hh:hh:hh:hh:hh:hh]-[wtbl:dd]-[ownmac:dd]-[type:xx]-[mode:mmm]-[bw:dd]-[nss:ss]-[maxrate:kkk_dd]\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\tmac: peer's mac address in hex format\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\t\tExample=> mac:00:0c:43:12:34:56\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\twtbl: the WTBL entry index peer will occupied, in range 1~127\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\t\tExample=> wtbl:1\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\townmac: the OwnMAC index we'll used to send frame to this peer, in range 0~4 or 16~31\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\t\tExample=> ownmac:0\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\ttype: peer's operation type, is a ap or sta, allow input: \"ap\" or \"sta\"\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\t\tExample=> type:ap\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\tmode: peer's phy operation mode, allow input: a/b/g/gn/an/ac\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\t\tExample=> mode:aanac	to indicate peer can support A/AN/AC mode\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\tbw: Peer's bandwidth capability, in range to 20/40/80/160\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\t\tExample=> bw:40	indicate peer can support BW_40\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\tnss: Peer's capability for Spatial stream which can tx/rx, in range of 1~4 with restriction of Software/Hardware cap.\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\t\tExample=> nss:2	indicate peer can support 2ss for both tx/rx\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\tmaxrate: Peer's data rate capability for tx/rx, separate as two parts and separate by '_' character\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\t\t\t kkk: phy modulation mode, allow input:'cck', 'ofdm', 'htmix', 'htgf', 'vht'\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\t\t\t dd:phy mcs rate, for CCK:0~3, OFDM:0~7, HT:0~32, VHT:0~9\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\t\tExample=> maxrate:cck_1	indicate we only can transmit CCK and MCS 1(2Mbps) or lower MCS to peer\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\t\tExample=> maxrate:ofdm_3	indicate we only can transmit OFDM and MCS 3(24Mbps) to peer\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\t\t\tExample=> maxrate:htmix_3	indicate we only can transmit OFDM and MCS 3(24Mbps) to peer\n");
	return FALSE;
}



INT SetATETxBfDutInitProc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	BOOLEAN     fgDbdc;
	RTMP_STRING cmdStr[24];
	ULONG       stTimeChk0, stTimeChk1;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
#ifndef CONFIG_WLAN_SERVICE
	struct wifi_dev *wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, control_band_idx, wdev[0]);
#else
	struct wifi_dev *wdev = &pAd->ate_wdev[TESTMODE_GET_BAND_IDX(pAd)][0];
#endif /* CONFIG_WLAN_SERVICE */
	int ret;


	fgDbdc = simple_strtol(Arg, 0, 10) & 1;

	NdisGetSystemUpTime(&stTimeChk0);

	/* Do ATESTART */
	SetATE(pAd, "ATESTART");

#if defined(DOT11_HE_AX)
	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		/* Set DUT as AP type */
		SetATE(pAd, "ATEAP");
		if (IS_MT7915(pAd)) {
			if (IS_MT7915_FW_VER_E1(pAd))
				set_ate_tx_policy(pAd, "2:1");
		}
	}
#endif

	/* set ATEDA=00:11:11:11:11:11 */
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr1[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr1[0][1], 0x11);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr1[0][2], 0x11);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr1[0][3], 0x11);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr1[0][4], 0x11);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr1[0][5], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][1], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][2], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][3], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][4], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][5], 0x11);

	if (IS_MT7915(pAd)) {
		if (IS_MT7915_FW_VER_E1(pAd)) {
			ret = snprintf(cmdStr, sizeof(cmdStr), "4-00:%.2x:%.2x:%.2x:%.2x:%.2x",
				0x11, 0x11, 0x11, 0x11, 0x11);
			if (os_snprintf_error(sizeof(cmdStr), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"cmdStr snprintf error!\n");
				return FALSE;
			}
		} else {
			ret = snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
				0x11, 0x11, 0x11, 0x11, 0x11);
			if (os_snprintf_error(sizeof(cmdStr), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"cmdStr snprintf error!\n");
				return FALSE;
			}
		}
	} else {
		ret = snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
			0x11, 0x11, 0x11, 0x11, 0x11);
		if (os_snprintf_error(sizeof(cmdStr), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmdStr snprintf error!\n");
			return FALSE;
		}
	}
	SetATEDa(pAd, cmdStr);
	/* set ATESA=00:22:22:22:22:22 */
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr2[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr2[0][1], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr2[0][2], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr2[0][3], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr2[0][4], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr2[0][5], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][1], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][2], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][3], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][4], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][5], 0x22);
	/* snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x", */
	/* 0x22, 0x22, 0x22, 0x22, 0x22); */
	/* SetATESa(pAd, cmdStr); */
	AsicDevInfoUpdate(
				pAd,
				HcGetOmacIdx(pAd, wdev),
				TESTMODE_GET_PARAM(pAd, control_band_idx, addr2[0]),
				control_band_idx,
				TRUE,
				DEVINFO_ACTIVE_FEATURE);

	/* set ATEBSSID=00:22:22:22:22:22 */
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr3[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr3[0][1], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr3[0][2], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr3[0][3], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr3[0][4], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr3[0][5], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][1], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][2], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][3], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][4], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][5], 0x22);
	/* snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x", */
	/* 0x22, 0x22, 0x22, 0x22, 0x22); */
	/* SetATEBssid(pAd, cmdStr); */

	ret = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:00:%.2x:%.2x:%.2x:%.2x:%.2x",
			HcGetOmacIdx(pAd, wdev), wdev->bss_info_argument.ucBssIndex, 0x22, 0x22, 0x22, 0x22, 0x22);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}

	MTWF_PRINT("%s: ownmac ID = %d, BSS index = %d\n",
		__func__, HcGetOmacIdx(pAd, wdev), wdev->bss_info_argument.ucBssIndex);

	Set_BssInfoUpdate(pAd, cmdStr);
	/* set ATETXMODE=2 */
	SetATETxMode(pAd, "2");
	/* set ATETXBW=0 */
	SetATETxBw(pAd, "0");
	/* set ATETXGI=0 */
	SetATETxGi(pAd, "0");
	/* Set ATEIPG=999 */
	SetATEIpg(pAd, "999");
	/* Enable i/eBF */
	SetATETXBFProc(pAd, "3");

	if (fgDbdc) {
		/* set ATETXMCS=15 */
		SetATETxMcs(pAd, "15");

		if (IS_MT7915(pAd)) {
			if (control_band_idx == DBDC_BAND0) {
				/* set ATETXANT=3 2T */
				SetATETxAntenna(pAd, "3");
				/* set ATERXANT=3  2R*/
				SetATERxAntenna(pAd, "3");
			} else {
				/* set ATETXANT=12 2T */
				SetATETxAntenna(pAd, "12");
				/* set ATERXANT=12  2R*/
				SetATERxAntenna(pAd, "12");
			}
		} else {
			if (IS_MT7986(pAd)) {
				UINT8 ucTxPath;

#ifdef DBDC_MODE
				if (control_band_idx == DBDC_BAND0)
					ucTxPath = pAd->dbdc_band0_tx_path;
				else
					ucTxPath = pAd->dbdc_band1_tx_path;
#else
				ucTxPath = pAd->Antenna.field.TxPath;
#endif

				switch (ucTxPath) {
				case TX_PATH_4:
					/* set ATETXANT=4 4T */
					SetATETxAntenna(pAd, "15");
					/* set ATERXANT=4  4R*/
					SetATERxAntenna(pAd, "15");
					/* set ATETXMCS=31 */
					SetATETxMcs(pAd, "31");
					break;

				case TX_PATH_3:
					/* set ATETXANT=3 3T */
					SetATETxAntenna(pAd, "7");
					/* set ATERXANT=3  3R*/
					SetATERxAntenna(pAd, "7");
					/* set ATETXMCS=31 */
					SetATETxMcs(pAd, "23");
					break;

				case TX_PATH_2:
					/* set ATETXANT=2 2T */
					SetATETxAntenna(pAd, "3");
					/* set ATERXANT=2  2R*/
					SetATERxAntenna(pAd, "3");
					/* set ATETXMCS=15 */
					SetATETxMcs(pAd, "15");
					break;
				}
			} else if (IS_MT7916(pAd) || IS_MT7981(pAd)) {
				/* set ATETXANT=7 3T */
				SetATETxAntenna(pAd, "7");
				/* set ATERXANT=7  3R*/
				SetATERxAntenna(pAd, "7");
				/* set ATETXMCS=15 */
				SetATETxMcs(pAd, "23");
			} else {
				/* set ATETXANT=3 2T */
				SetATETxAntenna(pAd, "3");
				/* set ATERXANT=3  2R*/
				SetATERxAntenna(pAd, "3");
			}
		}
	} else {
		UINT8 ucTxPath;
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
			if (control_band_idx == DBDC_BAND0)
				ucTxPath = pAd->dbdc_band0_tx_path;
			else
				ucTxPath = pAd->dbdc_band1_tx_path;
		}
#else
		ucTxPath = pAd->Antenna.field.TxPath;

#endif

		switch (ucTxPath) {
		case TX_PATH_2:
			/* set ATETXANT=3 2T */
			SetATETxAntenna(pAd, "3");
			/* set ATERXANT=3  2R*/
			SetATERxAntenna(pAd, "3");

			/* set ATETXMCS=15 */
			SetATETxMcs(pAd, "15");
			break;

		case TX_PATH_3:
			/* set ATETXANT=7 3T */
			SetATETxAntenna(pAd, "7");
			/* set ATERXANT=7  3R*/
			SetATERxAntenna(pAd, "7");

			/* set ATETXMCS=23 */
			SetATETxMcs(pAd, "23");
			break;

		case TX_PATH_4:
		default:
			/* set ATETXANT=15 4T */
			SetATETxAntenna(pAd, "15");
			/* set ATERXANT=15  4R*/
			SetATERxAntenna(pAd, "15");

			/* set ATETXMCS=31 */
			SetATETxMcs(pAd, "31");
			break;
		}
	}

	/* SetATETxPower0(pAd, "14"); */
	TESTMODE_SET_PARAM(pAd, control_band_idx, fgEBfEverEnabled, FALSE);

	NdisGetSystemUpTime(&stTimeChk1);
	MTWF_PRINT("%s: Time consumption : %lu sec\n", __func__, (stTimeChk1 - stTimeChk0) * 1000 / OS_HZ);

	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		/* Update TXCOMMIT */
		SetATE(pAd, "TXCOMMIT");
	}

	/* Enable Tx MAC HW before trigger sounding */
	MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, control_band_idx);

	/* Init iBF phase calibration */
	if (ops->iBFPhaseCalInit)
		ops->iBFPhaseCalInit(pAd);

	return TRUE;
}


INT SetATETxBfGdInitProc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	BOOLEAN     fgDbdc;
	RTMP_STRING cmdStr[80];
	ULONG       stTimeChk0, stTimeChk1;
	UCHAR       *addr1;
#ifndef CONFIG_WLAN_SERVICE
	struct wifi_dev *wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, control_band_idx, wdev[0]);
#else
	struct wifi_dev *wdev = &pAd->ate_wdev[TESTMODE_GET_BAND_IDX(pAd)][0];
#endif /* CONFIG_WLAN_SERVICE */
	int ret;


	fgDbdc = simple_strtol(Arg, 0, 10);

	NdisGetSystemUpTime(&stTimeChk0);

	/* Do ATESTART */
	SetATE(pAd, "ATESTART");

#if defined(DOT11_HE_AX)
	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		/* Set DUT as AP type */
		SetATE(pAd, "ATEAP");
		if (IS_MT7915(pAd)) {
			if (IS_MT7915_FW_VER_E1(pAd))
				set_ate_tx_policy(pAd, "2:1");
		}
	}
#endif

	/* set ATEDA=00:22:22:22:22:22 */
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr1[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr1[0][1], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr1[0][2], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr1[0][3], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr1[0][4], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr1[0][5], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][1], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][2], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][3], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][4], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr1[0][5], 0x22);

	if (IS_MT7915(pAd)) {
		if (IS_MT7915_FW_VER_E1(pAd)) {
			ret = snprintf(cmdStr, sizeof(cmdStr), "4-00:%.2x:%.2x:%.2x:%.2x:%.2x",
				0x22, 0x22, 0x22, 0x22, 0x22);
			if (os_snprintf_error(sizeof(cmdStr), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"cmdStr snprintf error!\n");
				return FALSE;
			}
		} else {
			ret = snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
				0x22, 0x22, 0x22, 0x22, 0x22);
			if (os_snprintf_error(sizeof(cmdStr), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"cmdStr snprintf error!\n");
				return FALSE;
			}
		}
	} else {
		ret = snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
			0x22, 0x22, 0x22, 0x22, 0x22);
		if (os_snprintf_error(sizeof(cmdStr), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmdStr snprintf error!\n");
			return FALSE;
		}
	}
	SetATEDa(pAd, cmdStr);

	/* set ATESA=00:11:11:11:11:11 */
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr2[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr2[0][1], 0x11);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr2[0][2], 0x11);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr2[0][3], 0x11);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr2[0][4], 0x11);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr2[0][5], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][1], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][2], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][3], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][4], 0x11);
	TESTMODE_SET_PARAM(pAd, 0, addr2[0][5], 0x11);

	/* snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x", */
	/* 0x11, 0x11, 0x11, 0x11, 0x11); */
	/* SetATESa(pAd, cmdStr); */
	MTWF_PRINT("%s: control_band_idx = %d\n", __func__, control_band_idx);

	AsicDevInfoUpdate(
				pAd,
				HcGetOmacIdx(pAd, wdev),
				TESTMODE_GET_PARAM(pAd, control_band_idx, addr2[0]),
				control_band_idx,
				TRUE,
				DEVINFO_ACTIVE_FEATURE);

	/* set ATEBSSID=00:22:22:22:22:22 */
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr3[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr3[0][1], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr3[0][2], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr3[0][3], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr3[0][4], 0x22);
	TESTMODE_SET_PARAM(pAd, control_band_idx, addr3[0][5], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][0], 0x00);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][1], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][2], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][3], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][4], 0x22);
	TESTMODE_SET_PARAM(pAd, 0, addr3[0][5], 0x22);
	/* snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x", */
	/* 0x22, 0x22, 0x22, 0x22, 0x22); */
	/* SetATEBssid(pAd, cmdStr); */

	ret = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:00:%.2x:%.2x:%.2x:%.2x:%.2x",
			HcGetOmacIdx(pAd, wdev), wdev->bss_info_argument.ucBssIndex, 0x22, 0x22, 0x22, 0x22, 0x22);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}

	MTWF_PRINT("%s: ownmac ID = %d, BSS index = %d\n",
		__func__, HcGetOmacIdx(pAd, wdev), wdev->bss_info_argument.ucBssIndex);

	Set_BssInfoUpdate(pAd, cmdStr);
	/* set ATETXMODE=2 */
	SetATETxMode(pAd, "2");
	/* set ATETXMCS=0 */
	SetATETxMcs(pAd, "0");
	/* set ATETXBW=0 */
	SetATETxBw(pAd, "0");
	/* set ATETXGI=0 */
	SetATETxGi(pAd, "0");
	/* Set ATETXCNT=0 */
	SetATETxCount(pAd, "0");
	/* Set ATETXLEN=1024 */
	SetATETxLength(pAd, "1024");

	if (IS_MT7915(pAd)) {
		if (control_band_idx == DBDC_BAND0) {
			/* set ATETXANT=1 1T */
			SetATETxAntenna(pAd, "1");
			/* set ATERXANT=1  1R*/
			SetATERxAntenna(pAd, "1");
		} else {
			/* set ATETXANT=1 1T */
			SetATETxAntenna(pAd, "4");
			/* set ATERXANT=1  1R*/
			SetATERxAntenna(pAd, "4");
		}
	} else {
		/* set ATETXANT=1 1T */
		SetATETxAntenna(pAd, "1");
		/* set ATERXANT=1  1R*/
		SetATERxAntenna(pAd, "1");
	}

	/* Update TXCOMMIT */
	SetATE(pAd, "TXCOMMIT");

	/* Configure WTBL */
	/* iwpriv ra0 set ManualAssoc =mac:222222222222-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:1-pfmuId:0 */
	addr1 = TESTMODE_GET_PARAM(pAd, control_band_idx, addr1[0]);
	ret = snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:ap-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:1-pfmuId:0\n",
			 addr1[0], addr1[1], addr1[2], addr1[3], addr1[4], addr1[5]);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	MTWF_PRINT("%s\n", cmdStr);
	ate_set_manual_assoc(pAd, cmdStr);

	/* Enable Tx MAC HW before trigger sounding */
	MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, control_band_idx);

	NdisGetSystemUpTime(&stTimeChk1);
	MTWF_PRINT("%s: Time consumption : %lu sec\n", __func__, (stTimeChk1 - stTimeChk0) * 1000 / OS_HZ);
	return TRUE;
}


INT32 SetATETxPacketWithBf(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	INT32    i;
	UCHAR    ucWlanId, ucTxCnt, *value, ucBuf[4], cmdStr[32];
	BOOLEAN  fgBf;
	int ret;
#ifdef CONFIG_ATE
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test;
	serv_test = (struct service_test *)(pAd->serv.serv_handle);
#else
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
#endif/*  CONFIG_WLAN_SERVICE */
#endif/* CONFIG_ATE */


	MTWF_PRINT("%s: control_band_idx = %d\n", __func__, control_band_idx);


	if (Arg == NULL)
		return FALSE;

	if (strlen(Arg) != 8)
		return FALSE;

	for (i = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	fgBf     = ucBuf[0];
	ucWlanId = ucBuf[1];
	ucTxCnt  = ucBuf[2];

	/* Assign Wlan ID for fixed rate TxD */
#ifdef CONFIG_WLAN_SERVICE
	CONFIG_SET_PARAM(serv_test, wcid_ref,
			(u_int8)ucWlanId, control_band_idx);
#else
	ATECtrl->wcid_ref = ucWlanId;
#endif

	/* At TxD, enable/disable BF Tx at DW6 bit28 */
	if (fgBf) {
		/* Stop Rx before ready to Tx */
		SetATE(pAd, "RXSTOP");

		/* Invalid iBF profile */
		TxBfProfileTagRead(pAd, 2, TRUE);
#if defined(DOT11_HE_AX)
		TxBfProfileTag_InValid(&pAd->pfmu_tags_info, FALSE);
#else
		TxBfProfileTag_InValid(&pAd->rPfmuTag1, FALSE);
#endif
		TxBfProfileTagWrite(pAd,
					&pAd->rPfmuTag1,
					&pAd->rPfmuTag2,
					2);
		/* ATECtrl->eTxBf = TRUE; */
		/* ATECtrl->iTxBf = TRUE; */
		TESTMODE_SET_PARAM(pAd, control_band_idx, ebf, TRUE);
		TESTMODE_SET_PARAM(pAd, control_band_idx, ibf, TRUE);
		TESTMODE_SET_PARAM(pAd, control_band_idx, fgEBfEverEnabled, TRUE);
		/* Stop Tx when the action of Tx packet is done */
		SetATE(pAd, "TXSTOP");
		/* Set the number of Tx packets */
		ret = snprintf(cmdStr, sizeof(cmdStr), "%d", ucTxCnt);
		if (os_snprintf_error(sizeof(cmdStr), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmdStr snprintf error!\n");
			return FALSE;
		}
		SetATETxCount(pAd, cmdStr);

#if defined(DOT11_HE_AX)
		/* Update TXCOMMIT */
		SetATE(pAd, "TXCOMMIT");
#endif

		/* Start packet Tx */
		SetATE(pAd, "TXFRAME");
	} else {
		if (TESTMODE_GET_PARAM(pAd, control_band_idx, fgEBfEverEnabled) == FALSE) {
			/* Stop Rx before ready to Tx */
			SetATE(pAd, "RXSTOP");

			/* ATECtrl->eTxBf = FALSE; */
			/* ATECtrl->iTxBf = FALSE; */
			TESTMODE_SET_PARAM(pAd, control_band_idx, ebf, FALSE);
			TESTMODE_SET_PARAM(pAd, control_band_idx, ibf, FALSE);
			/* Stop Tx when the action of Tx packet is done */
			SetATE(pAd, "TXSTOP");
			/* Set the number of Tx packets */
			ret = snprintf(cmdStr, sizeof(cmdStr), "%d", ucTxCnt);
			if (os_snprintf_error(sizeof(cmdStr), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"cmdStr snprintf error!\n");
				return FALSE;
			}
			SetATETxCount(pAd, cmdStr);

#if defined(DOT11_HE_AX)
		/* Update TXCOMMIT */
		SetATE(pAd, "TXCOMMIT");
#endif

			/* Start packet Tx */
			SetATE(pAd, "TXFRAME");
		} else {
			/* Invalid iBF profile */
			TxBfProfileTagRead(pAd, 2, TRUE);
#if defined(DOT11_HE_AX)
			TxBfProfileTag_InValid(&pAd->pfmu_tags_info, TRUE);
#else
			TxBfProfileTag_InValid(&pAd->rPfmuTag1, TRUE);
#endif
			TxBfProfileTagWrite(pAd,
						&pAd->rPfmuTag1,
						&pAd->rPfmuTag2,
						2);
			TESTMODE_SET_PARAM(pAd, control_band_idx, fgEBfEverEnabled, FALSE);
		}
	}

	return TRUE;
}


INT32 SetATETxBfChanProfileUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	/* struct _ATE_CTRL    *ATECtrl = &(pAd->ATECtrl); */
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	INT32       i;
	UCHAR       *value;
	RTMP_STRING cmdStr[80];
	CHAR	    value_T[12] = {0};
	UCHAR       strLen;
	BOOLEAN     fgFinalData;
	UINT16      u2Buf[11] = {0};
	UINT16      u2PfmuId,   u2Subcarr;
	INT16       i2Phi11,     i2Phi21,    i2Phi31;
	INT16       i2H11,       i2AngleH11, i2H21, i2AngleH21, i2H31, i2AngleH31, i2H41, i2AngleH41;
	INT32       Ret = 0;
	UINT8       ucTxPath = pAd->Antenna.field.TxPath;
	int         ret;

	if (Arg == NULL)
		return FALSE;

	if (strlen(Arg) != 43)
		return FALSE;

	if (!wdev)
		return FALSE;

	for (i = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) > 3) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		strLen = strlen(value);

		if (strLen & 1) {
			ret = snprintf(value_T, sizeof(value_T) - strlen(value_T), "%s%s", "0", value);
			if (os_snprintf_error(sizeof(value_T), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"value_T snprintf error!\n");
				return FALSE;
			}
			AtoH(value_T, (PCHAR)(&u2Buf[i]), 2);
			u2Buf[i] = be2cpu16(u2Buf[i]);
			i++;
		}
	}

	u2PfmuId   = u2Buf[0];
	u2Subcarr  = u2Buf[1];
	fgFinalData = u2Buf[2];
	i2H11      = (INT16)(u2Buf[3] << 3) >> 3;
	i2AngleH11 = (INT16)(u2Buf[4] << 3) >> 3;
	i2H21      = (INT16)(u2Buf[5] << 3) >> 3;
	i2AngleH21 = (INT16)(u2Buf[6] << 3) >> 3;
	i2H31      = (INT16)(u2Buf[7] << 3) >> 3;
	i2AngleH31 = (INT16)(u2Buf[8] << 3) >> 3;
	i2H41      = (INT16)(u2Buf[9] << 3) >> 3;
	i2AngleH41 = (INT16)(u2Buf[10] << 3) >> 3;
	i2Phi11    = 0;
	i2Phi21    = 0;
	i2Phi31    = 0;

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		UINT8 band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0)
			ucTxPath = pAd->dbdc_band0_tx_path;
		else
			ucTxPath = pAd->dbdc_band1_tx_path;
	}
#endif

	switch (ucTxPath) {
	case TX_PATH_2:
		i2Phi11 = i2AngleH21 - i2AngleH11;
		i2Phi21	  = 0;
		break;

	case TX_PATH_3:
		i2Phi11 = i2AngleH31 - i2AngleH11;
		i2Phi21 = i2AngleH31 - i2AngleH21;
		break;

	case TX_PATH_4:
	default:
#if defined(MT7986)
		i2Phi11 = i2AngleH41 - i2AngleH11;
		i2Phi21 = i2AngleH41 - i2AngleH21;
		i2Phi31 = i2AngleH41 - i2AngleH31;
#elif defined(MT7916) || defined(MT7981)
		i2Phi11 = i2AngleH41 - i2AngleH11;
		i2Phi21 = i2AngleH41 - i2AngleH21;
		i2Phi31 = 0;
#else
		if (pAd->CommonCfg.dbdc_mode) {
			i2Phi11 = i2AngleH21 - i2AngleH11;
			i2Phi21	  = 0;
			i2Phi31	  = 0;
		} else {
			i2Phi11 = i2AngleH41 - i2AngleH11;
			i2Phi21 = i2AngleH41 - i2AngleH21;
			i2Phi31 = i2AngleH41 - i2AngleH31;
		}
#endif
		break;
	}

	MTWF_PRINT("%s: i2AngleH11 = 0x%x, i2AngleH21 = 0x%x, i2AngleH31 = 0x%x, i2AngleH41 = 0x%x\n",
		__func__, i2AngleH11, i2AngleH21, i2AngleH31, i2AngleH41);

	/* Update the tag to enable eBF profile */
	if (fgFinalData) {
		ret = snprintf(cmdStr, sizeof(cmdStr), "%02x:01", u2PfmuId);
		if (os_snprintf_error(sizeof(cmdStr), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmdStr snprintf error!\n");
			return FALSE;
		}
		/* check the range for coverity */
		if (*cmdStr < 256) {
			Set_TxBfProfileTagRead(pAd, cmdStr);
			pAd->rPfmuTag1.rField.ucInvalidProf = TRUE;
			Set_TxBfProfileTagWrite(pAd, cmdStr);
		}
	}

	/* Update the profile data per subcarrier */
	switch (ucTxPath) {
	case TX_PATH_3:
		ret = snprintf(cmdStr, sizeof(cmdStr), "%02x:%03x:%03x:00:%03x:00:000:00:000:00:000:00:000:00:00:00:00:00",
				 u2PfmuId, u2Subcarr,
				 (UINT16)((UINT16)i2Phi11 & 0xFFF),
				 (UINT16)((UINT16)i2Phi21 & 0xFFF));
		if (os_snprintf_error(sizeof(cmdStr), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmdStr snprintf error!\n");
			return FALSE;
		}
		break;

	case TX_PATH_4:
	default:
		ret = snprintf(cmdStr, sizeof(cmdStr), "%02x:%03x:%03x:00:%03x:00:%03x:00:000:00:000:00:000:00:00:00:00:00",
				 u2PfmuId, u2Subcarr,
				 (UINT16)((UINT16)i2Phi11 & 0xFFF),
				 (UINT16)((UINT16)i2Phi21 & 0xFFF),
				 (UINT16)((UINT16)i2Phi31 & 0xFFF));
		if (os_snprintf_error(sizeof(cmdStr), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmdStr snprintf error!\n");
			return FALSE;
		}
		break;
	}

	MTWF_PRINT("%s", __func__);
	/* check the range for coverity */
	if (*cmdStr < 256) {
		Ret = Set_TxBfProfileDataWrite(pAd, cmdStr);
	}

	return Ret;
}


INT32 SetATETxBfProfileRead(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	/* struct _ATE_CTRL    *ATECtrl = &(pAd->ATECtrl); */
	INT32       i;
	UCHAR       ucPfmuId,    *value;
	RTMP_STRING cmdStr[32];
	CHAR	    value_T[12] = {0};
	UCHAR       strLen;
	UINT16      u2Buf[2] = {0};
	UINT16      u2SubCarrier;
	int         ret;

	if (Arg == NULL)
		return FALSE;

	if (strlen(Arg) != 7)
		return FALSE;

	for (i = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 3) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))) || (!isxdigit(*(value + 2))))
			return FALSE;  /*Invalid*/

		strLen = strlen(value);

		if (strLen & 1) {
			ret = snprintf(value_T, sizeof(value_T) - strlen(value_T), "%s%s", "0", value);
			if (os_snprintf_error(sizeof(value_T), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"value_T snprintf error!\n");
				return FALSE;
			}
			AtoH(value_T, (PCHAR)(&u2Buf[i]), 2);
			u2Buf[i] = be2cpu16(u2Buf[i]);
			i++;
		}
	}

	ucPfmuId     = u2Buf[0];
	u2SubCarrier = u2Buf[1];
	ret = snprintf(cmdStr, 11, "%.2x:01:%.2x:%.2x", ucPfmuId, (u2SubCarrier >> 8), (u2SubCarrier & 0xFF));
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}

	/* check the range for coverity */
	if (*cmdStr < 256) {
		Set_TxBfProfileDataRead(pAd, cmdStr);
	}

	return TRUE;
}


INT32 SetATETXBFProc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	UCHAR TxBfEn;
#ifdef MT_MAC
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	MTWF_PRINT("%s: control_band_idx = %d\n", __func__, control_band_idx);
#endif
	TxBfEn = simple_strtol(Arg, 0, 10);
#ifdef MT_MAC

	switch (TxBfEn) {
	case 0:
		/* no BF */
		TESTMODE_SET_PARAM(pAd, control_band_idx, ibf, FALSE);
		TESTMODE_SET_PARAM(pAd, control_band_idx, ebf, FALSE);
		break;

	case 1:
		/* ETxBF */
		TESTMODE_SET_PARAM(pAd, control_band_idx, ibf, FALSE);
		TESTMODE_SET_PARAM(pAd, control_band_idx, ebf, TRUE);
		break;

	case 2:
		/* ITxBF */
		TESTMODE_SET_PARAM(pAd, control_band_idx, ibf, TRUE);
		TESTMODE_SET_PARAM(pAd, control_band_idx, ebf, FALSE);
		break;

	case 3:
		/* Enable TXBF support */
		TESTMODE_SET_PARAM(pAd, control_band_idx, ibf, TRUE);
		TESTMODE_SET_PARAM(pAd, control_band_idx, ebf, TRUE);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Set_ATE_TXBF_Proc: Invalid parameter %d\n", TxBfEn);
		Ret = TRUE;
		break;
	}

#endif

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATEIBfGdCal(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32       i;
	UCHAR       ucGroup,    ucGroup_L_M_H, *value, ucBuf[4];
	BOOLEAN     fgSX2;
	UCHAR       ucPhaseCal, ucPhaseVerifyLnaGainLevel;
	INT32       Ret = 0;

	if (Arg == NULL)
		return FALSE;

	if (strlen(Arg) != 11)
		return FALSE;

	for (i = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	MTWF_PRINT("%s: \n", __func__);
	ucGroup       = ucBuf[0];
	ucGroup_L_M_H = ucBuf[1];
	fgSX2         = ucBuf[2];
	ucPhaseCal    = ucBuf[3];
	ucPhaseVerifyLnaGainLevel = 0;
	Ret = CmdITxBfPhaseCal(pAd,
						   ucGroup,
						   ucGroup_L_M_H,
						   fgSX2,
						   ucPhaseCal,
						   ucPhaseVerifyLnaGainLevel);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATEIBfInstCal(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32       i;
	UCHAR       ucGroup,    ucGroup_L_M_H, *value, ucBuf[5];
	BOOLEAN     fgSX2;
	UCHAR       ucPhaseCal, ucPhaseLnaGainLevel;
	INT32       Ret = 0;

	if (Arg == NULL)
		return FALSE;

	if (strlen(Arg) != 14)
		return FALSE;

	for (i = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	MTWF_PRINT("%s: \n", __func__);
	ucGroup             = ucBuf[0];
	ucGroup_L_M_H       = ucBuf[1];
	fgSX2               = ucBuf[2];
	ucPhaseCal          = ucBuf[3];
	ucPhaseLnaGainLevel = ucBuf[4];
	Ret = CmdITxBfPhaseCal(pAd,
						   ucGroup,
						   ucGroup_L_M_H,
						   fgSX2,
						   ucPhaseCal,
						   ucPhaseLnaGainLevel);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATETxBfLnaGain(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR       ucLnaGain;
	INT32       Ret = 0;

	if (Arg == NULL)
		return FALSE;

	ucLnaGain = simple_strtol(Arg, 0, 10);
	MTWF_PRINT("%s: \n", __func__);
	Ret = CmdTxBfLnaGain(pAd, ucLnaGain);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATEIBfProfileUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	INT32 i;
	UINT16 u2Wcid;
	UCHAR Nr, Nc, PfmuIdx, NdpNss, *value, ucBuf[3], aucPfmuMemRow[8] = {0}, aucPfmuMemCol[8] = {0};
	UCHAR ucTxAntConfig, *addr1 = NULL;
	RTMP_STRING cmdStr[80];
	int ret;
#ifndef CONFIG_WLAN_SERVICE
	struct wifi_dev *wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, control_band_idx, wdev[0]);
#else
	struct wifi_dev *wdev = &pAd->ate_wdev[control_band_idx][0];
#endif /* CONFIG_WLAN_SERVICE */

	if (Arg == NULL)
		return FALSE;

	if (strlen(Arg) != 8)
		return FALSE;

	addr1 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr1[0][0]);

	for (i = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	PfmuIdx = ucBuf[0];

	MTWF_PRINT("%s: band[%d]'s TxAntennaSel = 0x%x\n",
		__func__, control_band_idx, TESTMODE_GET_PARAM(pAd, control_band_idx, tx_ant));
	ucTxAntConfig = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_ant);

	switch (ucTxAntConfig) {
	case 3:
		Nr = 1;
		break;
	case 7:
		Nr = 2;
		break;
	case 12:
		if (IS_MT7915(pAd))
			Nr = (IS_MT7915_FW_VER_E1(pAd)) ? 1 : 3;
		else
			Nr = 3;
		break;
	case 15:
		Nr = 3;
		break;
	default:
		Nr = 3;
		break;
	}

	Nc = ucBuf[2];
	/* Configure iBF tag */
	/* PFMU ID */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", PfmuIdx);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_PfmuIdx(pAd, cmdStr);
	/* ITxBf */
	Set_TxBfProfileTag_BfType(pAd, "0");
	/* BW20 */
	Set_TxBfProfileTag_DBW(pAd, "0");
	/* SU */
	Set_TxBfProfileTag_SuMu(pAd, "0");
	/* PFMU memory allocation */
	mt_WrapIBfCalGetIBfMemAlloc(pAd, aucPfmuMemRow, aucPfmuMemCol);
	ret = snprintf(cmdStr, 24, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", aucPfmuMemCol[0], aucPfmuMemRow[0],
									aucPfmuMemCol[1], aucPfmuMemRow[1],
									aucPfmuMemCol[2], aucPfmuMemRow[2],
									aucPfmuMemCol[3], aucPfmuMemRow[3]);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_Mem(pAd, cmdStr);
	/* Nr:Nc:Ng:LM:CB:HTCE */
	/* snprintf(cmdStr, 18, "%.2x:%.2x:00:01:00:00", Nr, Nc); */
	ret = snprintf(cmdStr, 18, "%.2x:%.2x:00:00:00:00", Nr, Nc);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_Matrix(pAd, cmdStr);
	/* SNR */
	ret = snprintf(cmdStr, 12, "00:00:00:00");
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_SNR(pAd, cmdStr);
	/* SMART Antenna */
	Set_TxBfProfileTag_SmartAnt(pAd, "0");
	/* SE index */
	Set_TxBfProfileTag_SeIdx(pAd, "0");
	/* Rmsd */
	Set_TxBfProfileTag_RmsdThrd(pAd, "0");
	/* MCS threshold */
	ret = snprintf(cmdStr, 18, "00:00:00:00:00:00");
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_McsThrd(pAd, cmdStr);
	/* Time out disable */
	Set_TxBfProfileTag_TimeOut(pAd, "255");
	/* Desired BW20 */
	Set_TxBfProfileTag_DesiredBW(pAd, "0");
	/* Nr */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", Nr);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_DesiredNr(pAd, cmdStr);
	/* Nc */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", Nc);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_DesiredNc(pAd, cmdStr);
	/* Invalid the tag */
	Set_TxBfProfileTag_InValid(pAd, "1");
	/* Update PFMU tag */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", PfmuIdx);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTagWrite(pAd, cmdStr);

	/* Configure the BF StaRec */
	ret = snprintf(cmdStr, sizeof(cmdStr), "01:%.2x:00:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
			wdev->BssIdx, PRINT_MAC(addr1));
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	ate_set_cmm_starec(pAd, cmdStr);

	switch (Nr) {
	case 1:
		NdpNss = 8;  /* MCS8, 2 streams */
		break;

	case 2:
		NdpNss = 16; /* MCS16, 3 streams */
		break;

	case 3:
		NdpNss = 24; /* MCS24, 4 streams */
		break;

	default:
		NdpNss = 24;
		break;
	}

	if (IS_MT7915(pAd))
		u2Wcid = (IS_MT7915_FW_VER_E1(pAd)) ? 4 : 1;
	else
		u2Wcid = 1;

	ret = snprintf(cmdStr, sizeof(cmdStr), "%.2x:00:%.2x:00:00:00:%.2x:00:02:%.2x:%.2x:00:00:00:00:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
					u2Wcid, PfmuIdx, NdpNss, Nc, Nr,
					aucPfmuMemRow[0], aucPfmuMemCol[0],
					aucPfmuMemRow[1], aucPfmuMemCol[1],
					aucPfmuMemRow[2], aucPfmuMemCol[2],
					aucPfmuMemRow[3], aucPfmuMemCol[3]);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}

	Set_StaRecBfUpdate(pAd, cmdStr);

	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", u2Wcid);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_StaRecBfRead(pAd, cmdStr);

	/* Configure WTBL */
	/* iwpriv ra0 set ManualAssoc =mac:222222222222-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:2-pfmuId:0 */
	ret = snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:%d-pfmuId:%d\n",
			 PRINT_MAC(addr1),
			 (Nc + 1), PfmuIdx);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	ate_set_manual_assoc(pAd, cmdStr);

	return TRUE;
}


INT32 SetATEEBfProfileConfig(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	INT32 i;
	UINT16 u2Wcid;
	UCHAR Nr, Nc, PfmuIdx, NdpNss, *value, ucBuf[3], aucPfmuMemRow[8] = {0}, aucPfmuMemCol[8] = {0};
	UCHAR ucTxAntConfig, *addr1 = NULL;
	RTMP_STRING cmdStr[80];
	int ret;
#ifndef CONFIG_WLAN_SERVICE
	struct wifi_dev *wdev = (struct wifi_dev *)TESTMODE_GET_PARAM(pAd, control_band_idx, wdev[0]);
#else
	struct wifi_dev *wdev = &pAd->ate_wdev[control_band_idx][0];
#endif /* CONFIG_WLAN_SERVICE */

	if (Arg == NULL)
		return FALSE;

	if (strlen(Arg) != 8)
		return FALSE;

	addr1 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr1[0][0]);
	for (i = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	PfmuIdx = ucBuf[0];

	ucTxAntConfig = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_ant);
	switch (ucTxAntConfig) {
	case 3:
		Nr = 1;
		break;
	case 7:
		Nr = 2;
		break;
	case 12:
		if (IS_MT7915(pAd))
			Nr = (IS_MT7915_FW_VER_E1(pAd)) ? 1 : 3;
		else
			Nr = 3;
		break;
	case 15:
		Nr = 3;
		break;
	default:
		Nr = 3;
		break;
	}

	Nc = ucBuf[2];
	/* Configure iBF tag */
	/* PFMU ID */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", PfmuIdx);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_PfmuIdx(pAd, cmdStr);
	/* ETxBf */
	Set_TxBfProfileTag_BfType(pAd, "1");
	/* BW20 */
	Set_TxBfProfileTag_DBW(pAd, "0");
	/* SU */
	Set_TxBfProfileTag_SuMu(pAd, "0");
	/* PFMU memory allocation */
	mt_WrapIBfCalGetEBfMemAlloc(pAd, aucPfmuMemRow, aucPfmuMemCol);
	ret = snprintf(cmdStr, 24, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", aucPfmuMemCol[0], aucPfmuMemRow[0],
									aucPfmuMemCol[1], aucPfmuMemRow[1],
									aucPfmuMemCol[2], aucPfmuMemRow[2],
									aucPfmuMemCol[3], aucPfmuMemRow[3]);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_Mem(pAd, cmdStr);
	/* Nr:Nc:Ng:LM:CB:HTCE */
	ret = snprintf(cmdStr, 18, "%.2x:%.2x:00:01:00:00", Nr, Nc);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_Matrix(pAd, cmdStr);
	/* SNR */
	ret = snprintf(cmdStr, 12, "00:00:00:00");
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_SNR(pAd, cmdStr);
	/* SMART Antenna */
	Set_TxBfProfileTag_SmartAnt(pAd, "0");
	/* SE index */
	Set_TxBfProfileTag_SeIdx(pAd, "0");
	/* Rmsd */
	Set_TxBfProfileTag_RmsdThrd(pAd, "0");
	/* MCS threshold */
	ret = snprintf(cmdStr, 18, "00:00:00:00:00:00");
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_McsThrd(pAd, cmdStr);
	/* Invalid the tag */
	Set_TxBfProfileTag_InValid(pAd, "1");
	/* Update PFMU tag */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", PfmuIdx);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTagWrite(pAd, cmdStr);

	/* Configure the BF StaRec */
	ret = snprintf(cmdStr, sizeof(cmdStr), "01:%.2x:00:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
				wdev->BssIdx, PRINT_MAC(addr1));
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}

	ate_set_cmm_starec(pAd, cmdStr);

	switch (Nr) {
	case 1:
		NdpNss = 8;  /* MCS8, 2 streams */
		break;

	case 2:
		NdpNss = 16; /* MCS16, 3 streams */
		break;

	case 3:
		NdpNss = 24; /* MCS24, 4 streams */
		break;

	default:
		NdpNss = 24;
		break;
	}

	if (IS_MT7915(pAd)) {
		u2Wcid = (IS_MT7915_FW_VER_E1(pAd)) ? 4 : 1;
	} else {
		u2Wcid = 1;
	}

	ret = snprintf(cmdStr, sizeof(cmdStr), "%.2x:00:%.2x:00:01:00:%.2x:00:02:%.2x:%.2x:00:00:00:00:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
					u2Wcid, PfmuIdx, NdpNss, Nc, Nr,
					aucPfmuMemRow[0], aucPfmuMemCol[0],
					aucPfmuMemRow[1], aucPfmuMemCol[1],
					aucPfmuMemRow[2], aucPfmuMemCol[2],
					aucPfmuMemRow[3], aucPfmuMemCol[3]);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}

	Set_StaRecBfUpdate(pAd, cmdStr);

	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", u2Wcid);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_StaRecBfRead(pAd, cmdStr);

	/* Configure WTBL */
	/* iwpriv ra0 set ManualAssoc =mac:222222222222-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:2-pfmuId:0 */
	ret = snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:%d-pfmuId:%d\n",
			 PRINT_MAC(addr1),
			 (Nc + 1), PfmuIdx);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	ate_set_manual_assoc(pAd, cmdStr);

	return TRUE;
}


INT32 SetATEIBfPhaseComp(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32   Ret,         i;
	UCHAR   ucBW,        ucGroup, ucBand, ucDbdcBandIdx, *value, ucBuf[6];
	BOOLEAN fgRdFromE2p, fgDisComp, fgBw160Nc;

	if (Arg == NULL)
		return FALSE;

	if ((strlen(Arg) != 14) && (strlen(Arg) != 17))
		return FALSE;

	for (i = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	ucBW          = ucBuf[0];
	ucDbdcBandIdx = ucBuf[1];
	ucGroup       = ucBuf[2];
	fgRdFromE2p   = ucBuf[3];
	fgDisComp     = ucBuf[4];
	fgBw160Nc     = (i == 6) ? ucBuf[4] : 0;
	ucBand = (ucGroup == 1) ? 1 : 0;
	Ret = CmdITxBfPhaseComp(pAd, ucBW, ucBand, ucDbdcBandIdx, ucGroup, fgRdFromE2p, fgDisComp);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATEIBfPhaseVerify(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
	INT32   Ret, i;
	UCHAR   ucGroup, ucGroup_L_M_H, ucPhaseCalType, ucBand, *value, ucBuf[6];
	BOOLEAN fgSX2,   fgRdFromE2p;
	UCHAR   ucPhaseVerifyLnaGainLevel;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (Arg == NULL)
		return FALSE;

	if (strlen(Arg) != 17)
		return FALSE;

	for (i = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "\n");
	MTWF_PRINT("%s: \n", __func__);
	ucGroup        = ucBuf[0];
	ucGroup_L_M_H  = ucBuf[1];
	fgSX2          = ucBuf[2];
	ucPhaseCalType = ucBuf[3];
	ucPhaseVerifyLnaGainLevel = ucBuf[4];
	fgRdFromE2p    = ucBuf[5];
	ucBand = (ucGroup == 1) ? 1 : 0;
	Ret = CmdITxBfPhaseComp(pAd,
							BW_20,
							ucBand,
							fgSX2,
							ucGroup,
							fgRdFromE2p,
							FALSE);

	if (Ret) {
		/* Free memory allocated by iBF phase calibration */
		if (ops->iBFPhaseFreeMem)
			ops->iBFPhaseFreeMem(pAd);

		return FALSE;
	}

	Ret = CmdITxBfPhaseCal(pAd,
						   ucGroup,
						   ucGroup_L_M_H,
						   fgSX2,
						   ucPhaseCalType,
						   ucPhaseVerifyLnaGainLevel);

	if (Ret) {
		/* Free memory allocated by iBF phase calibration */
		if (ops->iBFPhaseFreeMem)
			ops->iBFPhaseFreeMem(pAd);

		return FALSE;
	}

	return TRUE;
}


INT32 SetATETxBfPhaseE2pUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32   i;
	UCHAR   ucGroup, ucUpdateAllType, *value, ucBuf[3];
	BOOLEAN fgSX2;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (Arg == NULL)
		return FALSE;

	if (strlen(Arg) != 8)
		return FALSE;

	for (i = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	ucGroup         = ucBuf[0];
	fgSX2           = ucBuf[1];
	ucUpdateAllType = ucBuf[2];

	/* Bit0   : BW160 ?
	   Bit1~3 : reserved
	   Bit4~5 : 0(Clean all), 1(Clean 2G iBF E2p only), 2(Clean 5G iBF E2p only)
	   Bit6~7 : reserved
	*/
	switch (fgSX2 >> 4)
	{
	case CLEAN_ALL:
		pAd->u1IbfCalPhase2G5GE2pClean = 0; // Clean all
		break;
	case CLEAN_2G:
		pAd->u1IbfCalPhase2G5GE2pClean = 1; // Clean 2G
		break;
	case CLEAN_5G:
		pAd->u1IbfCalPhase2G5GE2pClean = 2; // Clean 5G
		break;
	default:
		pAd->u1IbfCalPhase2G5GE2pClean = 0; // Clean all
		break;
	}

	ops->iBFPhaseCalE2PUpdate(pAd, ucGroup, fgSX2, ucUpdateAllType);
	return TRUE;
}


INT32 SetATETxSoundingProc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32         Ret = 0;
	UCHAR         SoundingMode;
	struct _ATE_CTRL      *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_PRINT("%s: SoundingMode = %s\n", __func__, Arg);
	SoundingMode = simple_strtol(Arg, 0, 10);
	Ret = ATEOp->SetATETxSoundingProc(pAd, SoundingMode);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}

INT32 SetATEConTxETxBfInitProc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	UCHAR       control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT8       loop_index, u1Spe;
	INT         status = TRUE;
	CHAR        *value = 0;
	UINT8       TxMode = 0;
	UINT8       MCS = 0;
	UINT8       BW = 0;
	UINT8       BFBW = 0;
	UINT8       VhtNss = 0;
	UINT8       TRxStream = 0;
	UINT8       Power = 0;
	UINT8       Channel = 0;
	UINT8       Channel2 = 0;
	UINT8       Channl_band = 0;
	UINT16      TxPktLength = 0;
	UINT8       Nr = 0;
	UINT8       LM = 0;
	UCHAR       OwnMacIdx = 0;
	UCHAR       WlanIdx = 1;
	UCHAR       BssIdx = 0;
	UCHAR       PfmuId = WlanIdx - 1;
	ULONG       stTimeChk0, stTimeChk1;
	RTMP_STRING cmdStr[80];
	UCHAR       *template, *addr1, *addr2, *addr3;
	int         ret;

	addr1 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr1[0][0]);
	addr2 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr2[0][0]);
	addr3 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr3[0][0]);

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Configure Input Parameter */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/

	/* sanity check for input parameter*/
	if (Arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "No parameters!!\n");
		return FALSE;
	}

	if (strlen(Arg) != 33) {
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong parameter format!!\n");
		return FALSE;
	}

	/* Parsing input parameter */
	for (loop_index = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":"), loop_index++) {
		switch (loop_index) {
		case 0:
			TxMode = simple_strtol(value, 0, 10); /* 2-bit format */
			break;

		case 1:
			MCS = simple_strtol(value, 0, 10); /* 2-bit format */
			break;

		case 2:
			BW = simple_strtol(value, 0, 10); /* 2-bit format */
			if (BW == 5) {
				BFBW = 3; /* BW160 */
			} else if (BW == 0 || BW == 1 || BW == 2 || BW == 6) {
				BFBW = BW; /* BW 20, 40, 80, 8080 */
			} else {
				BFBW = BW + 1; /* BW 10, 5 */
			}
			break;

		case 3:
			VhtNss = simple_strtol(value, 0, 10); /* 2-bit format */
			break;

		case 4:
			TRxStream = simple_strtol(value, 0, 10); /* 2-bit format */
			break;

		case 5:
			Power = simple_strtol(value, 0, 10); /* 2-bit format */
			break;

		case 6:
			Channel = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 7:
			Channel2 = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 8:
			Channl_band = simple_strtol(value, 0, 10); /* 1-bit format */
			break;

		case 9:
			TxPktLength = simple_strtol(value, 0, 10); /* 5-bit format */
			break;

		default: {
			status = FALSE;
			MTWF_PRINT("%s: Set wrong parameters\n", __func__);
			break;
		}
		}
	}

	MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"TxMode = %d, MCS = %d, BW = %d, VhtNss = %d, TRxStream = %d\n", TxMode, MCS, BW, VhtNss, TRxStream);

	MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Power = %d, Channel = %d, Channel2 = %d, Channl_band = %d, TxPktLength = %d\n",
		Power, Channel, Channel2, Channl_band, TxPktLength);

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Load Preliminary Configuration */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
#ifdef CONFIG_AP_SUPPORT
	MTWF_PRINT("%s: control_band_idx = %d\n", __func__, control_band_idx);

#endif /* CONFIG_AP_SUPPORT */


	/* obtain TemplateFrame */
	NdisMoveMemory(TESTMODE_GET_PADDR(pAd, control_band_idx, template_frame), TemplateFrame, 32);

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* DUT TxBf Initialization */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
#if defined(DOT11_HE_AX)
	SetATE(pAd, "ATEAP");
#endif
	NdisGetSystemUpTime(&stTimeChk0);
	/* Start ATE Mode */
	SetATE(pAd, "ATESTART");
	/* Enable ETxBF Capability */
	CmdTxBfHwEnableStatusUpdate(pAd, TRUE, FALSE);

	/* set ATEDA=00:11:11:11:11:11 */
	os_move_mem(addr1, Addr1, MAC_ADDR_LEN);
	ret = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", PRINT_MAC(addr1));
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	SetATEDa(pAd, cmdStr);

	/* set ATESA=00:22:22:22:22:22 */
	os_move_mem(addr2, Addr2, MAC_ADDR_LEN);

	/* set ATEBSSID=00:22:22:22:22:22 */
	os_move_mem(addr3, Addr3, MAC_ADDR_LEN);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		AsicDevInfoUpdate(
			pAd,
			0x0,
			addr2,
			control_band_idx,
			TRUE,
			DEVINFO_ACTIVE_FEATURE);
	}
#endif /* CONFIG_AP_SUPPORT */

	/* BSS Info Update */
	BssIdx = control_band_idx;
	/* OwnMacIdx = 0; */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
			 OwnMacIdx, BssIdx, PRINT_MAC(addr3));
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_BssInfoUpdate(pAd, cmdStr);

	/* Set ATE Tx Frame content */
	template = TESTMODE_GET_PARAM(pAd, control_band_idx, template_frame); /* structure type of TemplateFrame structure is HEADER_802_11 */
	NdisMoveMemory(template +  4, Addr1, MAC_ADDR_LEN);
	NdisMoveMemory(template + 10, Addr2, MAC_ADDR_LEN);
	NdisMoveMemory(template + 16, Addr3, MAC_ADDR_LEN);
	/* Set Tx mode */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", TxMode);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	SetATETxMode(pAd, cmdStr);  /* 0: CCK  1: OFDM  2: HT Mixe dmode 3: HT Green Mode   4: VHT mode  8: HE mode*/
	/* RtmpOsMsDelay(100); */
	/* Set Tx MCS */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", MCS);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	SetATETxMcs(pAd, cmdStr);
	/* RtmpOsMsDelay(100); */
	/* Set Tx BW */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d:%d", BW, BW);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	SetATETxBw(pAd, cmdStr);  /* 0: 20MHz  1: 40MHz  2: 80MHz  3: 10M  4: 5M  5: 160MHz(160C)  6: 160MHz (160NC) */
	/* RtmpOsMsDelay(100); */

	/* Set Tx VhtNss */
	if (TxMode == 4 || TxMode == 8) {
		ret = snprintf(cmdStr, sizeof(cmdStr), "%d", VhtNss);
		if (os_snprintf_error(sizeof(cmdStr), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmdStr snprintf error!\n");
			return FALSE;
		}
		SetATETxNss(pAd, cmdStr);
	}

	/* set ATETXGI=0 */
	SetATETxGi(pAd, "0");
	/* set ATETXLDPC=1 */
	SetATETxLdpc(pAd, "1");

	/* Set ATE Channel */
	TESTMODE_SET_PARAM(pAd, control_band_idx, channel, Channel);
	TESTMODE_SET_PARAM(pAd, control_band_idx, channel_2nd, Channel2);
	/* Set ATE Tx Power = 36 (unit is 0.5 dBm) */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", Power);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	SetATETxPower0(pAd, cmdStr);
	NdisGetSystemUpTime(&stTimeChk1);
	MTWF_PRINT("%s(): DUT Init Time consumption : %lu sec\n", __func__, (stTimeChk1 - stTimeChk0) * 1000 / OS_HZ);

	/* Device info Update */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", OwnMacIdx, PRINT_MAC(addr2), control_band_idx);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_DevInfoUpdate(pAd, cmdStr);
	/* STOP AUTO Sounding */
	Set_Stop_Sounding_Proc(pAd, "1");

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Continuous packet Tx Initializaton */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Set ATE EBF Enable */
	SetATEEBfTx(pAd, "1");  /* need to before switch channel for TxStream config since TxStream only can update correct when etxbf is enable for 3T and 2T */
	/* Set ATE Channel */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d:%d:0:%d", Channel, Channl_band, Channel2);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	SetATEChannel(pAd, cmdStr);
	RtmpOsMsDelay(1000);

	/* Set Tx Rx Ant */
	/* bitwise representration, ex: 0x3 means wifi[0] and wifi[1] ON */
	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		if (TRxStream == 4) {
			if (control_band_idx == DBDC_BAND1) {
				ret = snprintf(cmdStr, sizeof(cmdStr), "1:25");
			if (os_snprintf_error(sizeof(cmdStr), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"cmdStr snprintf error!\n");
				return FALSE;
			}
			} else {
				ret = snprintf(cmdStr, sizeof(cmdStr), "1:24");
				if (os_snprintf_error(sizeof(cmdStr), ret)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"cmdStr snprintf error!\n");
					return FALSE;
				}
			}
			SetATETxAntenna(pAd, cmdStr);  /* 15 (0xF:  wifi[0], wifi[1], wifi[2], wifi[3] on) */
			SetATERxAntenna(pAd, "15");
		} else if (TRxStream == 3) {
			if (control_band_idx == DBDC_BAND1) {
				ret = snprintf(cmdStr, sizeof(cmdStr), "1:25");
				if (os_snprintf_error(sizeof(cmdStr), ret)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"cmdStr snprintf error!\n");
					return FALSE;
				}
			} else {
				ret = snprintf(cmdStr, sizeof(cmdStr), "1:24");
				if (os_snprintf_error(sizeof(cmdStr), ret)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"cmdStr snprintf error!\n");
					return FALSE;
				}
			}
			SetATETxAntenna(pAd, cmdStr);  /* 7 (0x7:  wifi[0], wifi[1], wifi[2] on) */
			SetATERxAntenna(pAd, "7");  /* 7 (0x7:  wifi[0], wifi[1], wifi[2] on) */
		} else if (TRxStream == 2) {
			if (control_band_idx == DBDC_BAND1) {
				ret = snprintf(cmdStr, sizeof(cmdStr), "1:25");
				if (os_snprintf_error(sizeof(cmdStr), ret)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"cmdStr snprintf error!\n");
					return FALSE;
				}

				SetATETxAntenna(pAd, cmdStr);	/* 12 (0xC:  wifi[2], wifi[3] on) */
				SetATERxAntenna(pAd, "12");
			} else {
				ret = snprintf(cmdStr, sizeof(cmdStr), "1:24");
				if (os_snprintf_error(sizeof(cmdStr), ret)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"cmdStr snprintf error!\n");
					return FALSE;
				}

				SetATETxAntenna(pAd, cmdStr);	 /* 3 (0x3:  wifi[0], wifi[1] on) */
				SetATERxAntenna(pAd, "3");
			}
		}
	} else {
		if (TRxStream == 4) {
			SetATETxAntenna(pAd, "15");  /* 15 (0xF:  wifi[0], wifi[1], wifi[2], wifi[3] on) */
			SetATERxAntenna(pAd, "15");
		} else if (TRxStream == 3) {
			SetATETxAntenna(pAd, "7");	 /* 7 (0x7:  wifi[0], wifi[1], wifi[2] on) */
			SetATERxAntenna(pAd, "7");
		} else if (TRxStream == 2) {
			SetATETxAntenna(pAd, "3");	 /* 3 (0x3:  wifi[0], wifi[1] on) */
			SetATERxAntenna(pAd, "3");
		}
	}

	/* Set ATE Tx packet Length (unit is byte)  */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", TxPktLength);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}

	SetATETxLength(pAd, cmdStr);
	/* Set ATE Tx packet number = 0 (Continuous packet Tx)  */
	SetATETxCount(pAd, "0");
	/* Set ATE Tx packet Length = 4 (unit is slot time)  */
	SetATEIpg(pAd, "4");
	/* Set Queue Priority = 1 (WMM_BK Queue)  */
	SetATEQid(pAd, "1");
#if defined(DOT11_HE_AX)
	/* Commit ATE setting. Create WTBL. */
	SetATE(pAd, "TXCOMMIT");
#endif

	/* Set ATE Tx Dequeue size = 4 (allocate 4 packet after receiving 1 free count) (NOT Use Now!!!)*/
	/* ATE Start Continuos Packet Tx */
	/* SetATE(pAd, "TXFRAMESKB"); */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* EBF Profile Cnfiguration */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* set TxBfProfileTag_PFMU ID */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", PfmuId);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_PfmuIdx(pAd, cmdStr);
	/* set TxBfProfileTag_Bf Type */
	Set_TxBfProfileTag_BfType(pAd, "1"); /* 0: iBF  1: eBF */
	/* set TxBfProfileTag_DBW */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", BW); /* 0: 20MHz  1: 40MHz  2: 80MHz  3: 10M  4: 5M  5: 160MHz(160C)  6: 160MHz (160NC) */
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_DBW(pAd, cmdStr);
	/* set TxBfProfileTag_SUMU */
	Set_TxBfProfileTag_SuMu(pAd, "0"); /* 0: SU  1: MU */
	/* PFMU memory allocation */
	ret = snprintf(cmdStr, 24, "00:00:00:01:00:02:00:03");
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_Mem(pAd, cmdStr);

	/* set TxBfProfileTag_Matrix */
	if (IS_MT7915(pAd) && (TxMode == 4 || TxMode == 8) && ((BFBW == 3) || (BFBW == 6))) { /* 2 antenna for each 80MHz band in Harrier 160MHz DBDC mode */
		if (TRxStream == 4)
			Nr = 1;
		else
			MTWF_PRINT("%s: Invalid Configuration for BW160!! For BW160, TxStream number must be 4!!\n", __func__);
	} else {
		if (TRxStream == 4)
			Nr = 3;
		else if (TRxStream == 3)
			Nr = 2;
		else if (TRxStream == 2)
			Nr = 1;
	}

	if (TxMode == 8)
		LM = 3;
	else if (TxMode == 4)
		LM = 2;
	else if (TxMode == 2)
		LM = 1;

	ret = snprintf(cmdStr, 18, "%.2x:00:00:%.2x:00:00", Nr, LM); /* Nr:Nc:Ng:LM:CB:HTCE */
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_Matrix(pAd, cmdStr);
	/* set TxBfProfileTag_SNR */
	ret = snprintf(cmdStr, 12, "00:00:00:00");
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_SNR(pAd, cmdStr);
	/* set TxBfProfileTag_Smart Antenna */
	Set_TxBfProfileTag_SmartAnt(pAd, "0");
	/* set TxBfProfileTag_SE index */

	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		if (control_band_idx == DBDC_BAND1) {
			u1Spe = 25;
		} else {
			u1Spe = 24;
		}
	} else {
		u1Spe = 0;
	}
	ret = snprintf(cmdStr, sizeof(cmdStr), "%u", u1Spe);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_SeIdx(pAd, cmdStr);
	MTWF_PRINT("%s: u1Spe: %u\n", __func__, u1Spe);

	/* set TxBfProfileTag_Rmsd */
	Set_TxBfProfileTag_RmsdThrd(pAd, "0");
	/* set TxBfProfileTag_MCS Threshold */
	ret = snprintf(cmdStr, 18, "00:00:00:00:00:00");
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTag_McsThrd(pAd, cmdStr);
	/* set TxBfProfileTag_Invalid Tag */
	Set_TxBfProfileTag_InValid(pAd, "1");
	/* Update PFMU Tag */
	ret = snprintf(cmdStr, sizeof(cmdStr), "00");
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfProfileTagWrite(pAd, cmdStr);

#if !defined(DOT11_HE_AX)
	/* Station Record Common Info Update */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:00:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", WlanIdx, BssIdx, PRINT_MAC(addr1));
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_StaRecCmmUpdate(pAd, cmdStr);
#endif
	/* Station Record BF Info Update */
	if (TxMode == 8) {
		ret = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:00:01:49:00:49:08:00:%.2x:%.2x:00:00:00:00:00:01:00:02:00:03:00", WlanIdx, BssIdx, PfmuId, Nr, BFBW);
		if (os_snprintf_error(sizeof(cmdStr), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmdStr snprintf error!\n");
			return FALSE;
		}
	}

	else if (TxMode == 4) {
		ret = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:00:01:09:00:09:04:00:%.2x:%.2x:00:00:00:00:00:01:00:02:00:03:00", WlanIdx, BssIdx, PfmuId, Nr, BFBW);
		if (os_snprintf_error(sizeof(cmdStr), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmdStr snprintf error!\n");
			return FALSE;
		}
	}

	else if (TxMode == 2) {
		if (TRxStream == 4) {
			ret = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:00:01:00:18:00:02:00:%.2x:%.2x:00:00:00:00:00:01:00:02:00:03:00", WlanIdx, BssIdx, PfmuId, Nr, BW);
			if (os_snprintf_error(sizeof(cmdStr), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"cmdStr snprintf error!\n");
				return FALSE;
			}
		}

		else if (TRxStream == 3) {
			ret = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:00:01:00:10:00:02:00:%.2x:%.2x:00:00:00:00:00:01:00:02:00:03:00", WlanIdx, BssIdx, PfmuId, Nr, BW);
			if (os_snprintf_error(sizeof(cmdStr), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"cmdStr snprintf error!\n");
				return FALSE;
			}
		}

		else if (TRxStream == 2) {
			ret = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:00:01:00:08:00:02:00:%.2x:%.2x:00:00:00:00:00:01:00:02:00:03:00", WlanIdx, BssIdx, PfmuId, Nr, BW);
			if (os_snprintf_error(sizeof(cmdStr), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"cmdStr snprintf error!\n");
				return FALSE;
			}
		}

	}
	Set_StaRecBfUpdate(pAd, cmdStr);

#if !defined(DOT11_HE_AX)
	/* Configure WTBL and Manual Association */
	/* iwpriv ra0 set ManualAssoc=mac:22:22:22:22:22:22-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:2-pfmuId:0 */
	if (BW == 0) {
		ret = snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:sta-wtbl:%.2x-ownmac:%.2x-mode:aanac-bw:20-nss:2-pfmuId:0\n",
				 PRINT_MAC(addr1), WlanIdx, OwnMacIdx);
		if (os_snprintf_error(sizeof(cmdStr), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmdStr snprintf error!\n");
			return FALSE;
		}

	} else if (BW == 1) {
		ret = snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:sta-wtbl:%.2x-ownmac:%.2x-mode:aanac-bw:40-nss:2-pfmuId:0\n",
				 PRINT_MAC(addr1), WlanIdx, OwnMacIdx);
		if (os_snprintf_error(sizeof(cmdStr), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmdStr snprintf error!\n");
			return FALSE;
		}

	} else {
		ret = snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:sta-wtbl:%.2x-ownmac:%.2x-mode:aanac-bw:80-nss:2-pfmuId:0\n",
				 PRINT_MAC(addr1), WlanIdx, OwnMacIdx);
		if (os_snprintf_error(sizeof(cmdStr), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmdStr snprintf error!\n");
			return FALSE;
		}

	}
	SetATEAssocProc(pAd, cmdStr);
#endif
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* EBF TxBf Apply */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* WTBL Update TxBf Apply */
	ret = snprintf(cmdStr, sizeof(cmdStr), "01:01:00:00:00");
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_TxBfTxApply(pAd, cmdStr);
	/* Read Station Bf Record */
	Set_StaRecBfRead(pAd, "1");
	/* Trigger one shot Sounding packet */
	ret = snprintf(cmdStr, sizeof(cmdStr), "00:01:00:01:00:00:00");
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	Set_Trigger_Sounding_Proc(pAd, cmdStr);
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Periodical Sounding Trigger */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Trigger Periodical Sounding packet */
	ret = snprintf(cmdStr, sizeof(cmdStr), "02:01:FF:01:00:00:00");
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}

	Set_Trigger_Sounding_Proc(pAd, cmdStr);

	/* Enable MAC Rx */
	SetATE(pAd, "RXFRAME");
	return status;
}


INT32 SetATEConTxETxBfGdProc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	UCHAR       control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT8       loop_index;
	INT         status = TRUE;
	CHAR        *value = 0;
	CHAR        OwnMacIdx = 0;
	UCHAR       BssIdx = 0;
	UINT32      TxMode = 0;
	UINT32      MCS = 0;
	UINT32      BW = 0;
	UINT32      Channel = 0;
	UINT8       Channel2 = 0;
	UINT8       Channl_band = 0;
	UINT32      CRvalue = 0;
	ULONG       stTimeChk0, stTimeChk1;
	RTMP_STRING cmdStr[80];
	UCHAR *addr1 = NULL, *addr2 = NULL, *addr3 = NULL;
	int         ret;
#if !defined(DOT11_HE_AX)
	UCHAR       WlanIdx = 1;
#endif

	addr1 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr1[0][0]);
	addr2 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr2[0][0]);
	addr3 = TESTMODE_GET_PADDR(pAd, control_band_idx, addr3[0][0]);


	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Configure Input Parameter */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/

	/* sanity check for input parameter*/
	if (Arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "No parameters!!\n");
		return FALSE;
	}

	if (strlen(Arg) != 18) {
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong parameter format!!\n");
		return FALSE;
	}

	/* Parsing input parameter */
	for (loop_index = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":"), loop_index++) {
		switch (loop_index) {
		case 0:
			TxMode = simple_strtol(value, 0, 10); /* 2-bit format */
			break;

		case 1:
			MCS = simple_strtol(value, 0, 10); /* 2-bit format */
			break;

		case 2:
			BW = simple_strtol(value, 0, 10); /* 2-bit format */
			break;

		case 3:
			Channel = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 4:
			Channel2 = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 5:
			Channl_band = simple_strtol(value, 0, 10); /* 1-bit format */
			break;

		default: {
			status = FALSE;
			MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Set wrong parameters\n");
			break;
		}
		}
	}

	MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"TxMode = %d, MCS = %d, BW = %d, Channel = %d, Channel2 = %d, Channl_band = %d\n",
		TxMode, MCS, BW, Channel, Channel2, Channl_band);
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* GOLDEN TxBf Initialization */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	NdisGetSystemUpTime(&stTimeChk0);
#if defined(DOT11_HE_AX)
	SetATE(pAd, "ATEAP");
#endif
	/* Start ATE Mode */
	SetATE(pAd, "ATESTART");

	/* set ATEDA=00:22:22:22:22:22 */
	os_move_mem(addr1, Addr2, MAC_ADDR_LEN);
	ret = snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", PRINT_MAC(addr1));
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	SetATEDa(pAd, cmdStr);

	/* set ATESA=00:11:11:11:11:11 */
	os_move_mem(addr2, Addr1, MAC_ADDR_LEN);

	/* set ATEBSSID=00:22:22:22:22:22 */
	os_move_mem(addr3, Addr3, MAC_ADDR_LEN);

	/* Set Tx mode */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", TxMode);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	SetATETxMode(pAd, cmdStr);  /* 0: CCK  1: OFDM  2: HT Mixe dmode 3: HT Green Mode   4: VHT mode  8: HE mode*/
	/* Set Tx MCS */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d", MCS);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	SetATETxMcs(pAd, cmdStr);
	/* Set Tx BW */
	ret = snprintf(cmdStr, sizeof(cmdStr), "%d:%d", BW, BW);
	if (os_snprintf_error(sizeof(cmdStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmdStr snprintf error!\n");
		return FALSE;
	}
	SetATETxBw(pAd, cmdStr);  /* 0: 20MHz  1: 40MHz  2: 80MHz  3: 160MHz(160C)  4: 5M  5: 10M  6: 160MHz (160NC) */
	/* set ATETXGI=0 */
	SetATETxGi(pAd, "0");

	/* Set Tx Rx Ant */
	/* bitwise representration, ex: 0x5 means wifi[0] and wifi[2] ON */
	if (IS_MT7915(pAd)) {
#ifdef DBDC_MODE
		if (IS_ATE_DBDC(pAd)) { /* DBDC */
			if (control_band_idx == DBDC_BAND1) {
				if ((BW == 5) || (BW == 6)) {
					SetATETxAntenna(pAd, "12"); /* WF2/3 for BW160C, BW160NC */
					SetATERxAntenna(pAd, "12");
				} else {
					SetATETxAntenna(pAd, "4"); /* WF2 for BW20, BW40, BW80 */
					SetATERxAntenna(pAd, "4");
				}
			} else {
				if ((BW == 5) || (BW == 6)) {
					SetATETxAntenna(pAd, "3"); /* WF0/1 for BW160C, BW160NC */
					SetATERxAntenna(pAd, "3");
				} else {
					SetATETxAntenna(pAd, "1"); /* WF0 for BW20, BW40, BW80 */
					SetATERxAntenna(pAd, "1");
				}
			}
		} else
#endif /* DBDC_MODE */
		{ /* Single Band */
			if ((BW == 5) || (BW == 6)) {
				SetATETxAntenna(pAd, "5"); /* WF 0/2 for BW160C, BW160NC */
				SetATERxAntenna(pAd, "5");
			} else {
				SetATETxAntenna(pAd, "1"); /* WF 0 for BW20, BW40, BW80 */
				SetATERxAntenna(pAd, "1");
			}
		}
	} else if (IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		SetATERxAntenna(pAd, "1");
	} else {
		if ((BW == 5) || (BW == 6)) {
			SetATETxAntenna(pAd, "5"); /* for BW160C, BW160NC */
			SetATERxAntenna(pAd, "5");
		} else {
			SetATETxAntenna(pAd, "1"); /* for BW20, BW40, BW80 */
			SetATERxAntenna(pAd, "1");
		}
	}

#if !defined(DOT11_HE_AX)
	/* Configure WTBL */
	if (BW == 0) {
		snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:sta-wtbl:%.2x-ownmac:%.2x-mode:aanac-bw:20-nss:2-pfmuId:0\n",
				 PRINT_MAC(addr1), WlanIdx, OwnMacIdx);
	} else if (BW == 1) {
		snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:sta-wtbl:%.2x-ownmac:%.2x-mode:aanac-bw:40-nss:2-pfmuId:0\n",
				 PRINT_MAC(addr1), WlanIdx, OwnMacIdx);
	} else {
		snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:sta-wtbl:%.2x-ownmac:%.2x-mode:aanac-bw:80-nss:2-pfmuId:0\n",
				 PRINT_MAC(addr1), WlanIdx, OwnMacIdx);
	}
	SetATEAssocProc(pAd, cmdStr);
#endif
	NdisGetSystemUpTime(&stTimeChk1);
	MTWF_PRINT("%s: SetATETxBfGdInitProc Time consumption : %lu sec\n",
		__func__, (stTimeChk1 - stTimeChk0) * 1000 / OS_HZ);
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Turn On BBP CR for Rx */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* iwpriv ra0 mac 82070280=00008001 */
	PHY_IO_WRITE32(pAd->hdev_ctrl, 0x10280, 0x00008001);
	/* check  */
	PHY_IO_READ32(pAd->hdev_ctrl, 0x10280, &CRvalue);
	MTWF_PRINT("%s: <0x82070280> = 0x%x\n", __func__, CRvalue);
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Sounding Mechanism TRx configuration */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Set Channel Info to ATE Control structure */
	TESTMODE_SET_PARAM(pAd, control_band_idx, channel, Channel);
	TESTMODE_SET_PARAM(pAd, control_band_idx, channel_2nd, Channel2);
#if !defined(CONFIG_AP_SUPPORT) && !defined(DBDC_MODE)
	/* Only to prevent build error: unused variable ¡¥BandIdx¡¦*/
	MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"control_band_idx=%d\n", control_band_idx);

#endif
	/* Set ATE Channel */
	snprintf(cmdStr, sizeof(cmdStr), "%d:%d:0:%d", Channel, Channl_band, Channel2);
	SetATEChannel(pAd, cmdStr);
#if defined(DOT11_HE_AX)
	/* Commit ATE setting. Create WTBL. */
	SetATE(pAd, "TXCOMMIT");
#endif
	RtmpOsMsDelay(1000);
	/* ATE Start Continuos Packet Rx */
	SetATE(pAd, "RXFRAME");
	/* ATE MAC TRx configuration */
	MtATESetMacTxRx(pAd, 6, 1, control_band_idx); /* ENUM_ATE_MAC_RX_RXV: MAC to PHY Rx Enable */
#if defined(DOT11_HE_AX)
	/* Enable Tx MAC HW before trigger sounding */
	MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, control_band_idx);
#endif

	/* Device info Update */
	snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
			 OwnMacIdx, PRINT_MAC(addr2), control_band_idx);
	Set_DevInfoUpdate(pAd, cmdStr);

	BssIdx = control_band_idx;
	snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
			 OwnMacIdx, BssIdx, PRINT_MAC(addr3));
	Set_BssInfoUpdate(pAd, cmdStr);

	return status;
}


INT32 SetATESpeIdx(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	UINT8	loop_index;
	CHAR	*value = 0;
	INT	status = TRUE;

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Configure Input Parameter */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/

	/* sanity check for input parameter*/
	if (Arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"No parameters!!\n");
		return FALSE;
	}

	if (strlen(Arg) != 1) {
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Wrong parameter format!!\n");
		return FALSE;
	}

	/* Parsing input parameter */
	for (loop_index = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":"), loop_index++) {
		switch (loop_index) {
		case 0:
			BF_ON_certification = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Set wrong parameters\n");
			break;
		}
		}
	}

	MTWF_PRINT("%s: BF_ON_certification = %d !!!!!\n", __func__, BF_ON_certification);
	return status;
}


INT32 SetATEEBfTx(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UCHAR	control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT8	loop_index;
	CHAR	*value = 0;
	UINT32	eTxBf = 0;
	INT	status = TRUE;
	UCHAR	addr[6] = {0x00, 0x11, 0x11, 0x11, 0x11, 0x11};
	UCHAR	*pate_pkt;
	UCHAR	WlanIdx = 1;
	UCHAR	PfmuId = WlanIdx - 1;

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Configure Input Parameter */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/

	/* sanity check for input parameter*/
	if (Arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"No parameters!!\n");
		return FALSE;
	}

	if (strlen(Arg) != 1) {
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Wrong parameter format!!\n");
		return FALSE;
	}

	/* Parsing input parameter */
	for (loop_index = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":"), loop_index++) {
		switch (loop_index) {
		case 0:
			eTxBf = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Set wrong parameters\n");
			break;
		}
		}
	}

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* EBF Configuration */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	TESTMODE_SET_PARAM(pAd, control_band_idx, ebf, eTxBf);
	MTWF_PRINT("%s: band[%d] eTxBf = %d !!!!!\n",
		__func__, control_band_idx, TESTMODE_GET_PARAM(pAd, control_band_idx, ebf));
	ATECtrl->wcid_ref = WlanIdx; /* For Sportan certification, only Golden */
	NdisCopyMemory(ATECtrl->pfmu_info[PfmuId].addr, addr, MAC_ADDR_LEN);

	if (TESTMODE_GET_PARAM(pAd, control_band_idx, ebf))
		SetATESpeIdx(pAd, "1");
	else
		SetATESpeIdx(pAd, "0");

	pate_pkt = TESTMODE_GET_PARAM(pAd, control_band_idx, test_pkt);
	/* Generate new packet with new contents */
	MT_ATEComposePkt(pAd, pate_pkt, control_band_idx, 0);
	return status;
}


INT32 SetATEEBFCE(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	UINT8	loop_index;
	CHAR	*value = 0;
	INT	status = TRUE;

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Configure Input Parameter */
	/*----------------------------------------------------------------------------------------------------------------------------------------*/

	/* sanity check for input parameter*/
	if (Arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"No parameters!!\n");
		return FALSE;
	}

	if (strlen(Arg) != 1) {
		MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Wrong parameter format!!\n");
		return FALSE;
	}

	/* Parsing input parameter */
	for (loop_index = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":"), loop_index++) {
		switch (loop_index) {
		case 0:
			g_EBF_certification = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_DBG(pAd, DBG_CAT_BF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Set wrong parameters\n");
			break;
		}
		}
	}

	MTWF_PRINT("%s: g_EBF_certification = %d !!!!!\n", __func__, g_EBF_certification);
	return status;
}


INT32 SetATEEBFCEInfo(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT	status = TRUE;

	MTWF_PRINT("%s: g_EBF_certification = %d !!!!!\n", __func__, g_EBF_certification);
	MTWF_PRINT("%s: BF_ON_certification = %d !!!!!\n", __func__, BF_ON_certification);
	return status;
}

INT32 SetATEEBFCEHelp(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT	status = TRUE;

	MTWF_PRINT("=============================================================================================\n");
	MTWF_PRINT("                            ATE ETxBF Certification Procedure Guide\n");
	MTWF_PRINT("=============================================================================================\n");
	MTWF_PRINT("For HT20 mode\n");
	MTWF_PRINT("\n");
	MTWF_PRINT(" 1)  iwpriv ra0 set ATEEBFCE=1\n");
	MTWF_PRINT(" 2)  iwpriv ra0 set ATEConTxETxBfGdProc=02:00:00:036:112:1 (Use in Golden Device)\n");
	MTWF_PRINT(" 3)  iwpriv ra0 set ATEConTxETxBfInitProc=02:00:00:01:04:18:036:112:1:04000\n");
	MTWF_PRINT(" 4)  iwpriv ra0 set ATETXEBF=1 (Tx packet apply BF On)\n");
	MTWF_PRINT(" 5)  iwpriv ra0 set ATE=TXFRAME\n");
	MTWF_PRINT(" 6)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)\n");
	MTWF_PRINT(" 7)  check IQxel waveform\n");
	MTWF_PRINT(" 8)  iwpriv ra0 set ATETXEBF=0 (Tx packet apply BF Off)\n");
	MTWF_PRINT(" 9)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)\n");
	MTWF_PRINT("10)  check Iqxel waveformn");
	MTWF_PRINT("---------------------------------------------------------------------------------------------\n");
	MTWF_PRINT("For HT40 mode\n");
	MTWF_PRINT("\n");
	MTWF_PRINT(" 1)  iwpriv ra0 set ATEEBFCE=1\n");
	MTWF_PRINT(" 2)  iwpriv ra0 set ATEConTxETxBfGdProc=02:00:01:036:112:1 (Use in Golden Device)\n");
	MTWF_PRINT(" 3)  iwpriv ra0 set ATEConTxETxBfInitProc=02:00:01:01:04:18:036:112:1:04000\n");
	MTWF_PRINT(" 4)  iwpriv ra0 set ATETXEBF=1 (Tx packet apply BF On)\n");
	MTWF_PRINT(" 5)  iwpriv ra0 set ATE=TXFRAMESKB\n");
	MTWF_PRINT(" 6)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)\n");
	MTWF_PRINT(" 7)  check IQxel waveform\n");
	MTWF_PRINT(" 8)  iwpriv ra0 set ATETXEBF=0 (Tx packet apply BF Off)\n");
	MTWF_PRINT(" 9)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)\n");
	MTWF_PRINT("10)  check Iqxel waveform\n");
	MTWF_PRINT("---------------------------------------------------------------------------------------------\n");
	MTWF_PRINT("For VHT80 mode\n");
	MTWF_PRINT("\n");
	MTWF_PRINT(" 1)  iwpriv ra0 set ATEEBFCE=1\n");
	MTWF_PRINT(" 2)  iwpriv ra0 set ATEConTxETxBfGdProc=04:00:02:036:112:1 (Use in Golden Device)\n");
	MTWF_PRINT(" 3)  iwpriv ra0 set ATEConTxETxBfInitProc=04:00:02:01:04:18:036:112:1:16000\n");
	MTWF_PRINT(" 4)  iwpriv ra0 set ATETXEBF=1 (Tx packet apply BF On)\n");
	MTWF_PRINT(" 5)  iwpriv ra0 set ATE=TXFRAMESKB\n");
	MTWF_PRINT(" 6)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)\n");
	MTWF_PRINT(" 7)  check IQxel waveform\n");
	MTWF_PRINT(" 8)  iwpriv ra0 set ATETXEBF=0 (Tx packet apply BF Off)\n");
	MTWF_PRINT(" 9)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)\n");
	MTWF_PRINT("10)  check Iqxel waveform\n");
	MTWF_PRINT("---------------------------------------------------------------------------------------------\n");
	MTWF_PRINT("For VHT160C mode\n");
	MTWF_PRINT("\n");
	MTWF_PRINT(" 1)  iwpriv ra0 set ATEEBFCE=1\n");
	MTWF_PRINT(" 2)  iwpriv ra0 set ATEConTxETxBfGdProc=04:00:06:036:112:1 (Use in Golden Device)\n");
	MTWF_PRINT(" 3)  iwpriv ra0 set ATEConTxETxBfInitProc=04:00:03:01:04:18:036:112:1:16000\n");
	MTWF_PRINT(" 4)  iwpriv ra0 set ATETXEBF=1 (Tx packet apply BF On)\n");
	MTWF_PRINT(" 5)  iwpriv ra0 set ATE=TXFRAMESKB\n");
	MTWF_PRINT(" 6)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)\n");
	MTWF_PRINT(" 7)  check IQxel waveform\n");
	MTWF_PRINT(" 8)  iwpriv ra0 set ATETXEBF=0 (Tx packet apply BF Off)\n");
	MTWF_PRINT(" 9)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)\n");
	MTWF_PRINT("10)  check Iqxel waveform\n");
	MTWF_PRINT("---------------------------------------------------------------------------------------------\n");
	MTWF_PRINT("For VHT160NC mode\n");
	MTWF_PRINT("\n");
	MTWF_PRINT(" 1)  iwpriv ra0 set ATEEBFCE=1\n");
	MTWF_PRINT(" 2)  iwpriv ra0 set ATEConTxETxBfGdProc=04:00:06:036:112:1 (Use in Golden Device)\n");
	MTWF_PRINT(" 3)  iwpriv ra0 set ATEConTxETxBfInitProc=04:00:06:01:04:18:036:112:1:16000\n");
	MTWF_PRINT(" 4)  iwpriv ra0 set ATETXEBF=1 (Tx packet apply BF On)\n");
	MTWF_PRINT(" 5)  iwpriv ra0 set ATE=TXFRAMESKB\n");
	MTWF_PRINT(" 6)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)\n");
	MTWF_PRINT(" 7)  check IQxel waveform\n");
	MTWF_PRINT(" 8)  iwpriv ra0 set ATETXEBF=0 (Tx packet apply BF Off)\n");
	MTWF_PRINT(" 9)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)\n");
	MTWF_PRINT("10)  check Iqxel waveform\n");
	MTWF_PRINT("---------------------------------------------------------------------------------------------\n");
	MTWF_PRINT("For DBDC Band1 HT20 mode\n");
	MTWF_PRINT("\n");
	MTWF_PRINT(" 1)  configure DBDC mode and Reboot system\n");
	MTWF_PRINT(" 2)  iwpriv ra1 set ATEEBFCE=1\n");
	MTWF_PRINT(" 3)  iwpriv ra1 set ATEConTxETxBfGdProc=02:00:00:36:112:1 (Use in Golden Device)\n");
	MTWF_PRINT(" 4)  iwpriv ra1 set ATEConTxETxBfInitProc=02:00:00:01:02:18:36:112:1:04000\n");
	MTWF_PRINT(" 5)  iwpriv ra1 set ATETXEBF=1 (Tx packet apply BF On)\n");
	MTWF_PRINT(" 6)  iwpriv ra1 set ATE=TXFRAMESKB\n");
	MTWF_PRINT(" 7)  iwpriv ra1 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)\n");
	MTWF_PRINT(" 8)  check IQxel waveform\n");
	MTWF_PRINT(" 9)  iwpriv ra1 set ATETXEBF=0 (Tx packet apply BF Off)\n");
	MTWF_PRINT("10)  iwpriv ra1 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)\n");
	MTWF_PRINT("11)  check Iqxel waveform\n");
	MTWF_PRINT("=============================================================================================\n");
	MTWF_PRINT("                           Method for Dynamical Control Tx Power\n");
	MTWF_PRINT("=============================================================================================\n");
	MTWF_PRINT(" 1)  Follow ETxBF Certification Procedure to enable TxBf packet at first\n");
	MTWF_PRINT(" 2)  Use command \"iwpriv ra0 set ATE=TXSTOP\" to stop Tx\n");
	MTWF_PRINT(" 3)  Use command \"iwpriv ra0 set ATETXPOW0=XX\" to configure Tx Power DAC value for OFDM 54M\n");
	MTWF_PRINT(" 4)  USe command \"ra0 set ATE=TXFRAMESKB\" to start continuous packet Tx\n");
	MTWF_PRINT("=============================================================================================\n");


	return status;
}


#endif /* TXBF_SUPPORT && MT_MAC */


INT32 SetATEHelp(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{

	MTWF_PRINT("ATE=ATESTART, ATESTOP, TXCONT, TXCARR, TXCARS, TXFRAME, RXFRAME\n");
	MTWF_PRINT("ATEDA\n");
	MTWF_PRINT("ATESA\n");
	MTWF_PRINT("ATEBSSID\n");
	MTWF_PRINT("ATECHANNEL, range:0~14\n");
	MTWF_PRINT("ATETXPOW0, set power level of antenna 1.\n");
	MTWF_PRINT("ATETXPOW1, set power level of antenna 2.\n");
	MTWF_PRINT("ATETXANT, set TX antenna. 0:all, 1:antenna one, 2:antenna two.\n");
	MTWF_PRINT("ATERXANT, set RX antenna.0:all, 1:antenna one, 2:antenna two, 3:antenna three.\n");
	MTWF_PRINT("ATETXBW, set BandWidth, 0:20MHz, 1:40MHz\n");
	MTWF_PRINT("ATETXLEN, set Frame length, range 24~%d\n", (MAX_FRAME_SIZE - 34/* == 2312 */));
	MTWF_PRINT("ATETXCNT, set how many frame going to transmit.\n");
	MTWF_PRINT("ATETXMCS, set MCS, reference to rate table.\n");
	MTWF_PRINT("ATETXMODE, set Mode 0:CCK, 1:OFDM, 2:HT-Mix, 3:GreenField, 4:VHT, reference to rate table.\n");
	MTWF_PRINT("ATETXGI, set GI interval, 0:Long, 1:Short\n");
	MTWF_PRINT("ATERXFER, 0:disable Rx Frame error rate. 1:enable Rx Frame error rate.\n");
	MTWF_PRINT("ATERRF, show all RF registers.\n");
	MTWF_PRINT("ATELDE2P, load EEPROM from .bin file.\n");
	MTWF_PRINT("ATERE2P, display all EEPROM content.\n");
	MTWF_PRINT("ATEAUTOALC, enable ATE auto Tx alc (Tx auto level control).\n");
	MTWF_PRINT("ATEIPG, set ATE Tx frame IPG.\n");
	MTWF_PRINT("ATEPAYLOAD, set ATE payload pattern for TxFrame.\n");
	MTWF_PRINT("ATESHOW, display all parameters of ATE.\n");
	MTWF_PRINT("ATEHELP, online help.\n");

	return TRUE;
}


INT32 ATESampleRssi(PRTMP_ADAPTER pAd, RXWI_STRUC *pRxWI)
{
	return TRUE;
}


#ifdef RTMP_PCI_SUPPORT
PNDIS_PACKET ATEPayloadInit(RTMP_ADAPTER *pAd, UINT32 TxIdx)
{
	return NULL;
}
#endif /* RTMP_MAC_PCI */

#ifdef RTMP_PCI_SUPPORT
INT32 ATEPayloadAlloc(PRTMP_ADAPTER pAd, UINT32 Index)
{
	return NDIS_STATUS_SUCCESS;
}
#endif /* RTMP_MAC_PCI */

INT32 ATEInit(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	NdisZeroMemory(ATECtrl, sizeof(*ATECtrl));
#ifdef DBDC_MODE
	NdisZeroMemory(&ATECtrl->band_ext[0], sizeof(struct _BAND_INFO));
#endif /* DBDC_MODE */
	/* restore wdev */
	ATECtrl->op_mode  = ATE_STOP;
	ATECtrl->tx_cnt = 0xFFFFFFFF;
	ATECtrl->payload[0] = 0xAA;
	ATECtrl->fixed_payload = 1;
	ATECtrl->tx_len = 1058;/* 1058 : sync with QA */
	ATECtrl->bw = BW_20;
	ATECtrl->tx_mode = MODE_OFDM;
	ATECtrl->mcs = 7;
	ATECtrl->sgi = 0;/* LONG GI : 800 ns*/
	if (BOARD_IS_5G_ONLY(pAd))
		ATECtrl->channel = 36;
	else
		ATECtrl->channel = 1;
	ATECtrl->tx_ant = 1;
	ATECtrl->rx_ant = 0;
	ATECtrl->ac_idx = QID_AC_BE;
	ATECtrl->addr1[0][0] = 0x00;
	ATECtrl->addr1[0][1] = 0x11;
	ATECtrl->addr1[0][2] = 0x22;
	ATECtrl->addr1[0][3] = 0xAA;
	ATECtrl->addr1[0][4] = 0xBB;
	ATECtrl->addr1[0][5] = 0xCC;
	NdisMoveMemory(ATECtrl->addr2[0], ATECtrl->addr1[0], MAC_ADDR_LEN);
	NdisMoveMemory(ATECtrl->addr3[0], ATECtrl->addr1[0], MAC_ADDR_LEN);
	ATECtrl->bQAEnabled = FALSE;
	ATECtrl->bQATxStart = FALSE;
	ATECtrl->bQARxStart = FALSE;
	ATECtrl->duty_cycle = 0;
	ATECtrl->tx_time_param.pkt_tx_time_en = FALSE;
	ATECtrl->tx_time_param.pkt_tx_time = 0;
	ATECtrl->ipg_param.ipg = 0;
	ATECtrl->ipg_param.sig_ext = SIG_EXTENSION;
	ATECtrl->ipg_param.slot_time = DEFAULT_SLOT_TIME;
	ATECtrl->ipg_param.sifs_time = DEFAULT_SIFS_TIME;
	ATECtrl->ipg_param.ac_num = QID_AC_BE;
	ATECtrl->ipg_param.aifsn = MIN_AIFSN;
	ATECtrl->ipg_param.cw = MIN_CW;
	ATECtrl->ipg_param.txop = 0;
	ATECtrl->control_band_idx = 0;	/* Control band0 as default setting */
	/* Assign wdev_idx */
	ATECtrl->wdev_idx = 0;
	ATECtrl->wmm_idx = 0; /* Need to modify after j mode implement done */
#ifdef DBDC_MODE
	ATECtrl->band_ext[0].op_mode = ATE_STOP;
	if (ATECtrl->use_apcli == 0)
	{
		{
		ATECtrl->band_ext[0].wdev_idx = 1;
		}
	}
	else
		ATECtrl->band_ext[0].wdev_idx = 3;
	ATECtrl->band_ext[0].wmm_idx = 1;
#endif /* DBDC_MODE */
#ifdef TXBF_SUPPORT
	ATECtrl->ebf = FALSE;
	ATECtrl->ibf = FALSE;
#endif
	os_move_mem(&ATECtrl->template_frame, TemplateFrame, sizeof(TemplateFrame));
#ifdef MT_MAC
	Ret = MT_ATEInit(pAd);
#endif
#ifdef CONFIG_QA
	ATECtrl->TxStatus = 0;
#endif
	return Ret;
}

INT32 ATEExit(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;

#ifdef MT_MAC
	Ret = MT_ATEExit(pAd);
#endif
	return Ret;
}

VOID  ATEPeriodicExec(PVOID SystemSpecific1, PVOID FunctionContext,
					  PVOID SystemSpecific2, PVOID SystemSpecific3)
{
}

INT32 SetATE(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT32 Ret = 0;

#ifdef SPECIAL_11B_OBW_FEATURE
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	struct wifi_dev *wdev = &pMbss->wdev;
#endif

	MTWF_PRINT("%s: Arg = %s\n", __func__, Arg);


#if defined(CONFIG_WLAN_SERVICE)
	Ret = mt_agent_cli_act(Arg, &pAd->serv);

	if (Ret)
		goto err1;
#else
	{
		struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
		struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
		UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
		UINT32 mode = 0;

		mode = TESTMODE_GET_PARAM(pAd, control_band_idx, op_mode);

		if (!strcmp(Arg, "ATESTART") && (mode != ATE_START)) { /* support restart w/o ATESTOP */
			if (mode & fATE_TXCONT_ENABLE) {
				/* TODO Get Correct TxfdMode*/
				UINT32 TxfdMode = 1;

				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Stop Continuous Tx\n");
				Ret += ATEOp->StopContinousTx(pAd, TxfdMode);
			}

			if (mode & fATE_TXCARRSUPP_ENABLE) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Stop Carrier Suppression Test\n");
				Ret += ATEOp->StopTxTone(pAd);
			}

			/* MT76x6 Test Mode Freqency offset restore*/
			if (ATECtrl->en_man_set_freq) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"MT76x6 Manual Set Frequency Restore\n");
				MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_ENABLE);
				MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_VALUE);
				ATECtrl->en_man_set_freq = 0;
			}

			if (Ret)
				goto err1;

#if defined(MT_MAC)
#ifdef TXBF_SUPPORT
			/* Before going into ATE mode, stop sounding first */
			mt_Trigger_Sounding_Packet(pAd,
									   FALSE,
									   0,
									   0,
									   0,
									   NULL);
#endif /* TXBF_SUPPORT */
#endif /* MAC */
			Ret = ATEOp->ATEStart(pAd);
		} else if (!strcmp(Arg, "ATESTOP") && (mode & ATE_START))
			Ret = ATEOp->ATEStop(pAd);
		else if (!strcmp(Arg, "TRXENABLE") && (mode & ATE_START))
			MtATESetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE, control_band_idx);
		else if (!strcmp(Arg, "TRXDISABLE") && (mode & ATE_START))
			MtATESetMacTxRx(pAd, ASIC_MAC_TXRX, FALSE, control_band_idx);
		else if (!strcmp(Arg, "TXSTREAM") && (mode & ATE_START))
			MtATESetTxStream(pAd, 3, control_band_idx);
		else if (!strcmp(Arg, "RXSTREAM") && (mode & ATE_START))
			MtATESetRxPath(pAd, 1, control_band_idx);
		else if (!strcmp(Arg, "APSTOP") && (mode == ATE_STOP))
			Ret = ATEOp->ATEStart(pAd);
		else if (!strcmp(Arg, "APSTART") && (mode & ATE_START))
			Ret = ATEOp->ATEStop(pAd);
		else if (!strcmp(Arg, "TXCOMMIT") && (mode & ATE_START))
			Ret = ATEOp->tx_commit(pAd);
		else if (!strcmp(Arg, "TXREVERT") && (mode & ATE_START))
			Ret = ATEOp->tx_revert(pAd);
		else if (!strcmp(Arg, "TXFRAME") && (mode & ATE_START))
			Ret = ATEOp->StartTx(pAd);

#if defined(TXBF_SUPPORT) && defined(MT_MAC)
		else if (!strcmp(Arg, "TXFRAMESKB") && (mode & ATE_START))
			Ret = ATEOp->StartTxSKB(pAd);

#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */
		else if (!strcmp(Arg, "RXFRAME") && (mode & ATE_START))
			Ret = ATEOp->StartRx(pAd);
		else if (!strcmp(Arg, "TXSTOP") && (mode & ATE_START))
			Ret = ATEOp->StopTx(pAd);
		else if (!strcmp(Arg, "RXSTOP") && (mode & ATE_START))
			Ret = ATEOp->StopRx(pAd);
		else if (!strcmp(Arg, "TXCONT") && (mode & ATE_START)) {
			/* 0: All 1:TX0 2:TX1 */
			/* TODO: Correct band selection */
			UINT32 ant = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_ant);
			/* TODO Get Correct TxfdMode*/
			UINT32 TxfdMode = 3; /* continuous payload OFDM/CCK */
			Ret = ATEOp->StartContinousTx(pAd, ant, TxfdMode);
			mode |= ATE_TXCONT;
			TESTMODE_SET_PARAM(pAd, control_band_idx, op_mode, mode);
		} else if (!strcmp(Arg, "TXCONTSTOP") && (mode & ATE_START)) {
			if (mode & fATE_TXCONT_ENABLE) {
				/* TODO Get Correct TxfdMode*/
				UINT32 TxfdMode = 3;

				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Stop Continuous Tx\n");
				ATEOp->StopContinousTx(pAd, TxfdMode);
			}

			mode &= ~ATE_TXCONT;
			TESTMODE_SET_PARAM(pAd, control_band_idx, op_mode, mode);
		} else if (!strcmp(Arg, "TXCARRSUPP") && (mode & ATE_START)) {
			INT32 pwr1 = 0xf;
			INT32 pwr2 = 0;
			UINT32 ant = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_ant);

			/* 0: All 1:TX0 2:TX1 */
			switch (ant) {
			case 0:
				MTWF_PRINT("%s: not support two 2 TXCARR\n", __func__);
				break;

			case 1:
				if (ATECtrl->TxPower0 > 30)
					pwr2 = (ATECtrl->TxPower0 - 30) << 1;
				else {
					pwr1 = (ATECtrl->TxPower0 & 0x1e) >> 1;
					pwr2 = (ATECtrl->TxPower0 & 0x01) << 1;
				}

				ATEOp->StartTxTone(pAd, WF0_TX_TWO_TONE_5M);
				break;

			case 2:
				if (ATECtrl->TxPower1 > 30)
					pwr2 = (ATECtrl->TxPower1 - 30) << 1;
				else {
					pwr1 = (ATECtrl->TxPower1 & 0x1e) >> 1;
					pwr2 = (ATECtrl->TxPower1 & 0x01) << 1;
				}

				ATEOp->StartTxTone(pAd, WF1_TX_TWO_TONE_5M);
				break;
			}

			ATEOp->SetTxTonePower(pAd, pwr1, pwr2);
			mode |= ATE_TXCARRSUPP;
			TESTMODE_SET_PARAM(pAd, control_band_idx, op_mode, mode);
		} else if (!strcmp(Arg, "TXCARR") && (mode & ATE_START)) {
			UINT32 ant = TESTMODE_GET_PARAM(pAd, control_band_idx, tx_ant);
			INT32 pwr1 = 0xf;
			INT32 pwr2 = 0;

			if (IS_MT7626(pAd)) {
				UINT32 Control = 1, ToneType = 0, ToneFreq = 0, DcOffset_I = 10, DcOffset_Q = 10, Band;

				Band = (control_band_idx ==  0) ? 24070000 : 50000000;

				if (ATECtrl->TxPower0 >= -32 && ATECtrl->TxPower0 <= 31) {
					if (ATEOp->SetDBDCTxTonePower)
						ATEOp->SetDBDCTxTonePower(pAd, 0, ATECtrl->TxPower0, ant);
				}

				if (ATEOp->DBDCTxTone)
					ATEOp->DBDCTxTone(pAd, Control, ant, ToneType, ToneFreq, DcOffset_I, DcOffset_Q, Band);
			} else {
				switch (ant) {
				case 0:
					MTWF_PRINT("%s: not support two 2 TXCARR\n", __func__);
					break;

				case 1:
					if (ATECtrl->TxPower0 > 30)
						pwr2 = (ATECtrl->TxPower0 - 30) << 1;
					else {
						pwr1 = (ATECtrl->TxPower0 & 0x1e) >> 1;
						pwr2 = (ATECtrl->TxPower0 & 0x01) << 1;
					}

					ATEOp->StartTxTone(pAd, WF0_TX_ONE_TONE_DC);
					break;

				case 2:
					if (ATECtrl->TxPower1 > 30)
						pwr2 = (ATECtrl->TxPower1 - 30) << 1;
					else {
						pwr1 = (ATECtrl->TxPower1 & 0x1e) >> 1;
						pwr2 = (ATECtrl->TxPower1 & 0x01) << 1;
					}

					ATEOp->StartTxTone(pAd, WF1_TX_ONE_TONE_DC);
					break;
				}

				ATEOp->SetTxTonePower(pAd, pwr1, pwr2);
			}

			mode |= ATE_TXCARR;
			TESTMODE_SET_PARAM(pAd, control_band_idx, op_mode, mode);
		}

#ifdef TXBF_SUPPORT
		else if (!strcmp(Arg, "MUENABLE")) {
			ATECtrl->mu_enable = TRUE;
			ATECtrl->mu_usrs = 4;
			TESTMODE_SET_PARAM(pAd, 0, ebf, 1);
		} else if (!strcmp(Arg, "MUDISABLE")) {
			ATECtrl->mu_enable = FALSE;
			ATECtrl->mu_usrs = 0;
			TESTMODE_SET_PARAM(pAd, 0, ebf, 0);
			AsicSetMacTxRx(pAd, ASIC_MAC_TX, TRUE);
		} else if (!strcmp(Arg, "BFENABLE"))
			TESTMODE_SET_PARAM(pAd, 0, ebf, 1);
		else if (!strcmp(Arg, "BFDISABLE"))
			TESTMODE_SET_PARAM(pAd, 0, ebf, 0);

#endif
#ifdef PRE_CAL_MT7622_SUPPORT
		else if (!strcmp(Arg, "TXDPD7622") && (mode & ATE_START)) {
			if (IS_MT7622(pAd)) {
			Ret = ATEOp->TxDPDTest7622(pAd, "0");
			Ret = 0;
			}
		}
#endif /*PRE_CAL_MT7622_SUPPORT*/
#ifdef PRE_CAL_MT7626_SUPPORT
		else if (!strcmp(Arg, "7626PREKClean") && (mode & ATE_START)) {
			Ret = ATEOp->PreCal7626(pAd, "0");
			Ret = 0;
		} else if (!strcmp(Arg, "7626PREK") && (mode & ATE_START)) {
			Ret = ATEOp->PreCal7626(pAd, "1");
			Ret = 0;
		} else if (!strcmp(Arg, "7626DPDClean") && (mode & ATE_START)) {
			Ret = ATEOp->TxDPD7626(pAd, "0");
			Ret = 0;
		} else if (!strcmp(Arg, "7626DPD5G") && (mode & ATE_START)) {
			Ret = ATEOp->TxDPD7626(pAd, "1");
			Ret = 0;
		} else if (!strcmp(Arg, "7626DPD2G") && (mode & ATE_START)) {
			Ret = ATEOp->TxDPD7626(pAd, "2");
			Ret = 0;
		}
#endif /* PRE_CAL_MT7626_SUPPORT */
#ifdef PRE_CAL_TRX_SET1_SUPPORT
		else if (!strcmp(Arg, "RX2GSELFTEST") && (mode & ATE_START)) {
			Ret = ATEOp->RxSelfTest(pAd, "0");
			Ret = 0;
		} else if (!strcmp(Arg, "RX5GSELFTEST") && (mode & ATE_START)) {
			Ret = ATEOp->RxSelfTest(pAd, "1");
			Ret = 0;
		} else if (!strcmp(Arg, "RXSELFTEST") && (mode & ATE_START)) {
			Ret = ATEOp->RxSelfTest(pAd, "2");
			Ret = 0;
		} else if (!strcmp(Arg, "TX2GDPD") && (mode & ATE_START)) {
			Ret = ATEOp->TxDPDTest(pAd, "0");
			Ret = 0;
		} else if (!strcmp(Arg, "TX5GDPD") && (mode & ATE_START)) {
			Ret = ATEOp->TxDPDTest(pAd, "1");
			Ret = 0;
		} else if (!strcmp(Arg, "TXDPD") && (mode & ATE_START)) {
			Ret = ATEOp->TxDPDTest(pAd, "2");
			Ret = 0;
		}
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
#ifdef PRE_CAL_TRX_SET2_SUPPORT
		else if ((strcmp(Arg, "PRECAL") > 0) && (mode & ATE_START)) {
			UINT32 ChGrpId = 0;

			Ret = sscanf(Arg + strlen("PRECAL") + 1, "%d", &ChGrpId);
			if (Ret == 1)
				MTWF_PRINT("%s: ChGrpId %d\n", __func__, ChGrpId);

			Ret = ATEOp->PreCalTest(pAd, 0, ChGrpId);
			Ret = 0;
		} else if ((strcmp(Arg, "PRECALTX") > 0) && (mode & ATE_START)) {
			UINT32 ChGrpId = 0;

			Ret = sscanf(Arg + strlen("PRECALTX") + 1, "%d", &ChGrpId);
			if (Ret == 1)
				MTWF_PRINT("%s: ChGrpId %d\n", __func__, ChGrpId);
			Ret = ATEOp->PreCalTest(pAd, 1, ChGrpId);
			Ret = 0;
		}
#endif /* PRE_CAL_TRX_SET2_SUPPORT */
#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
		else if ((strcmp(Arg, "PATRIM") > 0) && (mode & ATE_START)) {
			if (IS_MT7615(pAd)) {
				INT32 i;
				UINT32 Data[4] = {0};
				RTMP_STRING *value = NULL;

				for (i = 0, value = rstrtok(Arg + 7, "-"); value; value = rstrtok(NULL, "-"), i++) {
					Data[i] = simple_strtol(value, 0, 16);
					MTWF_PRINT("\x1b[32m%s: WF%d = 0x%08x \x1b[m\n", __func__, i, Data[i]);
				}
				Ret = ATEOp->PATrim(pAd, &Data[0]);
				Ret = 0;
			}
		}
#endif /* CAL_BIN_FILE_SUPPORT */
#ifdef DBDC_MODE
		else if (!strcmp(Arg, "ATEAPCLI")) {
			if (mode & fATE_EXIT) {
				ATECtrl->use_apcli = 1;
				ATECtrl->band_ext[0].wdev_idx = 1;
				ATECtrl->band_ext[0].wmm_idx = 1;
				MTWF_PRINT("%s: (param = (%s), mode = (%d)), ATECtrl->use_apcli = %d\n",
					__func__, Arg, TESTMODE_GET_PARAM(pAd, TESTMODE_BAND0, op_mode), ATECtrl->use_apcli);
			} else {
				MTWF_PRINT("%s: [Warning](param = (%s), mode = (%d)), ATECtrl->use_apcli = %d\n",
					__func__, Arg, TESTMODE_GET_PARAM(pAd, TESTMODE_BAND0, op_mode), ATECtrl->use_apcli);
			}
		} else if (!strcmp(Arg, "ATEAP")) {
			if (mode & fATE_EXIT) {
				ATECtrl->use_apcli = 0;
				ATECtrl->band_ext[0].wdev_idx = 1;
				ATECtrl->band_ext[0].wmm_idx = 1;
				MTWF_PRINT("%s: (param = (%s), mode = (%d)), ATECtrl->use_apcli = %d\n",
					__func__, Arg, TESTMODE_GET_PARAM(pAd, TESTMODE_BAND0, op_mode), ATECtrl->use_apcli);
			} else {
				MTWF_PRINT("%s: [Warning](param = (%s), mode = (%d)), ATECtrl->use_apcli = %d\n",
					__func__, Arg, TESTMODE_GET_PARAM(pAd, TESTMODE_BAND0, op_mode), ATECtrl->use_apcli);
			}
		}
#endif
		else if (!strcmp(Arg, "showTXV") && (mode & ATE_START)) {
			struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

			if (chip_dbg->show_txv_info)
				Ret = chip_dbg->show_txv_info(pAd->hdev_ctrl, NULL);
			else
				Ret = -1;
		}
#if defined(DOT11_HE_AX)
		else if (!strcmp(Arg, "showRUInfo") && (mode & ATE_START)) {
			if (ATEOp->show_ru_info)
				Ret = ATEOp->show_ru_info(pAd, TESTMODE_GET_BAND_IDX(pAd));
		}
#endif
		else if (!strcmp(Arg, "checkTXV") && (mode & ATE_START)) {
			struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);
			struct _MAC_TABLE_ENTRY_STACK *mac_tbl_entry_stack = TESTMODE_GET_PADDR(pAd, TESTMODE_GET_BAND_IDX(pAd), stack);
			struct _MAC_TABLE_ENTRY *entry = (struct _MAC_TABLE_ENTRY *)mac_tbl_entry_stack->mac_tbl_entry[0];

			if (chip_dbg->check_txv && entry) {
				UINT8 phy_mode = entry->phy_param.phy_mode;
				UINT8 ofdm_rate[8] = {0xb, 0xf, 0xa, 0xe, 0x9, 0xd, 0x8, 0xc};

				Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "spe idx", entry->phy_param.spe_idx, control_band_idx);
				Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "phy mode", entry->phy_param.phy_mode, control_band_idx);
				Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "dbw", entry->phy_param.bw, control_band_idx);
				Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "ER-106T", entry->phy_param.su_ext_tone, control_band_idx);
				Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "DCM", entry->phy_param.dcm, control_band_idx);
				if (phy_mode == MODE_CCK) {
					if (entry->phy_param.rate > MCS_8)
						Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "Rate", entry->phy_param.rate+5, control_band_idx);
					else
						Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "Rate", entry->phy_param.rate, control_band_idx);
				} else if (phy_mode == MODE_OFDM) {
					Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "Rate", ofdm_rate[entry->phy_param.rate], control_band_idx);
				} else
					Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "Rate", entry->phy_param.rate, control_band_idx);
				if (phy_mode < MODE_HTMIX)
					Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "NSTS", 0);
				else if (phy_mode < MODE_VHT)
					Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "NSTS", (entry->phy_param.rate >> 3), control_band_idx);
				else
					Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "NSTS", entry->phy_param.vht_nss-1, control_band_idx);
				Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "HE LTF", entry->phy_param.ltf_type, control_band_idx);
				Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "HE GI", entry->phy_param.gi_type, control_band_idx);
				Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "stbc", entry->phy_param.stbc, control_band_idx);
				Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "FEC Coding", entry->phy_param.ldpc, control_band_idx);
				if (entry->phy_param.phy_mode > MODE_VHT)
					Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "Max TPE", TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), max_pkt_ext), control_band_idx);
				else
					Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "Max TPE", 0, control_band_idx);
				Ret = chip_dbg->check_txv(pAd->hdev_ctrl, "TX LEN",
										  TESTMODE_GET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), tx_len)+((phy_mode > MODE_OFDM) ? 8 : 4), control_band_idx);	/* 4 bytes of FCS and 4 bytes of AMPDU delimiter*/
			}
		}
#if defined(MT7986)
		else if (!strcmp(Arg, "DNLKCLEAN") && (mode & ATE_START)) {
			Ret = ATEOp->DnlK7986(pAd, "0");
			Ret = 0;
		} else if (!strcmp(Arg, "DNLK2G") && (mode & ATE_START)) {
			Ret = ATEOp->DnlK7986(pAd, "1");
			Ret = 0;
		} else if (!strcmp(Arg, "DNLK5G") && (mode & ATE_START)) {
			Ret = ATEOp->DnlK7986(pAd, "2");
			Ret = 0;
		} else if (!strcmp(Arg, "RXGAINCAL") && (mode & ATE_START)) {
			Ret = ATEOp->RXGAINK7986(pAd, "1");
			Ret = 0;
		}
#endif
#if defined(MT7916)
		else if (!strcmp(Arg, "DNLKCLEAN") && (mode & ATE_START)) {
			Ret = ATEOp->DnlK7916(pAd, "0");
			Ret = 0;
		} else if (!strcmp(Arg, "DNLK2G") && (mode & ATE_START)) {
			Ret = ATEOp->DnlK7916(pAd, "1");
			Ret = 0;
		} else if (!strcmp(Arg, "DNLK5G") && (mode & ATE_START)) {
			Ret = ATEOp->DnlK7916(pAd, "2");
			Ret = 0;
		} else if (!strcmp(Arg, "RXGAINCAL") && (mode & ATE_START)) {
			Ret = ATEOp->RXGAINK7916(pAd, "1");
			Ret = 0;
		}
#endif
#if defined(MT7981)
		else if (!strcmp(Arg, "DNLKCLEAN") && (mode & ATE_START)) {
			Ret = ATEOp->DnlK7981(pAd, "0");
			Ret = 0;
		} else if (!strcmp(Arg, "DNLK2G") && (mode & ATE_START)) {
			Ret = ATEOp->DnlK7981(pAd, "1");
			Ret = 0;
		} else if (!strcmp(Arg, "DNLK5G") && (mode & ATE_START)) {
			Ret = ATEOp->DnlK7981(pAd, "2");
			Ret = 0;
		} else if (!strcmp(Arg, "RXGAINCAL") && (mode & ATE_START)) {
			Ret = ATEOp->RXGAINK7981(pAd, "1");
			Ret = 0;
		}
#endif
		else {
			MTWF_PRINT("%s: do nothing(param = (%s), mode = (%d))\n",
				__func__, Arg, TESTMODE_GET_PARAM(pAd, TESTMODE_BAND0, op_mode));
		}
	}
#endif	/* CONFIG_WLAN_SERVICE */

#ifdef SPECIAL_11B_OBW_FEATURE
	if (wdev->channel >= 1 && wdev->channel <= 14) {
		MtCmdSetTxTdCck(pAd, TRUE);
	}
#endif

	if (!Ret)
		return TRUE;

err1:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"- RF-test stop fail, ret:%d\n", Ret);
	return FALSE;
}


INT32 SetATEChannel(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_set_ext("ATECHANNEL", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	INT32 Ret = 0;
	const INT idx_num = 4;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT32 param[idx_num];
	INT i = 0;
	CHAR *value;

	MTWF_PRINT("%s: control_band_idx:%x, Channel = %s\n",
		__func__, control_band_idx, Arg);

	for (i = 0; i < idx_num; i++)
		param[i] = 0;

	for (i = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":")) {
		if (i == idx_num)
			break;

		param[i++] = simple_strtol(value, 0, 10);
	}

	/* For backward compatibility */
	if (param[0] >= 36 && param[0] <= 181) {
		if (param[1] == 0) {
			param[1] = 1; /* channel_band 5G as 1*/
			MTWF_PRINT("\x1b[41m%s(): 5G Channel:%d, then must be Channel_Band:%d !!\x1b[m\n",
				__func__, param[0], param[1]);
		}
	}

	TESTMODE_SET_PARAM(pAd, control_band_idx, channel, param[0]);
	TESTMODE_SET_PARAM(pAd, control_band_idx, ch_band, param[1]);
#ifdef DOT11_VHT_AC
	TESTMODE_SET_PARAM(pAd, control_band_idx, channel_2nd, param[3]);
#endif
	Ret = ATEOp->SetChannel(pAd, param[0], param[2], 0, param[1]);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
#endif
}


#ifdef DOT11_VHT_AC
INT32 set_ate_channel_ext(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
#define ATE_SET_CH_EXT_PARAM_CNT    8
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
	struct _HQA_EXT_SET_CH param;
	INT32 ret = 0;
	UINT32 len = 0;
	UINT32 pri_ch = 0;
	UINT32 band_idx = 0;
	UINT32 bw = 0;
	UINT32 per_pkt_bw = 0;
	INT i = 0;
	CHAR *value;
	UINT32 data[ATE_SET_CH_EXT_PARAM_CNT] = {0};

	len = strlen(arg);
	MTWF_PRINT("%s: Arg = %s\n", __func__, arg);

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {

		if (i == ATE_SET_CH_EXT_PARAM_CNT)
			break;

		data[i] = simple_strtol(value, 0, 10);
		i++;
	}

	if (i == ATE_SET_CH_EXT_PARAM_CNT) {
		param.band_idx      = data[0];
		param.central_ch0   = data[1];
		param.central_ch1   = data[2];
		param.sys_bw        = data[3];
		param.perpkt_bw     = data[4];
		param.pri_sel       = data[5];
		param.reason        = data[6];
		param.ch_band       = data[7];
	} else
		return ret;

	if (param.band_idx > TESTMODE_BAND_NUM-1) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto err0;
	}

	band_idx = param.band_idx;

	switch (param.sys_bw) {
	case ATE_BAND_WIDTH_20:
		bw = BAND_WIDTH_20;
		break;

	case ATE_BAND_WIDTH_40:
		bw = BAND_WIDTH_40;
		break;

	case ATE_BAND_WIDTH_80:
		bw = BAND_WIDTH_80;
		break;

	case ATE_BAND_WIDTH_10:
		bw = BAND_WIDTH_10;
		break;

	case ATE_BAND_WIDTH_5:
		bw = BAND_WIDTH_5;
		break;

	case ATE_BAND_WIDTH_160:
		bw = BAND_WIDTH_160;
		break;

	case ATE_BAND_WIDTH_8080:
		bw = BAND_WIDTH_8080;
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"Cannot find BW with param.sys_bw:%x\n", param.sys_bw);
		bw = param.sys_bw;
		break;
	}

	switch (param.perpkt_bw) {
	case ATE_BAND_WIDTH_20:
		per_pkt_bw = BAND_WIDTH_20;
		break;

	case ATE_BAND_WIDTH_40:
		per_pkt_bw = BAND_WIDTH_40;
		break;

	case ATE_BAND_WIDTH_80:
		per_pkt_bw = BAND_WIDTH_80;
		break;

	case ATE_BAND_WIDTH_10:
		per_pkt_bw = BAND_WIDTH_10;
		break;

	case ATE_BAND_WIDTH_5:
		per_pkt_bw = BAND_WIDTH_5;
		break;

	case ATE_BAND_WIDTH_160:
	case ATE_BAND_WIDTH_8080:
		per_pkt_bw = BAND_WIDTH_160;
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"Cannot find BW with param.sys_bw:%x\n", param.sys_bw);
		per_pkt_bw = bw;
		break;
	}

	/* Set Param */
	TESTMODE_SET_PARAM(pAd, band_idx, channel, param.central_ch0);
#ifdef DOT11_VHT_AC
	TESTMODE_SET_PARAM(pAd, band_idx, channel_2nd, param.central_ch1);
#endif
	TESTMODE_SET_PARAM(pAd, band_idx, per_pkt_bw, per_pkt_bw);
	TESTMODE_SET_PARAM(pAd, band_idx, bw, bw);
	TESTMODE_SET_PARAM(pAd, band_idx, pri_sel, param.pri_sel);
	TESTMODE_SET_PARAM(pAd, band_idx, ch_band, param.ch_band);
	ret = ate_ops->SetChannel(pAd, param.central_ch0, param.pri_sel, param.reason, param.ch_band);

	if (ret == 0)
		ret = TRUE;

err0:
	MTWF_PRINT("%s: len:%x, band_idx:%x, ch0:%u, ch1:%u, sys_bw:%x, bw_conver:%x, ",
		__func__, len, param.band_idx,
		param.central_ch0, param.central_ch1, param.sys_bw, bw);
	MTWF_PRINT("perpkt_bw:%x, pri_sel:%x, pri_ch:%u\n", param.perpkt_bw, param.pri_sel, pri_ch);
	return ret;
}


INT32 set_ate_start_tx_ext(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
#define ATE_START_TX_EXT_PARAM_CNT  14
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
	struct _HQA_EXT_TXV param;
	ATE_TXPOWER TxPower;
	INT32 ret = 0;
	INT32 len = 0;
	UINT32 band_idx = 0;
	UINT32 Channel = 0, Ch_Band = 0, SysBw = 0, PktBw = 0, ipg = 0;
	INT i = 0;
	CHAR *value;
	UINT32 data[ATE_START_TX_EXT_PARAM_CNT] = {0};

	len = strlen(arg);
	MTWF_PRINT("%s: Arg = %s\n", __func__, arg);

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {

		if (i == ATE_START_TX_EXT_PARAM_CNT)
			break;

		data[i] = simple_strtol(value, 0, 10);
		i++;
	}

	if (i == ATE_START_TX_EXT_PARAM_CNT) {
		param.band_idx  = data[0];
		param.pkt_cnt   = data[1];
		param.tx_mode	= data[2];
		param.rate      = data[3];
		param.pwr       = data[4];
		param.stbc      = data[5];
		param.ldpc      = data[6];
		param.ibf       = data[7];
		param.ebf       = data[8];
		param.wlan_id   = data[9];
		param.aifs      = data[10];
		param.gi        = data[11];
		param.tx_path   = data[12];
		param.nss       = data[13];
	} else
		return ret;

	band_idx = param.band_idx;

	if (!param.pkt_cnt)
		param.pkt_cnt = 0x8fffffff;

	TESTMODE_SET_PARAM(pAd, band_idx, ATE_TX_CNT, param.pkt_cnt);
	TESTMODE_SET_PARAM(pAd, band_idx, tx_mode, param.tx_mode);
	TESTMODE_SET_PARAM(pAd, band_idx, mcs, param.rate);
	TESTMODE_SET_PARAM(pAd, band_idx, stbc, param.stbc);
	TESTMODE_SET_PARAM(pAd, band_idx, ldpc, param.ldpc);
#ifdef TXBF_SUPPORT
	TESTMODE_SET_PARAM(pAd, band_idx, ibf, param.ibf);
	TESTMODE_SET_PARAM(pAd, band_idx, ebf, param.ebf);
#endif
	ate_ctrl->wcid_ref = param.wlan_id;
	TESTMODE_SET_PARAM(pAd, band_idx, ipg_param.ipg, param.aifs);        /* Fix me */
	TESTMODE_SET_PARAM(pAd, band_idx, sgi, param.gi);
	TESTMODE_SET_PARAM(pAd, band_idx, tx_ant, param.tx_path);
	TESTMODE_SET_PARAM(pAd, band_idx, nss, param.nss);
	Channel = TESTMODE_GET_PARAM(pAd, band_idx, channel);
	Ch_Band = TESTMODE_GET_PARAM(pAd, band_idx, ch_band);
	PktBw = TESTMODE_GET_PARAM(pAd, band_idx, per_pkt_bw);
	SysBw = TESTMODE_GET_PARAM(pAd, band_idx, bw);
	ipg = TESTMODE_GET_PARAM(pAd, band_idx, ipg_param.ipg);

	if (param.tx_mode < MODE_HE && param.rate == 32 && PktBw != BAND_WIDTH_40 && SysBw != BAND_WIDTH_40) {
		ret = -1;
		MTWF_PRINT("%s: Bandwidth must to be 40 at MCS 32\n", __func__);
		goto err0;
	}

	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Power = param.pwr;
	TxPower.Channel = Channel;
	TxPower.Dbdc_idx = band_idx;
	TxPower.Band_idx = Ch_Band;
	ret = ate_ops->SetTxPower0(pAd, TxPower);
	ret = ate_ops->SetIPG(pAd, ipg);
	ret = ate_ops->tx_commit(pAd);
	ret = ate_ops->StartTx(pAd);

	if (ret == 0)
		ret = TRUE;

err0:
	MTWF_PRINT("%s: band_idx:%u, pkt_cnt:%u, phy:%u, mcs:%u, stbc:%u, ldpc:%u\n",
		__func__, param.band_idx, param.pkt_cnt, param.tx_mode, param.rate, param.stbc, param.ldpc);
	MTWF_PRINT("%s: ibf:%u, ebf:%u, wlan_id:%u, aifs:%u, gi:%u, tx_path:%x, nss:%x\n",
		__func__, param.ibf, param.ebf, param.wlan_id, param.aifs, param.gi, param.tx_path, param.nss);
	return ret;
}
#endif /* MT7615 */


INT32 SetATETxBw(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
#if defined(CONFIG_WLAN_SERVICE)
	return (mt_agent_cli_set_ext("ATETXBW", &pAd->serv, Arg) == SERV_STATUS_SUCCESS) ? TRUE : FALSE;
#else
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Ret = 0;
	UINT16 system_bw, per_pkt_bw;
	const INT idx_num = 2;
	UINT32 param[idx_num];
	INT i = 0;
	CHAR *value;

	MTWF_PRINT("%s: Bw = %s\n", __func__, Arg);

	/* Initialization */
	for (i = 0; i < idx_num; i++)
		param[i] = BAND_WIDTH_NUM;

	for (i = 0, value = rstrtok(Arg, ":"); value; value = rstrtok(NULL, ":")) {
		if (i == idx_num)
			break;
		param[i++] = simple_strtol(value, 0, 10);
	}

	system_bw = param[0];
	per_pkt_bw = param[1];

	Ret = ATEOp->SetBW(pAd, system_bw, per_pkt_bw);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
#endif
}


VOID rtmp_ate_init(RTMP_ADAPTER *pAd)
{

	if (ATEInit(pAd) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ATE initialization failed !\n");
		ATEExit(pAd);
		return;
	}
}


VOID RTMPCfgTssiGainFromEEPROM(RTMP_ADAPTER *pAd)
{
	USHORT value = 0;

	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_GAIN_AND_ATTENUATION, value);
	value = (value & 0x00FF);
	pAd->TssiGain = 0x03; /* RT5392 uses 3 as TSSI gain/attenuation default value */

	if ((value != 0x00) && (value != 0xFF))
		pAd->TssiGain =  (UCHAR) (value & 0x000F);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"EEPROM_TSSI_GAIN_AND_ATTENUATION = 0x%X, pAd->TssiGain=0x%x\n", value, pAd->TssiGain);
}

#ifdef SINGLE_SKU_V2
INT32 SetATESingleSKUEn(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION  *ATEOp = ATECtrl->ATEOp;
#endif /* CONFIG_HW_HAL_OFFLOAD */
	UCHAR  control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	BOOLEAN  fgSKUEn = FALSE;
#ifdef DBDC_MODE
	struct _BAND_INFO *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */
	fgSKUEn = simple_strtol(Arg, 0, 10);

	MTWF_PRINT("%s: fgSKUEn: %d, control_band_idx: %d\n",
		__func__, fgSKUEn, control_band_idx);

	/* Update SKU Status in ATECTRL Structure */
	if (BAND0 == control_band_idx)
		ATECtrl->tx_pwr_sku_en = fgSKUEn;
#ifdef DBDC_MODE
	else if (BAND1 == control_band_idx)
		Info->tx_pwr_sku_en = fgSKUEn;
#endif /* DBDC_MODE */

#ifdef CONFIG_HW_HAL_OFFLOAD
	ATEOp->SetCfgOnOff(pAd, EXT_CFG_ONOFF_SINGLE_SKU, fgSKUEn);
#endif /* CONFIG_HW_HAL_OFFLOAD */
	return TRUE;
}
#endif /* SINGLE_SKU_V2 */

INT32 SetATEBFBackoffMode(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	pAd->ucBFBackOffMode = simple_strtol(Arg, 0, 10);
	MTWF_PRINT("%s: ucBFBackOffMode: %d\n", __func__, pAd->ucBFBackOffMode);
	return TRUE;
}

INT32 SetATETempCompEn(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION  *ATEOp = ATECtrl->ATEOp;
#endif /* CONFIG_HW_HAL_OFFLOAD */
	BOOLEAN  fgTempCompEn = FALSE;

	fgTempCompEn = simple_strtol(Arg, 0, 10);

	MTWF_PRINT("%s: fgTempCompEn: %d\n", __func__, fgTempCompEn);

#ifdef CONFIG_HW_HAL_OFFLOAD
	ATEOp->SetCfgOnOff(pAd, EXT_CFG_ONOFF_TEMP_COMP, fgTempCompEn);
#endif /* CONFIG_HW_HAL_OFFLOAD */

	return TRUE;
}


INT32 SetATEPowerPercentEn(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION  *ATEOp = ATECtrl->ATEOp;
#endif /* CONFIG_HW_HAL_OFFLOAD */
	UCHAR  control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	BOOLEAN  fgPowerPercentEn = FALSE;
#ifdef DBDC_MODE
	struct _BAND_INFO *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */
	fgPowerPercentEn = simple_strtol(Arg, 0, 10);

	MTWF_PRINT("%s: fgPowerPercentEn: %d\n", __func__, fgPowerPercentEn);

	/* Update Percentage Status in ATECTRL Structure */
	if (BAND0 == control_band_idx)
		ATECtrl->tx_pwr_percentage_en = fgPowerPercentEn;
#ifdef DBDC_MODE
	else if (BAND1 == control_band_idx)
		Info->tx_pwr_percentage_en = fgPowerPercentEn;
#endif /* DBDC_MODE */

#ifdef CONFIG_HW_HAL_OFFLOAD
	ATEOp->SetCfgOnOff(pAd, EXT_CFG_ONOFF_POWER_PERCENTAGE, fgPowerPercentEn);
#endif /* CONFIG_HW_HAL_OFFLOAD */

	return TRUE;
}


INT32 SetATEPowerPercentCtrl(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
#endif /* CONFIG_HW_HAL_OFFLOAD */
	INT32 Ret = 0;
	UCHAR  control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UINT32  PowerPercentLevel;
#ifdef DBDC_MODE
	struct _BAND_INFO *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */
	PowerPercentLevel = simple_strtol(Arg, 0, 10);

	MTWF_PRINT("%s: PowerPercentLevel = %d\n", __func__, PowerPercentLevel);

	/* Sanity check */
	if (PowerPercentLevel > 100) /*PowerPercentLevel must >= 0*/
		goto err0;

	/* Update TxPower Drop Status in ATECTRL Structure */
	if (BAND0 == control_band_idx)
		ATECtrl->tx_pwr_percentage_level = PowerPercentLevel;
#ifdef DBDC_MODE
	else if (BAND1 == control_band_idx)
		Info->tx_pwr_percentage_level = PowerPercentLevel;
#endif /* DBDC_MODE */

#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret = ATEOp->SetPowerDropLevel(pAd, PowerPercentLevel);
#endif /* CONFIG_HW_HAL_OFFLOAD */

	if (!Ret)
		return TRUE;

err0:
	MTWF_PRINT("%s: Please input X which is 0~100\n", __func__);
	return FALSE;
}


INT32 SetATEBFBackoffEn(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION  *ATEOp = ATECtrl->ATEOp;
#endif /* CONFIG_HW_HAL_OFFLOAD */
	UCHAR  control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	BOOLEAN  fgBFBackoffEn = 0;
#ifdef DBDC_MODE
	struct _BAND_INFO *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */
	fgBFBackoffEn = simple_strtol(Arg, 0, 10);

	MTWF_PRINT("%s: fgBFBackoffEn: %d\n", __func__, fgBFBackoffEn);

	/* Update BF Backoff Status in ATECTRL Structure */
	if (BAND0 == control_band_idx)
		ATECtrl->tx_pwr_backoff_en = fgBFBackoffEn;
#ifdef DBDC_MODE
	else if (BAND1 == control_band_idx)
		Info->tx_pwr_backoff_en = fgBFBackoffEn;
#endif /* DBDC_MODE */

#ifdef CONFIG_HW_HAL_OFFLOAD
	ATEOp->SetCfgOnOff(pAd, EXT_CFG_ONOFF_BF_BACKOFF, fgBFBackoffEn);
#endif /* CONFIG_HW_HAL_OFFLOAD */

		return TRUE;
}


INT32 SetATETSSIEn(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _ATE_CTRL          *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION     *ATEOp = ATECtrl->ATEOp;
#endif /* CONFIG_HW_HAL_OFFLOAD */
	BOOLEAN           fgTSSIEn = 0;

	fgTSSIEn = simple_strtol(Arg, 0, 10);

	MTWF_PRINT("%s: fgBFBackoffEn: %d\n", __func__, fgTSSIEn);

#ifdef CONFIG_HW_HAL_OFFLOAD
	ATEOp->SetCfgOnOff(pAd, EXT_CFG_ONOFF_TSSI, fgTSSIEn);
#endif /* CONFIG_HW_HAL_OFFLOAD */

	return TRUE;
}


INT32 SetATETxPowerCtrlEn(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _ATE_CTRL         *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION    *ATEOp = ATECtrl->ATEOp;
#endif /* CONFIG_HW_HAL_OFFLOAD */
	BOOLEAN          fgTxPowerCtrlEn = 0;

	fgTxPowerCtrlEn = simple_strtol(Arg, 0, 10);

	MTWF_PRINT("%s: fgTxPowerCtrlEn: %d\n", __func__, fgTxPowerCtrlEn);

#ifdef CONFIG_HW_HAL_OFFLOAD
	ATEOp->SetCfgOnOff(pAd, EXT_CFG_ONOFF_TXPOWER_CTRL, fgTxPowerCtrlEn);
#endif /* CONFIG_HW_HAL_OFFLOAD */


	return TRUE;
}

