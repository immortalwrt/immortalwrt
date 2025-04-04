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
	rtmp_mcu.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include	"rt_config.h"

INT32 MCUSysPrepare(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	MCU_CTRL_INIT(pAd);
	chip_fw_init(pAd);

	return Ret;
}

INT32 MCUSysInit(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
	MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	MCU_CTRL_INIT(pAd);
	chip_fw_init(pAd);
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
			return -1;
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
	Ret = NICLoadFirmware(pAd);

	if (Ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "NICLoadFirmware failed, Status[=0x%08x]\n", Ret);
		return -1;
	}

	return Ret;
}


INT32 MCUSysExit(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->FwExit)
		ops->FwExit(pAd);

	MCU_CTRL_EXIT(pAd);
	return Ret;
}


VOID ChipOpsMCUHook(PRTMP_ADAPTER pAd, enum MCU_TYPE MCUType)
{
	RTMP_CHIP_OP *pChipOps = hc_get_chip_ops(pAd->hdev_ctrl);

#ifdef CONFIG_ANDES_SUPPORT

	if ((MCUType & ANDES) == ANDES) {
		pChipOps->FwInit = hif_mcu_fw_init;
		pChipOps->FwExit = hif_mcu_fw_exit;
		pChipOps->kick_out_cmd_msg = hif_kick_out_cmd_msg;
		pChipOps->MtCmdTx = AndesSendCmdMsg;
		pChipOps->MCUCtrlInit = AndesCtrlInit;
		pChipOps->MCUCtrlExit = AndesCtrlExit;
		FwdlHookInit(pAd);
	}

#endif /* CONFIG_ANDES_SUPPORT */
}

