/*=============================================================================
//             INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//      Copyright (c) 2009-2010 Intel Corporation. All Rights Reserved.
//
//  File Name: l2sd_ta_mod.c
//  Description:
//===========================================================================*/

#ifdef WIDI_SUPPORT
#include "rt_config.h"

void WidiWscNotify(
	IN ULONG pAdapter,
	IN ULONG pWidiMsg,
	IN ULONG MsgSize,
	IN ULONG MsgType)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdapter;
	PUCHAR buff = (PUCHAR)pWidiMsg;
	INT len = (INT)MsgSize;
	PNET_DEV dev = pAd->net_dev;

	if (MsgType == WIDI_MSG_TYPE_QUERY_OR_TRIGGER) {
		PWIDI_QUERY_OR_TRIGGER_MSG pMsg = (PWIDI_QUERY_OR_TRIGGER_MSG)buff;

		/* need shift type(2) + len(2) = 4 bytes */
		if (!IS_INTEL_SMI((&pMsg->vendorExt[4])))
			return;
	}

	RtmpOSWidiNotify(dev, buff, len, MsgType);
}

VOID WidiUpdateStateToDaemon(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR BssIdx,
	IN UCHAR WidiType,
	IN PUCHAR pPeerMac,
	IN PUCHAR pSsid,
	IN UCHAR  SsidLen,
	IN UCHAR WidiAssocStat)
{
	PWIDI_ASSOC_MSG pWidiMsg = NULL;

	/* From ra0 */
	if ((BssIdx == MIN_NET_DEVICE_FOR_MBSSID) &&
		(!pAd->StaCfg[0].bWIDI || pAd->StaCfg[0].WscControl.bWscTrigger))
		return;

#ifdef P2P_SUPPORT
	/* From p2p0 */
	else if ((BssIdx >= MIN_NET_DEVICE_FOR_P2P_CLI) && (!pAd->P2pCfg.bWIDI))
		return;

#endif /* P2P_SUPPORT */
	os_alloc_mem(NULL, (UCHAR **)&pWidiMsg, sizeof(WIDI_ASSOC_MSG));

	if (pWidiMsg) {
		RTMPZeroMemory(pWidiMsg, sizeof(WIDI_ASSOC_MSG));
		pWidiMsg->type = WidiType;
		NdisMoveMemory(pWidiMsg->peer_mac, pPeerMac, MAC_ADDR_LEN);
		pWidiMsg->assoc_stat = WidiAssocStat;

		if (pSsid) {
			NdisMoveMemory(pWidiMsg->ssid, pSsid, SsidLen);
			pWidiMsg->ssid_len = SsidLen;
		}

		WidiWscNotify((ULONG)pAd, (ULONG)pWidiMsg, sizeof(WIDI_ASSOC_MSG), pWidiMsg->type);
		os_free_mem(pWidiMsg);
	}
}
VOID WidiNotifyVendorExtToDaemon(
	IN PRTMP_ADAPTER pAd,
	IN PWIDI_VENDOR_EXT pWiDiVendorExtList,
	IN PUCHAR pSrcMac,
	IN UCHAR channel,
	IN PCHAR Ssid,
	IN UCHAR SsidLen)
{
	WIDI_QUERY_OR_TRIGGER_MSG widiMsg;
	RTMPZeroMemory(&widiMsg, sizeof(WIDI_QUERY_OR_TRIGGER_MSG));

	if (pWiDiVendorExtList != NULL) {
		WIDI_VENDOR_EXT *pCurrVendorExt = pWiDiVendorExtList;
		NdisMoveMemory(widiMsg.src_mac, pSrcMac, MAC_ADDR_LEN);
		widiMsg.channel = channel;
		NdisMoveMemory(widiMsg.ssid, Ssid, SsidLen);
		widiMsg.ssid_len = SsidLen;
		widiMsg.type = WIDI_MSG_TYPE_QUERY_OR_TRIGGER;

		while (pCurrVendorExt) {
			/* Notify WiDI Vendor Ext to Host */
			if (pAd->StaCfg[0].bWIDI) {
				NdisMoveMemory(&widiMsg.vendorExt[0], &pCurrVendorExt->VendorExt[0], WIDI_QUERY_TRIGGER_VE_LEN);
				WidiWscNotify((ULONG)pAd, (ULONG)&widiMsg, sizeof(WIDI_QUERY_OR_TRIGGER_MSG), widiMsg.type);
			}

			/* Free WiDi Vendor Ext */
			pWiDiVendorExtList = pCurrVendorExt;
			pCurrVendorExt = pCurrVendorExt->pNext;
			pWiDiVendorExtList->pNext = NULL;
			os_free_mem(pWiDiVendorExtList);
		}

		pWiDiVendorExtList = NULL;
	}
}

