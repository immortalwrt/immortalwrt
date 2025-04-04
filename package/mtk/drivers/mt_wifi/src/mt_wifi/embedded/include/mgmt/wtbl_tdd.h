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

#ifndef __WTBL_TDD_H__
#define __WTBL_TDD_H__

struct _RTMP_ADAPTER;
struct _RX_BLK;
struct _MAC_TABLE_ENTRY;
struct _STA_TR_ENTRY;

#define WTBL_TDD_FSM_IDLE                   0
#define WTBL_TDD_FSM_SWAP_DONE              1
#define WTBL_TDD_FSM_MAX_STATE              2

#define WTBL_TDD_FSM_BASE                   0
#define WTBL_TDD_FSM_SWAP_REQ               0
#define WTBL_TDD_FSM_RECYCLE_REQ            1
#define WTBL_TDD_FSM_MAX_MSG                2

#define WTBL_TDD_FSM_FUNC_SIZE              (WTBL_TDD_FSM_MAX_STATE * WTBL_TDD_FSM_MAX_MSG)

typedef struct _WTBL_TDD_MSG {
	QUEUE_ENTRY queueEntry;
	UCHAR addr[MAC_ADDR_LEN];
	UCHAR action;
} WTBL_TDD_MSG, *PWTBL_TDD_MSG;

VOID wtbl_tdd_fsm_init(
	struct _RTMP_ADAPTER *pAd,
	UCHAR wcid, STATE_MACHINE *Sm,
	STATE_MACHINE_FUNC Trans[]);

enum {
	WTBL_TDD_ACTION_RX = 0,
	WTBL_TDD_ACTION_TX
};

enum {
	WTBL_TDD_DBG_DUMP = 0,
	WTBL_TDD_DBG_TIME_LOG,
	WTBL_TDD_DBG_STATE_LOG,
	WTBL_TDD_DBG_MAX_NUM
};

typedef enum _WTBL_TDD_DBG {
	WTBL_TDD_DBG_DUMP_FLAG = (1 << WTBL_TDD_DBG_DUMP),
	WTBL_TDD_DBG_TIME_LOG_FLAG = (1 << WTBL_TDD_DBG_TIME_LOG),
	WTBL_TDD_DBG_STATE_FLAG = (1 << WTBL_TDD_DBG_STATE_LOG),
	WTBL_TDD_DBG_MAX_NUM_FLAG = (1 << WTBL_TDD_DBG_MAX_NUM)
} WTBL_TDD_DBG;



#ifdef DBG
#define MT_WTBL_TDD_LOG(pAd, Level, Fmt)	\
	do {	\
		if ((Level) & (pAd->wtbl_dbg_level)) \
				MTWF_PRINT Fmt; \
	} while (0)

#define MT_WTBL_TDD_TIME_LOG_IN(pAd, In)	\
	do {	\
		if ((WTBL_TDD_DBG_TIME_LOG_FLAG) & (pAd->wtbl_dbg_level)) \
				In = jiffies; \
	} while (0)

#define MT_WTBL_TDD_TIME_LOG_OUT(pAd, Out)	\
	do {	\
		if ((WTBL_TDD_DBG_TIME_LOG_FLAG) & (pAd->wtbl_dbg_level)) \
				Out = jiffies; \
	} while (0)

#define MT_WTBL_TDD_TIME_LOG_ENABLED(pAd) ((pAd->wtbl_dbg_level) & (WTBL_TDD_DBG_TIME_LOG_FLAG))

#define MT_WTBL_TDD_HEX_DUMP(pAd, str, pSrcBufVA, SrcBufLen)	\
	do {	\
		if ((WTBL_TDD_DBG_TIME_LOG_FLAG) & (pAd->wtbl_dbg_level)) {\
			hex_dump_with_lvl(str, pSrcBufVA, SrcBufLen, DBG_LVL_OFF); \
		} \
	} while (0)

#else
#define MT_WTBL_TDD_LOG(pAd, Level, Fmt)
#define MT_WTBL_TDD_TIME_LOG_IN(pAd, In)
#define MT_WTBL_TDD_TIME_LOG_OUT(pAd, Out)
#define MT_WTBL_TDD_TIME_LOG_ENABLED(pAd)
#define MT_WTBL_TDD_HEX_DUMP(pAd, str, pSrcBufVA, SrcBufLen)
#endif

enum WTBL_TDD_STA_STATE {
	WTBL_TDD_STA_IDLE = 0,
	WTBL_TDD_STA_SWAP_REQ_ING,
	WTBL_TDD_STA_SWAP_OUT_ING,
	WTBL_TDD_STA_SWAP_OUT_DONE,
	WTBL_TDD_STA_SWAP_IN_ING,
	WTBL_TDD_STA_SWAP_IN_DONE,
};

typedef struct _WTBL_TDD_CTRL {
	UINT16 LastWtblIdx; /*save the last wtbl idx */
	UINT16 LastExtIdx; /* use for link to Ext Table */
	UCHAR SegIdx; /* use for link to Ext Table */
	UCHAR ExtIdx; /*useless sofar */
	ULONG LastActiveTime;
	ULONG ConnectTime;
	UINT ConnectID;
	UCHAR state;
	NDIS_SPIN_LOCK enqMlmeCountLock;
	UINT enqMlmeCount;
	STATE_MACHINE	   WtblTddFsm;
	STATE_MACHINE_FUNC WtblTddFun[WTBL_TDD_FSM_FUNC_SIZE];
	UCHAR guardTime;
} WTBL_TDD_CTRL, *PWTBL_TDD_CTRL;

