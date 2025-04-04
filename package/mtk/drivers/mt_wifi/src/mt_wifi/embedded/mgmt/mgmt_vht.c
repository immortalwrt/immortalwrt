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

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"


#ifdef DOT11_VHT_AC

static char *vht_bw_str[] = {
	"20/40",
	"80",
	"160",
	"80+80",
	"invalid",
};

char *VhtBw2Str(INT VhtBw)
{
	if (VhtBw <= VHT_BW_8080)
		return vht_bw_str[VhtBw];
	else
		return vht_bw_str[4];
}
/*
	========================================================================
	Routine Description:
		Caller ensures we has 802.11ac support.
		Calls at setting VHT from AP/STASetinformation

	Arguments:
		pAd - Pointer to our adapter
		phymode  -

	========================================================================
*/
VOID RTMPSetVHT(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev)
{
#ifdef VHT_TXBF_SUPPORT
	VHT_CAP_INFO *vht_cap = &pAd->CommonCfg.vht_cap_ie.vht_cap;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->FlgHwTxBfCap) {
		/* Set ETxBF */
#ifdef MT_MAC
		mt_WrapSetVHTETxBFCap(pAd, wdev, vht_cap);
#endif
	}

#endif /* TXBF_SUPPORT */
}


VOID rtmp_set_vht(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RT_PHY_INFO *phy_info)
{
	UCHAR vht_bw;

	if (!phy_info)
		return;

	vht_bw = wlan_config_get_vht_bw(wdev);

	if (phy_info->bVhtEnable) {
		if (vht_bw <= VHT_BW_8080)
			phy_info->vht_bw = vht_bw;
		else
			phy_info->vht_bw = VHT_BW_2040;
	}
}


INT SetCommonVHT(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	if (!WMODE_CAP_AC(wdev->PhyMode)) {
		/* Clear previous VHT information */
		return FALSE;
	}

	RTMPSetVHT(pAd, wdev);
	return TRUE;
}

#ifdef CONFIG_STA_SUPPORT
/*
	========================================================================

	Routine Description:
		Verify the support rate for HT phy type

	Arguments:
		pAd				Pointer to our adapter

	Return Value:
		FALSE if pAd->CommonCfg.SupportedHtPhy doesn't accept the pHtCapability.  (AP Mode)

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
BOOLEAN RTMPCheckVht(
	IN PRTMP_ADAPTER pAd,
	IN UINT16 Wcid,
	IN VHT_CAP_IE * vht_cap,
	IN VHT_OP_IE * vht_op)
{
	VHT_CAP_INFO *vht_cap_info = &vht_cap->vht_cap;
	MAC_TABLE_ENTRY *pEntry;
	PSTA_ADMIN_CONFIG pStaCfg;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
		return FALSE;

	pEntry = &pAd->MacTab.Content[Wcid];
	pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);
	CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_VHT_CAPABLE);

	/* Save Peer Capability*/
	if (wlan_config_get_vht_sgi(pEntry->wdev) == GI_400) {
		if (vht_cap_info->sgi_80M)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI80_CAPABLE);

		if (vht_cap_info->sgi_160M)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI160_CAPABLE);
	}

	if (wlan_config_get_vht_stbc(pEntry->wdev)) {
		if (vht_cap_info->tx_stbc)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_VHT_TXSTBC_CAPABLE);

		if (vht_cap_info->rx_stbc)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_VHT_RXSTBC_CAPABLE);
	}

	if (wlan_config_get_vht_ldpc(pEntry->wdev) && (cap->phy_caps & fPHY_CAP_LDPC)) {
		if (vht_cap_info->rx_ldpc)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_VHT_RX_LDPC_CAPABLE);
	}

	/* Will check ChannelWidth for MCSSet[4] below */
	NdisZeroMemory(&pStaCfg->MlmeAux.vht_cap.mcs_set, sizeof(VHT_MCS_SET));
	pStaCfg->MlmeAux.vht_cap.mcs_set.rx_high_rate = wlan_operate_get_rx_stream(pEntry->wdev) * 325;
	pStaCfg->MlmeAux.vht_cap.mcs_set.tx_high_rate = wlan_operate_get_tx_stream(pEntry->wdev) * 325;
	/* pStaCfg->MlmeAux.vht_cap.vht_cap.ch_width = vht_cap_info->ch_width; */
#ifdef VHT_TXBF_SUPPORT

	if (cap->FlgHwTxBfCap) {
#ifdef MT_MAC
		mt_WrapSetVHTETxBFCap(pAd, pEntry->wdev, &pStaCfg->MlmeAux.vht_cap.vht_cap);
#endif
	}

#endif /* TXBF_SUPPORT */
	return TRUE;
}
#endif

#ifdef CONFIG_STA_SUPPORT
/* refine from RTMPCheckVht, will replace it when fully testing */
BOOLEAN check_vht(
	struct _RTMP_ADAPTER *ad,
	UINT16 wcid,
	struct common_ies *cmm_ies)
{
	struct _MAC_TABLE_ENTRY *peer;
	struct _STA_ADMIN_CONFIG *sta_cfg;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	if (!VALID_UCAST_ENTRY_WCID(ad, wcid))
		return FALSE;

	peer = &ad->MacTab.Content[wcid];
	sta_cfg = GetStaCfgByWdev(ad, peer->wdev);

	NdisZeroMemory(&sta_cfg->MlmeAux.vht_cap.mcs_set, sizeof(VHT_MCS_SET));
	sta_cfg->MlmeAux.vht_cap.mcs_set.rx_high_rate = wlan_operate_get_rx_stream(peer->wdev) * 325;
	sta_cfg->MlmeAux.vht_cap.mcs_set.tx_high_rate = wlan_operate_get_tx_stream(peer->wdev) * 325;
	/* sta_cfg->MlmeAux.vht_cap.vht_cap.ch_width = vht_cap_info->ch_width; */
#ifdef VHT_TXBF_SUPPORT
	if (cap->FlgHwTxBfCap) {
#ifdef MT_MAC
		mt_WrapSetVHTETxBFCap(ad, peer->wdev, &sta_cfg->MlmeAux.vht_cap.vht_cap);
#else
		setVHTETxBFCap(ad, &sta_cfg->MlmeAux.vht_cap.vht_cap);
#endif
	}
#endif /* TXBF_SUPPORT */
	return TRUE;
}
#endif	/* CONFIG_STA_SUPPORT */


