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
	ate.h
*/

#ifndef __ATE_H__
#define __ATE_H__
#include "LoopBack.h"
#if defined(MT_DMAC)
#include "mac/mac_mt/dmac/mt_dmac.h"
#endif

#if defined(MT_FMAC)
#include "mac/mac_mt/fmac/mt_fmac.h"
#endif

#ifndef COMPOS_TESTMODE_WIN
/* #define LOGDUMP_TO_FILE 1 */
#define ATE_TXTHREAD 1
#endif
#define IOCTLBUFF 2048

/* Note: temp for wlan_service/original coexistence */
#ifdef CONFIG_WLAN_SERVICE
#define SERV_ON(_p) ((_p->serv.serv_id == SERV_HANDLE_TEST)	\
		&& (((((struct service_test *)(_p->serv.serv_handle))->test_config[0].op_mode)	\
		& OP_MODE_START) == OP_MODE_START))
#define ATE_ON(_p) (((((_p)->ATECtrl.op_mode) & ATE_START) == ATE_START) || SERV_ON(_p))
#else
#define ATE_ON(_p) ((((_p)->ATECtrl.op_mode) & ATE_START) == ATE_START)
#endif /* CONFIG_WLAN_SERVICE */

INT32 ATEInit(struct _RTMP_ADAPTER *pAd);
INT32 ATEExit(struct _RTMP_ADAPTER *pAd);

#define FREQ_OFFSET_MANUAL_ENABLE	0x81021238
#define FREQ_OFFSET_MANUAL_VALUE	0x81021234

/* For CA53 GPIO CR remap usage */
#define CA53_GPIO_REMAP_SIZE 0x10

#ifdef ARBITRARY_CCK_OFDM_TX
/* MCU PTA CR */
#define ANT_SWITCH_CON2 0x810600CC
#define ANT_SWITCH_CON3 0x810600D0
#define ANT_SWITCH_CON4 0x810600D4
#define ANT_SWITCH_CON6 0x810600DC
#define ANT_SWITCH_CON7 0x810600E0
#define ANT_SWITCH_CON8 0x810600E4
#endif

/*
 *	Use bitmap to allow coexist of ATE_TXFRAME
 *	and ATE_RXFRAME(i.e.,to support LoopBack mode).
 */
#define fATE_IDLE				(1 << 0)
#define fATE_TX_ENABLE				(1 << 1)
#define fATE_RX_ENABLE				(1 << 2)
#define fATE_TXCONT_ENABLE			(1 << 3)
#define fATE_TXCARR_ENABLE			(1 << 4)
#define fATE_TXCARRSUPP_ENABLE			(1 << 5)
#define fATE_MPS				(1 << 6)
#define fATE_FFT_ENABLE				(1 << 7)
#define fATE_EXIT				(1 << 8)
#define fATE_IN_RFTEST				(1 << 9)
#define fATE_IN_BF				(1 << 10)
#define fATE_IN_ICAPOVERLAP			(1 << 11)
/* Stop Transmission */
#define ATE_TXSTOP				((~(fATE_TX_ENABLE))&(~(fATE_TXCONT_ENABLE))&(~(fATE_TXCARR_ENABLE))&(~(fATE_TXCARRSUPP_ENABLE))&(~(fATE_MPS)))
/* Stop Receiving Frames */
#define ATE_RXSTOP				(~(fATE_RX_ENABLE))

/* Enter/Reset ATE */
#define	ATE_START				(fATE_IDLE)
/* Stop/Exit ATE */
#define	ATE_STOP				(fATE_EXIT)
/* Continuous Transmit Frames (without time gap) */
#define	ATE_TXCONT				((fATE_TX_ENABLE)|(fATE_TXCONT_ENABLE))
/* Transmit Carrier */
#define	ATE_TXCARR				((fATE_TX_ENABLE)|(fATE_TXCARR_ENABLE))
/* Transmit Carrier Suppression (information without carrier) */
#define	ATE_TXCARRSUPP				((fATE_TX_ENABLE)|(fATE_TXCARRSUPP_ENABLE))
/* Transmit Frames */
#define	ATE_TXFRAME				(fATE_TX_ENABLE)
/* Receive Frames */
#define	ATE_RXFRAME				(fATE_RX_ENABLE)
/* MPS */
#define	ATE_MPS					((fATE_TX_ENABLE)|(fATE_MPS))

#define ATE_FFT					((fATE_FFT_ENABLE)|(fATE_IN_RFTEST))

/* WiFi PHY mode capability */
#define TEST_WMODE_CAP_24G		(WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G)
#define TEST_WMODE_CAP_5G		(WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G)
#define TEST_WMODE_CAP_6G		(WMODE_AN | WMODE_AC | WMODE_AX_6G)

/* ContiTxTone */
#define WF0_TX_ONE_TONE_5M		0x0
#define WF0_TX_TWO_TONE_5M		0x1
#define WF1_TX_ONE_TONE_5M		0x2
#define WF1_TX_TWO_TONE_5M		0x3
#define WF0_TX_ONE_TONE_10M		0x4
#define WF1_TX_ONE_TONE_10M		0x5
#define WF0_TX_ONE_TONE_DC		0x6
#define WF1_TX_ONE_TONE_DC		0x7

#define MAX_TEST_PKT_LEN        1496
#define MIN_TEST_PKT_LEN        25
#define MAX_TEST_BKCR_NUM       34

