/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	wapp.h
*/
#include "rt_config.h"

#ifndef _WAPP_H_
#define __WAPP_H__

#ifdef WAPP_SUPPORT
struct wapp_req;
struct wapp_event;
struct _wdev_op_class_info;
struct _wdev_chn_info;

typedef enum {
	INACTIVE = 0,
	ACTIVE,
} STA_STATUS;

INT wapp_event_handle(PRTMP_ADAPTER pAd, struct wapp_req *req);

INT wapp_send_cli_join_event(PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *mac_entry);

INT wapp_send_cli_leave_event(PRTMP_ADAPTER pAd, UINT32 ifindex,
			      UCHAR *mac_addr, MAC_TABLE_ENTRY *mac_entry);

INT wapp_send_cli_probe_event(PRTMP_ADAPTER pAd, UINT32 ifindex,
			      UCHAR *mac_addr, MLME_QUEUE_ELEM *elem);

BOOLEAN wapp_init(PRTMP_ADAPTER pAd, BSS_STRUCT *pMbss);

VOID wext_send_wapp_qry_rsp(PNET_DEV pNetDev, struct wapp_event *event);

INT wapp_send_bss_state_change(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev,
			       UINT8 bss_state);

INT wapp_send_ch_change_rsp(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UINT8 ch);

INT wapp_send_apcli_association_change(UINT8 apcli_assoc_state,
				       struct _RTMP_ADAPTER *ad,
				       struct _APCLI_STRUCT *ApCliEntry);

#ifdef CONVERTER_MODE_SWITCH_SUPPORT

INT wapp_send_apcli_association_change_vendor10(
	UINT8 apcli_assoc_state, struct _RTMP_ADAPTER *ad,
	struct _APCLI_STRUCT *ApCliEntry);

#endif /* CONVERTER_MODE_SWITCH_SUPPORT */

INT wapp_send_bssload_crossing(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev,
			       UCHAR bssload_high_thrd, UCHAR bssload_low_thrd,
			       UCHAR bssload);

VOID wapp_send_bcn_report(IN PRTMP_ADAPTER pAd, IN PMAC_TABLE_ENTRY pEntry,
			  IN PUCHAR pFramePtr, IN ULONG MsgLen);

VOID wapp_send_bcn_report_complete(IN PRTMP_ADAPTER pAd,
				   IN PMAC_TABLE_ENTRY pEntry);

#ifdef AIR_MONITOR
VOID wapp_send_air_mnt_rssi(IN PRTMP_ADAPTER pAd, IN PMAC_TABLE_ENTRY pEntry,
			    IN PMNT_STA_ENTRY pMntEntry);
#endif

#ifdef CONFIG_MAP_SUPPORT
VOID wapp_send_cac_period_event(IN PRTMP_ADAPTER pAd, IN UINT32 ifindex,
				IN UCHAR channel, IN UCHAR cac_enable,
				IN USHORT cac_time);
#endif
VOID wapp_bss_load_check(struct _RTMP_ADAPTER *ad);

VOID wapp_send_csa_event(IN PRTMP_ADAPTER pAd, IN UINT32 ifindex,
			 IN UCHAR new_channel);

VOID wapp_send_cli_active_change(IN PRTMP_ADAPTER pAd,
				 IN PMAC_TABLE_ENTRY pEntry,
				 IN STA_STATUS stat);

INT set_wapp_param(IN PRTMP_ADAPTER pAd, UINT32 Param, UINT32 Value);

INT wapp_set_ap_ie(IN PRTMP_ADAPTER pAd, IN RTMP_STRING *IE, IN UINT32 IELen,
		   IN UCHAR ApIdx);

INT wapp_send_sta_connect_rejected(struct _RTMP_ADAPTER *ad,
				   struct wifi_dev *wdev, UCHAR *sta_mac_addr,
				   UCHAR *bssid, UINT8 connect_stage,
				   UINT16 reason, USHORT status_code,
				   USHORT reason_code);

INT wapp_send_wsc_scan_complete_notification(PRTMP_ADAPTER pAd,
					     struct wifi_dev *wdev);
INT wapp_send_wsc_eapol_start_notification(PRTMP_ADAPTER pAd,
					   struct wifi_dev *wdev);
INT wapp_send_wsc_eapol_complete_notif(PRTMP_ADAPTER pAd,
				       struct wifi_dev *wdev);
#ifdef CONFIG_MAP_SUPPORT
INT wapp_send_scan_complete_notification(PRTMP_ADAPTER pAd,
					 struct wifi_dev *wdev);
#endif
VOID wapp_send_cac_stop(IN PRTMP_ADAPTER pAd, IN UINT32 ifindex,
			IN UCHAR channel, IN UCHAR ret);
#ifdef DFS_CAC_R2
void wapp_get_cac_cap(IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev,
		      struct cac_capability_lib *cac_cap);
#endif

#ifdef A4_CONN
INT wapp_send_a4_entry_missing(PRTMP_ADAPTER pAd, UINT32 ifindex, UCHAR *ip);
#endif
UINT8 get_channel_utilization(PRTMP_ADAPTER pAd, u32 ifindex);
UCHAR map_set_op_class_info(PRTMP_ADAPTER pAd, struct wifi_dev *wdev,
			    struct _wdev_op_class_info *op_class);

VOID setChannelList(PRTMP_ADAPTER pAd, struct wifi_dev *wdev,
		    struct _wdev_chn_info *chn_list);
#ifdef MAP_R2
INT wapp_send_sta_disassoc_stats_event(PRTMP_ADAPTER pAd,
				       MAC_TABLE_ENTRY *pEntry, USHORT reason);

void wapp_handle_sta_disassoc(PRTMP_ADAPTER pAd, UINT16 wcid, UINT16 Reason);

#endif

INT wapp_send_radar_detect_notif(PRTMP_ADAPTER pAd, struct wifi_dev *wdev,
				 unsigned char channel,
				 unsigned char ch_status);
void wapp_prepare_nop_channel_list(PRTMP_ADAPTER pAd,
				   struct nop_channel_list_s *nop_list);
#ifdef DPP_SUPPORT
void cache_dpp_frame_rx_event(struct wifi_dev *wdev, const char *peer_mac_addr,
			      UINT channel, const char *frm, UINT16 frm_len,
			      BOOL is_gas, UINT32 frm_count);
void wext_send_dpp_frame_rx_event(struct wifi_dev *wdev, UINT32 frm_count);
void wext_send_dpp_action_frame(PRTMP_ADAPTER pAd, struct wifi_dev *wdev,
				const char *peer_mac_addr, UINT channel,
				const char *frm, UINT16 frm_len,
				BOOLEAN is_gas);
void wext_send_dpp_frame_tx_status(PRTMP_ADAPTER pAd, struct wifi_dev *wdev,
				   BOOLEAN tx_error, UINT16 seq_no);

#endif /* DPP_SUPPORT */
#ifdef MAP_R2
VOID Update_Mib_Bucket_for_map(RTMP_ADAPTER *pAd);
#endif
#endif /* WAPP_SUPPORT */
#endif /* _WAPP_H_ */
