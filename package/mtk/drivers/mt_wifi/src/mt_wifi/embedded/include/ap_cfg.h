#ifndef __AP_CFG_H__
#define __AP_CFG_H__

INT RTMPAPPrivIoctlSet(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * pIoctlCmdStr);

INT RTMPAPPrivIoctlShow(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * pIoctlCmdStr);

#ifdef VENDOR_FEATURE6_SUPPORT
VOID RTMPAPGetAssoMacTable(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * pIoctlCmdStr);
#endif /* VENDOR_FEATURE6_SUPPORT */

#if defined(INF_AR9) || defined(BB_SOC)
#if defined(AR9_MAPI_SUPPORT) || defined(BB_SOC)
INT RTMPAPPrivIoctlAR9Show(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * pIoctlCmdStr);

VOID RTMPAR9IoctlGetMacTable(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetSTAT2(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetRadioDynInfo(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /*AR9_MAPI_SUPPORT*/
#endif/* INF_AR9 */

INT RTMPAPSetInformation(
	IN	PRTMP_ADAPTER	pAd,
	IN	OUT	RTMP_IOCTL_INPUT_STRUCT * rq,
	IN	INT				cmd);

INT RTMPAPQueryInformation(
	IN	PRTMP_ADAPTER       pAd,
	IN	OUT	RTMP_IOCTL_INPUT_STRUCT * rq,
	IN	INT                 cmd);

VOID RTMPIoctlStatistics(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

INT RTMPIoctlRXStatistics(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetMacTableStaInfo(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetMacTable(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetDriverInfo(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPAPIoctlE2PROM(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_IOCTL_INPUT_STRUCT * wrq);

INT RTMPPhyState(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq);

#if defined(DBG) || (defined(BB_SOC) && defined(CONFIG_ATE))
VOID RTMPAPIoctlBBP(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_IOCTL_INPUT_STRUCT * wrq);

#ifdef RTMP_RF_RW_SUPPORT
VOID RTMPAPIoctlRF(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /* RTMP_RF_RW_SUPPORT */

#endif /* DBG */

VOID RtmpDrvMaxRateGet(
	IN	VOID					*pReserved,
	/*	IN	PHTTRANSMIT_SETTING		pHtPhyMode, */
	IN	UINT8					MODE,
	IN	UINT8					ShortGI,
	IN	UINT8					BW,
	IN	UINT8					MCS,
	IN	UINT8					Antenna,
	OUT	UINT64 *pRate);

#ifdef WSC_AP_SUPPORT
VOID RTMPGetCurrentCred(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);
VOID RTMPIoctlWscProfile(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlWscProfile(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);
/*add by woody */
#if defined(INF_AR9) || defined(BB_SOC)
#if defined(AR9_MAPI_SUPPORT) || defined(BB_SOC)
VOID RTMPAR9IoctlWscProfile(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlWscPINCode(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);


VOID RTMPIoctlWscStatus(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetWscDynInfo(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlGetWscRegsDynInfo(
	IN PRTMP_ADAPTER pAdapter,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif/*AR9_MAPI_SUPPORT*/
#endif/* INF_AR9 */
#endif /* WSC_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
VOID RTMPIoctlQueryBaTable(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /* DOT11_N_SUPPORT */

#ifdef DOT1X_SUPPORT
VOID RTMPIoctlAddPMKIDCache(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlSetIdleTimeout(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlQueryStaAid(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlQueryStaRsn(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);


#ifdef RADIUS_ACCOUNTING_SUPPORT
VOID RTMPIoctlQueryStaData(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /* RADIUS_ACCOUNTING_SUPPORT */

#ifdef RADIUS_MAC_ACL_SUPPORT
PRT_802_11_RADIUS_ACL_ENTRY RadiusFindAclEntry(
	PLIST_HEADER            pCacheList,
	IN      PUCHAR          pMacAddr);

VOID RTMPIoctlAddRadiusMacAuthCache(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlDelRadiusMacAuthCache(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlClearRadiusMacAuthCache(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* DOT1X_SUPPORT */

INT Set_AP_Daemon_Status(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 WorkSpaceID,
	IN BOOLEAN Status);

INT Send_ANQP_Req_For_Test(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);

INT Send_ANQP_Rsp(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * PeerMACAddr,
	IN RTMP_STRING * ANQPReq,
	IN UINT32 ANQPReqLen);

INT	ApCfg_Set_PerMbssMaxStaNum_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN INT				apidx,
	IN RTMP_STRING * arg);

INT	set_bss_max_idle_period_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

struct apcfg_parameters {
	LONG cfg_mode[2]; /*WirelessMode*/
	ULONG tx_power_percentage; /*TxPower*/
	ULONG tx_preamble; /*TxPreamble*/
	UINT32 conf_len_thld; /*RTSThreshold*/
	UINT32 oper_len_thld;
	UINT32 conf_frag_thld; /*FragThreshold*/
	UINT32 oper_frag_thld;
	BOOLEAN bEnableTxBurst;	/*TxBurst*/
	BOOLEAN bUseShortSlotTime;	/*ShortSlot*/
#ifdef DOT11_N_SUPPORT
	UCHAR conf_ht_bw; /*HT_BW*/
	UCHAR oper_ht_bw;
#ifdef DOT11N_DRAFT3
	BOOLEAN bBssCoexEnable; /*HT_BSSCoexistence*/
#endif
	UCHAR ht_tx_streams; /*HT_TxStream*/
	UCHAR ht_rx_streams; /*HT_RxStream*/
	UINT8 ba_decline; /*HT_BADecline*/
	UINT8 ba_en; /*HT_AutoBA*/
	UINT32 AmsduEnable; /*HT_AMSDU*/
	UINT32 ba_rx_wsize; /*HT_BAWinSize*/
	UINT32 ba_tx_wsize; /*HT_BAWinSize*/
	UCHAR ht_gi; /*HT_GI*/
	UCHAR ht_stbc; /*HT_STBC*/
	UCHAR ht_ldpc; /*HT_LDPC*/
	BOOLEAN bRdg; /*HT_RDG*/
#endif

	BOOLEAN HT_DisallowTKIP; /*HT_DisallowTKIP*/

#ifdef DOT11_VHT_AC
	UCHAR conf_vht_bw; /*VHT_BW*/
	UCHAR oper_vht_bw;
	UCHAR vht_sgi; /*VHT_SGI*/
	UCHAR vht_stbc; /*VHT_STBC*/
	UCHAR vht_bw_signal; /*VHT_BW_SIGNAL*/
	UCHAR vht_ldpc; /*VHT_LDPC*/
	BOOLEAN g_band_256_qam; /*G_BAND_256QAM*/
#endif

	BOOLEAN bIEEE80211H; /*IEEE80211H*/

#ifdef MT_DFS_SUPPORT
	BOOLEAN bDfsEnable; /*DfsEnable*/
#endif

#ifdef BACKGROUND_SCAN_SUPPORT
	BOOLEAN DfsZeroWaitSupport; /*DfsZeroWait*/
#endif

	UCHAR MuOfdmaDlEnable;
	UCHAR MuOfdmaUlEnable;
	UCHAR MuMimoDlEnable;
	UCHAR MuMimoUlEnable;

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	ULONG ETxBfEnCond; /*ETxBfEnCond*/
#endif
#endif

	UINT32 ITxBfEn; /*ITxBfEn*/

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	ULONG MUTxRxEnable; /*MUTxRxEnable*/
#endif
#endif
	UCHAR channel;
	UCHAR CentralChannel;
	UCHAR ext_channel;

#ifdef CFG_SUPPORT_FALCON_MURU
	ULONG TamArbOpMode;
	ULONG HE_PpduFmt;
	ULONG HE_OfdmaUserNum;
#endif
	BOOLEAN vht_1024_qam;
	UCHAR HeOmiUlMuDataDisableRx;
	UCHAR HeErSuRxDisable;
};
INT Set_Quick_Channel_Switch_En_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef CONFIG_AP_SUPPORT
void ap_phy_rrm_init_byRf(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#endif
VOID MacTableResetNonMapWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#ifdef CONFIG_MAP_SUPPORT
INT Set_Bh_Bss_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
INT Set_Fh_Bss_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
INT Set_Map_Channel_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
INT Set_Map_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
#ifdef MAP_R2
INT Set_Map_Bh_Primary_Vid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Map_Bh_Primary_Pcp_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Map_Bh_Vid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Map_Fh_Vid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Map_Transparent_Vid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MapR2_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif
#ifdef MAP_R3
INT Set_MapR3_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif
#ifdef MAP_R4
INT Set_MapR4_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif

#ifdef MAP_TS_TRAFFIC_SUPPORT
INT Set_MapTS_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif
#ifdef MAP_BL_SUPPORT
INT Set_BlackList_Add(
	PRTMP_ADAPTER pAd,
	char *arg);
INT Set_BlackList_Del(
	PRTMP_ADAPTER pAd,
	char *arg);
INT Set_BlackList_Show(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif /*  MAP_BL_SUPPORT */
#endif /* CONFIG_MAP_SUPPORT */
#ifdef DPP_SUPPORT
INT Set_Enable_Dpp_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif /* DPP_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT
VOID RTMPApCliAddKey(
	IN	PRTMP_ADAPTER	    pAd,
	IN	INT				apidx,
	IN	PNDIS_APCLI_802_11_KEY    pApcliKey);
#endif /* WPA_SUPPLICANT_SUPPORT */
#endif /* APCLI_SUPPORT */

INT set_qiscdump_proc(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
INT set_ch_switch_monitor_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT cancel_ch_switch_monitor_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_ch_switch_monitor_cfg(IN PRTMP_ADAPTER pAd, struct ch_switch_cfg *ch_sw_cfg);
VOID ch_switch_monitor_state_machine_init(struct _RTMP_ADAPTER *pAd);
VOID ch_switch_monitor_exit(struct _RTMP_ADAPTER *pAd);
VOID ch_switch_monitor_timeout(IN PVOID system_specific1, IN PVOID function_context,
			IN PVOID system_specific2, IN PVOID system_specific3);
extern INT ch_switch_monitor_scan_ch_restore(RTMP_ADAPTER *pAd, UCHAR OpMode, struct wifi_dev *pwdev);
INT ch_switch_monitor_cancel(IN PRTMP_ADAPTER pAd, struct wifi_dev *pwdev);
#endif

#if defined(DOT11_HE_AX) && defined(FIXED_HE_GI_SUPPORT)
INT set_fgi_and_ltf_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_fgi_and_ltf_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_fgi_and_ltf_profile(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 value);
#endif

#ifdef OCE_FILS_SUPPORT
VOID RTMPIoctlStaMlmeEvent(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlRsneSyncEvent(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlKeyEvent(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq);

VOID RTMPIoctlPmkCacheEvent(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT *wrq);

#endif /* OCE_FILS_SUPPORT */
#ifdef CFG_SUPPORT_CSI
INT Set_CSI_Ctrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#ifdef DPP_SUPPORT
INT wext_send_dpp_cached_frame(struct wifi_dev *wdev, UINT32 frm_id,
				RTMP_IOCTL_INPUT_STRUCT *wrq);
INT mtk_set_pmk(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, struct pmk_req *pmk_data);
#endif /*DPP_SUPPORT*/

#ifdef CONFIG_ICS_FRAME_HANDLE
INT set_rxv_ics_proc(PRTMP_ADAPTER pAd, char *arg);
#endif

struct radio_dev;
INT32 HcUpdateMSDUTxAllow(struct radio_dev *rdev);

#endif /* __AP_CFG_H__ */
BOOLEAN wdev_down_exec_ioctl(RTMP_IOCTL_INPUT_STRUCT *wrq, USHORT subcmd);
