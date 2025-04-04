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

	Define all structures, data types that rtmp.h needed in this file. Don't
	put any sturctures and functions definition which refer to RTMP_ADAPTER
	Here.


***************************************************************************/

#ifdef P2P_SUPPORT


#ifndef __P2P_CMM_H__
#define __P2P_CMM_H__

#include "wfa_p2p.h"

#ifdef WFD_SUPPORT
#include "wfd_cmm.h"
#endif /* WFD_SUPPORT */

extern UCHAR WILDP2PSSID[];
extern UCHAR WILDP2PSSIDLEN;

/* Marco for interna use */
#define P2P_INF_ON(_pAd) \
	(((_pAd)->flg_p2p_init) \
	 && ((_pAd)->p2p_dev) \
	 && (RtmpOSNetDevIsUp((_pAd)->p2p_dev) == TRUE))

#define IS_P2P_GO_NEG(_A)		(((_A)->P2pCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_CLIENT) || ((_A)->P2pCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO))

#define IS_P2P_REGISTRA(_A)		(((_A)->P2pCfg.P2PConnectState == P2P_WPS_REGISTRA))
#define IS_P2P_ENROLLEE(_A)		(((_A)->P2pCfg.P2PConnectState == P2P_DO_WPS_ENROLLEE))
#define IS_P2P_PROVISIONING(_A)		(IS_P2P_ENROLLEE(_A) || IS_P2P_REGISTRA(_A))
/*#define IS_P2P_GO_WPA2PSKING(_A)		((_A)->P2pCfg.P2PConnectState == P2P_GO_ASSOC_AUTH) */
/*#define IS_P2P_CLIENT_WPA2PSKING(_A)		((_A)->P2pCfg.P2PConnectState == P2P_I_AM_CLIENT_ASSOC_AUTH) */

/*#define IS_P2P_GO_OP(_A)		(((_A)->P2pCfg.P2PConnectState >= P2P_I_AM_GO_OP)) */
/*#define IS_P2P_CLIENT_OP(_A)		(((_A)->P2pCfg.P2PConnectState == P2P_I_AM_CLIENT_OP)) */
/*#define IS_P2P_OP(_A)		(IS_P2P_GO_OP(_A) || IS_P2P_CLIENT_OP(_A)) */

#define IS_P2P_CONNECT_IDLE(_A)		(((_A)->P2pCfg.P2PConnectState) == P2P_CONNECT_IDLE)
#define IS_P2P_CONNECTING(_A)		(((_A)->P2pCfg.P2PConnectState) > P2P_CONNECT_IDLE)
#define IS_P2P_GO_NEGOING(_A)		(((_A)->P2pCfg.P2PConnectState < P2P_DO_GO_SCAN_BEGIN) &&  ((_A)->P2pCfg.P2PConnectState > P2P_CONNECT_IDLE))
#define IS_P2P_GROUP_FORMING(_A)		(((_A)->P2pCfg.P2PConnectState <= P2P_WPS_REGISTRA) &&  ((_A)->P2pCfg.P2PConnectState > P2P_CONNECT_IDLE))
#define IS_P2P_INVITING(_A)		(((_A)->P2pCfg.P2PConnectState == P2P_INVITE))
/* Check for Peer's State. */
#define IS_P2P_PEER_CLIENT_OP(_C)		(((_C)->P2pClientState <= P2PSTATE_CLIENT_FIND) && ((_C)->P2pClientState >= P2PSTATE_CLIENT_OPERATING))
#define IS_P2P_PEER_PROVISIONING(_C)		(((_C)->P2pClientState == P2PSTATE_CLIENT_WPS) && ((_C)->P2pClientState == P2PSTATE_GO_WPS))
#define IS_P2P_PEER_WPAPSK(_C)		(((_C)->P2pClientState <= P2PSTATE_CLIENT_ASSOC) && ((_C)->P2pClientState >= P2PSTATE_CLIENT_AUTH))
#define IS_P2P_PEER_GO_OP(_C)		(((_C)->P2pClientState <= P2PSTATE_GO_FIND) && ((_C)->P2pClientState >= P2PSTATE_GO_OPERATING))
#define IS_P2P_PEER_OP(_C)		((IS_P2P_PEER_CLIENT_OP(_C)) || (IS_P2P_PEER_GO_OP(_C)))
#define IS_P2P_PEER_DISCOVERY(_C)		(((_C)->P2pClientState <= P2PSTATE_DISCOVERY_CLIENT) && ((_C)->P2pClientState >= P2PSTATE_DISCOVERY))
#define IS_P2PPEER_CLIENT_GO_FORM(_C)		(((_C)->P2pClientState <= P2PSTATE_GO_COMFIRM_ACK_SUCCESS) && ((_C)->P2pClientState >= P2PSTATE_DISCOVERY))

/* P2P Control Reg. */
#define IS_MANAGED_ON(_A)		((_A->P2pCfg.P2pControl.word & P2P_CONTROL_REG_MANAGED) != 0)
#define IS_PERSISTENT_ON(_A)	((_A->P2pCfg.P2pControl.word & P2P_CONTROL_REG_PERSISTENT) != 0)
#define IS_INVITE_ON(_A)		((_A->P2pCfg.P2pControl.word & P2P_CONTROL_REG_INVITE) != 0)
#define IS_INTRA_BSS_ON(_A)		((_A->P2pCfg.P2pControl.word & P2P_CONTROL_REG_INTRA_BSS) != 0)
#define IS_CLIENT_DISCOVERY_ON(_A)		((_A->P2pCfg.P2pControl.word & P2P_CONTROL_REG_CLI_DISCOVERY) != 0)
#define IS_EXT_LISTEN_ON(_A)		((_A->P2pCfg.P2pControl.word & P2P_CONTROL_REG_EXT_LISTEN) != 0)
#define IS_SERVICE_DISCOVERY_ON(_A)		((_A->P2pCfg.P2pControl.word & P2P_CONTROL_REG_SERVICE_DISCOVERY) != 0)
#define IS_OPPS_ON(_A)		((_A->P2pCfg.P2pControl.word & P2P_CONTROL_REG_OPPS) != 0)
#define IS_SW_NOA_TIMER(_A) ((_A->P2pCfg.P2pControl.word & P2P_CONTROL_REG_SW_NOA) != 0)
#define IS_P2P_SUPPORT_EXT_LISTEN(_A)	(((_A)->P2pCfg.ExtListenInterval != 0) && ((_A)->P2pCfg.ExtListenPeriod != 0) && ((_A)->P2pCfg.P2pControl.field.ExtendListen == 1))
#define IS_P2P_ABSENCE(_A)	(((_A)->P2pCfg.bPreKeepSlient) || ((_A)->P2pCfg.bKeepSlient))

/* P2P Discovery State Machine. */
#define IS_P2P_LISTEN(_pAd)	\
	(_pAd->P2pCfg.DiscCurrentState == P2P_DISC_LISTEN)
#define IS_P2P_SEARCH(_pAd)	\
	(_pAd->P2pCfg.DiscCurrentState == P2P_DISC_SEARCH)
