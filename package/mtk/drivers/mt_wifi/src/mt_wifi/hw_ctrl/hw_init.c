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
	hw_init.c
*/

#include "rt_config.h"

/*HW related init*/

INT32 WfHifHwInit(RTMP_ADAPTER *pAd, HIF_INFO_T *pHifInfo)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	ret = hif_sys_init(pAd->hdev_ctrl);
	return ret;
}

static INT32 WfTopHwInit(RTMP_ADAPTER *pAd)
{
	AsicTOPInit(pAd);
	return NDIS_STATUS_SUCCESS;
}

static INT32 WfMcuHwInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

#ifdef WIFI_RAM_EMI_SUPPORT
	if (ops->parse_emi_phy_addr)
		ops->parse_emi_phy_addr(pAd);

	if (ops->fw_ram_emi_dl) {
		ret = ops->fw_ram_emi_dl(pAd);
		if (ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "emi ram download failed, Status[=0x%08x]\n", ret);
			return NDIS_STATUS_FAILURE;
		}
	}
#endif /* WIFI_RAM_EMI_SUPPORT */

	if (ops->prepare_fwdl_img)
		ops->prepare_fwdl_img(pAd);

	/* rxv cap init */
	if (ops->rxv_cap_init)
		ops->rxv_cap_init(pAd);

	ret = NICLoadRomPatch(pAd);

	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "NICLoadRomPatch failed, Status[=0x%08x]\n", ret);
		return NDIS_STATUS_FAILURE;
	}

	{
		UINT32 Value = 0;
		UINT32 Addr = 0;

#ifdef MT7915_MT7916_COEXIST_COMPATIBLE
		if (IS_MT7915(pAd)) {
			Addr = WF_SW_DEF_CR_ICAP_SPECTRUM_MODE_ADDR_MT7915;
		} else if (IS_MT7916(pAd)) {
			Addr = WF_SW_DEF_CR_ICAP_SPECTRUM_MODE_ADDR_MT7916;
		} else {
			MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid chip id:0x%04x\n", pAd->ChipID);
			return NDIS_STATUS_FAILURE;
		}
#else
		Addr = WF_SW_DEF_CR_ICAP_SPECTRUM_MODE_ADDR;
#endif
#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
		/* Refer to profile setting to decide the sysram partition format */
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "\x1b[42m Before NICLoadFirmware, check ICapMode = %d \x1b[m\n", pAd->ICapMode);

		if (pAd->ICapMode == 1) {/* ICap */
			/* We need to set SW_DEF_CR before FW is downloaded in order */
			/* to determine UMAC/MCU sysram statement during FW init.    */
			HW_IO_READ32(pAd->hdev_ctrl, Addr, &Value);
			Value = Value & (~WF_SW_DEF_CR_FWOPMODE);
			Value = Value | (pAd->ICapMode << WF_SW_DEF_CR_FWOPMODE_SHFT);
			HW_IO_WRITE32(pAd->hdev_ctrl, Addr, Value);
		} else if (pAd->ICapMode == 2) { /* Wifi-spectrum */
			/* We need to set SW_DEF_CR before FW is downloaded in order */
			/* to determine UMAC/MCU sysram statement during FW init.    */
			if (IS_MT7615(pAd)) {
				HW_IO_READ32(pAd->hdev_ctrl, CONFG_COM1_REG3, &Value);
				Value = Value | CONFG_COM1_REG3_FWOPMODE;
				HW_IO_WRITE32(pAd->hdev_ctrl, CONFG_COM1_REG3, Value);
			} else if (IS_MT7622(pAd)) {
				HW_IO_READ32(pAd->hdev_ctrl, CONFG_COM2_REG3, &Value);
				Value = Value | CONFG_COM2_REG3_FWOPMODE;
				HW_IO_WRITE32(pAd->hdev_ctrl, CONFG_COM2_REG3, Value);
			} else {
				HW_IO_READ32(pAd->hdev_ctrl, Addr, &Value);
				Value = Value & (~WF_SW_DEF_CR_FWOPMODE);
				Value = Value | (pAd->ICapMode << WF_SW_DEF_CR_FWOPMODE_SHFT);
				HW_IO_WRITE32(pAd->hdev_ctrl, Addr, Value);
			}
		} else
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */
		{
			/* We need to set SW_DEF_CR before FW is downloaded in order */
			/* to determine UMAC/MCU sysram statement during FW init.    */
			if (IS_MT7615(pAd)) {/* Normal */
				HW_IO_READ32(pAd->hdev_ctrl, CONFG_COM1_REG3, &Value);
				Value = Value & (~CONFG_COM1_REG3_FWOPMODE);
				HW_IO_WRITE32(pAd->hdev_ctrl, CONFG_COM1_REG3, Value);
			} else if (IS_MT7622(pAd)) {
				HW_IO_READ32(pAd->hdev_ctrl, CONFG_COM2_REG3, &Value);
				Value = Value & (~CONFG_COM2_REG3_FWOPMODE);
				HW_IO_WRITE32(pAd->hdev_ctrl, CONFG_COM2_REG3, Value);
			} else {
				HW_IO_READ32(pAd->hdev_ctrl, Addr, &Value);
				Value = Value & (~WF_SW_DEF_CR_FWOPMODE);
				HW_IO_WRITE32(pAd->hdev_ctrl, Addr, Value);
			}
		}
	}
	ret = NICLoadFirmware(pAd);

	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "NICLoadFirmware failed, Status[=0x%08x]\n", ret);
		return NDIS_STATUS_FAILURE;
	}

	/*After fw download should disalbe dma schedule bypass mode*/
