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

	All CFG80211 Function Prototype.

***************************************************************************/

#ifndef __CFG80211EXTR_H__
#define __CFG80211EXTR_H__

#ifdef RT_CFG80211_SUPPORT

#define CFG80211CB				    (pAd->pCfg80211_CB)
#define RT_CFG80211_DEBUG			/* debug use */
#ifdef RT_CFG80211_DEBUG
#define CFG80211DBG(__Flg, __pMsg)		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, __Flg, __pMsg)
#else
#define CFG80211DBG(__Flg, __pMsg)
#endif /* RT_CFG80211_DEBUG */

/* CFG_TODO */
#include "wfa_p2p.h"

#define RT_CFG80211_REGISTER(__pDev, __pNetDev)								\
	CFG80211_Register(__pDev, __pNetDev);

#define RT_CFG80211_BEACON_CR_PARSE(__pAd, __pVIE, __LenVIE)				\
	CFG80211_BeaconCountryRegionParse((VOID *)__pAd, __pVIE, __LenVIE);

#define RT_CFG80211_BEACON_TIM_UPDATE(__pAd)                                \
	CFG80211_UpdateBeacon((VOID *)__pAd, NULL, 0, NULL, 0, FALSE);

#define RT_CFG80211_CRDA_REG_HINT(__pAd, __pCountryIe, __CountryIeLen)		\
	CFG80211_RegHint((VOID *)__pAd, __pCountryIe, __CountryIeLen);

#define RT_CFG80211_CRDA_REG_HINT11D(__pAd, __pCountryIe, __CountryIeLen)	\
	CFG80211_RegHint11D((VOID *)__pAd, __pCountryIe, __CountryIeLen);

#define RT_CFG80211_CRDA_REG_RULE_APPLY(__pAd)								\
	CFG80211_RegRuleApply((VOID *)__pAd, NULL, __pAd->cfg80211_ctrl.Cfg80211_Alpha2);

#define RT_CFG80211_CONN_RESULT_INFORM(__pAd, __pBSSID, __pReqIe,			\
									   __ReqIeLen,	__pRspIe, __RspIeLen, __FlgIsSuccess)				\
CFG80211_ConnectResultInform((VOID *)__pAd, __pBSSID,					\
							 __pReqIe, __ReqIeLen, __pRspIe, __RspIeLen, __FlgIsSuccess);

#define RT_CFG80211_SCANNING_INFORM(__pAd, __BssIdx, __ChanId, __pFrame,	\
									__FrameLen, __RSSI)									\
CFG80211_Scaning((VOID *)__pAd, __BssIdx, __ChanId, __pFrame,			\
				 __FrameLen, __RSSI);

#define RT_CFG80211_SCAN_END(__pAd, __FlgIsAborted)							\
	CFG80211_ScanEnd((VOID *)__pAd, __FlgIsAborted);
#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
#define RT_CFG80211_LOST_AP_INFORM(__pAd, __wdev)									\
	CFG80211_LostApInform((VOID *)__pAd, __wdev);
#endif /*CONFIG_STA_SUPPORT*/
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
#define RT_CFG80211_LOST_GO_INFORM(__pAd)									\
	CFG80211_LostP2pGoInform((VOID *)__pAd);
#endif /*RT_CFG80211_P2P_CONCURRENT_DEVICE*/
#define RT_CFG80211_REINIT(__pAd, __wdev)											\
	CFG80211_SupBandReInit((VOID *)__pAd, (VOID *)__wdev);

#define RT_CFG80211_RFKILL_STATUS_UPDATE(_pAd, _active)					\
	CFG80211_RFKillStatusUpdate(_pAd, _active);

#ifdef APCLI_CFG80211_SUPPORT
#define RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(__pAd, __pBSSID, __ifIndex, __pReqIe,   \
					__ReqIeLen, __pRspIe, __RspIeLen, __FlgIsSuccess)	\
		CFG80211_ApClientConnectResultInform(__pAd, __pBSSID, __ifIndex,				    \
			__pReqIe, __ReqIeLen, __pRspIe, __RspIeLen, __FlgIsSuccess);
#else
#define RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(__pAd, __pBSSID, __pReqIe,   \
		__ReqIeLen,	__pRspIe, __RspIeLen, __FlgIsSuccess)				\