enum _WTBL_TDD_SW_MAC_TAB_SEG {
	WTBL_TDD_SW_MAC_TAB_SEG_P0 = 0,
	WTBL_TDD_SW_MAC_TAB_SEG_P1 = 1,
	WTBL_TDD_SW_MAC_TAB_SEG_NUM,
};

typedef enum _TABLE_MODE{
	SW_TABLE = 0,
	HW_TABLE
} TABLE_MODE;

typedef struct _NODE_IFNO {
	UINT index;
	UCHAR Addr[MAC_ADDR_LEN];
	TABLE_MODE tableMode;
	BOOLEAN isValid;
	UCHAR swLastIndex;
	UCHAR swLastSegIndex;
	UCHAR hwWtblIndex;
	VOID *entryPoint;
	/* counter info. */
	ULONG pktCnt;
	ULONG swapInCnt;
	ULONG swapOutCnt;
	struct _NODE_IFNO *pNext;
} NODE_INFO, *PNODE_INFO;

#define MAX_LEN_OF_NODE_TABLE 512
typedef struct _WTBL_TDD_IFNO {
	BOOLEAN enabled;
	UINT nodeInfoTabSize;
	NODE_INFO nodeInfo[MAX_LEN_OF_NODE_TABLE];
	NDIS_SPIN_LOCK nodeInfoLock;
	NDIS_SPIN_LOCK NodeHashLock;
	NODE_INFO * NodeHash[HASH_TABLE_SIZE];
	UINT wcidScore[MAX_LEN_OF_MAC_TABLE];
	BOOLEAN wcidScoreUsed[MAX_LEN_OF_MAC_TABLE];
	NDIS_SPIN_LOCK updateScoreLock;
	RTMP_OS_COMPLETION acquireComplete;
	RTMP_OS_COMPLETION swapOutComplete;
	QUEUE_HEADER txPendingList;
	NDIS_SPIN_LOCK txPendingListLock;
	QUEUE_HEADER swapReqEventQue;
	NDIS_SPIN_LOCK swapReqEventQueLock;
	UINT txMissCounter;
	UINT rxMissCounter;
	UINT MlmeQFullCounter;
	UINT MlmeQInCounter;
} WTBL_TDD_INFO, *PWTBL_TDD_INFO;

#define IS_WTBL_TDD_ENABLED(_AD)		((_AD)->wtblTddInfo.enabled == TRUE)
#define SET_WTBL_TDD_DISABLED(_AD)		((_AD)->wtblTddInfo.enabled = FALSE)
#define SET_WTBL_TDD_ENABLED(_AD)		((_AD)->wtblTddInfo.enabled = TRUE)

BOOLEAN WtblTdd_ActiveList_SwapOut(
	struct _RTMP_ADAPTER *pAd,
	UCHAR SegIdx,
	UINT16 inActWcid,
	UINT16 actWcid);

UINT16 WtblTdd_ActiveList_SwapIn(
	struct _RTMP_ADAPTER *pAd,
	struct _MAC_TABLE_ENTRY *pInActEntry,
	struct _STA_TR_ENTRY *pInActTrEntry);

UINT16 WtblTdd_ActiveList_SelectBad(
	struct _RTMP_ADAPTER *pAd);

VOID WtblTdd_InactiveList_Predict(
	struct _RTMP_ADAPTER *pAd,
	ULONG targetTime);

UINT16 WtblTdd_AcquireUcastWcid(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR *useExt,
	UCHAR *SegIdx);

struct _MAC_TABLE_ENTRY *WtblTdd_InactiveList_Lookup(
	struct _RTMP_ADAPTER *pAd,
	UCHAR *pAddr);

NODE_INFO *WtblTdd_InfoNode_Lookup(
	struct _RTMP_ADAPTER *pAd,
	UCHAR *pAddr);

BOOLEAN WtblTdd_InactiveList_EntryDel(
	struct _RTMP_ADAPTER *pAd,
	struct _MAC_TABLE_ENTRY *pEntry);

BOOLEAN WtblTdd_RxPacket_BlockList(
	struct _RTMP_ADAPTER *pAd,
	struct _RX_BLK *rx_blk);

UCHAR WtblTdd_Entry_RxPacket(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *pInActEntry);

UCHAR WtblTdd_Entry_TxPacket(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *pInActEntry);

UCHAR WtblTdd_Entry_Init(
	struct _RTMP_ADAPTER *pAd,
	struct _MAC_TABLE_ENTRY *pEntry,
	UCHAR SegIdx);

INT WtblTdd_DumpTab(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

INT WtblTdd_TestAction(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);

VOID WtblTdd_Init(
	struct _RTMP_ADAPTER *pAd);

VOID WtblTdd_DeInit(
	struct _RTMP_ADAPTER *pAd);

TABLE_MODE WtblTdd_Entry_DeInit(
	struct _RTMP_ADAPTER *pAd,
	USHORT wcid,
	UCHAR *pAddr);

VOID WtblTdd_ActiveList_MlmeCal(
	struct _RTMP_ADAPTER *pAd);

VOID WtblTdd_ActiveList_ScoreDel(
	struct _RTMP_ADAPTER *pAd,
	UCHAR wcid);

BOOLEAN WtblTdd_InAct_MacTableDeleteEntry(
	struct _RTMP_ADAPTER *pAd,
	USHORT SegIdx,
	USHORT inAct_wcid,
	UCHAR *pAddr);

BOOLEAN WtblTdd_InAct_MacTableInsertEntry(
	struct _RTMP_ADAPTER *pAd,
	UCHAR SegIdx,
	UINT16 inActWcid,
	UINT16 actWcid);


#endif /* __WTBL_TDD_H__ */

