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
	cmm_asic_mt.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"
#include "hdev/hdev.h"

/* DEV Info */
INT32 MtAsicSetDevMacByFw(
	RTMP_ADAPTER * pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 EnableFeature)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		return UniCmdDevInfoUpdate(pAd,
							   OwnMacIdx,
							   OwnMacAddr,
							   BandIdx,
							   Active,
							   EnableFeature);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdExtDevInfoUpdate(pAd,
							   OwnMacIdx,
							   OwnMacAddr,
							   BandIdx,
							   Active,
							   EnableFeature);
	}
}


/* BSS Info */
INT32 MtAsicSetBssidByFw(
	struct _RTMP_ADAPTER *pAd,
	struct _BSS_INFO_ARGUMENT_T *bss_info_argument)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		return UniCmdBssInfoUpdate(pAd, bss_info_argument);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdExtBssInfoUpdate(pAd, bss_info_argument);
	}
}

/* STARec Info */
INT32 MtAsicSetStaRecByFw(
	RTMP_ADAPTER * pAd,
	STA_REC_CFG_T *pStaCfg)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		return UniCmdStaRecUpdate(pAd, pStaCfg);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdExtStaRecUpdate(pAd, pStaCfg);
	}

}

INT32 MtAsicUpdateStaRecBaByFw(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_BA_CFG_T StaRecBaCfg)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		return UniCmdStaRecBaUpdate(pAd, StaRecBaCfg);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdExtStaRecBaUpdate(pAd, StaRecBaCfg);
	}
}


VOID MtSetTmrCRByFw(struct _RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx)
{
	CmdExtSetTmrCR(pAd, enable, BandIdx);
}


VOID AsicAutoBATrigger(struct _RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Timeout)
{
	CmdAutoBATrigger(pAd, Enable, Timeout);
}


VOID MtAsicDelWcidTabByFw(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 wcid_idx)
{
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, " --->\n");
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wcid_idx(%d)\n", wcid_idx);

	if (wcid_idx == WCID_ALL)
		CmdExtWtblUpdate(pAd, 0, RESET_ALL_WTBL, NULL, 0);
	else
		CmdExtWtblUpdate(pAd, wcid_idx, RESET_WTBL_AND_SET, NULL, 0);

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, " <---\n");
}


#ifdef HTC_DECRYPT_IOT
/*
	Old Chip PATH (ex: MT7615 / MT7622 ) :
*/
VOID MtAsicSetWcidAAD_OMByFw(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 wcid_idx,
	IN UCHAR value)
{
	UINT32 mask = 0xfffffff7;
	UINT32 val;

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wcid_idx(%d), value(%d)\n", wcid_idx, value);

	if (value) {
		val = 0x8;
		WtblDwSet(pAd, wcid_idx, 1, 2, mask, val);
	} else {
		val = 0x0;
		WtblDwSet(pAd, wcid_idx, 1, 2, mask, val);
	}

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<---\n");
}

/*
	CONNAC F/W CMD PATH:
*/
INT32 MtAsicUpdateStaRecAadOmByFw(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 Wcid,
	IN UINT8 AadOm)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		return UniCmdStaRecAADOmUpdate(pAd, Wcid, AadOm);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdExtStaRecAADOmUpdate(pAd, Wcid, AadOm);
	}

}

#endif /* HTC_DECRYPT_IOT */

INT32 MtAsicSetWcidPsmByFw(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 wcid_idx,
	IN UCHAR value)
{
	INT32 ret = 0;
	struct _RTMP_CHIP_DBG *chip_dbg;

	chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->set_sta_psm)
		ret = chip_dbg->set_sta_psm(pAd, wcid_idx, value);
	else
		AsicNotSupportFunc(pAd, __func__);

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"wcid_idx(%d), value(%d)\n", wcid_idx, value);

	return ret;
}

INT32 MtAsicUpdateStaRecPsmByFw(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 Wcid,
	IN UINT8 Psm)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		return UniCmdStaRecPsmUpdate(pAd, Wcid, Psm);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdExtStaRecPsmUpdate(pAd, Wcid, Psm);
	}

}

INT32 MtAsicUpdateStaRecSNByFw(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 Wcid,
	IN UINT16 Sn)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		return UniCmdStaRecSNUpdate(pAd, Wcid, Sn);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdExtStaRecSNUpdate(pAd, Wcid, Sn);
	}

}

#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
VOID MtAsicSetWcid4Addr_HdrTransByFw(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 wcid_idx,
	IN UCHAR IsEnable,
	IN UCHAR IsApcliEntry)
{

	CMD_WTBL_HDR_TRANS_T	rWtblHdrTrans = {0};

	rWtblHdrTrans.u2Tag = WTBL_HDR_TRANS;
	rWtblHdrTrans.u2Length = sizeof(CMD_WTBL_HDR_TRANS_T);

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"WCID %u ISenable %u\n",
						wcid_idx, IsEnable);
	/*Set to 1 */
	if (IsEnable) {
		rWtblHdrTrans.ucTd = 1;
		rWtblHdrTrans.ucFd = 1;
	} else if (IsApcliEntry) {
		rWtblHdrTrans.ucTd = 1;
		rWtblHdrTrans.ucFd = 0;
	} else {
		rWtblHdrTrans.ucTd = 0;
		rWtblHdrTrans.ucFd = 1;
	}
	rWtblHdrTrans.ucDisRhtr = 0;
	CmdExtWtblUpdate(pAd, wcid_idx, SET_WTBL, &rWtblHdrTrans, sizeof(CMD_WTBL_HDR_TRANS_T));

}
#endif
VOID MtAsicUpdateRxWCIDTableByFw(
	IN PRTMP_ADAPTER pAd,
	IN MT_WCID_TABLE_INFO_T WtblInfo)
{
	NDIS_STATUS					Status = NDIS_STATUS_SUCCESS;
	UCHAR						*pTlvBuffer = NULL;
	UCHAR						*pTempBuffer = NULL;
	UINT32						u4TotalTlvLen = 0;
	UCHAR						ucTotalTlvNumber = 0;
	/* Tag = 0, Generic */
	CMD_WTBL_GENERIC_T		rWtblGeneric = {0};
	/* Tage = 1, Rx */
	CMD_WTBL_RX_T				rWtblRx = {0};
#ifdef DOT11_N_SUPPORT
	/* Tag = 2, HT */
	CMD_WTBL_HT_T				rWtblHt = {0};
#ifdef DOT11_VHT_AC
	/* Tag = 3, VHT */
	CMD_WTBL_VHT_T			rWtblVht = {0};
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	/* Tag = 5, TxPs */
	CMD_WTBL_TX_PS_T			rWtblTxPs = {0};
#if defined(HDR_TRANS_TX_SUPPORT) || defined(HDR_TRANS_RX_SUPPORT)
	/* Tag = 6, Hdr Trans */
	CMD_WTBL_HDR_TRANS_T	rWtblHdrTrans = {0};
#endif /* HDR_TRANS_TX_SUPPORT */
	/* Tag = 7, Security Key */
	CMD_WTBL_SECURITY_KEY_T	rWtblSecurityKey = {0};
	/* Tag = 9, Rdg */
	CMD_WTBL_RDG_T			rWtblRdg = {0};
#ifdef TXBF_SUPPORT
	/* Tag = 12, BF */
	CMD_WTBL_BF_T           rWtblBf = {0};
#endif /* TXBF_SUPPORT */
	/* Tag = 13, SMPS */
	CMD_WTBL_SMPS_T			rWtblSmPs = {0};
	/* Tag = 16, SPE */
	CMD_WTBL_SPE_T          rWtblSpe = {0};
	/* Allocate TLV msg */
	Status = os_alloc_mem(pAd, (UCHAR **)&pTlvBuffer, MAX_BUF_SIZE_OF_WTBL_INFO);
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, " %d,%d,%d,%d,%d,%d,%d,%d,%d,("MACSTR"),%d,%d,%d,%d,%d,%d)\n",
			 WtblInfo.Wcid,
			 WtblInfo.Aid,
			 WtblInfo.BssidIdx,
			 WtblInfo.MacAddrIdx,
			 WtblInfo.SmpsMode,
			 WtblInfo.MaxRAmpduFactor,
			 WtblInfo.MpduDensity,
			 WtblInfo.WcidType,
			 WtblInfo.aad_om,
			 MAC2STR(WtblInfo.Addr),
			 WtblInfo.CipherSuit,
			 WtblInfo.PfmuId,
			 WtblInfo.SupportHT,
			 WtblInfo.SupportVHT,
			 WtblInfo.SupportRDG,
			 WtblInfo.SupportQoS);

	if ((Status != NDIS_STATUS_SUCCESS) || (pTlvBuffer == NULL))
		goto error;

	rWtblRx.ucRv   = WtblInfo.rv;
	rWtblRx.ucRca2 = WtblInfo.rca2;

	if (WtblInfo.WcidType == MT_WCID_TYPE_APCLI_MCAST) {
		/* prevent BMC ICV message dumped during GTK rekey */
		if (IF_COMBO_HAVE_AP_STA(pAd) && HcGetWcidLinkType(pAd, WtblInfo.Wcid) == WDEV_TYPE_STA)
			rWtblRx.ucRcid = 1;
	}

	/* Manipulate TLV msg */
	if (WtblInfo.WcidType == MT_WCID_TYPE_BMCAST) {
		/* Tag = 0 */
		rWtblGeneric.ucMUARIndex = 0x0e;
		/* Tag = 1 */
		rWtblRx.ucRv = 1;
		rWtblRx.ucRca1 = 1;
		/* if (pAd->OpMode == OPMODE_AP) */
		{
			rWtblRx.ucRca2 = 1;
		}
		/* Tag = 7 */
		rWtblSecurityKey.ucAlgorithmId = WTBL_CIPHER_NONE;
		/* Tag = 6 */
#ifdef HDR_TRANS_TX_SUPPORT

		if (pAd->OpMode == OPMODE_AP) {
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 0;
		}

#endif
	} else {
		/* Tag = 0 */
		rWtblGeneric.ucMUARIndex = WtblInfo.MacAddrIdx;
		rWtblGeneric.ucQos = (WtblInfo.SupportQoS) ? 1 : 0;
		rWtblGeneric.u2PartialAID = WtblInfo.Aid;
		rWtblGeneric.ucAadOm = WtblInfo.aad_om;

		/* Tag = 1 */
		if ((WtblInfo.WcidType == MT_WCID_TYPE_APCLI) ||
			(WtblInfo.WcidType == MT_WCID_TYPE_REPEATER) ||
			(WtblInfo.WcidType == MT_WCID_TYPE_AP) ||
			(WtblInfo.WcidType == MT_WCID_TYPE_APCLI_MCAST))
			rWtblRx.ucRca1 = 1;

		rWtblRx.ucRv = 1;
		rWtblRx.ucRca2 = 1;
		/* Tag = 7 */
		rWtblSecurityKey.ucAlgorithmId = WtblInfo.CipherSuit;
		rWtblSecurityKey.ucRkv = (WtblInfo.CipherSuit != WTBL_CIPHER_NONE) ? 1 : 0;
		/* Tag = 6 */
#ifdef HDR_TRANS_TX_SUPPORT

		switch (WtblInfo.WcidType) {
		case MT_WCID_TYPE_AP:
			rWtblHdrTrans.ucFd = 0;
			rWtblHdrTrans.ucTd = 1;
			break;

		case MT_WCID_TYPE_CLI:
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 0;
#if defined(A4_CONN) || defined(MBSS_AS_WDS_AP_SUPPORT)
			if (WtblInfo.a4_enable) {
				rWtblHdrTrans.ucFd = 1;
				rWtblHdrTrans.ucTd = 1;
				/* MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,"MtAsicUpdateRxWCIDTableByFw MT_WCID_TYPE_CLI: do FdTd settings in rWtblHdrTrans\n"); */
			}
#endif /* A4_CONN */
			break;

		case MT_WCID_TYPE_APCLI:
		case MT_WCID_TYPE_REPEATER:
			rWtblHdrTrans.ucFd = 0;
			rWtblHdrTrans.ucTd = 1;
#if defined(A4_CONN) || defined(APCLI_AS_WDS_STA_SUPPORT)
			if (WtblInfo.a4_enable) {
				rWtblHdrTrans.ucFd = 1;
				rWtblHdrTrans.ucTd = 1;
				/* MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,"MtAsicUpdateRxWCIDTableByFw MT_WCID_TYPE_APCLI/MT_WCID_TYPE_REPEATER do FdTd settings in rWtblHdrTrans\n"); */
			}
#endif /* A4_CONN */
			break;

		case MT_WCID_TYPE_WDS:
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 1;
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "Unknown entry type(%d) do not support header translation\n",
					  WtblInfo.WcidType);
			break;
		}

