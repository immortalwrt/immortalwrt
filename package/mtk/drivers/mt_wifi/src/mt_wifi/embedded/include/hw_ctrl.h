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
	hw_ctrl.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __HW_CTRL_H__
#define __HW_CTRL_H__
#include "rtmp_type.h"
#include "rtmp_os.h"
#include "security/wpa_cmm.h"
#include "protocol/protection.h"

struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _EDCA_PARM;
struct _WMM_CFG;
struct wifi_dev;
struct _BSS_INFO_ARGUMENT_T;
struct _MAC_TABLE_ENTRY;
struct WIFI_SYS_CTRL;
struct _STA_ADMIN_CONFIG;
struct twt_agrt_para;

typedef NTSTATUS(*HwCmdCb)(struct _RTMP_ADAPTER *pAd, VOID * Args);


#define HWCTRL_CMD_TIMEOUT (100 << 1)
#define HWCTRL_CMD_WAITTIME (5000 << 1)
#define ETSI_RXBLOCKER4R 4
#define ETSI_RXBLOCKER1R 1

#define MAX_LEN_OF_HWCTRL_QUEUE            (MAX_LEN_OF_MAC_TABLE<<2)
#define HWCTRL_QUE_SCH				16

/*for command classify*/
enum {
	HWCMD_TYPE_FIRST = 0,
	HWCMD_TYPE_RADIO = HWCMD_TYPE_FIRST, /*Need Radio Resource Mgmt Related*/
	HWCMD_TYPE_SECURITY,					 /*Security related*/
	HWCMD_TYPE_PERIPHERAL,				/*Peripheral related*/
	HWCMD_TYPE_HT_CAP,						/*HT related*/
	HWCMD_TYPE_PS,							/*Power Saving related*/
	HWCMD_TYPE_WIFISYS,
	HWCMD_TYPE_WMM,
	HWCMD_TYPE_PROTECT,
	HWCMD_TYPE_END
};


