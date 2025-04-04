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
    ap.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#ifndef __AP_H__
#define __AP_H__

#ifdef DOT11R_FT_SUPPORT
#include "ft_cmm.h"
#endif /* DOT11R_FT_SUPPORT */

#ifdef DELAY_TCP_ACK_V2
#define PEAK_TP_AVG_TX_PKT_LEN 256
#define PEAK_TP_AVG_RX_PKT_LEN 1000
#define PEAK_TP_WO_REPORT_TIME 0x6
#endif /* DELAY_TCP_ACK_V2 */
#define MULTI_CLIENT_NUMS_EAP_TH 16

/* mcli iwpriv cmd */
#define MCLI_RPS_ADJUST_ENABLE	1
#define MCLI_FORCE_AGGLIMIT 	2
#define MCLI_SHOW_INFO 		3
#define MCLI_DEBUG_ON		4
#define MCLI_RX_RPS_ENABLE	5
#define MCLI_RX_RPS_CPUMAP	6
#define MCLI_FORCE_RPS_CFG	7
#define MCLI_FORCE_TX_PROCESS_CNT	8
#define MCLI_CLI_NUMS_EAP_TH		9

#define PER_DN_TH 35
#define PER_UP_TH 15
#define WINSIZE_KP_IDX 0x3

#define NEAR_FAR_FAST_PHY_RATE_TH	520
#define NEAR_FAR_SLOW_PHY_RATE_TH	54

/* ============================================================= */
/*      Common definition */
/* ============================================================= */
#define MBSS_VLAN_INFO_GET(__pAd, __VLAN_VID, __VLAN_Priority, __func_idx) \
	{																		\
		if ((__func_idx < __pAd->ApCfg.BssidNum) &&					\
			(VALID_MBSS(__pAd, __func_idx)) &&						\
			(__pAd->ApCfg.MBSSID[__func_idx].wdev.VLAN_VID != 0)) {			\
				__VLAN_VID = __pAd->ApCfg.MBSSID[__func_idx].wdev.VLAN_VID;	\
				__VLAN_Priority = __pAd->ApCfg.MBSSID[__func_idx].wdev.VLAN_Priority; \
		}																	\
	}

#define WDEV_VLAN_INFO_GET(__pAd, __VLAN_VID, __VLAN_Priority, __wdev) \
	{																		\
		if ((__wdev->VLAN_VID != 0)) {			\
			__VLAN_VID = __wdev->VLAN_VID;	\
			__VLAN_Priority = __wdev->VLAN_Priority; \
		}																	\
	}

#if !defined(HOSTAPD_11R_SUPPORT) && !defined(HOSTAPD_WPA3_SUPPORT)
typedef struct _AUTH_FRAME_INFO {
	UCHAR addr1[MAC_ADDR_LEN];
	UCHAR addr2[MAC_ADDR_LEN];
	USHORT auth_alg;
	USHORT auth_seq;
	USHORT auth_status;
	CHAR Chtxt[CIPHER_TEXT_LEN];
#ifdef DOT11R_FT_SUPPORT
	FT_INFO FtInfo;
#endif /* DOT11R_FT_SUPPORT */
} AUTH_FRAME_INFO;
#endif


#ifdef CONVERTER_MODE_SWITCH_SUPPORT
typedef enum _ENUM_AP_START_STATE_T	{
	AP_STATE_ALWAYS_START_AP_DEFAULT = 0,
	AP_STATE_START_AFTER_APCLI_CONNECTION,
	AP_STATE_NEVER_START_AP,
	AP_STATE_INVALID_MAX
} ENUM_AP_START_STATE;

typedef enum _ENUM_APCLI_MODE_T	{
	APCLI_MODE_ALWAYS_START_AP_DEFAULT = 0,
	APCLI_MODE_START_AP_AFTER_APCLI_CONNECTION,
	APCLI_MODE_NEVER_START_AP,
	APCLI_MODE_INVALID_MAX
} ENUM_APCLI_MODE;

#endif /* CONVERTER_MODE_SWITCH_SUPPORT */



typedef enum _ENUM_AP_BSS_OPER_T {
	AP_BSS_OPER_ALL = 0,
	AP_BSS_OPER_BY_RF,
	AP_BSS_OPER_SINGLE,
	AP_BSS_OPER_NUM
} ENUM_AP_BSS_OPER;