#endif /* HDR_TRANS_TX_SUPPORT */
#ifdef HDR_TRANS_RX_SUPPORT

		if (WtblInfo.DisRHTR)
			rWtblHdrTrans.ucDisRhtr = 1;
		else
			rWtblHdrTrans.ucDisRhtr = 0;

#endif /* HDR_TRANS_RX_SUPPORT */
#ifdef DOT11_N_SUPPORT

		if (WtblInfo.SupportHT) {
			/* Tag = 0 */
			rWtblGeneric.ucQos = 1;
			rWtblGeneric.ucBafEn = 0;
			/* Tag = 2 */
			rWtblHt.ucHt = 1;
			rWtblHt.ucMm = WtblInfo.MpduDensity;
			rWtblHt.ucAf = WtblInfo.MaxRAmpduFactor;

			/* Tga = 9 */
			if (WtblInfo.SupportRDG) {
				rWtblRdg.ucR = 1;
				rWtblRdg.ucRdgBa = 1;
			}

			/* Tag = 13*/
			if (WtblInfo.SmpsMode == MMPS_DYNAMIC)
				rWtblSmPs.ucSmPs = 1;
			else
				rWtblSmPs.ucSmPs = 0;

#ifdef DOT11_VHT_AC

			/* Tag = 3 */
			if (WtblInfo.SupportVHT) {
				rWtblVht.ucVht = 1;
				/* ucDynBw for WTBL: 0 - not DynBW / 1 -DynBW */
				rWtblVht.ucDynBw = (WtblInfo.dyn_bw == BW_SIGNALING_DYNAMIC)?1:0;
			}

#endif /* DOT11_VHT_AC */
		}

#endif /* DOT11_N_SUPPORT */
	}

	/* Tag = 0 */
	os_move_mem(rWtblGeneric.aucPeerAddress, WtblInfo.Addr, MAC_ADDR_LEN);
	/* Tag = 5 */
	rWtblTxPs.ucTxPs = 0;
#ifdef TXBF_SUPPORT
	/* Tag = 0xc */
	rWtblBf.ucGid     = WtblInfo.gid;
	rWtblBf.ucPFMUIdx = WtblInfo.PfmuId;
	rWtblBf.ucTiBf    = WtblInfo.fgTiBf;
	rWtblBf.ucTeBf    = WtblInfo.fgTeBf;
	rWtblBf.ucTibfVht = WtblInfo.fgTibfVht;
	rWtblBf.ucTebfVht = WtblInfo.fgTebfVht;
#endif /* TXBF_SUPPORT */
	/* Tag = 0x10 */
	rWtblSpe.ucSpeIdx = WtblInfo.spe_idx;
	/* Append TLV msg */
	pTempBuffer = pTlvAppend(
					  pTlvBuffer,
					  (WTBL_GENERIC),
					  (sizeof(CMD_WTBL_GENERIC_T)),
					  &rWtblGeneric,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_RX),
					  (sizeof(CMD_WTBL_RX_T)),
					  &rWtblRx,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#ifdef DOT11_N_SUPPORT
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_HT),
					  (sizeof(CMD_WTBL_HT_T)),
					  &rWtblHt,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_RDG),
					  (sizeof(CMD_WTBL_RDG_T)),
					  &rWtblRdg,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_SMPS),
					  (sizeof(CMD_WTBL_SMPS_T)),
					  &rWtblSmPs,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#ifdef DOT11_VHT_AC
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_VHT),
					  (sizeof(CMD_WTBL_VHT_T)),
					  &rWtblVht,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_TX_PS),
					  (sizeof(CMD_WTBL_TX_PS_T)),
					  &rWtblTxPs,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#if defined(HDR_TRANS_RX_SUPPORT) || defined(HDR_TRANS_TX_SUPPORT)
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_HDR_TRANS),
					  (sizeof(CMD_WTBL_HDR_TRANS_T)),
					  &rWtblHdrTrans,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#endif /* HDR_TRANS_RX_SUPPORT || HDR_TRANS_TX_SUPPORT */
	if (WtblInfo.SkipClearPrevSecKey == FALSE)
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_SECURITY_KEY),
					  (sizeof(CMD_WTBL_SECURITY_KEY_T)),
					  &rWtblSecurityKey,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#ifdef TXBF_SUPPORT

	if (pAd->rStaRecBf.u2PfmuId != 0xFFFF) {
		pTempBuffer = pTlvAppend(
						  pTempBuffer,
						  (WTBL_BF),
						  (sizeof(CMD_WTBL_BF_T)),
						  &rWtblBf,
						  &u4TotalTlvLen,
						  &ucTotalTlvNumber);
	}

#endif /* TXBF_SUPPORT */
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_SPE),
					  (sizeof(CMD_WTBL_SPE_T)),
					  &rWtblSpe,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	/* Send TLV msg*/
	if (WtblInfo.SkipClearPrevSecKey == FALSE || WtblInfo.IsReset == TRUE)
		CmdExtWtblUpdate(pAd, WtblInfo.Wcid, RESET_WTBL_AND_SET, pTlvBuffer, u4TotalTlvLen);
	else
		CmdExtWtblUpdate(pAd, WtblInfo.Wcid, SET_WTBL, pTlvBuffer, u4TotalTlvLen);
	/* Free TLV msg */
	if (pTlvBuffer)
		os_free_mem(pTlvBuffer);

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(Ret = %d)\n", Status);
}



