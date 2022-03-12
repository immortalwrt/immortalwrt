#ifndef __MBO_H
#define __MBO_H

#if defined(MBO_SUPPORT) || defined(OCE_SUPPORT)
#include "rtmp_type.h"
#include "rt_config.h"

#ifndef OCE_SUPPORT
#define IS_OCE_ENABLE(_wdev) (FALSE)
#endif /* OCE_SUPPORT */

#define MBO_NPC_MAX_LEN							50		/* Non Preferred Channel List Max Len */

#define IE_MBO_ELEMENT_ID						221	/* 0xDD */
#define MBO_ATTR_MAX_LEN						252	/* spec 0.0.23 - IE LEN is 256 = OUI 4 + ATTR 252 */
#define MBO_NEIGHBOR_REPORT_MAX_LEN				2304	/* 11v spec define BTM candidate list max len = 2304 */
#define MBO_RRM_SUBID_BSS_TRANSITION_CANDIDATE_PREFERENCE 3

#define MBO_OUI_NON_PREFERRED_CHANNEL_REPORT	0x2
#define MBO_OUI_CELLULAR_DATA_CAPABILITY		0x3

/* MBO Country IE Country String field definition */
#define MBO_AP_USE_GLOBAL_OPERATING_CLASS		0x04
#define MBO_AP_USE_LOCAL_OPERATING_CLASS		0x20

/* MBO Attribute Id List */
#define MBO_ATTR_AP_CAP_INDCATION				1
#define MBO_ATTR_STA_NOT_PREFER_CH_REP			2
#define MBO_ATTR_STA_CDC						3		/* Cellular Data Capability */
#define MBO_ATTR_AP_ASSOC_DISALLOW				4
#define MBO_ATTR_AP_CDCP						5		/* Cellular Data Connection Preference */
#define MBO_ATTR_AP_TRANS_REASON				6
#define MBO_ATTR_STA_TRANS_REJ_REASON			7
#define MBO_ATTR_AP_ASSOC_RETRY_DELAY			8
#define MBO_ATTR_STA_NOT_PREFER_CH_REP_2ND		9		/* enable sta to insert 2nd NPC attr in assoc */
#define MBO_WDEV_ATTR_MAX_NUM				9		/* Should be updated according to ID list */

/* MBO_ATTR_AP_CAP_INDCATION field value */
#define MBO_AP_CAP_NOT_SUPPORT					0x0
#define MBO_AP_CAP_CELLULAR_AWARE				0x40	/* indicates supporting MBO */

/* MBO_ATTR_AP_ASSOC_DISALLOW Reason field value */
#define MBO_AP_DISALLOW_REASON_RESERVED			0x0	/* allow assoc , no disallow attr in MBO IE */
#define MBO_AP_DISALLOW_REASON_UNSPECIFIED		0x1
#define MBO_AP_DISALLOW_MAX_STA_NUM_REACHED		0x2
#define MBO_AP_DISALLOW_AIR_OVERLOADED			0x3
#define MBO_AP_DISALLOW_AUTH_SERVER_OVERLOADED	0x4
#define MBO_AP_DISALLOW_RSSI_TOO_LOW			0x5
#define MBO_AP_ASSOC_ALLOW				0x6

/* MBO_ATTR_AP_CDCP field value */
#define MBO_AP_CDCP_FORBID_STA_USE_CDC			0x0
#define MBO_AP_CDCP_PREFER_STA_NOT_USE_CDC		0x1
#define MBO_AP_CDCP_PREFER_STA_USE_CDC			0xFF	/* 255 - prefer cellular data connection */

