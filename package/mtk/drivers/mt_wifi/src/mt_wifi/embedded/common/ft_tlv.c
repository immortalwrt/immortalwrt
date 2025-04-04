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
 ****************************************************************************

    Module Name:
    ft_tlv.c

    Abstract:

    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Fonchi Wu  12-02-2008    created for 11r soft-AP
 */

#ifdef DOT11R_FT_SUPPORT


#include "rt_config.h"
#include "dot11r_ft.h"


VOID FT_InsertMdIE(
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUINT8 pMdId,
	IN FT_CAP_AND_POLICY FtCapPlc)
{
	ULONG TempLen;
	UINT8 Length;
	UCHAR MDIE = IE_FT_MDIE;
	Length = 3;
	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&MDIE,
						1,				&Length,
						2,				(PUCHAR)pMdId,
						1,				(PUCHAR)&FtCapPlc.word,
						END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
	return;
}

VOID FT_InsertFTIE(
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Length,
	IN FT_MIC_CTR_FIELD MICCtr,
	IN PUINT8 pMic,
	IN PUINT8 pANonce,
	IN PUINT8 pSNonce)
{
	ULONG TempLen;
	UINT16 MICCtrBuf;
	UCHAR FTIE = IE_FT_FTIE;
	MICCtrBuf = cpu2le16(MICCtr.word);
	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&FTIE,
						1,				&Length,
						2,				(PUCHAR)&MICCtrBuf,
						16,				(PUCHAR)pMic,
						32,				(PUCHAR)pANonce,
						32,				(PUCHAR)pSNonce,
						END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
}

VOID FT_FTIE_InsertKhIdSubIE(
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN FT_SUB_ELEMENT_ID SubId,
	IN PUINT8 pKhId,
	IN UINT8 KhIdLen)
{
	ULONG TempLen;
	UCHAR TempSubID;

	if (SubId != FT_R0KH_ID && SubId != FT_R1KH_ID) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_ERROR, "unknown SubId (%d)\n",
				 SubId);
		return;
	}

	/* The lenght of R1KHID must only be 6 octects. */
	if ((SubId == FT_R1KH_ID) && (KhIdLen != 6)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_ERROR, "Invalid R1KHID length (%d)\n",
				 KhIdLen);
		return;
	}

	/* The length of R0KHID must in range of 1 to 48 octects.*/
	if ((SubId == FT_R0KH_ID) && ((KhIdLen > 48) && (KhIdLen < 1))) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_ERROR, "Invalid R0KHID length (%d)\n",
				 KhIdLen);
	}

	TempSubID = (UCHAR)SubId;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&TempSubID,
						1,				&KhIdLen,
						KhIdLen,		(PUCHAR)pKhId,
						END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
}

VOID FT_FTIE_InsertSubIE(
	IN	PUCHAR	pFrameBuf,
	OUT PULONG pFrameLen,
	IN  UINT8  kde_id,
	IN	PUINT8	pSubIe,
	IN	UINT8	SubIe_len)
{
	ULONG TempLen;
	UINT8 Length;
	Length = SubIe_len;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&kde_id,
						1,				&Length,
						Length,			pSubIe,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}


VOID FT_InsertTimeoutIntervalIE(
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN FT_TIMEOUT_INTERVAL_TYPE Type,
	IN UINT32 TimeOutValue)
{
	ULONG TempLen;
	UINT8 Length;
	UINT8 TimeOutIntervalIE;
	UINT8 TimeoutType;
	UINT32 TimeoutValueBuf;
	Length = 5;
	TimeOutIntervalIE = IE_FT_TIMEOUT_INTERVAL;
	TimeoutType = Type;
	TimeoutValueBuf = cpu2le32(TimeOutValue);
	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&TimeOutIntervalIE,
						1,				&Length,
						1,				(PUCHAR)&TimeoutType,
						4,				(PUCHAR)&TimeoutValueBuf,
						END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
}



#endif /* DOT11R_FT_SUPPORT */

