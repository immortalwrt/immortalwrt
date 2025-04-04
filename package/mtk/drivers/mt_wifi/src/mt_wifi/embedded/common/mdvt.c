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
 ****************************************************************************

    Module Name:
	mdvt.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/


/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#ifdef WIFI_MODULE_DVT
#include "rt_config.h"

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#define SET_MDVT_INIT(pmdvt) \
		((pmdvt)->init = TRUE)

#define SET_MDVT_DEINIT(pmdvt) \
		((pmdvt)->init = FALSE)

#define SET_MDVT_ENABLE(pmdvt) \
		((pmdvt)->enable = TRUE)

#define SET_MDVT_DISABLE(pmdvt) \
		((pmdvt)->enable = FALSE)

#define GET_MDVT_ENABLE_STATE(pmdvt) \
		((pmdvt)->enable)

#define GET_MDVT_INIT_STATE(pmdvt) \
		((pmdvt)->init)

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
typedef struct _MDVT_BYPASS_CMD_T {
	UINT_8			  ucCmdID;
	UINT_8			  ucExtCmdID;
} MDVT_BYPASS_CMD_T, *P_MDVT_BYPASS_CMD_T;

typedef struct _MDVT_MODULE_UPDATE_T {
	ENUM_MDVT_MODULE_T	eModuleId;
	RTMP_STRING			*pucParserStr;
} MDVT_MODULE_UPDATE_T, *P_MDVT_MODULE_UPDATE_T;

typedef struct _MDVT_CTRL_T {
	BOOLEAN init;
	BOOLEAN enable;
} MDVT_CTRL_T, *P_MDVT_CTRL_T;

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

enum {
    /* debug commands */
	MDVT_FW_TEST = 1,
    /*END*/
};

const MDVT_BYPASS_CMD_T arMdvtBypassCmdTable[] = {
	{EXT_CID, EXT_CMD_ID_MDVT},
	/* Fw re-download by bin mode */
	{MT_TARGET_ADDRESS_LEN_REQ, 0},
	{MT_FW_START_REQ, 0},
	{CMD_ID_NIC_POWER_CTRL, 0},
	{MT_PATCH_START_REQ, 0},
	{MT_PATCH_FINISH_REQ, 0},
	{MT_PATCH_SEM_CONTROL, 0},
	{INIT_CMD_ID_CR4, 0},
	{MT_FW_SCATTER, 0},
};

size_t u4MdvtBypassCmdTableSize = sizeof(arMdvtBypassCmdTable) / sizeof(MDVT_BYPASS_CMD_T);