/* MBO_ATTR_AP_TRANS_REASON field value */
#define MBO_AP_TRANS_REASON_UNSPECIFIED			0x0
#define MBO_AP_TRANS_REASON_TX_PER_TOO_HIGH		0x1	/* excessive frame loss rate */
#define MBO_AP_TRANS_REASON_TRAFFIC_DELAY		0x2
#define MBO_AP_TRANS_REASON_INSUFFICIENT_BW		0x3
#define MBO_AP_TRANS_REASON_LOAD_BALACING		0x4
#define MBO_AP_TRANS_REASON_RSSI_LOW			0x5
#define MBO_AP_TRANS_REASON_TOO_MANY_RETRY		0x6	/* Received excessive number of retransmissions */
#define MBO_AP_TRANS_REASON_HIGH_INTRFERENCE	0x7
#define MBO_AP_TRANS_REASON_GRAY_ZONE			0x8
#define MBO_AP_TRANS_REASON_TO_PREMIUM_AP		0x9

#define MAX_NOT_PREFER_CH_NUM 16
#define MAX_NOT_PREFER_CH_REG_NUM 2
#define MAX_ASSOC_DISALLOW_AP_NUM 			16

#define MBO_OCE_OUI_0 					0x50
#define MBO_OCE_OUI_1 					0x6f
#define MBO_OCE_OUI_2 					0x9a
#define MBO_OCE_OUI_TYPE 				0x16

/* A default pref value of the auto-populated NR entries */
#define MBO_AP_DEFAULT_CAND_PREF                255

typedef enum {
    MBO_FRAME_TYPE_BEACON,
    MBO_FRAME_TYPE_PROBE_REQ,
    MBO_FRAME_TYPE_PROBE_RSP,
    MBO_FRAME_TYPE_ASSOC_REQ,
    MBO_FRAME_TYPE_ASSOC_RSP,
    MBO_FRAME_TYPE_BTM_REQ,
    MBO_FRAME_TYPE_WNM_REQ,
} MBO_FRAME_TYPE, *P_MBO_FRAME_TYPE;

typedef enum {
	MBO_SUCCESS = 0,
	MBO_INVALID_ARG,
	MBO_RESOURCE_ALLOC_FAIL,
	MBO_NOT_INITIALIZED,
	MBO_UNEXP,
} MBO_ERR_CODE;

typedef enum {
	MBO_MSG_NEIGHBOR_REPORT = 0,
	MBO_MSG_STA_PREF_UPDATE,
	MBO_MSG_CDC_UPDATE,
	MBO_MSG_STA_STEERING,
	MBO_MSG_DISASSOC_STA,
	MBO_MSG_AP_TERMINATION,
	MBO_MSG_BSSID_UPDATE,
	MBO_MSG_REMOVE_STA,
/* MBO_MSG_STA_SEC_INFO_UPDATE, */
} MBO_MSG_TYPE;


typedef enum {
	PARAM_MBO_AP_ASSOC_DISALLOW,
	PARAM_MBO_AP_CAP,
	PARAM_MBO_AP_CDCP,
	PARAM_MBO_AP_BSS_TERM,
} MBO_PARAM;
#ifndef WAPP_SUPPORT
#define PER_EVENT_LIST_MAX_NUM 5

typedef struct GNU_PACKED _tbtt_info_set
{
	UINT8 NrAPTbttOffset;
	UINT32 ShortBssid;
} tbtt_info_set;

typedef struct GNU_PACKED _wapp_nr_info
{
	u8	Bssid[MAC_ADDR_LEN];
	u32 BssidInfo;
	u8	RegulatoryClass;
	u8	ChNum;
	u8	PhyType;
	u8	CandidatePrefSubID;
	u8	CandidatePrefSubLen;
	u8	CandidatePref;
	/* extra sec info */
	u32 akm;
	u32 cipher;
	u8	TbttInfoSetNum;
	tbtt_info_set TbttInfoSet;
	u8	Rssi;
} wapp_nr_info;


typedef struct GNU_PACKED daemon_neighbor_report_list {
	UINT8	Newlist;
	UINT8	TotalNum;
	UINT8	CurrNum;
	UINT8	reserved;
	wapp_nr_info EvtNRInfo[PER_EVENT_LIST_MAX_NUM];
} DAEMON_EVENT_NR_LIST, *P_DAEMON_EVENT_NR_LIST;

