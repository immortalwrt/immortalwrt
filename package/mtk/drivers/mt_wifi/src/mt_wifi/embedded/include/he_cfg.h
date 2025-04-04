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
	SET_TXCMD_DBG_SOP,
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
#ifdef RT_BIG_ENDIAN
	UINT_32 qid 			: 7,	/* DW0 */
			pktFt			: 2,
			rsv1			: 3,
			txdNum			: 4,
			txByteCnt		: 16;
	UINT_32 ownMac			: 6,	/* DW1 */
			txop			: 1,
			preload 		: 1,
			serialId		: 8,
			band			: 1,
			tv				: 1,
			presentSpTblIdx : 4,
			spl 			: 1,
			txcmdType		: 4,
			sxnCnt			: 5;
	UINT_32 antId			: 24,	 /* DW2 */
			smartAnt		: 1,
			ignoreBw		: 1,
			bandwidth		: 3,
			aggPolicy		: 3;
	UINT_32 rsv2			: 26,	 /* DW3 */
			bs				: 1,
			speIdx			: 5;
#else
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
#endif /* RT_BIG_ENDIAN */
} __attribute__((__packed__));

struct TXCMD_RU_INFO {
#ifdef RT_BIG_ENDIAN
	UINT_16 ruAlloc 	: 4,
			aid 		: 12;
#else
	UINT_16 aid         : 12,
			ruAlloc     : 4;
#endif /* RT_BIG_ENDIAN */
} __attribute__((__packed__));

struct TXCMD_SXN_PROTECT {
#ifdef RT_BIG_ENDIAN
	UINT_32 tfPad			: 2,	/* DW0 */
			rsv2			: 2,
			csRequired		: 1,
			cascadeIdx		: 1,
			staCnt			: 6,
			protect 		: 2,
			rsv1			: 2,
			sxnType 		: 3,
			sxnIdx			: 5,
			sxnDDwCnt		: 8;
	UINT_32 rsv3			: 17,	 /* DW1 */
			doppler 		: 1,
			nsts			: 3,
			txMode			: 4,
			txMode			: 4,
			rate			: 6;
#else
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
#endif /* RT_BIG_ENDIAN */
	struct TXCMD_RU_INFO ruInfo[MAX_NUM_TXCMD_RU_INFO];   /* FIXME *//* DW2~17 */
} __attribute__((__packed__));

struct TXCMD_USER_INFO {
#ifdef RT_BIG_ENDIAN
	UINT_32 wlanId          : 10,   /* DW0 */
			multiTid        : 1,
			coding          : 1,
			txPowerAlpha    : 9,
			rsv1            : 11;
	UINT_32 rsv2            : 2,    /* DW1 */
			preload         : 1,
			aggOld          : 1,
			cbSta           : 1,
			muBar           : 1,
			suBar           : 1,
			ackGroup        : 5,
			ruAlloc         : 7,
			ruAllocBn       : 1,
			startStream     : 3,
			muMimoSpatial   : 4,
			muMimoGroup     : 5;
	UINT_32 srRate          : 16,    /* DW2 */
			lpCtrl          : 4,
			ackPol          : 2,
			contentCh       : 1,
			nsts            : 3,
			rate            : 6;
	UINT_32 bf              : 1,   /* DW3 */
			splPrimaryUser  : 1,
			barRuRatio      : 12,
			acNum           : 2,
			acSeq           : 4,
			ruRatio         : 12;
	UINT_32 ac3Ratio        : 8,    /* DW4 */
			ac2Ratio        : 8,
			ac1Ratio        : 8,
			ac0Ratio        : 8;
	UINT_32 LSigLen         : 12,    /* DW5 */
			baType          : 4,
			rsv5            : 3,
			barNsts         : 3,
			barMode         : 4,
			barRate         : 6;
	UINT_32 barAckPol       : 1,    /* DW6 */
			dcm             : 1,
			coding2         : 1,
			spatialReuse    : 16,
			pktExt          : 3,
			ldpcExtraSym    : 1,
			stbc            : 1,
			ltfSym          : 3,
			ltfType         : 2,
			bw              : 2,
			csRequired      : 1;
	UINT_32 tidInfo         : 4,    /* DW7 */
			rsv6            : 2,
			doppler         : 1,
			targetRssi      : 7,
			ssAlloc         : 6,
			ackMcs          : 4,
			ackRuAlloc      : 7,
			ackRuAllocBn    : 1;
#else
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
#endif /* RT_BIG_ENDIAN */
} __attribute__((__packed__));