static struct vht_ch_layout vht_ch_40M[] = {
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

static struct vht_ch_layout vht_ch_40M_6G[] = {
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

static struct vht_ch_layout vht_ch_80M_6G[] = {
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

static struct  vht_ch_layout vht_ch_160M_6G[] = {
	{1, 29, 15},
	{33, 61, 47},
	{65, 93, 79},
	{97, 125, 111},
	{129, 157, 143},
	{161, 189, 175},
	{193, 221, 207},
	{0, 0, 0},
};

static struct vht_ch_layout vht_ch_80M[] = {
	{36, 48, 42},
	{52, 64, 58},
	{100, 112, 106},
	{116, 128, 122},
	{132, 144, 138},
	{149, 161, 155},
	{165, 177, 171},
	{0, 0, 0},
};

static struct  vht_ch_layout vht_ch_160M[] = {
	{36, 64, 50},
	{100, 128, 114},
	{149, 177, 163},
	{0, 0, 0},
};

static struct  vht_ch_layout vht_ch_160M_non_weather[] = {
	{36, 64, 50},
	{149, 177, 163},
	{0, 0, 0},
};

struct vht_ch_layout *get_ch_array(UINT8 bw, UCHAR ch_band)
{
	switch (ch_band) {
	case CMD_CH_BAND_24G:
	case CMD_CH_BAND_5G:
		if (bw == BW_40)
			return vht_ch_40M;
		else if (bw == BW_80)
			return vht_ch_80M;
		else
			return vht_ch_160M;

	case CMD_CH_BAND_6G:
		if (bw == BW_80)
			return vht_ch_80M_6G;
		else
			return vht_ch_160M_6G;

	default:
		return NULL;
	}
}

VOID dump_vht_cap(RTMP_ADAPTER *pAd, VHT_CAP_IE *vht_ie)
{
	VHT_CAP_INFO *vht_cap = &vht_ie->vht_cap;
	VHT_MCS_SET *vht_mcs = &vht_ie->mcs_set;

	MTWF_PRINT("Dump VHT_CAP IE\n");
	hex_dump("VHT CAP IE Raw Data", (UCHAR *)vht_ie, sizeof(VHT_CAP_IE));
	MTWF_PRINT("VHT Capabilities Info Field\n");
	MTWF_PRINT("\tMaximum MPDU Length=%d\n", vht_cap->max_mpdu_len);
	MTWF_PRINT("\tSupported Channel Width=%d\n", vht_cap->ch_width);
	MTWF_PRINT("\tRxLDPC=%d\n", vht_cap->rx_ldpc);
	MTWF_PRINT("\tShortGI_80M=%d\n", vht_cap->sgi_80M);
	MTWF_PRINT("\tShortGI_160M=%d\n", vht_cap->sgi_160M);
	MTWF_PRINT("\tTxSTBC=%d\n", vht_cap->tx_stbc);
	MTWF_PRINT("\tRxSTBC=%d\n", vht_cap->rx_stbc);
	MTWF_PRINT("\tSU BeamformerCap=%d\n", vht_cap->bfer_cap_su);
	MTWF_PRINT("\tSU BeamformeeCap=%d\n", vht_cap->bfee_cap_su);
	MTWF_PRINT("\tCompressedSteeringNumOfBeamformerAnt=%d\n", vht_cap->bfee_sts_cap);
	MTWF_PRINT("\tNumber of Sounding Dimensions=%d\n", vht_cap->num_snd_dimension);
	MTWF_PRINT("\tMU BeamformerCap=%d\n", vht_cap->bfer_cap_mu);
	MTWF_PRINT("\tMU BeamformeeCap=%d\n", vht_cap->bfee_cap_mu);
	MTWF_PRINT("\tVHT TXOP PS=%d\n", vht_cap->vht_txop_ps);
	MTWF_PRINT("\t+HTC-VHT Capable=%d\n", vht_cap->htc_vht_cap);
	MTWF_PRINT("\tMaximum A-MPDU Length Exponent=%d\n", vht_cap->max_ampdu_exp);
	MTWF_PRINT("\tVHT LinkAdaptation Capable=%d\n", vht_cap->vht_link_adapt);
	MTWF_PRINT("VHT Supported MCS Set Field\n");
	MTWF_PRINT("\tRx Highest SupDataRate=%d\n", vht_mcs->rx_high_rate);
	MTWF_PRINT("\tRxMCS Map_1SS=%d\n", vht_mcs->rx_mcs_map.mcs_ss1);
	MTWF_PRINT("\tRxMCS Map_2SS=%d\n", vht_mcs->rx_mcs_map.mcs_ss2);
	MTWF_PRINT("\tTx Highest SupDataRate=%d\n", vht_mcs->tx_high_rate);
	MTWF_PRINT("\tTxMCS Map_1SS=%d\n", vht_mcs->tx_mcs_map.mcs_ss1);
	MTWF_PRINT("\tTxMCS Map_2SS=%d\n", vht_mcs->tx_mcs_map.mcs_ss2);
}


VOID dump_vht_op(RTMP_ADAPTER *pAd, VHT_OP_IE *vht_ie)
{
	struct vht_opinfo *vht_op = &vht_ie->vht_op_info;
	VHT_MCS_MAP *vht_mcs = &vht_ie->basic_mcs_set;

	MTWF_PRINT("Dump VHT_OP IE\n");
	hex_dump("VHT OP IE Raw Data", (UCHAR *)vht_ie, sizeof(VHT_OP_IE));
	MTWF_PRINT("VHT Operation Info Field\n");
	MTWF_PRINT("\tChannelWidth=%d\n", vht_op->ch_width);
	MTWF_PRINT("\tChannelCenterFrequency Seg 0=%d\n", vht_op->ccfs_0);
	MTWF_PRINT("\tChannelCenterFrequency Seg 1=%d\n", vht_op->ccfs_1);
	MTWF_PRINT("VHT Basic MCS Set Field\n");
	MTWF_PRINT("\tRxMCS Map_1SS=%d\n", vht_mcs->mcs_ss1);
	MTWF_PRINT("\tRxMCS Map_2SS=%d\n", vht_mcs->mcs_ss2);
}


#ifdef VHT_TXBF_SUPPORT
VOID trigger_vht_ndpa(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *entry)
{
	UCHAR *buf;
	VHT_NDPA_FRAME *vht_ndpa;
	struct wifi_dev *wdev = entry->wdev;
	UINT frm_len, sta_cnt;
	SNDING_STA_INFO *sta_info;

	if (MlmeAllocateMemory(pAd, &buf) != NDIS_STATUS_SUCCESS)
		return;

	NdisZeroMemory(buf, MAX_MGMT_PKT_LEN);
	vht_ndpa = (VHT_NDPA_FRAME *)buf;
	frm_len = sizeof(VHT_NDPA_FRAME);
	vht_ndpa->fc.Type = FC_TYPE_CNTL;
	vht_ndpa->fc.SubType = SUBTYPE_VHT_NDPA;
	COPY_MAC_ADDR(vht_ndpa->ra, entry->Addr);
	COPY_MAC_ADDR(vht_ndpa->ta, wdev->if_addr);
	/* Currnetly we only support 1 STA for a VHT DNPA */
	sta_info = vht_ndpa->sta_info;

	for (sta_cnt = 0; sta_cnt < 1; sta_cnt++) {
		sta_info->aid12 = entry->Aid;
		sta_info->fb_type = SNDING_FB_SU;
		sta_info->nc_idx = 0;
		vht_ndpa->token.token_num = entry->snd_dialog_token;
		frm_len += sizeof(SNDING_STA_INFO);
		sta_info++;

		if (frm_len >= (MAX_MGMT_PKT_LEN - sizeof(SNDING_STA_INFO))) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "len(%d) too large!cnt=%d\n",
					 frm_len, sta_cnt);
			break;
		}
	}

	if (entry->snd_dialog_token & 0xc0)
		entry->snd_dialog_token = 0;
	else
		entry->snd_dialog_token++;

	vht_ndpa->duration = pAd->CommonCfg.Dsifs +
						 RTMPCalcDuration(pAd, pAd->CommonCfg.MlmeRate, frm_len);
	MTWF_PRINT("Send VHT NDPA Frame to STA("MACSTR")\n",
			 MAC2STR(entry->Addr));
	hex_dump("VHT NDPA Frame", buf, frm_len);
	MiniportMMRequest(pAd, 0, buf, frm_len);
	MlmeFreeMemory(buf);
#ifdef SOFT_SOUNDING

	if (1) {
		HEADER_802_11 *pNullFr;
		UCHAR *qos_p;
		UCHAR NullFrame[48];

		NdisZeroMemory(NullFrame, 48);
		pNullFr = (PHEADER_802_11)&NullFrame[0];
		frm_len = sizeof(HEADER_802_11);
		pNullFr->FC.Type = FC_TYPE_DATA;
		pNullFr->FC.SubType = SUBTYPE_QOS_NULL;
		pNullFr->FC.FrDs = 1;
		pNullFr->FC.ToDs = 0;
		COPY_MAC_ADDR(pNullFr->Addr1, entry->Addr);
		COPY_MAC_ADDR(pNullFr->Addr2, wdev->if_addr);
		COPY_MAC_ADDR(pNullFr->Addr3, wdev->bssid);
		qos_p = ((UCHAR *)pNullFr) + frm_len;
		qos_p[0] = 0;
		qos_p[1] = 0;
		frm_len += 2;
		entry->snd_reqired = TRUE;
		MTWF_PRINT("Send sounding QoSNULL Frame to STA("MACSTR")\n",
				 MAC2STR(entry->Addr));
		hex_dump("VHT NDP Frame(QoSNull)", NullFrame, frm_len);
		hif_kickout_nullframe_tx(pAd, 0, NullFrame, frm_len);
	}

#endif /* SOFT_SOUNDING */
}
#endif /* VHT_TXBF_SUPPORT */


/*
	Get BBP Channel Index by RF channel info
	return value: 0~3
*/
UCHAR vht_prim_ch_idx(UCHAR vht_cent_ch, UCHAR prim_ch, UINT8 rf_bw)
{
	INT idx = 0;
	UCHAR bbp_idx = 0;

	if (vht_cent_ch == prim_ch)
		goto done;

	if (rf_bw == RF_BW_80) {
		while (vht_ch_80M[idx].ch_up_bnd != 0) {
			if (vht_cent_ch == vht_ch_80M[idx].cent_freq_idx) {
				if (prim_ch == vht_ch_80M[idx].ch_up_bnd)
					bbp_idx = 3;
				else if (prim_ch == vht_ch_80M[idx].ch_low_bnd)
					bbp_idx = 0;
				else
					bbp_idx = prim_ch > vht_cent_ch ? 2 : 1;

				break;
			}

			idx++;
		}
	} else if (rf_bw == RF_BW_160) {
		/* TODO: Shiang-MT7615, fix me!! */
		while (vht_ch_160M[idx].ch_up_bnd != 0) {
			if (vht_cent_ch == vht_ch_160M[idx].cent_freq_idx) {
				if (prim_ch == vht_ch_160M[idx].ch_up_bnd)
					bbp_idx = 3;
				else if (prim_ch == vht_ch_160M[idx].ch_low_bnd)
					bbp_idx = 0;
				else
					bbp_idx = prim_ch > vht_cent_ch ? 2 : 1;

				break;
			}

			idx++;
		}
	}

done:
	MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "%s():(VhtCentCh=%d, PrimCh=%d) =>BbpChIdx=%d\n",
			  __func__, vht_cent_ch, prim_ch, bbp_idx);
	return bbp_idx;
}


/*
	Currently we only consider about VHT 80MHz!
*/
UCHAR vht_cent_ch_freq(UCHAR prim_ch, UCHAR vht_bw, UCHAR ch_band)
{
	INT idx = 0;
	struct vht_ch_layout *ch_80M = get_ch_array(BW_80, ch_band);
	struct vht_ch_layout *ch_160M = get_ch_array(BW_160, ch_band);

	if (vht_bw == VHT_BW_2040 || ((prim_ch < 36) && ch_band == CMD_CH_BAND_5G))
		return prim_ch;
	else if ((vht_bw == VHT_BW_80) || (vht_bw == VHT_BW_8080)) {
		while (ch_80M && ch_80M[idx].ch_up_bnd != 0) {
			if (prim_ch >= ch_80M[idx].ch_low_bnd &&
				prim_ch <= ch_80M[idx].ch_up_bnd)
				return ch_80M[idx].cent_freq_idx;

			idx++;
		}
	} else if (vht_bw == VHT_BW_160) {
		while (ch_160M && ch_160M[idx].ch_up_bnd != 0) {
			if (prim_ch >= ch_160M[idx].ch_low_bnd &&
				prim_ch <= ch_160M[idx].ch_up_bnd)
				return ch_160M[idx].cent_freq_idx;

			idx++;
		}
	}

	return prim_ch;
}