/* For packet tx time, in unit of byte */
#define MAX_HT_AMPDU_LEN        65000
#define MAX_VHT_MPDU_LEN        6700    /* 11454 */
#define DEFAULT_MPDU_LEN        4096
#define MAX_MSDU_LEN            2304
#define MIN_MSDU_LEN            22
#define DEFAULT_MAC_HDR_LEN     24
#define QOS_MAC_HDR_LEN         26

/* For ipg and duty cycle, in unit of us */
#define SIG_EXTENSION           6
#define DEFAULT_SLOT_TIME       9
#define DEFAULT_SIFS_TIME       10
#define MAX_SIFS_TIME           127     /* ICR has 7-bit only */ /* For the ATCR/TRCR limitation 8-bit/9-bit only*/
#define MAX_AIFSN               0xF
#define MIN_AIFSN               0x1
#define MAX_CW                  0x10
#define MIN_CW                  0x0
#define NORMAL_CLOCK_TIME       50      /* in uint of ns */
#define BBP_PROCESSING_TIME     1500    /* in uint of ns */

/* The expected enqueue packet number in one time RX event trigger */
#define ATE_ENQUEUE_PACKET_NUM	100

#if defined(DOT11_VHT_AC)
#define ATE_TESTPKT_LEN	13311	/* Setting max packet length to 13311 on MT7615 */
#else
#define ATE_TESTPKT_LEN	4095	/* AMPDU delimiter 12 bit, maximum 4095 */
#endif
#define ATE_MAX_PATTERN_SIZE 128
#define TESTMODE_BAND0 0
#define TESTMODE_BAND1 1
#define ATE_BF_WCID 1
#define ATE_BFMU_NUM 4
struct _RTMP_ADAPTER;
struct _RX_BLK;

#ifdef DBDC_MODE
#define IS_ATE_DBDC(_pAd) _pAd->CommonCfg.dbdc_mode
#define TESTMODE_BAND_NUM 2
#else
#define IS_ATE_DBDC(_pAd) FALSE
#define TESTMODE_BAND_NUM 1
#endif

#if defined(DOT11_HE_AX)
#define MAX_MULTI_TX_STA 16
#else
#define MAX_MULTI_TX_STA 2
#endif

#define MODE_VHT_MIMO 12	/* should aligh with rtmp_comm.h */
#define TX_MODE_MAX (MODE_VHT_MIMO+1)

/* Antenna mode */
#define ANT_MODE_DEFAULT 0
#define ANT_MODE_SPE_IDX 1

#if !defined(COMPOS_TESTMODE_WIN)
/* Allow Sleep */
#define TESTMODE_SEM struct semaphore
#define	TESTMODE_SEM_INIT(_psem, _val) sema_init(_psem, _val)
#define TESTMODE_SEM_DOWN(_psem) down(_psem)
#define TESTMODE_SEM_DOWN_INTERRUPTIBLE(_psem) down_interruptible(_psem)
#define TESTMODE_SEM_DOWN_TRYLOCK(_psem) down_trylock(_psem)
#define TESTMODE_SEM_UP(_psem) up(_psem)
#endif


enum _TESTMODE_MODE {
	HQA_VERIFY,
	ATE_LOOPBACK,
	MODE_NUM
};

enum _MPS_PARAM_TYPE {
	MPS_SEQDATA,
	MPS_PHYMODE,
	MPS_PATH,
	MPS_RATE,
	MPS_PAYLOAD_LEN,
	MPS_TX_COUNT,
	MPS_PWR_GAIN,
	MPS_PARAM_NUM,
	MPS_NSS,
	MPS_PKT_BW,
};

enum _RECAL_IN_DUMP_STATUS {
	DISABLE_RECAL_IN_DUMP  = 0,
	REENABLE_RECAL_IN_DUMP = 1,
	NO_CHANGE_RECAL        = 2,
	STATUS_NUM             = 3,
};

struct _ATE_CHIP_OPERATION {
	BOOLEAN (*fill_tx_blk)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, void *tx_blk);
};