CFG80211_P2pClientConnectResultInform(__pAd, __pBSSID,				    \
									  __pReqIe, __ReqIeLen, __pRspIe, __RspIeLen, __FlgIsSuccess);
#endif /* APCLI_CFG80211_SUPPORT */

#ifdef SUPP_SAE_SUPPORT
int mtk_cfg80211_event_connect_params(void *pAd, UCHAR *pmk, int pmk_len);
int mtk_cfg80211_event_pmksa(void *pAd, UCHAR *pmk, int pmk_len, UCHAR *pmkid,
				UINT32 akmp, UINT8 *aa);
#endif

#define RT_CFG80211_P2P_CLI_SEND_NULL_FRAME(_pAd, _PwrMgmt)					\
	CFG80211_P2pClientSendNullFrame(_pAd, _PwrMgmt);

#define RT_CFG80211_JOIN_IBSS(_pAd, _pBssid) \
	CFG80211_JoinIBSS(_pAd, _pBssid);


#ifdef SINGLE_SKU
#define CFG80211_BANDINFO_FILL(__pAd, __wdev, __pBandInfo)\
	{\
		do {\
			(__pBandInfo)->RFICType = HcGetRadioRfIC(__pAd);\
			(__pBandInfo)->MpduDensity = __pAd->CommonCfg.BACapability.field.MpduDensity;\
			(__pBandInfo)->TxStream = ((__wdev == NULL) || (__wdev->wpf_op == NULL)) ? hc_get_chip_cap(__pAd->hdev_ctrl)->mcs_nss.max_nss : wlan_operate_get_tx_stream(__wdev);			\
			(__pBandInfo)->RxStream = ((__wdev == NULL) || (__wdev->wpf_op == NULL)) ? hc_get_chip_cap(__pAd->hdev_ctrl)->mcs_nss.max_nss : wlan_operate_get_rx_stream(__wdev);			\
			(__pBandInfo)->MaxTxPwr = __pAd->CommonCfg.DefineMaxTxPwr;\
			if (WMODE_EQUAL(HcGetRadioPhyMode(__pAd), WMODE_B))\
				(__pBandInfo)->FlgIsBMode = TRUE;\
			else\
				(__pBandInfo)->FlgIsBMode = FALSE;\
			(__pBandInfo)->MaxBssTable = MAX_LEN_OF_BSS_TABLE;\
			(__pBandInfo)->RtsThreshold = ((__wdev == NULL) || (__wdev->wpf_op == NULL)) ? DEFAULT_RTS_LEN_THLD : wlan_operate_get_rts_len_thld(__wdev);\
			(__pBandInfo)->FragmentThreshold = ((__wdev == NULL) || (__wdev->wpf_op == NULL)) ? DEFAULT_FRAG_THLD : wlan_operate_get_frag_thld(__wdev);\
			(__pBandInfo)->RetryMaxCnt = 0;	\
		} while (0);\
	}
#else
#define CFG80211_BANDINFO_FILL(__pAd, __wdev, __pBandInfo)\
	{\
		do {\
			(__pBandInfo)->RFICType = HcGetRadioRfIC(__pAd);\
			(__pBandInfo)->MpduDensity = __pAd->CommonCfg.BACapability.field.MpduDensity;\
			(__pBandInfo)->TxStream = ((__wdev == NULL) || (__wdev->wpf_op == NULL)) ? hc_get_chip_cap(__pAd->hdev_ctrl)->mcs_nss.max_nss[0] : wlan_operate_get_tx_stream(__wdev);			\
			(__pBandInfo)->RxStream = ((__wdev == NULL) || (__wdev->wpf_op == NULL)) ? hc_get_chip_cap(__pAd->hdev_ctrl)->mcs_nss.max_nss[0] : wlan_operate_get_rx_stream(__wdev);			\
			(__pBandInfo)->MaxTxPwr = 0;\
			if (WMODE_EQUAL(HcGetRadioPhyMode(__pAd), WMODE_B))\
				(__pBandInfo)->FlgIsBMode = TRUE;\
			else\
				(__pBandInfo)->FlgIsBMode = FALSE;\
			(__pBandInfo)->MaxBssTable = MAX_LEN_OF_BSS_TABLE;\
			(__pBandInfo)->RtsThreshold = ((__wdev == NULL) || (__wdev->wpf_op == NULL)) ? DEFAULT_RTS_LEN_THLD : wlan_operate_get_rts_len_thld(__wdev);\
			(__pBandInfo)->FragmentThreshold = ((__wdev == NULL) || (__wdev->wpf_op == NULL)) ? DEFAULT_FRAG_THLD : wlan_operate_get_frag_thld(__wdev);\
			(__pBandInfo)->RetryMaxCnt = 0; \
		} while (0); \
	}
