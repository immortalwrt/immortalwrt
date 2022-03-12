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
	phystate.c
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


/*******************************************************************************
 *   PRIVATE DEFINITIONS
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE TYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/

static VOID phyStateEventDispatcher(struct cmd_msg *msg, char *rsp_payload,
	UINT16 rsp_payload_len);


/*******************************************************************************
 *    PRIVATE DATA
 ******************************************************************************/


/*******************************************************************************
 *    PUBLIC DATA
 ******************************************************************************/


/*******************************************************************************
 *    EXTERNAL DATA
 ******************************************************************************/


/*******************************************************************************
 *    EXTERNAL FUNCTION PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *    PRIVATE FUNCTIONS
 ******************************************************************************/
static VOID ShowPhyStateLastRxRate(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EXT_EVENT_PHY_STATE_RX_RATE rx_rate = (P_EXT_EVENT_PHY_STATE_RX_RATE) msg->attr.rsp.wb_buf_in_calbk;
	P_EXT_EVENT_PHY_STATE_RX_RATE ptr = (P_EXT_EVENT_PHY_STATE_RX_RATE) Data;

	rx_rate->u1PhyStateCate = ptr->u1PhyStateCate;
	rx_rate->u1RxRate = ptr->u1RxRate;
	rx_rate->u1RxMode = ptr->u1RxMode;
	rx_rate->u1RxNsts = ptr->u1RxNsts;
	rx_rate->u1Gi     = ptr->u1Gi;
	rx_rate->u1Coding = ptr->u1Coding;
	rx_rate->u1Stbc   = ptr->u1Stbc;
	rx_rate->u1BW     = ptr->u1BW;
}

static VOID phyStateEventDispatcher(struct cmd_msg *msg, char *rsp_payload,
							UINT16 rsp_payload_len)
{
	UINT8 u1EventId = (*(UINT8 *)rsp_payload);

	switch (u1EventId) {
	case EVENT_PHY_STATE_CONTENTION_RX_PHYRATE:
		ShowPhyStateLastRxRate(msg, rsp_payload, rsp_payload_len);
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s: event:%d\n", __func__, u1EventId));
		break;
	}
}

INT PhyStatGetRssi(
	RTMP_ADAPTER *pAd,
	UINT8 band_idx,
	CHAR *rssi,
	UINT8 *len
	)
{
	RX_STATISTIC_RXV *rx_stat = NULL;
	UINT8 path_idx = 0;
	UINT16 i = 0;
	PMAC_TABLE_ENTRY pEntry = NULL, pCurrEntry = NULL;
	CHAR rssi_tmp[4] = {0};
	UINT16 wcid = 0;
	UINT32 ent_type = ENTRY_CLIENT;
	BOOLEAN fgTestMode = FALSE;

	if (!rssi)
		goto error0;

	if (!len)
		goto error0;

	if (band_idx >= DBDC_BAND_NUM)
		goto error1;

	rx_stat = pAd->rx_stat_rxv + band_idx;

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
		fgTestMode = TRUE;
	else
		fgTestMode = FALSE;
#else
	fgTestMode = FALSE;
#endif /* CONFIG_ATE */

	if (fgTestMode) {
		*len = 4;
		for (path_idx = 0; path_idx < 4; path_idx++)
			*(rssi + path_idx) = rx_stat->RSSI[path_idx];

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s: rssi: %d, %d, %d, %d.\n", __func__,
			rx_stat->RSSI[0], rx_stat->RSSI[1], rx_stat->RSSI[2], rx_stat->RSSI[3]));
	} else {
		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pEntry = &pAd->MacTab.Content[i];
			/* dump MacTable entries which match the EntryType */
			if (pEntry->EntryType != ent_type)
				continue;

			if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
					&& (pEntry->Sst != SST_ASSOC))
				continue;

			pCurrEntry = pEntry;
			break;
		}

		*len = 4;
		if (!pCurrEntry)
			goto error2;

		wcid = pCurrEntry->wcid;
		chip_get_rssi(pAd, wcid, rssi_tmp);

		for (i = 0; i < *len; i++)
			*(rssi + i) = rssi_tmp[i];

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s: wcid: %d, rssi: %d, %d, %d, %d.\n", __func__,
			pCurrEntry->wcid, rssi_tmp[0], rssi_tmp[1], rssi_tmp[2], rssi_tmp[3]));
	}

	return 0;

error0:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: null pointer for content buffer.\n", __func__));
	return 1;

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: invalid band index(%d).\n", __func__, band_idx));
	return 1;

