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
    sta_ioctl.c

    Abstract:
    IOCTL related subroutines

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/

#include	"rt_config.h"

#ifdef CFG_SUPPORT_MU_MIMO
INT SetMuStaParamProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

#define BSSID_WCID_TO_REMOVE 1 /* Pat:TODO */

#ifdef P2P_SUPPORT
extern INT Set_P2p_OpMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

extern INT Set_AP_WscPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#endif /* P2P_SUPPORT */


INT Set_AutoReconnect_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AdhocN_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef CONFIG_MULTI_CHANNEL
INT Set_CH1StayTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_CH2StayTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_SwitchIdleTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_NULL_Frame_Count_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_CH1_BW_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_CH2_BW_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_CH1_Channel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_CH2_Channel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_MCC_Start_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_Pkt_Dbg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Show_CH1StayTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Show_CH2StayTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Show_SwitchIdleTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Show_NULL_Frame_Count_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CONFIG_MULTI_CHANNEL */

INT show_timer_list(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT show_wtbl_state(RTMP_ADAPTER *pAd, RTMP_STRING *arg);


#ifdef WIDI_SUPPORT
INT Set_WiDiEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* WIDI_SUPPORT */

#ifdef RTMP_RBUS_SUPPORT
#ifdef LED_CONTROL_SUPPORT
INT Set_WlanLed_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* LED_CONTROL_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */


#ifdef CARRIER_DETECTION_SUPPORT
INT Set_StaCarrierDetect_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef IWSC_SUPPORT
INT	Set_IWscLimitedUI_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef IWSC_TEST_SUPPORT
INT	Set_IWscDefaultSecurity_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_IWscSmpbcEnrScanOnly_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_IWscEmptySubmaskList_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_IWscBlockConnection_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* IWSC_TEST_SUPPORT */

INT	Set_IWscOOB_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_IWscSinglePIN_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* IWSC_SUPPORT */

#ifdef REDUCE_TCP_ACK_SUPPORT
INT Set_ReduceAckEnable_Proc(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_STRING     *pParam);

INT Show_ReduceAckInfo_Proc(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_STRING     *pParam);


INT Set_ReduceAckProb_Proc(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_STRING     *pParam);
#endif

INT show_radio_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef SNIFFER_SUPPORT
INT set_monitor_channel(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT bw = 0, channel = 0, pri_idx = 0, rv = 0;
	/* UINT rf_val = 0; */
	UINT8 ext_ch = 0;
	/* UCHAR BandIdx=0; */
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	/* PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if]; */
	/* struct wifi_dev *wdev = pStaCfg->wdev; */
	struct wifi_dev *wdev = &pAd->StaCfg[pObj->ioctl_if].wdev;
	UCHAR cen_ch_2 = 0;
	UCHAR vht_bw = VHT_BW_2040;
	UCHAR ht_bw = HT_BW_20;
	struct freq_cfg freq;

	os_zero_mem(&freq, sizeof(freq));

	if (arg) {
		rv = sscanf(arg, "%d,%d,%d", &(bw), &(channel), &(pri_idx));
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "rv = %d, bw = %d, channel = %d, pri_idx = %d\n", rv, bw, channel, pri_idx);
		wdev->channel = channel;
		freq.prim_ch = channel;

		if (rv == 3) {
			if (bw == BW_40) {
				if (pri_idx == 0)
					ext_ch = EXTCHA_ABOVE;
				else if (pri_idx == 1)
					ext_ch = EXTCHA_BELOW;
				else {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pri_idx(%d) is invalid\n", pri_idx);
					return FALSE;
				}
			}

#ifdef SNIFFER_MT7615
#ifdef DOT11_VHT_AC
			else if ((bw == BW_8080) && channel > 14) {
				UCHAR ch_band = wlan_config_get_ch_band(wdev);
				vht_bw = VHT_BW_8080;
				cen_ch_2 = vht_cent_ch_freq(pri_idx, VHT_BW_8080, ch_band);
			}

#endif /* DOT11_VHT_AC */
#endif /*SNIFFER_MT7615*/
		} else if (rv == 2) {
			if (bw == BW_20) {
#ifdef SNIFFER_MT7615
			} else if ((bw == BW_160) && channel > 14) {
#ifdef DOT11_VHT_AC
				vht_bw = VHT_BW_160;
#endif /* DOT11_VHT_AC */
#endif /* SNIFFER_MT7615 */
			} else if ((bw == BW_80) && channel > 14) {
#ifdef DOT11_VHT_AC
				vht_bw = VHT_BW_80;
#endif
			} else
				return FALSE;
		} else
			return FALSE;
	}

	if (bw > BW_20)
		ht_bw = HT_BW_40;

	freq.ext_cha = ext_ch;
	freq.ht_bw = ht_bw;
	freq.vht_bw = vht_bw;
	freq.cen_ch_2 = cen_ch_2;
	wlan_operate_set_phy(wdev, &freq);
	return TRUE;
}
#endif

static struct {
	RTMP_STRING *name;
	INT (*set_proc)(PRTMP_ADAPTER pAdapter, RTMP_STRING *arg);
} *PRTMP_PRIVATE_SET_PROC, RTMP_PRIVATE_SUPPORT_PROC[] = {
	{"CountryRegion",				Set_CountryRegion_Proc},
	{"CountryRegionABand",		Set_CountryRegionABand_Proc},
	{"SSID",						Set_SSID_Proc},
	{"WirelessMode",				Set_WirelessMode_Proc},
	{"TxBurst",					Set_TxBurst_Proc},
	{"TxPreamble",				Set_TxPreamble_Proc},
	{"TxPower",					Set_TxPower_Proc},
	{"pwrinfo",					chip_show_pwr_info},
	{"Channel",					Set_Channel_Proc},
	{"BGProtection",				Set_BGProtection_Proc},
	{"RTSThreshold",				Set_RTSThreshold_Proc},
	{"FragThreshold",				Set_FragThreshold_Proc},
#ifdef DOT11_N_SUPPORT
	{"BASetup",					Set_BASetup_Proc},
	{"HtBw",		                Set_HtBw_Proc},
	{"HtMcs",		                Set_HtMcs_Proc},
	{"HtGi",		                Set_HtGi_Proc},
	{"HtOpMode",		            Set_HtOpMode_Proc},
	{"HtLdpc",						Set_HtLdpc_Proc},
	{"HtStbc",					Set_HtStbc_Proc},
	{"HtExtcha",		            Set_HtExtcha_Proc},
	{"HtMpduDensity",		        Set_HtMpduDensity_Proc},
	{"HtBaWinSize",				Set_HtBaWinSize_Proc},
	{"HtRdg",					Set_HtRdg_Proc},
	{"HtAmsdu",					Set_HtAmsdu_Proc},
	{"HtAutoBa",				Set_HtAutoBa_Proc},
	{"HtBaDecline",				Set_BADecline_Proc},
	{"HtProtect",				Set_HtProtect_Proc},
	{"HtMimoPs",				Set_HtMimoPs_Proc},
	{"HtDisallowTKIP",				Set_HtDisallowTKIP_Proc},
#ifdef DOT11N_DRAFT3
	{"HtBssCoex",				Set_HT_BssCoex_Proc},
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef REDUCE_TCP_ACK_SUPPORT
	{"ReduceAckEnable",             Set_ReduceAckEnable_Proc},
	{"ReduceAckProb",               Set_ReduceAckProb_Proc},
#endif
#ifdef DOT11_VHT_AC
	{"VhtBw",					Set_VhtBw_Proc},
	{"VhtLdpc",					Set_VhtLdpc_Proc},
	{"VhtStbc",					Set_VhtStbc_Proc},
	{"VhtBwSignal",				set_VhtBwSignal_Proc},
#endif /* DOT11_VHT_AC */

#ifdef AGGREGATION_SUPPORT
	{"PktAggregate",				Set_PktAggregate_Proc},
#endif /* AGGREGATION_SUPPORT */

	{"WmmCapable",					Set_WmmCapable_Proc},
	{"UAPSDCapable",					Set_APSDCapable_Proc}, /* backward compatible with old SDK */
	{"APSDCapable",					Set_APSDCapable_Proc},
	{"APSDAC",					Set_APSDAC_Proc},
	{"MaxSPLength",					Set_MaxSPLength_Proc},

	{"IEEE80211H",					Set_IEEE80211H_Proc},
	{"NetworkType",                 Set_NetworkType_Proc},
	{"AuthMode",					Set_AuthMode_Proc},
	{"EncrypType",					Set_EncrypType_Proc},
	{"DefaultKeyID",				Set_DefaultKeyID_Proc},
	{"Key1",						Set_Key1_Proc},
	{"Key2",						Set_Key2_Proc},
	{"Key3",						Set_Key3_Proc},
	{"Key4",						Set_Key4_Proc},
	{"WPAPSK",						Set_WPAPSK_Proc},
	{"ResetCounter",				Set_ResetStatCounter_Proc},
	{"PSMode",                      Set_PSMode_Proc},
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	{"PcieAspm",					set_pcie_aspm_dym_ctrl_cap_proc},
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
#ifdef CFG_TDLS_SUPPORT
	{"TdlsChannelSwitch",					Set_CfgTdlsChannelSwitchRequest_Proc},
	{"TdlsCHSWTime",				Set_CfgTdlsChannelSwitchBaseStayTime_Proc},
	{"TdlsChannelSwitchBW",					Set_CfgTdlsChannelSwitchBW_Proc},
	{"TdlsChannelSwitchDisable",	Set_TdlsDisableChannelSwitchProc},
#endif /* CFG_TDLS_SUPPORT */
#ifdef DBG
	{"Debug",						Set_Debug_Proc},
	{"DebugCat",					Set_DebugCategory_Proc},
#endif /* DBG */
#ifdef RANDOM_PKT_GEN
	{"TxCtrl",					Set_TxCtrl_Proc},
#endif
#ifdef CSO_TEST_SUPPORT
	{"CsCtrl",					Set_CsCtrl_Proc},
#endif
#ifdef MT_MAC
	{"txvinfo",					show_txvinfo},
#endif

#ifdef TXBF_SUPPORT
	{"InvTxBfTag",					Set_InvTxBfTag_Proc},
	{"ITxBfDivCal",					Set_ITxBfDivCal_Proc},
	{"TxBfFbRptDbgInfo",			Set_TxBfFbRptDbgInfo},
	{"ITxBfEn",						Set_ITxBfEn_Proc},
	{"ETxBfEnCond",					Set_ETxBfEnCond_Proc},
	{"ETxBfCodebook",				Set_ETxBfCodebook_Proc},
	{"ETxBfCoefficient",			Set_ETxBfCoefficient_Proc},
	{"ETxBfGrouping",				Set_ETxBfGrouping_Proc},
	{"ETxBfNoncompress",			Set_ETxBfNoncompress_Proc},
	{"ETxBfIncapable",				Set_ETxBfIncapable_Proc},
#ifdef MT_MAC
	{"TxBfTxApply",                 Set_TxBfTxApply},
	{"TriggerSounding",			    Set_Trigger_Sounding_Proc},
	{"StopSounding",			    Set_Stop_Sounding_Proc},
	{"StaRecBfRead",                Set_StaRecBfRead},
#ifdef CONFIG_ATE
	{"StaRecBfUpdate",              Set_StaRecBfUpdate},
	{"StaRecBfHeUpdate",            set_txbf_he_bf_starec},
	{"StaRecCmmUpdate",             Set_StaRecCmmUpdate},
	{"BssInfoUpdate",               Set_BssInfoUpdate},
	{"DevInfoUpdate",               Set_DevInfoUpdate},
	{"ManualAssoc",                 SetATEAssocProc},
#endif /* CONFIG_ATE */
	{"TxBfPfmuMemAlloc",            Set_TxBfPfmuMemAlloc},
	{"TxBfPfmuMemRelease",          Set_TxBfPfmuMemRelease},
	{"TxBfPfmuMemAllocMapRead",     Set_TxBfPfmuMemAllocMapRead},
	{"TxBfProfileTagHelp",          Set_TxBfProfileTag_Help},
	{"TxBfProfileTagInValid",       Set_TxBfProfileTag_InValid},
	{"TxBfProfileTagPfmuIdx",       Set_TxBfProfileTag_PfmuIdx},
	{"TxBfProfileTagBfType",        Set_TxBfProfileTag_BfType},
	{"TxBfProfileTagBw",            Set_TxBfProfileTag_DBW},
	{"TxBfProfileTagSuMu",          Set_TxBfProfileTag_SuMu},
	{"TxBfProfileTagMemAlloc",      Set_TxBfProfileTag_Mem},
	{"TxBfProfileTagMatrix",        Set_TxBfProfileTag_Matrix},
	{"TxBfProfileTagRURang",        set_txbf_prof_tag_ru_range},
	{"TxBfProfileTagMobCalEn",      set_txbf_prof_tag_mob_cal_en},
	{"TxBfProfileTagSnr",           Set_TxBfProfileTag_SNR},
	{"TxBfProfileTagSmtAnt",        Set_TxBfProfileTag_SmartAnt},
	{"TxBfProfileTagSeIdx",         Set_TxBfProfileTag_SeIdx},
	{"TxBfProfileTagRmsdThrd",      Set_TxBfProfileTag_RmsdThrd},
	{"TxBfProfileTagMcsThrd",       Set_TxBfProfileTag_McsThrd},
	{"TxBfProfileTagTimeOut",       Set_TxBfProfileTag_TimeOut},
	{"TxBfProfileTagDesiredBw",     Set_TxBfProfileTag_DesiredBW},
	{"TxBfProfileTagDesiredNc",     Set_TxBfProfileTag_DesiredNc},
	{"TxBfProfileTagDesiredNr",     Set_TxBfProfileTag_DesiredNr},
	{"TxBfProfileTagRUAlloc",       set_txbf_prof_tag_ru_alloc},
	{"TxBfProfileTagRead",          Set_TxBfProfileTagRead},
	{"TxBfProfileTagWrite",         Set_TxBfProfileTagWrite},
	{"TxBfProfileDataRead",         Set_TxBfProfileDataRead},
	{"TxBfProfileDataWrite",        Set_TxBfProfileDataWrite},
	{"TxBfAngleWrite",              set_txbf_angle_write},
	{"TxBfDSNRWrite",               set_txbf_dsnr_write},
	{"TxBfPFMUDataWrite",           set_txbf_pfmu_data_write},
	{"TxBfProfilePnRead",           Set_TxBfProfilePnRead},
	{"TxBfProfilePnWrite",          Set_TxBfProfilePnWrite},
	{"TxBfTxSndInfo",               Set_TxBfTxSndInfo},
	{"TxBfPlyInfo",                 Set_TxBfPlyInfo},
	{"TxBfProfileSwTagWrite",       Set_TxBfProfileSwTagWrite}, /* For BFmee Pseudo PHY */
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
	{"StreamMode",					Set_StreamMode_Proc},
	{"StreamModeMCS",				Set_StreamModeMCS_Proc},
#endif /* STREAM_MODE_SUPPORT */

	{"RateAlg",					    Set_RateAlg_Proc},
#ifdef NEW_RATE_ADAPT_SUPPORT
	{"LowTrafficThrd",				Set_LowTrafficThrd_Proc},
	{"TrainUpRule",					Set_TrainUpRule_Proc},
	{"TrainUpRuleRSSI",				Set_TrainUpRuleRSSI_Proc},
	{"TrainUpLowThrd",				Set_TrainUpLowThrd_Proc},
	{"TrainUpHighThrd",				Set_TrainUpHighThrd_Proc},
#endif /* NEW_RATE_ADAPT_SUPPORT */



#ifdef MT_MAC
#ifdef DBG
	{"FixedRate",				    Set_Fixed_Rate_Proc},
	{"FixedRateFallback",		    Set_Fixed_Rate_With_FallBack_Proc},
	{"RaDebug",						Set_RA_Debug_Proc},
#endif /* DBG */
#endif /* MT_MAC */


#ifdef CONFIG_ATE
	{"ATE",							SetATE},
	{"ATEDA",						SetATEDa},
	{"ATESA",						SetATESa},
	{"ATELOGEN",					SetATELOGEnable},
	{"ATEMPSPHY",					SetATEMPSPhyMode},
	{"ATEMPSRATE",					SetATEMPSRate},
	{"ATEMPSPATH",					SetATEMPSPath},
	{"ATEMPSLEN",					SetATEMPSPayloadLen},
	{"ATEMPSTXCNT",				SetATEMPSPktCnt},
	{"ATEMPSTXPWR",				SetATEMPSPwr},
	{"ATEMPSTXSTART",				SetATEMPSStart},
	{"ATELOGDUMP",					SetATELOGDump},
	{"ATEMACTRX",					SetATEMACTRx},
	{"ATETXSTREAM",				SetATETxStream},
	{"ATERXSTREAM",				SetATERxStream},
	{"ATERXFILTER",				SetATERxFilter},
	{"ATELOGDIS",					SetATELOGDisable},
	{"ADCDump",						SetADCDump},
	{"ATEBSSID",					SetATEBssid},
	{"ATECHANNEL",					SetATEChannel},
	{"ATEDUTYCYCLE",				set_ate_duty_cycle},
	{"ATEPKTTXTIME",				set_ate_pkt_tx_time},
	{"ATECTRLBANDIDX",				set_ate_control_band_idx},
	{"ATEINITCHAN",					SetATEInitChan},
#ifdef RTMP_TEMPERATURE_CALIBRATION
	{"ATETEMPCAL",				SetATETempCal},
	{"ATESHOWTSSI",					SetATEShowTssi},
#endif /* RTMP_TEMPERATURE_CALIBRATION */
	{"ATETXPOW0",					SetATETxPower0},
	{"ATETXPOW1",					SetATETxPower1},
	{"ATETXPOW2",					SetATETxPower2},
	{"ATETXPOW3",					SetATETxPower3},
	{"ATETXPOWEVAL",				SetATETxPowerEvaluation},
	{"ATETXANT",					SetATETxAntenna},
	{"ATERXANT",					SetATERxAntenna},
	{"ATETXFREQOFFSET",				SetATETxFreqOffset},
	{"ATETXBW",					SetATETxBw},
	{"ATETXLEN",					SetATETxLength},
	{"ATETXCNT",					SetATETxCount},
	{"ATETXMCS",					SetATETxMcs},
	{"ATETXLDPC",					SetATETxLdpc},
	{"ATETXSTBC",					SetATETxStbc},
	{"ATETXMODE",					SetATETxMode},
	{"ATETXGI",					SetATETxGi},
	{"ATERXFER",					SetATERxFer},
	{"ATERRF",					SetATEReadRF},
#if (!defined(RTMP_RF_RW_SUPPORT)) && (!defined(RLT_RF))
	{"ATEWRF1",						SetATEWriteRF1},
	{"ATEWRF2",						SetATEWriteRF2},
	{"ATEWRF3",						SetATEWriteRF3},
	{"ATEWRF4",						SetATEWriteRF4},
#endif /* (!defined(RTMP_RF_RW_SUPPORT)) && (!defined(RLT_RF)) */
	{"ATELDE2P",				    SetATELoadE2p},
	/* #ifdef RTMP_EFUSE_SUPPORT */
	/* {"ATELDE2PFROMBUF",			Set_ATE_Load_E2P_From_Buf_Proc}, */
	/* #endif */ /* RTMP_EFUSE_SUPPORT */
	{"ATERE2P",						SetATEReadE2p},
	{"ATEAUTOALC",					SetATEAutoAlc},
	{"ATETEMPSENSOR", SetATETempSensor},
	{"ATEIPG",						SetATEIpg},
	{"ATEPAYLOAD",					SetATEPayload},
	{"ATEFIXEDPAYLOAD",				SetATEFixedPayload},
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
	{"ATETXBF",                     SetATETXBFProc},
	{"ATETXSOUNDING",               SetATETxSoundingProc},
#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */
	{"ATETTR",						SetATETtr},
	{"ATESHOW",						SetATEShow},
	{"ATEHELP",						SetATEHelp},

#ifdef CONFIG_QA
	{"TxStop",						SetTxStop},
	{"RxStop",						SetRxStop},
#ifdef DBG
	{"EERead",						SetEERead},
	{"EEWrite",						SetEEWrite},
	{"BBPRead",                     SetBBPRead},
	{"BBPWrite",                    SetBBPWrite},
#endif /* DBG */
#endif /* CONFIG_QA */


#endif /* CONFIG_ATE */

#ifdef WPA_SUPPLICANT_SUPPORT
	{"WpaSupport",                  Set_Wpa_Support},
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef WSC_STA_SUPPORT
	{"WscUuidE",					Set_WscUUIDE_Proc},
	{"WscGetConf",					Set_WscGetConf_Proc},
	{"WscVendorPinCode",            Set_WscVendorPinCode_Proc},
#ifdef WSC_V2_SUPPORT
	{"WscForceSetAP",               Set_WscForceSetAP_Proc},
#endif /* WSC_V2_SUPPORT */
#ifdef IWSC_SUPPORT
	{"IWscLimitedUI",		Set_IWscLimitedUI_Proc},
#ifdef IWSC_TEST_SUPPORT
	{"IWscDefaultSecurity",		Set_IWscDefaultSecurity_Proc},
	{"IWscSmpbcScanOnly",		Set_IWscSmpbcEnrScanOnly_Proc},
	{"IWscEmptySubmaskList",		Set_IWscEmptySubmaskList_Proc},
	{"IWscBlockConnection",		Set_IWscBlockConnection_Proc},
#endif /* IWSC_TEST_SUPPORT */
	{"IWscOOB",				Set_IWscOOB_Proc},
	{"IWscSinglePIN",		Set_IWscSinglePIN_Proc},
#endif /* IWSC_SUPPORT // */
#endif /* WSC_STA_SUPPORT */

#ifdef ETH_CONVERT_SUPPORT
	{"EthConvertMode",              Set_EthConvertMode_Proc},
	{"EthCloneMac",					Set_EthCloneMac_Proc},
#endif /* ETH_CONVERT_SUPPORT */


#ifdef DOT11R_FT_SUPPORT
	{"ft",							FT_Ioctl},
#endif /* DOT11R_FT_SUPPORT */

	{"FixedTxMode",                 Set_FixedTxMode_Proc},
#ifdef CONFIG_APSTA_MIXED_SUPPORT
	{"OpMode",						Set_OpMode_Proc},
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef ETH_CONVERT_SUPPORT
#ifdef IP_ASSEMBLY
	{"FragFlag",		            Set_FragTest_Proc},
#endif /* IP_ASSEMBLY */
#endif /* ETH_CONVERT_SUPPORT */
#ifdef DOT11_N_SUPPORT
	{"TGnWifiTest",                 Set_TGnWifiTest_Proc},
#endif /* DOT11_N_SUPPORT */
	{"LongRetry",				Set_LongRetryLimit_Proc},
	{"ShortRetry",				Set_ShortRetryLimit_Proc},
	{"AutoFallBack",			Set_AutoFallBack_Proc},
#ifdef EXT_BUILD_CHANNEL_LIST
	{"11dClientMode",				Set_Ieee80211dClientMode_Proc},
	{"CountryCode",				Set_ExtCountryCode_Proc},
	{"DfsType",					Set_ExtDfsType_Proc},
	{"ChannelListAdd",				Set_ChannelListAdd_Proc},
	{"ChannelListShow",			Set_ChannelListShow_Proc},
	{"ChannelListDel",				Set_ChannelListDel_Proc},
#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef CARRIER_DETECTION_SUPPORT
	{"CarrierDetect",				Set_StaCarrierDetect_Proc},
#endif /* CARRIER_DETECTION_SUPPORT */


#ifdef RTMP_EFUSE_SUPPORT
	{"efuseLoadFromBin",			set_eFuseLoadFromBin_Proc}, /* For backward compatible, the usage is the same as bufferLoadFromBin + bufferWriteBack */
	{"efuseFreeNumber",			set_eFuseGetFreeBlockCount_Proc},
	{"efuseDump",				set_eFusedump_Proc},
#ifdef CONFIG_ATE
	{"bufferLoadFromEfuse",		Set_LoadEepromBufferFromEfuse_Proc},
	{"efuseBufferModeWriteBack",	set_eFuseBufferModeWriteBack_Proc}, /* For backward compatible, the usage is the same as bufferWriteBack */
#endif /* CONFIG_ATE */
#endif /* RTMP_EFUSE_SUPPORT */
	{"bufferLoadFromBin",			Set_LoadEepromBufferFromBin_Proc},
	{"bufferWriteBack",			Set_EepromBufferWriteBack_Proc},

#ifdef CAL_FREE_IC_SUPPORT
	{"bufferLoadFromCalFree",		Set_LoadCalFreeData_Proc},
	{"CheckCalFree",		        Set_CheckCalFree_Proc},
#endif

	{"BeaconLostTime",				Set_BeaconLostTime_Proc},
	{"AutoRoaming",					Set_AutoRoaming_Proc},
	{"SiteSurvey",					Set_SiteSurvey_Proc},
	{"ForceTxBurst",				Set_ForceTxBurst_Proc},

#ifdef RTMP_RBUS_SUPPORT
#ifdef LED_CONTROL_SUPPORT
	{"WlanLed",					Set_WlanLed_Proc},
#endif /* LED_CONTROL_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

	{"tpc",						set_thermal_protection_criteria_proc},
	{"tpc_duty",					set_thermal_protection_admin_ctrl_duty_proc},
	{"show_tpc_duty",					get_thermal_protection_admin_ctrl_duty_proc},

#ifdef DOT11Z_TDLS_SUPPORT
	{"TdlsCapable",					Set_TdlsCapable_Proc},
	{"TdlsSetup",					Set_TdlsSetup_Proc},
	{"TdlsTearDown",				Set_TdlsTearDown_Proc},
	{"TdlsDiscoveryReq",				Set_TdlsDiscoveryReq_Proc},
	{"TdlsTPKLifeTime",				Set_TdlsTPKLifeTime_Proc},
#ifdef WFD_SUPPORT
	{"TdlsTunneledReq",				Set_TdlsTunneledReqProc},
#endif /* WFD_SUPPORT */
	{"TdlsAcceptWeakSec",			Set_TdlsAcceptWeakSecurityProc},
	{"TdlsChannelSwitch",			Set_TdlsChannelSwitch_Proc},
	{"TdlsChannelSwitchBW",			Set_TdlsChannelSwitchBW_Proc},
	{"TdlsChannelSwitchDisable",	Set_TdlsChannelSwitchDisable_Proc},
#ifdef TDLS_AUTOLINK_SUPPORT
	{"TdlsAutoLink",					Set_TdlsAutoLinkProc},
	{"TdlsRssiMeasurementPeriod",		Set_TdlsRssiMeasurementPeriodProc},
	{"TdlsAutoDiscoveryPeriod",			Set_TdlsAutoDiscoveryPeriodProc},
	{"TdlsAutoSetupRssiThreshold",		Set_TdlsAutoSetupRssiThresholdProc},
	{"TdlsDisabledPeriodByTeardown",		Set_TdlsDisabledPeriodByTeardownProc},
	{"TdlsAutoTeardownRssiThreshold",	Set_TdlsAutoTeardownRssiThresholdProc},
#endif /* TDLS_AUTOLINK_SUPPORT // */
#ifdef UAPSD_SUPPORT
	{"tdls",						TDLS_Ioctl},
#endif /* UAPSD_SUPPORT */
#endif /* DOT11Z_TDLS_SUPPORT */



	{"AutoReconnect",				Set_AutoReconnect_Proc},
	{"AdhocN",						Set_AdhocN_Proc},
#ifdef WIDI_SUPPORT
	{"WidiEnable",					Set_WiDiEnable_Proc},
#endif /* WIDI_SUPPORT */
#ifdef AGS_SUPPORT
	{"Ags",						Show_AGS_Proc},
#endif /* AGS_SUPPORT */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
	{"wow_enable",					Set_WOW_Enable},
	{"wow_gpio",					Set_WOW_GPIO},
	{"wow_delay",					Set_WOW_Delay},
	{"wow_hold",                    Set_WOW_Hold},
	{"wow_inband",                  Set_WOW_InBand},
	{"wow_interface",               Set_WOW_Interface},
	{"wow_ipaddress",				Set_WOW_IPAddress},
	{"wow_gpiohighlow",             Set_WOW_GPIOHighLow},
	{"wow_ifdownsupport",             Set_WOW_IfDown_Support},
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)  || defined(MT_WOW_SUPPORT) */

#ifdef SNIFFER_SUPPORT
	{"MonitorMode",				Set_MonitorMode_Proc},
#endif /* SNIFFER_SUPPORT */

#ifdef WFD_SUPPORT
	{"WfdEnable",						Set_WfdEnable_Proc},
#ifdef RT_CFG80211_SUPPORT
	{"WfdInsertIe",						Set_WfdInsertIe_Proc},
#endif /* RT_CFG80211_SUPPORT */
	{"WfdDevType",						Set_WfdDeviceType_Proc},
	{"WfdCouple",						Set_WfdCouple_Proc},
	{"WfdSessionAvail",				Set_WfdSessionAvailable_Proc},
	{"WfdCP",							Set_WfdCP_Proc},
	{"WfdRtspPort",						Set_WfdRtspPort_Proc},
	{"WfdMaxThroughput",				Set_WfdMaxThroughput_Proc},
	{"WfdLocalIp",						Set_WfdLocalIp_Proc},
	{"WfdPeerRtspPort",					Set_PeerRtspPort_Proc},
#endif /* WFD_SUPPORT */

#ifdef SINGLE_SKU
	{"ModuleTxpower",				Set_ModuleTxpower_Proc},
#endif /* SINGLE_SKU */

#ifdef DOT11W_PMF_SUPPORT
	{"PMFMFPC",                                     Set_PMFMFPC_Proc},
	{"PMFMFPR",                                     Set_PMFMFPR_Proc},
	{"PMFSHA256",                                   Set_PMFSHA256_Proc},
#endif /* DOT11W_PMF_SUPPORT */

	{"rf", SetRF},
#ifdef MICROWAVE_OVEN_SUPPORT
	{"MO_FalseCCATh",               Set_MO_FalseCCATh_Proc},
#endif /* MICROWAVE_OVEN_SUPPORT */
#ifdef SNIFFER_SUPPORT
	{"mc", set_monitor_channel},
#endif
	{"tssi_enable", set_tssi_enable},
#ifdef CONFIG_MULTI_CHANNEL
	{"mcc_ch1_stay",				Set_CH1StayTime_Proc},
	{"mcc_ch2_stay",				Set_CH2StayTime_Proc},
	{"mcc_idle",				Set_SwitchIdleTime_Proc},
	{"mcc_null_cnt",			Set_NULL_Frame_Count_Proc},
	{"mcc_ch1_bw",					Set_CH1_BW_Proc},
	{"mcc_ch2_bw",					Set_CH2_BW_Proc},
	{"mcc_ch1_ch",					Set_CH1_Channel_Proc},
	{"mcc_ch2_ch",					Set_CH2_Channel_Proc},
	{"mcc_start",					Set_MCC_Start_Proc},
	{"pkt_dbg",					Set_Pkt_Dbg_Proc},
#endif /* CONFIG_MULTI_CHANNEL */

#ifdef MT_MAC
	/* support CR4 cmds */
	{"cr4_query",                   set_cr4_query},
	{"cr4_set",                     set_cr4_set},
	{"cr4_capability",              set_cr4_capability},
	{"cr4_debug",                   set_cr4_debug},

	{"rdg",         set_manual_rdg},
	{"protect",     set_manual_protect},
	{"fwlog", set_fw_log},
	{"isrcmd",		set_isr_cmd},
	{"fwset",                   set_fw_cmd},
	{"fwget",                   get_fw_cmd},
	{"get_thermal_sensor", Set_themal_sensor},
#ifdef FW_DUMP_SUPPORT
	{"fwdump_maxsize", set_fwdump_max_size},
	{"fwdump_path", set_fwdump_path},
	{"fwdump_print", fwdump_print},
#endif
#endif
	{"RadioOn",     Set_RadioOn_Proc},
	{"Lp",		Set_Lp_Proc},

#ifdef MT_MAC
#ifdef COEX_SUPPORT
	{"MT76xxCoexBeacomLimit",		Set_MT76xxCoexBeaconLimit_Proc},
	{"MT76xxCoexBaSize",		Set_MT76xxCoexBaWinsize_Proc},
	{"MT76xxCoexMode",		Set_MT76xxCoexMode_Proc},
	{"MT76xxCoexSupportMode",		Set_MT76xxCoexSupportMode_Proc},
	{"MT76xxCoex",		Set_MT76xxCoex_Proc},
	{"MT76xxCoexProctectionMode",		Set_MT76xxCoex_Protection_Mode_Proc},
	{"MT76xxCoexProctectionRate",		Set_MT76xxCoex_Protection_Rate_Proc},
	{"MT76xxCoexMode",		Set_MT76xxCoex_Proc},
	{"MT76xxCoexSkipFDDFix20MH",		Set_CoexSkipFDDFix20MH_Proc},
#endif /* COEX_SUPPORT */
#endif
#ifdef RT_CFG80211_SUPPORT
	{"DisableCfg2040Scan",				Set_DisableCfg2040Scan_Proc},
#endif /* RT_CFG80211_SUPPORT */
#ifdef CFG_SUPPORT_MU_MIMO
	{"set_mu_gidup",                    SetMuStaParamProc},
#endif
	{"wifi_sys",	show_wifi_sys},
	{"timer_list", show_timer_list},
	{"wtbl_stat", show_wtbl_state},
	{"radio_info", show_radio_info_proc},
	{"tx_amsdu", set_tx_amsdu},
	{"rx_amsdu", set_rx_amsdu},
	{"tx_max_cnt", set_tx_max_cnt},
	{"rx_max_cnt", set_rx_max_cnt},
	{"rx_cnt_io_thd", set_rx_cnt_io_thd},
	{"ba_dbg", set_ba_dbg},
#ifdef DOT11_HE_AX
	{"color_dbg", set_color_dbg},
#endif
#ifdef VERIFICATION_MODE
	{"dump_rx", set_dump_rx_debug},
	{"veri_pkt_head", set_veri_pkt_head},
	{"veri_pkt_ctnt", set_veri_pkt_ctnt},
	{"send_veri_pkt", send_veri_pkt},
	{"veri_pkt_ctrl_en", set_veri_pkt_ctrl_en},
	{"veri_pkt_ctrl_assign", set_veri_pkt_ctrl_assign},
	{"veri_switch", veri_mode_switch},
#endif
#ifdef DOT11_SAE_SUPPORT
	{"sae_k_iter", sae_set_k_iteration},
#endif
#ifdef CONFIG_WIFI_SYSDVT
	{"dvt", dvt_feature_search},
#endif /*CONFIG_WIFI_SYSDVT*/
#ifdef CONFIG_WIFI_DBG_TXCMD
		{"dbg_txcmd", dbg_txcmd_feature_search},
#endif /*CONFIG_WIFI_DBG_TXCMD*/
	{NULL,}
};

static struct {
	RTMP_STRING *name;
	INT (*set_proc)(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
} *PRTMP_PRIVATE_SHOW_PROC, RTMP_PRIVATE_SHOW_SUPPORT_PROC[] = {

	{"devinfo", show_devinfo_proc},
	{"stainfo", Show_MacTable_Proc},
	{"hwctrl", Show_HwCtrlStatistic_Proc},
	{"trinfo", show_trinfo_proc},
	{"tpinfo", show_tpinfo_proc},
	{"sysinfo", show_sysinfo_proc},
	{"pwrinfo", chip_show_pwr_info},
#ifdef MT_MAC
	{"wtbl", show_wtbl_proc},
	{"mibinfo", show_mib_proc},
	{"amsduinfo", show_amsdu_proc},
	{"tmacinfo", ShowTmacInfo},
	{"agginfo", ShowAggInfo},
	{"arbinfo", ShowArbInfo},
	{"pseinfo", ShowPseInfo},
	{"pleinfo", ShowPLEInfo},
	{"txdinfo", show_TXD_proc},
	{"dumpmem", show_mem_proc},
	{"psedata", ShowPseData},
	{"dschinfo", show_dmasch_proc},
#ifdef DBDC_MODE
	{"dbdcinfo", ShowDbdcProc},
#endif /*DBDC_MODE*/
	{"txopinfo", show_tx_burst_info},
	{"driverinfo", show_driverinfo_proc},
#endif /* MT_MAC */
#ifdef MEM_ALLOC_INFO_SUPPORT
	{"meminfo", Show_MemInfo_Proc},
	{"pktmeminfo", Show_PktInfo_Proc},
#endif /* MEM_ALLOC_INFO_SUPPORT */
	{"wfintcnt", ShowWifiInterruptCntProc},
#ifdef RTMP_EFUSE_SUPPORT
	{"efuseinfo", show_efuseinfo_proc},
#endif
	{"e2pinfo", show_e2pinfo_proc},
#ifdef REDUCE_TCP_ACK_SUPPORT
	{"ReduceAckShow", Show_ReduceAckInfo_Proc},
#endif
#ifdef DOT11_SAE_SUPPORT
	{"saeinfo", show_sae_info_proc},
#endif
	{"timelog", show_time_log_info},
#ifdef DOT11_HE_AX
	{"colorinfo", show_bsscolor_proc},
#endif
	{NULL,}
};


INT RTMPSTAPrivIoctlSet(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *SetProcName,
	IN RTMP_STRING *ProcArg)
{
	int ret = 0;

	for (PRTMP_PRIVATE_SET_PROC = RTMP_PRIVATE_SUPPORT_PROC; PRTMP_PRIVATE_SET_PROC->name; PRTMP_PRIVATE_SET_PROC++) {
		if (rtstrcasecmp(SetProcName, PRTMP_PRIVATE_SET_PROC->name) == TRUE) {
			if (!PRTMP_PRIVATE_SET_PROC->set_proc(pAd, ProcArg)) {
				/*FALSE:Set private failed then return Invalid argument */
				return NDIS_STATUS_FAILURE;
			}

			break;	/*Exit for loop. */
		}
	}

	if (PRTMP_PRIVATE_SET_PROC->name == NULL) {
		/*Not found argument */
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "(iwpriv) Not Support Set Command [%s=%s]\n", SetProcName, ProcArg);
		return -EINVAL;
	}

	return ret;
}

INT RTMPSTAPrivIoctlShow(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *SetProcName,
	IN RTMP_STRING *ProcArg)
{
	int ret = 0;

	for (PRTMP_PRIVATE_SHOW_PROC = RTMP_PRIVATE_SHOW_SUPPORT_PROC; PRTMP_PRIVATE_SHOW_PROC->name;
		 PRTMP_PRIVATE_SHOW_PROC++) {
		if (rtstrcasecmp(SetProcName, PRTMP_PRIVATE_SHOW_PROC->name) == TRUE) {
			if (!PRTMP_PRIVATE_SHOW_PROC->set_proc(pAd, ProcArg)) {
				/*FALSE:Set private failed then return Invalid argument */
				return NDIS_STATUS_FAILURE;
			}

			break;	/*Exit for loop. */
		}
	}

	if (PRTMP_PRIVATE_SHOW_PROC->name == NULL) {
		/*Not found argument */
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "(iwpriv) Not Support Show Command [%s=%s]\n", SetProcName, ProcArg);
		return -EINVAL;
	}

	return ret;
}

/*
 *    ==========================================================================
 *    Description:
 *	Set SSID
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT Set_SSID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	NDIS_802_11_SSID Ssid, *pSsid = NULL;
	BOOLEAN StateMachineTouched = FALSE;
	int success = TRUE;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev; /* snowpin for cntl mgmt */
	SCAN_INFO *ScanInfo = NULL;

	wdev = &pStaCfg->wdev; /* snowpin for cntl mgmt */
	ScanInfo = &wdev->ScanInfo;
	/*
	 *	Set the AutoReconnectSsid to prevent it reconnect to old SSID
	 *	Since calling this indicate user don't want to connect to that SSID anymore.
	 */
	pStaCfg->MlmeAux.AutoReconnectSsidLen = 32;
	NdisZeroMemory(pStaCfg->MlmeAux.AutoReconnectSsid, pStaCfg->MlmeAux.AutoReconnectSsidLen);

	if (strlen(arg) <= MAX_LEN_OF_SSID) {
		NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));

		if (strlen(arg) != 0) {
			NdisMoveMemory(Ssid.Ssid, arg, strlen(arg));
			Ssid.SsidLength = strlen(arg);
			/* Calculate PMK */
			SetWPAPSKKey(pAd, pStaCfg->wdev.SecConfig.PSK, strlen(pStaCfg->wdev.SecConfig.PSK), &Ssid.Ssid[0], Ssid.SsidLength,
						 pStaCfg->wdev.SecConfig.PMK);
		} else { /*ANY ssid */
			Ssid.SsidLength = 0;
			memcpy(Ssid.Ssid, "", 0);
			pStaCfg->BssType = BSS_INFRA;
			SET_AKM_OPEN_Entry(&pStaCfg->wdev);
			SET_CIPHER_NONE_Entry(&pStaCfg->wdev);
#ifdef WCX_SUPPORT
			{
				BOOTMODE boot_mode;

				boot_mode = get_boot_mode();

				if (boot_mode == FACTORY_BOOT) {
					NdisMoveMemory(Ssid.Ssid, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen);
					Ssid.SsidLength = pAd->CommonCfg.SsidLen;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "!!! arg:%s, CommonCfg.Ssid:%s, SsidLen:%d\n", Ssid.Ssid,
							 pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen);
				}
			}
#endif
		}

		pSsid = &Ssid;

		/* Record the desired user settings to MlmeAux */
		NdisZeroMemory(pStaCfg->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pStaCfg->MlmeAux.Ssid, Ssid.Ssid, Ssid.SsidLength);
		pStaCfg->MlmeAux.SsidLen = (UCHAR)Ssid.SsidLength;
		NdisMoveMemory(pStaCfg->MlmeAux.AutoReconnectSsid, Ssid.Ssid, Ssid.SsidLength);
		pStaCfg->MlmeAux.AutoReconnectSsidLen = (UCHAR)Ssid.SsidLength;
		pStaCfg->MlmeAux.CurrReqIsFromNdis = TRUE;
		pStaCfg->bSkipAutoScanConn = FALSE;
		pStaCfg->bConfigChanged = TRUE;
		pStaCfg->bNotFirstScan = FALSE;
#ifdef DOT11W_PMF_SUPPORT
		{
			PMF_CFG *pPmfCfg = &pStaCfg->wdev.SecConfig.PmfCfg;

			pPmfCfg->MFPC = FALSE;
			pPmfCfg->MFPR = FALSE;
			pPmfCfg->PMFSHA256 = FALSE;

			if ((IS_AKM_WPA2_Entry(&pStaCfg->wdev) ||
			     IS_AKM_WPA2PSK_Entry(&pStaCfg->wdev) ||
			     IS_AKM_OWE_Entry((&pStaCfg->wdev))) &&
			     IS_CIPHER_AES_Entry(&pStaCfg->wdev)) {
				pPmfCfg->PMFSHA256 = pPmfCfg->Desired_PMFSHA256;

				if (pPmfCfg->Desired_MFPC) {
					pPmfCfg->MFPC = TRUE;
					pPmfCfg->MFPR = pPmfCfg->Desired_MFPR;

					if (pPmfCfg->MFPR)
						pPmfCfg->PMFSHA256 = TRUE;
				}
			} else if (pPmfCfg->Desired_MFPC)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[PMF]:: Security is not WPA2/WPA2PSK AES\n");

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[PMF]:: MFPC=%d, MFPR=%d, SHA256=%d\n",
					 pPmfCfg->MFPC, pPmfCfg->MFPR,
					 pPmfCfg->PMFSHA256);
		}
#endif /* DOT11W_PMF_SUPPORT */
		cntl_connect_request(wdev, CNTL_CONNECT_BY_SSID, sizeof(NDIS_802_11_SSID), (UCHAR *)pSsid);
		StateMachineTouched = TRUE;

		if (Ssid.SsidLength == MAX_LEN_OF_SSID)
			hex_dump("Set_SSID_Proc::Ssid", Ssid.Ssid, Ssid.SsidLength);
		else
			MTWF_PRINT("Len=%d,Ssid=%s\n", Ssid.SsidLength, Ssid.Ssid);
	} else
		success = FALSE;

	if (StateMachineTouched) /* Upper layer sent a MLME-related operations */
		RTMP_MLME_HANDLER(pAd);

	return success;
}


/*
 *    ==========================================================================
 *    Description:
 *	Set WmmCapable Enable or Disable
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT	Set_WmmCapable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	BOOLEAN	bWmmCapable;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	bWmmCapable = os_str_tol(arg, 0, 10);

	if (bWmmCapable == 1)
		pStaCfg->wdev.bWmmCapable = TRUE;
	else if (bWmmCapable == 0)
		pStaCfg->wdev.bWmmCapable = FALSE;
	else
		return FALSE;  /*Invalid argument */

	MTWF_PRINT("bWmmCapable=%d\n", pStaCfg->wdev.bWmmCapable);
	return TRUE;
}


/*
 *    ==========================================================================
 *    Description:
 *	Set UAPSDCapable Enable or Disable
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT	Set_APSDCapable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	BOOLEAN APSDCapable;

	APSDCapable = os_str_tol(arg, 0, 10);

	if (APSDCapable)
		pStaCfg->wdev.UapsdInfo.bAPSDCapable = TRUE;
	else
		pStaCfg->wdev.UapsdInfo.bAPSDCapable = FALSE;

	MTWF_PRINT("UAPSDCapable=%d\n", pStaCfg->wdev.UapsdInfo.bAPSDCapable);
	return TRUE;
}


/*
 *    ==========================================================================
 *    Description:
 *	Set APSDAC Enable or Disable
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT	Set_APSDAC_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *value = NULL;
	UCHAR i = 0;
	BOOLEAN apsd_ac[4] = {0};

	for (i = 0, value = rstrtok(arg, ":"); (i < WMM_NUM_OF_AC && value != NULL); value = rstrtok(NULL, ":"), i++) {
		apsd_ac[i] = (BOOLEAN)os_str_tol(value, 0, 10);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APSDAC[%d]=%d\n", i,  apsd_ac[i]);
	}

	if (i  !=  WMM_NUM_OF_AC) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid parameters\n");
		return FALSE;
	}

	pAd->CommonCfg.bAPSDAC_BE = apsd_ac[0];
	pAd->CommonCfg.bAPSDAC_BK = apsd_ac[1];
	pAd->CommonCfg.bAPSDAC_VI = apsd_ac[2];
	pAd->CommonCfg.bAPSDAC_VO = apsd_ac[3];
	pAd->CommonCfg.bACMAPSDTr[0] = apsd_ac[0];
	pAd->CommonCfg.bACMAPSDTr[1] = apsd_ac[1];
	pAd->CommonCfg.bACMAPSDTr[2] = apsd_ac[2];
	pAd->CommonCfg.bACMAPSDTr[3] = apsd_ac[3];
	MTWF_PRINT("APSDAC::(BE,BK,VI,VO)=(%d,%d,%d,%d)\n",
			 pAd->CommonCfg.bAPSDAC_BE,
			 pAd->CommonCfg.bAPSDAC_BK,
			 pAd->CommonCfg.bAPSDAC_VI,
			 pAd->CommonCfg.bAPSDAC_VO);
	return TRUE;
}


/*
 *    ==========================================================================
 *    Description:
 *	Set WmmCapable Enable or Disable
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT	Set_MaxSPLength_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->CommonCfg.MaxSPLength = os_str_tol(arg, 0, 10);
	MTWF_PRINT("MaxSPLength=%d\n", pAd->CommonCfg.MaxSPLength);
	return TRUE;
}


/*
 *    ==========================================================================
 *    Description:
 *	Set Network Type(Infrastructure/Adhoc mode)
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT Set_NetworkType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;

	if (strcmp(arg, "Adhoc") == 0) {
		if (pStaCfg->BssType != BSS_ADHOC) {
			/* Config has changed */
			pStaCfg->bConfigChanged = TRUE;

			if (MONITOR_ON(pAd)) {
				STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
				pStaCfg->bAutoReconnect = TRUE;
				LinkDown(pAd, FALSE, &pStaCfg->wdev, NULL);
			}

			if (INFRA_ON(pStaCfg)) {
				/*BOOLEAN Cancelled; */
				/* Set the AutoReconnectSsid to prevent it reconnect to old SSID */
				/* Since calling this indicate user don't want to connect to that SSID anymore. */
				pStaCfg->MlmeAux.AutoReconnectSsidLen = 32;
				NdisZeroMemory(pStaCfg->MlmeAux.AutoReconnectSsid, pStaCfg->MlmeAux.AutoReconnectSsidLen);
				LinkDown(pAd, FALSE, &pStaCfg->wdev, NULL);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "NDIS_STATUS_MEDIA_DISCONNECT Event BB!\n");
			}

#ifdef DOT11_N_SUPPORT
			SetCommonHtVht(pAd, &pStaCfg->wdev);
#endif /* DOT11_N_SUPPORT */
		}

		pStaCfg->BssType = BSS_ADHOC;
		RTMP_OS_NETDEV_SET_TYPE(pAd->net_dev, pStaCfg->OriDevType);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AD-HOC\n");
	} else if (strcmp(arg, "Infra") == 0) {
		if (pStaCfg->BssType != BSS_INFRA) {
			/* Config has changed */
			pStaCfg->bConfigChanged = TRUE;

			if (MONITOR_ON(pAd)) {
				STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
				pStaCfg->bAutoReconnect = TRUE;
				LinkDown(pAd, FALSE, &pStaCfg->wdev, NULL);
				pAd->monitor_ctrl.bMonitorOn = FALSE;
			}

			if (ADHOC_ON(pAd)) {
				/* Set the AutoReconnectSsid to prevent it reconnect to old SSID */
				/* Since calling this indicate user don't want to connect to that SSID anymore. */
				pStaCfg->MlmeAux.AutoReconnectSsidLen = 32;
				NdisZeroMemory(pStaCfg->MlmeAux.AutoReconnectSsid, pStaCfg->MlmeAux.AutoReconnectSsidLen);
				LinkDown(pAd, FALSE, &pStaCfg->wdev, NULL);
			}

#ifdef DOT11_N_SUPPORT
			SetCommonHtVht(pAd, &pStaCfg->wdev);
#endif /* DOT11_N_SUPPORT */
		}

		pStaCfg->BssType = BSS_INFRA;
		RTMP_OS_NETDEV_SET_TYPE(pAd->net_dev, pStaCfg->OriDevType);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "INFRA\n");
	}

#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
	/*
	 *	Monitor2 is for 3593 11n wireshark sniffer tool.
	 *	The name, Monitor2, follows the command format in RT2883.
	 */
	else if ((strcmp(arg, "Monitor") == 0) || (strcmp(arg, "Monitor2") == 0))
#else
	else if (strcmp(arg, "Monitor") == 0)
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */
	{
#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT

		if (strcmp(arg, "Monitor2") == 0)
			pStaCfg->BssMonitorFlag |= MONITOR_FLAG_11N_SNIFFER;

#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */
		STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_INFRA_ON);
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADHOC_ON);
		STA_STATUS_SET_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
		/* disable all periodic state machine */
		pStaCfg->bAutoReconnect = FALSE;
		/* reset all mlme state machine */
		RTMP_MLME_RESET_STATE_MACHINE(pAd, &pStaCfg->wdev);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "fSTA_STATUS_MEDIA_STATE_CONNECTED\n");
		wlan_operate_set_prim_ch(wdev, wdev->channel);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BW_%s, CtrlChannel(%d), CentralChannel(%d)\n",
				 (wlan_operate_get_bw(wdev) == BW_40 ? "40" : "20"),
				 wdev->channel,
				 wlan_operate_get_cen_ch_1(wdev));
		pStaCfg->BssType = BSS_MONITOR;
		pAd->monitor_ctrl.bMonitorOn = TRUE;
		RTMP_OS_NETDEV_SET_TYPE_MONITOR(pAd->net_dev);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MONITOR\n");
#ifdef SNIFFER_MT7615

		if (IS_MT7615(pAd)) {
			MAC_IO_WRITE32(pAd->hdev_ctrl, WF_RMAC_BASE + 0x04, 0);
			MAC_IO_WRITE32(pAd->hdev_ctrl, WF_DMA_BASE + 0x74, 0);
			MAC_IO_WRITE32(pAd->hdev_ctrl, WF_DMA_BASE + 0xb4, 0);
		}

#endif
	}

	/* Set Rx Filter after eaxctly know what mode currently we work on */
	AsicSetRxFilter(pAd);
	/* Reset Ralink supplicant to not use, it will be set to start when UI set PMK key */
	pStaCfg->WpaState = SS_NOTUSE;
	MTWF_PRINT("NetworkType=%d\n", pStaCfg->BssType);

	return TRUE;
}

/*
 *    ==========================================================================
 *    Description:
 *	Set Authentication mode
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT Set_AuthMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;
	struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

	SetWdevAuthMode(pSecConfig, arg);
	wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	MTWF_PRINT("AKM=0x%x\n", pSecConfig->AKMMap);

	return TRUE;
}


/*
 *    ==========================================================================
 *    Description:
 *	Set Encryption Type
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT Set_EncrypType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	CHANNEL_CTRL *pChCtrl;
	UCHAR BandIdx;
	struct wifi_dev *wdev = &pStaCfg->wdev;
	struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

	SetWdevEncrypMode(pSecConfig, arg);

	/* TODO: Why need to do it here */
	if (pStaCfg->BssType == BSS_ADHOC) {
		/* Build all corresponding channel information */
		/* Change channel state to NONE */
		BandIdx = HcGetBandByWdev(wdev);
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
#ifdef EXT_BUILD_CHANNEL_LIST
		BuildChannelListEx(pAd, wdev);
#else
		BuildChannelList(pAd, wdev);
#endif
		RTMPSetPhyMode(pAd, wdev, pAd->CommonCfg.cfg_wmode);
	}

	MTWF_PRINT("CIPHER=0x%x\n", pSecConfig->AKMMap);

	return TRUE;
}

/*
 *    ==========================================================================
 *    Description:
 *	Set Default Key ID
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT Set_DefaultKeyID_Proc(RTMP_ADAPTER *pAdapter, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAdapter->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAdapter->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;
	ULONG KeyIdx;
	struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

	KeyIdx = os_str_tol(arg, 0, 10);

	if ((KeyIdx >= 1) && (KeyIdx <= 4))
		pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
	else
		return FALSE;

	MTWF_PRINT("DefaultKeyId=%d\n", pSecConfig->PairwiseKeyId);

	return TRUE;
}

INT Set_Wep_Key_Proc(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  RTMP_STRING *Key,
	IN  INT             KeyLen,
	IN  INT             KeyId)
{
	POS_COOKIE pObj = (POS_COOKIE)pAdapter->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAdapter->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;
	struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;
	INT retVal = FALSE;

	retVal = ParseWebKey(pSecConfig, Key, KeyId, KeyLen);
	MTWF_PRINT("KeyID=%d, key=%s\n", KeyId, Key);

	return retVal;
}

/*
 *    ==========================================================================
 *    Description:
 *	Set WEP KEY1
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT Set_Key1_Proc(RTMP_ADAPTER *pAdapter, RTMP_STRING *arg)
{
	return Set_Wep_Key_Proc(pAdapter, arg, strlen(arg), 0);
}
/*
 *    ==========================================================================
 *
 *    Description:
 *	Set WEP KEY2
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT Set_Key2_Proc(RTMP_ADAPTER *pAdapter, RTMP_STRING *arg)
{
	return Set_Wep_Key_Proc(pAdapter, arg, strlen(arg), 1);
}
/*
 *    ==========================================================================
 *    Description:
 *	Set WEP KEY3
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT Set_Key3_Proc(RTMP_ADAPTER *pAdapter, RTMP_STRING *arg)
{
	return Set_Wep_Key_Proc(pAdapter, arg, strlen(arg), 2);
}
/*
 *    ==========================================================================
 *    Description:
 *	Set WEP KEY4
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT Set_Key4_Proc(RTMP_ADAPTER *pAdapter, RTMP_STRING *arg)
{
	return Set_Wep_Key_Proc(pAdapter, arg, strlen(arg), 3);
}

/*
 *    ==========================================================================
 *    Description:
 *	Set WPA PSK key
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT Set_WPAPSK_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;
	struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

	if (strlen(arg) < 65) {
		os_move_mem(pSecConfig->PSK, arg, strlen(arg));
		pSecConfig->PSK[strlen(arg)] = '\0';
	} else
		pSecConfig->PSK[0] = '\0';

	MTWF_PRINT("PSK=%s\n", arg);
#ifdef WSC_STA_SUPPORT
	NdisZeroMemory(wdev->WscControl.WpaPsk, 64);
	wdev->WscControl.WpaPskLen = 0;
	NdisMoveMemory(wdev->WscControl.WpaPsk, arg, strlen(arg));
	wdev->WscControl.WpaPskLen = (INT)strlen(arg);
#endif /* WSC_STA_SUPPORT */
	return 0;
}


#ifdef CFG_TDLS_SUPPORT
INT	Set_CfgTdlsChannelSwitchBW_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING		*arg)
{
	UCHAR	ChannelBW;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	ChannelBW = os_str_tol(arg, 0, 10);
	pStaCfg->wpa_supplicant_info.CFG_Tdls_info.TargetOffChannelBW = ChannelBW;
	MTWF_PRINT("TdlsChannelSwitchBW=%d\n",
			  pStaCfg->wpa_supplicant_info.CFG_Tdls_info.TargetOffChannelBW);

	return TRUE;
}

INT	Set_CfgTdlsChannelSwitchBaseStayTime_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING		*arg)
{
	UCHAR	StayTime;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	StayTime = os_str_tol(arg, 0, 10);
	pStaCfg->wpa_supplicant_info.CFG_Tdls_info.BaseChannelStayTime = StayTime;
	MTWF_PRINT("BaseChannelStayTime=%d\n",
		pStaCfg->wpa_supplicant_info.CFG_Tdls_info.BaseChannelStayTime);

	return TRUE;
}


INT	Set_TdlsDisableChannelSwitchProc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING		*arg)
{
	UCHAR					macAddr[MAC_ADDR_LEN];
	RTMP_STRING				*value;
	INT						i;
	BOOLEAN TimerCancelled = 0;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	if (strlen(arg) != 17) /* Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /* Invalid */

		AtoH(value, &macAddr[i++], 1);
	}

	if (pStaCfg->wpa_supplicant_info.CFG_Tdls_info.bChannelSwitchInitiator &&
		pStaCfg->wpa_supplicant_info.CFG_Tdls_info.IamInOffChannel)
		cfg_tdls_chsw_req(pAd, macAddr, pStaCfg->wpa_supplicant_info.CFG_Tdls_info.BaseChannel,
						  pStaCfg->wpa_supplicant_info.CFG_Tdls_info.BaseChannelBW);

	pStaCfg->wpa_supplicant_info.CFG_Tdls_info.bDoingPeriodChannelSwitch = FALSE;
	pStaCfg->wpa_supplicant_info.CFG_Tdls_info.bChannelSwitchInitiator = FALSE;
	RTMPCancelTimer(&pStaCfg->wpa_supplicant_info.CFG_Tdls_info.BaseChannelSwitchTimer, &TimerCancelled);
	return TRUE;
}


INT Set_CfgTdlsChannelSwitchRequest_Proc(
	IN  PRTMP_ADAPTER	pAd,
	IN  RTMP_STRING     *arg)
{
	ULONG i = 0;
	INT link_id = 0;
	UCHAR	peermacAddr[MAC_ADDR_LEN], TargetChannel = 0;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;
	/*
	 *	The Set_TdlsChannelSwitchProc inupt string format should be xx:xx:xx:xx:xx:xx-d,
	 *		=>The six 2 digit hex-decimal number previous are the Mac address,
	 *		=>The seventh decimal number is the channel value.
	 */

	if (strlen(arg) <
		19) /* Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and channel value in decimal format. */
		return FALSE;

	token = strchr(arg, DASH);

	if ((token != NULL) && (strlen(token) > 1)) {
		TargetChannel = (UCHAR) os_str_tol((token + 1), 0, 10);
		*token = '\0';

		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++) {
			if ((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token + 1))))
				return FALSE;

			AtoH(token, (&peermacAddr[i]), 1);
		}

		if (i != 6)
			return FALSE;

		link_id = cfg_tdls_search_ValidLinkIndex(pAd, peermacAddr);

		if (link_id == -1) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "\nCan't find TDLS Peer "MACSTR"\n",
					 MAC2STR(peermacAddr));
			return FALSE;
		}

		MTWF_PRINT("\nChannel Switch Peer "MACSTR"-%d\n", MAC2STR(peermacAddr), TargetChannel);
		/* pStaCfg->wpa_supplicant_info.CFG_Tdls_info.bDoingPeriodChannelSwitch = TRUE;  //Wait util rcv CHSW resp to set this true */
		pStaCfg->wpa_supplicant_info.CFG_Tdls_info.BaseChannel = wdev->channel;
		pStaCfg->wpa_supplicant_info.CFG_Tdls_info.BaseChannelBW = wlan_operate_get_ht_bw(wdev);
		pStaCfg->wpa_supplicant_info.CFG_Tdls_info.OrigTargetOffChannel = TargetChannel;
		pStaCfg->wpa_supplicant_info.CFG_Tdls_info.TargetOffChannel = TargetChannel;

		if (pStaCfg->wpa_supplicant_info.CFG_Tdls_info.TargetOffChannelBW != 0) {
			UCHAR Channel = TargetChannel;
			UCHAR target_ext_ch_offset = EXTCHA_NONE;

			if (Channel > 14) {
				if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
					(Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157)) {
					/* TDLS_InsertSecondaryChOffsetIE(pAd, (pOutBuffer + FrameLen), &FrameLen, EXTCHA_ABOVE); */
					target_ext_ch_offset = EXTCHA_ABOVE;
				} else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) ||
						   (Channel == 112) ||
						   (Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161)) {
					/* TDLS_InsertSecondaryChOffsetIE(pAd, (pOutBuffer + FrameLen), &FrameLen, EXTCHA_BELOW); */
					target_ext_ch_offset = EXTCHA_BELOW;
				}
			} else {
				do {
					if (Channel - 4 < 1)
						target_ext_ch_offset = EXTCHA_ABOVE;
					else if (Channel + 4 > 11)
						target_ext_ch_offset = EXTCHA_BELOW;
				} while (FALSE);
			}

			pStaCfg->wpa_supplicant_info.CFG_Tdls_info.TargetOffChannelExt = target_ext_ch_offset;

			if (target_ext_ch_offset == EXTCHA_BELOW)
				pStaCfg->wpa_supplicant_info.CFG_Tdls_info.TargetOffChannel = TargetChannel - 2;
			else
				pStaCfg->wpa_supplicant_info.CFG_Tdls_info.TargetOffChannel = TargetChannel + 2;
		}

		cfg_tdls_chsw_req(pAd, peermacAddr, pStaCfg->wpa_supplicant_info.CFG_Tdls_info.OrigTargetOffChannel
						  , pStaCfg->wpa_supplicant_info.CFG_Tdls_info.TargetOffChannelBW);
		pStaCfg->wpa_supplicant_info.CFG_Tdls_info.bChannelSwitchInitiator = TRUE;
	}

	return TRUE;
}
#endif /*CFG_TDLS_SUPPORT*/


/*
 *    ==========================================================================
 *    Description:
 *	Set Power Saving mode
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT Set_PSMode_Proc(RTMP_ADAPTER *pAdapter, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAdapter->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAdapter->StaCfg[pObj->ioctl_if];

	if (pObj->ioctl_if_type == INT_APCLI) {
		struct wifi_dev *wdev = &pStaCfg->wdev;
		MTWF_DBG(pAdapter, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"wdev(type=%d,if=%s)\n", wdev->wdev_type, wdev->if_dev->name);
	}

	if (pStaCfg->BssType == BSS_INFRA) {
		if ((strcmp(arg, "Max_PSP") == 0) ||
			(strcmp(arg, "max_psp") == 0) ||
			(strcmp(arg, "MAX_PSP") == 0)) {
			/* do NOT turn on PSM bit here, wait until MlmeCheckPsmChange() */
			/* to exclude certain situations. */
			if (pStaCfg->bWindowsACCAMEnable == FALSE)
				pStaCfg->WindowsPowerMode = Ndis802_11PowerModeMAX_PSP;

			pStaCfg->WindowsBatteryPowerMode = Ndis802_11PowerModeMAX_PSP;
			OPSTATUS_SET_FLAG(pAdapter, fOP_STATUS_RECEIVE_DTIM);
			pStaCfg->DefaultListenCount = 5;
		} else if ((strcmp(arg, "Fast_PSP") == 0) ||
				   (strcmp(arg, "fast_psp") == 0) ||
				   (strcmp(arg, "FAST_PSP") == 0)) {
			/* do NOT turn on PSM bit here, wait until MlmeCheckPsmChange() */
			/* to exclude certain situations. */
			OPSTATUS_SET_FLAG(pAdapter, fOP_STATUS_RECEIVE_DTIM);

			if (pStaCfg->bWindowsACCAMEnable == FALSE)
				pStaCfg->WindowsPowerMode = Ndis802_11PowerModeFast_PSP;

			pStaCfg->WindowsBatteryPowerMode = Ndis802_11PowerModeFast_PSP;
			pStaCfg->DefaultListenCount = 3;
		} else if ((strcmp(arg, "Legacy_PSP") == 0) ||
				   (strcmp(arg, "legacy_psp") == 0) ||
				   (strcmp(arg, "LEGACY_PSP") == 0)) {
			/* do NOT turn on PSM bit here, wait until MlmeCheckPsmChange() */
			/* to exclude certain situations. */
			OPSTATUS_SET_FLAG(pAdapter, fOP_STATUS_RECEIVE_DTIM);

			if (pStaCfg->bWindowsACCAMEnable == FALSE) {
				if (pStaCfg->WindowsPowerMode == Ndis802_11PowerModeCAM)
					RTMP_SLEEP_FORCE_AUTO_WAKEUP(pAdapter, pStaCfg);

				pStaCfg->WindowsPowerMode = Ndis802_11PowerModeLegacy_PSP;
			}

			pStaCfg->WindowsBatteryPowerMode = Ndis802_11PowerModeLegacy_PSP;
			pStaCfg->DefaultListenCount = 3;
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			pStaCfg->DefaultListenCount = 1;
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) // */
		} else {
			/*Default Ndis802_11PowerModeCAM */
			/* clear PSM bit immediately */
			RTMP_SET_PSM_BIT(pAdapter, pStaCfg, PWR_ACTIVE);
			OPSTATUS_SET_FLAG(pAdapter, fOP_STATUS_RECEIVE_DTIM);

			if (pStaCfg->bWindowsACCAMEnable == FALSE) {
				if (pStaCfg->WindowsPowerMode != Ndis802_11PowerModeCAM) {
					RTMP_FORCE_WAKEUP(pAdapter, pStaCfg);
					pStaCfg->WindowsPowerMode = Ndis802_11PowerModeCAM;
				}
			}

			pStaCfg->WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
		}

		MTWF_PRINT("PSMode=%ld\n", pStaCfg->WindowsPowerMode);
	} else
		return FALSE;

	return TRUE;
}

#ifdef WPA_SUPPLICANT_SUPPORT
/*
 *    ==========================================================================
 *    Description:
 *	Set WpaSupport flag.
 *    Value:
 *	0: Driver ignore wpa_supplicant.
 *	1: wpa_supplicant initiates scanning and AP selection.
 *	2: driver takes care of scanning, AP selection, and IEEE 802.11 association parameters.
 *    Return:
 *	TRUE if all parameters are OK, FALSE otherwise
 *    ==========================================================================
 */
INT Set_Wpa_Support(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	if (os_str_tol(arg, 0, 10) == 0)
		pStaCfg->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_DISABLE;
	else if (os_str_tol(arg, 0, 10) == 1)
		pStaCfg->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE;
	else if (os_str_tol(arg, 0, 10) == 2)
		pStaCfg->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE_WITH_WEB_UI;
	else
		pStaCfg->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_DISABLE;

	MTWF_PRINT("WpaSupplicantUP=%d\n", pStaCfg->wpa_supplicant_info.WpaSupplicantUP);

	return TRUE;
}
#endif /* WPA_SUPPLICANT_SUPPORT */


#ifdef WSC_STA_SUPPORT
#define WSC_GET_CONF_MODE_EAP	1
#define WSC_GET_CONF_MODE_UPNP	2
INT	 Set_WscConfMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PWSC_CTRL	pWscControl;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	pWscControl = &pStaCfg->wdev.WscControl;

	if (os_str_tol(arg, 0, 10) == 0)
		pWscControl->WscConfMode = WSC_DISABLE;
	else if (os_str_tol(arg, 0, 10) == 1)
		pWscControl->WscConfMode = WSC_ENROLLEE;
	else if (os_str_tol(arg, 0, 10) == 2)
		pWscControl->WscConfMode = WSC_REGISTRAR;
	else
		pWscControl->WscConfMode = WSC_DISABLE;

	MTWF_PRINT("WscConfMode(0,1,2)=%d\n", pWscControl->WscConfMode);
#ifdef IWSC_SUPPORT
#ifdef IWSC_TEST_SUPPORT

	if (pStaCfg->BssType == BSS_ADHOC) {
		PIWSC_INFO pIWscInfo = &pStaCfg->IWscInfo;

		pIWscInfo->IWscConfMode = WSC_DISABLE;

		if (os_str_tol(arg, 0, 10) == 1)
			pIWscInfo->IWscConfMode = WSC_ENROLLEE;
		else if (os_str_tol(arg, 0, 10) == 2)
			pIWscInfo->IWscConfMode = WSC_REGISTRAR;
		else
			return FALSE;

		MTWF_PRINT("IWscConfMode=%d\n", pIWscInfo->IWscConfMode));
	}

#endif /* IWSC_TEST_SUPPORT */
#endif /* IWSC_SUPPORT */
	return TRUE;
}

INT	Set_WscConfStatus_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR     IsAPConfigured = 0;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PWSC_CTRL	pWscControl;
	UINT32 staidx = 0;

	if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pObj->ioctl_if is invalid value\n");
		return FALSE;
	}

	staidx = pObj->ioctl_if;
	pWscControl = &pAd->StaCfg[staidx].wdev.WscControl;
	IsAPConfigured = (UCHAR)os_str_tol(arg, 0, 10);

	if ((IsAPConfigured  > 0) && (IsAPConfigured  <= 2))
		pWscControl->WscConfStatus = IsAPConfigured;
	else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Set failed!!(WscConfStatus=%s), WscConfStatus is 1 or 2\n", arg);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"WscConfStatus is not changed (%d)\n", pWscControl->WscConfStatus);
		return FALSE;  /*Invalid argument */
	}

	MTWF_PRINT("WscConfStatus=%d\n", pWscControl->WscConfStatus);
	return TRUE;
}

INT Set_WscSsid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PWSC_CTRL	pWscControl;
	ULONG		ApIdx = 0;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	pWscControl = &wdev->WscControl;
	NdisZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));

	if ((strlen(arg) > 0) && (strlen(arg) <= MAX_LEN_OF_SSID)) {
		NdisMoveMemory(pWscControl->WscSsid.Ssid, arg, strlen(arg));
		pWscControl->WscSsid.SsidLength = strlen(arg);
		/* add SSID into pStaCfg->MlmeAux.Ssid */
		NdisZeroMemory(pStaCfg->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pStaCfg->MlmeAux.Ssid, arg, strlen(arg));
		pStaCfg->MlmeAux.SsidLen = strlen(arg);
		NdisZeroMemory(pWscControl->WscBssid, MAC_ADDR_LEN);
		ApIdx = WscSearchWpsApBySSID(pAd,
									 pWscControl->WscSsid.Ssid,
									 pWscControl->WscSsid.SsidLength,
									 WSC_PIN_MODE,
									 &pStaCfg->wdev);

		if (ApIdx != BSS_NOT_FOUND) {
			NdisMoveMemory(pWscControl->WscBssid, ScanTab->BssEntry[ApIdx].Bssid, MAC_ADDR_LEN);
			pStaCfg->MlmeAux.Channel = ScanTab->BssEntry[ApIdx].Channel;
		}

		hex_dump("Set_WscSsid_Proc:: WscBssid", pWscControl->WscBssid, MAC_ADDR_LEN);
		MTWF_PRINT("Select SsidLen=%d,Ssid=%s\n",
				 pWscControl->WscSsid.SsidLength, pWscControl->WscSsid.Ssid);
	} else
		return FALSE;	/*Invalid argument */

	return TRUE;
}


INT Set_WscBssid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR					MacAddr[MAC_ADDR_LEN];
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	if (rtstrmactohex(arg, (RTMP_STRING *) &MacAddr[0]) == FALSE)
		return FALSE;

	RTMPZeroMemory(pStaCfg->wdev.WscControl.WscBssid, MAC_ADDR_LEN);
	RTMPMoveMemory(pStaCfg->wdev.WscControl.WscBssid, MacAddr, MAC_ADDR_LEN);
	MTWF_PRINT(""MACSTR"\n", MAC2STR(MacAddr));
	return TRUE;
}

INT	Set_WscMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	        WscMode;
	PWSC_CTRL	pWscControl;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	pWscControl = &pStaCfg->wdev.WscControl;
	WscMode = (INT)os_str_tol(arg, 0, 10);

	if ((WscMode == WSC_SMPBC_MODE) &&
		(pStaCfg->BssType == BSS_INFRA)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Set failed!!(Set_WscMode_Proc=%s)\n", arg);
		return FALSE;  /*Invalid argument */
	}

	if ((WscMode == WSC_PIN_MODE) ||
		(WscMode == WSC_PBC_MODE) ||
		(WscMode == WSC_SMPBC_MODE)) {
		/* save wsc mode */
		pWscControl->WscMode = WscMode;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Set failed!!(Set_WscMode_Proc=%s)\n", arg);
		return FALSE;  /*Invalid argument */
	}

	MTWF_PRINT("WscMode=%d\n", pWscControl->WscMode);
	return TRUE;
}

INT	Set_WscPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PWSC_CTRL	pWscControl;
	BOOLEAN     validatePin;
	UINT        PinCode = 0;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	pWscControl = &pStaCfg->wdev.WscControl;
	PinCode = os_str_tol(arg, 0, 10); /* When PinCode is 03571361, return value is 3571361. */

	if (strlen(arg) == 4)
		validatePin = TRUE;
	else
		validatePin = ValidateChecksum(PinCode);

	if (validatePin) {
		if (pWscControl->WscRejectSamePinFromEnrollee &&
			(PinCode == pWscControl->WscLastPinFromEnrollee)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"PIN auth or comm error occurs!\n"
				"NOT accept the same PIN again!(PIN:%08u)\n", PinCode);
			return FALSE;
		} else {
			pWscControl->WscRejectSamePinFromEnrollee = FALSE;
			pWscControl->WscPinCode = PinCode;
			pWscControl->WscLastPinFromEnrollee = pWscControl->WscPinCode;

			if (strlen(arg) == 4)
				pWscControl->WscPinCodeLen = 4;
			else
				pWscControl->WscPinCodeLen = 8;

			WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl);
#ifdef IWSC_SUPPORT

			if ((pStaCfg->BssType == BSS_ADHOC) &&
				(pWscControl->bWscTrigger == TRUE)) {
				MlmeEnqueue(pAd, IWSC_STATE_MACHINE, IWSC_MT2_PEER_PIN, 0, NULL, 0);
				RTMP_MLME_HANDLER(pAd);
			}

#endif /* IWSC_SUPPORT */
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Checksum is invalid\n");
		return FALSE;
	}

	MTWF_PRINT("PinCode=%d\n", pWscControl->WscPinCode);
	return TRUE;
}

INT	Set_WscUUIDE_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PWSC_CTRL           pWscControl;
	int i, UUIDLen, ret;
	UCHAR				tmp_Uuid_Str[UUID_LEN_STR];
	UCHAR				Wsc_Uuid_E[UUID_LEN_HEX];
	UCHAR				uuidTmpStr[UUID_LEN_STR + 2];
	WSC_UUID_T uuid_t;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	UUIDLen = strlen(arg);

	if (UUIDLen != 32) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Length of UUID key is wrong length=%d\n", UUIDLen);
		return FALSE;
	}

	NdisMoveMemory(tmp_Uuid_Str, arg, UUIDLen);
	pWscControl = &pStaCfg->wdev.WscControl;

	for (i = 0; i < UUIDLen; i++) {
		if (!isxdigit(tmp_Uuid_Str[i])) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Input %d not Hex Value\n", i);
			return FALSE;  /* Not Hex value; */
		}
	}

	AtoH(tmp_Uuid_Str, Wsc_Uuid_E, UUID_LEN_HEX);
	NdisMoveMemory(&uuid_t, Wsc_Uuid_E, UUID_LEN_HEX);
	NdisZeroMemory(uuidTmpStr, sizeof(uuidTmpStr));
	ret = snprintf(uuidTmpStr, sizeof(uuidTmpStr), "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			(unsigned int)uuid_t.timeLow, uuid_t.timeMid, uuid_t.timeHi_Version, uuid_t.clockSeqHi_Var, uuid_t.clockSeqLow,
			uuid_t.node[0], uuid_t.node[1], uuid_t.node[2], uuid_t.node[3], uuid_t.node[4], uuid_t.node[5]);
	if (os_snprintf_error(sizeof(uuidTmpStr), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "uuidTmpStr snprintf error!\n");
		return FALSE;
		}

	if (strlen(uuidTmpStr) > UUID_LEN_STR)
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "WARN:UUID String size too large!\n");

	strncpy((RTMP_STRING *)pWscControl->Wsc_Uuid_Str, uuidTmpStr, UUID_LEN_STR);
	pWscControl->Wsc_Uuid_Str[UUID_LEN_STR - 1] = '\0';

	NdisMoveMemory(&pWscControl->Wsc_Uuid_E[0], Wsc_Uuid_E, UUID_LEN_HEX);

	MTWF_PRINT("The UUID Hex string is:");
	for (i = 0; i < 16; i++)
		MTWF_PRINT("%02x", (pWscControl->Wsc_Uuid_E[i] & 0xff));
	MTWF_PRINT("\nThe UUID ASCII string is:%s!\n", pWscControl->Wsc_Uuid_Str);

	return 0;
}

INT	Set_WscGetConf_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PWSC_CTRL           pWscControl;
	PWSC_UPNP_NODE_INFO pWscUPnPNodeInfo;
	INT	                idx;
	BOOLEAN             StateMachineTouched = FALSE;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
#ifdef WSC_LED_SUPPORT
	UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
	pWscControl = &pStaCfg->wdev.WscControl;
#ifdef IWSC_SUPPORT

	if (pStaCfg->BssType == BSS_ADHOC) {
		if (pWscControl->bWscTrigger)
			IWSC_Stop(pAd, TRUE);
		else
			IWSC_Stop(pAd, FALSE);

		if (STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED) &&
			(pStaCfg->IWscInfo.IpMethod == IWSC_IPV4_ASSIGNMENT) &&
			(pStaCfg->IWscInfo.RegDepth != 0) &&
			(pStaCfg->IWscInfo.AvaSubMaskListCount == 0)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Empty Available IPv4 Submask List. Cannot be Registrar\n");
			pWscControl->WscStatus = STATUS_WSC_EMPTY_IPV4_SUBMASK_LIST;
			return TRUE;
		}

		MlmeEnqueue(pAd, IWSC_STATE_MACHINE, IWSC_MT2_MLME_START, 0, NULL, 0);
		RTMP_MLME_HANDLER(pAd);
		return TRUE;
	}

#endif /* IWSC_SUPPORT */
	pWscUPnPNodeInfo = &pWscControl->WscUPnPNodeInfo;

	if (pWscControl->WscConfMode == WSC_DISABLE) {
		pWscControl->bWscTrigger = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Set_WscGetConf_Proc: WPS is disabled.\n");
		return FALSE;
	}

	WscStop(pAd,
#ifdef CONFIG_AP_SUPPORT
			FALSE,
#endif /* CONFIG_AP_SUPPORT */
			pWscControl);
	/* trigger wsc re-generate public key */
	pWscControl->RegData.ReComputePke = 1;
	/* Change to init state before sending a disassociation frame */
	pWscControl->WscState = WSC_STATE_INIT;

	/* 0. Send a disassoication frame */
	if (INFRA_ON(pStaCfg)) {

		/* Set to immediately send the media disconnect event */
		pStaCfg->MlmeAux.CurrReqIsFromNdis = TRUE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "disassociate with current AP before starting WPS\n");
		cntl_disconnect_request(&pStaCfg->wdev,
								CNTL_DISASSOC,
								pStaCfg->Bssid,
								REASON_DISASSOC_STA_LEAVING);

		/* Set the AutoReconnectSsid to prevent it reconnect to old SSID */
		pStaCfg->MlmeAux.AutoReconnectSsidLen = 32;
		NdisZeroMemory(pStaCfg->MlmeAux.AutoReconnectSsid, pStaCfg->MlmeAux.AutoReconnectSsidLen);
		pWscControl->bWscTrigger = FALSE;	/* check to disable */
		OS_WAIT(500);  /* leave enough time for this DISASSOC frame */
	} else if (ADHOC_ON(pAd)) {
		USHORT	TmpWscMode;
		/*
		 *	Set the AutoReconnectSsid to prevent it reconnect to old SSID
		 *	Since calling this indicate user don't want to connect to that SSID anymore.
		 */
		pStaCfg->MlmeAux.AutoReconnectSsidLen = 32;
		NdisZeroMemory(pStaCfg->MlmeAux.AutoReconnectSsid, pStaCfg->MlmeAux.AutoReconnectSsidLen);

		if (pWscControl->WscMode == 1)
			TmpWscMode = DEV_PASS_ID_PIN;
		else
			TmpWscMode = DEV_PASS_ID_PBC;

		AsicDisableSync(pAd, HW_BSSID_0);
		WscBuildBeaconIE(pAd, pWscControl->WscConfStatus, TRUE, TmpWscMode, pWscControl->WscConfigMethods, BSS0, NULL, 0,
						 STA_MODE);

		if (pWscControl->WscConfMode == WSC_REGISTRAR) {
			WscBuildProbeRespIE(pAd,
								WSC_MSGTYPE_REGISTRAR,
								pWscControl->WscConfStatus,
								TRUE,
								TmpWscMode,
								pWscControl->WscConfigMethods,
								BSS0,
								NULL,
								0,
								STA_MODE);
			UpdateBeaconHandler(
				pAd,
				&pStaCfg->wdev,
				BCN_UPDATE_IF_STATE_CHG);
			AsicEnableIbssSync(
				pAd,
				pAd->CommonCfg.BeaconPeriod[DBDC_BAND0],
				HW_BSSID_0,
				OPMODE_ADHOC);
		} else {
			WscBuildProbeRespIE(pAd,
								WSC_MSGTYPE_ENROLLEE_INFO_ONLY,
								pWscControl->WscConfStatus,
								TRUE,
								TmpWscMode,
								pWscControl->WscConfigMethods,
								BSS0,
								NULL,
								0,
								STA_MODE);
			LinkDown(pAd, FALSE, &pStaCfg->wdev, NULL);
		}
	}

	pWscControl->bWscTrigger = TRUE;
	pWscControl->WscConfStatus = WSC_SCSTATE_UNCONFIGURED;

	/* */
	/* Action : PIN, PBC */
	/* */
	if (pWscControl->WscMode == 1) {
		if ((pStaCfg->BssType == BSS_INFRA) &&
			(pWscControl->WscConfMode == WSC_ENROLLEE)) {
			/*BSSID is set from UI, so do not reset*/
			/* pWscControl->WscSsid.SsidLength = 0; */
			/* NdisZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID)); */
			/* RTMPZeroMemory(pWscControl->WscBssid, MAC_ADDR_LEN); */
			pWscControl->WscPINBssCount = 0;
			pWscControl->ScanCountToincludeWPSPin1 = 0;
			/* WPS - SW PIN */
			WscPINAction(pAd, pWscControl);
			StateMachineTouched = TRUE;
		} else {
			INT		WaitCnt = 0;
			/* PIN  - default */

			/* 2. Enqueue BSSID/SSID connection command */
			WscInitRegistrarPair(pAd, pWscControl, BSS0);
			/*
			 *	We need to make sure target AP is in the scan table.
			 */
			pStaCfg->bSkipAutoScanConn = TRUE;

			while ((scan_in_run_state(pAd, &pStaCfg->wdev) == TRUE) && (WaitCnt++ < 200))
				OS_WAIT(500);

			pStaCfg->MlmeAux.AutoReconnectSsidLen = 0;
			pStaCfg->bConfigChanged = TRUE;
			cntl_connect_request(&pStaCfg->wdev,
								 CNTL_CONNECT_BY_BSSID, MAC_ADDR_LEN, pStaCfg->wdev.WscControl.WscBssid);

			pWscControl->WscState = WSC_STATE_START;
			StateMachineTouched = TRUE;
			RTMPSetTimer(&pWscControl->Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
			pWscControl->Wsc2MinsTimerRunning = TRUE;
			pWscControl->WscStatus = STATUS_WSC_LINK_UP;
			/*WscSendUPnPConfReqMsg(pAd, apidx, pAd->ApCfg.MBSSID[apidx].Ssid, pAd->ApCfg.MBSSID[apidx].Bssid, 3, 0); */
		}
	} else {
		if ((pStaCfg->BssType == BSS_INFRA) ||
			(pWscControl->WscConfMode == WSC_ENROLLEE)) {
			pWscControl->WscSsid.SsidLength = 0;
			NdisZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));
			RTMPZeroMemory(pWscControl->WscBssid, MAC_ADDR_LEN);
			pWscControl->WscPBCBssCount = 0;
			/* WPS - SW PBC */
			WscPushPBCAction(pAd, pWscControl);
			StateMachineTouched = TRUE;
		} else {
			WscInitRegistrarPair(pAd, pWscControl, BSS0);
			WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl);
			RTMPSetTimer(&pWscControl->Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
			pWscControl->Wsc2MinsTimerRunning = TRUE;
			pWscControl->WscState = WSC_STATE_LINK_UP;
		}
	}

#ifdef WSC_LED_SUPPORT
#ifdef CONFIG_WIFI_LED_SUPPORT
	/* Change FW default mode to WPS LED share mode*/
	pAd->LedCntl.MCULedCntl.word &= 0x80;
	pAd->LedCntl.MCULedCntl.word |= WPS_LED_MODE_SHARE;

	if (LED_MODE(pAd) == WPS_LED_MODE_SHARE) {
		WPSLEDStatus = LED_WPS_PRE_STAGE;
		RTMPSetTimer(&pWscControl->WscLEDTimer, WSC_WPS_PREPOST_WIFI_LED_TIMEOUT);
	} else
#endif /* CONFIG_WIFI_LED_SUPPORT */
		WPSLEDStatus = LED_WPS_IN_PROCESS;

	RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
#endif /* WSC_LED_SUPPORT */

	/* Enrollee 192 random bytes for DH key generation */
	for (idx = 0; idx < 192; idx++)
		pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAd);

	if (pWscControl->WscProfileRetryTimerRunning) {
		BOOLEAN Cancelled;

		pWscControl->WscProfileRetryTimerRunning = FALSE;
		RTMPCancelTimer(&pWscControl->WscProfileRetryTimer, &Cancelled);
	}

	if (StateMachineTouched) /* Upper layer sent a MLME-related operations */
		RTMP_MLME_HANDLER(pAd);

	MTWF_PRINT("Trigger WSC state machine\n");

	return TRUE;
}

#ifdef WSC_V2_SUPPORT
INT Set_WscForceSetAP_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	PWSC_V2_INFO pWscV2Info = &pStaCfg->wdev.WscControl.WscV2Info;

	if (os_str_tol(arg, 0, 10) == 0)
		pWscV2Info->bForceSetAP = FALSE;
	else
		pWscV2Info->bForceSetAP = TRUE;

	MTWF_PRINT("bForceSetAP=%d\n", pWscV2Info->bForceSetAP);

	return TRUE;
}
#endif /* WSC_V2_SUPPORT */


#ifdef IWSC_SUPPORT
INT	Set_IWscLimitedUI_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PWSC_CTRL	pWscControl;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	pWscControl = &pStaCfg->wdev.WscControl;

	if (os_str_tol(arg, 0, 10) == 0)
		pStaCfg->IWscInfo.bLimitedUI = FALSE;
	else
		pStaCfg->IWscInfo.bLimitedUI = TRUE;

	MTWF_PRINT("bLimitedUI=%d\n", pStaCfg->IWscInfo.bLimitedUI);
	return TRUE;
}

#ifdef IWSC_TEST_SUPPORT
INT	Set_IWscDefaultSecurity_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8		TmpValue;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	TmpValue = (UINT8)os_str_tol(arg, 0, 10);

	if ((TmpValue != 0) && (TmpValue <= 3)) {
		pStaCfg->IWscInfo.IWscDefaultSecurity = TmpValue;
		MTWF_PRINT("IWscDefaultSecurity=%d\n", pStaCfg->IWscInfo.IWscDefaultSecurity);
		return TRUE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Value=%d\n", TmpValue);
		return FALSE;
	}
}

INT	Set_IWscSmpbcEnrScanOnly_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PIWSC_INFO pIWscInfo;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	pIWscInfo = &pStaCfg->IWscInfo;

	if (os_str_tol(arg, 0, 10) == 0)
		pIWscInfo->bIwscSmpbcScanningOnly = FALSE;
	else
		pIWscInfo->bIwscSmpbcScanningOnly = TRUE;
}

INT	Set_IWscEmptySubmaskList_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PIWSC_INFO pIWscInfo;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	pIWscInfo = &pStaCfg->IWscInfo;

	if (os_str_tol(arg, 0, 10) == 0)
		pIWscInfo->bEmptySubmaskList = FALSE;
	else
		pIWscInfo->bEmptySubmaskList = TRUE;
}

INT	Set_IWscBlockConnection_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PIWSC_INFO pIWscInfo;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	pIWscInfo = &pStaCfg->IWscInfo;

	if (os_str_tol(arg, 0, 10) == 0)
		pIWscInfo->bBlockConnection = FALSE;
	else
		pIWscInfo->bBlockConnection = TRUE;

	if (pIWscInfo->bBlockConnection) {
		INT i;

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];

			if (!IS_ENTRY_CLIENT(pEntry))
				continue;

			MlmeDeAuthAction(pAd, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
		}

		if (pAd->MacTab.Size == 0) {
			STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED);
			pAd->IndicateMediaState = NdisMediaStateDisconnected;
			RTMP_IndicateMediaState(pAd);
		}
	}
}
#endif /* IWSC_TEST_SUPPORT */

INT	Set_IWscOOB_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PWSC_CTRL	pWscControl;
	NDIS_802_11_SSID	Ssid;
	USHORT WpaPskLen = 0;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	if (pStaCfg->BssType != BSS_ADHOC) {
		if (INFRA_ON(pStaCfg)) {
			/*BOOLEAN Cancelled; */
			/* Set the AutoReconnectSsid to prevent it reconnect to old SSID */
			/* Since calling this indicate user don't want to connect to that SSID anymore. */
			pStaCfg->MlmeAux.AutoReconnectSsidLen = 32;
			NdisZeroMemory(pStaCfg->MlmeAux.AutoReconnectSsid, pStaCfg->MlmeAux.AutoReconnectSsidLen);
			LinkDown(pAd, FALSE, &pStaCfg->wdev, NULL);
		}

#ifdef DOT11_N_SUPPORT
		SetCommonHtVht(pAd, &pStaCfg->wdev);
#endif /* DOT11_N_SUPPORT */
		pStaCfg->BssType = BSS_ADHOC;
	}

	pWscControl = &pStaCfg->wdev.WscControl;
	pWscControl->WscConfStatus = WSC_SCSTATE_UNCONFIGURED;
	pWscControl->WscConfMode = WSC_DISABLE;
	pWscControl->WscState = WSC_STATE_OFF;
	pWscControl->WscStatus = STATUS_WSC_IDLE;
	pWscControl->bWscTrigger = FALSE;
	NdisZeroMemory(&pWscControl->WscProfile, sizeof(WSC_PROFILE));
	sprintf(&Ssid.Ssid[0], "IWSC%02X%02X%02X", RandomByte(pAd), RandomByte(pAd), RandomByte(pAd));
	Ssid.SsidLength = strlen(&Ssid.Ssid[0]);
	WscGenRandomKey(pAd, pWscControl, pWscControl->WpaPsk, &WpaPskLen);
	pWscControl->WpaPskLen = (INT)WpaPskLen;
	NdisZeroMemory(pStaCfg->WpaPassPhrase, 64);
	NdisMoveMemory(pStaCfg->WpaPassPhrase, pWscControl->WpaPsk, pWscControl->WpaPskLen);
	pStaCfg->WpaPassPhraseLen = pWscControl->WpaPskLen;
	pStaCfg->wdev.AuthMode = Ndis802_11AuthModeWPA2PSK;
	pStaCfg->wdev.WepStatus = Ndis802_11AESEnable;

	if ((pStaCfg->WpaPassPhraseLen >= 8) &&
		(pStaCfg->WpaPassPhraseLen <= 64)) {
		UCHAR keyMaterial[40];

		RTMPZeroMemory(pStaCfg->PMK, 32);

		if (pStaCfg->WpaPassPhraseLen == 64)
			AtoH((RTMP_STRING *) pStaCfg->WpaPassPhrase, pStaCfg->PMK, 32);
		else {
			RtmpPasswordHash((RTMP_STRING *) pStaCfg->WpaPassPhrase, Ssid.Ssid, Ssid.SsidLength, keyMaterial);
			NdisMoveMemory(pStaCfg->PMK, keyMaterial, 32);
		}
	}

	pStaCfg->MlmeAux.CurrReqIsFromNdis = TRUE;
	pStaCfg->bConfigChanged = TRUE;
	pStaCfg->IWscInfo.bDoNotChangeBSSID = FALSE;
	MlmeEnqueue(pAd,
				MLME_CNTL_STATE_MACHINE,
				OID_802_11_SSID,
				sizeof(NDIS_802_11_SSID),
				(VOID *)&Ssid, 0);
	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}

INT	Set_IWscSinglePIN_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	if (os_str_tol(arg, 0, 10) == 0)
		pStaCfg->IWscInfo.bSinglePIN = FALSE;
	else
		pStaCfg->IWscInfo.bSinglePIN = TRUE;

	MTWF_PRINT("bSinglePIN=%d\n", pStaCfg->IWscInfo.bSinglePIN);
	return TRUE;
}
#endif /* IWSC_SUPPORT */

#endif /* WSC_STA_SUPPORT */

INT Set_TGnWifiTest_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	if (os_str_tol(arg, 0, 10) == 0)
		pStaCfg->bTGnWifiTest = FALSE;
	else
		pStaCfg->bTGnWifiTest = TRUE;

	MTWF_PRINT("bTGnWifiTest=%d\n", pStaCfg->bTGnWifiTest);
	return TRUE;
}

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_Ieee80211dClientMode_Proc(RTMP_ADAPTER *pAdapter, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAdapter->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAdapter->StaCfg[pObj->ioctl_if];

	if (os_str_tol(arg, 0, 10) == 0)
		pStaCfg->IEEE80211dClientMode = Rt802_11_D_None;
	else if (os_str_tol(arg, 0, 10) == 1)
		pStaCfg->IEEE80211dClientMode = Rt802_11_D_Flexible;
	else if (os_str_tol(arg, 0, 10) == 2)
		pStaCfg->IEEE80211dClientMode = Rt802_11_D_Strict;
	else
		return FALSE;

	MTWF_PRINT("IEEEE0211dMode=%d\n", pStaCfg->IEEE80211dClientMode);
	return TRUE;
}
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef CARRIER_DETECTION_SUPPORT
INT Set_StaCarrierDetect_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	if (os_str_tol(arg, 0, 10) == 0)
		pAd->CommonCfg.CarrierDetect.Enable = FALSE;
	else
		pAd->CommonCfg.CarrierDetect.Enable = TRUE;

	MTWF_PRINT("CarrierDetect.Enable=%d\n", pAd->CommonCfg.CarrierDetect.Enable);
	return TRUE;
}
#endif /* CARRIER_DETECTION_SUPPORT */

INT	Show_Adhoc_MacTable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *extra,
	IN	UINT32			size)
{
	INT i, ret;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;
	ADD_HT_INFO_IE *addht = wlan_operate_get_addht(wdev);
	UINT32 extra_left;

	extra_left = size - strlen(extra);
	ret = snprintf(extra, extra_left, "\n");
	if (os_snprintf_error(extra_left, ret))
		goto error;

#ifdef DOT11_N_SUPPORT
	extra_left = size - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "HT Operating Mode : %d\n", addht->AddHtInfo2.OperaionMode);
	if (os_snprintf_error(extra_left, ret))
		goto error;
#endif /* DOT11_N_SUPPORT */
	extra_left = size - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "\n%-19s%-4s%-4s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s\n",
			"MAC", "AID", "BSS", "RSSI0", "RSSI1", "RSSI2", "PhMd", "BW", "MCS", "SGI", "STBC");
	if (os_snprintf_error(extra_left, ret))
		goto error;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (strlen(extra) > (size - 30))
			break;

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry)) && (pEntry->Sst == SST_ASSOC)) {
			extra_left = size - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "%02X:%02X:%02X:%02X:%02X:%02X  ",
					pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
					pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]);
			if (os_snprintf_error(extra_left, ret))
				goto error;
			extra_left = size - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "%-4d", (int)pEntry->Aid);
			if (os_snprintf_error(extra_left, ret))
				goto error;
			extra_left = size - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "%-4d", (int)pEntry->func_tb_idx);
			if (os_snprintf_error(extra_left, ret))
				goto error;
			extra_left = size - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "%-7d", pEntry->RssiSample.AvgRssi[0]);
			if (os_snprintf_error(extra_left, ret))
				goto error;
			extra_left = size - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "%-7d", pEntry->RssiSample.AvgRssi[1]);
			if (os_snprintf_error(extra_left, ret))
				goto error;
			extra_left = size - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "%-7d", pEntry->RssiSample.AvgRssi[2]);
			if (os_snprintf_error(extra_left, ret))
				goto error;
			extra_left = size - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "%-10s",
												get_phymode_str(pEntry->HTPhyMode.field.MODE));
			if (os_snprintf_error(extra_left, ret))
				goto error;
			extra_left = size - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "%-6s", get_bw_str(pEntry->HTPhyMode.field.BW));
			if (os_snprintf_error(extra_left, ret))
				goto error;
			extra_left = size - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "%-6d", pEntry->HTPhyMode.field.MCS);
			if (os_snprintf_error(extra_left, ret))
				goto error;
			extra_left = size - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "%-6d", pEntry->HTPhyMode.field.ShortGI);
			if (os_snprintf_error(extra_left, ret))
				goto error;
			extra_left = size - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "%-6d", pEntry->HTPhyMode.field.STBC);
			if (os_snprintf_error(extra_left, ret))
				goto error;
			extra_left = size - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "%-10d, %d, %d%%\n",
					pEntry->DebugFIFOCount, pEntry->DebugTxCount,
					(pEntry->DebugTxCount) ? ((pEntry->DebugTxCount - pEntry->DebugFIFOCount) * 100 /
					pEntry->DebugTxCount) : 0);
			if (os_snprintf_error(extra_left, ret))
				goto error;
			extra_left = size - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "\n");
			if (os_snprintf_error(extra_left, ret))
				goto error;
		}
	}

	return TRUE;

error:
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Show_Adhoc_MacTable_Proc snprintf error!\n");
	return FALSE;
}


INT Set_BeaconLostTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	ULONG ltmp = (ULONG)os_str_tol(arg, 0, 10);

	if ((ltmp != 0) && (ltmp <= 60))
		pStaCfg->BeaconLostTime = (ltmp * OS_HZ);

	MTWF_PRINT("BeaconLostTime=%ld\n", pStaCfg->BeaconLostTime);
	return TRUE;
}

INT Set_AutoRoaming_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	if (os_str_tol(arg, 0, 10) == 0)
		pStaCfg->bAutoRoaming = FALSE;
	else
		pStaCfg->bAutoRoaming = TRUE;

	MTWF_PRINT("bAutoRoaming=%d\n", pStaCfg->bAutoRoaming);
	return TRUE;
}


/*
 *    ==========================================================================
 *    Description:
 *	Issue a site survey command to driver
 *	Arguments:
 *	    pAdapter                    Pointer to our adapter
 *	    wrq                         Pointer to the ioctl argument
 *
 *    Return Value:
 *	None
 *
 *    Note:
 *	Usage:
 *	       1.) iwpriv ra0 set site_survey
 *    ==========================================================================
 */

INT Set_ForceTxBurst_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	if (os_str_tol(arg, 0, 10) == 0)
		pStaCfg->bForceTxBurst = FALSE;
	else
		pStaCfg->bForceTxBurst = TRUE;

	MTWF_PRINT("bForceTxBurst=%d\n", pStaCfg->bForceTxBurst);
	return TRUE;
}




#ifdef ETH_CONVERT_SUPPORT
#ifdef IP_ASSEMBLY
INT Set_FragTest_Proc(RTMP_ADAPTER *pAdapter, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAdapter->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAdapter->StaCfg[pObj->ioctl_if];

	if (os_str_tol(arg, 0, 10) == 0)
		pStaCfg->bFragFlag = FALSE;
	else
		pStaCfg->bFragFlag = TRUE;

	return TRUE;
}
#endif /* IP_ASSEMBLY */
#endif /* ETH_CONVERT_SUPPORT */

VOID RTMPAddKey(RTMP_ADAPTER *pAd, PNDIS_802_11_KEY pKey, struct wifi_dev *wdev)
{
}


INT Set_AutoReconnect_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	if (os_str_tol(arg, 0, 10) == 0)
		pStaCfg->bAutoReconnect = FALSE;
	else
		pStaCfg->bAutoReconnect = TRUE;

	MTWF_PRINT("bAutoReconnect=%d\n", pStaCfg->bAutoReconnect);
	return TRUE;
}

INT Set_AdhocN_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct adhoc_info *adhocInfo = &pStaCfg->adhocInfo;
#ifdef DOT11_N_SUPPORT

	if (os_str_tol(arg, 0, 10) == 0)
		adhocInfo->bAdhocN = FALSE;
	else
		adhocInfo->bAdhocN = TRUE;

	MTWF_PRINT("bAdhocN=%d\n", adhocInfo->bAdhocN);
#endif /* DOT11_N_SUPPORT */
	return TRUE;
}


#ifdef CONFIG_MULTI_CHANNEL
INT Set_CH1StayTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	pStaCfg->channel_1st_staytime = (UINT32)os_str_tol(arg, 0, 10);
	pStaCfg->channel_1st_staytime = (pStaCfg->channel_1st_staytime - pStaCfg->switch_idle_time);
	MTWF_PRINT("channel_1st_staytime=%u, switch_idle_time=%u\n",
		pStaCfg->channel_1st_staytime, pStaCfg->switch_idle_time);
	return TRUE;
}

INT Set_CH2StayTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	pStaCfg->channel_2nd_staytime = (UINT32)os_str_tol(arg, 0, 10);
	pStaCfg->channel_2nd_staytime = (pStaCfg->channel_2nd_staytime - pStaCfg->switch_idle_time);
	MTWF_PRINT("channel_2nd_staytime=%u, switch_idle_time=%u\n",
			  pStaCfg->channel_2nd_staytime, pStaCfg->switch_idle_time);
	return TRUE;
}


INT Set_SwitchIdleTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pStaCfg->switch_idle_time = (UINT32)os_str_tol(arg, 0, 10);
	MTWF_PRINT("switch_idle_time=%u\n", pStaCfg->switch_idle_time);
	return TRUE;
}


INT Set_NULL_Frame_Count_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pStaCfg->null_frame_count = (UINT32)os_str_tol(arg, 0, 10);
	MTWF_PRINT("null_frame_count=%u\n", pStaCfg->null_frame_count);
	return TRUE;
}

INT	Set_CH1_BW_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pStaCfg->channel_1st_bw = (UINT32)os_str_tol(arg, 0, 10);
	MTWF_PRINT("channel_1st_bw = %u\n", pStaCfg->channel_1st_bw);
	return TRUE;
}


INT	Set_CH2_BW_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pStaCfg->channel_2nd_bw = (UINT32)os_str_tol(arg, 0, 10);
	MTWF_PRINT("channel_2nd_bw = %u\n", pStaCfg->channel_2nd_bw);
	return TRUE;
}


INT	Set_CH1_Channel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/*
	 *	pStaCfg->channel_1st_channel = (UINT32)os_str_tol(arg, 0, 10);
	 *	MTWF_PRINT("channel_1st_channel = %u\n", pStaCfg->channel_1st_channel);
	 *	return TRUE;
	 */
}


INT	Set_CH2_Channel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/*
	 *	pStaCfg->channel_2nd_channel = (UINT32)os_str_tol(arg, 0, 10);
	 *
	 *	MTWF_PRINT("channel_2nd_channel = %u\n", pStaCfg->channel_2nd_channel);
	 *	return TRUE;
	 */
}

INT Set_MCC_Start_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int start = 0;

	MTWF_PRINT("start=%d\n", start);
	return TRUE;
}


INT Set_Pkt_Dbg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pStaCfg->bpkt_dbg = (UINT32)os_str_tol(arg, 0, 10);
	MTWF_PRINT("pStaCfg->bpkt_dbg=%d\n", pStaCfg->bpkt_dbg);
	return TRUE;
}


INT	Show_CH1StayTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_PRINT("channel_1st_staytime = %u\n", pStaCfg->channel_2nd_staytime);
	return TRUE;
}

INT	Show_CH2StayTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_PRINT("channel_2nd_staytime = %u\n", pStaCfg->channel_2nd_staytime);
	return TRUE;
}


INT	Show_SwitchIdleTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_PRINT("switch_idle_time = %u\n", pStaCfg->switch_idle_time);
	return TRUE;
}


INT	Show_NULL_Frame_Count_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_PRINT("null_frame_count = %u\n", pStaCfg->null_frame_count);
	return TRUE;
}
#endif /* CONFIG_MULTI_CHANNEL */


#ifdef WIDI_SUPPORT
INT Set_WiDiEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	if (os_str_tol(arg, 0, 10) == 0)
		pStaCfg->bWIDI = FALSE;
	else
		pStaCfg->bWIDI = TRUE;

	MTWF_PRINT("bWIDI=%d\n", pStaCfg->bWIDI);
	return TRUE;
}
#endif

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
/* set WOW enable */
INT Set_WOW_Enable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 Val;
	UINT8 Pin = pAd->WOW_Cfg.nSelectedGPIO;
	ULONG Value = os_str_tol(arg, 0, 10);

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		pAd->WOW_Cfg.bEnable = (BOOLEAN)Value;
		MTWF_PRINT("WOW_Enable = %d\n", pAd->WOW_Cfg.bEnable);
		return TRUE;
	}

	if (Value != 1)
		Value = 0;  /* default is disable */

	pAd->WOW_Cfg.bEnable = (BOOLEAN)Value;
	/* pull GPIO high */
	RTMP_IO_READ32(pAd->hdev_ctrl, GPIO_CTRL_CFG, &Val);

	if (Pin <= 7) {
		Val &= ~(1UL << (Pin + 8));	/* direction: 0 out, 1 in */
		Val |= 1UL << Pin;			/* data */
	} else {
		Val &= ~(1UL << (Pin + 16));	/* direction: 0 out, 1 in */
		Val |= 1UL << (Pin + 8);		/* data */
	}

	RTMP_IO_WRITE32(pAd->hdev_ctrl, GPIO_CTRL_CFG, Val);
	MTWF_PRINT("WOW_Enable = %d, GPIO = %x\n", pAd->WOW_Cfg.bEnable, Val);
	return TRUE;
}

/* set GPIO pin for wake-up signal */
INT Set_WOW_GPIO(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value = os_str_tol(arg, 0, 10);

	pAd->WOW_Cfg.nSelectedGPIO = (UINT8)Value;
	MTWF_PRINT("WOW_GPIO = %d\n", pAd->WOW_Cfg.nSelectedGPIO);
	return TRUE;
}

/* set delay time for WOW really enable */
INT Set_WOW_Delay(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value = os_str_tol(arg, 0, 10);

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "MTxxxx does not support this!\n");
		return FALSE;
	}

	if (Value > 255)
		Value = 3; /* default delay time */

	pAd->WOW_Cfg.nDelay = (UINT8)Value;
	MTWF_PRINT("WOW_Delay = %d, equal to %d sec\n", pAd->WOW_Cfg.nDelay,
			 (pAd->WOW_Cfg.nDelay + 1) * 3);
	return TRUE;
}

/* set wake up hold time */
INT Set_WOW_Hold(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value = os_str_tol(arg, 0, 10);

	pAd->WOW_Cfg.nHoldTime = (UINT32)Value;
	MTWF_PRINT("WOW_Hold = %d, equal to %d us\n", pAd->WOW_Cfg.nHoldTime,
			 (pAd->WOW_Cfg.nHoldTime) * 1);
	return TRUE;
}

/* set wake up signal type */
INT Set_WOW_InBand(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value = os_str_tol(arg, 0, 10);

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "MTxxxx does not support this!\n");
		return FALSE;
	}

	if (Value != 1)
		Value = 0; /* use GPIO to wakeup system */

	pAd->WOW_Cfg.bInBand = (UINT8)Value;
	MTWF_PRINT("WOW_Inband = %d\n", pAd->WOW_Cfg.bInBand);
	return TRUE;
}

/* set wake up interface */
INT Set_WOW_Interface(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value = os_str_tol(arg, 0, 10);

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		pAd->WOW_Cfg.nWakeupInterface = (UINT8)Value;
		MTWF_PRINT("WakeupInterface = %d\n", pAd->WOW_Cfg.nWakeupInterface);
		return TRUE;
	}

	return TRUE;
}

/* set wow if down support */
INT Set_WOW_IfDown_Support(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value = os_str_tol(arg, 0, 10);

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		pAd->WOW_Cfg.bWowIfDownSupport = (UINT8)Value;
		MTWF_PRINT("WOW_IfDown_Support = %d\n", pAd->WOW_Cfg.bWowIfDownSupport);
		return TRUE;
	}

	return TRUE;
}


/* set IP Address for ARP response */
INT Set_WOW_IPAddress(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		rtinet_aton(arg, (UINT32 *)(&pAd->WOW_Cfg.IPAddress));
		MTWF_PRINT("IP Address = %d.%d.%d.%d\n",
				 pAd->WOW_Cfg.IPAddress[0],
				 pAd->WOW_Cfg.IPAddress[1],
				 pAd->WOW_Cfg.IPAddress[2],
				 pAd->WOW_Cfg.IPAddress[3]);
		return TRUE;
	}

	return TRUE;
}

/* set wake up GPIO High Low */
INT Set_WOW_GPIOHighLow(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Value = os_str_tol(arg, 0, 10);

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		pAd->WOW_Cfg.bGPIOHighLow = (UINT8)Value;
		MTWF_PRINT("GPIOHighLow = %d\n", pAd->WOW_Cfg.bGPIOHighLow);
		return TRUE;
	}

	return TRUE;
}

#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT) */



INT RTMPSetInformation(
	IN RTMP_ADAPTER *pAd,
	INOUT RTMP_IOCTL_INPUT_STRUCT * rq,
	IN  INT cmd,
	IN struct wifi_dev *wdev)
{
	CHANNEL_CTRL *pChCtrl;
	UCHAR BandIdx;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	RTMP_IOCTL_INPUT_STRUCT *wrq = (RTMP_IOCTL_INPUT_STRUCT *) rq;
	NDIS_802_11_SSID Ssid;
	NDIS_802_11_MAC_ADDRESS Bssid;
	RT_802_11_PHY_MODE PhyMode = PHY_MODE_MAX;
	RT_802_11_STA_CONFIG StaConfig;
	NDIS_802_11_RATES aryRates;
	RT_802_11_PREAMBLE Preamble = Rt802_11PreambleAuto;
	NDIS_802_11_WEP_STATUS WepStatus = Ndis802_11WEPEnabled;
	NDIS_802_11_AUTHENTICATION_MODE AuthMode = Ndis802_11AuthModeMax;
	NDIS_802_11_NETWORK_INFRASTRUCTURE  BssType = Ndis802_11InfrastructureMax;
	NDIS_802_11_RTS_THRESHOLD RtsThresh = 0;
	NDIS_802_11_FRAGMENTATION_THRESHOLD FragThresh = 0;
	NDIS_802_11_POWER_MODE PowerMode = Ndis802_11PowerModeMax;
	PNDIS_802_11_KEY pKey = NULL;
	PNDIS_802_11_WEP pWepKey = NULL;
	PNDIS_802_11_REMOVE_KEY pRemoveKey = NULL;
	NDIS_802_11_CONFIGURATION *pConfig = NULL;
	NDIS_802_11_NETWORK_TYPE NetType = Ndis802_11NetworkTypeMax;
	ULONG Now;
	UINT KeyIdx = 0;
	INT Status = NDIS_STATUS_SUCCESS, MaxPhyMode;
	UINT8 PowerTemp = 0;
	BOOLEAN RadioState = FALSE;
	BOOLEAN StateMachineTouched = FALSE;
	PNDIS_802_11_PASSPHRASE ppassphrase = NULL;
#ifdef DOT11_N_SUPPORT
	OID_SET_HT_PHYMODE HT_PhyMode;	/*11n */
#endif /* DOT11_N_SUPPORT */
#ifdef WPA_SUPPLICANT_SUPPORT
	PNDIS_802_11_PMKID pPmkId = NULL;
	BOOLEAN IEEE8021xState = FALSE;
	BOOLEAN IEEE8021x_required_keys = FALSE;
	UCHAR wpa_supplicant_enable = 0;
#endif /* WPA_SUPPLICANT_SUPPORT */
	UINT StaKeyLen = 0;
#ifdef SNMP_SUPPORT
	TX_RTY_CFG_STRUC tx_rty_cfg;
	ULONG ShortRetryLimit, LongRetryLimit;
	UCHAR ctmp;
#endif /* SNMP_SUPPORT */
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
	UINT	WPSLedMode10 = 0;
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef DOT11_N_SUPPORT
	MaxPhyMode = PHY_11N_5G;
#else
	MaxPhyMode = PHY_11G;
#endif /* DOT11_N_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CFG: IOCTL cmd:0x%08x\n", cmd & 0x7FFF);

	switch (cmd & 0x7FFF) {
	case RT_OID_802_11_COUNTRY_REGION:
		if (wrq->u.data.length < sizeof(UCHAR))
			Status = -EINVAL;
		/* Only avaliable when EEPROM not programming */
		else if (!(pAd->CommonCfg.CountryRegion & 0x80) && !(pAd->CommonCfg.CountryRegionForABand & 0x80)) {
			ULONG   Country = 0, sz;

			sz = (wrq->u.data.length > sizeof(Country) ? sizeof(Country) : wrq->u.data.length);
			Status = copy_from_user(&Country, wrq->u.data.pointer, sz);
			pAd->CommonCfg.CountryRegion = (UCHAR)(Country & 0x000000FF);
			pAd->CommonCfg.CountryRegionForABand = (UCHAR)((Country >> 8) & 0x000000FF);
			/* Build all corresponding channel information */
			/* Change channel state to NONE */
			BandIdx = HcGetBandByWdev(wdev);
			pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
			hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
			BuildChannelList(pAd, wdev);
			RTMPSetPhyMode(pAd, wdev, wdev->PhyMode);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_COUNTRY_REGION (A:%d  B/G:%d)\n",
					 pAd->CommonCfg.CountryRegionForABand,
					 pAd->CommonCfg.CountryRegion);
		}

		break;

	case OID_802_11_BSSID_LIST_SCAN:
		RTMP_GetCurrentSystemTick(&Now);
		/*            Now = jiffies; */
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_BSSID_LIST_SCAN, TxCnt = %d\n",
				 pAd->RalinkCounters.LastOneSecTotalTxCount);

		if (MONITOR_ON(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "!!! Driver is in Monitor Mode now !!!\n");
			break;
		}

		/*Benson add 20080527, when radio off, sta don't need to scan */
		if (IsHcRadioCurStatOffByWdev(wdev))
			break;

		if (pAd->RalinkCounters.LastOneSecTotalTxCount > 100) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "!!! Link UP, ignore this set::OID_802_11_BSSID_LIST_SCAN\n");
			Status = NDIS_STATUS_SUCCESS;
			break;
		}

		if ((STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) &&
			IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap) &&
			(wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "!!! Link UP, Port Not Secured! ignore this set::OID_802_11_BSSID_LIST_SCAN\n");
			Status = NDIS_STATUS_SUCCESS;
			break;
		}

#ifdef WSC_STA_SUPPORT

		if ((pStaCfg->wdev.WscControl.WscConfMode != WSC_DISABLE) &&
			(pStaCfg->wdev.WscControl.WscState >= WSC_STATE_LINK_UP)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "!!! WPS processing now! ignore this set::OID_802_11_BSSID_LIST_SCAN\n");
			Status = NDIS_STATUS_SUCCESS;
			break;
		}

#endif /* WSC_STA_SUPPORT */
		StaSiteSurvey(pAd, NULL, SCAN_ACTIVE, &pStaCfg->wdev);
		break;

	case OID_802_11_SSID:
		if (wrq->u.data.length != sizeof(NDIS_802_11_SSID))
			Status = -EINVAL;
		else {
			RTMP_STRING *pSsidString = NULL;
			NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
			Status = copy_from_user(&Ssid, wrq->u.data.pointer, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_SSID (Len=%d,Ssid=%s)\n", Ssid.SsidLength,
					 Ssid.Ssid);

			if (Ssid.SsidLength > MAX_LEN_OF_SSID)
				Status = -EINVAL;
			else {
				if (Ssid.SsidLength == 0)
					Set_SSID_Proc(pAd, "");
				else {
					os_alloc_mem(pAd, (UCHAR **)&pSsidString, MAX_LEN_OF_SSID + 1);

					if (pSsidString) {
						NdisZeroMemory(pSsidString, MAX_LEN_OF_SSID + 1);
						NdisMoveMemory(pSsidString, Ssid.Ssid, MAX_LEN_OF_SSID);
						pSsidString[MAX_LEN_OF_SSID] = 0x00;
						Set_SSID_Proc(pAd, pSsidString);
						os_free_mem(pSsidString);
					} else
						Status = -ENOMEM;
				}
			}
		}

		break;
	case OID_802_11_SET_PASSPHRASE:
		os_alloc_mem(pAd, (UCHAR **)&ppassphrase, wrq->u.data.length);

		if (ppassphrase == NULL) {
			Status = -ENOMEM;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_SET_PASSPHRASE, Failed!!\n");
			break;
		} else {
			Status = copy_from_user(ppassphrase, wrq->u.data.pointer, wrq->u.data.length);

			if (Status) {
				Status  = -EINVAL;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_SET_PASSPHRASE, Failed (length mismatch)!!\n");
			} else {
				if (ppassphrase->KeyLength < 8 || ppassphrase->KeyLength > 64) {
					Status  = -EINVAL;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "OID_802_11_SET_PASSPHRASE, Failed (len less than 8 or greater than 64)!!\n");
				} else {
					/* set key passphrase and length */
					NdisZeroMemory(pStaCfg->WpaPassPhrase, 64);
					if (ppassphrase->KeyLength > ARRAY_SIZE(pStaCfg->WpaPassPhrase))
						StaKeyLen = ARRAY_SIZE(pStaCfg->WpaPassPhrase);
					else
						StaKeyLen = ppassphrase->KeyLength;
					NdisMoveMemory(pStaCfg->WpaPassPhrase, &ppassphrase->KeyMaterial, StaKeyLen);
					pStaCfg->WpaPassPhraseLen = StaKeyLen;
					hex_dump("pStaCfg->WpaPassPhrase", pStaCfg->WpaPassPhrase, 64);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WpaPassPhrase=%s\n", pStaCfg->WpaPassPhrase);
				}
			}
		}

		os_free_mem(ppassphrase);
		break;

	case OID_802_11_BSSID:
		if (wrq->u.data.length != sizeof(NDIS_802_11_MAC_ADDRESS))
			Status  = -EINVAL;
		else {
			NdisZeroMemory(&Bssid, sizeof(NDIS_802_11_MAC_ADDRESS));
			Status = copy_from_user(&Bssid, wrq->u.data.pointer, wrq->u.data.length);
			if (Status) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"copy_from_user fail[%d]!\n", Status);
			}
			/* tell CNTL state machine to call NdisMSetInformationComplete() after completing */
			/* this request, because this request is initiated by NDIS. */
			pStaCfg->MlmeAux.CurrReqIsFromNdis = FALSE;
			/* Prevent to connect AP again in STAMlmePeriodicExec */
			pStaCfg->MlmeAux.AutoReconnectSsidLen = 32;

			cntl_connect_request(wdev, CNTL_CONNECT_BY_BSSID, MAC_ADDR_LEN, Bssid);

			Status = NDIS_STATUS_SUCCESS;
			StateMachineTouched = TRUE;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_BSSID "MACSTR"\n",
					 MAC2STR(Bssid));
		}

		break;

	case RT_OID_802_11_RADIO:
		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status  = -EINVAL;
		else {
			Status = copy_from_user(&RadioState, wrq->u.data.pointer, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_RADIO (=%d)\n", RadioState);

			if (pStaCfg->bSwRadio != RadioState) {
				pStaCfg->bSwRadio = RadioState;

				if (pStaCfg->bRadio != (pStaCfg->bHwRadio && pStaCfg->bSwRadio)) {
					pStaCfg->bRadio = (pStaCfg->bHwRadio && pStaCfg->bSwRadio);

					if (pStaCfg->bRadio == TRUE) {
						MlmeRadioOn(pAd, wdev);
						/* Update extra information */
						pAd->ExtraInfo = EXTRA_INFO_CLEAR;
					} else {
						RTMP_MLME_RESET_STATE_MACHINE(pAd, &pStaCfg->wdev);
						MlmeRadioOff(pAd, wdev);
						/* Update extra information */
						pAd->ExtraInfo = SW_RADIO_OFF;
					}
				}
			}
		}

		break;

	case RT_OID_802_11_PHY_MODE:
		if (wrq->u.data.length != sizeof(RT_802_11_PHY_MODE))
			Status  = -EINVAL;
		else {
			/* TODO: shiang-6590, fix this for PhyMode!! */
			Status = copy_from_user(&PhyMode, wrq->u.data.pointer, wrq->u.data.length);

			if (PhyMode <= MaxPhyMode) {
				pAd->CommonCfg.cfg_wmode = PhyMode;
				/* Change channel state to NONE */
				BandIdx = HcGetBandByWdev(wdev);
				pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
				hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
				BuildChannelList(pAd, wdev);
				RTMPSetPhyMode(pAd, wdev, PhyMode);
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_PHY_MODE (=%d)\n", PhyMode);
		}

		break;

	case RT_OID_802_11_STA_CONFIG:
		if (wrq->u.data.length != sizeof(RT_802_11_STA_CONFIG))
			Status  = -EINVAL;
		else {
			UINT8 i;
			NdisZeroMemory(&StaConfig, sizeof(RT_802_11_STA_CONFIG));
			Status = copy_from_user(&StaConfig, wrq->u.data.pointer, wrq->u.data.length);
			pAd->CommonCfg.bEnableTxBurst = StaConfig.EnableTxBurst;
			pAd->CommonCfg.UseBGProtection = StaConfig.UseBGProtection;
			for (i = 0; i < DBDC_BAND_NUM; i++) {
				pAd->CommonCfg.bUseShortSlotTime[i] = TRUE;
				pAd->CommonCfg.SlotTime[i] = 9;
			}

			if ((wdev->PhyMode != StaConfig.AdhocMode) &&
				(StaConfig.AdhocMode <= MaxPhyMode)) {
				/* allow dynamic change of "USE OFDM rate or not" in ADHOC mode */
				/* if setting changed, need to reset current TX rate as well as BEACON frame format */
				if (pStaCfg->BssType == BSS_ADHOC) {
					/* Change channel state to NONE */
					BandIdx = HcGetBandByWdev(wdev);
					pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
					hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
					BuildChannelList(pAd, wdev);
					RTMPSetPhyMode(pAd, wdev, StaConfig.AdhocMode);
					MlmeUpdateTxRates(pAd, FALSE, 0);
					UpdateBeaconHandler(
						pAd,
						wdev,
						BCN_UPDATE_IF_STATE_CHG);
					AsicEnableIbssSync(
						pAd,
						pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)],
						HW_BSSID_0,
						OPMODE_ADHOC);
				}
			}
			BandIdx = HcGetBandByWdev(wdev);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "RT_OID_802_11_SET_STA_CONFIG (Burst=%d, Protection=%ld,ShortSlot=%d\n",
					  pAd->CommonCfg.bEnableTxBurst,
					  pAd->CommonCfg.UseBGProtection,
					  pAd->CommonCfg.bUseShortSlotTime[BandIdx]);
			AsicSetRxFilter(pAd);
		}

		break;

	case OID_802_11_DESIRED_RATES:
		if (wrq->u.data.length != sizeof(NDIS_802_11_RATES))
			Status  = -EINVAL;
		else {
			NdisZeroMemory(&aryRates, sizeof(NDIS_802_11_RATES));
			Status = copy_from_user(&aryRates, wrq->u.data.pointer, wrq->u.data.length);
			NdisZeroMemory(wdev->rate.DesireRate, MAX_LEN_OF_SUPPORTED_RATES);
			NdisMoveMemory(wdev->rate.DesireRate, &aryRates, sizeof(NDIS_802_11_RATES));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "OID_802_11_DESIRED_RATES (%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x)\n",
					  wdev->rate.DesireRate[0], wdev->rate.DesireRate[1],
					  wdev->rate.DesireRate[2], wdev->rate.DesireRate[3],
					  wdev->rate.DesireRate[4], wdev->rate.DesireRate[5],
					  wdev->rate.DesireRate[6], wdev->rate.DesireRate[7]);
			/* Changing DesiredRate may affect the MAX TX rate we used to TX frames out */
			MlmeUpdateTxRates(pAd, FALSE, 0);
		}

		break;

	case RT_OID_802_11_PREAMBLE:
		if (wrq->u.data.length != sizeof(RT_802_11_PREAMBLE))
			Status  = -EINVAL;
		else {
			Status = copy_from_user(&Preamble, wrq->u.data.pointer, wrq->u.data.length);

			if (Preamble == Rt802_11PreambleShort) {
				pAd->CommonCfg.TxPreamble = Preamble;
				MlmeSetTxPreamble(pAd, Rt802_11PreambleShort);
			} else if ((Preamble == Rt802_11PreambleLong) || (Preamble == Rt802_11PreambleAuto)) {
				/* if user wants AUTO, initialize to LONG here, then change according to AP's */
				/* capability upon association. */
				pAd->CommonCfg.TxPreamble = Preamble;
				MlmeSetTxPreamble(pAd, Rt802_11PreambleLong);
			} else {
				Status = -EINVAL;
				break;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_PREAMBLE (=%d)\n", Preamble);
		}

		break;

	case OID_802_11_WEP_STATUS:
		if (wrq->u.data.length != sizeof(NDIS_802_11_WEP_STATUS))
			Status  = -EINVAL;
		else {
			UINT32 EncryType = 0;

			Status = copy_from_user(&WepStatus, wrq->u.data.pointer, wrq->u.data.length);
			EncryType = SecEncryModeOldToNew(WepStatus);

			/* Since TKIP, AES, WEP are all supported. It should not have any invalid setting */
			if (pStaCfg->wdev.SecConfig.PairwiseCipher != EncryType) {
				/* Config has changed */
				pStaCfg->bConfigChanged = TRUE;
			}

			pStaCfg->wdev.SecConfig.PairwiseCipher   = EncryType;
			pStaCfg->PairwiseCipher = EncryType;
			pStaCfg->GroupCipher   = EncryType;

			if (pStaCfg->BssType == BSS_ADHOC) {
				/* Change channel state to NONE */
				BandIdx = HcGetBandByWdev(wdev);
				pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
				hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
				BuildChannelList(pAd, wdev);
				RTMPSetPhyMode(pAd, &pStaCfg->wdev, pAd->CommonCfg.cfg_wmode);
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_WEP_STATUS (=0x%x)\n", EncryType);
		}

		break;

	case OID_802_11_AUTHENTICATION_MODE:
		if (wrq->u.data.length != sizeof(NDIS_802_11_AUTHENTICATION_MODE))
			Status  = -EINVAL;
		else {
			UINT32 AKMMap = 0;

			Status = copy_from_user(&AuthMode, wrq->u.data.pointer, wrq->u.data.length);

			if (AuthMode > Ndis802_11AuthModeMax) {
				Status  = -EINVAL;
				break;
			} else {
				AKMMap = SecAuthModeOldToNew(AuthMode);

				if (wdev->SecConfig.AKMMap != AKMMap) {
					/* Config has changed */
					pStaCfg->bConfigChanged = TRUE;
				}

				wdev->SecConfig.AKMMap = AKMMap;
			}

			wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_AUTHENTICATION_MODE (=0x%x)\n",
					 wdev->SecConfig.AKMMap);
		}

		break;

	case OID_802_11_INFRASTRUCTURE_MODE:
		if (wrq->u.data.length != sizeof(NDIS_802_11_NETWORK_INFRASTRUCTURE))
			Status  = -EINVAL;
		else {
			Status = copy_from_user(&BssType, wrq->u.data.pointer, wrq->u.data.length);

			if (BssType == Ndis802_11IBSS)
				Set_NetworkType_Proc(pAd, "Adhoc");
			else if (BssType == Ndis802_11Infrastructure)
				Set_NetworkType_Proc(pAd, "Infra");
			else if (BssType == Ndis802_11Monitor)
				Set_NetworkType_Proc(pAd, "Monitor");
			else {
				Status  = -EINVAL;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_INFRASTRUCTURE_MODE (unknown)\n");
			}
		}

		break;

	case OID_802_11_REMOVE_WEP:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_REMOVE_WEP\n");

		if (wrq->u.data.length != sizeof(NDIS_802_11_KEY_INDEX))
			Status = -EINVAL;
		else {
			KeyIdx = *(NDIS_802_11_KEY_INDEX *) wrq->u.data.pointer;

			if (KeyIdx & 0x80000000) {
				/* Should never set default bit when remove key */
				Status = -EINVAL;
			} else {
				KeyIdx = KeyIdx & 0x0fffffff;

				if (KeyIdx >= 4)
					Status = -EINVAL;
				else {
					pAd->SharedKey[BSS0][KeyIdx].KeyLen = 0;
					pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_NONE;
					AsicRemoveSharedKeyEntry(pAd, 0, (UCHAR)KeyIdx);
				}
			}
		}

		break;

	case RT_OID_802_11_RESET_COUNTERS:
		NdisZeroMemory(&pAd->WlanCounters, sizeof(COUNTER_802_11));
		NdisZeroMemory(&pAd->Counters8023, sizeof(COUNTER_802_3));
		NdisZeroMemory(&pAd->RalinkCounters, sizeof(COUNTER_RALINK));
		pAd->Counters8023.RxNoBuffer   = 0;
		pAd->Counters8023.GoodReceives = 0;
		pAd->Counters8023.RxNoBuffer   = 0;
#ifdef TXBF_SUPPORT
{
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

		if (cap->FlgHwTxBfCap) {
			int i;

			for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++)
				NdisZeroMemory(&pAd->MacTab.Content[i].TxBFCounters, sizeof(pAd->MacTab.Content[i].TxBFCounters));
		}
}
#endif /* TXBF_SUPPORT */
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_RESET_COUNTERS\n");
		break;

	case OID_802_11_RTS_THRESHOLD:
		if (wrq->u.data.length != sizeof(NDIS_802_11_RTS_THRESHOLD))
			Status  = -EINVAL;
		else {
			Status = copy_from_user(&RtsThresh, wrq->u.data.pointer, wrq->u.data.length);

			if (RtsThresh > MAX_RTS_THRESHOLD) {
				Status = -EINVAL;
				RtsThresh = 0; /* avoid compile warning in printk() */
			} else
				wlan_operate_set_rts_len_thld(wdev, (UINT32)RtsThresh);
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_RTS_THRESHOLD (=%ld)\n", RtsThresh);
		break;

	case OID_802_11_FRAGMENTATION_THRESHOLD:
		if (wrq->u.data.length != sizeof(NDIS_802_11_FRAGMENTATION_THRESHOLD)) {
			Status  = -EINVAL;
			FragThresh = 0; /* avoid compile warning in printk() */
		} else {
			Status = copy_from_user(&FragThresh, wrq->u.data.pointer, wrq->u.data.length);
			pAd->CommonCfg.bUseZeroToDisableFragment = FALSE;

			if (FragThresh > MAX_FRAG_THRESHOLD || FragThresh < MIN_FRAG_THRESHOLD) {
				if (FragThresh == 0) {
					wlan_operate_set_frag_thld(wdev, MAX_FRAG_THRESHOLD);
					pAd->CommonCfg.bUseZeroToDisableFragment = TRUE;
				} else
					Status  = -EINVAL;
			} else
				wlan_operate_set_frag_thld(wdev, FragThresh);
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_FRAGMENTATION_THRESHOLD (=%ld)\n", FragThresh);
		break;

	case OID_802_11_POWER_MODE:
		if (wrq->u.data.length != sizeof(NDIS_802_11_POWER_MODE)) {
			Status = -EINVAL;
			PowerMode = 0; /* avoid compile warning in printk() */
		} else {
			Status = copy_from_user(&PowerMode, wrq->u.data.pointer, wrq->u.data.length);

			if (PowerMode == Ndis802_11PowerModeCAM)
				Set_PSMode_Proc(pAd, "CAM");
			else if (PowerMode == Ndis802_11PowerModeMAX_PSP)
				Set_PSMode_Proc(pAd, "Max_PSP");
			else if (PowerMode == Ndis802_11PowerModeFast_PSP)
				Set_PSMode_Proc(pAd, "Fast_PSP");
			else if (PowerMode == Ndis802_11PowerModeLegacy_PSP)
				Set_PSMode_Proc(pAd, "Legacy_PSP");
			else
				Status = -EINVAL;
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_POWER_MODE (=%d)\n", PowerMode);
		break;

	case RT_OID_802_11_TX_POWER_LEVEL_1:
		if (wrq->u.data.length  < sizeof(ULONG))
			Status = -EINVAL;
		else {
			ULONG sz;

			sz = (wrq->u.data.length > sizeof(PowerTemp) ? sizeof(PowerTemp) : wrq->u.data.length);
			Status = copy_from_user(&PowerTemp, wrq->u.data.pointer, sz);

			if (PowerTemp > 100)
				PowerTemp = 100;  /* AUTO */

			pAd->CommonCfg.ucTxPowerDefault[BAND0] = PowerTemp; /* keep current setting. */
			pAd->CommonCfg.ucTxPowerPercentage[BAND0] = pAd->CommonCfg.ucTxPowerDefault[BAND0];
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_TX_POWER_LEVEL_1 (BAND0) (=%d)\n",
					 pAd->CommonCfg.ucTxPowerPercentage[BAND0]);
#ifdef DBDC_MODE
			pAd->CommonCfg.ucTxPowerDefault[BAND1] = PowerTemp; /* keep current setting. */
			pAd->CommonCfg.ucTxPowerPercentage[BAND1] = pAd->CommonCfg.ucTxPowerDefault[BAND1];
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_TX_POWER_LEVEL_1 (BAND1) (=%d)\n",
					 pAd->CommonCfg.ucTxPowerPercentage[BAND1]);
#endif /* DBDC_MODE */
		}

		break;

	case OID_802_11_NETWORK_TYPE_IN_USE:
		if (wrq->u.data.length != sizeof(NDIS_802_11_NETWORK_TYPE))
			Status = -EINVAL;
		else {
			Status = copy_from_user(&NetType, wrq->u.data.pointer, wrq->u.data.length);

			if (NetType == Ndis802_11DS) {
				/* Change channel state to NONE */
				BandIdx = HcGetBandByWdev(wdev);
				pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
				hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
				BuildChannelList(pAd, wdev);
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[RTMPSetInformation][BandIdx=%d] Ndis802_11DS\n", BandIdx);
				RTMPSetPhyMode(pAd, wdev, WMODE_B);
			} else if (NetType == Ndis802_11OFDM24) {
				/* Change channel state to NONE */
				BandIdx = HcGetBandByWdev(wdev);
				pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
				hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
				BuildChannelList(pAd, wdev);
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[RTMPSetInformation][BandIdx=%d] Ndis802_11OFDM24\n", BandIdx);
				RTMPSetPhyMode(pAd, wdev, WMODE_B | WMODE_G);
			} else if (NetType == Ndis802_11OFDM5) {
				/* Change channel state to NONE */
				BandIdx = HcGetBandByWdev(wdev);
				pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
				hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
				BuildChannelList(pAd, wdev);
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[RTMPSetInformation][BandIdx=%d] Ndis802_11OFDM5\n", BandIdx);
				RTMPSetPhyMode(pAd, wdev, WMODE_A);
			}
			else
				Status = -EINVAL;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_NETWORK_TYPE_IN_USE (=%d)\n", NetType);
		}

		break;

	/* For WPA PSK PMK key */
	case RT_OID_802_11_ADD_WPA:
		os_alloc_mem(pAd, (UCHAR **)&pKey, wrq->u.data.length);

		if (pKey == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(pKey, wrq->u.data.pointer, wrq->u.data.length);

		if ((pKey->Length != wrq->u.data.length) ||
			(pKey->KeyLength > LEN_PMK)) {
			Status  = -EINVAL;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_802_11_ADD_WPA, Failed!!\n");
		} else {
			if ((!IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap)) &&
				(!IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap)) &&
				(!IS_AKM_WPANONE(wdev->SecConfig.AKMMap))) {
				Status = -EOPNOTSUPP;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "RT_OID_802_11_ADD_WPA, Failed!! [AuthMode != WPAPSK/WPA2PSK/WPANONE]\n");
			} else if (IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap) ||
					   IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap) ||
					   IS_AKM_WPANONE(wdev->SecConfig.AKMMap)) {   /* Only for WPA PSK mode */
				NdisMoveMemory(pStaCfg->PMK, &pKey->KeyMaterial, pKey->KeyLength);

				/* Use RaConfig as PSK agent. */
				/* Start STA supplicant state machine */
				if (!IS_AKM_WPANONE(wdev->SecConfig.AKMMap))
					pStaCfg->WpaState = SS_START;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_ADD_WPA (id=0x%x, Len=%d-byte)\n",
						 pKey->KeyIndex, pKey->KeyLength);
			} else {
				pStaCfg->WpaState = SS_NOTUSE;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_ADD_WPA (id=0x%x, Len=%d-byte)\n",
						 pKey->KeyIndex, pKey->KeyLength);
			}
		}

		os_free_mem(pKey);
		break;

	case OID_802_11_REMOVE_KEY:
		os_alloc_mem(pAd, (UCHAR **)&pRemoveKey, wrq->u.data.length);

		if (pRemoveKey == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(pRemoveKey, wrq->u.data.pointer, wrq->u.data.length);

		if (pRemoveKey->Length != wrq->u.data.length) {
			Status  = -EINVAL;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_REMOVE_KEY, Failed!!\n");
		} else {
			if IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap)
			{
				RTMPWPARemoveKeyProc(pAd, pRemoveKey);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_REMOVE_KEY, Remove WPA Key!!\n");
			} else {
				KeyIdx = pRemoveKey->KeyIndex;

				if (KeyIdx & 0x80000000) {
					/* Should never set default bit when remove key */
					Status  = -EINVAL;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "OID_802_11_REMOVE_KEY, Failed!!(Should never set default bit when remove key)\n");
				} else {
					KeyIdx = KeyIdx & 0x0fffffff;

					if (KeyIdx > 3) {
						Status  = -EINVAL;
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_REMOVE_KEY, Failed!!(KeyId[%d] out of range)\n",
								 KeyIdx);
					} else {
						wdev->SecConfig.WepKey[KeyIdx].KeyLen = 0;
						/* wdev->SecConfig.WepKey[KeyIdx].CipherAlg = CIPHER_NONE; */
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_REMOVE_KEY (id=0x%x, Len=%d-byte)\n",
								 pRemoveKey->KeyIndex, pRemoveKey->Length);
					}
				}
			}
		}

		os_free_mem(pRemoveKey);
		break;

	/* New for WPA */
	case OID_802_11_ADD_KEY:
		os_alloc_mem(pAd, (UCHAR **)&pKey, wrq->u.data.length);

		if (pKey == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(pKey, wrq->u.data.pointer, wrq->u.data.length);

		if ((pKey->Length != wrq->u.data.length) ||
			(pKey->KeyLength > LEN_PMK)) {
			Status  = -EINVAL;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_ADD_KEY, Failed!!\n");
		} else {
			RTMPAddKey(pAd, pKey, wdev);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_ADD_KEY (id=0x%x, Len=%d-byte)\n",
					 pKey->KeyIndex, pKey->KeyLength);
		}

		os_free_mem(pKey);
		break;

	case OID_802_11_CONFIGURATION:
		if (wrq->u.data.length != sizeof(NDIS_802_11_CONFIGURATION))
			Status  = -EINVAL;
		else {
			os_alloc_mem(NULL, (UCHAR **)&pConfig, sizeof(NDIS_802_11_CONFIGURATION));

			if (pConfig != NULL) {
				Status = copy_from_user(pConfig, wrq->u.data.pointer, wrq->u.data.length);

				/*                pConfig = &Config; */

				if ((pConfig->BeaconPeriod >= 20) && (pConfig->BeaconPeriod <= 400))
					pAd->CommonCfg.BeaconPeriod[DBDC_BAND0] = (USHORT) pConfig->BeaconPeriod;

				pStaCfg->StaActive.AtimWin = (USHORT) pConfig->ATIMWindow;
				MAP_KHZ_TO_CHANNEL_ID(pConfig->DSConfig, wdev->channel);
				/* */
				/* Save the channel on MlmeAux for CntlOidRTBssidProc used. */
				/* */
				pStaCfg->MlmeAux.Channel = wdev->channel;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "OID_802_11_CONFIGURATION (BeacnPeriod=%ld,AtimW=%ld,Ch=%d)\n",
						  pConfig->BeaconPeriod, pConfig->ATIMWindow, wdev->channel);
				/* Config has changed */
				pStaCfg->bConfigChanged = TRUE;
				os_free_mem(pConfig);
			}
		}

		break;
#ifdef DOT11_N_SUPPORT

	case RT_OID_802_11_SET_HT_PHYMODE:
		if (wrq->u.data.length	!= sizeof(OID_SET_HT_PHYMODE))
			Status = -EINVAL;
		else {
			POID_SET_HT_PHYMODE	pHTPhyMode = &HT_PhyMode;
			NdisZeroMemory(&HT_PhyMode, sizeof(OID_SET_HT_PHYMODE));
			Status = copy_from_user(&HT_PhyMode, wrq->u.data.pointer, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "pHTPhyMode	(PhyMode = %d,TransmitNo = %d, HtMode =	%d,	ExtOffset =	%d , MCS = %d, BW =	%d,	STBC = %d, SHORTGI = %d)\n",
					  pHTPhyMode->PhyMode, pHTPhyMode->TransmitNo, pHTPhyMode->HtMode, pHTPhyMode->ExtOffset,
					  pHTPhyMode->MCS, pHTPhyMode->BW, pHTPhyMode->STBC, pHTPhyMode->SHORTGI);
			pHTPhyMode->BandIdx = HcGetBandByWdev(wdev);
			pHTPhyMode->Channel = wdev->channel;

			if (WMODE_CAP_N(wdev->PhyMode))
				RTMPSetHT(pAd,	pHTPhyMode, wdev);
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "RT_OID_802_11_SET_HT_PHYMODE(MCS=%d,BW=%d,SGI=%d,STBC=%d)\n",
				  wdev->HTPhyMode.field.MCS, wdev->HTPhyMode.field.BW, wdev->HTPhyMode.field.ShortGI,
				  wdev->HTPhyMode.field.STBC);
		break;
#endif /* DOT11_N_SUPPORT */

	case RT_OID_802_11_SET_APSD_SETTING:
		if (wrq->u.data.length != sizeof(ULONG))
			Status = -EINVAL;
		else {
			ULONG apsd = 0;

			Status = copy_from_user(&apsd, wrq->u.data.pointer,	wrq->u.data.length);
			/*-------------------------------------------------------------------
			 *|B31~B7	|	B6~B5	 |	 B4	 |	 B3	 |	B2	 |	B1	 |	   B0		|
			 *---------------------------------------------------------------------
			 *| Rsvd	| Max SP Len | AC_VO | AC_VI | AC_BK | AC_BE | APSD	Capable	|
			 *---------------------------------------------------------------------
			 */
			pStaCfg->wdev.UapsdInfo.bAPSDCapable = (apsd & 0x00000001) ? TRUE :	FALSE;
			pAd->CommonCfg.bAPSDAC_BE = ((apsd	& 0x00000002) >> 1)	? TRUE : FALSE;
			pAd->CommonCfg.bAPSDAC_BK = ((apsd	& 0x00000004) >> 2)	? TRUE : FALSE;
			pAd->CommonCfg.bAPSDAC_VI = ((apsd	& 0x00000008) >> 3)	? TRUE : FALSE;
			pAd->CommonCfg.bAPSDAC_VO = ((apsd	& 0x00000010) >> 4)	? TRUE : FALSE;
			pAd->CommonCfg.MaxSPLength	= (UCHAR)((apsd	& 0x00000060) >> 5);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "RT_OID_802_11_SET_APSD_SETTING (apsd=0x%lx, APSDCap=%d, [BE,BK,VI,VO]=[%d/%d/%d/%d],	MaxSPLen=%d)\n",
					  apsd, pStaCfg->wdev.UapsdInfo.bAPSDCapable,
					  pAd->CommonCfg.bAPSDAC_BE,
					  pAd->CommonCfg.bAPSDAC_BK,
					  pAd->CommonCfg.bAPSDAC_VI,
					  pAd->CommonCfg.bAPSDAC_VO,
					  pAd->CommonCfg.MaxSPLength);
		}

		break;

	case RT_OID_802_11_SET_APSD_PSM:
		if (wrq->u.data.length	!= sizeof(ULONG))
			Status = -EINVAL;
		else {
			/* Driver needs to notify AP when PSM changes */
			Status = copy_from_user(&pAd->CommonCfg.bAPSDForcePowerSave, wrq->u.data.pointer, wrq->u.data.length);

			if (pAd->CommonCfg.bAPSDForcePowerSave != pStaCfg->PwrMgmt.Psm) {
				MAC_TABLE_ENTRY *pEntry = GetAssociatedAPByWdev(pAd, &pStaCfg->wdev);

				RTMP_SET_PSM_BIT(pAd, pStaCfg, pAd->CommonCfg.bAPSDForcePowerSave);
				RTMPSendNullFrame(pAd,
								  pEntry,
								  pAd->CommonCfg.TxRate,
								  TRUE,
								  pAd->CommonCfg.bAPSDForcePowerSave ? PWR_SAVE : pStaCfg->PwrMgmt.Psm);
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_SET_APSD_PSM (bAPSDForcePowerSave:%d)\n",
					 pAd->CommonCfg.bAPSDForcePowerSave);
		}

		break;
#ifdef DOT11Z_TDLS_SUPPORT

	case RT_OID_802_11_SET_TDLS:
		if (wrq->u.data.length != sizeof(ULONG))
			Status = -EINVAL;
		else {
			BOOLEAN	oldvalue = pStaCfg->TdlsInfo.bTDLSCapable;

			Status = copy_from_user(&pStaCfg->TdlsInfo.bTDLSCapable, wrq->u.data.pointer, wrq->u.data.length);

			if (oldvalue &&	!pStaCfg->TdlsInfo.bTDLSCapable) {
				/* tear	down local dls table entry */
				TDLS_LinkTearDown(pAd, TRUE);
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_SET_DLS (=%d)\n",
					 pAd->CommonCfg.bDLSCapable);
		}

		break;

	case RT_OID_802_11_SET_TDLS_PARAM:
		if (wrq->u.data.length	!= sizeof(RT_802_11_TDLS_UI))
			Status = -EINVAL;
		else {
			RT_802_11_TDLS		TDLS;
			/* Initialized mlme request */
			RTMPZeroMemory(&TDLS, sizeof(RT_802_11_TDLS));
			RTMPMoveMemory(&TDLS, wrq->u.data.pointer, sizeof(RT_802_11_TDLS_UI));
			MlmeEnqueue(pAd,
						MLME_CNTL_STATE_MACHINE,
						RT_OID_802_11_SET_TDLS_PARAM,
						sizeof(RT_802_11_TDLS),
						&TDLS, 0);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_SET_TDLS_PARAM\n");
		}

		break;
#endif /* DOT11Z_TDLS_SUPPORT */

	case RT_OID_802_11_SET_WMM:
		if (wrq->u.data.length	!= sizeof(BOOLEAN))
			Status = -EINVAL;
		else {
			Status = copy_from_user(&wdev->bWmmCapable, wrq->u.data.pointer, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_SET_WMM (=%d)\n", wdev->bWmmCapable);
		}

		break;

	case OID_802_11_DISASSOCIATE:
		/* */
		/* Set NdisRadioStateOff to	TRUE, instead of called	MlmeRadioOff. */
		/* Later on, NDIS_802_11_BSSID_LIST_EX->NumberOfItems should be	0 */
		/* when	query OID_802_11_BSSID_LIST. */
		/* */
		/* TRUE:  NumberOfItems	will set to	0. */
		/* FALSE: NumberOfItems	no change. */
		/* */
		pAd->CommonCfg.NdisRadioStateOff =	TRUE;
		/* Set to immediately send the media disconnect	event */
		pStaCfg->MlmeAux.CurrReqIsFromNdis	= TRUE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_DISASSOCIATE\n");
#ifdef WSC_STA_SUPPORT
#ifdef WSC_LED_SUPPORT

		if (pStaCfg->wdev.WscControl.bSkipWPSTurnOffLED == FALSE) {
			UCHAR WPSLEDStatus = LED_WPS_TURN_LED_OFF;
			BOOLEAN Cancelled;

			RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(&pStaCfg->wdev));
			/* Cancel the WPS LED timer. */
			RTMPCancelTimer(&pStaCfg->wdev.WscControl.WscLEDTimer, &Cancelled);
		}

#endif /* WSC_LED_SUPPORT */
#endif /* WSC_STA_SUPPORT */

		if (INFRA_ON(pStaCfg)) {
			cntl_disconnect_request(wdev,
									CNTL_DISASSOC,
									pStaCfg->Bssid,
									REASON_DISASSOC_STA_LEAVING);

			StateMachineTouched	= TRUE;
		}

		break;
#ifdef DOT11_N_SUPPORT

	case RT_OID_802_11_SET_IMME_BA_CAP:

		if (wrq->u.data.length != sizeof(OID_BACAP_STRUC))
			Status = -EINVAL;
		else {
			OID_BACAP_STRUC Orde;
			UINT16 ba_tx_wsize = 0, ba_rx_wsize = 0;

			NdisZeroMemory(&Orde, sizeof(OID_BACAP_STRUC));
			Status = copy_from_user(&Orde, wrq->u.data.pointer, wrq->u.data.length);

			ba_tx_wsize = wlan_config_get_ba_tx_wsize(wdev);
			ba_rx_wsize = wlan_config_get_ba_rx_wsize(wdev);


			if (Orde.Policy > IMMED_BA) {
				Status = NDIS_STATUS_INVALID_DATA;
				break;
			}
			wlan_config_set_ba_enable(wdev, Orde.AutoBA);
			wlan_operate_set_min_start_space(wdev, Orde.MpduDensity);
			wlan_operate_set_mmps(wdev, Orde.MMPSmode);
			wlan_operate_set_mmps(wdev, Orde.MMPSmode);
			wlan_operate_set_max_amsdu_len(wdev, Orde.AmsduSize);
			wlan_operate_set_min_start_space(wdev, Orde.MpduDensity);

			if (ba_rx_wsize > MAX_RX_REORDERBUF)
				ba_rx_wsize = MAX_RX_REORDERBUF;
#ifdef COEX_SUPPORT
			if (ba_tx_wsize > MAX_RX_REORDERBUF)
				ba_tx_wsize = MAX_RX_REORDERBUF;
#endif /* COEX_SUPPORT */
			wlan_config_set_ba_txrx_wsize(wdev, ba_tx_wsize, ba_rx_wsize);

			pAd->CommonCfg.REGBACapability.word = pAd->CommonCfg.BACapability.word;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "(Orde.AutoBA = %d)(ReBAWinLimit=%d)(TxBAWinLimit=%d)\n",
					  Orde.AutoBA,
					  ba_rx_wsize, ba_tx_wsize);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "(MimoPs = %d)(AmsduSize=%d)(MpduDensity=%d)\n",
					  Orde.MMPSmode,
					  Orde.AmsduSize,
					  Orde.MpduDensity);
		}
		break;

	case RT_OID_802_11_ADD_IMME_BA:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_ADD_IMME_BA\n");

		if (wrq->u.data.length != sizeof(OID_ADD_BA_ENTRY))
			Status = -EINVAL;
		else {
			UCHAR		        index;
			OID_ADD_BA_ENTRY    BA;
			MAC_TABLE_ENTRY     *pEntry;

			NdisZeroMemory(&BA, sizeof(OID_ADD_BA_ENTRY));
			Status = copy_from_user(&BA, wrq->u.data.pointer, wrq->u.data.length);

			if (BA.TID > (NUM_OF_TID - 1)) {
				Status = NDIS_STATUS_INVALID_DATA;
				break;
			} else {
				/*BATableInsertEntry */
				/*As ad-hoc mode, BA pair is not limited to only BSSID. so add via OID. */
				index = BA.TID;
				/* in ad hoc mode, when adding BA pair, we should insert this entry into MACEntry too */
				pEntry = MacTableLookup2(pAd, BA.MACAddr, wdev);

				if (!pEntry) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_802_11_ADD_IMME_BA. break on no connection.----:%x:%x\n",
							 BA.MACAddr[4], BA.MACAddr[5]);
					break;
				}

				if (BA.IsRecipient == FALSE) {
					if (pEntry->bIAmBadAtheros == TRUE)
						wlan_config_set_ba_txrx_wsize(pEntry->wdev, 0x10, 0x10);

					ba_ori_session_setup(pAd, pEntry->wcid, index, 0);
				} else {
					/*BATableInsertEntry(pAd, pEntry->Aid, BA.MACAddr, 0, 0xffff, BA.TID, BA.nMSDU, BA.IsRecipient); */
				}

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "RT_OID_802_11_ADD_IMME_BA. Rec = %d. Mac = "MACSTR" .\n",
						  BA.IsRecipient, MAC2STR(BA.MACAddr));
			}
		}

		break;

	case RT_OID_802_11_TEAR_IMME_BA:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_TEAR_IMME_BA\n");

		if (wrq->u.data.length != sizeof(OID_ADD_BA_ENTRY))
			Status = -EINVAL;
		else {
			POID_ADD_BA_ENTRY	pBA;
			MAC_TABLE_ENTRY *pEntry;

			os_alloc_mem(pAd, (UCHAR **)&pBA, wrq->u.data.length);

			if (pBA == NULL) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "RT_OID_802_11_TEAR_IMME_BA: can't allocate enough memory\n");
				Status = NDIS_STATUS_FAILURE;
			} else {
				Status = copy_from_user(pBA, wrq->u.data.pointer, wrq->u.data.length);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_TEAR_IMME_BA(TID=%d, bAllTid=%d)\n",
						 pBA->TID, pBA->bAllTid);

				if (!pBA->bAllTid && (pBA->TID > (NUM_OF_TID - 1))) {
					Status = NDIS_STATUS_INVALID_DATA;
					os_free_mem(pBA);
					break;
				}

				if (pBA->IsRecipient == FALSE) {
					pEntry = MacTableLookup2(pAd, pBA->MACAddr, wdev);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pBA->IsRecipient == FALSE\n");

					if (pEntry) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pBA->pEntry\n");
						ba_ori_session_tear_down(pAd, pEntry->wcid, pBA->TID, FALSE);
					} else
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Not found pEntry\n");
				} else {
					pEntry = MacTableLookup2(pAd, pBA->MACAddr, wdev);

					if (pEntry)
						ba_rec_session_tear_down(pAd, pEntry->wcid, pBA->TID, TRUE);
					else
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Not found pEntry\n");
				}

				os_free_mem(pBA);
			}
		}

		break;
#endif /* DOT11_N_SUPPORT */

	/* For WPA_SUPPLICANT to set static wep key */
	case OID_802_11_ADD_WEP:
		os_alloc_mem(pAd, (UCHAR **)&pWepKey, wrq->u.data.length);

		if (pWepKey == NULL) {
			Status = -ENOMEM;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_ADD_WEP, Failed!!\n");
			break;
		}

		Status = copy_from_user(pWepKey, wrq->u.data.pointer, wrq->u.data.length);

		if (Status) {
			Status  = -EINVAL;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_ADD_WEP, Failed (length mismatch)!!\n");
		} else {
			KeyIdx = pWepKey->KeyIndex & 0x0fffffff;

			/* KeyIdx must be 0 ~ 3 */
			if (KeyIdx >= 4) {
				Status  = -EINVAL;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "OID_802_11_ADD_WEP, Failed (KeyIdx must be smaller than 4)!!\n");
			} else if (pWepKey->KeyLength > 32) {
				Status  = -EINVAL;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "OID_802_11_ADD_WEP, Failed (pWepKey->KeyLength can not larger than 32)!!\n");
			} else {
				UCHAR CipherAlg = 0;
				PUCHAR Key;
				/* Zero the specific shared key */
				NdisZeroMemory(&wdev->SecConfig.WepKey[KeyIdx], sizeof(CIPHER_KEY));
				/* set key material and key length */
				wdev->SecConfig.WepKey[KeyIdx].KeyLen = (UCHAR) pWepKey->KeyLength;
				NdisMoveMemory(wdev->SecConfig.WepKey[KeyIdx].Key, &pWepKey->KeyMaterial, pWepKey->KeyLength);

				switch (pWepKey->KeyLength) {
				case 5:
					CipherAlg = CIPHER_WEP64;
					break;

				case 13:
					CipherAlg = CIPHER_WEP128;
					break;

				default:
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "OID_802_11_ADD_WEP, only support CIPHER_WEP64(len:5) & CIPHER_WEP128(len:13)!!\n");
					Status = -EINVAL;
					break;
				}

				pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CipherAlg;

				/* Default key for tx (shared key) */
				if (pWepKey->KeyIndex & 0x80000000) {
#ifdef WPA_SUPPLICANT_SUPPORT
					NdisZeroMemory(&pStaCfg->wpa_supplicant_info.DesireSharedKey[KeyIdx], sizeof(CIPHER_KEY));
					/* set key material and key length */
					pStaCfg->wpa_supplicant_info.DesireSharedKey[KeyIdx].KeyLen = (UCHAR) pWepKey->KeyLength;
					NdisMoveMemory(pStaCfg->wpa_supplicant_info.DesireSharedKey[KeyIdx].Key,
								   &pWepKey->KeyMaterial,
								   pWepKey->KeyLength);
					pStaCfg->wpa_supplicant_info.DesireSharedKeyId = KeyIdx;
					pStaCfg->wpa_supplicant_info.DesireSharedKey[KeyIdx].CipherAlg = CipherAlg;
#endif /* WPA_SUPPLICANT_SUPPORT */
					wdev->SecConfig.PairwiseKeyId = (UCHAR) KeyIdx;
				}

#ifdef WPA_SUPPLICANT_SUPPORT

				if ((pStaCfg->wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
					(IS_AKM_WPA_CAPABILITY_Entry(wdev))) {
					Key = pWepKey->KeyMaterial;
					/* Set Group key material to Asic */
					AsicAddSharedKeyEntry(pAd, BSS0, KeyIdx, &pAd->SharedKey[BSS0][KeyIdx]);
						STA_PORT_SECURED_BY_WDEV(pAd, pStaCfg->wdev);
				} else if (wdev->PortSecured == WPA_802_1X_PORT_SECURED)
#endif /* WPA_SUPPLICANT_SUPPORT */
				{
					Key = pAd->SharedKey[BSS0][KeyIdx].Key;
					/* Set key material and cipherAlg to Asic */
					AsicAddSharedKeyEntry(pAd, BSS0, KeyIdx, &pAd->SharedKey[BSS0][KeyIdx]);

					if (pWepKey->KeyIndex & 0x80000000) {
						MAC_TABLE_ENTRY *pEntry = NULL;
						/* pEntry = GetAssociatedAPByWdev(pAd, wdev); */
						pEntry = &pAd->MacTab.Content[BSSID_WCID_TO_REMOVE];
					}
				}

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "OID_802_11_ADD_WEP (id=0x%x, Len=%d-byte), Port %s Secured\n",
						  pWepKey->KeyIndex, pWepKey->KeyLength,
						  (wdev->PortSecured == WPA_802_1X_PORT_SECURED) ? "" : "NOT");
			}
		}

		os_free_mem(pWepKey);
		break;
#ifdef WPA_SUPPLICANT_SUPPORT

	case OID_SET_COUNTERMEASURES:
		if (wrq->u.data.length != sizeof(int))
			Status  = -EINVAL;
		else {
			int enabled = 0;

			Status = copy_from_user(&enabled, wrq->u.data.pointer, wrq->u.data.length);

			if (enabled == 1)
				pStaCfg->bBlockAssoc = TRUE;
			else
				/* WPA MIC error should block association attempt for 60 seconds */
				pStaCfg->bBlockAssoc = FALSE;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_SET_COUNTERMEASURES bBlockAssoc=%s\n",
					 pStaCfg->bBlockAssoc ? "TRUE" : "FALSE");
		}

		break;

	case RT_OID_WPA_SUPPLICANT_SUPPORT:
		if (wrq->u.data.length != sizeof(UCHAR))
			Status  = -EINVAL;
		else {
			Status = copy_from_user(&wpa_supplicant_enable, wrq->u.data.pointer, wrq->u.data.length);

			if (wpa_supplicant_enable & WPA_SUPPLICANT_ENABLE_WPS)
				pStaCfg->wpa_supplicant_info.WpaSupplicantUP |= WPA_SUPPLICANT_ENABLE_WPS;
			else {
				pStaCfg->wpa_supplicant_info.WpaSupplicantUP = wpa_supplicant_enable;
				pStaCfg->wpa_supplicant_info.WpaSupplicantUP &= 0x7F;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WPA_SUPPLICANT_SUPPORT (=0x%02X)\n",
					 pStaCfg->wpa_supplicant_info.WpaSupplicantUP);
		}

		break;

	case OID_802_11_DEAUTHENTICATION:
		if (wrq->u.data.length != sizeof(MLME_DEAUTH_REQ_STRUCT))
			Status  = -EINVAL;
		else {
			MLME_DEAUTH_REQ_STRUCT      *pInfo;
			MLME_QUEUE_ELEM *MsgElem;

			os_alloc_mem(pAd, (UCHAR **)&MsgElem, sizeof(MLME_QUEUE_ELEM));

			if (MsgElem == NULL) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Alloc memory failed!\n");
				return -EINVAL;
			}

			pInfo = (MLME_DEAUTH_REQ_STRUCT *) MsgElem->Msg;
			Status = copy_from_user(pInfo, wrq->u.data.pointer, wrq->u.data.length);
			MlmeDeauthReqAction(pAd, MsgElem);
			os_free_mem(MsgElem);

			if (INFRA_ON(pStaCfg)) {
				LinkDown(pAd, FALSE, &pStaCfg->wdev, NULL);
				pStaCfg->AssocMachine.CurrState = ASSOC_IDLE;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_DEAUTHENTICATION (Reason=%d)\n", pInfo->Reason);
		}

		break;

	case OID_802_11_DROP_UNENCRYPTED:
		if (wrq->u.data.length != sizeof(int))
			Status  = -EINVAL;
		else {
			int enabled = 0;
			MAC_TABLE_ENTRY *pEntry = NULL;
			/* pEntry = GetAssociatedAPByWdev(pAd, wdev); */
			pEntry = &pAd->MacTab.Content[BSSID_WCID_TO_REMOVE];
			Status = copy_from_user(&enabled, wrq->u.data.pointer, wrq->u.data.length);

			if (enabled == 1)
				pStaCfg->wdev.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			else
				pStaCfg->wdev.PortSecured = WPA_802_1X_PORT_SECURED;

			NdisAcquireSpinLock(&pAd->MacTabLock);
			pAd->MacTab.tr_entry[pEntry->wcid].PortSecured = pStaCfg->wdev.PortSecured;
			NdisReleaseSpinLock(&pAd->MacTabLock);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_DROP_UNENCRYPTED (=%d)\n", enabled);
		}

		break;

	case OID_802_11_SET_IEEE8021X:
		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status  = -EINVAL;
		else {
			Status = copy_from_user(&IEEE8021xState, wrq->u.data.pointer, wrq->u.data.length);
			pStaCfg->wdev.SecConfig.IEEE8021X = IEEE8021xState;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_SET_IEEE8021X (=%d)\n", IEEE8021xState);
		}

		break;

	case OID_802_11_SET_IEEE8021X_REQUIRE_KEY:
		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status  = -EINVAL;
		else {
			Status = copy_from_user(&IEEE8021x_required_keys, wrq->u.data.pointer, wrq->u.data.length);
			pStaCfg->wpa_supplicant_info.IEEE8021x_required_keys = IEEE8021x_required_keys;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_SET_IEEE8021X_REQUIRE_KEY (%d)\n",
					 IEEE8021x_required_keys);
		}

		break;

	case OID_802_11_PMKID:
		os_alloc_mem(pAd, (UCHAR **)&pPmkId, wrq->u.data.length);

		if (pPmkId == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(pPmkId, wrq->u.data.pointer, wrq->u.data.length);

		/* check the PMKID information */
		if (pPmkId->BSSIDInfoCount == 0)
			NdisZeroMemory(pStaCfg->SavedPMK, sizeof(BSSID_INFO)*PMKID_NO);
		else {
			PBSSID_INFO	pBssIdInfo;
			UINT		BssIdx;
			UINT		CachedIdx;

			for (BssIdx = 0; BssIdx < pPmkId->BSSIDInfoCount; BssIdx++) {
				/* point to the indexed BSSID_INFO structure */
				pBssIdInfo = (PBSSID_INFO) ((PUCHAR) pPmkId + 2 * sizeof(UINT) + BssIdx * sizeof(BSSID_INFO));

				/* Find the entry in the saved data base. */
				for (CachedIdx = 0; CachedIdx < pStaCfg->SavedPMKNum; CachedIdx++) {
					/* compare the BSSID */
					if (NdisEqualMemory(pBssIdInfo->BSSID, pStaCfg->SavedPMK[CachedIdx].BSSID, sizeof(NDIS_802_11_MAC_ADDRESS)))
						break;
				}

				/* Found, replace it */
				if (CachedIdx < PMKID_NO) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Update OID_802_11_PMKID, idx = %d\n", CachedIdx);
					NdisMoveMemory(&pStaCfg->SavedPMK[CachedIdx], pBssIdInfo, sizeof(BSSID_INFO));
					NdisZeroMemory(&pStaCfg->SavedPMK[CachedIdx].PMK, LEN_PMK); /* Ellis: why no PMK in wpa supplicant path? */
					pStaCfg->SavedPMKNum++;
				}
				/* Not found, replace the last one */
				else {
					/* Randomly replace one */
					CachedIdx = (pBssIdInfo->BSSID[5] % PMKID_NO);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Update OID_802_11_PMKID, idx = %d\n", CachedIdx);
					NdisMoveMemory(&pStaCfg->SavedPMK[CachedIdx], pBssIdInfo, sizeof(BSSID_INFO));
					NdisZeroMemory(&pStaCfg->SavedPMK[CachedIdx].PMK, LEN_PMK); /* Ellis: why no PMK in wpa supplicant path? */
				}
			}
		}

		if (pPmkId)
			os_free_mem(pPmkId);

		break;

	case RT_OID_WPS_PROBE_REQ_IE:
		if (pStaCfg->wpa_supplicant_info.pWpsProbeReqIe) {
			os_free_mem(pStaCfg->wpa_supplicant_info.pWpsProbeReqIe);
			pStaCfg->wpa_supplicant_info.pWpsProbeReqIe = NULL;
		}

		pStaCfg->wpa_supplicant_info.WpsProbeReqIeLen = 0;
		os_alloc_mem(pAd, (UCHAR **) &(pStaCfg->wpa_supplicant_info.pWpsProbeReqIe), wrq->u.data.length);

		if (pStaCfg->wpa_supplicant_info.pWpsProbeReqIe) {
			Status = copy_from_user(pStaCfg->wpa_supplicant_info.pWpsProbeReqIe, wrq->u.data.pointer, wrq->u.data.length);

			if (Status) {
				Status  = -EINVAL;

				if (pStaCfg->wpa_supplicant_info.pWpsProbeReqIe) {
					os_free_mem(pStaCfg->wpa_supplicant_info.pWpsProbeReqIe);
					pStaCfg->wpa_supplicant_info.pWpsProbeReqIe = NULL;
				}

				pStaCfg->wpa_supplicant_info.WpsProbeReqIeLen = 0;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "RT_OID_WPS_PROBE_REQ_IE, Failed (copy_from_user failed)!!\n");
			} else {
				pStaCfg->wpa_supplicant_info.WpsProbeReqIeLen = wrq->u.data.length;
				hex_dump("WpsProbeReqIe",
						 pStaCfg->wpa_supplicant_info.pWpsProbeReqIe,
						 pStaCfg->wpa_supplicant_info.WpsProbeReqIeLen);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WPS_PROBE_REQ_IE, WpsProbeReqIeLen = %d!!\n",
						 pStaCfg->wpa_supplicant_info.WpsProbeReqIeLen);
			}
		} else
			Status = -ENOMEM;

		break;
#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef WSC_STA_SUPPORT

	case RT_OID_WSC_EAPMSG: {
		RTMP_WSC_U2KMSG_HDR *msgHdr = NULL;
		PUCHAR pUPnPMsg = NULL;
		UINT msgLen = 0, Machine = 0, msgType = 0;
		int retVal, senderID = 0;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_EAPMSG, wrq->u.data.length=%d!\n",
				 wrq->u.data.length);
		msgLen = wrq->u.data.length;
		os_alloc_mem(pAd, (UCHAR **)&pUPnPMsg, msgLen);

		if (pUPnPMsg == NULL)
			Status = -EINVAL;
		else {
			memset(pUPnPMsg, 0, msgLen);
			retVal = copy_from_user(pUPnPMsg, wrq->u.data.pointer, msgLen);
			if (retVal) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Copy_from_user fail[%d]!\n", retVal);
			}
			msgHdr = (RTMP_WSC_U2KMSG_HDR *)pUPnPMsg;
			senderID = *((int *)&msgHdr->Addr2);
			/*assign the STATE_MACHINE type */
			{
				Machine = WSC_STATE_MACHINE;
				msgType = WSC_EAPOL_UPNP_MSG;
				retVal = MlmeEnqueueForWsc(pAd, msgHdr->envID, senderID, Machine, msgType, msgLen, pUPnPMsg, wdev);

				if ((retVal == FALSE) && (msgHdr->envID != 0)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "MlmeEnqueuForWsc return False and envID=0x%x!\n",
							 msgHdr->envID);
					Status = -EINVAL;
				}
			}
			os_free_mem(pUPnPMsg);
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_EAPMSG finished!\n");
	}
	break;

	case RT_OID_WSC_SET_PROFILE:
		if (wrq->u.data.length != sizeof(WSC_PROFILE))
			Status = -EINVAL;
		else {
			PWSC_PROFILE pWscProfile = &wdev->WscControl.WscProfile;

			NdisZeroMemory(pWscProfile, sizeof(WSC_PROFILE));
			Status = copy_from_user(pWscProfile, wrq->u.data.pointer, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_SET_PROFILE:: ProfileCnt = %d\n",
					 pWscProfile->ProfileCnt);
		}

		break;

	case RT_OID_WSC_SET_CONF_MODE: /* WPS disable, Enrollee or Registrar */
		if (wrq->u.data.length != sizeof(INT))
			Status = -EINVAL;
		else {
			INT WscConfMode = 0;

			Status = copy_from_user(&WscConfMode, wrq->u.data.pointer, wrq->u.data.length);

			if (Status == 0) {
				if (WscConfMode == 2)
					WscConfMode = 4;

				switch (WscConfMode) {
				case WSC_ENROLLEE:
					Set_WscConfMode_Proc(pAd, "1");
					break;

				case WSC_REGISTRAR:
					Set_WscConfMode_Proc(pAd, "2");
					WscConfMode = 2;
					break;

				case WSC_DISABLE:
				default:
					Set_WscConfMode_Proc(pAd, "0");
					break;
				}
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_SET_CONF_MODE:: WscConfMode = %d\n", WscConfMode);
		}

		break;

	case RT_OID_WSC_SET_MODE:
		if (wrq->u.data.length != sizeof(INT))
			Status = -EINVAL;
		else {
			INT WscMode = 0; /* PIN or PBC */

			Status = copy_from_user(&WscMode, wrq->u.data.pointer, wrq->u.data.length);

			if (Status == 0) {
				if (WscMode == WSC_PIN_MODE) { /* PIN */
					if (Set_WscMode_Proc(pAd, "1") == FALSE)
						Status = -EINVAL;
				} else if (WscMode == WSC_PBC_MODE) { /* PBC */
					if (Set_WscMode_Proc(pAd, "2") == FALSE)
						Status = -EINVAL;
				} else if (WscMode == WSC_SMPBC_MODE) { /* SMPBC */
					if (Set_WscMode_Proc(pAd, "3") == FALSE)
						Status = -EINVAL;
				} else {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_WSC_SET_MODE::unknown WscMode = %d\n", WscMode);
					Status = -EINVAL;
				}
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_SET_MODE::WscMode = %d\n", WscMode);
		}

		break;

	case RT_OID_WSC_SET_PIN_CODE:
		if (wrq->u.data.length != 8) /* PIN Code Length is 8 */
			Status = -EINVAL;
		else {
			CHAR PinCode[9] = {0};

			Status = copy_from_user(&PinCode[0], wrq->u.data.pointer, wrq->u.data.length);

			if (Status == 0) {
				if (Set_WscPinCode_Proc(pAd, (RTMP_STRING *) &PinCode[0]) == FALSE)
					Status = -EINVAL;
			}
		}

		break;

	case RT_OID_WSC_SET_SSID:
		if (wrq->u.data.length != sizeof(NDIS_802_11_SSID))
			Status = -EINVAL;
		else {
			NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
			Status = copy_from_user(&Ssid, wrq->u.data.pointer, wrq->u.data.length);
			Ssid.Ssid[NDIS_802_11_LENGTH_SSID - 1] = 0x00;
			Set_WscSsid_Proc(pAd, (RTMP_STRING *) Ssid.Ssid);
		}

		break;

	case RT_OID_WSC_SET_CONN_BY_PROFILE_INDEX:
		if (wrq->u.data.length != sizeof(UINT))
			Status = -EINVAL;
		else {
			UINT wsc_profile_index = 0; /* PIN or PBC */
			PWSC_CTRL   pWscControl = &wdev->WscControl;
			unsigned long	IrqFlags;

			Status = copy_from_user(&wsc_profile_index, wrq->u.data.pointer, wrq->u.data.length);

			if (wsc_profile_index < pWscControl->WscProfile.ProfileCnt) {
				RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
				WscWriteConfToPortCfg(pAd, pWscControl, &pWscControl->WscProfile.Profile[wsc_profile_index], TRUE);
				RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
				pStaCfg->MlmeAux.CurrReqIsFromNdis = TRUE;
				LinkDown(pAd, TRUE, &pStaCfg->wdev, NULL);
			} else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "RT_OID_WSC_SET_CONN_BY_PROFILE_INDEX:: wrong wsc_profile_index(%d)\n", wsc_profile_index);
		}

		break;

	case RT_OID_WSC_DRIVER_AUTO_CONNECT:
		if (wrq->u.data.length != sizeof(UCHAR))
			Status = -EINVAL;
		else {
			Status = copy_from_user(&wdev->WscControl.WscDriverAutoConnect, wrq->u.data.pointer, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_DRIVER_AUTO_CONNECT::WscDriverAutoConnect is %d\n",
					 wdev->WscControl.WscDriverAutoConnect);
		}

		break;

	case RT_OID_WSC_SET_PASSPHRASE:
		if (wrq->u.data.length > 64 || wrq->u.data.length < 8)
			Status = -EINVAL;
		else {
			Status = copy_from_user(wdev->WscControl.WpaPsk, wrq->u.data.pointer, wrq->u.data.length);
			NdisZeroMemory(wdev->WscControl.WpaPsk, 64);
			wdev->WscControl.WpaPskLen = wrq->u.data.length;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_SET_PASSPHRASE::KeyLen(%d)\n",
					 wdev->WscControl.WpaPskLen);
		}

		break;
#endif /* WSC_STA_SUPPORT */
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT

	case RT_OID_LED_WPS_MODE10:
		if (!(pStaCfg->bRadio) ||
			(wrq->u.data.length != sizeof(UINT)))
			Status = -EINVAL;
		else {
			Status = copy_from_user(&WPSLedMode10, wrq->u.data.pointer, wrq->u.data.length);

			if ((WPSLedMode10 != LINK_STATUS_WPS_MODE10_TURN_ON) &&
				(WPSLedMode10 != LINK_STATUS_WPS_MODE10_FLASH) &&
				(WPSLedMode10 != LINK_STATUS_WPS_MODE10_TURN_OFF)) {
				Status = NDIS_STATUS_INVALID_DATA;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "WPS LED Mode 10::Parameter of LED Mode 10 must be 0x00, or 0x01, or 0x02\n");
			} else
				RTMP_SET_LED(pAd, WPSLedMode10, HcGetBandByWdev(wdev));
		}

		break;
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef SNMP_SUPPORT

	case OID_802_11_SHORTRETRYLIMIT:
		if (wrq->u.data.length != sizeof(ULONG))
			Status = -EINVAL;
		else {
			Status = copy_from_user(&ShortRetryLimit, wrq->u.data.pointer, wrq->u.data.length);
			AsicSetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_SHORT, ShortRetryLimit);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_SHORTRETRYLIMIT (ShortRetryLimit=%ld)\n",
					 ShortRetryLimit);
		}

		break;

	case OID_802_11_LONGRETRYLIMIT:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_LONGRETRYLIMIT\n");

		if (wrq->u.data.length != sizeof(ULONG))
			Status = -EINVAL;
		else {
			Status = copy_from_user(&LongRetryLimit, wrq->u.data.pointer, wrq->u.data.length);
			AsicSetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_LONG, LongRetryLimit);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_LONGRETRYLIMIT (LongRetryLimit=%ld)\n",
					 LongRetryLimit);
		}

		break;

	case OID_802_11_WEPDEFAULTKEYVALUE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_WEPDEFAULTKEYVALUE\n");
		os_alloc_mem(pAd, (UCHAR **)&pKey, wrq->u.data.length);

		if (pKey == NULL) {
			Status = -EINVAL;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_WEPDEFAULTKEYVALUE, Failed!!\n");
			break;
		}

		Status = copy_from_user(pKey, wrq->u.data.pointer, wrq->u.data.length);

		/*pKey = &WepKey; */

		if (pKey->Length != wrq->u.data.length) {
			Status = -EINVAL;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_WEPDEFAULTKEYVALUE, Failed!!\n");
		}

		KeyIdx = pKey->KeyIndex & 0x0fffffff;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pKey->KeyIndex =%d, pKey->KeyLength=%d\n", pKey->KeyIndex,
				 pKey->KeyLength);
		if ((UCHAR) pKey->KeyLength > ARRAY_SIZE(pAd->SharedKey[BSS0][pStaCfg->DefaultKeyId].Key))
			StaKeyLen = ARRAY_SIZE(pAd->SharedKey[BSS0][pStaCfg->DefaultKeyId].Key);
		else
			StaKeyLen = (UCHAR) pKey->KeyLength;
		/* it is a shared key */
		if (KeyIdx >= 4)
			Status = -EINVAL;
		else {
			pAd->SharedKey[BSS0][pStaCfg->DefaultKeyId].KeyLen = StaKeyLen;
			NdisMoveMemory(&pAd->SharedKey[BSS0][pStaCfg->DefaultKeyId].Key, &pKey->KeyMaterial, StaKeyLen);

			if (pKey->KeyIndex & 0x80000000) {
				/* Default key for tx (shared key) */
				pStaCfg->DefaultKeyId = (UCHAR) KeyIdx;
			}

			/*RestartAPIsRequired = TRUE; */
		}

		os_free_mem(pKey);
		break;

	case OID_802_11_WEPDEFAULTKEYID:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_WEPDEFAULTKEYID\n");

		if (wrq->u.data.length != sizeof(UCHAR))
			Status = -EINVAL;
		else
			Status = copy_from_user(&pStaCfg->DefaultKeyId, wrq->u.data.pointer, wrq->u.data.length);

		break;

	case OID_802_11_CURRENTCHANNEL:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_CURRENTCHANNEL\n");

		if (wrq->u.data.length != sizeof(UCHAR))
			Status = -EINVAL;
		else {
			RTMP_STRING ChStr[5] = {0};

			Status = copy_from_user(&ctmp, wrq->u.data.pointer, wrq->u.data.length);
			snprintf(ChStr, sizeof(ChStr), "%d", ctmp);
			Set_Channel_Proc(pAd, ChStr);
		}

		break;
#endif /* SNMP_SUPPORT */
#ifdef XLINK_SUPPORT

	case RT_OID_802_11_SET_PSPXLINK_MODE:
		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status  = -EINVAL;
		else {
			Status = copy_from_user(&pStaCfg->PSPXlink, wrq->u.data.pointer, wrq->u.data.length);
			/*if (pStaCfg->PSPXlink)
			 *	RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_PROMISCUOUS)
			 */
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_SET_PSPXLINK_MODE(=%d)\n",
					 pStaCfg->PSPXlink);
		}

		break;
#endif /* XLINK_SUPPORT */
#ifdef DOT11R_FT_SUPPORT

	case OID_802_11R_SUPPORT:
		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status  = -EINVAL;
		else {
			Status = copy_from_user(&pStaCfg->Dot11RCommInfo.bFtSupport, wrq->u.data.pointer, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11R_SUPPORT(=%d)\n",
					 pStaCfg->Dot11RCommInfo.bFtSupport);
		}

		break;
#endif /* DOT11R_FT_SUPPORT */
#ifdef WIDI_SUPPORT

	case RT_OID_INTEL_WIDI:
		if (pStaCfg->bWIDI) {
			if ((wrq->u.data.length < 1) || (wrq->u.data.length > sizeof(WIDI_SERVICE_MSG))) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_INTEL_WIDI : len=(%d) not correct!\n",
						 wrq->u.data.length);
				Status = -EINVAL;
				break;
			} else {
				WIDI_SERVICE_MSG msg;

				NdisZeroMemory(&msg, sizeof(WIDI_SERVICE_MSG));
				Status = copy_from_user(&msg, wrq->u.data.pointer, wrq->u.data.length);

				if (msg.type != WIDI_MSG_TYPE_SERVICE) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_INTEL_WIDI : Invalid msg_type(%d) received!\n",
							 msg.type);
					Status = -EINVAL;
					break;
				}

				NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
				WidiSendProbeRequest(pAd, msg.dst_mac, 0, Ssid.Ssid, msg.device_name, msg.primary_dev, msg.ext + 4,
									 WIDI_SERVICE_VE_LEN - 4, msg.channel);
			}
		}

		break;

	case RT_OID_WSC_GEN_PIN_CODE:
		if (wrq->u.data.length != 1) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_WSC_GEN_PIN_CODE : len=(%d) not correct!\n",
					 wrq->u.data.length);
			Status = -EINVAL;
			break;
		} else {
			UCHAR Enable;

			Status = copy_from_user(&Enable, wrq->u.data.pointer, wrq->u.data.length);

			if (Enable) {
				if (pStaCfg->WscControl.WscEnrollee4digitPinCode) {
					pStaCfg->WscControl.WscEnrolleePinCodeLen = 4;
					pStaCfg->WscControl.WscEnrolleePinCode = WscRandomGen4digitPinCode(pAd);
				} else {
					pStaCfg->WscControl.WscEnrolleePinCodeLen = 8;
					pStaCfg->WscControl.WscEnrolleePinCode = WscSpecialRandomGeneratePinCode(pAd);
				}
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_GEN_PIN_CODE (=%d)\n", Enable);
		}

		break;
#endif /* WIDI_SUPPORT */

	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "unknown IOCTL's subcmd = 0x%08x\n", cmd);
		Status = -EOPNOTSUPP;
		break;
	}

	return Status;
}

INT RTMPQueryInformation(
	IN  PRTMP_ADAPTER pAd,
	IN  OUT RTMP_IOCTL_INPUT_STRUCT    *rq,
	IN  INT                 cmd,
	IN struct wifi_dev *wdev)
{
	RTMP_IOCTL_INPUT_STRUCT				*wrq = (RTMP_IOCTL_INPUT_STRUCT *) rq;
	NDIS_802_11_BSSID_LIST_EX           *pBssidList = NULL;
	PNDIS_WLAN_BSSID_EX                 pBss;
	NDIS_802_11_SSID                    Ssid;
	NDIS_802_11_CONFIGURATION           *pConfiguration = NULL;
	RT_802_11_LINK_STATUS               *pLinkStatus = NULL;
	RT_802_11_STA_CONFIG                *pStaConfig = NULL;
	NDIS_802_11_STATISTICS              *pStatistics = NULL;
	NDIS_802_11_RTS_THRESHOLD           RtsThresh;
	NDIS_802_11_FRAGMENTATION_THRESHOLD FragThresh;
	NDIS_802_11_POWER_MODE              PowerMode;
	NDIS_802_11_NETWORK_INFRASTRUCTURE  BssType;
	RT_802_11_PREAMBLE                  PreamType;
	NDIS_802_11_AUTHENTICATION_MODE AuthMode;
	NDIS_802_11_WEP_STATUS WepStatus;
	NDIS_MEDIA_STATE MediaState;
	ULONG BssBufSize, ulInfo = 0, NetworkTypeList[4], apsd = 0, RateValue = 0;
	USHORT BssLen = 0;
	PUCHAR pBuf = NULL, pPtr;
	INT Status = NDIS_STATUS_SUCCESS;
	UINT we_version_compiled;
	UCHAR Padding = 0;
	UINT i;
	INT ret;
	BOOLEAN RadioState;
	RTMP_STRING driverVersion[8];
	OID_SET_HT_PHYMODE *pHTPhyMode = NULL;
	HTTRANSMIT_SETTING	HTPhyMode;
#ifdef WSC_STA_SUPPORT
	UINT	                            WscPinCode = 0;
	PWSC_PROFILE						pProfile;
#endif /* WSC_STA_SUPPORT */
#ifdef SNMP_SUPPORT
	DefaultKeyIdxValue			*pKeyIdxValue;
	INT							valueLen;
	TX_RTY_CFG_STRUC			tx_rty_cfg;
	ULONG						ShortRetryLimit, LongRetryLimit;
	UCHAR						tmp[64];
#endif /*SNMP */
#ifdef P2P_SUPPORT
	PRT_P2P_UI_TABLE pUI_table;
	PRT_P2P_TABLE			pP2pTable;
	PRT_P2P_CLIENT_ENTRY   pPAdCli, pUICli;
	PRT_P2P_CONFIG	pP2PCtrl; /* = &pAd->P2pCfg; */
	UCHAR tmpP2P[24];
#endif /* P2P_SUPPORT */
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
	UINT32 Freq = 0;
	ULONG TxPowerPercentage;
	/* pEntry = GetAssociatedAPByWdev(pAd, wdev); */
	pEntry = &pAd->MacTab.Content[BSSID_WCID_TO_REMOVE];
	BandIdx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	switch (cmd) {
	case RT_OID_DEVICE_NAME:
		wrq->u.data.length = sizeof(pAd->nickname);
		Status = copy_to_user(wrq->u.data.pointer, pAd->nickname, wrq->u.data.length);
		break;

	case RT_OID_VERSION_INFO:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_VERSION_INFO\n");
		wrq->u.data.length = 8 * sizeof(CHAR);
		ret = snprintf(&driverVersion[0], sizeof(driverVersion), "%s", STA_DRIVER_VERSION);
		if (os_snprintf_error(sizeof(driverVersion), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "driverVersion snprintf error!\n");
			return NDIS_STATUS_FAILURE;
		}
		driverVersion[7] = '\0';

		if (copy_to_user(wrq->u.data.pointer, &driverVersion[0], wrq->u.data.length))
			Status = -EFAULT;

		break;

	case OID_802_11_BSSID_LIST:
#ifdef WPA_SUPPLICANT_SUPPORT

		if ((pStaCfg->wpa_supplicant_info.WpaSupplicantUP & 0x7F) == WPA_SUPPLICANT_ENABLE)
			pStaCfg->wpa_supplicant_info.WpaSupplicantScanCount = 0;

#endif /* WPA_SUPPLICANT_SUPPORT */
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_BSSID_LIST (%d BSS returned)\n",
				 ScanTab->BssNr);
		pStaCfg->bSkipAutoScanConn = FALSE;
		/* Claculate total buffer size required */
		BssBufSize = sizeof(ULONG);

		for (i = 0; i < ScanTab->BssNr; i++) {
			/* Align pointer to 4 bytes boundary. */
			/*Padding = 4 - (pAd->ScanTab.BssEntry[i].VarIELen & 0x0003); */
			/*if (Padding == 4) */
			/*    Padding = 0; */
			BssBufSize += (sizeof(NDIS_WLAN_BSSID_EX) - 1 + sizeof(NDIS_802_11_FIXED_IEs) + ScanTab->BssEntry[i].VarIELen + Padding);
		}

		/* For safety issue, we add 256 bytes just in case */
		BssBufSize += 256;
		/* Allocate the same size as passed from higher layer */
		os_alloc_mem(pAd, (UCHAR **)&pBuf, BssBufSize);

		if (pBuf == NULL) {
			Status = -ENOMEM;
			break;
		}

		/* Init 802_11_BSSID_LIST_EX structure */
		NdisZeroMemory(pBuf, BssBufSize);
		pBssidList = (PNDIS_802_11_BSSID_LIST_EX) pBuf;
		pBssidList->NumberOfItems = ScanTab->BssNr;
		/* Calculate total buffer length */
		BssLen = 4; /* Consist of NumberOfItems */
		/* Point to start of NDIS_WLAN_BSSID_EX */
		/* pPtr = pBuf + sizeof(ULONG); */
		pPtr = (PUCHAR) &pBssidList->Bssid[0];

		for (i = 0; i < ScanTab->BssNr; i++) {
			pBss = (PNDIS_WLAN_BSSID_EX) pPtr;
			NdisMoveMemory(&pBss->MacAddress, &ScanTab->BssEntry[i].Bssid, MAC_ADDR_LEN);

			if ((ScanTab->BssEntry[i].Hidden == 1) && (pStaCfg->bShowHiddenSSID == FALSE)) {
				/* */
				/* We must return this SSID during 4way handshaking, otherwise Aegis will failed to parse WPA infomation */
				/* and then failed to send EAPOl farame. */
				/* */
				if (IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap) && (wdev->PortSecured != WPA_802_1X_PORT_SECURED)) {
					pBss->Ssid.SsidLength = ScanTab->BssEntry[i].SsidLen;
					NdisMoveMemory(pBss->Ssid.Ssid, ScanTab->BssEntry[i].Ssid, ScanTab->BssEntry[i].SsidLen);
				} else
					pBss->Ssid.SsidLength = 0;
			} else {
				pBss->Ssid.SsidLength = ScanTab->BssEntry[i].SsidLen;
				NdisMoveMemory(pBss->Ssid.Ssid, ScanTab->BssEntry[i].Ssid, ScanTab->BssEntry[i].SsidLen);
			}

			pBss->Privacy = ScanTab->BssEntry[i].Privacy;
			pBss->Rssi = ScanTab->BssEntry[i].Rssi - pAd->BbpRssiToDbmDelta;
			pBss->MinSNR = ScanTab->BssEntry[i].MinSNR;
			pBss->NetworkTypeInUse = NetworkTypeInUseSanity(&ScanTab->BssEntry[i]);
			pBss->Configuration.Length = sizeof(NDIS_802_11_CONFIGURATION);
			pBss->Configuration.BeaconPeriod = ScanTab->BssEntry[i].BeaconPeriod;
			pBss->Configuration.ATIMWindow = ScanTab->BssEntry[i].AtimWin;
			/*NdisMoveMemory(&pBss->QBssLoad, &pAd->ScanTab.BssEntry[i].QbssLoad, sizeof(QBSS_LOAD_UI)); */
			MAP_CHANNEL_ID_TO_KHZ(ScanTab->BssEntry[i].Channel, Freq);
			pBss->Configuration.DSConfig = (ULONG)Freq;

			if (ScanTab->BssEntry[i].BssType == BSS_INFRA)
				pBss->InfrastructureMode = Ndis802_11Infrastructure;
			else
				pBss->InfrastructureMode = Ndis802_11IBSS;

			NdisMoveMemory(pBss->SupportedRates, ScanTab->BssEntry[i].SupRate, ScanTab->BssEntry[i].SupRateLen);
			NdisMoveMemory(pBss->SupportedRates + ScanTab->BssEntry[i].SupRateLen,
						   ScanTab->BssEntry[i].ExtRate,
						   ScanTab->BssEntry[i].ExtRateLen);

			if (ScanTab->BssEntry[i].VarIELen == 0) {
				pBss->IELength = sizeof(NDIS_802_11_FIXED_IEs);
				NdisMoveMemory(pBss->IEs, &ScanTab->BssEntry[i].FixIEs, sizeof(NDIS_802_11_FIXED_IEs));
				pPtr = pPtr + sizeof(NDIS_WLAN_BSSID_EX) - 1 + sizeof(NDIS_802_11_FIXED_IEs);
			} else {
				pBss->IELength = (ULONG)(sizeof(NDIS_802_11_FIXED_IEs) + ScanTab->BssEntry[i].VarIELen);
				pPtr = pPtr + sizeof(NDIS_WLAN_BSSID_EX) - 1 + sizeof(NDIS_802_11_FIXED_IEs);
				NdisMoveMemory(pBss->IEs, &ScanTab->BssEntry[i].FixIEs, sizeof(NDIS_802_11_FIXED_IEs));
				NdisMoveMemory(pBss->IEs + sizeof(NDIS_802_11_FIXED_IEs), ScanTab->BssEntry[i].VarIEs, ScanTab->BssEntry[i].VarIELen);
				pPtr += ScanTab->BssEntry[i].VarIELen;
			}

			pBss->Length = (ULONG)(sizeof(NDIS_WLAN_BSSID_EX) - 1 + sizeof(NDIS_802_11_FIXED_IEs) + ScanTab->BssEntry[i].VarIELen + Padding);

			if (RtmpOsWirelessExtVerGet() < 17) {
				if ((BssLen + pBss->Length) < wrq->u.data.length)
					BssLen += pBss->Length;
				else {
					pBssidList->NumberOfItems = i;
					break;
				}
			} else
				BssLen += pBss->Length;

		}

		if (RtmpOsWirelessExtVerGet() < 17)
			wrq->u.data.length = BssLen;
		else {
			if (BssLen > wrq->u.data.length) {
				os_free_mem(pBssidList);
				return -E2BIG;
			} else
				wrq->u.data.length = BssLen;
		}

		Status = copy_to_user(wrq->u.data.pointer, pBssidList, BssLen);
		os_free_mem(pBssidList);
		break;

	case OID_802_3_CURRENT_ADDRESS:
		wrq->u.data.length = MAC_ADDR_LEN;
		Status = copy_to_user(wrq->u.data.pointer, wdev->if_addr, wrq->u.data.length);
		break;

	case OID_GEN_MEDIA_CONNECT_STATUS:
		if (pAd->IndicateMediaState == NdisMediaStateConnected)
			MediaState = NdisMediaStateConnected;
		else
			MediaState = NdisMediaStateDisconnected;

		wrq->u.data.length = sizeof(NDIS_MEDIA_STATE);
		Status = copy_to_user(wrq->u.data.pointer, &MediaState, wrq->u.data.length);
		break;

	case OID_802_11_BSSID:
		if (INFRA_ON(pStaCfg) || ADHOC_ON(pAd)) {
			Status = copy_to_user(wrq->u.data.pointer, &pStaCfg->Bssid, sizeof(NDIS_802_11_MAC_ADDRESS));
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_BSSID(=EMPTY)\n");
			Status = -ENOTCONN;
		}

		break;

	case OID_802_11_SSID:
		NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
		NdisZeroMemory(Ssid.Ssid, MAX_LEN_OF_SSID);
		Ssid.SsidLength = pStaCfg->SsidLen;
		memcpy(Ssid.Ssid, pStaCfg->Ssid,	Ssid.SsidLength);
		wrq->u.data.length = sizeof(NDIS_802_11_SSID);
		Status = copy_to_user(wrq->u.data.pointer, &Ssid, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_SSID (Len=%d, ssid=%s)\n", Ssid.SsidLength,
				 Ssid.Ssid);
		break;

	case RT_OID_802_11_QUERY_LINK_STATUS:
		os_alloc_mem(pAd, (UCHAR **)&pLinkStatus, sizeof(RT_802_11_LINK_STATUS));

		if (pLinkStatus) {
			pLinkStatus->CurrTxRate = RateIdTo500Kbps[pAd->CommonCfg.TxRate];   /* unit : 500 kbps */
			pLinkStatus->ChannelQuality = pStaCfg->ChannelQuality;
			pLinkStatus->RxByteCount = pAd->RalinkCounters.ReceivedByteCount;
			pLinkStatus->TxByteCount = pAd->RalinkCounters.TransmittedByteCount;
			pLinkStatus->CentralChannel = wlan_operate_get_cen_ch_1(&pStaCfg->wdev);
			wrq->u.data.length = sizeof(RT_802_11_LINK_STATUS);
			Status = copy_to_user(wrq->u.data.pointer, pLinkStatus, wrq->u.data.length);
			os_free_mem(pLinkStatus);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_QUERY_LINK_STATUS\n");
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_802_11_QUERY_LINK_STATUS(mem alloc failed)\n");
			Status = -EFAULT;
		}

		break;

	case OID_802_11_CONFIGURATION:
		os_alloc_mem(pAd, (UCHAR **)&pConfiguration, sizeof(NDIS_802_11_CONFIGURATION));

		if (pConfiguration) {
			pConfiguration->Length = sizeof(NDIS_802_11_CONFIGURATION);
			pConfiguration->BeaconPeriod = pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)];
			pConfiguration->ATIMWindow = pStaCfg->StaActive.AtimWin;
			MAP_CHANNEL_ID_TO_KHZ(wdev->channel, Freq);
			pConfiguration->DSConfig = (ULONG)Freq;
			wrq->u.data.length = sizeof(NDIS_802_11_CONFIGURATION);
			Status = copy_to_user(wrq->u.data.pointer, pConfiguration, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "OID_802_11_CONFIGURATION(BeaconPeriod=%ld,AtimW=%ld,Channel=%d)\n",
					  pConfiguration->BeaconPeriod, pConfiguration->ATIMWindow, wdev->channel);
			os_free_mem(pConfiguration);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_CONFIGURATION(mem alloc failed)\n");
			Status = -EFAULT;
		}

		break;

	case RT_OID_802_11_SNR_0:
		if ((pStaCfg->wdev.LastSNR0 > 0)) {
			ulInfo = ConvertToSnr(pAd, pStaCfg->wdev.LastSNR0);
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_SNR_0(0x=%lx)\n", ulInfo);
		} else
			Status = -EFAULT;

		break;

	case RT_OID_802_11_SNR_1:
		if ((pAd->Antenna.field.RxPath > 1) && (pStaCfg->wdev.LastSNR1 > 0)) {
			ulInfo = ConvertToSnr(pAd, pStaCfg->wdev.LastSNR1);
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_SNR_1(0x=%lx)\n", ulInfo);
		} else
			Status = -EFAULT;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_SNR_1(pStaCfg->LastSNR1=%d)\n",
				 pStaCfg->wdev.LastSNR1);
		break;

	case OID_802_11_RSSI_TRIGGER:
		ulInfo = pStaCfg->RssiSample.LastRssi[0] - pAd->BbpRssiToDbmDelta;
		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_RSSI_TRIGGER(=%ld)\n", ulInfo);
		break;

	case OID_802_11_RSSI:
	case RT_OID_802_11_RSSI:
		ulInfo = pStaCfg->RssiSample.LastRssi[0];
		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
		break;

	case RT_OID_802_11_RSSI_1:
		ulInfo = pStaCfg->RssiSample.LastRssi[1];
		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
		break;

	case RT_OID_802_11_RSSI_2:
		ulInfo = pStaCfg->RssiSample.LastRssi[2];
		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
		break;

	case OID_802_11_STATISTICS:
		os_alloc_mem(pAd, (UCHAR **)&pStatistics, sizeof(NDIS_802_11_STATISTICS));

		if (pStatistics) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_STATISTICS\n");
			/* add the most up-to-date h/w raw counters into software counters */
			NICUpdateRawCountersNew(pAd);

			/* Sanity check for calculation of sucessful count */
			if (pAd->WlanCounters[0].TransmittedFragmentCount.QuadPart < pAd->WlanCounters[0].RetryCount.QuadPart)
				pAd->WlanCounters[0].TransmittedFragmentCount.QuadPart = pAd->WlanCounters[0].RetryCount.QuadPart;

			pStatistics->TransmittedFragmentCount.QuadPart = pAd->WlanCounters[0].TransmittedFragmentCount.QuadPart +
					pAd->WlanCounters[0].MulticastTransmittedFrameCount.QuadPart;
			pStatistics->MulticastTransmittedFrameCount.QuadPart = pAd->WlanCounters[0].MulticastTransmittedFrameCount.QuadPart;
			pStatistics->FailedCount.QuadPart = pAd->WlanCounters[0].FailedCount.QuadPart;
			pStatistics->RetryCount.QuadPart = pAd->WlanCounters[0].RetryCount.QuadPart;
			pStatistics->MultipleRetryCount.QuadPart = pAd->WlanCounters[0].MultipleRetryCount.QuadPart;
			pStatistics->RTSSuccessCount.QuadPart = pAd->WlanCounters[0].RTSSuccessCount.QuadPart;
			pStatistics->RTSFailureCount.QuadPart = pAd->WlanCounters[0].RTSFailureCount.QuadPart;
			pStatistics->ACKFailureCount.QuadPart = pAd->WlanCounters[0].ACKFailureCount.QuadPart;
			pStatistics->FrameDuplicateCount.QuadPart = pAd->WlanCounters[0].FrameDuplicateCount.QuadPart;
			pStatistics->ReceivedFragmentCount.QuadPart = pAd->WlanCounters[0].ReceivedFragmentCount.QuadPart;
			pStatistics->MulticastReceivedFrameCount.QuadPart = pAd->WlanCounters[0].MulticastReceivedFrameCount.QuadPart;
#ifdef DBG
			pStatistics->FCSErrorCount = pAd->RalinkCounters.RealFcsErrCount;
#else
			pStatistics->FCSErrorCount.QuadPart = pAd->WlanCounters[0].FCSErrorCount.QuadPart;
			pStatistics->FrameDuplicateCount.u.LowPart = pAd->WlanCounters[0].FrameDuplicateCount.u.LowPart / 100;
#endif
			pStatistics->TransmittedFrameCount.QuadPart = pAd->WlanCounters[0].TransmittedFragmentCount.QuadPart;
			pStatistics->WEPUndecryptableCount.QuadPart = pAd->WlanCounters[0].WEPUndecryptableCount.QuadPart;
			wrq->u.data.length = sizeof(NDIS_802_11_STATISTICS);
			Status = copy_to_user(wrq->u.data.pointer, pStatistics, wrq->u.data.length);
			os_free_mem(pStatistics);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_STATISTICS(mem alloc failed)\n");
			Status = -EFAULT;
		}

		break;
#ifdef TXBF_SUPPORT

	case RT_OID_802_11_QUERY_TXBF_TABLE:
	{
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

		if (cap->FlgHwTxBfCap) {
			INT i;
			RT_802_11_TXBF_TABLE *pMacTab;

			os_alloc_mem(pAd, (UCHAR **)&pMacTab, sizeof(RT_802_11_TXBF_TABLE));

			if (pMacTab) {
				pMacTab->Num = 0;

				for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
					if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) && (pAd->MacTab.Content[i].Sst == SST_ASSOC)) {
						NdisMoveMemory(&pMacTab->Entry[pMacTab->Num], &pAd->MacTab.Content[i].TxBFCounters, sizeof(RT_COUNTER_TXBF));
						pMacTab->Num++;
					}
				}

				wrq->u.data.length = sizeof(RT_802_11_TXBF_TABLE);
				Status = copy_to_user(wrq->u.data.pointer, pMacTab, wrq->u.data.length);
				os_free_mem(pMacTab);
			} else
				Status = -EFAULT;
		} else
			Status = -EFAULT;

		break;
	}
#endif /* TXBF_SUPPORT */

	case OID_GEN_RCV_OK:
		ulInfo = pAd->Counters8023.GoodReceives;
		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		break;

	case OID_GEN_RCV_NO_BUFFER:
		ulInfo = pAd->Counters8023.RxNoBuffer;
		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		break;

	case RT_OID_802_11_PHY_MODE:
		ulInfo = (ULONG)wdev->PhyMode;
		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_PHY_MODE (=%ld)\n", ulInfo);
		break;

	case RT_OID_802_11_STA_CONFIG:
		os_alloc_mem(pAd, (UCHAR **)&pStaConfig, sizeof(RT_802_11_STA_CONFIG));

		if (pStaConfig) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_STA_CONFIG\n");
			pStaConfig->EnableTxBurst = pAd->CommonCfg.bEnableTxBurst;
			pStaConfig->EnableTurboRate = 0;
			pStaConfig->UseBGProtection = pAd->CommonCfg.UseBGProtection;
			pStaConfig->UseShortSlotTime = pAd->CommonCfg.bUseShortSlotTime[BandIdx];
			/*pStaConfig->AdhocMode = pStaCfg->AdhocMode; */
			pStaConfig->HwRadioStatus = (pStaCfg->bHwRadio == TRUE) ? 1 : 0;
			pStaConfig->Rsv1 = 0;
			pStaConfig->SystemErrorBitmap = pAd->SystemErrorBitmap;
			wrq->u.data.length = sizeof(RT_802_11_STA_CONFIG);
			Status = copy_to_user(wrq->u.data.pointer, pStaConfig, wrq->u.data.length);
			os_free_mem(pStaConfig);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_802_11_STA_CONFIG(mem alloc failed)\n");
			Status = -EFAULT;
		}

		break;

	case OID_802_11_RTS_THRESHOLD:
		RtsThresh = wlan_operate_get_rts_len_thld(wdev);
		wrq->u.data.length = sizeof(RtsThresh);
		Status = copy_to_user(wrq->u.data.pointer, &RtsThresh, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_RTS_THRESHOLD(=%ld)\n", RtsThresh);
		break;

	case OID_802_11_FRAGMENTATION_THRESHOLD:
		FragThresh = wlan_operate_get_frag_thld(wdev);

		if (pAd->CommonCfg.bUseZeroToDisableFragment == TRUE)
			FragThresh = 0;

		wrq->u.data.length = sizeof(FragThresh);
		Status = copy_to_user(wrq->u.data.pointer, &FragThresh, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_FRAGMENTATION_THRESHOLD(=%ld)\n", FragThresh);
		break;

	case OID_802_11_POWER_MODE:
		PowerMode = pStaCfg->WindowsPowerMode;
		wrq->u.data.length = sizeof(PowerMode);
		Status = copy_to_user(wrq->u.data.pointer, &PowerMode, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_POWER_MODE(=%d)\n", PowerMode);
		break;

	case RT_OID_802_11_RADIO:
		RadioState = (BOOLEAN) pStaCfg->bSwRadio;
		wrq->u.data.length = sizeof(RadioState);
		Status = copy_to_user(wrq->u.data.pointer, &RadioState, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_QUERY_RADIO (=%d)\n", RadioState);
		break;

	case OID_802_11_INFRASTRUCTURE_MODE:
		if (pStaCfg->BssType == BSS_ADHOC)
			BssType = Ndis802_11IBSS;
		else if (pStaCfg->BssType == BSS_INFRA)
			BssType = Ndis802_11Infrastructure;
		else if (pStaCfg->BssType == BSS_MONITOR)
			BssType = Ndis802_11Monitor;
		else
			BssType = Ndis802_11AutoUnknown;

		wrq->u.data.length = sizeof(BssType);
		Status = copy_to_user(wrq->u.data.pointer, &BssType, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_INFRASTRUCTURE_MODE(=%d)\n", BssType);
		break;

	case RT_OID_802_11_PREAMBLE:
		PreamType = pAd->CommonCfg.TxPreamble;
		wrq->u.data.length = sizeof(PreamType);
		Status = copy_to_user(wrq->u.data.pointer, &PreamType, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_PREAMBLE(=%d)\n", PreamType);
		break;

	case OID_802_11_AUTHENTICATION_MODE:
		AuthMode = SecAuthModeNewToOld(pStaCfg->wdev.SecConfig.AKMMap);
		wrq->u.data.length = sizeof(AuthMode);
		Status = copy_to_user(wrq->u.data.pointer, &AuthMode, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_AUTHENTICATION_MODE(=%d)\n", AuthMode);
		break;

	case OID_802_11_WEP_STATUS:
		WepStatus = SecEncryModeNewToOld(pStaCfg->wdev.SecConfig.PairwiseCipher);
		wrq->u.data.length = sizeof(WepStatus);
		Status = copy_to_user(wrq->u.data.pointer, &WepStatus, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_WEP_STATUS(=%d)\n", WepStatus);
		break;

	case OID_802_11_TX_POWER_LEVEL:
		wrq->u.data.length = sizeof(ULONG);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.TxPower, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_TX_POWER_LEVEL %x\n", pAd->CommonCfg.TxPower);
		break;

	case RT_OID_802_11_TX_POWER_LEVEL_1:
		wrq->u.data.length = sizeof(ULONG);
		TxPowerPercentage = (ULONG)(pAd->CommonCfg.ucTxPowerPercentage[BAND0]);
		Status = copy_to_user(wrq->u.data.pointer, &TxPowerPercentage, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"RT_OID_802_11_TX_POWER_LEVEL_1 (BAND0) (=%d) STATUS[%d]\n",
				 pAd->CommonCfg.ucTxPowerPercentage[BAND0], Status);
#ifdef DBDC_MODE
		TxPowerPercentage = (ULONG)(pAd->CommonCfg.ucTxPowerPercentage[BAND1]);
		Status = copy_to_user(wrq->u.data.pointer, &TxPowerPercentage, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_TX_POWER_LEVEL_1 (BAND1) (=%d)\n",
				 pAd->CommonCfg.ucTxPowerPercentage[BAND1]);
#endif /* DBDC_MODE */
		break;

	case OID_802_11_NETWORK_TYPES_SUPPORTED:
		if ((pAd->RfIcType == RFIC_2850) ||
			(pAd->RfIcType == RFIC_2750) ||
			(pAd->RfIcType == RFIC_3052) ||
			(pAd->RfIcType == RFIC_3053) ||
			(pAd->RfIcType == RFIC_2853) ||
			(pAd->RfIcType == RFIC_3853) ||
			(pAd->RfIcType == RFIC_5592)) {
			NetworkTypeList[0] = 3;                 /* NumberOfItems = 3 */
			NetworkTypeList[1] = Ndis802_11DS;      /* NetworkType[1] = 11b */
			NetworkTypeList[2] = Ndis802_11OFDM24;  /* NetworkType[2] = 11g */
			NetworkTypeList[3] = Ndis802_11OFDM5;   /* NetworkType[3] = 11a */
			wrq->u.data.length = 16;
			Status = copy_to_user(wrq->u.data.pointer, &NetworkTypeList[0], wrq->u.data.length);
		} else {
			NetworkTypeList[0] = 2;                 /* NumberOfItems = 2 */
			NetworkTypeList[1] = Ndis802_11DS;      /* NetworkType[1] = 11b */
			NetworkTypeList[2] = Ndis802_11OFDM24;  /* NetworkType[2] = 11g */
			wrq->u.data.length = 12;
			Status = copy_to_user(wrq->u.data.pointer, &NetworkTypeList[0], wrq->u.data.length);
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_NETWORK_TYPES_SUPPORTED\n");
		break;

	case OID_802_11_NETWORK_TYPE_IN_USE:
		wrq->u.data.length = sizeof(ULONG);

		if (WMODE_EQUAL(wdev->PhyMode, WMODE_A))
			ulInfo = Ndis802_11OFDM5;
		else if (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_B)) ||
				 WMODE_EQUAL(wdev->PhyMode, WMODE_G))
			ulInfo = Ndis802_11OFDM24;
		else
			ulInfo = Ndis802_11DS;

		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		break;

	case RT_OID_802_11_QUERY_LAST_RX_RATE:
		ulInfo = (ULONG)pAd->LastRxRate;
		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_QUERY_LAST_RX_RATE (=%ld)\n", ulInfo);
		break;

	case RT_OID_802_11_QUERY_LAST_TX_RATE:
		ulInfo = (ULONG)pAd->LastTxRate;
		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_QUERY_LAST_TX_RATE (=%lx)\n", ulInfo);
		break;

	case RT_OID_802_11_QUERY_MAP_REAL_RX_RATE:
		RateValue = 0;
		HTPhyMode.word = (USHORT) pEntry->LastRxRate;
		getRate(HTPhyMode, &RateValue);
		wrq->u.data.length = sizeof(RateValue);
		Status = copy_to_user(wrq->u.data.pointer, &RateValue, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_QUERY_LAST_RX_RATE (=%ld)\n", RateValue);
		break;

	case RT_OID_802_11_QUERY_MAP_REAL_TX_RATE:
		RateValue = 0;
		HTPhyMode.word = (USHORT)pAd->LastTxRate;
		getRate(HTPhyMode, &RateValue);
		wrq->u.data.length = sizeof(RateValue);
		Status = copy_to_user(wrq->u.data.pointer, &RateValue, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_QUERY_LAST_TX_RATE (=%ld)\n", RateValue);
		break;

	case RT_OID_802_11_QUERY_TX_PHYMODE:
		ulInfo = (ULONG)pEntry->HTPhyMode.word;
		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_QUERY_TX_PHYMODE (=%lx)\n", ulInfo);
		break;

	case RT_OID_802_11_QUERY_EEPROM_VERSION:
		wrq->u.data.length = sizeof(ULONG);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->EepromVersion, wrq->u.data.length);
		break;

	case RT_OID_802_11_QUERY_FIRMWARE_VERSION:
		wrq->u.data.length = sizeof(ULONG);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->FirmwareVersion, wrq->u.data.length);
		break;

	case RT_OID_802_11_QUERY_NOISE_LEVEL: {
		UCHAR noise = RTMPMaxRssi(pAd, pStaCfg->RssiSample.AvgRssi[0],
								  pStaCfg->RssiSample.AvgRssi[1],
								  pStaCfg->RssiSample.AvgRssi[2]) -
					  RTMPMinSnr(pAd, pStaCfg->RssiSample.AvgSnr[0],
								 pStaCfg->RssiSample.AvgSnr[1]);
		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer, &noise, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_QUERY_NOISE_LEVEL (=%d)\n", noise);
	}
	break;

	case RT_OID_802_11_EXTRA_INFO:
		wrq->u.data.length = sizeof(ULONG);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->ExtraInfo, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_EXTRA_INFO (=%ld)\n", pAd->ExtraInfo);
		break;

	case RT_OID_WE_VERSION_COMPILED:
		wrq->u.data.length = sizeof(UINT);
		we_version_compiled = RtmpOsWirelessExtVerGet();
		Status = copy_to_user(wrq->u.data.pointer, &we_version_compiled, wrq->u.data.length);
		break;

	case RT_OID_802_11_QUERY_APSD_SETTING:
		apsd = (pStaCfg->wdev.UapsdInfo.bAPSDCapable | (pAd->CommonCfg.bAPSDAC_BE << 1) | (pAd->CommonCfg.bAPSDAC_BK << 2)
				| (pAd->CommonCfg.bAPSDAC_VI << 3)	| (pAd->CommonCfg.bAPSDAC_VO << 4)	| (pAd->CommonCfg.MaxSPLength << 5));
		wrq->u.data.length = sizeof(ULONG);
		Status = copy_to_user(wrq->u.data.pointer, &apsd, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "RT_OID_802_11_QUERY_APSD_SETTING (=0x%lx,APSDCap=%d,AC_BE=%d,AC_BK=%d,AC_VI=%d,AC_VO=%d,MAXSPLen=%d)\n",
				  apsd,
				  pStaCfg->wdev.UapsdInfo.bAPSDCapable,
				  pAd->CommonCfg.bAPSDAC_BE,
				  pAd->CommonCfg.bAPSDAC_BK,
				  pAd->CommonCfg.bAPSDAC_VI,
				  pAd->CommonCfg.bAPSDAC_VO,
				  pAd->CommonCfg.MaxSPLength);
		break;

	case RT_OID_802_11_QUERY_APSD_PSM:
		wrq->u.data.length = sizeof(ULONG);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.bAPSDForcePowerSave, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_QUERY_APSD_PSM (=%d)\n",
				 pAd->CommonCfg.bAPSDForcePowerSave);
		break;

	case RT_OID_802_11_QUERY_WMM:
		wrq->u.data.length = sizeof(BOOLEAN);
		Status = copy_to_user(wrq->u.data.pointer, &wdev->bWmmCapable, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_QUERY_WMM (=%d)\n",	wdev->bWmmCapable);
		break;
#ifdef WPA_SUPPLICANT_SUPPORT

	case RT_OID_NEW_DRIVER: {
		UCHAR enabled = 1;

		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer, &enabled, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_NEW_DRIVER (=%d)\n", enabled);
	}
	break;

	case RT_OID_WPA_SUPPLICANT_SUPPORT:
		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer, &pStaCfg->wpa_supplicant_info.WpaSupplicantUP, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WPA_SUPPLICANT_SUPPORT (=%d)\n",
				 pStaCfg->wpa_supplicant_info.WpaSupplicantUP);
		break;
#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef WSC_STA_SUPPORT

	case RT_OID_WSC_QUERY_STATUS:
		wrq->u.data.length = sizeof(INT);

		if (copy_to_user(wrq->u.data.pointer, &wdev->WscControl.WscStatus, wrq->u.data.length))
			Status = -EFAULT;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_QUERY_STATUS (=%d)\n",
				 wdev->WscControl.WscStatus);
		break;

	case RT_OID_WSC_PIN_CODE:
		wrq->u.data.length = sizeof(UINT);
		WscPinCode = wdev->WscControl.WscEnrolleePinCode;

		if (copy_to_user(wrq->u.data.pointer, &WscPinCode, wrq->u.data.length))
			Status = -EFAULT;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_PIN_CODE (=%d)\n", WscPinCode);
		break;

	case RT_OID_WSC_QUERY_DEFAULT_PROFILE:
		wrq->u.data.length = sizeof(WSC_PROFILE);
		os_alloc_mem(pAd, (UCHAR **)&pProfile, sizeof(WSC_PROFILE));

		if (pProfile == NULL) {
			Status = -EFAULT;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_WSC_QUERY_DEFAULT_PROFILE fail!\n");
			break;
		}

		RTMPZeroMemory(pProfile, sizeof(WSC_PROFILE));
		WscCreateProfileFromCfg(pAd, STA_MODE, &wdev->WscControl, pProfile);

		if (copy_to_user(wrq->u.data.pointer, pProfile, wrq->u.data.length))
			Status = -EFAULT;
		else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC Profile:\n");
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SSID = %s\n", pProfile->Profile[0].SSID.Ssid);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AuthType = %s\n",
					 WscGetAuthTypeStr(pProfile->Profile[0].AuthType));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "EncrpType = %s\n",
					 WscGetEncryTypeStr(pProfile->Profile[0].EncrType));

			if (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_WEP) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WEP Key = %s\n", pProfile->Profile[0].Key);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DefaultKey ID = %d\n\n", pProfile->Profile[0].KeyIndex);
			} else if ((pProfile->Profile[0].EncrType == WSC_ENCRTYPE_TKIP) ||
					   (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_AES)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PassPhrase Key = %s\n\n", pProfile->Profile[0].Key);
				pProfile->Profile[0].KeyIndex = 1;
			}
		}

		os_free_mem(pProfile);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_QUERY_DEFAULT_PROFILE\n");
		break;

	case RT_OID_802_11_WSC_QUERY_PROFILE:
		wrq->u.data.length = sizeof(WSC_PROFILE);
		os_alloc_mem(pAd, (UCHAR **)&pProfile, sizeof(WSC_PROFILE));

		if (pProfile == NULL) {
			Status = -EFAULT;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_802_11_WSC_QUERY_PROFILE fail!\n");
			break;
		}

		RTMPZeroMemory(pProfile, sizeof(WSC_PROFILE));
		NdisMoveMemory(pProfile, &wdev->WscControl.WscProfile, sizeof(WSC_PROFILE));

		if ((pProfile->Profile[0].AuthType == WSC_AUTHTYPE_OPEN) && (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_NONE)) {
			pProfile->Profile[0].KeyLength = 0;
			NdisZeroMemory(pProfile->Profile[0].Key, 64);
		}


		if (copy_to_user(wrq->u.data.pointer, pProfile, wrq->u.data.length))
			Status = -EFAULT;
		else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC Profile:\n");
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SSID = %s\n", pProfile->Profile[0].SSID.Ssid);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AuthType = %s\n",
					 WscGetAuthTypeStr(pProfile->Profile[0].AuthType));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "EncrpType = %s\n",
					 WscGetEncryTypeStr(pProfile->Profile[0].EncrType));

			if (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_WEP) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WEP Key = %s\n", pProfile->Profile[0].Key);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DefaultKey ID = %d\n", pProfile->Profile[0].KeyIndex);
			} else if ((pProfile->Profile[0].EncrType == WSC_ENCRTYPE_TKIP) ||
					   (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_AES)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PassPhrase Key = %s\n", pProfile->Profile[0].Key);
				pProfile->Profile[0].KeyIndex = 1;
			}
		}

		os_free_mem(pProfile);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_WSC_QUERY_PROFILE\n");
		break;

	case RT_OID_WSC_UUID:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_QUERY_UUID\n");
		wrq->u.data.length = UUID_LEN_STR;

		if (copy_to_user(wrq->u.data.pointer, &wdev->WscControl.Wsc_Uuid_Str[0], UUID_LEN_STR))
			Status = -EFAULT;

		break;

	case RT_OID_WSC_MAC_ADDRESS:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_MAC_ADDRESS\n");
		wrq->u.data.length = MAC_ADDR_LEN;

		if (copy_to_user(wrq->u.data.pointer, wdev->if_addr, wrq->u.data.length))
			Status = -EFAULT;

		break;
#endif /* WSC_STA_SUPPORT */

	case RT_OID_DRIVER_DEVICE_NAME:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_DRIVER_DEVICE_NAME\n");
		wrq->u.data.length = 16;

		if (copy_to_user(wrq->u.data.pointer, pStaCfg->dev_name, wrq->u.data.length))
			Status = -EFAULT;

		break;

	case RT_OID_802_11_QUERY_HT_PHYMODE:
		os_alloc_mem(pAd, (UCHAR **)&pHTPhyMode, sizeof(OID_SET_HT_PHYMODE));

		if (pHTPhyMode) {
			pHTPhyMode->PhyMode = wdev->PhyMode;
			pHTPhyMode->HtMode = (UCHAR)pEntry->HTPhyMode.field.MODE;
			pHTPhyMode->BW = (UCHAR)pEntry->HTPhyMode.field.BW;
			pHTPhyMode->MCS = (UCHAR)pEntry->HTPhyMode.field.MCS;
			pHTPhyMode->SHORTGI = (UCHAR)pEntry->HTPhyMode.field.ShortGI;
			pHTPhyMode->STBC = (UCHAR)pEntry->HTPhyMode.field.STBC;
			pHTPhyMode->ExtOffset = wlan_operate_get_ext_cha(&pStaCfg->wdev);
			wrq->u.data.length = sizeof(OID_SET_HT_PHYMODE);

			if (copy_to_user(wrq->u.data.pointer, pHTPhyMode, wrq->u.data.length))
				Status = -EFAULT;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "RT_OID_802_11_QUERY_HT_PHYMODE (PhyMode = %d, MCS =%d, BW = %d, STBC = %d, ExtOffset=%d)\n",
					  pHTPhyMode->HtMode, pHTPhyMode->MCS, pHTPhyMode->BW, pHTPhyMode->STBC, pHTPhyMode->ExtOffset);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "word = %x\n", pEntry->HTPhyMode.word);
			os_free_mem(pHTPhyMode);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_802_11_STA_CONFIG(mem alloc failed)\n");
			Status = -EFAULT;
		}

		break;

	case RT_OID_802_11_COUNTRY_REGION:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_COUNTRY_REGION\n");
		wrq->u.data.length = sizeof(ulInfo);
		ulInfo = pAd->CommonCfg.CountryRegionForABand;
		ulInfo = (ulInfo << 8) | (pAd->CommonCfg.CountryRegion);

		if (copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length))
			Status = -EFAULT;

		break;

	case RT_OID_802_11_QUERY_DAT_HT_PHYMODE:
		os_alloc_mem(pAd, (UCHAR **)&pHTPhyMode, sizeof(OID_SET_HT_PHYMODE));

		if (pHTPhyMode) {
			pHTPhyMode->PhyMode = wmode_2_cfgmode(wdev->PhyMode);
			pHTPhyMode->HtMode = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.HTMODE;
			pHTPhyMode->BW = (UCHAR)wlan_operate_get_ht_bw(wdev);
			pHTPhyMode->MCS = (UCHAR)pStaCfg->wdev.DesiredTransmitSetting.field.MCS;
			pHTPhyMode->SHORTGI = (UCHAR)wlan_config_get_ht_gi(wdev);
			pHTPhyMode->STBC = wlan_config_get_ht_stbc(wdev);
			pHTPhyMode->ExtOffset = wlan_operate_get_ext_cha(wdev);
			wrq->u.data.length = sizeof(OID_SET_HT_PHYMODE);

			if (copy_to_user(wrq->u.data.pointer, pHTPhyMode, wrq->u.data.length))
				Status = -EFAULT;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "RT_OID_802_11_QUERY_HT_PHYMODE (PhyMode = %d, MCS =%d, BW = %d, STBC = %d, ExtOffset=%d)\n",
					  pHTPhyMode->HtMode, pHTPhyMode->MCS, pHTPhyMode->BW, pHTPhyMode->STBC, pHTPhyMode->ExtOffset);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ".word = %x\n", pEntry->HTPhyMode.word);
			os_free_mem(pHTPhyMode);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_STA_CONFIG(mem alloc failed)\n");
			Status = -EFAULT;
		}

		break;

#ifdef SNMP_SUPPORT

	case RT_OID_802_11_MAC_ADDRESS:
		wrq->u.data.length = MAC_ADDR_LEN;
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CurrentAddress, wrq->u.data.length);
		break;

	case RT_OID_802_11_MANUFACTUREROUI:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_MANUFACTUREROUI\n");
		wrq->u.data.length = ManufacturerOUI_LEN;
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CurrentAddress, wrq->u.data.length);
		break;

	case RT_OID_802_11_MANUFACTURERNAME:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_MANUFACTURERNAME\n");
		wrq->u.data.length = strlen(ManufacturerNAME);
		Status = copy_to_user(wrq->u.data.pointer, ManufacturerNAME, wrq->u.data.length);
		break;

	case RT_OID_802_11_RESOURCETYPEIDNAME:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_RESOURCETYPEIDNAME\n");
		wrq->u.data.length = strlen(ResourceTypeIdName);
		Status = copy_to_user(wrq->u.data.pointer, ResourceTypeIdName, wrq->u.data.length);
		break;

	case RT_OID_802_11_PRIVACYOPTIONIMPLEMENTED:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_PRIVACYOPTIONIMPLEMENTED\n");
		ulInfo = 1; /* 1 is support wep else 2 is not support. */
		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		break;

	case RT_OID_802_11_POWERMANAGEMENTMODE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_POWERMANAGEMENTMODE\n");

		if (pStaCfg->PwrMgmt.Psm == PSMP_ACTION)
			ulInfo = 1; /* 1 is power active else 2 is power save. */
		else
			ulInfo = 2;

		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		break;

	case OID_802_11_WEPDEFAULTKEYVALUE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_WEPDEFAULTKEYVALUE\n");
		pKeyIdxValue = wrq->u.data.pointer;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "KeyIdxValue.KeyIdx = %d,\n", pKeyIdxValue->KeyIdx);
		valueLen = pAd->SharedKey[BSS0][pStaCfg->DefaultKeyId].KeyLen;
		NdisMoveMemory(pKeyIdxValue->Value,
					   &pAd->SharedKey[BSS0][pStaCfg->DefaultKeyId].Key,
					   valueLen);
		pKeyIdxValue->Value[valueLen] = '\0';
		wrq->u.data.length = sizeof(DefaultKeyIdxValue);
		Status = copy_to_user(wrq->u.data.pointer, pKeyIdxValue, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "DefaultKeyId = %d, total len = %d, str len=%d, KeyValue= %02x %02x %02x %02x\n",
				  pStaCfg->DefaultKeyId,
				  wrq->u.data.length,
				  pAd->SharedKey[BSS0][pStaCfg->DefaultKeyId].KeyLen,
				  pAd->SharedKey[BSS0][0].Key[0],
				  pAd->SharedKey[BSS0][1].Key[0],
				  pAd->SharedKey[BSS0][2].Key[0],
				  pAd->SharedKey[BSS0][3].Key[0]);
		break;

	case OID_802_11_WEPDEFAULTKEYID:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_WEPDEFAULTKEYID\n");
		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer, &pStaCfg->DefaultKeyId, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DefaultKeyId =%d\n", pStaCfg->DefaultKeyId);
		break;

	case RT_OID_802_11_WEPKEYMAPPINGLENGTH:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_WEPKEYMAPPINGLENGTH\n");
		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer,
							  &pAd->SharedKey[BSS0][pStaCfg->DefaultKeyId].KeyLen,
							  wrq->u.data.length);
		break;

	case OID_802_11_SHORTRETRYLIMIT:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_SHORTRETRYLIMIT\n");
		wrq->u.data.length = sizeof(ULONG);
		ShortRetryLimit = AsicGetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_SHORT);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ShortRetryLimit =%ld\n", ShortRetryLimit);
		Status = copy_to_user(wrq->u.data.pointer, &ShortRetryLimit, wrq->u.data.length);
		break;

	case OID_802_11_LONGRETRYLIMIT:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_LONGRETRYLIMIT\n");
		wrq->u.data.length = sizeof(ULONG);
		LongRetryLimit = AsicGetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_LONG);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "LongRetryLimit =%ld\n", LongRetryLimit);
		Status = copy_to_user(wrq->u.data.pointer, &LongRetryLimit, wrq->u.data.length);
		break;

	case RT_OID_802_11_PRODUCTID:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_PRODUCTID\n");
#ifdef RTMP_MAC_PCI
		{
			USHORT  device_id;

			if (((POS_COOKIE)pAd->OS_Cookie)->pci_dev != NULL)
				pci_read_config_word(((POS_COOKIE)pAd->OS_Cookie)->pci_dev, PCI_DEVICE_ID, &device_id);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, " pci_dev = NULL\n");

			snprintf((RTMP_STRING *)tmp, sizeof(tmp), "%04x %04x\n", NIC_PCI_VENDOR_ID, device_id);
		}
#endif /* RTMP_MAC_PCI */
		wrq->u.data.length = strlen((RTMP_STRING *)tmp);
		Status = copy_to_user(wrq->u.data.pointer, tmp, wrq->u.data.length);
		break;

	case RT_OID_802_11_MANUFACTUREID:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_802_11_MANUFACTUREID\n");
		wrq->u.data.length = strlen(ManufacturerNAME);
		Status = copy_to_user(wrq->u.data.pointer, ManufacturerNAME, wrq->u.data.length);
		break;

	case OID_802_11_CURRENTCHANNEL:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_CURRENTCHANNEL\n");
		wrq->u.data.length = sizeof(UCHAR);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "sizeof UCHAR=%d, channel=%d\n", sizeof(UCHAR), wdev->channel);
		Status = copy_to_user(wrq->u.data.pointer, &wdev->channel, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Status=%d\n", Status);
		break;
#endif /*SNMP_SUPPORT */

	case OID_802_11_BUILD_CHANNEL_EX: {
		UCHAR value;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_BUILD_CHANNEL_EX\n");
		wrq->u.data.length = sizeof(UCHAR);
#ifdef EXT_BUILD_CHANNEL_LIST
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Support EXT_BUILD_CHANNEL_LIST.\n");
		value = 1;
#else
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Doesn't support EXT_BUILD_CHANNEL_LIST.\n");
		value = 0;
#endif /* EXT_BUILD_CHANNEL_LIST */
		Status = copy_to_user(wrq->u.data.pointer, &value, 1);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Status=%d\n", Status);
	}
	break;

	case OID_802_11_GET_CH_LIST: {
		PRT_CHANNEL_LIST_INFO pChListBuf;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_GET_CH_LIST\n");

		if (pAd->ChannelListNum == 0) {
			wrq->u.data.length = 0;
			break;
		}

		os_alloc_mem(pAd, (UCHAR **)&pChListBuf, sizeof(RT_CHANNEL_LIST_INFO));

		if (pChListBuf == NULL) {
			wrq->u.data.length = 0;
			break;
		}

		pChListBuf->ChannelListNum = pChCtrl->ChListNum;

		for (i = 0; i < pChListBuf->ChannelListNum; i++)
			pChListBuf->ChannelList[i] = pChCtrl->ChList[i].Channel;

		wrq->u.data.length = sizeof(RT_CHANNEL_LIST_INFO);
		Status = copy_to_user(wrq->u.data.pointer, pChListBuf, sizeof(RT_CHANNEL_LIST_INFO));
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Status=%d\n", Status);

		if (pChListBuf)
			os_free_mem(pChListBuf);
	}
	break;

	case OID_802_11_GET_COUNTRY_CODE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_GET_COUNTRY_CODE\n");
		wrq->u.data.length = 2;
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.CountryCode, 2);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Status=%d\n", Status);
		break;
#ifdef EXT_BUILD_CHANNEL_LIST

	case OID_802_11_GET_CHANNEL_GEOGRAPHY:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_GET_CHANNEL_GEOGRAPHY\n");
		wrq->u.data.length = 1;
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.Geography, 1);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Status=%d\n", Status);
		break;
#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef P2P_SUPPORT

	case OID_802_11_P2P_MODE:
		wrq->u.data.length = sizeof(char);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->P2pCfg.Rule, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_P2P_MODE (Len=%d, Rule=%s)\n", sizeof(char),
				 pAd->P2pCfg.Rule);
		break;

	case OID_802_11_P2P_DEVICE_NAME:
		wrq->u.data.length = pAd->P2pCfg.DeviceNameLen;
		Status = copy_to_user(wrq->u.data.pointer, pAd->P2pCfg.DeviceName, pAd->P2pCfg.DeviceNameLen);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_P2P_DEVICE_NAME (Len=%d, DeviceName=%s)\n",
				 pAd->P2pCfg.DeviceNameLen, pAd->P2pCfg.DeviceName);
		break;

	case OID_802_11_P2P_LISTEN_CHANNEL:
		wrq->u.data.length = sizeof(char);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->P2pCfg.ListenChannel, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_P2P_LISTEN_CHANNEL (Len=%d, Listen_Ch=%d)\n",
				 sizeof(char), pAd->P2pCfg.ListenChannel);
		break;

	case OID_802_11_P2P_OPERATION_CHANNEL:
		wrq->u.data.length = sizeof(char);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->P2pCfg.GroupChannel, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_P2P_OPERATION_CHANNEL (Len=%d, Op_Ch=%d)\n",
				 sizeof(char), pAd->P2pCfg.GroupOpChannel);
		break;

	case OID_802_11_P2P_DEV_ADDR:
		wrq->u.data.length = 6;
		Status = copy_to_user(wrq->u.data.pointer, pAd->P2pCfg.Bssid, wrq->u.data.length);
		/*MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("OID_802_11_P2P_MAC_ADDR (Len=%d, Rule=%s)\n", sizeof(char),pAd->P2pCfg.GroupOpChannel)); */
		break;

	case OID_802_11_P2P_CTRL_STATUS:
		wrq->u.data.length = 24;
		pP2PCtrl = &pAd->P2pCfg;
		NdisZeroMemory(tmpP2P, 24);
		sprintf(tmpP2P, "%s", decodeCtrlState(pP2PCtrl->CtrlCurrentState));
		Status = copy_to_user(wrq->u.data.pointer, tmpP2P, 24);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_P2P_MODE (Len=%d, DeviceName=%s)\n",
				 pAd->P2pCfg.DeviceNameLen, pAd->P2pCfg.DeviceName);
		break;

	case OID_802_11_P2P_DISC_STATUS:
		wrq->u.data.length = 24;
		pP2PCtrl = &pAd->P2pCfg;
		NdisZeroMemory(tmpP2P, 24);
		sprintf(tmpP2P, "%s", decodeDiscoveryState(pP2PCtrl->DiscCurrentState));
		Status = copy_to_user(wrq->u.data.pointer, tmpP2P, 24);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_P2P_MODE (Len=%d, DeviceName=%s)\n",
				 pAd->P2pCfg.DeviceNameLen, pAd->P2pCfg.DeviceName);
		break;

	case OID_802_11_P2P_GOFORM_STATUS:
		wrq->u.data.length = 24;
		pP2PCtrl = &pAd->P2pCfg;
		NdisZeroMemory(tmpP2P, 24);
		sprintf(tmpP2P, "%s", decodeGroupFormationState(pP2PCtrl->GoFormCurrentState));
		Status = copy_to_user(wrq->u.data.pointer, tmpP2P, 24);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_P2P_MODE (Len=%d, DeviceName=%s)\n",
				 pAd->P2pCfg.DeviceNameLen, pAd->P2pCfg.DeviceName);
		break;

	case OID_802_11_P2P_SCAN_LIST:
		os_alloc_mem(NULL, (UCHAR **)&pUI_table, sizeof(RT_P2P_UI_TABLE));

		if (pUI_table) {
			pP2pTable = &pAd->P2pTable;
			NdisZeroMemory(pUI_table, sizeof(RT_P2P_UI_TABLE));
			pUI_table->ClientNumber = pAd->P2pTable.ClientNumber;

			for (i = 0; i < pAd->P2pTable.ClientNumber; i++) {
				pPAdCli = &pP2pTable->Client[i];
				pUICli = &pUI_table->Client[i];
				NdisMoveMemory(pUICli, pPAdCli, sizeof(RT_P2P_CLIENT_ENTRY));
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_P2P_SCAN_LIST\n");
			Status = copy_to_user(wrq->u.data.pointer, pUI_table, sizeof(RT_P2P_UI_TABLE));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_P2P_SCAN_LIST (Len=%d, Rule=%s)\n",
					 sizeof(char), pAd->P2pCfg.GroupOpChannel);
			os_free_mem(pUI_table);
		} else
			Status = -ENOMEM;

		break;

	case OID_P2P_WSC_PIN_CODE:
		wrq->u.data.length = sizeof(UINT);
		WscPinCode = pAd->StaCfg[0].WscControl.WscEnrolleePinCode;

		if (copy_to_user(wrq->u.data.pointer, &WscPinCode, wrq->u.data.length))
			Status = -EFAULT;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_PIN_CODE (=%d)\n", WscPinCode);
		break;
#endif /* P2P_SUPPORT */
#ifdef XLINK_SUPPORT

	case OID_802_11_SET_PSPXLINK_MODE:
		wrq->u.data.length = sizeof(BOOLEAN);
		Status = copy_to_user(wrq->u.data.pointer, &pStaCfg->PSPXlink, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_SET_PSPXLINK_MODE(=%d)\n", pStaCfg->PSPXlink);
		break;
#endif /* XLINK_SUPPORT */
#ifdef DOT11Z_TDLS_SUPPORT
#ifdef WFD_SUPPORT

	/*
	 * Query WFD TDLS connection status
	 * Value:
	 *	0: TDLS connecting or no link
	 *	4: TDLS connected
	 *	6: Peer PC (Preferred Connectivity) bit of WFD device is P2P
	 *	7: TDLS weak security
	 */
	case RT_OID_802_11_QUERY_WFD_TDLS_CONNECT_STATUS: {
		INT i;
		UCHAR bFound = CONNECTING_OR_NO_LINK;

		if (pStaCfg->WfdCfg.PeerPC == WFD_PC_P2P) {
			bFound = WFD_PEER_PC_P2P;
			pStaCfg->WfdCfg.PeerPC = WFD_PC_TDLS;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "RT_OID_802_11_QUERY_WFD_TDLS_CONNECT_STATUS::Peer WFD PC is P2P!\n");
		} else if (pStaCfg->WfdCfg.TdlsSecurity == WFD_TDLS_WEAK_SECURITY) {
			bFound = WFD_PEER_TDLS_WEAK_SECURITY;
			pStaCfg->WfdCfg.TdlsSecurity = WFD_TDLS_STRONG_SECURITY;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "RT_OID_802_11_QUERY_WFD_TDLS_CONNECT_STATUS::TDLS weak security!\n");
		} else {
			for (i = 0; i < MAX_NUM_OF_TDLS_ENTRY; i++) {
				if ((pStaCfg->TdlsInfo.TDLSEntry[i].Valid) && (pStaCfg->TdlsInfo.TDLSEntry[i].Status == TDLS_MODE_CONNECTED))
					break;
			}

			if (i == MAX_NUM_OF_TDLS_ENTRY)
				bFound = CONNECTING_OR_NO_LINK;
			else
				bFound = TDLS_LINKED;

			if (bFound == TDLS_LINKED)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "RT_OID_802_11_QUERY_WFD_TDLS_CONNECT_STATUS:: TDLS Connected!\n");
		}

		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer, &bFound, wrq->u.data.length);
	}
	break;

	/*
	 * Query peer WFD TDLS IP address
	 *	Field		  | Size (octets)  | Value	    | Description
	 *	---------------------------------------------------------------------------------------------
	 *	Version	  | 1			 | 1		    | Version 1: IPv4 address field follows
	 *	IPv4 address  | 4			 | (IP address) | This field is the IPv4 host address of the STA
	 */
	case RT_OID_802_11_QUERY_WFD_TDLS_PEER_IP_ADDR: {
		INT i;
		UCHAR peer_ip_addr[5] = {0};

		for (i = 0; i < MAX_NUM_OF_TDLS_ENTRY; i++) {
			if ((pStaCfg->TdlsInfo.TDLSEntry[i].Valid) && (pStaCfg->TdlsInfo.TDLSEntry[i].Status == TDLS_MODE_CONNECTED)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Peer IP Addr = %d.%d.%d.%d\n",
						 pStaCfg->TdlsInfo.TDLSEntry[i].WfdEntryInfo.wfd_serv_disc_query_info.wfd_local_ip_ie[1],
						 pStaCfg->TdlsInfo.TDLSEntry[i].WfdEntryInfo.wfd_serv_disc_query_info.wfd_local_ip_ie[2],
						 pStaCfg->TdlsInfo.TDLSEntry[i].WfdEntryInfo.wfd_serv_disc_query_info.wfd_local_ip_ie[3],
						 pStaCfg->TdlsInfo.TDLSEntry[i].WfdEntryInfo.wfd_serv_disc_query_info.wfd_local_ip_ie[4]);
				RTMPMoveMemory(&peer_ip_addr, &pStaCfg->TdlsInfo.TDLSEntry[i].WfdEntryInfo.wfd_serv_disc_query_info.wfd_local_ip_ie,
							   sizeof(peer_ip_addr));
				break;
			}
		}

		wrq->u.data.length = sizeof(peer_ip_addr);
		Status = copy_to_user(wrq->u.data.pointer, &peer_ip_addr, wrq->u.data.length);
	}
	break;
#endif /* WFD_SUPPORT */
#endif /* DOT11Z_TDLS_SUPPORT */
#ifdef RTMP_RBUS_SUPPORT

	case OID_802_11_QUERY_WirelessMode:
		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer, &wdev->PhyMode, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_QUERY_WirelessMode(=%d)\n", wdev->PhyMode);
		break;
#endif /* RTMP_RBUS_SUPPORT */
#ifdef DOT11R_FT_SUPPORT

	case OID_802_11R_SUPPORT:
		wrq->u.data.length = sizeof(BOOLEAN);
		Status = copy_to_user(wrq->u.data.pointer, &pStaCfg->Dot11RCommInfo.bFtSupport, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11R_SUPPORT(=%d)\n",
				 pStaCfg->Dot11RCommInfo.bFtSupport);
		break;

	case  OID_802_11R_MDID:
		wrq->u.data.length = 2;
		memset(wrq->u.data.pointer, 0x00, 2);
		Status = copy_to_user(wrq->u.data.pointer, pStaCfg->Dot11RCommInfo.MdIeInfo.MdId, 2);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11R_MDID(=%x%x)\n",
				 pStaCfg->Dot11RCommInfo.MdIeInfo.MdId[0],
				 pStaCfg->Dot11RCommInfo.MdIeInfo.MdId[1]);
		break;
#endif /* DOT11R_FT_SUPPORT */
#ifdef IWSC_SUPPORT

	case RT_OID_IWSC_SELF_IPV4:
		if (wrq->u.data.length != sizeof(UINT32)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_IWSC_SELF_IPV4(=%d)\n",
					 wrq->u.data.length);
			Status  = -EINVAL;
		} else {
			Status = copy_to_user(wrq->u.data.pointer, &pStaCfg->IWscInfo.SelfIpv4Addr, sizeof(UINT32));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_IWSC_SELF_IPV4(=0x%08x), Status = %d\n",
					 pStaCfg->IWscInfo.SelfIpv4Addr, Status);
		}

		break;

	case RT_OID_IWSC_REGISTRAR_IPV4:
		if (wrq->u.data.length != sizeof(UINT32)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_IWSC_REGISTRAR_IPV4(=%d)\n",
					 wrq->u.data.length);
			Status  = -EINVAL;
		} else {
			Status = copy_to_user(wrq->u.data.pointer, &pStaCfg->IWscInfo.RegIpv4Addr, sizeof(UINT32));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_IWSC_REGISTRAR_IPV4(=0x%08x), Status = %d\n",
					 pStaCfg->IWscInfo.RegIpv4Addr, Status);
		}

		break;

	case RT_OID_IWSC_SMPBC_ENROLLEE_COUNT:
		if (wrq->u.data.length != sizeof(UINT8)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_IWSC_SMPBC_ENROLLEE_COUNT(=%d)\n",
					 wrq->u.data.length));
			Status  = -EINVAL;
		} else {
			Status = copy_to_user(wrq->u.data.pointer, &pStaCfg->IWscInfo.SmpbcEnrolleeCount, sizeof(UINT8));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "RT_OID_IWSC_SMPBC_ENROLLEE_COUNT(=0x%08x), Status = %d\n",
					 pStaCfg->IWscInfo.SmpbcEnrolleeCount, Status);
		}

		break;
#endif /* IWSC_SUPPORT */

	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Unknown IOCTL's subcmd = 0x%08x\n", cmd);
		Status = -EOPNOTSUPP;
		break;
	}

	return Status;
}


#ifdef DBG
/*
 *    ==========================================================================
 *    Description:
 *	Read / Write E2PROM
 *    Arguments:
 *	pAd                    Pointer to our adapter
 *	wrq                         Pointer to the ioctl argument
 *
 *    Return Value:
 *	None
 *
 *    Note:
 *	Usage:
 *	       1.) iwpriv ra0 e2p 0	==> read E2PROM where Addr=0x0
 *	       2.) iwpriv ra0 e2p 0=1234    ==> write E2PROM where Addr=0x0, value=1234
 *    ==========================================================================
 */
VOID RTMPIoctlE2PROM(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq)
{
	RTMP_STRING *this_char;
	RTMP_STRING *value;
	INT					j = 0, k = 0;
	/*	RTMP_STRING msg[1024]; */
	RTMP_STRING *msg = NULL;
	/*	RTMP_STRING arg[255]; */
	RTMP_STRING *arg = NULL;
	USHORT				eepAddr = 0;
	UCHAR				temp[16];
	RTMP_STRING temp2[16];
	USHORT				eepValue = 0, string_size = 1024;
	int					Status, ret;
	BOOLEAN				bIsPrintAllE2P = FALSE;
	BOOLEAN				bIsRange = FALSE;
	UINT32 msg_left;
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&msg, sizeof(RTMP_STRING) * string_size);

	if (msg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory fail!!!\n");
		goto LabelOK;
	}

	os_alloc_mem(NULL, (UCHAR **)&arg, sizeof(RTMP_STRING) * 255);

	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory fail!!!\n");
		goto LabelOK;
	}

	memset(msg, 0x00, string_size);
	memset(arg, 0x00, 255);

	if (wrq->u.data.length > 1) { /*No parameters. */
		Status = copy_from_user(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
		msg_left = string_size - strlen(msg);
		ret = snprintf(msg, msg_left, "\n");
		if (os_snprintf_error(msg_left, ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RTMPIoctlE2PROM snprintf error!\n");
			goto LabelOK;
		}
		arg[254] = 0x00;
		/*Parsing Read or Write */
		this_char = arg;

		if (!*this_char)
			goto next;

		value = rtstrchr(this_char, '=');

		if (value != NULL)
			* value++ = 0;
		else {
			value = rtstrchr(this_char, '-');

			if (value != NULL) {
				*value++ = 0;
				bIsRange = TRUE;
				goto range;
			}
		}

		if (!value || !*value) {
			/*Read */

			/* Sanity check */
			if (strlen(this_char) > 4)
				goto next;

			for (j = 0; j < strlen(this_char); j++) {
				if (this_char[j] > 'f' || this_char[j] < '0')
					goto LabelOK;
			}

			/* E2PROM addr */
			for (k = strlen(this_char), j = strlen(this_char) - 1; j >= 0; j--)
				this_char[4 - k + j] = this_char[j];

			while (k < 4)
				this_char[3 - k++] = '0';

			this_char[4] = '\0';

			if (strlen(this_char) == 4) {
				AtoH(this_char, temp, 2);
				eepAddr = *temp * 256 + temp[1];

				if (eepAddr < 0xFFFF) {
					RT28xx_EEPROM_READ16(pAd, eepAddr, eepValue);
					msg_left = string_size - strlen(msg);
					ret = snprintf(msg + strlen(msg), msg_left, "[0x%04X]:0x%04X  ", eepAddr, eepValue);
					if (os_snprintf_error(msg_left, ret)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RTMPIoctlE2PROM snprintf error!\n");
						goto LabelOK;
					}
				} else {
					/*Invalid parametes, so default printk all bbp */
					bIsPrintAllE2P = TRUE;
					goto next;
				}
			}
		} else {
			/*Write */
			memcpy(&temp2, value, strlen(value));
			temp2[strlen(value)] = '\0';

			/* Sanity check */
			if ((strlen(this_char) > 4) || strlen(temp2) > 8)
				goto next;

			for (j = 0; j < strlen(this_char); j++) {
				if (this_char[j] > 'f' || this_char[j] < '0')
					goto LabelOK;
			}

			for (j = 0; j < strlen(temp2); j++) {
				if (temp2[j] > 'f' || temp2[j] < '0')
					goto LabelOK;
			}

			/*MAC Addr */
			for (k = strlen(this_char), j = strlen(this_char) - 1; j >= 0; j--)
				this_char[4 - k + j] = this_char[j];

			while (k < 4)
				this_char[3 - k++] = '0';

			this_char[4] = '\0';

			/*MAC value */
			for (k = strlen(temp2), j = strlen(temp2) - 1; j >= 0 && (4 - k + j >= 0); j--)
				temp2[4 - k + j] = temp2[j];

			while (k < 4)
				temp2[3 - k++] = '0';

			temp2[4] = '\0';
			AtoH(this_char, temp, 2);
			eepAddr = *temp * 256 + temp[1];
			AtoH(temp2, temp, 2);
			eepValue = *temp * 256 + temp[1];
			RT28xx_EEPROM_WRITE16(pAd, eepAddr, eepValue);
			msg_left = string_size - strlen(msg);
			ret = snprintf(msg + strlen(msg), msg_left, "[0x%02X]:%02X	", eepAddr, eepValue);
			if (os_snprintf_error(msg_left, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RTMPIoctlE2PROM snprintf error!\n");
				goto LabelOK;
			}
		}
	} else
		bIsPrintAllE2P = TRUE;

range:

	if (bIsRange) {
		USHORT *pMacContent, *pMacIndex, e2pAddrStart = 0, e2pAddrEnd = 0;
		UINT32 ContentLen;
		/* DBGPRINT(RT_DEBUG_ERROR, ("[Jason]Range: this_char=%s, strlen(value)=%d, value=%s\n", this_char, (INT32)strlen(value), value)); */
		msg_left = string_size - strlen(msg);
		ret = snprintf(msg + strlen(msg), msg_left, "printing range e2p value\n");
		if (os_snprintf_error(msg_left, ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RTMPIoctlE2PROM snprintf error!\n");
			goto LabelOK;
		}
		if (strlen(value) <= sizeof(temp2))
			memcpy(&temp2, value, strlen(value));
		else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				"The length of value is too long\n");
			memcpy(&temp2, value, sizeof(temp2));
		}

		temp2[strlen(value)] = '\0';

		/* Sanity check */
		if ((strlen(this_char) > 4) || strlen(temp2) > 4)
			goto next;

		for (j = 0; j < strlen(this_char) ; j++) {
			if (this_char[j] > 'f' || this_char[j] < '0')
				goto LabelOK;
		}

		for (j =  0; j < strlen(temp2); j++) {
			if (temp2[j] > 'f' || temp2[j] < '0')
				goto LabelOK;
		}

		/*MAC Addr */
		for (k = strlen(this_char), j = strlen(this_char) - 1; j >= 0; j--)
			this_char[4 - k + j] = this_char[j];

		while (k < 4)
			this_char[3 - k++] = '0';

		this_char[4] = '\0';

		for (k = strlen(temp2), j = strlen(temp2) - 1; j >= 0; j--)
			temp2[4 - k + j] = temp2[j];

		while (k < 4)
			temp2[3 - k++] = '0';

		temp2[4] = '\0';
		AtoH(this_char, temp, 2);
		e2pAddrStart = *temp * 256 + temp[1];
		AtoH(temp2, temp, 2);
		e2pAddrEnd = *temp * 256 + temp[1];

		/* DBGPRINT(RT_DEBUG_ERROR, ("[Jason]e2pAddrStart=%x, e2pAddrEnd=%x\n", e2pAddrStart, e2pAddrEnd)); */
		if (e2pAddrEnd > e2pAddrStart) {
			ContentLen = (UINT32)(e2pAddrEnd - e2pAddrStart);
			os_alloc_mem(NULL, (UCHAR **)&pMacContent, ContentLen);

			if (pMacContent == NULL)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory fail!\n");
			else {
				/* get MAC content */
				USHORT eepValue = 0;

				pMacIndex = pMacContent;

				while (e2pAddrStart <= e2pAddrEnd) {
					RT28xx_EEPROM_READ16(pAd, e2pAddrStart, eepValue);
					*pMacIndex = eepValue;
					/* DBGPRINT(RT_DEBUG_ERROR, ("eepAddr=%08x, eepValue=%04x\n", e2pAddrStart, eepValue)); */
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "eepAddr=%08x, eepValue=%04x\n", e2pAddrStart, eepValue);
					pMacIndex++;
					e2pAddrStart += 2;
				}

				/* print content to a file */
				/* RtmpDrvRangeE2PPrint(pAd, pMacContent, e2pAddrStart, e2pAddrEnd, 2); */
				os_free_mem(pMacContent);
			    }
			} else {
				msg_left = string_size - strlen(msg);
				ret = snprintf(msg + strlen(msg), msg_left, "wrong input range!\n");
				if (os_snprintf_error(msg_left, ret)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RTMPIoctlE2PROM snprintf error!\n");
					goto LabelOK;
				}
			}
	}

next:

	if (bIsPrintAllE2P) {
		USHORT *pMacContent, *pMacIndex;
		UINT32 ContentLen;
		UINT32 AddrEnd = 0xFE;

		ContentLen = AddrEnd + 2;
		os_alloc_mem(NULL, (UCHAR **)&pMacContent, ContentLen);

		if (pMacContent == NULL)
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory fail!\n");
		else {
			/* get MAC content */
			USHORT eepAddr = 0;
			USHORT eepValue = 0;

			pMacIndex = pMacContent;

			while (eepAddr <= AddrEnd) {
				RT28xx_EEPROM_READ16(pAd, eepAddr, eepValue);
				*pMacIndex = eepValue;
				pMacIndex++;
				eepAddr += 2;
			}

			/* print content to a file */
			RtmpDrvAllE2PPrint(pAd, pMacContent, AddrEnd, 2);
			os_free_mem(pMacContent);
		}

	}

	if (strlen(msg) == 1) {
		msg_left = string_size - strlen(msg);
		ret = snprintf(msg + strlen(msg), msg_left, "===>Error command format!");
		if (os_snprintf_error(msg_left, ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RTMPIoctlE2PROM snprintf error!\n");
		goto LabelOK;
		}
	}

	/* Copy the information into the user buffer */
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
LabelOK:

	if (msg != NULL)
		os_free_mem(msg);

	if (arg != NULL)
		os_free_mem(arg);

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<==RTMPIoctlE2PROM\n");
}




#ifdef RTMP_RF_RW_SUPPORT


/*
 *    ==========================================================================
 *    Description:
 *	Read / Write RF register
 *Arguments:
 *    pAd                    Pointer to our adapter
 *    wrq                         Pointer to the ioctl argument
 *
 *    Return Value:
 *	None
 *
 *    Note:
 *	Usage:
 *	       1.) iwpriv ra0 rf                ==> read all RF registers
 *	       2.) iwpriv ra0 rf 1              ==> read RF where RegID=1
 *	       3.) iwpriv ra0 rf 1=10		    ==> write RF R1=0x10
 *    ==========================================================================
 */
VOID RTMPIoctlRF(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq)
{
	do {
	} while (0);

	return;
}
#endif /* RTMP_RF_RW_SUPPORT */
#endif /* DBG */




VOID RTMPIoctlBbp(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq,
	IN CHAR *extra,
	IN UINT32 size)
{
}




#ifdef DOT11_N_SUPPORT
void	getBaInfo(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *pOutBuf,
	IN	UINT32			size)
{
	INT i, j, ret;
	BA_ORI_ENTRY *pOriBAEntry;
	BA_REC_ENTRY *pRecBAEntry;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct ba_control *ba_ctl = &tr_ctl->ba_ctl;
	UINT32 pOutBuf_left;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_TDLS(pEntry)) && (pEntry->Sst == SST_ASSOC))
			|| IS_ENTRY_WDS(pEntry) || IS_ENTRY_MESH(pEntry)) {
			pOutBuf_left = size - strlen(pOutBuf);
			ret = snprintf(pOutBuf + strlen(pOutBuf), pOutBuf_left,
					"\n%02X:%02X:%02X:%02X:%02X:%02X (Aid = %d) (AP) -\n",
					pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
					pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5], pEntry->Aid);
			if (os_snprintf_error(pOutBuf_left, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "getBaInfo snprintf error!\n");
				return;
				}
			pOutBuf_left = size - strlen(pOutBuf);
			ret = snprintf(pOutBuf + strlen(pOutBuf), pOutBuf_left, "[Recipient]\n");
			if (os_snprintf_error(pOutBuf_left, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "getBaInfo snprintf error!\n");
				return;
				}

			for (j = 0; j < NUM_OF_TID; j++) {
				if (pEntry->BARecWcidArray[j] != 0) {
					pRecBAEntry = &ba_ctl->BARecEntry[pEntry->BARecWcidArray[j]];
					pOutBuf_left = size - strlen(pOutBuf);
					ret = snprintf(pOutBuf + strlen(pOutBuf), pOutBuf_left,
							"TID=%d, BAWinSize=%d, LastIndSeq=%d, ReorderingPkts=%d\n",
							j, pRecBAEntry->BAWinSize, pRecBAEntry->LastIndSeq, pRecBAEntry->list.qlen);
					if (os_snprintf_error(pOutBuf_left, ret)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "getBaInfo snprintf error!\n");
						return;
					}
				}
			}

			pOutBuf_left = size - strlen(pOutBuf);
			ret = snprintf(pOutBuf + strlen(pOutBuf), pOutBuf_left, "\n");
			if (os_snprintf_error(pOutBuf_left, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "getBaInfo snprintf error!\n");
				return;
			}
			pOutBuf_left = size - strlen(pOutBuf);
			ret = snprintf(pOutBuf + strlen(pOutBuf), pOutBuf_left, "[Originator]\n");
			if (os_snprintf_error(pOutBuf_left, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "getBaInfo snprintf error!\n");
				return;
			}

			for (j = 0; j < NUM_OF_TID; j++) {
				if (pEntry->BAOriWcidArray[j] != 0) {
					pOriBAEntry = &ba_ctl->BAOriEntry[pEntry->BAOriWcidArray[j]];
					pOutBuf_left = size - strlen(pOutBuf);
					ret = snprintf(pOutBuf + strlen(pOutBuf), pOutBuf_left,
							"TID=%d, BAWinSize=%d, StartSeq=%d, CurTxSeq=%d\n",
							j, pOriBAEntry->BAWinSize, pOriBAEntry->Sequence,
							tr_ctl->tr_entry[pEntry->wcid].TxSeq[j]);
					if (os_snprintf_error(pOutBuf_left, ret)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "getBaInfo snprintf error!\n");
						return;
					}
				}
			}

			pOutBuf_left = size - strlen(pOutBuf);
			ret = snprintf(pOutBuf + strlen(pOutBuf), pOutBuf_left, "\n\n");
			if (os_snprintf_error(size - pOutBuf_left, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "getBaInfo snprintf error!\n");
				return;
			}
		}

		if (strlen(pOutBuf) > (size - 30))
			break;
	}

	return;
}
#endif /* DOT11_N_SUPPORT */


VOID RTMPIoctlShow(
	IN	PRTMP_ADAPTER			pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	UINT32					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	RT_CMD_STA_IOCTL_SHOW *pIoctlShow = (RT_CMD_STA_IOCTL_SHOW *)pData;
	INT Status = 0, ret;
	char *extra = (char *)pIoctlShow->pData;
	UINT32 size = (UINT32)(pIoctlShow->MaxSize);
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	RTMP_STRING *this_char;
	struct wifi_dev *wdev = &pStaCfg->wdev;

	os_alloc_mem(NULL, (PUCHAR *)&this_char, wrq->u.data.length + 1);
	if (this_char == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory fail!\n");
		return;
	}
	NdisZeroMemory(this_char,  wrq->u.data.length + 1);
	if (copy_from_user(this_char, wrq->u.data.pointer, wrq->u.data.length)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Copy from user failed\n");
	}
	this_char[wrq->u.data.length] = '\0';
		switch (subcmd) {
#ifdef ETH_CONVERT_SUPPORT
#ifdef MAT_SUPPORT

		case SHOW_IPV4_MAT_INFO: {
			extern VOID getIPMacTbInfo(MAT_STRUCT *, char *, ULONG);
			getIPMacTbInfo(&pAd->MatCfg, extra, size);
			wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
		}
		break;

		case SHOW_IPV6_MAT_INFO: {
			extern VOID getIPv6MacTbInfo(MAT_STRUCT *, char *, ULONG);
			getIPv6MacTbInfo(&pAd->MatCfg, extra, size);
			wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
		}
		break;

		case SHOW_ETH_CLONE_MAC:
			ret = snprintf(extra, size, "%02X:%02X:%02X:%02X:%02X:%02X\n", pAd->EthConvert.EthCloneMac[0],
					 pAd->EthConvert.EthCloneMac[1],
					 pAd->EthConvert.EthCloneMac[2],
					 pAd->EthConvert.EthCloneMac[3],
					 pAd->EthConvert.EthCloneMac[4],
					 pAd->EthConvert.EthCloneMac[5]);
			if (os_snprintf_error(size, ret))
				goto err_out;
			wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
			break;
#endif /* MAT_SUPPORT */
#endif /* ETH_CONVERT_SUPPORT */

		case SHOW_CONN_STATUS:
			if (MONITOR_ON(pAd)) {
#ifdef DOT11_N_SUPPORT

				if (WMODE_CAP_N(wdev->PhyMode) &&
					wlan_operate_get_ht_bw(wdev)) {
					ret = snprintf(extra, size, "Monitor Mode(CentralChannel %d)\n",
						wlan_operate_get_cen_ch_1(wdev));
					if (os_snprintf_error(size, ret))
						goto err_out;
				} else {
#endif /* DOT11_N_SUPPORT */
					ret = snprintf(extra, size, "Monitor Mode(Channel %d)\n", wdev->channel);
					if (os_snprintf_error(size, ret))
						goto err_out;
					}
			} else {
				if (pAd->IndicateMediaState == NdisMediaStateConnected) {
					if (INFRA_ON(pStaCfg)) {
						ret = snprintf(extra, size, "Connected(AP: %s[%02X:%02X:%02X:%02X:%02X:%02X])\n",
								 pStaCfg->Ssid,
								 pStaCfg->Bssid[0],
								 pStaCfg->Bssid[1],
								 pStaCfg->Bssid[2],
								 pStaCfg->Bssid[3],
								 pStaCfg->Bssid[4],
								 pStaCfg->Bssid[5]);
						if (os_snprintf_error(size, ret))
							goto err_out;
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Ssid=%s ,Ssidlen = %d\n", pStaCfg->Ssid, pStaCfg->SsidLen);
					} else if (ADHOC_ON(pAd)) {
						ret = snprintf(extra, size, "Connected\n");
						if (os_snprintf_error(size, ret))
							goto err_out;
						}
				} else {
					ret = snprintf(extra, size, "Disconnected\n");
					if (os_snprintf_error(size, ret))
						goto err_out;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ConnStatus is not connected\n");
				}
			}

			wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
			break;

		case SHOW_DRVIER_VERION:
			/* remove __DATE__, __TIME/__ for checkpatch.pl check*/
			ret = snprintf(extra, size, "Driver version-%s\n", STA_DRIVER_VERSION);
			if (os_snprintf_error(size, ret))
				goto err_out;
			wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
			break;
#ifdef DOT11_N_SUPPORT

		case SHOW_BA_INFO:
			getBaInfo(pAd, extra, size);
			wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
			break;
#endif /* DOT11_N_SUPPORT */

		case SHOW_DESC_INFO: {
			Show_DescInfo_Proc(pAd, NULL);
			wrq->u.data.length = 0; /* 1: size of '\0' */
		}
		break;

		case RAIO_OFF:

			/* Link down first if any association exists*/
			if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
				if (INFRA_ON(pStaCfg) || ADHOC_ON(pAd)) {
					cntl_disconnect_request(wdev,
											CNTL_DISASSOC,
											pStaCfg->Bssid,
											REASON_DISASSOC_STA_LEAVING);
						RtmpusecDelay(1000);

				}
			}

			RTMP_MLME_RESET_STATE_MACHINE(pAd, &pStaCfg->wdev);

			pStaCfg->bSwRadio = FALSE;

			if (pStaCfg->bRadio != (pStaCfg->bHwRadio && pStaCfg->bSwRadio)) {
				pStaCfg->bRadio = (pStaCfg->bHwRadio && pStaCfg->bSwRadio);

				if (pStaCfg->bRadio == FALSE) {
					MlmeRadioOff(pAd, wdev);
					/* Update extra information */
					pAd->ExtraInfo = SW_RADIO_OFF;
				}
			}

			ret = snprintf(extra, size, "Radio Off\n");
			if (os_snprintf_error(size, ret))
				goto err_out;
			wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
			break;

		case RAIO_ON:
			pStaCfg->bSwRadio = TRUE;
			/*if (pStaCfg->bRadio != (pStaCfg->bHwRadio && pStaCfg->bSwRadio)) */
			{
				pStaCfg->bRadio = (pStaCfg->bHwRadio && pStaCfg->bSwRadio);

				if (pStaCfg->bRadio == TRUE) {
					MlmeRadioOn(pAd, wdev);
					/* Update extra information */
					pAd->ExtraInfo = EXTRA_INFO_CLEAR;
				}
			}
			ret = snprintf(extra, size, "Radio On\n");
			if (os_snprintf_error(size, ret))
				goto err_out;
			wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
			break;
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

		case SHOW_TDLS_ENTRY_INFO: {
#ifdef DOT11Z_TDLS_SUPPORT
			Set_TdlsEntryInfo_Display_Proc(pAd, NULL);
#else
			cfg_tdls_EntryInfo_Display_Proc(pAd, NULL);
#endif
			wrq->u.data.length = 0; /* 1: size of '\0' */
		}
		break;
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

		case SHOW_CFG_VALUE: {
			Status = RTMPShowCfgValue(pAd, (RTMP_STRING *) this_char,
									  extra, pIoctlShow->MaxSize);

			if (Status == 0)
				wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
		}
		break;

		case SHOW_ADHOC_ENTRY_INFO:
			Show_Adhoc_MacTable_Proc(pAd, extra, size);
			wrq->u.data.length = strlen(extra) + 1; /* 1: size of '\0' */
			break;

		case SHOW_DEV_INFO:
			show_devinfo_proc(pAd, NULL);
			wrq->u.data.length = 0;
			break;

		case SHOW_STA_INFO:
			Show_MacTable_Proc(pAd, "ap");
			wrq->u.data.length = 0;
			break;

		case SHOW_TR_INFO:
			show_trinfo_proc(pAd, NULL);
			wrq->u.data.length = 0;
			break;

		case SHOW_TP_INFO:
			show_tpinfo_proc(pAd, NULL);
			wrq->u.data.length = 0;
			break;

		case SHOW_SYS_INFO:
			show_sysinfo_proc(pAd, NULL);
			wrq->u.data.length = 0;
			break;

		case SHOW_PWR_INFO:
			chip_show_pwr_info(pAd, NULL);
			wrq->u.data.length = 0;
			break;

		case SHOW_DIAGNOSE_INFO:
			/* Show_Diag_Proc(pAd, NULL); */
			wrq->u.data.length = 0;
			break;
#ifdef MT_MAC

		case SHOW_WTBL_INFO:
			show_wtbl_proc(pAd, this_char);
			wrq->u.data.length = 0;
			break;

		case SHOW_MIB_INFO:
			show_mib_proc(pAd, this_char);
			wrq->u.data.length = 0;
			break;
#ifdef DBDC_MODE

		case SHOW_DBDC_INFO:
			ShowDbdcProc(pAd, NULL);
			wrq->u.data.length = 0;
			break;
#endif /*DBDC_MODE*/

		case SHOW_CHCTRL_INFO:
			ShowChCtrl(pAd, NULL);
			wrq->u.data.length = 0;
			break;
		case SHOW_TXOP_INFO:
			show_tx_burst_info(pAd, NULL);
			wrq->u.data.length = 0;
			break;

		case SHOW_TMAC_INFO:
			ShowTmacInfo(pAd, NULL);
			wrq->u.data.length = 0;
			break;

		case SHOW_AGG_INFO:
			ShowAggInfo(pAd, NULL);
			wrq->u.data.length = 0;
			break;

		case SHOW_PSE_INFO:
			ShowPseInfo(pAd, NULL);
			wrq->u.data.length = 0;
			break;

		case SHOW_PLE_INFO:
			ShowPLEInfo(pAd, this_char);
			wrq->u.data.length = 0;
			break;

		case SHOW_TXD_INFO:
			show_TXD_proc(pAd, this_char);
			wrq->u.data.length = 0;
			break;

		case DUMP_MEM_INFO:
			show_mem_proc(pAd, this_char);
			wrq->u.data.length = 0;
			break;

		case SHOW_PSE_DATA:
			ShowPseData(pAd, this_char);
			wrq->u.data.length = 0;
			break;

		case SHOW_DSCH_INFO:
			show_dmasch_proc(pAd, NULL);
			wrq->u.data.length = 0;
			break;

		case SHOW_WTBLTLV_INFO:
			show_wtbltlv_proc(pAd, this_char);
			wrq->u.data.length = 0;
			break;
#endif /* MT_MAC */
#ifdef MEM_ALLOC_INFO_SUPPORT

		case SHOW_MEM_INFO:
			Show_MemInfo_Proc(pAd, NULL);
			wrq->u.data.length = 0;
			break;

		case SHOW_PKTMEM_INFO:
			Show_PktInfo_Proc(pAd, NULL);
			wrq->u.data.length = 0;
			break;
#endif /* MEM_ALLOC_INFO_SUPPORT */

		case SHOW_WIFI_INT_CNT:
			ShowWifiInterruptCntProc(pAd, NULL);
			wrq->u.data.length = 0;
			break;
#ifdef RTMP_EFUSE_SUPPORT
		case SHOW_EFUSE_INFO:
			show_efuseinfo_proc(pAd, NULL);
			wrq->u.data.length = 0;
			break;
#endif
#ifdef REDUCE_TCP_ACK_SUPPORT

		case SHOW_TCP_RACK_INFO:
			Show_ReduceAckInfo_Proc(pAd, NULL);
			wrq->u.data.length = 0;
			break;
#endif

		case SHOW_E2P_INFO:
			show_e2pinfo_proc(pAd, NULL);
			wrq->u.data.length = 0;
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Unknow subcmd = %d\n", subcmd);
			break;
		}
	os_free_mem((PUCHAR)this_char);
	return;

err_out:
	os_free_mem((PUCHAR)this_char);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RTMPIoctlShow snprintf error!\n");
}


/* ------------------- Functions for Standard IOCTL ------------------------- */
#define RTMP_STA_STANDARD_IOCTL_HANDLE(__pAd, __pData, __Data, __SubCmd, __wdev)	\
	do {																		\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWFREQ:									\
		return RtmpIoctl_rt_ioctl_siwfreq(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWFREQ:									\
		return RtmpIoctl_rt_ioctl_giwfreq(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWMODE:									\
		return RtmpIoctl_rt_ioctl_siwmode(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWMODE:									\
		return RtmpIoctl_rt_ioctl_giwmode(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWAP:									\
		return RtmpIoctl_rt_ioctl_siwap(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWAP:									\
		return RtmpIoctl_rt_ioctl_giwap(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWSCAN:									\
		return RtmpIoctl_rt_ioctl_siwscan(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWSCAN:									\
		return RtmpIoctl_rt_ioctl_giwscan(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWESSID:									\
		return RtmpIoctl_rt_ioctl_siwessid(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWESSID:									\
		return RtmpIoctl_rt_ioctl_giwessid(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWNICKN:									\
		return RtmpIoctl_rt_ioctl_siwnickn(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWNICKN:									\
		return RtmpIoctl_rt_ioctl_giwnickn(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWRTS:									\
		return RtmpIoctl_rt_ioctl_siwrts(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWRTS:									\
		return RtmpIoctl_rt_ioctl_giwrts(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWFRAG:									\
		return RtmpIoctl_rt_ioctl_siwfrag(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWFRAG:									\
		return RtmpIoctl_rt_ioctl_giwfrag(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWENCODE:								\
		return RtmpIoctl_rt_ioctl_siwencode(__pAd, __pData, __Data);		\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWENCODE:								\
		return RtmpIoctl_rt_ioctl_giwencode(__pAd, __pData, __Data);		\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWMLME:									\
		return RtmpIoctl_rt_ioctl_siwmlme(__pAd, __pData, __Data, __SubCmd);\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWAUTH:									\
		return RtmpIoctl_rt_ioctl_siwauth(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWAUTH:									\
		return RtmpIoctl_rt_ioctl_giwauth(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWENCODEEXT:								\
		return RtmpIoctl_rt_ioctl_siwencodeext(__pAd, __pData, __Data, __wdev);		\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWENCODEEXT:								\
		return RtmpIoctl_rt_ioctl_giwencodeext(__pAd, __pData, __Data);		\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWGENIE:									\
		return RtmpIoctl_rt_ioctl_siwgenie(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWGENIE:									\
		return RtmpIoctl_rt_ioctl_giwgenie(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWPMKSA:									\
		return RtmpIoctl_rt_ioctl_siwpmksa(__pAd, __pData, __Data);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWRATE:									\
		return RtmpIoctl_rt_ioctl_siwrate(__pAd, __pData, __Data, __wdev);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIWRATE:									\
		return RtmpIoctl_rt_ioctl_giwrate(__pAd, __pData, __Data, __wdev);			\
	case CMD_RTPRIV_IOCTL_STA_SIOCGIFHWADDR:								\
		return RtmpIoctl_rt_ioctl_gifhwaddr(__pAd, __pData, __Data);		\
	case CMD_RTPRIV_IOCTL_STA_SIOCSIWPRIVRSSI:								\
		return RtmpIoctl_rt_ioctl_rssi(__pAd, __pData, __Data);		\
	case CMD_RTPRIV_IOCTL_STA_IW_SET_WSC_U32_ITEM:							\
		return RtmpIoctl_rt_private_set_wsc_u32_item(__pAd, __pData, __Data);\
	case CMD_RTPRIV_IOCTL_STA_IW_SET_WSC_STR_ITEM:							\
		return RtmpIoctl_rt_private_set_wsc_string_item(__pAd, __pData, __Data);\
	case CMD_RTPRIV_IOCTL_STA_IW_GET_STATISTICS:							\
		return RtmpIoctl_rt_private_get_statistics(__pAd, __pData, __Data);	\
	} while (0)


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWFREQ.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_siwfreq(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	RT_CMD_STA_IOCTL_FREQ *pIoctlFreq = (RT_CMD_STA_IOCTL_FREQ *)pData;
	int	chan = -1;
	ULONG	freq;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;

	if (pIoctlFreq->m > 100000000)
		freq = pIoctlFreq->m / 100000;
	else if (pIoctlFreq->m > 100000)
		freq = pIoctlFreq->m / 100;
	else
		freq = pIoctlFreq->m;

	if ((pIoctlFreq->e == 0) && (freq <= 1000))
		chan = pIoctlFreq->m;	/* Setting by channel number */
	else
		MAP_KHZ_TO_CHANNEL_ID(freq, chan); /* Setting by frequency - search the table , like 2.412G, 2.422G, */

	if (ChannelSanity(pAd, chan) == TRUE) {
		wdev->channel = chan;
		/* Save the channel on MlmeAux for CntlOidRTBssidProc used. */
		pStaCfg->MlmeAux.Channel = wdev->channel;
		/*save connect info*/
		pStaCfg->ConnectinfoChannel = wdev->channel;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SIOCSIWFREQ(Channel=%d)\n", wdev->channel);
	} else
		return NDIS_STATUS_FAILURE;

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWFREQ.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_giwfreq(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	UCHAR ch;
	UINT32	m = 2412000;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;
		ch = wdev->channel;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Channel:%d\n", ch);
	MAP_CHANNEL_ID_TO_KHZ(ch, m);
	*(ULONG *)pData = (ULONG)m;
	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWMODE.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_siwmode(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	switch (Data) {
	case RTMP_CMD_STA_MODE_ADHOC:
		Set_NetworkType_Proc(pAd, "Adhoc");
		break;

	case RTMP_CMD_STA_MODE_INFRA:
		Set_NetworkType_Proc(pAd, "Infra");
		break;

	case RTMP_CMD_STA_MODE_MONITOR:
		Set_NetworkType_Proc(pAd, "Monitor");
		break;
	}

	/* Reset Ralink supplicant to not use, it will be set to start when UI set PMK key */
	pStaCfg->WpaState = SS_NOTUSE;
	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWMODE.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_giwmode(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT32 staidx = 0;

	if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pObj->ioctl_if is invalid value\n");
		return NDIS_STATUS_INVALID_DATA;
	}
	staidx = pObj->ioctl_if;
	if (ADHOC_ON(pAd))
		*(ULONG *)pData = RTMP_CMD_STA_MODE_ADHOC;
	else if (INFRA_ON(&pAd->StaCfg[staidx]))
		*(ULONG *)pData = RTMP_CMD_STA_MODE_INFRA;
	else if (MONITOR_ON(pAd))
		*(ULONG *)pData = RTMP_CMD_STA_MODE_MONITOR;
	else
		*(ULONG *)pData = RTMP_CMD_STA_MODE_AUTO;

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWAP.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_siwap(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	UCHAR *pBssid = (UCHAR *)pData;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	/* tell CNTL state machine to call NdisMSetInformationComplete() after completing */
	/* this request, because this request is initiated by NDIS. */
	pStaCfg->MlmeAux.CurrReqIsFromNdis = FALSE;
	/* Prevent to connect AP again in STAMlmePeriodicExec */
	pStaCfg->MlmeAux.AutoReconnectSsidLen = 32;

	if (MAC_ADDR_EQUAL(pBssid, ZERO_MAC_ADDR)) {
		if (INFRA_ON(pStaCfg))
			LinkDown(pAd, FALSE, &pStaCfg->wdev, NULL);
	} else {
		cntl_connect_request(&pStaCfg->wdev,
							 CNTL_CONNECT_BY_BSSID, MAC_ADDR_LEN, pBssid);
	}

	return NDIS_STATUS_SUCCESS;
}

/**
 *  adapter_to_ioctl_sta_cfg - get sta_cfg from StaCfg[] in pAd
 *
 *  @pAd	A pointer to RTMP_ADAPTER
 *
 *  Returns: a pointer to WPA_SUPPLICANT_INFO on success, NULL on failure
 */
static STA_ADMIN_CONFIG *adapter_to_ioctl_sta_cfg(RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj;
	INT ioctl_if;

	if (!pAd || !pAd->OS_Cookie)
		return NULL;

	pObj = (POS_COOKIE)pAd->OS_Cookie;
	ioctl_if = pObj->ioctl_if;

	if (ioctl_if >= 0 && ioctl_if < ARRAY_SIZE(pAd->StaCfg))
		return &pAd->StaCfg[ioctl_if];
	else
		return NULL;
}

/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWAP.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_giwap(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	STA_ADMIN_CONFIG *pStaCfg;
#ifdef WPA_SUPPLICANT_SUPPORT
	WPA_SUPPLICANT_INFO *supc_i;
#endif /* WPA_SUPPLICANT_SUPPORT */
	pStaCfg = adapter_to_ioctl_sta_cfg(pAd);
	if (pStaCfg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pStaCfg is NULL!!!\n");
		return NDIS_STATUS_FAILURE;
	}
#ifdef WPA_SUPPLICANT_SUPPORT
	supc_i = (pStaCfg) ? &pStaCfg->wpa_supplicant_info : NULL;
#endif /* WPA_SUPPLICANT_SUPPORT */

	if (INFRA_ON(pStaCfg) || ADHOC_ON(pAd))
		NdisCopyMemory(pData, pStaCfg->Bssid, MAC_ADDR_LEN);

#ifdef WPA_SUPPLICANT_SUPPORT
	/* GeK: temp workaround for build error */
	else if (supc_i && supc_i->WpaSupplicantUP != WPA_SUPPLICANT_DISABLE)
		NdisCopyMemory(pData, pStaCfg->MlmeAux.Bssid, MAC_ADDR_LEN);

#endif /* WPA_SUPPLICANT_SUPPORT */
	else
		return NDIS_STATUS_FAILURE;

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWSCAN.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT RtmpIoctl_rt_ioctl_siwscan(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;
	UINT scanType = SCAN_ACTIVE;
	/*
	 *	Can not use SIOCGIWSCAN definition, it is used in wireless.h
	 *	We will not see the definition in MODULE.
	 *	The definition can be saw in UTIL and NETIF.
	 */
	/* #if defined(SIOCGIWSCAN) || defined(RT_CFG80211_SUPPORT) */
	RT_CMD_STA_IOCTL_SCAN *pConfig = (RT_CMD_STA_IOCTL_SCAN *)pData;
	int Status = NDIS_STATUS_SUCCESS;
#ifdef ANDROID_SUPPORT

	if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
#ifdef IFUP_IN_PROBE
		|| (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
		|| (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
#endif /* IFUP_IN_PROBE */
	   ) {
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_SCAN, -1, NULL, NULL, 0);
		return NDIS_STATUS_SUCCESS;
	}

#endif /* ANDROID_SUPPORT */
	pConfig->Status = 0;

	if (MONITOR_ON(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "!!! Driver is in Monitor Mode now !!!\n");
		pConfig->Status = RTMP_IO_EINVAL;
		return NDIS_STATUS_FAILURE;
	}

#ifdef WPA_SUPPLICANT_SUPPORT

	if ((pStaCfg->wpa_supplicant_info.WpaSupplicantUP & 0x7F) == WPA_SUPPLICANT_ENABLE)
		pStaCfg->wpa_supplicant_info.WpaSupplicantScanCount++;

#endif /* WPA_SUPPLICANT_SUPPORT */
	pStaCfg->bSkipAutoScanConn = TRUE;

	do {

		if ((STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) &&
			IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap) &&
			(wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "!!! Link UP, Port Not Secured! ignore this set::OID_802_11_BSSID_LIST_SCAN\n");
			Status = NDIS_STATUS_SUCCESS;
			break;
		}

		if (pConfig->ScanType != 0)
			scanType = pConfig->ScanType;
		else
			scanType = SCAN_ACTIVE;

#ifdef WPA_SUPPLICANT_SUPPORT

		if (pConfig->FlgScanThisSsid) {
			NDIS_802_11_SSID          Ssid;

			Ssid.SsidLength = pConfig->SsidLen;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "req.essid_len-%d, essid-%s\n",
					 pConfig->SsidLen, pConfig->pSsid);
			NdisZeroMemory(&Ssid.Ssid, NDIS_802_11_LENGTH_SSID);
			NdisMoveMemory(Ssid.Ssid, pConfig->pSsid, Ssid.SsidLength);
			StaSiteSurvey(pAd, &Ssid, scanType);
		} else
#endif /* WPA_SUPPLICANT_SUPPORT */
		StaSiteSurvey(pAd, NULL, scanType, wdev);
	} while (0);

	pConfig->Status = Status;
	/* #endif */ /* SIOCGIWSCAN || RT_CFG80211_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Set the signal quality.
 *
 *Arguments:
 *	*pSignal		- signal structure
 *	pBssEntry		- the BSS information
 *
 *Return Value:
 *	None
 *
 *Note:
 *========================================================================
 */
static void set_quality(
	IN RT_CMD_STA_IOCTL_BSS * pSignal,
	IN BSS_ENTRY *pBssEntry)
{
	memcpy(pSignal->Bssid, pBssEntry->Bssid, MAC_ADDR_LEN);

	/* Normalize Rssi */
	if (pBssEntry->Rssi >= -50)
		pSignal->ChannelQuality = 100;
	else if (pBssEntry->Rssi >= -80) /* between -50 ~ -80dbm */
		pSignal->ChannelQuality = (UCHAR)(24 + ((pBssEntry->Rssi + 80) * 26) / 10);
	else if (pBssEntry->Rssi >= -90)   /* between -80 ~ -90dbm */
		pSignal->ChannelQuality = (UCHAR)((pBssEntry->Rssi + 90) * 26) / 10;
	else
		pSignal->ChannelQuality = 0;

	pSignal->Rssi = (UCHAR)(pBssEntry->Rssi);

	if (pBssEntry->Rssi >= -70)
		pSignal->Noise = -92;
	else
		pSignal->Noise = pBssEntry->Rssi - pBssEntry->MinSNR;
}


/*
 *========================================================================
 * *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWSCAN.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_giwscan(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	RT_CMD_STA_IOCTL_SCAN_TABLE *pIoctlScan = (RT_CMD_STA_IOCTL_SCAN_TABLE *)pData;
	RT_CMD_STA_IOCTL_BSS_TABLE *pBssTable;
	BSS_ENTRY *pBssEntry;
	UINT32 IdBss;
	struct wifi_dev *wdev = &pStaCfg->wdev;
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

	pIoctlScan->BssNr = 0;
#ifdef WPA_SUPPLICANT_SUPPORT

	if ((pStaCfg->wpa_supplicant_info.WpaSupplicantUP & 0x7F) == WPA_SUPPLICANT_ENABLE)
		pStaCfg->wpa_supplicant_info.WpaSupplicantScanCount = 0;

#endif /* WPA_SUPPLICANT_SUPPORT */
	pIoctlScan->BssNr = ScanTab->BssNr;

	if (pIoctlScan->BssNr == 0)
		return NDIS_STATUS_SUCCESS;

	os_alloc_mem(NULL, (UCHAR **) &(pIoctlScan->pBssTable),
				 ScanTab->BssNr * sizeof(RT_CMD_STA_IOCTL_BSS_TABLE));

	if (pIoctlScan->pBssTable == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory fail!\n");
		return NDIS_STATUS_FAILURE;
	}
	NdisZeroMemory(pIoctlScan->pBssTable, sizeof(RT_CMD_STA_IOCTL_BSS_TABLE));
	for (IdBss = 0; IdBss < ScanTab->BssNr; IdBss++) {
		HT_CAP_INFO capInfo = ScanTab->BssEntry[IdBss].HtCapability.HtCapInfo;

		pBssTable = pIoctlScan->pBssTable + IdBss;
		pBssEntry = &ScanTab->BssEntry[IdBss];
		memcpy(pBssTable->Bssid, pBssEntry->Bssid, ETH_ALEN);
		pBssTable->Channel = pBssEntry->Channel;
		pBssTable->BssType = pBssEntry->BssType;
		if (HAS_HT_CAPS_EXIST(pBssEntry->ie_exists))
			pBssTable->has_ht_cap = TRUE;

		memcpy(pBssTable->SupRate, pBssEntry->SupRate, 12);
		pBssTable->SupRateLen = pBssEntry->SupRateLen;
		memcpy(pBssTable->ExtRate, pBssEntry->ExtRate, 12);
		pBssTable->ExtRateLen = pBssEntry->ExtRateLen;
		pBssTable->SsidLen = pBssEntry->SsidLen;
		memcpy(pBssTable->Ssid, pBssEntry->Ssid, 32);
		pBssTable->CapabilityInfo = pBssEntry->CapabilityInfo;
		pBssTable->ChannelWidth = capInfo.ChannelWidth;
		pBssTable->ShortGIfor40 = capInfo.ShortGIfor40;
		pBssTable->ShortGIfor20 = capInfo.ShortGIfor20;
		pBssTable->MCSSet = pBssEntry->HtCapability.MCSSet[1];
		pBssTable->WpaIeLen = pBssEntry->WpaIE.IELen;
		pBssTable->pWpaIe = pBssEntry->WpaIE.IE;
		pBssTable->RsnIeLen = pBssEntry->RsnIE.IELen;
		pBssTable->pRsnIe = pBssEntry->RsnIE.IE;
		pBssTable->WpsIeLen = pBssEntry->WpsIE.IELen;
		pBssTable->pWpsIe = pBssEntry->WpsIE.IE;
		pBssTable->FlgIsPrivacyOn = CAP_IS_PRIVACY_ON(pBssEntry->CapabilityInfo);
		set_quality(&pBssTable->Signal, pBssEntry);
	}

	memcpy(pIoctlScan->MainSharedKey[0], pAd->SharedKey[BSS0][0].Key, 16);
	memcpy(pIoctlScan->MainSharedKey[1], pAd->SharedKey[BSS0][1].Key, 16);
	memcpy(pIoctlScan->MainSharedKey[2], pAd->SharedKey[BSS0][2].Key, 16);
	memcpy(pIoctlScan->MainSharedKey[3], pAd->SharedKey[BSS0][3].Key, 16);
	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 * *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWESSID.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_siwessid(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	RT_CMD_STA_IOCTL_SSID *pSsid = (RT_CMD_STA_IOCTL_SSID *)pData;

	if (pSsid->FlgAnySsid) {
		RTMP_STRING *pSsidString = NULL;
		/* Includes null character. */
		os_alloc_mem(NULL, (UCHAR **)&pSsidString, MAX_LEN_OF_SSID + 1);

		if (pSsidString) {
			NdisZeroMemory(pSsidString, MAX_LEN_OF_SSID + 1);
			NdisMoveMemory(pSsidString, pSsid->pSsid, pSsid->SsidLen);

			if (Set_SSID_Proc(pAd, pSsidString) == FALSE) {
				os_free_mem(pSsidString);
				pSsid->Status = RTMP_IO_EINVAL;
				return NDIS_STATUS_SUCCESS;
			}

			os_free_mem(pSsidString);
		} else {
			pSsid->Status = RTMP_IO_ENOMEM;
			return NDIS_STATUS_SUCCESS;
		}
	} else {
		/* ANY ssid */
		if (Set_SSID_Proc(pAd, "") == FALSE) {
			pSsid->Status = RTMP_IO_EINVAL;
			return NDIS_STATUS_SUCCESS;
		}
	}

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWESSID.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_giwessid(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	RT_CMD_STA_IOCTL_SSID *pSsid = (RT_CMD_STA_IOCTL_SSID *)pData;
	STA_ADMIN_CONFIG *pStaCfg;
#ifdef WPA_SUPPLICANT_SUPPORT
	WPA_SUPPLICANT_INFO *supc_i;
#endif /* WPA_SUPPLICANT_SUPPORT */
	pStaCfg = adapter_to_ioctl_sta_cfg(pAd);
	if (pStaCfg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pStaCfg is NULL!!!\n");
		return NDIS_STATUS_FAILURE;
	}
#ifdef WPA_SUPPLICANT_SUPPORT
	supc_i = (pStaCfg) ? &pStaCfg->wpa_supplicant_info : NULL;
#endif /* WPA_SUPPLICANT_SUPPORT */

	if (MONITOR_ON(pAd)) {
		pSsid->SsidLen = 0;
		return NDIS_STATUS_SUCCESS;
	}

	if (STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MediaState is connected\n");
		pSsid->SsidLen = pStaCfg->SsidLen;
		memcpy(pSsid->pSsid, pStaCfg->Ssid, pStaCfg->SsidLen);
	}

#ifdef WPA_SUPPLICANT_SUPPORT
	else if (supc_i->WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) {
		pSsid->SsidLen = pStaCfg->SsidLen;
		memcpy(pSsid->pSsid, pStaCfg->Ssid, pStaCfg->SsidLen);
	}

#endif /* WPA_SUPPLICANT_SUPPORT */
	else {
		/*the ANY ssid was specified */
		pSsid->SsidLen = 0;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "MediaState is not connected, ess\n");
	}

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWNICKN.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_siwnickn(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	memset(pAd->nickname, 0, IW_ESSID_MAX_SIZE + 1);
	memcpy(pAd->nickname, pData, Data);
	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWNICKN.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_giwnickn(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	RT_CMD_STA_IOCTL_NICK_NAME *IoctlName = (RT_CMD_STA_IOCTL_NICK_NAME *)pData;

	if (IoctlName->NameLen > strlen((RTMP_STRING *) pAd->nickname) + 1)
		IoctlName->NameLen = strlen((RTMP_STRING *) pAd->nickname) + 1;

	if (IoctlName->NameLen > 0) {
		memcpy(IoctlName->pName, pAd->nickname, IoctlName->NameLen - 1);
		IoctlName->pName[IoctlName->NameLen - 1] = '\0';
	}

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWRTS.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_siwrts(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return NDIS_STATUS_SUCCESS;

	wlan_operate_set_rts_len_thld(wdev, (UINT32)Data);
	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWRTS.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_giwrts(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 len_thld = 0;

	if (!wdev)
		return NDIS_STATUS_SUCCESS;

	len_thld = wlan_operate_get_rts_len_thld(wdev);
	*(USHORT *)pData = len_thld;
	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWFRAG.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_siwfrag(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (!wdev)
		return NDIS_STATUS_SUCCESS;

	wlan_operate_set_frag_thld(wdev, (UINT32)Data);
	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWFRAG.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_giwfrag(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 frag_thld = 0;

	if (!wdev)
		return NDIS_STATUS_SUCCESS;

	frag_thld = wlan_operate_get_frag_thld(wdev);
	*(USHORT *)pData = frag_thld;
	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWENCODE.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 * *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
#define NR_WEP_KEYS				4
#define MAX_WEP_KEY_SIZE			13
#define MIN_WEP_KEY_SIZE			5
INT RtmpIoctl_rt_ioctl_siwencode(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	RT_CMD_STA_IOCTL_SECURITY *pIoctlSec = (RT_CMD_STA_IOCTL_SECURITY *)pData;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;

	if ((pIoctlSec->length == 0) &&
		(pIoctlSec->flags & RT_CMD_STA_IOCTL_SECURITY_DISABLED)) {
		SET_CIPHER_NONE(pStaCfg->PairwiseCipher);
		SET_CIPHER_NONE(pStaCfg->GroupCipher);
		SET_AKM_OPEN(wdev->SecConfig.AKMMap);
		SET_CIPHER_NONE(wdev->SecConfig.PairwiseCipher);
		goto done;
	} else if (pIoctlSec->flags & RT_CMD_STA_IOCTL_SECURITY_RESTRICTED ||
			   pIoctlSec->flags & RT_CMD_STA_IOCTL_SECURITY_OPEN) {
		wdev->PortSecured = WPA_802_1X_PORT_SECURED;
		SET_CIPHER_WEP(pStaCfg->PairwiseCipher);
		SET_CIPHER_WEP(pStaCfg->GroupCipher);
		SET_CIPHER_WEP(wdev->SecConfig.PairwiseCipher);

		if (pIoctlSec->flags & RT_CMD_STA_IOCTL_SECURITY_RESTRICTED)
			SET_AKM_SHARED(wdev->SecConfig.AKMMap);
		else
			SET_AKM_OPEN(wdev->SecConfig.AKMMap);
	}

	if (pIoctlSec->length > 0) {
		UINT32 keyIdx = pIoctlSec->KeyIdx; /*(erq->flags & IW_ENCODE_INDEX) - 1; */

		/* Check the size of the key */
		if (pIoctlSec->length > MAX_WEP_KEY_SIZE) {
			pIoctlSec->Status = RTMP_IO_EINVAL;
			return NDIS_STATUS_SUCCESS;
		}

		/* Check key index */
		if (keyIdx >= NR_WEP_KEYS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "Wrong keyIdx=%d! Using default key instead (%d)\n",
					  keyIdx, wdev->SecConfig.PairwiseKeyId);
			/*Using default key */
			keyIdx = wdev->SecConfig.PairwiseKeyId;
		} else
			wdev->SecConfig.PairwiseKeyId = keyIdx;

		NdisZeroMemory(wdev->SecConfig.WepKey[keyIdx].Key,  16);

		if (pIoctlSec->length == MAX_WEP_KEY_SIZE) {
			wdev->SecConfig.WepKey[keyIdx].KeyLen = MAX_WEP_KEY_SIZE;
			/* wdev->SecConfig.WepKey[keyIdx].CipherAlg = CIPHER_WEP128; */
		} else if (pIoctlSec->length == MIN_WEP_KEY_SIZE) {
			wdev->SecConfig.WepKey[keyIdx].KeyLen = MIN_WEP_KEY_SIZE;
			/* wdev->SecConfig.WepKey[keyIdx].CipherAlg = CIPHER_WEP64; */
		} else
			/* Disable the key */
			wdev->SecConfig.WepKey[keyIdx].KeyLen = 0;

		/* Check if the key is not marked as invalid */
		if (!(pIoctlSec->flags & RT_CMD_STA_IOCTL_SECURITY_NOKEY)) {
			/* Copy the key in the driver */
			NdisMoveMemory(wdev->SecConfig.WepKey[keyIdx].Key, pIoctlSec->pData, pIoctlSec->length);
		}
	} else {
		/* Do we want to just set the transmit key index ? */
		int index = pIoctlSec->KeyIdx; /*(erq->flags & IW_ENCODE_INDEX) - 1; */

		if ((index >= 0) && (index < 4))
			pStaCfg->wdev.SecConfig.PairwiseKeyId = index;
		else

			/* Don't complain if only change the mode */
			if (!(pIoctlSec->flags & RT_CMD_STA_IOCTL_SECURITY_MODE)) {
				pIoctlSec->Status = RTMP_IO_EINVAL;
				return NDIS_STATUS_SUCCESS;
			}
	}

done:
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "erq->flags=%x\n", pIoctlSec->flags);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AuthMode=0x%x\n",
			 wdev->SecConfig.AKMMap);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DefaultKeyId=%x, KeyLen = %d\n",
			 wdev->SecConfig.PairwiseKeyId, wdev->SecConfig.WepKey[wdev->SecConfig.PairwiseKeyId].KeyLen);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WepStatus=0x%x\n",
			 wdev->SecConfig.PairwiseCipher);
	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWENCODE.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT RtmpIoctl_rt_ioctl_giwencode(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	RT_CMD_STA_IOCTL_SECURITY *pIoctlSec = (RT_CMD_STA_IOCTL_SECURITY *)pData;
	int kid;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;

	kid = pIoctlSec->KeyIdx; /*erq->flags & IW_ENCODE_INDEX; */
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===>rt_ioctl_giwencode %d\n", kid);

	if (IS_CIPHER_NONE(wdev->SecConfig.PairwiseCipher)) {
		pIoctlSec->length = 0;
		pIoctlSec->flags = RT_CMD_STA_IOCTL_SECURITY_DISABLED;
	} else if ((kid > 0) && (kid <= 4)) {
		/* copy wep key */
		pIoctlSec->KeyIdx = kid;

		if (pIoctlSec->length > wdev->SecConfig.WepKey[kid - 1].KeyLen)
			pIoctlSec->length = wdev->SecConfig.WepKey[kid - 1].KeyLen;

		memcpy(pIoctlSec->pData, wdev->SecConfig.WepKey[kid - 1].Key, pIoctlSec->length);

		if (IS_AKM_SHARED(wdev->SecConfig.AKMMap))
			pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_RESTRICTED;		/* XXX */
		else
			pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_OPEN;		/* XXX */
	} else if (kid == 0) {
		if (IS_AKM_SHARED(wdev->SecConfig.AKMMap))
			pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_RESTRICTED;		/* XXX */
		else
			pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_OPEN;		/* XXX */

		pIoctlSec->length = wdev->SecConfig.WepKey[wdev->SecConfig.PairwiseKeyId].KeyLen;
		memcpy(pIoctlSec->pData, wdev->SecConfig.WepKey[wdev->SecConfig.PairwiseKeyId].Key, pIoctlSec->length);

		/* copy default key ID */
		if (IS_AKM_SHARED(wdev->SecConfig.AKMMap))
			pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_RESTRICTED;		/* XXX */
		else
			pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_OPEN;		/* XXX */

		pIoctlSec->KeyIdx = wdev->SecConfig.PairwiseKeyId + 1;			/* NB: base 1 */
		pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_ENABLED;	/* XXX */
	}

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWMLME.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_siwmlme(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data,
	IN	UINT32					Subcmd)
{
	MLME_QUEUE_ELEM				*pMsgElem = NULL;
	ULONG						reason_code = (ULONG)Data;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	UINT link_down_type = 0;
	struct wifi_dev *wdev;

	wdev = &pStaCfg->wdev;
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pMsgElem, sizeof(MLME_QUEUE_ELEM));

	if (pMsgElem == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory fail!!!\n");
		return NDIS_STATUS_FAILURE;
	}

	switch (Subcmd) {
	case RT_CMD_STA_IOCTL_IW_MLME_DEAUTH:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IW_MLME_DEAUTH\n");
		cntl_disconnect_request(wdev,
								CNTL_DEAUTH,
								pStaCfg->Bssid,
								(USHORT)reason_code);
		OS_WAIT(1000);	/* leave enough time for this de-auth frame */


		if (INFRA_ON(pStaCfg)) {
			LinkDown(pAd, link_down_type, &pStaCfg->wdev, NULL);
			pStaCfg->wdev.assoc_machine.CurrState = ASSOC_IDLE;
		}

		break;

	case RT_CMD_STA_IOCTL_IW_MLME_DISASSOC:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IW_MLME_DISASSOC\n");
		NdisZeroMemory(pStaCfg->ConnectinfoSsid, MAX_LEN_OF_SSID);
		NdisZeroMemory(pStaCfg->ConnectinfoBssid, MAC_ADDR_LEN);
		pStaCfg->ConnectinfoSsidLen  = 0;
		pStaCfg->ConnectinfoBssType  = 1;
		pStaCfg->ConnectinfoChannel = 0;
		cntl_disconnect_request(wdev,
								CNTL_DISASSOC,
								pStaCfg->Bssid,
								(USHORT)reason_code);
		break;
	}

	if (pMsgElem != NULL)
		os_free_mem(pMsgElem);

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWAUTH.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_siwauth(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	RT_CMD_STA_IOCTL_SECURITY_ADV *pIoctlWpa = (RT_CMD_STA_IOCTL_SECURITY_ADV *)pData;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;

	switch (pIoctlWpa->flags) {
	case RT_CMD_STA_IOCTL_WPA_VERSION:
		if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_VERSION1) {
			if (pStaCfg->BssType == BSS_ADHOC)
				SET_AKM_WPANONE(wdev->SecConfig.AKMMap);
			else
				SET_AKM_WPA1PSK(wdev->SecConfig.AKMMap);
		} else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_VERSION2)
			SET_AKM_WPA2PSK(wdev->SecConfig.AKMMap);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IW_AUTH_WPA_VERSION - param->value = %d!\n",
				 pIoctlWpa->value);
		break;

	case RT_CMD_STA_IOCTL_WPA_PAIRWISE:
		if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_PAIRWISE_NONE) {
			SET_CIPHER_NONE(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_NONE(pStaCfg->PairwiseCipher);
		} else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_PAIRWISE_WEP40 ||
				   pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_PAIRWISE_WEP104) {
			SET_CIPHER_WEP(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_WEP(pStaCfg->PairwiseCipher);
#ifdef WPA_SUPPLICANT_SUPPORT
			pStaCfg->wdev.SecConfig.IEEE8021X = FALSE;
#endif /* WPA_SUPPLICANT_SUPPORT */
		} else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_PAIRWISE_TKIP) {
			SET_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_TKIP(pStaCfg->PairwiseCipher);
		} else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_PAIRWISE_CCMP) {
			SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_CCMP128(pStaCfg->PairwiseCipher);
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IW_AUTH_CIPHER_PAIRWISE - param->value = %d!\n",
				 pIoctlWpa->value);
		break;

	case RT_CMD_STA_IOCTL_WPA_GROUP:
		if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_GROUP_NONE)
			SET_CIPHER_NONE(wdev->SecConfig.GroupCipher);
		else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_GROUP_WEP40)
			SET_CIPHER_WEP40(wdev->SecConfig.GroupCipher);
		else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_GROUP_WEP104)
			SET_CIPHER_WEP104(wdev->SecConfig.GroupCipher);
		else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_GROUP_TKIP)
			SET_CIPHER_TKIP(wdev->SecConfig.GroupCipher);
		else if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_GROUP_CCMP)
			SET_CIPHER_CCMP128(wdev->SecConfig.GroupCipher);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IW_AUTH_CIPHER_GROUP - param->value = %d!\n",
				 pIoctlWpa->value);
		break;

	case RT_CMD_STA_IOCTL_WPA_KEY_MGMT:
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
		pStaCfg->wpa_supplicant_info.WpaSupplicantUP &= 0x7F;
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */

		if (pIoctlWpa->value == RT_CMD_STA_IOCTL_WPA_KEY_MGMT_1X) {
			if (IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap)) {
				CLEAR_SEC_AKM(wdev->SecConfig.AKMMap);
				SET_AKM_WPA1(wdev->SecConfig.AKMMap);
#ifdef WPA_SUPPLICANT_SUPPORT
				wdev->SecConfig.IEEE8021X = FALSE;
#endif /* WPA_SUPPLICANT_SUPPORT */
			} else if (IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap)) {
				CLEAR_SEC_AKM(wdev->SecConfig.AKMMap);
				SET_AKM_WPA2(wdev->SecConfig.AKMMap);
#ifdef WPA_SUPPLICANT_SUPPORT
				wdev->SecConfig.IEEE8021X = FALSE;
#endif /* WPA_SUPPLICANT_SUPPORT */
			}

#ifdef WPA_SUPPLICANT_SUPPORT
			else
				/* WEP 1x */
				wdev->SecConfig.IEEE8021X = TRUE;

#endif /* WPA_SUPPLICANT_SUPPORT */
		}

#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
		else if (pIoctlWpa->value == 0)
			wdev->PortSecured = WPA_802_1X_PORT_SECURED;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IW_AUTH_KEY_MGMT - param->value = %d!\n",
				 pIoctlWpa->value);
		break;

	case RT_CMD_STA_IOCTL_WPA_AUTH_RX_UNENCRYPTED_EAPOL:
		break;

	case RT_CMD_STA_IOCTL_WPA_AUTH_PRIVACY_INVOKED:
		/*
		 *if (pIoctlWpa->value == 0)
		 *{
		 *    pStaCfg->AuthMode = Ndis802_11AuthModeOpen;
		 *    pStaCfg->WepStatus = Ndis802_11WEPDisabled;
		 *    pStaCfg->PairCipher = Ndis802_11WEPDisabled;
		 *    pStaCfg->GroupCipher = Ndis802_11WEPDisabled;
		 *}
		 */
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IW_AUTH_PRIVACY_INVOKED - param->value = %d!\n",
				 pIoctlWpa->value);
		break;

	case RT_CMD_STA_IOCTL_WPA_AUTH_DROP_UNENCRYPTED:
		if (pIoctlWpa->value != 0)
			wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		else
			wdev->PortSecured = WPA_802_1X_PORT_SECURED;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IW_AUTH_DROP_UNENCRYPTED - param->value = %d!\n",
				 pIoctlWpa->value);
		break;

	case RT_CMD_STA_IOCTL_WPA_AUTH_80211_AUTH_ALG:
		SET_AKM_AUTOSWITCH(wdev->SecConfig.AKMMap);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IW_AUTH_80211_AUTH_ALG - param->value = %d!\n",
				 pIoctlWpa->value);
		break;

	case RT_CMD_STA_IOCTL_WPA_AUTH_WPA_ENABLED:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "IW_AUTH_WPA_ENABLED - Driver supports WPA!(param->value = %d)\n", pIoctlWpa->value);
		break;

	case RT_CMD_STA_IOCTL_WPA_AUTH_COUNTERMEASURES:
		if (pIoctlWpa->value == 1)
			pStaCfg->bBlockAssoc = TRUE;
		else
			pStaCfg->bBlockAssoc = FALSE;
	}

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWAUTH.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_giwauth(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	RT_CMD_STA_IOCTL_SECURITY_ADV *pIoctlWpa = (RT_CMD_STA_IOCTL_SECURITY_ADV *)pData;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;

	switch (pIoctlWpa->flags) {
	case RT_CMD_STA_IOCTL_WPA_AUTH_DROP_UNENCRYPTED:
		if (IS_AKM_OPEN(wdev->SecConfig.AKMMap))
			pIoctlWpa->value = 0;
		else
			pIoctlWpa->value = 1;

		break;

	case RT_CMD_STA_IOCTL_WPA_AUTH_80211_AUTH_ALG:
		if (IS_AKM_SHARED(wdev->SecConfig.AKMMap))
			pIoctlWpa->value = 0;
		else
			pIoctlWpa->value = 1;

		break;

	case RT_CMD_STA_IOCTL_WPA_AUTH_WPA_ENABLED:
		if (IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap))
			pIoctlWpa->value = 1;
		else
			pIoctlWpa->value = 0;

		break;
	}

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Set security key.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	keyIdx			- key index
 *	CipherAlg		- cipher algorithm
 *	bGTK			- 1: the key is group key
 *	*pKey			- the key
 *
 *Return Value:
 *	None
 *
 *Note:
 *========================================================================
 */
void fnSetCipherKey(
	IN  PRTMP_ADAPTER   pAd,
	IN  INT             keyIdx,
	IN  UCHAR           CipherAlg,
	IN  BOOLEAN         bGTK,
	IN  UCHAR			*pKey,
	IN  INT             KeyLen,
	IN  struct wifi_dev *wdev)
{
	ASIC_SEC_INFO Info = {0};
	MAC_TABLE_ENTRY *pEntry = NULL;

	pEntry = GetAssociatedAPByWdev(pAd, wdev);

	if (!bGTK) {
		/* Set key material to Asic */
		os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
		/* NdisMoveMemory(pEntry->SecConfig.PTK, pKey, LEN_MAX_PTK); */
		Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
		Info.Direction = SEC_ASIC_KEY_BOTH;
		Info.Wcid = pEntry->wcid;
		Info.BssIndex = pEntry->func_tb_idx;
		Info.Cipher = pEntry->SecConfig.PairwiseCipher;
		Info.KeyIdx = pEntry->SecConfig.PairwiseKeyId;
		os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
		os_move_mem(Info.Key.Key, pKey, (LEN_TK + LEN_TK2));

		if (IS_CIPHER_TKIP(Info.Cipher))
			WPAInstallKey(pAd, &Info, TRUE, TRUE);
		else
			WPAInstallKey(pAd, &Info, FALSE, TRUE);
	} else {
		NdisMoveMemory(pEntry->SecConfig.GTK, pKey, MAX_LEN_GTK);
		pEntry->SecConfig.GroupKeyId = keyIdx;
		/* Set key material to Asic */
		os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
		Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
		Info.Direction = SEC_ASIC_KEY_RX;
		Info.Wcid = wdev->bss_info_argument.bmc_wlan_idx;
		Info.BssIndex = BSS0;
		Info.Cipher = pEntry->SecConfig.GroupCipher;
		Info.KeyIdx = pEntry->SecConfig.GroupKeyId;
		os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
		os_move_mem(Info.Key.Key, pEntry->SecConfig.GTK, (LEN_TK + LEN_TK2));
#ifdef DOT11W_PMF_SUPPORT

		if (KeyLen == MAX_LEN_GTK && IS_CIPHER_CCMP128(Info.Cipher)) { /* code for PMF */
			os_move_mem(Info.IGTK, pKey + LEN_TK, LEN_BIP128_IGTK);
			Info.IGTKKeyLen = LEN_BIP128_IGTK;
		}

#endif

		if (IS_CIPHER_TKIP(Info.Cipher))
			WPAInstallKey(pAd, &Info, TRUE, TRUE);
		else
			WPAInstallKey(pAd, &Info, FALSE, TRUE);
	}

}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWENCODEEXT.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT RtmpIoctl_rt_ioctl_siwencodeext(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data, struct wifi_dev *wdev)
{
	RT_CMD_STA_IOCTL_SECURITY *pIoctlSec = (RT_CMD_STA_IOCTL_SECURITY *)pData;
	int keyIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	STA_TR_ENTRY *tr_entry;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;

	pEntry = GetAssociatedAPByWdev(pAd, wdev);
	/* pEntry = &pAd->MacTab.Content[BSSID_WCID_TO_REMOVE]; */
	{
		/* Get Key Index and convet to our own defined key index */
		keyIdx = pIoctlSec->KeyIdx; /*(encoding->flags & IW_ENCODE_INDEX) - 1; */

		if ((keyIdx < 0) || (keyIdx >= NR_WEP_KEYS))
			return NDIS_STATUS_FAILURE;

		if (pIoctlSec->ext_flags & RT_CMD_STA_IOCTL_SECURTIY_EXT_SET_TX_KEY) {
			wdev->SecConfig.PairwiseKeyId = keyIdx;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DefaultKeyId = %d\n",
				wdev->SecConfig.PairwiseKeyId);
		}

		switch (pIoctlSec->Alg) {
		case RT_CMD_STA_IOCTL_SECURITY_ALG_NONE:
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IW_ENCODE_ALG_NONE\n");
			break;

		case RT_CMD_STA_IOCTL_SECURITY_ALG_WEP:
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IW_ENCODE_ALG_WEP - ext->key_len = %d, keyIdx = %d\n",
					 pIoctlSec->length, keyIdx);

			if (pIoctlSec->length == MAX_WEP_KEY_SIZE) {
				pAd->SharedKey[BSS0][keyIdx].KeyLen = MAX_WEP_KEY_SIZE;
				pAd->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_WEP128;
			} else if (pIoctlSec->length == MIN_WEP_KEY_SIZE) {
				pAd->SharedKey[BSS0][keyIdx].KeyLen = MIN_WEP_KEY_SIZE;
				pAd->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_WEP64;
			} else
				return NDIS_STATUS_FAILURE;

			NdisZeroMemory(pAd->SharedKey[BSS0][keyIdx].Key,  16);
			NdisMoveMemory(pAd->SharedKey[BSS0][keyIdx].Key, pIoctlSec->pData, pIoctlSec->length);

			if ((pStaCfg->GroupCipher == Ndis802_11GroupWEP40Enabled) ||
				(pStaCfg->GroupCipher == Ndis802_11GroupWEP104Enabled)) {
				/* Set Group key material to Asic */
				AsicAddSharedKeyEntry(pAd, BSS0, keyIdx, &pAd->SharedKey[BSS0][keyIdx]);
					STA_PORT_SECURED_BY_WDEV(pAd, wdev);
			}

			break;

		case RT_CMD_STA_IOCTL_SECURITY_ALG_TKIP:
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IW_ENCODE_ALG_TKIP - keyIdx = %d, ext->key_len = %d\n",
					 keyIdx, pIoctlSec->length);

			if (pIoctlSec->length == 32) {
				if IS_AKM_WPANONE(wdev->SecConfig.AKMMap)
				{
					RTMPZeroMemory(pStaCfg->PMK, LEN_PMK);
					RTMPMoveMemory(pStaCfg->PMK, pIoctlSec->pData, pIoctlSec->length);

					if (pIoctlSec->ext_flags & RT_CMD_STA_IOCTL_SECURTIY_EXT_SET_TX_KEY)
						fnSetCipherKey(pAd, keyIdx, CIPHER_TKIP, FALSE, pIoctlSec->pData, pIoctlSec->length, wdev);
					else
						fnSetCipherKey(pAd, keyIdx, CIPHER_TKIP, TRUE, pIoctlSec->pData, pIoctlSec->length, wdev);

						STA_PORT_SECURED_BY_WDEV(pAd, wdev);
				} else {
					if (pIoctlSec->ext_flags & RT_CMD_STA_IOCTL_SECURTIY_EXT_SET_TX_KEY) {
						fnSetCipherKey(pAd, keyIdx, CIPHER_TKIP, FALSE, pIoctlSec->pData, pIoctlSec->length, wdev);

						if (IS_AKM_WPA2(wdev->SecConfig.AKMMap) || IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap))
				    STA_PORT_SECURED_BY_WDEV(pAd, wdev);
					} else if (pIoctlSec->ext_flags & RT_CMD_STA_IOCTL_SECURTIY_EXT_GROUP_KEY) {
						fnSetCipherKey(pAd, keyIdx, CIPHER_TKIP, TRUE, pIoctlSec->pData, pIoctlSec->length, wdev);
						/* set 802.1x port control */
				STA_PORT_SECURED_BY_WDEV(pAd, wdev);
					}
				}

				/* open 802.1x port control and privacy filter*/
				tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
				tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
				pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				WifiSysUpdatePortSecur(pAd, pEntry, NULL);
			} else
				return NDIS_STATUS_FAILURE;

			break;

		case RT_CMD_STA_IOCTL_SECURITY_ALG_CCMP:
			if (IS_AKM_WPANONE(wdev->SecConfig.AKMMap)) {
				RTMPZeroMemory(pStaCfg->PMK, LEN_PMK);
				RTMPMoveMemory(pStaCfg->PMK, pIoctlSec->pData, pIoctlSec->length);

				if (pIoctlSec->ext_flags & RT_CMD_STA_IOCTL_SECURTIY_EXT_SET_TX_KEY)
					fnSetCipherKey(pAd, keyIdx, CIPHER_AES, FALSE, pIoctlSec->pData, pIoctlSec->length, wdev);
				else
					fnSetCipherKey(pAd, keyIdx, CIPHER_AES, TRUE, pIoctlSec->pData, pIoctlSec->length, wdev);

		    STA_PORT_SECURED_BY_WDEV(pAd, wdev);
			} else {
				if (pIoctlSec->ext_flags & RT_CMD_STA_IOCTL_SECURTIY_EXT_SET_TX_KEY) {
					fnSetCipherKey(pAd, keyIdx, CIPHER_AES, FALSE, pIoctlSec->pData, pIoctlSec->length, wdev);

					if (IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap))
				STA_PORT_SECURED_BY_WDEV(pAd, wdev);
				} else if (pIoctlSec->ext_flags & RT_CMD_STA_IOCTL_SECURTIY_EXT_GROUP_KEY) {
					fnSetCipherKey(pAd, keyIdx, CIPHER_AES, TRUE, pIoctlSec->pData, pIoctlSec->length, wdev);
					/* set 802.1x port control */
				STA_PORT_SECURED_BY_WDEV(pAd, wdev);
				}
			}

			/* open 802.1x port control and privacy filter*/
			tr_entry = &tr_ctl->tr_entry[pEntry->wcid];
			tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
			pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
			WifiSysUpdatePortSecur(pAd, pEntry, NULL);
			break;

		default:
			return NDIS_STATUS_FAILURE;
		}
	}

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWENCODEEXT.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT RtmpIoctl_rt_ioctl_giwencodeext(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	RT_CMD_STA_IOCTL_SECURITY *pIoctlSec = (RT_CMD_STA_IOCTL_SECURITY *)pData;
	UINT32 idx;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;

	idx = pIoctlSec->KeyIdx;

	if (idx) {
		if (idx < 1 || idx > 4) {
			pIoctlSec->Status = RTMP_IO_EINVAL;
			return NDIS_STATUS_FAILURE;
		}

		idx--;

		if (IS_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher) ||
			IS_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher)) {
			if (idx != wdev->SecConfig.PairwiseKeyId) {
				pIoctlSec->Status = 0;
				pIoctlSec->length = 0;
				return NDIS_STATUS_FAILURE;
			}
		}
	} else
		idx = wdev->SecConfig.PairwiseKeyId;

	pIoctlSec->KeyIdx = idx + 1;
	pIoctlSec->length = 0;

	if (IS_CIPHER_NONE(wdev->SecConfig.PairwiseCipher)) {
		pIoctlSec->Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_NONE;
		pIoctlSec->flags |= RT_CMD_STA_IOCTL_SECURITY_DISABLED;
	} else if (IS_CIPHER_WEP(wdev->SecConfig.PairwiseCipher)) {
		pIoctlSec->Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_WEP;

		if (wdev->SecConfig.WepKey[idx].KeyLen > pIoctlSec->MaxKeyLen) {
			pIoctlSec->Status = RTMP_IO_E2BIG;
			return NDIS_STATUS_FAILURE;
		} else {
			pIoctlSec->length = wdev->SecConfig.WepKey[idx].KeyLen;
			pIoctlSec->pData = (PCHAR) &(wdev->SecConfig.WepKey[idx].Key[0]);
		}
	} else if (IS_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher)) {
		pIoctlSec->Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_TKIP;

		if (pIoctlSec->MaxKeyLen < 32) {
			pIoctlSec->Status = RTMP_IO_E2BIG;
			return NDIS_STATUS_FAILURE;
		} else {
			pIoctlSec->length = 32;
			pIoctlSec->pData = (PCHAR)&wdev->SecConfig.PMK[0];
		}
	} else if (IS_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher)) {
		pIoctlSec->Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_CCMP;

		if (pIoctlSec->MaxKeyLen < 32) {
			pIoctlSec->Status = RTMP_IO_E2BIG;
			return NDIS_STATUS_FAILURE;
		} else {
			pIoctlSec->length = 32;
			pIoctlSec->pData = (PCHAR)&wdev->SecConfig.PMK[0];
		}
	} else {
		pIoctlSec->Status = RTMP_IO_EINVAL;
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWGENIE.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_siwgenie(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
#ifdef WPA_SUPPLICANT_SUPPORT
	ULONG length = (ULONG)Data;
	STA_ADMIN_CONFIG *pStaCfg;
	WPA_SUPPLICANT_INFO *supc_i;

	pStaCfg = adapter_to_ioctl_sta_cfg(pAd);
	supc_i = (pStaCfg) ? &pStaCfg->wpa_supplicant_info : NULL;

	if (supc_i && supc_i->WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "===> rt_ioctl_siwgenie\n");
		supc_i->bRSN_IE_FromWpaSupplicant = FALSE;

		if ((length > 0) && (pData == NULL))
			return NDIS_STATUS_FAILURE;

		if (length) {
			if (supc_i->pWpaAssocIe) {
				os_free_mem(supc_i->pWpaAssocIe);
				supc_i->pWpaAssocIe = NULL;
			}

			os_alloc_mem(NULL, (UCHAR **)&supc_i->pWpaAssocIe, length);

			if (supc_i->pWpaAssocIe) {
				supc_i->WpaAssocIeLen = length;
				NdisMoveMemory(supc_i->pWpaAssocIe, pData, supc_i->WpaAssocIeLen);
				supc_i->bRSN_IE_FromWpaSupplicant = TRUE;
			} else
				supc_i->WpaAssocIeLen = 0;
		}

		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_INVALID_DATA;

#else
	return NDIS_STATUS_SUCCESS;
#endif /* WPA_SUPPLICANT_SUPPORT */
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWGENIE.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_giwgenie(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	RT_CMD_STA_IOCTL_RSN_IE *IoctlRsnIe = (RT_CMD_STA_IOCTL_RSN_IE *)pData;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = &pStaCfg->wdev;

	if ((pStaCfg->RSNIE_Len == 0)
		|| IS_AKM_OPEN(wdev->SecConfig.AKMMap)
		|| IS_AKM_SHARED(wdev->SecConfig.AKMMap)
		|| IS_AKM_AUTOSWITCH(wdev->SecConfig.AKMMap)) {
		IoctlRsnIe->length = 0;
		return NDIS_STATUS_SUCCESS;
	}

#ifdef WPA_SUPPLICANT_SUPPORT

	/*
	 *	Can not use SIOCSIWGENIE definition, it is used in wireless.h
	 *	We will not see the definition in MODULE.
	 *	The definition can be saw in UTIL and NETIF.
	 */
	/* #ifdef SIOCSIWGENIE */
	if ((pStaCfg->wpa_supplicant_info.WpaSupplicantUP & 0x7F) == WPA_SUPPLICANT_ENABLE &&
		(pStaCfg->wpa_supplicant_info.WpaAssocIeLen > 0)) {
		if (IoctlRsnIe->length < pStaCfg->wpa_supplicant_info.WpaAssocIeLen)
			return NDIS_STATUS_FAILURE;

		IoctlRsnIe->length = pStaCfg->wpa_supplicant_info.WpaAssocIeLen;
		memcpy(IoctlRsnIe->pRsnIe, pStaCfg->wpa_supplicant_info.pWpaAssocIe, pStaCfg->wpa_supplicant_info.WpaAssocIeLen);
	} else
		/* #endif */ /* SIOCSIWGENIE */
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
	{
		UCHAR RSNIe = IE_WPA;

		if (IoctlRsnIe->length < (pStaCfg->RSNIE_Len + 2)) /* ID, Len */
			return NDIS_STATUS_FAILURE;

		IoctlRsnIe->length = pStaCfg->RSNIE_Len + 2;

		if (IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap)  ||
			IS_AKM_WPA2(wdev->SecConfig.AKMMap))
			RSNIe = IE_RSN;

		IoctlRsnIe->pRsnIe[0] = (char)RSNIe;
		IoctlRsnIe->pRsnIe[1] = pStaCfg->RSNIE_Len;
		memcpy(IoctlRsnIe->pRsnIe + 2, &pStaCfg->RSN_IE[0], pStaCfg->RSNIE_Len);
	}

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWPMKSA.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_siwpmksa(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	RT_CMD_STA_IOCTL_PMA_SA *pIoctlPmaSa = (RT_CMD_STA_IOCTL_PMA_SA *)pData;
	INT	CachedIdx = 0, idx = 0;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
#ifdef SUPP_SAE_SUPPORT
	struct wifi_dev *wdev = &pStaCfg->wdev;
	struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;
	if (pSecConfig && IS_AKM_SAE_SHA256(pSecConfig->AKMMap))
		return NDIS_STATUS_SUCCESS;
#endif
	switch (pIoctlPmaSa->Cmd) {
	case RT_CMD_STA_IOCTL_PMA_SA_FLUSH:
		NdisZeroMemory(pStaCfg->SavedPMK, sizeof(BSSID_INFO)*PMKID_NO);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "rt_ioctl_siwpmksa - IW_PMKSA_FLUSH\n");
		break;

	case RT_CMD_STA_IOCTL_PMA_SA_REMOVE:
		for (CachedIdx = 0; CachedIdx < pStaCfg->SavedPMKNum; CachedIdx++) {
			/* compare the BSSID */
			if (NdisEqualMemory(pIoctlPmaSa->pBssid, pStaCfg->SavedPMK[CachedIdx].BSSID, MAC_ADDR_LEN)) {
				NdisZeroMemory(pStaCfg->SavedPMK[CachedIdx].BSSID, MAC_ADDR_LEN);
				NdisZeroMemory(pStaCfg->SavedPMK[CachedIdx].PMKID, 16);

				for (idx = CachedIdx; idx < (pStaCfg->SavedPMKNum - 1); idx++) {
					NdisMoveMemory(&pStaCfg->SavedPMK[idx].BSSID[0], &pStaCfg->SavedPMK[idx + 1].BSSID[0], MAC_ADDR_LEN);
					NdisMoveMemory(&pStaCfg->SavedPMK[idx].PMKID[0], &pStaCfg->SavedPMK[idx + 1].PMKID[0], 16);
				}

				pStaCfg->SavedPMKNum--;
				break;
			}
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "rt_ioctl_siwpmksa - IW_PMKSA_REMOVE\n");
		break;

	case RT_CMD_STA_IOCTL_PMA_SA_ADD:
		for (CachedIdx = 0; CachedIdx < pStaCfg->SavedPMKNum; CachedIdx++) {
			/* compare the BSSID */
			if (NdisEqualMemory(pIoctlPmaSa->pBssid, pStaCfg->SavedPMK[CachedIdx].BSSID, MAC_ADDR_LEN))
				break;
		}

		/* Found, replace it */
		if (CachedIdx < PMKID_NO) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Update PMKID, idx = %d\n", CachedIdx);
			NdisMoveMemory(&pStaCfg->SavedPMK[CachedIdx].BSSID[0], pIoctlPmaSa->pBssid, MAC_ADDR_LEN);
			NdisMoveMemory(&pStaCfg->SavedPMK[CachedIdx].PMKID[0], pIoctlPmaSa->pPmkid, 16);
			pStaCfg->SavedPMKNum++;
		}
		/* Not found, replace the last one */
		else {
			/* Randomly replace one */
			CachedIdx = (pIoctlPmaSa->pBssid[5] % PMKID_NO);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Update PMKID, idx = %d\n", CachedIdx);
			NdisMoveMemory(&pStaCfg->SavedPMK[CachedIdx].BSSID[0], pIoctlPmaSa->pBssid, MAC_ADDR_LEN);
			NdisMoveMemory(&pStaCfg->SavedPMK[CachedIdx].PMKID[0], pIoctlPmaSa->pPmkid, 16);
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "rt_ioctl_siwpmksa - IW_PMKSA_ADD\n");
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "rt_ioctl_siwpmksa - Unknow Command!!\n");
		break;
	}

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWRATE.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT RtmpIoctl_rt_ioctl_siwrate(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data, struct wifi_dev *wdev)
{
	RT_CMD_RATE_SET *pCmdRate = (RT_CMD_RATE_SET *)pData;
	UINT32 rate = pCmdRate->Rate;
	UINT32 fixed = pCmdRate->Fixed;
	MAC_TABLE_ENTRY *pEntry = NULL;
	/* pEntry = GetAssociatedAPByWdev(pAd, wdev); */
	pEntry = &pAd->MacTab.Content[BSSID_WCID_TO_REMOVE];

	/*
	 *	rate = -1 => auto rate
	 *rate = X, => fixed = 1 and rate fixed at X
	 */
	if (rate == -1) {
		/*Auto Rate */
		wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
		wdev->bAutoTxRateSwitch = TRUE;

		if ((!WMODE_CAP_N(wdev->PhyMode)) ||
			(pEntry->HTPhyMode.field.MODE <= MODE_OFDM))
			RTMPSetDesiredRates(pAd, wdev, -1);

#ifdef DOT11_N_SUPPORT
		SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
	} else {
		if (fixed) {
			wdev->bAutoTxRateSwitch = FALSE;

			if ((!WMODE_CAP_N(wdev->PhyMode)) ||
				(pEntry->HTPhyMode.field.MODE <= MODE_OFDM))
				RTMPSetDesiredRates(pAd, wdev, rate);
			else {
				wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
#ifdef DOT11_N_SUPPORT
				SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "rt_ioctl_siwrate::(HtMcs=%d)\n", wdev->DesiredTransmitSetting.field.MCS);
		} else {
			/* TODO: rate = X, fixed = 0 => (rates <= X) */
			return NDIS_STATUS_FAILURE;
		}
	}

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWRATE.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_giwrate(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data, struct wifi_dev *wdev)
{
	int rate_index = 0, rate_count = 0;
	HTTRANSMIT_SETTING ht_setting;
	MAC_TABLE_ENTRY *pEntry = NULL;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
	/* pEntry = GetAssociatedAPByWdev(pAd, wdev); */
	pEntry = &pAd->MacTab.Content[BSSID_WCID_TO_REMOVE];
	rate_count = RT_RateSize / sizeof(INT32);

	if ((wdev->bAutoTxRateSwitch == FALSE) &&
		(INFRA_ON(pStaCfg)) &&
		((!WMODE_CAP_N(wdev->PhyMode)) || (pEntry->HTPhyMode.field.MODE <= MODE_OFDM)))
		ht_setting.word = wdev->HTPhyMode.word;
	else
		ht_setting.word = pEntry->HTPhyMode.word;

#ifdef DOT11_N_SUPPORT

	if (ht_setting.field.MODE >= MODE_HTMIX) {
		/*	rate_index = 12 + ((UCHAR)ht_setting.field.BW *16) + ((UCHAR)ht_setting.field.ShortGI *32) + ((UCHAR)ht_setting.field.MCS); */
		rate_index = 12 + ((UCHAR)ht_setting.field.BW * 24) + ((UCHAR)ht_setting.field.ShortGI * 48) + ((
						 UCHAR)ht_setting.field.MCS);
	} else
#endif /* DOT11_N_SUPPORT */
		if (ht_setting.field.MODE == MODE_OFDM)
			rate_index = (UCHAR)(ht_setting.field.MCS) + 4;
		else if (ht_setting.field.MODE == MODE_CCK)
			rate_index = (UCHAR)(ht_setting.field.MCS);

	if (rate_index >= rate_count)
		rate_index = rate_count - 1;

	if (rate_index < 0)
		rate_index = 0;

	*(ULONG *)pData = ralinkrate[rate_index] * 500000;
	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIFHWADDR.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_gifhwaddr(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT32 staidx = 0;

	if (pObj->ioctl_if < 0 || pObj->ioctl_if >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pObj->ioctl_if is invalid value\n");
		return NDIS_STATUS_INVALID_DATA;
	}
	staidx = pObj->ioctl_if;
	memcpy(pData, pAd->StaCfg[staidx].wdev.if_addr, ETH_ALEN);
	return NDIS_STATUS_SUCCESS;
}

/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_SIOCSIWPRIVRSSI.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_rssi(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	(*(CHAR *)pData) =  pStaCfg->RssiSample.AvgRssi[0];
	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_IW_SET_PARAM_PRE.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_ioctl_setparam(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef P2P_SUPPORT
		if (Data == INT_P2P) {
			pObj->ioctl_if_type = INT_P2P;
			pObj->ioctl_if = 0;
		} else
#endif /* P2P_SUPPORT */
		{
			pObj->ioctl_if_type = INT_MAIN;
			pObj->ioctl_if = MAIN_MBSSID;
		}

	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_IW_SET_WSC_U32_ITEM.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_private_set_wsc_u32_item(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
#ifdef WSC_STA_SUPPORT
	RT_CMD_STA_IOCTL_WSC_U32_ITEM *pIoctlWscU32 = (RT_CMD_STA_IOCTL_WSC_U32_ITEM *)pData;
	UINT32 subcmd = *(pIoctlWscU32->pUWrq);
	PWSC_PROFILE    pWscProfile = NULL;
	UINT32 value = 0;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	WSC_CTRL *wsc_ctrl;

	wsc_ctrl = &pStaCfg->wdev.WscControl;
	pWscProfile = &wsc_ctrl->WscProfile;

	switch (subcmd) {
	case WSC_CREDENTIAL_COUNT:
		value = *(pIoctlWscU32->pUWrq + 1);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_CREDENTIAL_COUNT, value = %d\n", value);

		if (value <= 8)
			pWscProfile->ProfileCnt = value;
		else
			pIoctlWscU32->Status = RTMP_IO_EINVAL;

		break;

	case WSC_SET_DRIVER_CONNECT_BY_CREDENTIAL_IDX:
		value = *(pIoctlWscU32->pUWrq + 1);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_SET_DRIVER_CONNECT_BY_CREDENTIAL_IDX, value = %d\n", value);

		if ((value <= 7) &&
			(value < pWscProfile->ProfileCnt)) {
			WscWriteConfToPortCfg(pAd, wsc_ctrl, &pWscProfile->Profile[value], TRUE);
			pStaCfg->MlmeAux.CurrReqIsFromNdis = TRUE;
			LinkDown(pAd, TRUE, &pStaCfg->wdev, NULL);
		} else
			pIoctlWscU32->Status = RTMP_IO_EINVAL;

		break;

	case WSC_SET_DRIVER_AUTO_CONNECT:
		value = *(pIoctlWscU32->pUWrq + 1);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_SET_DRIVER_AUTO_CONNECT, value = %d\n", value);

		if ((value == 0x00) ||
			(value == 0x01) ||
			(value == 0x02))
			wsc_ctrl->WscDriverAutoConnect = value;
		else
			pIoctlWscU32->Status = RTMP_IO_EINVAL;

		break;

	case WSC_SET_CONF_MODE:
		value = *(pIoctlWscU32->pUWrq + 1);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_SET_CONF_MODE, value = %d\n", value);

		if (value == 2)
			value = 4;

		switch (value) {
		case WSC_DISABLE:
			Set_WscConfMode_Proc(pAd, "0");
			break;

		case WSC_ENROLLEE:
			Set_WscConfMode_Proc(pAd, "1");
			break;

		case WSC_REGISTRAR:
			Set_WscConfMode_Proc(pAd, "2");
			break;

		default:
			pIoctlWscU32->Status = RTMP_IO_EINVAL;
			break;
		}

		break;

	case WSC_SET_MODE:
		value = *(pIoctlWscU32->pUWrq + 1);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_SET_MODE, value = %d\n", value);

		switch (value) {
		case WSC_PIN_MODE:
			if (Set_WscMode_Proc(pAd, "1") == FALSE)
				pIoctlWscU32->Status = RTMP_IO_EINVAL;

			break;

		case WSC_PBC_MODE:
			if (Set_WscMode_Proc(pAd, "2") == FALSE)
				pIoctlWscU32->Status = RTMP_IO_EINVAL;

			break;

		case WSC_SMPBC_MODE:
			if (Set_WscMode_Proc(pAd, "3") == FALSE)
				pIoctlWscU32->Status = RTMP_IO_EINVAL;

			break;

		default:
			pIoctlWscU32->Status = RTMP_IO_EINVAL;
			break;
		}

		break;

	case WSC_START:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_START\n");
		Set_WscGetConf_Proc(pAd, "1");
		break;

	case WSC_STOP:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_STOP\n");

		/* Disassociate the link if WPS is working. */
		if (INFRA_ON(pStaCfg) &&
			(wsc_ctrl->bWscTrigger == TRUE) &&
			(wsc_ctrl->WscConfMode != WSC_DISABLE)) {
			cntl_disconnect_request(&pStaCfg->wdev,
									CNTL_DISASSOC,
									pStaCfg->Bssid,
									(USHORT)REASON_DISASSOC_STA_LEAVING);
		}

#ifdef IWSC_SUPPORT

		if (pStaCfg->BssType == BSS_ADHOC) {
			wsc_ctrl->WscConfMode = WSC_DISABLE;
			wsc_ctrl->WscState = WSC_STATE_INIT;
			wsc_ctrl->WscStatus = STATUS_WSC_IDLE;

			if (wsc_ctrl->bWscTrigger)
				IWSC_Stop(pAd, TRUE);
			else
				IWSC_Stop(pAd, FALSE);
		} else
#endif /* IWSC_SUPPORT */
		{
			/* Turn off WSC state matchine */
			WscStop(pAd,
#ifdef CONFIG_AP_SUPPORT
					FALSE,
#endif /* CONFIG_AP_SUPPORT */
					wsc_ctrl);
			wsc_ctrl->WscConfMode = WSC_DISABLE;
			BssTableDeleteEntry(&pStaCfg->MlmeAux.SsidBssTab, pStaCfg->MlmeAux.Bssid, pStaCfg->MlmeAux.Channel);
		}

		break;

	case WSC_GEN_PIN_CODE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_GEN_PIN_CODE\n");
		Set_WscGenPinCode_Proc(pAd, "1");
		break;

	case WSC_AP_BAND:
		value = *(pIoctlWscU32->pUWrq + 1);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_PBC_BAND, value = %d\n", value);

		if (value < PREFERRED_WPS_AP_PHY_TYPE_MAXIMUM)
			wsc_ctrl->WpsApBand = value;

		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Unknow subcmd = %d, value = %d\n", subcmd, value);
		break;
	}

#endif /* WSC_STA_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_IW_SET_WSC_STR_ITEM.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT
RtmpIoctl_rt_private_set_wsc_string_item(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
#ifdef WSC_STA_SUPPORT
	RT_CMD_STA_IOCTL_WSC_STR_ITEM *pIoctlWscStr = (RT_CMD_STA_IOCTL_WSC_STR_ITEM *)pData;
	/*    int  Status=0; */
	UINT32 subcmd = pIoctlWscStr->Subcmd;
	UINT32 tmpProfileIndex = (UINT32)(pIoctlWscStr->pData[0] - 0x30);
	UINT32 dataLen;
	PWSC_PROFILE    pWscProfile = NULL;
	USHORT  tmpAuth = 0, tmpEncr = 0;
	char *extra = (char *)pIoctlWscStr->pData;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];

	pWscProfile = &pStaCfg->wdev.WscControl.WscProfile;

	if ((subcmd != WSC_SET_SSID) &&
		(subcmd != WSC_SET_PIN) &&
		(subcmd != WSC_SET_BSSID) &&
		(tmpProfileIndex > 7)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "subcmd = %d, tmpProfileIndex = %d\n", subcmd,
				 tmpProfileIndex);
		pIoctlWscStr->Status = RTMP_IO_EINVAL;
		return NDIS_STATUS_SUCCESS;
	}

	if ((subcmd != WSC_SET_SSID) &&
		(subcmd != WSC_SET_PIN) &&
		(subcmd != WSC_SET_BSSID))
		/* extra: "1 input_string", dwrq->length includes '\0'. 3 is size of [index, blank and '\0'] */
		dataLen = pIoctlWscStr->length - 3;
	else
		dataLen = pIoctlWscStr->length;

	switch (subcmd) {
	case WSC_CREDENTIAL_SSID:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_CREDENTIAL_SSID(%s)\n", extra + 2);

		if (dataLen == (NDIS_802_11_LENGTH_SSID + 1))
			dataLen = NDIS_802_11_LENGTH_SSID;

		if (dataLen > 0 && dataLen <= NDIS_802_11_LENGTH_SSID) {
			pWscProfile->Profile[tmpProfileIndex].SSID.SsidLength = dataLen;
			NdisZeroMemory(pWscProfile->Profile[tmpProfileIndex].SSID.Ssid, NDIS_802_11_LENGTH_SSID);
			NdisMoveMemory(pWscProfile->Profile[tmpProfileIndex].SSID.Ssid, extra + 2, dataLen);
		} else
			pIoctlWscStr->Status = RTMP_IO_E2BIG;

		break;

	case WSC_CREDENTIAL_AUTH_MODE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_CREDENTIAL_AUTH_MODE(%s)\n", extra + 2);
		tmpAuth = WscGetAuthTypeFromStr(extra + 2);

		if (tmpAuth != 0)
			pWscProfile->Profile[tmpProfileIndex].AuthType = tmpAuth;
		else
			pIoctlWscStr->Status = RTMP_IO_EINVAL;

		break;

	case WSC_CREDENTIAL_ENCR_TYPE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_CREDENTIAL_ENCR_TYPE(%s)\n", extra + 2);
		tmpEncr = WscGetEncrypTypeFromStr(extra + 2);

		if (tmpEncr != 0)
			pWscProfile->Profile[tmpProfileIndex].EncrType = tmpEncr;
		else
			pIoctlWscStr->Status = RTMP_IO_EINVAL;

		break;

	case WSC_CREDENTIAL_KEY_INDEX:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_CREDENTIAL_KEY_INDEX(%s)\n", extra + 2);

		if (*(extra + 2) >= 0x31 && *(extra + 2) <= 0x34)
			pWscProfile->Profile[tmpProfileIndex].KeyIndex = (UCHAR) *(extra + 2) - 0x30;
		else
			pIoctlWscStr->Status = RTMP_IO_EINVAL;

		break;

	case WSC_CREDENTIAL_KEY:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_CREDENTIAL_KEY(%s)\n", extra + 2);

		if ((dataLen >= 8 && dataLen <= 64) ||
			(dataLen == 5 || dataLen == 10 || dataLen == 13 || dataLen == 26)) {
			pWscProfile->Profile[tmpProfileIndex].KeyLength = dataLen;
			NdisZeroMemory(pWscProfile->Profile[tmpProfileIndex].Key, 64);
			NdisMoveMemory(pWscProfile->Profile[tmpProfileIndex].Key, extra + 2, dataLen);
		} else
			pIoctlWscStr->Status = RTMP_IO_EINVAL;

		break;

	case WSC_CREDENTIAL_MAC:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_CREDENTIAL_MAC(%s)\n", extra + 2);
		{
			INT sscanf_rv = 0;
			UINT tmp_val[6] = {0};

			sscanf_rv = sscanf(extra + 2, "%02x:%02x:%02x:%02x:%02x:%02x",
							   &tmp_val[0],
							   &tmp_val[1],
							   &tmp_val[2],
							   &tmp_val[3],
							   &tmp_val[4],
							   &tmp_val[5]);

			if (sscanf_rv == 6) {
				int ii;

				NdisZeroMemory(pWscProfile->Profile[tmpProfileIndex].MacAddr, 6);

				for (ii = 0; ii < 6; ii++)
					pWscProfile->Profile[tmpProfileIndex].MacAddr[ii] = (UCHAR)tmp_val[ii];
			} else
				pIoctlWscStr->Status = RTMP_IO_EINVAL;
		}
		break;

	case WSC_SET_SSID:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_SET_SSID(%s)\n", extra);

		if (dataLen == (NDIS_802_11_LENGTH_SSID + 1))
			dataLen = NDIS_802_11_LENGTH_SSID;

		if (dataLen > 0 && dataLen <= NDIS_802_11_LENGTH_SSID)
			Set_WscSsid_Proc(pAd, (RTMP_STRING *) extra);
		else
			pIoctlWscStr->Status = RTMP_IO_E2BIG;

		break;

	case WSC_SET_PIN:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_SET_PIN, value = (%s)\n", extra);

		if (dataLen > 0) {
			if (Set_WscPinCode_Proc(pAd, extra) == FALSE)
				pIoctlWscStr->Status = RTMP_IO_EINVAL;
		} else
			pIoctlWscStr->Status = RTMP_IO_EINVAL;

		break;

	case WSC_SET_BSSID:
		if (dataLen > 0) {
			if (Set_WscBssid_Proc(pAd, (RTMP_STRING *) extra) == FALSE)
				pIoctlWscStr->Status = RTMP_IO_EINVAL;
		} else
			pIoctlWscStr->Status = RTMP_IO_EINVAL;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC_SET_BSSID\n");
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Unknow subcmd = %d\n", subcmd);
		break;
	}

#endif /* WSC_STA_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}


/*
 *========================================================================
 *Routine Description:
 *	Handler for CMD_RTPRIV_IOCTL_STA_IW_GET_STATISTICS.
 *
 *Arguments:
 *	pAd				- WLAN control block pointer
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
static VOID ShowAmpduCounter(RTMP_ADAPTER *pAd, UCHAR BandIdx, RTMP_STRING *msg)
{
	COUNTER_802_11 *WlanCounter = &pAd->WlanCounters[BandIdx];
	ULONG per;
	INT ret;
	UINT32 msg_left;

	msg_left = IW_PRIV_SIZE_MASK - strlen(msg);
	ret = snprintf(msg + strlen(msg), msg_left, "BandIdx: %d\n", BandIdx);
	if (os_snprintf_error(msg_left, ret))
		goto err_out;
	msg_left = IW_PRIV_SIZE_MASK - strlen(msg);
	ret = snprintf(msg + strlen(msg), msg_left, "TX AGG Range 1 (1) 			 = %ld\n", (LONG)(WlanCounter->TxAggRange1Count.u.LowPart));
	if (os_snprintf_error(msg_left, ret))
		goto err_out;
	msg_left = IW_PRIV_SIZE_MASK - strlen(msg);
	ret = snprintf(msg + strlen(msg), msg_left, "TX AGG Range 2 (2~5)			 = %ld\n", (LONG)(WlanCounter->TxAggRange2Count.u.LowPart));
	if (os_snprintf_error(msg_left, ret))
		goto err_out;
	msg_left = IW_PRIV_SIZE_MASK - strlen(msg);
	ret = snprintf(msg + strlen(msg), msg_left, "TX AGG Range 3 (6~15)			 = %ld\n", (LONG)(WlanCounter->TxAggRange3Count.u.LowPart));
	if (os_snprintf_error(msg_left, ret))
		goto err_out;
	msg_left = IW_PRIV_SIZE_MASK - strlen(msg);
	ret = snprintf(msg + strlen(msg), msg_left, "TX AGG Range 4 (>15)			 = %ld\n", (LONG)(WlanCounter->TxAggRange4Count.u.LowPart));
	if (os_snprintf_error(msg_left, ret))
		goto err_out;
	{
		ULONG mpduTXCount;

		mpduTXCount = WlanCounter->AmpduSuccessCount.u.LowPart;
		msg_left = IW_PRIV_SIZE_MASK - strlen(msg);
		ret = snprintf(msg + strlen(msg), msg_left, "AMPDU Tx success				 = %ld\n", mpduTXCount);
		if (os_snprintf_error(msg_left, ret))
			goto err_out;
		per = mpduTXCount == 0 ? 0 : 1000 * (WlanCounter->AmpduFailCount.u.LowPart) / (WlanCounter->AmpduFailCount.u.LowPart +
				mpduTXCount);
		msg_left = IW_PRIV_SIZE_MASK - strlen(msg);
		ret = snprintf(msg + strlen(msg), msg_left, "AMPDU Tx fail count			 = %ld, PER=%ld.%1ld%%\n",
				(ULONG)WlanCounter->AmpduFailCount.u.LowPart,
				per / 10, per % 10);
		if (os_snprintf_error(msg_left, ret))
			goto err_out;
	}
	return;

err_out:
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ShowAmpduCounter snprintf error!\n");
	return;
}


INT
RtmpIoctl_rt_private_get_statistics(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	INT ret;
	UINT32 extra_left;
	char *extra = (char *)pData;
	ULONG txCount = 0;
#ifdef ENHANCED_STAT_DISPLAY
	ULONG per, plr;
#endif
#ifdef WSC_STA_SUPPORT
	UINT32 MaxSize = (UINT32)Data;
	WSC_CTRL *wsc_ctrl;
#endif /* WSC_STA_SUPPORT */
	UINT32 i;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand = BAND0;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif

	if (wdev != NULL)
		ucBand = HcGetBandByWdev(wdev);

	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra, extra_left, "\n\n");
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

	if (cap->fgRateAdaptFWOffload == TRUE) {
		EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
		HTTRANSMIT_SETTING LastTxRate;

		os_zero_mem(&rTxStatResult, sizeof(EXT_EVENT_TX_STATISTIC_RESULT_T));
		MtCmdGetTxStatistic(pAd, GET_TX_STAT_TOTAL_TX_CNT | GET_TX_STAT_LAST_TX_RATE, ucBand, 0, &rTxStatResult);
		pAd->WlanCounters[0].TransmittedFragmentCount.u.LowPart += (rTxStatResult.u4TotalTxCount -
				rTxStatResult.u4TotalTxFailCount);
		pAd->WlanCounters[0].FailedCount.u.LowPart += rTxStatResult.u4TotalTxFailCount;
		LastTxRate.field.MODE = rTxStatResult.rLastTxRate.MODE;
		LastTxRate.field.BW = rTxStatResult.rLastTxRate.BW;
		LastTxRate.field.ldpc = rTxStatResult.rLastTxRate.ldpc ? 1 : 0;
		LastTxRate.field.ShortGI = rTxStatResult.rLastTxRate.ShortGI ? 1 : 0;
		LastTxRate.field.STBC = rTxStatResult.rLastTxRate.STBC;

		if (LastTxRate.field.MODE >= MODE_VHT)
			LastTxRate.field.MCS = (((rTxStatResult.rLastTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rLastTxRate.MCS;
		else
			LastTxRate.field.MCS = rTxStatResult.rLastTxRate.MCS;

		pAd->LastTxRate = (USHORT)(LastTxRate.word);
	}

#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		txCount = pAd->ATECtrl.tx_done_cnt;
	else
#endif /* CONFIG_ATE */
		txCount = (ULONG)pAd->WlanCounters[0].TransmittedFragmentCount.u.LowPart;

	RTMP_GET_TEMPERATURE(pAd, ucBand, &pAd->temperature);
	/* Sanity check for calculation of sucessful count */
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "CurrentTemperature			   = %d\n", pAd->temperature);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "Tx success					   = %lu\n", txCount);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;

#ifdef ENHANCED_STAT_DISPLAY

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		per = txCount == 0 ? 0 : 1000 * (pAd->WlanCounters[0].FailedCount.u.LowPart) /
			  (pAd->WlanCounters[0].FailedCount.u.LowPart + txCount);
		extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
		ret = snprintf(extra + strlen(extra), extra_left, "Tx fail count				   = %ld, PER=%ld.%1ld%%\n",
				(ULONG)pAd->WlanCounters[0].FailedCount.u.LowPart,
				per / 10, per % 10);
		if (os_snprintf_error(extra_left, ret))
			goto err_out;
	} else {
		per = txCount == 0 ? 0 : 1000 * (pAd->WlanCounters[0].RetryCount.u.LowPart +
										 pAd->WlanCounters[0].FailedCount.u.LowPart) / (pAd->WlanCounters[0].RetryCount.u.LowPart +
												 pAd->WlanCounters[0].FailedCount.u.LowPart + txCount);
		extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
		ret = snprintf(extra + strlen(extra), extra_left, "Tx retry count				   = %lu, PER=%ld.%1ld%%\n",
				(ULONG)pAd->WlanCounters[0].RetryCount.u.LowPart,
				per / 10, per % 10);
		if (os_snprintf_error(extra_left, ret))
			goto err_out;
		plr = txCount == 0 ? 0 : 10000 * pAd->WlanCounters[0].FailedCount.u.LowPart /
			  (pAd->WlanCounters[0].FailedCount.u.LowPart + txCount);
		extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
		ret = snprintf(extra + strlen(extra), extra_left, "Tx fail to Rcv ACK after retry  = %lu, PLR=%ld.%02ld%%\n",
				(ULONG)pAd->WlanCounters[0].FailedCount.u.LowPart, plr / 100, plr % 100);
		if (os_snprintf_error(extra_left, ret))
			goto err_out;
	}

#else
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "Tx retry count			  = %lu\n", (ULONG)pAd->WlanCounters[0].RetryCount.u.LowPart);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "Tx fail to Rcv ACK after retry  = %lu\n",
			(ULONG)pAd->WlanCounters[0].FailedCount.u.LowPart);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "RTS Success Rcv CTS			   = %lu\n",
			(ULONG)pAd->WlanCounters[0].RTSSuccessCount.u.LowPart);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "RTS Fail Rcv CTS 			   = %lu\n",
			(ULONG)pAd->WlanCounters[0].RTSFailureCount.u.LowPart);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
#endif /* ENHANCED_STAT_DISPLAY */
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "Rx success					   = %lu\n",
			(ULONG)pAd->WlanCounters[0].ReceivedFragmentCount.QuadPart);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
#ifdef ENHANCED_STAT_DISPLAY
	per = pAd->WlanCounters[0].ReceivedFragmentCount.u.LowPart == 0 ? 0 : 1000 *
		  (pAd->WlanCounters[0].FCSErrorCount.u.LowPart) / (pAd->WlanCounters[0].FCSErrorCount.u.LowPart +
				  pAd->WlanCounters[0].ReceivedFragmentCount.u.LowPart);
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "Rx with CRC					   = %ld, PER=%ld.%1ld%%\n",
			(ULONG)pAd->WlanCounters[0].FCSErrorCount.u.LowPart, per / 10, per % 10);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "Rx drop due to out of resource  = %lu\n", (ULONG)pAd->Counters8023.RxNoBuffer);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "Rx duplicate frame			   = %lu\n",
			(ULONG)pAd->WlanCounters[0].FrameDuplicateCount.u.LowPart);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "False CCA					   = %lu\n", (ULONG)pAd->RalinkCounters.FalseCCACnt);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
#else
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "Rx with CRC					   = %lu\n",
			(ULONG)pAd->WlanCounters[0].FCSErrorCount.u.LowPart);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "Rx drop due to out of resource  = %lu\n", (ULONG)pAd->Counters8023.RxNoBuffer);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "Rx duplicate frame			   = %lu\n",
			(ULONG)pAd->WlanCounters[0].FrameDuplicateCount.u.LowPart);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "False CCA (one second)		   = %lu\n", (ULONG)pAd->RalinkCounters.OneSecFalseCCACnt);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
#endif /* ENHANCED_STAT_DISPLAY */
#ifdef CONFIG_ATE

	if (ATE_ON(pAd)) {
		/* struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl); */
		/* sprintf(extra+strlen(extra), "ATE Rx success                      = %lu\n", (ULONG)ATECtrl->RxTotalCnt); */
	} else
#endif /* CONFIG_ATE */
	{
#ifdef ENHANCED_STAT_DISPLAY
		extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
		ret = snprintf(extra + strlen(extra), extra_left, "RSSI 						   = %ld %ld %ld %ld\n",
				(LONG)(pStaCfg->RssiSample.LastRssi[0] - pAd->BbpRssiToDbmDelta),
				(LONG)(pStaCfg->RssiSample.LastRssi[1] - pAd->BbpRssiToDbmDelta),
				(LONG)(pStaCfg->RssiSample.LastRssi[2] - pAd->BbpRssiToDbmDelta),
				(LONG)(pStaCfg->RssiSample.LastRssi[3] - pAd->BbpRssiToDbmDelta));
		if (os_snprintf_error(extra_left, ret))
			goto err_out;

		/* Display Last Rx Rate and BF SNR of first Associated entry in MAC table */
		if (pAd->MacTab.Size > 0) {
			static char *phyMode[5] = {"CCK", "OFDM", "MM", "GF", "VHT"};
			int i;

			for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
				PMAC_TABLE_ENTRY pEntry = &(pAd->MacTab.Content[i]);

				if (IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
					UINT32 lastRxRate = pEntry->LastRxRate;
					UINT32 lastTxRate = pEntry->LastTxRate;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

					if (cap->fgRateAdaptFWOffload == TRUE) {
						if (pEntry->bAutoTxRateSwitch == TRUE) {
							EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
							HTTRANSMIT_SETTING LastTxRate;

							NdisZeroMemory(&rTxStatResult, sizeof(EXT_EVENT_TX_STATISTIC_RESULT_T));
							MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
							LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
							LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
							LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
							LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
							LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

							if (LastTxRate.field.MODE >= MODE_VHT)
								LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
							else
								LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

							lastTxRate = (UINT32)(LastTxRate.word);
						}
					}

#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
#ifdef MT_MAC

					if (IS_HIF_TYPE(pAd, HIF_MT))  {
						StatRateToString(pAd, extra, 0, lastTxRate);
						StatRateToString(pAd, extra, 1, lastRxRate);
					} else
#endif /* MT_MAC */
					{
						extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
						ret = snprintf(extra + strlen(extra), extra_left, "Last TX Rate 				   = MCS%d, %2dM, %cGI, %s%s\n",
								lastTxRate & 0x7F,	((lastTxRate >> 7) & 0x1) ? 40 : 20,
								((lastTxRate >> 8) & 0x1) ? 'S' : 'L',
								phyMode[(lastTxRate >> 14) & 0x3],
								((lastTxRate >> 9) & 0x3) ? ", STBC" : " ");
						if (os_snprintf_error(extra_left, ret))
							goto err_out;
						extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
						ret = snprintf(extra + strlen(extra), extra_left, "Last RX Rate 				   = MCS %d, %2dM, %cGI, %s%s\n",
								lastRxRate & 0x7F,	((lastRxRate >> 7) & 0x1) ? 40 : 20,
								((lastRxRate >> 8) & 0x1) ? 'S' : 'L',
								phyMode[(lastRxRate >> 14) & 0x3],
								((lastRxRate >> 9) & 0x3) ? ", STBC" : " ");
						if (os_snprintf_error(extra_left, ret))
							goto err_out;
					}

					break;
				}
			}
		}

#ifdef MT_MAC

		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			for (i = 0; i < DBDC_BAND_NUM; i++)
				ShowAmpduCounter(pAd, i, extra);

			if (pAd->CommonCfg.bTXRX_RXV_ON) {
				extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
				ret = snprintf(extra + strlen(extra), extra_left, "/* Condition Number should enable mode4 of 0x6020_426c */\n");
				if (os_snprintf_error(extra_left, ret))
					goto err_out;
			extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left,
						"--10 packets Condition Number	 = [%d|%d|%d|%d|%d|%d|%d|%d|%d|%d]\n",
						(UINT8)(pAd->rxv2_cyc3[0] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[1] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[2] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[3] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[4] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[5] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[6] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[7] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[8] & 0xff),
						(UINT8)(pAd->rxv2_cyc3[9] & 0xff)
					   );
			if (os_snprintf_error(extra_left, ret))
				goto err_out;
			}
		}

#endif /* MT_MAC */
#else
		extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
		ret = snprintf(extra + strlen(extra), extra_left, "RSSI-A						   = %ld\n",
				(LONG)(pStaCfg->RssiSample.AvgRssi[0] - pAd->BbpRssiToDbmDelta));
		if (os_snprintf_error(extra_left, ret))
			goto err_out;
		extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
		ret = snprintf(extra + strlen(extra), extra_left, "RSSI-B (if available)		   = %ld\n",
				(LONG)(pStaCfg->RssiSample.AvgRssi[1] - pAd->BbpRssiToDbmDelta));
		if (os_snprintf_error(extra_left, ret))
			goto err_out;
		extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
		ret = snprintf(extra + strlen(extra), extra_left, "RSSI-C (if available)		   = %ld\n\n",
				(LONG)(pStaCfg->RssiSample.AvgRssi[2] - pAd->BbpRssiToDbmDelta));
		if (os_snprintf_error(extra_left, ret))
			goto err_out;
#endif /* ENHANCED_STAT_DISPLAY */
		extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
		ret = snprintf(extra + strlen(extra), extra_left, "SNR-A						  = %ld\n", (LONG)(pStaCfg->RssiSample.AvgSnr[0]));
		if (os_snprintf_error(extra_left, ret))
			goto err_out;
		extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
		ret = snprintf(extra + strlen(extra), extra_left, "SNR-B (if available) 		  = %ld\n\n", (LONG)(pStaCfg->RssiSample.AvgSnr[1]));
		if (os_snprintf_error(extra_left, ret))
			goto err_out;
	}

#ifdef WPA_SUPPLICANT_SUPPORT
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "WpaSupplicantUP				   = %d\n\n",
			pStaCfg->wpa_supplicant_info.WpaSupplicantUP);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
	ret = snprintf(extra + strlen(extra), extra_left, "FtSupport					   = %d\n\n", pStaCfg->Dot11RCommInfo.bFtSupport);
	if (os_snprintf_error(extra_left, ret))
		goto err_out;
#endif /* DOT11R_FT_SUPPORT */
#ifdef WSC_STA_SUPPORT
	wsc_ctrl = &pStaCfg->wdev.WscControl;

	/* display pin code */
	if (wsc_ctrl->WscEnrolleePinCodeLen == 8) {
		extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
		ret = snprintf(extra + strlen(extra), extra_left, "RT2860 Linux STA PinCode\t%08u\n", wsc_ctrl->WscEnrolleePinCode);
		if (os_snprintf_error(extra_left, ret))
			goto err_out;
	} else {
		extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
		ret = snprintf(extra + strlen(extra), extra_left, "RT2860 Linux STA PinCode\t%04u\n", wsc_ctrl->WscEnrolleePinCode);
		if (os_snprintf_error(extra_left, ret))
			goto err_out;
		}
	{
		char	mode_str[16] = {0};
		ULONG	wps_status, wps_state;
		int     idx = 0;

		wps_state = wsc_ctrl->WscState;
		wps_status = wsc_ctrl->WscStatus;

		if (wsc_ctrl->WscMode == WSC_PIN_MODE) {
			extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
			ret = snprintf(mode_str, extra_left, "PIN -");
			if (os_snprintf_error(extra_left, ret))
				goto err_out;
		} else {
			extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
			ret =  snprintf(mode_str, extra_left, "PBC -");
			if (os_snprintf_error(extra_left, ret))
				goto err_out;
			}
		extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
		ret = snprintf(extra + strlen(extra), extra_left, "WPS Information(Driver Auto-Connect is %s - %d):\n",
				wsc_ctrl->WscDriverAutoConnect ? "Enabled" : "Disabled",
				wsc_ctrl->WscDriverAutoConnect);
		if (os_snprintf_error(extra_left, ret))
			goto err_out;

		/* display pin code */
		/*sprintf(extra+strlen(extra), "RT2860 Linux STA PinCode\t%08u\n", pStaCfg->WscControl.WscEnrolleePinCode); */
		/* display status */
		if ((wps_state == WSC_STATE_OFF) || (wps_status & 0xff00)) {
			if (wps_status == STATUS_WSC_CONFIGURED) {
				extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
				ret = snprintf(extra + strlen(extra), extra_left, "WPS messages exchange successfully !!!\n");
				if (os_snprintf_error(extra_left, ret))
					goto err_out;
			} else if (wps_status == STATUS_WSC_NOTUSED) {
				extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
				ret = snprintf(extra + strlen(extra), extra_left, "WPS not used.\n");
				if (os_snprintf_error(extra_left, ret))
					goto err_out;
			} else if (wps_status & 0xff00) {	/* error message */
				if (wps_status == STATUS_WSC_PBC_TOO_MANY_AP) {
					extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
					ret = snprintf(extra + strlen(extra), extra_left, "%s Too many PBC AP. Stop WPS.\n", mode_str);
					if (os_snprintf_error(extra_left, ret))
						goto err_out;
				} else if (wps_status == STATUS_WSC_PBC_NO_AP) {
					extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
					ret = snprintf(extra + strlen(extra), extra_left, "%s No available PBC AP. Please wait...\n", mode_str);
					if (os_snprintf_error(extra_left, ret))
						goto err_out;
				} else if (wps_status & 0x0100) {
					extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
					ret = snprintf(extra + strlen(extra), extra_left, "%s Proceed to get the Registrar profile. Please wait...\n", mode_str);
					if (os_snprintf_error(extra_left, ret))
						goto err_out;
				} else {	/* status of eap failed */
					extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
					ret = snprintf(extra + strlen(extra), extra_left, "WPS didn't complete !!!\n");
					if (os_snprintf_error(extra_left, ret))
						goto err_out;
					}
			} else {
				/* wrong state */
			}
		} else {
			extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "%s WPS Proceed. Please wait...\n", mode_str);
			if (os_snprintf_error(extra_left, ret))
				goto err_out;
			}
		extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
		ret = snprintf(extra + strlen(extra), extra_left, "\n");
		if (os_snprintf_error(extra_left, ret))
			goto err_out;
		extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
		ret = snprintf(extra + strlen(extra), extra_left, "WPS Profile Count			   = %d\n", wsc_ctrl->WscProfile.ProfileCnt);
		if (os_snprintf_error(extra_left, ret))
			goto err_out;

		for (idx = 0; idx < wsc_ctrl->WscProfile.ProfileCnt ; idx++) {
			PWSC_CREDENTIAL pCredential = &wsc_ctrl->WscProfile.Profile[idx];
			char ssid_print[MAX_LEN_OF_SSID + 1];

			NdisZeroMemory(&ssid_print[0], MAX_LEN_OF_SSID + 1);

			if (strlen(extra) + sizeof(WSC_CREDENTIAL) >= MaxSize)
				break;

			extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "Profile[%d]:\n", idx);
			if (os_snprintf_error(extra_left, ret))
				goto err_out;
			NdisMoveMemory(&ssid_print[0], pCredential->SSID.Ssid, pCredential->SSID.SsidLength);
			extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "SSID 						   = %s\n", ssid_print);
			if (os_snprintf_error(extra_left, ret))
				goto err_out;
			extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "MAC							   = %02X:%02X:%02X:%02X:%02X:%02X\n",
					pCredential->MacAddr[0],
					pCredential->MacAddr[1],
					pCredential->MacAddr[2],
					pCredential->MacAddr[3],
					pCredential->MacAddr[4],
					pCredential->MacAddr[5]);
			if (os_snprintf_error(extra_left, ret))
				goto err_out;

			extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "AuthType 					   = %s\n", WscGetAuthTypeStr(pCredential->AuthType));
			if (os_snprintf_error(extra_left, ret))
				goto err_out;
			extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "EncrypType					   = %s\n", WscGetEncryTypeStr(pCredential->EncrType));
			if (os_snprintf_error(extra_left, ret))
				goto err_out;
			extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
			ret = snprintf(extra + strlen(extra), extra_left, "KeyIndex 					   = %d\n", pCredential->KeyIndex);
			if (os_snprintf_error(extra_left, ret))
				goto err_out;

			if (pCredential->KeyLength != 0) {
				if (pCredential->AuthType & (WSC_AUTHTYPE_WPAPSK | WSC_AUTHTYPE_WPA2PSK | WSC_AUTHTYPE_WPANONE)) {
					if (pCredential->KeyLength < 64) {
						extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
						ret = snprintf(extra + strlen(extra), extra_left, "Key							   = %s\n", pCredential->Key);
						if (os_snprintf_error(extra_left, ret))
							goto err_out;
					} else {
						char key_print[65] = {0};

						NdisMoveMemory(key_print, pCredential->Key, 64);
						extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
						ret = snprintf(extra + strlen(extra), extra_left, "Key							   = %s\n", key_print);
						if (os_snprintf_error(extra_left, ret))
							goto err_out;
					}
				} else if ((pCredential->AuthType == WSC_AUTHTYPE_OPEN) ||
						   (pCredential->AuthType == WSC_AUTHTYPE_SHARED)) {
					/*check key string is ASCII or not */
					if (RTMPCheckStrPrintAble((PCHAR)pCredential->Key, (UCHAR)pCredential->KeyLength)) {
						extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
						ret = snprintf(extra + strlen(extra), extra_left, "Key							   = %s\n", pCredential->Key);
						if (os_snprintf_error(extra_left, ret))
							goto err_out;
					} else {
						int idx;
						extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
						ret = snprintf(extra + strlen(extra), extra_left,  "Key 							= ");
						if (os_snprintf_error(extra_left, ret))
							goto err_out;

						for (idx = 0; idx < pCredential->KeyLength && idx < sizeof(pCredential->Key); idx++) {
							extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
							ret = snprintf(extra + strlen(extra), extra_left, "%02X", pCredential->Key[idx]);
							if (os_snprintf_error(extra_left, ret))
								goto err_out;
						}
						extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
						ret = snprintf(extra + strlen(extra), extra_left, "\n");
						if (os_snprintf_error(extra_left, ret))
							goto err_out;
					}
				}
			}

#ifdef IWSC_SUPPORT

			if (pStaCfg->BssType == BSS_ADHOC) {
				PIWSC_INFO	pIWscInfo = &pStaCfg->IWscInfo;

				extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
				ret = snprintf(extra + strlen(extra), extra_left, "Credential Registrar IPv4 Addr  = %d:%d:%d:%d\n",
						(pCredential->RegIpv4Addr & 0xFF000000) >> 24,
						(pCredential->RegIpv4Addr & 0x00FF0000) >> 16,
						(pCredential->RegIpv4Addr & 0x0000FF00) >> 8,
						(pCredential->RegIpv4Addr & 0x000000FF));
				if (os_snprintf_error(extra_left, ret))
					goto err_out;
				extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
				ret = snprintf(extra + strlen(extra), extra_left, "Credential Entrollee IPv4 Addr  = %d:%d:%d:%d\n",
						(pCredential->EnrIpv4Addr & 0xFF000000) >> 24,
						(pCredential->EnrIpv4Addr & 0x00FF0000) >> 16,
						(pCredential->EnrIpv4Addr & 0x0000FF00) >> 8,
						(pCredential->EnrIpv4Addr & 0x000000FF));
				if (os_snprintf_error(extra_left, ret))
					goto err_out;
				extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
				ret = snprintf(extra + strlen(extra), extra_left, "\nSelf IPv4 Addr 				 = %d:%d:%d:%d\n",
						(pIWscInfo->SelfIpv4Addr & 0xFF000000) >> 24,
						(pIWscInfo->SelfIpv4Addr & 0x00FF0000) >> 16,
						(pIWscInfo->SelfIpv4Addr & 0x0000FF00) >> 8,
						(pIWscInfo->SelfIpv4Addr & 0x000000FF));
				if (os_snprintf_error(extra_left, ret))
					goto err_out;
				extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
				ret = snprintf(extra + strlen(extra), extra_left, "IPv4 Subnet Mask 			   = %d:%d:%d:%d\n",
						(pIWscInfo->Ipv4SubMask & 0xFF000000) >> 24,
						(pIWscInfo->Ipv4SubMask & 0x00FF0000) >> 16,
						(pIWscInfo->Ipv4SubMask & 0x0000FF00) >> 8,
						(pIWscInfo->Ipv4SubMask & 0x000000FF));
				if (os_snprintf_error(extra_left, ret))
					goto err_out;
				extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
				ret = snprintf(extra + strlen(extra), extra_left, "AvaSubMaskListCount			   = %d", pIWscInfo->AvaSubMaskListCount);
				if (os_snprintf_error(extra_left, ret))
					goto err_out;
			}

#endif /* IWSC_SUPPORT */
		}
		extra_left = IW_PRIV_SIZE_MASK - strlen(extra);
		ret = snprintf(extra + strlen(extra), extra_left, "\n");
		if (os_snprintf_error(extra_left, ret))
			goto err_out;
		}
#endif /* WSC_STA_SUPPORT */
#ifdef DOT11_N_SUPPORT

	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
		/* Display Tx Aggregation statistics */
		DisplayTxAgg(pAd);
	}

#endif /* DOT11_N_SUPPORT */
	return NDIS_STATUS_SUCCESS;

err_out:
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RtmpIoctl_rt_private_get_statistics snprintf error!\n");
	return NDIS_STATUS_FAILURE;
}

#ifdef CONFIG_DOT11V_WNM
INT Send_BTM_Rsp(
		IN PRTMP_ADAPTER pAd,
		IN RTMP_STRING *PeerMACAddr,
		IN RTMP_STRING *BTMRsp,
		IN UINT32 BTMRspLen)
{
	UCHAR *Buf;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APcliIndex = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl = &pAd->StaCfg[APcliIndex].WNMCtrl;
	BTM_EVENT_DATA *Event;
	BTM_PEER_ENTRY *BTMPeerEntry;
	UINT32 Len = 0;
	INT32 Ret;
	BOOLEAN IsFound = FALSE;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEach(BTMPeerEntry, &pWNMCtrl->BTMPeerList, BTM_PEER_ENTRY, List) {
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, PeerMACAddr)) {
			IsFound = TRUE;
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);

	if (!IsFound) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "BTMPeerEntry not found\n");
		goto error0;
	}

	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event) + BTMRspLen);

	if (!Buf) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not available memory\n");
		goto error0;
	}
	NdisZeroMemory(Buf, sizeof(*Event) + BTMRspLen);

	Event = (BTM_EVENT_DATA *)Buf;
	Event->ControlIndex = APcliIndex;
	Len += 1;
	NdisMoveMemory(Event->PeerMACAddr, PeerMACAddr, MAC_ADDR_LEN);
	Len += MAC_ADDR_LEN;
	Event->EventType = BTM_RSP;
	Len += 2;
	Event->u.BTM_RSP_DATA.DialogToken = BTMPeerEntry->DialogToken;
	Len += 1;
	Event->u.BTM_RSP_DATA.BTMRspLen = BTMRspLen;
	Len += 2;
	NdisMoveMemory(Event->u.BTM_RSP_DATA.BTMRsp, BTMRsp, BTMRspLen);
	Len += BTMRspLen;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\nBefore adding BSS Transition Candidate List Entries::BTMRspLen=%d, Len=%d\n",
			 BTMRspLen, Len);
	MlmeEnqueue(pAd, BTM_STATE_MACHINE, BTM_RSP, Len, Buf, 0);
	os_free_mem(Buf);
	return TRUE;

error0:
	return FALSE;
}
#endif

/*
 *========================================================================
 *Routine Description:
 *	Communication with DRIVER module, whatever IOCTL.
 *
 *Arguments:
 *	pAdSrc			- WLAN control block pointer
 *	*pRequest		- the request from IOCTL
 *	Command			- communication command
 *	Subcmd			- communication sub-command
 *	*pData			- the communication data pointer
 *	Data			- the communication data
 *
 *Return Value:
 *	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
 *
 *Note:
 *========================================================================
 */
INT RTMP_STA_IoctlHandle(
	IN	VOID					*pAdSrc,
	IN	RTMP_IOCTL_INPUT_STRUCT	*pRequest,
	IN	INT						Command,
	IN	USHORT					Subcmd,
	IN	VOID					*pData,
	IN  ULONG					Data,
	IN  USHORT                  priv_flags)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	struct wifi_dev *wdev = NULL;
	SCAN_INFO *ScanInfo = NULL;
	INT Status = NDIS_STATUS_SUCCESS;
	BSS_TABLE *ScanTab = NULL;

#ifdef CONFIG_APSTA_MIXED_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_AP(GET_OPMODE_FROM_PAD(pAd)) {
		pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
	}
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

	ASSERT(pStaCfg);

	if (!pStaCfg)
		return 0;

	wdev = &pStaCfg->wdev;
	/* handle by command */
	switch (Command) {
	case CMD_RTPRIV_IOCTL_PREPARE: {
		RT_CMD_IOCTL_INTERFACE_CONFIG *pConfig = (RT_CMD_IOCTL_INTERFACE_CONFIG *)pData;

		pConfig->Status = RTMP_STA_IoctlPrepare(pAd, pData);

		if (pConfig->Status != 0)
			return NDIS_STATUS_FAILURE;
	}
	break;

	case (CMD_RTPRIV_IOCTL_STATION)CMD_RT_PRIV_IOCTL:
		if (Subcmd & OID_GET_SET_TOGGLE)
				Status = RTMPSetInformation(pAd, pRequest,  Subcmd, wdev);
		else
				Status = RTMPQueryInformation(pAd, pRequest, Subcmd, wdev);

		break;

	case CMD_RTPRIV_IOCTL_PARAM_SET: {
		RT_CMD_PARAM_SET *pCmdParam = (RT_CMD_PARAM_SET *)pData;
		RTMP_STRING *this_char = pCmdParam->pThisChar;
		RTMP_STRING *value = pCmdParam->pValue;

		Status = RTMPSTAPrivIoctlSet(pAd, this_char, value);
	}
	break;

	case CMD_RTPRIV_IOCTL_PARAM_SHOW: {
		RT_CMD_PARAM_SET *pCmdParam = (RT_CMD_PARAM_SET *)pData;
		RTMP_STRING *this_char = pCmdParam->pThisChar;
		RTMP_STRING *value = pCmdParam->pValue;

		Status = RTMPSTAPrivIoctlShow(pAd, this_char, value);
	}
	break;
#ifdef WCX_SUPPORT

	case CMD_MTPRIV_IOCTL_STA_META_SET:
		do_meta_cmd(MTPRIV_IOCTL_META_SET, pAd, pRequest, pData);
		break;

	case CMD_MTPRIV_IOCTL_STA_META_QUERY:
		do_meta_cmd(MTPRIV_IOCTL_META_QUERY, pAd, pRequest, pData);
		break;
#endif

	case CMD_RTPRIV_IOCTL_SITESURVEY_GET:
		RTMPIoctlGetSiteSurvey(pAd, pRequest);
		break;

	case (CMD_RTPRIV_IOCTL_STATION)CMD_RTPRIV_IOCTL_MAC:
		RTMPIoctlMAC(pAd, pRequest);
		break;

	case (CMD_RTPRIV_IOCTL_STATION)CMD_RTPRIV_IOCTL_E2P:
		RTMPIoctlE2PROM(pAd, pRequest);
		break;

	case (CMD_RTPRIV_IOCTL_STATION)CMD_RTPRIV_IOCTL_RF:
#ifdef RTMP_RF_RW_SUPPORT
		RTMPIoctlRF(pAd, pRequest);
#endif /* RTMP_RF_RW_SUPPORT */
		break;

	case (CMD_RTPRIV_IOCTL_STATION)CMD_RTPRIV_IOCTL_BBP:
		RTMPIoctlBbp(pAd, pRequest, pData, Data);
		break;

	case (CMD_RTPRIV_IOCTL_STATION)CMD_RTPRIV_IOCTL_SHOW:
		RTMPIoctlShow(pAd, pRequest, Subcmd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_SITESURVEY:
			StaSiteSurvey(pAd, (NDIS_802_11_SSID *)pData, Data, wdev);
		break;

	case (CMD_RTPRIV_IOCTL_STATION)CMD_RTPRIV_IOCTL_CHID_2_FREQ:
		RTMP_MapChannelID2KHZ(Data, (UINT32 *)pData);
		break;

	case (CMD_RTPRIV_IOCTL_STATION)CMD_RTPRIV_IOCTL_FREQ_2_CHID:
		RTMP_MapKHZ2ChannelID(Data, (UINT32 *)pData);
		break;

	case CMD_RTPRIV_IOCTL_STA_SCAN_SANITY_CHECK:
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) {
			/*
			 * Still scanning, indicate the caller should try again.
			 */
			pStaCfg->bSkipAutoScanConn = TRUE;
			return NDIS_STATUS_FAILURE;
		}
			ScanInfo = &wdev->ScanInfo;
			if (ScanInfo->bImprovedScan) {
			/*
			 * Fast scanning doesn't complete yet.
			 */
			pStaCfg->bSkipAutoScanConn = TRUE;
			return NDIS_STATUS_FAILURE;
		}

		break;

	case CMD_RTPRIV_IOCTL_STA_SCAN_END:
		ScanTab = get_scan_tab_by_wdev(pAd, wdev);
		pStaCfg->bSkipAutoScanConn = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%d(%d) BSS returned, data->length = %ld\n",
				ScanTab->BssNr, ScanTab->BssNr, Data);
		break;

	case CMD_RTPRIV_IOCTL_BSS_LIST_GET: {
		RT_CMD_STA_IOCTL_BSS_LIST *pBssList = (RT_CMD_STA_IOCTL_BSS_LIST *)pData;
		RT_CMD_STA_IOCTL_BSS *pList;
		UINT32 i;
		ScanTab = get_scan_tab_by_wdev(pAd, wdev);
			pBssList->BssNum = ScanTab->BssNr;

		for (i = 0; i < pBssList->MaxNum ; i++) {
				if (i >=  ScanTab->BssNr)
				break;

			pList = (pBssList->pList) + i;
				set_quality(pList, &ScanTab->BssEntry[i]);
		}
	}
	break;

	case CMD_RTPRIV_IOCTL_MSTA_INIT:
		MSTA_Init(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_MSTA_REMOVE:
		MSTA_Remove(pAd);
		break;

		/* snowpin for ap/sta ++ */
		/* snowpin for ap/sta -- */

		/* ------------------------------------------------------------------ */
		/* for standard IOCTL in LINUX OS */
		RTMP_STA_STANDARD_IOCTL_HANDLE(pAd, pData, Data, Subcmd, &pStaCfg->wdev);

	/* ------------------------------------------------------------------ */

	default:
		/* for IOCTL that also can be used in AP mode */
		Status = RTMP_COM_IoctlHandle(pAd, pRequest, Command, Subcmd, pData, Data);
		break;
	}

	return Status;
}

