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
*/


#include "rt_config.h"


/*
 * Build up common HE IEs
 */
static UINT8 *build_he_mac_cap(struct wifi_dev *wdev, UINT8 *f_buf)
{
	struct he_mac_capinfo he_mac_cap;
	struct ppdu_caps *ppdu = wlan_config_get_ppdu_caps(wdev);
	UINT32 cap_1 = 0;
	UINT16 cap_2 = 0;
	UINT8 *pos = f_buf;

	cap_1 |= DOT11AX_MAC_CAP_HTC;
#ifdef WIFI_TWT_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_AP) {
#ifdef CONFIG_AP_SUPPORT
		struct _RTMP_ADAPTER *ad = NULL;
		BTWT_BUF_STRUCT *btwt = NULL;
		UINT8 band = 0;

		ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
		band = HcGetBandByWdev(wdev);
		btwt = &ad->ApCfg.btwt[band];
		cap_1 &= ~DOT11AX_MAC_CAP_FLEX_TWT_SCHDL;

		if (wlan_config_get_asic_twt_caps(wdev)) {
			if (TWT_SUPPORT_ITWT(wlan_config_get_he_twt_support(wdev))) {
				cap_1 &= ~DOT11AX_MAC_CAP_TWT_REQ;
				cap_1 |= DOT11AX_MAC_CAP_TWT_RSP;
			}
			if (TWT_SUPPORT_BTWT(wlan_config_get_he_twt_support(wdev)) &&
				btwt->btwt_element_exist &&
				btwt->btwt_bcn_offset) {
				cap_1 |= DOT11AX_MAC_CAP_BROADCAST_TWT;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
		cap_1 &= ~DOT11AX_MAC_CAP_FLEX_TWT_SCHDL;
		if (wlan_config_get_asic_twt_caps(wdev)) {
			if (TWT_SUPPORT_ITWT(wlan_config_get_he_twt_support(wdev))) {
				cap_1 |= DOT11AX_MAC_CAP_TWT_REQ;
				cap_1 &= ~DOT11AX_MAC_CAP_TWT_RSP;
			}
			if (TWT_SUPPORT_BTWT(wlan_config_get_he_twt_support(wdev))) {
				cap_1 |= DOT11AX_MAC_CAP_BROADCAST_TWT;
			}
		}
	} else {
		cap_1 &= ~(DOT11AX_MAC_CAP_TWT_REQ
				| DOT11AX_MAC_CAP_TWT_RSP
				| DOT11AX_MAC_CAP_BROADCAST_TWT
				| DOT11AX_MAC_CAP_FLEX_TWT_SCHDL);
	}
#endif /* WIFI_TWT_SUPPORT */

	if (wdev->wdev_type == WDEV_TYPE_STA) {
		cap_1 |= (((ppdu->trig_mac_pad_dur) << DOT11AX_MAC_CAP_TF_MAC_PAD_SHIFT)
			& DOT11AX_MAC_CAP_TF_MAC_PAD_MASK);
		cap_1 |= ((wlan_operate_get_he_af(wdev)) << DOT11AX_MAC_CAP_MAX_AMPDU_LEN_EXP_SHIFT);
	} else if (wdev->wdev_type == WDEV_TYPE_AP)  {
		cap_1 |= ((ppdu->he_max_ampdu_len_exp) << DOT11AX_MAC_CAP_MAX_AMPDU_LEN_EXP_SHIFT);
		cap_1 |= ((ppdu->max_agg_tid_num - 1) << DOT11AX_MAC_CAP_MULTI_TID_AGG_TX_SHIFT);
		/*cap_1 |= DOT11AX_MAC_CAP_ALL_ACK;*/
		cap_1 |= DOT11AX_MAC_CAP_BSR;
		/*cap_1 |= DOT11AX_MAC_CAP_ACK_EN_AGG;*/
		if (wlan_config_get_ul_mu_data_disable_rx(wdev))
			cap_2 |= DOT11AX_MAC_CAP_OM_CTRL_UL_MU_DIS_RX;
		cap_2 |= DOT11AX_MAC_CAP_BQR;
	}

	cap_1 |= DOT11AX_MAC_CAP_OM_CTRL;

	if (wlan_config_get_amsdu_en(wdev))
		cap_2 |= DOT11AX_MAC_CAP_AMSDU_IN_ACK_EN_AMPDU;

	he_mac_cap.mac_capinfo_1 = cpu_to_le32(cap_1);
	he_mac_cap.mac_capinfo_2 = cpu_to_le16(cap_2);
	NdisMoveMemory(f_buf, (UINT8 *)&he_mac_cap, sizeof(he_mac_cap));
	pos += sizeof(he_mac_cap);

	return pos;
}

static UINT8 *build_he_phy_cap(struct wifi_dev *wdev, UINT8 *f_buf)
{
	struct he_phy_capinfo he_phy_cap = {0};
	UINT32 phy_cap_1 = 0;
	UINT32 phy_cap_2 = 0;
	UINT8 phy_cap_3 = 0, phy_cap_4 = 0;
	UINT32 phy_ch_width = 0;
	UINT8 *pos = f_buf;
	enum PHY_CAP phy_caps;
	UINT8 he_bw = wlan_operate_get_he_bw(wdev);
#ifdef CONFIG_STA_SUPPORT
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#endif /* CONFIG_STA_SUPPORT */

	phy_caps = wlan_config_get_phy_caps(wdev);
	/*support 2.4G*/
	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_24G)
			&& WMODE_CAP_AX_2G(wdev->PhyMode)) {
#ifdef CONFIG_STA_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_STA) {
			if (he_bw >= HE_BW_2040)
				phy_ch_width |= SUPP_40M_CW_IN_24G_BAND;
		}
#endif
		if (he_bw == HE_BW_2040) {
			phy_ch_width |= SUPP_40M_CW_IN_24G_BAND;
#ifdef CONFIG_STA_SUPPORT
			if (wdev->wdev_type == WDEV_TYPE_STA) {
				if (IS_PHY_CAPS(phy_caps, fPHY_CAP_BW20_242TONE))
					phy_ch_width |= SUPP_20MSTA_RX_242TONE_RU_IN_24G_BAND;
			}
#endif
		}
	}
	/*support 5G or 6G*/
	if ((IS_PHY_CAPS(phy_caps, fPHY_CAP_5G) && WMODE_CAP_AX_5G(wdev->PhyMode)) ||
			(IS_PHY_CAPS(phy_caps, fPHY_CAP_6G) && WMODE_CAP_AX_6G(wdev->PhyMode))) {
		if (he_bw >= HE_BW_2040)
			phy_ch_width |= SUPP_40M_80M_CW_IN_5G_BAND;
		if (he_bw > HE_BW_80) {
			phy_ch_width |= SUPP_160M_CW_IN_5G_BAND;
			if (he_bw > HE_BW_160)
				phy_ch_width |= SUPP_160M_8080M_CW_IN_5G_BAND;
#ifdef CONFIG_STA_SUPPORT
			if (wdev->wdev_type == WDEV_TYPE_STA) {
				if (IS_PHY_CAPS(phy_caps, fPHY_CAP_BW20_242TONE))
					phy_ch_width |= SUPP_20MSTA_RX_242TONE_RU_IN_5G_BAND;
			}
#endif
		}
	}
	phy_cap_1 |= (phy_ch_width << DOT11AX_PHY_CAP_CH_WIDTH_SET_SHIFT);

#ifdef CONFIG_STA_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA) {
		phy_cap_1 |= DOT11AX_PHY_CAP_DEVICE_CLASS_A;
		phy_cap_1 |= DOT11AX_PHY_CAP_FULL_BW_UL_MU_MIMO;
		phy_cap_1 |= DOT11AX_PHY_CAP_PARTIAL_BW_UL_MU_MIMO;

#ifdef CFG_SUPPORT_FALCON_PP
		if ((IS_PHY_CAPS(phy_caps, fPHY_CAP_5G) || IS_PHY_CAPS(phy_caps, fPHY_CAP_6G)) &&
			(he_bw >= HE_BW_80)) {
			phy_cap_1 |= (0x3 << DOT11AX_PHY_CAP_PUNC_PREAMBLE_RX_SHIFT);
		}
#endif /* CFG_SUPPORT_FALCON_PP */

		phy_cap_2 |= DOT11AX_PHY_CAP_PPE_THRLD_PRESENT;
	}
#endif

	if (wlan_config_get_he_ldpc(wdev))
		phy_cap_1 |= DOT11AX_PHY_CAP_LDPC;


	if (wlan_config_get_he_tx_stbc(wdev)) {
		phy_cap_1 |= DOT11AX_PHY_CAP_TX_STBC_LE_EQ_80M;
		if (he_bw > HE_BW_80)
			phy_cap_2 |= DOT11AX_PHY_CAP_TX_STBC_GT_80M;
	}
	if (wlan_config_get_he_rx_stbc(wdev)) {
		phy_cap_1 |= DOT11AX_PHY_CAP_RX_STBC_LE_EQ_80M;
		if (he_bw > HE_BW_80)
			phy_cap_2 |= DOT11AX_PHY_CAP_RX_STBC_GT_80M;
	}

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		if (IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_PPE_EXIST))
			phy_cap_2 |= DOT11AX_PHY_CAP_PPE_THRLD_PRESENT;

		if (IS_PHY_CAPS(phy_caps, fPHY_CAP_TX_DOPPLER))
			phy_cap_1 |= DOT11AX_PHY_CAP_TX_DOPPLER;

		if (IS_PHY_CAPS(phy_caps, fPHY_CAP_RX_DOPPLER))
			phy_cap_1 |= DOT11AX_PHY_CAP_RX_DOPPLER;

		phy_cap_1 |= (0x2 << DOT11AX_PHY_CAP_TX_DCM_MAX_CONSTELLATION_SHIFT);
		phy_cap_1 |= (0x2 << DOT11AX_PHY_CAP_RX_DCM_MAX_CONSTELLATION_SHIFT);

		if (IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_ER_SU))
			if (!wlan_config_get_er_su_rx_disable(wdev))
				phy_cap_3 |= (DOT11AX_PHY_CAP_ER_SU_PPDU_4x_HE_LTF_DOT8_US |
						DOT11AX_PHY_CAP_ER_SU_PPDU_1x_HE_LTF_DOT8_US);

	}

#ifdef CONFIG_STA_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA) {
		if (IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_PPE_EXIST))
			phy_cap_2 |= DOT11AX_PHY_CAP_PPE_THRLD_PRESENT;

		phy_cap_1 |= (0x2 << DOT11AX_PHY_CAP_TX_DCM_MAX_CONSTELLATION_SHIFT);
		phy_cap_1 |= (0x2 << DOT11AX_PHY_CAP_RX_DCM_MAX_CONSTELLATION_SHIFT);

		if (IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_ER_SU))
			phy_cap_3 |= (DOT11AX_PHY_CAP_ER_SU_PPDU_4x_HE_LTF_DOT8_US |
					DOT11AX_PHY_CAP_ER_SU_PPDU_1x_HE_LTF_DOT8_US);
	}
#endif

	if (wlan_config_get_mu_dl_ofdma(wdev)) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			phy_cap_1 |= (0 << DOT11AX_PHY_CAP_PUNC_PREAMBLE_RX_SHIFT);
		}
	}

#ifdef CONFIG_STA_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA) {
		if (he_bw > HE_BW_160)
			phy_cap_3 |= (DOT11AX_PHY_CAP_20M_IN_160M_8080M_HE_PPDU |
					DOT11AX_PHY_CAP_80M_IN_160M_8080M_HE_PPDU);
		if (phy_ch_width & SUPP_20MSTA_RX_242TONE_RU_IN_24G_BAND)
			phy_cap_3 |= DOT11AX_PHY_CAP_20M_IN_40M_HE_PPDU_24G;
	}
#endif

#ifdef CONFIG_STA_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA)
		phy_cap_2 |= DOT11AX_PHY_CAP_PARTIAL_BW_DL_MU_MIMO;
#endif

	if (wlan_config_get_mu_ul_mimo(wdev)) {
		phy_cap_1 |= DOT11AX_PHY_CAP_FULL_BW_UL_MU_MIMO;
		phy_cap_1 |= DOT11AX_PHY_CAP_PARTIAL_BW_UL_MU_MIMO;
	}

	phy_cap_2 |= DOT11AX_PHY_CAP_PARTIAL_BW_ER;

#ifdef CONFIG_STA_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA) {
		if (!ad->CommonCfg.wifi_cert)
			phy_cap_1 |= DOT11AX_PHY_CAP_SU_PPDU_1x_HE_LTF_DOT8US_GI;

		phy_cap_2 |= DOT11AX_PHY_CAP_TRIG_CQI_FEEDBACK;

		phy_cap_2 |= DOT11AX_PHY_CAP_PWR_BOOST_FACTOR;
		phy_cap_2 |= DOT11AX_PHY_CAP_SU_MU_PPDU_4x_HE_LTF_DOT8_US;

		phy_cap_4 |= DOT11AX_PHY_CAP_LONGER_16_HE_SIGB_OFDM_SYM;
		phy_cap_4 |= DOT11AX_PHY_CAP_NON_TRIG_CQI_FEEDBACK;
		phy_cap_4 |= DOT11AX_PHY_CAP_TX_1024QAM_LT_242_TONE_RU;
		phy_cap_4 |= DOT11AX_PHY_CAP_RX_1024QAM_LT_242_TONE_RU;
		phy_cap_4 |= DOT11AX_PHY_CAP_RX_FULL_BW_HE_MU_PPDU_W_COMPRESS_SIGB;
		phy_cap_4 |= DOT11AX_PHY_CAP_RX_FULL_BW_HE_MU_PPDU_W_NON_COMPRESS_SIGB;
		phy_cap_4 |= (1 << DOT11AX_PHY_CAP_NOMINAL_PKT_PAD_SHIFT);
	}
#endif
	phy_cap_4 |= DOT11AX_PHY_CAP_TX_1024QAM_LT_242_TONE_RU;
	phy_cap_4 |= DOT11AX_PHY_CAP_RX_1024QAM_LT_242_TONE_RU;
#ifdef HE_TXBF_SUPPORT
	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_TXBF)) {
		struct he_bf_info he_bf_struct;

		NdisZeroMemory(&he_bf_struct, sizeof(struct he_bf_info));
		mt_wrap_get_he_bf_cap(wdev, &he_bf_struct);

		phy_cap_1 |= DOT11AX_PHY_CAP_NDP_4x_HE_LTF_3DOT2MS_GI;

		if (he_bf_struct.bf_cap & HE_SU_BFER)
			phy_cap_1 |= DOT11AX_PHY_CAP_SU_BFER;

		if (he_bf_struct.bf_cap & HE_SU_BFEE)
			phy_cap_2 |= DOT11AX_PHY_CAP_SU_BFEE;

		if (he_bf_struct.bf_cap & HE_MU_BFER) {
			if (wlan_config_get_mu_dl_mimo(wdev)) {
				if (wdev->wdev_type == WDEV_TYPE_AP)
					phy_cap_2 |= DOT11AX_PHY_CAP_MU_BFER;
			}
		}

		if (he_bf_struct.bf_cap & HE_BFEE_NG_16_SU_FEEDBACK)
			phy_cap_2 |= DOT11AX_PHY_CAP_NG16_SU_FEEDBACK;

		if (he_bf_struct.bf_cap & HE_BFEE_NG_16_MU_FEEDBACK)
			phy_cap_2 |= DOT11AX_PHY_CAP_NG16_MU_FEEDBACK;

		if (he_bf_struct.bf_cap & HE_BFEE_CODEBOOK_SU_FEEDBACK)
			phy_cap_2 |= DOT11AX_PHY_CAP_CODEBOOK_SU_FEEDBACK;

		if (he_bf_struct.bf_cap & HE_BFEE_CODEBOOK_MU_FEEDBACK)
			phy_cap_2 |= DOT11AX_PHY_CAP_CODEBOOK_MU_FEEDBACK;

		if (he_bf_struct.bf_cap & HE_TRIG_SU_BFEE_FEEDBACK)
			phy_cap_2 |= DOT11AX_PHY_CAP_TRIG_SU_BF_FEEDBACK;

		if (he_bf_struct.bf_cap & HE_TRIG_MU_BFEE_FEEDBACK)
			phy_cap_2 |= DOT11AX_PHY_CAP_TRIG_MU_BF_PARTIAL_BW_FEEDBACK;


		phy_cap_2 &= ~(DOT11AX_PHY_CAP_BFEE_STS_LE_EQ_80M_MASK);
		phy_cap_2 |= he_bf_struct.bfee_sts_le_eq_bw80 << DOT11AX_PHY_CAP_BFEE_STS_LE_EQ_80M_SHIFT;

		phy_cap_2 &= ~(DOT11AX_PHY_CAP_BFEE_STS_GT_80M_MASK);
		phy_cap_2 |= he_bf_struct.bfee_sts_gt_bw80 << DOT11AX_PHY_CAP_BFEE_STS_GT_80M_SHIFT;

		phy_cap_2 &= ~(DOT11AX_PHY_CAP_SOUND_DIM_NUM_LE_EQ_80M_MASK);
		phy_cap_2 |= he_bf_struct.snd_dim_le_eq_bw80 << DOT11AX_PHY_CAP_SOUND_DIM_NUM_LE_EQ_80M_SHIFT;

		phy_cap_2 &= ~(DOT11AX_PHY_CAP_SOUND_DIM_NUM_GT_80M_MASK);
		phy_cap_2 |= he_bf_struct.snd_dim_gt_bw80 << DOT11AX_PHY_CAP_SOUND_DIM_NUM_GT_80M_SHIFT;

		phy_cap_2 &= ~(DOT11AX_PHY_CAP_MAX_NC_MASK);
		phy_cap_2 |= he_bf_struct.bfee_max_nc << DOT11AX_PHY_CAP_MAX_NC_SHIFT;
	}