UCHAR vht_cent_ch_freq_40mhz(UCHAR prim_ch, UCHAR vht_bw, UCHAR ch_band)
{
	INT idx = 0;
	struct vht_ch_layout *ch_40M = get_ch_array(BW_40, ch_band);

	if ((vht_bw == VHT_BW_2040) && (ch_band == CMD_CH_BAND_5G))
		while (ch_40M && ch_40M[idx].ch_up_bnd != 0) {
			if (prim_ch >= ch_40M[idx].ch_low_bnd &&
				prim_ch <= ch_40M[idx].ch_up_bnd)
				return ch_40M[idx].cent_freq_idx;

			idx++;
		}

	return prim_ch;
}

UINT16 vht_max_mpdu_size[3] = {3839u, 7935u, 11454u};

INT vht_mode_adjust(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry,
	VHT_CAP_IE *cap, VHT_OP_IE *op, OPERATING_MODE *op_mode, UCHAR *bw_from_opclass)
{
	RT_PHY_INFO *ht_phyinfo;
	struct wifi_dev *wdev = pEntry->wdev;
	ADD_HT_INFO_IE *addht;
	UCHAR vht_sgi = 0;
	UCHAR band = 0;

	pEntry->MaxHTPhyMode.field.MODE = MODE_VHT;
	pEntry->cap.modes |= VHT_SUPPORT;

	if (!wdev)
		return FALSE;

	addht = wlan_operate_get_addht(wdev);
	band = HcGetBandByWdev(pEntry->wdev);
	pAd->MacTab.fAnyStationNonGF[band] = TRUE;
	vht_sgi = wlan_config_get_vht_sgi(wdev);

	/*
	if (op->vht_op_info.ch_width >= 1 && pEntry->MaxHTPhyMode.field.BW == BW_40)
	{
		pEntry->MaxHTPhyMode.field.BW= BW_80;
		pEntry->MaxHTPhyMode.field.ShortGI = (cap->vht_cap.sgi_80M);
		pEntry->MaxHTPhyMode.field.STBC = (cap->vht_cap.rx_stbc > 1 ? 1 : 0);
	}
	*/
	if ((pEntry->MaxHTPhyMode.field.BW == BW_40) && (pEntry->wdev)) {
		ht_phyinfo = &pEntry->wdev->DesiredHtPhyInfo;

		if (ht_phyinfo) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "DesiredHtPhyInfo->vht_bw=%d, ch_width=%d\n",
					 ht_phyinfo->vht_bw, cap->vht_cap.ch_width);

			if ((ht_phyinfo->vht_bw == VHT_BW_2040)) {
				pEntry->MaxHTPhyMode.field.STBC = ((wlan_config_get_vht_stbc(pEntry->wdev) & cap->vht_cap.rx_stbc) ? 1 : 0);
			} else if ((ht_phyinfo->vht_bw >= VHT_BW_80) && (cap->vht_cap.ch_width == 0)) {
				if (op != NULL) {
					if (op->vht_op_info.ch_width == 0)  /* peer support VHT20,40 */
						pEntry->MaxHTPhyMode.field.BW = BW_40;
					else {
						pEntry->MaxHTPhyMode.field.BW = BW_80;
						pEntry->MaxHTPhyMode.field.ShortGI = (vht_sgi & (cap->vht_cap.sgi_80M));
					}
				} else if (op_mode != NULL) {
					if (op_mode->ch_width == 0)  /* peer support VHT20 */
						pEntry->MaxHTPhyMode.field.BW = BW_20;
					else if (op_mode->ch_width == 1)
						pEntry->MaxHTPhyMode.field.BW = BW_40;
					else {
						pEntry->MaxHTPhyMode.field.BW = BW_80;
						pEntry->MaxHTPhyMode.field.ShortGI = (vht_sgi & (cap->vht_cap.sgi_80M));
					}
				} else {
					/* can not know peer capability,
					use the bw from support opclass as maximum capability. */
					if ((bw_from_opclass != NULL) && ((*bw_from_opclass) < BW_80))
						pEntry->MaxHTPhyMode.field.BW = BW_40;
					else {
						pEntry->MaxHTPhyMode.field.BW = BW_80;
						pEntry->MaxHTPhyMode.field.ShortGI = (vht_sgi & (cap->vht_cap.sgi_80M));
					}
				}
				pEntry->MaxHTPhyMode.field.STBC = ((wlan_config_get_vht_stbc(pEntry->wdev) & cap->vht_cap.rx_stbc)  ? 1 : 0);
			} else if ((ht_phyinfo->vht_bw == VHT_BW_80) && (cap->vht_cap.ch_width != 0)) {
				/* bw80 */
				pEntry->MaxHTPhyMode.field.BW = BW_80;
				pEntry->MaxHTPhyMode.field.ShortGI = (vht_sgi & (cap->vht_cap.sgi_80M));
				pEntry->MaxHTPhyMode.field.STBC = ((wlan_config_get_vht_stbc(pEntry->wdev) & cap->vht_cap.rx_stbc)  ? 1 : 0);
			} else if (((ht_phyinfo->vht_bw == VHT_BW_160) || (ht_phyinfo->vht_bw == VHT_BW_8080)) &&
					   (cap->vht_cap.ch_width != 0)) {
				/* bw160 or bw80+80 */
				pEntry->MaxHTPhyMode.field.BW = BW_160;
				pEntry->MaxHTPhyMode.field.ShortGI = (vht_sgi & (cap->vht_cap.sgi_160M));
				pEntry->MaxHTPhyMode.field.STBC = ((wlan_config_get_vht_stbc(pEntry->wdev) & cap->vht_cap.rx_stbc) ? 1 : 0);
			}
		}
	}

	return TRUE;
}

VOID set_vht_cap(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *entry, VHT_CAP_IE *vht_cap_ie)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 cfg_max_mpdu_len;

	CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_VHT_CAPABLE);

	if (wlan_config_get_vht_ldpc(entry->wdev) && (cap->phy_caps & fPHY_CAP_LDPC) &&
		(vht_cap_ie->vht_cap.rx_ldpc))
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_VHT_RX_LDPC_CAPABLE);

	if (wlan_config_get_vht_sgi(entry->wdev) == GI_400) {
		if (vht_cap_ie->vht_cap.sgi_80M)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_SGI80_CAPABLE);

		if (vht_cap_ie->vht_cap.sgi_160M)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_SGI160_CAPABLE);
	}

	if (wlan_config_get_vht_stbc(entry->wdev)) {
		if (vht_cap_ie->vht_cap.tx_stbc)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_VHT_TXSTBC_CAPABLE);

		if (vht_cap_ie->vht_cap.rx_stbc)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_VHT_RXSTBC_CAPABLE);
	}

	entry->MaxRAmpduFactor = (vht_cap_ie->vht_cap.max_ampdu_exp > entry->MaxRAmpduFactor) ?
		vht_cap_ie->vht_cap.max_ampdu_exp : entry->MaxRAmpduFactor;

	cfg_max_mpdu_len = wlan_config_get_vht_max_mpdu_len(entry->wdev);
	if (cfg_max_mpdu_len > cap->ppdu.max_mpdu_len)
		cfg_max_mpdu_len = cap->ppdu.max_mpdu_len;
	entry->AMsduSize = vht_cap_ie->vht_cap.max_mpdu_len;
	if (vht_cap_ie->vht_cap.max_mpdu_len > cfg_max_mpdu_len)
		entry->AMsduSize = cfg_max_mpdu_len;

	if (entry->AMsduSize < (sizeof(vht_max_mpdu_size) / sizeof(vht_max_mpdu_size[0])))
		entry->amsdu_limit_len = vht_max_mpdu_size[entry->AMsduSize];
	else
		entry->amsdu_limit_len = 0;
	entry->amsdu_limit_len_adjust = entry->amsdu_limit_len;
}

static UCHAR dot11_vht_mcs_bw_cap[] = {
	8,		8,		9,		8, /* BW20, 1SS~4SS */
	9,		9,		9,		9, /* BW40, 1SS~4SS */
	9,		9,		9,		9, /* BW80, 1SS~4SS */
	9,		9,		8,		9, /* BW160, 1SS~4SS */
};

static UCHAR spec_cap_to_mcs[] = {
	7,  /* VHT_MCS_CAP_7 */
	8, /* VHT_MCS_CAP_8 */
	9, /* VHT_MCS_CAP_9 */
	0, /* VHT_MCS_CAP_NA */
};

