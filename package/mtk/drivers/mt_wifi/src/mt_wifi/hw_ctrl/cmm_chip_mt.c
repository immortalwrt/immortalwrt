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
	cmm_asic.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/


#include "rt_config.h"

#define BSSID_WCID_TO_REMOVE 1 /* Pat:TODO */

#ifdef CONFIG_AP_SUPPORT
VOID APCheckBcnQHandler(RTMP_ADAPTER *pAd, INT apidx, BOOLEAN *is_pretbtt_int)
{
	/* no implementation after mt7615 */
	return;
}
#endif /* CONFIG_AP_SUPPORT */

BOOLEAN MTPollTxRxEmpty(RTMP_ADAPTER *pAd, UINT8 pcie_port_or_all)
{
	return hif_poll_txrx_empty(pAd->hdev_ctrl, pcie_port_or_all);
}

VOID MTHifPolling(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx)
{
#ifdef RTMP_MAC_PCI
	UINT32 Loop, RxPending = 0;
	PNDIS_PACKET pRxPacket = NULL;
	RX_BLK RxBlk, *pRxBlk;
	BOOLEAN bReschedule = FALSE;
	EVENT_EXT_CMD_RESULT_T	rResult = {0};

	for (Loop = 0; Loop < 10; Loop++) {
		while (1) {
			pRxBlk = &RxBlk;
			/* pRxPacket = asic_get_pkt_from_rx_resource(pAd, &bReschedule, &RxPending, 0); */

			if ((RxPending == 0) && (bReschedule == FALSE))
				break;
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);

			msleep(20);
		}
	}

	for (Loop = 0; Loop < 10; Loop++) {
		AsicExtWifiHifCtrl(pAd, ucDbdcIdx, HIF_CTRL_ID_HIF_USB_TX_RX_IDLE, &rResult);

		if (rResult.u4Status == 0)
			break;

		while (1) {
			pRxBlk = &RxBlk;
			/* pRxPacket = asic_get_pkt_from_rx_resource(pAd, &bReschedule, &RxPending, 0); */

			if ((RxPending == 0) && (bReschedule == FALSE))
				break;
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
		}

		if (Loop == 1) {
			/* Above scenario should pass at 1st time or assert */
			MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Failed to poll RX path empry.\n");
			ASSERT(0);
		}
	}

#endif /* RTMP_MAC_PCI */
}

VOID MTRadioOn(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
#ifdef CONFIG_FWOWN_SUPPORT
	/* Driver Own */
	if (IS_MT7622(pAd) || IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7981(pAd)) {
		if (IsHcAllSupportedBandsRadioOff(pAd)) {
			DriverOwn(pAd);
		}
	}
#endif /* CONFIG_FWOWN_SUPPORT */

	/*  Send radio on command and wait for ack */
	if (IS_MT7603(pAd))
		MtCmdRadioOnOffCtrl(pAd, WIFI_RADIO_ON);
	else
		RTMP_RADIO_ON_OFF_CTRL(pAd, HcGetBandByChannel(pAd, wdev->channel), WIFI_RADIO_ON);

	/* Send Led on command */

	/* Enable RX */
	if (IS_MT7603(pAd))
		AsicSetMacTxRx(pAd, ASIC_MAC_RX, TRUE);

	HcSetRadioCurStatByChannel(pAd, wdev->channel, PHY_INUSE);
}