struct _ATE_OPERATION {
	INT32 (*ATEStart)(struct _RTMP_ADAPTER *pAd);
	INT32 (*ATEStop)(struct _RTMP_ADAPTER *pAd);
	INT32 (*tx_commit)(struct _RTMP_ADAPTER *ad);
	INT32 (*tx_revert)(struct _RTMP_ADAPTER *ad);
	INT32 (*StartTx)(struct _RTMP_ADAPTER *pAd);
	INT32 (*StartRx)(struct _RTMP_ADAPTER *pAd);
	INT32 (*StopTx)(struct _RTMP_ADAPTER *pAd);
	INT32 (*StopRx)(struct _RTMP_ADAPTER *pAd);
	INT32 (*SetTxPath)(struct _RTMP_ADAPTER *pAd);
	INT32 (*SetRxPath)(struct _RTMP_ADAPTER *pAd);
	INT32 (*SetRxUserIdx)(struct _RTMP_ADAPTER *pAd, UCHAR band_idx, UINT16 user_idx);
	INT32 (*SetTxPower0)(struct _RTMP_ADAPTER *pAd, struct _ATE_TXPOWER TxPower);
	INT32 (*SetTxPower1)(struct _RTMP_ADAPTER *pAd, struct _ATE_TXPOWER TxPower);
	INT32 (*SetTxPower2)(struct _RTMP_ADAPTER *pAd, struct _ATE_TXPOWER TxPower);
	INT32 (*SetTxPower3)(struct _RTMP_ADAPTER *pAd, struct _ATE_TXPOWER TxPower);
	INT32 (*SetTxForceTxPower)(struct _RTMP_ADAPTER *pAd, INT8 cTxPower, UINT8 ucTxMode, UINT8 ucTxRate, UINT8 ucBW);
	INT32 (*SetTxPowerX)(struct _RTMP_ADAPTER *pAd, struct _ATE_TXPOWER TxPower);
	INT32 (*SetTxAntenna)(struct _RTMP_ADAPTER *pAd, UINT32 Ant);
	INT32 (*SetRxAntenna)(struct _RTMP_ADAPTER *pAd, UINT32 Ant);
	INT32 (*SetTxFreqOffset)(struct _RTMP_ADAPTER *pAd, UINT32 FreqOffset);
	INT32 (*GetTxFreqOffset)(struct _RTMP_ADAPTER *pAd, UINT32 *FreqOffset);
	INT32 (*SetChannel)(struct _RTMP_ADAPTER *pAd, INT16 Value, UINT32 pri_sel, UINT32 reason, UINT32 Ch_Band);
	INT32 (*SetBW)(struct _RTMP_ADAPTER *pAd, UINT16 system_bw, UINT16 per_pkt_bw);
	INT32 (*SetDutyCycle)(struct _RTMP_ADAPTER *pAd, UINT32 value);
	INT32 (*SetPktTxTime)(struct _RTMP_ADAPTER *pAd, UINT32 value);
	INT32 (*SampleRssi)(struct _RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk);
	INT32 (*SetIPG)(struct _RTMP_ADAPTER *pAd, UINT32 value);
	INT32 (*SetSlotTime)(struct _RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime);
	INT32 (*SetAIFS)(struct _RTMP_ADAPTER *pAd, CHAR Value);
	INT32 (*SetPowerDropLevel)(struct _RTMP_ADAPTER *pAd, UINT32 PowerDropLevel);
	INT32 (*SetTSSI)(struct _RTMP_ADAPTER *pAd, CHAR WFSel, CHAR Setting);
	INT32 (*LowPower)(struct _RTMP_ADAPTER *pAd, UINT32 Control);
	INT32 (*SetEepromToFw)(struct _RTMP_ADAPTER *pAd);
	INT32 (*SetDPD)(struct _RTMP_ADAPTER *pAd, CHAR WFSel, CHAR Setting);
	INT32 (*StartTxTone)(struct _RTMP_ADAPTER *pAd, UINT32 Mode);
	INT32 (*SetTxTonePower)(struct _RTMP_ADAPTER *pAd, INT32 pwr1, INT pwr2);
	INT32 (*SetDBDCTxTonePower)(struct _RTMP_ADAPTER *pAd, INT32 pwr1, INT pwr2, UINT32 AntIdx);
	INT32 (*GetDBDCTxTonePower)(struct _RTMP_ADAPTER *pAd, PINT32 pPwr, UINT32 AntIdx);
	INT32 (*StopTxTone)(struct _RTMP_ADAPTER *pAd);
	INT32 (*StartContinousTx)(struct _RTMP_ADAPTER *pAd, CHAR WFSel, UINT32 TxfdMode);
	INT32 (*StopContinousTx)(struct _RTMP_ADAPTER *pAd, UINT32 TxfdMode);
	INT32 (*EfuseGetFreeBlock)(struct _RTMP_ADAPTER *pAd, PVOID GetFreeBlock, PVOID Result);
	INT32 (*EfuseAccessCheck)(struct _RTMP_ADAPTER *pAd, UINT32 offset, PUCHAR pData);
	INT32 (*RfRegWrite)(struct _RTMP_ADAPTER *pAd, UINT32 WFSel, UINT32 Offset, UINT32 Value);
	INT32 (*RfRegRead)(struct _RTMP_ADAPTER *pAd, UINT32 WFSel, UINT32 Offset, UINT32 *Value);
	INT32 (*GetFWInfo)(struct _RTMP_ADAPTER *pAd, UCHAR *FWInfo);
#ifdef PRE_CAL_MT7622_SUPPORT
	INT32 (*TxDPDTest7622)(struct _RTMP_ADAPTER *pAd, RTMP_STRING * arg);
#endif /*PRE_CAL_MT7622_SUPPORT*/
#ifdef PRE_CAL_MT7626_SUPPORT
	INT32 (*PreCal7626)(struct _RTMP_ADAPTER *pAd, RTMP_STRING * arg);
	INT32 (*TxDPD7626)(struct _RTMP_ADAPTER *pAd, RTMP_STRING * arg);
#endif /* PRE_CAL_MT7626_SUPPORT */
#ifdef PRE_CAL_MT7915_SUPPORT
	INT32 (*PreCal7915)(struct _RTMP_ADAPTER *pAd, UINT8 op);
	INT32 (*TxDPD7915)(struct _RTMP_ADAPTER *pAd, UINT8 op);
#endif /* PRE_CAL_MT7915_SUPPORT */
#ifdef PRE_CAL_MT7986_SUPPORT
	INT32 (*PreCal7986)(struct _RTMP_ADAPTER *pAd, UINT8 op);
	INT32 (*TxDPD7986)(struct _RTMP_ADAPTER *pAd, UINT8 op);
#endif /* PRE_CAL_MT7986_SUPPORT */
#ifdef PRE_CAL_MT7916_SUPPORT
	INT32 (*PreCal7916)(struct _RTMP_ADAPTER *pAd, UINT8 op);
	INT32 (*TxDPD7916)(struct _RTMP_ADAPTER *pAd, UINT8 op);
#endif /* PRE_CAL_MT7916_SUPPORT */
#ifdef PRE_CAL_MT7981_SUPPORT
	INT32 (*PreCal7981)(struct _RTMP_ADAPTER *pAd, UINT8 op);
	INT32 (*TxDPD7981)(struct _RTMP_ADAPTER *pAd, UINT8 op);
#endif /* PRE_CAL_MT7981_SUPPORT */
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	INT32 (*RxSelfTest)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	INT32 (*TxDPDTest)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
#ifdef PRE_CAL_TRX_SET2_SUPPORT
	INT32 (*PreCalTest)(struct _RTMP_ADAPTER *pAd, UINT8 CalId, UINT32 ChGrpId);
#endif /* PRE_CAL_TRX_SET2_SUPPORT */
#if defined(CAL_BIN_FILE_SUPPORT) && defined(MT7615)
	INT32 (*PATrim)(struct _RTMP_ADAPTER *pAd, PUINT32 pData);
#endif /* CAL_BIN_FILE_SUPPORT */
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
	INT32 (*SetATETxSoundingProc)(struct _RTMP_ADAPTER *pAd, UCHAR SoundingMode);
	INT32 (*StartTxSKB)(struct _RTMP_ADAPTER *pAd);
#endif /* TXBF_SUPPORT && MT_MAC */
#if defined(MT7986)
	INT32 (*DnlK7986)(
		struct _RTMP_ADAPTER *pAd,
		RTMP_STRING * arg);
	INT32 (*RXGAINK7986)(
		struct _RTMP_ADAPTER *pAd,
		RTMP_STRING * arg);
#endif
#if defined(MT7916)
	INT32 (*DnlK7916)(
		struct _RTMP_ADAPTER *pAd,
		RTMP_STRING * arg);
	INT32 (*RXGAINK7916)(
		struct _RTMP_ADAPTER *pAd,
		RTMP_STRING * arg);
#endif
#if defined(MT7981)
	INT32 (*DnlK7981)(
		struct _RTMP_ADAPTER *pAd,
		RTMP_STRING * arg);
	INT32 (*RXGAINK7981)(
		struct _RTMP_ADAPTER *pAd,
		RTMP_STRING * arg);
#endif
	INT32 (*MPSSetParm)(struct _RTMP_ADAPTER *pAd, enum _MPS_PARAM_TYPE data_type, INT32 items, UINT32 *data);
	INT32 (*MPSTxStart)(struct _RTMP_ADAPTER *pAd);
	INT32 (*MPSTxStop)(struct _RTMP_ADAPTER *pAd);
	INT32 (*SetAutoResp)(struct _RTMP_ADAPTER *pAd, UCHAR *mac, UCHAR mode);
	INT32 (*SetFFTMode)(struct _RTMP_ADAPTER *pAd, UINT32 mode);
	INT32 (*onOffRDD)(struct _RTMP_ADAPTER *pAd, UINT32 rdd_num, UINT32 rdd_in_sel, UINT32 is_start);
	INT32 (*SetCfgOnOff)(struct _RTMP_ADAPTER *pAd, UINT32 Type, UINT32 Enable);
	INT32 (*GetCfgOnOff)(struct _RTMP_ADAPTER *pAd, UINT32 Type, UINT32 *Result);
	INT32 (*SetRXFilterPktLen)(struct _RTMP_ADAPTER *pAd, UINT32 Enable, UINT32 RxPktLen);
	INT32 (*DBDCTxTone)(struct _RTMP_ADAPTER *pAd, UINT32 Control, UINT32 AntIndex, UINT32 ToneType, UINT32 ToneFreq, INT32 DcOffset_I, INT32 DcOffset_Q, UINT32 Band);
	INT32 (*GetTxPower)(struct _RTMP_ADAPTER *pAd, UINT32 Enable, UINT32 Ch_Band, UINT32 u4AntIdx, PUINT32 Power);
	INT32 (*BssInfoUpdate)(struct _RTMP_ADAPTER *pAd, UINT32 OwnMacIdx, UINT32 BssIdx, UCHAR *Bssid);
	INT32 (*DevInfoUpdate)(struct _RTMP_ADAPTER *pAd, UINT32 OwnMacIdx, UCHAR *Bssid);
	INT32 (*LogOnOff)(struct _RTMP_ADAPTER *pAd, UINT32 type, UINT32 on_off, UINT32 num_log);
	INT32 (*GetDumpRXV)(struct _RTMP_ADAPTER *pAd, PINT32 pData, PINT32 pCount);
	INT32 (*SetICapStart)(struct _RTMP_ADAPTER *pAd, PUINT8 pData);
	INT32 (*GetICapStatus)(struct _RTMP_ADAPTER *pAd);
	INT32 (*GetICapIQData)(struct _RTMP_ADAPTER *pAd, PINT32 pData, PINT32 pDataLen, UINT32 IQ_Type, UINT32 WF_Num);
	INT32 (*SetAntennaPort)(struct _RTMP_ADAPTER *pAd, UINT32 RfModeMask, UINT32 RfPortMask, UINT32 AntPortMask);
	INT32 (*ClockSwitchDisable)(struct _RTMP_ADAPTER *pAd, UINT8 isDisable);
#if OFF_CH_SCAN_SUPPORT
	INT32 (*off_ch_scan)(struct _RTMP_ADAPTER *pAd, struct _ATE_OFF_CH_SCAN *param);
#endif
#if defined(DOT11_HE_AX)
	INT32 (*show_ru_info)(struct _RTMP_ADAPTER *ad, UINT8 band_idx);
	INT32 (*set_ru_info)(struct _RTMP_ADAPTER *ad, UINT8 band_idx, UCHAR *str);
#endif
};