#define IS_P2P_SCAN(_pAd)	\
	(_pAd->P2pCfg.DiscCurrentState == P2P_DISC_SCAN)
#define IS_P2P_DEVICE_DISCOVERING(_pAd) \
	(_pAd->P2pCfg.CtrlCurrentState == P2P_CTRL_DISCOVERY)

/* Definition */
#define P2P_WSC_CONF_MTHD_DEFAULT(_pAd)	\
	_pAd->P2pCfg.WscMode = WSC_PIN_MODE;	\
	_pAd->P2pCfg.ConfigMethod = 0x188;	\
	_pAd->P2pCfg.Dpid = DEV_PASS_ID_NOSPEC;

/*
  * P2P CTRL State machine: states, events, total function
  */
typedef	enum	_P2P_CTRL_STATE {
	P2P_CTRL_IDLE,
	P2P_CTRL_DISCOVERY,
	P2P_CTRL_GROUP_FORMATION,
	P2P_CTRL_DONE,
	P2P_CTRL_MAX_STATES,
}	P2P_CTRL_STATE;

typedef enum _P2P_CTRL_EVENT {
	P2P_CTRL_DISC_EVT,
	P2P_CTRL_DISC_CANL_EVT,
	P2P_CTRL_DISC_DONE_EVT,
	P2P_CTRL_GO_BACK_TO_OP_CH_EVT,
	P2P_CTRL_GO_NEGO_EVT,
	P2P_CTRL_GO_NEGO_CANL_EVT,
	P2P_CTRL_GO_NEGO_DONE_EVT,
	P2P_CTRL_MAX_EVENTS,
} P2P_CTRL_EVENT;

#define	P2P_CTRL_FUNC_SIZE	(P2P_CTRL_MAX_STATES * P2P_CTRL_MAX_EVENTS)


/*
  * P2P DISC state machine: states, evvents, total function
  */
typedef enum	_P2P_DISC_STATE {
	P2P_DISC_IDLE,
	P2P_DISC_SCAN,
	P2P_DISC_LISTEN,
	P2P_DISC_SEARCH,
	P2P_DISC_MAX_STATES,
} P2P_DISC_STATE;

typedef enum	_P2P_DISC_EVENT {
	P2P_DISC_SCAN_CMD_EVT,
	P2P_DISC_LISTEN_CMD_EVT,
	P2P_DISC_SEARCH_CMD_EVT,
	P2P_DISC_CANL_CMD_EVT,
	P2P_DISC_PEER_PROB_REQ,
	P2P_DISC_PEER_PROB_RSP,
	P2P_DISC_MAX_EVENTS,
} P2P_DISC_EVENT;

#define	P2P_DISC_FUNC_SIZE	(P2P_DISC_MAX_STATES * P2P_DISC_MAX_EVENTS)

/*
  * P2P GO_FORM state machine: states, evvents, total function
  */
typedef enum	_P2P_GO_FORM_STATE {
	P2P_GO_FORM_IDLE,
	P2P_WAIT_GO_FORM_RSP,
	P2P_WAIT_GO_FORM_CONF,
	P2P_GO_FORM_DONE,
	P2P_GO_FORM_PROV,
	P2P_WAIT_GO_FORM_PROV_RSP,
	P2P_GO_FORM_INVITE,
	P2P_WAIT_GO_FORM_INVITE_RSP,
	P2P_WAIT_GO_FORM_DEV_DISC_RSP,
	P2P_WAIT_GO_FORM_SRV_DISC_RSP,
	P2P_GO_FORM_MAX_STATES,
} P2P_GO_FORM_STATE;

typedef enum	_P2P_GO_FORM_EVENT {
	P2P_GO_NEGO_REQ_CMD_EVT,
	P2P_PEER_GO_NEGO_REQ_EVT,
	P2P_GO_NEGO_RSP_EVT,
	P2P_PEER_GO_NEGO_RSP_EVT,
	P2P_GO_NEGO_CONFIRM_EVT,
	P2P_PEER_GO_NEGO_CONFIRM_EVT,
	P2P_GO_NEGO_PROV_REQ_CMD_EVT,
	P2P_PEER_GO_NEGO_PROV_REQ_EVT,
	P2P_GO_NEGO_PROV_RSP_EVT,
	P2P_PEER_GO_NEGO_PROV_RSP_EVT,
	P2P_GO_NEGO_CANCEL_EVT,
	P2P_PEER_INVITE_REQ_EVT,
	P2P_PEER_INVITE_RSP_EVT,
	P2P_DEV_DISC_REQ_CMD_EVT,
	P2P_PEER_DEV_DISC_REQ_EVT,
	P2P_DEV_DISC_RSP_EVT,
	P2P_PEER_DEV_DISC_RSP_EVT,
	P2P_START_COMMUNICATE_CMD_EVT,
	P2P_SEND_PASSED_CMD_EVT,
	P2P_GO_NEGO_MAX_EVENTS,
} P2P_GO_FORM_EVENT;

typedef enum	_P2P_LINK_DOWN_TYPE {
	P2P_DISCONNECTED,
	P2P_CONNECT_FAIL,
} P2P_LINK_DOWN_TYPE;

#define	P2P_GO_FORM_FUNC_SIZE	(P2P_GO_FORM_MAX_STATES * P2P_GO_NEGO_MAX_EVENTS)

/*
  * P2P state machine: states, events, total function #
  */
#define MT2_MAX_PEER_SUPPORT              3
typedef enum	_P2P_ACTION_STATE {
	P2P_IDLE_STATE,
	MAX_P2P_STATE,
} P2P_ACTION_STATE;

typedef enum	_P2P_ACTION_EVENT {
	MT2_PEER_P2P_NOA,
	MT2_PEER_P2P_PRESENCE_REQ,
	MT2_PEER_P2P_PRESENCE_RSP,
	MT2_PEER_P2P_GO_DIS_REQ,
	MT2_MLME_P2P_NOA,
	MT2_MLME_P2P_PRESENCE_REQ,
	MT2_MLME_P2P_PRESENCE_RSP,
	MT2_MLME_P2P_GO_DIS_REQ,
	MT2_MLME_P2P_GAS_INT_REQ,
	MT2_MLME_P2P_GAS_INT_RSP,
	MT2_MLME_P2P_GAS_CB_REQ,
	MT2_MLME_P2P_GAS_CB_RSP,
	MAX_P2P_MSG,
} P2P_ACTION_EVENT;

#define P2P_ACTION_FUNC_SIZE	(MAX_P2P_STATE * MAX_P2P_MSG)


/* ----- General ----- */

/* P2P Oprtation Mode */
#define	P2P_ONLY					(1<<0)	/* STA or P2P in single interface */
#define	P2P_CONCURRENT				(1<<1)	/* STA and P2P in different interface */

#define P2P_IS_GO					1
#define P2P_IS_CLIENT				2
#define P2P_IS_DEVICE				3
#define P2P_IS_CLIENT_IN_GROUP		4