#endif

	if (!IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_PPE_EXIST))
		phy_cap_4 |= (NOMINAL_PAD_16US << DOT11AX_PHY_CAP_NOMINAL_PKT_PAD_SHIFT);

	he_phy_cap.phy_capinfo_1 = cpu_to_le32(phy_cap_1);
	he_phy_cap.phy_capinfo_2 = cpu_to_le32(phy_cap_2);
	he_phy_cap.phy_capinfo_3 = phy_cap_3;
	he_phy_cap.phy_capinfo_4 = phy_cap_4;
	NdisMoveMemory(f_buf, (UINT8 *)&he_phy_cap, sizeof(he_phy_cap));
	pos += sizeof(he_phy_cap);

	return pos;
}

UINT16 he_mcs_map(UINT8 nss, UINT8 he_mcs)
{
	UINT16 max_mcs_nss = 0xFFFF;

	max_mcs_nss &= ~(DOT11AX_MCS_1SS_MASK);
	max_mcs_nss |= HE_MAX_MCS_NSS(1, he_mcs);
	if (nss > 1) {
		max_mcs_nss &= ~(DOT11AX_MCS_2SS_MASK);
		max_mcs_nss |= HE_MAX_MCS_NSS(2, he_mcs);
	}
	if (nss > 2) {
		max_mcs_nss &= ~(DOT11AX_MCS_3SS_MASK);
		max_mcs_nss |= HE_MAX_MCS_NSS(3, he_mcs);
	}
	if (nss > 3) {
		max_mcs_nss &= ~(DOT11AX_MCS_4SS_MASK);
		max_mcs_nss |= HE_MAX_MCS_NSS(4, he_mcs);
	}
	if (nss > 4) {
		max_mcs_nss &= ~(DOT11AX_MCS_5SS_MASK);
		max_mcs_nss |= HE_MAX_MCS_NSS(5, he_mcs);
	}
	if (nss > 5) {
		max_mcs_nss &= ~(DOT11AX_MCS_6SS_MASK);
		max_mcs_nss |= HE_MAX_MCS_NSS(6, he_mcs);
	}
	if (nss > 6) {
		max_mcs_nss &= ~(DOT11AX_MCS_7SS_MASK);
		max_mcs_nss |= HE_MAX_MCS_NSS(7, he_mcs);
	}
	if (nss > 7) {
		max_mcs_nss &= ~(DOT11AX_MCS_8SS_MASK);
		max_mcs_nss |= HE_MAX_MCS_NSS(8, he_mcs);
	}

	return max_mcs_nss;
}

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
static UINT16 he_mcs_eap_map(UINT8 nss, UINT8 eap_mcs[])
{
	UINT16 max_mcs = 0xffff;
	UINT8 i;

	if (nss > HE_MAX_SUPPORT_STREAM)
		return max_mcs;

	for (i = 0; (i < nss) && (i < HE_MAX_SUPPORT_STREAM); i++) {
		max_mcs &= ~(0x3 << ( 2 * i));
		max_mcs |= eap_mcs[i] << (2 * i);
	}

	return max_mcs;
}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

static UINT8 *build_he_mcs_nss(struct wifi_dev *wdev, UINT8 *f_buf)
{
	struct he_txrx_mcs_nss he_mcs_nss = {0};
	struct mcs_nss_caps *mcs_nss = wlan_config_get_mcs_nss_caps(wdev);
	UINT8 he_bw = wlan_operate_get_he_bw(wdev);
	UINT8 tx_nss = wlan_config_get_tx_stream(wdev);
	UINT8 rx_nss = wlan_config_get_rx_stream(wdev);
	UINT8 fixed_mcs = wlan_config_get_fixed_mcs(wdev);
	UINT8 mcs_cap;
	UINT8 *pos = f_buf;

	if (fixed_mcs < FIXED_RATE_CMD_AUTO_MCS) {
		if (fixed_mcs <= HE_MCS_7)
			mcs_cap = HE_MCS_0_7;
		else if (fixed_mcs <= HE_MCS_9)
			mcs_cap = HE_MCS_0_9;
		else if (fixed_mcs <= HE_MCS_11)
			mcs_cap = HE_MCS_0_11;
		else
			mcs_cap = HE_MCS_0_11;
	} else
		mcs_cap = HE_MCS_0_11;

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
#define SET_HE_MCS_NSS(eap_rx_mcs, eap_tx_mcs)			\
	if (wdev->eap.eap_hesuprate_en) {			\
		he_mcs_nss.max_tx_mcs_nss = cpu_to_le16(	\
			he_mcs_eap_map(tx_nss, eap_tx_mcs));	\
		he_mcs_nss.max_rx_mcs_nss = cpu_to_le16(	\
			he_mcs_eap_map(rx_nss, eap_rx_mcs));	\
	}							\
	else {							\
		he_mcs_nss.max_tx_mcs_nss = cpu_to_le16(	\
			he_mcs_map(tx_nss, mcs_cap));		\
		he_mcs_nss.max_rx_mcs_nss = cpu_to_le16(	\
			he_mcs_map(rx_nss, mcs_cap));		\
	}
#else
#define SET_HE_MCS_NSS(eap_rx_mcs, eap_tx_mcs)		\
	he_mcs_nss.max_tx_mcs_nss = cpu_to_le16(	\
		he_mcs_map(tx_nss, mcs_cap));		\
	he_mcs_nss.max_rx_mcs_nss = cpu_to_le16(	\
		he_mcs_map(rx_nss, mcs_cap));
#endif

	SET_HE_MCS_NSS(wdev->eap.rate.he80_rx_nss_mcs,
		       wdev->eap.rate.he80_tx_nss_mcs);
	NdisMoveMemory(pos, (UINT8 *)&he_mcs_nss, sizeof(he_mcs_nss));
	pos += sizeof(he_mcs_nss);

	if (he_bw > HE_BW_80) {
		if (tx_nss > mcs_nss->bw160_max_nss)
			tx_nss = mcs_nss->bw160_max_nss;
		if (rx_nss > mcs_nss->bw160_max_nss)
			rx_nss = mcs_nss->bw160_max_nss;
		SET_HE_MCS_NSS(wdev->eap.rate.he160_rx_nss_mcs,
			       wdev->eap.rate.he160_tx_nss_mcs);
		NdisMoveMemory(pos, (UINT8 *)&he_mcs_nss, sizeof(he_mcs_nss));
		pos += sizeof(he_mcs_nss);

		if (he_bw > HE_BW_160) {
			SET_HE_MCS_NSS(wdev->eap.rate.he8080_rx_nss_mcs,
				       wdev->eap.rate.he8080_tx_nss_mcs);
			NdisMoveMemory(pos, (UINT8 *)&he_mcs_nss, sizeof(he_mcs_nss));
			pos += sizeof(he_mcs_nss);
		}
	}

	return pos;
}

VOID init_default_ppe(struct he_pe_info *pe_info, UINT8 max_nss, UINT8 max_ru_num)
{
	struct he_pe_thld *pe_thld = NULL;
	UINT8 ppe_max_len = 0;
	UINT8 i, j;

	ppe_max_len = max_nss * max_ru_num * sizeof(struct he_pe_thld);
	os_alloc_mem(NULL, &pe_info->pe_thld, ppe_max_len);
	if (!pe_info->pe_thld)
		return;
	pe_info->nss_m1 = max_nss - 1;
	pe_info->ru_num = max_ru_num;
	pe_thld = (struct he_pe_thld *)pe_info->pe_thld;
	for (i = 0; i < max_nss; i++) {
		for (j = 0; j < max_ru_num; j++, pe_thld++) {
			pe_thld->ppet16 = CONSTELLATION_BPSK_IDX;
			pe_thld->ppet8 = CONSTELLATION_NONE_IDX;
		}
	}
}

static UINT8 build_ppet_info_field(UINT8 *pos, UINT8 nss_num, UINT8 ru_num, UINT8 ru_alloc)
{
	UINT8 *ie_buf = pos;
	UINT8 i, ie_len = 0, array_idx = 0;
	UINT8 ppet_mask = 0xff, ppet_len = 0, ppet_bits = 0, ppet_pad_bit_bum = 0;
	/* Four consecutive sets of PPET8 for 011 and PPET16 for 100 in network order */
	UINT8 ppet_default[] = {0x1c, 0xc7, 0x71};
	BOOLEAN need_padd = FALSE;

	ppet_bits = (nss_num * (DOT11AX_PPE_PPET8_PPET16_BITS_NUM * ru_num));
	ppet_len = (ppet_bits >> 3);
	if ((ppet_bits & 0x07) != 0) {
		ppet_len++;
		need_padd = TRUE;
		ppet_pad_bit_bum = 8 - (ppet_bits & 0x07);
	}
	ie_len = 1 + ppet_len;
	*ie_buf = (UINT8)((nss_num - 1) | DOT11AX_PPE_RU_IDX_MSK(ru_alloc));
	ie_buf++;
	for (i = 0, array_idx = 0; i < ppet_len; i++, array_idx++) {
		/* go back to head and reuse element, avoid using division for performance purpose */
		if (array_idx == 3)
			array_idx = 0;
		*ie_buf = ppet_default[array_idx];
		if ((i == (ppet_len - 1)) && (need_padd == TRUE))
			*ie_buf = ppet_default[array_idx] & (ppet_mask >> ppet_pad_bit_bum);

		ie_buf++;
	}

	return ie_len;
}

static UINT8 *build_he_ppe(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	struct pe_control *pe_ctrl = hc_get_pe_ctrl(wdev);
	UINT8 ru_alloc_idx = DOT11AX_RU242_ALLOC_IDX_BITMSK;
	UINT8 he_bw = wlan_operate_get_he_bw(wdev);
	UINT8 nss_num = wlan_config_get_tx_stream(wdev);
	UINT8 ru_num = 1;
	UINT8 ie_len = 0;

	if (!pe_ctrl)
		return pos;

	if (he_bw > HE_BW_20) {
		ru_alloc_idx |= DOT11AX_RU484_ALLOC_IDX_BITMSK;
		ru_num++;
	}
	if (he_bw > HE_BW_2040) {
		ru_alloc_idx |= DOT11AX_RU996_ALLOC_IDX_BITMSK;
		ru_num++;
	}
	if (he_bw > HE_BW_80) {
		ru_alloc_idx |= DOT11AX_RU2x996_ALLOC_IDX_BITMSK;
		ru_num++;
	}

	if (pe_ctrl->pe_info.ru_num < ru_num)
		ru_num = pe_ctrl->pe_info.ru_num;
	ie_len = build_ppet_info_field(pos, nss_num, ru_num, ru_alloc_idx);
	pos += ie_len;

	return pos;
}

static UINT8 *build_he_cap_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	enum PHY_CAP phy_caps = wlan_config_get_phy_caps(wdev);
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	UINT8 ie_len = 0;

	eid->Eid = EID_EXTENSION;
	eid->Octet[0] = EID_EXT_HE_CAPS;
	pos += sizeof(struct _EID_STRUCT);

	/* HE MAC Capablities Information */
	pos = build_he_mac_cap(wdev, pos);
	/* HE PHY Capablities Information */
	pos = build_he_phy_cap(wdev, pos);
	/* Tx Rx HE-MCS NSS Support */
	pos = build_he_mcs_nss(wdev, pos);
	/* PPE Thresholds (optional) */
	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_PPE_EXIST))
		pos = build_he_ppe(wdev, pos);

	/*update eid_length final*/
	ie_len = pos - f_buf - 2;
	if (ie_len != 0)
		eid->Len = ie_len;

	return pos;
}

static UINT8 *build_he_op_params(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	/*UINT8 co_loc_bss = 0;*//*ALPS05330149*/
	UINT8 he_op_param2 = 0;
	UINT16 he_op_param1 = 0;
	UINT32 tmp = 0;
	struct ppdu_caps *ppdu = wlan_config_get_ppdu_caps(wdev);
	UINT8 vhtop_present = wlan_config_get_he_vhtop_present(wdev);
	UINT8 he6g_op_present = 0;
	UINT16 txop_rts_thld = wlan_operate_get_he_txop_dur_rts_thld(wdev);
	enum PHY_CAP phy_caps;

	/* default PE duration: 16us (unit: 4us) */
	tmp |= (ppdu->default_pe_duration & DOT11AX_OP_DEFAULT_PE_DURATION_MASK);
	/* TWT required */
	tmp &= ~DOT11AX_OP_TWT_REQUIRED;
	/* TXOP duration RTS threshold */
	tmp |= (txop_rts_thld << DOT11AX_OP_TXOP_DUR_RTS_THLD_SHIFT);
	/* vht op info present */
	if (vhtop_present)
		tmp |= DOT11AX_OP_VHT_OPINFO_PRESENT;
	/* co-located bss
	if (co_loc_bss)
		tmp |= DOT11AX_OP_CO_HOSTED_BSS;*//*ALPS05330149*/
	/* he_op_parm_1 */
	he_op_param1 = cpu_to_le16(tmp);
	NdisMoveMemory(pos, (UINT8 *)&he_op_param1, sizeof(he_op_param1));
	pos += sizeof(he_op_param1);

	tmp = 0;
	/* er su disable */
	phy_caps = wlan_config_get_phy_caps(wdev);
	if (IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_ER_SU))
		if (wlan_config_get_er_su_rx_disable(wdev))
			tmp |= DOT11AX_OP_ER_SU_DISABLE;

	/* he_op_parm_2 */
	he6g_op_present = wlan_config_get_he6g_op_present(wdev);
	if (he6g_op_present)
		tmp |= DOT11AX_OP_6G_OPINFO_PRESENT;
	he_op_param2 = (UINT8)tmp;
	NdisMoveMemory(pos, (UINT8 *)&he_op_param2, sizeof(he_op_param2));
	pos += sizeof(he_op_param2);

	return pos;
}

static UINT8 *build_he_bss_color_info(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	UINT8 intra_bss = wlan_operate_get_he_intra_bss_info(wdev);
	UINT8 he_bss_color, tmp = 0;

	tmp |= GET_BSS_COLOR(intra_bss);
	if (IS_PARTIAL_BSS_COLOR(intra_bss))
		tmp |= DOT11AX_PARTIAL_BSS_COLOR;
	if (IS_BSS_COLOR_DIS(intra_bss))
		tmp |= DOT11AX_BSS_COLOR_DISABLE;

	he_bss_color = tmp;
	NdisMoveMemory(pos, (UINT8 *)&he_bss_color, sizeof(he_bss_color));
	pos += sizeof(he_bss_color);

	return pos;
}

static UINT8 *build_basic_he_mcs_nss(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	UINT16 he_basic_mcs_nss = 0xFFFF;

	he_basic_mcs_nss = he_mcs_map(1, HE_MCS_0_7);
#ifdef RT_BIG_ENDIAN
	he_basic_mcs_nss = cpu2le16(he_basic_mcs_nss);
#endif
	NdisMoveMemory(pos, (UINT8 *)&he_basic_mcs_nss,
			sizeof(he_basic_mcs_nss));
	pos += sizeof(he_basic_mcs_nss);

	return pos;
}

static UINT8 *build_he_vht_op_info(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	UINT8 vhtop_present = wlan_config_get_he_vhtop_present(wdev);

	if (!vhtop_present)
		return pos;

	return pos;
}

static UINT8 *build_max_colocated_bssid_ind(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;

	return pos;
}

#ifdef CONFIG_6G_SUPPORT
static UINT8 *build_he_6g_op_info(struct wifi_dev *wdev, UINT8 *f_buf)
{
	struct he_6g_op_info he6g_op = {0};
	UINT8 *pos = f_buf;
	UINT8 he6g_present = wlan_config_get_he6g_op_present(wdev);
	UINT8 wdev_bw = wlan_operate_get_bw(wdev);
	UINT8 prim_ch = wdev->channel;
	UINT8 cent_ch_1 = wlan_operate_get_cen_ch_1(wdev);

	/*HE 6 GHz Operation Information field (option)*/
	if (he6g_present) {
		he6g_op.prim_ch = prim_ch;
		he6g_op.ctrl = HE_6G_OP_CTRL_SET_CH_WIDTH(wdev_bw);/*2:BW80,3:BW8080/160*/
		he6g_op.ctrl |= (1 << HE_6G_OP_CONTROL_DUP_BCN_SHIFT);
		he6g_op.min_rate = 6;

		if (HE_6G_OP_CTRL_GET_CH_WIDTH(he6g_op.ctrl) == 0x3) {
			/* BW160 */
			he6g_op.ccfs_0 = GET_BW160_PRIM80_CENT(prim_ch, cent_ch_1);
			he6g_op.ccfs_1 = cent_ch_1;
		} else {
			/* BW80 */
			he6g_op.ccfs_0 = cent_ch_1;
			he6g_op.ccfs_1 = 0;
		}
		NdisMoveMemory(pos, (UINT8 *)&he6g_op, sizeof(he6g_op));
		pos += sizeof(he6g_op);
	}

	return pos;
}
#endif