const MDVT_MODULE_UPDATE_T arMdvtModuleUpdateTable[] = {
	{MDVT_MODULE_WFARB,						"arb"},
	{MDVT_MODULE_AGG,						"agg"},
	{MDVT_MODULE_DMA,						"dma"},
	{MDVT_MODULE_WFMIMO,					"mimo"},
	{MDVT_MODULE_WFCTRL,					"ctrl"},
	{MDVT_MODULE_WFETXBF,					"txbf"},
	{MDVT_MODULE_WFCFG,						"cfg"},
	{MDVT_MODULE_WFHIF,						"hif"},
	{MDVT_MODULE_WFOFF,						"off"},
	{MDVT_MODULE_WFON,						"on"},
	{MDVT_MODULE_WFPF,						"pf"},
	{MDVT_MODULE_WFRMAC,					"rmac"},
	{MDVT_MODULE_WFUMAC_PLE,				"ple"},
	{MDVT_MODULE_WFUMAC_PSE,				"pse"},
	{MDVT_MODULE_WFUMAC_PP,					"pp"},
	{MDVT_MODULE_WFUMAC_AMSDU,				"amsdu"},
	{MDVT_MODULE_WFSEC,						"sec"},
	{MDVT_MODULE_WFTMAC,					"tmac"},
	{MDVT_MODULE_WFTMAC_TXPWR,				"txpwr"},
	{MDVT_MODULE_WFTXCR,					"txcr"},
	{MDVT_MODULE_WFMIB,						"mib"},
	{MDVT_MODULE_WFSYSON,					"syson"},
	{MDVT_MODULE_WFLPON,					"lpon"},
	{MDVT_MODULE_WFINT,						"int"},
	{MDVT_MODULE_CONNCFG,					"conncfg"},
	{MDVT_MODULE_MUCOP,						"mucop"},
	{MDVT_MODULE_WFMDP,						"mdp"},
	{MDVT_MODULE_WFRDM_PHYRX,				"phyrx"},
	{MDVT_MODULE_WFRDM_PHYDFS,				"phydfs"},
	{MDVT_MODULE_WFRDM_PHYRX_COMM,			"phyrxcomm"},
	{MDVT_MODULE_WFRDM_WTBLOFF,				"wtbloff"},
	{MDVT_MODULE_PHYDFE_CTRL_WF_TSSI,		"tssi"},
	{MDVT_MODULE_PHYDFE_RFINTF_WF_CMM,		"wfcmm"},
	{MDVT_MODULE_PHYRX_CTRL_WF_COMM_RDD,	"rdd"},
	{MDVT_MODULE_PHYRX_CTRL_WF_COMM_CSI,	"csi"},
	{MDVT_MODULE_PHYRX_CTRL_WF_COMM_CMM,	"cmm"},
	{MDVT_MODULE_PHYRX_CTRL_WF_COMM_TOAE,	"toae"},
	{MDVT_MODULE_PHYRX_CSD_WF_COMM_CMM,		"cmm"},
	{MDVT_MODULE_PHYRX_POST_CMM,			"postcmm"},
	{MDVT_MODULE_PHYDFS_WF_COMM_RDD,		"rdd"},
	{MDVT_MODULE_PHYRX_CTRL_TOAE,			"toae"},
	{MDVT_MODULE_PHYRX_CTRL_MURU,			"muru"},
	{MDVT_MODULE_PHYRX_CTRL_RDD,			"rdd"},
	{MDVT_MODULE_PHYRX_CTRL_MULQ,			"mulq"},
	{MDVT_MODULE_PHYRX_CTRL_CMM,			"ctrlcmm"},
	{MDVT_MODULE_PHYRX_CTRL_CSI,			"csi"},
	{MDVT_MODULE_PHYDFE_CTRL_PWR_REGU,		"requ"},
	{MDVT_MODULE_PHYRX_CTRL_BF,				"bf"},
	{MDVT_MODULE_PHYDFE_CTRL_CMM,			"dfectrlcmm"},
	{MDVT_MODULE_WFRBIST,					"rbist"},
	{MDVT_MODULE_WTBL,						"wtbl"},
	{MDVT_MODULE_RX,						"rx"},
	{MDVT_MODULE_LPON,						"lpon"},
	{MDVT_MODULE_MDP_RX,					"mdprx"},
	{MDVT_MODULE_TXCMD,						"txcmd"},
	{MDVT_MODULE_SEC_ECC,					"sececc"},
	{MDVT_MODULE_MIB,						"mib"},
	{MDVT_MODULE_WFTWT,						"twt"},
	{MDVT_MODULE_DRR,						"drr"},
	{MDVT_MODULE_RUOFDMA,					"ruofdma"},
	{MDVT_MODULE_WFCMDRPTTX,				"cmdrpttx"},
	{MDVT_MODULE_WFCMDRPT_TRIG,				"cmdrpttrig"},
	{MDVT_MODULE_MLO,						"mlo"},
	{MDVT_MODULE_TXD,						"txd"},
	{MDVT_MODULE_PH_TPUT,					"tput"},
	{MDVT_MODULE_MAX,						"all"}
};
size_t u4MdvtModuleUpdateTableSize = sizeof(arMdvtModuleUpdateTable) / sizeof(MDVT_MODULE_UPDATE_T);

