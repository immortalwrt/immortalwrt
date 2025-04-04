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
	cmm_asic.h

	Abstract:
	Ralink Wireless Chip HW related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __ASIC_CTRL_H__
#define __ASIC_CTRL_H__

#ifdef MT_MAC
#include "hw_ctrl/cmm_asic_mt.h"
#include "hw_ctrl/cmm_asic_mt_fw.h"
#include "hw_ctrl/cmm_asic_mt_fmac.h"
#endif /* MT_MAC */

#include "common/wifi_sys_info.h"

struct _TX_BLK;
struct _RX_BLK;
struct freq_oper;
struct wmm_entry;

enum PACKET_TYPE;

#define TX_RTY_CFG_RTY_LIMIT_SHORT		0x1
#define TX_RTY_CFG_RTY_LIMIT_LONG		0x2

#define MINIMUM_POWER_VALUE            -127
#define TX_STREAM_PATH                    4
#define RX_STREAM_PATH_SINGLE_MODE        4
#define RX_STREAM_PATH_DBDC_MODE          2

#define CHANNEL_BAND_2G                   0
#define CHANNEL_BAND_5G                   1

#define CMW_RSSI_SOURCE_BBP               0
#define CMW_RSSI_SOURCE_WTBL              1

#define CMW_RCPI_MA_1_1                   1
#define CMW_RCPI_MA_1_2                   2
#define CMW_RCPI_MA_1_4                   4
#define CMW_RCPI_MA_1_8                   8

#define TX_DEFAULT_CSD_STATE              0
#define TX_ZERO_CSD_STATE                 1
#define TX_UNDEFINED_CSD_STATE            0xFF

#define TX_DEFAULT_POWER_STATE            0
#define TX_BOOST_POWER_STATE              1
#define TX_UNDEFINED_POWER_STATE          0xFF

#define TX_DEFAULT_BW_STATE               0
#define TX_SWITCHING_BW_STATE             1
#define TX_UNDEFINED_BW_STATE             0xFF

#define TX_DEFAULT_SPEIDX_STATE           0
#define TX_SWITCHING_SPEIDX_STATE         1
#define TX_UNDEFINED_SPEIDX_STATE         0xFF

#define TX_DEFAULT_MAXIN_STATE            0
#define TX_SPECIFIC_ACR_STATE             1
#define TX_UNDEFINED_RXFILTER_STATE       0xFF

#define RX_DEFAULT_RCPI_STATE             0
#define RX_SPECIFIC_RCPI_STATE            1
#define RX_UNDEFINED_RCPI_STATE           0xFF

#define RX_DEFAULT_RXSTREAM_STATE         15
#define RX_RXSTREAM_WF0_STATE             1
#define RX_RXSTREAM_WF1_STATE             2
#define RX_RXSTREAM_WF2_STATE             4
#define RX_RXSTREAM_WF3_STATE             8
#define RX_RXSTREAM_WF01_STATE            3
#define RX_RXSTREAM_WF02_STATE            5
#define RX_RXSTREAM_WF03_STATE            9
#define RX_RXSTREAM_WF12_STATE            6
#define RX_RXSTREAM_WF13_STATE            10
#define RX_RXSTREAM_WF23_STATE            12
#define RX_RXSTREAM_WF012_STATE           7
#define RX_RXSTREAM_WF013_STATE           11
#define RX_RXSTREAM_WF023_STATE           13
#define RX_RXSTREAM_WF123_STATE           14
#define RX_UNDEFINED_RXSTREAM_STATE       0xFF

#define CMW_POWER_UP_RATE_NUM             13
#define CMW_POWER_UP_CATEGORY_NUM         4

#define LINK_TEST_AUTO_RSSI_THRESHOLD     0xFF

#ifdef LINK_TEST_SUPPORT
typedef enum _ENUM_RSSI_CHECK_SOURCE {
	RSSI_CHECK_WTBL_RSSI = 0,
	RSSI_CHECK_BBP_WBRSSI,
	RSSI_CHECK_BBP_IBRSSI,
	RSSI_CHECK_NUM
} ENUM_RSSI_CHECK_SOURCE, *P_ENUM_RSSI_CHECK_SOURCE;

typedef enum _ENUM_RSSI_REASON {
	RSSI_REASON_SENSITIVITY = 0,
	RSSI_REASON_RX_BLOCKING,
	RSSI_REASON_NUM
} ENUM_RSSI_REASON, *P_ENUM_RSSI_REASON;
#endif /* LINK_TEST_SUPPORT */

VOID AsicNotSupportFunc(struct _RTMP_ADAPTER *pAd, const RTMP_STRING *caller);


VOID AsicUpdateRtsThld(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 PktNumThrd, UINT32 PpduLengthThrd);
VOID AsicUpdateProtect(struct _RTMP_ADAPTER *pAd, struct prot_info *prot);

