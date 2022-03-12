#ifndef __HE_CFG_H__
#define __HE_CFG_H__

#ifdef CFG_SUPPORT_FALCON_TXCMD_DBG

#define MAX_NUM_TXCMD_RU_INFO			16
#define MAX_NUM_TXCMD_TX_USER           	16
#define MAX_NUM_TXCMD_RX_USER           	16
#define MAX_NUM_TXCMD_USER_INFO			MAX_NUM_TXCMD_TX_USER
#define MAX_NUM_TXCMD_USER_ACK_INFO		MAX_NUM_TXCMD_RX_USER
#define MAX_NUM_TXCMD_TF_BASIC_USER		MAX_NUM_TXCMD_RX_USER
#define MAX_NUM_TXCMD_SW_FID_INFO		16


enum txcmd_dbg_cmm_id {
	SET_TXCMD_DBG_CTRL,
	SET_TXCMD_DBG_CLEAR,
	SET_TXCMD_DBG_SXN_GLOBAL,
	SET_TXCMD_DBG_SXN_PROTECT,
	SET_TXCMD_DBG_SXN_PROTECT_RUINFO,
	SET_TXCMD_DBG_SXN_TXDATA,
	SET_TXCMD_DBG_SXN_TXDATA_USER_INFO,
	SET_TXCMD_DBG_SXN_TRIGDATA,
	SET_TXCMD_DBG_SXN_TRIGDATA_USER_ACK_INFO,
	SET_TXCMD_DBG_TF_TXD,
	SET_TXCMD_DBG_TF_BASIC,
	SET_TXCMD_DBG_TF_BASIC_USER,
	SET_TXCMD_DBG_SXN_SW_FID,
	SET_TXCMD_DBG_SXN_SW_FID_INFO,
	SET_TXCMD_DBG_SW_FID_TXD,
	GET_TXCMD_DBG_STATUS,
	GET_TXCMD_DBG_SXN_GLOBAL,
	GET_TXCMD_DBG_SXN_PROTECT,
	GET_TXCMD_DBG_SXN_TXDATA,
	GET_TXCMD_DBG_SXN_TRIGDATA,
	GET_TXCMD_DBG_TF_TXD,
	GET_TXCMD_DBG_TF_BASIC,
	GET_TXCMD_DBG_SXN_SW_FID,
	GET_TXCMD_DBG_SW_FID_TXD,
};

enum section_idx {
	SXN_GLOBAL_ID,
	SXN_PROTECT_ID,
	SXN_TX_DATA_ID,
	SXN_TRIG_DATA_ID,
	TF_TXD_ID,
	TF_BASIC_ID,
	SXN_SW_FID_ID,
	SW_FID_TXD_ID,
};

struct TXCMD_SXN_GLOBAL {
	UINT_32 txByteCnt       : 16,   /* DW0 */
			txdNum          : 4,
			rsv1            : 3,
			pktFt           : 2,
			qid             : 7;
	UINT_32 sxnCnt          : 5,    /* DW1 */
			txcmdType       : 4,
			spl             : 1,
			presentSpTblIdx : 4,
			tv              : 1,
			band            : 1,
			serialId        : 8,
			preload         : 1,
			txop            : 1,
			ownMac          : 6;
	UINT_32 aggPolicy       : 3,    /* DW2 */
			bandwidth       : 3,
			ignoreBw        : 1,
			smartAnt        : 1,
			antId           : 24;
	UINT_32 speIdx          : 5,    /* DW3 */
			bs              : 1,
			rsv2            : 26;
} __attribute__((__packed__));

struct TXCMD_RU_INFO {
	UINT_16 aid             : 12,
		ruAlloc         : 4;
} __attribute__((__packed__));