static UINT8 *build_he_op_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	UINT8 ie_len = 0;

	eid->Eid = EID_EXTENSION;
	eid->Octet[0] = EID_EXT_HE_OP;
	pos += sizeof(struct _EID_STRUCT);

	/* HE Operation Parameters */
	pos = build_he_op_params(wdev, pos);
	/* BSS Color Information */
	pos = build_he_bss_color_info(wdev, pos);
	/* Basic HE-MCS And Nss Set */
	pos = build_basic_he_mcs_nss(wdev, pos);
	/* VHT Operation Information */
	pos = build_he_vht_op_info(wdev, pos);
	/* MaxBSSID Indicator */
	pos = build_max_colocated_bssid_ind(wdev, pos);
#ifdef CONFIG_6G_SUPPORT
	/* 6 GHz Operation Information */
	pos = build_he_6g_op_info(wdev, pos);
#endif
	/*update eid_length final*/
	ie_len = pos - f_buf - 2;
	if (ie_len != 0)
		eid->Len = ie_len;

	return pos;
}

static UINT8 *build_btwt_ie(struct wifi_dev *wdev, UINT8 *f_buf, UINT32 f_len, UINT8 subtype)
{
	UINT8 *pos = f_buf;
	struct _RTMP_ADAPTER *ad = NULL;
	BTWT_BUF_STRUCT *btwt = NULL;
	struct btwt_ie *btwt_element = NULL;
	UINT8 band = 0;
	BOOLEAN btwt_element_exist = FALSE;
	UINT32 current_tsf[2] = {0};

	if (!wdev)
		return pos;

	ad = (struct _RTMP_ADAPTER *)(wdev->sys_handle);
	if (!ad)
		return pos;

	band = HcGetBandByWdev(wdev);

	NdisAcquireSpinLock(&ad->ApCfg.btwt_ie_lock);
	btwt = &ad->ApCfg.btwt[band];
	btwt_element = (struct btwt_ie *)&btwt->btwt_element;
	btwt_element_exist = btwt->btwt_element_exist;
	NdisReleaseSpinLock(&ad->ApCfg.btwt_ie_lock);

	if (btwt_element_exist && subtype == SUBTYPE_PROBE_RSP)
		twt_get_current_tsf(wdev, current_tsf);

	NdisAcquireSpinLock(&ad->ApCfg.btwt_ie_lock);
	if (subtype == SUBTYPE_BEACON)
		btwt->btwt_bcn_offset = f_len;
	if (subtype == SUBTYPE_PROBE_RSP)
		btwt->btwt_probe_rsp_offset = f_len;

	if (btwt->btwt_element_exist && btwt->btwt_bcn_offset) {
		if (subtype == SUBTYPE_PROBE_RSP)
			twt_update_btwt_twt(wdev, current_tsf);

		NdisMoveMemory(pos, btwt->btwt_element, btwt_element->len + 2);
		pos += (btwt_element->len + 2);
	}
	NdisReleaseSpinLock(&ad->ApCfg.btwt_ie_lock);

	return pos;
}

static UINT8 *build_mu_ac_param(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf, qos_info = 0, aci_aifsn = 0, w_max_min = 0;
	struct mu_edca_params he_mu_edca;
	struct mu_edca_cfg *mu_edca = wlan_config_get_he_mu_edca(wdev);
	struct mu_ac_param *ac_parm_rec = NULL;
	struct _EDCA_PARM *edca = wlan_config_get_ht_edca(wdev);
	UINT8 aci = 0;

	if ((edca == NULL) || (mu_edca == NULL))
		return pos;
	/* QoS info field */
	qos_info |= (edca->EdcaUpdateCount & DOT11AX_AP_QOS_INFO_EDCA_UPDATE_CNT_MASK);
	if (edca->bQAck)
		qos_info |= DOT11AX_AP_QOS_INFO_QACK;
	if (edca->bQueueRequest)
		qos_info |= DOT11AX_AP_QOS_INFO_QUE_REQ;
	if (edca->bTxopRequest)
		qos_info |= DOT11AX_AP_QOS_INFO_TXOP_REQ;
	he_mu_edca.qos_info = qos_info;

	for (aci = 0; aci < ACI_AC_NUM; aci++) {
		ac_parm_rec = &mu_edca->mu_ac_rec[aci];
		aci_aifsn =
			((ac_parm_rec->aifsn & DOT11AX_MU_EDCA_ACI_AIFSN_MASK) |
			(aci << DOT11AX_MU_EDCA_ACI_AIFSN_ACI_SHIFT));
		if (ac_parm_rec->acm)
			aci_aifsn |= DOT11AX_MU_EDCA_ACI_AIFSN_ACM;
		w_max_min =
			((ac_parm_rec->ecw_min & DOT11AX_MU_EDCA_ECWIN_MIN_MASK) |
			 (ac_parm_rec->ecw_max << DOT11AX_MU_EDCA_ECWIN_MAX_SHIFT));

		he_mu_edca.ac_rec[aci].aci_aifsn = aci_aifsn;
		he_mu_edca.ac_rec[aci].ec_wmax_wmin = w_max_min;
		he_mu_edca.ac_rec[aci].mu_edca_timer = ac_parm_rec->mu_edca_timer;
	}
	NdisMoveMemory(pos, (UINT8 *)&he_mu_edca, sizeof(he_mu_edca));
	pos += sizeof(he_mu_edca);

	return pos;
}

static UINT8 *build_he_mu_edca_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	enum PHY_CAP phy_caps;
	UINT8 ie_len = 0;

	phy_caps = wlan_config_get_phy_caps(wdev);
	if (!IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_UL_MUOFDMA))
		return pos;

	eid->Eid = EID_EXTENSION;
	eid->Octet[0] = EID_EXT_MU_EDCA_PARAM;
	pos += sizeof(struct _EID_STRUCT);

	pos = build_mu_ac_param(wdev, pos);

	/*update eid_length final*/
	ie_len = pos - f_buf - 2;
	if (ie_len != 0)
		eid->Len = ie_len;

	return pos;
}

static UINT8 *build_bss_color_change_announce_ie(struct wifi_dev *wdev, UINT8 *f_buf, UINT32 f_len)
{
	UINT8 *pos = f_buf;
	UINT8 next_color = 0, countdown = 0, ie_len = 0;
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;

	if (wlan_operate_get_he_bss_next_color(wdev, &next_color, &countdown) != WLAN_OPER_OK)
		return pos;

	/* no need to add this ie */
	if (countdown == 0) {
		wdev->bcn_buf.bcc_ie_location = 0;
		return pos;
	}

	wdev->bcn_buf.bcc_ie_location = f_len;
	eid->Eid = EID_EXTENSION;
	eid->Octet[0] = EID_EXT_BSS_COLOR_CHANGE_ANNOUNCE;
	pos += sizeof(struct _EID_STRUCT);

	NdisMoveMemory(pos, (UINT8 *)&countdown, sizeof(countdown));
	pos += sizeof(countdown);

	NdisMoveMemory(pos, (UINT8 *)&next_color, sizeof(next_color));
	pos += sizeof(next_color);

	/*update eid_length final*/
	ie_len = pos - f_buf - 2;
	if (ie_len != 0)
		eid->Len = ie_len;

	return pos;
}

static UINT8 *build_spatial_reuse_param_set_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	enum PHY_CAP phy_caps;
	UINT8 sr_control = 0, ie_len = 0;

	phy_caps = wlan_config_get_phy_caps(wdev);
	if (!IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_SR))
		return pos;

	eid->Eid = EID_EXTENSION;
	eid->Octet[0] = EID_EXT_SR_PARAM_SET;
	pos += sizeof(struct _EID_STRUCT);

	sr_control |= (DOT11AX_SR_CTRL_SRP_DISALLOW |
			DOT11AX_SR_CTRL_NONSRG_OBSS_PD_SR_DISALLOW);
	NdisMoveMemory(pos, (UINT8 *)&sr_control, sizeof(sr_control));
	pos += sizeof(sr_control);

	/*update eid_length final*/
	ie_len = pos - f_buf - 2;
	if (ie_len != 0)
		eid->Len = ie_len;

	return pos;
}

static UINT8 *build_uora_param_set_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	struct ul_ofdma_random_access uora;
	enum PHY_CAP phy_caps;
	UINT8 ie_len = 0;

	phy_caps = wlan_config_get_phy_caps(wdev);
	if (!IS_PHY_CAPS(phy_caps, fPHY_CAP_HE_UORA))
		return pos;

	eid->Eid = EID_EXTENSION;
	eid->Octet[0] = EID_EXT_UORA_PARAM_SET;
	pos += sizeof(struct _EID_STRUCT);

	uora.ocw_range = 0;
	NdisMoveMemory(pos, (UINT8 *)&uora, sizeof(uora));
	pos += sizeof(uora);

	/*update eid_length final*/
	ie_len = pos - f_buf - 2;
	if (ie_len != 0)
		eid->Len = ie_len;

	return pos;
}

#ifdef CONFIG_6G_SUPPORT
static UINT8 *build_he_6g_cap_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	UINT8 ie_len = 0;
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	struct he_6g_cap_ie he6g_cap = {0};
	struct ppdu_caps *ppdu;
	enum PHY_CAP phy_caps;

	phy_caps = wlan_config_get_phy_caps(wdev);
	if (!IS_PHY_CAPS(phy_caps, fPHY_CAP_6G) || !WMODE_CAP_AX_6G(wdev->PhyMode))
		return pos;

	eid->Eid = EID_EXTENSION;
	eid->Octet[0] = EID_EXT_HE_6G_CAPS;
	pos += sizeof(struct _EID_STRUCT);

	ppdu = (struct ppdu_caps *)wlan_config_get_ppdu_caps(wdev);
	he6g_cap.caps_info |= (DOT11AX_6G_SET_MPDU_LEN(ppdu->he6g_max_mpdu_len) |
			DOT11AX_6G_SET_MAX_AMPDU_LEN_EXP(ppdu->he6g_max_ampdu_len_exp) |
			DOT11AX_6G_SET_SMPS(ppdu->he6g_smps) |
			DOT11AX_6G_SET_MAX_MPDU_START_SPACE(ppdu->he6g_start_spacing));

	NdisMoveMemory(pos, (UINT8 *)&he6g_cap, sizeof(he6g_cap));
	pos += sizeof(he6g_cap);

	/*update eid_length final*/
	ie_len = pos - f_buf - 2;
	if (ie_len != 0)
		eid->Len = ie_len;

	return pos;
}

static UINT8 *build_he_6g_rnr(struct wifi_dev *wdev, UINT8 *f_buf, UINT32 queried_s_ssid)
{
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)f_buf;
	UINT8 *pos = f_buf;
	UINT8 ie_len = 0;
	struct neighbor_ap_info nap_info = {0};
	UINT8 tbtt_info_len = 0;
	struct _BSS_STRUCT *bss = (struct _BSS_STRUCT *)wdev->func_dev;
	struct _repted_bss_info *reported_bss = NULL;
	pdiscov_oob dsc_oob = &wdev->ap6g_cfg.dsc_oob;
	UINT32 reported_bss_features = 0;
	UINT32 reported_bss_s_ssid = 0;
	UINT32 i = 0;
	UINT8 bss_param = 0;
	UINT8 psd_bw20 = 1;                 /* temp value of 20 MHz PSD */
	BOOLEAN is_same_ssid = FALSE;
	BOOLEAN en_short_ssid = TRUE;		/* force carry short-ssid field */
	BOOLEAN en_queried_bss = FALSE;		/* reply queried bss only */
	BOOLEAN en_multi_rnr = FALSE;		/* neighbour APs be part of multiple RNR IEs */

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s: wdev(%d), queried_s_ssid(%x), reported count(%d), bss_list %p\n",
			 __func__, wdev->wdev_idx, queried_s_ssid, dsc_oob->repted_bss_cnt, dsc_oob->repted_bss_list);

	/* check reported count */
	if ((dsc_oob->repted_bss_cnt == 0) || (dsc_oob->repted_bss_list == NULL))
		return pos;

	RTMP_SEM_LOCK(&dsc_oob->list_lock);

	/*
	 * RNR IE rules:
	 *   1.Multiple-RNR: RNR IE for each APs
	 *   2.Single-RNR:   RNR IE for all APs
	 *                   - possibly different TBTT Info Len, froce carry short_ssid (same_ssid=0)
	 */

	for (i = 0; i < dsc_oob->repted_bss_cnt; i++) {
		reported_bss = dsc_oob->repted_bss_list + i;

		/* check if reply queried bss only */
		reported_bss_s_ssid = Crcbitbybitfast(reported_bss->ssid, reported_bss->ssid_len);
		if (en_queried_bss && (queried_s_ssid != 0) && (reported_bss_s_ssid != queried_s_ssid))
			continue;

		/* Add IE for multi_rnr or 1st RNR */
		if ((en_multi_rnr) || (i == 0)) {

			/* EID per-reported bss */
			eid->Eid = EID_REDUCED_NEIGHBOR_REPORT;
			pos = &eid->Octet[0];

			/* if same ssid, short-ssid is useless */
			if ((en_multi_rnr) && (!en_short_ssid) && (reported_bss->ssid_len == bss->SsidLen) &&
				(memcmp(reported_bss->ssid, bss->Ssid, reported_bss->ssid_len) == 0)) {
				is_same_ssid = TRUE;
				tbtt_info_len = sizeof(struct he_6g_rnr_tbtt_info_set_wo_ssid);
			} else {
				is_same_ssid = FALSE;
				tbtt_info_len = sizeof(struct he_6g_rnr_tbtt_info_set_w_ssid);
			}

			/*Neighbot AP Information: TBTT Information Header/Operating Class/Channel Number*/
			nap_info.tbtt_info_hdr |= DOT11_RNR_TBTT_INFO_HDR_FILTERED_NEIGHBOR_AP;

			if (en_multi_rnr) {
				DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_COUNT(nap_info.tbtt_info_hdr, 0);	/* minus one */
			} else {
				DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_COUNT(nap_info.tbtt_info_hdr, (dsc_oob->repted_bss_cnt - 1));	/* minus one */
			}

			DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_LEN(nap_info.tbtt_info_hdr, tbtt_info_len);
			nap_info.op_class = reported_bss->op_class;
			nap_info.ch_num = reported_bss->channel;
			NdisMoveMemory(pos, (UINT8 *)&nap_info, sizeof(nap_info));
			pos += sizeof(nap_info);
		}

		reported_bss_features = reported_bss->bss_feature_set;
		bss_param =
			(is_same_ssid ? TBTT_INFO_BSS_PARAM_SAME_SSID : 0) |
			TBTT_INFO_BSS_PARAM_MULTI_BSSID |	/* always enable Multiple-BSSID bit */
			((reported_bss_features & AP_6G_TRANS_BSSID) ? TBTT_INFO_BSS_PARAM_TRANSMIT_BSSID : 0) |
			((reported_bss_features & AP_6G_UNSOL_PROBE_RSP_EN) ?
				TBTT_INFO_BSS_PARAM_UNSOLICITED_PROBE_RSP_ACTIVE : 0) |
			TBTT_INFO_BSS_PARAM_COLOCATED_AP;

		/*Neighbor AP Information: TBTT Information Set*/
		if (is_same_ssid) {
			struct he_6g_rnr_tbtt_info_set_wo_ssid he6g_rnr_tbtt_info_wo_ssid = {0};

			NdisZeroMemory((UINT8 *)&he6g_rnr_tbtt_info_wo_ssid, sizeof(he6g_rnr_tbtt_info_wo_ssid));
			he6g_rnr_tbtt_info_wo_ssid.nap_tbtt_offset = 0xFF;
			COPY_MAC_ADDR(he6g_rnr_tbtt_info_wo_ssid.bssid, reported_bss->bssid);
			he6g_rnr_tbtt_info_wo_ssid.bss_param = bss_param;
			he6g_rnr_tbtt_info_wo_ssid.psd_bw20 = psd_bw20;
			NdisMoveMemory(pos, (UINT8 *)&he6g_rnr_tbtt_info_wo_ssid, sizeof(he6g_rnr_tbtt_info_wo_ssid));

			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "\tbssid=%02x-%02x-%02x-%02x-%02x-%02x, bss_param=%02x\n",
					 PRINT_MAC(he6g_rnr_tbtt_info_wo_ssid.bssid),
					 he6g_rnr_tbtt_info_wo_ssid.bss_param);
		} else {
			struct he_6g_rnr_tbtt_info_set_w_ssid he6g_rnr_tbtt_info_w_ssid = {0};

			NdisZeroMemory((UINT8 *)&he6g_rnr_tbtt_info_w_ssid, sizeof(he6g_rnr_tbtt_info_w_ssid));
			he6g_rnr_tbtt_info_w_ssid.nap_tbtt_offset = 0xFF;
			COPY_MAC_ADDR(he6g_rnr_tbtt_info_w_ssid.bssid, reported_bss->bssid);
			he6g_rnr_tbtt_info_w_ssid.short_ssid = reported_bss_s_ssid;
			he6g_rnr_tbtt_info_w_ssid.bss_param = bss_param;
			he6g_rnr_tbtt_info_w_ssid.psd_bw20 = psd_bw20;
			NdisMoveMemory(pos, (UINT8 *)&he6g_rnr_tbtt_info_w_ssid, sizeof(he6g_rnr_tbtt_info_w_ssid));

			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "\tbssid=%02x-%02x-%02x-%02x-%02x-%02x, short_ssid=%08x, bss_param=%02x\n",
					 PRINT_MAC(he6g_rnr_tbtt_info_w_ssid.bssid),
					 he6g_rnr_tbtt_info_w_ssid.short_ssid,
					 he6g_rnr_tbtt_info_w_ssid.bss_param);
		}
		pos += tbtt_info_len;

		/* update eid_length to multi_rnr or last RNR */
		if ((en_multi_rnr) || (i == (dsc_oob->repted_bss_cnt - 1))) {
			ie_len = pos - (UINT8 *)eid - 2;
			if (ie_len != 0) {
				eid->Len = ie_len;
			}

			MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "\tAdd IE Len = %d\n", eid->Len);

			/* point to next EID */
			eid = (struct _EID_STRUCT *)pos;
		}
	}

	RTMP_SEM_UNLOCK(&dsc_oob->list_lock);

	return pos;
}
#endif /* CONFIG_6G_SUPPORT */