INT AsicSetTxStream(struct _RTMP_ADAPTER *pAd, UINT32 StreamNum, UCHAR opmode, BOOLEAN up, UCHAR BandIdx);
INT AsicSetRxStream(struct _RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx);
INT AsicSetBW(struct _RTMP_ADAPTER *pAd, INT bw, UCHAR BandIdx);
INT AsicSetCtrlCh(struct _RTMP_ADAPTER *pAd, UINT8 extch);
#ifdef DOT11_VHT_AC
INT AsicSetRtsSignalTA(struct _RTMP_ADAPTER *pAd, UCHAR bw_sig);
#endif /* DOT11_VHT_AC */
VOID AsicAntennaSelect(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
VOID AsicBBPAdjust(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
VOID AsicSwitchChannel(struct _RTMP_ADAPTER *pAd, UCHAR band_idx, struct freq_oper *oper, BOOLEAN bScan);

#ifdef CONFIG_STA_SUPPORT
VOID AsicSleepAutoWakeup(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg);
VOID AsicWakeup(struct _RTMP_ADAPTER *pAd, BOOLEAN bFromTx, struct _STA_ADMIN_CONFIG *pStaCfg);
#endif /* CONFIG_STA_SUPPORT */

VOID AsicSetBssid(struct _RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR curr_bssid_idx);
VOID AsicDelWcidTab(struct _RTMP_ADAPTER *pAd, UINT16 Wcid);

#ifdef HTC_DECRYPT_IOT
VOID AsicSetWcidAAD_OM(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, CHAR value);
#endif /* HTC_DECRYPT_IOT */
VOID AsicSetWcidPsm(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, UCHAR value);
VOID AsicSetWcidSN(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, UINT16 Sn);
#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
VOID AsicSetWcid4Addr_HdrTrans(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, UCHAR IsEnable);
#endif


#ifdef MAC_APCLI_SUPPORT
VOID AsicSetApCliBssid(struct _RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR index);
#endif /* MAC_APCLI_SUPPORT */

INT AsicSetRxFilter(struct _RTMP_ADAPTER *pAd);

VOID AsicSetTmrCR(struct _RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx);

#ifdef DOT11_N_SUPPORT
INT AsicSetRDG(struct _RTMP_ADAPTER *pAd,
			   UINT16 wlan_idx, UCHAR band_idx, UCHAR init, UCHAR resp);
#ifdef MT_MAC
INT AsicWtblSetRDG(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable);
INT AsicUpdateTxOP(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 ac_num, UINT32 txop_val);
#endif /* MT_MAC */
#endif /* DOT11_N_SUPPORT */

INT AsicSetPreTbtt(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR HwBssidIdx);
INT AsicSetGPTimer(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout);
INT AsicSetChBusyStat(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
INT AsicGetTsfTime(
	struct _RTMP_ADAPTER *pAd,
	UINT32 *high_part,
	UINT32 *low_part,
	UCHAR HwBssidIdx);

VOID AsicSetSyncModeAndEnable(
	struct _RTMP_ADAPTER *pAd,
	USHORT BeaconPeriod,
	UCHAR HWBssidIdx,
	UCHAR OPMode);

VOID AsicDisableSync(struct _RTMP_ADAPTER *pAd, UCHAR HWBssidIdx);

#ifdef CONFIG_STA_SUPPORT
VOID AsicEnableIbssSync(
	struct _RTMP_ADAPTER *pAd,
	USHORT BeaconPeriod,
	UCHAR HWBssidIdx,
	UCHAR OPMode);
#endif

UINT32 AsicGetWmmParam(struct _RTMP_ADAPTER *pAd, UINT32 ac, UINT32 type);
#ifdef WIFI_UNIFIED_COMMAND
INT AsicSetWmmParam(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR idx, UINT ac, UINT type, UINT val);
#else
INT AsicSetWmmParam(struct _RTMP_ADAPTER *pAd, UCHAR idx, UINT ac, UINT type, UINT val);
#endif /* WIFI_UNIFIED_COMMAND */
VOID AsicSetEdcaParm(struct _RTMP_ADAPTER *pAd, struct wmm_entry *entry, struct wifi_dev *wdev);
INT AsicSetRetryLimit(struct _RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit);
UINT32 AsicGetRetryLimit(struct _RTMP_ADAPTER *pAd, UINT32 type);
VOID AsicSetSlotTime(struct _RTMP_ADAPTER *pAd, BOOLEAN bUseShortSlotTime, UCHAR channel, struct wifi_dev *wdev);
INT AsicSetMacMaxLen(struct _RTMP_ADAPTER *pAd);

VOID AsicAddSharedKeyEntry(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR BssIdx,
	IN UCHAR KeyIdx,
	IN PCIPHER_KEY pCipherKey);

VOID AsicRemoveSharedKeyEntry(struct _RTMP_ADAPTER *pAd, UCHAR BssIdx, UCHAR KeyIdx);

VOID AsicUpdateRxWCIDTable(struct _RTMP_ADAPTER *pAd, USHORT WCID, UCHAR *pAddr, BOOLEAN IsBCMCWCID, BOOLEAN IsReset);
VOID AsicUpdateBASession(struct _RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR tid, UINT16 sn, UINT16 basize, BOOLEAN isAdd, INT ses_type, UCHAR amsdu);

#ifdef TXBF_SUPPORT
VOID AsicUpdateClientBfCap(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#endif /* TXBF_SUPPORT */

#ifdef MT_MAC
VOID AsicAddRemoveKeyTab(struct _RTMP_ADAPTER *pAd, struct _ASIC_SEC_INFO *pInfo);
#endif

#ifdef CONFIG_AP_SUPPORT
VOID AsicSetWdevIfAddr(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT opmode);
#endif /* CONFIG_AP_SUPPORT */

BOOLEAN AsicDisableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev);
BOOLEAN AsicEnableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev);
BOOLEAN AsicUpdateBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev, BOOLEAN BcnSntReq, UCHAR UpdateReason);

INT32 AsicDevInfoUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucOwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 u4EnableFeature);

INT32 AsicStaRecUpdate(
	RTMP_ADAPTER * pAd,
	STA_REC_CTRL_T *sta_rec_ctrl);

INT32 AsicRaParamStaRecUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16 WlanIdx,
	struct _STAREC_AUTO_RATE_UPDATE_T *prParam,
	UINT32 EnableFeature);

INT32 AsicBssInfoUpdate(
	struct _RTMP_ADAPTER *pAd,
	struct _BSS_INFO_ARGUMENT_T *bss_info_argument);

#define AsicBssInfoReNew(pAd, bss_info_argument) AsicBssInfoUpdate(pAd, bss_info_argument)

INT32 AsicExtPwrMgtBitWifi(struct _RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT8 ucPwrMgtBit);
INT32 AsicRadioOnOffCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 ucRadio);
#ifdef GREENAP_SUPPORT
INT32 AsicGreenAPOnOffCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN ucGreenAPOn);
#endif /* GREENAP_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
INT32 asic_pcie_aspm_dym_ctrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN fgL1Enable, BOOLEAN fgL0sEnable);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
INT32 asic_twt_agrt_update(struct _RTMP_ADAPTER *ad, struct twt_agrt_para twt_agrt_para);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
INT32 AsicExtPmStateCtrl(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg, UINT8 ucPmNumber, UINT8 ucPmState);
INT32 AsicExtWifiHifCtrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 PmStatCtrl, VOID *pResult);

INT32 AsicMccStart(struct _RTMP_ADAPTER *ad,
				   UCHAR channel_1st,
				   UCHAR channel_2nd,
				   UINT32 bw_1st,
				   UINT32 bw_2nd,
				   UCHAR central_1st_seg0,
				   UCHAR central_1st_seg1,
				   UCHAR central_2nd_seg0,
				   UCHAR central_2nd_seg1,
				   UCHAR role_1st,
				   UCHAR role_2nd,
				   USHORT stay_time_1st,
				   USHORT stay_time_2nd,
				   USHORT idle_time,
				   USHORT null_repeat_cnt,
				   UINT32 start_tsf);

INT32 AsicBfSoundingPeriodicTriggerCtrl(struct _RTMP_ADAPTER *pAd, UINT32 WlanIdx, UINT8 On);
INT32 AsicThermalProtect(
	RTMP_ADAPTER * pAd,
	UINT8 ucBand,
	UINT8 HighEn,
	CHAR HighTempTh,
	UINT8 LowEn,
	CHAR LowTempTh,
	UINT32 RechkTimer,
	UINT8 RFOffEn,
	CHAR RFOffTh,
	UINT8 ucType);

INT32 AsicThermalProtectAdmitDuty(
	RTMP_ADAPTER * pAd,
	UINT8 ucBand,
	UINT32 u4Lv0Duty,
	UINT32 u4Lv1Duty,
	UINT32 u4Lv2Duty,
	UINT32 u4Lv3Duty
);

