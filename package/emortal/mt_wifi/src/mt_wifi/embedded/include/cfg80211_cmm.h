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
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All CFG80211 Function Prototype.

***************************************************************************/


#ifndef __CFG80211CMM_H__
#define __CFG80211CMM_H__

#ifdef RT_CFG80211_SUPPORT

#define RTMP_CFG80211_HOSTAPD_ON(__pAd) (__pAd->net_dev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP)

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
#define CFG80211_GetEventDevice(__pAd) __pAd->cfg80211_ctrl.dummy_p2p_net_dev
#else
#define CFG80211_GetEventDevice(__pAd) __pAd->net_dev
#endif	/* RT_CFG80211_P2P_CONCURRENT_DEVICE */


#define CFG_GO_BSSID_IDX (MAIN_MBSSID + 1)
#ifdef RT_CFG80211_P2P_SUPPORT

/* yiwei debug for P2P 7.1.3 */
/* yiwei tmp hard code */
#define IS_SW_NOA_TIMER(_A) (1)
#define P2P_OPPS_BIT		0x80

/*
 *  Macros for bit check
*/

#define CFG80211_P2P_TEST_BIT(_M, _F)      (((_M) & (_F)) != 0)

#define CFG_P2P_DISABLE	0x00000000
#define CFG_P2P_GO_UP		0x00000001
#define CFG_P2P_CLI_UP		0x00000002

#define IS_CFG80211_P2P_ABSENCE(_A)	(((_A)->cfg80211_ctrl.bPreKeepSlient) || ((_A)->cfg80211_ctrl.bKeepSlient))


typedef	struct	_P2PCLIENT_NOA_SCHEDULE	{
	BOOLEAN		bValid;
	BOOLEAN		bInAwake;


	UCHAR		Token;


	ULONG		SwTimerTickCounter; /* this Counter os used for sw-base NoA implementation tick counter */

	ULONG		CurrentTargetTimePoint; /* For sw-base method NoA usage */
	ULONG           NextTargetTimePoint;

	UCHAR           Count;
	ULONG           Duration;
	ULONG           Interval;
	ULONG           StartTime;
	ULONG		OngoingAwakeTime; /* this time will keep increasing as time go by. indecate the current awake time point */


	ULONG		LastBeaconTimeStamp;
}	P2PCLIENT_NOA_SCHEDULE, *PP2PCLIENT_NOA_SCHEDULE;


typedef struct __CFG_P2P_ENTRY_PARM {
	UCHAR					CTWindow;	/* As GO, Store client's Presence request NoA.  As Client, Store GO's NoA In beacon or P2P Action frame */
	P2PCLIENT_NOA_SCHEDULE	NoADesc[1];	/* As GO, Store client's Presence request NoA.  As Client, Store GO's NoA In beacon or P2P Action frame */
} CFG_P2P_ENTRY_PARM, *PCFG_P2P_ENTRY_PARM;


typedef struct {
	UCHAR   Eid;
	UCHAR   Len[2];
	CHAR   Octet[1];
} P2PEID_STRUCT, *PP2PEID_STRUCT;



VOID CFG80211_P2PCTWindowTimer(
	IN PVOID	SystemSpecific1,
	IN PVOID	FunctionContext,
	IN PVOID	SystemSpecific2,
	IN PVOID	SystemSpecific3);

VOID CFG80211_P2pSwNoATimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID CFG80211_P2pPreAbsenTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

#endif /* RT_CFG80211_P2P_SUPPORT */

#ifdef CFG_TDLS_SUPPORT
#define CATEGORY_TDLS				12
#define	PROTO_NAME_TDLS				2
#define TDLS_TIMEOUT				5000	/* unit: msec */

#define TDLS_ELM_LEN_LINK_IDENTIFIER		18
#define LENGTH_TDLS_H				24
#define LENGTH_TDLS_PAYLOAD_H		3  /* payload type(1 byte) + category(1 byte) + action(1 byte) */

/* Information element ID defined in 802.11z D13.0 specification. */
#define IE_TDLS_LINK_IDENTIFIER			101
#define IE_TDLS_WAKEUP_SCHEDULE			102
#define IE_TDLS_CHANNEL_SWITCH_TIMING	104
#define IE_TDLS_PTI_CONTROL				105
#define IE_TDLS_PU_BUFFER_STATUS		106


#define TDLS_ACTION_CODE_SETUP_REQUEST				0
#define TDLS_ACTION_CODE_SETUP_RESPONSE				1
#define TDLS_ACTION_CODE_SETUP_CONFIRM				2
#define TDLS_ACTION_CODE_TEARDOWN					3
#define TDLS_ACTION_CODE_PEER_TRAFFIC_INDICATION	4 /* for TDLS UAPSD */
#define TDLS_ACTION_CODE_CHANNEL_SWITCH_REQUEST		5
#define TDLS_ACTION_CODE_CHANNEL_SWITCH_RESPONSE	6
#define TDLS_ACTION_CODE_PEER_PSM_REQUEST			7
#define TDLS_ACTION_CODE_PEER_PSM_RESPONSE			8
#define TDLS_ACTION_CODE_PEER_TRAFFIC_RESPONSE		9 /* for TDLS UAPSD */
#define TDLS_ACTION_CODE_DISCOVERY_REQUEST			10
#define TDLS_ACTION_CODE_WFD_TUNNELED_PROBE_REQ			11
#define TDLS_ACTION_CODE_WFD_TUNNELED_PROBE_RSP			12
#define TDLS_ACTION_CODE_AUTO_TEARDOWN				13 /* auto teardown */

