
#ifndef _MT_TESTMODE_H
#define _MT_TESTMODE_H

#if defined(MT7915) || defined(AXE) || defined(MT7986) || defined(MT7916) || defined(MT7981)
#include "mt_testmode_fmac.h"
#endif

#include "mt_testmode_dmac.h"


#define TESTMODE_WCID_BAND0 2
#define TESTMODE_WCID_BAND1 3

#define ATE_SINGLE_BAND	1
#define ATE_DUAL_BAND	2
#define ATE_GBAND_TYPE	1
#define ATE_ABAND_TYPE	2

#define ATE_BAND_WIDTH_20		0
#define ATE_BAND_WIDTH_40		1
#define ATE_BAND_WIDTH_80		2
#define ATE_BAND_WIDTH_10		3
#define ATE_BAND_WIDTH_5		4
#define ATE_BAND_WIDTH_160		5
#define ATE_BAND_WIDTH_8080		6

enum {
	ATE_USER_PAYLOAD,
	ATE_FIXED_PAYLOAD,
	ATE_RANDOM_PAYLOAD,
};

struct _ATE_HE_PHY_RU_CONST {
	UINT8	max_index;
	USHORT	sd;			/* pilot_subcarriers */
	USHORT	sd_d;		/* total_subcarriers */
	USHORT	sd_s;		/* pilot_subcarriers */
	USHORT	sd_s_d;		/* pilot_subcarriers */
};

struct _ATE_DATA_RATE_MAP {
	UCHAR   mcs;            /* MCS index */
	UINT32  tx_data_rate;   /* Data rate in K Bit */
};

struct _ATE_ANT_MAP {
	UINT32 ant_sel;
	UINT32 spe_idx;
};

struct _ATE_TXPWR_GROUP_MAP {
	UINT32 start;
	UINT32 end;
	UINT32 group[4];
};

struct _ATE_CH_KHZ_MAP {
	UINT32 Channel;
	UINT32 Freq;
};

#if defined(COMPOS_TESTMODE_WIN)/* for MT_testmode.c */

#define INC_RING_INDEX(_idx, _RingSize)	\
	do {\
		(_idx)++;						\
		if ((_idx) >= (_RingSize))		\
			_idx = 0;					\
	} while (0)

#define BAND_WIDTH_20		0
#define BAND_WIDTH_40		1
#define BAND_WIDTH_80		2
#define BAND_WIDTH_160		3
#define BAND_WIDTH_10		4	/* 802.11j has 10MHz. This definition is for internal usage. doesn't fill in the IE or other field. */
#define BAND_WIDTH_BOTH		5	/* BW20 + BW40 */
#define BAND_WIDTH_5		6
#define BAND_WIDTH_8080		7

#define TX1_G_BAND_TARGET_PWR 0x5E
#define TX0_G_BAND_TARGET_PWR 0x58
enum {
	DMA_TX,
	DMA_RX,
	DMA_TX_RX,
};

#define TESTMODE_GET_PADDR(_pstruct, _band, _member) (&_pstruct->_member)
#define TESTMODE_GET_PARAM(_pstruct, _band, _member) (_pstruct->_member)
#define TESTMODE_SET_PARAM(_pstruct, _band, _member, _val) (_pstruct->_member = _val)
#ifndef COMPOS_TESTMODE_WIN/* NDIS only */
#define MAC_ADDR_LEN    6
/* 2-byte Frame control field */
struct _QAFRAME_CONTROL {
	UINT16		Ver : 2;
	UINT16		Type : 2;
	UINT16		SubType : 4;
	UINT16		ToDs : 1;
	UINT16		FrDs : 1;
	UINT16		MoreFrag : 1;
	UINT16		Retry : 1;
	UINT16		PwrMgmt : 1;
	UINT16		MoreData : 1;
	UINT16		Wep : 1;
	UINT16		Order : 1;
};