INT AsicThermalProtectAdmitDutyInfo(
    struct _RTMP_ADAPTER *pAd
);

INT AsicSendCommandToMcu(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR         Command,
	IN UCHAR         Token,
	IN UCHAR         Arg0,
	IN UCHAR         Arg1,
	IN BOOLEAN in_atomic);

BOOLEAN AsicSendCmdToMcuAndWait(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic);


#ifdef STREAM_MODE_SUPPORT
VOID AsicSetStreamMode(
	struct _RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddr,
	IN INT chainIdx,
	IN BOOLEAN bEnabled);

VOID AsicStreamModeInit(struct _RTMP_ADAPTER *pAd);
#endif /*STREAM_MODE_SUPPORT*/

INT AsicSetAutoFallBack(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
INT AsicAutoFallbackInit(struct _RTMP_ADAPTER *pAd);

VOID AsicSetPiggyBack(struct _RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack);
VOID AsicGetTxTsc(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 pn_type_mask, UCHAR *pTxTsc);
VOID AsicSetSMPS(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, UCHAR smps);
VOID AsicTurnOffRFClk(struct _RTMP_ADAPTER *pAd, UCHAR Channel);

#ifdef MAC_REPEATER_SUPPORT
VOID AsicInsertRepeaterEntry(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR CliIdx,
	IN PUCHAR pAddr);

VOID AsicRemoveRepeaterEntry(
	struct _RTMP_ADAPTER *pAd,
	IN UCHAR CliIdx);

#ifdef MT_MAC
VOID AsicInsertRepeaterRootEntry(
	struct _RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	IN  UCHAR *pAddr,
	IN UCHAR ReptCliIdx);
#endif /* MT_MAC */
#endif /* MAC_REPEATER_SUPPORT*/

INT32 AsicRxHeaderTransCtl(struct _RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan, BOOLEAN SwPcP);
INT32 AsicRxHeaderTaranBLCtl(struct _RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType);
INT AsicSetRxvFilter(RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR ucBandIdx);
#ifdef VLAN_SUPPORT
INT32 asic_update_vlan_id(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT8 omac_idx, UINT16 vid);
INT32 asic_update_vlan_priority(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT8 omac_idx, UINT8 priority);
#endif

#ifdef CONFIG_AP_SUPPORT
VOID AsicSetMbssHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
VOID AsicSetExtMbssEnableCR(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
VOID AsicSetExtTTTTHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
#endif /* CONFIG_AP_SUPPORT */
INT32 AsicGetAntMode(struct _RTMP_ADAPTER *pAd, UCHAR *AntMode);

#ifdef DBDC_MODE
INT32 AsicSetDbdcCtrl(struct _RTMP_ADAPTER *pAd, struct _BCTRL_INFO_T *pBctrlInfo);
INT32 AsicGetDbdcCtrl(struct _RTMP_ADAPTER *pAd, struct _BCTRL_INFO_T *pBctrlInfo);
#endif /*DBDC_MODE*/

VOID AsicNotSupportFunc(struct _RTMP_ADAPTER *pAd, const RTMP_STRING *caller);

#ifdef IGMP_SNOOP_SUPPORT
BOOLEAN AsicMcastEntryInsert(struct _RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, UINT8 Type, PUCHAR MemberAddr, PNET_DEV dev, UINT16 wcid);
BOOLEAN AsicMcastEntryDelete(struct _RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, PUCHAR MemberAddr, PNET_DEV dev, UINT16 wcid);
#ifdef IGMP_SNOOPING_DENY_LIST
BOOLEAN AsicMcastEntryDenyList(struct _RTMP_ADAPTER *pAd, PNET_DEV dev, UINT8 entry_cnt, UINT8 add_to_list, UINT8 * pAddr);
#endif
#ifdef IGMP_TVM_SUPPORT
BOOLEAN AsicMcastConfigAgeOut(RTMP_ADAPTER *pAd, UINT8 AgeOutTime, UINT8 omac_idx);
BOOLEAN AsicMcastGetMcastTable(RTMP_ADAPTER *pAd, UINT8 ucOwnMacIdx, struct wifi_dev *wdev);
#endif /* IGMP_TVM_SUPPORT */
#endif
VOID RssiUpdate(struct _RTMP_ADAPTER *pAd);
UINT32 rtmp_get_rssi(RTMP_ADAPTER *pAd, UINT16 Wcid, CHAR *rssi, UINT8 rssi_len);
VOID SnrUpdate(struct _RTMP_ADAPTER *pAd);
UINT32 rtmp_get_snr(RTMP_ADAPTER *pAd, UINT16 Wcid, CHAR *snr, UINT8 snr_len);

#ifdef ETSI_RX_BLOCKER_SUPPORT
VOID CheckRssi(struct _RTMP_ADAPTER *pAd);
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */
char *get_bw_str(int bandwidth);
VOID AsicFeLossGet(struct _RTMP_ADAPTER *pAd, UCHAR channel, CHAR *RssiOffset);
VOID AsicTxCapAndRateTableUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16 u2Wcid,
	RA_PHY_CFG_T *prTxPhyCfg,
	UINT32 *Rate,
	BOOL fgSpeEn);
VOID AsicUpdateRxWCIDTableDetail(struct _RTMP_ADAPTER *pAd, MT_WCID_TABLE_INFO_T WtblInfo);
INT AsicGetTsfTime(struct _RTMP_ADAPTER *pAd, UINT32 *high_part, UINT32 *low_part, UCHAR hw_bssid);
VOID AsicTxCntUpdate(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, MT_TX_COUNTER *pTxInfo);
INT asic_rts_on_off(struct wifi_dev *wdev, BOOLEAN rts_en);
INT asic_set_agglimit(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UCHAR ac, struct wifi_dev *wdev, UINT32 agg_limit);
INT asic_set_rts_retrylimit(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT32 limit);
INT AsicAmpduEfficiencyAdjust(struct wifi_dev *wdev, UCHAR	aifs_adjust);
INT AsicSetRxPath(struct _RTMP_ADAPTER *pAd, UINT32 RxPathSel, UCHAR BandIdx);
UINT32 AsicGetRxStat(struct _RTMP_ADAPTER *pAd, UINT type);
INT32 AsicGetFwSyncValue(struct _RTMP_ADAPTER *pAd);
VOID AsicInitMac(struct _RTMP_ADAPTER *pAd);
VOID AsicSetTmrCal(struct _RTMP_ADAPTER *pAd, UCHAR TmrType, UCHAR Channel, UCHAR Bw);
VOID AsicTOPInit(struct _RTMP_ADAPTER *pAd);
INT asic_rts_on_off_detail(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT32 rts_num, UINT32 rts_len, BOOLEAN rts_en);
UINT32 asic_get_hwq_from_ac(struct _RTMP_ADAPTER *ad, UCHAR wmm_idx, UCHAR ac);


/**
 * arch operation HAL
 * @check_hw_resource: check hw resource if enough or not for TX.
 * @indicate_tx_resource_state: indicate hw resource in safe region or not
 * @hw_tx: fill hw descriptor and kick out to hw resource for data frame
 * @mlme_hw_tx: fill hw descriptor and kick out to hw resource for mlme frame
 * @ate_hw_tx: fill hw descriptor and kick out to hw resource for ate frame
 * @write_tx_resource:HIF resource arrangement for MSDU TX packet transfer.
 * @write_multi_tx_resource:HIF resource arrangement for A-MSDU TX packet transfer.
 * @write_final_tx_resource:HIF resource arrangement for A-MSDU TX packet transfer
 *	for total bytes	update in tx hw header.
 * @write_frag_tx_resource:HIF resource arrangement for fragment TX packet transfer
 * @write_tmac_info_fixed_rate: per 802.11/802.3 frame TMAC descriptor handle that use fixed rate.
 * @write_tmac_info: per 802.11/802.3 frame TMAC descriptor handle.
 * @write txp_info: per 802.11/802.3 data frame TMAC data information (address and length) handle.
 * @dump_tmac_info: dump tmac header information
 * @rx_pkt_process: per rx packet handle (build RX_BLK and packet processing).
 * @get_packet_type: get packet
 * @trans_rxd_into_rxblk: translate rxd into rxblk
 * @txdone_handle: tx done event handle
 * @dump_rmac_info: dump rmac header infomration
 * @get_mgmt_resource_free_num: get management HIF resource available numbers
 * @get_bcn_resource_free_num: get beacon HIF resource available numbers
 * @get_cmd_resource_free_num: get command HIF resource available numbers
 * @get_fw_loading_resource_free_num: get fw loading HIF resource available numbers
 * @get_hif_buf: get one available HIF pre-allocated dma region for hw header
 * @is_tx_resource_empty: check if tx data HIF resource is empty or not
 * @is_rx_resource_full: check if rx HIF resource is full or not
 * @get_rx_resource_pending_num: get rx HIF resource pending numbers
 * @dump_wtbl_info: dump wtbl info
 * @dump_wtbl_base_info: dump wtbl base address
 * @arch_calculate_ecc: calculate ecc point with scalar
 * @get_bcn_tx_cnt: get mib beacon tx count
 * @txd_post_process: txd post process
 */
typedef struct _RTMP_ARCH_OP {
	UINT32 (*archGetCrcErrCnt)(struct _RTMP_ADAPTER *pAd);
	UINT32 (*archGetCCACnt)(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx);
	UINT32 (*archGetChBusyCnt)(struct _RTMP_ADAPTER *pAd, UCHAR ch_idx);
	INT (*archSetAutoFallBack)(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
	INT (*archAutoFallbackInit)(struct _RTMP_ADAPTER *pAd);
	VOID (*archUpdateProtect)(struct _RTMP_ADAPTER *pAd, VOID *cookie);
	VOID (*archUpdateRtsThld)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR pkt_thld, UINT32 len_thld);
#ifdef DOT11_N_SUPPORT
	INT (*archSetRDG)(struct _RTMP_ADAPTER *pAd, MT_RDG_CTRL_T *Rdg);
#endif /* DOT11_N_SUPPORT */
	VOID (*archSwitchChannel)(struct _RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg);
	INT32 (*archSetDevMac)(
		struct _RTMP_ADAPTER *pAd,
		UINT8 OwnMacIdx,
		UINT8 *OwnMacAddr,
		UINT8 BandIdx,
		UINT8 Active,
		UINT32 EnableFeature);

	INT32 (*archSetBssid)(
		struct _RTMP_ADAPTER *pAd,
		BSS_INFO_ARGUMENT_T *bss_info_argument);
	VOID (*archSetTmrCal)(struct _RTMP_ADAPTER *pAd, UCHAR TmrType, UCHAR Channel, UCHAR Bw);
	UINT32 (*archGetHwQFromAc)(UCHAR wmm_idx, UCHAR wmm_ac);
	INT32 (*archSetStaRec)(struct _RTMP_ADAPTER *pAd, STA_REC_CFG_T *pStaCfg);
	VOID (*archDelWcidTab)(struct _RTMP_ADAPTER *pAd, UINT16 wcid_idx);
#ifdef HTC_DECRYPT_IOT
	INT32 (*archSetWcidAAD_OM)(struct _RTMP_ADAPTER *pAd, UINT16 wcid_idx, UCHAR value);
#endif /* HTC_DECRYPT_IOT */
	INT32 (*archSetWcidPsm)(struct _RTMP_ADAPTER *pAd, UINT16 wcid_idx, UCHAR value);
	INT32 (*archSetWcidSN)(struct _RTMP_ADAPTER *pAd, UINT16 wcid_idx, UINT16 Sn);
#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
	VOID (*archSetWcid4Addr_HdrTrans)(struct _RTMP_ADAPTER *pAd, UINT16 wcid_idx, UCHAR IsEnable, UCHAR IsApcliEntry);
#endif

	VOID (*archAddRemoveKeyTab)(struct _RTMP_ADAPTER *pAd, struct _ASIC_SEC_INFO *pInfo);

	BOOLEAN (*archEnableBeacon)(struct _RTMP_ADAPTER *pAd, VOID *wdev_void);
	BOOLEAN (*archDisableBeacon)(struct _RTMP_ADAPTER *pAd, VOID *wdev_void);
	BOOLEAN (*archUpdateBeacon)(struct _RTMP_ADAPTER *pAd, VOID *wdev_void, BOOLEAN BcnSntReq, UCHAR UpdateReason);
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	INT (*archSetReptFuncEnable)(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR band_idx);
	VOID (*archInsertRepeaterEntry)(struct _RTMP_ADAPTER *pAd, UCHAR CliIdx, PUCHAR pAddr);
	VOID (*archRemoveRepeaterEntry)(struct _RTMP_ADAPTER *pAd, UCHAR CliIdx);
	VOID (*archInsertRepeaterRootEntry)(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, UCHAR *pAddr, UCHAR ReptCliIdx);
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */
	VOID (*archTxCntUpdate)(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, MT_TX_COUNTER *pTxInfo);
	VOID (*archTxCapAndRateTableUpdate)(
		RTMP_ADAPTER *pAd,
		UINT16 u2Wcid,
		RA_PHY_CFG_T *prTxPhyCfg,
		UINT32 *Rate,
		BOOL fgSpeEn);
	INT (*archSetRxFilter)(struct _RTMP_ADAPTER *pAd, MT_RX_FILTER_CTRL_T RxFilter);
	VOID (*archSetPiggyBack)(struct _RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack);
	INT (*archSetPreTbtt)(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable, UCHAR HwBssidIdx);
	INT (*archSetGPTimer)(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout);
	INT (*archSetChBusyStat)(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
	INT (*archGetTsfTime)(
		struct _RTMP_ADAPTER *pAd,
		UINT32 *high_part,
		UINT32 *low_part,
		UCHAR HwBssidIdx);

	VOID (*archDisableSync)(struct _RTMP_ADAPTER *pAd, UCHAR HWBssidIdx);

	VOID (*archSetSyncModeAndEnable)(
		struct _RTMP_ADAPTER *pAd,
		USHORT BeaconPeriod,
		UCHAR HWBssidIdx,
		UCHAR OPMode);

#ifdef CONFIG_STA_SUPPORT
	VOID (*archEnableIbssSync)(struct _RTMP_ADAPTER *pAd,
							   USHORT BeaconPeriod,
							   UCHAR HWBssidIdx,
							   UCHAR OPMode);
#endif /* CONFIG_STA_SUPPORT */
	INT (*archSetWmmParam)(struct _RTMP_ADAPTER *pAd, UCHAR idx, UINT32 AcNum, UINT32 EdcaType, UINT32 EdcaValue);
#ifdef WIFI_UNIFIED_COMMAND
	INT (*archUniCmdSetWmmParam)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR idx, UINT32 AcNum, UINT32 EdcaType, UINT32 EdcaValue);
#endif /* WIFI_UNIFIED_COMMAND */
	VOID (*archSetEdcaParm)(struct _RTMP_ADAPTER *pAd, UCHAR idx, UCHAR tx_mode, struct _EDCA_PARM *pEdcaParm);
#ifdef WIFI_UNIFIED_COMMAND
	VOID (*archUniCmdSetEdcaParm)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR idx, UCHAR tx_mode, PEDCA_PARM pEdcaParm);
#endif /* WIFI_UNIFIED_COMMAND */
	VOID (*archFeLossGet)(struct _RTMP_ADAPTER *pAd, UCHAR channel, CHAR *RssiOffset);
	VOID (*archRcpiReset)(struct _RTMP_ADAPTER *pAd, UINT16 wcid);
	UINT32 (*archGetWmmParam)(struct _RTMP_ADAPTER *pAd,  UINT32 AcNum, UINT32 EdcaType);
	INT (*archSetRetryLimit)(struct _RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit);
	UINT32 (*archGetRetryLimit)(struct _RTMP_ADAPTER *pAd, UINT32 type);
	VOID (*archSetSlotTime)(struct _RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime, UCHAR BandIdx);
#ifdef WIFI_UNIFIED_COMMAND
	VOID (*archUniCmdSetSlotTime)(struct _RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime, struct wifi_dev *wdev);
#endif /* WIFI_UNIFIED_COMMAND */
	INT (*archSetMacMaxLen)(struct _RTMP_ADAPTER *pAd);
	VOID (*archGetTxTsc)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT32 pn_type_mask, UCHAR *pTxTsc);
	VOID (*archAddSharedKeyEntry)(struct _RTMP_ADAPTER *pAd, UCHAR BssIndex, UCHAR KeyIdx, struct _CIPHER_KEY *pCipherKey);
	VOID (*archRemoveSharedKeyEntry)(struct _RTMP_ADAPTER *pAd, UCHAR BssIndex, UCHAR KeyIdx);
	VOID (*archAddPairwiseKeyEntry)(struct _RTMP_ADAPTER *pAd, UINT16 wcid, PCIPHER_KEY pCipherKey);
	INT (*archSetRtsSignalTA)(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx, BOOLEAN Enable);
	INT (*archSetRxStream)(struct _RTMP_ADAPTER *pAd, UINT32 rx_path, UCHAR BandIdx);
	INT (*archSetTxStream)(struct _RTMP_ADAPTER *pAd, UINT32 rx_path, UCHAR BandIdx);
	INT (*archSetBW)(struct _RTMP_ADAPTER *pAd, INT bw, UCHAR BandIdx);
	INT (*archSetCtrlCh)(struct _RTMP_ADAPTER *pAd, UINT8 extch);
	INT (*archWaitMacTxRxIdle)(struct _RTMP_ADAPTER *pAd);
	INT (*archSetMacTxRx)(struct _RTMP_ADAPTER *pAd, INT txrx, BOOLEAN enable, UCHAR BandIdx);
	INT (*archSetRxvFilter)(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR BandIdx);
	INT (*archSetMacTxQ)(struct _RTMP_ADAPTER *pAd, INT WmmSet, INT band, BOOLEAN Enable);
	INT (*archSetMacWD)(struct _RTMP_ADAPTER *pAd);
	INT (*archTOPInit)(struct _RTMP_ADAPTER *pAd);
	VOID (*archSetTmrCR)(struct _RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx);
	INT (*archSetRxPath)(struct _RTMP_ADAPTER *pAd, UINT32 RxPathSel, UCHAR BandIdx);
	UINT32 (*archGetRxStat)(struct _RTMP_ADAPTER *pAd, UINT type);
	INT32 (*archGetFwSyncValue)(struct _RTMP_ADAPTER *pAd);
	VOID (*archInitMac)(struct _RTMP_ADAPTER *pAd);
	VOID (*show_mac_info)(struct _RTMP_ADAPTER *pAd);
	INT (*init_wtbl)(struct _RTMP_ADAPTER *pAd, BOOLEAN bHardReset);
	INT (*get_wtbl_entry234)(struct _RTMP_ADAPTER *pAd, UINT16 widx, struct wtbl_entry *ent);
	UCHAR (*get_nsts_by_mcs)(UCHAR phy_mode, UCHAR mcs, BOOLEAN stbc, UCHAR vht_nss);
	UINT16 (*tx_rate_to_tmi_rate)(UINT8 mode, UINT8 mcs, UINT8 nss, BOOLEAN stbc, UINT8 preamble);
	VOID (*update_raw_counters)(struct _RTMP_ADAPTER *pAd);
	VOID (*update_mib_bucket)(struct _RTMP_ADAPTER *pAd);