struct _ATE_IF_OPERATION {
	INT32 (*init)(struct _RTMP_ADAPTER *pAd);
	INT32 (*clean_trx_q)(struct _RTMP_ADAPTER *pAd);
	INT32 (*setup_frame)(struct _RTMP_ADAPTER *pAd, UINT32 q_idx);
	INT32 (*test_frame_tx)(struct _RTMP_ADAPTER *pAd);
	INT32 (*test_frame_rx)(struct _RTMP_ADAPTER *pAd);
	INT32 (*ate_leave)(struct _RTMP_ADAPTER *pAd);
};

struct _HQA_MPS_SETTING {
	UINT32 phy;
	UINT32 pkt_len;
	UINT32 pkt_cnt;
	UINT32 pwr;
	UINT32 nss;
	UINT32 pkt_bw;
};

enum _TEST_BK_CR_TYPE {
	TEST_EMPTY_BKCR = 0,
	TEST_MAC_BKCR,
	TEST_HIF_BKCR,
	TEST_PHY_BKCR,
	TEST_HW_BKCR,
	TEST_MCU_BKCR,
	TEST_BKCR_TYPE_NUM,
};

struct _TESTMODE_BK_CR {
	ULONG offset;
	UINT32 val;
	enum _TEST_BK_CR_TYPE type;
};

