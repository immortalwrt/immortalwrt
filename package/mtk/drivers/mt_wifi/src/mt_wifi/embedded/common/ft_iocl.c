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

	All related IEEE802.11r IOCTL function body.

***************************************************************************/

#ifdef DOT11R_FT_SUPPORT



#include "rt_config.h"

#define TYPE_FUNC
#define FT_FUNC_SIMULATION

#ifdef FT_FUNC_SIMULATION
#endif /* FT_FUNC_SIMULATION */

/* ----- Private Variable ----- */
#ifdef FT_FUNC_SIMULATION
static UCHAR gFT_MAC_STA[ETH_ALEN] = { 0x00, 0x0e, 0x2e, 0x82, 0xe7, 0x6d };
UCHAR gFT_MAC_OldAP[ETH_ALEN] = { 0x00, 0x0e, 0x2e, 0x12, 0x34, 0x56 };

#ifdef CONFIG_STA_SUPPORT
#define FT_RIC_SIM_AP_MAX	10
static FT_RIC_STATUS gFT_RIC_RspStatus[FT_RIC_SIM_AP_MAX];
static UINT32 gFT_RIC_RspStatusIndex;
#endif /* CONFIG_STA_SUPPORT */

#endif /* FT_FUNC_SIMULATION */


/* ----- Extern Function ----- */
#ifdef CONFIG_AP_SUPPORT
extern BOOLEAN FT_KDP_R0KH_InfoAdd(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pR0KHID,
	IN	UCHAR				*pMAC,
	IN	UINT32				IP);
#endif /* CONFIG_AP_SUPPORT */


/* ----- Private Function ----- */

#ifdef CONFIG_AP_SUPPORT
#ifndef FT_KDP_FUNC_SOCK_COMM
static VOID FT_KDP_CMD_EventList(PRTMP_ADAPTER pAd, INT32 Argc, CHAR *pArgv);
#endif /* FT_KDP_FUNC_SOCK_COMM */

static VOID FT_KDP_CMD_DbgFlagCtrl(PRTMP_ADAPTER pAd, INT32 Argc, CHAR *pArgv);

#ifdef FT_FUNC_SIMULATION
static VOID FT_KDP_CMD_SimEvtFtAssoc(PRTMP_ADAPTER pAd, INT32 Argc, CHAR *pArgv);
static VOID FT_KDP_CMD_SimEvtFtReAssoc(PRTMP_ADAPTER pAd, INT32 Argc, CHAR *pArgv);
static VOID FT_KDP_CMD_SimKeyReq(PRTMP_ADAPTER pAd, INT32 Argc, CHAR *pArgv);

#ifdef FT_KDP_FUNC_R0KH_IP_RECORD
static VOID FT_KDP_CMD_SimR0KH_InfoCreate(PRTMP_ADAPTER pAd, INT32 Argc, CHAR *pArgv);
#endif /* FT_KDP_FUNC_R0KH_IP_RECORD */

static VOID FT_RIC_CMD_SimRscReqHdlTspec(PRTMP_ADAPTER pAd, INT32 Argc, CHAR *pArgv);
static VOID FT_RIC_CMD_SimRscReqHandle(PRTMP_ADAPTER pAd, INT32 Argc, CHAR *pArgv);
static VOID FT_RRB_CMD_SimSend(PRTMP_ADAPTER pAd, INT32 Argc, CHAR *pArgv);
static VOID FT_11K_CMD_SimInfoReq(PRTMP_ADAPTER pAd, INT32 Argc, CHAR *pArgv);
static VOID FT_11K_CMD_SimKeyShow(PRTMP_ADAPTER pAd, INT32 Argc, CHAR *pArgv);
#endif /* FT_FUNC_SIMULATION */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef FT_FUNC_SIMULATION
static VOID FT_RIC_CMD_StatusDisplay(FT_RIC_STATUS *pRspStatus);
static VOID FT_RIC_CMD_SimRscReqStart(PRTMP_ADAPTER pAd, INT32 Argc, CHAR *pArgv);
static VOID FT_RIC_CMD_SimRscReq(PRTMP_ADAPTER pAd, INT32 Argc, CHAR *pArgv);
static VOID FT_RIC_CMD_SimRscReqEnd(PRTMP_ADAPTER pAd, INT32 Argc, CHAR *pArgv);
static VOID FT_RIC_CMD_SimRscReqRspList(PRTMP_ADAPTER pAd, INT32 Argc, CHAR *pArgv);
#endif /* FT_FUNC_SIMULATION */
#endif /* CONFIG_STA_SUPPORT */

static UINT32 FT_CMD_UtilHexGet(CHAR **ppArgv);
static UINT32 FT_CMD_UtilNumGet(CHAR **ppArgv);
static VOID FT_CMD_UtilMacGet(CHAR **ppArgv, UCHAR *pDevMac);



#ifdef CONFIG_AP_SUPPORT
void test11r(PRTMP_ADAPTER pAd)
{
	FT_KDP_CMD_SimEvtFtAssoc(pAd, 0, NULL);
}
#endif /* CONFIG_AP_SUPPORT */

/*
========================================================================
Routine Description:
	Get argument number value.

Arguments:
	**ppArgv			- input parameters

Return Value:
	decimal number

Note:
========================================================================
*/
static UINT32 TYPE_FUNC FT_CMD_UtilHexGet(
	IN	CHAR	**ppArgv)
{
	CHAR buf[3], *pNum;
	UINT32 ID;
	UCHAR Value;

	pNum = (*ppArgv);
	buf[0] = 0x30;
	buf[1] = 0x30;
	buf[2] = 0;

	for (ID = 0; ID < sizeof(buf) - 1; ID++) {
		if ((*pNum == '_') || (*pNum == 0x00))
			break;

		/* End of if */
		pNum++;
	} /* End of for */

	if (ID == 0)
		return 0; /* argument length is too small */

	/* End of if */

	if (ID >= 2)
		memcpy(buf, (*ppArgv), 2);
	else
		buf[1] = (**ppArgv);

	/* End of if */
	(*ppArgv) += ID;

	if ((**ppArgv) == '_')
		(*ppArgv)++; /* skip _ */

	/* End of if */
	FT_ARG_ATOH(buf, &Value);
	return (UINT32)Value;
} /* End of FT_CMD_UtilHexGet */