#endif /* SINGLE_SKU */

/* NoA Command Parm */
#define P2P_NOA_DISABLED 0x00
#define P2P_NOA_TX_ON    0x01
#define P2P_NOA_RX_ON    0x02

#define WLAN_AKM_SUITE_OWE			0x000FAC12
#define WDEV_NOT_FOUND				-1


/* Scan Releated */
#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
BOOLEAN CFG80211DRV_OpsScanRunning(VOID *pAdOrg);
#endif /*CONFIG_STA_SUPPORT*/

BOOLEAN CFG80211DRV_OpsScanSetSpecifyChannel(
	VOID *pAdOrg, VOID *pData, UINT8 dataLen);

BOOLEAN CFG80211DRV_OpsScanCheckStatus(
	VOID *pAdOrg, UINT8	IfType);

BOOLEAN CFG80211DRV_OpsScanExtraIesSet(VOID *pAdOrg);

VOID CFG80211DRV_OpsScanInLinkDownAction(VOID *pAdOrg);

#ifdef APCLI_CFG80211_SUPPORT
VOID CFG80211DRV_ApcliSiteSurvey(VOID *pAdOrg, VOID *pData);

VOID CFG80211DRV_SetApCliAssocIe(VOID *pAdOrg, PNET_DEV pNetDev, VOID *pData, UINT ie_len);

VOID CFG80211DRV_ApClientKeyAdd(VOID *pAdOrg, VOID *pData);

#endif /* APCLI_CFG80211_SUPPORT */

#ifdef CONFIG_MULTI_CHANNEL
VOID CFG80211DRV_Set_NOA(VOID *pAdOrg, VOID *pData);
#endif /* CONFIG_MULTI_CHANNEL */

INT CFG80211DRV_OpsScanGetNextChannel(VOID *pAdOrg);

VOID CFG80211_ScanStatusLockInit(VOID *pAdCB, UINT init);

VOID CFG80211_Scaning(VOID *pAdCB, UINT32 BssIdx, UINT32 ChanId,
					  UCHAR *pFrame, UINT32 FrameLen, INT32 RSSI);

VOID CFG80211_ScanEnd(VOID *pAdCB, BOOLEAN FlgIsAborted);

/* Connect Releated */
BOOLEAN CFG80211DRV_OpsJoinIbss(VOID *pAdOrg, VOID *pData);
BOOLEAN CFG80211DRV_OpsLeave(VOID *pAdOrg, PNET_DEV pNetDev);
BOOLEAN CFG80211DRV_Connect(VOID *pAdOrg, VOID *pData);
VOID CFG80211_P2pClientConnectResultInform(
	IN VOID                                         *pAdCB,
	IN UCHAR                                        *pBSSID,
	IN UCHAR                                        *pReqIe,
	IN UINT32                                       ReqIeLen,
	IN UCHAR                                        *pRspIe,
	IN UINT32                                       RspIeLen,
	IN UCHAR                                        FlgIsSuccess);

#ifdef APCLI_CFG80211_SUPPORT
VOID CFG80211_ApClientConnectResultInform(
	IN VOID						*pAdCB,
	IN UCHAR					*pBSSID,
	IN UCHAR					ifIndex,
	IN UCHAR					*pReqIe,
	IN UINT32					ReqIeLen,
	IN UCHAR					*pRspIe,
	IN UINT32					RspIeLen,
	IN UCHAR					FlgIsSuccess);
#endif /* APCLI_CFG80211_SUPPORT */

VOID CFG80211_ConnectResultInform(
	VOID *pAdCB, UCHAR *pBSSID,	UCHAR *pReqIe, UINT32 ReqIeLen,
	UCHAR *pRspIe, UINT32 RspIeLen,	UCHAR FlgIsSuccess);