/*
 * Defined in IEEE 802.11AX
 * Appeared in Beacon, ProbResp frames
 */
INT build_he_txpwr_envelope(struct wifi_dev *wdev, UINT8 *f_buf)
{
	INT len = 0, pwr_cnt;
	UINT8 u1TxPwrInterpretation, u1TxpwrCategory;
	UINT8 u1NValue;
	HE_TXPWR_ENV_IE txpwr_env;
	UCHAR he_bw = wlan_config_get_he_bw(wdev);
	UCHAR ht_bw = wlan_operate_get_ht_bw(wdev);
	UCHAR ucBand = wlan_config_get_ch_band(wdev);

	NdisZeroMemory(&txpwr_env, sizeof(txpwr_env));

	if (ucBand != CMD_CH_BAND_6G) {
		u1TxpwrCategory = TX_PWR_CATEGORY_DEFAULT;
		u1TxPwrInterpretation = TX_PWR_INTERPRET_REG_CLIENT_EIRP;
	}
	else {
		u1TxpwrCategory = TX_PWR_CATEGORY_DEFAULT;
		u1TxPwrInterpretation = TX_PWR_INTERPRET_REG_CLIENT_EIRP_PSD;
	}

	if ((u1TxPwrInterpretation == TX_PWR_INTERPRET_EIRP)
		|| (u1TxPwrInterpretation == TX_PWR_INTERPRET_REG_CLIENT_EIRP)) {
		if ((he_bw == HE_BW_160)
			|| (he_bw == HE_BW_8080))
			pwr_cnt = 3;
		else if (he_bw == HE_BW_80)
			pwr_cnt = 2;
		else {
			if (ht_bw == HT_BW_40)
				pwr_cnt = 1;
			else
				pwr_cnt = 0;
		}

		u1NValue = pwr_cnt;
		txpwr_env.tx_pwr_info.max_tx_pwr_category = TX_PWR_CATEGORY_DEFAULT;
		txpwr_env.tx_pwr_info.max_tx_pwr_cnt = pwr_cnt;
		txpwr_env.tx_pwr_info.max_tx_pwr_interpretation = u1TxPwrInterpretation;

		for (len = 0; len <= u1NValue; len++)
			txpwr_env.tx_pwr_bw[len] = 47;
	}

	else if ((u1TxPwrInterpretation == TX_PWR_INTERPRET_EIRP_PSD)
		|| (u1TxPwrInterpretation == TX_PWR_INTERPRET_REG_CLIENT_EIRP_PSD)) {
		if ((he_bw == HE_BW_160)
			|| (he_bw == HE_BW_8080))
			pwr_cnt = 4;
		else if (he_bw == HE_BW_80)
			pwr_cnt = 3;
		else {
			if (ht_bw == HT_BW_40)
				pwr_cnt = 2;
			else if (ht_bw == HT_BW_20)
				pwr_cnt = 1;
			else
				pwr_cnt = 0;
		}

		switch (pwr_cnt) {
			case 0:
				u1NValue = 0;
				break;
			case 1:
				u1NValue = 0;
				break;
			case 2:
				u1NValue = 1;
				break;
			case 3:
				u1NValue = 3;
				break;
			case 4:
				u1NValue = 7;
				break;
			default:
				u1NValue = 0;
		}

		txpwr_env.tx_pwr_info.max_tx_pwr_cnt = pwr_cnt;
		txpwr_env.tx_pwr_info.max_tx_pwr_interpretation = u1TxPwrInterpretation;
		txpwr_env.tx_pwr_info.max_tx_pwr_category = u1TxpwrCategory;

		for (len = 0; len <= u1NValue; len++)
			txpwr_env.tx_pwr_bw[len] = 0xFE;
	}

	len = 1 + len;
	NdisMoveMemory(f_buf, &txpwr_env, len);
	if (parse_transmit_power_envelope_ie(pwr_cnt, len))
		return len;
	else
		return 0;
}

/*
 * Build up HE IEs for AP
 */
UINT32 add_beacon_he_ies(struct wifi_dev *wdev, UINT8 *f_buf, UINT32 f_len)
{
	UINT8 *local_fbuf = f_buf + f_len;
	UINT32 offset = 0;

	/*RNR*/
	/*Multiple BSSID confg*/
	/*HE Cap.*/
	local_fbuf = build_he_cap_ie(wdev, local_fbuf);
	/*HE Op.*/
	local_fbuf = build_he_op_ie(wdev, local_fbuf);
	/*TWT*/
#ifdef WIFI_TWT_SUPPORT
	local_fbuf = build_btwt_ie(wdev, local_fbuf, (local_fbuf - f_buf), SUBTYPE_BEACON);
#endif /* WIFI_TWT_SUPPORT */
	/*UORA Param Set*/
	local_fbuf = build_uora_param_set_ie(wdev, local_fbuf);
	/*BSS color change announce*/
	local_fbuf = build_bss_color_change_announce_ie(wdev, local_fbuf, (local_fbuf - f_buf));
	/*Spatial Reuse param*/
	local_fbuf = build_spatial_reuse_param_set_ie(wdev, local_fbuf);
	/*MU EDCA param*/
	local_fbuf = build_he_mu_edca_ie(wdev, local_fbuf);
#ifdef CONFIG_6G_SUPPORT
	/*HE 6 GHz Band Capabilities*/
	local_fbuf = build_he_6g_cap_ie(wdev, local_fbuf);
#endif

	offset = (UINT32)(local_fbuf - f_buf - f_len);

	return offset;
}

UINT32 add_probe_rsp_he_ies(struct wifi_dev *wdev, UINT8 *f_buf, UINT32 f_len)
{
	UINT8 *local_fbuf = f_buf + f_len;
	UINT32 offset = 0;

	/*RNR*/
	/*Multiple BSSID confg*/
	/*HE Cap.*/
	local_fbuf = build_he_cap_ie(wdev, local_fbuf);
	/*HE Op.*/
	local_fbuf = build_he_op_ie(wdev, local_fbuf);
	/*TWT*/
#ifdef WIFI_TWT_SUPPORT
	local_fbuf = build_btwt_ie(wdev, local_fbuf, (local_fbuf - f_buf), SUBTYPE_PROBE_RSP);
#endif /* WIFI_TWT_SUPPORT */
	/*UORA Param Set*/
	local_fbuf = build_uora_param_set_ie(wdev, local_fbuf);
	/*BSS color change announce*/
	local_fbuf = build_bss_color_change_announce_ie(wdev, local_fbuf, (local_fbuf - f_buf));
	/*Spatial Reuse param*/
	local_fbuf = build_spatial_reuse_param_set_ie(wdev, local_fbuf);
	/*MU EDCA param*/
	local_fbuf = build_he_mu_edca_ie(wdev, local_fbuf);
#ifdef CONFIG_6G_SUPPORT
	/*HE 6 GHz Band Capabilities*/
	local_fbuf = build_he_6g_cap_ie(wdev, local_fbuf);
#endif

	offset = (UINT32)(local_fbuf - f_buf - f_len);

	return offset;
}

UINT32 add_assoc_rsp_he_ies(struct wifi_dev *wdev, UINT8 *f_buf, UINT32 f_len)
{
	UINT8 *local_fbuf = f_buf + f_len;
	UINT32 offset = 0;

	/*TWT*/
	/*HE Cap.*/
	local_fbuf = build_he_cap_ie(wdev, local_fbuf);
	/*HE Op.*/
	local_fbuf = build_he_op_ie(wdev, local_fbuf);
	/*BSS color change announce*/
	local_fbuf = build_bss_color_change_announce_ie(wdev, local_fbuf, (local_fbuf - f_buf));
	/*Spatial Reuse param*/
	local_fbuf = build_spatial_reuse_param_set_ie(wdev, local_fbuf);
	/*MU EDCA param*/
	local_fbuf = build_he_mu_edca_ie(wdev, local_fbuf);
	/*UORA paramter*/
	local_fbuf = build_uora_param_set_ie(wdev, local_fbuf);
#ifdef CONFIG_6G_SUPPORT
	/*HE 6 GHz Band Capabilities*/
	local_fbuf = build_he_6g_cap_ie(wdev, local_fbuf);
#endif

	offset = (UINT32)(local_fbuf - f_buf - f_len);
	/*dump_he_ies("build he assoc_rsp", f_buf + f_len, offset);*/

	return offset;
}

UINT32 add_reassoc_rsp_he_ies(struct wifi_dev *wdev, UINT8 *f_buf, UINT32 f_len)
{
	UINT8 *local_fbuf = f_buf + f_len;
	UINT32 offset = 0;

	local_fbuf = build_he_cap_ie(wdev, local_fbuf);
	local_fbuf = build_he_op_ie(wdev, local_fbuf);
	local_fbuf = build_bss_color_change_announce_ie(wdev, local_fbuf, f_len);
	local_fbuf = build_spatial_reuse_param_set_ie(wdev, local_fbuf);
	local_fbuf = build_he_mu_edca_ie(wdev, local_fbuf);
#ifdef CONFIG_6G_SUPPORT
	/*HE 6 GHz Band Capabilities*/
	local_fbuf = build_he_6g_cap_ie(wdev, local_fbuf);
#endif

	offset = (UINT32)(local_fbuf - f_buf - f_len);

	return offset;
}

#ifdef CONFIG_6G_SUPPORT
UINT32 add_he_6g_rnr_ie(struct wifi_dev *wdev, UINT8 *f_buf, UINT32 f_len, UINT32 queried_s_ssid)
{
	UINT8 *local_fbuf = f_buf + f_len;
	UINT32 offset = 0;

	local_fbuf = build_he_6g_rnr(wdev, local_fbuf, queried_s_ssid);
	offset = (UINT32)(local_fbuf - f_buf - f_len);

	return offset;
}
#endif

UINT32 add_fils_tpe_ie(struct wifi_dev *wdev, UINT8 *f_buf, UINT32 f_len)
{
	UINT8 *local_fbuf = f_buf + f_len;
	UINT32 offset = 0;

	const UINT8 he_txpwr_env_ie = IE_VHT_TXPWR_ENV;
	HE_TXPWR_ENV_IE txpwr_env;
	UINT8 u1tp_len;

	*local_fbuf = he_txpwr_env_ie;

	u1tp_len = build_he_txpwr_envelope(wdev, (UCHAR *)&txpwr_env);;
	*(local_fbuf + 1) = u1tp_len;
	local_fbuf += 2;
	NdisMoveMemory(local_fbuf, &txpwr_env, u1tp_len);
	local_fbuf += u1tp_len;

	offset = (UINT32)(local_fbuf - f_buf - f_len);

	return offset;
}

/*
 * Build up IEs for STA
 */
UINT32 add_probe_req_he_ies(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *local_fbuf = f_buf;
	UINT32 offset = 0;

	/*HE Cap.*/
	local_fbuf = build_he_cap_ie(wdev, local_fbuf);
#ifdef CONFIG_6G_SUPPORT
	/*HE 6 GHz Band Capabilities*/
	local_fbuf = build_he_6g_cap_ie(wdev, local_fbuf);
#endif
	offset = (UINT32)(local_fbuf - f_buf);

	return offset;
}

UINT32 add_assoc_req_he_ies(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *local_fbuf = f_buf;
	UINT32 offset = 0;

	/*HE Cap.*/
	local_fbuf = build_he_cap_ie(wdev, local_fbuf);
	/*HE Op.*/
	local_fbuf = build_he_op_ie(wdev, local_fbuf);
#ifdef CONFIG_6G_SUPPORT
	/*HE 6 GHz Band Capabilities*/
	local_fbuf = build_he_6g_cap_ie(wdev, local_fbuf);
#endif
	offset = (UINT32)(local_fbuf - f_buf);
	/*Channel Switch Timing*/

	return offset;
}

UINT32 add_reassoc_req_he_ies(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *local_fbuf = f_buf;
	UINT32 offset = 0;

	local_fbuf = build_he_cap_ie(wdev, local_fbuf);
#ifdef CONFIG_6G_SUPPORT
	/*HE 6 GHz Band Capabilities*/
	local_fbuf = build_he_6g_cap_ie(wdev, local_fbuf);
#endif
	offset = (UINT32)(local_fbuf - f_buf);

	return offset;
}

/*
 * Parsing HE IEs
 */
static VOID peer_he_mac_caps(struct _MAC_TABLE_ENTRY *peer, struct he_mac_capinfo *mac_cap)
{
	/*init he_mac_cap*/
	peer->cap.he_mac_cap = 0;
	if (mac_cap->mac_capinfo_1 & DOT11AX_MAC_CAP_HTC)
		peer->cap.he_mac_cap |= HE_HTC;
	if (mac_cap->mac_capinfo_1 & DOT11AX_MAC_CAP_TWT_REQ)
		peer->cap.he_mac_cap |= HE_TWT_REQUEST;
	if (mac_cap->mac_capinfo_1 & DOT11AX_MAC_CAP_TWT_RSP)
		peer->cap.he_mac_cap |= HE_TWT_RESPOND;
	peer->cap.he_frag_level = GET_DOT11AX_FRAG_LVL(mac_cap->mac_capinfo_1);
	peer->cap.max_frag_msdu_num =
		GET_DOT11AX_MAX_FRAG_MSDU_AMSDU_EXP(mac_cap->mac_capinfo_1);
	peer->cap.min_frag_size =
		GET_DOT11AX_MIN_FRAG_SIZE(mac_cap->mac_capinfo_1);
	peer->cap.tf_mac_pad_duration =
		GET_DOT11AX_TF_MAC_PAD(mac_cap->mac_capinfo_1);
	peer->cap.ampdu.multi_tid_agg =
		GET_DOT11AX_MULTI_TID_AGG_TX(mac_cap->mac_capinfo_1);
	peer->cap.he_link_adapt = GET_DOT11AX_LINK_ADPT(mac_cap->mac_capinfo_1);
	if (mac_cap->mac_capinfo_1 & DOT11AX_MAC_CAP_ALL_ACK)
		peer->cap.he_mac_cap |= HE_ALL_ACK;
	if (mac_cap->mac_capinfo_1 & DOT11AX_MAC_CAP_UMRS)
		peer->cap.he_mac_cap |= HE_UMRS;
	if (mac_cap->mac_capinfo_1 & DOT11AX_MAC_CAP_BSR)
		peer->cap.he_mac_cap |= HE_BSR;
	if (mac_cap->mac_capinfo_1 & DOT11AX_MAC_CAP_BROADCAST_TWT)
		peer->cap.he_mac_cap |= HE_BROADCAST_TWT;
	if (mac_cap->mac_capinfo_1 & DOT11AX_MAC_CAP_32BA_BITMAP)
		peer->cap.he_mac_cap |= HE_32BIT_BA_BITMAP;
	if (mac_cap->mac_capinfo_1 & DOT11AX_MAC_CAP_MU_CASCADE)
		peer->cap.he_mac_cap |= HE_MU_CASCADING;
	if (mac_cap->mac_capinfo_1 & DOT11AX_MAC_CAP_ACK_EN_AGG)
		peer->cap.he_mac_cap |= HE_ACK_EN_AGG;
	if (mac_cap->mac_capinfo_1 & DOT11AX_MAC_CAP_OM_CTRL)
		peer->cap.he_mac_cap |= HE_OM_CTRL;
	if (mac_cap->mac_capinfo_1 & DOT11AX_MAC_CAP_OFDMA_RA)
		peer->cap.he_mac_cap |= HE_OFDMA_RA;
	peer->cap.ampdu.max_he_ampdu_len_exp =
		GET_DOT11AX_MAX_AMPDU_LEN_EXP(mac_cap->mac_capinfo_1);
	if (mac_cap->mac_capinfo_1 & DOT11AX_MAC_CAP_AMSDU_FRAG)
		peer->cap.he_mac_cap |= HE_AMSDU_FRAG;
	if (mac_cap->mac_capinfo_1 & DOT11AX_MAC_CAP_FLEX_TWT_SCHDL)
		peer->cap.he_mac_cap |= HE_FLEX_TWT_SCHDL;
	if (mac_cap->mac_capinfo_1 & DOT11AX_MAC_CAP_RX_CTRL_FRAME_2_MULTI_BSS)
		peer->cap.he_mac_cap |= HE_RX_CTRL_FRAME_TO_MULTIBSS;

	if (mac_cap->mac_capinfo_2 & DOT11AX_MAC_CAP_BSRP_BQRP_AMPDU_AGG)
		peer->cap.he_mac_cap |= HE_BSRP_BQRP_AMPDU_AGG;
	if (mac_cap->mac_capinfo_2 & DOT11AX_MAC_CAP_QTP)
		peer->cap.he_mac_cap |= HE_QTP;
	if (mac_cap->mac_capinfo_2 & DOT11AX_MAC_CAP_BQR)
		peer->cap.he_mac_cap |= HE_BQR;
	if (mac_cap->mac_capinfo_2 & DOT11AX_MAC_CAP_SRP_RSP)
		peer->cap.he_mac_cap |= HE_SRP_RESPONDER;
	if (mac_cap->mac_capinfo_2 & DOT11AX_MAC_CAP_NDP_FEEDBACK_REPORT)
		peer->cap.he_mac_cap |= HE_NDP_FEEDBACK_REPORT;
	if (mac_cap->mac_capinfo_2 & DOT11AX_MAC_CAP_OPS)
		peer->cap.he_mac_cap |= HE_OPS;
	if (mac_cap->mac_capinfo_2 & DOT11AX_MAC_CAP_AMSDU_IN_ACK_EN_AMPDU)
		peer->cap.he_mac_cap |= HE_AMSDU_IN_ACK_EN_AMPDU;
	if (mac_cap->mac_capinfo_2 & DOT11AX_MAC_CAP_HE_DYN_SMPS)
		peer->cap.he_mac_cap |= HE_DYN_SMPS;
}