#define P2P_SET_FLAG(_M, _F)       ((_M)->P2pFlag |= (_F))
#define P2P_CLEAR_FLAG(_M, _F)     ((_M)->P2pFlag &= ~(_F))
#define P2P_TEST_FLAG(_M, _F)      (((_M)->P2pFlag & (_F)) != 0)
#define P2P_TEST_FLAGS(_M, _F)     (((_M)->P2pFlag & (_F)) == (_F))


#define P2PFLAG_PROVISIONED						0x01
#define P2PFLAG_INVITE_ENABLED					0x02
#define P2PFLAG_DEVICE_DISCOVERABLE				0x04

#define P2P_OUI							0x099a6f50 /* spec. P2P OUI */
#define P2P_RANDOM_BASE					35	/*; 3 second. */
#define P2P_RANDOM_WPS_BASE				10	/*; 5 second. */
#define P2P_RANDOM_BIAS					10	/*; 2 second. */
#define P2P_RANDOM_PERST_BIAS			60	/*; 2 second. */
#define P2P_SCAN_CHANNEL_STEP			11
#define P2P_SCAN_PERIOD					1800	/* unit : 100ms. Scan period how long ? 2min ? */
#define P2P_SCAN_SHORTPERIOD			100	/* unit : 100ms. Scan period how long ? 2min ? */
#define P2P_EXT_LISTEN_INTERVAL			4000	/* unit is 1ms */
#define P2P_EXT_LISTEN_PERIOD			1000	/* unit is 1ms */

#define P2P_CHECK_GO_TIMER				300000	/* 3 minutes */
#define P2P_CHECK_CLIENT_TIMER			30000	/* 30 seconds */
#define P2P_WSC_TIMER					120000	/* 120 seconds */
#define P2P_TRANSMIT_TIMER				500		/* 500 ms */


#define ONETU							100

/* P2P Tab index */
#define P2P_NOT_FOUND                    0xFF

#define MINOR_REASON_SUCCESS				0
#define MINOR_REASON_CROSS_CONNECT			1
#define MINOR_REASON_MANAGED_BIT_ZERO		2
#define MINOR_REASON_COEXIST_PARM_BAD		3
#define MINOR_REASON_MANAGED_BIT_ONE		4


/* Invittion Flags */
#define P2P_INVITE_FLAG_REINVOKE		1


#define GROUP_MODE_TEMP		1
#define GROUP_MODE_PERSISTENT		2
#define MAX_P2P_GROUP_SIZE		30 /* Max mactab size(32) - 2 */

#ifdef WIDI_SUPPORT
#define MAX_P2P_TABLE_SIZE		25 /*  Save Presistent entry */
#else
#define MAX_P2P_TABLE_SIZE		8 /*  Save Presistent entry */
#endif /* WIDI_SUPPORT */

#define PROFILE_P2P		1
#define PROFILE_WPSE		2
#define P2P_DEVICE_TYPE_LEN		8
#define P2P_DEVICE_NAME_LEN		32

/* P2P Scan */
#define P2P_STOP_SCAN			0
#define P2P_SCANNING			1
#define P2P_STOP_SCAN_AND_LISTEN	2

#define CONNECTING_OR_NO_LINK		0
#define P2P_GO		1
#define P2P_CLIENT	2
#define P2P_DEVICE	3
#define TDLS_LINKED 4
#define WFD_SESSION_UNAVALIBLE	5
#define WFD_PEER_PC_P2P	6
#define WFD_PEER_TDLS_WEAK_SECURITY	7

/* P2P operation channel selection policy */
#define CHECK_PEER_CHANNEL_DISABLE	0
#define CHECK_PEER_CHANNEL_IF_NO_CONNECTION	1
#define CHECK_PEER_CHANNEL_EVEN_IF_CONNECTION	2


/* Describe the peer's state when I am performing P2P Operation with the peer. */
typedef	enum	_P2pClientState {
	P2PSTATE_NONE,
	P2PSTATE_DISCOVERY, /* Not associated. Because need to get SSID from Probe Response. So add this state. */
	P2PSTATE_DISCOVERY_GO,	/* this device is a GO. has beacon */
	P2PSTATE_DISCOVERY_CLIENT,	/* this device is a client that associates with a GO. (in a p2p group.) */
	P2PSTATE_DISCOVERY_UNKNOWN, /* Need to scan to decide this peer's rule is GO or Client or Device. */
	P2PSTATE_CLIENT_DISCO_COMMAND,	/* Do Client Discovery. */
	P2PSTATE_WAIT_GO_DISCO_ACK, /* Do Client Discovery. */
	P2PSTATE_WAIT_GO_DISCO_ACK_SUCCESS, /* Do Client Discovery. */
	P2PSTATE_GO_DISCO_COMMAND,	/* Need to send to this GO when doing Client Discovery */
	P2PSTATE_INVITE_COMMAND,	/* Wait to send Invite Req. */
	P2PSTATE_CONNECT_COMMAND,	/* wait to send Go Nego Req. */
	P2PSTATE_PROVISION_COMMAND, /* Provision first, then connect. */
	P2PSTATE_SERVICE_DISCOVER_INIT_COMMAND, /* Do Service Discovery. */
	P2PSTATE_SERVICE_COMEBACK_COMMAND, /* Do Service Discovery. */
	P2PSTATE_SENT_INVITE_REQ,
	P2PSTATE_SENT_PROVISION_REQ,
	P2PSTATE_SENT_PROVISION_RSP,
	P2PSTATE_WAIT_REVOKEINVITE_RSP_ACK, /* 15 */
	P2PSTATE_REVOKEINVITE_RSP_ACK_SUCCESS,
	P2PSTATE_SENT_GO_NEG_REQ,
	P2PSTATE_GOT_GO_RSP_INFO_UNAVAI,	/* got GO Nego Rsp with Status : information unavailable. Still need 120 sec more to time out. */
	P2PSTATE_WAIT_GO_COMFIRM,
	P2PSTATE_WAIT_GO_COMFIRM_ACK,
	P2PSTATE_GOT_GO_COMFIRM,
	P2PSTATE_GO_COMFIRM_ACK_SUCCESS,
	P2PSTATE_REINVOKEINVITE_TILLCONFIGTIME,
	P2PSTATE_GO_DONE,
	P2PSTATE_GO_WPS,	/* Internal registra */
	P2PSTATE_GO_AUTH,
	P2PSTATE_GO_ASSOC,		/*30 */
	P2PSTATE_CLIENT_WPS,		/* Enrollee */
	P2PSTATE_CLIENT_WPS_DONE,		/* Enrollee */
	P2PSTATE_CLIENT_AUTH,
	P2PSTATE_CLIENT_ASSOC,
	P2PSTATE_CLIENT_OPERATING,	 /* 35 */
	P2PSTATE_CLIENT_ABSENCE,
	P2PSTATE_CLIENT_SCAN,		/* Already in P2P group, but go to scan phase. */
	P2PSTATE_CLIENT_FIND,		/* Already in P2P group, but go to find phase. */
	P2PSTATE_GO_OPERATING,
	P2PSTATE_GO_ABSENCE,		/*During absence period. */
	P2PSTATE_GO_SCAN,		/* Already in P2P group, but go to scan phase. */
	P2PSTATE_GO_FIND,		/* Already in P2P group, but go to find phase. */
	/* Go can support legacy station not use WiFi Direct Spec to connect to. Assign following 2 states to such client. */
	P2PSTATE_NONP2P_PSK,		/* legacy client that uses WPA2PSK-AES to connect to GO(me). */
	P2PSTATE_NONP2P_WPS,		/* Legacy client that uses WPS to connect to GO(me). */
	P2PSTATE_MAX_STATE,
}	P2P_CLIENT_STATE;