#ifdef P2P_SUPPORT
VOID WidiNotifyP2pIPToDaemon(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pWpsIe,
	IN UCHAR WpsIeLen,
	IN PUCHAR pSrcMac,
	IN UCHAR channel,
	IN PCHAR Ssid,
	IN UCHAR SsidLen)
{
	PWIDI_P2P_IP pWidiP2pIP = NULL;
	BOOLEAN bFound = FALSE;
	UCHAR *tmp_data;
	UCHAR tmp_len = 0;
	USHORT attrib_type = 0;
	USHORT attrib_len = 0;
	/*
		Parse the WPS IE and find out whether there is Vendor Extension
		Attribute with Intel SMI
	*/
	tmp_len = WpsIeLen;
	tmp_data = pWpsIe;

	while (tmp_len > 4) {
		WSC_IE	WscIE;
		NdisMoveMemory(&WscIE, tmp_data, sizeof(WSC_IE));
		attrib_type = be2cpu16(WscIE.Type);
		attrib_len = be2cpu16(WscIE.Length);

		if (attrib_type == WPS_VENDOR_EXT_CODE) {
			if (IS_INTEL_SMI(tmp_data + 4)) {
				bFound = TRUE;
				break;
			}
		}

		tmp_data += (attrib_len + 4);
		tmp_len -= (attrib_len + 4);
	}

	if (!bFound)
		return;

	if (attrib_len == 0 || (attrib_len + 4) > WIDI_P2P_IP_VE_LEN) {
		MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Attrib Len is either 0 or too big: len is %d\n", attrib_len));
		return;
	}

	os_alloc_mem(NULL, (UCHAR **)&pWidiP2pIP, sizeof(WIDI_P2P_IP));

	if (pWidiP2pIP) {
		NdisZeroMemory(pWidiP2pIP, sizeof(WIDI_P2P_IP));
		NdisMoveMemory(pWidiP2pIP->src_mac, pSrcMac, MAC_ADDR_LEN);
		pWidiP2pIP->channel = channel;
		NdisMoveMemory(pWidiP2pIP->ssid, Ssid, SsidLen);
		pWidiP2pIP->ssid_len = SsidLen;
		NdisMoveMemory(pWidiP2pIP->qa_ta_ext, tmp_data, attrib_len + 4);
		pWidiP2pIP->type = WIDI_MSG_TYPE_P2P_IP;
		WidiWscNotify((ULONG)pAd, (ULONG)pWidiP2pIP, sizeof(WIDI_P2P_IP), pWidiP2pIP->type);
		os_free_mem(pWidiP2pIP);
	}
}

/**
 * This notification is used by the driver to tell application
 * layer that it has received a valid WIDI speicific probe
 * request.
 *
 * @param pAd
 * @param pWpsIe
 * @param WpsIeLen
 * @param Channel
 * @param ssid
 * @param ssid_len
 *
 * @return BOOLEAN
 */
BOOLEAN  WidiNotifyP2pProbeIeToDaemon(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pWpsIe,
	IN UCHAR WpsIeLen,
	IN PUCHAR pSrcMac,
	IN UINT8 channel,
	IN UCHAR *Ssid,
	IN USHORT SsidLen)
{
	PWIDI_P2P_PROBE_WPS_IE_MSG pWidiP2pProbeWpsIe = NULL;
	BOOLEAN bFound = FALSE;
	UCHAR *tmp_data;
	UCHAR tmp_len = 0;
	USHORT attrib_type = 0;
	USHORT attrib_len = 0;
	USHORT category_id = 0;
	/*
		Parse the WPS IE and find out whether there is WPS_VENDOR_EXT_WIDI_SRC_CODE
		Attribute with Intel OUI
	*/
	tmp_len = WpsIeLen;
	tmp_data = pWpsIe;

	while (tmp_len > 4) {
		WSC_IE	WscIE;
		NdisMoveMemory(&WscIE, tmp_data, sizeof(WSC_IE));
		attrib_type = be2cpu16(WscIE.Type);
		attrib_len = be2cpu16(WscIE.Length);

		if (attrib_type == WPS_VENDOR_EXT_WIDI_SRC_CODE) {
			NdisMoveMemory(&category_id, tmp_data + 4, 2);
			category_id = be2cpu16(category_id);

			if (IS_INTEL_OUI(tmp_data + 6)) {
				bFound = TRUE;
				break;
			}
		}

		tmp_data += (attrib_len + 4);
		tmp_len -= (attrib_len + 4);
	}

	if (!bFound)
		return FALSE;

	os_alloc_mem(NULL, (UCHAR **)&pWidiP2pProbeWpsIe, sizeof(WIDI_P2P_PROBE_WPS_IE_MSG));

	if (pWidiP2pProbeWpsIe) {
		NdisZeroMemory(pWidiP2pProbeWpsIe, sizeof(WIDI_P2P_PROBE_WPS_IE_MSG));
		NdisMoveMemory(pWidiP2pProbeWpsIe->src_mac, pSrcMac, MAC_ADDR_LEN);
		pWidiP2pProbeWpsIe->channel = channel;
		NdisMoveMemory(pWidiP2pProbeWpsIe->ssid, Ssid, SsidLen);
		pWidiP2pProbeWpsIe->ssid_len = SsidLen;
		pWidiP2pProbeWpsIe->category_id = category_id;
		pWidiP2pProbeWpsIe->type = WIDI_MSG_TYPE_P2P_PROBE;
		WidiWscNotify((ULONG)pAd, (ULONG)pWidiP2pProbeWpsIe, sizeof(WIDI_P2P_PROBE_WPS_IE_MSG), pWidiP2pProbeWpsIe->type);
		os_free_mem(pWidiP2pProbeWpsIe);
	} else
		return FALSE;

	return TRUE;
}

#endif /* P2P_SUPPORT */

#endif /* WIDI_SUPPORT */