VOID CFG80211DRV_PmkidConfig(VOID *pAdOrg, VOID *pData);
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
VOID CFG80211_LostP2pGoInform(VOID *pAdCB);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
VOID CFG80211_LostApInform(VOID *pAdCB, struct wifi_dev	*wdev);

INT CFG80211_StaPortSecured(
	VOID                         *pAdCB,
	UCHAR                        *pMac,
	UINT						flag);

/* AP Related*/
#ifdef HOSTAPD_MAP_SUPPORT /* This could be a generic fix*/
INT CFG80211_ApStaDel(VOID *pAdCB, VOID *pData, UINT reason);
#else
INT CFG80211_ApStaDel(VOID *pAdCB, UCHAR *pMac, UINT reason);
#endif /* HOSTAPD_MAP_SUPPORT */

#ifdef HOSTAPD_PMKID_IN_DRIVER_SUPPORT
INT CFG80211_ApUpdateStaPmkid(VOID *pAdCB, VOID *pData);
#endif /*HOSTAPD_PMKID_IN_DRIVER_SUPPORT*/

VOID CFG80211_UpdateBeacon(
	VOID                           *pAdOrg,
	UCHAR                          *beacon_head_buf,
	UINT32                          beacon_head_len,
	UCHAR                          *beacon_tail_buf,
	UINT32                          beacon_tail_len,
	BOOLEAN                         isAllUpdate,
	UINT32							apidx);


INT CFG80211_ApStaDelSendEvent(PRTMP_ADAPTER pAd, const PUCHAR mac_addr, IN PNET_DEV pNetDevIn);
INT CFG80211_FindMbssApIdxByNetDevice(RTMP_ADAPTER *pAd, PNET_DEV pNetDev);

#ifdef APCLI_CFG80211_SUPPORT
INT CFG80211_FindStaIdxByNetDevice(RTMP_ADAPTER *pAd, PNET_DEV pNetDev);
#endif

/* Information Releated */
BOOLEAN CFG80211DRV_StaGet(
	VOID						*pAdOrg,
	VOID						*pData);

#ifdef WIFI_IAP_STA_DUMP_FEATURE
/* Information Releated */
BOOLEAN CFG80211DRV_Ap_StaGet(
	VOID *pAdOrg,
	VOID *pData);
#endif/*WIFI_IAP_STA_DUMP_FEATURE*/

#ifdef WIFI_IAP_POWER_SAVE_FEATURE
INT CFG80211DRV_AP_SetPowerMgmt (
	VOID * pAdOrg,
	VOID * infwdev,
	UINT enable);
#endif/*WIFI_IAP_POWER_SAVE_FEATURE*/

VOID CFG80211DRV_SurveyGet(
	VOID						*pAdOrg,
	VOID						*pData);

INT CFG80211_reSetToDefault(
	VOID						*pAdCB);


/* Key Releated */
BOOLEAN CFG80211DRV_StaKeyAdd(
	VOID						*pAdOrg,
	VOID						*pData);

BOOLEAN CFG80211DRV_ApKeyAdd(
	VOID                    *pAdOrg,
	VOID                    *pData);

VOID CFG80211DRV_RtsThresholdAdd(
	VOID                                            *pAdOrg,
	struct wifi_dev	*wdev,
	UINT                                            threshold);

VOID CFG80211DRV_FragThresholdAdd(
	VOID                                            *pAdOrg,
	struct wifi_dev	*wdev,
	UINT                                            threshold);

#ifdef ACK_CTS_TIMEOUT_SUPPORT
BOOLEAN CFG80211DRV_AckThresholdAdd(
	VOID * pAdOrg,
	struct wifi_dev	*wdev,
	UINT threshold);
#endif/*ACK_CTS_TIMEOUT_SUPPORT*/
BOOLEAN CFG80211DRV_ApKeyDel(
	VOID						*pAdOrg,
	VOID						*pData);

INT CFG80211_setApDefaultKey(
		IN VOID 				   *pAdCB,
		IN struct net_device		*pNetdev,
		IN UINT 					Data);

