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
 ****************************************************************************

    Module Name:
	ap_cfg.c

    Abstract:
    IOCTL related subroutines

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/


#include "rt_config.h"

#ifdef RLM_CAL_CACHE_SUPPORT
#include "phy/rlm_cal_cache.h"
#endif /* RLM_CAL_CACHE_SUPPORT */

#if defined(TXRX_STAT_SUPPORT) || defined(TR181_SUPPORT)
#include "hdev/hdev_basic.h"
#endif /*TR181_SUPPORT || TXRX_STAT_SUPPORT*/

#ifdef TR181_SUPPORT
#include "wlan_config/config_internal.h"
#endif

#define A_BAND_REGION_0				0
#define A_BAND_REGION_1				1
#define A_BAND_REGION_2				2
#define A_BAND_REGION_3				3
#define A_BAND_REGION_4				4
#define A_BAND_REGION_5				5
#define A_BAND_REGION_6				6
#define A_BAND_REGION_7				7
#define A_BAND_REGION_8				8
#define A_BAND_REGION_9				9
#define A_BAND_REGION_10			10

#define G_BAND_REGION_0				0
#define G_BAND_REGION_1				1
#define G_BAND_REGION_2				2
#define G_BAND_REGION_3				3
#define G_BAND_REGION_4				4
#define G_BAND_REGION_5				5
#define G_BAND_REGION_6				6


COUNTRY_CODE_TO_COUNTRY_REGION allCountry[] = {
	/* {Country Number, ISO Name, Country Name, Support 11A, 11A Country Region, Support 11G, 11G Country Region} */
	{0,		"DB",	"Debug",				TRUE,	A_BAND_REGION_7,	TRUE,	G_BAND_REGION_5},
	{8,		"AL",	"ALBANIA",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{12,	"DZ",	"ALGERIA",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{32,	"AR",	"ARGENTINA",			TRUE,	A_BAND_REGION_3,	TRUE,	G_BAND_REGION_1},
	{51,	"AM",	"ARMENIA",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{36,	"AU",	"AUSTRALIA",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{40,	"AT",	"AUSTRIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{31,	"AZ",	"AZERBAIJAN",			TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{48,	"BH",	"BAHRAIN",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{112,	"BY",	"BELARUS",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{56,	"BE",	"BELGIUM",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{84,	"BZ",	"BELIZE",				TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{68,	"BO",	"BOLIVIA",				TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{76,	"BR",	"BRAZIL",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{96,	"BN",	"BRUNEI DARUSSALAM",	TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{100,	"BG",	"BULGARIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{124,	"CA",	"CANADA",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{152,	"CL",	"CHILE",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{156,	"CN",	"CHINA",				TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{170,	"CO",	"COLOMBIA",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{188,	"CR",	"COSTA RICA",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{191,	"HR",	"CROATIA",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{196,	"CY",	"CYPRUS",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{203,	"CZ",	"CZECH REPUBLIC",		TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{208,	"DK",	"DENMARK",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{214,	"DO",	"DOMINICAN REPUBLIC",	TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{218,	"EC",	"ECUADOR",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{818,	"EG",	"EGYPT",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{222,	"SV",	"EL SALVADOR",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{233,	"EE",	"ESTONIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{246,	"FI",	"FINLAND",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{250,	"FR",	"FRANCE",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{268,	"GE",	"GEORGIA",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{276,	"DE",	"GERMANY",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{300,	"GR",	"GREECE",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{320,	"GT",	"GUATEMALA",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{340,	"HN",	"HONDURAS",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{344,	"HK",	"HONG KONG",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{348,	"HU",	"HUNGARY",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{352,	"IS",	"ICELAND",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{356,	"IN",	"INDIA",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{360,	"ID",	"INDONESIA",			TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{364,	"IR",	"IRAN",					TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{372,	"IE",	"IRELAND",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{376,	"IL",	"ISRAEL",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{380,	"IT",	"ITALY",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{392,	"JP",	"JAPAN",				TRUE,	A_BAND_REGION_9,	TRUE,	G_BAND_REGION_1},
	{400,	"JO",	"JORDAN",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{398,	"KZ",	"KAZAKHSTAN",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{408,	"KP",	"KOREA DEMOCRATIC PEOPLE'S REPUBLIC OF", TRUE,	A_BAND_REGION_5,	TRUE,	G_BAND_REGION_1},
	{410,	"KR",	"KOREA REPUBLIC OF",	TRUE,	A_BAND_REGION_5,	TRUE,	G_BAND_REGION_1},
	{414,	"KW",	"KUWAIT",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{428,	"LV",	"LATVIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{422,	"LB",	"LEBANON",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{438,	"LI",	"LIECHTENSTEIN",		TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{440,	"LT",	"LITHUANIA",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{442,	"LU",	"LUXEMBOURG",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{446,	"MO",	"MACAU",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{807,	"MK",	"MACEDONIA",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{458,	"MY",	"MALAYSIA",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{484,	"MX",	"MEXICO",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{492,	"MC",	"MONACO",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{504,	"MA",	"MOROCCO",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{528,	"NL",	"NETHERLANDS",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{554,	"NZ",	"NEW ZEALAND",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{578,	"NO",	"NORWAY",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{512,	"OM",	"OMAN",					TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{586,	"PK",	"PAKISTAN",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{591,	"PA",	"PANAMA",				TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{604,	"PE",	"PERU",					TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{608,	"PH",	"PHILIPPINES",			TRUE,	A_BAND_REGION_4,	TRUE,	G_BAND_REGION_1},
	{616,	"PL",	"POLAND",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{620,	"PT",	"PORTUGAL",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{630,	"PR",	"PUERTO RICO",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{634,	"QA",	"QATAR",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{642,	"RO",	"ROMANIA",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{643,	"RU",	"RUSSIA FEDERATION",	FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{682,	"SA",	"SAUDI ARABIA",			FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{702,	"SG",	"SINGAPORE",			TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{703,	"SK",	"SLOVAKIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{705,	"SI",	"SLOVENIA",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{710,	"ZA",	"SOUTH AFRICA",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{724,	"ES",	"SPAIN",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{752,	"SE",	"SWEDEN",				TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{756,	"CH",	"SWITZERLAND",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{760,	"SY",	"SYRIAN ARAB REPUBLIC",	FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{158,	"TW",	"TAIWAN",				TRUE,	A_BAND_REGION_3,	TRUE,	G_BAND_REGION_0},
	{764,	"TH",	"THAILAND",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{780,	"TT",	"TRINIDAD AND TOBAGO",	TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{788,	"TN",	"TUNISIA",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{792,	"TR",	"TURKEY",				TRUE,	A_BAND_REGION_2,	TRUE,	G_BAND_REGION_1},
	{804,	"UA",	"UKRAINE",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{784,	"AE",	"UNITED ARAB EMIRATES",	FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{826,	"GB",	"UNITED KINGDOM",		TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_1},
	{840,	"US",	"UNITED STATES",		TRUE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_0},
	{858,	"UY",	"URUGUAY",				TRUE,	A_BAND_REGION_5,	TRUE,	G_BAND_REGION_1},
	{860,	"UZ",	"UZBEKISTAN",			TRUE,	A_BAND_REGION_1,	TRUE,	G_BAND_REGION_0},
	{862,	"VE",	"VENEZUELA",			TRUE,	A_BAND_REGION_5,	TRUE,	G_BAND_REGION_1},
	{704,	"VN",	"VIET NAM",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{887,	"YE",	"YEMEN",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{716,	"ZW",	"ZIMBABWE",				FALSE,	A_BAND_REGION_0,	TRUE,	G_BAND_REGION_1},
	{999,	"",	"",	0,	0,	0,	0}
};

#define NUM_OF_COUNTRIES	(sizeof(allCountry)/sizeof(COUNTRY_CODE_TO_COUNTRY_REGION))

static const struct apcfg_parameters apcfg_for_peak = {
	.cfg_mode[0] = 16, /*WirelessMode*/
	.cfg_mode[1] = 17,
	.tx_power_percentage = 100, /*TxPower*/
	.tx_preamble = 1, /*TxPreamble*/
	.conf_len_thld = 2347, /*RTSThreshold*/
	.oper_len_thld = 2347,
	.conf_frag_thld = 2346, /*FragThreshold*/
	.oper_frag_thld = 2346,
	.bEnableTxBurst = 1, /*TxBurst*/
	.bUseShortSlotTime = 1, /*ShortSlot*/
#ifdef DOT11_N_SUPPORT
	.conf_ht_bw = 1, /*HT_BW*/
	.oper_ht_bw = 1,
#ifdef DOT11N_DRAFT3
	.bBssCoexEnable = 1, /*HT_BSSCoexistence*/
#endif

	.ht_tx_streams = 4, /*HT_TxStream*/
	.ht_rx_streams = 4, /*HT_RxStream*/

	.AmsduEnable = 1, /*HT_AMSDU*/
	.ba_decline = 1, /*HT_BADecline*/
	.ba_en = 1, /*HT_AutoBA*/
	.ba_rx_wsize = BA_WIN_SZ_256, /*HT_BAWinSize*/
	.ba_tx_wsize = BA_WIN_SZ_256, /*HT_BAWinSize*/
	.ht_gi = 1, /*HT_GI*/
	.ht_stbc = 1, /*HT_STBC*/
	.ht_ldpc = 1, /*HT_LDPC*/
	.bRdg = 1, /*HT_RDG*/
#endif
	.HT_DisallowTKIP = 1, /*HT_DisallowTKIP*/

#ifdef DOT11_VHT_AC
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	.conf_vht_bw = 2, /*VHT_BW, 5G only*/
	.oper_vht_bw = 2,
#else
	.conf_vht_bw = 1, /*VHT_BW, 5G only*/
	.oper_vht_bw = 1,
#endif
	.vht_sgi = 1, /*VHT_SGI, 5G only*/
	.vht_stbc = 1, /*VHT_STBC, 5G only*/
	.vht_bw_signal = 0, /*VHT_BW_SIGNAL, 5G only*/
	.vht_ldpc = 1, /*VHT_LDPC, 5G only*/
	.g_band_256_qam = 1, /*G_BAND_256QAM, 2.4G only*/
#endif

	.bIEEE80211H = 1, /*IEEE80211H*/

#ifdef MT_DFS_SUPPORT
	.bDfsEnable = 1, /*DfsEnable, 5G only*/
#endif

#ifdef BACKGROUND_SCAN_SUPPORT
	.DfsZeroWaitSupport = 0, /*DfsZeroWait, Single band only*/
#endif

	.MuOfdmaDlEnable = 1, /* MuOfdmaDlEnable */
	.MuOfdmaUlEnable = 1, /* MuOfdmaUlEnable */
	.MuMimoDlEnable = 1, /* MuMimoDlEnable */
	.MuMimoUlEnable = 1, /* MuMimoUlEnable */

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	.ETxBfEnCond = 1, /*ETxBfEnCond*/
#endif
#endif

	.ITxBfEn = 1, /*ITxBfEn*/

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	.MUTxRxEnable = 1, /*MUTxRxEnable*/
#endif
#endif
	.vht_1024_qam = 1, /* VHT 1024QAM Support */
	.HeOmiUlMuDataDisableRx = 1, /* UL MU Data Disable RX */
	.HeErSuRxDisable = 0, /* ER SU Rx Disable */
};

#ifdef CFG_SUPPORT_MU_MIMO

/* iwprive test code */

INT32 hqa_mu_get_qd(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_get_init_mcs(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_get_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT32 hqa_mu_cal_init_mcs(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_cal_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT32 hqa_mu_set_snr_offset(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_set_zero_nss(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_set_speedup_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_set_mu_table(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_set_group(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_set_enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_set_gid_up(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_su_cal_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_su_get_lq(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_set_trigger_mu_tx(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_reset_murx_cnt(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT32 hqa_mu_reset_mutx_cnt(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

/* get function */
INT ShowMuEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuProfileProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowClusterTblEntryProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowGroupUserThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowGroupNssThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowTxReqMinTimeProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowSuNssCheckProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowCalcInitMCSProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowTxopDefaultProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowSuLossThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuGainThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowSecondaryAcPolicyProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowGroupTblDmcsMaskProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMaxGroupSearchCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowMuProfileTxStsCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowHqaMURxPktCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT ShowHqaMUTxPktCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/* set function */
INT SetMuProfileProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetGroupTblEntryProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetClusterTblEntryProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetGroupUserThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetGroupNssThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTxReqMinTimeProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetSuNssCheckProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetCalculateInitMCSProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTriggerGIDMgmtFrameProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTriggerMuTxProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTriggerDegroupProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTriggerGroupProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTriggerBbpProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTriggerSndProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuNdpDeltaTxPwr(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetTxopDefaultProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetSuLossThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuGainThresholdProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetSecondaryAcPolicyProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetGroupTblDmcsMaskProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMaxGroupSearchCntProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

#ifdef AIR_MONITOR
INT Set_Enable_Air_Monitor_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_Monitor_max_pkt_cnt_app(IN RTMP_ADAPTER * pAd, IN RTMP_STRING * arg);
INT Set_MonitorRule_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorIndex_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorShowAll_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorClearCounter_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget0_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget1_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget2_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget3_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget4_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget5_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget6_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget7_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget8_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget9_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget10_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget11_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget12_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget13_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget14_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MonitorTarget15_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID Air_Monitor_Pkt_Report_Action(RTMP_ADAPTER *pAd, UINT16 wcid, RX_BLK *pRxBlk);
#endif /* AIR_MONITOR */

#ifdef DSCP_PRI_SUPPORT
INT	Set_Dscp_Pri_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg);

INT	Set_Dscp_Pri_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg);

INT	Show_Dscp_Pri_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg);
#endif /*DSCP_PRI_SUPPORT*/

#ifdef CFG_SUPPORT_MU_MIMO
INT SetMuStaParamProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

#ifdef MAC_REPEATER_SUPPORT
INT set_dump_rx_ba_scoreboard_proc(
       IN PRTMP_ADAPTER   pAd,
       IN RTMP_STRING     *arg);
#endif /* MAC_REPEATER_SUPPORT */

#ifdef CFG_SUPPORT_MU_MIMO_RA
/* mura set function */
INT SetMuraPeriodicSndProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraTestAlgorithmProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraTestAlgorithmInit(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraFixedSndParamProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraHwFallbackProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraDisableCN3CN4Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/* mura get function */
INT GetMuraPFIDStatProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT Set_CountryString_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_CountryCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_ChGeography_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef SPECIFIC_TX_POWER_SUPPORT
INT Set_AP_PKT_PWR(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* SPECIFIC_TX_POWER_SUPPORT */

#ifdef SW_CONNECT_SUPPORT
INT	Set_DummyWtbl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* SW_CONNECT_SUPPORT */

INT Set_AP_SSID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_TxRate_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_OLBCDetection_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_AP_PerMbssMaxStaNum_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_AP_SlotTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef IAPP_SUPPORT
INT	Set_IappPID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* IAPP_SUPPORT */

INT Set_AP_WpaMixPairCipher_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AP_RekeyInterval_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AP_RekeyMethod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AP_PMKCachePeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AP_ASSOC_REQ_RSSI_THRESHOLD(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_AP_KickStaRssiLow_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_BasicRate_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

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

INT Set_BeaconPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DtimPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_NoForwarding_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_NoForwardingBTNSSID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AP_WmmCapable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_HideSSID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef VLAN_SUPPORT
INT Set_VLANEn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_VLANID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_VLANPriority_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_VLAN_TAG_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_VLAN_Policy_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_VLAN_Info_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /*VLAN_SUPPORT*/

#ifdef CONFIG_VLAN_GTK_SUPPORT
INT Show_VLAN_GTK_Info_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT Set_AccessPolicy_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ACLAddEntry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ACLDelEntry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ACLShowAll_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ACLClearAll_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_SiteSurvey_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AutoChannelSel_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef CONFIG_6G_SUPPORT
INT Set_PSC_ACS_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT Set_PartialScan_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_PartialScan_Timer_Interval_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_PartialScan_Num_of_CH_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Scan_SkipList_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Scan_DwellTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Scan_DFS_CH_Utilization_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_scan_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef AP_SCAN_SUPPORT
INT Set_AutoChannelSelCheckTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_AutoChannelSelCheckMinTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ClearSiteSurvey_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#endif /* AP_SCAN_SUPPORT */

INT Set_BADecline_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef EAP_STATS_SUPPORT
INT Show_Eap_Stats_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* EAP_STATS_SUPPORT */

#ifdef TXRX_STAT_SUPPORT
INT Show_Sta_Stat_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_Bss_Stat_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_Radio_Stat_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Enable_Last_Sec_TXRX_Stats(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Enable_RSSI_Stats(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT Show_StaCount_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_Sat_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
static INT show_apcfg_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Show_RAInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef TXBF_SUPPORT
INT Show_TxBfInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* TXBF_SUPPORT */
#ifdef RTMP_MAC_PCI
#ifdef DBG_DIAGNOSE
INT Set_DiagOpt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_diag_cond_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_Diag_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DBG_DAIGNOSE */
#endif /* RTMP_MAC_PCI */

INT show_timer_list(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_wtbl_state(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT show_radio_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Show_Sat_Reset_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_MATTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef DOT1X_SUPPORT
INT Set_IEEE8021X_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_PreAuth_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_RADIUS_Server_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_RADIUS_Port_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_RADIUS_Key_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DeletePMKID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DumpPMKID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef RADIUS_MAC_ACL_SUPPORT
INT Set_RADIUS_MacAuth_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_RADIUS_CacheTimeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT show_RADIUS_acl_cache(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* DOT1X_SUPPORT */

INT Set_DisConnectSta_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DisConnectAllSta_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
INT Set_Wds_Proc(PRTMP_ADAPTER	pAd, RTMP_STRING *arg);
#endif

#ifdef MBSS_AS_WDS_AP_SUPPORT
INT Set_WdsMac_Proc(PRTMP_ADAPTER	pAd, RTMP_STRING *arg);
#endif

#ifdef NF_SUPPORT_V2
static INT Set_NF_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
#endif

#ifdef RADIUS_MAC_AUTH_SUPPORT
INT Set_Radius_Mac_Auth_Policy_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
#endif

#ifdef DYNAMIC_VLAN_SUPPORT
INT Set_Dvlan_Proc(PRTMP_ADAPTER	pAd, RTMP_STRING *arg);
INT Set_Sta_Vlan(PRTMP_ADAPTER pAd, RT_CMD_AP_STA_VLAN *sta_vlan);
#endif

#ifdef HOSTAPD_11R_SUPPORT
INT Set_Ft_Param(PRTMP_ADAPTER pAd, RT_CMD_AP_11R_PARAM *ap_11r_param);
#endif
#if defined(APCLI_SUPPORT) || defined(CONFIG_STA_SUPPORT)
INT Set_ApCli_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#ifdef APCLI_SUPPORT
INT Set_ApCli_Ssid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ApCli_Bssid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ApCli_TxMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ApCli_TxMcs_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ApCli_WirelessMode_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

#ifdef APCLI_AUTO_CONNECT_SUPPORT
INT Set_ApCli_AutoConnect_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#ifdef APCLI_CONNECTION_TRIAL
INT Set_ApCli_Trial_Ch_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* APCLI_CONNECTION_TRIAL */

#ifdef WPA_SUPPLICANT_SUPPORT
INT Set_ApCli_Wpa_Support(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_ApCli_IEEE8021X_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif/*WPA_SUPPLICANT_SUPPORT*/

#ifdef MAC_REPEATER_SUPPORT
INT Set_ReptMode_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Cli_Link_Map_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MAC_REPEATER_SUPPORT */

#ifdef WSC_AP_SUPPORT
INT Set_AP_WscSsid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef APCLI_SUPPORT
INT Set_ApCli_WscScanMode_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
#endif /* APCLI_SUPPORT */
#endif /* WSC_AP_SUPPORT */

#ifdef APCLI_SUPPORT
INT Set_ApCli_Cert_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_ApCli_Cert_Agg_Limit_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* APCLI_SUPPORT */

/* Add for APCLI PMF 5.3.3.3 option test item. (Only Tx De-auth Req. and make sure the pkt can be Encrypted) */
INT ApCliTxDeAuth(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef ROAMING_ENHANCE_SUPPORT
INT Set_RoamingEnhance_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */
#ifdef UAPSD_SUPPORT
INT Set_UAPSD_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* UAPSD_SUPPORT */

#ifdef WSC_AP_SUPPORT
INT Set_WscStatus_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef BB_SOC
INT Set_WscOOB_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT Set_WscStop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID RTMPIoctlWscProfile(
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

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
INT RTMPIoctlQueryScanResult(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq);
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

BOOLEAN WscCheckEnrolleeNonceFromUpnp(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_STRING *pData,
	IN  USHORT			Length,
	IN  PWSC_CTRL       pWscControl);

UCHAR	WscRxMsgTypeFromUpnp(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  RTMP_STRING *pData,
	IN	USHORT				Length);

INT	    WscGetConfForUpnp(
	IN	PRTMP_ADAPTER	pAd,
	IN  PWSC_CTRL       pWscControl);

#ifdef CON_WPS
INT     Set_ConWpsApCliMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT     Set_ConWpsApcliAutoPreferIface_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT     Set_ConWpsApCliDisabled_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT     Set_ConWpsApDisabled_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CON_WPS */
INT	Set_AP_WscConfMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_AP_WscConfStatus_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_AP_WscPinCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AP_WscSecurityMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_AP_WscMultiByteCheck_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscVersion_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef VENDOR_FEATURE6_SUPPORT
INT	Set_WscUUID_STR_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_WscUUID_HEX_E_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* VENDOR_FEATURE6_SUPPORT */

#ifdef WSC_V2_SUPPORT
INT	Set_WscV2Support_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscVersion2_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscExtraTlvTag_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscExtraTlvType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscExtraTlvData_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscSetupLock_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscFragment_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscFragmentSize_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscMaxPinAttack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_WscSetupLockTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* WSC_V2_SUPPORT */
INT	Set_WscAutoTriggerDisable_Proc(
	IN	RTMP_ADAPTER *pAd,
	IN	RTMP_STRING	 *arg);
#endif /* WSC_AP_SUPPORT */


#ifdef CONFIG_AP_SUPPORT

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
INT set_mgm_rate_proc(IN RTMP_ADAPTER * pAd, IN RTMP_STRING * arg);
INT show_mgmrate(IN RTMP_ADAPTER * pAd, IN RTMP_STRING * arg);
INT show_bcnrate(IN RTMP_ADAPTER * pAd, IN RTMP_STRING * arg);
INT set_suprateset_proc(IN RTMP_ADAPTER * pAd, IN RTMP_STRING * arg);
INT set_htsuprateset_proc(IN RTMP_ADAPTER * pAd, IN RTMP_STRING * arg);
INT set_vhtsuprateset_proc(IN RTMP_ADAPTER * pAd, IN RTMP_STRING * arg);
#ifdef DOT11_HE_AX
INT set_hesuprateset_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg);
#endif
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */


#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
INT Set_McastType(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MCAST_VENDOR10_CUSTOM_FEATURE */
INT Set_McastPhyMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_McastMcs(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_McastRate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MCAST_RATE_SPECIFIC */

#ifdef HIGHPRI_RATE_SPECIFIC
INT Set_HighPriPhyMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_HighPriMcs(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_HighPriRate(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* HIGHPRI_RATE_SPECIFIC */

#ifdef DOT11N_DRAFT3
INT Set_OBSSScanParam_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_AP2040ReScan_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DOT11N_DRAFT3 */

INT Set_EntryLifeCheck_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
static INT Set_StaPowerOffDetect_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef RTMP_RBUS_SUPPORT
#ifdef LED_CONTROL_SUPPORT
INT Set_WlanLed_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* LED_CONTROL_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

#ifdef AP_QLOAD_SUPPORT
INT Set_QloadClr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/* QLOAD ALARM */
INT Set_QloadAlarmTimeThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_QloadAlarmNumThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

INT Set_MemDebug_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef CONFIG_AP_SUPPORT
INT Set_PowerSaveLifeTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* CONFIG_AP_SUPPORT */

#ifdef P2P_SUPPORT
INT Set_P2p_OpMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_P2pCli_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_P2pCli_Ssid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_P2pCli_Bssid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_P2pCli_AuthMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_P2pCli_EncrypType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_P2pCli_DefaultKeyID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_P2pCli_WPAPSK_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_P2pCli_Key1_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_P2pCli_Key2_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_P2pCli_Key3_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_P2pCli_Key4_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_P2pCli_TxMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_P2pCli_TxMcs_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef WSC_AP_SUPPORT
INT Set_P2pCli_WscSsid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* WSC_AP_SUPPORT */
#endif /* P2P_SUPPORT */

#ifdef DYNAMIC_VGA_SUPPORT
INT	Set_DyncVgaEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT set_false_cca_hi_th(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_false_cca_low_th(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DYNAMIC_VGA_SUPPORT */

#ifdef MT_MAC
INT setApTmrEnableProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_TmrCalResult_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef DBG
INT    Set_AP_DumpTime_Proc(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_STRING     *arg);

INT    Set_BcnStateCtrl_Proc(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_STRING     *arg);
#endif /*DBG*/

#ifdef PRE_CAL_TRX_SET1_SUPPORT
INT Set_KtoFlash_Debug_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);

INT Set_RDCE_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif /* PRE_CAL_TRX_SET1_SUPPORT */

#ifdef TX_AGG_ADJUST_WKR
INT Set_AggAdjWkr_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif /* TX_AGG_ADJUST_WKR */

#ifdef RLM_CAL_CACHE_SUPPORT
INT Set_RLM_Cal_Cache_Debug_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif /* RLM_CAL_CACHE_SUPPORT */

#ifdef PKT_BUDGET_CTRL_SUPPORT
INT Set_PBC_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);

#endif /*PKT_BUDGET_CTRL_SUPPORT*/

#ifdef PS_STA_FLUSH_SUPPORT
INT Set_PS_Flush_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg);
#endif /*PS_STA_FLUSH_SUPPORT*/

INT Set_BWF_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);

#ifdef CONFIG_HOTSPOT_R2
INT Set_CR4_Hotspot_Flag(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif /* CONFIG_HOTSPOT_R2 */

#ifdef HOSTAPD_HS_R2_SUPPORT

INT Set_Qload_Bss(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);

INT Set_Icmpv4_Deny(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);

INT Set_L2_Filter(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);

INT Set_DGAF_Disable(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg);

INT Set_QosMap(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg);

#endif
#if defined(HOSTAPD_HS_R2_SUPPORT) || defined(CONFIG_PROXY_ARP)
INT Set_ProxyArp_Enable(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg);

#endif

#ifdef HOSTAPD_HS_R3_SUPPORT
INT Set_Osu_Enable(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg);
#endif

#ifdef HTC_DECRYPT_IOT
INT Set_HTC_Err_TH_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
INT Set_Entry_HTC_Err_Cnt_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
INT Set_WTBL_AAD_OM_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif /* HTC_DECRYPT_IOT */

#ifdef DHCP_UC_SUPPORT
INT Set_DHCP_UC_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg);
#endif /* DHCP_UC_SUPPORT */
#endif /* MT_MAC */

#ifdef CONFIG_TX_DELAY
INT Set_TX_Batch_Cnt_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);

INT Set_Pkt_Min_Len_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);

INT Set_Pkt_Max_Len_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);

INT Set_TX_Delay_Timeout_Proc(
	PRTMP_ADAPTER pAd,
	char *arg);
#endif

#ifdef DBG_STARVATION
static INT show_starv_info_proc(
	struct _RTMP_ADAPTER *ad,
	char *arg)
{
	starv_log_dump(&ad->starv_log_ctrl);
	return TRUE;
}
#endif /*DBG_STARVATION*/

#ifdef WIFI_MD_COEX_SUPPORT
INT Set_CoexDebug_Level(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

INT32 show_wmm_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef DOT11_HE_AX
UINT16 he_mcs_map(UINT8 nss, UINT8 he_mcs);
#endif

#ifdef CONFIG_6G_SUPPORT
INT set_6g_ap_iob_discovery_proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING		* arg);

INT set_6g_ap_oob_discovery_proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING		* arg);

INT set_6g_ap_qos_injector_proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING		* arg);

#endif

#ifdef CONFIG_MAP_SUPPORT
INT Set_BS_ProbeEvent_Proc(RTMP_ADAPTER * pAd, RTMP_STRING * arg);
#endif

#ifdef VLAN_SUPPORT
INT Set_Vlan2eth_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	char *arg);
#endif
BOOLEAN wdev_down_exec_ioctl(RTMP_IOCTL_INPUT_STRUCT       * wrq, USHORT subcmd);
static struct {
	RTMP_STRING *name;
	INT (*set_proc)(PRTMP_ADAPTER pAdapter, RTMP_STRING *arg);
} *PRTMP_PRIVATE_SET_PROC, RTMP_PRIVATE_SUPPORT_PROC[] = {
	{"RateAlg",						Set_RateAlg_Proc},
#ifdef NEW_RATE_ADAPT_SUPPORT
	{"PerThrdAdj",					Set_PerThrdAdj_Proc},
	{"LowTrafficThrd",				Set_LowTrafficThrd_Proc},
	{"TrainUpRule",					Set_TrainUpRule_Proc},
	{"TrainUpRuleRSSI",				Set_TrainUpRuleRSSI_Proc},
	{"TrainUpLowThrd",				Set_TrainUpLowThrd_Proc},
	{"TrainUpHighThrd",				Set_TrainUpHighThrd_Proc},
#endif /* NEW_RATE_ADAPT_SUPPORT */
	{"CountryRegion",				Set_CountryRegion_Proc},
	{"CountryRegionABand",			Set_CountryRegionABand_Proc},
	{"CountryString",				Set_CountryString_Proc},
	{"CountryCode",				Set_CountryCode_Proc},
#ifdef EXT_BUILD_CHANNEL_LIST
	{"ChGeography",				Set_ChGeography_Proc},
#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef AIR_MONITOR
	{"mnt_en",				Set_Enable_Air_Monitor_Proc},
	{"mnt_max_pkt_app",			set_Monitor_max_pkt_cnt_app},
	{"mnt_rule",				Set_MonitorRule_Proc},
	{"mnt_sta",				Set_MonitorTarget_Proc},
	{"mnt_idx",				Set_MonitorIndex_Proc},
	{"mnt_show",				Set_MonitorShowAll_Proc},
	{"mnt_clr",				Set_MonitorClearCounter_Proc},
	{"mnt_sta0",				Set_MonitorTarget0_Proc},
	{"mnt_sta1",				Set_MonitorTarget1_Proc},
	{"mnt_sta2",				Set_MonitorTarget2_Proc},
	{"mnt_sta3",				Set_MonitorTarget3_Proc},
	{"mnt_sta4",				Set_MonitorTarget4_Proc},
	{"mnt_sta5",				Set_MonitorTarget5_Proc},
	{"mnt_sta6",				Set_MonitorTarget6_Proc},
	{"mnt_sta7",				Set_MonitorTarget7_Proc},
	{"mnt_sta8",				Set_MonitorTarget8_Proc},
	{"mnt_sta9",				Set_MonitorTarget9_Proc},
	{"mnt_sta10",				Set_MonitorTarget10_Proc},
	{"mnt_sta11",				Set_MonitorTarget11_Proc},
	{"mnt_sta12",				Set_MonitorTarget12_Proc},
	{"mnt_sta13",				Set_MonitorTarget13_Proc},
	{"mnt_sta14",				Set_MonitorTarget14_Proc},
	{"mnt_sta15",				Set_MonitorTarget15_Proc},
#endif /* AIR_MONITOR */
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
	{"ch_sw",					set_ch_switch_monitor_proc},
	{"ch_sw_cancel",			cancel_ch_switch_monitor_proc},
#endif
#ifdef MWDS
	{"ApMWDS",					Set_Ap_MWDS_Proc},
#endif /* MWDS */
#ifdef A4_CONN
	{"APProxyRefresh",                      Set_APProxy_Refresh_Proc},
	{"APProxyStatus",                       Set_APProxy_Status_Show_Proc},
#endif /* A4_CONN */
#ifdef WH_EVENT_NOTIFIER
	{"CustomOUI",				SetCustomOUIProc},
	{"CustomVIE",				SetCustomVIEProc},
	{"ChLoadDetectPeriod",		SetChannelLoadDetectPeriodProc},
	{"StaRssiDetectThrd",	    SetStaRssiDetectThresholdProc},
	{"StaTxPktDetectPeriod",	SetStaTxPktDetectPeriodProc},
	{"StaTxPktDetectThrd",	    SetStaTxPacketDetectThresholdProc},
	{"StaRxPktDetectPeriod",	SetStaRxPktDetectPeriodProc},
	{"StaRxPktDetectThrd",	    SetStaRxPacketDetectThresholdProc},
#endif /* WH_EVENT_NOTIFIER */

#ifdef WTBL_TDD_SUPPORT
	{"wtbltddinfo",					WtblTdd_DumpTab},
	{"wtbltddtest",					WtblTdd_TestAction},
	{"wtbltdd_dbg",				WtblTdd_DbgSet},
	{"wtbltdd_timelog",				WtblTdd_TimeLogAction},
	{"wtbldumpAll",				WtblAll_DumpAction},
	{"wtblEntryDump",				WtblEntry_DumpAction},
#endif /* WTBL_TDD_SUPPORT */

#ifdef SW_CONNECT_SUPPORT
	{"dummy_set",				Set_DummyWtbl_Proc},
#endif /* SW_CONNECT_SUPPORT */

	{"SSID",						Set_AP_SSID_Proc},
	{"WirelessMode",				Set_WirelessMode_Proc},
	{"BasicRate",					Set_BasicRate_Proc},
	{"ShortSlot",					Set_ShortSlot_Proc},
	{"ProbeRspTimes",			Set_Probe_Rsp_Times_Proc},
	{"Channel",					Set_Channel_Proc},
	{"SeamlessCSA",				Set_SeamlessCSA_Proc},
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
		{"V10Converter",				Set_V10ConverterMode_Proc},
#endif /*CONVERTER_MODE_SWITCH_SUPPORT*/
	{"PhyChannel",					set_phy_channel_proc},

#ifdef WIFI_MD_COEX_SUPPORT
	{"UnsafeState",					Set_UnsafeChannel_State},
	{"UnsafeChList",				Set_UnsafeChannel_Proc},
	{"TriggerUnsafe",				Trigger_UnsafeChannel_Event},
	{"CoexDebug",					Set_CoexDebug_Level},
	{"IdcState",					Set_Idc_State},
#endif

#ifdef REDUCE_TCP_ACK_SUPPORT
	{"ReduceAckEnable",             Set_ReduceAckEnable_Proc},
	{"ReduceAckProb",               Set_ReduceAckProb_Proc},
#endif
	{"BeaconPeriod",				Set_BeaconPeriod_Proc},
	{"DtimPeriod",					Set_DtimPeriod_Proc},
	{"TxPower",					Set_TxPower_Proc},
	{"MaxTxPwr",				Set_MaxTxPwr_Proc},
	{"BGProtection",				Set_BGProtection_Proc},
	{"DisableOLBC",				Set_OLBCDetection_Proc},
	{"TxPreamble",				Set_TxPreamble_Proc},
	{"RTSThreshold",				Set_RTSThreshold_Proc},
	{"FragThreshold",				Set_FragThreshold_Proc},
	{"TxBurst",					Set_TxBurst_Proc},
#ifdef DELAY_TCP_ACK_V2
	{"Txoppeaktpen",			Set_peak_tp_txop_dynamic_adjust_enable_Proc},
	{"Txoppeaktpaifsn",			Set_peak_tp_be_aifsn_Proc},
	{"Txoppeaktprxavglen",		Set_peak_tp_rx_avg_len_th_Proc},
	{"PeakTp",		            show_peak_tput_proc},
	{"PeakTpRxLowBoundTh",		    Set_peak_tp_rx_lower_bound_th_Proc},
	{"PeakTpRxHigherBoundTh",		Set_peak_tp_rx_higher_bound_th_Proc},
#endif /* DELAY_TCP_ACK_V2 */
	{"MbssMaxStaNum",					Set_AP_PerMbssMaxStaNum_Proc},
#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
	{"wds",							Set_Wds_Proc},
#endif
#ifdef MBSS_AS_WDS_AP_SUPPORT
	{"WdsMac",						Set_WdsMac_Proc},
#endif
#ifdef NF_SUPPORT_V2
	{"NF",							Set_NF_Proc},
#endif
	{"BssMaxIdle",					set_bss_max_idle_period_proc},
	{"SlotTime",					Set_AP_SlotTime_Proc},
	{"DynamicAGG",				Set_DynamicAGG_Proc},
#ifdef DOT11_N_SUPPORT
	{"BASetup",					Set_BASetup_Proc},
	{"BADecline",					Set_BADecline_Proc},
	{"SendMIMOPS",				Set_SendSMPSAction_Proc},
	{"BAOriTearDown",				Set_BAOriTearDown_Proc},
	{"BARecTearDown",				Set_BARecTearDown_Proc},
	{"HtBw",						Set_HtBw_Proc},
	{"HtMcs",						Set_HtMcs_Proc},
	{"HtGi",						Set_HtGi_Proc},
	{"HtOpMode",					Set_HtOpMode_Proc},
	{"HtLdpc",						Set_HtLdpc_Proc},
	{"HtStbc",					Set_HtStbc_Proc},
	{"HtExtcha",					Set_HtExtcha_Proc},
	{"HtMpduDensity",				Set_HtMpduDensity_Proc},
	{"HtBaWinSize",				Set_HtBaWinSize_Proc},
	{"HtMIMOPS",					Set_HtMIMOPSmode_Proc},
	{"HtRdg",						Set_HtRdg_Proc},
	{"HtLinkAdapt",				Set_HtLinkAdapt_Proc},
	{"HtAmsdu",					Set_HtAmsdu_Proc},
	{"HtAutoBa",					Set_HtAutoBa_Proc},
	{"HtProtect",					Set_HtProtect_Proc},
	{"HtMimoPs",					Set_HtMimoPs_Proc},
	{"HtTxStream",				Set_HtTxStream_Proc},
	{"HtRxStream",				Set_HtRxStream_Proc},
	{"ForceShortGI",				Set_ForceShortGI_Proc},
	{"ForceGF",	Set_ForceGF_Proc},
	{"HtTxBASize",					Set_HtTxBASize_Proc},
	{"BurstMode",					Set_BurstMode_Proc},
#ifdef GREENAP_SUPPORT
	{"GreenAP",					Set_GreenAP_Proc},
#endif /* GREENAP_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	{"PcieAspm",				set_pcie_aspm_dym_ctrl_cap_proc},
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	{"twtsupport",				set_twt_support_proc},
	{"twt",						set_twt_proc},
	{"btwt",					set_btwt_proc},
#endif /* WIFI_TWT_SUPPORT */
	{"MuEdcaTimer",					set_muedca_proc},
#endif /* DOT11_HE_AX */
	{"HtDisallowTKIP",				Set_HtDisallowTKIP_Proc},
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
	{"VhtBw",					Set_VhtBw_Proc},
	{"VhtLdpc",					Set_VhtLdpc_Proc},
	{"VhtStbc",					Set_VhtStbc_Proc},
	{"VhtBwSignal",				set_VhtBwSignal_Proc},
	{"VhtDisallowNonVHT",		Set_VhtDisallowNonVHT_Proc},
#endif /* DOT11_VHT_AC */

#ifdef IAPP_SUPPORT
	{"IappPID",					Set_IappPID_Proc},
#endif /* IAPP_SUPPORT */
#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
	{"V10RssiEnbl",				Set_RSSI_Enbl},
#endif
#ifdef AGGREGATION_SUPPORT
	{"PktAggregate",				Set_PktAggregate_Proc},
#endif /* AGGREGATION_SUPPORT */

	{"WmmCapable",				Set_AP_WmmCapable_Proc},

	{"BSSAifsn",				Set_BSSAifsn_Proc},
	{"BSSCwmin",				Set_BSSCwmin_Proc},
	{"BSSCwmax",				Set_BSSCwmax_Proc},
	{"BSSTxop",					Set_BSSTxop_Proc},

	{"APAifsn",					Set_APAifsn_Proc},
	{"APCwmin",					Set_APCwmin_Proc},
	{"APCwmax",					Set_APCwmax_Proc},
	{"APTxop",					Set_APTxop_Proc},

	{"NoForwarding",				Set_NoForwarding_Proc},
	{"NoForwardingBTNBSSID",		Set_NoForwardingBTNSSID_Proc},
	{"HideSSID",					Set_HideSSID_Proc},
	{"IEEE80211H",				Set_IEEE80211H_Proc},
#ifdef VLAN_SUPPORT
	{"VLANEn",					Set_VLANEn_Proc},
	{"VLANID",					Set_VLANID_Proc},
	{"VLANPriority",				Set_VLANPriority_Proc},
	{"VLANTag",				Set_VLAN_TAG_Proc},
	{"VLANPolicy",				Set_VLAN_Policy_Proc},
#endif /*VLAN_SUPPORT*/
	{"AuthMode",					Set_SecAuthMode_Proc},
	{"EncrypType",					Set_SecEncrypType_Proc},
	{"WpaMixPairCipher",			Set_AP_WpaMixPairCipher_Proc},
	{"RekeyInterval",				Set_AP_RekeyInterval_Proc},
	{"RekeyMethod",				Set_AP_RekeyMethod_Proc},
	{"DefaultKeyID",				Set_SecDefaultKeyID_Proc},
	{"Key1",						Set_SecKey1_Proc},
	{"Key2",						Set_SecKey2_Proc},
	{"Key3",						Set_SecKey3_Proc},
	{"Key4",						Set_SecKey4_Proc},
	{"AccessPolicy",				Set_AccessPolicy_Proc},
	{"ACLAddEntry",					Set_ACLAddEntry_Proc},
	{"ACLDelEntry",					Set_ACLDelEntry_Proc},
	{"ACLShowAll",					Set_ACLShowAll_Proc},
	{"ACLClearAll",					Set_ACLClearAll_Proc},
	{"WPAPSK",					Set_SecWPAPSK_Proc},
	{"RadioOn",					Set_RadioOn_Proc},
	{"Lp",	Set_Lp_Proc},
#ifdef SPECIFIC_TX_POWER_SUPPORT
	{"PktPwr",						Set_AP_PKT_PWR},
#endif /* SPECIFIC_TX_POWER_SUPPORT */
	{"AssocReqRssiThres",           Set_AP_ASSOC_REQ_RSSI_THRESHOLD},
	{"KickStaRssiLow",				Set_AP_KickStaRssiLow_Proc},
	{"PtkRekey",					Set_PtkRekey_Proc},
#ifdef OCE_SUPPORT
	{"OceRssiThreshold",			Set_OceRssiThreshold_Proc},
	{"OceAssocRetryDelay",			Set_OceAssocRetryDelay_Proc},
	{"OceFdFrameEnable",			Set_OceFdFrameCtrl_Proc},
	{"OceReducedNeighborReport",	Set_OceReducedNRIndicate_Proc},
	{"OceDownlinkAvailCap",			Set_OceDownlinkAvailCap_Proc},
	{"OceUplinkAvailCap",			Set_OceUplinkAvailCap_Proc},
	{"OceReducedWanEnable",			Set_OceReducedWanEnable_Proc},
	{"OceEspEnable",				Set_OceEspEnable_Proc},
#endif /* OCE_SUPPORT */
#ifdef AP_SCAN_SUPPORT
	{"SiteSurvey",					Set_SiteSurvey_Proc},
	{"AutoChannelSel",				Set_AutoChannelSel_Proc},
#ifdef CONFIG_6G_SUPPORT
	{"PSC_ACS",				Set_PSC_ACS_Proc},
#endif
	{"PartialScan",					Set_PartialScan_Proc},
	{"ACSCheckTime",				Set_AutoChannelSelCheckTime_Proc},
	{"ACSCheckMinTime",				Set_AutoChannelSelCheckMinTime_Proc},
	{"ClearSiteSurvey",					Set_ClearSiteSurvey_Proc},
	{"PartialScanTimerInterval",			Set_PartialScan_Timer_Interval_Proc},
	{"PartialScanNumOfCh",			Set_PartialScan_Num_of_CH_Proc},
	{"ScanSkipList",					Set_Scan_SkipList_Proc},
	{"ScanDwellTime",				Set_Scan_DwellTime_Proc},
	{"DfsUtilization",					Set_Scan_DFS_CH_Utilization_Proc},
#endif /* AP_SCAN_SUPPORT */
#ifdef ZERO_LOSS_CSA_SUPPORT
	{"WcidSkipTx",			Set_WcidSkipTx_Proc},
	{"ApChannelList",			Set_APChannelList_Proc},
	{"CSATriggerCount",		Set_CSATriggerCount_Proc},
	{"ZeroLossEnable",			Set_ZeroPktLossEnable_Proc},
	{"CsaActionFrameEnable",	Set_CsaActionFrameEnable_Proc},
	{"PsQLimit",				Set_StaPsQLimit_Proc},
	{"MacTxEnable",			Set_MacTxEnable_Proc},
	{"CHSWPeriod",			Set_CHSWPeriod_Proc},
	{"ZeroLossStaAdd",		Set_ZeroLossStaAdd_Proc},
	{"ZeroLossStaRemove",		Set_ZeroLossStaRemove_Proc},
	{"StaTimeout",			Set_STA_Tx_Unblock_Timeout_Proc},
#endif /*ZERO_LOSS_CSA_SUPPORT*/
#ifdef TXRX_STAT_SUPPORT
	{"enable_rssi", 		Set_Enable_RSSI_Stats},
	{"enable_txrx_stat",	Set_Enable_Last_Sec_TXRX_Stats},
#endif
	{"ResetCounter",				Set_ResetStatCounter_Proc},
	{"DisConnectSta",				Set_DisConnectSta_Proc},
	{"DisConnectAllSta",			Set_DisConnectAllSta_Proc},
#ifdef DOT1X_SUPPORT
	{"IEEE8021X",					Set_IEEE8021X_Proc},
	{"PreAuth",						Set_PreAuth_Proc},
	{"PMKCachePeriod",				Set_AP_PMKCachePeriod_Proc},
	{"own_ip_addr",					Set_OwnIPAddr_Proc},
	{"EAPifname",					Set_EAPIfName_Proc},
	{"PreAuthifname",				Set_PreAuthIfName_Proc},
	{"RADIUS_Server",				Set_RADIUS_Server_Proc},
	{"RADIUS_Port",					Set_RADIUS_Port_Proc},
	{"RADIUS_Key",					Set_RADIUS_Key_Proc},
	{"DeletePMKID",					Set_DeletePMKID_Proc},
	{"DumpPMKID",					Set_DumpPMKID_Proc},
#ifdef RADIUS_MAC_ACL_SUPPORT
	{"RADIUS_MacAuth_Enable",                       Set_RADIUS_MacAuth_Enable_Proc},
	{"RADIUS_CacheTimeout",                         Set_RADIUS_CacheTimeout_Proc},
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* DOT1X_SUPPORT */
#ifdef DBG
	{"Debug",						Set_Debug_Proc},
	{"DebugCat",					Set_DebugCategory_Proc},
#ifdef DBG_ENHANCE
	{"DebugOption",					Set_DebugOption_Proc},
#endif /* DBG_ENHANCE */
#endif /* DBG */
#ifdef ANTENNA_DIVERSITY_SUPPORT
	{"dbg_tp_deg_th",				set_ant_diversity_dbg_tp_deg_th_proc},
	{"dbg_cn_deg_th",			    set_ant_diversity_dbg_cn_deg_th_proc},
	{"dbg_rx_rate_deg_th",			set_ant_diversity_dbg_rx_rate_deg_th_proc},
	{"dbg_countdown",				set_ant_diversity_dbg_countdown_proc},
	{"dbg_ul_th",					set_ant_diversity_dbg_ul_th_proc},
	{"ant_div_dis",					set_ant_diversity_disable_forcedly},
	{"ant_div_dbg",					set_ant_diversity_debug_log},
	{"ant_div_sel_ant",				set_ant_diversity_select_antenna},
	{"ant_div_rx_rate_delta_th",	set_ant_diversity_rx_rate_delta_th},
	{"ant_div_read_interval",		set_read_interval_proc},
	{"ant_div_set_cn_con_deg_th",	set_ant_diversity_cn_deg_continuous_th},
	{"ant_div_set_rx_rate_con_deg_th",	set_ant_diversity_rx_rate_deg_continuous_th},
#endif
#ifdef CONFIG_TP_DBG
	{"TPDbgLevel",				Set_TPDbg_Level},
#endif /* CONFIG_TP_DBG */
#ifdef RANDOM_PKT_GEN
	{"TxCtrl",					Set_TxCtrl_Proc},
#endif
#ifdef CSO_TEST_SUPPORT
	{"CsCtrl",					Set_CsCtrl_Proc},
#endif

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
	{"RBIST_SwitchMode",	        Set_RBIST_Switch_Mode},
	{"RBIST_CaptureStart",	        Set_RBIST_Capture_Start},
	{"RBIST_CaptureStatus",	        Get_RBIST_Capture_Status},
	{"RBIST_RawDataProc",           Get_RBIST_Raw_Data_Proc},
	{"RBIST_IQDataProc",            Get_RBIST_IQ_Data_Proc},
	{"WirelessInfo",                Get_System_Wireless_Info},
#endif/* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

#if defined(PHY_ICS_SUPPORT)
	{"PhyIcs_Start",                Set_PhyIcs_Start},
#endif /* defined(PHY_ICS_SUPPORT) */

	{"IRR_ADC",                     Set_IRR_ADC},
	{"IRR_RxGain",                  Set_IRR_RxGain},
	{"IRR_TTG",                     Set_IRR_TTG},
	{"IRR_TTGOnOff",                Set_IRR_TTGOnOff},
#ifdef OFFCHANNEL_SCAN_FEATURE
	{"ApScanChannel",				Set_ApScan_Proc},
	{"ApScanResults",				Set_ScanResults_Proc},
#endif
#ifdef MT_DFS_SUPPORT /* Jelly20150301 */
	{"ShowDfsCh",                   Set_DfsChannelShow_Proc},
	{"ShowDfsBw",                   Set_DfsBwShow_Proc},
	{"ShowDfsRDMode",               Set_DfsRDModeShow_Proc},

	{"ShowDfsRegion",               Set_DfsRDDRegionShow_Proc},
	/*for non 7615 && non 7622*/
	{"RadarDetectMode",             Set_RadarDetectMode_Proc},
	/*for 7615 || 7622*/
	{"RadarDetectStart",            Set_RadarDetectStart_Proc},
	{"RadarDetectStop",             Set_RadarDetectStop_Proc},
	{"ByPassCac",                   Set_ByPassCac_Proc},
	{"ShowDfsNOP",					Show_DfsNonOccupancy_Proc},
	{"DfsNOPClean",					Set_DfsNOP_Proc},
	{"RDDReport",					Set_RDDReport_Proc},
	{"TriggerRdd",					Trigger_RDD_Event},

	/* DFS zero wait */
	{"DfsZeroWaitCacTime",          Set_DfsZeroWaitCacTime_Proc},
	{"DfsDedicatedBwCh",			Set_DedicatedBwCh_Proc},
	{"DfsModifyChList",				Set_ModifyChannelList_Proc},
	{"DfsDynamicCtrl",				Set_DfsZeroWaitDynamicCtrl_Proc},
	{"DfsForceNOP",					Set_DfsZeroWaitNOP_Proc},
	{"DfsTargetCh",					Set_DfsTargetCh_Proc},
	{"DfsZeroWaitEnable",		    Set_DfsZeroWaitEnable_Proc},
	{"DfsChSelPrefer",		    Set_DfsChSelPrefer_Proc},
#ifdef DFS_ZEROWAIT_SUPPORT
	{"ZWDfsChannelSwitch",	 Set_ZWDfsChannelSwitch_Proc},
#endif

#ifdef DFS_ADJ_BW_ZERO_WAIT
	{"DfsBypassNop",				Set_DfsBypassNop_Proc},
#endif

#ifdef BACKGROUND_SCAN_SUPPORT
	/* Dedicated RX */
	{"DfsRxCtrl", 			set_dfs_dedicated_rx_proc},
	{"DfsRxHist", 			set_dedicated_rx_hist_proc},
#endif

#ifdef IPI_SCAN_SUPPORT
	{"IPIScanCtrl", 			set_ipi_scan_ctrl_proc},
	{"IPIScanHist", 			set_ipi_scan_hist_proc},
#endif

	{"RadarDbgLogConfig",			set_radar_dbg_log_config_proc},
	{"RadarMinLPN",					set_radar_min_lpn_proc},
	{"RadarThresholdParam",			set_radar_thres_param_proc},
	{"RadarPulseThresholdParam",	set_radar_pls_thres_param_proc},
	{"RadarPulsePattern",			set_radar_test_pls_pattern_proc},
#endif /* MT_DFS_SUPPORT */

#if defined(BB_SOC) && defined(TCSUPPORT_WLAN_SW_RPS)
	{"rxThreshold", Set_RxMaxTraffic_Proc},
	{"rxDetectFlag", Set_rx_detect_flag_Proc},
#endif
#if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)
#ifdef CARRIER_DETECTION_SUPPORT
	{"CarrierDetect",				Set_CarrierDetect_Proc},
	{"CarrierCriteria",				Set_CarrierCriteria_Proc},
	{"CarrierReCheck",				Set_CarrierReCheck_Proc},
	{"CarrierGoneThreshold",			Set_CarrierGoneThreshold_Proc},
	{"CarrierDebug",				Set_CarrierDebug_Proc},
	{"Delta",						Set_CarrierDelta_Proc},
	{"DivFlag",						Set_CarrierDivFlag_Proc},
	{"CarrThrd",					Set_CarrierThrd_Proc},
	/* v2 functions */
	{"SymRund",					Set_CarrierSymRund_Proc},
	{"CarrMask",					Set_CarrierMask_Proc},
#endif /* CARRIER_DETECTION_SUPPORT */
#endif /* defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT) */

#ifdef CONFIG_ATE
	{"ATE", SetATE},
	{"ATERXUSER", SetATERxUser},
	{"ATEMPSDUMP", SetATEMPSDump},
	{"ATEMPSPHY", SetATEMPSPhyMode},
	{"ATEMPSRATE", SetATEMPSRate},
	{"ATEMPSPATH", SetATEMPSPath},
	{"ATEMPSLEN", SetATEMPSPayloadLen},
	{"ATEMPSTXCNT", SetATEMPSPktCnt},
	{"ATEMPSTXPWR", SetATEMPSPwr},
	{"ATEMPSNSS", SetATEMPSNss},
	{"ATEMPSPKTBW", SetATEMPSPktBw},
	{"ATEMPSTXSTART", SetATEMPSStart},
	{"ATELOGEN", SetATELOGEnable},
	{"ATELOGDUMP", SetATELOGDump},
	{"ATEMACTRX", SetATEMACTRx},
	{"ATETXSTREAM", SetATETxStream},
	{"ATERXSTREAM", SetATERxStream},
	{"ATETXSENABLE", SetATETxSEnable},
	{"ATERXFILTER", SetATERxFilter},
	{"ATELOGDIS", SetATELOGDisable},
	{"ATEDEQCNT", SetATEDeqCnt},
	{"ATEQID", SetATEQid},
	{"ATEDA", SetATEDa},
	{"ATESA", SetATESa},
	{"ADCDump", SetADCDump},
	{"ATEBSSID", SetATEBssid},
	{"ATECHANNEL", SetATEChannel},
	{"ATEDUTYCYCLE", set_ate_duty_cycle},
	{"ATEPKTTXTIME", set_ate_pkt_tx_time},
	{"ATECTRLBANDIDX", set_ate_control_band_idx},
	{"ATERXSTAT", set_ate_show_rx_stat},
	{"ATERXSTATRESET", set_ate_rx_stat_reset},
#ifdef DOT11_VHT_AC
	{"ATECHANNELEXT", set_ate_channel_ext},
	{"ATESTARTTXEXT", set_ate_start_tx_ext},
#endif /* DOT11_VHT_AC */
	{"ATEINITCHAN", SetATEInitChan},
#ifdef RTMP_TEMPERATURE_CALIBRATION
	{"ATETEMPCAL", SetATETempCal},
	{"ATESHOWTSSI",	SetATEShowTssi},
#endif /* RTMP_TEMPERATURE_CALIBRATION */
	{"ATETXPOW0", SetATETxPower0},
	{"ATETXPOW1", SetATETxPower1},
	{"ATETXPOW2", SetATETxPower2},
	{"ATETXPOW3", SetATETxPower3},
	{"ATEFORCETXPOWER", SetATEForceTxPower},
	{"ATETXPOWEVAL", SetATETxPowerEvaluation},
	{"ATETXANT", SetATETxAntenna},
	{"ATERXANT", SetATERxAntenna},
	{"ATETXFREQOFFSET", SetATETxFreqOffset},
	{"ATETXBW", SetATETxBw},
	{"ATETXLEN", SetATETxLength},
	{"ATETXCNT", SetATETxCount},
	{"ATETXMCS", SetATETxMcs},
	{"ATETXNSS", SetATETxNss},
	{"ATEVHTNSS", SetATETxNss},
	{"ATETXLDPC", SetATETxLdpc},
	{"ATETXSTBC", SetATETxStbc},
	{"ATETXMODE", SetATETxMode},
	{"ATETXGI", SetATETxGi},
	{"ATERETRY", set_ate_retry},
#if defined(DOT11_HE_AX)
	{"ATETXMAXPE", set_ate_max_pe},
	{"ATERUINFO", set_ate_ru_info},
	{"ATEMUAID", set_ate_ru_rx_aid},
	{"ATETXMTHD", set_ate_tx_policy},
#endif
	{"ATERXFER", SetATERxFer},
	{"ATERRF", SetATEReadRF},
#if (!defined(RTMP_RF_RW_SUPPORT)) && (!defined(RLT_RF))
	{"ATEWRF1",	SetATEWriteRF1},
	{"ATEWRF2", SetATEWriteRF2},
	{"ATEWRF3", SetATEWriteRF3},
	{"ATEWRF4",	SetATEWriteRF4},
#endif /* (!defined(RTMP_RF_RW_SUPPORT)) && (!defined(RLT_RF)) */
	{"ATELDE2P", SetATELoadE2p},
	{"ATERE2P", SetATEReadE2p},
#ifdef LED_CONTROL_SUPPORT
#endif /* LED_CONTROL_SUPPORT */
	{"ATEAUTOALC", SetATEAutoAlc},
	{"ATETEMPSENSOR", SetATETempSensor},
	{"ATEIPG", SetATEIpg},
	{"ATEPAYLOAD", SetATEPayload},
	{"ATEFIXEDPAYLOAD", SetATEFixedPayload},
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
	{"ATETxBfInit",              SetATETxBfDutInitProc},
	{"ATETxBfGdInit",            SetATETxBfGdInitProc},
	{"ATETxBfChanProfileUpdate", SetATETxBfChanProfileUpdate},
	{"ATETXBF",                  SetATETXBFProc},
	{"ATETXSOUNDING",            SetATETxSoundingProc},
	{"ATEIBfGdCal",              SetATEIBfGdCal},
	{"ATEIBfInstCal",            SetATEIBfInstCal},
	{"ATETxBfLnaGain",           SetATETxBfLnaGain},
	{"ATEIBfProfileConfig",      SetATEIBfProfileUpdate},
	{"ATEEBfProfileConfig",      SetATEEBfProfileConfig},
	{"ATETxBfProfileRead",       SetATETxBfProfileRead},
	{"ATETxPacketWithBf",        SetATETxPacketWithBf},
	{"ATEIBFPhaseE2pUpdate",     SetATETxBfPhaseE2pUpdate},
	{"ATEIBFPhaseComp",          SetATEIBfPhaseComp},
	{"ATEIBFPhaseVerify",        SetATEIBfPhaseVerify},
	{"ATEConTxETxBfGdProc",      SetATEConTxETxBfGdProc},
	{"ATEConTxETxBfInitProc",    SetATEConTxETxBfInitProc},
	{"ATESPE",                   SetATESpeIdx},
	{"ATETXEBF",                 SetATEEBfTx},
	{"ATEEBFCE",                 SetATEEBFCE},
	{"ATEEBFCEInfo",             SetATEEBFCEInfo},
	{"ATEEBFCEHELP",             SetATEEBFCEHelp},
#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */
	{"ATETTR", SetATETtr},
	{"ATESHOW", SetATEShow},
	{"ATEHELP", SetATEHelp},
#ifdef CONFIG_QA
	{"TxStop", SetTxStop},
	{"RxStop", SetRxStop},
#ifdef DBG
	{"EERead", SetEERead},
	{"EEWrite",	SetEEWrite},
	{"BBPRead", SetBBPRead},
	{"BBPWrite", SetBBPWrite},
#endif /* DBG */
#endif /* CONFIG_QA */


#endif /* CONFIG_ATE */

#ifdef APCLI_SUPPORT
	{"ApCliEnable",				Set_ApCli_Enable_Proc},
	{"ApCliSsid",					Set_ApCli_Ssid_Proc},
	{"ApCliBssid",					Set_ApCli_Bssid_Proc},
	{"ApCliAuthMode",				Set_SecAuthMode_Proc},
	{"ApCliEncrypType",			Set_SecEncrypType_Proc},
	{"ApCliDefaultKeyID",			Set_SecDefaultKeyID_Proc},
	{"ApCliWPAPSK",				Set_SecWPAPSK_Proc},
	{"ApCliKey1",					Set_SecKey1_Proc},
	{"ApCliKey2",					Set_SecKey2_Proc},
	{"ApCliKey3",					Set_SecKey3_Proc},
	{"ApCliKey4",					Set_SecKey4_Proc},
	{"ApCliTxMode",					Set_ApCli_TxMode_Proc},
	{"ApCliTxMcs",					Set_ApCli_TxMcs_Proc},
	{"ApCliPwr",					Set_ApCli_PwrSet_Proc},
	{"ApCliSendPsPoll",					Set_ApCli_SendPsPoll_Proc},
	{"ApCliTransDisableSupported",					Set_ApCli_Trans_Disable_Proc},
	{"ApCliOCVSupport",                             Set_apcli_ocv_support_proc},
	{"ApCliPESupport",                             Set_apcli_pe_support_proc},
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	{"ApCliTwtParams",				Set_ApCli_Twt_Proc},
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
#ifdef APCLI_CONNECTION_TRIAL
	/*
	 for Trial the root AP which locates on another channel
	 what if the connection is ok, it will make BSSID switch to the new channel.
	*/
	{"ApCliTrialCh",				Set_ApCli_Trial_Ch_Proc},
#endif /* APCLI_CONNECTION_TRIAL */
#ifdef DBDC_MODE
	{"ApCliWirelessMode",					Set_ApCli_WirelessMode_Proc},
#endif /*DBDC_MODE*/
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	{"ApCliAutoConnect",			Set_ApCli_AutoConnect_Proc},
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#ifdef WPA_SUPPLICANT_SUPPORT
	{"ApCliWpaSupport",					Set_ApCli_Wpa_Support},
	{"ApCliIEEE1X",					Set_ApCli_IEEE8021X_Proc},
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
	{"MACRepeaterEn",			    Set_ReptMode_Enable_Proc},
	{"CliLinkMAP",                  Set_Cli_Link_Map_Proc},
#endif /* MAC_REPEATER_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
	{"bhbss",						Set_Bh_Bss_Proc},
	{"fhbss",						Set_Fh_Bss_Proc},
	{"mapEnable",					Set_Map_Proc},
	{"MapChannel",					Set_Map_Channel_Proc},
#ifdef MAP_R2
	{"mapR2Enable",					Set_MapR2_Proc},
	{"ts_bh_primary_vid",			Set_Map_Bh_Primary_Vid_Proc},
	{"ts_bh_primary_pcp",			Set_Map_Bh_Primary_Pcp_Proc},
	{"ts_bh_vid",					Set_Map_Bh_Vid_Proc},
	{"ts_fh_vid",					Set_Map_Fh_Vid_Proc},
	{"transparent_vid",				Set_Map_Transparent_Vid_Proc},
#endif
#ifdef MAP_R3
	{"mapR3Enable",					Set_MapR3_Proc},
#endif
#ifdef MAP_R4
	{"mapR4Enable",					Set_MapR4_Proc},
#endif
#ifdef MAP_TS_TRAFFIC_SUPPORT
	{"mapTSEnable",					Set_MapTS_Proc},
#endif
#ifdef MAP_BL_SUPPORT
	{"BlAdd",						Set_BlackList_Add},
	{"BlDel",						Set_BlackList_Del},
	{"BlShow",						Set_BlackList_Show},
#endif /*  MAP_BL_SUPPORT */
#endif /* CONFIG_MAP_SUPPORT */
	{"QuickChannelSwitch",			Set_Quick_Channel_Switch_En_Proc},
#ifdef MWDS
	{"ApCliMWDS",				Set_ApCli_MWDS_Proc},
#endif /* MWDS */

#ifdef RADIUS_MAC_AUTH_SUPPORT
	{"radius_auth_enable",		Set_Radius_Mac_Auth_Policy_Proc},
#endif

#ifdef DYNAMIC_VLAN_SUPPORT
	{"dvlan",						Set_Dvlan_Proc},
#endif

#ifdef WSC_AP_SUPPORT
	{"ApCliWscSsid",				Set_AP_WscSsid_Proc},
	{"ApCliWscScanMode",			Set_ApCli_WscScanMode_Proc},
#endif /* WSC_AP_SUPPORT */

#ifdef APCLI_SUPPORT
	{"ApCliCertEnable",			Set_ApCli_Cert_Enable_Proc},
	{"ApCliCertAggLimit", 		Set_ApCli_Cert_Agg_Limit_Proc},
#endif /* APCLI_SUPPORT */

	/* Add for APCLI PMF 5.3.3.3 option test item. (Only Tx De-auth Req. and make sure the pkt can be Encrypted) */
	{"ApCliTxDeAuth",				ApCliTxDeAuth},

#ifdef DOT11W_PMF_SUPPORT
	{"ApCliPMFMFPC",                                         Set_ApCliPMFMFPC_Proc},
	{"ApCliPMFMFPR",                                         Set_ApCliPMFMFPR_Proc},
	{"ApCliPMFSHA256",                                      Set_ApCliPMFSHA256_Proc},
#endif /* DOT11W_PMF_SUPPORT */
#ifdef ROAMING_ENHANCE_SUPPORT
	{"RoamingEnhance",			Set_RoamingEnhance_Enable_Proc},
#endif /* ROAMING_ENHANCE_SUPPORT */
#ifdef DOT11_SAE_SUPPORT
	{"ApCliSAEGroup",                                         set_apcli_sae_group_proc},
	{"ApCliSAEPK",                                            Set_apcli_sae_pk_proc},
	{"ApCliSAEPKOnly",                                        Set_apcli_sae_pk_only_proc},
#endif /* DOT11_SAE_SUPPORT */
#ifdef CONFIG_OWE_SUPPORT
	{"ApCliOWEGroup",                                         set_apcli_owe_group_proc},
#endif /* CONFIG_OWE_SUPPORT */
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	{"ApCliDelPMKIDList",                                         set_apcli_del_pmkid_list},
#endif /* defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT) */
#endif /* APCLI_SUPPORT */
#ifdef WSC_AP_SUPPORT
#ifdef CON_WPS
	{"ConWpsApCliMode",			Set_ConWpsApCliMode_Proc},
	{"ConWpsApCliDisabled",                     Set_ConWpsApCliDisabled_Proc},
	{"ConWpsApDisabled",                     Set_ConWpsApDisabled_Proc},
	{"ConWpsApcliPreferIface",		Set_ConWpsApcliAutoPreferIface_Proc},
#endif /* CON_WPS */
	{"WscConfMode",				Set_AP_WscConfMode_Proc},
	{"WscConfStatus",				Set_AP_WscConfStatus_Proc},
	{"WscMode",					Set_AP_WscMode_Proc},
	{"WscStatus",					Set_WscStatus_Proc},
	{"WscGetConf",				Set_AP_WscGetConf_Proc},
	{"WscPinCode",				Set_AP_WscPinCode_Proc},
	{"WscStop",		Set_WscStop_Proc},
	{"WscGenPinCode",	Set_WscGenPinCode_Proc},
	{"WscVendorPinCode",            Set_WscVendorPinCode_Proc},
	{"WscSecurityMode",				Set_AP_WscSecurityMode_Proc},
	{"WscMultiByteCheck",			Set_AP_WscMultiByteCheck_Proc},
	{"WscVersion",					Set_WscVersion_Proc},
#ifdef VENDOR_FEATURE6_SUPPORT
	/* HEX : 32 Length */
	{"WscUUID_E",					Set_WscUUID_HEX_E_Proc},
	/* 37 Length */
	{"WscUUID_Str",					Set_WscUUID_STR_Proc},
#endif /* VENDOR_FEATURE6_SUPPORT */
#ifdef WSC_V2_SUPPORT
	{"WscV2Support",				Set_WscV2Support_Proc},
	{"WscVersion2",				Set_WscVersion2_Proc},
	{"WscExtraTlvTag",				Set_WscExtraTlvTag_Proc},
	{"WscExtraTlvType",				Set_WscExtraTlvType_Proc},
	{"WscExtraTlvData",			Set_WscExtraTlvData_Proc},
	{"WscSetupLock",				Set_WscSetupLock_Proc},
	{"WscFragment",					Set_WscFragment_Proc},
	{"WscFragmentSize",			Set_WscFragmentSize_Proc},
	{"WscMaxPinAttack",			Set_WscMaxPinAttack_Proc},
	{"WscSetupLockTime",			Set_WscSetupLockTime_Proc},
#endif /* WSC_V2_SUPPORT */
	{"WscAutoTriggerDisable",		Set_WscAutoTriggerDisable_Proc},
#endif /* WSC_AP_SUPPORT */
#ifdef UAPSD_SUPPORT
	{"UAPSDCapable",				Set_UAPSD_Proc},
#endif /* UAPSD_SUPPORT */
#ifdef IGMP_SNOOP_SUPPORT
	{"IgmpSnEnable",				Set_IgmpSn_Enable_Proc},
	{"IgmpAdd",					Set_IgmpSn_AddEntry_Proc},
	{"IgmpDel",					Set_IgmpSn_DelEntry_Proc},
#ifdef IGMP_SNOOPING_DENY_LIST
	{"IgmpSnoopDeny",			Set_IgmpSn_Deny_Proc},
#endif
#ifdef IGMP_TVM_SUPPORT
	{"IgmpSnExemptIP",				Set_IgmpSn_BlackList_Proc},
	{"IgmpSnAgeOut",				Set_IgmpSn_AgeOut_Proc},
#endif /* IGMP_TVM_SUPPORT */
	{"IgmpSnAllowNonMembEnable",			Set_IgmpSn_Allow_Non_Memb_Enable_Proc},
	{"IgmpFloodingCIDR",				Set_Igmp_Flooding_CIDR_Proc},
#endif /* IGMP_SNOOP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	{"mgmrateset",				set_mgm_rate_proc},
	{"suprateset",				set_suprateset_proc},
	{"htsuprateset",				set_htsuprateset_proc},
	{"vhtsuprateset",				set_vhtsuprateset_proc},
#ifdef DOT11_HE_AX
	{"hesuprateset",				set_hesuprateset_proc},
#endif
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
	{"McastType",					Set_McastType},
#endif /* MCAST_VENDOR10_CUSTOM_FEATURE */
	{"McastPhyMode",				Set_McastPhyMode},
	{"McastMcs",					Set_McastMcs},
#endif /* MCAST_RATE_SPECIFIC */
#ifdef HIGHPRI_RATE_SPECIFIC
	{"HighPriPhyMode",				Set_HighPriPhyMode},
	{"HighPriMcs",					Set_HighPriMcs},
#endif /* HIGHPRI_RATE_SPECIFIC */

#endif /* CONFIG_AP_SUPPORT */
	{"FixedTxMode",                 Set_FixedTxMode_Proc},
#ifdef CONFIG_APSTA_MIXED_SUPPORT
	{"OpMode",					Set_OpMode_Proc},
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef DSCP_PRI_SUPPORT
	{"DscpPri",					Set_Dscp_Pri_Proc},
	{"DscpPriEnable",					Set_Dscp_Pri_Enable_Proc},
#endif /*DSCP_PRI_SUPPORT*/

#ifdef TXBF_SUPPORT
	{"TxBfStopReportPoll",		set_txbf_stop_report_poll_proc},
	{"InvTxBfTag",				Set_InvTxBfTag_Proc},
	{"ITxBfDivCal",				Set_ITxBfDivCal_Proc},

	{"ITxBfEn",					Set_ITxBfEn_Proc},
	{"ETxBfEnCond",				Set_ETxBfEnCond_Proc},
	{"ETxBfCodebook",			Set_ETxBfCodebook_Proc},
	{"ETxBfCoefficient",		Set_ETxBfCoefficient_Proc},
	{"ETxBfGrouping",			Set_ETxBfGrouping_Proc},
	{"ETxBfNoncompress",		Set_ETxBfNoncompress_Proc},
	{"ETxBfIncapable",			Set_ETxBfIncapable_Proc},

#ifdef MT_MAC
	{"TxBfTxApply",             Set_TxBfTxApply},
	{"TriggerSounding",			Set_Trigger_Sounding_Proc},
	{"StopSounding",			Set_Stop_Sounding_Proc},
	{"StaRecBfUpdate",          Set_StaRecBfUpdate},
	{"StaRecBfHeUpdate",        set_txbf_he_bf_starec},
	{"StaRecBfRead",            Set_StaRecBfRead},
	{"TxBfAwareCtrl",           Set_TxBfAwareCtrl},
	{"TxBfSetAid",              Set_TxBfAidUpdate},
	{"dynsndEnIntr",            set_dynsnd_en_intr},
	{"HostRepTxLatency",        Set_HostReportTxLatency},
	{"RxFilterDropCtrl",        Set_RxFilterDropCtrlFrame},
	{"CertCfg",                 Set_CertCfg},
#ifdef CFG_SUPPORT_MU_MIMO
	{"dynsndCfgdMcs",           set_dynsnd_cfg_dmcs},
	{"dynsndEnMuIntr",          set_dynsnd_en_mu_intr},
#endif
#ifdef CONFIG_ATE
	{"StaRecCmmUpdate",         Set_StaRecCmmUpdate},
	{"BssInfoUpdate",           Set_BssInfoUpdate},
	{"DevInfoUpdate",           Set_DevInfoUpdate},
	{"ManualAssoc",             SetATEAssocProc},
#endif /* CONFIG_ATE */
	{"TxBfPfmuMemAlloc",        Set_TxBfPfmuMemAlloc},
	{"TxBfPfmuMemRelease",      Set_TxBfPfmuMemRelease},
	{"TxBfPfmuMemAllocMapRead", Set_TxBfPfmuMemAllocMapRead},
	{"TxBfProfileTagHelp",      Set_TxBfProfileTag_Help},
	{"TxBfProfileTagInValid",   Set_TxBfProfileTag_InValid},
	{"TxBfProfileTagPfmuIdx",   Set_TxBfProfileTag_PfmuIdx},
	{"TxBfProfileTagBfType",    Set_TxBfProfileTag_BfType},
	{"TxBfProfileTagBw",        Set_TxBfProfileTag_DBW},
	{"TxBfProfileTagSuMu",      Set_TxBfProfileTag_SuMu},
	{"TxBfProfileTagMemAlloc",  Set_TxBfProfileTag_Mem},
	{"TxBfProfileTagMatrix",    Set_TxBfProfileTag_Matrix},
	{"TxBfProfileTagRURang",    set_txbf_prof_tag_ru_range},
	{"TxBfProfileTagMobCalEn",  set_txbf_prof_tag_mob_cal_en},
	{"TxBfProfileTagSnr",       Set_TxBfProfileTag_SNR},
	{"TxBfProfileTagSmtAnt",    Set_TxBfProfileTag_SmartAnt},
	{"TxBfProfileTagSeIdx",     Set_TxBfProfileTag_SeIdx},
	{"TxBfProfileTagRmsdThrd",  Set_TxBfProfileTag_RmsdThrd},
	{"TxBfProfileTagMcsThrd",   Set_TxBfProfileTag_McsThrd},
	{"TxBfProfileTagTimeOut",   Set_TxBfProfileTag_TimeOut},
	{"TxBfProfileTagDesiredBw", Set_TxBfProfileTag_DesiredBW},
	{"TxBfProfileTagDesiredNc", Set_TxBfProfileTag_DesiredNc},
	{"TxBfProfileTagDesiredNr", Set_TxBfProfileTag_DesiredNr},
	{"TxBfProfileTagRUAlloc",   set_txbf_prof_tag_ru_alloc},
	{"TxBfProfileTagRead",      Set_TxBfProfileTagRead},
	{"TxBfProfileTagWrite",     Set_TxBfProfileTagWrite},
	{"TxBfProfileDataRead",     Set_TxBfProfileDataRead},
	{"TxBfProfileDataWrite",    Set_TxBfProfileDataWrite},
	{"TxBfAngleWrite",          set_txbf_angle_write},
	{"TxBfDSNRWrite",           set_txbf_dsnr_write},
	{"TxBfPFMUDataWrite",       set_txbf_pfmu_data_write},
#ifdef CONFIG_ATE
	{"TxBfProfileData20MAllWrite", Set_TxBfProfileData20MAllWrite},
#endif
	{"TxBfProfilePnRead",       Set_TxBfProfilePnRead},
	{"TxBfProfilePnWrite",      Set_TxBfProfilePnWrite},
	{"TxBfFbRptDbgInfo",        Set_TxBfFbRptDbgInfo},
	{"TxBfTxSndInfo",           Set_TxBfTxSndInfo},
	{"TxBfPlyInfo",             Set_TxBfPlyInfo},
	{"TxBfTxCmd",               Set_TxBfTxCmd},
	{"HeRaMuMetricInfo",        Set_HeRaMuMetricInfo},
#ifdef TXBF_DYNAMIC_DISABLE
	{"TxBfDisable",             Set_TxBfDynamicDisable_Proc},
#endif /* TXBF_DYNAMIC_DISABLE */
	/* Enable/disable TxBF Dynamic Mechanism*/
	{"bfdm",					set_txbf_dynamic_mechanism_proc},
	{"TxBfProfileSwTagWrite",   Set_TxBfProfileSwTagWrite}, /* For BFmee Pseudo PHY */
	{"TxBfSndCnt",              Set_TxBfSndCnt},
#endif /* MT_MAC */

#endif /* TXBF_SUPPORT */
#ifdef VHT_TXBF_SUPPORT
	{"VhtNDPA",				    Set_VhtNDPA_Sounding_Proc},
#endif /* VHT_TXBF_SUPPORT */
	{"mec_ctrl",				set_mec_ctrl}, /* MAC Efficiency Control */
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
	{"wf_fwd",		Set_WifiFwd_Proc},
	{"wf_fwd_down",		Set_WifiFwd_Down},
	{"wf_fwd_acs",	Set_WifiFwdAccessSchedule_Proc},
	{"wf_fwd_hij",	Set_WifiFwdHijack_Proc},
	{"wf_fwd_bpdu", Set_WifiFwdBpdu_Proc},
	{"wf_fwd_rep",	Set_WifiFwdRepDevice},
	{"wf_fwd_show",	Set_WifiFwdShowEntry},
	{"wf_fwd_del",	Set_WifiFwdDeleteEntry},
	{"pkt_src_show",   Set_PacketSourceShowEntry},
	{"pkt_src_del",	Set_PacketSourceDeleteEntry},
	{"wf_fwd_bridge",	Set_WifiFwdBridge_Proc},
#endif /* CONFIG_WIFI_PKT_FWD */

#ifdef MT_MAC
	{"hera",                    Set_Hera_Proc}, /* HERA general purpose */
	{"PtecPerPpduDis",          SetHeraProtectionPerPpduDis},
	{"MuInitRateIntv",          SetHeraMuInitRateInterval},
	{"MuDisSwitchSu",           SetHeraMuDisableSwitchSu},
	{"SingleNssTxEn",           SetHeraSingleNssTxEnable},
#ifdef DYNAMIC_WMM_SUPPORT
	{"DynWmmEnable",            SetDynamicWmmEnableProc},
#endif /* DYNAMIC_WMM_SUPPORT */
#ifdef DBG
	{"FixedRate",				Set_Fixed_Rate_Proc},
	{"FixedRateWoSTA",          Set_Fixed_Rate_WO_STA_Proc},
	{"FixedRateFallback",		Set_Fixed_Rate_With_FallBack_Proc},
	{"FixedRatePerBSS",			Set_Fixed_Rate_PerBSS_Proc},
	{"RaDebug",					Set_RA_Debug_Proc},
	{"FixedHeLtF",              Set_Fixed_HE_LT_F},
	{"FixedMcs",                Set_Fixed_Mcs_Update},
	{"FixedVhtNss",             Set_Fixed_VhtNss_Update},
	{"FixedBw",                 Set_Fixed_BW_Update},
	{"FixedGi",                 Set_Fixed_GI_Update},
	{"FixedEcc",                Set_Fixed_Ecc_Update},
	{"FixedStbc",               Set_Fixed_STBC_Update},
	{"FixedHeLtFUl",            Set_Fixed_UL_HE_LTF_Update},
	{"FixedMcsUl",              Set_Fixed_UL_Mcs_Update},
	{"FixedVhtNssUl",           Set_Fixed_UL_VhtNss_Update},
	{"FixedGiUl",               Set_Fixed_UL_GI_Update},
	{"FixedEccUl",              Set_Fixed_UL_Ecc_Update},
	{"FixedStbcUl",             Set_Fixed_UL_STBC_Update},
	{"AutoRate",                Set_AutoRate_Update},
	{"AutoRateUl",              Set_UL_AutoRate_Update},
	{"AutoRatePerBSS",			Set_AutoRate_PerBss_Update},
	{"FixedSpe",                 Set_Fixed_Spe_Update},
#endif /* DBG */
#endif /* MT_MAC */

#ifdef STREAM_MODE_SUPPORT
	{"StreamMode",				Set_StreamMode_Proc},
	{"StreamModeMac",			Set_StreamModeMac_Proc},
	{"StreamModeMCS",			Set_StreamModeMCS_Proc},
#endif /* STREAM_MODE_SUPPORT */

	{"LongRetry",	Set_LongRetryLimit_Proc},
	{"ShortRetry",	Set_ShortRetryLimit_Proc},
	{"AutoFallBack",		Set_AutoFallBack_Proc},
#ifdef RTMP_MAC_PCI
#ifdef DBG_DIAGNOSE
	{"DiagOpt",					    Set_DiagOpt_Proc},
	{"diag_cond",                   Set_diag_cond_Proc},
#endif /* DBG_DIAGNOSE */
#endif /* RTMP_MAC_PCI */

	{"MeasureReq",					Set_MeasureReq_Proc},
	{"PwrConstraint",				Set_PwrConstraint},
#ifdef TPC_SUPPORT
	{"TpcReq",						Set_TpcReq_Proc},
	{"TpcReqByAddr",				Set_TpcReqByAddr_Proc},
	{"TpcCtrl",						Set_TpcCtrl_Proc},
	{"TpcEn",						Set_TpcEnable_Proc},
#ifdef TPC_MODE_CTRL
	{"Tpcintvl",					Set_TpcInterval_Proc},
	{"TpcMode",					Set_TpcCtrlMode_Proc},
	{"TpcRssiThld",					Set_tpc_RssiThld_Proc},
	{"TpcLnkMgnCtrl",				Set_TpcLinkMargin_Proc},
#endif
#endif /* TPC_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
	{"BcnReq",						Set_BeaconReq_Proc},
	{"BcnReqRandInt",				Set_BeaconReq_RandInt_Proc},
	{"LinkReq",						Set_LinkMeasureReq_Proc},
	{"RrmEnable",					Set_Dot11kRRM_Enable_Proc},
	{"TxReq",						Set_TxStreamMeasureReq_Proc},
	{"LastBeaconRptIndication",                     Set_BeaconReq_LastBcnRptInd_Proc},


	/* only for selftesting and debugging. */
	{"rrm",						Set_RRM_Selftest_Proc},
#endif /* DOT11K_RRM_SUPPORT */
#ifdef CONFIG_DOT11V_WNM
	{"SendBTMReq",					Set_SendBTMReq_Proc},
#endif
	{"RegDomain",					Set_Reg_Domain_Proc},
#ifdef DOT11N_DRAFT3
	{"OBSSScanParam",				Set_OBSSScanParam_Proc},
	{"AP2040Rescan",			    Set_AP2040ReScan_Proc},
	{"HtBssCoex",					Set_HT_BssCoex_Proc},
	{"HtBssCoexApCntThr",			Set_HT_BssCoexApCntThr_Proc},
#endif /* DOT11N_DRAFT3 */
	{"EntryLifeCheck",				Set_EntryLifeCheck_Proc},
	{"StaPowerOffDetect",			Set_StaPowerOffDetect_Proc},
#ifdef DOT11R_FT_SUPPORT
	{"ft",							FT_Ioctl},
	{"ftenable",					Set_FT_Enable},
	{"ftmdid",						Set_FT_Mdid},
	{"ftr0khid",					Set_FT_R0khid},
	{"ftric",						Set_FT_RIC},
	{"ftotd",						Set_FT_OTD},
#endif /* DOT11R_FT_SUPPORT */

#ifdef RTMP_EFUSE_SUPPORT
	{"efuseLoadFromBin",			set_eFuseLoadFromBin_Proc}, /* For backward compatible, the usage is the same as bufferLoadFromBin + bufferWriteBack */
	{"efuseFreeNumber",			    set_eFuseGetFreeBlockCount_Proc},
	{"efuseDump",				    set_eFusedump_Proc},
#ifdef CONFIG_ATE
	{"bufferLoadFromEfuse",		    Set_LoadEepromBufferFromEfuse_Proc},
#ifdef BB_SOC
	{"efuseBufferModeWriteBack",	set_BinModeWriteBack_Proc},
#else
	{"efuseBufferModeWriteBack",	set_eFuseBufferModeWriteBack_Proc}, /* For backward compatible, the usage is the same as bufferWriteBack */
#endif
#endif /* CONFIG_ATE */
#endif /* RTMP_EFUSE_SUPPORT */
	{"bufferLoadFromBin",			Set_LoadEepromBufferFromBin_Proc},
	{"bufferWriteBack",			Set_EepromBufferWriteBack_Proc},
	{"bufferMode",			        Set_bufferMode_Proc},

#ifdef CAL_FREE_IC_SUPPORT
	{"bufferLoadFromCalFree",		Set_LoadCalFreeData_Proc},
	{"CheckCalFree",		        Set_CheckCalFree_Proc},
#endif

	{"CalFreeApply",                SetCalFreeApply},
	{"WriteEffuseRFpara",           SetWriteEffuseRFpara},

#ifdef RTMP_RBUS_SUPPORT
#ifdef LED_CONTROL_SUPPORT
	{"WlanLed",					    Set_WlanLed_Proc},
#endif /* LED_CONTROL_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

#ifdef AP_QLOAD_SUPPORT
	{"qloadclr",					Set_QloadClr_Proc},
	{"qloadalarmtimethres",			Set_QloadAlarmTimeThreshold_Proc}, /* QLOAD ALARM */
	{"qloadalarmnumthres",			Set_QloadAlarmNumThreshold_Proc}, /* QLOAD ALARM */
#endif /* AP_QLOAD_SUPPORT */

	{"ra_interval",					Set_RateAdaptInterval},

	{"tpc",						    set_thermal_protection_criteria_proc},
	{"tpc_duty",					set_thermal_protection_admin_ctrl_duty_proc},
	{"show_tpc_duty",					get_thermal_protection_admin_ctrl_duty_proc},

#ifdef SMART_ANTENNA
	{"sa",						    Set_SmartAnt_Proc},
	{"sa_msc",					    Set_McsStableCnt_Proc},
	{"sa_mcs",					    Set_SA_McsBound_Proc},
	{"sa_sta",					    Set_SA_Station_Proc},
	{"sa_starule",				    Set_SA_StationCandRule_Proc},
	{"sa_mode",					Set_SA_Mode_Proc},
	{"sa_txNss",					set_SA_txNss_Proc},
	{"sa_ant",					    Set_SA_StaticAntPair_Proc},
	{"sa_agsp",					Set_SA_AGSP_Proc},
	{"sa_tseq",					Set_SA_TrainSeq_Proc},
	{"sa_tdelay",					Set_SA_TrainDelay_Proc},
	{"sa_tcond",					Set_SA_TrainCond_Proc},
	{"sa_rssivar",					Set_SA_RssiVariance_Proc},
	{"sa_rssith",					Set_SA_RssiThreshold_Proc},
	{"sa_skipconf",				Set_SA_SkipConfirmStage_Proc},
	{"sa_tcand",					Set_SA_AntCand_Proc},
	{"sa_tp",					    Set_TestPeriod_Proc},
	{"sa_tc",					    Set_MaxAntennaTry_Proc},
	{"sa_dbg",					    Set_DbgLogOn_Proc},
#endif /* SMART_ANTENNA // */

	{"memdebug",					Set_MemDebug_Proc},

#ifdef CONFIG_AP_SUPPORT
	{"pslifetime",					Set_PowerSaveLifeTime_Proc},

#ifdef MBSS_SUPPORT
	{"MBSSWirelessMode",			Set_MBSS_WirelessMode_Proc},
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef P2P_SUPPORT
	{"P2pCliEnable",				Set_P2pCli_Enable_Proc},
	{"P2pCliSsid",					Set_P2pCli_Ssid_Proc},
	{"P2pCliBssid",				    Set_P2pCli_Bssid_Proc},
	{"P2pCliAuthMode",				Set_P2pCli_AuthMode_Proc},
	{"P2pCliEncrypType",			Set_P2pCli_EncrypType_Proc},
	{"P2pCliDefaultKeyID",			Set_P2pCli_DefaultKeyID_Proc},
	{"P2pCliWPAPSK",				Set_P2pCli_WPAPSK_Proc},
	{"P2pCliKey1",					Set_P2pCli_Key1_Proc},
	{"P2pCliKey2",					Set_P2pCli_Key2_Proc},
	{"P2pCliKey3",					Set_P2pCli_Key3_Proc},
	{"P2pCliKey4",					Set_P2pCli_Key4_Proc},
	{"P2pCliTxMode",				Set_P2pCli_TxMode_Proc},
	{"P2pCliTxMcs",				    Set_P2pCli_TxMcs_Proc},
#ifdef WSC_AP_SUPPORT
	{"P2pCliWscSsid",				Set_P2pCli_WscSsid_Proc},
#endif /* WSC_AP_SUPPORT */
	{"P2pOpMode",				    Set_P2p_OpMode_Proc},
	{"p2pEnable",					Set_P2P_Enable},
	{"p2pLisCh",					Set_P2P_Listen_Channel},
	{"p2pOpCh",					Set_P2P_Operation_Channel},
	{"p2pGoInt",					Set_P2P_GO_Intent},
	{"p2pDevName",					Set_P2P_Device_Name},
	{"p2pWscMode",					Set_P2P_WSC_Mode},
	{"p2pWscConf",					Set_P2P_WSC_ConfMethod},
	{"p2pNoACnt",					Set_P2P_NoA_Count},
	{"p2pNoADuration",				Set_P2P_NoA_Duration},
	{"p2pNoAInv",					Set_P2P_NoA_Interval},
	{"p2pExtLst",					Set_P2P_Extend_Listen},
	{"p2pExtLstPrd",				Set_P2P_Extend_Listen_Periodic},
	{"p2pExtLstInv",				Set_P2P_Extend_Listen_Interval},
	{"p2pIntraBss",				Set_P2P_Intra_Bss},
	{"p2pScan",					Set_P2P_Scan},
	{"p2pProv",					Set_P2P_Provision_Proc},
	{"p2pInv",						Set_P2P_Invite_Proc},
	{"p2pDevDisc",					Set_P2P_Device_Discoverability_Proc},
	{"p2pLink",					Set_P2P_Connect_GoIndex_Proc},
	{"p2pCfg",						Set_P2P_Print_Cfg},
	{"p2pTab",						Set_P2P_Print_GroupTable_Proc},
	{"p2pPerTab",					Set_P2P_Print_PersistentTable_Proc},
	{"p2pStat",					Set_P2P_State_Proc},
	{"p2pReset",					Set_P2P_Reset_Proc},
	{"p2pDefConfMthd",				Set_P2P_Default_Config_Method_Proc},
#endif /* P2P_SUPPORT */

#ifdef SNIFFER_SUPPORT
	{"MonitorMode",				    Set_MonitorMode_Proc},
	{"MonitorFilterSize",			Set_MonitorFilterSize_Proc},
	{"MonitorFrameType",			Set_MonitorFrameType_Proc},
	{"MonitorMacFilter",			Set_MonitorMacFilter_Proc},
	{"MonitorMacFilterOff",			Set_MonitorMacFilterOff_Proc},
#endif /* SNIFFER_SUPPORT */

#ifdef SNIFFER_RADIOTAP_SUPPORT
	{"SnifferBox",				Set_SnifferBox_Proc},
#endif
	{"hwautodebug",				Set_Hw_Auto_Debug_Proc},
#ifdef SINGLE_SKU
	{"ModuleTxpower",				Set_ModuleTxpower_Proc},
#endif /* SINGLE_SKU */

#ifdef DOT11W_PMF_SUPPORT
	{"PMFMFPC",                     Set_PMFMFPC_Proc},
	{"PMFMFPR",                     Set_PMFMFPR_Proc},
	{"PMFSHA256",                   Set_PMFSHA256_Proc},
	{"PMFSA_Q",			Set_PMFSA_Q_Proc},
#endif /* DOT11W_PMF_SUPPORT */

#ifdef MICROWAVE_OVEN_SUPPORT
	{"MO_FalseCCATh",				Set_MO_FalseCCATh_Proc},
#endif /* MICROWAVE_OVEN_SUPPORT */

	{"no_bcn",					set_no_bcn},
	{"assignWcid",				set_assign_wcid_proc},
#ifdef AUTOMATION
	{"txs_test",				set_txs_test},
	{"txs_test_result",			set_txs_test_result},
	{"rxv_test",				set_rxv_test},
	{"rxv_test_result",			set_rxv_test_result},
	{"ApMbssCheck",				set_ap_mbss_check_proc},
	{"ApMbssGetResult",			set_ap_mbss_get_result_proc},
#ifdef APCLI_SUPPORT
#ifdef UAPSD_SUPPORT
	{"ApCliUAPSDCapable",		set_ApCli_UAPSD_Proc},
	{"ApCliAPSDCapable",		set_ApCli_UAPSD_Proc}, /* backward compatible with old SDK */
	{"ApCliAPSDAC",			set_ApCli_APSDAC_Proc},
	{"ApCliMaxSPLength",		set_ApCli_MaxSPLength_Proc},
	{"ApCliBlockPkt",			set_ApCli_Block_Proc},
	{"ApCliCheckPkt",			set_ApCli_Rx_Packet_Check_Proc},
#endif /* UAPSD_SUPPORT */
#endif /* APCLI_SUPPORT */
#ifdef HDR_TRANS_RX_SUPPORT
	{"hdr_translate_blist",		set_hdr_translate_blist},
#endif /* HDR_TRANS_RX_SUPPORT */
	{"txrx_dbg_cfg",			set_txrx_dbg_cfg},
#endif /* AUTOMATION */
	{"rf",						SetRF},
	{"tssi_enable", set_tssi_enable},
#ifdef DYNAMIC_VGA_SUPPORT
	{"DyncVgaEnable", Set_DyncVgaEnable_Proc},
	{"fc_hth", set_false_cca_hi_th},
	{"fc_lth", set_false_cca_low_th},
#endif /* DYNAMIC_VGA_SUPPORT */
#ifdef MT_MAC
	{"TmrVer",                  setTmrVerProc},
	{"TmrEnable",				setTmrEnableProc},/* 0: disalbe, 1: initialiter, 2: responser. */
	{"TmrCal",                  SetTmrCalProc},
#ifdef CONFIG_HOTSPOT
	{"Anqp",				Send_ANQP_Req_For_Test},
#endif
	/* support CR4 cmds */
	{"cr4_query",                   set_cr4_query},
	{"cr4_set",                     set_cr4_set},
	{"cr4_capability",              set_cr4_capability},
	{"cr4_debug",                   set_cr4_debug},
	{"ReCal",                       set_re_calibration},
	{"ThermalDbgCmd",               set_thermal_dbg_cmd},
	{"fwlog", set_fw_log},
	{"fw_dbg", set_fw_dbg},
	{"isrcmd",		set_isr_cmd},
	{"txop",		set_txop_cfg},
	{"rts",		set_rts_cfg},
	{"fwset",                   set_fw_cmd},
	{"fwget",                   get_fw_cmd},

#ifdef FW_DUMP_SUPPORT
	{"fwdump_maxsize", set_fwdump_max_size},
	{"fwdump_path", set_fwdump_path},
	{"fwdump_print", fwdump_print},
#endif
	{"protect", set_manual_protect},
	{"rdg", set_manual_rdg},
	{"cca_en", set_cca_en},
#ifdef DBG
	{"DumpTime",                Set_AP_DumpTime_Proc},/* only for development purpose!! iwpriv ra0 set */
	{"BcnStateCtrl",            Set_BcnStateCtrl_Proc},/* only for development purpose!! iwpriv ra0 set */
#endif
#ifdef MAC_REPEATER_SUPPORT
	{"DumpRxSB",				set_dump_rx_ba_scoreboard_proc},
#endif /* MAC_REPEATER_SUPPORT */
	{"get_thermal_sensor", Set_themal_sensor},
#ifdef CFG_SUPPORT_MU_MIMO
	{"hqa_mu_cal_init_mcs",         hqa_mu_cal_init_mcs},
	{"hqa_mu_cal_lq",               hqa_mu_cal_lq},
	{"hqa_su_cal_lq",               hqa_su_cal_lq},
	{"hqa_mu_set_snr_offset",       hqa_mu_set_snr_offset},
	{"hqa_mu_set_zero_nss",         hqa_mu_set_zero_nss},
	{"hqa_mu_set_speedup_lq",       hqa_mu_set_speedup_lq},
	{"hqa_mu_set_mu_tbl",           hqa_mu_set_mu_table},
	{"hqa_mu_set_group",            hqa_mu_set_group},
	{"hqa_mu_set_enable",           hqa_mu_set_enable},
	{"hqa_mu_set_gid_up",           hqa_mu_set_gid_up},
	{"hqa_mu_set_trigger_mu_tx",    hqa_mu_set_trigger_mu_tx},
	{"hqa_mu_reset_murx_cnt",       hqa_mu_reset_murx_cnt},
	{"hqa_mu_reset_mutx_cnt",       hqa_mu_reset_mutx_cnt},

	/* jeffrey, 20141116 */
	/* the followings are relative to MU debbuging/verification, and not use for ATE */
	{"set_mu_profile",              SetMuProfileProc},              /* set MU profile */
	{"set_mu_grouptbl",             SetGroupTblEntryProc},          /* set group table entry */
	{"set_mu_clustertbl",           SetClusterTblEntryProc},        /* set cluster table entry */
	{"set_mu_enable",               SetMuEnableProc},           /* set MU enable or disable */
	{"set_mu_groupuserthreshold",   SetGroupUserThresholdProc},     /* set group threshold */
	{"set_mu_groupnssthreshold",    SetGroupNssThresholdProc},      /* set group NSS */
	{"set_mu_txreqmintime",         SetTxReqMinTimeProc},           /* set TX req min. time */
	{"set_mu_calcinitmcs",          SetCalculateInitMCSProc},       /* set calculate init MCS */
	{"set_mu_sunsscheck",           SetSuNssCheckProc},             /* set enable or disable NSS check */
	{"set_mu_txopdefault",          SetTxopDefaultProc},            /* set MU enable or disable */
	{"set_mu_sulossthreshold",      SetSuLossThresholdProc},        /* set SU loss threshold */
	{"set_mu_mugainthreshold",      SetMuGainThresholdProc},        /* set MU gain threshold */
	{"set_mu_secondaryacpolicy",    SetSecondaryAcPolicyProc},      /* set secondary AC policy */
	{"set_mu_grouptbldmcsmask",     SetGroupTblDmcsMaskProc},       /* set group table DMCS mask enable or disable */
	{"set_mu_maxgroupsearchcnt",    SetMaxGroupSearchCntProc},      /* set Max group search count */
	/* the followings are relative to trigger MU-flow test command */
	{"set_mu_send_gid_mgmt_frame",  SetTriggerGIDMgmtFrameProc},    /* set trigger GID mgmt. frame */
	{"set_mu_trigger_mutx",    SetTriggerMuTxProc},     /* set trigger MU TX */
	{"set_mu_trigger_degroup",      SetTriggerDegroupProc},     /* set trigger MU degrouping */
	{"set_mu_trigger_group",        SetTriggerGroupProc},       /* set trigger MU grouping */
	{"set_mu_trigger_bbp",          SetTriggerBbpProc},         /* set trigger LQ */
	{"set_mu_trigger_sounding",     SetTriggerSndProc},         /* set trigger MU sounding */
	{"set_mu_ndp_delta_txpwr",      SetMuNdpDeltaTxPwr},
	/* the followings are relative to channel model setting */

#endif
#ifdef CFG_SUPPORT_FALCON_MURU
	{"set_muru_bsrp_ctrl",				SetMuruBsrpCtrl},
	{"set_muru_global_prot_sec_ctrl",	SetMuruGlobalProtSecCtrl},
	{"set_muru_tx_data_sec_ctrl",		SetMuruTxDataSecCtrl},
	{"set_muru_trig_data_sec_ctrl",		SetMuruTrigDataSecCtrl},
	{"set_muru_hesnd_ctrl",				SetMuruHeSndCtrl},
	{"set_muru_op_mode",				SetMuruArbOpMode},
	{"set_muru_algo_dbg",				SetMuruAlgoDbgCtrl},
	{"set_muru_sutx",					SetMuruSuTx},
	{"set_muru_data",                   SetMuruData},
	{"set_muru_20m_dyn_algo",			SetMuru20MDynAlgo},
	{"set_muru_prot_frame_thr",			SetMuruProtFrameThr},
	{"set_muru_dbdc_en",				SetMuruDbdcEnCtrlWorkaround},
	{"set_muru_txop_on",    			SetMuruTxopOnOff},
	{"set_muru_ul_off",                  SetMuruUlOnOff},
	{"set_mu_type",                     SetMuruTypeSelect},
	{"config_muru_stat",                SetMuruStatisticConfig},
	{"set_muru_txc_tx_stats",            SetMuruTxcTxStats},
	{"set_muru_dlul",                   SetMuruCfgDlUlVal},
	{"muru_dl_enable",			SetMuOfdmaDlEnableProc},
	{"muru_ul_enable",			SetMuOfdmaUlEnableProc},
	{"mu_dl_enable",			SetMuMimoDlEnableProc},
	{"mu_ul_enable",			SetMuMimoUlEnableProc},
	{"muruDbgInfo",                     set_muru_debug_flow_info},
#ifdef DABS_QOS
	{"set_muru_qos",			SetMuruQoSCfg},
#endif
	{"get_muru_fw_black_list_ctrl",		get_muru_fw_black_list_ctrl},
	{"set_muru_drv_black_list_ctrl",	set_muru_drv_black_list_ctrl},

	/* HQA AP commands */
	{"hqa_muru_set_dl_tx_muru_config",		hqa_muru_set_dl_tx_muru_config},
	{"hqa_muru_set_ul_tx_muru_config",		hqa_muru_set_ul_tx_muru_config},
	{"hqa_muru_set_ul_tx_trigger",			hqa_muru_set_ul_tx_trigger},
	{"hqa_muru_reset_ul_tx_cnt",			hqa_muru_reset_ul_tx_cnt},
	{"hqa_muru_get_ul_tx_cnt",				hqa_muru_get_ul_tx_cnt},
	{"hqa_muru_set_aggpol",					hqa_muru_set_agg_policy},
	{"hqa_muru_set_mu_tx_pkt_en",			hqa_muru_set_mu_tx_pkt_en},
	{"hqa_muru_set_mu_tx_pkt_cnt",			hqa_muru_set_mu_tx_pkt_cnt},

	/* muru manual config */
	{"set_muru_manual_config",				set_muru_manual_config},
	{"set_muru_debug_info",					set_muru_debug_info},
	{"disable_contention_tx",				set_disable_contention_tx},

#endif

#ifdef CFG_SUPPORT_FALCON_TXCMD_DBG
	{"txcmd_dbg_enable",                    set_txcmd_dbg_enable},
	{"txcmd_dbg_clear",			set_txcmd_dbg_clear},
	{"txcmd_dbg_muru_algo_disable",         set_txcmd_dbg_muru_algo_disable},
	{"txcmd_dbg_ra_algo_enable",            set_txcmd_dbg_ra_algo_enable},
	{"txcmd_dbg_dl_ul_tp",                  set_txcmd_dbg_dl_ul_tp},
	{"txcmd_dbg_send_trig",                 set_txcmd_dbg_send_trig},
	{"txcmd_global",                        set_txcmd_sxn_global},
	{"txcmd_protect_cmm",                   set_txcmd_sxn_protect},
	{"txcmd_protect_ruinfo",                set_txcmd_sxn_protect_ruinfo},
	{"txcmd_txdata_cmm",	                set_txcmd_sxn_txdata},
	{"txcmd_txdata_rualloc",	        set_txcmd_sxn_txdata_rualloc},
	{"txcmd_txdata_user_info",	        set_txcmd_sxn_txdata_userinfo},
	{"txcmd_trigdata_cmm",		        set_txcmd_sxn_trigdata},
	{"txcmd_trigdata_rualloc",		set_txcmd_sxn_trigdata_rualloc},
	{"txcmd_trigdata_user_ack_info",        set_txcmd_sxn_trigdata_user_ackinfo},
	{"txcmd_tf_txd_cmm",		        set_txcmd_tf_txd},
	{"txcmd_tf_basic_cmm",		        set_txcmd_tf_basic},
	{"txcmd_tf_basic_user",		        set_txcmd_tf_basic_user},
	{"txcmd_sxn_dw",		        set_txcmd_sxn_dw},
	{"txcmd_sxn_user_dw",		        set_txcmd_sxn_user_dw},
	{"txcmd_debug_sop",			set_txcmd_dbg_sop},
#endif /* CFG_SUPPORT_FALCON_TXCMD_DBG */
	{"mcli",                   set_mcli_cfg},
#ifdef VOW_SUPPORT
	/* VOW GROUP table */
	{"vow_min_rate_token",  set_vow_min_rate_token},
	{"vow_max_rate_token",  set_vow_max_rate_token},
	{"vow_min_airtime_token",  set_vow_min_airtime_token},
	{"vow_max_airtime_token",  set_vow_max_airtime_token},
	{"vow_min_rate_bucket",  set_vow_min_rate_bucket},
	{"vow_max_rate_bucket",  set_vow_max_rate_bucket},
	{"vow_min_airtime_bucket",  set_vow_min_airtime_bucket},
	{"vow_max_airtime_bucket",  set_vow_max_airtime_bucket},
	{"vow_max_wait_time", set_vow_max_wait_time},
	{"vow_max_backlog_size", set_vow_max_backlog_size},

	/* VOW CTRL */
	{"vow_bw_enable", set_vow_bw_en},
	{"vow_refill_en", set_vow_refill_en},
	{"vow_airtime_fairness_en", set_vow_airtime_fairness_en},
	{"vow_txop_switch_bss_en", set_vow_txop_switch_bss_en},
	{"vow_dbdc_search_rule", set_vow_dbdc_search_rule},
	{"vow_refill_period", set_vow_refill_period},
	{"vow_bss_enable", set_vow_bss_en},
	{"vow_spl_sta_num", set_vow_spl_sta_num},
	{"vow_mcli_schedule_en", set_vow_mcli_schedule_en},
	{"vow_mcli_schedule_parm", set_vow_mcli_schedule_parm},

	{"vow_airtime_control_en", set_vow_airtime_ctrl_en},
	{"vow_bw_control_en", set_vow_bw_ctrl_en},
	{"vow_sch_ctrl", set_vow_schedule_ctrl},

	/* group other */
	{"vow_bss_dwrr_quantum", set_vow_bss_dwrr_quantum},
	{"vow_group_dwrr_max_wait_time", set_vow_group_dwrr_max_wait_time},
	{"vow_group2band_map", set_vow_group2band_map},

	/* VOW STA table */
	{"vow_sta_dwrr_quantum", set_vow_sta_dwrr_quantum},
	{"vow_sta_dwrr_quantum_id", set_vow_sta_dwrr_quantum_id},
	{"vow_sta_ac_priority", set_vow_sta_ac_priority},
	{"vow_sta_pause", set_vow_sta_pause},
	{"vow_sta_psm", set_vow_sta_psm},
	{"vow_sta_group", set_vow_sta_group},
	{"vow_dwrr_max_wait_time", set_vow_dwrr_max_wait_time},

	/* STA fast round robin */
	{"vow_sta_frr_quantum", set_vow_sta_frr_quantum},

	/* near far adjustment */
	{"vow_near_far_en", set_vow_near_far_en},
	{"vow_near_far_th", set_vow_near_far_th},

	/* USER */
	{"vow_min_rate", set_vow_min_rate},
	{"vow_max_rate", set_vow_max_rate},
	{"vow_min_ratio", set_vow_min_ratio},
	{"vow_max_ratio", set_vow_max_ratio},

	/* RX airtime */
	{"vow_rx_counter_clr", set_vow_rx_counter_clr},
	{"vow_rx_airtime_en", set_vow_rx_airtime_en},
	{"vow_rx_early_end_en", set_vow_rx_early_end_en},
	{"vow_rx_ed_offset", set_vow_rx_ed_offset},
	{"vow_rx_obss_backoff", set_vow_rx_obss_backoff},
	/* {"vow_rx_add_obss", set_vow_rx_add_obss}, */
	/* {"vow_rx_add_non_wifi", set_vow_rx_add_non_wifi}, */
	{"vow_rx_wmm_backoff", set_vow_rx_wmm_backoff},
	{"vow_om_wmm_backoff", set_vow_rx_om_wmm_backoff},
	{"vow_repeater_wmm_backoff", set_vow_rx_repeater_wmm_backoff},
	{"vow_rx_non_qos_backoff",  set_vow_rx_non_qos_backoff},
	{"vow_rx_bss_wmmset", set_vow_rx_bss_wmmset},
	{"vow_rx_om_wmm_sel", set_vow_rx_om_wmm_select},

	/* airtime estimator */
	{"vow_at_est_en", set_vow_at_est_en},
	{"vow_at_mon_period", set_vow_at_mon_period},

	/* badnode detector */
	{"vow_bn_en", set_vow_bn_en},
	{"vow_bn_mon_period", set_vow_bn_mon_period},
	{"vow_bn_fallback_th", set_vow_bn_fallback_th},
	{"vow_bn_per_th", set_vow_bn_per_th},

	/* airtime counter test */
	{"vow_counter_test", set_vow_counter_test_en},
	{"vow_counter_test_period", set_vow_counter_test_period},
	{"vow_counter_test_band", set_vow_counter_test_band},
	{"vow_counter_test_avgcnt", set_vow_counter_test_avgcnt},
	{"vow_counter_test_target", set_vow_counter_test_target},

	/* DVT */
	{"vow_dvt_en", set_vow_dvt_en},
	{"vow_show_en", set_vow_show_en},
	{"vow_monitor_sta", set_vow_monitor_sta},
	{"vow_show_sta", set_vow_show_sta},
	{"vow_monitor_bss", set_vow_monitor_bss},
	{"vow_monitor_mbss", set_vow_monitor_mbss},
	{"vow_show_mbss", set_vow_show_mbss},
	{"vow_avg_num", set_vow_avg_num},

	/*WATF*/
	{"vow_watf_en", set_vow_watf_en},
	{"vow_watf_q", set_vow_watf_q},
	{"vow_watf_add_entry", set_vow_watf_add_entry},
	{"vow_watf_del_entry", set_vow_watf_del_entry},

	/* help */
	{"vow_help", set_vow_help},



	/*
	{"vow_rx_add_obss", set_vow_rx_add_obss},
	{"vow_rx_add_non_wifi", set_vow_rx_add_non_wifi},
	*/
#endif /* VOW_SUPPORT */

#ifdef DABS_QOS
	{"dabs_qos",					set_dabs_qos_param},
#endif
#ifdef RED_SUPPORT
	{"red_en",						set_red_enable},
	{"red_show_sta",				set_red_show_sta},
	{"red_tar_delay",				set_red_target_delay},
	{"red_debug_en",				set_red_debug_enable},
	{"red_dump_reset",				set_red_dump_reset},
	{"red_drop",					set_red_drop},
	{"red_config",					set_red_config},
#endif /* RED_SUPPORT */
#ifdef FQ_SCH_SUPPORT
	{"fq_en",                                              set_fq_enable},
	{"fq_debug_en",                                set_fq_debug_enable},
	{"fq_listmap",					set_fq_dbg_listmap},
	{"fq_linklist",					set_fq_dbg_linklist},
#endif /* RRSCH_SUPPORT */
	{"cp_support",					set_cp_support_en},
#ifdef PKTLOSS_CHK
	{"pktloss",                                       set_pktloss_chk},
#endif
#ifdef CFG_SUPPORT_MU_MIMO_RA
	{"mura_periodic_sounding",      SetMuraPeriodicSndProc},         /* set trigger MURGA Algorithm sounding */
	{"mura_algorithm_test",         SetMuraTestAlgorithmProc},
	{"mura_algorithm_init",         SetMuraTestAlgorithmInit},
	{"mura_sounding_fixed_param",   SetMuraFixedSndParamProc},
	{"mura_disabe_CN3_CN4",			SetMuraDisableCN3CN4Proc},
	{"mura_mobility_en",              SetMuraMobilityCtrlProc},
	{"mura_mobility_interval_ctrl",   SetMuraMobilityIntervalCtrlProc},
	{"mura_mobility_snr_ctrl",        SetMuraMobilitySNRCtrlProc},
	{"mura_mobility_threshold_ctrl",  SetMuraMobilityThresholdCtrlProc},
	{"mura_mobility_snd_cnt",         SetMuraMobilitySndCountProc},
	{"mura_mobility_mode_ctrl",       SetMuraMobilityModeCtrlProc},
	{"mura_mobility_log_ctrl",        SetMuraMobilityLogCtrlProc},
	{"mura_mobility_test_ctrl",       SetMuraMobilityTestCtrlProc},
	{"mura_hwfb_up_down",             SetMuraHwFallbackProc},
#endif

#if (defined(CFG_SUPPORT_MU_MIMO_RA) || defined(CFG_SUPPORT_FALCON_MURU))
	{"mura_algorithm_fixed",          SetMuMimoFixedRate},
	{"mura_algorithm_fixed_group",    SetMuMiMoFixedGroupRateProc},
	{"mum_force_mu",                  SetMuMimoForceMu},
#endif /*(CFG_SUPPORT_MU_MIMO_RA) || (CFG_SUPPORT_FALCON_MURU)*/

#endif /* MT_MAC */
#ifdef BACKGROUND_SCAN_SUPPORT
	{"bgndscan",               set_background_scan},
	{"bgndscantest",		set_background_scan_test},
	{"bgndscannotify",		set_background_scan_notify},
	{"bgndscancfg",               set_background_scan_cfg},
#endif /* BACKGROUND_SCAN_SUPPORT */
#ifdef NEW_SET_RX_STREAM
	{"RxStream",                        Set_RxStream_Proc},
#endif
#ifdef ERR_RECOVERY
	{"ErrDetectOn", Set_ErrDetectOn_Proc},
	{"ErrDetectMode", Set_ErrDetectMode_Proc},
#endif /* ERR_RECOVERY */
#ifdef SMART_CARRIER_SENSE_SUPPORT
	{"SCSEnable", Set_SCSEnable_Proc},
	{"SCSpdrange", Set_SCSPdThrRange_Proc},
	{"SCSCfg", Set_SCSCfg_Proc},
	{"SCSPd", Set_SCSPd_Proc},
#endif /*  SMART_CARRIER_SENSE_SUPPORT */
#ifdef SWACI_MECHANISM
	{"LNAGainAdj", Set_LNAGain_Proc},
	{"LNATableRssi", Set_LNATable_Proc},
	{"LNACondition", Set_LNACondition_Proc},
	{"LNADense", Set_LNADenseParam_Proc},
	{"LNATimerEnable", Set_LNATimer_Proc},
	{"LNAEnable", Set_LNAEnable_Proc},
	{"LNAThreshold", Set_LNAThreshConfig_Proc},
	{"LNATestMode", Set_LNATestMode_Proc},
#endif
#ifdef BAND_STEERING
	{"BndStrgEnable",		Set_BndStrg_Enable},
	{"BndStrgBssIdx",		Set_BndStrg_BssIdx},
	{"BndStrgParam",    Set_BndStrg_Param},
#ifdef BND_STRG_DBG
	{"BndStrgMntAddr",	Set_BndStrg_MonitorAddr},
#endif /* BND_STRG_DBG */
#endif /* BAND_STEERING */
	{"DyncBWEn",	SetHeraOptionDyncBW_Proc},
	{"LegFreqDupEn",	SetHeraOptionFrequecyDup_Proc},
	{"IARaEn",	SetHeraIara_Proc},
	{"TxPwrManualSet",  SetTxPwrManualCtrl},
	{"TpcManCtrl",			SetTpcManCtrl},
	{"TpcEnable",			SetTpcEnable},
	{"TpcWlanIdCtrl",		SetTpcWlanIdCtrl},
	{"TpcUlAlgoCtrl",		SetTpcUlAlgoCtrl},
	{"TpcDlAlgoCtrl",		SetTpcDlAlgoCtrl},
	{"TpcManTblInfo",		SetTpcManTblInfo},
	{"TpcAlgoUlUtCfg",		SetTpcAlgoUlUtCfg},
	{"TpcAlgoUlUtGo",		SetTpcAlgoUlUtGo},
	{"SKUCtrl",				SetSKUCtrl},
	{"PercentageCtrl",		SetPercentageCtrl},
	{"PowerDropCtrl",		SetPowerDropCtrl},
	{"DecreasePower",		SetDecreasePwrCtrl},
	{"BFBackoffCtrl",		SetBfBackoffCtrl},
	{"ThermoCompCtrl",		SetThermoCompCtrl},
	{"RFTxAnt",				SetRfTxAnt},
	{"TxPowerInfo",			SetTxPowerInfo},
	{"TOAECtrl",			SetTOAECtrl},
	{"SKUInfo",				SetSKUInfo},
	{"BFBackoffInfo",		SetBFBackoffInfo},
	{"CCKTxStream",         SetCCKTxStream},
	{"RxvEnCtrl",			SetRxvEnCtrlProc},
	{"RxvRuCtrl",			SetRxvRuCtrlProc},
	{"rxv_dump",			SetRxvRawDump},
	{"rxv_info",			SetRxvInfo},
	{"rxv_stat_reset",		SetRxvStatReset},
	{"rxv_log_ctrl",		SetRxvLogCtrl},
	{"rxv_list_info",		SetRxvListInfo},
	{"rxv_report_info", SetRxvRptInfo},
	{"ThermalManMode",		SetThermalManCtrl},
	{"ThermalTaskInfo",     SetThermalTaskInfo},
	{"ThermalTaskCtrl",     SetThermalTaskCtrl},
	{"thermal_protect_enable", SetThermalProtectEnable},
	{"thermal_protect_disable", SetThermalProtectDisable},
	{"thermal_protect_duty_cfg", SetThermalProtectDutyCfg},
	{"thermal_protect_info", SetThermalProtectInfo},
	{"thermal_protect_duty_info", SetThermalProtectDutyInfo},
	{"thermal_protect_state_act", SetThermalProtectStateAct},
	{"mgmt_txpwr_offset",   set_mgmt_txpwr_offset},
	{"support_rate_table_ctrl", set_support_rate_table_ctrl},
	{"support_rate_table_info", set_support_rate_table_info},
	{"ra_dbg", set_ra_dbg_ctrl},
#ifdef AMPDU_CONF_SUPPORT
	{"AGG_MPDU_Count",      Set_AMPDU_MPDU_Count},
	{"AMPDUretrycount", 	Set_AMPDU_Retry_Count},
#endif
#ifdef WIFI_EAP_FEATURE
	{"InitIPICtrl",         SetInitIPICtrl},
	{"data_txpwr_offset",   set_data_txpwr_offset},
	{"fw_ratbl",            set_fw_ratbl_ctrl},
#endif
	{"EDCCAThreshold",      SetEDCCAThresholdCtrl},
	{"EDCCAEnable",		SetEDCCAEnableCtrl},
#ifdef WIFI_GPIO_CTRL
	{"GPOEn",               set_gpio_ctrl},
	{"GPOVal",              set_gpio_value},
#endif /* WIFI_GPIO_CTRL */
#ifdef LINK_TEST_SUPPORT
	{"LinkTestRxParamCtrl",		SetLinkTestRxParamCtrl},
	{"LinkTestModeCtrl",		SetLinkTestModeCtrl},
	{"LinkTestPowerUpTblCtrl",	SetLinkTestPowerUpTblCtrl},
	{"LinkTestPowerUpTblInfo",	SetLinkTestPowerUpTblInfo},
	{"LinkTestInfo",			SetLinkTestInfo},
#endif /* LINK_TEST_SUPPORT */
#ifdef ETSI_RX_BLOCKER_SUPPORT
	{"ETSISetFixWbIbRssiCtrl",	SetFixWbIbRssiCtrl},
	{"ETSISetRssiThCtrl",		SetRssiThCtrl},
	{"ETSISetCheckThCtrl",		SetCheckThCtrl},
	{"ETSISetAdaptRxBlockCtrl", SetAdaptRxBlockCtrl},
	{"ETSISetWbRssiDirectCtrl",   SetWbRssiDirectCtrl},
	{"ETSISetIbRssiDirectCtrl",   SetIbRssiDirectCtrl},
#endif /* end ETSI_RX_BLOCKER_SUPPORT */
	{"MUTxPower",           SetMuTxPower},
#ifdef DATA_TXPWR_CTRL
	{"data_frame_pwr",    SetTxPwrDataFrame},
	{"data_frame_min_pwr",    SetTxPwrLimitDataFrame},
#endif
#ifdef TX_POWER_CONTROL_SUPPORT
	{"TxPowerBoostCtrl",	SetTxPowerBoostCtrl},
	{"TxPowerBoostInfo",	SetTxPowerBoostInfo},
#endif

#ifdef LED_CONTROL_SUPPORT
	{"led_setting", Set_Led_Proc},
#endif /* LED_CONTROL_SUPPORT */
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	{"ktoflash_debug", Set_KtoFlash_Debug_Proc},
	{"RDCE", Set_RDCE_Proc},
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
	{"hw_nat_register", set_hnat_register},
	{"MibBucket", Set_MibBucket_Proc},
#ifdef RLM_CAL_CACHE_SUPPORT
	{"rlm_cal_cache",  Set_RLM_Cal_Cache_Debug_Proc},
#endif /* RLM_CAL_CACHE_SUPPORT */
#ifdef PKT_BUDGET_CTRL_SUPPORT
	{"pbc_ubound", Set_PBC_Proc},
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
#ifdef PS_STA_FLUSH_SUPPORT
	{"ps_flush", Set_PS_Flush_Proc},
#endif /*PS_STA_FLUSH_SUPPORT*/
	{"bwf", Set_BWF_Enable_Proc},
#ifdef TX_AGG_ADJUST_WKR
	{"agg_adj_wkr", Set_AggAdjWkr_Proc},
#endif /* TX_AGG_ADJUST_WKR */
#ifdef HTC_DECRYPT_IOT
	{"htc_th",		Set_HTC_Err_TH_Proc},
	{"htc_entry_err_cnt",		Set_Entry_HTC_Err_Cnt_Proc},
	{"wtbl_addom",		Set_WTBL_AAD_OM_Proc},
#endif /* HTC_DECRYPT_IOT */
#ifdef DHCP_UC_SUPPORT
	{"dhcp_uc",		Set_DHCP_UC_Proc},
#endif /* DHCP_UC_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2
	{"hs_flag", Set_CR4_Hotspot_Flag},
#endif /* CONFIG_HOTSPOT_R2 */

#ifdef HOSTAPD_HS_R2_SUPPORT
	{"icmpv4_deny", Set_Icmpv4_Deny},
	{"l2_filter", 	Set_L2_Filter},
	{"qload_bss",  Set_Qload_Bss},
	{"dgaf_disable",   Set_DGAF_Disable},
	{"configure_qosmap", Set_QosMap},
#endif
#if defined(HOSTAPD_HS_R2_SUPPORT) || defined(CONFIG_PROXY_ARP)
	{"proxyarp_enable", Set_ProxyArp_Enable},
#endif
#ifdef HOSTAPD_HS_R3_SUPPORT
	{"osu", 		Set_Osu_Enable},
#endif
	{"ser", set_ser},
#ifdef RTMP_PCI_SUPPORT
	{"rxd_debug", set_rxd_debug},
#endif
#ifdef CONFIG_TX_DELAY
	{"tx_batch_cnt", Set_TX_Batch_Cnt_Proc},
	{"tx_delay_timeout", Set_TX_Delay_Timeout_Proc},
	{"tx_pkt_min_len", Set_Pkt_Min_Len_Proc},
	{"tx_pkt_max_len", Set_Pkt_Max_Len_Proc},
#endif
	{"tx_amsdu", set_tx_amsdu},
	{"rx_amsdu", set_rx_amsdu},
#ifdef CUT_THROUGH
	{"token_setting", set_token_setting},
#endif
	{"tx_max_cnt", set_tx_max_cnt},
	{"rx_max_cnt", set_rx_max_cnt},
	{"tx_deq_cpu", set_tx_deq_cpu},
	{"ba_dbg", set_ba_dbg},
#ifdef MBO_SUPPORT
	{"mbo_nr",		SetMboNRIndicateProc},
	{"AutoRoaming", Set_AutoRoaming_Proc},
	{"mbo_ch_pref",  SetMboChPrefProc},
#endif /* MBO_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
	{"Disallow_ProbeEvent",	Set_BS_ProbeEvent_Proc},
#endif /* CONFIG_MAP_SUPPORT */
	{"rx_cnt_io_thd", set_rx_cnt_io_thd},
	{"rx_delay_ctl", set_rx_dly_ctl},
	{"tx_delay_ctl", set_tx_dly_ctl},
#ifdef MT7663_RFB_MANUAL_CAL
	{"manual_cal_id", set_manual_cal_id},
#endif /* MT7663_RFB_MANUAL_CAL */
#ifdef FW_LOG_DUMP
	{"fwlogdir", set_fw_log_dest_dir},
	{"binarylog", set_binary_log},
#endif /* FW_LOG_DUMP */
	{"vie_op", vie_oper_proc},
	{"hwctrl_dbg", set_hwctrl_dbg},
	{"hwctrl_q_len", set_hwctrl_q_len},
#ifdef DOT11_HE_AX
	{"color_dbg", set_color_dbg},
	{"txop_dur_rts_thld", set_txop_duration_prot_threshold},
#endif

#ifdef PER_PKT_CTRL_FOR_CTMR
	{"host_pkt_ctrl_en", set_host_pkt_ctrl_en},
#endif

#ifdef VERIFICATION_MODE
	{"dump_rx", set_dump_rx_debug},
	{"skip_ageout", set_skip_ageout},
	{"veri_pkt_head", set_veri_pkt_head},
	{"veri_pkt_ctnt", set_veri_pkt_ctnt},
	{"send_veri_pkt", send_veri_pkt},
	{"veri_pkt_ctrl_en", set_veri_pkt_ctrl_en},
	{"veri_pkt_ctrl_assign", set_veri_pkt_ctrl_assign},
	{"veri_switch", veri_mode_switch},
	{"ecc_test", ecc_calculate_test},

#endif
	{"wpa3_test", set_wpa3_test},
	{"transition_disable", set_transition_disable},
#ifdef DOT11_SAE_SUPPORT
	{"sae_commit_msg", sae_set_commit_msg},
	{"sae_pk_private_key_overwrite", sae_pk_set_pri_key_overwrite},
	{"sae_pk_test", sae_pk_set_test_ctrl},
	{"sae_k_iter", sae_set_k_iteration},
	{"sae_anti_clogging_th", sae_set_anti_clogging_th},
	{"sae_fixed_group_id", sae_set_fixed_group_id},
	{"sae_debug", sae_set_debug_level},
#endif
#ifdef CONFIG_WIFI_SYSDVT
	{"dvt", dvt_feature_search},
#ifdef DOT11_HE_AX
	{"sta_he_test", dvt_enable_sta_he_test},
#endif /* DOT11_HE_AX */
#endif /*CONFIG_WIFI_SYSDVT*/
#ifdef CFG_SUPPORT_FALCON_SR
	{"srcapsren", SetSrCapSrEn},
	{"srcap", SetSrCapAll},
	{"srpara", SetSrParaAll},
	{"srdropta", SetSrDropTa},
	{"srsta", SetSrSta},
	{"srstainit", SetSrStaInit},
	{"srcond", SetSrCondAll},
	{"srrcpitbl", SetSrRcpiTblAll},
	{"srrcpitblofst", SetSrRcpiTblOfstAll},
	{"srqctrl", SetSrQCtrlAll},
	{"sribpd", SetSrIBPDAll},
	{"srnrt", SetSrNRTAll},
	{"srnrtreset", SetSrNRTResetAll},
	{"srnrtctrl", SetSrNRTCtrlAll},
	{"srfnqctrl", SetSrFNQCtrlAll},
	{"srfrmfilt", SetSrFrmFiltAll},
	{"srinterpsctrl", SetSrInterPsCtrlAll},
	{"srcfgsren", SetSrCfgSrEnable},
	{"srcfgsrsden", SetSrCfgSrSdEnable},
	{"srcfgsrbf", SetSrCfgSrBf},
	{"srcfgsratf", SetSrCfgSrAtf},
	{"srcfgsrmode", SetSrCfgSrMode},
	{"srcfgdisrten", SetSrCfgDISRTEnable},
	{"srcfgdisrtmin", SetSrCfgDISRTMinRssi},
	{"srcfgtxcq", SetSrCfgTxcQueue},
	{"srcfgtxcqid", SetSrCfgTxcQid},
	{"srcfgtxcpath", SetSrCfgTxcPath},
	{"srcfgac", SetSrCfgAcMethod},
	{"srcfgsrperiodthr", SetSrCfgSrPeriodThr},
	{"srcfgsrquerytxd", SetSrCfgQueryTxDMethod},
	{"srcfgsrsdcg", SetSrCfgSrSdCgRatio},
	{"srcfgsrsdobss", SetSrCfgSrSdObssRatio},
	{"srcfgsrprofile", SetSrCfgProfile},
	{"srcfgfnqen", SetSrCfgFnqEnable},
	{"srcfgdpden", SetSrCfgDPDEnable},
	{"srcfgsrtxen", SetSrCfgSrTxEnable},
	{"srcfgomen", SetSrCfgObssMonitorEnable},
	{"srcfgsrtxalignen", SetSrCfgSrTxAlignEnable},
	{"srcfgsrtxalignrssi", SetSrCfgSrTxAlignRssiThr},
	{"srcfgdabsmode", SetSrCfgDabsMode},
	{"srcfgsrdpdthreshold", SetSrCfgSrDPDThreshold},
	{"srsrgbm", SetSrSrgBitmap},
	{"srsrgbmrefresh", SetSrSrgBitmapRefresh},
	{"srmeshsrgbm", SetSrMeshSrgBitmap},
	{"srsiga", SetSrSiga},
	{"srsigaauto", SetSrSigaAuto},
	{"srselfsrginfo", SetSrSelfSrgInfo},
	{"srmeshtopolock", SetSrMeshTopoLock},
	{"srmeshremotefhrssi", SetSrMeshRemoteFhRssi},
	{"srmeshremotefhrssiTh", SetSrMeshRemoteFhRssiTh},
	{"srmeshstath", SetSrMeshStaThreshold},
	{"srmeshbhsrgbm", SetSrMeshBHSrgBitmap},
	{"srmeshfhsrgbm", SetSrMeshFHSrgBitmap},
	{"srmeshbhdlobsspdth", SetSrMeshBHDownThreshold},
	{"srmeshfhdlobsspdth", SetSrMeshFHDownThreshold},
	{"srmeshforbitsr", SetSrMeshBHDownLinkForbidSR},
	{"srmeshflushforbitsr", SetSrMeshResetBhDLForbidSR},
	{"srmeshremotebhinfo", SetSrMeshRemoteBhInfo},
	{"srmeshtopo", SetSrMeshTopo},
	{"srmapbalance", SetSrMeshMapBalance},
	{"srmeshuplinkevent", SetSrMeshUplinkEvent},
	{"srmeshmacdetect", SrMeshApcliDetect},
	{"srsetmeshsd", SetMeshSRsd},
	{"srintermac", SetMeshMac},
	{"srmeshulmode", SetSrMeshUlMode},
	{"srmeshstamoderptlock", SetSrMeshStaModeRptLock},
	{"srmeshallremotestahe", SetSrMeshRemoteStaHe},
#endif /* CFG_SUPPORT_FALCON_SR */
#ifdef CONFIG_WIFI_DBG_TXCMD
	{"dbg_txcmd", dbg_txcmd_feature_search},
#endif /*CONFIG_WIFI_DBG_TXCMD*/
	{"load_fw_method", SetLoadFwMethod},
	{"trig_core_dump", SetTrigCoreDump},
#ifdef RATE_PRIOR_SUPPORT
	{"LowRateCtrl", Set_RatePrior_Proc},
	{"LowRateRatioThreshold", Set_LowRateRatio_Proc},
	{"LowRateCountPeriod", Set_LowRateCountPeriod_Proc},
	{"TotalCntThreshold", Set_TotalCntThreshold_Proc},
	{"BlackListTimeout", Set_BlackListTimeout_Proc},
#endif /*RATE_PRIOR_SUPPORT*/
	{"ap_rfeatures", set_ap_rfeatures},
#if defined(DOT11_HE_AX) && defined(FIXED_HE_GI_SUPPORT)
	{"fgi_per_bss", set_fgi_and_ltf_proc},
#endif
#if defined(APCLI_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	{"sta_wireless", set_sta_wireless},
	{"sta_rfeatures", set_sta_rfeatures},
	{"PSMode", Set_PSMode_Proc},
#endif /* defined(APCLI_SUPPORT) || defined(CONFIG_STA_SUPPORT) */
#ifdef FW_LOG_DUMP
	{"fwlogserverip", set_fwlog_serverip},
	{"fwlogservermac", set_fwlog_servermac},
#endif /* FW_LOG_DUMP */
	{"fwcmdtoprintcnt", set_fwcmd_timeout_print_cnt},
	{"fw_cpu_util_en", Set_CpuUtilEn_Proc},
	{"fw_cpu_util_mode", Set_CpuUtilMode_Proc},
#ifdef WIFI_MODULE_DVT
	{"mdvt", SetMdvtModuleParameterProc},
#endif
#ifdef CFG_SUPPORT_FALCON_PP
	{"ppcapctrl", set_pp_cap_ctrl},
#endif /* CFG_SUPPORT_FALCON_PP */
	{"idlepwr", set_idle_pwr_test},
	{"set_enable_nf", SetEnableNf},
#ifdef MGMT_TXPWR_CTRL
		{"mgmt_frame_pwr",	set_mgmt_frame_power},
#endif
#ifdef IWCOMMAND_CFG80211_SUPPORT
	{"apmacaddress",	set_apmacaddress},
	{"apclimacaddress",	set_apclimacaddress},
#endif /* IWCOMMAND_CFG80211_SUPPORT */
#ifdef ANTENNA_CONTROL_SUPPORT
	{"AntCtrl", 	Set_Antenna_Control_Proc},
#endif /* ANTENNA_CONTROL_SUPPORT */
#ifdef ACK_CTS_TIMEOUT_SUPPORT
	{"ACK_CTS_TOUT_EN",		set_ackcts_timeout_enable_porc},
	{"CCK_ACK_TOUT", set_cck_ack_timeout_porc},
	{"OFDM_ACK_TOUT",	set_ofdm_ack_timeout_proc},
	{"OFDMA_ACK_TOUT",	set_ofdma_ack_timeout_proc},
	{"Distance",		set_dst2acktimeout_proc},
#endif/*ACK_CTS_TIMEOUT_SUPPORT*/
#ifdef ANDLINK_FEATURE_SUPPORT
	{"andlink_en", 	set_andlink_en_porc},
	{"andlink_ip_hostname_en", set_andlink_ip_hostname_en_porc},
#ifdef ANDLINK_V4_0
	{"andlink_simple_val",	set_andlink_simple_val},
	{"andlink_uplink_period",	set_andlink_uplink_period},
	{"andlink_uplink_sample",	set_andlink_uplink_sample},
	{"andlink_sta_period",	set_andlink_sta_period},
	{"andlink_sta_sample",	set_andlink_sta_sample},
#endif
#endif/*ANDLINK_FEATURE_SUPPORT*/

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	{"MaxProbeRspIECnt", Set_Max_ProbeRsp_IE_Cnt_Proc},
#endif /*CUSTOMER_VENDOR_IE_SUPPORT*/
#ifdef MLME_MULTI_QUEUE_SUPPORT
	{"MlmeMultiQCtrl",	set_mlme_queue_ration},
#endif
#ifdef CFG_SUPPORT_CSI
	{"csi",	Set_CSI_Ctrl_Proc},
#endif
#ifdef WIFI_CSI_CN_INFO_SUPPORT
	{"EnableCNInfo", set_csi_cn_info_proc},
#endif /* WIFI_CSI_CN_INFO_SUPPORT */
	{"dev_send_frame", dev_send_frame},
	{"rts2cts", SetRtsThenCtsRetryCnt},
#ifdef CONFIG_6G_SUPPORT
	{"6giob", set_6g_ap_iob_discovery_proc},
	{"6goob", set_6g_ap_oob_discovery_proc},
	{"6gqos", set_6g_ap_qos_injector_proc},
#endif
#ifdef DPP_SUPPORT
	{"DppEnable", Set_Enable_Dpp_Proc},
#endif /* DPP_SUPPORT */
#ifdef QOS_R1
	{"QoSR1Enable",			Set_QoSR1Enable_Proc},
	{"QoSConfig",			QoS_Config_for_RDUT_Test},
#ifdef QOS_R2
	{"ScsEnable",			Set_ScsEnable_Proc},
	{"DSCPPolicyEnable",		Set_DSCPPolicyEnable_Proc},
	{"QoSMgmtCapa",			Set_QoSMgmtCapa_Proc},
	{"QosMapCapa",			Set_QoSMapCapa_Proc},
#endif
#ifdef MSCS_PROPRIETARY
	{"dabs_drop_thre",			Set_Dabs_Drop_Thre_Proc},
	{"keybitmap",			Set_DABSkeybitmap_Proc},
#endif
#endif
#ifdef IXIA_C50_MODE
	{"ixia_debug_lvl", Set_ixia_debug_Proc},
	{"ixia_offset", Set_pktlen_offset_Proc},
	{"ixia_force", Set_ForceIxia_Proc},
	{"ixia_round_reset", Set_ixia_round_reset_Proc},
#endif
#ifdef CONFIG_ICS_FRAME_HANDLE
	{"rxvIcs",					set_rxv_ics_proc},
#endif
#ifdef VLAN_SUPPORT
	{"v2e", Set_Vlan2eth_Proc},
#endif
#ifdef CONFIG_6G_AFC_SUPPORT
	{"afcparams",          afc_cmd_set_afc_params},
	{"afcevents",          set_afc_event},
#endif /* CONFIG_6G_AFC_SUPPORT */
	{NULL,}
};

static struct {
	RTMP_STRING *name;
	INT (*set_proc)(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
} *PRTMP_PRIVATE_SHOW_PROC, RTMP_PRIVATE_SHOW_SUPPORT_PROC[] = {
#ifdef ACL_BLK_COUNT_SUPPORT
	{"ACLRejectCount",				Show_ACLRejectCount_Proc},
#endif/*ACL_BLK_COUNT_SUPPORT*/
	{"stainfo",			Show_MacTable_Proc},
#ifdef MWDS
	{"mwdsinfo",			Show_MWDS_Proc},
#endif
#ifdef ZERO_LOSS_CSA_SUPPORT
	{"ChannelSwitchTime",	Show_ChannelSwitchTime_Proc},
	{"PerSTATxStop",		Show_PerSTA_StopTx_Proc},
	{"ZeroLossStaList",	Show_ZeroLossStaList_Proc},
#endif /*ZERO_LOSS_CSA_SUPPORT*/
	{"partial_mib_info",		Show_Mib_Info_Proc},
#if defined(DOT11_HE_AX) && defined(FIXED_HE_GI_SUPPORT)
	{"fgi_info",		show_fgi_and_ltf_proc},
#endif
#ifdef TXRX_STAT_SUPPORT
	{"sta_stat",			Show_Sta_Stat_Proc},
	{"mbss_stat",			Show_Bss_Stat_Proc},
	{"radio_stat",			Show_Radio_Stat_Proc},
#endif
#ifdef MEM_ALLOC_INFO_SUPPORT
	{"meminfo",			Show_MemInfo_Proc},
	{"pktmeminfo",		Show_PktInfo_Proc},
#endif /* MEM_ALLOC_INFO_SUPPORT */
#ifdef MT_MAC
	{"psinfo",			Show_PSTable_Proc},
	{"wtbl",				show_wtbl_proc},
	{"wtbltlv",				show_wtbltlv_proc},
	{"mibinfo", show_mib_proc},
	{"amsduinfo", show_amsdu_proc},
	{"wifi_sys", show_wifi_sys},
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	{"GetClientIdleTime", show_client_idle_time},
#endif
#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
	{"GetCurrentRSSI",	show_current_rssi},
#endif
#ifdef DBDC_MODE
	{"dbdcinfo",		ShowDbdcProc},
#endif
	{"wmm_info", show_wmm_info},
	{"channelinfo", ShowChCtrl},
#ifdef GREENAP_SUPPORT
	{"greenapinfo",		ShowGreenAPProc},
#endif /* GREENAP_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	{"pcieaspminfo",		show_pcie_aspm_dym_ctrl_cap_proc},
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	{"twtsupportinfo",		show_twt_support_cap_proc},
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
	{"txopinfo", show_tx_burst_info},
	{"tmacinfo", ShowTmacInfo},
	{"agginfo", ShowAggInfo},
	{"arbinfo", ShowArbInfo},
	{"manual_txop", ShowManualTxOP},
	{"pseinfo", ShowPseInfo},
	{"psedata", ShowPseData},
	{"pleinfo",             ShowPLEInfo},
	{"drrinfo",             show_drr_proc},
	{"txdinfo",             show_TXD_proc},
	{"dumpmem",             show_mem_proc},
	{"protectinfo", show_protect_info},
	{"ccainfo", show_cca_info},
	{"txvinfo", show_txvinfo},
#ifdef MT_FDB
	{"fdbn9log",            show_fdb_n9_log},
	{"fdbcr4log",           show_fdb_cr4_log},
#endif /* MT_FDB */

	{"dschinfo", show_dmasch_proc},

#endif /* MT_MAC */
	{"sta_tr",				Show_sta_tr_proc},
#ifdef SW_CONNECT_SUPPORT
	{"dummy_stat",		Show_dummy_stat_proc},
#endif /* SW_CONNECT_SUPPORT */
	{"peerinfo",			show_stainfo_proc},
	{"stacountinfo",			Show_StaCount_Proc},
#ifdef EAP_STATS_SUPPORT
	{"eap_stats",			Show_Eap_Stats_Proc},
#endif /* EAP_STATS_SUPPORT */
	{"secinfo",			Show_APSecurityInfo_Proc},
	{"descinfo",			Show_DescInfo_Proc},
#ifdef RXD_WED_SCATTER_SUPPORT
	{"rxdinfo",	            show_rxdinfo_proc},
#endif /* RXD_WED_SCATTER_SUPPORT */
	{"driverinfo",			show_driverinfo_proc},
	{"apcfginfo",			show_apcfg_info},
	{"devinfo",			show_devinfo_proc},
	{"sysinfo",			show_sysinfo_proc},
	{"trinfo",	show_trinfo_proc},
	{"tpinfo",	            show_tpinfo_proc},
	{"mlmeinfo",	        show_mlmeinfo_proc},
#ifdef CONFIG_TP_DBG
	{"tpdbginfo",			show_TPDbg_info_proc},
#endif /* CONFIG_TP_DBG */
#ifdef AP_SCAN_SUPPORT
	{"scaninfo",		show_scan_info_proc},
#endif /* AP_SCAN_SUPPORT */
	{"BSSEdca",			show_BSSEdca_proc},
	{"APEdca",			show_APEdca_proc},
	{"pwrinfo", chip_show_pwr_info},
	{"txqinfo",			show_txqinfo_proc},
	{"swqinfo", show_swqinfo},
#ifdef RTMP_EFUSE_SUPPORT
	{"efuseinfo",			    show_efuseinfo_proc},
#endif
	{"e2pinfo",			    show_e2pinfo_proc},
#ifdef WDS_SUPPORT
	{"wdsinfo",				Show_WdsTable_Proc},
#endif /* WDS_SUPPORT */
#ifdef DOT11_N_SUPPORT
	{"bainfo",				Show_BaTable_Proc},
	{"channelset",				Show_ChannelSet_Proc},
#endif /* DOT11_N_SUPPORT */
	{"stat",				Show_Sat_Proc},
#ifdef RTMP_MAC_PCI
#ifdef DBG_DIAGNOSE
	{"diag",				Show_Diag_Proc},
#endif /* DBG_DIAGNOSE */
#endif /* RTMP_MAC_PCI */
	{"stat_reset",			Show_Sat_Reset_Proc},
#ifdef IGMP_SNOOP_SUPPORT
	{"igmpinfo",			Set_IgmpSn_TabDisplay_Proc},
#ifdef IGMP_TVM_SUPPORT
	{"IgmpSnExemptIP",		Show_IgmpSn_BlackList_Proc},
	{"IgmpSnMcastTable",		Show_IgmpSn_McastTable_Proc},
#endif /* IGMP_TVM_SUPPORT */
	{"igmpwl",			Set_Igmp_Show_Flooding_CIDR_Proc},
	{"igmpfwwl",			Set_Igmp_Show_FW_Flooding_CIDR_Proc},
#endif /* IGMP_SNOOP_SUPPORT */
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	{"mgmrate",			show_mgmrate},
	{"bcnrate",			show_bcnrate},
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
#ifdef MCAST_RATE_SPECIFIC
	{"mcastrate",			Show_McastRate},
#endif /* MCAST_RATE_SPECIFIC */
#ifdef HIGHPRI_RATE_SPECIFIC
	{"HighPriRate",			Show_HighPriRate},
#endif /* MCAST_RATE_SPECIFIC */

#ifdef MAT_SUPPORT
	{"matinfo",			Show_MATTable_Proc},
#endif /* MAT_SUPPORT */
#ifdef MT_DFS_SUPPORT
	{"DfsProvideChList",		Show_available_BwCh_Proc},
	{"DfsNOP",			Show_DfsNonOccupancy_Proc},
	{"DfsNOPOfChList",		Show_NOP_Of_ChList},
	{"DfsTargetInfo",		Show_Target_Ch_Info},
	{"DfsChInfo",			show_dfs_ch_info_proc},
	{"RadarThresholdParam",	show_radar_threshold_param_proc},
	/* DFS radar pulse dump */
	{"DfsDebug",			show_dfs_debug_proc},
#endif
#ifdef DOT11R_FT_SUPPORT
	{"ftinfo",				Show_FTConfig_Proc},
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
	{"rrminfo",				RRM_InfoDisplay_Proc},
#endif /* DOT11K_RRM_SUPPORT */
#ifdef AP_QLOAD_SUPPORT
	{"qload",				Show_QoSLoad_Proc},
#endif /* AP_QLOAD_SUPPORT */
	{"TmrCal",				Show_TmrCalResult_Proc},
#ifdef APCLI_SUPPORT
	{"connStatus",			RTMPIoctlConnStatus},
#endif /* APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
	{"reptinfo",			Show_Repeater_Cli_Proc},
	{"rept_table",			Show_ReptTable_Proc},
#endif /* MAC_REPEATER_SUPPORT */
#ifdef SMART_ANTENNA
	{"sainfo",				Show_SA_CfgInfo_Proc},
	{"sadbginfo",			Show_SA_DbgInfo_Proc},
#endif /* SMART_ANTENNA // */

	{"rainfo",				Show_RAInfo_Proc},
#ifdef TXBF_SUPPORT
	{"txbfinfo",			Show_TxBfInfo_Proc},
#endif /* TXBF_SUPPORT */
#ifdef MBSS_SUPPORT
	{"mbss",			Show_MbssInfo_Display_Proc},
#endif /* MBSS_SUPPORT */
#ifdef WSC_AP_SUPPORT
	{"WscPeerList",		WscApShowPeerList},
	{"WscPin",			WscApShowPin},
#endif /* WSC_AP_SUPPORT */
#ifdef VLAN_SUPPORT
	{"vlaninfo", Show_VLAN_Info_Proc},
#endif /*VLAN_SUPPORT*/
#ifdef CONFIG_VLAN_GTK_SUPPORT
	{"vlan_gtk_info", Show_VLAN_GTK_Info_Proc},
#endif
#ifdef WIFI_MD_COEX_SUPPORT
	{"unsafeinfo", Show_UnsafeChannel_Info},
	{"idcinfo", Show_Idc_Info},
#endif
	{"rfinfo", ShowRFInfo},
	{"bbpinfo", ShowBBPInfo},
	{"wfintcnt", ShowWifiInterruptCntProc},
#ifdef CFG_SUPPORT_MU_MIMO
	{"hqa_mu_get_init_mcs",         hqa_mu_get_init_mcs},
	{"hqa_mu_get_qd",               hqa_mu_get_qd},
	{"hqa_mu_get_lq",               hqa_mu_get_lq},
	{"hqa_su_get_lq",               hqa_su_get_lq},
	{"hqa_get_murx_pktcnt",         ShowHqaMURxPktCnt},
	{"hqa_get_mutx_pktcnt",         ShowHqaMUTxPktCnt},

	{"get_mu_enable",               ShowMuEnableProc},                  /* show mu enable or disable */
	{"get_mu_profile",              ShowMuProfileProc},                 /* show mu profile entry */
	{"get_mu_clustertbl",           ShowClusterTblEntryProc},           /* show cluster table entry */
	{"get_mu_groupuserthreshold",   ShowGroupUserThresholdProc},        /* show group user threshold */
	{"get_mu_groupnssthreshold",    ShowGroupNssThresholdProc},         /* show group NSS threshold */
	{"get_mu_txreqmintime",         ShowTxReqMinTimeProc},              /* show tx req. min. time */
	{"get_mu_sunsscheck",           ShowSuNssCheckProc},                /* show SU Nss check enable or disable */
	{"get_mu_calcinitmcs",          ShowCalcInitMCSProc},               /* show Init MCS */
	{"get_mu_txopdefault",          ShowTxopDefaultProc},               /* show TXOP default */
	{"get_mu_sulossthreshold",      ShowSuLossThresholdProc},           /* show SU loss threshold */
	{"get_mu_mugainthreshold",      ShowMuGainThresholdProc},           /* show MU gain threshold */
	{"get_mu_secondaryacpolicy",    ShowSecondaryAcPolicyProc},         /* show secondary AC policay */
	{"get_mu_grouptbldmcsmask",     ShowGroupTblDmcsMaskProc},          /* show group table DMCS mask enable or disable */
	{"get_mu_maxgroupsearchcnt",    ShowMaxGroupSearchCntProc},         /* show max. group table search count */
	{"get_mu_txstatus",             ShowMuProfileTxStsCntProc},         /* show mu profile tx status */
#endif
#ifdef CFG_SUPPORT_FALCON_MURU
	{"get_muru_bsrp_ctrl",				ShowMuruBsrpCtrl},
	{"get_muru_global_prot_sec_ctrl",	ShowMuruGlobalProtSecCtrl},
	{"get_muru_tx_data_sec_ctrl",		ShowMuruTxDataSecCtrl},
	{"get_muru_trig_data_sec_ctrl",		ShowMuruTrigDataSecCtrl},
	{"get_mum",				ShowMuruMumCtrl},
	{"get_muru_hesnd_ctrl",				ShowMuruHeSndCtrl},
	{"get_muru_splcnt",					ShowMuruSplCnt},
	{"get_muru_glo_addr",				ShowMuruGloAddr},
	{"get_muru_local_data",				ShowMuruLocalData},
	{"get_muru_tx_info",                ShowMuruTxInfo},
	{"get_muru_shared_data",            ShowMuruSharedData},
	{"get_muru_mancfg_data",            ShowMuruManCfgData},
	{"get_last_spl",					ShowMuruLastSplByQid},
	{"get_muru_stacap_info",            ShowMuruStaCapInfo},
	{"get_muedca",				ShowMuruMuEdcaParam},
	{"get_ulru_status",			ShowMuruUlRuStatus},
	{"get_muru_txc_tx_stats",    ShowMuruTxcTxStats},
#endif/*END CFG_SUPPORT_FALCON_MURU*/

#if (defined(CFG_SUPPORT_FALCON_MURU) || defined(CFG_SUPPORT_MU_MIMO))
	{"get_mu_grouptbl",                ShowMuMimoGroupTblEntry},
#endif

#if (defined(CFG_SUPPORT_MU_MIMO_RA) || defined(CFG_SUPPORT_FALCON_MURU))
	{"mura_algorithm_monitor",  ShowMuMimoAlgorithmMonitor},
#endif

#ifdef CFG_SUPPORT_MU_MIMO_RA
	{"get_mura_pfid_stat",      GetMuraPFIDStatProc},
#endif

#ifdef CFG_SUPPORT_FALCON_TXCMD_DBG
	{"txcmd_status",	show_txcmd_dbg_status},
	{"txcmd_global",	show_txcmd_sxn_global},
	{"txcmd_protect",	show_txcmd_sxn_protect},
	{"txcmd_txdata",	show_txcmd_sxn_txdata},
	{"txcmd_trigdata",	show_txcmd_sxn_trigdata},
	{"txcmd_tf_txd",	show_txcmd_tf_txd},
	{"txcmd_tf_basic",	show_txcmd_tf_basic},
	{"txcmd_sw_fid",	show_txcmd_sw_fid},
	{"txcmd_sw_fid_txd",	show_txcmd_sw_fid_txd},
#endif /* CFG_SUPPORT_FALCON_TXCMD_DBG */

	{"hwctrl", Show_HwCtrlStatistic_Proc},
#ifdef DOT11_HE_AX
	{"colorinfo", show_bsscolor_proc},
#endif

#ifdef VOW_SUPPORT
	/* VOW RX */
	{"vow_rx_time", show_vow_rx_time},
	/* {"vow_get_sta_token", show_vow_get_sta_token}, */
	{"vow_sta_conf", show_vow_sta_conf},
	{"vow_all_sta_conf", show_vow_all_sta_conf},
	{"vow_bss_conf", show_vow_bss_conf},
	{"vow_all_bss_conf", show_vow_all_bss_conf},
	{"vow_info", show_vow_info},

	/* {"vow_status", show_vow_status} */

	/* CR dump */
	{"vow_dump_sta", show_vow_dump_sta},
	{"vow_dump_bss_bitmap", show_vow_dump_bss_bitmap},
	{"vow_dump_bss", show_vow_dump_bss},
	{"vow_dump_vow", show_vow_dump_vow},
	{"vow_show_sta_dtoken", vow_show_sta_dtoken},
	{"vow_show_bss_dtoken", vow_show_bss_dtoken},
	{"vow_show_bss_atoken", vow_show_bss_atoken},
	{"vow_show_bss_ltoken", vow_show_bss_ltoken},

	/* DVT */
	{"vow_show_queue", vow_show_queue_status},

	/*WATF*/
	{"vow_watf_info", show_vow_watf_info},

	/* near far adjustment */
	{"vow_show_near_far", show_vow_near_far},

	{"vow_sch_ctrl", show_vow_schedule_ctrl},

	/* help */
	{"vow_help", show_vow_help},
#endif /* VOW_SUPPORT */
#ifdef RED_SUPPORT
	{"red_info", show_red_info},
#endif/* RED_SUPPORT */
#ifdef FQ_SCH_SUPPORT
	{"fq_info", show_fq_info},
#endif
	{"timer_list", show_timer_list},
	{"wtbl_stat", show_wtbl_state},
#ifdef SMART_CARRIER_SENSE_SUPPORT
#ifdef SCS_FW_OFFLOAD
	{"SCSInfo", Show_SCS_FW_Offload_info_proc},
	{"get_scs_glo_addr",	ShowScsGloAddr},
	{"SCSInfo_2",	ShowSCSinfo_ver2_proc}, /*For SCS_VER2*/
#else
	{"SCSInfo", Show_SCSinfo_proc},
#endif /* SCS_FW_OFFLOAD */
#endif /* SMART_CARRIER_SENSE_SUPPORT */
	{"MibBucket",       Show_MibBucket_Proc},
#ifdef REDUCE_TCP_ACK_SUPPORT
	{"ReduceAckShow",       Show_ReduceAckInfo_Proc},
#endif
#ifdef BAND_STEERING
	{"BndStrgList",		Show_BndStrg_List},
	{"BndStrgInfo",		Show_BndStrg_Info},
#endif /* BAND_STEERING */
	{"radio_info", show_radio_info_proc},
#ifdef BACKGROUND_SCAN_SUPPORT
	{"bgndscaninfo", show_background_scan_info},
#endif
#ifdef ERR_RECOVERY
	{"serinfo",		ShowSerProc},
	{"ser",			ShowSerProc2},
#endif
	{"bcninfo",		ShowBcnProc},
#ifdef RADIUS_MAC_ACL_SUPPORT
	{"RadiusAclCache",      show_RADIUS_acl_cache},
#endif /* RADIUS_MAC_ACL_SUPPORT */
#ifdef DBG_STARVATION
	{"starv_info", show_starv_info_proc},
#endif /*DBG_STARVATION*/
#ifdef ETSI_RX_BLOCKER_SUPPORT
	{"ETSIShowRssiThInfo",		        ShowRssiThInfo},
#endif /* ETSI_RX_BLOCKER_SUPPORT */
#ifdef CONFIG_ATE
	{"ATETXFREQOFFGET", GetATETxFreqOffset},
	{"ATETxDoneInfo", ShowATETxDoneInfo},
	{"ATERUINFO", ShowATERUInfo},
#endif
	{"qdisc_dump", set_qiscdump_proc},
#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT
	{"UpdateEfuse", show_UpdateEfuse_Example},
	{"hwcfg_info", show_hwcfg_proc},
#endif	/*#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT*/
#ifdef EEPROM_RETRIEVE_SUPPORT
    {"e2p", show_e2p_proc},
#endif /* EEPROM_RETRIEVE_SUPPORT */
#ifdef MBO_SUPPORT
	{"mbo",			ShowMboStatProc},
#endif /* MBO_SUPPORT */
#ifdef OCE_SUPPORT
	{"oce",			Show_OceStat_Proc},
#endif /* OCE_SUPPORT */
#ifdef DOT11_SAE_SUPPORT
	{"saeinfo", show_sae_info_proc},
#endif
	{"timelog", show_time_log_info},
	{"l1profile",			ShowL1profile},
#ifdef ANTENNA_CONTROL_SUPPORT
	{"AntCtrl",		Show_Antenna_Control_info},
#endif
#ifdef WIFI_EAP_FEATURE
	{"IPIValue",            ShowIPIValue},
	{"RATblInfo",           show_ratbl_info},
#endif
	{"EDCCAThreshold",      ShowEDCCAThreshold},
	{"EDCCAEnable",		ShowEDCCAEnable},
	{"RuRainfo",   ShowHeraRuRaInfoProc},
	{"MuRainfo",   ShowHeraMuRaInfoProc},
	{"HeraRelatedinfo",   ShowHeraRelatedInfoProc},
	{"show_wifi_cap_list",	show_wifi_cap_list},
#ifdef CFG_SUPPORT_FALCON_SR
	{"srcap", ShowSrCap},
	{"srpara", ShowSrPara},
	{"srind", ShowSrInd},
	{"srinfo", ShowSrInfo},
	{"srcond", ShowSrCond},
	{"srrcpitbl", ShowSrRcpiTbl},
	{"srrcpitblofst", ShowSrRcpiTblOfst},
	{"srqctrl", ShowSrQCtrl},
	{"sribpd", ShowSrIBPD},
	{"srnrt", ShowSrNRT},
	{"srnrtctrl", ShowSrNRTCtrl},
	{"srfnqctrl", ShowSrFNQCtrlAll},
	{"srfrmfilt", ShowSrFrmFiltAll},
	{"srinterpsctrl", ShowSrInterPsCtrlAll},
	{"srinterpsdbg", ShowSrInterPsDbgAll},
	{"srcfgsren", ShowSrCfgSrEnable},
	{"srcfgsrsden", ShowSrCfgSrSdEnable},
	{"srcfgsrbf", ShowSrCfgSrBf},
	{"srcfgsratf", ShowSrCfgSrAtf},
	{"srcfgsrmode", ShowSrCfgSrMode},
	{"srcfgdisrten", ShowSrCfgDISRTEnable},
	{"srcfgdisrtmin", ShowSrCfgDISRTMinRssi},
	{"srcfgtxcq", ShowSrCfgTxcQueue},
	{"srcfgtxcqid", ShowSrCfgTxcQid},
	{"srcfgtxcpath", ShowSrCfgTxcPath},
	{"srcfgac", ShowSrCfgAcMethod},
	{"srcfgsrperiodthr", ShowSrCfgSrPeriodThr},
	{"srcfgsrquerytxd", ShowSrCfgQueryTxDMethod},
	{"srcfgsrsdcg", ShowSrCfgSrSdCgRatio},
	{"srcfgsrsdobss", ShowSrCfgSrSdObssRatio},
	{"srcfgsrprofile", ShowSrCfgProfile},
	{"srcfgfnqen", ShowSrCfgFnqEnable},
	{"srcfgdpden", ShowSrCfgDPDEnable},
	{"srcfgsrtxen", ShowSrCfgSrTxEnable},
	{"srcfgomen", ShowSrCfgObssMonitorEnable},
	{"srcfgsrtxalignen", ShowSrCfgSrTxAlignEnable},
	{"srcfgsrtxalignrssi", ShowSrCfgSrTxAlignRssiThr},
	{"srcfgdabsmode", ShowSrCfgDabsMode},
	{"srcfgdropminmcs", ShowSrCfgDropMinMCS},
	{"srcfgsrdpdthreshold", ShowSrCfgSrDPDThreshold},
	{"srcnt", ShowSrCnt},
	{"srsd", ShowSrSd},
	{"srsrgbm", ShowSrSrgBitmap},
	{"srmeshsrgbm", ShowSrMeshSrgBitmap},
	{"srsiga", ShowSrSiga},
	{"srsigaauto", ShowSrSigaAuto},
	{"srselfsrginfo", ShowSrSelfSrgInfo},
	{"srmeshtopolock", ShowSrMeshTopoLock},
	{"srmeshremotefhrssi", ShowSrMeshRemoteFhRssi},
	{"srmeshremotefhrssiTh", ShowSrMeshFhRssiTh},
	{"srmeshstath", ShowSrMeshstatTh},
	{"srmeshbhsrgbm", ShowSrMeshBHSrgBitmap},
	{"srmeshfhsrgbm", ShowSrMeshFHSrgBitmap},
	{"srmeshbhdlobsspdth", ShowSrMeshBHDownThreshold},
	{"srmeshfhdlobsspdth", ShowSrMeshFHDownThreshold},
	{"srmeshforbitsr", ShowSrMeshBHDownLinkForbidSR},
	{"srmeshremotebhinfo", ShowSrMeshRemoteBhInfo},
	{"srmeshtopo", ShowSrMeshTopo},
	{"srmeshtoporcv", ShowSrMeshTopoUpdateParams},
	{"srmeshuplinkevent", ShowSrMeshUplinkEvent},
	{"srmeshphase", ShowSrMeshPhase},
	{"srmeshulmode", ShowSrMeshUlMode},
	{"srmeshstamoderptlock", ShowSrMeshStaModeRptLock},
	{"srmeshallremotestahe", ShowSrMeshRemoteStaHe},
#endif /* CFG_SUPPORT_FALCON_SR */
	{"aidinfo", show_aid_info},
	{"core_dump", Show_CoreDump_Proc},
	{"fw_sim_section", Show_SimSectionUlm_Proc},
	{"fw_dbg_info", Show_FwDbgInfo_Proc},
	{"bus_dbg_info", Show_BusDbgInfo_Proc},
#ifdef DSCP_PRI_SUPPORT
	{"DscpPri",	Show_Dscp_Pri_Proc},
#endif /*DSCP_PRI_SUPPORT*/
	{"fwcmdtoinfo", show_fwcmd_timeout_info},
#ifdef MGMT_TXPWR_CTRL
		{"mgmt_frame_pwr",	show_mgmt_frame_power},
#endif
#ifdef DATA_TXPWR_CTRL
	{"data_frame_pwr",	show_data_frame_power},
#endif
#ifdef MAP_R2
	{"ts_info", show_traffic_separation_info},
#endif
#ifdef WIFI_CSI_CN_INFO_SUPPORT
	{"get_csi_data", show_csi_data_info},
#endif /* WIFI_CSI_CN_INFO_SUPPORT */
#ifdef ACK_CTS_TIMEOUT_SUPPORT
	{"CCK_ACK_TOUT", show_cck_ack_timeout_porc},
	{"OFDM_ACK_TOUT",	show_ofdm_ack_timeout_proc},
	{"OFDMA_ACK_TOUT",	show_ofdma_ack_timeout_proc},
	{"Distance", 	show_distance_proc},
#endif /*ACK_CTS_TIMEOUT_SUPPORT*/
#ifdef AMPDU_CONF_SUPPORT
	{"AMPDUretrycount", Show_AMPDU_Retry_Count},
#endif
#ifdef CONFIG_6G_SUPPORT
	{"bssmgmt", show_bssmnger_info},
#endif
#ifdef QOS_R1
	{"QoSMapIE",			Show_QoS_MapIE_Proc},
#ifdef DABS_QOS
	{"qos_dabs_table",		Show_qos_dabs_table_Proc},
#endif
#endif
#ifdef CONFIG_WIFI_MSI_SUPPORT
	{"msiinfo", show_msiinfo_proc},
#endif
#ifdef TPC_SUPPORT
#ifdef TPC_MODE_CTRL
	{"tpcinfo", Show_TPC_Info_Proc},
#endif
#endif
#ifdef CONFIG_6G_AFC_SUPPORT
	{"afcparams",          afc_cmd_show_afc_params},
#endif /* CONFIG_6G_AFC_SUPPORT */
	{NULL,}
};

static struct {
	UINT16 idx;
	INT (*phy_stat_proc)(RTMP_ADAPTER*pAd, RTMP_STRING*arg, BOOLEAN fgset);
	RTMP_STRING *name;
} *PRTMP_PRIVATE_PHT_STATE, PRTMP_PRIVATE_PHT_STATE_SUPPORT_PROC[] = {
	{PHY_STATE_TX_PHYRATE, ShowTxPhyRate, "tx rate"},
	{PHY_STATE_RX_PHYRATE, ShowRxPhyRate, "rx rate"},
};

/**
 * @addtogroup embedded_ioctl
 * @{
 * @code AP RX IOCTL
 */

static struct {
	RTMP_STRING *name;
	INT (*rx_proc)(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq);
} *PRTMP_PRIVATE_RX_PROC, RTMP_PRIVATE_RX_SUPPORT_PROC[] = {
	{"stat",          Show_Rx_Statistic},
	{"vector",      Set_Rx_Vector_Control},
	{NULL,}
};

#if defined(WAPP_SUPPORT)
INT mtk_cancel_roc(PRTMP_ADAPTER pAd, struct wifi_dev *wdev);
INT mtk_start_roc(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, struct roc_req *roc);
INT mtk_send_offchannel_action_frame(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, struct action_frm_data *frm);
#endif

/* sync with iwpriv source codes */
#define IWPRIV_HELP_MSG_SIZE 10240
#define IWPRIV_HELP_MSG_LINE_SIZE 100


UINT16 iwprivPhyStatHelp(char *src, UINT16 limitLen)
{
	UINT16 totalLen = 0, tmpLen = 0;
	char line[IWPRIV_HELP_MSG_LINE_SIZE] = {0};
	int ret, left_buf_size;

	totalLen += snprintf(src, limitLen, " available commands:\n");
	for (PRTMP_PRIVATE_PHT_STATE = PRTMP_PRIVATE_PHT_STATE_SUPPORT_PROC;
	PRTMP_PRIVATE_PHT_STATE->idx < PHY_STATE_CMD_NUM; PRTMP_PRIVATE_PHT_STATE++) {
		tmpLen = snprintf(line, IWPRIV_HELP_MSG_LINE_SIZE,
			"\t%d  %s\n", PRTMP_PRIVATE_PHT_STATE->idx, PRTMP_PRIVATE_PHT_STATE->name);
		if (totalLen + tmpLen < limitLen) {
			totalLen += tmpLen;
			left_buf_size = limitLen - strlen(src);
			ret = snprintf(src + strlen(src), left_buf_size, "%s", line);
			if (os_snprintf_error(left_buf_size, ret)) {
				MTWF_PRINT("final_name snprintf error!\n");
				return 1;
			}
		} else {
			return 0;
		}
	}
	MTWF_PRINT("%s iwpriv ra0 phystate read(write)=Idx \n", __func__);
	MTWF_PRINT("%s\n", src);
	return totalLen;
}
void showPhyStatHelpMsg(RTMP_IOCTL_INPUT_STRUCT *pIoctlCmdStr, UINT16 (*handler)(char *, UINT16))
{
	char *msg = pIoctlCmdStr->u.data.pointer;
	UINT16 len = 0;

	len = handler(msg, IWPRIV_HELP_MSG_SIZE);
	if (len == 0)
		return;
	pIoctlCmdStr->u.data.length = len;
}

INT RTMPAPPrivIoctlSet(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *pIoctlCmdStr)
{
	RTMP_STRING *this_char, *value;
	INT Status = NDIS_STATUS_SUCCESS;
	UCHAR *tmp = NULL, *buf = NULL;

#ifdef TP_VXWORKS
#ifdef IWPRIVHELP
	/* iwpriv cmd set u.data.length=1,u.data.pointer="\0" when user does not assign input param */
	if (pIoctlCmdStr->u.data.length == 1 && *pIoctlCmdStr->u.data.pointer == '\0')
	{
		showHelpMsg(pIoctlCmdStr, iwprivSetCmdHelp);
		return Status;
	}
#endif
#endif

	os_alloc_mem(NULL, (UCHAR **)&buf, pIoctlCmdStr->u.data.length + 1);

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, pIoctlCmdStr->u.data.pointer, pIoctlCmdStr->u.data.length)) {
		os_free_mem(buf);
		return -EFAULT;
	}

	/* Play safe - take care of a situation in which user-space didn't NULL terminate */
	buf[pIoctlCmdStr->u.data.length] = 0;
	/* Use tmp to parse string, because strsep() would change it */
	tmp = buf;

	while ((this_char = strsep((char **)&tmp, "\0")) != NULL) {
		if (!*this_char)
			continue;
#ifdef DBG
#ifdef DBG_ENHANCE
		{
			struct wifi_dev *wdev = NULL;
			INT ifIndex, ifType;
			struct net_device *netDev = NULL;
			POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

			ifIndex = pObj->ioctl_if;
			ifType = pObj->ioctl_if_type;

			if (ifIndex >= 0) {
				if (ifType == INT_MAIN || ifType == INT_MBSSID) {
					if (VALID_MBSS(pAd, ifIndex))
						wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
				} else if (ifType == INT_APCLI) {
					if (ifIndex < MAX_MULTI_STA)
						wdev = &pAd->StaCfg[ifIndex].wdev;
				} else if (ifType == INT_WDS) {
					if (ifIndex < MAX_WDS_ENTRY)
						wdev = &pAd->WdsTab.WdsEntry[ifIndex].wdev;
				} else {
				}
			}

			if (wdev) {
				netDev = (struct net_device *) wdev->if_dev;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"CFG: iwpriv set %s %s\n",
					(netDev && netDev->name)?netDev->name:"N/A", this_char);
			}
		}
#endif /* DBG_ENHANCE */
#endif /* DBG */
		value = strchr(this_char, '=');

		if (value != NULL)
			*value++ = 0;

		if (!value
#ifdef WSC_AP_SUPPORT
			&& (
				(strcmp(this_char, "WscStop") != 0) &&
				(strcmp(this_char, "ser") != 0) &&
#ifdef BB_SOC
				(strcmp(this_char, "WscResetPinCode") != 0) &&
#endif
				(strcmp(this_char, "WscGenPinCode") != 0)
			)
#endif /* WSC_AP_SUPPORT */
#ifdef SMART_ANTENNA
			&& (strcmp(this_char, "sa") != 0)
#endif /* SMART_ANTENNA */
		   )
			continue;

		for (PRTMP_PRIVATE_SET_PROC = RTMP_PRIVATE_SUPPORT_PROC; PRTMP_PRIVATE_SET_PROC->name; PRTMP_PRIVATE_SET_PROC++) {
			if (rtstrcasecmp(this_char, PRTMP_PRIVATE_SET_PROC->name) == TRUE) {
				if (!PRTMP_PRIVATE_SET_PROC->set_proc(pAd, value)) {
					/*FALSE:Set private failed then return Invalid argument */
					Status = -EINVAL;
				}

				break;  /*Exit for loop. */
			}
		}

		if (PRTMP_PRIVATE_SET_PROC->name == NULL) {
			/*Not found argument */
			Status = -EINVAL;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "IOCTL::(iwpriv) Command not Support [%s=%s]\n", this_char,
					 value);
			break;
		}
	}

	os_free_mem(buf);
	return Status;
}

INT RTMPAPPrivIoctlShow(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *pIoctlCmdStr)
{
	RTMP_STRING *this_char, *value = NULL;
	INT Status = NDIS_STATUS_SUCCESS;
	UCHAR *tmp = NULL, *buf = NULL;


	os_alloc_mem(NULL, (UCHAR **)&buf, pIoctlCmdStr->u.data.length + 1);

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, pIoctlCmdStr->u.data.pointer, pIoctlCmdStr->u.data.length)) {
		os_free_mem(buf);
		return -EFAULT;
	}

	/* Play safe - take care of a situation in which user-space didn't NULL terminate */
	buf[pIoctlCmdStr->u.data.length] = 0;
	/* Use tmp to parse string, because strsep() would change it */
	tmp = buf;

	while ((this_char = strsep((char **)&tmp, ",")) != NULL) {
		if (!*this_char)
			continue;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Before check, this_char=%s\n", this_char);
		value = strchr(this_char, '=');

		if (value) {
			if (strlen(value) > 1) {
				*value = 0;
				value++;
			} else
				value = NULL;
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "After check, this_char=%s, value=%s\n",
				 this_char, (value == NULL ? "" : value));

		for (PRTMP_PRIVATE_SHOW_PROC = RTMP_PRIVATE_SHOW_SUPPORT_PROC; PRTMP_PRIVATE_SHOW_PROC->name;
			 PRTMP_PRIVATE_SHOW_PROC++) {
			if (rtstrcasecmp(this_char, PRTMP_PRIVATE_SHOW_PROC->name) == TRUE) {
				if (!PRTMP_PRIVATE_SHOW_PROC->set_proc(pAd, value)) {
					/*FALSE:Set private failed then return Invalid argument */
					Status = -EINVAL;
				}

				break;  /*Exit for loop. */
			}
		}

		if (PRTMP_PRIVATE_SHOW_PROC->name == NULL) {
			/*Not found argument */
			Status = -EINVAL;
#ifdef RTMP_RBUS_SUPPORT

			if (pAd->infType == RTMP_DEV_INF_RBUS) {
				for (PRTMP_PRIVATE_SHOW_PROC = RTMP_PRIVATE_SHOW_SUPPORT_PROC; PRTMP_PRIVATE_SHOW_PROC->name; PRTMP_PRIVATE_SHOW_PROC++)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s\n", PRTMP_PRIVATE_SHOW_PROC->name);
			}

#endif /* RTMP_RBUS_SUPPORT */
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "IOCTL::(iwpriv) Command not Support [%s=%s]\n", this_char,
					 value);
			break;
		}
	}

	os_free_mem(buf);
	return Status;
}

#ifdef VENDOR_FEATURE6_SUPPORT
#define	ASSO_MAC_LINE_LEN	(1+19+4+4+4+4+8+7+7+7+7+10+6+6+6+6+7+7+7+1)
VOID RTMPAPGetAssoMacTable(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	UINT64 DataRate = 0;
	INT i;
	char *msg;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pAd);
	UINT msg_len = wtbl_max_num * ASSO_MAC_LINE_LEN;

	os_alloc_mem(NULL, (UCHAR **)&msg, msg_len);

	if (msg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Alloc memory failed\n");
		return;
	}

	memset(msg, 0, msg_len);
	snprintf(msg + strlen(msg), msg_len - strlen(msg), "\n%-19s%-4s%-4s%-4s%-4s%-8s",
			"MAC", "AID", "BSS", "PSM", "WMM", "MIMOPS");
	snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-7s%-7s%-7s%-7s", "RSSI0", "RSSI1", "RSSI2", "RSSI3");
	snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-10s%-6s%-6s%-6s%-6s%-7s%-7s%-7s\n", "PhMd", "BW", "MCS", "SGI", "STBC", "Idle", "Rate", "TIME");

	for (i = 0; i < wtbl_max_num; i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if ((IS_ENTRY_CLIENT(pEntry) || (IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)))
			&& (pEntry->Sst == SST_ASSOC)) {
			if ((strlen(msg) + ASSO_MAC_LINE_LEN) >= msg_len)
				break;

			DataRate = 0;
			DataRate = 0;
			/* getRate(pEntry->HTPhyMode, &DataRate); */
			RtmpDrvMaxRateGet(pAd, pEntry->HTPhyMode.field.MODE, pEntry->HTPhyMode.field.ShortGI,
							  pEntry->HTPhyMode.field.BW, pEntry->HTPhyMode.field.MCS,
							  (pEntry->MaxHTPhyMode.field.MCS >> 4) + 1, &DataRate);
			DataRate /= 500000;
			snprintf(msg + strlen(msg), msg_len - strlen(msg), "%02X:%02X:%02X:%02X:%02X:%02X  ", PRINT_MAC(pEntry->Addr));
			snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-4d", (int)pEntry->Aid);
			snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-4d", (int)pEntry->apidx);
			snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-4d", (int)pEntry->PsMode);
			snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE));
			snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-8d", (int)pEntry->MmpsMode);
			snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-7d%-7d%-7d%-7d", pEntry->RssiSample.AvgRssi[0], pEntry->RssiSample.AvgRssi[1], pEntry->RssiSample.AvgRssi[2], pEntry->RssiSample.AvgRssi[3]);
			snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE));
			snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-6s", get_bw_str(pEntry->HTPhyMode.field.BW));
#ifdef DOT11_VHT_AC
			if (pEntry->HTPhyMode.field.MODE >= MODE_VHT)
				snprintf(msg + strlen(msg), msg_len - strlen(msg), "%dS-M%-3d", ((pEntry->HTPhyMode.field.MCS >> 4) + 1), (pEntry->HTPhyMode.field.MCS & 0xf));
			else
#endif /* DOT11_VHT_AC */
				snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-6d", pEntry->HTPhyMode.field.MCS);

			snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-6d", pEntry->HTPhyMode.field.ShortGI);
			snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-6d", pEntry->HTPhyMode.field.STBC);
			snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-7d", (int)(pEntry->StaIdleTimeout - pEntry->NoDataIdleCount));
			snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-7d", (int)DataRate);
			snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-7d", (int)pEntry->StaConnectTime);
			snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount,
					(pEntry->DebugTxCount) ? ((pEntry->DebugTxCount - pEntry->DebugFIFOCount) * 100 / pEntry->DebugTxCount) : 0);
			snprintf(msg + strlen(msg), msg_len - strlen(msg), "\n");
		}
	}

	/* for compatible with old API just do the printk to console*/
	wrq->u.data.length = strlen(msg);

	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s", msg);

	os_free_mem(msg);
}
#endif /* VENDOR_FEATURE6_SUPPORT */

#if defined(INF_AR9) || defined(BB_SOC)
#if defined(AR9_MAPI_SUPPORT) || defined(BB_SOC)
INT RTMPAPPrivIoctlAR9Show(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *pIoctlCmdStr)
{
	INT Status = NDIS_STATUS_SUCCESS;

	if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_mac_table"))
		RTMPAR9IoctlGetMacTable(pAd, pIoctlCmdStr);
	else if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_stat2"))
		RTMPIoctlGetSTAT2(pAd, pIoctlCmdStr);
	else if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_radio_dyn_info"))
		RTMPIoctlGetRadioDynInfo(pAd, pIoctlCmdStr);

#ifdef WSC_AP_SUPPORT
	else if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_wsc_profile"))
		RTMPAR9IoctlWscProfile(pAd, pIoctlCmdStr);
	else if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_wsc_pincode"))
		RTMPIoctlWscPINCode(pAd, pIoctlCmdStr);
	else if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_wsc_status"))
		RTMPIoctlWscStatus(pAd, pIoctlCmdStr);
	else if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_wps_dyn_info"))
		RTMPIoctlGetWscDynInfo(pAd, pIoctlCmdStr);
	else if (!strcmp(pIoctlCmdStr->u.data.pointer, "get_wps_regs_dyn_info"))
		RTMPIoctlGetWscRegsDynInfo(pAd, pIoctlCmdStr);

#endif
	return Status;
}
#endif /*AR9_MAPI_SUPPORT*/
#endif/*AR9_INF*/

VOID restart_ap(void *wdev_obj)
{
	struct wifi_dev *wdev = wdev_obj;
	BSS_STRUCT *mbss = wdev->func_dev;
	UpdateBeaconHandler(
		wdev->sys_handle,
		wdev,
		BCN_UPDATE_DISABLE_TX);
	if (wifi_sys_linkdown(wdev) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "linkdown fail.\n");
		return;
	}

	if (wifi_sys_close(wdev) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "close fail.\n");
		return;
	}

	if (wifi_sys_open(wdev) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "open fail.\n");
		return;
	}

	if (wifi_sys_linkup(wdev, NULL) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "linkup fail.\n");
		return;
	}

	if (IS_SECURITY(&wdev->SecConfig))
		mbss->CapabilityInfo |= 0x0010;
	else
		mbss->CapabilityInfo &= (~0x0010);

#ifdef MGMT_TXPWR_CTRL
	if (wdev->bPwrCtrlEn)
		wtbl_update_pwr_offset(wdev->sys_handle, wdev);
#endif
	UpdateBeaconHandler(wdev->sys_handle, wdev, BCN_UPDATE_IE_CHG);
}

#ifdef QOS_R1
void qos_set_dscp2up_mapping(PRTMP_ADAPTER pAd, BSS_STRUCT *pMbss)
{
#ifdef DSCP_PRI_SUPPORT
	/*write CR4 for DSCP user prio and flag*/
	if (pMbss->dscp_pri_map_enable) {
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
		if (IS_MT7915(pAd) || IS_MT7986(pAd) ||	IS_MT7916(pAd) || IS_MT7981(pAd)) {
			MtCmdSetDscpPri(pAd, pMbss->mbss_idx);
			set_cp_support_en(pAd, "1");
		}
	}
#endif
#endif /*DSCP_PRI_SUPPORT*/
}
#endif /*QOS_R1*/

INT RTMPAPSetInformation(
	IN PRTMP_ADAPTER pAd,
	INOUT RTMP_IOCTL_INPUT_STRUCT *rq,
	IN INT cmd)
{
	RTMP_IOCTL_INPUT_STRUCT *wrq = (RTMP_IOCTL_INPUT_STRUCT *) rq;
	UCHAR Addr[MAC_ADDR_LEN];
	INT Status = NDIS_STATUS_SUCCESS;
#ifdef SNMP_SUPPORT
	/*snmp */
	UINT						KeyIdx = 0;
	PNDIS_AP_802_11_KEY			pKey = NULL;
	TX_RTY_CFG_STRUC			tx_rty_cfg;
	ULONG						ShortRetryLimit, LongRetryLimit;
	UCHAR						ctmp;
#endif /* SNMP_SUPPORT */
	NDIS_802_11_WEP_STATUS              WepStatus = Ndis802_11WEPDisabled;
	NDIS_802_11_AUTHENTICATION_MODE     AuthMode = Ndis802_11AuthModeMax;
	NDIS_802_11_SSID                    Ssid;
	UINT ifIndex;
#ifdef APCLI_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT
	BOOLEAN apcliEn = FALSE;
	PNDIS_APCLI_802_11_PMKID                  pPmkId = NULL;
	BOOLEAN IEEE8021xState = FALSE;
	BOOLEAN IEEE8021x_required_keys = FALSE;
	UCHAR wpa_supplicant_enable = 0;
	PNDIS_802_11_REMOVE_KEY             pRemoveKey = NULL;
	INT BssIdx, i;
	PNDIS_802_11_WEP pWepKey = NULL;
	PSTA_ADMIN_CONFIG pApCliEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = (MAC_TABLE_ENTRY *)NULL;
	PNDIS_APCLI_802_11_KEY                    pApCliKey = NULL;
	MLME_DISASSOC_REQ_STRUCT DisassocReq;
	MLME_DEAUTH_REQ_STRUCT	DeAuthFrame;
	PULONG pCurrState;
	STA_TR_ENTRY *apcli_tr_entry;
#endif/*WPA_SUPPLICANT_SUPPORT*/
#endif/*APCLI_SUPPORT*/
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;

#ifdef DBG
#ifdef DBG_ENHANCE
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"CFG: IOCTL cmd:0x%X\n", cmd);
#endif /* DBG_ENHANCE */
#endif /* DBG */

	switch (cmd & 0x7FFF) {
#ifdef NF_SUPPORT_V2
		case OID_802_11_SET_NF:
		{
			UCHAR en = 0;
			wrq->u.data.length = sizeof(UCHAR);
			Status = copy_from_user(&en, wrq->u.data.pointer, wrq->u.data.length);
			if(en) {
				HW_NF_UPDATE(pAd, TRUE);		/*send FW cmd for starting NF calculate*/
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\nioctl start calculate NF...\n");
			}
			else {
				HW_NF_UPDATE(pAd, FALSE);		/*send FW cmd for stopping NF calculate*/
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\nioctl stop calculate NF...\n");
				/*clear val*/
				pAd->Avg_NF[BAND0] = 0;
#ifdef DBDC_MODE
				pAd->Avg_NF[BAND1] = 0;
#endif
			}
			break;
		}
#endif
#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
	case OID_GET_CURRENT_RSSI:
	{
		int i = 0;
		MSG_RSSI_LIST Rsp;
		INT32 ifIndex = pObj->ioctl_if;
		struct wifi_dev *wdev;

		if ((pObj->ioctl_if_type != INT_APCLI) || (ifIndex >= MAX_APCLI_NUM)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Invalid Interface Type %d Number %d \n", pObj->ioctl_if_type, ifIndex);
			Status = EFAULT;
			break;
		} else
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

		if (IS_VENDOR10_RSSI_VALID(wdev) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RSSI Feature Disabled \n");
			Status = EFAULT;
			break;
		}

		NdisZeroMemory((void *)&Rsp, sizeof(MSG_RSSI_LIST));

		for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

			if (IS_VALID_ENTRY(pEntry)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, MACSTR" ", MAC2STR(pEntry->Addr));
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%d\n", pEntry->CurRssi);

				Rsp.CurRssi = pEntry->CurRssi;
				COPY_MAC_ADDR(&Rsp.Addr, pEntry->Addr);
				break;
			}
		}

		if (i > MAX_LEN_OF_MAC_TABLE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "No Entry Found \n");
			Status = EFAULT;
			break;
		}

		Status = copy_to_user(wrq->u.data.pointer, &Rsp, wrq->u.data.length);
	}
	break;

	case OID_SET_VENDOR10_RSSI:
	{
		BOOLEAN RssiEnbl;
		UINT32 mac_val = 0;
		INT32 ifIndex = pObj->ioctl_if;
		struct wifi_dev *wdev;

		if ((pObj->ioctl_if_type != INT_APCLI) || (ifIndex >= MAX_APCLI_NUM)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Invalid Interface Type %d Number %d \n", pObj->ioctl_if_type, ifIndex);
			Status = EFAULT;
			break;
		} else
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status	= -EINVAL;
		else {
			Status = copy_from_user(&RssiEnbl, wrq->u.data.pointer, wrq->u.data.length);
			SET_VENDOR10_RSSI_VALID(wdev, RssiEnbl);

			/* RCPI include ACK and Data */
			MAC_IO_READ32(pAd, WTBL_OFF_RMVTCR, &mac_val);

			if (RX_MV_MODE & mac_val)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Current Alg Respond Frame Alg %d\n", mac_val);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Current Alg Respond Frame/Data Alg %d\n", mac_val);

			if (IS_VENDOR10_RSSI_VALID(wdev) && !(RX_MV_MODE & mac_val)) {
				mac_val |= RX_MV_MODE;
				MAC_IO_WRITE32(pAd, WTBL_OFF_RMVTCR, mac_val);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "New Alg Respond Frame/Data %d\n", mac_val);
			} else if ((IS_VENDOR10_RSSI_VALID(wdev) == FALSE) && (RX_MV_MODE & mac_val)) {
				mac_val &= ~RX_MV_MODE;
				MAC_IO_WRITE32(pAd, WTBL_OFF_RMVTCR, mac_val);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "New Alg Respond Frame %d\n", mac_val);
			}
		}
	}
	break;
#endif

#ifdef OFFCHANNEL_SCAN_FEATURE
	case OID_802_11_CHANNELINFO:
	{
		int i = 0;
		struct msg_channel_list *pRsp = NULL;
		UCHAR BandIdx = 0;
		CHANNEL_CTRL *pChCtrl = NULL;
		struct wifi_dev *wdev = NULL;

		os_alloc_mem_suspend(NULL, (UCHAR **)&pRsp, sizeof(struct msg_channel_list));

		if (pRsp == NULL) {
			Status = -ENOMEM;
			break;
		}

		if (pObj->ioctl_if_type == INT_MBSSID) {
			ifIndex = pObj->ioctl_if;
			if (!VALID_MBSS(pAd, ifIndex)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
				os_free_mem(pRsp);
				return FALSE;
			}

			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		} else
			wdev = &pAd->ApCfg.MBSSID[0].wdev;
		BandIdx = HcGetBandByWdev(wdev);
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		NdisZeroMemory((void *)pRsp, sizeof(struct msg_channel_list));
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "(sync) Msg received !! \n");
		pAd->ChannelListNum = pChCtrl->ChListNum;
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			pRsp->CHANNELLIST[i].channel = pChCtrl->ChList[i].Channel;
			pAd->ChannelList[i].Channel = pChCtrl->ChList[i].Channel;
			pRsp->CHANNELLIST[i].channel_idx = i;
			pRsp->CHANNELLIST[i].dfs_req = pChCtrl->ChList[i].DfsReq;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"channel : %d channel_id : %d dfs_req : %d	!! \n",
				pRsp->CHANNELLIST[i].channel,
				pRsp->CHANNELLIST[i].channel_idx, pRsp->CHANNELLIST[i].dfs_req);
		}
		Status = copy_to_user(wrq->u.data.pointer, pRsp, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL,
				Status == NDIS_STATUS_SUCCESS ? DBG_LVL_INFO : DBG_LVL_ERROR,
				"OID_802_11_CHANNELINFO: channel count: %d  IOCTL_Call_Status:%d!! \n",
				pChCtrl->ChListNum, Status);

		if (pRsp)
			os_free_mem(pRsp);
	}
	break;
	case OID_802_11_CURRENT_CHANNEL_INFO:
	{
		CHANNEL_INFO Rsp;
		struct wifi_dev *wdev = NULL;
		INT32 ifIndex = pObj->ioctl_if;
		UCHAR BandIdx = DBDC_BAND0;

		if (pObj->ioctl_if_type == INT_MBSSID) {
			ifIndex = pObj->ioctl_if;
			if (!VALID_MBSS(pAd, ifIndex)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
				return FALSE;
			}

			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		} else
			wdev = &pAd->ApCfg.MBSSID[0].wdev;
		BandIdx = HcGetBandByWdev(wdev);
		memcpy(pAd->ScanCtrl[BandIdx].if_name, wrq->ifr_ifrn.ifrn_name, IFNAMSIZ);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"(sync) Msg received for current channel info!! \n");
		memset(&Rsp, 0, sizeof(CHANNEL_INFO));
		Rsp.channel = wlan_operate_get_prim_ch(wdev);
		Rsp.channel_idx = Channel2Index(pAd, Rsp.channel, BandIdx);
		Rsp.dfs_req = pAd->ChannelList[Rsp.channel_idx].DfsReq;
		Rsp.tx_time = pAd->Ch_Stats[BandIdx].Tx_Time;
		Rsp.rx_time = pAd->Ch_Stats[BandIdx].Rx_Time;
		Rsp.obss_time = pAd->Ch_Stats[BandIdx].Obss_Time;
		Rsp.channel_busy_time = pAd->Ch_BusyTime[BandIdx];
		/* Convert MeasurementDuration to milli seconds */
		Rsp.actual_measured_time = (pAd->ChannelStats.MeasurementDuration / 1000);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"Current channel : %d Current channel_id : %d Current ch dfs_req : %d NF : %d Ch_bsy_time : %u!! \n",
		Rsp.channel, Rsp.channel_idx, Rsp.dfs_req, Rsp.NF, Rsp.channel_busy_time);
		Status = copy_to_user(wrq->u.data.pointer, &Rsp, wrq->u.data.length);
	}
	break;
	case OID_OPERATING_INFO:
	{
		OPERATING_INFO Info;
		INT32 ifIndex = pObj->ioctl_if;
		struct wifi_dev *wdev;

		os_zero_mem(&Info, sizeof(OPERATING_INFO));
		if (pObj->ioctl_if_type == INT_MBSSID) {
			ifIndex = pObj->ioctl_if;
			if (ifIndex < 0 || !VALID_MBSS(pAd, ifIndex)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
				return FALSE;
			}
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		} else
			wdev = &pAd->ApCfg.MBSSID[0].wdev;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"(sync) Msg received for current channel info!! \n");
		Info.cfg_ht_bw = wlan_config_get_ht_bw(wdev);
		Info.cfg_vht_bw = wlan_config_get_vht_bw(wdev);
		Info.RDDurRegion = pAd->CommonCfg.RDDurRegion;
		Info.region = GetCountryRegionFromCountryCode(pAd->CommonCfg.CountryCode);
		Info.channel = wlan_operate_get_prim_ch(wdev);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Channel: %d Info.cfg_ht_bw : %d Info.cfg_vht_bw : %d !! \n",
				 Info.channel, Info.cfg_ht_bw, Info.cfg_vht_bw);
		Status = copy_to_user(wrq->u.data.pointer, &Info, wrq->u.data.length);
	}
	break;
#endif

#ifdef APCLI_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT

	case OID_802_11_SET_IEEE8021X:
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		ifIndex = pObj->ioctl_if;
		apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;

		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status  = -EINVAL;
		else {
			if (apcliEn == TRUE) {
				Status = copy_from_user(&IEEE8021xState, wrq->u.data.pointer, wrq->u.data.length);
				pAd->StaCfg[ifIndex].wdev.IEEE8021X = IEEE8021xState;
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set Apcli(%d)::OID_802_11_SET_IEEE8021X (=%d)\n", ifIndex,
						 IEEE8021xState);
			} else
				Status  = -EINVAL;
		}

		break;

	case OID_802_11_SET_IEEE8021X_REQUIRE_KEY:
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		ifIndex = pObj->ioctl_if;
		apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;

		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status  = -EINVAL;
		else {
			if (apcliEn == TRUE) {
				Status = copy_from_user(&IEEE8021x_required_keys, wrq->u.data.pointer, wrq->u.data.length);
				pAd->StaCfg[ifIndex].wpa_supplicant_info.IEEE8021x_required_keys = IEEE8021x_required_keys;
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set Apcli(%d)::OID_802_11_SET_IEEE8021X_REQUIRE_KEY (%d)\n",
						 ifIndex, IEEE8021x_required_keys);
			} else
				Status  = -EINVAL;
		}

		break;

	case OID_802_11_PMKID:
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		ifIndex = pObj->ioctl_if;
		apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;

		if (!apcliEn)
			return FALSE;

		os_alloc_mem(NULL, (UCHAR **)&pPmkId, wrq->u.data.length);

		if (pPmkId == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(pPmkId, wrq->u.data.pointer, wrq->u.data.length);

		/* check the PMKID information */
		if (pPmkId->BSSIDInfoCount == 0)
			NdisZeroMemory(pAd->StaCfg[ifIndex].SavedPMK, sizeof(BSSID_INFO)*PMKID_NO);
		else {
			PBSSID_INFO	pBssIdInfo;
			UINT		BssIdx;
			UINT		CachedIdx;

			for (BssIdx = 0; BssIdx < pPmkId->BSSIDInfoCount; BssIdx++) {
				/* point to the indexed BSSID_INFO structure */
				pBssIdInfo = (PBSSID_INFO) ((PUCHAR) pPmkId + 2 * sizeof(UINT) + BssIdx * sizeof(BSSID_INFO));

				/* Find the entry in the saved data base. */
				for (CachedIdx = 0; CachedIdx < pAd->StaCfg[ifIndex].SavedPMKNum; CachedIdx++) {
					/* compare the BSSID */
					if (NdisEqualMemory(pBssIdInfo->BSSID, pAd->StaCfg[ifIndex].SavedPMK[CachedIdx].BSSID,
										sizeof(NDIS_802_11_MAC_ADDRESS)))
						break;
				}

				/* Found, replace it */
				if (CachedIdx < PMKID_NO) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Update OID_802_11_PMKID, idx = %d\n", CachedIdx);
					NdisMoveMemory(&pAd->StaCfg[ifIndex].SavedPMK[CachedIdx], pBssIdInfo, sizeof(BSSID_INFO));
					NdisZeroMemory(&pAd->StaCfg[ifIndex].SavedPMK[CachedIdx].PMK, LEN_PMK);
					pAd->StaCfg[ifIndex].SavedPMKNum++;
				}
				/* Not found, replace the last one */
				else {
					/* Randomly replace one */
					CachedIdx = (pBssIdInfo->BSSID[5] % PMKID_NO);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Update OID_802_11_PMKID, idx = %d\n", CachedIdx);
					NdisMoveMemory(&pAd->StaCfg[ifIndex].SavedPMK[CachedIdx], pBssIdInfo, sizeof(BSSID_INFO));
					NdisZeroMemory(&pAd->StaCfg[ifIndex].SavedPMK[CachedIdx].PMK, LEN_PMK);
				}
			}
		}

		if (pPmkId)
			os_free_mem(pPmkId);

		break;

	case RT_OID_WPA_SUPPLICANT_SUPPORT:
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		ifIndex = pObj->ioctl_if;
		apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;

		if (!apcliEn)
			return FALSE;

		if (wrq->u.data.length != sizeof(UCHAR))
			Status  = -EINVAL;
		else {
			Status = copy_from_user(&wpa_supplicant_enable, wrq->u.data.pointer, wrq->u.data.length);

			if (wpa_supplicant_enable & WPA_SUPPLICANT_ENABLE_WPS)
				pAd->StaCfg[ifIndex].wpa_supplicant_info.WpaSupplicantUP |= WPA_SUPPLICANT_ENABLE_WPS;
			else {
				pAd->StaCfg[ifIndex].wpa_supplicant_info.WpaSupplicantUP = wpa_supplicant_enable;
				pAd->StaCfg[ifIndex].wpa_supplicant_info.WpaSupplicantUP &= 0x7F;
			}

			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "APCLI Set::RT_OID_WPA_SUPPLICANT_SUPPORT (=0x%02X)\n",
					 pAd->StaCfg[ifIndex].wpa_supplicant_info.WpaSupplicantUP);
		}

		break;

	case OID_802_11_REMOVE_KEY:
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		ifIndex = pObj->ioctl_if;
		apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;

		if (!apcliEn)
			return FALSE;

		os_alloc_mem(NULL, (UCHAR **)&pRemoveKey, wrq->u.data.length);

		if (pRemoveKey == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(pRemoveKey, wrq->u.data.pointer, wrq->u.data.length);

		if (pRemoveKey->Length != wrq->u.data.length) {
			Status  = -EINVAL;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Set::OID_802_11_REMOVE_KEY, Failed!!\n");
		} else {
			if (pAd->StaCfg[ifIndex].wdev.AuthMode >= Ndis802_11AuthModeWPA) {
				RTMPWPARemoveKeyProc(pAd, pRemoveKey);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_REMOVE_KEY, Remove WPA Key!!\n");
			} else {
				UINT KeyIdx;

				BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + ifIndex;
				KeyIdx = pRemoveKey->KeyIndex;

				if (KeyIdx & 0x80000000) {
					/* Should never set default bit when remove key */
					Status  = -EINVAL;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Set::OID_802_11_REMOVE_KEY, Failed!!(Should never set default bit when remove key)\n");
				} else {
					KeyIdx = KeyIdx & 0x0fffffff;

					if (KeyIdx > 3) {
						Status  = -EINVAL;
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Set::OID_802_11_REMOVE_KEY, Failed!!(KeyId[%d] out of range)\n",
								 KeyIdx);
					} else {
						pAd->StaCfg[ifIndex].SharedKey[KeyIdx].KeyLen = 0;
						pAd->StaCfg[ifIndex].SharedKey[KeyIdx].CipherAlg = CIPHER_NONE;
						AsicRemoveSharedKeyEntry(pAd, BssIdx, (UCHAR)KeyIdx);
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_REMOVE_KEY (id=0x%x, Len=%d-byte)\n",
								 pRemoveKey->KeyIndex, pRemoveKey->Length);
					}
				}
			}
		}

		if (pRemoveKey)
			os_free_mem(pRemoveKey);

		break;

	case OID_802_11_ADD_WEP:
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		ifIndex = pObj->ioctl_if;
		apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;

		if (!apcliEn)
			return FALSE;

		os_alloc_mem(NULL, (UCHAR **)&pWepKey, wrq->u.data.length);

		if (pWepKey == NULL) {
			Status = -ENOMEM;
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Set Apcli::OID_802_11_ADD_WEP, Failed!!\n");
			break;
		}

		BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + ifIndex;
		pApCliEntry = &pAd->StaCfg[ifIndex];
		pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
		Status = copy_from_user(pWepKey, wrq->u.data.pointer, wrq->u.data.length);

		if (Status) {
			Status  = -EINVAL;
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Set::OID_802_11_ADD_WEP, Failed (length mismatch)!!\n");
		} else {
			UINT KeyIdx;

			apcli_tr_entry = &pAd->MacTab.tr_entry[pApCliEntry->MacTabWCID];
			KeyIdx = pWepKey->KeyIndex & 0x0fffffff;

			/* KeyIdx must be 0 ~ 3 */
			if (KeyIdx > 4) {
				Status  = -EINVAL;
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "Set ApCli::OID_802_11_ADD_WEP, Failed (KeyIdx must be smaller than 4)!!\n");
			} else {
				UCHAR CipherAlg = 0;
				PUCHAR Key;
				UINT KeyLenth = 0;
				/* Zero the specific shared key */
				NdisZeroMemory(&pApCliEntry->SharedKey[KeyIdx], sizeof(CIPHER_KEY));
				/* set key material and key length */
				if ((UCHAR)pWepKey->KeyLength > ARRAY_SIZE(pApCliEntry->SharedKey[KeyIdx].Key))
					KeyLenth = ARRAY_SIZE(pApCliEntry->SharedKey[KeyIdx].Key);
				else
					KeyLenth = (UCHAR)pWepKey->KeyLength;
				pApCliEntry->SharedKey[KeyIdx].KeyLen = KeyLenth;
				NdisMoveMemory(pApCliEntry->SharedKey[KeyIdx].Key, &pWepKey->KeyMaterial, KeyLenth);

				switch (pWepKey->KeyLength) {
				case 5:
					CipherAlg = CIPHER_WEP64;
					break;

				case 13:
					CipherAlg = CIPHER_WEP128;
					break;

				default:
					MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Set::OID_802_11_ADD_WEP, only support CIPHER_WEP64(len:5) & CIPHER_WEP128(len:13)!!\n");
					Status = -EINVAL;
					break;
				}

				pApCliEntry->SharedKey[KeyIdx].CipherAlg = CipherAlg;

				/* Default key for tx (shared key) */
				if (pWepKey->KeyIndex & 0x80000000) {
					NdisZeroMemory(&pApCliEntry->wpa_supplicant_info.DesireSharedKey[KeyIdx], sizeof(CIPHER_KEY));
					/* set key material and key length */
					if ((UCHAR)pWepKey->KeyLength > ARRAY_SIZE(pApCliEntry->wpa_supplicant_info.DesireSharedKey[KeyIdx].Key))
						KeyLenth = ARRAY_SIZE(pApCliEntry->wpa_supplicant_info.DesireSharedKey[KeyIdx].Key);
					else
						KeyLenth = (UCHAR)pWepKey->KeyLength;
					pApCliEntry->wpa_supplicant_info.DesireSharedKey[KeyIdx].KeyLen = KeyLenth;
					NdisMoveMemory(pApCliEntry->wpa_supplicant_info.DesireSharedKey[KeyIdx].Key, &pWepKey->KeyMaterial, KeyLenth);
					pApCliEntry->wpa_supplicant_info.DesireSharedKeyId = KeyIdx;
					pApCliEntry->wpa_supplicant_info.DesireSharedKey[KeyIdx].CipherAlg = CipherAlg;
					pApCliEntry->wdev.DefaultKeyId = (UCHAR) KeyIdx;
				}

				if ((pApCliEntry->wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
					(pApCliEntry->wdev.AuthMode >= Ndis802_11AuthModeWPA)) {
					Key = pWepKey->KeyMaterial;
					/* Set Group key material to Asic */
					AsicAddSharedKeyEntry(pAd, BssIdx, KeyIdx, &pApCliEntry->SharedKey[KeyIdx]);
					NdisAcquireSpinLock(&pAd->MacTabLock);
					/* TODO: shiang-usw, need to replace upper setting with tr_entry */
					apcli_tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
					NdisReleaseSpinLock(&pAd->MacTabLock);
				} else if ((pApCliEntry->ApcliInfStat.Valid == TRUE)
						   && (apcli_tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
					Key = pApCliEntry->SharedKey[KeyIdx].Key;
					/* Set key material and cipherAlg to Asic */
					AsicAddSharedKeyEntry(pAd, BssIdx, KeyIdx, &pApCliEntry->SharedKey[KeyIdx]);
				}

				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set ApCli::OID_802_11_ADD_WEP (id=0x%x, Len=%d-byte), %s\n",
						 pWepKey->KeyIndex, pWepKey->KeyLength,
						 (apcli_tr_entry->PortSecured == WPA_802_1X_PORT_SECURED ? "Port Secured" : "Port NOT Secured"));
			}
		}

		if (pWepKey)
			os_free_mem(pWepKey);

		break;

	case OID_802_11_ADD_KEY:
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		ifIndex = pObj->ioctl_if;
		apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;

		if (!apcliEn)
			return FALSE;

		BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + ifIndex;
		pApCliEntry = &pAd->StaCfg[ifIndex];
		pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
		os_alloc_mem(NULL, (UCHAR **)&pApCliKey, wrq->u.data.length);

		if (pApCliKey == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(pApCliKey, wrq->u.data.pointer, wrq->u.data.length);

		if (pApCliKey->Length != wrq->u.data.length) {
			Status  = -EINVAL;
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Set::OID_802_11_ADD_KEY, Failed!!\n");
		} else {
			RTMPApCliAddKey(pAd, ifIndex, pApCliKey);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_ADD_KEY (id=0x%x, Len=%d-byte)\n",
					 pApCliKey->KeyIndex, pApCliKey->KeyLength);
		}

		if (pApCliKey)
			os_free_mem(pApCliKey);

		break;

	case OID_802_11_DISASSOCIATE:
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		ifIndex = pObj->ioctl_if;
		apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;
		pApCliEntry = &pAd->StaCfg[ifIndex];
		pCurrState = &pAd->StaCfg[ifIndex].CtrlCurrState;

		if (!apcliEn || ifIndex >= MAX_APCLI_NUM)
			return FALSE;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_DISASSOCIATE\n");
		DisassocParmFill(pAd, &DisassocReq, pAd->MlmeAux.Bssid, REASON_DISASSOC_STA_LEAVING);
		MlmeEnqueue(pAd, ASSOC_FSM, ASSOC_FSM_MLME_DISASSOC_REQ,
					sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq, ifIndex);

		if (pApCliEntry->ApcliInfStat.Valid)
			ApCliLinkDown(pAd, ifIndex);

		/* set the apcli interface be invalid. */
		pApCliEntry->ApcliInfStat.Valid = FALSE;
		/* clear MlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pAd->MlmeAux.Bssid, MAC_ADDR_LEN);
		pAd->MlmeAux.SsidLen = 0;
		NdisZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		pAd->MlmeAux.Rssi = 0;
		*pCurrState = APCLI_CTRL_DEASSOC;
		break;

	case OID_802_11_DROP_UNENCRYPTED:
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		ifIndex = pObj->ioctl_if;
		apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;
		pApCliEntry = &pAd->StaCfg[ifIndex];
		pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
		apcli_tr_entry = &pAd->MacTab.tr_entry[pApCliEntry->MacTabWCID];

		if (!apcliEn)
			return FALSE;

		if (wrq->u.data.length != sizeof(int))
			Status  = -EINVAL;
		else {
			int enabled = 0;

			Status = copy_from_user(&enabled, wrq->u.data.pointer, wrq->u.data.length);
			NdisAcquireSpinLock(&pAd->MacTabLock);

			if (enabled == 1)
				apcli_tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			else
				apcli_tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

			NdisReleaseSpinLock(&pAd->MacTabLock);
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set ApCLi::OID_802_11_DROP_UNENCRYPTED (=%d)\n", enabled);
		}

		break;

	case OID_SET_COUNTERMEASURES:
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		ifIndex = pObj->ioctl_if;
		apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;
		pApCliEntry = &pAd->StaCfg[ifIndex];
		pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];

		if (!apcliEn)
			return FALSE;

		if (wrq->u.data.length != sizeof(int))
			Status  = -EINVAL;
		else {
			int enabled = 0;

			Status = copy_from_user(&enabled, wrq->u.data.pointer, wrq->u.data.length);

			if (enabled == 1)
				pApCliEntry->bBlockAssoc = TRUE;
			else
				/* WPA MIC error should block association attempt for 60 seconds */
				pApCliEntry->bBlockAssoc = FALSE;

			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set ApCli::OID_SET_COUNTERMEASURES bBlockAssoc=%s\n",
					 pApCliEntry->bBlockAssoc ? "TRUE" : "FALSE");
		}

		break;
#endif/*WPA_SUPPLICANT_SUPPORT*/
#endif/*APCLI_SUPPORT*/
#ifdef P2P_SUPPORT

	case OID_802_11_P2P_MODE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_P2P_MODE\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "wrq->u.data.length =%d sizeof(UCHAR)=%d\n", wrq->u.data.length, sizeof(UCHAR));

		if (wrq->u.data.length != sizeof(UCHAR))
			Status = -EINVAL;
		else {
			UCHAR OpMode = 1;

			Status = copy_from_user(&OpMode, wrq->u.data.pointer, wrq->u.data.length);

			if (OpMode == OPMODE_AP) {
				if (P2P_CLI_ON(pAd))
					P2P_CliStop(pAd);

				if ((!P2P_GO_ON(pAd)) || (P2P_GO_ON(pAd))) {
					P2PCfgInit(pAd);
					P2P_GoStartUp(pAd, MAIN_MBSSID);
				}
			} else if (OpMode == OPMODE_APSTA) {
				if (P2P_GO_ON(pAd))
					P2P_GoStop(pAd);

				if ((!P2P_CLI_ON(pAd)) || (P2P_CLI_ON(pAd))) {
					P2PCfgInit(pAd);
					P2P_CliStartUp(pAd);
					AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod[DBDC_BAND0],
						HW_BSSID_0, OPMODE_AP);
				}
			} else {
				if (P2P_CLI_ON(pAd))
					P2P_CliStop(pAd);
				else if (P2P_GO_ON(pAd)) {
					P2P_GoStop(pAd);

					if (INFRA_ON(pAd))
						AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod[DBDC_BAND0],
							HW_BSSID_0, OPMODE_STA);
				}

				P2PCfgInit(pAd);
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(p2p%d) Set_P2p_OpMode_Proc::(OpMode = %d)\n", pObj->ioctl_if,
					 OpMode);
		}

		break;

	case OID_802_11_P2P_DEVICE_NAME:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_P2P_DEVICE_NAME\n");

		if (wrq->u.data.length > 32)
			Status = -EINVAL;
		else {
			UCHAR DeviceName[MAX_LEN_OF_SSID] = {0};

			NdisZeroMemory(DeviceName, sizeof(DeviceName));
			Status = copy_from_user(&DeviceName, wrq->u.data.pointer, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_P2P_DEVICE_NAME DeviceName=%s\n", DeviceName);
			{
				pAd->P2pCfg.DeviceNameLen = wrq->u.data.length;
				NdisZeroMemory(pAd->P2pCfg.DeviceName, 32);
				NdisMoveMemory(pAd->P2pCfg.DeviceName, DeviceName, pAd->P2pCfg.DeviceNameLen);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Device Name = %s.\n", pAd->P2pCfg.DeviceName);
			}
		}

		break;

	case OID_802_11_P2P_LISTEN_CHANNEL:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_P2P_LISTEN_CHANNEL\n");

		if (wrq->u.data.length != sizeof(UCHAR))
			Status = -EINVAL;
		else {
			/*UCHAR __buf[4]; */
			UCHAR listen_ch;

			Status = copy_from_user(&listen_ch, wrq->u.data.pointer, wrq->u.data.length);
			/*sprintf(__buf, "%d", listen_ch);*/
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "listen_ch = %d.\n", listen_ch);
			{
				/*
				POS_COOKIE			pObj;
				UINT32 channel;

				pObj = (POS_COOKIE) pAd->OS_Cookie;
				if (pObj->ioctl_if_type != INT_P2P)
					return 0;

				channel = (UCHAR) os_str_tol(arg, 0, 10);
				*/
				/* check if this channel is valid */
				if (ChannelSanity(pAd, listen_ch) == TRUE)
					pAd->P2pCfg.ListenChannel = listen_ch;
				else {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Listen Channel out of range, using default.\n");
					pAd->P2pCfg.ListenChannel = 1;
				}

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Listen Channel = %d.\n",
						 pAd->P2pCfg.ListenChannel);
			}
		}

		break;

	case OID_802_11_P2P_OPERATION_CHANNEL:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_P2P_OPERATION_CHANNEL\n");

		if (wrq->u.data.length != sizeof(UCHAR))
			Status = -EINVAL;
		else {
			UCHAR op_ch;

			Status = copy_from_user(&op_ch, wrq->u.data.pointer, wrq->u.data.length);

			/* check if this channel is valid */
			if (ChannelSanity(pAd, op_ch) == TRUE)
				pAd->P2pCfg.GroupChannel = op_ch;
			else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Opertation Channel out of range, using default.\n");
				pAd->P2pCfg.GroupChannel = 1;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Op Channel = %d.\n", pAd->P2pCfg.GroupChannel);
		}

		break;

	case OID_802_11_P2P_GO_INT:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_P2P_GO_INT\n");

		if (wrq->u.data.length != sizeof(UCHAR))
			Status = -EINVAL;
		else {
			UCHAR intent;

			Status = copy_from_user(&intent, wrq->u.data.pointer, wrq->u.data.length);

			/* check if this channel is valid */

			if (intent <= 15)
				pAd->P2pCfg.GoIntentIdx = intent;
			else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "GO Intent out of range 0 ~ 15, using default.\n");
				pAd->P2pCfg.GoIntentIdx = 0;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "GO Intent = %d.\n", pAd->P2pCfg.GoIntentIdx);
		}

		break;

	case OID_802_11_P2P_SCAN:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_P2P_SCAN\n");

		if (wrq->u.data.length != sizeof(UCHAR))
			Status = -EINVAL;
		else {
			UCHAR bScan;

			Status = copy_from_user(&bScan, wrq->u.data.pointer, wrq->u.data.length);

			if (bScan) {
				pAd->StaCfg[0].bAutoReconnect = FALSE;
				P2pScan(pAd);
			} else {
				pAd->StaCfg[0].bAutoReconnect = TRUE;
				P2pStopScan(pAd);
			}
		}

		break;

	case OID_P2P_WSC_PIN_CODE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_P2P_WSC_PIN_CODE wrq->u.data.length=%d\n",
				 wrq->u.data.length);

		if (wrq->u.data.length != 8) /* PIN Code Length is 8 */
			Status = -EINVAL;
		else {
			CHAR PinCode[9] = {0};

			Status = copy_from_user(&PinCode[0], wrq->u.data.pointer, wrq->u.data.length);

			if (Status == 0) {
				if (Set_AP_WscPinCode_Proc(pAd, (RTMP_STRING *) &PinCode[0]) == FALSE)
					Status = -EINVAL;
			}
		}

		break;

	case OID_802_11_P2P_WscMode:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_P2P_WscMode\n");

		if (wrq->u.data.length != sizeof(UCHAR))
			Status = -EINVAL;
		else {
			UCHAR p2pWscMode;

			Status = copy_from_user(&p2pWscMode, wrq->u.data.pointer, wrq->u.data.length);

			/* check if this channel is valid */

			if (p2pWscMode <= 2 && p2pWscMode >= 1)
				pAd->P2pCfg.WscMode = p2pWscMode;
			else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WscMode is invalid, using default.\n");
				pAd->P2pCfg.WscMode = WSC_PIN_MODE; /* PIN */
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WscMode = %s.\n",
					 (p2pWscMode == 1) ? "PIN" : "PBC");
		}

		break;

	case OID_802_11_P2P_WscConf:
		MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_P2P_WscConf\n");

		if (wrq->u.data.length != sizeof(UCHAR))
			Status = -EINVAL;
		else {
			UCHAR method;

			Status = copy_from_user(&method, wrq->u.data.pointer, wrq->u.data.length);

			if (pAd->P2pCfg.WscMode == WSC_PIN_MODE) {
				if (method == 1) {
					/* Display PIN */
					pAd->P2pCfg.Dpid = DEV_PASS_ID_REG;
					pAd->P2pCfg.ConfigMethod =  WSC_CONFMET_DISPLAY;
					MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "    *************************************************\n");
					MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "    *       PIN Code = %08u                     *\n",
							 pAd->StaCfg[0].WscControl.WscEnrolleePinCode);
					MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "    *************************************************\n");
				} else if (method == 2) {
					/* Enter PIN */
					pAd->P2pCfg.Dpid = DEV_PASS_ID_USER;
					pAd->P2pCfg.ConfigMethod =  WSC_CONFMET_KEYPAD;
				}
			} else if (pAd->P2pCfg.WscMode == WSC_PBC_MODE) {
				if (method == 3) {
					pAd->P2pCfg.Dpid = DEV_PASS_ID_PBC;
					pAd->P2pCfg.ConfigMethod = WSC_CONFMET_PBC;
				}
			}

			MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Config Method = %s.\n",
					 decodeConfigMethod(pAd->P2pCfg.ConfigMethod));
		}

		break;

	case OID_802_11_P2P_Link:
		MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_P2P_Link\n");

		if (wrq->u.data.length != sizeof(UCHAR))
			Status = -EINVAL;
		else {
			UCHAR p2pindex;
			PUCHAR	pAddr;

			Status = copy_from_user(&p2pindex, wrq->u.data.pointer, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TabIdx[%d]\n", p2pindex);

			if (p2pindex < pAd->P2pTable.ClientNumber) {
				/*P2PPrintP2PEntry(pAd, P2pTabIdx); */
				/*pAd->P2pCfg.ConnectingIndex = 0; */
				/*if (pAd->P2pTable.Client[P2pTabIdx].P2pClientState == P2PSTATE_DISCOVERY) */
				/*	pAd->P2pTable.Client[P2pTabIdx].P2pClientState = P2PSTATE_CONNECT_COMMAND; */
				/*COPY_MAC_ADDR(pAd->P2pCfg.ConnectingMAC, pAd->P2pTable.Client[P2pTabIdx].addr); */
				/*pAd->P2pTable.Client[P2pTabIdx].StateCount = 10; */
				/*pAd->P2pTable.Client[P2pTabIdx].bValid = TRUE; */
				/*P2pConnect(pAd); */
				pAddr = &pAd->P2pTable.Client[p2pindex].addr[0];
				P2pConnectPrepare(pAd, pAddr, P2PSTATE_CONNECT_COMMAND);
			} else
				MTWF_DBG(pAd, DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Table Idx out of range!\n");
		}

		break;
#endif /* P2P_SUPPORT */

	case OID_802_11_DEAUTHENTICATION:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_DEAUTHENTICATION\n");

		if (wrq->u.data.length != sizeof(MLME_DISCONNECT_STRUCT))
			Status  = -EINVAL;
		else {
			MAC_TABLE_ENTRY *pEntry = NULL;
			MLME_DISCONNECT_STRUCT *pInfo = NULL;
			MLME_QUEUE_ELEM *Elem;

			os_alloc_mem(pAd, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));

			if (Elem == NULL) {
				Status = -ENOMEM;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Set::OID_802_11_DEAUTHENTICATION, Failed!!\n");
				break;
			}

#ifdef APCLI_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT

			if (pObj->ioctl_if_type == INT_APCLI) {
				ifIndex = pObj->ioctl_if;
				apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;
				pApCliEntry = &pAd->StaCfg[ifIndex];
				pCurrState = &pAd->StaCfg[ifIndex].CtrlCurrState;

				if (ifIndex >= MAX_APCLI_NUM) {
					os_free_mem(Elem);
					return FALSE;
				}

				os_alloc_mem(pAd, (UCHAR **)&pInfo, sizeof(MLME_DEAUTH_REQ_STRUCT));
				Status = copy_from_user(pInfo, wrq->u.data.pointer, wrq->u.data.length);
				/* Fill in the related information */
				DeAuthFrame.Reason = (USHORT)pInfo->Reason;
				COPY_MAC_ADDR(DeAuthFrame.Addr, pInfo->Addr);
				MlmeEnqueue(pAd,
							AUTH_FSM,
							AUTH_FSM_MLME_DEAUTH_REQ,
							sizeof(MLME_DISCONNECT_STRUCT),
							&DeAuthFrame,
							ifIndex);

				if (pApCliEntry->ApcliInfStat.Valid)
					ApCliLinkDown(pAd, ifIndex);

				/* set the apcli interface be invalid.*/
				pApCliEntry->ApcliInfStat.Valid = FALSE;
				/* clear MlmeAux.Ssid and Bssid.*/
				NdisZeroMemory(pAd->MlmeAux.Bssid, MAC_ADDR_LEN);
				pAd->MlmeAux.SsidLen = 0;
				NdisZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
				pAd->MlmeAux.Rssi = 0;
				*pCurrState = APCLI_CTRL_DISCONNECTED;

				if (pInfo)
					os_free_mem(pInfo);

				os_free_mem(Elem);
			} else
#endif /* WPA_SUPPLICANT_SUPPORT */
#endif/*APCLI_SUPPORT*/
			{
				pInfo = (MLME_DISCONNECT_STRUCT *) Elem->Msg;
				Status = copy_from_user(pInfo, wrq->u.data.pointer, wrq->u.data.length);
				pEntry = MacTableLookup(pAd, pInfo->addr);

				if (pEntry != NULL) {
					Elem->Wcid = pEntry->wcid;
					cntl_disconnect_request(pEntry->wdev, CNTL_DEAUTH,
						pEntry->Addr, pInfo->reason);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_DEAUTHENTICATION (Reason=%d)\n", pInfo->reason);
				}
				os_free_mem(Elem);
			}
		}

		break;
#ifdef IAPP_SUPPORT

	case RT_SET_IAPP_PID: {
		unsigned IappPid;

		if (sizeof(IappPid) != wrq->u.data.length) {
			Status = -EFAULT;
			break;
		}

		if (copy_from_user(&IappPid, wrq->u.data.pointer, wrq->u.data.length))
			Status = -EFAULT;
		else {
			RTMP_GET_OS_PID(pObj->IappPid, IappPid);
			pObj->IappPid_nr = IappPid;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_SET_APD_PID::(IappPid=%u)\n", IappPid);
		}
	}
	break;
#endif /* IAPP_SUPPORT */
#ifdef DOT11R_FT_SUPPORT

	case RT_SET_FT_STATION_NOTIFY:
	case RT_SET_FT_KEY_REQ:
	case RT_SET_FT_KEY_RSP:
	case RT_FT_KEY_SET:
	case RT_FT_NEIGHBOR_REPORT:
	case RT_FT_NEIGHBOR_REQUEST:
	case RT_FT_NEIGHBOR_RESPONSE:
	case RT_FT_ACTION_FORWARD: {
		UCHAR *pBuffer;

		FT_MEM_ALLOC(pAd, &pBuffer, wrq->u.data.length + 1);

		if (pBuffer == NULL)
			break;

		if (copy_from_user(pBuffer, wrq->u.data.pointer, wrq->u.data.length)) {
			Status = -EFAULT;
			FT_MEM_FREE(pAd, pBuffer);
			break;
		}

		switch (cmd & 0x7FFF) {
		case RT_SET_FT_STATION_NOTIFY:
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "RT_SET_FT_STATION_NOTIFY\n");
			FT_KDP_StationInform(pAd, pBuffer, wrq->u.data.length);
			break;

		case RT_SET_FT_KEY_REQ:
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "RT_SET_FT_KEY_REQ\n");
			FT_KDP_IOCTL_KEY_REQ(pAd, pBuffer, wrq->u.data.length);
			break;

		case RT_SET_FT_KEY_RSP:
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "RT_SET_FT_KEY_RSP\n");
			FT_KDP_KeyResponseToUs(pAd, pBuffer, wrq->u.data.length);
			break;

		case RT_FT_KEY_SET:
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "RT_FT_KEY_SET\n");
			/* Note: the key must be ended by 0x00 */
			pBuffer[wrq->u.data.length] = 0x00;
			FT_KDP_CryptKeySet(pAd, pBuffer, wrq->u.data.length);
			break;

		case RT_FT_NEIGHBOR_REPORT:
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "RT_FT_NEIGHBOR_REPORT\n");
#ifdef FT_KDP_FUNC_INFO_BROADCAST
			FT_KDP_NeighborReportHandle(pAd, pBuffer, wrq->u.data.length);
#endif /* FT_KDP_FUNC_INFO_BROADCAST */
			break;

		case RT_FT_NEIGHBOR_REQUEST:
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "RT_FT_NEIGHBOR_REPORT\n");
			FT_KDP_NeighborRequestHandle(pAd, pBuffer, wrq->u.data.length);
			break;

		case RT_FT_NEIGHBOR_RESPONSE:
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "RT_FT_NEIGHBOR_RESPONSE\n");
			FT_KDP_NeighborResponseHandle(pAd, pBuffer, wrq->u.data.length);
			break;

		case RT_FT_ACTION_FORWARD:
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "[ra%d] RT_FT_ACTION_FORWARD\n", pObj->ioctl_if);
			FT_RRB_ActionHandle(pAd, pObj->ioctl_if, pBuffer, wrq->u.data.length);
			break;
		}

		FT_MEM_FREE(pAd, pBuffer);
	}
	break;

	case OID_802_11R_SUPPORT:
		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status  = -EINVAL;
		else {
			UCHAR apidx = pObj->ioctl_if;
			ULONG value = 0;

			Status = copy_from_user(&value, wrq->u.data.pointer, wrq->u.data.length);
			pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtCapFlag.Dot11rFtEnable = (value == 0 ? FALSE : TRUE);
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "Set::OID_802_11R_SUPPORT(=%d)\n",
					 pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtCapFlag.Dot11rFtEnable);
		}

		break;

	case OID_802_11R_MDID:
		if (wrq->u.data.length != FT_MDID_LEN)
			Status  = -EINVAL;
		else {
			UCHAR apidx = pObj->ioctl_if;

			Status = copy_from_user(pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtMdId, wrq->u.data.pointer, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "Set::OID_802_11R_MDID(=%c%c)\n",
					 pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtMdId[0],
					 pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtMdId[0]);
			/*#ifdef WH_EZ_SETUP
				Dynamic update of MdId in ez security Info not supported currently
			#endif */
		}

		break;

	case OID_802_11R_R0KHID:
		if (wrq->u.data.length <= FT_ROKH_ID_LEN)
			Status  = -EINVAL;
		else {
			UCHAR apidx = pObj->ioctl_if;

			Status = copy_from_user(pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtR0khId, wrq->u.data.pointer, wrq->u.data.length);
			pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtR0khIdLen = wrq->u.data.length;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "Set::OID_802_11R_OID_802_11R_R0KHID(=%s) Len=%d\n",
					 pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtR0khId,
					 pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtR0khIdLen);
		}

		break;

	case OID_802_11R_RIC:
		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status  = -EINVAL;
		else {
			UCHAR apidx = pObj->ioctl_if;
			ULONG value = 0;

			Status = copy_from_user(&value, wrq->u.data.pointer, wrq->u.data.length);
			pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtCapFlag.RsrReqCap = (value == 0 ? FALSE : TRUE);
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "Set::OID_802_11R_RIC(=%d)\n",
					 pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtCapFlag.Dot11rFtEnable);
		}

		break;

	case OID_802_11R_OTD:
		if (wrq->u.data.length != sizeof(BOOLEAN))
			Status  = -EINVAL;
		else {
			UCHAR apidx = pObj->ioctl_if;
			ULONG value = 0;

			Status = copy_from_user(&value, wrq->u.data.pointer, wrq->u.data.length);
			pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtCapFlag.FtOverDs = (value == 0 ? FALSE : TRUE);
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "Set::OID_802_11R_OTD(=%d)\n",
					 pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtCapFlag.Dot11rFtEnable);
		}

		break;
#endif /* DOT11R_FT_SUPPORT */

	case RT_SET_APD_PID: {
		unsigned long apd_pid;

		if (sizeof(apd_pid) != wrq->u.data.length) {
			Status = -EFAULT;
			break;
		}

		if (copy_from_user(&apd_pid, wrq->u.data.pointer, wrq->u.data.length))
			Status = -EFAULT;
		else {
			RTMP_GET_OS_PID(pObj->apd_pid, apd_pid);
			pObj->apd_pid_nr = apd_pid;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_SET_APD_PID::(ApdPid=%lu)\n", apd_pid);
		}
	}
	break;

	case RT_SET_DEL_MAC_ENTRY:
		if (wrq->u.data.length != MAC_ADDR_LEN) {
			Status = -EFAULT;
			break;
		}

		if (copy_from_user(Addr, wrq->u.data.pointer, wrq->u.data.length))
			Status = -EFAULT;
		else {
			MAC_TABLE_ENTRY *pEntry = NULL;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_SET_DEL_MAC_ENTRY::("MACSTR")\n",
					 MAC2STR(Addr));

			if ((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) {
				struct wifi_dev *wdev = NULL;

				ifIndex = pObj->ioctl_if;
				if (!VALID_MBSS(pAd, ifIndex)) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
					return FALSE;
				}

				wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
				pEntry = MacTableLookup2(pAd, Addr, wdev);

			} else
			pEntry = MacTableLookup(pAd, Addr);

			if (pEntry) {
#ifdef MAC_REPEATER_SUPPORT
				/*
					Need to delete repeater entry if this is mac repeater entry.
				*/
				if (pAd->ApCfg.bMACRepeaterEn) {
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;

					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, TRUE, pEntry->Addr, TRUE);
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "Delete ("MACSTR") mac repeater entry\n",
							  MAC2STR(Addr));
					RepeaterDisconnectRootAP(pAd, pReptEntry, APCLI_DISCONNECT_SUB_REASON_APCFG_DEL_MAC_ENTRY);
				}
#endif /* MAC_REPEATER_SUPPORT */
#ifdef DOT11R_FT_SUPPORT

				/*
					If AP send de-auth to Apple STA,
					Apple STA will re-do auth/assoc and security handshaking with AP again.
					@20150313
				*/
				if (IS_FT_RSN_STA(pEntry))
					MacTableDeleteEntry(pAd, pEntry->wcid, Addr);
				else
#endif /* DOT11R_FT_SUPPORT */
					MlmeDeAuthAction(pAd, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
			}
		}

		break;
#ifdef WSC_AP_SUPPORT
#ifdef CON_WPS

	case RT_OID_WSC_SET_CON_WPS_STOP: {
		UCHAR       apidx = pObj->ioctl_if;
		PWSC_UPNP_CTRL_WSC_BAND_STOP pWscUpnpBandStop;
		PWSC_CTRL pWpsCtrl = NULL;
		INT         IsAPConfigured;

		os_alloc_mem(NULL, (UCHAR **)&pWscUpnpBandStop, sizeof(WSC_UPNP_CTRL_WSC_BAND_STOP));

		if (pWscUpnpBandStop) {
			Status = copy_from_user(pWscUpnpBandStop, wrq->u.data.pointer, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CON_WPS BAND_STOP_CMD From[%s], isApCli[%d], is2gBand[%d]\n",
					 pWscUpnpBandStop->ifName, pWscUpnpBandStop->isApCli, pWscUpnpBandStop->is2gBand);

			if (pWscUpnpBandStop->isApCli) {
				UCHAR i;
				struct wifi_dev *apcli_wdev;

				for (i = 0; i < MAX_APCLI_NUM; i++) {
					pWpsCtrl = &pAd->StaCfg[i].wdev.WscControl;
					apcli_wdev = &pAd->StaCfg[i].wdev;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CON_WPS FROM IOCTL: Stop the %s WPS, state [%d]\n",
							 apcli_wdev->if_dev->name, pWpsCtrl->WscState);

					if (pWpsCtrl->WscState != WSC_STATE_OFF) {
						WscStop(pAd, TRUE, pWpsCtrl);
						pWpsCtrl->WscConfMode = WSC_DISABLE;
					}
				}
			} else {
				struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

				pWpsCtrl = &wdev->WscControl;
				IsAPConfigured = pWpsCtrl->WscConfStatus;

				if ((pWpsCtrl->WscConfMode != WSC_DISABLE) &&
					(pWpsCtrl->bWscTrigger == TRUE)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "FROM IOCTL CON_WPS[%d]: Stop the AP Wsc Machine\n", apidx);
					WscBuildBeaconIE(pAd, IsAPConfigured, FALSE, 0, 0, apidx, NULL, 0, AP_MODE);
					WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0,
										apidx, NULL, 0, AP_MODE);
					UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
					WscStop(pAd, FALSE, pWpsCtrl);
				}
			}

			os_free_mem(pWscUpnpBandStop);
		}
	}
	break;
#endif /* CON_WPS */

	case RT_OID_WSC_SET_SELECTED_REGISTRAR: {
		PUCHAR      upnpInfo;
		UCHAR	    apidx = pObj->ioctl_if;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "WSC::RT_OID_WSC_SET_SELECTED_REGISTRAR, wrq->u.data.length=%d!\n", wrq->u.data.length);
			os_alloc_mem(pAd, (UCHAR **)&upnpInfo, wrq->u.data.length);

			if (upnpInfo) {
				int len, Status;

				Status = copy_from_user(upnpInfo, wrq->u.data.pointer, wrq->u.data.length);

				if (Status == NDIS_STATUS_SUCCESS) {
					len = wrq->u.data.length;

					if ((pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfMode & WSC_PROXY)) {
						WscSelectedRegistrar(pAd, upnpInfo, len, apidx);

						if (pAd->ApCfg.MBSSID[apidx].wdev.WscControl.Wsc2MinsTimerRunning == TRUE) {
							BOOLEAN Cancelled;

							RTMPCancelTimer(&pAd->ApCfg.MBSSID[apidx].wdev.WscControl.Wsc2MinsTimer, &Cancelled);
						}

						/* 2mins time-out timer */
						RTMPSetTimer(&pAd->ApCfg.MBSSID[apidx].wdev.WscControl.Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
						pAd->ApCfg.MBSSID[apidx].wdev.WscControl.Wsc2MinsTimerRunning = TRUE;
					}
				}

				os_free_mem(upnpInfo);
			} else
				Status = -EINVAL;

	}
	break;

	case RT_OID_WSC_EAPMSG: {
		RTMP_WSC_U2KMSG_HDR *msgHdr = NULL;
		PUCHAR pUPnPMsg = NULL;
		UINT msgLen = 0, Machine = 0, msgType = 0;
		int retVal, senderID = 0;
		struct wifi_dev *wdev;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WSC::RT_OID_WSC_EAPMSG, wrq->u.data.length=%d, ioctl_if=%d\n",
					 wrq->u.data.length, pObj->ioctl_if);
			msgLen = wrq->u.data.length;
			os_alloc_mem(pAd, (UCHAR **)&pUPnPMsg, msgLen);

			if (pUPnPMsg == NULL)
				Status = -EINVAL;
			else {
				int HeaderLen;
				RTMP_STRING *pWpsMsg;
				UINT WpsMsgLen;
				PWSC_CTRL pWscControl;
				BOOLEAN	bGetDeviceInfo = FALSE;

				NdisZeroMemory(pUPnPMsg, msgLen);
				retVal = copy_from_user(pUPnPMsg, wrq->u.data.pointer, msgLen);

				if (retVal) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"copy from user error!\n");
					os_free_mem(pUPnPMsg);
					Status = -EINVAL;
					break;
				}
				msgHdr = (RTMP_WSC_U2KMSG_HDR *)pUPnPMsg;
				wdev = wdev_search_by_address(pAd, &msgHdr->Addr1[0]);

				if (!wdev) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "cannot find wdev by addr("MACSTR").\n", MAC2STR(msgHdr->Addr1));
					os_free_mem(pUPnPMsg);
					Status = -EINVAL;
					break;
				}

				senderID = get_unaligned((INT32 *)(&msgHdr->Addr2[0]));
				/*senderID = *((int *)&msgHdr->Addr2); */
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_EAPMSG++++++++\n\n");
				hex_dump("MAC::", &msgHdr->Addr3[0], MAC_ADDR_LEN);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_EAPMSG++++++++\n\n");
				HeaderLen = LENGTH_802_11 + LENGTH_802_1_H + sizeof(IEEE8021X_FRAME) + sizeof(EAP_FRAME);
				pWpsMsg = (RTMP_STRING *)&pUPnPMsg[HeaderLen];
				/* take care of a situation in which string isn't NULL terminate */
				pWpsMsg[msgLen - HeaderLen] = 0;
				WpsMsgLen = msgLen - HeaderLen;
				/*assign the STATE_MACHINE type */
				Machine = WSC_STATE_MACHINE;
				msgType = WSC_EAPOL_UPNP_MSG;
				pWscControl = &wdev->WscControl;

				/* If AP is unconfigured, WPS state machine will be triggered after received M2. */
				if ((pWscControl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED)
#ifdef WSC_V2_SUPPORT
					&& (pWscControl->WscV2Info.bWpsEnable || (pWscControl->WscV2Info.bEnableWpsV2 == FALSE))
#endif /* WSC_V2_SUPPORT */
				   ) {
					if (strstr(pWpsMsg, "SimpleConfig") &&
						!pWscControl->EapMsgRunning &&
						!pWscControl->WscUPnPNodeInfo.bUPnPInProgress) {
						/* GetDeviceInfo */
						WscInit(pAd, FALSE, pObj->ioctl_if);
						/* trigger wsc re-generate public key */
						pWscControl->RegData.ReComputePke = 1;
						bGetDeviceInfo = TRUE;
					} else if (WscRxMsgTypeFromUpnp(pAd, pWpsMsg, WpsMsgLen) == WSC_MSG_M2 &&
							   !pWscControl->EapMsgRunning &&
							   !pWscControl->WscUPnPNodeInfo.bUPnPInProgress) {
						/* Check Enrollee Nonce of M2 */
						if (WscCheckEnrolleeNonceFromUpnp(pAd, pWpsMsg, WpsMsgLen, pWscControl)) {
							WscGetConfWithoutTrigger(pAd, pWscControl, TRUE);
							pWscControl->WscState = WSC_STATE_SENT_M1;
						}
					}
				}

				retVal = MlmeEnqueueForWsc(pAd, msgHdr->envID, senderID, Machine, msgType, msgLen, pUPnPMsg, wdev);

				if ((retVal == FALSE) && (msgHdr->envID != 0)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "MlmeEnqueuForWsc return False and envID=0x%x!\n",
							 msgHdr->envID);
					Status = -EINVAL;
				}

				os_free_mem(pUPnPMsg);
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_EAPMSG finished!\n");
	}
	break;

	case RT_OID_WSC_READ_UFD_FILE:
		if (wrq->u.data.length > 0) {
			RTMP_STRING *pWscUfdFileName = NULL;
			UCHAR apIdx = pObj->ioctl_if;
			BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apIdx];
			WSC_CTRL *pWscCtrl = &pMbss->wdev.WscControl;

			os_alloc_mem(pAd, (UCHAR **)&pWscUfdFileName, wrq->u.data.length + 1);

			if (pWscUfdFileName) {
				RTMPZeroMemory(pWscUfdFileName, wrq->u.data.length + 1);

				if (copy_from_user(pWscUfdFileName, wrq->u.data.pointer, wrq->u.data.length))
					Status = -EFAULT;
				else {
					/* take care of a situation in which string isn't NULL terminate */
					pWscUfdFileName[wrq->u.data.length] = 0;

					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_READ_UFD_FILE (WscUfdFileName=%s)\n",
						 pWscUfdFileName);

					if (pWscCtrl->WscConfStatus == WSC_SCSTATE_UNCONFIGURED) {
						if (WscReadProfileFromUfdFile(pAd, apIdx, pWscUfdFileName)) {
							pWscCtrl->WscConfStatus = WSC_SCSTATE_CONFIGURED;
							APStop(pAd, pMbss, AP_BSS_OPER_SINGLE);
							APStartUp(pAd, pMbss, AP_BSS_OPER_SINGLE);
						}
					} else {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_WSC_READ_UFD_FILE: AP is configured.\n");
						Status = -EINVAL;
					}
				}

				os_free_mem(pWscUfdFileName);
			} else
				Status = -ENOMEM;
		} else
			Status = -EINVAL;

		break;

	case RT_OID_WSC_WRITE_UFD_FILE:
		if (wrq->u.data.length > 0) {
			RTMP_STRING *pWscUfdFileName = NULL;
			UCHAR apIdx = pObj->ioctl_if;
			WSC_CTRL *pWscCtrl = &pAd->ApCfg.MBSSID[apIdx].wdev.WscControl;

			os_alloc_mem(pAd, (UCHAR **)&pWscUfdFileName, wrq->u.data.length + 1);

			if (pWscUfdFileName) {
				RTMPZeroMemory(pWscUfdFileName, wrq->u.data.length + 1);

				if (copy_from_user(pWscUfdFileName, wrq->u.data.pointer, wrq->u.data.length))
					Status = -EFAULT;
				else {
					/* take care of a situation in which string isn't NULL terminate */
					pWscUfdFileName[wrq->u.data.length] = 0;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_OID_WSC_WRITE_UFD_FILE (WscUfdFileName=%s)\n",
							 pWscUfdFileName);

					if (pWscCtrl->WscConfStatus == WSC_SCSTATE_CONFIGURED)
						WscWriteProfileToUfdFile(pAd, apIdx, pWscUfdFileName);
					else {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_WSC_WRITE_UFD_FILE: AP is un-configured.\n");
						Status = -EINVAL;
					}
				}

				os_free_mem(pWscUfdFileName);
			} else
				Status = -ENOMEM;
		} else
			Status = -EINVAL;

		break;
	case OID_WSC_UUID:
	case RT_OID_WSC_UUID:
		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			return FALSE;
		}

		if (wrq->u.data.length == (UUID_LEN_STR - 1)) {
			pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.Wsc_Uuid_Str[0] = '\0';
			Status = copy_from_user(&pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.Wsc_Uuid_Str[0],
									wrq->u.data.pointer,
									wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "UUID ASCII string: %s\n",
					 pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.Wsc_Uuid_Str);
		} else if (wrq->u.data.length == UUID_LEN_HEX) {
			UCHAR		ii;

			Status = copy_from_user(&pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.Wsc_Uuid_E[0],
									wrq->u.data.pointer,
									wrq->u.data.length);

			for (ii = 0; ii < 16; ii++)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%02x",
						 (pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.Wsc_Uuid_E[ii] & 0xff));
		} else
			Status = -EINVAL;

		break;
#endif /* WSC_AP_SUPPORT */
	case OID_SET_SSID:

		if (wrq->u.data.length <= MAX_LEN_OF_SSID) {
			struct wifi_dev *wdev;
			BOOLEAN apcliEn;
			PSTA_ADMIN_CONFIG pApCliEntry;
			struct DOT11_H *pDot11h = NULL;
			BSS_STRUCT *pMbss = NULL;

			ifIndex = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
			if (pObj->ioctl_if_type == INT_APCLI) {
				if (ifIndex >= MAX_MULTI_STA) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error station index\n");
					return FALSE;
				}

				pApCliEntry = &pAd->StaCfg[ifIndex];
				/* bring apcli interface down first */
				apcliEn = pApCliEntry->ApcliInfStat.Enable;

				if (apcliEn == TRUE) {
					pApCliEntry->ApcliInfStat.Enable = FALSE;
					ApCliIfDown(pAd);
				}

				pApCliEntry->ApcliInfStat.bPeerExist = FALSE;
				NdisZeroMemory(pApCliEntry->CfgSsid, MAX_LEN_OF_SSID);
				Status = copy_from_user(pApCliEntry->CfgSsid,
							wrq->u.data.pointer,
							wrq->u.data.length);
				pApCliEntry->CfgSsidLen = (UCHAR)wrq->u.data.length;
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"I/F(apcli%d) OID_APCLI_SSID::(Len=%d,Ssid=%s)\n",
					ifIndex, pApCliEntry->CfgSsidLen,
					pApCliEntry->CfgSsid);
				pApCliEntry->ApcliInfStat.Enable = apcliEn;
				break;
			}
#endif
			if (ifIndex < MAX_MBSSID_NUM(pAd)) {
				pMbss = &pAd->ApCfg.MBSSID[ifIndex];
				wdev = &pMbss->wdev;
				NdisZeroMemory(pMbss->Ssid, MAX_LEN_OF_SSID);
				Status = copy_from_user(pMbss->Ssid,
							wrq->u.data.pointer,
							wrq->u.data.length);
				pMbss->SsidLen = wrq->u.data.length;
				if (wdev == NULL) {
					Status = -EINVAL;
					break;
				}
				pDot11h = wdev->pDot11_H;
				if (pDot11h == NULL) {
					Status = -EINVAL;
					break;
				}
#ifdef P2P_SUPPORT
				if (pObj->ioctl_if_type == INT_P2P) {
					if (P2P_GO_ON(pAd)) {
						P2P_GoStop(pAd);
						P2P_GoStartUp(pAd, MAIN_MBSSID);
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"I/F(p2p%d) Set_SSID::(Len=%d,Ssid=%s)\n",
							ifIndex, pMbss->SsidLen, pMbss->Ssid);
					}
				} else
#endif /* P2P_SUPPORT */
				{
					OPSTATUS_CLEAR_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
					ap_send_broadcast_deauth(pAd, wdev);
					if (IS_SECURITY(&wdev->SecConfig))
						pMbss->CapabilityInfo |= 0x0010;
					else
						pMbss->CapabilityInfo &= ~(0x0010);
					APSecInit(pAd, wdev);
					restart_ap(&pMbss->wdev);
					OPSTATUS_SET_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
					if (pAd->CommonCfg.need_fallback == 1 && pMbss->wdev.channel < 15 && WMODE_CAP_2G(pMbss->wdev.PhyMode)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"Need fallback to 20 MHz: pMbss->wdev.channel:%d\n", pMbss->wdev.channel);
						wlan_operate_set_ht_bw(&pMbss->wdev, HT_BW_20, EXTCHA_NONE);
					}
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"I/F(ra%d) Set_SSID::(Len=%d,Ssid=%s)\n",
						ifIndex, pMbss->SsidLen, pMbss->Ssid);

#ifdef QOS_R1
					qos_set_dscp2up_mapping(pAd, pMbss);
#endif /*QOS_R1*/
				}
			}
			break;
		}
		Status = -EINVAL;
		break;

	case OID_SET_PSK:
		{
			struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;
#ifdef MAP_R4
			BSS_STRUCT * pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
			INT i;
#endif /* MAP_R4 */

			if (pSecConfig == NULL) {
				MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"pSecConfig == NULL \n");
				return FALSE;
			}

			if (wrq->u.data.length < 65) {
				Status = copy_from_user(pSecConfig->PSK,
							wrq->u.data.pointer,
							wrq->u.data.length);
				pSecConfig->PSK[wrq->u.data.length] = '\0';
			} else
				pSecConfig->PSK[0] = '\0';
#ifdef MAP_R4
			if (!IS_AKM_DPP(pSecConfig->AKMMap)) {
				for (i = 0; i < MAX_PMKID_COUNT; i++) {
					if ((pAd->ApCfg.PMKIDCache.BSSIDInfo[i].Valid == TRUE)
							&& (pAd->ApCfg.PMKIDCache.BSSIDInfo[i].Mbssidx == pMbss->mbss_idx)) {
						pAd->ApCfg.PMKIDCache.BSSIDInfo[i].Valid = FALSE;
						MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_WARN,
								"%s():Modify PSK and clear PMKID (idx %d)from (mbssidx %d)\n",
								i, pMbss->mbss_idx);
					}
				}
			}
#endif /* MAP_R4 */

			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PSK = %s\n",
				pSecConfig->PSK);
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				WSC_CTRL *pWscControl = NULL;

				if ((pObj->ioctl_if_type == INT_MAIN || pObj->ioctl_if_type == INT_MBSSID)) {
					UCHAR apidx = pObj->ioctl_if;

					pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
				}

#ifdef APCLI_SUPPORT
				else if (pObj->ioctl_if_type == INT_APCLI) {
					UCHAR    apcli_idx = pObj->ioctl_if;

					pWscControl = &pAd->StaCfg[apcli_idx].wdev.WscControl;
				}

#endif /* APCLI_SUPPORT */

				if (pWscControl) {
					NdisZeroMemory(pWscControl->WpaPsk, 64);
					pWscControl->WpaPskLen = 0;
					pWscControl->WpaPskLen = wrq->u.data.length;
					if (pWscControl->WpaPskLen <= 64)
						Status = copy_from_user(pWscControl->WpaPsk,
								wrq->u.data.pointer,
								wrq->u.data.length);
					MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PSK = %s\n",
						pWscControl->WpaPsk);
				}
			}
#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef MAP_R4
#ifdef CONFIG_STA_SUPPORT
#ifdef APCLI_SUPPORT
		if (pObj->ioctl_if_type == INT_APCLI) {
			UCHAR sta_idx = pObj->ioctl_if;

			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						"%s():Delete pmk cache on password change\n");

			if (!IS_AKM_DPP(pSecConfig->AKMMap))
				sta_delete_psk_pmkid_cache_all(pAd, sta_idx);
		}
#endif /* APCLI_SUPPORT */
#endif
#endif /* MAP_R4 */
			break;
		}

#ifdef SNMP_SUPPORT

	case OID_802_11_SHORTRETRYLIMIT:
		if (wrq->u.data.length != sizeof(ULONG))
			Status = -EINVAL;
		else {
			Status = copy_from_user(&ShortRetryLimit, wrq->u.data.pointer, wrq->u.data.length);
			AsicSetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_SHORT, ShortRetryLimit);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_SHORTRETRYLIMIT (ShortRetryLimit=%ld)\n",
					 ShortRetryLimit);
		}

		break;

	case OID_802_11_LONGRETRYLIMIT:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_LONGRETRYLIMIT\n");

		if (wrq->u.data.length != sizeof(ULONG))
			Status = -EINVAL;
		else {
			Status = copy_from_user(&LongRetryLimit, wrq->u.data.pointer, wrq->u.data.length);
			AsicSetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_LONG, LongRetryLimit);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_LONGRETRYLIMIT (,LongRetryLimit=%ld)\n",
					 LongRetryLimit);
		}

		break;

	case OID_802_11_WEPDEFAULTKEYVALUE: {
		UINT KeyIdx;
		UINT KeyLen = 0;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_WEPDEFAULTKEYVALUE\n");

		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			return FALSE;
		}

		os_alloc_mem(pAd, (UCHAR **)&pKey, wrq->u.data.length);

		if (pKey == NULL) {
			Status = -EINVAL;
			break;
		}

		Status = copy_from_user(pKey, wrq->u.data.pointer, wrq->u.data.length);

		/*pKey = &WepKey; */

		if (pKey->Length != wrq->u.data.length) {
			Status = -EINVAL;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Set::OID_802_11_WEPDEFAULTKEYVALUE, Failed!!\n");
		}

		KeyIdx = pKey->KeyIndex & 0x0fffffff;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pKey->KeyIndex =%d, pKey->KeyLength=%d\n", pKey->KeyIndex,
				 pKey->KeyLength);

		/* it is a shared key */
		if (KeyIdx > 4)
			Status = -EINVAL;
		else {
			if ((UCHAR)pKey->KeyLength > ARRAY_SIZE(pAd->SharedKey[ifIndex][pAd->ApCfg.MBSSID[ifIndex].DefaultKeyId].Key))
				KeyLen = ARRAY_SIZE(pAd->SharedKey[ifIndex][pAd->ApCfg.MBSSID[ifIndex].DefaultKeyId].Key);
			else
				KeyLen = (UCHAR)pKey->KeyLength;
			pAd->SharedKey[ifIndex][pAd->ApCfg.MBSSID[ifIndex].DefaultKeyId].KeyLen = KeyLen;
			NdisMoveMemory(&pAd->SharedKey[ifIndex][pAd->ApCfg.MBSSID[ifIndex].DefaultKeyId].Key, &pKey->KeyMaterial, KeyLen);

			if (pKey->KeyIndex & 0x80000000) {
				/* Default key for tx (shared key) */
				pAd->ApCfg.MBSSID[ifIndex].DefaultKeyId = (UCHAR) KeyIdx;
			}

			/*RestartAPIsRequired = TRUE; */
		}

		os_free_mem(pKey);
		break;
	}

	case OID_802_11_WEPDEFAULTKEYID:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_WEPDEFAULTKEYID\n");

		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			return FALSE;
		}

		if (wrq->u.data.length != sizeof(UCHAR))
			Status = -EINVAL;
		else
			Status = copy_from_user(&pAd->ApCfg.MBSSID[ifIndex].DefaultKeyId, wrq->u.data.pointer, wrq->u.data.length);

		break;

	case OID_802_11_CURRENTCHANNEL:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_CURRENTCHANNEL\n");

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
#ifdef DOT1X_SUPPORT

	case OID_802_DOT1X_PMKID_CACHE:
		RTMPIoctlAddPMKIDCache(pAd, wrq);
		break;

	case OID_802_DOT1X_RADIUS_DATA:
		Dot1xIoctlRadiusData(pAd, wrq);
		break;

	case OID_802_DOT1X_WPA_KEY:
		Dot1xIoctlAddWPAKey(pAd, wrq);
		break;

	case OID_802_DOT1X_STATIC_WEP_COPY:
		Dot1xIoctlStaticWepCopy(pAd, wrq);
		break;

	case OID_802_DOT1X_IDLE_TIMEOUT:
		RTMPIoctlSetIdleTimeout(pAd, wrq);
		break;
#ifdef RADIUS_MAC_ACL_SUPPORT

	case OID_802_DOT1X_RADIUS_ACL_NEW_CACHE:
		RTMPIoctlAddRadiusMacAuthCache(pAd, wrq);
		break;

	case OID_802_DOT1X_RADIUS_ACL_DEL_CACHE:
		RTMPIoctlDelRadiusMacAuthCache(pAd, wrq);
		break;

	case OID_802_DOT1X_RADIUS_ACL_CLEAR_CACHE:
		RTMPIoctlClearRadiusMacAuthCache(pAd, wrq);
		break;
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* DOT1X_SUPPORT */

#ifdef OCE_FILS_SUPPORT
	case OID_802_DOT1X_MLME_EVENT:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"######## Recive OID_802_DOT1X_MLME_EVENT #######\n");
		RTMPIoctlStaMlmeEvent(pAd, wrq);
		break;
	case OID_802_DOT1X_RSNE_SYNC:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"######## Recive OID_802_DOT1X_RSNE_SYNC #######\n");
		RTMPIoctlRsneSyncEvent(pAd, wrq);
		break;
	case OID_802_DOT1X_KEY_EVENT:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"######## Recive OID_802_DOT1X_KEY_EVENT #######\n");
		RTMPIoctlKeyEvent(pAd, wrq);
		break;
	case OID_802_DOT1X_PMK_CACHE_EVENT:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			"######## Recive OID_802_DOT1X_PMK_CACHE_EVENT #######\n");
		RTMPIoctlPmkCacheEvent(pAd, wrq);
		break;
#endif /* OCE_FILS_SUPPORT */

	case OID_802_11_AUTHENTICATION_MODE: {
		struct wifi_dev *wdev = NULL;
		BSS_STRUCT *pMbss = NULL;

		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			return FALSE;
		}

		pMbss = &pAd->ApCfg.MBSSID[ifIndex];
		if (wrq->u.data.length != sizeof(NDIS_802_11_AUTHENTICATION_MODE))
			Status  = -EINVAL;
		else {
			UINT32 AKMMap = 0;

			Status = copy_from_user(&AuthMode, wrq->u.data.pointer, wrq->u.data.length);

			if (AuthMode > Ndis802_11AuthModeMax) {
				Status  = -EINVAL;
				break;
			}

			AKMMap = SecAuthModeOldToNew(AuthMode);
#ifdef APCLI_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT

			if (pObj->ioctl_if_type == INT_APCLI) {
				if (ifIndex >= MAX_MULTI_STA) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"error station index\n");
					return FALSE;
				}

				PSTA_ADMIN_CONFIG pApCliEntry = &pAd->StaCfg[ifIndex];

				wdev = &pApCliEntry->wdev;
				apcliEn = pApCliEntry->ApcliInfStat.Enable;

				if (apcliEn) {
					if (wdev->SecConfig.AKMMap != AKMMap) {
						/* Config has changed */
						pApCliEntry->bConfigChanged = TRUE;
					}

					wdev->SecConfig.AKMMap = AKMMap;

					for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
						if (IS_ENTRY_PEER_AP(&pAd->MacTab.Content[i]))
							pAd->MacTab.tr_entry[i].PortSecured  = WPA_802_1X_PORT_NOT_SECURED;
					}

					WPAMakeRSNIE(wdev->wdev_type, &wdev->SecConfig, NULL);
					wdev->SecConfig.PairwiseKeyId  = 0;

					if (IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap))
						wdev->DefaultKeyId = 1;
				}
			} else
#endif/*WPA_SUPPLICANT_SUPPORT*/
#endif/*APCLI_SUPPORT*/
			{
				wdev = &pMbss->wdev;

				if (wdev->SecConfig.AKMMap != AKMMap) {
					/* Config has changed */
					pAd->bConfigChanged = TRUE;
				}

				wdev->SecConfig.AKMMap = AKMMap;
			}

			wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_AUTHENTICATION_MODE (=0x%x)\n",
					 wdev->SecConfig.AKMMap);
		}

		if (wdev) {
			APStop(pAd, pMbss, AP_BSS_OPER_SINGLE);
			APStartUp(pAd, pMbss, AP_BSS_OPER_SINGLE);
		}

		break;
	}

	case OID_802_11_WEP_STATUS:
		if (wrq->u.data.length != sizeof(NDIS_802_11_WEP_STATUS))
			Status  = -EINVAL;
		else {
			UINT32 EncryType = 0;

			Status = copy_from_user(&WepStatus, wrq->u.data.pointer, wrq->u.data.length);
			EncryType = SecEncryModeOldToNew(WepStatus);
#ifdef APCLI_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT

			if (pObj->ioctl_if_type == INT_APCLI) {
				ifIndex = pObj->ioctl_if;
				if (ifIndex >= MAX_MULTI_STA) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error station index\n");
					return FALSE;
				}

				wdev = &pAd->StaCfg[ifIndex].wdev;
				apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;
				pApCliEntry = &pAd->StaCfg[ifIndex];
				pCurrState = &pAd->StaCfg[ifIndex].CtrlCurrState;

				if (ifIndex >= MAX_APCLI_NUM)
					return FALSE;

				if (apcliEn) {
					if (WepStatus <= Ndis802_11GroupWEP104Enabled) {
						if (pAd->StaCfg[ifIndex].wdev.WepStatus != WepStatus) {
							/* Config has changed */
							pAd->StaCfg[ifIndex].bConfigChanged = TRUE;
						}

						pAd->StaCfg[ifIndex].wdev.WepStatus = WepStatus;

						for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
							if (IS_ENTRY_PEER_AP(&pAd->MacTab.Content[i])) {
								INT idx = pAd->MacTab.Content[i].wcid;

								pAd->MacTab.tr_entry[idx].PortSecured = WPA_802_1X_PORT_NOT_SECURED;
							}
						}

						pApCliEntry->PairCipher     = pApCliEntry->wdev.WepStatus;
						pApCliEntry->GroupCipher    = pApCliEntry->wdev.WepStatus;
						pApCliEntry->bMixCipher		= FALSE;

						if (IS_AKM_WPA_CAPABILITY(pAd->StaCfg[ifIndex].wdev.SecConfig.AKMMap))
							pApCliEntry->wdev.SecConfig.PairwiseKeyId = 1;

						WPAMakeRSNIE(pAd->ApCfg.ApCliTab[ifIndex].wdev.wdev_type, &pAd->ApCfg.ApCliTab[ifIndex].wdev.SecConfig, NULL);
					}
				}
			} else
#endif/*WPA_SUPPLICANT_SUPPORT*/
#endif/*APCLI_SUPPORT*/
			{
				BSS_STRUCT *pMbss = NULL;
				struct wifi_dev *wdev = NULL;

				ifIndex = pObj->ioctl_if;
				if (!VALID_MBSS(pAd, ifIndex)) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
					return FALSE;
				}

				pMbss = &pAd->ApCfg.MBSSID[ifIndex];
				wdev = &pMbss->wdev;

				/* Since TKIP, AES, WEP are all supported. It should not have any invalid setting */
				if (EncryType != 0) {
					if (wdev->SecConfig.PairwiseCipher != EncryType) {
						/* Config has changed */
						pAd->bConfigChanged = TRUE;
					}

					wdev->SecConfig.PairwiseCipher = EncryType;

					if (IS_CIPHER_TKIP(EncryType) && IS_CIPHER_CCMP128(EncryType))
						SET_CIPHER_TKIP(wdev->SecConfig.GroupCipher);
					else
						wdev->SecConfig.GroupCipher = EncryType;
				} else {
					Status  = -EINVAL;
					break;
				}

				APStop(pAd, pMbss, AP_BSS_OPER_SINGLE);
				APStartUp(pAd, pMbss, AP_BSS_OPER_SINGLE);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_WEP_STATUS (=0x%x)\n", EncryType);
			}
		}

		break;

	case OID_802_11_SSID:
		os_zero_mem(&Ssid, sizeof(NDIS_802_11_SSID));
		if (wrq->u.data.length != sizeof(NDIS_802_11_SSID))
			Status = -EINVAL;
		else {
			RTMP_STRING *pSsidString = NULL;

			Status = copy_from_user(&Ssid, wrq->u.data.pointer, wrq->u.data.length);

			if (Ssid.SsidLength > MAX_LEN_OF_SSID)
				Status = -EINVAL;
			else {
				if (Ssid.SsidLength == 0)
					Status = -EINVAL;
				else {
					os_alloc_mem(NULL, (UCHAR **)&pSsidString, MAX_LEN_OF_SSID + 1);

					if (pSsidString) {
						NdisZeroMemory(pSsidString, MAX_LEN_OF_SSID + 1);
						NdisMoveMemory(pSsidString, Ssid.Ssid, MAX_LEN_OF_SSID);

						ifIndex = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT
						if (pObj->ioctl_if_type == INT_APCLI) {
							if (ifIndex >= MAX_MULTI_STA) {
								MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
										"error station index\n");
								os_free_mem(pSsidString);
								break;
							}

							apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;

							if (apcliEn)
								Set_ApCli_Ssid_Proc(pAd, pSsidString);
						} else
#endif/*WPA_SUPPLICANT_SUPPORT*/
#endif/*APCLI_SUPPORT*/
						{
							if (!VALID_MBSS(pAd, ifIndex)) {
								MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
										"error AP index\n");
								os_free_mem(pSsidString);
								return FALSE;
							}

							NdisZeroMemory((PCHAR)pAd->ApCfg.MBSSID[ifIndex].Ssid,
										   MAX_LEN_OF_SSID);
							strncpy((PCHAR)pAd->ApCfg.MBSSID[ifIndex].Ssid,
									pSsidString
									, MAX_LEN_OF_SSID);
							pAd->ApCfg.MBSSID[ifIndex].Ssid[MAX_LEN_OF_SSID] =
								(CHAR)'\0';
							pAd->ApCfg.MBSSID[ifIndex].SsidLen =
								strlen(pSsidString);
						}

						os_free_mem(pSsidString);
					} else
						Status = -ENOMEM;
				}
			}
		}

		break;
#ifdef VENDOR_FEATURE6_SUPPORT

	case OID_802_11_PASSPHRASES: {
		INT i;
		BSS_STRUCT *pMBSSStruct;
		INT retval;
		NDIS80211PSK tmpPSK;

		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			return FALSE;
		}

		pObj = (POS_COOKIE) pAd->OS_Cookie;
		NdisZeroMemory(&tmpPSK, sizeof(tmpPSK));
		Status = copy_from_user(&tmpPSK, wrq->u.data.pointer, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_802_11_PASSPHRASE\n");

		for (i = 0 ; i < tmpPSK.WPAKeyLen ; i++)
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%c", tmpPSK.WPAKey[i]);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
		pMBSSStruct = &pAd->ApCfg.MBSSID[ifIndex];
		retval = SetWPAPSKKey(pAd, &tmpPSK.WPAKey[0], tmpPSK.WPAKeyLen, (PUCHAR)pMBSSStruct->Ssid, pMBSSStruct->SsidLen, pMBSSStruct->PMK);

		if (retval == FALSE)
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "PassPhrase Generate Fail\n");

#ifdef WSC_AP_SUPPORT
		NdisZeroMemory(pMBSSStruct->wdev.WscControl.WpaPsk, 64);
		pMBSSStruct->wdev.WscControl.WpaPskLen = tmpPSK.WPAKeyLen;
		NdisMoveMemory(pMBSSStruct->wdev.WscControl.WpaPsk, &tmpPSK.WPAKey[0],
			pMBSSStruct->wdev.WscControl.WpaPskLen);
#endif /* WSC_AP_SUPPORT */
	}
	break;
#endif /* VENDOR_FEATURE6_SUPPORT */
#if defined(WAPP_SUPPORT)
	case OID_802_11_HS_ANQP_RSP: {
		UCHAR *Buf;
		struct anqp_rsp_data *rsp_data;

		os_alloc_mem(NULL, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		rsp_data = (struct anqp_rsp_data *)Buf;
		if (rsp_data->anqp_rsp_len > MAX_MGMT_PKT_LEN) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"rsp_data->anqp_rsp_len too large, size = %d\n", rsp_data->anqp_rsp_len);
		}
		Send_ANQP_Rsp(pAd,
					  rsp_data->peer_mac_addr,
					  rsp_data->anqp_rsp,
					  rsp_data->anqp_rsp_len);
		os_free_mem(Buf);
	}
	break;
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
	case OID_SEND_OFFCHAN_ACTION_FRAME:
	{
		UCHAR *Buf = NULL;
		struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

#ifdef MAP_R3
		struct hdev_obj *obj = wdev->pHObj;
		struct radio_dev  *rdev = obj->rdev;

		if (wdev->forbid_data_tx & (0x1 << MSDU_FORBID_CHANNEL_MISMATCH)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"%s: TX allow called as data tx is forbid\n", __func__);
			/*after update channel resum tx*/
			HcUpdateMSDUTxAllow(rdev);
		}
#endif /* MaP_R3 */

		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		mtk_send_offchannel_action_frame(pAd, wdev, (struct action_frm_data *)Buf);
		os_free_mem(Buf);
	}
	break;
	case OID_802_11_CANCEL_ROC:
	{
		struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
		mtk_cancel_roc(pAd, wdev);
		/* ch_switch_monitor_cancel(pAd, wdev); */
	}
	break;
	case OID_802_11_START_ROC:
	{
		UCHAR *Buf = NULL;
		struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		mtk_start_roc(pAd, wdev, (struct roc_req *)Buf);
		os_free_mem(Buf);
	}
	break;
#endif
#ifdef DPP_SUPPORT
	case OID_802_11_SET_PMK:
	{
		if (pAd->bDppEnable) {
			UCHAR *Buf = NULL;
			struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,
							pObj->ioctl_if, pObj->ioctl_if_type);

			os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);
			Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
			mtk_set_pmk(pAd, wdev, (struct pmk_req *)Buf);
			os_free_mem(Buf);
		} else
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"DPP Disabled please enable it\n");
	}
	break;
#endif /* DPP_SUPPORT */
#ifdef MAP_R3
	case OID_802_11_DEL_CCE_IE:
	{
		if (pAd->bDppEnable) {
			struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,
					pObj->ioctl_if, pObj->ioctl_if_type);

			if (wdev == NULL) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"wdev is null, return!\n");
				break;
			}

			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Del::OID_CCe_IE_DEL\n");

			if (wdev->DPPCfg.cce_ie_len) {
				os_zero_mem(wdev->DPPCfg.cce_ie_buf, 6);
				wdev->DPPCfg.cce_ie_len = 0;
				if (wdev->bAllowBeaconing)
					UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
			} else {
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"CCE IE not enabled for %s wdev\n", RtmpOsGetNetDevName(wdev->if_dev));
			}

		} else
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"DPP Disabled please enable it\n");
	}
	break;
	case OID_MAP_R3_DPP_URI:
	{
		if (pAd->bDppEnable) {
			UCHAR *Buf = NULL;
			struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,
							pObj->ioctl_if, pObj->ioctl_if_type);

			if (wdev == NULL) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"wdev is null, return!\n");
				break;
			}

			/* If len coming from wapp is zero then clear the    *
			 * existing allocated buffer so legacy WPS can work  *
			 */
			if (wrq->u.data.length == 0) {
				if (pAd->dpp_uri_ptr) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"dpp uri ptr is not NULL and len is zero!\n");
					os_free_mem(pAd->dpp_uri_ptr);
					pAd->dpp_uri_ptr = NULL;
				} else
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"dpp uri ptr is already NULL and len is zero!\n");
				break;
			}

			os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);
			Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);

			if (pAd->dpp_uri_ptr) {
				/* URI already present now need to replace it if not same*/
				if (memcmp(pAd->dpp_uri_ptr, Buf, pAd->dpp_uri_len) != 0) {
					os_free_mem(pAd->dpp_uri_ptr);
					pAd->dpp_uri_ptr = NULL;
				}
			}

			if (pAd->dpp_uri_ptr == NULL) {
				os_alloc_mem(pAd->dpp_uri_ptr, (UCHAR **)&pAd->dpp_uri_ptr, wrq->u.data.length);

				if (pAd->dpp_uri_ptr == NULL) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"dpp uri ptr is null, return!\n");
					os_free_mem(Buf);
					Buf = NULL;
					break;
				}
				pAd->dpp_uri_len = (UCHAR)wrq->u.data.length;
				memcpy(pAd->dpp_uri_ptr, Buf, wrq->u.data.length);
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"URI copied for wdev:%s URI:%s len:%u\n",
					RtmpOsGetNetDevName(wdev->if_dev), pAd->dpp_uri_ptr, pAd->dpp_uri_len);
			} else {
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"URI already present for wdev:%s URI:%s len:%u\n",
					RtmpOsGetNetDevName(wdev->if_dev), pAd->dpp_uri_ptr, pAd->dpp_uri_len);
				os_free_mem(Buf);
				Buf = NULL;
				break;
			}
			if (Buf) {
				os_free_mem(Buf);
				Buf = NULL;
			}
		} else
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"DPP Disabled please enable it\n");
	}
	break;

	case OID_MAP_R3_1905_SEC_ENABLED:
	{
		UCHAR *Buf = NULL;
		struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,
				pObj->ioctl_if, pObj->ioctl_if_type);

		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"wdev is null, return!\n");
			break;
		}

		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);

		if (!Buf) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"alloc mem fail!\n");
			break;
		}

		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);

		pAd->map_sec_enable = *Buf;
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"setting the security flag %u for wdev:%s\n",
				 pAd->map_sec_enable, RtmpOsGetNetDevName(wdev->if_dev));
		os_free_mem(Buf);

	}
	break;
	case OID_MAP_R3_ONBOARDING_TYPE:
	{
		UCHAR *Buf = NULL;
		struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,
				pObj->ioctl_if, pObj->ioctl_if_type);

		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"wdev is null, return!\n");
			break;
		}

		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);

		if (!Buf) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"alloc mem fail!\n");
			break;
		}

		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);

		pAd->map_onboard_type = *Buf;
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"setting the onboarding type %u for wdev:%s\n",
				 pAd->map_onboard_type, RtmpOsGetNetDevName(wdev->if_dev));
		os_free_mem(Buf);
	}
	break;
#endif /* MAP_R3 */
#ifdef QOS_R1
	case OID_SEND_QOS_ACTION_FRAME:
	{
		UCHAR *Buf = NULL;
		struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);
		if (!Buf) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"alloc mem fail for send action frame.\n");
			break;
		}
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		QoS_send_action_frame(pAd, wdev, (struct action_frm_data *)Buf);
		os_free_mem(Buf);
	}
	break;
#ifdef MSCS_PROPRIETARY
	case OID_SEND_QOS_UP_TUPLE_EXPIRED_NOTIFY:
	{
		UCHAR *Buf = NULL;
		MAC_TABLE_ENTRY	*pEntry;
		struct wapp_vend_spec_classifier_para_report *classifier_para = NULL;

		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);
		if (!Buf) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"alloc mem fail for up tuple.\n");
			break;
		}
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"OID_SEND_QOS_UP_TUPLE_EXPIRED_NOTIFY in driver.\n");

		classifier_para = (struct wapp_vend_spec_classifier_para_report *)Buf;
		if (classifier_para->id > MAX_QOS_PARAM_TBL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"para id is out of range.\n");
		} else {
			pEntry = MacTableLookup(pAd, classifier_para->sta_mac);
			if (pEntry) {
				delete_qos_param(pAd, classifier_para->id);
				QoS_send_mscs_rsp(pAd, pEntry->wcid, 0, TCLAS_PROCESSING_TERMINATED, classifier_para);
			}
		}
		os_free_mem(Buf);
	}
	break;
#endif/*MSCS_PROPRIETARY*/
#endif
#ifdef CONFIG_HOTSPOT_R2
	case OID_802_11_HS_ONOFF: {
		UCHAR *Buf = NULL;
		struct hs_onoff *onoff;

		os_alloc_mem(NULL, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		onoff = (struct hs_onoff *)Buf;
		Set_HotSpot_OnOff(pAd, onoff->hs_onoff, onoff->event_trigger, onoff->event_type);
		os_free_mem(Buf);
	}
	break;

	case OID_802_11_HS_RESET_RESOURCE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "hotspot reset some resource\n");
		Clear_Hotspot_All_IE(pAd);
		/* Clear_All_PROXY_TABLE(pAd); */
		break;

	case OID_802_11_HS_SASN_ENABLE: {
		UCHAR *Buf = NULL;
		PHOTSPOT_CTRL pHSCtrl = NULL;
		struct wifi_dev *wdev = NULL;

		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			return FALSE;
		}

		pHSCtrl =  &pAd->ApCfg.MBSSID[ifIndex].HotSpotCtrl;

		os_alloc_mem(NULL, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		pHSCtrl->bASANEnable = Buf[0];

		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is null, return!\n");
			os_free_mem(Buf);
			break;
		}

		if (wdev->if_up_down_state == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "interface is down, return!\n");
			os_free_mem(Buf);
			break;
		}

		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
		/* for 7615 offload to CR4 */
		hotspot_update_bssflag(pAd, fgASANEnable, Buf[0], pHSCtrl);
		hotspot_update_bss_info_to_cr4(pAd, pObj->ioctl_if, wdev->bss_info_argument.ucBssIndex);
		os_free_mem(Buf);
	}
	break;

	case OID_802_11_BSS_LOAD: {
		UCHAR *Buf = NULL;
		PHOTSPOT_CTRL pHSCtrl;

		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			return FALSE;
		}

		pHSCtrl =  &pAd->ApCfg.MBSSID[ifIndex].HotSpotCtrl;

		os_alloc_mem(NULL, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		pHSCtrl->QLoadTestEnable = Buf[0];
		pHSCtrl->QLoadCU = Buf[1];
		memcpy(&pHSCtrl->QLoadStaCnt, &Buf[2], 2);
		os_free_mem(Buf);
	}
	break;
#endif
#endif
#ifdef CONFIG_DOT11V_WNM
	case OID_802_11_WNM_COMMAND: {
		UCHAR *Buf = NULL;
		struct wnm_command *cmd_data;

		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);

		if (!Buf) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"alloc mem fail.\n");
			break;
		}

		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		cmd_data = (struct wnm_command *)Buf;
		if (cmd_data->command_len > MAX_MGMT_PKT_LEN) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd_data->command_len too large, size = %d\n", cmd_data->command_len);
		}

		if (wnm_handle_command(pAd, cmd_data) != NDIS_STATUS_SUCCESS)
			Status = -EINVAL;

		os_free_mem(Buf);
	}
	break;
#ifdef WAPP_SUPPORT
	case OID_802_11_WAPP_IE: {
		UCHAR *IE = NULL;

		os_alloc_mem(NULL, (UCHAR **)&IE, wrq->u.data.length);
		Status = copy_from_user(IE, wrq->u.data.pointer, wrq->u.data.length);
		wapp_set_ap_ie(pAd, IE, wrq->u.data.length, (UCHAR)pObj->ioctl_if);
		os_free_mem(IE);
	}
	break;
	case OID_802_11_WAPP_PARAM_SETTING: {
		UCHAR *Buf = NULL;
		struct wapp_param_setting *param_setting;

		os_alloc_mem(NULL, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		param_setting = (struct wapp_param_setting *)Buf;
		set_wapp_param(pAd, param_setting->param, param_setting->value);
		os_free_mem(Buf);
	}
	break;
	case OID_802_11_INTERWORKING_ENABLE: {
		UCHAR *Buf = NULL;
		PGAS_CTRL pGASCtrl;

		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,"error AP index\n");
			return FALSE;
		}

		pGASCtrl = &pAd->ApCfg.MBSSID[ifIndex].GASCtrl;

		os_alloc_mem(NULL, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		pGASCtrl->b11U_enable = Buf[0] ? TRUE : FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"%s GAS service for MBSSID[%d]\n", pGASCtrl->b11U_enable?"Enable":"Disable", ifIndex);
		os_free_mem(Buf);
	}
	break;

	case OID_802_11_WNM_BTM_REQ: {
		UCHAR *Buf = NULL;
		MAC_TABLE_ENTRY *pEntry = NULL;
		struct btm_req_data *req_data;

		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			return FALSE;
		}

		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		req_data = (struct btm_req_data *)Buf;
		if (req_data->btm_req_len > MAX_MGMT_PKT_LEN) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"req_data->btm_req_len too large, size = %d\n", req_data->btm_req_len);
		}
		pEntry = MacTableLookup(pAd, req_data->peer_mac_addr);

		if (pEntry == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "NO ENTRY!!!!!!\n");
		} /*else if (pEntry->BssTransitionManmtSupport != 1) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 "OID_802_11_WNM_BTM_REQ::the peer does not support BTM(%d)\n",
					  pEntry->BssTransitionManmtSupport);
		} */else if (IS_AKM_OPEN(pAd->ApCfg.MBSSID[ifIndex].wdev.SecConfig.AKMMap) ||
				   ((pEntry->SecConfig.Handshake.WpaState == AS_PTKINITDONE) &&
					(pEntry->SecConfig.Handshake.GTKState == REKEY_ESTABLISHED))) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "btm1\n");
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "peer_mac_addr="MACSTR"\n", MAC2STR(req_data->peer_mac_addr));
			Send_BTM_Req(pAd,
						 req_data->peer_mac_addr,
						 req_data->btm_req,
						 req_data->btm_req_len);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!btm2\n");
			pEntry->IsBTMReqValid = TRUE;
			os_alloc_mem(pEntry->ReqbtmData, (UCHAR **)&pEntry->ReqbtmData, sizeof(struct btm_req_data) + req_data->btm_req_len);
			memcpy(pEntry->ReqbtmData, Buf, sizeof(struct btm_req_data) + req_data->btm_req_len);
		}

		os_free_mem(Buf);
	}
	break;

#ifdef CONFIG_STA_SUPPORT
	case OID_802_11_WNM_BTM_RSP: {
		UCHAR *Buf = NULL;
		MAC_TABLE_ENTRY *pEntry = NULL;
		struct btm_rsp_data *rsp_data;
		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		rsp_data = (struct btm_rsp_data *)Buf;
		if (rsp_data->btm_rsp_len > MAX_MGMT_PKT_LEN) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"rsp_data->btm_rsp_len too large, size = %d\n", rsp_data->btm_rsp_len);
		}
		pEntry = MacTableLookup(pAd, rsp_data->peer_mac_addr);

		if (pEntry == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "NO ENTRY!!!!!!\n");
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"peer_mac_addr="MACSTR"\n",	MAC2STR(rsp_data->peer_mac_addr));
			Send_BTM_Rsp(pAd,
					rsp_data->peer_mac_addr,
					rsp_data->btm_rsp,
					rsp_data->btm_rsp_len);
		}
		os_free_mem(Buf);
	}
	break;
#endif

#ifdef OCE_SUPPORT
	case OID_802_11_OCE_REDUCED_NEIGHBOR_REPORT: {/*pure 1x is enabled. */
		struct wifi_dev *wdev;
		REDUCED_NR_LIST_INFO ReducedNRListInfo;
		BSS_STRUCT *pMBSS;
		UINT32 ValueLen = 0;

		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			return FALSE;
		}

		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

		if (IS_OCE_RNR_ENABLE(wdev)) {
			pAd->ApCfg.MBSSID[ifIndex].ReducedNRListExist  = TRUE;
			NdisZeroMemory(&ReducedNRListInfo, sizeof(REDUCED_NR_LIST_INFO));
			Status = copy_from_user(&ReducedNRListInfo, wrq->u.data.pointer, wrq->u.data.length);
			pMBSS = &pAd->ApCfg.MBSSID[ifIndex];
			if (ReducedNRListInfo.ValueLen > ARRAY_SIZE(pMBSS->ReducedNRListInfo.Value))
				ValueLen = ARRAY_SIZE(pMBSS->ReducedNRListInfo.Value);
			else
				ValueLen = ReducedNRListInfo.ValueLen;
			NdisMoveMemory(pMBSS->ReducedNRListInfo.Value,
					ReducedNRListInfo.Value, ValueLen);
			pMBSS->ReducedNRListInfo.ValueLen = ValueLen;
			UpdateBeaconHandler(
				pAd,
				wdev,
				BCN_UPDATE_IE_CHG);
		}
	}
	break;
#endif /*OCE_SUPPORT*/

	case OID_802_11_WNM_NOTIFY_REQ: {
		UCHAR *Buf = NULL;
		MAC_TABLE_ENTRY *pEntry;
		struct wnm_req_data *req_data;

		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			return FALSE;
		}

		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		req_data = (struct wnm_req_data *)Buf;
		if (req_data->wnm_req_len > MAX_MGMT_PKT_LEN) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"req_data->wnm_req_len too large, size = %d\n", req_data->wnm_req_len);
		}
		pEntry = MacTableLookup(pAd, req_data->peer_mac_addr);

		if (pEntry == NULL) {
		} else if (
			(IS_AKM_OPEN(pAd->ApCfg.MBSSID[ifIndex].wdev.SecConfig.AKMMap)) ||
				   ((pEntry->SecConfig.Handshake.WpaState == AS_PTKINITDONE) &&
					(pEntry->SecConfig.Handshake.GTKState == REKEY_ESTABLISHED))) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wnm1\n");
			Send_WNM_Notify_Req(pAd,
								req_data->peer_mac_addr,
								req_data->wnm_req,
								req_data->wnm_req_len,
								req_data->type);
		}
#ifdef CONFIG_HOTSPOT_R2
		else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wnm2\n");
			pEntry->IsWNMReqValid = TRUE;
			os_alloc_mem(pEntry->ReqData, (UCHAR **)&pEntry->ReqData, sizeof(struct wnm_req_data) + req_data->wnm_req_len);
			memcpy(pEntry->ReqData, Buf, sizeof(struct wnm_req_data) + req_data->wnm_req_len);
		}
#endif /* CONFIG_HOTSPOT_R2 */

		os_free_mem(Buf);
	}
	break;

#ifdef CONFIG_HOTSPOT_R2
	case OID_802_11_QOSMAP_CONFIGURE: {
		UCHAR *Buf = NULL;
		MAC_TABLE_ENTRY *pEntry;
		struct qosmap_data *req_data;
		unsigned int i;
		UCHAR PoolID = 0;

		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		req_data = (struct qosmap_data *)Buf;
		pEntry = MacTableLookup(pAd, req_data->peer_mac_addr);

		if (pEntry != NULL) {
			/* clear previous data */
			pEntry->DscpExceptionCount = 0;
			memset(pEntry->DscpRange, 0xff, 16);
			memset(pEntry->DscpException, 0xff, 42);
			pEntry->DscpExceptionCount = req_data->qosmap_len - 16;
			memcpy((UCHAR *)pEntry->DscpRange, &req_data->qosmap[pEntry->DscpExceptionCount], 16);

			if (pEntry->DscpExceptionCount != 0)
				memcpy((UCHAR *)pEntry->DscpException, req_data->qosmap, pEntry->DscpExceptionCount);

			PoolID = hotspot_qosmap_add_pool(pAd, pEntry);
			hotspot_qosmap_update_sta_mapping_to_cr4(pAd, pEntry, PoolID);
			Send_QOSMAP_Configure(pAd,
								  req_data->peer_mac_addr,
								  req_data->qosmap,
								  req_data->qosmap_len,
								  pEntry->func_tb_idx);
		} else if ((req_data->peer_mac_addr[0] == 0)
				   && (req_data->peer_mac_addr[1] == 0)
				   && (req_data->peer_mac_addr[2] == 0)
				   && (req_data->peer_mac_addr[3] == 0)
				   && (req_data->peer_mac_addr[4] == 0)
				   && (req_data->peer_mac_addr[5] == 0)) {
			/* Special MAC 00:00:00:00:00:00 for HS2 QoS Map Change using. */
			for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
				pEntry = &pAd->MacTab.Content[i];

				if ((IS_ENTRY_CLIENT(pEntry))
					&& (pEntry->Sst == SST_ASSOC)) {
					if (pEntry->QosMapSupport) {
						pEntry->DscpExceptionCount = 0;
						memset(pEntry->DscpRange, 0xff, 16);
						memset(pEntry->DscpException, 0xff, 42);
						pEntry->DscpExceptionCount = req_data->qosmap_len - 16;
						memcpy((UCHAR *)pEntry->DscpRange,
							   &req_data->qosmap[pEntry->DscpExceptionCount], 16);

						if (pEntry->DscpExceptionCount != 0) {
							memcpy((UCHAR *)pEntry->DscpException,
								   req_data->qosmap, pEntry->DscpExceptionCount);
						}

						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
								 "send QoS map frame: apidx=%d\n", pEntry->func_tb_idx);
						Send_QOSMAP_Configure(pAd,
											  pEntry->Addr,
											  req_data->qosmap,
											  req_data->qosmap_len,
											  pEntry->func_tb_idx);
					}
				}
			}
		}

		os_free_mem(Buf);
	}
	break;
#endif /* CONFIG_HOTSPOT_R2 */
#endif /* WAPP_SUPPORT */
#endif
#ifdef	DOT11K_RRM_SUPPORT

	case OID_802_11_RRM_COMMAND:
		Status = rrm_MsgHandle(pAd, wrq);

		if (Status != NDIS_STATUS_SUCCESS)
			Status = -NDIS_STATUS_FAILURE;

		break;
#endif
#ifdef MBO_SUPPORT
	case OID_802_11_MBO_MSG:
	{
		UCHAR *Buf = NULL;
		struct wapp_param_setting *param_setting;

		os_alloc_mem(NULL, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		if (Status) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"copy_from_user return status received = failure\n");
			Status = -NDIS_STATUS_FAILURE;
		}
		param_setting = (struct wapp_param_setting *)Buf;
		MBO_MsgHandle(pAd, param_setting->param, param_setting->value);
		os_free_mem(Buf);
	}
		break;
#endif /* MBO_SUPPORT */
#if defined(CONFIG_6G_SUPPORT) && defined(BSSMGR_CROSS_MODULE_SUPPORT)
	case OID_BSS_MGMT_SET_EVENT:
	{
		UCHAR *Buf = NULL;
		struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

		os_alloc_mem(NULL, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		if (Status) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"copy_from_user return status received = failure\n");
			Status = -NDIS_STATUS_FAILURE;
		}
		bssmnger_set_event_handle(pAd, wdev, Buf);
		os_free_mem(Buf);
	}
		break;
#endif /* CONFIG_6G_SUPPORT && BSSMGR_CROSS_MODULE_SUPPORT */
#ifdef WAPP_SUPPORT
	case OID_WAPP_EVENT:
	{
		UCHAR *buf = NULL;
		struct wapp_req *req;

		os_alloc_mem(NULL, (UCHAR **)&buf, wrq->u.data.length);
		Status = copy_from_user(buf, wrq->u.data.pointer, wrq->u.data.length);
		if (Status) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"copy_from_user return status received = failure\n");
			Status = -NDIS_STATUS_FAILURE;
		}
		req = (struct wapp_req *)buf;
		wapp_event_handle(pAd, req);
		os_free_mem(buf);
	}
		break;

#ifdef CONFIG_CPE_SUPPORT
	case OID_LPPE_EVENT:
	{
		UCHAR *buf = NULL;
		struct wapp_req *req;

		os_alloc_mem(NULL, (UCHAR **)&buf, wrq->u.data.length);
		Status = copy_from_user(buf, wrq->u.data.pointer, wrq->u.data.length);
		if (Status) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"copy_from_user return status received = failure\n");
			Status = -NDIS_STATUS_FAILURE;
		}
		req = (struct wapp_req *)buf;

		if (req->req_id == LPPE_GET_SCAN_RESULTS) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Get scan result for LPPE\n");
			RTMPLppeGetScanResults(pAd, req);
		}

		os_free_mem(buf);
	}
		break;
#endif
#endif /* WAPP_SUPPORT */
#ifdef OFFCHANNEL_SCAN_FEATURE
	case OID_OFFCHANNEL_INFO:
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
		"Msg received !! \n");
		Status = Channel_Info_MsgHandle(pAd, wrq, pObj);
		if (!Status) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Channel_Info_MsgHandle return status received = failure changing it to negative for application \n");
			Status = -NDIS_STATUS_FAILURE;
		}
		break;
#endif
#ifdef CONFIG_6G_AFC_SUPPORT
	case OID_SET_AFC_CONFIG:
	{
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"AFC response received !!\n");

		Status = afc_daemon_response(pAd, wrq);
		break;
	}
#endif/* CONFIG_6G_AFC_SUPPORT */
#ifdef VOW_SUPPORT
#define VOW_CMD_STR_LEN 16

	case OID_802_11_VOW_BW_EN:
		/* not used now */
		break;

	case OID_802_11_VOW_BW_AT_EN: {
		P_VOW_UI_CONFIG cfg;
		UCHAR buf[VOW_CMD_STR_LEN];
		UINT8 group;

		os_alloc_mem(pAd, (UCHAR **)&cfg, wrq->u.data.length);

		if (cfg == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(cfg, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			cfg->ssid_num = (cfg->ssid_num > VOW_MAX_GROUP_NUM) ? VOW_MAX_GROUP_NUM : cfg->ssid_num;
			for (group = 0; group < cfg->ssid_num; group++) {
				snprintf(buf, sizeof(buf), "%d-%d", group, cfg->val[group]);
				set_vow_airtime_ctrl_en(pAd, buf);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_VOW_BW_AT_EN(0x%08x) -> val %d\n", cmd,
						 cfg->val[group]);
			}
		} else
			Status = -EFAULT;

		os_free_mem(cfg);
	}
	break;

	case OID_802_11_VOW_BW_TPUT_EN: {
		P_VOW_UI_CONFIG cfg;
		UCHAR buf[VOW_CMD_STR_LEN];
		UINT8 group;

		os_alloc_mem(pAd, (UCHAR **)&cfg, wrq->u.data.length);

		if (cfg == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(cfg, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			cfg->ssid_num = (cfg->ssid_num > VOW_MAX_GROUP_NUM) ? VOW_MAX_GROUP_NUM : cfg->ssid_num;
			for (group = 0; group < cfg->ssid_num; group++) {
				snprintf(buf, sizeof(buf), "%d-%d", group, cfg->val[group]);
				set_vow_bw_ctrl_en(pAd, buf);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_VOW_BW_TPUT_EN(0x%08x) -> val %d\n", cmd,
						 cfg->val[group]);
			}
		} else
			Status = -EFAULT;

		os_free_mem(cfg);
	}
	break;

	case OID_802_11_VOW_ATF_EN: {
		UCHAR *val, buf[VOW_CMD_STR_LEN];

		os_alloc_mem(pAd, (UCHAR **)&val, wrq->u.data.length);

		if (val == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(val, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			snprintf(buf, sizeof(buf), "%d", val[0]);
			set_vow_airtime_fairness_en(pAd, buf);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_VOW_ATF_EN(0x%08x) -> val %d\n", cmd, val[0]);
		} else
			Status = -EFAULT;

		os_free_mem(val);
	}
	break;

	case OID_802_11_VOW_RX_EN: {
		UCHAR *val, buf[VOW_CMD_STR_LEN];
		int ret;

		os_alloc_mem(pAd, (UCHAR **)&val, wrq->u.data.length);

		if (val == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(val, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			ret = snprintf(buf, sizeof(buf), "%d", val[0]);
			if (os_snprintf_error(sizeof(buf), ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"final_name snprintf error!\n");
			}
			set_vow_rx_airtime_en(pAd, buf);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_VOW_RX_EN(0x%08x) -> val %d\n", cmd, val[0]);
		} else
			Status = -EFAULT;

		os_free_mem(val);
	}
	break;

	case OID_802_11_VOW_GROUP_MAX_RATE: {
		P_VOW_UI_CONFIG cfg;
		UCHAR buf[VOW_CMD_STR_LEN];
		UINT8 group;

		os_alloc_mem(pAd, (UCHAR **)&cfg, wrq->u.data.length);

		if (cfg == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(cfg, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			for (group = 0; group < cfg->ssid_num; group++) {
				snprintf(buf, sizeof(buf), "%d-%d", group, cfg->val[group]);
				set_vow_max_rate(pAd, buf);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_VOW_GROUP_MAX_RATE(0x%08x) -> val %d\n", cmd,
						 cfg->val[group]);
			}
		} else
			Status = -EFAULT;

		os_free_mem(cfg);
	}
	break;

	case OID_802_11_VOW_GROUP_MIN_RATE: {
		P_VOW_UI_CONFIG cfg;
		UCHAR buf[VOW_CMD_STR_LEN];
		UINT8 group;

		os_alloc_mem(pAd, (UCHAR **)&cfg, wrq->u.data.length);

		if (cfg == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(cfg, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			for (group = 0; group < cfg->ssid_num; group++) {
				snprintf(buf, sizeof(buf), "%d-%d", group, cfg->val[group]);
				set_vow_min_rate(pAd, buf);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_VOW_GROUP_MIN_RATE(0x%08x) -> val %d\n", cmd,
						 cfg->val[group]);
			}
		} else
			Status = -EFAULT;

		os_free_mem(cfg);
	}
	break;

	case OID_802_11_VOW_GROUP_MAX_RATIO: {
		P_VOW_UI_CONFIG cfg;
		UCHAR buf[VOW_CMD_STR_LEN];
		UINT8 group;

		os_alloc_mem(pAd, (UCHAR **)&cfg, wrq->u.data.length);

		if (cfg == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(cfg, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			for (group = 0; group < cfg->ssid_num; group++) {
				snprintf(buf, sizeof(buf), "%d-%d", group, cfg->val[group]);
				set_vow_max_ratio(pAd, buf);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_VOW_GROUP_MAX_RATIO(0x%08x) -> val %d\n", cmd,
						 cfg->val[group]);
			}
		} else
			Status = -EFAULT;

		os_free_mem(cfg);
	}
	break;

	case OID_802_11_VOW_GROUP_MIN_RATIO: {
		P_VOW_UI_CONFIG cfg;
		UCHAR buf[VOW_CMD_STR_LEN];
		UINT8 group;

		os_alloc_mem(pAd, (UCHAR **)&cfg, wrq->u.data.length);

		if (cfg == NULL) {
			Status = -ENOMEM;
			break;
		}

		Status = copy_from_user(cfg, wrq->u.data.pointer, wrq->u.data.length);

		if (Status == NDIS_STATUS_SUCCESS) {
			for (group = 0; group < cfg->ssid_num; group++) {
				snprintf(buf, sizeof(buf), "%d-%d", group, cfg->val[group]);
				set_vow_min_ratio(pAd, buf);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_VOW_GROUP_MIN_RATIO(0x%08x) -> val %d\n", cmd,
						 cfg->val[group]);
			}
		} else
			Status = -EFAULT;

		os_free_mem(cfg);
	}
	break;
#endif /* VOW_SUPPORT */
#ifdef WIFI_SPECTRUM_SUPPORT

	case OID_802_11_WIFISPECTRUM_SET_PARAMETER: {
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
		P_RBIST_CAP_START_T pSpectrumInfo;
		UINT_32 ret;

		os_alloc_mem(pAd, (UCHAR **)&pSpectrumInfo, wrq->u.data.length);
		ret = copy_from_user(pSpectrumInfo, wrq->u.data.pointer, wrq->u.data.length);
		pSpectrumInfo->u4BW = Get_System_Bw_Info(pAd, pSpectrumInfo->u4CaptureNode);

		if (ops->SpectrumStatus != NULL)
			ops->SpectrumStatus(pAd);
		else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "The function is not hooked !!\n");
		}

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n Trigger = 0x%08x\n"
				 " RingCapEn  = 0x%08x\n TriggerEvent  = 0x%08x\n CaptureNode = 0x%08x\n CaptureLen = 0x%08x\n"
				 " CapStopCycle = 0x%08x\n BW = 0x%08x\n PdEnable = 0x%08x\n FixRxGain = 0x%08x\n"
				 " WifiPath = 0x%08x\n BandIdx = 0x%08x\n", pSpectrumInfo->fgTrigger, pSpectrumInfo->fgRingCapEn,
				 pSpectrumInfo->u4TriggerEvent, pSpectrumInfo->u4CaptureNode, pSpectrumInfo->u4CaptureLen, pSpectrumInfo->u4CapStopCycle,
				 pSpectrumInfo->u4BW, pSpectrumInfo->u4PdEnable, pSpectrumInfo->u4FixRxGain,
				 pSpectrumInfo->u4WifiPath, pSpectrumInfo->u4BandIdx);
		os_free_mem(pSpectrumInfo);
	}
	break;

	case OID_802_11_WIFISPECTRUM_GET_CAPTURE_STATUS: {
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->SpectrumStatus != NULL)
			Status = ops->SpectrumStatus(pAd);
		else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "The function is not hooked !!\n");
		}

		if (Status != NDIS_STATUS_SUCCESS)
			Status = -NDIS_STATUS_FAILURE;

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "OID_802_11_WIFISPECTRUM_GET_CAPTURE_STATUS Status : %d\n", Status);
	}
	break;

	case OID_802_11_WIFISPECTRUM_DUMP_DATA: {
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		if (ops->SpectrumCmdRawDataProc != NULL)
			Status = ops->SpectrumCmdRawDataProc(pAd);
		else {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "The function is not hooked !!\n");
		}

		if (Status != NDIS_STATUS_SUCCESS)
			Status = -NDIS_STATUS_FAILURE;

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "OID_802_11_WIFISPECTRUM_DUMP_DATA Status : %d\n", Status);
	}
	break;
#endif /* WIFI_SPECTRUM_SUPPORT */
#ifdef BAND_STEERING

	case OID_BNDSTRG_MSG:
		BndStrg_MsgHandle(pAd, wrq, pObj->ioctl_if);
		break;
#endif /* BAND_STEERING */

#ifdef MT_DFS_SUPPORT
	case OID_DFS_ZERO_WAIT:
		Status = ZeroWaitDfsCmdHandler(pAd, wrq);
		break;
#ifdef DFS_ZEROWAIT_SUPPORT
	case OID_DFS_CHANNEL_SWITCH:
		printk("OID_DFS_CHANNEL_SWITCH\n");
		Status = ZeroWaitDfsChannelSwitch(pAd, wrq);
		break;
#endif
#endif

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	case OID_AP_VENDOR_IE_SET: {
		INT ret;
		CHAR *vendor_ie_temp;
		struct wifi_dev *wdev;
		struct customer_vendor_ie *ap_vendor_ie;

		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			return FALSE;
		}

		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

		if (wdev->if_up_down_state == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,"interface is down, return!\n");
			break;
		}

		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is null, return!\n");
			break;
		}

		if (wrq->u.data.length == 0) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "data len is 0, return!\n");
			break;
		}

		ret = os_alloc_mem(pAd,
				(UCHAR **)&vendor_ie_temp,
				 wrq->u.data.length);
		if (ret == NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Set::OID_AP_VENDOR_IE_SET\n");
			Status = copy_from_user(vendor_ie_temp,
								wrq->u.data.pointer,
								wrq->u.data.length);
			ap_vendor_ie = &pAd->ApCfg.MBSSID[ifIndex].ap_vendor_ie;
			RTMP_SPIN_LOCK(&ap_vendor_ie->vendor_ie_lock);
			if (ap_vendor_ie->pointer != NULL)
				os_free_mem(ap_vendor_ie->pointer);
			ap_vendor_ie->pointer = vendor_ie_temp;
			ap_vendor_ie->length = wrq->u.data.length;
			RTMP_SPIN_UNLOCK(&ap_vendor_ie->vendor_ie_lock);

			/* start sending BEACON out */
			UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "alloc memory fail\n");
	}
	break;

	case OID_AP_VENDOR_IE_DEL:
	{
		struct wifi_dev *wdev;
		struct customer_vendor_ie *ap_vendor_ie;

		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			return FALSE;
		}

		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is null, return!\n");
			break;
		}

		if (wdev->if_up_down_state == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "interface is down, return!\n");
			break;
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Del::OID_AP_VENDOR_IE_DEL\n");

		ap_vendor_ie = &pAd->ApCfg.MBSSID[ifIndex].ap_vendor_ie;

		RTMP_SPIN_LOCK(&ap_vendor_ie->vendor_ie_lock);
		if (ap_vendor_ie->pointer != NULL) {
			os_free_mem(ap_vendor_ie->pointer);
			ap_vendor_ie->pointer = NULL;
		}
		ap_vendor_ie->length = 0;
		RTMP_SPIN_UNLOCK(&ap_vendor_ie->vendor_ie_lock);

		/* start sending BEACON out */
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	}
		break;

	case OID_SET_OUI_FILTER:
	{
		INT ret;
		CHAR *customer_oui_temp;
		struct customer_oui_filter *ap_customer_oui;

		if (wrq->u.data.length == 0) {
			ap_customer_oui = &pAd->ApCfg.ap_customer_oui;
			RTMP_SPIN_LOCK(&ap_customer_oui->oui_filter_lock);
			if (ap_customer_oui->pointer != NULL) {
				os_free_mem(ap_customer_oui->pointer);
				ap_customer_oui->pointer = NULL;
			}
			ap_customer_oui->length = 0;
			RTMP_SPIN_UNLOCK(&ap_customer_oui->oui_filter_lock);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"OUI data length is 0, Delete Filter and return!!\n");
			break;
		}

		ret = os_alloc_mem(pAd,
				 (UCHAR **)&customer_oui_temp,
				 wrq->u.data.length);
		if (ret == NDIS_STATUS_SUCCESS) {
			Status = copy_from_user(customer_oui_temp,
						wrq->u.data.pointer,
						wrq->u.data.length);

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Set::OID_SET_OUI_FILTER\n");
			ap_customer_oui = &pAd->ApCfg.ap_customer_oui;
			RTMP_SPIN_LOCK(&ap_customer_oui->oui_filter_lock);
			if (ap_customer_oui->pointer != NULL)
				os_free_mem(ap_customer_oui->pointer);
			ap_customer_oui->pointer = customer_oui_temp;
			ap_customer_oui->length = wrq->u.data.length;

			RTMP_SPIN_UNLOCK(&ap_customer_oui->oui_filter_lock);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "alloc memory fail\n");
	}
		break;

#ifdef APCLI_SUPPORT
	case OID_APCLI_VENDOR_IE_SET:
	{
		INT ret;
		CHAR *vendor_ie_temp;
		struct wifi_dev *wdev;
		struct customer_vendor_ie *apcli_vendor_ie;

		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		ifIndex = pObj->ioctl_if;
		if (ifIndex >= MAX_MULTI_STA) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error station index\n");
			return FALSE;
		}

		wdev = &pAd->StaCfg[ifIndex].wdev;
		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is null, return!\n");
			break;
		}

		if (wdev->if_up_down_state == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "interface is down, return!\n");
			break;
		}

		if (wrq->u.data.length == 0) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "data len is 0, return!\n");
			break;
		}

		ret = os_alloc_mem(pAd,
				 (UCHAR **)&vendor_ie_temp,
				 wrq->u.data.length);
		if (ret == NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::OID_APCLI_VENDOR_IE_SET\n");
			Status = copy_from_user(vendor_ie_temp,
						wrq->u.data.pointer,
						wrq->u.data.length);
			apcli_vendor_ie = &pAd->StaCfg[ifIndex].apcli_vendor_ie;
			RTMP_SPIN_LOCK(&apcli_vendor_ie->vendor_ie_lock);
			if (apcli_vendor_ie->pointer != NULL)
				os_free_mem(apcli_vendor_ie->pointer);
			apcli_vendor_ie->pointer = vendor_ie_temp;
			apcli_vendor_ie->length = wrq->u.data.length;
			RTMP_SPIN_UNLOCK(&apcli_vendor_ie->vendor_ie_lock);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "alloc memory fail\n");
	}
		break;

	case OID_APCLI_VENDOR_IE_DEL:
	{
		struct wifi_dev *wdev;
		struct customer_vendor_ie *apcli_vendor_ie;

		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		ifIndex = pObj->ioctl_if;
		if (ifIndex >= MAX_MULTI_STA) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,"error station index\n");
			return FALSE;
		}

		wdev = &pAd->StaCfg[ifIndex].wdev;
		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is null, return!\n");
			break;
		}

		if (wdev->if_up_down_state == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "interface is down, return!\n");
			break;
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Del::OID_APCLI_VENDOR_IE_DEL\n");

		apcli_vendor_ie = &pAd->StaCfg[ifIndex].apcli_vendor_ie;
		RTMP_SPIN_LOCK(&apcli_vendor_ie->vendor_ie_lock);
		if (apcli_vendor_ie->pointer != NULL) {
			os_free_mem(apcli_vendor_ie->pointer);
			apcli_vendor_ie->pointer = NULL;
		}
		apcli_vendor_ie->length = 0;
		RTMP_SPIN_UNLOCK(&apcli_vendor_ie->vendor_ie_lock);
	}
		break;
#endif /* APCLI_SUPPORT */
	case OID_AP_PROBE_RSP_VENDOR_IE_SET:
	{
		INT apidx;
		INT ret;
		CHAR *vendor_ie_temp;
		UCHAR *vendor_ie_temp_header;
		struct wifi_dev *wdev;
		CUSTOMER_PROBE_RSP_VENDOR_IE *ap_probe_rsp_vendor_ie = NULL;
		BSS_STRUCT *mbss = NULL;
		PDL_LIST ap_probe_rsp_vendor_ie_list = NULL;
		BOOLEAN found = FALSE;
		UCHAR band;

		apidx = pObj->ioctl_if;
		mbss = &pAd->ApCfg.MBSSID[apidx];
		wdev = &mbss->wdev;

		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is null, return!\n");
			break;
		}

		if (wdev->if_up_down_state == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "interface is down, return!\n");
			break;
		}

		if (wrq->u.data.length > 264 || wrq->u.data.length < 12 ) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"data len[%d] is invalid, return!\n", wrq->u.data.length);
			break;
		}
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Set::OID_AP_PROBE_RSP_VENDOR_IE_SET\n");
		/* alloc mem for mac_addr and band*/
		ret = os_alloc_mem(pAd, (UCHAR **)&vendor_ie_temp_header, 7);
		if (ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Fail to alloc memory for vendor_ie_temp_header, break!!!\n");
			break;
		}
		/* alloc mem for vie*/
		ret = os_alloc_mem(pAd, (UCHAR **)&vendor_ie_temp, wrq->u.data.length - 7);
		if (ret != NDIS_STATUS_SUCCESS) {
			os_free_mem(vendor_ie_temp_header);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Fail to alloc memory for vendor_ie_temp, break!!!\n");
			break;
		}

		Status = copy_from_user(vendor_ie_temp_header,
					wrq->u.data.pointer, 7);

		Status = copy_from_user(vendor_ie_temp,
					wrq->u.data.pointer + 7, wrq->u.data.length - 7);

		RTMP_SPIN_LOCK(&mbss->probe_rsp_vendor_ie_lock);
		ap_probe_rsp_vendor_ie_list = &mbss->ap_probe_rsp_vendor_ie_list;
		DlListForEach(ap_probe_rsp_vendor_ie, ap_probe_rsp_vendor_ie_list,
			CUSTOMER_PROBE_RSP_VENDOR_IE, List) {
			if (memcmp(ap_probe_rsp_vendor_ie->stamac, vendor_ie_temp_header, MAC_ADDR_LEN) == 0) {
				found = TRUE;
				break;
			}
		}

		band = vendor_ie_temp_header[6];
		if (found) {
			ap_probe_rsp_vendor_ie->band = band;
			if (ap_probe_rsp_vendor_ie->pointer != NULL)
				os_free_mem(ap_probe_rsp_vendor_ie->pointer);
			ap_probe_rsp_vendor_ie->pointer = vendor_ie_temp;
			ap_probe_rsp_vendor_ie->length = wrq->u.data.length - 7;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"found STA MAC["MACSTR"], replace probe response VIE!\n",
					MAC2STR(vendor_ie_temp_header));
		} else {
			if(pAd->ApCfg.ap_probe_rsp_vendor_ie_count < pAd->ApCfg.ap_probe_rsp_vendor_ie_max_count) {
				ret = os_alloc_mem(pAd, (UCHAR **)&ap_probe_rsp_vendor_ie,
					sizeof(CUSTOMER_PROBE_RSP_VENDOR_IE));

				if (ret != NDIS_STATUS_SUCCESS) {
					os_free_mem(vendor_ie_temp);
					os_free_mem(vendor_ie_temp_header);
					RTMP_SPIN_UNLOCK(&mbss->probe_rsp_vendor_ie_lock);
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Fail to alloc memory for ap_probe_rsp_vendor_ie, break!\n");
					break;
				}
				NdisZeroMemory(ap_probe_rsp_vendor_ie, sizeof(CUSTOMER_PROBE_RSP_VENDOR_IE));
				ap_probe_rsp_vendor_ie->band = band;
				COPY_MAC_ADDR(ap_probe_rsp_vendor_ie->stamac, vendor_ie_temp_header);
				ap_probe_rsp_vendor_ie->pointer = vendor_ie_temp;
				ap_probe_rsp_vendor_ie->length = wrq->u.data.length - 7;

				DlListAddTail(&mbss->ap_probe_rsp_vendor_ie_list, &ap_probe_rsp_vendor_ie->List);
				pAd->ApCfg.ap_probe_rsp_vendor_ie_count++;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Add new STA MAC["MACSTR"]!!!\n", MAC2STR(ap_probe_rsp_vendor_ie->stamac));
				hex_dump_with_lvl("Probe rsp IE: ",ap_probe_rsp_vendor_ie->pointer ,
					ap_probe_rsp_vendor_ie->length, DBG_LVL_DEBUG);
			} else {
				os_free_mem(vendor_ie_temp);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Fail to add ap_probe_rsp_vendor_ie, exceed List max count[%d]!\n",
					pAd->ApCfg.ap_probe_rsp_vendor_ie_max_count);
			}
		}
		RTMP_SPIN_UNLOCK(&mbss->probe_rsp_vendor_ie_lock);

		os_free_mem(vendor_ie_temp_header);
	}
		break;

	case OID_AP_PROBE_RSP_VENDOR_IE_DEL:
	{
		CUSTOMER_PROBE_RSP_VENDOR_IE *ap_probe_rsp_vendor_ie = NULL;
		BSS_STRUCT *mbss = NULL;
		struct wifi_dev *wdev;
		PDL_LIST ap_probe_rsp_vendor_ie_list = NULL;
		UINT32 ie_count;
		BOOLEAN found = FALSE;
		INT apidx, ret;
		UCHAR *vendor_ie_temp_header;

		apidx = pObj->ioctl_if;
		mbss = &pAd->ApCfg.MBSSID[apidx];
		wdev = &mbss->wdev;

		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is null, return!\n");
			break;
		}

		if (wdev->if_up_down_state == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "interface is down, return!\n");
			break;
		}

		if (wrq->u.data.length < 6) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"data len[%d] is invalid, return!\n", wrq->u.data.length);
			break;
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"DeL::OID_AP_PROBE_RSP_VENDOR_IE_DEL\n");

		ret = os_alloc_mem(pAd, (UCHAR **)&vendor_ie_temp_header, 7);
		if (ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Fail to alloc memory for vendor_ie_temp_header, break!\n");
			break;
		}

		Status = copy_from_user(vendor_ie_temp_header,
					wrq->u.data.pointer, 7);

		RTMP_SPIN_LOCK(&mbss->probe_rsp_vendor_ie_lock);
		ap_probe_rsp_vendor_ie_list = &mbss->ap_probe_rsp_vendor_ie_list;
		ie_count = DlListLen(ap_probe_rsp_vendor_ie_list);
		if (ie_count) {
			DlListForEach(ap_probe_rsp_vendor_ie, ap_probe_rsp_vendor_ie_list,
				CUSTOMER_PROBE_RSP_VENDOR_IE, List) {
				if (memcmp(ap_probe_rsp_vendor_ie->stamac, vendor_ie_temp_header, MAC_ADDR_LEN) == 0) {
					DlListDel(&ap_probe_rsp_vendor_ie->List);
					pAd->ApCfg.ap_probe_rsp_vendor_ie_count--;
					if (ap_probe_rsp_vendor_ie->pointer)
						os_free_mem(ap_probe_rsp_vendor_ie->pointer);
					os_free_mem(ap_probe_rsp_vendor_ie);
					found = TRUE;
					break;
				}
			}
		}
		if (found)
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Free STA MAC["MACSTR"]!\n", MAC2STR(ap_probe_rsp_vendor_ie->stamac));
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Free STA MAC["MACSTR"] not found!\n", MAC2STR(vendor_ie_temp_header));

		RTMP_SPIN_UNLOCK(&mbss->probe_rsp_vendor_ie_lock);
		os_free_mem(vendor_ie_temp_header);
	}
		break;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
	case OID_SET_CH_SWITCH_DURATION:
		{
			INT ret;
			struct ch_switch_user_cfg *ch_sw_user_cfg;
			struct ch_switch_cfg *ch_sw_cfg;

			if (wrq->u.data.length == 0 || wrq->u.data.length != sizeof(struct ch_switch_user_cfg)) {
				Status = -EFAULT;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "data len is invalid, return!\n");
				break;
			}
			ret = os_alloc_mem(pAd, (UCHAR **)&ch_sw_cfg, wrq->u.data.length);
			if (ret == NDIS_STATUS_SUCCESS) {
				ret = os_alloc_mem(pAd, (UCHAR **)&ch_sw_user_cfg, sizeof(struct ch_switch_user_cfg));
				if (ret == NDIS_STATUS_SUCCESS) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"Set::OID_SET_CH_SWITCH_DURATION\n");
					if (copy_from_user(ch_sw_user_cfg, wrq->u.data.pointer, wrq->u.data.length)) {
						Status = -EFAULT;
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy from user fail\n");
					} else {
						ch_sw_cfg->channel = ch_sw_user_cfg->channel;
						ch_sw_cfg->duration = ch_sw_user_cfg->duration;
						Status = set_ch_switch_monitor_cfg(pAd, ch_sw_cfg);
					}
					os_free_mem(ch_sw_user_cfg);
				} else {
					Status = -EFAULT;
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "alloc memory fail\n");
				}
				os_free_mem(ch_sw_cfg);
			} else {
				Status = -EFAULT;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "alloc memory fail\n");
			}
		}
		break;
#endif
	case OID_802_11_VENDOR_IE_ADD:
	case OID_802_11_VENDOR_IE_UPDATE:
	case OID_802_11_VENDOR_IE_REMOVE:
	{
		UCHAR *Buf;
		struct vie_op_data_s *vie_op_data;
		struct wifi_dev *wdev;
		UINT32 length = 0;
		UINT32 oui_oitype = 0;

		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			return FALSE;
		}

		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

		os_alloc_mem(pAd, (UCHAR **)&Buf, wrq->u.data.length);
		Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
		vie_op_data = (struct vie_op_data_s *)Buf;
		length = vie_op_data->vie_length;
		NdisMoveMemory((UCHAR *)&oui_oitype, vie_op_data->oui_oitype, sizeof(UINT32));

		if ((cmd & 0x7FFF) == VIE_REMOVE) {
			if (remove_vie(pAd,
				       wdev,
				       vie_op_data->frm_type_map,
				       oui_oitype,
				       length,
				       vie_op_data->app_ie_ctnt) == NDIS_STATUS_FAILURE) {
				Status = NDIS_STATUS_FAILURE;
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"OID_802_11_VENDOR_IE_REMOVE failed.\n");
			}
		} else {
			if (add_vie(pAd,
				    wdev,
				    vie_op_data->frm_type_map,
				    oui_oitype,
				    length,
				    vie_op_data->app_ie_ctnt) == NDIS_STATUS_FAILURE) {
				Status = NDIS_STATUS_FAILURE;
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"OID_802_11_VENDOR_IE_ADD failed.\n");
			}
		}

		os_free_mem(Buf);
	}
		break;
	case OID_802_11_VENDOR_IE_SHOW:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"OID_802_11_VENDOR_IE_SHOW not finish yet\n");
		break;
#ifdef MAP_R2
	case OID_802_11_CAC_STOP:
	{
		int i = 0;
		UCHAR BandIdx = 0;
		struct wifi_dev *wdev;
		BOOLEAN bSupport5G = HcIsRfSupport(pAd, RFIC_5GHZ);

		if (pObj->ioctl_if_type == INT_MBSSID) {
			ifIndex = pObj->ioctl_if;
			if (!VALID_MBSS(pAd, ifIndex)) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"error AP index\n");
				return FALSE;
			}

			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		} else
			wdev = &pAd->ApCfg.MBSSID[0].wdev;

		if (wdev->pHObj == NULL)
			break;

		BandIdx = HcGetBandByWdev(wdev);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_CAC_STOP!!\n");

#ifdef A_BAND_SUPPORT
		if (bSupport5G && (pAd->CommonCfg.bIEEE80211H == 1)) {
			BOOLEAN BandInCac[DBDC_BAND_NUM];
			struct DOT11_H *pDot11hTest = NULL;

			for (i = 0; i < DBDC_BAND_NUM; i++)
				BandInCac[i] = FALSE;

			pDot11hTest = &pAd->Dot11_H[BandIdx];

			if (pDot11hTest == NULL)
				break;
#ifdef MT_DFS_SUPPORT
			if (pDot11hTest->RDMode == RD_SILENCE_MODE) {
#ifdef MAP_R2
				BandInCac[BandIdx] = TRUE;
#endif
				pDot11hTest->RDCount = 0;
				MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CAC_END, 0, NULL, HcGetBandByWdev(wdev));
				AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod[DBDC_BAND0],
					HW_BSSID_0,  OPMODE_AP);
				pDot11hTest->RDMode = RD_NORMAL_MODE;
			}
		}
#endif
#endif /* A_BAND_SUPPORT */
	}
	break;
#endif
#ifdef DABS_QOS
		case OID_AP_DABS_RULE_SET:
		{
			INT apidx, ret;
			BSS_STRUCT *mbss = NULL;
			struct wifi_dev *wdev;
			struct qos_param_rec_add *qos_param_rec_add = NULL;
			UINT32 idx;

			apidx = pObj->ioctl_if;
			mbss = &pAd->ApCfg.MBSSID[apidx];
			wdev = &mbss->wdev;

			if (wdev == NULL) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is null, return!\n");
				break;
			}

			if (wdev->if_up_down_state == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "interface is down, return!\n");
				break;
			}

			if (wrq->u.data.length > sizeof(struct qos_param_rec_add)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"data len[%d] is invalid, return!\n", wrq->u.data.length);
				break;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_AP_DABS_RULE_SET\n");

			ret = os_alloc_mem(pAd, (UCHAR **)&qos_param_rec_add, sizeof(struct qos_param_rec_add));

			if (ret != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Fail to alloc memory for qos_param_rec_add, break!\n");
				break;
			}

			Status = copy_from_user(qos_param_rec_add,
						wrq->u.data.pointer, sizeof(struct qos_param_rec_add));

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Set ip_src %x, ip_dest %x, dest_mac["MACSTR"],\n\
				protocol = %u, sport = %u, dport = %u, priority = %u, delay_bound = %u, delay_weight = %u\n\
				delay_req = %u, data_rate = %u, bw_req = %u, dir = %u, drop_thres = %u, app_type = %u\n",
				qos_param_rec_add->ip_src, qos_param_rec_add->ip_dest,
				MAC2STR(qos_param_rec_add->dest_mac),
				qos_param_rec_add->protocol, qos_param_rec_add->sport, qos_param_rec_add->dport,
				qos_param_rec_add->priority, qos_param_rec_add->delay_bound, qos_param_rec_add->delay_weight,
				qos_param_rec_add->delay_req, qos_param_rec_add->data_rate, qos_param_rec_add->bw_req,
				qos_param_rec_add->dir, qos_param_rec_add->drop_thres, qos_param_rec_add->app_type);

			idx = ioctl_search_qos_param_tbl_idx_by_5_tuple(pAd, (VOID *)qos_param_rec_add, TRUE);

			if (idx < MAX_QOS_PARAM_TBL) {
				if (update_qos_param(pAd, idx, qos_param_rec_add) != TRUE) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"update qos para Fail!\n");
				}
			} else {
				idx = search_free_qos_param_tbl_idx(pAd);

				if (idx < MAX_QOS_PARAM_TBL) {
					if (update_qos_param(pAd, idx, qos_param_rec_add) != TRUE) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"update qos para Fail!\n");
					}
				} else
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"no free table for setting , Fail!\n");
			}

			os_free_mem(qos_param_rec_add);
		}
		break;
		case OID_AP_DABS_RULE_DEL:
		{
			INT apidx, ret;
			BSS_STRUCT *mbss = NULL;
			struct wifi_dev *wdev;
			struct qos_param_rec_del *qos_param_rec_del = NULL;
			UINT32 idx;

			apidx = pObj->ioctl_if;
			mbss = &pAd->ApCfg.MBSSID[apidx];
			wdev = &mbss->wdev;

			if (wdev == NULL) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is null, return!\n");
				break;
			}

			if (wdev->if_up_down_state == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "interface is down, return!\n");
				break;
			}

			if (wrq->u.data.length > sizeof(struct qos_param_rec_del)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"data len[%d] is invalid, return!!!\n", wrq->u.data.length);
				break;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"OID_AP_DABS_RULE_DEL\n");

			ret = os_alloc_mem(pAd, (UCHAR **)&qos_param_rec_del, sizeof(struct qos_param_rec_del));

			if (ret != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Fail to alloc memory for qos_param_rec_del, break!\n");
				break;
			}

			Status = copy_from_user(qos_param_rec_del,
						wrq->u.data.pointer, sizeof(struct qos_param_rec_del));

			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Set ip_src %x, ip_dest %x, protocol = %u, sport = %u, dport = %u\n",
				qos_param_rec_del->ip_src, qos_param_rec_del->ip_dest,
				qos_param_rec_del->protocol, qos_param_rec_del->sport, qos_param_rec_del->dport);

			idx = ioctl_search_qos_param_tbl_idx_by_5_tuple(pAd, (VOID *)qos_param_rec_del, FALSE);
			if (idx < MAX_QOS_PARAM_TBL) {
				if (delete_qos_param(pAd, idx) != TRUE)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"delete qos para Fail!\n");
			} else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"cannot find table to delete!\n");

			os_free_mem(qos_param_rec_del);
		}
		break;
#endif
	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::unknown IOCTL's subcmd = 0x%08x\n", cmd);
		Status = -EOPNOTSUPP;
		break;
	}

	return Status;
}

#ifdef CONFIG_MAP_SUPPORT
int Fill_OID_WSC_PROFILE(IN PRTMP_ADAPTER pAd, UCHAR ifIndex, p_wsc_apcli_config_msg *wsc_config)
{
	PWSC_CTRL pWscControl = NULL;
	PWSC_CREDENTIAL pCredential = NULL;
	WSC_PROFILE *pWscProfile = NULL;
	struct wifi_dev *wdev;
	int  i = 0, TotalLen = 0;
	wsc_apcli_config_msg *temp_wsc_config = NULL;

	pWscControl = &pAd->StaCfg[ifIndex].wdev.WscControl;
	pWscProfile = &pWscControl->WscProfile;
	wdev = &pAd->StaCfg[ifIndex].wdev;
	TotalLen =	sizeof(wsc_apcli_config_msg) +
		sizeof(wsc_apcli_config) * pWscProfile->ProfileCnt;
	if (TotalLen > 512) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Total Length is >512 return\n");
		return FALSE;
	}
	os_alloc_mem(NULL, (PUCHAR *)&temp_wsc_config, TotalLen);
	if (temp_wsc_config == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "failed to allocated memory\n");
		return FALSE;
	}
	NdisZeroMemory(temp_wsc_config, TotalLen);
	*wsc_config = (wsc_apcli_config_msg *)temp_wsc_config;
	temp_wsc_config->profile_count = pWscProfile->ProfileCnt;
	for (i = 0; i < pWscProfile->ProfileCnt; i++) {
		pCredential = &pWscProfile->Profile[i];
		NdisZeroMemory(temp_wsc_config->apcli_config[i].ssid,
			sizeof(temp_wsc_config->apcli_config[i].ssid));
		NdisCopyMemory(temp_wsc_config->apcli_config[i].ssid, pCredential->SSID.Ssid,
										pCredential->SSID.SsidLength);
		NdisCopyMemory(temp_wsc_config->apcli_config[i].Key,
			pCredential->Key, pCredential->KeyLength);
		NdisCopyMemory(temp_wsc_config->apcli_config[i].bssid,
			pCredential->MacAddr, MAC_ADDR_LEN);
		temp_wsc_config->apcli_config[i].SsidLen = pCredential->SSID.SsidLength;
		temp_wsc_config->apcli_config[i].AuthType = pCredential->AuthType;
		temp_wsc_config->apcli_config[i].EncrType = pCredential->EncrType;
		temp_wsc_config->apcli_config[i].KeyLength = pCredential->KeyLength;
		temp_wsc_config->apcli_config[i].KeyIndex = pCredential->KeyIndex;
		temp_wsc_config->apcli_config[i].peer_map_role = pCredential->DevPeerRole;
		temp_wsc_config->apcli_config[i].own_map_role = wdev->MAPCfg.DevOwnRole;
	}
	return TotalLen;
}
#endif
#ifdef OFFCHANNEL_SCAN_FEATURE
INT Channel_Info_MsgHandle(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, POS_COOKIE pObj)
{
	OFFCHANNEL_SCAN_MSG msg;
	SCAN_CTRL *ScanCtrl = NULL;
	struct wifi_dev *wdev = NULL;
	INT i = 0;
	UINT ifIndex = pObj->ioctl_if;
	INT Status;
#ifdef CONFIG_MAP_SUPPORT
	struct DOT11_H *pDot11h = NULL;
	UCHAR index;
	STA_ADMIN_CONFIG *pApCliEntry;
	UCHAR BandIdx = 255;/*initializing to a non-valid value*/
	UCHAR BandIdx_AP = 255;/*initializing to a non-valid value*/
#endif


	os_zero_mem(&msg, sizeof(OFFCHANNEL_SCAN_MSG));

	if (pObj->ioctl_if_type  == INT_MBSSID) {
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			return CHANNEL_MONITOR_STRG_FAILURE;
		}

		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	} else
		wdev = &pAd->ApCfg.MBSSID[0].wdev;
	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Ioctl : %d!\n", ifIndex);
	if (wrq->u.data.length != sizeof(OFFCHANNEL_SCAN_MSG)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error\n");
		return CHANNEL_MONITOR_STRG_FAILURE;
	} else {
		Status = copy_from_user(&msg, wrq->u.data.pointer, wrq->u.data.length);
		memcpy(ScanCtrl->if_name, msg.ifrn_name, IFNAMSIZ);
		switch (msg.Action) {
		case GET_OFFCHANNEL_INFO:
		{

			/*To do OffChannelScan, need TakeChannelOpCharge first*/
			if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_SCAN, TRUE)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
					"TakeChannelOpCharge fail for Off-Channel SCAN!!\n");
				return CHANNEL_MONITOR_STRG_FAILURE;
			}

			if (scan_in_run_state(pAd, wdev) == TRUE) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Scan in running State\n");
				ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_SCAN);
				return CHANNEL_MONITOR_STRG_FAILURE;
			}

			if (ScanCtrl->state != OFFCHANNEL_SCAN_INVALID) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"failed because offchannel scan is still ongoing\n");
				ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_SCAN);
				Status = CHANNEL_MONITOR_STRG_FAILURE;
			} else {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"num of away channel to scan = %d\n",
						 msg.data.offchannel_param.Num_of_Away_Channel);

				if (msg.data.offchannel_param.Num_of_Away_Channel > MAX_AWAY_CHANNEL) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid Argument\n");
					ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_SCAN);
					return CHANNEL_MONITOR_STRG_INVALID_ARG;
				}


				ScanCtrl->Num_Of_Channels = msg.data.offchannel_param.Num_of_Away_Channel;
				ScanCtrl->Off_Ch_Scan_BW = msg.data.offchannel_param.bw;
				/* Fillup the paramters received for all channels */
				for (i = 0; i < msg.data.offchannel_param.Num_of_Away_Channel; i++) {
					ScanCtrl->ScanGivenChannel[i] = msg.data.offchannel_param.channel[i];
					/* TODO: Raghav: update on the first ch scan also */
					ScanCtrl->Offchan_Scan_Type[i] = msg.data.offchannel_param.scan_type[i];
					ScanCtrl->ScanTime[i] = msg.data.offchannel_param.scan_time[i];
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"channel = %d:scan type = %d:scan time = %d\n",
							 ScanCtrl->ScanGivenChannel[i],
							 ScanCtrl->Offchan_Scan_Type[i], ScanCtrl->ScanTime[i]);
				}
#ifdef CONFIG_MAP_SUPPORT
				if (IS_MAP_ENABLE(pAd)) {
					if (wdev) {
						BandIdx_AP = HcGetBandByWdev(wdev);
					}
					for (index = 0; index < MAX_APCLI_NUM; index++) {
						pApCliEntry = &pAd->StaCfg[index];
						if (pApCliEntry->ApcliInfStat.Valid == TRUE) {
								BandIdx = HcGetBandByWdev(&pApCliEntry->wdev);
								break;
							}
					}
					/*if off channel scan req received at BandIdx_AP matches
					with the BandIdx of APCLI which is connected to a
					controller , then only there is need to set the ChChangeCSA
					flag as true in MAP case This flag is used to prevent apcli
					disconnect due to beacon loss during off channel scan*/
					if ((BandIdx == BandIdx_AP) && wdev) {
						pDot11h = wdev->pDot11_H;
						if (pDot11h) {
							pDot11h->ChChangeCSA = TRUE;
						}
					}
				}
#endif
				Status = ApSiteSurveyNew_by_wdev(pAd, msg.data.offchannel_param.channel[0],
							msg.data.offchannel_param.scan_time[0],
							msg.data.offchannel_param.scan_type[0], FALSE, wdev);
			}
		}
			break;
		case TRIGGER_DRIVER_CHANNEL_SWITCH:
			Status = rtmp_set_channel(pAd, wdev, msg.data.channel_data.channel);
			break;
		case UPDATE_DRIVER_SORTED_CHANNEL_LIST:
			MTWF_DBG(pAd, DBG_SUBCAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "num of channels in sorted channel list received from App =%d\n",
				  msg.data.sorted_channel_list.size);
			if (msg.data.sorted_channel_list.size > 0) {
				pAd->sorted_list.size = msg.data.sorted_channel_list.size;
				for (i = 0; i < msg.data.sorted_channel_list.size; i++) {
					pAd->sorted_list.SortedMaxChannelBusyTimeList[i] = msg.data.sorted_channel_list.SortedMaxChannelBusyTimeList[i];
					MTWF_DBG(pAd, DBG_SUBCAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"channel[%d] = %d\n", i, pAd->sorted_list.SortedMaxChannelBusyTimeList[i]);
					pAd->sorted_list.SortedMinChannelBusyTimeList[i] = msg.data.sorted_channel_list.SortedMinChannelBusyTimeList[i];
				}
				Status = CHANNEL_MONITOR_STRG_SUCCESS;
			} else {
				MTWF_DBG(pAd, DBG_SUBCAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"num of channels in sorted channel list received is invalid\n");
				Status = CHANNEL_MONITOR_STRG_FAILURE;
			}
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					"unknown action code. (%d)\n", msg.Action);
			break;
		}
	}
	return Status;
}
#endif
#ifdef ANDLINK_FEATURE_SUPPORT

/**
* andlink_get_pollinfos() - poll interface infos
* @pAd:	wifi adapter.
* @apidx:  ap interface index.
*
*Andlink get interface infos of mac/ssid/channel infos, then
* sent wireless event to andlink app.
*
* Return:  0  Success, other Fail
*/

INT andlink_get_pollinfos(
	RTMP_ADAPTER *pAd,
	INT apidx) {
	struct wifi_event event;
	if (!VALID_MBSS(pAd, pAd->ApCfg.BssidNum)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"input apidx: %d > %d BSSIDNUM\n", apidx, pAd->ApCfg.BssidNum);
		return -EFAULT;
	}

	NdisZeroMemory(&event, sizeof(struct wifi_event));
	event.type = EVENTTYPE_INF_POLL;
	event.data.poll.channel = pAd->ApCfg.MBSSID[apidx].wdev.channel;//pAd->CommonCfg.Channel;
	NdisMoveMemory(event.MAC, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
	NdisMoveMemory(event.SSID, pAd->ApCfg.MBSSID[apidx].Ssid, MAX_LEN_OF_SSID);

	RtmpOSWrielessEventSend(
		pAd->net_dev,
		RT_WLAN_EVENT_CUSTOM,
		OID_ANDLINK_EVENT,
		NULL,
		(char *)&event,
		sizeof(struct wifi_event));

	return NDIS_STATUS_SUCCESS;
}

/**
* andlink_get_stainfos() - get station infos
* @pAd:	wifi adapter.
* @apidx:  ap interface index.
* @wrq: wireless structure
*
*Andlink get interface of this ap index stainfos:
* sent wireless event to andlink app.
*
* Return:
*		0 : success
*		other : Fail
*/
INT andlink_get_stainfos (
	RTMP_ADAPTER *pAd,
	INT apidx,
	RTMP_IOCTL_INPUT_STRUCT	*wrq) {
	INT idx = 0;
	UCHAR *dst_ptr = NULL;
	UCHAR *src_ptr = NULL;
	MAC_TABLE_ENTRY *pEnt = NULL;
	INT	Status = NDIS_STATUS_SUCCESS;
	struct wifi_ioctl_sta_info *sta_info;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pRepEnt = NULL;
#endif/*MAC_REPEATER_SUPPORT*/
	struct wifi_dev *inf_wdev = NULL;
	struct mtk_rate_info tx_rate_info;
	struct mtk_rate_info rx_rate_info;

	if (!VALID_MBSS(pAd, apidx) || NULL == wrq) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"input apidx: %d > %d BSSIDNUM\n", apidx, pAd->ApCfg.BssidNum);
		return -EFAULT;
	}

	inf_wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	Status = os_alloc_mem(pAd, (UCHAR **)&sta_info, sizeof(struct wifi_ioctl_sta_info));
	if (Status != NDIS_STATUS_SUCCESS) {
		return -ENOMEM;
	}

	NdisZeroMemory(sta_info, sizeof(struct wifi_ioctl_sta_info));
	sta_info->sta_cnt = 0;
	sta_info->rept_sta_cnt=0;

	for (idx = 0; idx < MAX_LEN_OF_MAC_TABLE; idx++) {
		pEnt = &pAd->MacTab.Content[idx];

		if (!pEnt || !inf_wdev || (inf_wdev != pEnt->wdev))
			continue;

		if (pEnt->Sst == SST_ASSOC) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"\nassoc(2):%d Addr "MACSTR";inf_wdev=%p,  pEnt->wdev=%p\n",
					pEnt->Sst, MAC2STR(pEnt->Addr), inf_wdev, pEnt->wdev);
#ifdef MAC_REPEATER_SUPPORT
				if (pAd->ApCfg.bMACRepeaterEn && pEnt &&
					(IS_ENTRY_PEER_AP(pEnt) || IS_ENTRY_APCLI(pEnt)) &&
					(pEnt->Sst == SST_ASSOC) && IS_REPT_LINK_UP(pEnt->pReptCli)) {
						if (sta_info->rept_sta_cnt >= MAX_ASSOC_NUM) {
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
								"rept_sta_cnt: %d > %d(MAX_ASSOC_NUM)\n",
								sta_info->rept_sta_cnt, MAX_ASSOC_NUM);
							break;
						}
						/*rept OMAC*/
						dst_ptr = sta_info->rept_item[sta_info->rept_sta_cnt].MacAddress;
						src_ptr = pEnt->pReptCli->OriginalAddress;
						NdisMoveMemory(dst_ptr, src_ptr, MAC_ADDR_LEN);

						/*rept VMAC*/
						dst_ptr = sta_info->rept_item[sta_info->rept_sta_cnt].VMacAddr;
						src_ptr = pEnt->pReptCli->CurrentAddress;
						NdisMoveMemory(dst_ptr, src_ptr, MAC_ADDR_LEN);
						sta_info->rept_sta_cnt++;
					}
#endif/*MAC_REPEATER_SUPPORT*/
					if (pEnt->Sst == SST_ASSOC && pEnt->EntryType == ENTRY_CLIENT) {
						if (sta_info->sta_cnt >= MAX_ASSOC_NUM) {
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
								"sta_cnt: %d > %d(MAX_ASSOC_NUM)\n", sta_info->sta_cnt, MAX_ASSOC_NUM);
							break;
						}

						dst_ptr = sta_info->item[sta_info->sta_cnt].MacAddress;
						NdisZeroMemory(dst_ptr, sizeof(struct wifi_ioctl_sta_info));/*omac*/
						dst_ptr = sta_info->item[sta_info->sta_cnt].VMacAddr;
						NdisZeroMemory(dst_ptr, sizeof(struct wifi_ioctl_sta_info));/*vmac*/

						/*sta OMAC*/
						dst_ptr = sta_info->item[sta_info->sta_cnt].MacAddress;
						src_ptr = pEnt->Addr;
						NdisMoveMemory(dst_ptr, src_ptr, MAC_ADDR_LEN);

						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"\nRmac "MACSTR"\n", MAC2STR(dst_ptr));
#ifdef MAC_REPEATER_SUPPORT
						pRepEnt = RTMPLookupRepeaterCliEntry(pAd, TRUE, pEnt->Addr, TRUE);
						if (pRepEnt != NULL && pAd->ApCfg.bMACRepeaterEn) {
							/*sta vamc*/
							dst_ptr = sta_info->item[sta_info->sta_cnt].VMacAddr;
							src_ptr = pRepEnt->CurrentAddress;
							NdisMoveMemory(dst_ptr, src_ptr, MAC_ADDR_LEN);
						} else
#endif/*MAC_REPEATER_SUPPORT*/
						{
							dst_ptr = sta_info->item[sta_info->sta_cnt].VMacAddr;
							src_ptr = pEnt->Addr;
							NdisMoveMemory(dst_ptr, src_ptr, MAC_ADDR_LEN);
						}

						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\nVmac "MACSTR"\n",
							MAC2STR(dst_ptr));
						/* RSSI */
						sta_info->item[sta_info->sta_cnt].RSSI = RTMPMaxRssi(pAd, pEnt->RssiSample.AvgRssi[0],
							pEnt->RssiSample.AvgRssi[1],pEnt->RssiSample.AvgRssi[2]);
						sta_info->item[sta_info->sta_cnt].UpTime = (ULONGLONG)pEnt->StaConnectTime;

						/*Rate*/
						get_sta_rate_info(pAd, pEnt, &tx_rate_info, &rx_rate_info);
						sta_info->item[sta_info->sta_cnt].TxRate = tx_rate_info.legacy;/*tx_rate*/
						sta_info->item[sta_info->sta_cnt].RxRate = rx_rate_info.legacy;

						/*Rate_rt*/
						#ifdef TXRX_STAT_SUPPORT
						sta_info->item[sta_info->sta_cnt].TxRate_rt = pEnt->TxDataPacketByte1SecValue.QuadPart;
						sta_info->item[sta_info->sta_cnt].RxRate_rt = pEnt->RxDataPacketByte1SecValue.QuadPart;
						#else/*TXRX_STAT_SUPPORT*/
						sta_info->item[sta_info->sta_cnt].TxRate_rt = pEnt->OneSecTxBytes;
						sta_info->item[sta_info->sta_cnt].RxRate_rt = pEnt->OneSecRxBytes;
						#endif/*NO-TXRX_STAT_SUPPORT*/
						sta_info->sta_cnt++;
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							"\n sta_cnt: %d\n", sta_info->sta_cnt);
					}
				}
			}

			if (wrq->u.data.length >= sizeof(struct wifi_ioctl_sta_info)) {
				wrq->u.data.length = sizeof(struct wifi_ioctl_sta_info);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"\n2: sta_cnt: %d\n", sta_info->sta_cnt);
				Status = copy_to_user(wrq->u.data.pointer, sta_info, wrq->u.data.length);
			} else
				Status = -EFAULT;

	os_free_mem(sta_info);

	return Status;
}

/**
* andlink_get_sta_hostname() - get station infos
* @pAd:	wifi adapter.
* @apidx:  ap interface index.
* @wrq: wireless structure
*
*Andlink get interface of this ap index stainfos:
* sent wireless event to andlink app.
*
* Return:
*		0 : success
*		other : Fail
*/
INT andlink_get_sta_hostname_ip
	(RTMP_ADAPTER *pAd,
	INT apidx,
	RTMP_IOCTL_INPUT_STRUCT	*wrq) {

	INT idx = 0;
	struct wifi_dev *inf_wdev = NULL;
	MAC_TABLE_ENTRY *pEnt = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pRepEnt = NULL;
#endif/*MAC_REPEATER_SUPPORT*/
	INT Status = NDIS_STATUS_SUCCESS;
	struct wifi_ioctl_hostname_ip *hostname_ip_info;

	if (!VALID_MBSS(pAd, apidx) || NULL == wrq) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"input apidx: %d > %d BSSIDNUM\n", apidx, pAd->ApCfg.BssidNum);
		return -EFAULT;
	}

	inf_wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	Status = os_alloc_mem(pAd, (UCHAR **)&hostname_ip_info, sizeof(struct wifi_ioctl_hostname_ip));
	if (Status != NDIS_STATUS_SUCCESS) {
		return -ENOMEM;
	}

	NdisZeroMemory(hostname_ip_info, sizeof(struct wifi_ioctl_hostname_ip));
	hostname_ip_info->sta_cnt = 0;
	hostname_ip_info->rept_sta_cnt=0;

    for (idx = 0; idx < MAX_LEN_OF_MAC_TABLE; idx++) {
		pEnt = &pAd->MacTab.Content[idx];
		if (!pEnt || !inf_wdev || (inf_wdev != pEnt->wdev))
			continue;

		if (pEnt->Sst != SST_ASSOC)
			continue;
#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn && pEnt &&
				(IS_ENTRY_APCLI(pEnt) || IS_ENTRY_PEER_AP(pEnt))
				&& (pEnt->Sst == SST_ASSOC) && (IS_REPT_LINK_UP(pEnt->pReptCli))) {

				if (hostname_ip_info->rept_sta_cnt >= MAX_ASSOC_NUM) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						"rept_sta_cnt: %d > %d(MAX_ASSOC_NUM)\n",
						hostname_ip_info->rept_sta_cnt, MAX_ASSOC_NUM);
					break;
				}
				pRepEnt = pEnt->pReptCli;

			NdisMoveMemory(hostname_ip_info->rept_item[hostname_ip_info->rept_sta_cnt].MacAddress,
					pRepEnt->OriginalAddress, MAC_ADDR_LEN);
				/*ipaddr*/
                hostname_ip_info->rept_item[hostname_ip_info->rept_sta_cnt].IpAddr = pRepEnt->ipaddr;
                /*hostname*/
				NdisZeroMemory(hostname_ip_info->rept_item[hostname_ip_info->rept_sta_cnt].HostName, HOSTNAME_LEN);
				NdisMoveMemory(hostname_ip_info->rept_item[hostname_ip_info->rept_sta_cnt].HostName, pRepEnt->hostname, MAC_ADDR_LEN);
				hostname_ip_info->rept_sta_cnt++;
		}
#endif/*MAC_REPEATER_SUPPORT*/
		if (pEnt->Sst == SST_ASSOC && pEnt->EntryType == ENTRY_CLIENT) {
			if (hostname_ip_info->sta_cnt >= MAX_ASSOC_NUM) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					"sta_cnt: %d > %d(MAX_ASSOC_NUM)\n",
					hostname_ip_info->sta_cnt, MAX_ASSOC_NUM);
				break;
			}

			NdisZeroMemory(hostname_ip_info->item[hostname_ip_info->sta_cnt].MacAddress, MAC_ADDR_LEN);
			NdisMoveMemory(hostname_ip_info->item[hostname_ip_info->sta_cnt].MacAddress, pEnt->Addr, MAC_ADDR_LEN);
#ifdef MAC_REPEATER_SUPPORT
			pRepEnt = RTMPLookupRepeaterCliEntry(pAd, TRUE, pEnt->Addr, TRUE);
			if(pRepEnt != NULL && pAd->ApCfg.bMACRepeaterEn) {
                /*omac*/
                NdisZeroMemory(hostname_ip_info->item[hostname_ip_info->sta_cnt].MacAddress, MAC_ADDR_LEN);
				NdisMoveMemory(hostname_ip_info->item[hostname_ip_info->sta_cnt].MacAddress, pRepEnt->OriginalAddress, MAC_ADDR_LEN);
                /*ipaddr*/
                hostname_ip_info->item[hostname_ip_info->sta_cnt].IpAddr = pRepEnt->ipaddr;
                /*hostname*/
                NdisZeroMemory(hostname_ip_info->item[hostname_ip_info->sta_cnt].HostName, HOSTNAME_LEN);
                NdisMoveMemory(hostname_ip_info->item[hostname_ip_info->sta_cnt].HostName, pRepEnt->hostname, HOSTNAME_LEN);
			} else
#endif/*MAC_REPEATER_SUPPORT*/
            {
                /*ipaddr*/
                hostname_ip_info->item[hostname_ip_info->sta_cnt].IpAddr = pEnt->ipaddr;
                /*hostname*/
                NdisZeroMemory(hostname_ip_info->item[hostname_ip_info->sta_cnt].HostName, HOSTNAME_LEN);
                NdisMoveMemory(hostname_ip_info->item[hostname_ip_info->sta_cnt].HostName, pEnt->hostname, HOSTNAME_LEN);
            }
			hostname_ip_info->sta_cnt++;
		}
    }

    if (wrq->u.data.length >= sizeof(struct wifi_ioctl_hostname_ip)) {
		wrq->u.data.length = sizeof(struct wifi_ioctl_hostname_ip);
		Status = copy_to_user(wrq->u.data.pointer, hostname_ip_info, wrq->u.data.length);
    } else
	    Status = -EFAULT;

	os_free_mem(hostname_ip_info);

	return Status;
}



#ifdef ANDLINK_V4_0
/**
* andlink_get_stainfos() - get station infos
* @pAd:	wifi adapter.
* @apidx:  ap interface index.
* @wrq: wireless structure
*
*Andlink get interface of this ap index stainfos:
* sent wireless event to andlink app.
*
* Return:
*		0 : success
*		other : Fail
*/
INT andlink_get_stainfos_v4_0 (
	RTMP_ADAPTER *pAd,
	INT apidx,
	RTMP_IOCTL_INPUT_STRUCT	*wrq) {
	INT idx = 0, if_idx = 0;
	UCHAR *dst_ptr = NULL;
	UCHAR *src_ptr = NULL;
	MAC_TABLE_ENTRY *pEnt = NULL;
	INT	Status = NDIS_STATUS_SUCCESS;
	INT ret = 0, offset = 0, sta_idx = 0;
	struct mtk_andlink_wifi_sta_info *sta_info = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pRepEnt = NULL;
#endif/*MAC_REPEATER_SUPPORT*/
	struct wifi_dev *inf_wdev = NULL;
	struct mtk_rate_info tx_rate_info;
	struct mtk_rate_info rx_rate_info;

	if (!VALID_MBSS(pAd, apidx) || NULL == wrq||
		wrq->u.data.length > sizeof(struct mtk_andlink_wifi_sta_info)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"input apidx: %d > %d BSSIDNUM, user len(%d) or kernel mem len(%uz)\n",
					apidx, pAd->ApCfg.BssidNum, wrq->u.data.length,
					sizeof(struct mtk_andlink_wifi_sta_info));
		return -EFAULT;
	}

	inf_wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	Status = os_alloc_mem(pAd, (UCHAR **)&sta_info, sizeof(struct mtk_andlink_wifi_sta_info));
	if (Status != NDIS_STATUS_SUCCESS) {
		return -ENOMEM;
	}

	NdisZeroMemory(sta_info, sizeof(struct mtk_andlink_wifi_sta_info));

	ret = copy_from_user(sta_info, wrq->u.data.pointer, wrq->u.data.length);
	if (0 != ret) {
		os_free_mem(sta_info);
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"copy_from_user fail.\n");
		return -EFAULT;
	}
	offset = sta_info->offset;

	NdisZeroMemory(sta_info, sizeof(struct mtk_andlink_wifi_sta_info));
	sta_info->sta_cnt = 0;

	for (idx = 0; idx < MAX_LEN_OF_MAC_TABLE; idx++) {
		pEnt = &pAd->MacTab.Content[idx];

		if (!pEnt || !inf_wdev || (inf_wdev != pEnt->wdev))
			continue;

		if (pEnt->Sst == SST_ASSOC) {
				sta_idx++;
				if (sta_idx <= offset)
					continue;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"\nassoc(2):%d Addr %02x:%02x:%02x:%02x:%02x:%02x;inf_wdev=%p, pEnt->wdev=%p\n",
					pEnt->Sst, PRINT_MAC(pEnt->Addr), inf_wdev, pEnt->wdev);
#ifdef MAC_REPEATER_SUPPORT
				if (pAd->ApCfg.bMACRepeaterEn && pEnt &&
					(IS_ENTRY_PEER_AP(pEnt) || IS_ENTRY_APCLI(pEnt)) &&
					(pEnt->Sst == SST_ASSOC) && IS_REPT_LINK_UP(pEnt->pReptCli)) {
						if (sta_info->sta_cnt >= ANDLINK_MAX_ASSOC_NUM) {
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							"rept_sta_cnt: %d > %d(MAX_ASSOC_NUM)\n",
							sta_info->sta_cnt, ANDLINK_MAX_ASSOC_NUM);
							break;
						}
						/*rept OMAC*/
						dst_ptr = sta_info->sta_entry[sta_info->sta_cnt].mac_addr;
						src_ptr = pEnt->pReptCli->OriginalAddress;
						NdisMoveMemory(dst_ptr, src_ptr, MAC_ADDR_LEN);

						/*rept VMAC*/
						dst_ptr = sta_info->sta_entry[sta_info->sta_cnt].vmac_addr;
						src_ptr = pEnt->pReptCli->CurrentAddress;
						NdisMoveMemory(dst_ptr, src_ptr, MAC_ADDR_LEN);
					}
#endif/*MAC_REPEATER_SUPPORT*/
					if (pEnt->Sst == SST_ASSOC && pEnt->EntryType == ENTRY_CLIENT) {
						if (sta_info->sta_cnt >= ANDLINK_MAX_ASSOC_NUM) {
							MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							"sta_cnt: %d > %d(MAX_ASSOC_NUM)\n",
							sta_info->sta_cnt, ANDLINK_MAX_ASSOC_NUM);
							break;
						}

						/*sta OMAC*/
						dst_ptr = sta_info->sta_entry[sta_info->sta_cnt].mac_addr;
						src_ptr = pEnt->Addr;
						NdisMoveMemory(dst_ptr, src_ptr, MAC_ADDR_LEN);

						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							"\nRmac %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(dst_ptr));
#ifdef MAC_REPEATER_SUPPORT
						pRepEnt = RTMPLookupRepeaterCliEntry(pAd, TRUE, pEnt->Addr, TRUE);
						if (pRepEnt != NULL && pAd->ApCfg.bMACRepeaterEn) {
							/*sta vamc*/
							dst_ptr = sta_info->sta_entry[sta_info->sta_cnt].vmac_addr;
							src_ptr = pRepEnt->CurrentAddress;
							NdisMoveMemory(dst_ptr, src_ptr, MAC_ADDR_LEN);
						} else
#endif/*MAC_REPEATER_SUPPORT*/
						{
							dst_ptr = sta_info->sta_entry[sta_info->sta_cnt].vmac_addr;
							src_ptr = pEnt->Addr;
							NdisMoveMemory(dst_ptr, src_ptr, MAC_ADDR_LEN);
						}

						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "\nVmac %02x:%02x:%02x:%02x:%02x:%02x\n",
							PRINT_MAC(dst_ptr));
						/* RSSI */
						sta_info->sta_entry[sta_info->sta_cnt].rssi= RTMPMaxRssi(pAd, pEnt->RssiSample.AvgRssi[0],
							pEnt->RssiSample.AvgRssi[1],pEnt->RssiSample.AvgRssi[2]);
						sta_info->sta_entry[sta_info->sta_cnt].uptime = (ULONGLONG)pEnt->StaConnectTime;

						/*Rate*/
						get_sta_rate_info(pAd, pEnt, &tx_rate_info, &rx_rate_info);
						sta_info->sta_entry[sta_info->sta_cnt].tx_rate= tx_rate_info.legacy;/*tx_rate*/
						sta_info->sta_entry[sta_info->sta_cnt].rx_rate = rx_rate_info.legacy;
						/*bw*/
						sta_info->sta_entry[sta_info->sta_cnt].bw = rx_rate_info.bw;

						/*Rate_rt*/
						#ifdef TXRX_STAT_SUPPORT
						sta_info->sta_entry[sta_info->sta_cnt].tx_rate_rt = pEnt->TxDataPacketByte1SecValue.QuadPart / 128;/*kbps*/
						sta_info->sta_entry[sta_info->sta_cnt].rx_rate_rt = pEnt->RxDataPacketByte1SecValue.QuadPart / 128;/*kbps*/
						for (if_idx = 0; if_idx < ANDLINK_IF_MAX; if_idx++) {
							sta_info->sta_entry[sta_info->sta_cnt].avg_tx_rate[if_idx] = pEnt->andlink_avg_tx_rate[if_idx] / 128;
							sta_info->sta_entry[sta_info->sta_cnt].avg_rx_rate[if_idx] = pEnt->andlink_avg_rx_rate[if_idx] / 128;
							sta_info->sta_entry[sta_info->sta_cnt].max_tx_rate[if_idx] = pEnt->andlink_max_tx_rate[if_idx] / 128;
							sta_info->sta_entry[sta_info->sta_cnt].max_rx_rate[if_idx] = pEnt->andlink_max_rx_rate[if_idx] / 128;
						}
						sta_info->sta_entry[sta_info->sta_cnt].tx_pkts = (ULONGLONG)pEnt->TxDataPacketCount.QuadPart;
						sta_info->sta_entry[sta_info->sta_cnt].rx_pkts = (ULONGLONG)pEnt->RxDataPacketCount.QuadPart;
						sta_info->sta_entry[sta_info->sta_cnt].tx_bytes = (ULONGLONG)pEnt->TxDataPacketByte.QuadPart;
						sta_info->sta_entry[sta_info->sta_cnt].rx_bytes = (ULONGLONG)pEnt->RxDataPacketByte.QuadPart;
						#endif/*TXRX_STAT_SUPPORT*/
						sta_info->sta_cnt++;

						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "\n sta_cnt: %d\n", sta_info->sta_cnt);
					}
				}
			}

			if (wrq->u.data.length >= sizeof(struct mtk_andlink_wifi_sta_info)) {
				wrq->u.data.length = sizeof(struct mtk_andlink_wifi_sta_info);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "\n: sta_cnt: %d\n", sta_info->sta_cnt);
				Status = copy_to_user(wrq->u.data.pointer, sta_info, wrq->u.data.length);
			} else
				Status = -EFAULT;

	os_free_mem(sta_info);

	return Status;
}

INT	andlink_get_scan_result_info_v4_0(
	RTMP_ADAPTER *pAd,
	INT ap_idx,
	RTMP_IOCTL_INPUT_STRUCT	*wrq)
{
	INT idx = 0;
	INT bss_idx = 0;
	INT ret = 0;
	INT offset = 0;
	INT	status = NDIS_STATUS_SUCCESS;
	//struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct wifi_dev *wdev =  NULL;
	struct mtk_wifi_scan_info *andlink_scan_info = NULL;
	BSS_TABLE *scan_tab = NULL;
	BSS_ENTRY *bss = NULL;
	NDIS_802_11_NETWORK_TYPE    wireless_mode;
	int bw20 = 1, bw40 = 2, bw80 = 3, bw160 = 4, bw80_80 = 5;

	if (!VALID_MBSS(pAd, ap_idx) || NULL == wrq||
		wrq->u.data.length > sizeof(struct mtk_andlink_wifi_sta_info)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"input apidx: %d > %d BSSIDNUM, user len(%d) or kernel mem len(%uz)\n",
					ap_idx, pAd->ApCfg.BssidNum, wrq->u.data.length,
					sizeof(struct mtk_andlink_wifi_sta_info));
		return -EFAULT;
	}

	status = os_alloc_mem(pAd, (UCHAR **)&andlink_scan_info, sizeof(struct mtk_wifi_scan_info));
	if (status != NDIS_STATUS_SUCCESS) {
		return -ENOMEM;
	}

	NdisZeroMemory(andlink_scan_info, sizeof(struct mtk_wifi_scan_info));

	ret = copy_from_user(andlink_scan_info, wrq->u.data.pointer, wrq->u.data.length);
	offset = andlink_scan_info->offset;

	NdisZeroMemory(andlink_scan_info, sizeof(struct mtk_wifi_scan_info));
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
	scan_tab = get_scan_tab_by_wdev(pAd, wdev);

	if (NULL == scan_tab) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"scan_tab is null(%p).\n", scan_tab);
		return -EFAULT;
	}

	for (idx = 0; idx < scan_tab->BssNr; idx++) {
		if (idx < offset)
			continue;
		bss = &scan_tab->BssEntry[idx];
		if (NULL == bss || bss->SsidLen <= 0 || bss->Channel == 0) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					"bss(%p)  SSID(%s) or channel(%d) is invalide.\n",
					bss, bss->Ssid, bss->Channel);
			continue;
		}
		bss_idx = idx - offset;
		if ( bss_idx >= ANDLINK_MAX_WLAN_NEIGHBOR) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"bss(%p) is null or bss_idx >= %d\n", bss, bss_idx);
			break;
		}

		NdisMoveMemory(andlink_scan_info->scan_entry[bss_idx].ssid, bss->Ssid, SSID_LEN-1);/*bss*/
		COPY_MAC_ADDR(andlink_scan_info->scan_entry[bss_idx].mac_addr, bss->Bssid);/*mac_addr*/
		andlink_scan_info->scan_entry[bss_idx].channel = bss->Channel;/*channel*/
		andlink_scan_info->scan_entry[bss_idx].rssi = bss->Rssi;/*rssi*/
		/* bandwidth*/
		if (WMODE_CAP_6G(wdev->PhyMode))
			andlink_scan_info->scan_entry[bss_idx].bandwidth = 0;
		else if (WMODE_CAP_5G(wdev->PhyMode)) {
			if (bss->vht_op_ie.vht_op_info.ch_width == 3)
				andlink_scan_info->scan_entry[bss_idx].bandwidth = bw80_80; /*80+80MHz*/
			else if (bss->vht_op_ie.vht_op_info.ch_width == 2)
				andlink_scan_info->scan_entry[bss_idx].bandwidth = bw160; /*160MHz*/
			else if (bss->vht_op_ie.vht_op_info.ch_width == 1) {
				if (bss->vht_op_ie.vht_op_info.ccfs_1 == 0)
					andlink_scan_info->scan_entry[bss_idx].bandwidth = bw80; /*80MHz*/
				else {
					if (((bss->vht_op_ie.vht_op_info.ccfs_1) - (bss->vht_op_ie.vht_op_info.ccfs_0)) == 8
							|| ((bss->vht_op_ie.vht_op_info.ccfs_1) - (bss->vht_op_ie.vht_op_info.ccfs_0)) == -8)
						andlink_scan_info->scan_entry[bss_idx].bandwidth = bw160; /*160MHz*/
					else
						andlink_scan_info->scan_entry[bss_idx].bandwidth = bw80_80; /*80+80MHz*/
				}
			} else if (bss->vht_op_ie.vht_op_info.ch_width == 0 && bss->vht_op_ie.vht_op_info.ccfs_0 == 1)
				andlink_scan_info->scan_entry[bss_idx].bandwidth = bw40; /*40MHz*/
			else
				andlink_scan_info->scan_entry[bss_idx].bandwidth = bw20; /*20MHz*/
		} else if (bss->HtCapability.HtCapInfo.ChannelWidth == HT_BW_20)
			andlink_scan_info->scan_entry[bss_idx].bandwidth = bw20;/*20*/
		else
			andlink_scan_info->scan_entry[bss_idx].bandwidth = bw20;/*40MHz*/
		/* wifistandard*/
		wireless_mode = NetworkTypeInUseSanity(bss);
		if (wireless_mode == Ndis802_11OFDM5)
			andlink_scan_info->scan_entry[bss_idx].wifistandard = 1;
		else if (wireless_mode == Ndis802_11OFDM5_N || wireless_mode == Ndis802_11OFDM24_N)
			andlink_scan_info->scan_entry[bss_idx].wifistandard = 2;
		else if (wireless_mode == Ndis802_11OFDM5_AC)
			andlink_scan_info->scan_entry[bss_idx].wifistandard = 3;
		else if (wireless_mode == Ndis802_11OFDM24_HE || wireless_mode == Ndis802_11OFDM5_HE)
			andlink_scan_info->scan_entry[bss_idx].wifistandard = 4;
		else
			andlink_scan_info->scan_entry[bss_idx].wifistandard = 0;

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"idx(%d), SSID(%s)\n", bss_idx, bss->Ssid);
	}
	andlink_scan_info->num = bss_idx;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"bss_idx(%d), offset(%d), ssid(%s).\n",
					bss_idx, offset, bss->Ssid);

	if (wrq->u.data.length >= sizeof(struct mtk_wifi_scan_info)) {
		wrq->u.data.length = sizeof(struct mtk_wifi_scan_info);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"\ncopy_to_user: bss_idx(%d)\n", bss_idx);
		ret = copy_to_user(wrq->u.data.pointer, andlink_scan_info, wrq->u.data.length);
	} else
		ret = -EFAULT;

	os_free_mem(andlink_scan_info);

	return ret;
}


INT	andlink_get_uplink_info_v4_0(
	RTMP_ADAPTER *pAd,
	INT apcli_idx,
	RTMP_IOCTL_INPUT_STRUCT	*wrq)
{
	INT idx = 0, if_idx = 0;
	INT	status = NDIS_STATUS_SUCCESS;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct wifi_dev *wdev =  NULL;
	struct mtk_wifi_uplink_info andlink_uplink_info;

	if (apcli_idx >= MAX_APCLI_NUM || NULL == wrq) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"input apidx: %d > MAX_APCLI_NUM(%d) or wrq: %p\n",
					apcli_idx, MAX_APCLI_NUM, wrq);
		return -EFAULT;
	}
	/*init andlink_uplink_info*/
	NdisZeroMemory(&andlink_uplink_info, sizeof(struct mtk_wifi_uplink_info));

	wdev = &pAd->StaCfg[apcli_idx].wdev;
	if (!wdev || !(HcGetBandByWdev(wdev) < DBDC_BAND_NUM) ||
		TRUE != pAd->CommonCfg.andlink_enable[HcGetBandByWdev(wdev)]) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"wdev is null or andlink enbable(%d) in band(%d).\n",
					pAd->CommonCfg.andlink_enable[HcGetBandByWdev(wdev)],
					HcGetBandByWdev(wdev));
		return -EFAULT;
	}

	/*find apcli entry*/
	if (((GetAssociatedAPByWdev(pAd, wdev)) != NULL) && (pAd->StaCfg[apcli_idx].SsidLen != 0)) {
		for (idx = 0; VALID_UCAST_ENTRY_WCID(pAd, idx); idx++) {
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[idx];
			STA_TR_ENTRY *tr_entry = &tr_ctl->tr_entry[idx];
			if (IS_ENTRY_PEER_AP(pEntry)
				&& (pEntry->Sst == SST_ASSOC)
				&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
				if (pEntry->wdev == &pAd->StaCfg[apcli_idx].wdev) {
					//CHAR rssi[4] = {-127, -127, -127, -127};
					//PSTA_ADMIN_CONFIG psta_cfg = &pAd->StaCfg[apcli_idx];
					PMAC_TABLE_ENTRY apcli_entry = NULL;
					mtk_rate_info_t tx_rate;
					mtk_rate_info_t rx_rate;


					NdisMoveMemory(&andlink_uplink_info.ssid, pAd->StaCfg[apcli_idx].Ssid,
					pAd->StaCfg[apcli_idx].SsidLen > MAX_LEN_OF_SSID ? MAX_LEN_OF_SSID : pAd->StaCfg[apcli_idx].SsidLen);
					/*channel*/
					andlink_uplink_info.channel = pAd->StaCfg[apcli_idx].wdev.channel;

					if ((apcli_idx >= 0) && IS_WCID_VALID(pAd, pAd->StaCfg[apcli_idx].MacTabWCID))
						apcli_entry = &pAd->MacTab.Content[pAd->StaCfg[apcli_idx].MacTabWCID];

					if (apcli_entry) {
						/*rssi*/
						andlink_uplink_info.rssi = RTMPMaxRssi(pAd, apcli_entry->RssiSample.AvgRssi[0],
													apcli_entry->RssiSample.AvgRssi[1],
													apcli_entry->RssiSample.AvgRssi[2]);
						/*noise*/
						andlink_uplink_info.noise = RTMPMaxRssi(pAd, pAd->ApCfg.RssiSample.AvgRssi[0],
							pAd->ApCfg.RssiSample.AvgRssi[1],pAd->ApCfg.RssiSample.AvgRssi[2]) -
							RTMPMinSnr(pAd, pAd->ApCfg.RssiSample.AvgSnr[0],pAd->ApCfg.RssiSample.AvgSnr[1]);

						/*snr*/
						andlink_uplink_info.snr = andlink_uplink_info.rssi - andlink_uplink_info.noise;
					}

					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "ApCli%d		   rssi: %d; snr: %d; noise: %d\n",
							  apcli_idx, andlink_uplink_info.rssi, andlink_uplink_info.snr, andlink_uplink_info.noise);

					if (TRUE == get_sta_rate_info(pAd, pEntry, &tx_rate, &rx_rate)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							 "ApCli%d         Tx_Rate: %d Mb/s; Rx_Rate: %d Mb/s;\n",
							  apcli_idx, tx_rate.legacy, rx_rate.legacy);
						/*tx/rx_rate*/
						andlink_uplink_info.tx_rate = (UINT)tx_rate.legacy;
						andlink_uplink_info.rx_rate = (UINT)rx_rate.legacy;
					}

					/*tx/rx_rate_rt*/
					andlink_uplink_info.tx_rate_rt = pEntry->TxDataPacketByte1SecValue.QuadPart / 128;/*kbps*/
					andlink_uplink_info.rx_rate_rt = pEntry->TxDataPacketByte1SecValue.QuadPart / 128;/*kbps*/
					for (if_idx = 0; if_idx < ANDLINK_IF_MAX; if_idx++) {
						/*avg_tx/rx_rt*/
						andlink_uplink_info.avg_tx_rate[if_idx] = pAd->StaCfg[apcli_idx].andlink_avg_tx_rate[if_idx] / 128 ;
						andlink_uplink_info.avg_rx_rate[if_idx] = pAd->StaCfg[apcli_idx].andlink_avg_rx_rate[if_idx] / 128;
						/*max_tx/rx_rt*/
						andlink_uplink_info.max_tx_rate[if_idx] = pAd->StaCfg[apcli_idx].andlink_max_tx_rate[if_idx] / 128;
						andlink_uplink_info.max_rx_rate[if_idx] = pAd->StaCfg[apcli_idx].andlink_max_rx_rate[if_idx] / 128;
					}
				}
			}
		}
	}
	if (wrq->u.data.length >= sizeof(struct mtk_wifi_uplink_info)) {
			wrq->u.data.length = sizeof(struct mtk_wifi_uplink_info);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"\ncopy_to_user\n");
			status = copy_to_user(wrq->u.data.pointer, &andlink_uplink_info, wrq->u.data.length);
		} else
			status = -EFAULT;

	return status;
}
/**
* andlink_get_bss_infos() - get radio or bss infos
* @pAd:	wifi adapter.
* @apidx:  ap interface index.
* @wrq: wireless structure
*
*Andlink get interface of this ap band or bss infos to andlink app.
*
* Return:
*		0 : success
*		other : Fail
*/
INT	andlink_get_bss_info_v4_0(
	RTMP_ADAPTER *pAd,
	INT ap_idx,
	RTMP_IOCTL_INPUT_STRUCT	*wrq)
{
	INT band_idx = 0;
	INT	status = NDIS_STATUS_SUCCESS;
	struct wifi_dev *wdev =  NULL;
	struct mtk_andlink_radio_info radio_info;
	RX_STATISTIC_RXV *rx_stat_rxv = NULL;

	if (ap_idx >= MAX_BEACON_NUM || NULL == wrq) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"input apidx: %d > MAX_APCLI_NUM(%d) or wrq: %p\n",
					ap_idx, MAX_BEACON_NUM, wrq);
		return -EFAULT;
	}

	NdisZeroMemory(&radio_info, sizeof(radio_info));
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
	band_idx = HcGetBandByWdev(wdev);
	rx_stat_rxv = pAd->rx_stat_rxv + band_idx;

	/*band snr/rssi/noise infos*/
	radio_info.rssi = RTMPMaxRssi(pAd, rx_stat_rxv->RSSI[0], rx_stat_rxv->RSSI[1], rx_stat_rxv->RSSI[2]);
	radio_info.snr = rx_stat_rxv->SNR[0];
	radio_info.noise = radio_info.rssi - radio_info.snr;

	if (wrq->u.data.length >= sizeof(radio_info)) {
		wrq->u.data.length = sizeof(radio_info);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"\nrssi(%d),snr(%d) and noise(%d).\n",
			radio_info.rssi, radio_info.snr, radio_info.noise);
		status = copy_to_user(wrq->u.data.pointer, &radio_info, wrq->u.data.length);
	} else
		status = -EFAULT;

	return status;
}

#endif/*ANDLINK_V4_0*/

#endif/*ANDLINK_FEATURE_SUPPORT*/


#ifdef TR181_SUPPORT
void Get_Sta_Info(
	IN RTMP_ADAPTER * pAd,
	IN OUT WIFI_STATION * sta_info)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	mtk_rate_info_t tx_rate, rx_rate;

	sta_info->valid = FALSE;

	pEntry = MacTableLookup(pAd, sta_info->MacAddr);
	if (!(IS_VALID_ENTRY(pEntry)) || (pEntry->Sst != SST_ASSOC))
		return;

	if (pEntry->wdev)
		sta_info->PhyMode = pEntry->wdev->PhyMode;

	sta_info->LastConnectTime = pEntry->StaConnectTime;
	if (pEntry->wcid < MAX_LEN_OF_TR_TABLE)
		sta_info->AuthenticationState =
			(pAd->tr_ctl.tr_entry[pEntry->tr_tb_idx].PortSecured == WPA_802_1X_PORT_SECURED) ? TRUE : FALSE;

	sta_info->SignalStrength = rtmp_avg_rssi(pAd, &pEntry->RssiSample);
	sta_info->Noise = RTMPMaxRssi2(pAd, pEntry->RssiSample.AvgRssi[0],
										pEntry->RssiSample.AvgRssi[1],
										pEntry->RssiSample.AvgRssi[2],
										pEntry->RssiSample.AvgRssi[3]) -
						RTMPMinSnr(pAd, pEntry->RssiSample.AvgSnr[0],
										pEntry->RssiSample.AvgSnr[1]);

	sta_info->Retransmissions = 0; /* not support */
	sta_info->UtilizationReceive = pEntry->TxRxTime[0][0] +
									pEntry->TxRxTime[1][0] +
									pEntry->TxRxTime[2][0] +
									pEntry->TxRxTime[3][0];
	sta_info->UtilizationTransmit =	pEntry->TxRxTime[0][1] +
									pEntry->TxRxTime[1][1] +
									pEntry->TxRxTime[2][1] +
									pEntry->TxRxTime[3][1];
#ifdef MAC_REPEATER_SUPPORT
	if (pAd->ApCfg.bMACRepeaterEn && (IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_APCLI(pEntry)) &&
		IS_REPT_LINK_UP(pEntry->pReptCli)) {
		NdisCopyMemory(sta_info->VMacAddr, pEntry->pReptCli->CurrentAddress,
			MAC_ADDR_LEN);
		sta_info->repeater = TRUE;
	}
#endif/*MAC_REPEATER_SUPPORT*/
	sta_info->valid = TRUE;

	/* STATS */
	sta_info->stats.BytesReceived = pEntry->RxBytes;
	sta_info->stats.BytesSent = pEntry->TxBytes;
	sta_info->stats.PacketsReceived = pEntry->RxPackets.QuadPart;
	sta_info->stats.PacketsSent = pEntry->TxPackets.QuadPart;
	sta_info->stats.ErrorsSent = 0;
	sta_info->stats.ErrorsReceived = 0;
	sta_info->stats.RetransCount = 0;
	sta_info->stats.FailedRetransCount = 0;
	sta_info->stats.RetryCount = 0;
	sta_info->stats.MultipleRetryCount = 0;

	/* RATE */
	os_zero_mem(&tx_rate, sizeof(mtk_rate_info_t));
	os_zero_mem(&rx_rate, sizeof(mtk_rate_info_t));
	if (pEntry->wdev && (get_sta_rate_info(pAd, pEntry, &tx_rate, &rx_rate) == TRUE)) {
		sta_info->rates.TxRate = tx_rate.legacy;
		sta_info->rates.RxRate = rx_rate.legacy;
	}
	/* Mbps = (n Bytes x 8) / (1024 * 1024) = (n >> 17) */;
	sta_info->rates.TxRate_rt = pEntry->AvgTxBytes >> 17;
	sta_info->rates.RxRate_rt = pEntry->AvgRxBytes >> 17;
	sta_info->rates.avg_tx_rate = 0; /* not support */
	sta_info->rates.avg_rx_rate = 0; /* not support */
}

static void Get_Sta_Stats(
	IN RTMP_ADAPTER * pAd,
	IN OUT WIFI_STA_STATS * staStats)
{
	PMAC_TABLE_ENTRY pEntry = NULL;

	staStats->valid = FALSE;

	pEntry = MacTableLookup(pAd, staStats->MacAddr);
	if (!(IS_VALID_ENTRY(pEntry)) || (pEntry->Sst != SST_ASSOC))
		return;

	staStats->stats.BytesReceived = pEntry->RxBytes;
	staStats->stats.BytesSent = pEntry->TxBytes;
	staStats->stats.PacketsReceived = pEntry->RxPackets.QuadPart;
	staStats->stats.PacketsSent = pEntry->TxPackets.QuadPart;
	staStats->stats.ErrorsSent = 0;
	staStats->stats.ErrorsReceived = 0;
	staStats->stats.RetransCount = 0;
	staStats->stats.FailedRetransCount = 0;
	staStats->stats.RetryCount = 0;
	staStats->stats.MultipleRetryCount = 0;
	staStats->valid = TRUE;
}

static void Get_Sta_Rates(
	IN RTMP_ADAPTER * pAd,
	IN OUT WIFI_STA_RATES * staRates)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	mtk_rate_info_t tx_rate, rx_rate;

	staRates->valid = FALSE;

	pEntry = MacTableLookup(pAd, staRates->MacAddr);
	if (!(IS_VALID_ENTRY(pEntry)) || (pEntry->Sst != SST_ASSOC))
		return;

	/* RATE */
	os_zero_mem(&tx_rate, sizeof(mtk_rate_info_t));
	os_zero_mem(&rx_rate, sizeof(mtk_rate_info_t));
	if (pEntry->wdev && (get_sta_rate_info(pAd, pEntry, &tx_rate, &rx_rate) == TRUE)) {
		staRates->rates.TxRate = tx_rate.legacy;
		staRates->rates.RxRate = rx_rate.legacy;
	}
	/* Mbps = (n Bytes x 8) / (1024 * 1024) = (n >> 17) */;
	staRates->rates.TxRate_rt = pEntry->AvgTxBytes >> 17;
	staRates->rates.RxRate_rt = pEntry->AvgRxBytes >> 17;
	staRates->rates.avg_tx_rate = 0; /* not support */
	staRates->rates.avg_rx_rate = 0; /* not support */
	staRates->valid = TRUE;
}

static INT Get_Radio_Channel_Change_Info(
	IN RTMP_ADAPTER * pAd,
	OUT UCHAR band_idx,
	OUT RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
	ULONG TNow;
	UINT32	Time, TimeDelta;
	INT	Status = NDIS_STATUS_SUCCESS;

	NdisGetSystemUpTime(&TNow);
	Time = jiffies_to_usecs(TNow);
	TimeDelta = Time - ctrl->rdev[band_idx].pRadioCtrl->CurChannelUpTime;

	wrq->u.data.length = sizeof(UINT32);
	Status = copy_to_user(wrq->u.data.pointer, &TimeDelta, wrq->u.data.length);

	return Status;
}

static INT Get_Dev_Ac(
	IN RTMP_ADAPTER * pAd,
	IN UINT ap_idx,
	OUT WIFI_DEV_AC * ac)
{
	int i;
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
	struct wlan_config *cfg = NULL;

	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;

	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pBssEdca is NULL\n");
		return NDIS_STATUS_FAILURE;
	}

	cfg = (struct wlan_config *)wdev->wpf_cfg;
	if (!cfg) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wpf_cfg NULL\n");
		return NDIS_STATUS_FAILURE;
	}
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		ac[i].AccessCategory = i;
		ac[i].AIFSN = pBssEdca->Aifsn[i];
		ac[i].ECWMin = pBssEdca->Cwmin[i];
		ac[i].ECWMax = pBssEdca->Cwmax[i];
		ac[i].TxOpMax = pBssEdca->Txop[i];
		ac[i].AckPolicy = cfg->phy_conf.ack_policy[i];
	}
	return NDIS_STATUS_SUCCESS;
}

#ifdef STAT_ENHANCE_SUPPORT
static INT Get_BSS_AC_Statistics(
	IN RTMP_ADAPTER * pAd,
	IN UINT ap_idx,
	OUT WIFI_DEV_AC_STATS * pStat)
{
	BSS_STRUCT *pMbss;
	INT i;
	if (!VALID_MBSS(pAd, ap_idx)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Error AP index \n");
		return -EINVAL;
	}

	pMbss = &pAd->ApCfg.MBSSID[ap_idx];

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pStat[i].AccessCategory = i;
		pStat[i].PacketsReceived =  pMbss->RxCountPerAC[i];
		pStat[i].BytesReceived = pMbss->ReceivedByteCountPerAC[i];
		pStat[i].ErrorsReceived =  pMbss->RxErrorCountPerAC[i];
		pStat[i].DiscardPacketsReceived =  pMbss->RxDropCountPerAC[i];
		pStat[i].PacketsSent = 0; /* Not Support */
		pStat[i].BytesSent = 0; /* Not Support */
		pStat[i].ErrorsSent =  0; /* Not Support */
		pStat[i].DiscardPacketsSent = 0; /* Not Support */
		pStat[i].RetransCount = 0; /* Not Support */
	}
	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef RADIUS_ACCOUNTING_SUPPORT
static void Get_Accounting_Server(
	IN RTMP_ADAPTER * pAd,
	IN UINT ap_idx,
	OUT WIFI_ACCOUNTING_SERVER * accounting_server)
{
	os_zero_mem(accounting_server, sizeof(WIFI_ACCOUNTING_SERVER));
	accounting_server->ServerIPAddr[0] = pAd->ApCfg.MBSSID[ap_idx].wdev.SecConfig.radius_acct_srv_info[0].radius_ip;
	accounting_server->ServerIPAddr[1] = pAd->ApCfg.MBSSID[ap_idx].wdev.SecConfig.radius_acct_srv_info[1].radius_ip;
	accounting_server->ServerPort[0] = pAd->ApCfg.MBSSID[ap_idx].wdev.SecConfig.radius_acct_srv_info[0].radius_port;
	accounting_server->ServerPort[1] =  pAd->ApCfg.MBSSID[ap_idx].wdev.SecConfig.radius_acct_srv_info[1].radius_port;
	NdisCopyMemory(accounting_server->Secret[0],
		pAd->ApCfg.MBSSID[ap_idx].wdev.SecConfig.radius_acct_srv_info[0].radius_key,
		pAd->ApCfg.MBSSID[ap_idx].wdev.SecConfig.radius_acct_srv_info[0].radius_key_len);
	NdisCopyMemory(accounting_server->Secret[1],
		pAd->ApCfg.MBSSID[ap_idx].wdev.SecConfig.radius_acct_srv_info[1].radius_key,
		pAd->ApCfg.MBSSID[ap_idx].wdev.SecConfig.radius_acct_srv_info[1].radius_key_len);
}
#endif

UCHAR Get_ScanSecurityMode(
	IN UINT32 authMode)
{
	/*	0: None
		1: WEP
		2: WPA
		3: WPA2
		4: WPA-WPA2
		5: WPA-Enterprise
		6: WPA2-Enterprise
		7: WPA-WPA2-Enterprise
		8: WPA3-SAE
		9: WPA2-PSK-WPA3-SAE
		10: WPA3-Enterprise */

	if (IS_AKM_OPEN(authMode))
		return 0;
	else if (IS_AKM_AUTOSWITCH(authMode))
		return 1;
	else if (IS_AKM_WPA1PSK(authMode))
		return 2;
	else if (IS_AKM_WPA2PSK(authMode))
		return 3;
	else if (IS_AKM_FT_WPA2PSK(authMode) && IS_AKM_WPA1PSK(authMode))
		return 4;
	else if (IS_AKM_WPA1(authMode))
		return 5;
	else if (IS_AKM_WPA2(authMode))
		return 6;
	else if (IS_AKM_WPA1(authMode) && IS_AKM_WPA2(authMode))
		return 7;
	else if (IS_AKM_FT_SAE_SHA256(authMode) && IS_AKM_WPA3PSK(authMode))
		return 8;
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_WPA3PSK(authMode) && IS_AKM_FT_SAE_SHA256(authMode))
		return 9;
	else if (IS_AKM_WPA3(authMode))
		return 10;
	else
		return 0;
}

UCHAR Get_ScanEncryMode(
	IN UINT32 encryMode)
{
	/*0: TKIP
	  1: AES
	  2: TKIP AES Mix
	  3: WEP
	  4: None*/
	if (IS_CIPHER_TKIP(encryMode))
		return 0;
	else if (IS_CIPHER_CCMP128(encryMode))
		return 1;
	else if (IS_CIPHER_TKIP(encryMode) && IS_CIPHER_CCMP128(encryMode))
		return 2;
	else if (IS_CIPHER_WEP(encryMode))
		return 3;
	else
		return 4;
}

static void Get_80211_Standard(
	IN BSS_ENTRY * pBss,
	OUT UCHAR *SupportedStandards)
{
	NDIS_802_11_NETWORK_TYPE wireless_mode = NetworkTypeInUseSanity(pBss);
	int ret;

	if (wireless_mode == Ndis802_11FH ||
		wireless_mode == Ndis802_11DS) {
		ret = snprintf(SupportedStandards, 2, "%s", "b");
		if (os_snprintf_error(2, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM5) {
		ret = snprintf(SupportedStandards, 2, "%s", "a");
		if (os_snprintf_error(2, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM5_N) {
		ret = snprintf(SupportedStandards, 4, "%s", "a/n");
		if (os_snprintf_error(4, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM5_AC) {
		ret = snprintf(SupportedStandards, 7, "%s", "a/n/ac");
		if (os_snprintf_error(7, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM24) {
		ret = snprintf(SupportedStandards, 4, "%s", "b/g");
		if (os_snprintf_error(4, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM24_N) {
		ret = snprintf(SupportedStandards, 6, "%s", "b/g/n");
		if (os_snprintf_error(6, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM24_HE) {
		ret = snprintf(SupportedStandards, 9, "%s", "b/g/n/ax");
		if (os_snprintf_error(9, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			return;
		}
	} else if (wireless_mode == Ndis802_11OFDM5_HE) {
		ret = snprintf(SupportedStandards, 10, "%s", "a/n/ac/ax");
		if (os_snprintf_error(10, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			return;
		}
	} else {
		ret = snprintf(SupportedStandards, 8, "%s", "unknown");
		if (os_snprintf_error(8, ret)) {
			MTWF_PRINT("final_name snprintf error!\n");
			return;
		}
	}
}

static INT Get_Nbr_Diag_Result(
	IN RTMP_ADAPTER * pAd,
	IN NBR_DIAG_RESULT	* result)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT32 IfIdx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,
		pObj->ioctl_if, pObj->ioctl_if_type);
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
	BSS_ENTRY *pBss;
	UINT8 len;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wdev is null! IfIdx: %d.\n", IfIdx);
		return -EINVAL;
	}

	if (scan_in_run_state(pAd, wdev)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"\n\n\n SCANNING, CANNOT GET SCAN RESULT NOW!\n\n\n");
		/*#define	EBUSY		16*/
		/* Device or resource busy */
		return (-EBUSY);
	}

	if (ScanTab->BssNr == 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "No Scan result.\n");
		return -EINVAL;
	}

	if (result->index >= ScanTab->BssNr) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Index %d more than BssNr %d.\n", result->index, ScanTab->BssNr);
		return -EINVAL;
	}

	pBss = &ScanTab->BssEntry[result->index];
	result->Channel = pBss->Channel;
	NdisCopyMemory(result->ssid, pBss->Ssid, MAX_LEN_OF_SSID);
	NdisCopyMemory(result->bssid, pBss->Bssid, MAC_ADDR_LEN);

	if (wdev->if_dev) {
		len = (sizeof(wdev->if_dev->name) > 15) ? 15 : sizeof(wdev->if_dev->name);
		strncpy(result->radio_name, wdev->if_dev->name, len);
	}

	result->radio_mode = pBss->BssType;
	result->SignalStrength = pBss->Rssi;
	result->SecurityModeEnabled = Get_ScanSecurityMode(pBss->AKMMap);
	result->BeaconPeriod = pBss->BeaconPeriod;
	result->EncryptionMode = Get_ScanEncryMode(pBss->PairwiseCipher);
	result->DTIMPeriod = pBss->DtimPeriod;

	if (WMODE_CAP_2G(wdev->PhyMode))
		result->OperatingFrequencyBand = 0;  /* 2G */
	else if (WMODE_CAP_5G(wdev->PhyMode))
		result->OperatingFrequencyBand = 1;  /* 5G */
	else if (WMODE_CAP_6G(wdev->PhyMode))
		result->OperatingFrequencyBand = 2;  /* 6G */

	Get_80211_Standard(pBss, result->SupportedStandards);
	NdisCopyMemory(result->OperatingStandards, result->SupportedStandards,
		sizeof(result->SupportedStandards));
	result->OperatingChannelBandwidth = 0;  /* Not Support */
	result->Noise = 0; /* Not support */
	NdisCopyMemory(result->SupRate, pBss->SupRate, pBss->SupRateLen);
	NdisCopyMemory(result->ExtRate, pBss->ExtRate, pBss->ExtRateLen);
	return NDIS_STATUS_SUCCESS;
}

static INT Get_Radio_Statistics(
	IN RTMP_ADAPTER * pAd,
	OUT WIFI_RADIO_STATS * pStat)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand = BAND0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	ULONG txCount = 0, rxCount = 0, txBytes = 0, rxBytes = 0;
	INT ifIndex = pObj->ioctl_if;
	BSS_STRUCT *pMbss;
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error AP index \n");
		return -EINVAL;
	}

	pMbss = &pAd->ApCfg.MBSSID[ifIndex];
	if (wdev)
		ucBand = HcGetBandByWdev(wdev);

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ra offload=%d\n", cap->fgRateAdaptFWOffload);
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	if (cap->fgRateAdaptFWOffload == TRUE) {
		EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;

		os_zero_mem(&rTxStatResult, sizeof(EXT_EVENT_TX_STATISTIC_RESULT_T));
		MtCmdGetTxStatistic(pAd, GET_TX_STAT_TOTAL_TX_CNT, ucBand, 0, &rTxStatResult);
		pAd->WlanCounters[ucBand].TransmittedFragmentCount.u.LowPart += (rTxStatResult.u4TotalTxCount -
				rTxStatResult.u4TotalTxFailCount);
		pAd->WlanCounters[ucBand].FailedCount.u.LowPart += rTxStatResult.u4TotalTxFailCount;
		pAd->WlanCounters[ucBand].CurrentBwTxCount.u.LowPart += rTxStatResult.u4CurrBwTxCnt;
		pAd->WlanCounters[ucBand].OtherBwTxCount.u.LowPart += rTxStatResult.u4OtherBwTxCnt;
	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

#ifdef CONFIG_ATE
	if (ATE_ON(pAd)) {
		struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

		txCount = TESTMODE_GET_PARAM(pAd, ucBand, ATE_TXDONE_CNT);
		rxCount = ATECtrl->rx_stat.RxTotalCnt[ucBand];
	} else
#endif	/* CONFIG_ATE */
	{
		txCount = pAd->WlanCounters[ucBand].TransmittedFragmentCount.u.LowPart;
			rxCount = pAd->WlanCounters[ucBand].ReceivedFragmentCount.QuadPart;
	}

	txBytes = pAd->WlanCounters[ucBand].TxTotByteCount.QuadPart;
	rxBytes = pAd->WlanCounters[ucBand].RxTotByteCount.QuadPart;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"txBytes:%ld, rxBytes:%ld, TxCount raw:%lld, RxCount raw:%lld\n",
		txBytes, rxBytes, pAd->WlanCounters[ucBand].TxTotByteCount.QuadPart,
		pAd->WlanCounters[ucBand].RxTotByteCount.QuadPart);

#ifdef CONFIG_QA
	if (ATE_ON(pAd)) {
		pStat->DiscardPacketsReceived =	pAd->WlanCounters[0].RxFifoFullCount.u.LowPart;
		pStat->ErrorsReceived = pAd->WlanCounters[0].FCSErrorCount.u.LowPart;
		} else
#endif /* CONFIG_QA */

	{
		pStat->DiscardPacketsReceived = pAd->Counters8023.RxNoBuffer;
		pStat->ErrorsReceived = pAd->WlanCounters[ucBand].FCSErrorCount.u.LowPart;
	}

	pStat->PacketsSent = txCount;
	pStat->PacketsReceived = rxCount;
	pStat->BytesSent = txBytes;
	pStat->BytesReceived = rxBytes;
	pStat->MulticastPacketsSent = pAd->WlanCounters[ucBand].mcPktsTx.QuadPart;
	pStat->MulticastPacketsReceived = pAd->WlanCounters[ucBand].mcPktsRx.QuadPart;
	pStat->BroadcastPacketsSent = pAd->WlanCounters[ucBand].bcPktsTx.QuadPart;
	pStat->BroadcastPacketsReceived = pAd->WlanCounters[ucBand].bcPktsRx.QuadPart;
	pStat->UnicastPacketsSent = pStat->PacketsSent - pStat->MulticastPacketsSent
									- pStat->BroadcastPacketsSent;
	pStat->UnicastPacketsReceived = pStat->PacketsReceived - pStat->MulticastPacketsReceived
									- pStat->BroadcastPacketsReceived;

	pStat->TotalChannelChangeCount =
		(ctrl->rdev[ucBand].pRadioCtrl->TotalChannelChangeCount +
		pAd->ApBootACSChannelChangePerBandCount[ucBand]);
	pStat->ManualChannelChangeCount =
		ctrl->rdev[ucBand].pRadioCtrl->ManualChannelChangeCount;
	pStat->AutoStartupChannelChangeCount =
		pAd->ApBootACSChannelChangePerBandCount[ucBand];
	pStat->AutoUserChannelChangeCount =
		ctrl->rdev[ucBand].pRadioCtrl->ForceACSChannelChangeCount;
	pStat->AutoRefreshChannelChangeCount =
		ctrl->rdev[ucBand].pRadioCtrl->RefreshACSChannelChangeCount;
	pStat->AutoDynamicChannelChangeCount = 0; /* no support */
	pStat->AutoDFSChannelChangeCount =
		ctrl->rdev[ucBand].pRadioCtrl->DFSTriggeredChannelChangeCount;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"PacketsSent=%ld, PacketsReceived=%ld\n", pStat->PacketsSent, pStat->PacketsReceived);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"TransmittedByteCount=%ld, ReceivedByteCount=%ld\n",
		pStat->BytesSent, pStat->BytesReceived);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"ErrorsReceived=%ld, DiscardPacketsReceived=%ld\n",
		pStat->ErrorsReceived, pStat->DiscardPacketsReceived);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"ErrorsSent=%ld, DiscardPacketsSent=%ld\n",
		pStat->ErrorsSent, pStat->DiscardPacketsSent);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"ucPktsTx=%ld, ucPktsRx=%ld\n", pStat->UnicastPacketsSent, pStat->UnicastPacketsReceived);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"mcPktsTx=%ld, mcPktsRx=%ld\n", pStat->MulticastPacketsSent, pStat->MulticastPacketsReceived);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"bcPktsTx=%ld, bcPktsRx=%ld\n", pStat->BroadcastPacketsSent, pStat->BroadcastPacketsReceived);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"TotalChannelChangeCount=%d, ManualChannelChangeCount=%d\n",
		pStat->TotalChannelChangeCount, pStat->ManualChannelChangeCount);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"AutoStartupChannelChangeCount=%d, AutoUserChannelChangeCount=%d\n",
		pStat->AutoStartupChannelChangeCount, pStat->AutoUserChannelChangeCount);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"AutoRefreshChannelChangeCount=%d, AutoDynamicChannelChangeCount=%d\n",
		pStat->AutoRefreshChannelChangeCount, pStat->AutoDynamicChannelChangeCount);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"AutoDFSChannelChangeCount=%d\n", pStat->AutoDFSChannelChangeCount);

	return NDIS_STATUS_SUCCESS;
}

static INT Get_BSS_Statistics(
	IN RTMP_ADAPTER * pAd,
	OUT WIFI_BSS_STATS * pStat)
{
	BSS_STRUCT *pMbss;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error AP index\n");
		return -EINVAL;
	}

	pMbss = &pAd->ApCfg.MBSSID[ifIndex];

	pStat->PacketsSent =  pMbss->TxCount;
	pStat->PacketsReceived =  pMbss->RxCount;
	pStat->BytesSent = pMbss->TransmittedByteCount;
	pStat->BytesReceived = pMbss->ReceivedByteCount;
	pStat->ErrorsReceived =  pMbss->RxErrorCount;
	pStat->DiscardPacketsReceived =  pMbss->RxDropCount;
	pStat->ErrorsSent =  pMbss->TxErrorCount;
	pStat->DiscardPacketsSent =  pMbss->TxDropCount;
	pStat->UnicastPacketsSent =  pMbss->ucPktsTx;
	pStat->UnicastPacketsReceived =  pMbss->ucPktsRx;
	pStat->MulticastPacketsSent =  pMbss->mcPktsTx;
	pStat->MulticastPacketsReceived =  pMbss->mcPktsRx;
	pStat->BroadcastPacketsSent = pMbss->bcPktsTx;
	pStat->BroadcastPacketsReceived = pMbss->bcPktsRx;
	return NDIS_STATUS_SUCCESS;
}

#ifdef WSC_AP_SUPPORT
PWSC_CTRL Get_WscControl(
	IN RTMP_ADAPTER * pAd)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = 0;

	if (pObj)
		apidx = pObj->ioctl_if;
	else
		return NULL;

#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI) {
		return &pAd->StaCfg[apidx].wdev.WscControl;
	} else
#endif /* APCLI_SUPPORT */
#ifdef P2P_SUPPORT
	if (pObj->ioctl_if_type == INT_P2P) {
		if (P2P_CLI_ON(pAd))
			return &pAd->StaCfg[apidx].wdev.WscControl;
		else
			return &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
	} else
#endif /* P2P_SUPPORT */
		return &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
}
#endif
#endif

INT RTMPAPQueryInformation(
	IN RTMP_ADAPTER *pAd,
	IN OUT RTMP_IOCTL_INPUT_STRUCT *rq,
	IN INT cmd)
{
	RTMP_IOCTL_INPUT_STRUCT	 *wrq = (RTMP_IOCTL_INPUT_STRUCT *) rq;
	INT	Status = NDIS_STATUS_SUCCESS;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_STRING driverVersion[16];
	UCHAR apidx = pObj->ioctl_if;
#ifdef CONFIG_MAP_SUPPORT
	PUCHAR pStaMacAddr = NULL;
#endif
#ifdef WSC_AP_SUPPORT
	UINT WscPinCode = 0;
	PWSC_PROFILE pProfile;
	PWSC_CTRL pWscControl;
#endif /* WSC_AP_SUPPORT */
#if defined(SNMP_SUPPORT) || defined(VENDOR_FEATURE6_SUPPORT) || defined(TR181_SUPPORT)
	ULONG ulInfo = 0;
#endif /*  defined(SNMP_SUPPORT) || defined(VENDOR_FEATURE6_SUPPORT) */
#ifdef SNMP_SUPPORT
	DefaultKeyIdxValue *pKeyIdxValue;
	INT valueLen;
	TX_RTY_CFG_STRUC tx_rty_cfg;
	ULONG ShortRetryLimit, LongRetryLimit;
	UCHAR snmp_tmp[64];
#endif /* SNMP_SUPPORT */
#if (defined(APCLI_SUPPORT) || defined(WH_EZ_SETUP))
	NDIS_802_11_SSID                    Ssid;
#endif
	UINT ifIndex;
#ifdef APCLI_SUPPORT
	BOOLEAN apcliEn = FALSE;
	PSTA_ADMIN_CONFIG pApCliEntry = NULL;
#ifdef WPA_SUPPLICANT_SUPPORT
	INT i, Padding = 0;
	ULONG						BssBufSize;
	PUCHAR                              pBuf = NULL, pPtr = NULL;
	NDIS_802_11_BSSID_LIST_EX           *pBssidList = NULL;
	USHORT                              BssLen = 0;
	PNDIS_WLAN_BSSID_EX                 pBss;
	MAC_TABLE_ENTRY				*pMacEntry = (MAC_TABLE_ENTRY *)NULL;
	STA_TR_ENTRY *tr_entry;
	UINT                                we_version_compiled;
#endif/*WPA_SUPPLICANT_SUPPORT*/
#endif/*APCLI_SUPPORT*/
	NDIS_802_11_STATISTICS	 *pStatistics;
#ifdef P2P_SUPPORT
	/*RT_P2P_UI_TABLE UI_table;*/
	PRT_P2P_UI_TABLE pUI_table;
	PRT_P2P_TABLE			pP2pTable;
	PRT_P2P_CLIENT_ENTRY   pPAdCli, pUICli;
	PRT_P2P_CONFIG	pP2PCtrl; /* = &pAd->P2pCfg; */
	UCHAR tmp[24];
	UCHAR i;
#endif /* P2P_SUPPORT */
#ifdef DOT1X_SUPPORT
	INT IEEE8021X = 0;
#endif /* DOT1X_SUPPORT */
	NDIS_802_11_AUTHENTICATION_MODE AuthMode = Ndis802_11AuthModeMax;
	struct wifi_dev *wdev = NULL;
	BSS_TABLE *ScanTab = NULL;

	/* For all ioctl to this function, we assume that's query for AP/APCLI/GO device */
	ifIndex = pObj->ioctl_if;
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		if (ifIndex >= pAd->ApCfg.BssidNum)
			return -EFAULT;

		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		ScanTab = get_scan_tab_by_wdev(pAd, wdev);
	}

	switch (cmd) {
#ifdef NF_SUPPORT_V2
	case OID_802_11_GET_NF:
		{
			UCHAR band_idx = 0;
			if (wdev) {
				band_idx = HcGetBandByWdev(wdev);
				wrq->u.data.length = sizeof(INT32);
				Status = copy_to_user(wrq->u.data.pointer, &pAd->Avg_NF[band_idx], wrq->u.data.length);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Band[%d] Current noise = %d\n", band_idx, pAd->Avg_NF[band_idx]);
			} else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid wdev for cmd %d\n", cmd);
 			break;
		}
#endif
#ifdef ACS_CTCC_SUPPORT
	case OID_802_11_GET_ACS_CHANNEL_SCORE:
		{
			INT channel_idx = 0;
			struct acs_channel_score *ch_score;
			UCHAR band_idx = 0;
			AUTO_CH_CTRL *auto_ch_ctrl = NULL;
			PCHANNELINFO channel_info = NULL;

			if (wdev) {
				band_idx = HcGetBandByWdev(wdev);
				auto_ch_ctrl = HcGetAutoChCtrlbyBandIdx(pAd, band_idx);
				channel_info = auto_ch_ctrl->pChannelInfo;
				os_alloc_mem(pAd, (UCHAR **)&ch_score, sizeof(struct acs_channel_score));
				if (ch_score == NULL) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "allocate memory is failed\n");
					Status = RTMP_IO_EFAULT;
					break;
				}
				ch_score->acs_alg = pAd->ApCfg.AutoChannelAlg[band_idx];
				for (channel_idx = 0; channel_idx < channel_info->channel_list_num; channel_idx++) {
					ch_score->acs_channel_score[channel_idx].score = channel_info->channel_score[channel_idx].score;
					ch_score->acs_channel_score[channel_idx].channel = channel_info->channel_score[channel_idx].channel;
				}
				wrq->u.data.length = sizeof(struct acs_channel_score);
				Status = copy_to_user(wrq->u.data.pointer, ch_score, wrq->u.data.length);
				if (Status != 0) {
					Status = RTMP_IO_EFAULT;
				    MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_GET_ACS_CHANNEL_SCORE is failed\n" );
				}
				os_free_mem(ch_score);
			} else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid wdev for cmd %d\n", cmd);
 			break;
		}
#endif
#ifdef AIR_MONITOR
#ifdef WAPP_SUPPORT
	case OID_GET_AIR_MONITOR_RESULT:
		{
			MNT_STA_ENTRY *pTmpEntry = NULL;
			UINT32 pEntryLen = 0;
			UCHAR i;
			UCHAR band_idx = 0;
			wapp_mnt_info *mnt_info = NULL;
			if (wdev) {
				band_idx = HcGetBandByWdev(wdev);
				os_alloc_mem(NULL, (UCHAR **)&mnt_info, MAX_NUM_OF_MONITOR_STA * sizeof(wapp_mnt_info));
				if (mnt_info == NULL) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Query::OID_GET_AIR_MONITOR_RESULT alloc memory failed!\n");
					return -EFAULT;
				}
				NdisZeroMemory(mnt_info, MAX_NUM_OF_MONITOR_STA * sizeof(wapp_mnt_info));

				for (i = 0; i < MAX_NUM_OF_MONITOR_STA; i++) {
					pTmpEntry = &pAd->MntTable[band_idx][i];
					if (pTmpEntry->Count != 0 && pTmpEntry->Count < pAd->MntMaxPktCnt[band_idx]) {
						NdisCopyMemory(mnt_info[pEntryLen].sta_addr, pTmpEntry->addr, MAC_ADDR_LEN);
						mnt_info[pEntryLen].rssi = (char)(pTmpEntry->RssiTotal/(LONG)pTmpEntry->Count);
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"STA ["MACSTR"] packet received %ld and total rssi %ld avg rssi %d\n",
							MAC2STR(pTmpEntry->addr), pTmpEntry->Count, pTmpEntry->RssiTotal, mnt_info[pEntryLen].rssi);
						pEntryLen += 1;
					}
				}

				Status = copy_to_user(wrq->u.data.pointer, mnt_info, MAX_NUM_OF_MONITOR_STA * sizeof(wapp_mnt_info));
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Query::OID_GET_AIR_MONITOR_RESULT success!\n");
				os_free_mem(mnt_info);
			} else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_GET_AIR_MONITOR_RESULT get null wdev!\n");
		}
		break;
#endif
#endif
#ifdef DOT1X_SUPPORT

	case OID_802_11_SET_IEEE8021X:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_SET_IEEE8021X\n");
		wrq->u.data.length = sizeof(INT);

		if (IS_IEEE8021X_Entry(&pAd->ApCfg.MBSSID[ifIndex].wdev))
			IEEE8021X = 1;
		else
			IEEE8021X = 0;

		Status = copy_to_user(wrq->u.data.pointer, &IEEE8021X, wrq->u.data.length);
		break;
#endif /* DOT1X_SUPPORT */

	case OID_802_11_AUTHENTICATION_MODE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_AUTHENTICATION_MODE\n");
		wrq->u.data.length = sizeof(NDIS_802_11_AUTHENTICATION_MODE);
		AuthMode = SecAuthModeNewToOld(pAd->ApCfg.MBSSID[ifIndex].wdev.SecConfig.AKMMap);
		Status = copy_to_user(wrq->u.data.pointer, &AuthMode, wrq->u.data.length);
		break;

	case OID_GET_CPU_TEMPERATURE:{
		UCHAR band = 0;
		if (wdev) {
			band = HcGetBandByWdev(wdev);
			RTMP_GET_TEMPERATURE(pAd, band, &pAd->temperature);

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "CurrentTemperature = %d\n", pAd->temperature);
			wrq->u.data.length = sizeof(INT);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->temperature, wrq->u.data.length);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid wdev for cmd %d\n", cmd);
		break;
	}

	case OID_MTK_CHIP_ID:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query ChipID = %x\n", pAd->ChipID);
		wrq->u.data.length = sizeof(UINT32);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->ChipID, sizeof(UINT32));
		break;

	case OID_MTK_DRVER_VERSION:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Driver Version = %s\n", AP_DRIVER_VERSION);
		wrq->u.data.length = strlen(AP_DRIVER_VERSION) + 1;
		Status = copy_to_user(wrq->u.data.pointer, AP_DRIVER_VERSION, wrq->u.data.length);
		break;

	case OID_MAX_NUM_OF_STA: {
		UINT16 MaxStaNum = HcGetMaxStaNum(pAd);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Driver MaxStaNum = %d\n", MaxStaNum);
		wrq->u.data.length = sizeof(UINT16);
		Status = copy_to_user(wrq->u.data.pointer, &MaxStaNum, wrq->u.data.length);
		break;
	}

#ifdef APCLI_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT

	case RT_OID_NEW_DRIVER: {
		UCHAR enabled = 1;

		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer, &enabled, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query apcli::RT_OID_NEW_DRIVER (=%d)\n", enabled);
		break;
	}

	case RT_OID_WE_VERSION_COMPILED:
		wrq->u.data.length = sizeof(UINT);
		we_version_compiled = RtmpOsWirelessExtVerGet();
		Status = copy_to_user(wrq->u.data.pointer, &we_version_compiled, wrq->u.data.length);
		break;

	case OID_802_11_BSSID_LIST:
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		if (ifIndex >= MAX_MULTI_STA) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error station index\n");
			return FALSE;
		}

		pApCliEntry = &pAd->StaCfg[ifIndex];
		apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;

		if (!apcliEn)
			return FALSE;

		pMacEntry = &pAd->MacTab.Content[pAd->StaCfg[ifIndex].MacTabWCID];
		tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) {
			/*
			 * Still scanning, indicate the caller should try again.
			 */
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_BSSID_LIST (Still scanning)\n");
			return -EAGAIN;
		}

		if ((pAd->StaCfg[ifIndex].wpa_supplicant_info.WpaSupplicantUP & 0x7F) == WPA_SUPPLICANT_ENABLE)
			pAd->StaCfg[ifIndex].wpa_supplicant_info.WpaSupplicantScanCount = 0;

		BssBufSize = sizeof(ULONG);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_BSSID_LIST (%d BSS returned)\n",
			 ScanTab->BssNr);

		for (i = 0; i < ScanTab->BssNr; i++)
			BssBufSize += (sizeof(NDIS_WLAN_BSSID_EX) - 1 + sizeof(NDIS_802_11_FIXED_IEs) + ScanTab->BssEntry[i].VarIELen +
					   Padding);

		BssBufSize += 256;
		os_alloc_mem(pAd, (UCHAR **)&pBuf, BssBufSize);

		if (pBuf == NULL) {
			Status = -ENOMEM;
			break;
		}

		NdisZeroMemory(pBuf, BssBufSize);
		pBssidList = (PNDIS_802_11_BSSID_LIST_EX) pBuf;
		pBssidList->NumberOfItems = ScanTab->BssNr;
		BssLen = 4; /* Consist of NumberOfItems */
		pPtr = (PUCHAR) &pBssidList->Bssid[0];

		for (i = 0; i < ScanTab->BssNr; i++) {
			pBss = (PNDIS_WLAN_BSSID_EX) pPtr;
			NdisMoveMemory(&pBss->MacAddress, &ScanTab->BssEntry[i].Bssid, MAC_ADDR_LEN);

			if ((ScanTab->BssEntry[i].Hidden == 1)) {
				/*
				 We must return this SSID during 4way handshaking, otherwise Aegis will failed to parse WPA infomation
				 and then failed to send EAPOl farame.
				*/
				if ((pAd->StaCfg[ifIndex].wdev.AuthMode >= Ndis802_11AuthModeWPA) &&
					(tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
					pBss->Ssid.SsidLength = ScanTab->BssEntry[i].SsidLen;
					NdisMoveMemory(pBss->Ssid.Ssid, ScanTab->BssEntry[i].Ssid, ScanTab->BssEntry[i].SsidLen);
				} else
					pBss->Ssid.SsidLength = 0;
			} else {
				pBss->Ssid.SsidLength = pAd->ScanTab.BssEntry[i].SsidLen;
				NdisMoveMemory(pBss->Ssid.Ssid, ScanTab->BssEntry[i].Ssid, ScanTab->BssEntry[i].SsidLen);
			}

			pBss->Privacy = ScanTab->BssEntry[i].Privacy;
			pBss->Rssi = ScanTab->BssEntry[i].Rssi - pAd->BbpRssiToDbmDelta;
			pBss->NetworkTypeInUse = NetworkTypeInUseSanity(&pAd->ScanTab.BssEntry[i]);
			pBss->Configuration.Length = sizeof(NDIS_802_11_CONFIGURATION);
			pBss->Configuration.BeaconPeriod = ScanTab->BssEntry[i].BeaconPeriod;
			pBss->Configuration.ATIMWindow = ScanTab->BssEntry[i].AtimWin;
			MAP_CHANNEL_ID_TO_KHZ(ScanTab->BssEntry[i].Channel, pBss->Configuration.DSConfig);

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
				NdisMoveMemory(pBss->IEs + sizeof(NDIS_802_11_FIXED_IEs), ScanTab->BssEntry[i].VarIEs,
							   ScanTab->BssEntry[i].VarIELen);
				pPtr += ScanTab->BssEntry[i].VarIELen;
			}

			pBss->Length = (ULONG)(sizeof(NDIS_WLAN_BSSID_EX) - 1 + sizeof(NDIS_802_11_FIXED_IEs) +
								   ScanTab->BssEntry[i].VarIELen + Padding);
#if WIRELESS_EXT < 17

			if ((BssLen + pBss->Length) < wrq->u.data.length)
				BssLen += pBss->Length;
			else {
				pBssidList->NumberOfItems = i;
				break;
			}

#else
			BssLen += pBss->Length;
#endif
		}

#if WIRELESS_EXT < 17
		wrq->u.data.length = BssLen;
#else

		if (BssLen > wrq->u.data.length) {
			os_free_mem(pBssidList);
			return -E2BIG;
		}

		wrq->u.data.length = BssLen;
#endif
		Status = copy_to_user(wrq->u.data.pointer, pBssidList, BssLen);
		os_free_mem(pBssidList);
		break;

	case OID_802_3_CURRENT_ADDRESS:
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;

		if (ifIndex >= MAX_MULTI_STA) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error station index\n");
			return FALSE;
		}

		pApCliEntry =  &pAd->StaCfg[ifIndex];
		apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;

		if (!apcliEn)
			return FALSE;

		pMacEntry = &pAd->MacTab.Content[pAd->StaCfg[ifIndex].MacTabWCID];
		wrq->u.data.length = MAC_ADDR_LEN;
		Status = copy_to_user(wrq->u.data.pointer, pApCliEntry->wdev.if_addr, wrq->u.data.length);
		break;
#endif/*WPA_SUPPLICANT_SUPPORT*/

	case OID_802_11_BSSID:
		if ((pObj->ioctl_if_type != INT_APCLI)
#ifdef VENDOR_FEATURE6_SUPPORT
			&& (pObj->ioctl_if_type != INT_MAIN)
#endif /* VENDOR_FEATURE6_SUPPORT */
		   )
			return FALSE;

		if (ifIndex >= MAX_MULTI_STA) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error station index\n");
			return FALSE;
		}

		if (pObj->ioctl_if_type == INT_APCLI) {
			pApCliEntry = &pAd->StaCfg[ifIndex];
			apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;

			if (!apcliEn)
				return FALSE;

			Status = copy_to_user(wrq->u.data.pointer, pApCliEntry->MlmeAux.Bssid, sizeof(NDIS_802_11_MAC_ADDRESS));
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"IOCTL::SIOCGIWAP(="MACSTR")\n", MAC2STR(pApCliEntry->MlmeAux.Bssid));
		}

#ifdef VENDOR_FEATURE6_SUPPORT
		else if (pObj->ioctl_if_type == INT_MAIN) {
			if (wdev) {
				wrq->u.data.length = MAC_ADDR_LEN;
				Status = copy_to_user(wrq->u.data.pointer, &wdev->bssid[0], wrq->u.data.length);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Query::OID_802_11_BSSID ("MACSTR")\n", MAC2STR(wdev->bssid));
			}
		}

#endif /* VENDOR_FEATURE6_SUPPORT */
		else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_BSSID(=EMPTY)\n");
			Status = -ENOTCONN;
		}

		break;
#endif/*APCLI_SUPPORT*/
#if (defined(APCLI_SUPPORT) || defined(WH_EZ_SETUP))

	case OID_802_11_SSID:
		NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
#ifdef APCLI_SUPPORT
		if (ifIndex >= MAX_MULTI_STA) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error station index\n");
			return FALSE;
		}

		if (pObj->ioctl_if_type == INT_APCLI) {
			pApCliEntry =  &pAd->StaCfg[ifIndex];
			apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;

			if (!apcliEn)
				return FALSE;

			Ssid.SsidLength = pApCliEntry->CfgSsidLen;
			NdisMoveMemory(Ssid.Ssid, pApCliEntry->CfgSsid, Ssid.SsidLength);
		}

#endif /* APCLI_SUPPORT */
		wrq->u.data.length = sizeof(NDIS_802_11_SSID);
		Status = copy_to_user(wrq->u.data.pointer, &Ssid, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query OID_802_11_SSID (Len=%d, ssid=%s)\n", Ssid.SsidLength,
				 Ssid.Ssid);
		break;
	case OID_802_11_GET_SSID_BSSID: {
		struct wifi_dev *wdev = NULL;
		BSS_TABLE *ScanTab = NULL;
		NDIS_802_11_GET_SSID_BSSID scan_result = {0};
		PSSID_BSSID scan_entry = NULL;
		UINT i = 0;

		if (pObj->ioctl_if_type == INT_APCLI) {
			wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
			if (wdev) {
				ScanTab = get_scan_tab_by_wdev(pAd, wdev);

				for (i = 0; i < ScanTab->BssNr  && ScanTab->BssNr < MAX_LEN_OF_BSS_TABLE; i++) {
					scan_entry = &scan_result.entry[i];
					scan_entry->ssid_len = ScanTab->BssEntry[i].SsidLen;
					NdisMoveMemory(scan_entry->ssid, ScanTab->BssEntry[i].Ssid, ScanTab->BssEntry[i].SsidLen);
					NdisMoveMemory(scan_entry->bssid, ScanTab->BssEntry[i].Bssid, MAC_ADDR_LEN);
					scan_result.entry_num++;
					if (scan_result.entry_num >= MAX_SSID_BSSID_ENTRY_NUM)
						break;
				}
			} else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid wdev for cmd %d\n", cmd);
		}

		wrq->u.data.length = sizeof(NDIS_802_11_GET_SSID_BSSID);
		Status = copy_to_user(wrq->u.data.pointer, &scan_result, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Query OID_802_11_GET_SSID_BSSID (Len=%d)\n", wrq->u.data.length);
	}
		break;
	case OID_802_11_RSSI: {
		CHAR rssi = -127;
		MAC_TABLE_ENTRY *entry = NULL;

		if (ifIndex >= MAX_MULTI_STA) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error station index\n");
			return FALSE;
		}

		if (pObj->ioctl_if_type == INT_APCLI) {
			pApCliEntry = &pAd->StaCfg[ifIndex];
			entry = GetAssociatedAPByWdev(pAd, &pApCliEntry->wdev);

			if (!pApCliEntry->ApcliInfStat.Enable)
				return FALSE;

			rssi = entry->RssiSample.AvgRssi[0];
		}

		wrq->u.data.length = sizeof(rssi);
		Status = copy_to_user(wrq->u.data.pointer, &rssi, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Query OID_802_11_RSSI (Len=%d, Rssi=%d)\n", wrq->u.data.length, rssi);
	}
		break;
#endif /* (defined(APCLI_SUPPORT) || defined(WH_EZ_SETUP)) */
#ifdef P2P_SUPPORT

	case OID_802_11_P2P_Connected_MAC:
		wrq->u.data.length = MAC_ADDR_LEN;
		Status = copy_to_user(wrq->u.data.pointer, &pAd->P2pCfg.ConnectingMAC, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_P2P_Connected_MAC\n");
		break;

	case OID_802_11_P2P_MODE:
		wrq->u.data.length = sizeof(char);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->P2pCfg.Rule, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_P2P_MODE (Len=%d, Rule=%s)\n", sizeof(char),
				 pAd->P2pCfg.Rule);
		break;

	case OID_802_11_P2P_DEVICE_NAME:
		wrq->u.data.length = pAd->P2pCfg.DeviceNameLen;
		Status = copy_to_user(wrq->u.data.pointer, pAd->P2pCfg.DeviceName, pAd->P2pCfg.DeviceNameLen);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_P2P_DEVICE_NAME (Len=%d, DeviceName=%s)\n",
				 pAd->P2pCfg.DeviceNameLen, pAd->P2pCfg.DeviceName);
		break;

	case OID_802_11_P2P_LISTEN_CHANNEL:
		wrq->u.data.length = sizeof(char);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->P2pCfg.ListenChannel, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_P2P_LISTEN_CHANNEL (Len=%d, Listen_Ch=%d)\n",
				 sizeof(char), pAd->P2pCfg.ListenChannel);
		break;

	case OID_802_11_P2P_OPERATION_CHANNEL:
		wrq->u.data.length = sizeof(char);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->P2pCfg.GroupChannel, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_P2P_OPERATION_CHANNEL (Len=%d, Op_Ch=%d)\n",
				 sizeof(char), pAd->P2pCfg.GroupOpChannel);
		break;

	case OID_802_11_P2P_MAC_ADDR:
		wrq->u.data.length = 6;
		Status = copy_to_user(wrq->u.data.pointer, pAd->P2pCfg.Bssid, wrq->u.data.length);
		/*MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, "Query::OID_802_11_P2P_MAC_ADDR (Len=%d, Rule=%s)\n", sizeof(char),pAd->P2pCfg.GroupOpChannel); */
		break;

	case OID_802_11_P2P_CTRL_STATUS:
		wrq->u.data.length = 24;
		pP2PCtrl = &pAd->P2pCfg;
		NdisZeroMemory(tmp, 24);
		snprintf(tmp, sizeof(tmp), "%s", decodeCtrlState(pP2PCtrl->CtrlCurrentState));
		Status = copy_to_user(wrq->u.data.pointer, tmp, 24);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_P2P_MODE (Len=%d, DeviceName=%s)\n",
				 pAd->P2pCfg.DeviceNameLen, pAd->P2pCfg.DeviceName);
		break;

	case OID_802_11_P2P_DISC_STATUS:
		wrq->u.data.length = 24;
		pP2PCtrl = &pAd->P2pCfg;
		NdisZeroMemory(tmp, 24);
		snprintf(tmp, sizeof(tmp), "%s", decodeDiscoveryState(pP2PCtrl->DiscCurrentState));
		Status = copy_to_user(wrq->u.data.pointer, tmp, 24);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_P2P_MODE (Len=%d, DeviceName=%s)\n",
				 pAd->P2pCfg.DeviceNameLen, pAd->P2pCfg.DeviceName);
		break;

	case OID_802_11_P2P_GOFORM_STATUS:
		wrq->u.data.length = 24;
		pP2PCtrl = &pAd->P2pCfg;
		NdisZeroMemory(tmp, 24);
		snprintf(tmp, sizeof(tmp), "%s", decodeGroupFormationState(pP2PCtrl->GoFormCurrentState));
		Status = copy_to_user(wrq->u.data.pointer, tmp, 24);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_P2P_MODE (Len=%d, DeviceName=%s)\n",
				 pAd->P2pCfg.DeviceNameLen, pAd->P2pCfg.DeviceName);
		break;

	case OID_802_11_P2P_SCAN_LIST:
		os_alloc_mem(NULL, (UCHAR **)&pUI_table, sizeof(RT_P2P_UI_TABLE));
		pP2pTable = &pAd->P2pTable;
		/*NdisZeroMemory(&UI_table, sizeof(UI_table));*/
		/*pUI_table = &UI_table;*/
		NdisZeroMemory(pUI_table, sizeof(RT_P2P_UI_TABLE));
		pUI_table->ClientNumber = pAd->P2pTable.ClientNumber;

		for (i = 0; i < pAd->P2pTable.ClientNumber; i++) {
			pPAdCli = &pP2pTable->Client[i];
			pUICli = &pUI_table->Client[i];
			NdisMoveMemory(pUICli, pPAdCli, sizeof(RT_P2P_CLIENT_ENTRY));
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Query::OID_802_11_P2P_SCAN_LIST\n");
		Status = copy_to_user(wrq->u.data.pointer, pUI_table, sizeof(RT_P2P_UI_TABLE));
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_P2P_SCAN_LIST (Len=%d, Rule=%s)\n",
				 sizeof(char), pAd->P2pCfg.GroupOpChannel);
		os_free_mem(pUI_table);
		break;

	case OID_P2P_WSC_PIN_CODE:
		wrq->u.data.length = sizeof(UINT);
		WscPinCode = pAd->StaCfg[apidx].WscControl.WscEnrolleePinCode;

		if (copy_to_user(wrq->u.data.pointer, &WscPinCode, wrq->u.data.length))
			Status = -EFAULT;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_WSC_PIN_CODE (=%d)\n", WscPinCode);
		break;
#endif /* P2P_SUPPORT */
#ifdef ANDLINK_FEATURE_SUPPORT
		case OID_ANDLINK_POLL:
			if (wdev && HcGetBandByWdev(wdev) < DBDC_BAND_NUM &&
				TRUE == pAd->CommonCfg.andlink_enable[HcGetBandByWdev(wdev)]) {
				Status = andlink_get_pollinfos(pAd, apidx);
			}else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Query OID_ANDLINK_POLL: andlink_enable is disable.\n");

			}
			break;
		case OID_ANDLINK_STAINFO:
			if ((pObj->ioctl_if_type == INT_MAIN || pObj->ioctl_if_type == INT_MBSSID) &&
				wdev && HcGetBandByWdev(wdev) < DBDC_BAND_NUM &&
				TRUE == pAd->CommonCfg.andlink_enable[HcGetBandByWdev(wdev)]) {
#ifdef ANDLINK_V4_0
				Status = andlink_get_stainfos_v4_0(pAd, apidx, wrq);
#else
				Status = andlink_get_stainfos(pAd, apidx, wrq);
#endif
			}else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Query OID_ANDLINK_STAINFO: andlink_enable is disable.\n");

			}
			break;
		case OID_ANDLINK_HOSTNAME_IP:
			if (wdev && HcGetBandByWdev(wdev) < DBDC_BAND_NUM &&
				TRUE == pAd->CommonCfg.andlink_enable[HcGetBandByWdev(wdev)]) {
				Status = andlink_get_sta_hostname_ip(pAd, apidx, wrq);
			}else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Query OID_ANDLINK_HOSTNAME_IP: andlink_enable is disable.\n");
			}
			break;
#ifdef ANDLINK_V4_0
		case OID_ANDLINK_UPLINK:
			if (pObj->ioctl_if_type == INT_APCLI) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Query OID_ANDLINK_UPLINK: will get uplink result.\n");
				Status = andlink_get_uplink_info_v4_0(pAd, apidx, wrq);
			}else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Query OID_ANDLINK_UPLINK:if_type:0x%x;andlink_en is disable.\n",
					pObj->ioctl_if_type);

			}
			break;
		case OID_ANDLINK_NRINFO:
			if ((pObj->ioctl_if_type == INT_MAIN || pObj->ioctl_if_type == INT_MBSSID)
				&& wdev && HcGetBandByWdev(wdev) < DBDC_BAND_NUM
				&& TRUE == pAd->CommonCfg.andlink_enable[HcGetBandByWdev(wdev)]) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Query OID_ANDLINK_NRINFO: will get scan result.\n");
				Status = andlink_get_scan_result_info_v4_0(pAd, apidx, wrq);
			}else {
				MTWF_DBG(pAd,DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Query OID_ANDLINK_NRINFO:if_type:%d;andlink_en is disable.\n",
					pObj->ioctl_if_type);

			}
			break;
		case OID_ANDLINK_BSSINFO:
			if ((pObj->ioctl_if_type == INT_MAIN || pObj->ioctl_if_type == INT_MBSSID)
				&& wdev && HcGetBandByWdev(wdev) < DBDC_BAND_NUM
				&& TRUE == pAd->CommonCfg.andlink_enable[HcGetBandByWdev(wdev)]) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Query OID_ANDLINK_BSSINFO: will get scan result.\n");
				Status = andlink_get_bss_info_v4_0(pAd, apidx, wrq);
			}else {
				MTWF_DBG(pAd,DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Query OID_ANDLINK_BSSINFO:if_type:%d;andlink_en is disable.\n",
					pObj->ioctl_if_type);

			}
			break;
#endif/*ANDLINK_V4_0*/

#endif/*ANDLINK_FEATURE_SUPPORT*/

	case RT_OID_VERSION_INFO: {
		int ret;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_VERSION_INFO\n");
		wrq->u.data.length = strlen(AP_DRIVER_VERSION);
		ret = snprintf(&driverVersion[0], sizeof(driverVersion), "%s", AP_DRIVER_VERSION);
		if (os_snprintf_error(sizeof(driverVersion), ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			break;
		}

		driverVersion[wrq->u.data.length] = '\0';

		if (copy_to_user(wrq->u.data.pointer, &driverVersion, wrq->u.data.length))
			Status = -EFAULT;
	}
		break;
#if defined(VENDOR_FEATURE6_SUPPORT) || defined(TR181_SUPPORT)
	case RT_OID_802_11_PHY_MODE: {
		UCHAR *temp_wmode = NULL;

		if (wdev) {
			ulInfo = (ULONG)wdev->PhyMode;
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			temp_wmode = wmode_2_str(wdev->PhyMode);

			if (temp_wmode != NULL) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Query::RT_OID_802_11_PHY_MODE (=%lu), %s\n",
					 ulInfo, temp_wmode);
				os_free_mem(temp_wmode);
			} else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Query::RT_OID_802_11_PHY_MODE (=%lu), Null\n", ulInfo);
		}
	}
	break;
#endif
#ifdef VENDOR_FEATURE6_SUPPORT
	case OID_802_11_CHANNEL_WIDTH: {
		if (wdev) {
			wrq->u.data.length = sizeof(UCHAR);
			ulInfo =  wlan_operate_get_ht_bw(wdev);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_CHANNEL_WIDTH (=%lu)\n", ulInfo);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid wdev for cmd %d\n", cmd);
		}
		break;
	}
	break;

	case RT_OID_802_11_COUNTRY_REGION:
		wrq->u.data.length = sizeof(ulInfo);
		ulInfo = pAd->CommonCfg.CountryRegionForABand;
		ulInfo = (ulInfo << 8) | (pAd->CommonCfg.CountryRegion);

		if (copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length))
			Status = -EFAULT;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_COUNTRY_REGION (=%lu)\n", ulInfo);
		break;

	case OID_802_11_BEACON_PERIOD:
		wrq->u.data.length = sizeof(ulInfo);
		ulInfo = pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)];

		if (copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length))
			Status = -EFAULT;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_BEACON_PERIOD (=%lu)\n", ulInfo);
		break;

	case RT_OID_802_11_TX_POWER_LEVEL_1:
		wrq->u.data.length = sizeof(UINT8);
		ulInfo = pAd->CommonCfg.ucTxPowerPercentage[BAND0];
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.ucTxPowerPercentage[BAND0], wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_TX_POWER_LEVEL_1 (=%u)\n", pAd->CommonCfg.ucTxPowerPercentage[BAND0]);
		break;

	case RT_OID_802_11_QUERY_WMM:
		if (wdev) {
			wrq->u.data.length = sizeof(BOOLEAN);
			Status = copy_to_user(wrq->u.data.pointer, &wdev->bWmmCapable, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_QUERY_WMM (=%d)\n", wdev->bWmmCapable);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid wdev for cmd %d\n", cmd);

		break;

	case RT_OID_802_11_PREAMBLE:
		wrq->u.data.length = sizeof(ulInfo);
		ulInfo = pAd->CommonCfg.TxPreamble;
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_PREAMBLE(=%lu)\n", pAd->CommonCfg.TxPreamble);
		break;

	case OID_802_11_HT_STBC:
		if (wdev) {
			wrq->u.data.length = sizeof(UCHAR);
			ulInfo = wlan_config_get_ht_stbc(wdev);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_HT_STBC(=%lu)\n", ulInfo);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid wdev for cmd %d\n", cmd);

		break;

	case OID_802_11_UAPSD:
		if (wdev) {
			wrq->u.data.length = sizeof(BOOLEAN);
			Status = copy_to_user(wrq->u.data.pointer, &wdev->UapsdInfo.bAPSDCapable, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_UAPSD (=%d)\n", wdev->UapsdInfo.bAPSDCapable);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid wdev for cmd %d\n", cmd);

		break;

	case OID_802_11_TX_PACKET_BURST:
		wrq->u.data.length = sizeof(BOOLEAN);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.bEnableTxBurst, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_TX_PACKET_BURST (=%d)\n", pAd->CommonCfg.bEnableTxBurst);
		break;

	case OID_802_11_AMSDU:
		wrq->u.data.length = sizeof(ulInfo);
		ulInfo = pAd->CommonCfg.BACapability.field.AmsduEnable;
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_AMSDU (=%lu)\n", ulInfo);
		break;

	case OID_802_11_AMPDU:
		if (wdev) {
			wrq->u.data.length = sizeof(ulInfo);
			ulInfo = wlan_config_get_ba_enable(wdev);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_AMPDU (=%lu)\n", ulInfo);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid wdev for cmd %d\n", cmd);

		break;

	case OID_802_11_ASSOLIST:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_ASSOLIST\n");
		RTMPAPGetAssoMacTable(pAd, wrq);
		break;
#ifdef WSC_AP_SUPPORT

	case OID_802_11_CURRENT_CRED:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_CURRENT_CRED\n");
		RTMPGetCurrentCred(pAd, wrq);
		break;
#endif /* WSC_AP_SUPPORT */
#endif /* VENDOR_FEATURE6_SUPPORT */

#if (defined(VENDOR_FEATURE6_SUPPORT) || defined(CONFIG_MAP_SUPPORT))
	case OID_802_11_COEXISTENCE:
		wrq->u.data.length = sizeof(BOOLEAN);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.bBssCoexEnable, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_COEXISTENCE (=%d)\n", pAd->CommonCfg.bBssCoexEnable);
		break;
#endif /* defined(VENDOR_FEATURE6_SUPPORT) || defined(CONFIG_MAP_SUPPORT) */

	case OID_802_11_NETWORK_TYPES_SUPPORTED:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_NETWORK_TYPES_SUPPORTED\n");
		wrq->u.data.length = sizeof(UCHAR);

		if (copy_to_user(wrq->u.data.pointer, &pAd->RfIcType, wrq->u.data.length))
			Status = -EFAULT;

		break;
#ifdef IAPP_SUPPORT

	case RT_QUERY_SIGNAL_CONTEXT: {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_QUERY_SIGNAL_CONTEXT\n");
#ifndef DOT11R_FT_SUPPORT
		Status = -EFAULT;
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
		{
			FT_KDP_SIGNAL *pFtKdp;
			FT_KDP_EVT_HEADER *pEvtHdr;
			/* query signal content for 11r */
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_QUERY_FT_KDP_CONTEXT\n");
			FT_KDP_EventGet(pAd, &pFtKdp);

			if (pFtKdp != NULL)
				pEvtHdr = (FT_KDP_EVT_HEADER *)pFtKdp->Content;

			/* End of if */

			if ((pFtKdp != NULL) &&
				((RT_SIGNAL_STRUC_HDR_SIZE + pEvtHdr->EventLen) <=
				 wrq->u.data.length)) {
				/* copy the event */
				if (copy_to_user(
						wrq->u.data.pointer,
						pFtKdp,
						RT_SIGNAL_STRUC_HDR_SIZE + pEvtHdr->EventLen)) {
					wrq->u.data.length = 0;
					Status = -EFAULT;
				} else {
					wrq->u.data.length = RT_SIGNAL_STRUC_HDR_SIZE;
					wrq->u.data.length += pEvtHdr->EventLen;
				}

				FT_MEM_FREE(pAd, pFtKdp);
			} else {
				/* no event is queued */
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ft_kdp> no event is queued!\n");
				wrq->u.data.length = 0;
			}
		}

#endif /* DOT11R_FT_SUPPORT */
	}
	break;

#ifdef QUERY_KVRH_SUPPORT
	case RT_QUERY_11H_CAPABILITY:
		{
			UINT16 TotalLen = 0;
			PMAC_TABLE_ENTRY pEntry = NULL;
			IEEE_80211H_CAPABILITY Capability;

			TotalLen = wrq->u.data.length;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					("RT_QUERY_80211H_CAPABILITY\n"));

			if (TotalLen != sizeof(Capability)) {
				Status = -EINVAL;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"RT_QUERY_KVR_CAPABILITY Wrong TotalLen(%d)!\n",
					TotalLen);
				break;
			}

			if (copy_from_user(&Capability, wrq->u.data.pointer, wrq->u.data.length)) {
				Status = -EFAULT;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"RT_QUERY_80211H_CAPABILITY copy from user failed!\n");
				break;
			}

			pEntry = MacTableLookup(pAd, Capability.StaMac);
			if (!pEntry || !(IS_AKM_OPEN(pAd->ApCfg.MBSSID[ifIndex].wdev.SecConfig.AKMMap) ||
				((pEntry->SecConfig.Handshake.WpaState == AS_PTKINITDONE) &&
				(pEntry->SecConfig.Handshake.GTKState == REKEY_ESTABLISHED)))) {
				Status = -ENOTCONN;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"RT_QUERY_80211H_CAPABILITY STA("MACSTR") not associates with Ap\n",
					MAC2STR(Capability.StaMac));
				break;
			}

			Capability.IsSupport80211h = 0;

			if (pEntry->CapabilityInfo & 0x0100)
				Capability.IsSupport80211h = 1;

			if (copy_to_user(wrq->u.data.pointer, &Capability, wrq->u.data.length)) {
				Status = -EFAULT;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"RT_QUERY_80211H_CAPABILITY copy to user failed!\n");
				break;
			}
		}
		break;

	case RT_QUERY_KVR_CAPABILITY:
		{
			PMAC_TABLE_ENTRY pEntry = NULL;
			UINT16 TotalLen = 0;
			KVR_CAPABILITY Capability;

			TotalLen = wrq->u.data.length;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RT_QUERY_KVR_CAPABILITY\n");

			if (TotalLen != sizeof(Capability)) {
				Status = -EINVAL;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"RT_QUERY_KVR_CAPABILITY Wrong TotalLen(%d)!\n", TotalLen);
				break;
			}

			if (copy_from_user(&Capability, wrq->u.data.pointer, wrq->u.data.length)) {
				Status = -EFAULT;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"RT_QUERY_KVR_CAPABILITY copy from user failed!\n");
				break;
			}

			pEntry = MacTableLookup(pAd, Capability.StaMac);
			if (!pEntry || !(IS_AKM_OPEN(pAd->ApCfg.MBSSID[ifIndex].wdev.SecConfig.AKMMap) ||
				((pEntry->SecConfig.Handshake.WpaState == AS_PTKINITDONE) &&
				(pEntry->SecConfig.Handshake.GTKState == REKEY_ESTABLISHED)))) {
				Status = -ENOTCONN;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"RT_QUERY_KVR_CAPABILITY STA("MACSTR") not associates with Ap\n",
					MAC2STR(Capability.StaMac));
				break;
			}

			Capability.KVRCap = 0;

#ifdef DOT11K_RRM_SUPPORT
			if ((pEntry->CapabilityInfo & RRM_CAP_BIT) &&
				(pEntry->RrmEnCap.field.BeaconPassiveMeasureCap ||
				pEntry->RrmEnCap.field.BeaconActiveMeasureCap))
				Capability.KVRCap |= SUPPORT_11K_CAP;
#endif
#ifdef CONFIG_DOT11V_WNM
			if (pEntry->BssTransitionManmtSupport)
				Capability.KVRCap |= SUPPORT_11V_CAP;
#endif
#ifdef DOT11R_FT_SUPPORT
			if (IS_FT_STA(pEntry))
				Capability.KVRCap |= SUPPORT_11R_CAP;
#endif

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"RT_QUERY_KVR_CAPABILITY KVRCap=%d\n", Capability.KVRCap);

			if (copy_to_user(wrq->u.data.pointer, &Capability, wrq->u.data.length)) {
				Status = -EFAULT;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"RT_QUERY_11KVR_CAPABILITY copy to user failed!\n");
				break;
			}
		}
		break;
#endif /* QUERY_KVRH_SUPPORT */

#ifdef DOT11R_FT_SUPPORT

	case RT_FT_DATA_ENCRYPT:
	case RT_FT_DATA_DECRYPT: {
		UCHAR *pBuffer;
		UINT32 DataLen;

		DataLen = wrq->u.data.length;

		/*
			Make sure the data length is multiple of 8
			due to AES_KEY_WRAP() limitation.
		*/
		if (DataLen & 0x07)
			DataLen += 8 - (DataLen & 0x07);

		/* End of if */
		FT_MEM_ALLOC(pAd, &pBuffer, DataLen + FT_KDP_KEY_ENCRYPTION_EXTEND);

		if (pBuffer == NULL)
			break;

		NdisZeroMemory(pBuffer, DataLen + FT_KDP_KEY_ENCRYPTION_EXTEND);

		if (copy_from_user(pBuffer, wrq->u.data.pointer, wrq->u.data.length)) {
			Status = -EFAULT;
			FT_MEM_FREE(pAd, pBuffer);
			break;
		}

		switch (cmd) {
		case RT_FT_DATA_ENCRYPT:
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_FT_DATA_ENCRYPT\n");
			FT_KDP_DataEncrypt(pAd, (UCHAR *)pBuffer, &DataLen);
			break;

		case RT_FT_DATA_DECRYPT:
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_FT_DATA_DECRYPT\n");
			FT_KDP_DataDecrypt(pAd, (UCHAR *)pBuffer, &DataLen);
			break;
		}

		wrq->u.data.length = DataLen;

		if (copy_to_user(wrq->u.data.pointer, pBuffer, wrq->u.data.length))
			Status = -EFAULT;

		FT_MEM_FREE(pAd, pBuffer);
	}
	break;

	case RT_OID_802_11R_INFO: {
		PFT_CONFIG_INFO pFtConfig;
		PFT_CFG pFtCfg;

		os_alloc_mem(pAd, (UCHAR **)&pFtConfig, sizeof(FT_CONFIG_INFO));

		if (pFtConfig == NULL)
			break;

		pFtCfg = &pAd->ApCfg.MBSSID[apidx].wdev.FtCfg;
		NdisZeroMemory(pFtConfig, sizeof(FT_CONFIG_INFO));
		pFtConfig->FtSupport = pFtCfg->FtCapFlag.Dot11rFtEnable;
		pFtConfig->FtRicSupport = pFtCfg->FtCapFlag.RsrReqCap;
		pFtConfig->FtOtdSupport = pFtCfg->FtCapFlag.FtOverDs;
		NdisMoveMemory(pFtConfig->MdId, pFtCfg->FtMdId, FT_MDID_LEN);
		pFtConfig->R0KHIdLen = pFtCfg->FtR0khIdLen;
		NdisMoveMemory(pFtConfig->R0KHId, pFtCfg->FtR0khId, pFtCfg->FtR0khIdLen);
		wrq->u.data.length = sizeof(FT_CONFIG_INFO);
		Status = copy_to_user(wrq->u.data.pointer, pFtConfig, wrq->u.data.length);
		os_free_mem(pFtConfig);
	}
	break;
#endif /* DOT11R_FT_SUPPORT */
#endif /* IAPP_SUPPORT */

#ifdef CONFIG_DOT11V_WNM
	case RT_QUERY_WNM_CAPABILITY: {
		PUCHAR p_wnm_query_data = NULL;
		PMAC_TABLE_ENTRY pEntry = NULL;
		PUINT8 p_cap = NULL;
		struct wnm_command *p_wnm_command = NULL;
		PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[apidx].WNMCtrl;
		UINT16 TotalLen = 0;

		TotalLen = wrq->u.data.length;
		if (TotalLen != sizeof(*p_wnm_command)+MAC_ADDR_LEN+1) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
				"RT_QUERY_WNM_CAPABILITY: length(%d) check failed\n", TotalLen);
			Status = EINVAL;
			break;
		}

		if (!pWNMCtrl->WNMBTMEnable) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
				"RT_QUERY_WNM_CAPABILITY: btm off\n");
			Status = EINVAL;
			break;
		}

		os_alloc_mem(NULL, (UCHAR **)&p_wnm_command, wrq->u.data.length);
		if (p_wnm_command == NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
				"RT_QUERY_WNM_CAPABILITY: no memory!\n");
			Status = ENOMEM;
			break;
		}

		if (copy_from_user(p_wnm_command, wrq->u.data.pointer, wrq->u.data.length)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"RT_QUERY_WNM_CAPABILITY: copy from user failed!\n");
			Status = EFAULT;
			os_free_mem(p_wnm_command);
			break;
		}

		p_wnm_query_data = p_wnm_command->command_body;

		/*first six bytes of data is sta mac*/
		pEntry = MacTableLookup(pAd, p_wnm_query_data);
		if (!pEntry ||
			!(IS_AKM_OPEN(pAd->ApCfg.MBSSID[ifIndex].wdev.SecConfig.AKMMap) ||
			((pEntry->SecConfig.Handshake.WpaState == AS_PTKINITDONE) &&
			(pEntry->SecConfig.Handshake.GTKState == REKEY_ESTABLISHED)))) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
					"RT_QUERY_WNM_CAPABILITY: STA("MACSTR") not associates with AP!\n",
					MAC2STR(p_wnm_query_data));
				Status = EINVAL;
				os_free_mem(p_wnm_command);
				break;
		}
		/*check for btm capablility*/
		p_cap = (UINT8 *)(p_wnm_query_data+MAC_ADDR_LEN);
		*p_cap = 0;

		if (p_wnm_command->command_id == OID_802_11_WNM_CMD_QUERY_BTM_CAP) {
			if (pEntry->BssTransitionManmtSupport) {
				*p_cap = 1;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"RT_QUERY_WNM_CAPABILITY: BTMCap=%d\n", (*p_cap));
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"RT_QUERY_WNM_CAPABILITY: only check btm cap now\n");
			Status = EINVAL;
			os_free_mem(p_wnm_command);
			break;
		}

		if (copy_to_user(wrq->u.data.pointer, (PUCHAR)p_wnm_command, wrq->u.data.length)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"RT_QUERY_WNM_CAPABILITY: copy to user failed!\n");
			Status = EFAULT;
			os_free_mem(p_wnm_command);
			break;
		}
		os_free_mem(p_wnm_command);
	}
	break;
#endif

#ifdef DOT11K_RRM_SUPPORT
	case RT_QUERY_RRM_CAPABILITY: {
		PUCHAR p_rrm_query_data = NULL;
		PMAC_TABLE_ENTRY pEntry = NULL;
		p_rrm_command_t p_rrm_command = NULL;
		BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
		PUINT8 p_cap = NULL;
		UINT16 TotalLen = 0;

		TotalLen = wrq->u.data.length;
		if (TotalLen != sizeof(*p_rrm_command)+MAC_ADDR_LEN+8) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
				"RT_QUERY_RRM_CAPABILITY: length(%d) check failed\n",
				TotalLen);
			Status = EINVAL;
			break;
		}
		if (!pMbss->wdev.RrmCfg.bDot11kRRMEnable) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_WNM, DBG_LVL_ERROR,
				"RT_QUERY_RRM_CAPABILITY: rrm off\n");
			Status = EINVAL;
			break;
		}
		os_alloc_mem(NULL, (UCHAR **)&p_rrm_command, wrq->u.data.length);
		if (p_rrm_command == NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
				"RT_QUERY_RRM_CAPABILITY : no memory!\n");
			Status = ENOMEM;
			break;
		}

		if (copy_from_user(p_rrm_command, wrq->u.data.pointer, wrq->u.data.length)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"RT_QUERY_RRM_CAPABILITY: copy from user failed!\n");
			Status = EFAULT;
			os_free_mem(p_rrm_command);
			break;
		}

		p_rrm_query_data = p_rrm_command->command_body;

		/*first six bytes of data is sta mac*/
		pEntry = MacTableLookup(pAd, p_rrm_query_data);
		if (!pEntry ||
			!(IS_AKM_OPEN(pAd->ApCfg.MBSSID[ifIndex].wdev.SecConfig.AKMMap) ||
			((pEntry->SecConfig.Handshake.WpaState == AS_PTKINITDONE) &&
			(pEntry->SecConfig.Handshake.GTKState == REKEY_ESTABLISHED)))) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
				"RT_QUERY_RRM_CAPABILITY: STA("MACSTR") not associates with AP!\n",
				MAC2STR(p_rrm_query_data));
			Status = EINVAL;
			os_free_mem(p_rrm_command);
			break;
		}
		/*check for btm capablility*/
		p_cap = (UINT8 *)(p_rrm_query_data+MAC_ADDR_LEN);
		memset(p_cap, 0, 8);

		if (p_rrm_command->command_id == OID_802_11_RRM_CMD_QUERY_CAP) {
			memcpy(p_cap, (PUCHAR)&(pEntry->RrmEnCap), sizeof(pEntry->RrmEnCap));
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"RT_QUERY_WNM_CAPABILITY: only check rrm cap now\n");
			Status = EINVAL;
			os_free_mem(p_rrm_command);
			break;
		}

		if (copy_to_user(wrq->u.data.pointer, (PUCHAR)p_rrm_command, wrq->u.data.length)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"RT_QUERY_RRM_CAPABILITY: copy to user failed!\n");
			Status = EFAULT;
			os_free_mem(p_rrm_command);
			break;
		}
		os_free_mem(p_rrm_command);
	}
	break;
#endif

#ifdef WSC_AP_SUPPORT

	case RT_OID_WSC_QUERY_STATUS: {
		INT WscStatus;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_WSC_QUERY_STATUS\n");
#ifdef APCLI_SUPPORT

		if (pObj->ioctl_if_type == INT_APCLI) {
			UINT ApCliIdx = pObj->ioctl_if;

			APCLI_MR_APIDX_SANITY_CHECK(ApCliIdx);
			WscStatus = pAd->StaCfg[ApCliIdx].wdev.WscControl.WscStatus;
		} else
#endif /* APCLI_SUPPORT */
#ifdef P2P_SUPPORT
			if (pObj->ioctl_if_type == INT_P2P) {
				if (P2P_CLI_ON(pAd)) {
					UINT ApCliIdx = pObj->ioctl_if;

					APCLI_MR_APIDX_SANITY_CHECK(ApCliIdx);
					WscStatus = pAd->StaCfg[ApCliIdx].wdev.WscControl.WscStatus;
				} else
					WscStatus = pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscStatus;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "IF(p2p%d) Set_WscConfMode_Proc:: This command is from p2p interface now.\n", apidx);
			} else
#endif /* P2P_SUPPORT */
			{
				WscStatus = pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscStatus;
			}

		wrq->u.data.length = sizeof(INT);

		if (copy_to_user(wrq->u.data.pointer, &WscStatus, wrq->u.data.length))
			Status = -EFAULT;

		break;
	}

	case RT_OID_WSC_PIN_CODE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_WSC_PIN_CODE\n");
		wrq->u.data.length = sizeof(UINT);
		/*WscPinCode = GenerateWpsPinCode(pAd, FALSE, apidx); */
#ifdef P2P_SUPPORT

		if (pObj->ioctl_if_type == INT_P2P) {
			if (P2P_CLI_ON(pAd))
				pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
			else
				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "IF(p2p%d) Set_WscConfMode_Proc:: This command is from p2p interface now.\n", apidx);
		} else
#endif /* P2P_SUPPORT */
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;

		WscPinCode = pWscControl->WscEnrolleePinCode;

		if (copy_to_user(wrq->u.data.pointer, &WscPinCode, wrq->u.data.length))
			Status = -EFAULT;

		break;
#ifdef APCLI_SUPPORT

	case RT_OID_APCLI_WSC_PIN_CODE:
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_APCLI_WSC_PIN_CODE\n");
		wrq->u.data.length = sizeof(UINT);
		/*WscPinCode = GenerateWpsPinCode(pAd, TRUE, apidx); */
		WscPinCode = pAd->StaCfg[apidx].wdev.WscControl.WscEnrolleePinCode;

		if (copy_to_user(wrq->u.data.pointer, &WscPinCode, wrq->u.data.length))
			Status = -EFAULT;

		break;
#endif /* APCLI_SUPPORT */
	case OID_WSC_UUID:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_WSC_QUERY_UUID\n");
		pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
		wrq->u.data.length = UUID_LEN_HEX;
		if (copy_to_user(wrq->u.data.pointer, &pWscControl->Wsc_Uuid_E[0], UUID_LEN_HEX))
			Status = -EFAULT;
		break;

	case RT_OID_WSC_UUID:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_WSC_QUERY_UUID\n");
		wrq->u.data.length = UUID_LEN_STR;
#ifdef P2P_SUPPORT

		if (pObj->ioctl_if_type == INT_P2P) {
			if (P2P_CLI_ON(pAd))
				pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
			else
				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "IF(p2p%d) Set_WscConfMode_Proc:: This command is from p2p interface now.\n", apidx);
		} else
#endif /* P2P_SUPPORT */
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;

		if (copy_to_user(wrq->u.data.pointer, &pWscControl->Wsc_Uuid_Str[0], UUID_LEN_STR))
			Status = -EFAULT;

		break;

	case RT_OID_WSC_MAC_ADDRESS:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_WSC_MAC_ADDRESS\n");
		wrq->u.data.length = MAC_ADDR_LEN;

		if (copy_to_user(wrq->u.data.pointer, pAd->ApCfg.MBSSID[apidx].wdev.bssid, wrq->u.data.length))
			Status = -EFAULT;

		break;

	case RT_OID_WSC_CONFIG_STATUS:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_WSC_CONFIG_STATUS\n");
		wrq->u.data.length = sizeof(UCHAR);

		if (copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfStatus, wrq->u.data.length))
			Status = -EFAULT;

		break;

	case RT_OID_WSC_QUERY_PEER_INFO_ON_RUNNING:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_WSC_QUERY_PEER_INFO_ON_RUNNING\n");

		if (pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscState > WSC_STATE_WAIT_M2) {
			wrq->u.data.length = sizeof(WSC_PEER_DEV_INFO);

			if (copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscPeerInfo, wrq->u.data.length))
				Status = -EFAULT;
		} else
			Status = -EFAULT;

		break;

	case RT_OID_802_11_WSC_QUERY_PROFILE:
		wrq->u.data.length = sizeof(WSC_PROFILE);
		os_alloc_mem(pAd, (UCHAR **)&pProfile, sizeof(WSC_PROFILE));

		if (pProfile == NULL) {
			Status = -EFAULT;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RT_OID_802_11_WSC_QUERY_PROFILE fail!\n");
			break;
		}

#ifdef P2P_SUPPORT

		if (pObj->ioctl_if_type == INT_P2P) {
			if (P2P_CLI_ON(pAd))
				pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
			else
				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "IF(p2p%d) Set_WscConfMode_Proc:: This command is from p2p interface now.\n", apidx);
		} else
#endif /* P2P_SUPPORT */
#ifdef APCLI_SUPPORT
			if (pObj->ioctl_if_type == INT_APCLI) {
				APCLI_MR_APIDX_SANITY_CHECK(apidx);
				pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "IF(apcli : %d) RT_OID_802_11_WSC_QUERY_PROFILE :: This command is from apcli interface now.\n", apidx);
			} else
#endif /* APCLI_SUPPORT */
				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;

		RTMPZeroMemory(pProfile, sizeof(WSC_PROFILE));
		NdisMoveMemory(pProfile, &pWscControl->WscProfile, sizeof(WSC_PROFILE));

		if ((pProfile->Profile[0].AuthType == WSC_AUTHTYPE_OPEN) && (pProfile->Profile[0].EncrType == WSC_ENCRTYPE_NONE)) {
			pProfile->Profile[0].KeyLength = 0;
			NdisZeroMemory(pProfile->Profile[0].Key, 64);
		}

		if (copy_to_user(wrq->u.data.pointer, pProfile, wrq->u.data.length))
			Status = -EFAULT;

		os_free_mem(pProfile);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_WSC_QUERY_PROFILE\n");
		break;
#ifdef WSC_V2_SUPPORT

	case RT_OID_WSC_V2_SUPPORT:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_WSC_V2_SUPPORT (=%d)\n",
				 pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscV2Info.bEnableWpsV2);
		wrq->u.data.length = sizeof(BOOLEAN);

		if (copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscV2Info.bEnableWpsV2,
						 wrq->u.data.length))
			Status = -EFAULT;

		break;

	case RT_OID_WSC_FRAGMENT_SIZE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_WSC_FRAGMENT_SIZE (=%d)\n",
				 pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscFragSize);
		wrq->u.data.length = sizeof(USHORT);

		if (copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscFragSize, wrq->u.data.length))
			Status = -EFAULT;

		break;
#endif /* WSC_V2_SUPPORT */
#endif /* WSC_AP_SUPPORT */
#ifdef LLTD_SUPPORT

	case RT_OID_GET_LLTD_ASSO_TABLE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::Get LLTD association table\n");

		if ((wrq->u.data.pointer == NULL) || (apidx != MAIN_MBSSID))
			Status = -EFAULT;
		else {
			INT						    i;
			RT_LLTD_ASSOICATION_TABLE	AssocTab;

			AssocTab.Num = 0;

			for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
				if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) && (pAd->MacTab.Content[i].Sst == SST_ASSOC)) {
					COPY_MAC_ADDR(AssocTab.Entry[AssocTab.Num].Addr, &pAd->MacTab.Content[i].Addr);
					AssocTab.Entry[AssocTab.Num].phyMode = pAd->ApCfg.MBSSID[apidx].wdev.PhyMode;
					AssocTab.Entry[AssocTab.Num].MOR = RateIdToMbps[pAd->ApCfg.MBSSID[apidx].MaxTxRate] * 2;
					AssocTab.Num += 1;
				}
			}

			wrq->u.data.length = sizeof(RT_LLTD_ASSOICATION_TABLE);

			if (copy_to_user(wrq->u.data.pointer, &AssocTab, wrq->u.data.length)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_to_user() fail\n");
				Status = -EFAULT;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AssocTab.Num = %d\n", AssocTab.Num);
		}

		break;
#ifdef APCLI_SUPPORT

	case RT_OID_GET_REPEATER_AP_LINEAGE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Not Support : Get repeater AP lineage.\n");
		break;
#endif /* APCLI_SUPPORT */
#endif /* LLTD_SUPPORT */
#ifdef DOT1X_SUPPORT

	case OID_802_DOT1X_CONFIGURATION:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::Get Radius setting(%lu)\n",
				 (ULONG)sizeof(DOT1X_CMM_CONF));
		Dot1xIoctlQueryRadiusConf(pAd, wrq);
		break;

	case OID_802_DOT1X_QUERY_STA_AID:
		RTMPIoctlQueryStaAid(pAd, wrq);
		break;
#ifdef RADIUS_ACCOUNTING_SUPPORT

	case OID_802_DOT1X_QUERY_STA_DATA:
		RTMPIoctlQueryStaData(pAd, wrq);
		break;
#endif /* RADIUS_ACCOUNTING_SUPPORT */
	case OID_802_DOT1X_QUERY_STA_RSN:
		RTMPIoctlQueryStaRsn(pAd, wrq);
		break;
#endif /* DOT1X_SUPPORT */

	case RT_OID_802_11_MAC_ADDRESS:
		wrq->u.data.length = MAC_ADDR_LEN;
		Status = copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].wdev.bssid, wrq->u.data.length);
		break;
#ifdef SNMP_SUPPORT

	case RT_OID_802_11_MANUFACTUREROUI:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_MANUFACTUREROUI\n");
		wrq->u.data.length = ManufacturerOUI_LEN;
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CurrentAddress, wrq->u.data.length);
		break;

	case RT_OID_802_11_MANUFACTURERNAME:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_MANUFACTURERNAME\n");
		wrq->u.data.length = strlen(ManufacturerNAME);
		Status = copy_to_user(wrq->u.data.pointer, ManufacturerNAME, wrq->u.data.length);
		break;

	case RT_OID_802_11_RESOURCETYPEIDNAME:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_RESOURCETYPEIDNAME\n");
		wrq->u.data.length = strlen(ResourceTypeIdName);
		Status = copy_to_user(wrq->u.data.pointer, ResourceTypeIdName, wrq->u.data.length);
		break;

	case RT_OID_802_11_PRIVACYOPTIONIMPLEMENTED: {
		ULONG ulInfo;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_PRIVACYOPTIONIMPLEMENTED\n");
		ulInfo = 1; /* 1 is support wep else 2 is not support. */
		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		break;
	}

	case RT_OID_802_11_POWERMANAGEMENTMODE: {
		ULONG ulInfo;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_POWERMANAGEMENTMODE\n");
		ulInfo = 1; /* 1 is power active else 2 is power save. */
		wrq->u.data.length = sizeof(ulInfo);
		Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
		break;
	}

	case OID_802_11_WEPDEFAULTKEYVALUE:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_WEPDEFAULTKEYVALUE\n");
		pKeyIdxValue = wrq->u.data.pointer;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "KeyIdxValue.KeyIdx = %d,\n", pKeyIdxValue->KeyIdx);
		valueLen = pAd->SharedKey[ifIndex][pAd->ApCfg.MBSSID[ifIndex].DefaultKeyId].KeyLen;
		NdisMoveMemory(pKeyIdxValue->Value,
					   &pAd->SharedKey[ifIndex][pAd->ApCfg.MBSSID[ifIndex].DefaultKeyId].Key,
					   valueLen);
		pKeyIdxValue->Value[valueLen] = '\0';
		wrq->u.data.length = sizeof(DefaultKeyIdxValue);
		Status = copy_to_user(wrq->u.data.pointer, pKeyIdxValue, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "DefaultKeyId = %d, total len = %d, str len=%d, KeyValue= %02x %02x %02x %02x\n",
				  pAd->ApCfg.MBSSID[ifIndex].DefaultKeyId, wrq->u.data.length,
				  pAd->SharedKey[ifIndex][pAd->ApCfg.MBSSID[ifIndex].DefaultKeyId].KeyLen,
				  pAd->SharedKey[ifIndex][0].Key[0],
				  pAd->SharedKey[ifIndex][1].Key[0],
				  pAd->SharedKey[ifIndex][2].Key[0],
				  pAd->SharedKey[ifIndex][3].Key[0]);
		break;

	case OID_802_11_WEPDEFAULTKEYID:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_WEPDEFAULTKEYID\n");
		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[ifIndex].DefaultKeyId, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DefaultKeyId =%d\n",
				 pAd->ApCfg.MBSSID[ifIndex].DefaultKeyId);
		break;

	case RT_OID_802_11_WEPKEYMAPPINGLENGTH:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_WEPKEYMAPPINGLENGTH\n");
		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer,
							  &pAd->SharedKey[ifIndex][pAd->ApCfg.MBSSID[ifIndex].DefaultKeyId].KeyLen,
							  wrq->u.data.length);
		break;

	case OID_802_11_SHORTRETRYLIMIT:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_SHORTRETRYLIMIT\n");
		wrq->u.data.length = sizeof(ULONG);
		ShortRetryLimit = AsicGetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_SHORT);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ShortRetryLimit =%ld\n", ShortRetryLimit);
		Status = copy_to_user(wrq->u.data.pointer, &ShortRetryLimit, wrq->u.data.length);
		break;

	case OID_802_11_LONGRETRYLIMIT:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_LONGRETRYLIMIT\n");
		wrq->u.data.length = sizeof(ULONG);
		LongRetryLimit = AsicGetRetryLimit(pAd, TX_RTY_CFG_RTY_LIMIT_LONG);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "LongRetryLimit =%ld\n", LongRetryLimit);
		Status = copy_to_user(wrq->u.data.pointer, &LongRetryLimit, wrq->u.data.length);
		break;

	case RT_OID_802_11_PRODUCTID:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_PRODUCTID\n");
#ifdef RTMP_PCI_SUPPORT

		if (IS_PCI_INF(pAd)) {
			USHORT  device_id;

			if (((POS_COOKIE)pAd->OS_Cookie)->pci_dev != NULL)
				pci_read_config_word(((POS_COOKIE)pAd->OS_Cookie)->pci_dev, PCI_DEVICE_ID, &device_id);
			else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "pci_dev = NULL\n");

			snprintf((RTMP_STRING *)snmp_tmp, sizeof(snmp_tmp), "%04x %04x\n", NIC_PCI_VENDOR_ID, device_id);
		}

#endif /* RTMP_PCI_SUPPORT */
		wrq->u.data.length = strlen((RTMP_STRING *) snmp_tmp);
		Status = copy_to_user(wrq->u.data.pointer, snmp_tmp, wrq->u.data.length);
		break;

	case RT_OID_802_11_MANUFACTUREID:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_MANUFACTUREID\n");
		wrq->u.data.length = strlen(ManufacturerNAME);
		Status = copy_to_user(wrq->u.data.pointer, ManufacturerNAME, wrq->u.data.length);
		break;
#endif /* SNMP_SUPPORT */
#if (defined(SNMP_SUPPORT) || defined(WH_EZ_SETUP) || defined(VENDOR_FEATURE6_SUPPORT) || defined(TR181_SUPPORT))

	case OID_802_11_CURRENTCHANNEL:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_CURRENTCHANNEL\n");
		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[apidx].wdev.channel, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Status=%d\n", Status);
		break;
#endif

	case OID_802_11_STATISTICS:
		os_alloc_mem(pAd, (UCHAR **)&pStatistics, sizeof(NDIS_802_11_STATISTICS));

		if (pStatistics) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_STATISTICS\n");
			/* add the most up-to-date h/w raw counters into software counters */
			/*NICUpdateRawCountersNew(pAd);*/
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
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Query::OID_802_11_STATISTICS(mem alloc failed)\n");
			Status = -EFAULT;
		}

		break;

#ifdef ACL_BLK_COUNT_SUPPORT
	case OID_802_11_ACL_BLK_REJCT_COUNT_STATICS:
		{
			int count = 0;
			ULONG Policy;
			ULONG Num;
			ULONG Reject_Count = 0;
			UCHAR *pAddr;

			wrq->u.data.length = sizeof(RT_802_11_ACL);
			Policy = pAd->ApCfg.MBSSID[apidx].AccessControlList.Policy;
			Num = pAd->ApCfg.MBSSID[apidx].AccessControlList.Num;
			pAddr = (pAd->ApCfg.MBSSID[apidx].AccessControlList.Entry[count].Addr);
			if (Policy == 2)
				Status = copy_to_user(wrq->u.data.pointer,
							&pAd->ApCfg.MBSSID[apidx].AccessControlList,
								wrq->u.data.length);
			else {
				for (count = 0; count < pAd->ApCfg.MBSSID[apidx].AccessControlList.Num; count++)
					pAd->ApCfg.MBSSID[apidx].AccessControlList.Entry[count].Reject_Count = 0;
				Status = copy_to_user(wrq->u.data.pointer,
								&pAd->ApCfg.MBSSID[apidx].AccessControlList,
								wrq->u.data.length);
			}
			break;
		}
#endif/*ACL_BLK_COUNT_SUPPORT*/
	case RT_OID_802_11_PER_BSS_STATISTICS: {
		PMBSS_STATISTICS pMbssStat;
		BSS_STRUCT *pMbss;

		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			return FALSE;
		}

		pMbss = &pAd->ApCfg.MBSSID[ifIndex];
		os_alloc_mem(pAd, (UCHAR **) &pMbssStat, sizeof(MBSS_STATISTICS));
		NdisZeroMemory(pMbssStat, sizeof(MBSS_STATISTICS));
		pMbssStat->TransmittedByteCount = pMbss->TransmittedByteCount;
		pMbssStat->ReceivedByteCount =  pMbss->ReceivedByteCount;
		pMbssStat->TxCount =  pMbss->TxCount;
		pMbssStat->RxCount =  pMbss->RxCount;
		pMbssStat->RxErrorCount =  pMbss->RxErrorCount;
		pMbssStat->RxDropCount =  pMbss->RxDropCount;
		pMbssStat->TxErrorCount =  pMbss->TxErrorCount;
		pMbssStat->TxDropCount =  pMbss->TxDropCount;
		pMbssStat->ucPktsTx =  pMbss->ucPktsTx;
		pMbssStat->ucPktsRx =  pMbss->ucPktsRx;
		pMbssStat->mcPktsTx =  pMbss->mcPktsTx;
		pMbssStat->mcPktsRx =  pMbss->mcPktsRx;
		pMbssStat->bcPktsTx = pMbss->bcPktsTx;
		pMbssStat->bcPktsRx = pMbss->bcPktsRx;
		wrq->u.data.length = sizeof(MBSS_STATISTICS);
		Status = copy_to_user(wrq->u.data.pointer, pMbssStat, wrq->u.data.length);
		os_free_mem(pMbssStat);
	}
	break;
#ifdef APCLI_SUPPORT

	case OID_GEN_MEDIA_CONNECT_STATUS: {
		ULONG ApCliIdx = pObj->ioctl_if;
		NDIS_MEDIA_STATE MediaState;
		PMAC_TABLE_ENTRY pEntry;
		STA_TR_ENTRY *tr_entry;
		PSTA_ADMIN_CONFIG pApCliEntry;

		if (pObj->ioctl_if_type != INT_APCLI) {
			Status = -EOPNOTSUPP;
			break;
		}

		APCLI_MR_APIDX_SANITY_CHECK(ApCliIdx);
		pApCliEntry = &pAd->StaCfg[ApCliIdx];

		if (!IS_WCID_VALID(pAd, pApCliEntry->MacTabWCID)) {
			Status = -EOPNOTSUPP;
			break;
		}

		pEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
		tr_entry = &pAd->tr_ctl.tr_entry[pApCliEntry->MacTabWCID];

		if (!IS_ENTRY_PEER_AP(pEntry) && !IS_ENTRY_REPEATER(pEntry)) {
			Status = -EOPNOTSUPP;
			break;
		}

		if ((pAd->StaCfg[ApCliIdx].ApcliInfStat.Valid == TRUE)
			&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
			MediaState = NdisMediaStateConnected;
		else
			MediaState = NdisMediaStateDisconnected;

		wrq->u.data.length = sizeof(NDIS_MEDIA_STATE);
		Status = copy_to_user(wrq->u.data.pointer, &MediaState, wrq->u.data.length);
	}
	break;
#ifdef REPEATER_TX_RX_STATISTIC
	case RT_OID_802_11_REPEATER_TXRX_STATISTIC:
	{
		RETRTXRXINFO *pReptStatis = NULL;
		UINT16 upr_wcid = WCID_INVALID;
		UCHAR upr_ApCliIdx = (UCHAR)pObj->ioctl_if;

		if ((wrq->u.data.length == 0) || (wrq->u.data.length != sizeof(RETRTXRXINFO)))
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"wrq->u.data.length from user wrong!\n");
		os_alloc_mem(pAd, (UCHAR **)&pReptStatis, sizeof(RETRTXRXINFO));
		if (pReptStatis) {
			NdisZeroMemory(pReptStatis, sizeof(RETRTXRXINFO));
			MtRepeaterGetTxRxInfo(pAd, upr_wcid, upr_ApCliIdx, pReptStatis);
			if (copy_to_user(wrq->u.data.pointer, pReptStatis, wrq->u.data.length)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_to_user failed!\n");
				Status = -EFAULT;
			}
			os_free_mem(pReptStatis);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Query::RT_OID_802_11_REPEATER_TXRX_STATISTIC(mem alloc failed)\n");
			Status = -EFAULT;
		}
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Query::RT_OID_802_11_REPEATER_TXRX_STATISTIC ======DONE!======\n");
	}
		break;
#endif /* REPEATER_TX_RX_STATISTIC */
#endif /* APCLI_SUPPORT */
#ifdef RTMP_RBUS_SUPPORT

	case RT_OID_802_11_SNR_0:
		if (wdev && wdev->LastSNR0 > 0) {
			ULONG ulInfo;

			ulInfo = ConvertToSnr(pAd, wdev->LastSNR0);
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_SNR_0(0x=%lx)\n", ulInfo);
		} else
			Status = -EFAULT;

		break;

	case RT_OID_802_11_SNR_1:
		if (wdev && (pAd->Antenna.field.RxPath > 1) && (wdev->LastSNR1 > 0)) {
			ULONG ulInfo;

			ulInfo = ConvertToSnr(pAd, wdev->LastSNR1);
			wrq->u.data.length = sizeof(ulInfo);
			Status = copy_to_user(wrq->u.data.pointer, &ulInfo,	wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_SNR_1(%lx, LastSNR1=%d)\n", ulInfo,
					 wdev->LastSNR1);
		} else
			Status = -EFAULT;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::RT_OID_802_11_SNR_1, Status=%d\n", Status);
		break;
#endif /* RTMP_RBUS_SUPPORT */

	case OID_802_11_ACL_LIST:
		if (wrq->u.data.length < sizeof(RT_802_11_ACL))
			Status = -EINVAL;
		else
			Status = copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[ifIndex].AccessControlList, sizeof(RT_802_11_ACL));

		break;
#ifdef CONFIG_HOTSPOT
#ifdef CONFIG_DOT11V_WNM

	case OID_802_11_WNM_IPV4_PROXY_ARP_LIST: {
		BSS_STRUCT *pMbss;
		PUCHAR pProxyARPTable;
		UINT32 ARPTableLen;

		pMbss = &pAd->ApCfg.MBSSID[ifIndex];
		ARPTableLen = IPv4ProxyARPTableLen(pAd, pMbss);
		os_alloc_mem(NULL, &pProxyARPTable, ARPTableLen);
		GetIPv4ProxyARPTable(pAd, pMbss, &pProxyARPTable);
		wrq->u.data.length = ARPTableLen;
		Status = copy_to_user(wrq->u.data.pointer, pProxyARPTable, ARPTableLen);
		os_free_mem(pProxyARPTable);
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::OID_802_11_WNM_PROXY_ARP_LIST\n");
	break;

	case OID_802_11_WNM_IPV6_PROXY_ARP_LIST: {
		BSS_STRUCT *pMbss;
		PUCHAR pProxyARPTable;
		UINT32 ARPTableLen;

		pMbss = &pAd->ApCfg.MBSSID[ifIndex];
		ARPTableLen = IPv6ProxyARPTableLen(pAd, pMbss);
		os_alloc_mem(NULL, &pProxyARPTable, ARPTableLen);
		GetIPv6ProxyARPTable(pAd, pMbss, &pProxyARPTable);
		wrq->u.data.length = ARPTableLen;
		Status = copy_to_user(wrq->u.data.pointer, pProxyARPTable, ARPTableLen);
		os_free_mem(pProxyARPTable);
	}
	break;
#endif

	case OID_802_11_SECURITY_TYPE: {
		BSS_STRUCT *pMbss;
		PUCHAR pType;
		struct security_type *SecurityType;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Query:OID_802_11_SECURITY_TYPE\n");
		os_alloc_mem(NULL, &pType, sizeof(*SecurityType));
		SecurityType = (struct security_type *)pType;
		pMbss = &pAd->ApCfg.MBSSID[ifIndex];
		SecurityType->ifindex = ifIndex;
		SecurityType->auth_mode = SecAuthModeNewToOld(pMbss->wdev.SecConfig.AKMMap);
		SecurityType->encryp_type = SecEncryModeNewToOld(pMbss->wdev.SecConfig.PairwiseCipher);
		wrq->u.data.length = sizeof(*SecurityType);
		Status = copy_to_user(wrq->u.data.pointer, pType, sizeof(*SecurityType));
		os_free_mem(pType);
	}
	break;

	case OID_802_11_HS_BSSID: {
		BSS_STRUCT *pMbss;

		pMbss = &pAd->ApCfg.MBSSID[ifIndex];
		wrq->u.data.length = 6;
		Status = copy_to_user(wrq->u.data.pointer, pMbss->wdev.bssid, 6);
	}
	break;
#ifdef CONFIG_HOTSPOT_R2

	case OID_802_11_HS_OSU_SSID: {
		wrq->u.data.length = pAd->ApCfg.MBSSID[ifIndex].SsidLen; /* +2; */
		/* tmpbuf[0] = IE_SSID; */
		/* tmpbuf[1] = pAd->ApCfg.MBSSID[pObj->ioctl_if].SsidLen; */
		/* memcpy(&tmpbuf[2], pAd->ApCfg.MBSSID[pObj->ioctl_if].Ssid, pAd->ApCfg.MBSSID[pObj->ioctl_if].SsidLen); */
		/* Status = copy_to_user(wrq->u.data.pointer, tmpbuf, wrq->u.data.length); */
		Status = copy_to_user(wrq->u.data.pointer, pAd->ApCfg.MBSSID[ifIndex].Ssid,
							  pAd->ApCfg.MBSSID[ifIndex].SsidLen);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\033[1;32m, OID_802_11_HS_OSU_SSID [%s]\033[0m\n"
				 , pAd->ApCfg.MBSSID[ifIndex].Ssid);
		/* this is an osu ssid , disable 11U capability */
		pAd->ApCfg.MBSSID[ifIndex].GASCtrl.b11U_enable = FALSE;
	}
	break;
#endif /* CONFIG_HOTSPOT_R2 */
#ifdef CONFIG_HOTSPOT_R3

	case OID_802_11_GET_STA_HSINFO: {
		UCHAR *Buf = NULL;
		STA_HS_CONSORTIUM_OI *hs_consortium;
		MAC_TABLE_ENTRY *mac_entry = NULL;
		os_alloc_mem(Buf, (UCHAR **)&Buf, wrq->u.data.length);

		if (Buf) {
			Status = copy_from_user(Buf, wrq->u.data.pointer, wrq->u.data.length);
			hs_consortium = (struct _sta_hs_consortium_oi *)Buf;
			mac_entry = &pAd->MacTab.Content[hs_consortium->sta_wcid];

			if (mac_entry != NULL) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Roaming Consortium\n");
				hs_consortium->sta_wcid = mac_entry->hs_consortium_oi.sta_wcid;
				hs_consortium->oi_len = mac_entry->hs_consortium_oi.oi_len;
				memcpy(hs_consortium->selected_roaming_consortium_oi, mac_entry->hs_consortium_oi.selected_roaming_consortium_oi,
											 hs_consortium->oi_len);
				wrq->u.data.length = sizeof(STA_HS_CONSORTIUM_OI) + hs_consortium->oi_len;
				Status = copy_to_user(wrq->u.data.pointer, hs_consortium, wrq->u.data.length);
			}
			os_free_mem(Buf);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Memory Allocation Failed\n");
	}
	break;
#endif /* CONFIG_HOTSPOT_R3 */
#endif
#ifdef WAPP_SUPPORT
	case OID_802_11_WIFI_VER: {
		int ret;
		wrq->u.data.length = strlen(AP_DRIVER_VERSION);
		ret = snprintf(&driverVersion[0], sizeof(driverVersion), "%s", AP_DRIVER_VERSION);
		if (os_snprintf_error(sizeof(driverVersion), ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			break;
		}

		driverVersion[wrq->u.data.length] = '\0';

		Status = copy_to_user(wrq->u.data.pointer, driverVersion, wrq->u.data.length);
	}
	break;
	case OID_802_11_WAPP_SUPPORT_VER: {
		RTMP_STRING wapp_support_ver[16];
		int ret;

		wrq->u.data.length = strlen(WAPP_SUPPORT_VERSION);
		ret = snprintf(&wapp_support_ver[0], sizeof(wapp_support_ver), "%s", WAPP_SUPPORT_VERSION);
		if (os_snprintf_error(sizeof(driverVersion), ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			break;
		}

		wapp_support_ver[wrq->u.data.length] = '\0';

		Status = copy_to_user(wrq->u.data.pointer, wapp_support_ver, wrq->u.data.length);
	}
	break;
#endif /* WAPP_SUPPORT */
#ifdef WIFI_SPECTRUM_SUPPORT

	case OID_802_11_WIFISPECTRUM_GET_CAPTURE_BW: {
		UINT32 CapNode;
		UCHAR CapBw;

		CapNode = Get_System_CapNode_Info(pAd);
		CapBw = Get_System_Bw_Info(pAd, CapNode);
		wrq->u.data.length = sizeof(UCHAR);
		Status = copy_to_user(wrq->u.data.pointer, &CapBw, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Query::OID_802_11_WIFISPECTRUM_GET_CAPTURE_BW CapBw = %d\n", CapBw);
	}
	break;

	case OID_802_11_WIFISPECTRUM_GET_CENTRAL_FREQ: {
		UINT32 CapNode;
		USHORT CenFreq;

		CapNode = Get_System_CapNode_Info(pAd);
		CenFreq = Get_System_CenFreq_Info(pAd, CapNode);
		wrq->u.data.length = sizeof(USHORT);
		Status = copy_to_user(wrq->u.data.pointer, &CenFreq, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Query::OID_802_11_WIFISPECTRUM_GET_CENTRAL_FREQ CenFreq = %d\n", CenFreq);
	}
	break;
#endif /* WIFI_SPECTRUM_SUPPORT */
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
		case OID_GET_SCAN_RESULT:
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Receive GET_SCAN_RESULT command from Userspace\n");
#ifdef APCLI_SUPPORT
			if (pObj->ioctl_if_type == INT_APCLI) {
				if (ifIndex >= MAX_MULTI_STA) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"error station index\n");
					return FALSE;
				}

				wdev = &pAd->StaCfg[ifIndex].wdev;
				apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;
				if (!apcliEn)
					return FALSE;
			} else
#endif /* APCLI_SUPPORT */
			{
				if (!VALID_MBSS(pAd, ifIndex)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
					return FALSE;
				}

				wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			}
			if (scan_in_run_state(pAd, wdev)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"\n\n\nQuery::!!!SCANNING, CANNOT GET SCAN RESULT NOW!!!\n\n\n");
				/*#define	EBUSY		16*/
				/* Device or resource busy */
				Status = -EBUSY;
				break;
			}
			Status = RTMPIoctlQueryScanResult(pAd, wrq);
			break;
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

#ifdef MT_DFS_SUPPORT
		case OID_DFS_ZERO_WAIT:
			Status = ZeroWaitDfsQueryCmdHandler(pAd, wrq);
			break;
#endif
	case OID_QUERY_FEATURE_SUP_LIST:
		Status = copy_to_user(wrq->u.data.pointer, &pAd->wifi_cap_list, wrq->u.data.length);
		break;
#if defined(CONFIG_BS_SUPPORT) || defined(CONFIG_MAP_SUPPORT)
	case OID_GET_WSC_PROFILES:
		{
			int TotalLen = 0;
			wsc_apcli_config_msg *wsc_config = NULL;

			ifIndex = pObj->ioctl_if;
			TotalLen = Fill_OID_WSC_PROFILE(pAd, ifIndex, &wsc_config);
			if (!wsc_config) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"wsc_config is NULL return FALSE\n");
				return FALSE;
			}
			wrq->u.data.length = TotalLen;
			Status = copy_to_user(wrq->u.data.pointer, wsc_config, wrq->u.data.length);
			if (wsc_config)
				os_free_mem((PUCHAR)wsc_config);
			break;
		}
#ifdef DPP_SUPPORT
	case OID_802_11_GET_DPP_FRAME:
		{
			if (pAd->bDppEnable) {
				UINT32 frm_id = 0;
				ifIndex = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
				/*No need to assign in case of AP, done in initialization of function*/
				if (pObj->ioctl_if_type == INT_APCLI)
					wdev = &pAd->StaCfg[ifIndex].wdev;
#endif /* APCLI_SUPPORT */
				if (wdev == NULL) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"wdev is null, return!\n");
					break;
				}
				Status = copy_from_user(&frm_id, wrq->u.data.pointer, 4);
				if (wext_send_dpp_cached_frame(wdev, frm_id, wrq) == false) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Unsuccessful send to Wapp\n");
					wrq->u.data.length = 0;
					break;
				}
			} else
				MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"DPP Disabled pleae enable in driver\n");
			break;
		}
#endif /* DPP_SUPPORT */
#ifdef MAP_R2
		case OID_GET_CAC_CAP:
		{
			struct DOT11_H *pDot11h = NULL;
			UCHAR bandIdx;

			PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

			if (wdev) {
				NdisZeroMemory(&wdev->cac_capability, sizeof(wdev->cac_capability));
				wapp_get_cac_cap(pAd, wdev, &wdev->cac_capability);
				NdisMoveMemory(&wdev->cac_capability.country_code[0],
					pAd->CommonCfg.CountryCode, 2);
				wdev->cac_capability.rdd_region = pAd->CommonCfg.RDDurRegion;

				/* CAC Ongoing Update */
				bandIdx = HcGetBandByWdev(wdev);
				pDot11h = wdev->pDot11_H;
				if (pDot11h == NULL)
					return FALSE;

				wdev->cac_capability.active_cac = FALSE;
				if (pDot11h->RDMode == RD_SILENCE_MODE) {
					wdev->cac_capability.active_cac = TRUE;
					if (pDfsParam->band_bw[bandIdx] == BW_80 ||
					pDfsParam->band_bw[bandIdx] == BW_160)
						wdev->cac_capability.ch_num =
							DfsPrimToCent(pDfsParam->PrimCh, pDfsParam->band_bw[bandIdx]);
					else
						wdev->cac_capability.ch_num = pDfsParam->PrimCh;
					if (pDot11h->cac_time > pDot11h->RDCount)
						wdev->cac_capability.remain_time =
							(pDot11h->cac_time - pDot11h->RDCount);
					else
						wdev->cac_capability.remain_time = 0;
				}
				wdev->cac_capability.cac_mode = get_cac_mode(pAd, pDfsParam, wdev);

				wrq->u.data.length = sizeof(wdev->cac_capability);
				Status = copy_to_user(wrq->u.data.pointer,
					&wdev->cac_capability, wrq->u.data.length);
			} else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid wdev for cmd %d\n", cmd);
			break;
		}
#endif

	case OID_GET_MISC_CAP:
		{
			wdev_misc_cap misc_cap;
			NdisZeroMemory(&misc_cap, sizeof(wdev_misc_cap));
			misc_cap.max_num_of_cli = 64;
			misc_cap.max_num_of_bss = 32;
			misc_cap.num_of_bss = pAd->ApCfg.BssidNum;
			misc_cap.max_num_of_block_cli = BLOCK_LIST_NUM;
			wrq->u.data.length = sizeof(wdev_misc_cap);
			Status = copy_to_user(wrq->u.data.pointer, &misc_cap, wrq->u.data.length);
			break;
		}
	case OID_GET_HT_CAP:
		{
			wdev_ht_cap ht_cap;
			NdisZeroMemory(&ht_cap, sizeof(wdev_ht_cap));
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			ht_cap.tx_stream = wlan_config_get_tx_stream(wdev);
			ht_cap.rx_stream = wlan_config_get_rx_stream(wdev);
			ht_cap.sgi_20 = (wlan_config_get_ht_gi(wdev) == GI_400) ? 1:0;
			ht_cap.sgi_40 = (wlan_config_get_ht_gi(wdev) == GI_400) ?  1:0;
			ht_cap.ht_40 = (wlan_operate_get_ht_bw(wdev) == BW_40) ? 1:0;
			wrq->u.data.length = sizeof(wdev_ht_cap);
			Status = copy_to_user(wrq->u.data.pointer, &ht_cap, wrq->u.data.length);
			break;
		}
	case OID_GET_VHT_CAP:
		{
			wdev_vht_cap vht_cap;
			VHT_CAP_INFO drv_vht_cap;
			VHT_OP_IE drv_vht_op;
			struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			NdisZeroMemory(&vht_cap, sizeof(wdev_vht_cap));
			NdisZeroMemory(&drv_vht_cap, sizeof(VHT_CAP_INFO));
			NdisZeroMemory(&drv_vht_op, sizeof(VHT_OP_IE));
			NdisCopyMemory(&drv_vht_cap, &pAd->CommonCfg.vht_cap_ie.vht_cap, sizeof(VHT_CAP_INFO));
			if (wdev->wdev_type != WDEV_TYPE_AP) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is not an AP\n");
				break;
			}
			mt_WrapSetVHTETxBFCap(pAd, wdev, &drv_vht_cap);
			drv_vht_op.basic_mcs_set.mcs_ss1 = VHT_MCS_CAP_NA;
			drv_vht_op.basic_mcs_set.mcs_ss2 = VHT_MCS_CAP_NA;
			drv_vht_op.basic_mcs_set.mcs_ss3 = VHT_MCS_CAP_NA;
			drv_vht_op.basic_mcs_set.mcs_ss4 = VHT_MCS_CAP_NA;
			drv_vht_op.basic_mcs_set.mcs_ss5 = VHT_MCS_CAP_NA;
			drv_vht_op.basic_mcs_set.mcs_ss6 = VHT_MCS_CAP_NA;
			drv_vht_op.basic_mcs_set.mcs_ss7 = VHT_MCS_CAP_NA;
			drv_vht_op.basic_mcs_set.mcs_ss8 = VHT_MCS_CAP_NA;
			switch	(wlan_operate_get_rx_stream(wdev)) {
			case 4:
				drv_vht_op.basic_mcs_set.mcs_ss4 = cap->mcs_nss.max_vht_mcs;
				/* fall through */
			case 3:
				drv_vht_op.basic_mcs_set.mcs_ss3 = cap->mcs_nss.max_vht_mcs;
				/* fall through */
			case 2:
				drv_vht_op.basic_mcs_set.mcs_ss2 = cap->mcs_nss.max_vht_mcs;
				/* fall through */
			case 1:
				drv_vht_op.basic_mcs_set.mcs_ss1 = cap->mcs_nss.max_vht_mcs;
				break;
			}
			NdisMoveMemory(vht_cap.sup_tx_mcs,
							&drv_vht_op.basic_mcs_set,
							sizeof(vht_cap.sup_tx_mcs));
			NdisMoveMemory(vht_cap.sup_rx_mcs,
							&drv_vht_op.basic_mcs_set,
							sizeof(vht_cap.sup_rx_mcs));
			vht_cap.tx_stream = wlan_config_get_tx_stream(wdev);
			vht_cap.rx_stream = wlan_config_get_tx_stream(wdev);
			vht_cap.sgi_80 = (wlan_config_get_ht_gi(wdev) == GI_400) ? 1:0;
			vht_cap.sgi_160 = (wlan_config_get_ht_gi(wdev) == GI_400) ? 1:0;
			vht_cap.vht_160 = (wlan_operate_get_vht_bw(wdev) == VHT_BW_160) ? 1:0;
			vht_cap.vht_8080 = (wlan_operate_get_vht_bw(wdev) == VHT_BW_8080) ? 1:0;
			vht_cap.su_bf = (drv_vht_cap.bfer_cap_su) ? 1:0;
			vht_cap.mu_bf = (drv_vht_cap.bfer_cap_mu) ? 1:0;
			wrq->u.data.length = sizeof(wdev_vht_cap);
			Status = copy_to_user(wrq->u.data.pointer, &vht_cap, wrq->u.data.length);
			break;
		}
	case OID_GET_CHAN_LIST:
		{
			int i = 0;
			UCHAR BandIdx = 0;
			CHANNEL_CTRL *pChCtrl = NULL;
			struct wifi_dev *wdev;
			wdev_chn_info *chn_list;

			os_alloc_mem(pAd, (UCHAR **)&chn_list, sizeof(wdev_chn_info));
			if (chn_list == NULL)
				break;
			NdisZeroMemory(chn_list, sizeof(wdev_chn_info));

			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			if (wdev->wdev_type != WDEV_TYPE_AP) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is not an AP\n");
				os_free_mem(chn_list);
				break;
			}
			BandIdx = HcGetBandByWdev(wdev);
			pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(sync) Msg received!\n");
			pAd->ChannelListNum = pChCtrl->ChListNum;
			for (i = 0; i < pChCtrl->ChListNum; i++) {
				pAd->ChannelList[i].Channel = pChCtrl->ChList[i].Channel;
				pAd->ChannelList[i].DfsReq = pChCtrl->ChList[i].DfsReq;
			}
			chn_list->band = wdev->PhyMode;
			chn_list->op_ch = wlan_operate_get_prim_ch(wdev);
			chn_list->op_class = get_regulatory_class(pAd, wdev->channel, wdev->PhyMode, wdev);
			chn_list->ch_list_num = pAd->ChannelListNum;
			chn_list->dl_mcs = wdev->HTPhyMode.field.MCS;
			setChannelList(pAd, wdev, chn_list);
#ifdef CONFIG_MAP_SUPPORT
			chn_list->non_op_chn_num = getNonOpChnNum(pAd, wdev, chn_list->op_class);
			setNonOpChnList(pAd,
							wdev,
							chn_list->non_op_ch_list,
							chn_list->op_class,
							chn_list->non_op_chn_num);

			setAutoChannelSkipList(pAd, wdev, chn_list);
#endif /* CONFIG_MAP_SUPPORT */
			wrq->u.data.length = sizeof(wdev_chn_info);
			Status = copy_to_user(wrq->u.data.pointer, chn_list, wrq->u.data.length);
			os_free_mem(chn_list);
			break;
		}
#ifdef MAP_6E_SUPPORT
	case OID_GET_OP_CLASS:
		{
			wdev_op_class_info_ext *op_class;

			os_alloc_mem(pAd, (UCHAR **)&op_class, sizeof(wdev_op_class_info_ext));
			if (op_class == NULL)
				break;
			NdisZeroMemory(op_class, sizeof(wdev_op_class_info_ext));

			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			if (wdev->wdev_type != WDEV_TYPE_AP) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is not an AP\n");
				os_free_mem(op_class);
				break;
			}
#ifdef CONFIG_MAP_SUPPORT
			if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G))
				op_class->num_of_op_class = map_set_op_class_info_6g(pAd, wdev, op_class);
			else
				op_class->num_of_op_class = map_set_op_class_info(pAd, wdev, op_class);
#endif /* CONFIG_MAP_SUPPORT */
			wrq->u.data.length = sizeof(wdev_op_class_info_ext);
			Status = copy_to_user(wrq->u.data.pointer, op_class, wrq->u.data.length);
			os_free_mem(op_class);
			break;
		}
#else
	case OID_GET_OP_CLASS:
		{
			wdev_op_class_info *op_class;

			os_alloc_mem(pAd, (UCHAR **)&op_class, sizeof(wdev_op_class_info));
			if (op_class == NULL)
				break;
			NdisZeroMemory(op_class, sizeof(wdev_op_class_info));

			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			if (wdev->wdev_type != WDEV_TYPE_AP) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is not an AP\n");
				os_free_mem(op_class);
				break;
			}
#ifdef CONFIG_MAP_SUPPORT
			op_class->num_of_op_class = map_set_op_class_info(pAd, wdev, op_class);
#endif /* CONFIG_MAP_SUPPORT */
			wrq->u.data.length = sizeof(wdev_op_class_info);
			Status = copy_to_user(wrq->u.data.pointer, op_class, wrq->u.data.length);
			os_free_mem(op_class);
			break;
		}
#endif
	case OID_GET_BSS_INFO:
		{
			wdev_bss_info *bss_info;

			os_alloc_mem(pAd, (UCHAR **)&bss_info, sizeof(wdev_bss_info));
			if (bss_info == NULL)
				break;
			NdisZeroMemory(bss_info, sizeof(wdev_bss_info));

			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			if (wdev->wdev_type != WDEV_TYPE_AP) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is not an AP\n");
				os_free_mem(bss_info);
				break;
			}
			bss_info->SsidLen = pAd->ApCfg.MBSSID[ifIndex].SsidLen;
			NdisMoveMemory(bss_info->ssid,
					pAd->ApCfg.MBSSID[ifIndex].Ssid,
					(MAX_LEN_OF_SSID+1));
			NdisMoveMemory(bss_info->bssid, wdev->bssid, MAC_ADDR_LEN);
			NdisMoveMemory(bss_info->if_addr, wdev->if_addr, MAC_ADDR_LEN);
#ifdef CONFIG_MAP_SUPPORT
			bss_info->map_role = wdev->MAPCfg.DevOwnRole;
			bss_info->auth_mode = pAd->ApCfg.MBSSID[ifIndex].wdev.SecConfig.AKMMap;
			bss_info->enc_type = pAd->ApCfg.MBSSID[ifIndex].wdev.SecConfig.PairwiseCipher;
#ifdef WSC_AP_SUPPORT
			bss_info->key_len = strlen(pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.WpaPsk);
			NdisMoveMemory(bss_info->key,
				pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.WpaPsk, bss_info->key_len);
#else
			bss_info->key_len = strlen(pAd->ApCfg.MBSSID[ifIndex].PSK);
			NdisMoveMemory(bss_info->key, pAd->ApCfg.MBSSID[ifIndex].PSK, bss_info->key_len);
#endif
			bss_info->hidden_ssid = pAd->ApCfg.MBSSID[ifIndex].bHideSsid;
#endif
			wrq->u.data.length = sizeof(wdev_bss_info);
			Status = copy_to_user(wrq->u.data.pointer, bss_info, wrq->u.data.length);
			os_free_mem(bss_info);
			break;
		}
	case OID_GET_AP_METRICS:
		{
			wdev_ap_metric *ap_metric;
			BSS_STRUCT *mbss;

			os_alloc_mem(pAd, (UCHAR **)&ap_metric, sizeof(wdev_ap_metric));
			if (ap_metric == NULL)
				break;
			NdisZeroMemory(ap_metric, sizeof(wdev_ap_metric));

			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			if (wdev->wdev_type != WDEV_TYPE_AP) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is not an AP\n");
				os_free_mem(ap_metric);
				break;
			}
			mbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
			NdisMoveMemory(ap_metric->bssid, wdev->bssid, MAC_ADDR_LEN);
			ap_metric->cu = get_channel_utilization(pAd, ifIndex);
			NdisCopyMemory(ap_metric->ESPI_AC[ESPI_BE], mbss->ESPI_AC_BE, sizeof(mbss->ESPI_AC_BE));
			NdisCopyMemory(ap_metric->ESPI_AC[ESPI_BK], mbss->ESPI_AC_BK, sizeof(mbss->ESPI_AC_BK));
			NdisCopyMemory(ap_metric->ESPI_AC[ESPI_VO], mbss->ESPI_AC_VO, sizeof(mbss->ESPI_AC_VO));
			NdisCopyMemory(ap_metric->ESPI_AC[ESPI_VI], mbss->ESPI_AC_VI, sizeof(mbss->ESPI_AC_VI));
			wrq->u.data.length = sizeof(wdev_ap_metric);
			Status = copy_to_user(wrq->u.data.pointer, ap_metric, wrq->u.data.length);
			os_free_mem(ap_metric);
			break;
			}
	case OID_GET_NOP_CHANNEL_LIST:
		{
			struct nop_channel_list_s *nop_channels;

			os_alloc_mem(pAd, (UCHAR **)&nop_channels, sizeof(struct nop_channel_list_s));
			if (nop_channels == NULL)
				break;
			NdisZeroMemory(nop_channels, sizeof(struct nop_channel_list_s));

			wapp_prepare_nop_channel_list(pAd, nop_channels);
			Status = copy_to_user(wrq->u.data.pointer, nop_channels, wrq->u.data.length);

			os_free_mem(nop_channels);
			break;
		}
#ifdef DOT11_HE_AX
#ifdef MAP_R3
	case OID_WF6_CAPABILITY:
		{
			wdev_wf6_cap_roles wf6_cap;
			UINT16 i = 0;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WF6:Driver: OID_WF6_CAPABILITY is called from WAPP in driver\n");
			wrq->u.data.length = sizeof(wdev_wf6_cap_roles);
			NdisZeroMemory(&wf6_cap, sizeof(wdev_wf6_cap_roles));
			if (IF_COMBO_HAVE_AP_STA(pAd)) {
				wf6_cap.role_supp = 2;
			} else {
				wf6_cap.role_supp = 1;
			}
#define WF6_AP_INFO 0
#define WF6_APCLI_INFO 1
			/* Role supported as 1 when previously used*/

			for (i = 0; i < wf6_cap.role_supp; i++) {
				UINT16 he_max_mcs_nss = 0, *he_mcs_pos = NULL, he_mcs_len = 0;
				struct mcs_nss_caps *mcs_nss = NULL;
				UINT8 he_bw = 0;
				struct he_bf_info he_bf_struct;
				UCHAR band_idx = 0;
#ifdef APCLI_SUPPORT
				UCHAR b_idx = 0;
#endif
				wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
				if (!wdev) {
					wf6_cap.role_supp = 1;
					continue;
				}
				if (!wdev->pHObj) {
					wf6_cap.role_supp = 1;
					continue;
				}
#ifdef APCLI_SUPPORT
				b_idx = HcGetBandByWdev(wdev);
				if ((i == 1) && (pAd->ApCfg.ApCliInfRunned))
					wdev = &pAd->StaCfg[b_idx].wdev;
				else if (i == 1) {
					wf6_cap.role_supp = 1;
					continue;
				}
				if (!wdev) {
					wf6_cap.role_supp = 1;
					continue;
				}
				if (!wdev->pHObj) {
					wf6_cap.role_supp = 1;
					continue;
				}
#endif
				mcs_nss = wlan_config_get_mcs_nss_caps(wdev);
				he_bw = wlan_config_get_he_bw(wdev);

				wf6_cap.wf6_role[i].tx_stream = wlan_config_get_tx_stream(wdev);
				wf6_cap.wf6_role[i].rx_stream = wlan_config_get_rx_stream(wdev);

				he_mcs_pos = (UINT16 *)wf6_cap.wf6_role[i].he_mcs;

				he_max_mcs_nss = he_mcs_map(wf6_cap.wf6_role[i].tx_stream, HE_MCS_0_11);
				NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
				he_mcs_pos++;

				he_max_mcs_nss = he_mcs_map(wf6_cap.wf6_role[i].rx_stream, HE_MCS_0_11);
				NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
				he_mcs_pos++;

				he_mcs_len += 4;

					if (he_bw > HE_BW_80
#ifdef MAP_R3
						|| ((IS_MAP_ENABLE(pAd)
							&& IS_MAP_CERT_ENABLE(pAd)
							&& IS_MAP_R3_ENABLE(pAd)))
#endif

					) {
					if (wf6_cap.wf6_role[i].tx_stream > mcs_nss->bw160_max_nss)
						wf6_cap.wf6_role[i].tx_stream = mcs_nss->bw160_max_nss;
					if (wf6_cap.wf6_role[i].rx_stream > mcs_nss->bw160_max_nss)
						wf6_cap.wf6_role[i].rx_stream = mcs_nss->bw160_max_nss;

					he_max_mcs_nss = he_mcs_map(wf6_cap.wf6_role[i].tx_stream, HE_MCS_0_11);
					NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
					he_mcs_pos++;

					he_max_mcs_nss = he_mcs_map(wf6_cap.wf6_role[i].rx_stream, HE_MCS_0_11);
					NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
					he_mcs_pos++;

					he_mcs_len += 4;
					wf6_cap.wf6_role[i].he_160 = 1;

					if (he_bw > HE_BW_160
#ifdef MAP_R3
						|| ((IS_MAP_ENABLE(pAd)
						&& IS_MAP_CERT_ENABLE(pAd)
						&& IS_MAP_R3_ENABLE(pAd)))
#endif
					) {
						he_max_mcs_nss = he_mcs_map(wf6_cap.wf6_role[i].tx_stream, HE_MCS_0_11);
						NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
						he_mcs_pos++;

						he_max_mcs_nss = he_mcs_map(wf6_cap.wf6_role[i].rx_stream, HE_MCS_0_11);
						NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
						he_mcs_pos++;

						he_mcs_len += 4;
						wf6_cap.wf6_role[i].he_8080 = 1;
					 }
				 }

				wf6_cap.wf6_role[i].he_mcs_len = he_mcs_len;

				NdisZeroMemory(&he_bf_struct, sizeof(struct he_bf_info));
				mt_wrap_get_he_bf_cap(wdev, &he_bf_struct);

				if (he_bf_struct.bf_cap & HE_SU_BFER)
					wf6_cap.wf6_role[i].su_bf_cap = 1;
				if (he_bf_struct.bf_cap & HE_MU_BFER)
					wf6_cap.wf6_role[i].mu_bf_cap = 1;
				if (wlan_config_get_mu_dl_mimo(wdev)) {
					if (wdev->wdev_type == WDEV_TYPE_AP)
						wf6_cap.wf6_role[i].dl_mu_mimo_ofdma_cap = 1;
				}
				if (wlan_config_get_mu_ul_mimo(wdev)) {
					if (wdev->wdev_type == WDEV_TYPE_AP) {
						wf6_cap.wf6_role[i].ul_mu_mimo_ofdma_cap = 1;
						wf6_cap.wf6_role[i].ul_mu_mimo_cap = 1;
					}
				}
				/* agent_role = 0 for AP, agent_role = 1 for APCLI */
				wf6_cap.wf6_role[i].agent_role = i;
				wf6_cap.wf6_role[i].su_beamformee_status = 0;
				wf6_cap.wf6_role[i].beamformee_sts_less80 = 0;
				wf6_cap.wf6_role[i].beamformee_sts_more80 = 0;
				wf6_cap.wf6_role[i].max_user_dl_tx_mu_mimo = 0;
				wf6_cap.wf6_role[i].max_user_ul_rx_mu_mimo = 0;
				wf6_cap.wf6_role[i].max_user_dl_tx_ofdma = 0;
				wf6_cap.wf6_role[i].max_user_ul_rx_ofdma = 0;
				wf6_cap.wf6_role[i].rts_status = 0;
				wf6_cap.wf6_role[i].mu_rts_status = 0; /* Not Supported */
				wf6_cap.wf6_role[i].m_bssid_status = 0;
				wf6_cap.wf6_role[i].mu_edca_status = 0;
				wf6_cap.wf6_role[i].twt_requester_status = 0;
				wf6_cap.wf6_role[i].twt_responder_status = 0;
				wf6_cap.wf6_role[i].ul_ofdma_cap = 0;
				wf6_cap.wf6_role[i].dl_ofdma_cap = 0;
#ifdef HE_TXBF_SUPPORT
				if (he_bf_struct.bf_cap & HE_SU_BFEE)
					wf6_cap.wf6_role[i].su_beamformee_status = 1;

				if (he_bf_struct.bfee_sts_le_eq_bw80)
					wf6_cap.wf6_role[i].beamformee_sts_less80 = 1;

				if (he_bf_struct.bfee_sts_gt_bw80)
					wf6_cap.wf6_role[i].beamformee_sts_more80 = 1;
#endif /*HE_TXBF_SUPPORT*/
#ifdef WIFI_TWT_SUPPORT
				if (wlan_config_get_asic_twt_caps(wdev) &&
						(TWT_SUPPORT_ITWT(wlan_config_get_he_twt_support(wdev)))) {
					if (i == WF6_AP_INFO) {
						wf6_cap.wf6_role[i].twt_requester_status = 0;
						wf6_cap.wf6_role[i].twt_responder_status = 1;
					} else {
						wf6_cap.wf6_role[i].twt_requester_status = 1;
						wf6_cap.wf6_role[i].twt_responder_status = 0;
					}
				}
#endif /*WIFI_TWT_SUPPORT*/

				if (wlan_config_get_he_mu_edca(wdev))
					wf6_cap.wf6_role[i].mu_edca_status = 1;

				if (wlan_config_get_mu_dl_ofdma(wdev) &&
					wdev->wdev_type == WDEV_TYPE_AP) {
					wf6_cap.wf6_role[i].dl_ofdma_cap = 1;
					wf6_cap.wf6_role[i].max_user_dl_tx_ofdma = wlan_config_get_ofdma_user_cnt(wdev);
#ifdef CFG_SUPPORT_FALCON_MURU
					if (!wf6_cap.wf6_role[i].max_user_dl_tx_ofdma) {
					/*iwpriv ra0 set set_muru_manual_config=ul_comm_user_cnt:*/
						wf6_cap.wf6_role[i].max_user_dl_tx_ofdma = (UINT8)(pAd->CommonCfg.HE_OfdmaUserNum);
					}
#endif
				}

				if (wlan_config_get_mu_ul_ofdma(wdev) &&
						wdev->wdev_type == WDEV_TYPE_AP) {
					wf6_cap.wf6_role[i].ul_ofdma_cap = 1;
					wf6_cap.wf6_role[i].max_user_ul_rx_ofdma = wlan_config_get_ofdma_user_cnt(wdev);
#ifdef CFG_SUPPORT_FALCON_MURU
					if (!wf6_cap.wf6_role[i].max_user_ul_rx_ofdma) {
						wf6_cap.wf6_role[i].max_user_ul_rx_ofdma = (UINT8)(pAd->CommonCfg.HE_OfdmaUserNum);
					}
#endif
				}

#ifdef MBSS_SUPPORT
				if (MAX_MBSSID_NUM(pAd) > 1)
					wf6_cap.wf6_role[i].m_bssid_status = 1;
#endif /*MBSS_SUPPORT*/

				band_idx = HcGetBandByChannelRange(pAd, wdev->channel);
				if (pAd->mcli_ctl[band_idx].c2s_only == TRUE)
					wf6_cap.wf6_role[i].rts_status = 1;

				if (wlan_config_get_mu_dl_mimo(wdev)) {
					if (wdev->wdev_type == WDEV_TYPE_AP)
						wf6_cap.wf6_role[i].max_user_dl_tx_mu_mimo = wf6_cap.wf6_role[i].max_user_dl_tx_ofdma;
				}
				if (wlan_config_get_mu_ul_mimo(wdev)) {
					if (wdev->wdev_type == WDEV_TYPE_AP) {
						wf6_cap.wf6_role[i].max_user_ul_rx_mu_mimo = wf6_cap.wf6_role[i].max_user_ul_rx_ofdma;
					}
				}
			}

			Status = copy_to_user(wrq->u.data.pointer, &wf6_cap, wrq->u.data.length);
			break;
		}
#endif
	case OID_GET_HE_CAP:
		{
			wdev_he_cap he_cap;
			UINT16 he_max_mcs_nss, *he_mcs_pos, he_mcs_len = 0;
			UINT8 he_bw;
			struct he_bf_info he_bf_struct;
			struct mcs_nss_caps *mcs_nss;
			if (wdev) {
				mcs_nss = wlan_config_get_mcs_nss_caps(wdev);
				wrq->u.data.length = sizeof(wdev_he_cap);
				wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
				he_bw = wlan_config_get_he_bw(wdev);
				NdisZeroMemory(&he_cap, sizeof(wdev_he_cap));
				he_cap.tx_stream = wlan_config_get_tx_stream(wdev);
				he_cap.rx_stream = wlan_config_get_rx_stream(wdev);
				he_mcs_pos = (UINT16 *)he_cap.he_mcs;
				he_max_mcs_nss = he_mcs_map(he_cap.tx_stream, HE_MCS_0_11);
				NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
				he_mcs_pos++;
				he_max_mcs_nss = he_mcs_map(he_cap.rx_stream, HE_MCS_0_11);
				NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
				he_mcs_pos++;
				he_mcs_len += 4;
				if (he_bw > HE_BW_80) {
					if (he_cap.tx_stream > mcs_nss->bw160_max_nss)
						he_cap.tx_stream = mcs_nss->bw160_max_nss;
					if (he_cap.rx_stream > mcs_nss->bw160_max_nss)
						he_cap.rx_stream = mcs_nss->bw160_max_nss;
					he_max_mcs_nss = he_mcs_map(he_cap.tx_stream, HE_MCS_0_11);
					NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
					he_mcs_pos++;
					he_max_mcs_nss = he_mcs_map(he_cap.rx_stream, HE_MCS_0_11);
					NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
					he_mcs_pos++;
					he_mcs_len += 4;
					he_cap.he_160 = 1;
					if (he_bw > HE_BW_160) {
						he_max_mcs_nss = he_mcs_map(he_cap.tx_stream, HE_MCS_0_11);
						NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
						he_mcs_pos++;
						he_max_mcs_nss = he_mcs_map(he_cap.rx_stream, HE_MCS_0_11);
						NdisMoveMemory(he_mcs_pos, &he_max_mcs_nss, sizeof(UINT16));
						he_mcs_pos++;
						he_mcs_len += 4;
						he_cap.he_8080 = 1;
					}
				}
				he_cap.he_mcs_len = he_mcs_len;


				NdisZeroMemory(&he_bf_struct, sizeof(struct he_bf_info));
				mt_wrap_get_he_bf_cap(wdev, &he_bf_struct);

				if (he_bf_struct.bf_cap & HE_SU_BFER)
					he_cap.su_bf_cap = 1;
				if (he_bf_struct.bf_cap & HE_MU_BFER)
					he_cap.mu_bf_cap = 1;
				if (wlan_config_get_mu_dl_mimo(wdev)) {
					if (wdev->wdev_type == WDEV_TYPE_AP)
						he_cap.dl_mu_mimo_ofdma_cap = 1;
					he_cap.dl_ofdma_cap = 1; /*To see*/
				}
				if (wlan_config_get_mu_ul_mimo(wdev)) {
					if (wdev->wdev_type == WDEV_TYPE_AP) {
						he_cap.ul_mu_mimo_ofdma_cap = 1;
						he_cap.ul_mu_mimo_cap = 1;
						he_cap.ul_ofdma_cap = 1; /*To see*/
					}
				}
				he_cap.gi = wlan_config_get_he_gi(wdev);
				Status = copy_to_user(wrq->u.data.pointer, &he_cap, wrq->u.data.length);
			} else
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid wdev for cmd %d\n", cmd);

			break;
		}
#endif /*DOT11_HE_AX*/

	case OID_GET_WMODE:
	{
		UCHAR wmode;

		if (wdev) {
			wmode = wdev->PhyMode;
			wmode = wmode_2_cfgmode(wmode);
			wrq->u.data.length = sizeof(wmode);
			Status = copy_to_user(wrq->u.data.pointer, &wmode, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"OID_GET_WMODE, intf %s  wmode=%d\n", wdev->if_dev->name, wmode);
		}
		break;
	}
#endif
	case OID_GET_WIRELESS_BAND:
	{
		UCHAR wireless_band;

		if (wdev) {
			wireless_band = wlan_operate_get_ch_band(wdev);
			wrq->u.data.length = sizeof(wireless_band);
			Status = copy_to_user(wrq->u.data.pointer, &wireless_band, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"OID_GET_WIRELESS_BAND, intf %s wireless_band=%d\n", wdev->if_dev->name, wireless_band);
		}
		break;
	}

#if defined(CONFIG_6G_SUPPORT) && defined(BSSMGR_CROSS_MODULE_SUPPORT)
	case OID_BSS_MGMT_GET_EVENT:
	{
		UCHAR *Buf = NULL;
		int ret;
		struct bmg_entry bmgentry;

		Status = copy_from_user(&bmgentry, wrq->u.data.pointer, sizeof(struct bmg_entry));

		ret = bssmnger_get_event_handle(pAd, &Buf, bmgentry.devinfo.ifname);

		if (ret == NDIS_STATUS_SUCCESS && Buf)
			Status = copy_to_user(wrq->u.data.pointer, Buf, wrq->u.data.length);
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"OID_BSS_MGMT_GET_EVENT bss entry Not Found\n");
		break;
	}
#endif /* CONFIG_6G_SUPPORT && BSSMGR_CROSS_MODULE_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
#ifdef MAP_6E_SUPPORT
	case OID_GET_TX_PWR:
	{
		wdev_tx_power tx_pwr;
		wdev_op_class_info_ext *op_class;
		UINT8 op = 0, ch = 0, pwr_num = 0;
		UINT8 channel_set_num = 0, MaxTxPwr = 0, ChSetMinLimPwr;
		PUCHAR channel_set = NULL;
		os_alloc_mem(pAd, (UCHAR **)&op_class, sizeof(wdev_op_class_info_ext));
		if (op_class == NULL)
			break;
		NdisZeroMemory(op_class, sizeof(wdev_op_class_info_ext));

		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		if (wdev->wdev_type != WDEV_TYPE_AP) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is not an AP\n");
			os_free_mem(op_class);
			break;
		}
#ifdef CONFIG_MAP_SUPPORT
		if (WMODE_CAP(wdev->PhyMode, WMODE_AX_6G))
			op_class->num_of_op_class = map_set_op_class_info_6g(pAd, wdev, op_class);
		else
			op_class->num_of_op_class = map_set_op_class_info(pAd, wdev, op_class);
#endif
		tx_pwr.num_of_op_class = op_class->num_of_op_class;
		for (op = 0; op < op_class->num_of_op_class; op++) {
			if (op_class->opClassInfoExt[op].op_class == 0)
				continue;
			tx_pwr.tx_pwr_limit[pwr_num].op_class = op_class->opClassInfoExt[op].op_class;
			channel_set = get_channelset_by_reg_class(pAd, op_class->opClassInfoExt[op].op_class, wdev->PhyMode);
			channel_set_num = get_channel_set_num(channel_set);
			/* no match channel set. */
			if (channel_set == NULL)
				continue;
			/* empty channel set. */
			if (channel_set_num == 0)
				continue;
			/*
				There is many channel which have different limit tx power
				we choose the minimum
			*/
			ChSetMinLimPwr = 0xFF;
			for (ch = 0; ch < channel_set_num; ch++) {
				MaxTxPwr = GetRegulatoryMaxTxPwr(pAd, channel_set[ch], wdev);
				if (!strncmp((RTMP_STRING *) pAd->CommonCfg.CountryCode, "CN", 2))
					MaxTxPwr = pAd->MaxTxPwr;/*for CN CountryCode*/
				if (MaxTxPwr < ChSetMinLimPwr)
					ChSetMinLimPwr = MaxTxPwr;
				if (ChSetMinLimPwr == 0xff)
					ChSetMinLimPwr = pAd->MaxTxPwr;
			}
			tx_pwr.tx_pwr_limit[pwr_num].max_pwr = ChSetMinLimPwr;
			pwr_num++;
		}
		Status = copy_to_user(wrq->u.data.pointer, &tx_pwr, wrq->u.data.length);
		if (op_class)
			os_free_mem(op_class);
		break;
	}
#else
	case OID_GET_TX_PWR:
	{
		wdev_tx_power tx_pwr;
		wdev_op_class_info *op_class;
		UINT8 op = 0, ch = 0, pwr_num = 0;
		UINT8 channel_set_num = 0, MaxTxPwr = 0, ChSetMinLimPwr;
		PUCHAR channel_set = NULL;
		os_alloc_mem(pAd, (UCHAR **)&op_class, sizeof(wdev_op_class_info));
		if (op_class == NULL)
			break;
		NdisZeroMemory(op_class, sizeof(wdev_op_class_info));

		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		if (wdev->wdev_type != WDEV_TYPE_AP) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is not an AP\n");
			os_free_mem(op_class);
			break;
		}
#ifdef CONFIG_MAP_SUPPORT
		op_class->num_of_op_class = map_set_op_class_info(pAd, wdev, op_class);
#endif
		tx_pwr.num_of_op_class = op_class->num_of_op_class;
		for (op = 0; op < op_class->num_of_op_class; op++) {
			if (op_class->opClassInfo[op].op_class == 0)
				continue;
			tx_pwr.tx_pwr_limit[pwr_num].op_class = op_class->opClassInfo[op].op_class;
			channel_set = get_channelset_by_reg_class(pAd, op_class->opClassInfo[op].op_class, wdev->PhyMode);
			channel_set_num = get_channel_set_num(channel_set);
			/* no match channel set. */
			if (channel_set == NULL)
				continue;
			/* empty channel set. */
			if (channel_set_num == 0)
				continue;
			/*
				There is many channel which have different limit tx power
				we choose the minimum
			*/
			ChSetMinLimPwr = 0xFF;
			for (ch = 0; ch < channel_set_num; ch++) {
				MaxTxPwr = GetRegulatoryMaxTxPwr(pAd, channel_set[ch], wdev);
				if (!strncmp((RTMP_STRING *) pAd->CommonCfg.CountryCode, "CN", 2))
					MaxTxPwr = pAd->MaxTxPwr;/*for CN CountryCode*/
				if (MaxTxPwr < ChSetMinLimPwr)
					ChSetMinLimPwr = MaxTxPwr;
				if (ChSetMinLimPwr == 0xff)
					ChSetMinLimPwr = pAd->MaxTxPwr;
			}
			tx_pwr.tx_pwr_limit[pwr_num].max_pwr = ChSetMinLimPwr;
			pwr_num++;
		}
		Status = copy_to_user(wrq->u.data.pointer, &tx_pwr, wrq->u.data.length);
		if (op_class)
			os_free_mem(op_class);
		break;
	}
#endif /* MAP_6E_SUPPORT */
	case OID_GET_ASSOC_REQ_FRAME:
		{
			os_alloc_mem(NULL, &pStaMacAddr, MAC_ADDR_LEN);

			if (pStaMacAddr) {
				Status = copy_from_user(pStaMacAddr, wrq->u.data.pointer, MAC_ADDR_LEN);

				if (Status == NDIS_STATUS_SUCCESS) {
					PMAC_TABLE_ENTRY        pEntry = NULL;

					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"GET::(ASSOC REQ STA - "MACSTR")\n", MAC2STR(pStaMacAddr));
					pEntry = MacTableLookup(pAd, pStaMacAddr);

					if (pEntry) {
						Status = copy_to_user(wrq->u.data.pointer,
							pEntry->assoc_req_frame,
							wrq->u.data.length);
					} else
						wrq->u.data.length = 0;
				}

				os_free_mem(pStaMacAddr);
			}
			break;
		}

	case OID_GET_SRG_INFO:
		{
			struct wapp_mesh_sr_info srg_info;
			UCHAR band_idx = 0;

			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				"GET:: OID_GET_SRG_INFO\n");

			wrq->u.data.length = sizeof(srg_info);
			NdisZeroMemory(&srg_info, sizeof(srg_info));

			if (wdev) {
				band_idx = HcGetBandByWdev(wdev);
				SrMeshGetSrMode(pAd, band_idx, (PUINT_8)&srg_info.sr_mode);
				SrMeshGetSrgBitmap(pAd, band_idx, (PUINT_8)&srg_info.bm_info);
			}

			Status = copy_to_user(wrq->u.data.pointer, &srg_info, wrq->u.data.length);
			break;
		}
#ifdef TR181_SUPPORT
#ifdef VOW_SUPPORT
	case OID_802_11_VOW_ATF_EN:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Get OID_802_11_VOW_ATF_EN\n");
		wrq->u.data.length = sizeof(pAd->vow_cfg.en_airtime_fairness);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->vow_cfg.en_airtime_fairness, wrq->u.data.length);
		break;
	case OID_802_11_VOW_GROUP_MAX_RATIO:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Get OID_802_11_VOW_GROUP_MAX_RATIO, ifIndex=%d,max_airtime_ratio=%d\n",
			ifIndex, pAd->vow_bss_cfg[ifIndex].max_airtime_ratio);
		wrq->u.data.length = sizeof(pAd->vow_bss_cfg[ifIndex].max_airtime_ratio);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->vow_bss_cfg[ifIndex].max_airtime_ratio, wrq->u.data.length);
		break;
	case OID_802_11_VOW_GROUP_MIN_RATIO:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Get OID_802_11_VOW_GROUP_MIN_RATIO,ifIndex=%d,min_airtime_ratio=%d\n",
			ifIndex, pAd->vow_bss_cfg[ifIndex].min_airtime_ratio);
		wrq->u.data.length = sizeof(pAd->vow_bss_cfg[ifIndex].min_airtime_ratio);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->vow_bss_cfg[ifIndex].min_airtime_ratio, wrq->u.data.length);
		break;
#endif
#ifdef AP_SCAN_SUPPORT
	case OID_802_11_ACS_REFRESH_PERIOD:
	{
		UCHAR band_idx = 0;

		if (wdev) {
			band_idx = HcGetBandByWdev(wdev);
			wrq->u.data.length = sizeof(pAd->ApCfg.ACSCheckTime[band_idx]);
			Status = copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.ACSCheckTime[band_idx], wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Band[%d] ACS_REFRESH_PERIOD = %d\n",
				band_idx, pAd->ApCfg.ACSCheckTime[band_idx]);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid wdev for cmd %d\n", cmd);
		break;
	}
#endif
	case OID_802_11_MAX_NUM_OF_SSID:
	{
		UINT8 max_bss_num = MAX_MBSSID_NUM(pAd);

		if (wdev) {
			wrq->u.data.length = sizeof(max_bss_num);
			Status = copy_to_user(wrq->u.data.pointer, &max_bss_num, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "max_bss_num = %d\n", max_bss_num);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid wdev for cmd %d\n", cmd);
		break;
	}
	case OID_802_11_BW:
	{
		UCHAR bw;

		if (wdev) {
			bw = wlan_operate_get_bw(wdev);
			wrq->u.data.length = sizeof(bw);
			Status = copy_to_user(wrq->u.data.pointer, &bw, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "bw = %d\n", bw);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid wdev for cmd %d\n", cmd);
		break;
	}
	case OID_802_11_EXTENSION_CHANNEL:
	{
		UCHAR ext_cha;

		if (wdev) {
			/*	EXTCHA_NONE  0
				EXTCHA_ABOVE 1
				EXTCHA_BELOW 3 */
			ext_cha = wlan_config_get_ext_cha(wdev);
			wrq->u.data.length = sizeof(ext_cha);
			Status = copy_to_user(wrq->u.data.pointer, &ext_cha, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ext_cha = %d\n", ext_cha);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid wdev for cmd %d\n", cmd);
		break;
	}
	case OID_802_11_IEEE80211H:
		wrq->u.data.length = sizeof(pAd->CommonCfg.bIEEE80211H);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.bIEEE80211H, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "bIEEE80211H = %d\n",
			pAd->CommonCfg.bIEEE80211H);
		break;
	case OID_802_11_COUNTRYCODE:
		wrq->u.data.length = sizeof(pAd->CommonCfg.CountryCode);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.CountryCode, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CountryCode = %s\n",
			pAd->CommonCfg.CountryCode);
		break;
	case OID_802_11_DTIMPERIOD:
		wrq->u.data.length = sizeof(pAd->ApCfg.MBSSID[ifIndex].DtimPeriod);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->ApCfg.MBSSID[ifIndex].DtimPeriod, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DtimPeriod[%d] = %d\n", ifIndex,
			pAd->ApCfg.MBSSID[ifIndex].DtimPeriod);
		break;
	case OID_802_11_TXPREAMBLE:
		wrq->u.data.length = sizeof(pAd->CommonCfg.TxPreamble);
		/* Rt802_11PreambleLong = 1, Rt802_11PreambleShort = 0, Rt802_11PreambleAuto = 2 */
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.TxPreamble, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "TxPreamble = %ld\n",
			pAd->CommonCfg.TxPreamble);
		break;
	case OID_802_11_BASICRATE:
		wrq->u.data.length = sizeof(pAd->CommonCfg.BasicRateBitmap);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->CommonCfg.BasicRateBitmap, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BasicRateBitmap = %lx\n",
			pAd->CommonCfg.BasicRateBitmap);
		break;
	case OID_802_11_SUPPRATE:
	{
		int j;

		if (wdev) {
			wrq->u.data.length = sizeof(wdev->rate.legacy_rate.sup_rate);
			Status = copy_to_user(wrq->u.data.pointer, &wdev->rate.legacy_rate.sup_rate, wdev->rate.legacy_rate.sup_rate_len);
			for (j = 0; j < wdev->rate.legacy_rate.sup_rate_len; j++)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "sup_rate[%d] = %x\n", j,
					wdev->rate.legacy_rate.sup_rate[j]);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid wdev for cmd %d\n", cmd);
		break;
	}
	case OID_802_11_RRM:
		if (wdev) {
			wrq->u.data.length = sizeof(wdev->RrmCfg.bDot11kRRMEnable);
			Status = copy_to_user(wrq->u.data.pointer, &wdev->RrmCfg.bDot11kRRMEnable, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "bDot11kRRMEnable = %d\n",
				wdev->RrmCfg.bDot11kRRMEnable);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid wdev for cmd %d\n", cmd);
		break;
	case OID_802_11_RADIO_STATS:
	{
		WIFI_RADIO_STATS Stats;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_RADIO_STATS\n");
		NdisZeroMemory(&Stats, sizeof(Stats));
		Status = Get_Radio_Statistics(pAd, &Stats);
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"OID_802_11_RADIO_STATS error. %d\n", Status);
			break;
		}
		wrq->u.data.length = sizeof(Stats);
		Status = copy_to_user(wrq->u.data.pointer, &Stats, wrq->u.data.length);
		break;
	}
	case OID_802_11_RADIO_CHANNEL_LAST_CHANGE:
	{
		UCHAR bandIdx = BAND0;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_RADIO_CHANNEL_CHANGE_STATS\n");
		if (wdev)
			bandIdx = HcGetBandByWdev(wdev);
		Status = Get_Radio_Channel_Change_Info(pAd, bandIdx, wrq);
		break;
	}
	case OID_802_11_WIFI_DIAG_RESULT:
	{
		NBR_DIAG_RESULT result;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_WIFI_DIAG_RESULT\n");
		NdisZeroMemory(&result, sizeof(result));
		Status = copy_from_user(&result, wrq->u.data.pointer, sizeof(result));
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"OID_802_11_WIFI_DIAG_RESULT error. %d\n", Status);
			break;
		}
		Status = Get_Nbr_Diag_Result(pAd, &result);
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"OID_802_11_WIFI_DIAG_RESULT error. %d\n", Status);
			break;
		}
		wrq->u.data.length = sizeof(result);
		Status = copy_to_user(wrq->u.data.pointer, &result, wrq->u.data.length);
		break;
	}
	case OID_802_11_WIFI_DIAG_RESULT_NUM:
	{
		BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

		if (!ScanTab) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"OID_802_11_WIFI_DIAG_RESULT_NUM get scan table error.\n");
			Status = NDIS_STATUS_FAILURE;
			break;
		}
		wrq->u.data.length = sizeof(ScanTab->BssNr);
		Status = copy_to_user(wrq->u.data.pointer, &ScanTab->BssNr, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"OID_802_11_WIFI_DIAG_RESULT_NUM=%d\n", ScanTab->BssNr);
		break;
	}
	case OID_802_11_STA:
	{
		WIFI_STATION sta;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_STA\n");
		NdisZeroMemory(&sta, sizeof(sta));
		Status = copy_from_user(&sta, wrq->u.data.pointer, sizeof(sta));
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_STA copy_from_user error.\n");
			break;
		}
		if (Status == NDIS_STATUS_SUCCESS) {
			Get_Sta_Info(pAd, &sta);
			wrq->u.data.length = sizeof(sta);
			Status = copy_to_user(wrq->u.data.pointer, &sta, wrq->u.data.length);
		}
		break;
	}
	case OID_802_11_STA_STATS:
	{
		WIFI_STA_STATS staStats;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_STA_STATS\n");
		NdisZeroMemory(&staStats, sizeof(staStats));
		Status = copy_from_user(&staStats, wrq->u.data.pointer, sizeof(staStats));
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_STA_STATS copy_from_user error.\n");
			break;
		}
		Get_Sta_Stats(pAd, &staStats);
		wrq->u.data.length = sizeof(staStats);
		Status = copy_to_user(wrq->u.data.pointer, &staStats, wrq->u.data.length);
		break;
	}
	case OID_802_11_STA_RATES:
	{
		WIFI_STA_RATES staRates;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_STA_RATES\n");
		NdisZeroMemory(&staRates, sizeof(staRates));
		Status = copy_from_user(&staRates, wrq->u.data.pointer, sizeof(staRates));
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_STA_RATES copy_from_user error.\n");
			break;
		}
		Get_Sta_Rates(pAd, &staRates);
		wrq->u.data.length = sizeof(staRates);
		Status = copy_to_user(wrq->u.data.pointer, &staRates, wrq->u.data.length);
		break;
	}
	case OID_802_11_AP:
	{
		WIFI_AP ap_info;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_AP\n");
		NdisZeroMemory(&ap_info, sizeof(ap_info));
		ap_info.SSIDAdvertisementEnabled = pAd->ApCfg.MBSSID[ifIndex].bHideSsid ? FALSE : TRUE;
		ap_info.AssociatedDeviceNumberOfEntries = 0; /* not support */
		ap_info.IsolationEnable = pAd->ApCfg.MBSSID[ifIndex].IsolateInterStaTraffic;
		ap_info.MACAddressControlEnabled = pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Policy ? TRUE : FALSE;
		ap_info.MaxAllowedAssociations = pAd->ApCfg.MBSSID[ifIndex].MaxStaNum;
		ap_info.MaxAssociatedDevices = pAd->ApCfg.MBSSID[ifIndex].MaxStaNum;
		if (wdev) {
			ap_info.WMMCapability = wdev->bWmmCapable;
			ap_info.UAPSDCapability = wdev->UapsdInfo.bAPSDCapable;
		}
		wrq->u.data.length = sizeof(ap_info);
		Status = copy_to_user(wrq->u.data.pointer, &ap_info, wrq->u.data.length);
		break;
	}
	case OID_802_11_AP_ACLLIST:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_AP_ACLLIST\n");
		wrq->u.data.length = sizeof(pAd->ApCfg.MBSSID[apidx].AccessControlList);
		Status = copy_to_user(wrq->u.data.pointer,
					&pAd->ApCfg.MBSSID[apidx].AccessControlList,
					wrq->u.data.length);
		break;
	case OID_802_11_SEC_REKEYING_INTERVAL:
		if (wdev) {
			wrq->u.data.length = sizeof(wdev->SecConfig.GroupReKeyInterval);
			Status = copy_to_user(wrq->u.data.pointer,
						&wdev->SecConfig.GroupReKeyInterval,
						wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_SEC_REKEYING_INTERVAL=%ld\n",
				wdev->SecConfig.GroupReKeyInterval);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid wdev for cmd %d\n", cmd);
		break;
	case OID_802_11_SEC_MFP_CFG:
		wrq->u.data.length = sizeof(pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_MFPR);
		Status = copy_to_user(wrq->u.data.pointer,
					&pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_MFPR,
					wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_SEC_MFP_CFG=%d\n",
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_MFPR);
		break;
#ifdef WSC_AP_SUPPORT
	case OID_802_11_AP_WSCCONFMODE:
		pWscControl = Get_WscControl(pAd);
		if (!pWscControl) {
			Status = NDIS_STATUS_FAILURE;
			break;
		}
		wrq->u.data.length = sizeof(pWscControl->WscConfMode);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_AP_WSCMODE=%d\n",
			pWscControl->WscConfMode);
		Status = copy_to_user(wrq->u.data.pointer, &pWscControl->WscConfMode,
					wrq->u.data.length);
		break;
	case OID_802_11_AP_WSCMODE:
		pWscControl = Get_WscControl(pAd);
		if (!pWscControl) {
			Status = NDIS_STATUS_FAILURE;
			break;
		}
		wrq->u.data.length = sizeof(pWscControl->WscMode);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_AP_WSCMODE=%d\n",
			pWscControl->WscMode);
		Status = copy_to_user(wrq->u.data.pointer, &pWscControl->WscMode,
					wrq->u.data.length);
		break;
#endif
	case OID_802_11_AP_AC:
	{
		WIFI_DEV_AC ac[WMM_NUM_OF_AC];

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_AP_AC\n");
		wrq->u.data.length = sizeof(ac);
		NdisZeroMemory(&ac, sizeof(ac));
		Status = Get_Dev_Ac(pAd, apidx, (WIFI_DEV_AC *)&ac);
		if (Status != NDIS_STATUS_SUCCESS)
			break;
		Status = copy_to_user(wrq->u.data.pointer, &ac, wrq->u.data.length);
		break;
	}
#ifdef STAT_ENHANCE_SUPPORT
	case OID_802_11_AP_AC_STATS:
	{
		WIFI_DEV_AC_STATS ac_stats[WMM_NUM_OF_AC];

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_AP_AC_STATS\n");
		NdisZeroMemory(&ac_stats, sizeof(ac_stats));
		Status = Get_BSS_AC_Statistics(pAd, apidx, (WIFI_DEV_AC_STATS *)&ac_stats);
		if (Status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OID_802_11_STA_STATS error.\n");
			break;
		}
		wrq->u.data.length = sizeof(ac_stats);
		Status = copy_to_user(wrq->u.data.pointer, &ac_stats, wrq->u.data.length);
		break;
	}
#endif
	case OID_802_11_WIFI_SSID_STATS:
	{
		WIFI_BSS_STATS stats;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_WIFI_SSID_STATS\n");
		NdisZeroMemory(&stats, sizeof(stats));
		wrq->u.data.length = sizeof(stats);
		Status = Get_BSS_Statistics(pAd, &stats);
		if (Status != NDIS_STATUS_SUCCESS)
			break;
		Status = copy_to_user(wrq->u.data.pointer, &stats,	wrq->u.data.length);
		break;
	}
#ifdef RADIUS_ACCOUNTING_SUPPORT
	case OID_802_11_ACCOUNTING_SERVER:
	{
		WIFI_ACCOUNTING_SERVER accounting_server;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_ACCOUNTING\n");
		wrq->u.data.length = sizeof(accounting_server);
		Get_Accounting_Server(pAd, apidx, &accounting_server);
		Status = copy_to_user(wrq->u.data.pointer, &accounting_server, wrq->u.data.length);
		break;
	}
	case OID_802_11_ACCOUNTING_INTERIMINTERVAL:
		wrq->u.data.length = sizeof(pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.acct_interim_interval);
		Status = copy_to_user(wrq->u.data.pointer,
			&pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.acct_interim_interval, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_ACCOUNTING_INTERIMINTERVAL=%d\n",
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.acct_interim_interval);
		break;
#endif
#endif /* TR181_SUPPORT */

#endif /* defined(CONFIG_BS_SUPPORT) || defined(CONFIG_MAP_SUPPORT) */
#ifdef QOS_R2
	case OID_GET_PMK_BY_PEER_MAC:
	{
		os_alloc_mem(NULL, &pStaMacAddr, MAC_ADDR_LEN);
		if (pStaMacAddr) {
			Status = copy_from_user(pStaMacAddr, wrq->u.data.pointer, MAC_ADDR_LEN);
			if (Status == NDIS_STATUS_SUCCESS) {
				PMAC_TABLE_ENTRY  pEntry = NULL;

				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"get pmk of STA:"MACSTR".\n", MAC2STR(pStaMacAddr));

				pEntry = MacTableLookup(pAd, pStaMacAddr);
				if (pEntry) {
					Status = copy_to_user(wrq->u.data.pointer, pEntry->SecConfig.PMK, LEN_PMK);
					wrq->u.data.length = LEN_PMK;
				} else
					wrq->u.data.length = 0;
			}

			os_free_mem(pStaMacAddr);
		}
		break;
	}
#endif

#ifdef CCAPI_API_SUPPORT
	case OID_802_11_GET_CURRENT_CHANNEL_STATS:
		if (pAd->MsMibBucket.Enabled) {
			UCHAR band_idx = 0;
			CURRENT_CHANNEL_STATS *ChannelStats;
			band_idx = HcGetBandByWdev(wdev);
			os_alloc_mem(pAd, (UCHAR **)&ChannelStats, sizeof(CURRENT_CHANNEL_STATS));
			ChannelStats->SamplePeriod = pAd->ChannelStats.SampleDuration[band_idx];
			ChannelStats->ObssTime = pAd->ChannelStats.OBSSAirtime[band_idx];
			ChannelStats->EdCcaBusyTime = pAd->ChannelStats.EDCCAtime[band_idx];
			/* Channel busy = (MyTxTime-TxOpInitTime)+MyRxTime+OBSSTime+EDCCA Time*/
			ChannelStats->ChannelBusyTime = (pAd->ChannelStats.MyTxAirtime[band_idx] - pAd->ChannelStats.TxOpInitTime[band_idx]) +
											pAd->ChannelStats.MyRxAirtime[band_idx] + pAd->ChannelStats.OBSSAirtime[band_idx] +
											pAd->ChannelStats.EDCCAtime[band_idx];
			/*ApActivity = ChannelBusyTime - OBSSTime - EDCCA Time*/
			ChannelStats->ChannelApActivity = ChannelStats->ChannelBusyTime - ChannelStats->ObssTime - ChannelStats->EdCcaBusyTime;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"SamplePeriod = %llu, ObssTime = %llu, EdCcaTime = %llu, BusyTime = %llu\n",
				ChannelStats->SamplePeriod, ChannelStats->ObssTime,
				ChannelStats->EdCcaBusyTime, ChannelStats->ChannelBusyTime);
			wrq->u.data.length = sizeof(CURRENT_CHANNEL_STATS);
			Status = copy_to_user(wrq->u.data.pointer, ChannelStats, wrq->u.data.length);
			pAd->ChannelStats.SampleDuration[band_idx] = 0;
			pAd->ChannelStats.OBSSAirtime[band_idx] = 0;
			pAd->ChannelStats.EDCCAtime[band_idx] = 0;
			pAd->ChannelStats.MyTxAirtime[band_idx] = 0;
			pAd->ChannelStats.MyRxAirtime[band_idx] = 0;
			pAd->ChannelStats.TxOpInitTime[band_idx] = 0;
			if (ChannelStats)
				os_free_mem(ChannelStats);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Radio Channel Stats monitoring is not enabled \n");
			Status = RTMP_IO_EFAULT;
		}
		break;

case OID_GET_SECURITY_TYPE:
	{
		BSS_STRUCT *pMbss;
		PUCHAR pType;
		struct security_type *SecurityType;
		os_alloc_mem(NULL, &pType, sizeof(*SecurityType));
		SecurityType = (struct security_type *)pType;
		pMbss = &pAd->ApCfg.MBSSID[ifIndex];
		SecurityType->ifindex = ifIndex;
		SecurityType->auth_mode = SecAuthModeNewToOld(pMbss->wdev.SecConfig.AKMMap);
		SecurityType->encryp_type = SecEncryModeNewToOld(pMbss->wdev.SecConfig.PairwiseCipher);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"OID_GET_SECURITY_TYPE, Auth Mode %d	Encryp Type=%d\n",
				SecurityType->auth_mode, SecurityType->encryp_type);
		wrq->u.data.length = sizeof(*SecurityType);
		Status = copy_to_user(wrq->u.data.pointer, pType, sizeof(*SecurityType));
		os_free_mem(pType);
	}
	break;

	case OID_GET_DTIM_PERIOD:
	{
		UCHAR dtimPeriod;
		/* Validation of pAd, wdev, apidx aready performed in beginning of func*/
#ifdef MBSS_DTIM_SUPPORT
		dtimPeriod = pAd->ApCfg.MBSSID[apidx].DtimPeriod;
#else
		dtimPeriod = pAd->ApCfg.DtimPeriod;
#endif
		wrq->u.data.length = sizeof(dtimPeriod);
		Status = copy_to_user(wrq->u.data.pointer, &dtimPeriod, wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"OID_GET_DTIM_PERIOD, intf %s	dtimPeriod=%d\n",
					wdev->if_dev->name, dtimPeriod);
		break;
	}

	case OID_GET_DFS_STATE:
	{
		UCHAR dfsState;
		UCHAR wmode;
		if (wdev) {
			wmode = wdev->PhyMode;
			if (WMODE_CAP_5G(wmode))
				/* Validation of pAd, wdev, apidx aready performed in beginning of func*/
				dfsState = pAd->CommonCfg.DfsParameter.bDfsEnable;
			else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"intf %s do not support DFS\n", wdev->if_dev->name);
				dfsState = 0;
			}
			wrq->u.data.length = sizeof(dfsState);
			Status = copy_to_user(wrq->u.data.pointer, &dfsState, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"OID_GET_DFS_STATE, intf %s  dfsState=%d\n",
				wdev->if_dev->name, dfsState);
		}
		break;
	}

	case OID_GET_BCN_RATE:
	{
		BCN_RATE  bcnRate;
		if (wdev) {
			bcnRate.phymode = (UINT8)wdev->eap.bcnphymode.field.MODE;
			bcnRate.mcs = (UINT8)wdev->eap.bcnphymode.field.MCS;
			wrq->u.data.length = sizeof(bcnRate);
			Status = copy_to_user(wrq->u.data.pointer, &bcnRate, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"OID_GET_BCN_RATE, intf %s  bcnRate=(%d  %d)\n",
				wdev->if_dev->name, bcnRate.phymode, bcnRate.mcs);
		}
		break;
	}

	case OID_GET_MGM_RATE:
	{
		MGM_RATE  mgmRate;
		if (wdev) {
			mgmRate.phymode = (UINT8)wdev->eap.mgmphymode.field.MODE;
			mgmRate.mcs = (UINT8)wdev->eap.mgmphymode.field.MCS;
			wrq->u.data.length = sizeof(mgmRate);
			Status = copy_to_user(wrq->u.data.pointer, &mgmRate, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"OID_GET_MGM_RATE, intf %s  mgmRate=(%d %d)\n",
				wdev->if_dev->name, mgmRate.phymode, mgmRate.mcs);
		}
		break;
	}

	case OID_GET_PASSWORD:
	{
		struct _SECURITY_CONFIG *pSecConfig = NULL;
		if (wdev) {
			pSecConfig = &wdev->SecConfig;
			wrq->u.data.length = sizeof(pSecConfig->PSK);
			Status = copy_to_user(wrq->u.data.pointer, &pSecConfig->PSK, wrq->u.data.length);
			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Password %s\n",
				pSecConfig->PSK);
		}
		break;
	}
#endif /* CCAPI_API_SUPPORT */
	case OID_802_11_BCN_TX_CNT:
	{
		UCHAR bandIdx = BAND0;
		if (wdev)
			bandIdx = HcGetBandByWdev(wdev);
		wrq->u.data.length = sizeof(pAd->BcnCheckInfo[bandIdx].totalbcncnt);
		Status = copy_to_user(wrq->u.data.pointer, &pAd->BcnCheckInfo[bandIdx].totalbcncnt,
			wrq->u.data.length);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OID_802_11_BCN_TX_CNT %d\n",
			pAd->BcnCheckInfo[bandIdx].totalbcncnt);
	}
		break;
#ifdef CONFIG_6G_AFC_SUPPORT
	case OID_GET_AFC_CONFIG:
	{
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Received get request from afc daemon\n");

		Status = afc_daemon_channel_info(pAd, wrq);
		break;
	}
#endif/* CONFIG_6G_AFC_SUPPORT */
	default:
		Status = -EOPNOTSUPP;

		if (Status == -EOPNOTSUPP) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "Query::unknown IOCTL's subcmd = 0x%08x, apidx=%d\n", cmd, apidx);
		}

		break;
	}

	return Status;
}

INT RTMPPhyState(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *pIoctlCmdStr)
{
	RTMP_STRING *value;
	INT Status = NDIS_STATUS_SUCCESS;
	UCHAR *tmp = NULL, *buf = NULL;
	BOOLEAN fgSet = FALSE;
	int i, phyStateIdx = 0;

	/* iwpriv cmd set u.data.length=1,u.data.pointer="\0" when user does not assign input param */
	if (pIoctlCmdStr->u.data.length == 1 && *pIoctlCmdStr->u.data.pointer == '\0') {
		showPhyStatHelpMsg(pIoctlCmdStr, iwprivPhyStatHelp);
		return Status;
	}
	os_alloc_mem(NULL, (UCHAR **)&buf, pIoctlCmdStr->u.data.length + 1);

	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, pIoctlCmdStr->u.data.pointer, pIoctlCmdStr->u.data.length)) {
		os_free_mem(buf);
		return -EFAULT;
	}

	/* Play safe - take care of a situation in which user-space didn't NULL terminate */
	buf[pIoctlCmdStr->u.data.length] = 0;
	/* Use tmp to parse string, because strsep() would change it */
	tmp = buf;

	/* parameter parsing */
	for (i = 0, value = rstrtok(tmp, "="); value; value = rstrtok(NULL, "="), i++) {
		switch (i) {
		case 0:
			if (strcmp(value, "read") == 0)
				fgSet = FALSE;
			else if (strcmp(value, "write") == 0)
				fgSet = TRUE;
			break;

		case 1:
			phyStateIdx = simple_strtol(value, 0, 10);
			break;

		default: {
			Status = FALSE;
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "set wrong parameters\n");
			break;
		}
		}
	}

	for (PRTMP_PRIVATE_PHT_STATE = PRTMP_PRIVATE_PHT_STATE_SUPPORT_PROC;
				PRTMP_PRIVATE_PHT_STATE->idx < PHY_STATE_CMD_NUM; PRTMP_PRIVATE_PHT_STATE++) {
		if (phyStateIdx == PRTMP_PRIVATE_PHT_STATE->idx) {
			if (!PRTMP_PRIVATE_PHT_STATE->phy_stat_proc(pAd, NULL, fgSet)) {
				/*FALSE:Set private failed then return Invalid argument */
				Status = -EINVAL;
			}
			break;  /*Exit for loop. */
		}
	}
	os_free_mem(buf);
	return Status;
}

/*
    ==========================================================================
    Description:
	Set Country Code.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryCode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	/* Check RF lock Status */
	if (chip_check_rf_lock_down(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RF lock down! Cannot config CountryCode status!\n");
		return TRUE;
	}

#ifdef EXT_BUILD_CHANNEL_LIST
	/* reset temp table status */
	pAd->CommonCfg.pChDesp = NULL;
	pAd->CommonCfg.DfsType = MAX_RD_REGION;
#endif /* EXT_BUILD_CHANNEL_LIST */

	if (strlen(arg) == 2) {
		NdisMoveMemory(pAd->CommonCfg.CountryCode, arg, 2);
		pAd->CommonCfg.bCountryFlag = TRUE;
	} else {
		NdisZeroMemory(pAd->CommonCfg.CountryCode,
					   sizeof(pAd->CommonCfg.CountryCode));
		pAd->CommonCfg.bCountryFlag = FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set_CountryCode_Proc::(bCountryFlag=%d, CountryCode=%s)\n",
			 pAd->CommonCfg.bCountryFlag, pAd->CommonCfg.CountryCode);
	return TRUE;
}

#ifdef EXT_BUILD_CHANNEL_LIST
INT Set_ChGeography_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG Geography;

	Geography = os_str_tol(arg, 0, 10);

	if (Geography <= BOTH)
		pAd->CommonCfg.Geography = Geography;
	else
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Set_ChannelGeography_Proc::(wrong setting. 0: Out-door, 1: in-door, 2: both)\n");

	pAd->CommonCfg.CountryCode[2] =
		(pAd->CommonCfg.Geography == BOTH) ? ' ' : ((pAd->CommonCfg.Geography == IDOR) ? 'I' : 'O');
	MTWF_PRINT("Set_ChannelGeography_Proc:: Geography = %s\n",
			 pAd->CommonCfg.Geography == ODOR ? "out-door" : (pAd->CommonCfg.Geography == IDOR ? "in-door" : "both"));
	/* After Set ChGeography need invoke SSID change procedural again for Beacon update. */
	/* it's no longer necessary since APStartUp will rebuild channel again. */
	/*BuildChannelListEx(pAd); */
	return TRUE;
}
#endif /* EXT_BUILD_CHANNEL_LIST */

/*
    ==========================================================================
    Description:
	Set Country String.
	This command will not work, if the field of CountryRegion in eeprom is programmed.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_CountryString_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT   index = 0;
	INT   success = TRUE;
	RTMP_STRING name_buffer[40] = {0};
	BOOLEAN IsSupport5G = HcIsRfSupport(pAd, RFIC_5GHZ);
	BOOLEAN IsSupport2G = HcIsRfSupport(pAd, RFIC_24GHZ);
	int ret;
#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif /* EXT_BUILD_CHANNEL_LIST */

	if (strlen(arg) <= 38) {
		if (strlen(arg) < 4) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Parameter of CountryString are too short!\n");
			return FALSE;
		}

		for (index = 0; index < strlen(arg); index++) {
			if ((arg[index] >= 'a') && (arg[index] <= 'z'))
				arg[index] = toupper(arg[index]);
		}

		for (index = 0; index < NUM_OF_COUNTRIES; index++) {
			NdisZeroMemory(name_buffer, sizeof(name_buffer));
			ret = snprintf(name_buffer, sizeof(name_buffer), "\"%s\"", (RTMP_STRING *) allCountry[index].pCountryName);
			if (os_snprintf_error(sizeof(name_buffer), ret)) {
				MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"final_name snprintf error!\n");
				return FALSE;
			}

			if (strncmp((RTMP_STRING *) allCountry[index].pCountryName, arg, strlen(arg)) == 0)
				break;
			else if (strncmp(name_buffer, arg, strlen(arg)) == 0)
				break;
		}

		if (index == NUM_OF_COUNTRIES)
			success = FALSE;
	} else
		success = FALSE;

	if (success == TRUE) {
		if (pAd->CommonCfg.CountryRegion & 0x80) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "parameter of CountryRegion in eeprom is programmed\n");
			success = FALSE;
		} else {
			success = FALSE;

			if (IsSupport2G) {
				if (allCountry[index].SupportGBand == TRUE) {
					pAd->CommonCfg.CountryRegion = (UCHAR) allCountry[index].RegDomainNum11G;
					success = TRUE;
				} else
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "The Country are not Support G Band Channel\n");
			}

			if (IsSupport5G) {
				if (allCountry[index].SupportABand == TRUE) {
					pAd->CommonCfg.CountryRegionForABand = (UCHAR) allCountry[index].RegDomainNum11A;
					success = TRUE;
				} else
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "The Country are not Support A Band Channel\n");
			}
		}
	}

	if (success == TRUE) {
		os_zero_mem(pAd->CommonCfg.CountryCode, sizeof(pAd->CommonCfg.CountryCode));
		os_move_mem(pAd->CommonCfg.CountryCode, allCountry[index].IsoName, 2);
		pAd->CommonCfg.CountryCode[2] = ' ';
		/* After Set ChGeography need invoke SSID change procedural again for Beacon update. */
		/* it's no longer necessary since APStartUp will rebuild channel again. */
		/*BuildChannelList(pAd); */
		pAd->CommonCfg.bCountryFlag = TRUE;
		/* if set country string, driver needs to be reset */
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Set_CountryString_Proc::(CountryString=%s CountryRegin=%d CountryCode=%s)\n",
				  allCountry[index].pCountryName, pAd->CommonCfg.CountryRegion, pAd->CommonCfg.CountryCode);
	} else
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Parameters out of range\n");

	return success;
}

#ifdef SW_CONNECT_SUPPORT
INT	Set_DummyWtbl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT ifIndex = pObj->ioctl_if;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *wdev = NULL;
	ULONG On = (USHORT) os_str_tol(arg, 0, 10);

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid ioctl_if %d\n", ifIndex);
		return FALSE;
	}

	pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
	wdev = &pMbss->wdev;

	if (On == 1)
		wdev_dummy_obj_acquire(pAd, wdev);
	else
		HcReleaseDummyObj(pAd, wdev);

	return TRUE;
}
#endif /* SW_CONNECT_SUPPORT */

/*
    ==========================================================================
    Description:
	Set SSID
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_SSID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT success = FALSE, i = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	BSS_STRUCT *pMbss = NULL;
	struct DOT11_H *pDot11h = NULL;

	if ((VALID_MBSS(pAd, pObj->ioctl_if)) && (strlen(arg) <= MAX_LEN_OF_SSID)) {
		struct wifi_dev *wdev = NULL;

		pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
		wdev = &pMbss->wdev;

		if (wdev == NULL)
			return FALSE;

		pDot11h = wdev->pDot11_H;

		if (pDot11h == NULL)
			return FALSE;

		if (!SSID_EQUAL(arg, strlen(arg), pMbss->Ssid, strlen(pMbss->Ssid))) {

			for (i = 0; i < MAX_PMKID_COUNT; i++) {
				if ((pAd->ApCfg.PMKIDCache.BSSIDInfo[i].Valid == TRUE)
					&& (pAd->ApCfg.PMKIDCache.BSSIDInfo[i].Mbssidx == pMbss->mbss_idx)) {
					pAd->ApCfg.PMKIDCache.BSSIDInfo[i].Valid = FALSE;
					MTWF_PRINT("Modify SSID and clear PMKID (idx %d)from (mbssidx %d)\n", i, pMbss->mbss_idx);
				}
			}
		}
		NdisZeroMemory(pMbss->Ssid, MAX_LEN_OF_SSID + 1);
		NdisMoveMemory(pMbss->Ssid, arg, strlen(arg));
		pMbss->SsidLen = (UCHAR)strlen(arg);
		pMbss->ShortSSID = Crcbitbybitfast(pMbss->Ssid, pMbss->SsidLen);
		success = TRUE;

#ifdef P2P_SUPPORT

		if (pObj->ioctl_if_type == INT_P2P) {
			if (P2P_GO_ON(pAd)) {
				P2P_GoStop(pAd);
				P2P_GoStartUp(pAd, MAIN_MBSSID);
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(p2p%d) Set_SSID_Proc::(Len=%d,Ssid=%s)\n", pObj->ioctl_if,
						 mbss->SsidLen, mbss->Ssid);
			}
		} else
#endif /* P2P_SUPPORT */
		{
			OPSTATUS_CLEAR_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
			ap_send_broadcast_deauth(pAd, wdev);
			if (IS_SECURITY(&wdev->SecConfig))
				pMbss->CapabilityInfo |= 0x0010;
			else
				pMbss->CapabilityInfo &= ~(0x0010);
			APSecInit(pAd, wdev);
			restart_ap(&pMbss->wdev);
			OPSTATUS_SET_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"I/F(ra%d) Set_SSID_Proc::(Len=%d,Ssid=%s)\n",
				pObj->ioctl_if, pMbss->SsidLen, pMbss->Ssid);

#ifdef QOS_R1
			qos_set_dscp2up_mapping(pAd, pMbss);
#endif
		}
#ifdef ANDLINK_V4_0
		if (pAd->CommonCfg.andlink_enable[HcGetBandByWdev(wdev)]) {
			struct andlink_wifi_ch_info wifi_ch_info;
			/*security mode*/
			snprintf(wifi_ch_info.sec_mode, ANDLINK_SEC_LEN,"%s", GetAuthModeStr(wdev->SecConfig.AKMMap));
			/*pwd*/
			snprintf(wifi_ch_info.pwd, LEN_PSK,"%s", wdev->SecConfig.PSK);
			/*ssid*/
			snprintf(wifi_ch_info.ssid, SSID_LEN, "%s", pMbss->Ssid);
			wifi_ch_info.max_sta_num = pMbss->MaxStaNum;
			wifi_ch_info.is_hidden = pMbss->bHideSsid;
			/*send wireless event*/
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				"\nsend andlink_send_wifi_chg_event\n");
			andlink_send_wifi_chg_event(pAd, wdev, &wifi_ch_info);
		}
#endif/*ANDLINK_V4_0*/
	} else
		success = FALSE;

	return success;
}

/*
    ==========================================================================
    Description:
	Set TxRate
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_TxRate_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid ioctl_if %d\n", ifIndex);
		return FALSE;
	}

	NdisZeroMemory(pAd->ApCfg.MBSSID[ifIndex].DesiredRates, MAX_LEN_OF_SUPPORTED_RATES);
	pAd->ApCfg.MBSSID[ifIndex].DesiredRatesIndex = os_str_tol(arg, 0, 10);
	/* todo RTMPBuildDesireRate(pAd, pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].DesiredRatesIndex); */
	/*todo MlmeUpdateTxRates(pAd); */
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set BasicRate
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_BasicRate_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	ULONG BasicRateBitmap;

	BasicRateBitmap = (ULONG) os_str_tol(arg, 0, 10);

	if (BasicRateBitmap > 4095) /* (2 ^ MAX_LEN_OF_SUPPORTED_RATES) -1 */
		return FALSE;

	pAd->CommonCfg.BasicRateBitmap = BasicRateBitmap;
	pAd->CommonCfg.BasicRateBitmapOld = BasicRateBitmap;
	MlmeUpdateTxRates(pAd, FALSE, (UCHAR)pObj->ioctl_if);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set_BasicRate_Proc::(BasicRateBitmap=0x%08lx)\n",
			 pAd->CommonCfg.BasicRateBitmap);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set Beacon Period
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_BeaconPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	USHORT BeaconPeriod;
	INT   success = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;

	UCHAR i = 0;
	struct wifi_dev *tdev;

	/* only do this for AP MBSS, ignore other inf type */
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		UINT8 IfIdx = pObj->ioctl_if;

		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
	} else
		return FALSE;

	BeaconPeriod = (USHORT) os_str_tol(arg, 0, 10);
	if ((BeaconPeriod >= 20) && (BeaconPeriod < 1024) && wdev) {
	    /* TBD: new chips have the capability to support separate Beacon Period for 2G/5G band */
		pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)] = BeaconPeriod;
		success = TRUE;
		/* TBD: new chips have the capability to support separate Beacon Period for 2G/5G band */
#ifdef AP_QLOAD_SUPPORT
		/* re-calculate QloadBusyTimeThreshold */
		QBSS_LoadAlarmReset(pAd);
#endif /* AP_QLOAD_SUPPORT */
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_ALL_AP_RENEW);

		for (i = 0; i < WDEV_NUM_MAX; i++) {
			tdev = pAd->wdev_list[i];
			if (tdev && (tdev->wdev_type == WDEV_TYPE_AP) && HcIsRadioAcq(tdev)) {
				/* For global setting, syc change to each MBSS */
				MTWF_PRINT("sync for : %s\n", (char *)tdev->if_dev->name);
				tdev->bss_info_argument.bcn_period = pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(tdev)];
				tdev->bss_info_argument.u4BssInfoFeature = BSS_INFO_BASIC_FEATURE;
				if (AsicBssInfoUpdate(pAd, &tdev->bss_info_argument) != NDIS_STATUS_SUCCESS)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"Fail to apply the bssinfo\n");
				}
			}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(BeaconPeriod=%d)\n",
				 pAd->CommonCfg.BeaconPeriod[HcGetBandByWdev(wdev)]);
	}
	return success;
}

/*
    ==========================================================================
    Description:
	Set Dtim Period
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_DtimPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	USHORT DtimPeriod;
	INT retValue = FALSE;
	UINT ifIndex = 0;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;

	/* only do this for AP MBSS, ignore other inf type */
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid ioctl_if %d\n", ifIndex);
			return FALSE;
		}
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	} else
		return FALSE;

	DtimPeriod = (USHORT) os_str_tol(arg, 0, 10);

	if ((DtimPeriod >= 1) && (DtimPeriod <= 255)) {

		pAd->ApCfg.MBSSID[ifIndex].DtimPeriod = DtimPeriod;

		UpdateBeaconHandler(
			pAd,
			wdev,
			BCN_UPDATE_ALL_AP_RENEW);

		wdev->bss_info_argument.dtim_period = DtimPeriod;
		wdev->bss_info_argument.u4BssInfoFeature = BSS_INFO_BASIC_FEATURE;
		if (AsicBssInfoUpdate(pAd, &wdev->bss_info_argument) != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
				"Fail to apply the bssinfo\n");

		retValue = TRUE;
	}

#ifdef MBSS_DTIM_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "(i/f %d) (DtimPeriod=%d)\n",
	ifIndex, pAd->ApCfg.MBSSID[ifIndex].DtimPeriod);
#else
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "(DtimPeriod=%d)\n",
	pAd->ApCfg.DtimPeriod);
#endif
	return retValue;
}

/*
    ==========================================================================
    Description:
	Disable/enable OLBC detection manually
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_OLBCDetection_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	switch (os_str_tol(arg, 0, 10)) {
	case 0: /*enable OLBC detect */
		pAd->CommonCfg.DisableOLBCDetect = 0;
		break;

	case 1: /*disable OLBC detect */
		pAd->CommonCfg.DisableOLBCDetect = 1;
		break;

	default:  /*Invalid argument */
		return FALSE;
	}

	return TRUE;
}

/*
*
*/
INT set_qiscdump_proc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(
								pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	PNET_DEV ndev = NULL;

	if (wdev == NULL)
		return FALSE;

	ndev = wdev->if_dev;
	MTWF_PRINT("wdev=%p, idx=%d, dev=%p, %s\n", wdev, wdev->wdev_idx, ndev, ndev->name);
#ifdef CONFIG_DBG_QDISC

	if (wdev && wdev->func_dev)
		os_system_tx_queue_dump(ndev);

#endif
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set WmmCapable Enable or Disable
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_WmmCapable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	BOOLEAN	bWmmCapable;
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev;
	UINT ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid ioctl_if %d\n", ifIndex);
		return FALSE;
	}

	bWmmCapable = os_str_tol(arg, 0, 10);
	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

	if (bWmmCapable == 1)
		wdev->bWmmCapable = TRUE;
	else if (bWmmCapable == 0)
		wdev->bWmmCapable = FALSE;
	else
		return FALSE;  /*Invalid argument */

	pAd->ApCfg.MBSSID[ifIndex].bWmmCapableOrg =
		pAd->ApCfg.MBSSID[ifIndex].wdev.bWmmCapable;
#ifdef RTL865X_FAST_PATH

	if (!isFastPathCapable(pAd)) {
		rtlairgo_fast_tx_unregister();
		rtl865x_extDev_unregisterUcastTxDev(pAd->net_dev);
	}

#endif
#ifdef DOT11_N_SUPPORT
	/*Sync with the HT relate info. In N mode, we should re-enable it */
	SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d) (bWmmCapable=%d)\n",
			 ifIndex, wdev->bWmmCapable);
	return TRUE;
}

INT	Set_AP_PerMbssMaxStaNum_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT			apidx = pObj->ioctl_if;

	return ApCfg_Set_PerMbssMaxStaNum_Proc(pAd, apidx, arg);
}

/*
    ==========================================================================
    Description:
	Set BSS Max Idle Period (in units of 1000 TUs)
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	set_bss_max_idle_period_proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING		*arg)
{
	USHORT idle_period;
	UINT8 IfIdx = 0;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;

	/* only do this for AP MBSS, ignore other inf type */
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		if (pObj->ioctl_if < 0 || pObj->ioctl_if >= MAX_BEACON_NUM) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Array index error\n");
			return FALSE;
		}
		IfIdx = PF_TO_BSS_IDX(pAd, pObj->ioctl_if);
		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
	} else
		return FALSE;

	idle_period = (USHORT) os_str_tol(arg, 0, 10);

	if ((idle_period >= 1) && (idle_period <= 0xffff)) {
		pAd->ApCfg.MBSSID[IfIdx].max_idle_period = idle_period;
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s::(i/f %d) (MaxIdlePeriod=%d)\n",
			__func__, IfIdx, idle_period);

		return TRUE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"%s::(i/f %d) (ERROR! range 1 to 65535)\n",
			__func__, IfIdx);

		return FALSE;
	}
}

/*
    ==========================================================================
    Description:
	Set slot time
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_SlotTime_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT8 SlotTime;
	struct wifi_dev *wdev = NULL;
	UCHAR BandIdx;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Incorrect BSS!!\n");
		return FALSE;
	}
	BandIdx = HcGetBandByWdev(wdev);

	if (wdev->channel > 14) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "5G only support slottime 9us\n");
		return FALSE;
	}
	SlotTime = (UINT8) os_str_tol(arg, 0, 10);
	if ((SlotTime < 9) || (SlotTime > 25)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid arguments! 9~25\n");
		return FALSE;
	}

	pAd->CommonCfg.bUseShortSlotTime[BandIdx] = (SlotTime == 9) ? TRUE : FALSE;
	pAd->CommonCfg.SlotTime[BandIdx] = SlotTime;
	HW_SET_SLOTTIME(pAd, pAd->CommonCfg.bUseShortSlotTime[BandIdx], wdev->channel, wdev);

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set No Forwarding Enable or Disable
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_NoForwarding_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	ULONG NoForwarding;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid ioctl_if %d\n", ifIndex);
		return FALSE;
	}

	NoForwarding = os_str_tol(arg, 0, 10);

	if (NoForwarding == 1)
		pAd->ApCfg.MBSSID[ifIndex].IsolateInterStaTraffic = TRUE;
	else if (NoForwarding == 0)
		pAd->ApCfg.MBSSID[ifIndex].IsolateInterStaTraffic = FALSE;
	else
		return FALSE;  /*Invalid argument */

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d) Set_NoForwarding_Proc::(NoForwarding=%ld)\n",
			 ifIndex, pAd->ApCfg.MBSSID[ifIndex].IsolateInterStaTraffic);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set No Forwarding between each SSID
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_NoForwardingBTNSSID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	ULONG NoForwarding;

	NoForwarding = os_str_tol(arg, 0, 10);

	if (NoForwarding == 1)
		pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = TRUE;
	else if (NoForwarding == 0)
		pAd->ApCfg.IsolateInterStaTrafficBTNBSSID = FALSE;
	else
		return FALSE;  /*Invalid argument */

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(NoForwarding=%ld)\n",
			 pAd->ApCfg.IsolateInterStaTrafficBTNBSSID);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set Hide SSID Enable or Disable
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_HideSSID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	BOOLEAN bHideSsid;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid ioctl_if %d\n", ifIndex);
		return FALSE;
	}

	bHideSsid = os_str_tol(arg, 0, 10);

	if (bHideSsid == 1)
		bHideSsid = TRUE;
	else if (bHideSsid == 0)
		bHideSsid = FALSE;
	else
		return FALSE;  /*Invalid argument */

	if (pAd->ApCfg.MBSSID[ifIndex].bHideSsid != bHideSsid)
		pAd->ApCfg.MBSSID[ifIndex].bHideSsid = bHideSsid;

#ifdef WSC_V2_SUPPORT

	if (pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.WscV2Info.bEnableWpsV2)
		WscOnOff(pAd, ifIndex, pAd->ApCfg.MBSSID[ifIndex].bHideSsid);

#endif /* WSC_V2_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d) (HideSSID=%d)\n", ifIndex,
			 pAd->ApCfg.MBSSID[ifIndex].bHideSsid);
	return TRUE;
}

#ifdef VLAN_SUPPORT
/*
    ==========================================================================
    Description:
	Enable/Disable VLAN function
    Return:
	TRUE if parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_VLANEn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT vlan_en;

	vlan_en = os_str_tol(arg, 0, 10);

	if (vlan_en == 1)
		pAd->CommonCfg.bEnableVlan = TRUE;
	else if (vlan_en == 0)
		pAd->CommonCfg.bEnableVlan = FALSE;
	else
		return FALSE;  /*Invalid argument */

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "(vlan_en=%d)\n",
			 pAd->CommonCfg.bEnableVlan);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set VLAN's ID field
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_VLANID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	UINT ifIndex, if_type;
	UINT16 vid;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	vid = os_str_tol(arg, 0, 10);

	if (vid > MAX_VID) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Invalid VID value\n");
		return FALSE;
	}

	ifIndex = pObj->ioctl_if;
	if_type = pObj->ioctl_if_type;

	if (if_type == INT_MAIN || if_type == INT_MBSSID) {
		if (VALID_MBSS(pAd, ifIndex))
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid MBSS index %d\n", ifIndex);
	} else if (if_type == INT_APCLI) {
		if (ifIndex < MAX_MULTI_STA)
			wdev = &pAd->StaCfg[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid APCLI index %d\n", ifIndex);
	} else if (if_type == INT_WDS) {
		if (ifIndex < MAX_WDS_ENTRY)
			wdev = &pAd->WdsTab.WdsEntry[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid WDS index %d\n", ifIndex);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Unexpected if_type\n");
		return FALSE;
	}

	if (!wdev)
		return FALSE;

	wdev->VLAN_VID = vid;
	if (cap->vlan_rx_tag_mode == VLAN_RX_TAG_HW_MODE)
		asic_update_vlan_id(pAd, HcGetBandByWdev(wdev), wdev->OmacIdx, vid);

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set VLAN's priority field
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_VLANPriority_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	UINT ifIndex, if_type;
	UINT8 priority;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	priority = os_str_tol(arg, 0, 10);
	if (priority > MAX_PCP) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Invalid PCP value\n");
		return FALSE;
	}

	ifIndex = pObj->ioctl_if;
	if_type = pObj->ioctl_if_type;

	if (if_type == INT_MAIN || if_type == INT_MBSSID) {
		if (VALID_MBSS(pAd, ifIndex))
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid MBSS index %d\n", ifIndex);
	} else if (if_type == INT_APCLI) {
		if (ifIndex < MAX_MULTI_STA)
			wdev = &pAd->StaCfg[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid APCLI index %d\n", ifIndex);
	} else if (if_type == INT_WDS) {
		if (ifIndex < MAX_WDS_ENTRY)
			wdev = &pAd->WdsTab.WdsEntry[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid WDS index %d\n", ifIndex);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Unexpected if_type\n");
		return FALSE;
	}

	if (!wdev)
		return FALSE;

	wdev->VLAN_Priority = priority;

	if (cap->vlan_rx_tag_mode == VLAN_RX_TAG_HW_MODE)
		asic_update_vlan_priority(pAd, HcGetBandByWdev(wdev), wdev->OmacIdx, priority);

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set enable or disable carry VLAN in the air
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_VLAN_TAG_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	UINT ifIndex, if_type;
	BOOLEAN	bVLAN_Tag;

	bVLAN_Tag = (os_str_tol(arg, 0, 10)) ? TRUE : FALSE;
	ifIndex = pObj->ioctl_if;
	if_type = pObj->ioctl_if_type;

	if (if_type == INT_MAIN || if_type == INT_MBSSID) {
		if (VALID_MBSS(pAd, ifIndex))
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid MBSS index %d\n", ifIndex);
	} else if (if_type == INT_APCLI) {
		if (ifIndex < MAX_MULTI_STA)
			wdev = &pAd->StaCfg[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid APCLI index %d\n", ifIndex);
	} else if (if_type == INT_WDS) {
		if (ifIndex < MAX_WDS_ENTRY)
			wdev = &pAd->WdsTab.WdsEntry[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid WDS index %d\n", ifIndex);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Unexpected if_type\n");
		return FALSE;
	}
	if (!wdev)
		return FALSE;

	if (wdev->if_up_down_state == 0)
		return FALSE;

	if (wlan_operate_get_state(wdev) == WLAN_OPER_STATE_INVALID) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Interface WLAN_OPER_STATE_INVALID, Skip VLAN Tag setting\n");
		return FALSE;
	}

	wdev->bVLAN_Tag = bVLAN_Tag;
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set the VLAN Checking Policy
	For the detail, please refer to VLAN_TX_Policy &VLAN_RX_Policy
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_VLAN_Policy_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	UINT ifIndex, if_type, if_idx;
	char *direction, *policy;
	USHORT vlan_policy;
	BOOLEAN dir;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	ifIndex = pObj->ioctl_if;
	if_type = pObj->ioctl_if_type;

	if (if_type == INT_MAIN || if_type == INT_MBSSID) {
		if (VALID_MBSS(pAd, ifIndex))
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid MBSS index %d\n", ifIndex);
	} else if (if_type == INT_APCLI) {
		if (ifIndex < MAX_MULTI_STA)
			wdev = &pAd->StaCfg[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid APCLI index %d\n", ifIndex);
	} else if (if_type == INT_WDS) {
		if (ifIndex < MAX_WDS_ENTRY)
			wdev = &pAd->WdsTab.WdsEntry[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid WDS index %d\n", ifIndex);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Unexpected if_type\n");
		return FALSE;
	}

	direction = strsep(&arg, ":");
	policy = strsep(&arg, ":");
	if (!wdev || !direction || !policy)
		return FALSE;

	if (wlan_operate_get_state(wdev) == WLAN_OPER_STATE_INVALID) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Interface WLAN_OPER_STATE_INVALID, Skip VLAN Policy setting\n");
		return FALSE;
	}
	dir = (os_str_tol(direction, 0, 10) == TX_VLAN) ? TX_VLAN : RX_VLAN;
	vlan_policy = os_str_tol(policy, 0, 10);

	if ((dir > RX_VLAN)
		 || (dir == TX_VLAN && vlan_policy >= VLAN_TX_POLICY_NUM)
		 || (dir == RX_VLAN && vlan_policy >= VLAN_RX_POLICY_NUM)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Invalid argument\n");
		return FALSE;
	}

	wdev->VLAN_Policy[dir] = vlan_policy;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "Set the VLAN checking policy : %d\n",
			 wdev->VLAN_Policy[dir]);

	/*Update RX policy to DCR0 (DMA)*/
	if (dir == RX_VLAN) {
		switch (vlan_policy) {
		case VLAN_RX_UNTAG:
			/*InSVlan = 0, RmVlan = 1, SwPcP = 0*/
			AsicRxHeaderTransCtl(pAd, TRUE, FALSE, FALSE, TRUE, FALSE);
			break;
		case VLAN_RX_REPLACE_ALL:
			/*InSVlan = 1, RmVlan = 0, SwPcP = 1*/
			if (cap->vlan_rx_tag_mode == VLAN_RX_TAG_HW_MODE)
				AsicRxHeaderTransCtl(pAd, TRUE, FALSE, TRUE, FALSE, TRUE);
			else /*VLAN_RX_TAG_SW_MODE*/
				AsicRxHeaderTransCtl(pAd, TRUE, FALSE, FALSE, FALSE, FALSE);
			break;
		case VLAN_RX_REPLACE_VID:
			/*InSVlan = 1, RmVlan = 0, SwPcP = 0*/
			if (cap->vlan_rx_tag_mode == VLAN_RX_TAG_HW_MODE)
				AsicRxHeaderTransCtl(pAd, TRUE, FALSE, TRUE, FALSE, FALSE);
			else /*VLAN_RX_TAG_SW_MODE*/
				AsicRxHeaderTransCtl(pAd, TRUE, FALSE, FALSE, FALSE, FALSE);
			break;
		case VLAN_RX_DROP:
		case VLAN_RX_ALLOW:
		default:
			AsicRxHeaderTransCtl(pAd, TRUE, FALSE, FALSE, FALSE, FALSE);
			break;
		}

		/* For now, VLAN Rx Policy is HW-supported, so we have to sync the setting to all of the wdev on the pAd*/
		for (if_idx = 0; VALID_MBSS(pAd, if_idx); if_idx++) {
			pAd->ApCfg.MBSSID[if_idx].wdev.VLAN_Policy[RX_VLAN] = vlan_policy;
		}

		for (if_idx = 0; if_idx < MAX_MULTI_STA; if_idx++) {
			pAd->StaCfg[if_idx].wdev.VLAN_Policy[RX_VLAN] = vlan_policy;
		}

	}
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Show VLAN-related field
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Show_VLAN_Info_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	UINT ifIndex, if_type;

	ifIndex = pObj->ioctl_if;
	if_type = pObj->ioctl_if_type;

	if (if_type == INT_MAIN || if_type == INT_MBSSID) {
		if (VALID_MBSS(pAd, ifIndex))
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid MBSS index %d\n", ifIndex);
	} else if (if_type == INT_APCLI) {
		if (ifIndex < MAX_MULTI_STA)
			wdev = &pAd->StaCfg[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid APCLI index %d\n", ifIndex);
	} else if (if_type == INT_WDS) {
		if (ifIndex < MAX_WDS_ENTRY)
			wdev = &pAd->WdsTab.WdsEntry[ifIndex].wdev;
		else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "invalid WDS index %d\n", ifIndex);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Unexpected if_type\n");
		return FALSE;
	}

	if (!wdev)
		return FALSE;

	MTWF_PRINT("VLANEn=%d\n", pAd->CommonCfg.bEnableVlan);
	MTWF_PRINT("bVLAN_Tag=%d\n", wdev->bVLAN_Tag);
	MTWF_PRINT("VLANID=%d\n", wdev->VLAN_VID);
	MTWF_PRINT("VLANPriority=%d\n", wdev->VLAN_Priority);
	MTWF_PRINT("VLANPolicy(Tx)=%d\n", wdev->VLAN_Policy[TX_VLAN]);
	MTWF_PRINT("VLANPolicy(Rx)=%d\n", wdev->VLAN_Policy[RX_VLAN]);
	MTWF_PRINT("VLANTxdrop=%d\n", wdev->VLANTxdrop);
	MTWF_PRINT("VLANRxdrop=%d\n", wdev->VLANRxdrop);
	wdev->VLANTxdrop = 0;
	wdev->VLANRxdrop = 0;
	return TRUE;
}
#endif /*VLAN_SUPPORT*/

#ifdef CONFIG_VLAN_GTK_SUPPORT
INT Show_VLAN_GTK_Info_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	UINT ifIndex, if_type;
	struct list_head *listptr;
	struct vlan_gtk_info *vg_info;
	UINT16 i;

	ifIndex = pObj->ioctl_if;
	if_type = pObj->ioctl_if_type;
	if (VALID_MBSS(pAd, ifIndex))
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	else
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"VLAN interface hooked on %s:\n", wdev->if_dev->name);
	list_for_each(listptr, &wdev->vlan_gtk_list) {
		vg_info = list_entry(listptr, struct vlan_gtk_info, list);
		if (!vg_info)
			continue;

		char keyEle[4];
		char keyStr[64] = {0};

		for (i = 0; i < vg_info->gtk_len; i++) {
			snprintf(keyEle, sizeof(keyEle), "%02x ", vg_info->vlan_gtk[i]);
			strcat(keyStr, keyEle);
		}
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"    ifname=%s vlan_id=%d vlan_bmc_wcid=%d gtk_hexdump=%s\n",
				 vg_info->vlan_dev->name, vg_info->vlan_id, vg_info->vlan_bmc_idx, keyStr);
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"STAs which are in VLAN:\n");
	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (pEntry->vlan_id > 0) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"    %02X:%02X:%02X:%02X:%02X:%02X  vlan_id=%d\n",
					 PRINT_MAC(pEntry->Addr), pEntry->vlan_id);
		}
	}

	return TRUE;
}
#endif

INT	Set_AP_WpaMixPairCipher_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	/*
		In WPA-WPA2 mix mode, it provides a more flexible cipher combination.
		-	WPA-AES and WPA2-TKIP
		-	WPA-AES and WPA2-TKIPAES
		-	WPA-TKIP and WPA2-AES
		-	WPA-TKIP and WPA2-TKIPAES
		-	WPA-TKIPAES and WPA2-AES
		-	WPA-TKIPAES and WPA2-TKIP
		-	WPA-TKIPAES and WPA2-TKIPAES (default)
	 */
	Set_SecAuthMode_Proc(pAd, arg);
	Set_SecEncrypType_Proc(pAd, arg);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set WPA rekey interval value
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_RekeyInterval_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	ULONG value_interval;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;
	value_interval = os_str_tol(arg, 0, 10);

	if ((value_interval >= 10) && (value_interval < MAX_GROUP_REKEY_INTERVAL))
		pSecConfig->GroupReKeyInterval = value_interval;
	else /*Default*/
		pSecConfig->GroupReKeyInterval = DEFAULT_GROUP_REKEY_INTERVAL;

	pSecConfig->GroupReKeyMethod = SEC_GROUP_REKEY_TIME;

	WPAGroupRekeyByWdev(pAd, wdev);

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) GroupKey ReKeyInterval=%ld seconds\n",
			 INF_MBSSID_DEV_NAME, apidx, pSecConfig->GroupReKeyInterval);
	return TRUE;
}

#ifdef SPECIFIC_TX_POWER_SUPPORT
INT Set_AP_PKT_PWR(
	IN  PRTMP_ADAPTER    pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR           apidx = pObj->ioctl_if;
	INT input;

	input = os_str_tol(arg, 0, 10);

	/*
	  Tx_PWR_ADJ[3:0] From 0 to 7 is Positive & add with Tx Power (dB),
	      From 8 to 15 is minus with Tx Power mapping to -16 to -2 (step by 2),
	      Default value: 0.

	  [0x13BC]TX_ALC_MONITOR, 13:8
		  TX_ALC_REQ_ADJ TX ALC Req Saturated[5:0], unit (0.5dB)
	*/

	if ((input >= 0) && (input <= 15))
		pAd->ApCfg.MBSSID[apidx].TxPwrAdj = input;
	else
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "AP[%d]->PktPwr: Out of Range\n");

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "AP[%d]->PktPwr: %d\n", apidx,
			 pAd->ApCfg.MBSSID[apidx].TxPwrAdj);
	return TRUE;
}
#endif /* SPECIFIC_TX_POWER_SUPPORT */
/*
    ==========================================================================
    Description:
	Set WPA rekey method
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_RekeyMethod_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	if (rtstrcasecmp(arg, "TIME") == TRUE)
		pSecConfig->GroupReKeyMethod = SEC_GROUP_REKEY_TIME;
	else if (rtstrcasecmp(arg, "PKT") == TRUE)
		pSecConfig->GroupReKeyMethod = SEC_GROUP_REKEY_PACKET;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) GroupKey ReKeyMethod=%x\n",
			 INF_MBSSID_DEV_NAME, apidx, pSecConfig->GroupReKeyMethod);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set PMK-cache period
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AP_PMKCachePeriod_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;
	UINT32 val = os_str_tol(arg, 0, 10);

	pAd->ApCfg.MBSSID[apidx].PMKCachePeriod = val * 60 * OS_HZ;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(ra%d) Set_AP_PMKCachePeriod_Proc=%ld\n",
			 apidx, pAd->ApCfg.MBSSID[apidx].PMKCachePeriod);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set AssocReq RSSI Threshold to reject STA with weak signal.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_AP_ASSOC_REQ_RSSI_THRESHOLD(
	IN  PRTMP_ADAPTER    pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR           apidx = pObj->ioctl_if;
	UINT j;
	CHAR rssi;

	rssi = os_str_tol(arg, 0, 10);

	if (rssi == 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Disable AP_ASSOC_REQ_RSSI_THRESHOLD\n");
	else if (rssi > 0 || rssi < -100) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Value Error.\n");
		return FALSE;
	}

	pAd->ApCfg.MBSSID[apidx].AssocReqRssiThreshold = rssi;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(ra%d) Set_AP_ASSOC_REQ_RSSI_THRESHOLD=%d\n", apidx,
			 pAd->ApCfg.MBSSID[apidx].AssocReqRssiThreshold);

	for (j = BSS0; j < pAd->ApCfg.BssidNum; j++)
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%d. ==> %d\n", j, pAd->ApCfg.MBSSID[j].AssocReqRssiThreshold);

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set lower limit for AP kicking out a STA.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT Set_AP_KickStaRssiLow_Proc(
	IN  PRTMP_ADAPTER    pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR           apidx = pObj->ioctl_if;
	UINT j;
	CHAR rssi;

	rssi = os_str_tol(arg, 0, 10);

	if (rssi == 0)
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Disable RssiLowForStaKickOut Function\n");
	else if (rssi > 0 || rssi < -100) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RssiLowForStaKickOut Value Error.\n");
		return FALSE;
	}

	pAd->ApCfg.MBSSID[apidx].RssiLowForStaKickOut = rssi;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(ra%d) RssiLowForStaKickOut=%d\n", apidx,
			 pAd->ApCfg.MBSSID[apidx].RssiLowForStaKickOut);

	for (j = BSS0; j < pAd->ApCfg.BssidNum; j++)
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%d. ==> %d\n", j, pAd->ApCfg.MBSSID[j].RssiLowForStaKickOut);

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set Access ctrol policy
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_AccessPolicy_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid ioctl_if %d\n", ifIndex);
		return FALSE;
	}

#ifdef ACL_BLK_COUNT_SUPPORT
	if (os_str_tol(arg, 0, 10) != 2) {
		int count = 0;

		for (count = 0; count < pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Num; count++)
			pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Entry[count].Reject_Count = 0;
	}
#endif
	switch (os_str_tol(arg, 0, 10)) {
	case 0: /*Disable */
		pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Policy = 0;
		break;

	case 1: /* Allow All, and ACL is positive. */
		pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Policy = 1;
		break;

	case 2: /* Reject All, and ACL is negative. */
		pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Policy = 2;
		break;

	default: /*Invalid argument */
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid argument (=%s)\n", arg);
		return FALSE;
	}

	/* check if the change in ACL affects any existent association */
	ApUpdateAccessControlList(pAd, ifIndex);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d) Set_AccessPolicy_Proc::(AccessPolicy=%ld)\n",
			 ifIndex, pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Policy);
	return TRUE;
}

/* Replaced by Set_ACLAddEntry_Proc() and Set_ACLClearAll_Proc() */

/*
    ==========================================================================
    Description:
	Add one entry or several entries(if allowed to)
		into Access control mac table list
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ACLAddEntry_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR					macAddr[MAC_ADDR_LEN];
	/*	RT_802_11_ACL			acl; */
	RT_802_11_ACL			*pacl = NULL;
	RTMP_STRING *this_char;
	RTMP_STRING *value;
	INT						i, j;
	BOOLEAN					isDuplicate = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT 					ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid ioctl_if %d\n", ifIndex);
		return FALSE;
	}

	if (pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Num >= (MAX_NUM_OF_ACL_LIST - 1)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "The AccessControlList is full, and no more entry can join the list!\n");
		return FALSE;
	}

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pacl, sizeof(RT_802_11_ACL));

	if (pacl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory fail!\n");
		return FALSE;
	}

	NdisZeroMemory(pacl, sizeof(RT_802_11_ACL));
	NdisMoveMemory(pacl, &pAd->ApCfg.MBSSID[ifIndex].AccessControlList, sizeof(RT_802_11_ACL));

	while ((this_char = strsep((char **)&arg, ";")) != NULL) {
		if (*this_char == '\0') {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "An unnecessary delimiter entered!\n");
			continue;
		}

		if (strlen(this_char) != 17) { /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "illegal MAC address length!\n");
			continue;
		}

		for (i = 0, value = rstrtok(this_char, ":"); value; value = rstrtok(NULL, ":")) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1)))) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "illegal MAC address format or octet!\n");
				/* Do not use "continue" to replace "break" */
				break;
			}

			AtoH(value, &macAddr[i++], 1);
		}

		if (i != MAC_ADDR_LEN)
			continue;

		/* Check if this entry is duplicate. */
		isDuplicate = FALSE;

		for (j = 0; j < pacl->Num; j++) {
			if (memcmp(pacl->Entry[j].Addr, &macAddr, 6) == 0) {
				isDuplicate = TRUE;
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "You have added an entry before :\n");
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "The duplicate entry is "MACSTR"\n",
						 MAC2STR(macAddr));
			}
		}

		if (!isDuplicate)
			NdisMoveMemory(pacl->Entry[pacl->Num++].Addr, &macAddr, MAC_ADDR_LEN);

		if (pacl->Num == MAX_NUM_OF_ACL_LIST) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 "The AccessControlList is full, and no more entry can join the list!\n");
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "The last entry of ACL is "MACSTR"\n",
					 MAC2STR(macAddr));
			break;
		}
	}

	ASSERT(pacl->Num < MAX_NUM_OF_ACL_LIST);
	NdisZeroMemory(&pAd->ApCfg.MBSSID[ifIndex].AccessControlList, sizeof(RT_802_11_ACL));
	NdisMoveMemory(&pAd->ApCfg.MBSSID[ifIndex].AccessControlList, pacl, sizeof(RT_802_11_ACL));
	/* check if the change in ACL affects any existent association */
	ApUpdateAccessControlList(pAd, ifIndex);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Set (Policy=%ld, Entry#=%ld)\n",
			  pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Policy,
			  pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Num);
#ifdef DBG
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "=============== Entry ===============\n");

	for (i = 0; i < pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Num; i++) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Entry #%02d: ", i + 1);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 MACSTR"\n", MAC2STR(pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Entry[i].Addr));
	}

#endif

	if (pacl != NULL)
		os_free_mem(pacl);

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Delete one entry or several entries(if allowed to)
		from Access control mac table list
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ACLDelEntry_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR					macAddr[MAC_ADDR_LEN];
	UCHAR					nullAddr[MAC_ADDR_LEN];
	/*RT_802_11_ACL			acl; */
	RT_802_11_ACL			*pacl = NULL;
	RTMP_STRING *this_char;
	RTMP_STRING *value;
	INT						i, j;
	BOOLEAN					isFound = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT 					ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid ioctl_if %d\n", ifIndex);
		return FALSE;
	}

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pacl, sizeof(RT_802_11_ACL));

	if (pacl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory fail!!!\n");
		return FALSE;
	}

	NdisZeroMemory(pacl, sizeof(RT_802_11_ACL));
	NdisMoveMemory(pacl, &pAd->ApCfg.MBSSID[ifIndex].AccessControlList, sizeof(RT_802_11_ACL));
	NdisZeroMemory(nullAddr, MAC_ADDR_LEN);

	while ((this_char = strsep((char **)&arg, ";")) != NULL) {
		if (*this_char == '\0') {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "An unnecessary delimiter entered!\n");
			continue;
		}

		if (strlen(this_char) != 17) { /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "illegal MAC address length!\n");
			continue;
		}

		for (i = 0, value = rstrtok(this_char, ":"); value; value = rstrtok(NULL, ":")) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1)))) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "illegal MAC address format or octet!\n");
				/* Do not use "continue" to replace "break" */
				break;
			}

			AtoH(value, &macAddr[i++], 1);
		}

		if (i != MAC_ADDR_LEN)
			continue;

		/* Check if this entry existed. */
		isFound = FALSE;

		for (j = 0; j < pacl->Num; j++) {
			if (memcmp(pacl->Entry[j].Addr, &macAddr, MAC_ADDR_LEN) == 0) {
				isFound = TRUE;
				NdisZeroMemory(pacl->Entry[j].Addr, MAC_ADDR_LEN);
#ifdef ACL_BLK_COUNT_SUPPORT
				pacl->Entry[j].Reject_Count = 0;
#endif
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "The entry "MACSTR" founded will be deleted!\n",
						  MAC2STR(macAddr));
			}
		}

		if (!isFound) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "The entry "MACSTR" is not in the list!\n",
					 MAC2STR(macAddr));
		}
	}

	NdisZeroMemory(&pAd->ApCfg.MBSSID[ifIndex].AccessControlList, sizeof(RT_802_11_ACL));
	pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Policy = pacl->Policy;
	ASSERT(pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Num == 0);
	i = 0;

	for (j = 0; j < pacl->Num; j++) {
		if (memcmp(pacl->Entry[j].Addr, &nullAddr, MAC_ADDR_LEN) == 0)
			continue;
		else
			NdisMoveMemory(&(pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Entry[i++]), pacl->Entry[j].Addr, MAC_ADDR_LEN);
	}

	pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Num = i;
	ASSERT(pacl->Num >= pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Num);
	/* check if the change in ACL affects any existent association */
	ApUpdateAccessControlList(pAd, ifIndex);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set::%s(Policy=%ld, Entry#=%ld)\n",
			 __func__, pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Policy,
			 pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Num);
#ifdef DBG
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "=============== Entry ===============\n");

	for (i = 0; i < pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Num; i++) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Entry #%02d: ", i + 1);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 MACSTR"\n", MAC2STR(pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Entry[i].Addr));
	}

#endif
	os_free_mem(pacl);

	return TRUE;
}

/* for ACL policy message */
#define ACL_POLICY_TYPE_NUM	3
char const *pACL_PolicyMessage[ACL_POLICY_TYPE_NUM] = {
	"the Access Control feature is disabled",						/* 0 : Disable */
	"only the following entries are allowed to join this BSS",			/* 1 : Allow */
	"all the following entries are rejected to join this BSS",			/* 2 : Reject */
};

/*
    ==========================================================================
    Description:
	Dump all the entries in the Access control
		mac table list of a specified BSS
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ACLShowAll_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	/* RT_802_11_ACL			acl; */
	RT_802_11_ACL			*pacl = NULL;
	BOOLEAN					bDumpAll = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT						i;
	UINT 					ifIndex = pObj->ioctl_if;

	bDumpAll = os_str_tol(arg, 0, 10);

	if (bDumpAll == 1)
		bDumpAll = TRUE;
	else if (bDumpAll == 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Your input is 0!\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "The Access Control List will not be dumped!\n");
		return TRUE;
	} else {
		return FALSE;  /* Invalid argument */
	}

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid ioctl_if %d\n", ifIndex);
		return FALSE;
	}

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pacl, sizeof(RT_802_11_ACL));

	if (pacl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory fail!!!\n");
		return FALSE;
	}

	NdisZeroMemory(pacl, sizeof(RT_802_11_ACL));
	NdisMoveMemory(pacl, &pAd->ApCfg.MBSSID[ifIndex].AccessControlList, sizeof(RT_802_11_ACL));

	/* Check if the list is already empty. */
	if (pacl->Num == 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "The Access Control List is empty!\n");
		os_free_mem(pacl);
		return TRUE;
	}

	ASSERT(((bDumpAll == 1) && (pacl->Num > 0)));
	/* Show the corresponding policy first. */
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "=============== Access Control Policy ===============\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Policy is %ld : ", pacl->Policy);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s\n", pACL_PolicyMessage[pacl->Policy]);
	/* Dump the entry in the list one by one */
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "===============  Access Control List  ===============\n");

	for (i = 0; i < pacl->Num; i++) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Entry #%02d: ", i + 1);

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 MACSTR"\n", MAC2STR(pacl->Entry[i].Addr));
	}

	os_free_mem(pacl);

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Clear all the entries in the Access control
		mac table list of a specified BSS
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_ACLClearAll_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	/*	RT_802_11_ACL			acl; */
	RT_802_11_ACL			*pacl = NULL;
	BOOLEAN					bClearAll = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT 					ifIndex = pObj->ioctl_if;

	bClearAll = os_str_tol(arg, 0, 10);

	if (bClearAll == 1)
		bClearAll = TRUE;
	else if (bClearAll == 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Your input is 0!\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "The Access Control List will be kept unchanged!\n");
		return TRUE;
	} else {
		return FALSE;  /* Invalid argument */
	}

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid ioctl_if %d\n", ifIndex);
		return FALSE;
	}

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pacl, sizeof(RT_802_11_ACL));

	if (pacl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory fail!!!\n");
		return FALSE;
	}

	NdisZeroMemory(pacl, sizeof(RT_802_11_ACL));
	NdisMoveMemory(pacl, &pAd->ApCfg.MBSSID[ifIndex].AccessControlList, sizeof(RT_802_11_ACL));

	/* Check if the list is already empty. */
	if (pacl->Num == 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "The Access Control List is empty!\n");
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "No need to clear the Access Control List!\n");

		if (pacl != NULL)
			os_free_mem(pacl);

		return TRUE;
	}

	ASSERT(((bClearAll == 1) && (pacl->Num > 0)));

	/* Clear the entry in the list one by one */
	/* Keep the corresponding policy unchanged. */
	do {
		NdisZeroMemory(pacl->Entry[pacl->Num - 1].Addr, MAC_ADDR_LEN);
#ifdef ACL_BLK_COUNT_SUPPORT
		pacl->Entry[pacl->Num - 1].Reject_Count = 0;
#endif
		pacl->Num -= 1;
	} while (pacl->Num > 0);

	ASSERT(pacl->Num == 0);
	NdisZeroMemory(&(pAd->ApCfg.MBSSID[ifIndex].AccessControlList), sizeof(RT_802_11_ACL));
	NdisMoveMemory(&(pAd->ApCfg.MBSSID[ifIndex].AccessControlList), pacl, sizeof(RT_802_11_ACL));
	/* check if the change in ACL affects any existent association */
	ApUpdateAccessControlList(pAd, ifIndex);

	if (pacl != NULL)
		os_free_mem(pacl);

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set (Policy=%ld, Entry#=%ld)\n",
			 pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Policy,
			 pAd->ApCfg.MBSSID[ifIndex].AccessControlList.Num);
	return TRUE;
}

#ifdef DBG
static void _rtmp_hexdump(int level, const char *title, const UINT8 *buf,
						  size_t len, int show)
{
	size_t i;

	if (level < DebugLevel)
		return;

	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s - hexdump(len=%lu):", title, (unsigned long) len);

	if (show) {
		for (i = 0; i < len; i++)
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 " %02x", buf[i]);
	} else
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 " [REMOVED]");

	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
}
void rtmp_hexdump(int level, const char *title, const UINT8 *buf, size_t len)
{
	_rtmp_hexdump(level, title, buf, len, 1);
}
#endif

/*
    ==========================================================================
    Description:
	Reset statistics counter

    Arguments:
	pAdapter            Pointer to our adapter
	arg

    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/

#ifdef AP_SCAN_SUPPORT
/*
    ==========================================================================
    Description:
	Issue a site survey command to driver
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 set site_survey
    ==========================================================================
*/

/*
    ==========================================================================
    Description:
	Issue a Auto-Channel Selection command to driver
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
			1.) iwpriv ra0 set AutoChannelSel=1
			Ues the number of AP to choose
			2.) iwpriv ra0 set AutoChannelSel=2
			Ues the False CCA count to choose
			3.) iwpriv ra0 set AutoChannelSel=3
			Ues the channel busy count to choose
    ==========================================================================
*/
INT Set_AutoChannelSel_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	NDIS_802_11_SSID Ssid;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR IfIdx;
	struct wifi_dev *pwdev = NULL;
	UCHAR band_idx = BAND0;
	INT32 ret = 0;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;
#ifdef TR181_SUPPORT
	struct hdev_obj *hdev = NULL;
#endif /*TR181_SUPPORT*/

	NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
	Ssid.SsidLength = 0;

	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		IfIdx = pObj->ioctl_if;
		pwdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
	} else
		return FALSE;

	if (!pwdev)
		return FALSE;
#ifdef TR181_SUPPORT
	hdev = (struct hdev_obj *)pwdev->pHObj;
#endif /*TR181_SUPPORT*/

	band_idx = HcGetBandByWdev(pwdev);

	if (strlen(arg) <= MAX_LEN_OF_SSID) {
		if (strlen(arg) > 0) {
			NdisMoveMemory(Ssid.Ssid, arg, strlen(arg));
			Ssid.SsidLength = strlen(arg);
		} else { /*ANY ssid */
			Ssid.SsidLength = 0;
			memcpy(Ssid.Ssid, "", 0);
		}
	}

	if (strcmp(arg, "1") == 0)
		pAd->ApCfg.AutoChannelAlg[band_idx] = ChannelAlgApCnt;
	else if (strcmp(arg, "2") == 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Alg CCA not supported now!\n");
		return FALSE;
		}
	else if (strcmp(arg, "3") == 0)
		pAd->ApCfg.AutoChannelAlg[band_idx] = ChannelAlgBusyTime;

	else if (strcmp(arg, "5") == 0) {
		pAd->ApCfg.AutoChannelAlg[band_idx] = ChannelAlgApCnt;
		pAd->ApCfg.auto_ch_score_flag[band_idx] = TRUE;
	 } else if (strcmp(arg, "6") == 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Alg CCA not supported now!\n");
		return FALSE;
	 } else if (strcmp(arg, "7") == 0) {
		pAd->ApCfg.AutoChannelAlg[band_idx] = ChannelAlgBusyTime;
		pAd->ApCfg.auto_ch_score_flag[band_idx] = TRUE;
	 }

	else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Alg isn't defined\n");
		return FALSE;
	}

	MTWF_PRINT("Set_AutoChannelSel_Proc Alg=%d\n", pAd->ApCfg.AutoChannelAlg[band_idx]);
	MTWF_PRINT("\x1b[42m%s: Alg = %d \x1b[m\n", __func__, pAd->ApCfg.AutoChannelAlg[band_idx]);

		/*if scan is running, ACS will not be triggered*/
#ifdef SCAN_SUPPORT
	if (scan_in_run_state(pAd, pwdev)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Failed!!!Scan is running, please try again after scan done!\n");
		return FALSE;
	}
#endif

#ifdef TR181_SUPPORT
	/*set ACS trigger flag to Manual trigger*/
	hdev->rdev->pRadioCtrl->ACSTriggerFlag = 2;
#endif /*TR181_SUPPORT*/
/*To do ACS, need TakeChannelOpCharge first*/
	if (!TakeChannelOpCharge(pAd, pwdev, CH_OP_OWNER_ACS, TRUE)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "TakeChannelOpCharge fail for ACS!!\n");
		return FALSE;
	}

	pAd->ApCfg.iwpriv_event_flag = TRUE;
	AutoChSelScanStart(pAd, pwdev);
	ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ApCfg.set_ch_aync_done, ((600*100*OS_HZ)/1000));/* Wait 60s.*/
	if (ret)
		MTWF_PRINT("wait channel setting success.\n");
	else {
		MTWF_PRINT("wait channel setting timeout.\n");
		pAd->ApCfg.set_ch_async_flag = FALSE;
		pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, band_idx);
		if (pAutoChCtrl)
			pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState = AUTO_CH_SEL_SCAN_IDLE;
	}
	pAd->ApCfg.iwpriv_event_flag = FALSE;
	/*When ACS is done, release ChannelOpCharge here*/
	ReleaseChannelOpCharge(pAd, pwdev, CH_OP_OWNER_ACS);
	return TRUE;
}
#ifdef CONFIG_6G_SUPPORT
INT Set_PSC_ACS_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx;
	UINT8 Psc_acs;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR IfIdx;
	struct wifi_dev *pwdev = NULL;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;

	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		IfIdx = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, IfIdx)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"invalid mbss id(%d)\n", IfIdx);
			return FALSE;
		}
		pwdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
		BandIdx = HcGetBandByWdev(pwdev);
		pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
		Psc_acs = os_str_tol(arg, 0, 10);
		if (WMODE_CAP_6G(pwdev->PhyMode)) {
			if (Psc_acs > 0)
				pAutoChCtrl->AutoChSelCtrl.PSC_ACS = TRUE;
			else
				pAutoChCtrl->AutoChSelCtrl.PSC_ACS = FALSE;
			MTWF_PRINT("\x1b[42m: PSC_ACS=%d \x1b[m\n", pAutoChCtrl->AutoChSelCtrl.PSC_ACS);

		} else
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Not support in current band!!\n");
	} else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_ACS, DBG_LVL_ERROR, "Invalid interface!!\n");
	return TRUE;
}
#endif

INT Set_PartialScan_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		ifIndex;
	UINT8       bPartialScanning;
	struct wifi_dev *wdev = NULL;
	SCAN_CTRL *ScanCtrl;
	UCHAR mbss_idx = 0;

	if ((pObj->ioctl_if_type != INT_APCLI) &&
		(pObj->ioctl_if_type != INT_MAIN) &&
		(pObj->ioctl_if_type != INT_MBSSID))
		return FALSE;

	ifIndex = pObj->ioctl_if;
	bPartialScanning = os_str_tol(arg, 0, 10);
	if ((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) {
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"invalid mbss id(%d)\n", ifIndex);
			return FALSE;
		}
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	}
#ifdef APCLI_SUPPORT
	else if (pObj->ioctl_if_type == INT_APCLI) {
		if (ifIndex >= MAX_APCLI_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"invalid apcli id(%d)\n", ifIndex);
			return FALSE;
		}
		for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
			wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
			if (wdev && wdev->if_up_down_state &&
				(pAd->StaCfg[ifIndex].wdev.PhyMode == wdev->PhyMode))  {
					MTWF_PRINT("==>Set_PartialScan_Proc (wdev %s)\n",wdev->if_dev->name);
					break;
			}
		}
		if (!wdev) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cannot find ap wdev based on apcli wdev phymode\n");
			return FALSE;
		}
	}
#endif /* APCLI_SUPPORT */

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"do not support apcli, please use ap interface to trigger partial scan\n");
		return FALSE;
	}

	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

	if (bPartialScanning > 0) {
		if (ScanCtrl->PartialScan.bScanning) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"partial scan is under process is under process."
			" cannot trigger new partial scan\n");
			return FALSE;
		}
		/*To set ParitalScan flag TRUE, need TakeChannelOpCharge first*/
		if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_PARTIAL_SCAN, TRUE)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"TakeChannelOpCharge fail for PARTIAL SCAN!!\n");
			return FALSE;
		}
		ScanCtrl->PartialScan.pwdev = wdev;
		wdev->ScanInfo.LastScanChannel = 0;
	}
	ScanCtrl->PartialScan.bScanning = bPartialScanning ? TRUE : FALSE;

	MTWF_PRINT("bScanning = %u\n", ScanCtrl->PartialScan.bScanning);
	return TRUE;
}

INT Set_PartialScan_Timer_Interval_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 TimerInterval;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		ifIndex;
	struct wifi_dev *wdev = NULL;
	SCAN_CTRL *ScanCtrl = NULL;
	UCHAR mbss_idx = 0;

	if ((pObj->ioctl_if_type != INT_APCLI) &&
		(pObj->ioctl_if_type != INT_MAIN) &&
		(pObj->ioctl_if_type != INT_MBSSID))
		return FALSE;

	ifIndex = pObj->ioctl_if;
	TimerInterval = os_str_tol(arg, 0, 10);

	if ((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) {
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"invalid mbss id(%d)\n", ifIndex);
			return FALSE;
		}
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	}
#ifdef APCLI_SUPPORT
	else if (pObj->ioctl_if_type == INT_APCLI) {
		if (ifIndex >= MAX_APCLI_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"invalid apcli id(%d)\n", ifIndex);
			return FALSE;
		}
		for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
			if (pAd->StaCfg[ifIndex].wdev.PhyMode ==
				pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode)
				wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
		}
		if (!wdev) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cannot find ap wdev based on apcli wdev phymode.");
			return FALSE;
		}
	}
#endif /* APCLI_SUPPORT */

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"do not support apcli, please use ap interface to set"
		" partial scan timer interval\n");
		return FALSE;
	}

	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
	ScanCtrl->PartialScan.TimerInterval = TimerInterval;
	MTWF_PRINT("Set partial scan timer interval = %d (ms)\n",
			ScanCtrl->PartialScan.TimerInterval);

	return TRUE;
}

INT Set_PartialScan_Num_of_CH_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 num_of_ch = 0;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		ifIndex;
	struct wifi_dev *wdev = NULL;
	SCAN_CTRL *ScanCtrl = NULL;
	UCHAR mbss_idx = 0;

	if ((pObj->ioctl_if_type != INT_APCLI) &&
		(pObj->ioctl_if_type != INT_MAIN) &&
		(pObj->ioctl_if_type != INT_MBSSID))
		return FALSE;

	ifIndex = pObj->ioctl_if;
	num_of_ch = os_str_tol(arg, 0, 10);
	if (num_of_ch == 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"invalid parameter number of channel(0)\n");
		return FALSE;
	}

	if ((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) {
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"invalid mbss id(%d)\n", ifIndex);
			return FALSE;
		}
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	}
#ifdef APCLI_SUPPORT
	else if (pObj->ioctl_if_type == INT_APCLI) {
		if (ifIndex >= MAX_APCLI_NUM) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"invalid apcli id(%d)\n", ifIndex);
			return FALSE;
		}
		for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
			if (pAd->StaCfg[ifIndex].wdev.PhyMode ==
				pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode)
				wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
		}
		if (!wdev) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cannot find ap wdev based on apcli wdev phymode\n");
			return FALSE;
		}
	}
#endif /* APCLI_SUPPORT */

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"do not support apcli, please use ap interface to set"
		" partial scan number of channel\n");
		return FALSE;
	}

	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
	ScanCtrl->PartialScan.NumOfChannels = num_of_ch;
	MTWF_PRINT("Number of channels to scan = %d\n", ScanCtrl->PartialScan.NumOfChannels);
	return TRUE;

}

INT Set_Scan_SkipList_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		ifIndex;
	struct wifi_dev *wdev = NULL;
	SCAN_CTRL *ScanCtrl = NULL;
	UINT8 skiplist_cnt = 0;
	int i;
	char *tmp = NULL;
	char *buf = NULL;

	if ((pObj->ioctl_if_type != INT_APCLI) &&
			(pObj->ioctl_if_type != INT_MAIN) &&
			(pObj->ioctl_if_type != INT_MBSSID))
			return FALSE;

	if (arg == NULL || strlen(arg) == 0)
		return FALSE;

	os_alloc_mem(NULL, (UCHAR **)&buf, strlen(arg)*sizeof(RTMP_STRING));
	os_zero_mem(buf, strlen(arg)*sizeof(RTMP_STRING));

	tmp = buf;
	strncpy(buf, arg, strlen(arg)*sizeof(RTMP_STRING)-1);
	while (strsep(&buf, ":"))
		skiplist_cnt++;

	os_free_mem(tmp);

	MTWF_PRINT("Length of the channel skip list : %d\n", skiplist_cnt);

	ifIndex = pObj->ioctl_if;

	if (((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) &&
		(VALID_MBSS(pAd, ifIndex)))
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

#ifdef APCLI_SUPPORT
	else if ((pObj->ioctl_if_type == INT_APCLI) && (ifIndex < MAX_APCLI_NUM))
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

#endif /* APCLI_SUPPORT */
	else
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	if (!wdev)
		return FALSE;
	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

	ScanCtrl->SkipCh_Num = skiplist_cnt;
	if (ScanCtrl->SkipList)
		os_free_mem(ScanCtrl->SkipList);

	os_alloc_mem(NULL, (UCHAR **)&ScanCtrl->SkipList, skiplist_cnt*sizeof(SCAN_CHANNEL_SKIPLIST));
	for (i = 0; i < skiplist_cnt; i++) {
		tmp = strsep(&arg, ":");
		if (tmp) {
			ScanCtrl->SkipList[i].Channel = (UINT8) os_str_tol(tmp, 0, 10);
			MTWF_PRINT("Channel %d\n", ScanCtrl->SkipList[i].Channel);
		}
	}

	return TRUE;
}

INT Set_Scan_DwellTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		ifIndex;
	struct wifi_dev *wdev = NULL;
	SCAN_CTRL *ScanCtrl = NULL;
	char *band = NULL;
	char *dwell = NULL;

	if ((pObj->ioctl_if_type != INT_APCLI) &&
		(pObj->ioctl_if_type != INT_MAIN) &&
		(pObj->ioctl_if_type != INT_MBSSID))
		return FALSE;

	ifIndex = pObj->ioctl_if;
	if (((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) &&
		(VALID_MBSS(pAd, ifIndex)))
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
#ifdef APCLI_SUPPORT
	else if ((pObj->ioctl_if_type == INT_APCLI) && (ifIndex < MAX_APCLI_NUM))
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
#endif /* APCLI_SUPPORT */
	else
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	if (!wdev)
		return FALSE;
	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

	band = strsep(&arg, ":");
	dwell = strsep(&arg, ":");
	if (!dwell || !band)
		goto format_error;
	if (strcmp(band, "2") == 0)
		ScanCtrl->Usr_dwell.dwell_t_2g = os_str_tol(dwell, 0, 10);
	else if (strcmp(band, "5") == 0)
		ScanCtrl->Usr_dwell.dwell_t_5g = os_str_tol(dwell, 0, 10);
	else
		goto format_error;

	ScanCtrl->Usr_dwell.isActive = TRUE;
	MTWF_PRINT("5G Dwell Time : %d (msec)\n", ScanCtrl->Usr_dwell.dwell_t_5g);
	MTWF_PRINT("2G Dwell Time : %d (msec)\n", ScanCtrl->Usr_dwell.dwell_t_2g);
	return TRUE;

format_error:
	ScanCtrl->Usr_dwell.isActive = FALSE;
	return FALSE;
}

INT Set_Scan_DFS_CH_Utilization_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		ifIndex;
	struct wifi_dev *wdev = NULL;
	SCAN_CTRL *ScanCtrl = NULL;

	if ((pObj->ioctl_if_type != INT_APCLI) &&
		(pObj->ioctl_if_type != INT_MAIN) &&
		(pObj->ioctl_if_type != INT_MBSSID))
		return FALSE;

	ifIndex = pObj->ioctl_if;
	if (((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) &&
		(VALID_MBSS(pAd, ifIndex)))
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
#ifdef APCLI_SUPPORT
	else if ((pObj->ioctl_if_type == INT_APCLI) && (ifIndex < MAX_APCLI_NUM))
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
#endif /* APCLI_SUPPORT */
	else
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	if (!wdev)
		return FALSE;
	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

	if (!arg)
		goto format_error;
	ScanCtrl->dfs_ch_utilization = (os_str_tol(arg, 0, 10)) ? TRUE : FALSE;
	return TRUE;

format_error:
	ScanCtrl->dfs_ch_utilization = FALSE;
	return FALSE;
}

INT show_scan_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int i = 0;
	UINT32 TimerInterval = 0;
	UINT32 partial_scan_num = 0;
	BOOLEAN dfs_ch_utilization = FALSE;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		ifIndex;
	struct wifi_dev *wdev = NULL;
	SCAN_CTRL *ScanCtrl = NULL;

	if ((pObj->ioctl_if_type != INT_APCLI) &&
		(pObj->ioctl_if_type != INT_MAIN) &&
		(pObj->ioctl_if_type != INT_MBSSID))
		return FALSE;

	ifIndex = pObj->ioctl_if;

	if (((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) &&
		(VALID_MBSS(pAd, ifIndex)))
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

#ifdef APCLI_SUPPORT
	else if ((pObj->ioctl_if_type == INT_APCLI) && (ifIndex < MAX_APCLI_NUM))
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

#endif /* APCLI_SUPPORT */
	else
		return FALSE;

	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
	TimerInterval = ScanCtrl->PartialScan.TimerInterval;
	partial_scan_num = ScanCtrl->PartialScan.NumOfChannels;
	dfs_ch_utilization = ScanCtrl->dfs_ch_utilization;
	MTWF_PRINT("===== Scan Information =====\n");
	MTWF_PRINT("= Partial Scan =\n");
	MTWF_PRINT("Partial Scan Timer Interval = %d (ms)\n", TimerInterval);
	MTWF_PRINT("Number of channels to scan = %d\n", partial_scan_num);
	MTWF_PRINT("= DFS Channel utilization =\n");
	MTWF_PRINT("Using DFS channel is allowed = %d\n", dfs_ch_utilization);

	if (ScanCtrl->SkipList) {
			MTWF_PRINT("= Scan Skip List =\n");
			MTWF_PRINT("Length of the channel skip list : %d\n", ScanCtrl->SkipCh_Num);
			for (i = 0; i < ScanCtrl->SkipCh_Num; i++)
				MTWF_PRINT("Channel : %d\n", ScanCtrl->SkipList[i].Channel);
		}

	if (ScanCtrl->Usr_dwell.isActive) {
		MTWF_PRINT("= User-defined Dwell Time =\n");
		MTWF_PRINT("5G Dwell Time : %d (msec)\n", ScanCtrl->Usr_dwell.dwell_t_5g);
		MTWF_PRINT("2G Dwell Time : %d (msec)\n", ScanCtrl->Usr_dwell.dwell_t_2g);
	}

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set a periodic check time for auto channel selection (unit: hour)
   Arguments:
       pAdapter                    Pointer to our adapter

    Return Value:
	TRUE if success, FALSE otherwise

    Note:
	Usage:
		iwpriv ra0 set ACSCheckTime=Hour

    ==========================================================================
*/
INT Set_AutoChannelSelCheckTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx;
	UINT32 time;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR IfIdx;
	struct wifi_dev *pwdev = NULL;

	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		IfIdx = pObj->ioctl_if;
		pwdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
		BandIdx = HcGetBandByWdev(pwdev);
		time = simple_strtol(arg, 0, 10);
#if !defined(ACS_CTCC_SUPPORT)
		time = time * 3600;/* Hour to second */
#endif
		pAd->ApCfg.ACSCheckTime[BandIdx] = time;
		pAd->ApCfg.ACSCheckCount[BandIdx] = 0;/* Reset counter */
		MTWF_PRINT("\x1b[42m: ACSCheckTime[%d]=%u seconds\x1b[m\n",
				  BandIdx, pAd->ApCfg.ACSCheckTime[BandIdx]);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "\x1b[41m: Not support current interface type = %u!!\x1b[m\n",
				  pObj->ioctl_if_type);
	}

	return TRUE;
}
/*
    ==========================================================================
    Description:
	Set a periodic check time for auto channel selection (unit: min)
   Arguments:
       pAdapter                    Pointer to our adapter

    Return Value:
	TRUE if success, FALSE otherwise

    Note:
	Usage:
		iwpriv ra0 set ACSCheckTimeMin=Minute

    ==========================================================================
*/
INT Set_AutoChannelSelCheckMinTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx;
	UINT32 time;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR IfIdx;
	struct wifi_dev *pwdev = NULL;

	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		IfIdx = pObj->ioctl_if;
		pwdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
		BandIdx = HcGetBandByWdev(pwdev);
		time = simple_strtol(arg, 0, 10);
		time = time * 60;/* Minute to second */
		pAd->ApCfg.ACSCheckTime[BandIdx] = time;
		pAd->ApCfg.ACSCheckCount[BandIdx] = 0;/* Reset counter */
		MTWF_PRINT("\x1b[42m: ACSCheckTime[%d]=%u seconds\x1b[m\n",
				  BandIdx, pAd->ApCfg.ACSCheckTime[BandIdx]);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "\x1b[41m: Not support current interface type = %u!!\x1b[m\n",
				  pObj->ioctl_if_type);
	}

	return TRUE;
}

#endif /* AP_SCAN_SUPPORT */

static INT show_apcfg_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	struct apcfg_parameters apcfg_para_setting;
	LONG cfg_mode;
	USHORT wmode;
	POS_COOKIE pObj = NULL;
	CHAR str[10] = "";
	UINT ifIndex;
	UCHAR BandIdx;
	int ret;

	MTWF_PRINT("show ap cfg info:\n");
	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pObj is NULL\n");
		return FALSE;
	}

	ifIndex = pObj->ioctl_if;
	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid ioctl_if %d\n", ifIndex);
		return FALSE;
	}

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is NULL\n");
		return FALSE;
	}

	BandIdx = HcGetBandByWdev(wdev);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%-24s%-16s%-8s\n", " ", "WIFI_DRIVER", "PEAK_VALUE"));
	/*WirelessMode*/
	wmode = wdev->PhyMode;
	cfg_mode = wmode_2_cfgmode(wmode);

	if (WMODE_CAP_2G(wmode))
		apcfg_para_setting.cfg_mode[0] = cfg_mode;
	else if (WMODE_CAP_5G(wmode))
		apcfg_para_setting.cfg_mode[1] = cfg_mode;

	if (cfg_mode == 16)
		MTWF_PRINT("%-24s%-16ld%ld\n", "WirelessMode", cfg_mode,
				  apcfg_for_peak.cfg_mode[0]);
	else if (cfg_mode == 17)
		MTWF_PRINT("%-24s%-16ld%ld\n", "WirelessMode", cfg_mode,
				  apcfg_for_peak.cfg_mode[1]);
	else
		MTWF_PRINT("%-24s%-16ld%ld/%ld\n", "WirelessMode", cfg_mode,
				  apcfg_for_peak.cfg_mode[0], apcfg_for_peak.cfg_mode[1]);

	apcfg_para_setting.tx_power_percentage = pAd->CommonCfg.ucTxPowerPercentage[BandIdx];
	MTWF_PRINT("%-24s%-16lu%lu\n", "TxPower",
			  apcfg_para_setting.tx_power_percentage,
			  apcfg_for_peak.tx_power_percentage);
	/*TxPreamble*/
	apcfg_para_setting.tx_preamble = pAd->CommonCfg.TxPreamble;
	MTWF_PRINT("%-24s%-16lu%lu\n", "TxPreamble",
			  apcfg_para_setting.tx_preamble,
			  apcfg_for_peak.tx_preamble);
	/*RTSThreshold*/
	apcfg_para_setting.conf_len_thld = wlan_config_get_rts_len_thld(wdev);
	apcfg_para_setting.oper_len_thld = wlan_operate_get_rts_len_thld(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "RTSThreshold(config)",
			  apcfg_para_setting.conf_len_thld,
			  apcfg_for_peak.conf_len_thld);
	MTWF_PRINT("%-24s%-16d%d\n", "RTSThreshold(operate)",
			  apcfg_para_setting.oper_len_thld,
			  apcfg_for_peak.oper_len_thld);
	/*FragThreshold*/
	apcfg_para_setting.conf_frag_thld = wlan_config_get_frag_thld(wdev);
	apcfg_para_setting.oper_frag_thld = wlan_operate_get_frag_thld(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "FragThreshold(config)",
			  apcfg_para_setting.conf_frag_thld,
			  apcfg_for_peak.conf_frag_thld);
	MTWF_PRINT("%-24s%-16d%d\n", "FragThreshold(operate)",
			  apcfg_para_setting.oper_frag_thld,
			  apcfg_for_peak.oper_frag_thld);
	/*TxBurst*/
	apcfg_para_setting.bEnableTxBurst = pAd->CommonCfg.bEnableTxBurst;
	MTWF_PRINT("%-24s%-16d%d\n", "TxBurst",
			  apcfg_para_setting.bEnableTxBurst,
			  apcfg_for_peak.bEnableTxBurst);
	/*ShortSlot*/
	apcfg_para_setting.bUseShortSlotTime = pAd->CommonCfg.bUseShortSlotTime[BandIdx];

	MTWF_PRINT("%-24s%-16d%d\n", "ShortSlot",
			  apcfg_para_setting.bUseShortSlotTime,
			  apcfg_for_peak.bUseShortSlotTime);
#ifdef DOT11_N_SUPPORT
	/*HT_BW*/
	apcfg_para_setting.conf_ht_bw = wlan_config_get_ht_bw(wdev);
	apcfg_para_setting.oper_ht_bw = wlan_operate_get_ht_bw(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "HT_BW(config)",
			  apcfg_para_setting.conf_ht_bw,
			  apcfg_for_peak.conf_ht_bw);
	MTWF_PRINT("%-24s%-16d%d\n", "HT_BW(operate)",
			  apcfg_para_setting.oper_ht_bw,
			  apcfg_for_peak.oper_ht_bw);
#ifdef DOT11N_DRAFT3
	/*HT_BSSCoexistence */
	apcfg_para_setting.bBssCoexEnable = pAd->CommonCfg.bBssCoexEnable;
	MTWF_PRINT("%-24s%-16d%d\n", "HT_BSSCoexistence",
			  apcfg_para_setting.bBssCoexEnable,
			  apcfg_for_peak.bBssCoexEnable);
#endif
	/*HT_TxStream */
	apcfg_para_setting.ht_tx_streams = wlan_config_get_tx_stream(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "HT_TxStream",
			  apcfg_para_setting.ht_tx_streams,
			  (pAd->CommonCfg.dbdc_mode ? (apcfg_for_peak.ht_tx_streams - 2) : apcfg_for_peak.ht_tx_streams));
	/*HT_RxStream */
	apcfg_para_setting.ht_rx_streams = wlan_config_get_rx_stream(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "HT_RxStream",
			  apcfg_para_setting.ht_rx_streams,
			  (pAd->CommonCfg.dbdc_mode ? (apcfg_for_peak.ht_rx_streams - 2) : apcfg_for_peak.ht_rx_streams));
	/*HT_BADecline*/
	apcfg_para_setting.ba_decline = wlan_config_get_ba_decline(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "HT_BADecline",
			  apcfg_para_setting.ba_decline,
			  apcfg_for_peak.ba_decline);
	/*HT_AutoBA*/
	apcfg_para_setting.ba_en = wlan_config_get_ba_enable(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "HT_AutoBA",
			  apcfg_para_setting.ba_en,
			  apcfg_for_peak.ba_en);
	/*HT_AMSDU*/
	apcfg_para_setting.AmsduEnable = wlan_config_get_amsdu_en(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "HT_AMSDU",
			  apcfg_para_setting.AmsduEnable,
			  apcfg_for_peak.AmsduEnable);
	/*HT_BAWinSize*/
	apcfg_para_setting.ba_rx_wsize = wlan_config_get_ba_rx_wsize(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "HT_BAWinSize",
			  apcfg_para_setting.ba_rx_wsize,
			  apcfg_for_peak.ba_rx_wsize);
	/*HT_GI*/
	apcfg_para_setting.ht_gi = wlan_config_get_ht_gi(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "HT_GI",
			  apcfg_para_setting.ht_gi,
			  apcfg_for_peak.ht_gi);
	/*HT_STBC*/
	apcfg_para_setting.ht_stbc = wlan_config_get_ht_stbc(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "HT_STBC",
			  apcfg_para_setting.ht_stbc,
			  apcfg_for_peak.ht_stbc);
	/*HT_LDPC*/
	apcfg_para_setting.ht_ldpc = wlan_config_get_ht_ldpc(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "HT_LDPC",
			  apcfg_para_setting.ht_ldpc,
			  apcfg_for_peak.ht_ldpc);
	/*HT_RDG*/
	apcfg_para_setting.bRdg = pAd->CommonCfg.bRdg;
	MTWF_PRINT("%-24s%-16d%d\n", "HT_RDG",
			  apcfg_para_setting.bRdg,
			  apcfg_for_peak.bRdg);
#endif
	/*HT_DisallowTKIP*/
	apcfg_para_setting.HT_DisallowTKIP = pAd->CommonCfg.HT_DisallowTKIP;
	MTWF_PRINT("%-24s%-16d%d\n", "HT_DisallowTKIP",
			  apcfg_para_setting.HT_DisallowTKIP,
			  apcfg_for_peak.HT_DisallowTKIP);
#ifdef DOT11_VHT_AC

	if (WMODE_CAP_5G(wmode)) {
		/*VHT_BW*/
		apcfg_para_setting.conf_vht_bw = wlan_config_get_vht_bw(wdev);
		apcfg_para_setting.oper_vht_bw = wlan_operate_get_vht_bw(wdev);
		MTWF_PRINT("%-24s%-16d%d\n", "VHT_BW(config)",
				  apcfg_para_setting.conf_vht_bw,
				  apcfg_for_peak.conf_vht_bw);
		MTWF_PRINT("%-24s%-16d%d\n", "VHT_BW(operate)",
				  apcfg_para_setting.oper_vht_bw,
				  apcfg_for_peak.oper_vht_bw);
		/*VHT_SGI */
		apcfg_para_setting.vht_sgi = wlan_config_get_vht_sgi(wdev);
		MTWF_PRINT("%-24s%-16d%d\n", "VHT_SGI",
				  apcfg_para_setting.vht_sgi,
				  apcfg_for_peak.vht_sgi);
		/*VHT_STBC*/
		apcfg_para_setting.vht_stbc = wlan_config_get_vht_stbc(wdev);
		MTWF_PRINT("%-24s%-16d%d\n", "VHT_STBC",
				  apcfg_para_setting.vht_stbc,
				  apcfg_for_peak.vht_stbc);
		/*VHT_BW_SIGNAL*/
		apcfg_para_setting.vht_bw_signal = wlan_config_get_vht_bw_sig(wdev);
		MTWF_PRINT("%-24s%-16d%d\n", "VHT_BW_SIGNAL",
				  apcfg_para_setting.vht_bw_signal,
				  apcfg_for_peak.vht_bw_signal);
		/*VHT_LDPC*/
		apcfg_para_setting.vht_ldpc = wlan_config_get_vht_ldpc(wdev);
		MTWF_PRINT("%-24s%-16d%d\n", "VHT_LDPC",
				  apcfg_para_setting.vht_ldpc,
				  apcfg_for_peak.vht_ldpc);
		/* Vht1024QamSupport */
		apcfg_para_setting.vht_1024_qam = pAd->CommonCfg.vht_1024_qam;
		MTWF_PRINT("%-24s%-16d%d\n", "Vht1024QamSupport",
				  apcfg_para_setting.vht_1024_qam,
				  apcfg_for_peak.vht_1024_qam);
	}

	if (WMODE_CAP_2G(wmode)) {
		/*G_BAND_256QAM*/
		apcfg_para_setting.g_band_256_qam = pAd->CommonCfg.g_band_256_qam;
		MTWF_PRINT("%-24s%-16d%d\n", "G_BAND_256QAM",
				  apcfg_para_setting.g_band_256_qam,
				  apcfg_for_peak.g_band_256_qam);
	}

#endif
	/*IEEE80211H*/
	apcfg_para_setting.bIEEE80211H = pAd->CommonCfg.bIEEE80211H;
	MTWF_PRINT("%-24s%-16d%d\n", "IEEE80211H",
			  apcfg_para_setting.bIEEE80211H,
			  apcfg_for_peak.bIEEE80211H);
#ifdef MT_DFS_SUPPORT

	/*DfsEnable*/
	if (WMODE_CAP_5G(wmode)) {
		apcfg_para_setting.bDfsEnable = pAd->CommonCfg.DfsParameter.bDfsEnable;
		MTWF_PRINT("%-24s%-16d%d\n", "DfsEnable",
				  apcfg_para_setting.bDfsEnable,
				  apcfg_for_peak.bDfsEnable);
	}

#endif
#ifdef BACKGROUND_SCAN_SUPPORT

	/*DfsZeroWait*/
	if (!(pAd->CommonCfg.dbdc_mode)) {
		apcfg_para_setting.DfsZeroWaitSupport = pAd->BgndScanCtrl.DfsZeroWaitSupport;
		MTWF_PRINT("%-24s%-16d%d\n", "DfsZeroWait",
				  apcfg_para_setting.DfsZeroWaitSupport,
				  apcfg_for_peak.DfsZeroWaitSupport);
	}

#endif
#ifdef DOT11_HE_AX
	/* MuOfdmaDlEnable */
	apcfg_para_setting.MuOfdmaDlEnable = wlan_config_get_mu_dl_ofdma(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "MuOfdmaDlEnable",
		apcfg_para_setting.MuOfdmaDlEnable,
		apcfg_for_peak.MuOfdmaDlEnable);

	/* MuOfdmaUlEnable */
	apcfg_para_setting.MuOfdmaUlEnable = wlan_config_get_mu_ul_ofdma(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "MuOfdmaUlEnable",
		apcfg_para_setting.MuOfdmaUlEnable,
		apcfg_for_peak.MuOfdmaUlEnable);

	/* MuMimoDlEnable */
	apcfg_para_setting.MuMimoDlEnable = wlan_config_get_mu_dl_mimo(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "MuMimoDlEnable",
		apcfg_para_setting.MuMimoDlEnable,
		apcfg_for_peak.MuMimoDlEnable);

	/* MuMimoUlEnable */
	apcfg_para_setting.MuMimoUlEnable = wlan_config_get_mu_ul_mimo(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "MuMimoUlEnable",
		apcfg_para_setting.MuMimoUlEnable,
		apcfg_for_peak.MuMimoUlEnable);
#endif /* #ifdef DOT11_HE_AX */
#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
	/*ETxBfEnCond*/
	apcfg_para_setting.ETxBfEnCond = pAd->CommonCfg.ETxBfEnCond;
	MTWF_PRINT("%-24s%-16lu%lu\n", "CommonCfg.ETxBfEnCond",
		apcfg_para_setting.ETxBfEnCond,
		apcfg_for_peak.ETxBfEnCond);

    /*ETxBfEnCond*/
	apcfg_para_setting.ETxBfEnCond = wlan_config_get_etxbf(wdev);
	MTWF_PRINT("%-24s%-16lu%lu\n", "ETxBfEnCond",
		apcfg_para_setting.ETxBfEnCond,
		apcfg_for_peak.ETxBfEnCond);

	/*ITxBfEn*/
	apcfg_para_setting.ITxBfEn = pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn;
	MTWF_PRINT("%-24s%-16d%d\n", "CommonCfg.ITxBfEn",
			  apcfg_para_setting.ITxBfEn,
			  apcfg_for_peak.ITxBfEn);

	apcfg_para_setting.ITxBfEn = wlan_config_get_itxbf(wdev);
	MTWF_PRINT("%-24s%-16d%d\n", "ITxBfEn",
			  apcfg_para_setting.ITxBfEn,
			  apcfg_for_peak.ITxBfEn);

	/*MUTxRxEnable*/
	apcfg_para_setting.MUTxRxEnable = pAd->CommonCfg.MUTxRxEnable;
	MTWF_PRINT("%-24s%-16lu%lu\n", "MUTxRxEnable",
			  apcfg_para_setting.MUTxRxEnable,
			  apcfg_for_peak.MUTxRxEnable);
#endif
#endif
	MTWF_PRINT("-----------------------------------------------------\n");
	/*external channel*/
	apcfg_para_setting.channel = wdev->channel;
	MTWF_PRINT("%-24s%-16u\n", "current channel",
			  apcfg_para_setting.channel);
	apcfg_para_setting.ext_channel = wlan_operate_get_ext_cha(wdev);

	if (apcfg_para_setting.ext_channel == EXTCHA_ABOVE) {
		ret = snprintf(str, sizeof(str), "ABOVE");
		if (os_snprintf_error(sizeof(str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			return FALSE;
		}
	} else if (apcfg_para_setting.ext_channel == EXTCHA_BELOW) {
		ret = snprintf(str, sizeof(str), "BELOW");
		if (os_snprintf_error(sizeof(str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			return FALSE;
		}
	} else {
		ret = snprintf(str, sizeof(str), "NONE");
		if (os_snprintf_error(sizeof(str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			return FALSE;
		}
	}
	MTWF_PRINT("%-24s%-16s\n", "extension channel", str);
	return TRUE;
}

#ifdef TXRX_STAT_SUPPORT
INT Show_Radio_Stat_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 i;
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
	for (i = 0 ; i < DBDC_BAND_NUM ; i++) {
	  if (ctrl->rdev[i].pRadioCtrl->Last1TxCnt && ctrl->rdev[i].pRadioCtrl->Last1TxFailCnt)
		ctrl->rdev[i].pRadioCtrl->Last1SecPER = ((100 * (ctrl->rdev[i].pRadioCtrl->Last1TxFailCnt))/
									ctrl->rdev[i].pRadioCtrl->Last1TxCnt);
	}
	MTWF_PRINT("------------------BAND:0-----------------------\n");
	MTWF_PRINT("TxDataPacketCnt     = %lld\n", ctrl->rdev[0].pRadioCtrl->TxDataPacketCount.QuadPart);
	MTWF_PRINT("TxDataPacketByte    = %lld\n", ctrl->rdev[0].pRadioCtrl->TxDataPacketByte.QuadPart);
	MTWF_PRINT("RxDataPacketCnt     = %lld\n", ctrl->rdev[0].pRadioCtrl->RxDataPacketCount.QuadPart);
	MTWF_PRINT("RxDataPacketByte    = %lld\n", ctrl->rdev[0].pRadioCtrl->RxDataPacketByte.QuadPart);
	MTWF_PRINT("TxMgmtPacketCnt     = %lld\n", ctrl->rdev[0].pRadioCtrl->TxMgmtPacketCount.QuadPart);
	MTWF_PRINT("RxMgmtPacketCnt     = %lld\n", ctrl->rdev[0].pRadioCtrl->RxMgmtPacketCount.QuadPart);
	MTWF_PRINT("TxBcnPacketCnt      = %lld\n", ctrl->rdev[0].pRadioCtrl->TxBeaconPacketCount.QuadPart);
	MTWF_PRINT("ucPktsTx            = %lld\n", ctrl->rdev[0].pRadioCtrl->TxUnicastDataPacket.QuadPart);
	MTWF_PRINT("mcPktsTx            = %lld\n", ctrl->rdev[0].pRadioCtrl->TxMulticastDataPacket.QuadPart);
	MTWF_PRINT("bcPktsTx            = %lld\n", ctrl->rdev[0].pRadioCtrl->TxBroadcastDataPacket.QuadPart);
	MTWF_PRINT("TxDroppedPacketCnt  = %lld\n", ctrl->rdev[0].pRadioCtrl->TxPacketDroppedCount.QuadPart);
	MTWF_PRINT("ChannelUtilization  = %u\n", pAd->Ch_BusyTime_11k[DBDC_BAND0]);
	MTWF_PRINT("RxCRCErrorCount     = %lld\n", ctrl->rdev[0].pRadioCtrl->RxCRCErrorCount.QuadPart);
	MTWF_PRINT("RxMICErrorCount     = %lld\n", ctrl->rdev[0].pRadioCtrl->RxMICErrorCount.QuadPart);
	MTWF_PRINT("RxDecyptionErrorCnt = %lld\n", ctrl->rdev[0].pRadioCtrl->RxDecryptionErrorCount.QuadPart);
	MTWF_PRINT("RxTotalErrorCnt     = %lld\n", (ctrl->rdev[0].pRadioCtrl->RxDecryptionErrorCount.QuadPart + ctrl->rdev[0].pRadioCtrl->RxMICErrorCount.QuadPart
		+ ctrl->rdev[0].pRadioCtrl->RxMICErrorCount.QuadPart + ctrl->rdev[0].pRadioCtrl->RxCRCErrorCount.QuadPart));
	MTWF_PRINT("ThroughPut(TX Kbps)	= %lld\n", (ctrl->rdev[0].pRadioCtrl->LastSecTxByte.QuadPart >> 7));
	MTWF_PRINT("ThroughPut(RX Kbps)	= %lld\n", (ctrl->rdev[0].pRadioCtrl->LastSecRxByte.QuadPart >> 7));
	MTWF_PRINT("PER(pct from start)	= %u\n", ctrl->rdev[0].pRadioCtrl->TotalPER);
	MTWF_PRINT("Total PER(Last Sec)	= %u\n", ctrl->rdev[0].pRadioCtrl->Last1SecPER);
	MTWF_PRINT("Data				BK		BE		VI		VO\n");
	MTWF_PRINT("Packets Sent	:	%10lld	%10lld	%10lld	%10lld\n",
			ctrl->rdev[0].pRadioCtrl->TxDataPacketCountPerAC[QID_AC_BK].QuadPart,
			ctrl->rdev[0].pRadioCtrl->TxDataPacketCountPerAC[QID_AC_BE].QuadPart,
			ctrl->rdev[0].pRadioCtrl->TxDataPacketCountPerAC[QID_AC_VI].QuadPart,
			ctrl->rdev[0].pRadioCtrl->TxDataPacketCountPerAC[QID_AC_VO].QuadPart);
	MTWF_PRINT("Packets Rcvd	:	%10lld	%10lld	%10lld	%10lld\n",
			ctrl->rdev[0].pRadioCtrl->RxDataPacketCountPerAC[QID_AC_BK].QuadPart,
			ctrl->rdev[0].pRadioCtrl->RxDataPacketCountPerAC[QID_AC_BE].QuadPart,
			ctrl->rdev[0].pRadioCtrl->RxDataPacketCountPerAC[QID_AC_VI].QuadPart,
			ctrl->rdev[0].pRadioCtrl->RxDataPacketCountPerAC[QID_AC_VO].QuadPart);
	MTWF_PRINT("RSSI				0		1		2		3\n");
	MTWF_PRINT("Data Rx		:		%d		%d		%d		%d\n",
			ctrl->rdev[0].pRadioCtrl->LastDataPktRssi[0],
			ctrl->rdev[0].pRadioCtrl->LastDataPktRssi[1],
			ctrl->rdev[0].pRadioCtrl->LastDataPktRssi[2],
			ctrl->rdev[0].pRadioCtrl->LastDataPktRssi[3]);
	MTWF_PRINT("----------------------------------------------\n");
#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		MTWF_PRINT("------------------BAND:1-----------------------\n");
		MTWF_PRINT("TxDataPacketCnt     = %lld\n", ctrl->rdev[1].pRadioCtrl->TxDataPacketCount.QuadPart);
		MTWF_PRINT("TxDataPacketByte    = %lld\n", ctrl->rdev[1].pRadioCtrl->TxDataPacketByte.QuadPart);
		MTWF_PRINT("RxDataPacketCnt     = %lld\n", ctrl->rdev[1].pRadioCtrl->RxDataPacketCount.QuadPart);
		MTWF_PRINT("RxDataPacketByte    = %lld\n", ctrl->rdev[1].pRadioCtrl->RxDataPacketByte.QuadPart);
		MTWF_PRINT("TxMgmtPacketCnt     = %lld\n", ctrl->rdev[1].pRadioCtrl->TxMgmtPacketCount.QuadPart);
		MTWF_PRINT("RxMgmtPacketCnt     = %lld\n", ctrl->rdev[1].pRadioCtrl->RxMgmtPacketCount.QuadPart);
		MTWF_PRINT("TxBcnPacketCnt      = %lld\n", ctrl->rdev[1].pRadioCtrl->TxBeaconPacketCount.QuadPart);
		MTWF_PRINT("ucPktsTx            = %lld\n", ctrl->rdev[1].pRadioCtrl->TxUnicastDataPacket.QuadPart);
		MTWF_PRINT("mcPktsTx            = %lld\n", ctrl->rdev[1].pRadioCtrl->TxMulticastDataPacket.QuadPart);
		MTWF_PRINT("bcPktsTx            = %lld\n", ctrl->rdev[1].pRadioCtrl->TxBroadcastDataPacket.QuadPart);
		MTWF_PRINT("TxDroppedPacketCnt  = %lld\n", ctrl->rdev[1].pRadioCtrl->TxPacketDroppedCount.QuadPart);
		MTWF_PRINT("ChannelUtilization  = %u\n", pAd->Ch_BusyTime_11k[DBDC_BAND1]);
		MTWF_PRINT("RxCRCErrorCount     = %lld\n", ctrl->rdev[1].pRadioCtrl->RxCRCErrorCount.QuadPart);
		MTWF_PRINT("RxMICErrorCount     = %lld\n", ctrl->rdev[1].pRadioCtrl->RxMICErrorCount.QuadPart);
		MTWF_PRINT("RxDecyptionErrorCnt = %lld\n", ctrl->rdev[1].pRadioCtrl->RxDecryptionErrorCount.QuadPart);
		MTWF_PRINT("RxTotalErrorCnt     = %lld\n", (ctrl->rdev[1].pRadioCtrl->RxDecryptionErrorCount.QuadPart + ctrl->rdev[1].pRadioCtrl->RxMICErrorCount.QuadPart
			+ ctrl->rdev[1].pRadioCtrl->RxMICErrorCount.QuadPart + ctrl->rdev[1].pRadioCtrl->RxCRCErrorCount.QuadPart));
		MTWF_PRINT("ThroughPut(TX Kbps)	= %lld\n", (ctrl->rdev[1].pRadioCtrl->LastSecTxByte.QuadPart >> 7));
		MTWF_PRINT("ThroughPut(RX Kbps)	= %lld\n", (ctrl->rdev[1].pRadioCtrl->LastSecRxByte.QuadPart >> 7));
		MTWF_PRINT("PER(pct from start)	= %u\n", ctrl->rdev[1].pRadioCtrl->TotalPER);
		MTWF_PRINT("Total PER(Last Sec)	= %u\n", ctrl->rdev[1].pRadioCtrl->Last1SecPER);
		MTWF_PRINT("Data				BK		BE		VI		VO\n");
		MTWF_PRINT("Packets Sent	:	%10lld	%10lld	%10lld	%10lld\n",
				ctrl->rdev[1].pRadioCtrl->TxDataPacketCountPerAC[QID_AC_BK].QuadPart,
				ctrl->rdev[1].pRadioCtrl->TxDataPacketCountPerAC[QID_AC_BE].QuadPart,
				ctrl->rdev[1].pRadioCtrl->TxDataPacketCountPerAC[QID_AC_VI].QuadPart,
				ctrl->rdev[1].pRadioCtrl->TxDataPacketCountPerAC[QID_AC_VO].QuadPart);
		MTWF_PRINT("Packets Rcvd	:	%10lld	%10lld	%10lld	%10lld\n",
				ctrl->rdev[1].pRadioCtrl->RxDataPacketCountPerAC[QID_AC_BK].QuadPart,
				ctrl->rdev[1].pRadioCtrl->RxDataPacketCountPerAC[QID_AC_BE].QuadPart,
				ctrl->rdev[1].pRadioCtrl->RxDataPacketCountPerAC[QID_AC_VI].QuadPart,
				ctrl->rdev[1].pRadioCtrl->RxDataPacketCountPerAC[QID_AC_VO].QuadPart);
		MTWF_PRINT("RSSI				0		1		2		3\n");
		MTWF_PRINT("Data Rx		:		%d		%d		%d		%d\n",
					ctrl->rdev[1].pRadioCtrl->LastDataPktRssi[0],
					ctrl->rdev[1].pRadioCtrl->LastDataPktRssi[1],
					ctrl->rdev[1].pRadioCtrl->LastDataPktRssi[2],
					ctrl->rdev[1].pRadioCtrl->LastDataPktRssi[3]);
		MTWF_PRINT("----------------------------------------------\n");
	}
#endif
	return TRUE;
}

INT Show_Bss_Stat_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 Bss_Idx, i;
	BSS_STRUCT *pMbss = NULL;
	EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
	HTTRANSMIT_SETTING LastMbssTxPktRate;
	ULONG DataRate_Tx;
	ULONG Multicast_Tx_Rate;
	USHORT Multicast_Tx_MCS, Multicast_Tx_Mode;
#ifdef HIGHPRI_RATE_SPECIFIC
	UINT8 frame_type;
	ULONG HighPriority_Tx_Rate[HIGHPRI_MAX_TYPE];
	USHORT HighPriority_Tx_MCS[HIGHPRI_MAX_TYPE];
	USHORT HighPriority_Tx_Mode[HIGHPRI_MAX_TYPE];
#endif

	for (i = 0 ; i < pAd->ApCfg.BssidNum; i++) {
		pMbss = &pAd->ApCfg.MBSSID[i];
		if (pMbss->stat_bss.Last1TxCnt && pMbss->stat_bss.Last1TxFailCnt)
				pMbss->stat_bss.Last1SecPER = ((100 * (pMbss->stat_bss.Last1TxFailCnt))/
													pMbss->stat_bss.Last1TxCnt);
	}
	for (Bss_Idx = 0 ; Bss_Idx < pAd->ApCfg.BssidNum ; Bss_Idx++) {
		pMbss = &pAd->ApCfg.MBSSID[Bss_Idx];
		MTWF_PRINT("------------------BSS:%d-----------------------\n", Bss_Idx);
		MTWF_PRINT("TxDataPacketCnt     = %lld\n", pMbss->stat_bss.TxDataPacketCount.QuadPart);
		MTWF_PRINT("TxDataPacketByte    = %lld\n", pMbss->stat_bss.TxDataPacketByte.QuadPart);
		MTWF_PRINT("TxDataPayloadByte   = %lld\n", pMbss->stat_bss.TxDataPayloadByte.QuadPart);
		MTWF_PRINT("RxDataPacketCnt     = %lld\n", pMbss->stat_bss.RxDataPacketCount.QuadPart);
		MTWF_PRINT("RxDataPacketByte    = %lld\n", pMbss->stat_bss.RxDataPacketByte.QuadPart);
		MTWF_PRINT("RxDataPayloadByte   = %lld\n", pMbss->stat_bss.RxDataPayloadByte.QuadPart);
		MTWF_PRINT("TxMgmtPacketCnt     = %lld\n", pMbss->stat_bss.TxMgmtPacketCount.QuadPart);
		MTWF_PRINT("TxMgffChlPktCnt     = %lld\n", pMbss->stat_bss.TxMgmtOffChPktCount.QuadPart);
		MTWF_PRINT("RxMgmtPacketCnt     = %lld\n", pMbss->stat_bss.RxMgmtPacketCount.QuadPart);
		MTWF_PRINT("ucPktsTx            = %lld\n", pMbss->stat_bss.TxUnicastDataPacket.QuadPart);
		MTWF_PRINT("ucPktsRx            = %lld\n", pMbss->stat_bss.RxUnicastDataPacket.QuadPart);
		MTWF_PRINT("mcPktsTx            = %lld\n", pMbss->stat_bss.TxMulticastDataPacket.QuadPart);
		MTWF_PRINT("bcPktsTx            = %lld\n", pMbss->stat_bss.TxBroadcastDataPacket.QuadPart);
		MTWF_PRINT("TxDroppedPacketCnt  = %lld\n", pMbss->stat_bss.TxPacketDroppedCount.QuadPart);
		MTWF_PRINT("RxDroppedPacketCnt  = %lld\n", pMbss->stat_bss.RxPacketDroppedCount.QuadPart);
		MTWF_PRINT("AverageTXRate(Kbps) = %lld\n", (pMbss->stat_bss.LastSecTxBytes.QuadPart >> 7));
		MTWF_PRINT("AverageRXRate(Kbps) = %lld\n", (pMbss->stat_bss.LastSecRxBytes.QuadPart >> 7));
		MTWF_PRINT("PER (pcnt)          = %d\n", pMbss->stat_bss.Last1SecPER);
		MTWF_PRINT("TxRetriedPktCnt     = %lld\n", pMbss->stat_bss.TxRetriedPacketCount.QuadPart);
		MTWF_PRINT("RxMICErrorCount     = %lld\n", pMbss->stat_bss.RxMICErrorCount.QuadPart);
		MTWF_PRINT("RxDecryptionErrorCnt= %lld\n", pMbss->stat_bss.RxDecryptionErrorCount.QuadPart);
		if (VALID_UCAST_ENTRY_WCID(pAd, pMbss->stat_bss.LastPktStaWcid)) {
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[pMbss->stat_bss.LastPktStaWcid];
			if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC) && (pEntry->pMbss == pMbss)) {
				MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pMbss->stat_bss.LastPktStaWcid, &rTxStatResult);
				LastMbssTxPktRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
				LastMbssTxPktRate.field.BW = rTxStatResult.rEntryTxRate.BW;
				LastMbssTxPktRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
				LastMbssTxPktRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
				LastMbssTxPktRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;
				if (LastMbssTxPktRate.field.MODE == MODE_VHT)
					LastMbssTxPktRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
				else if (LastMbssTxPktRate.field.MODE == MODE_OFDM)
					LastMbssTxPktRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
				else
					LastMbssTxPktRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;
				getRate(LastMbssTxPktRate, &DataRate_Tx);
				MTWF_PRINT("MbssLastTxPktRate   = %lu\n", DataRate_Tx);
				MTWF_PRINT("MbssLastTxPktMCS    = %d\n", LastMbssTxPktRate.field.MCS);
			}
		}
		Multicast_Tx_MCS = pMbss->stat_bss.LastMulticastTxRate.field.MCS;
		Multicast_Tx_Mode = pMbss->stat_bss.LastMulticastTxRate.field.MODE;
		getRate(pMbss->stat_bss.LastMulticastTxRate, &Multicast_Tx_Rate);
		if (i >= 1) {
			BSS_STRUCT *FirstMbss = &pAd->ApCfg.MBSSID[0];
			Multicast_Tx_MCS =  FirstMbss->stat_bss.LastMulticastTxRate.field.MCS;
			getRate(FirstMbss->stat_bss.LastMulticastTxRate, &Multicast_Tx_Rate);
		}
#ifdef HIGHPRI_RATE_SPECIFIC
		for (frame_type = 0; frame_type < HIGHPRI_MAX_TYPE; frame_type++) {
			HighPriority_Tx_MCS[frame_type] = pMbss->stat_bss.LastHighPriorityTxRate[frame_type].field.MCS;
			HighPriority_Tx_Mode[frame_type] = pMbss->stat_bss.LastHighPriorityTxRate[frame_type].field.MODE;
			getRate(pMbss->stat_bss.LastHighPriorityTxRateARP, &HighPriority_TxRate[frame_type]);
			if (i >= 1) {
				BSS_STRUCT *FirstMbss = &pAd->ApCfg.MBSSID[0];

				HighPriority_Tx_MCS[frame_type] = FirstMbss->stat_bss.LastHighPriorityTxRate[frame_type].field.MCS;
				getRate(FirstMbss->stat_bss.LastHighPriorityTxRate[frame_type], &HighPriority_Tx_Rate[frame_type]);
			}
			switch (frame_type) {
			case HIGHPRI_ARP:
				MTWF_PRINT("HighPriority_Tx_Rate_ARP  = %lu\n", HighPriority_Tx_Rate[frame_type]);
				MTWF_PRINT("HighPriority_Tx_MCS_ARP   = %d\n", HighPriority_Tx_MCS[frame_type]);
				break;
			case HIGHPRI_DHCP:
				MTWF_PRINT("HighPriority_Tx_Rate_DHCP  = %lu\n", HighPriority_Tx_Rate[frame_type]);
				MTWF_PRINT("HighPriority_Tx_MCS_DHCP   = %d\n", HighPriority_Tx_MCS[frame_type]);
				break;
			case HIGHPRI_EAPOL:
				MTWF_PRINT("HighPriority_Tx_Rate_EAPOL  = %lu\n", HighPriority_Tx_Rate[frame_type]);
				MTWF_PRINT("HighPriority_Tx_MCS_EAPOL   = %d\n", HighPriority_Tx_MCS[frame_type]);
				break;
			default:
				break;
			}

		}
#endif
		MTWF_PRINT("MulitcastLastTxRate  = %lu\n", Multicast_Tx_Rate);
		MTWF_PRINT("MulitcastLastTxMCS   = %d\n", Multicast_Tx_MCS);
		MTWF_PRINT("Data				BK		BE		VI		VO\n");
		MTWF_PRINT("Packets Sent	:	%10lld	%10lld	%10lld	%10lld\n",
				pMbss->stat_bss.TxDataPacketCountPerAC[QID_AC_BK].QuadPart,
				pMbss->stat_bss.TxDataPacketCountPerAC[QID_AC_BE].QuadPart,
				pMbss->stat_bss.TxDataPacketCountPerAC[QID_AC_VI].QuadPart,
				pMbss->stat_bss.TxDataPacketCountPerAC[QID_AC_VO].QuadPart);
		MTWF_PRINT("Packets Rcvd	:	%10lld	%10lld	%10lld	%10lld\n",
				pMbss->stat_bss.RxDataPacketCountPerAC[QID_AC_BK].QuadPart,
				pMbss->stat_bss.RxDataPacketCountPerAC[QID_AC_BE].QuadPart,
				pMbss->stat_bss.RxDataPacketCountPerAC[QID_AC_VI].QuadPart,
				pMbss->stat_bss.RxDataPacketCountPerAC[QID_AC_VO].QuadPart);
		MTWF_PRINT("----------------------------------------------\n");
	}
	return TRUE;
}
INT Show_Sta_Stat_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
			ULONG DataRate_Tx;
			ULONG DataRate_Rx;
			EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
			HTTRANSMIT_SETTING LastTxRate;
			HTTRANSMIT_SETTING LastRxRate;
			UINT32 RawData, RawData_r;
			UCHAR bw, stbc;
			UCHAR phy_mode_r, rate_r, bw_r, stbc_r;
			UINT32 lastTxRate, lastRxRate;
			lastRxRate = pEntry->LastRxRate;
			RawData_r = lastRxRate;
			phy_mode_r = (RawData_r >> 13) & 0x7;
			rate_r = RawData_r & 0x3F;
			bw_r = (RawData_r >> 7) & 0x3;
			stbc_r = ((RawData_r >> 10) & 0x1);
			MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
			LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
			LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
			LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
			LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
			LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;
			if (LastTxRate.field.MODE == MODE_VHT)
				LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
			else if (LastTxRate.field.MODE == MODE_OFDM)
				LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
			else
				LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;
			lastTxRate = (UINT32)(LastTxRate.word);
			LastRxRate.word = (USHORT)(pEntry->LastRxRate);
			RawData = lastTxRate;
			bw = (RawData >> 7) & 0x3;
			stbc = ((RawData >> 10) & 0x1);
			if (phy_mode_r == MODE_OFDM) {
							if (rate_r == TMI_TX_RATE_OFDM_6M)
								LastRxRate.field.MCS = 0;
							else if (rate_r == TMI_TX_RATE_OFDM_9M)
								LastRxRate.field.MCS = 1;
							else if (rate_r == TMI_TX_RATE_OFDM_12M)
								LastRxRate.field.MCS = 2;
							else if (rate_r == TMI_TX_RATE_OFDM_18M)
								LastRxRate.field.MCS = 3;
							else if (rate_r == TMI_TX_RATE_OFDM_24M)
								LastRxRate.field.MCS = 4;
							else if (rate_r == TMI_TX_RATE_OFDM_36M)
								LastRxRate.field.MCS = 5;
							else if (rate_r == TMI_TX_RATE_OFDM_48M)
								LastRxRate.field.MCS = 6;
							else if (rate_r == TMI_TX_RATE_OFDM_54M)
								LastRxRate.field.MCS = 7;
							else
								LastRxRate.field.MCS = 0;

			} else if (phy_mode_r == MODE_CCK) {
							if (rate_r == TMI_TX_RATE_CCK_1M_LP)
								LastRxRate.field.MCS = 0;
							else if (rate_r == TMI_TX_RATE_CCK_2M_LP)
								LastRxRate.field.MCS = 1;
							else if (rate_r == TMI_TX_RATE_CCK_5M_LP)
								LastRxRate.field.MCS = 2;
							else if (rate_r == TMI_TX_RATE_CCK_11M_LP)
								LastRxRate.field.MCS = 3;
							else if (rate_r == TMI_TX_RATE_CCK_2M_SP)
								LastRxRate.field.MCS = 1;
							else if (rate_r == TMI_TX_RATE_CCK_5M_SP)
								LastRxRate.field.MCS = 2;
							else if (rate_r == TMI_TX_RATE_CCK_11M_SP)
								LastRxRate.field.MCS = 3;
							else
								LastRxRate.field.MCS = 0;
			}
			getRate(LastTxRate, &DataRate_Tx);
			getRate(LastRxRate, &DataRate_Rx);
			MTWF_PRINT("-----------------WCID:%d-----------------------\n", i);
			MTWF_PRINT("MAC                 = "MACSTR"\n", MAC2STR(pEntry->Addr));
			MTWF_PRINT("TxDataPacketCnt     = %lld\n", pEntry->TxDataPacketCount.QuadPart);
			MTWF_PRINT("TxDataPacketByte    = %lld\n", pEntry->TxDataPacketByte.QuadPart);
			MTWF_PRINT("TxSuccessCount      = %u\n", pEntry->TxSuccessByWtbl);
			MTWF_PRINT("TxUnicastSuccessCnt = %u\n", pEntry->TxSuccessByWtbl);
			MTWF_PRINT("RxDataPacketCnt     = %lld\n", pEntry->RxDataPacketCount.QuadPart);
			MTWF_PRINT("RxDataPacketByte    = %lld\n", pEntry->RxDataPacketByte.QuadPart);
			MTWF_PRINT("Data				BK		BE		VI		VO\n");
			MTWF_PRINT("Packets Sent	:	%10lld	%10lld	%10lld	%10lld\n",
				pEntry->TxDataPacketCountPerAC[QID_AC_BK].QuadPart,
				pEntry->TxDataPacketCountPerAC[QID_AC_BE].QuadPart,
				pEntry->TxDataPacketCountPerAC[QID_AC_VI].QuadPart,
				pEntry->TxDataPacketCountPerAC[QID_AC_VO].QuadPart);
			MTWF_PRINT("Packets Rcvd	:	%10lld	%10lld	%10lld	%10lld\n",
				pEntry->RxDataPacketCountPerAC[QID_AC_BK].QuadPart,
				pEntry->RxDataPacketCountPerAC[QID_AC_BE].QuadPart,
				pEntry->RxDataPacketCountPerAC[QID_AC_VI].QuadPart,
				pEntry->RxDataPacketCountPerAC[QID_AC_VO].QuadPart);
			MTWF_PRINT("TxMgmtPacketCnt     = %lld\n", pEntry->TxMgmtPacketCount.QuadPart);
			MTWF_PRINT("RxMgmtPacketCnt     = %lld\n", pEntry->RxMgmtPacketCount.QuadPart);
			MTWF_PRINT("LastSecTxPackets    = %lld\n", pEntry->TxDataPacketCount1SecValue.QuadPart);
			MTWF_PRINT("LastSecRxPackets    = %lld\n", pEntry->RxDataPacketCount1SecValue.QuadPart);
			MTWF_PRINT("LastSecTxBytes      = %lld\n", pEntry->TxDataPacketByte1SecValue.QuadPart);
			MTWF_PRINT("LastSecRxBytes      = %lld\n", pEntry->RxDataPacketByte1SecValue.QuadPart);
			MTWF_PRINT("AverageTXRate(Kbps) = %lld\n", (pEntry->TxDataPacketByte1SecValue.QuadPart >> 7));
			MTWF_PRINT("AverageRXRate(Kbps) = %lld\n", (pEntry->RxDataPacketByte1SecValue.QuadPart >> 7));
			MTWF_PRINT("Bandwidth(TX/RX)    = %s / %s\n", get_bw_str(bw), get_bw_str(bw_r));
			MTWF_PRINT("STBC(TX/RX)         = %d / %d\n", stbc, stbc_r);
			MTWF_PRINT("LastTXDataRate(Mbps)= %lu\n", DataRate_Tx);
			MTWF_PRINT("LastRXDataRate(Mbps)= %lu\n", DataRate_Rx);
			MTWF_PRINT("LastRXMgmtRate(Mbps)= %lu\n", pEntry->RxLastMgmtPktRate);
			MTWF_PRINT("LastOneSecPER(prcnt)= %u\n", pEntry->LastOneSecPER);
			MTWF_PRINT("RxMICErrorCount     = %lld\n", pEntry->RxMICErrorCount.QuadPart);
			MTWF_PRINT("RxDecryptionErrorCnt= %lld\n", pEntry->RxDecryptionErrorCount.QuadPart);
			MTWF_PRINT("RSSI					0		1		2		3\n");
			MTWF_PRINT("Data Rx			:		%d		%d		%d		%d\n",
					pEntry->LastDataPktRssi[0],
					pEntry->LastDataPktRssi[1],
					pEntry->LastDataPktRssi[2],
					pEntry->LastDataPktRssi[3]);
			MTWF_PRINT("Mgmt Rx			:		%d		%d		%d		%d\n",
					pEntry->LastMgmtPktRssi[0],
					pEntry->LastMgmtPktRssi[1],
					pEntry->LastMgmtPktRssi[2],
					pEntry->LastMgmtPktRssi[3]);
			MTWF_PRINT("ack  Rx			:		%d		%d		%d		%d\n",
					pEntry->RssiSample.AvgRssi[0],
					pEntry->RssiSample.AvgRssi[1],
					pEntry->RssiSample.AvgRssi[2],
					pEntry->RssiSample.AvgRssi[3]);
		}
	}
	return TRUE;
}

INT Set_Enable_RSSI_Stats(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG RssiEnable = os_str_tol(arg, 0, 10);
	if (RssiEnable == 1) {
		pAd->TXRX_EnableReadRssi = TRUE;
		MTWF_PRINT("RSSI calculation enabled \n");
	} else if (RssiEnable == 0) {
		pAd->TXRX_EnableReadRssi = FALSE;
		MTWF_PRINT("RSSI calculation disabled \n");
	} else {
		MTWF_PRINT("Invalid Value \n");
		return FALSE;
	}
	return TRUE;
}

INT Set_Enable_Last_Sec_TXRX_Stats(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG TxRxStats = os_str_tol(arg, 0, 10);
	if (TxRxStats == 1) {
		pAd->EnableTxRxStats = TRUE;
		MTWF_PRINT("TxRx calculation enabled \n");
	} else if (TxRxStats == 0) {
		pAd->EnableTxRxStats = FALSE;
		MTWF_PRINT("TxRx calculation disabled \n");
	} else{
		MTWF_PRINT("Invalid Value \n");
		return FALSE;
	}
	return TRUE;
}
#endif

#ifdef EAP_STATS_SUPPORT
INT Show_Eap_Stats_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 Wcid;
    MAC_TABLE_ENTRY *pEntry = NULL;

	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Please enter in the format \"iwpriv <inf_name> show ubnt_stats=<sta wcid>\"\n");
		return TRUE;
	}

	Wcid = simple_strtol(arg, 0, 10);

    if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid Wcid!!\n");
		return TRUE;
    } else {
		pEntry = &pAd->MacTab.Content[Wcid];
		if (IS_VALID_ENTRY(pEntry) && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {

			MTWF_PRINT("WCID\tMAC Address\t\tmpdu_attempts\tmpdu_retries\tmpdu_dropped(HW)\t"
					  "minimum latency(TU)\tmaximum latency(TU)\taverage latency(TU)"
#ifdef EAP_ENHANCE_STATS_SUPPORT
					  "\tminimum Airlatency(TU)\tmaximum Airlatency(TU)\taverage Airlatency(TU)"
					  "\tminimum TxCnt\tmaximum TxCnt\taverage TxCnt"
#endif /* EAP_ENHANCE_STATS_SUPPORT */
					  "\n");
			MTWF_PRINT("%d\t%02X:%02X:%02X:%02X:%02X:%02X\t",
					  pEntry->wcid, PRINT_MAC(pEntry->Addr));
			MTWF_PRINT("%10lld\t%10lld\t%10lld\t", pEntry->mpdu_attempts.QuadPart, pEntry->mpdu_retries.QuadPart, pEntry->mpdu_xretries.QuadPart);

			MTWF_PRINT("%20d\t%20d\t%20d", pEntry->tx_latency_min, pEntry->tx_latency_max, pEntry->tx_latency_avg);

#ifdef EAP_ENHANCE_STATS_SUPPORT
			MTWF_PRINT("\t%20d\t%20d\t%20d", pEntry->air_latency_min, pEntry->air_latency_max, pEntry->air_latency_avg);
			MTWF_PRINT("\t%20d\t%20d\t%20d", pEntry->tx_cnt_min, pEntry->tx_cnt_max, pEntry->tx_cnt_avg);
#endif /* EAP_ENHANCE_STATS_SUPPORT */

			MTWF_PRINT("\n");
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Check pEntry or association state\n");
			return TRUE;
		}
	}
	return TRUE;
}
#endif /* EAP_STATS_SUPPORT */

INT Show_StaCount_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	ADD_HT_INFO_IE *addht;

	if (!wdev)
		return FALSE;

	addht = wlan_operate_get_addht(wdev);
	MTWF_PRINT("\nHT Operating Mode : %d\n", addht->AddHtInfo2.OperaionMode);
	MTWF_PRINT("\n\n%-19s%-4s%-12s%-12s%-12s%-12s%-12s%-12s\n",
			  "MAC", "AID", "TxPackets", "RxPackets", "TxBytes", "RxBytes", "TP(Tx)", "TP(Rx)");

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
			&& (pEntry->Sst == SST_ASSOC)) || IS_ENTRY_WDS(pEntry)) {
			MTWF_PRINT(MACSTR"  ", MAC2STR(pEntry->Addr));
			MTWF_PRINT("%-4d", (int)pEntry->Aid);
			MTWF_PRINT("%-12lu", (ULONG)pEntry->TxPackets.QuadPart);
			MTWF_PRINT("%-12lu", (ULONG)pEntry->RxPackets.QuadPart);
			MTWF_PRINT("%-12lu", (ULONG)pEntry->TxBytes);
			MTWF_PRINT("%-12lu", (ULONG)pEntry->RxBytes);
#ifdef CONFIG_MAP_SUPPORT
			if (IS_MAP_ENABLE(pAd)) {
				MTWF_PRINT("%-4u",(UINT32)pEntry->TxBytesMAP);
				MTWF_PRINT("%-4u",(UINT32)pEntry->RxBytesMAP);
			}
#endif
			MTWF_PRINT("%lu %-12s", (pEntry->AvgTxBytes >> 17), "Mbps"); /* (n Bytes x 8) / (1024*1024) = (n >> 17) */
			MTWF_PRINT("%lu %-12s", (pEntry->AvgRxBytes >> 17), "Mbps");/* (n Bytes x 8) / (1024*1024) = (n >> 17) */
			MTWF_PRINT("\n");
		}
	}

	return TRUE;
}

INT Show_RAInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef NEW_RATE_ADAPT_SUPPORT
	MTWF_PRINT("LowTrafficThrd: %d\n", pAd->CommonCfg.lowTrafficThrd);
	MTWF_PRINT("TrainUpRule: %d\n", pAd->CommonCfg.TrainUpRule);
	MTWF_PRINT("TrainUpRuleRSSI: %d\n", pAd->CommonCfg.TrainUpRuleRSSI);
	MTWF_PRINT("TrainUpLowThrd: %d\n", pAd->CommonCfg.TrainUpLowThrd);
	MTWF_PRINT("TrainUpHighThrd: %d\n", pAd->CommonCfg.TrainUpHighThrd);
#endif /* NEW_RATE_ADAPT_SUPPORT */
#ifdef STREAM_MODE_SUPPORT
	MTWF_PRINT("StreamMode: %d\n", pAd->CommonCfg.StreamMode);
	MTWF_PRINT("StreamModeMCS: 0x%04x\n", pAd->CommonCfg.StreamModeMCS);
#endif /* STREAM_MODE_SUPPORT */
#ifdef TXBF_SUPPORT
	MTWF_PRINT( "ITxBfEn: %d\n", pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn);
	MTWF_PRINT("ITxBfTimeout: %ld\n", pAd->CommonCfg.ITxBfTimeout);
	MTWF_PRINT("ETxBfTimeout: %ld\n", pAd->CommonCfg.ETxBfTimeout);
	MTWF_PRINT("CommonCfg.ETxBfEnCond: %ld\n", pAd->CommonCfg.ETxBfEnCond);
	MTWF_PRINT("ETxBfNoncompress: %d\n", pAd->CommonCfg.ETxBfNoncompress);
	MTWF_PRINT("ETxBfIncapable: %d\n", pAd->CommonCfg.ETxBfIncapable);
#ifdef TXBF_DYNAMIC_DISABLE
	MTWF_PRINT("ucAutoSoundingCtrl: %d\n", pAd->CommonCfg.ucAutoSoundingCtrl);
#endif /* TXBF_DYNAMIC_DISABLE */
#endif /* TXBF_SUPPORT */
	return TRUE;
}

#ifdef TXBF_SUPPORT
INT Show_TxBfInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
    struct wifi_dev *wdev;
    HT_CAPABILITY_IE *ht_cap, HtCapabilityTmp;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
    HT_BF_CAP *pTxBFCap;
	UCHAR ucBandIdx = 0, ucEBfCap;
#ifdef VHT_TXBF_SUPPORT
	VHT_CAP_INFO vht_cap;
#endif /* VHT_TXBF_SUPPORT */


#ifdef HE_TXBF_SUPPORT
	enum PHY_CAP phy_caps;
	struct he_bf_info he_bf_struct;
#endif /* HE_TXBF_SUPPPORT*/


	if (cap->FlgHwTxBfCap) {
		CHAR *tmp = NULL;
		UINT8 bfee;
		UINT16 wcid;

		tmp = strsep(&arg, ":");

		if (tmp != NULL)
			bfee = os_str_tol(tmp, 0, 10);
		else
			bfee = 0;

		if (bfee) {
			PMAC_TABLE_ENTRY pEntry;
			BOOLEAN fgETxBfCap = FALSE;
			BOOLEAN fgSU_MU = FALSE;
			UINT8 ucTxMode = 0;
			UINT8 ucBw = 0;
			UINT8 ucBFeeMaxNr = 0;
			UINT8 ucPeerRxNumSupport = 0;
			UINT8 ucNc = 0;
			UINT8 ucTxMCSSetdefined = 0;
			UINT8 ucTxRxMCSSetNotEqual = 0;
			UINT8 ucTxMaxNumSpatilStream = 0;
			UINT8 u1PeerMaxRxNss = 0;

			tmp = strsep(&arg, "");

			if (tmp != NULL)
				wcid = os_str_tol(tmp, 0, 10);
			else
				return FALSE;

			if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
				return FALSE;

			pEntry = &pAd->MacTab.Content[wcid];

			switch (pEntry->MaxHTPhyMode.field.MODE) {
#ifdef HE_TXBF_SUPPORT
			case MODE_HE:
				ucTxMode = MODE_HE_SU;
				for (u1PeerMaxRxNss = 0; u1PeerMaxRxNss < DOT11AX_MAX_STREAM; u1PeerMaxRxNss++) {
					if (pEntry->cap.rate.he80_tx_nss_mcs[u1PeerMaxRxNss] == 3) /* 3 for not support */
						break;
				}
				MTWF_PRINT("Wcid:%d\n", wcid);
				MTWF_PRINT("TxMode:%d\n", MODE_HE_SU);
				MTWF_PRINT("MaxHTPhyMode.field.BW:%d\n", pEntry->MaxHTPhyMode.field.BW);
				MTWF_PRINT("he_ch_width:%d\n", pEntry->cap.ch_bw.he_ch_width);
				MTWF_PRINT("Rx Max Nss:%d\n", u1PeerMaxRxNss);
				MTWF_PRINT("SU Beamformer:%u\n", (pEntry->cap.he_bf.bf_cap & HE_SU_BFER)?1:0);
				MTWF_PRINT("SU Beamformee:%u\n", (pEntry->cap.he_bf.bf_cap & HE_SU_BFEE)?1:0);
				MTWF_PRINT("MU Beamformer:%u\n", (pEntry->cap.he_bf.bf_cap & HE_MU_BFER)?1:0);
				MTWF_PRINT("Beamformee STS <= 80MHz:%d\n", pEntry->cap.he_bf.bfee_sts_le_eq_bw80);
				MTWF_PRINT("Beamformee STS > 80MHz:%d\n", pEntry->cap.he_bf.bfee_sts_gt_bw80);
				MTWF_PRINT("Number of Sounding Dimension <= 80 MHz:%d\n", pEntry->cap.he_bf.snd_dim_le_eq_bw80);
				MTWF_PRINT("Number of Sounding Dimension > 80MHz:%d\n", pEntry->cap.he_bf.snd_dim_gt_bw80);
				MTWF_PRINT("Ng = 16 SU Feedback:%d\n", (pEntry->cap.he_bf.bf_cap & HE_BFEE_NG_16_SU_FEEDBACK)?1:0);
				MTWF_PRINT("Ng = 16 MU Feedback :%d\n", (pEntry->cap.he_bf.bf_cap & HE_BFEE_NG_16_MU_FEEDBACK)?1:0);
				MTWF_PRINT("Codebook Size {4,2} SU Feedback:%d\n", (pEntry->cap.he_bf.bf_cap & HE_BFEE_CODEBOOK_SU_FEEDBACK)?1:0);
				MTWF_PRINT("Codebook Size {7,5} MU Feedback:%d\n", (pEntry->cap.he_bf.bf_cap & HE_BFEE_CODEBOOK_MU_FEEDBACK)?1:0);
				MTWF_PRINT("Triggered SU Beamforming Feedback:%d\n", (pEntry->cap.he_bf.bf_cap & HE_TRIG_SU_BFEE_FEEDBACK)?1:0);
				MTWF_PRINT("Triggered MU Beamforing Partial BW Feedback:%d\n", (pEntry->cap.he_bf.bf_cap & HE_TRIG_MU_BFEE_FEEDBACK)?1:0);
				MTWF_PRINT("Triggered CQI Feedback:%d\n", (pEntry->cap.he_phy_cap & HE_TRIG_CQI_FEEDBACK)?1:0);
				MTWF_PRINT("Max Nc :%d\n", pEntry->cap.he_bf.bfee_max_nc);

				break;
#endif /* HE_TXBF_SUPPORT */
#ifdef VHT_TXBF_SUPPORT
			case MODE_VHT:
				fgETxBfCap = mt_WrapClientSupportsVhtETxBF(pAd, &pEntry->vht_cap_ie.vht_cap);
				/*
				fgSuBfer = pEntry->vht_cap_ie.vht_cap.bfer_cap_su;
				fgSuBfee = pEntry->vht_cap_ie.vht_cap.bfee_cap_su;
				fgMuBfer = pEntry->vht_cap_ie.vht_cap.bfer_cap_mu;
				fgMuBfee = pEntry->vht_cap_ie.vht_cap.bfee_cap_mu;
				*/
				ucTxMode = MODE_VHT; /* VHT mode */
				ucBw = pEntry->MaxHTPhyMode.field.BW;
				fgSU_MU = pEntry->vht_cap_ie.vht_cap.bfee_cap_mu;

				ucBFeeMaxNr = pEntry->vht_cap_ie.vht_cap.bfee_sts_cap;
				ucPeerRxNumSupport = (pEntry->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 != 3) ? 1 : 0;
				ucPeerRxNumSupport = (pEntry->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss3 != 3) ? 2 : ucPeerRxNumSupport;
				ucPeerRxNumSupport = (pEntry->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss4 != 3) ? 3 : ucPeerRxNumSupport;
				ucNc = ucPeerRxNumSupport;
				break;
#endif /* VHT_TXBF_SUPPORT */

			case MODE_HTMIX:
			case MODE_HTGREENFIELD:
				fgETxBfCap = mt_WrapClientSupportsETxBF(pAd, &pEntry->HTCapability.TxBFCap);
				ucTxMode = MODE_HTMIX;
				ucBw = pEntry->MaxHTPhyMode.field.BW;
				ucBFeeMaxNr = pEntry->HTCapability.TxBFCap.ComSteerBFAntSup;
				ucTxMCSSetdefined = ((pEntry->HTCapability.MCSSet[12] &
							TX_MCS_SET_DEFINED) >> TX_MCS_SET_DEFINED_OFFSET);
				ucTxRxMCSSetNotEqual = ((pEntry->HTCapability.MCSSet[12] &
							TX_RX_MCS_SET_N_EQUAL) >> TX_RX_MCS_SET_N_EQUAL_OFFSET);
				ucTxMaxNumSpatilStream = ((pEntry->HTCapability.MCSSet[12] &
							TX_MAX_NUM_SPATIAL_STREAMS_SUPPORTED)
						>> TX_MAX_NUM_SPATIAL_STREAMS_SUPPORTED_OFFSET);
				ucPeerRxNumSupport = (pEntry->HTCapability.MCSSet[1] > 0) ? 1 : 0;
				ucPeerRxNumSupport = (pEntry->HTCapability.MCSSet[2] > 0) ? 2 : ucPeerRxNumSupport;
				ucPeerRxNumSupport = (pEntry->HTCapability.MCSSet[3] > 0) ? 3 : ucPeerRxNumSupport;
				ucNc = ucPeerRxNumSupport;
				if ((ucTxMCSSetdefined == 1) && (ucTxRxMCSSetNotEqual == 1))
					ucNc = ucTxMaxNumSpatilStream;
				break;

			case MODE_OFDM:
			case MODE_CCK:
				ucTxMode = 1;
				ucNc = 0;
				ucBw = pEntry->MaxHTPhyMode.field.BW;
				break;

			default:
				break;
			}

			if (ucTxMode != MODE_HE_SU) {
				MTWF_PRINT("Wcid:%d\n", wcid);
				MTWF_PRINT("Peer_TxMode:%d\n", ucTxMode);
				MTWF_PRINT("Peer_ETxBfCap:%d\n", fgETxBfCap);
#ifdef VHT_TXBF_SUPPORT
				MTWF_PRINT("Peer_SUMU:%d\n", fgSU_MU);
#endif /* VHT_TXBF_SUPPORT */
				MTWF_PRINT("Peer_Bw:%d\n", ucBw);
				MTWF_PRINT("Peer_BFeeNr:%d\n", ucBFeeMaxNr);
				MTWF_PRINT("Peer_RxNumSupport:%d\n", ucPeerRxNumSupport);
				MTWF_PRINT("Peer_Nc:%d\n", ucNc);
			}
		} else {
			wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
			if (wdev->pHObj) {
				ucBandIdx = HcGetBandByWdev(wdev);
			}
			ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
			NdisMoveMemory(&HtCapabilityTmp, ht_cap, sizeof(HT_CAPABILITY_IE));
			if (HcIsBfCapSupport(wdev) == FALSE) {
			    ucEBfCap = wlan_config_get_etxbf(wdev);
			    wlan_config_set_etxbf(wdev, SUBF_OFF);
			    mt_WrapSetETxBFCap(pAd, wdev, &HtCapabilityTmp.TxBFCap);
			    wlan_config_set_etxbf(wdev, ucEBfCap);
			}
			pTxBFCap = &HtCapabilityTmp.TxBFCap;
			MTWF_PRINT("AP\n");
			MTWF_PRINT("Band Index:%d\n", ucBandIdx);
			MTWF_PRINT("IsBfBand:%d\n", HcIsBfCapSupport(wdev));
			MTWF_PRINT("CommonCfg.ETxBfEnCond:%ld\n", pAd->CommonCfg.ETxBfEnCond);
			MTWF_PRINT("ETxBfEnCond:%d\n", wlan_config_get_etxbf(wdev));
			MTWF_PRINT("CommonCfg.ITxBfEn:%d\n", pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn);
			MTWF_PRINT("ITxBfEn:%d\n", wlan_config_get_itxbf(wdev));
			MTWF_PRINT("BfSmthIntlBypass[%u]:%u\n", ucBandIdx, pAd->CommonCfg.BfSmthIntlBypass[ucBandIdx]);
			MTWF_PRINT("HeraStbcPriority[%u]:%u\n", ucBandIdx, pAd->CommonCfg.HeraStbcPriority[ucBandIdx]);
			MTWF_PRINT("bfdm_bitmap:%d\n", pAd->bfdm.bfdm_bitmap);
			MTWF_PRINT("bfdm_bfee_enabled:%d\n", pAd->bfdm.bfdm_bfee_enabled);
			MTWF_PRINT("HT TxBF Cap:\n");
			MTWF_PRINT("  TxBFRecCapable:%d\n", pTxBFCap->TxBFRecCapable);
			MTWF_PRINT("  RxSoundCapable:%d\n", pTxBFCap->RxSoundCapable);
			MTWF_PRINT("  TxSoundCapable:%d\n", pTxBFCap->TxSoundCapable);
			MTWF_PRINT("  RxNDPCapable:%d\n", pTxBFCap->RxNDPCapable);
			MTWF_PRINT("  TxNDPCapable:%d\n", pTxBFCap->TxNDPCapable);
			MTWF_PRINT("  ImpTxBFCapable:%d\n", pTxBFCap->ImpTxBFCapable);
			MTWF_PRINT("  Calibration:%d\n", pTxBFCap->Calibration);
			MTWF_PRINT("  ExpCSICapable:%d\n", pTxBFCap->ExpCSICapable);
			MTWF_PRINT("  ExpNoComSteerCapable:%d\n", pTxBFCap->ExpNoComSteerCapable);
			MTWF_PRINT("  ExpComSteerCapable:%d\n", pTxBFCap->ExpComSteerCapable);
			MTWF_PRINT("  ExpCSIFbk:%d\n", pTxBFCap->ExpCSIFbk);
			MTWF_PRINT("  ExpNoComBF:%d\n", pTxBFCap->ExpNoComBF);
			MTWF_PRINT("  ExpComBF:%d\n", pTxBFCap->ExpComBF);
			MTWF_PRINT("  MinGrouping:%d\n", pTxBFCap->MinGrouping);
			MTWF_PRINT("  CSIBFAntSup:%d\n", pTxBFCap->CSIBFAntSup);
			MTWF_PRINT("  NoComSteerBFAntSup:%d\n", pTxBFCap->NoComSteerBFAntSup);
			MTWF_PRINT("  ComSteerBFAntSup:%d\n", pTxBFCap->ComSteerBFAntSup);
			MTWF_PRINT("  CSIRowBFSup:%d\n", pTxBFCap->CSIRowBFSup);
			MTWF_PRINT("  ChanEstimation:%d\n", pTxBFCap->ChanEstimation);
			MTWF_PRINT("  rsv:%d\n", pTxBFCap->rsv);
#ifdef VHT_TXBF_SUPPORT
			NdisCopyMemory(&vht_cap, &pAd->CommonCfg.vht_cap_ie.vht_cap, sizeof(VHT_CAP_INFO));

			ucEBfCap = wlan_config_get_etxbf(wdev);
			if (HcIsBfCapSupport(wdev) == FALSE) {
				wlan_config_set_etxbf(wdev, SUBF_OFF);
			}
			mt_WrapSetVHTETxBFCap(pAd, wdev, &vht_cap);
			wlan_config_set_etxbf(wdev, ucEBfCap);

			MTWF_PRINT("VHT TxBF Cap:\n");
			MTWF_PRINT("  num_snd_dimension:%d\n", vht_cap.num_snd_dimension);
			MTWF_PRINT("  bfee_sts_cap:%d\n", vht_cap.bfee_sts_cap);
			MTWF_PRINT("  bfee_cap_su:%d\n", vht_cap.bfee_cap_su);
			MTWF_PRINT("  bfer_cap_su:%d\n", vht_cap.bfer_cap_su);

#endif /* VHT_TXBF_SUPPORT */


#ifdef HE_TXBF_SUPPORT
			if (wdev->pHObj) {
				phy_caps = wlan_config_get_phy_caps(wdev);
			} else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid pHObj in wdev\n");
				phy_caps = 0;
			}

			if (IS_PHY_CAPS(phy_caps, fPHY_CAP_TXBF)) {
				NdisZeroMemory(&he_bf_struct, sizeof(struct he_bf_info));
#ifdef DOT11_HE_AX
				mt_wrap_get_he_bf_cap(wdev, &he_bf_struct);
#endif
				MTWF_PRINT("HE TxBF Cap from he_bf_struct:\n");
				MTWF_PRINT("  SU Beamformer:%d\n", (he_bf_struct.bf_cap & HE_SU_BFER)?1:0);
				MTWF_PRINT("  SU Beamformee:%d\n", (he_bf_struct.bf_cap & HE_SU_BFEE)?1:0);
				MTWF_PRINT("  MU Beamformer:%d\n", (he_bf_struct.bf_cap & HE_MU_BFER)?1:0);
				MTWF_PRINT("  Beamformee STS <= 80MHz:%d\n", he_bf_struct.bfee_sts_le_eq_bw80);
				MTWF_PRINT("  Beamformee STS > 80MHz:%d\n", he_bf_struct.bfee_sts_gt_bw80);
				MTWF_PRINT("  Number Of Sounding Dimensions <= 80MHz:%d\n", he_bf_struct.snd_dim_le_eq_bw80);
				MTWF_PRINT("  Number Of Sounding Dimensions > 80MHz:%d\n", he_bf_struct.snd_dim_gt_bw80);
				MTWF_PRINT("  Ng = 16 SU Feedback:%d\n", (he_bf_struct.bf_cap & HE_BFEE_NG_16_SU_FEEDBACK)?1:0);
				MTWF_PRINT("  Ng = 16 MU Feedback:%d\n", (he_bf_struct.bf_cap & HE_BFEE_NG_16_MU_FEEDBACK)?1:0);
				MTWF_PRINT("  Codebook Size = {4,2} SU Feedback:%d\n", (he_bf_struct.bf_cap & HE_BFEE_CODEBOOK_SU_FEEDBACK)?1:0);
				MTWF_PRINT("  Codebook Size = {7,5} MU Feedback:%d\n", (he_bf_struct.bf_cap & HE_BFEE_CODEBOOK_MU_FEEDBACK)?1:0);
				MTWF_PRINT("  Triggered SU Beamforming Feedback:%d\n", (he_bf_struct.bf_cap & HE_TRIG_SU_BFEE_FEEDBACK)?1:0);
				MTWF_PRINT("  Triggered MU Beamforming Feedback:%d\n", (he_bf_struct.bf_cap & HE_TRIG_MU_BFEE_FEEDBACK)?1:0);
				MTWF_PRINT("  Max Nc:%d\n", he_bf_struct.bfee_max_nc);
			} else {
				MTWF_PRINT("Wlan Device has no HE TxBF Cap\n");
			}
#endif /* HE_TXBF_SUPPORT */


		}
	}
    return TRUE;
}
#endif /* TXBF_SUPPORT */

#ifdef RTMP_MAC_PCI
#ifdef DBG_DIAGNOSE
INT Set_DiagOpt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG diagOpt;
	/*POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie; */
	diagOpt = os_str_tol(arg, 0, 10);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "DiagOpt=%ld!\n", diagOpt);
	return TRUE;
}

INT Set_diag_cond_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 cond;

	cond = os_str_tol(arg, 0, 10);
	pAd->DiagStruct.diag_cond = cond;
	return TRUE;
}

INT Show_Diag_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RtmpDiagStruct *pDiag = NULL;
	UCHAR i, start, stop, que_idx;
	unsigned long irqFlags;

	os_alloc_mem(pAd, (UCHAR **)&pDiag, sizeof(RtmpDiagStruct));

	if (!pDiag) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "AllocMem failed!\n");
		return FALSE;
	}

	RTMP_IRQ_LOCK(&pAd->irq_lock, irqFlags);
	NdisMoveMemory(pDiag, &pAd->DiagStruct, sizeof(RtmpDiagStruct));
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqFlags);

	if (pDiag->inited == FALSE)
		goto done;

	start = pDiag->ArrayStartIdx;
	stop = pDiag->ArrayCurIdx;
	MTWF_PRINT("Start=%d, stop=%d!\n\n", start, stop);
	MTWF_PRINT("    %-12s", "Time(Sec)");

	for (i = 1; i < DIAGNOSE_TIME; i++)
		MTWF_PRINT("%-7d", i);

	MTWF_PRINT("\n    -------------------------------------------------------------------------------\n");
	MTWF_PRINT("Tx Info:\n");
	MTWF_PRINT("    %-12s", "TxDataCnt\n");

	for (que_idx = 0; que_idx < WMM_NUM_OF_AC; que_idx++) {
		MTWF_PRINT("\tQueue[%d]:", que_idx);

		for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
			MTWF_PRINT("%-7d", pDiag->diag_info[i].TxDataCnt[que_idx]);

		MTWF_PRINT("\n");
	}

	MTWF_PRINT("\n    %-12s", "TxFailCnt");

	for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
		MTWF_PRINT("%-7d", pDiag->diag_info[i].TxFailCnt);

#ifdef DBG_TX_AGG_CNT
	MTWF_PRINT("\n    %-12s", "TxAggCnt");

	for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
		MTWF_PRINT("%-7d", pDiag->diag_info[i].TxAggCnt);

	MTWF_PRINT("\n");
#endif /* DBG_TX_AGG_CNT */
#ifdef DBG_TXQ_DEPTH
	MTWF_PRINT("DeQueue Info:\n");
	MTWF_PRINT("\n    %-12s\n", "DeQueueFunc Called Distribution");
	MTWF_PRINT("\t");

	for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
		MTWF_PRINT("%-8d", pDiag->diag_info[i].deq_called);

	MTWF_PRINT("\n    %-12s\n", "DeQueueRound(Per-Call) Distribution");
	MTWF_PRINT("\t");

	for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
		MTWF_PRINT("%-8d", pDiag->diag_info[i].deq_round);

	MTWF_PRINT("\n    %-12s\n", "DeQueueCount(Per-Round) Distribution");

	for (SwQNumLevel = 0; SwQNumLevel < 9; SwQNumLevel++) {
		if (SwQNumLevel == 8)
			MTWF_PRINT("\t>%-5d",  SwQNumLevel);
		else
			MTWF_PRINT("\t%-6d", SwQNumLevel);

		for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
			MTWF_PRINT("%-7d", pDiag->diag_info[i].deq_cnt[SwQNumLevel]);

		MTWF_PRINT("\n");
	}

	MTWF_PRINT("\n    %-12s%d", "Sw-Queued TxSwQCnt for WCID ", pDiag->wcid);

	for (que_idx = 0; que_idx < WMM_NUM_OF_AC; que_idx++) {
		MTWF_PRINT("\n    %s[%d]\n", "Queue", que_idx);

		for (SwQNumLevel = 0; SwQNumLevel < 9; SwQNumLevel++) {
			if (SwQNumLevel == 8)
				MTWF_PRINT("\t>%-5d",  SwQNumLevel);
			else
				MTWF_PRINT("\t%-6d", SwQNumLevel);

			for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
				MTWF_PRINT("%-7d", pDiag->diag_info[i].TxSWQueCnt[que_idx][SwQNumLevel]);

			MTWF_PRINT("\n");
		}

		MTWF_PRINT("	%-12s\n", "TxEnQFailCnt");

		for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
			MTWF_PRINT("\t%-7d", pDiag->diag_info[i].enq_fall_cnt[que_idx]);

		MTWF_PRINT("\n");
	}

	MTWF_PRINT("\n	  %s\n", "DeQueFailedCnt:Reason NotTxResource");

	for (que_idx = 0; que_idx < WMM_NUM_OF_AC; que_idx++) {
		MTWF_PRINT("\n    %s[%d]:", "Queue", que_idx);

		for (i = start; i != stop;	i = (i + 1) % DIAGNOSE_TIME)
			MTWF_PRINT("\t%-7d", pDiag->diag_info[i].deq_fail_no_resource_cnt[que_idx]);
	}

#endif /* DBG_TXQ_DEPTH */
#ifdef DOT11_N_SUPPORT
#ifdef DBG_TX_AGG_CNT
	MTWF_PRINT("\n    %-12s\n", "Tx-Agged AMPDUCnt");

	for (McsIdx = 0; McsIdx < 16; McsIdx++) {
		MTWF_PRINT("\t%-6d", (McsIdx + 1));

		for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME) {
			MTWF_PRINT("%d(%d%%)  ", pDiag->diag_info[i].TxAMPDUCnt[McsIdx],
					  pDiag->diag_info[i].TxAMPDUCnt[McsIdx] ? (pDiag->diag_info[i].TxAMPDUCnt[McsIdx] * 100 / pDiag->diag_info[i].TxAggCnt) :
					  0);
		}

		MTWF_PRINT ("\n");
	}

#endif /* DBG_TX_AGG_CNT */
#endif /* DOT11_N_SUPPORT */
	MTWF_PRINT("Rx Info\n");
	MTWF_PRINT("    %-12s", "RxDataCnt");

	for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
		MTWF_PRINT("%-7d", pDiag->diag_info[i].RxDataCnt);

	MTWF_PRINT("\n    %-12s", "RxCrcErrCnt");

	for (i = start; i != stop;  i = (i + 1) % DIAGNOSE_TIME)
		MTWF_PRINT("%-7d", pDiag->diag_info[i].RxCrcErrCnt);

	MTWF_PRINT("\n-------------\n");
done:
	os_free_mem(pDiag);
	return TRUE;
}
#endif /* DBG_DIAGNOSE */
#endif /* RTMP_MAC_PCI */
#ifdef ANDLINK_FEATURE_SUPPORT
/**
* andlink_send_inf_stat_event() - set interafce stats infos
* @pAd:	wifi adapter.
* @apidx:  ap interface index.
*
*Andlink sent wireless event to andlink app of TxCount/TxCount
*ReceivedByteCount/TransmittedByteCount/RxErrorCount/RxDropCount
*of every interface.
*
* Return:
*		0 : success
*		other : Fail
*/

INT andlink_send_inf_stat_event(RTMP_ADAPTER *pAd, INT apidx)
{
	struct wifi_event event;

	if (!VALID_MBSS(pAd, apidx)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"input apidx: %d > %d BSSIDNUM\n", apidx, pAd->ApCfg.BssidNum);
		return FALSE;
	}

	NdisZeroMemory(&event, sizeof(struct wifi_event));

	event.type = EVENTTYPE_INF_STATS;
	NdisMoveMemory(event.MAC, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
	NdisMoveMemory(event.SSID, pAd->ApCfg.MBSSID[apidx].Ssid, MAX_LEN_OF_SSID);
	event.data.inf_stats.PacketsReceived = pAd->ApCfg.MBSSID[apidx].RxCount;
	event.data.inf_stats.PacketsSent = pAd->ApCfg.MBSSID[apidx].TxCount;
	event.data.inf_stats.BytesReceived = pAd->ApCfg.MBSSID[apidx].ReceivedByteCount;
	event.data.inf_stats.BytesSent = pAd->ApCfg.MBSSID[apidx].TransmittedByteCount;
	event.data.inf_stats.ErrorReceived = pAd->ApCfg.MBSSID[apidx].RxErrorCount;
	event.data.inf_stats.DropPacketsReceived = pAd->ApCfg.MBSSID[apidx].RxDropCount;

	RtmpOSWrielessEventSend(
			pAd->net_dev,
			RT_WLAN_EVENT_CUSTOM,
			OID_ANDLINK_EVENT,
			NULL,
			(char *)&event,
			sizeof(struct wifi_event));

	return TRUE;

}
#endif/*ANDLINK_FEATURE_SUPPORT*/

INT Show_Sat_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* Sanity check for calculation of sucessful count */
	MTWF_PRINT("TransmitCountFromOS = %d\n", pAd->WlanCounters[0].TransmitCountFrmOs.u.LowPart);
	MTWF_PRINT("TransmittedFragmentCount = %lld\n",
			  (INT64)pAd->WlanCounters[0].TransmittedFragmentCount.u.LowPart +
			  pAd->WlanCounters[0].MulticastTransmittedFrameCount.QuadPart);
	MTWF_PRINT("MulticastTransmittedFrameCount = %d\n", pAd->WlanCounters[0].MulticastTransmittedFrameCount.u.LowPart);
	MTWF_PRINT("FailedCount = %d\n", pAd->WlanCounters[0].FailedCount.u.LowPart);
	MTWF_PRINT("RetryCount = %d\n", pAd->WlanCounters[0].RetryCount.u.LowPart);
	MTWF_PRINT("MultipleRetryCount = %d\n", pAd->WlanCounters[0].MultipleRetryCount.u.LowPart);
	MTWF_PRINT("RTSSuccessCount = %d\n", pAd->WlanCounters[0].RTSSuccessCount.u.LowPart);
	MTWF_PRINT("RTSFailureCount = %d\n", pAd->WlanCounters[0].RTSFailureCount.u.LowPart);
	MTWF_PRINT("ACKFailureCount = %d\n", pAd->WlanCounters[0].ACKFailureCount.u.LowPart);
	MTWF_PRINT("FrameDuplicateCount = %d\n", pAd->WlanCounters[0].FrameDuplicateCount.u.LowPart);
	MTWF_PRINT("ReceivedFragmentCount = %d\n", pAd->WlanCounters[0].ReceivedFragmentCount.u.LowPart);
	MTWF_PRINT("MulticastReceivedFrameCount = %d\n", pAd->WlanCounters[0].MulticastReceivedFrameCount.u.LowPart);
	MTWF_PRINT("Rx drop due to out of resource  = %ld\n", (ULONG)pAd->Counters8023.RxNoBuffer);
#ifdef DBG
	MTWF_PRINT("RealFcsErrCount = %d\n", pAd->RalinkCounters.RealFcsErrCount.u.LowPart);
#else
	MTWF_PRINT("FCSErrorCount = %d\n", pAd->WlanCounters[0].FCSErrorCount.u.LowPart);
	MTWF_PRINT("FrameDuplicateCount.LowPart = %d\n", pAd->WlanCounters[0].FrameDuplicateCount.u.LowPart / 100);
#endif
	MTWF_PRINT("TransmittedFrameCount = %d\n", pAd->WlanCounters[0].TransmittedFragmentCount.u.LowPart);
	MTWF_PRINT("WEPUndecryptableCount = %d\n", pAd->WlanCounters[0].WEPUndecryptableCount.u.LowPart);
#ifdef OUI_CHECK_SUPPORT
	{
		INT32 i = 0;

		for (i = 0; i < DBDC_BAND_NUM; i++)
			MTWF_PRINT("band %d RxHWLookupWcidMismatchCount = %ld\n",
					 i, (ULONG)pAd->WlanCounters[i].RxHWLookupWcidErrCount.u.LowPart);
	}
#endif
#ifdef DOT11_N_SUPPORT
	MTWF_PRINT("\n===Some 11n statistics variables:\n");
	/* Some 11n statistics variables */
	MTWF_PRINT("TxAMSDUCount = %ld\n", (ULONG)pAd->RalinkCounters.TxAMSDUCount.u.LowPart);
	MTWF_PRINT("RxAMSDUCount = %ld\n", (ULONG)pAd->RalinkCounters.RxAMSDUCount.u.LowPart);
	MTWF_PRINT("TransmittedAMPDUCount = %ld\n", (ULONG)pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart);
	MTWF_PRINT("TransmittedMPDUsInAMPDUCount = %ld\n", (ULONG)pAd->RalinkCounters.TransmittedMPDUsInAMPDUCount.u.LowPart);
	MTWF_PRINT("TransmittedOctetsInAMPDUCount = %ld\n", (ULONG)pAd->RalinkCounters.TransmittedOctetsInAMPDUCount.u.LowPart);
	MTWF_PRINT("MPDUInReceivedAMPDUCount = %ld\n", (ULONG)pAd->RalinkCounters.MPDUInReceivedAMPDUCount.u.LowPart);
#ifdef DOT11N_DRAFT3
	MTWF_PRINT("fAnyStaFortyIntolerant=%d\n", pAd->MacTab.fAnyStaFortyIntolerant);
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
	{
		int apidx;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			MTWF_PRINT("-- IF-ra%d --\n", apidx);
			MTWF_PRINT("Packets Received = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].RxCount);
			MTWF_PRINT("Packets Sent = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].TxCount);
			MTWF_PRINT("Bytes Received = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].ReceivedByteCount);
			MTWF_PRINT("Byte Sent = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].TransmittedByteCount);
			MTWF_PRINT("Error Packets Received = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].RxErrorCount);
			MTWF_PRINT("Drop Received Packets = %ld\n", (ULONG)pAd->ApCfg.MBSSID[apidx].RxDropCount);
#ifdef ANDLINK_FEATURE_SUPPORT
			if (HcGetBandByWdev(&pAd->ApCfg.MBSSID[apidx].wdev) < DBDC_BAND_NUM &&
				TRUE == pAd->CommonCfg.andlink_enable[HcGetBandByWdev(&pAd->ApCfg.MBSSID[apidx].wdev)])
				andlink_send_inf_stat_event(pAd, apidx);
#endif/*ANDLINK_FEATURE_SUPPORT*/

#ifdef WSC_INCLUDED

			if (pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfMode != WSC_DISABLE) {
				WSC_CTRL *pWscCtrl;

				pWscCtrl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
				MTWF_PRINT("WscInfo:\n"
						  "\tWscConfMode=%d\n"
						  "\tWscMode=%s\n"
						  "\tWscConfStatus=%d\n"
						  "\tWscPinCode=%d\n"
						  "\tWscState=0x%x\n"
						  "\tWscStatus=0x%x\n",
						  pWscCtrl->WscConfMode,
						  ((pWscCtrl->WscMode == WSC_PIN_MODE) ? "PIN" : "PBC"),
						  pWscCtrl->WscConfStatus, pWscCtrl->WscEnrolleePinCode,
						  pWscCtrl->WscState, pWscCtrl->WscStatus);
			}

#endif /* WSC_INCLUDED */
			MTWF_PRINT("-- IF-ra%d end --\n", apidx);
		}
	}
	{
		int i, j, k, maxMcs = MAX_MCS_SET - 1;
		PMAC_TABLE_ENTRY pEntry;

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pEntry = &pAd->MacTab.Content[i];

			if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
				MTWF_PRINT("\n"MACSTR" - ", MAC2STR(pEntry->Addr));
				MTWF_PRINT("%-4d\n", (int)pEntry->Aid);

				for (j = maxMcs; j >= 0; j--) {
					if ((pEntry->TXMCSExpected[j] != 0) || (pEntry->TXMCSFailed[j] != 0)) {
						MTWF_PRINT("MCS[%02d]: Expected %u, Successful %u (%d%%), Failed %u\n",
								  j, pEntry->TXMCSExpected[j], pEntry->TXMCSSuccessful[j],
								  pEntry->TXMCSExpected[j] ? (100 * pEntry->TXMCSSuccessful[j]) / pEntry->TXMCSExpected[j] : 0,
								  pEntry->TXMCSFailed[j]);

						for (k = maxMcs; k >= 0; k--) {
							if (pEntry->TXMCSAutoFallBack[j][k] != 0) {
								MTWF_PRINT("\t\t\tAutoMCS[%02d]: %u (%d%%)\n", k, pEntry->TXMCSAutoFallBack[j][k],
										  (100 * pEntry->TXMCSAutoFallBack[j][k]) / pEntry->TXMCSExpected[j]);
							}
						}
					}
				}
			}
		}
	}
#ifdef DOT11_N_SUPPORT
	/* Display Tx Aggregation statistics */
	DisplayTxAgg(pAd);
#endif /* DOT11_N_SUPPORT */
	return TRUE;
}

/* fw debug methoed command*/
INT SetTrigCoreDump(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Manual trigger Fw Core dump\n");
	RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x89010108, 0x40000);
	RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x89010118, 0x40000);
	return TRUE;
}

INT Show_FwDbgInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_fw_dbg_info)
		chip_dbg->show_fw_dbg_info(pAd);

	return TRUE;

}


INT Show_BusDbgInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_bus_dbg_info)
		chip_dbg->show_bus_dbg_info(pAd);

	return TRUE;
}


INT Set_CpuUtilEn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT Enable;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (arg == NULL)
		return FALSE;

	Enable = os_str_toul(arg, 0, 10);


	if (chip_dbg->set_cpu_util_en)
		chip_dbg->set_cpu_util_en(pAd, Enable);

	return TRUE;
}

INT Set_CpuUtilMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT u4Mode;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (arg == NULL)
		return FALSE;

	u4Mode = os_str_toul(arg, 0, 10);


	if (chip_dbg->set_cpu_util_mode)
		chip_dbg->set_cpu_util_mode(pAd, u4Mode);

	return TRUE;
}

INT Show_CoreDump_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->show_coredump_proc)
		chip_dbg->show_coredump_proc(pAd);

	return TRUE;
}

INT Show_SimSectionUlm_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR fileName[64];
	struct file *file_w;
	mm_segment_t orig_fs;
	UINT32 startaddr = 0,  endaddr = 0, addr = 0;
	UINT32 macVal = 0;
	UINT Idx;/* enable load from bin */
	int ret;

	Idx = os_str_tol(arg, 0, 10);

	NdisZeroMemory(fileName, 64);

	if (Idx == 0 || Idx == 2 || Idx == 3) {
		ret = snprintf(fileName, sizeof(fileName), "/etc/mem_dump_ulm%d.txt", Idx);
		if (os_snprintf_error(sizeof(fileName), ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			return FALSE;
		}
	} else if (Idx == 4) {
		ret = snprintf(fileName, sizeof(fileName), "/etc/mem_dump_cache_sram.txt");
		if (os_snprintf_error(sizeof(fileName), ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			return FALSE;
		}
	} else
		return FALSE;

	if (Idx == 0) {
		startaddr = 0x00000000;
		endaddr = 0x0004FFFF;
	} else if (Idx == 2) {
		startaddr = 0x00200000;
		endaddr = 0x0024FFFF;
	} else if (Idx == 3) {
		startaddr = 0x00300000;
		endaddr = 0x0034FFFF;
	} else if (Idx == 4) {
		startaddr = 0xE0000000;
		endaddr = 0xE00D7FFF;
	}

	orig_fs = get_fs();
	set_fs(KERNEL_DS);
	/* open file */
	file_w = filp_open(fileName, O_WRONLY | O_CREAT, 0);

	if (IS_ERR(file_w)) {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "-->2) Error %ld opening %s\n", -PTR_ERR(file_w), fileName);
	} else {
			if (file_w->f_op)
				file_w->f_pos = 0;
			else
				goto  done;


		addr = startaddr;

		MTWF_PRINT("%s open success\n", fileName);

		while (addr <= endaddr) {

			HW_IO_READ32(pAd->hdev_ctrl, addr, &macVal);

			addr += 4;
#if (KERNEL_VERSION(4, 1, 0) > LINUX_VERSION_CODE)
			if (file_w->f_op->write)
				file_w->f_op->write(file_w, (UCHAR *)&macVal, 4, &file_w->f_pos);
			else
				MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "no file write method\n");
#elif (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
			kernel_write(file_w, (UCHAR *) &macVal, 4, &file_w->f_pos);
#else
			__vfs_write(file_w, (UCHAR *)&macVal, 4, &file_w->f_pos);
#endif
		}

		filp_close(file_w, NULL);
		MTWF_PRINT("%s write done\n", fileName);
	}

done:
	set_fs(orig_fs);

	return TRUE;
}

INT Show_Sat_Reset_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand = BAND0;

	if (wdev != NULL)
		ucBand = HcGetBandByWdev(wdev);

	/* Sanity check for calculation of sucessful count */
	MTWF_PRINT("TransmittedFragmentCount = %lld\n",
			  (INT64)pAd->WlanCounters[ucBand].TransmittedFragmentCount.u.LowPart +
			  pAd->WlanCounters[ucBand].MulticastTransmittedFrameCount.QuadPart);
	MTWF_PRINT("MulticastTransmittedFrameCount = %d\n", pAd->WlanCounters[ucBand].MulticastTransmittedFrameCount.u.LowPart);
	MTWF_PRINT("FailedCount = %d\n", pAd->WlanCounters[ucBand].FailedCount.u.LowPart);
	MTWF_PRINT("RetryCount = %d\n", pAd->WlanCounters[ucBand].RetryCount.u.LowPart);
	MTWF_PRINT("MultipleRetryCount = %d\n", pAd->WlanCounters[ucBand].MultipleRetryCount.u.LowPart);
	MTWF_PRINT("RTSSuccessCount = %d\n", pAd->WlanCounters[ucBand].RTSSuccessCount.u.LowPart);
	MTWF_PRINT("RTSFailureCount = %d\n", pAd->WlanCounters[ucBand].RTSFailureCount.u.LowPart);
	MTWF_PRINT("ACKFailureCount = %d\n", pAd->WlanCounters[ucBand].ACKFailureCount.u.LowPart);
	MTWF_PRINT("FrameDuplicateCount = %d\n", pAd->WlanCounters[ucBand].FrameDuplicateCount.u.LowPart);
	MTWF_PRINT("ReceivedFragmentCount = %d\n", pAd->WlanCounters[ucBand].ReceivedFragmentCount.u.LowPart);
	MTWF_PRINT("MulticastReceivedFrameCount = %d\n", pAd->WlanCounters[ucBand].MulticastReceivedFrameCount.u.LowPart);
	MTWF_PRINT("Rx drop due to out of resource  = %ld\n", (ULONG)pAd->Counters8023.RxNoBuffer);
#ifdef DBG
	MTWF_PRINT("RealFcsErrCount = %d\n", pAd->RalinkCounters.RealFcsErrCount.u.LowPart);
#else
	MTWF_PRINT("FCSErrorCount = %d\n", pAd->WlanCounters[0].FCSErrorCount.u.LowPart);
	MTWF_PRINT("FrameDuplicateCount.LowPart = %d\n", pAd->WlanCounters[0].FrameDuplicateCount.u.LowPart / 100);
#endif
	MTWF_PRINT("TransmittedFrameCount = %d\n", pAd->WlanCounters[ucBand].TransmittedFrameCount.u.LowPart);
	MTWF_PRINT("WEPUndecryptableCount = %d\n", pAd->WlanCounters[ucBand].WEPUndecryptableCount.u.LowPart);
	pAd->WlanCounters[ucBand].TransmittedFragmentCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].MulticastTransmittedFrameCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].FailedCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].RetryCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].MultipleRetryCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].RTSSuccessCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].RTSFailureCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].ACKFailureCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].FrameDuplicateCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].ReceivedFragmentCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].MulticastReceivedFrameCount.u.LowPart = 0;
	pAd->Counters8023.RxNoBuffer = 0;
#ifdef DBG
	pAd->RalinkCounters.RealFcsErrCount.u.LowPart = 0;
#else
	pAd->WlanCounters[ucBand].FCSErrorCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].FrameDuplicateCount.u.LowPart = 0;
#endif
	pAd->WlanCounters[ucBand].FCSErrorCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].FrameDuplicateCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].FCSErrorCount.u.HighPart = 0;
	pAd->WlanCounters[ucBand].FrameDuplicateCount.u.HighPart = 0;

	pAd->WlanCounters[ucBand].TransmittedFrameCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].WEPUndecryptableCount.u.LowPart = 0;
#ifdef STATS_COUNT_SUPPORT
	pAd->WlanCounters[ucBand].AmpduSuccessCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].AmpduFailCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].AmpduSuccessCount.u.HighPart = 0;
	pAd->WlanCounters[ucBand].AmpduFailCount.u.HighPart = 0;
#endif /* STATS_COUNT_SUPPORT */
	pAd->WlanCounters[ucBand].RxFcsErrorCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].RxFcsErrorCount.u.HighPart = 0;
	pAd->WlanCounters[ucBand].RxFifoFullCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].RxFifoFullCount.u.HighPart = 0;
	pAd->WlanCounters[ucBand].RxMpduCount.QuadPart = 0;
	pAd->WlanCounters[ucBand].ChannelIdleCount.QuadPart = 0;
	pAd->WlanCounters[ucBand].CcaNavTxTime.QuadPart = 0;
	pAd->WlanCounters[ucBand].RxMdrdyCount.QuadPart = 0;
	pAd->WlanCounters[ucBand].SCcaTime.QuadPart = 0;
	pAd->WlanCounters[ucBand].PEdTime.QuadPart = 0;
	pAd->WlanCounters[ucBand].RxTotByteCount.QuadPart = 0;
	pAd->WlanCounters[ucBand].CurrentBwTxCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].OtherBwTxCount.u.LowPart = 0;
	pAd->WlanCounters[ucBand].TxAggRange1Count.u.LowPart = 0;
	pAd->WlanCounters[ucBand].TxAggRange2Count.u.LowPart = 0;
	pAd->WlanCounters[ucBand].TxAggRange3Count.u.LowPart = 0;
	pAd->WlanCounters[ucBand].TxAggRange4Count.u.LowPart = 0;
	{
		int i, j, k, maxMcs = 15;
		PMAC_TABLE_ENTRY pEntry;

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pEntry = &pAd->MacTab.Content[i];

			if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
				MTWF_PRINT("\n"MACSTR" - ", MAC2STR(pEntry->Addr));
				MTWF_PRINT("%-4d\n", (int)pEntry->Aid);

				for (j = maxMcs; j >= 0; j--) {
					if ((pEntry->TXMCSExpected[j] != 0) || (pEntry->TXMCSFailed[j] != 0)) {
						MTWF_PRINT("MCS[%02d]: Expected %u, Successful %u (%d%%), Failed %u\n",
								  j, pEntry->TXMCSExpected[j], pEntry->TXMCSSuccessful[j],
								  pEntry->TXMCSExpected[j] ? (100 * pEntry->TXMCSSuccessful[j]) / pEntry->TXMCSExpected[j] : 0,
								  pEntry->TXMCSFailed[j]);

						for (k = maxMcs; k >= 0; k--) {
							if (pEntry->TXMCSAutoFallBack[j][k] != 0) {
								MTWF_PRINT("\t\t\tAutoMCS[%02d]: %u (%d%%)\n", k, pEntry->TXMCSAutoFallBack[j][k],
										  (100 * pEntry->TXMCSAutoFallBack[j][k]) / pEntry->TXMCSExpected[j]);
							}
						}
					}
				}
			}

			for (j = 0; j < (maxMcs + 1); j++) {
				pEntry->TXMCSExpected[j] = 0;
				pEntry->TXMCSSuccessful[j] = 0;
				pEntry->TXMCSFailed[j] = 0;

				for (k = maxMcs; k >= 0; k--)
					pEntry->TXMCSAutoFallBack[j][k] = 0;
			}
		}
	}
#ifdef DOT11_N_SUPPORT
	/* Display Tx Aggregation statistics */
	DisplayTxAgg(pAd);
#endif /* DOT11_N_SUPPORT */
	return TRUE;
}

#ifdef MAT_SUPPORT
INT Show_MATTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	extern VOID dumpIPMacTb(MAT_STRUCT * pMatCfg, int index);
	extern NDIS_STATUS dumpSesMacTb(MAT_STRUCT * pMatCfg, int hashIdx);
	extern NDIS_STATUS dumpUidMacTb(MAT_STRUCT * pMatCfg, int hashIdx);
	extern NDIS_STATUS dumpIPv6MacTb(MAT_STRUCT * pMatCfg, int hashIdx);
	dumpIPMacTb(&pAd->MatCfg, -1);
	dumpSesMacTb(&pAd->MatCfg, -1);
	dumpUidMacTb(&pAd->MatCfg, -1);
	dumpIPv6MacTb(&pAd->MatCfg, -1);
	MTWF_PRINT("Default BroadCast Address=%02x:%02x:%02x:%02x:%02x:%02x!\n", BROADCAST_ADDR[0], BROADCAST_ADDR[1],
			  BROADCAST_ADDR[2], BROADCAST_ADDR[3], BROADCAST_ADDR[4], BROADCAST_ADDR[5]);
	return TRUE;
}
#endif /* MAT_SUPPORT */

#ifdef DOT1X_SUPPORT
/*
    ==========================================================================
    Description:
	UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID RTMPIoctlAddPMKIDCache(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	UCHAR				apidx;
	NDIS_AP_802_11_KEY	 *pKey;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	apidx =	(UCHAR) pObj->ioctl_if;
	os_alloc_mem(pAd, (UCHAR **)&pKey, wrq->u.data.length);
	if (pKey == NULL)
		return;

	if (copy_from_user(pKey, wrq->u.data.pointer, wrq->u.data.length)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "copy from user failed\n");
		os_free_mem(pKey);
		return;
	}

	pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;

	if (IS_AKM_WPA2(pSecConfig->AKMMap)
		|| IS_AKM_WPA3_192BIT(pSecConfig->AKMMap)) {
		if (pKey->KeyLength == 32) {
			UCHAR digest[80] = {0}, PMK_key[20], macaddr[MAC_ADDR_LEN];

			MTWF_DBG(pAd, DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				"pKey->KeyLength = %d\n", pKey->KeyLength);
			/* Calculate PMKID */
			NdisMoveMemory(&PMK_key[0], "PMK Name", 8);
			NdisMoveMemory(&PMK_key[8], pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
			NdisMoveMemory(&PMK_key[14], pKey->addr, MAC_ADDR_LEN);
			if (IS_AKM_SHA384(pSecConfig->AKMMap))
				RT_HMAC_SHA384(pKey->KeyMaterial, LEN_PMK_SHA384, PMK_key, 20, digest, LEN_PMKID);
#ifdef OCE_FILS_SUPPORT
			/* Todo: why PMF sha256 didn't use it before ? */
			else if (IS_AKM_FILS_SHA256(pSecConfig->AKMMap)) {
				RT_HMAC_SHA256(pKey->KeyMaterial, LEN_PMK, PMK_key, 20, digest, LEN_PMKID);
			}
#endif /* OCE_FILS_SUPPORT */
			else
				RT_HMAC_SHA1(pKey->KeyMaterial, LEN_PMK, PMK_key, 20, digest, SHA1_DIGEST_SIZE);
			NdisMoveMemory(macaddr, pKey->addr, MAC_ADDR_LEN);
			RTMPAddPMKIDCache(&pAd->ApCfg.PMKIDCache,
					  apidx,
					  macaddr,
					  digest,
					  pKey->KeyMaterial,
					  FALSE,
					  pKey->KeyLength);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "WPA2(pre-auth):("MACSTR")Calc PMKID=%02x:%02x:%02x:%02x:%02x:%02x\n",
					  MAC2STR(pKey->addr), digest[0], digest[1],
					  digest[2], digest[3], digest[4], digest[5]);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PMK =%02x:%02x:%02x:%02x-%02x:%02x:%02x:%02x\n",
					 pKey->KeyMaterial[0], pKey->KeyMaterial[1],
					 pKey->KeyMaterial[2], pKey->KeyMaterial[3], pKey->KeyMaterial[4], pKey->KeyMaterial[5], pKey->KeyMaterial[6],
					 pKey->KeyMaterial[7]);
		} else
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Set::RT_OID_802_11_WPA2_ADD_PMKID_CACHE ERROR or is wep key\n");
	}

	os_free_mem(pKey);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<== RTMPIoctlAddPMKIDCache\n");
}

/*
    ==========================================================================
    Description:
	UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID RTMPIoctlStaticWepCopy(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	MAC_TABLE_ENTRY *pEntry;
	UCHAR MacAddr[MAC_ADDR_LEN] = {0};
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	apidx = (UCHAR) pObj->ioctl_if;
	ASIC_SEC_INFO Info = {0};
	UCHAR	KeyIdx;
	UINT32 len;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RTMPIoctlStaticWepCopy-IF(ra%d)\n", apidx);

	if (wrq->u.data.length != sizeof(MacAddr)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "the length isn't match (%d)\n",
				 wrq->u.data.length);
		return;
	}

	len = copy_from_user(&MacAddr, wrq->u.data.pointer, wrq->u.data.length);
	pEntry = MacTableLookup(pAd, MacAddr);

	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "the mac address isn't match\n");
		return;
	}

	KeyIdx = pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseKeyId;

	/* need to copy the default shared-key to pairwise key table for this entry in 802.1x mode */
	if (pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.WepKey[KeyIdx].KeyLen == 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ERROR: Can not get Default shared-key (index-%d)\n", KeyIdx);
		return;
	}

	pEntry->SecConfig.AKMMap = pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.AKMMap;
	pEntry->SecConfig.PairwiseCipher = pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseCipher;
	pEntry->SecConfig.PairwiseKeyId = pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseKeyId;
	NdisMoveMemory(pEntry->SecConfig.WepKey, pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.WepKey,
				   sizeof(SEC_KEY_INFO)*SEC_KEY_NUM);
	/* Set key material to Asic */
	os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
	Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
	Info.Direction = SEC_ASIC_KEY_BOTH;
	Info.Wcid = pEntry->wcid;
	Info.BssIndex = pEntry->func_tb_idx;
	Info.Cipher = pEntry->SecConfig.PairwiseCipher;
	Info.KeyIdx = pEntry->SecConfig.PairwiseKeyId;
	os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
	HW_ADDREMOVE_KEYTABLE(pAd, &Info);
}

/*
    ==========================================================================
    Description:
	UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID RTMPIoctlSetIdleTimeout(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	MAC_TABLE_ENTRY			*pEntry;
	PDOT1X_IDLE_TIMEOUT		pIdleTime;
	DOT1X_IDLE_TIMEOUT	IdleTime;

	if (wrq->u.data.length != sizeof(DOT1X_IDLE_TIMEOUT)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "the length is mis-match\n");
		return;
	}
	pIdleTime = &IdleTime;
	if (copy_from_user(&IdleTime, wrq->u.data.pointer, wrq->u.data.length)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy from user failed\n");
		return;
	}

	pEntry = MacTableLookup(pAd, pIdleTime->StaAddr);

	if (pEntry == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "the entry is empty\n");
		return;
	}

	pEntry->NoDataIdleCount = 0;
	/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
	tr_ctl->tr_entry[pEntry->wcid].NoDataIdleCount = 0;
	pEntry->StaIdleTimeout = pIdleTime->idle_timeout;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Update Idle-Timeout(%d) from dot1x daemon\n",
			 pEntry->StaIdleTimeout);
}

#ifdef RADIUS_MAC_ACL_SUPPORT
PRT_802_11_RADIUS_ACL_ENTRY RadiusFindAclEntry(
	PLIST_HEADER		pCacheList,
	IN	PUCHAR		pMacAddr)
{
	PRT_802_11_RADIUS_ACL_ENTRY	pAclEntry = NULL;
	RT_LIST_ENTRY		        *pListEntry = NULL;

	pListEntry = pCacheList->pHead;
	pAclEntry = (PRT_802_11_RADIUS_ACL_ENTRY)pListEntry;

	while (pAclEntry != NULL) {
		if (NdisEqualMemory(pAclEntry->Addr, pMacAddr, MAC_ADDR_LEN))
			return pAclEntry;

		pListEntry = pListEntry->pNext;
		pAclEntry = (PRT_802_11_RADIUS_ACL_ENTRY)pListEntry;
	}

	return NULL;
}

VOID RTMPIoctlAddRadiusMacAuthCache(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	RT_802_11_ACL_ENTRY newCache;
	UCHAR   apidx;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	PRT_802_11_RADIUS_ACL_ENTRY pAclEntry = NULL;

	apidx = (UCHAR) pObj->ioctl_if;

	if (pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.Policy != RADIUS_MAC_AUTH_ENABLE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "RADIUS_MAC_AUTH Function State in Disable.\n");
		return;
	}

	/* From userSpace struct using RT_802_11_ACL_ENTRY */
	if (wrq->u.data.length != sizeof(RT_802_11_ACL_ENTRY)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "the length is mis-match\n");
		return;
	}

	copy_from_user(&newCache, wrq->u.data.pointer, wrq->u.data.length);
	pAclEntry = RadiusFindAclEntry(&pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList, newCache.Addr);

	if (pAclEntry) {
		/* Replace the Cache if exist */
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "[%d] Found "MACSTR" in Cache And Update Result to %d.\n",
				  apidx, MAC2STR(newCache.Addr), newCache.Rsv);
		pAclEntry->result = newCache.Rsv;
		return;
	}

	/* Add new Cache */
	os_alloc_mem(NULL, (UCHAR **)&pAclEntry, sizeof(RT_802_11_RADIUS_ACL_ENTRY));

	if (pAclEntry) {
		NdisZeroMemory(pAclEntry, sizeof(RT_802_11_RADIUS_ACL_ENTRY));
		pAclEntry->pNext = NULL;
		NdisMoveMemory(pAclEntry->Addr, newCache.Addr, MAC_ADDR_LEN);
		pAclEntry->result = newCache.Rsv;
		insertTailList(&pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList, (RT_LIST_ENTRY *)pAclEntry);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "[%d] New "MACSTR" res(%d) in Cache(%d).\n",
				  apidx,
				  MAC2STR(pAclEntry->Addr), pAclEntry->result,
				  pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList.size);
	} else
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error in alloc mem in New Radius ACL Function.\n");
}

VOID RTMPIoctlDelRadiusMacAuthCache(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	UCHAR   apidx;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	RT_LIST_ENTRY     *pListEntry = NULL;
	UCHAR macBuf[MAC_ADDR_LEN];

	apidx = (UCHAR) pObj->ioctl_if;

	if (wrq->u.data.length != MAC_ADDR_LEN) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "the length is mis-match\n");
		return;
	}

	copy_from_user(&macBuf, wrq->u.data.pointer, wrq->u.data.length);
	pListEntry = (RT_LIST_ENTRY *)RadiusFindAclEntry(&pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList,
				 macBuf);

	if (pListEntry) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[%d]Del "MACSTR" in Cache(%d).\n", apidx,
				 MAC2STR(macBuf), pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList.size);
		delEntryList(&pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList, pListEntry);
		os_free_mem(pListEntry);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[%d]STA "MACSTR" not in Cache.\n",
				 apidx, MAC2STR(macBuf));
	}
}

VOID RTMPIoctlClearRadiusMacAuthCache(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR   apidx = (UCHAR) pObj->ioctl_if;
	RT_LIST_ENTRY     *pListEntry = NULL;
	PLIST_HEADER    pListHeader = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList;

	if (pListHeader->size == 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "[%d] Radius ACL Cache already in empty.\n", apidx);
		return;
	}

	pListEntry = pListHeader->pHead;

	while (pListEntry != NULL) {
		/*Remove ListEntry from Header*/
		removeHeadList(pListHeader);
		os_free_mem(pListEntry);
		pListEntry = pListHeader->pHead;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Clean [%d] Radius ACL Cache.\n", apidx);
}
#endif /* RADIUS_MAC_ACL_SUPPORT */

VOID RTMPIoctlQueryStaAid(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	DOT1X_QUERY_STA_AID macBuf;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (wrq->u.data.length != sizeof(DOT1X_QUERY_STA_AID)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "the length is mis-match\n");
		return;
	}

	if (copy_from_user(&macBuf, wrq->u.data.pointer, wrq->u.data.length)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_to_user fail\n");
		return;
	}

	pEntry = MacTableLookup(pAd, macBuf.StaAddr);

	if (pEntry != NULL) {
		wrq->u.data.length = sizeof(DOT1X_QUERY_STA_AID);
		macBuf.aid = pEntry->Aid;
		macBuf.wcid = pEntry->wcid;

		if (copy_to_user(wrq->u.data.pointer, &macBuf, wrq->u.data.length))
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_to_user fail\n");

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Query::OID_802_DOT1X_QUERY_STA_AID("MACSTR", AID=%d)\n",
				  MAC2STR(macBuf.StaAddr), macBuf.aid);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Query::OID_802_DOT1X_QUERY_STA_AID("MACSTR", Not Found)\n",
				  MAC2STR(macBuf.StaAddr));
	}
}

VOID RTMPIoctlQueryStaRsn(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	struct DOT1X_QUERY_STA_RSN sta_rsn;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (wrq->u.data.length != sizeof(struct DOT1X_QUERY_STA_RSN)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SUITEB, DBG_LVL_ERROR, "the length is mis-match\n");
		return;
	}

	if (copy_from_user(&sta_rsn, wrq->u.data.pointer, wrq->u.data.length)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SUITEB, DBG_LVL_ERROR, "copy_to_user fail\n");
		return;
	}

	pEntry = MacTableLookup(pAd, sta_rsn.sta_addr);

	if (pEntry != NULL) {
		wrq->u.data.length = sizeof(struct DOT1X_QUERY_STA_RSN);
		sta_rsn.akm = pEntry->SecConfig.AKMMap;
		sta_rsn.pairwise_cipher = pEntry->SecConfig.PairwiseCipher;
		sta_rsn.group_cipher = pEntry->SecConfig.GroupCipher;
#ifdef DOT11W_PMF_SUPPORT
		sta_rsn.group_mgmt_cipher = pEntry->SecConfig.PmfCfg.igtk_cipher;
#else
		sta_rsn.group_mgmt_cipher = 0;
#endif

		if (copy_to_user(wrq->u.data.pointer, &sta_rsn, wrq->u.data.length))
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SUITEB, DBG_LVL_ERROR, "copy_to_user fail\n");

		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SUITEB, DBG_LVL_INFO,
				 "Query::OID_802_DOT1X_QUERY_STA_RSN("MACSTR")\n",
				  MAC2STR(sta_rsn.sta_addr));
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SUITEB, DBG_LVL_INFO,
				"AKM=%x, pairwise=%x, group_cipher=%x, group_mgmt_cipher=%x\n",
				sta_rsn.akm, sta_rsn.pairwise_cipher, sta_rsn.group_cipher, sta_rsn.group_mgmt_cipher);
	} else {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SUITEB, DBG_LVL_INFO,
				 "Query::OID_802_DOT1X_QUERY_STA_RSN("MACSTR", Not Found)\n",
				  MAC2STR(sta_rsn.sta_addr));
	}
}

#ifdef RADIUS_ACCOUNTING_SUPPORT
VOID RTMPIoctlQueryStaData(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	DOT1X_QUERY_STA_DATA macBuf;
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT Status;

	if (wrq->u.data.length != sizeof(DOT1X_QUERY_STA_DATA)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "the length is mis-match\n");
		return;
	}

	Status = copy_from_user(&macBuf, wrq->u.data.pointer, wrq->u.data.length);
	pEntry = MacTableLookup(pAd, macBuf.StaAddr);

	if (pEntry != NULL) {
		wrq->u.data.length = sizeof(DOT1X_QUERY_STA_DATA);
		macBuf.rx_bytes = pEntry->RxBytes;
		macBuf.tx_bytes = pEntry->TxBytes;
		macBuf.rx_packets = pEntry->RxPackets.u.LowPart;
		macBuf.tx_packets = pEntry->TxPackets.u.LowPart;

		if (copy_to_user(wrq->u.data.pointer, &macBuf, wrq->u.data.length))
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s: copy_to_user fail\n");
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Query::OID_802_DOT1X_QUERY_STA_AID("MACSTR", Not Found)\n",
				  MAC2STR(macBuf.StaAddr));
	}
}
#endif /*RADIUS_ACCOUNTING_SUPPORT*/

#endif /* DOT1X_SUPPORT */

#if defined(DBG) || (defined(BB_SOC) && defined(CONFIG_ATE))

/*
    ==========================================================================
    Description:
	Read / Write BBP
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 bbp               ==> read all BBP
	       2.) iwpriv ra0 bbp 1             ==> read BBP where RegID=1
	       3.) iwpriv ra0 bbp 1=10		    ==> write BBP R1=0x10
    ==========================================================================
*/
VOID RTMPAPIoctlBBP(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IOCTL::(iwpriv) Command not Support!\n");
}

/* CFG TODO: double define in ap_cfg / sta_cfg */
#if defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
#else
#endif

#ifdef RTMP_RF_RW_SUPPORT
/*
    ==========================================================================
    Description:
	Read / Write RF register
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 rf		==> read all RF registers
	       2.) iwpriv ra0 rf 1		==> read RF where RegID=1
	       3.) iwpriv ra0 rf 1=10	==> write RF R1=0x10
    ==========================================================================
*/
VOID RTMPAPIoctlRF(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
#if defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
#else
#endif
}
#endif /* RTMP_RF_RW_SUPPORT */
#endif /*#ifdef DBG */

/*
    ==========================================================================
    Description:
	Read / Write E2PROM
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 e2p 0	==> read E2PROM where Addr=0x0
	       2.) iwpriv ra0 e2p 0=1234    ==> write E2PROM where Addr=0x0, value=1234
    ==========================================================================
*/
VOID RTMPAPIoctlE2PROM(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	RTMP_STRING *this_char, *value;
	INT					j = 0, k = 0;
	RTMP_STRING *mpool, *msg;  /*msg[1024]; */
	RTMP_STRING *arg, *ptr;
	USHORT				eepAddr = 0, offset = 2;
	UCHAR				temp[16];
	RTMP_STRING temp2[16];
	USHORT				eepValue = 0, string_size, new_line_size;
	BOOLEAN				bIsPrintAllE2PROM = FALSE;
	UINT_32 start = 0;
	UINT_32 end = 0;
	int ret, left_buf_size, rv;

	/* Count maximum allocation size (Show all e2p data) */

	string_size = (EEPROM_SIZE / offset) * strlen("[0x%04X]:%04X  ");
	new_line_size = EEPROM_SIZE / 8 + 1;
	string_size += new_line_size;

	/*for snpritf terminal, string_size++*/
	string_size = string_size + 1;
	os_alloc_mem(NULL, (UCHAR **)&mpool, sizeof(CHAR) * (string_size + 256 + 12));

	if (mpool == NULL)
		return;

	msg = (RTMP_STRING *)((ULONG)(mpool + 3) & (ULONG)~0x03);
	arg = (RTMP_STRING *)((ULONG)(msg + string_size + 3) & (ULONG)~0x03);
	memset(msg, 0x00, string_size);
	memset(arg, 0x00, 256);

	if (
#ifdef LINUX
		(wrq->u.data.length > 1) /* If no parameter, dump all e2p. */
#endif /* LINUX */
	) {
#ifdef LINUX
		if (copy_from_user(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length))
			goto done;
#else
		NdisMoveMemory(arg, wrq->u.data.pointer, (wrq->u.data.length > 255) ? 255 : wrq->u.data.length);
#endif
		ptr = arg;
		ret = snprintf(msg, string_size, "\n");
		if (os_snprintf_error(string_size, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}

		/*Parsing Read or Write */
		while ((this_char = strsep((char **)&ptr, ",")) != NULL) {

			if (!*this_char)
				continue;

			value = strchr(this_char, '=');

			if (value != NULL)
				*value++ = 0;

			if (!value || !*value) {
				/*Read */

				/* Sanity check */
				if (strstr(this_char, ":")) {
					struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAdapter->hdev_ctrl);
					UINT_32 e2p_end = chip_cap->EEPROM_DEFAULT_BIN_SIZE;
					UINT_32 e2p_print_lmt = EEPROM_SIZE;

#if defined(RTMP_FLASH_SUPPORT)
					if (pAdapter->E2pAccessMode == E2P_FLASH_MODE)
						e2p_end = get_dev_eeprom_size(pAdapter);
#endif
					rv = sscanf(arg, "%4x:%4x", &start, &end);
					if (rv > 1) {
						if ((start + end) > 0 && end > start && end < e2p_end) {
							if (end - start > e2p_print_lmt) {
								MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
										"Maximum display %d bytes, display 0x%x to 0x%x\n",
										e2p_print_lmt, start, start+e2p_print_lmt);
								end = start+e2p_print_lmt;
							}
							bIsPrintAllE2PROM = TRUE;
						}
					}
					break;
				} else if (strlen(this_char) > 4) {
					break;
				}

				j = strlen(this_char);

				while (j-- > 0) {
					if (this_char[j] > 'f' || this_char[j] < '0')
						goto done; /*return; */
				}

				/* E2PROM addr */
				k = j = strlen(this_char);

				while (j-- > 0)
					this_char[4 - k + j] = this_char[j];

				while (k < 4)
					this_char[3 - k++] = '0';

				this_char[4] = '\0';

				if (strlen(this_char) == 4) {
					AtoH(this_char, temp, 2);
					eepAddr = *temp * 256 + temp[1];

					if (eepAddr < 0xFFFF) {
						RT28xx_EEPROM_READ16(pAdapter, eepAddr, eepValue);
						left_buf_size = string_size - strlen(msg);
						ret = snprintf(msg + strlen(msg), left_buf_size, "[0x%04X]:0x%04X  ", eepAddr, eepValue);
						if (os_snprintf_error(left_buf_size, ret)) {
							MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"final_name snprintf error!\n");
							goto done;
						}
					} else {
						/*Invalid parametes, so default printk all bbp */
						break;
					}
				}
			} else {
				/*Write */
				if (strlen(value) >= sizeof(temp2)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
					"over fixed-size string temp2 by copying value, strlen(value)=%d\n",
					(UINT32)strlen(value));
					NdisMoveMemory(&temp2, value, sizeof(temp2));
					temp2[strlen(temp2)-1] = '\0';
				} else {
					NdisMoveMemory(&temp2, value, strlen(value));
					temp2[strlen(value)] = '\0';
				}

				/* Sanity check */
				if ((strlen(this_char) > 4) || strlen(temp2) > 8)
					break;

				j = strlen(this_char);

				while (j-- > 0) {
					if (this_char[j] > 'f' || this_char[j] < '0')
						goto done; /* return; */
				}

				j = strlen(temp2) - 1;

				while (j >= 0) {
					if (temp2[j] > 'f' || temp2[j] < '0')
						goto done; /* return; */

					j--;
				}

				/* MAC Addr */
				k = j = strlen(this_char);

				while (j-- > 0)
					this_char[4 - k + j] = this_char[j];

				while (k < 4)
					this_char[3 - k++] = '0';

				this_char[4] = '\0';
				/* MAC value */
				k = strlen(temp2);
				j = strlen(temp2) - 1;

				while (j >= 0) {
					if ((4 - k + j) < 0)
						break;

					temp2[4 - k + j] = temp2[j];
					j--;
				}

				while (k < 4)
					temp2[3 - k++] = '0';

				temp2[4] = '\0';
				AtoH(this_char, temp, 2);
				eepAddr = *temp * 256 + temp[1];
				AtoH(temp2, temp, 2);
				eepValue = *temp * 256 + temp[1];
				RT28xx_EEPROM_WRITE16(pAdapter, eepAddr, eepValue);

				left_buf_size = string_size - strlen(msg);
				ret = snprintf(msg + strlen(msg), string_size - strlen(msg), "[0x%02X]:%02X  ", eepAddr, eepValue);
				if (os_snprintf_error(left_buf_size, ret)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"final_name snprintf error!\n");
					goto done;
				}
			}
		}
	} else
		bIsPrintAllE2PROM = TRUE;

	if (bIsPrintAllE2PROM) {
#if defined(LINUX)
		ret = snprintf(msg, string_size, "\n");

		if (os_snprintf_error(string_size, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "final_name snprintf error!\n");
			goto done;
		}

		if ((start + end) == 0) {
			start = 0;
			end = EEPROM_SIZE;
		}

		/* E2PROM Registers */
		for (eepAddr = start; eepAddr < end; eepAddr += 2) {
			RT28xx_EEPROM_READ16(pAdapter, eepAddr, eepValue);
			left_buf_size = string_size - strlen(msg);
			ret = snprintf(msg + strlen(msg), left_buf_size, "[0x%04X]:%04X  ", eepAddr, eepValue);
			if (os_snprintf_error(left_buf_size, ret)) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "final_name snprintf error!\n");
				goto done;
			}

			if ((eepAddr & 0x6) == 0x6) {
				left_buf_size = string_size - strlen(msg);
				ret = snprintf(msg + strlen(msg), left_buf_size, "\n");
				if (os_snprintf_error(left_buf_size, ret)) {
					MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "final_name snprintf error!\n");
					goto done;
				}
			}
		}
#endif	/*  LINUX */
	}

	if (strlen(msg) == 1) {
		left_buf_size = string_size - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "===>Error command format!");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "final_name snprintf error!\n");
			goto done;
		}
	}

	/* Copy the information into the user buffer */
	AP_E2PROM_IOCTL_PostCtrl(wrq, msg);
done:
	os_free_mem(mpool);

	if (wrq->u.data.flags != RT_OID_802_11_HARDWARE_REGISTER)
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<==RTMPIoctlE2PROM\n");
}

/* #define ENHANCED_STAT_DISPLAY	// Display PER and PLR statistics */

/*
    ==========================================================================
    Description:
	Read statistics counter
Arguments:
    pAdapter                    Pointer to our adapter
    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 stat 0	==> Read statistics counter
    ==========================================================================
*/
VOID RTMPIoctlStatistics(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status;
	RTMP_STRING *msg;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand = BAND0;
	ULONG txCount = 0;
	ULONG rxCount = 0;
#ifdef ENHANCED_STAT_DISPLAY
	ULONG per;
	INT i;
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	UINT32 rx_rate = 0;
#endif
#endif
#ifdef RTMP_EFUSE_SUPPORT
	UINT efusefreenum = 0;
#endif /* RTMP_EFUSE_SUPPORT */
#ifdef BB_SOC
	ULONG txPackets = 0, rxPackets = 0, txBytes = 0, rxBytes = 0;
	UCHAR index = 0;
#endif
	BOOLEAN isfound = FALSE;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	CHAR rssi[4] = {0};
	UINT8 len = 0, path_idx = 0;
	UINT16 u2CnInfo = 0;
	UINT msg_len = 2048;
	int ret, left_buf_size;

	os_alloc_mem(pAd, (UCHAR **)&msg, msg_len);

	if (msg == NULL)
		return;

	if (wdev == NULL) {
		os_free_mem(msg);
		return;
	}

	ucBand = HcGetBandByWdev(wdev);

	memset(msg, 0x00, msg_len);
	ret = snprintf(msg, msg_len, "\n");

	if (os_snprintf_error(msg_len, ret)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		goto done;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "ra offload=%d\n", cap->fgRateAdaptFWOffload);
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

	if (cap->fgRateAdaptFWOffload == TRUE) {
		EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
		HTTRANSMIT_SETTING LastTxRate;

		os_zero_mem(&rTxStatResult, sizeof(EXT_EVENT_TX_STATISTIC_RESULT_T));
		MtCmdGetTxStatistic(pAd, GET_TX_STAT_TOTAL_TX_CNT | GET_TX_STAT_LAST_TX_RATE, ucBand, 0, &rTxStatResult);
		pAd->WlanCounters[ucBand].TransmittedFragmentCount.u.LowPart += (rTxStatResult.u4TotalTxCount -
				rTxStatResult.u4TotalTxFailCount);
		pAd->WlanCounters[ucBand].FailedCount.u.LowPart += rTxStatResult.u4TotalTxFailCount;
		pAd->WlanCounters[ucBand].CurrentBwTxCount.u.LowPart += rTxStatResult.u4CurrBwTxCnt;
		pAd->WlanCounters[ucBand].OtherBwTxCount.u.LowPart += rTxStatResult.u4OtherBwTxCnt;
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
	if (ATE_ON(pAd)) {
#if defined(CONFIG_WLAN_SERVICE)
		txCount = TESTMODE_GET_PARAM(pAd, ucBand, tx_stat.tx_done_cnt);
		/* rxCount = TESTMODE_GET_PARAM(pAd, ucBand, rx_stat.RxTotalCnt[ucBand]; */
#else
		struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

		txCount = TESTMODE_GET_PARAM(pAd, ucBand, ATE_TXDONE_CNT);
		rxCount = ATECtrl->rx_stat.RxTotalCnt[ucBand];
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"TxCount:%ld, RxCount:%ld, if_name:%s\n", txCount, rxCount, wdev->if_dev->name);
#endif	/* CONFIG_WLAN_SERVICE */
	} else
#endif	/* CONFIG_ATE */
	{
		txCount = pAd->WlanCounters[ucBand].TransmittedFragmentCount.u.LowPart;
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
		if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd))
			rxCount = (ULONG)pAd->WlanCounters[ucBand].RxMpduCount.QuadPart;
		else
#endif
			rxCount = pAd->WlanCounters[ucBand].ReceivedFragmentCount.QuadPart;
	}

	RTMP_GET_TEMPERATURE(pAd, ucBand, &pAd->temperature);
	left_buf_size = msg_len - strlen(msg);
	ret = snprintf(msg + strlen(msg), left_buf_size, "CurrentTemperature              = %d\n", pAd->temperature);
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		goto done;
	}
	ret = snprintf(msg + strlen(msg), msg_len - strlen(msg), "Tx success                      = %lu\n", txCount);
	if (os_snprintf_error(msg_len - strlen(msg), ret)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		goto done;
	}
#ifdef ENHANCED_STAT_DISPLAY

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		per = txCount == 0 ? 0 : 1000 * (pAd->WlanCounters[ucBand].FailedCount.u.LowPart) /
			  (pAd->WlanCounters[ucBand].FailedCount.u.LowPart + txCount);
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "Tx fail count                   = %lu, PER=%ld.%1ld%%\n",
				(ULONG)pAd->WlanCounters[ucBand].FailedCount.u.LowPart,
				per / 10, per % 10);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "Current BW Tx count             = %lu\n", (ULONG)pAd->WlanCounters[ucBand].CurrentBwTxCount.u.LowPart);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "Other BW Tx count               = %lu\n", (ULONG)pAd->WlanCounters[ucBand].OtherBwTxCount.u.LowPart);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	}
	left_buf_size = msg_len - strlen(msg);
	ret = snprintf(msg + strlen(msg), left_buf_size, "Rx success                      = %lu\n", rxCount);
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		goto done;
	}
#ifdef CONFIG_QA

	if (ATE_ON(pAd)) {
	} else
#endif /* CONFIG_QA */

#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		per = rxCount == 0 ? 0 : 1000 *
			(pAd->WlanCounters[ucBand].RxFcsErrorCount.u.LowPart) /
			(pAd->WlanCounters[ucBand].RxFcsErrorCount.u.LowPart + rxCount);

		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "Rx with CRC                     = %lu, PER=%ld.%1ld%%\n",
			(ULONG)pAd->WlanCounters[ucBand].RxFcsErrorCount.u.LowPart, per / 10, per % 10);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}

		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "Rx drop due to out of resource  = %lu\n",
			(ULONG)pAd->WlanCounters[ucBand].RxFifoFullCount.u.LowPart);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	} else
#endif
	{
		per = pAd->WlanCounters[ucBand].ReceivedFragmentCount.u.LowPart == 0 ? 0 : 1000 *
			(pAd->WlanCounters[ucBand].FCSErrorCount.u.LowPart) /
			(pAd->WlanCounters[ucBand].FCSErrorCount.u.LowPart +
			pAd->WlanCounters[ucBand].ReceivedFragmentCount.u.LowPart);

		snprintf(msg + strlen(msg), msg_len - strlen(msg), "Rx with CRC                     = %lu, PER=%ld.%1ld%%\n",
			(ULONG)pAd->WlanCounters[ucBand].FCSErrorCount.u.LowPart, per / 10, per % 10);
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "Rx drop due to out of resource  = %lu\n",
			(ULONG)pAd->Counters8023.RxNoBuffer);
	}
#endif /* ENHANCED_STAT_DISPLAY */
    /* rssi */

#ifdef APCLI_SUPPORT
    if (pObj->ioctl_if_type == INT_APCLI) {
		MAC_TABLE_ENTRY *entry = GetAssociatedAPByWdev(pAd, wdev);
		PSTA_ADMIN_CONFIG sta_cfg = GetStaCfgByWdev(pAd, wdev);
		len = 4;
		if (sta_cfg && entry) {
			if (sta_cfg->ApcliInfStat.Enable)
				rtmp_get_rssi(pAd, entry->wcid, rssi, len);
		}
    } else
#endif /* APCLI_SUPPORT */

		PhyStatGetRssi(pAd, ucBand, rssi, &len);
	PhyStatGetCnInfo(pAd, ucBand, &u2CnInfo);

	left_buf_size = msg_len - strlen(msg);
	ret = snprintf(msg + strlen(msg), left_buf_size, "Rssi: ");
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		goto done;
	}
	for (path_idx = 0; path_idx < len; path_idx++) {
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "%d ", rssi[path_idx]);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	}

	left_buf_size = msg_len - strlen(msg);
	ret = snprintf(msg + strlen(msg), left_buf_size, "\n");
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		goto done;
	}

	left_buf_size = msg_len - strlen(msg);
	ret = snprintf(msg + strlen(msg), left_buf_size, "CN Info:				= %d", u2CnInfo);
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		goto done;
	}

	left_buf_size = msg_len - strlen(msg);
	ret = snprintf(msg + strlen(msg), left_buf_size, "\n");
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		goto done;
	}

	{
#ifdef ENHANCED_STAT_DISPLAY

		/* Display Last Rx Rate and BF SNR of first Associated entry in MAC table */
		if (pAd->MacTab.Size > 0) {
			static char *phyMode[5] = {"CCK", "OFDM", "MM", "GF", "VHT"};

			for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
				PMAC_TABLE_ENTRY pEntry = &(pAd->MacTab.Content[i]);

				if ((pEntry->wdev == NULL) || HcGetBandByWdev(pEntry->wdev) != ucBand)
					continue;

				if ((IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) || IS_ENTRY_WDS(pEntry) || IS_ENTRY_PEER_AP(pEntry)) {
					/* sprintf(msg+strlen(msg), "sta mac: %02x:%02x:%02x:%02x:%02x:%02x\n", pEntry->wdev->if_addr[0], pEntry->wdev->if_addr[1],  pEntry->wdev->if_addr[2],  pEntry->wdev->if_addr[3],  pEntry->wdev->if_addr[4],  pEntry->wdev->if_addr[5]); */
					UINT32 lastRxRate = pEntry->LastRxRate;
					UINT32 lastTxRate = pEntry->LastTxRate;

					isfound = TRUE;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

					if (cap->fgRateAdaptFWOffload == TRUE) {
						if (1) {
							EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
							HTTRANSMIT_SETTING LastTxRate;

							os_zero_mem(&rTxStatResult, sizeof(EXT_EVENT_TX_STATISTIC_RESULT_T));
							MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, pEntry->wcid, &rTxStatResult);
							LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
							LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
							LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
							LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
							LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

							if (LastTxRate.field.MODE >= MODE_VHT)
								LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
							else if (LastTxRate.field.MODE == MODE_OFDM)
								LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
							else {
								LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS + ((rTxStatResult.rEntryTxRate.VhtNss - 1) << 3);
							}

							lastTxRate = (UINT32)(LastTxRate.word);

							/*
							 * if MODE or GI is extended, append them to target:
							 *   MODE bit[7:3] -> lastTxRate bit[20:16]
							 *   ShortGI bit[1:0] -> lastTxRate bit[22:21]
							 */
							if (rTxStatResult.rEntryTxRate.MODE >> 3)
								lastTxRate |= ((rTxStatResult.rEntryTxRate.MODE >> 3) & 0x1f) << 16;
							if (rTxStatResult.rEntryTxRate.ShortGI >> 1)
								lastTxRate |= (rTxStatResult.rEntryTxRate.ShortGI & 0x3) << (16 + 5);
						}
					}

#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
#ifdef MT_MAC

					if (IS_HIF_TYPE(pAd, HIF_MT))  {
						StatRateToString(pAd, msg, 0, lastTxRate);
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
						if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
							ShowLastRxPhyRate(pAd, ucBand, pEntry->wcid, &rx_rate);
							StatHERxRateToString(pAd, msg, rx_rate);
						} else
#endif
						StatRateToString(pAd, msg, 1, lastRxRate);
					} else
#endif /* MT_MAC */
					{
						left_buf_size = msg_len - strlen(msg);
						ret = snprintf(msg + strlen(msg), left_buf_size, "Last TX Rate                    = MCS%d, %2dM, %cGI, %s%s\n",
								lastTxRate & 0x7F,  ((lastTxRate >> 7) & 0x1) ? 40 : 20,
								((lastTxRate >> 8) & 0x1) ? 'S' : 'L',
								phyMode[(lastTxRate >> 14) & 0x3],
								((lastTxRate >> 9) & 0x3) ? ", STBC" : " ");
						if (os_snprintf_error(left_buf_size, ret)) {
							MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"final_name snprintf error!\n");
							goto done;
						}

						left_buf_size = msg_len - strlen(msg);
						ret = snprintf(msg + strlen(msg), left_buf_size, "Last RX Rate                    = MCS%d, %2dM, %cGI, %s%s\n",
								lastRxRate & 0x7F,  ((lastRxRate >> 7) & 0x1) ? 40 : 20,
								((lastRxRate >> 8) & 0x1) ? 'S' : 'L',
								phyMode[(lastRxRate >> 14) & 0x3],
								((lastRxRate >> 9) & 0x3) ? ", STBC" : " ");
						if (os_snprintf_error(left_buf_size, ret)) {
							MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								"final_name snprintf error!\n");
							goto done;
						}
					}

					break;
				}
			}
		}

#ifdef MT_MAC

		if (IS_HIF_TYPE(pAd, HIF_MT)) {


			if (pAd->CommonCfg.bTXRX_RXV_ON) {
				snprintf(msg + strlen(msg), msg_len - strlen(msg), "/* Condition Number should enable mode4 of 0x6020_426c */\n");
				snprintf(msg + strlen(msg), msg_len - strlen(msg),
						"--10 packets Condition Number   = [%d|%d|%d|%d|%d|%d|%d|%d|%d|%d]\n",
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
			}
		}

#endif /* MT_MAC */
#endif /* ENHANCED_STAT_DISPLAY */
	}

#if /*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_SCHEDULE)

	if (pAd->Flags & fRTMP_ADAPTER_RADIO_OFF)
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "Enable Wireless LAN		= %s\n", "0");
	else
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "Enable Wireless LAN		= %s\n", "1");

	snprintf(msg + strlen(msg), msg_len - strlen(msg), "\n");
#endif /*TCSUPPORT_COMPILE*/
	/*
	 * Let "iwpriv ra0 stat" can print out Tx/Rx Packet and Byte count.
	 * Therefore, we can parse them out in cfg_manager. --Trey */
#ifdef BB_SOC

	for (index = 0; index < pAd->ApCfg.BssidNum; index++) {
		rxPackets += (ULONG)pAd->ApCfg.MBSSID[index].RxCount;
		txPackets += (ULONG)pAd->ApCfg.MBSSID[index].TxCount;
		rxBytes += (ULONG)pAd->ApCfg.MBSSID[index].ReceivedByteCount;
		txBytes += (ULONG)pAd->ApCfg.MBSSID[index].TransmittedByteCount;
	}

	snprintf(msg + strlen(msg), msg_len - strlen(msg), "Packets Received       = %lu\n", rxPackets);
	snprintf(msg + strlen(msg), msg_len - strlen(msg), "Packets Sent           = %lu\n", txPackets);
	snprintf(msg + strlen(msg), msg_len - strlen(msg), "Bytes Received         = %lu\n", rxBytes);
	snprintf(msg + strlen(msg), msg_len - strlen(msg), "Bytes Sent             = %lu\n", txBytes);
	snprintf(msg + strlen(msg), msg_len - strlen(msg), "\n");
#endif
#ifdef RTMP_EFUSE_SUPPORT

	if (pAd->bUseEfuse == TRUE) {
		eFuseGetFreeBlockCount(pAd, &efusefreenum);
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "efuseFreeNumber                 = %d\n", efusefreenum);
	}

#endif /* RTMP_EFUSE_SUPPORT */

	{
#if defined(CONFIG_DOT11U_INTERWORKING) || defined(CONFIG_DOT11V_WNM) || defined(CONFIG_HOTSPOT)
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR apidx = pObj->ioctl_if;
#endif /* defined(CONFIG_DOT11U_INTERWORKING) || defined(CONFIG_DOT11V_WNM) || defined(CONFIG_HOTSPOT) */
#ifdef CONFIG_DOT11U_INTERWORKING
		PGAS_CTRL pGASCtrl = &pAd->ApCfg.MBSSID[apidx].GASCtrl;
#endif /* CONFIG_DOT11U_INTERWORKING */
#ifdef CONFIG_DOT11V_WNM
		PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[apidx].WNMCtrl;
#endif /* CONFIG_DOT11V_WNM */
#ifdef CONFIG_HOTSPOT
		PHOTSPOT_CTRL pHSCtrl;

		pHSCtrl = &pAd->ApCfg.MBSSID[apidx].HotSpotCtrl;
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "\n");
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "hotspot enable                    = %d\n", pHSCtrl->HotSpotEnable);
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "daemon ready                  = %d\n", pHSCtrl->HSDaemonReady);
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "hotspot DGAFDisable               = %d\n", pHSCtrl->DGAFDisable);
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "hotspot L2Filter              = %d\n", pHSCtrl->L2Filter);
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "hotspot ICMPv4Deny                = %d\n", pHSCtrl->ICMPv4Deny);
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "hotspot QosMapEnable              = %d\n", pHSCtrl->QosMapEnable);
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "hotspot OSEN enable                   = %d\n", pHSCtrl->bASANEnable);
#endif /* CONFIG_HOTSPOT */

#ifdef CONFIG_DOT11V_WNM
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "proxy arp enable              = %d\n", pWNMCtrl->ProxyARPEnable);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}

		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "WNMNotify enable              = %d\n", pWNMCtrl->WNMNotifyEnable);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}

		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "WNM BSS Transition Management enable = %d\n", pWNMCtrl->WNMBTMEnable);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
#endif

#ifdef CONFIG_DOT11U_INTERWORKING
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "GAS come back delay                       = %d\n", pGASCtrl->cb_delay);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}

		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "GAS MMPDU size                            = %d\n", pGASCtrl->MMPDUSize);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}

		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "GAS enable				= %d\n", pGASCtrl->b11U_enable);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
#endif /* CONFIG_DOT11U_INTERWORKING */

		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	}

	/* Copy the information into the user buffer */
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
done:
	os_free_mem(msg);
#if defined(TXBF_SUPPORT) && defined(ENHANCED_STAT_DISPLAY)
#endif /* defined(TXBF_SUPPORT) && defined(ENHANCED_STAT_DISPLAY) */
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<==RTMPIoctlStatistics\n");
}

INT RTMPIoctlRXStatistics(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	RTMP_STRING *this_char, *value = NULL;
	INT Status = NDIS_STATUS_SUCCESS;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "----------------->\n");
	os_alloc_mem(NULL, (UCHAR **)&this_char, wrq->u.data.length + 1);

	if (!this_char)
		return -ENOMEM;

	if (copy_from_user(this_char, wrq->u.data.pointer, wrq->u.data.length)) {
		os_free_mem(this_char);
		return -EFAULT;
	}

	this_char[wrq->u.data.length] = 0;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Before check, this_char = %s\n", this_char);
	value = strchr(this_char, '=');

	if (value) {
		if (strlen(value) > 1) {
			*value = 0;
			value++;
		} else
			value = NULL;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"After check, this_char = %s, value = %s\n", this_char, (value == NULL ? "" : value));

	for (PRTMP_PRIVATE_RX_PROC = RTMP_PRIVATE_RX_SUPPORT_PROC; PRTMP_PRIVATE_RX_PROC->name; PRTMP_PRIVATE_RX_PROC++) {
		if (!strcmp(this_char, PRTMP_PRIVATE_RX_PROC->name)) {
			if (!PRTMP_PRIVATE_RX_PROC->rx_proc(pAd, value, wrq)) {
				/*FALSE:Set private failed then return Invalid argument */
				Status = -EINVAL;
			}

			break;  /*Exit for loop. */
		}
	}

	if (PRTMP_PRIVATE_RX_PROC->name == NULL) {
		/*Not found argument */
		Status = -EINVAL;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "IOCTL::(iwpriv) Command not Support [%s = %s]\n", this_char, value);
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<-----------------\n");
	os_free_mem(this_char);
	return Status;
}

#ifdef DOT11_N_SUPPORT
/*
    ==========================================================================
    Description:
	Get Block ACK Table
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
			1.) iwpriv ra0 get_ba_table
			3.) UI needs to prepare at least 4096bytes to get the results
    ==========================================================================
*/
VOID RTMPIoctlQueryBaTable(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	/*char *msg; */
	UINT16	TotalEntry, i, j, index;
	QUERYBA_TABLE		*BAT;
	struct ba_control *ba_ctl = &pAd->tr_ctl.ba_ctl;

	BAT = vmalloc(sizeof(QUERYBA_TABLE));

	if (BAT == NULL)
		return;

	RTMPZeroMemory(BAT, sizeof(QUERYBA_TABLE));
	TotalEntry = pAd->MacTab.Size;
	index = 0;

	for (i = 0; ((VALID_UCAST_ENTRY_WCID(pAd, i)) && (TotalEntry > 0)); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC) && (pEntry->TXBAbitmap) && (index < 32)) {
			NdisMoveMemory(BAT->BAOriEntry[index].MACAddr, pEntry->Addr, 6);

			for (j = 0; j < 8; j++) {
				if (pEntry->BAOriWcidArray[j] != 0)
					BAT->BAOriEntry[index].BufSize[j] = ba_ctl->BAOriEntry[pEntry->BAOriWcidArray[j]].BAWinSize;
				else
					BAT->BAOriEntry[index].BufSize[j] = 0;
			}

			TotalEntry--;
			index++;
			BAT->OriNum++;
		}
	}

	TotalEntry = pAd->MacTab.Size;
	index = 0;

	for (i = 0; ((VALID_UCAST_ENTRY_WCID(pAd, i)) && (TotalEntry > 0)); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC) && (pEntry->RXBAbitmap) && (index < 32)) {
			NdisMoveMemory(BAT->BARecEntry[index].MACAddr, pEntry->Addr, 6);
			BAT->BARecEntry[index].BaBitmap = (UCHAR)pEntry->RXBAbitmap;

			for (j = 0; j < 8; j++) {
				if (pEntry->BARecWcidArray[j] != 0)
					BAT->BARecEntry[index].BufSize[j] = ba_ctl->BARecEntry[pEntry->BARecWcidArray[j]].BAWinSize;
				else
					BAT->BARecEntry[index].BufSize[j] = 0;
			}

			TotalEntry--;
			index++;
			BAT->RecNum++;
		}
	}

	wrq->u.data.length = sizeof(QUERYBA_TABLE);

	if (copy_to_user(wrq->u.data.pointer, BAT, wrq->u.data.length))
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_to_user() fail\n");

	vfree(BAT);
}
#endif /* DOT11_N_SUPPORT */

#ifdef APCLI_SUPPORT
/* APPS DVT MSP*/
INT Set_ApCli_SendPsPoll_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR ifIndex;
	UINT Enable;

	PSTA_ADMIN_CONFIG pApCliEntry = NULL;
	struct wifi_dev *wdev;
	MAC_TABLE *pMacTable;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	ifIndex = pObj->ioctl_if;
	Enable = os_str_tol(arg, 0, 16);
	pApCliEntry = &pAd->StaCfg[ifIndex];
	wdev = &pApCliEntry->wdev;
	pMacTable = &pAd->MacTab;

	if (Enable)
		ApCliRTMPSendPsPollFrame(pAd, ifIndex);

	return TRUE;
}

INT Set_ApCli_PwrSet_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR ifIndex;
	UINT16 wcid;
	UINT Enable;
	PSTA_ADMIN_CONFIG apcli_entry = NULL;
	struct wifi_dev *wdev;
	MAC_TABLE *pMacTable;
	MAC_TABLE_ENTRY *pEntry;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	Enable = os_str_tol(arg, 0, 16);
	apcli_entry = &pAd->StaCfg[ifIndex];
	wdev = &apcli_entry->wdev;
	pMacTable = &pAd->MacTab;
	apcli_entry->PwrSaveSet = (Enable > 0) ? TRUE : FALSE;

	MTWF_PRINT("I/F(apcli%d) Set_ApCli_PwrSet_Proc::(PwrSaveSet = %d)\n",
			  ifIndex,
			  apcli_entry->PwrSaveSet);

	for (wcid = 0; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		pEntry = &pMacTable->Content[wcid];

		if (pEntry && IS_ENTRY_PEER_AP(pEntry)) {
			if (apcli_entry->PwrSaveSet)
				AppsApCliRTMPSendNullFrame(pAd, pEntry->CurrTxRate, TRUE, pEntry, PWR_SAVE);
			else
				AppsApCliRTMPSendNullFrame(pAd, pEntry->CurrTxRate, TRUE, pEntry, PWR_ACTIVE);
		}
	}

	return TRUE;
}
#endif
#if defined(APCLI_SUPPORT) || defined(CONFIG_STA_SUPPORT)
INT SetApCliEnableByWdev(
	IN	PRTMP_ADAPTER pAd,
	IN	struct wifi_dev *wdev,
	IN	BOOLEAN flag)
{
	UCHAR ifIndex;
#ifdef CONFIG_MAP_SUPPORT
	struct DOT11_H *pDot11h = NULL;
#endif

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is null!\n");
		return FALSE;
	}

	ifIndex = wdev->func_idx;
#ifdef CONFIG_MAP_SUPPORT
	pDot11h = wdev->pDot11_H;
#endif

	if (ifIndex >= MAX_APCLI_NUM) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error ifIdx = %d\n", ifIndex);
		return FALSE;
	}

	if (APCLI_IF_UP_CHECK(pAd, ifIndex) != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"interface is not up, please do ifconfig %s up first\n", wdev->if_dev->name);
		return FALSE;
	}

#ifdef CONFIG_OWE_SUPPORT
	sta_reset_owe_parameters(pAd, ifIndex);
#endif

	pAd->StaCfg[ifIndex].ApcliInfStat.Enable = flag;

#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_TURNKEY_ENABLE(pAd)) {
		if (pAd->StaCfg[ifIndex].ApcliInfStat.Enable
			&& (pDot11h && pDot11h->RDMode == RD_SILENCE_MODE)) {
			pDot11h->RDCount = pDot11h->cac_time;
		}
	}
#endif
	MTWF_PRINT("%s: I/F(apcli%d) (enable = %d)\n", __func__,
		ifIndex, pAd->StaCfg[ifIndex].ApcliInfStat.Enable);

#ifdef APCLI_CONNECTION_TRIAL
	if (pAd->StaCfg[ifIndex].TrialCh == 0)
#endif /* APCLI_CONNECTION_TRIAL */
		ApCliIfDown(pAd);

#ifdef APCLI_CERT_SUPPORT
	/*Fix for TGac 5.2.57*/
	if (pAd->bApCliCertTest)
		wlan_operate_set_prim_ch(wdev, wdev->channel);
#endif
	if (pAd->StaCfg[ifIndex].ApcliInfStat.Enable == FALSE
		&& !cntl_idle(wdev))
		RTMP_MLME_RESET_STATE_MACHINE(pAd, wdev);

	return TRUE;
}

INT Set_ApCli_Enable_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	UINT Enable;
	BOOLEAN flag;
	POS_COOKIE pObj;
	UCHAR ifIndex;
	struct wifi_dev *wdev = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	wdev = &pAd->StaCfg[ifIndex].wdev;

#ifndef APCLI_CFG80211_SUPPORT
	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;
#endif

	Enable = os_str_tol(arg, 0, 16);
	flag = (Enable > 0) ? TRUE : FALSE;

	SetApCliEnableByWdev(pAd, wdev, flag);

	return TRUE;
}
#endif

#ifdef APCLI_SUPPORT
INT Set_ApCli_Ssid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR ifIndex;
	BOOLEAN apcliEn;
	INT success = FALSE;
	STA_ADMIN_CONFIG *apcli_entry;
	struct wifi_dev *wdev;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	if (strlen(arg) <= MAX_LEN_OF_SSID) {
		apcli_entry = &pAd->StaCfg[ifIndex];
		wdev = &apcli_entry->wdev;
		/* bring apcli interface down first */
		apcliEn = apcli_entry->ApcliInfStat.Enable;

#ifdef CONFIG_OWE_SUPPORT
		sta_reset_owe_parameters(pAd, ifIndex);
#endif

#ifdef APCLI_CONNECTION_TRIAL

		if (pAd->StaCfg[ifIndex].TrialCh == 0) {
#endif /* APCLI_CONNECTION_TRIAL */

			if (apcliEn == TRUE) {
				apcli_entry->ApcliInfStat.Enable = FALSE;
				ApCliIfDown(pAd);
			}

#ifdef APCLI_CONNECTION_TRIAL
		}

#endif /* APCLI_CONNECTION_TRIAL */
		apcli_entry->ApcliInfStat.bPeerExist = FALSE;
		NdisZeroMemory(apcli_entry->CfgSsid, MAX_LEN_OF_SSID);
		NdisMoveMemory(apcli_entry->CfgSsid, arg, strlen(arg));
		apcli_entry->CfgSsidLen = (UCHAR)strlen(arg);
		success = TRUE;
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) (Len=%d,Ssid=%s)\n",
				 ifIndex, apcli_entry->CfgSsidLen, apcli_entry->CfgSsid);
		apcli_entry->ApcliInfStat.Enable = apcliEn;
	} else
		success = FALSE;

	return success;
}


INT Set_ApCli_Bssid_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	INT i;
	RTMP_STRING *value;
	UCHAR ifIndex;
	BOOLEAN apcliEn;
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;

#ifdef CONFIG_OWE_SUPPORT
	sta_reset_owe_parameters(pAd, ifIndex);
#endif

	/* bring apcli interface down first */
	if (apcliEn == TRUE && (strcmp(arg, "00:00:00:00:00:00") != 0)) {
		pAd->StaCfg[ifIndex].ApcliInfStat.Enable = FALSE;
#ifdef APCLI_CONNECTION_TRIAL

		if (pAd->StaCfg[ifIndex].TrialCh == 0)
#endif /* APCLI_CONNECTION_TRIAL */
			ApCliIfDown(pAd);
	}

	NdisZeroMemory(pAd->StaCfg[ifIndex].CfgApCliBssid, MAC_ADDR_LEN);

	if (strlen(arg) == 17) { /* Mac address acceptable format 01:02:03:04:05:06 length 17 */
		for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
				return FALSE;  /* Invalid */

			AtoH(value, &pAd->StaCfg[ifIndex].CfgApCliBssid[i], 1);
		}

		if (i != 6)
			return FALSE;  /* Invalid */
	}

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(%2X:%2X:%2X:%2X:%2X:%2X)\n",
			 pAd->StaCfg[ifIndex].CfgApCliBssid[0],
			 pAd->StaCfg[ifIndex].CfgApCliBssid[1],
			 pAd->StaCfg[ifIndex].CfgApCliBssid[2],
			 pAd->StaCfg[ifIndex].CfgApCliBssid[3],
			 pAd->StaCfg[ifIndex].CfgApCliBssid[4],
			 pAd->StaCfg[ifIndex].CfgApCliBssid[5]);
	pAd->StaCfg[ifIndex].ApcliInfStat.Enable = apcliEn;
	return TRUE;
}

#ifdef APCLI_CFG80211_SUPPORT
INT Set_ApCli_AuthMode(RTMP_ADAPTER *pAd, INT staidx, RTMP_STRING *arg)
{
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	struct wifi_dev *wdev;

	wdev = &pAd->StaCfg[staidx].wdev;

	pSecConfig = &wdev->SecConfig;

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pSecConfig == NULL, arg=%s\n", arg);
		return FALSE;
	}
	SetWdevAuthMode(pSecConfig, arg);
	return TRUE;
}


INT Set_ApCli_EncrypType(RTMP_ADAPTER *pAd, INT staidx, RTMP_STRING *arg)
{
	/*struct _SECURITY_CONFIG *pSecConfig = &pAd->ApCfg.ApCliTab[0].wdev.SecConfig;*/
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	struct wifi_dev *wdev;

	wdev = &pAd->StaCfg[staidx].wdev;

	pSecConfig = &wdev->SecConfig;

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pSecConfig == NULL, arg=%s\n", arg);
		return FALSE;
	}
	SetWdevEncrypMode(pSecConfig, arg);
	return TRUE;
}

INT Set_ApCli_Enable(
	IN  PRTMP_ADAPTER pAd,
	IN 	INT staidx,
	IN  RTMP_STRING *arg)
{
	UINT Enable;
	UCHAR ifIndex;
	struct wifi_dev *wdev = NULL;

	ifIndex = staidx;

#ifdef CONFIG_OWE_SUPPORT
	sta_reset_owe_parameters(pAd, ifIndex);
#endif

	Enable = os_str_tol(arg, 0, 16);
	pAd->StaCfg[ifIndex].ApcliInfStat.Enable = (Enable > 0) ? TRUE : FALSE;
	MTWF_PRINT("I/F(apcli%d) Set_ApCli_Enable_Proc::(enable = %d)\n", ifIndex,
				pAd->StaCfg[ifIndex].ApcliInfStat.Enable);
	wdev = &pAd->StaCfg[ifIndex].wdev;
#ifdef APCLI_CONNECTION_TRIAL

	if (pAd->StaCfg[ifIndex].TrialCh == 0)
#endif /* APCLI_CONNECTION_TRIAL */
		ApCliIfDown(pAd);

	/*Fix for TGac 5.2.57*/
	wlan_operate_set_prim_ch(wdev, wdev->channel);
	return TRUE;
}

INT Set_ApCli_Ssid(RTMP_ADAPTER *pAd, INT staidx, RTMP_STRING *arg)
{
	UCHAR ifIndex;
	BOOLEAN apcliEn;
	INT success = FALSE;
	STA_ADMIN_CONFIG *apcli_entry;
	struct wifi_dev *wdev;

	ifIndex = staidx;

	if (strlen(arg) <= MAX_LEN_OF_SSID) {
		apcli_entry = &pAd->StaCfg[ifIndex];
		wdev = &apcli_entry->wdev;
		/* bring apcli interface down first */
		apcliEn = apcli_entry->ApcliInfStat.Enable;

#ifdef CONFIG_OWE_SUPPORT
		sta_reset_owe_parameters(pAd, ifIndex);
#endif

#ifdef APCLI_CONNECTION_TRIAL

		if (pAd->StaCfg[ifIndex].TrialCh == 0) {
#endif /* APCLI_CONNECTION_TRIAL */

			if (apcliEn == TRUE) {
				apcli_entry->ApcliInfStat.Enable = FALSE;
				ApCliIfDown(pAd);
			}

#ifdef APCLI_CONNECTION_TRIAL
		}

#endif /* APCLI_CONNECTION_TRIAL */
		apcli_entry->ApcliInfStat.bPeerExist = FALSE;
		NdisZeroMemory(apcli_entry->CfgSsid, MAX_LEN_OF_SSID);
		NdisMoveMemory(apcli_entry->CfgSsid, arg, strlen(arg));
		apcli_entry->CfgSsidLen = (UCHAR)strlen(arg);
		success = TRUE;
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"I/F(apcli%d) (Len=%d,Ssid=%s)\n", ifIndex, apcli_entry->CfgSsidLen, apcli_entry->CfgSsid);
		apcli_entry->ApcliInfStat.Enable = apcliEn;
#ifdef APCLI_CFG80211_SUPPORT
		apcli_entry->ReadyToConnect = TRUE;
#endif
	} else
		success = FALSE;

	return success;
}

#endif /* APCLI_CFG80211_SUPPORT */


INT Set_ApCli_TxMode_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev;
	UINT ifIndex = pObj->ioctl_if;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	if (ifIndex >= MAX_MULTI_STA) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid ioctl_if %d\n", ifIndex);
		return FALSE;
	}

	wdev = &pAd->StaCfg[ifIndex].wdev;
	wdev->DesiredTransmitSetting.field.FixedTxMode = RT_CfgSetFixedTxPhyMode(arg);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) = %d\n",
			 ifIndex, wdev->DesiredTransmitSetting.field.FixedTxMode);
	return TRUE;
}

INT Set_ApCli_WirelessMode_Proc(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev;
	struct dev_rate_info *rate;
	USHORT PhyMode, WirelessMode = os_str_tol(arg, 0, 10);
	CHANNEL_CTRL *pChCtrl;
	UCHAR BandIdx;
	STA_ADMIN_CONFIG *apcli_entry;
	UINT ifIndex = pObj->ioctl_if;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	if (ifIndex >= MAX_MULTI_STA) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid ioctl_if %d\n", ifIndex);
		return FALSE;
	}

	apcli_entry = &pAd->StaCfg[ifIndex];
	wdev = &apcli_entry->wdev;
	rate = &wdev->rate;
	PhyMode = cfgmode_2_wmode(WirelessMode);

	if (!APCLI_IF_UP_CHECK(pAd, ifIndex))
		return FALSE;

	/* apcli always follow per band's channel */
	if (WMODE_CAP_5G(PhyMode))
		wdev->channel = HcGetChannelByRf(pAd, RFIC_5GHZ);
	else
		wdev->channel = HcGetChannelByRf(pAd, RFIC_24GHZ);

	if (wdev_do_linkdown(wdev) != TRUE)
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, "linkdown fail!\n");

	os_msec_delay(100);

	if (wdev_do_close(wdev) != TRUE)
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "close fail!\n");

	/* fixed race condition between hw control task */
	os_msec_delay(100);
	wdev->PhyMode = PhyMode;
	HcAcquireRadioForWdev(pAd, wdev);
	/* Change channel state to NONE */
	BandIdx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
#ifdef EXT_BUILD_CHANNEL_LIST
	BuildChannelListEx(pAd, wdev);
#else
	BuildChannelList(pAd, wdev);
#endif
	/*update rate info for wdev*/
	RTMPUpdateRateInfo(wdev->PhyMode, rate);

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	rtmpeapupdaterateinfo(wdev->PhyMode, rate, &wdev->eap);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

	RTMPSetPhyMode(pAd, wdev, PhyMode);

	/* sync bw ext ch from mbss wdev */
	{
		if (pAd->CommonCfg.dbdc_mode == TRUE) {
			int mbss_idx;

			for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum ; mbss_idx++) {
				if (pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode == wdev->PhyMode) {
					update_att_from_wdev(wdev, &pAd->ApCfg.MBSSID[mbss_idx].wdev);
					break;
				}
			}
		} else {
			/* align phy mode to BSS0 by default */
			wdev->PhyMode = pAd->ApCfg.MBSSID[BSS0].wdev.PhyMode;
			update_att_from_wdev(wdev, &pAd->ApCfg.MBSSID[BSS0].wdev);
		}
	}
	os_msec_delay(100);

	/* Security initial  */
	if (wdev->SecConfig.AKMMap == 0x0)
		SET_AKM_OPEN(wdev->SecConfig.AKMMap);

	if (wdev->SecConfig.PairwiseCipher == 0x0) {
		SET_CIPHER_NONE(wdev->SecConfig.PairwiseCipher);
		SET_CIPHER_NONE(wdev->SecConfig.GroupCipher);
	}

	if (wdev_do_open(wdev) != TRUE)
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, "open fail!!!\n");

	os_msec_delay(100);
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) = %d\n",
			 ifIndex, PhyMode);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "SupRate[0]=%x\n", rate->legacy_rate.sup_rate[0]);
	return TRUE;
}

INT Set_ApCli_TxMcs_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev;
	UINT ifIndex = pObj->ioctl_if;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	if (ifIndex >= MAX_MULTI_STA) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid ioctl_if %d\n", ifIndex);
		return FALSE;
	}

	wdev = &pAd->StaCfg[ifIndex].wdev;
	wdev->DesiredTransmitSetting.field.MCS =
		RT_CfgSetTxMCSProc(arg, &wdev->bAutoTxRateSwitch);

	if (wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO)
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) Set_ApCli_TxMcs_Proc = AUTO\n", ifIndex);
	else {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) Set_ApCli_TxMcs_Proc = %d\n",
				 ifIndex, wdev->DesiredTransmitSetting.field.MCS);
	}

	return TRUE;
}

#ifdef APCLI_SUPPORT
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
INT Set_ApCli_Twt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR ifIndex;
	INT success = TRUE;
	STA_ADMIN_CONFIG *apcli_entry;
	struct wifi_dev *wdev;
	UCHAR i;
	PUCHAR	macptr;
	UINT16 au2Setting[CMD_TWT_MAX_PARAMS];
	struct twt_ctrl_t rTWTCtrl;
	struct twt_params_t *prTWTParams;
	UCHAR i4Argc;
	struct msg_twt_fsm_t msg = {0};
	MAC_TABLE_ENTRY *pEntry = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	i4Argc = 0;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	apcli_entry = &pAd->StaCfg[ifIndex];
	wdev = &apcli_entry->wdev;
	pEntry = (MAC_TABLE_ENTRY *)apcli_entry->pAssociatedAPEntry;

	if (!pEntry)
		return FALSE;

	for (i = 0, macptr = rstrtok(arg, "-"); macptr && (i < ARRAY_SIZE(au2Setting)); macptr = rstrtok(NULL, "-"), i++) {
		au2Setting[i] = (UINT16)simple_strtol(macptr, 0, 10);
		i4Argc++;
	}

	if (i4Argc == CMD_TWT_ACTION_TEN_PARAMS ||
		i4Argc == CMD_TWT_ACTION_THREE_PARAMS) {

		if ((IS_TWT_PARAM_ACTION_DEL(au2Setting[0]) ||
		IS_TWT_PARAM_ACTION_SUSPEND(au2Setting[0]) ||
		IS_TWT_PARAM_ACTION_RESUME(au2Setting[0])) &&
		i4Argc == CMD_TWT_ACTION_THREE_PARAMS) {

			MTWF_PRINT("Action=%d\n", au2Setting[0]);
			MTWF_PRINT("TWT Flow ID=%d\n", au2Setting[1]);
			MTWF_PRINT("ucBssIdx=%d\n", wdev->bss_info_argument.ucBssIndex);

			if (au2Setting[1] >= TWT_MAX_FLOW_NUM) {
				/* Simple sanity check failure */
				return FALSE;
			}

			rTWTCtrl.ucBssIdx = wdev->bss_info_argument.ucBssIndex;
			rTWTCtrl.ucCtrlAction = au2Setting[0];
			rTWTCtrl.ucTWTFlowId = au2Setting[1];

		} else if (i4Argc == CMD_TWT_ACTION_TEN_PARAMS) {
			MTWF_PRINT("Action bitmap=%d\n", au2Setting[0]);
			MTWF_PRINT("TWT Flow ID=%d Setup Command=%d Trig enabled=%d\n",
				au2Setting[1], au2Setting[2], au2Setting[3]);
			MTWF_PRINT("Unannounced enabled=%d Wake Interval Exponent=%d\n",
					au2Setting[4], au2Setting[5]);
			MTWF_PRINT("Protection enabled=%d Duration=%d\n",
					au2Setting[6], au2Setting[7]);
			MTWF_PRINT("Wake Interval Mantissa=%d\n", au2Setting[8]);
			MTWF_PRINT("ucBssIdx=%d\n", wdev->bss_info_argument.ucBssIndex);

			/*
			 *  au2Setting[0]: Whether bypassing nego or not
			 *  au2Setting[1]: TWT Flow ID
			 *  au2Setting[2]: TWT Setup Command
			 *  au2Setting[3]: Trigger enabled
			 *  au2Setting[4]: Unannounced enabled
			 *  au2Setting[5]: TWT Wake Interval Exponent
			 *  au2Setting[6]: TWT Protection enabled
			 *  au2Setting[7]: Nominal Minimum TWT Wake Duration
			 *  au2Setting[8]: TWT Wake Interval Mantissa
			 */
			if (au2Setting[1] >= TWT_MAX_FLOW_NUM ||
				au2Setting[2] > TWT_SETUP_CMD_DEMAND ||
				au2Setting[5] > TWT_MAX_WAKE_INTVAL_EXP) {
				/* Simple sanity check failure */
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR, "Invalid TWT Params\n");
				return FALSE;
			}

			prTWTParams = &(rTWTCtrl.rTWTParams);
			NdisZeroMemory(prTWTParams, sizeof(struct twt_params_t));
			prTWTParams->fgReq = TRUE;
			prTWTParams->ucSetupCmd = (UINT8) au2Setting[2];
			prTWTParams->fgTrigger = (au2Setting[3]) ? TRUE : FALSE;
			prTWTParams->fgUnannounced = (au2Setting[4]) ? TRUE : FALSE;
			prTWTParams->ucWakeIntvalExponent = (UINT8) au2Setting[5];
			prTWTParams->fgProtect = (au2Setting[6]) ? TRUE : FALSE;
			prTWTParams->ucMinWakeDur = (UINT8) au2Setting[7];
			prTWTParams->u2WakeIntvalMantiss = au2Setting[8];

			rTWTCtrl.ucBssIdx = wdev->bss_info_argument.ucBssIndex;
			rTWTCtrl.ucCtrlAction = au2Setting[0];
			rTWTCtrl.ucTWTFlowId = au2Setting[1];
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"wrong argc for update agrt: %d\n", i4Argc);
			return FALSE;
		}

		msg.wdev = wdev;
		msg.ucTWTFlowId = rTWTCtrl.ucTWTFlowId;
		msg.eMsgId = MID_TWT_PARAMS_SET;

		NdisCopyMemory(&msg.rtwtCtrl, &rTWTCtrl, sizeof(rTWTCtrl));

		/* enqueue message */
		MlmeEnqueueWithWdev(pAd,
			ACTION_STATE_MACHINE,
			MT2_MLME_S1G_CATE_TWT_SETUP,
			sizeof(struct msg_twt_fsm_t),
			(PVOID)&msg,
			0,
			wdev);
		RTMP_MLME_HANDLER(pAd);
	}

	return success;
}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
#endif /* APCLI_SUPPORT */

#ifdef APCLI_CONNECTION_TRIAL
INT Set_ApCli_Trial_Ch_Proc(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg)
{
	POS_COOKIE		pObj;
	UCHAR			ifIndex;
	PSTA_ADMIN_CONFIG	pApCliEntry = NULL;
	CHAR *str;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	if (ifIndex != (pAd->ApCfg.ApCliNum - 1)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "\n\rI/F(apcli%d) can not run connection trial, use apcli%d\n",
				  ifIndex, (MAX_APCLI_NUM - 1));
		return FALSE;
	}

	if (pAd->CommonCfg.dbdc_mode == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "\n\rI/F(apcli%d) can not run connection trial with DBDC mode\n",
				  ifIndex);
		return FALSE;
	}

	pApCliEntry = &pAd->StaCfg[ifIndex];
	pApCliEntry->TrialCh = os_str_tol(arg, 0, 10);

	if (pApCliEntry->TrialCh)
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) pApCliEntry->TrialCh = %d\n", ifIndex,
				 pApCliEntry->TrialCh);
	else
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) pApCliEntry->TrialCh = %d\n", ifIndex,
				 pApCliEntry->TrialCh);

#ifdef DBDC_MODE

	if ((WMODE_CAP_2G(pApCliEntry->wdev.PhyMode) && pApCliEntry->TrialCh > 14) ||
		(WMODE_CAP_5G(pApCliEntry->wdev.PhyMode) && pApCliEntry->TrialCh <= 14)) {
		pApCliEntry->TrialCh = 0;
		str = wmode_2_str(pApCliEntry->wdev.PhyMode);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "I/F(apcli%d) TrialCh = %d is not in this phy mode(%s)\n",
				  ifIndex, pApCliEntry->TrialCh, str);

		if (str)
			os_free_mem(str);

		return FALSE;
	}

#endif
	return TRUE;
}
#endif /* APCLI_CONNECTION_TRIAL */

#ifdef WPA_SUPPLICANT_SUPPORT
INT Set_ApCli_Wpa_Support(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE		pObj;
	UCHAR			ifIndex;
	PSTA_ADMIN_CONFIG	pApCliEntry = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->StaCfg[ifIndex];

	if (os_str_tol(arg, 0, 10) == 0)
		pApCliEntry->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_DISABLE;
	else if (os_str_tol(arg, 0, 10) == 1)
		pApCliEntry->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE;
	else if (os_str_tol(arg, 0, 10) == 2)
		pApCliEntry->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE_WITH_WEB_UI;
	else
		pApCliEntry->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_DISABLE;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(WpaSupplicantUP=%d)\n",
			 pApCliEntry->wpa_supplicant_info.WpaSupplicantUP);
	return TRUE;
}

INT	Set_ApCli_IEEE8021X_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	ULONG ieee8021x;
	POS_COOKIE		pObj;
	UCHAR			ifIndex;
	PSTA_ADMIN_CONFIG	pApCliEntry = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	ieee8021x = os_str_tol(arg, 0, 10);

	if (ieee8021x == 1)
		pApCliEntry->wdev.IEEE8021X = TRUE;
	else if (ieee8021x == 0)
		pApCliEntry->wdev.IEEE8021X = FALSE;
	else
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d) (IEEE8021X=%d)\n",
			 pObj->ioctl_if, pApCliEntry->wdev.IEEE8021X);
	return TRUE;
}
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
INT Set_Cli_Link_Map_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE          pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT                ifIndex;
	UCHAR MbssIdx = 0;
	struct wifi_dev *cli_link_wdev = NULL;
	struct wifi_dev *mbss_link_wdev = NULL;

	if (pAd->ApCfg.bMACRepeaterEn != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Rept has not been enabled yet.\n");
		return FALSE;
	}

	if ((pObj->ioctl_if_type != INT_APCLI) &&
		(pObj->ioctl_if_type != INT_MSTA))
		return FALSE;

	ifIndex = pObj->ioctl_if;

	if (ifIndex >= MAX_APCLI_NUM) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wrong cli link idx:%d to set link map.\n", ifIndex);
		return FALSE;
	}

	cli_link_wdev = &pAd->StaCfg[ifIndex].wdev;
	MbssIdx = os_str_tol(arg, 0, 10);

	if (!VALID_MBSS(pAd, MbssIdx)) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "wrong mbss idx:%d to set link map.\n", MbssIdx);
		return FALSE;
	}

	mbss_link_wdev = &pAd->ApCfg.MBSSID[MbssIdx].wdev;
	UpdateMbssCliLinkMap(pAd, MbssIdx, cli_link_wdev, mbss_link_wdev);
	return TRUE;
}

INT Set_ReptMode_Enable_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	UCHAR Enable = os_str_tol(arg, 0, 10);
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev;
	UCHAR band_idx = 0;
	UINT ifIndex = pObj->ioctl_if;

	switch (pObj->ioctl_if_type) {
	case INT_MAIN:
	case INT_MBSSID:
		if (VALID_MBSS(pAd, ifIndex)){
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			band_idx = HcGetBandByWdev(wdev);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index \n");
			return FALSE;
		}
		break;

	case INT_APCLI:
		if (ifIndex < MAX_MULTI_STA) {
			wdev = &pAd->StaCfg[ifIndex].wdev;
			band_idx = HcGetBandByWdev(wdev);
		} else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error station index \n");
			return FALSE;
		}
		break;
	default:
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "No Support This Type\n");
		return FALSE;

	}

	AsicSetReptFuncEnable(pAd, Enable, band_idx);

	return TRUE;
}

#endif /* MAC_REPEATER_SUPPORT */

#ifdef APCLI_AUTO_CONNECT_SUPPORT
/*
    ==========================================================================
    Description:
	Trigger Apcli Auto connect to find the missed AP.
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/

/*
0 : Disable apcli auto connect
1 : User Trigger Scan Mode
2 : Partial Scan Mode
3 : Driver Trigger Scan Mode
*/

INT Set_ApCli_AutoConnect_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR				ifIndex;
	AP_ADMIN_CONFIG *pApCfg;
	NDIS_802_11_SSID Ssid;
	struct wifi_dev *wdev;
	SCAN_CTRL *ScanCtrl;
	STA_ADMIN_CONFIG *apcli_entry;
	long scan_mode = simple_strtol(arg, 0, 10);
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR is_sta_connected = FALSE;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	pApCfg = &pAd->ApCfg;
	ifIndex = pObj->ioctl_if;
	apcli_entry = &pAd->StaCfg[ifIndex];
	wdev = &pAd->StaCfg[ifIndex].wdev;
	NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));

	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

	/*Before doing ApCli_Auto_connect, need check CH_OP_OWNER_SCAN first*/
	if (GetCurrentChannelOpOwner(pAd, wdev) == CH_OP_OWNER_SCAN) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Current scan is ongoing, just return and try ApCliAutoConnect next time\n");
		return TRUE;
	}

	if (cntl_idle(wdev) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "CNTL in reset SYNC\n");
		SetApCliEnableByWdev(pAd, wdev, FALSE);
		sync_fsm_reset(pAd, wdev);
		cntl_fsm_reset(wdev);
	}

	pEntry = MacTableLookup(pAd, wdev->bssid);
	if (pEntry && scan_mode == 4)
		is_sta_connected = TRUE;

#ifdef APCLI_CERT_SUPPORT
	/*Modfied RTS Threshold for 5.2.67A - Reverting back to Default */
	if (pAd->bApCliCertTest == TRUE) {
		wlan_operate_set_rts_len_thld(wdev, wlan_config_get_rts_len_thld(wdev));
	}
#endif
	if (scan_mode == 0) {/* disable it */
		apcli_entry->ApcliInfStat.AutoConnectFlag = FALSE;
		apcli_entry->ApCliAutoConnectRunning = FALSE;
		apcli_entry->ApCliAutoConnectType = 0;
		return TRUE;
	}

	if (APCLI_IF_UP_CHECK(pAd, ifIndex) != TRUE) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"[%s Abort] is not Enabled, please do ifconfig %s up first\n", wdev->if_dev->name, wdev->if_dev->name);
		return TRUE;
	}

	pAd->StaCfg[ifIndex].ApcliInfStat.AutoConnectFlag = TRUE;
	if ((scan_mode != 4) && !is_sta_connected)
		SetApCliEnableByWdev(pAd, wdev, FALSE);
	apcli_entry->ApCliAutoConnectRunning = TRUE;
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) (Len=%d,Ssid=%s)\n",
			 ifIndex, pAd->StaCfg[ifIndex].CfgSsidLen, pAd->StaCfg[ifIndex].CfgSsid);

	/*
		use site survey function to trigger auto connecting (when pAd->ApCfg.ApAutoConnectRunning == TRUE)
	*/
	switch (scan_mode) {
	case 4:
		apcli_entry->ApcliInfStat.AutoConnectFlag = FALSE;
		apcli_entry->ApCliAutoConnectRunning = FALSE;
		apcli_entry->ApCliAutoConnectType = TRIGGER_SCAN_BY_DRIVER;
		ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, &pAd->StaCfg[ifIndex].wdev);
		break;
	case 2:
		ScanCtrl->PartialScan.pwdev = wdev;
		ScanCtrl->PartialScan.bScanning = TRUE;
		apcli_entry->ApCliAutoConnectType = TRIGGER_SCAN_BY_USER;
		break;

	case 3:
		apcli_entry->ApCliAutoConnectType = TRIGGER_SCAN_BY_DRIVER;
		ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, &pAd->StaCfg[ifIndex].wdev);
		break;

	case 1:

	/* FALL Through: */
	default:
		apcli_entry->ApCliAutoConnectType = TRIGGER_SCAN_BY_USER;
		ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, &pAd->StaCfg[ifIndex].wdev);
		break;
	}

	return TRUE;
}

#endif  /* APCLI_AUTO_CONNECT_SUPPORT */

#ifdef WSC_AP_SUPPORT
INT Set_AP_WscSsid_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE		pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR			ifIndex = pObj->ioctl_if;
	PWSC_CTRL	    pWscControl = &pAd->StaCfg[ifIndex].wdev.WscControl;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	NdisZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));

	if ((strlen(arg) > 0) && (strlen(arg) <= MAX_LEN_OF_SSID)) {
		NdisMoveMemory(pWscControl->WscSsid.Ssid, arg, strlen(arg));
		pWscControl->WscSsid.SsidLength = strlen(arg);
		NdisZeroMemory(pAd->StaCfg[ifIndex].CfgSsid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->StaCfg[ifIndex].CfgSsid, arg, strlen(arg));
		pAd->StaCfg[ifIndex].CfgSsidLen = (UCHAR)strlen(arg);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(Select SsidLen=%d,Ssid=%s)\n",
				 pWscControl->WscSsid.SsidLength, pWscControl->WscSsid.Ssid);
	} else
		return FALSE;	/*Invalid argument */

	return TRUE;
}

#ifdef APCLI_SUPPORT
INT Set_ApCli_WscScanMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR Mode = TRIGGER_FULL_SCAN;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	PWSC_CTRL pWscControl = &pAd->StaCfg[ifIndex].wdev.WscControl;
	struct wifi_dev *wdev = NULL;
	SCAN_CTRL *ScanCtrl;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	wdev = &pAd->StaCfg[ifIndex].wdev;
	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

	Mode = simple_strtol(arg, 0, 10);
	if (Mode != TRIGGER_PARTIAL_SCAN)
		Mode = TRIGGER_FULL_SCAN;

#ifdef AP_SCAN_SUPPORT

	if ((pWscControl->WscApCliScanMode == TRIGGER_PARTIAL_SCAN) &&
		(Mode != TRIGGER_PARTIAL_SCAN)) {
		ScanCtrl->PartialScan.pwdev = NULL;
		ScanCtrl->PartialScan.LastScanChannel = 0;
		ScanCtrl->PartialScan.bScanning = FALSE;
	}

#endif /* AP_SCAN_SUPPORT */
	pWscControl->WscApCliScanMode = Mode;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(WscApCliScanMode=%d)\n", Mode);
	return TRUE;
}
#endif /* APCLI_SUPPORT */

#endif /* WSC_AP_SUPPORT */

#ifdef APCLI_SUPPORT
INT Set_ApCli_Cert_Enable_Proc(
	IN  RTMP_ADAPTER *pAd,
	IN  RTMP_STRING *arg)
{
	UINT Enable;
	POS_COOKIE pObj;
	UCHAR ifIndex;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	Enable = os_str_tol(arg, 0, 16);
	pAd->bApCliCertTest = (Enable > 0) ? TRUE : FALSE;

	if (pAd->bApCliCertTest == TRUE) {
		pAd->CommonCfg.bEnableTxBurst = FALSE;
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) (enable = %d)\n",
			 ifIndex, pAd->bApCliCertTest);
	return TRUE;
}

INT Set_ApCli_Cert_Agg_Limit_Proc(
	IN  RTMP_ADAPTER *pAd,
	IN  RTMP_STRING *arg)
{
	{
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820E2028, 0x0a0a0a0a);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820F2028, 0x0a0a0a0a);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820E202C, 0x0a0a0a0a);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820F202C, 0x0a0a0a0a);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820E2144, 0x0a0a0a0a);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820F2144, 0x0a0a0a0a);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820E214C, 0x0a0a0a0a);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820F214C, 0x0a0a0a0a);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820E2154, 0x0a0a0a0a);
		RTMP_IO_WRITE32(pAd->hdev_ctrl, 0x820F2154, 0x0a0a0a0a);
	}

	return TRUE;
}
#endif /* APCLI_CERT_SUPPORT */

/* Add for APCLI PMF 5.3.3.3 option test item. (Only Tx De-auth Req. and make sure the pkt can be Encrypted) */
INT ApCliTxDeAuth(
	IN PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	USHORT Reason = (USHORT)REASON_DEAUTH_STA_LEAVING;
	HEADER_802_11 DeauthHdr;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	NDIS_STATUS NStatus;
	POS_COOKIE pObj;
	UCHAR ifIndex;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "ifIndex=%u\n", ifIndex);

	if (ifIndex >= MAX_APCLI_NUM)
		return FALSE;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS)
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "DE-AUTH request (Reason=%d)...\n", Reason);
	ApCliMgtMacHeaderInit(pAd, &DeauthHdr, SUBTYPE_DEAUTH, 0, pAd->StaCfg[ifIndex].MlmeAux.Bssid,
						  pAd->StaCfg[ifIndex].MlmeAux.Bssid, ifIndex);

	MakeOutgoingFrame(pOutBuffer,           &FrameLen,
					  sizeof(HEADER_802_11), &DeauthHdr,
					  2,                    &Reason,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	return TRUE;
}

#ifdef ROAMING_ENHANCE_SUPPORT
INT Set_RoamingEnhance_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT Enable;
	POS_COOKIE pObj;
	UCHAR ifIndex;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	Enable = simple_strtol(arg, 0, 10);
	pAd->ApCfg.bRoamingEnhance = (Enable > 0) ? TRUE : FALSE;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(apcli%d) (enable = %d)\n",
			 ifIndex, pAd->ApCfg.bRoamingEnhance);
	return TRUE;
}
#endif /* ROAMING_ENHANCE_SUPPORT */
#endif /* APCLI_SUPPORT */

#ifdef WSC_AP_SUPPORT
#ifdef CON_WPS
static  INT WscPushConcurrentPBCAction(
	IN      PRTMP_ADAPTER   pAd,
	IN      PWSC_CTRL   pWscControl,
	IN      BOOLEAN     bIsApCli)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR           apidx = pObj->ioctl_if;
	INT                 idx;

	if (bIsApCli)
		pWscControl->WscConfMode = WSC_ENROLLEE;
	else
		pWscControl->WscConfMode = WSC_REGISTRAR;

	WscInit(pAd, bIsApCli, apidx);
	pWscControl->WscMode = WSC_PBC_MODE;
	WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl);
	WscStop(pAd, bIsApCli, pWscControl);
	pWscControl->RegData.ReComputePke = 1;
	WscInitRegistrarPair(pAd, pWscControl, apidx);

	for (idx = 0; idx < 192; idx++)
		pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAd);

	pWscControl->bWscAutoTigeer = FALSE;

	if (bIsApCli) {
		pAd->StaCfg[apidx].ApcliInfStat.Enable = FALSE;
		ApCliIfDown(pAd);
		pWscControl->WscSsid.SsidLength = 0;
		NdisZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));
		pWscControl->conWscStatus = CON_WPS_STATUS_APCLI_RUNNING;
		pWscControl->WscPBCBssCount = 0;
		pWscControl->con_wps_scan_trigger_count = 0;
		/* WPS - SW PBC */
		WscPushPBCAction(pAd, pWscControl);
	} else {
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

		WscBuildBeaconIE(pAd, pWscControl->WscConfStatus, TRUE, DEV_PASS_ID_PBC,
						 pWscControl->WscConfigMethods, apidx, NULL, 0, AP_MODE);
		WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, pWscControl->WscConfStatus, TRUE, DEV_PASS_ID_PBC,
							pWscControl->WscConfigMethods, apidx, NULL, 0, AP_MODE);
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
		RTMPSetTimer(&pWscControl->Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
		pWscControl->conWscStatus = CON_WPS_STATUS_AP_RUNNING;
		pWscControl->Wsc2MinsTimerRunning = TRUE;
		pWscControl->WscStatus = STATUS_WSC_LINK_UP;
		pWscControl->bWscTrigger = TRUE;
		RTMP_SEM_LOCK(&pWscControl->WscPeerListSemLock);
		WscClearPeerList(&pWscControl->WscPeerList);
		RTMP_SEM_UNLOCK(&pWscControl->WscPeerListSemLock);
	}

	return TRUE;
}

INT     Set_ConWpsApCliMode_Proc(
	RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef MULTI_INF_SUPPORT
	UINT Mode = CON_WPS_APCLI_BAND_AUTO;
	UINT opposBandIdx = !multi_inf_get_idx(pAd);
	PRTMP_ADAPTER pOpposAd = NULL;

	Mode = os_str_tol(arg, 0, 10);

	if (Mode >= CON_WPS_APCLI_BAND_MAX)
		return FALSE;

	pOpposAd = (PRTMP_ADAPTER)adapt_list[opposBandIdx];
	pAd->ApCfg.ConWpsApCliMode = Mode;

	if (pOpposAd != NULL) {
		pOpposAd->ApCfg.ConWpsApCliMode = Mode;
		MTWF_PRINT("Now: %s, Oppos: %s, Mode = %d\n", pAd->net_dev->name, pOpposAd->net_dev->name, Mode);
	} else {
		MTWF_PRINT("Now: %s, Mode = %d\n", pAd->net_dev->name, Mode);
	}

#else
	UINT Mode = CON_WPS_APCLI_BAND_AUTO;

	Mode = os_str_tol(arg, 0, 10);

	if (Mode >= CON_WPS_APCLI_BAND_MAX)
		return FALSE;

	pAd->ApCfg.ConWpsApCliMode = Mode;
	MTWF_PRINT("Now: %s, Mode = %d\n", pAd->net_dev->name, Mode);
#endif
	return TRUE;
}

INT     Set_ConWpsApcliAutoPreferIface_Proc(
	RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef MULTI_INF_SUPPORT
	UINT PreferIface = CON_WPS_APCLI_AUTO_PREFER_IFACE0;
	UINT opposBandIdx = !multi_inf_get_idx(pAd);
	PRTMP_ADAPTER pOpposAd = NULL;

	PreferIface = os_str_tol(arg, 0, 10);

	if (PreferIface >= CON_WPS_APCLI_AUTO_PREFER_IFACE_MAX)
		return FALSE;

	pOpposAd = (PRTMP_ADAPTER)adapt_list[opposBandIdx];
	pAd->ApCfg.ConWpsApcliAutoPreferIface = PreferIface;

	if (pOpposAd != NULL) {
		pOpposAd->ApCfg.ConWpsApcliAutoPreferIface = PreferIface;
		MTWF_PRINT("%s Now: %s, Oppos: %s, PreferIface = %d\n",
				 __func__, pAd->net_dev->name, pOpposAd->net_dev->name, PreferIface);
	} else {
		MTWF_PRINT("%s Now: %s, PreferIface = %d\n", __func__, pAd->net_dev->name, PreferIface);
	}

#else
	UINT PreferIface = CON_WPS_APCLI_AUTO_PREFER_IFACE0;

	PreferIface = os_str_tol(arg, 0, 10);
	pAd->ApCfg.ConWpsApcliAutoPreferIface = PreferIface;
	MTWF_PRINT("%s Now: %s, PreferIface = %d\n", __func__, pAd->net_dev->name, PreferIface);
#endif
	return TRUE;
}

INT     Set_ConWpsApCliDisabled_Proc(
	RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef MULTI_INF_SUPPORT
	UINT Disabled = FALSE;
	UINT opposBandIdx = !multi_inf_get_idx(pAd);
	PRTMP_ADAPTER pOpposAd = NULL;

	Disabled = os_str_tol(arg, 0, 10);
	pOpposAd = (PRTMP_ADAPTER)adapt_list[opposBandIdx];
	pAd->ApCfg.ConWpsApCliDisableSetting = Disabled;

	if (pOpposAd != NULL) {
		pOpposAd->ApCfg.ConWpsApCliDisableSetting = Disabled;
		MTWF_PRINT("%s Now: %s, Oppos: %s, ApClient Disabled = %d\n",
				 __func__, pAd->net_dev->name, pOpposAd->net_dev->name, Disabled);
	} else {
		MTWF_PRINT("%s Now: %s, ApClient Disabled = %d\n", __func__, pAd->net_dev->name, Disabled);
	}

#else
	UINT Disabled = FALSE;

	Disabled = os_str_tol(arg, 0, 10);
	pAd->ApCfg.ConWpsApCliDisableSetting = Disabled;
	MTWF_PRINT("%s Now: %s, ApClient Disabled = %d\n", __func__, pAd->net_dev->name, Disabled);
#endif
	return TRUE;
}

INT	Set_ConWpsApDisabled_Proc(
	RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef MULTI_INF_SUPPORT
	UINT Disabled = FALSE;
	UINT opposBandIdx = !multi_inf_get_idx(pAd);
	PRTMP_ADAPTER pOpposAd = NULL;

	Disabled = simple_strtol(arg, 0, 10);
	pOpposAd = (PRTMP_ADAPTER)adapt_list[opposBandIdx];
	pAd->ApCfg.ConWpsApDisableSetting = Disabled;

	if (pOpposAd != NULL) {
		pOpposAd->ApCfg.ConWpsApDisableSetting = Disabled;
		MTWF_PRINT("%s Now: %s, Oppos: %s, Ap Disabled = %d\n",
				 __func__, pAd->net_dev->name, pOpposAd->net_dev->name, Disabled);
	} else {
		MTWF_PRINT("%s Now: %s, Ap Disabled = %d\n", __func__, pAd->net_dev->name, Disabled);
	}

#else
	UINT Disabled = FALSE;

	Disabled = simple_strtol(arg, 0, 10);
	pAd->ApCfg.ConWpsApDisableSetting = Disabled;
	MTWF_PRINT("%s Now: %s, Ap Disabled = %d\n", __func__, pAd->net_dev->name, Disabled);
#endif
	return TRUE;
}
#endif /* CON_WPS */

INT	 Set_AP_WscConfMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	INT         ConfModeIdx;
	/*INT         IsAPConfigured; */
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if, mac_addr[MAC_ADDR_LEN];
	BOOLEAN     bFromApCli = FALSE;
	PWSC_CTRL   pWscControl;
#ifdef CON_WPS
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct wifi_dev *pWdev = NULL;
#endif
	ConfModeIdx = os_str_tol(arg, 0, 10);
#ifdef CON_WPS

	if (ConfModeIdx == WSC_ENROLLEE_REGISTRAR) {
		UINT ApClientWcid = 0;

		MTWF_PRINT("WPS is using concurrent WPS now apidx=%d\n", apidx);

		if ((pAd->ApCfg.ConWpsApCliDisableSetting == TRUE) &&
			(apidx < pAd->ApCfg.ApCliNum) &&
			(GetAssociatedAPByWdev(pAd, &pAd->StaCfg[apidx].wdev) != NULL)) {
			ApClientWcid = pAd->StaCfg[apidx].MacTabWCID;

			if ((pAd->MacTab.Content[ApClientWcid].Sst == SST_ASSOC) &&
				(tr_ctl->tr_entry[ApClientWcid].PortSecured == WPA_802_1X_PORT_SECURED))
				pAd->ApCfg.ConWpsApCliDisabled = TRUE;
		}

		if (pAd->ApCfg.ConWpsApCliDisabled == TRUE)
			MTWF_PRINT("Diabled the ApClient when using concurrent WPS now.\n");
		else if ((apidx < pAd->ApCfg.ApCliNum) &&
				 (pAd->StaCfg[apidx].ApcliInfStat.Enable)) {
			pWdev =  &(pAd->StaCfg[apidx].wdev);

			if (pWdev != NULL) {
				pWscControl = &pWdev->WscControl;
				MTWF_PRINT("Set_AP_WscConfMode_Proc enter WscPushConcurrentPBCAction(apcli%d) pWscControl=0x%p\n", apidx, pWscControl);
				RTMPZeroMemory(pWscControl->IfName, IFNAMSIZ);
				RTMPMoveMemory(pWscControl->IfName, pWdev->if_dev->name, IFNAMSIZ);
				WscPushConcurrentPBCAction(pAd, pWscControl, TRUE);
			}
		}

		if (pAd->ApCfg.ConWpsApDisableSetting == TRUE)
			MTWF_PRINT("Disable the AP when using concurrent WPS now\n");
		else {
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			MTWF_PRINT("Set_AP_WscConfMode_Proc enter WscPushConcurrentPBCAction(ra%d)\n",
					 apidx);
			WscPushConcurrentPBCAction(pAd, pWscControl, FALSE);
		}

		return TRUE;
	}

#endif /* CON_WPS */
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		bFromApCli = TRUE;
		pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "IF(apcli%d) Set_WscConfMode_Proc:: This command is from apcli interface now.\n", apidx);
	} else
#endif /* APCLI_SUPPORT */
#ifdef P2P_SUPPORT
		if (pObj->ioctl_if_type == INT_P2P) {
			if (P2P_CLI_ON(pAd)) {
				bFromApCli = TRUE;
				pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
			} else {
				bFromApCli = FALSE;
				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "IF(p2p%d) This command is from p2p interface now.\n", apidx);
		} else
#endif /* P2P_SUPPORT */
		{
			bFromApCli = FALSE;
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "IF(ra%d) This command is from ra interface now.\n", apidx);
		}


	if ((ConfModeIdx & WSC_ENROLLEE_PROXY_REGISTRAR) == WSC_DISABLE) {
		pWscControl->WscConfMode = WSC_DISABLE;
		pWscControl->WscStatus = STATUS_WSC_NOTUSED;

		if (bFromApCli)
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"IF(apcli%d) WPS is disabled.\n", apidx);
		else {
			struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"IF(ra%d) WPS is disabled.\n", apidx);
			/* Clear WPS IE in Beacon and ProbeResp */
			wdev->WscIEBeacon.ValueLen = 0;
			wdev->WscIEProbeResp.ValueLen = 0;
			UpdateBeaconHandler(
				pAd,
				wdev,
				BCN_UPDATE_IE_CHG);
		}
	} else {
#ifdef APCLI_SUPPORT

		if (bFromApCli) {
			if (ConfModeIdx == WSC_ENROLLEE) {
				pWscControl->WscConfMode = WSC_ENROLLEE;
				WscInit(pAd, TRUE, apidx);
			} else {
				MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "IF(apcli%d) Ap Client only supports Enrollee mode.(ConfModeIdx=%d)\n",
						  apidx, ConfModeIdx);
				return FALSE;
			}
		} else
#endif /* APCLI_SUPPORT */
		{
			pWscControl->WscConfMode = (ConfModeIdx & WSC_ENROLLEE_PROXY_REGISTRAR);
			WscInit(pAd, FALSE, apidx);
		}

		pWscControl->WscStatus = STATUS_WSC_IDLE;
	}

#ifdef APCLI_SUPPORT

	if (bFromApCli)
		memcpy(mac_addr, &pAd->StaCfg[apidx].wdev.if_addr[0], MAC_ADDR_LEN);
	else
#endif /* APCLI_SUPPORT */
	{
		memcpy(mac_addr, &pAd->ApCfg.MBSSID[apidx].wdev.bssid[0], MAC_ADDR_LEN);
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "IF("MACSTR") WscConfMode(0~7)=%d)\n",
			  MAC2STR(mac_addr), pWscControl->WscConfMode);
	return TRUE;
}

INT	Set_AP_WscConfStatus_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR       IsAPConfigured = 0;
	INT         IsSelectedRegistrar;
	USHORT      WscMode;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "IF(apcli%d) Set_WscConfStatus_Proc:: Ap Client doesn't need this command.\n", apidx);
		return FALSE;
	}

#endif /* APCLI_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

	if ((pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscV2Info.bWpsEnable == FALSE) &&
		(pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscV2Info.bEnableWpsV2)) {
		pAd->ApCfg.MBSSID[apidx].wdev.WscIEBeacon.ValueLen = 0;
		pAd->ApCfg.MBSSID[apidx].wdev.WscIEProbeResp.ValueLen = 0;
		return FALSE;
	}

#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef P2P_SUPPORT

	if (pObj->ioctl_if_type == INT_P2P) {
		if (P2P_CLI_ON(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "IF(p2p%d) P2P Client doesn't need this command.\n", apidx);
			return FALSE;
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "IF(p2p%d) This command is from p2p interface now.\n", apidx);
	}

#endif /* P2P_SUPPORT */
	IsAPConfigured = (UCHAR)os_str_tol(arg, 0, 10);
	IsSelectedRegistrar = pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscSelReg;

	if (pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscMode == 1)
		WscMode = DEV_PASS_ID_PIN;
	else
		WscMode = DEV_PASS_ID_PBC;

	if ((IsAPConfigured  > 0) && (IsAPConfigured  <= 2)) {
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

		pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfStatus = IsAPConfigured;
#ifdef WPS_UNCONFIG_FEATURE_SUPPORT
		if (pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfStatus == WSC_SCSTATE_UNCONFIGURED)
			pAd->ApCfg.MBSSID[apidx].wdev.WscControl.wps_unconfig_trigger = TRUE;
#endif
		/* Change SC State of WPS IE in Beacon and ProbeResp */
		WscBuildBeaconIE(pAd, IsAPConfigured, IsSelectedRegistrar, WscMode, 0, apidx, NULL, 0, AP_MODE);
		WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, IsSelectedRegistrar, WscMode, 0, apidx, NULL, 0,
							AP_MODE);
		UpdateBeaconHandler(
			pAd,
			wdev,
			BCN_UPDATE_IE_CHG);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "IF(ra%d) Set failed!!(WscConfStatus=%s), WscConfStatus is 1 or 2\n",
				  apidx, arg);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "IF(ra%d) WscConfStatus is not changed (%d)\n",
				  apidx, pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfStatus);
		return FALSE;  /*Invalid argument */
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "IF("MACSTR") (WscConfStatus=%d)\n",
			  MAC2STR(pAd->ApCfg.MBSSID[apidx].wdev.bssid),
			  pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfStatus);
	return TRUE;
}

INT	Set_AP_WscMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	INT         WscMode;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if, mac_addr[MAC_ADDR_LEN];
	PWSC_CTRL   pWscControl;
	BOOLEAN     bFromApCli = FALSE;
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		bFromApCli = TRUE;
		pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "IF(apcli%d) This command is from apcli interface now.\n",
				  apidx);
	} else
#endif /* APCLI_SUPPORT */
#ifdef P2P_SUPPORT
		if (pObj->ioctl_if_type == INT_P2P) {
			if (P2P_CLI_ON(pAd)) {
				bFromApCli = TRUE;
				pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
			} else {
				bFromApCli = FALSE;
				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "IF(p2p%d) This command is from p2p interface now.\n",
					  apidx);
		} else
#endif /* P2P_SUPPORT */
		{
			bFromApCli = FALSE;
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "IF(ra%d) This command is from ra interface now.\n",
					  apidx);
		}

	WscMode = os_str_tol(arg, 0, 10);

	if ((WscMode  > 0) && (WscMode  <= 2)) {
		pWscControl->WscMode = WscMode;

		if (WscMode == WSC_PBC_MODE)
			WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Set failed!!(WscMode=%s), WscConfStatus is 1 or 2\n",
				  arg);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "WscMode is not changed (%d)\n",
				  pWscControl->WscMode);
		return FALSE;  /*Invalid argument */
	}

#ifdef APCLI_SUPPORT

	if (bFromApCli)
		memcpy(mac_addr, pAd->StaCfg[apidx].wdev.if_addr, MAC_ADDR_LEN);
	else
#endif /* APCLI_SUPPORT */
	{
		memcpy(mac_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "IF("MACSTR") (WscMode=%d)\n",
			  MAC2STR(mac_addr), pWscControl->WscMode);
	return TRUE;
}

INT	Set_WscStatus_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d) (WscStatus=%d)\n",
			 apidx, pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscStatus);
	return TRUE;
}

#define WSC_GET_CONF_MODE_EAP	1
#define WSC_GET_CONF_MODE_UPNP	2
INT	Set_AP_WscGetConf_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	INT                 WscMode, wscGetConfMode = 0;
	INT                 IsAPConfigured;
	PWSC_CTRL           pWscControl;
	PWSC_UPNP_NODE_INFO pWscUPnPNodeInfo;
	INT	                idx;
	POS_COOKIE          pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	            apidx = pObj->ioctl_if, mac_addr[MAC_ADDR_LEN];
	BOOLEAN             bFromApCli = FALSE;
#ifdef WSC_V2_SUPPORT
	PWSC_V2_INFO		pWscV2Info = NULL;
#endif /* WSC_V2_SUPPORT */
#ifdef WSC_LED_SUPPORT
	UCHAR WPSLEDStatus;
#endif /* WSC_LED_SUPPORT */
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		if (pAd->StaCfg[apidx].ApcliInfStat.ApCliInit == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "IF(apcli%d) ApCli is disabled.\n", apidx);
			return FALSE;
		}

		bFromApCli = TRUE;
		apidx &= (~MIN_NET_DEVICE_FOR_APCLI);
		pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "IF(apcli%d) This command is from apcli interface now.\n", apidx);
	} else
#endif /* APCLI_SUPPORT */
#ifdef P2P_SUPPORT
		if (pObj->ioctl_if_type == INT_P2P) {
			if (P2P_CLI_ON(pAd)) {
				bFromApCli = TRUE;
				pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
			} else {
				bFromApCli = FALSE;
				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "IF(p2p%d) This command is from p2p interface now.\n", apidx);
		} else
#endif /* P2P_SUPPORT */
		{
			bFromApCli = FALSE;
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "IF(ra%d) This command is from ra interface now.\n",
					  apidx);
		}

	NdisZeroMemory(mac_addr, MAC_ADDR_LEN);
#ifdef WSC_V2_SUPPORT
	pWscV2Info = &pWscControl->WscV2Info;
#endif /* WSC_V2_SUPPORT */
	wscGetConfMode = os_str_tol(arg, 0, 10);
	IsAPConfigured = pWscControl->WscConfStatus;
	pWscUPnPNodeInfo = &pWscControl->WscUPnPNodeInfo;

	if ((pWscControl->WscConfMode == WSC_DISABLE)
#ifdef WSC_V2_SUPPORT
		|| ((pWscV2Info->bWpsEnable == FALSE) && (pWscV2Info->bEnableWpsV2))
#endif /* WSC_V2_SUPPORT */
	   ) {
		pWscControl->bWscTrigger = FALSE;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "WPS is disabled.\n");
		return FALSE;
	}

	WscStop(pAd, bFromApCli, pWscControl);
	/* trigger wsc re-generate public key */
	pWscControl->RegData.ReComputePke = 1;

	if (pWscControl->WscMode == 1)
		WscMode = DEV_PASS_ID_PIN;
	else
		WscMode = DEV_PASS_ID_PBC;

#ifdef P2P_SUPPORT

	if (pAd->P2pCfg.Dpid != DEV_PASS_ID_NOSPEC)
		WscMode = pAd->P2pCfg.Dpid;

#endif /* P2P_SUPPORT */
	WscInitRegistrarPair(pAd, pWscControl, apidx);

	/* Enrollee 192 random bytes for DH key generation */
	for (idx = 0; idx < 192; idx++)
		pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAd);

#ifdef APCLI_SUPPORT

	if (bFromApCli) {
		BOOLEAN apcliEn = pAd->StaCfg[apidx].ApcliInfStat.Enable;
		/* bring apcli interface down first */
		pAd->StaCfg[apidx].ApcliInfStat.Enable = FALSE;
		ApCliIfDown(pAd);
		/* Restoring the saved state of ApCliInterface in both cases - WPS PIN as well as PBC */
		pAd->StaCfg[apidx].ApcliInfStat.Enable = apcliEn;

		if (WscMode == DEV_PASS_ID_PIN) {
			NdisMoveMemory(pWscControl->RegData.SelfInfo.MacAddr,
						   pAd->StaCfg[apidx].wdev.if_addr, 6);
			/* Setting the WSC state here in case of WPS PIN - as WscPINAction is not called in PIN case */
			pWscControl->WscState = WSC_STATE_START;

		} else {
			pWscControl->WscSsid.SsidLength = 0;
			NdisZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));
			pWscControl->WscPBCBssCount = 0;
			/* WPS - SW PBC */
			WscPushPBCAction(pAd, pWscControl);
		}

		NdisMoveMemory(mac_addr, pAd->StaCfg[apidx].wdev.if_addr, MAC_ADDR_LEN);
	} else
#endif /* APCLI_SUPPORT */
	{
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

		WscBuildBeaconIE(pAd, IsAPConfigured, TRUE, WscMode, pWscControl->WscConfigMethods, apidx, NULL, 0, AP_MODE);
		WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, TRUE, WscMode, pWscControl->WscConfigMethods, apidx,
							NULL, 0, AP_MODE);
		UpdateBeaconHandler(
			pAd,
			wdev,
			BCN_UPDATE_IE_CHG);
		NdisMoveMemory(mac_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
	}

#ifdef APCLI_SUPPORT

	if (bFromApCli && (WscMode == DEV_PASS_ID_PBC))
		;
	else
#endif /* APCLI_SUPPORT */
	{
		/* 2mins time-out timer */
		RTMPSetTimer(&pWscControl->Wsc2MinsTimer, WSC_TWO_MINS_TIME_OUT);
		pWscControl->Wsc2MinsTimerRunning = TRUE;
		pWscControl->WscStatus = STATUS_WSC_LINK_UP;
/* WPS_BandSteering Support */
#ifdef BAND_STEERING
		/* WPS: clear any previosly existing WPS WHITELIST in case of AP wps trigger */
		if (!bFromApCli && (pAd->ApCfg.BandSteering)) {
			struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			PBND_STRG_CLI_TABLE table = Get_BndStrgTable(pAd, wdev->func_idx);

			if (table && table->bEnabled) {
				NdisAcquireSpinLock(&table->WpsWhiteListLock);
				ClearWpsWhiteList(&table->WpsWhiteList);
				NdisReleaseSpinLock(&table->WpsWhiteListLock);
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO, "channel %u wps whitelist cleared, size : %d\n",
				 table->Channel, table->WpsWhiteList.size);
			}
		}
#endif
		pWscControl->bWscTrigger = TRUE;
	}

	pWscControl->bWscAutoTigeer = FALSE;

	if (!bFromApCli) {
		if (WscMode == DEV_PASS_ID_PIN) {
			WscAssignEntryMAC(pAd, pWscControl);
			WscSendUPnPConfReqMsg(pAd, pWscControl->EntryIfIdx,
								  (PUCHAR)pAd->ApCfg.MBSSID[pWscControl->EntryIfIdx].Ssid,
								  pAd->ApCfg.MBSSID[apidx].wdev.bssid, 3, 0, AP_MODE);
		} else {
			RTMP_SEM_LOCK(&pWscControl->WscPeerListSemLock);
			WscClearPeerList(&pWscControl->WscPeerList);
			RTMP_SEM_UNLOCK(&pWscControl->WscPeerListSemLock);
		}
	}

#ifdef WSC_LED_SUPPORT
	WPSLEDStatus = LED_WPS_IN_PROCESS;
	RTMPSetLED(pAd, WPSLEDStatus, HcGetBandByWdev(pWscControl->wdev));
#endif /* WSC_LED_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "IF("MACSTR") trigger WSC state machine, wscGetConfMode=%d\n",
			  MAC2STR(mac_addr), wscGetConfMode);
	return TRUE;
}

INT	Set_AP_WscPinCode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT        PinCode = 0;
	BOOLEAN     validatePin, bFromApCli = FALSE;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if, mac_addr[MAC_ADDR_LEN];
	PWSC_CTRL   pWscControl;
#define IsZero(c) ('0' == (c) ? TRUE:FALSE)
	PinCode = os_str_tol(arg, 0, 10); /* When PinCode is 03571361, return value is 3571361. */
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		bFromApCli = TRUE;
		pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "IF(apcli%d) This command is from apcli interface now.\n", apidx);
	} else
#endif /* APCLI_SUPPORT */
#ifdef P2P_SUPPORT
		if (pObj->ioctl_if_type == INT_P2P) {
			if (P2P_CLI_ON(pAd)) {
				bFromApCli = TRUE;
				pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
			} else {
				bFromApCli = FALSE;
				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			}

			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "IF(p2p%d) This command is from p2p interface now.\n", apidx);
		} else
#endif /* P2P_SUPPORT */
		{
			bFromApCli = FALSE;
			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "IF(ra%d) This command is from ra interface now.\n", apidx);
		}

	if (strlen(arg) == 4)
		validatePin = TRUE;
	else
		validatePin = ValidateChecksum(PinCode);

	if (validatePin) {
		if (pWscControl->WscRejectSamePinFromEnrollee &&
			(PinCode == pWscControl->WscLastPinFromEnrollee)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PIN authentication or communication error occurs!!\n"
					 "Registrar does NOT accept the same PIN again!(PIN:%s)\n", arg);
			return FALSE;
		}

		pWscControl->WscPinCode = PinCode;
		pWscControl->WscLastPinFromEnrollee = pWscControl->WscPinCode;
		pWscControl->WscRejectSamePinFromEnrollee = FALSE;

		/* PIN Code */
		if (strlen(arg) == 4) {
			pWscControl->WscPinCodeLen = 4;
			pWscControl->RegData.PinCodeLen = 4;
			NdisMoveMemory(pWscControl->RegData.PIN, arg, 4);
		} else {
			pWscControl->WscPinCodeLen = 8;

			if (IsZero(*arg)) {
				pWscControl->RegData.PinCodeLen = 8;
				NdisMoveMemory(pWscControl->RegData.PIN, arg, 8);
			} else
				WscGetRegDataPIN(pAd, pWscControl->WscPinCode, pWscControl);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Set failed!!(WscPinCode=%s), PinCode Checksum invalid\n", arg);
		return FALSE;  /*Invalid argument */
	}

#ifdef APCLI_SUPPORT

	if (bFromApCli)
		memcpy(mac_addr, pAd->StaCfg[apidx].wdev.if_addr, MAC_ADDR_LEN);
	else
#endif /* APCLI_SUPPORT */
	{
		memcpy(mac_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "IF("MACSTR") (PinCode=%d)\n",
			  MAC2STR(mac_addr), pWscControl->WscPinCode);
	return TRUE;
}

INT	Set_WscOOB_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	char        *pTempSsid = NULL;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;
	int ret;
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "IF(apcli%d) Ap Client doesn't need this command.\n", apidx);
		return FALSE;
	}

#endif /* APCLI_SUPPORT */
#ifdef WSC_V2_SUPPORT
    Set_WscSetupLock_Proc(pAd, "0");
#endif
	Set_AP_WscConfStatus_Proc(pAd, "1");
	Set_SecAuthMode_Proc(pAd, "WPA2PSK");
	Set_SecEncrypType_Proc(pAd, "AES");
	pTempSsid = vmalloc(33);

	if (pTempSsid) {
		memset(pTempSsid, 0, 33);
		ret = snprintf(pTempSsid, 33, "RalinkInitialAP%02X%02X%02X",
				 pAd->ApCfg.MBSSID[apidx].wdev.bssid[3],
				 pAd->ApCfg.MBSSID[apidx].wdev.bssid[4],
				 pAd->ApCfg.MBSSID[apidx].wdev.bssid[5]);
		if (os_snprintf_error(33, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			vfree(pTempSsid);
			return FALSE;
		}

		Set_AP_SSID_Proc(pAd, pTempSsid);
		vfree(pTempSsid);
	}

	Set_SecWPAPSK_Proc(pAd, "RalinkInitialAPxx1234");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d) Set_WscOOB_Proc\n", apidx);
	return TRUE;
}

INT	Set_WscStop_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
	PWSC_CTRL   pWscControl;
	BOOLEAN     bFromApCli = FALSE;
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		bFromApCli = TRUE;
		pWscControl = &pAd->StaCfg[apidx].wdev.WscControl;
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "IF(apcli%d) This command is from apcli interface now.\n", apidx);
	} else
#endif /* APCLI_SUPPORT */
	{
		bFromApCli = FALSE;
		pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "IF(ra%d) This command is from ra interface now.\n", apidx);
	}

#ifdef APCLI_SUPPORT

	if (bFromApCli) {
		WscStop(pAd, TRUE, pWscControl);
		pWscControl->WscConfMode = WSC_DISABLE;
	} else
#endif /* APCLI_SUPPORT */
	{
		INT	 IsAPConfigured = pWscControl->WscConfStatus;
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

		WscBuildBeaconIE(pAd, IsAPConfigured, FALSE, 0, 0, apidx, NULL, 0, AP_MODE);
		WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0, apidx, NULL, 0, AP_MODE);
		UpdateBeaconHandler(
			pAd,
			wdev,
			BCN_UPDATE_IE_CHG);
		WscStop(pAd, FALSE, pWscControl);
	}

	pWscControl->bWscTrigger = FALSE;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<===== Set_WscStop_Proc");
	return TRUE;
}

#ifdef VENDOR_FEATURE6_SUPPORT
/* copy from RTMPIoctlWscProfile() but the strue is use WSC_CONFIGURED_VALUE_2 */
VOID RTMPGetCurrentCred(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	* wrq)
{
	WSC_CONFIGURED_VALUE_2 Profile;
	RTMP_STRING *msg;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	UINT msg_len = 2048;

	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	memset(&Profile, 0x00, sizeof(WSC_CONFIGURED_VALUE_2));
	Profile.WscConfigured = pMbss->wdev.WscControl.WscConfStatus;
	NdisZeroMemory(Profile.WscSsid, 32);
	NdisMoveMemory(Profile.WscSsid, pMbss->Ssid, pMbss->SsidLen);
	Profile.WscSsidLen = pMbss->SsidLen;
	Profile.WscAuthMode = WscGetAuthType(wdev->SecConfig.AKMMap);
	Profile.WscEncrypType = WscGetEncryType(wdev->SecConfig.PairwiseCipher);
	NdisZeroMemory(Profile.WscWPAKey, 64);

	if (Profile.WscEncrypType == 2) {
		Profile.DefaultKeyIdx = wdev->SecConfig.PairwiseKeyId + 1;
		{
			int i;

			for (i = 0; i < wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].KeyLen; i++) {
				snprintf((RTMP_STRING *) Profile.WscWPAKey, sizeof(Profile.WscWPAKey),
						 "%s%02x", Profile.WscWPAKey,
						 wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].Key[i]);
			}

			Profile.WscWPAKeyLen = wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].KeyLen;
		}
	} else if (Profile.WscEncrypType >= 4) {
		Profile.DefaultKeyIdx = 2;
		NdisMoveMemory(Profile.WscWPAKey, pMbss->wdev.WscControl.WpaPsk,
					   pMbss->wdev.WscControl.WpaPskLen);
		Profile.WscWPAKeyLen = pMbss->wdev.WscControl.WpaPskLen;
	} else
		Profile.DefaultKeyIdx = 1;

	wrq->u.data.length = sizeof(Profile);

	if (copy_to_user(wrq->u.data.pointer, &Profile, wrq->u.data.length))
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_to_user() fail\n");

	/*	msg = (RTMP_STRING *)kmalloc(sizeof(CHAR)*(2048), MEM_ALLOC_FLAG); */
	os_alloc_mem(pAd, (UCHAR **)&msg, msg_len);

	if (msg == NULL)
		return;

	memset(msg, 0x00, msg_len);
	snprintf(msg, msg_len, "%s", "\n");

	if (Profile.WscEncrypType == 1)
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s%-33s%-12s%-12s\n", "Configured", "SSID", "AuthMode", "EncrypType");
	else if (Profile.WscEncrypType == 2)
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s%-33s%-12s%-12s%-13s%-26s\n", "Configured", "SSID", "AuthMode", "EncrypType", "DefaultKeyID", "Key");
	else
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s%-33s%-12s%-12s%-64s\n", "Configured", "SSID", "AuthMode", "EncrypType", "Key");

	if (Profile.WscConfigured == 1)
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s", "No");
	else
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s", "Yes");

	snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-33s", Profile.WscSsid);

	if (IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap) || IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap))
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s", "WPAPSKWPA2PSK");
	else
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s", WscGetAuthTypeStr(Profile.WscAuthMode));

	if (IS_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher) || IS_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher))
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s", "TKIPAES");
	else
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s", WscGetEncryTypeStr(Profile.WscEncrypType));

	if (Profile.WscEncrypType == 1)
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%s\n", "");
	else if (Profile.WscEncrypType == 2) {
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-13d", Profile.DefaultKeyIdx);
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-26s\n", Profile.WscWPAKey);
	} else if (Profile.WscEncrypType >= 4)
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-64s\n", Profile.WscWPAKey);

#ifdef INF_AR9
	wrq->u.data.length = strlen(msg);
	copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
#endif/* INF_AR9 */
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s", msg);
	os_free_mem(msg);
}
#endif /* VENDOR_FEATURE6_SUPPORT */

/*
    ==========================================================================
    Description:
	Get WSC Profile
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
			1.) iwpriv ra0 get_wsc_profile
			3.) UI needs to prepare at least 4096bytes to get the results
    ==========================================================================
*/
VOID RTMPIoctlWscProfile(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	WSC_CONFIGURED_VALUE Profile;
	RTMP_STRING *msg;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	UINT msg_len = 2048;
	int ret, left_buf_size;

	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	memset(&Profile, 0x00, sizeof(WSC_CONFIGURED_VALUE));
	Profile.WscConfigured = pMbss->wdev.WscControl.WscConfStatus;
	NdisZeroMemory(Profile.WscSsid, 32 + 1);
	NdisMoveMemory(Profile.WscSsid, pMbss->Ssid, pMbss->SsidLen);
	Profile.WscSsid[pMbss->SsidLen] = '\0';
	Profile.WscAuthMode = WscGetAuthType(wdev->SecConfig.AKMMap);
	Profile.WscEncrypType = WscGetEncryType(wdev->SecConfig.PairwiseCipher);
	NdisZeroMemory(Profile.WscWPAKey, 64 + 1);

	if (Profile.WscEncrypType == 2) {
		Profile.DefaultKeyIdx = wdev->SecConfig.PairwiseKeyId + 1;
		{
			int i;

			for (i = 0; i < wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].KeyLen; i++) {
				left_buf_size = sizeof(Profile.WscWPAKey) - strlen(Profile.WscWPAKey);
				ret = snprintf((RTMP_STRING *) Profile.WscWPAKey + strlen(Profile.WscWPAKey),
					left_buf_size,
					"%02x", wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].Key[i]);
					if (os_snprintf_error(left_buf_size, ret)) {
						MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"final_name snprintf error!\n");
						return;
					}
			}

			Profile.WscWPAKey[(wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].KeyLen) * 2] = '\0';
		}
	} else if (Profile.WscEncrypType >= 4) {
		Profile.DefaultKeyIdx = 2;
		NdisMoveMemory(Profile.WscWPAKey, pMbss->wdev.WscControl.WpaPsk,
					   pMbss->wdev.WscControl.WpaPskLen);
		Profile.WscWPAKey[pMbss->wdev.WscControl.WpaPskLen] = '\0';
	} else
		Profile.DefaultKeyIdx = 1;

	wrq->u.data.length = sizeof(Profile);

	if (copy_to_user(wrq->u.data.pointer, &Profile, wrq->u.data.length))
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "copy_to_user() fail\n");

	os_alloc_mem(pAd, (UCHAR **)&msg, msg_len);

	if (msg == NULL)
		return;

	memset(msg, 0x00, msg_len);
	ret = snprintf(msg, msg_len, "%s", "\n");
	if (os_snprintf_error(msg_len, ret)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		goto done;
	}

	if (Profile.WscEncrypType == 1) {
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "%-12s%-33s%-12s%-12s\n", "Configured", "SSID", "AuthMode", "EncrypType");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	} else if (Profile.WscEncrypType == 2) {
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "%-12s%-33s%-12s%-12s%-13s%-26s\n", "Configured", "SSID", "AuthMode", "EncrypType",
				"DefaultKeyID", "Key");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	} else {
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "%-12s%-33s%-12s%-12s%-64s\n", "Configured", "SSID", "AuthMode", "EncrypType", "Key");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	}

	if (Profile.WscConfigured == 1) {
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "%-12s", "No");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	} else {
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "%-12s", "Yes");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	}

	left_buf_size = msg_len - strlen(msg);
	ret = snprintf(msg + strlen(msg), left_buf_size, "%-33s", Profile.WscSsid);
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
		goto done;
	}

	if (IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap) || IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap)) {
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "%-12s", "WPAPSKWPA2PSK");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	} else {
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "%-12s", WscGetAuthTypeStr(Profile.WscAuthMode));
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	}

	if (IS_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher) || IS_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher)) {
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "%-12s", "TKIPAES");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	} else {
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "%-12s", WscGetEncryTypeStr(Profile.WscEncrypType));
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	}

	if (Profile.WscEncrypType == 1) {
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "%s\n", "");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	} else if (Profile.WscEncrypType == 2) {
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "%-13d", Profile.DefaultKeyIdx);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}

		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "%-26s\n", Profile.WscWPAKey);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	} else if (Profile.WscEncrypType >= 4) {
		left_buf_size = msg_len - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "%-64s\n", Profile.WscWPAKey);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			goto done;
		}
	}

#ifdef INF_AR9
	wrq->u.data.length = strlen(msg);
	copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
#endif/* INF_AR9 */
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s", msg);
done:
	os_free_mem(msg);
}

#if defined(INF_AR9) || defined(BB_SOC)
#if defined(AR9_MAPI_SUPPORT) || defined(BB_SOC)

/*
    ==========================================================================
    Description:
	Get WSC Profile
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
			1.) iwpriv ra0 ar9_show get_wsc_profile
			3.) UI needs to prepare at least 4096bytes to get the results
    ==========================================================================
*/
VOID RTMPAR9IoctlWscProfile(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	WSC_CONFIGURED_VALUE Profile;
	RTMP_STRING *msg;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	    apidx = pObj->ioctl_if;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	UINT msg_len = 2048;

	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	memset(&Profile, 0x00, sizeof(WSC_CONFIGURED_VALUE));
	Profile.WscConfigured = pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfStatus;
	NdisZeroMemory(Profile.WscSsid, 32 + 1);
	NdisMoveMemory(Profile.WscSsid, pAd->ApCfg.MBSSID[apidx].Ssid,
				   pAd->ApCfg.MBSSID[apidx].SsidLen);
	Profile.WscSsid[pAd->ApCfg.MBSSID[apidx].SsidLen] = '\0';
	Profile.WscAuthMode = WscGetAuthType(wdev->SecConfig.AKMMap);
	Profile.WscEncrypType = WscGetEncryType(wdev->SecConfig.PairwiseCipher);
	NdisZeroMemory(Profile.WscWPAKey, 64 + 1);

	if (Profile.WscEncrypType == 2) {
		Profile.DefaultKeyIdx = wdev->SecConfig.PairwiseKeyId + 1;
		{
			int i;

			for (i = 0; i < wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].KeyLen; i++) {
				snprintf((RTMP_STRING *) Profile.WscWPAKey, sizeof(Profile.WscWPAKey),
						 "%s%02x", Profile.WscWPAKey,
						 wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].Key[i]);
			}

			Profile.WscWPAKey[(wdev->SecConfig.WepKey[Profile.DefaultKeyIdx].KeyLen) * 2] = '\0';
		}
	} else if (Profile.WscEncrypType >= 4) {
		Profile.DefaultKeyIdx = 2;
		NdisMoveMemory(Profile.WscWPAKey, pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WpaPsk,
					   pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WpaPskLen);
		Profile.WscWPAKey[pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WpaPskLen] = '\0';
	} else
		Profile.DefaultKeyIdx = 1;

	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR) * (2048));

	if (msg == NULL)
		return;

	memset(msg, 0x00, msg_len);
	snprintf(msg, msg_len, "%s", "\n");

	if (Profile.WscEncrypType == 1)
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s%-33s%-12s%-12s\n", "Configured", "SSID", "AuthMode", "EncrypType");
	else if (Profile.WscEncrypType == 2)
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s%-33s%-12s%-12s%-13s%-26s\n", "Configured", "SSID", "AuthMode", "EncrypType",
				"DefaultKeyID", "Key");
	else
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s%-33s%-12s%-12s%-64s\n", "Configured", "SSID", "AuthMode", "EncrypType", "Key");

	if (Profile.WscConfigured == 1)
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s", "No");
	else
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s", "Yes");

	snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-33s", Profile.WscSsid);

	if (IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap) || IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap))
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s", "WPAPSKWPA2PSK");
	else
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s", WscGetAuthTypeStr(Profile.WscAuthMode));

	if (IS_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher) || IS_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher))
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s", "TKIPAES");
	else
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-12s", WscGetEncryTypeStr(Profile.WscEncrypType));

	if (Profile.WscEncrypType == 1)
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%s\n", "");
	else if (Profile.WscEncrypType == 2) {
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-13d", Profile.DefaultKeyIdx);
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-26s\n", Profile.WscWPAKey);
	} else if (Profile.WscEncrypType >= 4)
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%-64s\n", Profile.WscWPAKey);

	wrq->u.data.length = strlen(msg);
	copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	{
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s", msg);
	}
	os_free_mem(msg);
}

VOID RTMPIoctlWscPINCode(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	RTMP_STRING	*msg;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;
	UCHAR		tempPIN[9] = {0};
	UINT		msg_len = 128;

	os_alloc_mem(NULL, (UCHAR **)&msg, msg_len);

	if (msg == NULL)
		return;

	memset(msg, 0x00, msg_len);
	snprintf(msg, msg_len, "%s", "\n");
	snprintf(msg + strlen(msg), msg_len - strlen(msg), "WSC_PINCode=");

	if (pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscEnrolleePinCode) {
		if (pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscEnrolleePinCodeLen == 8)
			snprintf((RTMP_STRING *) tempPIN, sizeof(tempPIN), "%08u", pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscEnrolleePinCode);
		else
			snprintf((RTMP_STRING *) tempPIN, sizeof(tempPIN), "%04u", pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscEnrolleePinCode);

		snprintf(msg + strlen(msg), msg_len - strlen(msg), "%s\n", tempPIN);
	}

	wrq->u.data.length = strlen(msg);
	copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	{
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s", msg);
	}
	os_free_mem(msg);
}

VOID RTMPIoctlWscStatus(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	RTMP_STRING	*msg;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;
	UINT		msg_len = 128;

	os_alloc_mem(NULL, (UCHAR **)&msg, msg_len);

	if (msg == NULL)
		return;

	memset(msg, 0x00, msg_len);
	snprintf(msg, msg_len, "%s", "\n");
	snprintf(msg + strlen(msg), msg_len - strlen(msg), "WSC_Status=");
	snprintf(msg + strlen(msg), msg_len - strlen(msg), "%s%d\n", pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscStatus);
	wrq->u.data.length = strlen(msg);
	copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	{
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s", msg);
	}
	os_free_mem(msg);
}

VOID RTMPIoctlGetWscDynInfo(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	char *msg;
	BSS_STRUCT *pMbss;
	INT apidx, configstate;
	UINT msg_len = pAd->ApCfg.BssidNum * (14 * 128);

	os_alloc_mem(NULL, (UCHAR **)&msg, msg_len);

	if (msg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Alloc memory failed\n");
		return;
	}

	memset(msg, 0, msg_len);
	snprintf(msg, msg_len, "%s", "\n");

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		pMbss =  &pAd->ApCfg.MBSSID[apidx];

		if (pMbss->wdev.WscControl.WscConfStatus == WSC_SCSTATE_UNCONFIGURED)
			configstate = 0;
		else
			configstate = 1;

		snprintf(msg + strlen(msg), msg_len - strlen(msg), "ra%d\n", apidx);
#ifdef BB_SOC
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "DeviceName = %s\n", (pMbss->wdev.WscControl.RegData.SelfInfo.DeviceName));
#endif
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "UUID = %s\n", (pMbss->wdev.WscControl.Wsc_Uuid_Str));
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "wpsVersion = 0x%x\n", WSC_VERSION);
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "setuoLockedState = %d\n", 0);
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "configstate = %d\n", configstate);
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "lastConfigError = %d\n", 0);
	}

	wrq->u.data.length = strlen(msg);

	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s", msg);

	os_free_mem(msg);
}

VOID RTMPIoctlGetWscRegsDynInfo(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq)
{
	char *msg;
	BSS_STRUCT *pMbss;
	INT apidx;
	UINT msg_len = pAd->ApCfg.BssidNum * (14 * 128);

	os_alloc_mem(NULL, (UCHAR **)&msg, msg_len);

	if (msg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Alloc memory failed\n");
		return;
	}

	memset(msg, 0, msg_len);
	snprintf(msg, msg_len, "%s", "\n");

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		pMbss =  &pAd->ApCfg.MBSSID[apidx];
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "ra%d\n", apidx);
#ifdef BB_SOC
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "DeviceName = %s\n", (pMbss->wdev.WscControl.RegData.SelfInfo.DeviceName));
#endif
		snprintf(msg + strlen(msg), msg_len - strlen(msg), "UUID_R = %s\n", (pMbss->wdev.WscControl.RegData.PeerInfo.Uuid));
	}

	wrq->u.data.length = strlen(msg);

	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "%s", msg);

	os_free_mem(msg);
}
#endif /* defined(AR9_MAPI_SUPPORT) || defined(BB_SOC) */
#endif /* defined(INF_AR9) || defined(BB_SOC) */

BOOLEAN WscCheckEnrolleeNonceFromUpnp(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_STRING *pData,
	IN  USHORT			Length,
	IN  PWSC_CTRL       pWscControl)
{
	USHORT	WscType, WscLen;
	USHORT  WscId = WSC_ID_ENROLLEE_NONCE;

	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "check Enrollee Nonce\n");

	/* We have to look for WSC_IE_MSG_TYPE to classify M2 ~ M8, the remain size must large than 4 */
	while (Length > 4) {
		WSC_IE	TLV_Recv;
		char ZeroNonce[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

		memcpy((UINT8 *)&TLV_Recv, pData, 4);
		WscType = be2cpu16(TLV_Recv.Type);
		WscLen  = be2cpu16(TLV_Recv.Length);
		if (Length < WscLen + 4) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "unexpected WSC IE Length(%u)\n", WscLen);
			break;
		}
		pData  += 4;
		Length -= 4;

		if (WscType == WscId) {
			if (RTMPCompareMemory(pWscControl->RegData.SelfNonce, pData, 16) == 0) {
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Nonce match!!\n");
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<----- WscCheckNonce\n");
				return TRUE;
			} else if (NdisEqualMemory(pData, ZeroNonce, 16)) {
				/* Intel external registrar will send WSC_NACK with enrollee nonce */
				/* "10 1A 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00" */
				/* when AP is configured and user selects not to configure AP. */
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Zero Enrollee Nonce!!\n");
				MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<----- WscCheckNonce\n");
				return TRUE;
			}
		}

		/* Offset to net WSC Ie */
		pData  += WscLen;
		Length -= WscLen;
	}

	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Nonce mismatch!!\n");
	return FALSE;
}

UCHAR	WscRxMsgTypeFromUpnp(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  RTMP_STRING *pData,
	IN	USHORT				Length)
{
	USHORT WscType, WscLen;
	{   /* Eap-Esp(Messages) */
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
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR, "unexpected WSC IE Length(%u)\n", WscLen);
				break;
			}

			if (ntohs(WscType) == WSC_ID_MSG_TYPE)
				return (*(pData + 4));	/* Found the message type */

			pData  += (WscLen + 4);
			Length -= (WscLen + 4);
		}
	}
out:
	return  WSC_MSG_UNKNOWN;
}

VOID RTMPIoctlSetWSCOOB(
	IN PRTMP_ADAPTER pAd)
{
	char        *pTempSsid = NULL;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;
	int ret;

	pObj->pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "IF(apcli%d) Ap Client doesn't need this command.\n", apidx);
		return;
	}

#endif /* APCLI_SUPPORT */
#ifdef WSC_V2_SUPPORT
    Set_WscSetupLock_Proc(pAd, "0");
#endif
	Set_AP_WscConfStatus_Proc(pAd, "1");
	Set_SecAuthMode_Proc(pAd, "WPAPSK");
	Set_SecEncrypType_Proc(pAd, "TKIP");
	pTempSsid = vmalloc(33);

	if (pTempSsid) {
		memset(pTempSsid, 0, 33);
		ret = snprintf(pTempSsid, 33, "RalinkInitialAP%02X%02X%02X",
				 pAd->ApCfg.MBSSID[apidx].wdev.bssid[3],
				 pAd->ApCfg.MBSSID[apidx].wdev.bssid[4],
				 pAd->ApCfg.MBSSID[apidx].wdev.bssid[5]);
		if (os_snprintf_error(33, ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
			vfree(pTempSsid);
			return;
		}

		Set_AP_SSID_Proc(pAd, pTempSsid);
		vfree(pTempSsid);
	}

	Set_SecWPAPSK_Proc(pAd, "RalinkInitialAPxx1234");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d) \n", apidx);
}

/*
	==========================================================================
	Description:
	Set Wsc Security Mode
	0 : WPA2PSK AES
	1 : WPA2PSK TKIP
	2 : WPAPSK AES
	3 : WPAPSK TKIP
	Return:
	TRUE if all parameters are OK, FALSE otherwise
	==========================================================================
*/
INT	Set_AP_WscSecurityMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		apidx = pObj->ioctl_if;

	if (strcmp(arg, "0") == 0)
		pAd->ApCfg.MBSSID[apidx].wdev.WscSecurityMode = WPA2PSKAES;
	else if (strcmp(arg, "1") == 0)
		pAd->ApCfg.MBSSID[apidx].wdev.WscSecurityMode = WPA2PSKTKIP;
	else if (strcmp(arg, "2") == 0)
		pAd->ApCfg.MBSSID[apidx].wdev.WscSecurityMode = WPAPSKAES;
	else if (strcmp(arg, "3") == 0)
		pAd->ApCfg.MBSSID[apidx].wdev.WscSecurityMode = WPAPSKTKIP;
	else
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d) (WscSecurityMode=%d)\n",
			 apidx, pAd->ApCfg.MBSSID[apidx].wdev.WscSecurityMode);
	return TRUE;
}

INT Set_AP_WscMultiByteCheck_Proc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE		pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR			apidx = pObj->ioctl_if;
	BOOLEAN			bEnable = FALSE;
	PWSC_CTRL		pWpsCtrl = NULL;
	BOOLEAN			bFromApCli = FALSE;
#ifdef APCLI_SUPPORT

	if (pObj->ioctl_if_type == INT_APCLI) {
		bFromApCli = TRUE;
		pWpsCtrl = &pAd->StaCfg[apidx].wdev.WscControl;
		MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"IF(apcli%d) This command is from apcli interface now.\n", apidx);
	} else
#endif /* APCLI_SUPPORT */
	{
		bFromApCli = FALSE;
		pWpsCtrl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"IF(ra%d) This command is from ra interface now.\n", apidx);
	}

	if (strcmp(arg, "0") == 0)
		bEnable = FALSE;
	else if (strcmp(arg, "1") == 0)
		bEnable = TRUE;
	else
		return FALSE;

	if (pWpsCtrl->bCheckMultiByte != bEnable)
		pWpsCtrl->bCheckMultiByte = bEnable;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d) (bCheckMultiByte=%d)\n",
			 apidx, pWpsCtrl->bCheckMultiByte);
	return TRUE;
}

INT	Set_WscVersion_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	version = (UCHAR)os_str_tol(arg, 0, 16);
	UINT	ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(version=%x)\n", version);
	pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.RegData.SelfInfo.Version = version;
	return TRUE;
}

#ifdef VENDOR_FEATURE6_SUPPORT
INT	Set_WscUUID_STR_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT		ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}

	if (strlen(arg) ==	(UUID_LEN_STR - 1)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[%d]::(arg=%s)\n", ifIndex, arg);
		pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.Wsc_Uuid_Str[UUID_LEN_STR - 1] = 0;
		NdisMoveMemory(&pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.Wsc_Uuid_Str[0], arg, strlen(arg));
		return TRUE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ERROR [%d]::(arg=%s), Leng(%d) is incorrect!\n", pObj->ioctl_if, arg, (int)strlen(arg));
		return FALSE;
	}
}

INT	Set_WscUUID_HEX_E_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT		ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}

	if (strlen(arg) ==	(UUID_LEN_HEX * 2)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[%d]::(arg=%s)\n", ifIndex, arg);
		AtoH(arg, &pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.Wsc_Uuid_E[0], UUID_LEN_HEX);
		hex_dump("Set_WscUUID_HEX_E_Proc OK:",
			&pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.Wsc_Uuid_E[0], UUID_LEN_HEX);
		return TRUE;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ERROR [%d]::(arg=%s), Leng(%d) is incorrect!\n", pObj->ioctl_if, arg, (int)strlen(arg));
		return FALSE;
	}
}
#endif /* VENDOR_FEATURE6_SUPPORT */

#ifdef WSC_V2_SUPPORT
INT	Set_WscFragment_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	bool_flag = (UCHAR)os_str_tol(arg, 0, 16);
	UINT	ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(bool_flag=%d)\n", bool_flag);
	pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.bWscFragment = bool_flag;
	return TRUE;
}

INT	Set_WscFragmentSize_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	USHORT		WscFragSize = (USHORT)os_str_tol(arg, 0, 10);
	UINT		ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(WscFragSize=%d)\n", WscFragSize);

	if ((WscFragSize >= 128) && (WscFragSize <= 300))
		pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.WscFragSize = WscFragSize;

	return TRUE;
}

INT	Set_WscSetupLock_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		bEnable = (UCHAR)os_str_tol(arg, 0, 10);
	PWSC_CTRL	pWscControl;
	UINT		ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}

	pWscControl = &pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl;

	if (bEnable == 0) {
		BOOLEAN bCancelled = FALSE;

		pWscControl->PinAttackCount = 0;

		if (pWscControl->WscSetupLockTimerRunning)
			RTMPCancelTimer(&pWscControl->WscSetupLockTimer, &bCancelled);

		WscSetupLockTimeout(NULL, pWscControl, NULL, NULL);
	} else {
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

		pWscControl->bSetupLock = TRUE;
		WscBuildBeaconIE(pAd,
						 pWscControl->WscConfStatus,
						 FALSE,
						 0,
						 0,
						 ifIndex,
						 NULL,
						 0,
						 AP_MODE);
		WscBuildProbeRespIE(pAd,
							WSC_MSGTYPE_AP_WLAN_MGR,
							pWscControl->WscConfStatus,
							FALSE,
							0,
							0,
							ifIndex,
							NULL,
							0,
							AP_MODE);
		UpdateBeaconHandler(
			pAd,
			wdev,
			BCN_UPDATE_IE_CHG);
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(bSetupLock=%d)\n",
			 pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.bSetupLock);
	return TRUE;
}

INT	Set_WscV2Support_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR bEnable = (UCHAR)os_str_tol(arg, 0, 10);
	PWSC_CTRL pWscControl;
	UINT	ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}

	pWscControl = &pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl;

	if (bEnable == 0)
		pWscControl->WscV2Info.bEnableWpsV2 = FALSE;
	else
		pWscControl->WscV2Info.bEnableWpsV2 = TRUE;

	if (pWscControl->WscV2Info.bEnableWpsV2) {
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		/*
			WPS V2 doesn't support Chiper WEP and TKIP.
		*/
		struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

		if (IS_CIPHER_WEP_TKIP_ONLY(pSecConfig->PairwiseCipher)
			|| (pAd->ApCfg.MBSSID[ifIndex].bHideSsid))
			WscOnOff(pAd, wdev->func_idx, TRUE);
		else
			WscOnOff(pAd, wdev->func_idx, FALSE);

		UpdateBeaconHandler(
			pAd,
			wdev,
			BCN_UPDATE_IE_CHG);
	} else
		WscInit(pAd, FALSE, ifIndex);

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(bEnableWpsV2=%d)\n",
			 pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.WscV2Info.bEnableWpsV2);
	return TRUE;
}

INT	Set_WscVersion2_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	version = (UCHAR)os_str_tol(arg, 0, 16);
	UINT	ifIndex = pObj->ioctl_if;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(version=%x)\n", version);

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}

	if (version >= 0x20)
		pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.RegData.SelfInfo.Version2 = version;
	else
		return FALSE;

	return TRUE;
}

INT	Set_WscExtraTlvTag_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	USHORT		new_tag = (USHORT)os_str_tol(arg, 0, 16);
	UINT		ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}

	pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.WscV2Info.ExtraTlv.TlvTag = new_tag;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(new_tag=0x%04X)\n", new_tag);
	return TRUE;
}

INT	Set_WscExtraTlvType_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		type = (UCHAR)os_str_tol(arg, 0, 10);
	UINT		ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}

	pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.WscV2Info.ExtraTlv.TlvType = type;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(type=%d)\n", type);
	return TRUE;
}

INT	Set_WscExtraTlvData_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE		pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT			DataLen = (UINT)strlen(arg);
	PWSC_TLV		pWscTLV;
	INT				i;
	UINT			ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}

	pWscTLV = &pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.WscV2Info.ExtraTlv;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(DataLen = %d)\n", DataLen);

	if ((DataLen != 0) && (pWscTLV->TlvType == TLV_HEX)) {
		for (i = 0; i < DataLen; i++) {
			if (!isxdigit(*(arg + i)))
				return FALSE;  /*Not Hex value; */
		}
	}

	if (pWscTLV->pTlvData) {
		os_free_mem(pWscTLV->pTlvData);
		pWscTLV->pTlvData = NULL;
	}

	if (DataLen == 0)
		return TRUE;

	pWscTLV->TlvLen = 0;
	os_alloc_mem(NULL, &pWscTLV->pTlvData, DataLen + 1);
	if (pWscTLV->pTlvData == NULL)
		return FALSE;

	pWscTLV->pTlvData[DataLen] = 0;

	if (pWscTLV->pTlvData) {
		if (pWscTLV->TlvType == TLV_ASCII) {
			NdisMoveMemory(pWscTLV->pTlvData, arg, DataLen);
			pWscTLV->TlvLen = DataLen;
		} else {
			pWscTLV->TlvLen = DataLen / 2;
			AtoH(arg, pWscTLV->pTlvData, pWscTLV->TlvLen);
		}

		return TRUE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "os_alloc_mem fail\n");
	return FALSE;
}

INT	Set_WscMaxPinAttack_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		MaxPinAttack = (UCHAR)os_str_tol(arg, 0, 10);
	UINT		ifIndex = pObj->ioctl_if;;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(MaxPinAttack=%d)\n", MaxPinAttack);
	pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.MaxPinAttack = MaxPinAttack;
	return TRUE;
}

INT	Set_WscSetupLockTime_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT		SetupLockTime = (UINT)os_str_tol(arg, 0, 10);
	UINT		ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(SetupLockTime=%d)\n",
			 SetupLockTime);
	pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.SetupLockTime = SetupLockTime;
	return TRUE;
}

#endif /* WSC_V2_SUPPORT */

INT	Set_WscAutoTriggerDisable_Proc(
	IN	RTMP_ADAPTER	 *pAd,
	IN	RTMP_STRING		*arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR bEnable = (UCHAR)os_str_tol(arg, 0, 10);
	PWSC_CTRL pWscCtrl;
	UINT	ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}

	pWscCtrl = &pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl;

	if (bEnable == 0)
		pWscCtrl->bWscAutoTriggerDisable = FALSE;
	else
		pWscCtrl->bWscAutoTriggerDisable = TRUE;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(bWscAutoTriggerDisable=%d)\n",
			 pWscCtrl->bWscAutoTriggerDisable);
	return TRUE;
}

#endif /* WSC_AP_SUPPORT */


#ifdef IAPP_SUPPORT
INT	Set_IappPID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	unsigned long IappPid;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;

	IappPid = os_str_tol(arg, 0, 10);
	RTMP_GET_OS_PID(pObj->IappPid, IappPid);
	pObj->IappPid_nr = IappPid;
	/*	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, "pObj->IappPid = %d", GET_PID_NUMBER(pObj->IappPid)); */
	return TRUE;
} /* End of Set_IappPID_Proc */
#endif /* IAPP_SUPPORT */

INT	Set_DisConnectSta_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR					macAddr[MAC_ADDR_LEN];
	RTMP_STRING *value;
	INT						i;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (strlen(arg) != 17) /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid */

		AtoH(value, (UCHAR *)&macAddr[i++], 1);
	}

	if (NdisEqualMemory(&macAddr[0], &BROADCAST_ADDR[0], MAC_ADDR_LEN)) {
		Set_DisConnectAllSta_Proc(pAd, "2");
		return TRUE;
	}

	pEntry = MacTableLookup(pAd, macAddr);

	if (pEntry) {
#ifdef MAP_R2
		if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
			pEntry->DisconnectReason = REASON_DISASSOC_STA_LEAVING;
#endif
		MlmeDeAuthAction(pAd, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
		/*		MacTableDeleteEntry(pAd, pEntry->wcid, Addr); */
	}

	return TRUE;
}

INT Set_DisConnectAllSta_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	INT i;
	MAC_TABLE_ENTRY *pEntry;
#ifdef DOT11W_PMF_SUPPORT
	CHAR value = os_str_tol(arg, 0, 10);

	if (value == 2) {
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "[PMF] apidx=%d\n", pObj->ioctl_if);
		APMlmeKickOutAllSta(pAd, pObj->ioctl_if, REASON_DEAUTH_STA_LEAVING);

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pEntry = &pAd->MacTab.Content[i];

			if (IS_ENTRY_CLIENT(pEntry)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[PMF] MacTableDeleteEntry %x:%x:%x:%x:%x:%x\n",
						 PRINT_MAC(pEntry->Addr));

#ifdef MAP_R2
				if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
					wapp_handle_sta_disassoc(pAd, i, REASON_DEAUTH_STA_LEAVING);
#endif
				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			}
		}
	} else
#endif /* DOT11W_PMF_SUPPORT */
	{
		UCHAR *pOutBuffer = NULL;
		NDIS_STATUS NStatus;
		HEADER_802_11 DeAuthHdr;
		USHORT Reason;
		ULONG FrameLen = 0;

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pEntry = &pAd->MacTab.Content[i];

			if (IS_ENTRY_CLIENT(pEntry)) {
				pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
					/* Before reset MacTable, send disassociation packet to client.*/
					if (pEntry->Sst == SST_ASSOC) {
						/*  send out a De-authentication request frame*/
						NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

						if (NStatus != NDIS_STATUS_SUCCESS) {
							MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
									 " MlmeAllocateMemory fail  ..\n");
							return FALSE;
						}
#ifdef MAP_R2
						if (IS_MAP_R2_ENABLE(pAd))
							wapp_handle_sta_disassoc(pAd, i, REASON_DEAUTH_STA_LEAVING);
#endif
						Reason = REASON_NO_LONGER_VALID;
						MTWF_DBG(pAd, DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN,
								 "Send DeAuth (Reason=%d) to "MACSTR"\n",
								  Reason, MAC2STR(pEntry->Addr));
						MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->Addr,
										 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.if_addr,
										 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid);
						MakeOutgoingFrame(pOutBuffer, &FrameLen,
										  sizeof(HEADER_802_11), &DeAuthHdr,
										  2, &Reason,
										  END_OF_ARGS);
						MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
						MlmeFreeMemory(pOutBuffer);
						RtmpusecDelay(5000);
					}
				}
#endif /* CONFIG_AP_SUPPORT */
				/* Delete a entry via WCID */
				MacTableDeleteEntry(pAd, i, pEntry->Addr);
			}
		}
	}

	return TRUE;
}

#ifdef DOT1X_SUPPORT
/*
    ==========================================================================
    Description:
	Set IEEE8021X.
	This parameter is 1 when 802.1x-wep turn on, otherwise 0
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_IEEE8021X_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	if (os_str_tol(arg, 0, 10) != 0)	/*Enable*/
		pSecConfig->IEEE8021X = TRUE;
	else /*Disable*/
		pSecConfig->IEEE8021X = FALSE;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) IEEE8021X=%d\n",
			 INF_MBSSID_DEV_NAME, apidx, pSecConfig->IEEE8021X);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set pre-authentication enable or disable when WPA/WPA2 turn on
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_PreAuth_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	if (os_str_tol(arg, 0, 10) != 0) /*Enable*/
		pSecConfig->PreAuth = TRUE;
	else /*Disable*/
		pSecConfig->PreAuth = FALSE;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(%s%d) PreAuth=%d\n",
			 INF_MBSSID_DEV_NAME, apidx, pSecConfig->PreAuth);
	return TRUE;
}

INT	Set_OwnIPAddr_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;
	SetWdevOwnIPAddr(pSecConfig, arg);
	return TRUE;
}

INT	Set_EAPIfName_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	if (strlen(arg) > 0 && strlen(arg) <= IFNAMSIZ) {
		pSecConfig->EAPifname_len = strlen(arg);
		NdisMoveMemory(pSecConfig->EAPifname, arg, strlen(arg));
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "EAPifname=%s, len=%d\n",
				 pSecConfig->EAPifname, pSecConfig->EAPifname_len);
	}

	return TRUE;
}

INT	Set_PreAuthIfName_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	if (strlen(arg) > 0 && strlen(arg) <= IFNAMSIZ) {
		pSecConfig->PreAuthifname_len = strlen(arg);
		NdisMoveMemory(pSecConfig->PreAuthifname, arg, strlen(arg));
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PreAuthifname=%s, len=%d\n",
				 pSecConfig->PreAuthifname, pSecConfig->PreAuthifname_len);
	}

	return TRUE;
}

INT	Set_RADIUS_Server_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	UINT32 ip_addr;
	INT count;
	RTMP_STRING *macptr;
	INT srv_cnt = 0;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	for (count = 0, macptr = rstrtok(arg, ";"); (macptr &&
			count < MAX_RADIUS_SRV_NUM); macptr = rstrtok(NULL, ";"), count++) {
		if (rtinet_aton(macptr, &ip_addr)) {
			PRADIUS_SRV_INFO pSrvInfo = &pSecConfig->radius_srv_info[srv_cnt];

			pSrvInfo->radius_ip = ip_addr;
			srv_cnt++;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d), radius_ip(seq-%d)=%s\n",
					 apidx, srv_cnt, arg);
		}

		if (srv_cnt > 0)
			pSecConfig->radius_srv_num = srv_cnt;
	}

	return TRUE;
}

INT	Set_RADIUS_Port_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	RTMP_STRING *macptr;
	INT count;
	INT srv_cnt = 0;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	for (count = 0, macptr = rstrtok(arg, ";"); (macptr &&
			count < MAX_RADIUS_SRV_NUM); macptr = rstrtok(NULL, ";"), count++) {
		if (srv_cnt < pSecConfig->radius_srv_num) {
			PRADIUS_SRV_INFO pSrvInfo = &pSecConfig->radius_srv_info[srv_cnt];

			pSrvInfo->radius_port = (UINT32) os_str_tol(macptr, 0, 10);
			srv_cnt++;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d), radius_port(seq-%d)=%d\n",
					 apidx, srv_cnt, pSrvInfo->radius_port);
		}
	}

	return TRUE;
}

INT	Set_RADIUS_Key_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	RTMP_STRING *macptr;
	INT count;
	INT srv_cnt = 0;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	pSecConfig = &wdev->SecConfig;

	for (count = 0, macptr = rstrtok(arg, ";"); (macptr &&
			count < MAX_RADIUS_SRV_NUM); macptr = rstrtok(NULL, ";"), count++) {
		if (strlen(macptr) > 0 && strlen(macptr) < 65 && srv_cnt < pSecConfig->radius_srv_num) {
			PRADIUS_SRV_INFO pSrvInfo = &pSecConfig->radius_srv_info[srv_cnt];

			pSrvInfo->radius_key_len = strlen(macptr);
			NdisMoveMemory(pSrvInfo->radius_key, macptr, pSrvInfo->radius_key_len);
			srv_cnt++;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d), radius_key(seq-%d)=%s, len=%d\n",
					 apidx, srv_cnt, pSrvInfo->radius_key, pSrvInfo->radius_key_len);
		}
	}

	return TRUE;
}

INT	Set_DeletePMKID_Proc(RTMP_ADAPTER *pAd,
						 RTMP_STRING *arg)  /* for testing sending deauth frame if PMKID not found */
{
	UCHAR apidx = os_str_tol(arg, 0, 10);
	INT32 i = 0;

	for (i = 0; i < MAX_PMKID_COUNT; i++)
		RTMPDeletePMKIDCache(&pAd->ApCfg.PMKIDCache, apidx, i);

	return TRUE;
}

INT	Set_DumpPMKID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR imput = os_str_tol(arg, 0, 10);
	INT32 i = 0;

	if (imput == 1) {
		for (i = 0; i < MAX_PMKID_COUNT; i++) {
			PAP_BSSID_INFO pBssInfo = &pAd->ApCfg.PMKIDCache.BSSIDInfo[i];

			if (pBssInfo->Valid) {
				MTWF_PRINT("Cacheidx = %d, Mbssidx = %d, Mac = "MACSTR"\n",
						  i, pBssInfo->Mbssidx, MAC2STR(pBssInfo->MAC));
			}
		}
	}

	return TRUE;
}

#ifdef RADIUS_MAC_ACL_SUPPORT
INT show_RADIUS_acl_cache(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;
	PRT_802_11_RADIUS_ACL_ENTRY pCacheEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;
	PLIST_HEADER pListHeader = NULL;

	pListHeader = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.cacheList;

	if (pListHeader->size != 0) {
		MTWF_PRINT("IF(ra%d), Radius ACL Cache List:\n", apidx);
		pListEntry = pListHeader->pHead;
		pCacheEntry = (PRT_802_11_RADIUS_ACL_ENTRY)pListEntry;

		while (pCacheEntry != NULL) {
			MTWF_PRINT(MACSTR" --> %d\n", MAC2STR(pCacheEntry->Addr), pCacheEntry->result);
			pListEntry = pListEntry->pNext;
			pCacheEntry = (PRT_802_11_RADIUS_ACL_ENTRY)pListEntry;
		}
	} else
		MTWF_PRINT("IF(ra%d), Radius ACL Cache empty\n", apidx);

	return TRUE;
}

INT Set_RADIUS_CacheTimeout_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;
	CHAR        val  = os_str_tol(arg, 0, 10);

	if (val > 0) {
		pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCacheTimeout = val;
		MTWF_PRINT("ra[%d] Radius Cache Timeout: %d\n", apidx, val);
		return TRUE;
	}

	return FALSE;
}

INT Set_RADIUS_MacAuth_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR       apidx = pObj->ioctl_if;
	CHAR        val  = os_str_tol(arg, 0, 10);

	if (val == 0)
		pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.Policy = RADIUS_MAC_AUTH_DISABLE;
	else if (val == 1)
		pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.Policy = RADIUS_MAC_AUTH_ENABLE;
	else
		return FALSE;

	MTWF_PRINT("IF(ra%d), Radius MAC Auth: %d\n", apidx,
			 pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.RadiusMacAuthCache.Policy);
	return TRUE;
}
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* DOT1X_SUPPORT */

#ifdef UAPSD_SUPPORT
INT Set_UAPSD_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR IdMbss = pObj->ioctl_if;

	if (os_str_tol(arg, 0, 10) != 0)
		pAd->ApCfg.MBSSID[IdMbss].wdev.UapsdInfo.bAPSDCapable = TRUE;
	else
		pAd->ApCfg.MBSSID[IdMbss].wdev.UapsdInfo.bAPSDCapable = FALSE;

	return TRUE;
} /* End of Set_UAPSD_Proc */
#endif /* UAPSD_SUPPORT */

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
INT set_mgm_rate_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	HTTRANSMIT_SETTING *transmit;
	/*UCHAR cfg_ht_bw;*/
	struct wifi_dev *wdev = NULL;
	BOOLEAN	status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	UINT32 ratetype = 0, phymode = 0, mcs = 0;
	INT i4Recv = 0;
	UCHAR bandIdx;


	if ((pObj->ioctl_if_type != INT_MBSSID) && (pObj->ioctl_if_type != INT_MAIN)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "Do nothing! This device interface is NOT AP mode!\n");
		return FALSE;
	}

	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "Invalid device interface!\n");
		return FALSE;
	}

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	bandIdx = HcGetBandByWdev(wdev);

	if (arg) {
		i4Recv = sscanf(arg, "%d-%d-%d", &(ratetype), &(phymode), &(mcs));
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "ratetype %d,phymode %d,mcs %d\n",
				 ratetype, phymode, mcs);

		if (i4Recv != 3) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Format Error!\n");
			return FALSE;
		}

		if (WMODE_CAP_5G(wdev->PhyMode)) {
			if (phymode == EAP_CCK || phymode == MCAST_CCK) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "We could not set CCK mode for Mgm in 5G band!\n");
				return FALSE;
			}
		}

		if (mcs > 15) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 "Mcs must be in range of 0 to 15\n");
			return FALSE;
		}

		if (ratetype == BCN_TYPE)
			transmit = &wdev->eap.bcnphymode;
		else if (ratetype == MGM_TYPE)
			transmit = &wdev->eap.mgmphymode;
		else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					"Error ratetype: %d\n", ratetype);
			return FALSE;
		}

		switch (phymode) {
		case EAP_RATE_DISABLE: /* disable */
			NdisMoveMemory(transmit, &wdev->rate.MlmeTransmit, sizeof(HTTRANSMIT_SETTING));
			transmit->field.BW =  BW_20;

			break;
		case EAP_CCK:	/* CCK */
			transmit->field.MODE = MODE_CCK;
			transmit->field.BW =  BW_20;
			if ((transmit->field.MCS > 11) || (transmit->field.MCS > 3 && transmit->field.MCS < 8))
				transmit->field.MCS = 3;
			break;
		case EAP_OFDM:	/* OFDM */
			transmit->field.MODE = MODE_OFDM;
			transmit->field.BW =  BW_20;
			if (transmit->field.MCS > 7)
				transmit->field.MCS = 7;

			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 "unknown PhyMode %d.\n", phymode);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 "0:Disable, 1:CCK, 2:OFDM\n");
			status = FALSE;
			break;
		}

		if (!status)
			return FALSE;

		switch (transmit->field.MODE) {
		case MODE_CCK:
			if ((mcs <= 3) || (mcs >= 8 && mcs <= 11))
				transmit->field.MCS = mcs;
			else {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "MCS must in range of 0 ~ 3 and 8 ~ 11 for CCK Mode.\n");
				status = FALSE;
			}

			break;
		case MODE_OFDM:
			if (mcs > 7) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						 "MCS must in range from 0 to 7 for OFDM Mode.\n");
				status = FALSE;
			} else
				transmit->field.MCS = mcs;

			break;
		default:
			transmit->field.MCS = mcs;
			break;
		}



	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, "Format Error!\n");
		return FALSE;
	}

	if (ratetype == BCN_TYPE) {
		wdev->eap.eap_bcnrate_en = TRUE;
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_DISABLE_TX);
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_ENABLE_TX);
		pAd->SkuBcnTxPwr[bandIdx] = 0;
	} else if (ratetype == MGM_TYPE) {
		wdev->eap.eap_mgmrate_en = TRUE;
		MlmeUpdateTxRates(pAd, FALSE, (UCHAR)pObj->ioctl_if);
		pAd->SkuMgmtTxPwr[bandIdx] = 0;
	}

	return status;
}


INT show_mgmrate(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	HTTRANSMIT_SETTING *transmit;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if ((pObj->ioctl_if_type != INT_MBSSID) && (pObj->ioctl_if_type != INT_MAIN)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
		"Do nothing! This device interface is NOT AP mode!\n");
		return FALSE;
	}
	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
		"Invalid device interface!\n");
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	transmit = &wdev->eap.mgmphymode;
	MTWF_PRINT("Mgm PhyMode = %d\n", transmit->field.MODE);
	MTWF_PRINT("Mgm Mcs = %d\n", transmit->field.MCS);
	MTWF_PRINT("Mgm BW = %d\n", transmit->field.BW);
	return TRUE;
}

INT show_bcnrate(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	HTTRANSMIT_SETTING *transmit;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if ((pObj->ioctl_if_type != INT_MBSSID) && (pObj->ioctl_if_type != INT_MAIN)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
		"Do nothing! This device interface is NOT AP mode!\n");
		return FALSE;
	}
	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
		"Invalid device interface!\n");
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	transmit = &wdev->eap.bcnphymode;
	MTWF_PRINT("Bcn PhyMode = %d\n", transmit->field.MODE);
	MTWF_PRINT("Bcn Mcs = %d\n", transmit->field.MCS);
	MTWF_PRINT("Bcn BW = %d\n", transmit->field.BW);
	return TRUE;
}


INT set_suprateset_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR rate[] = { 0x82, 0x84, 0x8b, 0x96, 0x8C, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6c};
	INT supratesetbitmap = 0, i = 0;
	MAC_TABLE_ENTRY *mac_entry;
	RA_ENTRY_INFO_T *raentry;

	struct legacy_rate *eap_legacy_rate = &wdev->eap.eap_legacy_rate;

	supratesetbitmap = (INT) os_str_tol(arg, 0, 10);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
		"SupRateSetBitmap %x\n", supratesetbitmap);
	if (supratesetbitmap > 4095) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"error ForceRateSetBitmap(%04X) > 4096\n", supratesetbitmap);
		return FALSE;
	}

	if (!wdev)
		return FALSE;

	wdev->eap.eap_suprate_en = TRUE;
	eap_legacy_rate->sup_rate_len = 0;
	eap_legacy_rate->ext_rate_len = 0;

	for (i = 0; i < MAX_LEN_OF_SUPPORTED_RATES; i++) {
		if (supratesetbitmap & (1 << i)) {
			if (WMODE_EQUAL(wdev->PhyMode, WMODE_B) && WMODE_CAP_2G(wdev->PhyMode)) {
				eap_legacy_rate->sup_rate[eap_legacy_rate->sup_rate_len] = rate[i];
				eap_legacy_rate->sup_rate_len++;
				wdev->eap.eapsupportcckmcs |= (1 << i);
				wdev->eap.eapsupportratemode |= SUPPORT_CCK_MODE;
			} else if (WMODE_CAP_5G(wdev->PhyMode) && (i > 3)) {
				eap_legacy_rate->sup_rate[eap_legacy_rate->sup_rate_len] = rate[i];
				eap_legacy_rate->sup_rate_len++;
				wdev->eap.eapsupportofdmmcs |= (1 << (i - 4));
				wdev->eap.eapsupportratemode |= SUPPORT_OFDM_MODE;
			} else {
				if ((i < 4) || (i == 5) || (i == 7) || (i == 9) || (i == 11)) {
					eap_legacy_rate->sup_rate[eap_legacy_rate->sup_rate_len] = rate[i];
					eap_legacy_rate->sup_rate_len++;
					if (i < 4) {
						wdev->eap.eapsupportcckmcs |= (1 << i);
						wdev->eap.eapsupportratemode |= SUPPORT_CCK_MODE;
					} else {
						wdev->eap.eapsupportofdmmcs |= (1 << (i - 4));
						wdev->eap.eapsupportratemode |= SUPPORT_OFDM_MODE;
					}
				} else {
					eap_legacy_rate->ext_rate[eap_legacy_rate->ext_rate_len] = rate[i] & 0x7f;
					eap_legacy_rate->ext_rate_len++;
					wdev->eap.eapsupportofdmmcs |= (1 << (i - 4));
					wdev->eap.eapsupportratemode |= SUPPORT_OFDM_MODE;
				}
			}
		}
	}

	rtmpeapupdaterateinfo(wdev->PhyMode, &wdev->rate, &wdev->eap);

	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);

	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		mac_entry = &pAd->MacTab.Content[i];

		if ((IS_ENTRY_CLIENT(mac_entry)) && (mac_entry->Sst == SST_ASSOC)) {
			if (mac_entry->wdev != wdev)
				continue;

			raentry = &mac_entry->RaEntry;
			eaprawrapperentryset(pAd, mac_entry, raentry);
			WifiSysRaInit(pAd, mac_entry);
	    }
	}

	return TRUE;
}

INT set_htsuprateset_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 htsupratesetbitmap = 0, i = 0;
	MAC_TABLE_ENTRY *mac_entry;
	RA_ENTRY_INFO_T *raentry;

	htsupratesetbitmap = (UINT32) os_str_tol(arg, 0, 10);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
		"HtSupRateSetBitmap %x\n", htsupratesetbitmap);
	if (!wdev)
		return FALSE;

	wdev->eap.eap_htsuprate_en = TRUE;
	wdev->eap.eapsupporthtmcs = htsupratesetbitmap;
	wdev->eap.eapmcsset[0] = htsupratesetbitmap & 0x000000ff;
	wdev->eap.eapmcsset[1] = (htsupratesetbitmap & 0x0000ff00) >> 8;
	wdev->eap.eapmcsset[2] = (htsupratesetbitmap & 0x00ff0000) >> 16;
	wdev->eap.eapmcsset[3] = (htsupratesetbitmap & 0xff000000) >> 24;
#ifdef DOT11_N_SUPPORT
	SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);

	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		mac_entry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(mac_entry) && (mac_entry->Sst == SST_ASSOC)) {
			if (mac_entry->wdev != wdev)
				continue;
			raentry = &mac_entry->RaEntry;
			eaprawrapperentryset(pAd, mac_entry, raentry);
			WifiSysRaInit(pAd, mac_entry);
	    }
	}
	return TRUE;
}

INT set_vhtsuprateset_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 vhtsupratesetitmap = 0;
	UINT32 i = 0;
	MAC_TABLE_ENTRY *mac_entry;
	RA_ENTRY_INFO_T *raentry;

	vhtsupratesetitmap = (UINT32) os_str_tol(arg, 0, 10);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
		"VhtSupRateSetBitmap %x\n", vhtsupratesetitmap);
	if (!wdev)
		return FALSE;

	wdev->eap.eap_vhtsuprate_en = TRUE;
	wdev->eap.rx_mcs_map.mcs_ss1 = vhtsupratesetitmap & 0x0000003;
	wdev->eap.rx_mcs_map.mcs_ss2 = (vhtsupratesetitmap & 0x0000000c) >> 2;
	wdev->eap.rx_mcs_map.mcs_ss3 = (vhtsupratesetitmap & 0x00000030) >> 4;
	wdev->eap.rx_mcs_map.mcs_ss4 = (vhtsupratesetitmap & 0x000000c0) >> 6;
	wdev->eap.rx_mcs_map.mcs_ss5 = (vhtsupratesetitmap & 0x00000300) >> 8;
	wdev->eap.rx_mcs_map.mcs_ss6 = (vhtsupratesetitmap & 0x00000c00) >> 10;
	wdev->eap.rx_mcs_map.mcs_ss7 = (vhtsupratesetitmap & 0x00003000) >> 12;
	wdev->eap.rx_mcs_map.mcs_ss8 = (vhtsupratesetitmap & 0x0000c000) >> 14;

	wdev->eap.tx_mcs_map.mcs_ss1 = (vhtsupratesetitmap & 0x00030000) >> 16;
	wdev->eap.tx_mcs_map.mcs_ss2 = (vhtsupratesetitmap & 0x000c0000) >> 18;
	wdev->eap.tx_mcs_map.mcs_ss3 = (vhtsupratesetitmap & 0x00300000) >> 20;
	wdev->eap.tx_mcs_map.mcs_ss4 = (vhtsupratesetitmap & 0x00c00000) >> 22;
	wdev->eap.tx_mcs_map.mcs_ss5 = (vhtsupratesetitmap & 0x03000000) >> 24;
	wdev->eap.tx_mcs_map.mcs_ss6 = (vhtsupratesetitmap & 0x0c000000) >> 26;
	wdev->eap.tx_mcs_map.mcs_ss7 = (vhtsupratesetitmap & 0x30000000) >> 28;
	wdev->eap.tx_mcs_map.mcs_ss8 = (vhtsupratesetitmap & 0xc0000000) >> 30;

#ifdef DOT11_N_SUPPORT
	SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		mac_entry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(mac_entry) && (mac_entry->Sst == SST_ASSOC)) {
			if (mac_entry->wdev != wdev)
				continue;
		raentry = &mac_entry->RaEntry;
		eaprawrapperentryset(pAd, mac_entry, raentry);
		WifiSysRaInit(pAd, mac_entry);
	    }
	}
	return TRUE;
}

#ifdef DOT11_HE_AX
static BOOLEAN check_hesuprateset(RTMP_STRING * arg)
{
	UINT comma = 0;
	PUCHAR c;

	MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
		"%s: arg=%s\n", __func__, arg);

	if (!*arg)
		goto HELP;
	for (c = arg; *c; c++) {
		if ((*c == ',') && (++comma > 2))
			goto HELP;

		if ((*c != 'x') && (*c != 'X') && !isxdigit(*c) && (*c != ','))
			goto HELP;
	}

	return TRUE;

HELP:
	MTWF_PRINT("hesuprateset: param error\n");
	MTWF_PRINT("Usage: iwpriv [interface] set hesuprateset=[BW<=80 MCS],[BW160 MCS],[BW8080 MCS]\n");
	MTWF_PRINT("       iwpriv [interface] set hesuprateset=[BW<=80 MCS],[BW160 MCS]\n");
	MTWF_PRINT("       iwpriv [interface] set hesuprateset=[BW<=80 MCS]\n");
	MTWF_PRINT("The [BW* MCS] format is:\n");
	MTWF_PRINT("  bit 1-0  : max MCS for RX/1SS\n");
	MTWF_PRINT("  ...\n");
	MTWF_PRINT("  bit 15-14: max MCS for RX/8SS\n");
	MTWF_PRINT("  bit 17-16: max MCS for TX/1SS\n");
	MTWF_PRINT("  ...\n");
	MTWF_PRINT("  bit 31-30: max MCS for TX/8SS\n");
	MTWF_PRINT("If the [BW* MCS] is omitted, the 0xffffff is set to.\n");
	MTWF_PRINT("eg: iwpriv ra0 set hesuprateset=0xfffdfff0\n");
	MTWF_PRINT("This command set the supported MCS(BW <= 80, RX, SS1&SS2) to 0 and MCS(BW <= 80, RX, SS1) to 1, the rest of MCS are 3.\n");

	return FALSE;
}

INT set_hesuprateset_proc(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(
		pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 mcs_set[3];
	PUCHAR	mcs_ptr;
	INT i = 0;

	if (!wdev)
		return FALSE;

	if (!check_hesuprateset(arg))
		return FALSE;

	memset(mcs_set, 0xff, sizeof(mcs_set));
	for (i = 0, mcs_ptr = rstrtok(arg, ",");
	     mcs_ptr && i < (ARRAY_SIZE(mcs_set));
	     mcs_ptr = rstrtok(NULL, ","), i++) {

		mcs_set[i] = (UINT32)os_str_tol(mcs_ptr, 0, 0);
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
		 "%s mcs_set: 0x%x 0x%x 0x%x\n", wdev->if_dev->name,
		  mcs_set[0], mcs_set[1], mcs_set[2]);

	for (i = 0; i < HE_MAX_SUPPORT_STREAM; i++ ) {
		wdev->eap.rate.he80_rx_nss_mcs[i] = (mcs_set[0] >> (i*2)) & 0x3;
		wdev->eap.rate.he80_tx_nss_mcs[i] = (mcs_set[0] >> (16+i*2)) & 0x3;
		wdev->eap.rate.he160_rx_nss_mcs[i] = (mcs_set[1] >> (i*2)) & 0x3;
		wdev->eap.rate.he160_tx_nss_mcs[i] = (mcs_set[1] >> (16+i*2)) & 0x3;
		wdev->eap.rate.he8080_rx_nss_mcs[i] = (mcs_set[2] >> (i*2)) & 0x3;
		wdev->eap.rate.he8080_tx_nss_mcs[i] = (mcs_set[2] >> (16+i*2)) & 0x3;
	}

	wdev->eap.eap_hesuprate_en = TRUE;

	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		MAC_TABLE_ENTRY *mac_entry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(mac_entry) &&
		    (mac_entry->Sst == SST_ASSOC)) {
			if (mac_entry->wdev != wdev)
				continue;

			eaprawrapperentryset(pAd, mac_entry, &mac_entry->RaEntry);
			RTEnqueueInternalCmd(pAd, CMDTHREAD_UPDATE_MAXRA,
					     mac_entry, sizeof(MAC_TABLE_ENTRY));
		}
	}
	return TRUE;
}
#endif /* DOT11_HE_AX */
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

#ifdef MCAST_RATE_SPECIFIC
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
INT Set_McastType(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	BOOLEAN	status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	MCAST_PKT_TYPE McastType = os_str_tol(arg, 0, 10);
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if ((pObj->ioctl_if_type != INT_MBSSID) && (pObj->ioctl_if_type != INT_MAIN)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Do nothing! This device interface is NOT AP mode!\n");
		return FALSE;
	}

	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Invalid device interface!\n");
		return FALSE;
	}

	if ((McastType >= MCAST_TYPE_BOTH_BCM_PKT) && (McastType <= MCAST_TYPE_BROADCAST_PKT)) {
		wdev->rate.McastType = McastType;
		pAd->CommonCfg.McastTypeFlag = TRUE;
	} else {
		wdev->rate.McastType = MCAST_TYPE_BOTH_BCM_PKT;
		pAd->CommonCfg.McastTypeFlag = FALSE;
		status = FALSE;
	}

	return status;
}
#endif /* MCAST_VENDOR10_CUSTOM_FEATURE */

INT Set_McastPhyMode(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	BSS_INFO_ARGUMENT_T bss_info_argument;
	HTTRANSMIT_SETTING *transmit;
	UCHAR cfg_ht_bw;
	struct wifi_dev *wdev = NULL;
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
	INT i = 0;
	UCHAR bandIdx, mbandIdx;
	struct wifi_dev *mwdev = NULL;
#endif
	BOOLEAN	status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	UCHAR mcast_phy_mode = os_str_tol(arg, 0, 10);

	if ((pObj->ioctl_if_type != INT_MBSSID) && (pObj->ioctl_if_type != INT_MAIN)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Do nothing! This device interface is NOT AP mode!\n");
		return FALSE;
	}

	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Invalid device interface!\n");
		return FALSE;
	}

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	if (WMODE_CAP_5G(wdev->PhyMode)) {
		if (mcast_phy_mode == MCAST_CCK) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "We could not set CCK mode for multicast frames in 5G band!\n");
			return FALSE;
		}
	} else {
#ifdef DOT11_VHT_AC
		if (mcast_phy_mode == MCAST_VHT) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "We could not set VHT mode for multicast frames in 2.4G band!\n");
			return FALSE;
		}
#endif /* DOT11_VHT_AC */
	}
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
	bandIdx = HcGetBandByWdev(wdev);
	transmit = (wdev->channel > 14) ? (&wdev->rate.MCastPhyMode_5G) : (&wdev->rate.MCastPhyMode);
#else
	transmit = &wdev->rate.mcastphymode;
#endif
	cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	transmit->field.BW = cfg_ht_bw;

	switch (mcast_phy_mode) {
	case MCAST_DISABLE: /* disable */
		NdisMoveMemory(transmit, &wdev->rate.MlmeTransmit, sizeof(HTTRANSMIT_SETTING));
		transmit->field.BW =  BW_20;
		break;

	case MCAST_CCK:	/* CCK */
		transmit->field.MODE = MODE_CCK;
		transmit->field.BW =  BW_20;

		if ((transmit->field.MCS > 11) || (transmit->field.MCS > 3 && transmit->field.MCS < 8))
			transmit->field.MCS = 3;

		break;

	case MCAST_OFDM:	/* OFDM */
		transmit->field.MODE = MODE_OFDM;
		transmit->field.BW =  BW_20;

		if (transmit->field.MCS > 7)
			transmit->field.MCS = 7;

		break;
#ifdef DOT11_N_SUPPORT

	case MCAST_HTMIX:	/* HTMIX */
		transmit->field.MODE = MODE_HTMIX;

		if (wlan_operate_get_bw(wdev) > BW_20)
			transmit->field.BW =  BW_40;
		else
			transmit->field.BW =  BW_20;

		break;
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC

	case MCAST_VHT: /* VHT */
		transmit->field.MODE = MODE_VHT;
		transmit->field.BW = wlan_operate_get_bw(wdev);
		break;
#endif /* DOT11_VHT_AC */

	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "unknown Muticast PhyMode %d.\n", mcast_phy_mode);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "0:Disable, 1:CCK, 2:OFDM, 3:HTMIX, 4:VHT.\n");
		status = FALSE;
		break;
	}

	if (!status)
		return FALSE;

#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		mwdev = &pAd->ApCfg.MBSSID[i].wdev;
		mbandIdx =  HcGetBandByWdev(mwdev);
		if (mbandIdx != bandIdx)
			continue;
#endif
		NdisZeroMemory(&bss_info_argument, sizeof(BSS_INFO_ARGUMENT_T));
		bss_info_argument.bss_state = BSS_ACTIVE;
		bss_info_argument.ucBssIndex = wdev->bss_info_argument.ucBssIndex;
		bss_info_argument.u4BssInfoFeature = BSS_INFO_BROADCAST_INFO_FEATURE;
		memmove(&bss_info_argument.BcTransmit, transmit, sizeof(HTTRANSMIT_SETTING));
		memmove(&bss_info_argument.McTransmit, transmit, sizeof(HTTRANSMIT_SETTING));

		if (AsicBssInfoUpdate(pAd, &bss_info_argument) != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Fail to apply the bssinfo, BSSID=%d!\n", apidx);
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
	}
#endif
	return TRUE;
}

INT Set_McastMcs(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	BSS_INFO_ARGUMENT_T bss_info_argument;
	HTTRANSMIT_SETTING *transmit;
	UCHAR txnss;
	struct wifi_dev *wdev = NULL;
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
	INT i = 0;
	UCHAR bandIdx, mbandIdx;
	struct wifi_dev *mwdev = NULL;
#endif
	BOOLEAN	status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	UCHAR Mcs = os_str_tol(arg, 0, 10);

	if ((pObj->ioctl_if_type != INT_MBSSID) && (pObj->ioctl_if_type != INT_MAIN)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Do nothing! This device interface is NOT AP mode!\n");
		return FALSE;
	}

	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Invalid device interface!\n");
		return FALSE;
	}

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
	bandIdx = HcGetBandByWdev(wdev);
	transmit = (wdev->channel > 14) ? (&wdev->rate.MCastPhyMode_5G) : (&wdev->rate.MCastPhyMode);
#else
	transmit = &wdev->rate.mcastphymode;
#endif
	txnss = wlan_operate_get_tx_stream(wdev);

	switch (transmit->field.MODE) {
	case MODE_CCK:
		if ((Mcs <= 3) || (Mcs >= 8 && Mcs <= 11)) {
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED) && (Mcs <= 3)) {
				if (Mcs > 0)
					Mcs = Mcs - 1;
				else
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Invalid MCS(CCK-1M) value when short preamble is enabled!\n");
			}
			transmit->field.MCS = Mcs;
		}
		else {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "MCS must in range of 0 ~ 3 and 8 ~ 11 for CCK Mode.\n");
			status = FALSE;
		}

		break;

	case MODE_OFDM:
		if (Mcs > 7) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "MCS must in range from 0 to 7 for OFDM Mode.\n");
			status = FALSE;
		} else
			transmit->field.MCS = Mcs;

		break;

	case MODE_HTMIX:
		if ((txnss == 1 && Mcs > 7) || (txnss == 2 && Mcs > 15)
			|| (txnss == 3 && Mcs > 23) || (txnss == 4 && Mcs > 31)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "MCS(%d) txnss(%d) error for HT Mode.\n", Mcs, txnss);
			status = FALSE;
		} else
			transmit->field.MCS = Mcs;

		break;

	case MODE_VHT:
		if ((Mcs & 0x0f) > 9 || ((Mcs>>4) & 0x3) + 1 > txnss) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "MCS(%d > 9) txnss(%d > %d) error for VHT Mode.\n",
					 Mcs & 0x0f, ((Mcs>>4) & 0x3) + 1, txnss);
			status = FALSE;
		} else
			transmit->field.MCS = Mcs;

		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "unknown Muticast PhyMode in set mcs %d.\n", transmit->field.MODE);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "0:Disable, 1:CCK, 2:OFDM, 3:HTMIX, 4:VHT.\n");
		status = FALSE;
		break;

	}

	if (!status)
		return FALSE;
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		mwdev = &pAd->ApCfg.MBSSID[i].wdev;
		mbandIdx =  HcGetBandByWdev(mwdev);
		if (mbandIdx != bandIdx)
			continue;
#endif
		NdisZeroMemory(&bss_info_argument, sizeof(BSS_INFO_ARGUMENT_T));
		bss_info_argument.bss_state = BSS_ACTIVE;
		bss_info_argument.ucBssIndex = wdev->bss_info_argument.ucBssIndex;
		bss_info_argument.u4BssInfoFeature = BSS_INFO_BROADCAST_INFO_FEATURE;
		memmove(&bss_info_argument.BcTransmit, transmit, sizeof(HTTRANSMIT_SETTING));
		memmove(&bss_info_argument.McTransmit, transmit, sizeof(HTTRANSMIT_SETTING));

		if (AsicBssInfoUpdate(pAd, &bss_info_argument) != NDIS_STATUS_SUCCESS)
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Fail to apply the bssinfo, BSSID=%d!\n", apidx);
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
	}
#endif
	return TRUE;
}

INT Show_McastRate(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	HTTRANSMIT_SETTING *transmit;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if ((pObj->ioctl_if_type != INT_MBSSID) && (pObj->ioctl_if_type != INT_MAIN)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Do nothing! This device interface is NOT AP mode!\n");
		return FALSE;
	}

	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid device interface!\n");
		return FALSE;
	}

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
	transmit = (wdev->channel > 14) ? (&wdev->rate.MCastPhyMode_5G) : (&wdev->rate.MCastPhyMode);
#else
	transmit = &wdev->rate.mcastphymode;
#endif
	MTWF_PRINT("Mcast PhyMode = %d\n", transmit->field.MODE);
	MTWF_PRINT("Mcast Mcs = %d\n", transmit->field.MCS);
	MTWF_PRINT("Mcast BW = %d\n", transmit->field.BW);
	MTWF_PRINT("Mcast Transmit = %x\n", transmit->word);
	if (transmit->field.MODE == MODE_CCK)
		MTWF_PRINT("Enable %s Preamble\n",
				OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED) ? "Short" : "Long");
	return TRUE;
}
#endif /* MCAST_RATE_SPECIFIC */

#ifdef HIGHPRI_RATE_SPECIFIC
INT Set_HighPriPhyMode(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	BSS_INFO_ARGUMENT_T bss_info_argument;
	HTTRANSMIT_SETTING *transmit;
	UCHAR cfg_ht_bw;
	struct wifi_dev *wdev = NULL;
	INT i = 0;
	UCHAR bandIdx, mbandIdx;
	struct wifi_dev *mwdev = NULL;
	BOOLEAN	status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	UCHAR highpri_phy_mode;
	UCHAR *this_char;
	UINT8 frame_type;
	UINT8 is_frame = 1;

	if ((pObj->ioctl_if_type != INT_MBSSID) && (pObj->ioctl_if_type != INT_MAIN)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Do nothing! This device interface is NOT AP mode!\n");
		return FALSE;
	}

	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Invalid device interface!\n");
		return FALSE;
	}

	while ((this_char = strsep((char **)&arg, "-")) != NULL) {
		if (is_frame) {
			if (strlen(this_char) != 1)
				goto HELP;

			frame_type = os_str_tol(this_char, 0, 10);
			if (frame_type >= HIGHPRI_MAX_TYPE)
				goto HELP;

			is_frame = 0;
		} else {
			highpri_phy_mode = os_str_tol(this_char, 0, 10);
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			if (WMODE_CAP_5G(wdev->PhyMode)) {
				if (highpri_phy_mode == HIGHPRI_CCK) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "We could not set CCK mode for high_pri frames in 5G band!\n");
					status = FALSE;
				}
			} else {
#ifdef DOT11_VHT_AC
				if (highpri_phy_mode == HIGHPRI_VHT) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "We could not set VHT mode for high_pri frames in 2.4G band!\n");
					status = FALSE;
				}
#endif /* DOT11_VHT_AC */
			}
			bandIdx = HcGetBandByWdev(wdev);
			transmit = (wdev->channel > 14) ?
						(&wdev->rate.HighPriPhyMode_5G[frame_type]) : (&wdev->rate.HighPriPhyMode[frame_type]);

			cfg_ht_bw = wlan_config_get_ht_bw(wdev);
			transmit->field.BW = cfg_ht_bw;

			switch (highpri_phy_mode) {
			case HIGHPRI_DISABLE: /* disable */
				NdisMoveMemory(transmit, &wdev->rate.MlmeTransmit, sizeof(HTTRANSMIT_SETTING));
				transmit->field.BW =  BW_20;
				break;

			case HIGHPRI_CCK:	/* CCK */
				transmit->field.MODE = MODE_CCK;
				transmit->field.BW =  BW_20;

				if ((transmit->field.MCS > 11) || (transmit->field.MCS > 3 && transmit->field.MCS < 8))
					transmit->field.MCS = 3;

				break;

			case HIGHPRI_OFDM:	/* OFDM */
				transmit->field.MODE = MODE_OFDM;
				transmit->field.BW =  BW_20;

				if (transmit->field.MCS > 7)
					transmit->field.MCS = 7;

				break;
#ifdef DOT11_N_SUPPORT

			case HIGHPRI_HTMIX:	/* HTMIX */
				transmit->field.MODE = MODE_HTMIX;

				if (wlan_operate_get_bw(wdev) > BW_20)
					transmit->field.BW =  BW_40;
				else
					transmit->field.BW =  BW_20;

				break;
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC

			case HIGHPRI_VHT: /* VHT */
				transmit->field.MODE = MODE_VHT;
				transmit->field.BW = wlan_operate_get_bw(wdev);
				break;
#endif /* DOT11_VHT_AC */

			default:
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "unknown HighPriority PhyMode %d.\n", highpri_phy_mode);
				goto HELP;
			}

			if (!status)
				return FALSE;

			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				mwdev = &pAd->ApCfg.MBSSID[i].wdev;
				mbandIdx =  HcGetBandByWdev(mwdev);
				if (mbandIdx != bandIdx)
					continue;
				NdisZeroMemory(&bss_info_argument, sizeof(BSS_INFO_ARGUMENT_T));
				bss_info_argument.bss_state = BSS_ACTIVE;
				bss_info_argument.ucBssIndex = wdev->bss_info_argument.ucBssIndex;
				bss_info_argument.u4BssInfoFeature = 1 << (BSS_INFO_HIGHPRI_RATE_ARP + frame_type);
				memmove(&bss_info_argument.HighPriTransmit[frame_type], transmit, sizeof(HTTRANSMIT_SETTING));

				if (AsicBssInfoUpdate(pAd, &bss_info_argument) != NDIS_STATUS_SUCCESS)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Fail to apply the bssinfo, BSSID=%d!\n", apidx);
			}

			return TRUE;
		}
	}
HELP:
	MTWF_PRINT("\
%s(): input param error\n\
Usage: iwpriv [inf] set HighPriPhyMode=[frame_type]-[phymode]\n\
The frame type format is 0 for ARP, 1 for DHCP, 2 for EAPOL\n\
The phymode format is 0:Disable, 1:CCK, 2:OFDM, 3:HTMIX, 4:VHT\n\
", __func__);
	return FALSE;
}

INT Set_HighPriMcs(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	BSS_INFO_ARGUMENT_T bss_info_argument;
	HTTRANSMIT_SETTING *transmit;
	UCHAR txnss;
	struct wifi_dev *wdev = NULL;
	INT i = 0;
	UCHAR bandIdx, mbandIdx;
	struct wifi_dev *mwdev = NULL;
	BOOLEAN	status = TRUE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	UCHAR Mcs;
	UCHAR *this_char;
	UINT8 frame_type;
	UINT8 is_frame = 1;

	if ((pObj->ioctl_if_type != INT_MBSSID) && (pObj->ioctl_if_type != INT_MAIN)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Do nothing! This device interface is NOT AP mode!\n");
		return FALSE;
	}

	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Invalid device interface!\n");
		return FALSE;
	}

	while ((this_char = strsep((char **)&arg, "-")) != NULL) {
		if (is_frame) {
			if (strlen(this_char) != 1)
				goto HELP;

			frame_type = os_str_tol(this_char, 0, 10);
			if (frame_type >= HIGHPRI_MAX_TYPE)
				goto HELP;
			is_frame = 0;
		} else {
			Mcs = os_str_tol(this_char, 0, 10);
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			bandIdx = HcGetBandByWdev(wdev);
			transmit = (wdev->channel > 14) ?
						(&wdev->rate.HighPriPhyMode_5G[frame_type]) : (&wdev->rate.HighPriPhyMode[frame_type]);

			txnss = wlan_operate_get_tx_stream(wdev);

			switch (transmit->field.MODE) {
			case MODE_CCK:
				if ((Mcs <= 3) || (Mcs >= 8 && Mcs <= 11)) {
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED) && (Mcs <= 3)) {
						if (Mcs > 0)
							Mcs = Mcs - 1;
						else
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
									"Invalid MCS(CCK-1M) value when short preamble is enabled!\n");
					}
					transmit->field.MCS = Mcs;
				} else {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "MCS must in range of 0 ~ 3 and 8 ~ 11 for CCK Mode.\n");
					status = FALSE;
				}

				break;

			case MODE_OFDM:
				if (Mcs > 7) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "MCS must in range from 0 to 7 for OFDM Mode.\n");
					status = FALSE;
				} else
					transmit->field.MCS = Mcs;

				break;

			case MODE_HTMIX:
				if ((txnss == 1 && Mcs > 7) || (txnss == 2 && Mcs > 15)
					|| (txnss == 3 && Mcs > 23) || (txnss == 4 && Mcs > 31)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "MCS(%d) txnss(%d) error for HT Mode.\n", Mcs, txnss);
					status = FALSE;
				} else
					transmit->field.MCS = Mcs;

				break;

			case MODE_VHT:
				if ((Mcs & 0x0f) > 9 || ((Mcs>>4) & 0x3) + 1 > txnss) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "MCS(%d > 9) txnss(%d > %d) error for VHT Mode.\n",
							 Mcs & 0x0f, ((Mcs>>4) & 0x3) + 1, txnss);
					status = FALSE;
				} else
					transmit->field.MCS = Mcs;

				break;

			default:
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "unknown high_pri PhyMode in set mcs %d.\n", transmit->field.MODE);
				goto HELP;
			}

			if (!status)
				return FALSE;

			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				mwdev = &pAd->ApCfg.MBSSID[i].wdev;
				mbandIdx =	HcGetBandByWdev(mwdev);
				if (mbandIdx != bandIdx)
					continue;

				NdisZeroMemory(&bss_info_argument, sizeof(BSS_INFO_ARGUMENT_T));
				bss_info_argument.bss_state = BSS_ACTIVE;
				bss_info_argument.ucBssIndex = wdev->bss_info_argument.ucBssIndex;
				bss_info_argument.u4BssInfoFeature = 1 << (BSS_INFO_HIGHPRI_RATE_ARP + frame_type);
				memmove(&bss_info_argument.HighPriTransmit[frame_type], transmit, sizeof(HTTRANSMIT_SETTING));
				if (AsicBssInfoUpdate(pAd, &bss_info_argument) != NDIS_STATUS_SUCCESS)
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "Fail to apply the bssinfo, BSSID=%d!\n", apidx);
			}

			return TRUE;
		}
	}
HELP:
	MTWF_PRINT("%s(): input param error\n"
				"Usage: iwpriv [inf] set HighPriMcs=[frame_type]-[mcs]\n"
				"The frame type format is 0 for ARP, 1 for DHCP, 2 for EAPOL\n"
				"MCS must in range of 0 ~ 3 and 8 ~ 11 for CCK Mode\n"
				"MCS must in range from 0 to 7 for OFDM Mode\n", __func__);
	return FALSE;

}

INT Show_HighPriRate(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	HTTRANSMIT_SETTING *transmit;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	UINT i = 0;

	if ((pObj->ioctl_if_type != INT_MBSSID) && (pObj->ioctl_if_type != INT_MAIN)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Do nothing! This device interface is NOT AP mode!\n");
		return FALSE;
	}

	if (apidx >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid device interface!\n");
		return FALSE;
	}

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	for (i = 0; i < HIGHPRI_MAX_TYPE; i++) {
		transmit = (wdev->channel > 14) ? (&wdev->rate.HighPriPhyMode_5G[i]) : (&wdev->rate.HighPriPhyMode[i]);

		switch (i) {
		case HIGHPRI_ARP:
			MTWF_PRINT("ARP frame:\n");
			break;
		case HIGHPRI_DHCP:
			MTWF_PRINT("DHCP frame:\n");
			break;
		case HIGHPRI_EAPOL:
			MTWF_PRINT("EAPOL fram:\n");
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "It exceeds HIGHPRI_MAX_TYPE\n");
		}
		MTWF_PRINT("HighPri PhyMode = %d\n", transmit->field.MODE);
		MTWF_PRINT("HighPri Mcs = %d\n", transmit->field.MCS);
		MTWF_PRINT("HighPri BW = %d\n", transmit->field.BW);
		MTWF_PRINT("HighPri Transmit = %x\n", transmit->word);
		if (transmit->field.MODE == MODE_CCK)
			MTWF_PRINT("Enable %s Preamble\n",
					OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED) ? "Short" : "Long");
	}
	return TRUE;
}
#endif /* HIGHPRI_RATE_SPECIFIC */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
INT Set_OBSSScanParam_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT ObssScanValue;
	UINT Idx;
	RTMP_STRING *thisChar;

	Idx = 0;

	while ((thisChar = strsep((char **)&arg, "-")) != NULL) {
		ObssScanValue = (INT) os_str_tol(thisChar, 0, 10);

		switch (Idx) {
		case 0:
			if (ObssScanValue < 5 || ObssScanValue > 1000)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "Invalid OBSSScanParam for Dot11OBssScanPassiveDwell(%d), should in range 5~1000\n", ObssScanValue);
			else {
				pAd->CommonCfg.Dot11OBssScanPassiveDwell = ObssScanValue;	/* Unit : TU. 5~1000 */
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11OBssScanPassiveDwell=%d\n",
						 ObssScanValue);
			}

			break;

		case 1:
			if (ObssScanValue < 10 || ObssScanValue > 1000)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "Invalid OBSSScanParam for Dot11OBssScanActiveDwell(%d), should in range 10~1000\n", ObssScanValue);
			else {
				pAd->CommonCfg.Dot11OBssScanActiveDwell = ObssScanValue;	/* Unit : TU. 10~1000 */
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11OBssScanActiveDwell=%d\n",
						 ObssScanValue);
			}

			break;

		case 2:
			pAd->CommonCfg.Dot11BssWidthTriggerScanInt = ObssScanValue;	/* Unit : Second */
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11BssWidthTriggerScanInt=%d\n",
					 ObssScanValue);
			break;

		case 3:
			if (ObssScanValue < 200 || ObssScanValue > 10000)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "Invalid OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel(%d), should in range 200~10000\n", ObssScanValue);
			else {
				pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = ObssScanValue;	/* Unit : TU. 200~10000 */
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11OBssScanPassiveTotalPerChannel=%d\n",
						 ObssScanValue);
			}

			break;

		case 4:
			if (ObssScanValue < 20 || ObssScanValue > 10000)
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 "Invalid OBSSScanParam for Dot11OBssScanActiveTotalPerChannel(%d), should in range 20~10000\n", ObssScanValue);
			else {
				pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = ObssScanValue;	/* Unit : TU. 20~10000 */
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11OBssScanActiveTotalPerChannel=%d\n",
						 ObssScanValue);
			}

			break;

		case 5:
			pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = ObssScanValue;
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11BssWidthChanTranDelayFactor=%d\n",
					 ObssScanValue);
			break;

		case 6:
			pAd->CommonCfg.Dot11OBssScanActivityThre = ObssScanValue;	/* Unit : percentage */
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11BssWidthChanTranDelayFactor=%d\n",
					 ObssScanValue);
			break;
		}

		Idx++;
	}

	if (Idx != 7) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Wrong OBSSScanParamtetrs format in ioctl cmd!!!!! Use default value\n");
		pAd->CommonCfg.Dot11OBssScanPassiveDwell = dot11OBSSScanPassiveDwell;	/* Unit : TU. 5~1000 */
		pAd->CommonCfg.Dot11OBssScanActiveDwell = dot11OBSSScanActiveDwell;	/* Unit : TU. 10~1000 */
		pAd->CommonCfg.Dot11BssWidthTriggerScanInt = dot11BSSWidthTriggerScanInterval;	/* Unit : Second */
		pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = dot11OBSSScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000 */
		pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = dot11OBSSScanActiveTotalPerChannel;	/* Unit : TU. 20~10000 */
		pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = dot11BSSWidthChannelTransactionDelayFactor;
		pAd->CommonCfg.Dot11OBssScanActivityThre = dot11BSSScanActivityThreshold;	/* Unit : percentage */
	}

	pAd->CommonCfg.Dot11BssWidthChanTranDelay = (ULONG)(pAd->CommonCfg.Dot11BssWidthTriggerScanInt *
			pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "OBSSScanParam for Dot11BssWidthChanTranDelay=%ld\n",
			 pAd->CommonCfg.Dot11BssWidthChanTranDelay);
	return TRUE;
}

INT	Set_AP2040ReScan_Proc(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev;
	UINT	ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}

	if (pAd->CommonCfg.bBssCoexEnable == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "BssCoex not enable\n");
		return FALSE;
	}

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	APOverlappingBSSScan(pAd, wdev);
	/* apply setting */
	SetCommonHtVht(pAd, wdev);

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Trigger AP ReScan !!!\n");
	return TRUE;
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

INT Set_EntryLifeCheck_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG LifeCheckCnt = (ULONG) os_str_tol(arg, 0, 10);

	if (LifeCheckCnt <= 65535)
		pAd->ApCfg.EntryLifeCheck = LifeCheckCnt;
	else
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "LifeCheckCnt must in range of 0 to 65535\n");

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "EntryLifeCheck Cnt = %ld.\n", pAd->ApCfg.EntryLifeCheck);
	return TRUE;
}

INT Set_StaPowerOffDetect_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_STRING *this_char;
	UINT16 err_times;
	ULONG tx_contd_fail_cnt;

	this_char = strsep((char **)&arg, "-");
	if (this_char == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid input arg");
		return FALSE;
	}
	err_times = (UINT16)os_str_tol(this_char, 0, 10);

	this_char = strsep((char **)&arg, "-");
	if (this_char == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid input arg");
		return FALSE;
	}
	tx_contd_fail_cnt = (ULONG)os_str_tol(this_char, 0, 10);

	pAd->ApCfg.per_err_total = err_times;
	pAd->ApCfg.tx_contd_fail_total = tx_contd_fail_cnt * err_times;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"set STA power off detect with per_err_total = %d, tx_contd_fail_total = %ld\n",
			pAd->ApCfg.per_err_total, pAd->ApCfg.tx_contd_fail_total);

	return TRUE;
}

INT	ApCfg_Set_PerMbssMaxStaNum_Proc(
	IN PRTMP_ADAPTER	pAd,
	IN INT				apidx,
	IN RTMP_STRING *arg)
{
	UINT	ifIndex = apidx;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
		return FALSE;
	}
	pAd->ApCfg.MBSSID[ifIndex].MaxStaNum = (UINT16)os_str_tol(arg, 0, 10);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(ra%d) (MaxStaNum=%d)\n",
			 ifIndex, pAd->ApCfg.MBSSID[ifIndex].MaxStaNum);

	return TRUE;
}

INT	Set_MemDebug_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef VENDOR_FEATURE2_SUPPORT
	MTWF_PRINT("Number of Packet Allocated = %lu\n", OS_NumOfPktAlloc);
	MTWF_PRINT("Number of Packet Freed = %lu\n", OS_NumOfPktFree);
	MTWF_PRINT("Offset of Packet Allocated/Freed = %lu\n", OS_NumOfPktAlloc - OS_NumOfPktFree);
#endif /* VENDOR_FEATURE2_SUPPORT */
#ifdef VENDOR_FEATURE4_SUPPORT
	MTWF_PRINT("OS_NumOfMemAlloc:%lu OS_NumOfMemFree:%lu", OS_NumOfMemAlloc, OS_NumOfMemFree);
#endif
	return TRUE;
}

#ifdef APCLI_SUPPORT
#ifdef WPA_SUPPLICANT_SUPPORT
VOID RTMPApCliAddKey(
	IN	PRTMP_ADAPTER	    pAd,
	IN	INT				apidx,
	IN	PNDIS_APCLI_802_11_KEY    pKey)
{
	ULONG				KeyIdx;
	MAC_TABLE_ENTRY		*pEntry;
	STA_TR_ENTRY *tr_entry;
	INT	ifIndex, BssIdx;
	PSTA_ADMIN_CONFIG pApCliEntry;
	struct wifi_dev *wdev;
	MAC_TABLE_ENTRY				*pMacEntry = (MAC_TABLE_ENTRY *)NULL;
	UINT KeyLength = 0;

	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RTMPApCliAddKey ------>\n");
	ifIndex = apidx;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	wdev = &pApCliEntry->wdev;
	pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
	tr_entry = &pAd->MacTab.tr_entry[pApCliEntry->MacTabWCID];
	BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + ifIndex;

	if (wdev->AuthMode >= Ndis802_11AuthModeWPA) {
		if (pKey->KeyIndex & 0x80000000) {
			if (wdev->AuthMode == Ndis802_11AuthModeWPANone) {
				NdisZeroMemory(pApCliEntry->PMK, 32);
				if (pKey->KeyLength > ARRAY_SIZE(pApCliEntry->PMK))
					KeyLength = ARRAY_SIZE(pApCliEntry->PMK);
				else
					KeyLength = pKey->KeyLength;
				NdisMoveMemory(pApCliEntry->PMK, pKey->KeyMaterial, KeyLength);
				goto end;
			}

			/* Update PTK */
			NdisZeroMemory(&pMacEntry->PairwiseKey, sizeof(CIPHER_KEY));
			pMacEntry->PairwiseKey.KeyLen = LEN_TK;
			NdisMoveMemory(pMacEntry->PairwiseKey.Key, pKey->KeyMaterial, LEN_TK);

			if (pApCliEntry->PairCipher == Ndis802_11TKIPEnable) {
				NdisMoveMemory(pMacEntry->PairwiseKey.RxMic, pKey->KeyMaterial + LEN_TK, LEN_TKIP_MIC);
				NdisMoveMemory(pMacEntry->PairwiseKey.TxMic, pKey->KeyMaterial + LEN_TK + LEN_TKIP_MIC, LEN_TKIP_MIC);
			} else {
				NdisMoveMemory(pMacEntry->PairwiseKey.TxMic, pKey->KeyMaterial + LEN_TK, LEN_TKIP_MIC);
				NdisMoveMemory(pMacEntry->PairwiseKey.RxMic, pKey->KeyMaterial + LEN_TK + LEN_TKIP_MIC, LEN_TKIP_MIC);
			}

			/* Decide its ChiperAlg */
			if (pApCliEntry->PairCipher == Ndis802_11TKIPEnable)
				pMacEntry->PairwiseKey.CipherAlg = CIPHER_TKIP;
			else if (pApCliEntry->PairCipher == Ndis802_11AESEnable)
				pMacEntry->PairwiseKey.CipherAlg = CIPHER_AES;
			else
				pMacEntry->PairwiseKey.CipherAlg = CIPHER_NONE;

			AsicAddPairwiseKeyEntry(
				pAd,
				pMacEntry->wcid,
				&pMacEntry->PairwiseKey);

			if (pMacEntry->AuthMode >= Ndis802_11AuthModeWPA) {
				/* set 802.1x port control */
				tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
				pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
			}
		} else {
			/* Update GTK  */
			wdev->DefaultKeyId = (pKey->KeyIndex & 0xFF);
			NdisZeroMemory(&pApCliEntry->SharedKey[wdev->DefaultKeyId], sizeof(CIPHER_KEY));
			pApCliEntry->SharedKey[wdev->DefaultKeyId].KeyLen = LEN_TK;
			NdisMoveMemory(pApCliEntry->SharedKey[wdev->DefaultKeyId].Key, pKey->KeyMaterial, LEN_TK);

			if (pApCliEntry->GroupCipher == Ndis802_11TKIPEnable) {
				NdisMoveMemory(pApCliEntry->SharedKey[wdev->DefaultKeyId].RxMic, pKey->KeyMaterial + LEN_TK, LEN_TKIP_MIC);
				NdisMoveMemory(pApCliEntry->SharedKey[wdev->DefaultKeyId].TxMic, pKey->KeyMaterial + LEN_TK + LEN_TKIP_MIC,
							   LEN_TKIP_MIC);
			} else {
				NdisMoveMemory(pApCliEntry->SharedKey[wdev->DefaultKeyId].TxMic, pKey->KeyMaterial + LEN_TK, LEN_TKIP_MIC);
				NdisMoveMemory(pApCliEntry->SharedKey[wdev->DefaultKeyId].RxMic, pKey->KeyMaterial + LEN_TK + LEN_TKIP_MIC,
							   LEN_TKIP_MIC);
			}

			/* Update Shared Key CipherAlg */
			pApCliEntry->SharedKey[wdev->DefaultKeyId].CipherAlg = CIPHER_NONE;

			if (pApCliEntry->GroupCipher == Ndis802_11TKIPEnable)
				pApCliEntry->SharedKey[wdev->DefaultKeyId].CipherAlg = CIPHER_TKIP;
			else if (pApCliEntry->GroupCipher == Ndis802_11AESEnable)
				pApCliEntry->SharedKey[wdev->DefaultKeyId].CipherAlg = CIPHER_AES;

			/* Update group key information to ASIC Shared Key Table */
			AsicAddSharedKeyEntry(pAd,
								  BssIdx,
								  wdev->DefaultKeyId,
								  &pApCliEntry->SharedKey[wdev->DefaultKeyId]);
			/* Update ASIC WCID attribute table and IVEIV table */
			RTMPAddWcidAttributeEntry(pAd,
									  BssIdx,
									  wdev->DefaultKeyId,
									  pApCliEntry->SharedKey[wdev->DefaultKeyId].CipherAlg,
									  NULL);

			/* set 802.1x port control */
			if (pMacEntry->AuthMode >= Ndis802_11AuthModeWPA) {
				/* set 802.1x port control */
				tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
				pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
			}
		}
	} else {	/* dynamic WEP from wpa_supplicant */
		UCHAR	CipherAlg;
		PUCHAR	Key;

		if (pKey->KeyLength == 32)
			goto end;

		KeyIdx = pKey->KeyIndex & 0x0fffffff;

		if (KeyIdx < 4) {
			/* it is a default shared key, for Pairwise key setting */
			if (pKey->KeyIndex & 0x80000000) {
				pEntry = MacTableLookup(pAd, pKey->BSSID);

				if (pEntry && IS_ENTRY_PEER_AP(pEntry)) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RTMPAddKey: Set Pair-wise Key\n");
					/* set key material and key length */
					if ((UCHAR)pKey->KeyLength > ARRAY_SIZE(pEntry->PairwiseKey.Key))
						KeyLength = ARRAY_SIZE(pEntry->PairwiseKey.Key);
					else
						KeyLength = (UCHAR)pKey->KeyLength ;
					pEntry->PairwiseKey.KeyLen = KeyLength;
					NdisMoveMemory(pEntry->PairwiseKey.Key, &pKey->KeyMaterial, KeyLength);

					/* set Cipher type */
					if (pKey->KeyLength == 5)
						pEntry->PairwiseKey.CipherAlg = CIPHER_WEP64;
					else
						pEntry->PairwiseKey.CipherAlg = CIPHER_WEP128;

					/* Add Pair-wise key to Asic */
					AsicAddPairwiseKeyEntry(
						pAd,
						pEntry->wcid,
						&pEntry->PairwiseKey);
				}
			} else {
				/* Default key for tx (shared key) */
				wdev->DefaultKeyId = (UCHAR) KeyIdx;
				/*/ set key material and key length */
				if ((UCHAR)pKey->KeyLength > ARRAY_SIZE(pApCliEntry->SharedKey[KeyIdx].Key))
					KeyLength = ARRAY_SIZE(pApCliEntry->SharedKey[KeyIdx].Key);
				else
					KeyLength = (UCHAR)pKey->KeyLength;
				pApCliEntry->SharedKey[KeyIdx].KeyLen = KeyLength;
				NdisMoveMemory(pApCliEntry->SharedKey[KeyIdx].Key, &pKey->KeyMaterial, KeyLength);

				/* Set Ciper type */
				if (pKey->KeyLength == 5)
					pApCliEntry->SharedKey[KeyIdx].CipherAlg = CIPHER_WEP64;
				else
					pApCliEntry->SharedKey[KeyIdx].CipherAlg = CIPHER_WEP128;

				CipherAlg = pApCliEntry->SharedKey[KeyIdx].CipherAlg;
				Key = pApCliEntry->SharedKey[KeyIdx].Key;
				/* Set Group key material to Asic */
				AsicAddSharedKeyEntry(pAd, BssIdx, KeyIdx, &pApCliEntry->SharedKey[KeyIdx]);
				/* STA doesn't need to set WCID attribute for group key */
				/* Update WCID attribute table and IVEIV table for this group key table */
				RTMPAddWcidAttributeEntry(pAd, BssIdx, KeyIdx, CipherAlg, NULL);
			}
		}
	}

end:
	MTWF_DBG(pAd, DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "<------ RTMPApCliAddKey\n");
}
#endif/*WPA_SUPPLICANT_SUPPORT*/
#endif/*APCLI_SUPPORT*/

#ifdef CONFIG_AP_SUPPORT
/*
========================================================================
Routine Description:
	Set power save life time.

Arguments:
	pAd					- WLAN control block pointer
	Arg					- Input arguments

Return Value:
	None

Note:
========================================================================
*/
INT	Set_PowerSaveLifeTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->MacTab.MsduLifeTime = os_str_tol(arg, 0, 10);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Set new life time = %d\n", pAd->MacTab.MsduLifeTime);
	return TRUE;
}
#endif /* CONFIG_AP_SUPPORT */

#ifdef MBSS_SUPPORT
/*
========================================================================
Routine Description:
	Show MBSS information.

Arguments:
	pAd					- WLAN control block pointer
	Arg					- Input arguments

Return Value:
	None

Note:
========================================================================
*/
extern UCHAR *wmode_2_str(USHORT wmode);

INT	Show_MbssInfo_Display_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 IdBss;
	CHAR *mod_str = NULL;
	BSS_STRUCT *mbss = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	UINT8 dump_lvl = 0;
	UINT8 DbdcIdx;
	struct wifi_dev *wdev = NULL;
	INT i;
	UINT8 bss_start_idx = 0;

	if (arg != NULL)
		dump_lvl = os_str_tol(arg, 0, 10);

	/* dump partially, msg_len is limited to 1024 bytes */
	if ((dump_lvl > 1) && (dump_lvl < pAd->ApCfg.BssidNum))
		bss_start_idx = dump_lvl;

	/* dump mbss info*/
	MTWF_PRINT("\n\tBssidNum=%d %s\n",
		pAd->ApCfg.BssidNum, pAd->CommonCfg.dbdc_mode ? "(DBDC)" : "");

	for (DbdcIdx = DBDC_BAND0; DbdcIdx < HcGetAmountOfBand(pAd); DbdcIdx++) {
#ifdef DOT11V_MBSSID_SUPPORT
		if (IS_BSSID_11V_ENABLED(pAd, DbdcIdx)) {
			MTWF_PRINT("\t[B%d]: BssidNum=%2d, max_bssid_indicator=%d, bitmap=0x%08x, trans_bss_idx=%d\n",
				DbdcIdx,
				pAd->ApCfg.BssidNumPerBand[DbdcIdx],
				pAd->ApCfg.dot11v_max_bssid_indicator[DbdcIdx],
				pAd->ApCfg.dot11v_mbssid_bitmap[DbdcIdx],
				pAd->ApCfg.dot11v_trans_bss_idx[DbdcIdx]);
		} else
#endif
			MTWF_PRINT("\t[B%d]: BssidNum=%2d\n", DbdcIdx, pAd->ApCfg.BssidNumPerBand[DbdcIdx]);
	}

	MTWF_PRINT("\n\tBSS Idx\t\tPhy Mode\tIF_addr\t\t\tgrp [max:%d]\n", pAd->max_bssgroup_num);

	for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {

		/* start bss index to dump */
		if (IdBss < bss_start_idx)
			continue;

		mbss = &pAd->ApCfg.MBSSID[IdBss];
		wdev = &mbss->wdev;
		pSecConfig = &wdev->SecConfig;
		mod_str = wmode_2_str(wdev->PhyMode);
		DbdcIdx = HcGetBandByWdev(wdev);

		MTWF_PRINT("\t%d\t%16s\t"MACSTR"\t%d\t(%s)",
			IdBss, mod_str, MAC2STR(wdev->if_addr),
			mbss->mbss_grp_idx,
			RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev));
#ifdef DOT11V_MBSSID_SUPPORT
		if (IS_BSSID_11V_ENABLED(pAd, DbdcIdx)) {
			MTWF_PRINT("\t(%s)",
				(IS_BSSID_11V_CO_HOSTED(pAd, mbss, DbdcIdx) ?
				"11vCoH" : (IS_BSSID_11V_TRANSMITTED(pAd, mbss, DbdcIdx) ? "11vT" : "11vNT")));
		} else {
			/* 11v bssid disabled */
			MTWF_PRINT("\t(Legacy)");
		}
#endif

		MTWF_PRINT("\n");
		/* advanced info */
		if (dump_lvl > 0) {
			MTWF_PRINT("\t\t\t - SSID: %s (H=%d), Short: %08x, pfIdx: %2d\n",
				 mbss->Ssid, mbss->bHideSsid, mbss->ShortSSID, PF_TO_BSS_IDX(pAd, IdBss));

			MTWF_PRINT("\t\t\t - Band:%d (OM:0x%02x), Ch:%3d, Dtim:%2d, StaCount:%3d\n",
				DbdcIdx, wdev->OmacIdx,
				wdev->channel, mbss->DtimPeriod, mbss->StaCount);

			MTWF_PRINT("\t\t\t - AuthMode: %s, Cipher(P:%s/G:%s), BmcIdx=%d\n",
				GetAuthModeStr(GET_SEC_AKM(pSecConfig)),
				GetEncryModeStr(GET_PAIRWISE_CIPHER(pSecConfig)),
				GetEncryModeStr(GET_GROUP_CIPHER(pSecConfig)),
				wdev->bss_info_argument.bmc_wlan_idx);

			MTWF_PRINT("\t\t\t - MaxIdle:(IEen:%d, Period:%5ds, Opt:0x%x)\n",
				mbss->max_idle_ie_en, mbss->max_idle_period, mbss->max_idle_option);
		}

		if (mod_str)
			os_free_mem(mod_str);
	}

#ifdef CONFIG_APSTA_MIXED_SUPPORT
	/* apcli info*/
	MTWF_PRINT("\n\tStaNum=%d\n", pAd->MSTANum);
	MTWF_PRINT("\n\tSTA Idx\t\tPhy Mode\tIF_addr\t\t\tBand\n");

	for (i = 0; i < pAd->MSTANum; i++) {
		wdev = &pAd->StaCfg[i].wdev;
		mod_str = wmode_2_str(wdev->PhyMode);
		DbdcIdx = HcGetBandByWdev(wdev);

		MTWF_PRINT("\t%d\t%16s\t"MACSTR"\t%d\t(%s)\n",
			 i, mod_str, MAC2STR(wdev->if_addr), DbdcIdx,
			 RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev));

		if (mod_str)
			os_free_mem(mod_str);
	}
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

	return TRUE;
} /* End of Show_MbssInfo_Display_Proc */
#endif /* MBSS_SUPPORT */


#ifdef MT_MAC
INT
Show_TmrCalResult_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg
)
{
	if (pAd->pTmrCtrlStruct)
		MTWF_PRINT("TmrCalResult=0x%X\n", pAd->pTmrCtrlStruct->TmrCalResult);
	else
		MTWF_PRINT("pTmrCtrlStruct is NULL\n");

	return TRUE;
}

#ifdef DBG
INT Set_AP_DumpTime_Proc(
	IN      PRTMP_ADAPTER   pAd,
	IN      RTMP_STRING     *arg)
{
	int apidx = 0, i = 0;
	BSS_STRUCT *pMbss;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n\t%-10s\t%-10s\n", "PreTBTTTime", "TBTTTime");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%-10lu\t%-10lu\n", pAd->HandlePreInterruptTime,
			 pAd->HandleInterruptTime);

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		pMbss = &pAd->ApCfg.MBSSID[apidx];

		if (!BeaconTransmitRequired(pAd, &pMbss->wdev, TRUE))
			continue;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n%s:%d\tBcn_State:%d\t%-10s: %d\n", "Apidx", apidx,
				 pMbss->wdev.bcn_buf.bcn_state, "recover", pMbss->bcn_recovery_num);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\t%-10s\t%-10s\t%-10s\t%-10s\n", "WriteBcnRing", "BcnDmaDone",
				 "TXS_TSF", "TXS_SN");

		for (i = 0; i < MAX_TIME_RECORD; i++)
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Idx[%d]:\t%-10lu\t%-10lu\t%-10lu\t%-10lu\n", i,
					 pMbss->WriteBcnDoneTime[i], pMbss->BcnDmaDoneTime[i], pMbss->TXS_TSF[i], pMbss->TXS_SN[i]);
	}

	return TRUE;
}

INT Set_BcnStateCtrl_Proc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	UCHAR bcn_state;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR apIndex = pObj->ioctl_if;
	BSS_STRUCT *pMbss = NULL;

	bcn_state = os_str_tol(arg, 0, 10);

	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "chipcap is not HIF_MT\n");
		return FALSE;
	}

	if ((bcn_state < BCN_TX_IDLE) || (bcn_state > BCN_TX_DMA_DONE)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "bcn_state is incorrect!!\n");
		return FALSE;
	}

	pMbss = &pAd->ApCfg.MBSSID[apIndex];
	ASSERT(pMbss);

	if (pMbss == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pMbss == NULL!!\n");
		return FALSE;
	}

	if (pMbss->wdev.bcn_buf.bcn_state != BCN_TX_IDLE) {
		pMbss->wdev.bcn_buf.bcn_state = bcn_state;
	}

	return TRUE;
}
#endif

#ifdef MAC_REPEATER_SUPPORT
INT set_dump_rx_ba_scoreboard_proc(
       IN PRTMP_ADAPTER   pAd,
       IN RTMP_STRING     *arg)
{
	UCHAR i;
	struct wifi_dev *wdev = NULL;
	RX_TRACKING_T *pTracking = NULL;
	RX_TA_TID_SEQ_MAPPING *pTaTidSeqMapEntry = NULL;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if ((wdev == NULL) || (wdev->wdev_type != WDEV_TYPE_STA))
			continue;

		pTracking = &wdev->rx_tracking;
		pTaTidSeqMapEntry = &pTracking->LastRxWlanIdx;

		if (pTaTidSeqMapEntry->RxDWlanIdx != 0xff) {
			MTWF_PRINT("wdev idx:%d, wdev_type:%x ,TriggerNum = %d\n",
				wdev->wdev_idx, wdev->wdev_type, pTracking->TriggerNum);
		}
	}

	return TRUE;
}
#endif /* MAC_REPEATER_SUPPORT */

INT setApTmrEnableProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	LONG enable;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR apIndex = pObj->ioctl_if;
	BSS_STRUCT *pMbss = NULL;
	UINT32  value = 0;
	struct wifi_dev *wdev;
	UCHAR bw;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	enable = os_str_tol(arg, 0, 10);

	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "chip_cap is not HIF_MT\n");
		return FALSE;
	}

	if ((enable < 0) || (enable > 2)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "enable is incorrect!!\n");
		return FALSE;
	}

	pMbss = &pAd->ApCfg.MBSSID[apIndex];
	ASSERT(pMbss);

	if (pMbss == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pMbss == NULL!!\n");
		return FALSE;
	}

	wdev = &pMbss->wdev;
	bw = wlan_operate_get_bw(wdev);

	switch (enable) {
	case TMR_INITIATOR: {
		MTWF_PRINT("enable TMR report, as Initialiter\n");
		MAC_IO_READ32(pAd->hdev_ctrl, RMAC_TMR_PA, &value);
		value = value | BIT31;
		value = value & ~BIT30;
		MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_TMR_PA, value);
		cap->TmrEnable = 1;
		MtCmdTmrCal(pAd,
					enable,
					(WMODE_CAP_5G(wdev->PhyMode) ? _A_BAND : _G_BAND),
					bw,
					0,/* Ant 0 at present */
					TMR_INITIATOR);
	}
	break;

	case TMR_RESPONDER: {
		MTWF_PRINT("%s: enable TMR report, as Responser\n", __func__);
		MAC_IO_READ32(pAd->hdev_ctrl, RMAC_TMR_PA, &value);
		value = value | BIT31;
		value = value | BIT30;
		value = value | 0x34;/* Action frame register */
		MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_TMR_PA, value);
		cap->TmrEnable = 2;
		MtCmdTmrCal(pAd,
					enable,
					(WMODE_CAP_5G(wdev->PhyMode) ? _A_BAND : _G_BAND),
					bw,
					0,/* Ant 0 at present */
					TMR_RESPONDER);
	}
	break;

	case TMR_DISABLE:
	default: {
		MTWF_PRINT("%s: disable TMR report\n", __func__);
		MAC_IO_READ32(pAd->hdev_ctrl, RMAC_TMR_PA, &value);
		value = value & ~BIT31;
		MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_TMR_PA, value);
		cap->TmrEnable = FALSE;
		MtCmdTmrCal(pAd,
					enable,
					(WMODE_CAP_5G(wdev->PhyMode) ? _A_BAND : _G_BAND),
					bw,
					0,/* Ant 0 at present */
					TMR_DISABLE);
	}
	}

	return TRUE;
}

#endif /* MT_MAC */

/*
========================================================================
Routine Description:
	Driver Ioctl for AP.

Arguments:
	pAdSrc			- WLAN control block pointer
	wrq				- the IOCTL parameters
	cmd				- the command ID
	subcmd			- the sub-command ID
	pData			- the IOCTL private data pointer
	Data			- the IOCTL private data

Return Value:
	NDIS_STATUS_SUCCESS	- IOCTL OK
	Otherwise			- IOCTL fail

Note:
========================================================================
*/
INT RTMP_AP_IoctlHandle(
	IN	VOID					*pAdSrc,
	IN	RTMP_IOCTL_INPUT_STRUCT	 *wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT Status = NDIS_STATUS_SUCCESS;
	UINT	ifIndex;

	pAd->IoctlHandleFlag = TRUE;

	switch (cmd) {
	case CMD_RTPRIV_IOCTL_SET:
		Status = RTMPAPPrivIoctlSet(pAd, wrq);
		break;

#ifdef DYNAMIC_VLAN_SUPPORT
	case CMD_RTPRIV_IOCTL_SET_STA_VLAN:
		Status = Set_Sta_Vlan(pAd, pData);
		break;
#endif
#ifdef HOSTAPD_11R_SUPPORT
	case CMD_RTPRIV_IOCTL_SET_FT_PARAM:
		Status = Set_Ft_Param(pAd, pData);
		break;
#endif
	case CMD_RT_PRIV_IOCTL:
		if (subcmd & OID_GET_SET_TOGGLE)
			Status = RTMPAPSetInformation(pAd, wrq,  (INT)subcmd);
		else {
#ifdef LLTD_SUPPORT

			if (subcmd == RT_OID_GET_PHY_MODE) {
				if (pData != NULL) {
					UINT modetmp = 0;

					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Query::Get phy mode (%02X)\n",
							 pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.PhyMode);
					modetmp = (UINT) pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.PhyMode;
					wrq->u.data.length = 1;

					/**(ULONG *)pData = (ULONG)pAd->ApCfg.MBSS[MAIN_MBSSID].wdev.PhyMode; */
					if (copy_to_user(pData, &modetmp, wrq->u.data.length))
						Status = -EFAULT;
				} else
					Status = -EFAULT;
			} else
#endif /* LLTD_SUPPORT */
				Status = RTMPAPQueryInformation(pAd, wrq, (INT)subcmd);
		}

		break;

	case CMD_RTPRIV_IOCTL_SHOW:
		Status = RTMPAPPrivIoctlShow(pAd, wrq);
		break;
#ifdef WSC_AP_SUPPORT

	case CMD_RTPRIV_IOCTL_SET_WSCOOB:
		RTMPIoctlSetWSCOOB(pAd);
		break;
#endif/*WSC_AP_SUPPORT*/

	case CMD_RTPRIV_IOCTL_GET_MAC_TABLE:
		RTMPIoctlGetMacTable(pAd, wrq);
		break;
#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)

	case CMD_RTPRIV_IOCTL_GSITESURVEY:
		RTMPIoctlGetSiteSurvey(pAd, wrq);
		break;
#endif /* AP_SCAN_SUPPORT */

	case CMD_RTPRIV_IOCTL_STATISTICS:
		RTMPIoctlStatistics(pAd, wrq);
		break;

	case CMD_MTPRIV_IOCTL_RD:
		RTMPIoctlRvRDebug(pAd, wrq);
		break;

	case CMD_RTPRIV_IOCTL_RX_STATISTICS:
		Status = RTMPIoctlRXStatistics(pAd, wrq);
		break;
#ifdef WSC_AP_SUPPORT

	case CMD_RTPRIV_IOCTL_WSC_PROFILE:
		RTMPIoctlWscProfile(pAd, wrq);
		break;
#endif /* WSC_AP_SUPPORT */
#ifdef DOT11_N_SUPPORT

	case CMD_RTPRIV_IOCTL_QUERY_BATABLE:
		RTMPIoctlQueryBaTable(pAd, wrq);
		break;
#endif /* DOT11_N_SUPPORT */

	case CMD_RTPRIV_IOCTL_E2P:
		RTMPAPIoctlE2PROM(pAd, wrq);
		break;
#ifdef DBG

	case CMD_RTPRIV_IOCTL_BBP:
		RTMPAPIoctlBBP(pAd, wrq);
		break;

	case CMD_RTPRIV_IOCTL_MAC:
		RTMPIoctlMAC(pAd, wrq);
		break;
#ifdef RTMP_RF_RW_SUPPORT

	case CMD_RTPRIV_IOCTL_RF:
		RTMPAPIoctlRF(pAd, wrq);
		break;
#endif /* RTMP_RF_RW_SUPPORT */
#endif /* DBG */

	case CMD_RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT:
		RTMPIoctlGetMacTableStaInfo(pAd, wrq);
		break;

	case CMD_RTPRIV_IOCTL_GET_DRIVER_INFO:
		RTMPIoctlGetDriverInfo(pAd, wrq);
		break;

	case CMD_RTPRIV_IOCTL_AP_SIOCGIFHWADDR:
		ifIndex = pObj->ioctl_if;
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
			Status = NDIS_STATUS_FAILURE;
			goto end;
		}

		NdisCopyMemory((RTMP_STRING *) wrq->u.name,
			(RTMP_STRING *) pAd->ApCfg.MBSSID[ifIndex].wdev.bssid, 6);
		break;

	case CMD_RTPRIV_IOCTL_AP_SIOCGIWESSID: {
		RT_CMD_AP_IOCTL_SSID *pSSID = (RT_CMD_AP_IOCTL_SSID *)pData;
#ifdef APCLI_SUPPORT

		if (pSSID->priv_flags == INT_APCLI) {
			ifIndex = pObj->ioctl_if;
			if (ifIndex >= MAX_MULTI_STA) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error station index\n");
				Status = NDIS_STATUS_FAILURE;
				goto end;
			}

			if (pAd->StaCfg[ifIndex].ApcliInfStat.Valid == TRUE) {
				pSSID->length = pAd->StaCfg[ifIndex].SsidLen;
				pSSID->pSsidStr = (char *)&pAd->StaCfg[ifIndex].Ssid;
			} else {
				pSSID->length = 0;
				pSSID->pSsidStr = NULL;
			}
		} else
#endif /* APCLI_SUPPORT */
		{
			ifIndex = pSSID->apidx;
			if (!VALID_MBSS(pAd, ifIndex)){
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
				Status = NDIS_STATUS_FAILURE;
				goto end;
			}

			pSSID->length = pAd->ApCfg.MBSSID[ifIndex].SsidLen;
			pSSID->pSsidStr = (char *)pAd->ApCfg.MBSSID[ifIndex].Ssid;
		}
	}
	break;
#ifdef MBSS_SUPPORT

	case CMD_RTPRIV_IOCTL_MBSS_BEACON_UPDATE:
		/* Carter, TODO. check this oid. */
		UpdateBeaconHandler(
			pAd,
			get_default_wdev(pAd),
			BCN_UPDATE_PRETBTT);
		break;

	case CMD_RTPRIV_IOCTL_MBSS_INIT:
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-> CMD_RTPRIV_IOCTL_MBSS_INIT\n");
		MBSS_Init(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_MBSS_REMOVE:
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-> CMD_RTPRIV_IOCTL_MBSS_REMOVE\n");
		MBSS_Remove(pAd);
		break;
#ifdef MT_MAC

	case CMD_RTPRIV_IOCTL_MBSS_CR_ENABLE:
		if (ext_mbss_hw_cr_enable(pData) != 0) {
			Status = NDIS_STATUS_FAILURE;
			goto end;
		}
		break;

	case CMD_RTPRIV_IOCTL_MBSS_CR_DISABLE:
		if (ext_mbss_hw_cr_disable(pData) != 0) {
			Status = NDIS_STATUS_FAILURE;
			goto end;
		}
		break;
#endif /* MT_MAC */
#endif /* MBSS_SUPPORT */

	case CMD_RTPRIV_IOCTL_WSC_INIT: {
#ifdef APCLI_SUPPORT
#ifdef WSC_AP_SUPPORT
#ifdef WSC_V2_SUPPORT
		PWSC_V2_INFO	pWscV2Info;
#endif /* WSC_V2_SUPPORT */
		STA_ADMIN_CONFIG *pApCliEntry = (STA_ADMIN_CONFIG *)pData;
		WSC_CTRL *wsc_ctrl;

		wsc_ctrl = &pApCliEntry->wdev.WscControl;
		WscGenerateUUID(pAd, &wsc_ctrl->Wsc_Uuid_E[0],
						&wsc_ctrl->Wsc_Uuid_Str[0], 0, FALSE, TRUE);
		wsc_ctrl->bWscFragment = FALSE;
		wsc_ctrl->WscFragSize = 128;
		wsc_ctrl->WscRxBufLen = 0;
		wsc_ctrl->pWscRxBuf = NULL;
		os_alloc_mem(pAd, &wsc_ctrl->pWscRxBuf, MAX_MGMT_PKT_LEN);

		if (wsc_ctrl->pWscRxBuf)
			NdisZeroMemory(wsc_ctrl->pWscRxBuf, MAX_MGMT_PKT_LEN);

		wsc_ctrl->WscTxBufLen = 0;
		wsc_ctrl->pWscTxBuf = NULL;
		os_alloc_mem(pAd, &wsc_ctrl->pWscTxBuf, MAX_MGMT_PKT_LEN);

		if (wsc_ctrl->pWscTxBuf)
			NdisZeroMemory(wsc_ctrl->pWscTxBuf, MAX_MGMT_PKT_LEN);

		initList(&wsc_ctrl->WscPeerList);
		NdisAllocateSpinLock(pAd, &wsc_ctrl->WscPeerListSemLock);
		wsc_ctrl->PinAttackCount = 0;
		wsc_ctrl->bSetupLock = FALSE;
#ifdef WSC_V2_SUPPORT
		pWscV2Info = &wsc_ctrl->WscV2Info;
		pWscV2Info->bWpsEnable = TRUE;
		pWscV2Info->ExtraTlv.TlvLen = 0;
		pWscV2Info->ExtraTlv.TlvTag = 0;
		pWscV2Info->ExtraTlv.pTlvData = NULL;
		pWscV2Info->ExtraTlv.TlvType = TLV_ASCII;
		pWscV2Info->bEnableWpsV2 = TRUE;
#endif /* WSC_V2_SUPPORT */
		WscInit(pAd, TRUE, Data);
#endif /* WSC_AP_SUPPORT */
#endif /* APCLI_SUPPORT */
	}
	break;
#ifdef APCLI_SUPPORT

	case CMD_RTPRIV_IOCTL_APC_UP:
		ApCliIfUp(pAd);
		break;

	case CMD_RTPRIV_IOCTL_APC_DISCONNECT:
	{
		PSTA_ADMIN_CONFIG apcli_entry = &pAd->StaCfg[Data];

		cntl_disconnect_request(&apcli_entry->wdev,
								CNTL_DISASSOC,
								apcli_entry->Bssid,
								REASON_DISASSOC_STA_LEAVING);
	}

		break;

#endif /* APCLI_SUPPORT */

	case CMD_RTPRIV_IOCTL_PREPARE: {
		RT_CMD_AP_IOCTL_CONFIG *pConfig = (RT_CMD_AP_IOCTL_CONFIG *)pData;

		pConfig->Status = RTMP_AP_IoctlPrepare(pAd, pData);

		if (pConfig->Status != 0) {
			Status = NDIS_STATUS_FAILURE;
			goto end;
		}
	}
	break;

	case CMD_RTPRIV_IOCTL_AP_SIOCGIWAP: {
		UCHAR *pBssidDest = (UCHAR *)pData;
		PCHAR pBssidStr;

		ifIndex = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
		if (Data == INT_APCLI) {
			if (ifIndex >= MAX_MULTI_STA) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
				Status = NDIS_STATUS_FAILURE;
				goto end;
			}

			if (pAd->StaCfg[ifIndex].ApcliInfStat.Valid == TRUE)
				pBssidStr = (PCHAR)&APCLI_ROOT_BSSID_GET(pAd, pAd->StaCfg[ifIndex].MacTabWCID);
			else
				pBssidStr = NULL;
		} else
#endif /* APCLI_SUPPORT */
		{
			if (!VALID_MBSS(pAd, ifIndex)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
				Status = NDIS_STATUS_FAILURE;
				goto end;
			}

			pBssidStr = (PCHAR) &pAd->ApCfg.MBSSID[ifIndex].wdev.bssid[0];
		}

		if (pBssidStr != NULL) {
			memcpy(pBssidDest, pBssidStr, ETH_ALEN);
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IOCTL::SIOCGIWAP(="MACSTR")\n",
					 MAC2STR(pBssidStr));
		} else
			memset(pBssidDest, 0, ETH_ALEN);
	}
	break;

	case CMD_RTPRIV_IOCTL_AP_SIOCGIWRATEQ:
		/* handle for SIOCGIWRATEQ */
	{
		RT_CMD_IOCTL_RATE *pRate = (RT_CMD_IOCTL_RATE *)pData;
		HTTRANSMIT_SETTING HtPhyMode;

		ifIndex = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
			if (pRate->priv_flags == INT_APCLI) {
				if (ifIndex >= MAX_MULTI_STA) {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"error station index\n");
					Status = NDIS_STATUS_FAILURE;
					goto end;
				}

				HtPhyMode = pAd->StaCfg[ifIndex].wdev.HTPhyMode;
			}
			else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
				if (pRate->priv_flags == INT_WDS) {
					if (ifIndex >= MAX_WDS_ENTRY) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"error WDS index\n");
						Status = NDIS_STATUS_FAILURE;
						goto end;
					}

					HtPhyMode = pAd->WdsTab.WdsEntry[ifIndex].wdev.HTPhyMode;
				} else
#endif /* WDS_SUPPORT */
				{
					if (!VALID_MBSS(pAd, ifIndex)) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error AP index\n");
						Status = NDIS_STATUS_FAILURE;
						goto end;
					}

					HtPhyMode = pAd->ApCfg.MBSSID[ifIndex].wdev.HTPhyMode;
				}

		RtmpDrvMaxRateGet(pAd, HtPhyMode.field.MODE, HtPhyMode.field.ShortGI,
						  HtPhyMode.field.BW, HtPhyMode.field.MCS,
						  pAd->Antenna.field.TxPath,
						  &pRate->BitRate);
	}
	break;
#ifdef RT_CFG80211_SUPPORT
    case CMD_RTPRIV_IOCTL_MAIN_OPEN:
	{
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

		wdev->bAllowBeaconing = TRUE;

		if (VIRTUAL_IF_NUM(pAd) != 0) {
			wifi_sys_open(wdev);
			pAd->ApCfg.MBSSID[MAIN_MBSSID].mbss_idx = MAIN_MBSSID;
			APStartUpForMbss(pAd, &pAd->ApCfg.MBSSID[MAIN_MBSSID]);
			APStartRekeyTimer(pAd, wdev);
		}
	}
	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"AP interface up for ra_%x\n", MAIN_MBSSID);
		break;
#endif
#ifdef WIFI_DIAG
	case CMD_RTPRIV_IOCTL_GET_PROCESS_INFO:
		diag_get_process_info(pAd, wrq);
		break;
#endif

	case CMD_RTPRIV_IOCTL_PHY_STATE:
		Status = RTMPPhyState(pAd, wrq);
		break;

	default:
		Status = RTMP_COM_IoctlHandle(pAd, wrq, cmd, subcmd, pData, Data);
		break;
	}

end:
	pAd->IoctlHandleFlag = FALSE;
	return Status;
}

#ifdef P2P_SUPPORT
INT Set_P2p_OpMode_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	UINT OpMode;
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	OpMode = os_str_tol(arg, 0, 16);

	if (OpMode == OPMODE_AP) {
		if (P2P_CLI_ON(pAd))
			P2P_CliStop(pAd);

		if ((!P2P_GO_ON(pAd)) || (P2P_GO_ON(pAd))) {
			P2PCfgInit(pAd);
			P2P_GoStartUp(pAd, MAIN_MBSSID);
		}
	} else if (OpMode == OPMODE_APSTA) {
		if (P2P_GO_ON(pAd))
			P2P_GoStop(pAd);

		if ((!P2P_CLI_ON(pAd)) || (P2P_CLI_ON(pAd))) {
			P2PCfgInit(pAd);
			P2P_CliStartUp(pAd);
			AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod[DBDC_BAND0]);
		}
	} else {
		if (P2P_CLI_ON(pAd))
			P2P_CliStop(pAd);
		else if (P2P_GO_ON(pAd)) {
			P2P_GoStop(pAd);

			if (INFRA_ON(pAd))
				AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod[DBDC_BAND0]);
		}

		P2PCfgInit(pAd);
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(p2p%d) (OpMode = %d)\n", pObj->ioctl_if,
			 OpMode);
	return TRUE;
}

INT Set_P2pCli_Enable_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	UINT Enable;
	POS_COOKIE pObj;
	UCHAR ifIndex;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	Enable = os_str_tol(arg, 0, 16);
	pAd->StaCfg[ifIndex].ApcliInfStat.Enable = (Enable > 0) ? TRUE : FALSE;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(p2p%d) (enable = %d)\n", ifIndex,
			 pAd->StaCfg[ifIndex].ApcliInfStat.Enable);
	ApCliIfDown(pAd);
	return TRUE;
}

INT Set_P2pCli_Ssid_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR ifIndex;
	BOOLEAN apcliEn;
	INT success = FALSE;
	/*UCHAR keyMaterial[40]; */
	UCHAR PskKey[100];

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	if (strlen(arg) <= MAX_LEN_OF_SSID) {
		apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;

		/* bring apcli interface down first */
		if (apcliEn == TRUE) {
			pAd->StaCfg[ifIndex].ApcliInfStat.Enable = FALSE;
			ApCliIfDown(pAd);
		}

		NdisZeroMemory(pAd->StaCfg[ifIndex].CfgSsid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->StaCfg[ifIndex].CfgSsid, arg, strlen(arg));
		pAd->StaCfg[ifIndex].CfgSsidLen = (UCHAR)strlen(arg);
		NdisZeroMemory(pAd->StaCfg[ifIndex].WscControl.WscSsid.Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->StaCfg[ifIndex].WscControl.WscSsid.Ssid, arg, strlen(arg));
		pAd->StaCfg[ifIndex].WscControl.WscSsid.SsidLength = (UCHAR)strlen(arg);
		success = TRUE;

		/* Upadte PMK and restart WPAPSK state machine for ApCli link */
		if (((pAd->StaCfg[ifIndex].wdev.AuthMode == Ndis802_11AuthModeWPAPSK) ||
			 (pAd->StaCfg[ifIndex].wdev.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
			pAd->StaCfg[ifIndex].PSKLen > 0) {
			NdisZeroMemory(PskKey, 100);
			NdisMoveMemory(PskKey, pAd->StaCfg[ifIndex].PSK, pAd->StaCfg[ifIndex].PSKLen);
			SetWPAPSKKey(pAd, (RTMP_STRING *)PskKey,
						 pAd->StaCfg[ifIndex].PSKLen,
						 (PUCHAR)pAd->StaCfg[ifIndex].CfgSsid,
						 pAd->StaCfg[ifIndex].CfgSsidLen,
						 pAd->StaCfg[ifIndex].PMK);
		}

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(p2p%d) (Len=%d,Ssid=%s)\n", ifIndex,
				 pAd->StaCfg[ifIndex].CfgSsidLen, pAd->StaCfg[ifIndex].CfgSsid);
		pAd->StaCfg[ifIndex].ApcliInfStat.Enable = apcliEn;
	} else
		success = FALSE;

	return success;
}

INT Set_P2pCli_Bssid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	RTMP_STRING *value;
	UCHAR ifIndex;
	BOOLEAN apcliEn;
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	apcliEn = pAd->StaCfg[ifIndex].ApcliInfStat.Enable;

	/* bring apcli interface down first */
	if (apcliEn == TRUE) {
		pAd->StaCfg[ifIndex].ApcliInfStat.Enable = FALSE;
		ApCliIfDown(pAd);
	}

	NdisZeroMemory(pAd->StaCfg[ifIndex].CfgApCliBssid, MAC_ADDR_LEN);

	if (strlen(arg) == 17) { /* Mac address acceptable format 01:02:03:04:05:06 length 17 */
		for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
				return FALSE;  /* Invalid */

			AtoH(value, &pAd->StaCfg[ifIndex].CfgApCliBssid[i], 1);
		}

		if (i != 6)
			return FALSE;  /* Invalid */
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "(%2X:%2X:%2X:%2X:%2X:%2X)\n",
			 pAd->StaCfg[ifIndex].CfgApCliBssid[0],
			 pAd->StaCfg[ifIndex].CfgApCliBssid[1],
			 pAd->StaCfg[ifIndex].CfgApCliBssid[2],
			 pAd->StaCfg[ifIndex].CfgApCliBssid[3],
			 pAd->StaCfg[ifIndex].CfgApCliBssid[4],
			 pAd->StaCfg[ifIndex].CfgApCliBssid[5]);
	pAd->StaCfg[ifIndex].ApcliInfStat.Enable = apcliEn;
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set ApCli-IF Authentication mode
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_P2pCli_AuthMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG       i;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		ifIndex;

	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	if (rtstrcasecmp(arg, "WEPAUTO") == TRUE)
		pAd->StaCfg[ifIndex].AuthMode = Ndis802_11AuthModeAutoSwitch;
	else if (rtstrcasecmp(arg, "SHARED") == TRUE)
		pAd->StaCfg[ifIndex].AuthMode = Ndis802_11AuthModeShared;
	else if (rtstrcasecmp(arg, "WPAPSK") == TRUE)
		pAd->StaCfg[ifIndex].AuthMode = Ndis802_11AuthModeWPAPSK;
	else if (rtstrcasecmp(arg, "WPA2PSK") == TRUE)
		pAd->StaCfg[ifIndex].AuthMode = Ndis802_11AuthModeWPA2PSK;
	else
		pAd->StaCfg[ifIndex].AuthMode = Ndis802_11AuthModeOpen;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		if (IS_ENTRY_PEER_AP(&pAd->MacTab.Content[i]))
			pAd->MacTab.tr_entry[i].PortSecured  = WPA_802_1X_PORT_NOT_SECURED;
	}

	RTMPMakeRSNIE(pAd, pAd->StaCfg[ifIndex].AuthMode, pAd->StaCfg[ifIndex].WepStatus,
				  (ifIndex + MIN_NET_DEVICE_FOR_APCLI));
	pAd->StaCfg[ifIndex].DefaultKeyId  = 0;

	if (pAd->StaCfg[ifIndex].AuthMode >= Ndis802_11AuthModeWPA)
		pAd->StaCfg[ifIndex].DefaultKeyId = 1;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(p2p%d) (AuthMode=%d)\n", ifIndex,
			 pAd->StaCfg[ifIndex].AuthMode);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set ApCli-IF Encryption Type
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_P2pCli_EncrypType_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR		ifIndex;
	PSTA_ADMIN_CONFIG   pApCliEntry = NULL;

	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	pApCliEntry->WepStatus = Ndis802_11WEPDisabled;

	if (rtstrcasecmp(arg, "WEP") == TRUE) {
		if (pApCliEntry->AuthMode < Ndis802_11AuthModeWPA)
			pApCliEntry->WepStatus = Ndis802_11WEPEnabled;
	} else if (rtstrcasecmp(arg, "TKIP") == TRUE) {
		if (pApCliEntry->AuthMode >= Ndis802_11AuthModeWPA)
			pApCliEntry->WepStatus = Ndis802_11TKIPEnable;
	} else if (rtstrcasecmp(arg, "AES") == TRUE) {
		if (pApCliEntry->AuthMode >= Ndis802_11AuthModeWPA)
			pApCliEntry->WepStatus = Ndis802_11AESEnable;
	} else
		pApCliEntry->WepStatus = Ndis802_11WEPDisabled;

	pApCliEntry->PairCipher     = pApCliEntry->WepStatus;
	pApCliEntry->GroupCipher    = pApCliEntry->WepStatus;
	pApCliEntry->bMixCipher		= FALSE;

	if (pApCliEntry->WepStatus >= Ndis802_11TKIPEnable)
		pApCliEntry->DefaultKeyId = 1;

	RTMPMakeRSNIE(pAd, pApCliEntry->AuthMode, pApCliEntry->WepStatus, (ifIndex + MIN_NET_DEVICE_FOR_APCLI));
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(p2p%d) (EncrypType=%d)\n",
			 ifIndex, pApCliEntry->WepStatus);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set Default Key ID
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_P2pCli_DefaultKeyID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG			KeyIdx;
	POS_COOKIE		pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR			ifIndex;
	PSTA_ADMIN_CONFIG   pApCliEntry = NULL;

	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	KeyIdx = os_str_tol(arg, 0, 10);

	if ((KeyIdx >= 1) && (KeyIdx <= 4))
		pApCliEntry->DefaultKeyId = (UCHAR) (KeyIdx - 1);
	else
		return FALSE;  /* Invalid argument  */

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "IF(p2p%d) (DefaultKeyID(0~3)=%d)\n", ifIndex, pApCliEntry->DefaultKeyId);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set WPA PSK key for ApCli link

    Arguments:
	pAdapter            Pointer to our adapter
	arg                 WPA pre-shared key string

    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_P2pCli_WPAPSK_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR ifIndex;
	POS_COOKIE pObj;
	PSTA_ADMIN_CONFIG   pApCliEntry = NULL;
	INT retval;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(p2p%d) (WPAPSK=%s)\n", ifIndex, arg);
	retval = SetWPAPSKKey(pAd, arg, strlen(arg), (PUCHAR)pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen, pApCliEntry->PMK);

	if (retval == FALSE)
		return FALSE;

	NdisMoveMemory(pApCliEntry->PSK, arg, strlen(arg));
	pApCliEntry->PSKLen = strlen(arg);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Set WEP KEY1 for ApCli-IF
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_P2pCli_Key1_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE		pObj = (POS_COOKIE) pAd->OS_Cookie;
	PSTA_ADMIN_CONFIG	pApCliEntry = NULL;
	UCHAR			ifIndex;
	INT				retVal;

	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	retVal = RT_CfgSetWepKey(pAd, arg, &pApCliEntry->SharedKey[0], 0);

	if (retVal == TRUE)
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(p2p%d) (Key1=%s) success!\n", ifIndex,
				 arg);

	return retVal;
}

/*
    ==========================================================================
    Description:
	Set WEP KEY2 for ApCli-IF
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_P2pCli_Key2_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE		pObj;
	PSTA_ADMIN_CONFIG	pApCliEntry = NULL;
	UCHAR			ifIndex;
	INT				retVal;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	retVal = RT_CfgSetWepKey(pAd, arg, &pApCliEntry->SharedKey[1], 1);

	if (retVal == TRUE)
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(p2p%d) (Key2=%s) success!\n", ifIndex,
				 arg);

	return retVal;
}

/*
    ==========================================================================
    Description:
	Set WEP KEY3 for ApCli-IF
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_P2pCli_Key3_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE		pObj;
	PSTA_ADMIN_CONFIG	pApCliEntry = NULL;
	UCHAR			ifIndex;
	INT				retVal;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	retVal = RT_CfgSetWepKey(pAd, arg, &pApCliEntry->SharedKey[2], 2);

	if (retVal == TRUE)
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(p2p%d) (Key3=%s) success!\n", ifIndex,
				 arg);

	return retVal;
}

/*
    ==========================================================================
    Description:
	Set WEP KEY4 for ApCli-IF
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_P2pCli_Key4_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE		pObj;
	PSTA_ADMIN_CONFIG	pApCliEntry = NULL;
	UCHAR			ifIndex;
	INT				retVal;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	retVal = RT_CfgSetWepKey(pAd, arg, &pApCliEntry->SharedKey[3], 3);

	if (retVal == TRUE)
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IF(p2p%d) (Key4=%s) success!\n", ifIndex,
				 arg);

	return retVal;
}

INT Set_P2pCli_TxMode_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE		pObj;
	UCHAR			ifIndex;
	PSTA_ADMIN_CONFIG	pApCliEntry = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	pApCliEntry->DesiredTransmitSetting.field.FixedTxMode = RT_CfgSetFixedTxPhyMode(arg);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(p2p%d) TxMode=%d\n", ifIndex,
			 pApCliEntry->DesiredTransmitSetting.field.FixedTxMode);
	return TRUE;
}

INT Set_P2pCli_TxMcs_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	POS_COOKIE		pObj;
	UCHAR			ifIndex;
	PSTA_ADMIN_CONFIG	pApCliEntry = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	ifIndex = pObj->ioctl_if;
	pApCliEntry = &pAd->StaCfg[ifIndex];
	pApCliEntry->DesiredTransmitSetting.field.MCS =
		RT_CfgSetTxMCSProc(arg, &pApCliEntry->bAutoTxRateSwitch);

	if (pApCliEntry->DesiredTransmitSetting.field.MCS == MCS_AUTO)
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(p2p%d) TxMcsc = AUTO\n", ifIndex);
	else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "I/F(p2p%d) TxMcs = %d\n", ifIndex,
				 pApCliEntry->DesiredTransmitSetting.field.MCS);
	}

	return TRUE;
}

#ifdef WSC_AP_SUPPORT
INT Set_P2pCli_WscSsid_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE		pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR			ifIndex = pObj->ioctl_if;
	PWSC_CTRL	    pWscControl = &pAd->StaCfg[ifIndex].WscControl;

	if (pObj->ioctl_if_type != INT_P2P)
		return FALSE;

	NdisZeroMemory(&pWscControl->WscSsid, sizeof(NDIS_802_11_SSID));

	if ((strlen(arg) > 0) && (strlen(arg) <= MAX_LEN_OF_SSID)) {
		NdisMoveMemory(pWscControl->WscSsid.Ssid, arg, strlen(arg));
		pWscControl->WscSsid.SsidLength = strlen(arg);
		NdisZeroMemory(pAd->StaCfg[ifIndex].CfgSsid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->StaCfg[ifIndex].CfgSsid, arg, strlen(arg));
		pAd->StaCfg[ifIndex].CfgSsidLen = (UCHAR)strlen(arg);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "I/F(p2p%d) (Select SsidLen=%d,Ssid=%s)\n", ifIndex,
				  pWscControl->WscSsid.SsidLength, pWscControl->WscSsid.Ssid);
	} else
		return FALSE;	/* Invalid argument  */

	return TRUE;
}
#endif /* WSC_AP_SUPPORT */
#endif /* P2P_SUPPORT */

#ifdef DYNAMIC_VGA_SUPPORT
INT Set_DyncVgaEnable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT Enable;

	Enable = os_str_tol(arg, 0, 10);
	pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable = (Enable > 0) ? TRUE : FALSE;

	if (pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable == TRUE)
		dynamic_vga_enable(pAd);
	else
		dynamic_vga_disable(pAd);

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(enable = %d)\n", pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable);
	return TRUE;
}

INT set_false_cca_hi_th(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 val = os_str_tol(arg, 0, 10);

	pAd->CommonCfg.lna_vga_ctl.nFalseCCATh = (val <= 0) ? 800 : val;
	MTWF_PRINT("%s::(false cca high threshould = %d)\n",
			 __func__, pAd->CommonCfg.lna_vga_ctl.nFalseCCATh);
	return TRUE;
}

INT set_false_cca_low_th(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	INT32 val = os_str_tol(arg, 0, 10);

	pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh = (val <= 0) ? 10 : val;
	MTWF_PRINT("%s::(false cca low threshould = %d)\n",
			 __func__, pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh);
	return TRUE;
}
#endif /* DYNAMIC_VGA_SUPPORT */

#ifdef CONFIG_HOTSPOT
INT Send_ANQP_Req_For_Test(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	UCHAR *Buf, *buf_anqp, *Pos;
	GAS_FRAME *GASFrame;
	UINT32 FrameLen = 0;
	UINT16 tmpLen = 0, tmp = 0; /* ,i; */
	static UINT32 Token;
	UINT32 anqp_req_len = 0;
	struct anqp_frame *anqp_req;
	UCHAR PeerMACAddr[MAC_ADDR_LEN];
	ULONG choice;
	RTMP_STRING *Value, ChanStr[4], *mac_str;
	INT32 Octet;
	INT ChanFreq, Channel;

	Value = rstrtok(arg, ",");
	if (Value == NULL)
		return FALSE;

	choice = os_str_tol(Value, 0, 10);

	Value = rstrtok(NULL, ",");
	if (Value == NULL)
		return FALSE;

	ChanFreq = os_str_tol(Value, 0, 10);
	RTMP_MapKHZ2ChannelID(ChanFreq, &Channel);
	sprintf(ChanStr, "%d", Channel);

	Value = rstrtok(NULL, ",");
	if (Value == NULL || strlen(Value) != 17)
		return FALSE;

	mac_str = Value;
	NdisZeroMemory(PeerMACAddr, MAC_ADDR_LEN);

	for (Octet = 0, Value = rstrtok(mac_str, ":"); Value; Value = rstrtok(NULL, ":")) {
		if ((strlen(Value) != 2) || (!isxdigit(*Value)) || (!isxdigit(*(Value + 1))))
			return FALSE;
		AtoH(Value, &PeerMACAddr[Octet++], 1);
	}

	if (Octet != MAC_ADDR_LEN)
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"choice %ld, ChanFreq %d, Channel %d, PeerMACAddr "MACSTR"\n",
		 choice, ChanFreq, Channel, MAC2STR(PeerMACAddr));

	Set_Channel_Proc(pAd, ChanStr);

	/* query_ap_geospatial_location  , query_ap_civic_location ,query_ap_location_public_uri */
	if (choice == 1) {
		os_alloc_mem(NULL, (UCHAR **)&buf_anqp, sizeof(*anqp_req) + 2);
		anqp_req = (struct anqp_frame *)buf_anqp;
		anqp_req->info_id = cpu2le16(ANQP_QUERY_LIST);
		anqp_req_len += 2;
		anqp_req->length = cpu2le16(2);
		anqp_req_len += 2;
		Pos = anqp_req->variable;
		tmp = cpu2le16(ROAMING_CONSORTIUM_LIST);
		/* tmp = cpu2le16(AP_GEOSPATIAL_LOCATION); */
		NdisMoveMemory(Pos, &tmp, 2);
		Pos += 2;
		/*
		tmp = cpu2le16(AP_CIVIC_LOCATION);
		NdisMoveMemory(Pos, &tmp, 2);
		Pos += 2;

		tmp = cpu2le16(AP_LOCATION_PUBLIC_IDENTIFIER_URI);
		NdisMoveMemory(Pos, &tmp, 2);
		Pos += 2;

		anqp_req_len += 6;
		*/
		anqp_req_len += 2;
	} else {
		os_alloc_mem(NULL, (UCHAR **)&buf_anqp, sizeof(*anqp_req) + 6);
		anqp_req = (struct anqp_frame *)buf_anqp;
		anqp_req->info_id = cpu2le16(ANQP_QUERY_LIST);
		anqp_req_len += 2;
		anqp_req->length = cpu2le16(6);
		anqp_req_len += 2;
		Pos = anqp_req->variable;
		tmp = cpu2le16(AP_GEOSPATIAL_LOCATION);
		NdisMoveMemory(Pos, &tmp, 2);
		Pos += 2;
		tmp = cpu2le16(AP_CIVIC_LOCATION);
		NdisMoveMemory(Pos, &tmp, 2);
		Pos += 2;
		tmp = cpu2le16(AP_LOCATION_PUBLIC_IDENTIFIER_URI);
		NdisMoveMemory(Pos, &tmp, 2);
		Pos += 2;
		anqp_req_len += 6;
	}

	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*GASFrame) + anqp_req_len);

	if (!Buf)
		goto error0;

	NdisZeroMemory(Buf, sizeof(*GASFrame) + anqp_req_len);
	GASFrame = (GAS_FRAME *)Buf;
	ActHeaderInit(pAd, &GASFrame->Hdr, PeerMACAddr, pAd->CurrentAddress,
				  PeerMACAddr);
	FrameLen += sizeof(HEADER_802_11);
	GASFrame->Category = CATEGORY_PUBLIC;
	GASFrame->u.GAS_INIT_REQ.Action = ACTION_GAS_INIT_REQ;
	GASFrame->u.GAS_INIT_REQ.DialogToken = Token++; /* Event->u.GAS_REQ_DATA.DialogToken; */
	FrameLen += 3;
	Pos = GASFrame->u.GAS_INIT_REQ.Variable;
	*Pos++ = IE_ADVERTISEMENT_PROTO;
	*Pos++ = 2; /* Length field */
	*Pos++ = 0; /* Query response info field */
	*Pos++ = ACCESS_NETWORK_QUERY_PROTOCOL; /* Advertisement Protocol ID field */
	tmpLen = cpu2le16(anqp_req_len);
	NdisMoveMemory(Pos, &tmpLen, 2);
	Pos += 2;
	FrameLen += 6;
	NdisMoveMemory(Pos, buf_anqp, anqp_req_len);
	FrameLen += anqp_req_len;
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
		"Location Anqp Req to "MACSTR"\n", MAC2STR(PeerMACAddr));
	MiniportMMRequest(pAd, 0, Buf, FrameLen);
	hex_dump_with_lvl("anqp", Buf, FrameLen, DBG_LVL_DEBUG);
	os_free_mem(Buf);
	os_free_mem(buf_anqp);
	return TRUE;
error0:
	os_free_mem(buf_anqp);
	return FALSE;
}
#endif

#ifdef WAPP_SUPPORT
INT Send_ANQP_Rsp(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *PeerMACAddr,
	IN RTMP_STRING *ANQPRsp,
	IN UINT32 ANQPRspLen)
{
	UCHAR *Buf;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	PGAS_CTRL pGASCtrl = &pAd->ApCfg.MBSSID[apidx].GASCtrl;
	GAS_EVENT_DATA *Event;
	GAS_PEER_ENTRY *GASPeerEntry;
	GAS_QUERY_RSP_FRAGMENT *GASQueryRspFrag, *Tmp;
	UINT32 Len = 0, i, QueryRspOffset = 0;
	BOOLEAN Cancelled;
	BOOLEAN IsFound = FALSE;

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	/* Cancel PostReply timer after receiving daemon response */
	DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, PeerMACAddr)) {
			if (GASPeerEntry->PostReplyTimerRunning) {
				RTMPCancelTimer(&GASPeerEntry->PostReplyTimer, &Cancelled);
				GASPeerEntry->PostReplyTimerRunning = FALSE;
			}

			break;
		}
	}
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event) + ANQPRspLen);

	if (!Buf) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not available memory\n");
		goto error0;
	}

	NdisZeroMemory(Buf, sizeof(*Event) + ANQPRspLen);
	Event = (GAS_EVENT_DATA *)Buf;
	Event->ControlIndex = apidx;
	Len += 1;
	NdisMoveMemory(Event->PeerMACAddr, PeerMACAddr, MAC_ADDR_LEN);
	Len += MAC_ADDR_LEN;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "ANQPRspLen %d pGASCtrl->MMPDUSize %d\n", ANQPRspLen, pGASCtrl->MMPDUSize);

	if ((ANQPRspLen > pGASCtrl->MMPDUSize) || (pGASCtrl->cb_delay != 0)) {

		Event->EventType = GAS_RSP_MORE;
		Len += 2;
		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List) {
			if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, PeerMACAddr)) {
				IsFound = TRUE;
				Event->u.GAS_RSP_MORE_DATA.DialogToken = GASPeerEntry->DialogToken;
				Len += 1;

				if ((ANQPRspLen % pGASCtrl->MMPDUSize) == 0)
					GASPeerEntry->GASRspFragNum = ANQPRspLen / pGASCtrl->MMPDUSize;
				else
					GASPeerEntry->GASRspFragNum = (ANQPRspLen / pGASCtrl->MMPDUSize) + 1;

				GASPeerEntry->CurrentGASFragNum = 0;

				for (i = 0; i < GASPeerEntry->GASRspFragNum; i++) {
					os_alloc_mem(NULL, (UCHAR **)&GASQueryRspFrag, sizeof(*GASQueryRspFrag));

					if (!GASQueryRspFrag) {
						MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not available memory\n");
						RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
						goto error1;
					}
					NdisZeroMemory(GASQueryRspFrag, sizeof(*GASQueryRspFrag));
					GASQueryRspFrag->GASRspFragID = i;

					if (i < (GASPeerEntry->GASRspFragNum - 1))
						GASQueryRspFrag->FragQueryRspLen = pGASCtrl->MMPDUSize;
					else
						GASQueryRspFrag->FragQueryRspLen = ANQPRspLen - (pGASCtrl->MMPDUSize * i);

					if (GASQueryRspFrag->FragQueryRspLen > 0) {
						os_alloc_mem(NULL, (UCHAR **)&GASQueryRspFrag->FragQueryRsp,
								 GASQueryRspFrag->FragQueryRspLen);

						if (!GASQueryRspFrag->FragQueryRsp) {
							MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not available memory\n");
							RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
							goto error2;
						}

						NdisMoveMemory(GASQueryRspFrag->FragQueryRsp, &ANQPRsp[QueryRspOffset],
								GASQueryRspFrag->FragQueryRspLen);
						QueryRspOffset += GASQueryRspFrag->FragQueryRspLen;
						DlListAddTail(&GASPeerEntry->GASQueryRspFragList,
								  &GASQueryRspFrag->List);
					}
				}

				break;
			}
		}

		if (IsFound)
			GASPeerEntry->CurrentState = WAIT_GAS_RSP;

		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

		if (!IsFound) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Can not find peer address in GASPeerList\n");
			goto error1;
		}

		Event->u.GAS_RSP_MORE_DATA.StatusCode = 0;
		Len += 2;
		Event->u.GAS_RSP_MORE_DATA.GASComebackDelay = (pGASCtrl->cb_delay == 0 ? 1 : pGASCtrl->cb_delay);
		Len += 2;
		Event->u.GAS_RSP_MORE_DATA.AdvertisementProID = ACCESS_NETWORK_QUERY_PROTOCOL;
		Len += 1;
		MlmeEnqueue(pAd, GAS_STATE_MACHINE, GAS_RSP_MORE, Len, Buf, 0);
	} else {
		Event->EventType = GAS_RSP;
		Len += 2;

		RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
		DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
		{
			if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, PeerMACAddr)) {
				IsFound = TRUE;
				printk("GAS RSP DialogToken = %x\n", GASPeerEntry->DialogToken);
				Event->u.GAS_RSP_DATA.DialogToken = GASPeerEntry->DialogToken;
				Len += 1;
			}
			break;
		}
		if (IsFound)
			GASPeerEntry->CurrentState = WAIT_GAS_RSP;

		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

		if (!IsFound) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Can not find peer address in GASPeerList\n");
			goto error1;
		}

		Event->u.GAS_RSP_DATA.StatusCode = 0;
		Len += 2;
		Event->u.GAS_RSP_DATA.GASComebackDelay = 0;
		Len += 2;
		Event->u.GAS_RSP_DATA.AdvertisementProID = ACCESS_NETWORK_QUERY_PROTOCOL;
		Len += 1;
		Event->u.GAS_RSP_DATA.QueryRspLen = ANQPRspLen;
		Len += 2;

		if (ANQPRspLen > 0)
			NdisMoveMemory(Event->u.GAS_RSP_DATA.QueryRsp, ANQPRsp, ANQPRspLen);
		Len += ANQPRspLen;

		MlmeEnqueue(pAd, GAS_STATE_MACHINE, GAS_RSP, Len, Buf, 0);
	}

	os_free_mem(Buf);
	return TRUE;
error2:
	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	/* DlListDel and free GASQueryRspFrag first to avoid be overwrite */
	DlListDel(&GASQueryRspFrag->List);
	os_free_mem(GASQueryRspFrag);
	DlListForEachSafe(GASQueryRspFrag, Tmp, &GASPeerEntry->GASQueryRspFragList,
					  GAS_QUERY_RSP_FRAGMENT, List) {
		DlListDel(&GASQueryRspFrag->List);
		os_free_mem(GASQueryRspFrag);
	}
	DlListInit(&GASPeerEntry->GASQueryRspFragList);
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
error1:
	os_free_mem(Buf);
error0:
	return FALSE;
}

#ifdef DPP_SUPPORT
INT wext_send_dpp_cached_frame(struct wifi_dev *wdev, UINT32 frm_id,
				RTMP_IOCTL_INPUT_STRUCT *wrq)
{

	struct wapp_event *event;
	struct wapp_dpp_action_frame *req_data = NULL;
	struct dpp_frame_list *dpp_frame;

	if (DlListEmpty(&wdev->dpp_frame_event_list)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"no frame latched on this wdev\n");
		return false;
	}
	DlListForEach(dpp_frame, &wdev->dpp_frame_event_list,  struct dpp_frame_list, List) {
		if (dpp_frame->dpp_frame_event->data.frame.wapp_dpp_frame_id_no == frm_id) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Found Matching Frame ID\n");
			break;
		}
	}
	if (dpp_frame == NULL) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"DPP Action frame of ID %d not found in Buffer\n", frm_id);
		return false;
	}

	event = (struct wapp_event *)dpp_frame->dpp_frame_event;
	req_data = (struct wapp_dpp_action_frame *)&(event->data.frame);
	wrq->u.data.length = sizeof(*req_data) + req_data->frm_len;
	copy_to_user(wrq->u.data.pointer, req_data, wrq->u.data.length);
	DlListDel(&dpp_frame->List);
	MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"list size is now=%d\n", DlListLen(&wdev->dpp_frame_event_list));
	os_free_mem(dpp_frame->dpp_frame_event);
	os_free_mem(dpp_frame);
	return true;
}

INT mtk_set_pmk(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, struct pmk_req *pmk_data)
{
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	pSecConfig = &wdev->SecConfig;

	if (!IS_AKM_DPP(pSecConfig->AKMMap)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"DPP not supported, first set configuration\n");
		return -1;
	}
	if (pmk_data->pmk_len > ARRAY_SIZE(pmk_data->pmk))
		pmk_data->pmk_len = ARRAY_SIZE(pmk_data->pmk);
	if (pmk_data->ssidlen > MAX_LEN_OF_SSID)
		pmk_data->ssidlen = MAX_LEN_OF_SSID;
	NdisMoveMemory(pSecConfig->PMK, pmk_data->pmk, pmk_data->pmk_len);

	/* add one entry in pmk cache */
	if ((pObj->ioctl_if_type == INT_MBSSID)  || (pObj->ioctl_if_type == INT_MAIN)) {
		RTMPAddPMKIDCache(&pAd->ApCfg.PMKIDCache,
				apidx,
				pmk_data->supplicant_addr,
				pmk_data->pmkid,
				pmk_data->pmk,
				FALSE,
				pmk_data->pmk_len);
	} else if (pObj->ioctl_if_type == INT_APCLI) {
		UINT32 sec_akm = 0;
		SET_AKM_DPP(sec_akm);

		sta_add_pmkid_cache(pAd, pmk_data->supplicant_addr, pmk_data->pmkid,
			pmk_data->pmk, pmk_data->pmk_len, wdev->func_idx, wdev, sec_akm, pmk_data->ssid, pmk_data->ssidlen);

	}
	MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%s", __func__);
	hex_dump("PMK", pSecConfig->PMK, pmk_data->pmk_len);

	return 0;
}
#endif /* DPP_SUPPORT */
#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
INT mtk_cancel_roc(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	struct ch_switch_cfg *ch_sw_info = NULL;
	INT ret;
	UCHAR BandIdx = 0;

	if (wdev == NULL)
		return -EFAULT;

	BandIdx = HcGetBandByWdev(wdev);

	ch_sw_info = &pAd->ch_sw_cfg[BandIdx];

	if (pAd == NULL)
		return -EFAULT;

	if (ch_sw_info == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ch info not found\n");
		return -EFAULT;
	}

	if (ch_sw_info->ch_sw_on_going == TRUE || ch_sw_info->ch_switch_sm.CurrState == CH_SWITCH_STATE_RUNNING) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "canceling ROC\n");
		ret = ch_switch_monitor_cancel(pAd, wdev);

		if (ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "cancel roc done\n");
			return ret;
		}
	} else
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Failed, ROC not going on\n");

	return 0;
}
INT mtk_switch_chan(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR channel, UINT duration)
{
	UCHAR band_idx = 0;
	INT ret;
	struct ch_switch_cfg *ch_sw_cfg;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "--> \n");

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is NULL!\n");
		return FALSE;
	}

	ret = os_alloc_mem(pAd, (UCHAR **)&ch_sw_cfg, sizeof(struct ch_switch_cfg));
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "alloc mem fail!\n");
		return FALSE;
	}
	band_idx = HcGetBandByWdev(wdev);

	NdisZeroMemory(ch_sw_cfg, sizeof(struct ch_switch_cfg));
	ch_sw_cfg->channel = channel;
	if (!duration)
		duration = 40;
	ch_sw_cfg->duration = duration;
	ch_sw_cfg->wdev = wdev;
	ret = set_ch_switch_monitor_cfg(pAd, ch_sw_cfg);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "set_ch_switch_monitor_cfg failed!\n");
		os_free_mem(ch_sw_cfg);
		return FALSE;
	}
	os_free_mem(ch_sw_cfg);

	return TRUE;
}

INT mtk_start_roc(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, struct roc_req *roc)
{
	UCHAR wdev_channel = 0;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is NULL!\n");
		return FALSE;
	}

	wdev_channel = wlan_operate_get_prim_ch(wdev);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");

	if (wdev_channel == roc->chan) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "wdev is in same channel as roc request\n");
		return 0;
	}

	return mtk_switch_chan(pAd, wdev, (UCHAR)roc->chan, roc->wait_time);
}

INT mtk_send_offchannel_action_frame(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, struct action_frm_data *frm)
{
	INT ret;
	UCHAR wdev_channel = 0;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 Hdr;
	PUCHAR pOutBuffer = NULL;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev is NULL!\n");
		return FALSE;
	}

	wdev_channel = wlan_operate_get_prim_ch(wdev);

	if (wdev_channel != frm->chan) {
		if(CheckNonOccupancyChannel(pAd, wdev, (UCHAR)frm->chan) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"channel %d is a RADAR channel no need to switch\n", (int)frm->chan);
			return FALSE;
		}
		ret = mtk_switch_chan(pAd, wdev, (UCHAR)frm->chan, frm->wait_time);
		if (ret == FALSE) {
			MTWF_DBG(pAd,DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"failed to change channel\n");
			return ret;
		}
	}

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS)
		return NStatus;
	MgtMacHeaderInitExt(pAd, &Hdr, SUBTYPE_ACTION, 0, frm->destination_addr,
					frm->transmitter_addr,
					frm->bssid);

	Hdr.Sequence = frm->seq_no;
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(HEADER_802_11), &Hdr,
					  frm->frm_len, frm->frm,
					  END_OF_ARGS);

	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);

	return 0;
}
#endif /* CHANNEL_SWITCH_MONITOR_CONFIG */
#endif

#ifdef CONFIG_DOT11V_WNM
INT Set_SendBTMReq_Proc(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	UCHAR macAddr[MAC_ADDR_LEN];
	RTMP_STRING *value;
	INT i;
	MAC_TABLE_ENTRY *pEntry = NULL;
	BTM_REQ_FRAME req_frame;

	NdisZeroMemory(&req_frame, sizeof(req_frame));
	if (ifIndex >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, "Unknown If index (%d)", ifIndex);
		return -1;
	}

	if (strlen(arg) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
			return FALSE;  /*Invalid */

		AtoH(value, (UCHAR *)&macAddr[i++], 1);
	}

	pEntry = MacTableLookup(pAd, macAddr);
	if (pEntry) {
		req_frame.request_mode = (req_frame.request_mode & ~0x02) | (1 << 1);
		req_frame.request_mode = (req_frame.request_mode & ~0x04) | (1 << 2);
		req_frame.disassociation_timer = 600;
		req_frame.validity_interval = 200;
		Send_BTM_Req(pAd, macAddr, (RTMP_STRING *) &req_frame, sizeof(BTM_REQ_FRAME));
	}

	return TRUE;
}

INT Send_BTM_Req(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *PeerMACAddr,
	IN RTMP_STRING *BTMReq,
	IN UINT32 BTMReqLen)
{
	UCHAR *Buf;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
	BTM_EVENT_DATA *Event;
	BTM_PEER_ENTRY *BTMPeerEntry;
	UINT32 Len = 0;
	INT32 Ret;
	BOOLEAN IsFound = FALSE, ret = FALSE;

	MTWF_PRINT("%s\n", __func__);
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
	DlListForEach(BTMPeerEntry, &pWNMCtrl->BTMPeerList, BTM_PEER_ENTRY, List) {
		if (MAC_ADDR_EQUAL(BTMPeerEntry->PeerMACAddr, PeerMACAddr)) {
			IsFound = TRUE;
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);

	if (!IsFound) {
		os_alloc_mem(NULL, (UCHAR **)&BTMPeerEntry, sizeof(*BTMPeerEntry));

		if (!BTMPeerEntry) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not available memory\n");
			goto error0;
		}

		NdisZeroMemory(BTMPeerEntry, sizeof(*BTMPeerEntry));
		BTMPeerEntry->CurrentState = WAIT_BTM_REQ;
		BTMPeerEntry->ControlIndex = APIndex;
		NdisMoveMemory(BTMPeerEntry->PeerMACAddr, PeerMACAddr, MAC_ADDR_LEN);
		BTMPeerEntry->DialogToken = 1;
		BTMPeerEntry->Priv = pAd;
		RTMPInitTimer(pAd, &BTMPeerEntry->WaitPeerBTMRspTimer,
					  GET_TIMER_FUNCTION(WaitPeerBTMRspTimeout), BTMPeerEntry, FALSE);
		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->BTMPeerListLock, Ret);
		DlListAddTail(&pWNMCtrl->BTMPeerList, &BTMPeerEntry->List);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->BTMPeerListLock);
	}

	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event) + BTMReqLen);

	if (!Buf) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not available memory\n");
		goto error1;
	}

	NdisZeroMemory(Buf, sizeof(*Event) + BTMReqLen);

	Event = (BTM_EVENT_DATA *)Buf;
	Event->ControlIndex = APIndex;
	Len += 1;
	NdisMoveMemory(Event->PeerMACAddr, PeerMACAddr, MAC_ADDR_LEN);
	Len += MAC_ADDR_LEN;
	Event->EventType = BTM_REQ;
	Len += 2;
	Event->u.BTM_REQ_DATA.DialogToken = BTMPeerEntry->DialogToken;
	Len += 1;
	Event->u.BTM_REQ_DATA.BTMReqLen = BTMReqLen;
	Len += 2;
	NdisMoveMemory(Event->u.BTM_REQ_DATA.BTMReq, BTMReq, BTMReqLen);
	Len += BTMReqLen;
	MTWF_PRINT("\nbefore adding BSS Transition Candidate List Entries::BTMReqLen=%d, Len=%d\n",
			  BTMReqLen, Len);
	ret = MlmeEnqueue(pAd, BTM_STATE_MACHINE, BTM_REQ, Len, Buf, 0);
	if (FALSE == ret)
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "enqueue MLME failed!\n");

	os_free_mem(Buf);
	return TRUE;
error1:

	if (!IsFound)
		BTM_free_Entry(BTMPeerEntry);

error0:
	return FALSE;
}

INT Send_WNM_Notify_Req(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *PeerMACAddr,
	IN RTMP_STRING *WNMNotifyReq,
	IN UINT32 WNMNotifyReqLen,
	IN UINT32 type)
{
	UCHAR *Buf;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
	WNM_NOTIFY_EVENT_DATA *Event;
	WNM_NOTIFY_PEER_ENTRY *WNMNotifyPeerEntry;
	UINT32 Len = 0;
	INT32 Ret;
	BOOLEAN IsFound = FALSE;

	MTWF_PRINT("%s\n", __func__);
	RTMP_SEM_EVENT_WAIT(&pWNMCtrl->WNMNotifyPeerListLock, Ret);
	DlListForEach(WNMNotifyPeerEntry, &pWNMCtrl->WNMNotifyPeerList, WNM_NOTIFY_PEER_ENTRY, List) {
		if (MAC_ADDR_EQUAL(WNMNotifyPeerEntry->PeerMACAddr, PeerMACAddr)) {
			IsFound = TRUE;
			break;
		}
	}
	RTMP_SEM_EVENT_UP(&pWNMCtrl->WNMNotifyPeerListLock);

	if (!IsFound) {
		os_alloc_mem(NULL, (UCHAR **)&WNMNotifyPeerEntry, sizeof(*WNMNotifyPeerEntry));

		if (!WNMNotifyPeerEntry) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not available memory\n");
			goto error0;
		}

		NdisZeroMemory(WNMNotifyPeerEntry, sizeof(*WNMNotifyPeerEntry));
		WNMNotifyPeerEntry->CurrentState = WNM_NOTIFY_REQ;
		WNMNotifyPeerEntry->ControlIndex = APIndex;
		NdisMoveMemory(WNMNotifyPeerEntry->PeerMACAddr, PeerMACAddr, MAC_ADDR_LEN);
		WNMNotifyPeerEntry->DialogToken = 1;
		WNMNotifyPeerEntry->Priv = pAd;
		RTMPInitTimer(pAd, &WNMNotifyPeerEntry->WaitPeerWNMNotifyRspTimer,
					  GET_TIMER_FUNCTION(WaitPeerWNMNotifyRspTimeout), WNMNotifyPeerEntry, FALSE);
		RTMP_SEM_EVENT_WAIT(&pWNMCtrl->WNMNotifyPeerListLock, Ret);
		DlListAddTail(&pWNMCtrl->WNMNotifyPeerList, &WNMNotifyPeerEntry->List);
		RTMP_SEM_EVENT_UP(&pWNMCtrl->WNMNotifyPeerListLock);
	}

	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event) + WNMNotifyReqLen);

	if (!Buf) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not available memory\n");
		goto error1;
	}

	/* RTMPusecDelay(200000); //200ms */
	NdisZeroMemory(Buf, sizeof(*Event) + WNMNotifyReqLen);
	Event = (WNM_NOTIFY_EVENT_DATA *)Buf;
	Event->ControlIndex = APIndex;
	Len += 1;
	NdisMoveMemory(Event->PeerMACAddr, PeerMACAddr, MAC_ADDR_LEN);
	Len += MAC_ADDR_LEN;
	Event->EventType = type; /* WNM_NOTIFY_REQ; */
	Len += 2;
	Event->u.WNM_NOTIFY_REQ_DATA.DialogToken = WNMNotifyPeerEntry->DialogToken;
	Len += 1;
	Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReqLen = WNMNotifyReqLen;
	Len += 2;
	NdisMoveMemory(Event->u.WNM_NOTIFY_REQ_DATA.WNMNotifyReq, WNMNotifyReq, WNMNotifyReqLen);
	Len += WNMNotifyReqLen;
	MlmeEnqueue(pAd, WNM_NOTIFY_STATE_MACHINE, WNM_NOTIFY_REQ, Len, Buf, 0);
	os_free_mem(Buf);
	return TRUE;
error1:

	if (!IsFound)
		os_free_mem(WNMNotifyPeerEntry);

error0:
	return FALSE;
}

#ifdef CONFIG_HOTSPOT_R2
INT Send_QOSMAP_Configure(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *PeerMACAddr,
	IN RTMP_STRING *QosMapBuf,
	IN UINT32	QosMapLen,
	IN UINT8	Apidx)
{
	MLME_QOS_ACTION_STRUCT QosMapConfig;
	QOSMAP_SET *pQOSMap = &QosMapConfig.QOSMap;

	NdisZeroMemory(&QosMapConfig, sizeof(MLME_QOS_ACTION_STRUCT));
	COPY_MAC_ADDR(QosMapConfig.Addr, PeerMACAddr);
	QosMapConfig.ActionField = ACTION_QOSMAP_CONFIG;
	QosMapConfig.apidx = Apidx;
	pQOSMap->DSCP_Field_Len = QosMapLen;
	NdisMoveMemory(pQOSMap->DSCP_Field, QosMapBuf, QosMapLen);
	MlmeEnqueue(pAd, ACTION_STATE_MACHINE, MT2_MLME_QOS_CATE, sizeof(MLME_QOS_ACTION_STRUCT), (PVOID)&QosMapConfig, 0);
	RTMP_MLME_HANDLER(pAd);
	return TRUE;
}

/* for debug */
INT Set_CR4_Hotspot_Flag(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT8 Flag = os_str_tol(arg, 0, 10);
	PHOTSPOT_CTRL pHSCtrl;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
	MT_HOTSPOT_INFO_UPDATE_T HotspotInfoUpdateT;

	pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;
	pHSCtrl->HotSpotEnable = IS_HOTSPOT_ENABLE(Flag);
	pHSCtrl->DGAFDisable = IS_DGAF_DISABLE(Flag);
	pHSCtrl->bASANEnable = IS_ASAN_ENABLE(Flag);
	pHSCtrl->QosMapEnable = IS_QOSMAP_ENABLE(Flag);
	pWNMCtrl->ProxyARPEnable = IS_PROXYARP_ENABLE(Flag);
	NdisZeroMemory(&HotspotInfoUpdateT, sizeof(HotspotInfoUpdateT));
	HotspotInfoUpdateT.ucUpdateType |= fgUpdateBssCapability;
	HotspotInfoUpdateT.ucHotspotBssFlags = Flag;
	HotspotInfoUpdateT.ucHotspotBssId = APIndex;
	MtCmdHotspotInfoUpdate(pAd, &HotspotInfoUpdateT);
	MTWF_PRINT("%s ===> Update BSS:%d  HotspotFlag:0x%x\n"
			 , __func__, HotspotInfoUpdateT.ucHotspotBssId, HotspotInfoUpdateT.ucHotspotBssFlags);
	hotspot_bssflag_dump(Flag);
	return TRUE;
}

#endif /* CONFIG_HOTSPOT_R2 */
#endif /* CONFIG_DOT11V_WNM */
#ifdef QOS_R1
#ifdef MSCS_PROPRIETARY
void Send_DABS_Announce
	(struct _RTMP_ADAPTER *pAd, UINT16 wcid)
{
	struct wifi_dev *wdev;
	MAC_TABLE_ENTRY *pEntry;
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS ret;
	FRAME_DABS_ACTION dabs_announce_frame;
	UCHAR mtk_oui[MTK_OUI_LEN] = {0x00, 0x0C, 0xE7};
	ULONG FrameLen;
	LARGE_INTEGER systemtime;

	NdisZeroMemory(&dabs_announce_frame, sizeof(FRAME_DABS_ACTION));
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Send_DABS_Announce:wcid=%d\n", wcid);
	ret = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "mscs rsp allocate memory failed\n");
		return;
	}

	pEntry = &pAd->MacTab.Content[wcid];
	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "entry not found\n");
		MlmeFreeMemory(pOutBuffer);
		return;
	}

	wdev = pEntry->wdev;
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev not found\n");
		MlmeFreeMemory(pOutBuffer);
		return;
	}
	RTMP_GetCurrentSystemTime(&systemtime);
	pEntry->APRandNum = systemtime.u.LowPart & 0xFFFF;
	if (pAd->keybitmap[2] == 0 && pAd->keybitmap[3] == 0)
		pAd->keybitmap[2] = GENERAL_KEY;

	ActHeaderInit(pAd, &(dabs_announce_frame.Hdr), pEntry->Addr, wdev->if_addr, wdev->bssid);
	dabs_announce_frame.Category = CATEGORY_VSP;
	dabs_announce_frame.version = pAd->dabs_version;
	NdisMoveMemory(dabs_announce_frame.Org_id, mtk_oui, MTK_OUI_LEN);
	dabs_announce_frame.trans_id = pEntry->dabs_trans_id;
	dabs_announce_frame.DABStype = DABS_ANNOUNCE;
	dabs_announce_frame.randnumber = pEntry->APRandNum;
	dabs_announce_frame.keybitmap[0] = pAd->keybitmap[0];
	dabs_announce_frame.keybitmap[1] = pAd->keybitmap[1];
	dabs_announce_frame.keybitmap[2] = pAd->keybitmap[2];
	dabs_announce_frame.keybitmap[3] = pAd->keybitmap[3];
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(FRAME_DABS_ACTION), &dabs_announce_frame,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);

	return;
}
void Send_DABS_Rsp
	(struct _RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR status, UINT32 *keybitmap)
{
	struct wifi_dev *wdev;
	MAC_TABLE_ENTRY *pEntry;
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS ret;
	FRAME_DABS_ACTION dabs_rsp_frame;
	UCHAR mtk_oui[MTK_OUI_LEN] = {0x00, 0x0C, 0xE7};
	ULONG FrameLen;
	EVENT_FAST_PATH_T event_fastpath;

	NdisZeroMemory(&dabs_rsp_frame, sizeof(FRAME_DABS_ACTION));
	NdisZeroMemory(&event_fastpath, sizeof(EVENT_FAST_PATH_T));

	ret = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "mscs rsp allocate memory failed\n");
		return;
	}

	pEntry = &pAd->MacTab.Content[wcid];
	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "entry not found\n");
		MlmeFreeMemory(pOutBuffer);
		return;
	}

	wdev = pEntry->wdev;
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wdev not found\n");
		MlmeFreeMemory(pOutBuffer);
		return;
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "dabs rsp(status=%d)\n", status);
	ActHeaderInit(pAd, &(dabs_rsp_frame.Hdr), pEntry->Addr, wdev->if_addr, wdev->bssid);
	dabs_rsp_frame.Category = CATEGORY_VSP;
	NdisMoveMemory(dabs_rsp_frame.Org_id, mtk_oui, MTK_OUI_LEN);
	dabs_rsp_frame.version = DABS_VERSION;
	dabs_rsp_frame.status = status;
	dabs_rsp_frame.trans_id = pEntry->dabs_trans_id;
	dabs_rsp_frame.DABStype = DABS_RSP;
	if (status == MIC_OK) {
		if (FastPathCheckMIC(pAd, FAST_PATH_CMD_CAL_MIC, pEntry->wcid,
							pEntry->STARandNum, MIC_STA, 0, keybitmap, &event_fastpath) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "fw check mic error\n");
			MlmeFreeMemory(pOutBuffer);
			return;
		}
		dabs_rsp_frame.u2MIC = event_fastpath.u2Mic;
		if (event_fastpath.ucKeynum >= 64)
			dabs_rsp_frame.keybitmap[1] = 1 << (event_fastpath.ucKeynum - 64);
		else
			dabs_rsp_frame.keybitmap[0] = 1 << event_fastpath.ucKeynum;
		dabs_rsp_frame.key_number = event_fastpath.ucKeynum;
	}
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(FRAME_DABS_ACTION), &dabs_rsp_frame,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "send dabs rsp(status=%d)\n", status);
	return;
}
#endif/*MSCS_PROPRIETARY*/
#endif/*QOS_R1*/


#ifdef HOSTAPD_HS_R2_SUPPORT
INT Set_Qload_Bss(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT32 qload_test;
	UINT32 qload_cu;
	UINT32 qload_sta_cnt;
	INT i4Recv;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PHOTSPOT_CTRL pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;

	if (arg) {
		i4Recv = sscanf(arg, "%d-%d-%d", (UINT32 *)&qload_test, (UINT32 *)&qload_cu, (UINT32 *)&qload_sta_cnt);
		MTWF_PRINT("%s(): Qload Test %d,Qload Cu %d,Qload Sta Cnt %d\n",
				__FUNCTION__, qload_test, qload_cu, qload_sta_cnt);

		if (i4Recv != 3) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Format Error!\n");
			return FALSE;
		}
		pHSCtrl->QLoadTestEnable = qload_test;
		pHSCtrl->QLoadCU = qload_cu;
		pHSCtrl->QLoadStaCnt = qload_sta_cnt;
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Format Error!\n");
		return FALSE;
	}
	return TRUE;
}

INT Set_Icmpv4_Deny(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT8 icmpv4_deny = simple_strtol(arg, 0, 10);
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PHOTSPOT_CTRL pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;

	pHSCtrl->ICMPv4Deny = icmpv4_deny;
	MTWF_PRINT("%s(): ICMPV4 Deny %d\n", __FUNCTION__, pHSCtrl->ICMPv4Deny);
	return TRUE;
}

INT Set_L2_Filter(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT8 l2_filter = simple_strtol(arg, 0, 10);
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PHOTSPOT_CTRL pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;

	pHSCtrl->L2Filter = l2_filter;
	MTWF_PRINT("%s(): L2 Filter %d\n", __FUNCTION__, pHSCtrl->L2Filter);
	return TRUE;
}

INT Set_DGAF_Disable(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR Disable = simple_strtol(arg, 0, 10);
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PHOTSPOT_CTRL pHSCtrl;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[APIndex].wdev;

	pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;

	pHSCtrl->DGAFDisable = Disable;

	/* for 7615 offload to CR4 */
	hotspot_update_bssflag(pAd, fgDGAFDisable, Disable, pHSCtrl);
	hotspot_update_bss_info_to_cr4(pAd, APIndex, wdev->bss_info_argument.ucBssIndex);
	MTWF_PRINT("%s(): DGAF Disable %d\n", __FUNCTION__, pHSCtrl->DGAFDisable);

	return TRUE;
}

INT Set_QosMap(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg)
{

	RTMP_STRING *token1, *exception, *range;
	RTMP_STRING *value;
	int i;
	UINT8 type;
	UINT8 peer_mac_addr[6];
	MAC_TABLE_ENTRY  *pEntry;
	UCHAR PoolID = 0;

	token1 = rstrtok(arg, " ");
	exception = rstrtok(NULL, " ");
	range = rstrtok(NULL, " ");
	if (range == NULL) {
		type = 1;
		range = exception;
		exception = NULL;
	} else
		type = 0;

	if (strlen(token1) != 17) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "illegal MAC address length!\n");
		return TRUE;
	}
    for (i = 0, value = rstrtok(token1, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1)))) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "illegal MAC address format or octet!\n");
			/* Do not use "continue" to replace "break" */
			return TRUE;
		}
		AtoH(value, &peer_mac_addr[i++], 1);
	}
	MTWF_PRINT("Mac Addres "MACSTR"\n",	MAC2STR(peer_mac_addr));

	pEntry = MacTableLookup(pAd, peer_mac_addr);
	if (pEntry != NULL) {
		UINT8 tmp = 0;
		RTMP_STRING *value2;
		UINT8 qos_map_buf[60];
		UINT8 qos_map_buf_len = 0;
		pEntry->DscpExceptionCount = 0;
		memset(pEntry->DscpRange, 0xff, 16);
		memset(pEntry->DscpException, 0xff, 42);
		if (type == 0) {
			value = rstrtok(exception, ":");
			while (value != NULL) {
				value2 = rstrtok(NULL, ":");
				if (value2 != NULL) {
					pEntry->DscpException[tmp] = (simple_strtol(value, 0, 10) & 0xff);
					pEntry->DscpException[tmp] |=  (simple_strtol(value2, 0, 10) & 0xff) << 8;
					qos_map_buf[qos_map_buf_len++] = simple_strtol(value, 0, 10);
					qos_map_buf[qos_map_buf_len++] = simple_strtol(value2, 0, 10);
					tmp++;
				} else {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "illegal DSCP Exeption Format!\n");
					return TRUE;
				}
				value = rstrtok(NULL, ":");
			}
			pEntry->DscpExceptionCount = tmp;
			tmp = 0;
			value = rstrtok(range, ":");
			while (value != NULL) {
				value2 = rstrtok(NULL, ":");
				if (value2 != NULL) {
					pEntry->DscpRange[tmp] = (simple_strtol(value, 0, 10) & 0xff);
					pEntry->DscpRange[tmp] |=  (simple_strtol(value2, 0, 10) & 0xff) << 8;
					qos_map_buf[qos_map_buf_len++] = simple_strtol(value, 0, 10);
					qos_map_buf[qos_map_buf_len++] = simple_strtol(value2, 0, 10);
					tmp++;
				} else {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "illegal DSCP Range Format!\n");
					return TRUE;
				}
				value = rstrtok(NULL, ":");
			}
		} else {

			value = rstrtok(range, ":");
			while (value != NULL) {
				value2 = rstrtok(NULL, ":");
				if (value2 != NULL) {
					pEntry->DscpRange[tmp] = (simple_strtol(value, 0, 10) & 0xff);
					pEntry->DscpRange[tmp] |=  (simple_strtol(value2, 0, 10) & 0xff) << 8;
					qos_map_buf[qos_map_buf_len++] = simple_strtol(value, 0, 10);
					qos_map_buf[qos_map_buf_len++] = simple_strtol(value2, 0, 10);
					tmp++;
				} else {
					MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "illegal DSCP Range Format!\n");
					return TRUE;
				}
				value = rstrtok(NULL, ":");
			}
		}

		PoolID = hotspot_qosmap_add_pool(pAd, pEntry);
		hotspot_qosmap_update_sta_mapping_to_cr4(pAd, pEntry, PoolID);

				Send_QOSMAP_Configure(pAd,
				     peer_mac_addr,
				     qos_map_buf,
				     qos_map_buf_len,
				     pEntry->func_tb_idx);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pEntry for "MACSTR" not found\n",
				MAC2STR(peer_mac_addr));
	}
	return TRUE;
}

#endif

#if defined(HOSTAPD_HS_R2_SUPPORT) || defined(CONFIG_PROXY_ARP)
INT Set_ProxyArp_Enable(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg)
{
	UCHAR Enable = simple_strtol(arg, 0, 10);
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[APIndex].wdev;
	PHOTSPOT_CTRL pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;
	PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;

	pWNMCtrl->ProxyARPEnable = Enable;
	hotspot_update_bssflag(pAd, fgProxyArpEnable, Enable, pHSCtrl);
	hotspot_update_bss_info_to_cr4(pAd, APIndex, wdev->bss_info_argument.ucBssIndex);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ProxyArp Enable %d\n", pWNMCtrl->ProxyARPEnable);
	return TRUE;
}
#endif

#ifdef HOSTAPD_HS_R3_SUPPORT
INT Set_Osu_Enable(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	UINT8 osu_enable = simple_strtol(arg, 0, 10);
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[APIndex];

	pMbss->osu_enable = osu_enable;
	MTWF_PRINT("OSU Enable %d\n", pMbss->osu_enable);
	return TRUE;
}
#endif

#ifdef PRE_CAL_TRX_SET1_SUPPORT
INT Set_KtoFlash_Debug_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT Enable;

	Enable = os_str_tol(arg, 0, 10);
	pAd->KtoFlashDebug = (Enable > 0) ? TRUE : FALSE;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(enable = %d)\n",	pAd->KtoFlashDebug);
	return TRUE;
}
INT Set_RDCE_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	INT Type;
	INT BW;
	INT Band;
	INT i4Recv;

	if (arg) {
		i4Recv = sscanf(arg, "%d-%d-%d", &(Type), &(BW), &(Band));
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Type %d,BW %d,Band %d\n",
				 Type, BW, Band);

		if (i4Recv != 3) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Format Error!\n");
			return FALSE;
		}

		MtCmdRDCE(pAd, Type, BW, Band);
	} else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Format Error!\n");
		return FALSE;
	}

	return TRUE;
}

#endif /* PRE_CAL_TRX_SET1_SUPPORT */

#ifdef RLM_CAL_CACHE_SUPPORT
INT Set_RLM_Cal_Cache_Debug_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	rlmCalCacheStatus(pAd->rlmCalCache);
	rlmCalCacheDump(pAd->rlmCalCache);
	return TRUE;
}
#endif /* RLM_CAL_CACHE_SUPPORT */

INT Set_BWF_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT Enable;

	Enable = os_str_tol(arg, 0, 10);
	MtCmdSetBWFEnable(pAd, (UINT8)Enable);
	return TRUE;
}

#ifdef TX_AGG_ADJUST_WKR
INT Set_AggAdjWkr_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	pAd->TxAggAdjsut = os_str_tol(arg, 0, 16);
	MTWF_PRINT("TxAggAdjsut = %u\n", pAd->TxAggAdjsut);
	return TRUE;
}
#endif /* TX_AGG_ADJUST_WKR */

#ifdef CONFIG_TX_DELAY
INT Set_TX_Batch_Cnt_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UINT8 idx;
	struct tx_delay_control *tx_delay_ctl = NULL;

	for (idx = 0; idx < 2; idx++) {
		tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl[idx];
		tx_delay_ctl->tx_process_batch_cnt = simple_strtol(arg, 0, 10);
		MTWF_PRINT("TxProcessBatchCnt = %u\n", tx_delay_ctl->tx_process_batch_cnt);
	}

	return TRUE;
}

INT Set_Pkt_Min_Len_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UINT8 idx;
	struct tx_delay_control *tx_delay_ctl = NULL;

	for (idx = 0; idx < 2; idx++) {
		tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl[idx];
		tx_delay_ctl->min_pkt_len = simple_strtol(arg, 0, 10);
		MTWF_PRINT("min_pkt_len = %u\n", tx_delay_ctl->min_pkt_len);
	}

	return TRUE;
}

INT Set_Pkt_Max_Len_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UINT8 idx;
	struct tx_delay_control *tx_delay_ctl = NULL;

	for (idx = 0; idx < 2; idx++) {
		tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl[idx];
		tx_delay_ctl->max_pkt_len = simple_strtol(arg, 0, 10);
		MTWF_PRINT("max_pkt_len = %u\n", tx_delay_ctl->max_pkt_len);
	}
	return TRUE;
}

INT Set_TX_Delay_Timeout_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UINT8 idx;
	struct tx_delay_control *tx_delay_ctl = NULL;

	for (idx = 0; idx < 2; idx++) {
		tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl[idx];
		tx_delay_ctl->que_agg_timeout_value = simple_strtol(arg, 0, 10);
		MTWF_PRINT("que_agg_timeout_value = %u\n", tx_delay_ctl->que_agg_timeout_value);
	}
	return TRUE;
}
#endif

#ifdef HTC_DECRYPT_IOT
INT Set_WTBL_AAD_OM_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	ULONG wcid = 0;
	ULONG value = 0;
	RTMP_STRING *pWcidDest  = NULL;

	pWcidDest = strsep(&arg, ":");

	if (pWcidDest == NULL || arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid parameters\n");
		return FALSE;
	}

	wcid = os_str_toul(pWcidDest, 0, 10);
	value = os_str_toul(arg, 0, 10);
	HW_SET_ASIC_WCID_AAD_OM(pAd, wcid, (UCHAR)value);
	return TRUE;
}

INT Set_HTC_Err_TH_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)

{
	UINT value;

	value = os_str_tol(arg, 0, 10);

	if (value)
		pAd->HTC_ICV_Err_TH = value;

	MTWF_PRINT("HTC_ICV_Err_TH=%u\n", pAd->HTC_ICV_Err_TH);
	return TRUE;
}

INT Set_Entry_HTC_Err_Cnt_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)

{
	UINT value;
	PMAC_TABLE_ENTRY pEntry;

	value = os_str_tol(arg, 0, 10);
	pEntry = &pAd->MacTab.Content[1];

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		ULONG Now32;

		NdisGetSystemUpTime(&Now32);
		pEntry->HTC_ICVErrCnt = value;
		/* Rx HTC and FAIL decryp case! */
		/* if (rxd_base->RxD2.IcvErr == 1) */
		{
			if (pEntry->HTC_ICVErrCnt++ > pAd->HTC_ICV_Err_TH) {
				pEntry->HTC_ICVErrCnt = 0; /* reset the history */

				if (pEntry->HTC_AAD_OM_Force == 0) {
					pEntry->HTC_AAD_OM_Force = 1;
					HW_SET_ASIC_WCID_AAD_OM(pAd, 1, 1);
				} else {
					pEntry->HTC_AAD_OM_Force = 0;
					HW_SET_ASIC_WCID_AAD_OM(pAd, 1, 0);
				}
			}
		}
		MTWF_PRINT("HTC_ICVErrCnt=%u\n", pEntry->HTC_ICVErrCnt);
	}

	return TRUE;
}
#endif /* HTC_DECRYPT_IOT */

#ifdef DHCP_UC_SUPPORT
INT Set_DHCP_UC_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT value;

	value = os_str_tol(arg, 0, 10);

	if (value)
		pAd->DhcpUcEnable = TRUE;
	else
		pAd->DhcpUcEnable = FALSE;

	MTWF_PRINT("DhcpUcEnable=%u\n", pAd->DhcpUcEnable);
	return TRUE;
}
#endif /* DHCP_UC_SUPPORT */

#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
INT Set_Wds_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE  pObj;
	UCHAR	    apidx;
	UINT8 wds_enable;
	struct wifi_dev *pWdev;
	MAC_TABLE_ENTRY *pEntry;
	int wcid;
	MAC_TABLE *pMacTable = &pAd->MacTab;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
	wds_enable = simple_strtol(arg, 0, 10);

#ifdef APCLI_SUPPORT
	if ((pObj->ioctl_if_type == INT_APCLI)) {
	STA_ADMIN_CONFIG *pApCli = &pAd->StaCfg[apidx];

		pWdev = &pApCli->wdev;
		if (wds_enable) {
			pWdev->wds_enable = TRUE;
		} else
			pWdev->wds_enable = FALSE;

		if (pApCli->MacTabWCID) {
			pEntry = &pMacTable->Content[pApCli->MacTabWCID];

			/*Enable 4 address mode for the entry */
			if (pEntry->bEnable4Addr) {
				HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pEntry->wcid, wds_enable);
			}
		}

		MTWF_PRINT("WDS STA 4 Address Mode set to  %d for wcid %d\n", wds_enable, pApCli->MacTabWCID);
	} else
#endif
	 {
		pWdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		if (wds_enable && pWdev->wds_enable)
			return TRUE;
		else if (wds_enable)
			pWdev->wds_enable = TRUE;
		else
			pWdev->wds_enable = FALSE;

		/*for all the connected entries set the 4 address mode */
		for (wcid = 1; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
			pEntry = &pMacTable->Content[wcid];
			if ((pEntry->wdev == pWdev) && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
				/*Enable 4 address mode for the entry */
				if (pEntry->bEnable4Addr)
					HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pEntry->wcid, wds_enable);
			}
		}

		MTWF_PRINT("Setting 4 Addr Hdr Translation for %d Bcast Entry \n", pWdev->bss_info_argument.bmc_wlan_idx);
		HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pWdev->bss_info_argument.bmc_wlan_idx, wds_enable);

		MtCmdSetA4Enable(pAd, HOST2CR4, pWdev->wds_enable);
		MTWF_PRINT("WDS AP 4 Address Mode set to  %d for Mbss %d \n", wds_enable, apidx);
	}
	return TRUE;
}
#endif

#ifdef MBSS_AS_WDS_AP_SUPPORT
INT Set_WdsMac_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR					Addr[MAC_ADDR_LEN];
	RTMP_STRING				*value;
	INT						i;
	MAC_TABLE_ENTRY *pEntry;
	int wcid;
	MAC_TABLE *pMacTable = &pAd->MacTab;
	POS_COOKIE  pObj;
	UCHAR	    apidx;
	struct wifi_dev *pWdev;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;

	pWdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (strlen(arg) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
			return FALSE;  /*Invalid */

		AtoH(value, (UCHAR *)&Addr[i++], 1);
	}
	COPY_MAC_ADDR(pAd->ApCfg.wds_mac, Addr);
	for (wcid = 1; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		pEntry = &pMacTable->Content[wcid];
		if ((pEntry->wdev == pWdev) && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
			/*Enable 4 address mode for the entry */
			if (pEntry->bEnable4Addr && MAC_ADDR_EQUAL(pEntry->Addr, Addr)) {
				HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(pAd, pEntry->wcid, TRUE);
				break;
			}
		}
	}
	MTWF_PRINT("WDS AP 4 Address Mode set to STA MAC %x:%x:%x:%x:%x:%x WCID %u \n", Addr[0],
				Addr[1], Addr[2], Addr[3], Addr[4], Addr[5], wcid);
	return TRUE;
}

#endif

#ifdef NF_SUPPORT_V2
static INT Set_NF_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	LONG nf_enable;

	kstrtol(arg, 10, &nf_enable);
	if (nf_enable == 1)
		HW_NF_UPDATE(pAd, TRUE);
	else if (nf_enable == 0) {
		HW_NF_UPDATE(pAd, FALSE);
		pAd->Avg_NF[BAND0] = 0;
#ifdef DBDC_MODE
		pAd->Avg_NF[BAND1] = 0;
#endif
	} else {
		MTWF_PRINT("Band 0 Average NF %d\n", pAd->Avg_NF[BAND0]);
#ifdef DBDC_MODE
		MTWF_PRINT("Band 1 Average NF %d\n", pAd->Avg_NF[BAND1]);
#endif
	}
	MTWF_PRINT("NF Enabled %d\n", nf_enable);
	return TRUE;
}
#endif


#ifdef DSCP_PRI_SUPPORT
INT	Set_Dscp_Pri_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	RTMP_STRING *this_char;
	UINT8 dscpValue, inf_idx;
	INT8 pri;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	inf_idx = pObj->ioctl_if;

	MTWF_PRINT("%s bss_idx : %d arg=%s\n", __func__, inf_idx, arg);
	this_char = strsep((char **)&arg, ":");
	if (this_char == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 "value not defined for Dscp and Priority\n");
		return FALSE;
	}

	dscpValue = simple_strtol(this_char, 0, 10);
	if (dscpValue > 63) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 "Invalid Dscp Value Valid Value between 0 to 63\n");
		return FALSE;
	}
	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 "Priority not defined for Dscp %d\n", dscpValue);
		return FALSE;
	}
	pri = simple_strtol(arg, 0, 10);

	if (pri < -1  || pri > 7) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 "Invalid Priority value Valid value between 0 to 7\n");
		return FALSE;
	}

	/* Not needed in CR4 case
	if (pri == 0)
		pri = 3;
	*/

	pAd->ApCfg.MBSSID[inf_idx].dscp_pri_map[dscpValue] = pri;

	/*write CR4 for user defined DSCP/Priority Mapping and flag*/
	if (pAd->ApCfg.MBSSID[inf_idx].dscp_pri_map_enable) {
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
		if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
			MtCmdSetDscpPri(pAd, inf_idx);
			set_cp_support_en(pAd, "1");
		}
#endif
	}

	return TRUE;
}

INT	Set_Dscp_Pri_Enable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT8 inf_idx, enable;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	enable = os_str_tol(arg, 0, 10);

	inf_idx = pObj->ioctl_if;
	/*
	* MTWF_PRINT("%s bss_idx : %d dscp_Pri_map_enable=%d\n",
	*					__func__, inf_idx, enable);
	*/

	if (enable)
		pAd->ApCfg.MBSSID[inf_idx].dscp_pri_map_enable = TRUE;
	else
		pAd->ApCfg.MBSSID[inf_idx].dscp_pri_map_enable = FALSE;

	MTWF_PRINT("%s bss_idx : %d dscp_Pri_map_enable=%d\n",
						__func__, inf_idx, pAd->ApCfg.MBSSID[inf_idx].dscp_pri_map_enable);

	/*update CR4 for user defined DSCP/Priority Mapping and enable flag*/
#if defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	if (IS_MT7915(pAd) || IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
		MtCmdSetDscpPri(pAd, inf_idx);
		set_cp_support_en(pAd, "1");
	}
#endif

	return TRUE;
}


INT Show_Dscp_Pri_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT8 idx;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];

	MTWF_PRINT("[%s] DSCP priority setting for Bss Idx %d: DscpPriEnable:%d\n",
				__func__, pObj->ioctl_if, mbss->dscp_pri_map_enable);
	MTWF_PRINT("DSCP  PRI  DSCP  PRI  DSCP  PRI  DSCP  PRI  DSCP  PRI  DSCP  PRI  DSCP  PRI  DSCP  PRI\n");
	for (idx = 0; idx < 64; idx++) {
		if (idx%8 == 0)
			MTWF_PRINT("\n");
		MTWF_PRINT("%4d  %3d", idx, mbss->dscp_pri_map[idx]);
	}
	return TRUE;
}
#endif /*DSCP_PRI_SUPPORT*/

#ifdef AIR_MONITOR
INT Set_Enable_Air_Monitor_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR mnt_enable = 0, band_idx, ret = FALSE;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "-->\n");
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "wdev is NULL\n");
			return FALSE;
	}
	mnt_enable = (UCHAR)simple_strtol(arg, 0, 10);
	band_idx = HcGetBandByWdev(wdev);
	pAd->MntEnable[band_idx] = mnt_enable;
	pAd->MntMaxPktCnt[band_idx] = 0;

	ret = asic_set_air_mon_enable(pAd, mnt_enable, band_idx);


	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "<--[Band%d]MntEnable=0x%x\n", band_idx, pAd->MntEnable[band_idx]);
	return TRUE;
}

INT set_Monitor_max_pkt_cnt_app(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT32 max_cnt = 0, band_idx;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE, "-->\n");
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "wdev is NULL\n");
			return FALSE;
	}
	max_cnt = (UINT32)simple_strtol(arg, 0, 10);
	band_idx = HcGetBandByWdev(wdev);
	pAd->MntMaxPktCnt[band_idx] = max_cnt;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--[Band%d]MntEnable=%d\n", band_idx, pAd->MntMaxPktCnt[band_idx]);
	return TRUE;
}

INT	Set_MonitorRule_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	INT ret;
	RTMP_STRING *this_char = NULL, *value = NULL;
	UINT idx = 0;
	UCHAR rx_rule[3], band_idx;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "--> %s()\n", __func__);

	while ((this_char = strsep((char **)&arg, ";")) != NULL) {
		if (*this_char == '\0') {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "An unnecessary delimiter entered!\n");
			continue;
		}

		if (strlen(this_char) != 5) { /* the acceptable format is like 0:1:1 with length 5 */
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 "illegal length! (acceptable format 0:1:1 length 5)\n");
			continue;
		}

		for (idx = 0, value = rstrtok(this_char, ":"); value; value = rstrtok(NULL, ":")) {
			if ((strlen(value) != 1) || (!isxdigit(*value))) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "illegal format!\n");
				break;
			}

			rx_rule[idx++] = (UCHAR)os_str_tol(value, 0, 10);
		}

		if (idx != 3)
			continue;
	}

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "wdev is NULL\n");
		return FALSE;
	}

	band_idx = HcGetBandByWdev(wdev);

	ret = asic_set_air_mon_rule(pAd, rx_rule, band_idx);



	return ret;
}

INT	Set_MonitorTarget_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	INT ret = TRUE;
	RTMP_STRING *this_char = NULL;
	RTMP_STRING *value = NULL;
	INT idx = 0;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");

	while ((this_char = strsep((char **)&arg, ";")) != NULL) {
		if (*this_char == '\0') {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "An unnecessary delimiter entered!\n");
			continue;
		}

		if (strlen(this_char) != 17) { /* the acceptable format of MAC address is like 01:02:03:04:05:06 with length 17 */
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 "illegal MAC address length! (acceptable format 01:02:03:04:05:06 length 17)\n");
			continue;
		}

		for (idx = 0, value = rstrtok(this_char, ":"); value; value = rstrtok(NULL, ":")) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1)))) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "illegal MAC address format or octet!\n");
				break;
			}

			AtoH(value, &pAd->curMntAddr[idx++], 1);
		}

		if (idx != MAC_ADDR_LEN)
			continue;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, MACSTR"\n", MAC2STR(pAd->curMntAddr));
	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<-- %s()\n", __func__);
	return ret;
}

INT Set_MonitorIndex_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	INT ret;
	UCHAR mnt_idx = 0, band_idx = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "wdev is NULL\n");
		return FALSE;
	}

	band_idx = HcGetBandByWdev(wdev);
	mnt_idx = (UCHAR)simple_strtol(arg, 0, 10);
	ret = asic_set_air_mon_idx(pAd, wdev, mnt_idx, band_idx);

	return ret;
}



INT	Set_MonitorShowAll_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	INT ret = TRUE;
	UCHAR i = 0, bidx = 0;
	MNT_STA_ENTRY *pEntry = NULL;

	MTWF_PRINT("--> %s()\n", __func__);
	for (bidx = 0; bidx < DBDC_BAND_NUM; bidx++) {
		MTWF_PRINT("  Monitor Enable: BAND%d: %d\n", bidx, pAd->MntEnable[bidx]);
		MTWF_PRINT("  Index last set: BAND%d: %d\n", bidx, pAd->MntIdx[bidx]);
		MTWF_PRINT("  BAND%d Count: %d\n", bidx, pAd->MonitrCnt[bidx]);
	}
	for (bidx = 0; bidx < DBDC_BAND_NUM; bidx++) {
		for (i = 0; i < MAX_NUM_OF_MONITOR_STA; i++) {
			pEntry = &pAd->MntTable[bidx][i];

			if (pEntry->bValid) {
				MTWF_PRINT("Band%d: Monitor STA[%d]\t", pEntry->Band, i);
				MTWF_PRINT(MACSTR"\t", MAC2STR(pEntry->addr));
				MTWF_PRINT("[DATA]=%08lu\t", pEntry->data_cnt);
				MTWF_PRINT("[MGMT]=%08lu\t", pEntry->mgmt_cnt);
				MTWF_PRINT("[CNTL]=%08lu\t", pEntry->cntl_cnt);
				MTWF_PRINT("[TOTAL]=%08lu\t", pEntry->Count);
				MTWF_PRINT("RSSI:%d,%d,%d,%d\n",
						 pEntry->RssiSample.AvgRssi[0], pEntry->RssiSample.AvgRssi[1],
						 pEntry->RssiSample.AvgRssi[2], pEntry->RssiSample.AvgRssi[3]);
			}
		}
	}
	MTWF_PRINT("<-- %s()\n", __func__);
	return ret;
}

INT	Set_MonitorClearCounter_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	INT ret = TRUE;
	UCHAR i = 0, bidx = 0;
	MNT_STA_ENTRY *pEntry = NULL;

	MTWF_PRINT("--> %s()\n", __func__);
	for (bidx = 0; bidx < DBDC_BAND_NUM; bidx++) {
		MTWF_PRINT("--> %s()[%d], %p, %p, %p\n",
					__func__, bidx, pAd->MntTable, pAd->MntTable[bidx], &pAd->MntTable[bidx]);
		for (i = 0; i < MAX_NUM_OF_MONITOR_STA; i++) {
			pEntry = pAd->MntTable[bidx] + i;
			pEntry->data_cnt = 0;
			pEntry->mgmt_cnt = 0;
			pEntry->cntl_cnt = 0;
			pEntry->Count = 0;
			NdisZeroMemory(&pEntry->RssiSample, sizeof(RSSI_SAMPLE));
		}
	}
	MTWF_PRINT("<-- %s()\n", __func__);
	return ret;
}

INT	Set_Enable_MonitorTarget_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg,
	IN USHORT	index)
{
	INT ret;
	CHAR str[8];

	ret = Set_MonitorTarget_Proc(pAd, arg);

	if (!ret)
		return ret;

	if (index < MAX_NUM_OF_MONITOR_STA) {
		ret = snprintf(str, sizeof(str), "%u", index);
		if (os_snprintf_error(sizeof(str), ret)) {
			MTWF_DBG(pAd, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				"final_name snprintf error!\n");
			return FALSE;
		}
		ret = Set_MonitorIndex_Proc(pAd, str);
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "The index is over the maximum limit.\n");
		ret = FALSE;
	}

	return ret;
}

INT	Set_MonitorTarget0_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 0);
}

INT	Set_MonitorTarget1_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 1);
}

INT	Set_MonitorTarget2_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 2);
}

INT	Set_MonitorTarget3_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 3);
}

INT	Set_MonitorTarget4_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 4);
}

INT	Set_MonitorTarget5_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 5);
}

INT	Set_MonitorTarget6_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 6);
}

INT	Set_MonitorTarget7_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 7);
}

INT	Set_MonitorTarget8_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 8);
}

INT	Set_MonitorTarget9_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 9);
}

INT	Set_MonitorTarget10_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 10);
}

INT	Set_MonitorTarget11_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 11);
}

INT	Set_MonitorTarget12_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 12);
}

INT	Set_MonitorTarget13_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 13);
}

INT	Set_MonitorTarget14_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 14);
}

INT	Set_MonitorTarget15_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN RTMP_STRING	*arg)
{
	return Set_Enable_MonitorTarget_Proc(pAd, arg, 15);
}

VOID Air_Monitor_Pkt_Report_Action(
	IN RTMP_ADAPTER	*pAd,
	IN UINT16	wcid,
	IN RX_BLK	*pRxBlk)
{
	UCHAR FrameBuf[512] = {0};
	AIR_RAW AirRaw = {0};
	HTTRANSMIT_SETTING HTSetting = {0};
	UCHAR Apidx = MAIN_MBSSID, BandIdx = 0, Channel = 0;
	UCHAR s_addr[MAC_ADDR_LEN];
	UCHAR ETH_P_AIR_MONITOR[LENGTH_802_3_TYPE] = {0x51, 0xA0};
	UINT32 frame_len = 0, offset = 0, i;
	struct sk_buff *skb = NULL;
	FRAME_CONTROL *fc = (FRAME_CONTROL *)pRxBlk->FC;
	MAC_TABLE_ENTRY *pMacEntry = &pAd->MacTab.Content[wcid];
	MNT_STA_ENTRY *pMntEntry = NULL;
	ULONG DataRate = 0;

	Channel = pRxBlk->channel_freq;

	if (Channel == 0)
		return;

	BandIdx = HcGetBandByChannel(pAd, Channel);

	if (BandIdx >= DBDC_BAND_NUM)
		return;

	pMntEntry = pAd->MntTable[BandIdx] + pMacEntry->mnt_idx[BandIdx];

	if (!pMntEntry->bValid)
		return;

	switch (fc->Type) {
	case FC_TYPE_MGMT:
		if (pAd->MntRuleBitMap & RULE_MGT)
			pMntEntry->mgmt_cnt++;
		else
			goto done;
		break;

	case FC_TYPE_CNTL:
		if (pAd->MntRuleBitMap & RULE_CTL)
			pMntEntry->cntl_cnt++;
		else
			goto done;
		break;

	case FC_TYPE_DATA:
		if (pAd->MntRuleBitMap & RULE_DATA)
			pMntEntry->data_cnt++;
		else
			goto done;
		break;

	default:
		goto done;
	}

	pMntEntry->Count++;
	NdisMoveMemory(pMntEntry->RssiSample.AvgRssi,
				   pRxBlk->rx_signal.raw_rssi, sizeof(pMntEntry->RssiSample.AvgRssi));


	for (i = 0; i < pAd->Antenna.field.RxPath; i++) {
		if (pMntEntry->RssiSample.AvgRssi[i] > 0)
			pMntEntry->RssiSample.AvgRssi[i] = -127;
	}

	/* Init frame buffer */
	NdisZeroMemory(FrameBuf, sizeof(FrameBuf));
	NdisZeroMemory(&AirRaw, sizeof(AirRaw));
	/* Fake a Source Address for transmission */
	COPY_MAC_ADDR(s_addr, pAd->ApCfg.MBSSID[Apidx].wdev.if_addr);

	if (s_addr[1] == 0xff)
		s_addr[1] = 0;
	else
		s_addr[1]++;

	/* Prepare the 802.3 header */
	MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[Apidx].wdev.if_addr, s_addr, ETH_P_AIR_MONITOR);
	offset += LENGTH_802_3;
	/* For Rate Info */
	HTSetting.field.MODE    = pRxBlk->rx_rate.field.MODE;
	HTSetting.field.MCS     = pRxBlk->rx_rate.field.MCS;
	HTSetting.field.BW      = pRxBlk->rx_rate.field.BW;
	HTSetting.field.ShortGI = pRxBlk->rx_rate.field.ShortGI;
	getRate(HTSetting, &DataRate);
	AirRaw.wlan_radio_tap.RATE = DataRate;
	AirRaw.wlan_radio_tap.PHYMODE = pRxBlk->rx_rate.field.MODE;
	AirRaw.wlan_radio_tap.MCS     = pRxBlk->rx_rate.field.MCS;
	AirRaw.wlan_radio_tap.BW      = pRxBlk->rx_rate.field.BW;
	AirRaw.wlan_radio_tap.ShortGI = pRxBlk->rx_rate.field.ShortGI;
#ifdef DOT11_VHT_AC
	if (AirRaw.wlan_radio_tap.PHYMODE >= MODE_VHT) {
		AirRaw.wlan_radio_tap.MCS = (pRxBlk->rx_rate.field.MCS & 0xf);
		AirRaw.wlan_radio_tap.STREAM  = (pRxBlk->rx_rate.field.MCS >> 4) + 1;
	} else
#endif /* DOT11_VHT_AC */
		if (AirRaw.wlan_radio_tap.PHYMODE == MODE_OFDM) {
			AirRaw.wlan_radio_tap.MCS = getLegacyOFDMMCSIndex(pRxBlk->rx_rate.field.MCS);
			AirRaw.wlan_radio_tap.STREAM  = (pRxBlk->rx_rate.field.MCS >> 4) + 1;
		} else {
			AirRaw.wlan_radio_tap.MCS = (pRxBlk->rx_rate.field.MCS % 8);
			AirRaw.wlan_radio_tap.STREAM  = (pRxBlk->rx_rate.field.MCS >> 3) + 1;
		}

	/* For RSSI */
	for (i = 0; i < pAd->Antenna.field.RxPath; i++)
		AirRaw.wlan_radio_tap.RSSI[i] = pMntEntry->RssiSample.AvgRssi[i];

	AirRaw.wlan_radio_tap.Channel = Channel;

	/* For 802.11 Header */
	NdisMoveMemory(&AirRaw.wlan_header.FC, fc, sizeof(*fc));
	AirRaw.wlan_header.Duration = pRxBlk->Duration;
	AirRaw.wlan_header.SN = pRxBlk->SN;
	AirRaw.wlan_header.FN = pRxBlk->FN;
	COPY_MAC_ADDR(AirRaw.wlan_header.Addr1, pRxBlk->Addr1);
	COPY_MAC_ADDR(AirRaw.wlan_header.Addr2, pRxBlk->Addr2);
	COPY_MAC_ADDR(AirRaw.wlan_header.Addr3, pRxBlk->Addr3);

	if (fc->ToDs == 1 && fc->FrDs == 1)
		COPY_MAC_ADDR(AirRaw.wlan_header.Addr4, pRxBlk->Addr4);

	/* Prepare payload*/
	if (offset + sizeof(AirRaw) < 512) {
		NdisCopyMemory(&FrameBuf[offset], (CHAR *)&AirRaw, sizeof(AirRaw));
		offset += sizeof(AirRaw);
		frame_len = offset;
	} else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Can't Copy AirRaw.\n");
		goto done;
	}

	/* Create skb */
	skb = dev_alloc_skb((frame_len + 2));

	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Error! Can't allocate a skb.\n");
		return;
	}

	SET_OS_PKT_NETDEV(skb, pAd->ApCfg.MBSSID[Apidx].wdev.if_dev);
	/* 16 byte align the IP header */
	skb_reserve(skb, 2);
	/* Insert the frame content */
	NdisMoveMemory(GET_OS_PKT_DATAPTR(skb), FrameBuf, frame_len);
	/* End this frame */
	skb_put(skb, frame_len);
	/* Report to upper layer */
	RtmpOsPktProtocolAssign(skb);
	RtmpOsPktRcvHandle(skb, pAd->tr_ctl.napi);
#ifdef CONFIG_MAP_SUPPORT
#if defined(WAPP_SUPPORT)
	if (pAd->MntMaxPktCnt[BandIdx] == 0)
		wapp_send_air_mnt_rssi(pAd, pMacEntry, pMntEntry);
	else if (pAd->MntMaxPktCnt[BandIdx] == pMntEntry->Count) {
		pMntEntry->RssiTotal += pMntEntry->RssiSample.AvgRssi[0];
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		 "total %ld avg %ld count %ld.\n", pMntEntry->RssiTotal, pMntEntry->RssiTotal/(LONG)pMntEntry->Count, pMntEntry->Count);
		pMntEntry->RssiSample.AvgRssi[0] = (char)(pMntEntry->RssiTotal/(LONG)pMntEntry->Count);
		wapp_send_air_mnt_rssi(pAd, pMacEntry, pMntEntry);
	} else
		pMntEntry->RssiTotal += pMntEntry->RssiSample.AvgRssi[0];
#endif /*WAPP_SUPPORT*/
#endif /* CONFIG_MAP_SUPPORT */
done:
	return;
}

#endif /* AIR_MONITOR */

#ifdef RADIUS_MAC_AUTH_SUPPORT
INT Set_Radius_Mac_Auth_Policy_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR apidx;
	UINT8 mac_auth_enable;
	struct wifi_dev *pWdev;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;
	mac_auth_enable = simple_strtol(arg, 0, 10);

	pWdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	if (mac_auth_enable)
		pWdev->radius_mac_auth_enable = TRUE;
	else
		pWdev->radius_mac_auth_enable = FALSE;

	MTWF_PRINT("Radius Mac Auth Policy Enable %d for Mbss %d\n", mac_auth_enable, apidx);
	return TRUE;
}

#endif

#ifdef DYNAMIC_VLAN_SUPPORT
INT Set_Sta_Vlan(RTMP_ADAPTER *pAd, RT_CMD_AP_STA_VLAN *sta_vlan)
{
	INT Status = NDIS_STATUS_SUCCESS;
	MAC_TABLE_ENTRY *pEntry;

	MTWF_PRINT("Set_Sta_Vlan searching pEntry for Addr "MACSTR"\n",
			MAC2STR(sta_vlan->sta_addr));

	/*Search for station address in the Mac Table if entry found set the vlan id */
	pEntry = MacTableLookup(pAd, sta_vlan->sta_addr);
	if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
		pEntry->vlan_id = sta_vlan->vlan_id;
		MTWF_PRINT("pEntry %p found and set the vlan id to %d\n", pEntry, pEntry->vlan_id);
	} else
		Status = NDIS_STATUS_FAILURE;

	return Status;
}
INT Set_Dvlan_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT8 vlan_id;
	MAC_TABLE_ENTRY *pEntry;
	MAC_TABLE *pMacTable = &pAd->MacTab;

	vlan_id = simple_strtol(arg, 0, 10);
	pEntry = &pMacTable->Content[1];

	pEntry->vlan_id = vlan_id;
	printk("wcid 1 configured for vlan id %d\n", vlan_id);
	return TRUE;
}
#endif

#ifdef HOSTAPD_11R_SUPPORT
INT Set_Ft_Param(RTMP_ADAPTER *pAd, RT_CMD_AP_11R_PARAM *ap_11r_param)
{
	PFT_CFG pFtCfg;
	INT Status = NDIS_STATUS_SUCCESS, apidx = 0;
	UINT nas_id_len = 0;

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"OwnMac: "MACSTR"\n", MAC2STR(ap_11r_param->own_mac));

	apidx = get_apidx_by_addr(pAd, ap_11r_param->own_mac);

	pFtCfg = &pAd->ApCfg.MBSSID[apidx].wdev.FtCfg;

	if (pFtCfg->FtCapFlag.Dot11rFtEnable) {
		NdisZeroMemory(pFtCfg->FtR0khId, sizeof(pFtCfg->FtR0khId));
		if (ap_11r_param->nas_id_len > ARRAY_SIZE(pFtCfg->FtR0khId))
			nas_id_len = ARRAY_SIZE(pFtCfg->FtR0khId);
		else
			nas_id_len = ap_11r_param->nas_id_len;
		NdisMoveMemory(pFtCfg->FtR0khId, ap_11r_param->nas_identifier, nas_id_len);
		pFtCfg->FtR0khIdLen = nas_id_len;
		NdisZeroMemory(pFtCfg->FtR1khId, MAC_ADDR_LEN);
		NdisMoveMemory(pFtCfg->FtR1khId, ap_11r_param->r1_key_holder, MAC_ADDR_LEN);

		pFtCfg->AssocDeadLine = ap_11r_param->reassociation_deadline;

		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"R1KH: %02x%02x%02x%02x%02x%02x", PRINT_MAC(ap_11r_param->r1_key_holder));
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"NAS-ID: %s", ap_11r_param->nas_identifier);
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"AssocDeadline: %d", ap_11r_param->reassociation_deadline);
	} else
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"11R disabled in driver\n");

	return Status;
}

#endif

#ifdef CONFIG_MAP_SUPPORT
#ifdef MAP_R2
/**
 * Set_MapR2_Proc: Function to enable/disable MAP R2 feature
 *
 * This API is used to enabled/disable MAP R2 feature in WLAN driver.
 **/
INT Set_MapR2_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UCHAR enable = os_str_tol(arg, 0, 10);

	if (pAd->bMapR2Enable == enable) {
		/* No need to do anything, current and previos values are same */
		MTWF_PRINT("%s MAP R2 is already %s\n", __func__, enable?"enabled":"disabled");
		return TRUE;
	}

	if (!enable)
		pAd->bMapR2Enable = FALSE;
	else
		pAd->bMapR2Enable = TRUE;

	MTWF_PRINT("%s: MAP R2 is %s\n", __func__, pAd->bMapR2Enable?"enabled":"disabled");

	return TRUE;
}
#endif

#ifdef MAP_R3
/**
 * Set_MapR3_Proc: Function to enable/disable MAP R3 feature
 *
 * This API is used to enabled/disable MAP R3 feature in WLAN driver.
 **/
INT Set_MapR3_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UCHAR enable = os_str_tol(arg, 0, 10);
	UINT8 BandIdx = DBDC_BAND0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (pAd->bMapR3Enable == enable) {
		/* No need to do anything, current and previos values are same */
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MAP R3 is already %s\n", enable?"enabled":"disabled");
		return TRUE;
	}

	if (!enable) {
		pAd->bMapR3Enable = FALSE;
		pAd->bMapR2Enable = FALSE;
	} else {
		pAd->bMapR3Enable = TRUE;
		pAd->bMapR2Enable = TRUE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"MAP R3 and MAP_R2 is %s, %s\n",
		pAd->bMapR3Enable?"enabled":"disabled",
		pAd->bMapR2Enable?"enabled":"disabled");

	if (pAd->MAPMode && pAd->bMapR3Enable == enable) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"Disable SCS\n");

		Set_SCSEnable_Proc(pAd, "0");

		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"wdev is NULL, so return\n");
			return FALSE;
		}

		BandIdx = HcGetBandByWdev(wdev);

		if (pAd->CommonCfg.SRMode[BandIdx]) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"Set Map Balance\n");
			SrSetMapBalance(pAd, BandIdx, pAd->MapBalance);

			if (IS_MAP_TURNKEY_ENABLE(pAd)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Set SRMeshUlMode[%u]:%u\n", BandIdx,
					pAd->CommonCfg.SRMeshUlMode[BandIdx]);

				SrCmdSetMeshUlMode(pAd, BandIdx,
					pAd->CommonCfg.SRMeshUlMode[BandIdx]);
			}
		}
	}

	return TRUE;
}
#endif

#ifdef MAP_R4
/**
 * Set_MapR4_Proc: Function to enable/disable MAP R3 feature
 *
 * This API is used to enabled/disable MAP R3 feature in WLAN driver.
 **/
INT Set_MapR4_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UCHAR enable = os_str_tol(arg, 0, 10);
	UINT8 BandIdx = DBDC_BAND0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev =
		get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (pAd->bMapR4Enable == enable) {
		/* No need to do anything, current and previos values are same */
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"MAP R4 is already %s\n", enable?"enabled":"disabled");
		return TRUE;
	}

	if (!enable) {
		pAd->bMapR4Enable = FALSE;
		pAd->bMapR3Enable = FALSE;
		pAd->bMapR2Enable = FALSE;
	} else {
		pAd->bMapR4Enable = TRUE;
		pAd->bMapR3Enable = TRUE;
		pAd->bMapR2Enable = TRUE;
	}

	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"MAP R4, MAP_R3 and MAP_R2 is %s, %s, %s\n",
		pAd->bMapR4Enable?"enabled":"disabled",
		pAd->bMapR3Enable?"enabled":"disabled",
		pAd->bMapR2Enable?"enabled":"disabled");


	return TRUE;
}
#endif /* MAP_R4 */


INT Set_Bh_Bss_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UCHAR enable = os_str_tol(arg, 0, 10);
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR band_idx = 0;

	/* only do this for AP MBSS, ignore other inf type */
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		UINT8	IfIdx = pObj->ioctl_if;

		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
	} else
		return FALSE;

/*	wdev->MAPCfg.DevOwnRole = 0;*/
	band_idx = HcGetBandByWdev(wdev);

	if (enable) {
		wdev->MAPCfg.DevOwnRole |= BIT(MAP_ROLE_BACKHAUL_BSS);
		pAd->bh_bss_wdev[band_idx] = wdev;
	} else
		wdev->MAPCfg.DevOwnRole &= ~BIT(MAP_ROLE_BACKHAUL_BSS);

	MTWF_PRINT("%s %s bandIdx = %d ,DevOwnRole 0x%x\n", __func__,
			wdev->if_dev->name, band_idx, wdev->MAPCfg.DevOwnRole);
	return TRUE;
}

INT Set_Fh_Bss_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UCHAR enable = os_str_tol(arg, 0, 10);
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR i = 0;
	PWSC_CTRL pWscControl = NULL;
	/* only do this for AP MBSS, ignore other inf type */
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		UINT8	IfIdx = pObj->ioctl_if;

		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
		pWscControl = &pAd->ApCfg.MBSSID[IfIdx].wdev.WscControl;
	} else
		return FALSE;

	if (enable)
		wdev->MAPCfg.DevOwnRole |= BIT(MAP_ROLE_FRONTHAUL_BSS);
	else {
		wdev->MAPCfg.DevOwnRole &= ~BIT(MAP_ROLE_FRONTHAUL_BSS);
		/* reset wsc backhaul profiles */
		for (i = 0; i < pWscControl->WscBhProfiles.ProfileCnt; i++) {
			NdisZeroMemory(&pWscControl->WscBhProfiles.Profile[i], sizeof(WSC_CREDENTIAL));
		}
		pWscControl->WscBhProfiles.ProfileCnt = 0;
	}

	MTWF_PRINT("%s %s wdev->MAPCfg.DevOwnRole 0x%x\n",
			__func__, wdev->if_dev->name, wdev->MAPCfg.DevOwnRole);

	return TRUE;
}
/**
 * reset_mtk_map_vendor_ie: Function to clear MAP vendor IE in beacons
 *
 * This API is used to clear MAP vendor IE in beacon of bss
 **/
static void reset_mtk_map_vendor_ie(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	os_zero_mem(wdev->MAPCfg.vendor_ie_buf, VENDOR_SPECIFIC_LEN);
	wdev->MAPCfg.vendor_ie_len = 0;
	if (wdev->bAllowBeaconing)
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
}

/**
 * Set_Map_Proc: Function to enable MAP
 *
 * This API is used to enabled/disable MAP in WLAN driver.
 * Upper layer should take care of killing deamons before disabling
 * and configuring MAP settings after enabling
 **/
INT Set_Map_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UCHAR map_mode = os_str_tol(arg, 0, 10);
	UCHAR ifIndex = 0;
	STA_ADMIN_CONFIG *pApCliEntry = NULL;
	PULONG pCurrState;
	PWSC_CTRL pWscControl = NULL;
	struct wifi_dev *wdev = NULL;
	int i = 0;

	for (i = BSS0; i < pAd->ApCfg.BssidNum; i++) {
		wdev = &pAd->ApCfg.MBSSID[i].wdev;
		if(wdev != NULL)
			NdisZeroMemory(&(wdev->MAPCfg.scan_bh_ssids), sizeof(struct scan_BH_ssids));
	}

	for (i = BSS0; i < MAX_APCLI_NUM; i++) {
		STA_ADMIN_CONFIG *apcli_entry = &pAd->StaCfg[i];
		if(apcli_entry) {
			wdev = &apcli_entry->wdev;
			if(wdev != NULL)
				NdisZeroMemory(&(wdev->MAPCfg.scan_bh_ssids), sizeof(struct scan_BH_ssids));
		}
	}

	if (pAd->MAPMode == map_mode) {
		/* No need to do anything, current and previos values are same */
		MTWF_PRINT("%s current MAP MODE is %d\n", __func__, map_mode);
		return TRUE;
	}

	/* In case of enable we don't need to do anything
	* since upper layer should configure interface roles */
	pAd->MAPMode = map_mode;
	if (IS_MAP_TURNKEY_ENABLE(pAd)) {
		for (i = BSS0; i < MAX_APCLI_NUM; i++) {
			STA_ADMIN_CONFIG *apcli_entry = &pAd->StaCfg[i];
				if (apcli_entry) {
					wdev = &apcli_entry->wdev;
					if (wdev != NULL)
						SetApCliEnableByWdev(pAd, wdev, FALSE);
			}
		}
	}

	if (!IS_MAP_ENABLE(pAd)) {
		/* Reset roles for For AP interfaces */
		for (ifIndex = 0; (ifIndex < MAX_MBSSID_NUM(pAd)); ifIndex++) {
			pAd->ApCfg.MBSSID[ifIndex].wdev.MAPCfg.DevOwnRole = BIT(MAP_ROLE_FRONTHAUL_BSS);
			APMlmeKickOutAllSta(pAd, ifIndex, REASON_DEAUTH_STA_LEAVING);
			pWscControl = &pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl;
			reset_mtk_map_vendor_ie(pAd, &pAd->ApCfg.MBSSID[ifIndex].wdev);
			if (pWscControl->bWscTrigger == TRUE)
				WscStop(pAd, FALSE, pWscControl);
		}

		/* Reset Roles for CLI interfaces */
		for (ifIndex = 0; (ifIndex < MAX_APCLI_NUM); ifIndex++) {
			pApCliEntry = &pAd->StaCfg[ifIndex];
			pCurrState = &pAd->StaCfg[ifIndex].wdev.cntl_machine.CurrState;
			if (!pApCliEntry->ApcliInfStat.Valid)
				continue;
			pWscControl = &pAd->StaCfg[ifIndex].wdev.WscControl;
			LinkDown(pAd, FALSE, &pApCliEntry->wdev, NULL);

			/* set the apcli interface be invalid.*/
			pApCliEntry->ApcliInfStat.Valid = FALSE;
			/* clear MlmeAux.Ssid and Bssid.*/
			NdisZeroMemory(pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
			pApCliEntry->MlmeAux.SsidLen = 0;
			NdisZeroMemory(pApCliEntry->MlmeAux.Ssid, MAX_LEN_OF_SSID);
			pApCliEntry->MlmeAux.Rssi = 0;
			*pCurrState = CNTL_IDLE;
			if (pWscControl->bWscTrigger == TRUE)
				WscStop(pAd, TRUE, pWscControl);
	}
		MTWF_PRINT("%s: disabled MAP", __func__);
	}
	return TRUE;
}


/**
 * Set_Map_Turnkey_Proc: Function to enable/disable MAP turnkey feature
 *
 * This API is used to enabled/disable MAP turnkey feature in WLAN driver.
 **/

#ifdef MAP_BL_SUPPORT
INT Set_BlackList_Add(
	PRTMP_ADAPTER pAd,
	char *arg)
{

	RTMP_STRING *this_char = NULL;
	RTMP_STRING *value = NULL;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT8	IfIdx = pObj->ioctl_if;
	UCHAR idx;
	UCHAR sta_mac[MAC_ADDR_LEN];
	BSS_STRUCT *pBss = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;

	/* only do this for AP MBSS, ignore other inf type */
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN))
		pBss = &pAd->ApCfg.MBSSID[IfIdx];
	 else
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "-->\n");

	while ((this_char = strsep((char **)&arg, ";")) != NULL) {
		if (*this_char == '\0') {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "An unnecessary delimiter entered!\n");
			continue;
		}

		if (strlen(this_char) != 17) { /* the acceptable format of MAC address is like 01:02:03:04:05:06 with length 17 */
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "illegal MAC address length! (acceptable format 01:02:03:04:05:06 length 17)\n");
			continue;
		}

		for (idx = 0, value = rstrtok(this_char, ":"); value; value = rstrtok(NULL, ":")) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1)))) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "illegal MAC address format or octet!\n");
				break;
			}

			AtoH(value, &sta_mac[idx++], 1);
		}

		if (idx != MAC_ADDR_LEN)
			continue;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "sta mac:"MACSTR"\n", MAC2STR(sta_mac));
	RTMP_SEM_LOCK(&pBss->BlackListLock);
	map_blacklist_add(&pBss->BlackList, sta_mac);
	RTMP_SEM_UNLOCK(&pBss->BlackListLock);

	pEntry = MacTableLookup(pAd, sta_mac);

	if (!pEntry || !pEntry->wdev) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pEntry or pEntry->wdev is null!\n");
		return FALSE;
	}

	if (pEntry && (pEntry->wdev->func_idx == IfIdx)) {
#ifdef MAP_R2
		pEntry->DisconnectReason = REASON_DISASSOC_STA_LEAVING;
#endif
		MlmeDeAuthAction(pAd, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<--\n");
	return TRUE;
}

INT Set_BlackList_Del(
	PRTMP_ADAPTER pAd,
	char *arg)
{

	RTMP_STRING *this_char = NULL;
	RTMP_STRING *value = NULL;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT8	IfIdx = pObj->ioctl_if;
	UCHAR idx;
	UCHAR sta_mac[MAC_ADDR_LEN];
	BSS_STRUCT *pBss;

	/* only do this for AP MBSS, ignore other inf type */
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN))
		pBss = &pAd->ApCfg.MBSSID[IfIdx];
	else
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "--> \n");

	while ((this_char = strsep((char **)&arg, ";")) != NULL) {
		if (*this_char == '\0') {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, "An unnecessary delimiter entered!\n");
			continue;
		}

		if (strlen(this_char) != 17) { /* the acceptable format of MAC address is like 01:02:03:04:05:06 with length 17 */
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "illegal MAC address length! (acceptable format 01:02:03:04:05:06 length 17)\n");
			continue;
		}

		for (idx = 0, value = rstrtok(this_char, ":"); value; value = rstrtok(NULL, ":")) {
			if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1)))) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "illegal MAC address format or octet!\n");
				break;
			}

			AtoH(value, &sta_mac[idx++], 1);
		}

		if (idx != MAC_ADDR_LEN)
			continue;
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "sta mac:"MACSTR"\n", MAC2STR(sta_mac));
	RTMP_SEM_LOCK(&pBss->BlackListLock);
	map_blacklist_del(&pBss->BlackList, sta_mac);
	RTMP_SEM_UNLOCK(&pBss->BlackListLock);

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "<-- \n");
	return TRUE;
}

INT Set_BlackList_Show(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR ap_idx = 0;

	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN))
		ap_idx = pObj->ioctl_if;
	else
		return FALSE;

	map_blacklist_show(pAd, ap_idx);

	return TRUE;
}
#endif /*  MAP_BL_SUPPORT */
#endif /* CONFIG_MAP_SUPPORT */

#ifdef DPP_SUPPORT
INT Set_Enable_Dpp_Proc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UCHAR enable = os_str_tol(arg, 0, 10);

	if (pAd->bDppEnable == enable) {
		/* No need to do anything, current and previos values are same */
		MTWF_PRINT("%s DPP is already %s\n", __func__, enable?"enabled":"disabled");
		return TRUE;
	}

	if (!enable)
		pAd->bDppEnable = FALSE;
	else
		pAd->bDppEnable = TRUE;

	MTWF_PRINT("%s: DPP is %s\n", __func__, pAd->bDppEnable?"enabled":"disabled");

	return TRUE;
}
#endif /* DPP_SUPPORT */

#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
VOID ch_switch_monitor_cfg_reset(IN PRTMP_ADAPTER pAd, struct ch_switch_cfg *cfg)
{
	MTWF_PRINT("%s:: enter\n", __func__);

	if (!cfg) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error! cfg is null!\n");
		return;
	}
	cfg->channel = 0;
	cfg->duration = 0;
	cfg->ch_sw_on_going = FALSE;
	cfg->wdev = NULL;
	cfg->ch_switch_sm.CurrState = CH_SWITCH_STATE_INIT;
	cfg->ioctl_if = -1;
}

INT ch_switch_monitor_cancel(IN PRTMP_ADAPTER pAd, struct wifi_dev *pwdev)
{
	BOOLEAN cancelled;
	UCHAR band_idx = 0;
	BOOLEAN ret = 0;
#ifdef MAP_R3
	INT ret_timeout = 0;
#endif /* MAP_R3 */
	MTWF_PRINT("%s:: enter\n", __func__);


	if ((!pAd) || (!pwdev)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error! pAd or pwdev is null!\n");
		return -EFAULT;
	}
	band_idx = HcGetBandByWdev(pwdev);
	RTMPCancelTimer(&pAd->ch_sw_cfg[band_idx].ch_sw_timer, &cancelled);
#ifdef MAP_R3
	if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd))
		RTMP_OS_INIT_COMPLETION(&pAd->ch_sw_cfg[band_idx].chan_switch_done_2);
#endif /* MAP_R3 */
	ret = MlmeEnqueueWithWdev(pAd, CH_SWITCH_MONITOR_STATE_MACHINE, CH_SWITCH_MSG_CANCLE, 0, NULL, 0, pwdev);
	if (ret)
		RTMP_MLME_HANDLER(pAd);
	else
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "enqueue MLME failed!\n");

#ifdef MAP_R3
	if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd)) {
		pAd->ch_sw_cfg[band_idx].wait_chan_switch_done_2 = 1;
		ret_timeout = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ch_sw_cfg[band_idx].chan_switch_done_2,
					RTMPMsecsToJiffies(10000));
		return ret_timeout;
	}
#endif /* MAP_R3 */

	return ret;
}

static VOID ch_switch_monitor_restore_last_ch(IN PRTMP_ADAPTER pAd, struct wifi_dev *pwdev)
{
	UCHAR band_idx = 0;
	struct ch_switch_cfg *ch_sw_info = NULL;

	MTWF_PRINT("%s:: enter\n", __func__);
	if ((!pAd) || (!pwdev)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error! pAd or pwdev is null!\n");
		return;
	}
	band_idx = HcGetBandByWdev(pwdev);
	ch_sw_info = &pAd->ch_sw_cfg[band_idx];
	if ((ch_sw_info) && (ch_sw_info->channel != (ch_sw_info->wdev)->channel))
		/*Restore to original channel & enable BCN Tx*/
		ch_switch_monitor_scan_ch_restore(pAd, OPMODE_AP, pwdev);
}

VOID ch_switch_monitor_timeout(
	IN PVOID system_specific1,
	IN PVOID function_context,
	IN PVOID system_specific2,
	IN PVOID system_specific3)
{
	BOOLEAN ret = 0;
	PTIMER_FUNC_CONTEXT pContext = (PTIMER_FUNC_CONTEXT)function_context;
	RTMP_ADAPTER *pAd;
	UCHAR band_idx = pContext->BandIdx;
	struct wifi_dev *wdev;

	MTWF_PRINT("%s:: enter\n", __func__);

	pAd = (RTMP_ADAPTER *)pContext->pAd;

	if (!pAd) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error! pAd is null!\n");
		return;
	}

	wdev = pAd->ch_sw_cfg[band_idx].wdev;

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error! wdev is null!\n");
		return;
	}

	ret = MlmeEnqueueWithWdev(pAd, CH_SWITCH_MONITOR_STATE_MACHINE, CH_SWITCH_MSG_TIMEOUT, 0, NULL, 0, wdev);
	if (ret)
		RTMP_MLME_HANDLER(pAd);
	else
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "enqueue MLME failed!\n");
}

static VOID ch_switch_monitor_listen_exit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR band_idx = 0;
	BOOLEAN ch_switch_done = TRUE;
	struct ch_switch_cfg *ch_sw_info = NULL;

	MTWF_PRINT("%s:: enter\n", __func__);

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"Failed! wdev is null!\n");
		return;
	}
	ch_switch_monitor_restore_last_ch(pAd, wdev);
	band_idx = HcGetBandByWdev(wdev);
	ch_sw_info = &pAd->ch_sw_cfg[band_idx];
#ifdef MAP_R3
	if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd)) {
		if (ch_sw_info->wait_chan_switch_done) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "from normal\n");
			RTMP_OS_COMPLETE(&ch_sw_info->chan_switch_done);
			ch_sw_info->wait_chan_switch_done = 0;
		}
		if (ch_sw_info->wait_chan_switch_done_2) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "from cancel timer\n");
			RTMP_OS_COMPLETE(&ch_sw_info->chan_switch_done_2);
			ch_sw_info->wait_chan_switch_done_2 = 0;
		}
	}
#endif /* MAP_R3 */
	ch_switch_monitor_cfg_reset(pAd, ch_sw_info);
	/* send event to userspace */
	RtmpOSWrielessEventSend(pAd->net_dev,
				RT_WLAN_EVENT_CUSTOM,
				CH_SWITCH_MONITOR_DONE_EVENT_FLAG,
				NULL,
				(char *)&ch_switch_done,
				sizeof(ch_switch_done));
}

static VOID ch_switch_monitor_listen_timeout(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *elem)
{
	struct wifi_dev *pwdev = elem->wdev;

	MTWF_PRINT("%s:: enter\n", __func__);

	if (!pAd || (!pwdev)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error! pwdev or pAd is NULL!\n");
		return;
	}
	ch_switch_monitor_listen_exit(pAd, pwdev);
}

static INT do_channel_switch_monitor(IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	UCHAR channel = 0;
	struct wifi_dev *target_wdev = NULL;
	struct wifi_dev *pwdev = NULL;
	UCHAR BssIdx = 0;
	UCHAR target_band_idx = 0;
	UCHAR band_idx_per_wdev = 0;
	struct ch_switch_cfg *ch_sw_info = NULL;

	MTWF_PRINT("%s:: enter\n", __func__);

	if (!pAd || (!wdev)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error! wdev or pAd is NULL!\n");
		return -1;
	}
	target_wdev = wdev;
	target_band_idx = HcGetBandByWdev(target_wdev);
	ch_sw_info = &pAd->ch_sw_cfg[target_band_idx];
	channel = ch_sw_info->channel;
	if (channel == target_wdev->channel) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"current channel is the target channel %d\n", channel);
		return -1;
	}

	/* Disable BCN */
	AsicDisableSync(pAd, HW_BSSID_0);
#ifdef CONFIG_AP_SUPPORT
	/* Disable beacon tx for target BSS */
	for (BssIdx = 0; BssIdx < pAd->ApCfg.BssidNum; BssIdx++) {
		pwdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;
		if (!pwdev)
			continue;
		band_idx_per_wdev = HcGetBandByWdev(pwdev);
		if (band_idx_per_wdev != target_band_idx)
			continue;
		if (pwdev->bAllowBeaconing)
			UpdateBeaconHandler(pAd, pwdev, BCN_UPDATE_DISABLE_TX);
	}
#endif /* CONFIG_AP_SUPPORT */
	/*switch channel*/
	wlan_operate_scan(target_wdev, channel);
#ifdef MAP_R3
	if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd)) {
		MTWF_PRINT("ch switch done %d\n", channel);
		RTMP_OS_COMPLETE(&ch_sw_info->chan_switch_done);
		ch_sw_info->wait_chan_switch_done = 0;
	}
#endif /* MAP_R3 */
	MTWF_PRINT("%s:: switch to channel %d\n", __func__, channel);

	return 0;
}

static VOID ch_switch_monitor_enter_listen(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *pElem)
{
	INT32 ret;
	UCHAR band_idx = 0;
	struct wifi_dev *pwdev = pElem->wdev;
	struct ch_switch_cfg *ch_sw_info = NULL;

	MTWF_PRINT("%s:: enter\n", __func__);

	if ((!pAd) || (!pwdev)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "error! pwdev or pAd is NULL!\n");
		return;
	}
	band_idx = HcGetBandByWdev(pwdev);
	ch_sw_info = &pAd->ch_sw_cfg[band_idx];
	/*switch channel*/
	ret = do_channel_switch_monitor(pAd, pwdev);
	if (ret != 0) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "fail by switch channel\n");
#ifdef MAP_R3
		if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "from ch change\n");
			RTMP_OS_COMPLETE(&ch_sw_info->chan_switch_done);
			ch_sw_info->wait_chan_switch_done = 0;
		}
#endif /* MAP_R3 */
		ch_switch_monitor_cfg_reset(pAd, ch_sw_info);
		return;
	}
	ch_sw_info->ch_switch_sm.CurrState = CH_SWITCH_STATE_RUNNING;
	RTMPSetTimer(&ch_sw_info->ch_sw_timer, ch_sw_info->duration);
}

static BOOLEAN ch_switch_monitor_sanity_check(IN PRTMP_ADAPTER pAd, UCHAR channel, struct wifi_dev *wdev)
{
	UCHAR ch_idx;
	UCHAR band_idx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);

	if (channel == 0) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"pass a invalid channel(0)\n");
		return FALSE;
	}

	/*check channel in channel list*/
	for (ch_idx = 0; ch_idx < pChCtrl->ChListNum; ch_idx++) {
		if (channel == pChCtrl->ChList[ch_idx].Channel)
			break;
	}

	if (ch_idx == pChCtrl->ChListNum) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"the channel parameter is out of channel list\n");
		return FALSE;
	}

	/*check channel & phy mode*/
	if ((channel < 14 && WMODE_CAP_5G(wdev->PhyMode)) ||
		(channel > 14 && WMODE_CAP_2G(wdev->PhyMode))) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"the channel parameter and phy mode is not matched\n");
		return FALSE;
	}

	return TRUE;
}

INT set_ch_switch_monitor_cfg(IN PRTMP_ADAPTER pAd, struct ch_switch_cfg *ch_sw_cfg)
{
	POS_COOKIE obj;
	INT ret = 0;
	struct wifi_dev *wdev;
	struct ch_switch_cfg *ch_sw_info = NULL;
	UCHAR band_idx = 0;

	MTWF_PRINT("%s:: enter\n", __func__);

	if (pAd == NULL)
		return -EFAULT;
	obj = (POS_COOKIE)pAd->OS_Cookie;
	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, obj->ioctl_if, obj->ioctl_if_type);
	if (wdev == NULL)
		return -EFAULT;
#ifdef SCAN_SUPPORT
	if (scan_in_run_state(pAd, wdev)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Failed! Scan is running, please try again after scan done!\n");
		return -EFAULT;
	}
#endif
	band_idx = HcGetBandByWdev(wdev);
	ch_sw_info = &pAd->ch_sw_cfg[band_idx];
	if (ch_sw_info == NULL)
		return -EFAULT;
	if ((ch_sw_info->ch_sw_on_going == TRUE) || (ch_sw_info->ch_switch_sm.CurrState == CH_SWITCH_STATE_RUNNING)) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"Failed! Please cancel last channel switch firstly!\n");
			return -EFAULT;
	}
	if (ch_switch_monitor_sanity_check(pAd, ch_sw_cfg->channel, wdev))
		ch_sw_info->channel = ch_sw_cfg->channel;
	else
		return -EFAULT;

	ch_sw_info->wdev = wdev;
	if (ch_sw_cfg->duration == 0)
		ch_sw_info->duration = CH_SWITCH_DFT_LISTEN_TIME;
	else
		ch_sw_info->duration = ch_sw_cfg->duration;
	ch_sw_info->ch_sw_on_going = TRUE;
#ifdef MAP_R3
	if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd))
		RTMP_OS_INIT_COMPLETION(&ch_sw_info->chan_switch_done);
#endif /* MAP_R3 */
	ret = MlmeEnqueueWithWdev(pAd, CH_SWITCH_MONITOR_STATE_MACHINE, CH_SWITCH_MSG_LISTEN, 0, NULL, wdev->func_idx, wdev);
	if (ret)
		RTMP_MLME_HANDLER(pAd);
	else {
		ch_sw_info->ch_sw_on_going = FALSE;
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "enqueue MLME failed!\n");
	}
#ifdef MAP_R3
	if (IS_MAP_ENABLE(pAd) && IS_MAP_R3_ENABLE(pAd)) {
		ch_sw_info->wait_chan_switch_done = 1;
		ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&ch_sw_info->chan_switch_done, RTMPMsecsToJiffies(10000));
	}
#endif /* MAP_R3 */

	return 0;
}

VOID ch_switch_monitor_exit(struct _RTMP_ADAPTER *pAd)
{
	BOOLEAN cancelled;
	UCHAR band_idx;
	struct ch_switch_cfg *ch_sw_cfg = NULL;

	MTWF_PRINT("%s:: Exit\n", __func__);

	if (pAd == NULL)
		return;
	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		ch_sw_cfg = &pAd->ch_sw_cfg[band_idx];
		RTMPCancelTimer(&ch_sw_cfg->ch_sw_timer, &cancelled);
		RTMPReleaseTimer(&ch_sw_cfg->ch_sw_timer, &cancelled);
		NdisZeroMemory(ch_sw_cfg, sizeof(struct ch_switch_cfg));
	}
}

VOID ch_switch_monitor_state_machine_init(struct _RTMP_ADAPTER *pAd)
{
	UCHAR band_idx;
	struct ch_switch_cfg *ch_sw_cfg = NULL;

	MTWF_PRINT("%s:: enter\n", __func__);

	if (pAd == NULL)
		return;
	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		ch_sw_cfg = &pAd->ch_sw_cfg[band_idx];
		ch_sw_cfg->ch_sw_timer_func_contex.pAd = pAd;
		ch_sw_cfg->ch_sw_timer_func_contex.BandIdx = band_idx;
		ch_switch_monitor_cfg_reset(pAd, ch_sw_cfg);
		StateMachineInit(&ch_sw_cfg->ch_switch_sm, ch_sw_cfg->ch_switch_state_func, CH_SWITCH_STATE_MAX, CH_SWITCH_MSG_MAX, (STATE_MACHINE_FUNC)Drop,
			CH_SWITCH_STATE_INIT, CH_SWITCH_STATE_BASE);
		StateMachineSetAction(&ch_sw_cfg->ch_switch_sm, CH_SWITCH_STATE_INIT, CH_SWITCH_MSG_LISTEN, (STATE_MACHINE_FUNC)ch_switch_monitor_enter_listen);
		StateMachineSetAction(&ch_sw_cfg->ch_switch_sm, CH_SWITCH_STATE_RUNNING, CH_SWITCH_MSG_CANCLE, (STATE_MACHINE_FUNC)ch_switch_monitor_listen_timeout);
		StateMachineSetAction(&ch_sw_cfg->ch_switch_sm, CH_SWITCH_STATE_RUNNING, CH_SWITCH_MSG_TIMEOUT, (STATE_MACHINE_FUNC)ch_switch_monitor_listen_timeout);
		RTMPInitTimer(pAd, &ch_sw_cfg->ch_sw_timer, GET_TIMER_FUNCTION(ch_switch_monitor_timeout), &ch_sw_cfg->ch_sw_timer_func_contex, FALSE);
	}
}

INT set_ch_switch_monitor_proc(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg)
{
	INT ret;
	struct ch_switch_cfg *ch_sw_cfg;
	INT32 i4Recv = 0;
	UINT32 target_ch = 0;
	UINT32 time = 0;/* ms */

	MTWF_PRINT("%s:: enter\n", __func__);

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d-%d", &(target_ch), &(time));
			if (i4Recv != 2) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "sscanf error!\n");
				return -1;
			}
			MTWF_PRINT("%s():target_ch = %d, time = %dms\n",
					 __func__, target_ch, time);
		} while (0);

		if ((target_ch == 0) || (time == 0)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong parameters!\n");
			return -1;
		} else
			MTWF_PRINT("%s():set parameters success!\n", __func__);
	}

	ret = os_alloc_mem(pAd, (UCHAR **)&ch_sw_cfg, sizeof(struct ch_switch_cfg));
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "alloc mem fail!\n");
		return FALSE;
	}

	NdisZeroMemory(ch_sw_cfg, sizeof(struct ch_switch_cfg));
	ch_sw_cfg->channel = target_ch;
	ch_sw_cfg->duration = time;
	set_ch_switch_monitor_cfg(pAd, ch_sw_cfg);
	os_free_mem(ch_sw_cfg);
	return TRUE;
}

INT cancel_ch_switch_monitor_proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg)
{
	UCHAR cancelled = 0;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;

	MTWF_PRINT("%s:: enter\n", __func__);

	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, obj->ioctl_if, obj->ioctl_if_type);
	if (wdev == NULL)
		return -EFAULT;

	cancelled = (UCHAR)os_str_tol(arg, 0, 10);
	if (cancelled)
		ch_switch_monitor_cancel(pAd, wdev);
	return TRUE;
}
#endif
#ifdef CFG_SUPPORT_CSI
/* example: iwpriv ra0 set csi=opt1-opt2-opt3-opt4 */
INT Set_CSI_Ctrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT_32 ret = 0;
	CHAR *par_value = NULL;
	UINT_8 ArgNum = 0;
	struct CMD_CSI_CONTROL_T *prCSICtrl = NULL;
	struct CSI_INFO_T *prCSIInfo = NULL;
	CHAR *mac_str = NULL;
	PCSI_STA pCSISta = NULL, pCSISta_tmp = NULL;
	INT idx;
	RTMP_STRING *tmp;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT apidx = pObj->ioctl_if;

	prCSIInfo = &pAd->rCSIInfo;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "Command\n");

	os_alloc_mem_suspend(NULL, (UCHAR **)&prCSICtrl, sizeof(struct CMD_CSI_CONTROL_T));

	if (!prCSICtrl) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"allocate memory for prCSICtrl failed!\n");
		ret = -1;
		goto out;
	}

	if (!arg || !strlen(arg)) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "arg string error!\n");
		ret = -1;
		goto out;
	}

	os_zero_mem(prCSICtrl, sizeof(struct CMD_CSI_CONTROL_T));

	/*at first,  check the band idx */
	if (pAd->ApCfg.MBSSID[apidx].wdev.channel > 14)
		prCSICtrl->BandIdx = BAND1;
	else
		prCSICtrl->BandIdx = BAND0;

	/* parameter parsing */
	for (ArgNum = 0, par_value = rstrtok(arg, "-"); par_value;
		 par_value = rstrtok(NULL, "-"), ArgNum++) {
		switch (ArgNum) {
		case 0:
			prCSICtrl->ucMode = os_str_toul(par_value, 0, 10);
			break;
		case 1:
			prCSICtrl->ucCfgItem = os_str_toul(par_value, 0, 10);
			break;
		case 2:
			prCSICtrl->ucValue1 = os_str_toul(par_value, 0, 10);
			break;
		case 3:
			prCSICtrl->ucValue2 = os_str_toul(par_value, 0, 10);
			break;
		case 4:
			tmp = NULL;
			mac_str = par_value;

			if (strlen(mac_str) < 17) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"check your mac addr format: xx:xx:xx:xx:xx:xx!\n");
				ret = -1;
				goto out;
			}

			/*macstr to hex*/
			for (idx = 0, tmp = rstrtok(mac_str, ":"); tmp; tmp = rstrtok(NULL, ":")) {
				if ((strlen(tmp) != 2) || (!isxdigit(*tmp)) || (!isxdigit(*(tmp + 1)))) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"illegal MAC address format or octet!\n");
					ret = -1;
					goto out;
				}

				AtoH(tmp, &prCSICtrl->mac_addr[idx++], 1);
			}
			break;
		default:
			break;
		}
	}

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_DEBUG, "parsing basic para is %d-%d-%d-%d.\n",
	prCSICtrl->ucMode, prCSICtrl->ucCfgItem, prCSICtrl->ucValue1, prCSICtrl->ucValue2);

	/*check arg num*/
	if (ArgNum < 1 || ArgNum > 5) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "argc %d is invalid!\n", ArgNum);
		ret = -1;
		goto out;
	}

	/*###check parameter 1###*/
	if (prCSICtrl->ucMode >= CSI_CONTROL_MODE_NUM) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"Invalid ucMode %d, should be (0-1-2-3).\n", prCSICtrl->ucMode);
		ret = -1;
		goto out;
	}

	/*normal case*/
	/*start or stop CSI*/
	if (prCSICtrl->ucMode == CSI_CONTROL_MODE_STOP ||
		prCSICtrl->ucMode == CSI_CONTROL_MODE_START) {
		prCSIInfo->bIncomplete = FALSE;
		prCSIInfo->u4CopiedDataSize = 0;
		prCSIInfo->u4RemainingDataSize = 0;
		prCSIInfo->u4CSIBufferHead = 0;
		prCSIInfo->u4CSIBufferTail = 0;
		prCSIInfo->u4CSIBufferUsed = 0;
		/*clear the CSI buffer*/
		os_zero_mem(prCSIInfo->arCSIBuffer, sizeof(struct CSI_DATA_T) * CSI_RING_SIZE);

		if (prCSICtrl->ucMode == CSI_CONTROL_MODE_STOP) {
			/*clear the list*/
			NdisAcquireSpinLock(&prCSIInfo->CSIStaListLock);
			DlListForEach(pCSISta, &prCSIInfo->CSIStaList, CSI_STA, List) {
				MTWF_PRINT("Remove csi sta, "MACSTR"\n", MAC2STR(pCSISta->Addr));
				pCSISta_tmp = pCSISta;
				pCSISta = DlListEntry(pCSISta->List.Prev, CSI_STA, List);
				DlListDel(&(pCSISta_tmp->List));
				os_free_mem(pCSISta_tmp);
			}
			NdisReleaseSpinLock(&prCSIInfo->CSIStaListLock);
			prCSIInfo->sta_sel_cnt = 0;
		}

		if (prCSICtrl->ucMode == CSI_CONTROL_MODE_START) {
			/*check report mode*/
			if (prCSICtrl->ucCfgItem == CSI_NETLINK) {
				prCSIInfo->CSI_report_mode = CSI_NETLINK;
				MTWF_PRINT("%s: CSI start with netlink mode!\n", __func__);
			} else if (prCSICtrl->ucCfgItem == CSI_PROC) {
				prCSIInfo->CSI_report_mode = CSI_PROC;
				MTWF_PRINT("%s: CSI start with proc mode(default)!\n", __func__);
			}

			HW_CSI_CTRL(pAd, (void *)prCSICtrl);		/*send fw cmd*/
			goto out;
		}
	}

	/*###check parameter 2###*/
	if (prCSICtrl->ucCfgItem >= CSI_CONFIG_ITEM_NUM) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		"Invalid csi cfg_item %u\n", prCSICtrl->ucCfgItem);
		ret = -1;
		goto out;
	}

	/*special case*/
	/*if usr need to use mac filter*/
	if ((prCSICtrl->ucCfgItem == CSI_CONFIG_FILTER_MODE) &&
		(prCSICtrl->ucValue1 == CSI_MAC_FILTER)) {
		if (prCSICtrl->ucValue2 == CSI_STA_MAC_ADD) {
			if (!mac_str) {
				MTWF_DBG(pAd,DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"CSI_MAC_FILTER with empty mac addr input!\n");
				ret = -1;
				goto out;
			}
			/*list full*/
			if (prCSIInfo->sta_sel_cnt >= Max_Sta_Mac_Num) {
				MTWF_DBG(pAd,DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"CSI List full!current:%d.\n", prCSIInfo->sta_sel_cnt);
				ret = -1;
				goto out;
			}

			/*search sta list*/
			NdisAcquireSpinLock(&prCSIInfo->CSIStaListLock);
			DlListForEach(pCSISta, &prCSIInfo->CSIStaList, CSI_STA, List) {
				if (NdisCmpMemory(pCSISta->Addr, prCSICtrl->mac_addr, MAC_ADDR_LEN) == 0) {
						MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Already in csi sta list:%02x:%02x:%02x:%02x:%02x:%02x\n",
							PRINT_MAC(pCSISta->Addr));
						NdisReleaseSpinLock(&prCSIInfo->CSIStaListLock);
						ret = -1;
						goto out;
				}
			}
			NdisReleaseSpinLock(&prCSIInfo->CSIStaListLock);

			/* new sta */
			pCSISta = NULL;
			os_alloc_mem_suspend(NULL, (UCHAR **)&pCSISta, sizeof(CSI_STA));
			if (!pCSISta) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"allocate memory for pCSISta failed!\n");
				ret = -1;
				goto out;
			}
			NdisZeroMemory(pCSISta, sizeof(CSI_STA));
			NdisCopyMemory(pCSISta->Addr, prCSICtrl->mac_addr, MAC_ADDR_LEN);
			NdisAcquireSpinLock(&prCSIInfo->CSIStaListLock);
			DlListAdd(&prCSIInfo->CSIStaList, &(pCSISta->List));
			NdisReleaseSpinLock(&prCSIInfo->CSIStaListLock);

			MTWF_PRINT("Add sta mac(%02x:%02x:%02x:%02x:%02x:%02x) to CSI List.\n",
				PRINT_MAC(pCSISta->Addr));

			prCSIInfo->sta_sel_cnt++;
			HW_CSI_CTRL(pAd, (void *)prCSICtrl);		/*send fw cmd*/
			goto out;
		} else if (prCSICtrl->ucValue2 == CSI_STA_MAC_DEL) {
			if (!mac_str) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"CSI_MAC_FILTER with empty mac addr input!!!\n");
				ret = -1;
				goto out;
			}
			/*list empty*/
			if (prCSIInfo->sta_sel_cnt == 0) {
				MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"CSI List is empty!current:%d.\n", prCSIInfo->sta_sel_cnt);
				ret = -1;
				goto out;
			}

			/*search sta list*/
			NdisAcquireSpinLock(&prCSIInfo->CSIStaListLock);
			DlListForEach(pCSISta, &prCSIInfo->CSIStaList, CSI_STA, List) {
			if (NdisCmpMemory(pCSISta->Addr, prCSICtrl->mac_addr, MAC_ADDR_LEN) == 0) {
				MTWF_PRINT("Del sta mac(%02x:%02x:%02x:%02x:%02x:%02x) from CSI List.\n",
					PRINT_MAC(pCSISta->Addr));
				pCSISta_tmp = pCSISta;
				pCSISta = DlListEntry(pCSISta->List.Prev, CSI_STA, List);
				DlListDel(&(pCSISta_tmp->List));
				os_free_mem(pCSISta_tmp);
				NdisReleaseSpinLock(&prCSIInfo->CSIStaListLock);
				prCSIInfo->sta_sel_cnt--;
				HW_CSI_CTRL(pAd, (void *)prCSICtrl);		/*send fw cmd*/
				goto out;
			}
			}

			/*not find*/
			NdisReleaseSpinLock(&prCSIInfo->CSIStaListLock);
			MTWF_DBG(pAd,DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,"Not find in CSI list.\n");
			ret = -1;
			goto out;
		} else if (prCSICtrl->ucValue2 == CSI_STA_MAC_SHOW) {
			/*search sta list*/
			NdisAcquireSpinLock(&prCSIInfo->CSIStaListLock);
			DlListForEach(pCSISta, &prCSIInfo->CSIStaList, CSI_STA, List) {
				MTWF_PRINT("=>Show csi sta mac:%02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(pCSISta->Addr));
			}
			NdisReleaseSpinLock(&prCSIInfo->CSIStaListLock);
			goto out;
		}
	}

	/*check invalid data*/
	/*check invalid chain mode 0~2, number 1~16*/
	if ((prCSICtrl->ucCfgItem == CSI_CONFIG_CHAIN_NUMBER) &&
		((prCSICtrl->ucValue1 > 2) || (prCSICtrl->ucValue2 > CSI_MAX_CHAIN_NUM))) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid cmd!\n");
		goto out;
	}

	/*check ping length filter 1~1500*/
	if ((prCSICtrl->ucCfgItem == CSI_CONFIG_FILTER_MODE) &&
		(prCSICtrl->ucValue1 == CSI_LENGTH_FILTER) &&
		((prCSICtrl->ucValue2 > CSI_MAX_PKT_BYTES) || (prCSICtrl->ucValue2 < 1))) {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "invalid pkt length!\n");
		goto out;
	}

	/*driver will filter specific pkt by TS, and usr will get a relatively precise result, like 10Hz.*/
	/*ex: usr will get all pkts with TS: Init + n* 100*1000 us */
	if ((prCSICtrl->ucCfgItem == CSI_CONFIG_FILTER_MODE) &&
		(prCSICtrl->ucValue1 == CSI_TS_FILTER)) {
		MTWF_PRINT("%s:set Ts filter length(%u us)!\n", __func__, prCSICtrl->ucValue2);
		if (prCSICtrl->ucValue2 >= CSI_MAX_TS_OFFSET)
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "At most 10s offset!\n");
		else
			prCSIInfo->usr_offset = prCSICtrl->ucValue2;
		goto out;
	}

	/*prCSICtrl => prCSIInfo */
	prCSIInfo->ucMode = prCSICtrl->ucMode;
	prCSIInfo->ucValue1[prCSICtrl->ucCfgItem] = prCSICtrl->ucValue1;
	prCSIInfo->ucValue2[prCSICtrl->ucCfgItem] = prCSICtrl->ucValue2;

	HW_CSI_CTRL(pAd, (void *)prCSICtrl);		/*send fw cmd*/
out:
	if (prCSICtrl)
		os_free_mem(prCSICtrl);
	return ret;
}
#endif

#ifdef WIFI_MD_COEX_SUPPORT
/**
* Set_CoexDebug_Level - seg debug log level in coex module.
* @pAd: pointer of the RTMP_ADAPTER
* @arg: dbg_level (0: disable; 1: Error; 2: Warning; 3: Trace.)
*
* This function is for feature debug
*
**/
INT Set_CoexDebug_Level(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR dbg_level;

	if (arg == NULL || strlen(arg) == 0)
		return FALSE;

	dbg_level = os_str_tol(arg, 0, 10);

	send_debug_level_to_wifi_md_coex(dbg_level);

	return TRUE;
}

#endif
INT32 rtmp_get_macPower(IN VOID *pAdSrc)
{
	UINT32 cmacVal = 0, omacVal = 0, val = 0, val0 = 0;
	INT32 retPwr = 0, oPwr = 0, cPwr = 0;
	UCHAR band = 0;
	PRTMP_ADAPTER ad = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE pObj = NULL;
	UCHAR apidx = 0;
	struct wifi_dev *wdev = NULL;

	if (ad == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "ad null!\n");
		return -EFAULT;
	}
	pObj = (POS_COOKIE)ad->OS_Cookie;
	if (pObj == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "pObj null!\n");
		return -EFAULT;
	}
	apidx = pObj->ioctl_if;
	/* For all ioctl to this function, we assume that's query for AP/APCLI/GO device */
	if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
		if (apidx >= ad->ApCfg.BssidNum)
			return -EFAULT;
		wdev = &ad->ApCfg.MBSSID[apidx].wdev;
	}
	if (wdev)
		band = HcGetBandByWdev(wdev);
	else
		band = BAND0;
	if (band == BAND0) {
		MAC_IO_READ32(ad->hdev_ctrl, 0x21024, &omacVal);
		MAC_IO_READ32(ad->hdev_ctrl, 0x21020, &cmacVal);
	} else {
		MAC_IO_READ32(ad->hdev_ctrl, 0xa1024, &omacVal);
		MAC_IO_READ32(ad->hdev_ctrl, 0xa1020, &cmacVal);
	}
		/*Unit: 0.5dBm*/
		/*The encoding rule is 2's complement.*/
		/*7'b0000000: 0dBm*/
		/*7'b0000001: 0.5dBm*/
		/*7'b0111111: 31.5dBm*/
		/*7'b1111111: -0.5dBm*/
		/*7'b1000000: -32dBm*/
	val = (cmacVal & 0xff);
	if (val <= 0x3f)
		cPwr = ((INT32)val >> 1);
	else
		cPwr = (((INT32)(val - 0x80) >> 1) | 0x80000000);
	val0 = (omacVal & 0xff);
	if (val0 <= 0x3f)
		oPwr = ((INT32)val0 >> 1);
	else
		oPwr = (((INT32)(val0 - 0x80) >> 1) | 0x80000000);

	retPwr = ((oPwr > cPwr) ? oPwr : cPwr);
	MTWF_PRINT("Got %s Power %d\n", (band == BAND0)?"2G":"5G", retPwr);
	return retPwr;
}

#ifdef CONFIG_6G_SUPPORT
INT show_bssmnger_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n%s:\n", __func__);
	// bss_mgmt_show_info();
	bssmnger_show_bsslist_info();
	return TRUE;
}

BOOLEAN in_band_radioinfo_update(struct wifi_dev *wdev)
{
	struct wifi_dev *tdev;
	struct _RTMP_ADAPTER *pAd;
	UCHAR i = 0, band_idx;

	if (wdev == NULL)
		return FALSE;

	band_idx = HcGetBandByWdev(wdev);
	pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	/* sync to bss manager */
	for (i = 0; i < WDEV_NUM_MAX; i++) {

		tdev = pAd->wdev_list[i];

		if (tdev == NULL)
			continue;

		if (HcIsRadioAcq(tdev) && (band_idx == HcGetBandByWdev(tdev) && (WDEV_BSS_STATE(tdev) == BSS_READY))) {
			bssmnger_update_radio_info(tdev);
#ifdef BSSMGR_CROSS_MODULE_SUPPORT
			/* Send event to wappd*/
			indicate_bssentry_to_wappd(tdev, BSSMNGER_MSG_UPDATE_RADIO);
#endif /* BSSMGR_CROSS_MODULE_SUPPORT */
		}
	}


	return TRUE;
}

BOOLEAN in_band_discovery_update(struct wifi_dev *wdev,
		UCHAR type, UCHAR interval, UCHAR tx_mode, UCHAR by_cfg)
{
	wlan_config_set_unsolicit_tx_type(wdev, type);
	wlan_config_set_unsolicit_tx_tu(wdev, interval);
	wlan_config_set_unsolicit_tx_mode(wdev, tx_mode);
	wlan_config_set_unsolicit_tx_by_cfg(wdev, by_cfg);

	if (WDEV_BSS_STATE(wdev) == BSS_READY) {
		/* sync to bss manager */
		// bss_mgmt_set_discovery_rules(wdev);
		bssmnger_update_discovery_rule(wdev);
#ifdef BSSMGR_CROSS_MODULE_SUPPORT
		/* Send event to wappd*/
		indicate_bssentry_to_wappd(wdev, BSSMNGER_MSG_IOB_DISCOVERY);
#endif /* BSSMGR_CROSS_MODULE_SUPPORT */
		/* sync discovery frame, enable beacon */
		UpdateBeaconHandler(wdev->sys_handle, wdev, BCN_UPDATE_ENABLE_TX);
	}

	return TRUE;
}

BOOLEAN qos_injector_update(struct wifi_dev *wdev,
		UCHAR interval, UCHAR state)
{
	wlan_config_set_qos_tx_tu(wdev, interval);
	wlan_config_set_qos_tx_state(wdev, state);

	if (WDEV_BSS_STATE(wdev) == BSS_READY) {
		/* sync discovery frame, enable beacon */
		UpdateBeaconHandler(wdev->sys_handle, wdev, BCN_UPDATE_ENABLE_TX);
	}

	return TRUE;
}

BOOLEAN out_band_discovery_update(struct wifi_dev *wdev,
		UCHAR rnr2g, UCHAR rnr5g, UCHAR rnr6g)
{
	wlan_config_set_rnr_in_probe_rsp(wdev, rnr2g, rnr5g, rnr6g);

	if (WDEV_BSS_STATE(wdev) == BSS_READY) {
		/* sync to bss manager */
		// bss_mgmt_set_discovery_rules(wdev);
		bssmnger_update_discovery_rule(wdev); // Ryan
#ifdef BSSMGR_CROSS_MODULE_SUPPORT
		/* Send event to wappd*/
		indicate_bssentry_to_wappd(wdev, BSSMNGER_MSG_OOB_DISCOVERY);
#endif /* BSSMGR_CROSS_MODULE_SUPPORT */
	}

	return TRUE;
}

INT set_6g_ap_iob_discovery_proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING		*arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	BOOLEAN fgStatus = FALSE;
	INT type, interval, txmode;
	INT32 i4Recv = 0;

	if (arg) {
		i4Recv = sscanf(arg, "%d-%d-%d", &(type), &(interval), &(txmode));
		if (i4Recv == 3 && type <= UNSOLICIT_TX_FILS_DISC) {
			MTWF_PRINT("type=%d, interval=%d, txmode=%d\n",
					 type, interval, txmode);
			fgStatus = in_band_discovery_update(wdev, type, interval, txmode, TRUE);
		}
	}

	if (fgStatus == FALSE) {
		MTWF_PRINT("iwpriv ra0 set 6giob=[Type]-[Interval]-[TxMode]\n");
		MTWF_PRINT("[Type]     0:Off, 1:BC.Probe.Rsp, 2:FILS.Discovery\n");
		MTWF_PRINT("[Interval] unit in TUs\n");
		MTWF_PRINT("[TxMode]   0:RATE_NON_HT, 1:RATE_NON_HT_DUP, 2:RATE_HE_SU\n");
	}

	return TRUE;
}

INT set_6g_ap_qos_injector_proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING		* arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	BOOLEAN fgStatus = FALSE;
	INT state, interval;
	INT32 i4Recv = 0;

	if (arg) {
		i4Recv = sscanf(arg, "%d-%d", &(interval), &(state));
		if (i4Recv == 2) {
			MTWF_PRINT("interval=%d, state=%d\n",
					 interval, state);
			fgStatus = qos_injector_update(wdev, interval, state);
		}
	}

	if (fgStatus == FALSE) {
		MTWF_PRINT("iwpriv ra0 set 6gqos=[Interval]-[State]\n");
		MTWF_PRINT("[Interval] unit in TUs\n");
		MTWF_PRINT("[State]   0:Disable, 1:Enable\n");
	}

	return TRUE;
}


INT set_6g_ap_oob_discovery_proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING		*arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	BOOLEAN fgStatus = FALSE;
	INT rnr2g, rnr5g, rnr6g;
	INT32 i4Recv = 0;

	if (arg) {
		i4Recv = sscanf(arg, "%d-%d-%d", &(rnr2g), &(rnr5g), &(rnr6g));
		if (i4Recv == 3) {
			MTWF_PRINT("RNR(2G:5G:6G)=(%d:%d:%d)\n", rnr2g, rnr5g, rnr6g);
			fgStatus = out_band_discovery_update(wdev, rnr2g, rnr5g, rnr6g);
		}
	}

	if (fgStatus == FALSE) {
		MTWF_PRINT("iwpriv ra0 set 6goob=[RNR2G]-[RNR5G]-[RNR6G]\n");
		MTWF_PRINT("[RNR2G/5G/6G] 0:Off, 1:MainBSS, 2:AllBSSs\n");
	}

	return TRUE;
}
#endif /* CONFIG_6G_SUPPORT */

#if defined(DOT11_HE_AX) && defined(FIXED_HE_GI_SUPPORT)
INT set_fgi_and_ltf_profile(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT32 value)
{
	UINT8 gi_idx = 0;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "enter\n");

	if (wdev == NULL)
		return -EFAULT;

	if (value == 0)
		gi_idx = GI_AUTO;
	else if (value == 800)
		gi_idx = FIXED_GI_08_US;
	else if (value == 1600)
		gi_idx = FIXED_GI_16_US;
	else if (value == 3200)
		gi_idx = FIXED_GI_32_US;
	else {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Invalid gi parameter(%d)!!!\n", value);
		return -1;
	}

	wlan_config_set_he_gi(wdev, gi_idx);
	MTWF_PRINT("set target_gi(%d)\n", value);

	return TRUE;
}

INT set_fgi_and_ltf_proc(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;
	INT32 i4Recv = 0;
	UINT32 target_gi = 0;
	UINT8 gi_idx = 0;

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "enter\n");

	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, obj->ioctl_if, obj->ioctl_if_type);
	if (wdev == NULL)
		return -EFAULT;

	if (arg) {
		do {
			i4Recv = sscanf(arg, "%d", &(target_gi));
		} while (0);

		if (((target_gi != 0) && (target_gi != 800) && (target_gi != 1600) && (target_gi != 3200))
			||(i4Recv != 1)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Wrong parameters!!!\n");
			return -1;
		} else {
			if (target_gi == 0)
				gi_idx = GI_AUTO;
			else if (target_gi == 800)
				gi_idx = FIXED_GI_08_US;
			else if (target_gi == 1600)
				gi_idx = FIXED_GI_16_US;
			else
				gi_idx = FIXED_GI_32_US;

			wlan_config_set_he_gi(wdev, gi_idx);
			MTWF_PRINT("set target_gi(%d)\n", target_gi);
		}
	}
	return TRUE;
}

INT show_fgi_and_ltf_proc(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;
	UINT32 cur_gi = 0;
	PCHAR gi_ltf_info[4] = {
	"Auto_GI",
	"GI_08_US + LTF_2x",
	"GI_16_US + LTF_2x",
	"GI_32_US + LTF_4x"
	};

	MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, "enter\n");

	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, obj->ioctl_if, obj->ioctl_if_type);
	if (wdev == NULL)
		return -EFAULT;
	cur_gi = wlan_config_get_he_gi(wdev);

	if (cur_gi > FIXED_GI_32_US) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Error!!!(cur_gi(%d) is invalid!\n", cur_gi);
		return -EFAULT;
	}

	MTWF_PRINT("Current_gi(%s)\n", gi_ltf_info[cur_gi]);

	return TRUE;
}
#endif
#ifdef VLAN_SUPPORT
INT Set_Vlan2eth_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	char *arg)
{
	UCHAR vlan2ethctrl = FALSE;

	vlan2ethctrl = simple_strtol(arg, 0, 10);
	if ((vlan2ethctrl == FALSE) || (vlan2ethctrl == TRUE)) {
		pAd->tr_ctl.vlan2ethctrl = vlan2ethctrl;
		MTWF_PRINT("%s():%d\n", __func__, pAd->tr_ctl.vlan2ethctrl);
	} else
		MTWF_PRINT("%s():Parameter is not valid.\n", __func__);
	return TRUE;
}
#endif

#ifdef CONFIG_MAP_SUPPORT
INT Set_BS_ProbeEvent_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	if (os_str_tol(arg, 0, 10) == 1)
		pAd->ApCfg.Disallow_ProbeEvent = TRUE;
	else
		pAd->ApCfg.Disallow_ProbeEvent = FALSE;

	return TRUE;
}
#endif

BOOLEAN wdev_down_exec_ioctl(RTMP_IOCTL_INPUT_STRUCT *wrq, USHORT subcmd)
{
	USHORT cmd = 0;
	BOOLEAN retval = FALSE;
	UCHAR *buf = NULL;
	INT Status;

	cmd = subcmd;
	if (cmd & OID_GET_SET_TOGGLE) {
		switch (cmd & 0x7FFF) {
#ifdef WAPP_SUPPORT
		case OID_WAPP_EVENT:{
			struct wapp_req *req;
			os_alloc_mem(NULL, (UCHAR **)&buf, wrq->u.data.length);
			if (buf == NULL) {
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"alloc memory fail!\n");
				break;
			}
			Status = copy_from_user(buf, wrq->u.data.pointer, wrq->u.data.length);
			if (Status != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"copy_from_user return status received = failure\n");
				break;
			}
			req = (struct wapp_req *)buf;
			if (req && (req->req_id == WAPP_BSS_START_REQ))
				retval = TRUE;
			break;
		}
#endif
		default:
			break;
		}
	}

	if (buf)
		os_free_mem(buf);

	return retval;
}

