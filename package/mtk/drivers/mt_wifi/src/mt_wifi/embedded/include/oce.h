#ifndef __OCE_H
#define __OCE_H

#ifdef OCE_SUPPORT
#include "rtmp_type.h"
#include "rt_config.h"

#define OCE_ATTR_MAX_LEN                252 /* ????? */
#define OCE_SCAN_11BOCEAP_PERIOD_TIME   180000 /* 180 sec */
#define OCE_RELEASE                     0x1 /* OCE release = 1 */
#define IS_STA_CFON                     0x0 /* IS not STA CFON */
#define HLP_ENABLED                     0x1 /* HLP_ENABLED */

#define OCE_RELEASE_MASK                BITS(0, 2)
#define OCE_RELEASE_OFFSET              0
#define OCE_IS_STA_CFON_MASK            BIT(3)
#define OCE_IS_STA_CFON_OFFSET          3
#define OCE_11B_ONLY_PRESENT_MASK       BIT(4)
#define OCE_11B_ONLY_PRESENT_OFFSET     4
#define OCE_HLP_ENABLED_MASK            BIT(5)
#define OCE_HLP_ENABLED_OFFSET          5
#define OCE_NONOCE_PRESENT_MASK         BIT(6)
#define OCE_NONOCE_PRESENT_OFFSET       6
#define OCE_AVAILABLE_CAP_MASK          BITS(0, 3)
#define OCE_AVAILABLE_CAP_OFFSET        4
#define OCE_RNR_IE_LEN 17
#define OCE_FD_FRAME_PERIOD 20 /* unit: ms */
#define OCE_RNR_SCAN_PERIOD 120000 /* unit: ms */
#define OCE_RNR_UPDATE_TIME_LOG 2000 /* unit: ms */

#define OCE_GET_CONTROL_FIELD(_OceCapIndication, _mask, _offset)	\
	(((_OceCapIndication) & (_mask)) >> (_offset))

#define OCE_SET_CONTROL_FIELD(_OceCapIndication, _value, _mask, _offset)	\
{ \
	(_OceCapIndication) &= ~(_mask); \
	(_OceCapIndication) |= (((_value) << (_offset)) & (_mask)); \
}

#define OCE_GET_DOWNLINK_AVAILABLE_CAP(_AvailableCap, _mask, _offset)	\
	((_AvailableCap) >> (_offset))

#define OCE_SET_DOWNLINK_AVAILABLE_CAP(_AvailableCap, _value, _mask, _offset)	\
{ \
	(_AvailableCap) &= (_mask); \
	(_AvailableCap) |= ((_value) << (_offset)); \
}

#define OCE_GET_UPLINK_AVAILABLE_CAP(_AvailableCap, _mask, _offset)	\
	((_AvailableCap) & (_mask))

#define OCE_SET_UPLINK_AVAILABLE_CAP(_AvailableCap, _value, _mask, _offset)	\
{ \
	(_AvailableCap) &= ~(_mask); \
	(_AvailableCap) |= ((_value) & (_mask)); \
}

/* OCE Attribute Id List */
#define OCE_ATTR_CAP_INDCATION					101
#define OCE_ATTR_AP_RSSI_REJCTION				102	/* RSSI-based (Re-)Association*/
#define OCE_ATTR_AP_REDUCED_WAN					103	/* Reduced WAN Metrics */
#define OCE_ATTR_AP_RNR_COMPLETE				104	/* RNR Completeness */
#define OCE_ATTR_STA_PRB_SUP_BSSID				105	/* Probe Suppression BSSIDs */
#define OCE_ATTR_STA_PRB_SUP_SSID				106	/* Probe Suppression SSIDs */
#define OCE_WDEV_ATTR_MAX_NUM					107	/* Should be updated according to ID list */

/* Element ID */
#define FILS_REQ_ID_EXTENSION				2	/* FILS Request Parameters Element ID Extension*/

/* OCE_ATTR_AP_CAP_INDCATION field value */
#define OCE_AP_CAP_NOT_SUPPORT					0x0

#define IS_OCE_ENABLE(_wdev) \
	((_wdev)->OceCtrl.bOceEnable == TRUE)
#define IS_OCE_RNR_ENABLE(_wdev) \
		((_wdev)->OceCtrl.bApRnrCompleteEnable == TRUE)
#define IS_OCE_FD_FRAME_ENABLE(_wdev) \
	((_wdev)->OceCtrl.bFdFrameEnable == TRUE)
#define VALID_OCE_ATTR_ID(_I) \
		(_I <= OCE_WDEV_ATTR_MAX_NUM)

#define FILS_CACHE_ID_LEN 2
#define FILS_REALMS_HASH_LEN 2