struct TXCMD_SXN_TX_DATA {
#ifdef RT_BIG_ENDIAN
	UINT_32 sigBCh2StaCnt       : 6,    /* DW0 */
			sigBCh1StaCnt       : 6,
			psIgnore            : 1,
			rsp                 : 1,
			rxv                 : 2,
			sxnType             : 3,
			sxnIdx              : 5,
			sxnDDwCnt           : 8;
	UINT_32 cmdPower            : 1,    /* DW1 */
			doppler             : 1,
			stbc                : 1,
			gi                  : 2,
			ltfSym              : 3,
			muMimoUsr           : 4,
			sigBCompress        : 1,
			sigBDcm             : 1,
			sigBMcs             : 3,
			sigBSym             : 7,
			rsv1                : 1,
			ra                  : 1,
			staCnt              : 6;
	UINT_8 ruAlloc[8];              /* DW2,3 */
	UINT_32 tfPad               : 2,/* DW4 */
			ltf                 : 2,
			txPower             : 8,
			muPpduDur           : 14,
			primaryUserIdx      : 6;
	UINT_32 rsv3                : 2,/* DW5 */
			mtb                 : 1,
			dynamicBw           : 1,
			txMode              : 4,
			preamblePuncture    : 8,
			ru26uSigBCh2        : 1,
			ru26dSigBCh1        : 1,
			muGroupId           : 6,
			mu3UserPosition     : 2,
			mu2UserPosition     : 2,
			mu1UserPosition     : 2,
			mu0UserPosition     : 2;
	UINT_32 rsv4                : 12, /* DW6 */
			responseDuration    : 10,
			protectionDuration  : 10;
	UINT_32 rsv5                : 32; /* DW7 */
#else
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
#endif /* RT_BIG_ENDIAN */
	struct TXCMD_USER_INFO txcmdUser[MAX_NUM_TXCMD_USER_INFO];  /* DW8~ */
} __attribute__((__packed__));

struct TXCMD_USER_ACK_INFO {
#ifdef RT_BIG_ENDIAN
	UINT_32 wlanId          : 10,  /* DW0 */
			contentCh       : 1,
			coding          : 1,
			ackTxPowerAlpha : 9,
			staId           : 11;
	UINT_32 ruRatio         : 12,   /* DW1 */
			ruAllNss        : 3,
			nsts            : 3,
			rate            : 6,
			ruAlloc         : 7,
			ruAllocBn       : 1;
	UINT_32 rsv1            : 28,
			splPrimaryUser  : 1,
			ac              : 2,
			sfEnable        : 1;
	UINT_32 rsv2;
#else
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
#endif /* RT_BIG_ENDIAN */
} __attribute__((__packed__));

