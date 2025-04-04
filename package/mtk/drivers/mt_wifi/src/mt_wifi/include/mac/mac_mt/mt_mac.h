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
/***************************************************************************
 ***************************************************************************

*/

#ifndef __MT_MAC_H__
#define __MT_MAC_H__

#include "mac/mac_mt/top.h"

#ifndef COMPOS_WIN
#endif /* COMPOS_WIN */


#ifdef MT_DMAC
#include "mac/mac_mt/dmac/mt_dmac.h"
#endif

#define SHORT_PREAMBLE 0
#define LONG_PREAMBLE 1

#define TMI_TX_RATE_BIT_MCS		0
#define TMI_TX_RATE_MASK_MCS		0x3f
#define TMI_TX_RATE_CCK_1M_LP		0
#define TMI_TX_RATE_CCK_2M_LP		1
#define TMI_TX_RATE_CCK_5M_LP		2
#define TMI_TX_RATE_CCK_11M_LP	3
#define TMI_TX_RATE_CCK_2M_SP		5
#define TMI_TX_RATE_CCK_5M_SP		6
#define TMI_TX_RATE_CCK_11M_SP	7
#define TMI_TX_RATE_OFDM_6M		11
#define TMI_TX_RATE_OFDM_9M		15
#define TMI_TX_RATE_OFDM_12M		10
#define TMI_TX_RATE_OFDM_18M		14
#define TMI_TX_RATE_OFDM_24M		9
#define TMI_TX_RATE_OFDM_36M		13
#define TMI_TX_RATE_OFDM_48M		8
#define TMI_TX_RATE_OFDM_54M		12
#define TMI_TX_RATE_HT_MCS0		0
#define TMI_TX_RATE_HT_MCS1		1
#define TMI_TX_RATE_HT_MCS2		2
#define TMI_TX_RATE_HT_MCS3		3
#define TMI_TX_RATE_HT_MCS4		4
#define TMI_TX_RATE_HT_MCS5		5
#define TMI_TX_RATE_HT_MCS6		6
#define TMI_TX_RATE_HT_MCS7		7
#define TMI_TX_RATE_HT_MCS8		8
#define TMI_TX_RATE_HT_MCS9		9
#define TMI_TX_RATE_HT_MCS10		10
#define TMI_TX_RATE_HT_MCS11		11
#define TMI_TX_RATE_HT_MCS12		12
#define TMI_TX_RATE_HT_MCS13		13
#define TMI_TX_RATE_HT_MCS14		14
#define TMI_TX_RATE_HT_MCS15		15
#define TMI_TX_RATE_HT_MCS16		16
#define TMI_TX_RATE_HT_MCS17		17
#define TMI_TX_RATE_HT_MCS18		18
#define TMI_TX_RATE_HT_MCS19		19
#define TMI_TX_RATE_HT_MCS20		20
#define TMI_TX_RATE_HT_MCS21		21
#define TMI_TX_RATE_HT_MCS22		22
#define TMI_TX_RATE_HT_MCS23		23
#define TMI_TX_RATE_HT_MCS32		32
#define TMI_TX_RATE_VHT_MCS0		0
#define TMI_TX_RATE_VHT_MCS1		1
#define TMI_TX_RATE_VHT_MCS2		2
#define TMI_TX_RATE_VHT_MCS3		3
#define TMI_TX_RATE_VHT_MCS4		4
#define TMI_TX_RATE_VHT_MCS5		5
#define TMI_TX_RATE_VHT_MCS6		6
#define TMI_TX_RATE_VHT_MCS7		7
#define TMI_TX_RATE_VHT_MCS8		8
#define TMI_TX_RATE_VHT_MCS9		9

#define NUM_OF_MSDU_ID_IN_TXD   4
#define TXD_MAX_BUF_NUM         4

typedef struct _TXD_PTR_LEN_T {
	UINT32 u4Ptr0;
	UINT16 u2Len0; /* Bit15: AL, Bit14: ML */
	UINT16 u2Len1; /* Bit15: AL, Bit14: ML */
	UINT32 u4Ptr1;
} TXD_PTR_LEN_T, *P_TXD_PTR_LEN_T;

typedef struct _MAC_TX_PKT_T {
	UINT16 au2MsduId[NUM_OF_MSDU_ID_IN_TXD];   /* Bit15 indicate valid */
	TXD_PTR_LEN_T arPtrLen[TXD_MAX_BUF_NUM / 2];
} MAC_TX_PKT_T, *P_MAC_TX_PKT_T;

#define TXD_MSDU_ID_VLD             BIT(15)     /* MSDU valid */
#define TXD_LEN_ML_V2               BIT(15)     /* MSDU last */
#define TXD_LEN_MASK_V2             BITS(0, 11)
#define TXD_LEN_AL                  BIT(15)     /* A-MSDU last */
#define TXD_LEN_ML                  BIT(14)     /* MSDU last */
#define RMAC_RX_PKT_TYPE_RX_TXS			0x00
#define RMAC_RX_PKT_TYPE_RX_TXRXV		0x01
#define RMAC_RX_PKT_TYPE_RX_NORMAL		0x02
#define RMAC_RX_PKT_TYPE_RX_DUP_RFB		0x03
#define RMAC_RX_PKT_TYPE_RX_TMR			0x04
#define RMAC_RX_PKT_TYPE_RETRIEVE		0x05
#ifdef CUT_THROUGH
#define RMAC_RX_PKT_TYPE_TXRX_NOTIFY    0x06
#define RMAC_RX_PKT_TYPE_TXRX_NOTIFY_V0    0x18
#endif /* CUT_THROUGH */
#define RMAC_RX_PKT_TYPE_RX_EVENT		0x07
#define RMAC_RX_PKT_TYPE_RX_ICS			0x0c