struct TXCMD_SXN_PROTECT {
	UINT_32 sxnDDwCnt       : 8,    /* DW0 */
			sxnIdx          : 5,
			sxnType         : 3,
			rsv1            : 2,
			protect         : 2,
			staCnt          : 6,
			cascadeIdx      : 1,
			csRequired      : 1,
			rsv2            : 2,
			tfPad           : 2;
	UINT_32 rate            : 6,    /* DW1 */
			txMode          : 4,
			nsts            : 3,
			coding          : 1,
			doppler         : 1,
			rsv3            : 17;
	struct TXCMD_RU_INFO ruInfo[MAX_NUM_TXCMD_RU_INFO];   /* FIXME *//* DW2~17 */
} __attribute__((__packed__));

struct TXCMD_USER_INFO {
	UINT_32 rsv1            : 11,   /* DW0 */
			txPowerAlpha    : 9,
			coding          : 1,
			multiTid        : 1,
			wlanId          : 10;
	UINT_32 muMimoGroup     : 5,    /* DW1 */
			muMimoSpatial   : 4,
			startStream     : 3,
			ruAllocBn       : 1,
			ruAlloc         : 7,
			ackGroup        : 5,
			suBar           : 1,
			muBar           : 1,
			cbSta           : 1,
			aggOld          : 1,
			preload         : 1,
			rsv2            : 2;
	UINT_32 rate            : 6,    /* DW2 */
			nsts            : 3,
			contentCh       : 1,
			ackPol          : 2,
			lpCtrl          : 4,
			srRate          : 16;
	UINT_32 ruRatio         : 12,   /* DW3 */
			acSeq           : 4,
			acNum           : 2,
			barRuRatio      : 12,
			splPrimaryUser  : 1,
			bf              : 1;
	UINT_32 ac0Ratio        : 8,    /* DW4 */
			ac1Ratio        : 8,
			ac2Ratio        : 8,
			ac3Ratio        : 8;
	UINT_32 barRate         : 6,    /* DW5 */
			barMode         : 4,
			barNsts         : 3,
			rsv5            : 3,
			baType          : 4,
			LSigLen         : 12;
	UINT_32 csRequired      : 1,    /* DW6 */
			bw              : 2,
			ltfType         : 2,
			ltfSym          : 3,
			stbc            : 1,
			ldpcExtraSym    : 1,
			pktExt          : 3,
			spatialReuse    : 16,
			coding2         : 1,
			dcm             : 1,
			barAckPol       : 1;
	UINT_32 ackRuAllocBn    : 1,    /* DW7 */
			ackRuAlloc      : 7,
			ackMcs          : 4,
			ssAlloc         : 6,
			targetRssi      : 7,
			doppler         : 1,
			rsv6            : 2,
			tidInfo         : 4;
} __attribute__((__packed__));

struct TXCMD_SXN_TX_DATA {
	UINT_32 sxnDDwCnt           : 8,    /* DW0 */
			sxnIdx              : 5,
			sxnType             : 3,
			rxv                 : 2,
			rsp                 : 1,
			psIgnore            : 1,
			sigBCh1StaCnt       : 6,
			sigBCh2StaCnt       : 6;
	UINT_32 staCnt              : 6,    /* DW1 */
			ra                  : 1,
			rsv1                : 1,
			sigBSym             : 7,
			sigBMcs             : 3,
			sigBDcm             : 1,
			sigBCompress        : 1,
			muMimoUsr           : 4,
			ltfSym              : 3,
			gi                  : 2,
			stbc                : 1,
			doppler             : 1,
			cmdPower            : 1;
	UINT_8 ruAlloc[8];              /* DW2,3 */
	UINT_32 primaryUserIdx      : 6,/* DW4 */
			muPpduDur           : 14,
			txPower             : 8,
			ltf                 : 2,
			tfPad               : 2;
	UINT_32 mu0UserPosition     : 2,/* DW5 */
			mu1UserPosition     : 2,
			mu2UserPosition     : 2,
			mu3UserPosition     : 2,
			muGroupId           : 6,
			ru26dSigBCh1        : 1,
			ru26uSigBCh2        : 1,
			preamblePuncture    : 8,
			txMode              : 4,
			dynamicBw           : 1,
			mtb                 : 1,
			rsv3                : 2;
	UINT_32 protectionDuration  : 10, /* DW6 */
			responseDuration    : 10,
			rsv4                : 12;
	UINT_32 rsv5                : 32; /* DW7 */
	struct TXCMD_USER_INFO txcmdUser[MAX_NUM_TXCMD_USER_INFO];  /* DW8~ */
} __attribute__((__packed__));