typedef struct GNU_PACKED neighbor_report_msg {
	DAEMON_EVENT_NR_LIST evt_nr_list;
} DAEMON_NR_MSG, *P_DAEMON_NR_MSG;
#endif

typedef struct GNU_PACKED non_pref_ch {
	UINT8 ch;
	UINT8 pref;
	UINT8 reason_code;

} STA_CH_PREF, *P_STA_CH_PREF;


typedef struct GNU_PACKED _MBO_STA_CH_PREF_CDC_INFO
{
	UINT8	mac_addr[MAC_ADDR_LEN];
	UINT8	bssid[MAC_ADDR_LEN];
	UINT8	cdc; /* cellular data capability */
	UINT8	npc_num;
	UINT32  akm;
	UINT32  cipher;
	struct non_pref_ch npc[MBO_NPC_MAX_LEN];
} MBO_STA_CH_PREF_CDC_INFO, *P_MBO_STA_CH_PREF_CDC_INFO;


typedef struct GNU_PACKED _MBO_EVENT_STA_DISASSOC
{
	UINT8 mac_addr[MAC_ADDR_LEN];
} MBO_EVENT_STA_DISASSOC, *P_MBO_EVENT_STA_DISASSOC;

typedef struct GNU_PACKED _MBO_EVENT_STA_AKM_CIPHER
{
	UINT8  mac_addr[MAC_ADDR_LEN];
	UINT32 akm;
	UINT32 cipher;
} MBO_EVENT_STA_AKM_CIPHER, *P_MBO_EVENT_STA_AKM_CIPHER;

typedef struct GNU_PACKED _MBO_EVENT_BSS_TERM
{
	UINT32 TsfLowPart;
	UINT32 TsfHighPart;
} MBO_EVENT_BSS_TERM, *P_MBO_EVENT_BSS_TERM;

typedef union GNU_PACKED _msg_body {
	MBO_STA_CH_PREF_CDC_INFO MboEvtStaInfo;
	MBO_EVENT_STA_DISASSOC MboEvtStaDisassoc;
	MBO_EVENT_BSS_TERM MboEvtBssTermTsf;
	/* MBO_EVENT_STA_AKM_CIPHER MboEvtStaSecInfo; */
} MBO_MSG_BODY;

typedef struct mbo_msg {
	UINT32  ifindex;
	UINT8	MboMsgLen;
	UINT8	MboMsgType;
	MBO_MSG_BODY MboMsgBody;
} MBO_MSG, *P_MBO_MSG;

#ifdef CONFIG_STA_SUPPORT

typedef struct _MBO_NOT_PREFER_CH_REP {
	UINT8 reg_class;
	struct non_pref_ch npc[MBO_NPC_MAX_LEN];
} MBO_NOT_PREFER_CH_REP, *PMBO_NOT_PREFER_CH_REP;

typedef struct GNU_PACKED _MBO_ASSOC_DISALLOW_INFO
{
	BOOLEAN	bAssocDisallow;
	UINT8	Bssid[MAC_ADDR_LEN];
	UINT8   AssocDisallowReason;
} MBO_ASSOC_DISALLOW_INFO, *PMBO_ASSOC_DISALLOW_INFO;

INT SetMboChPrefProc(
		PRTMP_ADAPTER pAd,
		RTMP_STRING *arg);

#endif /* CONFIG_STA_SUPPORT */
typedef struct _MBO_CTRL {
	BOOLEAN	bMboEnable;
	/* BOOLEAN bHaveBTMSta;*/      /* TRUE if this wdev still have STAs undergoing BTM disassoc procedure */
#ifdef CONFIG_STA_SUPPORT
	MBO_ASSOC_DISALLOW_INFO ADInfo[MAX_ASSOC_DISALLOW_AP_NUM];
#endif /* CONFIG_STA_SUPPORT */
	UINT8 reg_class;
	STA_CH_PREF npc[MBO_NPC_MAX_LEN];
	UINT8	MboCapIndication;
	UINT8   AssocDisallowReason;
	UINT8	CellularPreference;
	UINT8	TransitionReason;
	UINT16  ReAssocDelay; /* second */
} MBO_CTRL, *P_MBO_CTRL;

