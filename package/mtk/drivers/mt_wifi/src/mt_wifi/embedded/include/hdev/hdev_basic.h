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

#ifndef __HDEV_BASIC_H
#define __HDEV_BASIC_H

#include "common/link_list.h"

struct _RTMP_CHIP_CAP;
struct _RTMP_CHIP_OP;
struct _EDCA_PARM;
struct MCU_CTRL;
struct _BCTRL_INFO_T;
struct _BCTRL_ENTRY;
struct freq_oper;
struct wmm_entry;
struct pe_control;

struct radio_res {
	UCHAR reason;
	struct freq_oper *oper;
};

enum {
	REASON_NORMAL_SW,
	REASON_NORMAL_SCAN,
	REASON_ATE,
};

/*
* state machine:
* case1: NONE_OCCUPIED ->SW_OCCUPIED->NONE_OCCUPIED
*/

enum {
	WTBL_STATE_NONE_OCCUPIED = 0,
	WTBL_STATE_SW_OCCUPIED,
};

enum {
	WTBL_TYPE_NONE = 0,
	WTBL_TYPE_UCAST,
	WTBL_TYPE_MCAST,
	WTBL_TYPE_RESERVED, /* for no match wcid reseved */
};

#ifdef SW_CONNECT_SUPPORT
typedef struct _HW2SW_ENTRY {
	BOOLEAN bDummy;
	UINT16 	wcid_sw;
	UCHAR	State;
	UCHAR	type;
	atomic_t  ref_cnt;
} HW2SW_ENTRY, *PHW2SW_ENTRY;
#endif /* SW_CONNECT_SUPPORT */

typedef struct _WTBL_IDX_PARAMETER {
	UCHAR   State;
	UCHAR   LinkToOmacIdx;
	UCHAR   LinkToWdevType;
	UINT16  WtblIdx;
	UCHAR   type;
#ifdef SW_CONNECT_SUPPORT
	BOOLEAN bSw;
#endif /* SW_CONNECT_SUPPORT */
	DL_LIST list;
} WTBL_IDX_PARAMETER, *PWTBL_IDX_PARAMETER;

typedef struct _WTBL_CFG {
	UINT16 MaxUcastEntryNum;
	UINT16 MinMcastWcid;
#ifdef SW_CONNECT_SUPPORT
	UINT16 MaxUcastEntryNumSw;
	UINT16 MinMcastWcidSw;
	DUMMY_WCID_OBJ dummy_wcid_obj[DBDC_BAND_NUM];
	HW2SW_ENTRY *Hw2SwTbl;
#endif /* SW_CONNECT_SUPPORT */
	BOOLEAN mcast_wait;
	WTBL_IDX_PARAMETER WtblIdxRec[MAX_LEN_OF_MAC_TABLE];
	NDIS_SPIN_LOCK WtblIdxRecLock;
} WTBL_CFG;

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
struct twt_ctrl {
	BOOLEAN initd;
	UINT8 max_twt_node_num;				/* twt node num (i + b + g) */
	UINT8 free_twt_node_num_individual;	/* twt node num (individual) */
	UINT8 free_twt_node_num_btwt;		/* twt node num (btwt) */
	UINT8 free_twt_node_num_group;		/* twt node num (group) */
#ifdef DYNAMIC_TWT_NODE_SUPPORT
	struct twt_link_node **twt_node;	/* twt agrt num */
#else
	struct twt_link_node twt_node[TWT_HW_AGRT_MAX_NUM]; /* twt agrt num */
#endif /* DYNAMIC_TWT_NODE_SUPPORT */
	struct _DL_LIST twt_link[SCH_LINK_NUM]; /* twt sch-link/usch-link */
	NDIS_SPIN_LOCK twt_rec_lock;			/* twt agrt resource lock */
};
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

#define WTC_WAIT_TIMEOUT CMD_MSG_TIMEOUT