/*for command ID*/
enum {
	HWCMD_ID_FIRST = 0,
	/*Peripheral*/
	HWCMD_ID_GPIO_CHECK			= HWCMD_ID_FIRST,
	/*WSC & LED related*/
	HWCMD_ID_SET_LED_STATUS			= 3,
	HWCMD_ID_LED_WPS_MODE10			= 4,
	HWCMD_ID_LED_GPIO_MAP			= 5,
	/*Security related*/
	HWCMD_ID_DEL_ASIC_WCID			= 7,
#ifdef HTC_DECRYPT_IOT
	HWCMD_ID_SET_ASIC_AAD_OM		= 8,
#endif /* HTC_DECRYPT_IOT */
	HWCMD_ID_ADDREMOVE_ASIC_KEY		= 9,
	/*MT_MAC */
	HWCMD_ID_SET_CLIENT_MAC_ENTRY	= 10,
	HWCMD_ID_PS_CLEAR				= 11,
	HWCMD_ID_PS_RETRIEVE_START		= 12,
	HWCMD_ID_SET_TR_ENTRY			= 13,
	HWCMD_ID_UPDATE_DAW_COUNTER		= 14,
	HWCMD_ID_UPDATE_BEACON			= 15,
	HWCMD_ID_GET_TEMPERATURE		= 16,
	HWCMD_ID_SET_SLOTTIME			= 17,
	HWCMD_ID_SET_TX_BURST			= 18,
#ifdef TXBF_SUPPORT
	HWCMD_ID_SET_APCLI_BF_CAP		= 19,
	HWCMD_ID_SET_APCLI_BF_REPEATER	= 20,
	HWCMD_ID_ADJUST_STA_BF_SOUNDING	= 21,
	HWCMD_ID_TXBF_TX_APPLY_CTRL		= 22,
#endif /* TXBF_SUPPORT */
#ifdef ERR_RECOVERY
	HWCMD_ID_MAC_ERROR_DETECT		= 23,
#endif /* ERR_RECOVERY */
	/*AP realted*/
	HWCMD_ID_AP_ADJUST_EXP_ACK_TIME	= 24,
	HWCMD_ID_AP_RECOVER_EXP_ACK_TIME = 25,
	HWCMD_ID_UPDATE_BSSINFO			= 26,
	HWCMD_ID_SET_BA_REC				= 27,
	/*STA related*/
	HWCMD_ID_PWR_MGT_BIT_WIFI		= 28,
	HWCMD_ID_FORCE_WAKE_UP			= 29,
	HWCMD_ID_FORCE_SLEEP_AUTO_WAKEUP = 30,
	HWCMD_ID_MAKE_FW_OWN			= 31,
	HWCMD_ID_ENTER_PS_NULL			= 32,
	HWCMD_ID_SET_STA_DWRR			= 33,
	HWCMD_ID_UPDATE_RSSI			= 34,
	HWCMD_ID_SET_STA_DWRR_QUANTUM	= 35,
#ifdef CONFIG_STA_SUPPORT
	HWCMD_ID_PERODIC_CR_ACCESS_MLME_DYNAMIC_TX_RATE_SWITCHING	= 36,
#endif /* CONFIG_STA_SUPPORT */
	HWCMD_ID_PERODIC_CR_ACCESS_NIC_UPDATE_RAW_COUNTERS			= 37,
	HWCMD_ID_SET_BCN_OFFLOAD		= 38,
	HWCMD_ID_ADD_REPT_ENTRY			= 39,
	HWCMD_ID_REMOVE_REPT_ENTRY		= 40,
	HWCMD_ID_WIFISYS_LINKDOWN		= 41,
	HWCMD_ID_WIFISYS_LINKUP			= 42,
	HWCMD_ID_WIFISYS_OPEN			= 43,
	HWCMD_ID_WIFISYS_CLOSE			= 44,
	HWCMD_ID_WIFISYS_PEER_LINKUP	= 45,
	HWCMD_ID_WIFISYS_PEER_LINKDOWN	= 46,
	HWCMD_ID_WIFISYS_PEER_UPDATE	= 47,
	HWCMD_ID_THERMAL_PROTECTION_RADIOOFF = 48,
	HWCMD_ID_GET_TX_STATISTIC		= 49,
	HWCMD_ID_RADIO_ON_OFF			= 50,
	HWCMD_ID_PBC_CTRL				= 51,
#ifdef GREENAP_SUPPORT
	HWCMD_ID_GREENAP_ON_OFF			= 52,
#endif /* GREENAP_SUPPORT */
#ifdef LINK_TEST_SUPPORT
	HWCMD_ID_AUTO_LINK_TEST         = 53,
#endif /* LINK_TEST_SUPPORT */
	HWCMD_ID_HT_PROTECT				= 54,
	HWCMD_ID_RTS_THLD				= 55,
#ifdef HOST_RESUME_DONE_ACK_SUPPORT
	HWCMD_ID_HOST_RESUME_DONE_ACK	= 56,
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */
#ifdef ETSI_RX_BLOCKER_SUPPORT
	HWCMD_RX_CHECK_RSSI        = 57,
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	HWCMD_ID_PCIE_ASPM_DYM_CTRL		= 58,
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
#ifdef MBO_SUPPORT
	HWCMD_ID_BSS_TERMINATION        = 59,
#endif /* MBO_SUPPORT */
	HWCMD_ID_PART_SET_WMM			= 60,
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	HWCMD_ID_TWT_AGRT_UPDATE		= 61,
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
#ifdef OCE_SUPPORT
	HWCMD_ID_SET_FD_FRAME_OFFLOAD	= 62,
#endif /* OCE_SUPPORT */
#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
	HWCMD_ID_UPDATE_4ADDR_HDR_TRANS,
#endif
	HWCMD_ID_UPDATE_MIB_COUNTER		= 64,
#ifdef NF_SUPPORT_V2
	HWCMD_ID_GET_NF_BY_FW			= 65,
#endif
#ifdef WIFI_MD_COEX_SUPPORT
	HWCMD_ID_WIFI_COEX_APCCCI2FW	= 66,
	HWCMD_ID_QUERY_LTE_SAFE_CHANNEL			= 67,
#endif
#ifdef VOW_SUPPORT
	HWCMD_ID_SET_VOW_SCHEDULE_CTRL			= 68,
#endif
#ifdef CFG_SUPPORT_CSI
	HWCMD_ID_GET_CSI_RAW_DATA		= 69,
#endif
#ifdef DABS_QOS
	HWCMD_ID_SET_DEL_QOS				= 70,
#endif
#ifdef WTBL_TDD_SUPPORT
	HWCMD_ID_RW_WTBL_ALL		= 71,
	HWCMD_ID_WTBL_TDD_SWAP_OUT	= 72,
	HWCMD_ID_WTBL_TDD_SWAP_IN	= 73,
#endif /* WTBL_TDD_SUPPORT */
	HWCMD_ID_UPDATE_TX_PER					= 74,
	HWCMD_ID_UPDATE_SNR			= 75,
#ifdef ZERO_LOSS_CSA_SUPPORT
	HWCMD_ID_HANDLE_NULL_ACK_EVENT		= 76,
	HWCMD_ID_HANDLE_STA_NULL_ACK_TIMEOUT = 77,
#endif /*ZERO_LOSS_CSA_SUPPORT*/
#ifdef WIFI_MD_COEX_SUPPORT
	HWCMD_ID_SET_IDC_STATE 		= 78,
#endif
	HWCMD_ID_GET_TSF			= 79,
#ifdef PS_STA_FLUSH_SUPPORT
	HWCMD_ID_PS_FLOW_CTRL = 80,
#endif
#ifdef OFFCHANNEL_ZERO_LOSS
	HWCMD_ID_UPDATE_CHANNEL_STATS = 81,
 #endif
#ifdef WIFI_MD_COEX_SUPPORT
#ifdef COEX_DIRECT_PATH
	HWCMD_ID_WIFI_UPDATE_3WIRE_GRP	= 82,
#endif
#endif

	HWCMD_ID_END,
};


/*for flag ID, is bit mask, 1/2/4/8*/
enum {
	HWFLAG_ID_FIRST = 0,
	HWFLAG_ID_UPDATE_PROTECT = 1 << 0,
	HWFLAG_ID_END,
};

/*HwCtrl CMD structure*/
typedef struct _HwCmdQElmt {
	UINT32 type;
	UINT32 command;
	VOID *buffer;
	UINT32 bufferlength;
	BOOLEAN NeedWait;
	UINT32 WaitTime; /* ms */
	RTMP_OS_COMPLETION ack_done;
	VOID *RspBuffer;
	UINT32 RspBufferLen;
	HwCmdCb CallbackFun;
	VOID *CallbackArgs;
	NDIS_SPIN_LOCK lock;
	os_kref refcnt;
#ifdef DBG_STARVATION
	struct starv_dbg starv;
#endif /*DBG_STARVATION*/
	struct _HwCmdQElmt *next;
} HwCmdQElmt, *PHwCmdQElmt;

