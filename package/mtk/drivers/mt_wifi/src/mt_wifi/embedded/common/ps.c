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
/****************************************************************************
 ***************************************************************************/

/****************************************************************************

    Abstract:

	All related POWER SAVE function body.

***************************************************************************/

#include "rt_config.h"


/*
	========================================================================
	Routine Description:
		This routine is used to do insert packet into power-saveing queue.

	Arguments:
		pAd: Pointer to our adapter
		pPacket: Pointer to send packet
		pMacEntry: portint to entry of MacTab. the pMacEntry store attribute of client (STA).
		QueIdx: Priority queue idex.

	Return Value:
		NDIS_STATUS_SUCCESS:If succes to queue the packet into TxSwQ.
		NDIS_STATUS_FAILURE: If failed to do en-queue.
========================================================================
*/
NDIS_STATUS RtmpInsertPsQueue(
	RTMP_ADAPTER * pAd,
	PNDIS_PACKET pPacket,
	MAC_TABLE_ENTRY *pMacEntry,
	UCHAR QueIdx)
{
	STA_TR_ENTRY *tr_entry;
	ULONG IrqFlags;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
#ifdef UAPSD_SUPPORT
	/* put the U-APSD packet to its U-APSD queue by AC ID */
	UINT32 ac_id = QueIdx - QID_AC_BK; /* should be >= 0 */

	tr_entry = &tr_ctl->tr_entry[pMacEntry->wcid];

	if (UAPSD_MR_IS_UAPSD_AC(pMacEntry, ac_id)) {
		UAPSD_PacketEnqueue(pAd, pMacEntry, pPacket, ac_id, FALSE);
#if defined(DOT11Z_TDLS_SUPPORT)
		TDLS_UAPSDP_TrafficIndSend(pAd, pMacEntry->Addr);
#endif /* defined(DOT11Z_TDLS_SUPPORT) */
#ifdef RT_CFG80211_SUPPORT
#ifdef CFG_TDLS_SUPPORT
		cfg_tdls_send_PeerTrafficIndication(pAd, pMacEntry->Addr);
#endif /* CFG_TDLS_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */
	} else
#endif /* UAPSD_SUPPORT */
	{
		if (tr_entry->ps_queue.Number >= MAX_PACKETS_IN_PS_QUEUE) {
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
		} else {
			MTWF_DBG(pAd, DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO, "legacy ps> queue a packet!\n");
			RTMP_IRQ_LOCK(&pAd->irq_lock /*&tr_entry->ps_queue_lock*/, IrqFlags);
			InsertTailQueue(&tr_entry->ps_queue, PACKET_TO_QUEUE_ENTRY(pPacket));
			RTMP_IRQ_UNLOCK(&pAd->irq_lock /*&tr_entry->ps_queue_lock*/, IrqFlags);
		}
	}

#ifdef CONFIG_AP_SUPPORT
	/* mark corresponding TIM bit in outgoing BEACON frame */
#ifdef UAPSD_SUPPORT

	if (UAPSD_MR_IS_NOT_TIM_BIT_NEEDED_HANDLED(pMacEntry, QueIdx)) {
		/* 1. the station is UAPSD station;
		2. one of AC is non-UAPSD (legacy) AC;
		3. the destinated AC of the packet is UAPSD AC. */
		/* So we can not set TIM bit due to one of AC is legacy AC */
	} else
#endif /* UAPSD_SUPPORT */
	{
		WLAN_MR_TIM_BIT_SET(pAd, pMacEntry->func_tb_idx, pMacEntry->Aid);
	}

#endif /* CONFIG_AP_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}


/*
	==========================================================================
	Description:
		This routine is used to clean up a specified power-saving queue. It's
		used whenever a wireless client is deleted.
	==========================================================================
 */
VOID RtmpCleanupPsQueue(RTMP_ADAPTER *pAd, QUEUE_HEADER *pQueue)
{
	QUEUE_ENTRY *pQEntry;
	PNDIS_PACKET pPacket;

	MTWF_DBG(pAd, DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RtmpCleanupPsQueue (0x%08lx)...\n", (ULONG)pQueue);

	while (pQueue->Head) {
		MTWF_DBG(pAd, DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "RtmpCleanupPsQueue %d...\n", pQueue->Number);
		pQEntry = RemoveHeadQueue(pQueue);
		/*pPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReservedEx); */
		pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		MTWF_DBG(pAd, DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RtmpCleanupPsQueue pkt = %lx...\n", (ULONG)pPacket);
	}
}


/*
  ========================================================================
  Description:
	This routine frees all packets in PSQ that's destined to a specific DA.
	BCAST/MCAST in DTIMCount=0 case is also handled here, just like a PS-POLL
	is received from a WSTA which has MAC address FF:FF:FF:FF:FF:FF
  ========================================================================
*/
VOID RtmpHandleRxPsPoll(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT wcid, BOOLEAN isActive)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->APPSMode == APPS_MODE2)
		return;
}


/*
 * ==========================================================================
 * Description:
 * Update the station current power save mode.
 * ==========================================================================
 */
BOOLEAN RtmpPsIndicate(RTMP_ADAPTER *pAd, UCHAR *pAddr, UINT16 wcid, UCHAR Psm)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->APPSMode == APPS_MODE2) {
		MAC_TABLE_ENTRY *entry;
		UCHAR old_psmode;
		struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
		STA_TR_ENTRY *tr_entry;

		if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
			return PWR_ACTIVE;

		entry = &pAd->MacTab.Content[wcid];
		tr_entry = &tr_ctl->tr_entry[wcid];

		old_psmode = entry->PsMode;
		if (old_psmode == PWR_ACTIVE && Psm == PWR_SAVE)
			NdisGetSystemUpTime(&entry->sleep_from);
		else if (old_psmode == PWR_SAVE && Psm == PWR_ACTIVE)
			entry->sleep_from = 0;

		entry->NoDataIdleCount = 0;
		entry->PsMode = Psm;
		tr_ctl->tr_entry[wcid].PsMode = Psm;
	}
	return PWR_ACTIVE;
}