#ifdef CONN_FAIL_EVENT
struct CONN_FAIL_MSG {
	CHAR Ssid[32];
	UCHAR SsidLen;
	UCHAR StaAddr[6];
	USHORT ReasonCode;
};
#endif
/* ============================================================= */
/*      Function Prototypes */
/* ============================================================= */

BOOLEAN APBridgeToWirelessSta(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pHeader,
	IN  UINT            HdrLen,
	IN  PUCHAR          pData,
	IN  UINT            DataLen,
	IN  ULONG           fromwdsidx);
#ifdef ANTENNA_DIVERSITY_SUPPORT
VOID ant_diversity_ctrl_init(RTMP_ADAPTER *pAd, UINT8 band_idx);
VOID ant_diversity_ctrl_reset(RTMP_ADAPTER *pAd);
UINT8 ant_diversity_allowed(RTMP_ADAPTER *pAd, UINT8 band_idx);
UINT8 ant_diversity_update_indicator(RTMP_ADAPTER *pAd);
VOID ant_diversity_process(RTMP_ADAPTER *pAd, UINT8 band_idx);
VOID show_ant_diversity_debug_log(RTMP_ADAPTER *pAd, UINT8 band_idx);
VOID ant_diversity_periodic_exec(RTMP_ADAPTER *pAd);
INT set_ant_diversity_debug_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ant_diversity_disable_forcedly(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ant_diversity_dbg_tp_deg_th_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ant_diversity_dbg_cn_deg_th_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ant_diversity_dbg_rx_rate_deg_th_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ant_diversity_dbg_countdown_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ant_diversity_dbg_ul_th_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ant_diversity_select_antenna(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ant_diversity_rx_rate_delta_th(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_read_interval_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ant_diversity_cn_deg_continuous_th(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ant_diversity_rx_rate_deg_continuous_th(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
INT ap_tx_pkt_allowed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);
INT ap_fp_tx_pkt_allowed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);
INT ap_rx_pkt_allowed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk);
INT ap_rx_ps_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk);
BOOLEAN ap_dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry);
INT ap_rx_pkt_foward(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);
INT ap_mlme_mgmtq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk);
INT ap_mlme_dataq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk);
INT ap_ieee_802_3_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry);
INT ap_ieee_802_11_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry);
INT ap_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT ap_link_up(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT ap_link_down(struct wifi_dev *wdev);
INT ap_inf_open(struct wifi_dev *wdev);
INT ap_inf_close(struct wifi_dev *wdev);
INT ap_send_data_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);
INT ap_send_mlme_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, struct wifi_dev *wdev, UCHAR q_idx, BOOLEAN is_data_queue);
UINT32 starec_ap_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature);

NDIS_STATUS APInsertPsQueue(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pPacket,
	IN STA_TR_ENTRY * tr_entry,
	IN UCHAR QueIdx);

INT ap_tx_pkt_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
INT ap_legacy_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
INT ap_ampdu_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
INT ap_amsdu_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
INT ap_frag_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
VOID ap_ieee_802_11_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
VOID ap_ieee_802_3_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
VOID ap_find_cipher_algorithm(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
BOOLEAN ap_fill_non_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
BOOLEAN ap_fill_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);

VOID rx_eapol_frm_handle(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx);

