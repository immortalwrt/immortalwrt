/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_fw_uni_cmd.c
*/

#ifdef WIFI_UNIFIED_COMMAND
#include "rt_config.h"
#include "hdev/hdev.h"

BOOLEAN UniCmdCheckInitReady(struct _RTMP_ADAPTER *pAd)
{
	struct MCU_CTRL *Ctrl = &pAd->MCUCtrl;

	if (Ctrl && OS_TEST_BIT(MCU_INIT, &Ctrl->flags))
		return TRUE;
	else
		return FALSE;
}

struct cmd_msg *AndesAllocUniCmdMsg(struct _RTMP_ADAPTER *pAd, unsigned int length)
{
	return hif_mcu_alloc_msg(pAd, length, FALSE);
}

static INT32 UniCmdDevInfoActive(struct _RTMP_ADAPTER *pAd, UINT8 Active, UINT8 *OwnMacAddr, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_DEVINFO_ACTIVE_T pDevInfoActive = (P_UNI_CMD_DEVINFO_ACTIVE_T)pHandle;

	/* Fill TLV format */
	pDevInfoActive->u2Tag = UNI_CMD_DEVINFO_ACTIVE;
	pDevInfoActive->u2Length = sizeof(UNI_CMD_DEVINFO_ACTIVE_T);
#ifdef RT_BIG_ENDIAN
	pDevInfoActive->u2Tag = cpu2le16(pDevInfoActive->u2Tag);
	pDevInfoActive->u2Length = cpu2le16(pDevInfoActive->u2Length);
#endif /* RT_BIG_ENDIAN */
	pDevInfoActive->ucActive = Active;
	os_move_mem(pDevInfoActive->aucOwnMacAddr, OwnMacAddr, MAC_ADDR_LEN);

	return Ret;
}

static UNI_CMD_TAG_HANDLE_T UniCmdDevInfoTab[] = {
	{
		.u2CmdFeature = DEVINFO_ACTIVE_FEATURE,
		.u4StructSize = sizeof(UNI_CMD_DEVINFO_ACTIVE_T),
		.pfHandler = UniCmdDevInfoActive
	},
};

INT32 UniCmdDevInfoUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 u4EnableFeature)
{
	struct cmd_msg			*msg = NULL;
	INT32					Ret = NDIS_STATUS_SUCCESS;
	UINT8					i = 0;
	UINT16					u2TLVNumber = 0;
	PUCHAR					pTempBuf = NULL;
	PUCHAR					pNextHeadBuf = NULL;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4RealUseBufSize = 0;
	UINT32					u4SendBufSize = 0;
	UINT32					u4RemainingPayloadSize = 0;
	UINT32					u4ComCmdSize = 0;
	P_UNI_CMD_DEVINFO_T		pCmdDeviceInfoUpdate = NULL;
	RTMP_CHIP_CAP			*cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32 					DevInfoTabSize = (sizeof(UniCmdDevInfoTab) / sizeof(UniCmdDevInfoTab[0]));

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_DEVINFO_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < DevInfoTabSize; i++) {
		if (u4EnableFeature & UniCmdDevInfoTab[i].u2CmdFeature) {
			u4CmdNeedMaxBufSize += UniCmdDevInfoTab[i].u4StructSize;
		}
	}

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdDeviceInfoUpdate = (P_UNI_CMD_DEVINFO_T)pNextHeadBuf;
	pCmdDeviceInfoUpdate->ucOwnMacIdx = OwnMacIdx;
	pCmdDeviceInfoUpdate->ucDbdcIdx = BandIdx;
	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Traverse all support features */
	for (i = 0; i < DevInfoTabSize; i++) {
		switch (u4EnableFeature & UniCmdDevInfoTab[i].u2CmdFeature) {
		case DEVINFO_ACTIVE_FEATURE:
			if (UniCmdDevInfoTab[i].pfHandler != NULL) {
				Ret = ((PFN_DEVINFO_ACTIVE_HANDLE)(UniCmdDevInfoTab[i].pfHandler))(pAd, Active, OwnMacAddr, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += UniCmdDevInfoTab[i].u4StructSize;
					u2TLVNumber++;
				}
			}
			break;

		default:
			Ret = NDIS_STATUS_SUCCESS;
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"%s(): The hanlder of tag (0x%08x) not support!\n", __func__, u4EnableFeature);
			break;
		}

		if (Ret != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"The hanlder of tag (0x%08x) return fail!\n", UniCmdDevInfoTab[i].u2CmdFeature);
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Active = %d, OwnMacIdx = %d, band = %d (%02x-%02x-%02x-%02x-%02x-%02x), TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
		Active, OwnMacIdx, BandIdx, PRINT_MAC(OwnMacAddr), u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_DEVINFO);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if ((!bNeedFrag) || bLastFrag) {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
				SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
			} else {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			call_fw_cmd_notifieriers(WO_CMD_DEV_INFO, pAd, msg->net_pkt);
			Ret = AndesSendCmdMsg(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static INT32 UniCmdBssInfoBasic(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BSSINFO_BASIC_T pBssInfoBasic = (P_UNI_CMD_BSSINFO_BASIC_T)pHandle;

	/* Fill TLV format */
	pBssInfoBasic->u2Tag = UNI_CMD_BSSINFO_BASIC;
	pBssInfoBasic->u2Length = sizeof(UNI_CMD_BSSINFO_BASIC_T);
#ifdef RT_BIG_ENDIAN
	pBssInfoBasic->u2Tag = cpu2le16(pBssInfoBasic->u2Tag);
	pBssInfoBasic->u2Length = cpu2le16(pBssInfoBasic->u2Length);
#endif /* RT_BIG_ENDIAN */
	if (bss_info->bss_state >= BSS_ACTIVE)
		pBssInfoBasic->ucActive = TRUE;
	else
		pBssInfoBasic->ucActive = FALSE;
	pBssInfoBasic->ucOwnMacIdx = bss_info->OwnMacIdx;
	pBssInfoBasic->ucHwBSSIndex = (bss_info->OwnMacIdx > HW_BSSID_MAX) ? HW_BSSID_0 : bss_info->OwnMacIdx;
	pBssInfoBasic->ucDbdcIdx = bss_info->ucBandIdx;
	pBssInfoBasic->u4ConnectionType = cpu2le32(bss_info->u4ConnectionType);
	pBssInfoBasic->ucConnectionState = (bss_info->bss_state < BSS_ACTIVE) ? MEDIA_STATE_DISCONNECTED : MEDIA_STATE_CONNECTED;
	pBssInfoBasic->ucWmmIdx = bss_info->WmmIdx;
	os_move_mem(pBssInfoBasic->aucBSSID, bss_info->Bssid, MAC_ADDR_LEN);
	pBssInfoBasic->u2BcMcWlanidx = cpu2le16(bss_info->bmc_wlan_idx);
	pBssInfoBasic->u2BcnInterval = cpu2le16(bss_info->bcn_period);
	pBssInfoBasic->ucDtimPeriod = bss_info->dtim_period;
	pBssInfoBasic->ucPhyMode = bss_info->ucPhyMode;
	pBssInfoBasic->u2StaRecIdxOfAP = (bss_info->u4ConnectionType == CONNECTION_INFRA_STA) ? \
										cpu2le16(bss_info->peer_wlan_idx) : 0;
	pBssInfoBasic->u2NonHTBasicPhyType = 0;
	pBssInfoBasic->ucPhyModeExt = 0;
#ifdef DOT11_HE_AX
	if (WMODE_CAP_6G(bss_info->ucPhyMode))
		pBssInfoBasic->ucPhyModeExt |= BIT(0);
#endif /* DOT11_HE_AX */

	MTWF_DBG_NP(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "ucDbdcIdx = %d, u4ConnectionType = %d, ucActive = %d, u2BcnInterval = %d, ucWmmIdx = %d,ucDtimPeriod = %d, u2BcMcWlanidx = %d, ucPhyMode=%x, BSSID = %02x:%02x:%02x:%02x:%02x:%02x\n",
			  pBssInfoBasic->ucDbdcIdx,
			  pBssInfoBasic->u4ConnectionType,
			  pBssInfoBasic->ucActive,
			  le2cpu16(pBssInfoBasic->u2BcnInterval),
			  pBssInfoBasic->ucWmmIdx,
			  pBssInfoBasic->ucDtimPeriod,
			  le2cpu16(pBssInfoBasic->u2BcMcWlanidx),
			  pBssInfoBasic->ucPhyMode,
			  PRINT_MAC(pBssInfoBasic->aucBSSID));

	return Ret;
}

static INT32 UniCmdBssInfoRate(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BSSINFO_RATE_T pBssInfoRate = (P_UNI_CMD_BSSINFO_RATE_T)pHandle;

	if (bss_info->bss_state >= BSS_ACTIVE) {
		/* Fill TLV format */
		pBssInfoRate->u2Tag = UNI_CMD_BSSINFO_RATE;
		pBssInfoRate->u2Length = sizeof(UNI_CMD_BSSINFO_RATE_T);
#ifdef RT_BIG_ENDIAN
		pBssInfoRate->u2Tag = cpu2le16(pBssInfoRate->u2Tag);
		pBssInfoRate->u2Length = cpu2le16(pBssInfoRate->u2Length);
#endif /* RT_BIG_ENDIAN */

		pBssInfoRate->u2BcRate = cpu2le16((UINT16)(bss_info->BcTransmit.word));
		pBssInfoRate->u2McRate = cpu2le16((UINT16)(bss_info->McTransmit.word));
		pBssInfoRate->ucPreambleMode = OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	} else {
		Ret = NDIS_STATUS_FAILURE;
	}

#ifdef TXRX_STAT_SUPPORT
	{
		ULONG Multicast_Tx_Rate;

		pAd->ApCfg.MBSSID[bss_info->ucBssIndex].stat_bss.LastMulticastTxRate.word = bss_info->McTransmit.word;
		getRate(bss_info->McTransmit, &Multicast_Tx_Rate);
	}
#endif

	return Ret;
}

static INT32 UniCmdBssInfoSec(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BSSINFO_SEC_T pBssInfoSec = (P_UNI_CMD_BSSINFO_SEC_T)pHandle;

	/* Fill TLV format */
	pBssInfoSec->u2Tag = UNI_CMD_BSSINFO_SEC;
	pBssInfoSec->u2Length = sizeof(UNI_CMD_BSSINFO_SEC_T);
#ifdef RT_BIG_ENDIAN
	pBssInfoSec->u2Tag = cpu2le16(pBssInfoSec->u2Tag);
	pBssInfoSec->u2Length = cpu2le16(pBssInfoSec->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBssInfoSec->ucAuthMode = 0; /* for mobile */
	pBssInfoSec->ucEncStatus = 0; /* for mobile */
	pBssInfoSec->ucCipherSuit = bss_info->CipherSuit;

	return Ret;
}

static INT32 UniCmdBssInfoTxCmd(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BSSINFO_TXCMD_T pBssInfoTxCmd = (P_UNI_CMD_BSSINFO_TXCMD_T)pHandle;

	if (bss_info->bss_state >= BSS_ACTIVE) {
		struct wifi_dev *wdev = (struct wifi_dev *)bss_info->priv;
		struct hdev_obj *obj = wdev->pHObj;
		struct wmm_entry *entry = wmm_ctrl_get_entry_by_idx(pAd->hdev_ctrl, obj->WmmIdx);;

		/* Fill TLV format */
		pBssInfoTxCmd->u2Tag = UNI_CMD_BSSINFO_TXCMD;
		pBssInfoTxCmd->u2Length = sizeof(UNI_CMD_BSSINFO_TXCMD_T);
#ifdef RT_BIG_ENDIAN
		pBssInfoTxCmd->u2Tag = cpu2le16(pBssInfoTxCmd->u2Tag);
		pBssInfoTxCmd->u2Length = cpu2le16(pBssInfoTxCmd->u2Length);
#endif /* RT_BIG_ENDIAN */
		pBssInfoTxCmd->fgUseTxCMD = (entry->tx_mode == HOBJ_TX_MODE_TXCMD)?TRUE:FALSE;
	} else {
		Ret = NDIS_STATUS_FAILURE;
	}

	return Ret;
}

static INT32 UniCmdBssInfoBasicWrapAll(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, UINT32 *offset, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	PUCHAR pTempBuf = (PUCHAR)pHandle;
	*offset = 0;

	Ret = UniCmdBssInfoBasic(pAd, bss_info, pTempBuf);
	if (Ret == NDIS_STATUS_SUCCESS) {
		pTempBuf += sizeof(UNI_CMD_BSSINFO_BASIC_T);
		*offset += sizeof(UNI_CMD_BSSINFO_BASIC_T);
	}

	/* below TLV is for WA only */
	Ret = UniCmdBssInfoRate(pAd, bss_info, pTempBuf);
	if (Ret == NDIS_STATUS_SUCCESS) {
		pTempBuf += sizeof(UNI_CMD_BSSINFO_RATE_T);
		*offset += sizeof(UNI_CMD_BSSINFO_RATE_T);
	}

	Ret = UniCmdBssInfoSec(pAd, bss_info, pTempBuf);
	if (Ret == NDIS_STATUS_SUCCESS) {
		pTempBuf += sizeof(UNI_CMD_BSSINFO_SEC_T);
		*offset += sizeof(UNI_CMD_BSSINFO_SEC_T);
	}

	Ret = UniCmdBssInfoTxCmd(pAd, bss_info, pTempBuf);
	if (Ret == NDIS_STATUS_SUCCESS) {
		pTempBuf += sizeof(UNI_CMD_BSSINFO_TXCMD_T);
		*offset += sizeof(UNI_CMD_BSSINFO_TXCMD_T);
	}

	return (*offset > 0) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE;
}

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
static INT32 UniCmdBssInfoRA(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BSSINFO_RA_T pBssInfoRA = (P_UNI_CMD_BSSINFO_RA_T)pHandle;

	/* Fill TLV format */
	pBssInfoRA->u2Tag = UNI_CMD_BSSINFO_RA;
	pBssInfoRA->u2Length = sizeof(UNI_CMD_BSSINFO_RA_T);
#ifdef RT_BIG_ENDIAN
	pBssInfoRA->u2Tag = cpu2le16(pBssInfoRA->u2Tag);
	pBssInfoRA->u2Length = cpu2le16(pBssInfoRA->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBssInfoRA->fgShortPreamble = bss_info->ra_cfg.fgShortPreamble;
	pBssInfoRA->fgTestbedForceShortGI = bss_info->ra_cfg.TestbedForceShortGI;
	pBssInfoRA->fgTestbedForceGreenField = bss_info->ra_cfg.TestbedForceGreenField;
#ifdef DOT11_N_SUPPORT
	pBssInfoRA->ucHtMode = bss_info->ra_cfg.HtMode;
#endif /* DOT11_N_SUPPORT */
	pBssInfoRA->fgSeOff = bss_info->ra_cfg.fgSeOff;
	pBssInfoRA->ucAntennaIndex = bss_info->ra_cfg.ucAntennaIndex;
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7981)
	pBssInfoRA->u2MaxPhyRate =  cpu2le16(bss_info->ra_cfg.u2MaxPhyRate);
#endif
	pBssInfoRA->ucForceTxStream =  bss_info->ra_cfg.ucForceTxStream;

	return Ret;
}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

static INT32 UniCmdBssInfoRLM(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BSSINFO_RLM_T pBssInfoRLM = (P_UNI_CMD_BSSINFO_RLM_T)pHandle;
	struct freq_oper *chan_oper = &bss_info->chan_oper;

	/* Fill TLV format */
	pBssInfoRLM->u2Tag = UNI_CMD_BSSINFO_RLM;
	pBssInfoRLM->u2Length = sizeof(UNI_CMD_BSSINFO_RLM_T);
#ifdef RT_BIG_ENDIAN
	pBssInfoRLM->u2Tag = cpu2le16(pBssInfoRLM->u2Tag);
	pBssInfoRLM->u2Length = cpu2le16(pBssInfoRLM->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBssInfoRLM->ucPrimaryChannel = chan_oper->prim_ch;
	pBssInfoRLM->ucCenterChannelSeg0 = chan_oper->cen_ch_1;
	pBssInfoRLM->ucCenterChannelSeg1 = chan_oper->cen_ch_2;
	pBssInfoRLM->ucBandwidth = chan_oper->bw;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	pBssInfoRLM->ucTxStream = bss_info->ra_cfg.TxStream;
	pBssInfoRLM->ucRxStream = bss_info->ra_cfg.RxStream;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "ucPrimaryChannel = %d, ucCenterChannelSeg0 = %d, ucCenterChannelSeg1 = %d, ucBandwidth = %d, ucTxStream = %d, ucRxStream = %d\n",
			  pBssInfoRLM->ucPrimaryChannel,
			  pBssInfoRLM->ucCenterChannelSeg0,
			  pBssInfoRLM->ucCenterChannelSeg1,
			  pBssInfoRLM->ucBandwidth,
			  pBssInfoRLM->ucTxStream,
			  pBssInfoRLM->ucRxStream);

	return Ret;
}

static INT32 UniCmdBssInfoProtect(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BSSINFO_PROT_T pBssInfoProt = (P_UNI_CMD_BSSINFO_PROT_T)pHandle;
	struct prot_info *prot = &bss_info->prot;

	/* Fill TLV format */
	pBssInfoProt->u2Tag = UNI_CMD_BSSINFO_PROTECT;
	pBssInfoProt->u2Length = sizeof(UNI_CMD_BSSINFO_PROT_T);
#ifdef RT_BIG_ENDIAN
	pBssInfoProt->u2Tag = cpu2le16(pBssInfoProt->u2Tag);
	pBssInfoProt->u2Length = cpu2le16(pBssInfoProt->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBssInfoProt->u4ProtectMode = cpu2le32(prot->cookie.protect_mode);

	return Ret;
}

#ifdef DOT11_HE_AX
static INT32 UniCmdBssInfoBSSColor(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BSSINFO_BSS_COLOR_T pBssInfoBssColor = (P_UNI_CMD_BSSINFO_BSS_COLOR_T)pHandle;
	struct bss_color_ctrl *bss_color = &bss_info->bss_color;

	/* Fill TLV format */
	pBssInfoBssColor->u2Tag = UNI_CMD_BSSINFO_BSS_COLOR;
	pBssInfoBssColor->u2Length = sizeof(UNI_CMD_BSSINFO_BSS_COLOR_T);
#ifdef RT_BIG_ENDIAN
	pBssInfoBssColor->u2Tag = cpu2le16(pBssInfoBssColor->u2Tag);
	pBssInfoBssColor->u2Length = cpu2le16(pBssInfoBssColor->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBssInfoBssColor->fgEnable = (bss_color->disabled == TRUE) ? FALSE : FALSE;
	pBssInfoBssColor->ucBssColor = bss_color->color;

	return Ret;
}

static INT32 UniCmdBssInfoHEBasic(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BSSINFO_HE_T pBssInfoHE = (P_UNI_CMD_BSSINFO_HE_T)pHandle;
	struct he_mcs_info *mcs = &bss_info->he_bss.max_nss_mcs;
	UINT32  i = 0;

	/* Fill TLV format */
	pBssInfoHE->u2Tag = UNI_CMD_BSSINFO_HE;
	pBssInfoHE->u2Length = sizeof(UNI_CMD_BSSINFO_HE_T);
#ifdef RT_BIG_ENDIAN
	pBssInfoHE->u2Tag = cpu2le16(pBssInfoHE->u2Tag);
	pBssInfoHE->u2Length = cpu2le16(pBssInfoHE->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBssInfoHE->u2TxopDurationRtsThreshold = cpu2le16(bss_info->he_bss.txop_dur_rts_thr);
	pBssInfoHE->ucDefaultPEDuration = bss_info->he_bss.default_pe_dur;
	for (i = 0 ; i < HE_MAX_SUPPORT_STREAM; i++) {
		pBssInfoHE->au2MaxNssMcs[CMD_HE_MCS_BW80] |= (mcs->bw80_mcs[i] << (i * 2));
		pBssInfoHE->au2MaxNssMcs[CMD_HE_MCS_BW160] |= (mcs->bw160_mcs[i] << (i * 2));
		pBssInfoHE->au2MaxNssMcs[CMD_HE_MCS_BW8080] |= (mcs->bw8080_mcs[i] << (i * 2));
	}

	for (i = 0 ; i < CMD_HE_MCS_BW_NUM; i++)
		pBssInfoHE->au2MaxNssMcs[i] = cpu2le16(pBssInfoHE->au2MaxNssMcs[i]);

	return Ret;
}
#endif /* DOT11_HE_AX */

#ifdef DOT11V_MBSSID_SUPPORT
static INT32 UniCmdBssInfo11vMBSSID(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BSSINFO_11V_MBSSID_T pBssInfo11vMbssid = (P_UNI_CMD_BSSINFO_11V_MBSSID_T)pHandle;

	/* Fill TLV format */
	pBssInfo11vMbssid->u2Tag = UNI_CMD_BSSINFO_11V_MBSSID;
	pBssInfo11vMbssid->u2Length = sizeof(UNI_CMD_BSSINFO_11V_MBSSID_T);
#ifdef RT_BIG_ENDIAN
	pBssInfo11vMbssid->u2Tag = cpu2le16(pBssInfo11vMbssid->u2Tag);
	pBssInfo11vMbssid->u2Length = cpu2le16(pBssInfo11vMbssid->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBssInfo11vMbssid->ucMaxBSSIDIndicator = bss_info->max_bssid_indicator;
	pBssInfo11vMbssid->ucMBSSIDIndex = bss_info->mbssid_index;

	return Ret;
}
#endif /* DOT11V_MBSSID_SUPPORT */

#ifdef BCN_OFFLOAD_SUPPORT
static INT32 UniCmdBssInfoBcnContent(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BSSINFO_BCN_CONTENT_T pBssInfoBcnContent = (P_UNI_CMD_BSSINFO_BCN_CONTENT_T)pHandle;
	struct wifi_dev *wdev = (struct wifi_dev *)bss_info->priv;
	PBCN_BUF_STRUCT bcn_buf = &wdev->bcn_buf;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* Fill TLV format */
	pBssInfoBcnContent->u2Tag = UNI_CMD_BSSINFO_BCN_CONTENT;
	if ((bcn_buf->bBcnSntReq == TRUE) && (bcn_buf->BeaconPkt))
		pBssInfoBcnContent->u2Length = sizeof(UNI_CMD_BSSINFO_BCN_CONTENT_T) + cap->tx_hw_hdr_len + bcn_buf->FrameLen;
	else
		pBssInfoBcnContent->u2Length = sizeof(UNI_CMD_BSSINFO_BCN_CONTENT_T);
#ifdef RT_BIG_ENDIAN
	pBssInfoBcnContent->u2Tag = cpu2le16(pBssInfoBcnContent->u2Tag);
	pBssInfoBcnContent->u2Length = cpu2le16(pBssInfoBcnContent->u2Length);
#endif /* RT_BIG_ENDIAN */

	if ((bcn_buf->bBcnSntReq == TRUE) && (bcn_buf->BeaconPkt)) {
		pBssInfoBcnContent->ucAction = BCN_ACTION_ENABLE;
		pBssInfoBcnContent->u2TimIeOffset = cpu2le16(bcn_buf->TimIELocationInBeacon);
		pBssInfoBcnContent->u2CsaIeOffset = cpu2le16(bcn_buf->CsaIELocationInBeacon);
#ifdef DOT11_HE_AX
		pBssInfoBcnContent->u2BccIeOffset = cpu2le16(bcn_buf->bcc_ie_location);
#endif
		/* thes aucPktContent field include TXD,
		 * MAC header and payload
		 */
		pBssInfoBcnContent->aucPktContentType = 0;
		pBssInfoBcnContent->u2PktLength = cpu2le16(cap->tx_hw_hdr_len + bcn_buf->FrameLen);
		pBssInfoBcnContent->u2TimIeOffset = cpu2le16(pBssInfoBcnContent->u2TimIeOffset);
		pBssInfoBcnContent->u2CsaIeOffset = cpu2le16(pBssInfoBcnContent->u2CsaIeOffset);
		pBssInfoBcnContent->u2BccIeOffset = cpu2le16(pBssInfoBcnContent->u2BccIeOffset);
		pBssInfoBcnContent->u2PktLength = cpu2le16(pBssInfoBcnContent->u2PktLength);

		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS(%d), BcnLength = %d, IE offset(TIM/CSA/BCC) = %d/%d/%d\n",
				bss_info->ucBssIndex, le2cpu16(pBssInfoBcnContent->u2PktLength), le2cpu16(pBssInfoBcnContent->u2TimIeOffset),
				le2cpu16(pBssInfoBcnContent->u2CsaIeOffset), le2cpu16(pBssInfoBcnContent->u2BccIeOffset));

		os_move_mem(pBssInfoBcnContent->aucPktContent, (char *)GET_OS_PKT_DATAPTR(bcn_buf->BeaconPkt),
					(cap->tx_hw_hdr_len + bcn_buf->FrameLen));
	} else {
		pBssInfoBcnContent->ucAction = BCN_ACTION_DISABLE;
	}

	return Ret;
}

static INT32 UniCmdBssInfoBcnCSA(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BSSINFO_BCN_CSA_T pBssInfoBcnCSA = (P_UNI_CMD_BSSINFO_BCN_CSA_T)pHandle;
	struct wifi_dev *wdev = (struct wifi_dev *)bss_info->priv;

	if ((wdev->csa_count == 0) || (bss_info->bUpdateReason != BCN_UPDATE_CSA))
		return NDIS_STATUS_FAILURE;

	/* Fill TLV format */
	pBssInfoBcnCSA->u2Tag = UNI_CMD_BSSINFO_BCN_CSA;
	pBssInfoBcnCSA->u2Length = sizeof(UNI_CMD_BSSINFO_BCN_CSA_T);
#ifdef RT_BIG_ENDIAN
	pBssInfoBcnCSA->u2Tag = cpu2le16(pBssInfoBcnCSA->u2Tag);
	pBssInfoBcnCSA->u2Length = cpu2le16(pBssInfoBcnCSA->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBssInfoBcnCSA->ucCsaCount = wdev->csa_count;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS(%d), CsaCount = %d\n",
			bss_info->ucBssIndex, wdev->csa_count);

	return Ret;
}

#ifdef DOT11_HE_AX
static INT32 UniCmdBssInfoBcnBCC(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BSSINFO_BCN_BCC_T pBssInfoBcnBCC = (P_UNI_CMD_BSSINFO_BCN_BCC_T)pHandle;
	struct wifi_dev *wdev = (struct wifi_dev *)bss_info->priv;
	struct bss_color_ctrl *bss_color = &bss_info->bss_color;

	if (wdev->bcn_buf.bcc_ie_location == 0)
		return NDIS_STATUS_FAILURE;

	/* Fill TLV format */
	pBssInfoBcnBCC->u2Tag = UNI_CMD_BSSINFO_BCN_BCC;
	pBssInfoBcnBCC->u2Length = sizeof(UNI_CMD_BSSINFO_BCN_CSA_T);
#ifdef RT_BIG_ENDIAN
	pBssInfoBcnBCC->u2Tag = cpu2le16(pBssInfoBcnBCC->u2Tag);
	pBssInfoBcnBCC->u2Length = cpu2le16(pBssInfoBcnBCC->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBssInfoBcnBCC->ucBccCount = bss_color->u.ap_ctrl.bcc_count;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BSS(%d), BccCount = %d\n",
			bss_info->ucBssIndex, bss_color->u.ap_ctrl.bcc_count);

	return Ret;
}
#endif /* DOT11_HE_AX */

#ifdef DOT11V_MBSSID_SUPPORT
static INT32 UniCmdBssInfoBcnMBSSID(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BSSINFO_BCN_MBSSID_T pBssInfoBcnMBSSID = (P_UNI_CMD_BSSINFO_BCN_MBSSID_T)pHandle;
	UINT8 DbdcIdx = DBDC_BAND0;
	BSS_STRUCT *pMbss = NULL;
	INT32 IdBss;

	if (IS_BSSID_11V_ENABLED(pAd, bss_info->ucBandIdx) == FALSE)
		return NDIS_STATUS_FAILURE;

	/* Fill TLV format */
	pBssInfoBcnMBSSID->u2Tag = UNI_CMD_BSSINFO_BCN_MBSSID;
	pBssInfoBcnMBSSID->u2Length = sizeof(UNI_CMD_BSSINFO_BCN_MBSSID_T);
#ifdef RT_BIG_ENDIAN
	pBssInfoBcnMBSSID->u2Tag = cpu2le16(pBssInfoBcnMBSSID->u2Tag);
	pBssInfoBcnMBSSID->u2Length = cpu2le16(pBssInfoBcnMBSSID->u2Length);
#endif /* RT_BIG_ENDIAN */

	for (IdBss = MAIN_MBSSID; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
		pMbss = &pAd->ApCfg.MBSSID[IdBss];
		DbdcIdx = HcGetBandByWdev(&pMbss->wdev);
		/* update TIM offset at the same band */
		if (DbdcIdx == bss_info->ucBandIdx) {
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DbdcIdx(%d), BSS(%d), IE Offset = %d\n",
				DbdcIdx, IdBss, pMbss->wdev.bcn_buf.TimIELocationInBeacon);

			pBssInfoBcnMBSSID->u2MbssidIeOffset[IdBss] = cpu2le16(pMbss->wdev.bcn_buf.TimIELocationInBeacon);
		}

		/* build global 11v mbssid bitmap */
		if (pAd->ApCfg.dot11v_mbssid_bitmap[DbdcIdx] & (1 << pMbss->mbss_grp_idx))
			pBssInfoBcnMBSSID->u4Dot11vMbssidBitmap |= (1 << IdBss);
	}
	pBssInfoBcnMBSSID->u4Dot11vMbssidBitmap = cpu2le32(pBssInfoBcnMBSSID->u4Dot11vMbssidBitmap);

	return Ret;
}
#endif /* DOT11V_MBSSID_SUPPORT */

static INT32 UniCmdBssInfoBcnOffloadWrapAll(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, UINT32 *offset, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	PUCHAR	pTempBuf = (PUCHAR)pHandle;
	P_UNI_CMD_BSSINFO_BCN_CONTENT_T pBssInfoBcnContent = NULL;
	*offset = 0;

	Ret = UniCmdBssInfoBcnCSA(pAd, bss_info, pTempBuf);
	if (Ret == NDIS_STATUS_SUCCESS) {
		pTempBuf += sizeof(UNI_CMD_BSSINFO_BCN_CSA_T);
		*offset += sizeof(UNI_CMD_BSSINFO_BCN_CSA_T);
	}

#ifdef DOT11_HE_AX
	Ret = UniCmdBssInfoBcnBCC(pAd, bss_info, pTempBuf);
	if (Ret == NDIS_STATUS_SUCCESS) {
		pTempBuf += sizeof(UNI_CMD_BSSINFO_BCN_BCC_T);
		*offset += sizeof(UNI_CMD_BSSINFO_BCN_BCC_T);
	}
#endif /* DOT11_HE_AX */

#ifdef DOT11V_MBSSID_SUPPORT
	Ret = UniCmdBssInfoBcnMBSSID(pAd, bss_info, pTempBuf);
	if (Ret == NDIS_STATUS_SUCCESS) {
		pTempBuf += sizeof(UNI_CMD_BSSINFO_BCN_MBSSID_T);
		*offset += sizeof(UNI_CMD_BSSINFO_BCN_MBSSID_T);
	}
#endif /* DOT11V_MBSSID_SUPPORT */

	Ret = UniCmdBssInfoBcnContent(pAd, bss_info, pTempBuf);
	if (Ret == NDIS_STATUS_SUCCESS) {
		pBssInfoBcnContent = (P_UNI_CMD_BSSINFO_BCN_CONTENT_T)pTempBuf;
		if (pBssInfoBcnContent->u2PktLength > 0) {
			pTempBuf += pBssInfoBcnContent->u2PktLength;
			*offset += pBssInfoBcnContent->u2PktLength;
		}
		pTempBuf += sizeof(UNI_CMD_BSSINFO_BCN_CONTENT_T);
		*offset += sizeof(UNI_CMD_BSSINFO_BCN_CONTENT_T);
	}

	return (*offset > 0) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE;
}
#endif /* BCN_OFFLOAD_SUPPORT */

#ifdef BCN_PROTECTION_SUPPORT
static INT32 UniCmdBssInfoBcnProt(struct _RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BSSINFO_BCN_PROT_T pBssInfoBcnProt = (P_UNI_CMD_BSSINFO_BCN_PROT_T)pHandle;
	struct bcn_protection_cfg *bcn_prot_cfg = &bss_info->bcn_prot_cfg;
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR table_idx = get_bigtk_table_idx(bcn_prot_cfg);

	/* Fill TLV format */
	pBssInfoBcnProt->u2Tag = UNI_CMD_BSS_INFO_BCN_PROT;
	pBssInfoBcnProt->u2Length = sizeof(UNI_CMD_BSSINFO_BCN_PROT_T);
#ifdef RT_BIG_ENDIAN
	pBssInfoBcnProt->u2Tag = cpu2le16(pBssInfoBcnProt->u2Tag);
	pBssInfoBcnProt->u2Length = cpu2le16(pBssInfoBcnProt->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBssInfoBcnProt->ucBcnProtEnabled = (bcn_prot_cfg->bcn_prot_en) ? chip_cap->bcn_prot_sup : BCN_PROT_EN_OFF;
	pBssInfoBcnProt->ucBcnProtKeyId = bcn_prot_cfg->bigtk_key_idx;
	if (IS_CIPHER_BIP_CMAC128(bcn_prot_cfg->bigtk_cipher))
		pBssInfoBcnProt->ucBcnProtCipherId = SEC_CIPHER_ID_BIP_CMAC_128;
	else if (IS_CIPHER_BIP_CMAC256(bcn_prot_cfg->bigtk_cipher))
		pBssInfoBcnProt->ucBcnProtCipherId = SEC_CIPHER_ID_BIP_CMAC_256;
	else {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_ERROR,
					 "not support bigtk cipher (0x%x), use default bip-cmac-128\n",
					 bcn_prot_cfg->bigtk_cipher);
		pBssInfoBcnProt->ucBcnProtCipherId = SEC_CIPHER_ID_BIP_CMAC_128;
	}
	os_move_mem(&pBssInfoBcnProt->aucBcnProtKey, &bcn_prot_cfg->bigtk[table_idx][0], LEN_MAX_BIGTK);
	os_move_mem(&pBssInfoBcnProt->aucBcnProtPN, &bcn_prot_cfg->bipn[table_idx][0], LEN_WPA_TSC);

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_DEBUG, "%s:ucBcnProtEnabled (%d), ucBcnProtCipherId(%d), ucBcnProtKeyId(%d)\n",
			__func__, pBssInfoBcnProt->ucBcnProtEnabled, pBssInfoBcnProt->ucBcnProtCipherId, pBssInfoBcnProt->ucBcnProtKeyId);
	hex_dump_with_cat_and_lvl("aucBcnProtKey:", pBssInfoBcnProt->aucBcnProtKey,
						LEN_MAX_BIGTK, DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_DEBUG);
	hex_dump_with_cat_and_lvl("aucBcnProtPN:", pBssInfoBcnProt->aucBcnProtPN,
						LEN_WPA_TSC, DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_DEBUG);

	return Ret;
}
#endif /* BCN_PROTECTION_SUPPORT */

static UNI_CMD_TAG_HANDLE_T UniCmdBssInfoTab[] = {
	{
		.u2CmdFeature = BSS_INFO_BASIC_FEATURE, /* BSS_INFO_OWN_MAC_FEATURE is the same, so ignore it now. */
		.u4StructSize = (sizeof(UNI_CMD_BSSINFO_BASIC_T) +
						 sizeof(UNI_CMD_BSSINFO_RATE_T) +
						 sizeof(UNI_CMD_BSSINFO_SEC_T) +
						 sizeof(UNI_CMD_BSSINFO_TXCMD_T)),
		.pfHandler = UniCmdBssInfoBasicWrapAll
	},
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	{
		.u2CmdFeature = BSS_INFO_RA_FEATURE,
		.u4StructSize = sizeof(UNI_CMD_BSSINFO_RA_T),
		.pfHandler = UniCmdBssInfoRA
	},
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	{
		.u2CmdFeature = BSS_INFO_RF_CH_FEATURE,
		.u4StructSize = sizeof(UNI_CMD_BSSINFO_RLM_T),
		.pfHandler = UniCmdBssInfoRLM
	},
	{
		.u2CmdFeature = BSS_INFO_PROTECT_INFO_FEATURE,
		.u4StructSize = sizeof(UNI_CMD_BSSINFO_PROT_T),
		.pfHandler = UniCmdBssInfoProtect
	},
#ifdef DOT11_HE_AX
	{
		.u2CmdFeature = BSS_INFO_BSS_COLOR_FEATURE,
		.u4StructSize = sizeof(UNI_CMD_BSSINFO_BSS_COLOR_T),
		.pfHandler = UniCmdBssInfoBSSColor
	},
	{
		.u2CmdFeature = BSS_INFO_HE_BASIC_FEATURE,
		.u4StructSize = sizeof(UNI_CMD_BSSINFO_HE_T),
		.pfHandler = UniCmdBssInfoHEBasic
	},
#endif /* DOT11_HE_AX */
#ifdef DOT11V_MBSSID_SUPPORT
	{
		.u2CmdFeature = BSS_INFO_11V_MBSSID_FEATURE,
		.u4StructSize = sizeof(UNI_CMD_BSSINFO_11V_MBSSID_T),
		.pfHandler = UniCmdBssInfo11vMBSSID
	},
#endif /* DOT11V_MBSSID_SUPPORT */
#ifdef BCN_OFFLOAD_SUPPORT
	{
		.u2CmdFeature = BSS_INFO_OFFLOAD_PKT_FEATURE,
		.u4StructSize = (sizeof(UNI_CMD_BSSINFO_BCN_CONTENT_T) +
						 sizeof(UNI_CMD_BSSINFO_BCN_CSA_T) +
						 sizeof(UNI_CMD_BSSINFO_BCN_BCC_T) +
						 sizeof(UNI_CMD_BSSINFO_BCN_MBSSID_T)),
		.pfHandler = UniCmdBssInfoBcnOffloadWrapAll
	},
#endif /* BCN_OFFLOAD_SUPPORT */
	{
		.u2CmdFeature = BSS_INFO_BROADCAST_INFO_FEATURE,
		.u4StructSize = sizeof(UNI_CMD_BSSINFO_RATE_T),
		.pfHandler = UniCmdBssInfoRate
	},
#ifdef BCN_PROTECTION_SUPPORT
	{
		.u2CmdFeature = BSS_INFO_BCN_PROT_FEATURE,
		.u4StructSize = sizeof(UNI_CMD_BSSINFO_BCN_PROT_T),
		.pfHandler = UniCmdBssInfoBcnProt
	},
#endif /* BCN_PROTECTION_SUPPORT */
	{
		.u2CmdFeature = BSS_INFO_BCN_PROT_FEATURE,
		.u4StructSize = sizeof(UNI_CMD_BSSINFO_TXCMD_T),
		.pfHandler = UniCmdBssInfoBcnProt
	},
};

static UINT32 UniCmdBssInfoExtraAllocDynSizeCheck(RTMP_ADAPTER *pAd, BSS_INFO_ARGUMENT_T *bss_info)
{
	UINT32 i = 0;
	UINT32 BufSize = 0;
	UINT32 BssInfoTabSize = (sizeof(UniCmdBssInfoTab) / sizeof(UniCmdBssInfoTab[0]));

	for (i = 0; i < BssInfoTabSize; i++) {
		if (bss_info->u4BssInfoFeature & UniCmdBssInfoTab[i].u2CmdFeature) {
			if (UniCmdBssInfoTab[i].u2CmdFeature & BSS_INFO_OFFLOAD_PKT_FEATURE) {
				struct wifi_dev *wdev = (struct wifi_dev *)bss_info->priv;
				PBCN_BUF_STRUCT bcn_buf = &wdev->bcn_buf;
				RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

				if ((bcn_buf->bBcnSntReq == TRUE) && (bcn_buf->BeaconPkt)) {
					BufSize += (cap->tx_hw_hdr_len + bcn_buf->FrameLen);
				}
			}
		}
	}

	return BufSize;
}

INT32 UniCmdBssInfoUpdate(
	RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info)
{
	struct cmd_msg          *msg = NULL;
	INT32                   Ret = NDIS_STATUS_SUCCESS;
	UINT32                  i = 0;
	UINT16                  u2TLVNumber = 0;
	PUCHAR					pTempBuf = NULL;
	PUCHAR					pNextHeadBuf = NULL;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4RealUseBufSize = 0;
	UINT32					u4SendBufSize = 0;
	UINT32					u4RemainingPayloadSize = 0;
	UINT32					u4ComCmdSize = 0;
	UINT32					u4Offset = 0;
	P_UNI_CMD_BSSINFO_T		pCmdBssInfoUpdate = NULL;
	RTMP_CHIP_CAP			*cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32 					BssInfoTabSize = (sizeof(UniCmdBssInfoTab) / sizeof(UniCmdBssInfoTab[0]));

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_BSSINFO_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < BssInfoTabSize; i++) {
		if (bss_info->u4BssInfoFeature & UniCmdBssInfoTab[i].u2CmdFeature)
			u4CmdNeedMaxBufSize += UniCmdBssInfoTab[i].u4StructSize;
	}
	u4CmdNeedMaxBufSize += UniCmdBssInfoExtraAllocDynSizeCheck(pAd, bss_info);

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdBssInfoUpdate = (P_UNI_CMD_BSSINFO_T)pNextHeadBuf;
	pCmdBssInfoUpdate->ucBssInfoIdx = bss_info->ucBssIndex;
	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Traverse all support features */
	for (i = 0; i < BssInfoTabSize; i++) {
		switch (bss_info->u4BssInfoFeature & UniCmdBssInfoTab[i].u2CmdFeature) {
		case BSS_INFO_BASIC_FEATURE:
			if (UniCmdBssInfoTab[i].pfHandler != NULL) {
				Ret = ((PFN_BSSINFO_BASIC_HANDLE)(UniCmdBssInfoTab[i].pfHandler))(pAd, bss_info, &u4Offset, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += u4Offset;
					u2TLVNumber++;
				}
			}
			break;

		case BSS_INFO_RA_FEATURE:
			if (UniCmdBssInfoTab[i].pfHandler != NULL) {
				Ret = ((PFN_BSSINFO_RA_HANDLE)(UniCmdBssInfoTab[i].pfHandler))(pAd, bss_info, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += UniCmdBssInfoTab[i].u4StructSize;
					u2TLVNumber++;
				}
			}
			break;

		case BSS_INFO_RF_CH_FEATURE:
			if (UniCmdBssInfoTab[i].pfHandler != NULL) {
				Ret = ((PFN_BSSINFO_RLM_HANDLE)(UniCmdBssInfoTab[i].pfHandler))(pAd, bss_info, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += UniCmdBssInfoTab[i].u4StructSize;
					u2TLVNumber++;
				}
			}
			break;

		case BSS_INFO_PROTECT_INFO_FEATURE:
			if (UniCmdBssInfoTab[i].pfHandler != NULL) {
				Ret = ((PFN_BSSINFO_PROT_HANDLE)(UniCmdBssInfoTab[i].pfHandler))(pAd, bss_info, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += UniCmdBssInfoTab[i].u4StructSize;
					u2TLVNumber++;
				}
			}
			break;

		case BSS_INFO_BSS_COLOR_FEATURE:
			if (UniCmdBssInfoTab[i].pfHandler != NULL) {
				Ret = ((PFN_BSSINFO_BSS_COLOR_HANDLE)(UniCmdBssInfoTab[i].pfHandler))(pAd, bss_info, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += UniCmdBssInfoTab[i].u4StructSize;
					u2TLVNumber++;
				}
			}
			break;

		case BSS_INFO_HE_BASIC_FEATURE:
			if (UniCmdBssInfoTab[i].pfHandler != NULL) {
				Ret = ((PFN_BSSINFO_HE_HANDLE)(UniCmdBssInfoTab[i].pfHandler))(pAd, bss_info, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += UniCmdBssInfoTab[i].u4StructSize;
					u2TLVNumber++;
				}
			}
			break;

		case BSS_INFO_11V_MBSSID_FEATURE:
			if (UniCmdBssInfoTab[i].pfHandler != NULL) {
				Ret = ((PFN_BSSINFO_11V_MBSSID_HANDLE)(UniCmdBssInfoTab[i].pfHandler))(pAd, bss_info, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += UniCmdBssInfoTab[i].u4StructSize;
					u2TLVNumber++;
				}
			}
			break;

		case BSS_INFO_OFFLOAD_PKT_FEATURE:
			if (UniCmdBssInfoTab[i].pfHandler != NULL) {
				Ret = ((PFN_BSSINFO_BCN_OFFLOAD_HANDLE)(UniCmdBssInfoTab[i].pfHandler))(pAd, bss_info, &u4Offset, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += u4Offset;
					u2TLVNumber++;
				}
			}
			break;

		case BSS_INFO_BROADCAST_INFO_FEATURE:
			if (UniCmdBssInfoTab[i].pfHandler != NULL) {
				Ret = ((PFN_BSSINFO_RATE_HANDLE)(UniCmdBssInfoTab[i].pfHandler))(pAd, bss_info, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += UniCmdBssInfoTab[i].u4StructSize;
					u2TLVNumber++;
				}
			}
			break;

		case BSS_INFO_BCN_PROT_FEATURE:
			if (UniCmdBssInfoTab[i].pfHandler != NULL) {
				Ret = ((PFN_BSSINFO_BCN_PROT_HANDLE)(UniCmdBssInfoTab[i].pfHandler))(pAd, bss_info, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += UniCmdBssInfoTab[i].u4StructSize;
					u2TLVNumber++;
				}
			}
			break;

		default:
			Ret = NDIS_STATUS_SUCCESS;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"The hanlder of tag (0x%08x) not support!\n", bss_info->u4BssInfoFeature);
			break;
		}

		if (Ret != NDIS_STATUS_SUCCESS)
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s(): The hanlder of tag (0x%08x) return fail!\n", __func__, UniCmdBssInfoTab[i].u2CmdFeature);
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG_NP(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"OwnMacIdx = %d, Band = %d, BssIndex = %d (%02x-%02x-%02x-%02x-%02x-%02x), TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			bss_info->OwnMacIdx, bss_info->ucBandIdx, bss_info->ucBssIndex,
			PRINT_MAC(bss_info->Bssid), u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_BSSINFO);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if (!bNeedFrag || bLastFrag) {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
				SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
			} else {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			call_fw_cmd_notifieriers(WO_CMD_BSS_INFO, pAd, msg->net_pkt);
			Ret = AndesSendCmdMsg(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

/*****************************************
 * BssInfo Power Save information (Tag 0x15)
 *****************************************/
INT32 MtUniCmdPmStateCtrl(struct _RTMP_ADAPTER *pAd, MT_PMSTAT_CTRL_T PmStatCtrl)
{
	struct cmd_msg *msg = NULL;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UNI_CMD_BSSINFO_T UniCmdBssInfo;
	UNI_CMD_BSSINFO_POWER_SAVE_T UniCmdBssInfoPowerSave;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	struct wifi_dev *wdev = NULL;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdBssInfo);
	os_zero_mem(&UniCmdBssInfo, u4ComCmdSize);
	os_zero_mem(&UniCmdBssInfoPowerSave, sizeof(UniCmdBssInfoPowerSave));

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(UniCmdBssInfoPowerSave);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_BSSINFO);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
	SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	wdev = wdev_search_by_band_omac_idx(pAd, PmStatCtrl.DbdcIdx, PmStatCtrl.OwnMacIdx);
	if (!wdev) {
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}
	UniCmdBssInfo.ucBssInfoIdx = wdev->BssIdx;
	AndesAppendCmdMsg(msg, (char *)&UniCmdBssInfo, u4ComCmdSize);

	/* Step 4: Fill TLV parameters here */
	UniCmdBssInfoPowerSave.u2Tag = UNI_CMD_BSSINFO_POWER_SAVE;
	UniCmdBssInfoPowerSave.u2Length = (u4CmdNeedMaxBufSize - u4ComCmdSize);
#ifdef RT_BIG_ENDIAN
	UniCmdBssInfoPowerSave.u2Tag = cpu2le16(UniCmdBssInfoPowerSave.u2Tag);
	UniCmdBssInfoPowerSave.u2Length = cpu2le16(UniCmdBssInfoPowerSave.u2Length);
#endif /* RT_BIG_ENDIAN */
	if (PmStatCtrl.PmState == ENTER_PM_STATE)
		UniCmdBssInfoPowerSave.ucPsProfile = ENUM_PSP_CONTINUOUS_POWER_SAVE;
	else
		UniCmdBssInfoPowerSave.ucPsProfile = ENUM_PSP_CONTINUOUS_ACTIVE;
	AndesAppendCmdMsg(msg, (char *)&UniCmdBssInfoPowerSave, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BssidIdx(%d),ucPsProfile(%d)\n",
			  UniCmdBssInfo.ucBssInfoIdx, UniCmdBssInfoPowerSave.ucPsProfile);

	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(Ret = %d)\n", __func__, ret);
	return ret;
}

/*****************************************
 * BssInfo IFS time information (Tag 0x17)
 *****************************************/
INT32 MtUniCmdSlotTimeSet(
	struct _RTMP_ADAPTER *pAd,
	UINT16 SlotTime,
	UINT16 SifsTime,
	UINT16 RifsTime,
	UINT16 EifsTime,
	struct wifi_dev *wdev)
{
	struct cmd_msg *msg;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UNI_CMD_BSSINFO_T UniCmdBssInfo;
	UNI_CMD_BSSINFO_IFS_TIME_T UniCmdBssInfoIFSTime;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdBssInfo);
	os_zero_mem(&UniCmdBssInfo, u4ComCmdSize);
	os_zero_mem(&UniCmdBssInfoIFSTime, sizeof(UniCmdBssInfoIFSTime));

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(UniCmdBssInfoIFSTime);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_BSSINFO);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
	SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	UniCmdBssInfo.ucBssInfoIdx = wdev->BssIdx;
	AndesAppendCmdMsg(msg, (char *)&UniCmdBssInfo, u4ComCmdSize);

	/* Step 4: Fill TLV parameters here */
	UniCmdBssInfoIFSTime.u2Tag = UNI_CMD_BSSINFO_IFS_TIME;
	UniCmdBssInfoIFSTime.u2Length = (u4CmdNeedMaxBufSize - u4ComCmdSize);
#ifdef RT_BIG_ENDIAN
	UniCmdBssInfoIFSTime.u2Tag = cpu2le16(UniCmdBssInfoIFSTime.u2Tag);
	UniCmdBssInfoIFSTime.u2Length = cpu2le16(UniCmdBssInfoIFSTime.u2Length);
#endif /* RT_BIG_ENDIAN */
	UniCmdBssInfoIFSTime.u2SlotTime = cpu2le16(SlotTime);
	UniCmdBssInfoIFSTime.fgSlotValid = TRUE;
	UniCmdBssInfoIFSTime.u2SifsTime = cpu2le16(SifsTime);
	UniCmdBssInfoIFSTime.fgSifsValid = TRUE;
	UniCmdBssInfoIFSTime.u2RifsTime = RifsTime;
	UniCmdBssInfoIFSTime.fgRifsValid = TRUE;
	UniCmdBssInfoIFSTime.u2EifsTime = EifsTime;
	UniCmdBssInfoIFSTime.fgEifsValid = TRUE;
	AndesAppendCmdMsg(msg, (char *)&UniCmdBssInfoIFSTime, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	ret = chip_cmd_tx(pAd, msg);
	return ret;
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

#ifdef OCE_SUPPORT
/*****************************************
 * BssInfo IFS time information (Tag 0x19)
 *****************************************/
INT32 MtUniCmdFdFrameOffloadSet(RTMP_ADAPTER *pAd, P_CMD_FD_FRAME_OFFLOAD_T fdFrame_offload)
{
	struct cmd_msg *msg;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _CMD_ATTRIBUTE attr = {0};
	UNI_CMD_BSSINFO_T UniCmdBssInfo;
	UNI_CMD_BSSINFO_OFFLOAD_PKT_T UniCmdBssInfoOffloadPkt;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	struct wifi_dev *wdev = NULL;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s(): Enable = %d, OwnMacIdx = %d, WlanIdx = %d, Band = %d, Len = %d\n",
			  __func__, fdFrame_offload->ucEnable, fdFrame_offload->ucOwnMacIdx,
			  fdFrame_offload->ucWlanIdx, fdFrame_offload->ucBandIdx, fdFrame_offload->u2PktLength));

	u4ComCmdSize = sizeof(UniCmdBssInfo);
	os_zero_mem(&UniCmdBssInfo, u4ComCmdSize);
	os_zero_mem(&UniCmdBssInfoOffloadPkt, sizeof(UniCmdBssInfoOffloadPkt));

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(UniCmdBssInfoOffloadPkt) + fdFrame_offload->u2PktLength;

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_BSSINFO);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
	SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	wdev = wdev_search_by_band_omac_idx(pAd, fdFrame_offload->ucBandIdx, fdFrame_offload->ucOwnMacIdx);
	if (!wdev) {
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}
	UniCmdBssInfo.ucBssInfoIdx = wdev->bss_info_argument.ucBssIndex;
	AndesAppendCmdMsg(msg, (char *)&UniCmdBssInfo, u4ComCmdSize);

	/* Step 4: Fill TLV parameters here */
	UniCmdBssInfoOffloadPkt.u2Tag = UNI_CMD_BSSINFO_OFFLOAD_PKT;
	UniCmdBssInfoOffloadPkt.u2Length = (u4CmdNeedMaxBufSize - u4ComCmdSize);
#ifdef RT_BIG_ENDIAN
	UniCmdBssInfoOffloadPkt.u2Tag = cpu2le16(UniCmdBssInfoOffloadPkt.u2Tag);
	UniCmdBssInfoOffloadPkt.u2Length = cpu2le16(UniCmdBssInfoOffloadPkt.u2Length);
#endif /* RT_BIG_ENDIAN */
	os_move_mem(UniCmdBssInfoOffloadPkt.aucPktContent, fdFrame_offload->acPktContent, fdFrame_offload->u2PktLength);
	UniCmdBssInfoOffloadPkt.u2OffloadPktLength = cpu2le16(fdFrame_offload->u2PktLength);
	UniCmdBssInfoOffloadPkt.ucTxMode = 0;
	UniCmdBssInfoOffloadPkt.ucTxType = BSSINFO_UNSOLICIT_TX_FILS_DISC;
	UniCmdBssInfoOffloadPkt.ucTxInterval = 200;
	UniCmdBssInfoOffloadPkt.fgEnable = fdFrame_offload->ucEnable;
	UniCmdBssInfoOffloadPkt.u2Wcid = fdFrame_offload.ucWlanIdx;
	UniCmdBssInfoOffloadPkt.u2Wcid = cpu2le16(UniCmdBssInfoOffloadPkt.u2Wcid);
	AndesAppendCmdMsg(msg, (char *)&UniCmdBssInfoOffloadPkt, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#endif /* OCE_SUPPORT */


/*
* Unified command UNI_CMD_STAREC_BASIC (TAG 0x00) handler
* refer to StaRecUpdateBasic
*/
static INT32 UniCmdStaRecBasic(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	/* Fill STA Rec Common */
#ifdef CONFIG_STA_SUPPORT
	EDCA_PARM *pEdca = NULL;
#endif /*CONFIG_STA_SUPPORT*/
	P_CMD_STAREC_COMMON_T pStaRecCommon = (P_CMD_STAREC_COMMON_T)pHandle;
#ifdef CONFIG_STA_SUPPORT
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	STA_TR_ENTRY *tr_entry = &tr_ctl->tr_entry[pStaRecCfg->u2WlanIdx];
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, tr_entry->wdev);
#endif
	/* Fill TLV format */
	pStaRecCommon->u2Tag = UNI_CMD_STAREC_BASIC;
	pStaRecCommon->u2Length = sizeof(CMD_STAREC_COMMON_T);
	pStaRecCommon->u4ConnectionType = cpu2le32(pStaRecCfg->ConnectionType);
	pStaRecCommon->ucConnectionState = pStaRecCfg->ConnectionState;
	/* New info to indicate this is new way to update STAREC */
	pStaRecCommon->u2ExtraInfo = STAREC_COMMON_EXTRAINFO_V2;

	if (pStaRecCfg->IsNewSTARec)
		pStaRecCommon->u2ExtraInfo |= STAREC_COMMON_EXTRAINFO_NEWSTAREC;

#ifdef CONFIG_AP_SUPPORT

	if (pEntry) {
		pStaRecCommon->ucIsQBSS =
			CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE) ?
			TRUE : FALSE;
		pStaRecCommon->u2AID = cpu2le16(pEntry->Aid);
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (pStaCfg) {
		pEdca = HcGetEdca(pAd, &pStaCfg->wdev);

		if (pEdca)
			pStaRecCommon->ucIsQBSS = pEdca->bValid;

		pStaRecCommon->u2AID = cpu2le16(pStaCfg->StaActive.Aid);
	}

#endif /*CONFIG_STA_SUPPORT*/

	if (pEntry) {
		os_move_mem(pStaRecCommon->aucPeerMacAddr,
					pEntry->Addr, MAC_ADDR_LEN);
	} else {
		os_move_mem(pStaRecCommon->aucPeerMacAddr,
					BROADCAST_ADDR, MAC_ADDR_LEN);
	}

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s(CMD_STAREC_COMMON_T), u4ConnectionType = %d, ucConnectionState = %d, ucIsQBSS = %d, u2AID = %d, aucPeerMacAddr = %02x:%02x:%02x:%02x:%02x:%02x\n",
			  __func__,
			  le2cpu32(pStaRecCommon->u4ConnectionType),
			  pStaRecCommon->ucConnectionState,
			  pStaRecCommon->ucIsQBSS,
			  le2cpu16(pStaRecCommon->u2AID),
			  PRINT_MAC(pStaRecCommon->aucPeerMacAddr));
	/* Append this feature */
#ifdef RT_BIG_ENDIAN
	pStaRecCommon->u2Tag = cpu2le16(pStaRecCommon->u2Tag);
	pStaRecCommon->u2Length = cpu2le16(pStaRecCommon->u2Length);
	pStaRecCommon->u2ExtraInfo = cpu2le16(pStaRecCommon->u2ExtraInfo);
#endif
	return Ret;
}

/*
* Unified command UNI_CMD_STAREC_RA (TAG 0x01) handler
* refer to StaRecUpdateRa
*/
static INT32 UniCmdStaRecRA(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg,
	VOID *pHandle)
{
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	P_CMD_STAREC_AUTO_RATE_T CmdStaRecAutoRate = (P_CMD_STAREC_AUTO_RATE_T)pHandle;

	if (pEntry) {
		os_zero_mem(CmdStaRecAutoRate, sizeof(CMD_STAREC_AUTO_RATE_T));
		StaRecAutoRateParamSet(&pEntry->RaEntry, CmdStaRecAutoRate);
		CmdStaRecAutoRate->u2Tag = UNI_CMD_STAREC_RA;
		CmdStaRecAutoRate->u2Length = sizeof(CMD_STAREC_AUTO_RATE_T);
#ifdef RT_BIG_ENDIAN
		CmdStaRecAutoRate->u2Tag = cpu2le16(CmdStaRecAutoRate->u2Tag);
		CmdStaRecAutoRate->u2Length = cpu2le16(CmdStaRecAutoRate->u2Length);
#endif
		return NDIS_STATUS_SUCCESS;
	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	return NDIS_STATUS_FAILURE;
}

/*
* Unified command UNI_CMD_STAREC_RA_UPDATE (TAG 0x03) handler
* refer to StaRecUpdateRaUpdate
*/
static INT32 UniCmdStaRecRAUpdate(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg,
	VOID *pHandle)
{
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	P_CMD_STAREC_AUTO_RATE_UPDATE_T pCmdStaRecAutoRateUpdate = pHandle;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;

	if (pEntry) {
		os_zero_mem(pCmdStaRecAutoRateUpdate, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
		StaRecAutoRateUpdate(&pEntry->RaEntry, &pEntry->RaInternal,
							 pStaRecCfg->pRaParam, pCmdStaRecAutoRateUpdate);

		pCmdStaRecAutoRateUpdate->u2Tag = UNI_CMD_STAREC_RA_UPDATE;
		pCmdStaRecAutoRateUpdate->u2Length = sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T);
#ifdef RT_BIG_ENDIAN
		pCmdStaRecAutoRateUpdate->u2Tag = cpu2le16(pCmdStaRecAutoRateUpdate->u2Tag);
		pCmdStaRecAutoRateUpdate->u2Length = cpu2le16(pCmdStaRecAutoRateUpdate->u2Length);
#endif
		return NDIS_STATUS_SUCCESS;
	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	return NDIS_STATUS_FAILURE;
}

/*
* Unified command UNI_CMD_STAREC_HT_BASIC (TAG 0x09) handler
* refer to StaRecUpdateHtInfo
*/
static INT32 UniCmdStaRecHTBasic(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg,
	VOID *pHandle)
{
	P_CMD_STAREC_HT_INFO_T pCmdStaRecHtInfo = (P_CMD_STAREC_HT_INFO_T)pHandle;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;

	if (pEntry) {
		os_zero_mem(pCmdStaRecHtInfo, sizeof(CMD_STAREC_HT_INFO_T));
		pCmdStaRecHtInfo->u2Tag = UNI_CMD_STAREC_HT_BASIC;
		pCmdStaRecHtInfo->u2Length = sizeof(CMD_STAREC_HT_INFO_T);
		/* FIXME: may need separate function to compose the payload */
		os_move_mem(&(pCmdStaRecHtInfo->u2HtCap), &(pEntry->HTCapability.HtCapInfo),
					sizeof(pCmdStaRecHtInfo->u2HtCap));
#ifdef RT_BIG_ENDIAN
		pCmdStaRecHtInfo->u2HtCap = cpu2le16(pCmdStaRecHtInfo->u2HtCap);
		pCmdStaRecHtInfo->u2Tag = cpu2le16(pCmdStaRecHtInfo->u2Tag);
		pCmdStaRecHtInfo->u2Length = cpu2le16(pCmdStaRecHtInfo->u2Length);
#endif
		return NDIS_STATUS_SUCCESS;
	}

	return NDIS_STATUS_FAILURE;
}

/*
* Unified command UNI_CMD_STAREC_VHT_BASIC (TAG 0x0A) handler
* refer to StaRecUpdateVhtInfo
*/
static INT32 UniCmdStaRecVHTBasic(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg,
	VOID *pHandle)
{
	P_CMD_STAREC_VHT_INFO_T pCmdStaRecVHtInfo = (P_CMD_STAREC_VHT_INFO_T) pHandle;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;

	if (pEntry) {
		os_zero_mem(pCmdStaRecVHtInfo, sizeof(CMD_STAREC_VHT_INFO_T));
		pCmdStaRecVHtInfo->u2Tag = UNI_CMD_STAREC_VHT_BASIC;
		pCmdStaRecVHtInfo->u2Length = sizeof(CMD_STAREC_VHT_INFO_T);
		/* FIXME: may need separate function to compose the payload */
		os_move_mem(&(pCmdStaRecVHtInfo->u4VhtCap),
					&(pEntry->vht_cap_ie.vht_cap),
					sizeof(pCmdStaRecVHtInfo->u4VhtCap));
		os_move_mem(&(pCmdStaRecVHtInfo->u2VhtRxMcsMap),
					&(pEntry->vht_cap_ie.mcs_set.rx_mcs_map),
					sizeof(pCmdStaRecVHtInfo->u2VhtRxMcsMap));
		os_move_mem(&(pCmdStaRecVHtInfo->u2VhtTxMcsMap),
					&(pEntry->vht_cap_ie.mcs_set.tx_mcs_map),
					sizeof(pCmdStaRecVHtInfo->u2VhtTxMcsMap));

		if (IS_VHT_STA(pEntry) && !IS_HE_2G_STA(pEntry->cap.modes)) {
				UCHAR ucRTSBWSig = wlan_config_get_vht_bw_sig(pEntry->wdev);
				/* for StaRec: 0-disable DynBW, 1-static BW, 2 Dynamic BW */
				pCmdStaRecVHtInfo->ucRTSBWSig = ucRTSBWSig;
		}

#ifdef RT_BIG_ENDIAN
		pCmdStaRecVHtInfo->u2Tag = cpu2le16(pCmdStaRecVHtInfo->u2Tag);
		pCmdStaRecVHtInfo->u2Length = cpu2le16(pCmdStaRecVHtInfo->u2Length);
		pCmdStaRecVHtInfo->u4VhtCap = cpu2le32(pCmdStaRecVHtInfo->u4VhtCap);
		pCmdStaRecVHtInfo->u2VhtRxMcsMap = cpu2le16(pCmdStaRecVHtInfo->u2VhtRxMcsMap);
		pCmdStaRecVHtInfo->u2VhtTxMcsMap = cpu2le16(pCmdStaRecVHtInfo->u2VhtTxMcsMap);
#endif
		return NDIS_STATUS_SUCCESS;
	}

	return NDIS_STATUS_FAILURE;
}

/*
* Unified command UNI_CMD_STAREC_WTBL (TAG 0x0D) handler
* refer to StaRecUpdateWtbl
*/
static INT32 UniCmdStaRecWTBL(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg,
	VOID *pHandle)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	MAC_TABLE_ENTRY	*pEntry = pStaRecCfg->pEntry;
	P_UNI_CMD_CMD_STAREC_WTBL_T	pStarec_wtbl = (P_UNI_CMD_CMD_STAREC_WTBL_T) pHandle;
	BOOLEAN		IsBCMCWCID = FALSE;
	CMD_WTBL_GENERIC_T	rWtblGeneric = {0};	/* Tag = 0, Generic */
	CMD_WTBL_RX_T		rWtblRx = {0};		/* Tage = 1, Rx */
#ifdef DOT11_N_SUPPORT
	CMD_WTBL_HT_T		rWtblHt = {0};		/* Tag = 2, HT */
#ifdef DOT11_VHT_AC
	CMD_WTBL_VHT_T		rWtblVht = {0};		/* Tag = 3, VHT */
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	CMD_WTBL_TX_PS_T	rWtblTxPs = {0};	/* Tag = 5, TxPs */
	CMD_WTBL_HDR_TRANS_T	rWtblHdrTrans = {0};	/* Tag = 6, Hdr Trans */
	CMD_WTBL_RDG_T		rWtblRdg = {0};		/* Tag = 9, Rdg */
#ifdef TXBF_SUPPORT
	CMD_WTBL_BF_T           rWtblBf = {0};		/* Tag = 12, BF */
#endif /* TXBF_SUPPORT */
	CMD_WTBL_SMPS_T		rWtblSmPs = {0};	/* Tag = 13, SMPS */
	CMD_WTBL_SPE_T          rWtblSpe = {0};		/* Tag = 16, SPE */
	UCHAR		*pTlvBuffer = NULL;
	UCHAR		*pTempBuffer = NULL;
	UINT32		u4TotalTlvLen = 0;
	UCHAR		ucTotalTlvNumber = 0;
	P_CMD_WTBL_UPDATE_T	pCmdWtblUpdate = NULL;
	/* Allocate TLV msg */
	os_zero_mem(pStarec_wtbl, sizeof(UNI_CMD_CMD_STAREC_WTBL_T) + MAX_BUF_SIZE_OF_WTBL_INFO);
	pStarec_wtbl->u2Tag = UNI_CMD_STAREC_WTBL;
	pStarec_wtbl->u2Length = sizeof(UNI_CMD_CMD_STAREC_WTBL_T) + MAX_BUF_SIZE_OF_WTBL_INFO;

#ifdef RT_BIG_ENDIAN
	pStarec_wtbl->u2Tag = cpu2le16(pStarec_wtbl->u2Tag);
	pStarec_wtbl->u2Length = cpu2le16(pStarec_wtbl->u2Length);
#endif

	if (pStaRecCfg->ConnectionType == CONNECTION_INFRA_BC)
		IsBCMCWCID = TRUE;

	/* Tag = 1 */
	rWtblRx.ucRv = 1;
	rWtblRx.ucRca2 = 1;

	if (IsBCMCWCID) {
		/* Tag = 0 */
		struct _STA_TR_ENTRY *tr_entry = NULL;
		UINT32 bcmc_wdev_type = 0;

		rWtblGeneric.ucMUARIndex = 0x0e;

		os_move_mem(rWtblGeneric.aucPeerAddress, BROADCAST_ADDR, MAC_ADDR_LEN);

		/* tmp solution to update STA mode AP address */
		tr_entry = &tr_ctl->tr_entry[pStaRecCfg->u2WlanIdx];

		if (tr_entry) {
			if (tr_entry->wdev)
				bcmc_wdev_type = tr_entry->wdev->wdev_type;

			ASSERT(tr_entry->EntryType == ENTRY_CAT_MCAST);
		}

		if ((bcmc_wdev_type == WDEV_TYPE_STA) ||
			(bcmc_wdev_type == WDEV_TYPE_REPEATER) ||
			(bcmc_wdev_type == WDEV_TYPE_ADHOC)) {
			WIFI_SYS_INFO_T *pWifiSysInfo = &pAd->WifiSysInfo;
			WIFI_INFO_CLASS_T *pWifiClass = &pWifiSysInfo->BssInfo;
			BSS_INFO_ARGUMENT_T *pBssInfo = NULL;

			OS_SEM_LOCK(&pWifiSysInfo->lock);

			DlListForEach(pBssInfo, &pWifiClass->Head, BSS_INFO_ARGUMENT_T, list) {

				if (pBssInfo->ucBssIndex == pStaRecCfg->ucBssIndex) {
					/* Update to AP MAC when in STA mode */
					os_move_mem(rWtblGeneric.aucPeerAddress, pBssInfo->Bssid, MAC_ADDR_LEN);
				}
			}

			OS_SEM_UNLOCK(&pWifiSysInfo->lock);
		}

		/* Tag = 1 */
		rWtblRx.ucRca1 = 1;

		/* Tag = 6 */
		if (pAd->OpMode == OPMODE_AP) {
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 0;
		}
	} else {

		struct wifi_dev *wdev = NULL;

		if (pEntry == NULL) {

			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "\n\nERROR: No MacEntry. widx=%d ConnTyp=0x%x feat=0x%x\n\n\n",
					  pStaRecCfg->u2WlanIdx, pStaRecCfg->ConnectionType, pStaRecCfg->u4EnableFeature);

			return NDIS_STATUS_FAILURE;
		}

		wdev = pEntry->wdev;

		/* Tag = 0 */
		/* rWtblGeneric.ucMUARIndex = pEntry->wdev->OmacIdx; */
		rWtblGeneric.ucQos = CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE) ? 1 : 0;
		rWtblGeneric.u2PartialAID = cpu2le16(pEntry->Aid);
		rWtblGeneric.ucMUARIndex = pStaRecCfg->MuarIdx;

		/* Tag = 0 */
		os_move_mem(rWtblGeneric.aucPeerAddress, pEntry->Addr, MAC_ADDR_LEN);

		if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE)
			&& CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET))
			rWtblGeneric.ucAadOm = 1;

		/* Tag = 1 */
		if (wdev->wdev_type == WDEV_TYPE_STA || wdev->wdev_type == WDEV_TYPE_REPEATER)
			rWtblRx.ucRca1 = 1;

		/* Tag = 6 */
		if (wdev->wdev_type == WDEV_TYPE_STA || wdev->wdev_type == WDEV_TYPE_REPEATER) {
			rWtblHdrTrans.ucFd = 0;
			rWtblHdrTrans.ucTd = 1;
		} else if (wdev->wdev_type == WDEV_TYPE_AP) {
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 0;
		} else if (wdev->wdev_type == WDEV_TYPE_WDS) {
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 1;
		} else
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Unknown wdev type(%d) do not support header translation\n",
					  pEntry->wdev->wdev_type);

#ifdef A4_CONN
		if (IS_ENTRY_A4(pEntry) && (wdev->wdev_type == WDEV_TYPE_STA)) {
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 1;
		}
#endif
#ifdef APCLI_AS_WDS_STA_SUPPORT
		if ((wdev->wdev_type == WDEV_TYPE_STA) && (wdev->wds_enable) && pEntry->bEnable4Addr) {
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 1;
		}
#endif
#ifdef HDR_TRANS_RX_SUPPORT

		if (IS_CIPHER_TKIP_Entry(pEntry))
			rWtblHdrTrans.ucDisRhtr = 1;
		else
			rWtblHdrTrans.ucDisRhtr = 0;

#endif /* HDR_TRANS_RX_SUPPORT */
#ifdef DOT11_N_SUPPORT

		if (IS_HT_STA(pEntry)) {
			/* Tag = 0 */
			rWtblGeneric.ucQos = 1;
			rWtblGeneric.ucBafEn = 0;

			/* Tag = 2 */
			rWtblHt.ucHt = 1;
			rWtblHt.ucMm = pEntry->MpduDensity;
			rWtblHt.ucAf = pEntry->MaxRAmpduFactor;

			if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE)) {
				/* Tga = 9 */
				rWtblRdg.ucR = 1;

				if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_P18(pAd) || IS_MT7663(pAd) ||
					IS_AXE(pAd) || IS_MT7626(pAd) || IS_MT7915(pAd) || IS_MT7986(pAd) ||
					IS_MT7916(pAd) || IS_MT7981(pAd))
					rWtblRdg.ucRdgBa = 0;
				else
					rWtblRdg.ucRdgBa = 1;
			}

			/* Tag = 13*/
			if (pEntry->MmpsMode == MMPS_DYNAMIC)
				rWtblSmPs.ucSmPs = 1;
			else
				rWtblSmPs.ucSmPs = 0;

#ifdef DOT11_VHT_AC
			/* Tag = 3 */
			if (IS_VHT_STA(pEntry) && !IS_HE_2G_STA(pEntry->cap.modes)) {
				UCHAR ucDynBw = wlan_config_get_vht_bw_sig(pEntry->wdev);
				rWtblVht.ucVht = 1;
				if (ucDynBw == BW_SIGNALING_DYNAMIC)
					rWtblVht.ucDynBw = 1;
				else
					rWtblVht.ucDynBw = 0;
			}
#endif /* DOT11_VHT_AC */
		}

#endif /* DOT11_N_SUPPORT */
	}

	/* Tag = 5 */
	rWtblTxPs.ucTxPs = 0;
#ifdef TXBF_SUPPORT

	if (!IsBCMCWCID) {
		/* Tag = 0xc */
		rWtblBf.ucGid     = 0;
		rWtblBf.ucPFMUIdx = pAd->rStaRecBf.u2PfmuId;

		if (IS_HT_STA(pEntry)) {
			if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn)
				rWtblBf.ucTiBf = IS_ITXBF_SUP(pEntry->rStaRecBf.u1TxBfCap);
			else
				rWtblBf.ucTiBf = FALSE;

			if (pAd->CommonCfg.ETxBfEnCond)
				rWtblBf.ucTeBf = IS_ETXBF_SUP(pEntry->rStaRecBf.u1TxBfCap);
			else
				rWtblBf.ucTeBf	  = FALSE;
		}

		if (IS_VHT_STA(pEntry)) {
			if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn)
				rWtblBf.ucTibfVht = IS_ITXBF_SUP(pEntry->rStaRecBf.u1TxBfCap);
			else
				rWtblBf.ucTibfVht = FALSE;

			if (pAd->CommonCfg.ETxBfEnCond)
				rWtblBf.ucTebfVht = IS_ETXBF_SUP(pEntry->rStaRecBf.u1TxBfCap);
			else
				rWtblBf.ucTebfVht = FALSE;
		}
	}

#endif /* TXBF_SUPPORT */
	/* Tag = 0x10 */
	rWtblSpe.ucSpeIdx = 0;
	/* From WTBL COMMAND in STEREC TLV */
	pCmdWtblUpdate = (P_CMD_WTBL_UPDATE_T)&pStarec_wtbl->aucBuffer[0];
	pTlvBuffer = &pCmdWtblUpdate->aucBuffer[0];
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
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_HDR_TRANS),
					  (sizeof(CMD_WTBL_HDR_TRANS_T)),
					  &rWtblHdrTrans,
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
	pCmdWtblUpdate->u2TotalElementNum = cpu2le16(ucTotalTlvNumber);
	WCID_SET_H_L(pCmdWtblUpdate->ucWlanIdxHnVer, pCmdWtblUpdate->ucWlanIdxL, pStaRecCfg->u2WlanIdx);
	pCmdWtblUpdate->ucOperation = RESET_WTBL_AND_SET; /* In STAREC, currently reset and set only. */
	return NDIS_STATUS_SUCCESS;
}

/*
* Unified command UNI_CMD_STAREC_HE_BASIC (TAG 0x0E) handler
* refer to sta_rec_update_he_info
*/
static INT32 UniCmdStaRecHEInfo(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg,
	VOID *pHandle)
{
	P_UNI_CMD_STAREC_HE_INFO_T phe_info = (P_UNI_CMD_STAREC_HE_INFO_T) pHandle;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	struct he_sta_mac_info *mac_info = &pStaRecCfg->he_sta.mac_info;
	struct he_sta_phy_info *phy_info = &pStaRecCfg->he_sta.phy_info;
	struct he_mcs_info *mcs = &pStaRecCfg->he_sta.max_nss_mcs;
	int i;

	if (pEntry) {
		os_zero_mem(phe_info, sizeof(UNI_CMD_STAREC_HE_INFO_T));
		phe_info->u2Tag = UNI_CMD_STAREC_HE_BASIC;
		phe_info->u2Length = sizeof(UNI_CMD_STAREC_HE_INFO_T);
		/*mac info*/
		phe_info->u4HeCap |= (mac_info->bqr_support << STA_REC_HE_CAP_BQR);
		phe_info->u4HeCap |= (mac_info->htc_support << STA_REC_HE_CAP_HTC);
		phe_info->u4HeCap |= (mac_info->bsr_support << STA_REC_HE_CAP_BSR);
		phe_info->u4HeCap |= (mac_info->om_support << STA_REC_HE_CAP_OM);
		phe_info->u4HeCap |= (mac_info->amsdu_in_ampdu_support << STA_REC_HE_CAP_AMSDU_IN_AMPDU);
		phe_info->ucMaxAmpduLenExponent = mac_info->max_ampdu_len_exp;
		phe_info->ucMaxAmpduLenExponentExtension = 0; /* TODO: Check this value */
		phe_info->ucTrigerFrameMacPadDuration = mac_info->trigger_frame_mac_pad_dur;
		/*phy info*/
		phe_info->u4HeCap |= (phy_info->dual_band_support << STA_REC_HE_CAP_DUAL_BAND);
		phe_info->u4HeCap |= (phy_info->ldpc_support << STA_REC_HE_CAP_LDPC);
		phe_info->u4HeCap |= (phy_info->triggered_cqi_feedback_support << STA_REC_HE_CAP_TRIG_CQI_FK);
		phe_info->u4HeCap |= (phy_info->partial_bw_ext_range_support << STA_REC_HE_CAP_PARTIAL_BW_EXT_RANGE);
		phe_info->u4HeCap |= (phy_info->bw20_242tone << STA_REC_HE_CAP_BW20_RU242_SUPPORT);
		phe_info->ucChBwSet = phy_info->ch_width_set;
		phe_info->ucDeviceClass = phy_info->device_class;
		phe_info->ucPuncPreamRx = phy_info->punctured_preamble_rx;
		phe_info->ucPktExt = 2;	/* force Packet Extension as 16 us by default */
		phe_info->ucDcmTxMode = phy_info->dcm_cap_tx;
		phe_info->ucDcmRxMode = phy_info->dcm_cap_rx;
		phe_info->ucDcmTxMaxNss = phy_info->dcm_max_nss_tx;
		phe_info->ucDcmRxMaxNss = phy_info->dcm_max_nss_rx;
		phe_info->ucDcmMaxRu = phy_info->dcm_max_ru;
		/*1024QAM*/
		phe_info->u4HeCap |= (phy_info->tx_le_ru242_1024qam << STA_REC_HE_CAP_TX_1024QAM_UNDER_RU242);
		phe_info->u4HeCap |= (phy_info->rx_le_ru242_1024qam << STA_REC_HE_CAP_RX_1024QAM_UNDER_RU242);
		/*STBC*/
		if (phy_info->stbc_support & HE_LE_EQ_80M_TX_STBC)
			phe_info->u4HeCap |= (1 << STA_REC_HE_CAP_LE_EQ_80M_TX_STBC);
		if (phy_info->stbc_support & HE_LE_EQ_80M_RX_STBC)
			phe_info->u4HeCap |= (1 << STA_REC_HE_CAP_LE_EQ_80M_RX_STBC);
		if (phy_info->stbc_support & HE_GT_80M_RX_STBC)
			phe_info->u4HeCap |= (1 << STA_REC_HE_CAP_GT_80M_RX_STBC);
		if (phy_info->stbc_support & HE_GT_80M_TX_STBC)
			phe_info->u4HeCap |= (1 << STA_REC_HE_CAP_GT_80M_TX_STBC);
		/*GI*/
		if (phy_info->gi_cap & HE_SU_PPDU_1x_LTF_DOT8US_GI)
			phe_info->u4HeCap |= (1 << STA_REC_HE_CAP_SU_PPDU_1x_LTF_DOT8US_GI);
		if (phy_info->gi_cap & HE_SU_PPDU_MU_PPDU_4x_LTF_DOT8US_GI)
			phe_info->u4HeCap |= (1 << STA_REC_HE_CAP_SU_PPDU_MU_PPDU_4x_LTF_DOT8US_GI);
		if (phy_info->gi_cap & HE_ER_SU_PPDU_1x_LTF_DOT8US_GI)
			phe_info->u4HeCap |= (1 << STA_REC_HE_CAP_ER_SU_PPDU_1x_LTF_DOT8US_GI);
		if (phy_info->gi_cap & HE_ER_SU_PPDU_4x_LTF_DOT8US_GI)
			phe_info->u4HeCap |= (1 << STA_REC_HE_CAP_ER_SU_PPDU_4x_LTF_DOT8US_GI);
		if (phy_info->gi_cap & HE_NDP_4x_LTF_3DOT2MS_GI)
			phe_info->u4HeCap |= (1 << STA_REC_HE_CAP_NDP_4x_LTF_3DOT2MS_GI);
		/*MAX NSS MCS*/
		for (i = 0 ; i < HE_MAX_SUPPORT_STREAM; i++) {
			phe_info->au2MaxNssMcs[CMD_HE_MCS_BW80] |= (mcs->bw80_mcs[i] << (i * 2));
			phe_info->au2MaxNssMcs[CMD_HE_MCS_BW160] |= (mcs->bw160_mcs[i] << (i * 2));
			phe_info->au2MaxNssMcs[CMD_HE_MCS_BW8080] |= (mcs->bw8080_mcs[i] << (i * 2));
		}
#ifdef RT_BIG_ENDIAN
		phe_info->u2Tag = cpu2le16(phe_info->u2Tag);
		phe_info->u2Length = cpu2le16(phe_info->u2Length);
		phe_info->u4HeCap = cpu2le32(phe_info->u4HeCap);
		for (i = 0 ; i < CMD_HE_MCS_BW_NUM ; i++)
			phe_info->au2MaxNssMcs[i] = cpu2le16(phe_info->au2MaxNssMcs[i]);
#endif
		return NDIS_STATUS_SUCCESS;
	}

	return NDIS_STATUS_FAILURE;
}

/*
* Unified command UNI_CMD_STAREC_HW_AMSDU (TAG 0x0F) handler
* refer to StaRecUpdateHwAmsdu
*/
static INT32 UniCmdStaRecHwAmsdu(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg,
	VOID *pHandle)
{
	P_CMD_STAREC_AMSDU_T pCmdStaRecAmsdu = (P_CMD_STAREC_AMSDU_T) pHandle;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pEntry) {
		os_zero_mem(pCmdStaRecAmsdu, sizeof(CMD_STAREC_AMSDU_T));
		pCmdStaRecAmsdu->u2Tag = UNI_CMD_STAREC_HW_AMSDU;
		pCmdStaRecAmsdu->u2Length = sizeof(CMD_STAREC_AMSDU_T);
		pCmdStaRecAmsdu->ucMaxMpduSize = pEntry->AMsduSize;
		pCmdStaRecAmsdu->ucMaxAmsduNum = cap->hw_max_amsdu_nums;
		pCmdStaRecAmsdu->ucAmsduEnable = TRUE;
#ifdef RT_BIG_ENDIAN
		pCmdStaRecAmsdu->u2Tag = cpu2le16(pCmdStaRecAmsdu->u2Tag);
		pCmdStaRecAmsdu->u2Length = cpu2le16(pCmdStaRecAmsdu->u2Length);
#endif
		return NDIS_STATUS_SUCCESS;
	}

	return NDIS_STATUS_FAILURE;
}


/*
* Unified command UNI_CMD_STAREC_INSTALL_KEY (TAG 0x0C)
* and UNI_CMD_STAREC_INSTALL_KEY_V2 (TAG 11) handler
* refer to StaRecUpdateSecKey
*/
static INT32 UniCmdStaRecInstallKeyFeature(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg,
	VOID *pHandle)
{
	ASIC_SEC_INFO *asic_sec_info = &pStaRecCfg->asic_sec_info;
	UINT32 cmd_len = 0;
	INT32  Ret = NDIS_STATUS_SUCCESS;
#ifdef RT_BIG_ENDIAN
	P_CMD_WTBL_SECURITY_KEY_V2_T wtbl_security_key_ptr =
		(P_CMD_WTBL_SECURITY_KEY_V2_T) pHandle;
#endif

	Ret = chip_fill_key_install_uni_cmd(pAd->hdev_ctrl, asic_sec_info, STAREC_SEC_KEY_METHOD, pHandle, &cmd_len);

#ifdef RT_BIG_ENDIAN
	wtbl_security_key_ptr->u2Length = cpu2le16(wtbl_security_key_ptr->u2Length);
	wtbl_security_key_ptr->u2Tag = cpu2le16(wtbl_security_key_ptr->u2Tag);
#endif

	return Ret;
}

/*
* Unified command UNI_CMD_STAREC_BF (TAG 0x04)
* refer to StaRecUpdateBf
*/
#ifdef TXBF_SUPPORT
static INT32 UniCmdStaRecUpdateBf(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg,
	VOID *pHandle)
{
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	P_UNI_CMD_STAREC_BF_T pCmdStaRecBf = (P_UNI_CMD_STAREC_BF_T) pHandle;

	if (pEntry) {
		os_zero_mem(pCmdStaRecBf, sizeof(UNI_CMD_STAREC_BF_T));
		UniCmdStaRecBfUpdate(pEntry, pCmdStaRecBf);
		return NDIS_STATUS_SUCCESS;
	}

	return NDIS_STATUS_FAILURE;
}
#endif

/*
* Unified command UNI_CMD_STAREC_AP_PS (TAG 0x0b)
* refer to StaRecUpdateApPs
*/
static INT32 UniCmdStaRecUpdateApPs(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg,
	VOID *pHandle)
{
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	P_CMD_STAREC_PS_T pCmdPsInfo = (P_CMD_STAREC_PS_T) pHandle;
	UINT8 IdApsd;
	UINT8 ACTriSet = 0;
	UINT8 ACDelSet = 0;

	if (pEntry) {
		/* Fill TLV format */
		pCmdPsInfo->u2Tag = UNI_CMD_STAREC_AP_PS;
		pCmdPsInfo->u2Length = sizeof(CMD_STAREC_PS_T);
		/* Find Triggerable AC */
		/* Find Deliverable AC */
		ACTriSet = 0;
		ACDelSet = 0;

		for (IdApsd = 0; IdApsd < 4; IdApsd++) {
			if (pEntry->bAPSDCapablePerAC[IdApsd])
				ACTriSet |= 1 << IdApsd;

			if (pEntry->bAPSDDeliverEnabledPerAC[IdApsd])
				ACDelSet |= 1 << IdApsd;
		}

		pCmdPsInfo->ucStaBmpTriggerAC = ACTriSet;
		pCmdPsInfo->ucStaBmpDeliveryAC = ACDelSet;
		pCmdPsInfo->ucStaMaxSPLength = pStaRecCfg->pEntry->MaxSPLength;
		pCmdPsInfo->u2StaListenInterval = 0; /* TODO: */
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "%s(STA_REC_AP_PS), Delv=%x Trig=%x SP=%d LInt=%d",
				  __func__,
				  pCmdPsInfo->ucStaBmpDeliveryAC,
				  pCmdPsInfo->ucStaBmpTriggerAC,
				  pCmdPsInfo->ucStaMaxSPLength,
				  pCmdPsInfo->u2StaListenInterval);
#ifdef RT_BIG_ENDIAN
		pCmdPsInfo->u2Tag = cpu2le16(pCmdPsInfo->u2Tag);
		pCmdPsInfo->u2Length = cpu2le16(pCmdPsInfo->u2Length);
		pCmdPsInfo->u2StaListenInterval = cpu2le16(pCmdPsInfo->u2StaListenInterval);
#endif
		return NDIS_STATUS_SUCCESS;
	}

	return NDIS_STATUS_FAILURE;
}

/*
* Unified command UNI_CMD_STAREC_MURU (TAG 0x12)
* refer to sta_rec_update_muru_info
*/
static INT32 UniCmdStaRecUpdateMuruInfo(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg,
	VOID *pHandle)
{
	P_CMD_STAREC_MURU_T pmuru_info = (P_CMD_STAREC_MURU_T) pHandle;
	P_MURU_WDEV_CFG wdev_cfg = &pmuru_info->rMuRuStaCap.rWdevCfg;
	P_MURU_STA_DL_OFDMA dl_ofdma = &pmuru_info->rMuRuStaCap.rDlOfdma;
	P_MURU_STA_UL_OFDMA ul_ofdma = &pmuru_info->rMuRuStaCap.rUlOfdma;
	P_MURU_STA_DL_MIMO dl_mimo = &pmuru_info->rMuRuStaCap.rDlMimo;
	P_MURU_STA_UL_MIMO ul_mimo = &pmuru_info->rMuRuStaCap.rUlMimo;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	struct wifi_dev *wdev = pEntry->wdev;

	os_zero_mem(&pmuru_info, sizeof(CMD_STAREC_MURU_T));

	if (pEntry) {
		pmuru_info->u2Tag = UNI_CMD_STAREC_MURU;
		pmuru_info->u2Length = sizeof(CMD_STAREC_MURU_T);
#ifdef RT_BIG_ENDIAN
		pmuru_info->u2Tag = cpu2le16(pmuru_info->u2Tag);
		pmuru_info->u2Length	= cpu2le16(pmuru_info->u2Length);
#endif
		/* global wdev setting */
		wdev_cfg->fgDlOfdmaEn = wlan_config_get_mu_dl_ofdma(wdev);
		wdev_cfg->fgUlOfdmaEn = wlan_config_get_mu_ul_ofdma(wdev);
		wdev_cfg->fgDlMimoEn = wlan_config_get_mu_dl_mimo(wdev);
		wdev_cfg->fgUlMimoEn = wlan_config_get_mu_ul_mimo(wdev);

		/* Sta Cap. of DL OFDMA */
		dl_ofdma->u1PhyPunRx = pEntry->cap.punc_preamble_rx;
		dl_ofdma->u120MIn40M2G = (pEntry->cap.he_phy_cap & HE_24G_20M_IN_40M_PPDU) ? 1 : 0;
		dl_ofdma->u120MIn160M = (pEntry->cap.he_phy_cap & HE_20M_IN_160M_8080M_PPDU) ? 1 : 0;
		dl_ofdma->u180MIn160M = (pEntry->cap.he_phy_cap & HE_80M_IN_160M_8080M_PPDU) ? 1 : 0;
		dl_ofdma->u1Lt16SigB = 0;				/* Wait he_phy_cap to support the cap */
		dl_ofdma->u1RxSUCompSigB = 0;			/* Wait he_phy_cap to support the cap */
		dl_ofdma->u1RxSUNonCompSigB = 0;		/* Wait he_phy_cap to support the cap */

		/* Sta Cap. of UL OFDMA */
		ul_ofdma->u1TrigFrmPad = pEntry->cap.tf_mac_pad_duration;
		ul_ofdma->u1MuCascading = (pEntry->cap.he_mac_cap & HE_MU_CASCADING) ? 1 : 0;
		ul_ofdma->u1UoRa = (pEntry->cap.he_mac_cap & HE_OFDMA_RA) ? 1 : 0;
		ul_ofdma->u12x996Tone = 0;				/* Wait he_mac_cap to support the cap */
		ul_ofdma->u1RxTrgFrmBy11ac = 0;			/* Wait he_mac_cap to support the cap */
		ul_ofdma->u1RxCtrlFrmToMBss = 0;		/* TODO: Check this value */

		/* Sta Cap. of DL MIMO */
		dl_mimo->fgVhtMuBfee = pEntry->vht_cap_ie.vht_cap.bfee_cap_mu;
		dl_mimo->fgParBWDlMimo = (pEntry->cap.he_phy_cap & HE_PARTIAL_BW_DL_MU_MIMO) ? 1 : 0;

		/* Sta Cap. of UL MIMO */
		ul_mimo->fgFullUlMimo = (pEntry->cap.he_phy_cap & HE_FULL_BW_UL_MU_MIMO) ? 1 : 0;
		ul_mimo->fgParUlMimo = (pEntry->cap.he_phy_cap & HE_PARTIAL_BW_UL_MU_MIMO) ? 1 : 0;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"fgDlOfdmaEn = 0x%02X, fgUlOfdmaEn = 0x%02X\n",
				wdev_cfg->fgDlOfdmaEn, wdev_cfg->fgUlOfdmaEn);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"fgDlMimoEn = 0x%02X, fgUlMimoEn= 0x%02X\n",
				wdev_cfg->fgDlMimoEn, wdev_cfg->fgUlMimoEn);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"u1PhyPunRx = 0x%02X, u120MIn40M2G = 0x%02X\n",
				dl_ofdma->u1PhyPunRx, dl_ofdma->u120MIn40M2G);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"u120MIn160M = 0x%02X, u180MIn160M= 0x%02X\n",
				dl_ofdma->u120MIn160M, dl_ofdma->u180MIn160M);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"u1Lt16SigB = 0x%02X, u1RxSUCompSigB = 0x%02X\n",
				dl_ofdma->u1Lt16SigB, dl_ofdma->u1RxSUCompSigB);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"u1RxSUNonCompSigB = 0x%02X\n",
				dl_ofdma->u1RxSUNonCompSigB);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"u1TrigFrmPad = 0x%02X, u1MuCascading = 0x%02X\n",
				ul_ofdma->u1TrigFrmPad, ul_ofdma->u1MuCascading);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"u1UoRa = 0x%02X, u12x996Tone= 0x%02X\n",
				ul_ofdma->u1UoRa, ul_ofdma->u12x996Tone);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"u1RxTrgFrmBy11ac = 0x%02X\n", ul_ofdma->u1RxTrgFrmBy11ac);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"u1RxCtrlFrmToMBss = 0x%02X\n", ul_ofdma->u1RxCtrlFrmToMBss);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"fgVhtMuBfee = 0x%02X, fgParBWDlMimo = 0x%02X\n",
				dl_mimo->fgVhtMuBfee, dl_mimo->fgParBWDlMimo);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"fgFullUlMimo = 0x%02X, fgParUlMimo = 0x%02X\n",
				ul_mimo->fgFullUlMimo, ul_mimo->fgParUlMimo);

		return NDIS_STATUS_SUCCESS;
	}

	return NDIS_STATUS_FAILURE;
}

/*
* Unified command UNI_CMD_STAREC_BFEE (TAG 0x14)
* refer to StaRecUpdateBfee
*/
#ifdef TXBF_SUPPORT
static INT32 UniCmdStaRecUpdateBfee(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg,
	VOID *pHandle)
{
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	P_CMD_STAREC_BFEE pCmdStaRecBfee;

	if (pEntry) {
		os_zero_mem(pCmdStaRecBfee, sizeof(CMD_STAREC_BFEE));
		StaRecBfeeUpdate(pEntry, pCmdStaRecBfee);

		pCmdStaRecBfee->u2Tag = UNI_CMD_STAREC_BFEE;
		pCmdStaRecBfee->u2Length = sizeof(CMD_STAREC_BFEE);
#ifdef RT_BIG_ENDIAN
		pCmdStaRecBfee->u2Tag = cpu2le16(pCmdStaRecBfee->u2Tag);
		pCmdStaRecBfee->u2Length = cpu2le16(pCmdStaRecBfee->u2Length);
#endif
		return NDIS_STATUS_SUCCESS;
	}

	return NDIS_STATUS_FAILURE;
}
#endif

/*
* Unified command UNI_CMD_STAREC_HE_6G_CAP (TAG 0x17)
*/
static INT32 UniCmdStaRecUpdate6GCap(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg,
	VOID *pHandle)
{
	P_CMD_STAREC_HE_6G_CAP_T pCmdStaRecHE6GCap = (P_CMD_STAREC_HE_6G_CAP_T) pHandle;
	pCmdStaRecHE6GCap->u2Tag = UNI_CMD_STAREC_HE_6G_CAP;
	pCmdStaRecHE6GCap->u2Length = sizeof(CMD_STAREC_HE_6G_CAP_T);
	pCmdStaRecHE6GCap->u2He6gBandCapInfo = 0; /* TODO: Check this value */
#ifdef RT_BIG_ENDIAN
	pCmdStaRecHE6GCap->u2Tag     		 = cpu2le16(pCmdStaRecHE6GCap->u2Tag);
	pCmdStaRecHE6GCap->u2Length     	 = cpu2le16(pCmdStaRecHE6GCap->u2Length);
	pCmdStaRecHE6GCap->u2He6gBandCapInfo = cpu2le16(pCmdStaRecHE6GCap->u2He6gBandCapInfo);
#endif
	return NDIS_STATUS_SUCCESS;
}

static UNI_CMD_TAG_HANDLE_T UniCmdStaRecTab[] = {
	{
		.u2CmdFeature = STA_REC_BASIC_STA_RECORD_FEATURE,
		.u4StructSize = sizeof(CMD_STAREC_COMMON_T),
		.pfHandler = UniCmdStaRecBasic
	},
	{
		.u2CmdFeature = STA_REC_RA_FEATURE,
		.u4StructSize = sizeof(CMD_STAREC_AUTO_RATE_T),
		.pfHandler = UniCmdStaRecRA
	},
	{
		.u2CmdFeature = STA_REC_RA_UPDATE_FEATURE,
		.u4StructSize = sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T),
		.pfHandler =  UniCmdStaRecRAUpdate
	},
	{
		.u2CmdFeature = STA_REC_BASIC_HT_INFO_FEATURE,
		.u4StructSize = sizeof(CMD_STAREC_HT_INFO_T),
		.pfHandler = UniCmdStaRecHTBasic
	},
	{
		.u2CmdFeature = STA_REC_BASIC_VHT_INFO_FEATURE,
		.u4StructSize = sizeof(CMD_STAREC_VHT_INFO_T),
		.pfHandler = UniCmdStaRecVHTBasic
	},
	{
		.u2CmdFeature = STA_REC_INSTALL_KEY_FEATURE,
		.u4StructSize = 0, /* Calculated by UniCmdStaRecExtraAllocDynSizeCheck */
		.pfHandler = UniCmdStaRecInstallKeyFeature
	},
	{
		.u2CmdFeature = STA_REC_WTBL_FEATURE,
		.u4StructSize = sizeof(UNI_CMD_CMD_STAREC_WTBL_T) + MAX_BUF_SIZE_OF_WTBL_INFO,
		.pfHandler = UniCmdStaRecWTBL
	},
	{
		.u2CmdFeature = STA_REC_BASIC_HE_INFO_FEATURE,
		.u4StructSize = sizeof(UNI_CMD_STAREC_HE_INFO_T),
		.pfHandler = UniCmdStaRecHEInfo
	},
	{
		.u2CmdFeature = STA_REC_HW_AMSDU_FEATURE,
		.u4StructSize = sizeof(CMD_STAREC_AMSDU_T),
		.pfHandler = UniCmdStaRecHwAmsdu
	},
#ifdef TXBF_SUPPORT
	{
		.u2CmdFeature = STA_REC_BF_FEATURE,
		.u4StructSize = sizeof(P_UNI_CMD_STAREC_BF_T),
		.pfHandler = UniCmdStaRecUpdateBf
	},
#endif /* TXBF_SUPPORT */
	{
		.u2CmdFeature = STA_REC_AP_PS_FEATURE,
		.u4StructSize = sizeof(CMD_STAREC_PS_T),
		.pfHandler = UniCmdStaRecUpdateApPs
	},
	{
		.u2CmdFeature = STA_REC_MURU_FEATURE,
		.u4StructSize = sizeof(CMD_STAREC_MURU_T),
		.pfHandler = UniCmdStaRecUpdateMuruInfo
	},
#ifdef TXBF_SUPPORT
	{
		.u2CmdFeature = STA_REC_BFEE_FEATURE,
		.u4StructSize = sizeof(CMD_STAREC_BFEE),
		.pfHandler = UniCmdStaRecUpdateBfee
	},
#endif /* TXBF_SUPPORT */
	{
		.u2CmdFeature = STA_REC_HE_6G_CAP_FEATURE,
		.u4StructSize = sizeof(CMD_STAREC_HE_6G_CAP_T),
		.pfHandler = UniCmdStaRecUpdate6GCap
	},
};


UINT32 UniCmdStaRecExtraAllocDynSizeCheck(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T StaRecCfg)
{
	ASIC_SEC_INFO *asic_sec_info = &(StaRecCfg.asic_sec_info);
	UINT32 cmd_len = 0;
	UINT32 dynsize = 0;

	if (StaRecCfg.u4EnableFeature & STA_REC_INSTALL_KEY_FEATURE) {
		chip_fill_key_install_uni_cmd_dynsize_check(
			pAd->hdev_ctrl, asic_sec_info, &cmd_len);
	}
	dynsize += cmd_len;

	return dynsize;
}

INT32 UniCmdStaRecUpdate(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg)
{
	struct cmd_msg          *msg = NULL;
	INT32                   Ret = NDIS_STATUS_SUCCESS;
	UINT32                  i = 0;
	UINT16                  u2TLVNumber = 0;
	PUCHAR					pTempBuf = NULL;
	PUCHAR					pNextHeadBuf = NULL;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4RealUseBufSize = 0;
	UINT32					u4SendBufSize = 0;
	UINT32					u4RemainingPayloadSize = 0;
	UINT32					u4ComCmdSize = 0;
	P_UNI_CMD_STAREC_T   	pCmdStaRecUpdate = NULL;
	RTMP_CHIP_CAP			*cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32 					StaRecTabSize = (sizeof(UniCmdStaRecTab) / sizeof(UniCmdStaRecTab[0]));
	STA_REC_CFG_T StaRecCfg = *pStaRecCfg;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_STAREC_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < StaRecTabSize; i++) {
		if (StaRecCfg.u4EnableFeature & UniCmdStaRecTab[i].u2CmdFeature)
			u4CmdNeedMaxBufSize += UniCmdStaRecTab[i].u4StructSize;
	}
	u4CmdNeedMaxBufSize += UniCmdStaRecExtraAllocDynSizeCheck(pAd, StaRecCfg);

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdStaRecUpdate = (P_UNI_CMD_STAREC_T)pNextHeadBuf;
	pCmdStaRecUpdate->ucBssInfoIdx = StaRecCfg.ucBssIndex;
	WCID_SET_H_L(pCmdStaRecUpdate->ucWlanIdxHnVer,
		pCmdStaRecUpdate->ucWlanIdxL, StaRecCfg.u2WlanIdx);
	pCmdStaRecUpdate->ucMuarIdx = StaRecCfg.MuarIdx;
	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Traverse all support features */
	for (i = 0; i < StaRecTabSize; i++) {
		if (StaRecCfg.u4EnableFeature & UniCmdStaRecTab[i].u2CmdFeature) {
			if (UniCmdStaRecTab[i].pfHandler != NULL) {
				Ret = ((PFN_STAREC_HANDLE)(UniCmdStaRecTab[i].pfHandler))(pAd, &StaRecCfg, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += UniCmdStaRecTab[i].u4StructSize;
					u2TLVNumber++;
				}
			} else {
				MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "%s: StaRecTag = %d no corresponding function handler.\n",
						  __func__, UniCmdStaRecTab[i].u2CmdFeature);
			}
		}

		if (Ret != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"The hanlder of tag (0x%08x) return fail!\n", UniCmdStaRecTab[i].u2CmdFeature);
	}
	if (u2TLVNumber > 0) {
		pCmdStaRecUpdate->u2TotalElementNum = cpu2le16(u2TLVNumber);
		pCmdStaRecUpdate->ucAppendCmdTLV = TRUE;
	}


	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_STAREC_INFO);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if ((!bNeedFrag) || bLastFrag) {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
				SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
			} else {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			call_fw_cmd_notifieriers(WO_CMD_STA_REC, pAd, msg->net_pkt);
			Ret = AndesSendCmdMsg(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

/* Send unified command of "UNI_CMD_STAREC_BA" */
INT32 UniCmdStaRecBaUpdate(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_BA_CFG_T StaRecBaCfg)
{
	struct cmd_msg          *msg = NULL;
	INT32                   Ret = NDIS_STATUS_SUCCESS;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4ComCmdSize = 0;
	UNI_CMD_STAREC_T   		CmdStaRecUpdate;
	CMD_STAREC_BA_T			CmdStaRecBa;
	struct _CMD_ATTRIBUTE 	attr = {0};

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(CmdStaRecUpdate);
	os_zero_mem(&CmdStaRecUpdate, u4ComCmdSize);
	os_zero_mem(&CmdStaRecBa, sizeof(CmdStaRecBa));

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(CmdStaRecBa);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_STAREC_INFO);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
	SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	CmdStaRecUpdate.ucBssInfoIdx = StaRecBaCfg.BssIdx;
	WCID_SET_H_L(CmdStaRecUpdate.ucWlanIdxHnVer, CmdStaRecUpdate.ucWlanIdxL, StaRecBaCfg.WlanIdx);
	CmdStaRecUpdate.ucMuarIdx = StaRecBaCfg.MuarIdx;
	CmdStaRecUpdate.u2TotalElementNum = cpu2le16(1);
	CmdStaRecUpdate.ucAppendCmdTLV = TRUE;
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecUpdate, u4ComCmdSize);

	/* Step 4: Filled in parameters of CMD_STAREC_BA*/
	CmdStaRecBa.u2Tag = UNI_CMD_STAREC_BA;
	CmdStaRecBa.u2Length = (u4CmdNeedMaxBufSize - u4ComCmdSize);
	CmdStaRecBa.ucTid = StaRecBaCfg.tid;
	CmdStaRecBa.ucBaDirection = StaRecBaCfg.baDirection;
	CmdStaRecBa.ucAmsduCap = StaRecBaCfg.amsdu;
	CmdStaRecBa.ucBaEenable = StaRecBaCfg.BaEnable;
	CmdStaRecBa.u2BaStartSeq = StaRecBaCfg.sn;
	CmdStaRecBa.u2BaWinSize = StaRecBaCfg.ba_wsize;
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: BaInfo:\n", __func__);
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s: u2Tag=%d, u2Length=%d, ucTid=%d, u2BaDirectin=%d, BaEnable=%d, u2BaStartSeq=%d, u2BaWinSize=%d, ucAmsduCap=%d\n",
			  __func__,	CmdStaRecBa.u2Tag, CmdStaRecBa.u2Length, CmdStaRecBa.ucTid, CmdStaRecBa.ucBaDirection,
			  CmdStaRecBa.ucBaEenable, CmdStaRecBa.u2BaStartSeq, CmdStaRecBa.u2BaWinSize, CmdStaRecBa.ucAmsduCap);
#ifdef RT_BIG_ENDIAN
	CmdStaRecBa.u2Tag = cpu2le16(CmdStaRecBa.u2Tag);
	CmdStaRecBa.u2Length = cpu2le16(CmdStaRecBa.u2Length);
	CmdStaRecBa.u2BaStartSeq = cpu2le16(CmdStaRecBa.u2BaStartSeq);
	CmdStaRecBa.u2BaWinSize = cpu2le16(CmdStaRecBa.u2BaWinSize);
#endif
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecBa, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Send out CMD */
	call_fw_cmd_notifieriers(WO_CMD_STA_REC, pAd, msg->net_pkt);
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

#ifdef HTC_DECRYPT_IOT
/* Unified command for UNI_CMD_STAREC_AAD_OM 0x10 */
INT32 UniCmdStaRecAADOmUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16 Wcid,
	UINT8 AadOm)
{
	struct cmd_msg          *msg = NULL;
	INT32                   Ret = NDIS_STATUS_SUCCESS;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4ComCmdSize = 0;
	UNI_CMD_STAREC_T  	 	CmdStaRecUpdate;
	CMD_STAREC_AADOM_T		CmdStaRecAadom;
	struct _CMD_ATTRIBUTE 	attr = {0};

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(CmdStaRecUpdate);
	os_zero_mem(&CmdStaRecUpdate, u4ComCmdSize);
	os_zero_mem(&CmdStaRecAadom, sizeof(CmdStaRecAadom));

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(CmdStaRecAadom);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_STAREC_INFO);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
	SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill common parameters here */
	CmdStaRecUpdate.ucBssInfoIdx = 0; /* useless for this cmd */
	WCID_SET_H_L(CmdStaRecUpdate.ucWlanIdxHnVer, CmdStaRecUpdate.ucWlanIdxL, Wcid);
	CmdStaRecUpdate.ucMuarIdx = 0; /* useless for this cmd */
	CmdStaRecUpdate.u2TotalElementNum = cpu2le16(1);
	CmdStaRecUpdate.ucAppendCmdTLV = TRUE;
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecUpdate, u4ComCmdSize);

	/* Step 4: Filled in parameters of CMD_STAREC_BA*/
	CmdStaRecAadom.u2Tag = UNI_CMD_STAREC_AAD_OM;
	CmdStaRecAadom.u2Length = (u4CmdNeedMaxBufSize - u4ComCmdSize);
	CmdStaRecAadom.ucAadOm = AadOm;
#ifdef RT_BIG_ENDIAN
	CmdStaRecAadom.u2Tag = cpu2le16(CmdStaRecAadom.u2Tag);
	CmdStaRecAadom.u2Length = cpu2le16(CmdStaRecAadom.u2Length);
#endif
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecAadom, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step5: send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}
#endif /* HTC_DECRYPT_IOT */


/* Unified command for UNI_CMD_STAREC_PSM 0x2a */
INT32 UniCmdStaRecPsmUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16 Wcid,
	UINT8 Psm)
{
	struct cmd_msg			*msg = NULL;
	INT32					Ret = NDIS_STATUS_SUCCESS;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4ComCmdSize = 0;
	UNI_CMD_STAREC_T		CmdStaRecUpdate;
	CMD_STAREC_PSM_T		CmdStaRecPsm;
	struct _CMD_ATTRIBUTE	attr = {0};

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(CmdStaRecUpdate);
	os_zero_mem(&CmdStaRecUpdate, u4ComCmdSize);
	os_zero_mem(&CmdStaRecPsm, sizeof(CmdStaRecPsm));

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(CmdStaRecPsm);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_STAREC_INFO);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
	SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill common parameters here */
	CmdStaRecUpdate.ucBssInfoIdx = 0; /* useless for this cmd */
	WCID_SET_H_L(CmdStaRecUpdate.ucWlanIdxHnVer, CmdStaRecUpdate.ucWlanIdxL, Wcid);
	CmdStaRecUpdate.ucMuarIdx = 0; /* useless for this cmd */
	CmdStaRecUpdate.u2TotalElementNum = cpu2le16(1);
	CmdStaRecUpdate.ucAppendCmdTLV = TRUE;
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecUpdate, u4ComCmdSize);

	/* Step 4: Filled in parameters of CMD_STAREC_BA*/
	CmdStaRecPsm.u2Tag = UNI_CMD_STAREC_PSM;
	CmdStaRecPsm.u2Length = (u4CmdNeedMaxBufSize - u4ComCmdSize);
	CmdStaRecPsm.ucPsmMode = Psm;
#ifdef RT_BIG_ENDIAN
	CmdStaRecPsm.u2Tag = cpu2le16(CmdStaRecPsm.u2Tag);
	CmdStaRecPsm.u2Length = cpu2le16(CmdStaRecPsm.u2Length);
#endif
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecPsm, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step5: send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"(Ret = %d)\n", Ret);
	return Ret;
}

INT32 UniCmdStaRecSNUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16 Wcid,
	UINT16 Sn)
{
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
	"UniCmd support to be added for SN Update \n");

	return TRUE;
}

/*********************************************
 * Command ID: UNI_CMD_ID_STAREC_INFO = 0x03
 * Tag: UNI_CMD_STAREC_WTBL (Tag 0x0d)
 * Update wtbl by STAREC handler
 *
 *
 * Command Format:
 * ---------------------
 * | UNI_CMD_STAREC_T  |
 * ---------------------
 * | CMD_STAREC_WTBL_T |
 * ---------------------
 * | CMD_WTBL_UPDATE_T |
 * ---------------------
 * | TLVs			   |
 * ---------------------
 *********************************************/
INT32 UniCmdWtblUpdate(RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT8 ucOperation,
					   VOID *pBuffer, UINT32 u4BufferLen)
{
	struct cmd_msg          *msg = NULL;
	INT32                   Ret = NDIS_STATUS_SUCCESS;
	UINT16                  u2TLVNumber = 0;
	PUCHAR					pTempBuf = NULL;
	PUCHAR					pNextHeadBuf = NULL;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4RealUseBufSize = 0;
	UINT32					u4SendBufSize = 0;
	UINT32					u4RemainingPayloadSize = 0;
	UINT32					u4ComCmdSize = 0;
	P_UNI_CMD_STAREC_T   	pCmdStaRecUpdate = NULL;
	P_UNI_CMD_CMD_STAREC_WTBL_T   	pCmdStaRecWtbl = NULL;
	P_CMD_WTBL_UPDATE_T		pCmdWtblUpdate = NULL;
	RTMP_CHIP_CAP			*cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32					u4EnableFeature = 0;
	UINT32					u4RemainingTLVBufLen = 0;
	PUCHAR					pTlvTempBuffer = (PUCHAR)pBuffer;
	P_CMD_WTBL_GENERIC_TLV_T	pWtblGenericTlv = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev *wdev = NULL;

	if (!IS_WCID_VALID(pAd, u2WlanIdx)) {
		return NDIS_STATUS_INVALID_DATA;
	} else {
		if (VALID_UCAST_ENTRY_WCID(pAd, u2WlanIdx)) {
			pMacEntry = &pAd->MacTab.Content[u2WlanIdx];
		} else {
			pMacEntry = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE];
		}

		if (!IS_VALID_ENTRY(pMacEntry))
			return NDIS_STATUS_FAILURE;

		wdev = pMacEntry->wdev;
		if (wdev == NULL)
			return NDIS_STATUS_FAILURE;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_STAREC_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	u4CmdNeedMaxBufSize += sizeof(UNI_CMD_CMD_STAREC_WTBL_T);
	u4CmdNeedMaxBufSize += sizeof(CMD_WTBL_UPDATE_T);
	u4CmdNeedMaxBufSize += u4BufferLen;

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here (UNI_CMD_STAREC_T) */
	pCmdStaRecUpdate = (P_UNI_CMD_STAREC_T)pNextHeadBuf;
	pCmdStaRecUpdate->ucBssInfoIdx = wdev->bss_info_argument.ucBandIdx;
	WCID_SET_H_L(pCmdStaRecUpdate->ucWlanIdxHnVer, pCmdStaRecUpdate->ucWlanIdxL, u2WlanIdx);
	pCmdStaRecUpdate->ucMuarIdx = wdev->OmacIdx;
	pCmdStaRecUpdate->u2TotalElementNum = cpu2le16(1);
	pCmdStaRecUpdate->ucAppendCmdTLV = TRUE;
	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Fill STAREC_WTBL TLV parameters here (CMD_STAREC_WTBL_T)*/
	pCmdStaRecWtbl = (P_UNI_CMD_CMD_STAREC_WTBL_T)pNextHeadBuf;
	pCmdStaRecWtbl->u2Tag = UNI_CMD_STAREC_WTBL; /* TODO: Modify if operation is Query */
	pCmdStaRecWtbl->u2Length = sizeof(UNI_CMD_CMD_STAREC_WTBL_T) + sizeof(CMD_WTBL_UPDATE_T)  + u4BufferLen;
	pNextHeadBuf += sizeof(CMD_STAREC_WTBL_T);

	/* Step 5: Fill WTBL Update Common part (CMD_WTBL_UPDATE_T)*/
	/* Get TVL number from TLV buffer*/
	u4RemainingTLVBufLen = u4BufferLen;

	while (u4RemainingTLVBufLen > 0) {
		pWtblGenericTlv = (P_CMD_WTBL_GENERIC_TLV_T)pTlvTempBuffer;

		if (pWtblGenericTlv == NULL) {
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "pWtblGenericTlv is NULL\n");
			Ret = NDIS_STATUS_INVALID_DATA;
			break;
		} else if (pWtblGenericTlv->u2Length == 0) {
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "fail to handle T(%d), L(%d)\n", pWtblGenericTlv->u2Tag, pWtblGenericTlv->u2Length);
			Ret = NDIS_STATUS_INVALID_DATA;
			break;
		}

		u4EnableFeature |= (1 << (pWtblGenericTlv->u2Tag));
		pTlvTempBuffer += pWtblGenericTlv->u2Length;
		u4RemainingTLVBufLen -= pWtblGenericTlv->u2Length;
		u2TLVNumber++;
	}

	pCmdWtblUpdate = (P_CMD_WTBL_UPDATE_T)pNextHeadBuf;
	WCID_SET_H_L(pCmdWtblUpdate->ucWlanIdxHnVer, pCmdWtblUpdate->ucWlanIdxL, u2WlanIdx);
	pCmdWtblUpdate->ucOperation = RESET_WTBL_AND_SET; /* TODO: Modify if operation is Query */
	pCmdWtblUpdate->u2TotalElementNum = cpu2le16(u2TLVNumber);
	pNextHeadBuf += sizeof(CMD_WTBL_UPDATE_T);

	/* Step 6: Fill WTBL TLV part (TLVs)*/
	memcpy(pNextHeadBuf, pBuffer, u4BufferLen);
	pNextHeadBuf += u4BufferLen;

	/* Step 7: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 8: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_STAREC_INFO);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if ((!bNeedFrag) || bLastFrag) {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
				SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
			} else {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			call_fw_cmd_notifieriers(WO_CMD_STA_REC, pAd, msg->net_pkt);
			Ret = AndesSendCmdMsg(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}
/*****************************************
 * Command ID: UNI_CMD_ID_EDCA_SET = 0x04
*****************************************/
static INT32 UniCmdEDCAAcSet(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	MT_EDCA_CTRL_T EdcaParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	UINT32 i = 0;
	P_TX_AC_PARAM_T pAcParam = NULL;
	P_UNI_CMD_EDCA_AC_PARM_T pEDCAAc = (P_UNI_CMD_EDCA_AC_PARM_T)pHandle;

	for (i = 0; i < EdcaParam.ucTotalNum; i++) {
		pAcParam = &EdcaParam.rAcParam[i];
		/* Fill TLV format */
		pEDCAAc->u2Tag = UNI_CMD_EDCA_AC_PARM;
		pEDCAAc->u2Length = sizeof(UNI_CMD_EDCA_AC_PARM_T);
#ifdef RT_BIG_ENDIAN
		pEDCAAc->u2Tag = cpu2le16(pEDCAAc->u2Tag);
		pEDCAAc->u2Length = cpu2le16(pEDCAAc->u2Length);
#endif /* RT_BIG_ENDIAN */
		pEDCAAc->ucAcIndex = pAcParam->ucAcNum;

		if (pAcParam->ucVaildBit & CMD_EDCA_AIFS_BIT) {
			pEDCAAc->ucAifsn = pAcParam->ucAifs;
			pEDCAAc->ucValidBitmap |= MASK_AIFS_SET;
		}

		if (pAcParam->ucVaildBit & CMD_EDCA_WIN_MIN_BIT) {
			pEDCAAc->ucCWmin = pAcParam->ucWinMin;
			pEDCAAc->ucValidBitmap |= MASK_WINMIN_SET;
		}

		if (pAcParam->ucVaildBit & CMD_EDCA_WIN_MAX_BIT) {
			pEDCAAc->ucCWmax = (UINT8)pAcParam->u2WinMax;
			pEDCAAc->ucValidBitmap |= MASK_WINMAX_SET;
		}

		if (pAcParam->ucVaildBit & CMD_EDCA_TXOP_BIT) {
			pEDCAAc->u2TxopLimit = cpu2le16(pAcParam->u2Txop);
			pEDCAAc->ucValidBitmap |= MASK_TXOP_SET;
		}

		pEDCAAc++;
	}

	return Ret;
}

static UNI_CMD_TAG_HANDLE_T UniCmdEDCATab[UNI_CMD_EDCA_MAX_NUM] = {
	{
		.u2CmdFeature = UNI_CMD_EDCA_AC_PARM,
		.u4StructSize = sizeof(UNI_CMD_EDCA_AC_PARM_T),
		.pfHandler = UniCmdEDCAAcSet
	},
};

INT32 MtUniCmdEdcaParameterSet(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	MT_EDCA_CTRL_T EdcaParam)
{
	struct cmd_msg          *msg = NULL;
	INT32                   Ret = NDIS_STATUS_SUCCESS;
	UINT32                  i = 0;
	UINT16                  u2TLVNumber = 0;
	PUCHAR					pTempBuf = NULL;
	PUCHAR					pNextHeadBuf = NULL;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4RealUseBufSize = 0;
	UINT32					u4SendBufSize = 0;
	UINT32					u4RemainingPayloadSize = 0;
	UINT32					u4ComCmdSize = 0;
	P_UNI_CMD_EDCA_T		pCmdEDCAUpdate = NULL;
	RTMP_CHIP_CAP			*cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_EDCA_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < UNI_CMD_EDCA_MAX_NUM; i++) {
		if (i == UNI_CMD_EDCA_AC_PARM)
			u4CmdNeedMaxBufSize += (UniCmdEDCATab[i].u4StructSize * EdcaParam.ucTotalNum);
		else
			u4CmdNeedMaxBufSize += UniCmdEDCATab[i].u4StructSize;
	}

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdEDCAUpdate = (P_UNI_CMD_EDCA_T)pNextHeadBuf;
	pCmdEDCAUpdate->ucBssInfoIdx = wdev->bss_info_argument.ucBssIndex;
	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Traverse all support features */
	for (i = 0; i < UNI_CMD_EDCA_MAX_NUM; i++) {
		switch (i) {
		case UNI_CMD_EDCA_AC_PARM:
			if (UniCmdEDCATab[i].pfHandler != NULL) {
				Ret = ((PFN_EDCA_AC_HANDLE)(UniCmdEDCATab[i].pfHandler))(pAd, wdev, EdcaParam, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += (UniCmdEDCATab[i].u4StructSize * EdcaParam.ucTotalNum);
					u2TLVNumber++;
				}
			}
			break;

		default:
			Ret = NDIS_STATUS_SUCCESS;
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"%s(): The hanlder of tag (0x%08x) not support!\n", __func__, UniCmdEDCATab[i].u2CmdFeature);
			break;
		}

		if (Ret != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"The hanlder of tag (0x%08x) return fail!\n", UniCmdEDCATab[i].u2CmdFeature);
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_EDCA_SET);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if (!bNeedFrag || bLastFrag) {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
				SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
			} else {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			Ret = chip_cmd_tx(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static INT32 UniCmdBandCfgRadioOnOff(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T pBandCfgRadioOnOff = (P_UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T)pHandle;

	pBandCfgRadioOnOff->u2Tag = UNI_CMD_BAND_CONFIG_RADIO_ONOFF;
	pBandCfgRadioOnOff->u2Length = sizeof(UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T);
#ifdef RT_BIG_ENDIAN
	pBandCfgRadioOnOff->u2Tag = cpu2le16(pBandCfgRadioOnOff->u2Tag);
	pBandCfgRadioOnOff->u2Length = cpu2le16(pBandCfgRadioOnOff->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBandCfgRadioOnOff->fgRadioOn = pParam->fgRadioOn;

	return Ret;
}

static INT32 UniCmdBandCfgRXVCtrl(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_BAND_CONFIG_RXV_CTRL_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BAND_CONFIG_RXV_CTRL_T pBandCfgRxvCtrl = (P_UNI_CMD_BAND_CONFIG_RXV_CTRL_T)pHandle;

	pBandCfgRxvCtrl->u2Tag = UNI_CMD_BAND_CONFIG_RXV_CTRL;
	pBandCfgRxvCtrl->u2Length = sizeof(UNI_CMD_BAND_CONFIG_RXV_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pBandCfgRxvCtrl->u2Tag = cpu2le16(pBandCfgRxvCtrl->u2Tag);
	pBandCfgRxvCtrl->u2Length = cpu2le16(pBandCfgRxvCtrl->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBandCfgRxvCtrl->ucRxvOfRxEnable = pParam->ucRxvOfRxEnable;
	pBandCfgRxvCtrl->ucRxvOfTxEnable = pParam->ucRxvOfTxEnable;

	return Ret;
}

static INT32 UniCmdBandCfgSetRxFilter(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T pBandCfgSetRxFilter = (P_UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T)pHandle;

	pBandCfgSetRxFilter->u2Tag = UNI_CMD_BAND_CONFIG_SET_RX_FILTER;
	pBandCfgSetRxFilter->u2Length = sizeof(UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T);
#ifdef RT_BIG_ENDIAN
	pBandCfgSetRxFilter->u2Tag = cpu2le16(pBandCfgSetRxFilter->u2Tag);
	pBandCfgSetRxFilter->u2Length = cpu2le16(pBandCfgSetRxFilter->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBandCfgSetRxFilter->u4RxPacketFilter = cpu2le32(pParam->u4RxPacketFilter);

	return Ret;
}

static INT32 UniCmdBandCfgDropCtrlFrame(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T pBandCfgDropCtrlFrame = (P_UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T)pHandle;

	pBandCfgDropCtrlFrame->u2Tag = UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME;
	pBandCfgDropCtrlFrame->u2Length = sizeof(UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T);
#ifdef RT_BIG_ENDIAN
	pBandCfgDropCtrlFrame->u2Tag = cpu2le16(pBandCfgDropCtrlFrame->u2Tag);
	pBandCfgDropCtrlFrame->u2Length = cpu2le16(pBandCfgDropCtrlFrame->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBandCfgDropCtrlFrame->ucDropRts = pParam->ucDropRts;
	pBandCfgDropCtrlFrame->ucDropCts = pParam->ucDropCts;
	pBandCfgDropCtrlFrame->ucDropUnwantedCtrl = pParam->ucDropUnwantedCtrl;

	return Ret;
}

static INT32 UniCmdBandCfgAGGACLimit(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T pBandCfgAGGAcLimit = (P_UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T)pHandle;

	pBandCfgAGGAcLimit->u2Tag = UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT;
	pBandCfgAGGAcLimit->u2Length = sizeof(UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T);
#ifdef RT_BIG_ENDIAN
	pBandCfgAGGAcLimit->u2Tag = cpu2le16(pBandCfgAGGAcLimit->u2Tag);
	pBandCfgAGGAcLimit->u2Length = cpu2le16(pBandCfgAGGAcLimit->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBandCfgAGGAcLimit->ucWmmIdx = pParam->ucWmmIdx;
	pBandCfgAGGAcLimit->ucAc = pParam->ucAc;
	pBandCfgAGGAcLimit->ucAggLimit = pParam->ucAggLimit;

	return Ret;
}

static INT32 UniCmdBandCfgEDCCAEnable(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T pBandCfgEDCCAEnable = (P_UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T)pHandle;

	pBandCfgEDCCAEnable->u2Tag = UNI_CMD_BAND_CONFIG_EDCCA_ENABLE;
	pBandCfgEDCCAEnable->u2Length = sizeof(UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pBandCfgEDCCAEnable->u2Tag = cpu2le16(pBandCfgEDCCAEnable->u2Tag);
	pBandCfgEDCCAEnable->u2Length = cpu2le16(pBandCfgEDCCAEnable->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBandCfgEDCCAEnable->fgEDCCAEnable = pParam->fgEDCCAEnable;

	return Ret;
}

static INT32 UniCmdBandCfgEDCCAThreshold(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T pBandCfgEDCCAThreshold = (P_UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T)pHandle;
	UINT32 i;

	pBandCfgEDCCAThreshold->u2Tag = UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD;
	pBandCfgEDCCAThreshold->u2Length = sizeof(UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pBandCfgEDCCAThreshold->u2Tag = cpu2le16(pBandCfgEDCCAThreshold->u2Tag);
	pBandCfgEDCCAThreshold->u2Length = cpu2le16(pBandCfgEDCCAThreshold->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBandCfgEDCCAThreshold->fginit = pParam->fginit;
	for (i = 0; i < 3; i++)
		pBandCfgEDCCAThreshold->u1EDCCAThreshold[i] = pParam->u1EDCCAThreshold[i];

	return Ret;
}

static INT32 UniCmdConfigRTSSigEn(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN_T pBandCfgRTSSigEn = (P_UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN_T)pHandle;

	pBandCfgRTSSigEn->u2Tag = UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN;
	pBandCfgRTSSigEn->u2Length = sizeof(UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN_T);
#ifdef RT_BIG_ENDIAN
	pBandCfgRTSSigEn->u2Tag = cpu2le16(pBandCfgRTSSigEn->u2Tag);
	pBandCfgRTSSigEn->u2Length = cpu2le16(pBandCfgRTSSigEn->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBandCfgRTSSigEn->Enable = pParam->Enable;

	return Ret;
}

static INT32 UniCmdConfigSchDetDis(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T pBandCfgSchDetDis = (P_UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T)pHandle;

	pBandCfgSchDetDis->u2Tag = UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T;
	pBandCfgSchDetDis->u2Length = sizeof(UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T);
#ifdef RT_BIG_ENDIAN
	pBandCfgSchDetDis->u2Tag = cpu2le16(pBandCfgSchDetDis->u2Tag);
	pBandCfgSchDetDis->u2Length = cpu2le16(pBandCfgSchDetDis->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBandCfgSchDetDis->Disable = pParam->Disable;

	return Ret;
}

static INT32 UniCmdConfigRTS0PktThreshold(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_BAND_CONFIG_RTS0_PKT_THRESHOLD_CFG_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_BAND_CONFIG_RTS0_PKT_THRESHOLD_CFG_T pBandCfgSchRTS0PktThreshold = (P_UNI_CMD_BAND_CONFIG_RTS0_PKT_THRESHOLD_CFG_T)pHandle;

	pBandCfgSchRTS0PktThreshold->u2Tag = UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T;
	pBandCfgSchRTS0PktThreshold->u2Length = sizeof(UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T);
#ifdef RT_BIG_ENDIAN
	pBandCfgSchRTS0PktThreshold->u2Tag = cpu2le16(pBandCfgSchRTS0PktThreshold->u2Tag);
	pBandCfgSchRTS0PktThreshold->u2Length = cpu2le16(pBandCfgSchRTS0PktThreshold->u2Length);
#endif /* RT_BIG_ENDIAN */
	pBandCfgSchRTS0PktThreshold->Enable = pParam->Enable;
	pBandCfgSchRTS0PktThreshold->u4Value = pParam->u4Value;
	pBandCfgSchRTS0PktThreshold->ucType = pParam->ucType;

	return Ret;
}


static UNI_CMD_TAG_HANDLE_T UniCmdBandCfgTab[UNI_CMD_BAND_CONFIG_MAX_NUM] = {
	{
		.u2CmdFeature = UNI_CMD_BAND_CONFIG_RADIO_ONOFF,
		.u4StructSize = sizeof(UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T),
		.pfHandler = UniCmdBandCfgRadioOnOff
	},
	{
		.u2CmdFeature = UNI_CMD_BAND_CONFIG_RXV_CTRL,
		.u4StructSize = sizeof(UNI_CMD_BAND_CONFIG_RXV_CTRL_T),
		.pfHandler = UniCmdBandCfgRXVCtrl
	},
	{
		.u2CmdFeature = UNI_CMD_BAND_CONFIG_SET_RX_FILTER,
		.u4StructSize = sizeof(UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T),
		.pfHandler = UniCmdBandCfgSetRxFilter
	},
	{
		.u2CmdFeature = UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME,
		.u4StructSize = sizeof(UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T),
		.pfHandler = UniCmdBandCfgDropCtrlFrame
	},
	{
		.u2CmdFeature = UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT,
		.u4StructSize = sizeof(UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T),
		.pfHandler = UniCmdBandCfgAGGACLimit
	},
	{
		.u2CmdFeature = UNI_CMD_BAND_CONFIG_EDCCA_ENABLE,
		.u4StructSize = sizeof(UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T),
		.pfHandler = UniCmdBandCfgEDCCAEnable
	},
	{
		.u2CmdFeature = UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD,
		.u4StructSize = sizeof(UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T),
		.pfHandler = UniCmdBandCfgEDCCAThreshold
	},
	{
		.u2CmdFeature = UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN,
		.u4StructSize = sizeof(UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN_T),
		.pfHandler = UniCmdConfigRTSSigEn
	},
	{
		.u2CmdFeature = UNI_CMD_BAND_CONFIG_SCH_DET_DIS,
		.u4StructSize = sizeof(UNI_CMD_BAND_CONFIG_SCH_DET_DIS_T),
		.pfHandler = UniCmdConfigSchDetDis
	},
	{
		.u2CmdFeature = UNI_CMD_BAND_CONFIG_SCH_DET_DIS,
		.u4StructSize = sizeof(UNI_CMD_BAND_CONFIG_RTS0_PKT_THRESHOLD_CFG_T),
		.pfHandler = UniCmdConfigRTS0PktThreshold
	},
};

INT32 UniCmdBandConfig(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_BAND_CFG_PARAM_T pParamCtrl)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = NDIS_STATUS_SUCCESS;
	UINT32 i = 0;
	UINT16 u2TLVNumber = 0;
	PUCHAR pTempBuf = NULL;
	PUCHAR pNextHeadBuf = NULL;
	UINT32 u4CmdNeedMaxBufSize = 0;
	UINT32 u4RealUseBufSize = 0;
	UINT32 u4SendBufSize = 0;
	UINT32 u4RemainingPayloadSize = 0;
	UINT32 u4ComCmdSize = 0;
	P_UNI_CMD_BAND_CONFIG_T	pCmdBandCfg = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_BAND_CONFIG_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < UNI_CMD_BAND_CONFIG_MAX_NUM; i++) {
		if (pParamCtrl->BandCfgTagValid[i])
			u4CmdNeedMaxBufSize += UniCmdBandCfgTab[i].u4StructSize;
	}

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdBandCfg = (P_UNI_CMD_BAND_CONFIG_T)pNextHeadBuf;
	pCmdBandCfg->ucDbdcIdx = pParamCtrl->ucDbdcIdx;

	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Traverse all support features */
	for (i = 0; i < UNI_CMD_BAND_CONFIG_MAX_NUM; i++) {
		if (pParamCtrl->BandCfgTagValid[i]) {
			switch (i) {
			case UNI_CMD_BAND_CONFIG_RADIO_ONOFF:
				if (UniCmdBandCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_BAND_CFG_RADIO_ONOFF_HANDLE)(UniCmdBandCfgTab[i].pfHandler))(pAd, &pParamCtrl->BandCfgRadioOnOff, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdBandCfgTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_BAND_CONFIG_RXV_CTRL:
				if (UniCmdBandCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_BAND_CFG_RXV_CTRL_HANDLE)(UniCmdBandCfgTab[i].pfHandler))(pAd, &pParamCtrl->BandCfgRXVCtrl, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdBandCfgTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_BAND_CONFIG_SET_RX_FILTER:
				if (UniCmdBandCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_BAND_CFG_SET_RX_FILTER_HANDLE)(UniCmdBandCfgTab[i].pfHandler))(pAd, &pParamCtrl->BandCfgSetRxFilter, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdBandCfgTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME:
				if (UniCmdBandCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_BAND_CFG_DROP_CTRL_FRAME_HANDLE)(UniCmdBandCfgTab[i].pfHandler))(pAd, &pParamCtrl->BandCfgDropCtrlFrame, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdBandCfgTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT:
				if (UniCmdBandCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_BAND_CFG_AGG_AC_LIMIT_HANDLE)(UniCmdBandCfgTab[i].pfHandler))(pAd, &pParamCtrl->BandCfgAGGAcLimit, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdBandCfgTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_BAND_CONFIG_EDCCA_ENABLE:
				if (UniCmdBandCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_BAND_CFG_EDCCA_ENABLE_CTRL_HANDLE)(UniCmdBandCfgTab[i].pfHandler))(pAd, &pParamCtrl->BandCfgEDCCAEnable, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdBandCfgTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD:
				if (UniCmdBandCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_BAND_CFG_EDCCA_THRESHOLD_CTRL_HANDLE)(UniCmdBandCfgTab[i].pfHandler))(pAd, &pParamCtrl->BandCfgEDCCAThreshold, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdBandCfgTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_BAND_CONFIG_RTS_SIGTA_EN:
				if (UniCmdBandCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_BAND_CFG_RTS_SIGTA_EN_HANDLE)(UniCmdBandCfgTab[i].pfHandler))(pAd, &pParamCtrl->BandCfgRtsSigtaen, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdBandCfgTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_BAND_CONFIG_SCH_DET_DIS:
				if (UniCmdBandCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_BAND_CFG_SCH_DET_DIS_HANDLE)(UniCmdBandCfgTab[i].pfHandler))(pAd, &pParamCtrl->BandCfgSCHDetDis, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdBandCfgTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_BAND_CONFIG_RTS0_PKT_THRESHOLD_CFG:
				if (UniCmdBandCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_BAND_CFG_RTS0_PKT_THRESHOLD_HANDLE)(UniCmdBandCfgTab[i].pfHandler))(pAd, &pParamCtrl->BandCfgRTS0PktThreshold, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdBandCfgTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			default:
				Ret = NDIS_STATUS_SUCCESS;
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s(): The hanlder of tag (0x%08x) not support!\n", __func__, UniCmdBandCfgTab[i].u2CmdFeature);
				break;
			}

			if (Ret != NDIS_STATUS_SUCCESS)
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s(): The hanlder of tag (0x%08x) return fail!\n", __func__, UniCmdBandCfgTab[i].u2CmdFeature);
		}
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			 u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_BAND_CONFIG);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if (!bNeedFrag || bLastFrag) {
				if (pParamCtrl->bQuery) {
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_WAIT_RETRY_RSP);
					SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
					SET_CMD_ATTR_RSP_HANDLER(attr, UniEventEDCCAHandler);
				} else {
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
					SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
					SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
				}
			} else {
				if (pParamCtrl->bQuery)
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_RETRY);
				else
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			Ret = chip_cmd_tx(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

/*********************************************
 * UNI_CMD_BAND_CONFIG_RADIO_ONOFF (Tag 0x00)
 *********************************************/
INT32 UniCmdRadioOnOff(struct _RTMP_ADAPTER *pAd, MT_PMSTAT_CTRL_T PmStatCtrl)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	UNI_CMD_BAND_CFG_PARAM_T BandCfgParam;

	os_zero_mem(&BandCfgParam, sizeof(BandCfgParam));
	BandCfgParam.ucDbdcIdx = PmStatCtrl.DbdcIdx;
	if (PmStatCtrl.PmState == EXIT_PM_STATE)
		BandCfgParam.BandCfgRadioOnOff.fgRadioOn = TRUE;
	else
		BandCfgParam.BandCfgRadioOnOff.fgRadioOn = FALSE;
	BandCfgParam.BandCfgTagValid[UNI_CMD_BAND_CONFIG_RADIO_ONOFF] = TRUE;

	ret = UniCmdBandConfig(pAd, &BandCfgParam);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, ret);

	return ret;
}

/*********************************************
 * UNI_CMD_BAND_CONFIG_RXV_CTRL (Tag 0x01)
 *********************************************/
INT32 UniCmdRxvCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 TxRx, UINT8 ucEnable)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	UNI_CMD_BAND_CFG_PARAM_T BandCfgParam;

	if ((TxRx < ASIC_MAC_TXRX_RXV) || (TxRx > ASIC_MAC_RX_RXV))
		return NDIS_STATUS_INVALID_DATA;

	os_zero_mem(&BandCfgParam, sizeof(BandCfgParam));
	BandCfgParam.ucDbdcIdx = ucDbdcIdx;
	if (TxRx == ASIC_MAC_RXV) {
		BandCfgParam.BandCfgRXVCtrl.ucRxvOfTxEnable = ucEnable;
		BandCfgParam.BandCfgRXVCtrl.ucRxvOfRxEnable = pAd->RxvOfRxEnable[ucDbdcIdx];
		pAd->RxvOfTxEnable[ucDbdcIdx] = ucEnable;
	} else if (TxRx == ASIC_MAC_RX_RXV) {
		BandCfgParam.BandCfgRXVCtrl.ucRxvOfRxEnable = ucEnable;
		BandCfgParam.BandCfgRXVCtrl.ucRxvOfTxEnable = pAd->RxvOfTxEnable[ucDbdcIdx];
		pAd->RxvOfRxEnable[ucDbdcIdx] = ucEnable;
	} else if (TxRx == ASIC_MAC_TXRX_RXV) {
		BandCfgParam.BandCfgRXVCtrl.ucRxvOfRxEnable = ucEnable;
		BandCfgParam.BandCfgRXVCtrl.ucRxvOfTxEnable = ucEnable;
		pAd->RxvOfTxEnable[ucDbdcIdx] = ucEnable;
		pAd->RxvOfRxEnable[ucDbdcIdx] = ucEnable;
	}

	BandCfgParam.BandCfgTagValid[UNI_CMD_BAND_CONFIG_RXV_CTRL] = TRUE;

	ret = UniCmdBandConfig(pAd, &BandCfgParam);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, ret);

	return ret;
}

INT32 UniCmdSetEDCCAThreshold(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1edcca_threshold[],
	UINT8 u1BandIdx,
	BOOLEAN bInit
)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 status = TRUE;
	UNI_CMD_BAND_CFG_PARAM_T BandCfgParam;
	UINT32 i;

	os_zero_mem(&BandCfgParam, sizeof(BandCfgParam));

	BandCfgParam.ucDbdcIdx = u1BandIdx;
	BandCfgParam.BandCfgEDCCAThreshold.fginit = bInit;
	for (i = 0; i < 3; i++)
		BandCfgParam.BandCfgEDCCAThreshold.u1EDCCAThreshold[i] = u1edcca_threshold[i];
	BandCfgParam.BandCfgTagValid[UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD] = TRUE;

	ret = UniCmdBandConfig(pAd, &BandCfgParam);
	if (ret != NDIS_STATUS_SUCCESS)
		status = FALSE;

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, ret);

	return status;
}

INT32 UniCmdGetEDCCAThreshold(struct _RTMP_ADAPTER *pAd, UINT8 u1BandIdx, BOOLEAN bInit)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT status = TRUE;
	UNI_CMD_BAND_CFG_PARAM_T BandCfgParam;

	os_zero_mem(&BandCfgParam, sizeof(BandCfgParam));

	BandCfgParam.ucDbdcIdx = u1BandIdx;
	BandCfgParam.bQuery = TRUE;
	BandCfgParam.BandCfgEDCCAThreshold.fginit = bInit;
	BandCfgParam.BandCfgTagValid[UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD] = TRUE;

	ret = UniCmdBandConfig(pAd, &BandCfgParam);
	if (ret != NDIS_STATUS_SUCCESS)
		status = FALSE;

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, ret);

	return status;
}

INT32 UniCmdSetEDCCAEnable(struct _RTMP_ADAPTER *pAd, UINT8 u1EDCCACtrl, UINT8 u1BandIdx)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 status = TRUE;
	UNI_CMD_BAND_CFG_PARAM_T BandCfgParam;

	os_zero_mem(&BandCfgParam, sizeof(BandCfgParam));

	BandCfgParam.ucDbdcIdx = u1BandIdx;
	BandCfgParam.BandCfgEDCCAEnable.fgEDCCAEnable = u1EDCCACtrl;
	BandCfgParam.BandCfgTagValid[UNI_CMD_BAND_CONFIG_EDCCA_ENABLE] = TRUE;

	ret = UniCmdBandConfig(pAd, &BandCfgParam);
	if (ret != NDIS_STATUS_SUCCESS)
		status = FALSE;

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, ret);

	return status;
}

INT32 UniCmdGetEDCCAEnable(struct _RTMP_ADAPTER *pAd, UINT8 u1BandIdx)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	INT32 status = TRUE;
	UNI_CMD_BAND_CFG_PARAM_T BandCfgParam;

	os_zero_mem(&BandCfgParam, sizeof(BandCfgParam));

	BandCfgParam.ucDbdcIdx = u1BandIdx;
	BandCfgParam.bQuery = TRUE;
	BandCfgParam.BandCfgTagValid[UNI_CMD_BAND_CONFIG_EDCCA_ENABLE] = TRUE;

	ret = UniCmdBandConfig(pAd, &BandCfgParam);
	if (ret != NDIS_STATUS_SUCCESS)
		status = FALSE;

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, ret);

	return status;
}

static INT32 UniCmdFwLogCtrl(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_FW_LOG_CTRL_BASIC_T pParam, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_FW_LOG_CTRL_BASIC_T pFwLogCtrl = (P_UNI_CMD_FW_LOG_CTRL_BASIC_T)pHandle;

	pFwLogCtrl->u2Tag = UNI_CMD_WSYS_CONFIG_FW_LOG_CTRL;
	pFwLogCtrl->u2Length = sizeof(UNI_CMD_FW_LOG_CTRL_BASIC_T);
#ifdef RT_BIG_ENDIAN
	pFwLogCtrl->u2Tag = cpu2le16(pFwLogCtrl->u2Tag);
	pFwLogCtrl->u2Length = cpu2le16(pFwLogCtrl->u2Length);
#endif /* RT_BIG_ENDIAN */
	pFwLogCtrl->ucFwLog2HostCtrl = pParam->ucFwLog2HostCtrl;
	pFwLogCtrl->ucFwLog2HostInterval = pParam->ucFwLog2HostInterval;

	return Ret;
}

static INT32 UniCmdFwDbgCtrl(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_FW_DBG_CTRL_T pParam, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_FW_DBG_CTRL_T pFwDbgCtrl = (P_UNI_CMD_FW_DBG_CTRL_T)pHandle;

	pFwDbgCtrl->u2Tag = UNI_CMD_WSYS_CONFIG_FW_DBG_CTRL;
	pFwDbgCtrl->u2Length = sizeof(UNI_CMD_FW_DBG_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pFwDbgCtrl->u2Tag = cpu2le16(pFwDbgCtrl->u2Tag);
	pFwDbgCtrl->u2Length = cpu2le16(pFwDbgCtrl->u2Length);
#endif /* RT_BIG_ENDIAN */
	pFwDbgCtrl->u4DbgModuleIdx = cpu2le32(pParam->u4DbgModuleIdx);
	pFwDbgCtrl->ucDbgClass = pParam->ucDbgClass;

	return Ret;
}

static INT32 UniCmdFwLogUICtrl(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T pParam, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T pFwLogUICtrl = (P_UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T)pHandle;

	pFwLogUICtrl->u2Tag = UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL;
	pFwLogUICtrl->u2Length = sizeof(UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pFwLogUICtrl->u2Tag = cpu2le16(pFwLogUICtrl->u2Tag);
	pFwLogUICtrl->u2Length = cpu2le16(pFwLogUICtrl->u2Length);
#endif /* RT_BIG_ENDIAN */
	pFwLogUICtrl->ucVersion = cpu2le32(pParam->ucVersion);
	pFwLogUICtrl->ucLogLevel = cpu2le32(pParam->ucLogLevel);

	return Ret;
}

static INT32 UniCmdFwBasicConfig(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T pParam, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T pFwBasicConfig = (P_UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T)pHandle;

	pFwBasicConfig->u2Tag = UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG;
	pFwBasicConfig->u2Length = sizeof(UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T);
#ifdef RT_BIG_ENDIAN
	pFwBasicConfig->u2Tag = cpu2le16(pFwBasicConfig->u2Tag);
	pFwBasicConfig->u2Length = cpu2le16(pFwBasicConfig->u2Length);
#endif /* RT_BIG_ENDIAN */
	pFwBasicConfig->u2RxChecksum = cpu2le16(pParam->u2RxChecksum);
	pFwBasicConfig->u2TxChecksum = cpu2le16(pParam->u2TxChecksum);
	pFwBasicConfig->ucCtrlFlagAssertPath = pParam->ucCtrlFlagAssertPath;

	return Ret;
}

static INT32 UniCmdHostReportTxLatencyConfig(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T pParam, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T pHostReportTxLantencyCfg = (P_UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T)pHandle;

	pHostReportTxLantencyCfg->u2Tag = UNI_CMD_HOSTREPORT_TX_LATENCY_CONFIG;
	pHostReportTxLantencyCfg->u2Length = sizeof(UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T);
#ifdef RT_BIG_ENDIAN
	pHostReportTxLantencyCfg->u2Tag = cpu2le16(pHostReportTxLantencyCfg->u2Tag);
	pHostReportTxLantencyCfg->u2Length = cpu2le16(pHostReportTxLantencyCfg->u2Length);
#endif /* RT_BIG_ENDIAN */
	pHostReportTxLantencyCfg->ucActive = pParam->ucActive;

	return Ret;
}


static UNI_CMD_TAG_HANDLE_T UniCmdWsysCfgTab[UNI_CMD_WSYS_CONFIG_MAX_NUM] = {
	{
		.u2CmdFeature = UNI_CMD_WSYS_CONFIG_FW_LOG_CTRL,
		.u4StructSize = sizeof(UNI_CMD_FW_LOG_CTRL_BASIC_T),
		.pfHandler = UniCmdFwLogCtrl
	},
	{
		.u2CmdFeature = UNI_CMD_WSYS_CONFIG_FW_DBG_CTRL,
		.u4StructSize = sizeof(UNI_CMD_FW_DBG_CTRL_T),
		.pfHandler = UniCmdFwDbgCtrl
	},
	{
		.u2CmdFeature = UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL,
		.u4StructSize = sizeof(UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T),
		.pfHandler = UniCmdFwLogUICtrl
	},
	{
		.u2CmdFeature = UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG,
		.u4StructSize = sizeof(UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T),
		.pfHandler = UniCmdFwBasicConfig
	},
	{
		.u2CmdFeature = UNI_CMD_HOSTREPORT_TX_LATENCY_CONFIG,
		.u4StructSize = sizeof(UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T),
		.pfHandler = UniCmdHostReportTxLatencyConfig
	},
};

INT32 UniCmdWsysConfig(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_WSYS_CFG_PARAM_T pParamCtrl)
{
	struct cmd_msg          *msg = NULL;
	INT32                   Ret = NDIS_STATUS_SUCCESS;
	UINT32                  i = 0;
	UINT16                  u2TLVNumber = 0;
	PUCHAR					pTempBuf = NULL;
	PUCHAR					pNextHeadBuf = NULL;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4RealUseBufSize = 0;
	UINT32					u4SendBufSize = 0;
	UINT32					u4RemainingPayloadSize = 0;
	UINT32					u4ComCmdSize = 0;
	P_UNI_CMD_WSYS_CONFIG_T	pCmdWsysCfg = NULL;
	RTMP_CHIP_CAP			*cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_WSYS_CONFIG_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < UNI_CMD_WSYS_CONFIG_MAX_NUM; i++) {
		if (pParamCtrl->WsysCfgTagValid[i])
			u4CmdNeedMaxBufSize += UniCmdWsysCfgTab[i].u4StructSize;
	}

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdWsysCfg = (P_UNI_CMD_WSYS_CONFIG_T)pNextHeadBuf;
	/* Nothing to do */
	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Traverse all support features */
	for (i = 0; i < UNI_CMD_WSYS_CONFIG_MAX_NUM; i++) {
		if (pParamCtrl->WsysCfgTagValid[i]) {
			switch (i) {
			case UNI_CMD_WSYS_CONFIG_FW_LOG_CTRL:
				if (UniCmdWsysCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_WSYS_FW_LOG_CTRL_BASIC_HANDLE)(UniCmdWsysCfgTab[i].pfHandler))(pAd, &pParamCtrl->WsysFwLogCtrlBasic, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdWsysCfgTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_WSYS_CONFIG_FW_DBG_CTRL:
				if (UniCmdWsysCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_WSYS_FW_DBG_CTRL_HANDLE)(UniCmdWsysCfgTab[i].pfHandler))(pAd, &pParamCtrl->WsysFwDbgCtrl, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdWsysCfgTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL:
				if (UniCmdWsysCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_WSYS_FW_LOG_UI_CTRL_HANDLE)(UniCmdWsysCfgTab[i].pfHandler))(pAd, &pParamCtrl->WsysCfgFwLogUICtrl, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdWsysCfgTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG:
				if (UniCmdWsysCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_WSYS_FW_BASIC_CONFIG_HANDLE)(UniCmdWsysCfgTab[i].pfHandler))(pAd, &pParamCtrl->WsysCfgFwFwBasicConfig, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdWsysCfgTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_HOSTREPORT_TX_LATENCY_CONFIG:
				if (UniCmdWsysCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_HANDLE)(UniCmdWsysCfgTab[i].pfHandler))(pAd, &pParamCtrl->WsysCfgHostReportTxLatency, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdWsysCfgTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			default:
				Ret = NDIS_STATUS_SUCCESS;
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s(): The hanlder of tag (0x%08x) not support!\n", __func__, UniCmdWsysCfgTab[i].u2CmdFeature);
				break;
			}

			if (Ret != NDIS_STATUS_SUCCESS)
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"%s(): The hanlder of tag (0x%08x) return fail!\n", __func__, UniCmdWsysCfgTab[i].u2CmdFeature);
		}
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			" TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			if (pParamCtrl->WsysCfgTagValid[UNI_CMD_WSYS_CONFIG_FW_LOG_CTRL]) {
				if (pParamCtrl->McuDest == HOST2CR4)
					SET_CMD_ATTR_MCU_DEST(attr, pParamCtrl->McuDest);
				else
					SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			} else {
				SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			}
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_WSYS_CONFIG);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if (!bNeedFrag || bLastFrag) {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
				SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
			} else {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			if (pParamCtrl->McuDest != 0) {
				if (pParamCtrl->McuDest < HOST2WO) {
					Ret = chip_cmd_tx(pAd, msg);
				} else {
					Ret = call_fw_cmd_notifieriers(WO_CMD_FW_LOG_CTRL, pAd, msg->net_pkt);
					AndesFreeCmdMsg(msg);
				}
			} else {
				Ret = chip_cmd_tx(pAd, msg);
			}

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static INT32 UniCmdAccessRegBasic(
	struct _RTMP_ADAPTER *pAd,
	RTMP_REG_PAIR *RegPair,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_ACCESS_REG_BASIC_T pAccessReg = (P_UNI_CMD_ACCESS_REG_BASIC_T)pHandle;

	pAccessReg->u2Tag = UNI_CMD_ACCESS_REG_BASIC;
	pAccessReg->u2Length = sizeof(UNI_CMD_ACCESS_REG_BASIC_T);
#ifdef RT_BIG_ENDIAN
	pAccessReg->u2Tag = cpu2le16(pAccessReg->u2Tag);
	pAccessReg->u2Length = cpu2le16(pAccessReg->u2Length);
#endif
	pAccessReg->u4Addr = cpu2le32(RegPair->Register);
	pAccessReg->u4Value = cpu2le32(RegPair->Value);

	return Ret;
}

static INT32 UniCmdAccessRfRegBasic(
	struct _RTMP_ADAPTER *pAd,
	MT_RF_REG_PAIR *RfRegPair,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_ACCESS_RF_REG_BASIC_T pAccessReg = (P_UNI_CMD_ACCESS_RF_REG_BASIC_T)pHandle;

	pAccessReg->u2Tag = UNI_CMD_ACCESS_RF_REG_BASIC;
	pAccessReg->u2Length = sizeof(UNI_CMD_ACCESS_RF_REG_BASIC_T);
#ifdef RT_BIG_ENDIAN
	pAccessReg->u2Tag = cpu2le16(pAccessReg->u2Tag);
	pAccessReg->u2Length = cpu2le16(pAccessReg->u2Length);
#endif
	pAccessReg->u2WifiStream = RfRegPair->WiFiStream;
	pAccessReg->u2WifiStream = cpu2le16(pAccessReg->u2WifiStream);
	pAccessReg->u4Addr = cpu2le32(RfRegPair->Register);
	pAccessReg->u4Value = cpu2le32(RfRegPair->Value);

	return Ret;
}

static UNI_CMD_TAG_HANDLE_T UniCmdAccessRegTab[UNI_CMD_ACCESS_REG_MAX_NUM] = {
	{
		.u2CmdFeature = UNI_CMD_ACCESS_REG_BASIC,
		.u4StructSize = sizeof(UNI_CMD_ACCESS_REG_BASIC_T),
		.pfHandler = UniCmdAccessRegBasic
	},
	{
		.u2CmdFeature = UNI_CMD_ACCESS_RF_REG_BASIC,
		.u4StructSize = sizeof(UNI_CMD_ACCESS_RF_REG_BASIC_T),
		.pfHandler = UniCmdAccessRfRegBasic
	},
};

INT32 UniCmdAccessReg(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_ACCESS_REG_PARAM_T pParamCtrl)
{
	struct cmd_msg          *msg = NULL;
	INT32                   Ret = NDIS_STATUS_SUCCESS;
	UINT32                  i = 0, j = 0;
	UINT16                  u2TLVNumber = 0;
	PUCHAR					pTempBuf = NULL;
	PUCHAR					pNextHeadBuf = NULL;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4RealUseBufSize = 0;
	UINT32					u4SendBufSize = 0;
	UINT32					u4RemainingPayloadSize = 0;
	UINT32					u4ComCmdSize = 0;
	P_UNI_CMD_ACCESS_REG_T	pCmdAccessReg = NULL;
	RTMP_CHIP_CAP			*cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_ACCESS_REG_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < UNI_CMD_ACCESS_REG_MAX_NUM; i++) {
		if (pParamCtrl->AccessRegTagValid[i])
			u4CmdNeedMaxBufSize += (UniCmdAccessRegTab[i].u4StructSize * pParamCtrl->RegNum[i]);
	}

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdAccessReg = (P_UNI_CMD_ACCESS_REG_T)pNextHeadBuf;
	/* Nothing to do */
	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Traverse all support features */
	for (i = 0; i < UNI_CMD_ACCESS_REG_MAX_NUM; i++) {
		if (!pParamCtrl->AccessRegTagValid[i])
			continue;

		switch (i) {
		case UNI_CMD_ACCESS_REG_BASIC:
			if (UniCmdAccessRegTab[i].pfHandler != NULL) {
				for (j = 0 ; j < pParamCtrl->RegNum[UNI_CMD_ACCESS_REG_BASIC]; j++) {
					Ret = ((PFN_ACCESS_REG_BASIC_HANDLE)(UniCmdAccessRegTab[i].pfHandler))(pAd, &pParamCtrl->RegPair[j], pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdAccessRegTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
			}
			break;

		case UNI_CMD_ACCESS_RF_REG_BASIC:
			if (UniCmdAccessRegTab[i].pfHandler != NULL) {
				for (j = 0 ; j < pParamCtrl->RegNum[UNI_CMD_ACCESS_RF_REG_BASIC]; j++) {
					Ret = ((PFN_ACCESS_RF_REG_BASIC_HANDLE)(UniCmdAccessRegTab[i].pfHandler))(pAd, &pParamCtrl->RfRegPair[j], pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdAccessRegTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
			}
			break;

		default:
			Ret = NDIS_STATUS_SUCCESS;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"%s(): The hanlder of tag (0x%08x) not support!\n", __func__, UniCmdAccessRegTab[i].u2CmdFeature);
			break;
		}

		if (Ret != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s(): The hanlder of tag (0x%08x) return fail!\n", __func__, UniCmdAccessRegTab[i].u2CmdFeature);
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			" TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			 u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}
			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_ACCESS_REG);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if (!bNeedFrag || bLastFrag) {
				if (pParamCtrl->bQueryMode) {
					if (pParamCtrl->AccessRegTagValid[UNI_CMD_ACCESS_REG_BASIC])
						SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pParamCtrl->RegPair);
					else if (pParamCtrl->AccessRegTagValid[UNI_CMD_ACCESS_RF_REG_BASIC])
						SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pParamCtrl->RfRegPair);
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_WAIT_RETRY_RSP);
					SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
					SET_CMD_ATTR_RSP_HANDLER(attr, UniEventAccessRegHandler);
				} else {
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
					SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
					SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
				}
			} else {
				if (pParamCtrl->bQueryMode)
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_RETRY);
				else
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			Ret = chip_cmd_tx(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

INT32 UniCmdMultipleMacRegAccessRead(struct _RTMP_ADAPTER *pAd, struct _RTMP_REG_PAIR *RegPair, UINT32 Num)
{
	UNI_CMD_ACCESS_REG_PARAM_T AccessRegParam;

	os_zero_mem(&AccessRegParam, sizeof(AccessRegParam));

	AccessRegParam.bQueryMode = TRUE;
	AccessRegParam.RegPair = RegPair;
	AccessRegParam.RegNum[UNI_CMD_ACCESS_REG_BASIC] = Num;
	AccessRegParam.AccessRegTagValid[UNI_CMD_ACCESS_REG_BASIC] = TRUE;

	return UniCmdAccessReg(pAd, &AccessRegParam);
}

INT32 UniCmdMultipleMacRegAccessWrite(struct _RTMP_ADAPTER *pAd, struct _RTMP_REG_PAIR *RegPair, UINT32 Num)
{
	UNI_CMD_ACCESS_REG_PARAM_T AccessRegParam;

	os_zero_mem(&AccessRegParam, sizeof(AccessRegParam));

	AccessRegParam.bQueryMode = FALSE;
	AccessRegParam.RegPair = RegPair;
	AccessRegParam.RegNum[UNI_CMD_ACCESS_REG_BASIC] = Num;
	AccessRegParam.AccessRegTagValid[UNI_CMD_ACCESS_REG_BASIC] = TRUE;

	return UniCmdAccessReg(pAd, &AccessRegParam);
}

INT32 UniCmdMultipleRfRegAccessRead(struct _RTMP_ADAPTER *pAd, struct _MT_RF_REG_PAIR *RegPair, UINT32 Num)
{
	UNI_CMD_ACCESS_REG_PARAM_T AccessRegParam;

	os_zero_mem(&AccessRegParam, sizeof(AccessRegParam));

	AccessRegParam.bQueryMode = TRUE;
	AccessRegParam.RfRegPair = RegPair;
	AccessRegParam.RegNum[UNI_CMD_ACCESS_RF_REG_BASIC] = Num;
	AccessRegParam.AccessRegTagValid[UNI_CMD_ACCESS_RF_REG_BASIC] = TRUE;

	return UniCmdAccessReg(pAd, &AccessRegParam);
}

INT32 UniCmdMultipleRfRegAccessWrite(struct _RTMP_ADAPTER *pAd, struct _MT_RF_REG_PAIR *RegPair, UINT32 Num)
{
	UNI_CMD_ACCESS_REG_PARAM_T AccessRegParam;

	os_zero_mem(&AccessRegParam, sizeof(AccessRegParam));

	AccessRegParam.bQueryMode = FALSE;
	AccessRegParam.RfRegPair = RegPair;
	AccessRegParam.RegNum[UNI_CMD_ACCESS_RF_REG_BASIC] = Num;
	AccessRegParam.AccessRegTagValid[UNI_CMD_ACCESS_RF_REG_BASIC] = TRUE;

	return UniCmdAccessReg(pAd, &AccessRegParam);
}

INT32 UniCmdRFRegAccessRead(struct _RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 *Value)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	MT_RF_REG_PAIR RfRegPair;

	os_zero_mem(&RfRegPair, sizeof(RfRegPair));

	RfRegPair.WiFiStream = (UINT8)RFIdx;
	RfRegPair.Register = Offset;

	Ret = UniCmdMultipleRfRegAccessRead(pAd, &RfRegPair, 1);
	if (Ret == NDIS_STATUS_SUCCESS)
		*Value = RfRegPair.Value;

	return Ret;
}

INT32 UniCmdRFRegAccessWrite(struct _RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 Value)
{
	MT_RF_REG_PAIR RegPair;

	os_zero_mem(&RegPair, sizeof(RegPair));

	RegPair.WiFiStream = (UINT8)RFIdx;
	RegPair.Register = Offset;
	RegPair.Value = Value;

	return UniCmdMultipleRfRegAccessWrite(pAd, &RegPair, 1);
}

INT32 MtUniCmdRestartDLReqNoRsp(struct _RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _CMD_ATTRIBUTE attr = {0};
	UNI_CMD_ID_POWER_CTRL_T UniCmdPowerCtrl;
	UNI_CMD_POWER_OFF_T UniCmdPowerOff;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdPowerCtrl);
	os_zero_mem(&UniCmdPowerCtrl, u4ComCmdSize);
	os_zero_mem(&UniCmdPowerOff, sizeof(UniCmdPowerOff));

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(UniCmdPowerOff);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9); /* MUST be only to N9 */
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_POWER_CTRL);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdPowerCtrl, u4ComCmdSize);

	/* Step 4: Fill TLV parameters here */
	UniCmdPowerOff.u2Tag = UNI_CMD_POWER_OFF;
	UniCmdPowerOff.u2Length = (u4CmdNeedMaxBufSize - u4ComCmdSize);
#ifdef RT_BIG_ENDIAN
	UniCmdPowerOff.u2Tag = cpu2le16(UniCmdPowerOff.u2Tag);
	UniCmdPowerOff.u2Length = cpu2le16(UniCmdPowerOff.u2Length);
#endif /* RT_BIG_ENDIAN */
	UniCmdPowerOff.ucPowerMode = 1;
	AndesAppendCmdMsg(msg, (char *)&UniCmdPowerOff, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(ret = %d)\n", __func__, ret);
	return ret;
}

INT32 UniCmdCfgInfoUpdate(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	ENUM_CFG_FEATURE eFeature,
	VOID *param)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	UINT8 i, ucAction = 0;
	UNI_CMD_WSYS_CFG_PARAM_T WsysCfgParam;
	UNI_CMD_BAND_CFG_PARAM_T BandCfgParam;
	P_EXT_CMD_CFG_SET_AGG_AC_LIMIT_T prExtCmdAggAcLimitCfg = NULL;

	os_zero_mem(&WsysCfgParam, sizeof(WsysCfgParam));
	os_zero_mem(&BandCfgParam, sizeof(BandCfgParam));
	BandCfgParam.ucDbdcIdx = HcGetBandByWdev(wdev);

	for (i = 0; i < EXT_CMD_CFG_MAX_NUM; i++) {
		if (eFeature & (1 << i)) {
			switch (i) {
			case EXT_CMD_CFGINFO_HOSTREPORT_TX_LATENCY:
				ucAction = *((UINT8 *) param);
				WsysCfgParam.WsysCfgHostReportTxLatency.ucActive = ucAction;
				WsysCfgParam.WsysCfgTagValid[UNI_CMD_HOSTREPORT_TX_LATENCY_CONFIG] = TRUE;
				break;
			case EXT_CMD_CFGINFO_CHECKSUM:
				/* TODO */
				break;
			case EXT_CMD_CFGINFO_RX_FILTER_DROP_CTRL_FRAME:
				ucAction = *((UINT8 *) param);
				BandCfgParam.BandCfgDropCtrlFrame.ucDropRts = (ucAction & CFGINFO_DROP_CTS_CTRL_FRAME) ? 1 : 0;
				BandCfgParam.BandCfgDropCtrlFrame.ucDropCts = (ucAction & CFGINFO_DROP_CTS_CTRL_FRAME) ? 1 : 0;
				BandCfgParam.BandCfgDropCtrlFrame.ucDropUnwantedCtrl = (ucAction & CFGINFO_DROP_UNWANTED_CTRL_FRAME) ? 1 : 0;
				BandCfgParam.BandCfgTagValid[UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME] = TRUE;
				break;
			case EXT_CMD_CFGINFO_AGG_AC_LIMIT:
				prExtCmdAggAcLimitCfg = (P_EXT_CMD_CFG_SET_AGG_AC_LIMIT_T)param;
				BandCfgParam.BandCfgAGGAcLimit.ucWmmIdx = prExtCmdAggAcLimitCfg->ucWmmIdx;
				BandCfgParam.BandCfgAGGAcLimit.ucAc = prExtCmdAggAcLimitCfg->ucAc;
				BandCfgParam.BandCfgAGGAcLimit.ucAggLimit = prExtCmdAggAcLimitCfg->ucAggLimit;
				BandCfgParam.BandCfgTagValid[UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT] = TRUE;
				break;
			case EXT_CMD_CFGINFO_CERT_CFG:
				/* TODO */
				break;
			}
		}
	}

	for (i = 0; i < UNI_CMD_WSYS_CONFIG_MAX_NUM; i++) {
		if (WsysCfgParam.WsysCfgTagValid[i]) {
			Ret = UniCmdWsysConfig(pAd, &WsysCfgParam);
			break;
		}
	}
	if (Ret != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"%d => UniCmdWsysConfig return fail!\n", i);

	for (i = 0; i < UNI_CMD_BAND_CONFIG_MAX_NUM; i++) {
		if (BandCfgParam.BandCfgTagValid[i]) {
			Ret = UniCmdBandConfig(pAd, &BandCfgParam);
			break;
		}
	}
	if (Ret != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"%d => UniCmdBandConfig return fail!\n", i);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

INT32 MtUniCmdFwLog2Host(struct _RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 FWLog2HostCtrl)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	char *Dest[] = {
		"HOST2N9(WM)",
		"CR42N9",
		"HOST2CR4(WA)",
		"HOST2CR4N9",
		"HOST2WO",
	};
	UNI_CMD_WSYS_CFG_PARAM_T WsysCfgParam;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: McuDest(%d): %s\n", __func__, McuDest, Dest[McuDest]));

	os_zero_mem(&WsysCfgParam, sizeof(WsysCfgParam));
	WsysCfgParam.McuDest = McuDest;
	WsysCfgParam.WsysFwLogCtrlBasic.ucFwLog2HostCtrl = (UINT8)FWLog2HostCtrl;
	WsysCfgParam.WsysFwLogCtrlBasic.ucFwLog2HostInterval = 0;
	WsysCfgParam.WsysCfgTagValid[UNI_CMD_WSYS_CONFIG_FW_LOG_CTRL] = TRUE;

	Ret = UniCmdWsysConfig(pAd, &WsysCfgParam);

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: %s (ret = %d)\n", __func__, Dest[McuDest], Ret));

	return Ret;
}

INT32 MtUniCmdFwDbgCtrl(struct _RTMP_ADAPTER *pAd, UINT32 DbgClass, UINT32 ModuleIdx)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	UNI_CMD_WSYS_CFG_PARAM_T WsysCfgParam;

	os_zero_mem(&WsysCfgParam, sizeof(WsysCfgParam));
	WsysCfgParam.WsysFwDbgCtrl.u4DbgModuleIdx = ModuleIdx;
	WsysCfgParam.WsysFwDbgCtrl.ucDbgClass = (UINT8)DbgClass;
	WsysCfgParam.WsysCfgTagValid[UNI_CMD_WSYS_CONFIG_FW_DBG_CTRL] = TRUE;

	Ret = UniCmdWsysConfig(pAd, &WsysCfgParam);

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: ret = %d\n", __func__, Ret));

	return Ret;
}

#ifdef AIR_MONITOR
INT32 MtUniCmdSmeshConfigSet(struct _RTMP_ADAPTER *pAd, UCHAR *pdata, P_UNI_CMD_SMESH_PARAM_T prSmeshResult)
{
	struct cmd_msg			*msg = NULL;
	INT32					Ret = NDIS_STATUS_SUCCESS;
	PUCHAR					pTempBuf = NULL;
	PUCHAR					pNextHeadBuf = NULL;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4ComCmdSize = 0;
	UNI_CMD_SMESH_T			CmdSmesh = {0};
	UNI_CMD_SMESH_PARAM_T	CmdSmeshParam = {0};
	struct _CMD_ATTRIBUTE	attr = {0};
	EXT_CMD_SMESH_T 		*pCfgSmesh = (EXT_CMD_SMESH_T *)pdata;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(CmdSmesh);
	os_zero_mem(&CmdSmesh, u4ComCmdSize);
	os_zero_mem(&CmdSmeshParam, sizeof(CmdSmeshParam));

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(CmdSmeshParam);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_CFG_SMESH);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	if (pCfgSmesh->ucAccessMode) {
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
		SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
	} else {
		SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, prSmeshResult);
		SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_WAIT_RETRY_RSP);
		SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
		SET_CMD_ATTR_RSP_HANDLER(attr, UniEventSmeshInfoRsp);
	}
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	CmdSmesh.ucBand = pCfgSmesh->ucBand;
	AndesAppendCmdMsg(msg, (char *)&CmdSmesh, u4ComCmdSize);

	/* Step 4: Fill TLV parameters here */
	CmdSmeshParam.u2Tag = UNI_CMD_SMESH_PARAM;
	CmdSmeshParam.u2Length = (u4CmdNeedMaxBufSize - u4ComCmdSize);
#ifdef RT_BIG_ENDIAN
	CmdSmeshParam.u2Tag = cpu2le16(CmdSmeshParam.u2Tag);
	CmdSmeshParam.u2Length = cpu2le16(CmdSmeshParam.u2Length);
#endif /* RT_BIG_ENDIAN */
	CmdSmeshParam.ucEntryEnable = pCfgSmesh->ucSmeshEn;
	CmdSmeshParam.fgSmeshRxA2 = pCfgSmesh->fgSmeshRxA2;
	CmdSmeshParam.fgSmeshRxA1 = pCfgSmesh->fgSmeshRxA1;
	CmdSmeshParam.fgSmeshRxData = pCfgSmesh->fgSmeshRxData;
	CmdSmeshParam.fgSmeshRxMgnt = pCfgSmesh->fgSmeshRxMgnt;
	CmdSmeshParam.fgSmeshRxCtrl = pCfgSmesh->fgSmeshRxCtrl;
	AndesAppendCmdMsg(msg, (char *)&CmdSmeshParam, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"ucEntryEnable(%d), fgSmeshRxA2(%d), fgSmeshRxA1(%d), fgSmeshRxData(%d), fgSmeshRxMgnt(%d), fgSmeshRxCtrl(%d)\n",
			   CmdSmeshParam.ucEntryEnable, CmdSmeshParam.fgSmeshRxA2, CmdSmeshParam.fgSmeshRxA1, CmdSmeshParam.fgSmeshRxData,
			  CmdSmeshParam.fgSmeshRxMgnt, CmdSmeshParam.fgSmeshRxCtrl);

	Ret = chip_cmd_tx(pAd, msg);

error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(Ret = %d)\n", __func__, Ret);

	return Ret;
}
#endif /* AIR_MONITOR */

static INT32 UniCmdRxHdrTranEnable(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_RX_HDR_TRAN_ENABLE_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_RX_HDR_TRAN_ENABLE_T pRxHdrTranEnable = (P_UNI_CMD_RX_HDR_TRAN_ENABLE_T)pHandle;

	pRxHdrTranEnable->u2Tag = UNI_CMD_RX_HDR_TRAN_ENABLE;
	pRxHdrTranEnable->u2Length = sizeof(UNI_CMD_RX_HDR_TRAN_ENABLE_T);
#ifdef RT_BIG_ENDIAN
	pRxHdrTranEnable->u2Tag = cpu2le16(pRxHdrTranEnable->u2Tag);
	pRxHdrTranEnable->u2Length = cpu2le16(pRxHdrTranEnable->u2Length);
#endif
	pRxHdrTranEnable->fgEnable = pParam->fgEnable;
	pRxHdrTranEnable->fgCheckBssid = pParam->fgCheckBssid;
	pRxHdrTranEnable->ucTranslationMode = pParam->ucTranslationMode;

	return Ret;
}

static INT32 UniCmdRxHdrTranVlanConfig(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_RX_HDR_TRAN_VLAN_T pParam,
	VOID *pHandle)

{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_RX_HDR_TRAN_VLAN_T pRxHdrTranVlan = (P_UNI_CMD_RX_HDR_TRAN_VLAN_T)pHandle;

	pRxHdrTranVlan->u2Tag = UNI_CMD_RX_HDR_TRAN_ENABLE;
	pRxHdrTranVlan->u2Length = sizeof(UNI_CMD_RX_HDR_TRAN_VLAN_T);
#ifdef RT_BIG_ENDIAN
	pRxHdrTranVlan->u2Tag = cpu2le16(pRxHdrTranVlan->u2Tag);
	pRxHdrTranVlan->u2Length = cpu2le16(pRxHdrTranVlan->u2Length);
#endif
	pRxHdrTranVlan->fgInsertVlan = pParam->fgInsertVlan;
	pRxHdrTranVlan->fgRemoveVlan = pParam->fgRemoveVlan;
	pRxHdrTranVlan->fgUseQosTid = pParam->fgUseQosTid;

	return Ret;
}

static INT32 UniCmdRxHdrTranBlackListConfig(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_RX_HDR_TRAN_BLACKLIST_T pParam,
	VOID *pHandle)

{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_RX_HDR_TRAN_BLACKLIST_T pRxHdrTranBlackList = (P_UNI_CMD_RX_HDR_TRAN_BLACKLIST_T)pHandle;

	pRxHdrTranBlackList->u2Tag = UNI_CMD_RX_HDR_TRAN_BLACKLIST_CONFIG;
	pRxHdrTranBlackList->u2Length = sizeof(UNI_CMD_RX_HDR_TRAN_BLACKLIST_T);
#ifdef RT_BIG_ENDIAN
	pRxHdrTranBlackList->u2Tag = cpu2le16(pRxHdrTranBlackList->u2Tag);
	pRxHdrTranBlackList->u2Length = cpu2le16(pRxHdrTranBlackList->u2Length);
#endif
	pRxHdrTranBlackList->fgEnable = pParam->fgEnable;
	pRxHdrTranBlackList->ucBlackListIdx = pParam->ucBlackListIdx;
	pRxHdrTranBlackList->u2EtherType = cpu2le16(pParam->u2EtherType);

	return Ret;
}

static UNI_CMD_TAG_HANDLE_T UniCmdRxHdrTransTab[UNI_CMD_RX_HDR_TRAN_MAX_NUM] = {
	{
		.u2CmdFeature = UNI_CMD_RX_HDR_TRAN_ENABLE,
		.u4StructSize = sizeof(UNI_CMD_RX_HDR_TRAN_ENABLE_T),
		.pfHandler = UniCmdRxHdrTranEnable
	},
	{
		.u2CmdFeature = UNI_CMD_RX_HDR_TRAN_VLAN_CONFIG,
		.u4StructSize = sizeof(UNI_CMD_RX_HDR_TRAN_VLAN_T),
		.pfHandler = UniCmdRxHdrTranVlanConfig
	},
	{
		.u2CmdFeature = UNI_CMD_RX_HDR_TRAN_BLACKLIST_CONFIG,
		.u4StructSize = sizeof(UNI_CMD_RX_HDR_TRAN_BLACKLIST_T),
		.pfHandler = UniCmdRxHdrTranBlackListConfig
	},
};

INT32 UniCmdRxHdrTrans(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_RX_HDR_TRAN_PARAM_T pParamCtrl)
{
	struct cmd_msg          *msg = NULL;
	INT32                   Ret = NDIS_STATUS_SUCCESS;
	UINT32                  i = 0;
	UINT16                  u2TLVNumber = 0;
	PUCHAR					pTempBuf = NULL;
	PUCHAR					pNextHeadBuf = NULL;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4RealUseBufSize = 0;
	UINT32					u4SendBufSize = 0;
	UINT32					u4RemainingPayloadSize = 0;
	UINT32					u4ComCmdSize = 0;
	P_UNI_CMD_RX_HDR_TRAN_T	pCmdRxHdrTran = NULL;
	RTMP_CHIP_CAP			*cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_RX_HDR_TRAN_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < UNI_CMD_RX_HDR_TRAN_MAX_NUM; i++) {
		if (pParamCtrl->RxHdrTranValid[i])
			u4CmdNeedMaxBufSize += UniCmdRxHdrTransTab[i].u4StructSize;
	}

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdRxHdrTran = (P_UNI_CMD_RX_HDR_TRAN_T)pNextHeadBuf;
	/* Nothing to do */
	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Traverse all support features */
	for (i = 0; i < UNI_CMD_RX_HDR_TRAN_MAX_NUM; i++) {
		if (pParamCtrl->RxHdrTranValid[i]) {
			switch (i) {
			case UNI_CMD_RX_HDR_TRAN_ENABLE:
				if (UniCmdRxHdrTransTab[i].pfHandler != NULL) {
					Ret = ((PFN_RX_HDR_TRAN_ENABLE_HANDLE)(UniCmdRxHdrTransTab[i].pfHandler))(pAd, &pParamCtrl->RxHdrTranEnable, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdRxHdrTransTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_RX_HDR_TRAN_VLAN_CONFIG:
				if (UniCmdRxHdrTransTab[i].pfHandler != NULL) {
					Ret = ((PFN_RX_HDR_TRAN_VLAN_HANDLE)(UniCmdRxHdrTransTab[i].pfHandler))(pAd, &pParamCtrl->RxHdrTranVlan, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdRxHdrTransTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_RX_HDR_TRAN_BLACKLIST_CONFIG:
				if (UniCmdRxHdrTransTab[i].pfHandler != NULL) {
					Ret = ((PFN_RX_HDR_TRAN_BLACKLIST_HANDLE)(UniCmdRxHdrTransTab[i].pfHandler))(pAd, &pParamCtrl->RxHdrTranBlackList, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdRxHdrTransTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			default:
				Ret = NDIS_STATUS_SUCCESS;
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s(): The hanlder of tag (0x%08x) not support!\n", __func__, UniCmdRxHdrTransTab[i].u2CmdFeature);
				break;
			}

			if (Ret != NDIS_STATUS_SUCCESS)
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"%s(): The hanlder of tag (0x%08x) return fail!\n", __func__, UniCmdRxHdrTransTab[i].u2CmdFeature);
		}
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_RX_HDR_TRAN);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if (!bNeedFrag || bLastFrag) {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
				SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
			} else {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			Ret = chip_cmd_tx(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static INT32 UniCmdSERSetQuery(
	struct _RTMP_ADAPTER *pAd,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_SER_QUERY_T pSerSetQuery = (P_UNI_CMD_SER_QUERY_T)pHandle;

	pSerSetQuery->u2Tag = UNI_CMD_SER_QUERY;
	pSerSetQuery->u2Length = sizeof(UNI_CMD_SER_QUERY_T);
#ifdef RT_BIG_ENDIAN
	pSerSetQuery->u2Tag = cpu2le16(pSerSetQuery->u2Tag);
	pSerSetQuery->u2Length = cpu2le16(pSerSetQuery->u2Length);
#endif /* RT_BIG_ENDIAN */

	return Ret;
}

static INT32 UniCmdSERSetEnable(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgEnable,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_SER_ENABLE_T pSerSetEnable = (P_UNI_CMD_SER_ENABLE_T)pHandle;

	pSerSetEnable->u2Tag = UNI_CMD_SER_ENABLE;
	pSerSetEnable->u2Length = sizeof(UNI_CMD_SER_ENABLE_T);
#ifdef RT_BIG_ENDIAN
	pSerSetEnable->u2Tag = cpu2le16(pSerSetEnable->u2Tag);
	pSerSetEnable->u2Length = cpu2le16(pSerSetEnable->u2Length);
#endif /* RT_BIG_ENDIAN */
	pSerSetEnable->fgEnable = fgEnable;

	return Ret;
}

static INT32 UniCmdSERSetEnableMask(
	struct _RTMP_ADAPTER *pAd,
	UINT32 u4EnableMask,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_SER_SET_T pSerSetEnableMask = (P_UNI_CMD_SER_SET_T)pHandle;

	pSerSetEnableMask->u2Tag = UNI_CMD_SER_SET;
	pSerSetEnableMask->u2Length = sizeof(UNI_CMD_SER_SET_T);
#ifdef RT_BIG_ENDIAN
	pSerSetEnableMask->u2Tag = cpu2le16(pSerSetEnableMask->u2Tag);
	pSerSetEnableMask->u2Length = cpu2le16(pSerSetEnableMask->u2Length);
#endif /* RT_BIG_ENDIAN */
	pSerSetEnableMask->u4EnableMask = cpu2le32(u4EnableMask);

	return Ret;
}

static INT32 UniCmdSERSetTrigger(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucTriggerMethod,
	UINT8 ucDbdcIdx,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_SER_TRIGGER_T pSerSetTrigger = (P_UNI_CMD_SER_TRIGGER_T)pHandle;

	pSerSetTrigger->u2Tag = UNI_CMD_SER_TRIGGER;
	pSerSetTrigger->u2Length = sizeof(UNI_CMD_SER_TRIGGER_T);
#ifdef RT_BIG_ENDIAN
	pSerSetTrigger->u2Tag = cpu2le16(pSerSetTrigger->u2Tag);
	pSerSetTrigger->u2Length = cpu2le16(pSerSetTrigger->u2Length);
#endif /* RT_BIG_ENDIAN */
	pSerSetTrigger->ucTriggerMethod = ucTriggerMethod;
	pSerSetTrigger->ucDbdcIdx = ucDbdcIdx;

	return Ret;
}

static UNI_CMD_TAG_HANDLE_T UniCmdSerTab[UNI_CMD_SER_MAX_NUM] = {
	{
		.u2CmdFeature = UNI_SER_ACTION_SET_QUERY,
		.u4StructSize = sizeof(UNI_CMD_SER_QUERY_T),
		.pfHandler = UniCmdSERSetQuery
	},
	{
		.u2CmdFeature = UNI_SER_ACTION_SET_ENABLE,
		.u4StructSize = sizeof(UNI_CMD_SER_ENABLE_T),
		.pfHandler = UniCmdSERSetEnable
	},
	{
		.u2CmdFeature = UNI_SER_ACTION_SET_ENABLE_MASK,
		.u4StructSize = sizeof(UNI_CMD_SER_SET_T),
		.pfHandler = UniCmdSERSetEnableMask
	},
	{
		.u2CmdFeature = UNI_SER_ACTION_SET_TRIGGER,
		.u4StructSize = sizeof(UNI_CMD_SER_TRIGGER_T),
		.pfHandler = UniCmdSERSetTrigger
	},
};

INT32 UniCmdSER(struct _RTMP_ADAPTER *pAd, UINT32 u4Action, UINT32 u4SetValue, UINT8 ucDbdcIdx)
{
	struct cmd_msg          *msg = NULL;
	INT32                   Ret = NDIS_STATUS_SUCCESS;
	UINT32                  i = 0;
	UINT16                  u2TLVNumber = 0;
	PUCHAR					pTempBuf = NULL;
	PUCHAR					pNextHeadBuf = NULL;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4RealUseBufSize = 0;
	UINT32					u4SendBufSize = 0;
	UINT32					u4RemainingPayloadSize = 0;
	UINT32					u4ComCmdSize = 0;
	P_UNI_CMD_SER_T			pCmdSer = NULL;
	RTMP_CHIP_CAP			*cap = hc_get_chip_cap(pAd->hdev_ctrl);
	BOOLEAN					bQueryOnly = FALSE;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_SER_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < UNI_CMD_SER_MAX_NUM; i++) {
		if (u4Action & UniCmdSerTab[i].u2CmdFeature)
			u4CmdNeedMaxBufSize += UniCmdSerTab[i].u4StructSize;
	}

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdSer = (P_UNI_CMD_SER_T)pNextHeadBuf;
	/* Nothing to do */
	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Traverse all support features */
	for (i = 0; i < UNI_CMD_SER_MAX_NUM; i++) {
		switch (u4Action & UniCmdSerTab[i].u2CmdFeature) {
		case UNI_SER_ACTION_SET_QUERY:
			if (UniCmdSerTab[i].pfHandler != NULL) {
				Ret = ((PFN_SER_QUERY_HANDLE)(UniCmdSerTab[i].pfHandler))(pAd, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += UniCmdSerTab[i].u4StructSize;
					u2TLVNumber++;
				}
			}
			break;

		case UNI_SER_ACTION_SET_ENABLE:
			if (UniCmdSerTab[i].pfHandler != NULL) {
				BOOLEAN bEnable = (u4SetValue == SER_SET_ENABLE)?TRUE:FALSE;
				Ret = ((PFN_SER_ENABLE_HANDLE)(UniCmdSerTab[i].pfHandler))(pAd, bEnable, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += UniCmdSerTab[i].u4StructSize;
					u2TLVNumber++;
				}
			}
			break;

		case UNI_SER_ACTION_SET_ENABLE_MASK:
			if (UniCmdSerTab[i].pfHandler != NULL) {
				Ret = ((PFN_SER_ENABLE_MASK_HANDLE)(UniCmdSerTab[i].pfHandler))(pAd, u4SetValue, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += UniCmdSerTab[i].u4StructSize;
					u2TLVNumber++;
				}
			}
			break;

		case UNI_SER_ACTION_SET_TRIGGER:
			if (UniCmdSerTab[i].pfHandler != NULL) {
				Ret = ((PFN_SER_TRIGGER_HANDLE)(UniCmdSerTab[i].pfHandler))(pAd, (UINT8)u4SetValue, ucDbdcIdx, pNextHeadBuf);
				if (Ret == NDIS_STATUS_SUCCESS) {
					pNextHeadBuf += UniCmdSerTab[i].u4StructSize;
					u2TLVNumber++;
				}
			}
			break;

		default:
			Ret = NDIS_STATUS_SUCCESS;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				" The hanlder of tag (0x%08x) not support!\n", u4Action);
			break;
		}

		if (Ret != NDIS_STATUS_SUCCESS)
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s(): The hanlder of tag (0x%08x) return fail!\n", __func__, UniCmdSerTab[i].u2CmdFeature);
	}

	if ((u2TLVNumber == 1) && (u4Action & UNI_SER_ACTION_SET_QUERY))
		bQueryOnly = TRUE;

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			 u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SER);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if (!bNeedFrag || bLastFrag) {
				if (bQueryOnly) {
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_WAIT_RETRY_RSP);
					SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
					SET_CMD_ATTR_RSP_HANDLER(attr, UniEventSERHandler);
				} else {
					if ((u4Action & UNI_SER_ACTION_SET_TRIGGER) &&
						(u4SetValue == SER_SET_L1_RECOVER)) {
						SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
						SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
						SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
					} else {
						SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
						SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
						SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
					}
				}
			} else {
				if (bQueryOnly)
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_RETRY);
				else
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			Ret = chip_cmd_tx(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
INT32 UniCmdTWT(struct _RTMP_ADAPTER *pAd, struct mt_twt_agrt_para TwtAgrtPara)
{
	struct cmd_msg *msg;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UNI_CMD_TWT_T UniCmdTWT;
	UNI_CMD_TWT_ARGT_UPDATE_T UniCmdTWTArgtUpdate;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	UINT32 i;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdTWT);
	os_zero_mem(&UniCmdTWT, u4ComCmdSize);
	os_zero_mem(&UniCmdTWTArgtUpdate, sizeof(UniCmdTWTArgtUpdate));

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(UniCmdTWTArgtUpdate);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_TWT);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
	SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	UniCmdTWT.ucBssInfoIdx = TwtAgrtPara.bss_idx;
	AndesAppendCmdMsg(msg, (char *)&UniCmdTWT, u4ComCmdSize);

	/* Step 4: Fill TLV parameters here */
	UniCmdTWTArgtUpdate.u2Tag = UNI_CMD_TWT_AGRT_UPDATE;
	UniCmdTWTArgtUpdate.u2Length = (u4CmdNeedMaxBufSize - u4ComCmdSize);
#ifdef RT_BIG_ENDIAN
	UniCmdTWTArgtUpdate.u2Tag = cpu2le16(UniCmdTWTArgtUpdate.u2Tag);
	UniCmdTWTArgtUpdate.u2Length = cpu2le16(UniCmdTWTArgtUpdate.u2Length);
#endif /* RT_BIG_ENDIAN */
	UniCmdTWTArgtUpdate.ucAgrtTblIdx = TwtAgrtPara.agrt_tbl_idx;
	UniCmdTWTArgtUpdate.ucAgrtCtrlFlag = TwtAgrtPara.agrt_ctrl_flag;
	UniCmdTWTArgtUpdate.ucOwnMacId = TwtAgrtPara.own_mac_idx;
	UniCmdTWTArgtUpdate.ucFlowId = TwtAgrtPara.flow_id;
	UniCmdTWTArgtUpdate.u2PeerIdGrpId = TwtAgrtPara.peer_id_grp_id;
	UniCmdTWTArgtUpdate.ucAgrtSpDuration = TwtAgrtPara.agrt_sp_duration;
	UniCmdTWTArgtUpdate.ucBssIndex = TwtAgrtPara.bss_idx;
	UniCmdTWTArgtUpdate.u4AgrtSpStartTsf_low = cpu2le32(TwtAgrtPara.agrt_sp_start_tsf_low);
	UniCmdTWTArgtUpdate.u4AgrtSpStartTsf_high = cpu2le32(TwtAgrtPara.agrt_sp_start_tsf_high);
	UniCmdTWTArgtUpdate.u2AgrtSpWakeIntvlMantissa = cpu2le16(TwtAgrtPara.agrt_sp_wake_intvl_mantissa);
	UniCmdTWTArgtUpdate.ucAgrtSpWakeIntvlExponent = TwtAgrtPara.agrt_sp_wake_intvl_exponent;
	UniCmdTWTArgtUpdate.fgIsRoleAp = TwtAgrtPara.is_role_ap;
	UniCmdTWTArgtUpdate.ucAgrtParaBitmap = TwtAgrtPara.agrt_para_bitmap;
	UniCmdTWTArgtUpdate.ucReserved_a = TwtAgrtPara.persistence;
	UniCmdTWTArgtUpdate.u2Reserved_b = cpu2le16(TwtAgrtPara.reserved_b);
	UniCmdTWTArgtUpdate.ucGrpMemberCnt = TwtAgrtPara.grp_member_cnt;
	UniCmdTWTArgtUpdate.ucReserved_c = TwtAgrtPara.reserved_c;
	UniCmdTWTArgtUpdate.u2Reserved_d = cpu2le16(TwtAgrtPara.reserved_d);
	for (i = 0; i < TWT_HW_GRP_MAX_MEMBER_CNT; i++)
		UniCmdTWTArgtUpdate.au2StaList[i] = cpu2le16(TwtAgrtPara.sta_list[i]);

	AndesAppendCmdMsg(msg, (char *)&UniCmdTWTArgtUpdate, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	ret = chip_cmd_tx(pAd, msg);
	return ret;

error:
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"ret = %d)\n", ret);
	return ret;
}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */


/*********************************************
 * UNI_CMD_ID_REPT_MUAR (0x09)
 * UNI_CMD_ID_NORM_MUAR (0x0A)
 *********************************************/
INT32 UniCmdMuarConfigSet(struct _RTMP_ADAPTER *pAd, UCHAR *pdata)
{
	struct cmd_msg          		*msg = NULL;
	EXT_CMD_MUAR_T 					*pconfig_muar = (EXT_CMD_MUAR_T *)pdata;
	UINT8							ucEntryCnt = pconfig_muar->ucEntryCnt;
	UINT32							u4CmdNeedMaxBufSize = 0;
	UINT32							u4RealUseBufSize = 0;
	UINT32							u4SendBufSize = 0;
	UINT32							u4RemainingPayloadSize = 0;
	INT32							Ret = NDIS_STATUS_SUCCESS;
	UINT32							u4ComCmdSize = 0;
	UINT16                 			u2TLVNumber = 0;
	UINT8							i = 0;
	PUCHAR							pTempBuf = NULL;
	PUCHAR							pNextHeadBuf = NULL;
	PUCHAR							pExtCmdBuf = pdata;
	P_UNI_CMD_MUAR_T 				pUniCmdMuar = NULL;
	P_UNI_CMD_MUAR_ENTRY_T 			pUniCmdMuarEntry = NULL;
	P_EXT_CMD_MUAR_MULTI_ENTRY_T 	pExtCmdMuarEntry = NULL;
	RTMP_CHIP_CAP					*cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_MUAR_T);
	u4CmdNeedMaxBufSize = u4ComCmdSize + (ucEntryCnt * sizeof(UNI_CMD_MUAR_ENTRY_T));

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pUniCmdMuar = (P_UNI_CMD_MUAR_T)pNextHeadBuf;
	pUniCmdMuar->ucBand = pconfig_muar->ucBand;
	pNextHeadBuf += sizeof(UNI_CMD_MUAR_T);

	/* Step 4: Fill all tags here */
	pExtCmdBuf += sizeof(EXT_CMD_MUAR_T);
	for (i = 0; i < ucEntryCnt; i++) {
		pExtCmdMuarEntry = (P_EXT_CMD_MUAR_MULTI_ENTRY_T)pExtCmdBuf;
		pUniCmdMuarEntry = (P_UNI_CMD_MUAR_ENTRY_T) pNextHeadBuf;

		pUniCmdMuarEntry->u2Tag = cpu2le16(UNI_CMD_MUAR_ENTRY);
		pUniCmdMuarEntry->u2Length = cpu2le16(sizeof(UNI_CMD_MUAR_ENTRY_T));
		pUniCmdMuarEntry->fgSmesh = FALSE; /* TODO */
		pUniCmdMuarEntry->ucHwBssIndex = pExtCmdMuarEntry->ucBssid;
		pUniCmdMuarEntry->ucMuarIdx = pExtCmdMuarEntry->ucMuarIdx;
		pUniCmdMuarEntry->ucEntryAdd = 0;
		NdisCopyMemory(pUniCmdMuarEntry->aucMacAddr, pExtCmdMuarEntry->aucMacAddr, sizeof(pUniCmdMuarEntry->aucMacAddr));

		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s(): u2Tag = %d, u2Length = %d, fgSmesh = %d, ucHwBssIndex = %d, ucMuarIdx = %d, ucEntryAdd = %d, pUniCmdMuarEntry->aucMacAddr = %02x:%02x:%02x:%02x:%02x:%02x\n",
			__func__,
			le2cpu16(pUniCmdMuarEntry->u2Tag),
			le2cpu16(pUniCmdMuarEntry->u2Length),
			pUniCmdMuarEntry->fgSmesh,
			pUniCmdMuarEntry->ucHwBssIndex,
			pUniCmdMuarEntry->ucMuarIdx,
			pUniCmdMuarEntry->ucEntryAdd,
			PRINT_MAC(pUniCmdMuarEntry->aucMacAddr));

		pExtCmdBuf += sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T);
		pNextHeadBuf += sizeof(UNI_CMD_MUAR_ENTRY_T);
		u2TLVNumber++;
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			" BAND = %d, TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			pUniCmdMuar->ucBand, u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			if (pconfig_muar->ucMuarModeSel == MUAR_REPEATER)
				SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_REPT_MUAR);
			else
				SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_NORM_MUAR);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if (!bNeedFrag || bLastFrag) {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
				SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
			} else {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			Ret = chip_cmd_tx(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s:(Ret = %d)\n", __func__, Ret);

	return Ret;
}

/*********************************************
 * UNI_CMD_ID_ECC_OPER (0x18)
 *********************************************/
static struct _EC_GROUP_LENGTH_MAP_T ec_group_map[] = {
	{ECDH_GROUP_ID_256BIT, ECDH_LENGTH_256BIT},
	{ECDH_GROUP_ID_384BIT, ECDH_LENGTH_384BIT},
	{ECDH_GROUP_ID_521BIT, ECDH_LENGTH_521BIT},
	{ECDH_GROUP_ID_192BIT, ECDH_LENGTH_192BIT},
	{ECDH_GROUP_ID_224BIT, ECDH_LENGTH_224BIT},
	{0, 0}
};

#define SIZE_EC_GROUP_MAP   (sizeof(ec_group_map) / sizeof(struct _EC_GROUP_LENGTH_MAP_T))

INT32 UniCmdCalculateECC(struct _RTMP_ADAPTER *pAd, UINT32 oper, UINT32 group, UINT8 *scalar, UINT8 *point_x, UINT8 *point_y)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	UNI_CMD_ECC_OP_T	 UniCmdEccOp;
	UNI_CMD_ECC_OP_CAL_T UniCmdEccOpCal;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 size = 0, i, element_len = 0, cal_mode = ECC_CAL_DG_MODE;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT32 offset = 0;
	static UINT8 ecc_cmd_id;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdEccOp);
	os_zero_mem(&UniCmdEccOp, u4ComCmdSize);
	os_zero_mem(&UniCmdEccOpCal, sizeof(UniCmdEccOpCal));

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(UniCmdEccOpCal);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"oper = %d, group = %d\n", oper, group);

	for (i = 0; i < SIZE_EC_GROUP_MAP; i++) {
		if (group == ec_group_map[i].group_id) {
			element_len = ec_group_map[i].element_len;
			break;
		}

		if (ec_group_map[i].group_id == 0) {
			ret = NDIS_STATUS_INVALID_DATA;
			goto error;
		}
	}

	if (scalar)
		size += element_len;
	else {
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}

	if ((point_x) && (point_y)) {
		size += (element_len * 2);
		cal_mode = ECC_CAL_DQ_MODE;
	} else if (((point_x) && (point_y == NULL)) ||
		((point_x == NULL) && (point_y))) {
		/*we don't support pass x or y coordinate only. need to pass whole x & y coordingnates.*/
		ret = NDIS_STATUS_INVALID_DATA;
		goto error;
	}
	u4CmdNeedMaxBufSize += size;

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_ECC_OPER);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
	SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	AndesAppendCmdMsg(msg, (char *)&UniCmdEccOp, (u4ComCmdSize));

	/* Step 4: Fill and append TLV parameters here */
	UniCmdEccOpCal.u2Tag = UNI_CMD_ECC_OP_CAL_GROUP_POINT;
	UniCmdEccOpCal.u2Length = (u4CmdNeedMaxBufSize - u4ComCmdSize);
	UniCmdEccOpCal.ucGroupID = group;
	UniCmdEccOpCal.ucDataLength = size;
	UniCmdEccOpCal.ucDataType = cal_mode;
	UniCmdEccOpCal.ucEccCmdId = ecc_cmd_id;
	ecc_cmd_id++;
	NdisMoveMemory(&(UniCmdEccOpCal.aucBuffer[offset]), scalar, element_len);

	if (cal_mode == ECC_CAL_DQ_MODE) {
		offset += element_len;
		NdisMoveMemory(&(UniCmdEccOpCal.aucBuffer[offset]), point_x, element_len);
		offset += element_len;
		NdisMoveMemory(&(UniCmdEccOpCal.aucBuffer[offset]), point_y, element_len);
	}

#ifdef RT_BIG_ENDIAN
	UniCmdEccOpCal.u2Tag 	= cpu2le16(UniCmdEccOpCal.u2Tag);
	UniCmdEccOpCal.u2Length = cpu2le16(UniCmdEccOpCal.u2Length);
#endif
	AndesAppendCmdMsg(msg, (char *)&UniCmdEccOpCal, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(ret = %d)\n", __func__, ret);
	return ret;
}

#ifdef MT_DFS_SUPPORT
INT32 UniCmdRddCtrl(
	struct _RTMP_ADAPTER *pAd,
	UCHAR ucDfsCtrl,
	UCHAR ucRddIdex,
	UCHAR ucRddRxSel,
	UCHAR ucSetVal)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	UNI_CMD_RDD_T	 UniCmdRDD;
	UNI_CMD_RDD_ON_OFF_CTRL_PARM_T UniCmdRDDOnOffCtrl;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT16 timeOut;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			" dispath CMD start\n");

	u4ComCmdSize = sizeof(UniCmdRDD);
	os_zero_mem(&UniCmdRDD, u4ComCmdSize);
	os_zero_mem(&UniCmdRDDOnOffCtrl, sizeof(UniCmdRDDOnOffCtrl));

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(UniCmdRDDOnOffCtrl);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/*extend the timeout limit of CAC_END because this command will do calibration*/
	if (ucDfsCtrl == CAC_END) {
		timeOut = 10000;
	} else
		timeOut = 0;

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_RDD_ON_OFF_CTRL);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, timeOut);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
	SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdRDD, (u4ComCmdSize));

	/* Step 4: Fill and append TLV parameters here */
	UniCmdRDDOnOffCtrl.u2Tag = UNI_CMD_RDD_ON_OFF_CTRL_PARM;
	UniCmdRDDOnOffCtrl.u2Length = (u4CmdNeedMaxBufSize - u4ComCmdSize);
#ifdef RT_BIG_ENDIAN
	UniCmdRDDOnOffCtrl.u2Tag	= cpu2le16(UniCmdRDDOnOffCtrl.u2Tag);
	UniCmdRDDOnOffCtrl.u2Length = cpu2le16(UniCmdRDDOnOffCtrl.u2Length);
#endif /* RT_BIG_ENDIAN */
	UniCmdRDDOnOffCtrl.u1DfsCtrl = ucDfsCtrl;
	UniCmdRDDOnOffCtrl.u1RddIdx = ucRddIdex;
	UniCmdRDDOnOffCtrl.u1RddRxSel = ucRddRxSel;
	UniCmdRDDOnOffCtrl.u1SetVal = ucSetVal;

	AndesAppendCmdMsg(msg, (char *)&UniCmdRDDOnOffCtrl, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	/* Step 5: Send out cmd */
	ret = chip_cmd_tx(pAd, msg);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "dispath CMD complete\n");
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#endif /* MT_DFS_SUPPORT */

INT32 UniCmdGetTsfTime(struct _RTMP_ADAPTER *pAd, UCHAR HwBssidIdx, TSF_RESULT_T *pTsfResult)
{
	struct cmd_msg *msg;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _CMD_ATTRIBUTE attr = {0};
	UNI_CMD_GET_MAC_INFO_T UniCmdGetMacInfo;
	UNI_CMD_MAC_INFO_TSF_T UniCmdMacInfoTsf;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;
	struct wifi_dev *wdev = NULL;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	wdev = wdev_search_by_omac_idx(pAd, HwBssidIdx);
	if (!wdev) {
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdGetMacInfo);
	os_zero_mem(&UniCmdGetMacInfo, u4ComCmdSize);
	os_zero_mem(&UniCmdMacInfoTsf, sizeof(UniCmdMacInfoTsf));

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(UniCmdMacInfoTsf);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_GET_MAC_INFO);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, (VOID *)pTsfResult);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_HANDLER(attr, UniEventMACInfoHandler);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	/* Nothing to do */
	AndesAppendCmdMsg(msg, (char *)&UniCmdGetMacInfo, u4ComCmdSize);

	/* Step 4: Fill TLV parameters here */
	UniCmdMacInfoTsf.u2Tag = UNI_CMD_MAC_INFO_TSF;
	UniCmdMacInfoTsf.u2Length = (u4CmdNeedMaxBufSize - u4ComCmdSize);
#ifdef RT_BIG_ENDIAN
	UniCmdMacInfoTsf.u2Tag = cpu2le16(UniCmdMacInfoTsf.u2Tag);
	UniCmdMacInfoTsf.u2Length = cpu2le16(UniCmdMacInfoTsf.u2Length);
#endif /* RT_BIG_ENDIAN */
	UniCmdMacInfoTsf.ucDbdcIdx = wdev->DevInfo.BandIdx;
	UniCmdMacInfoTsf.ucHwBssidIndex = HwBssidIdx;

	AndesAppendCmdMsg(msg, (char *)&UniCmdMacInfoTsf, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(ret = %d)\n", __func__, ret);
	return ret;
}

#ifdef CFG_SUPPORT_FALCON_TXCMD_DBG

static INT32 UniCmdSetTxCmdCommon(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_SET_TXCMD_DBG_CTRL_ENTRY_T pParam, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T pSetTxCmdDbgCtrl = (P_UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T)pHandle;

	pSetTxCmdDbgCtrl->u2Tag = pParam->u2Tag;
	pSetTxCmdDbgCtrl->u2Length = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T) + pParam->u4DataLen;
#ifdef RT_BIG_ENDIAN
	pSetTxCmdDbgCtrl->u2Tag = cpu2le16(pSetTxCmdDbgCtrl->u2Tag);
	pSetTxCmdDbgCtrl->u2Length = cpu2le16(pSetTxCmdDbgCtrl->u2Length);
#endif /* RT_BIG_ENDIAN */
	pSetTxCmdDbgCtrl->ucUserIndex = pParam->ucUserIndex;
	pSetTxCmdDbgCtrl->ucDlUlidx = pParam->ucDlUlidx;
	if (pParam->u4DataLen > 0)
		os_move_mem(pSetTxCmdDbgCtrl->aucBuffer, pParam->pData, pParam->u4DataLen);

	return Ret;
}

static UNI_CMD_TAG_HANDLE_T UniCmdTxCmdDbgTab[UNI_CMD_TXCMD_CTRL_MAX_NUM] = {
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_CTRL,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_CLEAR,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_SXN_GLOBAL,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_SXN_PROTECT,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_SXN_PROTECT_RUINFO,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_SXN_TXDATA,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_SXN_TXDATA_USER_INFO,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_SXN_TRIGDATA,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_SXN_TRIGDATA_USER_ACK_INFO,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_TF_TXD,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_TF_BASIC,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_TF_BASIC_USER,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_SXN_SW_FID,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_SXN_SW_FID_INFO,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_SW_FID_TXD,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_GET_TXCMD_DBG_STATUS,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_GET_TXCMD_DBG_SXN_GLOBAL,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_GET_TXCMD_DBG_SXN_PROTECT,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_GET_TXCMD_DBG_SXN_TXDATA,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_GET_TXCMD_DBG_SXN_TRIGDATA,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_GET_TXCMD_DBG_TF_TXD,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_GET_TXCMD_DBG_TF_BASIC,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_GET_TXCMD_DBG_SXN_SW_FID,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_GET_TXCMD_DBG_SW_FID_TXD,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
	{
		.u2CmdFeature = UNI_CMD_SET_TXCMD_DBG_SOP,
		.u4StructSize = sizeof(UNI_CMD_SET_TXCMD_DBG_CMD_CTRL_T),
		.pfHandler = UniCmdSetTxCmdCommon
	},
};

INT32 UniCmdTxCmdDbgCtrl(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_TXCMD_DBG_CTRL_PARAM_T pParamCtrl)
{
	struct cmd_msg          *msg = NULL;
	INT32                   Ret = NDIS_STATUS_SUCCESS;
	UINT32                  i = 0;
	UINT16                  u2TLVNumber = 0;
	PUCHAR					pTempBuf = NULL;
	PUCHAR					pNextHeadBuf = NULL;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4RealUseBufSize = 0;
	UINT32					u4SendBufSize = 0;
	UINT32					u4RemainingPayloadSize = 0;
	UINT32					u4ComCmdSize = 0;
	P_UNI_CMD_TXCMD_CTRL_T	pCmdTxCmdCtrl = NULL;
	RTMP_CHIP_CAP			*cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_TXCMD_CTRL_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < UNI_CMD_TXCMD_CTRL_MAX_NUM; i++) {
		if (pParamCtrl->TxCmdDbgCtrlValid[i])
			u4CmdNeedMaxBufSize += (UniCmdTxCmdDbgTab[i].u4StructSize + pParamCtrl->SetTxCmdDbgEntry[i].u4DataLen);
	}

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdTxCmdCtrl = (P_UNI_CMD_TXCMD_CTRL_T)pNextHeadBuf;
	/* Nothing to do */
	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Traverse all support features */
	for (i = 0; i < UNI_CMD_TXCMD_CTRL_MAX_NUM; i++) {
		if (pParamCtrl->TxCmdDbgCtrlValid[i]) {
			switch (i) {
			case UNI_CMD_SET_TXCMD_DBG_CTRL:
			case UNI_CMD_SET_TXCMD_DBG_CLEAR:
			case UNI_CMD_SET_TXCMD_DBG_SXN_GLOBAL:
			case UNI_CMD_SET_TXCMD_DBG_SXN_PROTECT:
			case UNI_CMD_SET_TXCMD_DBG_SXN_PROTECT_RUINFO:
			case UNI_CMD_SET_TXCMD_DBG_SXN_TXDATA:
			case UNI_CMD_SET_TXCMD_DBG_SXN_TXDATA_USER_INFO:
			case UNI_CMD_SET_TXCMD_DBG_SXN_TRIGDATA:
			case UNI_CMD_SET_TXCMD_DBG_SXN_TRIGDATA_USER_ACK_INFO:
			case UNI_CMD_SET_TXCMD_DBG_TF_TXD:
			case UNI_CMD_SET_TXCMD_DBG_TF_BASIC:
			case UNI_CMD_SET_TXCMD_DBG_TF_BASIC_USER:
			case UNI_CMD_SET_TXCMD_DBG_SXN_SW_FID:
			case UNI_CMD_SET_TXCMD_DBG_SXN_SW_FID_INFO:
			case UNI_CMD_SET_TXCMD_DBG_SW_FID_TXD:
			case UNI_CMD_GET_TXCMD_DBG_STATUS:
			case UNI_CMD_GET_TXCMD_DBG_SXN_GLOBAL:
			case UNI_CMD_GET_TXCMD_DBG_SXN_PROTECT:
			case UNI_CMD_GET_TXCMD_DBG_SXN_TXDATA:
			case UNI_CMD_GET_TXCMD_DBG_SXN_TRIGDATA:
			case UNI_CMD_GET_TXCMD_DBG_TF_TXD:
			case UNI_CMD_GET_TXCMD_DBG_TF_BASIC:
			case UNI_CMD_GET_TXCMD_DBG_SXN_SW_FID:
			case UNI_CMD_GET_TXCMD_DBG_SW_FID_TXD:
			case UNI_CMD_SET_TXCMD_DBG_SOP:
				if (UniCmdTxCmdDbgTab[i].pfHandler != NULL) {
					Ret = ((PFN_SET_TXCMD_DBG_CTRL_HANDLE)(UniCmdTxCmdDbgTab[i].pfHandler))(pAd, &pParamCtrl->SetTxCmdDbgEntry[i], pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += (UniCmdTxCmdDbgTab[i].u4StructSize + pParamCtrl->SetTxCmdDbgEntry[i].u4DataLen);
						u2TLVNumber++;
					}
				}
				break;

			default:
				Ret = NDIS_STATUS_SUCCESS;
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s(): The hanlder of tag (0x%08x) not support!\n", __func__, UniCmdTxCmdDbgTab[i].u2CmdFeature);
				break;
			}

			if (Ret != NDIS_STATUS_SUCCESS)
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"%s(): The hanlder of tag (0x%08x) return fail!\n", __func__, UniCmdTxCmdDbgTab[i].u2CmdFeature);
		}
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}
			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_TXCMD_CTRL);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if (!bNeedFrag || bLastFrag) {
				if (pParamCtrl->bQuery) {
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_WAIT_RETRY_RSP);
					SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
					SET_CMD_ATTR_RSP_HANDLER(attr, UniEventTxCmdDbgHandler);
				} else {
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
					SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
					SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
				}
			} else {
				if (pParamCtrl->bQuery)
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_RETRY);
				else
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			Ret = chip_cmd_tx(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}
#endif /* CFG_SUPPORT_FALCON_TXCMD_DBG */

#ifdef SCS_FW_OFFLOAD
static INT32 UniCmdSCSEventSendData(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2 pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2 pSCSEventSendData =
		(P_UNI_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2)pHandle;

	pSCSEventSendData->u2Tag = pParam->u2Tag;
	pSCSEventSendData->u2Length = sizeof(UNI_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2);
#ifdef RT_BIG_ENDIAN
	pSCSEventSendData->u2Tag = cpu2le16(pSCSEventSendData->u2Tag);
	pSCSEventSendData->u2Length = cpu2le16(pSCSEventSendData->u2Length);
#endif /* RT_BIG_ENDIAN */
	pSCSEventSendData->u2ActiveSTA = cpu2le16(pParam->u2ActiveSTA);
	pSCSEventSendData->u2eTput = cpu2le16(pParam->u2eTput);
	pSCSEventSendData->fgRxOnly = pParam->fgRxOnly;
	pSCSEventSendData->fgPDreset = pParam->fgPDreset;
	pSCSEventSendData->i1SCSMinRSSI = pParam->i1SCSMinRSSI;
	pSCSEventSendData->ucReserved = pParam->ucReserved;

	return Ret;
}

static INT32 UniCmdSCSGetGloAddrCtrl(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_SCS_GET_GLO_ADDR_CTRL pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_SCS_GET_GLO_ADDR_CTRL pSCSGetGloAddrCtrl =
		(P_UNI_CMD_SCS_GET_GLO_ADDR_CTRL)pHandle;

	pSCSGetGloAddrCtrl->u2Tag = pParam->u2Tag;
	pSCSGetGloAddrCtrl->u2Length = sizeof(UNI_CMD_SCS_GET_GLO_ADDR_CTRL);
#ifdef RT_BIG_ENDIAN
	pSCSGetGloAddrCtrl->u2Tag = cpu2le16(pSCSGetGloAddrCtrl->u2Tag);
	pSCSGetGloAddrCtrl->u2Length = cpu2le16(pSCSGetGloAddrCtrl->u2Length);
#endif /* RT_BIG_ENDIAN */

	return Ret;
}

static INT32 UniCmdSCSEventSetPDThrRange(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_SET_SCS_PD_THR_RANGE pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_SET_SCS_PD_THR_RANGE pSCSSetPDThrRange =
		(P_UNI_CMD_SET_SCS_PD_THR_RANGE)pHandle;

	pSCSSetPDThrRange->u2Tag = pParam->u2Tag;
	pSCSSetPDThrRange->u2Length = sizeof(UNI_CMD_SET_SCS_PD_THR_RANGE);
#ifdef RT_BIG_ENDIAN
	pSCSSetPDThrRange->u2Tag = cpu2le16(pSCSSetPDThrRange->u2Tag);
	pSCSSetPDThrRange->u2Length = cpu2le16(pSCSSetPDThrRange->u2Length);
#endif /* RT_BIG_ENDIAN */
	pSCSSetPDThrRange->u2CckPdThrMax = cpu2le16(pParam->u2CckPdThrMax);
	pSCSSetPDThrRange->u2OfdmPdThrMax = cpu2le16(pParam->u2OfdmPdThrMax);
	pSCSSetPDThrRange->u2CckPdThrMin = cpu2le16(pParam->u2CckPdThrMin);
	pSCSSetPDThrRange->u2OfdmPdThrMin = cpu2le16(pParam->u2OfdmPdThrMin);

	return Ret;
}

static INT32 UniCmdSCSSmartCarrierEnable(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_SMART_CARRIER_ENABLE pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_SMART_CARRIER_ENABLE pSCSEnable =
		(P_UNI_CMD_SMART_CARRIER_ENABLE)pHandle;

	pSCSEnable->u2Tag = pParam->u2Tag;
	pSCSEnable->u2Length = sizeof(UNI_CMD_SMART_CARRIER_ENABLE);
#ifdef RT_BIG_ENDIAN
	pSCSEnable->u2Tag = cpu2le16(pSCSEnable->u2Tag);
	pSCSEnable->u2Length = cpu2le16(pSCSEnable->u2Length);
#endif /* RT_BIG_ENDIAN */
	pSCSEnable->u1SCSEnable = pParam->u1SCSEnable;

	return Ret;
}

static UNI_CMD_TAG_HANDLE_T UniCmdSCSTab[UNI_CMD_SCS_MAX_EVENT] = {
	{
		.u2CmdFeature = UNI_CMD_SCS_EVENT_SEND_DATA,
		.u4StructSize = sizeof(UNI_CMD_SMART_CARRIER_SENSE_CTRL_DATA_FW_VER2),
		.pfHandler = UniCmdSCSEventSendData
	},
	{
		.u2CmdFeature = UNI_CMD_SCS_EVENT_GET_GLO_ADDR,
		.u4StructSize = sizeof(UNI_CMD_SCS_GET_GLO_ADDR_CTRL),
		.pfHandler = UniCmdSCSGetGloAddrCtrl
	},
	{
		.u2CmdFeature = UNI_CMD_SCS_EVENT_SET_PD_THR_RANGE,
		.u4StructSize = sizeof(UNI_CMD_SET_SCS_PD_THR_RANGE),
		.pfHandler = UniCmdSCSEventSetPDThrRange
	},
	{
		.u2CmdFeature = UNI_CMD_SCS_EVENT_SCS_ENABLE,
		.u4StructSize = sizeof(UNI_CMD_SMART_CARRIER_ENABLE),
		.pfHandler = UniCmdSCSSmartCarrierEnable
	},
};

INT32 UniCmdSCS(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_SCS_PARAM_T pParamCtrl)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = NDIS_STATUS_SUCCESS;
	UINT32 i = 0;
	UINT16 u2TLVNumber = 0;
	PUCHAR pTempBuf = NULL;
	PUCHAR pNextHeadBuf = NULL;
	UINT32 u4CmdNeedMaxBufSize = 0;
	UINT32 u4RealUseBufSize = 0;
	UINT32 u4SendBufSize = 0;
	UINT32 u4RemainingPayloadSize = 0;
	UINT32 u4ComCmdSize = 0;
	P_UNI_CMD_SCS_T	pCmdSCS = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_BAND_CONFIG_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < UNI_CMD_SCS_MAX_EVENT; i++) {
		if (pParamCtrl->SCSTagValid[i])
			u4CmdNeedMaxBufSize += UniCmdSCSTab[i].u4StructSize;
	}

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdSCS = (P_UNI_CMD_SCS_T)pNextHeadBuf;
	pCmdSCS->ubandid = pParamCtrl->ucDbdcIdx;

	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Traverse all support features */
	for (i = 0; i < UNI_CMD_SCS_MAX_EVENT; i++) {
		if (pParamCtrl->SCSTagValid[i]) {
			switch (i) {
			case UNI_CMD_SCS_EVENT_SEND_DATA:
				if (UniCmdSCSTab[i].pfHandler != NULL) {
					Ret = ((PFN_SCS_SEND_DATA_HANDLE)(UniCmdSCSTab[i].pfHandler))(pAd, &pParamCtrl->SCSSendData, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdSCSTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_SCS_EVENT_GET_GLO_ADDR:
				if (UniCmdSCSTab[i].pfHandler != NULL) {
					Ret = ((PFN_SCS_GET_GLO_ADDR_CTRL_HANDLE)(UniCmdSCSTab[i].pfHandler))(pAd, &pParamCtrl->SCSGetGloAddrCtrl, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdSCSTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_SCS_EVENT_SET_PD_THR_RANGE:
				if (UniCmdSCSTab[i].pfHandler != NULL) {
					Ret = ((PFN_SCS_PD_THR_RANGE_HANDLE)(UniCmdSCSTab[i].pfHandler))(pAd, &pParamCtrl->SCSPDThrRange, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdSCSTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_SCS_EVENT_SCS_ENABLE:
				if (UniCmdSCSTab[i].pfHandler != NULL) {
					Ret = ((PFN_SCS_SMART_CARRIER_ENABLE_HANDLE)(UniCmdSCSTab[i].pfHandler))(pAd, &pParamCtrl->SCSEnable, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdSCSTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			default:
				Ret = NDIS_STATUS_SUCCESS;
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s(): The hanlder of tag (0x%08x) not support!\n", __func__, UniCmdSCSTab[i].u2CmdFeature);
				break;
			}

			if (Ret != NDIS_STATUS_SUCCESS)
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s(): The hanlder of tag (0x%08x) return fail!\n", __func__, UniCmdSCSTab[i].u2CmdFeature);
		}
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			" TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SCS);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if (!bNeedFrag || bLastFrag) {
				if (pParamCtrl->bQuery) {
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_WAIT_RETRY_RSP);
					SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
					SET_CMD_ATTR_RSP_HANDLER(attr, UniEventSCSHandler);
				} else {
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
					SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
					SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
				}
			} else {
				if (pParamCtrl->bQuery)
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_RETRY);
				else
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			Ret = chip_cmd_tx(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}
#endif /* SCS_FW_OFFLOAD */

#ifdef WIFI_MODULE_DVT
INT32 UniCmdMDVT(struct _RTMP_ADAPTER *pAd, UINT16 u2ModuleId, UINT16 u2CaseId)
{
	struct cmd_msg *msg;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _CMD_ATTRIBUTE attr = {0};
	UNI_CMD_DVT_CONFIG_T UniCmdDVT;
	UNI_CMD_MDVT_SET_PARA_T UniCmdMDVTSetPara;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdDVT);
	os_zero_mem(&UniCmdDVT, u4ComCmdSize);
	os_zero_mem(&UniCmdMDVTSetPara, sizeof(UniCmdMDVTSetPara));

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(UniCmdMDVTSetPara);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_DVT);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
	SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	UniCmdDVT.ucTestType = UNI_CMD_MODULE_DVT;
	AndesAppendCmdMsg(msg, (char *)&UniCmdDVT, u4ComCmdSize);

	/* Step 4: Fill TLV parameters here */
	UniCmdMDVTSetPara.u2Tag = UNI_CMD_MDVT_SET_PARA;
	UniCmdMDVTSetPara.u2Length = (u4CmdNeedMaxBufSize - u4ComCmdSize);
#ifdef RT_BIG_ENDIAN
	UniCmdMDVTSetPara.u2Tag = cpu2le16(UniCmdMDVTSetPara.u2Tag);
	UniCmdMDVTSetPara.u2Length = cpu2le16(UniCmdMDVTSetPara.u2Length);
#endif /* RT_BIG_ENDIAN */
	UniCmdMDVTSetPara.u2ModuleId = cpu2le16(u2ModuleId);
	UniCmdMDVTSetPara.u2CaseId = cpu2le16(u2CaseId);

	AndesAppendCmdMsg(msg, (char *)&UniCmdMDVTSetPara, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#endif /* WIFI_MODULE_DVT */

/*****************************************
 *	UNI_CMD_ID_MIB (0x22)
 *****************************************/
INT32 UniCmdMib(struct _RTMP_ADAPTER *pAd, UCHAR ChIdx, RTMP_MIB_PAIR *RegPair, UINT32 Num)
{
	struct cmd_msg 			*msg;
	P_UNI_CMD_MIB_T 		pUniCmdMib = NULL;
	P_UNI_CMD_MIB_DATA_T 	pUniCmdMibData = NULL;
	INT32                   Ret = NDIS_STATUS_SUCCESS;
	UINT16                  u2TLVNumber = 0;
	PUCHAR					pTempBuf = NULL;
	PUCHAR					pNextHeadBuf = NULL;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4RealUseBufSize = 0;
	UINT32					u4SendBufSize = 0;
	UINT32					u4RemainingPayloadSize = 0;
	UINT32					u4ComCmdSize = 0;
	UINT32 					Index;
	RTMP_CHIP_CAP			*cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_MIB_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	u4CmdNeedMaxBufSize += (sizeof(UNI_CMD_MIB_DATA_T) * Num);

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pUniCmdMib = (P_UNI_CMD_MIB_T)pNextHeadBuf;
	pUniCmdMib->ucBand = ChIdx;
	pNextHeadBuf += u4ComCmdSize;
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, " ucBand=%d\n", pUniCmdMib->ucBand);

	/* Step 4: Traverse all regs */
	for (Index = 0; Index < Num; Index++) {
		pUniCmdMibData = (P_UNI_CMD_MIB_DATA_T)pNextHeadBuf;
		pUniCmdMibData->u2Tag = UNI_CMD_MIB_DATA;
		pUniCmdMibData->u2Length = sizeof(UNI_CMD_MIB_DATA_T);
#ifdef RT_BIG_ENDIAN
		pUniCmdMibData->u2Tag = cpu2le16(pUniCmdMibData->u2Tag);
		pUniCmdMibData->u2Length = cpu2le16(pUniCmdMibData->u2Length);
#endif /* RT_BIG_ENDIAN */
		pUniCmdMibData->u4Counter = cpu2le32(RegPair[Index].Counter);
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, " Tag=%d, Length=%d, Counter=%d\n",
				pUniCmdMibData->u2Tag, pUniCmdMibData->u2Length, pUniCmdMibData->u4Counter);
		u2TLVNumber++;
		pNextHeadBuf += sizeof(UNI_CMD_MIB_DATA_T);
	}
	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);


	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}
			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_MIB);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, (VOID *)RegPair);
			if (!bNeedFrag || bLastFrag) {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_WAIT_RETRY_RSP);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_HANDLER(attr, UniEventMibHandler);
			} else {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_RETRY);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			Ret = chip_cmd_tx(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

#ifdef CONFIG_HW_HAL_OFFLOAD
/*****************************************
 *	UNI_CMD_ID_SNIFFER_MODE (0x24) // Sniffer Mode
 *****************************************/
INT32 UniCmdSetSnifferMode(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_SNIFFER_MODE_T param)
{
	struct cmd_msg *msg;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct _CMD_ATTRIBUTE attr = {0};
	UNI_CMD_SNIFFER_MODE_T UniCmdSnifferMode;
	UNI_CMD_SNIFFER_MODE_ENABLE_T UniCmdSnifferModeEnable;
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(UniCmdSnifferMode);
	os_zero_mem(&UniCmdSnifferMode, u4ComCmdSize);
	os_zero_mem(&UniCmdSnifferModeEnable, sizeof(UniCmdSnifferModeEnable));

	/* Step 1: Count maximum buffer size from per TLV */
	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(UniCmdSnifferModeEnable);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_SNIFFER_MODE);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
	SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	UniCmdSnifferMode.ucBandIdx = param.ucDbdcIdx;
	AndesAppendCmdMsg(msg, (char *)&UniCmdSnifferMode, u4ComCmdSize);

	/* Step 4: Fill TLV parameters here */
	UniCmdSnifferModeEnable.u2Tag = UNI_CMD_SNIFFER_MODE_ENABLE;
	UniCmdSnifferModeEnable.u2Length = (u4CmdNeedMaxBufSize - u4ComCmdSize);
#ifdef RT_BIG_ENDIAN
	UniCmdSnifferModeEnable.u2Tag = cpu2le16(UniCmdSnifferModeEnable.u2Tag);
	UniCmdSnifferModeEnable.u2Length = cpu2le16(UniCmdSnifferModeEnable.u2Length);
#endif /* RT_BIG_ENDIAN */
	UniCmdSnifferModeEnable.ucSNEnable = param.ucSnifferEn;

	AndesAppendCmdMsg(msg, (char *)&UniCmdSnifferModeEnable, (u4CmdNeedMaxBufSize - u4ComCmdSize));

	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(ret = %d)\n", __func__, ret);
	return ret;
}
#endif /* CONFIG_HW_HAL_OFFLOAD */

#ifdef WIFI_GPIO_CTRL
static INT32 UniCmdGPIOEnable(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_GPIO_ENABLE_T pParam, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_GPIO_ENABLE_T pGpioEnable = (P_UNI_CMD_GPIO_ENABLE_T)pHandle;

	pGpioEnable->u2Tag = UNI_CMD_GPIO_ENABLE;
	pGpioEnable->u2Length = sizeof(UNI_CMD_GPIO_ENABLE_T);
#ifdef RT_BIG_ENDIAN
	pGpioEnable->u2Tag = cpu2le16(pGpioEnable->u2Tag);
	pGpioEnable->u2Length = cpu2le16(pGpioEnable->u2Length);
#endif /* RT_BIG_ENDIAN */
	pGpioEnable->fgEnable = pParam->fgEnable;

	return Ret;
}

static INT32 UniCmdGPIOSetValue(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_GPIO_SET_VALUE_T pParam, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_GPIO_SET_VALUE_T pGpioSetValue = (P_UNI_CMD_GPIO_SET_VALUE_T)pHandle;

	pGpioSetValue->u2Tag = UNI_CMD_GPIO_SET_VALUE;
	pGpioSetValue->u2Length = sizeof(UNI_CMD_GPIO_SET_VALUE_T);
#ifdef RT_BIG_ENDIAN
	pGpioSetValue->u2Tag = cpu2le16(pGpioSetValue->u2Tag);
	pGpioSetValue->u2Length = cpu2le16(pGpioSetValue->u2Length);
#endif /* RT_BIG_ENDIAN */
	pGpioSetValue->ucGpioValue = pParam->ucGpioValue;

	return Ret;
}

static UNI_CMD_TAG_HANDLE_T UniCmdGPIOTab[UNI_CMD_GPIO_TAG_MAX_NUM] = {
	{
		.u2CmdFeature = UNI_CMD_GPIO_ENABLE,
		.u4StructSize = sizeof(UNI_CMD_GPIO_ENABLE_T),
		.pfHandler = UniCmdGPIOEnable
	},
	{
		.u2CmdFeature = UNI_CMD_GPIO_SET_VALUE,
		.u4StructSize = sizeof(UNI_CMD_GPIO_SET_VALUE_T),
		.pfHandler = UniCmdGPIOSetValue
	},
};

INT32 UniCmdGPIO(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_GPIO_CFG_PARAM_T pParamCtrl)
{
	struct cmd_msg          *msg = NULL;
	INT32                   Ret = NDIS_STATUS_SUCCESS;
	UINT32                  i = 0;
	UINT16                  u2TLVNumber = 0;
	PUCHAR					pTempBuf = NULL;
	PUCHAR					pNextHeadBuf = NULL;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4RealUseBufSize = 0;
	UINT32					u4SendBufSize = 0;
	UINT32					u4RemainingPayloadSize = 0;
	UINT32					u4ComCmdSize = 0;
	P_UNI_CMD_GPIO_CONFIG_T	pCmdGpioCfg = NULL;
	RTMP_CHIP_CAP			*cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_GPIO_CONFIG_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < UNI_CMD_GPIO_TAG_MAX_NUM; i++) {
		if (pParamCtrl->GpioCfgValid[i])
			u4CmdNeedMaxBufSize += UniCmdGPIOTab[i].u4StructSize;
	}

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdGpioCfg = (P_UNI_CMD_GPIO_CONFIG_T)pNextHeadBuf;
	pCmdGpioCfg->ucGpioIdx = pParamCtrl->ucGpioIdx;
	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Traverse all support features */
	for (i = 0; i < UNI_CMD_GPIO_TAG_MAX_NUM; i++) {
		if (pParamCtrl->GpioCfgValid[i]) {
			switch (i) {
			case UNI_CMD_GPIO_ENABLE:
				if (UniCmdGPIOTab[i].pfHandler != NULL) {
					Ret = ((PFN_GPIO_ENABLE_HANDLE)(UniCmdGPIOTab[i].pfHandler))(pAd, &pParamCtrl->GpioEnable, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdGPIOTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_GPIO_SET_VALUE:
				if (UniCmdWsysCfgTab[i].pfHandler != NULL) {
					Ret = ((PFN_GPIO_SET_VALUE_HANDLE)(UniCmdGPIOTab[i].pfHandler))(pAd, &pParamCtrl->GpioSetValue, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdGPIOTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			default:
				Ret = NDIS_STATUS_SUCCESS;
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s(): The hanlder of tag (0x%08x) not support!\n", __func__, UniCmdGPIOTab[i].u2CmdFeature);
				break;
			}

			if (Ret != NDIS_STATUS_SUCCESS)
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						"%s(): The hanlder of tag (0x%08x) return fail!\n", __func__, UniCmdGPIOTab[i].u2CmdFeature);
		}
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}
			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_GPIO);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if (!bNeedFrag || bLastFrag) {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
				SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
			} else {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			Ret = chip_cmd_tx(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}
#endif /* WIFI_GPIO_CTRL */

static INT32 UniCmdMuruBsrpCtrl(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MURU_BSRP_CTRL_T *pCmdBsrpCtrl = (struct UNI_CMD_MURU_BSRP_CTRL_T *)pHandle;
	PCHAR pch = NULL;

	pCmdBsrpCtrl->u2Tag = UNI_CMD_MURU_BSRP_CTRL;
	pCmdBsrpCtrl->u2Length = sizeof(*pCmdBsrpCtrl);

	pch = strsep(&arg, "-");
	if (pch != NULL) {
		if (os_str_tol(pch, 0, 10))
			pCmdBsrpCtrl->fgExtCmdBsrp = 1;
		else
			pCmdBsrpCtrl->fgExtCmdBsrp = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(fgExtCmdBsrp = %u\n",
			__func__, pCmdBsrpCtrl->fgExtCmdBsrp));
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL) {
		pCmdBsrpCtrl->u1TriggerFlow = os_str_tol(pch, 0, 10);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(ucTriggerFlow = %u\n",
			__func__, pCmdBsrpCtrl->u1TriggerFlow));
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	if (pch != NULL) {
		pCmdBsrpCtrl->u2BsrpInterval = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
		pCmdBsrpCtrl->u2BsrpInterval = cpu2le16(pCmdBsrpCtrl->u2BsrpInterval);
#endif

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(ucBsrpInterval = %u\n",
			__func__, pCmdBsrpCtrl->u2BsrpInterval));
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");

	if (pch != NULL) {
		pCmdBsrpCtrl->u2BsrpRuAlloc = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
		pCmdBsrpCtrl->u2BsrpRuAlloc = cpu2le16(pCmdBsrpCtrl->u2BsrpRuAlloc);
#endif

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(ucBsrpRuAlloc = %u\n",
			__func__, pCmdBsrpCtrl->u2BsrpRuAlloc));
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");

	if (pch != NULL) {
		pCmdBsrpCtrl->u4TriggerType = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
		pCmdBsrpCtrl->u4TriggerType = cpu2le32(pCmdBsrpCtrl->u4TriggerType);
#endif

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(u4TriggerType = %u\n", __func__, pCmdBsrpCtrl->u4TriggerType));
	} else {
		Ret = 0;
		goto error;
	}

error:

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static INT32 UniCmdMuruSetArbOpMode(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	VOID * pHandle
)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MURU_SET_ARB_OP_MODE_T *pCmdMuruArbOpModeCtrl = (struct UNI_CMD_MURU_SET_ARB_OP_MODE_T *)pHandle;
	UINT8 *OpMode = (UINT8 *)arg;

	pCmdMuruArbOpModeCtrl->u1OpMode = *OpMode;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: %d\n", __func__, pCmdMuruArbOpModeCtrl->u1OpMode));

	pCmdMuruArbOpModeCtrl->u2Tag = UNI_CMD_MURU_SET_ARB_OP_MODE;
	pCmdMuruArbOpModeCtrl->u2Length = sizeof(*pCmdMuruArbOpModeCtrl);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static INT32 UniCmdMuruSuTxCtrl(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	VOID * pHandle
)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MURU_SET_SUTX_T *pCmdMuruSuTxCtrl = (struct UNI_CMD_MURU_SET_SUTX_T *)pHandle;
	PCHAR pch = NULL;

	pch = strsep(&arg, "-");

	if (pch != NULL)
		pCmdMuruSuTxCtrl->u1ForceSuTx = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: %d\n", __func__, pCmdMuruSuTxCtrl->u1ForceSuTx));

	pCmdMuruSuTxCtrl->u2Tag = UNI_CMD_MURU_SUTX_CTRL;
	pCmdMuruSuTxCtrl->u2Length = sizeof(*pCmdMuruSuTxCtrl);

error:

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static INT32 UniCmdMuruSetFixedRate(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	VOID *pHandle
)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MURU_FIXED_RATE_CTRL_T *pCmdFixedRateCtrl = (struct UNI_CMD_MURU_FIXED_RATE_CTRL_T *)pHandle;

	if (arg != NULL) {
		pCmdFixedRateCtrl->u2Value = os_str_tol(arg, 0, 10);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(u2Value = %u\n", __func__, pCmdFixedRateCtrl->u2Value));
	} else {
		Ret = 0;
		goto error;
	}

	pCmdFixedRateCtrl->u2Tag = UNI_CMD_MURU_FIXED_RATE_CTRL;
	pCmdFixedRateCtrl->u2Length = sizeof(*pCmdFixedRateCtrl);

error:

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static INT32 UniCmdMuruSetFixedGroupRate(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	VOID *pHandle
)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	UINT8 ucNNS_MCS = 0;
	PCHAR pch = NULL;
	UINT8 MaxMcs = 0;
	struct UNI_CMD_MURU_FIXED_GRP_RATE_CTRL_T *pCmdFixedGrpRateCtrl = (struct UNI_CMD_MURU_FIXED_GRP_RATE_CTRL_T *)pHandle;

	pCmdFixedGrpRateCtrl->u2Tag = UNI_CMD_MURU_FIXED_GROUP_RATE_CTRL;
	pCmdFixedGrpRateCtrl->u2Length = sizeof(*pCmdFixedGrpRateCtrl);
	pCmdFixedGrpRateCtrl->u1CmdVersion = UNI_CMD_MURU_VER_HE;;

	MaxMcs = UNI_MAX_MCS_SUPPORT_HE;

	pch = strsep(&arg, "-");
	/*Get NumUsr*/
	if (pch != NULL) {
		pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1NumUser = os_str_tol(pch, 0, 10) - 1;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(u1NumUser = %u\n", __func__, pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1NumUser));
	} else {
		Ret = 0;
		goto error;
	}

	pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1RuAllocExt = 0;

	pch = strsep(&arg, "-");
	/*Get Rualloc*/
	if (pch != NULL) {
		pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1RuAlloc = os_str_tol(pch, 0, 10);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(u1RuAlloc = %u\n", __func__, pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1RuAlloc));
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	/*Get GuardInterval*/
	if (pch != NULL) {
		pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1GI = os_str_tol(pch, 0, 10);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(u1GI = %u\n", __func__, pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1GI));
	} else {
		Ret = 0;
		goto error;
	}

	pch = strsep(&arg, "-");
	/*Get Capability*/
	if (pch != NULL) {
		pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Capability = os_str_tol(pch, 0, 10);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(u1Capability = %u\n", __func__, pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Capability));
	} else {
		Ret = 0;
		goto error;
	}

	/*Get DL /UL*/
	pch = strsep(&arg, "-");
	if (pch != NULL)
		pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}

	if (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1NumUser == 0) {
		pch = strsep(&arg, "-");

		if (pch != NULL)
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u2WlidUser0 = os_str_tol(pch, 0, 10);
		else {
			Ret = 0;
			goto error;
		}

		pch = strsep(&arg, "-");

		if (pch != NULL) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Nss0 = (ucNNS_MCS > MaxMcs);
			if (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul != 1)
				pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1DlMcsUser0 = (ucNNS_MCS > MaxMcs) ? (ucNNS_MCS - MaxMcs) : ucNNS_MCS;
			else
				pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1UlMcsUser0 = (ucNNS_MCS > MaxMcs) ? (ucNNS_MCS - MaxMcs) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		if (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul == 2) {
			UINT_8 ucNNS_MCS = 0;

			pch = strsep(&arg, "");
			if (pch == NULL) {
				Ret = 0;
				goto error;
			}
			ucNNS_MCS = os_str_tol(pch, 0, 10);
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Nss0 = (ucNNS_MCS > MaxMcs);
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1UlMcsUser0 = (ucNNS_MCS > MaxMcs) ? (ucNNS_MCS - MaxMcs) : ucNNS_MCS;
		}
	}

	if (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1NumUser >= 1) {
		pch = strsep(&arg, "-");

		if (pch != NULL) {
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u2WlidUser0 = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u2WlidUser0 = cpu2le16(pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u2WlidUser0);
#endif
		} else {
			Ret = 0;
			goto error;
		}

		pch = strsep(&arg, "-");

		if (pch != NULL) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Nss0 = (ucNNS_MCS > MaxMcs);
			if (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul != 1)
				pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1DlMcsUser0 = (ucNNS_MCS > MaxMcs) ? (ucNNS_MCS - MaxMcs) : ucNNS_MCS;
			else
				pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1UlMcsUser0 = (ucNNS_MCS > MaxMcs) ? (ucNNS_MCS - MaxMcs) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		pch = strsep(&arg, "-");

		if ((pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul == 2) && (pch != NULL)) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Nss0 = (ucNNS_MCS > MaxMcs);
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1UlMcsUser0 = (ucNNS_MCS > MaxMcs) ? (ucNNS_MCS - MaxMcs) : ucNNS_MCS;
			pch = strsep(&arg, "-");
			} else if (pch == NULL) {
				Ret = 0;
				goto error;
			}

		if (pch != NULL) {
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u2WlidUser1 = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u2WlidUser1 = cpu2le16(pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u2WlidUser1);
#endif
		} else {
			Ret = 0;
			goto error;
		}

		if ((pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1NumUser == 1) && (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul != 2))
			pch = strsep(&arg, "");
		else
			pch = strsep(&arg, "-");

		if (pch != NULL) {
			ucNNS_MCS = os_str_tol(pch, 0, 10);
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Nss1 = (ucNNS_MCS > MaxMcs);
			if (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul != 1)
				pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1DlMcsUser1 = (ucNNS_MCS > MaxMcs) ? (ucNNS_MCS - MaxMcs) : ucNNS_MCS;
			else
				pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1UlMcsUser1 = (ucNNS_MCS > MaxMcs) ? (ucNNS_MCS - MaxMcs) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		if ((pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1NumUser == 1) && (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul == 2))
			pch = strsep(&arg, "");
		else if ((pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1NumUser != 1) && (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul == 2))
			pch = strsep(&arg, "-");

		if ((pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul == 2) && (pch != NULL)) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Nss1 = (ucNNS_MCS > MaxMcs);
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1UlMcsUser1 = (ucNNS_MCS > MaxMcs) ? (ucNNS_MCS - MaxMcs) : ucNNS_MCS;
			} else if (pch == NULL) {
				Ret = 0;
				goto error;
			}
	}

	if (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1NumUser >= 2) {
		pch = strsep(&arg, "-");

		if (pch != NULL) {
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u2WlidUser2 = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u2WlidUser2 = cpu2le16(pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u2WlidUser2);
#endif
		} else {
			Ret = 0;
			goto error;
		}

		if ((pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1NumUser == 2) && (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul != 2))
			pch = strsep(&arg, "");
		else
			pch = strsep(&arg, "-");

		if (pch != NULL) {
			ucNNS_MCS = os_str_tol(pch, 0, 10);
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Nss2 = (ucNNS_MCS > MaxMcs);
			if (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul != 1)
				pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1DlMcsUser2 = (ucNNS_MCS > MaxMcs) ? (ucNNS_MCS - MaxMcs) : ucNNS_MCS;
			else
				pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1UlMcsUser2 = (ucNNS_MCS > MaxMcs) ? (ucNNS_MCS - MaxMcs) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		if ((pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1NumUser == 2) && (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul == 2))
			pch = strsep(&arg, "");
		else if ((pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1NumUser != 2) && (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul == 2))
			pch = strsep(&arg, "-");

		if ((pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul == 2) && (pch != NULL)) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Nss2 = (ucNNS_MCS > MaxMcs);
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1UlMcsUser2 = (ucNNS_MCS > MaxMcs) ? (ucNNS_MCS - MaxMcs) : ucNNS_MCS;
		} else if (pch == NULL) {
				Ret = 0;
				goto error;
		}
	}

	if (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1NumUser >= 3) {
		pch = strsep(&arg, "-");

		if (pch != NULL) {
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u2WlidUser3 = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u2WlidUser3 = cpu2le16(pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u2WlidUser3);
#endif
		} else {
			Ret = 0;
			goto error;
		}

		if (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul != 2)
			pch = strsep(&arg, "");
		else
			pch = strsep(&arg, "-");

		if (pch != NULL) {
			ucNNS_MCS = os_str_tol(pch, 0, 10);
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Nss3 = (ucNNS_MCS > MaxMcs);
			if (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul != 1)
				pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1DlMcsUser3 = (ucNNS_MCS > MaxMcs) ? (ucNNS_MCS - MaxMcs) : ucNNS_MCS;
			else
				pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1UlMcsUser3 = (ucNNS_MCS > MaxMcs) ? (ucNNS_MCS - MaxMcs) : ucNNS_MCS;
		} else {
			Ret = 0;
			goto error;
		}

		if (pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul == 2)
			pch = strsep(&arg, "");

		if ((pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Dl_Ul == 2) && (pch != NULL)) {
			UINT_8 ucNNS_MCS = 0;

			ucNNS_MCS = os_str_tol(pch, 0, 10);
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1Nss3 = (ucNNS_MCS > MaxMcs);
			pCmdFixedGrpRateCtrl->rMuruSetGrpTblEntry.u1UlMcsUser3 = (ucNNS_MCS > MaxMcs) ? (ucNNS_MCS - MaxMcs) : ucNNS_MCS;
		} else if (pch == NULL) {
				Ret = 0;
				goto error;
		}
	}

error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static INT32 UniCmdMuruSetDbgInfo(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	VOID * pHandle
)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MURU_SET_DBG_INFO *pCmdMuruSetDbgInfo = (struct UNI_CMD_MURU_SET_DBG_INFO *)pHandle;
	PCHAR pch = NULL;

	pch = strsep(&arg, "-");

	if (pch != NULL)
		pCmdMuruSetDbgInfo->u2Item = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("invalid format, get_ulru_status=[item]-[value]\n"));
		goto error;
	}

	pch = strsep(&arg, "-");

	if (pch != NULL)
		pCmdMuruSetDbgInfo->u4Value = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("invalid format, get_ulru_status=[item]-[value]\n"));
		goto error;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: item=%u value=%u\n", __func__,
		pCmdMuruSetDbgInfo->u2Item, pCmdMuruSetDbgInfo->u4Value));

	pCmdMuruSetDbgInfo->u2Tag = UNI_CMD_MURU_DBG_INFO;
	pCmdMuruSetDbgInfo->u2Length = sizeof(*pCmdMuruSetDbgInfo);

error:
	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static UINT32 gu4UniMuruManCfgUsrListDl;
static UINT32 gu4UniMuruManCfgUsrListUl;

static struct UNI_MURU_MANUAL_CONFIG_T  grUniMuruManCfgInf;

INT32 uni_hqa_muru_parse_cmd_param_dltx(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *type,
	RTMP_STRING *val,
	struct UNI_MURU_MANUAL_CONFIG_T *pMuruManCfg
)
{
	INT32    status = FALSE;
#if defined(CONFIG_WLAN_SERVICE)
	UINT8  user_idx = 0;
	UINT8  tmpValue = 0;
	INT32  loop_cnt, loop_idx, ru_idx, c26_idx;
	PCHAR  pch = NULL;

	UINT32 *pu4UsrList;
	UINT32 *pu4ManCfgBmpDl;
	UINT32 *pu4ManCfgBmpCmm;
	struct wifi_dev *wdev = NULL;

	struct UNI_MURU_DL_MANUAL_CONFIG *pCfgDl = NULL;
	struct UNI_MURU_CMM_MANUAL_CONFIG *pCfgCmm = NULL;

	pu4UsrList = &gu4UniMuruManCfgUsrListDl;
	pu4ManCfgBmpDl = &pMuruManCfg->u4ManCfgBmpDl;
	pu4ManCfgBmpCmm = &pMuruManCfg->u4ManCfgBmpCmm;
	pCfgDl = &pMuruManCfg->rCfgDl;
	pCfgCmm = &pMuruManCfg->rCfgCmm;

	/* comm_cfg:[Band]:[BW]:[GI]:[LTF]:[total User#]:[VHT/HE]:[SPE] */
	if (strcmp("comm_cfg", type) == 0) {

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgCmm->u1Band = (uint8_t)os_str_tol(pch, 0, 10);

			if (pCfgCmm->u1Band < TESTMODE_BAND_NUM) {
				/* set TXCMD mode */
				wdev = &pAd->ate_wdev[pCfgCmm->u1Band][1];
				pCfgCmm->u1WmmSet = HcGetWmmIdx(pAd, wdev);
			} else {
				status = FALSE;
				goto error;
			}

			*pu4ManCfgBmpCmm |= (MURU_FIXED_CMM_BAND | MURU_FIXED_CMM_WMM_SET);
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->u1Bw = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->u1GI = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->u1Ltf = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgDl->u1UserCnt = (pAd->CommonCfg.HE_OfdmaUserNum) ?
									(uint8_t)(pAd->CommonCfg.HE_OfdmaUserNum) :
									(uint8_t)os_str_tol(pch, 0, 10);

			pCfgCmm->u1PpduFmt |= MURU_PPDU_HE_MU;
			pCfgCmm->u1SchType |= MURU_OFDMA_SCH_TYPE_DL;
			*pu4ManCfgBmpCmm |= (MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			if (strcmp("VHT", pch) == 0)
				pCfgDl->u1TxMode = TX_MODE_VHT;
			else if (strcmp("HE", pch) == 0)
				pCfgDl->u1TxMode = TX_MODE_HE;
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgCmm->u1SpeIdx = (uint8_t)os_str_tol(pch, 0, 10);

			*pu4ManCfgBmpCmm |= (MURU_FIXED_CMM_SPE_IDX);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:cmd=comm_cfg: band=%u, AC=%u, bw=%u, "
			"GI=%u, UserCnt=%u, TxMode=%u, SpeIdx=%u\n",
			__func__,
			pCfgCmm->u1Band, pCfgCmm->u1WmmSet, pCfgDl->u1Bw,
			pCfgDl->u1GI, pCfgDl->u1UserCnt, pCfgDl->u1TxMode,
			pCfgCmm->u1SpeIdx));

		*pu4UsrList = (1 << pCfgDl->u1UserCnt) - 1;
		*pu4ManCfgBmpDl |= (MURU_FIXED_BW | MURU_FIXED_GI | MURU_FIXED_LTF | MURU_FIXED_TOTAL_USER_CNT | MURU_FIXED_TX_MODE);

		status = TRUE;
	}

	/* comm_sigb_cfg:[sigb MCS]:[sigb DCM]:[sigb Compression] */
	if (strcmp("comm_sigb_cfg", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			tmpValue = (uint8_t)os_str_tol(pch, 0, 10);
			if (tmpValue != 0xFF) {
				pCfgDl->u1SigMcs = tmpValue;
				*pu4ManCfgBmpDl |= MURU_FIXED_SIGB_MCS;
			}
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			tmpValue = (uint8_t)os_str_tol(pch, 0, 10);
			if (tmpValue != 0xFF) {
				pCfgDl->u1SigDcm = tmpValue;
				*pu4ManCfgBmpDl |= MURU_FIXED_SIGB_DCM;
			}
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			tmpValue = (uint8_t)os_str_tol(pch, 0, 10);
			if (tmpValue != 0xFF) {
				pCfgDl->u1SigCmprs = tmpValue;
				*pu4ManCfgBmpDl |= MURU_FIXED_SIGB_CMPRS;
			}
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:cmd=comm_sig_cfg: sigb mcs=%u, "
			"sig dcm=%u, sig compress=%u\n",
			__func__,
			pCfgDl->u1SigMcs,
			pCfgDl->u1SigDcm, pCfgDl->u1SigCmprs));

		status = TRUE;
	}

	/* comm_toneplan:[RU1]:[RU2]:[RU3]:[RU4]:[D26]:[RU5]:[RU6]:[RU7]:[RU8]:[U26] */
	if (strcmp("comm_toneplan", type) == 0) {

		ru_idx = c26_idx = 0;

		switch (pCfgDl->u1Bw) {
		case 0:
			loop_cnt = 1;
			break; /* 20MHz */

		case 1:
			loop_cnt = 2;
			break; /* 40MHz */

		case 2:
			loop_cnt = 5;
			break; /* 80MHz */

		case 3:
			loop_cnt = 10;
			break;/* 160MHz */

		default:
			loop_cnt = 1;
			break;
		}

		for (loop_idx = 0 ; loop_idx < loop_cnt ; loop_idx++) {

			pch = strsep(&val, ":");
			if (pch != NULL) {
				if ((loop_idx % 5) == 4) {
					pCfgDl->au1C26[c26_idx] = (uint8_t)os_str_tol(pch, 0, 10);
					c26_idx++;
				} else {
					pCfgDl->au2RU[ru_idx] = (uint8_t)os_str_tol(pch, 0, 10);
					ru_idx++;
				}
			} else {
				status = FALSE;
				goto error;
			}
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:cmd=comm_toneplan: RU1=%u,RU2=%u,RU3=%u,"
			"RU4=%u,D26=%u,RU5=%u,RU6=%u,RU7=%u,RU8=%u,U26=%u\n",
			__func__,
			pCfgDl->au2RU[0], pCfgDl->au2RU[1], pCfgDl->au2RU[2],
			pCfgDl->au2RU[3], pCfgDl->au1C26[0],
			pCfgDl->au2RU[4], pCfgDl->au2RU[5], pCfgDl->au2RU[6],
			pCfgDl->au2RU[7], pCfgDl->au1C26[1]));

		*pu4ManCfgBmpDl |= MURU_FIXED_TONE_PLAN;
		status = TRUE;
	}

	/* user:[user #1]:[WLAN_ID]:[RBN]:[RU allocation]:[LDPC]:[Nsts]:[MCS]:[MU group]:[GID]:[UP]:[StartStream]:[MuMimoSpatial]:[AckPol] */
	if (strcmp("user", type) == 0) {

		pch = strsep(&val, ":");
		if (pch != NULL) {
			user_idx = (uint8_t)(os_str_tol(pch, 0, 10) - 1);
			if (user_idx >= MAX_NUM_TXCMD_USER_INFO)
				goto error;
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u2WlanIdx = (uint16_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1RuAllocBn = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1RuAllocIdx = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1Ldpc = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1Nss = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1Mcs = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgDl->arUserInfoDl[user_idx].u1MuGroupIdx = (uint8_t)os_str_tol(pch, 0, 10);

			if (pCfgDl->arUserInfoDl[user_idx].u1MuGroupIdx > 0)
				*pu4ManCfgBmpDl |= MURU_FIXED_USER_MUMIMO_GRP;
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1VhtGid = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1VhtUp = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1HeStartStream = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->arUserInfoDl[user_idx].u1HeMuMimoSpatial = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgDl->arUserInfoDl[user_idx].u1AckPolicy = (uint8_t)os_str_tol(pch, 0, 10);

			*pu4ManCfgBmpDl |= MURU_FIXED_USER_ACK_POLICY;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:cmd=user, user#%u, WlanIdx=%u, RBN=%u, RuAlloc=%u, "
			"Ldpc=%u, Nss=%u, Mcs=%u, MuGroup=%u, VhtGid=%u, "
			"VhtUp=%u, HeStartStream=%u, "
			"HeMuMimoSpatial=%u, AckPolicy=%u\n",
			__func__, user_idx+1,
			pCfgDl->arUserInfoDl[user_idx].u2WlanIdx,
			pCfgDl->arUserInfoDl[user_idx].u1RuAllocBn,
			pCfgDl->arUserInfoDl[user_idx].u1RuAllocIdx,
			pCfgDl->arUserInfoDl[user_idx].u1Ldpc,
			pCfgDl->arUserInfoDl[user_idx].u1Nss,
			pCfgDl->arUserInfoDl[user_idx].u1Mcs,
			pCfgDl->arUserInfoDl[user_idx].u1MuGroupIdx,
			pCfgDl->arUserInfoDl[user_idx].u1VhtGid,
			pCfgDl->arUserInfoDl[user_idx].u1VhtUp,
			pCfgDl->arUserInfoDl[user_idx].u1HeStartStream,
			pCfgDl->arUserInfoDl[user_idx].u1HeMuMimoSpatial,
			pCfgDl->arUserInfoDl[user_idx].u1AckPolicy));

		*pu4ManCfgBmpDl |= (MURU_FIXED_USER_WLAN_ID | MURU_FIXED_USER_RU_ALLOC | MURU_FIXED_USER_COD | MURU_FIXED_USER_MCS | MURU_FIXED_USER_NSS);
		*pu4UsrList &= ~(1 << user_idx);

		status = TRUE;
	}

error:
#endif
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		 "%s:(status = %d\n", __func__, status);

	return status;
}

INT32 uni_hqa_muru_parse_cmd_param_ultx(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *type,
	RTMP_STRING *val,
	struct UNI_MURU_MANUAL_CONFIG_T *pMuruManCfg
)
{
	INT32	status = FALSE;
#if defined(CONFIG_WLAN_SERVICE)
	UINT8	user_idx = 0;
	PCHAR	pch = NULL;
	INT32	loop_idx;

	UINT32	*pu4UsrList;
	UINT32	*pu4ManCfgBmpUl;
	UINT32	*pu4ManCfgBmpCmm;
	struct wifi_dev *wdev = NULL;

	struct UNI_MURU_UL_MANUAL_CONFIG *pCfgUl = NULL;
	struct UNI_MURU_CMM_MANUAL_CONFIG *pCfgCmm = NULL;

	pu4UsrList = &gu4UniMuruManCfgUsrListUl;
	pu4ManCfgBmpUl = &pMuruManCfg->u4ManCfgBmpUl;
	pu4ManCfgBmpCmm = &pMuruManCfg->u4ManCfgBmpCmm;
	pCfgUl = &pMuruManCfg->rCfgUl;
	pCfgCmm = &pMuruManCfg->rCfgCmm;

	/* comm_cfg:[Band]:[BW]:[GI&LTF]:[total User#]*/
	if (strcmp("comm_cfg", type) == 0) {

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgCmm->u1Band = (uint8_t)os_str_tol(pch, 0, 10);

			if (pCfgCmm->u1Band < TESTMODE_BAND_NUM) {
				/* set TXCMD mode */
				wdev = &pAd->ate_wdev[pCfgCmm->u1Band][1];
				pCfgCmm->u1WmmSet = HcGetWmmIdx(pAd, wdev);
			} else {
				status = FALSE;
				goto error;
			}

			*pu4ManCfgBmpCmm |= (MURU_FIXED_CMM_BAND | MURU_FIXED_CMM_WMM_SET);
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->u1UlBw = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->u1UlGiLtf = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u1UserCnt = (pAd->CommonCfg.HE_OfdmaUserNum) ?
									(uint8_t)(pAd->CommonCfg.HE_OfdmaUserNum) :
									(uint8_t)os_str_tol(pch, 0, 10);

			pCfgCmm->u1PpduFmt |= MURU_PPDU_HE_TRIG;
			pCfgCmm->u1SchType |= MURU_OFDMA_SCH_TYPE_UL;
			*pu4ManCfgBmpCmm |= (MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:cmd=comm_cfg: Band=%u, AC=%u, "
			"UlBw=%u, UlGiLtf=%u, UserCnt=%u\n",
			__func__,
			pCfgCmm->u1Band, pCfgCmm->u1WmmSet, pCfgUl->u1UlBw,
			pCfgUl->u1UlGiLtf, pCfgUl->u1UserCnt));

		*pu4UsrList = (1 << pCfgUl->u1UserCnt) - 1;
		*pu4ManCfgBmpUl |= (MURU_FIXED_UL_BW | MURU_FIXED_UL_GILTF | MURU_FIXED_UL_TOTAL_USER_CNT);

		status = TRUE;
	}

	/* comm_ta:[00]:[00]:[00]:[00]:[00]:[00] */
	if (strcmp("comm_ta", type) == 0) {

		for (loop_idx = 0 ; loop_idx < MAC_ADDR_LEN ; loop_idx++) {
			pch = strsep(&val, ":");

			if (pch != NULL) {
				pCfgUl->u1TrigTa[loop_idx] = (uint8_t)os_str_tol(pch, 0, 16);
			} else {
				status = FALSE;
				goto error;
			}
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:cmd=comm_ta:%02X:%02X:%02X:%02X:%02X:%02X\n",
			__func__,
			pCfgUl->u1TrigTa[0],
			pCfgUl->u1TrigTa[1],
			pCfgUl->u1TrigTa[2],
			pCfgUl->u1TrigTa[3],
			pCfgUl->u1TrigTa[4],
			pCfgUl->u1TrigTa[5]));

		*pu4ManCfgBmpUl |= MURU_FIXED_TRIG_TA;

		status = TRUE;
	}

	/* ul_trig_cfg:[HE_TRIG cnt]:[HE_TRIG interval] */
	if (strcmp("ul_trig_cfg", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u2TrigCnt = (uint16_t)os_str_tol(pch, 0, 10);
			if (pCfgUl->u2TrigCnt)
				*pu4ManCfgBmpUl |= MURU_FIXED_TRIG_CNT;
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u2TrigIntv = (uint16_t)os_str_tol(pch, 0, 10);
			if (pCfgUl->u2TrigIntv)
				*pu4ManCfgBmpUl |= MURU_FIXED_TRIG_INTV;
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:cmd=ul_trig_cfg: TrigCnt=%u, TrigIntv=%u\n",
			__func__, pCfgUl->u2TrigCnt, pCfgUl->u2TrigIntv));

		status = TRUE;
	}

	/* user:[user #1]:[WLAN_ID]:[RBN]:[RU allocation]:[LDPC]:[Nsts]:[MCS]:[packet size] */
	if (strcmp("user", type) == 0) {

		pch = strsep(&val, ":");
		if (pch != NULL) {
			user_idx = (uint8_t)(os_str_tol(pch, 0, 10) - 1);
			if (user_idx >= MAX_NUM_TXCMD_USER_INFO)
				goto error;
		} else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->arUserInfoUl[user_idx].u2WlanIdx = (uint16_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->arUserInfoUl[user_idx].u1RuAllocBn = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->arUserInfoUl[user_idx].u1RuAllocIdx = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->arUserInfoUl[user_idx].u1Ldpc = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->arUserInfoUl[user_idx].u1Nss = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->arUserInfoUl[user_idx].u1Mcs = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		if (MURU_MANUAL_CFG_CHK(*pu4ManCfgBmpUl, MURU_FIXED_TRIG_CNT)) {
			if (pch != NULL)
				pCfgUl->arUserInfoUl[user_idx].u4TrigPktSize = (UINT32)os_str_tol(pch, 0, 10);
			else {
				status = FALSE;
				goto error;
			}

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s:cmd=user, user#%u, PktSize=%u\n",
				__func__, user_idx+1,
				pCfgUl->arUserInfoUl[user_idx].u4TrigPktSize));

			*pu4ManCfgBmpUl |= MURU_FIXED_TRIG_PKT_SIZE;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:cmd=user, user#%u, WlanIdx=%u, RBN=%u, "
			"RuAlloc=%u, Ldpc=%u, Nss=%u, Mcs=%u\n",
			__func__, user_idx+1,
			pCfgUl->arUserInfoUl[user_idx].u2WlanIdx,
			pCfgUl->arUserInfoUl[user_idx].u1RuAllocBn,
			pCfgUl->arUserInfoUl[user_idx].u1RuAllocIdx,
			pCfgUl->arUserInfoUl[user_idx].u1Ldpc,
			pCfgUl->arUserInfoUl[user_idx].u1Nss,
			pCfgUl->arUserInfoUl[user_idx].u1Mcs));

		*pu4ManCfgBmpUl |= (MURU_FIXED_USER_UL_WLAN_ID | MURU_FIXED_USER_UL_RU_ALLOC | MURU_FIXED_USER_UL_COD | MURU_FIXED_USER_UL_NSS | MURU_FIXED_USER_UL_MCS);
		*pu4UsrList &= ~(1 << user_idx);

		status = TRUE;
	}

error:
#endif
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		 "%s:(status = %d\n", __func__, status);

	return status;
}

INT32 uni_muru_parse_cmd_param_muru_manual_config(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *type,
	RTMP_STRING *val,
	struct UNI_MURU_MANUAL_CONFIG_T *pMuruManCfg
)
{
	INT32	status = FALSE;
	INT32	loop_cnt, loop_idx, ru_idx, c26_idx;
	PCHAR	pch = NULL;

	struct UNI_MURU_DL_MANUAL_CONFIG *pCfgDl = NULL;
	struct UNI_MURU_UL_MANUAL_CONFIG *pCfgUl = NULL;
	struct UNI_MURU_CMM_MANUAL_CONFIG *pCfgCmm = NULL;
	struct UNI_MURU_DBG_MANUAL_CONFIG *pCfgDbg = NULL;
	UINT32 *pCfgBmpDl, *pCfgBmpUl, *pCfgBmpCmm, *pCfgBmpDbg;
	UINT32 *pUsrLstDl, *pUsrLstUl;

	pCfgDl = &pMuruManCfg->rCfgDl;
	pCfgUl = &pMuruManCfg->rCfgUl;
	pCfgCmm = &pMuruManCfg->rCfgCmm;
	pCfgDbg = &pMuruManCfg->rCfgDbg;
	pCfgBmpDl = &pMuruManCfg->u4ManCfgBmpDl;
	pCfgBmpUl = &pMuruManCfg->u4ManCfgBmpUl;
	pCfgBmpCmm = &pMuruManCfg->u4ManCfgBmpCmm;
	pCfgBmpDbg = &pMuruManCfg->u4ManCfgBmpDbg;
	pUsrLstDl = &gu4UniMuruManCfgUsrListDl;
	pUsrLstUl = &gu4UniMuruManCfgUsrListUl;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:\n", __func__));

	/********** Common **********/
	/* global_comm_band */
	if (strcmp("global_comm_band", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgCmm->u1Band = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("cmd=global_comm_band: %u\n", pCfgCmm->u1Band));

		*pCfgBmpCmm |= MURU_FIXED_CMM_BAND;

		status = TRUE;
	}

	/* global_comm_wmm */
	if (strcmp("global_comm_wmm", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgCmm->u1WmmSet = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("cmd=global_comm_wmm: %u\n",
			pCfgCmm->u1WmmSet));

		*pCfgBmpCmm |= MURU_FIXED_CMM_WMM_SET;

		status = TRUE;
	}


	/* dl_comm_bw */
	if (strcmp("dl_comm_bw", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->u1Bw = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("cmd=dl_comm_bw: %u\n", pCfgDl->u1Bw));

		*pCfgBmpDl |= MURU_FIXED_BW;

		status = TRUE;
	}

	/* dl_comm_gi */
	if (strcmp("dl_comm_gi", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->u1GI = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=dl_comm_gi: %u\n", pCfgDl->u1GI));

		*pCfgBmpDl |= MURU_FIXED_GI;

		status = TRUE;
	}

	/* dl_comm_txmode */
	if (strcmp("dl_comm_txmode", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->u1TxMode = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=dl_comm_txmode: %u\n", pCfgDl->u1TxMode));

		*pCfgBmpDl |= MURU_FIXED_TX_MODE;

		status = TRUE;
	}

	/* dl_comm_user_cnt */
	if (strcmp("dl_comm_user_cnt", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgDl->u1UserCnt = (pAd->CommonCfg.HE_OfdmaUserNum) ?
				(uint8_t)(pAd->CommonCfg.HE_OfdmaUserNum) :
				(uint8_t)os_str_tol(pch, 0, 10);

			pCfgCmm->u1PpduFmt |= MURU_PPDU_HE_MU;
			pCfgCmm->u1SchType |= MURU_OFDMA_SCH_TYPE_DL;
			*pCfgBmpCmm |= (MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("cmd=dl_comm_user_cnt: %u\n",
			pCfgDl->u1UserCnt));

		*pCfgBmpDl |= MURU_FIXED_TOTAL_USER_CNT;

		status = TRUE;
	}

	/* dl_comm_txpwr */
	if (strcmp("dl_comm_txpwr", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->u1TxPwr = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=dl_comm_txpwr: %u\n", pCfgDl->u1TxPwr));

		*pCfgBmpDl |= MURU_FIXED_TXPOWER;

		status = TRUE;
	}

	/* dl_user_wlan_idx */
	if (strcmp("dl_user_wlan_idx", type) == 0) {

		if (MURU_MANUAL_CFG_CHK(*pCfgBmpDl, MURU_FIXED_TOTAL_USER_CNT)) {
			loop_cnt = pCfgDl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgDl->arUserInfoDl[loop_idx].u2WlanIdx = (uint16_t)os_str_tol(pch, 0, 10);
				} else {
					status = FALSE;
					goto error;
				}
				MTWF_LOG(DBG_CAT_ALL,
					DBG_SUBCAT_ALL,
					DBG_LVL_OFF,
					("cmd=dl_user_wlan_idx: user %u, value=%u\n",
					loop_idx + 1,
					pCfgDl->arUserInfoDl[loop_idx].u2WlanIdx));
			}

			*pCfgBmpDl |= MURU_FIXED_USER_WLAN_ID;
			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=dl_user_wlan_idx: set dl_comm_user_cnt "
				"before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* dl_user_coding*/
	if (strcmp("dl_user_cod", type) == 0) {

		if (MURU_MANUAL_CFG_CHK(*pCfgBmpDl,
			MURU_FIXED_TOTAL_USER_CNT)) {
			loop_cnt = pCfgDl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgDl->arUserInfoDl[loop_idx].u1Ldpc =
						(uint8_t)os_str_tol(pch, 0, 10);
				} else {
					status = FALSE;
					goto error;
				}

				MTWF_LOG(DBG_CAT_ALL,
					DBG_SUBCAT_ALL,
					DBG_LVL_OFF,
					("cmd=dl_user_cod: user %u, value=%u\n",
					loop_idx + 1,
					pCfgDl->arUserInfoDl[loop_idx].u1Ldpc));
			}

			*pCfgBmpDl |= MURU_FIXED_USER_COD;
			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=dl_user_cod: set dl_comm_user_cnt before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* dl_user_mcs */
	if (strcmp("dl_user_mcs", type) == 0) {

		if (MURU_MANUAL_CFG_CHK(*pCfgBmpDl,
			MURU_FIXED_TOTAL_USER_CNT)) {
			loop_cnt = pCfgDl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgDl->arUserInfoDl[loop_idx].u1Mcs =
						(uint8_t)os_str_tol(pch, 0, 10);
				} else {
					status = FALSE;
					goto error;
				}

				MTWF_LOG(DBG_CAT_ALL,
					DBG_SUBCAT_ALL,
					DBG_LVL_OFF,
					("cmd=dl_user_mcs: user %u, value=%u\n",
					loop_idx + 1,
					pCfgDl->arUserInfoDl[loop_idx].u1Mcs));
			}

			*pCfgBmpDl |= MURU_FIXED_USER_MCS;
			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=dl_user_mcs: set dl_comm_user_cnt before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* dl_user_nss */
	if (strcmp("dl_user_nss", type) == 0) {

		if (MURU_MANUAL_CFG_CHK(*pCfgBmpDl,
			MURU_FIXED_TOTAL_USER_CNT)) {
			loop_cnt = pCfgDl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgDl->arUserInfoDl[loop_idx].u1Nss =
						(uint8_t)os_str_tol(pch, 0, 10);
				} else {
					status = FALSE;
					goto error;
				}

				MTWF_LOG(DBG_CAT_ALL,
					DBG_SUBCAT_ALL,
					DBG_LVL_OFF,
					("cmd=dl_user_nss: user %u, value=%u\n",
					loop_idx + 1,
					pCfgDl->arUserInfoDl[loop_idx].u1Nss));
			}

			*pCfgBmpDl |= MURU_FIXED_USER_NSS;
			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=dl_user_nss: set dl_comm_user_cnt before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* dl_comm_toneplan:[RU1]:[RU2]:[RU3]:[RU4]:[D26]:[RU5]:[RU6]:[RU7]:[RU8]:[U26] */
	if (strcmp("dl_comm_toneplan", type) == 0) {

		if (MURU_MANUAL_CFG_CHK(*pCfgBmpDl, MURU_FIXED_BW)) {
			ru_idx = c26_idx = 0;

			switch (pCfgDl->u1Bw) {
			case 0:
				loop_cnt = 1;
				break; /* 20MHz */

			case 1:
				loop_cnt = 2;
				break; /* 40MHz */

			case 2:
				loop_cnt = 5;
				break; /* 80MHz */

			case 3:
				loop_cnt = 10;
				break; /* 160MHz */

			default:
				loop_cnt = 1;
				break;
			}

			for (loop_idx = 0 ; loop_idx < loop_cnt ; loop_idx++) {

				pch = strsep(&val, ":");
				if (pch != NULL) {
					if ((loop_idx % 5) == 4) {
						pCfgDl->au1C26[c26_idx] = (uint8_t)os_str_tol(pch, 0, 10);
						c26_idx++;
					} else {
						pCfgDl->au2RU[ru_idx] = (uint8_t)os_str_tol(pch, 0, 10);
						ru_idx++;
					}
				} else {
					status = FALSE;
					goto error;
				}
			}

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("cmd=dl_comm_toneplan: RU1=%u,RU2=%u,RU3=%u,"
			"RU4=%u,D26=%u,RU5=%u,RU6=%u,RU7=%u,RU8=%u,U26=%u\n",
			pCfgDl->au2RU[0], pCfgDl->au2RU[1], pCfgDl->au2RU[2],
			pCfgDl->au2RU[3], pCfgDl->au1C26[0],
			pCfgDl->au2RU[4], pCfgDl->au2RU[5], pCfgDl->au2RU[6],
			pCfgDl->au2RU[7], pCfgDl->au1C26[1]));

			*pCfgBmpDl |= MURU_FIXED_TONE_PLAN;
			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("cmd=dl_comm_toneplan: set dl_comm_bw "
			"before config dl_comm_toneplan\n"));

			status = FALSE;
			goto error;
		}
	}

	/* dl_comm_ltf */
	if (strcmp("dl_comm_ltf", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgDl->u1Ltf = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=dl_comm_ltf: %u\n", pCfgDl->u1Ltf));

		*pCfgBmpDl |= MURU_FIXED_LTF;

		status = TRUE;
	}

	/* dl_user_ack_polocy */
	if (strcmp("dl_user_ack_policy", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpDl, MURU_FIXED_TOTAL_USER_CNT)) {
			loop_cnt = pCfgDl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgDl->arUserInfoDl[loop_idx].u1AckPolicy = (uint8_t)os_str_tol(pch, 0, 10);

					MTWF_LOG(DBG_CAT_ALL,
						DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("cmd=dl_user_ack_policy: "
						"user %u, value=%u\n",
						loop_idx + 1,
						pCfgDl->arUserInfoDl[loop_idx]
								.u1AckPolicy));
				} else {
					status = FALSE;
					goto error;
				}
			}

			*pCfgBmpDl |= MURU_FIXED_USER_ACK_POLICY;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=dl_user_ack_policy: set dl_comm_user_cnt "
				"before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* dl_user_ru_alloc :[RBN]:[RU alloc]*/
	if (strcmp("dl_user_ru_alloc", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpDl, MURU_FIXED_TOTAL_USER_CNT)) {
			loop_cnt = pCfgDl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL)
					pCfgDl->arUserInfoDl[loop_idx].u1RuAllocBn = (uint8_t)os_str_tol(pch, 0, 10);
				else {
					status = FALSE;
					goto error;
				}

				pch = strsep(&val, ":");
				if (pch != NULL)
					pCfgDl->arUserInfoDl[loop_idx].u1RuAllocIdx = (uint8_t)os_str_tol(pch, 0, 10);
				else {
					status = FALSE;
					goto error;
				}

				MTWF_LOG(DBG_CAT_ALL,
				DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=dl_user_ru_alloc:[RBN]:[RU alloc]= "
				"user %u, RBN=%u, RU alloc idx=%u\n",
				loop_idx + 1,
				pCfgDl->arUserInfoDl[loop_idx].u1RuAllocBn,
				pCfgDl->arUserInfoDl[loop_idx].u1RuAllocIdx));
			}

			*pCfgBmpDl |= MURU_FIXED_USER_RU_ALLOC;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=dl_user_ru_alloc: set dl_comm_user_cnt "
				"before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/********** Uplink **********/

	/* ul_comm_user_cnt */
	if (strcmp("ul_comm_user_cnt", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u1UserCnt = (pAd->CommonCfg.HE_OfdmaUserNum) ?
									(uint8_t)(pAd->CommonCfg.HE_OfdmaUserNum) :
									(uint8_t)os_str_tol(pch, 0, 10);

			pCfgCmm->u1PpduFmt |= MURU_PPDU_HE_TRIG;
			pCfgCmm->u1SchType |= MURU_OFDMA_SCH_TYPE_UL;
			*pCfgBmpCmm |= (MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("cmd=ul_comm_user_cnt: %u\n", pCfgUl->u1UserCnt));

		*pCfgBmpUl |= MURU_FIXED_UL_TOTAL_USER_CNT;

		status = TRUE;
	}

	/* ul_comm_ack_type */
	if (strcmp("ul_comm_ack_type", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->u1BaType = (uint8_t)os_str_tol(pch, 0, 10);
		else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("cmd=ul_comm_ack_type: %u\n", pCfgUl->u1BaType));

		*pCfgBmpUl |= MURU_FIXED_UL_ACK_TYPE;

		status = TRUE;
	}

	/* ul_comm_trig_intv */
	if (strcmp("ul_comm_trig_intv", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->u2TrigIntv = (uint16_t)os_str_tol(pch, 0, 10);

		if (pCfgUl->u2TrigIntv) {
			*pCfgBmpUl |= MURU_FIXED_TRIG_INTV;
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("cmd=ul_comm_trig_intv: %u\n", pCfgUl->u2TrigIntv));

		status = TRUE;
	}

	/* ul_comm_trig_cnt */
	if (strcmp("ul_comm_trig_cnt", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL)
			pCfgUl->u2TrigCnt = (uint16_t)os_str_tol(pch, 0, 10);

		if (pCfgUl->u2TrigIntv) {
			*pCfgBmpUl |= MURU_FIXED_TRIG_CNT;
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("cmd=ul_comm_trig_cnt: %u\n", pCfgUl->u2TrigCnt));

		status = TRUE;
	}

	/* ul_comm_trig_type */
	if (strcmp("ul_comm_trig_type", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u1TrigType = (uint8_t)os_str_tol(pch, 0, 10);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("cmd=ul_comm_trig_type: %u\n", pCfgUl->u1TrigType));

		*pCfgBmpUl |= MURU_FIXED_TRIG_TYPE;

		status = TRUE;
	}

	/* ul_comm_ta:[00]:[00]:[00]:[00]:[00]:[00] */
	if (strcmp("ul_comm_ta", type) == 0) {

		for (loop_idx = 0 ; loop_idx < MAC_ADDR_LEN ; loop_idx++) {
			pch = strsep(&val, ":");

			if (pch != NULL) {
				pCfgUl->u1TrigTa[loop_idx] = (uint8_t)os_str_tol(pch, 0, 16);
			} else {
				status = FALSE;
				goto error;
			}
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:cmd=comm_ta:%02X:%02X:%02X:%02X:%02X:%02X\n",
			__func__,
			pCfgUl->u1TrigTa[0],
			pCfgUl->u1TrigTa[1],
			pCfgUl->u1TrigTa[2],
			pCfgUl->u1TrigTa[3],
			pCfgUl->u1TrigTa[4],
			pCfgUl->u1TrigTa[5]));

		*pCfgBmpUl |= MURU_FIXED_TRIG_TA;

		status = TRUE;
	}

	/* ul_comm_bw */
	if (strcmp("ul_comm_bw", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u1UlBw = (uint8_t)os_str_tol(pch, 0, 10);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_comm_bw: %u\n", pCfgUl->u1UlBw));

		*pCfgBmpUl |= MURU_FIXED_UL_BW;

		status = TRUE;
	}

	/* ul_comm_gi_ltf */
	if (strcmp("ul_comm_gi_ltf", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u1UlGiLtf = (uint8_t)os_str_tol(pch, 0, 10);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_comm_gi_ltf: %u\n", pCfgUl->u1UlGiLtf));

		*pCfgBmpUl |= MURU_FIXED_UL_GILTF;

		status = TRUE;
	}

	/* ul_comm_length */
	if (strcmp("ul_comm_length", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgUl->u2UlLength = (uint16_t)os_str_tol(pch, 0, 10);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_comm_length: %u\n", pCfgUl->u2UlLength));

		*pCfgBmpUl |= MURU_FIXED_UL_LENGTH;

		status = TRUE;
	}

	/* ul_comm_tf_pad */
	if (strcmp("ul_comm_tf_pad", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			if (pAd->CommonCfg.HE_TrigPadding) {
				if (pAd->CommonCfg.HE_TrigPadding == 8)
					pCfgUl->u1TfPad = 1;
				else if (pAd->CommonCfg.HE_TrigPadding == 16)
					pCfgUl->u1TfPad = 2;
				else
					pCfgUl->u1TfPad = 2;
			} else {
				pCfgUl->u1TfPad = (uint8_t)os_str_tol(pch, 0, 10);
			}
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_comm_tf_pad: %u\n", pCfgUl->u1TfPad));

		*pCfgBmpUl |= MURU_FIXED_UL_TF_PAD;

		status = TRUE;
	}

    /* HETB RX Debug: ul_comm_rx_hetb_cfg1 */
	if (strcmp("ul_comm_rx_hetb_cfg1", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgDbg->au4RxHetbCfg[0] = (UINT32)os_str_tol(pch, 0, 10);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("cmd=ul_comm_rx_hetb_cfg1: %u\n", pCfgDbg->au4RxHetbCfg[0]));

		*pCfgBmpDbg |= MURU_FIXED_RX_HETB_CFG1;

		status = TRUE;
	}

	 /* HETB RX Debug:ul_comm_rx_hetb_cfg2 */
	if (strcmp("ul_comm_rx_hetb_cfg2", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgDbg->au4RxHetbCfg[1] = (UINT32)os_str_tol(pch, 0, 10);
		} else {
			status = FALSE;
			goto error;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("cmd=ul_comm_rx_hetb_cfg2: %u\n", pCfgDbg->au4RxHetbCfg[1]));

		*pCfgBmpDbg |= MURU_FIXED_RX_HETB_CFG2;

		status = TRUE;
	}

	/* ul_user_wlan_idx */
	if (strcmp("ul_user_wlan_idx", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpUl, MURU_FIXED_UL_TOTAL_USER_CNT)) {
			loop_cnt = pCfgUl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgUl->arUserInfoUl[loop_idx].u2WlanIdx = (uint16_t)os_str_tol(pch, 0, 10);

					MTWF_LOG(DBG_CAT_ALL,
					DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_wlan_idx: user %u, "
					"value=%u\n",
					loop_idx + 1,
					pCfgUl->arUserInfoUl[loop_idx].u2WlanIdx
					));
				} else {
					status = FALSE;
					goto error;
				}
			}

			*pCfgBmpUl |= MURU_FIXED_USER_UL_WLAN_ID;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("cmd=ul_user_wlan_idx: set ul_comm_user_cnt before "
			"user specific config\n"));

			status = FALSE;
			goto error;
		}
	}
	/* ul_user_cod */
	if (strcmp("ul_user_cod", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpUl, MURU_FIXED_UL_TOTAL_USER_CNT)) {
			loop_cnt = pCfgUl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgUl->arUserInfoUl[loop_idx].u1Ldpc = (uint8_t)os_str_tol(pch, 0, 10);

					MTWF_LOG(DBG_CAT_ALL,
					DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_cod: user %u, value=%u\n",
					loop_idx + 1,
					pCfgUl->arUserInfoUl[loop_idx].u1Ldpc));
				} else {
					status = FALSE;
					goto error;
				}
			}

			*pCfgBmpUl |= MURU_FIXED_USER_UL_COD;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("cmd=ul_user_cod: set ul_comm_user_cnt before "
			"user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* ul_user_mcs */
	if (strcmp("ul_user_mcs", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpUl, MURU_FIXED_UL_TOTAL_USER_CNT)) {
			loop_cnt = pCfgUl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgUl->arUserInfoUl[loop_idx].u1Mcs = (uint8_t)os_str_tol(pch, 0, 10);
					MTWF_LOG(DBG_CAT_ALL,
					DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_mcs: user %u, value=%u\n",
					loop_idx + 1,
					pCfgUl->arUserInfoUl[loop_idx].u1Mcs));
				} else {
					status = FALSE;
					goto error;
				}
			}

			*pCfgBmpUl |= MURU_FIXED_USER_UL_MCS;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_user_mcs: set ul_comm_user_cnt "
				"before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* ul_user_ssAlloc_raru */
	if (strcmp("ul_user_ssAlloc_raru", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpUl, MURU_FIXED_UL_TOTAL_USER_CNT)) {
			loop_cnt = pCfgUl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgUl->arUserInfoUl[loop_idx].u1Nss = (uint8_t)os_str_tol(pch, 0, 10);

					MTWF_LOG(DBG_CAT_ALL,
					DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_ssAlloc_raru: "
					"user %u, value=%u\n",
					loop_idx + 1,
					pCfgUl->arUserInfoUl[loop_idx].u1Nss));
				} else {
					status = FALSE;
					goto error;
				}
			}

			*pCfgBmpUl |= MURU_FIXED_USER_UL_NSS;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_user_ssAlloc_raru: set ul_comm_user_cnt"
				" before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* ul_user_rssi */
	if (strcmp("ul_user_rssi", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpUl, MURU_FIXED_UL_TOTAL_USER_CNT)) {
			loop_cnt = pCfgUl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL) {
					pCfgUl->arUserInfoUl[loop_idx].u1TargetRssi = (uint8_t)os_str_tol(pch, 0, 10);

					MTWF_LOG(DBG_CAT_ALL,
					DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("cmd=ul_user_rssi: user %u, value=%u\n",
					loop_idx + 1,
					pCfgUl->arUserInfoUl[loop_idx].u1TargetRssi));
				} else {
					status = FALSE;
					goto error;
				}
			}

			*pCfgBmpUl |= MURU_FIXED_USER_UL_TARGET_RSSI;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_user_rssi: set ul_comm_user_cnt "
				"before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* dl_comm_toneplan:[RU1]:[RU2]:[RU3]:[RU4]:[D26]:[RU5]:[RU6]:[RU7]:[RU8]:[U26] */
	if (strcmp("ul_comm_toneplan", type) == 0) {

		if (MURU_MANUAL_CFG_CHK(*pCfgBmpUl, MURU_FIXED_UL_BW)) {
			ru_idx = c26_idx = 0;

			switch (pCfgUl->u1UlBw) {
			case 0:
				loop_cnt = 1;
				break; /* 20MHz */

			case 1:
				loop_cnt = 2;
				break; /* 40MHz */

			case 2:
				loop_cnt = 5;
				break; /* 80MHz */

			case 3:
				loop_cnt = 10;
				break; /* 160MHz */

			default:
				loop_cnt = 1;
				break;
			}

			for (loop_idx = 0 ; loop_idx < loop_cnt ; loop_idx++) {

				pch = strsep(&val, ":");
				if (pch != NULL) {
					if ((loop_idx % 5) == 4) {
						pCfgUl->au1UlC26[c26_idx] = (uint8_t)os_str_tol(pch, 0, 10);
						c26_idx++;
					} else {
						pCfgUl->au2UlRU[ru_idx] = (uint8_t)os_str_tol(pch, 0, 10);
						ru_idx++;
					}
				} else {
					status = FALSE;
					goto error;
				}
			}

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("cmd=ul_comm_toneplan: RU1=%u,RU2=%u,RU3=%u,RU4=%u,"
			"D26=%u,RU5=%u,RU6=%u,RU7=%u,RU8=%u,U26=%u\n",
			pCfgUl->au2UlRU[0], pCfgUl->au2UlRU[1],
			pCfgUl->au2UlRU[2], pCfgUl->au2UlRU[3],
			pCfgUl->au1UlC26[0],
			pCfgUl->au2UlRU[4], pCfgUl->au2UlRU[5],
			pCfgUl->au2UlRU[6], pCfgUl->au2UlRU[7],
			pCfgUl->au1UlC26[1]));

			*pCfgBmpUl |= MURU_FIXED_TONE_PLAN;
			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("cmd=dl_comm_toneplan: set dl_comm_bw before "
			"config dl_comm_toneplan\n"));

			status = FALSE;
			goto error;
		}
	}

	/* ul_user_ru_alloc :[RBN]:[RU alloc]*/
	if (strcmp("ul_user_ru_alloc", type) == 0) {
		if (MURU_MANUAL_CFG_CHK(*pCfgBmpUl, MURU_FIXED_UL_TOTAL_USER_CNT)) {
			loop_cnt = pCfgUl->u1UserCnt;
			for (loop_idx = 0; loop_idx < loop_cnt; loop_idx++) {
				pch = strsep(&val, ":");
				if (pch != NULL)
					pCfgUl->arUserInfoUl[loop_idx].u1RuAllocBn = (uint8_t)os_str_tol(pch, 0, 10);
				else {
					status = FALSE;
					goto error;
				}

				pch = strsep(&val, ":");
				if (pch != NULL)
					pCfgUl->arUserInfoUl[loop_idx].u1RuAllocIdx = (uint8_t)os_str_tol(pch, 0, 10);
				else {
					status = FALSE;
					goto error;
				}

				MTWF_LOG(DBG_CAT_ALL,
				DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_user_ru_alloc:[RBN]:[RU alloc]= "
				"user %u, RBN=%u, RU alloc idx=%u\n",
				loop_idx + 1,
				pCfgUl->arUserInfoUl[loop_idx].u1RuAllocBn,
				pCfgUl->arUserInfoUl[loop_idx].u1RuAllocIdx));
			}

			*pCfgBmpUl |= MURU_FIXED_USER_UL_RU_ALLOC;

			status = TRUE;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_user_ru_alloc: set ul_comm_user_cnt "
				"before user specific config\n"));

			status = FALSE;
			goto error;
		}
	}

	/* ul_user_rx_nonsf_en_bitmap */
	if (strcmp("ul_user_rx_nonsf_en_bitmap", type) == 0) {
		pch = strsep(&val, ":");
		if (pch != NULL) {
			pCfgDbg->u4RxHetbNonsfEnBitmap = (UINT32)os_str_tol(pch, 0, 10);

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("cmd=ul_user_rx_nonsf_en_bitmap: value=0x%x\n",
				pCfgDbg->u4RxHetbNonsfEnBitmap));
		} else {
			status = FALSE;
			goto error;
		}
		*pCfgBmpDbg |= MURU_FIXED_NONSF_EN_BITMAP;
		status = TRUE;
	}

error:
	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		 "%s:(status = %d\n", __func__, status);

	return status;
}

INT32 uni_hqa_muru_set_dl_tx_muru_config(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg
)
{
	INT32 Ret = TRUE;
	char sep_val = ':';
	RTMP_STRING *param_type, *param_val;

	UINT32	*pu4UsrList;
	UINT32	*pu4ManCfgBmpDl;
	UINT32	*pu4ManCfgBmpCmm;

	struct UNI_MURU_DL_MANUAL_CONFIG *pCfgDl = NULL;
	struct UNI_MURU_CMM_MANUAL_CONFIG *pCfgCmm = NULL;

	pu4UsrList = &gu4UniMuruManCfgUsrListDl;
	pu4ManCfgBmpDl = &grUniMuruManCfgInf.u4ManCfgBmpDl;
	pu4ManCfgBmpCmm = &grUniMuruManCfgInf.u4ManCfgBmpCmm;
	pCfgDl = &grUniMuruManCfgInf.rCfgDl;
	pCfgCmm = &grUniMuruManCfgInf.rCfgCmm;

	param_type = arg;
	if (strlen(param_type)) {

		if (strcmp("init", param_type) == 0) {

			/* init */
			*pu4UsrList = 0;
			*pu4ManCfgBmpDl = 0;
			*pu4ManCfgBmpCmm &= ~(MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
			pCfgCmm->u1PpduFmt &= ~MURU_PPDU_HE_MU;
			pCfgCmm->u1SchType &= ~MURU_OFDMA_SCH_TYPE_DL;
			os_zero_mem(pCfgDl, sizeof(struct UNI_MURU_DL_MANUAL_CONFIG));

			if (pAd->CommonCfg.HE_OfdmaUserNum) {
				pCfgDl->u1UserCnt = (uint8_t)(pAd->CommonCfg.HE_OfdmaUserNum);
				*pu4ManCfgBmpDl |= MURU_FIXED_TOTAL_USER_CNT;
			}

			if (pAd->CommonCfg.HE_PpduFmt) {
				pCfgCmm->u1PpduFmt = (uint8_t)(pAd->CommonCfg.HE_PpduFmt);
				*pu4ManCfgBmpCmm |= MURU_FIXED_CMM_PPDU_FMT;
			}

			if (pAd->CommonCfg.HE_OfdmaSchType) {
				pCfgCmm->u1SchType = (uint8_t)(pAd->CommonCfg.HE_OfdmaSchType);
				*pu4ManCfgBmpCmm |= MURU_FIXED_CMM_SCH_TYPE;
			}

			Ret = TRUE;
		} else if (strcmp("update", param_type) == 0) {

			/* update */
			if (*pu4UsrList != 0) {
				MTWF_LOG(DBG_CAT_ALL,
					DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s:cmd=update, "
					"target_updated_user_bmp=0x%x, "
					"not_yet_updated_user_bmp:0x%x\n",
					__func__,
					((1 << pCfgDl->u1UserCnt) - 1),
					*pu4UsrList));
			} else {
				Ret = UniCmdMuruParameterSet(pAd, (RTMP_STRING *)&grUniMuruManCfgInf, UNI_CMD_MURU_MANUAL_CONFIG);
			}
		} else {

			param_val = rtstrchr(param_type, sep_val);

			if (param_val) {
				*param_val = 0;
				param_val++;
			}

			if (strlen(param_type) && param_val && strlen(param_val)) {
				Ret = uni_hqa_muru_parse_cmd_param_dltx(pAd, param_type, param_val, &grUniMuruManCfgInf);

				if (Ret == FALSE)
					goto error;
			}
		}
	}

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: cmd sub-group = %s, Ret = %d\n",
			 __func__, param_type, Ret));
	return Ret;
}

INT32 uni_hqa_muru_set_ul_tx_muru_config(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg
)
{
	INT32 Ret = TRUE;
	char sep_val = ':';
	RTMP_STRING *param_type, *param_val;

	UINT32	*pu4UsrList;
	UINT32	*pu4ManCfgBmpUl;
	UINT32	*pu4ManCfgBmpCmm;

	struct UNI_MURU_UL_MANUAL_CONFIG *pCfgUl = NULL;
	struct UNI_MURU_CMM_MANUAL_CONFIG *pCfgCmm = NULL;

	pu4UsrList = &gu4UniMuruManCfgUsrListUl;
	pu4ManCfgBmpUl = &grUniMuruManCfgInf.u4ManCfgBmpUl;
	pu4ManCfgBmpCmm = &grUniMuruManCfgInf.u4ManCfgBmpCmm;
	pCfgUl = &grUniMuruManCfgInf.rCfgUl;
	pCfgCmm = &grUniMuruManCfgInf.rCfgCmm;

	param_type = arg;
	if (strlen(param_type)) {

		if (strcmp("init", param_type) == 0) {
			/* init */
			*pu4UsrList = 0;
			*pu4ManCfgBmpUl = 0;
			*pu4ManCfgBmpCmm &= ~(MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
			pCfgCmm->u1PpduFmt &= ~MURU_PPDU_HE_TRIG;
			pCfgCmm->u1SchType &= ~MURU_OFDMA_SCH_TYPE_UL;
			os_zero_mem(pCfgUl, sizeof(struct UNI_MURU_UL_MANUAL_CONFIG));

			if (pAd->CommonCfg.HE_OfdmaUserNum) {
				pCfgUl->u1UserCnt = (uint8_t)(pAd->CommonCfg.HE_OfdmaUserNum);
				*pu4ManCfgBmpUl |= MURU_FIXED_UL_TOTAL_USER_CNT;
			}

			if (pAd->CommonCfg.HE_PpduFmt) {
				pCfgCmm->u1PpduFmt = (uint8_t)(pAd->CommonCfg.HE_PpduFmt);
				*pu4ManCfgBmpCmm |= MURU_FIXED_CMM_PPDU_FMT;
			}

			if (pAd->CommonCfg.HE_OfdmaSchType) {
				pCfgCmm->u1SchType = (uint8_t)(pAd->CommonCfg.HE_OfdmaSchType);
				*pu4ManCfgBmpCmm |= MURU_FIXED_CMM_SCH_TYPE;
			}

			if (pAd->CommonCfg.HE_TrigPadding) {
				if (pAd->CommonCfg.HE_TrigPadding == 8)
					pCfgUl->u1TfPad = 1;
				else if (pAd->CommonCfg.HE_TrigPadding == 16)
					pCfgUl->u1TfPad = 2;
				else
					pCfgUl->u1TfPad = 2;

				*pu4ManCfgBmpUl |= MURU_FIXED_UL_TF_PAD;
			}

			Ret = TRUE;
		} else if (strcmp("update", param_type) == 0) {

			if (*pu4UsrList != 0) {
				MTWF_LOG(DBG_CAT_ALL,
					DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s:cmd=update, "
					"target_updated_user_bmp=0x%x, "
					"not_yet_updated_user_bmp:0x%x\n",
					__func__,
					((1 << pCfgUl->u1UserCnt) - 1),
					*pu4UsrList));
			} else {
				Ret = UniCmdMuruParameterSet(pAd, (RTMP_STRING *)&grUniMuruManCfgInf, UNI_CMD_MURU_MANUAL_CONFIG);
			}
		} else {

			param_val = rtstrchr(param_type, sep_val);

			if (param_val) {
				*param_val = 0;
				param_val++;
			}

			if (strlen(param_type) && param_val && strlen(param_val)) {
				Ret = uni_hqa_muru_parse_cmd_param_ultx(pAd, param_type, param_val, &grUniMuruManCfgInf);

				if (Ret == FALSE)
					goto error;
			}
		}
	}

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: cmd sub-group = %s, Ret = %d\n",
			 __func__, param_type, Ret));
	return Ret;
}

INT32 uni_set_muru_manual_config(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg
)
{
	INT32 Ret = TRUE;
	char sep_val = ':';
	RTMP_STRING *param_type, *param_val;

	struct UNI_MURU_DL_MANUAL_CONFIG *pCfgDl = NULL;
	struct UNI_MURU_UL_MANUAL_CONFIG *pCfgUl = NULL;
	struct UNI_MURU_CMM_MANUAL_CONFIG *pCfgCmm = NULL;
	UINT32 *pCfgBmpDl, *pCfgBmpUl, *pCfgBmpCmm;
	UINT32 *pUsrLstDl, *pUsrLstUl;

	pCfgDl = &grUniMuruManCfgInf.rCfgDl;
	pCfgUl = &grUniMuruManCfgInf.rCfgUl;
	pCfgCmm = &grUniMuruManCfgInf.rCfgCmm;
	pCfgBmpDl = &grUniMuruManCfgInf.u4ManCfgBmpDl;
	pCfgBmpUl = &grUniMuruManCfgInf.u4ManCfgBmpUl;
	pCfgBmpCmm = &grUniMuruManCfgInf.u4ManCfgBmpCmm;
	pUsrLstDl = &gu4UniMuruManCfgUsrListDl;
	pUsrLstUl = &gu4UniMuruManCfgUsrListUl;

	param_type = arg;
	if (strlen(param_type)) {
		if (strcmp("dl_init", param_type) == 0) {
			/* dl_init */
			*pUsrLstDl = 0;
			*pCfgBmpDl = 0;

			pCfgCmm->u1PpduFmt &= ~MURU_PPDU_HE_MU;
			pCfgCmm->u1SchType &= ~MURU_OFDMA_SCH_TYPE_DL;
			*pCfgBmpCmm &= ~(MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
			os_zero_mem(pCfgDl, sizeof(struct UNI_MURU_DL_MANUAL_CONFIG));

			if (pAd->CommonCfg.HE_OfdmaUserNum) {
				pCfgDl->u1UserCnt = (uint8_t)(pAd->CommonCfg.HE_OfdmaUserNum);
				*pCfgBmpDl |= MURU_FIXED_TOTAL_USER_CNT;
			}

			if (pAd->CommonCfg.HE_PpduFmt) {
				pCfgCmm->u1PpduFmt = (uint8_t)(pAd->CommonCfg.HE_PpduFmt);
				*pCfgBmpCmm |= MURU_FIXED_CMM_PPDU_FMT;
			}

			if (pAd->CommonCfg.HE_OfdmaSchType) {
				pCfgCmm->u1SchType = (uint8_t)(pAd->CommonCfg.HE_OfdmaSchType);
				*pCfgBmpCmm |= MURU_FIXED_CMM_SCH_TYPE;
			}

			Ret = TRUE;
		} else if (strcmp("ul_init", param_type) == 0) {
			/* ul_init */
			*pUsrLstUl = 0;
			*pCfgBmpUl = 0;

			pCfgCmm->u1PpduFmt &= ~MURU_PPDU_HE_TRIG;
			pCfgCmm->u1SchType &= ~MURU_OFDMA_SCH_TYPE_UL;
			*pCfgBmpCmm &= ~(MURU_FIXED_CMM_PPDU_FMT | MURU_FIXED_CMM_SCH_TYPE);
			os_zero_mem(pCfgUl, sizeof(struct UNI_MURU_UL_MANUAL_CONFIG));

			if (pAd->CommonCfg.HE_OfdmaUserNum) {
				pCfgUl->u1UserCnt = (uint8_t)(pAd->CommonCfg.HE_OfdmaUserNum);
				*pCfgBmpUl |= MURU_FIXED_UL_TOTAL_USER_CNT;
			}

			if (pAd->CommonCfg.HE_PpduFmt) {
				pCfgCmm->u1PpduFmt = (uint8_t)(pAd->CommonCfg.HE_PpduFmt);
				*pCfgBmpCmm |= MURU_FIXED_CMM_PPDU_FMT;
			}

			if (pAd->CommonCfg.HE_OfdmaSchType) {
				pCfgCmm->u1SchType = (uint8_t)(pAd->CommonCfg.HE_OfdmaSchType);
				*pCfgBmpCmm |= MURU_FIXED_CMM_SCH_TYPE;
			}

			if (pAd->CommonCfg.HE_TrigPadding) {
				if (pAd->CommonCfg.HE_TrigPadding == 8)
					pCfgUl->u1TfPad = 1;
				else if (pAd->CommonCfg.HE_TrigPadding == 16)
					pCfgUl->u1TfPad = 2;
				else
					pCfgUl->u1TfPad = 2;

				*pCfgBmpUl |= MURU_FIXED_UL_TF_PAD;
			}

			Ret = TRUE;
		} else if (strcmp("update", param_type) == 0) {
			/* update */
			Ret = UniCmdMuruParameterSet(pAd, (RTMP_STRING *)&grUniMuruManCfgInf, UNI_CMD_MURU_MANUAL_CONFIG);
		} else {

			param_val = rtstrchr(param_type, sep_val);

			if (param_val) {
				*param_val = 0;
				param_val++;
			}

			if (strlen(param_type) && param_val && strlen(param_val)) {
				Ret = uni_muru_parse_cmd_param_muru_manual_config(pAd, param_type, param_val, &grUniMuruManCfgInf);

				if (Ret == FALSE)
					goto error;
			}
		}
	}

error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: cmd sub-group = %s, Ret = %d\n",
			 __func__, param_type, Ret));
	return Ret;
}


VOID uni_muru_update_he_cfg(
	struct _RTMP_ADAPTER *pAd
)
{
	INT32 Ret = TRUE;
	struct UNI_MURU_DL_MANUAL_CONFIG *pCfgDl = &grUniMuruManCfgInf.rCfgDl;
	struct UNI_MURU_UL_MANUAL_CONFIG *pCfgUl = &grUniMuruManCfgInf.rCfgUl;
	struct UNI_MURU_CMM_MANUAL_CONFIG *pCfgCmm = &grUniMuruManCfgInf.rCfgCmm;
	UINT32 *pCfgBmpDl = &grUniMuruManCfgInf.u4ManCfgBmpDl;
	UINT32 *pCfgBmpUl = &grUniMuruManCfgInf.u4ManCfgBmpUl;
	UINT32 *pCfgBmpCmm = &grUniMuruManCfgInf.u4ManCfgBmpCmm;
	INT32 updateCfg = FALSE;

	if (pAd->CommonCfg.HE_OfdmaUserNum) {
		pCfgDl->u1UserCnt = (uint8_t)(pAd->CommonCfg.HE_OfdmaUserNum);
		*pCfgBmpDl |= MURU_FIXED_TOTAL_USER_CNT;

		pCfgUl->u1UserCnt = (uint8_t)(pAd->CommonCfg.HE_OfdmaUserNum);
		*pCfgBmpUl |= MURU_FIXED_UL_TOTAL_USER_CNT;

		updateCfg |= TRUE;
	}

	if (pAd->CommonCfg.HE_PpduFmt) {
		pCfgCmm->u1PpduFmt = (uint8_t)(pAd->CommonCfg.HE_PpduFmt);
		*pCfgBmpCmm |= MURU_FIXED_CMM_PPDU_FMT;
		updateCfg |= TRUE;
	}

	if (pAd->CommonCfg.HE_OfdmaSchType) {
		pCfgCmm->u1SchType = (uint8_t)(pAd->CommonCfg.HE_OfdmaSchType);
		*pCfgBmpCmm |= MURU_FIXED_CMM_SCH_TYPE;
		updateCfg |= TRUE;
	}

	if (pAd->CommonCfg.HE_TrigPadding) {
		if (pAd->CommonCfg.HE_TrigPadding == 8)
			pCfgUl->u1TfPad = 1;
		else if (pAd->CommonCfg.HE_TrigPadding == 16)
			pCfgUl->u1TfPad = 2;
		else
			pCfgUl->u1TfPad = 2;

		*pCfgBmpUl |= MURU_FIXED_UL_TF_PAD;
		updateCfg |= TRUE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"set muru_update_he_cfg()!!!!\n");

	if (updateCfg == TRUE) {
		if (UniCmdMuruParameterSet(pAd, (RTMP_STRING *)&grUniMuruManCfgInf, UNI_CMD_MURU_MANUAL_CONFIG) == FALSE) {
			Ret = FALSE;
			goto error;
		}
	}
error:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"%s:(Ret = %d_\n", __func__, Ret);
}

INT32 UniCmdMuruSetManualConfig(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	VOID *pHandle
)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MURU_MANUAL_CONFIG_T *pCmdManCfgCtrl = (struct UNI_CMD_MURU_MANUAL_CONFIG_T *)pHandle;
	struct UNI_MURU_MANUAL_CONFIG_T *pMuruManCfg = (struct UNI_MURU_MANUAL_CONFIG_T *)arg;

	pCmdManCfgCtrl->u2Tag = UNI_CMD_MURU_MANUAL_CONFIG;
	pCmdManCfgCtrl->u2Length = sizeof(*pCmdManCfgCtrl);
	pCmdManCfgCtrl->u1CmdVersion = UNI_CMD_MURU_VER_HE;

	memcpy(&pCmdManCfgCtrl->rMuruManCfg, pMuruManCfg, sizeof(*pMuruManCfg));

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;

}

static INT32 UniCmdMuruSetMuDlAckPolicy(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	VOID * pHandle
)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MURU_SET_POLICY_TYPE_T *pCmdMuruPolicyCtrl = (struct UNI_CMD_MURU_SET_POLICY_TYPE_T *)pHandle;
	UINT8 *policy_type = (UINT8 *)arg;

	pCmdMuruPolicyCtrl->u1AckPolicy = *policy_type;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: %d\n", __func__, pCmdMuruPolicyCtrl->u1AckPolicy));

	pCmdMuruPolicyCtrl->u2Tag = UNI_CMD_MURU_SET_MUDL_ACK_POLICY;
	pCmdMuruPolicyCtrl->u2Length = sizeof(*pCmdMuruPolicyCtrl);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static INT32 UniCmdMuruSetTrigType(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	VOID * pHandle
)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MURU_SET_TRIG_TYPE_T *pCmdMuruTrigCtrl = (struct UNI_CMD_MURU_SET_TRIG_TYPE_T *)pHandle;
	UINT8 *trig_type = (UINT8 *)arg;

	pCmdMuruTrigCtrl->u1TrigValue = *trig_type;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: %d\n", __func__, pCmdMuruTrigCtrl->u1TrigValue));

	pCmdMuruTrigCtrl->u2Tag = UNI_CMD_MURU_SET_TRIG_TYPE;
	pCmdMuruTrigCtrl->u2Length = sizeof(*pCmdMuruTrigCtrl);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static INT32 UniCmdMuruSet20MDynAlgo(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	VOID * pHandle
)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MURU_SET_20M_DYN_ALGO_T *pCmdMuru20MDynCtrl = (struct UNI_CMD_MURU_SET_20M_DYN_ALGO_T *)pHandle;
	PCHAR pch = NULL;

	pch = strsep(&arg, "-");

	if (pch != NULL)
		pCmdMuru20MDynCtrl->u1DynAlgoEnable = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: %d\n", __func__, pCmdMuru20MDynCtrl->u1DynAlgoEnable));

	pCmdMuru20MDynCtrl->u2Tag = UNI_CMD_MURU_SET_20M_DYN_ALGO;
	pCmdMuru20MDynCtrl->u2Length = sizeof(*pCmdMuru20MDynCtrl);

error:

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static INT32 UniCmdMuruProtFrameThr(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	VOID * pHandle
)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MURU_SET_PROT_FRAME_THR_T *pCmdMuruProtFrameThrCtrl = (struct UNI_CMD_MURU_SET_PROT_FRAME_THR_T *)pHandle;
	PCHAR pch = NULL;

	pch = strsep(&arg, "-");

	if (pch != NULL)
		pCmdMuruProtFrameThrCtrl->u4ProtFrameThr = os_str_tol(pch, 0, 10);
#ifdef RT_BIG_ENDIAN
		pCmdMuruProtFrameThrCtrl->u4ProtFrameThr
			= cpu2le32(pCmdMuruProtFrameThrCtrl->u4ProtFrameThr);
#endif
	else {
		Ret = 0;
		goto error;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: %d\n", __func__, pCmdMuruProtFrameThrCtrl->u4ProtFrameThr));

	pCmdMuruProtFrameThrCtrl->u2Tag = UNI_CMD_MURU_PROT_FRAME_THR;
	pCmdMuruProtFrameThrCtrl->u2Length = sizeof(*pCmdMuruProtFrameThrCtrl);

error:

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static INT32 UniCmdMuruSetTxopOnOff(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	VOID * pHandle
)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MURU_SET_TXOP_ONOFF_T *pCmdMuruTxopCtrl = (struct UNI_CMD_MURU_SET_TXOP_ONOFF_T *)pHandle;
	PCHAR pch = NULL;

	pch = strsep(&arg, "-");

	if (pch != NULL)
		pCmdMuruTxopCtrl->u1TxopOnOff = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		goto error;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: %d\n", __func__, pCmdMuruTxopCtrl->u1TxopOnOff));

	pCmdMuruTxopCtrl->u2Tag = UNI_CMD_MURU_SET_TXOP_ONOFF;
	pCmdMuruTxopCtrl->u2Length = sizeof(*pCmdMuruTxopCtrl);

error:

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static INT32 UniCmdMuruSetUlOnOff(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	VOID * pHandle
)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MURU_UL_ONOFF_T *pCmdMuruUlCtrl = (struct UNI_CMD_MURU_UL_ONOFF_T *)pHandle;
	PCHAR pch = NULL;

	pch = strsep(&arg, "-");

	if (pch != NULL)
		pCmdMuruUlCtrl->u2UlBsrpOnOff = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("invalid format, iwpriv ra0 set set_muru_ul_off=[bsrp]-[data]\n"));
		goto error;
	}

	pch = strsep(&arg, "-");

	if (pch != NULL)
		pCmdMuruUlCtrl->u2UlDataOnOff = os_str_tol(pch, 0, 10);
	else {
		Ret = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("invalid format, iwpriv ra0 set set_muru_ul_off=[bsrp]-[data]\n"));
		goto error;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: bsrp=%d data=%d\n", __func__,
		pCmdMuruUlCtrl->u2UlBsrpOnOff, pCmdMuruUlCtrl->u2UlDataOnOff));

	pCmdMuruUlCtrl->u2Tag = UNI_CMD_MURU_UL_ONOFF;
	pCmdMuruUlCtrl->u2Length = sizeof(*pCmdMuruUlCtrl);

error:

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static UNI_CMD_TAG_HANDLE_T UniCmdSetMuruTab[UNI_CMD_MURU_MAX] = {
	{
		.u2CmdFeature = UNI_CMD_MURU_BSRP_CTRL,
		.u4StructSize = sizeof(struct UNI_CMD_MURU_BSRP_CTRL_T),
		.pfHandler = UniCmdMuruBsrpCtrl
	},
	{
		.u2CmdFeature = UNI_CMD_MURU_SET_ARB_OP_MODE,
		.u4StructSize = sizeof(struct UNI_CMD_MURU_SET_ARB_OP_MODE_T),
		.pfHandler = UniCmdMuruSetArbOpMode
	},
	{
		.u2CmdFeature = UNI_CMD_MURU_SUTX_CTRL,
		.u4StructSize = sizeof(struct UNI_CMD_MURU_SET_SUTX_T),
		.pfHandler = UniCmdMuruSuTxCtrl
	},
	{
		.u2CmdFeature = UNI_CMD_MURU_FIXED_RATE_CTRL,
		.u4StructSize = sizeof(struct UNI_CMD_MURU_FIXED_RATE_CTRL_T),
		.pfHandler = UniCmdMuruSetFixedRate
	},
	{
		.u2CmdFeature = UNI_CMD_MURU_FIXED_GROUP_RATE_CTRL,
		.u4StructSize = sizeof(struct UNI_CMD_MURU_FIXED_GRP_RATE_CTRL_T),
		.pfHandler = UniCmdMuruSetFixedGroupRate
	},
	{
		.u2CmdFeature = UNI_CMD_MURU_DBG_INFO,
		.u4StructSize = sizeof(struct UNI_CMD_MURU_SET_DBG_INFO),
		.pfHandler = UniCmdMuruSetDbgInfo
	},
	{
		.u2CmdFeature = UNI_CMD_MURU_MANUAL_CONFIG,
		.u4StructSize = sizeof(struct UNI_CMD_MURU_MANUAL_CONFIG_T),
		.pfHandler = UniCmdMuruSetManualConfig
	},
	{
		.u2CmdFeature = UNI_CMD_MURU_SET_MUDL_ACK_POLICY,
		.u4StructSize = sizeof(struct UNI_CMD_MURU_SET_POLICY_TYPE_T),
		.pfHandler = UniCmdMuruSetMuDlAckPolicy
	},
	{
		.u2CmdFeature = UNI_CMD_MURU_SET_TRIG_TYPE,
		.u4StructSize = sizeof(struct UNI_CMD_MURU_SET_TRIG_TYPE_T),
		.pfHandler = UniCmdMuruSetTrigType
	},
	{
		.u2CmdFeature = UNI_CMD_MURU_SET_20M_DYN_ALGO,
		.u4StructSize = sizeof(struct UNI_CMD_MURU_SET_20M_DYN_ALGO_T),
		.pfHandler = UniCmdMuruSet20MDynAlgo
	},
	{
		.u2CmdFeature = UNI_CMD_MURU_PROT_FRAME_THR,
		.u4StructSize = sizeof(struct UNI_CMD_MURU_SET_PROT_FRAME_THR_T),
		.pfHandler = UniCmdMuruProtFrameThr
	},
	{
		.u2CmdFeature = UNI_CMD_MURU_SET_TXOP_ONOFF,
		.u4StructSize = sizeof(struct UNI_CMD_MURU_SET_TXOP_ONOFF_T),
		.pfHandler = UniCmdMuruSetTxopOnOff
	},
	{
		.u2CmdFeature = UNI_CMD_MURU_UL_ONOFF,
		.u4StructSize = sizeof(struct UNI_CMD_MURU_UL_ONOFF_T),
		.pfHandler = UniCmdMuruSetUlOnOff
	},
};

INT32 UniCmdMuruParameterSet(struct _RTMP_ADAPTER *pAd, IN RTMP_STRING *arg, UINT32 u4EnableFeature)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = NDIS_STATUS_SUCCESS;
	UINT32 i = 0;
	UINT16 ucTLVNumber = 0;
	UINT8 *pTempBuf = NULL;
	UINT8 *pNextHeadBuf = NULL;
	UINT32 u4CmdNeedMaxBufSize = 0;
	UINT32 u4RealUseBufSize = 0;
	UINT32 u4SendBufSize = 0;
	UINT32 u4RemainingPayloadSize = 0;
	UINT32 u4ComCmdSize = 0;
	struct UNI_CMD_MURU_T *pCmdMuruUpdate = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(struct UNI_CMD_MURU_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < UNI_CMD_MURU_MAX; i++) {
		if (UniCmdSetMuruTab[i].u2CmdFeature == u4EnableFeature) {
			u4CmdNeedMaxBufSize += UniCmdSetMuruTab[i].u4StructSize;
			break;
		}
	}

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdMuruUpdate = (struct UNI_CMD_MURU_T *)pNextHeadBuf;
	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Fill params of  supported feature */
	if (UniCmdSetMuruTab[i].pfHandler != NULL) {
		Ret = ((PFN_MURU_HANDLE)(UniCmdSetMuruTab[i].pfHandler))(pAd, arg, pNextHeadBuf);
		if (Ret == NDIS_STATUS_SUCCESS) {
			pNextHeadBuf += UniCmdSetMuruTab[i].u4StructSize;
			ucTLVNumber++;
		}
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
		 ucTLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN bNeedFrag = FALSE;
		BOOLEAN bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_MURU);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if (!bNeedFrag || bLastFrag) {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
				SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
			} else {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			Ret = chip_cmd_tx(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static INT32 UniCmdMuruShowUlRuStatus(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg,
	VOID * pHandle
)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_MURU_SHOW_ULRU_STATUS_T *pCmdMuruUlRuStatusCtrl = (struct UNI_CMD_MURU_SHOW_ULRU_STATUS_T *)pHandle;

	pCmdMuruUlRuStatusCtrl->u2Tag = UNI_CMD_MURU_DBG_INFO;
	pCmdMuruUlRuStatusCtrl->u2Length = sizeof(*pCmdMuruUlRuStatusCtrl);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}

static UNI_CMD_TAG_HANDLE_T UniCmdGetMuruTab[UNI_CMD_MURU_MAX] = {
	{
		.u2CmdFeature = UNI_CMD_MURU_DBG_INFO,
		.u4StructSize = sizeof(struct UNI_CMD_MURU_SHOW_ULRU_STATUS_T),
		.pfHandler = UniCmdMuruShowUlRuStatus
	},
};

INT32 UniCmdMuruParameterGet(struct _RTMP_ADAPTER *pAd, IN RTMP_STRING *arg, UINT32 u4EnableFeature)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = NDIS_STATUS_SUCCESS;
	UINT32 i = 0;
	UINT16 ucTLVNumber = 0;
	UINT8 *pTempBuf = NULL;
	UINT8 *pNextHeadBuf = NULL;
	UINT32 u4CmdNeedMaxBufSize = 0;
	UINT32 u4RealUseBufSize = 0;
	UINT32 u4SendBufSize = 0;
	UINT32 u4RemainingPayloadSize = 0;
	UINT32 u4ComCmdSize = 0;
	struct UNI_CMD_MURU_T *pCmdMuruUpdate = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(struct UNI_CMD_MURU_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < UNI_CMD_MURU_MAX; i++) {
		if (UniCmdGetMuruTab[i].u2CmdFeature == u4EnableFeature) {
			u4CmdNeedMaxBufSize += UniCmdGetMuruTab[i].u4StructSize;
			break;
		}
	}

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdMuruUpdate = (struct UNI_CMD_MURU_T *)pNextHeadBuf;
	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Fill params of  supported feature */
	if (UniCmdGetMuruTab[i].pfHandler != NULL) {
		Ret = ((PFN_MURU_HANDLE)(UniCmdGetMuruTab[i].pfHandler))(pAd, arg, pNextHeadBuf);
		if (Ret == NDIS_STATUS_SUCCESS) {
			pNextHeadBuf += UniCmdGetMuruTab[i].u4StructSize;
			ucTLVNumber++;
		}
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		" TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
		ucTLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN bNeedFrag = FALSE;
		BOOLEAN bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_MURU);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if (!bNeedFrag || bLastFrag) {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_WAIT_RETRY_RSP);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
				SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
			} else {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_RETRY);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			Ret = chip_cmd_tx(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}


#ifdef CFG_SUPPORT_FALCON_PP
static INT32 UniCmdPPEnCtrl(struct _RTMP_ADAPTER *pAd, P_PP_CMD_T pp_cmd_cap, VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_PP_EN_CTRL_T pPPEnCtrl = (P_UNI_CMD_PP_EN_CTRL_T)pHandle;

	pPPEnCtrl->u2Tag = UNI_CMD_PP_EN_CTRL;
	pPPEnCtrl->u2Length = sizeof(UNI_CMD_PP_EN_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pPPEnCtrl->u2Tag = cpu2le16(pPPEnCtrl->u2Tag);
	pPPEnCtrl->u2Length = cpu2le16(pPPEnCtrl->u2Length);
#endif /* RT_BIG_ENDIAN */
	pPPEnCtrl->u1DbdcIdx = pp_cmd_cap->dbdc_idx;
	pPPEnCtrl->u1PpCtrl = pp_cmd_cap->pp_ctrl;
	pPPEnCtrl->u1PpAutoMode = pp_cmd_cap->pp_auto_mode;

	return Ret;
}

static UNI_CMD_TAG_HANDLE_T UniCmdPPCapCtrlTab[PP_CMD_NUM] = {
	{
		.u2CmdFeature = PP_CMD_Reserve,
		.u4StructSize = 0,
		.pfHandler = NULL
	},
	{
		.u2CmdFeature = PP_CMD_SET_PP_CAP_CTRL,
		.u4StructSize = sizeof(UNI_CMD_PP_EN_CTRL_T),
		.pfHandler = UniCmdPPEnCtrl
	},
};

INT32 UniCmdPPCapCtrl(struct _RTMP_ADAPTER *pAd, P_PP_CMD_T pp_cmd_cap)
{
	struct cmd_msg			*msg = NULL;
	INT32					Ret = NDIS_STATUS_SUCCESS;
	UINT32					i = 0;
	UINT16					u2TLVNumber = 0;
	PUCHAR					pTempBuf = NULL;
	PUCHAR					pNextHeadBuf = NULL;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4RealUseBufSize = 0;
	UINT32					u4SendBufSize = 0;
	UINT32					u4RemainingPayloadSize = 0;
	UINT32					u4ComCmdSize = 0;
	P_UNI_CMD_PP_T			pCmdPP = NULL;
	RTMP_CHIP_CAP			*cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_PP_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < PP_CMD_NUM; i++) {
		if (pp_cmd_cap->cmd_sub_id == UniCmdPPCapCtrlTab[i].u2CmdFeature)
			u4CmdNeedMaxBufSize += UniCmdPPCapCtrlTab[i].u4StructSize;
	}

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdPP = (P_UNI_CMD_PP_T)pNextHeadBuf;
	/* Nothing to do */
	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Traverse all support features */
	for (i = 0; i < PP_CMD_NUM; i++) {
		if (pp_cmd_cap->cmd_sub_id == UniCmdPPCapCtrlTab[i].u2CmdFeature) {
			switch (UniCmdPPCapCtrlTab[i].u2CmdFeature) {
			case PP_CMD_SET_PP_CAP_CTRL:
				if (UniCmdPPCapCtrlTab[i].pfHandler != NULL) {
					Ret = ((PFN_PP_EN_CTRL_HANDLE)(UniCmdPPCapCtrlTab[i].pfHandler))(pAd, pp_cmd_cap, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdPPCapCtrlTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			default:
				Ret = NDIS_STATUS_SUCCESS;
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s(): The hanlder of tag (0x%08x) not support!\n", __func__, UniCmdPPCapCtrlTab[i].u2CmdFeature);
				break;
			}

			if (Ret != NDIS_STATUS_SUCCESS)
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"The hanlder of tag (0x%08x) return fail!\n", UniCmdPPCapCtrlTab[i].u2CmdFeature);
		}
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN bNeedFrag = FALSE;
		BOOLEAN bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_PP);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if (!bNeedFrag || bLastFrag) {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
				SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
			} else {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			Ret = chip_cmd_tx(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;
}
#endif /* CFG_SUPPORT_FALCON_PP */

static INT32 UniCmdTPCManCtrlHook(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_TPC_MAN_CTRL_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_TPC_MAN_CTRL_T pTPCManCtrl = (P_UNI_CMD_TPC_MAN_CTRL_T)pHandle;

	pTPCManCtrl->u2Tag = UNI_CMD_TPC_ACT_MANUAL_MODE;
	pTPCManCtrl->u2Length = sizeof(UNI_CMD_TPC_MAN_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pTPCManCtrl->u2Tag = cpu2le16(pTPCManCtrl->u2Tag);
	pTPCManCtrl->u2Length = cpu2le16(pTPCManCtrl->u2Length);
#endif /* RT_BIG_ENDIAN */
	pTPCManCtrl->u1TpcCtrlFormatId = pParam->u1TpcCtrlFormatId;
	pTPCManCtrl->fgTpcEnable = pParam->fgTpcEnable;
	pTPCManCtrl->eTpcParamMode = pParam->eTpcParamMode;

	return Ret;
}

static INT32 UniCmdTPCUlAlgoCtrlHook(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_TPC_UL_ALGO_CTRL_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_TPC_UL_ALGO_CTRL_T pTPCUlAlgoCtrl = (P_UNI_CMD_TPC_UL_ALGO_CTRL_T)pHandle;

	pTPCUlAlgoCtrl->u2Tag = UNI_CMD_TPC_ACT_UL_TX_POWER_CONFIG;
	pTPCUlAlgoCtrl->u2Length = sizeof(UNI_CMD_TPC_UL_ALGO_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pTPCUlAlgoCtrl->u2Tag = cpu2le16(pTPCUlAlgoCtrl->u2Tag);
	pTPCUlAlgoCtrl->u2Length = cpu2le16(pTPCUlAlgoCtrl->u2Length);
#endif /* RT_BIG_ENDIAN */
	pTPCUlAlgoCtrl->u1TpcCtrlFormatId = pParam->u1TpcCtrlFormatId;
	pTPCUlAlgoCtrl->u1ApTxPwr = pParam->u1ApTxPwr;
	pTPCUlAlgoCtrl->u1EntryIdx = pParam->u1EntryIdx;
	pTPCUlAlgoCtrl->u1TargetRssi = pParam->u1TargetRssi;
	pTPCUlAlgoCtrl->u1UPH = pParam->u1UPH;
	pTPCUlAlgoCtrl->fgMinPwrFlag = pParam->fgMinPwrFlag;
	
	return Ret;
}

static INT32 UniCmdTPCUlTargetRSSIConfigHook(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_TPC_UL_ALGO_CTRL_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_TPC_UL_ALGO_CTRL_T pTPCUlAlgoCtrl = (P_UNI_CMD_TPC_UL_ALGO_CTRL_T)pHandle;

	pTPCUlAlgoCtrl->u2Tag = UNI_CMD_TPC_ACT_UL_TARGET_RSSI_CONFIG;
	pTPCUlAlgoCtrl->u2Length = sizeof(UNI_CMD_TPC_UL_ALGO_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pTPCUlAlgoCtrl->u2Tag = cpu2le16(pTPCUlAlgoCtrl->u2Tag);
	pTPCUlAlgoCtrl->u2Length = cpu2le16(pTPCUlAlgoCtrl->u2Length);
#endif /* RT_BIG_ENDIAN */
	pTPCUlAlgoCtrl->u1TpcCtrlFormatId = pParam->u1TpcCtrlFormatId;
	pTPCUlAlgoCtrl->u1ApTxPwr = pParam->u1ApTxPwr;
	pTPCUlAlgoCtrl->u1EntryIdx = pParam->u1EntryIdx;
	pTPCUlAlgoCtrl->u1TargetRssi = pParam->u1TargetRssi;
	pTPCUlAlgoCtrl->u1UPH = pParam->u1UPH;
	pTPCUlAlgoCtrl->fgMinPwrFlag = pParam->fgMinPwrFlag;
	
	return Ret;
}

static INT32 UniCmdTPCUlUPHMinPwrFGConfigHook(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_TPC_UL_ALGO_CTRL_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_TPC_UL_ALGO_CTRL_T pTPCUlAlgoCtrl = (P_UNI_CMD_TPC_UL_ALGO_CTRL_T)pHandle;

	pTPCUlAlgoCtrl->u2Tag = UNI_CMD_TPC_ACT_UL_UPH_MIN_PWR_FG_CONFIG;
	pTPCUlAlgoCtrl->u2Length = sizeof(UNI_CMD_TPC_UL_ALGO_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pTPCUlAlgoCtrl->u2Tag = cpu2le16(pTPCUlAlgoCtrl->u2Tag);
	pTPCUlAlgoCtrl->u2Length = cpu2le16(pTPCUlAlgoCtrl->u2Length);
#endif /* RT_BIG_ENDIAN */
	pTPCUlAlgoCtrl->u1TpcCtrlFormatId = pParam->u1TpcCtrlFormatId;
	pTPCUlAlgoCtrl->u1ApTxPwr = pParam->u1ApTxPwr;
	pTPCUlAlgoCtrl->u1EntryIdx = pParam->u1EntryIdx;
	pTPCUlAlgoCtrl->u1TargetRssi = pParam->u1TargetRssi;
	pTPCUlAlgoCtrl->u1UPH = pParam->u1UPH;
	pTPCUlAlgoCtrl->fgMinPwrFlag = pParam->fgMinPwrFlag;

	return Ret;
}

static INT32 UniCmdTPCDlTxPwrCmdCtrlConfigHook(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_TPC_DL_ALGO_CTRL_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_TPC_DL_ALGO_CTRL_T pTPCDlAlgoCtrl = (P_UNI_CMD_TPC_DL_ALGO_CTRL_T)pHandle;

	pTPCDlAlgoCtrl->u2Tag = UNI_CMD_TPC_ACT_DL_TX_POWER_CMD_CTRL_CONFIG;
	pTPCDlAlgoCtrl->u2Length = sizeof(UNI_CMD_TPC_DL_ALGO_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pTPCDlAlgoCtrl->u2Tag = cpu2le16(pTPCDlAlgoCtrl->u2Tag);
	pTPCDlAlgoCtrl->u2Length = cpu2le16(pTPCDlAlgoCtrl->u2Length);
#endif /* RT_BIG_ENDIAN */
	pTPCDlAlgoCtrl->u1TpcCtrlFormatId = pParam->u1TpcCtrlFormatId;
	pTPCDlAlgoCtrl->i1DlTxPwr = pParam->i1DlTxPwr;
	pTPCDlAlgoCtrl->fgDlTxPwrCmdCtrl = pParam->fgDlTxPwrCmdCtrl;
	pTPCDlAlgoCtrl->u1EntryIdx = pParam->u1EntryIdx;
	pTPCDlAlgoCtrl->i2DlTxPwrAlpha = cpu2le16(pParam->i2DlTxPwrAlpha);
	pTPCDlAlgoCtrl->eTpcDlTxType = pParam->eTpcDlTxType;

	return Ret;
}

static INT32 UniCmdTPCDlTxPwrConfigHook(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_TPC_DL_ALGO_CTRL_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_TPC_DL_ALGO_CTRL_T pTPCDlAlgoCtrl = (P_UNI_CMD_TPC_DL_ALGO_CTRL_T)pHandle;

	pTPCDlAlgoCtrl->u2Tag = UNI_CMD_TPC_ACT_DL_TX_POWER_CONFIG;
	pTPCDlAlgoCtrl->u2Length = sizeof(UNI_CMD_TPC_DL_ALGO_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pTPCDlAlgoCtrl->u2Tag = cpu2le16(pTPCDlAlgoCtrl->u2Tag);
	pTPCDlAlgoCtrl->u2Length = cpu2le16(pTPCDlAlgoCtrl->u2Length);
#endif /* RT_BIG_ENDIAN */
	pTPCDlAlgoCtrl->u1TpcCtrlFormatId = pParam->u1TpcCtrlFormatId;
	pTPCDlAlgoCtrl->i1DlTxPwr = pParam->i1DlTxPwr;
	pTPCDlAlgoCtrl->fgDlTxPwrCmdCtrl = pParam->fgDlTxPwrCmdCtrl;
	pTPCDlAlgoCtrl->u1EntryIdx = pParam->u1EntryIdx;
	pTPCDlAlgoCtrl->i2DlTxPwrAlpha = cpu2le16(pParam->i2DlTxPwrAlpha);
	pTPCDlAlgoCtrl->eTpcDlTxType = pParam->eTpcDlTxType;

	return Ret;
}

static INT32 UniCmdTPCDlTxPwrAlphaConfigHook(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_TPC_DL_ALGO_CTRL_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_TPC_DL_ALGO_CTRL_T pTPCDlAlgoCtrl = (P_UNI_CMD_TPC_DL_ALGO_CTRL_T)pHandle;

	pTPCDlAlgoCtrl->u2Tag = UNI_CMD_TPC_ACT_DL_TX_POWER_ALPHA_CONFIG;
	pTPCDlAlgoCtrl->u2Length = sizeof(UNI_CMD_TPC_DL_ALGO_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pTPCDlAlgoCtrl->u2Tag = cpu2le16(pTPCDlAlgoCtrl->u2Tag);
	pTPCDlAlgoCtrl->u2Length = cpu2le16(pTPCDlAlgoCtrl->u2Length);
#endif /* RT_BIG_ENDIAN */
	pTPCDlAlgoCtrl->u1TpcCtrlFormatId = pParam->u1TpcCtrlFormatId;
	pTPCDlAlgoCtrl->i1DlTxPwr = pParam->i1DlTxPwr;
	pTPCDlAlgoCtrl->fgDlTxPwrCmdCtrl = pParam->fgDlTxPwrCmdCtrl;
	pTPCDlAlgoCtrl->u1EntryIdx = pParam->u1EntryIdx;
	pTPCDlAlgoCtrl->i2DlTxPwrAlpha = cpu2le16(pParam->i2DlTxPwrAlpha);
	pTPCDlAlgoCtrl->eTpcDlTxType = pParam->eTpcDlTxType;

	return Ret;
}

static INT32 UniCmdTPCManTblInfoHook(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_TPC_MAN_TBL_INFO_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_TPC_MAN_TBL_INFO_T pTPCManTblInfo = (P_UNI_CMD_TPC_MAN_TBL_INFO_T)pHandle;

	pTPCManTblInfo->u2Tag = UNI_CMD_TPC_ACT_MAN_TBL_INFO;
	pTPCManTblInfo->u2Length = sizeof(UNI_CMD_TPC_MAN_TBL_INFO_T);
#ifdef RT_BIG_ENDIAN
	pTPCManTblInfo->u2Tag = cpu2le16(pTPCManTblInfo->u2Tag);
	pTPCManTblInfo->u2Length = cpu2le16(pTPCManTblInfo->u2Length);
#endif /* RT_BIG_ENDIAN */
	pTPCManTblInfo->u1TpcCtrlFormatId = pParam->u1TpcCtrlFormatId;
	pTPCManTblInfo->fgUplink = pParam->fgUplink;

	return Ret;
}

static INT32 UniCmdTPCManWlanIDCtrlHook(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_TPC_MAN_WLAN_ID_CTRL_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_TPC_MAN_WLAN_ID_CTRL_T pTPCManWlanIDCtrl = (P_UNI_CMD_TPC_MAN_WLAN_ID_CTRL_T)pHandle;

	pTPCManWlanIDCtrl->u2Tag = UNI_CMD_TPC_ACT_WLANID_CTRL;
	pTPCManWlanIDCtrl->u2Length = sizeof(UNI_CMD_TPC_MAN_WLAN_ID_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pTPCManWlanIDCtrl->u2Tag = cpu2le16(pTPCManWlanIDCtrl->u2Tag);
	pTPCManWlanIDCtrl->u2Length = cpu2le16(pTPCManWlanIDCtrl->u2Length);
#endif /* RT_BIG_ENDIAN */
	pTPCManWlanIDCtrl->u1TpcCtrlFormatId = pParam->u1TpcCtrlFormatId;
	pTPCManWlanIDCtrl->u1EntryIdx = pParam->u1EntryIdx;
	pTPCManWlanIDCtrl->u2WlanId = cpu2le16(pParam->u2WlanId);
	pTPCManWlanIDCtrl->fgUplink = pParam->fgUplink;
	pTPCManWlanIDCtrl->eTpcDlTxType = pParam->eTpcDlTxType;

	return Ret;
}

static INT32 UniCmdTPCUlUnitTestConfigHook(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_TPC_UL_UT_VAR_CFG_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_TPC_UL_UT_VAR_CFG_T pTPCUlUnitTestConfig = (P_UNI_CMD_TPC_UL_UT_VAR_CFG_T)pHandle;

	pTPCUlUnitTestConfig->u2Tag = UNI_CMD_TPC_ACT_UL_UNIT_TEST_CONFIG;
	pTPCUlUnitTestConfig->u2Length = sizeof(UNI_CMD_TPC_UL_UT_VAR_CFG_T);
#ifdef RT_BIG_ENDIAN
	pTPCUlUnitTestConfig->u2Tag = cpu2le16(pTPCUlUnitTestConfig->u2Tag);
	pTPCUlUnitTestConfig->u2Length = cpu2le16(pTPCUlUnitTestConfig->u2Length);
#endif /* RT_BIG_ENDIAN */
	pTPCUlUnitTestConfig->u1TpcCtrlFormatId = pParam->u1TpcCtrlFormatId;
	pTPCUlUnitTestConfig->u1EntryIdx = pParam->u1EntryIdx;
	pTPCUlUnitTestConfig->i2Value = cpu2le16(pParam->i2Value);
	pTPCUlUnitTestConfig->u1VarType = pParam->u1VarType;

	return Ret;
}

static INT32 UniCmdTPCUlUnitTestGoHook(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_TPC_UL_UT_CTRL_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_TPC_UL_UT_CTRL_T pTPCUlUnitTestGo = (P_UNI_CMD_TPC_UL_UT_CTRL_T)pHandle;

	pTPCUlUnitTestGo->u2Tag = UNI_CMD_TPC_ACT_UL_UNIT_TEST_GO;
	pTPCUlUnitTestGo->u2Length = sizeof(UNI_CMD_TPC_UL_UT_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pTPCUlUnitTestGo->u2Tag = cpu2le16(pTPCUlUnitTestGo->u2Tag);
	pTPCUlUnitTestGo->u2Length = cpu2le16(pTPCUlUnitTestGo->u2Length);
#endif /* RT_BIG_ENDIAN */
	pTPCUlUnitTestGo->u1TpcCtrlFormatId = pParam->u1TpcCtrlFormatId;
	pTPCUlUnitTestGo->fgTpcUtGo = pParam->fgTpcUtGo;

	return Ret;
}

static INT32 UniCmdTPCEnableConfigHook(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_TPC_MAN_CTRL_T pParam,
	VOID *pHandle)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_TPC_MAN_CTRL_T pTPCManCtrl = (P_UNI_CMD_TPC_MAN_CTRL_T)pHandle;

	pTPCManCtrl->u2Tag = UNI_CMD_TPC_ACT_ENABLE_CONFIG;
	pTPCManCtrl->u2Length = sizeof(UNI_CMD_TPC_MAN_CTRL_T);
#ifdef RT_BIG_ENDIAN
	pTPCManCtrl->u2Tag = cpu2le16(pTPCManCtrl->u2Tag);
	pTPCManCtrl->u2Length = cpu2le16(pTPCManCtrl->u2Length);
#endif /* RT_BIG_ENDIAN */
	pTPCManCtrl->u1TpcCtrlFormatId = pParam->u1TpcCtrlFormatId;
	pTPCManCtrl->fgTpcEnable = pParam->fgTpcEnable;
	pTPCManCtrl->eTpcParamMode = pParam->eTpcParamMode;

	return Ret;
}

static UNI_CMD_TAG_HANDLE_T UniCmdTPCTab[UNI_CMD_TPC_ALGO_ACTION_NUM] = {
	{
		.u2CmdFeature = UNI_CMD_TPC_ACT_MANUAL_MODE,
		.u4StructSize = sizeof(UNI_CMD_TPC_MAN_CTRL_T),
		.pfHandler = UniCmdTPCManCtrlHook
	},
	{
		.u2CmdFeature = UNI_CMD_TPC_ACT_UL_TX_POWER_CONFIG,
		.u4StructSize = sizeof(UNI_CMD_TPC_UL_ALGO_CTRL_T),
		.pfHandler = UniCmdTPCUlAlgoCtrlHook
	},
	{
		.u2CmdFeature = UNI_CMD_TPC_ACT_UL_TARGET_RSSI_CONFIG,
		.u4StructSize = sizeof(UNI_CMD_TPC_UL_ALGO_CTRL_T),
		.pfHandler = UniCmdTPCUlTargetRSSIConfigHook
	},
	{
		.u2CmdFeature = UNI_CMD_TPC_ACT_UL_UPH_MIN_PWR_FG_CONFIG,
		.u4StructSize = sizeof(UNI_CMD_TPC_UL_ALGO_CTRL_T),
		.pfHandler = UniCmdTPCUlUPHMinPwrFGConfigHook
	},
	{
		.u2CmdFeature = UNI_CMD_TPC_ACT_DL_TX_POWER_CMD_CTRL_CONFIG,
		.u4StructSize = sizeof(UNI_CMD_TPC_DL_ALGO_CTRL_T),
		.pfHandler = UniCmdTPCDlTxPwrCmdCtrlConfigHook
	},
	{
		.u2CmdFeature = UNI_CMD_TPC_ACT_DL_TX_POWER_CONFIG,
		.u4StructSize = sizeof(UNI_CMD_TPC_DL_ALGO_CTRL_T),
		.pfHandler = UniCmdTPCDlTxPwrConfigHook
	},
	{
		.u2CmdFeature = UNI_CMD_TPC_ACT_DL_TX_POWER_ALPHA_CONFIG,
		.u4StructSize = sizeof(UNI_CMD_TPC_DL_ALGO_CTRL_T),
		.pfHandler = UniCmdTPCDlTxPwrAlphaConfigHook
	},
	{
		.u2CmdFeature = UNI_CMD_TPC_ACT_MAN_TBL_INFO,
		.u4StructSize = sizeof(UNI_CMD_TPC_MAN_TBL_INFO_T),
		.pfHandler = UniCmdTPCManTblInfoHook
	},
	{
		.u2CmdFeature = UNI_CMD_TPC_ACT_WLANID_CTRL,
		.u4StructSize = sizeof(UNI_CMD_TPC_MAN_WLAN_ID_CTRL_T),
		.pfHandler = UniCmdTPCManWlanIDCtrlHook
	},
	{
		.u2CmdFeature = UNI_CMD_TPC_ACT_UL_UNIT_TEST_CONFIG,
		.u4StructSize = sizeof(UNI_CMD_TPC_UL_UT_VAR_CFG_T),
		.pfHandler = UniCmdTPCUlUnitTestConfigHook
	},
	{
		.u2CmdFeature = UNI_CMD_TPC_ACT_UL_UNIT_TEST_GO,
		.u4StructSize = sizeof(UNI_CMD_TPC_UL_UT_CTRL_T),
		.pfHandler = UniCmdTPCUlUnitTestGoHook
	},
	{
		.u2CmdFeature = UNI_CMD_TPC_ACT_ENABLE_CONFIG,
		.u4StructSize = sizeof(UNI_CMD_TPC_MAN_CTRL_T),
		.pfHandler = UniCmdTPCEnableConfigHook
	},
};

INT32 UniCmdTPC(struct _RTMP_ADAPTER *pAd, P_UNI_CMD_TPC_PARAM_T pParamCtrl)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = NDIS_STATUS_SUCCESS;
	UINT32 i = 0;
	UINT16 u2TLVNumber = 0;
	PUCHAR pTempBuf = NULL;
	PUCHAR pNextHeadBuf = NULL;
	UINT32 u4CmdNeedMaxBufSize = 0;
	UINT32 u4RealUseBufSize = 0;
	UINT32 u4SendBufSize = 0;
	UINT32 u4RemainingPayloadSize = 0;
	UINT32 u4ComCmdSize = 0;
	P_UNI_CMD_TPC_T	pCmdTPC = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_TPC_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < UNI_CMD_TPC_ALGO_ACTION_NUM; i++) {
		if (pParamCtrl->TPCTagValid[i])
			u4CmdNeedMaxBufSize += UniCmdTPCTab[i].u4StructSize;
	}

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pCmdTPC = (P_UNI_CMD_TPC_T)pNextHeadBuf;

	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Traverse all support features */
	for (i = 0; i < UNI_CMD_TPC_ALGO_ACTION_NUM; i++) {
		if (pParamCtrl->TPCTagValid[i]) {
			switch (i) {
			case UNI_CMD_TPC_ACT_MANUAL_MODE:
			case UNI_CMD_TPC_ACT_ENABLE_CONFIG:
				if (UniCmdTPCTab[i].pfHandler != NULL) {
					Ret = ((PFN_TPC_MAN_CTRL_HANDLE)(UniCmdTPCTab[i].pfHandler))(pAd, &pParamCtrl->TPCManCtrl, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdTPCTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_TPC_ACT_UL_TX_POWER_CONFIG:
			case UNI_CMD_TPC_ACT_UL_TARGET_RSSI_CONFIG:
			case UNI_CMD_TPC_ACT_UL_UPH_MIN_PWR_FG_CONFIG:
				if (UniCmdTPCTab[i].pfHandler != NULL) {
					Ret = ((PFN_TPC_UL_ALGO_CTRL_HANDLE)(UniCmdTPCTab[i].pfHandler))(pAd, &pParamCtrl->TPCUlAlgoCtrl, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdTPCTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_TPC_ACT_DL_TX_POWER_CMD_CTRL_CONFIG:
			case UNI_CMD_TPC_ACT_DL_TX_POWER_CONFIG:
			case UNI_CMD_TPC_ACT_DL_TX_POWER_ALPHA_CONFIG:
				if (UniCmdTPCTab[i].pfHandler != NULL) {
					Ret = ((PFN_TPC_DL_ALGO_CTRL_HANDLE)(UniCmdTPCTab[i].pfHandler))(pAd, &pParamCtrl->TPCDlAlgoCtrl, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdTPCTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_TPC_ACT_MAN_TBL_INFO:
				if (UniCmdTPCTab[i].pfHandler != NULL) {
					Ret = ((PFN_TPC_MAN_TBL_INFO_HANDLE)(UniCmdTPCTab[i].pfHandler))(pAd, &pParamCtrl->TPCManTblInfo, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdTPCTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_TPC_ACT_WLANID_CTRL:
				if (UniCmdTPCTab[i].pfHandler != NULL) {
					Ret = ((PFN_TPC_MAN_WLAN_ID_CTRL_HANDLE)(UniCmdTPCTab[i].pfHandler))(pAd, &pParamCtrl->TPCManWlanIDCtrl, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdTPCTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_TPC_ACT_UL_UNIT_TEST_CONFIG:
				if (UniCmdTPCTab[i].pfHandler != NULL) {
					Ret = ((PFN_TPC_UL_UT_VAR_CFG_HANDLE)(UniCmdTPCTab[i].pfHandler))(pAd, &pParamCtrl->TPCUlUTVarCfg, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdTPCTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			case UNI_CMD_TPC_ACT_UL_UNIT_TEST_GO:
				if (UniCmdTPCTab[i].pfHandler != NULL) {
					Ret = ((PFN_TPC_UL_UT_CTRL_HANDLE)(UniCmdTPCTab[i].pfHandler))(pAd, &pParamCtrl->TPCUlUTCtrl, pNextHeadBuf);
					if (Ret == NDIS_STATUS_SUCCESS) {
						pNextHeadBuf += UniCmdTPCTab[i].u4StructSize;
						u2TLVNumber++;
					}
				}
				break;

			default:
				Ret = NDIS_STATUS_SUCCESS;
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s(): The hanlder of tag (0x%08x) not support!\n", __func__, UniCmdTPCTab[i].u2CmdFeature);
				break;
			}

			if (Ret != NDIS_STATUS_SUCCESS)
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"%s(): The hanlder of tag (0x%08x) return fail!\n", __func__, UniCmdTPCTab[i].u2CmdFeature);
		}
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d\n",
			u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN	bNeedFrag = FALSE;
		BOOLEAN	bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE 	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4N9);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_TPC);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
			if (!bNeedFrag || bLastFrag) {
				if (pParamCtrl->bQuery) {
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_WAIT_RETRY_RSP);
					SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
					SET_CMD_ATTR_RSP_HANDLER(attr, UniEventTPCHandler);
				} else {
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
					SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
					SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
				}
			} else {
				if (pParamCtrl->bQuery)
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_RETRY);
				else
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			Ret = chip_cmd_tx(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;	
}

INT32 UniCmdTPCManCtrl(struct _RTMP_ADAPTER *pAd, BOOLEAN fgTpcManual)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	UNI_CMD_TPC_PARAM_T TPCParam;

	os_zero_mem(&TPCParam, sizeof(TPCParam));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: fgTpcManual: %d \n", __func__, fgTpcManual));

	TPCParam.TPCManCtrl.u1TpcCtrlFormatId = UNI_CMD_TPC_ACT_MANUAL_MODE;
	if (fgTpcManual == TRUE)
		TPCParam.TPCManCtrl.eTpcParamMode = UNI_TPC_PARAM_MAN_MODE;
	else
		TPCParam.TPCManCtrl.eTpcParamMode = UNI_TPC_PARAM_AUTO_MODE;

	TPCParam.TPCTagValid[UNI_CMD_TPC_ACT_MANUAL_MODE] = TRUE;

	ret = UniCmdTPC(pAd, &TPCParam);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, ret);

	return ret;
}

INT32 UniCmdTPCUlAlgoCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8	u1TpcCmd,
	UINT8	u1ApTxPwr,
	UINT8	u1EntryIdx,
	UINT8	u1TargetRssi,
	UINT8	u1UPH,
	BOOLEAN	fgMinPwrFlag
)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	UNI_CMD_TPC_PARAM_T TPCParam;

	os_zero_mem(&TPCParam, sizeof(TPCParam));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: u1TpcCmd: %d, u1ApTxPwr: %d\n", __func__, u1TpcCmd, u1ApTxPwr));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("u1EntryIdx: %d, u1TargetRssi: %d, u1UPH: %d, fgMinPwrFlag: %d\n",
				u1EntryIdx, u1TargetRssi, u1UPH, fgMinPwrFlag));

	TPCParam.TPCUlAlgoCtrl.u1TpcCtrlFormatId = (u1TpcCmd + 1);
	TPCParam.TPCUlAlgoCtrl.u1ApTxPwr = u1ApTxPwr;
	TPCParam.TPCUlAlgoCtrl.u1EntryIdx = u1EntryIdx;
	TPCParam.TPCUlAlgoCtrl.u1TargetRssi = u1TargetRssi;
	TPCParam.TPCUlAlgoCtrl.u1UPH = u1UPH;
	TPCParam.TPCUlAlgoCtrl.fgMinPwrFlag = fgMinPwrFlag;

	if (((u1TpcCmd + 1) >= UNI_CMD_TPC_ACT_UL_TX_POWER_CONFIG) &&
		((u1TpcCmd + 1) <= UNI_CMD_TPC_ACT_UL_UPH_MIN_PWR_FG_CONFIG))
		TPCParam.TPCTagValid[(u1TpcCmd + 1)] = TRUE;

	ret = UniCmdTPC(pAd, &TPCParam);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, ret);

	return ret;
}

INT32 UniCmdTPCDlAlgoCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8	u1TpcCmd,
	BOOLEAN	fgCmdCtrl,
	UINT8	u1DlTxType,
	CHAR	DlTxPwr,
	UINT8	u1EntryIdx,
	INT16	DlTxpwrAlpha
)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	UNI_CMD_TPC_PARAM_T TPCParam;

	os_zero_mem(&TPCParam, sizeof(TPCParam));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: u1TpcCmd: %d, fgCmdCtrl: %d\n", __func__, u1TpcCmd, fgCmdCtrl));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("u1DlTxType: %d, DlTxPwr: %d, u1EntryIdx: %d, DlTxpwrAlpha: %d\n",
				u1DlTxType, DlTxPwr, u1EntryIdx, DlTxpwrAlpha));

	TPCParam.TPCDlAlgoCtrl.u1TpcCtrlFormatId = (u1TpcCmd + 4);
	TPCParam.TPCDlAlgoCtrl.i1DlTxPwr = DlTxPwr;
	TPCParam.TPCDlAlgoCtrl.fgDlTxPwrCmdCtrl = fgCmdCtrl;
	TPCParam.TPCDlAlgoCtrl.u1EntryIdx = u1EntryIdx;
	TPCParam.TPCDlAlgoCtrl.i2DlTxPwrAlpha = DlTxpwrAlpha;
	TPCParam.TPCDlAlgoCtrl.eTpcDlTxType = (UNI_ENUM_TPC_DL_TX_TYPE)u1DlTxType;

	if (((u1TpcCmd + 4) >= UNI_CMD_TPC_ACT_DL_TX_POWER_CMD_CTRL_CONFIG) &&
		((u1TpcCmd + 4) <= UNI_CMD_TPC_ACT_DL_TX_POWER_ALPHA_CONFIG))
		TPCParam.TPCTagValid[(u1TpcCmd + 4)] = TRUE;

	ret = UniCmdTPC(pAd, &TPCParam);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, ret);

	return ret;
}

INT32 UniCmdTPCManTblInfo(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgUplink
)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	UNI_CMD_TPC_PARAM_T TPCParam;

	os_zero_mem(&TPCParam, sizeof(TPCParam));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: fgUplink: %d \n", __func__, fgUplink));

	TPCParam.bQuery = TRUE;
	TPCParam.TPCManTblInfo.u1TpcCtrlFormatId = UNI_CMD_TPC_ACT_MAN_TBL_INFO;
	TPCParam.TPCManTblInfo.fgUplink = fgUplink;
	TPCParam.TPCTagValid[UNI_CMD_TPC_ACT_MAN_TBL_INFO] = TRUE;

	ret = UniCmdTPC(pAd, &TPCParam);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, ret);

	return ret;
}

INT32 UniCmdTPCWlanIdCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgUplink,
	UINT8   u1EntryIdx,
	UINT16  u2WlanId,
	UINT8 u1DlTxType
)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	UNI_CMD_TPC_PARAM_T TPCParam;

	os_zero_mem(&TPCParam, sizeof(TPCParam));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: fgUplink: %d, u1EntryIdx: %d, u2WlanId: %d, u1DlTxType: %d\n",
				__func__, fgUplink, u1EntryIdx, u2WlanId, u1DlTxType));
	
	TPCParam.TPCManWlanIDCtrl.u1TpcCtrlFormatId = UNI_CMD_TPC_ACT_WLANID_CTRL;
	TPCParam.TPCManWlanIDCtrl.u1EntryIdx = u1EntryIdx;
	TPCParam.TPCManWlanIDCtrl.u2WlanId = u2WlanId;
	TPCParam.TPCManWlanIDCtrl.fgUplink = fgUplink;
	TPCParam.TPCManWlanIDCtrl.eTpcDlTxType = (UNI_ENUM_TPC_DL_TX_TYPE)u1DlTxType;
	TPCParam.TPCTagValid[UNI_CMD_TPC_ACT_WLANID_CTRL] = TRUE;

	ret = UniCmdTPC(pAd, &TPCParam);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, ret);

	return ret;
}

INT32 UniCmdTPCUlUtVarCfg(
	struct _RTMP_ADAPTER *pAd,
	UINT8	u1EntryIdx,
	UINT8	u1VarType,
	INT16	i2Value)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	UNI_CMD_TPC_PARAM_T TPCParam;

	os_zero_mem(&TPCParam, sizeof(TPCParam));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("u1EntryIdx: %d, VarType:%d, Value: %d\n", u1EntryIdx, u1VarType, i2Value));

	TPCParam.TPCUlUTVarCfg.u1TpcCtrlFormatId = UNI_CMD_TPC_ACT_UL_UNIT_TEST_CONFIG;
	TPCParam.TPCUlUTVarCfg.u1EntryIdx = u1EntryIdx;
	TPCParam.TPCUlUTVarCfg.i2Value = i2Value;
	TPCParam.TPCUlUTVarCfg.u1VarType = u1VarType;
	TPCParam.TPCTagValid[UNI_CMD_TPC_ACT_UL_UNIT_TEST_CONFIG] = TRUE;

	ret = UniCmdTPC(pAd, &TPCParam);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, ret);

	return ret;
}

INT32 UniCmdTPCUlUtGo(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgTpcUtGo
)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	UNI_CMD_TPC_PARAM_T TPCParam;

	os_zero_mem(&TPCParam, sizeof(TPCParam));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: fgTpcUtGo: %d \n", __func__, fgTpcUtGo));

	TPCParam.TPCUlUTCtrl.u1TpcCtrlFormatId = UNI_CMD_TPC_ACT_UL_UNIT_TEST_GO;
	TPCParam.TPCUlUTCtrl.fgTpcUtGo = fgTpcUtGo;
	TPCParam.TPCTagValid[UNI_CMD_TPC_ACT_UL_UNIT_TEST_GO] = TRUE;

	ret = UniCmdTPC(pAd, &TPCParam);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, ret);

	return ret;
}

INT32 UniCmdTPCEnableCfg(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgTpcEnable
)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	UNI_CMD_TPC_PARAM_T TPCParam;

	os_zero_mem(&TPCParam, sizeof(TPCParam));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: fgTpcEnable: %d \n", __func__, fgTpcEnable));

	TPCParam.TPCManCtrl.u1TpcCtrlFormatId = UNI_CMD_TPC_ACT_ENABLE_CONFIG;
	TPCParam.TPCManCtrl.fgTpcEnable = fgTpcEnable;
	TPCParam.TPCTagValid[UNI_CMD_TPC_ACT_ENABLE_CONFIG] = TRUE;

	ret = UniCmdTPC(pAd, &TPCParam);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, ret);

	return ret;
}


/*****************************************
 * UNI_CMD_ID_VOW (Tag 0x37)
 *****************************************/
/* UNI_CMD_VOW_DRR_CTRL (TAG 0x00): for station DWRR configration */
INT32 uni_cmd_vow_set_sta(PRTMP_ADAPTER pad, UINT16 sta_id, UINT32 subcmd)
{
	UINT32 Setting = 0;
	INT32 ret;
	UNI_CMD_VOW_PARAM_T VOWParam;
	P_UNI_CMD_VOW_DRR_CTRL_T pVowDrrCtrl = &(VOWParam.VowDrrCtrl);

	NdisZeroMemory(&VOWParam, sizeof(UNI_CMD_VOW_PARAM_T));
	pVowDrrCtrl->u4CtrlFieldID = subcmd;
	pVowDrrCtrl->u2StaID = sta_id;

	switch (subcmd) {
	case ENUM_VOW_DRR_CTRL_FIELD_STA_ALL:
		/* station configration */
		Setting |= pad->vow_sta_cfg[sta_id].group;
		Setting |= (pad->vow_sta_cfg[sta_id].ac_change_rule << pad->vow_gen.VOW_STA_AC_PRIORITY_OFFSET);
		Setting |= (pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_BK] << pad->vow_gen.VOW_STA_WMM_AC0_OFFSET);
		Setting |= (pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_BE] << pad->vow_gen.VOW_STA_WMM_AC1_OFFSET);
		Setting |= (pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_VI] << pad->vow_gen.VOW_STA_WMM_AC2_OFFSET);
		Setting |= (pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_VO] << pad->vow_gen.VOW_STA_WMM_AC3_OFFSET);
		pVowDrrCtrl->u4ComValue = Setting;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SubCmd %x, Value = 0x%x)\n", subcmd, Setting);
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_BSS_GROUP:
		pVowDrrCtrl->u4ComValue = pad->vow_sta_cfg[sta_id].group;
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_WMM_ID:
		pVowDrrCtrl->u4ComValue = pad->vow_sta_cfg[sta_id].wmm_idx;
		break;
	case ENUM_VOW_DRR_CTRL_FIELD_STA_PRIORITY:
		pVowDrrCtrl->u4ComValue = pad->vow_sta_cfg[sta_id].ac_change_rule;
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_AC0_QUA_ID:
		pVowDrrCtrl->u4ComValue = pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_BK];
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_AC1_QUA_ID:
		pVowDrrCtrl->u4ComValue = pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_BE];
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_AC2_QUA_ID:
		pVowDrrCtrl->u4ComValue = pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_VI];
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_AC3_QUA_ID:
		pVowDrrCtrl->u4ComValue = pad->vow_sta_cfg[sta_id].dwrr_quantum[WMM_AC_VO];
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L1:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L2:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L3:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L4:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L5:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L6:
	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L7:
		pVowDrrCtrl->u4ComValue = pad->vow_cfg.vow_sta_dwrr_quantum[subcmd - ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0];
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n",
		    subcmd, pad->vow_cfg.vow_sta_dwrr_quantum[subcmd - ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_L0]);
		break;

	case ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_ALL: {
		UINT32 i;

		/* station quantum configruation */
		for (i = 0; i < VOW_MAX_STA_DWRR_NUM; i++) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(vow_sta_dwrr_quantum[%d] = 0x%x)\n", i, pad->vow_cfg.vow_sta_dwrr_quantum[i]);
			pVowDrrCtrl->aucAirTimeQuantum[i] = pad->vow_cfg.vow_sta_dwrr_quantum[i];
		}

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, Setting);
	}
	break;

	case ENUM_VOW_DRR_CTRL_FIELD_STA_PAUSE_SETTING: {
		pVowDrrCtrl->u4ComValue = pad->vow_sta_cfg[sta_id].paused;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, pad->vow_sta_cfg[sta_id].paused));
	}
	break;

	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(No such command = 0x%x)\n", subcmd);
		break;
	}

	VOWParam.VOWTagValid[UNI_CMD_VOW_DRR_CTRL] = TRUE;
	ret = UniCmdVOWUpdate(pad, &VOWParam, TRUE, HOST2CR4N9, NULL);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_VOW_DRR_CTRL_T));
	return ret;
}

/* UNI_CMD_VOW_DRR_CTRL (TAG 0x00): for DWRR max wait time configuration */
INT uni_vmd_vow_set_sta_DWRR_max_time(PRTMP_ADAPTER pad)
{
	UNI_CMD_VOW_PARAM_T VOWParam;
	P_UNI_CMD_VOW_DRR_CTRL_T pVowDrrCtrl = &(VOWParam.VowDrrCtrl);
	INT32 ret;

	NdisZeroMemory(&VOWParam, sizeof(UNI_CMD_VOW_PARAM_T));
	pVowDrrCtrl->u4CtrlFieldID = ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_DEFICIT_BOUND;
	pVowDrrCtrl->u4ComValue = pad->vow_cfg.sta_max_wait_time;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(sta_max_wait_time = 0x%x)\n", pad->vow_cfg.sta_max_wait_time);

	VOWParam.VOWTagValid[UNI_CMD_VOW_DRR_CTRL] = TRUE;
	ret = UniCmdVOWUpdate(pad, &VOWParam, TRUE, HOST2CR4N9, NULL);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_VOW_DRR_CTRL_T));
	return ret;
}

/* UNI_CMD_VOW_DRR_CTRL (TAG 0x00): for DWRR max wait time configuration */
INT uni_cmd_vow_set_group_DWRR_max_time(PRTMP_ADAPTER pad)
{
	UNI_CMD_VOW_PARAM_T VOWParam;
	P_UNI_CMD_VOW_DRR_CTRL_T pVowDrrCtrl = &(VOWParam.VowDrrCtrl);
	INT32 ret;

	if (!(pad->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCTRL))
		return 0;

	NdisZeroMemory(&VOWParam, sizeof(UNI_CMD_VOW_PARAM_T));
	pVowDrrCtrl->u4CtrlFieldID = ENUM_VOW_DRR_CTRL_FIELD_BW_DEFICIT_BOUND;
	pVowDrrCtrl->u4ComValue = pad->vow_cfg.group_max_wait_time;
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(group_max_wait_time = 0x%x)\n", pad->vow_cfg.group_max_wait_time);

	VOWParam.VOWTagValid[UNI_CMD_VOW_DRR_CTRL] = TRUE;
	ret = UniCmdVOWUpdate(pad, &VOWParam, TRUE, HOST2CR4N9, NULL);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_VOW_DRR_CTRL_T));
	return ret;
}

/*
* Unified command UNI_CMD_VOW_DRR_CTRL (TAG 0x00) handler
*/
static INT32 UniCmdVowDrrCtrl(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_DRR_CTRL_T pUniCmdVowDrrCtrl = (P_UNI_CMD_VOW_DRR_CTRL_T)pHandle;
	P_UNI_CMD_VOW_DRR_CTRL_T pUniCmdVowDrrCtrlParam = &(pVowParam->VowDrrCtrl);

	/* Fill TLV format */
	memcpy(pUniCmdVowDrrCtrl, pUniCmdVowDrrCtrlParam, sizeof(UNI_CMD_VOW_DRR_CTRL_T));
	pUniCmdVowDrrCtrl->u2Tag = cpu2le16(UNI_CMD_VOW_DRR_CTRL);
	pUniCmdVowDrrCtrl->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_DRR_CTRL_T));

	pUniCmdVowDrrCtrl->u2StaID = cpu2le16(pUniCmdVowDrrCtrl->u2StaID);
	pUniCmdVowDrrCtrl->u4CtrlFieldID = cpu2le32(pUniCmdVowDrrCtrl->u4CtrlFieldID);
	pUniCmdVowDrrCtrl->u4ComValue = cpu2le32(pUniCmdVowDrrCtrl->u4ComValue);

	MTWF_DBG_NP(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, u2StaID=%d, u4CtrlFieldID=%d, u4ComValue=%d, aucAirTimeQuantum=%d-%d-%d-%d-%d-%d-%d-%d\n",
		le2cpu16(pUniCmdVowDrrCtrl->u2Tag), le2cpu16(pUniCmdVowDrrCtrl->u2Length),
		le2cpu16(pUniCmdVowDrrCtrl->u2StaID), le2cpu32(pUniCmdVowDrrCtrl->u4CtrlFieldID), le2cpu32(pUniCmdVowDrrCtrl->u4ComValue),
		pUniCmdVowDrrCtrl->aucAirTimeQuantum[0], pUniCmdVowDrrCtrl->aucAirTimeQuantum[1],
		pUniCmdVowDrrCtrl->aucAirTimeQuantum[2], pUniCmdVowDrrCtrl->aucAirTimeQuantum[3],
		pUniCmdVowDrrCtrl->aucAirTimeQuantum[4], pUniCmdVowDrrCtrl->aucAirTimeQuantum[5],
		pUniCmdVowDrrCtrl->aucAirTimeQuantum[6], pUniCmdVowDrrCtrl->aucAirTimeQuantum[7]);

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_DRR_CTRL_T);
	return Ret;

}

/* UNI_CMD_VOW_FEATURE_CTRL (TAG 0x01): for group configuration */
INT uni_vmd_vow_set_feature_all(PRTMP_ADAPTER pad)
{
	UNI_CMD_VOW_PARAM_T VOWParam;
	P_UNI_CMD_VOW_FEATURE_CTRL_T pVowFeatureCtrl = &(VOWParam.VowFeatureCtrl);
	INT32 ret, i;

	NdisZeroMemory(&VOWParam, sizeof(UNI_CMD_VOW_PARAM_T));

	/* DW0 - flags */
	pVowFeatureCtrl->u2IfApplyBss_0_to_16_CtrlFlag = 0xFFFF; /* 16'b */
	pVowFeatureCtrl->u2IfApplyRefillPerildFlag = TRUE; /* 1'b */
	pVowFeatureCtrl->u2IfApplyDbdc1SearchRuleFlag = TRUE; /* 1'b */
	pVowFeatureCtrl->u2IfApplyDbdc0SearchRuleFlag = TRUE; /* 1'b */
	pVowFeatureCtrl->u2IfApplyEnTxopNoChangeBssFlag = TRUE; /* 1'b */
	pVowFeatureCtrl->u2IfApplyAirTimeFairnessFlag = TRUE; /* 1'b */
	pVowFeatureCtrl->u2IfApplyWeightedAirTimeFairnessFlag = TRUE; /* 1'b */
	pVowFeatureCtrl->u2IfApplyEnbwrefillFlag = TRUE; /* 1'b */
	pVowFeatureCtrl->u2IfApplyEnbwCtrlFlag = TRUE; /* 1'b */
	pVowFeatureCtrl->u4IfApplyKeepQuantumFlag = TRUE; /* 1'b */
	/* DW1 - flags */
	pVowFeatureCtrl->u2IfApplyBssCheckTimeToken_0_to_16_CtrlFlag = 0xFFFF;
	/* DW2 - flags */
	pVowFeatureCtrl->u2IfApplyBssCheckLengthToken_0_to_16_CtrlFlag = 0xFFFF;
	/* DW5 - ctrl values */
	pVowFeatureCtrl->u2Bss_0_to_16_CtrlValue = pad->vow_cfg.per_bss_enable; /* 16'b */
	pVowFeatureCtrl->u2RefillPerildValue = pad->vow_cfg.refill_period; /* 8'b */
	pVowFeatureCtrl->u2Dbdc1SearchRuleValue = pad->vow_cfg.dbdc1_search_rule; /* 1'b */
	pVowFeatureCtrl->u2Dbdc0SearchRuleValue = pad->vow_cfg.dbdc0_search_rule; /* 1'b */
	pVowFeatureCtrl->u2WeightedAirTimeFairnessValue = pad->vow_watf_en; /* 1'b */
	pVowFeatureCtrl->u2EnTxopNoChangeBssValue = pad->vow_cfg.en_txop_no_change_bss; /* 1'b */
	pVowFeatureCtrl->u2AirTimeFairnessValue = pad->vow_cfg.en_airtime_fairness; /* 1'b */
	pVowFeatureCtrl->u2EnbwrefillValue = pad->vow_cfg.en_bw_refill; /* 1'b */
	pVowFeatureCtrl->u2EnbwCtrlValue = pad->vow_cfg.en_bw_ctrl; /* 1'b */
	pVowFeatureCtrl->u4KeepQuantumValue = pad->vow_misc_cfg.keep_quantum; /* 1'b */

	/* DW6 - ctrl values */
	for (i = 0; i < VOW_MAX_GROUP_NUM; i++)
		pVowFeatureCtrl->u2BssCheckTimeToken_0_to_16_CtrlValue |= (pad->vow_bss_cfg[i].at_on << i);

	/* DW7 - ctrl values */
	for (i = 0; i < VOW_MAX_GROUP_NUM; i++)
		pVowFeatureCtrl->u2BssCheckLengthToken_0_to_16_CtrlValue |= (pad->vow_bss_cfg[i].bw_on << i);

	if ((pad->vow_gen.VOW_GEN == VOW_GEN_2) ||
		(pad->vow_gen.VOW_GEN == VOW_GEN_TALOS) ||
		(pad->vow_gen.VOW_GEN == VOW_GEN_FALCON)) {
		/* DW8 - misc */
		pVowFeatureCtrl->u4IfApplyStaLockForRtsFlag = TRUE; /* 1'b */
		pVowFeatureCtrl->u4RtsStaLockValue = pad->vow_misc_cfg.rts_sta_lock; /* 1'b */
		/* VOW is disabled, skip all setting */
		if (vow_is_enabled(pad)) {
			pVowFeatureCtrl->u4IfApplyTxCountModeFlag = TRUE; /* 1'b */
			pVowFeatureCtrl->u4TxCountValue = pad->vow_misc_cfg.tx_rr_count; /* 4'b */
			pVowFeatureCtrl->u4IfApplyTxMeasurementModeFlag = TRUE; /* 1'b */
			pVowFeatureCtrl->u4TxMeasurementModeValue = pad->vow_misc_cfg.measurement_mode; /* 1'b */
			pVowFeatureCtrl->u4IfApplyTxBackOffBoundFlag = TRUE; /* 1'b */
			pVowFeatureCtrl->u4TxBackOffBoundEnable = pad->vow_misc_cfg.max_backoff_bound_en; /* 1'b */
			pVowFeatureCtrl->u4TxBackOffBoundValue = pad->vow_misc_cfg.max_backoff_bound; /* 4'b */
			pVowFeatureCtrl->u4IfApplyRtsFailedChargeDisFlag = TRUE; /* 1'b */
			pVowFeatureCtrl->u4RtsFailedChargeDisValue = pad->vow_misc_cfg.rts_failed_charge_time_en; /* 1'b */
			pVowFeatureCtrl->u4IfApplyRxEifsToZeroFlag = TRUE; /* 1'b */
			pVowFeatureCtrl->u4ApplyRxEifsToZeroValue = pad->vow_misc_cfg.zero_eifs_time; /* 1'b */
			pVowFeatureCtrl->u4IfApplyRxRifsModeforCckCtsFlag = TRUE; /* 1'b */
			pVowFeatureCtrl->u4RxRifsModeforCckCtsValue = pad->vow_misc_cfg.rx_rifs_mode; /* 1'b */
			pVowFeatureCtrl->u4IfApplyKeepVoWSettingForSerFlag = TRUE; /* 1'b */
			pVowFeatureCtrl->u4VowKeepSettingValue = pad->vow_misc_cfg.keep_vow_sram_setting; /* 1'b */
			pVowFeatureCtrl->u4VowKeepSettingBit = pad->vow_misc_cfg.keep_vow_sram_setting_bit; /* 1'b */
			if (pad->vow_gen.VOW_GEN < VOW_GEN_FALCON) {
				pVowFeatureCtrl->u4IfApplySplFlag = TRUE; /* 1'b */
				pVowFeatureCtrl->u4SplStaNumValue = pad->vow_misc_cfg.spl_sta_count; /* 3'b */
			}
		}
	}

	pVowFeatureCtrl->u4DbgPrnLvl = (pad->vow_show_en == 0) ? 0 : (pad->vow_show_en - 1);

	/* DW9 - schedule */
	pVowFeatureCtrl->u4IfApplyVowSchCtrl = pad->vow_sch_cfg.apply_sch_ctrl;
	pVowFeatureCtrl->u4VowScheduleType = pad->vow_sch_cfg.sch_type;
	pVowFeatureCtrl->u4VowSchedulePolicy = pad->vow_sch_cfg.sch_policy;

	if (pad->vow_gen.VOW_GEN == VOW_GEN_FALCON) {
		if (vow_is_enabled(pad)) {
			pVowFeatureCtrl->u4IfApplyRxEifsToZeroFlag = TRUE; /* 1'b */
			pVowFeatureCtrl->u4ApplyRxEifsToZeroValue = pad->vow_misc_cfg.zero_eifs_time; /* 1'b */
		}
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2Bss_0_to_16_CtrlValue	= 0x%x)\n", pad->vow_cfg.per_bss_enable);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2RefillPerildValue = 0x%x)\n", pad->vow_cfg.refill_period);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2Dbdc1SearchRuleValue = 0x%x)\n", pad->vow_cfg.dbdc1_search_rule);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2Dbdc0SearchRuleValue = 0x%x)\n", pad->vow_cfg.dbdc0_search_rule);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2EnTxopNoChangeBssValue = 0x%x)\n", pad->vow_cfg.en_txop_no_change_bss);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2AirTimeFairnessValue = 0x%x)\n", pad->vow_cfg.en_airtime_fairness);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2EnbwrefillValue = 0x%x)\n", pad->vow_cfg.en_bw_refill);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2EnbwCtrlValue = 0x%x)\n", pad->vow_cfg.en_bw_ctrl);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2WeightedAirTimeFairnessValue = 0x%x)\n", pad->vow_watf_en);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2BssCheckTimeToken_0_to_16_CtrlValue = 0x%x)\n", pVowFeatureCtrl->u2BssCheckTimeToken_0_to_16_CtrlValue);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(u2BssCheckLengthToken_0_to_16_CtrlValue = 0x%x)\n", pVowFeatureCtrl->u2BssCheckLengthToken_0_to_16_CtrlValue);

	VOWParam.VOWTagValid[UNI_CMD_VOW_FEATURE_CTRL] = TRUE;
	ret = UniCmdVOWUpdate(pad, &VOWParam, TRUE, HOST2CR4N9, NULL);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_VOW_FEATURE_CTRL_T));
	return ret;
}

/*
* Unified command UNI_CMD_VOW_FEATURE_CTRL (TAG 0x01) handler
*/
static INT32 UniCmdVowFeatureCtrl(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_FEATURE_CTRL_T pUniCmdVowFeatureCtrl = (P_UNI_CMD_VOW_FEATURE_CTRL_T)pHandle;
	P_UNI_CMD_VOW_FEATURE_CTRL_T pVowFeatureCtrlParam = &(pVowParam->VowFeatureCtrl);

	memcpy(pUniCmdVowFeatureCtrl, pVowFeatureCtrlParam, sizeof(UNI_CMD_VOW_FEATURE_CTRL_T));

	/* Fill TLV format */
	pUniCmdVowFeatureCtrl->u2Tag = cpu2le16(UNI_CMD_VOW_FEATURE_CTRL);
	pUniCmdVowFeatureCtrl->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_FEATURE_CTRL_T));

	pUniCmdVowFeatureCtrl->u2IfApplyBss_0_to_16_CtrlFlag = cpu2le16(pVowFeatureCtrlParam->u2IfApplyBss_0_to_16_CtrlFlag);
	pUniCmdVowFeatureCtrl->u2IfApplyBssCheckTimeToken_0_to_16_CtrlFlag = cpu2le16(pVowFeatureCtrlParam->u2IfApplyBssCheckTimeToken_0_to_16_CtrlFlag);
	pUniCmdVowFeatureCtrl->u2IfApplyBssCheckLengthToken_0_to_16_CtrlFlag = cpu2le16(pVowFeatureCtrlParam->u2IfApplyBssCheckLengthToken_0_to_16_CtrlFlag);
	pUniCmdVowFeatureCtrl->u2Bss_0_to_16_CtrlValue = cpu2le16(pVowFeatureCtrlParam->u2Bss_0_to_16_CtrlValue);
	pUniCmdVowFeatureCtrl->u2BssCheckTimeToken_0_to_16_CtrlValue = cpu2le16(pVowFeatureCtrlParam->u2BssCheckTimeToken_0_to_16_CtrlValue);
	pUniCmdVowFeatureCtrl->u2BssCheckLengthToken_0_to_16_CtrlValue = cpu2le16(pVowFeatureCtrlParam->u2BssCheckLengthToken_0_to_16_CtrlValue);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d\n",
		le2cpu16(pUniCmdVowFeatureCtrl->u2Tag), le2cpu16(pUniCmdVowFeatureCtrl->u2Length));

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_FEATURE_CTRL_T);
	return Ret;

}

/* TAG 02-06: for group configuration */
INT uni_cmd_vow_set_group(PRTMP_ADAPTER pad, UINT8 group_id, UINT32 subcmd)
{
	UNI_CMD_VOW_PARAM_T VOWParam;
	P_UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T pVowBssgroupCtrl1Group = &(VOWParam.VowBssgroupCtrl1Group);
	P_UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T pVowBssgroupTokenCfg = &(VOWParam.VowBssgroupTokenCfg);
	P_UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T pVowBssgroupCtrlAllGroup = &(VOWParam.VowBssgroupCtrlAllGroup);
	P_UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T pVowBssgroupBWGroupQuantum = &(VOWParam.VowBssgroupBWGroupQuantum);
	P_UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T pVowBssgroupBWGroupQuantumAll = &(VOWParam.VowBssgroupBWGroupQuantumAll);
	INT32 ret;

	NdisZeroMemory(&VOWParam, sizeof(UNI_CMD_VOW_PARAM_T));
	/* TAG 3 */
	pVowBssgroupTokenCfg->u4CfgItemId = subcmd;
	pVowBssgroupTokenCfg->ucBssGroupID = group_id;

	switch (subcmd) {
	/* group configuration */
	case ENUM_BSSGROUP_CTRL_ALL_ITEM_FOR_1_GROUP:
		VOWParam.VOWTagValid[UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP] = TRUE;
		pVowBssgroupCtrl1Group->ucBssGroupID = group_id;
		/* DW0 */
		pVowBssgroupCtrl1Group->rAllBssGroupMultiField.u2MinRateToken = pad->vow_bss_cfg[group_id].min_rate_token;
		pVowBssgroupCtrl1Group->rAllBssGroupMultiField.u2MaxRateToken = pad->vow_bss_cfg[group_id].max_rate_token;
		/* DW1 */
		pVowBssgroupCtrl1Group->rAllBssGroupMultiField.u4MinTokenBucketTimeSize = pad->vow_bss_cfg[group_id].min_airtimebucket_size;
		pVowBssgroupCtrl1Group->rAllBssGroupMultiField.u4MinAirTimeToken = pad->vow_bss_cfg[group_id].min_airtime_token;
		pVowBssgroupCtrl1Group->rAllBssGroupMultiField.u4MinTokenBucketLengSize = pad->vow_bss_cfg[group_id].min_ratebucket_size;
		/* DW2 */
		pVowBssgroupCtrl1Group->rAllBssGroupMultiField.u4MaxTokenBucketTimeSize = pad->vow_bss_cfg[group_id].max_airtimebucket_size;
		pVowBssgroupCtrl1Group->rAllBssGroupMultiField.u4MaxAirTimeToken = pad->vow_bss_cfg[group_id].max_airtime_token;
		pVowBssgroupCtrl1Group->rAllBssGroupMultiField.u4MaxTokenBucketLengSize = pad->vow_bss_cfg[group_id].max_ratebucket_size;
		/* DW3 */
		pVowBssgroupCtrl1Group->rAllBssGroupMultiField.u4MaxWaitTime = pad->vow_bss_cfg[group_id].max_wait_time;
		pVowBssgroupCtrl1Group->rAllBssGroupMultiField.u4MaxBacklogSize = pad->vow_bss_cfg[group_id].max_backlog_size;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(Group id = 0x%x, min_rate %d, max_rate %d, min_ratio %d, max_ratio %d)\n",
				 group_id,
				 pad->vow_bss_cfg[group_id].min_rate,
				 pad->vow_bss_cfg[group_id].max_rate,
				 pad->vow_bss_cfg[group_id].min_airtime_ratio,
				 pad->vow_bss_cfg[group_id].max_airtime_ratio);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(min rate token = 0x%x)\n", pad->vow_bss_cfg[group_id].min_rate_token);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max rate token = 0x%x)\n", pad->vow_bss_cfg[group_id].max_rate_token);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(min airtime token = 0x%x)\n", pad->vow_bss_cfg[group_id].min_airtime_token);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max airtime token = 0x%x)\n", pad->vow_bss_cfg[group_id].max_airtime_token);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(min rate bucket = 0x%x)\n", pad->vow_bss_cfg[group_id].min_ratebucket_size);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max rate bucket = 0x%x)\n", pad->vow_bss_cfg[group_id].max_ratebucket_size);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(min airtime bucket = 0x%x)\n", pad->vow_bss_cfg[group_id].min_airtimebucket_size);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max airtime bucket = 0x%x)\n", pad->vow_bss_cfg[group_id].max_airtimebucket_size);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max baclog size = 0x%x)\n", pad->vow_bss_cfg[group_id].max_backlog_size);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max wait time = 0x%x)\n", pad->vow_bss_cfg[group_id].max_wait_time);
		break;

	case ENUM_BSSGROUP_CTRL_MIN_RATE_TOKEN_CFG_ITEM:
		pVowBssgroupTokenCfg->u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].min_rate_token;
		VOWParam.VOWTagValid[UNI_CMD_VOW_BSSGROUP_TOKEN_CFG] = TRUE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, pVowBssgroupTokenCfg->u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MAX_RATE_TOKEN_CFG_ITEM:
		pVowBssgroupTokenCfg->u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_rate_token;
		VOWParam.VOWTagValid[UNI_CMD_VOW_BSSGROUP_TOKEN_CFG] = TRUE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, pVowBssgroupTokenCfg->u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MIN_TOKEN_BUCKET_TIME_SIZE_CFG_ITEM:
		pVowBssgroupTokenCfg->u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].min_airtimebucket_size;
		VOWParam.VOWTagValid[UNI_CMD_VOW_BSSGROUP_TOKEN_CFG] = TRUE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, pVowBssgroupTokenCfg->u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MIN_AIRTIME_TOKEN_CFG_ITEM:
		pVowBssgroupTokenCfg->u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].min_airtime_token;
		VOWParam.VOWTagValid[UNI_CMD_VOW_BSSGROUP_TOKEN_CFG] = TRUE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, pVowBssgroupTokenCfg->u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MIN_TOKEN_BUCKET_LENG_SIZE_CFG_ITEM:
		pVowBssgroupTokenCfg->u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].min_ratebucket_size;
		VOWParam.VOWTagValid[UNI_CMD_VOW_BSSGROUP_TOKEN_CFG] = TRUE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, pVowBssgroupTokenCfg->u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MAX_TOKEN_BUCKET_TIME_SIZE_CFG_ITEM:
		pVowBssgroupTokenCfg->u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_airtimebucket_size;
		VOWParam.VOWTagValid[UNI_CMD_VOW_BSSGROUP_TOKEN_CFG] = TRUE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, pVowBssgroupTokenCfg->u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MAX_AIRTIME_TOKEN_CFG_ITEM:
		pVowBssgroupTokenCfg->u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_airtime_token;
		VOWParam.VOWTagValid[UNI_CMD_VOW_BSSGROUP_TOKEN_CFG] = TRUE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, pVowBssgroupTokenCfg->u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MAX_TOKEN_BUCKET_LENG_SIZE_CFG_ITEM:
		pVowBssgroupTokenCfg->u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_ratebucket_size;
		VOWParam.VOWTagValid[UNI_CMD_VOW_BSSGROUP_TOKEN_CFG] = TRUE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, pVowBssgroupTokenCfg->u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MAX_WAIT_TIME_CFG_ITEM:
		pVowBssgroupTokenCfg->u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_wait_time;
		VOWParam.VOWTagValid[UNI_CMD_VOW_BSSGROUP_TOKEN_CFG] = TRUE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, pVowBssgroupTokenCfg->u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_MAX_BACKLOG_SIZE_CFG_ITEM:
		pVowBssgroupTokenCfg->u4SingleFieldIDValue = pad->vow_bss_cfg[group_id].max_backlog_size;
		VOWParam.VOWTagValid[UNI_CMD_VOW_BSSGROUP_TOKEN_CFG] = TRUE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SubCmd %x, Value = 0x%x)\n", subcmd, pVowBssgroupTokenCfg->u4SingleFieldIDValue);
		break;

	case ENUM_BSSGROUP_CTRL_ALL_ITEM_FOR_ALL_GROUP: {
		UINT32 i;

		VOWParam.VOWTagValid[UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP] = TRUE;
		for (i = 0; i < VOW_MAX_GROUP_NUM; i++) {
			/* DW0 */
			pVowBssgroupCtrlAllGroup->arAllBssGroupMultiField[i].u2MinRateToken = pad->vow_bss_cfg[i].min_rate_token;
			pVowBssgroupCtrlAllGroup->arAllBssGroupMultiField[i].u2MaxRateToken = pad->vow_bss_cfg[i].max_rate_token;
			/* DW1 */
			pVowBssgroupCtrlAllGroup->arAllBssGroupMultiField[i].u4MinTokenBucketTimeSize = pad->vow_bss_cfg[i].min_airtimebucket_size;
			pVowBssgroupCtrlAllGroup->arAllBssGroupMultiField[i].u4MinAirTimeToken = pad->vow_bss_cfg[i].min_airtime_token;
			pVowBssgroupCtrlAllGroup->arAllBssGroupMultiField[i].u4MinTokenBucketLengSize = pad->vow_bss_cfg[i].min_ratebucket_size;
			/* DW2 */
			pVowBssgroupCtrlAllGroup->arAllBssGroupMultiField[i].u4MaxTokenBucketTimeSize = pad->vow_bss_cfg[i].max_airtimebucket_size;
			pVowBssgroupCtrlAllGroup->arAllBssGroupMultiField[i].u4MaxAirTimeToken = pad->vow_bss_cfg[i].max_airtime_token;
			pVowBssgroupCtrlAllGroup->arAllBssGroupMultiField[i].u4MaxTokenBucketLengSize = pad->vow_bss_cfg[i].max_ratebucket_size;
			/* DW3 */
			pVowBssgroupCtrlAllGroup->arAllBssGroupMultiField[i].u4MaxWaitTime = pad->vow_bss_cfg[i].max_wait_time;
			pVowBssgroupCtrlAllGroup->arAllBssGroupMultiField[i].u4MaxBacklogSize = pad->vow_bss_cfg[i].max_backlog_size;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(Group id = 0x%x, min_rate %d, max_rate %d, min_ratio %d, max_ratio %d)\n",
					 i,
					 pad->vow_bss_cfg[i].min_rate,
					 pad->vow_bss_cfg[i].max_rate,
					 pad->vow_bss_cfg[i].min_airtime_ratio,
					 pad->vow_bss_cfg[i].max_airtime_ratio);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(min rate token = 0x%x)\n", pad->vow_bss_cfg[i].min_rate_token);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max rate token = 0x%x)\n", pad->vow_bss_cfg[i].max_rate_token);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(min airtime token = 0x%x)\n", pad->vow_bss_cfg[i].min_airtime_token);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max airtime token = 0x%x)\n", pad->vow_bss_cfg[i].max_airtime_token);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(min rate bucket = 0x%x)\n", pad->vow_bss_cfg[i].min_ratebucket_size);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max rate bucket = 0x%x)\n", pad->vow_bss_cfg[i].max_ratebucket_size);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(min airtime bucket = 0x%x)\n", pad->vow_bss_cfg[i].min_airtimebucket_size);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max airtime bucket = 0x%x)\n", pad->vow_bss_cfg[i].max_airtimebucket_size);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max baclog size = 0x%x)\n", pad->vow_bss_cfg[i].max_backlog_size);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(max wait time = 0x%x)\n", pad->vow_bss_cfg[i].max_wait_time);
		}
	}
	break;

	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_00:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_01:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_02:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_03:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_04:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_05:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_06:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_07:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_08:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_09:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0A:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0B:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0C:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0D:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0E:
	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_0F:
		pVowBssgroupBWGroupQuantum->ucBssGroupQuantumID = group_id - ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_L_00;
		pVowBssgroupBWGroupQuantum->ucBssGroupQuantumTime = pad->vow_bss_cfg[group_id].dwrr_quantum;
		VOWParam.VOWTagValid[UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM] = TRUE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(group %d DWRR quantum = 0x%x)\n", group_id, pad->vow_bss_cfg[group_id].dwrr_quantum);
		break;

	case ENUM_BSSGROUP_CTRL_BW_GROUP_QUANTUM_ALL: {
		UINT32 i;

		VOWParam.VOWTagValid[UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL] = TRUE;
		for (i = 0; i < VOW_MAX_GROUP_NUM; i++) {
			pVowBssgroupBWGroupQuantumAll->aucBssGroupQuantumTime[i] = pad->vow_bss_cfg[i].dwrr_quantum;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(group %d DWRR quantum = 0x%x)\n", i, pad->vow_bss_cfg[i].dwrr_quantum);
		}
	}
	break;

	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(No such command = 0x%x)\n", subcmd);
		break;
	}

	ret = UniCmdVOWUpdate(pad, &VOWParam, TRUE, HOST2CR4N9, NULL);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(ret = %d), sizeof %zu\n", ret, sizeof(EXT_CMD_BSS_CTRL_T));
	return ret;
}

/*
* Unified command UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP (TAG 0x02) handler
*/
static INT32 UniCmdVowBssGroupCtrl(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T pUniCmdVowBss = (P_UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T)pHandle;
	P_UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T pParam = &(pVowParam->VowBssgroupCtrl1Group);

	memcpy(pUniCmdVowBss, pParam, sizeof(UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T));
	/* Fill TLV format */
	pUniCmdVowBss->u2Tag = cpu2le16(UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP);
	pUniCmdVowBss->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T));

	pUniCmdVowBss->rAllBssGroupMultiField.u2MinRateToken = cpu2le16(pUniCmdVowBss->rAllBssGroupMultiField.u2MinRateToken);
	pUniCmdVowBss->rAllBssGroupMultiField.u2MaxRateToken = cpu2le16(pUniCmdVowBss->rAllBssGroupMultiField.u2MaxRateToken);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, ucBssGroupID=%d\n",
		le2cpu16(pUniCmdVowBss->u2Tag), le2cpu16(pUniCmdVowBss->u2Length), pUniCmdVowBss->ucBssGroupID);

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_BSSGROUP_CTRL_1_GROUP_T);
	return Ret;

}

/*
* Unified command UNI_CMD_VOW_BSSGROUP_TOKEN_CFG (TAG 0x03) handler
*/
static INT32 UniCmdVowBssGroupTokenCtrl(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T pUniCmdVowBss = (P_UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T)pHandle;
	P_UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T pParam = &(pVowParam->VowBssgroupTokenCfg);

	memcpy(pUniCmdVowBss, pParam, sizeof(UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T));
	/* Fill TLV format */
	pUniCmdVowBss->u2Tag = cpu2le16(UNI_CMD_VOW_BSSGROUP_TOKEN_CFG);
	pUniCmdVowBss->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T));

	pUniCmdVowBss->u4SingleFieldIDValue = cpu2le32(pUniCmdVowBss->u4SingleFieldIDValue);
	pUniCmdVowBss->u4CfgItemId = cpu2le32(pUniCmdVowBss->u4CfgItemId);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, ucBssGroupID=%d, u4SingleFieldIDValue=%d, u4CfgItemId=%d\n",
		le2cpu16(pUniCmdVowBss->u2Tag), le2cpu16(pUniCmdVowBss->u2Length), pUniCmdVowBss->ucBssGroupID,
		le2cpu32(pUniCmdVowBss->u4SingleFieldIDValue), le2cpu32(pUniCmdVowBss->u4CfgItemId));

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_BSSGROUP_TOKEN_CFG_T);
	return Ret;

}

/*
* Unified command UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP (TAG 0x04) handler
*/
static INT32 UniCmdVowBssGroupCtrlAllGroup(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS, i = 0;
	P_UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T pUniCmdVowBss = (P_UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T)pHandle;
	P_UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T pParam = &(pVowParam->VowBssgroupCtrlAllGroup);

	memcpy(pUniCmdVowBss, pParam, sizeof(UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T));
	/* Fill TLV format */
	pUniCmdVowBss->u2Tag = cpu2le16(UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP);
	pUniCmdVowBss->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T));

	for (i = 0; i < UNICMD_VOW_BWC_GROUP_NUMBER; i++) {
		pUniCmdVowBss->arAllBssGroupMultiField[i].u2MinRateToken = cpu2le16(pUniCmdVowBss->arAllBssGroupMultiField[i].u2MinRateToken);
		pUniCmdVowBss->arAllBssGroupMultiField[i].u2MaxRateToken = cpu2le16(pUniCmdVowBss->arAllBssGroupMultiField[i].u2MaxRateToken);
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d\n",
		le2cpu16(pUniCmdVowBss->u2Tag), le2cpu16(pUniCmdVowBss->u2Length));

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_BSSGROUP_CTRL_ALL_GROUP_T);
	return Ret;

}

/*
* Unified command UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM (TAG 0x05) handler
*/
static INT32 UniCmdVowBssGroupBwGroupQuantum(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T pUniCmdVowBss = (P_UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T)pHandle;
	P_UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T pParam = &(pVowParam->VowBssgroupBWGroupQuantum);

	memcpy(pUniCmdVowBss, pParam, sizeof(UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T));
	/* Fill TLV format */
	pUniCmdVowBss->u2Tag = cpu2le16(UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM);
	pUniCmdVowBss->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, ucBssGroupQuantumID=%u, ucBssGroupQuantumTime=%u\n",
		le2cpu16(pUniCmdVowBss->u2Tag), le2cpu16(pUniCmdVowBss->u2Length),
		pUniCmdVowBss->ucBssGroupQuantumID, pUniCmdVowBss->ucBssGroupQuantumTime);

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_T);
	return Ret;

}

/*
* Unified command UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL (TAG 0x06) handler
*/
static INT32 UniCmdVowBssGroupBwGroupQuantumALL(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T pUniCmdVowBss = (P_UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T)pHandle;
	P_UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T pParam = &(pVowParam->VowBssgroupBWGroupQuantumAll);

	memcpy(pUniCmdVowBss, pParam, sizeof(UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T));
	/* Fill TLV format */
	pUniCmdVowBss->u2Tag = cpu2le16(UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL);
	pUniCmdVowBss->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d\n",
		le2cpu16(pUniCmdVowBss->u2Tag), le2cpu16(pUniCmdVowBss->u2Length));

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T);
	return Ret;

}

/* TAG 07-08: for airtime estimator module */
INT uni_cmd_vow_set_at_estimator(PRTMP_ADAPTER pad, UINT32 subcmd)
{
	UNI_CMD_VOW_PARAM_T VOWParam;
	P_UNI_CMD_VOW_AT_PROC_EST_FEATURE_T pVowATProcEstFeature = &(VOWParam.VowATProcEstFeature);
	P_UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T pVowATProcEstMonitorPeriod = &(VOWParam.VowATProcEstMonitorPeriod);
	INT32 ret;

	NdisZeroMemory(&VOWParam, sizeof(UNI_CMD_VOW_PARAM_T));

	switch (subcmd) {
	case ENUM_AT_PROC_EST_FEATURE_CTRL:
		pVowATProcEstFeature->fgAtEstimateOnOff = pad->vow_at_est.at_estimator_en;
		VOWParam.VOWTagValid[UNI_CMD_VOW_AT_PROC_EST_FEATURE] = TRUE;
		break;

	case ENUM_AT_PROC_EST_MONITOR_PERIOD_CTRL:
		pVowATProcEstMonitorPeriod->u2AtEstMonitorPeriod = pad->vow_at_est.at_monitor_period;
		VOWParam.VOWTagValid[UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD] = TRUE;
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(No such command = 0x%x)\n", subcmd);
	}

	ret = UniCmdVOWUpdate(pad, &VOWParam, TRUE, HOST2CR4N9, NULL);
	return ret;
}

/*
* Unified command UNI_CMD_VOW_AT_PROC_EST_FEATURE (TAG 0x07) handler
*/
static INT32 UniCmdVowATProcEstFeature(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_AT_PROC_EST_FEATURE_T pUniCmdVowATProc = (P_UNI_CMD_VOW_AT_PROC_EST_FEATURE_T)pHandle;
	P_UNI_CMD_VOW_AT_PROC_EST_FEATURE_T pParam = &(pVowParam->VowATProcEstFeature);

	memcpy(pUniCmdVowATProc, pParam, sizeof(UNI_CMD_VOW_AT_PROC_EST_FEATURE_T));
	/* Fill TLV format */
	pUniCmdVowATProc->u2Tag = cpu2le16(UNI_CMD_VOW_AT_PROC_EST_FEATURE);
	pUniCmdVowATProc->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_AT_PROC_EST_FEATURE_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, fgAtEstimateOnOff=%d\n",
		le2cpu16(pUniCmdVowATProc->u2Tag), le2cpu16(pUniCmdVowATProc->u2Length), pUniCmdVowATProc->fgAtEstimateOnOff);

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_AT_PROC_EST_FEATURE_T);
	return Ret;

}

/*
* Unified command UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD (TAG 0x08) handler
*/
static INT32 UniCmdVowATProcEstMonitorPeriod(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T pUniCmdVowATProc = (P_UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T)pHandle;
	P_UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T pParam = &(pVowParam->VowATProcEstMonitorPeriod);

	memcpy(pUniCmdVowATProc, pParam, sizeof(UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T));
	/* Fill TLV format */
	pUniCmdVowATProc->u2Tag = cpu2le16(UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD);
	pUniCmdVowATProc->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T));
	pUniCmdVowATProc->u2AtEstMonitorPeriod = cpu2le16(pUniCmdVowATProc->u2AtEstMonitorPeriod);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, u2AtEstMonitorPeriod=%d\n",
		le2cpu16(pUniCmdVowATProc->u2Tag), le2cpu16(pUniCmdVowATProc->u2Length), le2cpu16(pUniCmdVowATProc->u2AtEstMonitorPeriod));

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_AT_PROC_EST_MONITOR_PERIOD_T);
	return Ret;

}

/* TAG 09-0A */
INT uni_cmd_vow_set_at_estimator_group(PRTMP_ADAPTER pad, UINT32 subcmd, UINT8 group_id)
{
	UNI_CMD_VOW_PARAM_T VOWParam;
	P_UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T pVowATProcEstGroupRatio = &(VOWParam.VowATProcEstGroupRatio);
	P_UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T pVowATProcEstGroupToBandMapping = &(VOWParam.VowATProcEstGroupToBandMapping);
	INT32 ret;

	NdisZeroMemory(&VOWParam, sizeof(UNI_CMD_VOW_PARAM_T));
	switch (subcmd) {
	case ENUM_AT_PROC_EST_GROUP_RATIO_CTRL:
		VOWParam.VOWTagValid[UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO] = TRUE;
		pVowATProcEstGroupRatio->u4GroupRatioBitMask |= (1UL << group_id);
		pVowATProcEstGroupRatio->u2GroupMaxRatioValue[group_id] = pad->vow_bss_cfg[group_id].max_airtime_ratio;
		pVowATProcEstGroupRatio->u2GroupMinRatioValue[group_id] = pad->vow_bss_cfg[group_id].min_airtime_ratio;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(group %d, val = 0x%x/0x%x)\n",
				 group_id,
				 pad->vow_bss_cfg[group_id].max_airtime_ratio,
				 pad->vow_bss_cfg[group_id].min_airtime_ratio);
		break;

	case ENUM_AT_PROC_EST_GROUP_TO_BAND_MAPPING:
		VOWParam.VOWTagValid[UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING] = TRUE;
		pVowATProcEstGroupToBandMapping->ucGrouptoSelectBand = group_id;
		pVowATProcEstGroupToBandMapping->ucBandSelectedfromGroup = pad->vow_bss_cfg[group_id].band_idx;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(group %d, val = 0x%x)\n",
				 group_id, pad->vow_bss_cfg[group_id].band_idx);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(No such command = 0x%x)\n", subcmd);
	}

	ret = UniCmdVOWUpdate(pad, &VOWParam, TRUE, HOST2CR4N9, NULL);
	return ret;
}

/*
* Unified command UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING (TAG 0x09) handler
*/
static INT32 UniCmdVowATProcEstGroupRatio(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS, i = 0;
	P_UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T pUniCmdVowATProc = (P_UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T)pHandle;
	P_UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T pParam = &(pVowParam->VowATProcEstGroupRatio);

	memcpy(pUniCmdVowATProc, pParam, sizeof(UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T));
	/* Fill TLV format */
	pUniCmdVowATProc->u2Tag = cpu2le16(UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO);
	pUniCmdVowATProc->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T));
	pUniCmdVowATProc->u4GroupRatioBitMask = cpu2le16(pUniCmdVowATProc->u4GroupRatioBitMask);
	for (i = 0; i < UNICMD_VOW_BWC_GROUP_NUMBER; i++) {
		pUniCmdVowATProc->u2GroupMaxRatioValue[i] = cpu2le16(pUniCmdVowATProc->u2GroupMaxRatioValue[i]);
		pUniCmdVowATProc->u2GroupMinRatioValue[i] = cpu2le16(pUniCmdVowATProc->u2GroupMinRatioValue[i]);
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, u4GroupRatioBitMask=%d\n",
		le2cpu16(pUniCmdVowATProc->u2Tag), le2cpu16(pUniCmdVowATProc->u2Length), le2cpu32(pUniCmdVowATProc->u4GroupRatioBitMask));

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_AT_PROC_EST_GROUP_RATIO_T);
	return Ret;

}

/*
* Unified command UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING (TAG 0x0A) handler
*/
static INT32 UniCmdVowATProcEstGroupToBandMapping(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T pUniCmdVowATProc = (P_UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T)pHandle;
	P_UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T pParam = &(pVowParam->VowATProcEstGroupToBandMapping);

	memcpy(pUniCmdVowATProc, pParam, sizeof(UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T));
	/* Fill TLV format */
	pUniCmdVowATProc->u2Tag = cpu2le16(UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING);
	pUniCmdVowATProc->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, ucGrouptoSelectBand=%d, ucBandSelectedfromGroup=%d\n",
		le2cpu16(pUniCmdVowATProc->u2Tag), le2cpu16(pUniCmdVowATProc->u2Length),
		pUniCmdVowATProc->ucGrouptoSelectBand, pUniCmdVowATProc->ucBandSelectedfromGroup);

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T);
	return Ret;

}

/* Tag B, D, E, 11 */
INT uni_cmd_vow_set_rx_airtime(PRTMP_ADAPTER pad, UINT8 cmd, UINT32 subcmd)
{
	UNI_CMD_VOW_PARAM_T VOWParam;
	P_UNI_CMD_VOW_RX_AT_AIRTIME_EN_T pVowRxAtAirtimeEn = &(VOWParam.VowRxAtAirtimeEn); /* TAG B */
	P_UNI_CMD_VOW_RX_AT_EARLYEND_EN_T pVowRxAtEarlyendEn = &(VOWParam.VowRxAtEarlyendEn); /* TAG D */
	P_UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T pVowRxAtAirtimeClrEn = &(VOWParam.VowRxAtAirtimeClrEn); /* TAG E */
	P_UNI_CMD_VOW_RX_AT_ED_OFFSET_T pVowRxAtEdOffset = &(VOWParam.VowRxAtEdOffset); /* TAG 11 */
	INT32 ret;

	NdisZeroMemory(&VOWParam, sizeof(UNI_CMD_VOW_PARAM_T));
	switch (cmd) {
	/* RX airtime feature control */
	case ENUM_RX_AT_FEATURE_CTRL:
		switch (subcmd) {
		case ENUM_RX_AT_FEATURE_SUB_TYPE_AIRTIME_EN:
			VOWParam.VOWTagValid[UNI_CMD_VOW_RX_AT_AIRTIME_EN] = TRUE;
			pVowRxAtAirtimeEn->fgRxAirTimeEn = pad->vow_rx_time_cfg.rx_time_en;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(cmd = 0x%x, subcmd = 0x%x, value = 0x%x)\n",
					 cmd, subcmd, pad->vow_rx_time_cfg.rx_time_en);
			break;

		case ENUM_RX_AT_FEATURE_SUB_TYPE_MIBTIME_EN:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(Not implemented yet = 0x%x)\n", subcmd);
			break;

		case ENUM_RX_AT_FEATURE_SUB_TYPE_EARLYEND_EN:
			VOWParam.VOWTagValid[UNI_CMD_VOW_RX_AT_EARLYEND_EN] = TRUE;
			pVowRxAtEarlyendEn->fgRxEarlyEndEn = pad->vow_rx_time_cfg.rx_early_end_en;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(cmd = 0x%x, subcmd = 0x%x, value = 0x%x)\n",
					 cmd, subcmd, pad->vow_rx_time_cfg.rx_early_end_en);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(No such sub command = 0x%x)\n", subcmd);
		}

		break;

	case ENUM_RX_AT_BITWISE_CTRL:
		switch (subcmd) {
		case ENUM_RX_AT_BITWISE_SUB_TYPE_AIRTIME_CLR: /* clear all RX airtime counters */
			VOWParam.VOWTagValid[UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN] = TRUE;
			pVowRxAtAirtimeClrEn->fgRxAirTimeClrEn = TRUE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(cmd = 0x%x, subcmd = 0x%x, value = 0x%x)\n",
					 cmd, subcmd, pVowRxAtAirtimeClrEn->fgRxAirTimeClrEn);
			break;

		case ENUM_RX_AT_BITWISE_SUB_TYPE_MIBTIME_CLR:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(Not implemented yet = 0x%x)\n", subcmd);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(No such sub command = 0x%x)\n", subcmd);
		}

		break;

	case ENUM_RX_AT_TIMER_VALUE_CTRL:
		switch (subcmd) {
		case ENUM_RX_AT_TIME_VALUE_SUB_TYPE_ED_OFFSET_CTRL:
			VOWParam.VOWTagValid[UNI_CMD_VOW_RX_AT_ED_OFFSET] = TRUE;
			pVowRxAtEdOffset->ucEdOffsetValue = pad->vow_rx_time_cfg.ed_offset;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(cmd = 0x%x, subcmd =  0x%x, value = 0x%x)\n",
					 cmd, subcmd, pad->vow_rx_time_cfg.ed_offset);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(No such sub command = 0x%x)\n", subcmd);
		}

		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(No such command = 0x%x)\n", subcmd);
		break;
	}

	ret = UniCmdVOWUpdate(pad, &VOWParam, TRUE, HOST2CR4N9, NULL);
	return ret;
}

/* Tag 0xF: select RX WMM backoff time for 4 OM */
INT uni_cmd_vow_set_wmm_selection(PRTMP_ADAPTER pad, UINT8 om)
{
	UNI_CMD_VOW_PARAM_T VOWParam;
	P_UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T pVowRxAtStaWmmCtrl = &(VOWParam.VowRxAtStaWmmCtrl); /* TAG F */
	INT32 ret;

	/* init structure to zero */
	NdisZeroMemory(&VOWParam, sizeof(UNI_CMD_VOW_PARAM_T));
	/* assign cmd and subcmd */
	VOWParam.VOWTagValid[UNI_CMD_VOW_RX_AT_STA_WMM_CTRL] = TRUE;
	pVowRxAtStaWmmCtrl->ucOwnMacID = om;
	pVowRxAtStaWmmCtrl->fgtoApplyWm00to03MibCfg = pad->vow_rx_time_cfg.wmm_backoff_sel[om];
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(cmd = 0x%x, subcmd = 0x%x, OM = 0x%x, Map = 0x%x)\n",
			 pVowRxAtStaWmmCtrl->ucOwnMacID,
			 pVowRxAtStaWmmCtrl->fgtoApplyWm00to03MibCfg);

	ret = UniCmdVOWUpdate(pad, &VOWParam, TRUE, HOST2CR4N9, NULL);
	return ret;
}

/* Tag 0x10: set 16 MBSS  mapping to 4 RX backoff time configurations */
INT uni_cmd_vow_set_mbss2wmm_map(PRTMP_ADAPTER pad, UINT8 bss_idx)
{
	UNI_CMD_VOW_PARAM_T VOWParam;
	P_UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T pVowRxAtMbssWmmCtrl = &(VOWParam.VowRxAtMbssWmmCtrl); /* TAG 10 */
	INT32 ret;

	/* init structure to zero */
	NdisZeroMemory(&VOWParam, sizeof(UNI_CMD_VOW_PARAM_T));
	/* assign cmd and subcmd */
	VOWParam.VOWTagValid[UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL] = TRUE;
	pVowRxAtMbssWmmCtrl->ucMbssGroup = bss_idx;
	pVowRxAtMbssWmmCtrl->ucWmmGroup = pad->vow_rx_time_cfg.bssid2wmm_set[bss_idx];
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(bss_idx = 0x%x, Map = 0x%x)\n",
			 bss_idx, pad->vow_rx_time_cfg.bssid2wmm_set[bss_idx]);

	ret = UniCmdVOWUpdate(pad, &VOWParam, TRUE, HOST2CR4N9, NULL);
	return ret;
}

/* Tag 0x13: set backoff time for RX*/
INT uni_cmd_vow_set_backoff_time(PRTMP_ADAPTER pad, UINT8 target)
{
	UNI_CMD_VOW_PARAM_T VOWParam;
	P_UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T pVowRxAtBackoffTimer = &(VOWParam.VowRxAtBackoffTimer); /* TAG 13 */
	INT32 ret;

	/* init structure to zero */
	NdisZeroMemory(&VOWParam, sizeof(UNI_CMD_VOW_PARAM_T));
	/* assign cmd and subcmd */
	VOWParam.VOWTagValid[UNI_CMD_VOW_RX_AT_BACKOFF_TIMER] = TRUE;
	pVowRxAtBackoffTimer->ucRxATBackoffWmmGroupIdx = target;

	switch (target) {
	case ENUM_RX_AT_WMM_GROUP_0:
	case ENUM_RX_AT_WMM_GROUP_1:
	case ENUM_RX_AT_WMM_GROUP_2:
	case ENUM_RX_AT_WMM_GROUP_3:
		pVowRxAtBackoffTimer->u2AC0Backoff =
			pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_BK];
		pVowRxAtBackoffTimer->u2AC1Backoff =
			pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_BE];
		pVowRxAtBackoffTimer->u2AC2Backoff =
			pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_VI];
		pVowRxAtBackoffTimer->u2AC3Backoff =
			pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_VO];
		pVowRxAtBackoffTimer->ucRxAtBackoffAcQMask =
			(ENUM_RX_AT_AC_Q0_MASK_T | ENUM_RX_AT_AC_Q1_MASK_T | ENUM_RX_AT_AC_Q2_MASK_T | ENUM_RX_AT_AC_Q3_MASK_T);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(group = 0x%x, BK = 0x%x, BE = 0x%x, VI = 0x%x, VO = 0x%x)\n",
				 target,
				 pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_BK],
				 pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_BE],
				 pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_VI],
				 pad->vow_rx_time_cfg.wmm_backoff[target][WMM_AC_VO]);
		break;

	case ENUM_RX_AT_WMM_GROUP_PEPEATER:
		pVowRxAtBackoffTimer->u2AC0Backoff =
			pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BK];
		pVowRxAtBackoffTimer->u2AC1Backoff =
			pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BE];
		pVowRxAtBackoffTimer->u2AC2Backoff =
			pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VI];
		pVowRxAtBackoffTimer->u2AC3Backoff =
			pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VO];
		pVowRxAtBackoffTimer->ucRxAtBackoffAcQMask =
			(ENUM_RX_AT_AC_Q0_MASK_T | ENUM_RX_AT_AC_Q1_MASK_T | ENUM_RX_AT_AC_Q2_MASK_T | ENUM_RX_AT_AC_Q3_MASK_T);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(group = 0x%x, BK = 0x%x, BE = 0x%x, VI = 0x%x, VO = 0x%x)\n",
				 target,
				 pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BK],
				 pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_BE],
				 pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VI],
				 pad->vow_rx_time_cfg.repeater_wmm_backoff[WMM_AC_VO]);
		break;

	case ENUM_RX_AT_WMM_GROUP_STA:
		pVowRxAtBackoffTimer->u2AC0Backoff =
			pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BK];
		pVowRxAtBackoffTimer->u2AC1Backoff =
			pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BE];
		pVowRxAtBackoffTimer->u2AC2Backoff =
			pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VI];
		pVowRxAtBackoffTimer->u2AC3Backoff =
			pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VO];
		pVowRxAtBackoffTimer->ucRxAtBackoffAcQMask =
			(ENUM_RX_AT_AC_Q0_MASK_T | ENUM_RX_AT_AC_Q1_MASK_T | ENUM_RX_AT_AC_Q2_MASK_T | ENUM_RX_AT_AC_Q3_MASK_T);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(group = 0x%x, BK = 0x%x, BE = 0x%x, VI = 0x%x, VO = 0x%x)\n",
				 target,
				 pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BK],
				 pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_BE],
				 pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VI],
				 pad->vow_rx_time_cfg.om_wmm_backoff[WMM_AC_VO]);
		break;

	case ENUM_RX_AT_NON_QOS:
		pVowRxAtBackoffTimer->u2AC0Backoff =
			pad->vow_rx_time_cfg.non_qos_backoff;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(group = 0x%x, backoff time = 0x%x)\n",
				 target, pad->vow_rx_time_cfg.non_qos_backoff);
		break;

	case ENUM_RX_AT_OBSS:
		pVowRxAtBackoffTimer->u2AC0Backoff =
			pad->vow_rx_time_cfg.obss_backoff;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(group = 0x%x, backoff time = 0x%x)\n",
				 target, pad->vow_rx_time_cfg.obss_backoff);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(No such command = 0x%x)\n", target);
		break;
	}

	ret = UniCmdVOWUpdate(pad, &VOWParam, TRUE, HOST2CR4N9, NULL);
	return ret;
}

/* TAG 14, 15: set backoff time for RX*/
INT uni_cmd_vow_get_rx_time_counter(PRTMP_ADAPTER pad, UINT8 target, UINT8 band_idx)
{
	UNI_CMD_VOW_PARAM_T VOWParam;
	UNI_EVENT_VOW_PARAM_T VOWResult;
	P_UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T pVowRxAtReportRxNonwifiTime = &(VOWParam.VowRxAtReportRxNonwifiTime); /* CMD TAG 14 */
	P_UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME_T pVowRxAtReportRxObssTime = &(VOWParam.VowRxAtReportRxObssTime); /* CMD TAG 15 */
	P_UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T pEventVowRxAtReportRxNonwifiTime = &(VOWResult.EventVowRxAtReportRxNonwifiTime); /* Event TAG 14 */
	P_UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME_T pEventVowRxAtReportRxObssTime = &(VOWResult.EventVowRxAtReportRxObssTime); /* Event TAG 15 */
	INT32 ret;

	/* init structure to zero */
	NdisZeroMemory(&VOWParam, sizeof(UNI_CMD_VOW_PARAM_T));
	/* assign cmd and subcmd */

	switch (target) {
	case ENUM_RX_AT_REPORT_SUB_TYPE_RX_NONWIFI_TIME:
		VOWParam.VOWTagValid[UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME] = TRUE;
		pVowRxAtReportRxNonwifiTime->ucRxNonWiFiBandIdx = band_idx;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(target = 0x%x, band_idx = 0x%x)\n",
				 target, band_idx);
		break;

	case ENUM_RX_AT_REPORT_SUB_TYPE_RX_OBSS_TIME:
		VOWParam.VOWTagValid[UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME] = TRUE;
		pVowRxAtReportRxObssTime->ucRxObssBandIdx = band_idx;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(target = 0x%x, band_idx = 0x%x)\n",
				 target, band_idx);
		break;

	case ENUM_RX_AT_REPORT_SUB_TYPE_MIB_OBSS_TIME:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(Not implemented yet = 0x%x)\n", target);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(No such command = 0x%x)\n", target);
	}

	ret = UniCmdVOWUpdate(pad, &VOWParam, FALSE, HOST2CR4N9, &VOWResult);

	if (target == ENUM_RX_AT_REPORT_SUB_TYPE_RX_NONWIFI_TIME)
		return pEventVowRxAtReportRxNonwifiTime->u4RxNonWiFiBandTimer;
	else if (target == ENUM_RX_AT_REPORT_SUB_TYPE_RX_OBSS_TIME)
		return pEventVowRxAtReportRxObssTime->u4RxObssBandTimer;
	else
		return -1;
}

/*
* Unified command UNI_CMD_VOW_RX_AT_AIRTIME_EN (TAG 0x0B) handler
*/
static INT32 UniCmdVowRxATAirtimeEn(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_RX_AT_AIRTIME_EN_T pUniCmdVowRxAt = (P_UNI_CMD_VOW_RX_AT_AIRTIME_EN_T)pHandle;
	P_UNI_CMD_VOW_RX_AT_AIRTIME_EN_T pParam = &(pVowParam->VowRxAtAirtimeEn);

	memcpy(pUniCmdVowRxAt, pParam, sizeof(UNI_CMD_VOW_RX_AT_AIRTIME_EN_T));
	/* Fill TLV format */
	pUniCmdVowRxAt->u2Tag = cpu2le16(UNI_CMD_VOW_RX_AT_AIRTIME_EN);
	pUniCmdVowRxAt->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_RX_AT_AIRTIME_EN_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"%s(): u2Tag=%d, u2Length=%d, fgRxAirTimeEn=%d\n",
		le2cpu16(pUniCmdVowRxAt->u2Tag), le2cpu16(pUniCmdVowRxAt->u2Length),
		pUniCmdVowRxAt->fgRxAirTimeEn);

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_RX_AT_AIRTIME_EN_T);
	return Ret;

}

/*
* Unified command UNI_CMD_VOW_RX_AT_MIBTIME_EN (TAG 0x0C) handler
*/
static INT32 UniCmdVowRxATMibtimeEn(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_RX_AT_MIBTIME_EN_T pUniCmdVowRxAt = (P_UNI_CMD_VOW_RX_AT_MIBTIME_EN_T)pHandle;
	P_UNI_CMD_VOW_RX_AT_MIBTIME_EN_T pParam = &(pVowParam->VowRxAtMibtimeEn);

	memcpy(pUniCmdVowRxAt, pParam, sizeof(UNI_CMD_VOW_RX_AT_MIBTIME_EN_T));
	/* Fill TLV format */
	pUniCmdVowRxAt->u2Tag = cpu2le16(UNI_CMD_VOW_RX_AT_MIBTIME_EN);
	pUniCmdVowRxAt->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_RX_AT_MIBTIME_EN_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, fgRxMibTimeEn=%d\n",
		le2cpu16(pUniCmdVowRxAt->u2Tag), le2cpu16(pUniCmdVowRxAt->u2Length),
		pUniCmdVowRxAt->fgRxMibTimeEn);

	return Ret;

}

/*
* Unified command UNI_CMD_VOW_RX_AT_EARLYEND_EN (TAG 0x0D) handler
*/
static INT32 UniCmdVowRxATEarlyendEn(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_RX_AT_EARLYEND_EN_T pUniCmdVowRxAt = (P_UNI_CMD_VOW_RX_AT_EARLYEND_EN_T)pHandle;
	P_UNI_CMD_VOW_RX_AT_EARLYEND_EN_T pParam = &(pVowParam->VowRxAtEarlyendEn);

	memcpy(pUniCmdVowRxAt, pParam, sizeof(UNI_CMD_VOW_RX_AT_EARLYEND_EN_T));
	/* Fill TLV format */
	pUniCmdVowRxAt->u2Tag = cpu2le16(UNI_CMD_VOW_RX_AT_EARLYEND_EN);
	pUniCmdVowRxAt->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_RX_AT_EARLYEND_EN_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, fgRxEarlyEndEn=%d\n",
		le2cpu16(pUniCmdVowRxAt->u2Tag), le2cpu16(pUniCmdVowRxAt->u2Length),
		pUniCmdVowRxAt->fgRxEarlyEndEn);

	return Ret;

}

/*
* Unified command UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN (TAG 0x0E) handler
*/
static INT32 UniCmdVowRxAtAirtimeClrEn(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T pUniCmdVowRxAt = (P_UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T)pHandle;
	P_UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T pParam = &(pVowParam->VowRxAtAirtimeClrEn);

	memcpy(pUniCmdVowRxAt, pParam, sizeof(UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T));
	/* Fill TLV format */
	pUniCmdVowRxAt->u2Tag = cpu2le16(UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN);
	pUniCmdVowRxAt->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, fgRxAirTimeClrEn=%d\n",
		le2cpu16(pUniCmdVowRxAt->u2Tag), le2cpu16(pUniCmdVowRxAt->u2Length),
		pUniCmdVowRxAt->fgRxAirTimeClrEn);

	return Ret;

}

/*
* Unified command UNI_CMD_VOW_RX_AT_STA_WMM_CTRL (TAG 0x0F) handler
*/
static INT32 UniCmdVowRxAtStaWmmCtrl(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T pUniCmdVowRxAt = (P_UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T)pHandle;
	P_UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T pParam = &(pVowParam->VowRxAtStaWmmCtrl);

	memcpy(pUniCmdVowRxAt, pParam, sizeof(UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T));
	/* Fill TLV format */
	pUniCmdVowRxAt->u2Tag = cpu2le16(UNI_CMD_VOW_RX_AT_STA_WMM_CTRL);
	pUniCmdVowRxAt->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, ucOwnMacID=%d, fgtoApplyWm00to03MibCfg=%d\n",
		le2cpu16(pUniCmdVowRxAt->u2Tag), le2cpu16(pUniCmdVowRxAt->u2Length),
		pUniCmdVowRxAt->ucOwnMacID, pUniCmdVowRxAt->fgtoApplyWm00to03MibCfg);

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_RX_AT_STA_WMM_CTRL_T);
	return Ret;

}

/*
* Unified command UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL (TAG 0x10) handler
*/
static INT32 UniCmdVowRxAtMbssWmmCtrl(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T pUniCmdVowRxAt = (P_UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T)pHandle;
	P_UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T pParam = &(pVowParam->VowRxAtMbssWmmCtrl);

	memcpy(pUniCmdVowRxAt, pParam, sizeof(UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T));
	/* Fill TLV format */
	pUniCmdVowRxAt->u2Tag = cpu2le16(UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL);
	pUniCmdVowRxAt->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, ucMbssGroup=%d, ucWmmGroup=%d\n",
		le2cpu16(pUniCmdVowRxAt->u2Tag), le2cpu16(pUniCmdVowRxAt->u2Length),
		pUniCmdVowRxAt->ucMbssGroup, pUniCmdVowRxAt->ucWmmGroup);

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_RX_AT_MBSS_WMM_CTRL_T);
	return Ret;
}

/*
* Unified command UNI_CMD_VOW_RX_AT_ED_OFFSET (TAG 0x11) handler
*/
static INT32 UniCmdVowRxAtEdOffset(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_RX_AT_ED_OFFSET_T pUniCmdVowRxAt = (P_UNI_CMD_VOW_RX_AT_ED_OFFSET_T)pHandle;
	P_UNI_CMD_VOW_RX_AT_ED_OFFSET_T pParam = &(pVowParam->VowRxAtEdOffset);

	memcpy(pUniCmdVowRxAt, pParam, sizeof(UNI_CMD_VOW_RX_AT_ED_OFFSET_T));
	/* Fill TLV format */
	pUniCmdVowRxAt->u2Tag = cpu2le16(UNI_CMD_VOW_RX_AT_ED_OFFSET);
	pUniCmdVowRxAt->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_RX_AT_ED_OFFSET_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, ucEdOffsetValue=%d\n",
		le2cpu16(pUniCmdVowRxAt->u2Tag), le2cpu16(pUniCmdVowRxAt->u2Length),
		pUniCmdVowRxAt->ucEdOffsetValue);

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_RX_AT_ED_OFFSET_T);
	return Ret;
}

/*
* Unified command UNI_CMD_VOW_RX_AT_SW_TIMER (TAG 0x12) handler
*/
static INT32 UniCmdVowRxAtSwTimer(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_RX_AT_SW_TIMER_T pUniCmdVowRxAt = (P_UNI_CMD_VOW_RX_AT_SW_TIMER_T)pHandle;
	P_UNI_CMD_VOW_RX_AT_SW_TIMER_T pParam = &(pVowParam->VowRxAtSwTimer);

	memcpy(pUniCmdVowRxAt, pParam, sizeof(UNI_CMD_VOW_RX_AT_SW_TIMER_T));
	/* Fill TLV format */
	pUniCmdVowRxAt->u2Tag = cpu2le16(UNI_CMD_VOW_RX_AT_SW_TIMER);
	pUniCmdVowRxAt->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_RX_AT_SW_TIMER_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, ucCompensateMode=%d, ucRxBand=%d, ucSwCompensateTimeValue=%d\n",
		le2cpu16(pUniCmdVowRxAt->u2Tag), le2cpu16(pUniCmdVowRxAt->u2Length),
		pUniCmdVowRxAt->ucCompensateMode, pUniCmdVowRxAt->ucRxBand, pUniCmdVowRxAt->ucSwCompensateTimeValue);

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_RX_AT_SW_TIMER_T);
	return Ret;
}

/*
* Unified command UNI_CMD_VOW_RX_AT_BACKOFF_TIMER (TAG 0x13) handler
*/
static INT32 UniCmdVowRxAtBackoffTimer(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T pUniCmdVowRxAt = (P_UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T)pHandle;
	P_UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T pParam = &(pVowParam->VowRxAtBackoffTimer);

	memcpy(pUniCmdVowRxAt, pParam, sizeof(UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T));
	/* Fill TLV format */
	pUniCmdVowRxAt->u2Tag = cpu2le16(UNI_CMD_VOW_RX_AT_BACKOFF_TIMER);
	pUniCmdVowRxAt->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T));

	pUniCmdVowRxAt->u2AC0Backoff = cpu2le16(pUniCmdVowRxAt->u2AC0Backoff);
	pUniCmdVowRxAt->u2AC1Backoff = cpu2le16(pUniCmdVowRxAt->u2AC1Backoff);
	pUniCmdVowRxAt->u2AC2Backoff = cpu2le16(pUniCmdVowRxAt->u2AC2Backoff);
	pUniCmdVowRxAt->u2AC3Backoff = cpu2le16(pUniCmdVowRxAt->u2AC3Backoff);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, ACBackoff=%d-%d-%d-%d, ucRxATBackoffWmmGroupIdx=%d, ucRxAtBackoffAcQMask=%d\n",
		le2cpu16(pUniCmdVowRxAt->u2Tag), le2cpu16(pUniCmdVowRxAt->u2Length),
		le2cpu16(pUniCmdVowRxAt->u2AC0Backoff),
		le2cpu16(pUniCmdVowRxAt->u2AC1Backoff),
		le2cpu16(pUniCmdVowRxAt->u2AC2Backoff),
		le2cpu16(pUniCmdVowRxAt->u2AC3Backoff),
		pUniCmdVowRxAt->ucRxATBackoffWmmGroupIdx, pUniCmdVowRxAt->ucRxAtBackoffAcQMask);

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_RX_AT_BACKOFF_TIMER_T);
	return Ret;
}

/*
* Unified command UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME (TAG 0x14) handler
*/
static INT32 UniCmdVowRxAtReportRxNonwifiTime(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T pUniCmdVowRxAt = (P_UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T)pHandle;
	P_UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T pParam = &(pVowParam->VowRxAtReportRxNonwifiTime);

	memcpy(pUniCmdVowRxAt, pParam, sizeof(UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T));
	/* Fill TLV format */
	pUniCmdVowRxAt->u2Tag = cpu2le16(UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME);
	pUniCmdVowRxAt->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, ucRxNonWiFiBandIdx=%d\n",
		le2cpu16(pUniCmdVowRxAt->u2Tag), le2cpu16(pUniCmdVowRxAt->u2Length),
		pUniCmdVowRxAt->ucRxNonWiFiBandIdx);

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T);
	return Ret;
}

/*
* Unified command UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME (TAG 0x15) handler
*/
static INT32 UniCmdVowRxAtReportRxObssTime(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME_T pUniCmdVowRxAt = (P_UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME_T)pHandle;
	P_UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME_T pParam = &(pVowParam->VowRxAtReportRxObssTime);

	memcpy(pUniCmdVowRxAt, pParam, sizeof(UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME_T));
	/* Fill TLV format */
	pUniCmdVowRxAt->u2Tag = cpu2le16(UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME);
	pUniCmdVowRxAt->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, ucRxObssBandIdx=%d\n",
		le2cpu16(pUniCmdVowRxAt->u2Tag), le2cpu16(pUniCmdVowRxAt->u2Length),
		pUniCmdVowRxAt->ucRxObssBandIdx);

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME_T);
	return Ret;
}

/*
* Unified command UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME (TAG 0x16) handler
*/
static INT32 UniCmdVowRxAtReportMibObssTime(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T pUniCmdVowRxAt = (P_UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T)pHandle;
	P_UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T pParam = &(pVowParam->VowRxAtReportMibObssTime);

	memcpy(pUniCmdVowRxAt, pParam, sizeof(UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T));
	/* Fill TLV format */
	pUniCmdVowRxAt->u2Tag = cpu2le16(UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME);
	pUniCmdVowRxAt->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, ucRxMibObssBandIdx=%d\n",
		le2cpu16(pUniCmdVowRxAt->u2Tag), le2cpu16(pUniCmdVowRxAt->u2Length),
		pUniCmdVowRxAt->ucRxMibObssBandIdx);

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T);
	return Ret;
}

/*
* Unified command UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME (TAG 0x17) handler
*/
static INT32 UniCmdVowRxAtReportPerStaRxTime(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T pUniCmdVowRxAt = (P_UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T)pHandle;
	P_UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T pParam = &(pVowParam->VowRxAtReportPerStaRxTime);

	memcpy(pUniCmdVowRxAt, pParam, sizeof(UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T));
	/* Fill TLV format */
	pUniCmdVowRxAt->u2Tag = cpu2le16(UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME);
	pUniCmdVowRxAt->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T));
	pUniCmdVowRxAt->u2StaId = cpu2le16(pUniCmdVowRxAt->u2StaId);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, u2StaId=%d\n",
		le2cpu16(pUniCmdVowRxAt->u2Tag), le2cpu16(pUniCmdVowRxAt->u2Length),
		le2cpu16(pUniCmdVowRxAt->u2StaId));

	(*u4RespStructSize) += sizeof(UNI_EVENT_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T);
	return Ret;
}

/* TAG 18 */
INT32 UniCmdSetRedEnable(RTMP_ADAPTER *pAd, UINT8 McuDest, UINT32 en)
{
	UNI_CMD_VOW_PARAM_T VOWParam;
	P_UNI_CMD_VOW_RED_ENABLE_T pVowRedEnable = &(VOWParam.VowRedEnable); /* TAG 0x18 */
	UINT8 UniCmdMcuDest;
	UINT ret;
	/* init structure to zero */
	NdisZeroMemory(&VOWParam, sizeof(UNI_CMD_VOW_PARAM_T));

	pVowRedEnable->ucRedEnable = en;
	UniCmdMcuDest = McuDest==HOST2CR4 ? HOST2CR4 : HOST2CR4N9;
	VOWParam.VOWTagValid[UNI_CMD_VOW_RED_ENABLE] = TRUE;
	ret = UniCmdVOWUpdate(pAd, &VOWParam, TRUE, HOST2CR4N9, NULL);

	return ret;
}

/*
* Unified command UNI_CMD_VOW_RED_ENABLE (TAG 0x18) handler
*/
static INT32 UniCmdVowRedEnable(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS;
	P_UNI_CMD_VOW_RED_ENABLE_T pUniCmdVowRed = (P_UNI_CMD_VOW_RED_ENABLE_T)pHandle;
	P_UNI_CMD_VOW_RED_ENABLE_T pParam = &(pVowParam->VowRedEnable);

	memcpy(pUniCmdVowRed, pParam, sizeof(UNI_CMD_VOW_RED_ENABLE_T));
	/* Fill TLV format */
	pUniCmdVowRed->u2Tag = cpu2le16(UNI_CMD_VOW_RED_ENABLE);
	pUniCmdVowRed->u2Length = cpu2le16(sizeof(UNI_CMD_VOW_RED_ENABLE_T));

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, ucRedEnable=%d\n",
		le2cpu16(pUniCmdVowRed->u2Tag), le2cpu16(pUniCmdVowRed->u2Length), pUniCmdVowRed->ucRedEnable);

	return Ret;
}

VOID UniCmdExtEventRedTxReportHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	INT32 i;
	UINT32 sta_num = 0;
	UINT8 BandIdx = 0;
	P_UNI_PARSE_EXT_EVENT_RED_TX_RPT_T prTxRptEvt;
	P_UNI_PARSE_RED_TX_RPT_T prTxRpt	= NULL;
	UINT32 *staInUseBitmap;
	PMAC_TABLE_ENTRY pEntry = NULL;
	P_UNI_CMD_VOW_PARAM_T pVOWParam;
	P_UNI_CMD_VOW_RED_TX_RPT_T pVowRedTxRpt; /* TAG 0x18: */
	P_UNI_CMD_RED_TX_RPT_T pUniCmdRedTxRpt;
	UINT8 TxRptCount = 0;

	prTxRptEvt = (P_UNI_PARSE_EXT_EVENT_RED_TX_RPT_T) Data;
	staInUseBitmap = &prTxRptEvt->staInUseBitmap[0];
	prTxRpt = (P_UNI_PARSE_RED_TX_RPT_T)(Data + sizeof(UNI_PARSE_EXT_EVENT_RED_TX_RPT_T));

	sta_num = prTxRptEvt->wordlen << 5;

	for (i = 1; i < sta_num; i++) {
		UINT32 th;

		if ((staInUseBitmap[i >> RED_INUSE_BITSHIFT] & (1 << (i & RED_INUSE_BITMASK))) == 0)
			continue;

		TxRptCount++;
		pEntry = &pAd->MacTab.Content[i];
		if (pEntry && pEntry->wdev &&  IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
			BandIdx = HcGetBandByWdev(pEntry->wdev);

			th = VERIWAVE_5G_PKT_CNT_TH;
			if (WMODE_CAP_2G(pEntry->wdev->PhyMode))
				th = VERIWAVE_2G_PKT_CNT_TH;

			if ((prTxRpt->u4TCPCnt > th) || (prTxRpt->u4TCPAckCnt > th))
				pAd->txop_ctl[BandIdx].multi_tcp_nums++;

#ifdef VOW_SUPPORT
			if (pAd->vow_cfg.mcli_schedule_en) {
				pEntry->mcliTcpCnt += prTxRpt->u4TCPCnt;
				pEntry->mcliTcpAckCnt += prTxRpt->u4TCPAckCnt;
			}
#endif
		}
		prTxRpt++;
	}

	os_alloc_mem(pAd, (UCHAR **)&pVOWParam, TxRptCount*sizeof(UNI_CMD_RED_TX_RPT_T) + sizeof(UNI_CMD_VOW_PARAM_T));
	pVowRedTxRpt = &(pVOWParam->VowRedTxRpt);
	pUniCmdRedTxRpt = pVowRedTxRpt->arTxRpt;

	pVowRedTxRpt->ucWordlen = prTxRptEvt->wordlen;
	memcpy(&(pVowRedTxRpt->u4StaInUseBitmap), &(prTxRptEvt->staInUseBitmap), sizeof(pVowRedTxRpt->u4StaInUseBitmap));
	for (i = 1; i < sta_num; i++) {
		if ((staInUseBitmap[i >> RED_INUSE_BITSHIFT] & (1 << (i & RED_INUSE_BITMASK))) == 0)
			continue;

		pUniCmdRedTxRpt->u4TCPAckCnt = cpu2le32(prTxRpt->u4TCPAckCnt);
		pUniCmdRedTxRpt->u4TCPCnt = cpu2le32(prTxRpt->u4TCPCnt);

		prTxRpt++;
		pUniCmdRedTxRpt++;
	}
	pVOWParam->VOWTagValid[UNI_CMD_VOW_RED_TX_RPT] = TRUE;
	pVowRedTxRpt->u2Length = TxRptCount*sizeof(UNI_CMD_RED_TX_RPT_T) + sizeof(UNI_CMD_VOW_RED_TX_RPT_T);
	return UniCmdVOWUpdate(pAd, pVOWParam, TRUE, HOST2CR4N9, NULL);
}

/*
* Unified command UNI_CMD_VOW_RED_TX_RPT (TAG 0x19) handler
*/
static INT32 UniCmdVowRedTxRpt(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVowParam,
	VOID *pHandle,
	UINT32 *u4RespStructSize)
{
	INT32 Ret = NDIS_STATUS_SUCCESS, i;
	P_UNI_CMD_VOW_RED_TX_RPT_T pUniCmdVowRed = (P_UNI_CMD_VOW_RED_TX_RPT_T)pHandle;
	P_UNI_CMD_VOW_RED_TX_RPT_T pParam = &(pVowParam->VowRedTxRpt);

	memcpy(pUniCmdVowRed, pParam, sizeof(UNI_CMD_VOW_RED_TX_RPT_T));
	/* Fill TLV format */
	pUniCmdVowRed->u2Tag = cpu2le16(UNI_CMD_VOW_RED_TX_RPT);
	pUniCmdVowRed->u2Length = cpu2le16(pUniCmdVowRed->u2Length);

	for (i = 0; i < 32; i++){
		pUniCmdVowRed->u4StaInUseBitmap[i] = cpu2le32(pUniCmdVowRed->u4StaInUseBitmap[i]);
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"u2Tag=%d, u2Length=%d, ucWordlen=%d\n",
		le2cpu16(pUniCmdVowRed->u2Tag), le2cpu16(pUniCmdVowRed->u2Length), pUniCmdVowRed->ucWordlen);

	return Ret;
}

static UNI_CMD_TAG_HANDLE_T UniCmdVowTab[] = {
	{
		.u2CmdFeature = UNI_CMD_VOW_DRR_CTRL,
		.u4StructSize = sizeof(UNI_CMD_VOW_DRR_CTRL_T),
		.pfHandler = UniCmdVowDrrCtrl
	},

	{
		.u2CmdFeature = UNI_CMD_VOW_FEATURE_CTRL,
		.u4StructSize = sizeof(UNI_CMD_VOW_FEATURE_CTRL_T),
		.pfHandler = UniCmdVowFeatureCtrl
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP,
		.u4StructSize = sizeof(UNI_CMD_VOW_BSSGROUP_CTRL_1_GROUP_T),
		.pfHandler = UniCmdVowBssGroupCtrl
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_BSSGROUP_TOKEN_CFG,
		.u4StructSize = sizeof(UNI_CMD_VOW_BSSGROUP_TOKEN_CFG_T),
		.pfHandler = UniCmdVowBssGroupTokenCtrl
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP,
		.u4StructSize = sizeof(UNI_CMD_VOW_BSSGROUP_CTRL_ALL_GROUP_T),
		.pfHandler = UniCmdVowBssGroupCtrlAllGroup
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM,
		.u4StructSize = sizeof(UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_T),
		.pfHandler = UniCmdVowBssGroupBwGroupQuantum
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL,
		.u4StructSize = sizeof(UNI_CMD_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL_T),
		.pfHandler = UniCmdVowBssGroupBwGroupQuantumALL
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_AT_PROC_EST_FEATURE,
		.u4StructSize = sizeof(UNI_CMD_VOW_AT_PROC_EST_FEATURE_T),
		.pfHandler = UniCmdVowATProcEstFeature
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD,
		.u4StructSize = sizeof(UNI_CMD_VOW_AT_PROC_EST_MONITOR_PERIOD_T),
		.pfHandler = UniCmdVowATProcEstMonitorPeriod
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO,
		.u4StructSize = sizeof(UNI_CMD_VOW_AT_PROC_EST_GROUP_RATIO_T),
		.pfHandler = UniCmdVowATProcEstGroupRatio
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING,
		.u4StructSize = sizeof(UNI_CMD_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING_T),
		.pfHandler = UniCmdVowATProcEstGroupToBandMapping
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_RX_AT_AIRTIME_EN,
		.u4StructSize = sizeof(UNI_CMD_VOW_RX_AT_AIRTIME_EN_T),
		.pfHandler = UniCmdVowRxATAirtimeEn
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_RX_AT_MIBTIME_EN,
		.u4StructSize = sizeof(UNI_CMD_VOW_RX_AT_MIBTIME_EN_T),
		.pfHandler = UniCmdVowRxATMibtimeEn
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_RX_AT_EARLYEND_EN,
		.u4StructSize = sizeof(UNI_CMD_VOW_RX_AT_EARLYEND_EN_T),
		.pfHandler = UniCmdVowRxATEarlyendEn
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN,
		.u4StructSize = sizeof(UNI_CMD_VOW_RX_AT_AIRTIME_CLR_EN_T),
		.pfHandler = UniCmdVowRxAtAirtimeClrEn
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_RX_AT_STA_WMM_CTRL,
		.u4StructSize = sizeof(UNI_CMD_VOW_RX_AT_STA_WMM_CTRL_T),
		.pfHandler = UniCmdVowRxAtStaWmmCtrl
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL,
		.u4StructSize = sizeof(UNI_CMD_VOW_RX_AT_MBSS_WMM_CTRL_T),
		.pfHandler = UniCmdVowRxAtMbssWmmCtrl
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_RX_AT_ED_OFFSET,
		.u4StructSize = sizeof(UNI_CMD_VOW_RX_AT_ED_OFFSET_T),
		.pfHandler = UniCmdVowRxAtEdOffset
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_RX_AT_SW_TIMER,
		.u4StructSize = sizeof(UNI_CMD_VOW_RX_AT_SW_TIMER_T),
		.pfHandler = UniCmdVowRxAtSwTimer
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_RX_AT_BACKOFF_TIMER,
		.u4StructSize = sizeof(UNI_CMD_VOW_RX_AT_BACKOFF_TIMER_T),
		.pfHandler = UniCmdVowRxAtBackoffTimer
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME,
		.u4StructSize = sizeof(UNI_CMD_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T),
		.pfHandler = UniCmdVowRxAtReportRxNonwifiTime
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME,
		.u4StructSize = sizeof(UNI_CMD_VOW_RX_AT_REPORT_RX_OBSS_TIME_T),
		.pfHandler = UniCmdVowRxAtReportRxObssTime
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME,
		.u4StructSize = sizeof(UNI_CMD_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T),
		.pfHandler = UniCmdVowRxAtReportMibObssTime
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME,
		.u4StructSize = sizeof(UNI_CMD_VOW_RX_AT_REPORT_PER_STA_RX_TIME_T),
		.pfHandler = UniCmdVowRxAtReportPerStaRxTime
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_RED_ENABLE,
		.u4StructSize = sizeof(UNI_CMD_VOW_RED_ENABLE_T),
		.pfHandler = UniCmdVowRedEnable
	},
	{
		.u2CmdFeature = UNI_CMD_VOW_RED_TX_RPT,
		.u4StructSize = sizeof(UNI_CMD_VOW_RED_TX_RPT_T),
		.pfHandler = UniCmdVowRedTxRpt
	},
};

static UINT32 UniCmdVowExtraAllocDynSizeCheck(RTMP_ADAPTER *pAd, P_UNI_CMD_VOW_PARAM_T pVOWParam)
{
	UINT32 i = 0;
	UINT32 BufSize = 0;

	if (pVOWParam->VOWTagValid[UNI_CMD_VOW_RED_TX_RPT])
		BufSize += pVOWParam->VowRedTxRpt.u2Length-sizeof(UNI_CMD_VOW_RED_TX_RPT_T);

	return BufSize;
}


INT32 UniCmdVOWUpdate(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_CMD_VOW_PARAM_T pVOWParam,
	BOOLEAN isSet,
	UINT8 McuDest,
	VOID *pResult)
{
	struct cmd_msg			*msg = NULL;
	INT32					Ret = NDIS_STATUS_SUCCESS;
	UINT8					i = 0;
	UINT16					u2TLVNumber = 0;
	PUCHAR					pTempBuf = NULL;
	PUCHAR					pNextHeadBuf = NULL;
	UINT32					u4CmdNeedMaxBufSize = 0;
	UINT32					u4RealUseBufSize = 0;
	UINT32					u4SendBufSize = 0;
	UINT32					u4RemainingPayloadSize = 0;
	UINT32					u4ComCmdSize = 0;
	UINT32					u4RespStructSize = 0;
	P_UNI_CMD_VOW_T			pUniCmdVow = NULL;
	RTMP_CHIP_CAP			*cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"System isn't ready for in-band command now!!!\n");
		Ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	/* Step 1: Count maximum buffer size from per TLV */
	u4ComCmdSize = sizeof(UNI_CMD_VOW_T);
	u4CmdNeedMaxBufSize += u4ComCmdSize;
	for (i = 0; i < UNI_CMD_VOW_MAX_NUM; i++) {
		if (pVOWParam->VOWTagValid[i])
			u4CmdNeedMaxBufSize += UniCmdVowTab[i].u4StructSize;
	}
	u4CmdNeedMaxBufSize += UniCmdVowExtraAllocDynSizeCheck(pAd, pVOWParam);

	/* Step 2: Allocate tempotary memory space for use later */
	os_alloc_mem(pAd, (UCHAR **)&pTempBuf, u4CmdNeedMaxBufSize);
	if (!pTempBuf) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	os_zero_mem(pTempBuf, u4CmdNeedMaxBufSize);
	pNextHeadBuf = pTempBuf;

	/* Step 3: Fill common parameters here */
	pUniCmdVow = (P_UNI_CMD_VOW_T)pNextHeadBuf;
	pNextHeadBuf += u4ComCmdSize;

	/* Step 4: Traverse all support features */
	for (i = 0; i < UNI_CMD_VOW_MAX_NUM; i++) {
		if (pVOWParam->VOWTagValid[i]) {
			Ret = ((PFN_VOW_HANDLE)(UniCmdVowTab[i].pfHandler))(pAd, pVOWParam, pNextHeadBuf, &u4RespStructSize);
			if (Ret == NDIS_STATUS_SUCCESS) {
				pNextHeadBuf += UniCmdVowTab[i].u4StructSize;
				u2TLVNumber++;
			} else {
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s: The hanlder of tag (0x%08x) return fail!\n", __func__, UniCmdVowTab[i].u2CmdFeature));
			}
		}
	}

	/* Step 5: Calculate real buffer size */
	u4RealUseBufSize = (pNextHeadBuf - pTempBuf);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Operation=%s, TLV Num = %d, CmdNeedMaxBufSize = %d, u4RealUseBufSize = %d, u4RespStructSize=%d\n",
		(isSet || u4RespStructSize == 0) ? "Set" : "Query",
		u2TLVNumber, u4CmdNeedMaxBufSize, u4RealUseBufSize, u4RespStructSize);

	/* Step 6: Send data packet and wrap fragement process if need */
	{
		UINT8 uSeqNum = AndesGetCmdMsgSeq(pAd);
		UINT8 uFragNum = 0;
		UINT8 uTotalFrag = 0;
		BOOLEAN bNeedFrag = FALSE;
		BOOLEAN bLastFrag = FALSE;

		if (u4RealUseBufSize > cap->u4MaxInBandCmdLen) {
			pNextHeadBuf = pTempBuf + u4ComCmdSize + 2; /* find first TLV length position */
			*pNextHeadBuf = (u4RealUseBufSize - u4ComCmdSize); /* fill in total length if need fragement */
#ifdef RT_BIG_ENDIAN
			*pNextHeadBuf = cpu2le16(*pNextHeadBuf);
#endif /* RT_BIG_ENDIAN */

			/* Calculate total fragment number */
			uTotalFrag = ((u4RealUseBufSize % cap->u4MaxInBandCmdLen) == 0) ?
						  (u4RealUseBufSize / cap->u4MaxInBandCmdLen) : ((u4RealUseBufSize / cap->u4MaxInBandCmdLen) + 1);
		}

		u4RemainingPayloadSize = u4RealUseBufSize;
		pNextHeadBuf = pTempBuf;
		do {
			struct _CMD_ATTRIBUTE	attr = {0};

			if (u4RemainingPayloadSize > cap->u4MaxInBandCmdLen) {
				bNeedFrag = TRUE;
				u4SendBufSize = cap->u4MaxInBandCmdLen;
				uFragNum++;
			} else {
				u4SendBufSize = u4RemainingPayloadSize;
				if (bNeedFrag) {
					uFragNum++;
					bLastFrag = TRUE;
				}
			}

			/* Allocate buffer */
			msg = AndesAllocUniCmdMsg(pAd, u4SendBufSize);
			if (!msg) {
				Ret = NDIS_STATUS_RESOURCES;
				goto error;
			}

			SET_CMD_ATTR_MCU_DEST(attr, McuDest);
			SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_VOW);
			SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
			SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pResult);
			if ((!bNeedFrag) || bLastFrag) {
				if (isSet) {
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_WAIT_RETRY_RSP);
					SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(UNI_EVENT_CMD_RESULT_T));
					SET_CMD_ATTR_RSP_HANDLER(attr, UniCmdResultRsp);
				} else if (u4RespStructSize > 0) {
					SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_WAIT_RETRY_RSP);
					SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(u4RespStructSize));
					SET_CMD_ATTR_RSP_HANDLER(attr, UniEventVowHandler);
				} else {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Query with u4RespStructSize is %d\n",
						u4RespStructSize);
					Ret = NDIS_STATUS_FAILURE;
					goto error;
				}
			} else {
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
			}
			AndesInitCmdMsg(msg, attr);

			/* Follow fragment rule if need */
			msg->total_frag = uTotalFrag;
			msg->frag_num = uFragNum;
			msg->seq = uSeqNum;

			/* Append this feature */
			AndesAppendCmdMsg(msg, (char *)pNextHeadBuf, u4SendBufSize);
			pNextHeadBuf += u4SendBufSize;

			/* Send out CMD */
			call_fw_cmd_notifieriers(WO_CMD_DEV_INFO, pAd, msg->net_pkt);
			Ret = chip_cmd_tx(pAd, msg);

			/* Process next remaining payload */
			u4RemainingPayloadSize -= u4SendBufSize;
		} while (u4RemainingPayloadSize > 0);
	}

error:
	if (pTempBuf)
		os_free_mem(pTempBuf);

	MTWF_DBG(NULL, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"%s:(Ret = %d)\n", __func__, Ret);
	return Ret;


}

#endif /* WIFI_UNIFIED_COMMAND */