error2:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		("%s: invalid entry. no station link up.\n", __func__));
	return 1;
}

INT PhyStatGetCnInfo(
	RTMP_ADAPTER *pAd,
	UINT8 ucband_idx,
	UINT16 *pCnInfo
	)
{
	if (ucband_idx >= DBDC_BAND_NUM)
		return FALSE;

	chip_get_cninfo(pAd, ucband_idx, pCnInfo);
	return TRUE;
}

INT ShowTxPhyRate(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	BOOLEAN fgset
)
{
	INT status = FALSE;
	UINT8 ucBandIdx;
	struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
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

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(PHY STATE INFO)\n"));

	/* READ */
	if (fgset == FALSE) {
		if (MtCmdPhyShowInfo(pAd, CMD_PHY_STATE_TX_PHYRATE, ucBandIdx) == RETURN_STATUS_TRUE)
			status = TRUE;
	}

	return status;
}

INT ShowLastRxPhyRate(
	RTMP_ADAPTER *pAd,
	UINT8 ucBandIdx,
	UINT16 u2Wcid,
	UINT32 *rx_rate
)
{
	INT status = FALSE;
	EXT_EVENT_PHY_STATE_RX_RATE rRxRateInfo;

	if (ucBandIdx >= DBDC_BAND_NUM)
		return FALSE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("(PHY STATE INFO)\n"));
	os_zero_mem(&rRxRateInfo, sizeof(rRxRateInfo));

	/* READ */
	if (MtCmdPhyGetRxRate(pAd, CMD_PHY_STATE_CONTENTION_RX_PHYRATE, ucBandIdx, u2Wcid, &rRxRateInfo) == RETURN_STATUS_TRUE)
		status = TRUE;

	*rx_rate = 0;

	/* Mode [19:16]     GI [15:14]     Rate [13:8]     BW [7:5]     STBC [4]     Coding [3]     Nsts [2:0]  */
	*rx_rate = ((rRxRateInfo.u1RxMode & 0xF) << 16) | ((rRxRateInfo.u1Gi & 0x3) << 14) |
				((rRxRateInfo.u1RxRate & 0x3F) << 8) | ((rRxRateInfo.u1BW & 0x7) << 5) |
				((rRxRateInfo.u1Stbc & 0x1) << 4) | ((rRxRateInfo.u1Coding & 0x1) << 3) |
				(rRxRateInfo.u1RxNsts & 0x7);

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("contention-based:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: Wcid:%u Rate::%u Mode:%u Nsts:%u\n",
			__func__, u2Wcid, rRxRateInfo.u1RxRate, rRxRateInfo.u1RxMode, rRxRateInfo.u1RxNsts));

	return status;
}

INT ShowRxPhyRate(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	BOOLEAN fgset
)
{
	INT status = FALSE;
	UINT8 ucBandIdx;
	struct	wifi_dev *wdev;
	UINT8 rx_rate = 0, rx_nsts = 0, rx_mode = 0, buf = 0;
	CHAR str[6][20] = {"CCK", "OFDM", "HT Mix-Mode", "HT Green-Field", "VHT", "HE"};
	CHAR rx_mode_str[20];
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
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

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(PHY STATE INFO)\n"));

	/* READ */
	if (fgset == FALSE) {
		if (MtCmdPhyShowInfo(pAd, CMD_PHY_STATE_RX_PHYRATE, ucBandIdx) == RETURN_STATUS_TRUE)
			status = TRUE;
	}

	/* contention-based rx rate parsing */
	buf = (pAd->phy_stat_elem.rx_raw & BITS(0, 6)) >> 0;
	rx_nsts = (pAd->phy_stat_elem.rx_raw & BITS(7, 9)) >> 7;
	rx_mode = (pAd->phy_stat_elem.rx_raw2 & BITS(4, 7)) >> 4;

	/* contention-based rx nsts handle */
	rx_nsts += 1;

	switch (rx_mode) {
	case 0:
		os_move_mem(rx_mode_str, *(str + 0), 20);
		rx_rate = buf & BITS(0, 1);
		break;
	case 1:
		os_move_mem(rx_mode_str, *(str + 1), 20);
		if ((buf & BITS(0, 3)) == 0xb)
			rx_rate = 0;
		else if ((buf & BITS(0, 3)) == 0xf)
			rx_rate = 1;
		else if ((buf & BITS(0, 3)) == 0xa)
			rx_rate = 2;
		else if ((buf & BITS(0, 3)) == 0xe)
			rx_rate = 3;
		else if ((buf & BITS(0, 3)) == 0x9)
			rx_rate = 4;
		else if ((buf & BITS(0, 3)) == 0xd)
			rx_rate = 5;
		else if ((buf & BITS(0, 3)) == 0x8)
			rx_rate = 6;
		else if ((buf & BITS(0, 3)) == 0xc)
			rx_rate = 7;
		break;
	case 2:
		os_move_mem(rx_mode_str, *(str + 2), 20);
		rx_rate = (buf & BITS(0, 6));
		break;
	case 3:
		os_move_mem(rx_mode_str, *(str + 3), 20);
		rx_rate = (buf & BITS(0, 6));
		break;
	case 4:
		os_move_mem(rx_mode_str, *(str + 4), 20);
		rx_rate = (buf & BITS(0, 3));
		break;
	case 8:
		os_move_mem(rx_mode_str, *(str + 5), 20);
		rx_rate = (buf & BITS(0, 3));
		break;
	case 9:
		os_move_mem(rx_mode_str, *(str + 5), 20);
		rx_rate = (buf & BITS(0, 3));
		break;
	case 10:
		os_move_mem(rx_mode_str, *(str + 5), 20);
		rx_rate = (buf & BITS(0, 3));
		break;
	case 11:
		os_move_mem(rx_mode_str, *(str + 5), 20);
		rx_rate = (buf & BITS(0, 3));
		break;
	default:
		break;
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("contention-based:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rx rate: %s(%dNss M%d)\n", rx_mode_str, rx_nsts, rx_rate));

	return status;
}