/*
========================================================================
Routine Description:
	Get argument number value.

Arguments:
	*pArgv			- input parameters

Return Value:
	decimal number

Note:
========================================================================
*/
static UINT32 TYPE_FUNC FT_CMD_UtilNumGet(
	IN	CHAR	**ppArgv)
{
	CHAR buf[20], *pNum;
	UINT32 ID;

	pNum = (*ppArgv);

	for (ID = 0; ID < sizeof(buf) - 1; ID++) {
		if ((*pNum == '_') || (*pNum == 0x00))
			break;

		/* End of if */
		pNum++;
	} /* End of for */

	if (ID == sizeof(buf) - 1)
		return 0; /* argument length is too large */

	/* End of if */
	memcpy(buf, (*ppArgv), ID);
	buf[ID] = 0x00;
	*ppArgv += ID + 1; /* skip _ */
	return FT_ARG_ATOI(buf);
} /* End of FT_CMD_UtilNumGet */


/*
========================================================================
Routine Description:
	Get argument MAC value.

Arguments:
	**ppArgv			- input parameters
	*pDevMac			- MAC address

Return Value:
	None

Note:
========================================================================
*/
static VOID TYPE_FUNC FT_CMD_UtilMacGet(
	IN	CHAR	**ppArgv,
	IN	UCHAR	*pDevMac)
{
	CHAR Buf[3];
	CHAR *pMAC = (CHAR *)(*ppArgv);
	UINT32 ID;

	if ((pMAC[0] == '0') && (pMAC[1] == '_')) {
		*ppArgv = (&pMAC[2]);
		return;
	} /* End of if */

	memset(pDevMac, 0, ETH_ALEN);

	/* must exist 18 octets */
	for (ID = 0; ID < 18; ID += 2) {
		if ((pMAC[ID] == '_') || (pMAC[ID] == 0x00)) {
			*ppArgv = (&pMAC[ID] + 1);
			return;
		} /* End of if */
	} /* End of for */

	/* get mac */
	for (ID = 0; ID < 18; ID += 3) {
		Buf[0] = pMAC[0];
		Buf[1] = pMAC[1];
		Buf[2] = 0x00;
		FT_ARG_ATOH(Buf, pDevMac);
		pMAC += 3;
		pDevMac++;
	} /* End of for */

	*ppArgv += 17 + 1; /* skip _ */
} /* End of FT_CMD_UtilMacGet */


#ifdef CONFIG_AP_SUPPORT
#ifdef FT_KDP_FUNC_R0KH_IP_RECORD
/*
========================================================================
Routine Description:
	Display all R0KH information.

Arguments:
	pAd				- WLAN control block pointer
	*pArgv			- input parameters

Return Value:
	TRUE

Note:
========================================================================
*/
INT TYPE_FUNC FT_KDP_CMD_R0KH_InfoShow(RTMP_ADAPTER * pAd, RTMP_STRING * pArgv)
{
	FT_KDP_R0KH_INFO *pInfo;
	UINT32 IdInfo, IdArray;

	RTMP_SEM_LOCK(&(pAd->ApCfg.FtTab.FT_KdpLock));
	pInfo = FT_KDP_CB->R0KH_InfoHead;
	IdInfo = 1;

	while (pInfo != NULL) {
		MTWF_PRINT("\n%03d. R0KHID = 0x", IdInfo);

		for (IdArray = 0; IdArray < sizeof(pInfo->R0KHID); IdArray++) {
			if (IdArray == (sizeof(pInfo->R0KHID) >> 1))
				MTWF_PRINT("\n                ");

			/* End of if */
			MTWF_PRINT(" %02x", pInfo->R0KHID[IdArray]);
		} /* End of for */

		MTWF_PRINT("\n%03d. MAC = "MACSTR"\n",
				  IdInfo, MAC2STR(pInfo->MAC));
		MTWF_PRINT("%03d. IP  = %d.%d.%d.%d\n", IdInfo,
				  (pInfo->IP & 0x000000FF) >> 0,
				  (pInfo->IP & 0x0000FF00) >> 8,
				  (pInfo->IP & 0x00FF0000) >> 16,
				  (pInfo->IP & 0xFF000000) >> 24);
		pInfo = pInfo->pNext;
		IdInfo++;
	} /* End of while */

	RTMP_SEM_UNLOCK(&(pAd->ApCfg.FtTab.FT_KdpLock));
	MTWF_PRINT("\n");
	return TRUE;
} /* End of FT_KDP_CMD_R0KH_InfoShow */
#endif /* FT_KDP_FUNC_R0KH_IP_RECORD */


#ifndef FT_KDP_FUNC_SOCK_COMM
/*
========================================================================
Routine Description:
	List all queued events.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
========================================================================
*/
static VOID TYPE_FUNC FT_KDP_CMD_EventList(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
	FT_KDP_SIGNAL *pFtKdp;
	ULONG SplFlags;

	pFtKdp = (FT_KDP_SIGNAL *)FT_KDP_CB->EventList.pHead;

	if (pFtKdp == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_ERROR, "ft_kdp> No any event!\n");
		return;
	} /* End of if */

	MTWF_PRINT("\nEvent\n");
	MTWF_PRINT("----------------------------------------------------\n");
	RTMP_SEM_LOCK(&(pAd->ApCfg.FtTab.FT_KdpLock));

	while (pFtKdp != NULL) {
		switch (pFtKdp->Sig) {
		case FT_KDP_SIG_KEY_TIMEOUT:
			MTWF_PRINT("KEY TIMEOUT\n");
			break;

		case FT_KDP_SIG_KEY_REQ:
			MTWF_PRINT("KEY REQUEST\n");
			break;

		case FT_KDP_SIG_FT_ASSOCIATION:
			MTWF_PRINT("STATION FT ASSOCIATION\n");
			break;

		case FT_KDP_SIG_TERMINATE:
			MTWF_PRINT("TERMINATE\n");
			break;

		default:
			MTWF_PRINT("UNKNOWN\n");
			break;
		} /* End of switch */

		pFtKdp = (FT_KDP_SIGNAL *)pFtKdp->pNext;
	} /* End of while  */

	RTMP_SEM_UNLOCK(&(pAd->ApCfg.FtTab.FT_KdpLock));
	MTWF_PRINT("\n");
} /* End of FT_KDP_CMD_EventList */
#endif /* FT_KDP_FUNC_SOCK_COMM */