struct _ATE_RXV_LOG {
	RX_VECTOR1_1ST_CYCLE rxv1_1st;
	RX_VECTOR1_2ND_CYCLE rxv1_2nd;
	RX_VECTOR1_3TH_CYCLE rxv1_3rd;
	RX_VECTOR1_4TH_CYCLE rxv1_4th;
	RX_VECTOR1_5TH_CYCLE rxv1_5th;
	RX_VECTOR1_6TH_CYCLE rxv1_6th;
	RX_VECTOR2_1ST_CYCLE rxv2_1st;
	RX_VECTOR2_2ND_CYCLE rxv2_2nd;
	RX_VECTOR2_3TH_CYCLE rxv2_3rd;
};

#define ATE_RDD_LOG_SIZE 8 /* Pulse size * num of pulse = 8 * 32 for one event*/
struct _ATE_RDD_LOG {
	UINT32 u4Prefix;
	UINT32 u4Count;
	UINT8 byPass;
	UINT8 aucBuffer[ATE_RDD_LOG_SIZE];
};

#define ATE_RECAL_LOG_SIZE (CAL_ALL_LEN >> 3)
struct _ATE_LOG_RECAL {
	UINT32 cal_idx;
	UINT32 cal_type;
	UINT32 cr_addr;
	UINT32 cr_val;
};

enum {
	ATE_LOG_RXV = 1,
	ATE_LOG_RDD,
	ATE_LOG_RE_CAL,
	ATE_LOG_TYPE_NUM,
	ATE_LOG_RXINFO,
	ATE_LOG_TXDUMP,
	ATE_LOG_TEST,
	ATE_LOG_TXSSHOW,
};

enum {
	ATE_LOG_OFF,
	ATE_LOG_ON,
	ATE_LOG_DUMP,
	ATE_LOG_CTRL_NUM,
};

#define fATE_LOG_RXV				(1 << ATE_LOG_RXV)
#define fATE_LOG_RDD				(1 << ATE_LOG_RDD)
#define fATE_LOG_RE_CAL				(1 << ATE_LOG_RE_CAL)
#define fATE_LOG_RXINFO				(1 << ATE_LOG_RXINFO)
#define fATE_LOG_TXDUMP				(1 << ATE_LOG_TXDUMP)
#define fATE_LOG_TEST				(1 << ATE_LOG_TEST)
#define fATE_LOG_TXSSHOW			(1 << ATE_LOG_TXSSHOW)

struct _ATE_LOG_DUMP_ENTRY {
	UINT32 log_type;
	UINT8 un_dumped;
	union {
		struct _ATE_RXV_LOG rxv;
		struct _ATE_RDD_LOG rdd;
		struct _ATE_LOG_RECAL re_cal;
	} log;
};

struct _ATE_LOG_DUMP_CB {
	NDIS_SPIN_LOCK lock;
	UINT8 overwritable;
	UINT8 is_dumping;
	UINT8 is_overwritten;
	INT32 idx;
	INT32 len;
	UINT32 recal_curr_type;
#ifdef LOGDUMP_TO_FILE
	INT32 file_idx;
	RTMP_OS_FD_EXT fd;
#endif
	struct _ATE_LOG_DUMP_ENTRY *entry;
};

#define ATE_MPS_ITEM_RUNNING	(1<<0)
struct _HQA_MPS_CB {
	NDIS_SPIN_LOCK lock;
	UINT32 mps_cnt;
	UINT32 band_idx;
	UINT32 stat;
	BOOLEAN setting_inuse;
	UINT32 ref_idx;
	struct _HQA_MPS_SETTING *mps_setting;
};

struct _ATE_PFMU_INFO {
	UINT16 wcid;
	UCHAR bss_idx;
	UCHAR up;
	UCHAR addr[MAC_ADDR_LEN];
};

struct _ATE_RU_ALLOCATION {
	UINT8 allocation[(0x1 << BW_160)];
};