#ifdef OFFCHANNEL_ZERO_LOSS
	VOID (*read_channel_stat_registers)(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx, void *ChStat);
#endif
#ifdef ZERO_LOSS_CSA_SUPPORT
	UINT8 (*read_skip_tx)(RTMP_ADAPTER *pAd, UINT16 wcid);
	VOID (*update_skip_tx)(RTMP_ADAPTER *pAd, UINT16 wcid, UINT8 set);
#endif /*ZERO_LOSS_CSA_SUPPORT*/
#ifdef CONFIG_AP_SUPPORT
	VOID (*archSetWdevIfAddr)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT opmode);
	VOID (*archSetMbssHwCRSetting)(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
	VOID (*archSetExtTTTTHwCRSetting)(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
	VOID (*archSetExtMbssEnableCR)(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
#endif

#ifdef DBDC_MODE
	INT (*archSetDbdcCtrl)(struct _RTMP_ADAPTER *pAd, struct _BCTRL_INFO_T *pBctrInfo);
	INT (*archGetDbdcCtrl)(struct _RTMP_ADAPTER *pAd, struct _BCTRL_INFO_T *pBctrInfo);
#endif

	VOID (*archUpdateRxWCIDTable)(struct _RTMP_ADAPTER *pAd, MT_WCID_TABLE_INFO_T WtblInfo);

#ifdef TXBF_SUPPORT
	VOID (*archUpdateClientBfCap)(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#endif

	INT32 (*archUpdateBASession)(struct _RTMP_ADAPTER *pAd, MT_BA_CTRL_T BaCtrl);
	INT32 (*archUpdateStaRecBa)(struct _RTMP_ADAPTER *pAd, STA_REC_BA_CFG_T StaRecCfg);
	VOID (*archSetSMPS)(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, UCHAR smps);
	INT32 (*archRxHeaderTransCtl)(struct _RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan, BOOLEAN SwPcP);
	INT32 (*archRxHeaderTaranBLCtl)(struct _RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType);

#ifdef VLAN_SUPPORT
	INT32 (*update_vlan_id)(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT8 omac_idx, UINT16 vid);
	INT32 (*update_vlan_priority)(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT8 omac_idx, UINT8 priority);
#endif

	/* TX */
	INT (*check_hw_resource)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR resource_idx);
	INT (*set_resource_state)(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx, BOOLEAN state);
	INT32 (*get_hw_resource_state)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
											UINT32 pkt_type, UCHAR resource_idx);
	INT (*hw_tx)(struct _RTMP_ADAPTER *ad, struct _TX_BLK *tx_blk);
	VOID (*write_tmac_info)(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
	INT32 (*write_txp_info)(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
	USHORT (*write_tx_resource)(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, BOOLEAN bIsLast, USHORT *FreeNumber);
#if defined(CTXD_SCATTER_AND_GATHER) || defined(CTXD_MEM_CPY)
	VOID (*write_last_tx_resource)(struct _RTMP_ADAPTER *pAd, UCHAR resource_idx);
#endif
	USHORT (*write_multi_tx_resource)(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR frameNum, USHORT *FreeNumber);
	VOID (*write_final_tx_resource)(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, USHORT totalMPDUSize, USHORT FirstTxIdx);
	USHORT (*write_frag_tx_resource)(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR fragNum, USHORT *FreeNumber);
	INT (*mlme_hw_tx)(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info, MAC_TX_INFO *info, HTTRANSMIT_SETTING *pTransmit, struct _TX_BLK *tx_blk);
#ifdef CONFIG_ATE
	INT32 (*ate_hw_tx)(struct _RTMP_ADAPTER *pAd, struct _TMAC_INFO *info, struct _TX_BLK *tx_blk);
#endif
	VOID (*write_tmac_info_fixed_rate)(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info, MAC_TX_INFO *info, HTTRANSMIT_SETTING *pTransmit);
	VOID (*dump_tmac_info)(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info);
	VOID (*fill_cmd_header)(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *net_pkt);
#ifdef WIFI_UNIFIED_COMMAND
	VOID (*fill_uni_cmd_header)(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *net_pkt);
#endif /* WIFI_UNIFIED_COMMAND */

	/*  RX */
	UINT32 (*rx_pkt_process)(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx, struct _RX_BLK *rx_blk, VOID *rx_pkt);
	UINT32 (*get_packet_type)(struct _RTMP_ADAPTER *pAd, VOID *rx_packet);
#ifdef SNIFFER_RADIOTAP_SUPPORT
	UINT32 (*trans_rxd_into_radiotap)(struct _RTMP_ADAPTER *pAd, VOID *rx_packet, struct _RX_BLK *rx_blk);
#endif
	INT32 (*trans_rxd_into_rxblk)(struct _RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk, VOID *rx_pkt);
	UINT32 (*txdone_handle)(struct _RTMP_ADAPTER *pAd, VOID *ptr, UINT8 resource_idx);
	UINT32 (*rxv_handler)(struct _RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk, VOID *rx_packet);
	VOID (*dump_rmac_info)(struct _RTMP_ADAPTER *pAd, UCHAR *rmac_info);
	VOID (*dump_rx_info)(struct _RTMP_ADAPTER *pAd, UCHAR *rx_info);
	VOID (*dump_rmac_info_for_icverr)(struct _RTMP_ADAPTER *pAd, UCHAR *rmac_info);
	INT (*dump_dmac_amsdu_info)(struct _RTMP_ADAPTER *pAd);
	VOID (*dump_txs)(struct _RTMP_ADAPTER *pAd, UINT8 format, CHAR *data);
	VOID (*rx_event_handler)(struct _RTMP_ADAPTER *ad, UCHAR *data);
#ifdef IGMP_SNOOP_SUPPORT
	BOOLEAN (*archMcastEntryInsert)(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, UINT8 Type, PUCHAR MemberAddr, PNET_DEV dev, UINT16 wcid);
	BOOLEAN (*archMcastEntryDelete)(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, PUCHAR MemberAddr, PNET_DEV dev, UINT16 wcid);
#ifdef IGMP_SNOOPING_DENY_LIST
	BOOLEAN (*archMcastEntryDenyList)(RTMP_ADAPTER *pAd, PNET_DEV dev, UINT8 entry_cnt, UINT8 add_to_list, UINT8 *pAddr);
#endif
#ifdef IGMP_TVM_SUPPORT
	BOOLEAN (*archMcastConfigAgeout)(RTMP_ADAPTER *pAd, UINT8 AgeOutTime, UINT8 ucOwnMacIdx);
	BOOLEAN (*archMcastGetMcastTable)(RTMP_ADAPTER *pAd, UINT8 ucOwnMacIdx, struct wifi_dev *wdev);
#endif /* IGMP_TVM_SUPPORT */
#endif
	INT (*asic_rts_on_off)(
		struct _RTMP_ADAPTER *ad,
		UCHAR band_idx,
		UINT32 rts_num,
		UINT32 rts_len,
		BOOLEAN rts_en);
	INT (*asic_set_agglimit)(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UCHAR ac, struct wifi_dev *wdev, UINT32 agg_limit);
	INT (*asic_ampdu_efficiency_on_off)(struct _RTMP_ADAPTER *ad, UCHAR	wmm_idx, UCHAR aifs_adjust);
	INT (*asic_set_rts_retrylimit)(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT32 limit);
#ifdef RED_SUPPORT
	bool (*archRedMarkPktDrop)(UINT16 u2WlanIdx, UINT8 ucQidx, struct _RTMP_ADAPTER *pAd);
#endif
	VOID (*dump_wtbl_info)(struct _RTMP_ADAPTER *pAd, UINT16 wtbl_idx);
	VOID (*dump_wtbl_base_info)(struct _RTMP_ADAPTER *pAd);
	VOID (*init_txrx_ring)(struct _RTMP_ADAPTER *ad);
	VOID (*asic_wa_update)(struct _RTMP_ADAPTER *ad);
	VOID (*arch_calculate_ecc)(struct _RTMP_ADAPTER *ad, UINT32 oper, UINT32 group, UINT8 *scalar, UINT8 *point_x, UINT8 *point_y);
	INT32 (*archGetAntMode)(struct _RTMP_ADAPTER *pAd, UCHAR *AntMode);
#ifdef TX_POWER_CONTROL_SUPPORT
	VOID (*arch_txpower_boost)(struct _RTMP_ADAPTER *pAd, UCHAR ucBandIdx);
	VOID (*arch_txpower_boost_ctrl)(struct _RTMP_ADAPTER *pAd, UCHAR ucBandIdx, CHAR cPwrUpCat, PUCHAR pcPwrUpValue);
	BOOLEAN (*arch_txpower_boost_rate_type)(struct _RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT8 u1PowerBoostRateType);
	BOOLEAN (*arch_txpower_boost_power_cat_type)(struct _RTMP_ADAPTER *pAd, UINT8 u1PhyMode, UINT8 u1Bw, PUINT8 pu1PwrUpCat);
	BOOLEAN (*arch_txpower_boost_info_V0)(struct _RTMP_ADAPTER *pAd, POWER_BOOST_TABLE_CATEGORY_V0 ePowerBoostRateType);
	BOOLEAN (*arch_txpower_boost_info_V1)(struct _RTMP_ADAPTER *pAd, POWER_BOOST_TABLE_CATEGORY_V1 ePowerBoostRateType);
	VOID (*arch_txpower_boost_profile)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *pBuffer);
#endif
#ifdef SINGLE_SKU_V2
	VOID (*arch_txpower_sku_cfg_para)(struct _RTMP_ADAPTER *pAd);
#endif
	VOID (*arch_txpower_all_rate_info)(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
	UINT32 (*arch_get_bcn_tx_cnt)(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx);
#ifdef AIR_MONITOR
	INT (*arch_set_air_mon_enable)(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR band_idx);
	INT (*arch_set_air_mon_rule)(struct _RTMP_ADAPTER *pAd, UCHAR *rule, UCHAR band_idx);
	INT (*arch_set_air_mon_idx)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR mnt_idx, UCHAR band_idx);
#endif

#ifdef WIFI_MD_COEX_SUPPORT
#ifdef COEX_DIRECT_PATH
	VOID (*set_conn_infra_sysram)(struct _RTMP_ADAPTER *pAd);
	VOID (*get_wm_pc_status)(struct _RTMP_ADAPTER *pAd);
#endif/* COEX_DIRECT_PATH */
#endif /* WIFI_MD_COEX_SUPPORT */
	VOID (*txd_post_process)(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info, MAC_TX_INFO *info, HTTRANSMIT_SETTING *pTransmit);
} RTMP_ARCH_OP;

#ifdef LINK_TEST_SUPPORT
VOID
LinkTestRcpiSet(
	struct _RTMP_ADAPTER *pAd,
	UINT16 wcid,
	UINT8 u1AntIdx,
	CHAR i1Rcpi
	);

VOID
LinkTestPeriodHandler(
	struct _RTMP_ADAPTER *pAd
	);

VOID
LinkTestTimeSlotLinkHandler(
	struct _RTMP_ADAPTER *pAd
	);

VOID
LinkTestStaLinkUpHandler(
	struct _RTMP_ADAPTER *pAd,
	struct _MAC_TABLE_ENTRY *pEntry
	);

VOID
LinkTestApClientLinkUpHandler(
	struct _RTMP_ADAPTER *pAd
	);

BOOLEAN
LinkTestInstrumentCheck(
	struct _RTMP_ADAPTER *pAd,
	struct _MAC_TABLE_ENTRY *pEntry
	);

VOID
LinkTestChannelBandUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT8 u1ControlChannel
	);

VOID
LinkTestChannelSwitchHandler(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx
	);

VOID
LinkTestRxCntCheck(
	struct _RTMP_ADAPTER *pAd
	);

VOID
LinkTestRxStreamCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgCmwLinkStatus,
	UINT16 wcid
	);