/*
========================================================================
Routine Description:
	Change the debug flag of IAPP daemon.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
========================================================================
*/
static VOID FT_KDP_CMD_DbgFlagCtrl(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
	INT32 DebugLevel;

	DebugLevel = FT_CMD_UtilNumGet(&pArgv);
	FT_KDP_EVENT_INFORM(pAd, BSS0, FT_KSP_SIG_DEBUG_TRACE,
						&DebugLevel, sizeof(DebugLevel), NULL);
} /* End of FT_KDP_CMD_DbgFlagCtrl */


#ifdef FT_FUNC_SIMULATION
/*
========================================================================
Routine Description:
	List all queued events.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
========================================================================
*/
static VOID TYPE_FUNC FT_KDP_CMD_SimEvtFtAssoc(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
	FT_KDP_EVT_ASSOC EvtAssoc;
	/* fill the station information */
	EvtAssoc.SeqNum = 0x1234;
	NdisMoveMemory(EvtAssoc.MacAddr, gFT_MAC_STA, MAC_ADDR_LEN);
	/* inform other APs a station associated to us */
	FT_KDP_EVENT_INFORM(pAd, BSS0, FT_KDP_SIG_FT_ASSOCIATION,
						&EvtAssoc, sizeof(FT_KDP_EVT_ASSOC), NULL);
} /* End of FT_KDP_CMD_SimEvtFtAssoc */


/*
========================================================================
Routine Description:
	List all queued events.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
========================================================================
*/
static VOID TYPE_FUNC FT_KDP_CMD_SimEvtFtReAssoc(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
	FT_KDP_EVT_REASSOC EvtReAssoc;
	/* fill the station information */
	EvtReAssoc.SeqNum = 0x1234;
	NdisMoveMemory(EvtReAssoc.MacAddr, gFT_MAC_STA, MAC_ADDR_LEN);
	NdisMoveMemory(EvtReAssoc.OldApMacAddr, gFT_MAC_OldAP, MAC_ADDR_LEN);
	/* inform other APs a station associated to us */
	FT_KDP_EVENT_INFORM(pAd, BSS0, FT_KDP_SIG_FT_REASSOCIATION,
						&EvtReAssoc, sizeof(FT_KDP_EVT_REASSOC), NULL);
} /* End of FT_KDP_CMD_SimEvtFtReAssoc */


/*
========================================================================
Routine Description:
	Request PMK-R1 Key from R0KH.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	1. Command Format:
		[R0KHID 1st byte (HEX)] [R1KHID 1st byte (HEX)]
========================================================================
*/
static VOID TYPE_FUNC FT_KDP_CMD_SimKeyReq(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
	FT_KDP_EVT_KEY_ELM EvtKeyReq, *pEvtKeyReq;
	UCHAR R0KHID_Byte, R1KHID_Byte;
	/* init */
	memset(&EvtKeyReq, 0, sizeof(FT_KDP_EVT_KEY_ELM));
	pEvtKeyReq = &EvtKeyReq;
	/* fill request content */
	pEvtKeyReq->ElmId = FT_KDP_ELM_ID_PRI;
	pEvtKeyReq->ElmLen = FT_KDP_ELM_PRI_LEN;
	pEvtKeyReq->OUI[0] = FT_KDP_ELM_PRI_OUI_0;
	pEvtKeyReq->OUI[1] = FT_KDP_ELM_PRI_OUI_1;
	pEvtKeyReq->OUI[2] = FT_KDP_ELM_PRI_OUI_2;
	memcpy(pEvtKeyReq->MacAddr, gFT_MAC_STA, ETH_ALEN);
	R0KHID_Byte = FT_CMD_UtilHexGet(&pArgv);
	memset(pEvtKeyReq->KeyInfo.R0KHID, R0KHID_Byte, FT_KDP_R0KHID_MAX_SIZE);
	R1KHID_Byte = FT_CMD_UtilHexGet(&pArgv);
	memset(pEvtKeyReq->KeyInfo.R1KHID, R1KHID_Byte, FT_KDP_R1KHID_MAX_SIZE);
	memcpy(pEvtKeyReq->KeyInfo.S1KHID, gFT_MAC_STA, FT_KDP_S1KHID_MAX_SIZE);
	memset(pEvtKeyReq->KeyInfo.RSV, 0x00, sizeof(pEvtKeyReq->KeyInfo.RSV));
	/* request PMK-R1 Key (our R1KH vs. the station) from the R0KH */
	FT_KDP_EVENT_INFORM(pAd, BSS0, FT_KDP_SIG_KEY_REQ,
						pEvtKeyReq, sizeof(FT_KDP_EVT_KEY_ELM), NULL);
} /* End of FT_KDP_CMD_SimKeyReq */


#ifdef FT_KDP_FUNC_R0KH_IP_RECORD
/*
========================================================================
Routine Description:
	Create a R0KH information.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	1. Command Format:
		[R0KHID 1st byte (HEX)] [IP1] [IP2] [IP3] [IP4]
========================================================================
*/
static VOID TYPE_FUNC FT_KDP_CMD_SimR0KH_InfoCreate(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
	UCHAR R0KHID[FT_KDP_R0KHID_MAX_SIZE];
	UCHAR MAC[ETH_ALEN];
	UCHAR ByteFirst;
	UCHAR IP[4];

	ByteFirst = FT_CMD_UtilHexGet(&pArgv);
	IP[0] = FT_CMD_UtilNumGet(&pArgv);
	IP[1] = FT_CMD_UtilNumGet(&pArgv);
	IP[2] = FT_CMD_UtilNumGet(&pArgv);
	IP[3] = FT_CMD_UtilNumGet(&pArgv);
	memset(R0KHID, ByteFirst, sizeof(R0KHID));
	memset(MAC, ByteFirst, sizeof(MAC));
	MAC[0] = 0x00;
	FT_KDP_R0KH_InfoAdd(pAd, R0KHID, MAC, *(UINT32 *)IP);
} /* End of FT_KDP_CMD_SimR0KH_InfoCreate */
#endif /* FT_KDP_FUNC_R0KH_IP_RECORD */


