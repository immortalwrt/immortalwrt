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
	hdev_ctrl.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __HDEV_CTRL_H__
#define __HDEV_CTRL_H__

struct _RTMP_ADAPTER;
struct _EDCA_PARM;
struct _QLOAD_CTRL;
struct _AUTO_CH_CTRL;
struct wifi_dev;
struct _OMAC_BSS_CTRL;
struct _REPEATER_CLIENT_ENTRY;
struct radio_res;
struct freq_oper;

struct wmm_entry {
	UCHAR dbdc_idx;
	UCHAR ref_cnt;
	UCHAR tx_mode;
	UCHAR wmm_set;
	struct _EDCA_PARM edca;
};

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
struct twt_link_node {
	/* twt link node control */
	struct _DL_LIST list;
	/* twt entry */
	UINT8	state;                      /* occupied status, reserve for ctrl acquire/release */
	UINT8	agrt_tbl_idx;
	UINT8	own_mac_idx;
	UINT8	flow_id;
	UINT16	peer_id_grp_id;				/* peer wcid, grp id, btwt_id */
	UINT8	agrt_sp_duration;
	UINT8	bss_idx;
	UINT8   tsf_type;                   /* indicate tsf from requester (REQUEST/SUGGEST/DEMAND)*/
	UINT64	schedule_sp_start_tsf;      /* used by scheduler and will be within TWT_SP_SPAN_TIME */
	UINT64	schedule_sp_start_tsf_abs;  /* get abssoluate tsf based on schedule_sp_start_tsf & current_tsf */
	UINT64	agrt_sp_start_tsf;          /* tsf from requester */
	UINT64	agrt_sp_info_tsf;           /* tsf from twt info frame */
	UINT16	agrt_sp_wake_intvl_mantissa;
	UINT8	agrt_sp_wake_intvl_exponent;
	UINT8	is_role_ap;
	UINT8	agrt_para_bitmap;           /* ..protect/announce/trigger */
	UINT8	type;                       /* individual/group/btwt, reserve for ctrl acquire/release */
	BOOLEAN suspend;                    /* */
	/* grp case */
	UINT8	grp_grade;
	UINT8	grp_member_cnt;
	UINT16	sta_list[TWT_HW_GRP_MAX_MEMBER_CNT];
	/* b.TWT */
	UINT8	band;
	UINT8	twt_info_frame_dis;
	UINT8	wake_dur_unit;
	UINT8	btwt_recommendation;
	UINT8	persistence;
	UINT8	twt_setup_cmd;
	UINT8	twt_channel;
	BOOLEAN present;	/* btwt ie is present in BCN/ProbeRsp */
};
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

enum {
	HC_STATUS_OK,
	HC_STATUS_FAIL
};

INT32 hc_radio_init(struct _RTMP_ADAPTER *pAd, UCHAR RfIC, UCHAR DbdcMode);
INT32 hc_radio_exit(struct _RTMP_ADAPTER *pAd, UCHAR dbdc_mode);
INT32 HcAcquireRadioForWdev(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
INT32 HcReleaseRadioForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
INT32 HcUpdateCsaCntByChannel(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
UCHAR HcGetBandByWdev(struct wifi_dev *wdev);
VOID HcSetRadioCurStatByWdev(struct wifi_dev *wdev, PHY_STATUS CurStat);
VOID HcSetRadioCurStatByChannel(RTMP_ADAPTER *pAd, UCHAR Channel, PHY_STATUS CurStat);
VOID HcSetAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd);
VOID HcSetAllSupportedBandsRadioOn(RTMP_ADAPTER *pAd);
BOOLEAN IsHcRadioCurStatOffByWdev(struct wifi_dev *wdev);
BOOLEAN IsHcRadioCurStatOffByChannel(RTMP_ADAPTER *pAd, UCHAR Channel);
BOOLEAN IsHcAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd);
#ifdef GREENAP_SUPPORT
VOID HcSetGreenAPActiveByBand(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx, BOOLEAN bGreenAPActive);
BOOLEAN IsHcGreenAPActiveByBand(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx);
BOOLEAN IsHcGreenAPActiveByWdev(struct wifi_dev *wdev);
#endif /* GREENAP_SUPPORT */
UCHAR HcGetChannelByBf(struct _RTMP_ADAPTER *pAd);
BOOLEAN HcIsRadioAcq(struct wifi_dev *wdev);
BOOLEAN HcIsBfCapSupport(struct wifi_dev *wdev);