#ifdef CONFIG_STA_SUPPORT
/*
========================================================================
Routine Description:
    Check if PM of any packet is set.

Arguments:
	pAd		Pointer to our adapter

Return Value:
    TRUE	can set
	FALSE	can not set

Note:
========================================================================
*/
BOOLEAN RtmpPktPmBitCheck(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg)
{
#ifdef DOT11Z_TDLS_SUPPORT

	/* check TDLS condition */
	if (pStaCfg->TdlsInfo.TdlsFlgIsKeepingActiveCountDown == TRUE)
		return FALSE;

#endif /* DOT11Z_TDLS_SUPPORT */

	return (pStaCfg->PwrMgmt.Psm == PWR_SAVE);
}


VOID RtmpPsActiveExtendCheck(RTMP_ADAPTER *pAd)
{
	/* count down the TDLS active counter */
#ifdef DOT11Z_TDLS_SUPPORT
	if (pAd->StaCfg[0].TdlsInfo.TdlsPowerSaveActiveCountDown > 0) {
		pAd->StaCfg[0].TdlsInfo.TdlsPowerSaveActiveCountDown--;

		if (pAd->StaCfg[0].TdlsInfo.TdlsPowerSaveActiveCountDown == 0) {
			/* recover our power save state */
			TDLS_RECOVER_POWER_SAVE(pAd);
			MTWF_DBG(pAd, DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TDLS PS> Recover PS mode!\n");
		}
	}

#endif /* DOT11Z_TDLS_SUPPORT */
}


VOID RtmpPsModeChange(RTMP_ADAPTER *pAd, UINT32 PsMode)
{
	MAC_TABLE_ENTRY *pEntry = GetAssociatedAPByWdev(pAd, &pAd->StaCfg[MAIN_MSTA_ID].wdev);

	if (pAd->StaCfg[0].BssType == BSS_INFRA) {
		/* reset ps mode */
		if (PsMode == Ndis802_11PowerModeMAX_PSP) {
			/* do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange() */
			/* to exclude certain situations. */
			/* MlmeSetPsm(pAd, PWR_SAVE); */
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

			if (pAd->StaCfg[0].bWindowsACCAMEnable == FALSE)
				pAd->StaCfg[0].WindowsPowerMode = Ndis802_11PowerModeMAX_PSP;

			pAd->StaCfg[0].WindowsBatteryPowerMode = Ndis802_11PowerModeMAX_PSP;
			pAd->StaCfg[0].DefaultListenCount = 5;
		} else if (PsMode == Ndis802_11PowerModeFast_PSP) {
			/* do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange() */
			/* to exclude certain situations. */
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

			if (pAd->StaCfg[0].bWindowsACCAMEnable == FALSE)
				pAd->StaCfg[0].WindowsPowerMode = Ndis802_11PowerModeFast_PSP;

			pAd->StaCfg[0].WindowsBatteryPowerMode = Ndis802_11PowerModeFast_PSP;
			pAd->StaCfg[0].DefaultListenCount = 3;
		} else if (PsMode == Ndis802_11PowerModeLegacy_PSP) {
			/* do NOT turn on PSM bit here, wait until MlmeCheckForPsmChange() */
			/* to exclude certain situations. */
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

			if (pAd->StaCfg[0].bWindowsACCAMEnable == FALSE)
				pAd->StaCfg[0].WindowsPowerMode = Ndis802_11PowerModeLegacy_PSP;

			pAd->StaCfg[0].WindowsBatteryPowerMode = Ndis802_11PowerModeLegacy_PSP;
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			pAd->StaCfg[0].DefaultListenCount = 1;
#else
			pAd->StaCfg[0].DefaultListenCount = 3;
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) // */
		} else {
			/* Default Ndis802_11PowerModeCAM */
			/* clear PSM bit immediately */
			RTMP_SET_PSM_BIT(pAd, &pAd->StaCfg[0], PWR_ACTIVE);
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);

			if (pAd->StaCfg[0].bWindowsACCAMEnable == FALSE)
				pAd->StaCfg[0].WindowsPowerMode = Ndis802_11PowerModeCAM;

			pAd->StaCfg[0].WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
		}

		/* change ps mode */
		RTMPSendNullFrame(pAd, pEntry, pAd->CommonCfg.TxRate, TRUE, FALSE);
		MTWF_DBG(pAd, DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PSMode=%ld\n", pAd->StaCfg[0].WindowsPowerMode);
	}
}

VOID EnqueuePsPoll(RTMP_ADAPTER *pAd, PSTA_ADMIN_CONFIG pStaCfg)
{
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */

	if (pStaCfg->WindowsPowerMode == Ndis802_11PowerModeLegacy_PSP)
		pAd->PsPollFrame.FC.PwrMgmt = PWR_SAVE;

	MiniportMMRequest(pAd, 0, (PUCHAR)&pAd->PsPollFrame, sizeof(PSPOLL_FRAME));
}

#endif /* CONFIG_STA_SUPPORT */