typedef enum {
	OCE_FRAME_TYPE_BEACON,
	OCE_FRAME_TYPE_PROBE_REQ,
	OCE_FRAME_TYPE_PROBE_RSP,
	OCE_FRAME_TYPE_ASSOC_REQ,
	OCE_FRAME_TYPE_ASSOC_RSP,
} OCE_FRAME_TYPE, *P_OCE_FRAME_TYPE;

typedef enum {
	OCE_SUCCESS = 0,
	OCE_INVALID_ARG,
	OCE_RESOURCE_ALLOC_FAIL,
	OCE_NOT_INITIALIZED,
	OCE_UNEXP,
} OCE_ERR_CODE;

typedef enum {
	OCE_MSG_INFO_UPDATE = 0,
} OCE_MSG_TYPE;

enum FD_CIPHER_SUITE_TYPE {
	FD_CIPHER_WEP40    = 1,
	FD_CIPHER_TKIP     = 2,
	FD_CIPHER_RESERVED = 3,
	FD_CIPHER_CCMP128  = 4,
	FD_CIPHER_WEP104   = 5,
	FD_CIPHER_BIPCMAC128 = 6,
	FD_CIPHER_GROUP_TRAFFIC_NOT_ALLOWED = 7,
	FD_CIPHER_GCMP128 = 8,
	FD_CIPHER_GCMP256 = 9,
	FD_CIPHER_CCMP256 = 10,
	FD_CIPHER_BIPGMAC128 = 11,
	FD_CIPHER_BIPGMAC256 = 12,
	FD_CIPHER_BIPCMAC256 = 13,
	FD_CIPHER_NO_SELECTED = 63,
};

enum FD_AKM_SUITE_TYPE {
	FD_AKM_USE_FROM_BCN_PROBERSP = 0,
	FD_AKM_FILS_SHA256 = 1,
	FD_AKM_FILS_SHA384 = 2,
	FD_AKM_FILS_SHA256_SHA384 = 3,
	FD_AKM_FT_FILS_SHA384 = 4,
	FD_AKM_NO_SELECTED = 63,
};

typedef struct GNU_PACKED _FD_CAP_SUB_FIELD {
	UCHAR ESS:1;
	UCHAR Privacy:1;
	UCHAR BSSOpChWidth:3;
	UCHAR MaxNumSS:3;

	UCHAR RESERVED:1;
	UCHAR MBSSIDSPresenceInd:1;
	UCHAR PhyIndex:3;
	UCHAR FILSMinRate:3;
} FD_CAP_SUB_FIELD, *PFD_CAP_SUB_FIELD;

typedef struct GNU_PACKED _FD_RSN_INFO {
	RSN_CAPABILITIES RSNCap;

	UINT32 GroupDataCipher:6;
	UINT32 GroupMgmtCipher:6;
	UINT32 PairwiseCipher:6;
	UINT32 AKMSuiteSelector:6;
} FD_RSN_INFO, *PFD_RSN_INFO;

typedef struct GNU_PACKED _FILS_DIS_FRAME_CTRL {
	UCHAR	SsidLength:5;
	UCHAR	CapPresenceInd:1;
	UCHAR	ShortSsidInd:1;
	UCHAR	AP_CSNPresenceInd:1;

	UCHAR	ANOPresenceInd:1;
	UCHAR	ChCenterFreqSegPresenceInd:1;
	UCHAR	PriChPresenceInd:1;
	UCHAR	RSNInfoPresenceInd:1;
	UCHAR	LengthPresenceInd:1;
	UCHAR	MDPresenceInd:1;
	UCHAR	AP11bPresent:1;
	UCHAR	NonOceAPPresent:1;
} FILS_DIS_FRAME_CTRL, *PFILS_DIS_FRAME_CTRL;

typedef struct GNU_PACKED _FILS_INFORMATION {
	UCHAR	NumOfPublicKeyID:3;
	UCHAR	NumOfRealmID:3;
	UCHAR	FilsIPConf:1;
	UCHAR	CacheIDIncluded:1;

	UCHAR	HESSIDIncluded:1;
	UCHAR	FilsSKAuthNoPFS:1;
	UCHAR	FilsSKAuthPFS:1;
	UCHAR	FilsPublicKeyAuth:1;
	UCHAR   RESERVED:4;
} FILS_INFORMATION, *PFILS_INFORMATION;

typedef struct _OCE_ATTR_STRUCT {
	UCHAR AttrID;
	UCHAR AttrLen;
	CHAR AttrBody[OCE_ATTR_MAX_LEN];
} OCE_ATTR_STRUCT, *P_OCE_ATTR_STRUCT;