/*
========================================================================
Routine Description:
	Simulate to make a resource request.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	1. Command Format:
		[1-WME] [TID:0~7] [dir:0~3] [access:1~3] [UP:0~7] [ack:0~1]
		[nom size:byte] [inact:sec] [mean data rate:bps] [min phy rate:bps]
		[surp factor:>=1] [tclas processing:0~1]
	2. dir: 0 - uplink, 1 - dnlink, 2 - bidirectional link, 3 - direct link
		access: 1 - EDCA, 2 - HCCA, 3 - EDCA + HCCA
		ack: 0 - normal ACK, 1 - no ACK
========================================================================
*/
static VOID FT_RIC_CMD_SimRscReqHdlTspec(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
} /* End of FT_RIC_CMD_SimRscReqHdlTspec */


/*
========================================================================
Routine Description:
	Simulate to handle resource requests.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	Test example:

	iwpriv ra0 set acm=50
	iwpriv ra0 set acm=10_0_0_1_1

	1. OK case
		iwpriv ra0 set ft=512_1_7_0_1_7_0_500_3000_200000_11000000_10_0
		iwpriv ra0 set ft=512_1_6_1_1_6_0_500_3000_200000_11000000_10_0
		iwpriv ra0 set ft=512_1_5_0_1_5_0_500_3000_200000_11000000_10_0
		iwpriv ra0 set ft=512_1_4_1_1_4_0_500_3000_200000_11000000_10_0

	2. Fail case
		iwpriv ra0 set ft=512_1_7_0_1_7_0_500_3000_200000_11000000_10_0
		iwpriv ra0 set ft=512_1_5_0_1_5_0_500_3000_200000_11000000_10_0
		iwpriv ra0 set ft=512_1_6_1_1_6_0_500_3000_11000000_11000000_10_0
		iwpriv ra0 set ft=512_1_4_1_1_4_0_500_3000_11000000_11000000_10_0

	iwpriv ra0 set ft=510
========================================================================
*/
static VOID TYPE_FUNC FT_RIC_CMD_SimRscReqHandle(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
}


/*
========================================================================
Routine Description:
	Simulate to send a RRB frame.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	1. Command Format:
		[0(REQ)/1(RSP)] [PeerMAC]
========================================================================
*/
static VOID TYPE_FUNC FT_RRB_CMD_SimSend(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
	FT_KDP_EVT_ACTION ActionCB, *pActionCB;
	UCHAR MacPeer[6] = {0};

	pActionCB = &ActionCB;
	pActionCB->RequestType = FT_CMD_UtilNumGet(&pArgv);
	FT_CMD_UtilMacGet(&pArgv, MacPeer);
	memcpy(pActionCB->MacDa, MacPeer, ETH_ALEN);
	memset(pActionCB->MacSa, 0x22, ETH_ALEN);
	memset(pActionCB->MacAp, 0x33, ETH_ALEN);
	pActionCB->MacDa[0] = 0x00;
	pActionCB->MacSa[0] = 0x00;
	pActionCB->MacAp[0] = 0x00;
	FT_KDP_EventInform(pAd, BSS0, FT_KDP_SIG_ACTION,
					   "testtesttesttesttest", 19, 0, (VOID *)pActionCB);
} /* End of FT_RRB_CMD_SimSend */


/*
========================================================================
Routine Description:
	Simulate to send a 11k neighbor request.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	1. Command Format:
		PeerSSID
========================================================================
*/
static VOID TYPE_FUNC FT_11K_CMD_SimInfoReq(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
	if (strlen((RTMP_STRING *) pArgv) <= MAX_LEN_OF_SSID) {
		FT_KDP_EVENT_INFORM(pAd, BSS0, FT_KDP_SIG_AP_INFO_REQ,
							pArgv, strlen((RTMP_STRING *) pArgv), NULL);
	} /* End of if */
} /* End of FT_11K_CMD_SimInfoReq */


/*
========================================================================
Routine Description:
	Show encryption/decryption key information.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
========================================================================
*/
static VOID TYPE_FUNC FT_11K_CMD_SimKeyShow(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
	hex_dump("Key=", FT_KDP_CB->CryptKey, sizeof(FT_KDP_CB->CryptKey));
} /* End of FT_11K_CMD_SimInfoReq */
#endif /* FT_FUNC_SIMULATION */


/*
========================================================================
Routine Description:
	Display a R0KH information.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:

========================================================================
*/
static VOID TYPE_FUNC FT_R1KH_InfoShow(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
	INT i;
	INT HashIdx;
	PFT_R1HK_ENTRY pEntry;

	RTMP_SEM_LOCK(&pAd->ApCfg.FtTab.FT_R1khEntryTabLock);

	for (HashIdx = 0; HashIdx < FT_R1KH_ENTRY_HASH_TABLE_SIZE; HashIdx++) {
		pEntry = (PFT_R1HK_ENTRY)\
				 (pAd->ApCfg.FtTab.FT_R1khEntryTab[HashIdx].pHead);
		if (pEntry != NULL)
			MTWF_PRINT("\nHashIdx=%d\n", HashIdx);

		while (pEntry != NULL) {
			MTWF_PRINT("StaMac="MACSTR", ",
					 MAC2STR(pEntry->StaMac));
			MTWF_PRINT("\nKeyLifeTime=%d, ", pEntry->KeyLifeTime);
			MTWF_PRINT("\nRassocDeadline=%d\n",
					 pEntry->RassocDeadline);
			MTWF_PRINT("PairwisChipher=");

			for (i = 0; i < 4; i++)
				MTWF_PRINT("%02x:", pEntry->PairwisChipher[i]);

			MTWF_PRINT("\nAkmSuite=");

			for (i = 0; i < 4; i++)
				MTWF_PRINT("%02x:", pEntry->AkmSuite[i]);

			MTWF_PRINT("\nPmkR0Name=");

			for (i = 0; i < 16; i++)
				MTWF_PRINT("%02x:", pEntry->PmkR0Name[i]);

			MTWF_PRINT("\nPmkR1Key=");

			for (i = 0; i < 32; i++)
				MTWF_PRINT("%02x:", pEntry->PmkR1Key[i]);

			MTWF_PRINT("\nPmkR1Name=");

			for (i = 0; i < 16; i++)
				MTWF_PRINT("%02x:", pEntry->PmkR1Name[i]);
			MTWF_PRINT("\nR0KHID=");
			for (i = 0; i < pEntry->R0khIdLen; i++)
				MTWF_PRINT("%02x:", pEntry->R0khId[i]);
			MTWF_PRINT("\n");
			pEntry = pEntry->pNext;
		}
	}

	RTMP_SEM_UNLOCK(&pAd->ApCfg.FtTab.FT_R1khEntryTabLock);
} /* End of FT_R1KH_InfoShow */