VOID MTRadioOff(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	BOOLEAN tx_rx_empty = FALSE;
	BOOLEAN all_bands_radio_off = FALSE;

	/* Disable RX */
	if (IS_MT7603(pAd))
		AsicSetMacTxRx(pAd, ASIC_MAC_RX, FALSE);

	/* Polling TX/RX path until packets empty */
	tx_rx_empty = MTPollTxRxEmpty(pAd, HcGetBandByChannel(pAd, wdev->channel));

	MTWF_DBG(pAd, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"(wdev): ch(%d), band(%d), tx_rx_empty(%d)\n",
		wdev->channel,
		HcGetBandByChannel(pAd, wdev->channel),
		tx_rx_empty);

	/* Set Radio off flag */
	HcSetRadioCurStatByChannel(pAd, wdev->channel, PHY_RADIOOFF);

	/* Send radio off command and wait for ack */
	if (IS_MT7603(pAd))
		MtCmdRadioOnOffCtrl(pAd, WIFI_RADIO_OFF);
	else
		RTMP_RADIO_ON_OFF_CTRL(pAd, HcGetBandByChannel(pAd, wdev->channel), WIFI_RADIO_OFF);

#ifdef CONFIG_FWOWN_SUPPORT
	/* Fw own */
	if (IS_MT7622(pAd) || IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7981(pAd)) {
		all_bands_radio_off = IsHcAllSupportedBandsRadioOff(pAd);
		if (all_bands_radio_off & tx_rx_empty) {
			FwOwn(pAd);
		} else {
			MTWF_DBG(pAd, DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"all_bands_radio_off(%d), tx_rx_empty(%d)\n",
				all_bands_radio_off,
				tx_rx_empty);
		}
	}
#endif /* CONFIG_FWOWN_SUPPORT */
}

