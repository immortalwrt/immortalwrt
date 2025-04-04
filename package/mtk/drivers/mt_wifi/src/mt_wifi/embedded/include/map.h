
#ifndef __MAP_H__
#define __MAP_H__

#include "rtmp_type.h"

#define ASSOC_REQ_LEN       512
#define BLOCK_LIST_NUM      128
#define VENDOR_SPECIFIC_LEN 128
#define MAX_BH_PROFILE_CNT    4
#define VEND_IE_TYPE 221

#ifdef MAP_R2
#define ASSOC_REQ_LEN_R2    512
#define INVALID_VLAN_ID		4095
#endif

/* For rssi steering*/
#define RCPI_TOLLENACE       8 /* 2dB */

#define IS_MAP_ENABLE(pAd) \
	((pAd->MAPMode != MAP_DISABLED) && (pAd->MAPMode != MAP_BS_2_0))

#define IS_MAP_TURNKEY_ENABLE(pAd) \
	((pAd->MAPMode == MAP_TURNKEY))

#define IS_MAP_BS_ENABLE(pAd) \
	((pAd->MAPMode == MAP_BS_2_0))

#define IS_MAP_API_ENABLE(pAd) \
	((pAd->MAPMode == MAP_API_MODE))

#define IS_MAP_CERT_ENABLE(pAd) \
	((pAd->MAPMode == MAP_CERT_MODE))

#ifdef MAP_R2
#define IS_MAP_R2_ENABLE(pAd) \
		(pAd->bMapR2Enable == TRUE)
#endif

#ifdef MAP_R3
#define IS_MAP_R3_ENABLE(pAd) \
		(pAd->bMapR3Enable == TRUE)
#endif

#ifdef MAP_R4
#define IS_MAP_R4_ENABLE(pAd) \
		(pAd->bMapR4Enable == TRUE)
#endif


#ifdef MAP_TS_TRAFFIC_SUPPORT
#define IS_VALID_VID(vid) \
	((vid) && (vid != INVALID_VLAN_ID))
#endif

typedef enum {
	BELOW_THRESHOLD = 0,
	ABOVE_THRESHOLD,
} RSSI_STATUS;

typedef enum {
	AGENT_INIT_STEER_DISALLOW = 0,
	AGENT_INIT_RSSI_STEER_MANDATE,
	AGENT_INIT_RSSI_STEER_ALLOW,
} STEERING_POLICY;

struct GNU_PACKED map_policy_setting {
	unsigned char steer_policy;
	unsigned char cu_thr;
	unsigned char rcpi_thr;
};
#define MAX_PROFILE_CNT 4
/*
struct scan_SSID {
	char ssid[32 + 1];
	unsigned char SsidLen;
};

struct GNU_PACKED scan_BH_ssids
{
	unsigned long scan_cookie;
	unsigned char scan_channel_count;
	unsigned char scan_channel_list[32];
	unsigned char profile_cnt;
	struct scan_SSID scan_SSID_val[MAX_PROFILE_CNT];
};
*/

#ifdef DFS_CAC_R2
struct GNU_PACKED cac_opcap
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[16];
	USHORT cac_time[16];
	unsigned int last_cac_time[16];
	USHORT non_occupancy_remain[16];
};

struct GNU_PACKED cac_capability_lib
{
	unsigned char country_code[2];
	unsigned char rdd_region;
	unsigned char op_class_num;
	struct cac_opcap opcap[16];
	unsigned char active_cac;
	unsigned char ch_num;
	unsigned int remain_time;
	unsigned char cac_mode;
};
#endif

#define MAP_DISABLED		0
#define MAP_TURNKEY			1
#define MAP_BS_2_0			2
#define MAP_API_MODE		3
#define MAP_CERT_MODE		4

typedef struct _MAP_CONFIG {
	/*Support Unassociated STA link metric report on current operating Bss*/
	BOOLEAN bUnAssocStaLinkMetricRptOpBss;
	/*Support Unassociated STA link metric report on currently non operating Bss */
	BOOLEAN bUnAssocStaLinkMetricRptNonOpBss;
	/*Support Agent-initiated Rssi-based steering */
	BOOLEAN bAgentInitRssiSteering;
	UCHAR DevOwnRole;
	UCHAR vendor_ie_buf[VENDOR_SPECIFIC_LEN];
	UCHAR vendor_ie_len;
	struct scan_BH_ssids scan_bh_ssids;
	BOOLEAN FireProbe_on_DFS;
#ifdef MAP_R2
	UINT16 primary_vid;
	UCHAR primary_pcp;
	UCHAR vid_num;
	UINT32 vids[128];
	UINT16 fh_vid;
	UINT32 bitmap_trans_vlan[128];
#endif
} MAP_CONFIG, *PMAP_CONFIG;

/* spec v171027 */
enum MAPRole {
	MAP_ROLE_TEARDOWN = 4,
	MAP_ROLE_FRONTHAUL_BSS = 5,
	MAP_ROLE_BACKHAUL_BSS = 6,
	MAP_ROLE_BACKHAUL_STA = 7,
};

