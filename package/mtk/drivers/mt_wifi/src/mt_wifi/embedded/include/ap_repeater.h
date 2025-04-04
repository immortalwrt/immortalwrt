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
    ap_repeater.h

    Abstract:
    repeater function related definition collection.

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Carter.Chen 2015-April-14   init version.
*/
#ifndef __AP_REPEATER_H__
#define __AP_REPEATER_H__

#include    "rtmp.h"

#define GET_MAX_REPEATER_ENTRY_NUM(_pChipCap)	_pChipCap->MaxRepeaterNum /* repeater pool for all bands */
#define GET_PER_BAND_MAX_REPEATER_ENTRY_NUM(_pChipCap)	((_pChipCap->MaxRepeaterNum)/(_pChipCap->band_cnt)) /* per band max repeater entry */

/*Repeater Client Type for REPEATER_CLIENT_ENTRY.Cli_Type*/
#define REPT_UNKNOWN_CLI 0x00
#define REPT_ETH_CLI 0x01
#define REPT_BRIDGE_CLI 0x02
#define REPT_WIRELESS_CLI 0x04

/*Operation macro for REPEATER_CLIENT_ENTRY.Cli_Type*/
#define SET_REPT_CLI_TYPE(_pRept, _type) ((_pRept)->Cli_Type = (_type))
#define ADD_REPT_CLI_TYPE(_pRept, _type) ((_pRept)->Cli_Type |= (_type))
#define CLEAN_REPT_CLI_TYPE(_pRept) ((_pRept)->Cli_Type = REPT_UNKNOWN_CLI)

/*Macro for checking*/
#define IS_REPT_CLI_TYPE(_pRept, _type) ((_pRept) ? (_pRept)->Cli_Type & (_type) : 0)
#define IS_REPT_LINK_UP(_pRept) ((_pRept) ? (_pRept)->CliConnectState == REPT_ENTRY_CONNTED : 0)
#define IS_REPT_IN_DISCONNECTING(_pRept) ((_pRept) ? (_pRept)->CliDisconnectState == REPT_ENTRY_DISCONNT_STATE_DISCONNTING : 0)

VOID RepeaterCtrlInit(RTMP_ADAPTER *pAd);
VOID RepeaterCliReset(RTMP_ADAPTER *pAd);
VOID RepeaterCtrlExit(RTMP_ADAPTER *pAd, UCHAR band_idx);
VOID CliLinkMapInit(RTMP_ADAPTER *pAd);

enum _REPT_ENTRY_CONNT_STATE {
	REPT_ENTRY_DISCONNT = 0,
	REPT_ENTRY_CONNTING = 1,
	REPT_ENTRY_CONNTED = 2,
};

enum _REPT_ENTRY_DISCONNT_STATE {
	REPT_ENTRY_DISCONNT_STATE_UNKNOWN = 0,
	REPT_ENTRY_DISCONNT_STATE_DISCONNTING = 1,
};


enum _REPEATER_MAC_ADDR_RULE_TYPE {
	FOLLOW_CLI_LINK_MAC_ADDR_OUI = 0,
	CASUALLY_DEFINE_MAC_ADDR = 1,
	VENDOR_DEFINED_MAC_ADDR_OUI = 2,
};

enum _REPEATER_TX_PKT_CHECK_RESULT {
	REPEATER_ENTRY_EXIST = 0,
	INSERT_REPT_ENTRY = 1,
	USE_CLI_LINK_INFO = 2,
	INSERT_REPT_ENTRY_AND_ALLOW = 3,
};

/* IOCTL*/
INT Show_ReptTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID RepeaterLinkMonitor(RTMP_ADAPTER *pAd);

UINT32 ReptTxPktCheckHandler(
	RTMP_ADAPTER *pAd,
	IN struct wifi_dev *cli_link_wdev,
	IN PNDIS_PACKET pPacket,
	OUT UINT16 *pWcid);

INT ReptGetMuarIdxByCliIdx(RTMP_ADAPTER *pAd, UCHAR CliIdx, UCHAR *muar_idx);

VOID RepeaterFillMlmeParaThenEnq(
	RTMP_ADAPTER *pAd,
	ULONG Machine,
	ULONG MsgType,
	REPEATER_CLIENT_ENTRY *pReptEntry);

INT AsicSetReptFuncEnable(RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR band_idx);

REPEATER_CLIENT_ENTRY *RTMPLookupRepeaterCliEntry(
	IN VOID *pData,
	IN BOOLEAN bRealMAC,
	IN PUCHAR pAddr,
	IN BOOLEAN bIsPad);

BOOLEAN RTMPQueryLookupRepeaterCliEntryMT(
	IN PVOID pData,
	IN PUCHAR pAddr,
	IN BOOLEAN bIsPad);

VOID RTMPInsertRepeaterEntry(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *main_sta_wdev,
	PUCHAR pAddr);

VOID RTMPRemoveRepeaterEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR CliIdx);

VOID RTMPRepeaterReconnectionCheck(
	IN RTMP_ADAPTER *pAd);

MAC_TABLE_ENTRY *RTMPInsertRepeaterMacEntry(
	IN  RTMP_ADAPTER *pAd,
	IN  PUCHAR pAddr,
	IN  struct wifi_dev *wdev,
	IN  UCHAR apidx,
	IN  UCHAR cliIdx,
	IN BOOLEAN CleanAll);

BOOLEAN RTMPRepeaterVaildMacEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN UCHAR band_idx);

INVAILD_TRIGGER_MAC_ENTRY *RepeaterInvaildMacLookup(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR * pAddr);

VOID InsertIgnoreAsRepeaterEntryTable(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR * pAddr);

BOOLEAN RepeaterRemoveIngoreEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR idx,
	IN UCHAR * pAddr);

INT Show_Repeater_Cli_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID ApCliAuthTimeoutExt(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID ApCliAssocTimeoutExt(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);


VOID UpdateMbssCliLinkMap(
	RTMP_ADAPTER *pAd,
	UCHAR MbssIdx,
	struct wifi_dev *cli_link_wdev,
	struct wifi_dev *mbss_link_wdev);

VOID RepeaterDisconnectRootAP(
    RTMP_ADAPTER *pAd,
    REPEATER_CLIENT_ENTRY *pReptCli,
    UINT reason);

VOID repeater_disconnect_by_band(
	RTMP_ADAPTER *ad,
	UCHAR band_idx);

#ifdef REPEATER_TX_RX_STATISTIC
BOOLEAN MtRepeaterGetTxRxInfo(
	RTMP_ADAPTER *pAd,
	UINT16 wcid,
	UCHAR CliIfIndex,
	RETRTXRXINFO *pReptStatis);
#endif /* REPEATER_TX_RX_STATISTIC */
REPEATER_CLIENT_ENTRY *lookup_rept_entry(RTMP_ADAPTER *pAd, PUCHAR address);

VOID ReptWaitLinkDown(REPEATER_CLIENT_ENTRY *pReptEntry);

BOOLEAN repeater_enable_by_any_band(RTMP_ADAPTER *ad);
VOID repeater_set_enable(RTMP_ADAPTER *ad, BOOLEAN enable, UINT8 idx);
BOOLEAN repeater_get_enable(RTMP_ADAPTER *ad, UINT8 idx);
PNET_DEV repeater_get_apcli_ifdev(RTMP_ADAPTER *ad, MAC_TABLE_ENTRY *mac_entry);

#endif  /* __AP_REPEATER_H__ */