#ifdef RTMP_MAC_PCI
VOID MTMlmeLpExit(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif
#ifdef CONFIG_FWOWN_SUPPORT
	DriverOwn(pAd);
#endif /* CONFIG_FWOWN_SUPPORT */
#ifdef RTMP_MAC_PCI
	/*  Enable PDMA */
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(%d)::PDMA\n", __LINE__);
	chip_set_hif_dma(pAd, DMA_TX_RX, 1);
	/* Make sure get clear FW own interrupt */
	RtmpOsMsDelay(100);
#endif /* RTMP_MAC_PCI */
#ifdef CONFIG_FWOWN_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(%d)::bDrvOwn(%d)\n", __LINE__, pAd->bDrvOwn);
#endif /* CONFIG_FWOWN_SUPPORT */
	MCU_CTRL_INIT(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	/*  Send radio on command and wait for ack */
	RTMP_RADIO_ON_OFF_CTRL(pAd, DBDC_BAND_NUM, WIFI_RADIO_ON);
	/* Send Led on command */
	/* Enable RX */
	/* Offlaod below task to AsicExtPmStateCtrl() */
	/* AsicSetMacTxRx(pAd, ASIC_MAC_RX, TRUE); */
	HcSetAllSupportedBandsRadioOn(pAd);
	/*  Resume sending TX packet */
	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
}

VOID MTMlmeLpEnter(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
	BSS_STRUCT *pMbss = NULL;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif
	/*  Stop send TX packets */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#ifdef CONFIG_STA_SUPPORT
	/* Clear PMKID cache.*/
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		pStaCfg->SavedPMKNum = 0;
		RTMPZeroMemory(pStaCfg->SavedPMK, (PMKID_NO * sizeof(BSSID_INFO)));

	/* Link down first if any association exists*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
			if (INFRA_ON(pStaCfg) || ADHOC_ON(pAd)) {
				cntl_disconnect_request(wdev, CNTL_DISASSOC, pStaCfg->Bssid, REASON_DISASSOC_STA_LEAVING);
				RtmpusecDelay(1000);
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

		/* Link down first if any association exists*/
		if (INFRA_ON(pStaCfg) || ADHOC_ON(pAd))
			LinkDown(pAd, FALSE, wdev, NULL);

		RtmpusecDelay(10000);
		/*==========================================*/
		/* Clean up old bss table*/
#ifndef ANDROID_SUPPORT
		/* because abdroid will get scan table when interface down, so we not clean scan table */
		BssTableInit(ScanTab);
#endif /* ANDROID_SUPPORT */
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/* first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			pMbss = &pAd->ApCfg.MBSSID[IdBss];
			if (pMbss->wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(pMbss->wdev.if_dev);
		}
		APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
	}
#endif /* CONFIG_AP_SUPPORT */
	/*  Disable RX */
	/* Below function is offloaded to AsicExtPmStateCtrl() */
	/* AsicSetMacTxRx(pAd, ASIC_MAC_RX, FALSE); */
	/* Set Radio off flag*/
	HcSetAllSupportedBandsRadioOff(pAd);
	/* Delay for CR access */
	msleep(1000);
	/*  Send Led off command */
	/*  Send radio off command and wait for ack */
	RTMP_RADIO_ON_OFF_CTRL(pAd, DBDC_BAND_NUM, WIFI_RADIO_OFF);
	/*  Polling TX/RX path until packets empty */
	MTHifPolling(pAd, HcGetBandByWdev(wdev));
#ifdef RTMP_MAC_PCI
	/*  Disable PDMA */
	chip_set_hif_dma(pAd, DMA_TX_RX, 0);
#endif /* RTMP_MAC_PCI */
#ifdef CONFIG_FWOWN_SUPPORT
	FwOwn(pAd);
#endif /* CONFIG_FWOWN_SUPPORT */
}

#endif /* RTMP_MAC_PCI */

#ifdef CONFIG_AP_SUPPORT
/*
 * NOTE:
 * this function is for MT7628/MT7603/MT7636 only.
 *
 * MT7636 has no MBSS function.
 * but below to MT MBSS gen1 mac address assignment rule
 */
VOID MtAsicSetMbssWdevIfAddrGen1(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT opmode)
{
	UINT32 Value = 0;
	UCHAR MacByte = 0, MacMask = 0;
	INT idx = wdev->func_idx;
	UCHAR *if_addr = (UCHAR *)wdev->if_addr;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s\n", __FILE__);

	if (opmode == OPMODE_AP) {
		COPY_MAC_ADDR(if_addr, pAd->CurrentAddress);
		/* read BTEIR bit[31:29] for determine to choose which byte to extend BSSID mac address.*/
		MAC_IO_READ32(pAd->hdev_ctrl, LPON_BTEIR, &Value);
		/* Note: Carter, make default will use byte4 bit[31:28] to extend Mac Address */
		Value = Value | (0x2 << 29);
		MAC_IO_WRITE32(pAd->hdev_ctrl, LPON_BTEIR, Value);
		MacByte = Value >> 29;
		MAC_IO_READ32(pAd->hdev_ctrl, RMAC_RMACDR, &Value);
		Value = Value & ~RMACDR_MBSSID_MASK;

		if (pAd->ApCfg.BssidNum <= 2) {
			Value |= RMACDR_MBSSID(0x0);
			MacMask = 0xef;
		} else if (pAd->ApCfg.BssidNum <= 4) {
			Value |= RMACDR_MBSSID(0x1);
			MacMask = 0xcf;
		} else if (pAd->ApCfg.BssidNum <= 8) {
			Value |= RMACDR_MBSSID(0x2);
			MacMask = 0x8f;
		} else if (pAd->ApCfg.BssidNum <= 16) {
			Value |= RMACDR_MBSSID(0x3);
			MacMask = 0x0f;
		} else {
			Value |= RMACDR_MBSSID(0x3);
			MacMask = 0x0f;
		}

		MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_RMACDR, Value);

		if (idx > 0) {
			/* MT7603, bit1 in byte0 shall always be b'1 for Multiple BSSID */
			if_addr[0] |= 0x2;

			switch (MacByte) {
			case 0x1: /* choose bit[23:20]*/
				if_addr[2] = if_addr[2] & MacMask;/* clear high 4 bits, */
				if_addr[2] = (if_addr[2] | (idx << 4));
				break;

			case 0x2: /* choose bit[31:28]*/
				if_addr[3] = if_addr[3] & MacMask;/* clear high 4 bits, */
				if_addr[3] = (if_addr[3] | (idx << 4));
				break;

			case 0x3: /* choose bit[39:36]*/
				if_addr[4] = if_addr[4] & MacMask;/* clear high 4 bits, */
				if_addr[4] = (if_addr[4] | (idx << 4));
				break;

			case 0x4: /* choose bit [47:44]*/
				if_addr[5] = if_addr[5] & MacMask;/* clear high 4 bits, */
				if_addr[5] = (if_addr[5] | (idx << 4));
				break;

			default: /* choose bit[15:12]*/
				if_addr[1] = if_addr[1] & MacMask;/* clear high 4 bits, */
				if_addr[1] = (if_addr[1] | (idx << 4));
				break;
			}
		}
	}
}