enum {
	HW_CMDQ_UTIL_TIME = 1 << 0,
	HW_CMDQ_UTIL_CONDITION_DROP = 1 << 1,
	HW_CMDQ_UTIL_WAIT_TIME_ADJ = 1 << 2,
	HW_CMDQ_UTIL_END,
};

typedef struct _HwCmdQ {
	UINT32 size;
	UINT32 max_size;
	HwCmdQElmt *head;
	HwCmdQElmt *tail;
	UINT32 CmdQState;
	UINT32 util_flag;
	ULONG LastQfullTime; /* jffies */
	UINT32 TotalWaitCnt;
	UINT32 TotalWaitTime; /* ms */
} HwCmdQ, *PHwCmdQ;

typedef struct _HW_CTRL_TXD {
	UINT32			CmdType;
	UINT32			CmdId;
	BOOLEAN			NeedWait;
	UINT32			wait_time;
	VOID			*pInformationBuffer;
	UINT32			InformationBufferLength;
	VOID			*pRespBuffer;
	UINT32			RespBufferLength;
	HwCmdCb		CallbackFun;
	VOID			*CallbackArgs;
} HW_CTRL_TXD;


/* dump timer log if ser execute time > threshold  */
#define SER_TIME_THRESHOLD		1000000		/* 1s, unit: us */

enum {
	SER_TIME_ID_T0 = 0,
	SER_TIME_ID_T1,
	SER_TIME_ID_T2,
	SER_TIME_ID_T3,
	SER_TIME_ID_T4,
	SER_TIME_ID_T5,
	SER_TIME_ID_T6,
	SER_TIME_ID_T7,
	SER_TIME_ID_END,
};

typedef struct _HWCTRL_OP {
	NTSTATUS (*wifi_sys_open)(struct WIFI_SYS_CTRL *wsys);
	NTSTATUS (*wifi_sys_close)(struct WIFI_SYS_CTRL *wsys);
	NTSTATUS (*wifi_sys_link_up)(struct WIFI_SYS_CTRL *wsys);
	NTSTATUS (*wifi_sys_link_down)(struct WIFI_SYS_CTRL *wsys);
	NTSTATUS (*wifi_sys_connt_act)(struct WIFI_SYS_CTRL *wsys);
	NTSTATUS (*wifi_sys_disconnt_act)(struct WIFI_SYS_CTRL *wsys);
	NTSTATUS (*wifi_sys_peer_update)(struct WIFI_SYS_CTRL *wsys);
} HWCTRL_OP;

typedef struct _HW_CTRL_T {
	HwCmdQ HwCtrlQ;
	NDIS_SPIN_LOCK HwCtrlQLock;	/* CmdQLock spinlock */
	RTMP_OS_TASK HwCtrlTask;
	UINT32 TotalCnt;
	HWCTRL_OP hwctrl_ops;
#ifdef ERR_RECOVERY
	RTMP_OS_TASK ser_task;
	INT ser_func_state;
	UINT32 ser_status;
	NDIS_SPIN_LOCK ser_lock;
	UINT32 ser_times[SER_TIME_ID_END];
#endif /* ERR_RECOVERY */
#ifdef DBG_STARVATION
	struct starv_dbg_block block;
#endif /*DBG_STARVATION*/
} HW_CTRL_T;


/*CMD structure */
typedef struct _RT_SET_ASIC_WCID {
	ULONG WCID;		/* mechanism for rekeying: 0:disable, 1: time-based, 2: packet-based */
	ULONG SetTid;		/* time-based: seconds, packet-based: kilo-packets */
	ULONG DeleteTid;	/* time-based: seconds, packet-based: kilo-packets */
	UCHAR Addr[MAC_ADDR_LEN];	/* avoid in interrupt when write key */
	UCHAR Tid;
	UINT16 SN;
	UCHAR Basize;
	INT   Ses_type;
	BOOLEAN IsAdd;
	BOOLEAN IsBMC;
	BOOLEAN IsReset;
} RT_SET_ASIC_WCID, *PRT_SET_ASIC_WCID;

#ifdef HTC_DECRYPT_IOT
typedef struct _RT_SET_ASIC_AAD_OM {
	ULONG WCID;
	UCHAR Value; /* 0 ==> off, 1 ==> on */
} RT_SET_ASIC_AAD_OM, *PRT_SET_ASIC_AAD_OM;
#endif /* HTC_DECRYPT_IOT */

#ifdef WTBL_TDD_SUPPORT
typedef struct _RT_RW_WTBL_ALL {
	UINT16 WCID;
	UINT16 inActWCID;
	UCHAR SegIdx;
	UCHAR Op;
    UCHAR ucWtblBegin;
    UCHAR ucWtblDwCnt;
	UINT32	au4WtblBuffer[5];    /* need 4 byte alignment */
} RT_RW_WTBL_ALL, *PRT_RW_WTBL_ALL;
#endif /* WTBL_TDD_SUPPORT */

#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
typedef struct _RT_ASIC_4ADDR_HDR_TRANS {
	ULONG  Wcid;
	UCHAR  Enable;
} RT_ASIC_4ADDR_HDR_TRANS, *PRT_ASIC_4ADDR_HDR_TRANS;
#endif

