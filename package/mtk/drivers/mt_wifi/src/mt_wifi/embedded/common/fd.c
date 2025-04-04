#include "rt_config.h"
#include "action.h"
#include "dot11ai_fd.h"

#ifdef CONFIG_6G_SUPPORT

static UCHAR *add_fd_capability(UCHAR *frm_buf, struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _BSS_STRUCT *bss = (struct _BSS_STRUCT *)wdev->func_dev;
	UCHAR *pos = frm_buf;
	UCHAR dbdc_idx = HcGetBandByWdev(wdev);
	UCHAR ht_bw = wlan_operate_get_ht_bw(wdev);
	UCHAR vht_bw = wlan_operate_get_vht_bw(wdev);
	UCHAR nss = wlan_config_get_tx_stream(wdev);
	UCHAR bss_op_width = FD_CAP_BSS_OP_CH_BW20;
	UINT16 fd_cap = 0;
	UINT8 phy_idx = 0;

	/*BSS Operating Channel Width*/
	if (ht_bw == BW_40 && vht_bw == VHT_BW_2040)
		bss_op_width = FD_CAP_BSS_OP_CH_BW40;
	else if (ht_bw == BW_40 && vht_bw == VHT_BW_80)
		bss_op_width = FD_CAP_BSS_OP_CH_BW80;
	else if (ht_bw == BW_40 && vht_bw == VHT_BW_160)
		bss_op_width = FD_CAP_BSS_OP_CH_BW160;
	else if (ht_bw == BW_40 && vht_bw == VHT_BW_8080)
		bss_op_width = FD_CAP_BSS_OP_CH_BW8080;
	fd_cap |= DOT11AI_FD_CAP_SET_BSS_OP_CH_WIDTH(bss_op_width);
	/*Max. NSS*/
	fd_cap |= DOT11AI_FD_CAP_SET_MAX_NSS(nss);
	/*Multi-BSSID presence*/
	if (IS_MBSSID_IE_NEEDED(ad, bss, dbdc_idx))
		fd_cap |= DOT11AI_FD_CAP_MBSSID_PRESENCE;
	/*PHY Index*/
	if (WMODE_CAP_AX(wdev->PhyMode))
		phy_idx = FD_CAP_PHY_IDX_HE;
	else if (WMODE_CAP_AC(wdev->PhyMode))
		phy_idx = FD_CAP_PHY_IDX_VHT;
	else if (WMODE_CAP_N(wdev->PhyMode))
		phy_idx = FD_CAP_PHY_IDX_HT;
	else if (WMODE_CAP(wdev->PhyMode, WMODE_B))
		phy_idx = FD_CAP_PHY_IDX_HR_DSSS;
	else
		phy_idx = FD_CAP_PHY_IDX_ERP_OFDM;
	fd_cap |= DOT11AI_FD_CAP_SET_PHY_IDX(phy_idx);
	/*FILS min. rate*/
	fd_cap |= DOT11AI_FD_CAP_SET_MIN_RATE(FD_CAP_FILS_MIN_RATE_0);

	NdisMoveMemory(pos, (UCHAR *)&fd_cap, sizeof(fd_cap));
	pos += sizeof(fd_cap);

	return pos;
}

static UCHAR *add_fd_ssid_or_short_ssid(UCHAR *frm_buf, struct wifi_dev *wdev, UINT8 use_short_ssid)
{
	struct _BSS_STRUCT *bss = (struct _BSS_STRUCT *)wdev->func_dev;
	UCHAR *pos = frm_buf;
	UCHAR *ssid = bss->Ssid;
	UINT16 ssid_len = bss->SsidLen;

	if (use_short_ssid) {
		/*short-ssid*/
		ssid = (UCHAR *)&bss->ShortSSID;
		ssid_len = sizeof(bss->ShortSSID);
	}
	NdisMoveMemory(pos, ssid, ssid_len);
	pos += ssid_len;

	return pos;
}

static UCHAR *add_fd_ccfs1(UCHAR *frm_buf, struct wifi_dev *wdev)
{
	UCHAR *pos = frm_buf;

	return pos;
}

static UCHAR *add_fd_prim_ch(UCHAR *frm_buf, struct wifi_dev *wdev)
{
	UCHAR *pos = frm_buf;
	UCHAR prim_ch = wlan_operate_get_prim_ch(wdev);

	NdisMoveMemory(pos, (UCHAR *)&prim_ch, sizeof(prim_ch));
	pos += sizeof(prim_ch);

	return pos;
}