/*
 * NOTE: 2015-April-2.
 * this function is for MT7637/MT7615 and afterward chips
 */
VOID MtAsicSetMbssWdevIfAddrGen2(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR zeroMac[6] = {0};
	UCHAR MacMask = 0;
	INT idx = wdev->func_idx;
	UCHAR *if_addr = (UCHAR *)wdev->if_addr;

	if (pAd->ApCfg.BssidNum <= 2)
		MacMask = 0xef;
	else if (pAd->ApCfg.BssidNum <= 4)
		MacMask = 0xcf;
	else if (pAd->ApCfg.BssidNum <= 8)
		MacMask = 0x8f;
	else if (pAd->ApCfg.BssidNum <= 16)
		MacMask = 0x0f;
	else
		MacMask = 0x0f;
	if (idx > 0) {
		if (NdisEqualMemory(zeroMac, pAd->ExtendMBssAddr[idx - 1], MAC_ADDR_LEN)) {
			COPY_MAC_ADDR(if_addr, pAd->CurrentAddress);
			if_addr[0] |= 0x2;
			/* default choose bit[31:28], if there is no assigned mac from profile. */
			if_addr[3] = if_addr[3] & MacMask;/* clear high 4 bits, */
			if_addr[3] = (if_addr[3] | ((idx % 16) << 4));

			/* reverse bit[24] if BssidNum >= 16 */
			if (idx >= 16)
				if_addr[3] ^= 0x1;

			COPY_MAC_ADDR(pAd->ExtendMBssAddr[idx - 1], if_addr);
		} else
			COPY_MAC_ADDR(if_addr, pAd->ExtendMBssAddr[idx - 1]);
	} else
		COPY_MAC_ADDR(if_addr, pAd->CurrentAddress);

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s mbss_idx = %d, if_addr = %x %x %x %x %x %x\n",
			  __FILE__, idx,
			  if_addr[0], if_addr[1], if_addr[2],
			  if_addr[3], if_addr[4], if_addr[5]);
}

#ifdef DOT11V_MBSSID_SUPPORT
/*
 * this function is for MT7915 and afterward chips
 * new DBDC arch. and (per-band) 11v address assignment
 */
