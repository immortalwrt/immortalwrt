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
	wsc.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Paul Lin	06-08-08		Initial
	Snowpin Lee 06-09-12        Do modifications and Add APIs for AP
	Snowpin Lee 07-04-19        Do modifications and Add APIs for STA
	Snowpin Lee 07-05-17        Do modifications and Add APIs for AP Client
*/

#include    "rt_config.h"

#ifdef WSC_INCLUDED
#include    "wsc_tlv.h"
/*#ifdef LINUX */
/*#include <net/iw_handler.h> */
/*#endif*/

#define WSC_UPNP_MSG_TIMEOUT            (150 * OS_HZ)
#define RTMP_WSC_NLMSG_SIGNATURE_LEN    8
#define MAX_WEPKEYNAME_LEN              20
#define MAX_WEPKEYTYPE_LEN              20

#ifndef PF_NOFREEZE
#define PF_NOFREEZE  0
#endif

char WSC_MSG_SIGNATURE[] = {"RAWSCMSG"};

UINT8 WPS_DH_G_VALUE[1] = {0x02};
UINT8 WPS_DH_P_VALUE[192] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
	0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
	0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
	0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
	0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
	0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
	0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
	0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
	0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
	0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
	0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
	0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
	0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
	0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
	0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
	0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A,
	0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
	0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96,
	0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
	0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
	0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
	0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x23, 0x73, 0x27,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

/* General used field */
UCHAR	STA_Wsc_Pri_Dev_Type[8] = {0x00, 0x01, 0x00, 0x50, 0xf2, 0x04, 0x00, 0x01};

#ifdef CON_WPS
UUID_BSSID_CH_INFO      TmpInfo_2G;
UUID_BSSID_CH_INFO      TmpInfo_5G;
#endif /*CON_WPS Dung_Ru*/

#ifdef CONFIG_AP_SUPPORT
UCHAR	AP_Wsc_Pri_Dev_Type[8] = {0x00, 0x06, 0x00, 0x50, 0xf2, 0x04, 0x00, 0x01};

#ifdef APCLI_SUPPORT

VOID WscApCliLinkDown(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl);
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

static BOOLEAN WscCheckNonce(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MLME_QUEUE_ELEM * Elem,
	IN  BOOLEAN bFlag,
	IN  PWSC_CTRL pWscControl);

#ifdef CONFIG_STA_SUPPORT
static VOID WscEapActionDisabled(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PWSC_CTRL pWscControl);
#endif /* CONFIG_STA_SUPPORT */

static VOID WscGetConfigErrFromNack(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MLME_QUEUE_ELEM * pElem,
	OUT USHORT * pConfigError);

static INT WscSetAuthMode(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR CurOpMode,
	IN  UCHAR apidx,
	IN  RTMP_STRING * arg);

static INT WscSetEncrypType(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR CurOpMode,
	IN  UCHAR apidx,
	IN  RTMP_STRING * arg);

static VOID WscSendNACK(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MAC_TABLE_ENTRY * pEntry,
	IN  PWSC_CTRL pWscControl);

static INT wsc_write_dat_file_thread(IN ULONG data);

#ifdef CONFIG_STA_SUPPORT
static VOID WscLinkDown(
	IN  PRTMP_ADAPTER pAd,
	IN  PVOID wdev_obj);
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
static UINT32 WscGetAuthMode(
	IN  USHORT authFlag);

static UINT32 WscGetWepStatus(
	IN  USHORT encryFlag);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CON_WPS
VOID ConWpsApCliMonitorTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	PRTMP_ADAPTER pActionAd;
#ifdef MULTI_INF_SUPPORT
	PRTMP_ADAPTER pOpposAd;
	UINT actionBandIdx = 0;
	UINT nowBandIdx = multi_inf_get_idx(pAd);
	UINT opsBandIdx = !nowBandIdx;
#endif /* MULTI_INF_SUPPORT */
	UCHAR index = BSS0;

#ifdef MULTI_INF_SUPPORT
	pOpposAd = (PRTMP_ADAPTER)adapt_list[opsBandIdx];
#endif /* MULTI_INF_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Interface %s\n", pAd->net_dev->name);
#ifdef MULTI_INF_SUPPORT
	if (pOpposAd) {
		if (pOpposAd->ApCfg.ConWpsApCliStatus == TRUE) {
			/* 2G/5G Band WPS Ready */
			if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_2G)
				actionBandIdx = 0;
			else
				actionBandIdx = 1;
		} else
			actionBandIdx = nowBandIdx;

		WscStop(pAd, TRUE, &pAd->StaCfg[BSS0].wdev.WscControl);
	} else
#endif /* MULTI_INF_SUPPORT */
	{
#ifdef MULTI_INF_SUPPORT
		actionBandIdx = nowBandIdx;
#endif

		/* Cancel the other band if onGoing */
		if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_2G)
			WscStop(pAd, TRUE, &pAd->StaCfg[BSS1].wdev.WscControl);
		else if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_5G)
			WscStop(pAd, TRUE, &pAd->StaCfg[BSS0].wdev.WscControl);
	}

#ifdef MULTI_INF_SUPPORT
	if (actionBandIdx < MAX_NUM_OF_INF)
		pActionAd = (PRTMP_ADAPTER)adapt_list[actionBandIdx];
	else
		pActionAd = pAd;
#else
	pActionAd = pAd;
#endif /* MULTI_INF_SUPPORT */
	{
		for (index = 0; index < MAX_APCLI_NUM; index++)
			RTEnqueueInternalCmd(pActionAd, CMDTHREAD_APCLI_IF_DOWN, (VOID *)&index, sizeof(UCHAR));
	}
	pActionAd->StaCfg[BSS0].ApcliInfStat.Enable = TRUE;

	for (index = 0; index < MAX_APCLI_NUM; index++)
		pAd->StaCfg[index].wdev.WscControl.conWscStatus = CON_WPS_STATUS_DISABLED;

#ifdef MULTI_INF_SUPPORT
	if (pOpposAd)
		pOpposAd->ApCfg.ConWpsApCliStatus = FALSE;
#endif

	pAd->ApCfg.ConWpsApCliStatus = FALSE;
}

DECLARE_TIMER_FUNCTION(ConWpsApCliMonitorTimeout);
BUILD_TIMER_FUNCTION(ConWpsApCliMonitorTimeout);
#endif /* CON_WPS */


/*
*	Standard UUID generation procedure. The UUID format generated by this function is base on UUID std. version 1.
*	It's a 16 bytes, one-time global unique number. and can show in string format like this:
*			550e8400-e29b-41d4-a716-446655440000
*	The format of uuid is:
*		uuid                        = <time_low> "-"
*					      <time_mid> "-"
*					      <time_high_and_version> "-"
*					      <clock_seq_high_and_reserved>
*		                          <clock_seq_low> "-"
*					      <node>
*		time_low                    = 4*<hex_octet>
*		time_mid                    = 2*<hex_octet>
*		time_high_and_version       = 2*<hex_octet>
*		clock_seq_high_and_reserved = <hex_octet>
*		clock_seq_low               = <hex_octet>
*		node                        = 6*<hex_octet>
*		hex_octet                   = <hex_digit> <hex_digit>
*		hex_digit                   = "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
*					      |"a"|"b"|"c"|"d"|"e"|"f"
*					      |"A"|"B"|"C"|"D"|"E"|"F"
*	Note:
*		Actually, to IOT with JumpStart, we fix the first 10 bytes of UUID string!!!!
*/
INT WscGenerateUUID(
	RTMP_ADAPTER * pAd,
	UCHAR *uuidHexStr,
	UCHAR *uuidAscStr,
	int apIdx,
	BOOLEAN	bUseCurrentTime,
	BOOLEAN from_apcli)
{
	WSC_UUID_T uuid_t;
	unsigned long long uuid_time;
	int i, ret;
	UINT16 clkSeq;
	char uuidTmpStr[UUID_LEN_STR + 2];
#ifdef MULTI_INF_SUPPORT
#ifdef CON_WPS_AP_SAME_UUID
	PRTMP_ADAPTER pOpposAd;
	PWSC_CTRL pWscControl;

	/* We Assume the 5G init phase after 2.4Ghz */
	if (from_apcli == FALSE) {
		for (i = 0; i < MAX_NUM_OF_INF; i++) {
			if ((pAd != adapt_list[i]) && (adapt_list[i] != NULL)) {
				pOpposAd = (PRTMP_ADAPTER) adapt_list[i];
				if (pOpposAd->ApCfg.MBSSID[apIdx].wdev.if_up_down_state) {
					pWscControl = &pOpposAd->ApCfg.MBSSID[apIdx].wdev.WscControl;
					NdisCopyMemory(uuidHexStr, &pWscControl->Wsc_Uuid_E[0], UUID_LEN_HEX);
					NdisCopyMemory(uuidAscStr, &pWscControl->Wsc_Uuid_Str[0], UUID_LEN_STR);
					goto show;
				}
			}
		}
	}


#endif /* CON_WPS_AP_SAME_UUID */
#endif /* MULTI_INF_SUPPORT */
#ifdef RTMP_RBUS_SUPPORT
	/* for fixed UUID -  YYHuang 07/10/09 */
#define FIXED_UUID
#endif /* RTMP_RBUS_SUPPORT */

	/* Get the current time. */
	if (bUseCurrentTime) {
		ULONG Now;

		NdisGetSystemUpTime(&Now);
		uuid_time = Now;
	} else
		uuid_time = 2860; /*xtime.tv_sec;	// Well, we fix this to make JumpStart  happy! */

	uuid_time *= 10000000;
	uuid_time += 0x01b21dd213814000LL;
#ifdef RTMP_RBUS_SUPPORT
#ifdef FIXED_UUID

	if (IS_RBUS_INF(pAd))
		uuid_time  = 0x2880288028802880LL;

#endif
#endif /* RTMP_RBUS_SUPPORT */
	uuid_t.timeLow = (UINT32)uuid_time & 0xFFFFFFFF;
	uuid_t.timeMid = (UINT16)((uuid_time >> 32) & 0xFFFF);
	uuid_t.timeHi_Version = (UINT16)((uuid_time >> 48) & 0x0FFF);
	uuid_t.timeHi_Version |= (1 << 12);
	NdisZeroMemory(uuid_t.node, 6);
	/* Get the clock sequence. */
	clkSeq = (UINT16)(0x0601/*jiffies*/ & 0xFFFF);		/* Again, we fix this to make JumpStart happy! */
#ifdef RTMP_RBUS_SUPPORT
#ifdef FIXED_UUID

	if (IS_RBUS_INF(pAd))
		clkSeq = (UINT16)0x2880;

#endif
#endif /* RTMP_RBUS_SUPPORT */
	uuid_t.clockSeqLow = clkSeq & 0xFF;
	uuid_t.clockSeqHi_Var = (clkSeq & 0x3F00) >> 8;
	uuid_t.clockSeqHi_Var |= 0x80;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		/* copy the Mac address as the value of node */
#ifdef APCLI_SUPPORT
		if (from_apcli)
			NdisMoveMemory(&uuid_t.node[0], &pAd->StaCfg[apIdx].wdev.if_addr[0], sizeof(uuid_t.node));
		else
#endif /* APCLI_SUPPORT */
		{
			/*
			 * Added code to generate UUID from AP Current Address, as UUID is unique
			 * per device. Also,Wdev field if_addr gets updated through wifi_sys_open ()
			 * which gets invoked during Virtual Interface Up
			 */
			NdisMoveMemory(&uuid_t.node[0], &pAd->CurrentAddress[0], sizeof(uuid_t.node));
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#ifdef P2P_SUPPORT

		/* copy the Mac address as the value of node */
		if (apIdx >= MIN_NET_DEVICE_FOR_P2P_GO)
			NdisMoveMemory(&uuid_t.node[0], &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.bssid[0], sizeof(uuid_t.node));
		else
#endif /* P2P_SUPPORT */
			NdisMoveMemory(&uuid_t.node[0], &pAd->StaCfg[0].wdev.if_addr[0], sizeof(uuid_t.node));
	}
#endif /* CONFIG_STA_SUPPORT */
	/* Create the UUID ASCII string. */
	memset(uuidTmpStr, 0, sizeof(uuidTmpStr));
	ret = snprintf(uuidTmpStr, sizeof(uuidTmpStr), "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			 (unsigned int)uuid_t.timeLow, uuid_t.timeMid, uuid_t.timeHi_Version, uuid_t.clockSeqHi_Var, uuid_t.clockSeqLow,
			 uuid_t.node[0], uuid_t.node[1], uuid_t.node[2], uuid_t.node[3], uuid_t.node[4], uuid_t.node[5]);
	if (os_snprintf_error(sizeof(uuidTmpStr), ret))
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "uuidTmpStr snprintf error!\n");

	if (strlen(uuidTmpStr) > UUID_LEN_STR)
		MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR:UUID String size too large!\n");

	strncpy((RTMP_STRING *)uuidAscStr, uuidTmpStr, UUID_LEN_STR);
	/* Create the UUID Hex format number */
	uuid_t.timeLow = cpu2be32(uuid_t.timeLow);
	NdisMoveMemory(&uuidHexStr[0], &uuid_t.timeLow, 4);
	uuid_t.timeMid = cpu2be16(uuid_t.timeMid);
	NdisMoveMemory(&uuidHexStr[4], &uuid_t.timeMid, 2);
	uuid_t.timeHi_Version = cpu2be16(uuid_t.timeHi_Version);
	NdisMoveMemory(&uuidHexStr[6], &uuid_t.timeHi_Version, 2);
	NdisMoveMemory(&uuidHexStr[8], &uuid_t.clockSeqHi_Var, 1);
	NdisMoveMemory(&uuidHexStr[9], &uuid_t.clockSeqLow, 1);
	NdisMoveMemory(&uuidHexStr[10], &uuid_t.node[0], 6);
#ifdef MULTI_INF_SUPPORT
#ifdef CON_WPS_AP_SAME_UUID
show:
#endif /* CON_WPS_AP_SAME_UUID */
#endif /* MULTI_INF_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "The UUID Hex string is:");

	for (i = 0; i < 16; i++)
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%02x", (uuidHexStr[i] & 0xff));

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "\n");
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "The UUID ASCII string is:%s!\n", uuidAscStr);
	return 0;
}

VOID	WscInitCommonTimers(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PWSC_CTRL pWscControl)
{
	WSC_TIMER_INIT(pAdapter, pWscControl, &pWscControl->EapolTimer, pWscControl->EapolTimerRunning, WscEAPOLTimeOutAction);
	WSC_TIMER_INIT(pAdapter, pWscControl, &pWscControl->Wsc2MinsTimer, pWscControl->Wsc2MinsTimerRunning, Wsc2MinsTimeOutAction);
	WSC_TIMER_INIT(pAdapter, pWscControl, &pWscControl->WscUPnPNodeInfo.UPnPMsgTimer, pWscControl->WscUPnPNodeInfo.bUPnPMsgTimerRunning, WscUPnPMsgTimeOutAction);
	pWscControl->WscUPnPNodeInfo.bUPnPMsgTimerPending = FALSE;
	WSC_TIMER_INIT(pAdapter, pWscControl, &pWscControl->M2DTimer, pWscControl->bM2DTimerRunning, WscM2DTimeOutAction);
#ifdef WSC_LED_SUPPORT
	WSC_TIMER_INIT(pAdapter, pWscControl, &pWscControl->WscLEDTimer, pWscControl->WscLEDTimerRunning, WscLEDTimer);
	WSC_TIMER_INIT(pAdapter, pWscControl, &pWscControl->WscSkipTurnOffLEDTimer, pWscControl->WscSkipTurnOffLEDTimerRunning, WscSkipTurnOffLEDTimer);
#endif /* WSC_LED_SUPPORT */
}

VOID	WscInitClientTimers(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PWSC_CTRL pWScControl)
{
	WSC_TIMER_INIT(pAdapter, pWScControl, &pWScControl->WscPBCTimer, pWScControl->WscPBCTimerRunning, WscPBCTimeOutAction);
#ifdef WSC_STA_SUPPORT
	WSC_TIMER_INIT(pAdapter, pWScControl, &pWScControl->WscPINTimer, pWScControl->WscPINTimerRunning, WscPINTimeOutAction);
#endif
	WSC_TIMER_INIT(pAdapter, pWScControl, &pWScControl->WscScanTimer, pWScControl->WscScanTimerRunning, WscScanTimeOutAction);
	WSC_TIMER_INIT(pAdapter, pWScControl, &pWScControl->WscProfileRetryTimer, pWScControl->WscProfileRetryTimerRunning, WscProfileRetryTimeout);  /* add by johnli, fix WPS test plan 5.1.1 */
#ifdef CON_WPS
	WSC_TIMER_INIT(pAdapter, pWScControl, &pWScControl->ConWscApcliScanDoneCheckTimer, pWScControl->ConWscApcliScanDoneCheckTimerRunning, WscScanDoneCheckTimeOutAction);
#endif /*CON_WPS*/
}

/*
*	==========================================================================
*	Description:
*		wps state machine init, including state transition and timer init
*	Parameters:
*		S - pointer to the association state machine
*	==========================================================================
*/
VOID    WscStateMachineInit(
	IN  PRTMP_ADAPTER pAd,
	IN  STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC Trans[])
{
	PWSC_CTRL pWScControl;

	StateMachineInit(S,	(STATE_MACHINE_FUNC *)Trans, MAX_WSC_STATE, MAX_WSC_MSG, (STATE_MACHINE_FUNC)Drop, WSC_IDLE, WSC_MACHINE_BASE);
	StateMachineSetAction(S, WSC_IDLE, WSC_EAPOL_START_MSG, (STATE_MACHINE_FUNC)WscEAPOLStartAction);
	StateMachineSetAction(S, WSC_IDLE, WSC_EAPOL_PACKET_MSG, (STATE_MACHINE_FUNC)WscEAPAction);
	StateMachineSetAction(S, WSC_IDLE, WSC_EAPOL_UPNP_MSG, (STATE_MACHINE_FUNC)WscEAPAction);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		UCHAR apidx;

		for (apidx = 0; apidx < MAX_MBSSID_NUM(pAd); apidx++) {
			pWScControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			pWScControl->pAd = pAd;
			pWScControl->wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			pWScControl->EntryIfIdx = (MIN_NET_DEVICE_FOR_MBSSID | apidx);
			WscInitCommonTimers(pAd, pWScControl);
			pWScControl->WscUpdatePortCfgTimerRunning = FALSE;
			WSC_TIMER_INIT(pAd, pWScControl, &pWScControl->WscUpdatePortCfgTimer, pWScControl->WscUpdatePortCfgTimerRunning, WscUpdatePortCfgTimeout);
#ifdef WSC_V2_SUPPORT
			WSC_TIMER_INIT(pAd, pWScControl, &pWScControl->WscSetupLockTimer, pWScControl->WscSetupLockTimerRunning, WscSetupLockTimeout);
#endif /* WSC_V2_SUPPORT */
		}

#ifdef APCLI_SUPPORT

		for (apidx = 0; apidx < MAX_APCLI_NUM; apidx++) {
			pWScControl = &pAd->StaCfg[apidx].wdev.WscControl;
			pWScControl->pAd = pAd;
			pWScControl->wdev = &pAd->StaCfg[apidx].wdev;
			pWScControl->EntryIfIdx = (MIN_NET_DEVICE_FOR_APCLI | apidx);
			WscInitCommonTimers(pAd, pWScControl);
			WscInitClientTimers(pAd, pWScControl);
		}

#endif /* APCLI_SUPPORT */
#ifdef CON_WPS
		RTMPInitTimer(pAd, &pAd->ApCfg.ConWpsApCliBandMonitorTimer, GET_TIMER_FUNCTION(ConWpsApCliMonitorTimeout), pAd, FALSE);
		pAd->ApCfg.ConWpsMonitorTimerRunning = FALSE;
#endif /* CON_WPS */
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		UCHAR IdSta;

		for (IdSta = 0; IdSta < pAd->MaxMSTANum; IdSta++) {
			pWScControl = &pAd->StaCfg[IdSta].wdev.WscControl;
			pWScControl->pAd = pAd;
			pWScControl->wdev = &pAd->StaCfg[IdSta].wdev;
			pWScControl->EntryIfIdx = IdSta;
			WscInitCommonTimers(pAd, pWScControl);
			WscInitClientTimers(pAd, pWScControl);
#ifdef P2P_SUPPORT
			pWScControl = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.WscControl;
			pWScControl->EntryIfIdx = MIN_NET_DEVICE_FOR_P2P_GO;
			WscInitCommonTimers(pAd, pWScControl);
			pWScControl->WscUpdatePortCfgTimerRunning = FALSE;
			WSC_TIMER_INIT(pAd, pWScControl, &pWScControl->WscUpdatePortCfgTimer, pWScControl->WscUpdatePortCfgTimerRunning, WscUpdatePortCfgTimeout);
			pWScControl = &pAd->StaCfg[MAIN_MBSSID].WscControl;
			pWScControl->EntryIfIdx = MIN_NET_DEVICE_FOR_P2P_CLI;
			WscInitCommonTimers(pAd, pWScControl);
			WscInitClientTimers(pAd, pWScControl);
#endif /* P2P_SUPPORT */
		}
	}
#endif /* CONFIG_STA_SUPPORT */
}

void WscM2DTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	/* For each state, we didn't care about the retry issue, we just send control message
	*	to notify the UPnP deamon that some error happened in STATE MACHINE.
	*/
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	PRTMP_ADAPTER pAd = NULL;
	WSC_UPNP_NODE_INFO *pWscNodeInfo;
#ifdef CONFIG_AP_SUPPORT
	MAC_TABLE_ENTRY *pEntry = NULL;
#endif /* CONFIG_AP_SUPPORT */
	BOOLEAN Cancelled;
	UCHAR CurOpMode = 0xFF;

	if (!pWscControl) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 "pWscControl is NULL!!\n");
		return;
	}

	pAd = (PRTMP_ADAPTER)pWscControl->pAd;

	if (!pAd) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 "pAd is NULL!!\n");
		return;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	CurOpMode = STA_MODE;
#ifdef P2P_SUPPORT

	if (pWscControl->EntryIfIdx != BSS0)
		CurOpMode = AP_MODE;

#endif /* P2P_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE)
		pEntry = MacTableLookup(pAd, pWscControl->EntryAddr);

#endif /* CONFIG_AP_SUPPORT */
	pWscNodeInfo = &pWscControl->WscUPnPNodeInfo;

	if (!pWscNodeInfo) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 "pWscNodeInfo is NULL!!\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "UPnP StateMachine TimeOut(State=%d!)\n", pWscControl->WscState);

	if (
#ifdef CONFIG_AP_SUPPORT
		(((pEntry == NULL) || (pWscNodeInfo->registrarID != 0)) &&  (CurOpMode == AP_MODE)) ||
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		((pWscNodeInfo->registrarID != 0) &&  (CurOpMode == STA_MODE)) ||
#endif /* CONFIG_STA_SUPPORT */
		(0)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s():pEntry maybe gone or already received M2 Packet!\n", __func__);
		goto done;
	}

	if (pWscControl->M2DACKBalance != 0) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s(): waiting for M2DACK balance, extend the time!\n", __func__);
		/* Waiting for M2DACK balance. */
		RTMPModTimer(&pWscControl->M2DTimer, WSC_EAP_ID_TIME_OUT);
		pWscControl->M2DACKBalance = 0;
		goto done;
	} else {
		RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
		pWscControl->EapolTimerRunning = FALSE;
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == AP_MODE) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s(): send EAP-Fail to wireless Station!\n", __func__);
			/* Send EAPFail to Wireless Station and reset the status of Wsc. */
			WscSendEapFail(pAd, pWscControl, TRUE);

			/*pEntry->bWscCapable = FALSE; */
			if (pEntry != NULL)
				pEntry->Receive_EapolStart_EapRspId = 0;
		}

#endif /* CONFIG_AP_SUPPORT */
		pWscControl->EapMsgRunning = FALSE;
		pWscControl->WscState = WSC_STATE_OFF;
	}

done:
	pWscControl->bM2DTimerRunning = FALSE;
	pWscControl->M2DACKBalance = 0;
	pWscNodeInfo->registrarID = 0;
}


VOID WscUPnPMsgTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	PRTMP_ADAPTER pAd = NULL;
	WSC_UPNP_NODE_INFO *pWscNodeInfo;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscUPnPMsgTimeOutAction\n");

	/*It shouldn't happened! */
	if (!pWscControl)
		return;

	pAd = (PRTMP_ADAPTER)pWscControl->pAd;

	if (!pAd) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 "pAd is NULL!!\n");
		return;
	}

	pWscNodeInfo = &pWscControl->WscUPnPNodeInfo;

	if (!pWscNodeInfo) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 "pWscNodeInfo is NULL!!\n");
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "UPnP StateMachine TimeOut(State=%d!)\n", pWscControl->WscState);

	if (pWscNodeInfo->bUPnPMsgTimerPending) {
#define WSC_UPNP_TIMER_PENDIND_WAIT	2000
		RTMPModTimer(&pWscNodeInfo->UPnPMsgTimer, WSC_UPNP_TIMER_PENDIND_WAIT);
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "UPnPMsgTimer Pending......\n");
	} else {
		int dataLen;
		UCHAR *pWscData;

		os_alloc_mem(NULL, (UCHAR **)&pWscData, WSC_MAX_DATA_LEN);

		if (pWscData != NULL) {
			memset(pWscData, 0, WSC_MAX_DATA_LEN);
			dataLen = BuildMessageNACK(pAd, pWscControl, pWscData);
			WscSendUPnPMessage(pAd, (pWscControl->EntryIfIdx & 0x1F),
							   WSC_OPCODE_UPNP_DATA, WSC_UPNP_DATA_SUB_NORMAL,
							   pWscData, dataLen, 0, 0, &pAd->CurrentAddress[0], AP_MODE);
			os_free_mem(pWscData);
		}

		pWscNodeInfo->bUPnPInProgress = FALSE;
		pWscNodeInfo->bUPnPMsgTimerPending = FALSE;
		pWscNodeInfo->bUPnPMsgTimerRunning = FALSE;
		pWscControl->WscState = WSC_STATE_OFF;
		pWscControl->WscStatus = STATUS_WSC_FAIL;
		RTMPSendWirelessEvent(pAd, IW_WSC_STATUS_FAIL, NULL, (pWscControl->EntryIfIdx & 0x1F), 0);
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscUPnPMsgTimeOutAction\n");
}

/*
*	==========================================================================
*	Description:
*		This function processes EapolStart packets from wps stations
*		or enqueued by self.
*
*	Return:
*		None
*	==========================================================================
*/
VOID WscEAPOLStartAction(
	IN  PRTMP_ADAPTER pAd,
	IN  MLME_QUEUE_ELEM * Elem)
{
	MAC_TABLE_ENTRY *pEntry;
	PWSC_CTRL pWpsCtrl = NULL;
	PHEADER_802_11 pHeader;
	PWSC_PEER_ENTRY pWscPeer = NULL;
	UCHAR CurOpMode;
	struct wifi_dev *wdev;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscEAPOLStartAction\n");
	wdev = Elem->wdev;
	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "wdev is NULL.\n");
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscEAPOLStartAction\n");
		return;
	}
	pHeader = (PHEADER_802_11)Elem->Msg;
	wdev->wdev_ops->mac_entry_lookup(pAd, pHeader->Addr2, wdev, &pEntry);
	pWpsCtrl = &wdev->WscControl;

	/* Cannot find this wps station in MacTable of WPS AP. */
	if (pEntry == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN, "pEntry is NULL.\n");
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscEAPOLStartAction\n");
		return;
	}

	if (WDEV_WSC_AP(wdev))
		CurOpMode = AP_MODE;
	else
		CurOpMode = STA_MODE;

#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
#ifdef IWSC_SUPPORT

		if ((pAd->StaCfg[0].BssType == BSS_ADHOC) &&
			(IWSC_PeerEapolStart(pAd, pEntry, Elem) == FALSE)) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Rejected by IWSC SM. Ignore EAPOL-Start.\n");
			return;
		}

#endif /* IWSC_SUPPORT */
	}

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef CON_WPS
	{
		PWSC_CTRL pApCliWpsCtrl = NULL;

		if (CurOpMode == AP_MODE) {
			/*we can not use GetStaCfgByWdev, as it returns NULL in case of AP mode*/
			if (pEntry->func_tb_idx < MAX_APCLI_NUM)
				pApCliWpsCtrl = &pAd->StaCfg[pEntry->func_tb_idx].wdev.WscControl;
			else
				pApCliWpsCtrl = &pAd->StaCfg[BSS0].wdev.WscControl;
		}
		if (pApCliWpsCtrl)
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN, "CON_WPS: Stop the ApCli WPS, state [%d]\n", pApCliWpsCtrl->WscState);

		if (pApCliWpsCtrl && (pApCliWpsCtrl->conWscStatus != CON_WPS_STATUS_DISABLED) &&
			(pApCliWpsCtrl->WscState != WSC_STATE_OFF)) {
			WscConWpsStop(pAd, TRUE, pApCliWpsCtrl);
			WscStop(pAd, TRUE, pApCliWpsCtrl);
			pApCliWpsCtrl->WscConfMode = WSC_DISABLE;
			/* APCLI: For stop the other side of the band with WSC SM */
		}
	}
#endif /* CON_WPS */
#endif /* CONFIG_AP_SUPPORT */
	RTMP_SEM_LOCK(&pWpsCtrl->WscPeerListSemLock);
	WscInsertPeerEntryByMAC(&pWpsCtrl->WscPeerList, pEntry->Addr);
	RTMP_SEM_UNLOCK(&pWpsCtrl->WscPeerListSemLock);
	WscMaintainPeerList(pAd, pWpsCtrl);

	/*
	*	Check this STA is first one or not
	*/
	if (pWpsCtrl->WscPeerList.size != 0) {
		pWscPeer = (PWSC_PEER_ENTRY)pWpsCtrl->WscPeerList.pHead;

		if (NdisEqualMemory(pEntry->Addr, pWscPeer->mac_addr, MAC_ADDR_LEN) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "This is not first WSC peer, ignore this EAPOL_Start!\n");
			hex_dump("pEntry->Addr", pEntry->Addr, MAC_ADDR_LEN);
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE)
				WscApShowPeerList(pAd, NULL);

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

			if (CurOpMode == STA_MODE)
				WscStaShowPeerList(pAd, NULL);

#endif /* CONFIG_STA_SUPPORT */
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscEAPOLStartAction\n");
			return;
		}
	}

#ifdef P2P_SUPPORT

	if (P2P_GO_ON(pAd) && (pWpsCtrl->bWscTrigger == FALSE)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Ignore this EAPOL_Start!\n");
		return;
	}

#endif /* P2P_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscState = %d\n", pWpsCtrl->WscState);

	if ((pEntry->Receive_EapolStart_EapRspId == 0) ||
		(pWpsCtrl->WscState <= WSC_STATE_WAIT_REQ_ID)) {
		/* Receive the first EapolStart packet of this wps station. */
		pEntry->Receive_EapolStart_EapRspId |= WSC_ENTRY_GET_EAPOL_START;
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEAPOLStartAction - receive EAPOL-Start from "MACSTR"\n",
				 MAC2STR(pEntry->Addr));
		/* EapolStart packet is sent by station means this station wants to do wps process with AP. */
		pWpsCtrl->EapMsgRunning = TRUE;
		/* Update EntryAddr again */
		NdisMoveMemory(pWpsCtrl->EntryAddr, pEntry->Addr, MAC_ADDR_LEN);

		if (pEntry->bWscCapable == FALSE)
			pEntry->bWscCapable = TRUE;

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEAPOLStartAction(ra%d) - send EAP-Req(Id) to "MACSTR"\n",
				 pEntry->func_tb_idx, MAC2STR(pEntry->Addr));
		/* Send EAP-Request/Id to station */
		WscSendEapReqId(pAd, pEntry, CurOpMode);
		pWpsCtrl->WscStatus = STATUS_WSC_EAP_REQ_ID_SENT;
		pWpsCtrl->WscState = WSC_STATE_WAIT_RESP_ID;

#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_TURNKEY_ENABLE(pAd))
			wapp_send_wsc_eapol_start_notification(pAd, wdev);
#endif /* CONFIG_MAP_SUPPORT */
		if (!pWpsCtrl->EapolTimerRunning) {
			pWpsCtrl->EapolTimerRunning = TRUE;
			/* Set WPS_EAP Messages timeout function. */
			RTMPSetTimer(&pWpsCtrl->EapolTimer, WSC_EAP_ID_TIME_OUT);
		}
	} else
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Ignore EAPOL-Start.\n");

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscEAPOLStartAction\n");
}

#ifdef APCLI_SUPPORT
#ifdef CON_WPS
static VOID WscConWPSChecking(
	IN  PRTMP_ADAPTER pAd,
	IN  PRTMP_ADAPTER pOpposAd,
	IN  UCHAR if_idx,
	IN  PWSC_CTRL pWscControl)
{
#ifdef MULTI_INF_SUPPORT

	if (pOpposAd) {
		if (pOpposAd->ApCfg.ConWpsApCliStatus == TRUE) {
			PRTMP_ADAPTER pActionAd;
			BOOLEAN Cancelled;

			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
					 "%s another Band WPS ready\n", pAd->net_dev->name);

			/* another Band WPS ready */
			if (pOpposAd->ApCfg.ConWpsMonitorTimerRunning) {
				RTMPCancelTimer(&pOpposAd->ApCfg.ConWpsApCliBandMonitorTimer, &Cancelled);
				pOpposAd->ApCfg.ConWpsMonitorTimerRunning = FALSE;

				/* BandSelection */
				if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_2G)
					pActionAd = (PRTMP_ADAPTER)adapt_list[0];
				else
					pActionAd = (PRTMP_ADAPTER)adapt_list[1];

				pActionAd->StaCfg[if_idx].ApcliInfStat.Enable = FALSE;
				ApCliIfDown(pActionAd);
				pActionAd->StaCfg[if_idx].ApcliInfStat.Enable = TRUE;
			}
		} else {
			/* This interface will be enabled in ConWpsApCliBandMonitorTimer */
			pAd->StaCfg[if_idx].ApcliInfStat.Enable = FALSE;

			ApCliIfDown(pAd);
			RTMPSetTimer(&pAd->ApCfg.ConWpsApCliBandMonitorTimer, CON_WPS_APCLIENT_MONITOR_TIME);
			pAd->ApCfg.ConWpsMonitorTimerRunning = TRUE;
		}

		pWscControl->conWscStatus = CON_WPS_STATUS_DISABLED;
	} else
#endif /* MULTI_INF_SUPPORT */
	{
		if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_2G)
			pAd->StaCfg[1].ApcliInfStat.Enable = FALSE;
		else
			pAd->StaCfg[0].ApcliInfStat.Enable = FALSE;

		ApCliIfDown(pAd);

		if (pWscControl->WscState != WSC_STATE_OFF) {
			RTMPSetTimer(&pAd->ApCfg.ConWpsApCliBandMonitorTimer, CON_WPS_APCLIENT_MONITOR_TIME);
			pAd->ApCfg.ConWpsMonitorTimerRunning = TRUE;
		}
	}
}
#endif /* CON_WPS */
#endif /* APCLI_SUPPORT */

#ifdef P2P_SUPPORT
static VOID WscP2pCheckConfigMethod(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl,
	IN  UCHAR *MacAddr,
	OUT BOOLEAN *bReadOwnPIN)
{
	if (P2P_CLI_ON(pAd) && (pWscControl->EntryIfIdx != BSS0)) {
		UCHAR	P2pIdx = P2P_NOT_FOUND;

		P2pIdx = P2pGroupTabSearch(pAd, MacAddr);

		if (P2pIdx != P2P_NOT_FOUND) {
			PRT_P2P_CLIENT_ENTRY pP2pEntry = &pAd->P2pTable.Client[P2pIdx];

			if (pP2pEntry && (pAd->P2pCfg.ConfigMethod == WSC_CONFMET_KEYPAD)) {
				/*
				*	I am KeyPad. We cannot use ConfigMethod or DPID to check peer's capability.
				*	Some P2P device is display but the value of ConfigMethod will be 0x0188 and  (ex. Samsung GALAXYSII).
				*/
				*bReadOwnPIN = FALSE;
			}
		}
	}
}
#endif /* P2P_SUPPORT */

/*
*	==========================================================================
*	Description:
*		This is state machine function when receiving EAP packets
*		which is WPS Registration Protocol.
*
*		There are two roles at our AP, as an
*		1. Enrollee
*		2. Internal Registrar
*		3. Proxy
*
*		There are two roles at our Station, as an
*		1. Enrollee
*		2. External Registrar
*
*		Running Scenarios:
*		-----------------------------------------------------------------
*		1a. Adding an AP as an Enrollee to a station as an External Registrar (EAP)
*			[External Registrar]<----EAP--->[Enrollee_AP]
*		-----------------------------------------------------------------
*		2a. Adding a station as an Enrollee to an AP with built-in Registrar (EAP)
*			[Registrar_AP]<----EAP--->[Enrollee_STA]
*		-----------------------------------------------------------------
*		3a. Adding an Enrollee with External Registrar (UPnP/EAP)
*			[External Registrar]<----UPnP--->[Proxy_AP]<---EAP--->[Enrollee_STA]
*		-----------------------------------------------------------------
*
*	Return:
*		None
*	==========================================================================
*/
VOID WscEAPAction(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MLME_QUEUE_ELEM * Elem)
{
	UCHAR MsgType;
	BOOLEAN bUPnPMsg, Cancelled;
	MAC_TABLE_ENTRY	*pEntry = NULL;
	UCHAR MacAddr[MAC_ADDR_LEN] = {0};
#ifdef CONFIG_AP_SUPPORT
	UCHAR apidx = MAIN_MBSSID;
	UCHAR current_band = 0;
	UCHAR bss_index = 0;
#endif /* CONFIG_AP_SUPPORT */
	PWSC_CTRL pWscControl = NULL;
	PWSC_UPNP_NODE_INFO pWscUPnPNodeInfo = NULL;
	UCHAR CurOpMode;
	struct wifi_dev *wdev = NULL;
	struct wifi_dev_ops *ops = NULL;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG sta_cfg = NULL;
#endif /* CONFIG_STA_SUPPORT */
	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			 "-----> WscEAPAction\n");
	/* The first 6 bytes in Elem->Msg is the MAC address of wps peer. */
	memmove(MacAddr, Elem->Msg, MAC_ADDR_LEN);
	memmove(Elem->Msg, Elem->Msg + 6, Elem->MsgLen);
#ifdef DBG
	hex_dump("(WscEAPAction)Elem->MsgLen", Elem->Msg, Elem->MsgLen);
#endif /* DBG */
	MsgType = WscRxMsgType(pAdapter, Elem);
	bUPnPMsg = Elem->MsgType == WSC_EAPOL_UPNP_MSG ? TRUE : FALSE;
	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			 "WscEAPAction: Addr: "MACSTR", MsgType: 0x%02X, bUPnPMsg: %s\n",
			  MAC2STR(MacAddr), MsgType, bUPnPMsg ? "TRUE" : "FALSE");
	wdev = Elem->wdev;
	ops = wdev->wdev_ops;
	pWscControl = &wdev->WscControl;

	if (!bUPnPMsg)
		ops->mac_entry_lookup(pAdapter, MacAddr, wdev, &pEntry);

	if (WDEV_WSC_AP(wdev))
		CurOpMode = AP_MODE;
	else {
		CurOpMode = STA_MODE;
#ifdef CONFIG_STA_SUPPORT
		sta_cfg = GetStaCfgByWdev(pAdapter, wdev);
		ASSERT(sta_cfg);
#endif /* CONFIG_STA_SUPPORT */
	}

#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (!bUPnPMsg) {
			if (pEntry) {
				if (IS_ENTRY_CLIENT(pEntry) && pEntry->func_tb_idx >= pAdapter->ApCfg.BssidNum) {
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
							"WscEAPAction: Unknown apidex(=%d).\n", pEntry->func_tb_idx);
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "<----- WscEAPAction\n");
					return;
				}

				apidx = pEntry->func_tb_idx;
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
						 "WscEAPAction: apidex=%d.\n", pEntry->func_tb_idx);
			} else {
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
						"WscEAPAction: pEntry is NULL.\n");
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
						 "<----- WscEAPAction\n");
				return;
			}
		}
	}

#endif /* CONFIG_AP_SUPPORT */
	pWscUPnPNodeInfo = &pWscControl->WscUPnPNodeInfo;
	pWscUPnPNodeInfo->bUPnPMsgTimerPending = TRUE;

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		if ((MsgType == WSC_MSG_EAP_REG_RSP_ID)
			|| (MsgType == WSC_MSG_EAP_ENR_RSP_ID)) {
			if ((pEntry->Receive_EapolStart_EapRspId & WSC_ENTRY_GET_EAP_RSP_ID)
				&& (pWscControl->WscState > WSC_STATE_WAIT_M1)) {
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
						 "WscEAPAction: Already receive EAP_RSP(Identitry) from this STA, ignore it.\n");
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
						 "<----- WscEAPAction\n");
				return;
			}

			pEntry->Receive_EapolStart_EapRspId |= WSC_ENTRY_GET_EAP_RSP_ID;
		}
	}

	pWscControl->EapolTimerPending = TRUE;
#ifdef WSC_V2_SUPPORT

	if (MsgType == WSC_MSG_EAP_FRAG_ACK) {
		WscSendEapFragData(pAdapter, pWscControl, pEntry);
		return;
	}

#endif /* WSC_V2_SUPPORT */

	if (MsgType == WSC_MSG_EAP_REG_RSP_ID) {

		/* Receive EAP-Response/Id from external registrar, so the role of AP is enrollee. */
		if (((pWscControl->WscConfMode & WSC_ENROLLEE) != 0) ||
			(((pWscControl->WscConfMode & WSC_PROXY) != 0) && bUPnPMsg)) {
			pWscControl->WscActionMode = WSC_ENROLLEE;
			pWscControl->WscUseUPnP = bUPnPMsg ? 1 : 0;
			MsgType = WSC_MSG_EAP_RSP_ID;
			WscEapEnrolleeAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
		}
	} else if (MsgType == WSC_MSG_EAP_ENR_RSP_ID) {
		/* Receive EAP-Response/Id from wps enrollee station, so the role of AP is Registrar or Proxy. */
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEAPAction: Rx Identity\n");
		pWscControl->WscActionMode = WSC_REGISTRAR;

		if (bUPnPMsg) {
			/* Receive enrollee identity from UPnP */
		} else {
#ifdef CONFIG_AP_SUPPORT

			/* Receive enrollee identity from EAP */
			if ((pWscControl->WscMode == WSC_PBC_MODE)
#ifdef P2P_SUPPORT
				/*
				*	P2P doesn't need to check PBC overlapping.
				*/
				&& (pWscControl->EntryIfIdx < MIN_NET_DEVICE_FOR_P2P_GO)
#endif /* P2P_SUPPORT */
			   ) {
				/*
				*	Some WPS PBC Station select AP from UI directly; doesn't do PBC scan.
				*	Need to check DPID from STA again here.
				*/
				current_band = HcGetBandByChannel(pAdapter, Elem->Channel);

				WscPBC_DPID_FromSTA(pAdapter, pWscControl->EntryAddr, current_band);
				WscPBCSessionOverlapCheck(pAdapter, current_band);

				if ((pAdapter->CommonCfg.WscStaPbcProbeInfo.WscPBCStaProbeCount[current_band] == 1) &&
					!NdisEqualMemory(pAdapter->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[current_band][0], &ZERO_MAC_ADDR[0], MAC_ADDR_LEN) &&
					(NdisEqualMemory(pAdapter->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[current_band][0], &pWscControl->EntryAddr[0], 6) == FALSE)) {
					for (bss_index = 0; bss_index < pAdapter->ApCfg.BssidNum; bss_index++) {
						if ((current_band == HcGetBandByWdev(&pAdapter->ApCfg.MBSSID[bss_index].wdev)) &&
								pAdapter->ApCfg.MBSSID[bss_index].wdev.WscControl.WscConfMode != WSC_DISABLE &&
								pAdapter->ApCfg.MBSSID[bss_index].wdev.WscControl.bWscTrigger == TRUE) {
							MTWF_DBG(pAdapter, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
									"%s(): found pAd->ApCfg.MBSSID[%d] WPS on\n",
									__func__, bss_index);
							break;
						}
					}

					/*bss in current band has triggered wps pbc, so check Peer DPID*/
					if (bss_index < pAdapter->ApCfg.BssidNum) {
						MTWF_DBG(pAdapter, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"pAd->ApCfg.MBSSID[%d] WPS on, PBC Overlap detected\n",
							bss_index);
						pAdapter->CommonCfg.WscPBCOverlap = TRUE;
					} else {
						MTWF_DBG(pAdapter, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"pAd->ApCfg.MBSSID[%d] WPS off, PBC Overlap is invalid\n",
							bss_index);
						pAdapter->CommonCfg.WscPBCOverlap = FALSE;
					}
				}

				if (pAdapter->CommonCfg.WscPBCOverlap) {
					hex_dump("EntryAddr", pWscControl->EntryAddr, 6);
					hex_dump("StaMacAddr0", pAdapter->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[current_band][0], 6);
					hex_dump("StaMacAddr1", pAdapter->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[current_band][1], 6);
					hex_dump("StaMacAddr2", pAdapter->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[current_band][2], 6);
					hex_dump("StaMacAddr3", pAdapter->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[current_band][3], 6);
				}
			}

			if ((pWscControl->WscMode == WSC_PBC_MODE) &&
				(pAdapter->CommonCfg.WscPBCOverlap == TRUE)) {
				/* PBC session overlap */
				pWscControl->WscStatus = STATUS_WSC_PBC_SESSION_OVERLAP;
					RTMPSendWirelessEvent(pAdapter, IW_WSC_PBC_SESSION_OVERLAP, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);

				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "WscEAPAction: PBC Session Overlap!\n");
#ifdef VENDOR_FEATURE6_SUPPORT
				WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x0F), WSC_OPCODE_UPNP_CTRL, WSC_UPNP_DATA_SUB_PBC_OVERLAP, &pAdapter->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x1F].wdev.bssid[0], MAC_ADDR_LEN, 0, 0, &pAdapter->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x1F].wdev.bssid[0], AP_MODE);
#endif /* VENDOR_FEATURE6_SUPPORT */
			} else
#endif /* CONFIG_AP_SUPPORT */
				if ((pWscControl->WscConfMode & WSC_PROXY_REGISTRAR) != 0) {
					/* Notify UPnP daemon before send Eap-Req(wsc-start) */
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: pEntry->Addr="MACSTR"\n",
							 __func__, MAC2STR(pEntry->Addr));
#ifdef CONFIG_AP_SUPPORT

					if (CurOpMode == AP_MODE) {
						if (pEntry)
							WscSendUPnPConfReqMsg(pAdapter, (pWscControl->EntryIfIdx & 0x1F),
												  (PUCHAR)pAdapter->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid, pEntry->Addr, 2, 0, CurOpMode);

						/* Reset the UPnP timer and status. */
						if (pWscControl->bM2DTimerRunning == TRUE) {
							RTMPCancelTimer(&pWscControl->M2DTimer, &Cancelled);
							pWscControl->bM2DTimerRunning = FALSE;
						}

						pWscControl->WscUPnPNodeInfo.registrarID = 0;
						pWscControl->M2DACKBalance = 0;
					}

#endif /* CONFIG_AP_SUPPORT */
					pWscControl->EapMsgRunning = TRUE;
					/* Change the state to next one */
					pWscControl->WscState = WSC_STATE_WAIT_M1;

					/* send EAP WSC_START */
					if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
						pWscControl->bWscLastOne = TRUE;

						if (CurOpMode == AP_MODE)
							WscSendMessage(pAdapter, WSC_OPCODE_START, NULL, 0, pWscControl, AP_MODE, EAP_CODE_REQ);

#ifdef CONFIG_STA_SUPPORT
						else if (ADHOC_ON(pAdapter) && (pWscControl->WscConfMode == WSC_REGISTRAR))
							WscSendMessage(pAdapter, WSC_OPCODE_START, NULL, 0, pWscControl, STA_MODE, EAP_CODE_REQ);
						else
							WscSendMessage(pAdapter, WSC_OPCODE_START, NULL, 0, pWscControl, STA_MODE, EAP_CODE_RSP);

#endif /* CONFIG_STA_SUPPORT */
					}
				}
		}
	} else if (MsgType == WSC_MSG_EAP_REQ_ID) {
		/* Receive EAP_Req/Identity from WPS AP or WCN */
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Receive EAP_Req/Identity from WPS AP or WCN\n");

		if (bUPnPMsg && (pWscControl->WscConfMode == WSC_ENROLLEE)) {
			pWscControl->WscActionMode = WSC_ENROLLEE;
			pWscControl->WscUseUPnP = 1;
			WscEapEnrolleeAction(pAdapter, Elem, WSC_MSG_EAP_REQ_START, pEntry, pWscControl);
		} else {
			/* Receive EAP_Req/Identity from WPS AP */
			if (pEntry != NULL)
				WscSendEapRspId(pAdapter, pEntry, pWscControl);
		}

		if (!bUPnPMsg) {
			if ((pWscControl->WscState < WSC_STATE_WAIT_M1) ||
				(pWscControl->WscState > WSC_STATE_WAIT_ACK)) {
				if (pWscControl->WscConfMode == WSC_REGISTRAR)
					pWscControl->WscState = WSC_STATE_WAIT_M1;
				else
					pWscControl->WscState = WSC_STATE_WAIT_WSC_START;
			}
		}
	} else if (MsgType == WSC_MSG_EAP_REQ_START) {
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Receive EAP_Req(Wsc_Start) from WPS AP\n");

		/* Receive EAP_Req(Wsc_Start) from WPS AP */
		if (pWscControl->WscConfMode == WSC_ENROLLEE) {
			pWscControl->WscActionMode = WSC_ENROLLEE;
			pWscControl->WscUseUPnP = bUPnPMsg ? 1 : 0;
			WscEapEnrolleeAction(pAdapter, Elem, WSC_MSG_EAP_REQ_START, pEntry, pWscControl);

			if (!pWscControl->EapolTimerRunning) {
				pWscControl->EapolTimerRunning = TRUE;
				RTMPSetTimer(&pWscControl->EapolTimer, WSC_EAP_ID_TIME_OUT);
			}
		} else
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Ignore EAP_Req(Wsc_Start) from WPS AP\n");
	} else if (MsgType == WSC_MSG_EAP_FAIL) {
		/* Receive EAP_Fail from WPS AP */
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Receive EAP_Fail from WPS AP\n");

		if (pWscControl->WscState >= WSC_STATE_WAIT_EAPFAIL) {
#ifdef WIDI_SUPPORT
#ifdef CONFIG_STA_SUPPORT
			int sendFail = 0;

			if (pAdapter->StaCfg[0].bWIDI && !pAdapter->StaCfg[0].WscControl.bWscTrigger) {
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s : WscStatus=%d\n", __func__, pWscControl->WscStatus);

				if (pWscControl->WscStatus != STATUS_WSC_CONFIGURED)
					sendFail = 1;
			}

#endif /* CONFIG_STA_SUPPORT */
#endif /* WIDI_SUPPORT */

			/*Inform Disassoc that EAPHandshake is completed*/
			if (pWscControl->WscState == WSC_STATE_WAIT_EAPFAIL)
				RTMP_OS_COMPLETE(&pWscControl->WscEAPHandshakeCompleted);

			pWscControl->WscState = WSC_STATE_WAIT_DISCONN;

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT

			if ((CurOpMode == STA_MODE) &&
				(IF_COMBO_HAVE_AP_STA(pAdapter) && wdev->wdev_type == WDEV_TYPE_STA)) {
				UCHAR if_idx = (pWscControl->EntryIfIdx & 0x0F);

				pWscControl->WscConfMode = WSC_DISABLE;
#ifdef CON_WPS

				if (pWscControl->conWscStatus != CON_WPS_STATUS_DISABLED) {
					if  (pAdapter->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_AUTO) {
						WscConWpsStop(pAdapter, TRUE, pWscControl);
						pWscControl->conWscStatus = CON_WPS_STATUS_DISABLED;
					} else {
						PRTMP_ADAPTER pOpposAd = NULL;
#ifdef MULTI_INF_SUPPORT
						UINT nowBandIdx, opposBandIdx;

						nowBandIdx = multi_inf_get_idx(pAdapter);
						opposBandIdx = !nowBandIdx;
						pOpposAd = (PRTMP_ADAPTER)adapt_list[opposBandIdx];
#endif /* MULTI_INF_SUPPORT */
						pAdapter->ApCfg.ConWpsApCliStatus = TRUE;
						WscConWPSChecking(pAdapter, pOpposAd, if_idx, pWscControl);
					}
				}

#endif /* CON_WPS */
#ifdef P2P_SUPPORT

				if (pWscControl->WscStatus == STATUS_WSC_CONFIGURED)
					pAdapter->P2pCfg.WscState = WSC_STATE_CONFIGURED;

#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
				P2pSendWirelessEvent(pAdapter, RT_P2P_WPS_COMPLETED, NULL, NULL);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
#endif /* P2P_SUPPORT */
				/* Bring apcli interface down first */
				if (pEntry && IS_ENTRY_PEER_AP(pEntry) && pAdapter->StaCfg[if_idx].ApcliInfStat.Enable == TRUE
#ifdef CON_WPS
					&& (pAdapter->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_AUTO)
#endif /* CON_WPS */
				   ) {
#ifdef P2P_SUPPORT
					UCHAR P2pIdx = P2pGroupTabSearch(pAdapter, pEntry->Addr);
#endif /* P2P_SUPPORT */
#ifdef P2P_SUPPORT

					if ((P2pIdx != P2P_NOT_FOUND)
						&& P2P_CLI_ON(pAdapter)
						&& ((pWscControl->WscStatus == STATUS_WSC_ERROR_DEV_PWD_AUTH_FAIL) || (pWscControl->WscStatus == STATUS_WSC_FAIL))) {
						pAdapter->P2pTable.Client[P2pIdx].P2pClientState = P2PSTATE_DISCOVERY;
						P2pLinkDown(pAdapter, P2P_CONNECT_FAIL);
					}

#endif /* P2P_SUPPORT */
				}
			}

#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifndef APCLI_SUPPORT
#ifdef CONFIG_STA_SUPPORT

			if (CurOpMode == STA_MODE) {
#ifdef IWSC_SUPPORT

				if ((pAdapter->OpMode == OPMODE_STA) &&
					(pAdapter->StaCfg[0].BssType == BSS_ADHOC) &&
					(pAdapter->StaCfg[0].WscControl.WscConfMode == WSC_ENROLLEE))
					pAdapter->StaCfg[0].IWscInfo.bReStart = TRUE;

#endif /* IWSC_SUPPORT */
				pWscControl->WscConfMode = WSC_DISABLE;
				WscLinkDown(pAdapter, wdev);
#ifdef WIDI_SUPPORT

				if (pAdapter->StaCfg[0].bWIDI && !pAdapter->StaCfg[0].WscControl.bWscTrigger) {
					if (sendFail) {
						MLME_SCAN_REQ_STRUCT	ScanReq;

						WidiUpdateStateToDaemon(pAdapter, MIN_NET_DEVICE_FOR_MBSSID, WIDI_MSG_TYPE_ASSOC_STATUS,
												pAdapter->CommonCfg.Bssid, NULL, 0, WIDI_WPS_STATUS_FAIL);
						ScanParmFill(pAdapter, &ScanReq, "", 0, BSS_ANY, SCAN_PASSIVE);

						cntl_scan_request(wdev, &ScanReq);

					} else
						WidiUpdateStateToDaemon(pAdapter, MIN_NET_DEVICE_FOR_MBSSID, WIDI_MSG_TYPE_ASSOC_STATUS,
												pAdapter->CommonCfg.Bssid, NULL, 0, WIDI_WPS_STATUS_SUCCESS);
				}

#endif /* WIDI_SUPPORT */
			}

#endif /* CONFIG_STA_SUPPORT */
#endif
		} else if (pWscControl->WscState == WSC_STATE_RX_M2D) {
			/* Wait M2; */
#ifdef IWSC_SUPPORT
			/*
			*	We need to send EAPOL_Start again to trigger WPS process
			*/
			if (sta_cfg->BssType == BSS_ADHOC) {
				pWscControl->WscState = WSC_STATE_LINK_UP;
				pWscControl->WscStatus = STATUS_WSC_LINK_UP;
				WscSendEapolStart(pAdapter, pWscControl->WscPeerMAC, STA_MODE, wdev);
			}

#endif /* IWSC_SUPPORT */
		} else if ((pWscControl->WscState <= WSC_STATE_WAIT_REQ_ID) &&
				   (pWscControl->WscState != WSC_STATE_FAIL)) {
			/* Ignore. D-Link DIR-628 AP sometimes would send EAP_Fail to station after Link UP first then send EAP_Req/Identity. */
		} else {
			pWscControl->WscStatus = STATUS_WSC_FAIL;
			RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_FAIL, NULL, (pWscControl->EntryIfIdx & 0x1F), 0);
#ifdef IWSC_SUPPORT

			if ((pAdapter->OpMode == OPMODE_STA) &&
				(sta_cfg->BssType == BSS_ADHOC) &&
				(pWscControl->WscConfMode == WSC_ENROLLEE))
				wdev->IWscInfo.bReStart = TRUE;

#endif /* IWSC_SUPPORT */
			pWscControl->WscConfMode = WSC_DISABLE;
			/* Change the state to next one */
			pWscControl->WscState = WSC_STATE_OFF;
#ifdef CONFIG_STA_SUPPORT

			if (CurOpMode == STA_MODE) {
				WscLinkDown(pAdapter, wdev);
#ifdef WIDI_SUPPORT
				WidiUpdateStateToDaemon(pAdapter,
										MIN_NET_DEVICE_FOR_MBSSID,
										WIDI_MSG_TYPE_ASSOC_STATUS,
										pAdapter->CommonCfg.Bssid,
										NULL, 0, WIDI_WPS_STATUS_FAIL);
#endif /* WIDI_SUPPORT */
			}

#endif /* CONFIG_STA_SUPPORT */
		}
	} else if (MsgType == WSC_MSG_M1) {
		UINT32 rv = 0;

		/*
		*	If Buffalo WPS STA doesn't receive M2D from AP, Buffalo WPS STA will stop to do WPS.
		*	Therefore we need to receive M1 and send M2D without trigger.
		*/
		if ((pWscControl->WscConfMode & WSC_REGISTRAR) != 0) {
			pWscControl->WscActionMode = WSC_REGISTRAR;

			/* If Message is from EAP, but UPnP Registrar is in progress now, ignore EAP_M1 */
			if (!bUPnPMsg && pWscControl->WscUPnPNodeInfo.bUPnPInProgress) {
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
						 "UPnP Registrar is working now, ignore EAP M1.\n");
				goto out;
			} else {
				if (pEntry)
					WscEapRegistrarAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
			}

			rv = 1;
		}

#ifdef CONFIG_AP_SUPPORT

		if (((pWscControl->WscConfMode & WSC_PROXY) != 0) && (!bUPnPMsg) && (CurOpMode == AP_MODE)) {
			if ((pWscControl->bWscTrigger
				)
				&& (pWscControl->WscState >= WSC_STATE_WAIT_M3))
				;
			else {
				pWscControl->WscActionMode = WSC_PROXY;
				WscEapApProxyAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
			}
		} else if ((!pWscControl->bWscTrigger) && ((pWscControl->WscConfMode & WSC_PROXY) == 0) && (pAdapter->OpMode == OPMODE_AP)) {
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscTrigger is FALSE, ignore EAP M1.\n");
			goto out;
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		else {
			if ((rv == 0) && (CurOpMode == STA_MODE)) {
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "(Line:%d)Ignore EAP M1.\n", __LINE__);
				goto out;
			}
		}

#endif /* CONFIG_STA_SUPPORT */
	} else if (MsgType == WSC_MSG_M3 ||
			   MsgType == WSC_MSG_M5 ||
			   MsgType == WSC_MSG_M7 ||
			   MsgType == WSC_MSG_WSC_DONE) {
		BOOLEAN bNonceMatch = WscCheckNonce(pAdapter, Elem, TRUE, pWscControl);

		if (((pWscControl->WscConfMode & WSC_REGISTRAR) != 0) &&
			(pWscControl->bWscTrigger
			) &&
			bNonceMatch) {
			/* If Message is from EAP, but UPnP Registrar is in progress now, ignore EAP Messages */
			if (!bUPnPMsg && pWscControl->WscUPnPNodeInfo.bUPnPInProgress) {
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "UPnP Registrar is working now, ignore EAP Messages.\n");
				goto out;
			} else {
				pWscControl->WscActionMode = WSC_REGISTRAR;

				if (pEntry)
					WscEapRegistrarAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
			}
		}

#ifdef CONFIG_AP_SUPPORT
		else if (((pWscControl->WscConfMode & WSC_PROXY) != 0) && (!bUPnPMsg) && (CurOpMode == AP_MODE)) {
			pWscControl->WscActionMode = WSC_PROXY;
			WscEapApProxyAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
		}

#endif /* CONFIG_AP_SUPPORT */
	} else if (MsgType == WSC_MSG_M2 ||
			   MsgType == WSC_MSG_M2D ||
			   MsgType == WSC_MSG_M4 ||
			   MsgType == WSC_MSG_M6 ||
			   MsgType == WSC_MSG_M8) {
		BOOLEAN bNonceMatch = WscCheckNonce(pAdapter, Elem, FALSE, pWscControl);
		BOOLEAN bGoWPS = FALSE;

		if ((CurOpMode == AP_MODE) ||
			((CurOpMode == STA_MODE) &&
			 (pWscControl->bWscTrigger
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
			 )))
			bGoWPS = TRUE;

#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

		if ((CurOpMode == AP_MODE) &&
			((pWscControl->WscV2Info.bWpsEnable == FALSE) && (pWscControl->WscV2Info.bEnableWpsV2)))
			bGoWPS = FALSE;

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

		if (((pWscControl->WscConfMode & WSC_ENROLLEE) != 0) &&
			bGoWPS &&
			bNonceMatch) {
			pWscControl->WscActionMode = WSC_ENROLLEE;
			pWscControl->WscUseUPnP = bUPnPMsg ? 1 : 0;

			if (MsgType == WSC_MSG_M2) {
				BOOLEAN	bReadOwnPIN = TRUE;
#ifdef CONFIG_AP_SUPPORT

				/* WPS Enrollee AP only supports PIN without trigger */
				if (CurOpMode == AP_MODE) {
					if (pWscControl->bWscTrigger == FALSE) {
						pWscControl->WscMode = 1;
						WscGetConfWithoutTrigger(pAdapter, pWscControl, FALSE);
					} else if (!(pWscControl->EntryIfIdx & MIN_NET_DEVICE_FOR_APCLI)) {
						WscBuildBeaconIE(pAdapter,
										 pWscControl->WscConfStatus,
										 TRUE,
										 pWscControl->WscMode,
										 pWscControl->WscConfigMethods,
										 (pWscControl->EntryIfIdx & 0x1F),
										 NULL,
										 0,
										 AP_MODE);
						WscBuildProbeRespIE(pAdapter,
											WSC_MSGTYPE_AP_WLAN_MGR,
											pWscControl->WscConfStatus,
											TRUE,
											pWscControl->WscMode,
											pWscControl->WscConfigMethods,
											pWscControl->EntryIfIdx,
											NULL,
											0,
											AP_MODE);
						wdev = (struct wifi_dev *)pWscControl->wdev;
						UpdateBeaconHandler(
							pAdapter,
							wdev,
							BCN_UPDATE_IE_CHG);
					}
				}

#endif /* CONFIG_AP_SUPPORT */
#ifdef P2P_SUPPORT
				WscP2pCheckConfigMethod(pAdapter, pWscControl, MacAddr, &bReadOwnPIN);
#endif /* P2P_SUPPORT */
#ifdef IWSC_SUPPORT

				if (sta_cfg->BssType == BSS_ADHOC)
					bReadOwnPIN = FALSE;

#endif /* IWSC_SUPPORT */

				if (bReadOwnPIN) {
					pWscControl->WscPinCodeLen = pWscControl->WscEnrolleePinCodeLen;
					WscGetRegDataPIN(pAdapter, pWscControl->WscEnrolleePinCode, pWscControl);
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "(%d) WscEnrolleePinCode: %08u\n", bReadOwnPIN, pWscControl->WscEnrolleePinCode);
				} else
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscPinCode: %08u\n", pWscControl->WscPinCode);
			}

			/* If Message is from EAP, but UPnP Registrar is in progress now, ignore EAP Messages */
			if (!bUPnPMsg && pWscControl->WscUPnPNodeInfo.bUPnPInProgress) {
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "UPnP Registrar is working now, ignore EAP Messages.\n");
				goto out;
			} else
				WscEapEnrolleeAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
		}

#ifdef CONFIG_AP_SUPPORT
		else if (((pWscControl->WscConfMode & WSC_PROXY) != 0) && (bUPnPMsg) && (CurOpMode == AP_MODE)) {
			pWscControl->WscActionMode = WSC_PROXY;
			WscEapApProxyAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
		}

#endif /* CONFIG_AP_SUPPORT */
	} else if (MsgType == WSC_MSG_WSC_ACK) {
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscState: %d\n", pWscControl->WscState);

		if (((pWscControl->WscConfMode & WSC_REGISTRAR) != 0) &&
			pWscControl->WscState <= WSC_STATE_SENT_M2D) {
			if (WscCheckNonce(pAdapter, Elem, TRUE, pWscControl)) {
				if (pWscControl->M2DACKBalance > 0)
					pWscControl->M2DACKBalance--;

				pWscControl->WscState = WSC_STATE_INIT;
				pWscControl->EapMsgRunning = FALSE;
			}
		} else {
			if (((pWscControl->WscConfMode & WSC_ENROLLEE) != 0) &&
				WscCheckNonce(pAdapter, Elem, FALSE, pWscControl)) {
				pWscControl->WscActionMode = WSC_ENROLLEE;
				pWscControl->WscUseUPnP = bUPnPMsg ? 1 : 0;
				WscEapEnrolleeAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
			}

#ifdef CONFIG_AP_SUPPORT
			else if (((pWscControl->WscConfMode & WSC_PROXY) != 0) && (CurOpMode == AP_MODE)) {
				pWscControl->WscActionMode = WSC_PROXY;
				WscEapApProxyAction(pAdapter, Elem, MsgType, pEntry, pWscControl);
			}

#endif /* CONFIG_AP_SUPPORT */
		}
	} else if (MsgType == WSC_MSG_WSC_NACK) {
		BOOLEAN bReSetWscIE = FALSE;

		if (bUPnPMsg) {
			if ((pWscControl->WscState == WSC_STATE_WAIT_M8) &&
				(pWscControl->WscConfStatus == WSC_SCSTATE_CONFIGURED)) {
				/* Some external sta will send NACK when AP is configured. */
				/* bWscTrigger should be set FALSE, otherwise Proxy will send NACK to enrollee. */
				pWscControl->bWscTrigger = FALSE;
				pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
				bReSetWscIE = TRUE;
			}

			{
				int dataLen;
				UCHAR *pWscData;
				BOOLEAN bUPnPStatus = FALSE;

				os_alloc_mem(NULL, (UCHAR **)&pWscData, WSC_MAX_DATA_LEN);

				if (pWscData != NULL) {
					memset(pWscData, 0, WSC_MAX_DATA_LEN);
					dataLen = BuildMessageNACK(pAdapter, pWscControl, pWscData);
					bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x1F), WSC_OPCODE_UPNP_DATA,
													 WSC_UPNP_DATA_SUB_NORMAL, pWscData, dataLen,
													 Elem->TimeStamp.u.LowPart, Elem->TimeStamp.u.HighPart,
													 &pAdapter->CurrentAddress[0], CurOpMode);
					os_free_mem(pWscData);

					if (bUPnPStatus == FALSE)
						WscUPnPErrHandle(pAdapter, pWscControl, Elem->TimeStamp.u.LowPart);
				}

				if (pWscUPnPNodeInfo->bUPnPMsgTimerRunning == TRUE) {
					RTMPCancelTimer(&pWscUPnPNodeInfo->UPnPMsgTimer, &Cancelled);
					pWscUPnPNodeInfo->bUPnPMsgTimerRunning = FALSE;
				}

				pWscUPnPNodeInfo->bUPnPInProgress = FALSE;
				RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_FAIL, NULL, (pWscControl->EntryIfIdx & 0x1F), 0);
			}
		}

		if (!bUPnPMsg &&
			(WscCheckNonce(pAdapter, Elem, FALSE, pWscControl) || WscCheckNonce(pAdapter, Elem, TRUE, pWscControl))) {
			USHORT config_error = 0;

			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Receive NACK from WPS client.\n");
			WscGetConfigErrFromNack(pAdapter, Elem, &config_error);

			/*
			*If a PIN authentication or communication error occurs,
			*the Registrar MUST warn the user and MUST NOT automatically reuse the PIN.
			*Furthermore, if the Registrar detects this situation and prompts the user for a new PIN from the Enrollee device,
			*it MUST NOT accept the same PIN again without warning the user of a potential attack.
			*/
			if ((pWscControl->WscState >= WSC_STATE_WAIT_M5) && (config_error != WSC_ERROR_SETUP_LOCKED)) {
				pWscControl->WscRejectSamePinFromEnrollee = TRUE;
				WscDelListEntryByMAC(&pWscControl->WscPeerList, pEntry->Addr);

				if (pWscControl->WscState < WSC_STATE_WAIT_M8) {
					pWscControl->WscStatus = STATUS_WSC_FAIL;
					RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_FAIL, NULL, (pWscControl->EntryIfIdx & 0x1F), 0);
					bReSetWscIE = TRUE;
				}
			}

#ifdef CONFIG_AP_SUPPORT

			if ((pWscControl->WscState == WSC_STATE_OFF)
				&& (CurOpMode == AP_MODE)
				&& (pWscControl->RegData.SelfInfo.ConfigError != WSC_ERROR_NO_ERROR))
				bReSetWscIE = TRUE;

#endif /* CONFIG_AP_SUPPORT */

			if ((pWscControl->WscState == WSC_STATE_WAIT_M8) &&
				(pWscControl->WscConfStatus == WSC_SCSTATE_CONFIGURED)) {
				/* Some external sta will send NACK when AP is configured. */
				/* bWscTrigger should be set FALSE, otherwise Proxy will send NACK to enrollee. */
				pWscControl->bWscTrigger = FALSE;
				bReSetWscIE = TRUE;
				pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
				pWscControl->WscRejectSamePinFromEnrollee = FALSE;
				RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_SUCCESS, NULL, (pWscControl->EntryIfIdx & 0x1F), 0);
#ifdef P2P_SUPPORT

				/*RTMPCancelTimer(&pAdapter->P2pCfg.P2pWscTimer, &Cancelled);*/
				if (P2P_GO_ON(pAdapter) && pWscControl->EntryIfIdx != BSS0) {
					UCHAR	P2pIdx = P2P_NOT_FOUND;

					P2pIdx = P2pGroupTabSearch(pAdapter, MacAddr);

					if (P2pIdx != P2P_NOT_FOUND) {
						PRT_P2P_CLIENT_ENTRY pP2pEntry = &pAdapter->P2pTable.Client[P2pIdx];
						/* Update p2p Entry's state. */
						pP2pEntry->P2pClientState = P2PSTATE_CLIENT_WPS_DONE;
					}
				}

				/* default set extended listening to zero for each connection. If this is persistent, will set it. */
				pAdapter->P2pCfg.ExtListenInterval = 0;
				pAdapter->P2pCfg.ExtListenPeriod = 0;

				if (IS_PERSISTENT_ON(pAdapter) && (pEntry->bP2pClient == TRUE)) {
					UCHAR	P2pIdx = P2P_NOT_FOUND;

					P2pIdx = P2pGroupTabSearch(pAdapter, pEntry->Addr);

					if (IS_P2P_GO_ENTRY(pEntry))
						MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "pEntry is P2P GO.\n");
					else if (IS_P2P_CLI_ENTRY(pEntry))
						MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "pEntry is P2P CLIENT.\n");
					else
						MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "pEntry is P2P NONE.\n");

					if ((P2pIdx != P2P_NOT_FOUND) && (IS_P2P_GO_ENTRY(pEntry) || IS_P2P_CLI_ENTRY(pEntry))) {
						MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
								 "P2pWPSDone- Save to persistent entry. GrpCap= %x\n",
								  pAdapter->P2pTable.Client[P2pIdx].GroupCapability);
						MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
								 "3. P2pWPSDone-	Set Extended timing !!!!!!!\n");
						MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
								 "    ======== Profile :: Cnt = %d ========\n",
								  pWscControl->WscProfile.ProfileCnt);
						MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
								 "    SSID[%d] = %s.\n",
								  pWscControl->WscProfile.Profile[0].SSID.SsidLength, pWscControl->WscProfile.Profile[0].SSID.Ssid);
						MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
								 "    AuthType = %d.    EncrType = %d.\n",
								  pWscControl->WscProfile.Profile[0].AuthType, pWscControl->WscProfile.Profile[0].EncrType);
						MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
								 "    MAC = "MACSTR".\n",
								  MAC2STR(pWscControl->WscProfile.Profile[0].MacAddr));
						MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
								 "    KeyLen = %d.    KeyIdx = %d.\n",
								  pWscControl->WscProfile.Profile[0].KeyLength,
								  pWscControl->WscProfile.Profile[0].KeyIndex);
						MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
								 "    Key :: %02x %02x %02x %02x  %02x %02x %02x %02x\n",
								  pWscControl->WscProfile.Profile[0].Key[0],
								  pWscControl->WscProfile.Profile[0].Key[1],
								  pWscControl->WscProfile.Profile[0].Key[2],
								  pWscControl->WscProfile.Profile[0].Key[3],
								  pWscControl->WscProfile.Profile[0].Key[4],
								  pWscControl->WscProfile.Profile[0].Key[5],
								  pWscControl->WscProfile.Profile[0].Key[6],
								  pWscControl->WscProfile.Profile[0].Key[7]);
						MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
								 "             %02x %02x %02x %02x  %02x %02x %02x %02x\n",
								  pWscControl->WscProfile.Profile[0].Key[8],
								  pWscControl->WscProfile.Profile[0].Key[9],
								  pWscControl->WscProfile.Profile[0].Key[10],
								  pWscControl->WscProfile.Profile[0].Key[11],
								  pWscControl->WscProfile.Profile[0].Key[12],
								  pWscControl->WscProfile.Profile[0].Key[13],
								  pWscControl->WscProfile.Profile[0].Key[14],
								  pWscControl->WscProfile.Profile[0].Key[15]);
						P2pPerstTabInsert(pAdapter, pEntry->Addr, &pWscControl->WscProfile.Profile[0]);
						/* this is a persistent connection. */
						pAdapter->P2pCfg.ExtListenInterval = P2P_EXT_LISTEN_INTERVAL;
						pAdapter->P2pCfg.ExtListenPeriod = P2P_EXT_LISTEN_PERIOD;
					}
				}

#endif /* P2P_SUPPORT */
			}

#ifdef CONFIG_AP_SUPPORT
			else if ((CurOpMode == AP_MODE) &&
					 (pWscControl->WscState == WSC_STATE_WAIT_DONE) &&
					 (pWscControl->WscConfStatus == WSC_SCSTATE_CONFIGURED) &&
					 (IS_CIPHER_WEP(pAdapter->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseCipher))) {
				bReSetWscIE = TRUE;
				pWscControl->WscStatus = STATUS_WSC_FAIL;
			}

			if ((CurOpMode == AP_MODE) && (bReSetWscIE)  && (!pWscControl->WscPinCode)) {
				UCHAR apidx = pWscControl->EntryIfIdx & 0x1F;
				struct wifi_dev *wdev = &pAdapter->ApCfg.MBSSID[apidx].wdev;

				WscBuildBeaconIE(pAdapter, pWscControl->WscConfStatus, FALSE, 0, 0, (pWscControl->EntryIfIdx & 0x1F), NULL, 0, CurOpMode);
				WscBuildProbeRespIE(pAdapter, WSC_MSGTYPE_AP_WLAN_MGR, pWscControl->WscConfStatus, FALSE, 0, 0, pWscControl->EntryIfIdx, NULL, 0, CurOpMode);
				UpdateBeaconHandler(
					pAdapter,
					wdev,
					BCN_UPDATE_IE_CHG);

				if (pWscControl->Wsc2MinsTimerRunning) {
					RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
					pWscControl->Wsc2MinsTimerRunning = FALSE;
				}

				if (pWscControl->bWscTrigger)
					pWscControl->bWscTrigger = FALSE;
			}

#endif /* CONFIG_AP_SUPPORT */

			if ((CurOpMode == AP_MODE)
				|| ((ADHOC_ON(pAdapter)) && (pWscControl->WscConfMode == WSC_REGISTRAR))
			   ) {
				WscSendEapFail(pAdapter, pWscControl, TRUE);
				pWscControl->WscState = WSC_STATE_FAIL;
			}

#ifdef CONFIG_STA_SUPPORT
			else if ((CurOpMode == STA_MODE) && INFRA_ON(sta_cfg)) {
				WscEapActionDisabled(pAdapter, pWscControl);
				pWscControl->WscState = WSC_STATE_WAIT_DISCONN;
			}

#endif /* CONFIG_STA_SUPPORT */
			if (!pWscControl->WscPinCode) {
				RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
				pWscControl->EapolTimerRunning = FALSE;
				pWscControl->RegData.ReComputePke = 1;
			}
		}
	} else {
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Unsupported Msg Type (%02X)\n", MsgType);
		pWscControl->WscStatus = STATUS_WSC_FAIL;
		pWscControl->RegData.SelfInfo.ConfigError = WSC_ERROR_NO_ERROR;
		WscSendNACK(pAdapter, pEntry, pWscControl);
		RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_FAIL, NULL, BSS0, 0);
		goto out;
	}

	if (bUPnPMsg) {
		/* Messages from UPnP */
		if (pWscUPnPNodeInfo->bUPnPMsgTimerRunning)
			RTMPModTimer(&pWscUPnPNodeInfo->UPnPMsgTimer, WSC_UPNP_MSG_TIME_OUT);
	} else {
		if ((pWscControl->EapMsgRunning == TRUE) &&
			(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_HALT_IN_PROGRESS |
							 fRTMP_ADAPTER_NIC_NOT_EXIST))) {
			/* Messages from EAP */
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
			pWscControl->EapolTimerRunning = TRUE;
		}
	}

	if (bUPnPMsg && pWscControl->EapolTimerRunning) {
#ifdef CONFIG_AP_SUPPORT

		if ((pWscControl->WscActionMode == WSC_PROXY) && (CurOpMode == AP_MODE))
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		else
#endif /* CONFIG_AP_SUPPORT */
		{
			RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
			pWscControl->EapolTimerRunning = FALSE;
		}
	}

out:

	if (bUPnPMsg)
		pWscUPnPNodeInfo->bUPnPMsgTimerPending = FALSE;

	pWscControl->EapolTimerPending = FALSE;
	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscEAPAction\n");
}
#ifdef CONFIG_MAP_SUPPORT
void wsc_send_config_event_to_wapp(IN  PRTMP_ADAPTER pAdapter,
		IN PWSC_CTRL pWscControl, IN WSC_PROFILE *pWscProfile,
		IN MAC_TABLE_ENTRY *pEntry)
{
	UCHAR *msg;
	struct wifi_dev *wdev;
	struct wapp_event *event;
	int TotalLen = 0, i = 0;
	PSTA_ADMIN_CONFIG pApCliTab;
	UCHAR CurApIdx = (pWscControl->EntryIfIdx & 0x0F);
	PWSC_CREDENTIAL pCredential;
	MTWF_DBG(pAdapter, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"%s:SEND Event to WAPP for WSC profile currAPIndex %d\n", __func__, CurApIdx);

	if (CurApIdx >= MAX_APCLI_NUM)
		return;

	pApCliTab = &pAdapter->StaCfg[CurApIdx];
	TotalLen = sizeof(struct wapp_event);
	os_alloc_mem(NULL, (PUCHAR *)&msg, TotalLen);
	if (msg == NULL) {
		MTWF_DBG(pAdapter, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"failed to allocated memory\n");
		return;
	}
	NdisZeroMemory(msg, TotalLen);
	event = (struct wapp_event *)msg;
	event->event_id = WAPP_MAP_WSC_CONFIG;
	wdev = &pApCliTab->wdev;
	if (wdev && wdev->if_dev) {
		event->ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		event->len = 0; /* No data */
		for (i = 0; i < pWscProfile->ProfileCnt; i++) {
			pCredential = &pWscProfile->Profile[i];
			pCredential->DevPeerRole = pEntry->DevPeerRole;
		}

		RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM,
				OID_WAPP_EVENT, NULL, (PUCHAR)event, TotalLen);
	}
	os_free_mem((PUCHAR)msg);
}
#endif

/*
*	============================================================================
*	Enrollee			Enrollee			Enrollee
*	============================================================================
*/
VOID WscEapEnrolleeAction(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MLME_QUEUE_ELEM * Elem,
	IN  UCHAR MsgType,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL pWscControl)
{
	INT DataLen = 0, rv = 0, DH_Len = 0;
	UCHAR OpCode, bssIdx;
	PUCHAR WscData = NULL;
	BOOLEAN bUPnPMsg, bUPnPStatus = FALSE, Cancelled;
	WSC_UPNP_NODE_INFO *pWscUPnPInfo = &pWscControl->WscUPnPNodeInfo;
	UINT MaxWscDataLen = WSC_MAX_DATA_LEN;
	UCHAR CurOpMode = 0xFF;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;

#endif /* CONFIG_STA_SUPPORT */
	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			 "WscEapEnrolleeAction Enter!\n");
	bUPnPMsg = Elem->MsgType == WSC_EAPOL_UPNP_MSG ? TRUE : FALSE;
	OpCode = bUPnPMsg ? WSC_OPCODE_UPNP_MASK : 0;
	bssIdx = 0;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAdapter)
	CurOpMode = STA_MODE;

	if (CurOpMode == STA_MODE) {
		/* pStaCfg should only be retrieved in case of STA_MODE only, reason Assert was coming in AP_MODE */
		pStaCfg = GetStaCfgByWdev(pAdapter, Elem->wdev);
		ASSERT(pStaCfg);
		if (!pStaCfg) {
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "pStaCfg is NULL\n");
			return;
		}
	}
#ifdef P2P_SUPPORT

	if (Elem->OpMode != OPMODE_STA)
		CurOpMode = AP_MODE;

#endif /* P2P_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		/* Early check. */
		if ((pWscControl->WscActionMode != WSC_ENROLLEE) ||
			(pWscControl->WscUseUPnP && pEntry) ||
			((pWscControl->WscUseUPnP == 0) && (!pEntry))) {
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
					 "EarlyCheckFailed: pWscControl->WscActionMode=%d, Configured=%d, WscUseUPnP=%d, pEntry=%p!\n",
					  pWscControl->WscActionMode, pWscControl->WscConfStatus, pWscControl->WscUseUPnP, pEntry);
			goto Fail;
		}

		bssIdx = (pWscControl->EntryIfIdx & 0x0F);
	}

#endif /* CONFIG_AP_SUPPORT */
	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			 "MsgType=0x%x, WscState=%d, bUPnPMsg=%d!\n", MsgType, pWscControl->WscState, bUPnPMsg);

	if (bUPnPMsg) {
#ifdef CONFIG_AP_SUPPORT

		if ((MsgType == WSC_MSG_EAP_RSP_ID) && (CurOpMode == AP_MODE)) {
			/* let it pass */
		} else
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			if ((MsgType == WSC_MSG_EAP_REQ_START) &&  (CurOpMode == STA_MODE)) {
				/*let it pass */
			} else
#endif /* CONFIG_STA_SUPPORT */
				if (MsgType == WSC_MSG_M2 && pWscUPnPInfo->bUPnPInProgress == FALSE) {
#ifdef CONFIG_AP_SUPPORT

					if (CurOpMode == AP_MODE) {
						MAC_TABLE_ENTRY *tempEntry;

						tempEntry = MacTableLookup(pAdapter, &pWscControl->EntryAddr[0]);

						if (tempEntry) {
							if ((tempEntry->Receive_EapolStart_EapRspId & WSC_ENTRY_GET_EAP_RSP_ID) == WSC_ENTRY_GET_EAP_RSP_ID)
								goto Done;
						}

						/* else cannot find the pEntry, so we need to handle this msg. */
					}

#endif /* CONFIG_AP_SUPPORT */
					pWscUPnPInfo->bUPnPInProgress = TRUE;
					/* Set the WscState as "WSC_STATE_WAIT_RESP_ID" because UPnP start from this state. */
					/* pWscControl->WscState = WSC_STATE_WAIT_RESP_ID; */
					RTMPSetTimer(&pWscUPnPInfo->UPnPMsgTimer, WSC_UPNP_MSG_TIME_OUT);
					pWscUPnPInfo->bUPnPMsgTimerRunning = TRUE;
				} else {
					/* For other messages, we must make sure pWscUPnPInfo->bUPnPInProgress== TRUE */
					if (pWscUPnPInfo->bUPnPInProgress == FALSE)
						goto Done;
				}
	}

#ifdef WSC_V2_SUPPORT
	MaxWscDataLen = MaxWscDataLen + (UINT)pWscControl->WscV2Info.ExtraTlv.TlvLen;
#endif /* WSC_V2_SUPPORT */
	os_alloc_mem(NULL, (UCHAR **)&WscData, MaxWscDataLen);

	if (WscData == NULL) {
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 "WscData Allocate failed!\n");
		goto Fail;
	}

	NdisZeroMemory(WscData, MaxWscDataLen);

	switch (MsgType) {
	case WSC_MSG_EAP_RSP_ID:
	case WSC_MSG_EAP_REQ_START:
		if (MsgType == WSC_MSG_EAP_RSP_ID)
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
					 "WscEapEnrolleeAction : Rx Identity(ReComputePke=%d)\n", pWscControl->RegData.ReComputePke);

		if (MsgType == WSC_MSG_EAP_REQ_START)
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
					 "WscEapEnrolleeAction : Rx Wsc_Start(ReComputePke=%d)\n", pWscControl->RegData.ReComputePke);

		{
#ifdef CONFIG_AP_SUPPORT
			/*
			*	We don't need to consider P2P case.
			*/
			IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter) {
				if ((pWscControl->bWscAutoTriggerDisable == TRUE) &&
					(pWscControl->bWscTrigger == FALSE)) {
					if (bUPnPMsg == TRUE) {
						MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
								 "WscAutoTrigger is disabled.\n");
						WscUPnPErrHandle(pAdapter, pWscControl, Elem->TimeStamp.u.LowPart);
						os_free_mem(WscData);
						return;
					} else if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
						MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
								 "WscAutoTrigger is disabled! Send EapFail to STA.\n");
						WscSendEapFail(pAdapter, pWscControl, TRUE);
						os_free_mem(WscData);
						return;
					}
				}
			}
#endif /* CONFIG_AP_SUPPORT */

			if (pWscControl->RegData.ReComputePke == 1) {
				INT idx;

				DH_Len = sizeof(pWscControl->RegData.Pke);

				/* Enrollee 192 random bytes for DH key generation */
				for (idx = 0; idx < 192; idx++)
					pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAdapter);

				NdisZeroMemory(pWscControl->RegData.Pke, sizeof(pWscControl->RegData.Pke));
				RT_DH_PublicKey_Generate(
					WPS_DH_G_VALUE, sizeof(WPS_DH_G_VALUE),
					WPS_DH_P_VALUE, sizeof(WPS_DH_P_VALUE),
					pWscControl->RegData.EnrolleeRandom, sizeof(pWscControl->RegData.EnrolleeRandom),
					pWscControl->RegData.Pke, (UINT *) &DH_Len);

				/* Need to prefix zero padding */
				if ((DH_Len != sizeof(pWscControl->RegData.Pke)) &&
					(DH_Len < sizeof(pWscControl->RegData.Pke))) {
					UCHAR TempKey[192];
					INT DiffCnt;

					DiffCnt = sizeof(pWscControl->RegData.Pke) - DH_Len;
					NdisFillMemory(&TempKey, DiffCnt, 0);
					NdisCopyMemory(&TempKey[DiffCnt], pWscControl->RegData.Pke, DH_Len);
					NdisCopyMemory(pWscControl->RegData.Pke, TempKey, sizeof(TempKey));
					DH_Len += DiffCnt;
					MTWF_DBG(pAdapter, DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s: Do zero padding!\n", __func__);
				}

				pWscControl->RegData.ReComputePke = 0;
			}
		}

		OpCode |= WSC_OPCODE_MSG;
		DataLen = BuildMessageM1(pAdapter, pWscControl, WscData);

		if (!bUPnPMsg) {
			pWscControl->EapMsgRunning = TRUE;
			pWscControl->WscStatus = STATUS_WSC_EAP_M1_SENT;
		} else
			/* Sometime out-of-band registrars (ex: Vista) get M1 for collecting information of device. */
			pWscControl->WscStatus = STATUS_WSC_IDLE;

		/* Change the state to next one */
		if (pWscControl->WscState < WSC_STATE_SENT_M1)
			pWscControl->WscState = WSC_STATE_SENT_M1;

		RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M1, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
		break;

	case WSC_MSG_M2:
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "WscEapEnrolleeAction : Rx M2\n");

		/* Receive M2, if we are at WSC_STATE_WAIT_M2 start, process it immediately */
		if (pWscControl->WscState == WSC_STATE_SENT_M1 ||
			pWscControl->WscState == WSC_STATE_RX_M2D) {
			/* Process M2 */
			pWscControl->WscStatus = STATUS_WSC_EAP_M2_RECEIVED;
			NdisMoveMemory(pWscControl->RegData.PeerInfo.MacAddr, pWscControl->EntryAddr, 6);
			rv = ProcessMessageM2(pAdapter, pWscControl, Elem->Msg, Elem->MsgLen, (pWscControl->EntryIfIdx & 0x0F), &pWscControl->RegData);

			if (rv)
				goto Fail;
			else {
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

				if ((CurOpMode == AP_MODE) && pWscControl->bSetupLock) {
					rv = WSC_ERROR_SETUP_LOCKED;
					goto Fail;
				}

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
				{
					OpCode |= WSC_OPCODE_MSG;
					DataLen = BuildMessageM3(pAdapter, pWscControl, WscData);
					pWscControl->WscStatus = STATUS_WSC_EAP_M3_SENT;
					/* Change the state to next one */
					pWscControl->WscState = WSC_STATE_WAIT_M4;
					RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M3, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
				}
			}
		}

		break;

	case WSC_MSG_M2D:
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "WscEapEnrolleeAction : Rx M2D\n");

		/* Receive M2D, if we are at WSC_STATE_WAIT_M2 start, process it immediately */
		if (pWscControl->WscState == WSC_STATE_SENT_M1 ||
			pWscControl->WscState == WSC_STATE_RX_M2D) {
			BOOLEAN bReplyNack = FALSE;

			rv = ProcessMessageM2D(pAdapter, Elem->Msg, Elem->MsgLen, &pWscControl->RegData);

			if (rv)
				goto Fail;

			pWscControl->WscStatus = STATUS_WSC_EAP_M2D_RECEIVED;

			if (CurOpMode == AP_MODE) {
				bReplyNack = TRUE;
#ifdef APCLI_SUPPORT

				if (pEntry && !IS_ENTRY_PEER_AP(pEntry))
					bReplyNack = TRUE;
				else
					bReplyNack = FALSE;

#endif
#ifdef P2P_SUPPORT

				if (P2P_INF_ON(pAdapter)) {
					if (P2P_GO_ON(pAdapter))
						bReplyNack = TRUE;
					else
						bReplyNack = FALSE;
				}

#endif
			}

			if (bReplyNack) {
				/* For VISTA SP1 internal registrar test */
				OpCode |= WSC_OPCODE_NACK;
				DataLen = BuildMessageNACK(pAdapter, pWscControl, WscData);
				RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_NACK, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
			} else {
				/* When external registrar is Marvell station, */
				/* wps station sends NACK may confuse or reset Marvell wps state machine. */
				OpCode |= WSC_OPCODE_ACK;
				DataLen = BuildMessageACK(pAdapter, pWscControl, WscData);
				RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_ACK, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
			}

			/* Change the state to next one */
			pWscControl->WscState = WSC_STATE_RX_M2D;
		}

		break;

	case WSC_MSG_M4:
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "WscEapEnrolleeAction : Rx M4\n");

		/* Receive M4, if we are at WSC_STATE_WAIT_M4 start, process it immediately */
		if (pWscControl->WscState == WSC_STATE_WAIT_M4) {
			/* Process M4 */
			pWscControl->WscStatus = STATUS_WSC_EAP_M4_RECEIVED;
			rv = ProcessMessageM4(pAdapter, pWscControl, Elem->Msg, Elem->MsgLen, &pWscControl->RegData);

			if ((rv)) {
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

				if (CurOpMode == AP_MODE)
					WscCheckPinAttackCount(pAdapter, pWscControl);

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
				goto Fail;
			} else {
				OpCode |= WSC_OPCODE_MSG;
				DataLen = BuildMessageM5(pAdapter, pWscControl, WscData);
				pWscControl->WscStatus = STATUS_WSC_EAP_M5_SENT;
				/* Change the state to next one */
				pWscControl->WscState = WSC_STATE_WAIT_M6;
				RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M5, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
			}
		}

		break;

	case WSC_MSG_M6:
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "WscEapEnrolleeAction : Rx M6\n");

		/* Receive M6, if we are at WSC_STATE_WAIT_M6 start, process it immediately */
		if (pWscControl->WscState == WSC_STATE_WAIT_M6) {
			/* Process M6 */
			pWscControl->WscStatus = STATUS_WSC_EAP_M6_RECEIVED;
			rv = ProcessMessageM6(pAdapter, pWscControl, Elem->Msg, Elem->MsgLen, &pWscControl->RegData);

			if (rv) {
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

				if (CurOpMode == AP_MODE)
					WscCheckPinAttackCount(pAdapter, pWscControl);

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
				goto Fail;
			} else {
				OpCode |= WSC_OPCODE_MSG;
				DataLen = BuildMessageM7(pAdapter, pWscControl, WscData);
				pWscControl->WscStatus = STATUS_WSC_EAP_M7_SENT;
				/* Change the state to next one */
				pWscControl->WscState = WSC_STATE_WAIT_M8;
				RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M7, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);

				/*
				*	Complete WPS with this STA. Delete it from WscPeerList for others STA to do WSC with AP
				*/
				if (pEntry) {
					RTMP_SEM_LOCK(&pWscControl->WscPeerListSemLock);
					WscDelListEntryByMAC(&pWscControl->WscPeerList, pEntry->Addr);
					RTMP_SEM_UNLOCK(&pWscControl->WscPeerListSemLock);
				}
			}
		}

		break;

	case WSC_MSG_M8:
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "WscEapEnrolleeAction : Rx M8\n");

		/* Receive M8, if we are at WSC_STATE_WAIT_M6 start, process it immediately */
		if (pWscControl->WscState == WSC_STATE_WAIT_M8) {
			/* Process M8 */
			pWscControl->WscStatus = STATUS_WSC_EAP_M8_RECEIVED;
			rv = ProcessMessageM8(pAdapter, Elem->Msg, Elem->MsgLen, pWscControl);

			if (rv)
				goto Fail;
			else {
				OpCode |= WSC_OPCODE_DONE;
				DataLen = BuildMessageDONE(pAdapter, pWscControl, WscData);
#ifdef CONFIG_AP_SUPPORT

				if (CurOpMode == AP_MODE) {
					/* Change the state to next one */
#ifdef APCLI_SUPPORT
					/* Ap Client only supports Inband(EAP)-Enrollee. */
					if (!bUPnPMsg && pEntry && IS_ENTRY_PEER_AP(pEntry)) {
						pWscControl->WscState = WSC_STATE_WAIT_EAPFAIL;
						RTMP_OS_INIT_COMPLETION(&pWscControl->WscEAPHandshakeCompleted);
					}
					else
#endif /* APCLI_SUPPORT */
						pWscControl->WscState = WSC_STATE_WAIT_ACK;

				}

#endif /* CONFIG_AP_SUPPORT */
#ifdef VENDOR_FEATURE6_SUPPORT
				WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x0F), WSC_OPCODE_UPNP_CTRL, WSC_UPNP_DATA_SUB_M8, &pAdapter->ApCfg.MBSSID[pWscControl->EntryIfIdx].wdev.bssid[0], MAC_ADDR_LEN, 0, 0, &pAdapter->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x1F].wdev.bssid[0], AP_MODE);
#endif /* VENDOR_FEATURE6_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

				if (CurOpMode == STA_MODE) {
					pWscControl->WscState = WSC_STATE_WAIT_EAPFAIL;
					pWscControl->WscStatus = STATUS_WSC_EAP_RSP_DONE_SENT;
				}

#endif /* CONFIG_STA_SUPPORT */
				RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_DONE, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
			}
		}

		break;
#ifdef CONFIG_AP_SUPPORT

	case WSC_MSG_WSC_ACK:
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "WscEapEnrolleeAction : Rx ACK\n");

		/* Receive ACK */
		if (pWscControl->WscState == WSC_STATE_WAIT_ACK) {
			/* Process ACK */
			pWscControl->WscStatus = STATUS_WSC_EAP_RAP_RSP_ACK;
			/* Send out EAP-Fail */
			WscSendEapFail(pAdapter, pWscControl, FALSE);
			pWscControl->WscState = WSC_STATE_CONFIGURED;
			pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
#ifdef P2P_SUPPORT
			pAdapter->P2pCfg.WscState = WSC_STATE_CONFIGURED;
#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
			P2pSendWirelessEvent(pAdapter, RT_P2P_WPS_COMPLETED, NULL, NULL);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
#endif /* P2P_SUPPORT */
		}

		break;
#endif /* CONFIG_AP_SUPPORT */

	default:
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "WscEapEnrolleeAction : Unsupported Msg Type\n");
		break;
	}

	if (bUPnPMsg) {
		if ((MsgType == WSC_MSG_M8) && (pWscControl->WscState == WSC_STATE_WAIT_ACK)) {
			pWscControl->EapMsgRunning = FALSE;
			pWscControl->WscState = WSC_STATE_CONFIGURED;
			pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
#ifdef P2P_SUPPORT
			pAdapter->P2pCfg.WscState = WSC_STATE_CONFIGURED;
#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
			P2pSendWirelessEvent(pAdapter, RT_P2P_WPS_COMPLETED, NULL, NULL);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
#endif /* P2P_SUPPORT */

			if (pWscUPnPInfo->bUPnPMsgTimerRunning == TRUE) {
				RTMPCancelTimer(&pWscUPnPInfo->UPnPMsgTimer, &Cancelled);
				pWscUPnPInfo->bUPnPMsgTimerRunning = FALSE;
			}

			pWscUPnPInfo->bUPnPInProgress = FALSE;
			pWscUPnPInfo->registrarID = 0;
		}
	} else {
		if (((MsgType == WSC_MSG_WSC_ACK) && (pWscControl->WscState == WSC_STATE_CONFIGURED)) ||
			((MsgType == WSC_MSG_M8) && (pWscControl->WscState == WSC_STATE_WAIT_ACK))) {
			RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
			pWscControl->EapolTimerRunning = FALSE;
			pWscControl->EapMsgRunning = FALSE;
			/*NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN); */
		}
	}

	if (OpCode > WSC_OPCODE_UPNP_MASK)
		bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x0F), WSC_OPCODE_UPNP_DATA,
										 WSC_UPNP_DATA_SUB_NORMAL, WscData, DataLen,
										 Elem->TimeStamp.u.LowPart, Elem->TimeStamp.u.HighPart,
										 &pAdapter->CurrentAddress[0], CurOpMode);
	else if (OpCode > 0 && OpCode < WSC_OPCODE_UPNP_MASK) {
		if (pWscControl->WscState != WSC_STATE_CONFIGURED) {
#ifdef WSC_V2_SUPPORT
			pWscControl->WscTxBufLen = 0;
			pWscControl->pWscCurBufIdx = NULL;
			pWscControl->bWscLastOne = TRUE;

			if (pWscControl->bWscFragment && (DataLen > pWscControl->WscFragSize)) {
				ASSERT(DataLen < MAX_MGMT_PKT_LEN);
				NdisMoveMemory(pWscControl->pWscTxBuf, WscData, DataLen);
				pWscControl->WscTxBufLen = DataLen;
				NdisZeroMemory(WscData, DataLen);
				pWscControl->bWscLastOne = FALSE;
				pWscControl->bWscFirstOne = TRUE;
				NdisMoveMemory(WscData, pWscControl->pWscTxBuf, pWscControl->WscFragSize);
				DataLen = pWscControl->WscFragSize;
				pWscControl->WscTxBufLen -= pWscControl->WscFragSize;
				pWscControl->pWscCurBufIdx = (pWscControl->pWscTxBuf + pWscControl->WscFragSize);
			}

#endif /* WSC_V2_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE) {
				if (pEntry && IS_ENTRY_PEER_AP(pEntry))
					WscSendMessage(pAdapter, OpCode, WscData, DataLen, pWscControl, AP_CLIENT_MODE, EAP_CODE_RSP);
				else
					WscSendMessage(pAdapter, OpCode, WscData, DataLen, pWscControl, AP_MODE, EAP_CODE_REQ);
			}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

			if (CurOpMode == STA_MODE)
				WscSendMessage(pAdapter, OpCode, WscData, DataLen, pWscControl, STA_MODE, EAP_CODE_RSP);

#endif /* CONFIG_STA_SUPPORT */
		}
	} else
		bUPnPStatus = TRUE;

Fail:
	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapEnrolleeAction : rv = %d\n", rv);

	if (rv) {
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

		if ((CurOpMode == AP_MODE) && pWscControl->bSetupLock)
			rv = WSC_ERROR_SETUP_LOCKED;

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

		if (rv <= WSC_ERROR_DEV_PWD_AUTH_FAIL)
			pWscControl->RegData.SelfInfo.ConfigError = rv;
		else if ((rv == WSC_ERROR_HASH_FAIL) || (rv == WSC_ERROR_HMAC_FAIL))
			pWscControl->RegData.SelfInfo.ConfigError = WSC_ERROR_DECRYPT_CRC_FAIL;

		switch (rv) {
		case WSC_ERROR_DEV_PWD_AUTH_FAIL:
			pWscControl->WscStatus = STATUS_WSC_ERROR_DEV_PWD_AUTH_FAIL;
			break;

		default:
			pWscControl->WscStatus = STATUS_WSC_FAIL;
			break;
		}

			RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_FAIL, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);

		if (bUPnPMsg) {
			if (pWscUPnPInfo->bUPnPMsgTimerRunning == TRUE) {
				RTMPCancelTimer(&pWscUPnPInfo->UPnPMsgTimer, &Cancelled);
				pWscUPnPInfo->bUPnPMsgTimerRunning = FALSE;
			}

			pWscUPnPInfo->bUPnPInProgress = FALSE;
		} else
			WscSendNACK(pAdapter, pEntry, pWscControl);

#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == AP_MODE) {
#ifdef P2P_SUPPORT

			if (P2P_CLI_ON(pAdapter))
				pWscControl->WscState = WSC_STATE_WAIT_DISCONN;
			else
#endif /* P2P_SUPPORT */
				pWscControl->WscState = WSC_STATE_OFF;
			if (pEntry && IS_ENTRY_PEER_AP(pEntry))
				pWscControl->WscState = WSC_STATE_WAIT_DISCONN;
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

		if (CurOpMode == STA_MODE) {
			pWscControl->WscState = WSC_STATE_WAIT_DISCONN;

			if (pWscControl->WscSsid.SsidLength) {
				pStaCfg->MlmeAux.AutoReconnectSsidLen = pWscControl->WscSsid.SsidLength;
				NdisZeroMemory(&pStaCfg->MlmeAux.AutoReconnectSsid[0], MAX_LEN_OF_SSID);
				NdisMoveMemory(&pStaCfg->MlmeAux.AutoReconnectSsid[0],
							   &pWscControl->WscSsid.Ssid[0],
							   pWscControl->WscSsid.SsidLength);
			} else
				pStaCfg->MlmeAux.AutoReconnectSsidLen = 0;
		}

#endif /* CONFIG_STA_SUPPORT */
		/*NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN); */
		/*pWscControl->WscMode = 1; */
		bUPnPStatus = FALSE;
	}

Done:

	if (WscData)
		os_free_mem(WscData);

	if (bUPnPMsg && (bUPnPStatus == FALSE))
		WscUPnPErrHandle(pAdapter, pWscControl, Elem->TimeStamp.u.LowPart);

	rv = 0;
#ifdef CONFIG_AP_SUPPORT

	if  (CurOpMode == AP_MODE) {
		if (((bUPnPMsg || (pEntry && IS_ENTRY_CLIENT(pEntry)))
			 && (pWscControl->WscState == WSC_STATE_CONFIGURED || pWscControl->WscState == WSC_STATE_WAIT_ACK))
#ifdef APCLI_SUPPORT
			|| ((!bUPnPMsg && pEntry && IS_ENTRY_PEER_AP(pEntry)) && (pWscControl->WscState == WSC_STATE_WAIT_EAPFAIL || pWscControl->WscState == WSC_STATE_CONFIGURED))
#endif /* APCLI_SUPPORT */
		   )
			rv = 1;
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		if ((pWscControl->WscState == WSC_STATE_WAIT_EAPFAIL) ||
			(pWscControl->WscState == WSC_STATE_CONFIGURED))
			rv = 1;
	}

#endif /* CONFIG_STA_SUPPORT */

	if (rv == 1) {
#ifdef WSC_LED_SUPPORT
		UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
		pWscControl->bWscTrigger = FALSE;
		pWscControl->RegData.ReComputePke = 1;
		RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);

		if (pWscControl->Wsc2MinsTimerRunning) {
			pWscControl->Wsc2MinsTimerRunning = FALSE;
			RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
		}

#ifdef IWSC_SUPPORT

		if ((pAdapter->OpMode == OPMODE_STA) && (pAdapter->StaCfg[0].BssType == BSS_ADHOC)) {
			pAdapter->StaCfg[0].IWscInfo.bReStart = TRUE;

			if (pAdapter->StaCfg[0].IWscInfo.bIWscT1TimerRunning) {
				pAdapter->StaCfg[0].IWscInfo.bIWscT1TimerRunning = FALSE;
				RTMPCancelTimer(&pAdapter->StaCfg[0].IWscInfo.IWscT1Timer, &Cancelled);
			}

			pAdapter->StaCfg[0].IWscInfo.bIWscDevQueryReqTimerRunning = TRUE;
		}

#endif /* IWSC_SUPPORT */

		if ((pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED)
#ifdef CONFIG_AP_SUPPORT
			|| (pWscControl->bWCNTest == TRUE)
#ifdef WSC_V2_SUPPORT
			|| (pWscControl->WscV2Info.bEnableWpsV2 && ((CurOpMode == AP_MODE) && !pWscControl->bSetupLock))
#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
		   ) {
			pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
			if (!(CurOpMode == AP_MODE && pEntry && IS_ENTRY_PEER_AP(pEntry)))
				pWscControl->WscConfStatus = WSC_SCSTATE_CONFIGURED;
			pWscControl->WscMode = 1;
			RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_SUCCESS, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
#ifdef WIDI_SUPPORT
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapEnrolleeAction : WPS Successful; ifIdx = %d\n", pWscControl->EntryIfIdx);
			WidiUpdateStateToDaemon(pAdapter, MIN_NET_DEVICE_FOR_MBSSID, WIDI_MSG_TYPE_ASSOC_STATUS, pEntry->Addr, NULL, 0, WIDI_P2P_WPS_SUCCESS);
#endif /* WIDI_SUPPORT */
#ifdef P2P_SUPPORT

			/*RTMPCancelTimer(&pAdapter->P2pCfg.P2pWscTimer, &Cancelled);*/
			if (P2P_GO_ON(pAdapter) && (pWscControl->EntryIfIdx != BSS0) && pEntry) {
				UCHAR	P2pIdx = P2P_NOT_FOUND;

				P2pIdx = P2pGroupTabSearch(pAdapter, pEntry->Addr);

				if (P2pIdx != P2P_NOT_FOUND) {
					PRT_P2P_CLIENT_ENTRY pP2pEntry = &pAdapter->P2pTable.Client[P2pIdx];
					/* Update p2p Entry's state. */
					pP2pEntry->P2pClientState = P2PSTATE_CLIENT_WPS_DONE;
				}
			}

			/* default set extended listening to zero for each connection. If this is persistent, will set it. */
			pAdapter->P2pCfg.ExtListenInterval = 0;
			pAdapter->P2pCfg.ExtListenPeriod = 0;

			if (IS_PERSISTENT_ON(pAdapter) && pEntry && (pEntry->bP2pClient == TRUE)) {
				UCHAR	P2pIdx = P2P_NOT_FOUND;

				P2pIdx = P2pGroupTabSearch(pAdapter, pEntry->Addr);

				if (IS_P2P_GO_ENTRY(pEntry))
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "pEntry is P2P GO.\n");
				else if (IS_P2P_CLI_ENTRY(pEntry))
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "pEntry is P2P CLIENT.\n");
				else
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "pEntry is P2P NONE.\n");

				if ((P2pIdx != P2P_NOT_FOUND) && (IS_P2P_GO_ENTRY(pEntry) || IS_P2P_CLI_ENTRY(pEntry))) {
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "P2pWPSDone- Save to persistent entry. GrpCap= %x\n",
							  pAdapter->P2pTable.Client[P2pIdx].GroupCapability);
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "2. P2pWPSDone-	Set Extended timing !!!!!!!\n");
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "    ======== Profile :: Cnt = %d ========\n",
							  pWscControl->WscProfile.ProfileCnt);
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "    SSID[%d] = %s.\n",
							  pWscControl->WscProfile.Profile[0].SSID.SsidLength,
							  pWscControl->WscProfile.Profile[0].SSID.Ssid);
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "    AuthType = %d.	 EncrType = %d.\n",
							  pWscControl->WscProfile.Profile[0].AuthType,
							  pWscControl->WscProfile.Profile[0].EncrType);
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "    MAC = "MACSTR".\n",
							  MAC2STR(pWscControl->WscProfile.Profile[0].MacAddr));
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "    KeyLen = %d.    KeyIdx = %d.\n",
							  pWscControl->WscProfile.Profile[0].KeyLength, pWscControl->WscProfile.Profile[0].KeyIndex);
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "    Key :: %02x %02x %02x %02x  %02x %02x %02x %02x\n",
							  pWscControl->WscProfile.Profile[0].Key[0],
							  pWscControl->WscProfile.Profile[0].Key[1],
							  pWscControl->WscProfile.Profile[0].Key[2],
							  pWscControl->WscProfile.Profile[0].Key[3],
							  pWscControl->WscProfile.Profile[0].Key[4],
							  pWscControl->WscProfile.Profile[0].Key[5],
							  pWscControl->WscProfile.Profile[0].Key[6],
							 n pWscControl->WscProfile.Profile[0].Key[7]);
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "			%02x %02x %02x %02x  %02x %02x %02x %02x\n",
							  pWscControl->WscProfile.Profile[0].Key[8],
							  pWscControl->WscProfile.Profile[0].Key[9],
							  pWscControl->WscProfile.Profile[0].Key[10],
							  pWscControl->WscProfile.Profile[0].Key[11],
							  pWscControl->WscProfile.Profile[0].Key[12],
							  pWscControl->WscProfile.Profile[0].Key[13],
							  pWscControl->WscProfile.Profile[0].Key[14],
							  pWscControl->WscProfile.Profile[0].Key[15]);
					P2pPerstTabInsert(pAdapter, pEntry->Addr, &pWscControl->WscProfile.Profile[0]);
					/* this is a persistent connection. */
					pAdapter->P2pCfg.ExtListenInterval = P2P_EXT_LISTEN_INTERVAL;
					pAdapter->P2pCfg.ExtListenPeriod = P2P_EXT_LISTEN_PERIOD;
				}
			}

#endif /* P2P_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE) {
				pWscControl->RegData.SelfInfo.ScState = pWscControl->WscConfStatus;
#ifdef APCLI_SUPPORT

				if (!bUPnPMsg && pEntry && IS_ENTRY_PEER_AP(pEntry)) {
					WscWriteConfToApCliCfg(pAdapter, pWscControl, &pWscControl->WscProfile.Profile[0], TRUE);
#ifdef CONFIG_MAP_SUPPORT
					wsc_send_config_event_to_wapp(pAdapter, pWscControl,
						&pWscControl->WscProfile, pEntry);
#endif /* CONFIG_MAP_SUPPORT */
					RtmpOsTaskWakeUp(&(pAdapter->wscTask));
				} else
#endif /* APCLI_SUPPORT */
				{
					RTMPSetTimer(&pWscControl->WscUpdatePortCfgTimer, 1000);
					pWscControl->WscUpdatePortCfgTimerRunning = TRUE;
				}

				if (bUPnPMsg || (pEntry && IS_ENTRY_CLIENT(pEntry))) {
					UCHAR apidx = pWscControl->EntryIfIdx & 0x1F;
					struct wifi_dev *wdev = &pAdapter->ApCfg.MBSSID[apidx].wdev;

					WscBuildBeaconIE(pAdapter, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, (pWscControl->EntryIfIdx & 0x1F), NULL, 0, CurOpMode);
					WscBuildProbeRespIE(pAdapter, WSC_MSGTYPE_AP_WLAN_MGR, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, (pWscControl->EntryIfIdx & 0x1F), NULL, 0, CurOpMode);
					UpdateBeaconHandler(
						pAdapter,
						wdev,
						BCN_UPDATE_IE_CHG);
				}
			}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

			if (CurOpMode == STA_MODE) {
				pWscControl->WscConfMode = WSC_DISABLE;

				if (bUPnPMsg) {
					pWscControl->WscState = WSC_STATE_OFF;
					WscLinkDown(pAdapter, pWscControl->wdev);
				}

				if (pWscControl->WscDriverAutoConnect != 0) {
					pAdapter->StaCfg[0].bAutoConnectByBssid = TRUE;
					pWscControl->WscProfile.ApplyProfileIdx = 0;  /* add by johnli, fix WPS test plan 5.1.1 */
					{
						WscWriteConfToPortCfg(pAdapter, pWscControl, &pWscControl->WscProfile.Profile[0], TRUE);
						pAdapter->WriteWscCfgToDatFile = (pWscControl->EntryIfIdx & 0x0F);
						RtmpOsTaskWakeUp(&(pAdapter->wscTask));
					}

#ifdef IWSC_SUPPORT

					if ((pAdapter->StaCfg[0].BssType == BSS_ADHOC) &&
						(pAdapter->StaCfg[0].IWscInfo.bIWscDevQueryReqTimerRunning == FALSE)) {
						pAdapter->StaCfg[0].IWscInfo.bIWscDevQueryReqTimerRunning = TRUE;
						RTMPSetTimer(&pAdapter->StaCfg[0].IWscInfo.IWscDevQueryTimer, 200);
					}

#endif /* IWSC_SUPPORT */
				}
			}

#endif /* CONFIG_STA_SUPPORT */
		}

#ifdef WSC_LED_SUPPORT
		/* The protocol is finished. */
		WPSLEDStatus = LED_WPS_SUCCESS;
		RTMPSetLED(pAdapter, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
#endif /* WSC_LED_SUPPORT */
	}
}

#ifdef CONFIG_AP_SUPPORT
/*
*	============================================================================
*	Proxy			Proxy			Proxy
*	============================================================================
*/
VOID WscEapApProxyAction(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MLME_QUEUE_ELEM * Elem,
	IN  UCHAR MsgType,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL pWscControl)
{
	PUCHAR  WscData = NULL;
	BOOLEAN sendToUPnP = FALSE, bUPnPStatus = FALSE, Cancelled;
	int reqID = 0;
	WSC_UPNP_NODE_INFO *pWscUPnPInfo = &pWscControl->WscUPnPNodeInfo;
	UINT MaxWscDataLen = WSC_MAX_DATA_LEN;

	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapApProxyAction Enter!\n");

	if (Elem->MsgType == WSC_EAPOL_UPNP_MSG) {
		reqID = Elem->TimeStamp.u.LowPart;

		if (reqID > 0)
			sendToUPnP = TRUE;
	}

	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapApProxyAction():pEntry=%p, ElemMsgType=%ld, MsgType=%d!\n", pEntry, Elem->MsgType, MsgType);

	if ((pWscControl->WscActionMode != WSC_PROXY) ||
		((Elem->MsgType == WSC_EAPOL_PACKET_MSG) && (pEntry == NULL))) {
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
			"EarlyCheckFailed: gWscActionMode=%d, pEntry=%p!\n", pWscControl->WscActionMode, pEntry);
		goto Fail;
	}

#ifdef WSC_V2_SUPPORT
	MaxWscDataLen = MaxWscDataLen + (UINT)pWscControl->WscV2Info.ExtraTlv.TlvLen;
#endif /* WSC_V2_SUPPORT */
	os_alloc_mem(NULL, (UCHAR **)&WscData, MaxWscDataLen);

	if (WscData == NULL) {
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "WscData Allocate failed!\n");
		goto Fail;
	}

	NdisZeroMemory(WscData, MaxWscDataLen);

	/* Base on state doing the Msg, State change diagram */
	if (Elem->MsgType == WSC_EAPOL_UPNP_MSG) {
		/* WSC message send from UPnP. */
		switch (MsgType) {
		case WSC_MSG_M2:
		case WSC_MSG_M4:
		case WSC_MSG_M6:
		case WSC_MSG_M8:
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapApProxyAction: Rx WscMsg(%d) from UPnP, eventID=0x%x!\n", MsgType, reqID);
			WscSendMessage(pAdapter, WSC_OPCODE_MSG, Elem->Msg, Elem->MsgLen, pWscControl, AP_MODE, EAP_CODE_REQ);

			/*Notify the UPnP daemon which remote registar is negotiating with enrollee. */
			if (MsgType == WSC_MSG_M2) {
				pWscUPnPInfo->registrarID = Elem->TimeStamp.u.HighPart;
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s():registrarID=0x%x!\n", __func__, pWscUPnPInfo->registrarID);
				bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x1F),
												 WSC_OPCODE_UPNP_MGMT, WSC_UPNP_MGMT_SUB_REG_SELECT,
												 (PUCHAR)(&pWscUPnPInfo->registrarID), sizeof(UINT), 0, 0, NULL, AP_MODE);

				/*Reset the UPnP timer and status. */
				if (pWscControl->bM2DTimerRunning == TRUE) {
					RTMPCancelTimer(&pWscControl->M2DTimer, &Cancelled);
					pWscControl->bM2DTimerRunning = FALSE;
				}

				pWscControl->M2DACKBalance = 0;
				pWscUPnPInfo->registrarID = 0;
			}

			if (MsgType == WSC_MSG_M8) {
				UCHAR apidx = pWscControl->EntryIfIdx & 0x1F;
				struct wifi_dev *wdev = &pAdapter->ApCfg.MBSSID[apidx].wdev;

				WscBuildBeaconIE(pAdapter, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, (pWscControl->EntryIfIdx & 0x1F), NULL, 0, AP_MODE);
				WscBuildProbeRespIE(pAdapter, WSC_MSGTYPE_AP_WLAN_MGR, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, pWscControl->EntryIfIdx, NULL, 0, AP_MODE);
				UpdateBeaconHandler(
					pAdapter,
					wdev,
					BCN_UPDATE_IE_CHG);
			}

			break;

		case WSC_MSG_M2D:
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapApProxyAction: Rx WscMsg M2D(%d) from UPnP, eventID=0x%x!\n", MsgType, reqID);

			/*If it's send by UPnP Action, response ok directly to remote UPnP Control Point! */
			if (reqID > 0)
				bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x1F),
												 WSC_OPCODE_UPNP_DATA, WSC_UPNP_DATA_SUB_ACK,
												 0, 0, reqID, 0, NULL, AP_MODE);

			/*Send M2D to wireless station. */
			WscSendMessage(pAdapter, WSC_OPCODE_MSG, Elem->Msg, Elem->MsgLen, pWscControl, AP_MODE, EAP_CODE_REQ);
			pWscControl->M2DACKBalance++;

			if ((pWscUPnPInfo->registrarID == 0) && (pWscControl->bM2DTimerRunning == FALSE)) {
				/* Add M2D timer used to trigger the EAPFail Packet! */
				RTMPSetTimer(&pWscControl->M2DTimer, WSC_UPNP_M2D_TIME_OUT);
				pWscControl->bM2DTimerRunning = TRUE;
			}

			break;

		case WSC_MSG_WSC_NACK:
		default:
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Recv WscMsg(%d) from UPnP, request EventID=%d! drop it!\n", MsgType, reqID);
			break;
		}
	} else {
		/*WSC msg send from EAP. */
		switch (MsgType) {
		case WSC_MSG_M1:
		case WSC_MSG_M3:
		case WSC_MSG_M5:
		case WSC_MSG_M7:
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapApProxyAction: Rx WscMsg(%d) from EAP\n", MsgType);

			/*This msg send to event-based external registrar */
			if (MsgType == WSC_MSG_M1) {
				bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x1F),
												 WSC_OPCODE_UPNP_DATA, WSC_UPNP_DATA_SUB_TO_ALL,
												 Elem->Msg, Elem->MsgLen, 0, 0, &pWscControl->EntryAddr[0], AP_MODE);
				pWscControl->WscState = WSC_STATE_SENT_M1;
			} else
				bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x1F),
												 WSC_OPCODE_UPNP_DATA, WSC_UPNP_DATA_SUB_TO_ALL,
												 Elem->Msg, Elem->MsgLen, 0, pWscUPnPInfo->registrarID,
												 &pWscControl->EntryAddr[0], AP_MODE);

			break;

		case WSC_MSG_WSC_ACK:
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapApProxyAction: Rx WSC_ACK from EAP\n");

			/* The M2D must appeared before the ACK, so we just need sub it when (pWscUPnPInfo->M2DACKBalance > 0) */
			if (pWscControl->M2DACKBalance > 0)
				pWscControl->M2DACKBalance--;

			break;

		case WSC_MSG_WSC_DONE:
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapApProxyAction: Rx WSC_DONE from EAP\n");
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapApProxyAction: send WSC_DONE to UPnP Registrar!\n");
			/*Send msg to event-based external registrar */
			bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x1F),
											 WSC_OPCODE_UPNP_DATA, WSC_UPNP_DATA_SUB_TO_ONE,
											 Elem->Msg, Elem->MsgLen, 0,
											 pWscUPnPInfo->registrarID, &pWscControl->EntryAddr[0], AP_MODE);
			/*Send EAPFail to wireless station to finish the whole process. */
			WscSendEapFail(pAdapter, pWscControl, FALSE);
			RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
			pWscControl->EapolTimerRunning = FALSE;
			pEntry->bWscCapable = FALSE;
			pWscControl->EapMsgRunning = FALSE;
			NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN);

			if (pWscControl->Wsc2MinsTimerRunning) {
				pWscControl->Wsc2MinsTimerRunning = FALSE;
				RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
			}

			break;

		default:
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Recv WSC Msg(%d) from EAP , it's impossible, drop it!\n", MsgType);
			break;
		}
	}

Fail:

	if (WscData)
		os_free_mem(WscData);

	if (sendToUPnP && (bUPnPStatus == FALSE)) {
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Need to send UPnP but bUPnPStatus is false!MsgType=%d, regID=0x%x!\n", MsgType, reqID);
		WscUPnPErrHandle(pAdapter, pWscControl, reqID);
	}
}
#endif /* CONFIG_AP_SUPPORT */

/*
*	============================================================================
*	Registrar			Registrar			Registrar
*	============================================================================
*/
VOID WscEapRegistrarAction(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MLME_QUEUE_ELEM * Elem,
	IN  UCHAR MsgType,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL pWscControl)
{
	INT DataLen = 0, rv = 0;
	UCHAR OpCode = 0;
	UCHAR *WscData = NULL;
	BOOLEAN bUPnPMsg, bUPnPStatus = FALSE, Cancelled;
	WSC_UPNP_NODE_INFO *pWscUPnPInfo = &pWscControl->WscUPnPNodeInfo;
	UINT MaxWscDataLen = WSC_MAX_DATA_LEN;
	UCHAR CurOpMode = 0xFF;
#ifdef P2P_SUPPORT
	BOOLEAN bReadOwnPIN = FALSE;
#endif /* P2P_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
#endif
	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapRegistrarAction Enter!\n");
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAdapter)
	CurOpMode = STA_MODE;

	if (CurOpMode == STA_MODE) {
		pStaCfg = GetStaCfgByWdev(pAdapter, Elem->wdev);
		ASSERT(pStaCfg);
		if (!pStaCfg) {
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "pStaCfg is NULL\n");
			return;
		}
	}
#ifdef P2P_SUPPORT

	if (Elem->OpMode != OPMODE_STA)
		CurOpMode = AP_MODE;

#endif /* P2P_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	bUPnPMsg = Elem->MsgType == WSC_EAPOL_UPNP_MSG ? TRUE : FALSE;

	if (bUPnPMsg) {
		if (MsgType == WSC_MSG_M1) {
			/* It's a M1 message, we may need to initialize our state machine. */
			if ((pWscControl->WscActionMode == WSC_REGISTRAR)
				&& (pWscControl->EntryIfIdx == WSC_INIT_ENTRY_APIDX)
				&& (pWscControl->WscState < WSC_STATE_WAIT_M1)
				&& (pWscUPnPInfo->bUPnPInProgress == FALSE)) {
				pWscUPnPInfo->bUPnPInProgress = TRUE;
				/*Set the WscState as "WSC_STATE_WAIT_RESP_ID" because UPnP start from this state. */
				pWscControl->WscState = WSC_STATE_WAIT_M1;
				RTMPSetTimer(&pWscUPnPInfo->UPnPMsgTimer, WSC_UPNP_MSG_TIME_OUT);
				pWscUPnPInfo->bUPnPMsgTimerRunning = TRUE;
			}
		}

		OpCode = WSC_OPCODE_UPNP_MASK;
	} else {
		if (pWscControl->EapolTimerRunning)
			pWscControl->EapolTimerRunning = FALSE;
	}

#ifdef WSC_V2_SUPPORT
	MaxWscDataLen = MaxWscDataLen + (UINT)pWscControl->WscV2Info.ExtraTlv.TlvLen;
#endif /* WSC_V2_SUPPORT */
	os_alloc_mem(NULL, (UCHAR **)&WscData, MaxWscDataLen);

	if (WscData == NULL) {
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "WscData Allocate failed!\n");
		goto Fail;
	}

	NdisZeroMemory(WscData, MaxWscDataLen);

	/* Base on state doing the Msg, State change diagram */
	switch (MsgType) {
	case WSC_MSG_M1:
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapRegistrarAction : Rx M1\n");
		/* Receive M1, if we are at WSC_STATE_WAIT_M1 start, process it immediately */
		pWscControl->WscStatus = STATUS_WSC_EAP_M1_RECEIVED;

		if (pWscControl->WscState == WSC_STATE_WAIT_M1) {
			OpCode |= WSC_OPCODE_MSG;
			/* Process M1 */
			rv = ProcessMessageM1(pAdapter, pWscControl, Elem->Msg, Elem->MsgLen, &pWscControl->RegData);

			if (rv)
				goto Fail;
			else {
				BOOLEAN	bSendM2D = TRUE;
#ifdef P2P_SUPPORT

				/*
				*	If own UI is limited UI, we need to use own PIN not PIN of Enrollee.
				*/
				if (P2P_GO_ON(pAdapter) && (pWscControl->EntryIfIdx != BSS0)) {
					if ((pAdapter->P2pCfg.ConfigMethod & WSC_CONFMET_KEYPAD) == 0)
						bReadOwnPIN = TRUE;
				}

				if (bReadOwnPIN) {
					pWscControl->WscPinCodeLen = pWscControl->WscEnrolleePinCodeLen;
					WscGetRegDataPIN(pAdapter, pWscControl->WscEnrolleePinCode, pWscControl);
				}

#endif /* P2P_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
				if (IS_MAP_ENABLE(pAdapter))
					pEntry->DevPeerRole = pWscControl->RegData.PeerInfo.map_DevPeerRole;
#endif /* CONFIG_MAP_SUPPORT */

				if (pWscControl->bWscTrigger && (!pWscControl->bWscAutoTigeer)) {
					if (((pWscControl->WscMode == WSC_PBC_MODE) || (pWscControl->WscMode == WSC_SMPBC_MODE))
						|| (pWscControl->WscMode == WSC_PIN_MODE && pWscControl->WscPinCode != 0))
						bSendM2D = FALSE;
				}


				if (bSendM2D) {
					DataLen = BuildMessageM2D(pAdapter, pWscControl, WscData);
					pWscControl->WscState = WSC_STATE_SENT_M2D;
					pWscControl->M2DACKBalance++;

					if (pWscControl->bM2DTimerRunning == FALSE) {
						/* Add M2D timer used to trigger the EAPFail Packet! */
						RTMPSetTimer(&pWscControl->M2DTimer, WSC_UPNP_M2D_TIME_OUT);
						pWscControl->bM2DTimerRunning = TRUE;
					}
				} else {
					pWscControl->WscStatus = STATUS_WSC_EAP_M2_SENT;
					DataLen = BuildMessageM2(pAdapter, pWscControl, WscData);
					/* Change the state to next one */
						pWscControl->WscState = WSC_STATE_WAIT_M3;

#ifdef CONFIG_STA_SUPPORT

					if ((CurOpMode == STA_MODE) &&
						INFRA_ON(&pAdapter->StaCfg[0]) &&
						!bUPnPMsg)
						pWscControl->WscConfStatus = pWscControl->bConfiguredAP ? WSC_SCSTATE_UNCONFIGURED : WSC_SCSTATE_CONFIGURED;

#endif /* CONFIG_STA_SUPPORT */
					RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M2, NULL, (pWscControl->EntryIfIdx & 0x1F), 0);
				}
			}
		}

		break;

	case WSC_MSG_M3:
		/* Receive M3 */
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapRegistrarAction : Rx M3\n");

		if (pWscControl->WscState == WSC_STATE_WAIT_M3) {
			pWscControl->WscStatus = STATUS_WSC_EAP_M3_RECEIVED;
			rv = ProcessMessageM3(pAdapter, Elem->Msg, Elem->MsgLen, &pWscControl->RegData);

			if (rv)
				goto Fail;
			else {
				OpCode |= WSC_OPCODE_MSG;
				DataLen = BuildMessageM4(pAdapter, pWscControl, WscData);
				pWscControl->WscStatus = STATUS_WSC_EAP_M4_SENT;
				/* Change the state to next one */
				pWscControl->WscState = WSC_STATE_WAIT_M5;
				RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M4, NULL, (pWscControl->EntryIfIdx & 0x1F), 0);
			}
		}

		break;

	case WSC_MSG_M5:
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapRegistrarAction : Rx M5\n");

		if (pWscControl->WscState == WSC_STATE_WAIT_M5) {
			pWscControl->WscStatus = STATUS_WSC_EAP_M5_RECEIVED;
			rv = ProcessMessageM5(pAdapter, pWscControl, Elem->Msg, Elem->MsgLen, &pWscControl->RegData);

			if (rv)
				goto Fail;
			else {
				OpCode |= WSC_OPCODE_MSG;
				DataLen = BuildMessageM6(pAdapter, pWscControl, WscData);
				pWscControl->WscStatus = STATUS_WSC_EAP_M6_SENT;
				/* Change the state to next one */
				pWscControl->WscState = WSC_STATE_WAIT_M7;
				RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M6, NULL, (pWscControl->EntryIfIdx & 0x1F), 0);
			}
		}

		break;

	case WSC_MSG_M7:
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapRegistrarAction : Rx M7\n");

		if (pWscControl->WscState == WSC_STATE_WAIT_M7) {
			pWscControl->WscStatus = STATUS_WSC_EAP_M7_RECEIVED;
			rv = ProcessMessageM7(pAdapter, pWscControl, Elem->Msg, Elem->MsgLen, &pWscControl->RegData);

			if (rv)
				goto Fail;
			else {
				if ((CurOpMode == AP_MODE)
#ifdef CONFIG_STA_SUPPORT
					|| ((CurOpMode == STA_MODE) && ((pWscControl->bConfiguredAP == FALSE)
#ifdef WSC_V2_SUPPORT
													/*
													 *	Check AP is v2 or v1, Check WscV2 Enabled or not
													*/
													|| (pWscControl->WscV2Info.bForceSetAP
														&& pWscControl->WscV2Info.bEnableWpsV2
														&& (pWscControl->RegData.PeerInfo.Version2 != 0))
#endif /* WSC_V2_SUPPORT */
												   ))
#endif /* CONFIG_STA_SUPPORT */
				   ) {
					OpCode |= WSC_OPCODE_MSG;
#ifdef IWSC_SUPPORT

					if ((pAdapter->OpMode == OPMODE_STA) && (pAdapter->StaCfg[0].BssType == BSS_ADHOC))
						pWscControl->WscConfStatus = WSC_SCSTATE_CONFIGURED;

#endif /* IWSC_SUPPORT */

#ifdef MAP_R3
					if (IS_MAP_ENABLE(pAdapter) && IS_MAP_R3_ENABLE(pAdapter)) {
						if (!pAdapter->map_onboard_type && (pWscControl->rcvd_uri_len != 0) && (pAdapter->map_sec_enable == TRUE))
							wext_send_dpp_uri_info(pAdapter, pWscControl->wdev, pWscControl);
					}
#endif /* MAP_R3 */
					DataLen = BuildMessageM8(pAdapter, pWscControl, WscData);
					pWscControl->WscStatus = STATUS_WSC_EAP_M8_SENT;
					/* Change the state to next one */
					pWscControl->WscState = WSC_STATE_WAIT_DONE;
#ifdef VENDOR_FEATURE6_SUPPORT
					/*Registrar case:*/
					{
						/*
						*	If APUT is Registra ,
						*	send event to wscd / miniupnpd , to let it query crendential and sync to system's flash & GUI.
						*/
						WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x1F), WSC_OPCODE_UPNP_CTRL, WSC_UPNP_DATA_SUB_WSC_DONE, &pAdapter->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x1F].wdev.bssid[0], MAC_ADDR_LEN, 0, 0, &pAdapter->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x1F].wdev.bssid[0], AP_MODE);
					}
#endif /* VENDOR_FEATURE6_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
					if (IS_MAP_TURNKEY_ENABLE(pAdapter))
						wapp_send_wsc_eapol_complete_notif (
								pAdapter,
								pWscControl);
#endif /* CONFIG_MAP_SUPPORT */
					RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_M8, NULL, (pWscControl->EntryIfIdx & 0x1F), 0);
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

					if (pWscControl->WscV2Info.bEnableWpsV2 && (CurOpMode == AP_MODE))
						WscAddEntryToAclList(pAdapter, pEntry->func_tb_idx, pEntry->Addr);

#endif /* WSC_V2_SUPPORT */

					/*
					*	1. Complete WPS with this STA. Delete it from WscPeerList for others STA to do WSC with AP
					*	2. Some WPS STA will send dis-assoc close to WSC_DONE
					*	   then AP will miss WSC_DONE from STA; hence we need to call WscDelListEntryByMAC here.
					*/
					if (pEntry && (CurOpMode == AP_MODE)) {
						RTMP_SEM_LOCK(&pWscControl->WscPeerListSemLock);
						WscDelListEntryByMAC(&pWscControl->WscPeerList, pEntry->Addr);
						RTMP_SEM_UNLOCK(&pWscControl->WscPeerListSemLock);
					}

#endif /* CONFIG_AP_SUPPORT */
				}

#ifdef CONFIG_STA_SUPPORT
				else if ((CurOpMode == STA_MODE) &&
						 (pWscControl->bConfiguredAP == TRUE)) {
					/* Some WPS AP expects to receive WSC_NACK when AP is configured */
					OpCode |= WSC_OPCODE_NACK;
					DataLen = BuildMessageNACK(pAdapter, pWscControl, WscData);
					pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
					pWscControl->WscState = WSC_STATE_CONFIGURED;
					pWscControl->EapMsgRunning = FALSE;
					RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_NACK, NULL, (pWscControl->EntryIfIdx & 0x1F), 0);
				}

#endif /* CONFIG_STA_SUPPORT */
			}
		}

		break;

	case WSC_MSG_WSC_DONE:
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapRegistrarAction : Rx DONE\n");

		if (pWscControl->WscState == WSC_STATE_WAIT_DONE) {
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE) {
				pWscControl->WscStatus = STATUS_WSC_EAP_RAP_RSP_DONE_SENT;
				/* Send EAP-Fail */
				WscSendEapFail(pAdapter, pWscControl, TRUE);
				pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
#ifdef P2P_SUPPORT

				if (P2P_GO_ON(pAdapter) &&
					(pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED) &&
					(pWscControl == &pAdapter->ApCfg.MBSSID[MAIN_MBSSID].wdev.WscControl))
					pAdapter->P2pCfg.bStopAuthRsp = TRUE;

#endif /* P2P_SUPPORT */
#ifdef CON_WPS

				/* AP: stop the other side of band */
				if (pWscControl->conWscStatus != CON_WPS_STATUS_DISABLED) {
					WscConWpsStop(pAdapter, FALSE, pWscControl);
					pWscControl->conWscStatus = CON_WPS_STATUS_DISABLED;
				}

#endif /* CON_WPS */
			}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

			if (CurOpMode == STA_MODE) {
				if ((CurOpMode == STA_MODE) && ADHOC_ON(pAdapter))
					WscSendEapFail(pAdapter, pWscControl, FALSE);
				else {
					OpCode |= WSC_OPCODE_ACK;
					DataLen = BuildMessageACK(pAdapter, pWscControl, WscData);
					RTMPSendWirelessEvent(pAdapter, IW_WSC_SEND_ACK, NULL, (pWscControl->EntryIfIdx & 0x0F), 0);
				}

#ifdef IWSC_SUPPORT

				if ((pAdapter->StaCfg[0].BssType == BSS_ADHOC) &&
					(pWscControl->WscMode == WSC_SMPBC_MODE)) {
					pAdapter->StaCfg[0].IWscInfo.IWscSmpbcAcceptCount--;
					pWscControl->WscStatus = STATUS_WSC_CONFIGURED;

					if (pEntry && pEntry->bIWscSmpbcAccept) {
						pEntry->bIWscSmpbcAccept = FALSE;
						WscDelListEntryByMAC(&pWscControl->WscPeerList, pEntry->Addr);
					}

					pAdapter->StaCfg[0].IWscInfo.SmpbcEnrolleeCount++;
				} else
#endif /* IWSC_SUPPORT */
					pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
			}

#endif /* CONFIG_STA_SUPPORT */
			pWscControl->WscState = WSC_STATE_CONFIGURED;
			RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_SUCCESS, NULL, (pWscControl->EntryIfIdx & 0x1F), 0);
#ifdef P2P_SUPPORT

			/*RTMPCancelTimer(&pAdapter->P2pCfg.P2pWscTimer, &Cancelled);*/
			if (P2P_GO_ON(pAdapter) && pEntry && pWscControl->EntryIfIdx != BSS0) {
				UCHAR	P2pIdx = P2P_NOT_FOUND;

				P2pIdx = P2pGroupTabSearch(pAdapter, pEntry->Addr);

				if (P2pIdx != P2P_NOT_FOUND) {
					PRT_P2P_CLIENT_ENTRY pP2pEntry = &pAdapter->P2pTable.Client[P2pIdx];
					/* Update p2p Entry's state. */
					pP2pEntry->P2pClientState = P2PSTATE_CLIENT_WPS_DONE;
				}
			}

			/* default set extended listening to zero for each connection. If this is persistent, will set it. */
			pAdapter->P2pCfg.ExtListenInterval = 0;
			pAdapter->P2pCfg.ExtListenPeriod = 0;
			pAdapter->P2pCfg.WscState = WSC_STATE_CONFIGURED;
#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
			P2pSendWirelessEvent(pAdapter, RT_P2P_WPS_COMPLETED, NULL, NULL);
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */

			if (IS_PERSISTENT_ON(pAdapter) && pEntry && (pEntry->bP2pClient == TRUE)) {
				UCHAR	P2pIdx = P2P_NOT_FOUND;

				P2pIdx = P2pGroupTabSearch(pAdapter, pEntry->Addr);

				if (IS_P2P_GO_ENTRY(pEntry))
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "pEntry is P2P GO.\n");
				else if (IS_P2P_CLI_ENTRY(pEntry))
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "pEntry is P2P CLIENT.\n");
				else
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "pEntry is P2P NONE[%d].\n", pEntry->EntryType);

				if ((P2pIdx != P2P_NOT_FOUND) && (IS_P2P_GO_ENTRY(pEntry) || IS_P2P_CLI_ENTRY(pEntry))) {
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "P2pWPSDone- Save to persistent entry. GrpCap= %x\n",
							  pAdapter->P2pTable.Client[P2pIdx].GroupCapability);
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "1. P2pWPSDone-	Set Extended timing !!!!!!!\n");
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "    ======== Profile :: Cnt = %d ========\n",
							  pWscControl->WscProfile.ProfileCnt);
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "    SSID[%d] = %s.\n",
							  pWscControl->WscProfile.Profile[0].SSID.SsidLength,
							  pWscControl->WscProfile.Profile[0].SSID.Ssid);
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "    AuthType = %x.    EncrType = %x.\n",
							  pWscControl->WscProfile.Profile[0].AuthType,
							  pWscControl->WscProfile.Profile[0].EncrType);
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "    MAC = "MACSTR".\n",
							  MAC2STR(pWscControl->WscProfile.Profile[0].MacAddr));
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "    KeyLen = %d.    KeyIdx = %d.\n",
							  pWscControl->WscProfile.Profile[0].KeyLength,
							  pWscControl->WscProfile.Profile[0].KeyIndex);
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "    Key :: %02x %02x %02x %02x  %02x %02x %02x %02x\n",
							  pWscControl->WscProfile.Profile[0].Key[0],
							  pWscControl->WscProfile.Profile[0].Key[1],
							  pWscControl->WscProfile.Profile[0].Key[2],
							  pWscControl->WscProfile.Profile[0].Key[3],
							  pWscControl->WscProfile.Profile[0].Key[4],
							  pWscControl->WscProfile.Profile[0].Key[5],
							  pWscControl->WscProfile.Profile[0].Key[6],
							  pWscControl->WscProfile.Profile[0].Key[7]);
					MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
							 "             %02x %02x %02x %02x  %02x %02x %02x %02x\n",
							  pWscControl->WscProfile.Profile[0].Key[8],
							  pWscControl->WscProfile.Profile[0].Key[9],
							  pWscControl->WscProfile.Profile[0].Key[10],
							  pWscControl->WscProfile.Profile[0].Key[11],
							  pWscControl->WscProfile.Profile[0].Key[12],
							  pWscControl->WscProfile.Profile[0].Key[13],
							  pWscControl->WscProfile.Profile[0].Key[14],
							  pWscControl->WscProfile.Profile[0].Key[15]);
					P2pPerstTabInsert(pAdapter, pEntry->Addr, &pWscControl->WscProfile.Profile[0]);
					/* this is a persistent connection. */
					pAdapter->P2pCfg.ExtListenInterval = P2P_EXT_LISTEN_INTERVAL;
					pAdapter->P2pCfg.ExtListenPeriod = P2P_EXT_LISTEN_PERIOD;
				}
			}

#endif /* P2P_SUPPORT */
			pWscControl->EapMsgRunning = FALSE;
		}

		break;

	default:
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "WscEapRegistrarAction : Unsupported Msg Type\n");

		if (WscData)
			os_free_mem(WscData);

		return;
	}

	if (OpCode > WSC_OPCODE_UPNP_MASK)
		bUPnPStatus = WscSendUPnPMessage(pAdapter, (pWscControl->EntryIfIdx & 0x1F),
										 WSC_OPCODE_UPNP_DATA, WSC_UPNP_DATA_SUB_NORMAL,
										 WscData, DataLen,
										 Elem->TimeStamp.u.LowPart, Elem->TimeStamp.u.HighPart, &pWscControl->EntryAddr[0], CurOpMode);
	else if (OpCode > 0 && OpCode < WSC_OPCODE_UPNP_MASK) {
#ifdef WSC_V2_SUPPORT
		pWscControl->WscTxBufLen = 0;
		pWscControl->pWscCurBufIdx = NULL;
		pWscControl->bWscLastOne = TRUE;

		if (pWscControl->bWscFragment && (DataLen > pWscControl->WscFragSize)) {
			ASSERT(DataLen < MAX_MGMT_PKT_LEN);
			NdisMoveMemory(pWscControl->pWscTxBuf, WscData, DataLen);
			pWscControl->WscTxBufLen = DataLen;
			NdisZeroMemory(WscData, DataLen);
			pWscControl->bWscLastOne = FALSE;
			pWscControl->bWscFirstOne = TRUE;
			NdisMoveMemory(WscData, pWscControl->pWscTxBuf, pWscControl->WscFragSize);
			DataLen = pWscControl->WscFragSize;
			pWscControl->WscTxBufLen -= pWscControl->WscFragSize;
			pWscControl->pWscCurBufIdx = (pWscControl->pWscTxBuf + pWscControl->WscFragSize);
		}

#endif /* WSC_V2_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == AP_MODE) {
			if (pWscControl->WscState != WSC_STATE_CONFIGURED)
				WscSendMessage(pAdapter, OpCode, WscData, DataLen, pWscControl, AP_MODE, EAP_CODE_REQ);
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

		if (CurOpMode == STA_MODE) {
			if (ADHOC_ON(pAdapter))
				WscSendMessage(pAdapter, OpCode, WscData, DataLen, pWscControl, STA_MODE, EAP_CODE_REQ);
			else
				WscSendMessage(pAdapter, OpCode, WscData, DataLen, pWscControl, STA_MODE, EAP_CODE_RSP);
		}

#endif /* CONFIG_STA_SUPPORT */
	} else
		bUPnPStatus = TRUE;

	if (bUPnPMsg) {
		if (pWscControl->WscState == WSC_STATE_SENT_M2D) {
			/*After M2D, reset the status of State Machine. */
			pWscControl->WscState = WSC_STATE_WAIT_UPNP_START;
			pWscUPnPInfo->bUPnPInProgress = FALSE;
		}
	}

Fail:
	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscEapRegistrarAction : rv = %d\n", rv);

	if (rv) {
		if (rv <= WSC_ERROR_DEV_PWD_AUTH_FAIL) {
			pWscControl->RegData.SelfInfo.ConfigError = rv;
		} else if ((rv == WSC_ERROR_HASH_FAIL) || (rv == WSC_ERROR_HMAC_FAIL))
			pWscControl->RegData.SelfInfo.ConfigError = WSC_ERROR_DECRYPT_CRC_FAIL;

		switch (rv) {
		case WSC_ERROR_HASH_FAIL:
			pWscControl->WscStatus = STATUS_WSC_ERROR_HASH_FAIL;
			break;

		case WSC_ERROR_HMAC_FAIL:
			pWscControl->WscStatus = STATUS_WSC_ERROR_HMAC_FAIL;
			break;

		default:
			pWscControl->WscStatus = STATUS_WSC_FAIL;
			break;
		}

			RTMPSendWirelessEvent(pAdapter, IW_WSC_STATUS_FAIL, NULL, (pWscControl->EntryIfIdx & 0x1F), 0);

		if (bUPnPMsg) {
			if (pWscUPnPInfo->bUPnPMsgTimerRunning == TRUE) {
				RTMPCancelTimer(&pWscUPnPInfo->UPnPMsgTimer, &Cancelled);
				pWscUPnPInfo->bUPnPMsgTimerRunning = FALSE;
			}

			pWscUPnPInfo->bUPnPInProgress = FALSE;
		} else {
			DataLen = BuildMessageNACK(pAdapter, pWscControl, WscData);
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE) {
				WscSendMessage(pAdapter, WSC_OPCODE_NACK, WscData, DataLen, pWscControl, AP_MODE, EAP_CODE_REQ);
				pEntry->bWscCapable = FALSE;
			}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

			if (CurOpMode == STA_MODE) {
				if (ADHOC_ON(pAdapter))
					WscSendMessage(pAdapter, WSC_OPCODE_NACK, WscData, DataLen, pWscControl, STA_MODE, EAP_CODE_REQ);
				else
					WscSendMessage(pAdapter, WSC_OPCODE_NACK, WscData, DataLen, pWscControl, STA_MODE, EAP_CODE_RSP);
			}

#endif /* CONFIG_STA_SUPPORT */
			RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
			pWscControl->EapolTimerRunning = FALSE;
		}

		/*
		*   If a PIN authentication or communication error occurs after sending message M6,
		*   the Registrar MUST warn the user and MUST NOT automatically reuse the PIN.
		*   Furthermore, if the Registrar detects this situation and prompts the user for a new PIN from the Enrollee device,
		*   it MUST NOT accept the same PIN again without warning the user of a potential attack.
		*/
		if (pWscControl->WscState >= WSC_STATE_WAIT_M7) {
			pWscControl->WscRejectSamePinFromEnrollee = TRUE;
			pWscControl->WscPinCode = 0;
		}

		pWscControl->WscState = WSC_STATE_OFF;
		pWscControl->WscStatus = STATUS_WSC_IDLE;
		/*NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN); */
		/*pWscControl->WscMode = 1; */
		bUPnPStatus = FALSE;
	}

	if (WscData)
		os_free_mem(WscData);

	if (bUPnPMsg && (bUPnPStatus == FALSE))
		WscUPnPErrHandle(pAdapter, pWscControl, Elem->TimeStamp.u.LowPart);

	if (pWscControl->WscState == WSC_STATE_CONFIGURED) {
#ifdef WSC_LED_SUPPORT
		UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
		pWscControl->bWscTrigger = FALSE;

/* WPS_BandSteering Support */
#ifdef BAND_STEERING
	if (pAdapter->ApCfg.BandSteering) {

		int apidx = pWscControl->EntryIfIdx & 0x1F;
		struct wifi_dev *wdev = NULL;

		if (VALID_MBSS(pAdapter, apidx))
			wdev = &pAdapter->ApCfg.MBSSID[apidx].wdev;

		if (wdev) {
			PBND_STRG_CLI_TABLE table = Get_BndStrgTable(pAdapter, wdev->func_idx);

			if (table && table->bEnabled) {
				NdisAcquireSpinLock(&table->WpsWhiteListLock);
				ClearWpsWhiteList(&table->WpsWhiteList);
				NdisReleaseSpinLock(&table->WpsWhiteListLock);
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s:channel %u wps whitelist cleared, size : %d\n",
				 __func__, table->Channel, table->WpsWhiteList.size);
			}
		}
	}
#endif


#ifdef IWSC_SUPPORT

		if ((pAdapter->OpMode == OPMODE_STA) &&
			(pAdapter->StaCfg[0].BssType == BSS_ADHOC)) {
			if (pAdapter->StaCfg[0].IWscInfo.bSinglePIN)
				pAdapter->StaCfg[0].IWscInfo.bDoNotStop = TRUE;
			else
				pAdapter->StaCfg[0].IWscInfo.bDoNotStop = FALSE;

			RTMP_SEM_LOCK(&pWscControl->WscConfiguredPeerListSemLock);
			WscInsertPeerEntryByMAC(&pWscControl->WscConfiguredPeerList, pWscControl->WscPeerMAC);
			RTMP_SEM_UNLOCK(&pWscControl->WscConfiguredPeerListSemLock);
			NdisZeroMemory(pWscControl->WscPeerMAC, MAC_ADDR_LEN); /* We need to clear here for 4-way handshaking */
			MlmeEnqueue(pAdapter, IWSC_STATE_MACHINE, IWSC_MT2_MLME_STOP, 0, NULL, 0);
			RTMP_MLME_HANDLER(pAdapter);
		}

#endif /* IWSC_SUPPORT */

		if (pWscControl->Wsc2MinsTimerRunning) {
			pWscControl->Wsc2MinsTimerRunning = FALSE;
			RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
		}

		if (bUPnPMsg) {
			if (pWscUPnPInfo->bUPnPMsgTimerRunning == TRUE) {
				RTMPCancelTimer(&pWscUPnPInfo->UPnPMsgTimer, &Cancelled);
				pWscUPnPInfo->bUPnPMsgTimerRunning = FALSE;
			}

			pWscUPnPInfo->bUPnPInProgress = FALSE;
			pWscUPnPInfo->registrarID = 0;
		}

#ifdef CONFIG_AP_SUPPORT
		else {
			if (CurOpMode == AP_MODE) {
				UCHAR apidx = pEntry->func_tb_idx;
				struct wifi_dev *wdev = &pAdapter->ApCfg.MBSSID[apidx].wdev;

				WscBuildBeaconIE(pAdapter, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, pEntry->func_tb_idx, NULL, 0, CurOpMode);
				WscBuildProbeRespIE(pAdapter, WSC_MSGTYPE_AP_WLAN_MGR, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, pWscControl->EntryIfIdx, NULL, 0, CurOpMode);
				UpdateBeaconHandler(
					pAdapter,
					wdev,
					BCN_UPDATE_IE_CHG);
			}
		}

		NdisZeroMemory(&pAdapter->CommonCfg.WscStaPbcProbeInfo, sizeof(WSC_STA_PBC_PROBE_INFO));
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

		if (CurOpMode == STA_MODE) {
#ifdef IWSC_SUPPORT

			if (pAdapter->StaCfg[0].IWscInfo.bDoNotStop == FALSE)
#endif /* IWSC_SUPPORT */
				pWscControl->WscConfMode = WSC_DISABLE;


			if (pAdapter->StaCfg[0].BssType == BSS_INFRA)
				pWscControl->WscState = WSC_STATE_WAIT_EAPFAIL;
		}

#endif /* CONFIG_STA_SUPPORT */

		if (
#ifdef CONFIG_STA_SUPPORT
			INFRA_ON(&pAdapter->StaCfg[0]) ||
#endif
			(
				(pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED) &&
				((CurOpMode == AP_MODE) || (ADHOC_ON(pAdapter)))
			)
		) {
			pWscControl->WscConfStatus = WSC_SCSTATE_CONFIGURED;
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE) {
				{
					/*
					*	Use ApplyProfileIdx to inform WscUpdatePortCfgTimer AP acts registrar.
					*/
#if defined(CONFIG_MAP_SUPPORT)
					if (!(IS_MAP_TURNKEY_ENABLE(pAdapter) ||
						(pEntry && (pEntry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)))))
#endif
					{
						pWscControl->WscProfile.ApplyProfileIdx |= 0x8000;
						RTMPSetTimer(&pWscControl->WscUpdatePortCfgTimer, 1000);
						pWscControl->WscUpdatePortCfgTimerRunning = TRUE;
					}
				}
			}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

			if (CurOpMode == STA_MODE) {
				pAdapter->StaCfg[0].bAutoConnectByBssid = TRUE;

				if ((pWscControl->bConfiguredAP)
#ifdef WSC_V2_SUPPORT
					/*
					*	Check AP is v2 or v1, Check WscV2 Enabled or not
					*/
					&& !(pWscControl->WscV2Info.bForceSetAP
						 && pWscControl->WscV2Info.bEnableWpsV2
						 && (pWscControl->RegData.PeerInfo.Version2 != 0))
#endif /* WSC_V2_SUPPORT */
				   ) {
					RTMPMoveMemory(&pWscControl->WscProfile, &pWscControl->WscM7Profile, sizeof(pWscControl->WscM7Profile));
				}

					WscWriteConfToPortCfg(pAdapter, pWscControl, &pWscControl->WscProfile.Profile[0], TRUE);

				{
					RtmpOsTaskWakeUp(&(pAdapter->wscTask));
				}

#ifdef IWSC_SUPPORT

				if (pAdapter->StaCfg[0].BssType == BSS_ADHOC) {
					if (pAdapter->StaCfg[0].IWscInfo.bDoNotStop == FALSE) {
						WscBuildBeaconIE(pAdapter, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, BSS0, NULL, 0, STA_MODE);
						WscBuildProbeRespIE(pAdapter, WSC_MSGTYPE_REGISTRAR, WSC_SCSTATE_CONFIGURED, FALSE, 0, 0, BSS0, NULL, 0, STA_MODE);
					}

					pAdapter->StaCfg[0].IWscInfo.bReStart = TRUE;
					WscLinkDown(pAdapter);

					if (pAdapter->StaCfg[0].IWscInfo.bIWscDevQueryReqTimerRunning == FALSE) {
						pAdapter->StaCfg[0].IWscInfo.bIWscDevQueryReqTimerRunning = TRUE;
						RTMPSetTimer(&pAdapter->StaCfg[0].IWscInfo.IWscDevQueryTimer, 200);
					}
				}

#endif /* IWSC_SUPPORT */
			}

#endif /* CONFIG_STA_SUPPORT */
		}

#ifdef WSC_LED_SUPPORT
		/* The protocol is finished. */
		WPSLEDStatus = LED_WPS_SUCCESS;
		RTMPSetLED(pAdapter, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
#endif /* WSC_LED_SUPPORT */
#ifdef IWSC_SUPPORT

		if (pAdapter->StaCfg[0].IWscInfo.bDoNotStop == FALSE)
#endif /* IWSC_SUPPORT */
		{
			pWscControl->WscPinCode = 0;
			pWscControl->WscMode = 1;
		}

		RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
		pWscControl->EapolTimerRunning = FALSE;
#ifdef IWSC_SUPPORT

		/*
		*	Some peer doesn't stop beacon and send de-auth, it will cause 4-way failed when our MAC is higher than peer.
		*	After WPS process complete, delete entry here for adding entry to table again for 4-way handshaking.
		*/
		if (pEntry && (pAdapter->StaCfg[0].BssType == BSS_ADHOC))
			MacTableDeleteEntry(pAdapter, pEntry->wcid, pEntry->Addr);

#endif /* IWSC_SUPPORT */
		return;
	}
}

VOID WscTimeOutProcess(
	IN  PRTMP_ADAPTER pAd,
	IN  PMAC_TABLE_ENTRY pEntry,
	IN  INT nWscState,
	IN  PWSC_CTRL pWscControl)
{
	INT WscMode;
	UCHAR CurOpMode = 0xFF;
#ifdef CONFIG_STA_SUPPORT
	UCHAR if_idx = (pWscControl->EntryIfIdx & 0x0F);
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	CurOpMode = STA_MODE;
#ifdef P2P_SUPPORT

	if (pWscControl->EntryIfIdx != BSS0)
		CurOpMode = AP_MODE;

#endif /* P2P_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

	if (nWscState == WSC_STATE_WAIT_ACK)
		pWscControl->WscState = WSC_STATE_CONFIGURED;
	else if (nWscState == WSC_STATE_WAIT_RESP_ID)
		pWscControl->WscState = WSC_STATE_OFF;
	else if (nWscState == WSC_STATE_WAIT_DISCONN) {
		pWscControl->WscState = WSC_STATE_OFF;
		pWscControl->EapolTimerRunning = FALSE;
#ifdef CONFIG_STA_SUPPORT
		if (pEntry && IS_ENTRY_PEER_AP(pEntry) &&
				pAd->StaCfg[if_idx].ApcliInfStat.Enable == TRUE) {
			RTEnqueueInternalCmd(pAd, CMDTHREAD_APCLI_IF_DOWN,
					(VOID *)&if_idx, sizeof(UCHAR));
		}
#endif /* CONFIG_STA_SUPPORT */
		return;
	}
	else if (nWscState == WSC_STATE_RX_M2D) {
		pWscControl->WscState = WSC_STATE_FAIL;
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == AP_MODE) {
			if (pEntry && IS_ENTRY_CLIENT(pEntry))
				WscSendEapFail(pAd, pWscControl, TRUE);

#ifdef APCLI_SUPPORT

			if (pEntry && IS_ENTRY_PEER_AP(pEntry))
				WscApCliLinkDown(pAd, pWscControl);

#endif /* APCLI_SUPPORT */
		}

#endif /* CONFIG_AP_SUPPORT */
		pWscControl->EapolTimerRunning = FALSE;
		pWscControl->WscRetryCount = 0;
#ifdef CONFIG_STA_SUPPORT

		if (CurOpMode == STA_MODE)
			WscLinkDown(pAd, pWscControl->wdev);

#endif /* CONFIG_STA_SUPPORT */
		return;
	} else if (nWscState == WSC_STATE_WAIT_EAPFAIL) {
		pWscControl->WscState = WSC_STATE_OFF;
		pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
		pWscControl->WscConfMode = WSC_DISABLE;
		/*Inform Disassoc that EAPHandshake is completed*/
		RTMP_OS_COMPLETE(&pWscControl->WscEAPHandshakeCompleted);
	} else {
#ifdef CONFIG_AP_SUPPORT

		if ((pWscControl->WscActionMode == WSC_PROXY) && (pAd->OpMode == OPMODE_AP))
			pWscControl->WscState = WSC_STATE_OFF;
		else
#endif /* CONFIG_AP_SUPPORT */
			pWscControl->WscState = WSC_STATE_FAIL;
	}

	if (nWscState == WSC_STATE_WAIT_M8)
		pWscControl->bWscTrigger = FALSE;

	pWscControl->WscRetryCount = 0;
	NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN);
	pWscControl->EapolTimerRunning = FALSE;

	if (pWscControl->WscMode == 1)
		WscMode = DEV_PASS_ID_PIN;
	else
		WscMode = DEV_PASS_ID_PBC;

#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if ((pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED) &&
			((nWscState == WSC_STATE_WAIT_DONE) || (nWscState == WSC_STATE_WAIT_ACK))) {
			pWscControl->bWscTrigger = FALSE;
			pWscControl->WscConfStatus = WSC_SCSTATE_CONFIGURED;
			WscBuildBeaconIE(pAd, pWscControl->WscConfStatus, FALSE, WscMode, pWscControl->WscConfigMethods, (pWscControl->EntryIfIdx & 0x1F), NULL, 0, CurOpMode);
			WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, pWscControl->WscConfStatus, FALSE, WscMode, pWscControl->WscConfigMethods, pWscControl->EntryIfIdx, NULL, 0, CurOpMode);
			pAd->WriteWscCfgToDatFile = pWscControl->EntryIfIdx;
			WscWriteConfToPortCfg(pAd,
								  pWscControl,
								  &pWscControl->WscProfile.Profile[0],
								  FALSE);
#ifdef P2P_SUPPORT

			if (pWscControl->EntryIfIdx & MIN_NET_DEVICE_FOR_P2P_GO) {
#ifdef RTMP_MAC_PCI
				OS_WAIT(1000);
#endif /* RTMP_MAC_PCI */
				P2P_GoStop(pAd);
				P2P_GoStartUp(pAd, MAIN_MBSSID);
			} else
#endif /* P2P_SUPPORT */
			{
				UCHAR apidx = pWscControl->EntryIfIdx & 0x1F;
				/*
				*	Using CMD thread to prevent in-band command failed.
				*	@20150710
				*	Need to add RfIC to do ApStop/ApStart.
				*	@20160321
				*/
				RTEnqueueInternalCmd(pAd, CMDTHREAD_AP_RESTART, (VOID *)&apidx, sizeof(UCHAR));
			}

			RtmpOsTaskWakeUp(&(pAd->wscTask));
		} else {
			if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
				pEntry->bWscCapable = FALSE;
				WscSendEapFail(pAd, pWscControl, TRUE);
			}

			WscBuildBeaconIE(pAd, pWscControl->WscConfStatus, FALSE, WscMode, pWscControl->WscConfigMethods, (pWscControl->EntryIfIdx & 0x1F), NULL, 0, CurOpMode);
			WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, pWscControl->WscConfStatus, FALSE, WscMode, pWscControl->WscConfigMethods, pWscControl->EntryIfIdx, NULL, 0, CurOpMode);
		}

#ifdef APCLI_SUPPORT

		if (pEntry && IS_ENTRY_PEER_AP(pEntry))
			WscApCliLinkDown(pAd, pWscControl);

#endif /* APCLI_SUPPORT */
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE)
		WscLinkDown(pAd, pWscControl->wdev);

#endif /* CONFIG_STA_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscTimeOutProcess\n");
}

VOID WscEAPOLTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PUCHAR WscData = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	PWSC_CTRL pWscControl = NULL;
	PRTMP_ADAPTER pAd = NULL;
	UINT MaxWscDataLen = WSC_MAX_DATA_LEN;
	UCHAR CurOpMode;
	struct wifi_dev *wdev;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG sta_cfg = NULL;
#endif /* CONFIG_STA_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscEAPOLTimeOutAction\n");
	ASSERT(FunctionContext);

	if (FunctionContext == 0)
		return;

	pWscControl = (PWSC_CTRL)FunctionContext;
	pAd = (PRTMP_ADAPTER)pWscControl->pAd;
	if (pAd == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "pAd is NULL.\n");
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscEAPOLTimeOutAction\n");
		return;
	}
	wdev = (struct wifi_dev *)pWscControl->wdev;
	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "wdev is NULL.\n");
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscEAPOLTimeOutAction\n");
		return;
	}
	wdev->wdev_ops->mac_entry_lookup(pAd, pWscControl->EntryAddr, wdev, &pEntry);

	if (WDEV_WSC_AP(wdev))
		CurOpMode = AP_MODE;
	else {
		CurOpMode = STA_MODE;
#ifdef CONFIG_STA_SUPPORT
		sta_cfg = GetStaCfgByWdev(pAd, wdev);

		if (sta_cfg == NULL)
			return;
#endif /* CONFIG_STA_SUPPORT */
	}

	if ((CurOpMode == AP_MODE) || ADHOC_ON(pAd)) {
		if (pEntry == NULL) {
#ifdef CONFIG_AP_SUPPORT

			/*
			*	Some WPS Client will send dis-assoc close to WSC_DONE.
			*	If AP misses WSC_DONE, WPS Client still sends dis-assoc to AP.
			*	AP driver needs to check wsc_state here for considering WPS process with this client is completed.
			*/
			if ((CurOpMode == AP_MODE) &&
				((pWscControl->WscState == WSC_STATE_WAIT_DONE) || (pWscControl->WscState == WSC_STATE_WAIT_ACK))) {
				pWscControl->WscStatus = STATUS_WSC_CONFIGURED;
				pWscControl->bWscTrigger = FALSE;
				pWscControl->RegData.ReComputePke = 1;

				if (pWscControl->Wsc2MinsTimerRunning) {
					BOOLEAN Cancelled;

					pWscControl->Wsc2MinsTimerRunning = FALSE;
					RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
				}

				WscTimeOutProcess(pAd, NULL, pWscControl->WscState, pWscControl);
			}

#endif /* CONFIG_AP_SUPPORT */
#ifdef IWSC_SUPPORT
#ifdef CONFIG_STA_SUPPORT

			if ((pAd->OpMode == OPMODE_STA) &&
				(sta_cfg->BssType == BSS_ADHOC) &&
				(pWscControl->WscConfMode == WSC_ENROLLEE))
				wdev->IWscInfo.bReStart = TRUE;

#endif /* CONFIG_STA_SUPPORT */
#endif /* IWSC_SUPPORT */
			pWscControl->EapolTimerRunning = FALSE;
			NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN);
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "sta is left.\n");
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscEAPOLTimeOutAction\n");
			return;
		}
	}

	if (!pWscControl->EapolTimerRunning) {
		pWscControl->WscRetryCount = 0;
		goto out;
	}

	if (pWscControl->EapolTimerPending) {
		RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "EapolTimer Pending......\n");
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscEAPOLTimeOutAction\n");
		return;
	}

#ifdef WSC_V2_SUPPORT
	MaxWscDataLen = MaxWscDataLen + (UINT)pWscControl->WscV2Info.ExtraTlv.TlvLen;
#endif /* WSC_V2_SUPPORT */
	os_alloc_mem(NULL, (UCHAR **)&WscData, MaxWscDataLen);

	if (WscData != NULL)
		NdisZeroMemory(WscData, WSC_MAX_DATA_LEN);

#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pWscControl->WscState <= WSC_STATE_CONFIGURED ||
			pWscControl->WscState == WSC_STATE_WAIT_RESP_ID) &&
			(pWscControl->WscActionMode != WSC_PROXY)) {
			/* A timer in the AP should cause to be disconnected after 5 seconds if a */
			/* valid EAP-Rsp/Identity indicating WPS is not received. */
			/* << from WPS EAPoL and RSN handling.doc >> */
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_RESP_ID, pWscControl);
			/* If do disassocation here, it will affect connection of non-WPS clients. */
			goto out;
		}
	}

#endif /* CONFIG_AP_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscState = %d\n", pWscControl->WscState);

	switch (pWscControl->WscState) {
	case WSC_STATE_WAIT_REQ_ID:

		/* For IWSC case, keep sending EAPOL_START until 2 mins timeout */
		if ((pWscControl->WscRetryCount >= 2)
#ifdef CONFIG_STA_SUPPORT
			&& (sta_cfg && (sta_cfg->BssType == BSS_INFRA))
#endif /* CONFIG_STA_SUPPORT */
		   )
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_REQ_ID, pWscControl);
		else {
			pWscControl->WscRetryCount++;
#ifdef CONFIG_STA_SUPPORT

			if (sta_cfg && (sta_cfg->BssType == BSS_INFRA) && (CurOpMode == STA_MODE))
				WscSendEapolStart(pAd, sta_cfg->Bssid, CurOpMode, wdev);
			else
#endif /* CONFIG_STA_SUPPORT */
				WscSendEapolStart(pAd, pEntry->Addr, CurOpMode, wdev);

			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_WSC_START:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_WSC_START, pWscControl);
		else {
			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_M1:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M1, pWscControl);
		else {
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE)
				WscSendMessage(pWscControl->pAd, WSC_OPCODE_START, NULL, 0, pWscControl, AP_MODE, EAP_CODE_REQ);

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

			if (CurOpMode == STA_MODE)
				WscSendEapRspId(pAd, pEntry, pWscControl);

#endif /* CONFIG_STA_SUPPORT */
			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_SENT_M1:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M2, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_ENROLLEE) {
#ifdef CONFIG_AP_SUPPORT

				if (CurOpMode == AP_MODE) {
					if (IS_ENTRY_CLIENT(pEntry))
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length, pWscControl,
									   AP_MODE, EAP_CODE_REQ);
					else if (IS_ENTRY_PEER_AP(pEntry))
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length,
									   pWscControl,
									   AP_CLIENT_MODE, EAP_CODE_RSP);
				}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

				if (CurOpMode == STA_MODE)
					WscSendMessage(pWscControl->pAd, WSC_OPCODE_MSG, pWscControl->RegData.LastTx.Data, pWscControl->RegData.LastTx.Length, pWscControl, STA_MODE, EAP_CODE_RSP);

#endif /* CONFIG_STA_SUPPORT */
			}

			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_RX_M2D:
		if (pWscControl->WscRetryCount >= 3)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_RX_M2D, pWscControl);
		else {
			pWscControl->WscRetryCount++;
#ifdef WSC_STA_SUPPORT
			if (sta_cfg)
				WscSendEapolStart(pAd, sta_cfg->Bssid, CurOpMode, wdev);
#endif
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_PIN:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_PIN, pWscControl);
		else {
			pWscControl->WscRetryCount++;
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "No PIN CODE, cannot send M2 out!\n");
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_M3:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M3, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_REGISTRAR) {
#ifdef CONFIG_AP_SUPPORT

				if (CurOpMode == AP_MODE)
					WscSendMessage(pWscControl->pAd,
								   WSC_OPCODE_MSG,
								   pWscControl->RegData.LastTx.Data,
								   pWscControl->RegData.LastTx.Length,
								   pWscControl,
								   AP_MODE, EAP_CODE_REQ);

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

				if (CurOpMode == STA_MODE) {
					if (ADHOC_ON(pAd))
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length,
									   pWscControl,
									   STA_MODE,
									   EAP_CODE_REQ);
					else
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length,
									   pWscControl,
									   STA_MODE,
									   EAP_CODE_RSP);
				}

#endif /* CONFIG_STA_SUPPORT */
			}

			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_M4:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M4, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_ENROLLEE) {
#ifdef CONFIG_AP_SUPPORT

				if (CurOpMode == AP_MODE) {
					if (IS_ENTRY_CLIENT(pEntry))
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length,
									   pWscControl,
									   AP_MODE,
									   EAP_CODE_REQ);
					else if (IS_ENTRY_PEER_AP(pEntry))
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length,
									   pWscControl,
									   AP_CLIENT_MODE,
									   EAP_CODE_RSP);
				}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

				if (CurOpMode == STA_MODE)
					WscSendMessage(pWscControl->pAd,
								   WSC_OPCODE_MSG, pWscControl->RegData.LastTx.Data,
								   pWscControl->RegData.LastTx.Length,
								   pWscControl,
								   STA_MODE, EAP_CODE_RSP);

#endif /* CONFIG_STA_SUPPORT */
			}

			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_M5:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M5, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_REGISTRAR) {
#ifdef CONFIG_AP_SUPPORT

				if (CurOpMode == AP_MODE)
					WscSendMessage(pWscControl->pAd,
								   WSC_OPCODE_MSG,
								   pWscControl->RegData.LastTx.Data,
								   pWscControl->RegData.LastTx.Length,
								   pWscControl,
								   AP_MODE, EAP_CODE_REQ);

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

				if (CurOpMode == STA_MODE) {
					if (ADHOC_ON(pAd))
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length,
									   pWscControl,
									   STA_MODE,
									   EAP_CODE_REQ);
					else
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length,
									   pWscControl,
									   STA_MODE,
									   EAP_CODE_RSP);
				}

#endif /* CONFIG_STA_SUPPORT */
			}

			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_M6:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M6, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_ENROLLEE) {
#ifdef CONFIG_AP_SUPPORT

				if (CurOpMode == AP_MODE) {
					if (IS_ENTRY_CLIENT(pEntry))
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length,
									   pWscControl,
									   AP_MODE, EAP_CODE_REQ);
					else if (IS_ENTRY_PEER_AP(pEntry))
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length,
									   pWscControl,
									   AP_CLIENT_MODE, EAP_CODE_RSP);
				}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

				if (CurOpMode == STA_MODE)
					WscSendMessage(pWscControl->pAd,
								   WSC_OPCODE_MSG,
								   pWscControl->RegData.LastTx.Data,
								   pWscControl->RegData.LastTx.Length,
								   pWscControl,
								   STA_MODE, EAP_CODE_RSP);

#endif /* CONFIG_STA_SUPPORT */
			}

			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_M7:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M7, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_REGISTRAR) {
#ifdef CONFIG_AP_SUPPORT

				if (CurOpMode == AP_MODE)
					WscSendMessage(pWscControl->pAd,
								   WSC_OPCODE_MSG,
								   pWscControl->RegData.LastTx.Data,
								   pWscControl->RegData.LastTx.Length,
								   pWscControl,
								   AP_MODE,
								   EAP_CODE_REQ);

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

				if (CurOpMode == STA_MODE) {
					if (ADHOC_ON(pAd))
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length,
									   pWscControl,
									   STA_MODE,
									   EAP_CODE_REQ);
					else
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length,
									   pWscControl,
									   STA_MODE,
									   EAP_CODE_RSP);
				}

#endif /* CONFIG_STA_SUPPORT */
			}

			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_M8:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_M8, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_ENROLLEE) {
#ifdef CONFIG_AP_SUPPORT

				if (CurOpMode == AP_MODE) {
					if (IS_ENTRY_CLIENT(pEntry))
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length,
									   pWscControl,
									   AP_MODE,
									   EAP_CODE_REQ);
					else if (IS_ENTRY_PEER_AP(pEntry))
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length,
									   pWscControl,
									   AP_CLIENT_MODE,
									   EAP_CODE_RSP);
				}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

				if (CurOpMode == STA_MODE)
					WscSendMessage(pWscControl->pAd,
								   WSC_OPCODE_MSG, pWscControl->RegData.LastTx.Data,
								   pWscControl->RegData.LastTx.Length,
								   pWscControl,
								   STA_MODE,
								   EAP_CODE_RSP);

#endif /* CONFIG_STA_SUPPORT */
			}

			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;

	case WSC_STATE_WAIT_DONE:
		if (pWscControl->WscRetryCount >= 2)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_DONE, pWscControl);
		else {
			if (pWscControl->WscActionMode == WSC_REGISTRAR) {
#ifdef CONFIG_AP_SUPPORT

				if (CurOpMode == AP_MODE)
					WscSendMessage(pWscControl->pAd,
								   WSC_OPCODE_MSG,
								   pWscControl->RegData.LastTx.Data,
								   pWscControl->RegData.LastTx.Length,
								   pWscControl,
								   AP_MODE,
								   EAP_CODE_REQ);

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

				if (CurOpMode == STA_MODE) {
					if (ADHOC_ON(pAd))
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length,
									   pWscControl,
									   STA_MODE,
									   EAP_CODE_REQ);
					else
						WscSendMessage(pWscControl->pAd,
									   WSC_OPCODE_MSG,
									   pWscControl->RegData.LastTx.Data,
									   pWscControl->RegData.LastTx.Length,
									   pWscControl,
									   STA_MODE,
									   EAP_CODE_RSP);
				}

#endif /* CONFIG_STA_SUPPORT */
			}

			pWscControl->WscRetryCount++;
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_MSG_TIME_OUT);
		}

		break;
#ifdef CONFIG_AP_SUPPORT

	/* Only AP_Enrollee needs to wait EAP_ACK */
	case WSC_STATE_WAIT_ACK:
		WscTimeOutProcess(pWscControl->pAd,
						  pEntry, WSC_STATE_WAIT_ACK, pWscControl);
		break;
#endif /* CONFIG_AP_SUPPORT */

	case WSC_STATE_WAIT_EAPFAIL:

		/* Wait 2 seconds */
		if (pWscControl->WscRetryCount >= 1)
			WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_EAPFAIL, pWscControl);
		else {
			RTMPModTimer(&pWscControl->EapolTimer, WSC_EAP_EAP_FAIL_TIME_OUT);
			pWscControl->WscRetryCount++;
		}

		break;

	case WSC_STATE_WAIT_DISCONN:

		/* Enrollee needs to wait for Deauth from Registrar */
		WscTimeOutProcess(pWscControl->pAd, pEntry, WSC_STATE_WAIT_DISCONN, pWscControl);

		break;

	default:
		break;
	}

out:

	if (WscData)
		os_free_mem(WscData);

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			 "<----- WscEAPOLTimeOutAction\n");
}

VOID Wsc2MinsTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	PRTMP_ADAPTER pAd = NULL;
#ifdef CONFIG_AP_SUPPORT
	INT IsAPConfigured = 0;
#endif /* CONFIG_AP_SUPPORT */
	BOOLEAN Cancelled;
	UCHAR CurOpMode = 0xFF;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
#endif /* CONFIG_STA_SUPPORT */
	struct wifi_dev *wdev;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> Wsc2MinsTimeOutAction\n");

	if (pWscControl != NULL) {
		wdev = (struct wifi_dev *)pWscControl->wdev;
		ASSERT(wdev);

		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, ("wdev is NULL!\n"));
			return;
		}

		pAd =  (PRTMP_ADAPTER)pWscControl->pAd;

		if (pAd == NULL) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "pAd is NULL!\n");
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- Wsc2MinsTimeOutAction\n");
			return;
		}

#ifdef CONFIG_AP_SUPPORT
#ifdef CON_WPS

		if (pWscControl->conWscStatus != CON_WPS_STATUS_DISABLED) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN, "CON_WPS: Reset the status to default.\n");
			pWscControl->conWscStatus = CON_WPS_STATUS_DISABLED;
		}

#endif /* CON_WPS */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			CurOpMode = AP_MODE;

			if (wdev && wdev->wdev_type == WDEV_TYPE_STA) {
				UCHAR apcli_idx = (pWscControl->EntryIfIdx & 0x0F);
				pStaCfg = &pAd->StaCfg[apcli_idx];
				CurOpMode = STA_MODE;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			CurOpMode = STA_MODE;
			pStaCfg = &pAd->StaCfg[0];
		}
#ifdef P2P_SUPPORT

		if (pWscControl->EntryIfIdx != BSS0)
			CurOpMode = AP_MODE;

#endif /* P2P_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Wsc2MinsTimerRunning is %s\n",
				 pWscControl->Wsc2MinsTimerRunning ? "TRUE, reset WscState to WSC_STATE_OFF" : "FALSE");
		/* Enable CLI interface */
#ifdef CONFIG_APCLI_SUPPORT
		if ((CurOpMode == STA_MODE) &&
			(wdev->wdev_type == WDEV_TYPE_STA)) {
			UCHAR apcli_idx = (pWscControl->EntryIfIdx & 0x0F);

			if (pAd->StaCfg[apcli_idx].ApcliInfStat.ApCliInit != FALSE)
				pAd->StaCfg[apcli_idx].ApcliInfStat.Enable = TRUE;
		}

#endif
#ifdef WSC_LED_SUPPORT
		/* 120 seconds WPS walk time expiration. */
		pWscControl->bWPSWalkTimeExpiration = TRUE;
#endif /* WSC_LED_SUPPORT */

		if (pWscControl->Wsc2MinsTimerRunning) {
			pWscControl->bWscTrigger = FALSE;
			pWscControl->EapolTimerRunning = FALSE;
			RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
#ifdef CONFIG_AP_SUPPORT

			if (CurOpMode == AP_MODE) {
				IsAPConfigured = pWscControl->WscConfStatus;

				if ((pWscControl->EntryIfIdx & 0x1F) < pAd->ApCfg.BssidNum) {
					UCHAR apidx = pWscControl->EntryIfIdx & 0x1F;
					struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
/* WPS_BandSteering Support */
#ifdef BAND_STEERING
					if (pAd->ApCfg.BandSteering) {
						PBND_STRG_CLI_TABLE table = Get_BndStrgTable(pAd, wdev->func_idx);

						if (table && table->bEnabled) {
							NdisAcquireSpinLock(&table->WpsWhiteListLock);
							ClearWpsWhiteList(&table->WpsWhiteList);
							NdisReleaseSpinLock(&table->WpsWhiteListLock);
							MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s:channel %u wps whitelist cleared, size : %d\n",
									__func__, table->Channel, table->WpsWhiteList.size);
						}
					}
#endif
					WscBuildBeaconIE(pWscControl->pAd, IsAPConfigured, FALSE, 0, 0, (pWscControl->EntryIfIdx & 0x1F), NULL, 0, CurOpMode);
					WscBuildProbeRespIE(pWscControl->pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0, pWscControl->EntryIfIdx, NULL, 0, CurOpMode);
					UpdateBeaconHandler(
						pWscControl->pAd,
						wdev,
						BCN_UPDATE_IE_CHG);
				}

				if ((pWscControl->WscConfMode & WSC_PROXY) == 0) {
					/* Proxy mechanism is disabled */
					pWscControl->WscState = WSC_STATE_OFF;
				}
			}

#endif /* CONFIG_AP_SUPPORT */
			pWscControl->WscMode = 1;
			pWscControl->WscRetryCount = 0;
			pWscControl->Wsc2MinsTimerRunning = FALSE;
			pWscControl->WscSelReg = 0;
			pWscControl->WscStatus = STATUS_WSC_IDLE;
			RTMPSendWirelessEvent(pAd, IW_WSC_2MINS_TIMEOUT, NULL, (pWscControl->EntryIfIdx & 0x1F), 0);
#ifdef VENDOR_FEATURE6_SUPPORT
#ifdef CONFIG_AP_SUPPORT
			WscSendUPnPMessage(pAd, (pWscControl->EntryIfIdx & 0x1F), WSC_OPCODE_UPNP_CTRL, WSC_UPNP_DATA_SUB_WSC_TIMEOUT, &pAd->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x1F].wdev.bssid[0], MAC_ADDR_LEN, 0, 0, &pAd->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x1F].wdev.bssid[0], AP_MODE);
#endif /* CONFIG_AP_SUPPORT */
#endif /* VENDOR_FEATURE6_SUPPORT */

			if (pWscControl->WscScanTimerRunning) {
				pWscControl->WscScanTimerRunning = FALSE;
				RTMPCancelTimer(&pWscControl->WscScanTimer, &Cancelled);
			}

			if (pWscControl->WscPBCTimerRunning) {
				pWscControl->WscPBCTimerRunning = FALSE;
				RTMPCancelTimer(&pWscControl->WscPBCTimer, &Cancelled);
			}

#ifdef CONFIG_STA_SUPPORT

			if (CurOpMode == STA_MODE) {
				pStaCfg->bAutoConnectByBssid = FALSE;
				RTMPZeroMemory(pStaCfg->MlmeAux.AutoReconnectSsid, MAX_LEN_OF_SSID);
				pStaCfg->MlmeAux.AutoReconnectSsidLen = pAd->StaCfg[0].SsidLen;
				RTMPMoveMemory(pStaCfg->MlmeAux.AutoReconnectSsid, pAd->StaCfg[0].Ssid, pAd->StaCfg[0].SsidLen);

				if (INFRA_ON(pStaCfg) ||
					(pWscControl->WscConfMode == WSC_ENROLLEE))
					WscLinkDown(pAd, &pStaCfg->wdev);
				else {
					AsicDisableSync(pAd, HW_BSSID_0);
					WscBuildBeaconIE(pAd, pWscControl->WscConfStatus, FALSE, 0, 0, BSS0, NULL, 0, CurOpMode);
					WscBuildProbeRespIE(pAd,
										WSC_MSGTYPE_REGISTRAR,
										pWscControl->WscConfStatus,
										FALSE,
										0,
										0,
										BSS0,
										NULL,
										0,
										CurOpMode);
					UpdateBeaconHandler(
						pAd,
						&pAd->StaCfg[0].wdev,
						BCN_UPDATE_IF_STATE_CHG);
					AsicEnableIbssSync(
						pAd,
						pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)],
						HW_BSSID_0,
						OPMODE_ADHOC);
				}

				pWscControl->WscConfMode = WSC_DISABLE;
				pWscControl->WscState = WSC_STATE_OFF;
			}

#endif /* CONFIG_STA_SUPPORT */
		}

#ifdef WSC_LED_SUPPORT

		/* if link is up, there shall be nothing wrong */
		/* perhaps we will set another flag to do it */
		if ((pStaCfg && STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) &&
			(pWscControl->WscState == WSC_STATE_OFF) &&
			(pWscControl->WscStatus == STATUS_WSC_CONFIGURED))
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscConnectTimeout --> Connection OK\n");
		else {
			UCHAR WPSLEDStatus;

			pWscControl->WscStatus = STATUS_WSC_FAIL;
			pWscControl->WscState = WSC_STATE_OFF;

			/* WPS LED mode 7, 8, 11 or 12. */
			if ((LED_MODE(pAd) == WPS_LED_MODE_7) ||
				(LED_MODE(pAd) == WPS_LED_MODE_8) ||
				(LED_MODE(pAd) == WPS_LED_MODE_11) ||
				(LED_MODE(pAd) == WPS_LED_MODE_12)) {
				pWscControl->bSkipWPSTurnOffLED = FALSE;
				/* Turn off the WPS LED modoe due to the maximum WPS processing time is expired (120 seconds). */
				WPSLEDStatus = LED_WPS_TURN_LED_OFF;
				RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
			} else if ((LED_MODE(pAd) == WPS_LED_MODE_9) /* WPS LED mode 9. */
					  ) {
				if (pWscControl->WscMode == WSC_PIN_MODE) { /* PIN method. */
					/* The NIC using PIN method fails to finish the WPS handshaking within 120 seconds. */
					WPSLEDStatus = LED_WPS_ERROR;
					RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
					/* Turn off the WPS LED after 15 seconds. */
					RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_WPS_FAIL_LED_PATTERN_TIMEOUT);
					/* The Ralink UI would make RT_OID_DISCONNECT_REQUEST request while it receive STATUS_WSC_EAP_FAILED. */
					/* Allow the NIC to turn off the WPS LED after WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT seconds. */
					pWscControl->bSkipWPSTurnOffLED = TRUE;
					RTMPSetTimer(&pWscControl->WscSkipTurnOffLEDTimer, WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT);
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: The NIC using PIN method fails to finish the WPS handshaking within 120 seconds.\n", __func__);
				} else if (pWscControl->WscMode == WSC_PBC_MODE) { /* PBC method. */
					switch (pWscControl->WscLastWarningLEDMode) { /* Based on last WPS warning LED mode. */
					case 0:
					case LED_WPS_ERROR:
					case LED_WPS_SESSION_OVERLAP_DETECTED:
						/* Failed to find any partner. */
						WPSLEDStatus = LED_WPS_ERROR;
						RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
						/* Turn off the WPS LED after 15 seconds. */
						RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_WPS_FAIL_LED_PATTERN_TIMEOUT);
						/* The Ralink UI would make RT_OID_DISCONNECT_REQUEST request while it receive STATUS_WSC_EAP_FAILED. */
						/* Allow the NIC to turn off the WPS LED after WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT seconds. */
						pWscControl->bSkipWPSTurnOffLED = TRUE;
						RTMPSetTimer(&pWscControl->WscSkipTurnOffLEDTimer, WSC_WPS_SKIP_TURN_OFF_LED_TIMEOUT);
						MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: Last WPS LED status is LED_WPS_ERROR.\n", __func__);
						break;

					default:
						/* do nothing. */
						break;
					}
				} else {
					/* do nothing. */
				}
			} else {
				/* do nothing. */
			}

			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN, "WscConnectTimeout --> Fail to connect\n");
		}

#endif /* WSC_LED_SUPPORT */
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- Wsc2MinsTimeOutAction\n");
}

/*
*	========================================================================
*
*	Routine Description:
*		Classify EAP message type for enrolee
*
*	Arguments:
*		pAd         - NIC Adapter pointer
*		Elem		- The EAP packet
*
*	Return Value:
*		Received EAP message type
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*
*	========================================================================
*/
UCHAR WscRxMsgType(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PMLME_QUEUE_ELEM pElem)
{
	USHORT Length;
	PUCHAR pData;
	USHORT WscType, WscLen;
	RTMP_STRING id_data[] = {"hello"};
	RTMP_STRING fail_data[] = {"EAP_FAIL"};
	RTMP_STRING wsc_start[] = {"WSC_START"};
#ifdef WSC_V2_SUPPORT
	RTMP_STRING wsc_frag_ack[] = "WSC_FRAG_ACK";
#endif /* WSC_V2_SUPPORT */
	RTMP_STRING regIdentity[] = {"WFA-SimpleConfig-Registrar"};
	RTMP_STRING enrIdentity[] = {"WFA-SimpleConfig-Enrollee"};

	if (pElem->Msg[0] == 'W' && pElem->Msg[1] == 'F' && pElem->Msg[2] == 'A') {
		/* Eap-Rsp(Identity) */
		if (memcmp(regIdentity, pElem->Msg, strlen(regIdentity)) == 0)
			return  WSC_MSG_EAP_REG_RSP_ID;
		else if (memcmp(enrIdentity, pElem->Msg, strlen(enrIdentity)) == 0)
			return  WSC_MSG_EAP_ENR_RSP_ID;
	} else if (pElem->MsgLen && NdisEqualMemory(id_data, pElem->Msg, strlen(id_data))) {
		/* Eap-Req/Identity(hello) */
		return  WSC_MSG_EAP_REQ_ID;
	} else if (pElem->MsgLen && NdisEqualMemory(fail_data, pElem->Msg, strlen(fail_data))) {
		/* Eap-Fail */
		return  WSC_MSG_EAP_FAIL;
	} else if (pElem->MsgLen && NdisEqualMemory(wsc_start, pElem->Msg, strlen(wsc_start))) {
		/* Eap-Req(Wsc_Start) */
		return WSC_MSG_EAP_REQ_START;
	}

#ifdef WSC_V2_SUPPORT
	else if (pElem->MsgLen && NdisEqualMemory(wsc_frag_ack, pElem->Msg, strlen(wsc_frag_ack))) {
		/* WSC FRAG ACK */
		return WSC_MSG_EAP_FRAG_ACK;
	}

#endif /* WSC_V2_SUPPORT */
	else {
		/* Eap-Esp(Messages) */
		pData = pElem->Msg;
		Length = (USHORT)pElem->MsgLen;
		/* the first TLV item in EAP Messages must be WSC_IE_VERSION */
		NdisMoveMemory(&WscType, pData, 2);

		if (ntohs(WscType) != WSC_ID_VERSION)
			goto out;

		/* Not Wsc Start, We have to look for WSC_IE_MSG_TYPE to classify M2 ~ M8, the remain size must large than 4 */
		while (Length > 4) {
			/* arm-cpu has packet alignment issue, it's better to use memcpy to retrieve data */
			NdisMoveMemory(&WscType, pData, 2);
			NdisMoveMemory(&WscLen,  pData + 2, 2);
			WscLen = ntohs(WscLen);
			if (Length < WscLen + 4) {
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "unexpected WSC IE Length(%u)\n", WscLen);
				break;
			}
			if (ntohs(WscType) == WSC_ID_MSG_TYPE)
				return *(pData + 4);	/* Found the message type */

			pData  += (WscLen + 4);
			Length -= (WscLen + 4);
		}
	}

out:
	return  WSC_MSG_UNKNOWN;
}

/*
*	========================================================================
*
*	Routine Description:
*		Classify WSC message type
*
*	Arguments:
*		EAPType		Value of EAP message type
*		MsgType		Internal Message definition for MLME state machine
*
*	Return Value:
*		TRUE		Found appropriate message type
*		FALSE		No appropriate message type
*
*	Note:
*		All these constants are defined in wsc.h
*		For supplicant, there is only EAPOL Key message avaliable
*
*	========================================================================
*/
BOOLEAN	WscMsgTypeSubst(
	IN  UCHAR EAPType,
	IN  UCHAR EAPCode,
	OUT INT *MsgType)
{
	switch (EAPType) {
	case EAPPacket:
		*MsgType = WSC_EAPOL_PACKET_MSG;
		break;

	case EAPOLStart:
		*MsgType = WSC_EAPOL_START_MSG;
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscMsgTypeSubst : unsupported EAP Type(%d);\n", EAPType);
		return FALSE;
	}

	return TRUE;
}

VOID WscInitRegistrarPair(RTMP_ADAPTER *pAd, WSC_CTRL *pWscControl, UCHAR apidx)
{
	UCHAR CurOpMode = 0xff;
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	INT ret;
#endif

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscInitRegistrarPair\n");
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	CurOpMode = STA_MODE;
#ifdef P2P_SUPPORT

	if (pWscControl->EntryIfIdx != BSS0) {
		CurOpMode = AP_MODE;
		apidx = MAIN_MBSSID;
	}

#endif /* P2P_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	pWscControl->WscActionMode = 0;
	/* 1. Version */
	/*pWscControl->RegData.SelfInfo.Version = WSC_VERSION; */
	/* 2. UUID Enrollee, last 6 bytes use MAC */
	NdisMoveMemory(&pWscControl->RegData.SelfInfo.Uuid[0], &pWscControl->Wsc_Uuid_E[0], UUID_LEN_HEX);
	/* 3. MAC address */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (!VALID_MBSS(pAd, apidx)) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
					 "apidx >= cap(%d)!\n", MAX_MBSSID_NUM(pAd));
			apidx = 0;
		}

		if (VALID_MBSS(pAd, apidx)) {
			NdisMoveMemory(pWscControl->RegData.SelfInfo.MacAddr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, 6);
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		NdisMoveMemory(pWscControl->RegData.SelfInfo.MacAddr, pAd->StaCfg[apidx].wdev.if_addr, 6);
		wdev = &pAd->StaCfg[apidx].wdev;
	}

#endif /* CONFIG_STA_SUPPORT */
	/* 4. Device Name */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (!RTMP_TEST_FLAG(pWscControl, 0x04)) {
			NdisZeroMemory(&pWscControl->RegData.SelfInfo.DeviceName[0],
						   sizeof(pWscControl->RegData.SelfInfo.DeviceName));
			ret = snprintf(&pWscControl->RegData.SelfInfo.DeviceName[0],
					 sizeof(pWscControl->RegData.SelfInfo.DeviceName), "%s_%d", AP_WSC_DEVICE_NAME, apidx);
			if (os_snprintf_error(sizeof(pWscControl->RegData.SelfInfo.DeviceName), ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "DeviceName snprintf error!\n");

		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		if (!RTMP_TEST_FLAG(pWscControl, 0x04)) {
#ifdef WIDI_SUPPORT

			if (pAd->StaCfg[0].bWIDI && !pAd->StaCfg[0].WscControl.bWscTrigger)
				NdisMoveMemory(&pWscControl->RegData.SelfInfo.DeviceName,
							   WIDI_STA_WSC_DEVICE_NAME, sizeof(WIDI_STA_WSC_DEVICE_NAME));
			else
#endif /* WIDI_SUPPORT */
				NdisMoveMemory(&pWscControl->RegData.SelfInfo.DeviceName,
							   STA_WSC_DEVICE_NAME, sizeof(STA_WSC_DEVICE_NAME));
		}
	}

#endif /* CONFIG_STA_SUPPORT */

	/* 5. Manufacture woody */
	if (!RTMP_TEST_FLAG(pWscControl, 0x01))
		NdisMoveMemory(&pWscControl->RegData.SelfInfo.Manufacturer,
					   WSC_MANUFACTURE, sizeof(WSC_MANUFACTURE));

	/* 6. Model Name */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (!RTMP_TEST_FLAG(pWscControl, 0x02))
			NdisMoveMemory(&pWscControl->RegData.SelfInfo.ModelName, AP_WSC_MODEL_NAME, sizeof(AP_WSC_MODEL_NAME));
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		if (!RTMP_TEST_FLAG(pWscControl, 0x02)) {
#ifdef WIDI_SUPPORT

			if (pAd->StaCfg[0].bWIDI && !pAd->StaCfg[0].WscControl.bWscTrigger)
				NdisMoveMemory(&pWscControl->RegData.SelfInfo.ModelName,
							   WIDI_STA_WSC_MODEL_NAME, sizeof(WIDI_STA_WSC_MODEL_NAME));
			else
#endif /* WIDI_SUPPORT */
				NdisMoveMemory(&pWscControl->RegData.SelfInfo.ModelName, STA_WSC_MODEL_NAME, sizeof(STA_WSC_MODEL_NAME));
		}
	}

#endif /* CONFIG_STA_SUPPORT */

	/* 7. Model Number */

	if (!RTMP_TEST_FLAG(pWscControl, 0x08))
		NdisMoveMemory(&pWscControl->RegData.SelfInfo.ModelNumber, WSC_MODEL_NUMBER, sizeof(WSC_MODEL_NUMBER));

	/* 8. Serial Number */
	if (!RTMP_TEST_FLAG(pWscControl, 0x10))
		NdisMoveMemory(&pWscControl->RegData.SelfInfo.SerialNumber, WSC_MODEL_SERIAL, sizeof(WSC_MODEL_SERIAL));

	/* 9. Authentication Type Flags */
	/* Open(=1), WPAPSK(=2),Shared(=4), WPA2PSK(=20),WPA(=8),WPA2(=10) */
	/* (0x01 | 0x02 | 0x04 | 0x20 | 0x08 | 0x10) = 0x3F */
	/* WCN vista logo will check this flags. */
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2)
		/*
		*	AuthTypeFlags only needs to include Open and WPA2PSK in WSC 2.0.
		*/
		pWscControl->RegData.SelfInfo.AuthTypeFlags = cpu2be16(0x0021);
	else
#endif /* WSC_V2_SUPPORT */
		pWscControl->RegData.SelfInfo.AuthTypeFlags = cpu2be16(0x003F);

	/* 10. Encryption Type Flags */
	/* None(=1), WEP(=2), TKIP(=4), AES(=8) */
	/* (0x01 | 0x02 | 0x04 | 0x08) = 0x0F */
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2)
		/*
		*	EncrTypeFlags only needs to include None and AES in WSC 2.0.
		*/
		pWscControl->RegData.SelfInfo.EncrTypeFlags = cpu2be16(0x0009);
	else
#endif /* WSC_V2_SUPPORT */
		pWscControl->RegData.SelfInfo.EncrTypeFlags  = cpu2be16(0x000F);

	/* 11. Connection Type Flag */
#ifdef CONFIG_STA_SUPPORT

	if ((pAd->StaCfg[0].BssType == BSS_ADHOC) &&
		(pAd->OpMode == OPMODE_STA)) {
		pWscControl->RegData.SelfInfo.ConnTypeFlags = 0x02;				/* IBSS */
	} else
#endif /* CONFIG_STA_SUPPORT */
		pWscControl->RegData.SelfInfo.ConnTypeFlags = 0x01;					/* ESS */

	/* 12. Associate state */
	pWscControl->RegData.SelfInfo.AssocState = cpu2be16(0x0000);		/* Not associated */
	/* 13. Configure Error */
	pWscControl->RegData.SelfInfo.ConfigError = cpu2be16(0x0000);		/* No error */
	/* 14. OS Version */
	pWscControl->RegData.SelfInfo.OsVersion = cpu2be32(0x80000000);		/* first bit must be 1 */
	/* 15. RF Band */
	/* Some WPS AP would check RfBand value in M1, ex. D-Link DIR-628 */
	pWscControl->RegData.SelfInfo.RfBand = 0x00;

	if (wdev) {
		if (WMODE_CAP_5G(wdev->PhyMode))
			pWscControl->RegData.SelfInfo.RfBand |= WSC_RFBAND_50GHZ;			/* 5.0G */

		if (WMODE_CAP_2G(wdev->PhyMode))
			pWscControl->RegData.SelfInfo.RfBand |= WSC_RFBAND_24GHZ;			/* 2.4G */
	}

	/* 16. Config Method */
	pWscControl->RegData.SelfInfo.ConfigMethods = cpu2be16(pWscControl->WscConfigMethods);
	/*pWscControl->RegData.EnrolleeInfo.ConfigMethods = cpu2be16(WSC_CONFIG_METHODS);		// Label, Display, PBC */
	/*pWscControl->RegData.EnrolleeInfo.ConfigMethods = cpu2be16(0x0084);		// Label, Display, PBC */
	/* 17. Simple Config State */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE)
		pWscControl->RegData.SelfInfo.ScState = pWscControl->WscConfStatus;

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE)
		pWscControl->RegData.SelfInfo.ScState = 0x01;

#endif /* CONFIG_STA_SUPPORT */

	/* 18. Device Password ID */
	if (pWscControl->WscMode == WSC_PBC_MODE)
		pWscControl->RegData.SelfInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PBC); /* PBC */

#ifdef IWSC_SUPPORT
	else if (pWscControl->WscMode == WSC_SMPBC_MODE)
		pWscControl->RegData.SelfInfo.DevPwdId = cpu2be16(DEV_PASS_ID_SMPBC); /* SMPBC mode */

#endif /* IWSC_SUPPORT */
	else {
		/* Let PIN be default DPID.  */
#ifdef IWSC_SUPPORT
		if ((pAd->StaCfg[0].BssType == BSS_ADHOC) &&
			(pAd->StaCfg[0].IWscInfo.bLimitedUI == FALSE)) {
			pWscControl->RegData.SelfInfo.DevPwdId = cpu2be16(DEV_PASS_ID_REG);		/* PIN mode */
		} else
#endif /* IWSC_SUPPORT */
			pWscControl->RegData.SelfInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PIN);		/* PIN mode */
	}

	/* 19. SSID */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (VALID_MBSS(pAd, apidx))
			NdisMoveMemory(pWscControl->RegData.SelfInfo.Ssid, pAd->ApCfg.MBSSID[apidx].Ssid, pAd->ApCfg.MBSSID[apidx].SsidLen);
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE)
		NdisMoveMemory(pWscControl->RegData.SelfInfo.Ssid, pAd->StaCfg[0].Ssid, pAd->StaCfg[0].SsidLen);

#endif /* CONFIG_STA_SUPPORT */
	/* 20. Primary Device Type */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
#ifdef P2P_SUPPORT

		if (pWscControl->EntryIfIdx >= MIN_NET_DEVICE_FOR_P2P_CLI)
			NdisMoveMemory(&pWscControl->RegData.SelfInfo.PriDeviceType, pAd->P2pCfg.DevInfo.PriDeviceType, 8);
		else
#endif /* P2P_SUPPORT */
			if (pWscControl->EntryIfIdx & MIN_NET_DEVICE_FOR_APCLI)
				NdisMoveMemory(&pWscControl->RegData.SelfInfo.PriDeviceType, &STA_Wsc_Pri_Dev_Type[0], 8);
			else
				NdisMoveMemory(&pWscControl->RegData.SelfInfo.PriDeviceType, &AP_Wsc_Pri_Dev_Type[0], 8);
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE)
		NdisMoveMemory(&pWscControl->RegData.SelfInfo.PriDeviceType, &STA_Wsc_Pri_Dev_Type[0], 8);

#endif /* CONFIG_STA_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscInitRegistrarPair\n");
}


VOID WscSendEapReqId(
	IN  PRTMP_ADAPTER pAd,
	IN  PMAC_TABLE_ENTRY pEntry,
	IN  UCHAR CurOpMode)
{
	UCHAR Header802_3[14];
	USHORT Length;
	IEEE8021X_FRAME Ieee_8021x;
	EAP_FRAME EapFrame;
	UCHAR *pOutBuffer = NULL;
	ULONG FrameLen = 0;
	UCHAR Data[] = "hello";
	UCHAR Id;
	PWSC_CTRL pWpsCtrl = NULL;

	NdisZeroMemory(Header802_3, sizeof(UCHAR) * 14);
	/* 1. Send EAP-Rsp Id */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscSendEapReqId\n");
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (VALID_MBSS(pAd, pEntry->func_tb_idx)) {
			pWpsCtrl = &pEntry->wdev->WscControl;
			MAKE_802_3_HEADER(Header802_3,
							  &pEntry->Addr[0],
							  &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid[0],
							  EAPOL);
		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		pWpsCtrl = &pEntry->wdev->WscControl;
		MAKE_802_3_HEADER(Header802_3,
						  &pEntry->Addr[0],
						  &pAd->StaCfg[0].wdev.if_addr[0],
						  EAPOL);
	}

#endif /* CONFIG_STA_SUPPORT */

	if (pWpsCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "pWpsCtrl == NULL!\n");
		return;
	}

	/* Length, -1 NULL pointer of string */
	Length = sizeof(EAP_FRAME) + sizeof(Data) - 1;
	/* Zero 802.1x body */
	NdisZeroMemory(&Ieee_8021x, sizeof(Ieee_8021x));
	Ieee_8021x.Version = EAPOL_VER;
	Ieee_8021x.Type    = EAPPacket;
	Ieee_8021x.Length  = cpu2be16(Length);
	/* Zero EAP frame */
	NdisZeroMemory(&EapFrame, sizeof(EapFrame));
	/* RFC 3748 Ch 4.1: recommended to initalize Identifier with a random number */
	Id = RandomByte(pAd);

	if (Id == pWpsCtrl->lastId)
		Id += 1;

	EapFrame.Code   = EAP_CODE_REQ;
	EapFrame.Id     = Id;
	EapFrame.Length = cpu2be16(Length);
	EapFrame.Type   = EAP_TYPE_ID;
	pWpsCtrl->lastId = Id;
	/* Out buffer for transmitting EAP-Req(Identity) */
	os_alloc_mem(NULL, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);

	if (pOutBuffer == NULL)
		return;

	FrameLen = 0;
	/* Make	 Transmitting frame */
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(IEEE8021X_FRAME), &Ieee_8021x,
					  sizeof(EapFrame), &EapFrame,
					  (sizeof(Data) - 1), Data,
					  END_OF_ARGS);
	/* Copy frame to Tx ring */
	RTMPToWirelessSta(pAd, pEntry, Header802_3, sizeof(Header802_3), (PUCHAR)pOutBuffer, FrameLen, TRUE);
	pWpsCtrl->WscRetryCount = 0;

	if (pOutBuffer)
		os_free_mem(pOutBuffer);

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscSendEapReqId\n");
}

/*
*	========================================================================
*
*	Routine Description:
*		Send EAPoL-Start packet to AP.
*
*	Arguments:
*		pAd         - NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		Actions after link up
*		1. Change the correct parameters
*		2. Send EAPOL - START
*
*	========================================================================
*/
VOID WscSendEapolStart(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PUCHAR pBssid,
	IN  UCHAR CurOpMode,
	IN  VOID *wdev_obj)
{
	IEEE8021X_FRAME Packet;
	UCHAR Header802_3[14];
	MAC_TABLE_ENTRY *pEntry;
	UCHAR if_idx = 0;
	struct wifi_dev *wdev;
	WSC_CTRL *wsc_ctrl;

	ASSERT(wdev_obj);

	if (!wdev_obj) {
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 "wdev_obj is null.\n");
		return;
	}

	wdev = (struct wifi_dev *)wdev_obj;
	wdev->wdev_ops->mac_entry_lookup(pAdapter, pBssid, wdev, &pEntry);

	if (pEntry == NULL) {
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
				 "%s: cannot find this entry("MACSTR")\n",
				  __func__,
				  MAC2STR(pBssid));
		return;
	}

	wsc_ctrl = &wdev->WscControl;
	/* SAER1 is taking long time to connect, so instead of idle wait, retry */
	/* EAPOL START frame after receiving M2D */
#ifdef WSC_STA_SUPPORT
	if (wsc_ctrl->WscState > WSC_STATE_RX_M2D)
		return;
#else
	if (wsc_ctrl->WscState >= WSC_STATE_WAIT_WSC_START)
		return;
#endif

	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscSendEapolStart\n");
	if_idx = pEntry->func_tb_idx;
	NdisZeroMemory(Header802_3, sizeof(UCHAR) * 14);
	/* 1. Change the authentication to open and encryption to none if necessary. */
	/* init 802.3 header and Fill Packet */
	MAKE_802_3_HEADER(Header802_3,
					  pBssid,
					  &wdev->if_addr[0],
					  EAPOL);
	/* Zero message 2 body */
	NdisZeroMemory(&Packet, sizeof(Packet));
	Packet.Version = EAPOL_VER;
	Packet.Type    = EAPOLStart;
	Packet.Length  = cpu2be16(0);

	if (pEntry)
		RTMPToWirelessSta(pAdapter, pEntry, Header802_3, sizeof(Header802_3), (PUCHAR)&Packet, 4, TRUE);

	/* Update WSC status */
	wsc_ctrl->WscStatus = STATUS_WSC_EAPOL_START_SENT;
	wsc_ctrl->WscState = WSC_STATE_WAIT_REQ_ID;

	if (!wsc_ctrl->EapolTimerRunning) {
		wsc_ctrl->EapolTimerRunning = TRUE;
		RTMPSetTimer(&wsc_ctrl->EapolTimer, WSC_EAPOL_START_TIME_OUT);
	}

	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscSendEapolStart\n");
}


VOID WscSendEapRspId(
	IN PRTMP_ADAPTER pAdapter,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PWSC_CTRL pWscControl)
{
	UCHAR Header802_3[14];
	USHORT Length = 0;
	IEEE8021X_FRAME Ieee_8021x;
	EAP_FRAME EapFrame;
	UCHAR *pOutBuffer = NULL;
	ULONG FrameLen = 0;
	UCHAR regIdentity[] = "WFA-SimpleConfig-Registrar-1-0";
	UCHAR enrIdentity[] = "WFA-SimpleConfig-Enrollee-1-0";
	UCHAR CurOpMode = 0xff;
	struct wifi_dev *wdev;

	wdev = (struct wifi_dev *)pWscControl->wdev;
	NdisZeroMemory(Header802_3, sizeof(UCHAR) * 14);
	/* 1. Send EAP-Rsp Id */
	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscSendEapRspId\n");

	if (WDEV_WSC_AP(wdev))
		CurOpMode = AP_MODE;
	else
		CurOpMode = STA_MODE;

	MAKE_802_3_HEADER(Header802_3,
					  &pEntry->Addr[0],
					  &wdev->if_addr[0],
					  EAPOL);

	/* Length, -1 NULL pointer of string */
	if (pWscControl->WscConfMode == WSC_ENROLLEE)
		Length = sizeof(EAP_FRAME) + sizeof(enrIdentity) - 1;
	else if (pWscControl->WscConfMode == WSC_REGISTRAR)
		Length = sizeof(EAP_FRAME) + sizeof(regIdentity) - 1;

	/* Zero 802.1x body */
	NdisZeroMemory(&Ieee_8021x, sizeof(Ieee_8021x));
	Ieee_8021x.Version = EAPOL_VER;
	Ieee_8021x.Type    = EAPPacket;
	Ieee_8021x.Length  = cpu2be16(Length);
	/* Zero EAP frame */
	NdisZeroMemory(&EapFrame, sizeof(EapFrame));
	EapFrame.Code   = EAP_CODE_RSP;
	EapFrame.Id     = pWscControl->lastId;
	EapFrame.Length = cpu2be16(Length);
	EapFrame.Type   = EAP_TYPE_ID;
	/* Out buffer for transmitting EAP-Req(Identity) */
	os_alloc_mem(NULL, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);

	if (pOutBuffer == NULL)
		return;

	FrameLen = 0;

	if (pWscControl->WscConfMode == WSC_REGISTRAR) {
		/* Make tx frame */
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(IEEE8021X_FRAME), &Ieee_8021x,
						  sizeof(EapFrame), &EapFrame,
						  (sizeof(regIdentity) - 1), regIdentity,
						  END_OF_ARGS);
	} else if (pWscControl->WscConfMode == WSC_ENROLLEE) {
#ifdef IWSC_SUPPORT
		PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAdapter, wdev);

		if (pStaCfg && pStaCfg->BssType == BSS_ADHOC)
			pWscControl->WscConfStatus = WSC_SCSTATE_UNCONFIGURED;

#endif /* IWSC_SUPPORT */
		/* Make	 Transmitting frame */
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(IEEE8021X_FRAME), &Ieee_8021x,
						  sizeof(EapFrame), &EapFrame,
						  (sizeof(enrIdentity) - 1), enrIdentity,
						  END_OF_ARGS);
	} else {
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
			"WscConfMode(%d) is not WSC_REGISTRAR nor WSC_ENROLLEE.\n", pWscControl->WscConfMode);
		goto out;
	}

	/* Copy frame to Tx ring */
	RTMPToWirelessSta((PRTMP_ADAPTER)pWscControl->pAd, pEntry,
					  Header802_3, LENGTH_802_3, (PUCHAR)pOutBuffer, FrameLen, TRUE);
	pWscControl->WscRetryCount = 0;

	if (!pWscControl->EapolTimerRunning) {
		pWscControl->EapolTimerRunning = TRUE;
		RTMPSetTimer(&pWscControl->EapolTimer, WSC_EAP_ID_TIME_OUT);
	}

out:

	if (pOutBuffer)
		os_free_mem(pOutBuffer);

	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			 "<----- WscSendEapRspId\n");
}

VOID WscUPnPErrHandle(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl,
	IN  UINT eventID)
{
	int dataLen;
	UCHAR *pWscData;
	UCHAR CurOpMode;
	struct wifi_dev *wdev;

	wdev = (struct wifi_dev *)pWscControl->wdev;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Into WscUPnPErrHandle, send WSC_OPCODE_UPNP_CTRL with eventID=0x%x!\n", eventID);
#ifdef P2P_SUPPORT
	return;
#endif /* P2P_SUPPORT */

	if (WDEV_WSC_AP(wdev))
		CurOpMode = AP_MODE;
	else
		CurOpMode = STA_MODE;

	os_alloc_mem(NULL, (UCHAR **)&pWscData, WSC_MAX_DATA_LEN);

	if (pWscData != NULL) {
		NdisZeroMemory(pWscData, WSC_MAX_DATA_LEN);
		dataLen = BuildMessageNACK(pAd, pWscControl, pWscData);
		WscSendUPnPMessage(pAd, (pWscControl->EntryIfIdx & 0x1F),
						   WSC_OPCODE_UPNP_DATA, WSC_UPNP_DATA_SUB_NORMAL,
						   pWscData, dataLen, eventID, 0, NULL, CurOpMode);
		os_free_mem(pWscData);
	} else {
		WscSendUPnPMessage(pAd, (pWscControl->EntryIfIdx & 0x1F),
						   WSC_OPCODE_UPNP_CTRL, 0, NULL, 0, eventID, 0, NULL, CurOpMode);
	}
}

/*
*	Format of iwcustom msg WSC clientJoin message:
*		1. SSID which station want to probe(32 bytes):
*			<SSID string>
*			*If the length if SSID string is small than 32 bytes, fill 0x0 for remaining bytes.
*		2. sender MAC address(6 bytes):
*		3. Status:
*			Set as 1 means change APStatus as 1.
*			Set as 2 means change STAStatus as 1.
*			Set as 3 means trigger msg.
*
*			32         6        1
*		+----------+--------+------+
*		|SSIDString| SrcMAC |Status|
*/
int WscSendUPnPConfReqMsg(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apIdx,
	IN PUCHAR ssidStr,
	IN PUCHAR macAddr,
	IN INT Status,
	IN UINT eventID,
	IN UCHAR CurOpMode)
{
#define WSC_JOIN_MSG_LEN (MAX_LEN_OF_SSID + MAC_ADDR_LEN + 1)
	UCHAR pData[WSC_JOIN_MSG_LEN] = {0};
#ifdef P2P_SUPPORT

	if (apIdx >= MIN_NET_DEVICE_FOR_P2P_CLI)
		return 0;

#endif /* P2P_SUPPORT */
	strncpy((RTMP_STRING *) pData, (RTMP_STRING *)ssidStr, MAX_LEN_OF_SSID);
	NdisMoveMemory(&pData[MAX_LEN_OF_SSID], macAddr, MAC_ADDR_LEN);
	pData[MAX_LEN_OF_SSID + MAC_ADDR_LEN] = Status;
	WscSendUPnPMessage(pAd, apIdx, WSC_OPCODE_UPNP_MGMT, WSC_UPNP_MGMT_SUB_CONFIG_REQ,
					   &pData[0], WSC_JOIN_MSG_LEN, eventID, 0, NULL, CurOpMode);
	return 0;
}


/*
*	NETLINK tunnel msg format send to WSCUPnP handler in user space:
*	1. Signature of following string(Not include the quote, 8 bytes)
*			"RAWSCMSG"
*	2. eID: eventID (4 bytes)
*			the ID of this message(4 bytes)
*	3. aID: ackID (4 bytes)
*			means that which event ID this mesage was response to.
*	4. TL:  Message Total Length (4 bytes)
*			Total length of this message.
*	5. F:   Flag (2 bytes)
*			used to notify some specific character of this msg segment.
*				Bit 1: fragment
*					set as 1 if netlink layer have more segment of this Msg need to send.
*				Bit 2~15: reserve, should set as 0 now.
*	5. SL:  Segment Length(2 bytes)
*			msg actual length in this segment, The SL may not equal the "TL" field if "F" ==1
*	6. devMac: device mac address(6 bytes)
*			Indicate the netdevice which this msg belong. For the wscd in user space will
*			depends this address dispatch the msg to correct UPnP Device instance to handle it.
*	7. "WSC_MSG" info:
*
*		 8                 4       4       4      2    2        6      variable length(MAXIMUM=232)
*	+------------+----+----+----+--+--+------+------------------------+
*	|  Signature       |eID  |aID  | TL   | F | SL|devMac| WSC_MSG                          |
*
*/
int WscSendUPnPMessage(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR devIfIdx,
	IN USHORT msgType,
	IN USHORT msgSubType,
	IN PUCHAR pData,
	IN INT dataLen,
	IN UINT eventID,
	IN UINT toIPAddr,
	IN PUCHAR pMACAddr,
	IN UCHAR CurOpMode)
{
	/*	union iwreq_data wrqu; */
	RTMP_WSC_NLMSG_HDR *pNLMsgHdr;
	RTMP_WSC_MSG_HDR *pWscMsgHdr;
	UCHAR hdrBuf[42]; /*RTMP_WSC_NLMSG_HDR_LEN + RTMP_WSC_MSG_HDR_LEN */
	int totalLen, leftLen, copyLen;
	PUCHAR pBuf = NULL, pBufPtr = NULL, pPos = NULL;
	PUCHAR	pDevAddr = NULL;
#ifdef CONFIG_AP_SUPPORT
	UCHAR	bssIdx = devIfIdx;
#endif /* CONFIG_AP_SUPPORT */
	ULONG Now;

	if (!pMACAddr)
		return FALSE;

#ifdef P2P_SUPPORT

	if (devIfIdx >= MIN_NET_DEVICE_FOR_P2P_CLI)
		return 0;

#endif /* P2P_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscSendUPnPMessage\n");

	if ((msgType & WSC_OPCODE_UPNP_MASK) != WSC_OPCODE_UPNP_MASK)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
#ifdef APCLI_SUPPORT

		if (devIfIdx & MIN_NET_DEVICE_FOR_APCLI) {
			bssIdx &= (~MIN_NET_DEVICE_FOR_APCLI);

			if (bssIdx >= MAX_APCLI_NUM)
				return FALSE;

			if (bssIdx < MAX_APCLI_NUM)
				pDevAddr = &pAd->StaCfg[bssIdx].wdev.if_addr[0];
		} else
#endif /* APCLI_SUPPORT */
			pDevAddr = &pAd->ApCfg.MBSSID[bssIdx].wdev.bssid[0];
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE)
		pDevAddr = &pAd->StaCfg[0].wdev.if_addr[0];

#endif /* CONFIG_STA_SUPPORT */

	if (pDevAddr == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "pDevAddr == NULL!\n");
		return FALSE;
	}

	/*Prepare the NLMsg header */
	memset(hdrBuf, 0, sizeof(hdrBuf));
	pNLMsgHdr = (RTMP_WSC_NLMSG_HDR *)hdrBuf;
	memcpy(pNLMsgHdr, WSC_MSG_SIGNATURE, RTMP_WSC_NLMSG_SIGNATURE_LEN);
	NdisGetSystemUpTime(&Now);
	pNLMsgHdr->envID = Now;
	pNLMsgHdr->ackID = eventID;
	pNLMsgHdr->msgLen = dataLen + RTMP_WSC_MSG_HDR_LEN;
	/*
	*	In order to support multiple wscd, we need this new field to notify
	*	the wscd which interface this msg send from.
	*/
	NdisMoveMemory(&pNLMsgHdr->devAddr[0],  pDevAddr, MAC_ADDR_LEN);
	/*Prepare the WscMsg header */
	pWscMsgHdr = (RTMP_WSC_MSG_HDR *)(hdrBuf + sizeof(RTMP_WSC_NLMSG_HDR));

	switch (msgType) {
	case WSC_OPCODE_UPNP_DATA:
		pWscMsgHdr->msgType = WSC_OPCODE_UPNP_DATA;
		break;

	case WSC_OPCODE_UPNP_MGMT:
		pWscMsgHdr->msgType = WSC_OPCODE_UPNP_MGMT;
		break;

	case WSC_OPCODE_UPNP_CTRL:
		pWscMsgHdr->msgType = WSC_OPCODE_UPNP_CTRL;
		break;

	default:
		return FALSE;
	}

	pWscMsgHdr->msgSubType = msgSubType;
	pWscMsgHdr->ipAddr = toIPAddr;
	pWscMsgHdr->msgLen = dataLen;

	if ((pWscMsgHdr->msgType == WSC_OPCODE_UPNP_DATA) &&
		(eventID == 0) && (NdisEqualMemory(pMACAddr, ZERO_MAC_ADDR, MAC_ADDR_LEN) == FALSE)) {
		pWscMsgHdr->msgSubType |= WSC_UPNP_DATA_SUB_INCLUDE_MAC;
		pNLMsgHdr->msgLen += MAC_ADDR_LEN;
		pWscMsgHdr->msgLen += MAC_ADDR_LEN;
	}

	/*Allocate memory and copy the msg. */
	totalLen = leftLen = pNLMsgHdr->msgLen;
	pPos = pData;
	os_alloc_mem(NULL, (UCHAR **)&pBuf, IWEVCUSTOM_MSG_MAX_LEN);

	if (pBuf != NULL) {
		int firstSeg = 1;

		while (leftLen) {
			/*Prepare the payload */
			memset(pBuf, 0, IWEVCUSTOM_MSG_MAX_LEN);
			pNLMsgHdr->segLen = (leftLen > IWEVCUSTOM_PAYLOD_MAX_LEN ? IWEVCUSTOM_PAYLOD_MAX_LEN : leftLen);
			leftLen -= pNLMsgHdr->segLen;
			pNLMsgHdr->flags = (leftLen > 0 ? 1 : 0);
			memcpy(pBuf, pNLMsgHdr, RTMP_WSC_NLMSG_HDR_LEN);
			pBufPtr = &pBuf[RTMP_WSC_NLMSG_HDR_LEN];

			if (firstSeg) {
				memcpy(pBufPtr, pWscMsgHdr, RTMP_WSC_MSG_HDR_LEN);
				pBufPtr += RTMP_WSC_MSG_HDR_LEN;
				copyLen = (pNLMsgHdr->segLen - RTMP_WSC_MSG_HDR_LEN);

				if ((pWscMsgHdr->msgSubType & WSC_UPNP_DATA_SUB_INCLUDE_MAC) == WSC_UPNP_DATA_SUB_INCLUDE_MAC) {
					NdisMoveMemory(pBufPtr, pMACAddr, MAC_ADDR_LEN);
					pBufPtr += MAC_ADDR_LEN;
					copyLen -= MAC_ADDR_LEN;
				}

				NdisMoveMemory(pBufPtr, pPos, copyLen);
				pPos += copyLen;
				firstSeg = 0;
			} else {
				NdisMoveMemory(pBufPtr, pPos, pNLMsgHdr->segLen);
				pPos += pNLMsgHdr->segLen;
			}

			/*Send WSC Msg to wscd, msg length = pNLMsgHdr->segLen + sizeof(RTMP_WSC_NLMSG_HDR) */
			RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_WSC_UPNP_EVENT_FLAG, NULL, pBuf, pNLMsgHdr->segLen + sizeof(RTMP_WSC_NLMSG_HDR));
		}

		os_free_mem(pBuf);
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscSendUPnPMessage\n");
	return TRUE;
}


VOID WscSendMessage(
	IN  PRTMP_ADAPTER pAdapter,
	IN  UCHAR OpCode,
	IN  PUCHAR pData,
	IN  INT Len,
	IN  PWSC_CTRL pWscControl,
	IN  UCHAR OpMode,
	IN  UCHAR EapType)
{
	/* Inb-EAP Message */
	UCHAR Header802_3[14];
	USHORT Length, MsgLen;
	IEEE8021X_FRAME Ieee_8021x;
	EAP_FRAME EapFrame;
	WSC_FRAME WscFrame;
	UCHAR *pOutBuffer = NULL;
	ULONG FrameLen = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef CONFIG_AP_SUPPORT
	UCHAR bssIdx = (pWscControl->EntryIfIdx & 0x1F);
#endif /* CONFIG_AP_SUPPORT */
	UCHAR CurOpMode = 0xFF;

	if ((Len <= 0) && (OpCode != WSC_OPCODE_START) && (OpCode != WSC_OPCODE_FRAG_ACK))
		return;

	/* Send message */
	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscSendMessage\n");
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAdapter)
	CurOpMode = STA_MODE;
#ifdef P2P_SUPPORT

	if (pWscControl->EntryIfIdx != BSS0)
		CurOpMode = AP_MODE;

#endif /* P2P_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	NdisZeroMemory(Header802_3, sizeof(UCHAR) * 14);
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (OpMode == AP_MODE) {
			if (VALID_MBSS(pAdapter, bssIdx))
				MAKE_802_3_HEADER(Header802_3, &pWscControl->EntryAddr[0], &pAdapter->ApCfg.MBSSID[bssIdx].wdev.bssid[0], EAPOL);
		}

#ifdef APCLI_SUPPORT
		else if (OpMode == AP_CLIENT_MODE) {
			bssIdx = bssIdx & 0x0F;
			if (bssIdx < MAX_APCLI_NUM)
				MAKE_802_3_HEADER(Header802_3, &pWscControl->EntryAddr[0], &pAdapter->StaCfg[bssIdx].wdev.if_addr[0], EAPOL);
		}

#endif /* APCLI_SUPPORT */
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		UCHAR *bssid;

		if (INFRA_ON(&pAdapter->StaCfg[0]))
			bssid = &pAdapter->StaCfg[0].Bssid[0];
		else
			bssid = &pWscControl->EntryAddr[0];

		MAKE_802_3_HEADER(Header802_3, bssid, &pAdapter->StaCfg[0].wdev.if_addr[0], EAPOL);
	}

#endif /* CONFIG_STA_SUPPORT */
	/* Length = EAP + WSC_Frame + Payload */
	Length = sizeof(EAP_FRAME) + sizeof(WSC_FRAME) + Len;

	if (pWscControl->bWscFragment && (pWscControl->bWscFirstOne)) {
		Length += 2;
		MsgLen = pWscControl->WscTxBufLen + Len;
		MsgLen = htons(MsgLen);
	}

	/* Zero 802.1x body */
	NdisZeroMemory(&Ieee_8021x, sizeof(Ieee_8021x));
	Ieee_8021x.Version = EAPOL_VER;
	Ieee_8021x.Type    = EAPPacket;
	Ieee_8021x.Length  = cpu2be16(Length);
	/* Zero EAP frame */
	NdisZeroMemory(&EapFrame, sizeof(EapFrame));

	if (EapType == EAP_CODE_REQ) {
		EapFrame.Code   = EAP_CODE_REQ;
		EapFrame.Id     = ++(pWscControl->lastId);
	} else {
		EapFrame.Code   = EAP_CODE_RSP;
		EapFrame.Id     = pWscControl->lastId; /* same as eap_req id */
	}

	EapFrame.Length = cpu2be16(Length);
	EapFrame.Type   = EAP_TYPE_WSC;
	/* Zero WSC Frame */
	NdisZeroMemory(&WscFrame, sizeof(WscFrame));
	WscFrame.SMI[0] = 0x00;
	WscFrame.SMI[1] = 0x37;
	WscFrame.SMI[2] = 0x2A;
	WscFrame.VendorType = cpu2be32(WSC_VENDOR_TYPE);
	WscFrame.OpCode = OpCode;
	WscFrame.Flags  = 0x00;

	if (pWscControl->bWscFragment && (pWscControl->bWscLastOne == FALSE))
		WscFrame.Flags  |= WSC_MSG_FLAG_MF;

	if (pWscControl->bWscFragment && (pWscControl->bWscFirstOne))
		WscFrame.Flags  |= WSC_MSG_FLAG_LF;

	/* Out buffer for transmitting message */
	os_alloc_mem(NULL, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);

	if (pOutBuffer == NULL)
		return;

	FrameLen = 0;

	/* Make	 Transmitting frame */
	if (pData && (Len > 0)) {
		if (pWscControl->bWscFragment && (pWscControl->bWscFirstOne)) {
			UCHAR	LF_Len = 2;
			ULONG	TmpLen = 0;

			pWscControl->bWscFirstOne = FALSE;
			MakeOutgoingFrame(pOutBuffer, &TmpLen,
							  sizeof(IEEE8021X_FRAME), &Ieee_8021x,
							  sizeof(EapFrame), &EapFrame,
							  sizeof(WscFrame), &WscFrame,
							  LF_Len, &MsgLen,
							  END_OF_ARGS);
			FrameLen += TmpLen;
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  Len, pData,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		} else {
			MakeOutgoingFrame(pOutBuffer, &FrameLen,
							  sizeof(IEEE8021X_FRAME), &Ieee_8021x,
							  sizeof(EapFrame), &EapFrame,
							  sizeof(WscFrame), &WscFrame,
							  Len, pData,
							  END_OF_ARGS);
		}
	} else
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(IEEE8021X_FRAME), &Ieee_8021x,
						  sizeof(EapFrame), &EapFrame,
						  sizeof(WscFrame), &WscFrame,
						  END_OF_ARGS);

	/* Copy frame to Tx ring */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter)
	pEntry = MacTableLookup(pAdapter, &pWscControl->EntryAddr[0]);
#endif
#ifdef CONFIG_STA_SUPPORT
	/* Pat: TODO */
	IF_DEV_CONFIG_OPMODE_ON_STA(pAdapter)
	pEntry = MacTableLookup2(pAdapter, &pWscControl->EntryAddr[0], NULL);
#endif

	if (pEntry)
		RTMPToWirelessSta(pAdapter, pEntry, Header802_3, sizeof(Header802_3), (PUCHAR)pOutBuffer, FrameLen, TRUE);
	else
		MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN, ("pEntry is NULL\n"));

	if (pOutBuffer)
		os_free_mem(pOutBuffer);

	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscSendMessage\n");
}

VOID WscBuildBeaconIE(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR b_configured,
	IN  BOOLEAN b_selRegistrar,
	IN  USHORT devPwdId,
	IN  USHORT selRegCfgMethods,
	IN  UCHAR apidx,
	IN  UCHAR *pAuthorizedMACs,
	IN  UCHAR AuthorizedMACsLen,
	IN  UCHAR CurOpMode)
{
	WSC_IE_HEADER ieHdr;
	UCHAR *Data = NULL;
	PUCHAR pData;
	INT Len = 0, templen = 0;
	USHORT tempVal = 0;
	PWSC_CTRL pWpsCtrl = NULL;
	PWSC_REG_DATA pReg = NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR inf_idx;
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&Data, 256);

	if (Data == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "Allocate memory fail!!!\n");
		return;
	}

	inf_idx = apidx & 0x1F;
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (VALID_MBSS(pAd, inf_idx))
			wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE)
		wdev = &pAd->StaCfg[inf_idx].wdev;

#endif /* CONFIG_STA_SUPPORT */

	if (wdev)
		pWpsCtrl = &wdev->WscControl;

	if (!pWpsCtrl || !wdev) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 "pWpsCtrl or wdev is NULL!!!\n");
		os_free_mem(Data);
		return;
	}

	pReg = &pWpsCtrl->RegData;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscBuildBeaconIE\n");
	/* WSC IE HEader */
	ieHdr.elemId = 221;
	ieHdr.length = 4;
	ieHdr.oui[0] = 0x00;
	ieHdr.oui[1] = 0x50;
	ieHdr.oui[2] = 0xF2;
#ifdef IWSC_SUPPORT

	if ((CurOpMode == STA_MODE) &&
		(pAd->StaCfg[inf_idx].BssType == BSS_ADHOC))
		ieHdr.oui[3] = 0x10;
	else
#endif /* IWSC_SUPPORT */
		ieHdr.oui[3] = 0x04;

	pData = (PUCHAR) &Data[0];
	Len = 0;
	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Simple Config State */
	templen = AppendWSCTLV(WSC_ID_SC_STATE, pData, (UINT8 *)&b_configured, 0);
	pData += templen;
	Len   += templen;
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

	if ((CurOpMode == AP_MODE) && pWpsCtrl->bSetupLock) {
		/* AP Setup Lock */
		templen = AppendWSCTLV(WSC_ID_AP_SETUP_LOCKED, pData, (UINT8 *)&pWpsCtrl->bSetupLock, 0);
		pData += templen;
		Len   += templen;
	}

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	if (b_selRegistrar) {
		/* 3.Selected Registrar */
		templen = AppendWSCTLV(WSC_ID_SEL_REGISTRAR, pData, (UINT8 *)&b_selRegistrar, 0);
		pData += templen;
		Len   += templen;
		/*4. Device Password ID */
		tempVal = htons(devPwdId);
		templen = AppendWSCTLV(WSC_ID_DEVICE_PWD_ID, pData, (UINT8 *)&tempVal, 0);
		pData += templen;
		Len   += templen;
		/* 5. Selected Registrar Config Methods */
		tempVal = selRegCfgMethods;
#ifdef IWSC_SUPPORT

		if (CurOpMode == STA_MODE) {
			if (pWpsCtrl->WscMode == WSC_PIN_MODE)
				tempVal &= 0x200F;
			else
				tempVal &= 0x02F0;

			if (wdev->IWscInfo.bLimitedUI)
				tempVal &= (~WSC_CONFMET_KEYPAD);
			else
				tempVal |= WSC_CONFMET_KEYPAD;
		}

#endif /* IWSC_SUPPORT */
		tempVal = htons(tempVal);
		templen = AppendWSCTLV(WSC_ID_SEL_REG_CFG_METHODS, pData, (UINT8 *)&tempVal, 0);
		pData += templen;
		Len   += templen;
	}

	/* 6. UUID last 6 bytes use MAC */
	templen = AppendWSCTLV(WSC_ID_UUID_E, pData, &pWpsCtrl->Wsc_Uuid_E[0], 0);
	pData += templen;
	Len   += templen;
	/* 7. RF Bands */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (wdev && WMODE_CAP_5G(wdev->PhyMode))
			tempVal = 2;
		else
			tempVal = 1;
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		if (wdev && wdev->PhyMode > 14)
			tempVal = 2;
		else
			tempVal = 1;
	}

#endif /* CONFIG_STA_SUPPORT */
#ifdef RT_BIG_ENDIAN
	tempVal = SWAP16(tempVal);
#endif /* RT_BIG_ENDIAN */
	templen = AppendWSCTLV(WSC_ID_RF_BAND, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
#ifdef IWSC_SUPPORT

	if ((pAd->StaCfg[inf_idx].BssType == BSS_ADHOC) &&
		(CurOpMode == STA_MODE)) {
		UCHAR respType;
		/* Connection Type Flag ESS */
		templen = AppendWSCTLV(WSC_ID_CONN_TYPE, pData, (UINT8 *)&pReg->SelfInfo.ConnTypeFlags, 0);
		pData += templen;
		Len   += templen;
#ifdef IWSC_TEST_SUPPORT

		/*
		*	This modification is for Broadcom test bed.
		*	Broadcom test bed use same buffer to record IWSC IE from Beacon and Probe Response.
		*	But the content of IWSC IE in Beacon is different from Probe Response.
		*/
		if ((pWpsCtrl->WscMode == WSC_SMPBC_MODE) &&
			(pWpsCtrl->WscConfMode == WSC_REGISTRAR)) {
			BOOLEAN bEntryAcceptable = FALSE;
			BOOLEAN bRegistrationReady = TRUE;
			PIWSC_INFO pIWscInfo = NULL;

			pIWscInfo = &wdev->IWscInfo;

			if (pIWscInfo->bIWscEntryTimerRunning)
				bEntryAcceptable = TRUE;

			/* Entry Acceptable (only for IBSS) */
			templen = AppendWSCTLV(WSC_ID_ENTRY_ACCEPTABLE, pData, (UINT8 *)&bEntryAcceptable, 0);
			pData += templen;
			Len   += templen;

			if (pWpsCtrl->EapMsgRunning)
				bRegistrationReady = FALSE;

			/* Registration Ready (only for IBSS) */
			templen = AppendWSCTLV(WSC_ID_REGISTRATON_READY, pData, (UINT8 *)&bRegistrationReady, 0);
			pData += templen;
			Len   += templen;
		}

#endif /* IWSC_TEST_SUPPORT */
		/* IWSC IP Address Configuration */
		tempVal = htons(wdev->IWscInfo.IpConfMethod);
		templen = AppendWSCTLV(WSC_ID_IP_ADDR_CONF_METHOD, pData, (UINT8 *)&tempVal, 0);
		pData += templen;
		Len   += templen;
#ifdef IWSC_TEST_SUPPORT
		/*
		*	This modification is for Broadcom test bed.
		*	Broadcom test bed use same buffer to record IWSC IE from Beacon and Probe Response.
		*	But the content of IWSC IE in Beacon is different from Probe Response.
		*/

		/* Response Type WSC_ID_RESP_TYPE */
		if (pWpsCtrl->WscConfMode == WSC_REGISTRAR)
			respType = WSC_MSGTYPE_REGISTRAR;
		else
			respType = WSC_MSGTYPE_ENROLLEE_OPEN_8021X;

		templen = AppendWSCTLV(WSC_ID_RESP_TYPE, pData, (UINT8 *)&respType, 0);
		pData += templen;
		Len   += templen;
#endif /* IWSC_TEST_SUPPORT */
	}

#endif /* IWSC_SUPPORT */
#ifdef WSC_V2_SUPPORT

	if (pWpsCtrl->WscV2Info.bEnableWpsV2) {
		PWSC_TLV pWscTLV = &pWpsCtrl->WscV2Info.ExtraTlv;

		WscGenV2Msg(pWpsCtrl,
					b_selRegistrar,
					pAuthorizedMACs,
					AuthorizedMACsLen,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */
#ifdef P2P_SUPPORT
	/* 12. Primary Device Type */
	templen = AppendWSCTLV(WSC_ID_PRIM_DEV_TYPE, pData, pReg->SelfInfo.PriDeviceType, 0);
	pData += templen;
	Len   += templen;
	/* 13. Device Name */
	NdisZeroMemory(pData, 32 + 4);
	templen = AppendWSCTLV(WSC_ID_DEVICE_NAME, pData, &pAd->P2pCfg.DeviceName, pAd->P2pCfg.DeviceNameLen);
	pData += templen;
	Len   += templen;
#endif /* P2P_SUPPORT */
	ieHdr.length = ieHdr.length + Len;
	NdisCopyMemory(wdev->WscIEBeacon.Value, &ieHdr, sizeof(WSC_IE_HEADER));
	NdisCopyMemory(wdev->WscIEBeacon.Value + sizeof(WSC_IE_HEADER), Data, Len);
	wdev->WscIEBeacon.ValueLen = sizeof(WSC_IE_HEADER) + Len;

	if (Data != NULL)
		os_free_mem(Data);

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscBuildBeaconIE\n");
}

VOID WscBuildProbeRespIE(
	IN  PRTMP_ADAPTER	pAd,
	IN  UCHAR respType,
	IN  UCHAR scState,
	IN  BOOLEAN b_selRegistrar,
	IN  USHORT devPwdId,
	IN  USHORT selRegCfgMethods,
	IN  UCHAR apidx,
	IN  UCHAR *pAuthorizedMACs,
	IN  INT AuthorizedMACsLen,
	IN  UCHAR CurOpMode)
{
	WSC_IE_HEADER ieHdr;
	UCHAR *Data = NULL;
	PUCHAR pData;
	INT Len = 0, templen = 0;
	USHORT tempVal = 0;
	PWSC_CTRL pWpsCtrl = NULL;
	PWSC_REG_DATA pReg = NULL;
	struct wifi_dev *wdev = NULL;
	UCHAR inf_idx;
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&Data, 512);

	if (Data == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 "Allocate memory fail!!!\n");
		return;
	}

	inf_idx = apidx & 0x1F;
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE)
		wdev = &pAd->ApCfg.MBSSID[inf_idx].wdev;

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE)
		wdev = &pAd->StaCfg[inf_idx].wdev;

#endif /* CONFIG_STA_SUPPORT */

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				 "Wdev is NULL!!!\n");
		os_free_mem(Data);
		return;
	}

	pWpsCtrl = &wdev->WscControl;
	pReg = &pWpsCtrl->RegData;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			 "-----> %s:: apidx = %x\n", __func__, apidx);
	/* WSC IE Header */
	ieHdr.elemId = 221;
	ieHdr.length = 4;
	ieHdr.oui[0] = 0x00;
	ieHdr.oui[1] = 0x50;
	ieHdr.oui[2] = 0xF2;
#ifdef IWSC_SUPPORT

	if ((CurOpMode == STA_MODE) &&
		(pAd->StaCfg[inf_idx].BssType == BSS_ADHOC))
		ieHdr.oui[3] = 0x10;
	else
#endif /* IWSC_SUPPORT */
		ieHdr.oui[3] = 0x04;

	pData = (PUCHAR) &Data[0];
	Len = 0;
	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* 2. Simple Config State */
	templen = AppendWSCTLV(WSC_ID_SC_STATE, pData, (UINT8 *)&scState, 0);
	pData += templen;
	Len   += templen;
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

	if ((CurOpMode == AP_MODE) && pWpsCtrl->bSetupLock) {
		/* AP Setup Lock */
		templen = AppendWSCTLV(WSC_ID_AP_SETUP_LOCKED, pData, (UINT8 *)&pWpsCtrl->bSetupLock, 0);
		pData += templen;
		Len   += templen;
	}

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	if (b_selRegistrar) {
		/* 3. Selected Registrar */
		templen = AppendWSCTLV(WSC_ID_SEL_REGISTRAR, pData, (UINT8 *)&b_selRegistrar, 0);
		pData += templen;
		Len   += templen;
		/* 4. Device Password ID */
		tempVal = htons(devPwdId);
		templen = AppendWSCTLV(WSC_ID_DEVICE_PWD_ID, pData, (UINT8 *)&tempVal, 0);
		pData += templen;
		Len   += templen;
		/* 5. Selected Registrar Config Methods */
#ifdef IWSC_SUPPORT

		if ((CurOpMode == STA_MODE) &&
			(wdev->IWscInfo.bSelRegStart == FALSE)) {
			if (pWpsCtrl->WscMode == WSC_PIN_MODE) {
				selRegCfgMethods &= 0x200F;

				if (wdev->IWscInfo.bLimitedUI)
					selRegCfgMethods &= (~WSC_CONFMET_KEYPAD);
				else
					selRegCfgMethods |= WSC_CONFMET_KEYPAD;
			} else
				selRegCfgMethods &= 0x02F0;
		}

#endif /* IWSC_SUPPORT */
		tempVal = htons(selRegCfgMethods);
		templen = AppendWSCTLV(WSC_ID_SEL_REG_CFG_METHODS, pData, (UINT8 *)&tempVal, 0);
		pData += templen;
		Len   += templen;
#ifdef IWSC_SUPPORT

		if ((CurOpMode == STA_MODE) && wdev->IWscInfo.bSelRegStart) {
			templen = AppendWSCTLV(WSC_ID_MAC_ADDR, pData, wdev->IWscInfo.RegMacAddr, 0);
			pData += templen;
			Len   += templen;
			wdev->IWscInfo.bSelRegStart = FALSE;
		}

#endif /* IWSC_SUPPORT */
	}

	/* 6. Response Type WSC_ID_RESP_TYPE */
	templen = AppendWSCTLV(WSC_ID_RESP_TYPE, pData, (UINT8 *)&respType, 0);
	pData += templen;
	Len   += templen;
	/* 7. UUID last 6 bytes use MAC */
	templen = AppendWSCTLV(WSC_ID_UUID_E, pData, &pWpsCtrl->Wsc_Uuid_E[0], 0);
	pData += templen;
	Len   += templen;
	/* 8. Manufacturer */
	NdisZeroMemory(pData, 64 + 4);
	templen = AppendWSCTLV(WSC_ID_MANUFACTURER, pData,  pReg->SelfInfo.Manufacturer, strlen((RTMP_STRING *) pReg->SelfInfo.Manufacturer));
	pData += templen;
	Len   += templen;
	/* 9. Model Name */
	NdisZeroMemory(pData, 32 + 4);
	templen = AppendWSCTLV(WSC_ID_MODEL_NAME, pData, pReg->SelfInfo.ModelName, strlen((RTMP_STRING *) pReg->SelfInfo.ModelName));
	pData += templen;
	Len   += templen;
	/* 10. Model Number */
	NdisZeroMemory(pData, 32 + 4);
	templen = AppendWSCTLV(WSC_ID_MODEL_NUMBER, pData, pReg->SelfInfo.ModelNumber, strlen((RTMP_STRING *) pReg->SelfInfo.ModelNumber));
	pData += templen;
	Len   += templen;
	/* 11. Serial Number */
	NdisZeroMemory(pData, 32 + 4);
	templen = AppendWSCTLV(WSC_ID_SERIAL_NUM, pData, pReg->SelfInfo.SerialNumber, strlen((RTMP_STRING *) pReg->SelfInfo.SerialNumber));
	pData += templen;
	Len   += templen;
	/* 12. Primary Device Type */
	if ((CurOpMode == AP_MODE) && b_selRegistrar) {
		NdisMoveMemory(&pReg->SelfInfo.PriDeviceType, &AP_Wsc_Pri_Dev_Type[0], 8);
		templen = AppendWSCTLV(WSC_ID_PRIM_DEV_TYPE, pData, pReg->SelfInfo.PriDeviceType, strlen((RTMP_STRING *) pReg->SelfInfo.PriDeviceType));
	} else
			templen = AppendWSCTLV(WSC_ID_PRIM_DEV_TYPE, pData, pReg->SelfInfo.PriDeviceType, 0);
	pData += templen;
	Len   += templen;
	/* 13. Device Name */
	NdisZeroMemory(pData, 32 + 4);
	templen = AppendWSCTLV(WSC_ID_DEVICE_NAME, pData, pReg->SelfInfo.DeviceName, strlen((RTMP_STRING *) pReg->SelfInfo.DeviceName));
	pData += templen;
	Len   += templen;
	/* 14. Config Methods */
	/*tempVal = htons(0x008a); */
	/*tempVal = htons(0x0084); */
#ifdef P2P_SUPPORT

	/*
	*	Some P2P Device will check this config method for PBC. (ex. Samsung GALAXYSII)
	*	If this config method doesn't include PBC, some P2P Device doesn't send provision request if we are P2P GO.
	*/
	if (apidx >= MIN_NET_DEVICE_FOR_P2P_GO) {
		if (pAd->P2pCfg.bSigmaEnabled == TRUE)
			/*
			*	P2P test plan 1.0.3 item 4.2.2.
			*	WSC IE does not have Push Button flag
			*	set ON in Config Method attribute.
			*/
			tempVal = pWpsCtrl->WscConfigMethods & 0xff7f;
		else
			tempVal = pWpsCtrl->WscConfigMethods;
	} else
#endif /* P2P_SUPPORT */
	{
		/*
		*	WSC 1.0 WCN logo testing will check the value of config method in probe response and M1.
		*	Config method shall be identical in probe response and M1.
		*/
#ifdef WSC_V2_SUPPORT
		if (pWpsCtrl->WscV2Info.bEnableWpsV2)
			tempVal = pWpsCtrl->WscConfigMethods & 0xF97F;
		else
#endif /* WSC_V2_SUPPORT */
			tempVal = pWpsCtrl->WscConfigMethods & 0x00FF;

		/*
		*	WSC 1.0 should set Config Methods ie for External Registrar.
		*/
		if (pReg->SelfInfo.Version == 0x10) {
			if (pWpsCtrl->WscMode == WSC_PIN_MODE)
				tempVal |= 0x04;
			else if (pWpsCtrl->WscMode == WSC_PBC_MODE)
				tempVal |= 0x8c;
		}
	}

	tempVal = htons(tempVal);
	templen = AppendWSCTLV(WSC_ID_CONFIG_METHODS, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
	/* 15. RF Bands */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (WMODE_CAP_5G(wdev->PhyMode))
			tempVal = 2;
		else
			tempVal = 1;
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		if (wdev->channel > 14)
			tempVal = 2;
		else
			tempVal = 1;
	}

#endif /* CONFIG_STA_SUPPORT */
#ifdef RT_BIG_ENDIAN
	tempVal = SWAP16(tempVal);
#endif /* RT_BIG_ENDIAN */
	templen = AppendWSCTLV(WSC_ID_RF_BAND, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
#ifdef IWSC_SUPPORT

	if ((pAd->StaCfg[inf_idx].BssType == BSS_ADHOC) &&
		(CurOpMode == STA_MODE)) {
		/* Connection Type Flag ESS */
		templen = AppendWSCTLV(WSC_ID_CONN_TYPE, pData, (UINT8 *)&pReg->SelfInfo.ConnTypeFlags, 0);
		pData += templen;
		Len   += templen;

		if ((pWpsCtrl->WscMode == WSC_SMPBC_MODE) &&
			(pWpsCtrl->WscConfMode == WSC_REGISTRAR)) {
			BOOLEAN bEntryAcceptable = FALSE;
			BOOLEAN bRegistrationReady = TRUE;
			PIWSC_INFO pIWscInfo = NULL;

			pIWscInfo = &wdev->IWscInfo;

			if (pIWscInfo->bIWscEntryTimerRunning)
				bEntryAcceptable = TRUE;

			/* Entry Acceptable (only for IBSS) */
			templen = AppendWSCTLV(WSC_ID_ENTRY_ACCEPTABLE, pData, (UINT8 *)&bEntryAcceptable, 0);
			pData += templen;
			Len   += templen;

			if (pWpsCtrl->EapMsgRunning)
				bRegistrationReady = FALSE;

			/* Registration Ready (only for IBSS) */
			templen = AppendWSCTLV(WSC_ID_REGISTRATON_READY, pData, (UINT8 *)&bRegistrationReady, 0);
			pData += templen;
			Len   += templen;
		}

		/* IWSC IP Address Configuration */
		tempVal = htons(wdev->IWscInfo.IpConfMethod);
		templen = AppendWSCTLV(WSC_ID_IP_ADDR_CONF_METHOD, pData, (UINT8 *)&tempVal, 0);
		pData += templen;
		Len   += templen;
	}

#endif /* IWSC_SUPPORT */
#ifdef WSC_V2_SUPPORT

	if (pWpsCtrl->WscV2Info.bEnableWpsV2) {
		PWSC_TLV pWscTLV = &pWpsCtrl->WscV2Info.ExtraTlv;

		WscGenV2Msg(pWpsCtrl,
					b_selRegistrar,
					pAuthorizedMACs,
					AuthorizedMACsLen,
					&pData,
					&Len);

		/* Extra attribute that is not defined in WSC Sepc. */
		if (pWscTLV->pTlvData && pWscTLV->TlvLen) {
			templen = AppendWSCTLV(pWscTLV->TlvTag, pData, (UINT8 *)pWscTLV->pTlvData, pWscTLV->TlvLen);
			pData += templen;
			Len   += templen;
		}
	}

#endif /* WSC_V2_SUPPORT */

	if (Len > 251)
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Len is overflow!\n");

	ieHdr.length = ieHdr.length + Len;
	NdisCopyMemory(wdev->WscIEProbeResp.Value, &ieHdr, sizeof(WSC_IE_HEADER));
	NdisCopyMemory(wdev->WscIEProbeResp.Value + sizeof(WSC_IE_HEADER), Data, Len);
	wdev->WscIEProbeResp.ValueLen = sizeof(WSC_IE_HEADER) + Len;

	if (Data != NULL)
		os_free_mem(Data);

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			 "<----- %s\n", __func__);
}

/*
*	========================================================================
*
*	Routine Description:
*		Ap send EAP-Fail to station
*
*	Arguments:
*		pAd    - NIC Adapter pointer
*		Id			- ID between EAP-Req and EAP-Rsp pair
*		pEntry		- The Station Entry information
*
*	Return Value:
*		None
*
*	========================================================================
*/
VOID WscSendEapFail(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl,
	IN  BOOLEAN bSendDeAuth)
{
	UCHAR Header802_3[14];
	USHORT Length;
	IEEE8021X_FRAME Ieee_8021x;
	EAP_FRAME EapFrame;
	UCHAR *pOutBuffer = NULL;
	ULONG FrameLen = 0;
#ifdef CONFIG_AP_SUPPORT
	UCHAR apidx = (pWscControl->EntryIfIdx & 0x1F);
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR CurOpMode = 0xFF;

	NdisZeroMemory(Header802_3, sizeof(UCHAR) * 14);
	/* 1. Send EAP-Rsp Id */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			 "-----> WscSendEapFail\n");
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	CurOpMode = STA_MODE;
#ifdef P2P_SUPPORT

	if (pWscControl->EntryIfIdx != BSS0)
		CurOpMode = AP_MODE;

#endif /* P2P_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		MAKE_802_3_HEADER(Header802_3,
						  &pWscControl->EntryAddr[0],
						  &pAd->ApCfg.MBSSID[apidx].wdev.bssid[0],
						  EAPOL);
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		MAKE_802_3_HEADER(Header802_3,
						  &pWscControl->EntryAddr[0],
						  &pAd->StaCfg[0].wdev.if_addr[0],
						  EAPOL);
	}

#endif /* CONFIG_STA_SUPPORT */
	/* Length, -1 type size, Eap-Fail doesn't need Type item */
	Length = sizeof(EAP_FRAME) - sizeof(UCHAR);
	/* Zero 802.1x body */
	NdisZeroMemory(&Ieee_8021x, sizeof(Ieee_8021x));
	Ieee_8021x.Version = EAPOL_VER;
	Ieee_8021x.Type    = EAPPacket;
	Ieee_8021x.Length  = cpu2be16(Length);
	/* Zero EAP frame */
	NdisZeroMemory(&EapFrame, sizeof(EapFrame));
	EapFrame.Code   = EAP_CODE_FAIL;
	EapFrame.Id     = pWscControl->lastId;
	EapFrame.Length = cpu2be16(Length);
	/* Out buffer for transmitting EAP-Req(Identity) */
	os_alloc_mem(NULL, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);

	if (pOutBuffer == NULL)
		return;

	FrameLen = 0;
	/* Make	 Transmitting frame */
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(IEEE8021X_FRAME), &Ieee_8021x,
					  sizeof(EapFrame) - 1, &EapFrame, END_OF_ARGS);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	pEntry = MacTableLookup(pAd, &pWscControl->EntryAddr[0]);
#endif
#ifdef CONFIG_STA_SUPPORT
	/* Pat: TODO */
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	pEntry = MacTableLookup2(pAd, &pWscControl->EntryAddr[0], NULL);
#endif
	/* Copy frame to Tx ring */
	RTMPToWirelessSta(pAd, pEntry, Header802_3, sizeof(Header802_3), (PUCHAR)pOutBuffer, FrameLen, TRUE);

	if (pOutBuffer)
		os_free_mem(pOutBuffer);

#ifdef CONFIG_AP_SUPPORT

	if (pEntry && bSendDeAuth && (CurOpMode == AP_MODE))
		MlmeDeAuthAction(pAd, pEntry, REASON_DEAUTH_STA_LEAVING, TRUE);

	if (pEntry == NULL) {
		/*
		*	If STA dis-connect un-normally, reset EntryAddr here.
		*/
		NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN);
	}

#endif /* CONFIG_AP_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscSendEapFail\n");
}

#ifdef CONFIG_AP_SUPPORT
VOID WscBuildAssocRespIE(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR ApIdx,
	IN  UCHAR Reason,
	OUT PUCHAR pOutBuf,
	OUT PUCHAR pIeLen)
{
	WSC_IE_HEADER ieHdr;
	UCHAR *Data = NULL;
	PUCHAR pData;
	INT Len = 0, templen = 0;
	UINT8 tempVal = 0;
	PWSC_REG_DATA pReg;
	struct wifi_dev *wdev;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscBuildAssocRespIE\n");
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&Data, 512);

	if (Data == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "Allocate memory fail!!!\n");
		return;
	}

	wdev = &pAd->ApCfg.MBSSID[ApIdx].wdev;
	pReg = &wdev->WscControl.RegData;
	Data[0] = 0;
	/* WSC IE Header */
	ieHdr.elemId = 221;
	ieHdr.length = 4;
	ieHdr.oui[0] = 0x00;
	ieHdr.oui[1] = 0x50;
	ieHdr.oui[2] = 0xF2;
	ieHdr.oui[3] = 0x04;
	pData = (PUCHAR) &Data[0];
	Len = 0;
	/* Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;
	/* Request Type */
	tempVal = WSC_MSGTYPE_AP_WLAN_MGR;
	templen = AppendWSCTLV(WSC_ID_RESP_TYPE, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (wdev->WscControl.WscV2Info.bEnableWpsV2) {
		WscGenV2Msg(&wdev->WscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);
	}

#endif /* WSC_V2_SUPPORT */
	ieHdr.length = ieHdr.length + Len;
	NdisMoveMemory(pOutBuf, &ieHdr, sizeof(WSC_IE_HEADER));
	NdisMoveMemory(pOutBuf + sizeof(WSC_IE_HEADER), Data, Len);
	*pIeLen = sizeof(WSC_IE_HEADER) + Len;

	if (Data != NULL)
		os_free_mem(Data);

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscBuildAssocRespIE\n");
}


VOID WscSelectedRegistrar(
	IN  PRTMP_ADAPTER pAd,
	IN  PUCHAR pReginfo,
	IN  UINT Length,
	IN  UCHAR apidx)
{
	PUCHAR pData;
	INT IsAPConfigured;
	UCHAR wsc_version, wsc_sel_reg = 0;
	USHORT wsc_dev_pass_id = 0, wsc_sel_reg_conf_mthd = 0;
	USHORT WscType, WscLen;
	PUCHAR pAuthorizedMACs = NULL;
	UCHAR AuthorizedMACsLen = 0;
	PWSC_CTRL pWscCtrl;
	struct wifi_dev *wdev;

	pData = (PUCHAR)pReginfo;

	if (Length < 4) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscSelectedRegistrar --> Unknown IE\n");
		return;
	}

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pWscCtrl = &wdev->WscControl;
	hex_dump("WscSelectedRegistrar - Reginfo", pReginfo, Length);

	while (Length > 4) {
		/* arm-cpu has packet alignment issue, it's better to use memcpy to retrieve data */
		NdisMoveMemory(&WscType, pData, 2);
		NdisMoveMemory(&WscLen,  pData + 2, 2);
		WscLen = ntohs(WscLen);
		if (Length < WscLen + 4) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "unexpected WSC IE Length(%u)\n", WscLen);
			break;
		}
		pData  += 4;
		Length -= 4;

		switch (ntohs(WscType)) {
		case WSC_ID_VERSION:
			wsc_version = *pData;
			break;

		case WSC_ID_SEL_REGISTRAR:
			wsc_sel_reg = *pData;
			break;

		case WSC_ID_DEVICE_PWD_ID:
			NdisMoveMemory(&wsc_dev_pass_id, pData, sizeof(USHORT));
			wsc_dev_pass_id = be2cpu16(wsc_dev_pass_id);
			break;

		case WSC_ID_SEL_REG_CFG_METHODS:
			NdisMoveMemory(&wsc_sel_reg_conf_mthd, pData, sizeof(USHORT));
			wsc_sel_reg_conf_mthd = be2cpu16(wsc_sel_reg_conf_mthd);
			break;

		case WSC_ID_VENDOR_EXT:
#ifdef WSC_V2_SUPPORT
			if (pWscCtrl->WscV2Info.bEnableWpsV2 && (WscLen > 0)) {
				/*
				*	Find WFA_EXT_ID_AUTHORIZEDMACS
				*/
				if (!pAuthorizedMACs)
					os_alloc_mem(NULL, &pAuthorizedMACs, WscLen);

				if (!pAuthorizedMACs) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
						"allocate pAuthorizedMACs memory fail!!\n");
					return;
				}

				NdisZeroMemory(pAuthorizedMACs, WscLen);
				WscParseV2SubItem(WFA_EXT_ID_AUTHORIZEDMACS, pData, WscLen, pAuthorizedMACs, &AuthorizedMACsLen);

				if ((AuthorizedMACsLen > 30) || strlen(pAuthorizedMACs) > 30) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "WscSelectedRegistrar --> AuthorizedMACsLen parse fail!\n");
					os_free_mem(pAuthorizedMACs);
					return;
				}
			}

#endif /* WSC_V2_SUPPORT */
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscSelectedRegistrar --> Unknown IE 0x%04x\n", WscType);
			break;
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	IsAPConfigured = pWscCtrl->WscConfStatus;

	if (wsc_sel_reg == 0x01) {
		pWscCtrl->WscSelReg = 1;
		WscBuildBeaconIE(pAd,
						 WSC_SCSTATE_CONFIGURED,
						 TRUE,
						 wsc_dev_pass_id,
						 wsc_sel_reg_conf_mthd,
						 apidx,
						 pAuthorizedMACs,
						 AuthorizedMACsLen,
						 AP_MODE);
		WscBuildProbeRespIE(pAd,
							WSC_MSGTYPE_AP_WLAN_MGR,
							WSC_SCSTATE_CONFIGURED,
							TRUE,
							wsc_dev_pass_id,
							wsc_sel_reg_conf_mthd,
							pWscCtrl->EntryIfIdx,
							pAuthorizedMACs,
							AuthorizedMACsLen,
							AP_MODE);
#ifdef WSC_V2_SUPPORT
		hex_dump("WscSelectedRegistrar - AuthorizedMACs::",
				 pAuthorizedMACs, AuthorizedMACsLen);

		if ((AuthorizedMACsLen == 6) &&
			(NdisEqualMemory(pAuthorizedMACs, BROADCAST_ADDR, MAC_ADDR_LEN) == FALSE) &&
			(NdisEqualMemory(pAuthorizedMACs, ZERO_MAC_ADDR, MAC_ADDR_LEN) == FALSE) &&
			(pWscCtrl->WscState <= WSC_STATE_WAIT_M3)) {
			PWSC_PEER_ENTRY	pWscPeer = NULL;

			NdisMoveMemory(pWscCtrl->EntryAddr, pAuthorizedMACs, MAC_ADDR_LEN);
			RTMP_SEM_LOCK(&pWscCtrl->WscPeerListSemLock);
			WscClearPeerList(&pWscCtrl->WscPeerList);
			os_alloc_mem(pAd, (UCHAR **)&pWscPeer, sizeof(WSC_PEER_ENTRY));

			if (pWscPeer) {
				NdisZeroMemory(pWscPeer, sizeof(WSC_PEER_ENTRY));
				NdisMoveMemory(pWscPeer->mac_addr, pAuthorizedMACs, MAC_ADDR_LEN);
				NdisGetSystemUpTime(&pWscPeer->receive_time);
				insertTailList(&pWscCtrl->WscPeerList,
							   (RT_LIST_ENTRY *)pWscPeer);
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
						 "WscSelectedRegistrar --> Add this MAC to WscPeerList\n");
			}

			ASSERT(pWscPeer != NULL);
			RTMP_SEM_UNLOCK(&pWscCtrl->WscPeerListSemLock);
		}

#endif /* WSC_V2_SUPPORT */
	} else {
		pWscCtrl->WscSelReg = 0;
		WscBuildBeaconIE(pAd,
						 WSC_SCSTATE_CONFIGURED,
						 FALSE, 0, 0, apidx, NULL, 0, AP_MODE);
		WscBuildProbeRespIE(pAd,
							WSC_MSGTYPE_AP_WLAN_MGR,
							WSC_SCSTATE_CONFIGURED,
							FALSE, 0, 0, pWscCtrl->EntryIfIdx, NULL, 0, AP_MODE);
	}

	UpdateBeaconHandler(
		pAd,
		wdev,
		BCN_UPDATE_IE_CHG);
#ifdef WSC_V2_SUPPORT

	if (pAuthorizedMACs)
		os_free_mem(pAuthorizedMACs);

#endif /* WSC_V2_SUPPORT */
}
#endif /* CONFIG_AP_SUPPORT */

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
/*
*	========================================================================
*
*	Routine Description:
*		Make WSC IE for the ProbeReq frame
*
*	Arguments:
*		pAd    - NIC Adapter pointer
*		pOutBuf		- all of WSC IE field
*		pIeLen		- length
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		None
*
*	========================================================================
*/
VOID WscBuildProbeReqIE(
	IN  RTMP_ADAPTER *pAd,
	IN  VOID *wdev_obj,
	OUT PUCHAR pOutBuf,
	OUT PUCHAR pIeLen)
{
	WSC_IE_HEADER ieHdr;
	UCHAR *OutMsgBuf = NULL; /* buffer to create message contents */
	INT Len = 0, templen = 0;
	PUCHAR pData;
	USHORT tempVal = 0;
	struct wifi_dev *wdev;
	PWSC_REG_DATA	pReg;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG sta_cfg;
#endif /* CONFIG_STA_SUPPORT */
	WSC_CTRL *wsc_ctrl;

	wdev = (struct wifi_dev *)wdev_obj;
	wsc_ctrl = &wdev->WscControl;
	pReg = &wsc_ctrl->RegData;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_DEBUG, "-----> WscBuildProbeReqIE\n");
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&OutMsgBuf, 512);

	if (OutMsgBuf == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "Allocate memory fail!!!\n");
		return;
	}

#ifdef CONFIG_STA_SUPPORT
	sta_cfg = GetStaCfgByWdev(pAd, wdev);
#endif /* CONFIG_STA_SUPPORT */
	/* WSC IE Header */
	ieHdr.elemId = 221;
	ieHdr.length = 4;
	ieHdr.oui[0] = 0x00;
	ieHdr.oui[1] = 0x50;
	ieHdr.oui[2] = 0xF2;
#ifdef IWSC_SUPPORT

	if (sta_cfg->BssType == BSS_ADHOC)
		ieHdr.oui[3] = 0x10;
	else
#endif /* IWSC_SUPPORT */
		ieHdr.oui[3] = 0x04;

	pData = (PUCHAR) &OutMsgBuf[0];
	Len = 0;
	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;

	/* 2. Request Type */
	if (wsc_ctrl->WscConfMode == WSC_REGISTRAR)
		tempVal = WSC_MSGTYPE_REGISTRAR;
	else if (wsc_ctrl->WscConfMode == WSC_ENROLLEE)
		tempVal = WSC_MSGTYPE_ENROLLEE_OPEN_8021X;
	else
		tempVal = WSC_MSGTYPE_ENROLLEE_INFO_ONLY;

	templen = AppendWSCTLV(WSC_ID_REQ_TYPE, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
	/* 3. Config method */
#ifdef WSC_V2_SUPPORT

	if (wsc_ctrl->WscV2Info.bEnableWpsV2)
		tempVal = wsc_ctrl->WscConfigMethods;
	else
#endif /* WSC_V2_SUPPORT */
	{
		tempVal = (wsc_ctrl->WscConfigMethods & 0x00FF);
	}

#ifdef IWSC_SUPPORT

	if (sta_cfg->BssType == BSS_ADHOC) {
		if (wdev->IWscInfo.bLimitedUI)
			tempVal &= (~WSC_CONFMET_KEYPAD);
		else
			tempVal |= WSC_CONFMET_KEYPAD;
	}

#endif /* IWSC_SUPPORT */
	tempVal = cpu2be16(tempVal);
	templen = AppendWSCTLV(WSC_ID_CONFIG_METHODS, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
	/* 4. UUID */
	templen = AppendWSCTLV(WSC_ID_UUID_E, pData, pReg->SelfInfo.Uuid, 0);
	pData += templen;
	Len   += templen;
	/* 5. Primary device type */
	templen = AppendWSCTLV(WSC_ID_PRIM_DEV_TYPE, pData, pReg->SelfInfo.PriDeviceType, 0);
	pData += templen;
	Len   += templen;
	/* 6. RF band, shall change based on current channel */
	templen = AppendWSCTLV(WSC_ID_RF_BAND, pData, &pReg->SelfInfo.RfBand, 0);
	pData += templen;
	Len   += templen;
	/* 7. Associate state */
	tempVal = pReg->SelfInfo.AssocState;
	templen = AppendWSCTLV(WSC_ID_ASSOC_STATE, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
	/* 8. Config error */
	tempVal = pReg->SelfInfo.ConfigError;
	templen = AppendWSCTLV(WSC_ID_CONFIG_ERROR, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
	/* 9. Device password ID */
	tempVal = pReg->SelfInfo.DevPwdId;
	templen = AppendWSCTLV(WSC_ID_DEVICE_PWD_ID, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
#ifdef IWSC_SUPPORT

	if (sta_cfg->BssType == BSS_ADHOC) {
		/* Connection Type Flag ESS */
		templen = AppendWSCTLV(WSC_ID_CONN_TYPE, pData, (UINT8 *)&pReg->SelfInfo.ConnTypeFlags, 0);
		pData += templen;
		Len   += templen;
		/* Connection Type Flag ESS */
		tempVal = htons(wdev->IWscInfo.IpMethod);
		templen = AppendWSCTLV(WSC_ID_IP_ADDR_CONF_METHOD, pData, (UINT8 *)&tempVal, 0);
		pData += templen;
		Len   += templen;
	}

#endif /* IWSC_SUPPORT */
#ifdef WSC_V2_SUPPORT

	if (wsc_ctrl->WscV2Info.bEnableWpsV2) {
		/* 10. Manufacturer */
		NdisZeroMemory(pData, 64 + 4);
		templen = AppendWSCTLV(WSC_ID_MANUFACTURER, pData,  pReg->SelfInfo.Manufacturer, strlen((RTMP_STRING *) pReg->SelfInfo.Manufacturer));
		pData += templen;
		Len   += templen;
		/* 11. Model Name */
		NdisZeroMemory(pData, 32 + 4);
		templen = AppendWSCTLV(WSC_ID_MODEL_NAME, pData, pReg->SelfInfo.ModelName, strlen((RTMP_STRING *) pReg->SelfInfo.ModelName));
		pData += templen;
		Len   += templen;
		/* 12. Model Number */
		NdisZeroMemory(pData, 32 + 4);
		templen = AppendWSCTLV(WSC_ID_MODEL_NUMBER, pData, pReg->SelfInfo.ModelNumber, strlen((RTMP_STRING *) pReg->SelfInfo.ModelNumber));
		pData += templen;
		Len   += templen;
		/* 13. Device Name */
		NdisZeroMemory(pData, 32 + 4);
		templen = AppendWSCTLV(WSC_ID_DEVICE_NAME, pData, pReg->SelfInfo.DeviceName, strlen((RTMP_STRING *) pReg->SelfInfo.DeviceName));
		pData += templen;
		Len   += templen;
		/* Version2 */
		WscGenV2Msg(wsc_ctrl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);
	}

#endif /* WSC_V2_SUPPORT */
	ieHdr.length = ieHdr.length + Len;
	RTMPMoveMemory(pOutBuf, &ieHdr, sizeof(WSC_IE_HEADER));
	RTMPMoveMemory(pOutBuf + sizeof(WSC_IE_HEADER), OutMsgBuf, Len);
	*pIeLen = sizeof(WSC_IE_HEADER) + Len;

	if (OutMsgBuf != NULL)
		os_free_mem(OutMsgBuf);

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_DEBUG, "<----- WscBuildProbeReqIE\n");
}


/*
*	========================================================================
*
*	Routine Description:
*		Make WSC IE for the AssocReq frame
*
*	Arguments:
*		pAd    - NIC Adapter pointer
*		pOutBuf		- all of WSC IE field
*		pIeLen		- length
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		None
*
*	========================================================================
*/
VOID WscBuildAssocReqIE(
	IN  PWSC_CTRL pWscControl,
	OUT PUCHAR pOutBuf,
	OUT PUCHAR pIeLen)
{
	WSC_IE_HEADER ieHdr;
	UCHAR *Data = NULL;
	PUCHAR pData;
	INT Len = 0, templen = 0;
	UINT8 tempVal = 0;
	PWSC_REG_DATA pReg = (PWSC_REG_DATA) &pWscControl->RegData;

	if (pWscControl == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "WscBuildAssocReqIE: pWscControl is NULL\n");
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscBuildAssocReqIE\n");
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&Data, 512);

	if (Data == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "Allocate memory fail!!!\n");
		return;
	}

	/* WSC IE Header */
	ieHdr.elemId = 221;
	ieHdr.length = 4;
	ieHdr.oui[0] = 0x00;
	ieHdr.oui[1] = 0x50;
	ieHdr.oui[2] = 0xF2;
	ieHdr.oui[3] = 0x04;
	pData = (PUCHAR) &Data[0];
	Len = 0;
	/* Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;

	/* Request Type */
	if (pWscControl->WscConfMode == WSC_ENROLLEE)
		tempVal = WSC_MSGTYPE_ENROLLEE_INFO_ONLY;
	else
		tempVal = WSC_MSGTYPE_REGISTRAR;

	templen = AppendWSCTLV(WSC_ID_REQ_TYPE, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		/* Version2 */
		WscGenV2Msg(pWscControl,
					FALSE,
					NULL,
					0,
					&pData,
					&Len);
	}

#endif /* WSC_V2_SUPPORT */
	ieHdr.length = ieHdr.length + Len;
	RTMPMoveMemory(pOutBuf, &ieHdr, sizeof(WSC_IE_HEADER));
	RTMPMoveMemory(pOutBuf + sizeof(WSC_IE_HEADER), Data, Len);
	*pIeLen = sizeof(WSC_IE_HEADER) + Len;

	if (Data != NULL)
		os_free_mem(Data);

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscBuildAssocReqIE\n");
}
#endif /* defined(CONFIG_STA_SUPPORT) || defined (APCLI_SUPPORT) */

#ifdef CONFIG_STA_SUPPORT
#ifdef WIDI_SUPPORT
/*
*	========================================================================
*
*	Routine Description:
*		Make WSC IE for the ProbeReq frame with L2SD_TA Vendor Ext
*
*	Arguments:
*		pAdapter    - NIC Adapter pointer
*		pOutBuf		- all of WSC IE field
*		pIeLen		- length
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		None
*
*	========================================================================
*/
VOID WscMakeProbeReqIEWithVendorExt(
	IN  PRTMP_ADAPTER pAd,
	IN  PUCHAR pDeviceName,
	IN  PUCHAR pPrimaryDeviceType,
	IN  PUCHAR pVendExt,
	IN  USHORT vendExtLen,
	OUT PUCHAR pOutBuf,
	OUT PUCHAR pIeLen)
{
	WSC_IE_HEADER ieHdr;
	UCHAR OutMsgBuf[512]; /* buffer to create message contents */
	UCHAR Len = 0, templen = 0;
	PUCHAR pData;
	USHORT tempVal = 0;
	PWSC_REG_DATA pReg = (PWSC_REG_DATA) &pAd->StaCfg[0].WscControl.RegData;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscMakeProbeReqIEWithVendorExt\n");
	/* WSC IE Header */
	ieHdr.elemId = 221;
	ieHdr.length = 4;
	ieHdr.oui[0] = 0x00;
	ieHdr.oui[1] = 0x50;
	ieHdr.oui[2] = 0xF2;
	ieHdr.oui[3] = 0x04;
	pData = (PUCHAR) &OutMsgBuf[0];
	Len = 0;
	/* 1. Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;

	/* 2. Request Type */
	if (pAd->StaCfg[0].WscControl.WscConfMode == WSC_ENROLLEE)
		tempVal = WSC_MSGTYPE_ENROLLEE_INFO_ONLY;
	else
		tempVal = WSC_MSGTYPE_REGISTRAR;

	templen = AppendWSCTLV(WSC_ID_REQ_TYPE, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
	/* 3. Config method */
	tempVal = htons(0x008c);
	templen = AppendWSCTLV(WSC_ID_CONFIG_METHODS, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
	/* 4. UUID */
	templen = AppendWSCTLV(WSC_ID_UUID_E, pData, pReg->SelfInfo.Uuid, 0);
	pData += templen;
	Len   += templen;
	/* 5. Primary device type */
	templen = AppendWSCTLV(WSC_ID_PRIM_DEV_TYPE, pData, pPrimaryDeviceType, 0);
	pData += templen;
	Len   += templen;
	/* 6. RF band, shall change based on current channel */
	templen = AppendWSCTLV(WSC_ID_RF_BAND, pData, &pReg->SelfInfo.RfBand, 0);
	pData += templen;
	Len   += templen;
	/* 7. Associate state */
	tempVal = pReg->SelfInfo.AssocState;
	templen = AppendWSCTLV(WSC_ID_ASSOC_STATE, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
	/* 8. Config error */
	tempVal = pReg->SelfInfo.ConfigError;
	templen = AppendWSCTLV(WSC_ID_CONFIG_ERROR, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
	/* 9. Device password ID */
	tempVal = pReg->SelfInfo.DevPwdId;
	templen = AppendWSCTLV(WSC_ID_DEVICE_PWD_ID, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;
	/* 10. Device Name */
	templen = AppendWSCTLV(WSC_ID_DEVICE_NAME, pData, pDeviceName, 0);
	pData += templen;
	Len   += templen;
	/* 11. Vendor Ext Attribute for L2SD Service Info */
	templen = AppendWSCTLV(WSC_ID_VENDOR_EXT, pData, pVendExt, vendExtLen);
	pData += templen;
	Len   += templen;
	ieHdr.length = ieHdr.length + Len;
	RTMPMoveMemory(pOutBuf, &ieHdr, sizeof(WSC_IE_HEADER));
	RTMPMoveMemory(pOutBuf + sizeof(WSC_IE_HEADER), OutMsgBuf, Len);
	*pIeLen = sizeof(WSC_IE_HEADER) + Len;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscMakeProbeReqIEWithVendorExt\n");
}
#endif /* WIDI_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */


VOID WscProfileRetryTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAdapter = NULL;
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	BOOLEAN bReConnect = TRUE;
	UCHAR CurOpMode = 0xFF;
	UCHAR if_idx = 0;
	struct tx_rx_ctl *tr_ctl = NULL;

	if (pWscControl == NULL)
		return;

	pAdapter = pWscControl->pAd;
	tr_ctl = &pAdapter->tr_ctl;

	if (pAdapter != NULL) {
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscProfileRetryTimeout:: WSC profile retry timeout index: %d\n", pWscControl->WscProfile.ApplyProfileIdx);
		if_idx = (pWscControl->EntryIfIdx & 0x0F);
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAdapter)
		CurOpMode = STA_MODE;
#ifdef P2P_SUPPORT

		if (pWscControl->EntryIfIdx != BSS0)
			CurOpMode = AP_MODE;

#endif /* P2P_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter)
		CurOpMode = AP_MODE;
#ifdef APCLI_SUPPORT

		if ((CurOpMode == AP_MODE)
			&& (GetAssociatedAPByWdev(pAdapter, &pAdapter->StaCfg[if_idx].wdev))
			&& (pAdapter->StaCfg[if_idx].SsidLen != 0)) {
			INT i;

			for (i = 0; VALID_UCAST_ENTRY_WCID(pAdapter, i); i++) {
				PMAC_TABLE_ENTRY pEntry = &pAdapter->MacTab.Content[i];
				STA_TR_ENTRY *tr_entry = &tr_ctl->tr_entry[i];

				if (IS_ENTRY_PEER_AP(pEntry) &&
					(pEntry->Sst == SST_ASSOC) &&
					(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
					bReConnect = FALSE;
			}
		}

#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

		if ((CurOpMode == STA_MODE)
			&& INFRA_ON(&pAdapter->StaCfg[0])
			&& (pAdapter->IndicateMediaState == NdisMediaStateConnected)) {
			pWscControl->WscProfileRetryTimerRunning = FALSE;
			bReConnect = FALSE;
		}

#endif /* CONFIG_STA_SUPPORT */

		if (bReConnect) {
			if (pWscControl->WscProfile.ApplyProfileIdx < pWscControl->WscProfile.ProfileCnt - 1)
				pWscControl->WscProfile.ApplyProfileIdx++;
			else
				pWscControl->WscProfile.ApplyProfileIdx = 0;

#ifdef APCLI_SUPPORT

			if (CurOpMode == AP_MODE) {
				WscWriteConfToApCliCfg(pAdapter,
									   pWscControl,
									   &pWscControl->WscProfile.Profile[pWscControl->WscProfile.ApplyProfileIdx],
									   TRUE);
				{
					RTEnqueueInternalCmd(pAdapter, CMDTHREAD_APCLI_IF_DOWN, (VOID *)&if_idx, sizeof(UCHAR));
				}
			}

#endif /* APCLI_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

			if (CurOpMode == STA_MODE) {
				WscWriteConfToPortCfg(pAdapter,
									  pWscControl,
									  &pWscControl->WscProfile.Profile[pWscControl->WscProfile.ApplyProfileIdx],
									  TRUE);
			}

#endif /* CONFIG_STA_SUPPORT */
			pAdapter->WriteWscCfgToDatFile = (pWscControl->EntryIfIdx & 0x0F);
			/*#ifdef KTHREAD_SUPPORT */
			/*			WAKE_UP(&(pAdapter->wscTask)); */
			/*#else */
			/*			RTMP_SEM_EVENT_UP(&(pAdapter->wscTask.taskSema)); */
			/*#endif */
			RtmpOsTaskWakeUp(&(pAdapter->wscTask));
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscProfileRetryTimeout:: WSC profile retry index: %d\n", pWscControl->WscProfile.ApplyProfileIdx);
		}

#ifdef CONFIG_STA_SUPPORT
		pAdapter->StaCfg[0].bAutoConnectByBssid = FALSE;
#endif /* CONFIG_STA_SUPPORT */
	}
}

VOID WscPBCTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	RTMP_ADAPTER *pAd = NULL;
	BOOLEAN Cancelled;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscPBCTimeOutAction\n");

	if (pWscControl != NULL)
		pAd = pWscControl->pAd;

#ifdef CON_WPS
#ifdef MULTI_INF_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "[IfaceIdx = %d] WscPBCTimeOutAction !!!\n", multi_inf_get_idx(pAd));
#endif /* MULTI_INF_SUPPORT */
#endif  /*CON_WPS*/

	if (pAd != NULL) {
		if (pWscControl->WscPBCTimerRunning) {
			pWscControl->WscPBCTimerRunning = FALSE;
			RTMPCancelTimer(&pWscControl->WscPBCTimer, &Cancelled);
		}

		WscPBCExec(pAd, FALSE, pWscControl);
		/* call Mlme handler to execute it */
		RTMP_MLME_HANDLER(pAd);
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscPBCTimeOutAction\n");
}

#ifdef WSC_STA_SUPPORT
VOID WscPINTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	RTMP_ADAPTER *pAd = NULL;
	BOOLEAN Cancelled;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscPINTimeOutAction\n");

	if (pWscControl != NULL)
		pAd = pWscControl->pAd;

	if (pAd != NULL) {
		if (pWscControl->WscPINTimerRunning) {
			pWscControl->WscPINTimerRunning = FALSE;
			RTMPCancelTimer(&pWscControl->WscPINTimer, &Cancelled);
		}

		WscPINExec(pAd, FALSE, pWscControl);
		/* call Mlme handler to execute it */
		RTMP_MLME_HANDLER(pAd);
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscPINTimeOutAction\n");
}
#endif

/*
*	========================================================================
*
*	Routine Description:
*		Exec scan after scan timer expiration
*
*	Arguments:
*		FunctionContext		NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*
*	========================================================================
*/
VOID WscScanTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = NULL;
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;

	if (pWscControl == NULL)
		return;

	pAd = pWscControl->pAd;

	if (pAd != NULL) {
		/* call to execute the scan actions */
		WscScanExec(pAd, pWscControl);

		/* register 10 second timer for PBC or PIN connection execution */
		if (pWscControl->WscMode == WSC_PBC_MODE) {
			/* Prevent infinite loop if conncet time out didn't stop the repeat scan */
			if (pWscControl->WscState != WSC_STATE_OFF &&
				!pWscControl->WscPBCTimerRunning) {
#ifdef CON_WPS /*Move to scan complete to trigger //Dung_Ru*/

				if (pWscControl->conWscStatus == CON_WPS_STATUS_DISABLED)
#endif /* CON_WPS */
				{
#ifdef APCLI_SUPPORT

					if (pWscControl->WscApCliScanMode != TRIGGER_PARTIAL_SCAN)
#endif /* APCLI_SUPPORT */
					{
#ifdef CONFIG_MAP_SUPPORT
						if (!IS_MAP_TURNKEY_ENABLE(pAd))
#endif /* CONFIG_MAP_SUPPORT */
						{
							RTMPSetTimer(&pWscControl->WscPBCTimer, 10000);
							pWscControl->WscPBCTimerRunning = TRUE;
						}
					}
				}
			}
		} else if (pWscControl->WscMode == WSC_PIN_MODE) {
			/* Prevent infinite loop if conncet time out didn't stop the repeat scan */
#ifdef WSC_STA_SUPPORT
			if (pWscControl->WscState != WSC_STATE_OFF &&
				!pWscControl->WscPINTimerRunning) {
				RTMPSetTimer(&pWscControl->WscPINTimer, 10000);
				pWscControl->WscPINTimerRunning = TRUE;
			}

#endif
		}

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "!!! WscScanTimeOutAction !!!\n");
		/* call Mlme handler to execute it */
		RTMP_MLME_HANDLER(pAd);
	}
}


#ifdef CON_WPS
VOID WscScanDoneCheckTimeOutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	RTMP_ADAPTER *pAd = NULL;
	BOOLEAN Cancelled;
	struct wifi_dev *wdev;
	UCHAR if_idx = 0;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscScanDoneCheckTimeOutAction\n");

	if (pWscControl != NULL) {
		pAd = pWscControl->pAd;
		if_idx = (pWscControl->EntryIfIdx & 0x0F);
	}

#ifdef MULTI_INF_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "[IfaceIdx = %d] WscScanDoneCheckTimeOutAction !!!\n", multi_inf_get_idx(pAd));
#endif /* MULTI_INF_SUPPORT */

	if (pAd != NULL) {
		pWscControl->ConWscApcliScanDoneCheckTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->ConWscApcliScanDoneCheckTimer, &Cancelled);

		/* call Mlme handler to execute it */
		if (if_idx < MAX_APCLI_NUM) {
			wdev = &(pAd->StaCfg[if_idx].wdev);
			MlmeEnqueueWithWdev(pAd, SYNC_FSM, SYNC_FSM_WSC_SCAN_COMP_CHECK_REQ,
				0, NULL, if_idx, wdev);
			RTMP_MLME_HANDLER(pAd);
		}
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscScanDoneCheckTimeOutAction\n");
}
#endif /*CON_WPS*/

BOOLEAN ValidateChecksum(
	IN UINT PIN)
{
	UINT accum = 0;

	accum += 3 * ((PIN / 10000000) % 10);
	accum += 1 * ((PIN / 1000000) % 10);
	accum += 3 * ((PIN / 100000) % 10);
	accum += 1 * ((PIN / 10000) % 10);
	accum += 3 * ((PIN / 1000) % 10);
	accum += 1 * ((PIN / 100) % 10);
	accum += 3 * ((PIN / 10) % 10);
	accum += 1 * ((PIN / 1) % 10);
	return (0 == (accum % 10));
} /* ValidateChecksum */

/*
*	Generate 4-digit random number, ex:1234
*/
UINT WscRandomGen4digitPinCode(
	IN  PRTMP_ADAPTER   pAd)
{
	UINT iPin;

	iPin = RandomByte2(pAd) * 256 * 256 + RandomByte2(pAd) * 256 + RandomByte2(pAd);
	iPin = iPin % 10000;
	return iPin;
}

UINT WscRandomGeneratePinCode(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR apidx)
{
	UINT iPin;
	UINT checksum;

	iPin = RandomByte(pAd) * 256 * 256 + RandomByte(pAd) * 256 + RandomByte(pAd);
	iPin = iPin % 10000000;
#ifdef WIDI_SUPPORT
#ifdef CONFIG_STA_SUPPORT

	if (pAd->StaCfg[0].bWIDI)
		iPin = ((iPin / 1000) * 1000);

#endif /* CONFIG_STA_SUPPORT */
#endif /* WIDI_SUPPORT */
	checksum = ComputeChecksum(iPin);
	iPin = iPin * 10 + checksum;
	return iPin;
}

#ifdef WIDI_SUPPORT
/*
*	Generate 4-digit random number, ex:1234
*	Add three zero, 1234000
*	Compute ckecksum, 1234000X
*
*/
UINT WscSpecialRandomGeneratePinCode(
	IN  PRTMP_ADAPTER pAd)
{
	UINT iPin;
	UINT checksum;

	iPin = RandomByte2(pAd) * 256 * 256 + RandomByte2(pAd) * 256 + RandomByte2(pAd);
	iPin = iPin % 10000000;
	iPin = ((iPin / 1000) * 1000);
	checksum = ComputeChecksum(iPin);
	iPin = iPin * 10 + checksum;
	return iPin;
}

UINT WscRandomGenerateP2PPinCode(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR apidx)
{
	UINT iPin;
	UINT checksum;

	iPin = RandomByte(pAd) * 256 * 256 + RandomByte(pAd) * 256 + RandomByte(pAd);
	iPin = iPin % 10000000;
	checksum = ComputeChecksum(iPin);
	iPin = iPin * 10 + checksum;
	return iPin;
}

#endif /* WIDI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID  WscInformFromWPA(
	IN  PMAC_TABLE_ENTRY pEntry)
{
	/* WPA_STATE_MACHINE informs this Entry is already WPA_802_1X_PORT_SECURED. */
	RTMP_ADAPTER *pAd = (PRTMP_ADAPTER)pEntry->pAd;
	BOOLEAN Cancelled;
	struct wifi_dev *wdev;

	if (pEntry->func_tb_idx >= pAd->ApCfg.BssidNum)
		return;

	wdev = pEntry->wdev;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscInformFromWPA\n");

	if (MAC_ADDR_EQUAL(pEntry->Addr, wdev->WscControl.EntryAddr)) {
		NdisZeroMemory(wdev->WscControl.EntryAddr, MAC_ADDR_LEN);
		RTMPCancelTimer(&wdev->WscControl.EapolTimer, &Cancelled);
		wdev->WscControl.EapolTimerRunning = FALSE;
		pEntry->bWscCapable = FALSE;
		wdev->WscControl.WscState = WSC_STATE_CONFIGURED;
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Reset EntryIfIdx to %d\n", WSC_INIT_ENTRY_APIDX);
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscInformFromWPA\n");
}
#endif /* CONFIG_AP_SUPPORT */

VOID WscStop(
	IN  PRTMP_ADAPTER pAd,
#ifdef CONFIG_AP_SUPPORT
	IN  BOOLEAN bFromApCli,
#endif /* CONFIG_AP_SUPPORT */
	IN  PWSC_CTRL pWscControl)
{
	PWSC_UPNP_NODE_INFO pWscUPnPInfo;
	BOOLEAN Cancelled;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[0];
#endif
#ifdef WSC_LED_SUPPORT
	UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	MAC_TABLE_ENTRY  *pEntry;
	/* UCHAR apidx = (pWscControl->EntryIfIdx & 0x0F); */
#endif /* CONFIG_AP_SUPPORT */
	UCHAR	CurOpMode = 0xff;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	CurOpMode = STA_MODE;
#ifdef P2P_SUPPORT

	if (pWscControl->EntryIfIdx != BSS0)
		CurOpMode = AP_MODE;

#endif /* P2P_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE)
		pWscControl->bConfiguredAP = FALSE;

#endif /* CONFIG_STA_SUPPORT */
	pWscUPnPInfo = &pWscControl->WscUPnPNodeInfo;

	if (pWscUPnPInfo->bUPnPMsgTimerRunning == TRUE) {
		pWscUPnPInfo->bUPnPMsgTimerRunning = FALSE;
		RTMPCancelTimer(&pWscUPnPInfo->UPnPMsgTimer, &Cancelled);
		pWscUPnPInfo->bUPnPMsgTimerPending = FALSE;
	}

	if (pWscControl->bM2DTimerRunning) {
		pWscControl->bM2DTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->M2DTimer, &Cancelled);
	}

	pWscUPnPInfo->bUPnPInProgress = FALSE;
	pWscControl->M2DACKBalance = 0;
	pWscUPnPInfo->registrarID = 0;

	if (pWscControl->Wsc2MinsTimerRunning) {
		pWscControl->Wsc2MinsTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
	}

	if (pWscControl->WscUpdatePortCfgTimerRunning) {
		pWscControl->WscUpdatePortCfgTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscUpdatePortCfgTimer, &Cancelled);
	}

	RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
	pWscControl->EapolTimerRunning = FALSE;
#ifdef CONFIG_AP_SUPPORT

	if ((pWscControl->EntryIfIdx & 0x1F) < pAd->ApCfg.BssidNum) {
		pEntry = MacTableLookup(pAd, pWscControl->EntryAddr);

		if (CurOpMode == AP_MODE) {
			if (pEntry && !bFromApCli)
				pEntry->bWscCapable = FALSE;
		}
	}

#endif /* CONFIG_AP_SUPPORT */
	NdisZeroMemory(pWscControl->EntryAddr, MAC_ADDR_LEN);
	pWscControl->WscSelReg = 0;

	if ((pWscControl->WscStatus == STATUS_WSC_CONFIGURED) ||
		(pWscControl->WscStatus == STATUS_WSC_FAIL) ||
		(pWscControl->WscStatus == STATUS_WSC_PBC_TOO_MANY_AP))
		;
	else
		pWscControl->WscStatus = STATUS_WSC_NOTUSED;

	pWscControl->WscState = WSC_STATE_OFF;
	pWscControl->lastId = 1;
	pWscControl->EapMsgRunning = FALSE;
	pWscControl->EapolTimerPending = FALSE;
	pWscControl->bWscTrigger = FALSE;

/* WPS_BandSteering Support */
#ifdef CONFIG_AP_SUPPORT
#ifdef BAND_STEERING
	/* WPS: clear WPS WHITELIST in case of AP Wsc Stop */
	if (!bFromApCli && (pAd->ApCfg.BandSteering)) {

		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x1F].wdev;
		PBND_STRG_CLI_TABLE table = Get_BndStrgTable(pAd, wdev->func_idx);

		if (table && table->bEnabled) {
			NdisAcquireSpinLock(&table->WpsWhiteListLock);
			ClearWpsWhiteList(&table->WpsWhiteList);
			NdisReleaseSpinLock(&table->WpsWhiteListLock);
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s:channel %u wps whitelist cleared, size : %d\n",
			 __func__, table->Channel, table->WpsWhiteList.size);
		}
	}
#endif
#endif /* CONFIG_AP_SUPPORT */

	if (pWscControl->WscScanTimerRunning) {
		pWscControl->WscScanTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscScanTimer, &Cancelled);
	}

	if (pWscControl->WscPBCTimerRunning) {
		pWscControl->WscPBCTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscPBCTimer, &Cancelled);
	}

#ifdef CON_WPS

	if (bFromApCli && (pWscControl->EntryIfIdx & 0x0F) < MAX_APCLI_NUM) {
		pAd->StaCfg[(pWscControl->EntryIfIdx & 0x0F)].ConWpsApCliModeScanDoneStatus = CON_WPS_APCLI_SCANDONE_STATUS_NOTRIGGER;
		pWscControl->conWscStatus = CON_WPS_STATUS_DISABLED;
		if (pWscControl->ConWscApcliScanDoneCheckTimerRunning) {
			pWscControl->ConWscApcliScanDoneCheckTimerRunning = FALSE;
			RTMPCancelTimer(&pWscControl->ConWscApcliScanDoneCheckTimer, &Cancelled);
		}
	}  else {
		if (pWscControl->ConWscApcliScanDoneCheckTimerRunning) {
			pWscControl->ConWscApcliScanDoneCheckTimerRunning = FALSE;
			RTMPCancelTimer(&pWscControl->ConWscApcliScanDoneCheckTimer, &Cancelled);
		}
	}

#endif /*CON_WPS*/
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		pStaCfg->MlmeAux.AutoReconnectSsidLen = pStaCfg->MlmeAux.SsidLen;
		NdisZeroMemory(&pStaCfg->MlmeAux.AutoReconnectSsid[0], MAX_LEN_OF_SSID);
		NdisMoveMemory(&pStaCfg->MlmeAux.AutoReconnectSsid[0], &pStaCfg->MlmeAux.Ssid[0], pStaCfg->MlmeAux.SsidLen);
		/* YF: Reset to default after active pbc mode */
		pWscControl->RegData.SelfInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PIN);
	}

#endif /* CONFIG_STA_SUPPORT */
#ifdef WSC_LED_SUPPORT

	if (pWscControl->WscLEDTimerRunning) {
		pWscControl->WscLEDTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscLEDTimer, &Cancelled);
	}

	if (pWscControl->WscSkipTurnOffLEDTimerRunning) {
		pWscControl->WscSkipTurnOffLEDTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscSkipTurnOffLEDTimer, &Cancelled);
	}

	/* Reset the WPS walk time. */
	pWscControl->bWPSWalkTimeExpiration = FALSE;
	WPSLEDStatus = LED_WPS_TURN_LED_OFF;
	RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_TURNKEY_ENABLE(pAd) && (bFromApCli == TRUE) && pWscControl->WscProfileRetryTimerRunning) {
		pWscControl->WscProfileRetryTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscProfileRetryTimer, &Cancelled);
	}
#endif /* MAP_SUPPORT */
#endif /* WSC_LED_SUPPORT */
}

#ifdef CON_WPS
VOID WscConWpsStop(
	IN  PRTMP_ADAPTER pAd,
	IN  BOOLEAN bFromApCli,
	IN  PWSC_CTRL pWscControl)
{
#ifdef MULTI_INF_SUPPORT
	/* Single Driver ctrl the WSC SM between the two pAd */
	PRTMP_ADAPTER pOpposAd;
	PWSC_CTRL pWpsCtrl = NULL;
	INT IsAPConfigured = 0;
	UCHAR pAdListInfo[2] = {0, 0};
	INT myBandIdx = 0, opsBandIdx = 0;
	INT nowBandIdx = multi_inf_get_idx(pAd);
	UCHAR loop;

	if (pWscControl->conWscStatus == CON_WPS_STATUS_DISABLED)
		return;

	/* Update the Global pAd List Band Info */
	/*pOpposAd = (PRTMP_ADAPTER)adapt_list[0];*/
	pAdListInfo[0] = 0;/* RFIC_IS_5G_BAND(pOpposAd); */
	/*pOpposAd = (PRTMP_ADAPTER)adapt_list[1];*/
	pAdListInfo[1] = 1;/* RFIC_IS_5G_BAND(pOpposAd); */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "pAdListInfo is5G---> [%d, %d]\n",
			 pAdListInfo[0], pAdListInfo[1]);

	/* which band from function in */
	if (nowBandIdx == pAdListInfo[0]) {
		myBandIdx = 0;
		opsBandIdx = 1;
	} else if (nowBandIdx == pAdListInfo[1]) {
		myBandIdx = 1;
		opsBandIdx = 0;
	} else {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "pAdListInfo is5G---> [%d, %d] Fail\n",
				 pAdListInfo[0], pAdListInfo[1]);
	}

	pOpposAd = (PRTMP_ADAPTER) adapt_list[opsBandIdx];

	if (bFromApCli) {
		if (pOpposAd) {
			for (loop = 0; loop < pOpposAd->ApCfg.ApCliNum; loop++) {
				pWpsCtrl = &pOpposAd->StaCfg[loop].wdev.WscControl;
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "CON_WPS: Stop the Band[%d] %s ApCli WPS, state [%d]\n",
						 opsBandIdx, pWpsCtrl->IfName, pWpsCtrl->WscState);

				if (pWpsCtrl->WscState != WSC_STATE_OFF) {
					WscStop(pOpposAd, TRUE, pWpsCtrl);
					pWpsCtrl->WscConfMode = WSC_DISABLE;
				}
			}
		}

		for (loop = 0; loop < pAd->ApCfg.ApCliNum; loop++) {
			if ((pWscControl->EntryIfIdx & 0x0F) == loop)
				continue;

			pWpsCtrl = &pAd->StaCfg[loop].wdev.WscControl;

			if (pWpsCtrl->WscState != WSC_STATE_OFF) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "CON_WPS: Stop the current pAd apcli [%d]\n",
						 loop);
				WscStop(pAd, TRUE, pWpsCtrl);
				pWpsCtrl->WscConfMode = WSC_DISABLE;
			}
		}
	} else {
		if (pOpposAd != NULL) {
			for (loop = 0; VALID_MBSS(pAd, loop); loop++) {
				struct wifi_dev *wdev = &pOpposAd->ApCfg.MBSSID[loop].wdev;

				pWpsCtrl = &wdev->WscControl;
				IsAPConfigured = pWpsCtrl->WscConfStatus;

				if ((wdev) &&
					(pWpsCtrl->WscConfMode != WSC_DISABLE) &&
					(pWpsCtrl->bWscTrigger == TRUE)) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "CON_WPS[%d]: Stop the %s AP Wsc Machine\n", opsBandIdx, pOpposAd->net_dev->name);
					WscBuildBeaconIE(pOpposAd, IsAPConfigured, FALSE, 0, 0, loop, NULL, 0, AP_MODE);
					WscBuildProbeRespIE(pOpposAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0,
										loop, NULL, 0, AP_MODE);
					UpdateBeaconHandler(pOpposAd, wdev, BCN_UPDATE_IE_CHG);
					WscStop(pOpposAd, FALSE, pWpsCtrl);
				}
			}
		} else {
			for (loop = 0; VALID_MBSS(pAd, loop); loop++) {
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[loop].wdev;

				pWpsCtrl = &wdev->WscControl;
				IsAPConfigured = pWpsCtrl->WscConfStatus;

				if ((wdev) &&
					(pWpsCtrl->WscConfMode != WSC_DISABLE) &&
					(pWpsCtrl->bWscTrigger == TRUE)) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "CON_WPS: Stop the %s AP Wsc Machine\n", wdev->if_dev->name);
					WscBuildBeaconIE(pAd, IsAPConfigured, FALSE, 0, 0, loop, NULL, 0, AP_MODE);
					WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0,
										loop, NULL, 0, AP_MODE);
					UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
					WscStop(pAd, FALSE, pWpsCtrl);
				}
			}
		}
	}

#endif /* MULTI_INF_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscConWpsStop\n");
}
#endif /* CON_WPS */


VOID WscInit(
	IN  PRTMP_ADAPTER pAd,
	IN  BOOLEAN bFromApCli,
	IN  UCHAR BssIndex)
{
	PWSC_CTRL pWscControl = NULL;
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
	UCHAR CurOpMode = AP_MODE;
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	INT IsAPConfigured;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

		if (bFromApCli) {
			if ((BssIndex & 0x0F) < MAX_APCLI_NUM)
				wdev = &pAd->StaCfg[BssIndex & 0x0F].wdev;
		} else
#endif /* APCLI_SUPPORT */
			wdev = &pAd->ApCfg.MBSSID[BssIndex & 0x1F].wdev;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#ifdef P2P_SUPPORT

		if (BssIndex >= MIN_NET_DEVICE_FOR_P2P_GO)
			wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
		else if (bFromApCli)
			wdev = &pAd->StaCfg[MAIN_MBSSID].wdev;
		else
#endif /* P2P_SUPPORT */
		{
			wdev = &pAd->StaCfg[BssIndex & 0x0F].wdev;
			CurOpMode = STA_MODE;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	if (wdev == NULL)
		return;

	pWscControl = &wdev->WscControl;

	if (pWscControl->WscEnrolleePinCode == 0) {
		if (pWscControl->WscEnrollee4digitPinCode) {
			pWscControl->WscEnrolleePinCodeLen = 4;
			pWscControl->WscEnrolleePinCode = WscRandomGen4digitPinCode(pAd);
		} else {
			pWscControl->WscEnrolleePinCode = GenerateWpsPinCode(pAd, bFromApCli, BssIndex);
			pWscControl->WscEnrolleePinCodeLen = 8;
		}
	}

	pWscControl->RegData.SelfInfo.Version = WSC_VERSION;
#ifdef WSC_V2_SUPPORT
	pWscControl->RegData.SelfInfo.Version2 = WSC_V2_VERSION;
#endif /* WSC_V2_SUPPORT */
	pWscControl->bWscLastOne = FALSE;
	pWscControl->bWscFirstOne = FALSE;
#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
	pWscControl->WscStatus = STATUS_WSC_IDLE;
#ifdef CONFIG_AP_SUPPORT

	if (((CurOpMode == AP_MODE) &&
		 (pWscControl->WscConfMode == WSC_DISABLE))
#ifdef WSC_V2_SUPPORT
		|| ((pWscControl->WscV2Info.bWpsEnable == FALSE) && pWscControl->WscV2Info.bEnableWpsV2)
#endif /* WSC_V2_SUPPORT */
	   ) {
		if (CurOpMode == AP_MODE) {
#ifdef APCLI_SUPPORT

			if (!bFromApCli)
#endif /* APCLI_SUPPORT */
			{
				wdev->WscIEBeacon.ValueLen = 0;
				wdev->WscIEProbeResp.ValueLen = 0;
			}

#ifdef APCLI_SUPPORT
			else
				WscInitRegistrarPair(pAd, pWscControl, BssIndex & 0x0F);

#endif /* APCLI_SUPPORT */
		}
	} else
#endif /* CONFIG_AP_SUPPORT */
	{
#ifdef P2P_SUPPORT

		if (pWscControl->WscConfMode == WSC_DISABLE) {
			if (BssIndex >= MIN_NET_DEVICE_FOR_P2P_GO) {
				wdev->WscIEBeacon.ValueLen = 0;
				wdev->WscIEProbeResp.ValueLen = 0;
			}

			return;
		}

#endif /* P2P_SUPPORT */
		WscInitRegistrarPair(pAd, pWscControl, BssIndex & 0x1F);
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == AP_MODE) {
#ifdef APCLI_SUPPORT

			if (!bFromApCli)
#endif /* APCLI_SUPPORT */
			{
				UCHAR apidx = pWscControl->EntryIfIdx & 0x1F;
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

				IsAPConfigured = pWscControl->WscConfStatus;
				WscBuildBeaconIE(pAd, IsAPConfigured, FALSE, 0, 0, (BssIndex & 0x1F), NULL, 0, AP_MODE);
				WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0, BssIndex, NULL, 0, AP_MODE);
				UpdateBeaconHandler(
					pAd,
					wdev,
					BCN_UPDATE_IE_CHG);
			}
		}

#endif /* CONFIG_AP_SUPPORT */
	}

}

USHORT WscGetAuthType(
	IN UINT32 authType)
{
	if (IS_AKM_OPEN(authType))
		return WSC_AUTHTYPE_OPEN;
	else if (IS_AKM_SHARED(authType))
		return WSC_AUTHTYPE_SHARED;
	else if (IS_AKM_WPANONE(authType))
		return WSC_AUTHTYPE_WPANONE;
#if (defined(MAP_R3) || defined(DPP_SUPPORT))
	else if (IS_AKM_DPP(authType) && IS_AKM_WPA3PSK(authType) && IS_AKM_WPA2PSK(authType))
		return WSC_AUTHTYPE_DPP | WSC_AUTHTYPE_SAE | WSC_AUTHTYPE_WPA2PSK;
	else if (IS_AKM_DPP(authType) && IS_AKM_WPA2PSK(authType))
		return WSC_AUTHTYPE_DPP | WSC_AUTHTYPE_WPA2PSK;
	else if (IS_AKM_DPP(authType) && IS_AKM_WPA3PSK(authType))
		return WSC_AUTHTYPE_DPP | WSC_AUTHTYPE_SAE;
	else if (IS_AKM_DPP(authType))
		return WSC_AUTHTYPE_DPP;
#endif
	else if (IS_AKM_WPA1(authType) && IS_AKM_WPA2(authType))
		return WSC_AUTHTYPE_WPA | WSC_AUTHTYPE_WPA2;
	else if (IS_AKM_WPA1PSK(authType) && IS_AKM_WPA2PSK(authType))
		return WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK;
	else if (IS_AKM_WPA1(authType))
		return WSC_AUTHTYPE_WPA;
	else if (IS_AKM_WPA1PSK(authType))
		return WSC_AUTHTYPE_WPAPSK;
#ifdef DOT11_SAE_SUPPORT
	else if (IS_AKM_WPA2PSK(authType) && IS_AKM_WPA3PSK(authType))
		return WSC_AUTHTYPE_WPA2PSK | WSC_AUTHTYPE_SAE;
#endif
	else if (IS_AKM_WPA2(authType))
		return WSC_AUTHTYPE_WPA2;
	else if (IS_AKM_WPA2PSK(authType))
		return WSC_AUTHTYPE_WPA2PSK;
#ifdef DOT11_SAE_SUPPORT
	else if (IS_AKM_WPA3PSK(authType))
		return WSC_AUTHTYPE_SAE;
#endif
	else
		return WSC_AUTHTYPE_OPEN;
}

USHORT WscGetEncryType(
	IN UINT32 encryType)
{
	if (IS_CIPHER_NONE(encryType))
		return WSC_ENCRTYPE_NONE;
	else if (IS_CIPHER_WEP(encryType))
		return WSC_ENCRTYPE_WEP;
	else if (IS_CIPHER_TKIP(encryType) && IS_CIPHER_CCMP128(encryType))
		return WSC_ENCRTYPE_AES | WSC_ENCRTYPE_TKIP;
	else if (IS_CIPHER_TKIP(encryType))
		return WSC_ENCRTYPE_TKIP;
	else if (IS_CIPHER_CCMP128(encryType))
		return WSC_ENCRTYPE_AES;
	else
		return WSC_ENCRTYPE_AES;
}

RTMP_STRING *WscGetAuthTypeStr(
	IN  USHORT authFlag)
{
	switch (authFlag) {
	case WSC_AUTHTYPE_OPEN:
		return "OPEN";

	case WSC_AUTHTYPE_WPAPSK:
		return "WPAPSK";

	case WSC_AUTHTYPE_SHARED:
		return "SHARED";

	case WSC_AUTHTYPE_WPANONE:
		return "WPANONE";

	case WSC_AUTHTYPE_WPA:
		return "WPA";

	case WSC_AUTHTYPE_WPA2:
		return "WPA2";

	default:
	case (WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK):
		return "WPAPSKWPA2PSK";

	case WSC_AUTHTYPE_WPA2PSK:
		return "WPA2PSK";

	case (WSC_AUTHTYPE_OPEN | WSC_AUTHTYPE_SHARED):
		return "WEPAUTO";
#if (defined(MAP_R3) || defined(DPP_SUPPORT))
	case WSC_AUTHTYPE_DPP:
		return "DPP";
	case (WSC_AUTHTYPE_DPP | WSC_AUTHTYPE_WPA2PSK):
		return "DPPWPA2PSK";
	case (WSC_AUTHTYPE_DPP | WSC_AUTHTYPE_SAE):
		return "DPPWPA3PSK";
	case (WSC_AUTHTYPE_DPP | WSC_AUTHTYPE_SAE | WSC_AUTHTYPE_WPA2PSK):
		return "DPPWPA3PSKWPA2PSK";
#endif

	}
}

RTMP_STRING *WscGetEncryTypeStr(
	IN  USHORT encryFlag)
{
	switch (encryFlag) {
	case WSC_ENCRTYPE_NONE:
		return "NONE";

	case WSC_ENCRTYPE_WEP:
		return "WEP";

	case WSC_ENCRTYPE_TKIP:
		return "TKIP";

	default:
	case (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES):
		return "TKIPAES";

	case WSC_ENCRTYPE_AES:
		return "AES";
	}
}

#ifdef CONFIG_AP_SUPPORT
static UINT32 WscGetAuthMode(
	IN  USHORT authFlag)
{
	UINT32 AKMMap = 0;

	switch (authFlag) {
	case WSC_AUTHTYPE_OPEN:
		SET_AKM_OPEN(AKMMap);
		break;

	case WSC_AUTHTYPE_WPAPSK:
		SET_AKM_WPA1PSK(AKMMap);
		break;

	case WSC_AUTHTYPE_SHARED:
		SET_AKM_SHARED(AKMMap);
		break;

	case WSC_AUTHTYPE_WPANONE:
		SET_AKM_WPANONE(AKMMap);
		break;

	case WSC_AUTHTYPE_WPA:
		SET_AKM_WPA1(AKMMap);
		break;

	case WSC_AUTHTYPE_WPA2:
		SET_AKM_WPA2(AKMMap);
		break;

	case (WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK):
		SET_AKM_WPA1PSK(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
		break;

	case WSC_AUTHTYPE_WPA2PSK:
		SET_AKM_WPA2PSK(AKMMap);
		break;
	}

	return AKMMap;
}

static UINT32 WscGetWepStatus(
	IN  USHORT encryFlag)
{
	UINT32 EncryType = 0;

	switch (encryFlag) {
	case WSC_ENCRTYPE_NONE:
		SET_CIPHER_NONE(EncryType);
		break;

	case WSC_ENCRTYPE_WEP:
		SET_CIPHER_WEP(EncryType);
		break;

	case WSC_ENCRTYPE_TKIP:
		SET_CIPHER_TKIP(EncryType);
		break;

	case (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES):
		SET_CIPHER_TKIP(EncryType);
		SET_CIPHER_CCMP128(EncryType);
		break;

	case WSC_ENCRTYPE_AES:
		SET_CIPHER_CCMP128(EncryType);
		break;
	}

	return EncryType;
}
#endif /* CONFIG_AP_SUPPORT */

static VOID WscSetWepCipher(
	IN  struct _SECURITY_CONFIG *pSecConfig,
	IN  PWSC_CREDENTIAL pCredential,
	IN  UCHAR WepKeyId,
	IN  USHORT WepKeyLen)
{
	if (WepKeyLen == 5 || WepKeyLen == 13) {
		pSecConfig->WepKey[WepKeyId].KeyLen = (UCHAR)WepKeyLen;
		os_move_mem(pSecConfig->WepKey[WepKeyId].Key, pCredential->Key, WepKeyLen);
		CLEAR_CIPHER(pSecConfig->PairwiseCipher);

		if (WepKeyLen == 5)
			SET_CIPHER_WEP40(pSecConfig->PairwiseCipher);
		else
			SET_CIPHER_WEP104(pSecConfig->PairwiseCipher);
	} else {
		pSecConfig->WepKey[WepKeyId].KeyLen = (UCHAR)(WepKeyLen / 2);
		AtoH((RTMP_STRING *) pCredential->Key, pSecConfig->WepKey[WepKeyId].Key, WepKeyLen / 2);

		if (WepKeyLen == 10)
			SET_CIPHER_WEP40(pSecConfig->PairwiseCipher);
		else
			SET_CIPHER_WEP104(pSecConfig->PairwiseCipher);
	}
}

void WscWriteConfToPortCfg(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl,
	IN  PWSC_CREDENTIAL pCredential,
	IN  BOOLEAN bEnrollee)
{
	UCHAR CurApIdx = MAIN_MBSSID;
	UCHAR CurOpMode = AP_MODE;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[0];
#endif
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscWriteConfToPortCfg\n");
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (pWscControl->EntryIfIdx == BSS0)
			CurOpMode = STA_MODE;
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE)
		CurApIdx = (pWscControl->EntryIfIdx & 0x1F);

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	if (bEnrollee || (CurOpMode == AP_MODE)) {
		if (CurOpMode == AP_MODE) {
			NdisZeroMemory(pAd->ApCfg.MBSSID[CurApIdx].Ssid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pAd->ApCfg.MBSSID[CurApIdx].Ssid, pCredential->SSID.Ssid, pCredential->SSID.SsidLength);
			pAd->ApCfg.MBSSID[CurApIdx].SsidLen = pCredential->SSID.SsidLength;
#ifdef P2P_SUPPORT

			if (P2P_GO_ON(pAd)) {
				NdisZeroMemory(pAd->P2pCfg.SSID, MAX_LEN_OF_SSID);
				pAd->P2pCfg.SSIDLen = pCredential->SSID.SsidLength;
				NdisMoveMemory(pAd->P2pCfg.SSID, pCredential->SSID.Ssid, pAd->P2pCfg.SSIDLen);
			}

#endif /* P2P_SUPPORT */
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

		if (CurOpMode == STA_MODE) {
			pStaCfg->MlmeAux.AutoReconnectSsidLen = pCredential->SSID.SsidLength;
			NdisZeroMemory(pStaCfg->MlmeAux.AutoReconnectSsid, NDIS_802_11_LENGTH_SSID);
			NdisMoveMemory(pStaCfg->MlmeAux.AutoReconnectSsid, pCredential->SSID.Ssid, pStaCfg->MlmeAux.AutoReconnectSsidLen);
			pStaCfg->MlmeAux.SsidLen = pCredential->SSID.SsidLength;
			NdisZeroMemory(pStaCfg->MlmeAux.Ssid, NDIS_802_11_LENGTH_SSID);
			NdisMoveMemory(pStaCfg->MlmeAux.Ssid, pCredential->SSID.Ssid, pStaCfg->MlmeAux.SsidLen);

			if (!NdisEqualMemory(pCredential->MacAddr, pAd->StaCfg[0].wdev.if_addr, MAC_ADDR_LEN)) {
				NdisZeroMemory(pStaCfg->MlmeAux.Bssid, MAC_ADDR_LEN);
				NdisMoveMemory(pStaCfg->MlmeAux.Bssid, pCredential->MacAddr, MAC_ADDR_LEN);
			}
		}

#endif /* CONFIG_STA_SUPPORT */
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "ra%d - AuthType: %u, EncrType: %u\n", CurApIdx, pCredential->AuthType, pCredential->EncrType);

		if (pCredential->AuthType & (WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK | WSC_AUTHTYPE_WPANONE)) {
			if (!(pCredential->EncrType & (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES))) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "AuthType is WPAPSK or WPA2PAK.\n"
						 "Get illegal EncrType(%d) from External Registrar, set EncrType to TKIP\n",
						 pCredential->EncrType);
				pCredential->EncrType = WSC_ENCRTYPE_TKIP;
			}

#ifdef CONFIG_STA_SUPPORT

			if (CurOpMode == STA_MODE)
				pAd->StaCfg[0].WpaState = SS_START;

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

			if ((CurOpMode == AP_MODE) && (pWscControl->WscV2Info.bEnableWpsV2)) {
				if (pCredential->AuthType == WSC_AUTHTYPE_WPAPSK)
					pCredential->AuthType = (WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK);

				if (pCredential->EncrType == WSC_ENCRTYPE_TKIP)
					pCredential->EncrType = (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES);
			}

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
		}

		WscSetAuthMode(pAd, CurOpMode, CurApIdx, WscGetAuthTypeStr(pCredential->AuthType));
		WscSetEncrypType(pAd, CurOpMode, CurApIdx, WscGetEncryTypeStr(pCredential->EncrType));

		if (pCredential->EncrType != WSC_ENCRTYPE_NONE) {
			if (pCredential->EncrType & (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES)) {
				struct wifi_dev *p_wdev = NULL;
#ifdef CONFIG_AP_SUPPORT

				if (CurOpMode == AP_MODE) {
					p_wdev = &pAd->ApCfg.MBSSID[CurApIdx].wdev;
					p_wdev->SecConfig.PairwiseKeyId = 1;
				}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

				if (CurOpMode == STA_MODE) {
					p_wdev = &pAd->StaCfg[0].wdev;
					p_wdev->SecConfig.PairwiseKeyId = 0;
				}

#endif /* CONFIG_STA_SUPPORT */

				if (pCredential->KeyLength >= 8 && pCredential->KeyLength <= 64) {
					RTMP_STRING PassphraseStr[65] = {0};

					pWscControl->WpaPskLen = pCredential->KeyLength;
					RTMPZeroMemory(pWscControl->WpaPsk, 64);
					RTMPMoveMemory(pWscControl->WpaPsk, pCredential->Key, pWscControl->WpaPskLen);
					RTMPMoveMemory(PassphraseStr, pCredential->Key, pWscControl->WpaPskLen);
					RTMPZeroMemory(p_wdev->SecConfig.PSK, LEN_PSK + 1);
					RTMPMoveMemory(p_wdev->SecConfig.PSK, pCredential->Key, pWscControl->WpaPskLen);
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WpaPskLen = %d\n", pWscControl->WpaPskLen);
				} else {
					pWscControl->WpaPskLen = 0;
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN, "WPAPSK: Invalid Key Length (%d)\n", pCredential->KeyLength);
				}
			} else if (pCredential->EncrType == WSC_ENCRTYPE_WEP) { /* Only for WPS 2.0 */
				UCHAR   WepKeyId = 0;
				USHORT  WepKeyLen = pCredential->KeyLength;

				if ((pCredential->KeyIndex >= 1) && (pCredential->KeyIndex <= 4)) {
					struct _SECURITY_CONFIG *pSecConfig = NULL;

					WepKeyId = (pCredential->KeyIndex - 1); /* KeyIndex = 1 ~ 4 */
#ifdef CONFIG_AP_SUPPORT

					if (CurOpMode == AP_MODE)
						pSecConfig = &pAd->ApCfg.MBSSID[CurApIdx].wdev.SecConfig;

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

					if (CurOpMode == STA_MODE)
						pSecConfig = &pAd->StaCfg[0].wdev.SecConfig;

#endif /* CONFIG_STA_SUPPORT */
					pSecConfig->PairwiseKeyId = WepKeyId;

					/* 5 or 13 ASCII characters */
					/* 10 or 26 Hex characters */
					if (WepKeyLen == 5 || WepKeyLen == 13 || WepKeyLen == 10 || WepKeyLen == 26)
						WscSetWepCipher(pSecConfig, pCredential, WepKeyId, WepKeyLen);
					else
						MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
							"WEP: Invalid Key Length (%d)\n", pCredential->KeyLength);
				} else {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
							"Unsupport default key index (%d)\n", WepKeyId);
#ifdef CONFIG_AP_SUPPORT

					if (CurOpMode == AP_MODE)
						pAd->ApCfg.MBSSID[CurApIdx].wdev.SecConfig.PairwiseKeyId = 0;

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

					if (CurOpMode == STA_MODE)
						pAd->StaCfg[0].wdev.SecConfig.PairwiseKeyId = 0;

#endif /* CONFIG_STA_SUPPORT */
				}
			}
		}

#ifdef CONFIG_AP_SUPPORT
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		/*
		*	Atheros WPS Testbed AP will put A-Band BSSID in credential of M7.
		*	To prevent 2.4G only STA would fail to re-connect by BSSID, set profile retry timer here.
		*/
		if ((pAd->StaCfg[0].BssType == BSS_INFRA) &&
			(pWscControl->WscDriverAutoConnect == 2) &&
			(pWscControl->WscProfile.ProfileCnt >= 1)) {
			pWscControl->WscProfileRetryTimerRunning = TRUE;
			RTMPSetTimer(&pWscControl->WscProfileRetryTimer, WSC_PROFILE_RETRY_TIME_OUT);
		}

#ifdef IWSC_SUPPORT

		if ((pAd->StaCfg[0].BssType == BSS_ADHOC) &&
			(pAd->StaCfg[0].IWscInfo.RegDepth != 0) &&
			(pAd->StaCfg[0].IWscInfo.AvaSubMaskListCount != 0)) {
			if ((pCredential->AvaIpv4SubmaskList[0] == 0) &&
				(pCredential->AvaIpv4SubmaskList[1] == 0) &&
				(pCredential->AvaIpv4SubmaskList[2] == 0))
				pAd->StaCfg[0].IWscInfo.AvaSubMaskListCount = 0;
		}

#endif /* IWSC_SUPPORT */
	}

#endif /* CONFIG_STA_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			 "<----- ra%d - WscWriteConfToPortCfg\n", CurApIdx);
}


VOID	WscWriteSsidToDatFile(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *pTempStr,
	IN  BOOLEAN bNewFormat,
	IN  UCHAR CurOpMode)
{
#ifdef CONFIG_AP_SUPPORT
	UCHAR	apidx;
	INT ret;
#endif /* CONFIG_AP_SUPPORT */
	INT offset = 0;

	if (bNewFormat == FALSE) {
		NdisZeroMemory(pTempStr, 1024);
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == AP_MODE) {
			for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
				if (apidx == 0) {
					NdisMoveMemory(pTempStr, "SSID=", strlen("SSID="));
					offset = strlen(pTempStr);
				} else {
					offset = strlen(pTempStr);
					NdisMoveMemory(pTempStr + offset, ";", 1);
					offset += 1;
				}

				NdisMoveMemory(pTempStr + offset, pAd->ApCfg.MBSSID[apidx].Ssid, pAd->ApCfg.MBSSID[apidx].SsidLen);
			}
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

		if (CurOpMode == STA_MODE) {
			UINT profile_idx = pAd->StaCfg[0].wdev.WscControl.WscProfile.ApplyProfileIdx;
			PWSC_CREDENTIAL pCredential = &pAd->StaCfg[0].wdev.WscControl.WscProfile.Profile[profile_idx];

			NdisMoveMemory(pTempStr, "SSID=", strlen("SSID="));
			offset = strlen(pTempStr);
			NdisMoveMemory(pTempStr + offset, pCredential->SSID.Ssid, pCredential->SSID.SsidLength);
		}

#endif /* CONFIG_STA_SUPPORT */
	}

#ifdef CONFIG_AP_SUPPORT
	else {
		RTMP_STRING item_str[10] = {0};

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			ret = snprintf(item_str, sizeof(item_str), "SSID%d", (apidx + 1));
			if (os_snprintf_error(sizeof(item_str), ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "item_str snprintf error!\n");

			if (rtstrstr(pTempStr, item_str)) {
				NdisZeroMemory(pTempStr, 1024);
				NdisMoveMemory(pTempStr, item_str, strlen(item_str));
				offset = strlen(pTempStr);
				NdisMoveMemory(pTempStr + offset, "=", 1);
				offset += 1;
				NdisMoveMemory(pTempStr + offset, pAd->ApCfg.MBSSID[apidx].Ssid, pAd->ApCfg.MBSSID[apidx].SsidLen);
			}

			NdisZeroMemory(item_str, 10);
		}
	}

#endif /* CONFIG_AP_SUPPORT */
}


VOID WscWriteWpaPskToDatFile(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *pTempStr,
	IN  BOOLEAN bNewFormat)
{
#ifdef CONFIG_AP_SUPPORT
	UCHAR apidx;
	INT ret;
#endif /* CONFIG_AP_SUPPORT */
	PWSC_CTRL pWscControl;
	INT offset = 0;

	if (bNewFormat == FALSE) {
		NdisZeroMemory(pTempStr, 512);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;

				if (apidx == 0) {
					NdisMoveMemory(pTempStr, "WPAPSK=", strlen("WPAPSK="));
					offset = strlen(pTempStr);
				} else {
					offset = strlen(pTempStr);
					NdisMoveMemory(pTempStr + offset, ";", 1);
					offset += 1;
				}

				NdisMoveMemory(pTempStr + offset, pWscControl->WpaPsk, pWscControl->WpaPskLen);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			pWscControl = &pAd->StaCfg[0].wdev.WscControl;
			NdisMoveMemory(pTempStr, "WPAPSK=", strlen("WPAPSK="));

			if (pWscControl->WpaPskLen) {
				offset = strlen(pTempStr);
				NdisMoveMemory(pTempStr + offset, pWscControl->WpaPsk, pWscControl->WpaPskLen);
			}
		}
#endif /* CONFIG_STA_SUPPORT */
	}

#ifdef CONFIG_AP_SUPPORT
	else {
		RTMP_STRING item_str[10] = {0};

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			ret = snprintf(item_str, sizeof(item_str), "WPAPSK%d", (apidx + 1));
			if (os_snprintf_error(sizeof(item_str), ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "item_str snprintf error!\n");

			if (rtstrstr(pTempStr, item_str)) {
				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
				NdisZeroMemory(pTempStr, 512);
				NdisMoveMemory(pTempStr, item_str, strlen(item_str));
				offset = strlen(pTempStr);
				NdisMoveMemory(pTempStr + offset, "=", 1);
				offset += 1;
				NdisMoveMemory(pTempStr + offset, pWscControl->WpaPsk, pWscControl->WpaPskLen);
			}

			NdisZeroMemory(item_str, 10);
		}
	}

#endif /* CONFIG_AP_SUPPORT */
}

static BOOLEAN WscCheckNonce(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MLME_QUEUE_ELEM * pElem,
	IN  BOOLEAN bFlag,
	IN  PWSC_CTRL pWscControl)
{
	USHORT Length;
	PUCHAR pData;
	USHORT WscType, WscLen, WscId;

	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscCheckNonce\n");

	if (bFlag) {
		/* check Registrar Nonce */
		WscId = WSC_ID_REGISTRAR_NONCE;
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "check Registrar Nonce\n");
	} else {
		/* check Enrollee Nonce */
		WscId = WSC_ID_ENROLLEE_NONCE;
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "check Enrollee Nonce\n");
	}

	pData = pElem->Msg;
	Length = pElem->MsgLen;

	/* We have to look for WSC_IE_MSG_TYPE to classify M2 ~ M8, the remain size must large than 4 */
	while (Length > 4) {
		WSC_IE	TLV_Recv;
		char ZeroNonce[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		if (Length < WscLen + 4) {
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "unexpected WSC IE Length(%u)\n", WscLen);
			break;
		}
		pData  += 4;
		Length -= 4;

		if (WscType == WscId) {
			if (RTMPCompareMemory(pWscControl->RegData.SelfNonce, pData, 16) == 0) {
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Nonce match!!\n");
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscCheckNonce\n");
				return TRUE;
			} else if (NdisEqualMemory(pData, ZeroNonce, 16)) {
				/* Intel external registrar will send WSC_NACK with enrollee nonce */
				/* "10 1A 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00" */
				/* when AP is configured and user selects not to configure AP. */
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Zero Enrollee Nonce!!\n");
				MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscCheckNonce\n");
				return TRUE;
			}
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN, "Nonce mismatch!!\n");
	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscCheckNonce\n");
	return FALSE;
}

VOID WscGetRegDataPIN(
	IN  PRTMP_ADAPTER pAdapter,
	IN  UINT PinCode,
	IN  PWSC_CTRL pWscControl)
{
	UCHAR tempPIN[9] = {0};
	INT ret;

	if ((pWscControl->WscMode == WSC_PBC_MODE) ||
		(pWscControl->WscMode == WSC_SMPBC_MODE))
		pWscControl->WscPinCode = 0;
	else
		pWscControl->WscPinCode = PinCode;

	memset(pWscControl->RegData.PIN, 0, 8);

	if (pWscControl->WscPinCode == 0) {
		ret = snprintf((RTMP_STRING *) tempPIN, sizeof(tempPIN), "00000000");
		if (os_snprintf_error(sizeof(tempPIN), ret))
			MTWF_DBG(pAdapter, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "tempPIN snprintf error!\n");
		memcpy(pWscControl->RegData.PIN, tempPIN, 8);
		pWscControl->RegData.PinCodeLen = 8;
	} else {
		if (pWscControl->WscPinCodeLen == 4) {
			UCHAR	temp4PIN[5] = {0};

			ret = snprintf((RTMP_STRING *) temp4PIN, sizeof(temp4PIN), "%04u", pWscControl->WscPinCode);
			if (os_snprintf_error(sizeof(temp4PIN), ret))
				MTWF_DBG(pAdapter, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "temp4PIN snprintf error!\n");
			memcpy(pWscControl->RegData.PIN, temp4PIN, 4);
			pWscControl->RegData.PinCodeLen = 4;
		} else {
			ret = snprintf((RTMP_STRING *) tempPIN, sizeof(tempPIN), "%08u", pWscControl->WscPinCode);
			if (os_snprintf_error(sizeof(tempPIN), ret))
				MTWF_DBG(pAdapter, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "tempPIN snprintf error!\n");
			memcpy(pWscControl->RegData.PIN, tempPIN, 8);
			pWscControl->RegData.PinCodeLen = 8;
		}
	}

	hex_dump("WscGetRegDataPIN - PIN", pWscControl->RegData.PIN, 8);
}

#ifdef CONFIG_STA_SUPPORT
static VOID WscEapActionDisabled(
	IN  PRTMP_ADAPTER pAdapter,
	IN  PWSC_CTRL pWscControl)
{
	INT DataLen = 0;
	UCHAR *WscData = NULL;

	os_alloc_mem(NULL, &WscData, 256);

	if (WscData == NULL) {
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "WscData alloc fail\n");
		return;
	}

	DataLen = BuildMessageNACK(pAdapter, pWscControl, WscData);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter) {
		if (pWscControl->EntryIfIdx & MIN_NET_DEVICE_FOR_APCLI)
			WscSendMessage(pAdapter, WSC_OPCODE_NACK, WscData, DataLen, pWscControl, AP_CLIENT_MODE, EAP_CODE_RSP);
		else
			WscSendMessage(pAdapter, WSC_OPCODE_NACK, WscData, DataLen, pWscControl, AP_MODE, EAP_CODE_REQ);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAdapter) {
		if (ADHOC_ON(pAdapter) && (pWscControl->WscConfMode & WSC_REGISTRAR))
			WscSendMessage(pAdapter, WSC_OPCODE_NACK, WscData, DataLen, pWscControl, STA_MODE, EAP_CODE_REQ);
		else
			WscSendMessage(pAdapter, WSC_OPCODE_NACK, WscData, DataLen, pWscControl, STA_MODE, EAP_CODE_RSP);
	}
#endif /* CONFIG_STA_SUPPORT */

	if (WscData)
		os_free_mem(WscData);
}
#endif /* CONFIG_STA_SUPPORT */

static VOID WscGetConfigErrFromNack(
	IN RTMP_ADAPTER *pAdapter,
	IN MLME_QUEUE_ELEM * pElem,
	OUT USHORT *pConfigError)
{
	USHORT Length = 0;
	PUCHAR pData;
	USHORT WscType, WscLen, ConfigError = 0;

	pData = pElem->Msg;
	Length = pElem->MsgLen;

	while (Length > 4) {
		WSC_IE	TLV_Recv;

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		if (Length < WscLen + 4) {
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "unexpected WSC IE Length(%u)\n", WscLen);
			break;
		}
		pData  += 4;
		Length -= 4;

		if (WscType == WSC_ID_CONFIG_ERROR) {
			NdisMoveMemory(&ConfigError, pData, sizeof(USHORT));
			MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WSC_ID_CONFIG_ERROR: %d\n", ntohs(ConfigError));
			*pConfigError = ntohs(ConfigError);
			return;
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WSC_ID_CONFIG_ERROR is missing\n");
}


static INT WscSetAuthMode(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR CurOpMode,
	IN  UCHAR apidx,
	IN  RTMP_STRING *arg)
{
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		UINT32	i;
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		struct _SECURITY_CONFIG *pSecConfig = NULL;
		struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

		pSecConfig = &wdev->SecConfig;
		SetWdevAuthMode(pSecConfig, arg);

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]))
#if defined(CONFIG_MAP_SUPPORT) && defined(WPS_UNCONFIG_FEATURE_SUPPORT)
				if (IS_MAP_ENABLE(pAd))
					if (!(pAd->MacTab.Content[i].DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)))
#endif
						tr_ctl->tr_entry[i].PortSecured  = WPA_802_1X_PORT_NOT_SECURED;
		}

		pAd->ApCfg.MBSSID[apidx].wdev.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "IF(ra%d) %s::(AuthMode=0x%x)\n", apidx, __func__, wdev->SecConfig.AKMMap);
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
		struct _SECURITY_CONFIG *pSecConfig = NULL;

		pSecConfig = &wdev->SecConfig;
		SetWdevAuthMode(pSecConfig, arg);
		wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscSetAuthMode::(AuthMode=0x%x)\n", wdev->SecConfig.AKMMap);
	}

#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}

static INT WscSetEncrypType(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR CurOpMode,
	IN  UCHAR apidx,
	IN  RTMP_STRING *arg)
{
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		struct _SECURITY_CONFIG *pSecConfig = NULL;

		pSecConfig = &wdev->SecConfig;
		SetWdevEncrypMode(pSecConfig, arg);
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "IF(ra%d) %s::(EncrypType=0x%x)\n", apidx, __func__, wdev->SecConfig.PairwiseCipher);
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
		struct _SECURITY_CONFIG *pSecConfig = NULL;

		pSecConfig = &wdev->SecConfig;
		SetWdevEncrypMode(pSecConfig, arg);
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscSetEncrypType::(EncrypType=0x%x)\n", wdev->SecConfig.PairwiseCipher);
	}

#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}

#ifdef CONFIG_STA_SUPPORT
USHORT WscGetAuthTypeFromStr(RTMP_STRING *arg)
{
	if (rtstrcasecmp(arg, "OPEN") == TRUE)
		return WSC_AUTHTYPE_OPEN;
	else if (rtstrcasecmp(arg, "SHARED") == TRUE)
		return WSC_AUTHTYPE_SHARED;
	else if (rtstrcasecmp(arg, "WPAPSK") == TRUE)
		return WSC_AUTHTYPE_WPAPSK;
	else if (rtstrcasecmp(arg, "WPA2PSK") == TRUE)
		return WSC_AUTHTYPE_WPA2PSK;

#ifdef WPA_SUPPLICANT_SUPPORT
	else if (rtstrcasecmp(arg, "WPA") == TRUE)
		return WSC_AUTHTYPE_WPA;
	else if (rtstrcasecmp(arg, "WPA2") == TRUE)
		return WSC_AUTHTYPE_WPA2;

#endif /* WPA_SUPPLICANT_SUPPORT */
	else
		return 0;
}

USHORT WscGetEncrypTypeFromStr(RTMP_STRING *arg)
{
	if (rtstrcasecmp(arg, "NONE") == TRUE)
		return WSC_ENCRTYPE_NONE;
	else if (rtstrcasecmp(arg, "WEP") == TRUE)
		return WSC_ENCRTYPE_WEP;
	else if (rtstrcasecmp(arg, "TKIP") == TRUE)
		return WSC_ENCRTYPE_TKIP;
	else if (rtstrcasecmp(arg, "AES") == TRUE)
		return WSC_ENCRTYPE_AES;
	else
		return 0;
}
#endif /* CONFIG_STA_SUPPORT */

/*
*	========================================================================
*
*	Routine Description:
*		Push PBC from HW/SW Buttton
*
*	Arguments:
*		pAd    - NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*
*	========================================================================
*/

VOID  WscPushPBCAction(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl)
{
	BOOLEAN Cancelled;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscPushPBCAction\n");

	/* 0. PBC mode, disregard the SSID information, we have to get the current AP list */
	/*    and check the beacon for Push buttoned AP. */
	/* 1. Cancel old timer to prevent use push continuously */
	if (pWscControl->Wsc2MinsTimerRunning) {
		pWscControl->Wsc2MinsTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
	}

	if (pWscControl->WscScanTimerRunning) {
		pWscControl->WscScanTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscScanTimer, &Cancelled);
	}

	if (pWscControl->WscPBCTimerRunning) {
		pWscControl->WscPBCTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscPBCTimer, &Cancelled);
	}

	/* Set WSC state to WSC_STATE_INIT */
	pWscControl->WscState = WSC_STATE_START;
	pWscControl->WscStatus = STATUS_WSC_SCAN_AP;
	/* Init Registrar pair structures */
	WscInitRegistrarPair(pAd, pWscControl, BSS0);
	/* For PBC, the PIN is all '0' */
	WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl);
	/* 2. Set 2 min timout routine */
	RTMPSetTimer(&pWscControl->Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
	pWscControl->Wsc2MinsTimerRunning = TRUE;
	pWscControl->bWscTrigger = TRUE;	/* start work */
	/* 3. Call WscScan subroutine */
	WscScanExec(pAd, pWscControl);
	/* 4. Set 10 second timer to invoke PBC connection actions. */
#ifdef CON_WPS /*Move to scan complete to trigger //Dung_Ru*/

	if (pWscControl->conWscStatus == CON_WPS_STATUS_DISABLED)
#endif /* CON_WPS */
	{
#ifdef APCLI_SUPPORT

		if (pWscControl->WscApCliScanMode != TRIGGER_PARTIAL_SCAN &&
			!pWscControl->WscPBCTimerRunning)
#endif /* APCLI_SUPPORT */
		{
#ifdef CONFIG_MAP_SUPPORT
			if (!IS_MAP_TURNKEY_ENABLE(pAd))
#endif /* CONFIG_MAP_SUPPORT */
			{
				RTMPSetTimer(&pWscControl->WscPBCTimer, 10000);
				pWscControl->WscPBCTimerRunning = TRUE;
			}
		}
	}
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscPushPBCAction\n");
}


#ifdef WSC_STA_SUPPORT

/*
*	========================================================================
*
*	Routine Description:
*		PIN from SW Buttton
*
*	Arguments:
*		pAd    - NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*
*	========================================================================
*/

VOID WscPINAction(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl)
{
	BOOLEAN Cancelled;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscPINAction\n");

	/* 0. PIN mode, disregard the SSID information, we have to get the current AP list */
	/*    and check the beacon for WSC PIN AP. */

	/* 1. Cancel old timer to prevent use push continuously */
	if (pWscControl->Wsc2MinsTimerRunning) {
		pWscControl->Wsc2MinsTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->Wsc2MinsTimer, &Cancelled);
	}

	if (pWscControl->WscScanTimerRunning) {
		pWscControl->WscScanTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscScanTimer, &Cancelled);
	}

	if (pWscControl->WscPINTimerRunning) {
		pWscControl->WscPINTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscPINTimer, &Cancelled);
	}

	/* Set WSC state to WSC_STATE_INIT */
	pWscControl->WscState = WSC_STATE_START;
	pWscControl->WscStatus = STATUS_WSC_SCAN_AP;
	/* Init Registrar pair structures */
	WscInitRegistrarPair(pAd, pWscControl, BSS0);
	/* For PBC, the PIN is all '0' */
	/* WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl); */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "Enrollee_pin_code-----> %u\n", pWscControl->WscEnrolleePinCode);
	/* 2. Set 2 min timout routine */
	RTMPSetTimer(&pWscControl->Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
	pWscControl->Wsc2MinsTimerRunning = TRUE;
	pWscControl->bWscTrigger = TRUE;	/* start work */
	/* 3. Call WscScan subroutine */
	WscScanExec(pAd, pWscControl);
	/* 4. Set 10 second timer to invoke PIN connection actions. */
	RTMPSetTimer(&pWscControl->WscPINTimer, 10000);
	pWscControl->WscPINTimerRunning = TRUE;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscPINAction\n");
}

#endif

/*
*	========================================================================
*
*	Routine Description:
*		Doing an active scan with empty SSID, the scanened list will
*		be processed in PBCexec or PINexec routines
*
*	Arguments:
*		pAd         - NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*
*	========================================================================
*/
VOID WscScanExec(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl)
{
#ifdef WSC_LED_SUPPORT
	UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
#ifdef APCLI_SUPPORT
	UCHAR if_idx = (pWscControl->EntryIfIdx & 0x0F);
	struct wifi_dev *wdev = (struct wifi_dev *)pWscControl->wdev;
	SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
	BOOLEAN Cancelled = FALSE;
#endif /*APCLI_SUPPORT*/

	/* Prevent infinite loop if conncet time out didn't stop the repeat scan */
	if ((pWscControl->WscStatus == STATUS_WSC_FAIL) ||
		(pWscControl->WscState == WSC_STATE_OFF))
		return;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "!!! WscScanExec !!!\n");
	pWscControl->WscStatus = STATUS_WSC_SCAN_AP;
#ifdef WSC_LED_SUPPORT
	/* The protocol is connecting to a partner. */
	WPSLEDStatus = LED_WPS_IN_PROCESS;
	RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
#endif /* WSC_LED_SUPPORT */
#ifdef APCLI_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (if_idx < MAX_APCLI_NUM) {
#ifdef AP_SCAN_SUPPORT

			if (pWscControl->WscApCliScanMode == TRIGGER_PARTIAL_SCAN) {
				if ((!ScanCtrl->PartialScan.bScanning) &&
					(ScanCtrl->PartialScan.LastScanChannel == 0)) {
					ScanCtrl->PartialScan.pwdev = &pAd->StaCfg[if_idx].wdev;
					ScanCtrl->PartialScan.bScanning = TRUE;
				}
			}

#endif /* AP_SCAN_SUPPORT */
#ifdef CON_WPS
			pAd->StaCfg[if_idx].ConWpsApCliModeScanDoneStatus = CON_WPS_APCLI_SCANDONE_STATUS_ONGOING;
#endif /*CON_WPS*/
			if (scan_in_run_state(pAd, wdev)) {
				RTMPCancelTimer(&ScanCtrl->ScanTimer, &Cancelled);
				sync_fsm_reset(pAd, wdev);
				cntl_fsm_reset(wdev);
			}

			ApSiteSurvey_by_wdev(pAd, NULL, SCAN_WSC_ACTIVE, FALSE, &pAd->StaCfg[if_idx].wdev);
		}
	}
#endif /* APCLI_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		pAd->StaCfg[0].bNotFirstScan = TRUE;
	StaSiteSurvey(pAd, &pWscControl->WscSsid, SCAN_WSC_ACTIVE, (struct wifi_dev *)pWscControl->wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
}

/*
*	========================================================================
*
*	Routine Description:
*		Doing PBC conenction verification, it will check current BSS list
*		and find the correct number of PBC AP. If only 1 exists, it will
*		start to make connection. Otherwise, it will set a scan timer
*		to perform another scan for next PBC connection execution.
*
*	Arguments:
*		pAd         - NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*
*	========================================================================
*/
BOOLEAN	WscPBCExec(
	IN  PRTMP_ADAPTER pAd,
	IN  BOOLEAN bFromM2,
	IN  PWSC_CTRL pWscControl)
{
#ifdef WSC_LED_SUPPORT
	UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG sta_cfg;
#endif
	struct wifi_dev *wdev;
	UCHAR CurOpMode;

	if (pWscControl == NULL)
		return FALSE;

	wdev = (struct wifi_dev *)pWscControl->wdev;
#ifdef CONFIG_STA_SUPPORT
	sta_cfg = GetStaCfgByWdev(pAd, wdev);

	if (sta_cfg)
		CurOpMode = STA_MODE;
	else
#endif /* CONFIG_STA_SUPPORT */
		CurOpMode = AP_MODE;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			 "-----> WscPBCExec (CurOpMode=%d) !!!\n", CurOpMode);
	/* 1. Search the qualified SSID from current SSID list */
	WscPBCBssTableSort(pAd, pWscControl);

	/* 2. Check the qualified AP for connection, if more than 1 AP avaliable, report error. */
	if (pWscControl->WscPBCBssCount != 1) {
		/* Set WSC state to WSC_FAIL */
		pWscControl->WscState = WSC_STATE_FAIL;

		if (pWscControl->WscPBCBssCount == 0) {
			pWscControl->WscStatus = STATUS_WSC_PBC_NO_AP;
#ifdef WSC_LED_SUPPORT
			/* Failed to find any partner. */
			WPSLEDStatus = LED_WPS_ERROR;
			RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
#ifdef CONFIG_WIFI_LED_SUPPORT

			if (LED_MODE(pAd) == WPS_LED_MODE_SHARE)
				RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_WPS_FAIL_WIFI_LED_TIMEOUT);

#endif /* CONFIG_WIFI_LED_SUPPORT */
#endif /* WSC_LED_SUPPORT */
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscPBCExec --> AP list is %d, wait for next time\n",
					 pWscControl->WscPBCBssCount);
#ifdef CONFIG_STA_SUPPORT

			/*
			*	P2P PBC CLI doesn't need to check PBC overlapping,
			*	so we don't need to consider P2P case here.
			*/
			if (sta_cfg && (sta_cfg->BssType == BSS_INFRA))
#endif /* CONFIG_STA_SUPPORT */
			{
				/* 2.1. Set 1 second timer to invoke another scan */
				RTMPSetTimer(&pWscControl->WscScanTimer, 1000);
				pWscControl->WscScanTimerRunning = TRUE;
			}
		} else {
			pWscControl->WscStatus = STATUS_WSC_PBC_TOO_MANY_AP;
				RTMPSendWirelessEvent(pAd, IW_WSC_PBC_SESSION_OVERLAP, NULL, BSS0, 0);

#ifdef WSC_LED_SUPPORT

			if (LED_MODE(pAd) == WPS_LED_MODE_9) { /* WPS LED mode 9. */
				/* In case of the WPS LED mode 9, the UI would abort the connection attempt by making the RT_OID_802_11_WSC_SET_WPS_STATE_MACHINE_TERMINATION request. */
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: Skip the WPS session overlap detected LED indication.\n", __func__);
			} else { /* Other LED mode. */
				/* Session overlap detected. */
				WPSLEDStatus = LED_WPS_SESSION_OVERLAP_DETECTED;
				RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
			}

#endif /* WSC_LED_SUPPORT */
			/*
			*	20101210 - According to the response from WFA:
			*	The station shall not continue scanning waiting for only one registrar to appear
			*/
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscPBCExec --> AP list is %d, stop WPS process!\n",
					 pWscControl->WscPBCBssCount);
			WscStop(pAd,
#ifdef CONFIG_AP_SUPPORT
					FALSE,
#endif /* CONFIG_AP_SUPPORT */
					pWscControl);
			pWscControl->WscConfMode = WSC_DISABLE;
#ifdef CONFIG_STA_SUPPORT
			if (sta_cfg) {
				RTMPZeroMemory(sta_cfg->MlmeAux.AutoReconnectSsid, MAX_LEN_OF_SSID);
				sta_cfg->MlmeAux.AutoReconnectSsidLen = sta_cfg->SsidLen;
				RTMPMoveMemory(sta_cfg->MlmeAux.AutoReconnectSsid, sta_cfg->Ssid, sta_cfg->SsidLen);
			}
#endif /* CONFIG_STA_SUPPORT */
		}

		/* 2.2 We have to quit for now */
		return FALSE;
	}

	if (bFromM2)
		return TRUE;

#ifdef CONFIG_STA_SUPPORT
	/* 3. Now we got the intend AP, Set the WSC state and enqueue the SSID connection command */
	if (sta_cfg)
		sta_cfg->MlmeAux.CurrReqIsFromNdis = FALSE;

	if (!cntl_idle(wdev)) {
		RTEnqueueInternalCmd(pAd, CMDTHREAD_MLME_RESET_STATE_MACHINE, pWscControl, sizeof(WSC_CTRL));
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "!!! WscPBCExec --> MLME busy, reset MLME state machine !!!\n");
	}

#endif /* CONFIG_STA_SUPPORT */
#ifdef WSC_LED_SUPPORT
	/* The protocol is connecting to a partner. */
	WPSLEDStatus = LED_WPS_IN_PROCESS;
	RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
#endif /* WSC_LED_SUPPORT */
#ifdef APCLI_SUPPORT

	if (CurOpMode == STA_MODE) {
		UCHAR apcli_idx = (pWscControl->EntryIfIdx & 0x0F);
		NdisMoveMemory(pWscControl->RegData.SelfInfo.MacAddr,
					   wdev->if_addr,
					   MAC_ADDR_LEN);
		pAd->StaCfg[apcli_idx].ApcliInfStat.Enable = TRUE;
		RTEnqueueInternalCmd(pAd, CMDTHREAD_APCLI_PBC_AP_FOUND, (VOID *)&apcli_idx, sizeof(UCHAR));
		return TRUE;
	}
#endif /* APCLI_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	/* Enqueue BSSID connection command */
	if (CurOpMode == STA_MODE) {
		if (sta_cfg->BssType != BSS_ADHOC)
			cntl_connect_request(wdev, CNTL_CONNECT_BY_BSSID, MAC_ADDR_LEN, (UCHAR *)&pWscControl->WscBssid[0]);
		else
			cntl_connect_request(wdev, CNTL_CONNECT_BY_SSID, sizeof(NDIS_802_11_SSID), (UCHAR *)&pWscControl->WscSsid);
	}

#endif /* CONFIG_STA_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscPBCExec !!!\n");
	return TRUE;
}

#ifdef WSC_STA_SUPPORT
/*
*	========================================================================
*
*	Routine Description:
*		Doing PIN conenction verification, it will check current BSS list
*		and find the correct number of PIN AP. If only 1 exists, it will
*		start to make connection. Otherwise, it will set a scan timer
*		to perform another scan for next PIN connection execution.
*
*	Arguments:
*		pAd         - NIC Adapter pointer
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*
*	========================================================================
*/
BOOLEAN	WscPINExec(
	IN  PRTMP_ADAPTER pAd,
	IN  BOOLEAN bFromM2,
	IN  PWSC_CTRL pWscControl)
{
#ifdef WSC_LED_SUPPORT
	UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
#if defined(CONFIG_STA_SUPPORT)
	UCHAR CurOpMode = AP_MODE;
#endif /* CONFIG_STA_SUPPORT*/
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[0];
#endif

	if (pWscControl == NULL)
		return FALSE;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (pWscControl->EntryIfIdx == BSS0)
			CurOpMode = STA_MODE;
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscPINExec !!!\n");
	/* 1. Search the qualified SSID from current SSID list */
	WscPINBssTableSort(pAd, pWscControl);

	/* 2. Check the qualified AP for connection, if more than 1 AP with same mac addr avaliable, report error. */
	if (pWscControl->WscPINBssCount != 1) {
		/* Set WSC state to WSC_FAIL */
		pWscControl->WscState = WSC_STATE_FAIL;

		if (pWscControl->WscPINBssCount == 0) {
			pWscControl->WscStatus = STATUS_WSC_PBC_NO_AP;	/*No error code regarding PIN, so use PBC error code*/
#ifdef WSC_LED_SUPPORT
			/* Failed to find any partner. */
			WPSLEDStatus = LED_WPS_ERROR;
			RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
#ifdef CONFIG_WIFI_LED_SUPPORT

			if (LED_MODE(pAd) == WPS_LED_MODE_SHARE)
				RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_WPS_FAIL_WIFI_LED_TIMEOUT);

#endif /* CONFIG_WIFI_LED_SUPPORT */
#endif /* WSC_LED_SUPPORT */
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscPINExec --> AP list is %d, wait for next time\n",
					 pWscControl->WscPINBssCount);
#ifdef CONFIG_STA_SUPPORT

			if (pAd->StaCfg[0].BssType == BSS_INFRA)
#endif /* CONFIG_STA_SUPPORT */
			{
				/* 2.1. Set 1 second timer to invoke another scan */
				RTMPSetTimer(&pWscControl->WscScanTimer, 1000);
				pWscControl->WscScanTimerRunning = TRUE;
			}
		} else {
			/* should never hit this case, as in PIN, bssid is preset */
			pWscControl->WscStatus = STATUS_WSC_PBC_TOO_MANY_AP;
				RTMPSendWirelessEvent(pAd, IW_WSC_PBC_SESSION_OVERLAP, NULL, BSS0, 0);

#ifdef WSC_LED_SUPPORT

			if (LED_MODE(pAd) == WPS_LED_MODE_9) { /* WPS LED mode 9. */
				/* In case of the WPS LED mode 9, the UI would abort the connection attempt by making the RT_OID_802_11_WSC_SET_WPS_STATE_MACHINE_TERMINATION request. */
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: Skip the WPS session overlap detected LED indication.\n", __func__);
			} else { /* Other LED mode. */
				/* Session overlap detected. */
				WPSLEDStatus = LED_WPS_SESSION_OVERLAP_DETECTED;
				RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
			}

#endif /* WSC_LED_SUPPORT */
			/*
			*	20101210 - According to the response from WFA:
			*	The station shall not continue scanning waiting for only one registrar to appear
			*/
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscPINExec --> AP list is %d, stop WPS process!\n",
					 pWscControl->WscPINBssCount);
			WscStop(pAd,
#ifdef CONFIG_AP_SUPPORT
					FALSE,
#endif /* CONFIG_AP_SUPPORT */
					pWscControl);
			pWscControl->WscConfMode = WSC_DISABLE;
#ifdef CONFIG_STA_SUPPORT
			RTMPZeroMemory(pStaCfg->MlmeAux.AutoReconnectSsid, MAX_LEN_OF_SSID);
			pStaCfg->MlmeAux.AutoReconnectSsidLen = pStaCfg->SsidLen;
			RTMPMoveMemory(pStaCfg->MlmeAux.AutoReconnectSsid, pStaCfg->Ssid, pStaCfg->SsidLen);
#endif /* CONFIG_STA_SUPPORT */
		}

		/* 2.2 We have to quit for now */
		return FALSE;
	}

	if (bFromM2) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "bfromM2\n");
		return TRUE;
	}

#ifdef CONFIG_STA_SUPPORT
	/* 3. Now we got the intend AP, Set the WSC state and enqueue the SSID connection command */
	pStaCfg->MlmeAux.CurrReqIsFromNdis = FALSE;

	if (!cntl_idle(&pAd->StaCfg[0].wdev)) {
		RTMP_MLME_RESET_STATE_MACHINE(pAd, &pAd->StaCfg[0].wdev);
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "!!! WscPINExec --> MLME busy, reset MLME state machine !!!\n");
	}

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	/* 4. Set WSC state to WSC_STATE_START */
	if (CurOpMode == STA_MODE) {
		pWscControl->WscState = WSC_STATE_START;
		pWscControl->WscStatus = STATUS_WSC_START_ASSOC;
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "wsc_state=start !!!\n");
	}

#endif /* CONFIG_STA_SUPPORT */
#ifdef WSC_LED_SUPPORT
	/* The protocol is connecting to a partner. */
	WPSLEDStatus = LED_WPS_IN_PROCESS;
	RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
#endif /* WSC_LED_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	/* Enqueue BSSID connection command */
	if (CurOpMode == STA_MODE) {
		if (pStaCfg->BssType == BSS_INFRA) {
			MlmeEnqueueWithWdev(pAd,
								MLME_CNTL_STATE_MACHINE,
								OID_802_11_BSSID,
								sizeof(NDIS_802_11_MAC_ADDRESS),
								(VOID *)&pWscControl->WscBssid[0], 0, &pStaCfg->wdev);
		} else {
			MlmeEnqueueWithWdev(pAd,
								MLME_CNTL_STATE_MACHINE,
								OID_802_11_SSID,
								sizeof(NDIS_802_11_SSID),
								(VOID *)&pWscControl->WscSsid, 0, &pStaCfg->wdev);
		}
	}

#endif /* CONFIG_STA_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscPINExec !!!\n");
	return TRUE;
}
#endif

static VOID WscCheckIsSameAP(
	IN  UUID_BSSID_CH_INFO	*ApUuidBssid,
	IN  UUID_BSSID_CH_INFO	*TmpInfo,
	IN  UCHAR *zeros16,
	OUT BOOLEAN *bSameAP)
{
	if (RTMPCompareMemory(&ApUuidBssid->Uuid[0], &TmpInfo->Uuid[0], 16) == 0) {
		if (RTMPCompareMemory(&TmpInfo->Uuid[0], zeros16, 16) != 0) {
			/*
			*	Same UUID, indicate concurrent AP
			*	We can indicate 1 AP only.
			*/
			*bSameAP = TRUE;
		} else if (RTMPCompareMemory(&TmpInfo->Uuid[0], zeros16, 16) == 0) {
			if (ApUuidBssid->Band != TmpInfo->Band) {
				if (RTMPCompareMemory(&ApUuidBssid->Bssid[0], &TmpInfo->Bssid[0], 5) == 0) {
					/*
					*	Zero UUID at different band, and first 5bytes of two BSSIDs are the same.
					*	Indicate concurrent AP, we can indicate 1 AP only.
					*/
					*bSameAP = TRUE;
				}
			}
		}
	} else if ((RTMPCompareMemory(&TmpInfo->Uuid[0], zeros16, 16) == 0) ||
			   (RTMPCompareMemory(&ApUuidBssid->Uuid[0], zeros16, 16) == 0)) {
		if ((RTMPCompareMemory(&ApUuidBssid->Bssid[0], &TmpInfo->Bssid[0], 5) == 0) &&
			(ApUuidBssid->Band != TmpInfo->Band)) {
			INT tmpDiff = (INT)(ApUuidBssid->Bssid[5] - TmpInfo->Bssid[5]);

			/*
			*	Zero UUID and Non-zero UUID at different band, and two BSSIDs are very close.
			*	Indicate concurrent AP, we can indicate 1 AP only.
			*/
			if ((tmpDiff <= 4) ||
				(tmpDiff >= -4))
				*bSameAP = TRUE;
		}
	}
}

BOOLEAN WscBssWpsIESearchForPBC(
	RTMP_ADAPTER *pAd,
	WSC_CTRL *pWscControl,
	BSS_ENTRY *pInBss,
	UUID_BSSID_CH_INFO ApUuidBssid[],
	INT VarIeLen,
	PUCHAR pVar)
{
	INT j = 0, Len = 0, idx = 0;
	BOOLEAN bFound, bSameAP, bSelReg;
	PUCHAR pData = NULL;
	PBEACON_EID_STRUCT pEid;
	USHORT DevicePasswordID;
	PWSC_IE pWscIE;
	UUID_BSSID_CH_INFO TmpInfo;
	UCHAR zeros16[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	BOOLEAN ret = FALSE;
#ifdef IWSC_SUPPORT
	UINT8 RspType = 0;
	BOOLEAN bEntryAcceptable = FALSE;
#endif /* IWSC_SUPPORT */
#ifdef CON_WPS
	UCHAR apcli_idx = 0;
	struct wifi_dev *Reqwdev = NULL;
	UCHAR dev_band = 0;
#endif /*CON_WPS Dung_Ru*/
	pData   = pVar;
	bFound  = FALSE;
	bSameAP = FALSE;
	bSelReg = FALSE;
	Len = VarIeLen;
	NdisZeroMemory(&TmpInfo, sizeof(UUID_BSSID_CH_INFO));
#ifdef CON_WPS

	if (pWscControl->EntryIfIdx >= MIN_NET_DEVICE_FOR_APCLI) {
		apcli_idx = (pWscControl->EntryIfIdx & 0x0F);
		Reqwdev = &(pAd->StaCfg[apcli_idx].wdev);

		if (Reqwdev)
			dev_band = HcGetBandByWdev(Reqwdev);
	}

#endif

	if (Len == 0 || Len > MAX_VIE_LEN)
		return ret;

	while ((Len > 0) && (bFound == FALSE)) {
		pEid = (PBEACON_EID_STRUCT) pData;

		/* No match, skip the Eid and move forward, IE_WFA_WSC = 0xdd */
		if (pEid->Eid != IE_WFA_WSC) {
			/* Set the offset and look for next IE */
			pData += (pEid->Len + 2);
			Len   -= (pEid->Len + 2);
			continue;
		} else {
			/* Found IE with 0xdd */
			/* check for WSC OUI -- 00 50 f2 04 */
			if ((NdisEqualMemory(pEid->Octet, WPS_OUI, 4) == FALSE)
#ifdef IWSC_SUPPORT
				&& (NdisEqualMemory(pEid->Octet, IWSC_OUI, 4) == FALSE)
#endif /* IWSC_SUPPORT */
			   ) {
				/* Set the offset and look for next IE */
				pData += (pEid->Len + 2);
				Len   -= (pEid->Len + 2);
				continue;
			}
		}

		/* 3. Found	AP with WSC IE in beacons, skip 6 bytes = 1 + 1 + 4 */
		pData += 6;
		Len   -= 6;

		if (Len == 0 || Len > MAX_VIE_LEN)
			return ret;
		/* 4. Start to look the PBC type within WSC VarIE */
		while (Len > 0) {
			/* Check for WSC IEs */
			pWscIE = (PWSC_IE) pData;

			if (Len < (be2cpu16(pWscIE->Length) + 4)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
					"unexpected WSC IE Length(%u)\n", be2cpu16(pWscIE->Length));
				break;
			}

			if (be2cpu16(pWscIE->Type) == WSC_ID_SEL_REGISTRAR) {
				hex_dump("SelReg:", pData, 5);
				bSelReg = pWscIE->Data[0];
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "bSelReg = %d\n", bSelReg);
			}

#ifdef IWSC_SUPPORT

			if ((pAd->OpMode == OPMODE_STA) &&
				(pAd->StaCfg[0].BssType == BSS_ADHOC)) {
				if (be2cpu16(pWscIE->Type) == WSC_ID_RESP_TYPE) {
					RspType = pWscIE->Data[0];

					if (RspType < WSC_MSGTYPE_REGISTRAR) {
						bFound = FALSE;
						break;
					}

					TmpInfo.RspType = RspType;
				}

				if (be2cpu16(pWscIE->Type) == WSC_ID_MAC_ADDR) {
					UCHAR mac_addr[MAC_ADDR_LEN];

					RTMPMoveMemory(mac_addr, (pData + 4), MAC_ADDR_LEN);

					if (NdisCmpMemory(pInBss->MacAddr, mac_addr, MAC_ADDR_LEN)) {
						bFound = FALSE;
						break;
					}
				}

				if (be2cpu16(pWscIE->Type) == WSC_ID_ENTRY_ACCEPTABLE) {
					hex_dump("EntryAcceptable:", pData, 5);
					bEntryAcceptable = pWscIE->Data[0];
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "bEntryAcceptable = %d\n", bEntryAcceptable);
				}
			}

#endif /* IWSC_SUPPORT */

			/* Check for device password ID, PBC = 0x0004, SMPBC = 0x0006 */
			if (be2cpu16(pWscIE->Type) == WSC_ID_DEVICE_PWD_ID) {
				/* Found device password ID */
#ifdef WINBOND
				/*The Winbond's platform will fail to retrive 2-bytes data, if use the original */
				/*be2cpu16<-- */
				DevicePasswordID = WINBON_GET16((PUCHAR)&pWscIE->Data[0]);
#else
				DevicePasswordID = be2cpu16(get_unaligned((USHORT *)&pWscIE->Data[0]));
				/*DevicePasswordID = be2cpu16(*((USHORT *) &pWscIE->Data[0])); */
#endif /* WINBOND */
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscPBCBssTableSort : DevicePasswordID = 0x%04x\n", DevicePasswordID);

				if (((pWscControl->WscMode == WSC_PBC_MODE) && (DevicePasswordID == DEV_PASS_ID_PBC)) ||
					((pWscControl->WscMode == WSC_SMPBC_MODE) && (DevicePasswordID == DEV_PASS_ID_SMPBC))) {
					/* Found matching PBC AP in current list, add it into table and add the count */
					bFound = TRUE;
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "DPID=PBC Found -->\n");
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "#  Bssid "MACSTR"\n",
							 MAC2STR(pInBss->Bssid));

					if (pInBss->Channel > 14)
						TmpInfo.Band = WSC_RFBAND_50GHZ;
					else
						TmpInfo.Band = WSC_RFBAND_24GHZ;

					RTMPMoveMemory(&TmpInfo.Bssid[0], &pInBss->Bssid[0], MAC_ADDR_LEN);
					TmpInfo.Channel = pInBss->Channel;
					RTMPZeroMemory(&TmpInfo.Ssid[0], MAX_LEN_OF_SSID);
					RTMPMoveMemory(&TmpInfo.Ssid[0], &pInBss->Ssid[0], pInBss->SsidLen);
					TmpInfo.SsidLen = pInBss->SsidLen;
				}
			}

			/* UUID_E is optional for beacons, but mandatory for probe-request */
			if (be2cpu16(pWscIE->Type) == WSC_ID_UUID_E) {
				/* Avoid error UUID-E storage from PIN mode */
				RTMPMoveMemory(&TmpInfo.Uuid[0], (UCHAR *)(pData + 4), 16);
			}

			/* Set the offset and look for PBC information */
			/* Since Type and Length are both short type, we need to offset 4, not 2 */
			pData += (be2cpu16(pWscIE->Length) + 4);
			Len   -= (be2cpu16(pWscIE->Length) + 4);
		}

#ifdef CON_WPS
#ifdef CONFIG_MAP_SUPPORT
		if (Reqwdev && IS_MAP_TURNKEY_ENABLE(pAd)
			&& (map_rc_get_band_idx_by_chan(pAd, pInBss->Channel) != HcGetBandByWdev(Reqwdev))) {
			if (bFound == TRUE)
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				"FIND AP TmpInfo.Band =%d ssid=%s but skip(band=%d wdev band=%d)\n",
				TmpInfo.Band, TmpInfo.Ssid,
				map_rc_get_band_idx_by_chan(pAd, pInBss->Channel),
				HcGetBandByWdev(Reqwdev));
			bFound = FALSE;
		}
#endif /* CONFIG_MAP_SUPPORT */
#endif /*CON_WPS*/
#ifdef IWSC_SUPPORT

		if ((pAd->StaCfg[0].BssType == BSS_ADHOC) &&
			(pWscControl->WscMode == WSC_SMPBC_MODE) &&
			(bEntryAcceptable == FALSE) && bFound)
			bFound = FALSE;

#endif /* IWSC_SUPPORT */

		if ((bFound == TRUE) && (bSelReg == TRUE)) {
			if (pWscControl->WscPBCBssCount == 8)
				break;

			if (pWscControl->WscPBCBssCount > 0) {
				for (j = 0; j < pWscControl->WscPBCBssCount; j++) {
					WscCheckIsSameAP(&ApUuidBssid[j], &TmpInfo, zeros16, &bSameAP);

					if (bSameAP)
						break;
				}
			}

			if (bSameAP) {
				if ((pWscControl->WpsApBand == PREFERRED_WPS_AP_PHY_TYPE_2DOT4_G_FIRST) &&
					(TmpInfo.Band == WSC_RFBAND_24GHZ) &&
					(ApUuidBssid[j].Band != TmpInfo.Band)) {
					RTMPMoveMemory(&(ApUuidBssid[j].Bssid[0]), &TmpInfo.Bssid[0], MAC_ADDR_LEN);
					RTMPZeroMemory(&(ApUuidBssid[j].Ssid[0]), MAX_LEN_OF_SSID);
					RTMPMoveMemory(&(ApUuidBssid[j].Ssid[0]), &TmpInfo.Ssid[0], TmpInfo.SsidLen);
					ApUuidBssid[j].SsidLen = TmpInfo.SsidLen;
					ApUuidBssid[j].Channel = TmpInfo.Channel;
				} else if ((pWscControl->WpsApBand == PREFERRED_WPS_AP_PHY_TYPE_5_G_FIRST) &&
						   (TmpInfo.Band == WSC_RFBAND_50GHZ) &&
						   (ApUuidBssid[j].Band != TmpInfo.Band)) {
					RTMPMoveMemory(&(ApUuidBssid[j].Bssid[0]), &TmpInfo.Bssid[0], MAC_ADDR_LEN);
					RTMPZeroMemory(&(ApUuidBssid[j].Ssid[0]), MAX_LEN_OF_SSID);
					RTMPMoveMemory(&(ApUuidBssid[j].Ssid[0]), &TmpInfo.Ssid[0], TmpInfo.SsidLen);
					ApUuidBssid[j].SsidLen = TmpInfo.SsidLen;
					ApUuidBssid[j].Channel = TmpInfo.Channel;
				}
			}

			if (bSameAP == FALSE) {
				UCHAR index = pWscControl->WscPBCBssCount;
				/* Store UUID */
				RTMPMoveMemory(&(ApUuidBssid[index].Uuid[0]), &TmpInfo.Uuid[0], 16);
				RTMPMoveMemory(&(ApUuidBssid[index].Bssid[0]), &pInBss->Bssid[0], MAC_ADDR_LEN);
				RTMPZeroMemory(&(ApUuidBssid[index].Ssid[0]), MAX_LEN_OF_SSID);
				RTMPMoveMemory(&(ApUuidBssid[index].Ssid[0]), &pInBss->Ssid[0], pInBss->SsidLen);
				ApUuidBssid[index].SsidLen = pInBss->SsidLen;
				ApUuidBssid[index].Channel = pInBss->Channel;

				if (ApUuidBssid[index].Channel > 14)
					ApUuidBssid[index].Band = WSC_RFBAND_50GHZ;
				else
					ApUuidBssid[index].Band = WSC_RFBAND_24GHZ;

				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "UUID-E= ");

				for (idx = 0; idx < 16; idx++)
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "%02x  ", ApUuidBssid[index].Uuid[idx]);

				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, " SSID: %s, CH: %d ", ApUuidBssid[index].Ssid, ApUuidBssid[index].Channel);

				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "\n");
				pWscControl->WscPBCBssCount++;
			}
		}
	}

	ret = (bFound && bSelReg);
	return ret;
}

#ifdef WSC_STA_SUPPORT
BOOLEAN WscBssWpsIESearchForPIN(
	RTMP_ADAPTER *pAd,
	WSC_CTRL *pWscControl,
	BSS_ENTRY *pInBss,
	UUID_BSSID_CH_INFO ApUuidBssid[],
	INT VarIeLen,
	PUCHAR pVar)
{
	INT j = 0, Len = 0, idx = 0;
	BOOLEAN bFound, bSameAP, bSelReg, bSelWpsPIN1;
	PUCHAR pData = NULL;
	PBEACON_EID_STRUCT pEid;
	USHORT DevicePasswordID;
	PWSC_IE pWscIE;
	UUID_BSSID_CH_INFO TmpInfo;
	UCHAR zeros16[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	BOOLEAN ret = FALSE;
#ifdef IWSC_SUPPORT
	UINT8 RspType = 0;
	BOOLEAN bEntryAcceptable = FALSE;
#endif /* IWSC_SUPPORT */
	pData   = pVar;
	bFound  = FALSE;
	bSameAP = FALSE;
	bSelReg = FALSE;
	bSelWpsPIN1 = FALSE;
	Len = VarIeLen;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s\n", __func__);
	NdisZeroMemory(&TmpInfo, sizeof(UUID_BSSID_CH_INFO));

	if (Len == 0 || Len > MAX_VIE_LEN)
		return ret;

	while ((Len > 0) && (bFound == FALSE)) {
		pEid = (PBEACON_EID_STRUCT) pData;

		/* No match, skip the Eid and move forward, IE_WFA_WSC = 0xdd */
		if (pEid->Eid != IE_WFA_WSC) {
			/* Set the offset and look for next IE */
			pData += (pEid->Len + 2);
			Len   -= (pEid->Len + 2);
			continue;
		} else {
			/* Found IE with 0xdd */
			/* check for WSC OUI -- 00 50 f2 04 */
			if ((NdisEqualMemory(pEid->Octet, WPS_OUI, 4) == FALSE)
#ifdef IWSC_SUPPORT
				&& (NdisEqualMemory(pEid->Octet, IWSC_OUI, 4) == FALSE)
#endif /* IWSC_SUPPORT */
			   ) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: wpsIE AP not found\n", __func__);
				/* Set the offset and look for next IE */
				pData += (pEid->Len + 2);
				Len   -= (pEid->Len + 2);
				continue;
			}
		}

		/* 3. Found	AP with WSC IE in beacons, skip 6 bytes = 1 + 1 + 4 */
		pData += 6;
		Len   -= 6;

		if (pWscControl->ScanCountToincludeWPSPin1 > 3)
			bSelWpsPIN1 = TRUE;

		if (Len == 0 || Len > MAX_VIE_LEN)
			return ret;
		/* 4. Start to look the PIN type within WSC VarIE */
		while (Len > 0) {
			/* Check for WSC IEs */
			pWscIE = (PWSC_IE) pData;

			if (Len < (be2cpu16(pWscIE->Length) + 4)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
					"unexpected WSC IE Length(%u)\n", be2cpu16(pWscIE->Length));
				break;
			}

			if (be2cpu16(pWscIE->Type) == WSC_ID_SEL_REGISTRAR) {
				hex_dump("SelReg:", pData, 5);
				bSelReg = pWscIE->Data[0];
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "bSelReg = %d\n", bSelReg);
			}

#ifdef IWSC_SUPPORT

			if ((pAd->OpMode == OPMODE_STA) &&
				(pAd->StaCfg[0].BssType == BSS_ADHOC)) {
				if (be2cpu16(pWscIE->Type) == WSC_ID_RESP_TYPE) {
					RspType = pWscIE->Data[0];

					if (RspType < WSC_MSGTYPE_REGISTRAR) {
						bFound = FALSE;
						break;
					}

					TmpInfo.RspType = RspType;
				}

				if (be2cpu16(pWscIE->Type) == WSC_ID_MAC_ADDR) {
					UCHAR mac_addr[MAC_ADDR_LEN];

					RTMPMoveMemory(mac_addr, (pData + 4), MAC_ADDR_LEN);

					if (NdisCmpMemory(pInBss->MacAddr, mac_addr, MAC_ADDR_LEN)) {
						bFound = FALSE;
						break;
					}
				}

				if (be2cpu16(pWscIE->Type) == WSC_ID_ENTRY_ACCEPTABLE) {
					hex_dump("EntryAcceptable:", pData, 5);
					bEntryAcceptable = pWscIE->Data[0];
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "bEntryAcceptable = %d\n", bEntryAcceptable);
				}
			}

#endif /* IWSC_SUPPORT */

			/* Check for device password ID, PIN = 0x0000 */
			if (be2cpu16(pWscIE->Type) == WSC_ID_DEVICE_PWD_ID) {
				/* Found device password ID */
#ifdef WINBOND
				/*The Winbond's platform will fail to retrive 2-bytes data, if use the original */
				/*be2cpu16<-- */
				DevicePasswordID = WINBON_GET16((PUCHAR)&pWscIE->Data[0]);
#else
				DevicePasswordID = be2cpu16(get_unaligned((USHORT *)&pWscIE->Data[0]));
				/*DevicePasswordID = be2cpu16(*((USHORT *) &pWscIE->Data[0])); */
#endif /* WINBOND */
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscPINBssTableSort : DevicePasswordID = 0x%04x\n", DevicePasswordID);

				if ((pWscControl->WscMode == WSC_PIN_MODE) && (DevicePasswordID == DEV_PASS_ID_PIN)) {
					/* Found matching PIN AP in current list, add it into table and add the count */
					bFound = TRUE;
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "DPID=PIN Found -->\n");
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "#  Bssid "MACSTR"\n",
							 MAC2STR(pInBss->Bssid));

					if (pInBss->Channel > 14)
						TmpInfo.Band = WSC_RFBAND_50GHZ;
					else
						TmpInfo.Band = WSC_RFBAND_24GHZ;

					RTMPMoveMemory(&TmpInfo.Bssid[0], &pInBss->Bssid[0], MAC_ADDR_LEN);
					TmpInfo.Channel = pInBss->Channel;
					RTMPZeroMemory(&TmpInfo.Ssid[0], MAX_LEN_OF_SSID);
					RTMPMoveMemory(&TmpInfo.Ssid[0], &pInBss->Ssid[0], pInBss->SsidLen);
					TmpInfo.SsidLen = pInBss->SsidLen;
				}
			}

			/* UUID_E is optional for beacons, but mandatory for probe-request */
			if (be2cpu16(pWscIE->Type) == WSC_ID_UUID_E) {
				/* Avoid error UUID-E storage from PIN mode */
				RTMPMoveMemory(&TmpInfo.Uuid[0], (UCHAR *)(pData + 4), 16);
			}

			/* Set the offset and look for PIN information */
			/* Since Type and Length are both short type, we need to offset 4, not 2 */
			pData += (be2cpu16(pWscIE->Length) + 4);
			Len   -= (be2cpu16(pWscIE->Length) + 4);
		}

#ifdef IWSC_SUPPORT

		if ((pAd->StaCfg[0].BssType == BSS_ADHOC) &&
			(pWscControl->WscMode == WSC_SMPBC_MODE) &&
			(bEntryAcceptable == FALSE) && bFound)
			bFound = FALSE;

#endif /* IWSC_SUPPORT */

		if ((bFound == TRUE) && (bSelReg == TRUE)) {
			if (pWscControl->WscPINBssCount == 8)
				break;

			if (pWscControl->WscPINBssCount > 0) {
				for (j = 0; j < pWscControl->WscPINBssCount; j++) {
					WscCheckIsSameAP(&ApUuidBssid[j], &TmpInfo, zeros16, &bSameAP);

					if (bSameAP)
						break;
				}
			}

			if (bSameAP) {
				if ((pWscControl->WpsApBand == PREFERRED_WPS_AP_PHY_TYPE_2DOT4_G_FIRST) &&
					(TmpInfo.Band == WSC_RFBAND_24GHZ) &&
					(ApUuidBssid[j].Band != TmpInfo.Band)) {
					RTMPMoveMemory(&(ApUuidBssid[j].Bssid[0]), &TmpInfo.Bssid[0], MAC_ADDR_LEN);
					RTMPZeroMemory(&(ApUuidBssid[j].Ssid[0]), MAX_LEN_OF_SSID);
					RTMPMoveMemory(&(ApUuidBssid[j].Ssid[0]), &TmpInfo.Ssid[0], TmpInfo.SsidLen);
					ApUuidBssid[j].SsidLen = TmpInfo.SsidLen;
					ApUuidBssid[j].Channel = TmpInfo.Channel;
				} else if ((pWscControl->WpsApBand == PREFERRED_WPS_AP_PHY_TYPE_5_G_FIRST) &&
						   (TmpInfo.Band == WSC_RFBAND_50GHZ) &&
						   (ApUuidBssid[j].Band != TmpInfo.Band)) {
					RTMPMoveMemory(&(ApUuidBssid[j].Bssid[0]), &TmpInfo.Bssid[0], MAC_ADDR_LEN);
					RTMPZeroMemory(&(ApUuidBssid[j].Ssid[0]), MAX_LEN_OF_SSID);
					RTMPMoveMemory(&(ApUuidBssid[j].Ssid[0]), &TmpInfo.Ssid[0], TmpInfo.SsidLen);
					ApUuidBssid[j].SsidLen = TmpInfo.SsidLen;
					ApUuidBssid[j].Channel = TmpInfo.Channel;
				}
			}

			if (bSameAP == FALSE) {
				UCHAR index = pWscControl->WscPINBssCount;
				/* Store UUID */
				RTMPMoveMemory(&(ApUuidBssid[index].Uuid[0]), &TmpInfo.Uuid[0], 16);
				RTMPMoveMemory(&(ApUuidBssid[index].Bssid[0]), &pInBss->Bssid[0], MAC_ADDR_LEN);
				RTMPZeroMemory(&(ApUuidBssid[index].Ssid[0]), MAX_LEN_OF_SSID);
				RTMPMoveMemory(&(ApUuidBssid[index].Ssid[0]), &pInBss->Ssid[0], pInBss->SsidLen);
				ApUuidBssid[index].SsidLen = pInBss->SsidLen;
				ApUuidBssid[index].Channel = pInBss->Channel;

				if (ApUuidBssid[index].Channel > 14)
					ApUuidBssid[index].Band = WSC_RFBAND_50GHZ;
				else
					ApUuidBssid[index].Band = WSC_RFBAND_24GHZ;

				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "UUID-E= ");

				for (idx = 0; idx < 16; idx++)
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "%02x  ", ApUuidBssid[index].Uuid[idx]);

				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "\n");
				pWscControl->WscPINBssCount++;
			}
		} else if (bSelWpsPIN1 == TRUE) {
			UCHAR index = pWscControl->WscPINBssCount;
			/* Store UUID */
			RTMPMoveMemory(&(ApUuidBssid[index].Uuid[0]), &TmpInfo.Uuid[0], 16);
			RTMPMoveMemory(&(ApUuidBssid[index].Bssid[0]), &pInBss->Bssid[0], MAC_ADDR_LEN);
			RTMPZeroMemory(&(ApUuidBssid[index].Ssid[0]), MAX_LEN_OF_SSID);
			RTMPMoveMemory(&(ApUuidBssid[index].Ssid[0]), &pInBss->Ssid[0], pInBss->SsidLen);
			ApUuidBssid[index].SsidLen = pInBss->SsidLen;
			ApUuidBssid[index].Channel = pInBss->Channel;

			if (ApUuidBssid[index].Channel > 14)
				ApUuidBssid[index].Band = WSC_RFBAND_50GHZ;
			else
				ApUuidBssid[index].Band = WSC_RFBAND_24GHZ;

			pWscControl->WscPINBssCount++;
		}
	}

	ret = ((bFound && bSelReg) || (bSelWpsPIN1));
	return ret;
}
#endif

/*
*	========================================================================
*
*	Routine Description:
*		Find WSC PBC activated AP list
*
*	Arguments:
*		pAd         - NIC Adapter pointer
*		OutTab		- Qualified AP BSS table
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		All these constants are defined in wsc.h
*
*	========================================================================
*/
VOID WscPBCBssTableSort(
	IN RTMP_ADAPTER *pAd,
	IN WSC_CTRL *pWscControl)
{
	UINT i;
	BSS_ENTRY *pInBss;
	UUID_BSSID_CH_INFO *ApUuidBssid = NULL;
	BOOLEAN rv = FALSE;
	UCHAR wdevBand = 0;
	struct wifi_dev *wdev = (struct wifi_dev *)pWscControl->wdev;
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

#ifdef CONFIG_STA_SUPPORT
	STA_ADMIN_CONFIG *sta_cfg = GetStaCfgByWdev(pAd, wdev);

	ASSERT(sta_cfg);
	if (sta_cfg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "sta_cfg is null !!!\n");
		return;
	}

#endif
	wdevBand = HcGetBandByWdev(wdev);
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ApUuidBssid, sizeof(UUID_BSSID_CH_INFO) * 8);
	if (ApUuidBssid == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "Allocate memory fail!!!\n");
		return;
	}

	NdisZeroMemory(&ApUuidBssid[0], sizeof(UUID_BSSID_CH_INFO));
	pWscControl->WscPBCBssCount = 0;

	for (i = 0; i < ScanTab->BssNr; i++) {
		/* BSS entry for VarIE processing */
		pInBss  = (BSS_ENTRY *) &ScanTab->BssEntry[i];

		/* 1. Check VarIE length */
		if (pInBss->VarIELen == 0)
			continue;

#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_MAP_SUPPORT
		if (wdevBand != map_rc_get_band_idx_by_chan(pAd, pInBss->Channel))
			continue;
#else
		if (wdevBand != HcGetBandByChannelRange(pAd, pInBss->Channel))
			continue;

#endif
		if (pInBss->BssType != sta_cfg->BssType)
			continue;
#endif /* CONFIG_STA_SUPPORT */

		/* 2. Search for WSC IE - 0xdd xx 00 50 f2 04 */
		rv = WscBssWpsIESearchForPBC(pAd,
									 pWscControl,
									 pInBss,
									 ApUuidBssid,
									 pInBss->VarIELen,
									 pInBss->VarIEs);

		if (rv == FALSE) {
			WscBssWpsIESearchForPBC(pAd,
									pWscControl,
									pInBss,
									ApUuidBssid,
									pInBss->VarIeFromProbeRspLen,
									pInBss->pVarIeFromProbRsp);
		}
	}

	if (pWscControl->WscPBCBssCount == 1) {
		RTMPZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));
		RTMPMoveMemory(pWscControl->WscSsid.Ssid, ApUuidBssid[0].Ssid, ApUuidBssid[0].SsidLen);
		pWscControl->WscSsid.SsidLength = ApUuidBssid[0].SsidLen;
		RTMPZeroMemory(pWscControl->WscBssid, MAC_ADDR_LEN);
		RTMPMoveMemory(pWscControl->WscBssid, ApUuidBssid[0].Bssid, MAC_ADDR_LEN);
		RTMPMoveMemory(pWscControl->WscPeerUuid, ApUuidBssid[0].Uuid, sizeof(ApUuidBssid[0].Uuid));
#ifdef CONFIG_STA_SUPPORT
		RTMPZeroMemory(pWscControl->WscPeerMAC, MAC_ADDR_LEN);
		RTMPMoveMemory(pWscControl->WscPeerMAC, ApUuidBssid[0].MacAddr, MAC_ADDR_LEN);
		sta_cfg->MlmeAux.Channel = ApUuidBssid[0].Channel;
#endif /* CONFIG_STA_SUPPORT */
#ifdef APCLI_SUPPORT
		if (IF_COMBO_HAVE_AP_STA(pAd)) {
			sta_cfg->MlmeAux.Channel = ApUuidBssid[0].Channel;
			COPY_MAC_ADDR(sta_cfg->CfgApCliBssid, pWscControl->WscBssid);
		}
		printk("will connect %s ("MACSTR") on %d\n", pWscControl->WscSsid.Ssid, MAC2STR(pWscControl->WscBssid), ApUuidBssid[0].Channel);

#endif /* APCLI_SUPPORT */
	}

	if (ApUuidBssid != NULL)
		os_free_mem(ApUuidBssid);

#ifdef IWSC_SUPPORT

	if (pWscControl->WscMode == WSC_SMPBC_MODE)
		MTWF_DBG(pAd,  DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscPBCBssTableSort : Total %d SMPBC Registrar Found\n", pWscControl->WscPBCBssCount);
	else
#endif /* IWSC_SUPPORT */
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscPBCBssTableSort : Total %d PBC Registrar Found\n", pWscControl->WscPBCBssCount);
}

#ifdef WSC_STA_SUPPORT
/*
*	========================================================================
*
*	Routine Description:
*		Find WSC PIN activated AP list
*
*	Arguments:
*		pAd         - NIC Adapter pointer
*		OutTab		- Qualified AP BSS table
*
*	Return Value:
*		None
*
*	IRQL = DISPATCH_LEVEL
*
*	Note:
*		All these constants are defined in wsc.h
*
*	========================================================================
*/
VOID WscPINBssTableSort(
	IN RTMP_ADAPTER *pAd,
	IN WSC_CTRL *pWscControl)
{
	UINT i;
	BSS_ENTRY *pInBss;
	UUID_BSSID_CH_INFO *ApUuidBssid = NULL;
	BOOLEAN rv = FALSE;
	STA_ADMIN_CONFIG *sta_cfg = NULL;
	struct wifi_dev *wdev;
	BSS_TABLE *ScanTab;

	if (pWscControl == NULL)
		return;

	wdev = (struct wifi_dev *)pWscControl->wdev;
	sta_cfg = GetStaCfgByWdev(pAd, wdev);
	ASSERT(sta_cfg);
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	pWscControl->ScanCountToincludeWPSPin1++;
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ApUuidBssid, sizeof(UUID_BSSID_CH_INFO) * 8);

	if (ApUuidBssid == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "Allocate memory fail!!!\n");
		return;
	}

	NdisZeroMemory(&ApUuidBssid[0], sizeof(UUID_BSSID_CH_INFO));
	pWscControl->WscPINBssCount = 0;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: scan result AP Count:%u\n",
		__func__, ScanTab->BssNr);

	for (i = 0; i < ScanTab->BssNr; i++) {
		/* BSS entry for VarIE processing */
		pInBss  = (BSS_ENTRY *) &ScanTab->BssEntry[i];

		/*0. compare UI supplied AP mac addr with scan list entry, if not same continue */
		if (RTMPCompareMemory(pWscControl->WscBssid, pInBss->Bssid, 6))
			continue;

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "#  wsc Bssid "MACSTR"\n",
				 MAC2STR(pWscControl->WscBssid));
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "# pIn Bssid "MACSTR"\n",
				 MAC2STR(pInBss->Bssid));

		/* 1. Check VarIE length */
		if (pInBss->VarIELen == 0)
			continue;

		if (sta_cfg && pInBss->BssType != sta_cfg->BssType)
			continue;

		/* 2. Search for WSC IE - 0xdd xx 00 50 f2 04 */
		rv = WscBssWpsIESearchForPIN(pAd,
									 pWscControl,
									 pInBss,
									 ApUuidBssid,
									 pInBss->VarIELen,
									 pInBss->VarIEs);

		if (rv == FALSE) {
			WscBssWpsIESearchForPIN(pAd,
									pWscControl,
									pInBss,
									ApUuidBssid,
									pInBss->VarIeFromProbeRspLen,
									pInBss->pVarIeFromProbRsp);
		}
	}

	if (pWscControl->WscPINBssCount == 1) {
		RTMPZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));
		RTMPMoveMemory(pWscControl->WscSsid.Ssid, ApUuidBssid[0].Ssid, ApUuidBssid[0].SsidLen);
		pWscControl->WscSsid.SsidLength = ApUuidBssid[0].SsidLen;
		RTMPZeroMemory(pWscControl->WscBssid, MAC_ADDR_LEN);
		RTMPMoveMemory(pWscControl->WscBssid, ApUuidBssid[0].Bssid, MAC_ADDR_LEN);
#ifdef CONFIG_STA_SUPPORT
		RTMPZeroMemory(pWscControl->WscPeerMAC, MAC_ADDR_LEN);
		RTMPMoveMemory(pWscControl->WscPeerMAC, ApUuidBssid[0].MacAddr, MAC_ADDR_LEN);
		if (sta_cfg)
			sta_cfg->MlmeAux.Channel = ApUuidBssid[0].Channel;
#endif /* CONFIG_STA_SUPPORT */
	}

	if (ApUuidBssid != NULL)
		os_free_mem(ApUuidBssid);

#ifdef IWSC_SUPPORT

	if (pWscControl->WscMode == WSC_SMPBC_MODE)
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscPINBssTableSort : Total %d SMPBC Registrar Found\n", pWscControl->WscPBCBssCount);
	else
#endif /* IWSC_SUPPORT */
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscPINBssTableSort : Total %d PIN Registrar Found\n", pWscControl->WscPINBssCount);
}
#endif

VOID WscGenRandomKey(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl,
	INOUT PUCHAR pKey,
	INOUT PUSHORT pKeyLen)
{
	UCHAR tempRandomByte = 0;
	UCHAR idx = 0;
	UCHAR keylen = 0;
	UCHAR retry = 0;
	INT ret;

	NdisZeroMemory(pKey, 64);

	/*
	*	Hex Key 64 digital
	*/
	if (pWscControl->WscKeyASCII == 0) {
		UCHAR	tmpStrB[3];

		for (idx = 0; idx < 32; idx++) {
			NdisZeroMemory(&tmpStrB[0], sizeof(tmpStrB));
			tempRandomByte = RandomByte(pAd);
			ret = snprintf((RTMP_STRING *) &tmpStrB[0], sizeof(tmpStrB), "%02x", tempRandomByte);
			if (os_snprintf_error(sizeof(tmpStrB), ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "tmpStrB snprintf error!\n");
			NdisMoveMemory(pKey + (idx * 2), &tmpStrB[0], 2);
		}

		*pKeyLen = 64;
	} else {
		/*
		*	ASCII Key, random length
		*/
		if (pWscControl->WscKeyASCII == 1) {
			do {
				keylen = RandomByte(pAd);
				keylen = keylen % 64;

				if (retry++ > 20)
					keylen = 8;
			} while (keylen < 8);
		} else
			keylen = pWscControl->WscKeyASCII;

		/*
		*	Generate printable ASCII (decimal 33 to 126)
		*/
		for (idx = 0; idx < keylen; idx++) {
			tempRandomByte = RandomByte(pAd) % 94 + 33;
			*(pKey + idx) = tempRandomByte;
		}

		*pKeyLen = keylen;
	}
}

VOID WscCreateProfileFromCfg(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR OpMode,
	IN  PWSC_CTRL pWscControl,
	OUT PWSC_PROFILE pWscProfile)
{
#ifdef CONFIG_AP_SUPPORT
	UCHAR apidx = (pWscControl->EntryIfIdx & 0x1F);
#endif /* CONFIG_AP_SUPPORT */
	USHORT authType = 0, encyType = 0;
	UCHAR WepKeyId = 0;
	PWSC_CREDENTIAL pCredential = NULL;
	UCHAR CurOpMode = AP_MODE;
	INT ret, tmp_buf_left;
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (pWscControl->EntryIfIdx == BSS0)
			CurOpMode = STA_MODE;
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if ((OpMode & 0x0F) == AP_MODE) {
			/*
			*	AP needs to choose the STA's authType and encyType in two cases.
			*	1. AP is unconfigurated (authType and encyType will be updated to mixed mode by WscWriteConfToPortCfg() )
			*	2. AP's authType is mixed mode, we should choose the suitable authType and encyType to STA
			*	STA's authType and encyType depend on WscSecurityMode flag
			*/
			if (((pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED) ||
				 (IS_AKM_FT_WPAPSKWPA2PSK_Entry(&pAd->ApCfg.MBSSID[apidx].wdev))) &&
				(OpMode & REGISTRAR_ACTION)) {
				switch (pAd->ApCfg.MBSSID[apidx].wdev.WscSecurityMode) {
				case WPAPSKTKIP:
					authType = WSC_AUTHTYPE_WPAPSK;
					encyType = WSC_ENCRTYPE_TKIP;
					break;

				case WPAPSKAES:
					authType = WSC_AUTHTYPE_WPAPSK;
					encyType = WSC_ENCRTYPE_AES;
					break;

				case WPA2PSKTKIP:
					authType = WSC_AUTHTYPE_WPA2PSK;
					encyType = WSC_ENCRTYPE_TKIP;
					break;

				case WPA2PSKAES:
					authType = WSC_AUTHTYPE_WPA2PSK;
					encyType = WSC_ENCRTYPE_AES;
					break;

				default:
					authType = (WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK);
					encyType = (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES);
					break;
				}

				if (pWscControl->WscConfStatus == WSC_SCSTATE_CONFIGURED) {
					/*
					*	Although AuthMode is mixed mode, cipher maybe not mixed mode.
					*	We need to correct cipher here.
					*/
					if (IS_CIPHER_TKIP_Entry(&pAd->ApCfg.MBSSID[apidx].wdev))
						encyType = WSC_ENCRTYPE_TKIP;

					if (IS_CIPHER_AES_Entry(&pAd->ApCfg.MBSSID[apidx].wdev))
						encyType |= WSC_ENCRTYPE_AES;	/*mixmode case, bitmap*/
				}
			} else {
				authType = WscGetAuthType(pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.AKMMap);
				encyType = WscGetEncryType(pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseCipher);
			}

			WepKeyId = pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.GroupKeyId;
		}

#ifdef APCLI_SUPPORT
		else if (OpMode == AP_CLIENT_MODE) {
			apidx = apidx & 0x0F;

			if (apidx < MAX_APCLI_NUM) {
				authType = WscGetAuthType(pAd->StaCfg[apidx].wdev.SecConfig.AKMMap);
				encyType = WscGetEncryType(pAd->StaCfg[apidx].wdev.SecConfig.PairwiseCipher);
				WepKeyId = pAd->StaCfg[apidx].wdev.SecConfig.GroupKeyId;
			}
		}

#endif /* APCLI_SUPPORT */
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		authType = WscGetAuthType(pAd->StaCfg[0].wdev.SecConfig.AKMMap);
		encyType = WscGetEncryType(pAd->StaCfg[0].wdev.SecConfig.PairwiseCipher);
		WepKeyId = pAd->StaCfg[0].wdev.SecConfig.GroupKeyId;
	}

#endif /* CONFIG_STA_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscGetDefaultProfileForM8\n");
	pCredential = &pWscProfile->Profile[0]; /*Only support one credential now. 20070515 */
	NdisZeroMemory(pCredential, sizeof(WSC_CREDENTIAL));
	pWscProfile->ProfileCnt = 1;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s:: pWscControl->WscConfStatus  = %d, OpMode = %d\n",
			 __func__, pWscControl->WscConfStatus, OpMode);

	/* NewKey, NewKeyIndex for M8 */
	if ((pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED) &&
		((((OpMode & 0x0F) == AP_MODE)
#ifdef CONFIG_STA_SUPPORT
		  || (((OpMode & 0x0F) == STA_MODE) && (pAd->StaCfg[0].BssType == BSS_ADHOC))
#endif /* CONFIG_STA_SUPPORT */
		 ) && (OpMode & REGISTRAR_ACTION))) {
		pCredential->KeyIndex = 1;

		if ((OpMode & 0x0F) == STA_MODE) {
#ifdef IWSC_TEST_SUPPORT

			if (pAd->StaCfg[0].IWscInfo.IWscDefaultSecurity == 1) {
				authType = WSC_AUTHTYPE_OPEN;
				encyType = WSC_ENCRTYPE_NONE;
				pCredential->KeyLength = 0;
				NdisZeroMemory(pCredential->Key, 64);
			} else if (pAd->StaCfg[0].IWscInfo.IWscDefaultSecurity == 2) {
				UCHAR idx;
				CHAR tempRandomByte;

				authType = WSC_AUTHTYPE_OPEN;
				encyType = WSC_ENCRTYPE_WEP;

				for (idx = 0; idx < 13; idx++) {
					tempRandomByte = RandomByte(pAd) % 94 + 33;
					sprintf((RTMP_STRING *) pCredential->Key + idx, "%c", tempRandomByte);
				}

				pCredential->KeyLength = 13;
			} else
#endif /* IWSC_TEST_SUPPORT */
			{
				WscGenRandomKey(pAd, pWscControl, pCredential->Key, &pCredential->KeyLength);
				authType = WSC_AUTHTYPE_WPA2PSK;
				encyType = WSC_ENCRTYPE_AES;
			}
		} else
			WscGenRandomKey(pAd, pWscControl, pCredential->Key, &pCredential->KeyLength);
	} else {
		struct _SECURITY_CONFIG *pSecConfig = NULL;

		pCredential->KeyIndex = 1;
		pCredential->KeyLength = 0;
		NdisZeroMemory(pCredential->Key, 64);

		switch (encyType) {
		case WSC_ENCRTYPE_NONE:
			break;

		case WSC_ENCRTYPE_WEP:
#ifdef CONFIG_AP_SUPPORT
			if (CurOpMode == AP_MODE)
				pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;

#ifdef APCLI_SUPPORT

			if (OpMode == AP_CLIENT_MODE) {
				if (apidx < MAX_APCLI_NUM)
					pSecConfig = &pAd->StaCfg[apidx].wdev.SecConfig;
			}

#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

			if (CurOpMode == STA_MODE)
				pSecConfig = &pAd->StaCfg[0].wdev.SecConfig;

#endif /* CONFIG_STA_SUPPORT */
			pCredential->KeyIndex = (WepKeyId + 1);

			if (pSecConfig && pSecConfig->WepKey[WepKeyId].KeyLen) {
				INT i;

				for (i = 0; i < pSecConfig->WepKey[WepKeyId].KeyLen; i++) {
					tmp_buf_left = 64 - strlen((RTMP_STRING *)pCredential->Key);
					ret = snprintf((RTMP_STRING *) pCredential->Key + strlen((RTMP_STRING *)pCredential->Key),
						tmp_buf_left, "%02x", pSecConfig->WepKey[WepKeyId].Key[i]);
					if (os_snprintf_error(tmp_buf_left, ret))
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pCredential->Key snprintf error!\n");
				}

				pCredential->KeyLength = pSecConfig->WepKey[WepKeyId].KeyLen * 2;
			}

			break;

		case WSC_ENCRTYPE_TKIP:
		case WSC_ENCRTYPE_AES:
		case (WSC_ENCRTYPE_AES | WSC_ENCRTYPE_TKIP):
			pCredential->KeyLength = pWscControl->WpaPskLen;
			memcpy(pCredential->Key,
				   pWscControl->WpaPsk,
				   pWscControl->WpaPskLen);
			break;
		}
	}

	pCredential->AuthType = authType;
	pCredential->EncrType = encyType;
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if ((OpMode & 0x0F) == AP_MODE) {
			NdisMoveMemory(pCredential->MacAddr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, 6);

			if ((pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED) &&
				(pWscControl->WscDefaultSsid.SsidLength > 0) &&
				(pWscControl->WscDefaultSsid.SsidLength < 33)) {
				NdisMoveMemory(pCredential->SSID.Ssid, pWscControl->WscDefaultSsid.Ssid, pWscControl->WscDefaultSsid.SsidLength);
				pCredential->SSID.SsidLength = pWscControl->WscDefaultSsid.SsidLength;
			} else {
				NdisMoveMemory(pCredential->SSID.Ssid, pAd->ApCfg.MBSSID[apidx].Ssid, pAd->ApCfg.MBSSID[apidx].SsidLen);
				pCredential->SSID.SsidLength = pAd->ApCfg.MBSSID[apidx].SsidLen;
			}
		}

#ifdef APCLI_SUPPORT
		else if (OpMode == AP_CLIENT_MODE) {
			if (apidx < MAX_APCLI_NUM) {
				NdisMoveMemory(pCredential->MacAddr, APCLI_ROOT_BSSID_GET(pAd, pAd->StaCfg[apidx].MacTabWCID), 6);
				NdisMoveMemory(pCredential->SSID.Ssid, pAd->StaCfg[apidx].Ssid, pAd->StaCfg[apidx].SsidLen);
				pCredential->SSID.SsidLength = pAd->StaCfg[apidx].SsidLen;
			}
		}

#endif /* APCLI_SUPPORT */
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		if (pAd->StaCfg[0].BssType == BSS_INFRA)
			NdisMoveMemory(pCredential->MacAddr, pAd->StaCfg[0].Bssid, 6);
		else
			NdisMoveMemory(pCredential->MacAddr, pAd->StaCfg[0].wdev.if_addr, 6);

		NdisMoveMemory(pCredential->SSID.Ssid, pAd->StaCfg[0].Ssid, pAd->StaCfg[0].SsidLen);
		pCredential->SSID.SsidLength = pAd->StaCfg[0].SsidLen;
	}

#endif /* CONFIG_STA_SUPPORT */
#ifdef WSC_V2_SUPPORT

	if (pWscControl->WscV2Info.bEnableWpsV2 && (OpMode & REGISTRAR_ACTION))
		NdisMoveMemory(pCredential->MacAddr, pWscControl->EntryAddr, 6);

#endif /* WSC_V2_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscCreateProfileFromCfg\n");
}

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
void    WscWriteConfToApCliCfg(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl,
	IN  PWSC_CREDENTIAL pCredential,
	IN  BOOLEAN bEnrollee)
{
	UCHAR CurApIdx = (pWscControl->EntryIfIdx & 0x0F);
	STA_ADMIN_CONFIG *pApCliTab;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT old_if_type = pObj->ioctl_if_type;
	struct _SECURITY_CONFIG *pOldSecConfig;

	pOldSecConfig = pObj->pSecConfig;

	if (CurApIdx >= MAX_APCLI_NUM)
		return;

	pObj->ioctl_if_type = INT_APCLI;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscWriteConfToApCliCfg (apcli%d)\n", CurApIdx);
	{
		pApCliTab = &pAd->StaCfg[CurApIdx];
		NdisZeroMemory(pApCliTab->Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pApCliTab->Ssid, pCredential->SSID.Ssid, pCredential->SSID.SsidLength);
		pApCliTab->SsidLen = pCredential->SSID.SsidLength;
		NdisZeroMemory(pApCliTab->CfgSsid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pApCliTab->CfgSsid, pCredential->SSID.Ssid, pCredential->SSID.SsidLength);
		pApCliTab->CfgSsidLen = pCredential->SSID.SsidLength;
		NdisZeroMemory(pApCliTab->CfgApCliBssid, MAC_ADDR_LEN);
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "AuthType: %d, EncrType: %d\n", pCredential->AuthType, pCredential->EncrType);

		if ((pCredential->AuthType == WSC_AUTHTYPE_WPAPSK) ||
			(pCredential->AuthType == WSC_AUTHTYPE_WPA2PSK)) {
			if ((pCredential->EncrType != WSC_ENCRTYPE_TKIP) && (pCredential->EncrType != WSC_ENCRTYPE_AES)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "AuthType is WPAPSK or WPA2PAK.\n"
						 "Get illegal EncrType(%d) from External Registrar, set EncrType to TKIP\n",
						 pCredential->EncrType);
				pCredential->EncrType = WSC_ENCRTYPE_TKIP;
			}
		}

		pObj->pSecConfig = &pApCliTab->wdev.SecConfig;
		Set_SecAuthMode_Proc(pAd, WscGetAuthTypeStr(pCredential->AuthType));
		Set_SecEncrypType_Proc(pAd, WscGetEncryTypeStr(pCredential->EncrType));

		if (pCredential->EncrType != WSC_ENCRTYPE_NONE) {
			if (pCredential->EncrType & (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES)) {
				pApCliTab->wdev.SecConfig.PairwiseKeyId = 0;

				if (pCredential->KeyLength >= 8 && pCredential->KeyLength <= 64) {
					pWscControl->WpaPskLen = (INT) pCredential->KeyLength;
					NdisZeroMemory(pWscControl->WpaPsk, 64);
					NdisMoveMemory(pWscControl->WpaPsk, pCredential->Key, pWscControl->WpaPskLen);
					os_zero_mem(pApCliTab->wdev.SecConfig.PSK, LEN_PSK + 1);
					os_move_mem(pApCliTab->wdev.SecConfig.PSK, pCredential->Key, pCredential->KeyLength);
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WpaPskLen = %d\n", pWscControl->WpaPskLen);
				} else {
					pWscControl->WpaPskLen = 0;
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
						"WPAPSK: Invalid Key Length (%d)\n", pCredential->KeyLength);
				}
			} else if (pCredential->EncrType == WSC_ENCRTYPE_WEP) {
				CHAR   WepKeyId = 0;
				USHORT  WepKeyLen = pCredential->KeyLength;

				WepKeyId = (pCredential->KeyIndex - 1); /* KeyIndex = 1 ~ 4 */

				if ((WepKeyId >= 0) && (WepKeyId <= 3)) {
					struct _SECURITY_CONFIG *pSecConfig = NULL;

					pSecConfig = &pApCliTab->wdev.SecConfig;
					pSecConfig->PairwiseKeyId = WepKeyId;

					/* 5 or 13 ASCII characters */
					/* 10 or 26 Hex characters */
					if (WepKeyLen == 5 || WepKeyLen == 13 || WepKeyLen == 10 || WepKeyLen == 26)
						WscSetWepCipher(pSecConfig, pCredential, WepKeyId, WepKeyLen);
					else
						MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
							"WEP: Invalid Key Length (%d)\n", pCredential->KeyLength);
				} else {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
						"Unsupport default key index (%d), use key Index 1.\n", WepKeyId);
					pApCliTab->wdev.SecConfig.PairwiseKeyId = 0;
				}
			}
		}
	}
#ifdef CONFIG_MAP_SUPPORT
	/* When MAP Turnkey is Enabled, WAPP uses profile and not driver */
	/* So don't use profile timer as it is doing intf down/up causing wps failure */
	if (!IS_MAP_TURNKEY_ENABLE(pAd))
#endif /* CONFIG_MAP_SUPPORT */
	if (pWscControl->WscProfile.ProfileCnt > 1) {
		pWscControl->WscProfileRetryTimerRunning = TRUE;
		RTMPSetTimer(&pWscControl->WscProfileRetryTimer, WSC_PROFILE_RETRY_TIME_OUT);
	}

#ifdef P2P_SUPPORT

	if (P2P_CLI_ON(pAd)) {
		NdisZeroMemory(pAd->P2pCfg.SSID, MAX_LEN_OF_SSID);
		pAd->P2pCfg.SSIDLen = pCredential->SSID.SsidLength;
		NdisMoveMemory(pAd->P2pCfg.SSID, pCredential->SSID.Ssid, pAd->P2pCfg.SSIDLen);
	}

#endif /* P2P_SUPPORT */
	pObj->ioctl_if_type = old_if_type;
	pObj->pSecConfig = pOldSecConfig;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscWriteConfToApCliCfg\n");
}

VOID WscApCliLinkDownById(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR apidx)
{
	BOOLEAN apcliEn;
	PWSC_CTRL pWscControl;
#ifdef CONFIG_MAP_SUPPORT
	BOOLEAN	entryBackhaulSta = FALSE;
#endif /* CONFIG_MAP_SUPPORT */

	if (apidx >= MAX_APCLI_NUM)
		return;

	apcliEn = pAd->StaCfg[apidx].ApcliInfStat.Enable;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscApCliLinkDownById, apidx=%u, apcliEn=%d\n", apidx, apcliEn);
	pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
	NdisMoveMemory(pWscControl->RegData.SelfInfo.MacAddr,
				   pAd->StaCfg[apidx].wdev.if_addr,
				   6);

#ifdef CONFIG_MAP_SUPPORT
	entryBackhaulSta = pWscControl->entryBackhaulSta;
#endif /* CONFIG_MAP_SUPPORT */
	/* bring apcli interface down first */
	if (apcliEn == TRUE) {
		if (apidx < MAX_APCLI_NUM) {
			pAd->StaCfg[apidx].ApcliInfStat.Enable = FALSE;
			ApCliIfDown(pAd);
		}
	}
#ifdef CONFIG_MAP_SUPPORT
	/* To avoid race condition ApcliEnable from iwpriv during wappctr */
	if (IS_MAP_TURNKEY_ENABLE(pAd) && (entryBackhaulSta == TRUE)) {
		pWscControl->entryBackhaulSta = FALSE;
		return;
	}
#endif /* CONFIG_MAP_SUPPORT */

	pAd->StaCfg[apidx].ApcliInfStat.Enable = apcliEn;
	pWscControl->WscStatus = STATUS_WSC_LINK_UP;
	pWscControl->bWscTrigger = TRUE;
}

VOID WscApCliLinkDown(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl)
{
	UCHAR apidx = (pWscControl->EntryIfIdx & 0x0F);

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscApCliLinkDown, apidx=%u\n", apidx);

	if (apidx >= MAX_APCLI_NUM)
		return;

	RTEnqueueInternalCmd(pAd, CMDTHREAD_WSC_APCLI_LINK_DOWN, (VOID *)&apidx, sizeof(UCHAR));
}

#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

VOID WpsSmProcess(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	INT HeaderLen = LENGTH_802_11;
	PHEADER_802_11 pHeader;
	PMAC_TABLE_ENTRY pEntry = NULL;
	INT apidx = MAIN_MBSSID;
	PWSC_CTRL pWpsCtrl = NULL;
	struct wifi_dev *wdev;
	struct wifi_dev_ops *ops;

	wdev = Elem->wdev;
	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "wdev is NULL.\n");
		return;
	}
	ops = wdev->wdev_ops;
	pHeader = (PHEADER_802_11)Elem->Msg;

#ifdef A4_CONN
	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1 && Elem->MsgType != WSC_EAPOL_UPNP_MSG)
		HeaderLen = LENGTH_802_11_WITH_ADDR4;
#endif

	HeaderLen += LENGTH_802_1_H + sizeof(IEEE8021X_FRAME) + sizeof(EAP_FRAME);

	if (Elem->MsgType == WSC_EAPOL_PACKET_MSG) {
		ops->mac_entry_lookup(pAd, pHeader->Addr2, wdev, &pEntry);

		if (pEntry)
			apidx = pEntry->func_tb_idx;
		else {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
					 "cannot find this entry("MACSTR")\n",
					 MAC2STR(pHeader->Addr2));
			return;
		}
	}

	pWpsCtrl = &wdev->WscControl;

	if ((Elem->MsgType == WSC_EAPOL_UPNP_MSG) && (Elem->MsgLen > HeaderLen)) {
		/*The WSC msg from UPnP daemon */
		PUCHAR		pData;
		UCHAR		MacAddr[MAC_ADDR_LEN] = {0};
		/* Skip the (802.11 + 802.1h + 802.1x + EAP) header */
		pData = (PUCHAR) &Elem->Msg[HeaderLen];
		Elem->MsgLen -= HeaderLen;
		/* The Addr1 of UPnP-Msg used to indicate the MAC address of the AP interface. Now always be ra0. */
		NdisMoveMemory(MacAddr, pHeader->Addr1, MAC_ADDR_LEN);
		NdisMoveMemory(Elem->Msg, MacAddr, MAC_ADDR_LEN);
		NdisMoveMemory(Elem->Msg + 6, pData, Elem->MsgLen);
		StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
	} else if (Elem->MsgType == WSC_EAPOL_START_MSG)
		StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
	else if (pEntry && (Elem->MsgType == WSC_EAPOL_PACKET_MSG)) {
		/* WSC_STATE_MACHINE can service only one station at one time */
		RTMP_STRING *pData;
		PEAP_FRAME  pEapFrame;
		/* Skip the EAP LLC header */
		pData = (RTMP_STRING *) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
#ifdef A4_CONN
		if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
			pData = (RTMP_STRING *) &Elem->Msg[LENGTH_802_11_WITH_ADDR4 + LENGTH_802_1_H];
#endif
		pEapFrame = (PEAP_FRAME)(pData + sizeof(IEEE8021X_FRAME));
		pData += sizeof(IEEE8021X_FRAME) + sizeof(EAP_FRAME);
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "EAPOL Packet.  Code = %d.    Type = %d\n",
				 pEapFrame->Code, pEapFrame->Type);

		if (pEapFrame->Code == EAP_CODE_FAIL) {
			/* EAP-Fail */
			RTMP_STRING fail_data[] = "EAP_FAIL";

			NdisMoveMemory(Elem->Msg, pHeader->Addr2, MAC_ADDR_LEN);
			NdisMoveMemory(Elem->Msg + MAC_ADDR_LEN, fail_data, strlen(fail_data));
			Elem->MsgLen = strlen(fail_data);
			StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
			return;
		} else if ((pEapFrame->Code == EAP_CODE_REQ) && (pEapFrame->Type == EAP_TYPE_ID)) {
			/* EAP-Req (Identity) */
			RTMP_STRING id_data[] = "hello";

			pWpsCtrl->lastId = pEapFrame->Id;
			NdisMoveMemory(Elem->Msg, pHeader->Addr2, MAC_ADDR_LEN);
			NdisMoveMemory(Elem->Msg + MAC_ADDR_LEN, id_data, strlen(id_data));
			Elem->MsgLen = strlen(id_data);
			StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
			return;
		} else if ((pEapFrame->Code == EAP_CODE_REQ) && (pEapFrame->Type == EAP_TYPE_WSC)) {
			/* EAP-Req (Messages) */
			if (Elem->MsgLen <= HeaderLen) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "Elem->MsgLen(%ld) <= HeaderLen(%d) !!\n", Elem->MsgLen, HeaderLen);
				return;
			}

			pWpsCtrl->lastId = pEapFrame->Id;
			Elem->MsgLen -= HeaderLen;

			if (WscCheckWSCHeader((PUCHAR)pData)) {
				PWSC_FRAME			pWsc = (PWSC_FRAME) pData;

				NdisMoveMemory(Elem->Msg, pHeader->Addr2, MAC_ADDR_LEN);

				if (pWsc->OpCode == WSC_OPCODE_FRAG_ACK) {
					/*
					*	Send rest WSC frag data
					*/
					RTMP_STRING wsc_frag_ack[] = "WSC_FRAG_ACK";

					NdisMoveMemory(Elem->Msg + MAC_ADDR_LEN, wsc_frag_ack, strlen(wsc_frag_ack));
					Elem->MsgLen = strlen(wsc_frag_ack);
					StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
				} else if (pWsc->OpCode == WSC_OPCODE_START) {
					RTMP_STRING wsc_start[] = "WSC_START";

					NdisMoveMemory(Elem->Msg + MAC_ADDR_LEN, wsc_start, strlen(wsc_start));
					Elem->MsgLen = strlen(wsc_start);
					StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
				} else {
					if (pWsc->Flags & WSC_MSG_FLAG_LF) {
						pData += (sizeof(WSC_FRAME) + 2);
						Elem->MsgLen -= (sizeof(WSC_FRAME) + 2);
					} else {
						pData += sizeof(WSC_FRAME);
						Elem->MsgLen -= sizeof(WSC_FRAME);
					}

					if ((pWpsCtrl->WscRxBufLen + Elem->MsgLen) < (MAX_MGMT_PKT_LEN - 6)) {
						NdisMoveMemory((pWpsCtrl->pWscRxBuf + pWpsCtrl->WscRxBufLen), pData, Elem->MsgLen);
						pWpsCtrl->WscRxBufLen += Elem->MsgLen;
					}

#ifdef WSC_V2_SUPPORT

					if (pWsc->Flags & WSC_MSG_FLAG_MF)
						WscSendEapFragAck(pAd, pWpsCtrl, pEntry);
					else
#endif /* WSC_V2_SUPPORT */
					{
						NdisMoveMemory(Elem->Msg + MAC_ADDR_LEN, pWpsCtrl->pWscRxBuf, pWpsCtrl->WscRxBufLen);
						Elem->MsgLen = pWpsCtrl->WscRxBufLen;
						StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
						pWpsCtrl->WscRxBufLen = 0;
						NdisZeroMemory(pWpsCtrl->pWscRxBuf, MAX_MGMT_PKT_LEN);
					}
				}

				return;
			}

			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN, "ERROR: WscCheckWSCHeader() return FALSE!\n");
			return;
		}

		if (Elem->MsgLen <= HeaderLen) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "Elem->MsgLen(%ld) <= HeaderLen(%d) !!\n", Elem->MsgLen, HeaderLen);
			return;
		}

		Elem->MsgLen -= HeaderLen;
		NdisMoveMemory(Elem->Msg, pHeader->Addr2, MAC_ADDR_LEN);

		if (IS_ENTRY_CLIENT(pEntry) &&
			(pEapFrame->Code == EAP_CODE_RSP) &&
			(pEapFrame->Type == EAP_TYPE_ID)) {
			BOOLEAN Cancelled;

			if (strstr(pData, "SimpleConfig")) {
				/* EAP-Rsp (Identity) */
				NdisMoveMemory(Elem->Msg + 6, pData, Elem->MsgLen);
				StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
				return;
			}

			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "RTMPCancelTimer EapolTimer!!\n");
			NdisZeroMemory(pWpsCtrl->EntryAddr, MAC_ADDR_LEN);
			pWpsCtrl->EapolTimerRunning = FALSE;
			RTMPCancelTimer(&pWpsCtrl->EapolTimer, &Cancelled);
			return;
		}

		if (WscCheckWSCHeader((PUCHAR) pData)) {
			/* EAP-Rsp (Messages) */
			PWSC_FRAME			pWsc = (PWSC_FRAME) pData;

			if (pWsc->OpCode == WSC_OPCODE_FRAG_ACK) {
				/*
				*	Send rest frag data
				*/
				RTMP_STRING wsc_frag_ack[] = "WSC_FRAG_ACK";

				NdisMoveMemory(Elem->Msg + MAC_ADDR_LEN, wsc_frag_ack, strlen(wsc_frag_ack));
				Elem->MsgLen = strlen(wsc_frag_ack);
				StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
			} else {
				if (pWsc->Flags & WSC_MSG_FLAG_LF) {
					pData += (sizeof(WSC_FRAME) + 2);
					Elem->MsgLen -= (sizeof(WSC_FRAME) + 2);
				} else {
					pData += sizeof(WSC_FRAME);
					Elem->MsgLen -= sizeof(WSC_FRAME);
				}

				if ((pWpsCtrl->WscRxBufLen + Elem->MsgLen) < (MAX_MGMT_PKT_LEN - 6)) {
					NdisMoveMemory((pWpsCtrl->pWscRxBuf + pWpsCtrl->WscRxBufLen), pData, Elem->MsgLen);
					pWpsCtrl->WscRxBufLen += Elem->MsgLen;
				}

#ifdef WSC_V2_SUPPORT

				if (pWsc->Flags & WSC_MSG_FLAG_MF)
					WscSendEapFragAck(pAd, pWpsCtrl, pEntry);
				else
#endif /* WSC_V2_SUPPORT */
				{
					/* NdisMoveMemory(Elem->Msg+6, pData, Elem->MsgLen); */
					NdisMoveMemory(Elem->Msg + 6, pWpsCtrl->pWscRxBuf, pWpsCtrl->WscRxBufLen);
					Elem->MsgLen = pWpsCtrl->WscRxBufLen;
					StateMachinePerformAction(pAd, &pAd->Mlme.WscMachine, Elem, pAd->Mlme.WscMachine.CurrState);
					pWpsCtrl->WscRxBufLen = 0;
					NdisZeroMemory(pWpsCtrl->pWscRxBuf, MAX_MGMT_PKT_LEN);
				}
			}

			return;
		}

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN, "ERROR: WscCheckWSCHeader() return FALSE!\n");
		return;
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN, ("Unknow Message Type (=%lu)\n", Elem->MsgType));
}

#ifdef CONFIG_AP_SUPPORT

#define WSC_SINGLE_TRIGGER_APPNAME  "unknown"

#ifdef SDK_GOAHEAD_HTTPD
#undef WSC_SINGLE_TRIGGER_APPNAME
#define WSC_SINGLE_TRIGGER_APPNAME  "goahead"
#endif /* SDK_GOAHEAD_HTTPD */

#ifdef SDK_USER_LIGHTY
#undef WSC_SINGLE_TRIGGER_APPNAME
#define WSC_SINGLE_TRIGGER_APPNAME  "nvram_daemon"
#endif /* SDK_USER_LIGHTY */

INT WscGetConfWithoutTrigger(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWscControl,
	IN  BOOLEAN bFromUPnP)
{
	INT WscMode;
	INT IsAPConfigured;
	PWSC_UPNP_NODE_INFO pWscUPnPNodeInfo;
	UCHAR apIdx;
	struct wifi_dev *wdev = NULL;
	/* TODO: Is it possible ApCli call this function?? */
	apIdx = (pWscControl->EntryIfIdx & 0x1F);

	if (VALID_MBSS(pAd, apIdx))
		wdev = &pAd->ApCfg.MBSSID[apIdx].wdev;

#ifdef LINUX
#ifdef WSC_SINGLE_TRIGGER

	if (wdev != NULL) {
		if (pAd->dev_idx == 0)
			RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, SIGXFSZ, NULL, NULL, 0); /* ra0 */
		else
			RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, SIGWINCH, NULL, NULL, 0); /* rai0 */
	}

#endif /* WSC_SINGLE_TRIGGER */
#endif /* LINUX */
	IsAPConfigured = pWscControl->WscConfStatus;
	pWscUPnPNodeInfo = &pWscControl->WscUPnPNodeInfo;

	if (pWscControl->WscConfMode == WSC_DISABLE) {
		pWscControl->bWscTrigger = FALSE;
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscGetConfForUpnp:: WPS is disabled.\n");
		return FALSE;
	}

	if (bFromUPnP)
		WscStop(pAd, FALSE, pWscControl);

	if (pWscControl->WscMode == 1)
		WscMode = DEV_PASS_ID_PIN;
	else
		WscMode = DEV_PASS_ID_PBC;

	WscBuildBeaconIE(pAd, IsAPConfigured, TRUE, WscMode, pWscControl->WscConfigMethods, (pWscControl->EntryIfIdx & 0x1F), NULL, 0, AP_MODE);
	WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, TRUE, WscMode, pWscControl->WscConfigMethods, pWscControl->EntryIfIdx, NULL, 0, AP_MODE);

	if (wdev != NULL)
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);

	/* 2mins time-out timer */
	RTMPSetTimer(&pWscControl->Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
	pWscControl->Wsc2MinsTimerRunning = TRUE;
	pWscControl->WscStatus = STATUS_WSC_LINK_UP;

	if (bFromUPnP)
		WscSendUPnPConfReqMsg(pAd, apIdx, (PUCHAR)pAd->ApCfg.MBSSID[apIdx].Ssid,
							  pAd->ApCfg.MBSSID[apIdx].wdev.bssid, 3, 0, AP_MODE);

	pWscControl->bWscTrigger = TRUE;
	pWscControl->bWscAutoTigeer = TRUE;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s:: trigger WSC state machine\n", __func__);
	return TRUE;
}
#endif /* CONFIG_AP_SUPPORT */

static VOID WscSendNACK(
	IN  PRTMP_ADAPTER pAdapter,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN  PWSC_CTRL pWscControl)
{
	INT DataLen = 0;
	PUCHAR  pWscData = NULL;
	BOOLEAN Cancelled;
	UCHAR CurOpMode = AP_MODE;
#ifdef CONFIG_STA_SUPPORT

	if ((pAdapter->OpMode == OPMODE_STA)
		&& (pWscControl->EntryIfIdx == BSS0))
		CurOpMode = STA_MODE;

#endif /* CONFIG_STA_SUPPORT */
	os_alloc_mem(NULL, (UCHAR **)&pWscData, WSC_MAX_DATA_LEN);

	if (pWscData == NULL) {
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "WscSendNACK:: WscData Allocate failed!\n");
		return;
	}

	NdisZeroMemory(pWscData, WSC_MAX_DATA_LEN);
	DataLen = BuildMessageNACK(pAdapter, pWscControl, pWscData);
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (pEntry &&
			(IS_ENTRY_PEER_AP(pEntry)
#ifdef P2P_SUPPORT
			 || (P2P_CLI_ON(pAdapter))
#endif /* P2P_SUPPORT */
			)
		   )
			WscSendMessage(pAdapter, WSC_OPCODE_NACK, pWscData, DataLen, pWscControl, AP_CLIENT_MODE, EAP_CODE_RSP);
		else
			WscSendMessage(pAdapter, WSC_OPCODE_NACK, pWscData, DataLen, pWscControl, AP_MODE, EAP_CODE_REQ);
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		if (ADHOC_ON(pAdapter) && (pWscControl->WscConfMode & WSC_REGISTRAR))
			WscSendMessage(pAdapter, WSC_OPCODE_NACK, pWscData, DataLen, pWscControl, STA_MODE, EAP_CODE_REQ);
		else
			WscSendMessage(pAdapter, WSC_OPCODE_NACK, pWscData, DataLen, pWscControl, STA_MODE, EAP_CODE_RSP);
	}

#endif /* CONFIG_STA_SUPPORT */
	RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
	pWscControl->EapolTimerRunning = FALSE;
	pWscControl->RegData.ReComputePke = 1;

	if (pWscData)
		os_free_mem(pWscData);
}

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

VOID WscCheckWpsIeFromWpsAP(
	IN  PRTMP_ADAPTER pAd,
	IN  PEID_STRUCT pEid,
	OUT PUSHORT pDPIDFromAP)
{
	PUCHAR pData;
	SHORT Len = 0;
	PWSC_IE pWscIE;
	USHORT DevicePasswordID;

	if (NdisEqualMemory(pEid->Octet, WPS_OUI, 4)
#ifdef IWSC_SUPPORT
		|| NdisEqualMemory(pEid->Octet, IWSC_OUI, 4)
#endif /* IWSC_SUPPORT */
	   ) {
		pData = (PUCHAR) pEid->Octet + 4;
		Len = (SHORT)(pEid->Len - 4);
		if (Len <= 0 || (Len > sizeof(DevicePasswordID) + 5))
			return;

		while (Len > 0) {
			WSC_IE	WscIE;

			NdisMoveMemory(&WscIE, pData, sizeof(WSC_IE));
			/* Check for WSC IEs */
			pWscIE = &WscIE;

			if (Len < (be2cpu16(pWscIE->Length) + 4)) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
					"unexpected WSC IE Length(%u)\n", be2cpu16(pWscIE->Length));
				break;
			}

			/* Check for device password ID, PIN = 0x0000, PBC = 0x0004 */
			if (pDPIDFromAP && be2cpu16(pWscIE->Type) == WSC_ID_DEVICE_PWD_ID) {
				/* Found device password ID */
				NdisMoveMemory(&DevicePasswordID, pData + 4, sizeof(DevicePasswordID));
				DevicePasswordID = be2cpu16(DevicePasswordID);
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_DEBUG, "WscCheckWpsIeFromWpsAP : DevicePasswordID = 0x%04x\n", DevicePasswordID);

				if (DevicePasswordID == DEV_PASS_ID_PIN) {
					/* PIN */
					*pDPIDFromAP = DEV_PASS_ID_PIN;
				} else if (DevicePasswordID == DEV_PASS_ID_PBC) {
					/* PBC */
					*pDPIDFromAP = DEV_PASS_ID_PBC;
				}
			}

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

			/* Set the offset and look for PBC information */
			/* Since Type and Length are both short type, we need to offset 4, not 2 */
			pData += (be2cpu16(pWscIE->Length) + 4);
			Len   -= (be2cpu16(pWscIE->Length) + 4);
		}
	}
}

#ifdef CONFIG_STA_SUPPORT
static VOID WscLinkDown(
	IN  PRTMP_ADAPTER pAd,
	IN  PVOID wdev_obj)
{
	struct wifi_dev *wdev = (struct wifi_dev *)wdev_obj;
	STA_ADMIN_CONFIG *sta_cfg = GetStaCfgByWdev(pAd, wdev);
	WSC_CTRL *wsc_ctrl = &wdev->WscControl;
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	if (INFRA_ON(sta_cfg) && STA_STATUS_TEST_FLAG(sta_cfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscLinkDown(): Disassociate with current WPS AP...\n");
		cntl_disconnect_request(wdev, CNTL_DISASSOC, sta_cfg->Bssid, REASON_DISASSOC_STA_LEAVING);
		sta_cfg->MlmeAux.CurrReqIsFromNdis = TRUE;
	}


	if (wsc_ctrl->WscConfMode != WSC_DISABLE) {
#ifdef WSC_LED_SUPPORT
		UCHAR WPSLEDStatus;
		/* The protocol is connecting to a partner. */
		WPSLEDStatus = LED_WPS_IN_PROCESS;
		RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(wdev));
#endif /* WSC_LED_SUPPORT */
#ifdef IWSC_SUPPORT

		/*
		*	We need to send EAPOL_Start again to trigger WPS process
		*/
		if (sta_cfg->BssType == BSS_ADHOC) {
			wdev->IWscInfo.bSendEapolStart = FALSE;
			wsc_ctrl->WscState = WSC_STATE_LINK_UP;
			wsc_ctrl->WscStatus = STATUS_WSC_LINK_UP;
			WscSendEapolStart(pAd, wsc_ctrl->WscPeerMAC, STA_MODE, wdev);
		} else
#endif /* IWSC_SUPPORT */
			wsc_ctrl->WscState = WSC_STATE_START;
	} else {
		sta_cfg->bConfigChanged = TRUE;
		wsc_ctrl->bWscTrigger = FALSE;

		if (sta_cfg->BssType == BSS_INFRA) {
			BssTableDeleteEntry(ScanTab, sta_cfg->Bssid, wdev->channel);
			sta_cfg->MlmeAux.SsidBssTab.BssNr = 0;

			cntl_connect_request(wdev, CNTL_CONNECT_BY_BSSID, MAC_ADDR_LEN, sta_cfg->MlmeAux.Bssid);
		}

#ifdef IWSC_SUPPORT
		else { /* BSS_ADHOC */
			NDIS_802_11_SSID Ssid;

			if (sta_cfg->IWscInfo.bReStart) {
				sta_cfg->bNotFirstScan = FALSE;
				sta_cfg->bAutoConnectByBssid = FALSE;
				sta_cfg->IWscInfo.bReStart = FALSE;
				sta_cfg->IWscInfo.bDoNotChangeBSSID = TRUE;
				LinkDown(pAd, FALSE, wdev);
				STA_STATUS_CLEAR_FLAG(sta_cfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
				RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
				pAd->ExtraInfo = GENERAL_LINK_DOWN;

				if (wsc_ctrl->WscStatus != STATUS_WSC_CONFIGURED) {
					Ssid.SsidLength = pAd->CommonCfg.SsidLen;
					NdisZeroMemory(Ssid.Ssid, MAX_LEN_OF_SSID);
					NdisMoveMemory(Ssid.Ssid, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen);
				} else {
					Ssid.SsidLength = sta_cfg->MlmeAux.SsidLen;
					NdisZeroMemory(Ssid.Ssid, MAX_LEN_OF_SSID);
					NdisMoveMemory(Ssid.Ssid, sta_cfg->MlmeAux.Ssid, Ssid.SsidLength);
				}

				cntl_connect_request(wdev, CNTL_CONNECT_BY_SSID, sizeof(NDIS_802_11_SSID), (UCHAR *)&Ssid);

			}

			MlmeEnqueue(pAd, IWSC_STATE_MACHINE, IWSC_MT2_MLME_STOP, 0, NULL, 0);
			RTMP_MLME_HANDLER(pAd);
		}

#endif /* IWSC_SUPPORT */
	}

	wsc_ctrl->RegData.ReComputePke = 1;

	/* YF: Reset to default after active pbc mode */
	if (!wsc_ctrl->Wsc2MinsTimerRunning)
		wsc_ctrl->RegData.SelfInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PIN);
}

ULONG WscSearchWpsApBySSID(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pSsid,
	IN UCHAR SsidLen,
	IN INT WscMode,
	IN VOID *wdev_obj)
{
	UINT i;
	USHORT DesiredDPID;
	BSS_ENTRY *pBss;
	PWSC_CTRL pWscControl;
	struct wifi_dev *wdev;
	BSS_TABLE *ScanTab;

	wdev = (struct wifi_dev *)wdev_obj;
	pWscControl = &wdev->WscControl;
	ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	if (WscMode == WSC_PBC_MODE)
		DesiredDPID = DEV_PASS_ID_PBC;
	else
		DesiredDPID = DEV_PASS_ID_PIN;

	for (i = 0; i < ScanTab->BssNr; i++) {
		pBss = &ScanTab->BssEntry[i];

		if (SSID_EQUAL(pSsid, SsidLen, pBss->Ssid, pBss->SsidLen) &&
			pBss->WpsAP &&
			((pBss->WscDPIDFromWpsAP == DesiredDPID) || (DesiredDPID == DEV_PASS_ID_PIN))) {
			if ((pWscControl->WpsApBand == PREFERRED_WPS_AP_PHY_TYPE_5_G_FIRST) &&
				(pBss->Channel <= 14))
				continue;
			else if ((pWscControl->WpsApBand == PREFERRED_WPS_AP_PHY_TYPE_2DOT4_G_FIRST) &&
					 (pBss->Channel > 14))
				continue;
			else
				return (ULONG)i;
		}
	}

	return (ULONG)BSS_NOT_FOUND;
}
#endif /* CONFIG_STA_SUPPORT */

VOID WscPBCSessionOverlapCheck(
	IN  PRTMP_ADAPTER pAd,
	IN	UCHAR current_band)
{
	ULONG now;
	PWSC_STA_PBC_PROBE_INFO pWscStaPbcProbeInfo = &pAd->CommonCfg.WscStaPbcProbeInfo;
#ifdef CONFIG_AP_SUPPORT
	UCHAR bss_index = 0;
#endif

	pAd->CommonCfg.WscPBCOverlap = FALSE;

	if (pWscStaPbcProbeInfo->WscPBCStaProbeCount[current_band] > 1) {
		UCHAR  i;

		for (i = 0; i < MAX_PBC_STA_TABLE_SIZE; i++) {
			NdisGetSystemUpTime(&now);

			if (pWscStaPbcProbeInfo->Valid[current_band][i] &&
				RTMP_TIME_AFTER(now, pWscStaPbcProbeInfo->ReciveTime[current_band][i] + 120 * OS_HZ)) {
				NdisZeroMemory(&(pWscStaPbcProbeInfo->StaMacAddr[current_band][i][0]), MAC_ADDR_LEN);
				pWscStaPbcProbeInfo->ReciveTime[current_band][i] = 0;
				pWscStaPbcProbeInfo->Valid[current_band][i] = FALSE;
				pWscStaPbcProbeInfo->WscPBCStaProbeCount[current_band]--;
			}
		}

		if (pWscStaPbcProbeInfo->WscPBCStaProbeCount[current_band] > 1) {
#ifdef CONFIG_AP_SUPPORT
			/* For the current band , check whether any bss has triggered wps, if yes,
			*	do WscCheckPeerDPID to add WscPBCStaProbeCount which is ued to
			*	check if any wps pbc overlap existed; if not, do nothing to avoid false
			*	alarm of wps pbc overlap
			*/
			for (bss_index = 0; bss_index < pAd->ApCfg.BssidNum; bss_index++) {
				if ((current_band == HcGetBandByWdev(&pAd->ApCfg.MBSSID[bss_index].wdev)) &&
					pAd->ApCfg.MBSSID[bss_index].wdev.WscControl.WscConfMode != WSC_DISABLE &&
					pAd->ApCfg.MBSSID[bss_index].wdev.WscControl.bWscTrigger == TRUE) {
					MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"%s(): found pAd->ApCfg.MBSSID[%d] WPS on\n",
							__func__, bss_index);
					break;
				}
			}
#ifdef DBDC_MODE
			/*bss in current band has triggered wps pbc, so check Peer DPID*/
			if (bss_index < pAd->ApCfg.BssidNum) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"pAd->ApCfg.MBSSID[%d] WPS on, PBC Overlap detected\n",
					bss_index);
				pAd->CommonCfg.WscPBCOverlap = TRUE;
			} else {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"pAd->ApCfg.MBSSID[%d] WPS off, PBC Overlap is invalid\n",
					bss_index);
				pAd->CommonCfg.WscPBCOverlap = FALSE;
			}
#else
			pAd->CommonCfg.WscPBCOverlap = TRUE;
#endif
#endif /* CONFIG_AP_SUPPORT */
		}
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "WscPBCSessionOverlapCheck : WscPBCStaProbeCount[%d] = %d\n",
			current_band, pWscStaPbcProbeInfo->WscPBCStaProbeCount[current_band]);
}

VOID WscPBC_DPID_FromSTA(
	IN  PRTMP_ADAPTER pAd,
	IN  PUCHAR pMacAddr,
	IN	UCHAR current_band)
{
	INT Index = 0;
	UCHAR tab_idx;
	BOOLEAN bAddEntry = FALSE;
	ULONG now;
	PWSC_STA_PBC_PROBE_INFO	pWscStaPbcProbeInfo = &pAd->CommonCfg.WscStaPbcProbeInfo;

	NdisGetSystemUpTime(&now);
	if (pWscStaPbcProbeInfo->WscPBCStaProbeCount[current_band] == 0)
		bAddEntry = TRUE;
	else {
		for (tab_idx = 0; tab_idx < MAX_PBC_STA_TABLE_SIZE; tab_idx++) {
			if (NdisEqualMemory(pMacAddr, pWscStaPbcProbeInfo->StaMacAddr[current_band][tab_idx], MAC_ADDR_LEN)) {
				pWscStaPbcProbeInfo->ReciveTime[current_band][tab_idx] = now;
				return;
			}
		}
		for (tab_idx = 0; tab_idx < MAX_PBC_STA_TABLE_SIZE; tab_idx++) {
			if (RTMP_TIME_AFTER(now, pWscStaPbcProbeInfo->ReciveTime[current_band][tab_idx] + 120 * OS_HZ) ||
				NdisEqualMemory(pWscStaPbcProbeInfo->StaMacAddr[current_band][tab_idx], &ZERO_MAC_ADDR[0], MAC_ADDR_LEN)) {
				if (pWscStaPbcProbeInfo->Valid[current_band][tab_idx] == FALSE) {
					Index = tab_idx;
					bAddEntry = TRUE;
					break;
				}
				pWscStaPbcProbeInfo->ReciveTime[current_band][tab_idx] = now;
				NdisMoveMemory(pWscStaPbcProbeInfo->StaMacAddr[current_band][tab_idx], pMacAddr, MAC_ADDR_LEN);
				return;
			}
		}
	}
	if (bAddEntry) {
		pWscStaPbcProbeInfo->WscPBCStaProbeCount[current_band]++;
		pWscStaPbcProbeInfo->ReciveTime[current_band][Index] = now;
		pWscStaPbcProbeInfo->Valid[current_band][Index] = TRUE;
		NdisMoveMemory(pWscStaPbcProbeInfo->StaMacAddr[current_band][Index], pMacAddr, MAC_ADDR_LEN);
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s(): STA_MAC = "MACSTR"\n",
			 __func__, MAC2STR(pMacAddr));
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s(): WscPBCStaProbeCount[%d] = %d\n",
			 __func__, current_band, pWscStaPbcProbeInfo->WscPBCStaProbeCount[current_band]);
}

#ifdef CONFIG_STA_SUPPORT

#ifndef FIRST_STA_PROFILE_PATH
#define FIRST_STA_PROFILE_PATH      "/etc/Wireless/RT2860/RT2860.dat"
#endif

#ifndef SECOND_STA_PROFILE_PATH
#define SECOND_STA_PROFILE_PATH "/etc/Wireless/iNIC/iNIC_sta.dat"
#endif

#ifndef THIRD_STA_PROFILE_PATH
#define THIRD_STA_PROFILE_PATH "/etc/Wireless/WIFI3/RT2870AP.dat"
#endif

#ifndef STA_PROFILE_PATH
#define STA_PROFILE_PATH	"/etc/Wireless/RT2860STA/RT2860STA.dat"
#endif

static UCHAR *get_dev_profile(RTMP_ADAPTER *pAd)
{
	UCHAR *src = NULL;
#if defined(CONFIG_RT_FIRST_CARD) || defined(CONFIG_RT_SECOND_CARD) || defined(CONFIG_RT_THIRD_CARD)
	INT card_idx = get_dev_config_idx(pAd);
#endif /* CONFIG_RT_FIRST_CARD || CONFIG_RT_SECOND_CARD */
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
#ifdef CONFIG_RT_FIRST_CARD

		if (card_idx == 0)
			src = FIRST_STA_PROFILE_PATH;
		else
#endif /* CONFIG_RT_FIRST_CARD */
#ifdef CONFIG_RT_SECOND_CARD
			if (card_idx == 1)
				src = SECOND_STA_PROFILE_PATH;
			else
#endif /* CONFIG_RT_SECOND_CARD */
#ifdef CONFIG_RT_THIRD_CARD
				if (card_idx == 2)
					src = THIRD_STA_PROFILE_PATH;
				else
#endif /* CONFIG_RT_THIRD_CARD */
				{
#ifdef PROFILE_PATH_DYNAMIC
					src = pAd->profilePath;
#else
					src = STA_PROFILE_PATH;
#endif /* PROFILE_PATH_DYNAMIC */
				}
	}
	return src;
}

#endif

static VOID WscWriteAuthToDAT(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR CurOpMode,
	IN  RTMP_STRING *pTempStr)
{
	INT ret, tmp_buf_left;
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		INT index;

		for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
			if (pAd->ApCfg.MBSSID[index].SsidLen) {
				tmp_buf_left = 512 - strlen(pTempStr);
				if (index == 0)
					ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%s",
										GetAuthModeStr(pAd->ApCfg.MBSSID[index].wdev.SecConfig.AKMMap));
				else
					ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, ";%s",
										GetAuthModeStr(pAd->ApCfg.MBSSID[index].wdev.SecConfig.AKMMap));

				if (os_snprintf_error(tmp_buf_left, ret))
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
			}
		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		USHORT auth_flag = WscGetAuthType(pAd->StaCfg[0].wdev.SecConfig.AKMMap);
		tmp_buf_left = 512 - strlen(pTempStr);
		ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%s", WscGetAuthTypeStr(auth_flag));
		if (os_snprintf_error(tmp_buf_left, ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
	}

#endif /* CONFIG_STA_SUPPORT */
}

static VOID WscWriteEncrToDAT(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR CurOpMode,
	IN  RTMP_STRING *pTempStr)
{
	INT ret, tmp_buf_left;
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		INT index;

		for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
			tmp_buf_left = 512 - strlen(pTempStr);
			if (index == 0)
				ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%s",
						GetEncryModeStr(pAd->ApCfg.MBSSID[index].wdev.SecConfig.PairwiseCipher));
			else
				ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, ";%s",
						GetEncryModeStr(pAd->ApCfg.MBSSID[index].wdev.SecConfig.PairwiseCipher));
			if (os_snprintf_error(tmp_buf_left, ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		USHORT encrypt_flag = WscGetEncryType(pAd->StaCfg[0].wdev.SecConfig.PairwiseCipher);
		tmp_buf_left = 512 - strlen(pTempStr);
		ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%s", WscGetEncryTypeStr(encrypt_flag));
		if (os_snprintf_error(tmp_buf_left, ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
	}

#endif /* CONFIG_STA_SUPPORT */
}

static VOID WscWriteWscConfModeToDAT(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR CurOpMode,
	IN  RTMP_STRING *pTempStr)
{
	WSC_CTRL *wsc_ctrl;
	INT ret, tmp_buf_left;
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		INT index;

		for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
			wsc_ctrl = &pAd->ApCfg.MBSSID[index].wdev.WscControl;
			tmp_buf_left = 512 - strlen(pTempStr);
			if (index == 0)
				ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%d", wsc_ctrl->WscConfMode);
			else
				ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, ";%d", wsc_ctrl->WscConfMode);
			if (os_snprintf_error(tmp_buf_left, ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");

		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		wsc_ctrl = &pAd->StaCfg[0].wdev.WscControl;
		tmp_buf_left = 512 - strlen(pTempStr);
		ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%d", wsc_ctrl->WscConfMode);
		if (os_snprintf_error(tmp_buf_left, ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
	}

#endif /* CONFIG_STA_SUPPORT */
}

static VOID WscWriteWscConfStatusToDAT(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR CurOpMode,
	IN  RTMP_STRING *pTempStr)
{
	WSC_CTRL *wsc_ctrl;
	INT ret, tmp_buf_left;
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		INT index;

		for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
			wsc_ctrl = &pAd->ApCfg.MBSSID[index].wdev.WscControl;
			tmp_buf_left = 512 - strlen(pTempStr);
			if (index == 0)
				ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%d", wsc_ctrl->WscConfStatus);
			else
				ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, ";%d", wsc_ctrl->WscConfStatus);
			if (os_snprintf_error(tmp_buf_left, ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		wsc_ctrl = &pAd->StaCfg[0].wdev.WscControl;
		tmp_buf_left = 512 - strlen(pTempStr);
		ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%d", wsc_ctrl->WscConfStatus);
		if (os_snprintf_error(tmp_buf_left, ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
	}

#endif /* CONFIG_STA_SUPPORT */
}

static VOID WscWriteDefaultKeyIdToDAT(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR CurOpMode,
	IN  RTMP_STRING *pTempStr)
{
	INT ret, tmp_buf_left;
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		INT index;

		for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
			tmp_buf_left = 512 - strlen(pTempStr);
			if (index == 0)
				ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%d",
							pAd->ApCfg.MBSSID[index].wdev.SecConfig.PairwiseKeyId + 1);
			else
				ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, ";%d",
							pAd->ApCfg.MBSSID[index].wdev.SecConfig.PairwiseKeyId + 1);
			if (os_snprintf_error(tmp_buf_left, ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		tmp_buf_left = 512 - strlen(pTempStr);
		ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%d", pAd->StaCfg[0].wdev.SecConfig.PairwiseKeyId + 1);
		if (os_snprintf_error(tmp_buf_left, ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
	}
#endif /* CONFIG_STA_SUPPORT */
}

#define WSC_PRINT_WEP_KEY(__idx, __key_len, __key, __out_buf) \
	{ 	INT ret, tmp_buf_left; \
		for (__idx = 0; __idx < __key_len; (__idx)++) { \
			tmp_buf_left = 512 - strlen(__out_buf); \
			ret = snprintf(__out_buf + strlen(__out_buf), tmp_buf_left, "%02x", __key[__idx]); \
			if (os_snprintf_error(tmp_buf_left, ret)) \
				MTWF_DBG(NULL, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n"); \
		} \
	}

static BOOLEAN WscWriteWEPKeyToDAT(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR CurOpMode,
	IN  RTMP_STRING *WepKeyFormatName,
	IN  RTMP_STRING *WepKeyName,
	IN  RTMP_STRING *pTempStr)
{
	WSC_CREDENTIAL *pCredentail;
	INT tempStrLen = 0;
	INT ret, tmp_buf_left;
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		UCHAR apidx = (pAd->WriteWscCfgToDatFile & 0x0F);

		if ((strncmp(pTempStr, WepKeyFormatName, strlen(WepKeyFormatName)) == 0)) {

			if (IS_CIPHER_WEP_Entry(&pAd->ApCfg.MBSSID[apidx].wdev)) {
				UCHAR idx = 0, KeyType[4] = {0};
				RTMP_STRING *ptr2, *temp_ptr;

				ptr2 = rtstrstr(pTempStr, "=");

				if (!ptr2)
					return FALSE;

				temp_ptr = pTempStr;
				pTempStr = ptr2 + 1;
				KeyType[0] = (UCHAR)(*pTempStr - 0x30);

				for (idx = 1; idx < 4; idx++) {
					ptr2 = rtstrstr(pTempStr, ";");

					if (ptr2 == NULL)
						break;

					pTempStr = ptr2 + 1;

					if ((*pTempStr == '0') || (*pTempStr == '1'))
						KeyType[idx] = (UCHAR)(*pTempStr - 0x30);
				}

				pTempStr = temp_ptr;
				NdisZeroMemory(pTempStr, 512);
				NdisMoveMemory(pTempStr, WepKeyFormatName, strlen(WepKeyFormatName));

				for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
					tmp_buf_left = 512 - strlen(pTempStr);
					if (idx == apidx)
						ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%d", 0);
					else
						ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%d", KeyType[idx]);
					if (os_snprintf_error(tmp_buf_left, ret))
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");

					if (apidx < (pAd->ApCfg.BssidNum - 1)) {
						tmp_buf_left = 512 - strlen(pTempStr);
						ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%s", ";");
						if (os_snprintf_error(tmp_buf_left, ret))
							MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
					}
				}
			}
		} else if ((strncmp(pTempStr, WepKeyName, strlen(WepKeyName)) == 0)) {
			pCredentail = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscProfile.Profile[0];

			if (IS_CIPHER_WEP_Entry(&pAd->ApCfg.MBSSID[apidx].wdev)) {
				NdisZeroMemory(pTempStr, 512);
				NdisMoveMemory(pTempStr, WepKeyName, strlen(WepKeyName));
				tempStrLen = strlen(pTempStr);

				if (pCredentail->KeyLength) {
					if ((pCredentail->KeyLength == 5) ||
						(pCredentail->KeyLength == 13)) {
						int jjj = 0;

						WSC_PRINT_WEP_KEY(jjj, pCredentail->KeyLength, pCredentail->Key, pTempStr);
					} else if ((pCredentail->KeyLength == 10) ||
							   (pCredentail->KeyLength == 26))
						NdisMoveMemory(pTempStr + tempStrLen, pCredentail->Key, pCredentail->KeyLength);
				}
			}
		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
		if (rtstrstr(pTempStr, (RTMP_STRING *) WepKeyFormatName)) {
			if (IS_CIPHER_WEP_Entry(&pAd->StaCfg[0].wdev)) {
				NdisZeroMemory(pTempStr, 512);
				ret = snprintf(pTempStr, 512, "%s0", WepKeyFormatName); /* Hex */
				if (os_snprintf_error(512, ret))
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
			}
		} else if (rtstrstr(pTempStr, (RTMP_STRING *) WepKeyName)) {
			UCHAR profile_idx = pAd->StaCfg[0].wdev.WscControl.WscProfile.ApplyProfileIdx;

			pCredentail = &pAd->StaCfg[0].wdev.WscControl.WscProfile.Profile[profile_idx];

			if (IS_CIPHER_WEP_Entry(&pAd->StaCfg[0].wdev)) {
				NdisZeroMemory(pTempStr, 512);
				NdisMoveMemory(pTempStr, WepKeyName, strlen(WepKeyName));
				tempStrLen = strlen(pTempStr);

				if (pCredentail->KeyLength) {
					if ((pCredentail->KeyLength == 5) ||
						(pCredentail->KeyLength == 13)) {
						int jjj = 0;

						WSC_PRINT_WEP_KEY(jjj, pCredentail->KeyLength, pCredentail->Key, pTempStr);
					} else if ((pCredentail->KeyLength == 10) ||
							   (pCredentail->KeyLength == 26))
						NdisMoveMemory(pTempStr + tempStrLen, pCredentail->Key, pCredentail->KeyLength);
				}
			}
		}
	}

#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}

VOID WscWriteConfToDatFile(RTMP_ADAPTER *pAd, UCHAR CurOpMode)
{
	char *cfgData = 0;
	RTMP_STRING *fileName = NULL;
	RTMP_OS_FD file_r, file_w;
	RTMP_OS_FS_INFO osFSInfo;
	LONG rv, fileLen = 0;
	char *offset = 0;
	RTMP_STRING *pTempStr = NULL;
	UINT StrLen = 1024;
	INT ret;
#ifdef CONFIG_AP_SUPPORT
	UCHAR apidx = (pAd->WriteWscCfgToDatFile & 0x0F);
#endif /* CONFIG_AP_SUPPORT */
	RTMP_STRING WepKeyName[MAX_WEPKEYNAME_LEN] = {0};
	RTMP_STRING WepKeyFormatName[MAX_WEPKEYNAME_LEN] = {0};

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
			 "-----> WscWriteConfToDatFile(CurOpMode = %d)\n", CurOpMode);
#ifdef CONFIG_AP_SUPPORT

	if (CurOpMode == AP_MODE) {
		if (apidx > pAd->ApCfg.BssidNum) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_WARN,
				"<----- WscWriteConfToDatFile (wrong apidx = %d)\n", apidx);
			return;
		}

		fileName = get_dev_l2profile(pAd);

		ret = snprintf((RTMP_STRING *) WepKeyName, sizeof(WepKeyName), "Key%dStr%d=",
						pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseKeyId + 1, apidx + 1);
		if (os_snprintf_error(sizeof(WepKeyName), ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "WepKeyName snprintf error!\n");
		ret = snprintf((RTMP_STRING *) WepKeyFormatName, sizeof(WepKeyFormatName), "Key%dType=",
						pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseKeyId + 1);
		if (os_snprintf_error(sizeof(WepKeyFormatName), ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "WepKeyFormatName snprintf error!\n");
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (CurOpMode == STA_MODE) {
#ifdef RTMP_RBUS_SUPPORT

		if (pAd->infType == RTMP_DEV_INF_RBUS)
			fileName = STA_PROFILE_PATH_RBUS;
		else
#endif /* RTMP_RBUS_SUPPORT */
			fileName = get_dev_profile(pAd); /* STA_PROFILE_PATH; */

		ret = snprintf(WepKeyName, sizeof(WepKeyName), "Key%dStr=", pAd->StaCfg[0].wdev.SecConfig.PairwiseKeyId + 1);
		if (os_snprintf_error(sizeof(WepKeyName), ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "WepKeyName snprintf error!\n");
		ret = snprintf(WepKeyFormatName, sizeof(WepKeyFormatName), "Key%dType=", pAd->StaCfg[0].wdev.SecConfig.PairwiseKeyId + 1);
		if (os_snprintf_error(sizeof(WepKeyFormatName), ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "WepKeyFormatName snprintf error!\n");
	}

#endif /* CONFIG_STA_SUPPORT */
	if (fileName == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "File Name is null !\n");
		return;
	}

	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	file_r = RtmpOSFileOpen(fileName, O_RDONLY, 0);

	if (IS_FILE_OPEN_ERR(file_r)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
			"-->1) %s: Error opening file %s\n", __func__, fileName);
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
		return;
	}

	{
		char tempStr[64] = {0};

		while ((rv = RtmpOSFileRead(file_r, tempStr, 64)) > 0)
			fileLen += rv;

		os_alloc_mem(NULL, (UCHAR **)&cfgData, fileLen);

		if (cfgData == NULL) {
			RtmpOSFileClose(file_r);
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
					"CfgData mem alloc fail. (fileLen = %ld)\n", fileLen);
			goto out;
		}

		NdisZeroMemory(cfgData, fileLen);
		RtmpOSFileSeek(file_r, 0);
		rv = RtmpOSFileRead(file_r, (RTMP_STRING *)cfgData, fileLen);
		RtmpOSFileClose(file_r);

		if (rv != fileLen) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
					"CfgData mem alloc fail, fileLen = %ld\n", fileLen);
			goto ReadErr;
		}
	}

	file_w = RtmpOSFileOpen(fileName, O_WRONLY | O_TRUNC, 0);

	if (IS_FILE_OPEN_ERR(file_w))
		goto WriteFileOpenErr;
	else {
		offset = (PCHAR) rtstrstr((RTMP_STRING *) cfgData, "Default\n");
		offset += strlen("Default\n");
		RtmpOSFileWrite(file_w, (RTMP_STRING *)cfgData, (int)(offset - cfgData));
		os_alloc_mem(NULL, (UCHAR **)&pTempStr, StrLen);

		if (!pTempStr) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
					"pTempStr mem alloc fail. (%d)\n", StrLen);
			RtmpOSFileClose(file_w);
			goto WriteErr;
		}

		for (;;) {
			int i = 0;
			RTMP_STRING *ptr;
			BOOLEAN	bNewFormat = TRUE;

			NdisZeroMemory(pTempStr, StrLen);

			if ((size_t)(offset - cfgData) < fileLen) {
				ptr = (RTMP_STRING *) offset;

				while (*ptr && *ptr != '\n')
					pTempStr[i++] = *ptr++;

				pTempStr[i] = 0x00;
				offset += strlen(pTempStr) + 1;

				if ((strncmp(pTempStr, "SSID=", strlen("SSID=")) == 0) ||
					strncmp(pTempStr, "SSID1=", strlen("SSID1=")) == 0 ||
					strncmp(pTempStr, "SSID2=", strlen("SSID2=")) == 0 ||
					strncmp(pTempStr, "SSID3=", strlen("SSID3=")) == 0 ||
					strncmp(pTempStr, "SSID4=", strlen("SSID4=")) == 0
				   ) {
					if (rtstrstr(pTempStr, "SSID="))
						bNewFormat = FALSE;

					WscWriteSsidToDatFile(pAd, pTempStr, bNewFormat, CurOpMode);
				}

#ifdef CONFIG_STA_SUPPORT
				else if (strncmp(pTempStr, "NetworkType=", strlen("NetworkType=")) == 0) {
					NdisZeroMemory(pTempStr, StrLen);

					if (pAd->StaCfg[0].BssType == BSS_ADHOC)
						ret = snprintf(pTempStr, 512, "NetworkType=Adhoc");
					else
						ret = snprintf(pTempStr, 512, "NetworkType=Infra");
					if (os_snprintf_error(512, ret))
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
				}

#endif /* CONFIG_STA_SUPPORT */
				else if (strncmp(pTempStr, "AuthMode=", strlen("AuthMode=")) == 0) {
					NdisZeroMemory(pTempStr, StrLen);
					ret = snprintf(pTempStr, StrLen, "AuthMode=");
					if (os_snprintf_error(StrLen, ret))
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
					WscWriteAuthToDAT(pAd, CurOpMode, pTempStr);
				} else if (strncmp(pTempStr, "EncrypType=", strlen("EncrypType=")) == 0) {
					NdisZeroMemory(pTempStr, StrLen);
					ret = snprintf(pTempStr, StrLen, "EncrypType=");
					if (os_snprintf_error(StrLen, ret))
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
					WscWriteEncrToDAT(pAd, CurOpMode, pTempStr);
				} else if ((strncmp(pTempStr, "WPAPSK=", strlen("WPAPSK=")) == 0) ||
						   (strncmp(pTempStr, "WPAPSK1=", strlen("WPAPSK1=")) == 0) ||
						   (strncmp(pTempStr, "WPAPSK2=", strlen("WPAPSK2=")) == 0) ||
						   (strncmp(pTempStr, "WPAPSK3=", strlen("WPAPSK3=")) == 0) ||
						   (strncmp(pTempStr, "WPAPSK4=", strlen("WPAPSK4=")) == 0)) {
					bNewFormat = TRUE;

					if (strstr(pTempStr, "WPAPSK="))
						bNewFormat = FALSE;

					WscWriteWpaPskToDatFile(pAd, pTempStr, bNewFormat);
				} else if (strncmp(pTempStr, "WscConfMode=", strlen("WscConfMode=")) == 0) {
					ret = snprintf(pTempStr, StrLen, "WscConfMode=");
					if (os_snprintf_error(StrLen, ret))
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
					WscWriteWscConfModeToDAT(pAd, CurOpMode, pTempStr);
				} else if (strncmp(pTempStr, "WscConfStatus=", strlen("WscConfStatus=")) == 0) {
					ret = snprintf(pTempStr, StrLen, "WscConfStatus=");
					if (os_snprintf_error(StrLen, ret))
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
					WscWriteWscConfStatusToDAT(pAd, CurOpMode, pTempStr);
				} else if (strncmp(pTempStr, "DefaultKeyID=", strlen("DefaultKeyID=")) == 0) {
					NdisZeroMemory(pTempStr, StrLen);
					ret = snprintf(pTempStr, StrLen, "DefaultKeyID=");
					if (os_snprintf_error(StrLen, ret))
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
					WscWriteDefaultKeyIdToDAT(pAd, CurOpMode, pTempStr);
				} else {
					if (WscWriteWEPKeyToDAT(pAd, CurOpMode, WepKeyFormatName, WepKeyName, pTempStr) == FALSE)
						goto WriteErr;
				}
				RtmpOSFileWrite(file_w, pTempStr, strlen(pTempStr));
				RtmpOSFileWrite(file_w, "\n", 1);
			} else
				break;
		}

		RtmpOSFileClose(file_w);
	}

WriteErr:

	if (pTempStr)
		os_free_mem(pTempStr);

ReadErr:
WriteFileOpenErr:

	if (cfgData)
		os_free_mem(cfgData);

out:
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscWriteConfToDatFile\n");
	return;
	}

#ifdef CONFIG_AP_SUPPORT
static VOID WscWriteWepKeyToAR9File(
	IN  RTMP_ADAPTER *pAd,
	IN  RTMP_STRING *pTempStr,
	IN  RTMP_STRING *pDatStr,
	OUT INT *datoffset)
{
	INT index;
	INT apidx;
	INT offset;
	INT tempStrLen, ret, tmp_buf_left;
	WSC_CREDENTIAL *pCredentail = NULL;
	RTMP_STRING WepKeyName[MAX_WEPKEYNAME_LEN] = {0};
	RTMP_STRING WepKeyFormatName[MAX_WEPKEYTYPE_LEN] = {0};

	for (index = 1; index <= 4; index++) {
		ret = snprintf(WepKeyFormatName, sizeof(WepKeyFormatName), "Key%dType=", index);
		if (os_snprintf_error(sizeof(WepKeyFormatName), ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "WepKeyFormatName snprintf error!\n");
		{
			NdisZeroMemory(pTempStr, 512);
			offset = 0;
			NdisMoveMemory(pTempStr, WepKeyFormatName, strlen(WepKeyFormatName));

			for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
				if (IS_CIPHER_WEP(pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.SecConfig.PairwiseCipher)) {
					pCredentail = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscProfile.Profile[0];
					tmp_buf_left = 512 - strlen(pTempStr);
					if ((pCredentail->KeyLength == 5) ||
						(pCredentail->KeyLength == 13))
						ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%d", 1); /* ASCII */
					else
						ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%d", 0); /* Hex */
					if (os_snprintf_error(tmp_buf_left, ret))
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
				}

				if (apidx < (pAd->ApCfg.BssidNum - 1)) {
					tmp_buf_left = 512 - strlen(pTempStr);
					ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%s", ";");
					if (os_snprintf_error(tmp_buf_left, ret))
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
				}
			}

			tmp_buf_left = 512 - strlen(pTempStr);
			ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%s", "\n");
			if (os_snprintf_error(tmp_buf_left, ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
			offset = strlen(pTempStr);
			NdisMoveMemory(pDatStr + (*datoffset), pTempStr, offset);
			(*datoffset) += offset;
		}
		ret = snprintf(WepKeyName, sizeof(WepKeyName), "Key%dStr=", index);
		if (os_snprintf_error(sizeof(WepKeyName), ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "WepKeyName snprintf error!\n");
		/*if (rtstrstr(pTempStr, WepKeyName)) */
		{
			NdisZeroMemory(pTempStr, 512);
			offset = 0;
			NdisMoveMemory(pTempStr, WepKeyName, strlen(WepKeyName));
			tempStrLen = strlen(pTempStr);

			for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
				pCredentail = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscProfile.Profile[0];

				if (pCredentail->KeyLength) {
					NdisMoveMemory(pTempStr + tempStrLen, pCredentail->Key, pCredentail->KeyLength);
					tempStrLen = strlen(pTempStr);
				}

				if (apidx < (pAd->ApCfg.BssidNum - 1))
					NdisMoveMemory(pTempStr + tempStrLen, ";", 1);

				tempStrLen += 1;
			}

			tmp_buf_left = 512 - strlen(pTempStr);
			ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%s", "\n");
			if (os_snprintf_error(tmp_buf_left, ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
			offset = strlen(pTempStr);
			NdisMoveMemory(pDatStr + (*datoffset), pTempStr, offset);
			(*datoffset) += offset;
		}

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			ret = snprintf(WepKeyName, sizeof(WepKeyName), "Key%dStr%d=", index, (apidx + 1));
			if (os_snprintf_error(sizeof(WepKeyName), ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "WepKeyName snprintf error!\n");

			if (IS_CIPHER_WEP(pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseCipher)) {
				NdisZeroMemory(pTempStr, 512);
				NdisMoveMemory(pTempStr, WepKeyName, strlen(WepKeyName));
				tempStrLen = strlen(pTempStr);
				pCredentail = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscProfile.Profile[0];
				NdisMoveMemory(pTempStr + tempStrLen, pCredentail->Key, pCredentail->KeyLength);
				NdisMoveMemory(pTempStr + tempStrLen + pCredentail->KeyLength, "\n", 1);
				tempStrLen = tempStrLen + pCredentail->KeyLength + 1;
			}

			offset = tempStrLen;
			NdisMoveMemory(pDatStr + (*datoffset), pTempStr, offset);
			(*datoffset) += offset;
		}
	}
}

void WscWriteConfToAR9File(
	IN  PRTMP_ADAPTER pAd,
	IN  UCHAR CurOpMode)
{
	RTMP_STRING *fileName = NULL;
	RTMP_OS_FD file_w;
	RTMP_OS_FS_INFO osFSInfo;
	INT offset = 0;
	INT datoffset = 0, ret, tmp_buf_left;
	RTMP_STRING *pTempStr = 0;
	RTMP_STRING *pDatStr = 0;
#ifdef CONFIG_AP_SUPPORT
	INT index = 0;
	UCHAR apidx = MAIN_MBSSID;
#endif /* CONFIG_AP_SUPPORT */
	PWSC_CTRL pWscControl = NULL;
	RTMP_STRING item_str[10] = {0};

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-----> WscWriteConfToAR9File\n");

#ifdef CONFIG_AP_SUPPORT
	if (CurOpMode == AP_MODE)
		fileName = get_dev_l2profile(pAd);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	if (CurOpMode == STA_MODE) {
#ifdef RTMP_RBUS_SUPPORT
		if (pAd->infType == RTMP_DEV_INF_RBUS)
			fileName = STA_PROFILE_PATH_RBUS;
		else
#endif /* RTMP_RBUS_SUPPORT */
			fileName = get_dev_profile(pAd);/* STA_PROFILE_PATH; */
	}
#endif /* CONFIG_STA_SUPPORT */

	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	if (fileName)
		file_w = RtmpOSFileOpen(fileName, O_WRONLY | O_CREAT, 0);
	else
		file_w = NULL;

	if (IS_FILE_OPEN_ERR(file_w))
		goto WriteFileOpenErr;
	else {
		os_alloc_mem(NULL, (UCHAR **)&pTempStr, 512);

		if (!pTempStr) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "pTempStr mem alloc fail. (512)\n");
			RtmpOSFileClose(file_w);
			goto WriteErr;
		}

		os_alloc_mem(NULL, (UCHAR **)&pDatStr, 4096);

		if (!pDatStr) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "pDatStr mem alloc fail. (4096)\n");
			RtmpOSFileClose(file_w);
			goto WriteErr;
		}

		NdisZeroMemory(pTempStr, 512);
		NdisZeroMemory(pDatStr, 4096);
		NdisZeroMemory(item_str, 10);

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			ret = snprintf(item_str, sizeof(item_str), "SSID%d", (apidx + 1));
			if (os_snprintf_error(sizeof(item_str), ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "item_str snprintf error!\n");
			{
				NdisMoveMemory(pTempStr, item_str, strlen(item_str));
				offset = strlen(pTempStr);
				NdisMoveMemory(pTempStr + offset, "=", 1);
				offset += 1;
				NdisMoveMemory(pTempStr + offset, pAd->ApCfg.MBSSID[apidx].Ssid, pAd->ApCfg.MBSSID[apidx].SsidLen);
				offset += pAd->ApCfg.MBSSID[apidx].SsidLen;
				NdisMoveMemory(pTempStr + offset, "\n", 1);
				offset += 1;
			}
			NdisZeroMemory(item_str, 10);
		}

		NdisMoveMemory(pDatStr, pTempStr, offset);
		datoffset += offset;
		offset = 0;
		NdisZeroMemory(pTempStr, 512);
		ret = snprintf(pTempStr, 512, "AuthMode=");
		if (os_snprintf_error(512, ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
		WscWriteAuthToDAT(pAd, CurOpMode, pTempStr);
		offset = strlen(pTempStr);
		NdisMoveMemory(pDatStr + datoffset, pTempStr, offset);
		datoffset += offset;
		offset = 0;
		NdisZeroMemory(pTempStr, 512);
		ret = snprintf(pTempStr, 512, "EncrypType=");
		if (os_snprintf_error(512, ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
		WscWriteEncrToDAT(pAd, CurOpMode, pTempStr);
		offset = strlen(pTempStr);
		NdisMoveMemory(pDatStr + datoffset, pTempStr, offset);
		datoffset += offset;
		offset = 0;
		NdisZeroMemory(pTempStr, 512);
		NdisZeroMemory(item_str, 10);

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			ret = snprintf(item_str, sizeof(item_str), "WPAPSK%d", (apidx + 1));
			if (os_snprintf_error(sizeof(item_str), ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "item_str snprintf error!\n");
			/*if (rtstrstr(pTempStr, item_str)) */
			{
				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
				NdisMoveMemory(pTempStr, item_str, strlen(item_str));
				offset = strlen(pTempStr);
				NdisMoveMemory(pTempStr + offset, "=", 1);
				offset += 1;
				NdisMoveMemory(pTempStr + offset, pWscControl->WpaPsk, pWscControl->WpaPskLen);
				offset += pWscControl->WpaPskLen;
				NdisMoveMemory(pTempStr + offset, "\n", 1);
				offset += 1;
			}
			NdisZeroMemory(item_str, 10);
		}

		NdisMoveMemory(pDatStr + datoffset, pTempStr, offset);
		datoffset += offset;
		offset = 0;
		NdisZeroMemory(pTempStr, 512);
		ret = snprintf(pTempStr, 512, "WscConfMode=");
		if (os_snprintf_error(512, ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
#ifdef CONFIG_AP_SUPPORT

		for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
			pWscControl = &pAd->ApCfg.MBSSID[index].wdev.WscControl;
			tmp_buf_left = 512 - strlen(pTempStr);
			if (index == 0)
				ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%d", pWscControl->WscConfMode);
			else
				ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, ";%d", pWscControl->WscConfMode);
			if (os_snprintf_error(tmp_buf_left, ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
		}

		tmp_buf_left = 512 - strlen(pTempStr);
		ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%s", "\n");
		if (os_snprintf_error(tmp_buf_left, ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
		offset = strlen(pTempStr);
		NdisMoveMemory(pDatStr + datoffset, pTempStr, offset);
		datoffset += offset;
#endif /* CONFIG_AP_SUPPORT */
		offset = 0;
		NdisZeroMemory(pTempStr, 512);
		ret = snprintf(pTempStr, 512, "WscConfStatus=");
		if (os_snprintf_error(512, ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
#ifdef CONFIG_AP_SUPPORT

		for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
			pWscControl = &pAd->ApCfg.MBSSID[index].wdev.WscControl;
			tmp_buf_left = 512 - strlen(pTempStr);
			if (index == 0)
				ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%d", pWscControl->WscConfStatus);
			else
				ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, ";%d", pWscControl->WscConfStatus);
			if (os_snprintf_error(tmp_buf_left, ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
		}

		tmp_buf_left = 512 - strlen(pTempStr);
		ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%s", "\n");
		if (os_snprintf_error(tmp_buf_left, ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
		offset = strlen(pTempStr);
		NdisMoveMemory(pDatStr + datoffset, pTempStr, offset);
		datoffset += offset;
#endif /* CONFIG_AP_SUPPORT */
		offset = 0;
		NdisZeroMemory(pTempStr, 512);
		ret = snprintf(pTempStr, 512, "DefaultKeyID=");
		if (os_snprintf_error(512, ret))
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == AP_MODE) {
			for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
				tmp_buf_left = 512 - strlen(pTempStr);
				if (index == 0)
					ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%d", pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseKeyId + 1);
				else
					ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, ";%d", pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseKeyId + 1);
				if (os_snprintf_error(tmp_buf_left, ret))
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
			}

			tmp_buf_left = 512 - strlen(pTempStr);
			ret = snprintf(pTempStr + strlen(pTempStr), tmp_buf_left, "%s", "\n");
			if (os_snprintf_error(tmp_buf_left, ret))
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pTempStr snprintf error!\n");
			offset = strlen(pTempStr);
			NdisMoveMemory(pDatStr + datoffset, pTempStr, offset);
			datoffset += offset;
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == AP_MODE)
			WscWriteWepKeyToAR9File(pAd, pTempStr, pDatStr, &datoffset);

#endif /* CONFIG_AP_SUPPORT */
		RtmpOSFileWrite(file_w, pDatStr, datoffset);
		RtmpOSFileClose(file_w);
	}

WriteErr:

	if (pTempStr)
		os_free_mem(pTempStr);

	if (pDatStr)
		os_free_mem(pDatStr);

WriteFileOpenErr:
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<----- WscWriteConfToAR9File\n");
}
#endif/*CONFIG_AP_SUPPORT*/

static INT wsc_write_dat_file_thread(
	IN ULONG Context)
{
	RTMP_OS_TASK *pTask;
	RTMP_ADAPTER *pAd;
	int	Status = 0;

	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

	if (pAd == NULL)
		return 0;

	RtmpOSTaskCustomize(pTask);

	while (pTask && !RTMP_OS_TASK_IS_KILLED(pTask)) {
		RtmpusecDelay(2000);

		if (RtmpOSTaskWait(pAd, pTask, &Status) == FALSE) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		if (Status != 0)
			break;


		if (pAd->pWscElme && (pAd->pWscElme->MsgLen != 0)) {
			MLME_QUEUE_ELEM	*pElme;

			os_alloc_mem(pAd, (UCHAR **)&pElme, sizeof(MLME_QUEUE_ELEM));

			if (pElme) {
				NdisZeroMemory(pElme, sizeof(MLME_QUEUE_ELEM));
				RTMP_SEM_LOCK(&pAd->WscElmeLock);
				NdisMoveMemory(pElme, pAd->pWscElme, sizeof(MLME_QUEUE_ELEM));
				pAd->pWscElme->MsgLen = 0;
				NdisZeroMemory(pAd->pWscElme->Msg, MAX_MGMT_PKT_LEN);
				RTMP_SEM_UNLOCK(&pAd->WscElmeLock);
				WpsSmProcess(pAd, pElme);
				os_free_mem(pElme);
			}
		}

		if (pAd->WriteWscCfgToDatFile != 0xFF) {
			UCHAR	CurOpMode = AP_MODE;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			CurOpMode = AP_MODE;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			CurOpMode = STA_MODE;

			if (pAd->WriteWscCfgToDatFile != BSS0)
				CurOpMode = AP_MODE;

#endif /* CONFIG_STA_SUPPORT */
			WscWriteConfToDatFile(pAd, CurOpMode);
			pAd->WriteWscCfgToDatFile = 0xFF;
		}
	}

	if (pTask)
		RtmpOSTaskNotifyToExit(pTask);

	return 0;
}


/*
* This kernel thread init in the probe fucntion, so we should kill it when do remove module.
*/
BOOLEAN WscThreadExit(RTMP_ADAPTER *pAd)
{
	INT ret;
	BOOLEAN Cancelled;
	/*
	*	This kernel thread init in the probe fucntion, so kill it when do remove module.
	*/
	ret = RtmpOSTaskKill(&pAd->wscTask);

	if (ret == NDIS_STATUS_FAILURE)
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "kill wsc task failed!\n");

	if (pAd->pHmacData) {
		os_free_mem(pAd->pHmacData);
		pAd->pHmacData = NULL;
	}

	if (pAd->pWscElme) {
		os_free_mem(pAd->pWscElme);
		pAd->pWscElme = NULL;
	}

	NdisFreeSpinLock(&pAd->WscElmeLock);
#ifdef CONFIG_AP_SUPPORT

	if ((pAd->OpMode == OPMODE_AP)
#ifdef P2P_SUPPORT
		/* P2P will use ApCfg.MBSSID and StaCfg also. */
		|| TRUE
#endif /* P2P_SUPPORT */
	   ) {
		INT ap_idx;
		UCHAR MaxBssidNum = MAX_MBSSID_NUM(pAd);

		for (ap_idx = 0; ap_idx < MaxBssidNum; ap_idx++) {
			PWSC_CTRL	pWpsCtrl = &pAd->ApCfg.MBSSID[ap_idx].wdev.WscControl;

			WscStop(pAd, FALSE, pWpsCtrl);
			RTMPReleaseTimer(&pWpsCtrl->EapolTimer, &Cancelled);
			RTMPReleaseTimer(&pWpsCtrl->Wsc2MinsTimer, &Cancelled);
			RTMPReleaseTimer(&pWpsCtrl->WscUPnPNodeInfo.UPnPMsgTimer, &Cancelled);
			RTMPReleaseTimer(&pWpsCtrl->M2DTimer, &Cancelled);
#ifdef WSC_LED_SUPPORT
			RTMPReleaseTimer(&pWpsCtrl->WscLEDTimer, &Cancelled);
			RTMPReleaseTimer(&pWpsCtrl->WscSkipTurnOffLEDTimer, &Cancelled);
#endif
			RTMPReleaseTimer(&pWpsCtrl->WscUpdatePortCfgTimer, &Cancelled);
#ifdef WSC_V2_SUPPORT
			RTMPReleaseTimer(&pWpsCtrl->WscSetupLockTimer, &Cancelled);
#endif
			pWpsCtrl->WscRxBufLen = 0;

			if (pWpsCtrl->pWscRxBuf) {
				os_free_mem(pWpsCtrl->pWscRxBuf);
				pWpsCtrl->pWscRxBuf = NULL;
			}

			pWpsCtrl->WscTxBufLen = 0;

			if (pWpsCtrl->pWscTxBuf) {
				os_free_mem(pWpsCtrl->pWscTxBuf);
				pWpsCtrl->pWscTxBuf = NULL;
			}

#ifdef WSC_V2_SUPPORT

			if (pWpsCtrl->WscSetupLockTimerRunning) {
				BOOLEAN Cancelled;

				pWpsCtrl->WscSetupLockTimerRunning = FALSE;
				RTMPCancelTimer(&pWpsCtrl->WscSetupLockTimer, &Cancelled);
			}

			if (pWpsCtrl->WscV2Info.ExtraTlv.pTlvData) {
				os_free_mem(pWpsCtrl->WscV2Info.ExtraTlv.pTlvData);
				pWpsCtrl->WscV2Info.ExtraTlv.pTlvData = NULL;
			}

#endif /* WSC_V2_SUPPORT */
			WscClearPeerList(&pWpsCtrl->WscPeerList);
			NdisFreeSpinLock(&pWpsCtrl->WscPeerListSemLock);
		}

#ifdef APCLI_SUPPORT
		{
			INT index;

			for (index = 0; index < MAX_APCLI_NUM; index++) {
				PWSC_CTRL       pWpsCtrl = &pAd->StaCfg[index].wdev.WscControl;

				WscStop(pAd, TRUE, pWpsCtrl);
				RTMPReleaseTimer(&pWpsCtrl->EapolTimer, &Cancelled);
				RTMPReleaseTimer(&pWpsCtrl->Wsc2MinsTimer, &Cancelled);
				RTMPReleaseTimer(&pWpsCtrl->WscUPnPNodeInfo.UPnPMsgTimer, &Cancelled);
				RTMPReleaseTimer(&pWpsCtrl->M2DTimer, &Cancelled);
#ifdef WSC_LED_SUPPORT
				RTMPReleaseTimer(&pWpsCtrl->WscLEDTimer, &Cancelled);
				RTMPReleaseTimer(&pWpsCtrl->WscSkipTurnOffLEDTimer, &Cancelled);
#endif
				RTMPReleaseTimer(&pWpsCtrl->WscPBCTimer, &Cancelled);
				RTMPReleaseTimer(&pWpsCtrl->WscScanTimer, &Cancelled);
				RTMPReleaseTimer(&pWpsCtrl->WscProfileRetryTimer, &Cancelled);
#ifdef CON_WPS
				RTMPReleaseTimer(&pWpsCtrl->ConWscApcliScanDoneCheckTimer, &Cancelled);
#endif /* CON_WPS */
				pWpsCtrl->WscTxBufLen = 0;

				if (pWpsCtrl->pWscTxBuf) {
					os_free_mem(pWpsCtrl->pWscTxBuf);
					pWpsCtrl->pWscTxBuf = NULL;
				}

				pWpsCtrl->WscRxBufLen = 0;

				if (pWpsCtrl->pWscRxBuf) {
					os_free_mem(pWpsCtrl->pWscRxBuf);
					pWpsCtrl->pWscRxBuf = NULL;
				}

				if (pWpsCtrl->pWscRxBuf)
					os_free_mem(pWpsCtrl->pWscRxBuf);

				WscClearPeerList(&pWpsCtrl->WscPeerList);
				NdisFreeSpinLock(&pWpsCtrl->WscPeerListSemLock);
			}
		}
#endif /* APCLI_SUPPORT */
#ifdef CON_WPS
		RTMPReleaseTimer(&pAd->ApCfg.ConWpsApCliBandMonitorTimer, &Cancelled);
#endif /* CON_WPS */
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		UCHAR IdSta;

		for (IdSta = 0; IdSta < pAd->MaxMSTANum; IdSta++) {
			PWSC_CTRL       pWpsCtrl = &pAd->StaCfg[IdSta].wdev.WscControl;

			WscStop(pAd,
#ifdef CONFIG_AP_SUPPORT
					FALSE,
#endif /* CONFIG_AP_SUPPORT */
					pWpsCtrl);
			RTMPReleaseTimer(&pWpsCtrl->EapolTimer, &Cancelled);
			RTMPReleaseTimer(&pWpsCtrl->Wsc2MinsTimer, &Cancelled);
			RTMPReleaseTimer(&pWpsCtrl->WscUPnPNodeInfo.UPnPMsgTimer, &Cancelled);
			RTMPReleaseTimer(&pWpsCtrl->M2DTimer, &Cancelled);
#ifdef WSC_LED_SUPPORT
			RTMPReleaseTimer(&pWpsCtrl->WscLEDTimer, &Cancelled);
			RTMPReleaseTimer(&pWpsCtrl->WscSkipTurnOffLEDTimer, &Cancelled);
#endif
			RTMPReleaseTimer(&pWpsCtrl->WscPBCTimer, &Cancelled);
			RTMPReleaseTimer(&pWpsCtrl->WscScanTimer, &Cancelled);
			RTMPReleaseTimer(&pWpsCtrl->WscProfileRetryTimer, &Cancelled);
			RTMPReleaseTimer(&pWpsCtrl->WscUpdatePortCfgTimer, &Cancelled);
#ifdef WSC_V2_SUPPORT
#ifdef CONFIG_AP_SUPPORT
			RTMPReleaseTimer(&pWpsCtrl->WscSetupLockTimer, &Cancelled);
#endif
#endif
			pWpsCtrl->WscRxBufLen = 0;

			if (pWpsCtrl->pWscRxBuf) {
				os_free_mem(pWpsCtrl->pWscRxBuf);
				pWpsCtrl->pWscRxBuf = NULL;
			}

			pWpsCtrl->WscTxBufLen = 0;

			if (pWpsCtrl->pWscTxBuf) {
				os_free_mem(pWpsCtrl->pWscTxBuf);
				pWpsCtrl->pWscTxBuf = NULL;
			}

			WscClearPeerList(&pWpsCtrl->WscPeerList);
			NdisFreeSpinLock(&pWpsCtrl->WscPeerListSemLock);
#ifdef IWSC_SUPPORT
			WscClearPeerList(&pWpsCtrl->WscConfiguredPeerList);
			NdisFreeSpinLock(&pWpsCtrl->WscConfiguredPeerListSemLock);
#endif /* IWSC_SUPPORT */
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	/* WSC hardware push button function 0811 */
	WSC_HDR_BTN_Stop(pAd);
	return TRUE;
}


/*
  * This kernel thread init in the probe function.
  */
NDIS_STATUS WscThreadInit(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS status;
	RTMP_OS_TASK *pTask;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "-->WscThreadInit()\n");
	pTask = &pAd->wscTask;
	RTMP_OS_TASK_INIT(pTask, "RtmpWscTask", pAd);
	status = RtmpOSTaskAttach(pTask, wsc_write_dat_file_thread, (ULONG)&pAd->wscTask);

	if (status == NDIS_STATUS_SUCCESS) {
		os_alloc_mem(NULL, &pAd->pHmacData, sizeof(CHAR) * (2048));

		if (pAd->pHmacData == NULL) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "Wsc HmacData memory alloc failed!\n");
			status = FALSE;
		}

		NdisAllocateSpinLock(pAd, &pAd->WscElmeLock);
		os_alloc_mem(NULL, (UCHAR **)&pAd->pWscElme, sizeof(MLME_QUEUE_ELEM));
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "<--WscThreadInit(), status=%d!\n", status);
	return status;
}


/* WSC hardware push button function 0811 */
/*
*========================================================================
*Routine Description:
*	Initialize the PUSH PUTTION Check Module.
*
*Arguments:
*	ad_p			- WLAN control block pointer
*
*Return Value:
*	None
*
*Note:
*========================================================================
*/
VOID WSC_HDR_BTN_Init(
	IN  PRTMP_ADAPTER pAd)
{
	pAd->CommonCfg.WscHdrPshBtnCheckCount = 0;
} /* End of WSC_HDR_BTN_Init */


/*
*========================================================================
*Routine Description:
*	Stop the PUSH PUTTION Check Module.
*
*Arguments:
*	ad_p			- WLAN control block pointer
*
*Return Value:
*	None
*
*Note:
*========================================================================
*/
VOID WSC_HDR_BTN_Stop(
	IN  PRTMP_ADAPTER pAd)
{
	pAd->CommonCfg.WscHdrPshBtnCheckCount = 0;
} /* End of WSC_HDR_BTN_Stop */


/*
*========================================================================
*Routine Description:
*	Start the PUSH PUTTION Check thread.
*
*Arguments:
*	*Context		- WLAN control block pointer
*
*Return Value:
*	0			- terminate the thread successfully
*
*Note:
*========================================================================
*/
VOID WSC_HDR_BTN_CheckHandler(
	IN  PRTMP_ADAPTER pAd)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	BOOLEAN flg_pressed = 0;

	WSC_HDR_BTN_MR_PRESS_FLG_GET(pAd, flg_pressed);

	if (flg_pressed) {
		/* the button is pressed */
		if (pAd->CommonCfg.WscHdrPshBtnCheckCount == WSC_HDR_BTN_CONT_TIMES) {
			/* we only handle once until the button is released */
			pAd->CommonCfg.WscHdrPshBtnCheckCount = 0;
			/* execute WSC PBC function */
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "wsc> execute WSC PBC...\n");
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				pObj->ioctl_if = 0;
				Set_AP_WscMode_Proc(pAd, (PUCHAR)"2"); /* 2: PBC */
				Set_AP_WscGetConf_Proc(pAd, (PUCHAR)"1"); /* 1: Trigger */
			}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
				pObj->ioctl_if = 0;
#ifdef WSC_STA_SUPPORT
				Set_WscConfMode_Proc(pAd, (PUCHAR)"1"); /* 1:  */

				if (Set_WscMode_Proc(pAd, (PUCHAR)"2") == FALSE) /* 2: PBC */
					return;

				Set_WscGetConf_Proc(pAd, (PUCHAR)"1"); /* 1: Trigger */
#endif /* WSC_STA_SUPPORT */
			}
#endif /* CONFIG_STA_SUPPORT */
			return;
		}

		pAd->CommonCfg.WscHdrPshBtnCheckCount++;
	} else {
		/* the button is released */
		pAd->CommonCfg.WscHdrPshBtnCheckCount = 0;
	}
}

#ifdef WSC_LED_SUPPORT
/* */
/* Support WPS LED mode (mode 7, mode 8 and mode 9). */
/* Ref: User Feedback (page 80, WPS specification 1.0) */
/* */
BOOLEAN WscSupportWPSLEDMode(
	IN PRTMP_ADAPTER pAd)
{
	if ((LED_MODE(pAd) == WPS_LED_MODE_7) ||
		(LED_MODE(pAd) == WPS_LED_MODE_8) ||
		(LED_MODE(pAd) == WPS_LED_MODE_9) ||
		(LED_MODE(pAd) == WPS_LED_MODE_11) ||
		(LED_MODE(pAd) == WPS_LED_MODE_12)
#ifdef CONFIG_WIFI_LED_SUPPORT
		|| (LED_MODE(pAd) == WPS_LED_MODE_SHARE)
#endif /* CONFIG_WIFI_LED_SUPPORT */
	   ) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: Support WPS LED mode (The WPS LED mode = %d).\n",
				 __func__, LED_MODE(pAd));
		return TRUE; /* Support WPS LED mode. */
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: Not support WPS LED mode (The WPS LED mode = %d).\n",
			 __func__, LED_MODE(pAd));
	return FALSE; /* Not support WPS LED mode. */
}

BOOLEAN WscSupportWPSLEDMode10(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR led_mode;

	led_mode = LED_MODE(pAd);

	if (led_mode == WPS_LED_MODE_10) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: Support WPS LED mode (The WPS LED mode = %d).\n",
				 __func__, led_mode);
		return TRUE; /*Support WPS LED mode 10. */
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: Not support WPS LED mode (The WPS LED mode = %d).\n",
			 __func__, led_mode);
	return FALSE; /* Not support WPS LED mode 10. */
}

/* */
/* Whether the WPS AP has security setting or not. */
/* Note that this function is valid only after the WPS handshaking. */
/* */
BOOLEAN WscAPHasSecuritySetting(
	IN PRTMP_ADAPTER pAdapter,
	IN PWSC_CTRL pWscControl)
{
	BOOLEAN bAPHasSecuritySetting = FALSE;
	UCHAR	currentIdx = MAIN_MBSSID;
#ifdef CONFIG_AP_SUPPORT
	currentIdx = (pWscControl->EntryIfIdx & 0x1F);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	currentIdx = pWscControl->WscProfile.ApplyProfileIdx;
#endif /* CONFIG_STA_SUPPORT */

	switch (pWscControl->WscProfile.Profile[currentIdx].EncrType) {
	case WSC_ENCRTYPE_NONE: {
		bAPHasSecuritySetting = FALSE;
		break;
	}

	case WSC_ENCRTYPE_WEP:
	case WSC_ENCRTYPE_TKIP:
	case (WSC_ENCRTYPE_TKIP | WSC_ENCRTYPE_AES):
	case WSC_ENCRTYPE_AES: {
		bAPHasSecuritySetting = TRUE;
		break;
	}

	default: {
		MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: Incorrect encryption types (%d)\n",
				 __func__, pWscControl->WscProfile.Profile[currentIdx].EncrType);
		ASSERT(FALSE);
		break;
	}
	}

	MTWF_DBG(pAdapter, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: WSC Entryption Type = %d\n",
			 __func__, pWscControl->WscProfile.Profile[currentIdx].EncrType);
	return bAPHasSecuritySetting;
}


/* */
/* After the NIC connects with a WPS AP or not, */
/* the WscLEDTimer timer controls the LED behavior according to LED mode. */
/* */
VOID WscLEDTimer(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pWscControl->pAd;
	UCHAR WPSLEDStatus = 0;

	/* WPS LED mode 7, 8, 11 and 12. */
	if ((LED_MODE(pAd) == WPS_LED_MODE_7) ||
		(LED_MODE(pAd) == WPS_LED_MODE_8) ||
		(LED_MODE(pAd) == WPS_LED_MODE_11) ||
		(LED_MODE(pAd) == WPS_LED_MODE_12)) {
		WPSLEDStatus = LED_WPS_TURN_LED_OFF;
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: Turn off the WPS successful LED pattern.\n", __func__);
	} else if ((LED_MODE(pAd) == WPS_LED_MODE_9) /* WPS LED mode 9. */
#ifdef CONFIG_WIFI_LED_SUPPORT
			   || (LED_MODE(pAd) == WPS_LED_MODE_SHARE)
#endif /* CONFIG_WIFI_LED_SUPPORT */
			  ) {
		switch (pWscControl->WscLEDMode) { /* Last WPS LED state. */
		/* Turn off the blue LED after 300 seconds. */
		case LED_WPS_SUCCESS:
			WPSLEDStatus = LED_WPS_TURN_LED_OFF;
			/* Turn on/off the WPS success LED according to AP's encryption algorithm after one second. */
			RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_WPS_TURN_OFF_LED_TIMEOUT);
			pWscControl->WscLEDTimerRunning = TRUE;
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: LED_WPS_SUCCESS => LED_WPS_TURN_LED_OFF\n", __func__);
			break;

		/* After turn off the blue LED for one second. */
		/* AP uses an encryption algorithm: */
		/* a) YES: Turn on the blue LED. */
		/* b) NO: Turn off the blue LED. */
		case LED_WPS_TURN_LED_OFF:
			if ((pWscControl->WscState == WSC_STATE_OFF) &&
				(pWscControl->WscStatus == STATUS_WSC_CONFIGURED)) {
				if (WscAPHasSecuritySetting(pAd, pWscControl) == TRUE) { /* The NIC connects with an AP using an encryption algorithm. */
					/* Turn WPS success LED. */
					WPSLEDStatus = LED_WPS_TURN_ON_BLUE_LED;
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: LED_WPS_TURN_LED_OFF => LED_WPS_TURN_ON_BLUE_LED\n", __func__);
				} else { /* The NIC connects with an AP using OPEN-NONE. */
					/* Turn off the WPS LED. */
					WPSLEDStatus = LED_WPS_TURN_LED_OFF;
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: LED_WPS_TURN_LED_OFF => LED_WPS_TURN_LED_OFF\n", __func__);
				}
			}

			break;

		/* Turn off the amber LED after 15 seconds. */
		case LED_WPS_ERROR:
			WPSLEDStatus = LED_WPS_TURN_LED_OFF; /* Turn off the WPS LED. */
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: LED_WPS_ERROR/LED_WPS_SESSION_OVERLAP_DETECTED => LED_WPS_TURN_LED_OFF\n", __func__);
			break;

		/* Turn off the amber LED after ~3 seconds. */
		case LED_WPS_SESSION_OVERLAP_DETECTED:
			WPSLEDStatus = LED_WPS_TURN_LED_OFF; /* Turn off the WPS LED. */
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: LED_WPS_SESSION_OVERLAP_DETECTED => LED_WPS_TURN_LED_OFF\n", __func__);
			break;

		default:
			/* do nothing. */
			break;
		}

		if (WPSLEDStatus)
			RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
	} else {
		/* do nothing. */
	}
}


VOID WscSkipTurnOffLEDTimer(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PWSC_CTRL pWscControl = (PWSC_CTRL)FunctionContext;
	/* Allow the NIC to turn off the WPS LED again. */
	pWscControl->bSkipWPSTurnOffLED = FALSE;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s: Allow the NIC to turn off the WPS LED again.\n", __func__);
}

#endif /* WSC_LED_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID WscUpdatePortCfgTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	WSC_CTRL *pWscControl = (WSC_CTRL *)FunctionContext;
	RTMP_ADAPTER *pAd = NULL;
	BOOLEAN	 bEnrollee = TRUE;
	WSC_CREDENTIAL *pCredential = NULL;
	BSS_STRUCT *pMbss = NULL;
#ifdef WPS_UNCONFIG_FEATURE_SUPPORT
	struct wifi_dev *wdev = NULL;
#endif
#ifdef CONFIG_MAP_SUPPORT
	PWSC_REG_DATA pReg = NULL;
#endif

	if (pWscControl == NULL)
		return;

	pCredential = (PWSC_CREDENTIAL) &pWscControl->WscProfile.Profile[0];
	pAd = (PRTMP_ADAPTER)pWscControl->pAd;

	if (pAd == NULL)
		return;

#ifdef CONFIG_MAP_SUPPORT
	pReg = (PWSC_REG_DATA) &pWscControl->RegData;

	if (pReg->PeerInfo.map_DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)) {
		UCHAR				apidx = (pWscControl->EntryIfIdx & 0x1F);
		UCHAR				band_idx = HcGetBandByWdev(&pAd->ApCfg.MBSSID[apidx].wdev);
		PWSC_CTRL			pBhWscControl = NULL;
		struct wifi_dev			*bh_wdev = NULL;

		bh_wdev = pAd->bh_bss_wdev[band_idx];

		if (bh_wdev) {
			pBhWscControl = &bh_wdev->WscControl;
			pWscControl = pBhWscControl;
		} else {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "bh_wdev is NULL\n");
			return;
		}
	}
#endif
	pMbss = &pAd->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x1F];
#ifdef WPS_UNCONFIG_FEATURE_SUPPORT
	wdev = &pAd->ApCfg.MBSSID[pWscControl->EntryIfIdx & 0x1F].wdev;
#endif
	if (WscGetAuthMode(pCredential->AuthType) == pMbss->wdev.SecConfig.AKMMap &&
		WscGetWepStatus(pCredential->EncrType) == pMbss->wdev.SecConfig.PairwiseCipher &&
		NdisEqualMemory(pMbss->Ssid, pCredential->SSID.Ssid, pCredential->SSID.SsidLength) &&
		NdisEqualMemory(pWscControl->WpaPsk, pCredential->Key, pCredential->KeyLength))
		return;

	if (pWscControl->WscProfile.ApplyProfileIdx & 0x8000)
		bEnrollee = FALSE;

	WscWriteConfToPortCfg(pAd,
						  pWscControl,
						  &pWscControl->WscProfile.Profile[0],
						  bEnrollee);
	pWscControl->WscProfile.ApplyProfileIdx &= 0x7FFF;
#ifdef P2P_SUPPORT

	if (pWscControl->EntryIfIdx & MIN_NET_DEVICE_FOR_P2P_GO) {
#ifdef RTMP_MAC_PCI
		OS_WAIT(1000);
#endif /* RTMP_MAC_PCI */
		P2P_GoStop(pAd);
		P2P_GoStartUp(pAd, MAIN_MBSSID);
	} else
#endif /* P2P_SUPPORT */
	{
		NDIS_STATUS enq_rv = 0;
		UCHAR apidx = pWscControl->EntryIfIdx & 0x1F;

		pAd->WriteWscCfgToDatFile = apidx;
		/*
		*	Using CMD thread to prevent in-band command failed.
		*	@20150710
		*/

#if defined(CONFIG_MAP_SUPPORT) && defined(WPS_UNCONFIG_FEATURE_SUPPORT)
		wapp_send_wps_config(pAd, wdev, &pWscControl->WscProfile.Profile[0]);
		/* Avoid AP restart when MAP enable since this will hit only when WPS unconfigured enable*/
		if (!IS_MAP_ENABLE(pAd)) {
#endif
			enq_rv = RTEnqueueInternalCmd(pAd, CMDTHREAD_AP_RESTART, (VOID *)&apidx, sizeof(UCHAR));
			MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"%s: en-queu CMDTHREAD_AP_RESTART - enq_rv = 0x%x\n", __func__, enq_rv);
#if defined(CONFIG_MAP_SUPPORT) && defined(WPS_UNCONFIG_FEATURE_SUPPORT)
		}
#endif
	}

	RtmpOsTaskWakeUp(&(pAd->wscTask));
}
#endif /* CONFIG_AP_SUPPORT */

VOID WscCheckPeerDPID(
	IN  PRTMP_ADAPTER pAd,
	IN  PFRAME_802_11 Fr,
	IN  PUCHAR eid_data,
	IN  INT eid_len,
	IN	UCHAR current_band)
{
	WSC_IE *pWscIE;
	PUCHAR pData = NULL;
	INT Len = 0;
	USHORT DevicePasswordID;
	PWSC_CTRL pWscCtrl = NULL;

	pData = eid_data + 4;
	Len = eid_len - 4;
#ifdef CONFIG_AP_SUPPORT

	if ((pAd->OpMode == OPMODE_AP)
#ifdef P2P_SUPPORT
		|| P2P_GO_ON(pAd)
#endif /* P2P_SUPPORT */
	   ) {
		UCHAR	ap_idx = 0;

		for (ap_idx = 0; ap_idx < pAd->ApCfg.BssidNum; ap_idx++) {
			if (NdisEqualMemory(Fr->Hdr.Addr1, pAd->ApCfg.MBSSID[ap_idx].wdev.bssid, MAC_ADDR_LEN)) {
				pWscCtrl = &pAd->ApCfg.MBSSID[ap_idx].wdev.WscControl;
				break;
			}
		}
	}

#endif /* CONFIG_AP_SUPPORT */

	if (Len <= 0 || (Len > sizeof(DevicePasswordID) + 4))
		return;

	while (Len > 0) {
		WSC_IE	WscIE;

		NdisMoveMemory(&WscIE, pData, sizeof(WSC_IE));
		/* Check for WSC IEs*/
		pWscIE = &WscIE;

		if (Len < (be2cpu16(pWscIE->Length) + 4)) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
				"unexpected WSC IE Length(%u)\n", be2cpu16(pWscIE->Length));
			break;
		}

		/* Check for device password ID, PBC = 0x0004*/
		if (be2cpu16(pWscIE->Type) == WSC_ID_DEVICE_PWD_ID) {
			/* Found device password ID*/
			NdisMoveMemory(&DevicePasswordID, pData + 4, sizeof(DevicePasswordID));
			DevicePasswordID = be2cpu16(DevicePasswordID);

			if (DevicePasswordID == DEV_PASS_ID_PBC) {	/* Check for PBC value*/
				WscPBC_DPID_FromSTA(pAd, Fr->Hdr.Addr2, current_band);
				hex_dump("PBC STA:", Fr->Hdr.Addr2, MAC_ADDR_LEN);
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "\n");
			} else if (DevicePasswordID == DEV_PASS_ID_PIN) {
				/*
				*	WSC 2.0 STA will send probe request with WPS IE anyway.
				*	Do NOT add this STA to WscPeerList after AP is triggered to do PBC.
				*/
				if (pWscCtrl &&
					(!pWscCtrl->bWscTrigger || (pWscCtrl->WscMode != WSC_PBC_MODE))) {
					RTMP_SEM_LOCK(&pWscCtrl->WscPeerListSemLock);
					WscInsertPeerEntryByMAC(&pWscCtrl->WscPeerList, Fr->Hdr.Addr2);
					RTMP_SEM_UNLOCK(&pWscCtrl->WscPeerListSemLock);
				}
			}

#ifdef IWSC_SUPPORT
			else if (DevicePasswordID == DEV_PASS_ID_SMPBC)
				IWSC_AddSmpbcEnrollee(pAd, Fr->Hdr.Addr2);

#endif /* IWSC_SUPPORT */
			else {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s : DevicePasswordID = 0x%04x\n",
						 __func__, DevicePasswordID);
			}

			break;
		}

#ifdef IWSC_SUPPORT

		if (pAd->StaCfg[0].BssType == BSS_ADHOC) {
			pWscCtrl = &pAd->StaCfg[0].wdev.WscControl;

			if ((be2cpu16(pWscIE->Type) == WSC_ID_CONFIG_METHODS) &&
				(pWscCtrl->WscConfMode == WSC_REGISTRAR) &&
				(pWscCtrl->bWscTrigger == TRUE)) {
				USHORT PeerConfigMethod = 0;
				PWSC_PEER_ENTRY pWscPeerEntry = NULL;

				RTMP_SEM_LOCK(&pWscCtrl->WscConfiguredPeerListSemLock);
				pWscPeerEntry = WscFindPeerEntry(&pWscCtrl->WscConfiguredPeerList, Fr->Hdr.Addr2);

				if (pWscPeerEntry == NULL) {
					NdisMoveMemory(&PeerConfigMethod, pData + 4, sizeof(PeerConfigMethod));
					PeerConfigMethod = be2cpu16(PeerConfigMethod);

					if (pWscCtrl->WscMode == WSC_PIN_MODE) {
						NdisMoveMemory(pWscCtrl->WscPeerMAC, Fr->Hdr.Addr2, MAC_ADDR_LEN);
						NdisMoveMemory(pWscCtrl->EntryAddr, Fr->Hdr.Addr2, MAC_ADDR_LEN);
					}

					MlmeEnqueue(pAd, IWSC_STATE_MACHINE, IWSC_MT2_PEER_PROBE_REQ, sizeof(USHORT), &PeerConfigMethod, 0);
					RTMP_MLME_HANDLER(pAd);
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s(): Add this peer: "MACSTR"\n",
							 __func__, MAC2STR(Fr->Hdr.Addr2));
				}

				RTMP_SEM_UNLOCK(&pWscCtrl->WscConfiguredPeerListSemLock);
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "%s() : PeerConfigMethod = 0x%04x\n",
						 __func__, PeerConfigMethod);
			}
		}

#endif /* IWSC_SUPPORT */

		/* Set the offset and look for PBC information*/
		/* Since Type and Length are both short type, we need to offset 4, not 2*/
		pData += (be2cpu16(pWscIE->Length) + 4);
		Len   -= (be2cpu16(pWscIE->Length) + 4);
	}
}

VOID WscClearPeerList(
	IN  PLIST_HEADER pWscEnList)
{
	RT_LIST_ENTRY *pEntry = NULL;

	pEntry = pWscEnList->pHead;

	while (pEntry != NULL) {
		removeHeadList(pWscEnList);
		os_free_mem(pEntry);
		pEntry = pWscEnList->pHead;
	}
}

PWSC_PEER_ENTRY	WscFindPeerEntry(
	IN  PLIST_HEADER pWscEnList,
	IN  PUCHAR pMacAddr)
{
	PWSC_PEER_ENTRY	pPeerEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pWscEnList->pHead;
	pPeerEntry = (PWSC_PEER_ENTRY)pListEntry;

	while (pPeerEntry != NULL) {
		if (NdisEqualMemory(pPeerEntry->mac_addr, pMacAddr, MAC_ADDR_LEN))
			return pPeerEntry;

		pListEntry = pListEntry->pNext;
		pPeerEntry = (PWSC_PEER_ENTRY)pListEntry;
	}

	return NULL;
}

VOID WscInsertPeerEntryByMAC(
	IN  PLIST_HEADER pWscEnList,
	IN  PUCHAR pMacAddr)
{
	PWSC_PEER_ENTRY pWscPeer = NULL;

	pWscPeer = WscFindPeerEntry(pWscEnList, pMacAddr);

	if (pWscPeer)
		NdisGetSystemUpTime(&pWscPeer->receive_time);
	else {
		os_alloc_mem(NULL, (UCHAR **)&pWscPeer, sizeof(WSC_PEER_ENTRY));

		if (pWscPeer) {
			NdisZeroMemory(pWscPeer, sizeof(WSC_PEER_ENTRY));
			pWscPeer->pNext = NULL;
			NdisMoveMemory(pWscPeer->mac_addr, pMacAddr, MAC_ADDR_LEN);
			NdisGetSystemUpTime(&pWscPeer->receive_time);
			insertTailList(pWscEnList, (RT_LIST_ENTRY *)pWscPeer);
		}

		ASSERT(pWscPeer != NULL);
	}
}

#ifdef CONFIG_AP_SUPPORT
INT WscApShowPeerList(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR ApIdx = 0;
	PWSC_CTRL pWscControl = NULL;
	PWSC_PEER_ENTRY	pPeerEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;
	PLIST_HEADER pWscEnList = NULL;

	for (ApIdx = 0; ApIdx < pAd->ApCfg.BssidNum; ApIdx++) {
		pWscControl = &pAd->ApCfg.MBSSID[ApIdx].wdev.WscControl;
		pWscEnList = &pWscControl->WscPeerList;

		if (pWscEnList->size != 0) {
			WscMaintainPeerList(pAd, pWscControl);
			RTMP_SEM_LOCK(&pWscControl->WscPeerListSemLock);
			pListEntry = pWscEnList->pHead;
			pPeerEntry = (PWSC_PEER_ENTRY)pListEntry;

			while (pPeerEntry != NULL) {
				MTWF_PRINT("MAC:"MACSTR"\tReveive Time:%lu\n",
						  MAC2STR(pPeerEntry->mac_addr),
						  pPeerEntry->receive_time);
				pListEntry = pListEntry->pNext;
				pPeerEntry = (PWSC_PEER_ENTRY)pListEntry;
			}

			RTMP_SEM_UNLOCK(&pWscControl->WscPeerListSemLock);
		}

		MTWF_PRINT("\n");
	}

	return TRUE;
}

INT WscApShowPin(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR inf_idx = pObj->ioctl_if;
	BOOLEAN bFromApCli;
	PWSC_CTRL pWscControl;
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		UCHAR idx_2;

		bFromApCli = TRUE;
		pWscControl = &pAd->StaCfg[inf_idx].wdev.WscControl;
		MTWF_PRINT("IF(apcli%d) WPS Information:\n", inf_idx);

		if (pWscControl->WscEnrolleePinCodeLen == 8)
			MTWF_PRINT("Enrollee PinCode(ApCli%d)        %08u\n",
					  inf_idx,
					  pWscControl->WscEnrolleePinCode);
		else
			MTWF_PRINT("Enrollee PinCode(ApCli%d)        %04u\n",
					  inf_idx,
					  pWscControl->WscEnrolleePinCode);

		MTWF_PRINT("Ap Client WPS Profile Count     = %d\n", pWscControl->WscProfile.ProfileCnt);

		for (idx_2 = 0; idx_2 < pWscControl->WscProfile.ProfileCnt; idx_2++) {
			PWSC_CREDENTIAL pCredential = &pWscControl->WscProfile.Profile[idx_2];

			MTWF_PRINT("Profile[%d]:\n", idx_2);
			MTWF_PRINT("SSID                            = %s\n", pCredential->SSID.Ssid);
			MTWF_PRINT("AuthType                        = %s\n", WscGetAuthTypeStr(pCredential->AuthType));
			MTWF_PRINT("EncrypType                      = %s\n", WscGetEncryTypeStr(pCredential->EncrType));
			MTWF_PRINT("KeyIndex                        = %d\n", pCredential->KeyIndex);

			if (pCredential->KeyLength != 0)
				MTWF_PRINT("Key                             = %s\n", pCredential->Key);
		}
	} else
#endif /* APCLI_SUPPORT */
	{
		bFromApCli = FALSE;
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
				 "IF(ra%d) WPS Information:\n", inf_idx);
#ifdef BB_SOC
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
				 "WPS Wsc2MinsTimerRunning(ra%d)        = %d\n",
				  inf_idx, pAd->ApCfg.MBSSID[inf_idx].wdev.WscControl.Wsc2MinsTimerRunning);

		/* display pin code */
		if (pAd->ApCfg.MBSSID[inf_idx].wdev.WscControl.WscEnrolleePinCodeLen == 8)
				MTWF_PRINT("Enrollee PinCode(ra%d)			%08u\n",
					inf_idx, pAd->ApCfg.MBSSID[inf_idx].wdev.WscControl.WscEnrolleePinCode);
		else
			MTWF_PRINT("Enrollee PinCode(ra%d)			%04u\n",
					inf_idx, pAd->ApCfg.MBSSID[inf_idx].wdev.WscControl.WscEnrolleePinCode);
#else

		/* display pin code */
		if (pAd->ApCfg.MBSSID[inf_idx].wdev.WscControl.WscEnrolleePinCodeLen == 8)
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
					 "Enrollee PinCode(ra%d)           %08u\n",
					  inf_idx, pAd->ApCfg.MBSSID[inf_idx].wdev.WscControl.WscEnrolleePinCode);
		else
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
					 "Enrollee PinCode(ra%d)           %04u\n",
					  inf_idx, pAd->ApCfg.MBSSID[inf_idx].wdev.WscControl.WscEnrolleePinCode);

#endif
	}

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "\n");
	return TRUE;
}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
INT WscStaShowPeerList(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PWSC_CTRL pWscControl = NULL;
	PWSC_PEER_ENTRY	pPeerEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;
	PLIST_HEADER pWscEnList = NULL;

	pWscControl = &pAd->StaCfg[0].wdev.WscControl;
	pWscEnList = &pWscControl->WscPeerList;
	RTMP_SEM_LOCK(&pWscControl->WscPeerListSemLock);

	if (pWscEnList->size != 0) {
		RTMP_SEM_UNLOCK(&pWscControl->WscPeerListSemLock);
		WscMaintainPeerList(pAd, pWscControl);
		RTMP_SEM_LOCK(&pWscControl->WscPeerListSemLock);
		pListEntry = pWscEnList->pHead;
		pPeerEntry = (PWSC_PEER_ENTRY)pListEntry;

		while (pPeerEntry != NULL) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "MAC:"MACSTR"\tReveive Time:%lu\n",
					 MAC2STR(pPeerEntry->mac_addr),
					 pPeerEntry->receive_time);
			pListEntry = pListEntry->pNext;
			pPeerEntry = (PWSC_PEER_ENTRY)pListEntry;
		}
	}

	RTMP_SEM_UNLOCK(&pWscControl->WscPeerListSemLock);
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "\n");
	return TRUE;
}
#endif /* CONFIG_STA_SUPPORT */

VOID WscMaintainPeerList(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWpsCtrl)
{
	PWSC_PEER_ENTRY	pPeerEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL, *pTempListEntry = NULL;
	PLIST_HEADER pWscEnList = NULL;
	ULONG now_time = 0;

	RTMP_SEM_LOCK(&pWpsCtrl->WscPeerListSemLock);
	pWscEnList = &pWpsCtrl->WscPeerList;
	NdisGetSystemUpTime(&now_time);
	pListEntry = pWscEnList->pHead;
	pPeerEntry = (PWSC_PEER_ENTRY)pListEntry;

	while (pPeerEntry != NULL) {
		if (RTMP_TIME_AFTER(now_time, pPeerEntry->receive_time + (30 * OS_HZ))) {
			pTempListEntry = pListEntry->pNext;
			delEntryList(pWscEnList, pListEntry);
			os_free_mem(pPeerEntry);
			pListEntry = pTempListEntry;
		} else
			pListEntry = pListEntry->pNext;

		pPeerEntry = (PWSC_PEER_ENTRY)pListEntry;
	}

	RTMP_SEM_UNLOCK(&pWpsCtrl->WscPeerListSemLock);
}

VOID WscDelListEntryByMAC(
	IN  PLIST_HEADER pWscEnList,
	IN  PUCHAR pMacAddr)
{
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = (RT_LIST_ENTRY *)WscFindPeerEntry(pWscEnList, pMacAddr);

	if (pListEntry) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
				 "WscDelListEntryByMAC : pMacAddr = "MACSTR"\n", MAC2STR(pMacAddr));
		delEntryList(pWscEnList, pListEntry);
		os_free_mem(pListEntry);
	}
}

VOID	WscAssignEntryMAC(
	IN  PRTMP_ADAPTER pAd,
	IN  PWSC_CTRL pWpsCtrl)
{
	PWSC_PEER_ENTRY pPeerEntry = NULL;

	WscMaintainPeerList(pAd, pWpsCtrl);
	RTMP_SEM_LOCK(&pWpsCtrl->WscPeerListSemLock);
	pPeerEntry = (PWSC_PEER_ENTRY)pWpsCtrl->WscPeerList.pHead;
	NdisZeroMemory(pWpsCtrl->EntryAddr, MAC_ADDR_LEN);

	if (pPeerEntry)
		NdisMoveMemory(pWpsCtrl->EntryAddr, pPeerEntry->mac_addr, MAC_ADDR_LEN);

	RTMP_SEM_UNLOCK(&pWpsCtrl->WscPeerListSemLock);
}


/*
*	Get WSC IE data from WSC Peer by Tag.
*/
BOOLEAN WscGetDataFromPeerByTag(
	IN  PRTMP_ADAPTER pAd,
	IN  PUCHAR pIeData,
	IN  INT IeDataLen,
	IN  USHORT WscTag,
	OUT PUCHAR pWscBuf,
	OUT PUSHORT pWscBufLen)
{
	PUCHAR pData = pIeData;
	INT Len = 0;
	USHORT DataLen = 0;
	PWSC_IE pWscIE;

	Len = IeDataLen;

	if (Len <= 0 || Len > 512)
		return FALSE;

	while (Len > 0) {
		WSC_IE	WscIE;

		NdisMoveMemory(&WscIE, pData, sizeof(WSC_IE));
		/* Check for WSC IEs */
		pWscIE = &WscIE;

		if (Len < (be2cpu16(pWscIE->Length) + 4)) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "unexpected WSC IE Length(%u)\n", be2cpu16(pWscIE->Length));
			break;
		}

		if (be2cpu16(pWscIE->Type) == WscTag) {
			DataLen = be2cpu16(pWscIE->Length);
			if (DataLen <=  (1 << (sizeof(USHORT) * 8)) - 1) {
				if (pWscBufLen)
					*pWscBufLen = DataLen;
				NdisMoveMemory(pWscBuf, pData + 4, DataLen);
				return TRUE;
			}
		}

		/* Set the offset and look for next WSC Tag information */
		/* Since Type and Length are both short type, we need to offset 4, not 2 */
		pData += (be2cpu16(pWscIE->Length) + 4);
		Len   -= (be2cpu16(pWscIE->Length) + 4);
	}

	return FALSE;
}

VOID WscUUIDInit(
	IN  PRTMP_ADAPTER pAd,
	IN  INT inf_idx,
	IN  UCHAR from_apcli)
{
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT

	if (from_apcli) {
		PWSC_CTRL pWpsCtrl = &pAd->StaCfg[inf_idx].wdev.WscControl;
#ifdef WSC_V2_SUPPORT
		PWSC_V2_INFO	pWscV2Info;
#endif /* WSC_V2_SUPPORT */
		NdisZeroMemory(pWpsCtrl->EntryAddr, MAC_ADDR_LEN);
#ifdef WSC_V2_SUPPORT
		pWpsCtrl->WscConfigMethods = 0x238C;
#else /* WSC_V2_SUPPORT */
		pWpsCtrl->WscConfigMethods = 0x018C;
#endif /* !WSC_V2_SUPPORT */
		WscGenerateUUID(pAd, &pWpsCtrl->Wsc_Uuid_E[0],
						&pWpsCtrl->Wsc_Uuid_Str[0], 0, FALSE, from_apcli);
		pWpsCtrl->bWscFragment = FALSE;
		pWpsCtrl->WscFragSize = 128;
		pWpsCtrl->WscRxBufLen = 0;

		if (pWpsCtrl->pWscRxBuf) {
			os_free_mem(pWpsCtrl->pWscRxBuf);
			pWpsCtrl->pWscRxBuf = NULL;
		}

		os_alloc_mem(pAd, &pWpsCtrl->pWscRxBuf, MAX_MGMT_PKT_LEN);

		if (pWpsCtrl->pWscRxBuf)
			NdisZeroMemory(pWpsCtrl->pWscRxBuf, MAX_MGMT_PKT_LEN);

		pWpsCtrl->WscTxBufLen = 0;

		if (pWpsCtrl->pWscTxBuf) {
			os_free_mem(pWpsCtrl->pWscTxBuf);
			pWpsCtrl->pWscTxBuf = NULL;
		}

		os_alloc_mem(pAd, &pWpsCtrl->pWscTxBuf, MAX_MGMT_PKT_LEN);

		if (pWpsCtrl->pWscTxBuf)
			NdisZeroMemory(pWpsCtrl->pWscTxBuf, MAX_MGMT_PKT_LEN);

		initList(&pWpsCtrl->WscPeerList);
		NdisAllocateSpinLock(pAd, &pWpsCtrl->WscPeerListSemLock);
		pWpsCtrl->PinAttackCount = 0;
		pWpsCtrl->bSetupLock = FALSE;
#ifdef WSC_V2_SUPPORT
		pWscV2Info = &pWpsCtrl->WscV2Info;
		pWscV2Info->bWpsEnable = TRUE;
		pWscV2Info->ExtraTlv.TlvLen = 0;
		pWscV2Info->ExtraTlv.TlvTag = 0;
		pWscV2Info->ExtraTlv.pTlvData = NULL;
		pWscV2Info->ExtraTlv.TlvType = TLV_ASCII;
		pWscV2Info->bEnableWpsV2 = TRUE;
#endif /* WSC_V2_SUPPORT */
		WscInit(pAd, TRUE, inf_idx);
	} else
#endif /* APCLI_SUPPORT */
	{
		{
			PWSC_CTRL pWscControl;
			UCHAR zeros16[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

			pWscControl = &pAd->ApCfg.MBSSID[inf_idx].wdev.WscControl;
			MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Generate UUID for apidx(%d)\n", inf_idx);

			if (NdisEqualMemory(&pWscControl->Wsc_Uuid_E[0], zeros16, UUID_LEN_HEX))
				WscGenerateUUID(pAd, &pWscControl->Wsc_Uuid_E[0], &pWscControl->Wsc_Uuid_Str[0], inf_idx, FALSE, from_apcli);

			WscInit(pAd, FALSE, inf_idx);
		}
	}

#endif /* CONFIG_AP_SUPPORT */
}

#endif /* WSC_INCLUDED */