VOID
LinkTestDCRFCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgDCRFenable
	);

VOID
LinkTestRxStreamTrans(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BandIdx,
	UINT8 u1SpeRssiIdx
	);

VOID
LinkTestTxBwSwitch(
	struct _RTMP_ADAPTER *pAd,
	struct _MAC_TABLE_ENTRY *pEntry
	);

VOID
LinkTestTxBwRestore(
	struct _RTMP_ADAPTER *pAd,
	struct _MAC_TABLE_ENTRY *pEntry
	);

VOID
LinkTestSpeIdxCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgCmwLinkStatus,
	UINT8 u1BandIdx
	);

VOID
LinkTestTxBwCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgCmwLinkStatus,
	struct _MAC_TABLE_ENTRY *pEntry
	);

VOID
LinkTestTxCsdCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgCmwLinkStatus,
	UINT8 u1BandIdx
	);

VOID
LinkTestTxPowerCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgCmwLinkStatus,
	UINT8 u1BandIdx
	);

VOID
LinkTestRcpiCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgCmwLinkStatus,
	UINT8 u1BandIdx
	);

VOID
LinkTestAcrCtrl(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN fgCmwLinkStatus,
	UINT16 wcid,
	UINT8 u1BandIdx
	);

VOID
LinkTestRssiGet(
	struct _RTMP_ADAPTER *pAd,
	ENUM_RSSI_CHECK_SOURCE eRssiSrc,
	UINT16 wcid,
	PCHAR pi1Rssi
	);