struct _QAHEADER_802_11 {
	struct _QAFRAME_CONTROL   FC;
	UINT16          Duration;
	UCHAR           Addr1[MAC_ADDR_LEN];
	UCHAR           Addr2[MAC_ADDR_LEN];
	UCHAR			Addr3[MAC_ADDR_LEN];
	UINT16			Frag: 4;
	UINT16			Sequence: 12;
};
#endif /* NOT COMPOS_TESTMODE_WIN */
#else
#if defined(CONFIG_WLAN_SERVICE)
#define ATE_TX_CNT tx_stat.tx_cnt
#define ATE_TXDONE_CNT tx_stat.tx_done_cnt
#define ATE_TXED_CNT tx_stat.txed_cnt

#define TESTMODE_GET_BAND_IDX(_ad) SERV_GET_PARAM(net_ad_wrap_service(_ad), ctrl_band_idx)

#define TESTMODE_GET_PARAM(_ad, _band, _member) CONFIG_GET_PARAM(net_ad_wrap_service(_ad), _member, _band)
#define TESTMODE_GET_PADDR(_ad, _band, _member) CONFIG_GET_PADDR(net_ad_wrap_service(_ad), _member, _band)
#define TESTMODE_SET_PARAM(_ad, _band, _member, _val) CONFIG_SET_PARAM(net_ad_wrap_service(_ad), _member, _val, _band)
#else
#define ATE_TX_CNT tx_cnt
#define ATE_TXDONE_CNT tx_done_cnt
#define ATE_TXED_CNT txed_cnt

#define TESTMODE_GET_BAND_IDX(_ad) _ad->ATECtrl.control_band_idx

#define TESTMODE_GET_PARAM(_ad, _band, _member) ((_band)?_ad->ATECtrl.band_ext[_band-1]._member:_ad->ATECtrl._member)
#define TESTMODE_GET_PADDR(_ad, _band, _member) ((_band) ?  &_ad->ATECtrl.band_ext[_band-1]._member :  &_ad->ATECtrl._member)
#define TESTMODE_SET_PARAM(_ad, _band, _member, _val) ({	\
		UINT32 _ret = _val;													\
		if (_band) {														\
			struct _BAND_INFO *_info = &(_ad->ATECtrl.band_ext[_band-1]);			\
			_info->_member = _val;										\
		} else															\
			_ad->ATECtrl._member = _val;									\
		_ret;															\
	})
#endif /* CONFIG_WLAN_SERVICE */
#endif /* defined(COMPOS_TESTMODE_WIN) */

enum _TESTMODE_STAT_TYPE {
	TESTMODE_RXV,
	TESTMODE_PER_PKT,
	TESTMODE_RESET_CNT,
	TESTMODE_COUNTER_802_11,
	TESTMODE_STAT_TYPE_NUM,
};

struct _RATE_TO_BE_FIX {
	UINT32	TXRate: 6;
	UINT32	TXMode: 3;
	UINT32	Nsts: 2;
	UINT32	STBC: 1;
	UINT32	Reserved: 20;
};

struct rssi_offset_eeprom {
	UINT32 **rssi_eeprom_band_offset;
	UINT32 *n_band_offset;
	UINT32 n_band;
};