/*Wtable Ctrl*/
UINT16 HcAcquireGroupKeyWcid(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID HcReleaseGroupKeyWcid(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT16 wcid);
UINT16 HcGetMaxStaNum(struct _RTMP_ADAPTER *pAd);
UINT16 HcSetMaxStaNum(struct _RTMP_ADAPTER *pAd);

#ifdef SW_CONNECT_SUPPORT
BOOLEAN HcAcquiredDummyObj(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID HcReleaseDummyObj(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
BOOLEAN HcSetDummyWcidFixedRate(struct _RTMP_ADAPTER *pAd, UINT16 wcid, HTTRANSMIT_SETTING rate);
BOOLEAN HcIsDummyWcid(struct _RTMP_ADAPTER *pAd, UINT16 wcid);
UINT16 HcGetDummyWcid(struct wifi_dev *wdev);
UINT16 HcGetMaxStaNumSw(struct _RTMP_ADAPTER *pAd);
UINT16 HcGetSwWcid(struct _RTMP_ADAPTER *pAd, UINT16 wcid_hw);
#endif /* SW_CONNECT_SUPPORT */

UCHAR HcGetWcidLinkType(struct _RTMP_ADAPTER *pAd, UINT16 Wcid);

#ifdef SW_CONNECT_SUPPORT
/* return S/W wcid , arg 5 : H/W acid, arg 6 : bSw */
UINT16 HcAcquireUcastWcid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN is_A4, BOOLEAN is_apcli, UINT16 *wcid_hw, BOOLEAN *bSw);
#else /* SW_CONNECT_SUPPORT */
UINT16 HcAcquireUcastWcid(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN is_A4, BOOLEAN is_apcli);
#endif /* !SW_CONNECT_SUPPORT */

UINT16 HcReleaseUcastWcid(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT16 wcid);
VOID HcWtblRecDump(struct _RTMP_ADAPTER *pAd);

#ifdef DBDC_MODE
VOID HcShowBandInfo(struct _RTMP_ADAPTER *pAd);
#endif /*DBDC_MODE*/

VOID HcShowChCtrlInfo(struct _RTMP_ADAPTER *pAd);
#ifdef GREENAP_SUPPORT
VOID HcShowGreenAPInfo(RTMP_ADAPTER *pAd);
#endif /* GREENAP_SUPPORT */

INT32 hdev_ctrl_init(struct _RTMP_ADAPTER *pAd, INT type);
VOID hdev_ctrl_exit(struct _RTMP_ADAPTER *pAd);
VOID hdev_resource_init(void *hdev_ctrl);

void hc_set_txcmd_mode(VOID *ctrl, UCHAR txcmd_mode);

/*
WMM
*/
BOOLEAN HcAcquiredEdca(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _EDCA_PARM *pEdca);
VOID HcReleaseEdca(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);


/*
* OmacCtrl
*/
UCHAR HcGetOmacIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);


/*Should remove it*/
UCHAR HcGetChannelByRf(struct _RTMP_ADAPTER *pAd, UCHAR RfIC);
USHORT HcGetRadioPhyMode(struct _RTMP_ADAPTER *pAd);
USHORT HcGetRadioPhyModeByBandIdx(RTMP_ADAPTER *pAd, UCHAR BandIdx);
UCHAR HcGetRadioChannel(struct _RTMP_ADAPTER *pAd);
BOOLEAN  HcIsRfSupport(struct _RTMP_ADAPTER *pAd, UCHAR RfIC);
BOOLEAN  HcIsRfRun(struct _RTMP_ADAPTER *pAd, UCHAR RfIC);

UCHAR HcGetBw(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

UCHAR HcGetRadioRfIC(struct _RTMP_ADAPTER *pAd);

struct _QLOAD_CTRL *HcGetQloadCtrlByRf(struct _RTMP_ADAPTER *pAd, UINT32 RfIC);
struct _QLOAD_CTRL *HcGetQloadCtrl(struct _RTMP_ADAPTER *pAd);
struct _AUTO_CH_CTRL *HcGetAutoChCtrl(struct _RTMP_ADAPTER *pAd);

struct _AUTO_CH_CTRL *HcGetAutoChCtrlbyBandIdx(RTMP_ADAPTER *pAd, UCHAR BandIdx);
UINT32 HcGetMgmtQueueIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum PACKET_TYPE pkt_type);
UINT32 HcGetBcnQueueIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UINT32 HcGetWmmIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UCHAR HcGetBandByChannel(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
UCHAR HcGetBandByChannelRange(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
EDCA_PARM *HcGetEdca(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID HcSetEdca(struct wifi_dev *wdev);
VOID HcCrossChannelCheck(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel);

#ifdef MAC_REPEATER_SUPPORT
INT32 HcAddRepeaterEntry(struct wifi_dev *wdev);
INT32 HcDelRepeaterEntry(struct wifi_dev *wdev);
UCHAR HcGetRepeaterOmac(struct wifi_dev *wdev);
#endif /*#MAC_REPEATER_SUPPORT*/
UCHAR HcGetAmountOfBand(struct _RTMP_ADAPTER *pAd);
INT32 HcUpdateMSDUTxAllowByChannel(RTMP_ADAPTER *pAd, UCHAR Channel);
INT32 HcSuspendMSDUTxByChannel(RTMP_ADAPTER *pAd, UCHAR Channel);

/*radio resource mgmt*/
BOOLEAN hc_radio_res_request(struct wifi_dev *wdev, struct radio_res *res);
UCHAR hc_reset_radio(struct _RTMP_ADAPTER *ad);
VOID hc_set_rrm_init(struct wifi_dev *wdev);
INT hc_oper_query_by_wdev(struct wifi_dev *wdev, struct freq_oper *oper);
INT hc_radio_query_by_wdev(struct wifi_dev *wdev, struct freq_oper *oper);
INT hc_radio_query_by_channel(struct _RTMP_ADAPTER *ad, UCHAR channel, struct freq_oper *oper);
INT hc_radio_query_by_index(struct _RTMP_ADAPTER *ad, UCHAR index, struct freq_oper *oper);
/*temporally use*/
INT hc_radio_query_by_rf(struct _RTMP_ADAPTER *ad, UCHAR rfic, struct freq_oper *oper);

VOID *hc_get_hdev_ctrl(struct wifi_dev *wdev);
INT hc_obj_init(struct wifi_dev *wdev, INT idx);
VOID hc_obj_exit(struct wifi_dev *wdev);
UCHAR hc_get_cur_rfic(struct wifi_dev *wdev);


CHANNEL_CTRL *hc_get_channel_ctrl(void *hdev_ctrl, UCHAR BandIdx);
UCHAR hc_init_ChCtrl(RTMP_ADAPTER *pAd);
UCHAR hc_init_ACSChCtrl(RTMP_ADAPTER *pAd);
UCHAR hc_init_ACSChCtrlByBandIdx(RTMP_ADAPTER *pAd, UCHAR BandIdx);
UCHAR hc_set_ChCtrl(CHANNEL_CTRL *ChCtrl, RTMP_ADAPTER *pAd, UCHAR ChIdx, UCHAR ChIdx2);
UCHAR hc_set_ChCtrlFlags_CAP(CHANNEL_CTRL *ChCtrl, UINT ChannelListFlag, UCHAR ChIdx);
UCHAR hc_set_ChCtrlChListStat(CHANNEL_CTRL *ChCtrl, CH_LIST_STATE ChListStat);
UCHAR hc_check_ChCtrlChListStat(CHANNEL_CTRL *ChCtrl, CH_LIST_STATE ChListStat);

#ifdef DOT11_HE_AX
BOOLEAN hc_bcolor_acquire(struct wifi_dev *wdev, UINT8 *color);
void hc_bcolor_release(struct wifi_dev *wdev, UINT8 color);
void hc_bcolor_occupy(struct wifi_dev *wdev, UINT8 color);
BOOLEAN hc_bcolor_is_occupied(struct wifi_dev *wdev, UINT8 color);
void hc_bcolor_ageout(struct wifi_dev *wdev, UINT8 sec);
void hc_bcolor_get_bitmap(struct wifi_dev *wdev, UINT8 *bitmap);
void hc_bcolor_update_by_bitmap(struct wifi_dev *wdev, UINT8 *bitmap);
struct pe_control *hc_get_pe_ctrl(struct wifi_dev *wdev);
#endif

/*chipcap & chipop related*/
struct _RTMP_CHIP_CAP *hc_get_chip_cap(void *hdev_ctrl);
struct _RTMP_CHIP_OP *hc_get_chip_ops(void *hdev_ctrl);
void hc_register_chip_ops(void *hdev_ctrl, struct _RTMP_CHIP_OP *ops);
struct _RTMP_ARCH_OP *hc_get_arch_ops(void *hdev_ctrl);
struct mt_io_ops *hc_get_io_ops(void *hdev_ctrl);
struct _RTMP_CHIP_DBG *hc_get_chip_dbg(void *hdev_ctrl);
UINT32 hc_get_hif_type(void *hdev_ctrl);
VOID hc_clear_asic_cap(void *hdev_ctrl, UINT32 caps);
VOID hc_set_asic_cap(void *hdev_ctrl, UINT32 caps);
UINT32 hc_get_asic_cap(void *hdev_ctrl);
UINT8 hc_get_chip_bcn_max_num(void *hdev_ctrl);
UINT16 hc_get_chip_wtbl_max_num(void *hdev_ctrl);
#ifdef SW_CONNECT_SUPPORT
UINT16 hc_get_chip_sw_sta_max_num(void *hdev_ctrl);
#endif /* SW_CONNECT_SUPPORT */
UINT16 hc_get_chip_wtbl_no_matched_idx(void *hdev_ctrl);
BOOLEAN hc_get_chip_wapi_sup(void *hdev_ctrl);
UINT32 hc_get_chip_tx_token_nums(void *hdev_ctrl);
UINT32 hc_get_chip_sw_tx_token_nums(void *hdev_ctrl);
UINT32 hc_get_chip_mac_rxd_size(void *hdev_ctrl);
VOID *hc_get_hif_ctrl(void *hdev_ctrl);
VOID *hc_get_os_cookie(void *hdev_ctrl);
VOID *hc_get_mcu_ctrl(void *hdev_ctrl);
void hc_show_edca_info(void *hdev_ctrl);
VOID hc_set_phy_cap(void *hdev_ctrl, enum PHY_CAP caps);
UINT32 hc_get_phy_cap(void *hdev_ctrl);
VOID hc_set_mac_cap(void *hdev_ctrl, enum MAC_CAP caps);
UINT32 hc_get_mac_cap(void *hdev_ctrl);
void *hc_get_hdev_privdata(void *hdev_ctrl);
#ifdef CUT_THROUGH
VOID *hc_get_ct_cb(void *hdev_ctrl);
VOID hc_set_ct_cb(void *hdev_ctrl, void *ct_cb);
#endif /*CUT_THROUGH*/
VOID *hc_get_hif_ops(void *hdev_ctrl);
VOID *hc_get_qload_by_wdev(struct wifi_dev *wdev);

#define RADIO_IN_ABAND(_wdev) (hc_get_cur_rfic(_wdev) == RFIC_5GHZ || hc_get_cur_rfic(_wdev) == RFIC_6GHZ)
#define RADIO_IN_2G(_wdev) (hc_get_cur_rfic(_wdev) == RFIC_24GHZ)
#define RADIO_IN_5G(_wdev) (hc_get_cur_rfic(_wdev) == RFIC_5GHZ)
#define RADIO_IN_6G(_wdev) (hc_get_cur_rfic(_wdev) == RFIC_6GHZ)


#endif /*__HDEV_CTRL_H__*/