struct TXCMD_SXN_TRIG_DATA {
#ifdef RT_BIG_ENDIAN
	UINT_32 rsv1                : 1,	/* DW0 */
			splAc               : 4,
			priOrder            : 1,
			baPol               : 2,
			staCnt              : 6,
			rxv                 : 2,
			sxnType             : 3,
			sxnIdx              : 5,
			sxnDDwCnt           : 8;
	UINT_32 rsv2                : 1,	/* DW1 */
			lSigLen             : 12,
			tfPad               : 2,
			fidType             : 5,
			tfFID               : 12;
	UINT_32 rxHetbCfg1;                 /* DW2 */
	UINT_32 rxHetbCfg2;                 /* DW3 */
	UINT_32 cmdPower            : 1,    /* DW4 */
			doppler             : 1,
			stbc                : 1,
			gi                  : 2,
			ltfSym              : 3,
			muMimoUsr           : 4,
			sigBCompress        : 1,
			sigBDcm             : 1,
			sigBMcs             : 3,
			sigBSym             : 7,
			rsv3                : 2,
			sigBCh1StaCnt       : 6;
	UINT_8 ruAlloc[8];                  /* DW5,6 */
	UINT_32 rsv5                : 2,    /* DW7 */
			ltf                 : 2,
			ackTxPower          : 8,
			muPpduDur           : 14,
			sigBCh2StaCnt       : 6;
	UINT_32 rsv7                : 4,   /* DW9 */
			ackTxMode           : 4,
			preambPunc          : 8,
			ru26uSigBCh2        : 1,
			ru26dSigBCh1        : 1,
			rsv6                : 14;
	UINT_32 rsv8                : 26,   /* DW7 */
			ssnUser             : 6;
#else
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
#endif /* RT_BIG_ENDIAN */
	struct TXCMD_USER_ACK_INFO txcmdUserAck[MAX_NUM_TXCMD_USER_ACK_INFO];
} __attribute__((__packed__));

struct TXCMD_SW_FID_INFO {
#ifdef RT_BIG_ENDIAN
	UINT_32 lSigLen         : 12,   /* DW0 */
			rsv1            : 1,
			tfPad           : 2,
			fidType         : 5,
			fid             : 12;
	UINT_32 rsv2            : 24,    /* DW1 */
			staCnt          : 8;
#else
	UINT_32 fid             : 12,   /* DW0 */
			fidType         : 5,
			tfPad           : 2,
			rsv1            : 1,
			lSigLen         : 12;
	UINT_32 staCnt          : 8,    /* DW1 */
			rsv2            : 24;
#endif /* RT_BIG_ENDIAN */
} __attribute__((__packed__));

struct TXCMD_SXN_SW_FID {
#ifdef RT_BIG_ENDIAN
	UINT_32 rsv1            : 8,	/* DW0 */
			fidCnt          : 6,
			rxv             : 2,
			sxnType         : 3,
			sxnIdx          : 5,
			sxnDDwCnt       : 8;
	UINT_32 rsv2;					/* DW1 */
#else
	UINT_32 sxnDDwCnt       : 8,    /* DW0 */
			sxnIdx          : 5,
			sxnType         : 3,
			rxv             : 2,
			fidCnt          : 6,
			rsv1            : 8;
	UINT_32 rsv2;                   /* DW1 */
#endif /* RT_BIG_ENDIAN */
	struct TXCMD_SW_FID_INFO swFidInfo[MAX_NUM_TXCMD_SW_FID_INFO]; /* DW2~33*/
} __attribute__((__packed__));

struct TXCMD_TXD_PTR_LEN_T {
	UINT_32 txpAddr0L;          /* DW0 */
#ifdef RT_BIG_ENDIAN
	UINT_32 mpduLast1   : 1,	/* DW1 */
			txpSrc1     : 1,
			txpAddr1H   : 2,
			txpLen1     : 12,
			mpduLast0   : 1,
			txpSrc0     : 1,
			txpAddr0H   : 2,
			txpLen0     : 12;
#else
	UINT_32 txpLen0     : 12,   /* DW1 */
			txpAddr0H   : 2,
			txpSrc0     : 1,
			mpduLast0   : 1,
			txpLen1     : 12,
			txpAddr1H   : 2,
			txpSrc1     : 1,
			mpduLast1   : 1;
#endif /* RT_BIG_ENDIAN */
	UINT_32 txpAddr1L;          /* DW2 */
} __attribute__((__packed__));