struct _ATE_RU_STA {
	BOOLEAN valid;
	UINT32 allocation;
	UINT32 aid;
	UINT32 ru_index;
	UINT32 rate;
	UINT32 ldpc;
	UINT32 nss;
	UINT32 start_sp_st;
	UINT32 mpdu_length;
	INT32 alpha;
	UINT32 ru_mu_nss;
	/* end of user input*/
	UINT32 t_pe;
	UINT32 afactor_init;
	UINT32 symbol_init;
	UINT32 excess;
	UINT32 dbps;
	UINT32 cbps;
	UINT32 dbps_s;
	UINT32 cbps_s;
	UINT32 pld;
	UINT32 avbits;
	UINT32 dbps_last;
	UINT32 cbps_last;
	UCHAR ldpc_extr_sym;
	UINT32 tx_time_x5;
	UCHAR pe_disamb;
	UINT16 punc;
	UINT32 l_len;
};

#define ATE_RXV_SIZE    9
#define ATE_ANT_NUM     4
#define ATE_DBDC_ANT_NUM    2
#define ATE_USER_NUM    16

struct _ATE_RX_STATISTIC {
	INT32 FreqOffsetFromRx[ATE_USER_NUM];
	UINT32 RxTotalCnt[TESTMODE_BAND_NUM];
	UINT32 NumOfAvgRssiSample;
	UINT32 RxMacFCSErrCount;
	UINT32 RxMacMdrdyCount;
	UINT32 RxMacFCSErrCount_band1;
	UINT32 RxMacMdrdyCount_band1;
	CHAR LastSNR[ATE_ANT_NUM];		/* last received SNR */
	CHAR LastRssi[ATE_ANT_NUM];		/* last received RSSI */
	CHAR AvgRssi[ATE_ANT_NUM];		/* last 8 frames' average RSSI */
	CHAR MaxRssi[ATE_ANT_NUM];
	CHAR MinRssi[ATE_ANT_NUM];
	SHORT AvgRssiX8[ATE_ANT_NUM];		/* sum of last 8 frames' RSSI */
	UINT32 RSSI[ATE_ANT_NUM];
	UINT32 SNR[ATE_USER_NUM];
	UINT32 RCPI[ATE_ANT_NUM];
	UINT32 FAGC_RSSI_IB[ATE_ANT_NUM];
	UINT32 FAGC_RSSI_WB[ATE_ANT_NUM];
#ifdef CFG_SUPPORT_MU_MIMO
	UINT32 RxMacMuPktCount;
#endif
	UINT32 SIG_MCS;
	UINT32 SINR;
	UINT32 RXVRSSI;
	UINT32 fcs_error_cnt[ATE_USER_NUM];
};

struct _ATE_TX_TIME_PARAM {
	BOOLEAN pkt_tx_time_en;	/* The packet transmission time feature enable or disable */
	UINT32 pkt_tx_time;	/* The target packet transmission time */
	UINT32 pkt_tx_len;
	UINT32 pkt_msdu_len;
	UINT32 pkt_hdr_len;
	UINT32 pkt_ampdu_cnt;
	UINT8 pkt_need_qos;
	UINT8 pkt_need_amsdu;
	UINT8 pkt_need_ampdu;
};

struct _ATE_IPG_PARAM {
	UINT32 ipg;         /* The target idle time */
	UINT8 sig_ext;      /* Only OFDM/HT/VHT need to consider sig_ext */
	UINT16 slot_time;
	UINT16 sifs_time;
	UINT8 ac_num;       /* 0: AC_BK, 1: AC_BE, 2: AC_VI, 3: AC_VO */
	UINT8 aifsn;
	UINT16 cw;
	UINT16 txop;
};

struct _MAC_TABLE_ENTRY_STACK {
	UINT8 entry_limit;
	UINT8 index;
	UINT8 q_idx;
	UINT16 quota;
	UINT8 da[MAX_MULTI_TX_STA][MAC_ADDR_LEN];
	void *mac_tbl_entry[MAX_MULTI_TX_STA];
	void *wdev[MAX_MULTI_TX_STA];
	void *pkt_skb[MAX_MULTI_TX_STA];
};