INT32 MtCmdPhyGetRxRate(
	RTMP_ADAPTER *pAd,
	UCHAR ucPhyStateInfoCatg,
	UINT8 ucBandIdx,
	UINT16 u2Wcid,
	P_EXT_EVENT_PHY_STATE_RX_RATE prRxRateInfo
)
{
	struct cmd_msg *msg;
	CMD_PHY_STATE_SHOW_INFO_T PhyStateShowInfoCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PHY_STATE_SHOW_INFO_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&PhyStateShowInfoCtrl, sizeof(CMD_PHY_STATE_SHOW_INFO_T));
	PhyStateShowInfoCtrl.ucPhyStateInfoCatg = ucPhyStateInfoCatg;
	PhyStateShowInfoCtrl.ucBandIdx = ucBandIdx;
	PhyStateShowInfoCtrl.u2Wcid = u2Wcid;
#ifdef RT_BIG_ENDIAN
	PhyStateShowInfoCtrl.u2Wcid = cpu2le16(PhyStateShowInfoCtrl.u2Wcid);
#endif
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PHY_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(*prRxRateInfo));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, prRxRateInfo);
	SET_CMD_ATTR_RSP_HANDLER(attr, phyStateEventDispatcher);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&PhyStateShowInfoCtrl,
						sizeof(CMD_PHY_STATE_SHOW_INFO_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s:(ret = %d)\n", __func__, ret));
	return ret;
}


/* Send command to firmware */
INT32 MtCmdPhyShowInfo(
	RTMP_ADAPTER *pAd,
	UCHAR ucPhyStateInfoCatg,
	UINT8 ucBandIdx
)
{
	struct cmd_msg *msg;
	CMD_PHY_STATE_SHOW_INFO_T PhyStateShowInfoCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: ucPhyStateInfoCatg: %d, BandIdx: %d \n", __func__, ucPhyStateInfoCatg, ucBandIdx));
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PHY_STATE_SHOW_INFO_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&PhyStateShowInfoCtrl, sizeof(CMD_PHY_STATE_SHOW_INFO_T));
	PhyStateShowInfoCtrl.ucPhyStateInfoCatg = ucPhyStateInfoCatg;
	PhyStateShowInfoCtrl.ucBandIdx = ucBandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PHY_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&PhyStateShowInfoCtrl,
						sizeof(CMD_PHY_STATE_SHOW_INFO_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}

VOID EventPhyStatHandler(PRTMP_ADAPTER pAd, UINT8 *Data, UINT_32 Length)
{
	/* Event ID */
	UINT8 u1PhyStateEventId;

	/* Get Event Category ID */
	u1PhyStateEventId = *Data;

	/**Prevent legitimate but wrong ID **/
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		 ("%s: u1PhyStateEventId = %d\n", __func__, u1PhyStateEventId));

	/* Event Handle for different Category ID */
	switch (u1PhyStateEventId) {
	case EVENT_PHY_STATE_TX_PHYRATE:
		EventPhyStatTxRate(pAd, Data, Length);
		break;

	case EVENT_PHY_STATE_RX_PHYRATE:
		EventPhyStatRxRate(pAd, Data, Length);
		break;

	default:
		break;
	}
}