struct TXCMD_TXD {
#ifdef RT_BIG_ENDIAN
	UINT_32 qid             : 7,  /* DW0 */
			pktFt           : 2,
			ethOffset       : 7,
			txByteCnt   	: 16;
	UINT_32 ft              : 1,  /* DW1 */
			tgId            : 1,
			ownMac          : 6,
			amsduC          : 1,
			tid             : 3,
			headerPadding   : 2,
			headerFormat    : 2,
			headerInfo      : 5,
			VTA             : 1,
			wlanId      	: 10;
	UINT_32 fr              : 1,   /* DW2 */ /* Fixed Rate */
			frm             : 1,   /* Fixed Rate Mode */
			powerOffset     : 6,
			txTime          : 8,
			frag            : 2,
			he              : 1,
			du              : 1,
			bip             : 1,
			bm              : 1,
			rts             : 1,
			sd              : 1,
			ndpa            : 1,
			ndp             : 1,
			type            : 2,
			subType     	: 4;
	UINT_32 snVld           : 1,   /* DW3 */ /* SN in TXD is valid */
			pnVld           : 1,   /* PN in TXD is valid */
			pm              : 1,   /* Power management */
			baDis           : 1,   /* Packet can NOT be aggregated to AMPDU */
			sn              : 12,  /* Sequence number */
			remainTxCnt     : 5,   /* Remaining TX count */
			txCnt           : 5,   /* TX count */
			tm              : 1,   /* Timing measurement */
			das             : 1,   /* DA source selection */
			eeosp           : 1,   /* Extend end of service period for cut-through */
			emrd            : 1,   /* Extend more data for cut-through */
			pf              : 1,   /* Protect frame field*/
			na              : 1;   /* No Ack */
	UINT_32 pn;                    /* Packet number */ /* DW4 */
	UINT_32 pn2             : 16,   /* DW5 */ /* FIXME */
			md              : 1,
			addba           : 1,   /* ADDBA */
			rsv1            : 3,
			txs2h           : 1,   /* TX statuc to host */
			txs2m           : 1,   /* TX status to MCU */
			txsFm           : 1,   /* TX status format */
			pid             : 8;   /* Packet ID */
	UINT_32 txImpBf         : 1,   /* DW6 */ /* Use implicit beamforming matrix to transmit packet */
			txExpBf         : 1,   /* Use explicit beamforming matrix to transmit packet */
			fixRate         : 14,  /* Rate to be fixed */
			gi              : 2,
			heLtf           : 2,
			ldpc            : 1,
			speIdxSel       : 1,
			rsv2            : 2,
			antId           : 4,   /* Smart antenna index */
			dynBw           : 1,   /* Dynamic bandwidth */
			bw              : 3;   /* Fixed bandwidth mode */
	UINT_32 txdLen          : 2,   /* DW7 */
			utCso           : 1,   /* UDP/TCP checksum offload */
			ipCso           : 1,   /* HW IP checksum offload */
			rsv5            : 1,
			ctxd            : 1,
			ctxdCnt         : 4,
			type2           : 2,
			subType2        : 4,
			speIdx          : 5,
			hwAmsdu         : 1,
			rsv4            : 2,
			swTxTime        : 8;
	UINT_32 msduId1Vld      : 1,  /* DW8 */
			msduId1         : 15,
			msduId0Vld      : 1,
			msduId0         : 15;
	UINT_32 msduId3Vld      : 1,  /* DW9 */
			msduId3         : 15,
			msduId2Vld      : 1,
			msduId2         : 15;
#else
	UINT_32 txByteCnt   	: 16,  /* DW0 */
			ethOffset       : 7,
			pktFt           : 2,
			qid             : 7;
	UINT_32 wlanId      	: 10,  /* DW1 */
			VTA             : 1,
			headerInfo      : 5,
			headerFormat    : 2,
			headerPadding   : 2,
			tid             : 3,
			amsduC          : 1,
			ownMac          : 6,
			tgId            : 1,
			ft              : 1;
	UINT_32 subType     	: 4,   /* DW2 */
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
#endif /* RT_BIG_ENDIAN */
	/* DW10 ~ DW15 */
	struct TXCMD_TXD_PTR_LEN_T arPtrLen[2];
} __attribute__((__packed__));