/*MT MAC Specific*/
typedef enum _SEC_ASIC_KEY_OPERATION {
	SEC_ASIC_ADD_PAIRWISE_KEY,
	SEC_ASIC_REMOVE_PAIRWISE_KEY,
	SEC_ASIC_ADD_GROUP_KEY,
	SEC_ASIC_REMOVE_GROUP_KEY,
} SEC_ASIC_KEY_OPERATION;

/*MT MAC Specific*/
typedef enum _SEC_ASIC_KEY_DIRECTION {
	SEC_ASIC_KEY_TX,
	SEC_ASIC_KEY_RX,
	SEC_ASIC_KEY_BOTH,
} SEC_ASIC_KEY_DIRECTION;

typedef struct _SEC_KEY_INFO {
	UCHAR Key[32]; /* TK(32) */
	UCHAR TxMic[8];
	UCHAR RxMic[8];
	UCHAR TxTsc[16]; /* TSC value. Change it from 48bit to 128bit */
	UCHAR RxTsc[16]; /* TSC value. Change it from 48bit to 128bit */
	UCHAR KeyLen; /* Key length for each key, 0: entry is invalid */
} SEC_KEY_INFO, *PSEC_KEY_INFO;

typedef struct _ASIC_SEC_INFO {
	SEC_ASIC_KEY_OPERATION Operation;
	SEC_ASIC_KEY_DIRECTION Direction;
	UINT32 Cipher;
	UINT16 Wcid;
	UCHAR BssIndex;
	UCHAR KeyIdx;
	SEC_KEY_INFO Key;
	UCHAR IGTK[32];
	UCHAR IGTKKeyLen;
	UCHAR igtk_key_idx;
	UCHAR bigtk[32];
	UCHAR bigtk_key_len;
	UCHAR bigtk_key_idx;
	UCHAR PeerAddr[MAC_ADDR_LEN];
} ASIC_SEC_INFO, *PASIC_SEC_INFO;

#define IS_ADDKEY_OPERATION(_pSecInfo)    ((_pSecInfo->Operation == SEC_ASIC_ADD_PAIRWISE_KEY) || (_pSecInfo->Operation == SEC_ASIC_ADD_GROUP_KEY))
#define IS_REMOVEKEY_OPERATION(_pSecInfo)    ((_pSecInfo->Operation == SEC_ASIC_REMOVE_PAIRWISE_KEY) || (_pSecInfo->Operation == SEC_ASIC_REMOVE_GROUP_KEY))
#define IS_PAIRWISEKEY_OPERATION(_pSecInfo)    ((_pSecInfo->Operation == SEC_ASIC_ADD_PAIRWISE_KEY) || (_pSecInfo->Operation == SEC_ASIC_REMOVE_PAIRWISE_KEY))
#define IS_GROUPKEY_OPERATION(_pSecInfo)    ((_pSecInfo->Operation == SEC_ASIC_ADD_GROUP_KEY) || (_pSecInfo->Operation == SEC_ASIC_REMOVE_GROUP_KEY))


typedef struct _MT_SET_BSSINFO {
	UCHAR OwnMacIdx;
	UINT8 ucBssIndex;
	UINT8 Bssid[MAC_ADDR_LEN];
	UINT8 BcMcWlanIdx;
	UINT32 NetworkType;
	UINT32 u4ConnectionType;
	UINT8 Active;
	UINT32 u4EnableFeature;
} MT_SET_BSSINFO, *PMT_SET_BSSINFO;

#ifdef BCN_OFFLOAD_SUPPORT
typedef struct _MT_SET_BCN_OFFLOAD {
	UINT8 WdevIdx;
	ULONG WholeLength;
	BOOLEAN Enable;
	UCHAR OffloadPktType;
	ULONG TimIePos;
	ULONG CsaIePos;
} MT_SET_BCN_OFFLOAD, *PMT_SET_BCN_OFFLOAD;
#endif

#ifdef OCE_SUPPORT
typedef struct _MT_SET_FD_FRAME_OFFLOAD {
	UINT8 WdevIdx;
	UINT8 ucEnable;
	UINT16 u2PktLength;
	UINT16 u2TimestampFieldPos;

	UINT8 acPktContent[1520];
} MT_SET_FD_FRAME_OFFLOAD, *PMT_SET_FD_FRAME_OFFLOAD;
#endif /* OCE_SUPPORT */

typedef struct _MT_UPDATE_BEACON {
	struct wifi_dev *wdev;
	UCHAR UpdateReason;
} MT_UPDATE_BEACON, *PMT_UPDATE_BEACON;

typedef struct _MT_SET_STA_REC {
	UINT8 BssIndex;
	UINT8 WlanIdx;
	UINT32 ConnectionType;
	UINT8 ConnectionState;
	UINT32 EnableFeature;
} MT_SET_STA_REC, *PMT_SET_STA_REC;

typedef struct _RT_SET_TR_ENTRY {
	ULONG WCID;
	VOID *pEntry;
} RT_SET_TR_ENTRY, *PRT_SET_TR_ENTRY;

typedef struct _MT_VOW_STA_GROUP {
	UINT16 StaIdx;
	UINT8 GroupIdx;
	UINT8 WmmIdx;
} MT_VOW_STA_GROUP, *PMT_VOW_STA_GROUP;