typedef struct _MBO_ATTR_STRUCT {
    UCHAR   AttrID;
    UCHAR   AttrLen;
    /* CHAR    AttrBody[1]; */
    CHAR AttrBody[MBO_ATTR_MAX_LEN];
} MBO_ATTR_STRUCT, *P_MBO_ATTR_STRUCT;



#define IS_MBO_ENABLE(_wdev) \
	((_wdev)->MboCtrl.bMboEnable == TRUE)

#define VALID_MBO_ATTR_ID(_I) \
	(_I <= MBO_WDEV_ATTR_MAX_NUM)

#define MBO_AP_ALLOW_ASSOC(_wdev) \
	((_wdev)->MboCtrl.AssocDisallowReason == MBO_AP_DISALLOW_REASON_RESERVED)


VOID MakeMboOceIE(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *pEntry,
	PUINT8 pFrameBuf,
	PULONG pFrameLen,
	UINT8 FrameType);

MBO_ERR_CODE ReadMboParameterFromFile(
    PRTMP_ADAPTER pAd,
    RTMP_STRING *tmpbuf,
    RTMP_STRING *pBuffer);

MBO_ERR_CODE MboInit(
	PRTMP_ADAPTER pAd);

RTMP_STRING *MboAttrValueToString(
	UINT8 AttrID,
	UINT8 AttrValue);

RTMP_STRING *MboMsgTypeToString(
	MBO_MSG_TYPE MsgType);


INT32 ShowMboStatProc(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

INT MboIndicateNeighborReportToDaemon(
	PRTMP_ADAPTER	pAd,
	struct wifi_dev *pWdev,
	BOOLEAN		AppendMode,
	UINT8			ReportNum);


INT SetMboNRIndicateProc(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING *arg);

INT MBO_MsgHandle(
	IN PRTMP_ADAPTER pAd,
	UINT32 Param,
	UINT32 Value);

VOID MboBssTermCountDownStart(
	UINT8 validity_intvl);

VOID MboParseStaNPCElement(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *pWdev,
	UCHAR *PktContent,
	UINT8 ElementLen,
	P_MBO_STA_CH_PREF_CDC_INFO pMboStaCHInfo,
	MBO_FRAME_TYPE MboFrameType);


INT MboIndicateStaInfoToDaemon(
	PRTMP_ADAPTER	pAd,
	P_MBO_STA_CH_PREF_CDC_INFO pStaInfo,
	MBO_MSG_TYPE MsgType);

BOOLEAN MboAssocDisallowLookup(
	PRTMP_ADAPTER pAd,
	UCHAR *pAddr);

VOID MboParseApMboIE(
	PRTMP_ADAPTER pAd,
	UCHAR *pAddr,
	UCHAR *buf,
	UCHAR len);

VOID MboParseStaMboIE(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *pWdev,
	struct _MAC_TABLE_ENTRY *pEntry,
	UCHAR *buf,
	UCHAR len,
	MBO_FRAME_TYPE MboFrameType);


INT MboIndicateStaDisassocToDaemon(
	PRTMP_ADAPTER	pAd,
	P_MBO_EVENT_STA_DISASSOC pStaDisassocInfo,
	MBO_MSG_TYPE MsgType);

VOID MboWaitAllStaGone(
	PRTMP_ADAPTER pAd,
	INT apidx);

VOID MboIndicateStaBssidInfo(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *pWdev,
	UCHAR *mac_addr);

VOID MboIndicateOneNRtoDaemonByBssEntry(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *pWdev,
	BSS_ENTRY *pBssEntry);

VOID MboBssTermStart(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *pWdev,
	UINT8 countdown);

VOID MboCheckBssTermination(
	PRTMP_ADAPTER pAd);
#endif /* MBO_SUPPORT OCE_SUPPORT*/
#endif /* __MBO_H */