static PMAC_TABLE_ENTRY FtEntry;
static VOID FT_OverDs_SimReq(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen;
	HEADER_802_11 Hdr;
	PFT_INFO FtInfoBuf;
	NDIS_STATUS NStatus;
	UCHAR StaAddr[MAC_ADDR_LEN] = {0x00, 0x0c, 0x43, 0x00, 0x00, 0x00};
	UCHAR TargetAddr[MAC_ADDR_LEN] = {0x00, 0x0c, 0x43, 0x26, 0x60, 0x0b};

	if (FtEntry == NULL) {
		FtEntry = MacTableInsertEntry(pAd, StaAddr, &pAd->ApCfg.MBSSID[0].wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);
		FtEntry->Sst = SST_ASSOC;
	}

	os_alloc_mem(pAd, (UCHAR **)&FtInfoBuf, sizeof(FT_INFO));
	if (FtInfoBuf == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_ERROR, "allocate memory failed.\n");
		return;
	}
	NdisZeroMemory(FtInfoBuf, sizeof(FT_INFO));

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	os_zero_mem(pOutBuffer, MAX_MGMT_PKT_LEN);
	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_ERROR, "allocate memory failed.\n");
		os_free_mem(FtInfoBuf);
		return;
	}
	/* Make 802.11 header. */
	ActHeaderInit(pAd, &Hdr, pAd->ApCfg.MBSSID[FtEntry->apidx].wdev.bssid,
				  FtEntry->Addr, pAd->ApCfg.MBSSID[FtEntry->apidx].wdev.bssid);

	FrameLen = 0;
	MakeOutgoingFrame(pOutBuffer,				&FrameLen,
					  sizeof(HEADER_802_11),		&Hdr,
					  END_OF_ARGS);

	/* RSN IE */
{
	PEID_STRUCT  eid_ptr;
	PUCHAR pRsnIe, pRsnIe_payload;
	UCHAR	rsn_len = 0;
	pRsnIe = (PUCHAR)FtInfoBuf->RSN_IE;
	eid_ptr = (PEID_STRUCT)pRsnIe;
	eid_ptr->Eid = IE_RSN;
	pRsnIe_payload = (PUCHAR)&eid_ptr->Octet[0];

{/* 1. insert cipher suite*/
	UCHAR	PairwiseCnt = 0;
	RSNIE2 *pRsnie_cipher = (RSNIE2 *) pRsnIe_payload;
	UCHAR OUI_WPA2_CIPHER_CCMP128[4] = {0x00, 0x0F, 0xAC, 0x04};

	pRsnie_cipher->version = 1;
	NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_CCMP128, 4);
	NdisMoveMemory(pRsnie_cipher->ucast[0].oui + PairwiseCnt*4, OUI_WPA2_CIPHER_CCMP128, 4);
	PairwiseCnt++;
	pRsnie_cipher->ucount = PairwiseCnt;

	pRsnie_cipher->version = cpu2le16(pRsnie_cipher->version);
	pRsnie_cipher->ucount = cpu2le16(pRsnie_cipher->ucount);

	rsn_len = sizeof(RSNIE2) + (4 * (PairwiseCnt - 1));
}
{/* 2. insert AKM*/
	PRSNIE_AUTH pRsnie_auth = (RSNIE_AUTH *) (pRsnIe_payload + rsn_len);
	UCHAR	AkmCnt = 0;
	UCHAR OUI_WPA2_AKM_FT_PSK[4] = {0x00, 0x0F, 0xAC, 0x04};

	NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA2_AKM_FT_PSK, 4);
	AkmCnt++;

	pRsnie_auth->acount = AkmCnt;
	pRsnie_auth->acount = cpu2le16(pRsnie_auth->acount);

	rsn_len += (sizeof(RSNIE_AUTH) + (4 * (AkmCnt - 1)));
}
{/* 3. insert capability*/
	RSN_CAPABILITIES *pRSN_Cap = (RSN_CAPABILITIES *) (pRsnIe_payload + rsn_len);
	pRSN_Cap->word = 0x000c;
	pRSN_Cap->word = cpu2le16(pRSN_Cap->word);

	rsn_len += sizeof(RSN_CAPABILITIES);
}
{/* 4. Insert PMKID */
	PUCHAR pPMKList = (PUCHAR)(pRsnIe_payload + rsn_len);
	UCHAR PMKID[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09
				, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
	UCHAR PMKCount[2] = {0x01, 0x00};

	NdisMoveMemory(pPMKList, PMKCount, 2);
	NdisMoveMemory(pPMKList+2, PMKID, 16);

	rsn_len += 18;
}
	/* hex_dump("The RSNE", pRsnIe_payload, rsn_len); */

	eid_ptr->Len = rsn_len;
	FtInfoBuf->RSNIE_Len = eid_ptr->Len + 2;
}
	/* MD IE */
{
	FtInfoBuf->MdIeInfo.Len = 3;
	FT_SET_MDID(FtInfoBuf->MdIeInfo.MdId, FT_DEFAULT_MDID);
	FtInfoBuf->MdIeInfo.FtCapPlc.field.FtOverDs = 1;
	FtInfoBuf->MdIeInfo.FtCapPlc.field.RsrReqCap = 1;
}
	/* FT IE */
{
	PFT_CFG pFtCfg;

	pFtCfg = &pAd->ApCfg.MBSSID[FtEntry->apidx].wdev.FtCfg;

	FtInfoBuf->FtIeInfo.Len = sizeof(FtInfoBuf->FtIeInfo.MICCtr);
	FtInfoBuf->FtIeInfo.Len += sizeof(FtInfoBuf->FtIeInfo.MIC);
	FtInfoBuf->FtIeInfo.Len += sizeof(FtInfoBuf->FtIeInfo.ANonce);
	FtInfoBuf->FtIeInfo.Len += sizeof(FtInfoBuf->FtIeInfo.SNonce);
	GenRandom(pAd, StaAddr, FtInfoBuf->FtIeInfo.SNonce);

	FtInfoBuf->FtIeInfo.Len += sizeof(((FT_OPTION_FIELD *)0)->SubElementId);
	FtInfoBuf->FtIeInfo.Len += sizeof(FtInfoBuf->FtIeInfo.R0khIdLen);
	FtInfoBuf->FtIeInfo.Len += pFtCfg->FtR0khIdLen;
	NdisMoveMemory(FtInfoBuf->FtIeInfo.R0khId, pFtCfg->FtR0khId, pFtCfg->FtR0khIdLen);
	FtInfoBuf->FtIeInfo.R0khIdLen = pFtCfg->FtR0khIdLen;

	/* hex_dump("The FTE", (PUCHAR)&FtInfoBuf.FtIeInfo, FtInfoBuf.FtIeInfo.Len); */
}
	FT_MakeFtActFrame(pAd, pOutBuffer, &FrameLen,
					  FT_ACTION_BT_REQ, FtEntry->Addr, TargetAddr, 0,
					  FtInfoBuf);

	/* enqueue it into FT action state machine. */