typedef VOID(*p2p_cmd_handler) (VOID * pAD, VOID * pElem);
typedef struct _P2P_CMD_STRUCT {
	UCHAR	Addr[MAC_ADDR_LEN];
	UCHAR	Idx;
	/*USHORT	ConfigMethod; */

} P2P_CMD_STRUCT, *PP2P_CMD_STRUCT;

/* Describe the current discovery state and provistioning state if WifI direct is enabled. */
typedef	enum	_P2pDiscoProvState {
	P2P_DISABLE,
	P2P_ENABLE_LISTEN_ONLY,	/* In this state, only reponse to P2P Probe req. that has P2P IE. enable listen mode. */
	/*	P2P_IDLE, */
	/*	P2P_SEARCH_COMMAND, */	/* 5	 */
	P2P_SCAN,
	P2P_SEARCH,		/* 6 */
	/*	P2P_SEARCH_COMPLETE, */
	/*	P2P_LISTEN_COMMAND, */
	P2P_LISTEN,		/* 9 */
	/*	P2P_LISTEN_COMPLETE, */
	/*	P2PGO_SEARCH, */	/* I am alreayd an GO, But go to other listen channel perform search  */
	/*	P2PGO_SEARCH_COMMAND, */	/*  go to other listen channel perform search  */
	P2P_MAX_DISC_STATE,
}	P2P_DISCOPROV_STATE;

/* Describe the current state when performing P2P Operation. */
typedef	enum	_P2pConnectState {
	P2P_CONNECT_NOUSE,
	P2P_CONNECT_IDLE = 13,	/* Set to bigger number, so don't overlap with P2P_DISCOPROV_STATE */
	P2P_INVITE,
	P2P_ANY_IN_FORMATION_AS_GO,	/* 15 */
	P2P_ANY_IN_FORMATION_AS_CLIENT,
	P2P_DO_GO_NEG_DONE_CLIENT,		/* 17 */
	P2P_DO_GO_SCAN_OP_BEGIN,		/* 18 */
	P2P_DO_GO_SCAN_OP_DONE,		/* 19 */
	P2P_DO_GO_SCAN_BEGIN,
	P2P_DO_GO_SCAN_DONE,
	P2P_WPS_REGISTRA,		/* 22 */
	P2P_DO_WPS_ENROLLEE,
	/*	P2P_DO_WPS_ENROLLEE_DONE, */
	/*	P2P_GO_ASSOC_AUTH,	 */
	/*	P2P_I_AM_CLIENT_ASSOC_AUTH,	 */
	/*	P2P_I_AM_CLIENT_OP, */		/* 27 I am operating as client */
	/*	P2P_I_AM_GO_OP,	*/		/* 28 I am operatin as Go. */
	/*	P2PGO_DO_WPS_REGISTRA, */		/* 29 */
	/*	P2PGO_I_AM_GO_ASSOC_AUTH, */
}	P2P_CONNECT_STATE;



#define CONFIG_MODE_DISABLE_P2P_WPSE				0 /* Uncheck "enable WFD feature" #define CONFIG_MODE_DISABLE_WIFI_DIRECT_WPSE     0 */
#define CONFIG_MODE_ENABLE_P2P					1    /* set device name to driver when check "enable WFD feature" */
#define CONFIG_MODE_P2P_SCAN						2    /* set device name to driver when press "Scan" */
#define CONFIG_MODE_CONNECT_P2P					3  /* set all config (RTMP_WIFI_DIRECT_CONFIG) to driver #define CONFIG_MODE_ACTIVATE_WIFI_DIRECT           1 */
#define CONFIG_MODE_ACTIVATE_WPSE				4    /* #define CONFIG_MODE_ACTIVATE_WPSE                 2 */
#define CONFIG_MODE_DELETE_ACTIVE_P2P			5    /* set to actived config (RTMP_WIFI_DIRECT_CONFIG) to driver (driver will send disconnect event and clean persistent table ) #define CONFIG_MODE_DELETE_ACTIVED_WIFI_DIRECT    3 */
#define CONFIG_MODE_DISCONNECT_P2P				6    /* do nothing in UI when press "Disconnect" #define CONFIG_MODE_DISCONNECT_WIFI_DIRECT         4 */
#define CONFIG_MODE_DELETE_PERST_P2P				7    /* set inactive but persistent config (RTMP_WIFI_DIRECT_CONFIG) to driver when delete perseistent profile from profile list #define CONFIG_MODE_DELETE_PERST_WIFI_DIRECT    5 */
#define CONFIG_MODE_PROVISION_THIS				8    /*   */
#define CONFIG_MODE_SET_GO_WPS					9    /*  When I am GO. Set WPS for indivial client. */
#define CONFIG_MODE_SERVICE_DISCOVERY			10    /*  Start SErvice Discovery */

#define P2P_PHYMODE_LEGACY_ONLY				0
#define P2P_PHYMODE_ENABLE_11N_20			1
#define P2P_PHYMODE_ENABLE_11N_40			2

#define P2P_GROUP_MODE_TEMP					1
#define P2P_GROUP_MODE_PERSISTENT				2

#define    P2P_WPSE_PIN_METHOD					0x10000000
#define    P2P_WPSE_PBC_METHOD					0x20000000
#define    P2P_WPSE_SMPBC_METHOD				0x30000000

#define RALINKOUIMODE_IPREQ					0x30
#define RALINKOUIMODE_IPRSP					0x31
#define RALINKOUIMODE_MBRIPRSP				0x32	/* Group Member ip notify */

/*define P2P_REG_CM_LABEL						1 */	/* Default I broadcast my config method is label. (DefaultConfigMethod:2) */
#define P2P_REG_CM_DISPLAY						1	/* Default I broadcast my config method is display. */
#define P2P_REG_CM_KEYPAD						2	/* Default I broadcast my config method is keypad. */
#define P2P_REG_CM_PBC							3	/* Default I broadcast my config method is pbc. (DefaultConfigMethod:2) */

/* WSC connection mode */
/*define	WSC_PIN_MODE		0x10000000 */	/* Default PIN */
/*define	WSC_PBC_MODE		0x20000000 */
#define	WSC_PIN_MODE_USER_SPEC			0x40000000
#define	WSC_PIN_MODE_REGISTRA_SPEC		0x80000000