INT dot11_vht_mcs_to_internal_mcs(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	VHT_CAP_IE *vht_cap,
	HTTRANSMIT_SETTING *tx)
{
	HTTRANSMIT_SETTING *tx_mode = tx;
	UCHAR spec_cap, peer_cap, cap_offset = 0, nss;
	/* TODO: implement get_vht_max_mcs to get peer max MCS */

	switch (tx_mode->field.BW) {
	case BW_20:
		cap_offset = 0;
		break;

	case BW_40:
		cap_offset = 4;
		break;

	case BW_80:
		cap_offset = 8;
		break;

	case BW_160:
		cap_offset = 12;
		break;

	default:
		cap_offset = 0;
		break;
	};

	if ((vht_cap->mcs_set.rx_mcs_map.mcs_ss1 != VHT_MCS_CAP_NA) && (wlan_operate_get_tx_stream(wdev) >= 1)) {
		nss = 1;
		spec_cap = dot11_vht_mcs_bw_cap[cap_offset + (nss - 1)];
		peer_cap = spec_cap_to_mcs[vht_cap->mcs_set.rx_mcs_map.mcs_ss1];

		if (peer_cap < spec_cap)
			tx_mode->field.MCS = ((nss - 1) << 4) | peer_cap;
		else
			tx_mode->field.MCS = ((nss - 1) << 4) | spec_cap;
	}

	if ((vht_cap->mcs_set.rx_mcs_map.mcs_ss2 != VHT_MCS_CAP_NA) && (wlan_operate_get_tx_stream(wdev) >= 2)) {
		nss = 2;
		spec_cap = dot11_vht_mcs_bw_cap[cap_offset + (nss - 1)];
		peer_cap = spec_cap_to_mcs[vht_cap->mcs_set.rx_mcs_map.mcs_ss2];

		if (peer_cap < spec_cap)
			tx_mode->field.MCS = ((nss - 1) << 4) | peer_cap;
		else
			tx_mode->field.MCS = ((nss - 1) << 4) | spec_cap;
	}

	if ((vht_cap->mcs_set.rx_mcs_map.mcs_ss3 != VHT_MCS_CAP_NA) && (wlan_operate_get_tx_stream(wdev) >= 3)) {
		nss = 3;
		spec_cap = dot11_vht_mcs_bw_cap[cap_offset + (nss - 1)];
		peer_cap = spec_cap_to_mcs[vht_cap->mcs_set.rx_mcs_map.mcs_ss3];

		if (peer_cap < spec_cap)
			tx_mode->field.MCS = ((nss - 1) << 4) | peer_cap;
		else
			tx_mode->field.MCS = ((nss - 1) << 4) | spec_cap;
	}

	if ((vht_cap->mcs_set.rx_mcs_map.mcs_ss4 != VHT_MCS_CAP_NA) && (wlan_operate_get_tx_stream(wdev) >= 4)) {
		nss = 4;
		spec_cap = dot11_vht_mcs_bw_cap[cap_offset + (nss - 1)];
		peer_cap = spec_cap_to_mcs[vht_cap->mcs_set.rx_mcs_map.mcs_ss4];

		if (peer_cap < spec_cap)
			tx_mode->field.MCS = ((nss - 1) << 4) | peer_cap;
		else
			tx_mode->field.MCS = ((nss - 1) << 4) | spec_cap;
	}

	return TRUE;
}


INT get_vht_op_ch_width(RTMP_ADAPTER *pAd)
{
	return TRUE;
}

/********************************************************************
	Procedures for 802.11 AC Information elements
********************************************************************/
/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, ProbResp frames
*/
INT build_quiet_channel(RTMP_ADAPTER *pAd, UCHAR *buf)
{
	INT len = 0;
	return len;
}


/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, ProbResp frames
*/
INT build_ext_bss_load(RTMP_ADAPTER *pAd, UCHAR *buf)
{
	INT len = 0;
	return len;
}

/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, ProbResp frames
*/
INT build_ext_pwr_constraint(RTMP_ADAPTER *pAd, UCHAR *buf)
{
	INT len = 0;
	return len;
}


/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, ProbResp frames
*/
INT build_vht_txpwr_envelope(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0, pwr_cnt;
	VHT_TXPWR_ENV_IE txpwr_env;
	UCHAR vht_bw = wlan_operate_get_vht_bw(wdev);
	UCHAR ht_bw = wlan_operate_get_ht_bw(wdev);

	NdisZeroMemory(&txpwr_env, sizeof(txpwr_env));

	if ((vht_bw == VHT_BW_160)
		|| (vht_bw == VHT_BW_8080))
		pwr_cnt = 3;
	else if (vht_bw == VHT_BW_80)
		pwr_cnt = 2;
	else {
		if (ht_bw == HT_BW_40)
			pwr_cnt = 1;
		else
			pwr_cnt = 0;
	}

	txpwr_env.tx_pwr_info.max_tx_pwr_cnt = pwr_cnt;
	txpwr_env.tx_pwr_info.max_tx_pwr_interpretation = TX_PWR_INTERPRET_EIRP;

	/* TODO: fixme, we need the real tx_pwr value for each port. */
	for (len = 0; len <= pwr_cnt; len++)
		txpwr_env.tx_pwr_bw[len] = 47; /* 23.5dB */

	len = 2 + pwr_cnt;
	NdisMoveMemory(buf, &txpwr_env, len);
	return len;
}

/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, (Re)AssocResp, ProbResp frames
*/
INT build_vht_oper_ie(RTMP_ADAPTER *pAd, UCHAR bw, UCHAR Channel, struct wifi_dev *wdev, UCHAR *buf)
{
	VHT_OP_IE vht_op;
	UCHAR cent_ch;
#ifdef RT_BIG_ENDIAN
	UINT16 tmp;
#endif /* RT_BIG_ENDIAN */
	UCHAR cen_ch_2;
	UCHAR ch_band = 0;

#ifdef CONFIG_AP_SUPPORT
	struct DOT11_H *pDot11h = NULL;
#endif

	if (wdev == NULL)
		return FALSE;

	cen_ch_2 = wlan_operate_get_cen_ch_2(wdev);

	NdisZeroMemory((UCHAR *)&vht_op, sizeof(VHT_OP_IE));
#ifdef CONFIG_AP_SUPPORT
	pDot11h = wdev->pDot11_H;
	ch_band = wlan_config_get_ch_band(wdev);

	if (pDot11h == NULL)
		return FALSE;

	if (WMODE_CAP_5G(wdev->PhyMode) &&
		(pAd->CommonCfg.bIEEE80211H == 1) &&
		(pDot11h->RDMode == RD_SWITCHING_MODE))
		cent_ch = vht_cent_ch_freq(pDot11h->org_ch, bw, ch_band);
	else
#endif /* CONFIG_AP_SUPPORT */
		cent_ch = vht_cent_ch_freq(Channel, bw, ch_band);

	switch (bw) {
	case VHT_BW_2040:
		vht_op.vht_op_info.ch_width = 0;
		vht_op.vht_op_info.ccfs_0 = vht_cent_ch_freq_40mhz(wdev->channel, VHT_BW_2040, ch_band);;
		vht_op.vht_op_info.ccfs_1 = 0;
		break;

	case VHT_BW_80:
		vht_op.vht_op_info.ch_width = 1;
		vht_op.vht_op_info.ccfs_0 = cent_ch;
		vht_op.vht_op_info.ccfs_1 = 0;
		break;

	case VHT_BW_160:
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		vht_op.vht_op_info.ch_width = 2;
#else
		vht_op.vht_op_info.ch_width = 1;
#endif
		vht_op.vht_op_info.ccfs_0 =
			(Channel > cent_ch) ? (cent_ch + 8) : (cent_ch - 8);
		vht_op.vht_op_info.ccfs_1 = cent_ch;
		break;

	case VHT_BW_8080:
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		vht_op.vht_op_info.ch_width = 3;
#else
		vht_op.vht_op_info.ch_width = 1;
#endif
		vht_op.vht_op_info.ccfs_0 = cent_ch;
		vht_op.vht_op_info.ccfs_1 = cen_ch_2;
		break;
	}

	vht_op.basic_mcs_set.mcs_ss1 = VHT_MCS_CAP_7;
	vht_op.basic_mcs_set.mcs_ss2 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss3 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss4 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss5 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss6 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss7 = VHT_MCS_CAP_NA;
	vht_op.basic_mcs_set.mcs_ss8 = VHT_MCS_CAP_NA;

#ifdef RT_BIG_ENDIAN
	/* SWAP16((UINT16)vht_op.basic_mcs_set); */
	NdisCopyMemory(&tmp, &vht_op.basic_mcs_set, sizeof(VHT_MCS_MAP));
	tmp = SWAP16(tmp);
	NdisCopyMemory(&vht_op.basic_mcs_set, &tmp, sizeof(VHT_MCS_MAP));
#endif /* RT_BIG_ENDIAN */
	NdisMoveMemory((UCHAR *)buf, (UCHAR *)&vht_op, sizeof(VHT_OP_IE));
	/* hex_dump("vht op info", (UCHAR *)&vht_op, sizeof(VHT_OP_IE)); */
	return sizeof(VHT_OP_IE);
}


/*
	Defined in IEEE 802.11AC

	Appeared in Beacon, (Re)AssocReq, (Re)AssocResp, ProbReq/Resp frames
*/
static UINT16 VHT_HIGH_RATE_BW80[3][4] = {
	{292, 585, 877, 1170},
	{351, 702, 1053, 1404},
	{390, 780, 1170, 1560},
};

static UINT16 VHT_HIGH_RATE_BW20[3][4] = {
	{65, 130, 195, 260},
	{78, 156, 234, 312},
	{0, 0, 260, 0},
};

static UINT16 VHT_HIGH_RATE_BW40[3][4] = {
	{135, 270, 405, 540},
	{162, 324, 486, 648},
	{180, 360, 540, 720},
};