#ifdef CUSTOMER_DCC_FEATURE
	REPORT_MGMT_FRAME_TO_MLME(pAd, FtEntry->wcid, pOutBuffer, FrameLen, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, OPMODE_AP, &pAd->ApCfg.MBSSID[FtEntry->apidx].wdev, FtEntry->HTPhyMode.field.MODE);
#else
	REPORT_MGMT_FRAME_TO_MLME(pAd, FtEntry->wcid, pOutBuffer, FrameLen, 0, 0, 0, 0, 0, 0, OPMODE_AP, &pAd->ApCfg.MBSSID[FtEntry->apidx].wdev, FtEntry->HTPhyMode.field.MODE);
#endif
	if (FtInfoBuf)
		os_free_mem(FtInfoBuf);
	if (pOutBuffer)
		os_free_mem(pOutBuffer);
}

static VOID FT_OverDs_SimConfirm(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
#ifdef FT_FUNC_SIMULATION
/*
========================================================================
Routine Description:
	Display the reponse status for a resource request.

Arguments:
	*pRspStatus		- the status

Return Value:
	None

Note:
========================================================================
*/
static VOID TYPE_FUNC FT_RIC_CMD_StatusDisplay(FT_RIC_STATUS *pRspStatus)
{
	MTWF_PRINT("\nAP MAC = "MACSTR"\n",
			  MAC2STR(pRspStatus->AP_MAC));

	if (pRspStatus->FlgHasBaResource == TRUE) {
		if (pRspStatus->FlgIsBaAccepted == TRUE)
			MTWF_PRINT("\tBA Resource is accepted.\n");
		else
			MTWF_PRINT("\tBA Resource is not accepted.\n"); /* End of if */
	} /* End of if */

	if (pRspStatus->TspecNumberOfRequested > 0) {
		MTWF_PRINT("\tNumber of requested TSPEC: %d\n",
				 pRspStatus->TspecNumberOfRequested);
		MTWF_PRINT("\tNumber of accepted TSPEC: %d\n",
				 pRspStatus->TspecNumberOfAccepted);
		MTWF_PRINT("\tTotal requested Time: %d us\n",
				 pRspStatus->TspecMediumTimeTotalCur);
		MTWF_PRINT("\tTotal accepted Time: %d us\n",
				 pRspStatus->TspecMediumTimeTotalNew);
	} /* End of if */

	if (pRspStatus->UnknownRDIE > 0) {
		MTWF_PRINT("\tNumber of unknown RDIE: %d\n",
				 pRspStatus->UnknownRDIE);
	} /* End of if */
} /* End of FT_RIC_CMD_StatusDisplay */


/*
========================================================================
Routine Description:
	Simulate to start our resource request to the target AP.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	Need to build any TSPEC before.

	EX: Build a simulated TSPEC.

	iwpriv ra0 set acm=69_0_0_1_1
	iwpriv ra0 set acm=65_1_7_0_1_7_0_500_3000_200000_11000000_10_0
	iwpriv ra0 set acm=65_1_6_1_1_6_0_500_3000_200000_11000000_10_0
	iwpriv ra0 set acm=65_1_5_0_1_5_0_500_3000_200000_11000000_10_0
	iwpriv ra0 set acm=65_1_4_1_1_4_0_500_3000_200000_11000000_10_0
	iwpriv ra0 set acm=06_1_0
	iwpriv ra0 set ft=506
	iwpriv ra0 set ft=507_00:11:11:11:11:11_1_0_0_1
	iwpriv ra0 set ft=507_00:22:22:22:22:22_0_0_1_1
	iwpriv ra0 set ft=507_00:33:33:33:33:33_1_1_1_0
	iwpriv ra0 set ft=508_00:22:22:22:22:22
	iwpriv ra0 set ft=509
========================================================================
*/
static VOID TYPE_FUNC FT_RIC_CMD_SimRscReqStart(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
	FT_RIC_ResourceRequestStart(pAd);
	gFT_RIC_RspStatusIndex = 0;
} /* End of FT_RIC_CMD_SimRscReqStart */


/*
========================================================================
Routine Description:
	Simulate to send our resource request to the target AP.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	Command: [AP MAC] [Status for TSPEC 1] [Status for TSPEC 2]
	[Status for TSPEC 3] [Status for TSPEC 4] [Status for TSPEC 5]
	[Status for TSPEC 6] [Status for TSPEC 7] [Status for TSPEC 8]
========================================================================
*/
static VOID TYPE_FUNC FT_RIC_CMD_SimRscReq(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
	FT_ELM_RIC_DATA_INFO *pElmDataInfoRsp;
	UCHAR *pFrame, *pFrameRDIE, *pFrameNextRDIE, *pFrameRSC;
	UINT32 FilledLen, RdieLen, RscLen, RspLen = 0;
	UCHAR MacPeer[6] = {0};
	BOOLEAN RspStatus[8];
	UINT32 IdStatus, RDIE_Index;

	/* sanity check */
	if (gFT_RIC_RspStatusIndex >= FT_RIC_SIM_AP_MAX)
		return;

	/* End of if */
	/* get AP mac address */
	FT_CMD_UtilMacGet(&pArgv, MacPeer);

	if (*(UINT32 *)MacPeer == 0)
		memcpy(MacPeer, gFT_MAC_OldAP, 6);

	/* End of if */
	memset(RspStatus, 0, sizeof(RspStatus));

	if (Argc > ARRAY_SIZE(RspStatus)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
			 "limit Argc(%d) to %lu\n",
			  Argc, (ULONG)ARRAY_SIZE(RspStatus));
		Argc = ARRAY_SIZE(RspStatus);
	}

	for (IdStatus = 0; IdStatus < (Argc - 1); IdStatus++)
		RspStatus[IdStatus] = FT_CMD_UtilNumGet(&pArgv);

	/* End of for */
	/* init */
	os_alloc_mem(pAd, (UCHAR **)&pFrame, 1000);

	if (pFrame == NULL)
		return;

	memset(pFrame, 0, 1000);
	RspLen = 0;
	/* simulate to request */
	FilledLen = FT_RIC_ResourceRequest(pAd, MacPeer, pFrame, 1000);

	/* modify response status */
	if (Argc > 1) {
		RDIE_Index = 0;
		RspLen = 0;
		pFrameRDIE = pFrame;

		while (FilledLen > 0) {
			/* update RDIE status */
			pElmDataInfoRsp = (FT_ELM_RIC_DATA_INFO *)pFrameRDIE;
			pElmDataInfoRsp->StatusCode = RspStatus[RDIE_Index];
			RdieLen = FT_ELM_HDR_LEN + *(pFrameRDIE + 1);
			pFrameRSC = pFrameRDIE + RdieLen;
			RscLen = FT_ELM_HDR_LEN + *(pFrameRSC + 1);
			RspLen += RdieLen;
			FilledLen -= (RdieLen + RscLen);

			/* update resource element */
			if (pElmDataInfoRsp->StatusCode != 0) {
				/* delete the following resource TSPEC */
				pElmDataInfoRsp->RD_Count = 0;
				pFrameNextRDIE = pFrameRSC + RscLen;
				memcpy(pFrameRSC, pFrameNextRDIE, FilledLen);
				pFrameRDIE += (RdieLen);
			} else {
				RspLen += RscLen;
				pFrameRDIE += (RdieLen + RscLen);
			} /* End of if */

			/* check next RDIE pair */
			RDIE_Index++;
		} /* End of while */
	} /* End of if */

	/* simulate to response (Medium Time will be 0) */
	FT_RIC_ResourceResponseHandle(pAd, MacPeer, pFrame, RspLen,
								  &gFT_RIC_RspStatus[gFT_RIC_RspStatusIndex]);
	/* display status */
	FT_RIC_CMD_StatusDisplay(&gFT_RIC_RspStatus[gFT_RIC_RspStatusIndex]);
	os_free_mem(pFrame);
	gFT_RIC_RspStatusIndex++;
} /* End of FT_RIC_CMD_SimRscReq */