#define P2P_CONTROL_REG_ENABLE				0x00000001
#define P2P_CONTROL_REG_MANAGED				0x00000002
#define P2P_CONTROL_REG_PERSISTENT			0x00000004
#define P2P_CONTROL_REG_INVITE				0x00000008
#define P2P_CONTROL_REG_INTENT				0x000000F0
#define P2P_CONTROL_REG_LISTEN_CHANNEL		0x00000F00
#define P2P_CONTROL_REG_CONFIG_MHD			0x00007000
#define P2P_CONTROL_REG_INTRA_BSS			0x00008000
#define P2P_CONTROL_REG_OP_CHANNEL			0x000F0000
#define P2P_CONTROL_REG_CLI_DISCOVERY		0x00100000
#define P2P_CONTROL_REG_EXT_LISTEN			0x00200000
#define P2P_CONTROL_REG_SERVICE_DISCOVERY	0x00400000
#define P2P_CONTROL_REG_OPPS				0x00800000
#define P2P_CONTROL_REG_GROUP_LIMIT			0x01000000
#define P2P_CONTROL_REG_DISABLE_CRS_CONN	0x02000000
#define P2P_CONTROL_REG_SW_NOA				0x80000000

/* key information */
typedef	union		{
	struct	{
		/* 1 */
		ULONG		Enable:1;			/* Enable WiFi Direct. */
		ULONG		Managed:1;			/* Support Managed AP */
		ULONG		EnablePresistent:1;			/* Enable persistent */
		ULONG		EnableInvite:1;			/* For invitation Test Case. */
		/* 2 */
		ULONG		DefaultIntentIndex:4;			/* the default intent index GUI used when first editing a profile. */
		/* 3 */
		ULONG		ListenChannel:4;			/* Listen Channel */
		/* 4 */
		ULONG		DefaultConfigMethod:3;			/* Default Config Method that is set in Probe Rsp when P2P On. */
		ULONG		EnableIntraBss:1;			/*  1: Enable Intra BSS function when I am GO */
		/* 5 */
		ULONG		OpChannel:4;			/* default use 2.4GHz channel */
		/* 6 */
		ULONG		ClientDiscovery:1;			/* Client Discoverbility */
		ULONG		ExtendListen:1;			/* Extended Listening */
		ULONG		ServiceDiscovery:1;			/* Service Discovery */
		ULONG		OpPSAlwaysOn:1;			/* Service Discovery */
		/* 7 */
		ULONG		P2PGroupLimit:1;			/* When Limit == 1. GO only support ONE device in my group.. */
		ULONG		ForceDisableCrossConnect:1;
		ULONG		Rsvd:5;			/* Not Used */
		ULONG		SwBasedNoA:1;			/* Software Based NoA implementation */
	}	field;
	UINT32			word;
}	P2P_CONTROL, *PP2P_CONTROL;


/*
  *  The miniport PORT structure
  */
typedef struct _PORT_COMMON_CONFIG {
	ULONG   OpStatusFlags;
	ULONG   OperationMode; /* DOT11_OPERATION_MODE_EXTENSIBLE_STATION or DOT11_OPERATION_MODE_NETWORK_MONITOR */
	UCHAR	Bssid[MAC_ADDR_LEN];
	CHAR	Ssid[MAX_LEN_OF_SSID]; /* NOT NULL-terminated */
	UCHAR	SsidLen;               /* the actual ssid length in used */
	UCHAR	LastSsidLen;               /* the actual ssid length in used */
	CHAR	LastSsid[MAX_LEN_OF_SSID]; /* NOT NULL-terminated */
	UCHAR	LastBssid[MAC_ADDR_LEN];
	/*UCHAR	Channel; */
	/*UCHAR	CentralChannel; */	/* Central Channel when using 40MHz is indicating. not real channel. */

	ULONG	AuthMode;       /* This should match to whatever microsoft defined, use ULONG Intead of DOT11_AUTH_ALGORITHM.	 */
	ULONG	WepStatus;		/*use ULONG intead of DOT11_CIPHER_ALGORITHM,  */
	UCHAR	DefaultKeyId;

	ULONG	OrigWepStatus;	/* Original wep status set from OID */
	/* Add to support different cipher suite for WPA2/WPA mode */
	ULONG	GroupCipher;		/* Multicast cipher suite */
	ULONG	PairCipher;			/* Unicast cipher suite */
	UCHAR		CipherAlg;
	NDIS_802_11_WEP_STATUS	GroupKeyWepStatus;
	/*	RT_802_11_CIPHER_SUITE_TYPE			MixedModeGroupCipher; */ /* for WPA+WEP mixed mode, and CCKM WEP */
	BOOLEAN								bMixCipher;			/* Indicate current Pair & Group use different cipher suites */
	USHORT								RsnCapability;
	ULONG                   MulticastCipherAlgorithmCount;
	ULONG       PacketFilter;       /* Packet filter for receiving */

	UINT                    MCAddressCount;
	/*	UCHAR                   MCAddressList[HW_MAX_MCAST_LIST_SIZE][DOT11_ADDRESS_SIZE]; */

	BOOLEAN					bHiddenNetworkEnabled;
	BOOLEAN					ExcludeUnencrypted;

	ULONG                               OperatingPhyId;         /* ID of currently operating PHY */
	ULONG                               SelectedPhyId;          /* index of PHY that any PHY specific OID is applied to */
	ULONG                               DefaultPhyId;
} PORT_COMMON_CONFIG, *PPORT_COMMON_CONFIG;

typedef struct _P2P_COUNTER_STRUCT {
	ULONG		ManageAPEnReconCounter;	/* right after shutdown,  */
	ULONG		ClientConnectedCounter;	/* If I am P2P Client and is connected,  */
	ULONG		DisableRetryGrpFormCounter;	/* If Group Nego is rejected because both intent is 15. can't retry in a short time.  */
	ULONG		Counter100ms;	/* plus 1 every 100ms  */
	ULONG		GoScanBeginCounter100ms;
	ULONG		Wpa2pskCounter100ms;
	ULONG		NextScanRound;	/* Unit : 100 TU */
	UCHAR		ListenInterval;	/* Unit : 100 TU */
	UCHAR		ListenIntervalBias;	/* for some important action that need more dwell time in scan channel, add this Bias. Unit : 100 TU */
	ULONG		CounterAftrScanButton;		/* Unit 100ms. Counter for After Pressing "Apply" Profile button. */
	ULONG		CounterAftrSetEvent;		/* Unit 100ms. Counter for Driver called KeSetEvent. */
	BOOLEAN		bListen;
	BOOLEAN		bStartScan;
	BOOLEAN		bNextScan;
	ULONG		UserAccept;
} P2P_COUNTER_STRUCT, *PP2P_COUNTER_STRUCT;

typedef struct _P2P_RALINK_STRUCT {
	UCHAR		ListenChanel[3];
	UCHAR		ListenChanelIndex;
	UCHAR		ListenChanelCount;
} P2P_RALINK_STRUCT, *PP2P_RALINK_STRUCT;

/* often need to change beacon content. So save the offset. */
typedef struct _P2P_BEACONOFFSET_STRUCT {
	ULONG				IpBitmapBcnOffset;		/*  USed as GO */
	ULONG				P2pCapOffset;
} P2P_BEACONOFFSET_STRUCT, *PP2P_BEACONOFFSET_STRUCT;