struct TXCMD_USER_ACK_INFO {
	UINT_32 staId           : 11,  /* DW0 */
			ackTxPowerAlpha : 9,
			coding          : 1,
			contentCh       : 1,
			wlanId          : 10;
	UINT_32 ruAllocBn       : 1,   /* DW1 */
			ruAlloc         : 7,
			rate            : 6,
			nsts            : 3,
			ruAllNss        : 3,
			ruRatio         : 12;
	UINT_32 sfEnable        : 1,
			ac              : 2,
			splPrimaryUser  : 1,
			rsv1            : 28;
	UINT_32 rsv2;
} __attribute__((__packed__));

struct TXCMD_SXN_TRIG_DATA {
	UINT_32 sxnDDwCnt           : 8,    /* DW0 */
			sxnIdx              : 5,
			sxnType             : 3,
			rxv                 : 2,
			staCnt              : 6,
			baPol               : 2,
			priOrder            : 1,
			splAc               : 4,
			rsv1                : 1;
	UINT_32 tfFID               : 12,   /* DW1 */
			fidType             : 5,
			tfPad               : 2,
			lSigLen             : 12,
			rsv2                : 1;
	UINT_32 rxHetbCfg1;                 /* DW2 */
	UINT_32 rxHetbCfg2;                 /* DW3 */
	UINT_32 sigBCh1StaCnt       : 6,    /* DW4 */
			rsv3                : 2,
			sigBSym             : 7,
			sigBMcs             : 3,
			sigBDcm             : 1,
			sigBCompress        : 1,
			muMimoUsr           : 4,
			ltfSym              : 3,
			gi                  : 2,
			stbc                : 1,
			doppler             : 1,
			cmdPower            : 1;
	UINT_8 ruAlloc[8];                  /* DW5,6 */
	UINT_32 sigBCh2StaCnt       : 6,    /* DW7 */
			muPpduDur           : 14,
			ackTxPower          : 8,
			ltf                 : 2,
			rsv5                : 2;
	UINT_32 rsv6                : 14,   /* DW9 */
			ru26dSigBCh1        : 1,
			ru26uSigBCh2        : 1,
			preambPunc          : 8,
			ackTxMode           : 4,
			rsv7                : 4;
	UINT_32 ssnUser             : 6,   /* DW7 */
			rsv8                : 26;
	struct TXCMD_USER_ACK_INFO txcmdUserAck[MAX_NUM_TXCMD_USER_ACK_INFO];
} __attribute__((__packed__));

struct TXCMD_SW_FID_INFO {
	UINT_32 fid             : 12,   /* DW0 */
			fidType         : 5,
			tfPad           : 2,
			rsv1            : 1,
			lSigLen         : 12;
	UINT_32 staCnt          : 8,    /* DW1 */
			rsv2            : 24;
} __attribute__((__packed__));

struct TXCMD_SXN_SW_FID {
	UINT_32 sxnDDwCnt       : 8,    /* DW0 */
			sxnIdx          : 5,
			sxnType         : 3,
			rxv             : 2,
			fidCnt          : 6,
			rsv1            : 8;
	UINT_32 rsv2;                   /* DW1 */
	struct TXCMD_SW_FID_INFO swFidInfo[MAX_NUM_TXCMD_SW_FID_INFO]; /* DW2~33*/
} __attribute__((__packed__));