VOID EventPhyStatTxRate(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_PHY_STATE_TX_RATE prEventPhyStatTxRate = NULL;
	UINT8 tx_rate = 0, tx_nsts = 0, tx_mode = 0;
	CHAR str[6][20] = {"CCK", "OFDM", "HT Mix-Mode", "HT Green-Field", "VHT", "HE"};
	CHAR tx_mode_str[20];

	prEventPhyStatTxRate = (P_EXT_EVENT_PHY_STATE_TX_RATE)Data;
	tx_rate = prEventPhyStatTxRate->u1TxRate;
	tx_mode = prEventPhyStatTxRate->u1TxMode;
	tx_nsts = prEventPhyStatTxRate->u1TxNsts;
	os_move_mem(tx_mode_str, *(str + tx_mode), 20);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx rate: %s(%dNss M%d)\n", tx_mode_str, tx_nsts, tx_rate));
}

VOID EventPhyStatRxRate(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_PHY_STATE_RX_RATE prEventPhyStatRxRate = NULL;
	UINT8 rx_rate = 0, rx_nsts = 0, rx_mode = 0;
	CHAR str[6][20] = {"CCK", "OFDM", "HT Mix-Mode", "HT Green-Field", "VHT", "HE"};
	CHAR rx_mode_str[20];

	prEventPhyStatRxRate = (P_EXT_EVENT_PHY_STATE_RX_RATE)Data;
	rx_rate = prEventPhyStatRxRate->u1RxRate;
	rx_mode = prEventPhyStatRxRate->u1RxMode;
	rx_nsts = prEventPhyStatRxRate->u1RxNsts;
	os_move_mem(rx_mode_str, *(str + rx_mode), 20);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("trigger-based:\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rx rate: %s(%dNss M%d)\n", rx_mode_str, rx_nsts, rx_rate));
}

static VOID phy_stat_rssi_rsp_handle(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_RSSI_REPORT rssi_rpt = (P_RSSI_REPORT) msg->attr.rsp.wb_buf_in_calbk;
	P_EXT_EVENT_PHY_STATE_RSSI ptr = (P_EXT_EVENT_PHY_STATE_RSSI) Data;
	UINT8 i = 0;
	UINT8 rcpi;
	CHAR rssi;

	for (i = 0; i < 4; i++) {
		rcpi = (ptr->u1Rcpi[i]);
		rssi = (rcpi - 220) / 2;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s(): ant: %d, rcpi: %d, rssi: %d\n", __func__, i, rcpi, rssi));

		if (rssi > 0)
			rssi_rpt->rssi[i] = -127;
		else
			rssi_rpt->rssi[i] = rssi;
	}
}

static VOID phy_stat_multi_rssi_rsp_handle(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT32 index;
	UINT32 num = Len / sizeof(EXT_EVENT_PHY_STATE_RSSI);
	P_EXT_EVENT_PHY_STATE_RSSI ptr = (P_EXT_EVENT_PHY_STATE_RSSI) Data;
	P_RSSI_PAIR RssiPair = (P_RSSI_PAIR) msg->attr.rsp.wb_buf_in_calbk;

	UINT8 i = 0;
	UINT8 rcpi;
	CHAR rssi;

	for (index = 0; index < num; index++) {
		RssiPair->u2WlanIdx = ptr->u2WlanIdx;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s(): WlanIdx: %d\n", __func__, RssiPair->u2WlanIdx));

		for (i = 0; i < 4; i++) {
			rcpi = (ptr->u1Rcpi[i]);
			rssi = (rcpi - 220) / 2;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s(): ant: %d, rcpi: %d, rssi: %d\n", __func__, i, rcpi, rssi));

			if (rssi > 0)
				RssiPair->rssi[i] = -127;
			else
				RssiPair->rssi[i] = rssi;
		}
		ptr++;
		RssiPair++;
	}
}