static UINT16 VHT_HIGH_RATE_BW160_BW8080[3][4] = {
	{585, 1170, 1755, 2340},
	{702, 1404, 2106, 2808},
	{780, 1560, 0, 3120},
};

INT build_vht_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	VHT_CAP_IE vht_cap_ie;
	INT rx_nss, tx_nss, mcs_cap;
	UCHAR cap_vht_bw = wlan_operate_get_vht_bw(wdev);
	UINT8 max_mpdu_len;
#ifdef RT_BIG_ENDIAN
	UINT32 tmp_1;
	UINT64 tmp_2;
#endif /*RT_BIG_ENDIAN*/
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	NdisZeroMemory((UCHAR *)&vht_cap_ie,  sizeof(VHT_CAP_IE));

	max_mpdu_len = wlan_config_get_vht_max_mpdu_len(wdev);
	if (max_mpdu_len > cap->ppdu.max_mpdu_len)
		max_mpdu_len = cap->ppdu.max_mpdu_len;
	vht_cap_ie.vht_cap.max_mpdu_len = max_mpdu_len;

	if (cap_vht_bw == VHT_BW_160) {
		if (!IS_PHY_CAPS(cap->phy_caps, fPHY_CAP_BW160C_STD)) {
			vht_cap_ie.vht_cap.ch_width  = 2;
			vht_cap_ie.vht_cap.ex_nss_bw = 3;
		} else {
			vht_cap_ie.vht_cap.ch_width  = 1;
			vht_cap_ie.vht_cap.ex_nss_bw = 0;
		}
		vht_cap_ie.vht_cap.sgi_160M = wlan_config_get_vht_sgi(wdev); 
	} else if (cap_vht_bw == VHT_BW_8080) {
		vht_cap_ie.vht_cap.ch_width = 2;
		vht_cap_ie.vht_cap.ex_nss_bw = 3;
		vht_cap_ie.vht_cap.sgi_160M = wlan_config_get_vht_sgi(wdev);
	} else
		vht_cap_ie.vht_cap.ch_width = 0;

	if (wlan_config_get_vht_ldpc(wdev) && (cap->phy_caps & fPHY_CAP_LDPC))
		vht_cap_ie.vht_cap.rx_ldpc = 1;
	else
		vht_cap_ie.vht_cap.rx_ldpc = 0;

	if (cap_vht_bw >= VHT_BW_80)
		vht_cap_ie.vht_cap.sgi_80M = wlan_config_get_vht_sgi(wdev);

	vht_cap_ie.vht_cap.htc_vht_cap = 1;
	vht_cap_ie.vht_cap.max_ampdu_exp = cap->ppdu.vht_max_ampdu_len_exp;
	vht_cap_ie.vht_cap.tx_stbc = 0;
	vht_cap_ie.vht_cap.rx_stbc = 0;

	if (wlan_config_get_vht_stbc(wdev)) {
		UINT8   ucTxPath = pAd->Antenna.field.TxPath;

#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
			UINT8 band_idx = HcGetBandByWdev(wdev);

			if (band_idx == DBDC_BAND0)
				ucTxPath = pAd->dbdc_band0_tx_path;
			else
				ucTxPath = pAd->dbdc_band1_tx_path;
		}
#endif

#ifdef ANTENNA_CONTROL_SUPPORT
		{
			UINT8 BandIdx = HcGetBandByWdev(wdev);
			if (pAd->bAntennaSetAPEnable[BandIdx])
				ucTxPath = pAd->TxStream[BandIdx];
		}
#endif /* ANTENNA_CONTROL_SUPPORT */

		if (ucTxPath >= 2)
			vht_cap_ie.vht_cap.tx_stbc = 1;
		else
			vht_cap_ie.vht_cap.tx_stbc = 0;

		vht_cap_ie.vht_cap.rx_stbc = 1;
	}

	vht_cap_ie.vht_cap.tx_ant_consistency = 1;
	vht_cap_ie.vht_cap.rx_ant_consistency = 1;
#ifdef VHT_TXBF_SUPPORT

	if (cap->FlgHwTxBfCap) {
		VHT_CAP_INFO vht_cap;

		NdisCopyMemory(&vht_cap, &pAd->CommonCfg.vht_cap_ie.vht_cap, sizeof(VHT_CAP_INFO));
		mt_WrapSetVHTETxBFCap(pAd, wdev, &vht_cap);
		vht_cap_ie.vht_cap.num_snd_dimension = vht_cap.num_snd_dimension;
		vht_cap_ie.vht_cap.bfee_sts_cap      = txbf_bfee_get_bfee_sts(vht_cap.bfee_sts_cap);
		vht_cap_ie.vht_cap.bfee_cap_su       = vht_cap.bfee_cap_su;
		vht_cap_ie.vht_cap.bfer_cap_su       = vht_cap.bfer_cap_su;
#ifdef MT_MAC
		vht_cap_ie.vht_cap.bfee_cap_mu       = vht_cap.bfee_cap_mu;
		vht_cap_ie.vht_cap.bfer_cap_mu       = (vht_cap.bfer_cap_mu && wlan_config_get_vht_bfer_cap_mu(wdev)) ? 1 : 0;
#endif
	}

#endif
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss1 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss3 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss4 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss5 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss6 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss7 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss8 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss1 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss2 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss3 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss4 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss5 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss6 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss7 = VHT_MCS_CAP_NA;
	vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss8 = VHT_MCS_CAP_NA;
	mcs_cap = cap->mcs_nss.max_vht_mcs;
	rx_nss = wlan_operate_get_rx_stream(wdev);
	tx_nss = wlan_operate_get_tx_stream(wdev);

	if ((mcs_cap <= VHT_MCS_CAP_9)
		&& ((rx_nss > 0) && (rx_nss <= 4))
		&& ((tx_nss > 0) && (tx_nss <= 4))) {

		/* BW 80M is the default setting*/
		vht_cap_ie.mcs_set.rx_high_rate = VHT_HIGH_RATE_BW80[mcs_cap][rx_nss - 1];
		vht_cap_ie.mcs_set.tx_high_rate = VHT_HIGH_RATE_BW80[mcs_cap][tx_nss - 1];

		if (rx_nss >= 1)
			vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss1 = mcs_cap;

		if (rx_nss >= 2)
			vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 = mcs_cap;

		if (rx_nss >= 3)
			vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss3 = mcs_cap;

		if (rx_nss >= 4)
			vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss4 = mcs_cap;

		if (tx_nss >= 1)
			vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss1 = mcs_cap;

		if (tx_nss >= 2)
			vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss2 = mcs_cap;

		if (tx_nss >= 3)
			vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss3 = mcs_cap;

		if (tx_nss >= 4)
			vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss4 = mcs_cap;

		vht_cap_ie.mcs_set.vht_ext_nss_bw_cap = 1;

		if (wlan_config_get_vht_bw(wdev) == VHT_BW_2040
			&& wlan_config_get_ht_bw(wdev) == HT_BW_20) {
			/* BW 20M only */

			vht_cap_ie.mcs_set.rx_high_rate = VHT_HIGH_RATE_BW20[mcs_cap][rx_nss - 1];
			vht_cap_ie.mcs_set.tx_high_rate = VHT_HIGH_RATE_BW20[mcs_cap][rx_nss - 1];

			if (mcs_cap == VHT_MCS_CAP_9) {

				if (vht_cap_ie.mcs_set.rx_high_rate == 0)
					vht_cap_ie.mcs_set.rx_high_rate = VHT_HIGH_RATE_BW20[VHT_MCS_CAP_8][rx_nss - 1];

				if (vht_cap_ie.mcs_set.tx_high_rate == 0)
					vht_cap_ie.mcs_set.tx_high_rate = VHT_HIGH_RATE_BW20[VHT_MCS_CAP_8][rx_nss - 1];

				if (rx_nss >= 1)
					vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss1 = VHT_MCS_CAP_8;

				if (rx_nss >= 2)
					vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 = VHT_MCS_CAP_8;

				if (rx_nss >= 3)
					vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss3 = VHT_MCS_CAP_9;

				if (rx_nss >= 4)
					vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss4 = VHT_MCS_CAP_8;

				if (tx_nss >= 1)
					vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss1 = VHT_MCS_CAP_8;

				if (tx_nss >= 2)
					vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss2 = VHT_MCS_CAP_8;

				if (tx_nss >= 3)
					vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss3 = VHT_MCS_CAP_9;

				if (tx_nss >= 4)
					vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss4 = VHT_MCS_CAP_8;
			}
		} else if (wlan_config_get_vht_bw(wdev) == VHT_BW_2040
			&& wlan_config_get_ht_bw(wdev) == HT_BW_40) {
			/* BW 40M */

			vht_cap_ie.mcs_set.rx_high_rate = VHT_HIGH_RATE_BW40[mcs_cap][rx_nss - 1];
			vht_cap_ie.mcs_set.tx_high_rate = VHT_HIGH_RATE_BW40[mcs_cap][rx_nss - 1];

		} else if ((wlan_config_get_vht_bw(wdev) == VHT_BW_160 || wlan_config_get_vht_bw(wdev) == VHT_BW_8080)
			&& wlan_config_get_ht_bw(wdev) == HT_BW_40) {
			/* BW 160M or 80M+80M */

			vht_cap_ie.mcs_set.rx_high_rate = VHT_HIGH_RATE_BW160_BW8080[mcs_cap][rx_nss - 1];
			vht_cap_ie.mcs_set.tx_high_rate = VHT_HIGH_RATE_BW160_BW8080[mcs_cap][rx_nss - 1];

			if (mcs_cap == VHT_MCS_CAP_9) {

				if (vht_cap_ie.mcs_set.rx_high_rate == 0)
					vht_cap_ie.mcs_set.rx_high_rate = VHT_HIGH_RATE_BW160_BW8080[VHT_MCS_CAP_8][rx_nss - 1];

				if (vht_cap_ie.mcs_set.tx_high_rate == 0)
					vht_cap_ie.mcs_set.tx_high_rate = VHT_HIGH_RATE_BW160_BW8080[VHT_MCS_CAP_8][rx_nss - 1];

				if (rx_nss >= 1)
					vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss1 = VHT_MCS_CAP_9;

				if (rx_nss >= 2)
					vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 = VHT_MCS_CAP_9;

				if (rx_nss >= 3)
					vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss3 = VHT_MCS_CAP_8;

				if (rx_nss >= 4)
					vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss4 = VHT_MCS_CAP_9;

				if (tx_nss >= 1)
					vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss1 = VHT_MCS_CAP_9;

				if (tx_nss >= 2)
					vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss2 = VHT_MCS_CAP_9;

				if (tx_nss >= 3)
					vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss3 = VHT_MCS_CAP_8;

				if (tx_nss >= 4)
					vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss4 = VHT_MCS_CAP_9;
			}
		}

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
		if (wdev->eap.eap_vhtsuprate_en == TRUE) {
			if (rx_nss >= 1)
				vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss1 = wdev->eap.rx_mcs_map.mcs_ss1;

			if (rx_nss >= 2)
				vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 = wdev->eap.rx_mcs_map.mcs_ss2;

			if (rx_nss >= 3)
				vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss3 = wdev->eap.rx_mcs_map.mcs_ss3;

			if (rx_nss >= 4)
				vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss4 = wdev->eap.rx_mcs_map.mcs_ss4;

			if (tx_nss >= 1)
				vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss1 = wdev->eap.tx_mcs_map.mcs_ss1;

			if (tx_nss >= 2)
				vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss2 = wdev->eap.tx_mcs_map.mcs_ss2;

			if (tx_nss >= 3)
				vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss3 = wdev->eap.tx_mcs_map.mcs_ss3;

			if (tx_nss >= 4)
				vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss4 = wdev->eap.tx_mcs_map.mcs_ss4;
		}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

	} else {
		vht_cap_ie.mcs_set.rx_high_rate = 0;
		vht_cap_ie.mcs_set.tx_high_rate = 0;
	}