struct TXCMD_TXD_PTR_LEN_T {
	UINT_32 txpAddr0L;          /* DW0 */
	UINT_32 txpLen0     : 12,   /* DW1 */
		txpAddr0H   : 2,
		txpSrc0     : 1,
		mpduLast0   : 1,
		txpLen1     : 12,
		txpAddr1H   : 2,
		txpSrc1     : 1,
		mpduLast1   : 1;
	UINT_32 txpAddr1L;          /* DW2 */
} __attribute__((__packed__));

struct TXCMD_TXD {
	UINT_32 txByteCnt   : 16,  /* DW0 */
		ethOffset       : 7,
		pktFt           : 2,
		qid             : 7;
	UINT_32 wlanId      : 10,  /* DW1 */
		VTA             : 1,
		headerInfo      : 5,
		headerFormat    : 2,
		headerPadding   : 2,
		tid             : 3,
		amsduC          : 1,
		ownMac          : 6,
		tgId            : 1,
		ft              : 1;
	UINT_32 subType     : 4,   /* DW2 */
		type            : 2,
		ndp             : 1,
		ndpa            : 1,
		sd              : 1,
		rts             : 1,
		bm              : 1,
		bip             : 1,
		du              : 1,
		he              : 1,
		frag            : 2,
		txTime          : 8,
		powerOffset     : 6,
		frm             : 1,   /* Fixed Rate Mode */
		fr              : 1;   /* Fixed Rate */
	UINT_32 na              : 1,    /* No Ack */ /* DW3 */
		pf              : 1,   /* Protect frame field*/
		emrd            : 1,   /* Extend more data for cut-through */
		eeosp           : 1,   /* Extend end of service period for cut-through */
		das             : 1,   /* DA source selection */
		tm              : 1,   /* Timing measurement */
		txCnt           : 5,   /* TX count */
		remainTxCnt     : 5,   /* Remaining TX count */
		sn              : 12,  /* Sequence number */
		baDis           : 1,   /* Packet can NOT be aggregated to AMPDU */
		pm              : 1,   /* Power management */
		pnVld           : 1,   /* PN in TXD is valid */
		snVld           : 1;   /* SN in TXD is valid */
	UINT_32 pn;                    /* Packet number */ /* DW4 */
	UINT_32 pid             : 8,    /* Packet ID */ /* DW5 */
		txsFm           : 1,   /* TX status format */
		txs2m           : 1,   /* TX status to MCU */
		txs2h           : 1,   /* TX statuc to host */
		rsv1            : 3,
		addba           : 1,   /* ADDBA */
		md              : 1,
		pn2             : 16;  /* FIXME */
	UINT_32 bw              : 3,    /* Fixed bandwidth mode */ /* DW6 */
		dynBw           : 1,   /* Dynamic bandwidth */
		antId           : 4,  /* Smart antenna index */
		rsv2            : 2,
		speIdxSel       : 1,
		ldpc            : 1,
		heLtf           : 2,
		gi              : 2,
		fixRate         : 14,  /* Rate to be fixed */
		txExpBf         : 1,   /* Use explicit beamforming matrix to transmit packet */
		txImpBf         : 1;   /* Use implicit beamforming matrix to transmit packet */
	UINT_32 swTxTime        : 8,   /* DW7 */
		rsv4            : 2,
		hwAmsdu         : 1,
		speIdx          : 5,
		subType2        : 4,
		type2           : 2,
		ctxdCnt         : 4,
		ctxd            : 1,
		rsv5            : 1,
		ipCso           : 1,   /* HW IP checksum offload */
		utCso           : 1,   /* UDP/TCP checksum offload */
		txdLen          : 2;
	UINT_32 msduId0         : 15,  /* DW8 */
		msduId0Vld      : 1,
		msduId1         : 15,
		msduId1Vld      : 1;
	UINT_32 msduId2         : 15,  /* DW9 */
		msduId2Vld      : 1,
		msduId3         : 15,
		msduId3Vld      : 1;

	/* DW10 ~ DW15 */
	struct TXCMD_TXD_PTR_LEN_T arPtrLen[2];
} __attribute__((__packed__));