VOID MtAsicUpdateBASessionByWtblTlv(RTMP_ADAPTER *pAd, MT_BA_CTRL_T BaCtrl)
{
	CMD_WTBL_BA_T		rWtblBa = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 *ba_range = cap->ppdu.ba_range;

	if (BaCtrl.Tid > 7) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "unknown tid(%d)\n", BaCtrl.Tid);
		return;
	}

	rWtblBa.u2Tag = WTBL_BA;
	rWtblBa.u2Length = sizeof(CMD_WTBL_BA_T);
	rWtblBa.ucTid = BaCtrl.Tid;
	rWtblBa.ucBaSessionType = BaCtrl.BaSessionType;

	if (BaCtrl.BaSessionType == BA_SESSION_RECP) {
		/* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
		if (BaCtrl.isAdd) {
			os_move_mem(rWtblBa.aucPeerAddress,  BaCtrl.PeerAddr, MAC_ADDR_LEN);
			rWtblBa.ucRstBaTid = BaCtrl.Tid;
			rWtblBa.ucRstBaSel = RST_BA_MAC_TID_MATCH;
			rWtblBa.ucStartRstBaSb = 1;
			CmdExtWtblUpdate(pAd, BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
		}
	} else {
		if (BaCtrl.isAdd) {
			INT idx = 0;
			/* Clear WTBL2. SN: Direct Updating */
			rWtblBa.u2Sn = BaCtrl.Sn;

			/*get ba win size from range */
			while (ba_range[idx] < BaCtrl.BaWinSize) {
				if (idx == 7)
					break;

				idx++;
			};

			if (ba_range[idx] > BaCtrl.BaWinSize)
				idx--;

			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.ucBaWinSizeIdx = idx;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 1;
		} else {
			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.ucBaWinSizeIdx = 0;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 0;
		}

		CmdExtWtblUpdate(pAd, BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
	}
}


INT32 MtAsicUpdateBASessionByFw(
	IN PRTMP_ADAPTER pAd,
	IN MT_BA_CTRL_T BaCtrl)
{
	INT32				Status = NDIS_STATUS_FAILURE;
	CMD_WTBL_BA_T		rWtblBa = {0};
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT16 *ba_range = cap->ppdu.ba_range;

	if (BaCtrl.Tid > 7) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "unknown tid(%d)\n", BaCtrl.Tid);
		return Status;
	}

	rWtblBa.u2Tag = WTBL_BA;
	rWtblBa.u2Length = sizeof(CMD_WTBL_BA_T);
	rWtblBa.ucTid = BaCtrl.Tid;
	rWtblBa.ucBaSessionType = BaCtrl.BaSessionType;

	if (BaCtrl.BaSessionType == BA_SESSION_RECP) {
		/* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
		if (BaCtrl.isAdd) {
			rWtblBa.ucBandIdx = BaCtrl.band_idx;
			os_move_mem(rWtblBa.aucPeerAddress,  BaCtrl.PeerAddr, MAC_ADDR_LEN);
			rWtblBa.ucRstBaTid = BaCtrl.Tid;
			rWtblBa.ucRstBaSel = RST_BA_MAC_TID_MATCH;
			rWtblBa.ucStartRstBaSb = 1;
			Status = CmdExtWtblUpdate(pAd, BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
		}

		/* TODO: Hanmin 7615, need rWtblBa.ucBaEn=0 for delete? */
	} else {
		if (BaCtrl.isAdd) {
			INT idx = 0;
			/* Clear WTBL2. SN: Direct Updating */
			rWtblBa.u2Sn = BaCtrl.Sn;

			/* Get ba win size from range */
			while (BaCtrl.BaWinSize > ba_range[idx]) {
				if (idx == (MT_DMAC_BA_AGG_RANGE - 1))
					break;

				idx++;
			};

			if ((idx > 0) && (ba_range[idx] > BaCtrl.BaWinSize))
				idx--;

			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.ucBaWinSizeIdx = idx;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 1;
		} else {
			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.ucBaWinSizeIdx = 0;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 0;
		}

		Status = CmdExtWtblUpdate(pAd, BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
	}

	return Status;
}

/* offload BA winsize index calculation to FW */
INT32 MtAsicUpdateBASessionOffloadByFw(
	IN PRTMP_ADAPTER pAd,
	IN MT_BA_CTRL_T BaCtrl)
{
	INT32				Status = NDIS_STATUS_FAILURE;
	CMD_WTBL_BA_T		rWtblBa = {0};

	if (BaCtrl.Tid > 7) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "unknown tid(%d)\n", BaCtrl.Tid);
		return Status;
	}

	rWtblBa.u2Tag = WTBL_BA;
	rWtblBa.u2Length = sizeof(CMD_WTBL_BA_T);
	rWtblBa.ucTid = BaCtrl.Tid;
	rWtblBa.ucBaSessionType = BaCtrl.BaSessionType;

	if (BaCtrl.BaSessionType == BA_SESSION_RECP) {
		/* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
		if (BaCtrl.isAdd) {
			rWtblBa.ucBandIdx = BaCtrl.band_idx;
			os_move_mem(rWtblBa.aucPeerAddress,  BaCtrl.PeerAddr, MAC_ADDR_LEN);
			rWtblBa.ucRstBaTid = BaCtrl.Tid;
			rWtblBa.ucRstBaSel = RST_BA_MAC_TID_MATCH;
			rWtblBa.ucStartRstBaSb = 1;
			/* After AX chip, need RX Ba Win size to determine BA_MODE in WTBL */
			rWtblBa.u2BaWinSize = BaCtrl.BaWinSize;
			Status = CmdExtWtblUpdate(pAd, BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
		}

		/* TODO: Hanmin 7615, need rWtblBa.ucBaEn=0 for delete? */
	} else {
		if (BaCtrl.isAdd) {
			/* Clear WTBL2. SN: Direct Updating */
			rWtblBa.u2Sn = BaCtrl.Sn;
			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.u2BaWinSize = BaCtrl.BaWinSize;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 1;
		} else {
			/* Clear BA_WIN_SIZE and set new value to it */
			rWtblBa.u2BaWinSize = 0;
			/* Enable BA_EN */
			rWtblBa.ucBaEn = 0;
		}

		Status = CmdExtWtblUpdate(pAd, BaCtrl.Wcid, SET_WTBL, &rWtblBa, sizeof(rWtblBa));
	}

	return Status;
}

VOID MtAsicAddRemoveKeyTabByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _ASIC_SEC_INFO *pInfo)
{
	CMD_WTBL_SECURITY_KEY_T *wtbl_security_key = NULL;
	UINT32 cmd_len = 0;

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wcid=%d, Operation=%d, Direction=%d\n",
			 pInfo->Wcid, pInfo->Operation, pInfo->Direction);

	if (chip_fill_key_install_cmd(pAd->hdev_ctrl, pInfo, WTBL_SEC_KEY_METHOD, (VOID **)&wtbl_security_key, &cmd_len) != NDIS_STATUS_SUCCESS) {
		if (wtbl_security_key) {
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "key install cmd failed free security key\n");
			os_free_mem(wtbl_security_key);
		}
		return;
	}

#ifdef SW_CONNECT_SUPPORT
	/* Skip to set key when wcid is Dummy Wcid, when S/W mode on */
	if (!HcIsDummyWcid(pAd, pInfo->Wcid))
#endif /* SW_CONNECT_SUPPORT */
	{
		CmdExtWtblUpdate(pAd, pInfo->Wcid, SET_WTBL, (PUCHAR)wtbl_security_key, cmd_len);
	}

	os_free_mem(wtbl_security_key);
}


VOID MtAsicSetSMPSByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	IN UCHAR Smps)
{
	CMD_WTBL_SMPS_T	CmdWtblSmPs = {0};

	CmdWtblSmPs.u2Tag = WTBL_SMPS;
	CmdWtblSmPs.u2Length = sizeof(CMD_WTBL_SMPS_T);
	CmdWtblSmPs.ucSmPs = Smps;
	CmdExtWtblUpdate(pAd, Wcid, SET_WTBL, (PUCHAR)&CmdWtblSmPs, sizeof(CMD_WTBL_SMPS_T));
}