VOID MtAsicSetMbssWdevIfAddrGen3(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR zeroMac[6] = {0};
	UCHAR *if_addr = (UCHAR *)wdev->if_addr;
	UINT8 ucDbdcIdx = HcGetBandByWdev(wdev);
	BSS_STRUCT *pMbss = wdev->func_dev;
	INT mbss_idx = pMbss->mbss_idx;
	INT mbss_grp_idx;
	INT mbss_trans_bss_idx = pAd->ApCfg.dot11v_trans_bss_idx[ucDbdcIdx];
	UCHAR MacMask = BITS(0, (pAd->ApCfg.dot11v_max_bssid_indicator[ucDbdcIdx] - 1));

	mbss_fill_per_band_idx(pAd, pMbss);
	mbss_grp_idx = pMbss->mbss_grp_idx;

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"mbss_idx(%d), band(%d), grp_idx(%d), trans idx(%d)\n",
			mbss_idx, ucDbdcIdx, mbss_grp_idx, mbss_trans_bss_idx);

	/* 1st BSS of band */
	if (mbss_grp_idx == 0) {
		if (mbss_idx == 0)		/* band 0*/
			COPY_MAC_ADDR(if_addr, pAd->CurrentAddress);
		else {
			if (NdisEqualMemory(zeroMac, pAd->ExtendMBssAddr[mbss_trans_bss_idx - 1], MAC_ADDR_LEN)) {
				/* band 1 (or afterware) mac addr not assigned */
				COPY_MAC_ADDR(if_addr, pAd->CurrentAddress);
				if_addr[0] |= (0x1 + ucDbdcIdx);
				/* write back to table */
				COPY_MAC_ADDR(pAd->ExtendMBssAddr[mbss_idx - 1], if_addr);
			} else {
				/* mac addr assigned */
				COPY_MAC_ADDR(if_addr, pAd->ExtendMBssAddr[mbss_trans_bss_idx - 1]);
			}
		}
	} else {
		if (NdisEqualMemory(zeroMac, pAd->ExtendMBssAddr[mbss_idx - 1], MAC_ADDR_LEN)) {
			/* mac addr not assigned */
			if (mbss_trans_bss_idx > 0)
				COPY_MAC_ADDR(if_addr, pAd->ExtendMBssAddr[mbss_trans_bss_idx - 1]);
			else
				COPY_MAC_ADDR(if_addr, pAd->CurrentAddress);

			/* clear n LSB bits */
			if_addr[5] &= ~MacMask;
			/* assign n LSB bits to [REF_BSSID + idx] mod 2^n */
			if_addr[5] |= ((pAd->CurrentAddress[5] + mbss_grp_idx) & MacMask);
			/* write back to table */
			COPY_MAC_ADDR(pAd->ExtendMBssAddr[mbss_idx - 1], if_addr);
		} else {
			/* mac addr assigned */
			COPY_MAC_ADDR(if_addr, pAd->ExtendMBssAddr[mbss_idx - 1]);
		}
	}

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tif_addr = "MACSTR"\n", MAC2STR(if_addr));
}
#endif /* DOT11V_MBSSID_SUPPORT */
#endif /*CONFIG_AP_SUPPORT*/

#ifdef CONFIG_APSTA_MIXED_SUPPORT
VOID MtAsicSetApcliWdevIfAddr(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR zeroMac[6] = {0};
	INT idx = wdev->func_idx;
	UCHAR *if_addr = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);
	if_addr = (UCHAR *)wdev->if_addr;

#ifdef MT_MAC
	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
#endif /* MT_MAC */

		if (cap->MBSSIDMode >= MBSSID_MODE1) {
			if ((pAd->ApCfg.BssidNum > 0) || (MAX_MESH_NUM > 0)) {
				UCHAR MacMask = 0;

				if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 2)
					MacMask = 0xFE;
				else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 4)
					MacMask = 0xFC;
				else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 8)
					MacMask = 0xF8;


				/*
					Refer to HW definition -
						Bit1 of MAC address Byte0 is local administration bit
						and should be set to 1 in extended multiple BSSIDs'
						Bit3~ of MAC address Byte0 is extended multiple BSSID index.
				*/
				if (cap->MBSSIDMode == MBSSID_MODE1) {
					/*
						Refer to HW definition -
							Bit1 of MAC address Byte0 is local administration bit
							and should be set to 1 in extended multiple BSSIDs'
							Bit3~ of MAC address Byte0 is extended multiple BSSID index.
					*/
#ifdef ENHANCE_NEW_MBSSID_MODE
		a			wdev->if_addr[0] &= (MacMask << 2);
#endif /* ENHANCE_NEW_MBSSID_MODE */
					wdev->if_addr[0] |= 0x2;
					wdev->if_addr[0] += (((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1) << 2);
				}

#ifdef ENHANCE_NEW_MBSSID_MODE
				else {
					wdev->if_addr[0] |= 0x2;
					wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] &= (MacMask);
					wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] += ((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1);
				}

#endif /* ENHANCE_NEW_MBSSID_MODE */
			}
		} else
			wdev->if_addr[MAC_ADDR_LEN - 1] = (wdev->if_addr[MAC_ADDR_LEN - 1] + pAd->ApCfg.BssidNum + MAX_MESH_NUM) & 0xFF;