struct TF_BASIC_USER {
	UINT_32 aid12           : 12,
		ruAllocBn       : 1,
		ruAlloc         : 7,
		codingType      : 1,
		mcs             : 4,
		dcm             : 1,
		ssAlloc         : 6;
	UINT_8  targetRssi      : 7,
		rsv1            : 1;
	UINT_8  mpduMuSpacing   : 2,
		tidAggrLimit    : 3,
		rsv2            : 1,
		preferredAc     : 2;
} __attribute__((__packed__));

struct TF_BASIC {
	UINT_16 frameCtrl;
	UINT_16 duration;
	UINT_8 ra[MAC_ADDR_LEN];
	UINT_8 ta[MAC_ADDR_LEN];
	UINT_32 triggerType     : 4,
		length          : 12,
		cascade         : 1,
		csRequired      : 1,
		bw              : 2,
		giLtfType       : 2,
		muMimoLtfMode   : 1,
		heLtfSymbols    : 3,
		stbc            : 1,
		ldpcExtra       : 1,
		apTxPowerLow    : 4;
	UINT_32 apTxPowerHigh   : 2,
		pktExt          : 3,
		spatialReuse    : 16,
		doppler         : 1,
		heSigARsv       : 9,
		rsv             : 1;
	struct TF_BASIC_USER arBasicUsr[MAX_NUM_TXCMD_TF_BASIC_USER];
} __attribute__((__packed__));

struct TXCMD_DBG_CMD_STATUS {
	UINT32  enable : 1,
		muru_algo_disable : 1,
		ra_algo_enable : 1,
		dl_ul_tp: 1,
		send_trigger: 1,
		rsv : 27;
	UINT32  spl_count;
	UINT32  txcmd_tx_count;
	UINT32  txcmd_protect_count;
	UINT32  txcmd_txdata_count;
	UINT32  txcmd_trigdata_count;
	UINT32  txcmd_swfid_count;
	UINT32  txdata_cmdrpt_count;
	UINT32  trigdata_cmdrpt_count;
	UINT32  rxrpt_count;
};

struct EXT_CMD_TXCMD_DBG_CTRL {
	UINT32 cmd_id: 6,
	       data_len: 16,
	       index: 7,
	       txcmd_entry_idx: 3;
};

union txcmd_sxn_all {
	struct TXCMD_SXN_GLOBAL *global;
	struct TXCMD_SXN_PROTECT *protect;
	struct TXCMD_RU_INFO *ru_info;
	struct TXCMD_SXN_TX_DATA *tx_data;
	struct TXCMD_USER_INFO *tx_data_user;
	struct TXCMD_SXN_TRIG_DATA *trig_data;
	struct TXCMD_USER_ACK_INFO *trig_data_user;
	struct TXCMD_TXD *tf_txd;
	struct TF_BASIC *tf_basic;
	struct TF_BASIC_USER *tf_basic_user;
	struct TXCMD_SXN_SW_FID *sw_fid;
	struct TXCMD_SW_FID_INFO *sw_fid_info;
	struct TXCMD_TXD *sw_fid_txd;
};

INT set_txcmd_dbg_enable(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_dbg_clear(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_dbg_muru_algo_disable(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_dbg_ra_algo_enable(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_dbg_dl_ul_tp(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_dbg_send_trig(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_sxn_global(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_sxn_protect(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_sxn_protect_ruinfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_sxn_txdata(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_sxn_txdata_rualloc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_sxn_txdata_userinfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_sxn_trigdata(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_sxn_trigdata_rualloc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_sxn_trigdata_user_ackinfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_tf_txd(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_tf_basic(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_tf_basic_user(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_sxn_dw(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_txcmd_sxn_user_dw(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT show_txcmd_dbg_status(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_sxn_global(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_sxn_protect(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_sxn_txdata(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_sxn_trigdata(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_tf_txd(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_tf_basic(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_sw_fid(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_sw_fid_txd(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#endif /* CFG_SUPPORT_FALCON_TXCMD_DBG */

#endif /* __HE_CFG_H__ */