INT32 MtCmdGetRssi(
	RTMP_ADAPTER *pAd,
	UINT16 u2WlanIdx,
	RSSI_REPORT *rssi_rpt
)
{
	struct cmd_msg *msg;
	CMD_PHY_STATE_SHOW_RSSI_T PhyStatRssi;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s: wcid: %d\n", __func__, u2WlanIdx));
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PHY_STATE_SHOW_RSSI_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(&PhyStatRssi, sizeof(CMD_PHY_STATE_SHOW_RSSI_T));
	PhyStatRssi.u1PhyStateInfoCatg = CMD_PHY_STATE_RSSI;
	PhyStatRssi.u2WlanIdx = u2WlanIdx;
#ifdef RT_BIG_ENDIAN
	PhyStatRssi.u2WlanIdx = cpu2le16(PhyStatRssi.u2WlanIdx);
#endif
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PHY_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_PHY_STATE_RSSI));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, rssi_rpt);
	SET_CMD_ATTR_RSP_HANDLER(attr, phy_stat_rssi_rsp_handle);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&PhyStatRssi,
						sizeof(CMD_PHY_STATE_SHOW_RSSI_T));
	ret = chip_cmd_tx(pAd, msg);
	return ret;

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}

INT32 MtCmdMultiRssi(
	RTMP_ADAPTER *pAd,
	RSSI_PAIR *RssiPair,
	UINT32 Num
)
{
	struct cmd_msg *msg;
	CMD_PHY_STATE_SHOW_RSSI_T PhyStatRssi;
	INT32 ret = 0;
	UINT32 index;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PHY_STATE_SHOW_RSSI_T) * Num);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PHY_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_PHY_STATE_RSSI) * Num);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, RssiPair);
	SET_CMD_ATTR_RSP_HANDLER(attr, phy_stat_multi_rssi_rsp_handle);
	AndesInitCmdMsg(msg, attr);

	for (index = 0; index < Num; index++) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s: wcid: %d\n", __func__, RssiPair[index].u2WlanIdx));
		os_zero_mem(&PhyStatRssi, sizeof(CMD_PHY_STATE_SHOW_RSSI_T));
		PhyStatRssi.u1PhyStateInfoCatg = CMD_PHY_STATE_RSSI;
		PhyStatRssi.u2WlanIdx = RssiPair[index].u2WlanIdx;
		AndesAppendCmdMsg(msg, (char *)&PhyStatRssi, sizeof(CMD_PHY_STATE_SHOW_RSSI_T));
	}
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}

static VOID phy_stat_cninfo_rsp_handle(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	PUINT16 pcninfo = (PUINT16) msg->attr.rsp.wb_buf_in_calbk;
	P_EXT_EVENT_PHY_STATE_OFDMLQ_CN ptr = (P_EXT_EVENT_PHY_STATE_OFDMLQ_CN) Data;

	(*pcninfo) = ptr->u2OfdmLqCn;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s(): CnInfo: %d\n", __func__, *pcninfo));

}

INT32 MtCmdGetCnInfo(
	RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT16 *u2cninfo
)
{
	struct cmd_msg *msg;
	CMD_PHY_STATE_SHOW_OFDMLQ_CN_T PhyStatCnInfo;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s: Bandidx: %d\n", __func__, u1BandIdx));
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PHY_STATE_SHOW_OFDMLQ_CN_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(&PhyStatCnInfo, sizeof(CMD_PHY_STATE_SHOW_OFDMLQ_CN_T));
	PhyStatCnInfo.u1PhyStateInfoCatg = CMD_PHY_STATE_OFDMLQ_CNINFO;
	PhyStatCnInfo.u1BandIdx = u1BandIdx;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PHY_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EXT_EVENT_PHY_STATE_RX_RATE));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, u2cninfo);
	SET_CMD_ATTR_RSP_HANDLER(attr, phy_stat_cninfo_rsp_handle);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&PhyStatCnInfo,
						sizeof(CMD_PHY_STATE_SHOW_OFDMLQ_CN_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}

#ifdef SPECIAL_11B_OBW_FEATURE
INT32 MtCmdSetTxTdCck(
	RTMP_ADAPTER *pAd,
	UINT8 u1Enable
)
{
	struct cmd_msg *msg;
	CMD_PHY_SET_TXTD_CCK_T PhysetTxTd;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PHY_SET_TXTD_CCK_T));
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(&PhysetTxTd, sizeof(CMD_PHY_SET_TXTD_CCK_T));
	PhysetTxTd.u1PhyStateInfoCatg = CMD_PHY_STATE_TX_TD_CCK;
	PhysetTxTd.u1Enable = u1Enable;
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_PHY_STAT_INFO);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&PhysetTxTd,
						sizeof(CMD_PHY_SET_TXTD_CCK_T));
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}
#endif /* SPECIAL_11B_OBW_FEATURE */