/*
========================================================================
Routine Description:
	Simulate to end our resource request to the target AP.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
	Command: [Selected AP MAC]
========================================================================
*/
static VOID TYPE_FUNC FT_RIC_CMD_SimRscReqEnd(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
	UCHAR MacPeer[6] = {0};
	/* get selected AP mac address */
	FT_CMD_UtilMacGet(&pArgv, MacPeer);

	if (*(UINT32 *)MacPeer == 0)
		memcpy(MacPeer, gFT_MAC_OldAP, 6);

	/* End of if */
	/* end resource request mechanism */
	FT_RIC_ResourceRequestEnd(pAd, MacPeer);
} /* End of FT_RIC_CMD_SimRscReqEnd */


/*
========================================================================
Routine Description:
	Simulate to list our resource request response from all APs.

Arguments:
	pAd				- WLAN control block pointer
	Argc			- the number of input parameters
	*pArgv			- input parameters

Return Value:
	None

Note:
========================================================================
*/
static VOID TYPE_FUNC FT_RIC_CMD_SimRscReqRspList(
	IN	PRTMP_ADAPTER		pAd,
	IN	INT32				Argc,
	IN	CHAR				*pArgv)
{
	UINT32 IdRsc;

	for (IdRsc = 0; IdRsc < gFT_RIC_RspStatusIndex; IdRsc++) {
		FT_RIC_CMD_StatusDisplay(&gFT_RIC_RspStatus[IdRsc]);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "\n");
	} /* End of for */
} /* End of FT_RIC_CMD_SimRscReqRspList */

#endif /* FT_FUNC_SIMULATION */
#endif /* CONFIG_STA_SUPPORT */




#define FT_KDP_CMD_DAEMON_KILL			0
#define FT_KDP_CMD_EVENT_LIST			1
#define FT_KDP_CMD_DEBUG_FLAG_CTRL		2

#define FT_KDP_CMD_SM_EVT_FT_ASSOC		500
#define FT_KDP_CMD_SM_EVT_FT_REASSOC	501
#define FT_KDP_CMD_SM_EVT_KEY_REQ		502
#define FT_KDP_CMD_SM_R0KH_INFO_SHOW	503
#define FT_KDP_CMD_SM_R0KH_INFO_CREATE	504
#define FT_R1KH_INFO_SHOW				505
#define FT_RIC_CMD_SM_REQ_START			506
#define FT_RIC_CMD_SM_REQ				507
#define FT_RIC_CMD_SM_REQ_END			508
#define FT_RIC_CMD_SM_REQ_RSP_LIST		509
#define FT_RIC_CMD_SM_REQ_HANDLE		510
#define FT_RIC_CMD_SM_RRB_SEND			511
#define FT_RIC_CMD_SM_REQ_HDL_TSPEC		512
#define FT_11K_CMD_INFO_REQ				513
#define FT_KDP_KEY_SHOW					514

#define FT_REQ_ACT						600
#define FT_CONFIRM_ACT					601
#define FT_ROAMING_ACT					602