VOID ap_rx_error_handle(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
BOOLEAN ap_chk_cl2_cl3_err(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
VOID ap_cls3_err_action(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

VOID RTMPDescriptorEndianChange(UCHAR *pData, ULONG DescriptorType);

VOID RTMPFrameEndianChange(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR *pData,
	IN  ULONG Dir,
	IN  BOOLEAN FromRxDoneInt);

/* ap_assoc.c */

VOID APAssocStateMachineInit(
	IN  PRTMP_ADAPTER   pAd,
	IN  STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID MbssKickOutStas(RTMP_ADAPTER *pAd, INT apidx, USHORT Reason);
VOID APMlmeKickOutSta(RTMP_ADAPTER *pAd, UCHAR *staAddr, UINT16 Wcid, USHORT Reason);

#ifdef BW_VENDOR10_CUSTOM_FEATURE
BOOLEAN IsClientConnected(RTMP_ADAPTER *pAd);
#endif

#ifdef DOT11W_PMF_SUPPORT
VOID APMlmeKickOutAllSta(RTMP_ADAPTER *pAd, UCHAR apidx, USHORT Reason);
#endif /* DOT11W_PMF_SUPPORT */

VOID  APCls3errAction(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_WPA3_SUPPORT)
/*for ap_assoc in cfg mode*/
BOOLEAN PeerAssocReqCmmSanity(
	RTMP_ADAPTER *pAd,
	BOOLEAN isReassoc,
	VOID *Msg,
	INT MsgLen,
	IE_LISTS * ie_lists);

USHORT APBuildAssociation(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN IE_LISTS * ie_list,
	IN UCHAR MaxSupportedRateIn500Kbps,
	OUT USHORT *pAid,
	IN BOOLEAN isReassoc);
#endif

/* ap_auth.c */

void APAuthStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID APCls2errAction(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

/* ap_connect.c */

#ifdef CONFIG_AP_SUPPORT
#ifdef MT_MAC
VOID APCheckBcnQHandler(RTMP_ADAPTER *pAd, INT apidx, BOOLEAN *is_pretbtt_int);
#endif
#ifdef ZERO_LOSS_CSA_SUPPORT
VOID CSALastBcnTxEventTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID ChnlSwitchStaNullAckWaitTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif /*ZERO_LOSS_CSA_SUPPORT*/

VOID CSAEventTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

#endif /* CONFIG_AP_SUPPORT */

/* ap_sync.c */
VOID ApSiteSurvey_by_wdev(
	IN	PRTMP_ADAPTER		pAd,
	IN	PNDIS_802_11_SSID	pSsid,
	IN	UCHAR				ScanType,
	IN	BOOLEAN				ChannelSel,
	IN  struct wifi_dev	*wdev);
#ifdef TXRX_STAT_SUPPORT
VOID Update_LastSec_TXRX_Stats(
	IN PRTMP_ADAPTER   pAd);
#endif
VOID Update_Wtbl_Counters(
	IN PRTMP_ADAPTER   pAd);
VOID SupportRate(
	IN struct legacy_rate *rate,
	OUT PUCHAR * Rates,
	OUT PUCHAR RatesLen,
	OUT PUCHAR pMaxSupportRate);
#ifdef OFFCHANNEL_SCAN_FEATURE
UCHAR Channel2Index(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR		channel,
	IN UCHAR		BandIdx);

INT ApSiteSurveyNew_by_wdev(
	IN	PRTMP_ADAPTER	pAd,
	IN	UINT			Channel,
	IN UINT			Timeout,
	IN	UCHAR			ScanType,
	IN	BOOLEAN			ChannelSel,
	IN  struct wifi_dev *wdev);
#endif

#ifdef DOT11_N_SUPPORT
VOID APUpdateOperationMode(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

#ifdef DOT11N_DRAFT3
VOID APOverlappingBSSScan(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

INT GetBssCoexEffectedChRange(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN BSS_COEX_CH_RANGE * pCoexChRange,
	IN UCHAR Channel);
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */


/* ap_mlme.c */
VOID APMlmePeriodicExec(RTMP_ADAPTER *pAd);

BOOLEAN APMsgTypeSubst(
	IN PRTMP_ADAPTER pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType);

VOID APQuickResponeForRateUpExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID APAsicEvaluateRxAnt(RTMP_ADAPTER *pAd);
VOID APAsicRxAntEvalTimeout(RTMP_ADAPTER *pAd);

/* ap.c */
UCHAR get_apidx_by_addr(RTMP_ADAPTER *pAd, UCHAR *addr);

NDIS_STATUS APOneShotSettingInitialize(RTMP_ADAPTER *pAd);

/* INT ap_func_init(RTMP_ADAPTER *pAd); */

VOID APShutdown(RTMP_ADAPTER *pAd);

VOID APStartUpForMbss(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss);
VOID APStartUp(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, ENUM_AP_BSS_OPER oper);
VOID APStartUpByBss(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss);

VOID APStop(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, ENUM_AP_BSS_OPER oper);
VOID APStopByBss(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss);

VOID APCleanupPsQueue(RTMP_ADAPTER *pAd, QUEUE_HEADER *pQueue);


VOID MacTableMaintenance(RTMP_ADAPTER *pAd);

UINT32 MacTableAssocStaNumGet(RTMP_ADAPTER *pAd);

MAC_TABLE_ENTRY *APSsPsInquiry(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pAddr,
	OUT SST * Sst,
	OUT USHORT          *Aid,
	OUT UCHAR           *PsMode,
	OUT UCHAR           *Rate);

#ifdef SYSTEM_LOG_SUPPORT
VOID ApLogEvent(
	IN PRTMP_ADAPTER    pAd,
	IN PUCHAR           pAddr,
	IN USHORT           Event);
#else
#define ApLogEvent(_pAd, _pAddr, _Event)
#endif /* SYSTEM_LOG_SUPPORT */


VOID ApUpdateCapabilityAndErpIe(RTMP_ADAPTER *pAd, struct _BSS_STRUCT *mbss);

BOOLEAN ApCheckAccessControlList(RTMP_ADAPTER *pAd, UCHAR *addr, UCHAR apidx);
VOID ApUpdateAccessControlList(RTMP_ADAPTER *pAd, UCHAR apidx);


#ifdef AP_QLOAD_SUPPORT
VOID QBSS_LoadInit(RTMP_ADAPTER *pAd);
VOID QBSS_LoadAlarmReset(RTMP_ADAPTER *pAd);
VOID QBSS_LoadAlarmResume(RTMP_ADAPTER *pAd);
UINT32 QBSS_LoadBusyTimeGet(RTMP_ADAPTER *pAd);
BOOLEAN QBSS_LoadIsAlarmIssued(RTMP_ADAPTER *pAd);
BOOLEAN QBSS_LoadIsBusyTimeAccepted(RTMP_ADAPTER *pAd, UINT32 BusyTime);
UINT32 QBSS_LoadElementAppend(RTMP_ADAPTER *pAd, UINT8 *pBeaconBuf, QLOAD_CTRL *pQloadCtrl, UCHAR apidx);
VOID QBSS_LoadUpdate(RTMP_ADAPTER *pAd, ULONG UpTime);
VOID update_ap_qload_to_bcn(RTMP_ADAPTER *pAd);
VOID QBSS_LoadStatusClear(struct wifi_dev *wdev);

INT	Show_QoSLoad_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef CONFIG_HOTSPOT_R2
UINT32 QBSS_LoadElementAppend_HSTEST(RTMP_ADAPTER *pAd, UINT8 *pBeaconBuf, UCHAR apidx);
#endif /* CONFIG_HOTSPOT_R2 */
#endif /* AP_QLOAD_SUPPORT */


#ifdef DOT1X_SUPPORT
INT	Set_OwnIPAddr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_EAPIfName_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_PreAuthIfName_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/* Define in ap.c */
BOOLEAN DOT1X_InternalCmdAction(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN UINT8 cmd);

BOOLEAN DOT1X_EapTriggerAction(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);
#endif /* DOT1X_SUPPORT */

VOID AP_E2PROM_IOCTL_PostCtrl(RTMP_IOCTL_INPUT_STRUCT *wrq, RTMP_STRING *msg);

VOID IAPP_L2_UpdatePostCtrl(RTMP_ADAPTER *pAd, UINT8 *mac, INT wdev_idx);

INT rtmp_ap_init(RTMP_ADAPTER *pAd);
VOID rtmp_ap_exit(RTMP_ADAPTER *pAd);

BOOLEAN media_state_connected(struct wifi_dev *wdev);

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
UINT32 vow_clone_legacy_frame(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk);
#endif
VOID ap_over_lapping_scan(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss);
VOID ap_handle_mic_error_event(RTMP_ADAPTER *ad, MAC_TABLE_ENTRY *entry, RX_BLK *rx_blk);
VOID ap_fsm_ops_hook(struct wifi_dev *wdev);

#ifdef CONN_FAIL_EVENT
void ApSendConnFailMsg(
	PRTMP_ADAPTER pAd,
	CHAR *Ssid,
	UCHAR SsidLen,
	UCHAR *StaAddr,
	USHORT ReasonCode);
#endif
#endif  /* __AP_H__ */