#ifdef VOW_SUPPORT
typedef struct _MT_VOW_STA_QUANTUM {
	BOOLEAN restore;
	UINT8 quantum;
} MT_VOW_STA_QUANTUM, *PMT_VOW_STA_QUANTUM;

#endif /* VOW_SUPPORT */

typedef struct _SLOT_CFG {
	BOOLEAN bUseShortSlotTime;
	UCHAR Channel;
	struct wifi_dev *wdev;
} SLOT_CFG;

typedef struct _REMOVE_REPT_ENTRY_STRUC {
	UCHAR CliIdx;
} REMOVE_REPT_ENTRY_STRUC, *PREMOVE_REPT_ENTRY_STRUC;

typedef struct _ADD_REPT_ENTRY_STRUC {
	struct wifi_dev *wdev;
	UCHAR arAddr[MAC_ADDR_LEN];
} ADD_REPT_ENTRY_STRUC, *PADD_REPT_ENTRY_STRUC;

typedef struct _TX_STAT_STRUC {
	UINT32 Field;	/* Tx Statistic update method from N9 (GET_TX_STAT_XXX) */
	UINT16 Wcid;
	UINT8 Band;
} TX_STAT_STRUC, *PTX_STAT_STRUC;

typedef struct _CR4_QUERY_STRUC {
	UINT16 number; /* Tx Statistic update method from CR4 */
	UINT8 reserve[2];
	UINT16 list[0];
} CR4_QUERY_STRUC, *PCR4_QUERY_STRUC;

#ifdef TXBF_SUPPORT
typedef struct _MT_STA_BF_ADJ {
	struct wifi_dev *wdev;
	UCHAR ConnectionState;
} MT_STA_BF_ADJ, *PMT_STA_BF_ADJ;
#endif /* TXBF_SUPPORT */

typedef struct _MT_SET_LED_STS {
	UCHAR Status;
	UCHAR BandIdx;
} MT_SET_LED_STS, *PMT_SET_LED_STS;
typedef struct _MT_LED_GPIO_MAP {
	UINT8 led_index;
	UINT16 map_index;
	BOOLEAN ctr_type;
} MT_LED_GPIO_MAP, *PMT_LED_GPIO_MAP;

/* For wifi and md coex in colgin project*/
#ifdef WIFI_MD_COEX_SUPPORT
typedef struct _MT_WIFI_COEX_APCCCI2FW {
	UCHAR data[1640]; /* FW cmd length limit 1.6k bytes*/
	UINT16 len;
} MT_WIFI_COEX_APCCCI2FW, *PMT_WIFI_COEX_APCCCI2FW;
#endif /*WIFI_MD_COEX_SUPPORT*/

/*Export API function*/
UINT32 HwCtrlInit(struct _RTMP_ADAPTER *pAd);
VOID HwCtrlExit(struct _RTMP_ADAPTER *pAd);
UINT32 HWCtrlOpsReg(struct _RTMP_ADAPTER *pAd);
UINT32 hwctrl_queue_len(struct _RTMP_ADAPTER *pAd);
BOOLEAN hwctrl_cmd_q_empty(struct _RTMP_ADAPTER *pAd);
VOID Show_HwCmdTable(struct _RTMP_ADAPTER *pAd, BOOLEAN bReset);

#ifdef WF_RESET_SUPPORT
UINT32 wf_reset_init(struct _RTMP_ADAPTER *pAd);
UINT32 wf_reset_exit(struct _RTMP_ADAPTER *pAd);
#endif

#ifdef MTK_FE_RESET_RECOVER
unsigned int mtk_fe_reset_notifier_init(struct _RTMP_ADAPTER *pAd);
void mtk_fe_reset_notifier_exit(struct _RTMP_ADAPTER *pAd);
#endif

NDIS_STATUS HwCtrlEnqueueCmd(
	struct _RTMP_ADAPTER *pAd,
	HW_CTRL_TXD HwCtrlTxd);

INT Show_HwCtrlStatistic_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);


/*Security*/
VOID HW_ADDREMOVE_KEYTABLE(struct _RTMP_ADAPTER *pAd, struct _ASIC_SEC_INFO *pInfo);

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)

#define RTMP_SET_TR_ENTRY(pAd, pEntry)   \
	TRTableInsertEntry(pAd, pEntry->wcid, pEntry);

#define RTMP_MLME_PRE_SANITY_CHECK(_pAd)

#define RTMP_AP_ADJUST_EXP_ACK_TIME(_pAd) \
	RTMP_IO_WRITE32(_pAd->hdev_ctrl,  EXP_ACK_TIME, 0x005400ca)

#define RTMP_AP_RECOVER_EXP_ACK_TIME(_pAd) \
	RTMP_IO_WRITE32(_pAd->hdev_ctrl,  EXP_ACK_TIME, 0x002400ca)

#define RTMP_SET_LED_STATUS(_pAd, _Status, _BandIdx) \
	RTMPSetLEDStatus(_pAd, _Status, _BandIdx)

#define RTMP_SET_LED(_pAd, _Mode, _BandIdx) \
	RTMPSetLED(_pAd, _Mode, _BandIdx)

VOID RTMP_PWR_MGT_BIT_WIFI(struct _RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT8 ucPwrMgtBit);
VOID RTMP_FORCE_WAKEUP(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg);
VOID RTMP_SLEEP_FORCE_AUTO_WAKEUP(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg);