INT32 MT_ATERFTestCB(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
INT32 MT_SetATEMPSDump(struct _RTMP_ADAPTER *pAd, UINT32 band_idx);
INT32 MtTestModeInit(struct _RTMP_ADAPTER *pAd);
INT32 MtTestModeExit(struct _RTMP_ADAPTER *pAd);
INT MtTestModeBkCr(struct _RTMP_ADAPTER *pAd, ULONG offset, enum _TEST_BK_CR_TYPE type);
INT MtTestModeRestoreCr(struct _RTMP_ADAPTER *pAd, ULONG offset);
INT32 MT_ATETxControl(struct _RTMP_ADAPTER *pAd, UINT32 band_idx, PNDIS_PACKET pkt);
VOID MT_ATEUpdateRxStatistic(struct _RTMP_ADAPTER *pAd, enum _TESTMODE_STAT_TYPE type, VOID *data);
INT Mt_TestModeInsertPeer(struct _RTMP_ADAPTER *pAd, UINT32 band_ext, CHAR *da, CHAR *sa, CHAR *bss);
INT32 mt_ate_enq_pkt(struct _RTMP_ADAPTER *pAd, UINT32 band_idx, UINT16 wcid);	/* Export for Loopback */
INT MtATESetMacTxRx(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx);
INT MtATESetTxStream(struct _RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx);
INT MtATESetRxPath(struct _RTMP_ADAPTER *pAd, UINT32 RxPathSel, UCHAR BandIdx);
INT MtATESetRxFilter(struct _RTMP_ADAPTER *pAd, MT_RX_FILTER_CTRL_T filter);
INT MtATESetCleanPerStaTxQueue(struct _RTMP_ADAPTER *pAd, BOOLEAN sta_pause_enable);
INT32 MT_ATEDumpLog(struct _RTMP_ADAPTER *pAd, struct _ATE_LOG_DUMP_CB *log_cb, UINT32 log_type);
VOID MtCmdATETestResp(struct cmd_msg *msg, char *data, UINT16 len);
INT32 MtATECh2Freq(UINT32 Channel, UINT32 band_idx);
INT32 MtATEGetTxPwrGroup(UINT32 Channel, UINT32 band_idx, UINT32 Ant_idx);
INT32 MT_ATEInsertLog(struct _RTMP_ADAPTER *pAd, UCHAR *log, UINT32 log_type, UINT32 len);
#if !defined(COMPOS_TESTMODE_WIN)/* for MT_testmode.c */
INT MT_ATERxDoneHandle(struct _RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
#endif
INT32 MtATERSSIOffset(struct _RTMP_ADAPTER *pAd, INT32 RSSI_org, UINT32 RSSI_idx, INT32 Ch_Band);
INT32 MtATETssiTrainingProc(struct _RTMP_ADAPTER *pAd, UCHAR ucBW, UCHAR ucBandIdx);
#ifdef PRE_CAL_MT7622_SUPPORT
INT MtATE_DPD_Cal_Store_Proc_7622(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /*PRE_CAL_MT7622_SUPPORT*/
#ifdef PRE_CAL_MT7626_SUPPORT
INT MtATE_Pre_Cal_Store_Proc_7626(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT MtATE_DPD_Cal_Store_Proc_7626(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* PRE_CAL_MT7626_SUPPORT */
#ifdef PRE_CAL_MT7915_SUPPORT
INT MtATE_Group_Pre_Cal_Store_Proc_7915(RTMP_ADAPTER *pAd, UINT8 op);
INT MtATE_DPD_Cal_Store_Proc_7915(RTMP_ADAPTER *pAd, UINT8 op);
VOID MtATE_Dump_Group_PreCal_7915(RTMP_ADAPTER *pAd);
VOID MtATE_Dump_DPD_PreCal_7915(RTMP_ADAPTER *pAd);
#endif /* PRE_CAL_MT7915_SUPPORT */
#ifdef PRE_CAL_MT7986_SUPPORT
INT MtATE_Group_Pre_Cal_Store_Proc_7986(RTMP_ADAPTER *pAd, UINT8 op);
INT MtATE_DPD_Cal_Store_Proc_7986(RTMP_ADAPTER *pAd, UINT8 op);
VOID MtATE_Dump_Group_PreCal_7986(RTMP_ADAPTER *pAd);
VOID MtATE_Dump_DPD_PreCal_7986(RTMP_ADAPTER *pAd);
#endif /* PRE_CAL_MT7986_SUPPORT */
#ifdef PRE_CAL_MT7916_SUPPORT
INT MtATE_Group_Pre_Cal_Store_Proc_7916(RTMP_ADAPTER *pAd, UINT8 op);
INT MtATE_DPD_Cal_Store_Proc_7916(RTMP_ADAPTER *pAd, UINT8 op);
VOID MtATE_Dump_Group_PreCal_7916(RTMP_ADAPTER *pAd);
VOID MtATE_Dump_DPD_PreCal_7916(RTMP_ADAPTER *pAd);
#endif /* PRE_CAL_MT7916_SUPPORT */
#ifdef PRE_CAL_MT7981_SUPPORT
INT MtATE_Group_Pre_Cal_Store_Proc_7981(RTMP_ADAPTER *pAd, UINT8 op);
INT MtATE_DPD_Cal_Store_Proc_7981(RTMP_ADAPTER *pAd, UINT8 op);
VOID MtATE_Dump_Group_PreCal_7981(RTMP_ADAPTER *pAd);
VOID MtATE_Dump_DPD_PreCal_7981(RTMP_ADAPTER *pAd);
#endif /* PRE_CAL_MT7981_SUPPORT */
#ifdef PRE_CAL_TRX_SET1_SUPPORT
INT MtATE_DPD_Cal_Store_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT MtATE_DCOC_Cal_Store_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* PRE_CAL_TRX_SET1_SUPPORT */

#ifdef PRE_CAL_TRX_SET2_SUPPORT
INT MtATE_Pre_Cal_Proc(RTMP_ADAPTER *pAd, UINT8 CalId, UINT32 ChGrpId);
#endif /* PRE_CAL_TRX_SET2_SUPPORT */


#if defined(MT7986)
INT MtATE_DNL_Cal_Store_Proc_7986(
	RTMP_ADAPTER *pAd,
	RTMP_STRING * arg);
INT MtATE_RXGAIN_Cal_Store_Proc_7986(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg);
#endif
#if defined(MT7916)
INT MtATE_DNL_Cal_Store_Proc_7916(
	RTMP_ADAPTER *pAd,
	RTMP_STRING * arg);
INT MtATE_RXGAIN_Cal_Store_Proc_7916(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg);
#endif
#if defined(MT7981)
INT MtATE_DNL_Cal_Store_Proc_7981(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *arg);
INT MtATE_RXGAIN_Cal_Store_Proc_7981(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg);	
#endif

INT32 mt_ate_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
INT32 mt_ate_tx_v2(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
INT mt_ate_wtbl_cfg(RTMP_ADAPTER *pAd, UINT32 band_idx);
INT mt_ate_wtbl_cfg_v2(RTMP_ADAPTER *pAd, UINT32 band_idx);
INT ate_tx_pkt_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
INT ate_enqueue_mlme_pkt(RTMP_ADAPTER *pAd,
			 PNDIS_PACKET pkt,
			 struct wifi_dev *wdev,
			 UCHAR q_idx,
			 BOOLEAN is_data_queue);
NDIS_STATUS mt_ate_store_tx_info(struct _RTMP_ADAPTER *ad,
												UCHAR band_idx, struct wifi_dev *wdev,
												UINT_8 *da,
												struct _MAC_TABLE_ENTRY *mac_tbl_entry,
												struct _ATE_TX_INFO *ate_tx_info);
#ifdef DOT11_HE_AX
INT32 mt_ate_add_allocation(struct _ATE_RU_ALLOCATION *alloc_info, UINT8 allocation, UINT8 seg, UINT32 ru_index);
INT32 mt_ate_fill_empty_allocation(struct _ATE_RU_ALLOCATION *alloc_info);
#endif

#if defined(COMPOS_TESTMODE_WIN)
#endif

#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
INT MtATE_PA_Trim_Proc(RTMP_ADAPTER *pAd, PUINT32 pData);
#endif /* CAL_BIN_FILE_SUPPORT */

#define MT_ATEInit(_pAd) ({		\
		UINT32 _ret;					\
		_ret = MtTestModeInit(_pAd);	\
		_ret;							\
	})

#define MT_ATEExit(_pAd) ({		\
		UINT32 _ret;					\
		_ret = MtTestModeExit(_pAd);	\
		_ret;							\
	})
#endif /* _MT_TESTMODE_H */