#ifdef DOT11W_PMF_SUPPORT
INT CFG80211_setApDefaultMgmtKey(
		IN VOID				*pAdCB,
		IN struct net_device		*pNetdev,
		IN UINT 			Data);
#endif /*DOT11W_PMF_SUPPORT*/


#ifdef CONFIG_STA_SUPPORT
INT CFG80211_setStaDefaultKey(PVOID pAdCB, struct net_device *pNetdev, UINT Data);
#else
INT CFG80211_setStaDefaultKey(PVOID pAdCB, UINT	Data);
#endif /*CONFIG_STA_SUPPORT*/

INT CFG80211_setPowerMgmt(
	VOID                     *pAdCB,
	UINT			Enable);

/* General Releated */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
#ifdef WIFI_IAP_IW_SET_CHANNEL_FEATURE
BOOLEAN CFG80211DRV_AP_OpsSetChannel(RTMP_ADAPTER *pAd, VOID *pData);
#endif/*WIFI_IAP_IW_SET_CHANNEL_FEATURE*/
#endif/*KERNEL_VERSION(4, 4, 0)*/

BOOLEAN CFG80211DRV_OpsSetChannel(RTMP_ADAPTER *pAd, VOID *pData);

BOOLEAN CFG80211DRV_OpsChgVirtualInf(RTMP_ADAPTER *pAd, VOID *pData);

VOID CFG80211DRV_OpsChangeBssParm(VOID *pAdOrg, VOID *pData);

VOID CFG80211DRV_OpsBitRateParm(VOID *pAdOrg, VOID *pData, INT apidx);

VOID CFG80211_UnRegister(VOID *pAdOrg,	VOID *pNetDev);

INT CFG80211DRV_IoctlHandle(
	VOID						*pAdSrc,
	RTMP_IOCTL_INPUT_STRUCT		*wrq,
	INT							cmd,
	USHORT						subcmd,
	VOID						*pData,
	ULONG						Data);

UCHAR CFG80211_getCenCh(RTMP_ADAPTER *pAd, UCHAR prim_ch);

/* CRDA Releatd */
VOID CFG80211DRV_RegNotify(
	VOID						*pAdOrg,
	VOID						*pData);

VOID CFG80211_RegHint(
	VOID						*pAdCB,
	UCHAR						*pCountryIe,
	ULONG						CountryIeLen);

VOID CFG80211_RegHint11D(
	VOID						*pAdCB,
	UCHAR						*pCountryIe,
	ULONG						CountryIeLen);

VOID CFG80211_RegRuleApply(
	VOID						*pAdCB,
	VOID						*pWiphy,
	UCHAR						*pAlpha2);

BOOLEAN CFG80211_SupBandReInit(
	VOID						*pAdCB,
	VOID	*wdev);

#ifdef RFKILL_HW_SUPPORT
VOID CFG80211_RFKillStatusUpdate(
	PVOID						pAd,
	BOOLEAN						active);
#endif /* RFKILL_HW_SUPPORT */

/* P2P Related */
VOID CFG80211DRV_SetP2pCliAssocIe(
	VOID						*pAdOrg,
	VOID						*pData,
	UINT                         ie_len);

VOID CFG80211DRV_P2pClientKeyAdd(
	VOID						*pAdOrg,
	VOID						*pData);

BOOLEAN CFG80211DRV_P2pClientConnect(
	VOID						*pAdOrg,
	VOID						*pData);

BOOLEAN CFG80211_checkScanTable(
	IN VOID                      *pAdCB);

VOID CFG80211_P2pClientSendNullFrame(
	VOID						*pAdCB,
	INT							 PwrMgmt);

VOID CFG80211RemainOnChannelTimeout(
	PVOID						SystemSpecific1,
	PVOID						FunctionContext,
	PVOID						SystemSpecific2,
	PVOID						SystemSpecific3);

BOOLEAN CFG80211DRV_OpsRemainOnChannel(
	VOID						*pAdOrg,
	VOID						*pData,
	UINT32						duration);

BOOLEAN CFG80211DRV_OpsCancelRemainOnChannel(
	VOID                                            *pAdOrg,
	UINT32                                          cookie);


VOID CFG80211DRV_OpsMgmtFrameProbeRegister(
	VOID                                            *pAdOrg,
	VOID                                            *pData,
	BOOLEAN                                          isReg);