#define NON_PREF 0
#define PREF_SCORE_1    BIT(4)
#define PREF_SCORE_2    BIT(5)
#define PREF_SCORE_3    (BIT(4)|BIT(5))
#define PREF_SCORE_4    BIT(6)
#define PREF_SCORE_5    (BIT(4)|BIT(6))
#define PREF_SCORE_6    (BIT(5)|BIT(6))
#define PREF_SCORE_7    (BIT(4)|BIT(5)|BIT(6))
#define PREF_SCORE_8    (BIT(7))
#define PREF_SCORE_9    (BIT(4)|BIT(7))
#define PREF_SCORE_10   (BIT(5)|BIT(7))
#define PREF_SCORE_11   (BIT(4)|BIT(5)|BIT(6))
#define PREF_SCORE_12   (BIT(6)|BIT(7))
#define PREF_SCORE_13   (BIT(4)|BIT(6)|BIT(7))
#define PREF_SCORE_14   (BIT(5)|BIT(6)|BIT(7))

#define UNSPECIFICIED 0
#define NON80211_INTERFERER_IN_LOCAL_ENV            BIT(0)
#define INTRA_NETWORK_80211_OBSS_INTERFERENCE       BIT(1)
#define ENTERNAL_NETWORK_80211_OBSS_INTERFERENCE    (BIT(0)|BIT(1))
#define REDUCED_COVERAGE                            BIT(2)
#define REDUCED_TP                                  (BIT(0)|BIT(2))
#define INDEVICE_INTERFERER                         (BIT(1)|BIT(2))
#define OP_DISALLOWED_DUE_TO_DFS                    (BIT(0)|BIT(1)|BIT(2))
#define OP_PREVENT_BACKHAUL_OP                      (BIT(3))
#define IMMEDIATE_OP_POSSIBLE_ON_DFS_CHN            (BIT(0)|BIT(3))
#define DFS_CHN_STATE_UNKNOWN                       (BIT(1)|BIT(3))

extern UCHAR multicast_mac_1905[MAC_ADDR_LEN];

VOID MAP_Init(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN IN UCHAR wdev_type
);

VOID MAP_InsertMapCapIE(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen
);

INT MAP_InsertMapWscAttr(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	OUT PUCHAR pFrameBuf
);

UINT32 map_rc_get_band_idx_by_chan(PRTMP_ADAPTER pad, UCHAR channel);

BOOLEAN map_check_cap_ie(
	IN PEID_STRUCT   eid,
	OUT  unsigned char *cap
#ifdef MAP_R2
	, OUT UCHAR *profile,
	OUT UINT16 *vid
#endif
);

UCHAR getNonOpChnNum(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR op_class
);

UCHAR getAutoChannelSkipListNum(
		IN PRTMP_ADAPTER pAd,
		IN struct wifi_dev *wdev
);

VOID setNonOpChnList(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN PCHAR nonOpChnList,
	IN UCHAR op_class,
	IN UCHAR nonOpChnNum
);

VOID setAutoChannelSkipList(
		IN PRTMP_ADAPTER pAd,
		IN struct wifi_dev *wdev,
		IN wdev_chn_info * chn_list
);

INT map_send_bh_sta_wps_done_event(
	IN PRTMP_ADAPTER adapter,
	IN struct _MAC_TABLE_ENTRY *mac_entry,
	IN BOOLEAN is_ap
);

VOID map_rssi_status_check(
	IN PRTMP_ADAPTER pAd);

INT ReadSRMeshUlModeParameterFromFile(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *pBuffer);

INT ReadMapParameterFromFile(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *pBuffer);

void scan_extra_probe_req(
	PRTMP_ADAPTER pAd,
	UCHAR OpMode,
	UCHAR ScanType,
	struct wifi_dev *wdev,
	UCHAR *desSsid,
	UCHAR desSsidLen);

#ifdef MAP_BL_SUPPORT
/* BS2.0 Blacklisting Support */
typedef struct _BS_BLACKLIST_ENTRY {
	struct _BS_BLACKLIST_ENTRY *pNext;
	UCHAR addr[MAC_ADDR_LEN];
} BS_BLACKLIST_ENTRY, *PBS_BLACKLIST_ENTRY;

VOID map_blacklist_add(
	IN  PLIST_HEADER pBlackList,
	IN  PUCHAR pMacAddr);

VOID map_blacklist_del(
	IN  PLIST_HEADER pBlackList,
	IN  PUCHAR pMacAddr);

VOID map_blacklist_show(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apidx);

BOOLEAN map_is_entry_bl(
	PRTMP_ADAPTER pAd,
	UCHAR *pAddr,
	UCHAR apidx);
#endif /*  MAP_BL_SUPPORT */

#ifdef A4_CONN
BOOLEAN map_a4_peer_enable(
	IN PRTMP_ADAPTER adapter,
	IN struct _MAC_TABLE_ENTRY *entry,
	IN BOOLEAN is_ap
);

BOOLEAN map_a4_peer_disable(
	IN PRTMP_ADAPTER adapter,
	IN struct _MAC_TABLE_ENTRY *entry,
	IN BOOLEAN is_ap
);

BOOLEAN map_a4_init(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap
);

BOOLEAN map_a4_deinit(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap
);
#endif /*A4_CONN*/
BOOLEAN MapNotRequestedChannel(struct wifi_dev *wdev, unsigned char channel);
int map_make_vend_ie(IN PRTMP_ADAPTER pAd, IN UCHAR ApIdx);
#ifdef MAP_TS_TRAFFIC_SUPPORT
BOOLEAN map_ts_tx_process(RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					PNDIS_PACKET pkt, struct _MAC_TABLE_ENTRY *peer_entry);
BOOLEAN map_ts_rx_process(RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					PNDIS_PACKET pkt, struct _MAC_TABLE_ENTRY *peer_entry);
#endif
#ifdef MAP_R2
UINT32 is_vid_configed(UINT16 vid, UINT32 vids[]);
#endif
#endif