struct _BAND_INFO {
	UCHAR tx_method[TX_MODE_MAX];
	UCHAR *test_pkt;	/* Buffer for TestPkt */
	PNDIS_PACKET pkt_skb[MAX_MULTI_TX_STA];
	UINT32 is_alloc_skb;
	RTMP_OS_COMPLETION tx_wait;
	UCHAR TxStatus;	/* TxStatus : 0 --> task is idle, 1 --> task is running */
	UINT32 op_mode;
	UINT32 tx_ant;
	UINT32 rx_ant;
	UCHAR backup_channel;
	UCHAR backup_phymode;
	void *wdev[2];
	struct _MAC_TABLE_ENTRY_STACK stack;
	UCHAR wdev_idx;
	UCHAR wmm_idx;
	USHORT ac_idx;
	UCHAR channel;
	UCHAR ch_band;
	UCHAR ctrl_ch;
	UCHAR ch_offset;
	UINT32 out_band_freq;
	UCHAR nss;
	UCHAR bw;
	UCHAR per_pkt_bw;
	UCHAR pri_sel;
	UCHAR tx_mode;
	UCHAR stbc;
	UCHAR ldpc;	/* 0:BCC 1:LDPC */
	UCHAR ltf_gi;
	UCHAR sgi;
	UCHAR mcs;
	UCHAR preamble;
	UINT16 user_idx;
	UINT32 fixed_payload;	/* Normal:0,Repeat:1,Random:2 */
	UINT32 tx_len;
	UINT32 tx_cnt;
	UINT32 tx_done_cnt;	/* Tx DMA Done */
	UINT32 txed_cnt;
	UCHAR remain_tx_cnt;
	UCHAR ba_disable;
	UINT32 rf_freq_offset;
	UINT32 thermal_val;
	UINT32 duty_cycle;
	UINT32 mu_rx_aid;
	UINT8 retry;
	TMAC_INFO tmac_info;
	struct _ATE_TX_TIME_PARAM tx_time_param;
	struct _ATE_IPG_PARAM ipg_param;
	UINT8 dmnt_ru_idx;
	struct _ATE_RU_ALLOCATION ru_alloc;
	struct _ATE_RU_STA ru_info_list[MAX_MULTI_TX_STA];
	UCHAR max_pkt_ext;
#ifdef TXBF_SUPPORT
	BOOLEAN fgBw160;
	UCHAR ebf;
	UCHAR ibf;
	UCHAR *txbf_info;
	UINT32 txbf_info_len;
	UCHAR iBFCalStatus;
	BOOLEAN fgEBfEverEnabled;
#endif
#ifdef DOT11_VHT_AC
	UCHAR channel_2nd;
#endif
	UCHAR fagc_path;
	/* Tx frame */
	UCHAR template_frame[32];
	UCHAR addr1[MAX_MULTI_TX_STA][MAC_ADDR_LEN];
	UCHAR addr2[MAX_MULTI_TX_STA][MAC_ADDR_LEN];
	UCHAR addr3[MAX_MULTI_TX_STA][MAC_ADDR_LEN];
	UCHAR payload[ATE_MAX_PATTERN_SIZE];
	UINT32 pl_len;
	USHORT hdr_len;		/* Header Length */
	USHORT seq;
	struct _HQA_MPS_CB mps_cb;
	BOOLEAN  tx_pwr_sku_en;        /* SKU On/Off status */
	BOOLEAN  tx_pwr_percentage_en; /* Power Percentage On/Off status */
	BOOLEAN  tx_pwr_backoff_en;  /* BF Backoff On/Off status */
	UINT32   tx_pwr_percentage_level;       /* TxPower Percentage Level */
	UCHAR   tx_strm_num;           /* TX stream number */
	UCHAR   rx_strm_pth;           /* RX antenna path */
	ULONGLONG hetb_rx_csd;
};

#ifdef ATE_TXTHREAD
#define ATE_THREAD_NUM 1
struct _ATE_TXTHREAD_CB {
	BOOLEAN is_init;
	RTMP_OS_TASK task;
	NDIS_SPIN_LOCK lock;
	UINT32 tx_cnt;
	UINT32 txed_cnt;
	UCHAR service_stat;
};

struct _ATE_PERIODIC_THREAD_CB {
	BOOLEAN is_init;
	RTMP_OS_TASK task;
	NDIS_SPIN_LOCK lock;
	BOOLEAN service_stat;
};
#endif

struct _ATE_TX_INFO {
	UINT8 tx_mode;
	UINT8 bw;
	UINT8 stbc;
	UINT8 ldpc;
	UINT8 ltf;
	UINT8 gi;
	UINT8 mcs;
	UINT8 nss;
	UINT8 ibf;
	UINT8 ebf;
	UINT32 mpdu_length;
};