VOID MtAsicGetTxTscByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT32 pn_type_mask,
	OUT UCHAR * pTxTsc)
{
	CMD_WTBL_PN_T cmdWtblPn[MAX_TSC_TYPE];
	USHORT wcid = 0;
	UCHAR tsc_cnt = 0;

	os_zero_mem(cmdWtblPn, MAX_TSC_TYPE * sizeof(CMD_WTBL_PN_T));

	if (pn_type_mask & TSC_TYPE_GTK_PN_MASK) {
		GET_GroupKey_WCID(wdev, wcid);
		cmdWtblPn[tsc_cnt].u2Tag = WTBL_PN;
		cmdWtblPn[tsc_cnt].u2Length = sizeof(CMD_WTBL_PN_T);
		cmdWtblPn[tsc_cnt].ucTscType = TSC_TYPE_GTK_PN;
		tsc_cnt++;
	}

	if (pn_type_mask & TSC_TYPE_IGTK_PN_MASK) {
		GET_GroupKey_WCID(wdev, wcid);
		cmdWtblPn[tsc_cnt].u2Tag = WTBL_PN;
		cmdWtblPn[tsc_cnt].u2Length = sizeof(CMD_WTBL_PN_T);
		cmdWtblPn[tsc_cnt].ucTscType = TSC_TYPE_IGTK_PN;
		tsc_cnt++;
	}

	if (pn_type_mask & TSC_TYPE_BIGTK_PN_MASK) {
		GET_GroupKey_WCID(wdev, wcid);
		cmdWtblPn[tsc_cnt].u2Tag = WTBL_PN;
		cmdWtblPn[tsc_cnt].u2Length = sizeof(CMD_WTBL_PN_T);
		cmdWtblPn[tsc_cnt].ucTscType = TSC_TYPE_BIGTK_PN;
		tsc_cnt++;
	}
	CmdExtWtblUpdate(pAd, wcid, QUERY_WTBL, (PUCHAR)&cmdWtblPn, tsc_cnt * sizeof(CMD_WTBL_PN_T));

	tsc_cnt = 0;

	if (pn_type_mask & TSC_TYPE_GTK_PN_MASK) {
		os_move_mem(&pTxTsc[6 * tsc_cnt], cmdWtblPn[tsc_cnt].aucPn, 6);
		hex_dump_with_cat_and_lvl("gtk pn:", &pTxTsc[6 * tsc_cnt],
						6, DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_DEBUG);
		tsc_cnt++;
	}

	if (pn_type_mask & TSC_TYPE_IGTK_PN_MASK) {
		os_move_mem(&pTxTsc[6 * tsc_cnt], cmdWtblPn[tsc_cnt].aucPn, 6);
		hex_dump_with_cat_and_lvl("igtk pn:", &pTxTsc[6 * tsc_cnt],
						6, DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_DEBUG);
		tsc_cnt++;
	}

	if (pn_type_mask & TSC_TYPE_BIGTK_PN_MASK) {
		os_move_mem(&pTxTsc[6 * tsc_cnt], cmdWtblPn[tsc_cnt].aucPn, 6);
		hex_dump_with_cat_and_lvl("bigtk pn:", &pTxTsc[6 * tsc_cnt],
						6, DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_DEBUG);
		tsc_cnt++;
	}
}

#ifdef ZERO_LOSS_CSA_SUPPORT
UINT8 mtf_read_skip_tx(IN struct _RTMP_ADAPTER *pAd, UINT16 wcid)
{
	CMD_WTBL_GENERIC_T cmdWtblGeneric = {0};

	cmdWtblGeneric.u2Tag = WTBL_GENERIC;
	cmdWtblGeneric.u2Length = sizeof(CMD_WTBL_GENERIC_T);
	/**read wtbl*/
	CmdExtWtblUpdate(pAd, wcid, QUERY_WTBL, &cmdWtblGeneric, sizeof(CMD_WTBL_GENERIC_T));
	return cmdWtblGeneric.ucSkipTx;
}

VOID mtf_update_skip_tx(IN struct _RTMP_ADAPTER *pAd, UINT16 wcid, UINT8 set)
{
	CMD_WTBL_GENERIC_T cmdWtblGeneric = {0};

	cmdWtblGeneric.u2Tag = WTBL_GENERIC;
	cmdWtblGeneric.u2Length = sizeof(CMD_WTBL_GENERIC_T);
	/*read wtbl*/
	CmdExtWtblUpdate(pAd, wcid, QUERY_WTBL, &cmdWtblGeneric, sizeof(CMD_WTBL_GENERIC_T));
	/*update skip tx bit*/
	cmdWtblGeneric.ucSkipTx = set;
	/*write back to wtbl*/
	CmdExtWtblUpdate(pAd, wcid, SET_WTBL, &cmdWtblGeneric, sizeof(CMD_WTBL_GENERIC_T));
}
#endif /*ZERO_LOSS_CSA_SUPPORT*/