VOID CFG80211DRV_OpsMgmtFrameActionRegister(
	VOID                                            *pAdOrg,
	VOID                                            *pData,
	BOOLEAN                                          isReg);

BOOLEAN CFG80211_CheckActionFrameType(
	IN  RTMP_ADAPTER								 *pAd,
	IN	PUCHAR										 preStr,
	IN	PUCHAR										 pData,
	IN	UINT32										length);

BOOLEAN CFG80211_SyncPacketWmmIe(RTMP_ADAPTER *pAd, VOID *pData, ULONG dataLen);
BOOLEAN CFG80211_HandleP2pMgmtFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR OpMode);
INT CFG80211_SendMgmtFrame(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data);

#ifdef RT_CFG80211_P2P_SUPPORT /* yiwei 7.1.3 */
VOID CFG80211_PeerP2pBeacon(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	pAddr2,
	IN MLME_QUEUE_ELEM * Elem,
	IN LARGE_INTEGER   TimeStamp);


VOID CFG80211_P2pStopNoA(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY	pMacClient);


BOOLEAN CFG80211_P2pResetNoATimer(
	IN PRTMP_ADAPTER pAd,
	IN	ULONG	DiffTimeInus);


BOOLEAN CFG80211_P2pHandleNoAAttri(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY	pMacClient,
	IN PUCHAR pData);

#endif /* RT_CFG80211_P2P_SUPPORT */


/* -------------------------------- */
/* VOID CFG80211_Convert802_3Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR *pHeader802_3); */
VOID CFG80211_Announce802_3Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR FromWhichBSSID);
VOID CFG80211_SendMgmtFrameDone(RTMP_ADAPTER *pAd, USHORT Sequence, BOOLEAN ack);
#ifdef CONFIG_AP_SUPPORT
VOID CFG80211_ParseBeaconIE(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, struct wifi_dev *wdev, UCHAR *wpa_ie, UCHAR *rsn_ie);
#endif /*CONFIG_AP_SUPPORT*/
VOID CFG80211_SwitchTxChannel(RTMP_ADAPTER *pAd, ULONG Data);

BOOLEAN CFG80211DRV_OpsBeaconSet(
	VOID                                            *pAdOrg,
	VOID                                            *pData);

BOOLEAN CFG80211DRV_OpsBeaconAdd(
	VOID                                            *pAdOrg,
	VOID                                            *pData);

#ifdef HOSTAPD_HS_R2_SUPPORT
BOOLEAN CFG80211DRV_SetQosParam(
		VOID 					*pAdOrg,
		VOID 					*pData,
		INT 					apindex);
#endif

VOID CFG80211DRV_DisableApInterface(PRTMP_ADAPTER pAd);

BOOLEAN CFG80211DRV_OpsVifAdd(VOID *pAdOrg, VOID *pData);

#ifdef CFG_TDLS_SUPPORT
BOOLEAN CFG80211DRV_StaTdlsInsertDeletepEntry(
	VOID						*pAdOrg,
	VOID						*pData,
	UINT						Data);
BOOLEAN CFG80211DRV_StaTdlsSetKeyCopyFlag(
	VOID						*pAdOrg);
BOOLEAN CFG80211_HandleTdlsDiscoverRespFrame(
	RTMP_ADAPTER				*pAd,
	RX_BLK						*pRxBlk,
	UCHAR						OpMode);

VOID cfg_tdls_send_PeerTrafficIndication(PRTMP_ADAPTER pAd, u8 *peer);
VOID cfg_tdls_rcv_PeerTrafficIndication(PRTMP_ADAPTER pAd, u8 dialog_token, u8 *peer);
VOID cfg_tdls_rcv_PeerTrafficResponse(PRTMP_ADAPTER pAd, u8 *peer);
INT cfg_tdls_search_wcid(PRTMP_ADAPTER pAd, u8 *peer);
INT cfg_tdls_search_ValidLinkIndex(PRTMP_ADAPTER pAd, u8 *peer);
INT cfg_tdls_build_frame(PRTMP_ADAPTER	pAd, u8 *peer, u8 dialog_token, u8 action_code, u16 status_code
						 , const u8 *extra_ies, size_t extra_ies_len, BOOLEAN send_by_tdls_link, u8 tdls_entry_wcid, u8 reason_code);