struct _ATE_CTRL {
	struct _ATE_CHIP_OPERATION *ate_chip_ops;
	struct _ATE_OPERATION *ATEOp;
	struct _ATE_IF_OPERATION *ATEIfOps;
	enum _TESTMODE_MODE verify_mode;
	struct _HQA_MPS_CB mps_cb;
	UINT32 en_log;
	UCHAR tx_method[TX_MODE_MAX];
#ifdef ATE_TXTHREAD
	struct _ATE_TXTHREAD_CB tx_thread[1];
	UINT32 current_init_thread;
	INT32 deq_cnt;
#endif
	HTTRANSMIT_SETTING HTTransmit[DBDC_BAND_NUM];
	struct phy_params phy_info[DBDC_BAND_NUM];
	struct _BAND_INFO band_ext[1];
	USHORT tx_path;
	USHORT rx_path;
	UCHAR *test_pkt;	/* Buffer for TestPkt */
	PNDIS_PACKET pkt_skb[MAX_MULTI_TX_STA];
	UINT32 is_alloc_skb;
	UINT32 op_mode;
	UINT32 use_apcli;
	CHAR TxPower0;
	CHAR TxPower1;
	CHAR TxPower2;
	CHAR TxPower3;
	UINT32 tx_ant;	/* band0 => TX0/TX1 , band1 => TX2/TX3 */
	UINT32 rx_ant;
	void *wdev[2];
	struct _MAC_TABLE_ENTRY_STACK stack;
	UCHAR wdev_idx;
	UCHAR wmm_idx;
	USHORT ac_idx;
	UCHAR channel;
	UCHAR ch_band;
	UCHAR ctrl_ch;
	UCHAR ch_offset;
	UINT32 out_band_freq;
	UCHAR nss;
	UCHAR bw;
	UCHAR per_pkt_bw;
	UCHAR pri_sel;
	UCHAR tx_mode;
	UCHAR stbc;
	UCHAR ldpc;			/* 0:BCC 1:LDPC */
	UCHAR sgi;			/* for HT/VHT, means guard intercal ; for HE, means ltf+gi combination accodring to 8021.11ax SPEC */
	UCHAR mcs;			/* rate index */
	UCHAR preamble;
	UCHAR Payload;		/* Payload pattern */
	UINT16 user_idx;
	UINT32 fixed_payload;
	UINT32 tx_len;
	UINT32 tx_cnt;
	UINT32 tx_done_cnt;	/* Tx DMA Done */
	UINT32 txed_cnt;
	UINT32 rf_freq_offset;
	UINT32 thermal_val;
	UINT32 duty_cycle;
	UINT32 mu_rx_aid;
	UINT8 retry;
	TMAC_INFO tmac_info;
	struct _ATE_TX_TIME_PARAM tx_time_param;
	struct _ATE_IPG_PARAM ipg_param;
	UCHAR max_pkt_ext;
#ifdef TXBF_SUPPORT
	BOOLEAN fgEBfEverEnabled;
	BOOLEAN fgBw160;
	UCHAR ebf;
	UCHAR ibf;
	UCHAR *txbf_info;
	UINT32 txbf_info_len;
	UCHAR iBFCalStatus;
#endif
#ifdef INTERNAL_CAPTURE_SUPPORT
	EXT_EVENT_RBIST_ADDR_T icap_info;
#endif /* INTERNAL_CAPTURE_SUPPORT */
#ifdef DOT11_VHT_AC
	UCHAR channel_2nd;
#endif
	/* Common part */
	UCHAR control_band_idx; /* The band_idx which user wants to control currently */
	/* Tx frame */
	UCHAR template_frame[32];
	UCHAR addr1[MAX_MULTI_TX_STA][MAC_ADDR_LEN];
	UCHAR addr2[MAX_MULTI_TX_STA][MAC_ADDR_LEN];
	UCHAR addr3[MAX_MULTI_TX_STA][MAC_ADDR_LEN];
	UCHAR payload[ATE_MAX_PATTERN_SIZE]; /* Payload pattern */
	UINT32 pl_len;
	USHORT hdr_len;		/* Header Length */
	USHORT seq;
	/* MU Related */
	BOOLEAN mu_enable;
	UINT32 mu_usrs;
	UINT16 wcid_ref;
	struct _ATE_PFMU_INFO pfmu_info[ATE_BFMU_NUM];
	UINT8 dmnt_ru_idx;
	struct _ATE_RU_ALLOCATION ru_alloc;
	struct _ATE_RU_STA ru_info_list[MAX_MULTI_TX_STA];
	/* counters */
	UINT32 num_rxv;
	UINT32 num_rxdata;
	UINT32 num_rxv_fcs;
	UINT32 num_rxdata_fcs;
	struct _ATE_RX_STATISTIC rx_stat;
	struct _ATE_LOG_DUMP_CB log_dump[ATE_LOG_TYPE_NUM];
	UCHAR fagc_path;
	/* Flag */
	BOOLEAN txs_enable;
	BOOLEAN	bQAEnabled;	/* QA is used. */
	BOOLEAN bQATxStart;	/* Have compiled QA in and use it to ATE tx. */
	BOOLEAN bQARxStart;	/* Have compiled QA in and use it to ATE rx. */
	BOOLEAN need_set_pwr;	/* For MPS switch power in right context */
	UCHAR TxStatus;		/* TxStatus : 0 --> task is idle, 1 --> task is running */
	UCHAR did_tx;
	UCHAR did_rx;
	UCHAR en_man_set_freq;
	/* Restore CR */
	struct _TESTMODE_BK_CR bk_cr[MAX_TEST_BKCR_NUM];
	/* OS related */
	RTMP_OS_COMPLETION tx_wait;
	RTMP_OS_COMPLETION cmd_done;
	ULONG cmd_expire;
#if !defined(COMPOS_TESTMODE_WIN)
	RALINK_TIMER_STRUCT PeriodicTimer;
	ULONG OneSecPeriodicRound;
	ULONG PeriodicRound;
	OS_NDIS_SPIN_LOCK TssiSemLock;
#endif
	BOOLEAN  tx_pwr_sku_en;        /* SKU On/Off status */
	BOOLEAN  tx_pwr_percentage_en; /* Power Percentage On/Off status */
	BOOLEAN  tx_pwr_backoff_en;  /* BF Backoff On/Off status */
	UINT32   tx_pwr_percentage_level;       /* TxPower Percentage Level */

	BOOLEAN  firstReCal;                          /* Enable/Disable Recal */
	BOOLEAN  firstRDD;                            /* Enable/Disable RDD */
	BOOLEAN  firstQATool;                         /* Enable/Disable QA Tool */
	BOOLEAN  firstRXV;                            /* Enable/Disable RXV */
	enum _RECAL_IN_DUMP_STATUS  reCalInDumpSts;   /* Recal status during dumping */
	UINT32   logStsInDump[3];                     /* 0: log_type, 1: log_ctrl, 2: log_size*/
	UCHAR   tx_strm_num;           /* TX stream number */
	UCHAR   rx_strm_pth;           /* RX antenna path */
#ifdef ATE_TXTHREAD
	struct _ATE_PERIODIC_THREAD_CB periodic_thread;
#endif
	ULONGLONG hetb_rx_csd;
	BOOLEAN backup_bEnableTxBurst;
	USHORT backup_bcn_period;
};

#if !defined(COMPOS_TESTMODE_WIN)
VOID RtmpDmaEnable(RTMP_ADAPTER *pAd, INT Enable);

VOID ATE_RTUSBBulkOutDataPacket(
	IN      PRTMP_ADAPTER	pAd,
	IN      UCHAR		BulkOutPipeId);

VOID ATE_RTUSBCancelPendingBulkInIRP(
	IN      PRTMP_ADAPTER   pAd);
#endif
INT MtATESetMacTxRx(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx);
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
INT SetATEApplyStaToMacTblEntry(RTMP_ADAPTER *pAd);
INT SetATEApplyStaToAsic(RTMP_ADAPTER *pAd);
#endif

INT ate_inf_open(struct wifi_dev *wdev);
INT ate_inf_close(struct wifi_dev *wdev);
INT ate_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);

#endif /*  __ATE_H__ */