/*******************************************************************************
*                              F U N C T I O N S
******************I**************************************************************
*/
static void mdvt_stop_tx_packet(struct _RTMP_ADAPTER *ad)
{
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = ad->ApCfg.BssidNum;
	BSS_STRUCT *pMbss = &ad->ApCfg.MBSSID[MAIN_MBSSID];
#endif
	BOOLEAN Cancelled;

	RTMP_OS_NETDEV_STOP_QUEUE(ad->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(ad) {
		if (MaxNumBss > MAX_MBSSID_NUM(ad))
			MaxNumBss = MAX_MBSSID_NUM(ad);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID;
			 IdBss < MAX_MBSSID_NUM(ad); IdBss++) {
			if (ad->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(ad->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}

	APStop(ad, pMbss, AP_BSS_OPER_ALL);
#endif

	RTMP_SET_FLAG(ad, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	RTMPCancelTimer(&ad->Mlme.PeriodicTimer, &Cancelled);
}

static void mdvt_start_tx_packet(struct _RTMP_ADAPTER *ad)
{
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = ad->ApCfg.BssidNum;
	BSS_STRUCT *pMbss = &ad->ApCfg.MBSSID[MAIN_MBSSID];
#endif

	RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	RTMP_OS_NETDEV_START_QUEUE(ad->net_dev);
	RTMPSetTimer(&ad->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);

#ifdef CONFIG_AP_SUPPORT
	APStartUp(ad, pMbss, AP_BSS_OPER_ALL);

	IF_DEV_CONFIG_OPMODE_ON_AP(ad) {
		if (MaxNumBss > MAX_MBSSID_NUM(ad))
			MaxNumBss = MAX_MBSSID_NUM(ad);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(ad); IdBss++) {
			if (ad->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(ad->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
}

static inline P_MDVT_CTRL_T mdvt_get_ctrl(struct _RTMP_ADAPTER *ad)
{
	if (!ad)
		return NULL;

	return ad->mdvt;

}
BOOLEAN mdvt_block_command(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg)
{
	int i;
	MDVT_CTRL_T *pmdvt = mdvt_get_ctrl(ad);

	if (pmdvt == NULL)
		return FALSE;

	/* Normal Case, Don't block in-band command */
	if (GET_MDVT_ENABLE_STATE(pmdvt) == FALSE)
		return FALSE;

	/* Check the bypass table */
	for (i = 0; i < u4MdvtBypassCmdTableSize; i++) {
	if ((msg->attr.ext_type == arMdvtBypassCmdTable[i].ucExtCmdID)
			&& (msg->attr.type == arMdvtBypassCmdTable[i].ucCmdID))
			return FALSE;
	}

	return TRUE;
}
void mdvt_enable(struct _RTMP_ADAPTER *ad)
{
	MDVT_CTRL_T *pmdvt = mdvt_get_ctrl(ad);

	if ((pmdvt != NULL)
		&& (GET_MDVT_INIT_STATE(pmdvt) == TRUE)
		&& (GET_MDVT_ENABLE_STATE(pmdvt) == FALSE)) {
		mdvt_stop_tx_packet(ad);
		SET_MDVT_ENABLE(pmdvt);
	}
}

void mdvt_disable(struct _RTMP_ADAPTER *ad)
{
	MDVT_CTRL_T *pmdvt = mdvt_get_ctrl(ad);

	if (pmdvt != NULL &&
	   (GET_MDVT_INIT_STATE(pmdvt) == TRUE) &&
	   (GET_MDVT_ENABLE_STATE(pmdvt) == TRUE)) {
		SET_MDVT_DISABLE(pmdvt);
		mdvt_start_tx_packet(ad);
	}
}

VOID mdvt_exit(struct _RTMP_ADAPTER *ad)
{
	MDVT_CTRL_T *pmdvt = mdvt_get_ctrl(ad);

	if (pmdvt == NULL)
		return;

	SET_MDVT_DEINIT(pmdvt);
	os_free_mem(pmdvt);
	ad->mdvt = NULL;
}

INT mdvt_init(struct _RTMP_ADAPTER *ad)
{
	P_MDVT_CTRL_T pmdvt;

	os_alloc_mem(ad, (UCHAR **)&ad->mdvt, sizeof(MDVT_CTRL_T));

	if (!ad->mdvt) {
		MTWF_DBG(ad, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		     "Allocate Fail\n");
		return FALSE;
	}

	pmdvt = ad->mdvt;
	os_zero_mem(pmdvt, sizeof(MDVT_CTRL_T));

	SET_MDVT_INIT(pmdvt);
	return 0;
}

INT SetMdvtModuleParameterProc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	RTMP_STRING *module_field = NULL;
	CMD_MDVT_TEST_T CmdMdvtModuleUpdate = {0};
	CHAR *set_str = NULL, *set_val = NULL;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT8 i;
	MDVT_CTRL_T *pmdvt = mdvt_get_ctrl(pAd);
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (!arg)
		goto show;

	set_str = strsep(&arg, "-");

	if (set_str == NULL)
		goto show;

	if (strcmp(set_str, "show") == 0)
		goto show;

	if (strcmp(set_str, "disable") == 0) {
		mdvt_disable(pAd);
		return TRUE;
	}

	for (i = 0 ; i < u4MdvtModuleUpdateTableSize; i++) {
		module_field = arMdvtModuleUpdateTable[i].pucParserStr;

		if (strcmp(set_str, module_field) == 0) {
			if ((pmdvt != NULL) &&
				GET_MDVT_ENABLE_STATE(pmdvt) == FALSE)
				mdvt_enable(pAd);

#ifdef WIFI_UNIFIED_COMMAND
			if (cap->uni_cmd_support) {
				set_val = strsep(&arg, "-");
				if (set_val) {
					UniCmdMDVT(pAd, arMdvtModuleUpdateTable[i].eModuleId, os_str_tol(set_val, 0, 10));

					MTWF_PRINT("%s:Module %s Module ID = %d Test Case Idx = %d\n",
						 __func__, module_field, arMdvtModuleUpdateTable[i].eModuleId, (int)os_str_tol(set_val, 0, 10));
				} else {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "set_val is NULL!\n");
				}
			} else
#endif /* WIFI_UNIFIED_COMMAND */
			{
				msg = AndesAllocCmdMsg(pAd, sizeof(CMD_MDVT_TEST_T));

				if (!msg)
					return FALSE;

				SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
				SET_CMD_ATTR_TYPE(attr, EXT_CID);
				SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MDVT);
				SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
				SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
				SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
				SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
				SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
				AndesInitCmdMsg(msg, attr);
				CmdMdvtModuleUpdate.ucTestMode = MDVT_FW_TEST;
				CmdMdvtModuleUpdate.u2TestModule = arMdvtModuleUpdateTable[i].eModuleId;

				set_val = strsep(&arg, "-");

				if (set_val)
					CmdMdvtModuleUpdate.u2TestCaseIdx = os_str_tol(set_val, 0, 10);

				/* Append this feature */
				AndesAppendCmdMsg(msg, (char *)&CmdMdvtModuleUpdate,
								  sizeof(CMD_MDVT_TEST_T));

				MTWF_PRINT("%s:Module %s Module ID = %d Test Case Idx = %d\n",
				 __func__, module_field, CmdMdvtModuleUpdate.u2TestModule,
				 CmdMdvtModuleUpdate.u2TestCaseIdx);

				AndesSendCmdMsg(pAd, msg);
			}

			break;
		}
	}

	if (i == u4MdvtModuleUpdateTableSize)
		goto show;

	return TRUE;

show:
	MTWF_PRINT("iwpriv ra0 set mdvt=(module)-(value)\n");

	for (i = 0 ; i < u4MdvtModuleUpdateTableSize; i++) {
		MTWF_PRINT("Module Name %s\n", arMdvtModuleUpdateTable[i].pucParserStr);
	}

	return TRUE;
}
#endif /* WIFI_MODULE_DVT */