#ifdef MT_MAC
	} else {
		UCHAR MacByte = 0;
		UINT32 Value = 0;


		HW_IO_READ32(pAd->hdev_ctrl, LPON_BTEIR, &Value);
		MacByte = Value >> 29;

		if (NdisEqualMemory(zeroMac, pAd->ApcliAddr[idx], MAC_ADDR_LEN)) {
			/*
			 * mac addr not assigned
			 * Flip bit[idx+2] for apcli(idx)
			 * Ex, BSS0 if_addr[0] = 0x4
			 *     -> apcli(0)=0x2, bit[1]=1 | bit[2]=0x1 -> 0x0
			 *     -> apcli(1)=0xe, bit[1]=1 | bit[3]=0x0 -> 0x1
			 */
			if_addr[0] |= 0x2;
			if_addr[0] ^= (0x1 << (2 + idx));

			switch (MacByte) {
			case 0x2: /* choose bit[31:28]*/
				wdev->if_addr[3] = wdev->if_addr[3] & 0x0f;
				break;
			}
			/* multiple inf down-up will change address because of "if_addr[0] ^= (0x1 << (2 + idx))" code */
			/* In case of DBDC mode (AX1800) it result into both apcli inf will have same MAC addr*/
			/* Store the first time derived address to ApcliAddr so that for next apcli down up same is used */
			COPY_MAC_ADDR(pAd->ApcliAddr[idx], if_addr);
		} else {
			/* mac addr assigned */
			COPY_MAC_ADDR(if_addr, pAd->ApcliAddr[idx]);
		}
	}
#endif /* MT_MAC */

	MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s apcli_idx = %d, if_addr = %x %x %x %x %x %x (MBSSIDMode:%d)\n",
			  __FILE__, idx,
			  PRINT_MAC(if_addr),
			  cap->MBSSIDMode);
}
#endif

VOID MtAsicSetWdevIfAddr(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT opmode)
{
	if (opmode == OPMODE_AP) {
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11V_MBSSID_SUPPORT
		UINT8 DbdcIdx;
		BOOLEAN bAddrGen3 = FALSE;

		/* use gen3(11v) mac addr assignment if enabled at any band, to prevent conflict */
		for (DbdcIdx = 0; DbdcIdx < DBDC_BAND_NUM; DbdcIdx++) {
			if (IS_BSSID_11V_ENABLED(pAd, DbdcIdx)) {
				bAddrGen3 = TRUE;
				break;
			}
		}

		if (bAddrGen3)
			MtAsicSetMbssWdevIfAddrGen3(pAd, wdev);
		else
#endif /* DOT11V_MBSSID_SUPPORT */
			MtAsicSetMbssWdevIfAddrGen2(pAd, wdev);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_APSTA_MIXED_SUPPORT
	} else if (opmode == OPMODE_STA) {
		MtAsicSetApcliWdevIfAddr(pAd, wdev);
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wdev(%d) non-supported opmode(%d)\n", wdev->wdev_idx, opmode);
	}
}

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
VOID mt_asic_pcie_aspm_dym_ctrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN fgL1Enable, BOOLEAN fgL0sEnable)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (!get_pcie_aspm_dym_ctrl_cap(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "retun since cap=0\n");
		return;
	}

	if (ops->pcie_aspm_dym_ctrl)
		ops->pcie_aspm_dym_ctrl(pAd, ucDbdcIdx, fgL1Enable, fgL0sEnable);
	else
		MTWF_DBG(pAd, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "For this chip, no specified dynamic aspm ctrl function!\n");
}
#endif /* #ifdef PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
VOID mt_asic_twt_agrt_update(struct wifi_dev *wdev, struct twt_agrt_para twt_agrt_para)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (!wlan_config_get_asic_twt_caps(wdev)) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "retun, twt h/w cap=0\n");
		return;
	}

	if (ops->twt_agrt_update)
		ops->twt_agrt_update(ad, twt_agrt_para);
	else
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "twt_agrt_update=NULL\n");
}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