/*
========================================================================
Routine Description:
	IO control handler for IEEE802.11r.

Arguments:
	pAd				- WLAN control block pointer
	pArgvIn			- the data flow information

Return Value:
	None

Note:
	All test commands are listed as follows:
		iwpriv ra0 set ft=[cmd id]_[arg1]_[arg2]_......_[argn]
		[cmd id] = xx, such as 000, 001, 002, 003, ...
========================================================================
*/
INT TYPE_FUNC FT_Ioctl(RTMP_ADAPTER * pAd, RTMP_STRING *pArgvIn)
{
	CHAR BufCmd[4] = { 0, 0, 0, 0 };
	CHAR *pArgv, *pParam;
	UINT32 Command, CmdEndOffset, IdCmd;
	INT32 Argc;
	/* init */
	pArgv = (CHAR *)pArgvIn;

	/* get command type */
	/* command format is iwpriv ra0 set acm=[cmd id]_[arg1]_......_[argn] */
	for (IdCmd = 0; IdCmd <= 3; IdCmd++) {
		if ((pArgv[IdCmd] == 0x00) || (pArgv[IdCmd] == '_')) {
			CmdEndOffset = IdCmd;
			break;
		} /* End of if */
	} /* End of for */

	memcpy(BufCmd, pArgv, IdCmd);
	Command = FT_ARG_ATOI(BufCmd);
	pArgv += IdCmd; /* skip command field */
	/* get Argc number */
	Argc = 0;
	pParam = pArgv;

	while (1) {
		if (*pParam == '_')
			Argc++;

		/* End of if */

		if ((*pParam == 0x00) || (Argc > 20))
			break;

		/* End of if */
		pParam++;
	} /* End of while */

	pArgv++; /* skip _ points to arg1 */

	/* handle the command */
	switch (Command) {
#ifdef CONFIG_AP_SUPPORT

	/* normal commands */
	case FT_KDP_CMD_DAEMON_KILL:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> kill daemon!\n");
		FT_KDP_EventInform(pAd, BSS0, FT_KDP_SIG_TERMINATE, NULL, 0, 0, NULL);
		break;
#ifndef FT_KDP_FUNC_SOCK_COMM

	case FT_KDP_CMD_EVENT_LIST:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> event list!\n");
		FT_KDP_CMD_EventList(pAd, Argc, pArgv);
		break;
#endif /* FT_KDP_FUNC_SOCK_COMM */

	case FT_KDP_CMD_DEBUG_FLAG_CTRL:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> IAPP daemon debug flag control!\n");
		FT_KDP_CMD_DbgFlagCtrl(pAd, Argc, pArgv);
		break;
#ifdef FT_FUNC_SIMULATION

	/* simulation commands */
	case FT_KDP_CMD_SM_EVT_FT_ASSOC:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> (sm) ft assoc!\n");
		FT_KDP_CMD_SimEvtFtAssoc(pAd, Argc, pArgv);
		break;

	case FT_KDP_CMD_SM_EVT_FT_REASSOC:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> (sm) ft reassoc!\n");
		FT_KDP_CMD_SimEvtFtReAssoc(pAd, Argc, pArgv);
		break;

	case FT_KDP_CMD_SM_EVT_KEY_REQ:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> (sm) key req!\n");
		FT_KDP_CMD_SimKeyReq(pAd, Argc, pArgv);
		break;
#ifdef FT_KDP_FUNC_R0KH_IP_RECORD

	case FT_KDP_CMD_SM_R0KH_INFO_CREATE:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> (sm) R0KH INFO create!\n");
		FT_KDP_CMD_SimR0KH_InfoCreate(pAd, Argc, pArgv);
		break;
#endif /* FT_KDP_FUNC_R0KH_IP_RECORD */

	case FT_RIC_CMD_SM_REQ_HANDLE:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> (sm) handle a resource request!\n");
		FT_RIC_CMD_SimRscReqHandle(pAd, Argc, pArgv);
		break;

	case FT_RIC_CMD_SM_REQ_HDL_TSPEC:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> (sm) make a resource request TSPEC!\n");
		FT_RIC_CMD_SimRscReqHdlTspec(pAd, Argc, pArgv);
		break;

	case FT_RIC_CMD_SM_RRB_SEND:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> (sm) send a RRB frame!\n");
		FT_RRB_CMD_SimSend(pAd, Argc, pArgv);
		break;

	case FT_11K_CMD_INFO_REQ:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> (sm) send a info request frame!\n");
		FT_11K_CMD_SimInfoReq(pAd, Argc, pArgv);
		break;

	case FT_KDP_KEY_SHOW:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> (sm) show key information!\n");
		FT_11K_CMD_SimKeyShow(pAd, Argc, pArgv);
		break;
#endif /* FT_FUNC_SIMULATION */
#ifdef FT_KDP_FUNC_R0KH_IP_RECORD

	case FT_KDP_CMD_SM_R0KH_INFO_SHOW:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> (sm) R0KH INFO show!\n");
		FT_KDP_CMD_R0KH_InfoShow(pAd, (RTMP_STRING *)pArgv);
		break;
#endif /* FT_KDP_FUNC_R0KH_IP_RECORD */

	case FT_R1KH_INFO_SHOW:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> (sm) R1KH INFO show!\n");
		FT_R1KH_InfoShow(pAd, Argc, pArgv);
		break;

	case FT_REQ_ACT:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> FT_REQ_ACT!\n");
		FT_OverDs_SimReq(pAd, Argc, pArgv);
		break;

	case FT_CONFIRM_ACT:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> FT_CONFIRM_ACT!\n");
		FT_OverDs_SimConfirm(pAd, Argc, pArgv);
		break;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#ifdef FT_FUNC_SIMULATION

	case FT_RIC_CMD_SM_REQ_START:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> (sm) Resource Request Start!\n");
		FT_RIC_CMD_SimRscReqStart(pAd, Argc, pArgv);
		break;

	case FT_RIC_CMD_SM_REQ:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> (sm) Resource Request!\n");
		FT_RIC_CMD_SimRscReq(pAd, Argc, pArgv);
		break;

	case FT_RIC_CMD_SM_REQ_END:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> (sm) Resource Request End!\n");
		FT_RIC_CMD_SimRscReqEnd(pAd, Argc, pArgv);
		break;

	case FT_RIC_CMD_SM_REQ_RSP_LIST:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO, "ft_iocl> (sm) Resource Req/Rsp List!\n");
		FT_RIC_CMD_SimRscReqRspList(pAd, Argc, pArgv);
		break;
#endif /* FT_FUNC_SIMULATION */
#endif /* CONFIG_STA_SUPPORT */

	default: /* error command type */
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_INFO,
				 "ft_iocl> ERROR! No such command %d!\n", Command);
		return -EINVAL; /* input error */
	} /* End of switch */

	return TRUE;
} /* End of FT_Ioctl */

#endif /* DOT11R_FT_SUPPORT */

/* End of ft_iocl.c */