UINT8 peer_max_bw_cap(INT8 ch_width_set)
{
	UINT8 ch_width = HE_BW_20;

	if (ch_width_set & SUPP_40M_CW_IN_24G_BAND)
		ch_width = HE_BW_2040;
	if (ch_width_set & SUPP_40M_80M_CW_IN_5G_BAND) {
		ch_width = HE_BW_80;
		if (ch_width_set & SUPP_160M_8080M_CW_IN_5G_BAND)
			ch_width = HE_BW_8080;
		if (ch_width_set & SUPP_160M_CW_IN_5G_BAND)
			ch_width = HE_BW_160;
	}
	return ch_width;
}

static UINT8 peer_bw20_242tone(INT8 ch_width_set)
{
	UINT8 bw20_242tone = 0;

	if ((ch_width_set & SUPP_20MSTA_RX_242TONE_RU_IN_24G_BAND) ||
			(ch_width_set & SUPP_20MSTA_RX_242TONE_RU_IN_5G_BAND))
		bw20_242tone = 1;
	return bw20_242tone;
}


static VOID peer_he_phy_caps(struct _MAC_TABLE_ENTRY *peer, struct he_phy_capinfo *phy_cap)
{
	/*init he_phy_cap/he_gi/he_stbc*/
	peer->cap.he_phy_cap = 0;
	peer->cap.he_gi = 0;
	peer->cap.stbc.he_stbc = 0;
	peer->cap.he_bf.bf_cap = 0;
	peer->cap.ch_bw.he_ch_width =
		GET_DOT11AX_CH_WIDTH(phy_cap->phy_capinfo_1);
	peer->cap.ch_bw.he_bw20_242tone =
		peer_bw20_242tone(GET_DOT11AX_CH_WIDTH(phy_cap->phy_capinfo_1));
	peer->cap.punc_preamble_rx =
		GET_DOT11AX_PUNC_PREAMBLE_RX(phy_cap->phy_capinfo_1);
	if (phy_cap->phy_capinfo_1 & DOT11AX_PHY_CAP_DEVICE_CLASS_A)
		peer->cap.he_phy_cap |= HE_DEV_CLASS_A;
	if (phy_cap->phy_capinfo_1 & DOT11AX_PHY_CAP_LDPC)
		peer->cap.he_phy_cap |= HE_LDPC;
	peer->cap.midamble_rx_max_nsts =
		GET_DOT11AX_MIDAMBLE_RX_NSTS(phy_cap->phy_capinfo_1);
	/*he_gi*/
	if (phy_cap->phy_capinfo_1 & DOT11AX_PHY_CAP_SU_PPDU_1x_HE_LTF_DOT8US_GI)
		peer->cap.he_gi |= HE_SU_PPDU_1x_LTF_DOT8US_GI;
	if (phy_cap->phy_capinfo_1 & DOT11AX_PHY_CAP_NDP_4x_HE_LTF_3DOT2MS_GI)
		peer->cap.he_gi |= HE_NDP_4x_LTF_3DOT2MS_GI;
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_SU_MU_PPDU_4x_HE_LTF_DOT8_US)
		peer->cap.he_gi |= HE_SU_PPDU_MU_PPDU_4x_LTF_DOT8US_GI;
	if (phy_cap->phy_capinfo_3 & DOT11AX_PHY_CAP_ER_SU_PPDU_4x_HE_LTF_DOT8_US)
		peer->cap.he_gi |= HE_ER_SU_PPDU_4x_LTF_DOT8US_GI;
	if (phy_cap->phy_capinfo_3 & DOT11AX_PHY_CAP_ER_SU_PPDU_1x_HE_LTF_DOT8_US)
		peer->cap.he_gi |= HE_ER_SU_PPDU_1x_LTF_DOT8US_GI;
	/*stbc*/
	if (phy_cap->phy_capinfo_1 & DOT11AX_PHY_CAP_TX_STBC_LE_EQ_80M)
		peer->cap.stbc.he_stbc |= HE_LE_EQ_80M_TX_STBC;
	if (phy_cap->phy_capinfo_1 & DOT11AX_PHY_CAP_RX_STBC_LE_EQ_80M)
		peer->cap.stbc.he_stbc |= HE_LE_EQ_80M_RX_STBC;
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_TX_STBC_GT_80M)
		peer->cap.stbc.he_stbc |= HE_GT_80M_TX_STBC;
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_RX_STBC_GT_80M)
		peer->cap.stbc.he_stbc |= HE_GT_80M_RX_STBC;
	/*doppler*/
	if (phy_cap->phy_capinfo_1 & DOT11AX_PHY_CAP_TX_DOPPLER)
		peer->cap.he_phy_cap |= HE_DOPPLER_TX;
	if (phy_cap->phy_capinfo_1 & DOT11AX_PHY_CAP_RX_DOPPLER)
		peer->cap.he_phy_cap |= HE_DOPPLER_RX;
	/*mu*/
	if (phy_cap->phy_capinfo_1 & DOT11AX_PHY_CAP_FULL_BW_UL_MU_MIMO)
		peer->cap.he_phy_cap |= HE_FULL_BW_UL_MU_MIMO;
	if (phy_cap->phy_capinfo_1 & DOT11AX_PHY_CAP_PARTIAL_BW_UL_MU_MIMO)
		peer->cap.he_phy_cap |= HE_PARTIAL_BW_UL_MU_MIMO;
	if (phy_cap->phy_capinfo_1 & DOT11AX_PHY_CAP_UL_HE_MU_PPDU)
		peer->cap.he_phy_cap |= HE_RX_MU_PPDU_FROM_STA;
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_PARTIAL_BW_DL_MU_MIMO)
		peer->cap.he_phy_cap |= HE_PARTIAL_BW_DL_MU_MIMO;
	/* dcm */
	peer->cap.dcm_max_constellation_tx =
		GET_DOT11AX_DCM_MAX_CONSTELLATION_TX(phy_cap->phy_capinfo_1);
	if (phy_cap->phy_capinfo_1 & DOT11AX_PHY_CAP_TX_DCM_MAX_NSS)
		peer->cap.he_phy_cap |= HE_DCM_MAX_NSS_TX;
	peer->cap.dcm_max_constellation_rx =
		GET_DOT11AX_DCM_MAX_CONSTELLATION_RX(phy_cap->phy_capinfo_1);
	if (phy_cap->phy_capinfo_1 & DOT11AX_PHY_CAP_RX_DCM_MAX_NSS)
		peer->cap.he_phy_cap |= HE_DCM_MAX_NSS_RX;
	peer->cap.dcm_max_ru = GET_DOT11AX_DCM_MAX_RU(phy_cap->phy_capinfo_3);
	/*bf*/
	if (phy_cap->phy_capinfo_1 & DOT11AX_PHY_CAP_SU_BFER)
		peer->cap.he_bf.bf_cap |= HE_SU_BFER;
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_SU_BFEE)
		peer->cap.he_bf.bf_cap |= HE_SU_BFEE;
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_MU_BFER)
		peer->cap.he_bf.bf_cap |= HE_MU_BFER;
	peer->cap.he_bf.bfee_sts_le_eq_bw80 =
		GET_DOT11AX_BFEE_STS_LE_EQ_80M(phy_cap->phy_capinfo_2);
	peer->cap.he_bf.bfee_sts_gt_bw80 =
		GET_DOT11AX_BFEE_STS_GT_80M(phy_cap->phy_capinfo_2);
	peer->cap.he_bf.snd_dim_le_eq_bw80 =
		GET_DOT11AX_SND_DIM_LE_EQ_80M(phy_cap->phy_capinfo_2);
	peer->cap.he_bf.snd_dim_gt_bw80 =
		GET_DOT11AX_SND_DIM_GT_80M(phy_cap->phy_capinfo_2);
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_NG16_SU_FEEDBACK)
		peer->cap.he_bf.bf_cap |= HE_BFEE_NG_16_SU_FEEDBACK;
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_NG16_MU_FEEDBACK)
		peer->cap.he_bf.bf_cap |= HE_BFEE_NG_16_MU_FEEDBACK;
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_CODEBOOK_SU_FEEDBACK)
		peer->cap.he_bf.bf_cap |= HE_BFEE_CODEBOOK_SU_FEEDBACK;
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_CODEBOOK_MU_FEEDBACK)
		peer->cap.he_bf.bf_cap |= HE_BFEE_CODEBOOK_MU_FEEDBACK;
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_TRIG_SU_BF_FEEDBACK)
		peer->cap.he_bf.bf_cap |= HE_TRIG_SU_BFEE_FEEDBACK;
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_TRIG_MU_BF_PARTIAL_BW_FEEDBACK)
		peer->cap.he_bf.bf_cap |= HE_TRIG_MU_BFEE_FEEDBACK;
	peer->cap.he_bf.bfee_max_nc =
		GET_DOT11AX_BFEE_MAX_NC(phy_cap->phy_capinfo_2);
	/*cqi*/
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_TRIG_CQI_FEEDBACK)
		peer->cap.he_phy_cap |= HE_TRIG_CQI_FEEDBACK;
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_PARTIAL_BW_ER)
		peer->cap.he_phy_cap |= HE_PARTIAL_BW_ER;
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_PPE_THRLD_PRESENT)
		peer->cap.he_phy_cap |= HE_PPE_THRESHOLD_PRESENT;
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_SRP_BASE_SR)
		peer->cap.he_phy_cap |= HE_SRP_BASED_SR;
	if (phy_cap->phy_capinfo_2 & DOT11AX_PHY_CAP_PWR_BOOST_FACTOR)
		peer->cap.he_phy_cap |= HE_PWR_BOOST_FACTOR;
	if (phy_cap->phy_capinfo_3 & DOT11AX_PHY_CAP_20M_IN_40M_HE_PPDU_24G)
		peer->cap.he_phy_cap |= HE_24G_20M_IN_40M_PPDU;
	if (phy_cap->phy_capinfo_3 & DOT11AX_PHY_CAP_20M_IN_160M_8080M_HE_PPDU)
		peer->cap.he_phy_cap |= HE_20M_IN_160M_8080M_PPDU;
	if (phy_cap->phy_capinfo_3 & DOT11AX_PHY_CAP_80M_IN_160M_8080M_HE_PPDU)
		peer->cap.he_phy_cap |= HE_80M_IN_160M_8080M_PPDU;
	if (phy_cap->phy_capinfo_3 & DOT11AX_PHY_CAP_MIDAMBLE_TXRX_2X_1X_HE_LTF)
		peer->cap.he_phy_cap |= HE_MID_RX_2x_AND_1x_LTF;
	/*1024QAM under RU242*/
	if (phy_cap->phy_capinfo_4 & DOT11AX_PHY_CAP_TX_1024QAM_LT_242_TONE_RU)
		peer->cap.he_phy_cap |= HE_TX_1024QAM_UNDER_RU242;
	if (phy_cap->phy_capinfo_4 & DOT11AX_PHY_CAP_RX_1024QAM_LT_242_TONE_RU)
		peer->cap.he_phy_cap |= HE_RX_1024QAM_UNDER_RU242;
}

static inline UINT8 he_support_max_mcs(UINT16 mcs_map, UINT8 nss_m1)
{
	return ((mcs_map >> (nss_m1 << 1)) & DOT11AX_MCS_NSS_MASK);
}

static VOID peer_he_txrx_mcs_nss(struct _MAC_TABLE_ENTRY *peer,
		struct he_txrx_mcs_nss *mcs_nss_80, struct he_txrx_mcs_nss *mcs_nss_160,
		struct he_txrx_mcs_nss *mcs_nss_8080)
{
	UINT8 i;

	/*bw80*/
	for (i = 0; i < DOT11AX_MAX_STREAM; i++) {
		peer->cap.rate.he80_rx_nss_mcs[i] =
			he_support_max_mcs(mcs_nss_80->max_rx_mcs_nss, i);
		peer->cap.rate.he80_tx_nss_mcs[i] =
			he_support_max_mcs(mcs_nss_80->max_tx_mcs_nss, i);
	}
	/*bw160*/
	if (peer->cap.ch_bw.he_ch_width & SUPP_160M_CW_IN_5G_BAND) {
		for (i = 0; i < DOT11AX_MAX_STREAM; i++) {
			peer->cap.rate.he160_rx_nss_mcs[i] =
				he_support_max_mcs(mcs_nss_160->max_rx_mcs_nss, i);
			peer->cap.rate.he160_tx_nss_mcs[i] =
				he_support_max_mcs(mcs_nss_160->max_tx_mcs_nss, i);
		}
	}
	/*bw80+80*/
	if (peer->cap.ch_bw.he_ch_width & SUPP_160M_8080M_CW_IN_5G_BAND) {
		for (i = 0; i < DOT11AX_MAX_STREAM; i++) {
			peer->cap.rate.he8080_rx_nss_mcs[i] =
				he_support_max_mcs(mcs_nss_8080->max_rx_mcs_nss, i);
			peer->cap.rate.he8080_tx_nss_mcs[i] =
				he_support_max_mcs(mcs_nss_8080->max_tx_mcs_nss, i);
		}
	}
}

static VOID peer_he_6g_caps(struct _MAC_TABLE_ENTRY *peer, struct he_6g_cap_ie *he6g_caps)
{
	peer->cap.ampdu.he6g_min_mpdu_start_spacing =
		DOT11AX_6G_GET_MAX_MPDU_START_SPACE(he6g_caps->caps_info);
	peer->cap.ampdu.he6g_max_mpdu_len =
		DOT11AX_6G_GET_MPDU_LEN(he6g_caps->caps_info);
	peer->cap.ampdu.he6g_max_ampdu_len_exp =
		DOT11AX_6G_GET_MAX_AMPDU_LEN_EXP(he6g_caps->caps_info);
	peer->cap.he6g_smps = DOT11AX_6G_GET_SMPS(he6g_caps->caps_info);

	/*not parsing rd_resp, tx_ant_pattern_consist, rx_ant_pattern_consist yet*/
	return;
}


static VOID peer_he_ppe_threshold(struct _MAC_TABLE_ENTRY *peer, UINT8 *ie_pos)
{
	return;
}

static VOID peer_he_basic_mcs_nss(struct _MAC_TABLE_ENTRY *peer, UINT16 he_basic_mcs_nss)
{
	return;
}

static VOID peer_bss_color_info(struct _MAC_TABLE_ENTRY *peer, UINT8 bss_color_info)
{
	/*bss color*/
	peer->cap.bss_color_info.bss_color =
		(bss_color_info & DOT11AX_BSS_COLOR_MASK);
	peer->cap.bss_color_info.partial_bss_color = 0;
	if (bss_color_info & DOT11AX_PARTIAL_BSS_COLOR)
		peer->cap.bss_color_info.partial_bss_color = 1;
	peer->cap.bss_color_info.bss_color_dis = 0;
	if (bss_color_info & DOT11AX_BSS_COLOR_DISABLE)
		peer->cap.bss_color_info.bss_color_dis = 1;
	/* save setting to bss color contrl block */
	set_bss_color_info(peer->wdev, peer->cap.bss_color_info.bss_color_dis,
		peer->cap.bss_color_info.bss_color);

	return;
};