#ifdef DMA_SCH_SUPPORT
	if (chip_wait_hif_dma_idle(pAd, 100, 1000) != TRUE) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
			ret =  NDIS_STATUS_FAILURE;
			return ret;
		}
	}

	AsicDMASchedulerInit(pAd, DMA_SCH_LMAC);
#endif

	if (cap->MCUType & CR4) {
		asic_wa_update(pAd);
	}
	return ret;
}


static INT32 WfEPROMHwInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	NICInitAsicFromEEPROM(pAd);
	return ret;
}


/*Common Part for externl*/

INT32 WfTopInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (WfTopHwInit(pAd) != NDIS_STATUS_SUCCESS)
		ret = NDIS_STATUS_FAILURE;

	return ret;
}

INT32 WfHifInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	HIF_INFO_T hifInfo;

	os_zero_mem(&hifInfo, sizeof(HIF_INFO_T));
	ret = WfHifSysInit(pAd, &hifInfo);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err;

	WfHifHwInit(pAd, &hifInfo);
	WLAN_HOOK_CALL(WLAN_HOOK_HIF_INIT, pAd, NULL);
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--(), Success!\n");
	return 0;
err:
	WfHifSysExit(pAd);
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--(), Err! status=%d\n", ret);
	return ret;
}

INT32 WfMcuInit(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	INT32 ret = NDIS_STATUS_SUCCESS;
	ret = WfMcuSysInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err;

	ret = WfMcuHwInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err;

	HWCtrlOpsReg(pAd);

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD) && cap->txd_flow_ctl)
		MtCmdCr4Set(pAd, WA_SET_OPTION_TXD_FLOW_CTRL, 1, 0);

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--(), Success!\n");
	return ret;
err:
	WfMcuSysExit(pAd);
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--(), Err! status=%d\n", ret);
	return ret;
}

static INT mac_init(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");
	AsicInitMac(pAd);

	/* re-set specific MAC registers for individual chip */
	if (ops->AsicMacInit != NULL)
		ops->AsicMacInit(pAd);

	/* auto-fall back settings */
	AsicAutoFallbackInit(pAd);
	AsicSetMacMaxLen(pAd);
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--\n");
	return TRUE;
}