VOID cfg_tdls_UAPSDP_PsmModeChange(PRTMP_ADAPTER pAd, USHORT	PsmOld, USHORT PsmNew);
BOOLEAN cfg_tdls_UAPSDP_AsicCanSleep(PRTMP_ADAPTER	pAd);
INT cfg_tdls_EntryInfo_Display_Proc(PRTMP_ADAPTER pAd, PUCHAR arg);
VOID cfg_tdls_TimerInit(PRTMP_ADAPTER pAd);
VOID cfg_tdls_PTITimeoutAction(IN PVOID SystemSpecific1, IN PVOID FunctionContext, IN PVOID SystemSpecific2, IN PVOID SystemSpecific3);
VOID cfg_tdls_BaseChannelTimeoutAction(IN PVOID SystemSpecific1, IN PVOID FunctionContext, IN PVOID SystemSpecific2, IN PVOID SystemSpecific3);
VOID cfg_tdls_rx_parsing(PRTMP_ADAPTER pAd, RX_BLK *pRxBlk);
INT cfg_tdls_chsw_req(PRTMP_ADAPTER	pAd, u8 *peer, u8 target_channel, u8 target_bw);
INT cfg_tdls_chsw_resp(PRTMP_ADAPTER	pAd, u8 *peer, UINT32 ch_sw_time, UINT32 ch_sw_timeout, u16 reason_code);
VOID cfg_tdls_prepare_null_frame(PRTMP_ADAPTER	pAd, BOOLEAN powersave, UCHAR dir, UCHAR *peerAddr);
VOID cfg_tdls_TunneledProbeRequest(PRTMP_ADAPTER pAd, PUCHAR pMacAddr, const u8  *extra_ies,	size_t extra_ies_len);
VOID cfg_tdls_TunneledProbeResponse(PRTMP_ADAPTER pAd, PUCHAR pMacAddr, const u8  *extra_ies,	size_t extra_ies_len);
VOID cfg_tdls_auto_teardown(PRTMP_ADAPTER pAd, PMAC_TABLE_ENTRY pEntry);
INT cfg_tdls_send_CH_SW_SETUP(RTMP_ADAPTER *ad, UCHAR cmd, UCHAR offch_prim, UCHAR offch_center, UCHAR bw_off, UCHAR role, UINT16 stay_time, UINT32 start_time_tsf, UINT16 switch_time,	UINT16 switch_timeout);



#endif /*CFG_TDLS_SUPPORT*/
#endif /* RT_CFG80211_SUPPORT */

VOID CFG80211_JoinIBSS(
	IN VOID						*pAdCB,
	IN UCHAR					*pBSSID);

#ifdef MT_MAC
VOID CFG80211_InitTxSCallBack(RTMP_ADAPTER *pAd);
#endif /* MT_MAC */

#ifdef CONFIG_VLAN_GTK_SUPPORT
struct vlan_gtk_info {
	PNET_DEV vlan_dev;           /* struct net_dev for vlan device */
	UINT16 vlan_id;              /* parse from interface name */
	UINT16 vlan_bmc_idx;         /* should be identical to vlan_tr_tb_idx */
	UINT16 vlan_tr_tb_idx;       /* should be identical to vlan_bmc_idx */
	UCHAR vlan_gtk[LEN_MAX_GTK]; /* only for debug print */
	UINT8 gtk_len;
	struct list_head list;
};

struct wifi_dev *CFG80211_GetWdevByVlandev(PRTMP_ADAPTER pAd, PNET_DEV net_dev);
BOOLEAN CFG80211_MatchVlandev(struct wifi_dev *wdev, PNET_DEV net_dev);
struct vlan_gtk_info *CFG80211_GetVlanInfoByVlandev(struct wifi_dev *wdev, PNET_DEV net_dev);
struct vlan_gtk_info *CFG80211_GetVlanInfoByVlanid(struct wifi_dev *wdev, UINT16 vlan_id);
INT16 CFG80211_IsVlanPkt(PNDIS_PACKET pkt);
#endif
#endif /* __CFG80211EXTR_H__ */