VOID mt_wtbltlv_debug(RTMP_ADAPTER *pAd, UINT16 u2Wcid, UCHAR ucCmdId, UCHAR ucAtion, union _wtbl_debug_u *debug_u)
{
	/* tag 0 */
	if (ucCmdId == WTBL_GENERIC) {
		debug_u->wtbl_generic_t.u2Tag = WTBL_GENERIC;
		debug_u->wtbl_generic_t.u2Length = sizeof(CMD_WTBL_GENERIC_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			UCHAR TestMac[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

			os_move_mem(debug_u->wtbl_generic_t.aucPeerAddress, TestMac, MAC_ADDR_LEN);
			debug_u->wtbl_generic_t.ucMUARIndex = 0x0;
			debug_u->wtbl_generic_t.ucSkipTx = 0;
			debug_u->wtbl_generic_t.ucCfAck = 0;
			debug_u->wtbl_generic_t.ucQos = 0;
			debug_u->wtbl_generic_t.ucMesh = 0;
			debug_u->wtbl_generic_t.ucAdm = 0;
			debug_u->wtbl_generic_t.u2PartialAID = 0;
			debug_u->wtbl_generic_t.ucBafEn = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_generic_t, sizeof(CMD_WTBL_GENERIC_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			UCHAR TestMac[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

			os_move_mem(debug_u->wtbl_generic_t.aucPeerAddress, TestMac, MAC_ADDR_LEN);
			debug_u->wtbl_generic_t.ucMUARIndex = 0x0e;
			debug_u->wtbl_generic_t.ucSkipTx = 1;
			debug_u->wtbl_generic_t.ucCfAck = 1;
			debug_u->wtbl_generic_t.ucQos = 1;
			debug_u->wtbl_generic_t.ucMesh = 1;
			debug_u->wtbl_generic_t.ucAdm = 1;
			debug_u->wtbl_generic_t.u2PartialAID = 32;
			debug_u->wtbl_generic_t.ucBafEn = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_generic_t, sizeof(CMD_WTBL_GENERIC_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_generic_t, sizeof(CMD_WTBL_GENERIC_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 1 */
	if (ucCmdId == WTBL_RX) {
		debug_u->wtbl_rx_t.u2Tag = WTBL_RX;
		debug_u->wtbl_rx_t.u2Length = sizeof(CMD_WTBL_RX_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_rx_t.ucRcid = 0;
			debug_u->wtbl_rx_t.ucRca1 = 0;
			debug_u->wtbl_rx_t.ucRca2 = 0;
			debug_u->wtbl_rx_t.ucRv = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_rx_t, sizeof(CMD_WTBL_RX_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_rx_t.ucRcid = 1;
			debug_u->wtbl_rx_t.ucRca1 = 1;
			debug_u->wtbl_rx_t.ucRca2 = 1;
			debug_u->wtbl_rx_t.ucRv = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_rx_t, sizeof(CMD_WTBL_RX_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_rx_t, sizeof(CMD_WTBL_RX_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 2 */
	if (ucCmdId == WTBL_HT) {
		debug_u->wtbl_ht_t.u2Tag = WTBL_HT;
		debug_u->wtbl_ht_t.u2Length = sizeof(CMD_WTBL_HT_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_ht_t.ucHt = 0;
			debug_u->wtbl_ht_t.ucLdpc = 0;
			debug_u->wtbl_ht_t.ucAf = 0;
			debug_u->wtbl_ht_t.ucMm = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_ht_t, sizeof(CMD_WTBL_HT_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_ht_t.ucHt = 1;
			debug_u->wtbl_ht_t.ucLdpc = 1;
			debug_u->wtbl_ht_t.ucAf = 1;
			debug_u->wtbl_ht_t.ucMm = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_ht_t, sizeof(CMD_WTBL_HT_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_ht_t, sizeof(CMD_WTBL_HT_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 3 */
	if (ucCmdId == WTBL_VHT) {
		debug_u->wtbl_vht_t.u2Tag = WTBL_VHT;
		debug_u->wtbl_vht_t.u2Length = sizeof(CMD_WTBL_VHT_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_vht_t.ucLdpcVht = 0;
			debug_u->wtbl_vht_t.ucDynBw = 0;
			debug_u->wtbl_vht_t.ucVht = 0;
			debug_u->wtbl_vht_t.ucTxopPsCap = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_vht_t, sizeof(CMD_WTBL_VHT_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_vht_t.ucLdpcVht = 1;
			debug_u->wtbl_vht_t.ucDynBw = 1;
			debug_u->wtbl_vht_t.ucVht = 1;
			debug_u->wtbl_vht_t.ucTxopPsCap = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_vht_t, sizeof(CMD_WTBL_VHT_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_vht_t, sizeof(CMD_WTBL_VHT_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 4 */
	if (ucCmdId == WTBL_PEER_PS) {
		debug_u->wtbl_peer_ps_t.u2Tag = WTBL_PEER_PS;
		debug_u->wtbl_peer_ps_t.u2Length = sizeof(CMD_WTBL_PEER_PS_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_peer_ps_t.ucDuIPsm = 0;
			debug_u->wtbl_peer_ps_t.ucIPsm = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_peer_ps_t, sizeof(CMD_WTBL_PEER_PS_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_peer_ps_t.ucDuIPsm = 1;
			debug_u->wtbl_peer_ps_t.ucIPsm = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_peer_ps_t, sizeof(CMD_WTBL_PEER_PS_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_peer_ps_t, sizeof(CMD_WTBL_PEER_PS_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 5 */
	if (ucCmdId == WTBL_TX_PS) {
		debug_u->wtbl_tx_ps_t.u2Tag = WTBL_TX_PS;
		debug_u->wtbl_tx_ps_t.u2Length = sizeof(CMD_WTBL_TX_PS_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_tx_ps_t.ucTxPs = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_tx_ps_t, sizeof(CMD_WTBL_TX_PS_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_tx_ps_t.ucTxPs = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_tx_ps_t, sizeof(CMD_WTBL_TX_PS_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_tx_ps_t, sizeof(CMD_WTBL_TX_PS_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 6 */
	if (ucCmdId == WTBL_HDR_TRANS) {
		debug_u->wtbl_hdr_trans_t.u2Tag = WTBL_HDR_TRANS;
		debug_u->wtbl_hdr_trans_t.u2Length = sizeof(CMD_WTBL_HDR_TRANS_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_hdr_trans_t.ucTd = 0;
			debug_u->wtbl_hdr_trans_t.ucFd = 0;
			debug_u->wtbl_hdr_trans_t.ucDisRhtr = 0;
			CmdExtWtblUpdate(pAd,
					 u2Wcid,
					 SET_WTBL,
					 &debug_u->wtbl_hdr_trans_t,
					 sizeof(CMD_WTBL_HDR_TRANS_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_hdr_trans_t.ucTd = 1;
			debug_u->wtbl_hdr_trans_t.ucFd = 1;
			debug_u->wtbl_hdr_trans_t.ucDisRhtr = 1;
			CmdExtWtblUpdate(pAd,
					 u2Wcid,
					 SET_WTBL,
					 &debug_u->wtbl_hdr_trans_t,
					 sizeof(CMD_WTBL_HDR_TRANS_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd,
					 u2Wcid,
					 QUERY_WTBL,
					 &debug_u->wtbl_hdr_trans_t,
					 sizeof(CMD_WTBL_HDR_TRANS_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 7 security */
	/* tag 8 BA */

	/* tag 9 */
	if (ucCmdId == WTBL_RDG) {
		debug_u->wtbl_rdg_t.u2Tag = WTBL_RDG;
		debug_u->wtbl_rdg_t.u2Length = sizeof(CMD_WTBL_RDG_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_rdg_t.ucRdgBa = 0;
			debug_u->wtbl_rdg_t.ucR = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_rdg_t, sizeof(CMD_WTBL_RDG_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_rdg_t.ucRdgBa = 1;
			debug_u->wtbl_rdg_t.ucR = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_rdg_t, sizeof(CMD_WTBL_RDG_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_rdg_t, sizeof(CMD_WTBL_RDG_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 10 */
	if (ucCmdId == WTBL_PROTECTION) {
		debug_u->wtbl_prot_t.u2Tag = WTBL_PROTECTION;
		debug_u->wtbl_prot_t.u2Length = sizeof(CMD_WTBL_PROTECTION_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_prot_t.ucRts = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_prot_t, sizeof(CMD_WTBL_PROTECTION_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_prot_t.ucRts = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_prot_t, sizeof(CMD_WTBL_PROTECTION_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_prot_t, sizeof(CMD_WTBL_PROTECTION_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 11 */
	if (ucCmdId == WTBL_CLEAR) {
		debug_u->wtbl_clear_t.u2Tag = WTBL_CLEAR;
		debug_u->wtbl_clear_t.u2Length = sizeof(CMD_WTBL_CLEAR_T);

		if (ucAtion == 0) {
			/* Set to 0 */
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_clear_t.ucClear = ((0 << 1) |
							 (1 << 1) |
							 (1 << 2) |
							 (1 << 3) |
							 (1 << 4) |
							 (1 << 5));
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_clear_t, sizeof(CMD_WTBL_CLEAR_T));
		} else if (ucAtion == 2) {
			/* query */
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 12 */
	if (ucCmdId == WTBL_BF) {
		debug_u->wtbl_bf_t.u2Tag = WTBL_BF;
		debug_u->wtbl_bf_t.u2Length = sizeof(CMD_WTBL_BF_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_bf_t.ucTiBf = 0;
			debug_u->wtbl_bf_t.ucTeBf = 0;
			debug_u->wtbl_bf_t.ucTibfVht = 0;
			debug_u->wtbl_bf_t.ucTebfVht = 0;
			debug_u->wtbl_bf_t.ucGid = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_bf_t, sizeof(CMD_WTBL_BF_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_bf_t.ucTiBf = 1;
			debug_u->wtbl_bf_t.ucTeBf = 1;
			debug_u->wtbl_bf_t.ucTibfVht = 1;
			debug_u->wtbl_bf_t.ucTebfVht = 1;
			debug_u->wtbl_bf_t.ucGid = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_bf_t, sizeof(CMD_WTBL_BF_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_bf_t, sizeof(CMD_WTBL_BF_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 13 */
	if (ucCmdId == WTBL_SMPS) {
		debug_u->wtbl_smps_t.u2Tag = WTBL_SMPS;
		debug_u->wtbl_smps_t.u2Length = sizeof(CMD_WTBL_SMPS_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_smps_t.ucSmPs = 0;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_smps_t, sizeof(CMD_WTBL_SMPS_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_smps_t.ucSmPs = 1;
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_smps_t, sizeof(CMD_WTBL_SMPS_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_smps_t, sizeof(CMD_WTBL_SMPS_T));
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 14 */
	if (ucCmdId == WTBL_RAW_DATA_RW) {
		debug_u->wtbl_raw_data_rw_t.u2Tag = WTBL_RAW_DATA_RW;
		debug_u->wtbl_raw_data_rw_t.u2Length = sizeof(CMD_WTBL_RAW_DATA_RW_T);

		if (ucAtion == 0) {
			/* Set to 0 */
			debug_u->wtbl_raw_data_rw_t.ucWtblIdx = 1;
			debug_u->wtbl_raw_data_rw_t.ucWhichDW = 0;
			debug_u->wtbl_raw_data_rw_t.u4DwMask = 0xffff00ff;
			debug_u->wtbl_raw_data_rw_t.u4DwValue = 0x12340078;
			CmdExtWtblUpdate(pAd,
					 u2Wcid,
					 SET_WTBL,
					 &debug_u->wtbl_raw_data_rw_t,
					 sizeof(CMD_WTBL_RAW_DATA_RW_T));
		} else if (ucAtion == 1) {
			/* Set to 1 */
			debug_u->wtbl_raw_data_rw_t.ucWtblIdx = 1;
			debug_u->wtbl_raw_data_rw_t.ucWhichDW = 0;
			debug_u->wtbl_raw_data_rw_t.u4DwMask = 0xffff00ff;
			debug_u->wtbl_raw_data_rw_t.u4DwValue = 0x12345678;
			CmdExtWtblUpdate(pAd,
					 u2Wcid,
					 SET_WTBL,
					 &debug_u->wtbl_raw_data_rw_t,
					 sizeof(CMD_WTBL_RAW_DATA_RW_T));
		} else if (ucAtion == 2) {
			/* query */
			debug_u->wtbl_raw_data_rw_t.ucWtblIdx = 1;
			CmdExtWtblUpdate(pAd,
					 u2Wcid,
					 QUERY_WTBL,
					 &debug_u->wtbl_raw_data_rw_t,
					 sizeof(CMD_WTBL_RAW_DATA_RW_T));
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"rWtblRawDataRw.u4DwValue(%x)\n",
					debug_u->wtbl_raw_data_rw_t.u4DwValue);
		} else
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Cmd Error\n");
	}

	/* tag 15 */
	if (ucCmdId == WTBL_PN) {
		debug_u->wtbl_pn_t.u2Tag = WTBL_PN;
		debug_u->wtbl_pn_t.u2Length = sizeof(CMD_WTBL_PN_T);

		if (ucAtion == 0) {
			/* Set to 0 */
		} else if (ucAtion == 1) {
			/* Set to 1 */
			os_fill_mem(debug_u->wtbl_pn_t.aucPn, 6, 0xf);
			CmdExtWtblUpdate(pAd, u2Wcid, SET_WTBL, &debug_u->wtbl_pn_t, sizeof(CMD_WTBL_PN_T));
		} else if (ucAtion == 2) {
			/* query */
			CmdExtWtblUpdate(pAd, u2Wcid, QUERY_WTBL, &debug_u->wtbl_pn_t, sizeof(CMD_WTBL_PN_T));
			hex_dump("WTBL_PN", debug_u->wtbl_pn_t.aucPn, 6);
		}
	}
}


VOID MtAsicUpdateProtectByFw(struct _RTMP_ADAPTER *ad, VOID *cookie)
{
	struct _EXT_CMD_UPDATE_PROTECT_T fw_protect;
	MT_PROTECT_CTRL_T *protect = (MT_PROTECT_CTRL_T *)cookie;

	os_zero_mem(&fw_protect, sizeof(fw_protect));
	fw_protect.ucProtectIdx = UPDATE_PROTECTION_CTRL;
	fw_protect.ucDbdcIdx = protect->band_idx;
	fw_protect.Data.rUpdateProtect.ucLongNav = protect->long_nav;
	fw_protect.Data.rUpdateProtect.ucMMProtect = protect->mix_mode;
	fw_protect.Data.rUpdateProtect.ucGFProtect = protect->gf;
	fw_protect.Data.rUpdateProtect.ucBW40Protect = protect->bw40;
	fw_protect.Data.rUpdateProtect.ucRifsProtect = protect->rifs;
	fw_protect.Data.rUpdateProtect.ucBW80Protect = protect->bw80;
	fw_protect.Data.rUpdateProtect.ucBW160Protect = protect->bw160;
	fw_protect.Data.rUpdateProtect.ucERProtectMask = protect->erp_mask;
	MtCmdUpdateProtect(ad, &fw_protect);
}


VOID MtAsicUpdateRtsThldByFw(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR pkt_num, UINT32 length)
{
	MT_RTS_THRESHOLD_T rts_thld = {0};

	rts_thld.band_idx = HcGetBandByWdev(wdev);
	rts_thld.pkt_num_thld = pkt_num;
	rts_thld.pkt_len_thld = length;

	if (MTK_REV_LT(pAd, MT7615, MT7615E3) && pAd->CommonCfg.dbdc_mode) {
		;/* DBDC does not support RTS setting */
	} else {
		struct _EXT_CMD_UPDATE_PROTECT_T fw_rts;

		os_zero_mem(&fw_rts, sizeof(fw_rts));
		fw_rts.ucProtectIdx = UPDATE_RTS_THRESHOLD;
		fw_rts.ucDbdcIdx = rts_thld.band_idx;
		fw_rts.Data.rUpdateRtsThld.u4RtsPktLenThreshold = cpu2le32(rts_thld.pkt_len_thld);
		fw_rts.Data.rUpdateRtsThld.u4RtsPktNumThreshold = cpu2le32(rts_thld.pkt_num_thld);
		MtCmdUpdateProtect(pAd, &fw_rts);
	}
}


INT MtAsicSetRDGByFw(RTMP_ADAPTER *pAd, MT_RDG_CTRL_T *Rdg)
{
	struct _EXT_CMD_RDG_CTRL_T fw_rdg;

	memset(&fw_rdg, 0, sizeof(struct _EXT_CMD_RDG_CTRL_T));
	fw_rdg.u4TxOP = Rdg->Txop;
	fw_rdg.ucLongNav = Rdg->LongNav;
	fw_rdg.ucInit = Rdg->Init;
	fw_rdg.ucResp = Rdg->Resp;
	WCID_SET_H_L(fw_rdg.ucWlanIdxHnVer, fw_rdg.ucWlanIdxL, Rdg->WlanIdx);
	fw_rdg.ucBand = Rdg->BandIdx;
	MtCmdSetRdg(pAd, &fw_rdg);
	return TRUE;
}


#ifdef DBDC_MODE
INT32  MtAsicGetDbdcCtrlByFw(RTMP_ADAPTER *pAd, BCTRL_INFO_T *pbInfo)
{
	UINT32 ret;
	UINT32 i = 0, j = 0;
	/*DBDC enable will not need BctrlEntries so minus 1*/
	pbInfo->TotalNum = 0;
	/*PTA*/
	pbInfo->BctrlEntries[i].Type = DBDC_TYPE_PTA;
	pbInfo->BctrlEntries[i].Index = 0;
	i++;
	/*MU*/
	pbInfo->BctrlEntries[i].Type = DBDC_TYPE_MU;
	pbInfo->BctrlEntries[i].Index = 0;
	i++;

	/*BF*/
	for (j = 0; j < 3; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_BF;
		pbInfo->BctrlEntries[i].Index = j;
		i++;
	}

	/*WMM*/
	for (j = 0; j < 4; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_WMM;
		pbInfo->BctrlEntries[i].Index = j;
		i++;
	}

	/*MGMT*/
	for (j = 0; j < 2; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_MGMT;
		pbInfo->BctrlEntries[i].Index = j;
		i++;
	}

	/*MBSS*/
	for (j = 0; j < 15; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_MBSS;
		pbInfo->BctrlEntries[i].Index = j;
		i++;
	}

	/*BSS*/
	for (j = 0; j < 5; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_BSS;
		pbInfo->BctrlEntries[i].Index = j;
		i++;
	}

	/*Repeater*/
	for (j = 0; j < 32; j++) {
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_REPEATER;
		pbInfo->BctrlEntries[i].Index = j;
		i++;
	}

	pbInfo->TotalNum = i;
	ret = MtCmdGetDbdcCtrl(pAd, pbInfo);
	return ret;
}


INT32 MtAsicSetDbdcCtrlByFw(RTMP_ADAPTER *pAd, BCTRL_INFO_T *pbInfo)
{
	UINT32 ret = 0;

	ret = MtCmdSetDbdcCtrl(pAd, pbInfo);
	return ret;
}

#endif /*DBDC_MODE*/

UINT32 MtAsicGetWmmParamByFw(RTMP_ADAPTER *pAd, UINT32 AcNum, UINT32 EdcaType)
{
	UINT32 ret, Value = 0;
	MT_EDCA_CTRL_T EdcaCtrl;

	os_zero_mem(&EdcaCtrl, sizeof(MT_EDCA_CTRL_T));
	EdcaCtrl.ucTotalNum = 1;
	EdcaCtrl.ucAction = EDCA_ACT_GET;
	EdcaCtrl.rAcParam[0].ucAcNum = AcNum;
	ret = MtCmdGetEdca(pAd, &EdcaCtrl);

	switch (EdcaType) {
	case WMM_PARAM_TXOP:
		Value = EdcaCtrl.rAcParam[0].u2Txop;
		break;

	case WMM_PARAM_AIFSN:
		Value = EdcaCtrl.rAcParam[0].ucAifs;
		break;

	case WMM_PARAM_CWMIN:
		Value = EdcaCtrl.rAcParam[0].ucWinMin;
		break;

	case WMM_PARAM_CWMAX:
		Value = EdcaCtrl.rAcParam[0].u2WinMax;
		break;

	default:
		Value = 0xdeadbeef;
		break;
	}

	return Value;
}

INT MtAsicSetWmmParam(
	RTMP_ADAPTER *pAd,
	UCHAR idx,
	UINT32 AcNum,
	UINT32 EdcaType,
	UINT32 EdcaValue)
{
	MT_EDCA_CTRL_T EdcaParam;
	P_TX_AC_PARAM_T pAcParam;
	UCHAR index = 0;

	/* Could write any queue by FW */
	if ((AcNum < 4) && (idx < 4))
		index = (idx * 4) + AcNum;
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Non-WMM Queue, WmmIdx/QueIdx=%d/%d!\n",
				  idx, AcNum);
		index = AcNum;
	}

	os_zero_mem(&EdcaParam, sizeof(MT_EDCA_CTRL_T));
	EdcaParam.ucTotalNum = 1;
	EdcaParam.ucAction = EDCA_ACT_SET;
	pAcParam = &EdcaParam.rAcParam[0];
	pAcParam->ucAcNum = (UINT8)index;

	switch (EdcaType) {
	case WMM_PARAM_TXOP:
		pAcParam->ucVaildBit = CMD_EDCA_TXOP_BIT;
		pAcParam->u2Txop = (UINT16)EdcaValue;
		break;

	case WMM_PARAM_AIFSN:
		pAcParam->ucVaildBit = CMD_EDCA_AIFS_BIT;
		pAcParam->ucAifs = (UINT8)EdcaValue;
		break;

	case WMM_PARAM_CWMIN:
		pAcParam->ucVaildBit = CMD_EDCA_WIN_MIN_BIT;
		pAcParam->ucWinMin = (UINT8)EdcaValue;
		break;

	case WMM_PARAM_CWMAX:
		pAcParam->ucVaildBit = CMD_EDCA_WIN_MAX_BIT;
		pAcParam->u2WinMax = (UINT16)EdcaValue;
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error type=%d\n", EdcaType);
		break;
	}
	MtCmdEdcaParameterSet(pAd, &EdcaParam);
	return NDIS_STATUS_SUCCESS;
}

#ifdef WIFI_UNIFIED_COMMAND
INT MtAsicUniCmdSetWmmParam(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR idx,
	UINT32 AcNum,
	UINT32 EdcaType,
	UINT32 EdcaValue)
{
	MT_EDCA_CTRL_T EdcaParam;
	P_TX_AC_PARAM_T pAcParam;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is NULL!\n");
		return NDIS_STATUS_FAILURE;
	}

	os_zero_mem(&EdcaParam, sizeof(MT_EDCA_CTRL_T));
	EdcaParam.ucTotalNum = 1;
	EdcaParam.ucAction = EDCA_ACT_SET;
	pAcParam = &EdcaParam.rAcParam[0];
	pAcParam->ucAcNum = (UINT8)AcNum;

	switch (EdcaType) {
	case WMM_PARAM_TXOP:
		pAcParam->ucVaildBit = CMD_EDCA_TXOP_BIT;
		pAcParam->u2Txop = (UINT16)EdcaValue;
		break;

	case WMM_PARAM_AIFSN:
		pAcParam->ucVaildBit = CMD_EDCA_AIFS_BIT;
		pAcParam->ucAifs = (UINT8)EdcaValue;
		break;

	case WMM_PARAM_CWMIN:
		pAcParam->ucVaildBit = CMD_EDCA_WIN_MIN_BIT;
		pAcParam->ucWinMin = (UINT8)EdcaValue;
		break;

	case WMM_PARAM_CWMAX:
		pAcParam->ucVaildBit = CMD_EDCA_WIN_MAX_BIT;
		pAcParam->u2WinMax = (UINT16)EdcaValue;
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " Error type=%d\n", EdcaType);
		break;
	}

	MtUniCmdEdcaParameterSet(pAd, wdev, EdcaParam);

	return NDIS_STATUS_SUCCESS;
}
#endif /* WIFI_UNIFIED_COMMAND */

VOID MtAsicSetEdcaParm(RTMP_ADAPTER *pAd, UCHAR idx, UCHAR tx_mode, PEDCA_PARM pEdcaParm)
{
	MT_EDCA_CTRL_T EdcaParam;
	P_TX_AC_PARAM_T pAcParam;
	UINT32 ac = 0, index = 0;

	os_zero_mem(&EdcaParam, sizeof(MT_EDCA_CTRL_T));

	if ((pEdcaParm != NULL) && (pEdcaParm->bValid != FALSE)) {
		EdcaParam.ucTotalNum = CMD_EDCA_AC_MAX;
		EdcaParam.ucTxModeValid = TRUE;
		EdcaParam.ucTxMode = tx_mode;

		for (ac = 0; ac < CMD_EDCA_AC_MAX;  ac++) {
			index = idx*4+ac;
			pAcParam = &EdcaParam.rAcParam[ac];
			pAcParam->ucVaildBit = CMD_EDCA_ALL_BITS;
			pAcParam->ucAcNum =  asic_get_hwq_from_ac(pAd, idx, ac);
			pAcParam->ucAifs = pEdcaParm->Aifsn[ac];
			pAcParam->ucWinMin = pEdcaParm->Cwmin[ac];
			pAcParam->u2WinMax = pEdcaParm->Cwmax[ac];
			pAcParam->u2Txop = pEdcaParm->Txop[ac];
		}
	}
	MtCmdEdcaParameterSet(pAd, &EdcaParam);
}

#ifdef WIFI_UNIFIED_COMMAND
VOID MtAsicUniCmdSetEdcaParm(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR idx, UCHAR tx_mode, PEDCA_PARM pEdcaParm)
{
	MT_EDCA_CTRL_T EdcaParam;
	P_TX_AC_PARAM_T pAcParam;
	UINT32 ac = 0;

	os_zero_mem(&EdcaParam, sizeof(MT_EDCA_CTRL_T));

	if ((pEdcaParm != NULL) && (pEdcaParm->bValid != FALSE)) {
		EdcaParam.ucTotalNum = CMD_EDCA_AC_MAX;
		EdcaParam.ucTxModeValid = TRUE;
		EdcaParam.ucTxMode = tx_mode;

		for (ac = 0; ac < CMD_EDCA_AC_MAX;  ac++) {
			pAcParam = &EdcaParam.rAcParam[ac];
			pAcParam->ucVaildBit = CMD_EDCA_ALL_BITS;
			pAcParam->ucAcNum = ac;
			pAcParam->ucAifs = pEdcaParm->Aifsn[ac];
			pAcParam->ucWinMin = pEdcaParm->Cwmin[ac];
			pAcParam->u2WinMax = pEdcaParm->Cwmax[ac];
			pAcParam->u2Txop = pEdcaParm->Txop[ac];
		}
	}

	MtUniCmdEdcaParameterSet(pAd, wdev, EdcaParam);
}
#endif /* WIFI_UNIFIED_COMMAND */

INT MtAsicGetTsfTimeByFirmware(
	RTMP_ADAPTER *pAd,
	UINT32 *high_part,
	UINT32 *low_part,
	UCHAR HwBssidIdx)
{
	TSF_RESULT_T TsfResult;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	os_zero_mem(&TsfResult, sizeof(TSF_RESULT_T));
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdGetTsfTime(pAd, HwBssidIdx, &TsfResult);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdGetTsfTime(pAd, HwBssidIdx, &TsfResult);
	*high_part = TsfResult.u4TsfBit63_32;
	*low_part = TsfResult.u4TsfBit0_31;
	return TRUE;
}

VOID MtAsicSetSlotTime(RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime, UCHAR BandIdx)
{
	UINT32 RifsTime = RIFS_TIME;
	UINT32 EifsTime = EIFS_TIME;

	MtCmdSlotTimeSet(pAd, (UINT8)SlotTime, (UINT8)SifsTime, (UINT8)RifsTime, (UINT16)EifsTime, BandIdx);
}
#ifdef WIFI_UNIFIED_COMMAND
VOID MtAsicUniCmdSetSlotTime(struct _RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime, struct wifi_dev *wdev)
{
	UINT32 RifsTime = RIFS_TIME;
	UINT32 EifsTime = EIFS_TIME;

	MtUniCmdSlotTimeSet(pAd, (UINT16)SlotTime, (UINT16)SifsTime, (UINT16)RifsTime, (UINT16)EifsTime, wdev);
}
#endif /* WIFI_UNIFIED_COMMAND */

UINT32 MtAsicGetChBusyCntByFw(RTMP_ADAPTER *pAd, UCHAR ch_idx)
{
	UINT32 msdr16 = 0, ret;

	ret = MtCmdGetChBusyCnt(pAd, ch_idx, &msdr16);
	return msdr16;
}

UINT32 MtAsicGetCCACnt(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	RTMP_MIB_PAIR Reg[1];
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	NdisZeroMemory(Reg, sizeof(Reg));

	Reg[0].Counter = MIB_CNT_CCA_NAV_TX_TIME; /* M0SDR9 */
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdMib(pAd, BandIdx, Reg, 1);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdMultipleMibRegAccessRead(pAd, BandIdx, Reg, 1);

	return (UINT32)Reg[0].Value;
}

INT32 MtAsicSetMacTxRxByFw(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx)
{
	UINT32 ret;

	ret = MtCmdSetMacTxRx(pAd, BandIdx, Enable);
	return ret;
}

INT32 MtAsicSetRxvFilter(RTMP_ADAPTER *pAd, BOOLEAN Enable, UCHAR BandIdx)
{
	UINT32 ret;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support)
		ret = UniCmdRxvCtrl(pAd, BandIdx, ASIC_MAC_TXRX_RXV, Enable);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		ret = MtCmdSetRxvFilter(pAd, BandIdx, Enable);
	return ret;
}

VOID MtAsicDisableSyncByFw(struct _RTMP_ADAPTER *pAd, UCHAR HWBssidIdx)
{
	struct wifi_dev *wdev = NULL;
	UCHAR i;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev != NULL) {
			if (wdev->OmacIdx == HWBssidIdx)
				break;
		} else
			continue;
	}

	/* ASSERT(wdev != NULL); */

	if (wdev == NULL)
		return;

	if (WDEV_BSS_STATE(wdev) == BSS_INIT) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: BssInfo idx (%d) is INIT currently!!!\n",
				 __func__, wdev->bss_info_argument.ucBssIndex));
		return;
	}

	WDEV_BSS_STATE(wdev) = BSS_INITED;
	CmdSetSyncModeByBssInfoUpdate(pAd, &wdev->bss_info_argument);
}

VOID MtAsicEnableBssSyncByFw(
	struct _RTMP_ADAPTER *pAd,
	USHORT BeaconPeriod,
	UCHAR HWBssidIdx,
	UCHAR OPMode)
{
	struct wifi_dev *wdev = NULL;
	UCHAR i;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev != NULL) {
			if (wdev->OmacIdx == HWBssidIdx)
				break;
		} else
			continue;
	}

	/* ASSERT(wdev != NULL); */

	if (wdev == NULL)
		return;

	if (WDEV_BSS_STATE(wdev) == BSS_INIT) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: BssInfo idx (%d) is INIT currently!!!\n",
				 __func__, wdev->bss_info_argument.ucBssIndex));
		return;
	}

	WDEV_BSS_STATE(wdev) = BSS_ACTIVE;
	CmdSetSyncModeByBssInfoUpdate(pAd, &wdev->bss_info_argument);
}

#if defined(MT_MAC) && defined(TXBF_SUPPORT)
/* STARec Info */
INT32 MtAsicSetAid(
	RTMP_ADAPTER *pAd,
	UINT16 Aid,
	UINT8 OmacIdx)
{
	return CmdETxBfAidSetting(pAd, Aid, 0, OmacIdx);
}
#endif

#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
/* TODO: Carter/Star for Repeater can support DBDC, after define STA/APCLI/Repeater */
INT MtAsicSetReptFuncEnableByFw(RTMP_ADAPTER *pAd, BOOLEAN bEnable, UCHAR band_idx)
{
	EXT_CMD_MUAR_T config_muar;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */
	NdisZeroMemory(&config_muar, sizeof(EXT_CMD_MUAR_T));

	if (bEnable == TRUE)
		config_muar.ucMuarModeSel = MUAR_REPEATER;
	else
		config_muar.ucMuarModeSel = MUAR_NORMAL;
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	config_muar.ucBand = band_idx;
#endif

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdMuarConfigSet(pAd, (UCHAR *)&config_muar);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdMuarConfigSet(pAd, (UCHAR *)&config_muar);
	return TRUE;
}

VOID MtAsicInsertRepeaterEntryByFw(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CliIdx,
	IN PUCHAR pAddr)
{
	UCHAR *pdata = NULL;
	UCHAR muar_idx = 0;
	EXT_CMD_MUAR_T config_muar;
	EXT_CMD_MUAR_MULTI_ENTRY_T muar_entry;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#ifdef WIFI_UNIFIED_COMMAND
	struct wifi_dev *wdev = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	NdisZeroMemory(&config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisZeroMemory(&muar_entry, sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	os_alloc_mem(pAd,
				 (UCHAR **)&pdata,
				 sizeof(EXT_CMD_MUAR_T) + sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));

	pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
	config_muar.ucMuarModeSel = MUAR_REPEATER;
	config_muar.ucEntryCnt = 1;
	config_muar.ucAccessMode = MUAR_WRITE;
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	config_muar.ucBand = HcGetBandByWdev(&pReptEntry->wdev);
#endif
	if (ReptGetMuarIdxByCliIdx(pAd, CliIdx, &muar_idx) == FALSE)
		goto done;
	muar_entry.ucMuarIdx = muar_idx;
#ifdef WIFI_UNIFIED_COMMAND
	wdev = pReptEntry->main_wdev;
	muar_entry.ucBssid = wdev->OmacIdx;
#endif /* WIFI_UNIFIED_COMMAND */
	COPY_MAC_ADDR(muar_entry.aucMacAddr, pAddr);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\n"MACSTR"-CliIdx(%d),MuarIdx(%d)\n",
		 MAC2STR(pAddr), CliIdx, muar_entry.ucMuarIdx);
	NdisMoveMemory(pdata, &config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisMoveMemory(pdata + sizeof(EXT_CMD_MUAR_T),
				   &muar_entry,
				   sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdMuarConfigSet(pAd, (UCHAR *)pdata);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdMuarConfigSet(pAd, (UCHAR *)pdata);
done:
	os_free_mem(pdata);
}


VOID MtAsicRemoveRepeaterEntryByFw(RTMP_ADAPTER *pAd, UCHAR CliIdx)
{
	UCHAR *pdata = NULL;
	UCHAR *ptr = NULL;
	UCHAR i = 0;
	UCHAR zeroMac[MAC_ADDR_LEN] = {0};
	UCHAR muar_idx = 0;
	EXT_CMD_MUAR_T config_muar;
	EXT_CMD_MUAR_MULTI_ENTRY_T muar_entry;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#ifdef WIFI_UNIFIED_COMMAND
	struct wifi_dev *wdev = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	NdisZeroMemory(&config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisZeroMemory(&muar_entry, sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
	config_muar.ucMuarModeSel = MUAR_REPEATER;
	config_muar.ucEntryCnt = 2;
	config_muar.ucAccessMode = MUAR_WRITE;
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	config_muar.ucBand = HcGetBandByWdev(&pReptEntry->wdev);
#endif
	os_alloc_mem(pAd,
				 (UCHAR **)&pdata,
				 sizeof(EXT_CMD_MUAR_T) +
				 (config_muar.ucEntryCnt * sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T)));

	NdisMoveMemory(pdata, &config_muar, sizeof(EXT_CMD_MUAR_T));
	ptr = pdata + sizeof(EXT_CMD_MUAR_T);

	for (i = 0; i < config_muar.ucEntryCnt; i++) {
		if (ReptGetMuarIdxByCliIdx(pAd, CliIdx, &muar_idx) == FALSE)
			goto done;
		muar_entry.ucMuarIdx = muar_idx + i;
#ifdef WIFI_UNIFIED_COMMAND
		wdev = pReptEntry->main_wdev;
		muar_entry.ucBssid = wdev->OmacIdx;
#endif /* WIFI_UNIFIED_COMMAND */
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"i=%d,CliIdx(%d),MuarIdx(%d)\n",
			i, CliIdx, muar_entry.ucMuarIdx);
		COPY_MAC_ADDR(muar_entry.aucMacAddr, zeroMac);
		NdisMoveMemory(ptr,
					   &muar_entry,
					   sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
		ptr = ptr + sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T);
	}

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdMuarConfigSet(pAd, (UCHAR *)pdata);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdMuarConfigSet(pAd, (UCHAR *)pdata);
done:
	os_free_mem(pdata);
}

VOID MtAsicInsertRepeaterRootEntryByFw(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 Wcid,
	IN UCHAR *pAddr,
	IN UCHAR ReptCliIdx)
{
	UCHAR *pdata = NULL;
	UCHAR muar_idx = 0;
	EXT_CMD_MUAR_T config_muar;
	EXT_CMD_MUAR_MULTI_ENTRY_T muar_entry;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#ifdef WIFI_UNIFIED_COMMAND
	struct wifi_dev *wdev = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	NdisZeroMemory(&config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisZeroMemory(&muar_entry, sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	os_alloc_mem(pAd,
				 (UCHAR **)&pdata,
				 sizeof(EXT_CMD_MUAR_T) + sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));

	pReptEntry = &pAd->ApCfg.pRepeaterCliPool[ReptCliIdx];
	config_muar.ucMuarModeSel = MUAR_REPEATER;
	config_muar.ucEntryCnt = 1;
	config_muar.ucAccessMode = MUAR_WRITE;
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	config_muar.ucBand = HcGetBandByWdev(&pReptEntry->wdev);
#endif
	if (ReptGetMuarIdxByCliIdx(pAd, ReptCliIdx, &muar_idx) == FALSE)
		goto done;
	muar_entry.ucMuarIdx = muar_idx + 1;
#ifdef WIFI_UNIFIED_COMMAND
	wdev = pReptEntry->main_wdev;
	muar_entry.ucBssid = wdev->OmacIdx;
#endif /* WIFI_UNIFIED_COMMAND */
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"\n"MACSTR"-CliIdx(%d),MuarIdx(%d)\n",
		MAC2STR(pAddr), ReptCliIdx, muar_entry.ucMuarIdx);
	COPY_MAC_ADDR(muar_entry.aucMacAddr, pAddr);
	NdisMoveMemory(pdata, &config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisMoveMemory(pdata + sizeof(EXT_CMD_MUAR_T),
				   &muar_entry,
				   sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdMuarConfigSet(pAd, (UCHAR *)pdata);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdMuarConfigSet(pAd, (UCHAR *)pdata);
done:
	os_free_mem(pdata);
}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */

INT32 MtAsicRxHeaderTransCtl(RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan, BOOLEAN SwPcP)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support) {
		UNI_CMD_RX_HDR_TRAN_PARAM_T RxHdrTranParam;

		os_zero_mem(&RxHdrTranParam, sizeof(RxHdrTranParam));
		RxHdrTranParam.RxHdrTranEnable.fgEnable = En;
		RxHdrTranParam.RxHdrTranEnable.fgCheckBssid = ChkBssid;
		RxHdrTranParam.RxHdrTranEnable.ucTranslationMode = 0;
		RxHdrTranParam.RxHdrTranValid[UNI_CMD_RX_HDR_TRAN_ENABLE] = TRUE;

		RxHdrTranParam.RxHdrTranVlan.fgInsertVlan = InSVlan;
		RxHdrTranParam.RxHdrTranVlan.fgRemoveVlan = RmVlan;
		RxHdrTranParam.RxHdrTranVlan.fgUseQosTid = !SwPcP;
		RxHdrTranParam.RxHdrTranValid[UNI_CMD_RX_HDR_TRAN_VLAN_CONFIG] = TRUE;

		return UniCmdRxHdrTrans(pAd, &RxHdrTranParam);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdRxHdrTransUpdate(pAd, En, ChkBssid, InSVlan, RmVlan, SwPcP);
	}
}

INT32 MtAsicRxHeaderTaranBLCtl(RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType)
{
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->uni_cmd_support) {
		UNI_CMD_RX_HDR_TRAN_PARAM_T RxHdrTranParam;

		os_zero_mem(&RxHdrTranParam, sizeof(RxHdrTranParam));
		RxHdrTranParam.RxHdrTranBlackList.fgEnable = En;
		RxHdrTranParam.RxHdrTranBlackList.ucBlackListIdx = (UINT8)Index;
		RxHdrTranParam.RxHdrTranBlackList.u2EtherType = (UINT16)EthType;
		RxHdrTranParam.RxHdrTranValid[UNI_CMD_RX_HDR_TRAN_BLACKLIST_CONFIG] = TRUE;

		return UniCmdRxHdrTrans(pAd, &RxHdrTranParam);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		return CmdRxHdrTransBLUpdate(pAd, Index, En, EthType);
	}
}

#ifdef VLAN_SUPPORT
INT32 mt_asic_update_vlan_id_by_fw(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT8 omac_idx, UINT16 vid)
{
	return cmd_vlan_update(ad, band_idx, omac_idx, BSS_INFO_SET_VLAN_ID, vid);
}

INT32 mt_asic_update_vlan_priority_by_fw(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT8 omac_idx, UINT8 priority)
{
	return cmd_vlan_update(ad, band_idx, omac_idx, BSS_INFO_SET_VLAN_PRIORITY, priority);
}
#endif