static UCHAR *add_fd_op_class(UCHAR *frm_buf, struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	UCHAR *pos = frm_buf;
	UCHAR prim_ch = wlan_operate_get_prim_ch(wdev);
	UCHAR op_class = get_regulatory_class(ad, prim_ch, wdev->PhyMode, wdev);

	NdisMoveMemory(pos, (UCHAR *)&op_class, sizeof(op_class));
	pos += sizeof(op_class);

	return pos;
}

static ULONG add_fd_info_field(UCHAR *frm_buf, struct wifi_dev *wdev)
{
	struct _BSS_STRUCT *bss = (struct _BSS_STRUCT *)wdev->func_dev;
	struct fils_discovery_info fd_disc_info = {0};
	UCHAR *pos = frm_buf;
	ULONG len = 0;
	UINT16 frm_ctrl = 0;
	UINT8 use_s_ssid = 0;
	UINT8 ssid_len = bss->SsidLen - 1;

	/*fils discovery frame control*/
	frm_ctrl |= (DOT11AI_FD_FRM_CTRL_CAP_PRESENCE
			| DOT11AI_FD_FRM_CTRL_PRIM_CH_PRESENCE);

	if (frm_ctrl & DOT11AI_FD_FRM_CTRL_SHORT_SSID_INDC) {
		use_s_ssid = 1;
		ssid_len = 3;
	}
	frm_ctrl |= (ssid_len & DOT11AI_FD_FRM_CTRL_SSID_LEN_MASK);
	fd_disc_info.fils_disc_frm_ctrl = cpu2le16(frm_ctrl);

	/*timestamp: n/a*/

	/*beacon interval*/
	fd_disc_info.bcn_interval = cpu2le16(wdev->bss_info_argument.bcn_period);
	NdisMoveMemory(pos, (UCHAR *)&fd_disc_info, sizeof(fd_disc_info));
	pos += sizeof(fd_disc_info);

	/*ssid / short-ssid*/
	pos = add_fd_ssid_or_short_ssid(pos, wdev, use_s_ssid);

	/*Length*/

	/*fd cap presence*/
	if (frm_ctrl & DOT11AI_FD_FRM_CTRL_CAP_PRESENCE)
		pos = add_fd_capability(pos, wdev);

	/*Primary Channel*/
	if (frm_ctrl & DOT11AI_FD_FRM_CTRL_PRIM_CH_PRESENCE) {
		pos = add_fd_op_class(pos, wdev);
		pos = add_fd_prim_ch(pos, wdev);
	}

	/*AP CSN*/
	/*Access Network Options*/
	/*FD RSN Information*/

	/*CCFS1*/
	if (frm_ctrl & DOT11AI_FD_FRM_CTRL_CCFS1_PRESENCE)
		pos = add_fd_ccfs1(pos, wdev);

	/*Mobility Domain*/

	len = pos - frm_buf;

	return len;
}

ULONG build_fils_discovery_action(struct wifi_dev *wdev, UCHAR *frm_buf)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	FRAME_ACTION_HDR act_frm;
	ULONG frm_len = sizeof(act_frm);
	UCHAR *pos = frm_buf;

	ActHeaderInit(ad, &act_frm.Hdr, BROADCAST_ADDR, wdev->if_addr, wdev->bssid);
	/*Category*/
	act_frm.Category = CATEGORY_PUBLIC;
	/*Public Action*/
	act_frm.Action = ACTION_FILS_DISCOVERY;
	NdisMoveMemory(pos, (UINT8 *)&act_frm, sizeof(act_frm));
	pos += sizeof(act_frm);

	/*FILS Discovery Information*/
	frm_len += add_fd_info_field(pos, wdev);
	/*Reduced Neighbor Report Element (optional)*/
	frm_len += add_he_6g_rnr_ie(wdev, frm_buf, frm_len, 0);
	frm_len += add_fils_tpe_ie(wdev, frm_buf, frm_len);

	/*FILS Indication Element (optional)*/
	/*Roaming Consortium Element (optional)*/
	/*Vendor Specific Element (optioanl)*/

	return frm_len;
}

#endif /* CONFIG_6G_SUPPORT */