typedef struct _OMAC_BSS_CTRL {
	UINT32 OmacBitMap;
	UINT32 HwMbssBitMap;
	UINT32 RepeaterBitMap;
} OMAC_BSS_CTRL, *POMAC_BSS_CTRL;

struct wmm_ctrl {
	UCHAR num;
	struct wmm_entry *entries;
};

#ifdef DOT11_HE_AX
struct bss_color_table {
	ULONG last_detected_time[BSS_COLOR_VALUE_MAX];
	NDIS_SPIN_LOCK bss_color_lock;
};
#endif


typedef struct radio_control {
	UCHAR BandIdx;
	UCHAR CurStat;
	USHORT PhyMode;
	UCHAR cur_rfic_type;
	UCHAR Channel;
	UCHAR Channel2;
	UCHAR CentralCh;
	UCHAR Bw;
	UCHAR ExtCha;
	/*check first radio update is for scan or not*/
	BOOLEAN scan_state;
	BOOLEAN IsBfBand;
	BOOLEAN BfSmthIntlBypass;
#ifdef GREENAP_SUPPORT
	BOOLEAN bGreenAPActive;
#endif /* GREENAP_SUPPORT */
	UCHAR rx_stream;
#ifdef TXRX_STAT_SUPPORT
	LARGE_INTEGER TxDataPacketCount;
	LARGE_INTEGER TxDataPacketByte;
	LARGE_INTEGER RxDataPacketCount;
	LARGE_INTEGER RxDataPacketByte;
	LARGE_INTEGER TxUnicastDataPacket;
	LARGE_INTEGER TxMulticastDataPacket;
	LARGE_INTEGER TxBroadcastDataPacket;
	LARGE_INTEGER TxMgmtPacketCount;
	LARGE_INTEGER RxMgmtPacketCount;
	LARGE_INTEGER TxBeaconPacketCount;
	LARGE_INTEGER TxDataPacketCountPerAC[4];	/*per access category*/
	LARGE_INTEGER RxDataPacketCountPerAC[4];	/*per access category*/
	CHAR LastDataPktRssi[4];
	LARGE_INTEGER TxPacketDroppedCount;
	LARGE_INTEGER RxDecryptionErrorCount;
	LARGE_INTEGER RxCRCErrorCount;
	LARGE_INTEGER RxMICErrorCount;
	LARGE_INTEGER LastSecTxByte;
	LARGE_INTEGER LastSecRxByte;
	UINT32 TotalPER;
	UINT32 TotalTxFailCnt;
	UINT32 TotalTxCnt;
	UINT32 Last1SecPER;
	UINT32 Last1TxFailCnt;
	UINT32 Last1TxCnt;
#endif
#ifdef TR181_SUPPORT
	UINT32 CurChannelUpTime;		/*usecs since system up*/
	UINT32 RefreshACSChannelChangeCount;
	UINT32 ForceACSChannelChangeCount;
	UINT32 ManualChannelChangeCount;
	UINT32 DFSTriggeredChannelChangeCount;
	UINT32 TotalChannelChangeCount;
	UINT8  ACSTriggerFlag;
#endif /*TR181_SUPPORT*/
} RADIO_CTRL;

typedef struct rtmp_phy_ctrl {
	UINT8 rf_band_cap;
#ifdef CONFIG_AP_SUPPORT
	AUTO_CH_CTRL AutoChCtrl;
#ifdef AP_QLOAD_SUPPORT
	QLOAD_CTRL QloadCtrl;
#endif /*AP_QLOAD_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */
	RADIO_CTRL RadioCtrl;
#ifdef DOT11_HE_AX
	struct pe_control pe_ctrl;
#endif
} RTMP_PHY_CTRL;