#ifdef DBDC_MODE
#define DBDC_BAND_NUM		2
#else
#define DBDC_BAND_NUM		1
#endif /* DBDC_MODE */

#define DFS_BAND_NONE   DBDC_BAND_NUM

#define DBDC_BAND0		0
#define DBDC_BAND1		1

#define MAX_PATH_TX     0
#define MAX_PATH_RX     1

#define TX_FREE_NOTIFY 0
#define RX_REORDER_NOTIFY 1
#define TXRX_NOTE_GET_TOKEN_LIST(_ptr) ((UINT8 *)(((UINT8 *)(_ptr)) + 8))

/* hw mode */
enum hw_phymode_type {
	HW_CCK_MODE,
	HW_OFDM_MODE,
	HW_HTMIX_MODE,
	HE_HTGF_MODE,
	HW_VHT_MODE,
	HW_HE_SU_MODE = 8,
	HW_HE_EXT_SU_MODE = 9,
	HW_HE_TRIG_MODE = 10,
	HE_HE_MU_MODE = 11
};

struct _RTMP_ADAPTER;

typedef struct _TX_RADIO_SET {
	BOOLEAN ItxBFEnable;/* IBF */
	BOOLEAN EtxBFEnable;/* EBF */
	BOOLEAN  ShortGI;
	BOOLEAN  Ldpc;
	BOOLEAN  Stbc;
	UINT8	   CurrentPerPktBW;/* BW_20, BW_40 for Fixed Rate */
	UINT8	    Premable;
	UINT8	    RateCode;
	UINT8	    PhyMode;
} TX_RADIO_SET_T;

typedef struct _TMAC_INFO {
	UINT16 PktLen;
	UINT8 WifiHdrLen; /*This80211HdrLen, wifi_hdr_len*/
	UINT8 QueIdx;
	UINT8 PortIdx;
	UINT8 WmmSet;
	BOOLEAN UsbNextValid; /*Check with Lens*/
	UINT16 Wcid;
	BOOLEAN bAckRequired;
	UINT8 UserPriority;
	UINT8 OwnMacIdx;
	UINT32 CipherAlg; /*Q: bProtectFrame*/
	UINT8 HdrPad; /*W:LengthQosPAD*/
	UINT8 FragIdx; /*W:FragmentByteForTxD*/
	UINT8 BarSsnCtrl;
	UINT8 Pid; /*W: PacketID*/
	UINT8 AntPri;
	UINT8 SpeEn;
	CHAR  PowerOffset;
	BOOLEAN	TimingMeasure;
	TX_RADIO_SET_T TxRadioSet;
	BOOLEAN LongFmt;
	BOOLEAN NeedTrans;
	BOOLEAN MoreData;
	BOOLEAN Eosp;
	BOOLEAN EtherFrame;
	BOOLEAN VlanFrame;
	BOOLEAN BmcPkt;
	BOOLEAN FixRate;
	BOOLEAN BaDisable;
	BOOLEAN TxS2Host;
	BOOLEAN TxS2Mcu;
	UINT8 TxSFmt;
	UINT8 MaxTxTime;
	UINT8 RemainTxCnt;
	UINT8 MpduHdrLen;
	UINT16 FrmType;
	UINT16 SubType;
	UINT16 Sn;
	UINT8 VhtNss;
	UCHAR band_idx;
} TMAC_INFO;

enum {
	TMI_TX_RATE_MODE_CCK,
	TMI_TX_RATE_MODE_OFDM,
	TMI_TX_RATE_MODE_HTMIX,
	TMI_TX_RATE_MODE_HTGF,
	TMI_TX_RATE_MODE_VHT,
	TMI_TX_RATE_MODE_HE_SU = 0x8,
	TMI_TX_RATE_MODE_HE_EXT_SU,
	TMI_TX_RATE_MODE_HE_TRIG,
	TMI_TX_RATE_MODE_HE_MU,
};

struct _TX_BLK;
struct _MAC_TX_INFO;
union _HTTRANSMIT_SETTING;

char *rxd_pkt_type_str(INT pkt_type);
INT mt_sf_hw_tx(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk);
INT mt_ct_check_hw_resource(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR resource_idx);
#ifdef CUT_THROUGH
INT mt_ct_get_hw_resource_free_num(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					UCHAR resource_idx, UINT32 *free_num, UINT32 *free_token);
#endif
INT32 mt_ct_get_hw_resource_state(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
									UINT32 pkt_type, UCHAR q_idx);
INT mt_ct_hw_tx(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk);
INT mt_ct_mlme_hw_tx(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info, struct _MAC_TX_INFO *info, union _HTTRANSMIT_SETTING *transmit, struct _TX_BLK *tx_blk);
INT32 mt_ct_ate_hw_tx(struct _RTMP_ADAPTER *pAd, struct _TMAC_INFO *info, struct _TX_BLK *tx_blk);
INT mt_sf_check_hw_resource(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR resource_idx);
INT mt_sf_mlme_hw_tx(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info, struct _MAC_TX_INFO *info, union _HTTRANSMIT_SETTING *transmit, struct _TX_BLK *tx_blk);
VOID mt_dump_rxinfo(struct _RTMP_ADAPTER *pAd, UCHAR *rxinfo);
VOID mt_dump_txs(struct _RTMP_ADAPTER *pAd, UINT8 Format, CHAR *Data);
VOID mt_dump_rmac_info_for_ICVERR(struct _RTMP_ADAPTER *pAd, UCHAR *rmac_info);

#endif /* __MT_MAC_H__ */