#ifdef RT_BIG_ENDIAN
	NdisCopyMemory(&tmp_1, &vht_cap_ie.vht_cap, sizeof(VHT_CAP_INFO));
	tmp_1 = SWAP32(tmp_1);
	NdisCopyMemory(&vht_cap_ie.vht_cap, &tmp_1, sizeof(VHT_CAP_INFO));
	NdisCopyMemory(&tmp_2, &vht_cap_ie.mcs_set, sizeof(VHT_MCS_SET));
	tmp_2 = SWAP64(tmp_2);
	NdisCopyMemory(&vht_cap_ie.mcs_set, &tmp_2, sizeof(VHT_MCS_SET));
	/* hex_dump("&vht_cap_ie", &vht_cap_ie,  sizeof(VHT_CAP_IE)); */
	/* SWAP32((UINT32)vht_cap_ie.vht_cap); */
	/* SWAP32((UINT32)vht_cap_ie.mcs_set); */
#endif
	NdisMoveMemory(buf, (UCHAR *)&vht_cap_ie, sizeof(VHT_CAP_IE));
	return sizeof(VHT_CAP_IE);
}

static INT build_vht_op_mode_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	EID_STRUCT eid_hdr;
	OPERATING_MODE operating_mode_ie;
	UCHAR op_vht_bw = wlan_operate_get_vht_bw(wdev);

	NdisZeroMemory(&eid_hdr, sizeof(EID_STRUCT));
	NdisZeroMemory((UCHAR *)&operating_mode_ie,  sizeof(OPERATING_MODE));
	eid_hdr.Eid = IE_OPERATING_MODE_NOTIFY;
	eid_hdr.Len = sizeof(OPERATING_MODE);
	NdisMoveMemory(buf, (UCHAR *)&eid_hdr, 2);
	len = 2;
	operating_mode_ie.rx_nss_type = 0;
	operating_mode_ie.rx_nss = (wlan_operate_get_rx_stream(wdev) - 1);

	if (op_vht_bw == VHT_BW_2040) {
		if (wlan_operate_get_ht_bw(wdev) == HT_BW_40)
			operating_mode_ie.ch_width = 1;
		else
			operating_mode_ie.ch_width = 0;
	} else if (op_vht_bw == VHT_BW_80)
		operating_mode_ie.ch_width = 2;
	else if ((op_vht_bw == VHT_BW_160) ||
			 (op_vht_bw == VHT_BW_8080)) {
		operating_mode_ie.ch_width = 2;
		operating_mode_ie.bw160_bw8080 = 1;
	}

	buf += len;
	NdisMoveMemory(buf, (UCHAR *)&operating_mode_ie, sizeof(OPERATING_MODE));
	len += eid_hdr.Len;
	return len;
}

#ifdef G_BAND_256QAM
static BOOLEAN g_band_256_qam_enable(
	struct _RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if ((pAd->CommonCfg.g_band_256_qam) &&
		(cap->mcs_nss.g_band_256_qam) &&
		(WMODE_CAP(info->phy_mode, WMODE_GN)) &&
		WMODE_CAP_2G(info->wdev->PhyMode))
		info->g_band_256_qam = TRUE;
	else
		info->g_band_256_qam = FALSE;

	return info->g_band_256_qam;
}
#endif /* G_BAND_256QAM */


INT build_vht_ies(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;
	EID_STRUCT eid_hdr;
	UCHAR vht_bw = wlan_operate_get_vht_bw(info->wdev);
	USHORT phy_mode = info->wdev->PhyMode;
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, info->wdev);
	if (WMODE_CAP_AX_6G(phy_mode))
		return len;

	if (
#if defined(G_BAND_256QAM)
		g_band_256_qam_enable(pAd, info) ||
#endif /* G_BAND_256QAM */
		(WMODE_CAP_AC(info->phy_mode) &&
		 WMODE_CAP_5G(info->wdev->PhyMode))
	) {
		NdisZeroMemory(&eid_hdr, sizeof(EID_STRUCT));
		eid_hdr.Eid = IE_VHT_CAP;
		eid_hdr.Len = sizeof(VHT_CAP_IE);
		NdisMoveMemory(info->frame_buf, (UCHAR *)&eid_hdr, 2);
		len = 2;
		len += build_vht_cap_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));

		if ((info->frame_subtype == SUBTYPE_BEACON) ||
			(info->frame_subtype == SUBTYPE_PROBE_RSP) ||
			(info->frame_subtype == SUBTYPE_ASSOC_RSP) ||
			(info->frame_subtype == SUBTYPE_REASSOC_RSP)) {
			eid_hdr.Eid = IE_VHT_OP;
			eid_hdr.Len = sizeof(VHT_OP_IE);
			NdisMoveMemory((UCHAR *)(info->frame_buf + len), (UCHAR *)&eid_hdr, 2);
			len += 2;
#if defined(G_BAND_256QAM)
			vht_bw = (g_band_256_qam_enable(pAd, info)) ? VHT_BW_2040 : vht_bw;
#endif /* G_BAND_256QAM */
			len += build_vht_oper_ie(pAd, vht_bw, info->channel,
					info->wdev, (UCHAR *)(info->frame_buf + len));
		} else if  ((info->frame_subtype == SUBTYPE_ASSOC_REQ) ||
					(info->frame_subtype == SUBTYPE_REASSOC_REQ)) {
			/* optional IE, FOR VHT STA/ApClient with op_mode ie to notify AP
			   and avoid the BW info not sync. Used in Beacon Protection test case
			 */
#ifdef BCN_PROTECTION_SUPPORT
			if (pStaCfg->MlmeAux.ExtCapInfo.operating_mode_notification && (!WMODE_CAP_AX(phy_mode)))
				len += build_vht_op_mode_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
#endif
		}
	}

	return len;
}


static UINT8 ch_offset_abs(UINT8 x, UINT8 y)
{
	if (x > y)
		return x - y;
	else
		return y - x;
}

void update_vht_op_info(UINT8 cap_bw, struct vht_opinfo *vht_op_info, struct _op_info *op_info)
{
	UINT8 p80ccf = vht_op_info->ccfs_0;
	UINT8 s80160ccf = vht_op_info->ccfs_1;

	if (op_info == NULL)
		return;

	/*check op bw should below or equal the cap*/
	if (vht_op_info->ch_width > cap_bw)
		op_info->bw = cap_bw;
	else
		op_info->bw = vht_op_info->ch_width;

	switch (vht_op_info->ch_width) {
	case VHT_BW_2040:
		break;

	case VHT_BW_80:
		if (cap_bw == VHT_BW_80)
			op_info->cent_ch = p80ccf;
		else if (cap_bw > VHT_BW_80) {
			if (s80160ccf == 0)
				op_info->cent_ch = p80ccf;
			else if (ch_offset_abs(s80160ccf, p80ccf) == 8) {
				op_info->bw = VHT_BW_160;
				op_info->cent_ch = s80160ccf;
			} else if (ch_offset_abs(s80160ccf, p80ccf) > 16) {
				op_info->bw = VHT_BW_8080;
				op_info->cent_ch = p80ccf;
			}
		}

		break;

	case VHT_BW_160:
		if (cap_bw == VHT_BW_80)
			op_info->bw = VHT_BW_80;

		op_info->cent_ch = p80ccf;
		break;

	case VHT_BW_8080:
		if (cap_bw == VHT_BW_80)
			op_info->bw = VHT_BW_80;

		op_info->cent_ch = p80ccf;
		break;

	default:
		op_info->bw = VHT_BW_80;
		op_info->cent_ch = p80ccf;
		break;
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO, "bw=%u, cent_ch=%u\n", op_info->bw, op_info->cent_ch);
	return;
}