#define TDLS_CHANNEL_SWITCH_TIME		11000 /* micro seconds */
#define TDLS_CHANNEL_SWITCH_TIMEOUT		19000 /* micro seconds */


#define IS_TDLS_SUPPORT(_P) \
	((_P)->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.bCfgTDLSCapable == TRUE)
#define TDLS_TIMESTAMP_GET(__pAd, __TimeStamp)			\
	{													\
		UINT32 __CSR = 0; UINT64 __Value64;				\
		RTMP_IO_READ32((__pAd->hdev_ctrl), TSF_TIMER_DW0, &__CSR); \
		__TimeStamp = (UINT32)__CSR;					\
	}


#define MAX_NUM_OF_CFG_TDLS_ENTRY	4

enum {
	to_AP = 0,
	to_PEER
};
typedef struct _CFG_TDLS_ENTRY {
	BOOLEAN			EntryValid;
	UCHAR			MacAddr[MAC_ADDR_LEN];
	RALINK_TIMER_STRUCT	Timer;			/* Use to time out Peer Traffic Indicator */

	UCHAR			Token;			/* Dialog token */
	BOOLEAN			bInitiator;		/* TRUE: I am TDLS Initiator STA, FALSE: I am TDLS Responder STA */
	UINT16			MacTabMatchWCID;
	PVOID			pAd;
#ifdef UAPSD_SUPPORT
	BOOLEAN			FlgIsWaitingUapsdTraRsp; /* 1: waiting for traffic rsp frame */
	UCHAR			UapsdTraIndDelayCnt; /* used to send a detect traffic ind */
	BOOLEAN			FlgIsUapsdTraRspRcv; /* if we have ever received rsp frame */
	UCHAR			QosCapability;
#endif /* UAPSD_SUPPORT */

	UCHAR					HtCapabilityLen;
	HT_CAPABILITY_IE		HtCapability;

} CFG_TDLS_ENTRY, *PCFG_TDLS_ENTRY;

typedef struct _CFG_TDLS_CHSW_PARAM {
	UCHAR cmd;
	UCHAR offch_prim;
	UCHAR offch_center;
	UCHAR bw_off;
	UCHAR role;
	UINT16 stay_time;
	UINT32 start_time_tsf;
	UINT16 switch_time;
	UINT16 switch_timeout;
} CFG_TDLS_CHSW_PARAM, *PCFG_TDLS_CHSW_PARAM;


#define FT_MIC_LEN					16
#define FT_NONCE_LEN				32

/* Information element ID defined in 802.11rD9.0 specification. */
#define IE_FT_MDIE				54
#define IE_FT_FTIE				55
#define IE_FT_TIMEOUT_INTERVAL	56
#define IE_FT_RIC_DATA			57
#define IE_FT_RIC_DESCRIPTOR	75


/* RIC Type */
#define FT_RIC_TYPE_BA			1

/* AKM SUITE */
#define FT_AKM_SUITE_1X		3
#define FT_AKM_SUITE_PSK	4

typedef union GNU_PACKED _FT_MIC_CTR_FIELD {
	/*
		IECnt: contains the number of IEs
		that are included int eht MIC calculation.
	*/
	struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
		UINT16 IECnt:8;
		UINT16 reserved:7;
		UINT16 rsnxe_used:1;
#else
		UINT16 rsnxe_used:1;
		UINT16 reserved:7;
		UINT16 IECnt:8;
#endif
	} field;
	UINT16 word;
} FT_MIC_CTR_FIELD, *PFT_MIC_CTR_FIELD;

/*
** FTIE: Fast Transition IE.
*/
typedef struct GNU_PACKED _FT_FTIE {
	FT_MIC_CTR_FIELD MICCtr;		/* 2 Octects. */
	UINT8 MIC[FT_MIC_LEN];			/* 16 Octects. */
	UINT8 ANonce[FT_NONCE_LEN];		/* 32 Octects. */
	UINT8 SNonce[FT_NONCE_LEN];		/* 32 Octects. */
	UINT8 Option[0];				/* 1:R1KHID, 2:GTK, 3:ROKHId, else:Res */
} FT_FTIE, *PFT_FTIE;


enum tdls_operation {
	TDLS_DISCOVERY_REQ,
	TDLS_SETUP,
	TDLS_TEARDOWN,
	TDLS_ENABLE_LINK,
	TDLS_DISABLE_LINK,
};

enum tdls_entry_op {
	tdls_insert_entry = 0,
	tdls_delete_entry
};
#endif /* CFG_TDLS_SUPPORT */


#endif /* RT_CFG80211_SUPPORT */

#endif /* __CFG80211CMM_H__ */