#else
VOID RTMP_MLME_PRE_SANITY_CHECK(struct _RTMP_ADAPTER *pAd);

/*Security*/

VOID RTMP_SET_TR_ENTRY(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
VOID RTMP_AP_ADJUST_EXP_ACK_TIME(struct _RTMP_ADAPTER *pAd);
VOID RTMP_AP_RECOVER_EXP_ACK_TIME(struct _RTMP_ADAPTER *pAd);
VOID RTMP_SET_LED_STATUS(struct _RTMP_ADAPTER *pAd, UCHAR Status, CHAR BandIdx);
VOID RTMP_SET_LED(struct _RTMP_ADAPTER *pAd, UINT32 WPSLedMode10, CHAR BandIdx);

/*STA*/
#ifdef CONFIG_STA_SUPPORT
VOID RTMP_PWR_MGT_BIT_WIFI(struct _RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT8 ucPwrMgtBit);
VOID RTMP_FORCE_WAKEUP(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg);
VOID RTMP_SLEEP_FORCE_AUTO_WAKEUP(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg);
#endif /* CONFIG_STA_SUPPORT */


#endif /*defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)*/
VOID RTMP_LED_GPIO_MAP(struct _RTMP_ADAPTER *pAd, UINT8 led_index, UINT16 map_index, BOOLEAN ctr_type);

/*Common*/
VOID RTMP_UPDATE_RAW_COUNTER(struct _RTMP_ADAPTER *pAd);
VOID RTMP_UPDATE_MIB_COUNTER(struct _RTMP_ADAPTER *pAd);
#ifdef OFFCHANNEL_ZERO_LOSS
VOID RTMP_UPDATE_CHANNEL_STATS(struct _RTMP_ADAPTER *pAd);
#endif
#ifdef ZERO_LOSS_CSA_SUPPORT
VOID HANDLE_NULL_ACK_EVENT(struct _RTMP_ADAPTER *pAd, UINT8 *data);
VOID HANDLE_STA_NULL_ACK_TIMEOUT(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx);
#endif /*ZERO_LOSS_CSA_SUPPORT*/
VOID RTMP_PS_RETRIVE_START(struct _RTMP_ADAPTER *pAd, UINT16 Wcid);
VOID RTMP_PS_RETRIVE_CLEAR(struct _RTMP_ADAPTER *pAd, UINT16 Wcid);
VOID RTMP_HANDLE_PRETBTT_INT_EVENT(struct _RTMP_ADAPTER *pAd);

VOID RTMP_SET_TX_BURST(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN enable);

VOID HW_SET_TX_BURST(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 ac_type,
					 UINT8 prio, UINT16 level, UINT8 enable);

VOID HW_SET_PART_WMM_PARAM(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					 UCHAR wmm_idx, UINT32 AcNum, UINT32 EdcaType, UINT32 EdcaValue);

#ifdef VOW_SUPPORT
VOID HW_SET_VOW_SCHEDULE_CTRL(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN apply_sch_ctrl,
	UINT8 sch_type,
	UINT8 sch_policy);
#endif

#ifdef DABS_QOS
bool HW_UPDATE_QOS_PARAM(struct _RTMP_ADAPTER *pAd, UINT32 idx, BOOLEAN set_del);
#endif

VOID HW_ADD_REPT_ENTRY(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	PUCHAR pAddr);

VOID HW_REMOVE_REPT_ENTRY(
	struct _RTMP_ADAPTER *pAd,
	UCHAR CliIdx);

VOID HW_UPDATE_BSSINFO(struct _RTMP_ADAPTER *pAd, struct _BSS_INFO_ARGUMENT_T *BssInfoArgs);

VOID RTMP_SET_BA_REC(struct _RTMP_ADAPTER *pAd, VOID *Buffer, UINT32 Len);
VOID HW_SET_BA_REC(
	struct _RTMP_ADAPTER *pAd,
	UINT16 wcid,
	UCHAR tid,
	UINT16 sn,
	UINT16 basize,
	BOOLEAN isAdd,
	INT ses_type,
	UCHAR amsdu);
VOID HW_SET_DEL_ASIC_WCID(struct _RTMP_ADAPTER *pAd, ULONG Wcid);

#ifdef HTC_DECRYPT_IOT
VOID HW_SET_ASIC_WCID_AAD_OM(struct _RTMP_ADAPTER *pAd, ULONG Wcid, UCHAR value);
#endif /* HTC_DECRYPT_IOT */

#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
VOID HW_SET_ASIC_WCID_4ADDR_HDR_TRANS(struct _RTMP_ADAPTER *pAd, ULONG Wcid, UCHAR IsEnable);
#endif


#ifdef OCE_SUPPORT
VOID HW_SET_FD_FRAME_OFFLOAD(struct _RTMP_ADAPTER *pAd,
				UINT8 WdevIdx,
				ULONG WholeLength,
				BOOLEAN Enable,
				UINT16 TimestampPos,
				UCHAR *Buf);
#endif /* OCE_SUPPORT */

VOID RTMP_GET_TEMPERATURE(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT32 *pTemperature);
VOID RTMP_RADIO_ON_OFF_CTRL(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 ucRadio);
int HW_GET_TSF(struct wifi_dev *wdev, UINT32 *current_tsf);

#ifdef MBO_SUPPORT
VOID RTMP_BSS_TERMINATION(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#endif /* MBO_SUPPORT */

#ifdef GREENAP_SUPPORT
VOID RTMP_GREENAP_ON_OFF_CTRL(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN ucGreenAP);
#endif /* GREENAP_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
VOID rtmp_pcie_aspm_dym_ctrl(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN fgL1Enable, BOOLEAN fgL0sEnable);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
VOID rtmp_twt_agrt_update(struct _RTMP_ADAPTER *ad, struct twt_agrt_para twt_agrt_para);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
#ifdef PS_STA_FLUSH_SUPPORT
VOID RTMP_SET_PS_FLOW_CTRL(struct _RTMP_ADAPTER *pAd);
#endif
VOID HW_SET_SLOTTIME(struct _RTMP_ADAPTER *pAd, BOOLEAN bUseShortSlotTime, UCHAR Channel, struct wifi_dev *wdev);
VOID HW_ENTER_PS_NULL(struct _RTMP_ADAPTER *pAd, struct _STA_ADMIN_CONFIG *pStaCfg);
VOID HW_BEACON_UPDATE(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR UpdateBeacon);


#ifdef PKT_BUDGET_CTRL_SUPPORT
VOID HW_SET_PBC_CTRL(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UCHAR type);
#endif /*PKT_BUDGET_CTRL_SUPPORT*/

VOID HW_SET_RTS_THLD(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR pkt_num, UINT32 length);

VOID HW_SET_PROTECT(struct _RTMP_ADAPTER *pAd,
		    struct wifi_dev *wdev,
		    enum prot_service_type type,
		    UINT32 cookie1,
		    UINT32 cookie2);

/* Insert the BA bitmap to ASIC for the Wcid entry */
#define RTMP_ADD_BA_SESSION_TO_ASIC(_pAd, _wcid, _TID, _SN, _basize, _type, _amsdu)	\
	HW_SET_BA_REC(_pAd, _wcid, _TID, _SN, _basize, 1, _type, _amsdu);


/* Remove the BA bitmap from ASIC for the Wcid entry */
/*		bitmap field starts at 0x10000 in ASIC WCID table */
#define RTMP_DEL_BA_SESSION_FROM_ASIC(_pAd, _wcid, _TID, _type, _amsdu) \
	HW_SET_BA_REC(_pAd, _wcid, _TID, 0, 0, 0, _type, _amsdu);

#ifdef TXBF_SUPPORT
VOID HW_APCLI_BF_CAP_CONFIG(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
VOID HW_APCLI_BF_REPEATER_CONFIG(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
VOID HW_STA_BF_SOUNDING_ADJUST(struct _RTMP_ADAPTER *pAd, UCHAR connState, struct wifi_dev *wdev);
VOID HW_AP_TXBF_TX_APPLY(struct _RTMP_ADAPTER *pAd, UCHAR enable);
#endif /* TXBF_SUPPORT */

#ifdef WTBL_TDD_SUPPORT
VOID HW_TDD_WCID_SWAP_IN(struct _RTMP_ADAPTER *pAd,  struct _MAC_TABLE_ENTRY *pMacEntry);
VOID HW_TDD_WCID_SWAP_OUT(struct _RTMP_ADAPTER *pAd,  struct _RT_RW_WTBL_ALL *pSwap_param);
VOID HW_RW_WTBL_ALL(struct _RTMP_ADAPTER *ad, struct _RT_RW_WTBL_ALL *pWTBL_ALL);
#endif /* WTBL_TDD_SUPPORT */

VOID RTMP_SET_STA_DWRR(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#ifdef VOW_SUPPORT
VOID RTMP_SET_STA_DWRR_QUANTUM(struct _RTMP_ADAPTER *pAd, BOOLEAN restore, UCHAR quantum);
#endif /* VOW_SUPPORT */
VOID RTMP_SET_UPDATE_PER(struct _RTMP_ADAPTER *pAd, UINT16 Wcid);
VOID RTMP_SET_UPDATE_RSSI(struct _RTMP_ADAPTER *pAd);
VOID RTMP_SET_UPDATE_SNR(struct _RTMP_ADAPTER *pAd);

#ifdef	ETSI_RX_BLOCKER_SUPPORT /* RX Blocker Solution */
VOID RTMP_CHECK_RSSI(struct _RTMP_ADAPTER *pAd);
#endif /* end ETSI_RX_BLOCKER_SUPPORT */

VOID RTMP_SET_THERMAL_RADIO_OFF(struct _RTMP_ADAPTER *pAd);

VOID NICUpdateRawCountersNew(
	struct _RTMP_ADAPTER *pAd);
#ifdef CONFIG_STA_SUPPORT
VOID MlmeDynamicTxRateSwitchingNew(
	struct _RTMP_ADAPTER *pAd);
#endif /* CONFIG_STA_SUPPORT */

#ifdef ERR_RECOVERY
#ifdef MTK_FE_RESET_RECOVER
#define MTK_FE_START_RESET 0x2000
#define MTK_FE_RESET_DONE 0x2001
#define MTK_WIFI_RESET_DONE 0x2002
#define MTK_WIFI_CHIP_ONLINE 0x2003
#define MTK_WIFI_CHIP_OFFLINE 0x2004

struct mtk_notifier_block {
	struct notifier_block nb;
	void *priv;
};
#endif
typedef enum _ERR_RECOVERY_STAGE {
	ERR_RECOV_STAGE_STOP_IDLE = 0,
	ERR_RECOV_STAGE_STOP_PDMA0,
	ERR_RECOV_STAGE_RESET_PDMA0,
	ERR_RECOV_STAGE_STOP_IDLE_DONE,
	ERR_RECOV_STAGE_WAIT_N9_NORMAL,
	ERR_RECOV_STAGE_EVENT_REENTRY,
	ERR_RECOV_STAGE_STATE_NUM
} ERR_RECOVERY_STAGE, *P_ERR_RECOVERY_STAGE;

typedef struct _ERR_RECOVERY_CTRL_T {
	ERR_RECOVERY_STAGE errRecovStage;
	UINT32 status;
#ifdef WHNAT_SUPPORT
	INT stop_rx_dma;
#endif
#ifdef MTK_FE_RESET_RECOVER
	struct completion fe_reset_done;
	atomic_t notify_fe;
	struct mtk_notifier_block mtk_nb;
#endif
} ERR_RECOVERY_CTRL_T, *P_ERR_RECOVERY_CTRL_T;

VOID RTMP_MAC_RECOVERY(struct _RTMP_ADAPTER *pAd, UINT32 Status);
INT IsStopingPdma(struct _ERR_RECOVERY_CTRL_T *pErrRecoveryCtl);
#ifdef WHNAT_SUPPORT
INT IsStopingRxDma(struct _ERR_RECOVERY_CTRL_T *pErrRecoveryCtl);
#endif
ERR_RECOVERY_STAGE ErrRecoveryCurStage(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl);
BOOLEAN IsErrRecoveryInIdleStat(struct _RTMP_ADAPTER *pAd);
VOID ser_sys_reset(RTMP_STRING *arg);
NTSTATUS HwRecoveryFromError(struct _RTMP_ADAPTER *pAd);
void SerTimeLogDump(struct _RTMP_ADAPTER *pAd);
#endif /* ERR_RECOVERY */

#ifdef SCAN_RADAR_COEX_SUPPORT
NTSTATUS UpdateRddReportHandle(struct _RTMP_ADAPTER *pAd);
#endif /* SCAN_RADAR_COEX_SUPPORT */

#ifdef WF_RESET_SUPPORT
NTSTATUS wf_reset_func(struct _RTMP_ADAPTER *pAd);
#endif

UINT32 HW_WIFISYS_OPEN(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);
UINT32 HW_WIFISYS_CLOSE(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);
UINT32 HW_WIFISYS_LINKUP(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);
UINT32 HW_WIFISYS_LINKDOWN(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);
UINT32 HW_WIFISYS_PEER_LINKUP(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);
UINT32 HW_WIFISYS_PEER_LINKDOWN(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);
UINT32 HW_WIFISYS_PEER_UPDATE(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);
VOID HW_WIFISYS_RA_UPDATE(struct _RTMP_ADAPTER *ad, struct WIFI_SYS_CTRL *wsys);

VOID HW_GET_TX_STATISTIC(struct _RTMP_ADAPTER *pAd, TX_STAT_STRUC *P_buf, UCHAR num);

#ifdef WIFI_MD_COEX_SUPPORT
VOID HW_QUERY_LTE_SAFE_CHANNEL(struct _RTMP_ADAPTER *pAd);
#ifdef COEX_DIRECT_PATH
VOID HW_WIFI_COEX_UPDATE_3WIRE_GRP(struct _RTMP_ADAPTER *pAd, VOID *pBuf, UINT32 len);
#endif
#endif


enum {
	HWCTRL_OP_TYPE_V1,
	HWCTRL_OP_TYPE_V2,
};

enum protect_mode_update_method {
	HWCTRL_PROT_UPDATE_METHOD_V1, /*update HW CR by command only*/
	HWCTRL_PROT_UPDATE_METHOD_V2, /*update BSSINFO and HW CR by bssinfo command tag*/
};

void hw_set_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					 UINT8 ac_type, UINT8 prio, UINT16 level, UINT8 enable);

#ifdef WIFI_SYS_FW_V1
VOID hw_ctrl_ops_v1_register(struct _HWCTRL_OP *hwctrl_ops);
#endif /*WIFI_SYS_FW_V1*/

#ifdef WIFI_SYS_FW_V2
VOID hw_ctrl_ops_v2_register(struct _HWCTRL_OP *hwctrl_ops);
#endif /*WIFI_SYS_FW_V2*/

#ifdef LINK_TEST_SUPPORT
VOID RTMP_AUTO_LINK_TEST(struct _RTMP_ADAPTER *pAd);
#endif /* LINK_TEST_SUPPORT */

#ifdef HOST_RESUME_DONE_ACK_SUPPORT
VOID rtmp_host_resume_done_ack(struct _RTMP_ADAPTER *pAd);
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */

#ifdef NF_SUPPORT_V2
VOID HW_NF_UPDATE(struct _RTMP_ADAPTER *pAd, UCHAR flag);
#endif

/* For wifi and md coex in colgin project*/
#ifdef WIFI_MD_COEX_SUPPORT
VOID HW_WIFI_COEX_APCCCI2FW(struct _RTMP_ADAPTER *pAd, VOID *apccci2fw_msg);
#endif /* WIFI_MD_COEX_SUPPORT */

#ifdef CFG_SUPPORT_CSI
VOID HW_CSI_CTRL(struct _RTMP_ADAPTER *pAd, void *prCSICtrl);
#endif

#endif