typedef	struct	_P2P_SAVED_PUBLIC_FRAME	{
	HEADER_802_11   p80211Header;
	UCHAR          Category;
	UCHAR           Action;
	UCHAR           OUI[3];
	UCHAR		OUIType;
	UCHAR		Subtype;
	UCHAR		Token;
}	P2P_SAVED_PUBLIC_FRAME, *PP2P_SAVED_PUBLIC_FRAME;

typedef struct _P2P_MANAGED_STRUCT {
	UCHAR			ManageAPBSsid[MAC_ADDR_LEN];	/* Store the AP's BSSID that is the managed AP and is the latest one that I recently connected to.. */
	UCHAR			ManageAPSsid[32];	/* Store the AP's BSSID that is the managed AP and is the latest one that I recently connected to.. */
	UCHAR			ManageAPSsidLen;	/* Store the AP's BSSID that is the managed AP and is the latest one that I recently connected to.. */
	UCHAR			APP2pManageability;	/* Store the AP's P2P Manageability byte that currently I want to associate. */
	UCHAR			APP2pMinorReason;	/* Store the AP's minorreason byte that recently being deauthed */
	UCHAR			ICSStatus;	/*  */
	UCHAR			APUsageChannel;	/*	The suggest channel from AP's IE=0x61 */
	UCHAR			TotalNumOfP2pAttribute;
} P2P_MANAGED_STRUCT, *PP2P_MANAGED_STRUCT;

typedef	struct	_P2PCLIENT_NOA_SCHEDULE	{
	BOOLEAN		bValid;
	BOOLEAN		bInAwake;
	BOOLEAN		bNeedResumeNoA;	/* Set to TRUE if need to restart infinite NoA */
	BOOLEAN		bWMMPSInAbsent;	/* Set to TRUE if enter GO absent period by supported UAPSD GO */
	UCHAR		Token;
	ULONG		SwTimerTickCounter; /* this Counter os used for sw-base NoA implementation tick counter */
	ULONG		CurrentTargetTimePoint; /* For sw-base method NoA usage */
	ULONG           NextTargetTimePoint;
	ULONG           NextTimePointForWMMPSCounting;	/* fro counting WMM PS EOSP bit. Not used for NoA implementation. */
	UCHAR          Count;
	ULONG           Duration;
	ULONG           Interval;
	ULONG           StartTime;
	ULONG		OngoingAwakeTime; /* this time will keep increasing as time go by. indecate the current awake time point */
	ULONG           TsfHighByte;
	ULONG           ThreToWrapAround;
	ULONG			LastBeaconTimeStamp;
}	P2PCLIENT_NOA_SCHEDULE, *PP2PCLIENT_NOA_SCHEDULE;

typedef struct {
	UCHAR		Length;
	UCHAR		DevAddr[MAC_ADDR_LEN];
	UCHAR            InterfaceAddr[MAC_ADDR_LEN];
	UCHAR            Capability;
	UCHAR            ConfigMethod[2];
	UCHAR            PrimaryDevType[P2P_DEVICE_TYPE_LEN];
	UCHAR		NumSecondaryType;
	UCHAR		Octet[1];
} P2P_CLIENT_INFO_DESC, *PP2P_CLIENT_INFO_DESC;

/* Save for "Persistent" P2P table. Temporary P2P doesn't need to save in the table. */
typedef struct _RT_P2P_PERSISTENT_ENTRY {
	BOOLEAN		bValid;
	UCHAR		MyRule;		/* My rule is GO or Client  */
	UCHAR		Addr[MAC_ADDR_LEN];		/* this addr is to distinguish this persistent entry is for which mac addr   */
	WSC_CREDENTIAL	Profile;				/*  profile's bssid is always the GO's bssid. */
} RT_P2P_PERSISTENT_ENTRY, *PRT_P2P_PERSISTENT_ENTRY;

typedef struct _P2P_ETHER_ADDR {
	UCHAR octet[MAC_ADDR_LEN];
} P2P_ETHER_ADDR, *PP2P_ETHER_ADDR;

typedef struct _P2P_PEER_SSID {
	UCHAR ssid[32 + 1];
} P2P_PEER_SSID, *PP2P_PEER_SSID;

typedef struct _P2P_PEER_DEV_TYPE {
	UCHAR dev_type;
} P2P_PEER_DEV_TYPE, *PP2P_PEER_DEV_TYPE;

typedef struct _P2P_STA_ASSOC_LIST {
	P2P_ETHER_ADDR	maclist[3];
	UINT			maclist_count;
	P2P_PEER_SSID	device_name[3];
	P2P_PEER_DEV_TYPE	device_type[3];
	BOOLEAN			is_p2p[3];
} P2P_STA_ASSOC_LIST, *PP2P_STA_ASSOC_LIST;