VOID
LinkTestRssiCheck(
	struct _RTMP_ADAPTER *pAd,
	PCHAR pi1Rssi,
	UINT8 u1BandIdx,
	PUINT8 pu1SpeRssiIdx,
	PUINT8 pu1RssiReason,
	UINT16 wcid
	);

UINT8
LinkTestRssiCheckItem(
	struct _RTMP_ADAPTER *pAd,
	PCHAR pi1Rssi,
	UINT8 u1BandIdx,
	ENUM_RSSI_CHECK_SOURCE eRssiCheckSrc,
	UINT16 wcid
	);

UINT8
LinkTestRssiComp(
	struct _RTMP_ADAPTER *pAd,
	PCHAR pi1Rssi,
	UINT8 u1RssiNum,
	ENUM_RSSI_CHECK_SOURCE eRssiCheckSrc
	);

UINT8
LinkTestRssiSpecificRxPath(
	struct _RTMP_ADAPTER *pAd,
	PCHAR pi1Rssi,
	PUINT8 pu1RxPathRssiOrder,
	UINT8 u1RssiNum,
	ENUM_RSSI_CHECK_SOURCE eRssiCheckSrc
	);

VOID
LinkTestSwap(
	PCHAR pi1Value1,
	PCHAR pi1Value2
	);