BOOLEAN vht40_channel_group(RTMP_ADAPTER *pAd, UCHAR channel, struct wifi_dev *wdev)
{
	INT idx = 0;
	UCHAR region = 0;
	UCHAR ch_band = wlan_config_get_ch_band(wdev);

	switch (ch_band) {
	case CMD_CH_BAND_5G:
		region = GetCountryRegionFromCountryCode(pAd->CommonCfg.CountryCode);
		while (vht_ch_40M[idx].ch_up_bnd != 0) {
			if (channel >= vht_ch_40M[idx].ch_low_bnd &&
				channel <= vht_ch_40M[idx].ch_up_bnd) {
				if (((region == CE) &&
					(vht_ch_40M[idx].cent_freq_idx == 142))
				) {
					idx++;
					continue;
				}
				if (!UNII4BandSupport(pAd) &&
					(vht_ch_40M[idx].cent_freq_idx == 167 ||
						vht_ch_40M[idx].cent_freq_idx == 175)) {
							idx++;
							continue;
					}
					return TRUE;
				}
			idx++;
		}
		break;

	case CMD_CH_BAND_6G:
		while (vht_ch_40M_6G[idx].ch_up_bnd != 0) {
			if (channel >= vht_ch_40M_6G[idx].ch_low_bnd &&
				channel <= vht_ch_40M_6G[idx].ch_up_bnd) {
				return TRUE;
			}
			idx++;
		}
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"invalid ch_band %d\n", ch_band);
		break;
	}
	return FALSE;
}

BOOLEAN vht80_channel_group(RTMP_ADAPTER *pAd, UCHAR channel, struct wifi_dev *wdev)
{
	INT idx = 0;
	UCHAR region = GetCountryRegionFromCountryCode(pAd->CommonCfg.CountryCode);
	UCHAR ch_band = wlan_config_get_ch_band(wdev);
	struct vht_ch_layout *vht_ch = get_ch_array(BW_80, ch_band);

	switch (ch_band) {
	case CMD_CH_BAND_5G:
		while (vht_ch && vht_ch[idx].ch_up_bnd != 0) {
			if (channel >= vht_ch[idx].ch_low_bnd &&
				channel <= vht_ch[idx].ch_up_bnd) {
#ifndef DFS_VENDOR10_CUSTOM_FEATURE
				if ((region == CE &&
					vht_ch[idx].cent_freq_idx == 138 &&
					ch_band == CMD_CH_BAND_5G && (strncmp(pAd->CommonCfg.CountryCode, "AU", 2) != 0))) {
#else
				if ((((pAd->CommonCfg.RDDurRegion == JAP ||
					pAd->CommonCfg.RDDurRegion == JAP_W53 ||
					pAd->CommonCfg.RDDurRegion == JAP_W56) &&
					vht_ch_80M[idx].cent_freq_idx == 138)
					||
					((region == JAP || region == CE) &&
					vht_ch_80M[idx].cent_freq_idx == 138))
					&&(!pAd->CommonCfg.bCh144Enabled)) {
#endif
					/* prevent using 132~144 while Region is CE */
					idx++;
					continue;
				}
					if (!UNII4BandSupport(pAd) && (vht_ch_80M[idx].cent_freq_idx == 171)) {
						idx++;
						continue;
					}
				return TRUE;
			}

			idx++;
		}
		break;

	case CMD_CH_BAND_6G:
		while (vht_ch && vht_ch[idx].ch_up_bnd != 0) {
			if (channel >= vht_ch[idx].ch_low_bnd &&
				channel <= vht_ch[idx].ch_up_bnd) {
				return TRUE;
			}
			idx++;
		}
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"invalid ch_band %d\n", ch_band);
		break;
	}

	return FALSE;
}

BOOLEAN CheckWeatherChannel(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR channel)
{
	BOOLEAN result = FALSE;
	UCHAR BandIdx, i, j;
	CHANNEL_CTRL *pChCtrl;
	u8 WeatherCh[3] = {120, 124, 128};
	BandIdx = HcGetBandByWdev(wdev);

	if (channel >= 36 && channel <= 64)
		return TRUE;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	for (i = 0; i < pChCtrl->ChListNum; i++) {
		for (j = 0; j < 3; j++) {
			if (WeatherCh[j] == pChCtrl->ChList[i].Channel) {
				result = TRUE;
				break;
			}
		}
	}
	return result;
}

BOOLEAN vht160_channel_group(RTMP_ADAPTER *pAd, UCHAR channel, struct wifi_dev *wdev)
{
	INT idx = 0;
	UCHAR ch_band = wlan_config_get_ch_band(wdev);
	struct vht_ch_layout *vht_ch = get_ch_array(BW_160, ch_band);

	switch (ch_band) {
	case CMD_CH_BAND_5G:
		/*Weather channel not supported*/
		if (!CheckWeatherChannel(pAd, wdev, channel)) {
			vht_ch = vht_ch_160M_non_weather;
		}

		while (vht_ch && vht_ch[idx].ch_up_bnd != 0) {
			if (channel >= vht_ch[idx].ch_low_bnd &&
				channel <= vht_ch[idx].ch_up_bnd){
				if (!UNII4BandSupport(pAd) && (vht_ch[idx].cent_freq_idx == 163)) {
					idx++;
					continue;
				}
				return TRUE;
			}

			idx++;
		}
		break;

	case CMD_CH_BAND_6G:
		while (vht_ch && vht_ch[idx].ch_up_bnd != 0) {
			if (channel >= vht_ch[idx].ch_low_bnd &&
				channel <= vht_ch[idx].ch_up_bnd) {
				return TRUE;
			}
			idx++;
		}
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"invalid ch_band %d\n", ch_band);
		break;
	}

	return FALSE;
}

void print_vht_op_info(struct vht_opinfo *vht_op)
{
	char *ch_with[] = {"20/40M", "80M", "160M", "80+80M"};

	MTWF_PRINT("VHT Operation Infomation: 0x%02X%02X%02X\n",
			  vht_op->ch_width, vht_op->ccfs_0, vht_op->ccfs_1);
	MTWF_PRINT("     - Channel Width: %u (%s)\n",
			  vht_op->ch_width, ch_with[vht_op->ch_width]);
	MTWF_PRINT("     - Channel Center Frequency Segment 0: %u\n",
			  vht_op->ccfs_0);
	MTWF_PRINT("     - Channel Center Frequency Segment 1: %u\n",
			  vht_op->ccfs_1);
}

/*sta rec vht features decision*/
UINT32 starec_vht_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature)
{
	/* struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER*)wdev->sys_handle; */
	UINT32 features = 0;

	if (CLIENT_STATUS_TEST_FLAG(entry, fCLIENT_STATUS_VHT_CAPABLE))
		features |= STA_REC_BASIC_VHT_INFO_FEATURE;

	/*return value, must use or operation*/
	*feature |= features;
	return TRUE;
}

UCHAR rf_bw_2_vht_bw(UCHAR rf_bw)
{
	UCHAR vht_bw = VHT_BW_2040;

	if (rf_bw == BW_80)
		vht_bw = VHT_BW_80;
	else if (rf_bw == BW_160)
		vht_bw = VHT_BW_160;
	else if (rf_bw == BW_8080)
		vht_bw = VHT_BW_8080;
	else
		vht_bw = VHT_BW_2040;

	return vht_bw;
}

/*
 * build up common VHT IEs
 */
static inline UINT32 set_ch_width_ext_nss_bw_caps(struct wifi_dev *wdev, UINT8 ch_width)
{
	UINT32 cap_info = 0;
	struct mcs_nss_caps *nss_cap = wlan_config_get_mcs_nss_caps(wdev);
	UINT8 band_idx = HcGetBandByWdev(wdev);

	if (nss_cap == NULL)
		return cap_info;

	if (ch_width > VHT_BW_80) {
		if (nss_cap->max_nss[band_idx] == nss_cap->bw160_max_nss) {
			/*support_ch_width_set = 2, ext_nss_bw = 0*/
			cap_info |= (CH_WIDTH_SET_2 << DOT11AC_CAP_SUPPORT_CH_WIDTH_SET_SHIFT);
			wlan_config_set_vht_ext_nss_bw(wdev, 0);
		}
		if ((nss_cap->max_nss[band_idx] / 2) == nss_cap->bw160_max_nss) {
			/*support_ch_width_set = 0, ext_nss_bw = 2*/
			cap_info |= (EXT_NSS_BW_2 << DOT11AC_CAP_EXT_NSS_BW_SHIFT);
			wlan_config_set_vht_ext_nss_bw(wdev, 2);
		}
		if ((nss_cap->max_nss[band_idx] << 1) == nss_cap->bw160_max_nss) {
			/*support_ch_width_set = 2, ext_nss_bw = 3*/
			cap_info |= ((CH_WIDTH_SET_2 << DOT11AC_CAP_SUPPORT_CH_WIDTH_SET_SHIFT)
					| (EXT_NSS_BW_2 << DOT11AC_CAP_EXT_NSS_BW_SHIFT));
			wlan_config_set_vht_ext_nss_bw(wdev, 3);
		}
	}
	/* otherwise: support_ch_width_set = 0, ext_nss_bw = 0 */

	return cap_info;
}