/* Store for persistent P2P group */
typedef struct _RT_P2P_CONFIG {
	/*	PNET_DEV					dev; */
	UCHAR						P2p_OpMode;
	UCHAR						CurrentAddress[MAC_ADDR_LEN];
	P2P_CTRL_STATE				CtrlCurrentState;
	STATE_MACHINE				P2PCtrlMachine;
	STATE_MACHINE_FUNC			P2PCtrlFunc[P2P_CTRL_FUNC_SIZE];
	P2P_DISC_STATE				DiscCurrentState;
	STATE_MACHINE				P2PDiscMachine;
	STATE_MACHINE_FUNC			P2PDiscFunc[P2P_DISC_FUNC_SIZE];
	P2P_GO_FORM_STATE			GoFormCurrentState;
	STATE_MACHINE				P2PGoFormMachine;
	STATE_MACHINE_FUNC			P2PGoFormFunc[P2P_GO_FORM_FUNC_SIZE];
	P2P_ACTION_STATE			ActionState;
	STATE_MACHINE				P2PActionMachine;
	STATE_MACHINE_FUNC			P2PActionFunc[P2P_ACTION_FUNC_SIZE];
	UCHAR						Rule;		/* Device / Client / GO. */


	/*	P2P_DISCOPROV_STATE		P2PDiscoProvState; */		/* P2P State for P2P discovery(listen, scan, search) and P2P provistioning (Service provision, wps provision) */
	P2P_CONNECT_STATE			P2PConnectState;		/* P2P State for P2P connection. */
	/*	P2P_EVENTQUEUE_STRUCT	P2pEventQueue; */
	P2P_COUNTER_STRUCT			P2pCounter;
	P2P_RALINK_STRUCT			P2pProprietary;	/* Ralink Proprietary to improve usage. */
	P2P_BEACONOFFSET_STRUCT	P2pBcnOffset;
	P2P_SAVED_PUBLIC_FRAME		LatestP2pPublicFrame;	/* Latest received P2P Public frame */
	P2P_MANAGED_STRUCT			P2pManagedParm;

	BOOLEAN						bFirstTimeCancelOpps;
	BOOLEAN						bPreKeepSlient;
	BOOLEAN						bKeepSlient;
	UINT16						MyGOwcid;
	UCHAR						LastSentInviteFlag;
	UCHAR						P2pCapability[2];
	ULONG						MyIp;
	WSC_DEV_INFO				DevInfo;
	UCHAR						Wsc_Uuid_E[UUID_LEN_STR];
	UCHAR						Wsc_Uuid_Str[UUID_LEN_STR];
	ULONG						WscMode;		/* predefined WSC mode, 1: PIN, 2: PBC */
	BOOLEAN						bConfiguredAP;	 /* true (1:un-configured), false(2: configured) ; used by GO */
	USHORT						Dpid;	/* WPS device password ID. */
	USHORT						ConfigMethod;	/* WPS device password ID. */
	UCHAR						PinCode[9];
	UCHAR						ConfigTimeout[2];
	UCHAR						AssocPriDeviceType[8];
	UCHAR						PhraseKey[64];
	UCHAR						PhraseKeyLen;
	P2P_STA_ASSOC_LIST				AssocList;
	UCHAR						P2pPhyMode;
	UCHAR						NoAIndex;
	UCHAR						PopUpIndex;	/* store the p2p entry index when receiving Go Nego Req and need a GUI to pop up a window to set connection. */
	UCHAR						ConnectingIndex;	/* Point to the ?rd Connecting MAC that I will try to connect with. */
	UCHAR						ConnectingMAC[MAC_ADDR_LEN];  /* Specify MAC address want to connect. Set to all 0xff or all 0x0 if not specified. */
	UCHAR						ConnectingDeviceName[MAX_P2P_GROUP_SIZE][32];  /* Specify the Device Name that want to connect. Set to all 0xff or all 0x0 if not specified. */
	UCHAR						SSID[32];
	UCHAR						SSIDLen;
	UCHAR						Token;
	UCHAR						Bssid[MAC_ADDR_LEN];
	UCHAR						Manufacturer[64];
	UCHAR						ManufacturerLen;
	UCHAR						ModelName[32];
	UCHAR						ModelNameLen;
	UCHAR						ModelNumber[32];
	UCHAR						ModelNumberLen;
	UCHAR						SerialNumber[32];
	UCHAR						SerialNumberLen;
	UCHAR						DeviceName[32];
	ULONG						DeviceNameLen;
	UCHAR						ListenChannel;	 /* The channel that perform the group formatuib oricedure. */
	UCHAR						GroupChannel;	 /* Group setting channel from GUI. Real OP channel need to be negociated.. */
	UCHAR						GroupOpChannel;	 /* Group operating channel. */
	UCHAR						GoIntentIdx;	/* Value = 0~15. Intent to be a GO in P2P */
	BOOLEAN						bIntraBss;
	BOOLEAN						bExtListen;
	USHORT						ExtListenPeriod;	/* Period for extended listening */
	USHORT						ExtListenInterval;	/* Interval for extended listening */
	UCHAR						CTWindows;	/* CTWindows and OppPS parameter field */
	ULONG						GOBeaconBufWscLen;
	UCHAR						LastConfigMode;	/* Disable, activate p2p, or activate WPSE, or delete p2p profile. */

	P2PCLIENT_NOA_SCHEDULE					GONoASchedule;
	RALINK_TIMER_STRUCT						P2pCTWindowTimer;
	RALINK_TIMER_STRUCT						P2pSwNoATimer;
	RALINK_TIMER_STRUCT						P2pPreAbsenTimer;
	RALINK_TIMER_STRUCT						P2pWscTimer;
	RALINK_TIMER_STRUCT						P2pReSendTimer;
	BOOLEAN									bP2pReSendTimerRunning;
	RALINK_TIMER_STRUCT						P2pCliReConnectTimer;
	BOOLEAN									bP2pCliReConnectTimerRunning;

	PORT_COMMON_CONFIG					PortCfg;
	UCHAR									DefaultConfigMethod;
	P2P_CONTROL				P2pControl;
	INT							WscState;
	BOOLEAN					bSigmaEnabled;
	BOOLEAN					bP2pCliPmEnable;
	BOOLEAN					bLowRateQoSNULL;
	BOOLEAN					bP2pCliReConnect;
	BOOLEAN					bStopAuthRsp;
	ULONG					DevDiscPeriod;		/* Unit 100ms */
	BOOLEAN					bPeriodicListen;
	BOOLEAN					bConfirmByUI;
	BOOLEAN					bP2pAutoAccept;
	INT					p2pMaxEntry;

	BOOLEAN						bProvAutoRsp;
	UCHAR						P2pProvIndex;
	USHORT						P2pProvConfigMethod;
	UCHAR						P2pProvToken;
	BOOLEAN						P2pProvUserNotify;
	PUCHAR						pGoNegoRspOutBuffer;
	BOOLEAN					bSentProbeRSP;
	BOOLEAN					bWaitGoNego;

	UCHAR					p2pidxForServiceCbReq;
	UCHAR					ServiceTransac;
	UCHAR					CheckPeerChannelPolicy;
	BOOLEAN					bP2pGoAcceptInvitationReq;

#ifdef WIDI_SUPPORT
	BOOLEAN				bWIDI;
	BOOLEAN					bP2pCliOnly;
#endif /* WIDI_SUPPORT */
	BOOLEAN                                 bAutoChannel;
	BOOLEAN                                 bAutoChannelAtBootup;
	BOOLEAN                                 bAutoChOnSocial;
	UCHAR                                   AutoChannelAlg;

} RT_P2P_CONFIG, *PRT_P2P_CONFIG;

typedef struct _RT_P2P_CLIENT_ENTRY {
	P2P_CLIENT_STATE		P2pClientState;	/* From the state, can know peer it registra or enrollee. */
	UCHAR					MyGOIndex;	/* If this device is a client in a P2P group, then this is its GO's table index. */
	ULONG					Peerip;
	CHAR					Dbm;
	UCHAR					P2pFlag;
	UCHAR					NoAToken;
	UCHAR					GeneralToken;
	UCHAR					StateCount;
	UCHAR					Rule;
	UCHAR					DevCapability;	/* table 10 */
	UCHAR					GroupCapability;
	USHORT					ConfigMethod;
	UCHAR					PIN[8];
	USHORT					Dpid;
	UCHAR					Ssid[MAX_LEN_OF_SSID];
	UCHAR					SsidLen;
	UCHAR					GoIntent;
	UCHAR					addr[MAC_ADDR_LEN];
	UCHAR					InterfaceAddr[MAC_ADDR_LEN];
	UCHAR					bssid[MAC_ADDR_LEN];
	UCHAR					PrimaryDevType[P2P_DEVICE_TYPE_LEN];
	UCHAR					NumSecondaryType;
	UCHAR					SecondaryDevType[P2P_DEVICE_TYPE_LEN];
	UCHAR					RegClass;
	UCHAR					ChannelNr;
	UCHAR					OpChannel;
	UCHAR					DeviceName[P2P_DEVICE_NAME_LEN];
	ULONG					DeviceNameLen;
	UCHAR					ConfigTimeOut;
	UCHAR					ListenChannel;
	ULONG					WscMode;
	USHORT					ExtListenPeriod;	/* Period for extended listening */
	USHORT					ExtListenInterval;	/* Interval for extended listening */
	UCHAR					CTWindow;	/* As GO, Store client's Presence request NoA.  As Client, Store GO's NoA In beacon or P2P Action frame */
	BOOLEAN					bValid;
	P2PCLIENT_NOA_SCHEDULE	NoADesc[1];	/* As GO, Store client's Presence request NoA.  As Client, Store GO's NoA In beacon or P2P Action frame */
	CHAR					Rssi;

	ULONG					ReTransmitCnt;
	UCHAR					DialogToken;
	BOOLEAN					bSupport5G;
	BOOLEAN					bSCCChannel;
	BOOLEAN					bGoUsePeerChannel;

#
#ifdef WFD_SUPPORT
	WFD_ENTRY_INFO			WfdEntryInfo;
#endif /* WFD_SUPPORT */
} RT_P2P_CLIENT_ENTRY, *PRT_P2P_CLIENT_ENTRY;