typedef struct _OCE_CTRL {
	BOOLEAN	bOceEnable;
	BOOLEAN	bFdFrameEnable;
	BOOLEAN	bApReducedWanEnable;
	BOOLEAN	bApRnrCompleteEnable;
	BOOLEAN bApEspEnable;
	BOOLEAN	bOceFilsHlpEnable;
	UINT8	OceCapIndication;
	UINT8	AvailableCap;/* DL & UL */

	/* AssocReq Reject */
	UINT8	AssocRetryDelay;
	INT8	AssocRSSIThres;

	/* FILS */
	UINT16  FilsRealmsHash;
	UINT16  FilsCacheId;
	UINT32	FilsDhcpServerIp;
	UINT32	FilsDhcpServerPort;

	/* Timer */
	RALINK_TIMER_STRUCT Scan11bOceAPTimer;
	BOOLEAN Scan11bOceAPTimerRunning;
	RALINK_TIMER_STRUCT MaxChannelTimer;
	BOOLEAN MaxChannelTimerRunning;
	BOOLEAN MaxChannelTimesUp;

	BOOLEAN	ShortSSIDEnabled;
	NDIS_802_11_MAC_ADDRESS OCE_SUPPRES_BSSID_LIST[];
} OCE_CTRL, *P_OCE_CTRL;

typedef struct GNU_PACKED _REDUCED_NR_LIST_INFO {
	UINT32  ifindex;
	UINT32  ValueLen;
	UCHAR   Value[512];
} REDUCED_NR_LIST_INFO;

struct oce_info {
	UINT8	mac_addr[MAC_ADDR_LEN];
	UINT8	bssid[MAC_ADDR_LEN];
	UINT8	OceCapIndication;
	INT8	DeltaAssocRSSI;
};

typedef union GNU_PACKED _oce_msg_body {
	struct oce_info OceEvtStaInfo;
} OCE_MSG_BODY;

typedef struct oce_msg {
	UINT32	ifindex;
	UINT8	OceMsgLen;
	UINT8	OceMsgType;
	OCE_MSG_BODY OceMsgBody;
} OCE_MSG, *P_OCE_MSG;

OCE_ERR_CODE OceInit(
	PRTMP_ADAPTER pAd);

OCE_ERR_CODE OceTimerInit(
	PRTMP_ADAPTER pAd);

OCE_ERR_CODE OceDeInit(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev);

OCE_ERR_CODE OceRelease(
	PRTMP_ADAPTER pAd);

OCE_ERR_CODE OceCollectAttribute(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *pEntry,
	PUINT8 pAttrLen,
	PUINT8 pAttrBuf,
	UINT8 FrameType);

VOID OceParseStaOceIE(
	PRTMP_ADAPTER pAd,
	UCHAR *buf,
	UCHAR len,
	PEER_PROBE_REQ_PARAM *ProbeReqParam);

VOID OceParseStaAssoc(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *pEntry,
	UCHAR *buf,
	UCHAR len,
	OCE_FRAME_TYPE OceFrameType);

BOOLEAN OceCheckOceCap(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR *buf,
	UCHAR len);

INT OceApAutoChSelection2G(
	AUTO_CH_CTRL * pAutoChCtrl,
	AUTOCH_SEL_CH_LIST *pACSChList);

VOID OceScanOceAPList(
	BSS_TABLE *Tab);

VOID OceTXSHandler(
	RTMP_ADAPTER *pAd,
	CHAR *Data);

VOID OceIndicateStaInfo(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *pWdev,
	UCHAR *mac_addr);

INT OceIndicateStaInfoToDaemon(
	PRTMP_ADAPTER	pAd,
	struct oce_info *poceInfo,
	OCE_MSG_TYPE MsgType);

void Oce_read_parameters_from_file(
	IN PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *pBuffer);

INT oce_build_ies(
	RTMP_ADAPTER *pAd,
	struct _build_ie_info *info,
	BOOLEAN is_oce_sta);

VOID OceSendFilsDiscoveryAction(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

INT build_rnr_element(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR *buf,
	UINT8 pos,
	UCHAR subtype);

INT build_esp_element(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR *buf);

INT	Set_OceRssiThreshold_Proc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *arg);

INT	Set_OceAssocRetryDelay_Proc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *arg);

INT	Set_OceFdFrameCtrl_Proc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *arg);

INT Set_OceDownlinkAvailCap_Proc(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING *arg);

INT Set_OceUplinkAvailCap_Proc(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING *arg);

INT Set_OceEspEnable_Proc(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING *arg);

INT32 Show_OceStat_Proc(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

RTMP_STRING *OceMsgTypeToString(
	OCE_MSG_TYPE MsgType);

INT Set_OceReducedNRIndicate_Proc(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING *arg);

INT Set_OceReducedWanEnable_Proc(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING *arg);

#endif /* OCE_SUPPORT */
#endif /* __OCE_H */