struct TF_BASIC_USER {
#ifdef RT_BIG_ENDIAN
	UINT_32 ssAlloc         : 6,
			dcm             : 1,
			mcs             : 4,
			codingType      : 1,
			ruAlloc         : 7,
			ruAllocBn       : 1,
			aid12           : 12;
	UINT_8  rsv1            : 1,
			targetRssi      : 7;
	UINT_8  preferredAc     : 2,
			rsv2            : 1,
			tidAggrLimit    : 3,
			mpduMuSpacing   : 2;
#else
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
#endif /* RT_BIG_ENDIAN */
} __attribute__((__packed__));

struct TF_BASIC {
	UINT_16 frameCtrl;
	UINT_16 duration;
	UINT_8 ra[MAC_ADDR_LEN];
	UINT_8 ta[MAC_ADDR_LEN];
#ifdef RT_BIG_ENDIAN
	UINT_32 apTxPowerLow    : 4,
			ldpcExtra       : 1,
			stbc            : 1,
			heLtfSymbols    : 3,
			muMimoLtfMode   : 1,
			giLtfType       : 2,
			bw              : 2,
			csRequired      : 1,
			cascade         : 1,
			length          : 12,
			triggerType     : 4;
	UINT_32 rsv             : 1,
			heSigARsv       : 9,
			doppler         : 1,
			spatialReuse    : 16,
			pktExt          : 3,
			apTxPowerHigh   : 2;
#else
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
#endif /* RT_BIG_ENDIAN */
	struct TF_BASIC_USER arBasicUsr[MAX_NUM_TXCMD_TF_BASIC_USER];
} __attribute__((__packed__));

struct TXCMD_DBG_CMD_STATUS {
#ifdef RT_BIG_ENDIAN
	UINT32	rsv: 27,
			send_trigger: 1,
			dl_ul_tp: 1,
			ra_algo_enable: 1,
			muru_algo_disable: 1,
			enable: 1;
#else
	UINT32	enable: 1,
			muru_algo_disable: 1,
			ra_algo_enable: 1,
			dl_ul_tp: 1,
			send_trigger: 1,
			rsv: 27;
#endif /* RT_BIG_ENDIAN */
	UINT32  spl_count;
	UINT32  txcmd_tx_count;
	UINT32  txcmd_protect_count;
	UINT32  txcmd_txdata_count;
	UINT32  txcmd_trigdata_count;
	UINT32  txcmd_swfid_count;
	UINT32  txdata_cmdrpt_count;
	UINT32  trigdata_cmdrpt_count;
	UINT32  rxrpt_count;
} __attribute__((__packed__));

typedef struct _TXCMD_DEBUG_SOP_CMD_T {
    UINT_16 u2AhDbgWcid;
    UINT_8 ucAhDbgAcQue;
    UINT_8 ucAhDbgDvtTestCase;
} TXCMD_DEBUG_SOP_CMD_T, *P_TXCMD_DEBUG_SOP_CMD_T;

struct EXT_CMD_TXCMD_DBG_CTRL {
#ifdef RT_BIG_ENDIAN
	UINT32 txcmd_entry_idx: 3,
		   index: 7,
		   data_len: 16,
		   cmd_id: 6;
#else
	UINT32 cmd_id: 6,
	       data_len: 16,
	       index: 7,
	       txcmd_entry_idx: 3;
#endif /* RT_BIG_ENDIAN */
} __attribute__((__packed__));

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
INT set_txcmd_dbg_sop(RTMP_ADAPTER *pAd, RTMP_STRING *arg);


INT show_txcmd_dbg_status(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_sxn_global(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_sxn_protect(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_sxn_txdata(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_sxn_trigdata(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_tf_txd(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_tf_basic(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_sw_fid(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT show_txcmd_sw_fid_txd(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef WIFI_UNIFIED_COMMAND
VOID UniEventTxCmdShow(struct _UNI_EVENT_GET_TXCMD_DBG_CMD_CTRL_T *TlvData);
INT UniCmdSendTxCmdDbg(RTMP_ADAPTER *pAd, UINT8 *cmd_data, UINT8 *rsp_payload);
#endif /* WIFI_UNIFIED_COMMAND */


#endif /* CFG_SUPPORT_FALCON_TXCMD_DBG */

#endif /* __HE_CFG_H__ */