#endif /* LINK_TEST_SUPPORT */


#ifdef	ETSI_RX_BLOCKER_SUPPORT
UINT8	ETSIWbRssiCheck(RTMP_ADAPTER *pAd);
#endif /* ETSI_RX_BLOCKER_SUPPORT */

VOID asic_write_tmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
USHORT asic_write_tx_resource(struct _RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk, BOOLEAN bIsLast, USHORT *freeCnt);
#if defined(CTXD_SCATTER_AND_GATHER) || defined(CTXD_MEM_CPY)
VOID asic_write_last_tx_resource(struct _RTMP_ADAPTER *pAd, UCHAR resource_idx);
#endif
VOID asic_write_tmac_info_fixed_rate(struct _RTMP_ADAPTER *pAd,
	UCHAR *tmac_info, MAC_TX_INFO *info, HTTRANSMIT_SETTING *pTransmit);
INT32 asic_write_txp_info(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
INT asic_check_hw_resource(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR resource_idx);
VOID asic_set_resource_state(struct _RTMP_ADAPTER *pAd, UCHAR resource_idx, BOOLEAN state);
INT asic_get_hw_resource_state(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
										UINT32 pkt_type, UCHAR resource_idx);
UINT32 asic_rx_pkt_process(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx, struct _RX_BLK *pRxBlk, VOID *pRxPacket);
UINT32 asic_get_packet_type(struct _RTMP_ADAPTER *pAd, VOID *rx_packet);
#ifdef SNIFFER_RADIOTAP_SUPPORT
UINT32 asic_trans_rxd_into_radiotap(RTMP_ADAPTER *pAd, VOID *rx_packet, struct _RX_BLK *rx_blk);
#endif

INT32 asic_trans_rxd_into_rxblk(RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk, VOID *rx_pkt);
VOID asic_dump_rmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *rmac_info);
INT asic_mlme_hw_tx(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info, MAC_TX_INFO *info, HTTRANSMIT_SETTING *pTransmit, struct _TX_BLK *tx_blk);
INT asic_hw_tx(struct _RTMP_ADAPTER *ad, struct _TX_BLK *tx_blk);
VOID asic_dump_tmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info);
UINT32 asic_txdone_handle(struct _RTMP_ADAPTER *pAd, VOID *ptr, UINT8 resource_idx);
UINT32 asic_rxv_handler(struct _RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk, VOID *rx_packet);
VOID asic_rx_event_handler(struct _RTMP_ADAPTER *pAd, VOID *rx_packet);
VOID asic_dump_rmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *rmac_info);
VOID asic_dump_rxinfo(struct _RTMP_ADAPTER *pAd, UCHAR *rx_info);
VOID asic_dump_txs(struct _RTMP_ADAPTER *pAd, UINT8 format, CHAR *data);
VOID asic_dump_rmac_info_for_ICVERR(struct _RTMP_ADAPTER *pAd, UCHAR *rmac_info);
INT asic_dump_dmac_amsdu_info(struct _RTMP_ADAPTER *pAd);