static inline UINT32 set_vht_bf_caps(struct wifi_dev *wdev)
{
	UINT32 cap_info = 0;

	/* TBD */
	/* su beamformer */
	/* su beamformee */
	/* beamformee sts */
	/* num of sounding dim. */
	/* mu beamformer */
	/* mu beamformee */
	return cap_info;
}

static inline UINT16 set_vht_mcs_map(UINT8 stream, UINT8 max_mcs)
{
	UINT16 mcs_map = 0xFFFF;

	mcs_map &= ~(DOT11AC_MCS_1SS_MASK);
	mcs_map |= VHT_MAX_MCS_NSS(1, max_mcs);
	if (stream > 1) {
		mcs_map &= ~(DOT11AC_MCS_2SS_MASK);
		mcs_map |= VHT_MAX_MCS_NSS(2, max_mcs);
	}
	if (stream > 2) {
		mcs_map &= ~(DOT11AC_MCS_3SS_MASK);
		mcs_map |= VHT_MAX_MCS_NSS(3, max_mcs);
	}
	if (stream > 3) {
		mcs_map &= ~(DOT11AC_MCS_4SS_MASK);
		mcs_map |= VHT_MAX_MCS_NSS(4, max_mcs);
	}
	if (stream > 4) {
		mcs_map &= ~(DOT11AC_MCS_5SS_MASK);
		mcs_map |= VHT_MAX_MCS_NSS(5, max_mcs);
	}
	if (stream > 5) {
		mcs_map &= ~(DOT11AC_MCS_6SS_MASK);
		mcs_map |= VHT_MAX_MCS_NSS(6, max_mcs);
	}
	if (stream > 6) {
		mcs_map &= ~(DOT11AC_MCS_7SS_MASK);
		mcs_map |= VHT_MAX_MCS_NSS(7, max_mcs);
	}
	if (stream > 7) {
		mcs_map &= ~(DOT11AC_MCS_8SS_MASK);
		mcs_map |= VHT_MAX_MCS_NSS(8, max_mcs);
	}

	return mcs_map;
}

static UINT8 *build_vht_caps_info(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	UINT8 rx_ldpc = wlan_config_get_vht_ldpc(wdev);
	UINT8 sgi = wlan_config_get_vht_sgi(wdev);
	UINT8 stbc = wlan_config_get_vht_stbc(wdev);
	UINT8 support_ch_width = wlan_config_get_vht_bw(wdev);
	struct ppdu_caps *mpdu = wlan_config_get_ppdu_caps(wdev);
	UINT32 cap_info = 0;

	/* max_mpdu_length */
	cap_info |= (mpdu->max_mpdu_len & DOT11AC_CAP_MAX_MPDU_LEN_MASK);
	/* support_ch_width_set */
	cap_info |= set_ch_width_ext_nss_bw_caps(wdev, support_ch_width);
	/* rx_ldpc */
	if (rx_ldpc)
		cap_info |= DOT11AC_CAP_RX_LDPC;
	/* bw80/bw160_8080 sgi */
	if (sgi) {
		cap_info |= DOT11AC_CAP_BW80_SGI;
		if (support_ch_width > VHT_BW_80)
			cap_info |= DOT11AC_CAP_BW160_8080_SGI;
	}
	/* tx/rx stbc */
	if (stbc)
		cap_info |= (DOT11AC_CAP_TX_STBC | (1 << DOT11AC_CAP_RX_STBC_SHIFT));
	/* bf */
	cap_info |= set_vht_bf_caps(wdev);
	/* max ampdu length exp */
	cap_info |= (mpdu->vht_max_ampdu_len_exp << DOT11AC_CAP_MAX_AMPDU_LEN_EXP_SHIFT);

	cap_info = cpu_to_le32(cap_info);
	NdisMoveMemory(f_buf, (UINT8 *)&cap_info, sizeof(cap_info));
	pos += sizeof(cap_info);

	return pos;
}

static UINT8 *build_vht_mcs_nss_set(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;
	struct vht_txrx_mcs_nss support_mcs_nss;
	UINT8 tx_nss = wlan_config_get_tx_stream(wdev);
	UINT8 rx_nss = wlan_config_get_rx_stream(wdev);
	UINT8 max_mcs = VHT_MCS_9;
	UINT16 rx_mcs_map = 0;
	UINT16 tx_mcs_map = 0;
	UINT16 rx_rate_max_nsts = 0;
	UINT16 tx_rate_ext_nss_bw = 0;
	struct mcs_nss_caps *mcs_nss = wlan_config_get_mcs_nss_caps(wdev);

	if (mcs_nss->max_vht_mcs < max_mcs)
		max_mcs = mcs_nss->max_vht_mcs;
	if (tx_nss > 4)
		tx_nss = 4;
	if (rx_nss > 4)
		rx_nss = 4;

	rx_mcs_map = set_vht_mcs_map(rx_nss, max_mcs);
	rx_rate_max_nsts = VHT_HIGH_RATE_BW80[max_mcs][rx_nss - 1];
	tx_mcs_map = set_vht_mcs_map(tx_nss, max_mcs);
	tx_rate_ext_nss_bw = VHT_HIGH_RATE_BW80[max_mcs][tx_nss - 1];
	/* default support interpreting ext_nss_bw */
	tx_rate_ext_nss_bw |= DOT11AC_MCS_EXT_NSS_BW_CAPABLE;

	support_mcs_nss.rx.mcs_map = cpu_to_le16(rx_mcs_map);
	support_mcs_nss.rx_gi_data_rate_max_nsts = cpu_to_le16(rx_rate_max_nsts);
	support_mcs_nss.tx.mcs_map = cpu_to_le16(tx_mcs_map);
	support_mcs_nss.tx_gi_data_rate_ext_nss_bw = cpu_to_le16(tx_rate_ext_nss_bw);
	NdisMoveMemory(f_buf, (UINT8 *)&support_mcs_nss, sizeof(support_mcs_nss));
	pos += sizeof(support_mcs_nss);

	return pos;
}

static UINT8 *build_vht_caps_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = f_buf;

	pos = build_vht_caps_info(wdev, pos);
	pos = build_vht_mcs_nss_set(wdev, pos);

	return pos;
}

UINT8 *build_vht_op_info(struct wifi_dev *wdev, UINT8 *f_buf)
{
	struct vht_opinfo vht_op_info;
	UINT8 prim_ch = wdev->channel;
	UINT8 cent_ch_1 = wlan_operate_get_cen_ch_1(wdev);
	UINT8 cent_ch_2 = 0;
	UINT8 vht_bw = wlan_operate_get_vht_bw(wdev);
	UINT8 tx_nss = wlan_config_get_tx_stream(wdev);
	struct mcs_nss_caps *mcs_nss = wlan_config_get_mcs_nss_caps(wdev);
	UINT8 nss_diff = 0;
	UINT8 *pos = f_buf;

	NdisZeroMemory((UINT8 *)&vht_op_info, sizeof(vht_op_info));
	nss_diff = (tx_nss == mcs_nss->bw160_max_nss) ? 0 : 1;
	vht_op_info.ch_width = 1;
	if (vht_bw < VHT_BW_80)
		vht_op_info.ch_width = 0;
	vht_op_info.ccfs_0 = cent_ch_1;
	if (vht_bw == VHT_BW_160) {
		vht_op_info.ccfs_0 = GET_BW160_PRIM80_CENT(prim_ch, cent_ch_1);
		vht_op_info.ccfs_1 = cent_ch_1;
		if (nss_diff) {
			vht_op_info.ccfs_1 = 0;
			/* TBD:	update_ht_op_ccfs2(cent_ch_1);*/
		}
	}
	if (vht_bw == VHT_BW_8080) {
		cent_ch_2 = wlan_operate_get_cen_ch_2(wdev);
		vht_op_info.ccfs_1 = cent_ch_2;
		if (nss_diff) {
			vht_op_info.ccfs_1 = 0;
			/* TBD:	update_ht_op_ccfs2(cent_ch_2);*/
		}
	}
	NdisMoveMemory(pos, (UINT8 *)&vht_op_info, sizeof(vht_op_info));
	pos += sizeof(vht_op_info);

	return pos;
}

static UINT8 *build_vht_op_ie(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *pos = build_vht_op_info(wdev, f_buf);
	pos = build_vht_mcs_nss_set(wdev, pos);

	return pos;
}

/*
 * Build up VHT IEs for AP
 */
UINT32 add_beacon_vht_ies(struct wifi_dev *wdev, UINT8 *f_buf)
{
	UINT8 *l_buf = f_buf;
	UINT32 offset = 0;

	l_buf = build_vht_caps_ie(wdev, l_buf);
	l_buf = build_vht_op_ie(wdev, l_buf);
	offset = (UINT32)(l_buf - f_buf);

	return offset;
}

#endif /* DOT11_VHT_AC */