typedef struct _P2P_ENTRY_PARM {
	P2P_CLIENT_STATE		P2pClientState;	/* From the state, can know peer it registra or enrollee. */
	UCHAR					P2pFlag;
	UCHAR					NoAToken;
	UCHAR					GeneralToken;
	UCHAR					DevCapability;	/* table 10 */
	UCHAR					GroupCapability;
	UCHAR					DevAddr[MAC_ADDR_LEN];	/* P2P Device Address */
	UCHAR					InterfaceAddr[MAC_ADDR_LEN];
	UCHAR					PrimaryDevType[P2P_DEVICE_TYPE_LEN];
	UCHAR					NumSecondaryType;
	UCHAR					SecondaryDevType[P2P_DEVICE_TYPE_LEN];	/* This definition only support save one 2ndary DevType */
	UCHAR					DeviceName[32];
	ULONG					DeviceNameLen;
	USHORT					ConfigMethod;
	/*	USHORT		ExtListenPeriod; */	/* Period for extended listening */
	/*	USHORT		ExtListenInterval; */	/* Interval for extended listening */
	UCHAR					CTWindow;	/* As GO, Store client's Presence request NoA.  As Client, Store GO's NoA In beacon or P2P Action frame */
	P2PCLIENT_NOA_SCHEDULE	NoADesc[1];	/* As GO, Store client's Presence request NoA.  As Client, Store GO's NoA In beacon or P2P Action frame */
	UCHAR					p2pIndex;
} P2P_ENTRY_PARM, *PP2P_ENTRY_PARM;

/* Save for "Persistent" P2P table. Temporary P2P doesn't need to save in the table. */
typedef struct _RT_GO_CREDENTIAL_ENTRY {
	BOOLEAN		bValid;
	UCHAR		InterAddr[MAC_ADDR_LEN];		/* this addr is to distinguish this persistent entry is for which mac addr   */
	WSC_CREDENTIAL	Profile;				/*  profile's bssid is always the GO's bssid.  */
} RT_GO_CREDENTIAL_ENTRY, *PRT_GO_CREDENTIAL_ENTRY;

/* Store for persistent P2P group */
typedef struct _RT_P2P_TABLE {
	UCHAR						PerstNumber;		/* What persistent profile is set ? */
	UCHAR						ClientNumber;	 /* What clients are in my group now? */
	RT_P2P_PERSISTENT_ENTRY		PerstEntry[MAX_P2P_TABLE_SIZE]; /* Save persistent profile for auto reconnect */
	RT_P2P_CLIENT_ENTRY			Client[MAX_P2P_GROUP_SIZE];	/* Store for current group member. */
	RT_GO_CREDENTIAL_ENTRY		TempCredential[2/*MAX_P2P_SAVECREDN_SIZE*/];
} RT_P2P_TABLE, *PRT_P2P_TABLE;

typedef struct _RT_P2P_UI_TABLE {
	UCHAR						ClientNumber;	 /* What clients are in my group now? */
	RT_P2P_CLIENT_ENTRY			Client[MAX_P2P_GROUP_SIZE];	/* Store for current group member. */
} RT_P2P_UI_TABLE, *PRT_P2P_UI_TABLE;

#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
typedef struct _RT_P2P_DEV_FOUND {
	UCHAR addr[MAC_ADDR_LEN];
	UCHAR dev_addr[MAC_ADDR_LEN];
	UCHAR pri_dev_type[P2P_DEVICE_TYPE_LEN];
	UCHAR dev_name[32];
	USHORT config_methods;
	USHORT dpid;
	UCHAR dev_capab;
	UCHAR group_capab;
	CHAR rssi;
#ifdef WFD_SUPPORT
	UCHAR	bWfdClient;
	UCHAR	wfd_devive_type;
	UCHAR	source_coupled;
	UCHAR	sink_coupled;
	UCHAR	session_avail;
	UCHAR	wfd_service_discovery;
	UCHAR	wfd_PC;
	UCHAR	wfd_CP;
	UCHAR	wfd_time_sync;
	USHORT	rtsp_port;
	USHORT	max_throughput;
	UCHAR	assoc_addr[MAC_ADDR_LEN];
	WFD_COUPLED_SINK_INFO	coupled_sink_status;
	UCHAR	coupled_peer_addr[MAC_ADDR_LEN];
#endif /* WFD_SUPPORT */
} RT_P2P_DEV_FOUND, *PRT_P2P_DEV_FOUND;

typedef struct _RT_P2P_PROV_DISC_REQ {
	UCHAR peer[MAC_ADDR_LEN];
	USHORT config_methods;
	UCHAR dev_addr[MAC_ADDR_LEN];
	UCHAR pri_dev_type[P2P_DEVICE_TYPE_LEN];
	UCHAR dev_name[32];
	USHORT supp_config_methods;
	UCHAR dev_capab;
	UCHAR group_capab;
} RT_P2P_PROV_DISC_REQ, *PRT_P2P_PROV_DISC_REQ;

typedef struct _RT_P2P_PROV_DISC_RESP {
	UCHAR peer[MAC_ADDR_LEN];
	USHORT config_methods;
} RT_P2P_PROV_DISC_RESP, *PRT_P2P_PROV_DISC_RESP;

typedef struct _RT_P2P_GO_NEG_REQ_RX {
	UCHAR src[MAC_ADDR_LEN];
	USHORT dev_passwd_id;
} RT_P2P_GO_NEG_REQ_RX, *PRT_P2P_GO_NEG_REQ_RX;

typedef struct _RT_P2P_GROUP_INFO {
	UCHAR Rule;
	UCHAR Bssid[MAC_ADDR_LEN];
	UCHAR Ssid[32];
	UCHAR peer[MAC_ADDR_LEN];
#ifdef WFD_SUPPORT
	USHORT rtsp_port;
#endif /* WFD_SUPPORT */
} RT_P2P_GROUP_INFO, *PRT_P2P_GROUP_INFO;
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */


#endif /* __P2P_CMM_H__ */
#endif /* P2P_SUPPORT */