static VOID peer_he_op_params(struct _MAC_TABLE_ENTRY *peer,
		struct he_op_params *he_op_param)
{
	/*default PE duration*/
	peer->cap.default_pe_dur =
		(he_op_param->param1 & DOT11AX_OP_DEFAULT_PE_DURATION_MASK);
	/*twt required*/
	/*txop duration RTS threshold*/
	peer->cap.txop_dur_rts_thld =
		(he_op_param->param1 & DOT11AX_OP_TXOP_DUR_RTS_THLD_MASK) >> DOT11AX_OP_TXOP_DUR_RTS_THLD_SHIFT;
	/*vhtop_info present*/
	HE_CLR_MAC_CAPS(peer->cap.he_mac_cap, HE_VHT_OPINFO_PRESENT);
	if (he_op_param->param1 & DOT11AX_OP_VHT_OPINFO_PRESENT)
		HE_SET_MAC_CAPS(peer->cap.he_mac_cap, HE_VHT_OPINFO_PRESENT);
	/*co-hosted BSS*/
	HE_CLR_MAC_CAPS(peer->cap.he_mac_cap, HE_CO_HOSTED_BSS);
	if (he_op_param->param1 & DOT11AX_OP_CO_HOSTED_BSS)
		HE_SET_MAC_CAPS(peer->cap.he_mac_cap, HE_CO_HOSTED_BSS);
	/*er su disable*/
	HE_CLR_MAC_CAPS(peer->cap.he_mac_cap, HE_ER_SU_DISABLE);
	if (he_op_param->param2 & DOT11AX_OP_ER_SU_DISABLE)
		HE_SET_MAC_CAPS(peer->cap.he_mac_cap, HE_ER_SU_DISABLE);
	/*he 6g operation info present*/
	if (he_op_param->param2 & DOT11AX_OP_6G_OPINFO_PRESENT)
		peer->cap.he6g_op_present = 1;
}

static VOID peer_he_6g_opinfo(struct _MAC_TABLE_ENTRY *peer,
		struct he_6g_op_info *he6g_opinfo)
{
	peer->cap.ch_bw.he6g_ch_width = (he6g_opinfo->ctrl & HE_6G_OP_CONTROL_CH_WIDTH_MASK);
	peer->cap.ch_bw.he6g_prim_ch = he6g_opinfo->prim_ch;
	peer->cap.ch_bw.he6g_ccfs_0 = he6g_opinfo->ccfs_0;
	peer->cap.ch_bw.he6g_ccfs_1 = he6g_opinfo->ccfs_1;
	peer->cap.min_rate = he6g_opinfo->min_rate;
	peer->cap.he6g_dup_bcn = HE_6G_OP_CTRL_GET_DUP_BCN(he6g_opinfo->ctrl);
}

static UINT8 *peer_he_vht_op_info(struct _MAC_TABLE_ENTRY *peer, UINT8 *ie_pos)
{
	UINT8 *next_ie_pos = ie_pos;

	next_ie_pos += sizeof(struct vht_opinfo);
	return next_ie_pos;
}

static UINT16 he_max_mpdu_size[3] = {3839u, 7935u, 11454u};

static VOID update_peer_he_6g_caps(struct _MAC_TABLE_ENTRY *peer, struct common_ies *cmm_ies)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)peer->pAd;
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct caps_info *cap = &peer->cap;
	UINT8 cfg_max_mpdu_len = wlan_config_get_vht_max_mpdu_len(peer->wdev);

	/*he6G caps*/
	peer_he_6g_caps(peer, &cmm_ies->he6g_caps);
	/*update 6g smps*/
	peer->MmpsMode = cap->he6g_smps;
	/*update 6g ampdu/amsdu cap*/
	peer->MaxRAmpduFactor = (cap->ampdu.he6g_max_ampdu_len_exp > peer->MaxRAmpduFactor) ?
	cap->ampdu.he6g_max_ampdu_len_exp : peer->MaxRAmpduFactor;

	if (cfg_max_mpdu_len > chip_cap->ppdu.he6g_max_mpdu_len)
		cfg_max_mpdu_len = chip_cap->ppdu.he6g_max_mpdu_len;
	peer->AMsduSize = cap->ampdu.he6g_max_mpdu_len;
	if (cap->ampdu.he6g_max_mpdu_len > cfg_max_mpdu_len)
		peer->AMsduSize = cfg_max_mpdu_len;

	if (peer->AMsduSize < (sizeof(he_max_mpdu_size) / sizeof(he_max_mpdu_size[0])))
		peer->amsdu_limit_len = he_max_mpdu_size[peer->AMsduSize];
	else
		peer->amsdu_limit_len = 0;

	peer->amsdu_limit_len_adjust = peer->amsdu_limit_len;
}

VOID update_peer_he_caps(struct _MAC_TABLE_ENTRY *peer, struct common_ies *cmm_ies)
{
	CLIENT_STATUS_SET_FLAG(peer, fCLIENT_STATUS_HE_CAPABLE);
	/*mac caps*/
	peer_he_mac_caps(peer, &cmm_ies->he_caps.mac_cap);
	/*phy caps*/
	peer_he_phy_caps(peer, &cmm_ies->he_caps.phy_cap);
	/*tx rx he-mcs nss*/
	peer_he_txrx_mcs_nss(peer, &cmm_ies->he_caps.txrx_mcs_nss,
			&cmm_ies->mcs_nss_160, &cmm_ies->mcs_nss_8080);
	/*ppe threshold*/
	if (peer->cap.he_phy_cap & HE_PPE_THRESHOLD_PRESENT)
		peer_he_ppe_threshold(peer, NULL);

	if (HAS_HE_6G_CAP_EXIST(cmm_ies->ie_exists)) {
		update_peer_he_6g_caps(peer, cmm_ies);
	}
}

VOID update_peer_he_operation(struct _MAC_TABLE_ENTRY *peer, struct common_ies *cmm_ies)
{
	/*he op param.*/
	peer_he_op_params(peer, &cmm_ies->he_ops.he_op_param);
	/*bss_color_info*/
	peer_bss_color_info(peer, cmm_ies->he_ops.bss_color_info);
	/*basic he-mcs and nss set*/
	peer_he_basic_mcs_nss(peer, cmm_ies->he_ops.he_basic_mcs_nss);
	/*vht operation info*/
	if (HE_CHK_MAC_CAPS(peer->cap.he_mac_cap, HE_VHT_OPINFO_PRESENT))
		peer_he_vht_op_info(peer, NULL);

	/*he 6G op param.*/
	if (peer->cap.he6g_op_present)
		peer_he_6g_opinfo(peer, &cmm_ies->he6g_opinfo);
}

static UINT32 parse_he_caps(UINT8 *ie_ctx, UINT8 ie_len, struct common_ies *cmm_ies)
{
	UINT8 ch_width = HE_BW_80;
	UINT32 partial_ie_len = 0;
	UINT8 *ie_ptr = ie_ctx;

	if (ie_len < sizeof(cmm_ies->he_caps))
		return partial_ie_len;
	SET_HE_CAPS_EXIST(cmm_ies->ie_exists);
	partial_ie_len = sizeof(cmm_ies->he_caps);
	NdisMoveMemory((UINT8 *)&cmm_ies->he_caps, ie_ptr, partial_ie_len);
#ifdef RT_BIG_ENDIAN
	cmm_ies->he_caps.phy_cap.phy_capinfo_1
		= le2cpu32(cmm_ies->he_caps.phy_cap.phy_capinfo_1);
	cmm_ies->he_caps.phy_cap.phy_capinfo_2
		= le2cpu32(cmm_ies->he_caps.phy_cap.phy_capinfo_2);
	cmm_ies->he_caps.mac_cap.mac_capinfo_1
		= le2cpu32(cmm_ies->he_caps.mac_cap.mac_capinfo_1);
	cmm_ies->he_caps.mac_cap.mac_capinfo_2
		= le2cpu16(cmm_ies->he_caps.mac_cap.mac_capinfo_2);
	cmm_ies->he_caps.txrx_mcs_nss.max_rx_mcs_nss
		= le2cpu16(cmm_ies->he_caps.txrx_mcs_nss.max_rx_mcs_nss);
	cmm_ies->he_caps.txrx_mcs_nss.max_tx_mcs_nss
		= le2cpu16(cmm_ies->he_caps.txrx_mcs_nss.max_tx_mcs_nss);
#endif
	ie_ptr += partial_ie_len;
	ch_width = peer_max_bw_cap(GET_DOT11AX_CH_WIDTH(cmm_ies->he_caps.phy_cap.phy_capinfo_1));
	if (ch_width == HE_BW_160) {
		partial_ie_len = sizeof(cmm_ies->mcs_nss_160);
		NdisMoveMemory((UINT8 *)&cmm_ies->mcs_nss_160, ie_ptr, partial_ie_len);
#ifdef RT_BIG_ENDIAN
		cmm_ies->mcs_nss_160.max_rx_mcs_nss
			= le2cpu16(cmm_ies->mcs_nss_160.max_rx_mcs_nss);
		cmm_ies->mcs_nss_160.max_tx_mcs_nss
			= le2cpu16(cmm_ies->mcs_nss_160.max_tx_mcs_nss);
#endif
		ie_ptr += partial_ie_len;
	}
	if (ch_width == HE_BW_8080) {
		partial_ie_len = sizeof(cmm_ies->mcs_nss_8080);
		NdisMoveMemory((UINT8 *)&cmm_ies->mcs_nss_8080, ie_ptr, partial_ie_len);
#ifdef RT_BIG_ENDIAN
		cmm_ies->mcs_nss_8080.max_rx_mcs_nss
			= le2cpu16(cmm_ies->mcs_nss_8080.max_rx_mcs_nss);
		cmm_ies->mcs_nss_8080.max_tx_mcs_nss
			= le2cpu16(cmm_ies->mcs_nss_8080.max_tx_mcs_nss);
#endif
		ie_ptr += partial_ie_len;
	}
	return ie_len;
}

static UINT32 parse_he_6g_caps(UINT8 *ie_ctx, UINT8 ie_len, struct common_ies *cmm_ies)
{
	UINT32 partial_ie_len = 0;
	UINT8 *ie_ptr = ie_ctx;

	if (ie_len < sizeof(cmm_ies->he6g_caps))
		return partial_ie_len;
	partial_ie_len = sizeof(cmm_ies->he6g_caps);
	NdisMoveMemory((UINT8 *)&cmm_ies->he6g_caps, ie_ptr, partial_ie_len);
	ie_ptr += partial_ie_len;

	return ie_len;
}

static UINT32 parse_he_operation(UINT8 *ie_ctx, UINT8 ie_len, struct common_ies *cmm_ies)
{
	UINT32 partial_ie_len = 0;
	UINT8 *ie_ptr = ie_ctx;
	UCHAR full_ie_len = 0;

	if (ie_len < sizeof(cmm_ies->he_ops))
		return partial_ie_len;
	SET_HE_OP_EXIST(cmm_ies->ie_exists);
	partial_ie_len = sizeof(cmm_ies->he_ops);
	NdisMoveMemory((UINT8 *)&cmm_ies->he_ops, ie_ptr, partial_ie_len);
#ifdef RT_BIG_ENDIAN
	cmm_ies->he_ops.he_basic_mcs_nss
		= le2cpu16(cmm_ies->he_ops.he_basic_mcs_nss);
	cmm_ies->he_ops.he_op_param.param1
		= le2cpu16(cmm_ies->he_ops.he_op_param.param1);
#endif
	ie_ptr += partial_ie_len;

	/*vht_opinfo present*/
	if (cmm_ies->he_ops.he_op_param.param1 & DOT11AX_OP_VHT_OPINFO_PRESENT) {
		partial_ie_len = sizeof(cmm_ies->he_vht_opinfo);
		NdisMoveMemory((UINT8 *)&cmm_ies->he_vht_opinfo, ie_ptr, partial_ie_len);
		ie_ptr += partial_ie_len;
	}
	/*max co-hosted bssid*/
	if (cmm_ies->he_ops.he_op_param.param1 & DOT11AX_OP_CO_HOSTED_BSS) {
		partial_ie_len = sizeof(cmm_ies->he_max_co_hosted_bssid_ind);
		NdisMoveMemory((UINT8 *)&cmm_ies->he_max_co_hosted_bssid_ind, ie_ptr, partial_ie_len);
		ie_ptr += partial_ie_len;
	}
	/*he 6g opinfo present*/
	if (cmm_ies->he_ops.he_op_param.param2 & DOT11AX_OP_6G_OPINFO_PRESENT) {
		partial_ie_len = sizeof(cmm_ies->he6g_opinfo);
		NdisMoveMemory((UINT8 *)&cmm_ies->he6g_opinfo, ie_ptr, partial_ie_len);
		ie_ptr += partial_ie_len;
	}
	full_ie_len = ie_ptr - ie_ctx;
	if (full_ie_len == ie_len)
		return ie_len;
	else
		return 0;
	return ie_len;
}

static UINT32 parse_he_muedca_ies(UINT8 *ie_ctx, UINT8 ie_len, struct common_ies *cmm_ies)
{
	UINT32 partial_ie_len = 0;
	UINT8 *ie_ptr = ie_ctx;

	NdisZeroMemory((UINT8 *)&cmm_ies->he_mu_edca, sizeof(struct mu_edca_params));

	if (!parse_mu_edca_ie(ie_len))
		return partial_ie_len;
	SET_HE_MU_EDCA_EXIST(cmm_ies->ie_exists);
	partial_ie_len = sizeof(cmm_ies->he_mu_edca);
	NdisMoveMemory((UINT8 *)&cmm_ies->he_mu_edca, ie_ptr, ie_len);
	ie_ptr += partial_ie_len;

	return ie_len;
}

static UINT32 parse_he_sr_ies(UINT8 *ie_ctx, UINT8 ie_len, struct common_ies *cmm_ies)
{
	UINT32 partial_ie_len = 0;
	UINT8 *ie_ptr = ie_ctx;

	NdisZeroMemory((UINT8 *)&cmm_ies->he_sr_ies, sizeof(struct he_sr_ie));

	if (!parse_spatial_resue_ie(ie_ptr, ie_len))
		return partial_ie_len;

	NdisMoveMemory((UINT8 *)&cmm_ies->he_sr_ies, ie_ptr, ie_len);

	return ie_len;
}

BOOL CompareMUEdcaParameters(
	struct mu_edca_params *prIeMUEdcaParam,
	struct _MAC_TABLE_ENTRY *peer)
{
	struct he_mu_edca_params *prBSSMUEdca;
	struct mu_ac_param_record *prMUAcParamInIE;
	enum aci_ac eAci;

	/* Check Set Count */
	/*
	 * The QoS Info field contains the EDCA Parameter Set Update Count subfield,
	 * which is initially set to 0 and is incremented each time
	 * any of the MU AC parameters in the MU EDCA Parameter Set element changes.
	*/
	if (peer->ucMUEdcaUpdateCnt &&
		(peer->ucMUEdcaUpdateCnt !=
		(prIeMUEdcaParam->qos_info & WMM_QOS_INFO_PARAM_SET_CNT)))
		return FALSE;

	for (eAci = 0; eAci < ACI_AC_NUM; eAci++) {
		prBSSMUEdca = &peer->arMUEdcaParams[eAci];
		prMUAcParamInIE = &prIeMUEdcaParam->ac_rec[eAci];

		/* ACM */
		if (prBSSMUEdca->ucIsACMSet != ((prMUAcParamInIE->aci_aifsn &
			WMM_ACIAIFSN_ACM) ? TRUE : FALSE))
			return FALSE;

		/* AIFSN */
		if (prBSSMUEdca->ucAifsn != (prMUAcParamInIE->aci_aifsn &
			WMM_ACIAIFSN_AIFSN))
			return FALSE;

		/* CW Max */
		if (prBSSMUEdca->ucECWmax != prMUAcParamInIE->ec_wmax_wmin)
			return FALSE;

		/* CW Min */
		if (prBSSMUEdca->ucECWmin != prMUAcParamInIE->ec_wmax_wmin)
			return FALSE;

		/* MU EDCA timer */
		if (prBSSMUEdca->ucMUEdcaTimer !=
			prMUAcParamInIE->mu_edca_timer)
			return FALSE;
	}

	return TRUE;
}
BOOL update_peer_he_muedca_ies(struct _MAC_TABLE_ENTRY *peer,
					struct common_ies *cmm_ies)
{
	BOOL fgIsNew = FALSE;
	struct mu_edca_params *he_muedca_ies = (struct mu_edca_params *)&cmm_ies->he_mu_edca;
	enum aci_ac eAci;
	struct he_mu_edca_params *prBSSMUEdca;
	struct mu_ac_param_record *prMUAcParamInIE;

	UCHAR tmp[24];

	NdisCopyMemory(tmp, he_muedca_ies, sizeof(struct mu_edca_params));