typedef struct _HD_RESOURCE_CFG {
	struct rtmp_phy_ctrl PhyCtrl[DBDC_BAND_NUM];
	struct wmm_ctrl wmm_ctrl;
	struct _OMAC_BSS_CTRL OmacBssCtl[DBDC_BAND_NUM];
	/* struct _REPEATER_CFG	RepeaterCfg; */
	struct _WTBL_CFG WtblCfg;
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	struct twt_ctrl twt_ctl;
#endif /* WIFI_TWT_SUPPORT */
	struct bss_color_table color_tbl[DBDC_BAND_NUM];
#endif
	UCHAR concurrent_bands;
	UCHAR txcmd_mode;
} HD_RESOURCE_CFG;

struct radio_dev {
	UCHAR Idx;
	RADIO_CTRL *pRadioCtrl;
	struct _OMAC_BSS_CTRL *omac_ctrl;
	DL_LIST DevObjList;
	UCHAR DevNum;
	/*implicit point to hdev_ctrl for sharing resource*/
	VOID     *priv;
};

union hif_cfg {
#ifdef RTMP_MAC_PCI
	struct _PCI_HIF_T pci;
#endif /*RTMP_MAC_PCI*/
};

struct hif_ctrl {
	union hif_cfg cfg;
	struct hif_ops ops;
};


enum {
	HOBJ_STATE_NONE = 0,
	HOBJ_STATE_USED,
};

enum {
	HOBJ_TX_MODE_TXD,
	HOBJ_TX_MODE_TXCMD,
};

struct hdev_obj {
	UCHAR Idx;
	USHORT Type;
	UCHAR OmacIdx;
	UCHAR WmmIdx;
	BOOLEAN bWmmAcquired;
	struct radio_dev *rdev;
	DL_LIST RepeaterList;
	DL_LIST list;
	UCHAR state;
	UCHAR RefCnt;
	UCHAR tx_mode;
	NDIS_SPIN_LOCK RefCntLock;
	VOID *h_ctrl;
};

struct hdev_ctrl {
	struct radio_dev rdev[DBDC_BAND_NUM];
	CHANNEL_CTRL ChCtrl[DBDC_BAND_NUM];
	/* PSE_CFG				PseCfg; */
	struct hif_ctrl hif;
	struct _RTMP_CHIP_CAP chip_cap;
	struct _RTMP_CHIP_OP chip_ops;
	struct _RTMP_ARCH_OP arch_ops;
	struct _RTMP_CHIP_DBG chip_dbg;
	struct mt_io_ops io_ops;
	struct _HD_RESOURCE_CFG HwResourceCfg;
	struct hdev_obj HObjList[WDEV_NUM_MAX];
	VOID *mcu_ctrl;
	VOID *cookie;
	VOID *priv; /*implicit point to upper struct*/
#ifdef OFFCHANNEL_ZERO_LOSS
	BOOLEAN SuspendMsduTx[DBDC_BAND_NUM];
#endif

};

typedef struct _HD_REPT_ENRTY {
	UCHAR CliIdx;
	UCHAR ReptOmacIdx;
	DL_LIST list;
} HD_REPT_ENRTY;

/*for hdev base functions*/
VOID HdevObjAdd(struct radio_dev *rdev, struct hdev_obj *obj);
VOID HdevObjDel(struct radio_dev *rdev, struct hdev_obj *obj);
BOOLEAN hdev_obj_state_ready(struct hdev_obj *obj);

INT32 HdevInit(struct hdev_ctrl *hdev_ctrl, UCHAR HdevIdx, RADIO_CTRL *pRadioCtrl);
INT32 HdevExit(struct hdev_ctrl *hdev_ctrl, UCHAR HdevIdx);
VOID HdevCfgShow(struct hdev_ctrl *hdev_ctrl);
VOID HdevObjShow(struct hdev_obj *obj);
VOID HdevHwResourceExit(struct hdev_ctrl *ctrl);
VOID HdevHwResourceInit(struct hdev_ctrl *ctrl);

#endif /*__HDEV_BASIC_H*/