VOID asic_dump_wtbl_base_info(struct _RTMP_ADAPTER *pAd);
VOID asic_dump_wtbl_info(struct _RTMP_ADAPTER *pAd, UINT16 wtbl_idx);
VOID asic_init_txrx_ring(struct _RTMP_ADAPTER *pAd);
VOID asic_wa_update(struct _RTMP_ADAPTER *ad);
VOID asic_calculate_ecc(struct _RTMP_ADAPTER *ad, UINT32 oper, UINT32 group, UINT8 *scalar, UINT8 *point_x, UINT8 *point_y);
INT asic_set_rxfilter(struct _RTMP_ADAPTER *pAd, MT_RX_FILTER_CTRL_T RxFilter);
INT asic_get_wtbl_entry234(struct _RTMP_ADAPTER *pAd, UINT16 widx, struct wtbl_entry *ent);
INT asic_init_wtbl(struct _RTMP_ADAPTER *pAd, BOOLEAN bHardReset);
VOID asic_show_mac_info(struct _RTMP_ADAPTER *pAd);
VOID asic_write_tmac_info_mgmt(struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev, UCHAR sub_type, UCHAR *tmac_buf, HTTRANSMIT_SETTING *beaconTransmit, ULONG frmLen);
UCHAR asic_get_nsts_by_mcs(struct _RTMP_ADAPTER *pAd, UCHAR phy_mode, UCHAR mcs, BOOLEAN stbc, UCHAR vht_nss);
UINT16 asic_tx_rate_to_tmi_rate(struct _RTMP_ADAPTER *pAd, UINT8 mode, UINT8 mcs, UINT8 nss, BOOLEAN stbc, UINT8 preamble);
VOID asic_update_raw_counters(struct _RTMP_ADAPTER *pAd);
VOID asic_update_mib_bucket(struct _RTMP_ADAPTER *pAd);
#ifdef OFFCHANNEL_ZERO_LOSS
VOID asic_read_channel_stat_registers(RTMP_ADAPTER *pAd, UINT8 BandIdx, void *ChStat);
#endif
#ifdef ZERO_LOSS_CSA_SUPPORT
UINT8 AsicReadSkipTx(RTMP_ADAPTER *pAd, UINT16 wcid);
VOID AsicUpdateSkipTx(RTMP_ADAPTER *pAd, UINT16 wcid, UINT8 set);
#endif /*ZERO_LOSS_CSA_SUPPORT*/
UINT32 asic_get_bcn_tx_cnt(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx);
#ifdef AIR_MONITOR
INT asic_set_air_mon_enable(RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR band_idx);
INT asic_set_air_mon_rule(RTMP_ADAPTER *pAd, UCHAR *rule, UCHAR band_idx);
INT asic_set_air_mon_idx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR mnt_idx, UCHAR band_idx);
#endif

#ifdef ACK_CTS_TIMEOUT_SUPPORT
typedef enum _WH_TX_ACK_CTS_TYPE_T {
	WH_TX_ACK_CTS_TYPE_CCK_DCF_TIMEOUT = 0,
	WH_TX_ACK_CTS_TYPE_OFDM_DCF_TIMEOUT,
	WH_TX_ACK_CTS_TYPE_OFDMA_MU_DCF_TIMEOUT,
	WH_TX_ACK_CTS_TYPE_MAX,
} WH_TX_ACK_CTS_TYPE_T;

INT asic_set_ack_timeout_mode_byband_by_fw(struct _RTMP_ADAPTER *pAd, UINT32 timeout, UINT32 bandidx, UINT8 ackmode);
INT32 asic_get_ack_timeout_mode_byband_by_fw(struct _RTMP_ADAPTER *pAd, UINT32 *ptimeout, UINT32 bandidx, UINT8 ackmode);

#endif /* ACK_CTS_TIMEOUT_SUPPORT */

#endif /* __ASIC_CTRL_H_ */