	do {
		if (CompareMUEdcaParameters(he_muedca_ies,
			peer)) {
			fgIsNew = FALSE;
			break;
		}


		fgIsNew = TRUE;

		peer->ucMUEdcaUpdateCnt = (he_muedca_ies->qos_info &
			WMM_QOS_INFO_PARAM_SET_CNT);

		/* Update MU EDCA parameters to mac entry structure */

		for (eAci = 0; eAci < ACI_AC_NUM; eAci++) {
			prMUAcParamInIE = &(he_muedca_ies->ac_rec[eAci]);
			prBSSMUEdca = &peer->arMUEdcaParams[eAci];

			prBSSMUEdca->ucECWmin = prMUAcParamInIE->ec_wmax_wmin &
				WMM_ECW_WMIN_MASK;
			prBSSMUEdca->ucECWmax = (prMUAcParamInIE->ec_wmax_wmin &
				WMM_ECW_WMAX_MASK) >> WMM_ECW_WMAX_OFFSET;
			prBSSMUEdca->ucAifsn = (prMUAcParamInIE->aci_aifsn &
				WMM_ACIAIFSN_AIFSN);
			prBSSMUEdca->ucIsACMSet = (prMUAcParamInIE->aci_aifsn &
				WMM_ACIAIFSN_ACM) ? TRUE : FALSE;
			prBSSMUEdca->ucMUEdcaTimer =
				prMUAcParamInIE->mu_edca_timer;

		}
	} while (FALSE);

	return	fgIsNew;
}

BOOL update_peer_he_sr_ies(struct _MAC_TABLE_ENTRY *peer,
					struct common_ies *cmm_ies)
{
	BOOL fgIsNew = FALSE;
	struct he_sr_ie *he_sr_ies = (struct he_sr_ie *)&cmm_ies->he_sr_ies;
	struct srg_sr_info *srg_sr_info_t = (struct srg_sr_info *)&he_sr_ies->srg_sr_info;


	if (peer->SRControl != he_sr_ies->sr_control) {
		fgIsNew = TRUE;
		peer->SRControl = he_sr_ies->sr_control;
	}

	if (peer->SRControl & SR_PARAM_NON_SRG_OFFSET_PRESENT) {
		if (peer->NonSRGObssPdMaxOffset !=
			he_sr_ies->non_srg_obss_pd_max_offset) {

			fgIsNew = TRUE;
			peer->NonSRGObssPdMaxOffset =
				he_sr_ies->non_srg_obss_pd_max_offset;
		}
	} else
		peer->NonSRGObssPdMaxOffset = 0;


	if (peer->SRControl & SR_PARAM_SRG_INFO_PRESENT) {

		if (peer->SRGObssPdMinOffset !=
			srg_sr_info_t->ucObssPdMinOffset) {
			fgIsNew = TRUE;
			peer->SRGObssPdMinOffset =
				srg_sr_info_t->ucObssPdMinOffset;
		}
		if (peer->SRGObssPdMaxOffset !=
			srg_sr_info_t->ucObssPdMaxOffset) {
			fgIsNew = TRUE;
			peer->SRGObssPdMaxOffset =
				srg_sr_info_t->ucObssPdMaxOffset;
		}
		if (peer->SRGBSSColorBitmap !=
			le2cpu64(srg_sr_info_t->u8BSSColorBitmap)) {
			fgIsNew = TRUE;
			peer->SRGBSSColorBitmap =
				le2cpu64(srg_sr_info_t->u8BSSColorBitmap);
		}
		if (peer->SRGPartialBSSIDBitmap !=
			le2cpu64(srg_sr_info_t->u8PartialBSSIDBitmap)) {
			fgIsNew = TRUE;
			peer->SRGPartialBSSIDBitmap =
				le2cpu64(srg_sr_info_t->u8PartialBSSIDBitmap);
		}
	} else {
		peer->SRGObssPdMinOffset = 0;
		peer->SRGObssPdMaxOffset = 0;
		peer->SRGBSSColorBitmap = 0;
		peer->SRGPartialBSSIDBitmap = 0;
	}

	return	fgIsNew;
}

/* AP/STA share parsing */
UINT32 parse_he_beacon_probe_rsp_ies(UINT8 *ie_head, VOID *ie_list)
{
	UINT8 eid_ext = ((struct _EID_STRUCT *)ie_head)->Octet[0];
	UINT8 ie_len = ((struct _EID_STRUCT *)ie_head)->Len - 1;
	UINT8 *ie_ctx = ie_head + sizeof(struct _EID_STRUCT);
	struct _bcn_ie_list *bcn_ie = (struct _bcn_ie_list *)ie_list;
	UINT32 offset = 0;

	switch (eid_ext) {
	case EID_EXT_HE_CAPS:
		offset = parse_he_caps(ie_ctx, ie_len, &bcn_ie->cmm_ies);
		/*dump_he_ies("Recv. peer bcn, he_cap", ie_head, ((struct _EID_STRUCT *)ie_head)->Len);*/
		break;
	case EID_EXT_HE_OP:
		offset = parse_he_operation(ie_ctx, ie_len, &bcn_ie->cmm_ies);
		/*dump_he_ies("Recv. peer bcn, he_op", ie_head, ((struct _EID_STRUCT *)ie_head)->Len);*/
		break;
	case EID_EXT_UORA_PARAM_SET:
		break;
	case EID_EXT_MU_EDCA_PARAM:
		offset = parse_he_muedca_ies(ie_ctx, ie_len, &bcn_ie->cmm_ies);
		break;
	case EID_EXT_SR_PARAM_SET:
		offset = parse_he_sr_ies(ie_ctx, ie_len, &bcn_ie->cmm_ies);
		break;
	case EID_EXT_NDP_FB_REPORT:
		break;
	case EID_EXT_BSS_COLOR_CHANGE_ANNOUNCE:
		break;
	case EID_EXT_QUIET_TIME_PERIOD:
		break;
	default:
		/*not a HE IEs*/
		break;
	}
	return offset;
}

UINT32 parse_he_assoc_rsp_ies(UINT8 *ie_head, VOID *ie_list)
{
	UINT8 eid_ext = ((struct _EID_STRUCT *)ie_head)->Octet[0];
	UINT8 ie_len = ((struct _EID_STRUCT *)ie_head)->Len - 1;
	UINT8 *ie_ctx = ie_head + sizeof(struct _EID_STRUCT);
	struct _IE_lists *ie = (struct _IE_lists *)ie_list;
	UINT32 offset = 0;

	switch (eid_ext) {
	case EID_EXT_HE_CAPS:
		offset = parse_he_caps(ie_ctx, ie_len, &ie->cmm_ies);
		/*dump_he_ies("Recv. peer assoc_rsp, he_cap", ie_head, ((struct _EID_STRUCT *)ie_head)->Len);*/
		break;
	case EID_EXT_HE_OP:
		offset = parse_he_operation(ie_ctx, ie_len, &ie->cmm_ies);
		/*dump_he_ies("Recv. peer assoc_rsp, he_op", ie_head, ((struct _EID_STRUCT *)ie_head)->Len);*/
		break;
	case EID_EXT_HE_6G_CAPS:
		offset = parse_he_6g_caps(ie_ctx, ie_len, &ie->cmm_ies);
		SET_HE_6G_CAP_EXIST(ie->cmm_ies.ie_exists);
		/*dump_he_ies("Recv. peer assoc_rsp, he_6g_cap", ie_head, ((struct _EID_STRUCT *)ie_head)->Len);*/
		break;
	case EID_EXT_UORA_PARAM_SET:
		break;
	case EID_EXT_MU_EDCA_PARAM:
		offset = parse_he_muedca_ies(ie_ctx, ie_len,  &ie->cmm_ies);
		/*dump_he_ies("Recv. peer assoc_rsp, mu_edca", ie_head, ((struct _EID_STRUCT *)ie_head)->Len);*/
		break;
	case EID_EXT_SR_PARAM_SET:
		offset = parse_he_sr_ies(ie_ctx, ie_len, &ie->cmm_ies);
		break;
	case EID_EXT_NDP_FB_REPORT:
		break;
	case EID_EXT_BSS_COLOR_CHANGE_ANNOUNCE:
		break;
	case EID_EXT_QUIET_TIME_PERIOD:
		break;
	default:
		/*not a HE IEs*/
		break;
	}
	return offset;
}

/* AP parsing */
UINT32 parse_he_probe_req_ies(UINT8 *ie_head, VOID *ie_list)
{
	UINT8 eid_ext = ((struct _EID_STRUCT *)ie_head)->Octet[0];
	UINT8 ie_len = ((struct _EID_STRUCT *)ie_head)->Len - 1;
	UINT8 *ie_ctx = ie_head + sizeof(struct _EID_STRUCT);
	struct _IE_lists *ie = (struct _IE_lists *)ie_list;
	UINT32 offset = 0;

	switch (eid_ext) {
	case EID_EXT_HE_CAPS:
		offset = parse_he_caps(ie_ctx, ie_len, &ie->cmm_ies);
		/*dump_he_ies("Recv. peer probe_req, he_cap", ie_head, ((struct _EID_STRUCT *)ie_head)->Len);*/
		break;
	case EID_EXT_HE_6G_CAPS:
		offset = parse_he_6g_caps(ie_ctx, ie_len, &ie->cmm_ies);
		break;
	default:
		/*not a HE IEs*/
		break;
	}
	return offset;
}

UINT32 parse_he_assoc_req_ies(UINT8 *ie_head, VOID *ie_list)
{
	UINT8 eid_ext = ((struct _EID_STRUCT *)ie_head)->Octet[0];
	UINT8 ie_len = ((struct _EID_STRUCT *)ie_head)->Len - 1;
	UINT8 *ie_ctx = ie_head + sizeof(struct _EID_STRUCT);
	struct _IE_lists *ie = (struct _IE_lists *)ie_list;
	UINT32 offset = 0;

	switch (eid_ext) {
	case EID_EXT_HE_CAPS:
		offset = parse_he_caps(ie_ctx, ie_len, &ie->cmm_ies);
		SET_HE_CAPS_EXIST(ie->cmm_ies.ie_exists);
		/*dump_he_ies("Recv. peer assoc_req, he_cap", ie_head, ((struct _EID_STRUCT *)ie_head)->Len);*/
		break;
	case EID_EXT_HE_OP:
		offset = parse_he_operation(ie_ctx, ie_len, &ie->cmm_ies);
		break;
	case EID_EXT_HE_6G_CAPS:
		offset = parse_he_6g_caps(ie_ctx, ie_len, &ie->cmm_ies);
		SET_HE_6G_CAP_EXIST(ie->cmm_ies.ie_exists);
		/*dump_he_ies("Recv. peer assoc_req, he_6g_cap", ie_head, ((struct _EID_STRUCT *)ie_head)->Len);*/
		break;
	default:
		/*not a HE IEs*/
		break;
	}
	return offset;
}

UCHAR he_bw_2_rf_bw(UCHAR he_bw)
{
	UCHAR rf_bw = BW_20;

	if ((he_bw == HE_BW_160) || (he_bw == HE_BW_8080))
		rf_bw = BW_160;
	else if (he_bw == HE_BW_80)
		rf_bw = BW_80;
	else if (he_bw == HE_BW_2040)
		rf_bw = BW_40;
	else
		rf_bw = BW_20;

	return rf_bw;
}

VOID he_mode_adjust(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *peer, UCHAR *bw_from_opclass)
{
	UCHAR PeerMaxBw;

	if (!WMODE_CAP_AX(wdev->PhyMode))
		return;

	peer->cap.modes &= ~(HE_24G_SUPPORT | HE_5G_SUPPORT | HE_6G_SUPPORT);
	if (WMODE_CAP_AX_2G(wdev->PhyMode))
		peer->cap.modes |= HE_24G_SUPPORT;
	if (WMODE_CAP_AX_5G(wdev->PhyMode))
		peer->cap.modes |= HE_5G_SUPPORT;
	if (WMODE_CAP_AX_6G(wdev->PhyMode))
		peer->cap.modes |= HE_6G_SUPPORT;
	peer->MaxHTPhyMode.field.MODE = MODE_HE;

	PeerMaxBw = BW_20;
	if (WMODE_CAP_AX_2G(wdev->PhyMode)) {/* 2G */
		if (peer->cap.ch_bw.he_ch_width & SUPP_40M_CW_IN_24G_BAND)
			PeerMaxBw = BW_40;
	} else {/* 5G */
		if (peer->cap.ch_bw.he_ch_width & SUPP_40M_80M_CW_IN_5G_BAND) {
			PeerMaxBw = BW_80;
			if (peer->cap.ch_bw.he_ch_width & SUPP_160M_CW_IN_5G_BAND)
				PeerMaxBw = BW_160;
			if (peer->cap.ch_bw.he_ch_width & SUPP_160M_8080M_CW_IN_5G_BAND)
				PeerMaxBw = BW_160;
			if ((PeerMaxBw == BW_80) && (bw_from_opclass != NULL) && ((*bw_from_opclass) < BW_80))
				PeerMaxBw = BW_40;
		}
	}

	if (peer->cap.he6g_op_present) { /* 6G */
		PeerMaxBw = peer->cap.ch_bw.he6g_ch_width;
	}

	PeerMaxBw = min(PeerMaxBw, he_bw_2_rf_bw(wlan_operate_get_he_bw(wdev)));
	peer->MaxHTPhyMode.field.BW = min(PeerMaxBw, wlan_operate_get_he_bw(wdev));
	/* check bw should below or equal the cap */
	peer->cap.ch_bw.he6g_ch_width = peer->MaxHTPhyMode.field.BW;

	MTWF_DBG(NULL, DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%s(), MODE = %d, BW = %d, SGI = %d, STBC = %d\n", __func__,
			 peer->MaxHTPhyMode.field.MODE,
			 peer->MaxHTPhyMode.field.BW,
			 peer->MaxHTPhyMode.field.ShortGI,
			 peer->MaxHTPhyMode.field.STBC);

}

/* debugging */
static UINT32 dump_hex_content(UINT8 *buf, UINT32 len)
{
	UINT32 i;

	for (i = 0; i < len; i++) {
		if ((i % 16) == 0)
			MTWF_PRINT("0x%04x| ", i);
		MTWF_PRINT(" %02x", *(buf + i));
		if ((i % 16) == 15)
			MTWF_PRINT("\n");
	}
	MTWF_PRINT("\n");
	return len;
}

VOID dump_he_ies(UCHAR *str, UINT8 *buf, UINT32 buf_len)
{
	struct _EID_STRUCT *eid_head = (struct _EID_STRUCT *)buf;
	UINT32 remain_len = buf_len;
	UINT32 offset = 0;

	MTWF_PRINT("\n%s:\n", str);
	while (remain_len) {
		if (eid_head->Eid != EID_EXTENSION)
			break;
		MTWF_PRINT("[EID:%d][Len:%d][EID_EXT:%d] ",
				 eid_head->Eid, eid_head->Len, eid_head->Octet[0]);
		switch (eid_head->Octet[0]) {
		case EID_EXT_HE_CAPS:
			MTWF_PRINT("HE_CAPS\n");
			break;
		case EID_EXT_HE_OP:
			MTWF_PRINT("HE_OP\n");
			break;
		case EID_EXT_UORA_PARAM_SET:
			MTWF_PRINT("UORA_PARAM_SET\n");
			break;
		case EID_EXT_MU_EDCA_PARAM:
			MTWF_PRINT("MU_EDCA_PARAM\n");
			break;
		case EID_EXT_SR_PARAM_SET:
			MTWF_PRINT("SR_PARAM_SET\n");
			break;
		case EID_EXT_NDP_FB_REPORT:
			MTWF_PRINT("NDP_FB_REPORT\n");
			break;
		case EID_EXT_BSS_COLOR_CHANGE_ANNOUNCE:
			MTWF_PRINT("BSS_COLOR_CHANGE_ANNOUNCE\n");
			break;
		case EID_EXT_QUIET_TIME_PERIOD:
			MTWF_PRINT("QUIET_TIME_PERIOD\n");
			break;
		case EID_EXT_SHORT_SSID_LIST:
			MTWF_PRINT("HE_SHORT_SSID_LIST\n");
			break;
		case EID_EXT_HE_6G_CAPS:
			MTWF_PRINT("HE_6G_CAPS\n");
			break;
		default:
			MTWF_PRINT("EID_EXT_NOT_RECOGNIZED\n");
			break;
		}
		offset = dump_hex_content(buf + sizeof(struct _EID_STRUCT),
				(eid_head->Len - 1));
		eid_head = (struct _EID_STRUCT *)(buf + sizeof(struct _EID_STRUCT) + offset);
		if (offset > remain_len)
			remain_len = 0;
		else
			remain_len -= offset;
	}
}

/*sta rec he features decision*/
UINT32 starec_he_feature_decision(struct wifi_dev *wdev,
		struct _MAC_TABLE_ENTRY *entry, UINT32 *feature)
{
	UINT32 features = 0;

	if (IS_HE_STA(entry->cap.modes))
		features |= STA_REC_BASIC_HE_INFO_FEATURE;

	if (IS_HE_6G_STA(entry->cap.modes))
		features |= STA_REC_BASIC_VHT_INFO_FEATURE;

	/* Client shall follow mu edca setting from AP. */
	if (wdev->wdev_type == WDEV_TYPE_STA)
		features |= STA_REC_MUEDCA_FEATURE;