INT32 WfMacInit(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;

	if (chip_wait_hif_dma_idle(pAd, ALL_DMA, 100, 1000) != TRUE) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
			ret =  NDIS_STATUS_FAILURE;
			return ret;
		}
	}
#ifdef DMA_SCH_SUPPORT
	AsicDMASchedulerInit(pAd, DMA_SCH_LMAC);
#endif
	mac_init(pAd);
	asic_init_wtbl(pAd, TRUE);
#ifdef HDR_TRANS_RX_SUPPORT
	AsicRxHeaderTransCtl(pAd, TRUE, FALSE, FALSE, TRUE, FALSE);
	AsicRxHeaderTaranBLCtl(pAd, 0, TRUE, ETH_TYPE_EAPOL);
	AsicRxHeaderTaranBLCtl(pAd, 1, TRUE, ETH_TYPE_WAI);
	AsicRxHeaderTaranBLCtl(pAd, 2, TRUE, ETH_TYPE_FASTROAMING);
#endif

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
		AsicAutoBATrigger(pAd, TRUE, BA_TRIGGER_OFFLOAD_TIMEOUT);
	return ret;
}

INT32 WfEPROMInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	ret = WfEPROMSysInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err;

	WfEPROMHwInit(pAd);
	return ret;
err:
	WfEPROMSysExit(pAd);
	return ret;
}

INT32 WfPhyInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	NICInitBBP(pAd);
	return ret;
}

INT32 WfInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	ret = WfTopInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err0;

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Top Init Done!\n");
	ret = WfHifInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err0;

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Hif Init Done!\n");
	ret = WfMcuInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err1;

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MCU Init Done!\n");
#ifdef RLM_CAL_CACHE_SUPPORT
	rlmCalCacheApply(pAd, pAd->rlmCalCache);
#endif /* RLM_CAL_CACHE_SUPPORT */

	/* re-update chip cap after mcu init */
	chip_update_chip_cap(pAd);

	/*Adjust eeprom + config => apply to HW*/
	ret = WfEPROMInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err2;

#ifdef SINGLE_SKU_V2
    /* Load SKU table to Host Driver */
    RTMPSetSkuParam(pAd);
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
    /* Load BF Backoff table to Host Driver */
    RTMPSetBackOffParam(pAd);
#endif /* defined(MT_MAC) && defined(TXBF_SUPPORT) */
#endif /* SINGLE_SKU_V2 */
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "EEPROM Init Done!\n");
	ret = WfMacInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err3;

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MAC Init Done!\n");
	ret = WfPhyInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err3;

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PHY Init Done!\n");
	return ret;
err3:
	WfEPROMSysExit(pAd);
err2:
	WfMcuSysExit(pAd);
err1:
	WfHifSysExit(pAd);
err0:
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " initial faild!! ret=%d\n", ret);
	return ret;
}


/*SW related init*/

INT32 WfSysPreInit(RTMP_ADAPTER *pAd)
{


#ifdef MT7986

	if (IS_MT7986(pAd))
		mt7986_init(pAd);

#endif

#ifdef MT7916

	if (IS_MT7916(pAd))
		mt7916_init(pAd);

#endif

#ifdef MT7981

	if (IS_MT7981(pAd))
		mt7981_init(pAd);

#endif

	wifi_sup_list_register(pAd, WIFI_CAP_CHIP);
	wifi_sup_list_register(pAd, WIFI_CAP_SEC);
	wifi_sup_list_register(pAd, WIFI_CAP_FEATURE);

	return 0;
}


INT32 WfSysPosExit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	WfEPROMSysExit(pAd);
	WfMcuSysExit(pAd);
	WfHifSysExit(pAd);
	return ret;
}

INT32 WfSysCfgInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	return ret;
}


INT32 WfSysCfgExit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	return ret;
}