	/*return value, must use or operation*/
	*feature |= features;
	return TRUE;
}

VOID fill_starec_he(struct wifi_dev *wdev,
		struct _MAC_TABLE_ENTRY *peer, struct _STA_REC_CTRL_T *sta_rec)
{
	struct ppdu_caps *ppdu_cap = (struct ppdu_caps *)wlan_config_get_ppdu_caps(wdev);
	enum ASIC_CAP asic_caps = wlan_config_get_asic_caps(wdev);
	enum PHY_CAP phy_caps = wlan_config_get_phy_caps(wdev);
	struct he_mcs_info *mcs;
	UINT8 he_af = 0;
	UINT8 he_ldpc = 0;
	UINT8 he_tx_stbc = 0, he_rx_stbc = 0;

	NdisZeroMemory(&sta_rec->he_sta, sizeof(sta_rec->he_sta));
	/*mac cap*/
	he_af = peer->cap.ampdu.max_he_ampdu_len_exp;
	if (peer->cap.ampdu.max_he_ampdu_len_exp > ppdu_cap->he_max_ampdu_len_exp)
		he_af = ppdu_cap->he_max_ampdu_len_exp;
	sta_rec->he_sta.mac_info.max_ampdu_len_exp = he_af;
	if ((peer->cap.he_mac_cap & HE_AMSDU_IN_ACK_EN_AMPDU) && ppdu_cap->tx_amsdu_support)
		sta_rec->he_sta.mac_info.amsdu_in_ampdu_support = 1;
	if (peer->cap.he_mac_cap & HE_HTC)
		sta_rec->he_sta.mac_info.htc_support = 1;
	if (peer->cap.he_mac_cap & HE_BQR)
		sta_rec->he_sta.mac_info.bqr_support = 1;
	if (peer->cap.he_mac_cap & HE_BSR)
		sta_rec->he_sta.mac_info.bsr_support = 1;
	if (peer->cap.he_mac_cap & HE_OM_CTRL)
		sta_rec->he_sta.mac_info.om_support = 1;
	sta_rec->he_sta.mac_info.trigger_frame_mac_pad_dur = peer->cap.tf_mac_pad_duration;
	if ((peer->cap.he_mac_cap & HE_DYN_SMPS) && wlan_config_get_he_dyn_smps(wdev))
		sta_rec->he_sta.mac_info.he_dyn_smps = 1;
	/*phy cap*/
	if ((peer->cap.he_phy_cap & HE_DUAL_BAND) && (asic_caps & fASIC_CAP_DBDC))
		sta_rec->he_sta.phy_info.dual_band_support = 1;
	sta_rec->he_sta.phy_info.ch_width_set = peer->cap.ch_bw.he_ch_width;
	if (phy_caps & fPHY_CAP_BW20_242TONE)
		sta_rec->he_sta.phy_info.bw20_242tone = peer->cap.ch_bw.he_bw20_242tone;
	sta_rec->he_sta.phy_info.punctured_preamble_rx = peer->cap.punc_preamble_rx;
	if (peer->cap.he_phy_cap & HE_DEV_CLASS_A)
		sta_rec->he_sta.phy_info.device_class = 1;
	he_ldpc = wlan_config_get_he_ldpc(wdev);
	if ((peer->cap.he_phy_cap & HE_LDPC) && he_ldpc)
		sta_rec->he_sta.phy_info.ldpc_support = 1;
	he_tx_stbc = wlan_config_get_he_tx_stbc(wdev);
	he_rx_stbc = wlan_config_get_he_rx_stbc(wdev);
	if (he_tx_stbc || he_rx_stbc)
		sta_rec->he_sta.phy_info.stbc_support = peer->cap.stbc.he_stbc;
	sta_rec->he_sta.phy_info.gi_cap = peer->cap.he_gi;
	if (peer->cap.he_phy_cap & HE_DCM_MAX_NSS_TX)
		sta_rec->he_sta.phy_info.dcm_max_nss_tx = 1;
	sta_rec->he_sta.phy_info.dcm_cap_tx = peer->cap.dcm_max_constellation_tx;
	if (peer->cap.he_phy_cap & HE_DCM_MAX_NSS_RX)
		sta_rec->he_sta.phy_info.dcm_max_nss_rx = 1;
	sta_rec->he_sta.phy_info.dcm_cap_rx = peer->cap.dcm_max_constellation_rx;
	sta_rec->he_sta.phy_info.dcm_max_ru = peer->cap.dcm_max_ru;
	if (peer->cap.he_phy_cap & HE_TX_1024QAM_UNDER_RU242)
		sta_rec->he_sta.phy_info.tx_le_ru242_1024qam = 1;
	if (peer->cap.he_phy_cap & HE_RX_1024QAM_UNDER_RU242)
		sta_rec->he_sta.phy_info.rx_le_ru242_1024qam = 1;
	if (peer->cap.he_phy_cap & HE_TRIG_CQI_FEEDBACK)
		sta_rec->he_sta.phy_info.triggered_cqi_feedback_support = 1;
	if (peer->cap.he_phy_cap & HE_PARTIAL_BW_ER)
		sta_rec->he_sta.phy_info.partial_bw_ext_range_support = 1;

	/*max nss mcs*/
	mcs = &sta_rec->he_sta.max_nss_mcs;
	os_move_mem(mcs->bw80_mcs, peer->cap.rate.he80_rx_nss_mcs, sizeof(mcs->bw80_mcs));
	os_move_mem(mcs->bw160_mcs, peer->cap.rate.he160_rx_nss_mcs, sizeof(mcs->bw160_mcs));
	os_move_mem(mcs->bw8080_mcs, peer->cap.rate.he8080_rx_nss_mcs, sizeof(mcs->bw8080_mcs));

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
	eaprawrapperentryset(peer->pAd, peer, &peer->RaEntry);
#endif

	/* SW patch of TGAX certification. */
	if ((wlan_config_get_ppdu_tx_type(wdev) == CAPI_MU)
		&& (wlan_config_get_tx_stream(wdev) == 2)
		&& wlan_config_get_mu_dl_mimo(wdev)) {
		/* [1]: for 2 NSTS, 3: not support */
		mcs->bw80_mcs[1] = 3;
		mcs->bw160_mcs[1] = 3;
		mcs->bw8080_mcs[1] = 3;
	}

}

UINT32 bssinfo_he_feature_decision(struct wifi_dev *wdev, UINT32 *feature)
{
	UINT32 features = 0;

	if (WMODE_CAP_AX(wdev->PhyMode))
		features |= BSS_INFO_HE_BASIC_FEATURE;

	if ((wdev->wdev_type == WDEV_TYPE_AP || wdev->wdev_type == WDEV_TYPE_ATE_STA)
		|| is_testmode_wdev(wdev->wdev_type))
		features |= BSS_INFO_BSS_COLOR_FEATURE;

	*feature |= features;
	return TRUE;
}

VOID fill_bssinfo_he(struct wifi_dev *wdev, struct _BSS_INFO_ARGUMENT_T *bssinfo)
{
	struct ppdu_caps *ppdu_cap = (struct ppdu_caps *)wlan_config_get_ppdu_caps(wdev);

	bssinfo->he_bss.default_pe_dur = ppdu_cap->default_pe_duration;
	bssinfo->he_bss.vht_oper_info_present = 0;
	bssinfo->he_bss.txop_dur_rts_thr = wlan_operate_get_he_txop_dur_rts_thld(wdev);

	bss_color_init(wdev, bssinfo);
}

VOID parse_he_bss_color_info(struct wifi_dev *wdev, VOID *ie_list)
{
	struct _bcn_ie_list *bcn_ie = (struct _bcn_ie_list *)ie_list;
	struct common_ies *cmm_ies = &bcn_ie->cmm_ies;
	UINT32 bss_color_info = cmm_ies->he_ops.bss_color_info;
	UINT8 color = bss_color_info & DOT11AX_BSS_COLOR_MASK;
	UINT8 disabled = (bss_color_info & DOT11AX_BSS_COLOR_DISABLE) ? 1 : 0;

#ifdef CONFIG_STA_SUPPORT
	struct _RTMP_ADAPTER *ad;
	PSTA_ADMIN_CONFIG pStaCfg;
	BOOLEAN is_my_bssid = FALSE;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_COLOR, DBG_LVL_DEBUG,
			"%s: bss_color_info = 0x%x, color = %d, disabled = %d\n",
			 __func__, bss_color_info, color, disabled);

	/* skip invalid parameters */
	if (!color)
		return;

	if (wdev->wdev_type == WDEV_TYPE_STA) {
		ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
		pStaCfg = GetStaCfgByWdev(ad, wdev);
		is_my_bssid = MAC_ADDR_EQUAL(bcn_ie->Bssid, pStaCfg->Bssid) ? TRUE : FALSE;

		if (is_my_bssid)
			set_bss_color_info(wdev, disabled, color);
		else
			bss_color_collision_detect(wdev, disabled, color);
	}
#endif

#ifdef CONFIG_AP_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		bss_color_collision_detect(wdev, disabled, color);
	}
#endif
}

VOID get_own_he_ie(struct wifi_dev *wdev, struct he_ies *he_ie)
{
	struct he_txrx_mcs_nss he_mcs_nss[3];

	/* he caps, including mac caps, phy caps, and tx rx he-mcs nss set */
	build_he_mac_cap(wdev, (UINT8 *)&he_ie->he_caps.mac_cap);
	build_he_phy_cap(wdev, (UINT8 *)&he_ie->he_caps.phy_cap);
	NdisZeroMemory(&he_mcs_nss[0], sizeof(struct he_txrx_mcs_nss) * 3);
	build_he_mcs_nss(wdev, (UINT8 *)&he_mcs_nss[0]);

	NdisMoveMemory(&he_ie->he_caps.txrx_mcs_nss, &he_mcs_nss[0], sizeof(struct he_txrx_mcs_nss));
	NdisMoveMemory(&he_ie->mcs_nss_160, &he_mcs_nss[1], sizeof(struct he_txrx_mcs_nss));
	NdisMoveMemory(&he_ie->mcs_nss_8080, &he_mcs_nss[2], sizeof(struct he_txrx_mcs_nss));

	/* he operation, including op params and he-mcs nss set */
	build_he_op_params(wdev, (UINT8 *)&he_ie->he_ops.he_op_param);
	build_basic_he_mcs_nss(wdev, (UINT8 *)&he_ie->he_ops.he_basic_mcs_nss);
}

/* current usage: WDS only */
VOID update_peer_he_params(struct _MAC_TABLE_ENTRY *peer, struct he_ies *he_ie)
{
	CLIENT_STATUS_SET_FLAG(peer, fCLIENT_STATUS_HE_CAPABLE);
	/* mac caps */
	peer_he_mac_caps(peer, &he_ie->he_caps.mac_cap);
	/* phy caps */
	peer_he_phy_caps(peer, &he_ie->he_caps.phy_cap);
	/* tx rx he-mcs nss */
	peer_he_txrx_mcs_nss(peer, &he_ie->he_caps.txrx_mcs_nss,
			&he_ie->mcs_nss_160, &he_ie->mcs_nss_8080);

	/* he op param */
	peer_he_op_params(peer, &he_ie->he_ops.he_op_param);
	/* bss_color_info */
	peer_bss_color_info(peer, he_ie->he_ops.bss_color_info);
	/* basic he-mcs and nss set */
	peer_he_basic_mcs_nss(peer, he_ie->he_ops.he_basic_mcs_nss);

}

#ifdef DOT11_HE_AX
VOID he_mac_cap_af_decision(struct wifi_dev *wdev, UINT32 mac_capinfo_1)
{
	UINT8 he_af = 0;
	UINT8 peer_he_af = (UINT8)((mac_capinfo_1 & DOT11AX_MAC_CAP_MAX_AMPDU_LEN_EXP_MASK) >>
		DOT11AX_MAC_CAP_MAX_AMPDU_LEN_EXP_SHIFT);

	he_af = min(wlan_operate_get_he_af(wdev), peer_he_af);

	MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, "%s:he_af(%d,%d,%d)\n",
		__func__, he_af, wlan_operate_get_he_af(wdev), peer_he_af);

	wlan_operate_set_he_af(wdev, he_af);
}
#endif /* DOT11_HE_AX */

static struct he_ch_layout he_ch_40M_6G[] = {
	{1, 5, 3},
	{9, 13, 11},
	{17, 21, 19},
	{25, 29, 27},
	{33, 37, 35},
	{41, 45, 43},
	{49, 53, 51},
	{57, 61, 59},
	{65, 69, 67},
	{73, 77, 75},
	{81, 85, 83},
	{89, 93, 91},
	{97, 101, 99},
	{105, 109, 107},
	{113, 117, 115},
	{121, 125, 123},
	{129, 133, 131},
	{137, 141, 139},
	{145, 149, 147},
	{153, 157, 155},
	{161, 165, 163},
	{169, 173, 171},
	{177, 181, 179},
	{185, 189, 187},
	{193, 197, 195},
	{201, 205, 203},
	{209, 213, 211},
	{217, 221, 219},
	{225, 229, 227},
	{0, 0, 0},
};

static struct he_ch_layout he_ch_80M_6G[] = {
	{1, 13, 7},
	{17, 29, 23},
	{33, 45, 39},
	{49, 61, 55},
	{65, 77, 71},
	{81, 93, 87},
	{97, 109, 103},
	{113, 125, 119},
	{129, 141, 135},
	{145, 157, 151},
	{161, 173, 167},
	{177, 189, 183},
	{193, 205, 199},
	{209, 221, 215},
	{0, 0, 0},
};

static struct he_ch_layout he_ch_160M_6G[] = {
	{1, 29, 15},
	{33, 61, 47},
	{65, 93, 79},
	{97, 125, 111},
	{129, 157, 143},
	{161, 189, 175},
	{193, 221, 207},
	{0, 0, 0},
};

static struct he_ch_layout he_ch_40M[] = {
	{36, 40, 38},
	{44, 48, 46},
	{52, 56, 54},
	{60, 64, 62},
	{100, 104, 102},
	{108, 112, 110},
	{116, 120, 118},
	{124, 128, 126},
	{132, 136, 134},
	{140, 144, 142},
	{149, 153, 151},
	{157, 161, 159},
	{165, 169, 167},
	{173, 177, 175},
	{0, 0, 0},
};

static struct he_ch_layout he_ch_80M[] = {
	{36, 48, 42},
	{52, 64, 58},
	{100, 112, 106},
	{116, 128, 122},
	{132, 144, 138},
	{149, 161, 155},
	{165, 177, 171},
	{0, 0, 0},
};

static struct he_ch_layout he_ch_160M[] = {
	{36, 64, 50},
	{100, 128, 114},
	{149, 177, 163},
	{0, 0, 0},
};

struct he_ch_layout *get_he_ch_array(UINT8 bw, UCHAR ch_band)
{
	switch (ch_band) {
	case CMD_CH_BAND_5G:
		if (bw == HE_BW_2040)
			return he_ch_40M;
		else if (bw == HE_BW_80)
			return he_ch_80M;
		else if (bw == HE_BW_160)
			return he_ch_160M;
		else
			return NULL;
		break;

	case CMD_CH_BAND_6G:
		if (bw == HE_BW_2040)
			return he_ch_40M_6G;
		else if (bw == HE_BW_80)
			return he_ch_80M_6G;
		else if (bw == HE_BW_160)
			return he_ch_160M_6G;
		else
			return NULL;
		break;

	default:
		return NULL;
	}
}

UCHAR he_cent_ch_freq(UCHAR prim_ch, UCHAR he_bw, UCHAR ch_band)
{
	INT idx = 0;
	struct he_ch_layout *ch_40M = get_he_ch_array(HE_BW_2040, ch_band);
	struct he_ch_layout *ch_80M = get_he_ch_array(HE_BW_80, ch_band);
	struct he_ch_layout *ch_160M = get_he_ch_array(HE_BW_160, ch_band);

	if ((he_bw == HE_BW_20) || (ch_band == CMD_CH_BAND_24G))
		return prim_ch;
	else if (he_bw == HE_BW_2040)
		while (ch_40M && (ch_40M[idx].ch_up_bnd != 0)) {
			if (prim_ch >= ch_40M[idx].ch_low_bnd &&
				prim_ch <= ch_40M[idx].ch_up_bnd)
				return ch_40M[idx].cent_freq_idx;

			idx++;
		}
	else if ((he_bw == HE_BW_80) || (he_bw == HE_BW_8080)) {
		while (ch_80M && (ch_80M[idx].ch_up_bnd != 0)) {
			if (prim_ch >= ch_80M[idx].ch_low_bnd &&
				prim_ch <= ch_80M[idx].ch_up_bnd)
				return ch_80M[idx].cent_freq_idx;

			idx++;
		}
	} else if (he_bw == HE_BW_160) {
		while (ch_160M && (ch_160M[idx].ch_up_bnd != 0)) {
			if (prim_ch >= ch_160M[idx].ch_low_bnd &&
				prim_ch <= ch_160M[idx].ch_up_bnd)
				return ch_160M[idx].cent_freq_idx;

			idx++;
		}
	}

	return prim_ch;
}
