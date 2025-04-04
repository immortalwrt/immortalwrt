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
	testmode_ioctl.c
*/

#if defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#include "hdev/hdev.h"
#endif


static INT EthGetParamAndShiftBuff(BOOLEAN convert, UINT size, UCHAR **buf,
								   IN UCHAR *out)
{
	if (!(*buf)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "*buf NULL pointer with size:%u\n", size);
		return -1;
	}

	if (!out) {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "out NULL pointer with size:%u\n", size);
		return -1;
	}

	NdisMoveMemory(out, *buf, size);
	*buf = *buf + size;

	if (!convert) {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "size %u, ", size);
		return 0;
	}

	if (size == sizeof(UINT32)) {
		UINT32 *tmp = (UINT32 *)out;
		*tmp = PKTL_TRAN_TO_HOST(*tmp);
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "size %u, val: %u\n", size, *tmp);
	} else if (size == sizeof(UINT16)) {
		UINT16 *tmp = (UINT16 *)out;
		*tmp = PKTS_TRAN_TO_HOST(*tmp);
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
				 "size %u, val: %u\n", size, *tmp);
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "size %u not supported\n", size);
		return -1;
	}

	return 0;
}


/* 1to do removed later */
/* to seperate Windows ndis/WDM and Linux */
#if defined(COMPOS_TESTMODE_WIN)
UINT32 mt_dft_mac_cr_range[] = {
	0x50022000, 0x50022000, 0xc84,	 /* USB Controller */
	0x50029000, 0x50029000, 0x210,	 /* USB DMA */
	0x800c006c, 0x800c006c, 0x100,	/* PSE Client */
	0x60000000, 0x20000, 0x200, /* WF_CFG */
	0x60100000, 0x21000, 0x200, /* WF_TRB */
	0x60110000, 0x21200, 0x200, /* WF_AGG */
	0x60120000, 0x21400, 0x200, /* WF_ARB */
	0x60130000, 0x21600, 0x200, /* WF_TMAC */
	0x60140000, 0x21800, 0x200, /* WF_RMAC */
	0x60150000, 0x21A00, 0x200, /* WF_SEC */
	0x60160000, 0x21C00, 0x200, /* WF_DMA */
	0x60170000, 0x21E00, 0x200, /* WF_CFGOFF */
	0x60180000, 0x22000, 0x1000, /* WF_PF */
	0x60190000, 0x23000, 0x200, /* WF_WTBLOFF */
	0x601A0000, 0x23200, 0x200, /* WF_ETBF */

	0x60300000, 0x24000, 0x400, /* WF_LPON */
	0x60310000, 0x24400, 0x200, /* WF_INT */
	0x60320000, 0x28000, 0x4000, /* WF_WTBLON */
	0x60330000, 0x2C000, 0x200, /* WF_MIB */
	0x60400000, 0x2D000, 0x200, /* WF_AON */

	0x80020000, 0x00000, 0x2000, /* TOP_CFG */
	0x80000000, 0x02000, 0x2000, /* MCU_CFG */
	0x50000000, 0x04000, 0x4000, /* PDMA_CFG */
	0xA0000000, 0x08000, 0x8000, /* PSE_CFG */
	0x60200000, 0x10000, 0x10000, /* WF_PHY */

	0x0, 0x0, 0x0,
};

BOOLEAN mt_mac_cr_range_mapping(RTMP_ADAPTER *pAd, UINT32 *mac_addr)
{
	UINT32 mac_addr_hif = *mac_addr;
	INT idx = 0;
	BOOLEAN IsFound = 0;
	UINT32 *mac_cr_range = NULL;

	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT76x6(pAd) || IS_MT7637(pAd))
		mac_cr_range = &mt_dft_mac_cr_range[0];

	if (!mac_cr_range) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "NotSupported Chip for this function!\n");
		return IsFound;
	}

	if (mac_addr_hif >= 0x40000) {
		do {
			if (mac_addr_hif >= mac_cr_range[idx] &&
				mac_addr_hif < (mac_cr_range[idx] + mac_cr_range[idx + 2])) {
				mac_addr_hif -= mac_cr_range[idx];
				mac_addr_hif += mac_cr_range[idx + 1];
				IsFound = 1;
				break;
			}

			idx += 3;
		} while (mac_cr_range[idx] != 0);
	} else
		IsFound = 1;

	*mac_addr = mac_addr_hif;
	return IsFound;
}


UINT32 mt_physical_addr_map(UINT32 addr)
{
	UINT32 global_addr = 0x0, idx = 1;
	extern UINT32 mt_mac_cr_range[];

	if (addr < 0x2000)
		global_addr = 0x80020000 + addr;
	else if ((addr >= 0x2000) && (addr < 0x4000))
		global_addr = 0x80000000 + addr - 0x2000;
	else if ((addr >= 0x4000) && (addr < 0x8000))
		global_addr = 0x50000000 + addr - 0x4000;
	else if ((addr >= 0x8000) && (addr < 0x10000))
		global_addr = 0xa0000000 + addr - 0x8000;
	else if ((addr >= 0x10000) && (addr < 0x20000))
		global_addr = 0x60200000 + addr - 0x10000;
	else if ((addr >= 0x20000) && (addr < 0x40000)) {
		do {
			if ((addr >= mt_mac_cr_range[idx]) && (addr < (mt_mac_cr_range[idx] + mt_mac_cr_range[idx + 1]))) {
				global_addr = mt_mac_cr_range[idx - 1] + (addr - mt_mac_cr_range[idx]);
				break;
			}

			idx += 3;
		} while (mt_mac_cr_range[idx] != 0);

		if (mt_mac_cr_range[idx] == 0) {
			MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "Unknow addr range = %x !!!\n", addr);
		}
	} else
		global_addr = addr;

	return global_addr;
}


static INT32 ResponseToQA(
	struct _HQA_CMD_FRAME *HqaCmdFrame,
	RTMP_IOCTL_INPUT_STRUCT	*WRQ,
	UINT32 Length,
	INT32 Status)
{
	NdisMoveMemory(HqaCmdFrame->Data, &Status, 2);
	HqaCmdFrame->Length = (UINT16)Length;
	*WRQ->BytesRet = sizeof((HqaCmdFrame)->MagicNo) + sizeof((HqaCmdFrame)->Type)
					 + sizeof((HqaCmdFrame)->Id) + sizeof((HqaCmdFrame)->Length)
					 + sizeof((HqaCmdFrame)->Sequence) + Length;

	if (*WRQ->BytesRet > WRQ->OutBufLen)
		*WRQ->BytesRet = WRQ->OutBufLen;

	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "OutBufLen = 0x%x\n", WRQ->OutBufLen);
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "BytesRet = 0x%x\n", *WRQ->BytesRet);
	RTMPMoveMemory(WRQ->OutBuf, WRQ->InBuf, *WRQ->BytesRet);
	return Status;
}

#else /* defined(COMPOS_TESTMODE_WIN) */
static INT ResponseToQA(
	struct _HQA_CMD_FRAME *HqaCmdFrame,
	RTMP_IOCTL_INPUT_STRUCT	*WRQ,
	INT32 Length,
	INT32 Status)
{
	HqaCmdFrame->Length = PKTS_TRAN_TO_NET((Length));
	Status = PKTS_TRAN_TO_NET((Status));
	NdisCopyMemory(HqaCmdFrame->Data, &Status, 2);
	WRQ->u.data.length = sizeof((HqaCmdFrame)->MagicNo) + sizeof((HqaCmdFrame)->Type)
						 + sizeof((HqaCmdFrame)->Id) + sizeof((HqaCmdFrame)->Length)
						 + sizeof((HqaCmdFrame)->Sequence) + PKTS_TRAN_TO_HOST((HqaCmdFrame)->Length);
	MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "WRQ->u.data.length = %d, usr_addr:%p, hqa_addr:%p\n",
			  WRQ->u.data.length, WRQ->u.data.pointer, HqaCmdFrame);
	/* check the range for coverity */
	if (WRQ->u.data.length <= 65535) {
		if (copy_to_user(WRQ->u.data.pointer, (UCHAR *)(HqaCmdFrame), WRQ->u.data.length)) {
			MTWF_DBG(NULL, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "copy_to_user() fail\n");
			return -EFAULT;
		}
	} else
		return -EFAULT;


	return 0;
}
#endif /* TODO: Add lack of functions temporarily, and delete after merge */


static INT32 HQA_OpenAdapter(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ATECtrl->bQAEnabled = TRUE;
	/* Prepare feedback as soon as we can to avoid QA timeout. */
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	if (ATEOp->ATEStart)
		Ret = ATEOp->ATEStart(pAd);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	return Ret;
}

static INT32 HQA_CloseAdapter(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ATECtrl->bQAEnabled = FALSE;
	/* Prepare feedback as soon as we can to avoid QA timeout. */
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	if (ATEOp->ATEStop)
		Ret = ATEOp->ATEStop(pAd);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	return Ret;
}

static INT32 HQA_StartTx(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 TxCount;
	UINT16 TxLength;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy(&TxCount, HqaCmdFrame->Data, 4);
	memcpy(&TxLength, HqaCmdFrame->Data + 4, 2);
	TxCount = PKTL_TRAN_TO_HOST(TxCount);
	TxLength = PKTS_TRAN_TO_HOST(TxLength);
	TESTMODE_SET_PARAM(pAd, TESTMODE_BAND0, ATE_TX_CNT, TxCount);
	TESTMODE_SET_PARAM(pAd, TESTMODE_BAND0, tx_len, TxLength);
	ATECtrl->bQATxStart = TRUE;

	if (ATEOp->tx_commit)
		ATEOp->tx_commit(pAd);

	if (ATEOp->StartTx)
		Ret = ATEOp->StartTx(pAd);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	if (ATECtrl->bQATxStart == TRUE)
		ATECtrl->TxStatus = 1;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

/* 1 todo not support yet */
static INT32 HQA_StartTxExt(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	return Ret;
}

/* 1 todo not support yet */
static INT32 HQA_StartTxContiTx(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

/* 1 todo not support yet */
static INT32 HQA_StartTxCarrier(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_StartRx(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ATECtrl->bQARxStart = TRUE;

	if (ATEOp->StartRx)
		Ret = ATEOp->StartRx(pAd);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_StopTx(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 Mode;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	Mode = TESTMODE_GET_PARAM(pAd, TESTMODE_BAND0, op_mode);
	Mode &= ATE_TXSTOP;
	TESTMODE_SET_PARAM(pAd, TESTMODE_BAND0, op_mode, Mode);
	ATECtrl->bQATxStart = FALSE;

	if (ATEOp->StopTx)
		ATEOp->StopTx(pAd);

	if (ATEOp->tx_revert)
		ATEOp->tx_revert(pAd);

	ATECtrl->TxStatus = 0;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_StopContiTx(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Ret = 0;
	/* TODO Get Correct TxfdMode*/
	UINT32 TxfdMode = 1;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	{
		if (ATEOp->StopContinousTx)
			ATEOp->StopContinousTx(pAd, TxfdMode);
		else
			Ret = TM_STATUS_NOTSUPPORT;
	}

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

/* 1 todo not support yet */
static INT32 HQA_StopTxCarrier(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_StopRx(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 Mode;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	Mode = TESTMODE_GET_PARAM(pAd, TESTMODE_BAND0, op_mode);
	Mode &= ATE_RXSTOP;
	TESTMODE_SET_PARAM(pAd, TESTMODE_BAND0, op_mode, Mode);
	ATECtrl->bQARxStart = FALSE;

	if (ATEOp->StopRx)
		Ret = ATEOp->StopRx(pAd);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetTxPath(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT16 Value = 0;
	INT32 ant_sel = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 2);
	ant_sel = PKTS_TRAN_TO_HOST(Value);

	if (ant_sel & 0x8000) {
		ant_sel &= 0x7FFF;
		ant_sel |= 0x80000000;
	}

	if (ATEOp->SetTxAntenna)
		Ret = ATEOp->SetTxAntenna(pAd, ant_sel);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetRxPath(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT16 Value = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 2);
	Value = PKTS_TRAN_TO_HOST(Value);

	if (ATEOp->SetRxAntenna)
		Ret = ATEOp->SetRxAntenna(pAd, (CHAR)Value);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetTxIPG(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Value = 0;
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "val:%d,0x%04x\n", Value, Value);

	if (ATEOp->SetAIFS)
		ATEOp->SetAIFS(pAd, (UINT32)Value);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetTxPower0(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT16 Value = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	ATE_TXPOWER TxPower;
	UINT8 band_idx = TESTMODE_BAND0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 2);
	Value = PKTS_TRAN_TO_HOST(Value);
	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Power = Value;
	TxPower.Dbdc_idx = band_idx;

	if (ATEOp->SetTxPower0)
		Ret = ATEOp->SetTxPower0(pAd, TxPower);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HAQ_SetTxPower1(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT16 Value = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	ATE_TXPOWER TxPower;
	UINT8 band_idx = TESTMODE_BAND0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 2);
	Value = PKTS_TRAN_TO_HOST(Value);
	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Power = Value;
	TxPower.Dbdc_idx = band_idx;

	if (ATEOp->SetTxPower1)
		Ret = ATEOp->SetTxPower1(pAd, TxPower);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetTxPowerExt(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data;
	ATE_TXPOWER TxPower;
	UINT32 power = 0;
	UINT32 Channel = 0;
	UINT32 dbdc_idx = 0;
	UINT32 band_idx = 0;
	UINT32 ant_idx = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&power);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&dbdc_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Channel);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&band_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ant_idx);

	ATECtrl->control_band_idx = (UCHAR)band_idx;
	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Ant_idx = ant_idx;
	TxPower.Power = power;
	TxPower.Channel = Channel;
	TxPower.Dbdc_idx = dbdc_idx;
	TxPower.Band_idx = band_idx;

	if (ATEOp->SetTxPowerX)
		Ret = ATEOp->SetTxPowerX(pAd, TxPower);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_AntennaSel(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	/* todo wait FW confirm */
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetOnOFF(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	/* MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n"); */
	return Ret;
}


static INT32 HQA_FWPacketCMD_ClockSwitchDisable(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 isDisable = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	EthGetParamAndShiftBuff(TRUE, sizeof(isDisable), &data, (UCHAR *)&isDisable);
	ATEOp->ClockSwitchDisable(pAd, isDisable);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetTxPowerEval(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	/* MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n"); */
	return Ret;
}


static INT32 HQA_AntennaSelExt(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 RfModeMask = 0;
	UINT32 RfPortMask = 0;
	UINT32 AntPortMask = 0;
	UINT32 BandIdx = 0;
	UCHAR *data = HqaCmdFrame->Data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	NdisMoveMemory((PUCHAR)&BandIdx, data, sizeof(BandIdx));
	data += sizeof(BandIdx);
	BandIdx = PKTL_TRAN_TO_HOST(BandIdx);
	NdisMoveMemory((PUCHAR)&RfModeMask, data, sizeof(RfModeMask));
	data += sizeof(RfModeMask);
	RfModeMask = PKTL_TRAN_TO_HOST(RfModeMask);
	NdisMoveMemory((PUCHAR)&RfPortMask, data, sizeof(RfPortMask));
	data += sizeof(RfPortMask);
	RfPortMask = PKTL_TRAN_TO_HOST(RfPortMask);
	NdisMoveMemory((PUCHAR)&AntPortMask, data, sizeof(AntPortMask));
	data += sizeof(AntPortMask);
	AntPortMask = PKTL_TRAN_TO_HOST(AntPortMask);
#if defined(COMPOS_TESTMODE_WIN)
	pAd->AntennaMainAux = AntPortMask;
#endif
	ATECtrl->control_band_idx = (UCHAR)BandIdx;

	Ret = ATEOp->SetAntennaPort(pAd, RfModeMask, RfPortMask, AntPortMask);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "BandIdx:%x, RfModeMask:%x, RfPortMask:%x, AntPortMask:%x\n",
			  BandIdx, RfModeMask, RfPortMask, AntPortMask);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static HQA_CMD_HANDLER HQA_CMD_SET0[] = {
	/* cmd id start from 0x1000 */
	HQA_OpenAdapter,	/* 0x1000 */
	HQA_CloseAdapter,	/* 0x1001 */
	HQA_StartTx,		/* 0x1002 */
	HQA_StartTxExt,		/* 0x1003 */
	HQA_StartTxContiTx,	/* 0x1004 */
	HQA_StartTxCarrier,	/* 0x1005 */
	HQA_StartRx,		/* 0x1006 */
	HQA_StopTx,		/* 0x1007 */
	HQA_StopContiTx,	/* 0x1008 */
	HQA_StopTxCarrier,	/* 0x1009 */
	HQA_StopRx,		/* 0x100A */
	HQA_SetTxPath,		/* 0x100B */
	HQA_SetRxPath,		/* 0x100C */
	HQA_SetTxIPG,		/* 0x100D */
	HQA_SetTxPower0,	/* 0x100E */
	HAQ_SetTxPower1,	/* 0x100F */
	HQA_SetTxPowerEval,	/* 0x1010 */
	HQA_SetTxPowerExt,	/* 0x1011 */
	HQA_SetOnOFF,		/* 0x1012 */
	HQA_AntennaSel,		/* 0x1013 */
	HQA_FWPacketCMD_ClockSwitchDisable, /* 0x1014 */
	HQA_AntennaSelExt,	/* 0x1015 */
	NULL,/* 0x1016 */
	NULL,/* 0x1017 */
	NULL, /* 0x1018 */
};


static INT32 HQA_SetChannel(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Ret = 0;
	UINT32 Value;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	memcpy((UINT8 *)&Value, HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Channel = %d, BW = %d\n", ATECtrl->channel, ATECtrl->bw);
	ATECtrl->channel = (UINT8)Value;

	if (ATEOp->SetChannel)
		Ret = ATEOp->SetChannel(pAd, (UINT16)Value, 0, 0, 0);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetPreamble(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Value = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	/* 000: Legacy CCK
	 * 001: Legacy OFDM
	 * 010: HT Mixed mode
	 * 011: HT Green field mode
	 * 100: VHT mode
	 */
	TESTMODE_SET_PARAM(pAd, ATECtrl->control_band_idx, tx_mode, (UCHAR)Value);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetRate(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Value = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	ATECtrl->mcs = (UCHAR)Value;
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetNss(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetSystemBW(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Value;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	/* 0: BW_20, 1:BW_40, 2:BW_80, 3:BW_160*/
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "BW = %d\n", Value);

	ATECtrl->bw = Value;
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetPerPktBW(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Value;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%u\n", Value);

	if (Value > ATECtrl->bw)
		Value = ATECtrl->bw;

	ATECtrl->per_pkt_bw = Value;
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetPrimaryBW(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetFreqOffset(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Value = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);

	if (ATEOp->SetTxFreqOffset)
		ATEOp->SetTxFreqOffset(pAd, (UINT32)Value);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetAutoResponder(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetTssiOnOff(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Value = 0, WFSel;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	/* ON/OFF:4 WF Sel:4 */
	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	memcpy((PUCHAR)&WFSel, HqaCmdFrame->Data + 4, 4);
	WFSel = PKTL_TRAN_TO_HOST(WFSel);

	if (ATEOp->SetTSSI)
		ATEOp->SetTSSI(pAd, (CHAR)WFSel, (CHAR)Value);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

/* 1 todo not support yet */
static INT32 HQA_SetRxHighLowTemperatureCompensation(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_LowPower(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Control = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data;

	EthGetParamAndShiftBuff(TRUE, sizeof(Control), &data, (UCHAR *)&Control);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Control:%d\n", Control);

	if (ATEOp->LowPower)
		ATEOp->LowPower(pAd, Control);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_SetEepromToFw(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

	if (ATEOp->SetEepromToFw)
		ATEOp->SetEepromToFw(pAd);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static HQA_CMD_HANDLER HQA_CMD_SET1[] = {
	/* cmd id start from 0x1100 */
	HQA_SetChannel,				/* 0x1100 */
	HQA_SetPreamble,			/* 0x1101 */
	HQA_SetRate,				/* 0x1102 */
	HQA_SetNss,				/* 0x1103 */
	HQA_SetSystemBW,			/* 0x1104 */
	HQA_SetPerPktBW,			/* 0x1105 */
	HQA_SetPrimaryBW,			/* 0x1106 */
	HQA_SetFreqOffset,			/* 0x1107 */
	HQA_SetAutoResponder,			/* 0x1108 */
	HQA_SetTssiOnOff,			/* 0x1109 */
	HQA_SetRxHighLowTemperatureCompensation,/* 0x110A */
	HQA_LowPower,				/* 0x110B */
	HQA_SetEepromToFw			/* 0x110C */
};


static INT32 HQA_ResetTxRxCounter(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 i = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	RX_STATISTIC_RXV *rx_stat;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	AsicGetRxStat(pAd, HQA_RX_RESET_PHY_COUNT);
	AsicGetRxStat(pAd, HQA_RX_RESET_MAC_COUNT);
#ifdef CONFIG_HW_HAL_OFFLOAD
	MtCmdSetPhyCounter(pAd, 0, TESTMODE_BAND0);
	MtCmdSetPhyCounter(pAd, 1, TESTMODE_BAND0);

	if (IS_ATE_DBDC(pAd)) {
		MtCmdSetPhyCounter(pAd, 0, TESTMODE_BAND1);
		MtCmdSetPhyCounter(pAd, 1, TESTMODE_BAND1);
	}

#endif
	MT_ATEUpdateRxStatistic(pAd, TESTMODE_RESET_CNT, NULL);
	ATECtrl->tx_done_cnt = 0;

	if (IS_ATE_DBDC(pAd))
		TESTMODE_SET_PARAM(pAd, TESTMODE_BAND1, ATE_TXDONE_CNT, 0);

	/* reset rx stat fcs error count */
	rx_stat = &pAd->rx_stat_rxv[DBDC_BAND0];
	for (i = 0; i < MAX_USER_NUM; i++)
		rx_stat->fcs_error_cnt[i] = 0;

	if (IS_ATE_DBDC(pAd)) {
		rx_stat = &pAd->rx_stat_rxv[DBDC_BAND1];
		for (i = 0; i < MAX_USER_NUM; i++)
			rx_stat->fcs_error_cnt[i] = 0;
	}

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_GetChipID(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 chip_id;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	chip_id = OS_NTOHL(pAd->ChipID);
	memcpy(HqaCmdFrame->Data + 2, &chip_id, 4);
	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	return Ret;
}

static INT32 HQA_GetFWVersion(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	struct fw_info *fw_info = NULL;
	UINT i = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

	fw_info = &pAd->MCUCtrl.fwdl_ctrl.fw_profile[WM_CPU].fw_info;
	if (fw_info != NULL) {
		MTWF_PRINT("Built date: ");
		for (i = 0; i < 12; i++)
			MTWF_PRINT("%c", fw_info->ram_built_date[i]);
		MTWF_PRINT("\n");

		memcpy(HqaCmdFrame->Data + 2, &fw_info->ram_built_date[0], 12);
	}

	ResponseToQA(HqaCmdFrame, WRQ, 2+12, Ret);
	return Ret;
}

static INT32 HQA_GetStatistics(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}
/* 1 todo not support yet */
static INT32 HQA_GetRxOKData(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

/* 1 todo not support yet */
static INT32 HQA_GetRxOKOther(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

/* 1 todo not support yet */
static INT32 HQA_GetRxAllPktCount(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_GetTxTransmitted(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Value = 0;
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	Value = ATECtrl->tx_done_cnt;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "TxDoneCount = %d\n", ATECtrl->tx_done_cnt);
	Value = PKTL_TRAN_TO_NET(Value);
	memcpy(HqaCmdFrame->Data + 2, &Value, 4);
	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	return Ret;
}

/* 1 todo not support yet */
static INT32 HQA_GetHwCounter(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_CalibrationOperation(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_CalibrationBypassExt(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 Ret = 0;
	UINT32 item = 0;
	UINT32 band_idx = 0;
	UCHAR *data = HqaCmdFrame->Data;

	NdisMoveMemory((UCHAR *)&item, data, sizeof(item));
	data += sizeof(item);
	item = PKTL_TRAN_TO_HOST(item);
	NdisMoveMemory((UCHAR *)&band_idx, data, sizeof(band_idx));
	data += sizeof(band_idx);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);

	ATECtrl->control_band_idx = (UCHAR)band_idx;

	MtCmdDoCalibration(pAd, CALIBRATION_BYPASS, item, band_idx);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "item:%x, band_idx:%x\n", item, band_idx);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetRXVectorIdx(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 Ret = 0;
	UINT32 band_idx = 0;
	UINT32 Group_1 = 0, Group_2 = 0;
	UCHAR *data = HqaCmdFrame->Data;

	NdisMoveMemory((UCHAR *)&band_idx, data, sizeof(band_idx));
	data += sizeof(band_idx);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);
	NdisMoveMemory((UCHAR *)&Group_1, data, sizeof(Group_1));
	data += sizeof(Group_1);
	Group_1 = PKTL_TRAN_TO_HOST(Group_1);
	NdisMoveMemory((UCHAR *)&Group_2, data, sizeof(Group_2));
	data += sizeof(Group_2);
	Group_2 = PKTL_TRAN_TO_HOST(Group_2);

	ATECtrl->control_band_idx = (UCHAR)band_idx;

#ifdef CONFIG_HW_HAL_OFFLOAD
	MtCmdSetRxvIndex(pAd, Group_1, Group_2, band_idx);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "band_idx:%d, G1:%d, G2:%d\n", band_idx, Group_1, Group_2);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetFAGCRssiPath(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	struct _ATE_CTRL *ate_ctrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UINT32 band_idx = 0;
	UINT32 FAGC_Path = 0;
	UCHAR *data = HqaCmdFrame->Data;

	NdisMoveMemory((UCHAR *)&band_idx, data, sizeof(band_idx));
	data += sizeof(band_idx);
	NdisMoveMemory((UCHAR *)&FAGC_Path, data, sizeof(FAGC_Path));
	data += sizeof(FAGC_Path);
	FAGC_Path = PKTL_TRAN_TO_HOST(FAGC_Path);
	TESTMODE_SET_PARAM(pAd, band_idx, fagc_path, FAGC_Path);

	ate_ctrl->control_band_idx = (UCHAR)band_idx;

#ifdef CONFIG_HW_HAL_OFFLOAD
	MtCmdSetFAGCPath(pAd, FAGC_Path, band_idx);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "band_idx:%d, FAGC_Path%d\n", band_idx, FAGC_Path);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static HQA_CMD_HANDLER HQA_CMD_SET2[] = {
	/* cmd id start from 0x1200 */
	HQA_ResetTxRxCounter,		/* 0x1200 */
	HQA_GetStatistics,		/* 0x1201 */
	HQA_GetRxOKData,		/* 0x1202 */
	HQA_GetRxOKOther,		/* 0x1203 */
	HQA_GetRxAllPktCount,		/* 0x1204 */
	HQA_GetTxTransmitted,		/* 0x1205 */
	HQA_GetHwCounter,		/* 0x1206 */
	HQA_CalibrationOperation,	/* 0x1207 */
	HQA_CalibrationBypassExt,	/* 0x1208 */
	HQA_SetRXVectorIdx,		/* 0x1209 */
	HQA_SetFAGCRssiPath,		/* 0x120A */
};


#if !defined(COMPOS_TESTMODE_WIN)
static VOID memcpy_exs(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, ULONG len)
{
	ULONG i;
	USHORT *pDst, *pSrc;

	pDst = (USHORT *) dst;
	pSrc = (USHORT *) src;

	for (i = 0; i < (len >> 1); i++) {
		*pDst = PKTS_TRAN_TO_HOST(*pSrc);
		pDst++;
		pSrc++;
	}

	if ((len % 2) != 0) {
		memcpy(pDst, pSrc, (len % 2));
		*pDst = PKTS_TRAN_TO_HOST(*pDst);
	}
}
#endif


static INT32 HQA_MacBbpRegRead(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Offset, Value = 0;

	memcpy(&Offset, HqaCmdFrame->Data, 4);
	Offset = PKTL_TRAN_TO_HOST(Offset);
#if defined(COMPOS_TESTMODE_WIN)
	Offset = mt_physical_addr_map(Offset);
#endif

	RTMP_IO_READ32(pAd->hdev_ctrl, Offset, &Value);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Offset = %x, Value = %x\n", Offset, Value);
	Value = PKTL_TRAN_TO_NET(Value);
	memcpy(HqaCmdFrame->Data + 2, &Value, 4);
	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	return Ret;
}


static INT32 HQA_MacBbpRegWrite(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Offset, Value;

	memcpy(&Offset, HqaCmdFrame->Data, 4);
	memcpy(&Value, HqaCmdFrame->Data + 4, 4);
	Offset = PKTL_TRAN_TO_HOST(Offset);
	Value = PKTL_TRAN_TO_HOST(Value);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Offset = %x, Value = %x\n", Offset, Value);
#if defined(COMPOS_TESTMODE_WIN)
	Offset = mt_physical_addr_map(Offset);
#endif

	RTMP_IO_WRITE32(pAd->hdev_ctrl, Offset, Value);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


#if !defined(COMPOS_TESTMODE_WIN)
#define REG_SIZE 128
RTMP_REG_PAIR RegPair[REG_SIZE];

VOID RTMP_IO_MCU_READ_BULK(PRTMP_ADAPTER pAd, UCHAR *Dst, UINT32 Offset, UINT32 Len)
{
	UINT32 Index, Value = 0;
	UCHAR *pDst;
	UINT32 NumOfReg = (Len >> 2);
	UINT32 Reg_idx = 0; /* Rate is 0 to REG_SIZE-1 */
	UINT32 i;
	UCHAR OffsetByte = 0x4;
	UINT32 Ret_idx = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */


	for (Index = 0 ; Index < NumOfReg; Index++) {
		RegPair[Reg_idx].Register = Offset + OffsetByte * Index;

		/* Read CR per REG_SIZE or lastest CR */
		if (Reg_idx == REG_SIZE - 1 || Index == NumOfReg - 1) {
#ifdef WIFI_UNIFIED_COMMAND
			if (cap->uni_cmd_support)
				UniCmdMultipleMacRegAccessRead(pAd, RegPair, Reg_idx + 1);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				MtCmdMultipleMacRegAccessRead(pAd, RegPair, Reg_idx + 1);

			for (i = 0; i <= Reg_idx; i++) {
				pDst = (Dst + ((Ret_idx) << 2));
				Value = RegPair[i].Value;
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
						 "Offset = %x, Value = %x\n", RegPair[i].Register, Value);
				Value = PKTL_TRAN_TO_NET(Value);
				memmove(pDst, &Value, 4);
				Ret_idx++;
			}

			Reg_idx = 0;
		} else
			Reg_idx++;
	}
}
#endif /* !defined(COMPOS_TESTMODE_WIN) */


VOID RTMP_IO_READ_BULK(PRTMP_ADAPTER pAd, UCHAR *Dst, UINT32 Offset, UINT32 Len)
{
	UINT32 Index, Value = 0;
	UCHAR *pDst;
	UINT32 NumOfReg = (Len >> 2);
	BOOLEAN IsFound;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UINT32 OffsetTmp = Offset;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n\n");
#if !defined(COMPOS_TESTMODE_WIN)
	IsFound = mt_mac_cr_range_mapping(pAd, &Offset);

	if (!IsFound && NumOfReg > 1 && Offset && !(ATECtrl->op_mode & fATE_IN_ICAPOVERLAP)) {
		RTMP_IO_MCU_READ_BULK(pAd, Dst, Offset, Len);
		return;
	}

#endif /* !defined(COMPOS_TESTMODE_WIN) */
	Offset = OffsetTmp;

	for (Index = 0 ; Index < NumOfReg; Index++) {
		pDst = (Dst + (Index << 2));
#if defined(COMPOS_TESTMODE_WIN)
		Offset = mt_physical_addr_map(Offset);
#endif

		RTMP_IO_READ32(pAd->hdev_ctrl, Offset, &Value);

		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Offset = %x, Value = %x\n", Offset, Value);
		Value = PKTL_TRAN_TO_NET(Value);
		memmove(pDst, &Value, 4);
		OffsetTmp += 4;
		Offset = OffsetTmp;
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n\n");
}


static INT32 HQA_MACBbpRegBulkRead(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Offset;
	UINT16 Len, Tmp;
	INT debug_lvl = DebugLevel;

	DebugLevel = DBG_LVL_OFF;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy(&Offset, HqaCmdFrame->Data, 4);
	Offset = PKTL_TRAN_TO_HOST(Offset);
	memcpy(&Len, HqaCmdFrame->Data + 4, 2);
	Len = PKTS_TRAN_TO_HOST(Len);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Offset = %x, Len(unit: 4bytes) = %d\n", Offset, Len);

	if (Len > 371) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "length requested is too large, make it smaller\n");
		HqaCmdFrame->Length = PKTS_TRAN_TO_NET(2);
		Tmp = PKTS_TRAN_TO_NET(1);
		memcpy(HqaCmdFrame->Data, &Tmp, 2);
		return -EFAULT;
	}

	RTMP_IO_READ_BULK(pAd, HqaCmdFrame->Data + 2, Offset, (Len << 2));/* unit in four bytes*/
	ResponseToQA(HqaCmdFrame, WRQ, 2 + (Len << 2), Ret);
	DebugLevel = debug_lvl;
	return Ret;
}


static INT32 HQA_RfRegBulkRead(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Index, WfSel, Offset, Length, Value;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	memcpy(&WfSel, HqaCmdFrame->Data, 4);
	WfSel = PKTL_TRAN_TO_HOST(WfSel);
	memcpy(&Offset, HqaCmdFrame->Data + 4, 4);
	Offset = PKTL_TRAN_TO_HOST(Offset);
	memcpy(&Length,  HqaCmdFrame->Data + 8, 4);
	Length = PKTL_TRAN_TO_HOST(Length);

	if (ATEOp->RfRegRead) {
		/* check the range for coverity */
		if (Length <= 65535) {
			for (Index = 0; Index < Length; Index++) {
				Ret = ATEOp->RfRegRead(pAd, WfSel, Offset + Index * 4, &Value);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "Wfsel = %d, Offset = %x, Value = %x\n",
						  WfSel, Offset + Index * 4, Value);

				if (Ret) {
					MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Wfsel = %d, Offset = %x, Value = %x fail\n",
							  WfSel, Offset + Index * 4, Value);
					break;
				}

				Value = PKTL_TRAN_TO_NET(Value);
				/* check the range for coverity */
				if (Index < 511)
					memcpy(HqaCmdFrame->Data + 2 + (Index * 4), &Value, 4);
				else
					Ret = TM_STATUS_NOTSUPPORT;
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Length larger than 65535\n");
			Ret = TM_STATUS_NOTSUPPORT;
		}
	} else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2 + (Length * 4), Ret);
	return Ret;
}


static INT32 HQA_RfRegBulkWrite(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Index, WfSel, Offset, Length, Value;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy(&WfSel, HqaCmdFrame->Data, 4);
	WfSel = PKTL_TRAN_TO_HOST(WfSel);
	memcpy(&Offset, HqaCmdFrame->Data + 4, 4);
	Offset = PKTL_TRAN_TO_HOST(Offset);
	memcpy(&Length,  HqaCmdFrame->Data + 8, 4);
	Length = PKTL_TRAN_TO_HOST(Length);

	if (ATEOp->RfRegWrite) {
		/* check the range for coverity, 509=(2048-12)/4*/
		if (Length < 509) {
			for (Index = 0; Index < Length; Index++) {
				memcpy(&Value, HqaCmdFrame->Data + 12 + (Index * 4), 4);
				Value = PKTL_TRAN_TO_HOST(Value);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 "Wfsel = %d, Offset = %x, Value = %x\n",
						  WfSel, Offset + Index * 4, Value);
				Ret = ATEOp->RfRegWrite(pAd, WfSel, Offset + Index * 4, Value);

				if (Ret) {
					MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Wfsel = %d, Offset = %x, Value = %x fail\n",
							  WfSel, Offset + Index * 4, Value);
					break;
				}
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Length larger than 509\n");
			Ret = TM_STATUS_NOTSUPPORT;
		}
	} else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_ReadEEPROM(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
#if !defined(COMPOS_TESTMODE_WIN)/* 1Todo	 RT28xx_EEPROM_READ16 */
	UINT16 Offset = 0, Value = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy(&Offset, HqaCmdFrame->Data, 2);
	Offset = PKTS_TRAN_TO_HOST(Offset);
	RT28xx_EEPROM_READ16(pAd, Offset, Value);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "e2p r %02Xh = 0x%02X\n", (Offset & 0x00FF), (Value & 0x00FF));
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "e2p r %02Xh = 0x%02X\n", (Offset & 0x00FF) + 1, (Value & 0xFF00) >> 8);
	Value = PKTS_TRAN_TO_NET(Value);
	memcpy(HqaCmdFrame->Data + 2, &Value, 2);
#endif
	ResponseToQA(HqaCmdFrame, WRQ, 4, Ret);
	return Ret;
}


static INT32 HQA_WriteEEPROM(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 Offset = 0, Value = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy(&Offset, HqaCmdFrame->Data, 2);
	Offset = PKTS_TRAN_TO_HOST(Offset);
	memcpy(&Value, HqaCmdFrame->Data + 2, 2);
	Value = PKTS_TRAN_TO_HOST(Value);
#if defined(COMPOS_TESTMODE_WIN)
	NdisMoveMemory(&(pAd->EEPROMImage[Offset]), &Value, 2);
#else
	RT28xx_EEPROM_WRITE16(pAd, Offset, Value);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "e2p w 0x%04X = 0x%04X\n", Offset, Value);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "e2p w %02Xh = 0x%02X\n", (Offset & 0x00FF), (Value & 0x00FF));
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "e2p w %02Xh = 0x%02X\n", (Offset & 0x00FF) + 1, (Value & 0xFF00) >> 8);
#endif
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_ReadBulkEEPROM(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 Offset;
	UINT16 Len;
	UINT32 size = EEPROM_SIZE;
	UINT16 *Buffer = NULL;
	struct _RTMP_CHIP_CAP *cap;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	memcpy(&Offset, HqaCmdFrame->Data, 2);
	Offset = PKTS_TRAN_TO_HOST(Offset);
	memcpy(&Len, HqaCmdFrame->Data + 2, 2);
	Len = PKTS_TRAN_TO_HOST(Len);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Offset = %x, Length = %x\n", Offset, Len);
#if defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT)
	if (pAd->E2pAccessMode == E2P_FLASH_MODE)
		size = get_dev_eeprom_size(pAd);
#endif

#if !defined(COMPOS_TESTMODE_WIN) /* 1Todo	 EEReadAll */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
	Ret = os_alloc_mem(pAd, (PUCHAR *)&Buffer, size);	/* TODO verify */

	if (Ret == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "allocate memory for read EEPROM fail\n");
		Len = 0;
		goto HQA_ReadBulkEEPROM_RET;
	}

	EEReadAll(pAd, (UINT16 *)Buffer, size);

	if (Offset + Len <= size)
		memcpy_exs(pAd, HqaCmdFrame->Data + 2, (UCHAR *)Buffer + Offset, Len);
	else {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "exceed EEPROM size (offset:%d, size:%d)\n", Offset+Len, size);
		Len = 0;
		Ret = -1;
	}

	os_free_mem(Buffer);
	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#else

	if (Offset + Len <= cap->EFUSE_BUFFER_CONTENT_SIZE)
		os_move_mem(HqaCmdFrame->Data + 2, (UCHAR *)pAd->EEPROMImage + Offset, Len);
	else {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "exceed EEPROM size\n");
		Len = 0;
		Ret = -1;
	}

#endif
HQA_ReadBulkEEPROM_RET:
	ResponseToQA(HqaCmdFrame, WRQ, 2 + Len, Ret);
	return Ret;
}


static VOID EEWriteBulk(PRTMP_ADAPTER pAd, UINT16 *Data, UINT16 Offset, UINT16 Length)
{
#if !defined(COMPOS_TESTMODE_WIN) /* 1Todo	struct _ATE_CTRL RT28xx_EEPROM_WRITE16 */
	UINT16 Pos;
	UINT16 Value;
	UINT16 Len = Length;

	for (Pos = 0; Pos < (Len >> 1);) {
		Value = Data[Pos];
		RT28xx_EEPROM_WRITE16(pAd, Offset + (Pos * 2), Value);
		Pos++;
	}

#endif
}


static INT32 HQA_WriteBulkEEPROM(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
#if !defined(COMPOS_TESTMODE_WIN) /* 1Todo	 RT28xx_EEPROM_WRITE16 */
	USHORT Offset;
	USHORT Len;
	UINT16 *Buffer = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy(&Offset, HqaCmdFrame->Data, 2);
	Offset = PKTS_TRAN_TO_HOST(Offset);
	memcpy(&Len, HqaCmdFrame->Data + 2, 2);
	Len = PKTS_TRAN_TO_HOST(Len);
	Ret = os_alloc_mem(pAd, (PUCHAR *)&Buffer, cap->EEPROM_DEFAULT_BIN_SIZE);	/* TODO verify */

	if (Ret == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "allocate memory for read EEPROM fail\n");
		goto HQA_WriteBulkEEPROM_RET;
	}

	memcpy_exs(pAd, (UCHAR *)Buffer + Offset, (UCHAR *)HqaCmdFrame->Data + 4, Len);
#if defined(RTMP_FLASH_SUPPORT)

	if (Len == 16)
		memcpy(pAd->EEPROMImage + Offset, (UCHAR *)Buffer + Offset, Len);

	if ((Offset + Len) == EEPROM_SIZE)
		rtmp_ee_flash_write_all(pAd);

	if (Len != 16)
#endif /* RTMP_FLASH_SUPPORT */
	{
		if ((Offset + Len) <= cap->EEPROM_DEFAULT_BIN_SIZE)
			EEWriteBulk(pAd, (UINT16 *)(((UCHAR *)Buffer) + Offset), Offset, Len);
		else {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "exceed EEPROM size(%d)\n", EEPROM_SIZE);
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Offset = %u\n", Offset);
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Length = %u\n", Len);
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 "Offset + Length=%u\n", (Offset + Len));
		}
	}
HQA_WriteBulkEEPROM_RET:
	os_free_mem(Buffer);
	ResponseToQA(HqaCmdFrame, WRQ, 2 + Len, Ret);
#endif
	return Ret;
}


#ifdef RTMP_EFUSE_SUPPORT
static INT32 HQA_CheckEfuseMode(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
#if !defined(COMPOS_TESTMODE_WIN) /* 1Todo	eFuseGetFreeBlockCount */
	UINT32 Value;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

	if (pAd->bUseEfuse)
		Value = 1;
	else
		Value = 0;

	Value = PKTL_TRAN_TO_NET(Value);
	memcpy(HqaCmdFrame->Data + 2, &Value, 4);
#endif
	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	return Ret;
}


static INT32 HQA_GetFreeEfuseBlock(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
#if !defined(COMPOS_TESTMODE_WIN) && !defined(MT7663) /* 1Todo	eFuseGetFreeBlockCount */
	/* remove this block when command RSP function ready */
	UINT32 Value;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	eFuseGetFreeBlockCount(pAd, &Value);
	Value = PKTL_TRAN_TO_NET(Value);
	memcpy(HqaCmdFrame->Data + 2, &Value, 4);
	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
#else
	INT32 GetFreeBlock = 0;
	INT32 offset = 0;
	UINT8 Version, DieIndex;/* 0: D die ; 1: A die */
	UINT32 Result = 0;
	struct _EXT_CMD_EFUSE_FREE_BLOCK_T CmdEfuseFreeBlock;
	struct _EXT_EVENT_EFUSE_FREE_BLOCK_V1_T EventEfuseFreeBlock;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	memcpy((PUCHAR)&Version, (PUCHAR)&HqaCmdFrame->Data, sizeof(Version));
	offset += sizeof(Version);

	memcpy((PUCHAR)&DieIndex, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(DieIndex));
	offset += sizeof(DieIndex);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Version:%d, DieIndex:%d\n", Version, DieIndex);

    memset(&EventEfuseFreeBlock, 0, sizeof(EventEfuseFreeBlock));
    CmdEfuseFreeBlock.ucVersion = Version;
    CmdEfuseFreeBlock.ucDieIndex = DieIndex;

	if (ATEOp->EfuseGetFreeBlock) {
		if (Version == 1) {
			UINT32 ucGetFreeBlock, ucGetTotalBlock;

			ATEOp->EfuseGetFreeBlock(pAd, (PVOID)&CmdEfuseFreeBlock, (PVOID)&EventEfuseFreeBlock);

			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"ucGetFreeBlock:%d, ucGetTotalBlock:%d\n",
					EventEfuseFreeBlock.ucFreeBlockNum, EventEfuseFreeBlock.ucTotalBlockNum);

			ucGetFreeBlock = PKTL_TRAN_TO_HOST(EventEfuseFreeBlock.ucFreeBlockNum);
			ucGetTotalBlock = PKTL_TRAN_TO_HOST(EventEfuseFreeBlock.ucTotalBlockNum);

			memcpy(HqaCmdFrame->Data + 2, &ucGetFreeBlock, 4);
			memcpy(HqaCmdFrame->Data + 6, &ucGetTotalBlock, 4);

			ResponseToQA(HqaCmdFrame, WRQ, 10, Ret);

		} else {
			ATEOp->EfuseGetFreeBlock(pAd, (PVOID)&GetFreeBlock, (PVOID)&Result);
			Result = PKTL_TRAN_TO_HOST(Result);
			memcpy(HqaCmdFrame->Data + 2, &Result, 4);
			ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
		}
	} else {
		Ret = TM_STATUS_NOTSUPPORT;
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	}

#endif
	return Ret;
}


/* 1 todo not support yet */
static INT32 HQA_GetEfuseBlockNr(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_WriteEFuseFromBuffer(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}
#endif /* RTMP_EFUSE_SUPPORT */

static INT32 HQA_GetTxPower(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Channel = 0, AntIdx = 0, Band = 0, Ch_Band = 0, Power = 0;
	UINT32 offset = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy((PUCHAR)&Channel, (PUCHAR)&HqaCmdFrame->Data, sizeof(Channel));
	Channel = PKTL_TRAN_TO_HOST(Channel);
	offset += sizeof(Channel);
	memcpy((PUCHAR)&Band, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Band));
	Band = PKTL_TRAN_TO_HOST(Band);
	offset += sizeof(Band);
	memcpy((PUCHAR)&Ch_Band, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Ch_Band));
	Ch_Band = PKTL_TRAN_TO_HOST(Ch_Band);
	offset += sizeof(Ch_Band);
	memcpy((PUCHAR)&AntIdx, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(AntIdx));
	AntIdx = PKTL_TRAN_TO_HOST(AntIdx);
	offset += sizeof(AntIdx);

	ATECtrl->control_band_idx = (UCHAR)Band;

	if (ATEOp->GetTxPower) {
		ATEOp->GetTxPower(pAd, Channel, Ch_Band, AntIdx, &Power);
		Power = PKTL_TRAN_TO_HOST(Power);
		memcpy(HqaCmdFrame->Data + 2 + 4, &Power, 4);
	} else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 10, Ret);
	return Ret;
}

static INT32 HQA_SetCfgOnOff(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Type, Enable, Band;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy((PUCHAR)&Type, HqaCmdFrame->Data, 4);
	Type = PKTL_TRAN_TO_HOST(Type);
	memcpy((PUCHAR)&Enable, HqaCmdFrame->Data + 4, 4);
	Enable = PKTL_TRAN_TO_HOST(Enable);
	memcpy((PUCHAR)&Band, HqaCmdFrame->Data + 8, 4);
	Band = PKTL_TRAN_TO_HOST(Band);

	ATECtrl->control_band_idx = (UCHAR)Band;

	if (ATEOp->SetCfgOnOff)
		ATEOp->SetCfgOnOff(pAd, Type, Enable);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_GetFreqOffset(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 FreqOffset = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

	if (ATEOp->GetTxFreqOffset) {
		ATEOp->GetTxFreqOffset(pAd, &FreqOffset);
		FreqOffset = PKTL_TRAN_TO_HOST(FreqOffset);
		memcpy(HqaCmdFrame->Data + 2, &FreqOffset, 4);
		ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	} else {
		Ret = TM_STATUS_NOTSUPPORT;
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	}

	return Ret;
}

static INT32 HQA_DBDCTXTone(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 BandIdx = 0, Control = 0, AntIndex = 0, ToneType = 0, ToneFreq = 0, DcOffset_I = 0, DcOffset_Q = 0, Band = 0;
	INT32 RF_Power = 0, Digital_Power = 0;
	INT32 offset = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	/* BandIdx:4 Control:4 AntIndex:4 ToneType:4 ToneFreq:4 DcOffset_I:4 DcOffset_Q:4 Band:4 */
	memcpy((PUCHAR)&BandIdx, (PUCHAR)&HqaCmdFrame->Data, sizeof(BandIdx));
	BandIdx = PKTL_TRAN_TO_HOST(BandIdx);
	offset += sizeof(BandIdx);
	memcpy((PUCHAR)&Control, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Control));
	Control = PKTL_TRAN_TO_HOST(Control);
	offset += sizeof(Control);
	memcpy((PUCHAR)&AntIndex, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(AntIndex));
	AntIndex = PKTL_TRAN_TO_HOST(AntIndex);
	offset += sizeof(AntIndex);
	memcpy((PUCHAR)&ToneType, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(ToneType));
	ToneType = PKTL_TRAN_TO_HOST(ToneType);
	offset += sizeof(ToneType);
	memcpy((PUCHAR)&ToneFreq, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(ToneFreq));
	ToneFreq = PKTL_TRAN_TO_HOST(ToneFreq);
	offset += sizeof(ToneFreq);
	memcpy((PUCHAR)&DcOffset_I, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(DcOffset_I));
	DcOffset_I = PKTL_TRAN_TO_HOST(DcOffset_I);
	offset += sizeof(DcOffset_I);
	memcpy((PUCHAR)&DcOffset_Q, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(DcOffset_Q));
	DcOffset_Q = PKTL_TRAN_TO_HOST(DcOffset_Q);
	offset += sizeof(DcOffset_Q);
	memcpy((PUCHAR)&Band, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Band));
	Band = PKTL_TRAN_TO_HOST(Band);
	offset += sizeof(Band);
	memcpy((PUCHAR)&RF_Power, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(RF_Power));
	RF_Power = PKTL_TRAN_TO_HOST(RF_Power);
	offset += sizeof(RF_Power);
	memcpy((PUCHAR)&Digital_Power, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Digital_Power));
	Digital_Power = PKTL_TRAN_TO_HOST(Digital_Power);
	offset += sizeof(Digital_Power);

	ATECtrl->control_band_idx = (UCHAR)BandIdx;

	if (ATEOp->DBDCTxTone)
		ATEOp->DBDCTxTone(pAd, Control, AntIndex, ToneType, ToneFreq, DcOffset_I, DcOffset_Q, Band);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	if (ATEOp->SetDBDCTxTonePower)
		ATEOp->SetDBDCTxTonePower(pAd, RF_Power, Digital_Power, AntIndex);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_GetDBDCTXTonePower(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 AntIndex = 0;
	INT32 Power = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

	memcpy((PUCHAR)&AntIndex, (PUCHAR)&HqaCmdFrame->Data, sizeof(AntIndex));
	AntIndex = PKTL_TRAN_TO_HOST(AntIndex);

	if (ATEOp->GetDBDCTxTonePower) {
		ATEOp->GetDBDCTxTonePower(pAd, &Power, AntIndex);
		Power = PKTL_TRAN_TO_HOST(Power);
		memcpy(HqaCmdFrame->Data + 2, &Power, 4);
		ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	} else {
		Ret = TM_STATUS_NOTSUPPORT;
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	}

	return Ret;
}

static INT32 HQA_DBDCContinuousTX(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Band = 0, Control = 0, AntMask = 0, tx_mode = 0, BW = 0;
	INT32 Pri_Ch = 0, Rate = 0, Central_Ch = 0, TxfdMode;
	INT32 offset = 0;
	UCHAR Band_idx = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	/* Band:4 Control:4 AntIndex:4 TxMode:4 BW:4 Pri_Ch:4 Rate:4 */
	memcpy((PUCHAR)&Band, (PUCHAR)&HqaCmdFrame->Data, sizeof(Band));
	Band = PKTL_TRAN_TO_HOST(Band);
	offset += sizeof(Band);
	memcpy((PUCHAR)&Control, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Control));
	Control = PKTL_TRAN_TO_HOST(Control);
	offset += sizeof(Control);
	memcpy((PUCHAR)&AntMask, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(AntMask));
	AntMask = PKTL_TRAN_TO_HOST(AntMask);
	offset += sizeof(AntMask);
	memcpy((PUCHAR)&tx_mode, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(tx_mode));
	tx_mode = PKTL_TRAN_TO_HOST(tx_mode);
	offset += sizeof(tx_mode);
	memcpy((PUCHAR)&BW, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(BW));
	BW = PKTL_TRAN_TO_HOST(BW);
	offset += sizeof(BW);
	memcpy((PUCHAR)&Pri_Ch, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Pri_Ch));
	Pri_Ch = PKTL_TRAN_TO_HOST(Pri_Ch);
	offset += sizeof(Pri_Ch);
	memcpy((PUCHAR)&Rate, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Rate));
	Rate = PKTL_TRAN_TO_HOST(Rate);
	offset += sizeof(Rate);
	memcpy((PUCHAR)&Central_Ch, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Central_Ch));
	Central_Ch = PKTL_TRAN_TO_HOST(Central_Ch);
	offset += sizeof(Central_Ch);
	memcpy((PUCHAR)&TxfdMode, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(TxfdMode));
	TxfdMode = PKTL_TRAN_TO_HOST(TxfdMode);
	offset += sizeof(TxfdMode);

	ATECtrl->control_band_idx = (UCHAR)Band;
	Band_idx = (UCHAR)Band;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Band = %d, Control = %d, AntIndex = %d, Phymode = %d, BW = %d, CH = %d, Rate = %d, Central_Ch = %d, TxfdMode = %d\n",
			  Band, Control, AntMask, tx_mode, BW, Pri_Ch, Rate, Central_Ch, TxfdMode);

	if (Control) {
		if (ATEOp->StartContinousTx && Band_idx < TEST_DBDC_BAND_NUM) {
			TESTMODE_SET_PARAM(pAd, Band_idx, tx_mode, tx_mode);
			TESTMODE_SET_PARAM(pAd, Band_idx, bw, BW);
			TESTMODE_SET_PARAM(pAd, Band_idx, ctrl_ch, Pri_Ch);
			TESTMODE_SET_PARAM(pAd, Band_idx, channel, Central_Ch);
			TESTMODE_SET_PARAM(pAd, Band_idx, mcs, Rate);
			TESTMODE_SET_PARAM(pAd, Band_idx, tx_ant, AntMask);
			ATEOp->StartContinousTx(pAd, AntMask, TxfdMode);
		} else
			Ret = TM_STATUS_NOTSUPPORT;
	} else {
		if (ATEOp->StopContinousTx)
			ATEOp->StopContinousTx(pAd, TxfdMode);
		else
			Ret = TM_STATUS_NOTSUPPORT;
	}

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_SetRXFilterPktLen(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Band = 0, Control = 0, RxPktlen = 0;
	INT32 offset = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy((PUCHAR)&Band, (PUCHAR)&HqaCmdFrame->Data, sizeof(Band));
	Band = PKTL_TRAN_TO_HOST(Band);
	offset += sizeof(Band);
	memcpy((PUCHAR)&Control, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Control));
	Control = PKTL_TRAN_TO_HOST(Control);
	offset += sizeof(Control);
	memcpy((PUCHAR)&RxPktlen, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(RxPktlen));
	RxPktlen = PKTL_TRAN_TO_HOST(RxPktlen);
	offset += sizeof(RxPktlen);

	ATECtrl->control_band_idx = (UCHAR)Band;

	if (ATEOp->SetRXFilterPktLen)
		ATEOp->SetRXFilterPktLen(pAd, Control, RxPktlen);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_GetTXInfo(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 txed_band0 = 0;
	UINT32 txed_band1 = 0;
	UCHAR *data = HqaCmdFrame->Data + 2;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	txed_band0 = TESTMODE_GET_PARAM(pAd, 0, ATE_TXDONE_CNT);
	if (IS_ATE_DBDC(pAd))
		txed_band1 = TESTMODE_GET_PARAM(pAd, 1, ATE_TXDONE_CNT);
	txed_band0 = PKTL_TRAN_TO_NET(txed_band0);
	txed_band1 = PKTL_TRAN_TO_NET(txed_band1);
	NdisMoveMemory(data, (UCHAR *)&txed_band0, sizeof(txed_band0));
	data += sizeof(txed_band0);
	NdisMoveMemory(data, (UCHAR *)&txed_band1, sizeof(txed_band1));
	ResponseToQA(HqaCmdFrame, WRQ, 2 + sizeof(txed_band0) + sizeof(txed_band1), Ret);
	return Ret;
}


static INT32 HQA_GetCfgOnOff(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Type = 0, Band = 0;
	UINT32 Result = 0;
	INT32 offset = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	memcpy((PUCHAR)&Type, (PUCHAR)&HqaCmdFrame->Data, sizeof(Type));
	Type = PKTL_TRAN_TO_HOST(Type);
	offset += sizeof(Type);
	memcpy((PUCHAR)&Band, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Band));
	Band = PKTL_TRAN_TO_HOST(Band);
	offset += sizeof(Band);

	ATECtrl->control_band_idx = (UCHAR)Band;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Type:%d Band:%d\n", Type, Band);

	if (ATEOp->GetCfgOnOff) {
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) || \
	defined(AXE) || defined(MT7626) || defined(MT7915) || defined(MT7986) || \
	defined(MT7916) || defined(MT7981)

		/*FW not support Get Rate power (Type=2)*/
		if ((IS_MT7615(pAd) || IS_MT7622(pAd) || IS_P18(pAd) || IS_MT7663(pAd) ||
			IS_AXE(pAd) || IS_MT7626(pAd) || IS_MT7915(pAd) || IS_MT7986(pAd) ||
			IS_MT7916(pAd) || IS_MT7981(pAd)) && Type != 2)
#endif
		{
			ATEOp->GetCfgOnOff(pAd, Type, &Result);
		}

		Result = PKTL_TRAN_TO_HOST(Result);
		memcpy(HqaCmdFrame->Data + 2, &Result, 4);
		ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	} else {
		Ret = TM_STATUS_NOTSUPPORT;
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	}

	return Ret;
}


static INT32 HQA_SetBufferBin(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
#if defined(COMPOS_TESTMODE_WIN)
	UINT32 buffer_mode_merge = 0;
	UCHAR *data = HqaCmdFrame->Data;

	EthGetParamAndShiftBuff(TRUE, sizeof(buffer_mode_merge), &data, (UCHAR *)&buffer_mode_merge);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "buffer_mode_merge = %d\n", buffer_mode_merge);

	if (buffer_mode_merge == 0)
		pAd->CalFreeMerge = FALSE;
	else
		pAd->CalFreeMerge = TRUE;

#else
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"not support this commmand, to disable cal-free merge use DisableCalFree in profile setting\n");
#endif
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_CA53RegRead(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 ret = 0;
	unsigned long offset;
	UINT32 value;

	NdisMoveMemory((PUCHAR)&offset, (PUCHAR)&HqaCmdFrame->Data, sizeof(unsigned long));

	offset = PKTL_TRAN_TO_HOST(offset);
	offset = (unsigned long)ioremap(offset, CA53_GPIO_REMAP_SIZE);
	RTMP_SYS_IO_READ32(offset, &value);
	iounmap((void *)offset);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"offset = %lx, value = %x\n", offset, value);

	value = PKTL_TRAN_TO_NET(value);
	NdisMoveMemory(HqaCmdFrame->Data + 2, (UCHAR *)&value, sizeof(value));

	ResponseToQA(HqaCmdFrame, WRQ, 6, ret);
	return ret;
}


static INT32 HQA_CA53RegWrite(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 ret = 0;
	unsigned long offset;
	UINT32 value;

	NdisMoveMemory((PUCHAR)&offset, (PUCHAR)&HqaCmdFrame->Data, sizeof(unsigned long));
	/* Shift 4 bytes only because dll cmd format */
	NdisMoveMemory((PUCHAR)&value, (PUCHAR)&HqaCmdFrame->Data + 4, sizeof(UINT32));

	offset = PKTL_TRAN_TO_HOST(offset);
	value = PKTL_TRAN_TO_HOST(value);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"offset = %lx, value = %x\n", offset, value);

	offset = (unsigned long)ioremap(offset, CA53_GPIO_REMAP_SIZE);
	RTMP_SYS_IO_WRITE32(offset, value);
	iounmap((void *)offset);

	ResponseToQA(HqaCmdFrame, WRQ, 2, ret);
	return ret;
}


static HQA_CMD_HANDLER HQA_CMD_SET3[] = {
	/* cmd id start from 0x1300 */
	HQA_MacBbpRegRead,		/* 0x1300 */
	HQA_MacBbpRegWrite,		/* 0x1301 */
	HQA_MACBbpRegBulkRead,		/* 0x1302 */
	HQA_RfRegBulkRead,		/* 0x1303 */
	HQA_RfRegBulkWrite,		/* 0x1304 */
	HQA_ReadEEPROM,			/* 0x1305 */
	HQA_WriteEEPROM,		/* 0x1306 */
	HQA_ReadBulkEEPROM,		/* 0x1307 */
	HQA_WriteBulkEEPROM,		/* 0x1308 */
#ifdef RTMP_EFUSE_SUPPORT
	HQA_CheckEfuseMode,		/* 0x1309 */
	HQA_GetFreeEfuseBlock,		/* 0x130A */
	HQA_GetEfuseBlockNr,		/* 0x130B */
	HQA_WriteEFuseFromBuffer,	/* 0x130C */
#else
	NULL,					/* 0x1309 ~ 0x130c */
	NULL,
	NULL,
	NULL,
#endif /* RTMP_EFUSE_SUPPORT */
	HQA_GetTxPower,			/* 0x130D */
	HQA_SetCfgOnOff,		/* 0x130E */
	HQA_GetFreqOffset,		/* 0x130F */
	HQA_DBDCTXTone,			/* 0x1310 */
	HQA_DBDCContinuousTX,		/* 0x1311 */
	HQA_SetRXFilterPktLen,		/* 0x1312 */
	HQA_GetTXInfo,			/* 0x1313 */
	HQA_GetCfgOnOff,		/* 0x1314 */
	NULL,
	HQA_SetBufferBin,		/* 0x1316 */
	HQA_GetFWVersion,		/* 0x1317 */
	HQA_CA53RegRead,		/* 0x1318 */
	HQA_CA53RegWrite,		/* 0x1319 */
	HQA_GetDBDCTXTonePower,	/* 0x131A */
};


/* 1 todo not support yet */
static INT32 HQA_ReadTempReferenceValue(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

VOID HQA_GetThermalValue_CB(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EXT_EVENT_THERMAL_SENSOR_INFO_T prEventExtCmdResult = (P_EXT_EVENT_THERMAL_SENSOR_INFO_T)Data;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	prEventExtCmdResult->u4SensorResult = le2cpu32(prEventExtCmdResult->u4SensorResult);
	ATECtrl->thermal_val = prEventExtCmdResult->u4SensorResult;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "value: 0x%x\n", prEventExtCmdResult->u4SensorResult);
#if !defined(COMPOS_TESTMODE_WIN)/* 1 todo windows no need RTMP_OS_COMPLETE */
	RTMP_OS_COMPLETE(&ATECtrl->cmd_done);
#endif
}

static INT32 HQA_GetThermalValue(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Value = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

	MtCmdGetThermalSensorResult(pAd, 0, control_band_idx,  &ATECtrl->thermal_val); /* 0: get temperature; 1: get adc */

	Value = PKTL_TRAN_TO_HOST(ATECtrl->thermal_val);
	memcpy(HqaCmdFrame->Data + 2, &Value, 4);
	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	return Ret;
}

/* 1 todo not support yet */
static INT32 HQA_SetSideBandOption(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static HQA_CMD_HANDLER HQA_CMD_SET4[] = {
	/* cmd id start from 0x1400 */
	HQA_ReadTempReferenceValue,	/* 0x1400 */
	HQA_GetThermalValue,		/* 0x1401 */
	HQA_SetSideBandOption,		/* 0x1402 */
};


static INT32 hqa_get_fw_info(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

	if (ATEOp->GetFWInfo)
		Ret = ATEOp->GetFWInfo(pAd, HqaCmdFrame->Data + 2);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, (2+1+8+6+15), Ret);
	return Ret;
}


static INT32 HQA_StartContinousTx(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Value = 0, WFSel = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	/* TODO Get Correct TxfdMode*/
	UINT32 TxfdMode = 1;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	/* TxMode:4 BW:4 PRI_CH:4 RATE:4 WFSel:4 */
	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	ATECtrl->tx_mode = (UCHAR)Value;
	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	ATECtrl->bw = (UCHAR)Value;
	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 8, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	ATECtrl->ctrl_ch = (UCHAR)Value;
	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 12, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	ATECtrl->mcs = (UCHAR)Value;
	memcpy((PUCHAR)&WFSel, (PUCHAR)&HqaCmdFrame->Data + 16, 4);
	WFSel = PKTL_TRAN_TO_HOST(WFSel);

	if (ATEOp->StartContinousTx)
		ATEOp->StartContinousTx(pAd, (CHAR)WFSel, TxfdMode);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetSTBC(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Value = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx:%d\n", control_band_idx);
	memcpy((UINT8 *)&Value, (UINT8 *)&HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	TESTMODE_SET_PARAM(pAd, control_band_idx, stbc, Value);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetShortGI(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Value = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "band_idx:%d\n", control_band_idx);
	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	TESTMODE_SET_PARAM(pAd, control_band_idx, sgi, Value);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetDPD(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Value = 0, WFSel;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	/* ON/OFF:4 WF Sel:4 */
	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	memcpy((PUCHAR)&WFSel, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
	WFSel = PKTL_TRAN_TO_HOST(WFSel);

	if (ATEOp->SetDPD)
		ATEOp->SetDPD(pAd, (CHAR)WFSel, (CHAR)Value);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_StartContiTxTone(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Value = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);

	if (ATEOp->StartTxTone)
		ATEOp->StartTxTone(pAd, Value);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_StopContiTxTone(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

	/* MtAsicSetTxToneTest(pAd, 0, 0); */
	if (ATEOp->StopTxTone)
		ATEOp->StopTxTone(pAd);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_CalibrationTestMode(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 Value = 0;
	INT32 Ret = 0;
	UINT8  Mode = 0;
	INT32 ICaplen = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	NdisMoveMemory((PUCHAR)&ICaplen, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
	ICaplen = PKTL_TRAN_TO_HOST(ICaplen);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "mode = %X ICapLen= %X\n", Value, ICaplen);

	if (Value == 0) {
		if (ATECtrl->op_mode & ATE_FFT)
			ATECtrl->op_mode &= ~ATE_FFT;

		ATECtrl->op_mode &= ~fATE_IN_RFTEST;
		Mode = OPERATION_NORMAL_MODE;
	} else if (Value == 1) {
		ATECtrl->op_mode |= fATE_IN_RFTEST;
		Mode = OPERATION_RFTEST_MODE;
	} else if (Value == 2) {
		ATECtrl->op_mode |= fATE_IN_RFTEST;
		Mode = OPERATION_ICAP_MODE;
	} else if (Value == 3) {
		ATECtrl->op_mode |= fATE_IN_RFTEST;
		ATECtrl->op_mode |= fATE_IN_ICAPOVERLAP;
		Mode = OPERATION_ICAP_OVERLAP;
	} else
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "Mode = %d error!!!\n", Value);

	MtCmdRfTestSwitchMode(pAd, Mode, ICaplen, RF_TEST_DEFAULT_RESP_LEN);
#if !defined(COMPOS_TESTMODE_WIN)
	msleep(100);
	RcUpdateBandCtrl((struct hdev_ctrl *)pAd->hdev_ctrl);
#endif
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_DoCalibrationTestItem(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UINT32 item = 0;
	UINT32 band_idx = 0;
	UCHAR *data = HqaCmdFrame->Data;

	NdisMoveMemory((UCHAR *)&item, data, sizeof(item));
	data += sizeof(item);
	item = PKTL_TRAN_TO_HOST(item);
	NdisMoveMemory((UCHAR *)&band_idx, data, sizeof(band_idx));
	data += sizeof(band_idx);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);

	ATECtrl->control_band_idx = (UCHAR)band_idx;

#if defined(COMPOS_TESTMODE_WIN)
	CreateThread(pAd);
	pAd->ReCalID = item;
	OpenFile(&pAd->hReCalibrationFile, RE_CALIBRATION_FILE, FILE_OPEN_IF, FILE_APPEND_DATA);
	WriteFile("[RECAL DUMP START]\r\n", strlen("[RECAL DUMP START]\r\n"), pAd->hReCalibrationFile);
#endif
	MtCmdDoCalibration(pAd, RE_CALIBRATION, item, band_idx);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "item:%x, band_idx:%x\n", item, band_idx);
#if defined(COMPOS_TESTMODE_WIN)
	os_msec_delay(1000);
	WriteFile("[RECAL DUMP END]\r\n", strlen("[RECAL DUMP END]\r\n"), pAd->hReCalibrationFile);
	CloseFile(pAd->hReCalibrationFile);
	/* create log dump finish file */
	OpenFile(&pAd->hReCalibrationFile, RE_CALIBRATION_FINISH_FILE, FILE_OPEN_IF, FILE_APPEND_DATA);
	CloseFile(pAd->hReCalibrationFile);
#endif
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_eFusePhysicalWrite(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	/* ToDo */
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_eFusePhysicalRead(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	/* ToDo */
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_eFuseLogicalRead(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	/* ToDo */
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_eFuseLogicalWrite(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	/* ToDo */
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_TMRSetting(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
#if !defined(COMPOS_TESTMODE_WIN)
	UCHAR *data = HqaCmdFrame->Data;
	UINT32 value = 0, version = 0, throughold = 0, iter = 0;
	CHAR TMR_Value[8];
	CHAR TMR_HW_Version[8];

	EthGetParamAndShiftBuff(TRUE, sizeof(value), &data, (UCHAR *)&value);
	EthGetParamAndShiftBuff(TRUE, sizeof(version), &data, (UCHAR *)&version);
	EthGetParamAndShiftBuff(TRUE, sizeof(throughold), &data, (UCHAR *)&throughold);
	EthGetParamAndShiftBuff(TRUE, sizeof(iter), &data, (UCHAR *)&iter);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "TMR setting: %u, TMR version: %u TMR throughold: %d TMR iter:%d\n",
			  value, version, throughold, iter);

	if (version == TMR_HW_VER_100)
		version = TMR_VER_1_0;
	else if (version == TMR_HW_VER_150)
		version = TMR_VER_1_5;
	else if (version == TMR_HW_VER_200)
		version = TMR_VER_2_0;
	else {
		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Wrong version %d!!\n", version);
		return FALSE;
	}

	snprintf_ret = snprintf(TMR_Value, sizeof(TMR_Value), "%d", value);
	if (os_snprintf_error(sizeof(TMR_Value), snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"TMR_Value snprintf error!\n");
		return FALSE;
	}

	snprintf_ret = snprintf(TMR_HW_Version, sizeof(TMR_HW_Version), "%d", version);
	if (os_snprintf_error(sizeof(TMR_HW_Version), snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"TMR_HW_Version snprintf error!\n");
		return FALSE;
	}

	TmrUpdateParameter(pAd, throughold, iter);
	setTmrVerProc(pAd, TMR_HW_Version);
	setTmrEnableProc(pAd, TMR_Value);
#endif
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_GetRxSNR(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Value = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	Value = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.SNR[0]);
	memcpy(HqaCmdFrame->Data + 2, &Value, 4);
	Value = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.SNR[1]);
	memcpy(HqaCmdFrame->Data + 6, &Value, 4);
	ResponseToQA(HqaCmdFrame, WRQ, 10, Ret);
	return Ret;
}


static INT32 HQA_WriteBufferDone(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Value = 0;
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
#if !defined(COMPOS_TESTMODE_WIN) /* 1Todo	Set_EepromBufferWriteBack_Proc */

	switch (Value) {
	case E2P_EFUSE_MODE:
		/* update status of Effuse write back */
		pAd->fgQAEffuseWriteBack = TRUE;
		Set_EepromBufferWriteBack_Proc(pAd, "1");
		break;

	case E2P_FLASH_MODE:
		Set_EepromBufferWriteBack_Proc(pAd, "2");
		break;

	case E2P_EEPROM_MODE:
		Set_EepromBufferWriteBack_Proc(pAd, "3");
		break;

	case E2P_BIN_MODE:
		Set_EepromBufferWriteBack_Proc(pAd, "4");
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "Unknow write back mode(%d)\n", Value);
	}

#endif
	/* update status of Effuse write back */
	pAd->fgQAEffuseWriteBack = FALSE;
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_FFT(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 Value = 0;
	INT32 Ret = 0;

	NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%d\n", Value);
	Ret = ATEOp->SetFFTMode(pAd, Value);
	return Ret;
}

static INT32 HQA_SetTxTonePower(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Value = 0;
	INT32 pwr1 = 0;
	INT32 pwr2 = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
	pwr1 = PKTL_TRAN_TO_HOST(Value);
	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
	pwr2 = PKTL_TRAN_TO_HOST(Value);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "pwr1:%d, pwr2:%d\n", pwr1, pwr2);

	if (ATEOp->SetTxTonePower)
		ATEOp->SetTxTonePower(pAd, pwr1, pwr2);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetAIFS(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Value = 0;
	UINT32 SlotTime = 0;
	UINT32 SifsTime = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
	SlotTime = PKTL_TRAN_TO_HOST(Value);
	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
	SifsTime = PKTL_TRAN_TO_HOST(Value);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "SlotTime:%d, SifsTime:%d\n", SlotTime, SifsTime);

	if (ATEOp->SetSlotTime)
		ATEOp->SetSlotTime(pAd, SlotTime, SifsTime);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_MPSSetSeqData(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	UINT32 i = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;
	UINT32 band_idx = 0;
	UINT32 offset = 0;
	struct _HQA_MPS_CB *mps_cb = NULL;

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length) / sizeof(UINT32) - 1;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "len:%u\n", len);

	if ((len > 512) || (len == 0))
		goto MPS_SEQ_DATA_RET;

	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32) * (len));

	if (Ret == NDIS_STATUS_FAILURE)
		goto MPS_SEQ_DATA_RET;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = OS_NTOHL(band_idx);

	ATECtrl->control_band_idx = (UCHAR)band_idx;

	for (i = 0; i < len; i++) {
		offset = 4 + 4 * i;

		if (offset + 4 > sizeof(HqaCmdFrame->Data)) /* Reserved at least 4 byte availbale data */
			break;

		NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + offset, 4);
		mps_setting[i] = PKTL_TRAN_TO_HOST(Value);
	}

	ATEOp->MPSSetParm(pAd, MPS_SEQDATA, len, mps_setting);
	os_free_mem(mps_setting);
MPS_SEQ_DATA_RET:
	if (band_idx < DBDC_BAND_NUM) {
		mps_cb = (struct _HQA_MPS_CB *)TESTMODE_GET_PADDR(pAd, band_idx, mps_cb);
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "len:%u, MPS_CNT:%u\n", len, mps_cb->mps_cnt);
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	} else
		Ret = TM_STATUS_NOTSUPPORT;
	return Ret;
}


static INT32 HQA_MPSSetPayloadLength(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	UINT32 i = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;
	UINT32 band_idx = 0;
	UINT32 offset = 0;
	struct _HQA_MPS_CB *mps_cb = NULL;

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length) / 4 - 1;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "len:%u\n", len);

	if ((len > 1024) || (len == 0))
		goto MPS_PKT_LEN_RET;

	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32) * (len));

	if (Ret == NDIS_STATUS_FAILURE)
		goto MPS_PKT_LEN_RET;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);

	ATECtrl->control_band_idx = (UCHAR)band_idx;

	for (i = 0; i < len; i++) {
		offset = 4 + 4 * i;

		if (offset + 4 > sizeof(HqaCmdFrame->Data)) /* Reserved at least 4 byte availbale data */
			break;

		NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + offset, 4);
		mps_setting[i] = PKTL_TRAN_TO_HOST(Value);
	}

	ATEOp->MPSSetParm(pAd, MPS_PAYLOAD_LEN, len, mps_setting);
	os_free_mem(mps_setting);
MPS_PKT_LEN_RET:
	if (band_idx < DBDC_BAND_NUM) {
		mps_cb = (struct _HQA_MPS_CB *)TESTMODE_GET_PADDR(pAd, band_idx, mps_cb);
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "len:%u, MPS_CNT:%u\n", len, mps_cb->mps_cnt);
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
		return Ret;
	} else {
		Ret = TM_STATUS_NOTSUPPORT;
		return Ret;
	}
}


static INT32 HQA_MPSSetPacketCount(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	UINT32 i = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;
	UINT32 band_idx = 0;
	UINT32 offset = 0;
	struct _HQA_MPS_CB *mps_cb = NULL;

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length) / 4 - 1;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "len:%u\n", len);

	if ((len > 1024) || (len == 0))
		goto MPS_PKT_CNT_RET;

	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32) * (len));

	if (Ret == NDIS_STATUS_FAILURE)
		goto MPS_PKT_CNT_RET;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);

	ATECtrl->control_band_idx = (UCHAR)band_idx;

	for (i = 0; i < len; i++) {
		offset = 4 + 4 * i;

		if (offset + 4 > sizeof(HqaCmdFrame->Data)) /* Reserved at least 4 byte availbale data */
			break;

		NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + offset, 4);
		mps_setting[i] = PKTL_TRAN_TO_HOST(Value);
	}

	ATEOp->MPSSetParm(pAd, MPS_TX_COUNT, len, mps_setting);
	os_free_mem(mps_setting);
MPS_PKT_CNT_RET:
	if (band_idx < DBDC_BAND_NUM) {
		mps_cb = (struct _HQA_MPS_CB *)TESTMODE_GET_PADDR(pAd, band_idx, mps_cb);
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "len:%u, MPS_CNT:%u\n", len, mps_cb->mps_cnt);
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
		return Ret;
	} else {
		Ret = TM_STATUS_NOTSUPPORT;
		return Ret;
	}
}

static INT32 HQA_MPSSetPowerGain(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	UINT32 i = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;
	UINT32 band_idx = 0;
	UINT32 offset = 0;
	struct _HQA_MPS_CB *mps_cb = NULL;

	len = PKTS_TRAN_TO_NET(HqaCmdFrame->Length) / 4 - 1;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "len:%u\n", len);

	if ((len > 1024) || (len == 0))
		goto MPS_SET_PWR_RET;

	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32) * (len));

	if (Ret == NDIS_STATUS_FAILURE)
		goto MPS_SET_PWR_RET;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);

	ATECtrl->control_band_idx = (UCHAR)band_idx;

	for (i = 0; i < len; i++) {
		offset = 4 + 4 * i;

		if (offset + 4 > sizeof(HqaCmdFrame->Data)) /* Reserved at least 4 byte availbale data */
			break;

		NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + offset, 4);
		mps_setting[i] = PKTL_TRAN_TO_HOST(Value);
	}

	ATEOp->MPSSetParm(pAd, MPS_PWR_GAIN, len, mps_setting);
	os_free_mem(mps_setting);
MPS_SET_PWR_RET:
	if (band_idx < DBDC_BAND_NUM) {
		mps_cb = (struct _HQA_MPS_CB *)TESTMODE_GET_PADDR(pAd, band_idx, mps_cb);
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "len:%u, MPS_CNT:%u\n", len, mps_cb->mps_cnt);
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
		return Ret;
	} else {
		Ret = TM_STATUS_NOTSUPPORT;
		return Ret;
	}
}


static INT32 HQA_MPSStart(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Ret = 0;
	UINT32 band_idx = 0;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);

	ATECtrl->control_band_idx = (UCHAR)band_idx;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	Ret = ATEOp->MPSTxStart(pAd);

	if (WRQ)
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}

static INT32 HQA_MPSStop(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 band_idx = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);

	ATECtrl->control_band_idx = (UCHAR)band_idx;

	Ret = ATEOp->MPSTxStop(pAd);

	if (WRQ)
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}

static INT32 HQA_MPSSetNss(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	UINT32 i = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;
	UINT32 band_idx = 0;
	UINT32 offset = 0;
	struct _HQA_MPS_CB *mps_cb = NULL;

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length) / sizeof(UINT32) - 1;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "len:%u\n", len);

	if ((len > 512) || (len == 0))
		goto out;

	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32) * (len));

	if (Ret == NDIS_STATUS_FAILURE)
		goto out;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = OS_NTOHL(band_idx);

	ATECtrl->control_band_idx = (UCHAR)band_idx;

	for (i = 0; i < len; i++) {
		offset = 4 + 4 * i;

		if (offset + 4 > sizeof(HqaCmdFrame->Data)) /* Reserved at least 4 byte availbale data */
			break;

		NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + offset, 4);
		mps_setting[i] = PKTL_TRAN_TO_HOST(Value);
	}

	ATEOp->MPSSetParm(pAd, MPS_NSS, len, mps_setting);
	os_free_mem(mps_setting);
out:
	if (band_idx < DBDC_BAND_NUM) {
		mps_cb = (struct _HQA_MPS_CB *)TESTMODE_GET_PADDR(pAd, band_idx, mps_cb);
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "len:%u, MPS_CNT:%u\n", len, mps_cb->mps_cnt);
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
		return Ret;
	} else {
		Ret = TM_STATUS_NOTSUPPORT;
		return Ret;
	}
}


static INT32 HQA_MPSSetPerpacketBW(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	UINT32 i = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;
	UINT32 band_idx = 0;
	UINT32 per_pkt_bw = 0;
	UINT32 offset = 0;
	struct _HQA_MPS_CB *mps_cb = NULL;

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length) / sizeof(UINT32) - 1;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "len:%u\n", len);

	if ((len > 512) || (len == 0))
		goto out;

	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32) * (len));

	if (Ret == NDIS_STATUS_FAILURE)
		goto out;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = OS_NTOHL(band_idx);

	ATECtrl->control_band_idx = (UCHAR)band_idx;

	for (i = 0; i < len; i++) {
		offset = 4 + 4 * i;

		if (offset + 4 > sizeof(HqaCmdFrame->Data)) /* Reserved at least 4 byte availbale data */
			break;

		NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + offset, 4);
		per_pkt_bw = PKTL_TRAN_TO_HOST(Value);

		switch (per_pkt_bw) {
		case ATE_BAND_WIDTH_20:
			per_pkt_bw = BAND_WIDTH_20;
			break;

		case ATE_BAND_WIDTH_40:
			per_pkt_bw = BAND_WIDTH_40;
			break;

		case ATE_BAND_WIDTH_80:
			per_pkt_bw = BAND_WIDTH_80;
			break;

		case ATE_BAND_WIDTH_10:
			per_pkt_bw = BAND_WIDTH_10;
			break;

		case ATE_BAND_WIDTH_5:
			per_pkt_bw = BAND_WIDTH_5;
			break;

		case ATE_BAND_WIDTH_160:
		case ATE_BAND_WIDTH_8080:
			per_pkt_bw = BAND_WIDTH_160;
			break;

		default:
			per_pkt_bw = BAND_WIDTH_20;
			break;
		}

		mps_setting[i] = per_pkt_bw;
	}

	ATEOp->MPSSetParm(pAd, MPS_PKT_BW, len, mps_setting);
	os_free_mem(mps_setting);
out:
	if (band_idx < DBDC_BAND_NUM) {
		mps_cb = (struct _HQA_MPS_CB *)TESTMODE_GET_PADDR(pAd, band_idx, mps_cb);
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "len:%u, MPS_CNT:%u\n", len, mps_cb->mps_cnt);
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
		return Ret;
	} else {
		Ret = TM_STATUS_NOTSUPPORT;
		return Ret;
	}
}


static INT32 HQA_CheckEfuseModeType(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 val = 0;
	INT32 Ret = 0;

#if !defined(COMPOS_TESTMODE_WIN)
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%x\n", pAd->E2pCtrl.e2pCurMode);
	val = PKTL_TRAN_TO_NET(pAd->E2pCtrl.e2pCurMode);	/* Fix me::pAd unify */
#endif
	NdisMoveMemory(HqaCmdFrame->Data + 2, &(val), sizeof(val));
	ResponseToQA(HqaCmdFrame, WRQ, 2 + sizeof(val), Ret);
	return Ret;
}


static INT32 HQA_CheckEfuseNativeModeType(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 val = 0;
	INT32 Ret = 0;

#if !defined(COMPOS_TESTMODE_WIN)
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%x\n", pAd->E2pAccessMode);
	val = PKTL_TRAN_TO_NET(pAd->E2pAccessMode);	/* Fix me::pAd unify */
#endif
	NdisMoveMemory(HqaCmdFrame->Data + 2, &(val), sizeof(val));
	ResponseToQA(HqaCmdFrame, WRQ, 2 + sizeof(val), Ret);
	return Ret;
}


static INT32 HQA_SetBandMode(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UCHAR *data = HqaCmdFrame->Data;
	UINT32 band_mode = 0;
	UINT32 band_type = 0;

	NdisMoveMemory((UCHAR *)&band_mode, data, sizeof(band_mode));
	data += sizeof(band_mode);
	band_mode = PKTL_TRAN_TO_HOST(band_mode);
	NdisMoveMemory((UCHAR *)&band_type, data, sizeof(band_type));
	data += sizeof(band_type);
	band_type = PKTL_TRAN_TO_HOST(band_type);
#ifndef COMPOS_TESTMODE_WIN

	/* todo: windows do not have Set_WirelessMode_Proc function */
	if (band_mode == ATE_SINGLE_BAND) {
		if (band_type == ATE_ABAND_TYPE)
			Set_WirelessMode_Proc(pAd, "14");
		else if (band_type == ATE_GBAND_TYPE)
			Set_WirelessMode_Proc(pAd, "9");
		else
			Ret = -1;
	}

#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "band_mode:%x, band_type:%x\n", band_mode, band_type);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_GetBandMode(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 band_mode = 0;
	UCHAR *data = HqaCmdFrame->Data;
	UINT32 band_idx = 0;
	UINT32 is_dbdc = IS_ATE_DBDC(pAd);

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&band_idx);

	/*
	    DLL will query two times per band0/band1 if DBDC chip set.
	    0: no this band
	    1: 2.4G
	    2: 5G
	    3. 2.4G+5G
	*/
#ifdef DBDC_MODE
	if (is_dbdc) {
		if (IS_MT7626(pAd))
			band_mode = (band_idx == TESTMODE_BAND0) ? 1 : 2;
		else
			band_mode = (band_idx == TESTMODE_BAND0) ? 3 : 2;
	} else
#endif /* DBDC_MODE */
	{
		band_mode = 3; /* Always report 2.4+5G*/

		/* If is_dbdc=0, band_idx should not be 1 so return band_mode=0 */
		if (band_idx == TESTMODE_BAND1)
			band_mode = 0;
	}

	band_mode = PKTL_TRAN_TO_HOST(band_mode);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"is_dbdc:%x, band_mode:%x, band_idx:%x\n",
		is_dbdc, band_mode, band_idx);
	NdisMoveMemory(HqaCmdFrame->Data + 2, &(band_mode), sizeof(band_mode));
	ResponseToQA(HqaCmdFrame, WRQ, 2 + sizeof(band_mode), Ret);
	return Ret;
}


static INT32 HQA_RDDStartExt(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 rdd_num = 0;
	UINT32 rdd_in_sel = 0;
	INT32 Ret = 0;
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data;

	NdisMoveMemory((PUCHAR)&rdd_num, data, sizeof(rdd_num));
	rdd_num = PKTL_TRAN_TO_HOST(rdd_num);
	data += sizeof(rdd_num);
	NdisMoveMemory((PUCHAR)&rdd_in_sel, data, sizeof(rdd_in_sel));
	rdd_in_sel = PKTL_TRAN_TO_HOST(rdd_in_sel);
	Ret = ate_ops->onOffRDD(pAd, rdd_num, rdd_in_sel, 1);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%x\n", rdd_num);
	return Ret;
}


static INT32 HQA_RDDStopExt(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 rdd_num = 0;
	UINT32 rdd_in_sel = 0;
	INT32 Ret = 0;
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data;

	NdisMoveMemory((PUCHAR)&rdd_num, data, sizeof(rdd_num));
	rdd_num = PKTL_TRAN_TO_HOST(rdd_num);
	data += sizeof(rdd_num);
	NdisMoveMemory((PUCHAR)&rdd_in_sel, data, sizeof(rdd_in_sel));
	rdd_in_sel = PKTL_TRAN_TO_HOST(rdd_in_sel);
	Ret = ate_ops->onOffRDD(pAd, rdd_num, rdd_in_sel, 0);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "%x\n", rdd_num);
	return Ret;
}


static INT32 HQA_BssInfoUpdate(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 OwnMacIdx = 0, BssIdx = 0;
	UCHAR Bssid[MAC_ADDR_LEN] = {0};
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data;

	EthGetParamAndShiftBuff(TRUE, sizeof(OwnMacIdx), &data, (UCHAR *)&OwnMacIdx);
	EthGetParamAndShiftBuff(TRUE, sizeof(BssIdx), &data, (UCHAR *)&BssIdx);
	EthGetParamAndShiftBuff(FALSE, sizeof(Bssid), &data, Bssid);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "BssIdx:%d Bssid:%02x:%02x:%02x:%02x:%02x:%02x\n",
			  BssIdx, PRINT_MAC(Bssid));
	Ret = ATEOp->BssInfoUpdate(pAd, OwnMacIdx, BssIdx, Bssid);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_DevInfoUpdate(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Band = 0, OwnMacIdx = 0;
	UCHAR Bssid[MAC_ADDR_LEN] = {0};
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data;

	EthGetParamAndShiftBuff(TRUE, sizeof(Band), &data, (UCHAR *)&Band);
	EthGetParamAndShiftBuff(TRUE, sizeof(OwnMacIdx), &data, (UCHAR *)&OwnMacIdx);
	EthGetParamAndShiftBuff(FALSE, sizeof(Bssid), &data, Bssid);
	ATECtrl->control_band_idx = (UCHAR)Band;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Band:%d OwnMacIdx:%d Bssid:%02x:%02x:%02x:%02x:%02x:%02x\n",
			  Band, OwnMacIdx, PRINT_MAC(Bssid));
	Ret = ATEOp->DevInfoUpdate(pAd, OwnMacIdx, Bssid);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static BOOLEAN checkRecalInDumpStatus(
	PRTMP_ADAPTER pAd,
	UINT32 log_type,
	UINT32 log_ctrl,
	UINT32 log_size
	)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_LOG_DUMP_CB *log_cb = &(ATECtrl->log_dump[ATE_LOG_RE_CAL-1]);
	BOOLEAN rStatus = FALSE;

	if (log_cb->is_dumping) {
		if (log_type == ATE_LOG_RE_CAL) {
			if (log_ctrl == ATE_LOG_ON)
				ATECtrl->reCalInDumpSts = REENABLE_RECAL_IN_DUMP;
			else if (log_ctrl == ATE_LOG_OFF) {
				ATECtrl->reCalInDumpSts = DISABLE_RECAL_IN_DUMP;
				ATECtrl->logStsInDump[0] = log_type;
				ATECtrl->logStsInDump[1] = log_ctrl;
				ATECtrl->logStsInDump[2] = log_size;
			} else {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						"Shouldn't happen ! log_ctrl:%d\n", log_ctrl);
				ATECtrl->reCalInDumpSts = DISABLE_RECAL_IN_DUMP;
			}
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"Recal Log dumping, log_ctrl:%d, log_type:%d, reCalInDumpSts:%d\n",
					log_ctrl, log_type, ATECtrl->reCalInDumpSts);
			rStatus = TRUE;
		} else {
			ATECtrl->reCalInDumpSts = NO_CHANGE_RECAL;
		}
	} else {
		ATECtrl->reCalInDumpSts = NO_CHANGE_RECAL;
	}
	return rStatus;
}

static INT32 HQA_LogOnOff(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 band_idx = 0;
	UINT32 log_type = 0;
	UINT32 log_ctrl = 0;
	UINT32 log_size = 200;
	UCHAR *data = HqaCmdFrame->Data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	NdisMoveMemory((PUCHAR)&band_idx, data, sizeof(band_idx));
	data += sizeof(band_idx);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);
	NdisMoveMemory((PUCHAR)&log_type, data, sizeof(log_type));
	data += sizeof(log_type);
	log_type = PKTL_TRAN_TO_HOST(log_type);
	NdisMoveMemory((PUCHAR)&log_ctrl, data, sizeof(log_ctrl));
	data += sizeof(log_ctrl);
	log_ctrl = PKTL_TRAN_TO_HOST(log_ctrl);
	NdisMoveMemory((PUCHAR)&log_size, data, sizeof(log_size));
	data += sizeof(log_size);
	log_size = PKTL_TRAN_TO_HOST(log_size);

	ATECtrl->control_band_idx = (UCHAR)band_idx;

	if (checkRecalInDumpStatus(pAd, log_type, log_ctrl, log_size) == TRUE)
		goto recalDumping;

	Ret = ATEOp->LogOnOff(pAd, log_type, log_ctrl, log_size);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "band_idx:%x, log_type:%x, log_ctrl:%x, en_log:%x, log_size:%u\n",
			  band_idx, log_type, log_ctrl, ATECtrl->en_log, log_size);

	if (log_ctrl == ATE_LOG_ON) {
		switch (log_type) {

		case ATE_LOG_RE_CAL:
			ATECtrl->firstReCal = TRUE;
			break;

		case ATE_LOG_RDD:
			ATECtrl->firstRDD = TRUE;
			break;

		case ATE_LOG_RXV:
			ATECtrl->firstRXV = TRUE;
			break;

		default:
			break;
		}
	}

recalDumping:

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_SetPowerToBufferBin(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 Ret = 0;
	UINT32 band_idx = 0;
	UINT32 power = 0;
	UINT32 channel = 0;
	UINT32 antenna_idx = 0;
	UINT32 efuse_offset = 0;
	UCHAR *data = HqaCmdFrame->Data;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	NdisMoveMemory((PUCHAR)&power, data, sizeof(power));
	data += sizeof(power);
	power = PKTL_TRAN_TO_HOST(power);
	NdisMoveMemory((PUCHAR)&channel, data, sizeof(channel));
	data += sizeof(channel);
	channel = PKTL_TRAN_TO_HOST(channel);
	NdisMoveMemory((PUCHAR)&band_idx, data, sizeof(band_idx));
	data += sizeof(band_idx);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);

	ATECtrl->control_band_idx = (UCHAR)band_idx;

	/* for MT7615 */
	efuse_offset = MtATEGetTxPwrGroup(channel, band_idx, antenna_idx);

	if (efuse_offset >= cap->EFUSE_BUFFER_CONTENT_SIZE)
		Ret = -1;
	else {
		pAd->EEPROMImage[efuse_offset] = power;
		Ret = 0;
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "power:%x, channel:%x, band_idx:%x, offset:%x, antenna_idx:%x\n",
			  power, channel, band_idx, efuse_offset, antenna_idx);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetFrequencyOffsetToBufferBin(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not support\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_HIFTestSetStartLoopback(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 len = 0;
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _HQA_HIFTestParam tmp, *param;
	struct _LOOPBACK_SETTING set;

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	NdisMoveMemory((UCHAR *)&tmp, HqaCmdFrame->Data, sizeof(tmp));
	param = PKTLA_TRAN_TO_HOST(len / 4, &tmp);
	set.StartLen = param->start_len;
	set.StopLen = param->stop_len;
	set.RepeatTimes = param->repeat_time;
	set.IsDefaultPattern = param->is_def_pattern;
	set.BulkOutNumber = param->bulkout_num;
	set.BulkInNumber = param->bulkin_num;
	set.TxAggNumber = param->txagg_num;
	set.RxAggPktLmt = param->rxagg_limit;
	set.RxAggLmt = param->rxagg_lm;
	set.RxAggTO = param->rxagg_to;
	set.RxAggEnable = param->enable_rxagg;
	ate_ctrl->verify_mode = ATE_LOOPBACK;

	if (!pAd->LbCtrl.LoopBackRunning)
		LoopBack_Start(pAd, &set);
	else
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "LB is running\n");

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "start_len:%u, stop_len:%u, repeat:%u, is_default:%u, bulkout_num:%u, bulkin_num:%u, txagg_num:%u, rxagg_limit:%u, rxagg_lm:%u, rxagg_to:%u, enable_rxagg:%u\n",
			  param->start_len, param->stop_len, param->repeat_time, param->is_def_pattern,
			  param->bulkout_num, param->bulkin_num, param->txagg_num, param->rxagg_limit, param->rxagg_lm, param->rxagg_to, param->enable_rxagg);
	return Ret;
}


static INT32 HQA_HIFTestSetStopLoopback(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");

	if (pAd->LbCtrl.LoopBackRunning)
		LoopBack_Stop(pAd);

	pAd->LbCtrl.DebugMode = FALSE;
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_HIFTestGetStatus(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	struct _LOOPBACK_RESULT tmp, *resp;

	LoopBack_Status(pAd, &tmp);
	resp = PKTLA_TRAN_TO_NET(sizeof(tmp) / 4, &tmp);
	NdisMoveMemory(HqaCmdFrame->Data + 2, (UCHAR *)resp, sizeof(tmp));
	ResponseToQA(HqaCmdFrame, WRQ, 2 + sizeof(tmp), Ret);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	return Ret;
}


static INT32 HQA_HIFTestSetTxData(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 len = 0;
	UINT32 tx_len = 0;
	UCHAR *raw = NULL;

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	NdisMoveMemory(&tx_len, HqaCmdFrame->Data, sizeof(tx_len));
	tx_len = PKTL_TRAN_TO_HOST(tx_len);
	raw = HqaCmdFrame->Data + sizeof(UINT32);

	if (pAd->LbCtrl.DebugMode) {
		UINT32 j = 0;

		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "TxExpect Dump(%u): ", tx_len);
		/* check the range for coverity,2044=2048-sizeof(UINT32) */
		if (tx_len < 2044) {
			for (j = 0; j < tx_len; j++)
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%02x", raw[j]);

			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
		} else {
			Ret = -1;
			return Ret;
		}
	}

	LoopBack_ExpectTx(pAd, tx_len, raw);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "len:%u, tx_len:%u\n", len, tx_len);
	return Ret;
}


static INT32 HQA_HIFTestSetRxData(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	UINT32 rx_len = 0;
	UCHAR *raw = NULL;

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	NdisMoveMemory(&rx_len, HqaCmdFrame->Data, sizeof(rx_len));
	rx_len = PKTL_TRAN_TO_HOST(rx_len);
	raw = HqaCmdFrame->Data + sizeof(UINT32);

	if (pAd->LbCtrl.DebugMode) {
		UINT32 j = 0;

		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "RxExpect Dump(%u): ", rx_len);
		/* check the range for coverity,2044=2048-sizeof(UINT32) */
		if (rx_len < 2044) {
			for (j = 0; j < rx_len; j++)
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "%02x", raw[j]);

			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
		} else {
			Ret = -1;
			return Ret;
		}
	}

	LoopBack_ExpectRx(pAd, rx_len, raw);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "len:%u, rx_len:%u\n", len, rx_len);
	return Ret;
}


static INT32 HQA_HIFTestGetTxRxData(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UCHAR *out = HqaCmdFrame->Data + 2;
	/* TODO:: Need check ated allocate size */
	UINT32 tlen = 0;
	UINT32 out_len = 0;
	UINT32 tmp = 0;

	/* Tx Data */
	LoopBack_RawData(pAd, &out_len, TRUE, out + sizeof(UINT32));
	tmp = PKTL_TRAN_TO_NET(out_len);
	NdisMoveMemory(out, &tmp, sizeof(UINT32));
	tlen += out_len;
	out += out_len + sizeof(UINT32);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "tx_out_len:%u\n", out_len);
	/* Rx Data */
	LoopBack_RawData(pAd, &out_len, FALSE, out + sizeof(UINT32));
	tmp = PKTL_TRAN_TO_NET(out_len);
	NdisMoveMemory(out, &tmp, sizeof(UINT32));
	tlen += out_len;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "rx_out_len:%u, total_len:%u\n", out_len, tlen);
	ResponseToQA(HqaCmdFrame, WRQ, 2 + tlen + sizeof(UINT32) * 2, Ret);
	return Ret;
}


static INT32 HQA_UDMAAction(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 val = 0;
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	NdisMoveMemory(HqaCmdFrame->Data + 2, &(val), sizeof(val));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_WIFIPowerOff(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 val = 0;
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	NdisMoveMemory(HqaCmdFrame->Data + 2, &(val), sizeof(val));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 ToDoFunction(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static HQA_CMD_HANDLER HQA_CMD_SET5[] = {
	/* cmd id start from 0x1500 */
	hqa_get_fw_info,					/* 0x1500 */
	HQA_StartContinousTx,				/* 0x1501 */
	HQA_SetSTBC,						/* 0x1502 */
	HQA_SetShortGI,						/* 0x1503 */
	HQA_SetDPD,							/* 0x1504 */
	HQA_SetTssiOnOff,					/* 0x1505 */
	ToDoFunction,				/* 0x1506, replaced by 0x151c */
	HQA_StartContiTxTone,				/* 0x1507 */
	HQA_StopContiTxTone,				/* 0x1508 */
	HQA_CalibrationTestMode,			/* 0x1509 */
	HQA_DoCalibrationTestItem,			/* 0x150A */
	HQA_eFusePhysicalWrite,				/* 0x150B */
	HQA_eFusePhysicalRead,				/* 0x150C */
	HQA_eFuseLogicalRead,				/* 0x150D */
	HQA_eFuseLogicalWrite,				/* 0x150E */
	HQA_TMRSetting,						/* 0x150F */
	HQA_GetRxSNR,						/* 0x1510 */
	HQA_WriteBufferDone,				/* 0x1511 */
	HQA_FFT,							/* 0x1512 */
	HQA_SetTxTonePower,					/* 0x1513 */
	HQA_GetChipID,						/* 0x1514 */
	HQA_MPSSetSeqData,					/* 0x1515 */
	HQA_MPSSetPayloadLength,			/* 0x1516 */
	HQA_MPSSetPacketCount,				/* 0x1517 */
	HQA_MPSSetPowerGain,				/* 0x1518 */
	HQA_MPSStart,						/* 0x1519 */
	HQA_MPSStop,						/* 0x151A */
	ToDoFunction,						/* 0x151B */
	ToDoFunction,						/* 0x151C */
	ToDoFunction,						/* 0x151D */
	ToDoFunction,						/* 0x151E */
	ToDoFunction,						/* 0x151F */
	ToDoFunction,						/* 0x1520 */
	HQA_SetAIFS,						/* 0x1521 */
	HQA_CheckEfuseModeType,				/* 0x1522 */
	HQA_CheckEfuseNativeModeType,		/* 0x1523 */
	HQA_HIFTestSetStartLoopback,		/* 0x1524 */
	HQA_HIFTestSetStopLoopback,			/* 0x1525 */
	HQA_HIFTestGetStatus,				/* 0x1526 */
	HQA_HIFTestSetTxData,				/* 0x1527 */
	HQA_HIFTestSetRxData,				/* 0x1528 */
	HQA_HIFTestGetTxRxData,				/* 0x1529 */
	HQA_UDMAAction,						/* 0x152A */
	HQA_WIFIPowerOff,					/* 0x152B */
	HQA_SetBandMode,					/* 0x152C */
	HQA_GetBandMode,					/* 0x152D */
	HQA_RDDStartExt,					/* 0x152E */
	HQA_RDDStopExt,						/* 0x152F */
	ToDoFunction,						/* 0x1530 */
	HQA_BssInfoUpdate,					/* 0x1531 */
	HQA_DevInfoUpdate,					/* 0x1532 */
	HQA_LogOnOff,						/* 0x1533 */
	HQA_SetPowerToBufferBin,			/* 0x1534 */
	HQA_SetFrequencyOffsetToBufferBin,	/* 0x1535 */
	HQA_MPSSetNss,						/* 0x1536 */
	HQA_MPSSetPerpacketBW,				/* 0x1537 */
};


#ifdef TXBF_SUPPORT
#if defined(MT_MAC) || defined(MT7637)
VOID HQA_BF_INFO_CB(RTMP_ADAPTER *pAd, unsigned char *data, UINT32 len)
{
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _EXT_EVENT_BF_STATUS_T *bf_info = (struct _EXT_EVENT_BF_STATUS_T *)data;
	struct _EXT_EVENT_IBF_STATUS_T *ibf_info = (struct _EXT_EVENT_IBF_STATUS_T *)data;
	UINT32 status = 0;
	UINT32 data_len = 0;
	UCHAR *bf_data = bf_info->aucBuffer;

	if (!(ate_ctrl->op_mode & fATE_IN_BF))
		return;

	ate_ctrl->txbf_info_len = 0;
	os_alloc_mem(pAd, (UCHAR **)&ate_ctrl->txbf_info, sizeof(UCHAR)*len);

	if (!ate_ctrl->txbf_info) {
		status = NDIS_STATUS_RESOURCES;
		goto err0;
	}

	os_zero_mem(ate_ctrl->txbf_info, sizeof(UCHAR)*len);
	ate_ctrl->txbf_info_len = len;

	switch (bf_info->ucBfDataFormatID) {
	case BF_PFMU_TAG:
		if (bf_info->fgBFer)
			data_len = sizeof(PFMU_PROFILE_TAG1) + sizeof(PFMU_PROFILE_TAG2);
		else
			data_len = sizeof(PFMU_PROFILE_TAG1);

		NdisMoveMemory(ate_ctrl->txbf_info, bf_data, data_len);
		ate_ctrl->txbf_info_len = data_len;
		break;

	case BF_PFMU_DATA:
		NdisMoveMemory(ate_ctrl->txbf_info, bf_data, sizeof(PFMU_DATA));
		data_len = sizeof(PFMU_DATA);
		ate_ctrl->txbf_info_len = data_len;
		break;

	case BF_CAL_PHASE:
		ate_ctrl->iBFCalStatus = ibf_info->ucStatus;
		break;

	case BF_QD_DATA:
		NdisMoveMemory(ate_ctrl->txbf_info, bf_data, sizeof(BF_QD));
		data_len = sizeof(BF_QD);
		ate_ctrl->txbf_info_len = data_len;
		break;

	default:
		break;
	}

err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "(%x)\n", status);
	RTMP_OS_COMPLETE(&ate_ctrl->cmd_done);
}
#endif /* defined(MT_MAC) || defined(MT7637) */

static INT32 HQA_TxBfProfileTagInValid(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 invalid = 0;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto BF_PROFILE_TAG_INVALID_FAIL;
	}

	NdisMoveMemory((PUCHAR)&invalid, (PUCHAR)&HqaCmdFrame->Data, sizeof(invalid));
	invalid = PKTL_TRAN_TO_HOST(invalid);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%u", invalid);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_TxBfProfileTag_InValid(pAd, cmd);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "val:%u, str:%s\n", invalid, cmd);
	os_free_mem(cmd);
BF_PROFILE_TAG_INVALID_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_TxBfProfileTagPfmuIdx(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 pfmuidx = 0;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto BF_PROFILE_TAG_PFMU_FAIL;
	}

	NdisMoveMemory((PUCHAR)&pfmuidx, (PUCHAR)&HqaCmdFrame->Data, sizeof(pfmuidx));
	pfmuidx = PKTL_TRAN_TO_HOST(pfmuidx);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%u", pfmuidx);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_TxBfProfileTag_PfmuIdx(pAd, cmd);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "val:%u, str:%s\n", pfmuidx, cmd);
	os_free_mem(cmd);
BF_PROFILE_TAG_PFMU_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_TxBfProfileTagBfType(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 bftype = 0;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto BF_PROFILE_TAG_BFTYPE_FAIL;
	}

	NdisMoveMemory((PUCHAR)&bftype, (PUCHAR)&HqaCmdFrame->Data, 4);
	bftype = PKTL_TRAN_TO_HOST(bftype);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%u", bftype);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_TxBfProfileTag_BfType(pAd, cmd);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "val:%u, str:%s\n", bftype, cmd);
	os_free_mem(cmd);
BF_PROFILE_TAG_BFTYPE_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_TxBfProfileTagBw(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 tag_bw = 0;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto BF_PROFILE_TAG_BW_FAIL;
	}

	NdisMoveMemory((PUCHAR)&tag_bw, (PUCHAR)&HqaCmdFrame->Data, 4);
	tag_bw = PKTL_TRAN_TO_HOST(tag_bw);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%u", tag_bw);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_TxBfProfileTag_DBW(pAd, cmd);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "val:%u, str:%s\n", tag_bw, cmd);
	os_free_mem(cmd);
BF_PROFILE_TAG_BW_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_TxBfProfileTagSuMu(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 su_mu = 0;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto BF_PROFILE_TAG_SUMU_FAIL;
	}

	NdisMoveMemory((PUCHAR)&su_mu, (PUCHAR)&HqaCmdFrame->Data, 4);
	su_mu = PKTL_TRAN_TO_HOST(su_mu);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%u", su_mu);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_TxBfProfileTag_SuMu(pAd, cmd);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "val:%u, str:%s\n", su_mu, cmd);
	os_free_mem(cmd);
BF_PROFILE_TAG_SUMU_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_TxBfProfileTagMemAlloc(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT16 len = 0;
	struct _HQA_BF_TAG_ALLOC tmp, *layout = NULL;
	RTMP_STRING *cmd = NULL;

	if (!HqaCmdFrame) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "HqaCmdFrame is NULL\n");
		Ret = NDIS_STATUS_INVALID_DATA;
		return Ret;
	}

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_MEMALLOC_FAIL;
	}

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);

	if (len > sizeof(tmp)) {
		Ret = NDIS_STATUS_INVALID_DATA;
		goto HQA_TAG_MEMALLOC_FAIL;
	}

	NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
	layout = PKTLA_TRAN_TO_HOST(len / 4, &tmp);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
			layout->col_idx0, layout->row_idx0, layout->col_idx1, layout->row_idx1,
			layout->col_idx2, layout->row_idx2, layout->col_idx3, layout->row_idx3);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_TxBfProfileTag_Mem(pAd, cmd);
#endif
HQA_TAG_MEMALLOC_FAIL:

	if (cmd)
		os_free_mem(cmd);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	if (layout)
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"col0/row0:%x/%x, col1/row1:%x/%x, col2/row2:%x/%x, col3/row3:%x/%x\n",
			layout->col_idx0, layout->row_idx0, layout->col_idx1, layout->row_idx1,
			layout->col_idx2, layout->row_idx2, layout->col_idx3, layout->row_idx3);
	return Ret;
}


static INT32 HQA_TxBfProfileTagMatrix(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT16 len = 0;
	struct _HQA_BF_TAG_MATRIX tmp, *matrix = NULL;
	RTMP_STRING *cmd;

	if (!HqaCmdFrame) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "HqaCmdFrame is NULL\n");
		Ret = NDIS_STATUS_INVALID_DATA;
		return Ret;
	}

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_MATRIX_FAIL;
	}

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	/* check the range for coverity, 24=sizeof(_HQA_BF_TAG_MATRIX)*/
	if (len < 24) {
		NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
		matrix = PKTLA_TRAN_TO_HOST(len / 4, &tmp);
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x", matrix->nrow, matrix->ncol, matrix->ngroup, matrix->LM, matrix->code_book, matrix->htc_exist);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}

#if defined(MT_MAC)
		Set_TxBfProfileTag_Matrix(pAd, cmd);
#endif
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"length larger than 2048\n");
			Ret = NDIS_STATUS_INVALID_DATA;
		}

	os_free_mem(cmd);
HQA_TAG_MATRIX_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	if (matrix)
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"nrow:%x, ncol:%x, ngroup:%x, LM:%x, code_book:%x, htc:%x\n",
		matrix->nrow, matrix->ncol, matrix->ngroup, matrix->LM,
		matrix->code_book, matrix->htc_exist);
	return Ret;
}


static INT32 HQA_TxBfProfileTagSnr(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT16 len = 0;
	struct _HQA_BF_TAG_SNR tmp, *snr = NULL;
	RTMP_STRING *cmd;

	if (!HqaCmdFrame) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "HqaCmdFrame is NULL\n");
		Ret = NDIS_STATUS_INVALID_DATA;
		return Ret;
	}

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_SNR_FAIL;
	}

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	/* check the range for coverity, 16=sizeof(_HQA_BF_TAG_SNR)*/
	if (len < 16) {
		NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
		snr = PKTLA_TRAN_TO_HOST(len / 4, &tmp);
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x", snr->snr_sts0, snr->snr_sts1, snr->snr_sts2, snr->snr_sts3);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}
#if defined(MT_MAC)
		Set_TxBfProfileTag_SNR(pAd, cmd);
#endif
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"length larger than 2048\n");
			Ret = NDIS_STATUS_INVALID_DATA;
		}

	os_free_mem(cmd);
HQA_TAG_SNR_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
		if (snr)
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "snr_sts0:%x, snr_sts1:%x, snr_sts2:%x, snr_sts3:%x,\n",
					 snr->snr_sts0, snr->snr_sts1, snr->snr_sts2, snr->snr_sts3);
	return Ret;

}


static INT32 HQA_TxBfProfileTagSmtAnt(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 smt_ant = 0;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_SMTANT_FAIL;
	}

	NdisMoveMemory((PUCHAR)&smt_ant, (PUCHAR)&HqaCmdFrame->Data, 4);
	/*
	 * DBDC mode:
	 * If Use Band 0, set [11:6]=0x0, set [5:0] as Ant cfg.
	 * If Use Band 1, set [11:6] as Ant cfg, set [5:0] = 0x0.
	 * Non-DBDC mode:
	 * Set [11:0] as Ant cfg.
	 */
	smt_ant = PKTL_TRAN_TO_HOST(smt_ant);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", smt_ant);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_TxBfProfileTag_SmartAnt(pAd, cmd);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "val:%x, str:%s\n", smt_ant, cmd);
	os_free_mem(cmd);
HQA_TAG_SMTANT_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_TxBfProfileTagSeIdx(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 se_idx = 0;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_SEIDX_FAIL;
	}

	NdisMoveMemory((PUCHAR)&se_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	se_idx = PKTL_TRAN_TO_HOST(se_idx);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", se_idx);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_TxBfProfileTag_SeIdx(pAd, cmd);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "val:%x, str:%s\n", se_idx, cmd);
	os_free_mem(cmd);
HQA_TAG_SEIDX_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_TxBfProfileTagRmsdThrd(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 rmsd_thrd = 0;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_RMSDTHRD_FAIL;
	}

	NdisMoveMemory((PUCHAR)&rmsd_thrd, (PUCHAR)&HqaCmdFrame->Data, 4);
	rmsd_thrd = PKTL_TRAN_TO_HOST(rmsd_thrd);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", rmsd_thrd);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_TxBfProfileTag_RmsdThrd(pAd, cmd);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "val:%x, cmd:%s\n", rmsd_thrd, cmd);
	os_free_mem(cmd);
HQA_TAG_RMSDTHRD_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_TxBfProfileTagMcsThrd(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT16 len = 0;
	struct _HQA_BF_TAG_MCS_THRD tmp, *mcs_thrd = NULL;
	RTMP_STRING *cmd;

	if (!HqaCmdFrame) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "HqaCmdFrame is NULL\n");
		Ret = NDIS_STATUS_INVALID_DATA;
		return Ret;
	}

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_MCSTHRD_FAIL;
	}

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	/* check the range for coverity,24=sizeof(_HQA_BF_TAG_MCS_THRD) */
	if (len < 24) {
		NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
		mcs_thrd = PKTLA_TRAN_TO_HOST(len / 4, &tmp);
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x", mcs_thrd->mcs_lss0, mcs_thrd->mcs_sss0, mcs_thrd->mcs_lss1, mcs_thrd->mcs_sss1, mcs_thrd->mcs_lss2, mcs_thrd->mcs_sss2);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}
#if defined(MT_MAC)
		Set_TxBfProfileTag_McsThrd(pAd, cmd);
#endif
		} else {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"length larger than 2048\n");
			Ret = NDIS_STATUS_INVALID_DATA;
		}

	os_free_mem(cmd);
HQA_TAG_MCSTHRD_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	if (mcs_thrd)
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"len:%x, mcs_lss0:%x, mcs_sss0:%x, mcs_lss1:%x, mcs_sss1:%x, mcs_lss2:%x, mcs_sss2:%x\n",
		len, mcs_thrd->mcs_lss0, mcs_thrd->mcs_sss0, mcs_thrd->mcs_lss1, mcs_thrd->mcs_sss1,
		mcs_thrd->mcs_lss2, mcs_thrd->mcs_sss2);
	return Ret;
}


static INT32 HQA_TxBfProfileTagTimeOut(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 bf_tout = 0;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_TOUT_FAIL;
	}

	NdisMoveMemory((PUCHAR)&bf_tout, (PUCHAR)&HqaCmdFrame->Data, 4);
	bf_tout = PKTL_TRAN_TO_HOST(bf_tout);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", bf_tout);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_TxBfProfileTag_TimeOut(pAd, cmd);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "val:%x, str:%s\n", bf_tout, cmd);
	os_free_mem(cmd);
HQA_TAG_TOUT_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_TxBfProfileTagDesiredBw(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 desire_bw = 0;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_DBW_FAIL;
	}

	NdisMoveMemory((PUCHAR)&desire_bw, (PUCHAR)&HqaCmdFrame->Data, 4);
	desire_bw = PKTL_TRAN_TO_HOST(desire_bw);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", desire_bw);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_TxBfProfileTag_DesiredBW(pAd, cmd);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "val:%x, str:%s\n", desire_bw, cmd);
	os_free_mem(cmd);
HQA_TAG_DBW_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_TxBfProfileTagDesiredNc(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 desire_nc = 0;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_DNC_FAIL;
	}

	NdisMoveMemory((PUCHAR)&desire_nc, (PUCHAR)&HqaCmdFrame->Data, 4);
	desire_nc = PKTL_TRAN_TO_HOST(desire_nc);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", desire_nc);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_TxBfProfileTag_DesiredNc(pAd, cmd);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "val:%x, str:%s\n", desire_nc, cmd);
	os_free_mem(cmd);
HQA_TAG_DNC_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_TxBfProfileTagDesiredNr(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 desire_nr = 0;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_DNR_FAIL;
	}

	NdisMoveMemory((PUCHAR)&desire_nr, (PUCHAR)&HqaCmdFrame->Data, 4);
	desire_nr = PKTL_TRAN_TO_HOST(desire_nr);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", desire_nr);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_TxBfProfileTag_DesiredNr(pAd, cmd);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "val:%x, str:%s\n", desire_nr, cmd);
	os_free_mem(cmd);
HQA_TAG_DNR_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_TxBfProfileTagWrite(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 idx = 0;	/* WLAN_IDX */
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_WRITE_FAIL;
	}

	NdisMoveMemory((PUCHAR)&idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	idx = PKTL_TRAN_TO_HOST(idx);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", idx);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_TxBfProfileTagWrite(pAd, cmd);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "val:%x, str:%s\n", idx, cmd);
	os_free_mem(cmd);
HQA_TAG_WRITE_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_TxBfProfileTagRead(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 idx = 0, isBFer = 0;
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	UCHAR *out = NULL;
	RTMP_STRING *cmd;

	ate_ctrl->txbf_info = NULL;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_READ_FAIL;
	}

	ate_ctrl->op_mode |= fATE_IN_BF;
	NdisMoveMemory((PUCHAR)&idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	idx = PKTL_TRAN_TO_HOST(idx);
	NdisMoveMemory((PUCHAR)&isBFer, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
	isBFer = PKTL_TRAN_TO_HOST(isBFer);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	/* check the range for coverity */
	if (idx < 256 && isBFer < 256) {
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x", idx, isBFer);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "val:%x %x, str:%s\n", idx, isBFer, cmd);
	} else
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "idx larger than 255!\n");
#if defined(MT_MAC) || defined(MT7637)
	Set_TxBfProfileTagRead(pAd, cmd);
	if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&ate_ctrl->cmd_done, ate_ctrl->cmd_expire)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wait cmd timeout!\n");
	}
#endif

	if (!ate_ctrl->txbf_info)
		goto HQA_TAG_READ_FAIL;

	out = PKTLA_TRAN_TO_NET(ate_ctrl->txbf_info_len / 4, ate_ctrl->txbf_info);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "val:%x %x, str:%s\n", idx, isBFer, cmd);
	NdisMoveMemory((PUCHAR)&HqaCmdFrame->Data + 2, (PUCHAR)out, ate_ctrl->txbf_info_len);
	ate_ctrl->op_mode &= ~fATE_IN_BF;
	os_free_mem(ate_ctrl->txbf_info);
	ate_ctrl->txbf_info = NULL;
HQA_TAG_READ_FAIL:

	if (cmd)
		os_free_mem(cmd);

	ResponseToQA(HqaCmdFrame, WRQ, 2 + ate_ctrl->txbf_info_len, Ret);
	return Ret;
}


static INT32 HQA_StaRecCmmUpdate(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT16 len = 0;
	struct _HQA_BF_STA_CMM_REC tmp, *rec = NULL;
	RTMP_STRING *cmd;
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _ATE_PFMU_INFO *pfmu_info = NULL;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto err0;
	}

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);

	if (len > sizeof(tmp)) {
		Ret = NDIS_STATUS_INVALID_DATA;
		goto err0;
	}

	NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
	rec = PKTLA_TRAN_TO_HOST((len - MAC_ADDR_LEN) / 4, &tmp);

	if (rec->wlan_idx > ATE_BFMU_NUM) {
		Ret = NDIS_STATUS_INVALID_DATA;
		goto err0;
	}

	pfmu_info = &ate_ctrl->pfmu_info[rec->wlan_idx - 1];
	pfmu_info->wcid = rec->wlan_idx;
	pfmu_info->bss_idx = rec->bss_idx;
	NdisMoveMemory(pfmu_info->addr, rec->mac, MAC_ADDR_LEN);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", rec->wlan_idx, rec->bss_idx, rec->aid, PRINT_MAC(rec->mac));
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_StaRecCmmUpdate(pAd, cmd);
#endif
err0:

	if (cmd)
		os_free_mem(cmd);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	if (rec)
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "wlan_idx:%x, bss_idx:%x, aid:%x, mac:%02x:%02x:%02x:%02x:%02x:%02x\n",
				  rec->wlan_idx, rec->bss_idx, rec->aid, PRINT_MAC(rec->mac));

	return Ret;
}


static INT32 HQA_StaRecBfUpdate(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT16 len = 0;
	struct _HQA_BF_STA_REC tmp, *rec = NULL;
	RTMP_STRING *cmd;

	if (!HqaCmdFrame) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "HqaCmdFrame is NULL\n");
		Ret = NDIS_STATUS_INVALID_DATA;
		return Ret;
	}

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_STAREC_BF_UPDATE_FAIL;
	}

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);

	if (len > sizeof(tmp)) {
		Ret = NDIS_STATUS_INVALID_DATA;
		goto HQA_STAREC_BF_UPDATE_FAIL;
	}

	NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
	rec = PKTLA_TRAN_TO_HOST((len) / 4, &tmp);
	PKTLA_DUMP(DBG_LVL_INFO, sizeof(*rec) / 4, rec);	/* Del after debug */
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
			rec->wlan_idx, rec->bss_idx, rec->PfmuId, rec->su_mu, rec->etxbf_cap, rec->ndpa_rate, rec->ndp_rate, rec->report_poll_rate,
			rec->tx_mode, rec->nc, rec->nr, rec->cbw, rec->spe_idx, rec->tot_mem_req, rec->mem_req_20m, rec->mem_row0, rec->mem_col0,
			rec->mem_row1, rec->mem_col1, rec->mem_row2, rec->mem_col2, rec->mem_row3, rec->mem_col3);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}

#if defined(MT_MAC)
	Set_StaRecBfUpdate(pAd, cmd);
#endif
HQA_STAREC_BF_UPDATE_FAIL:

	if (cmd)
		os_free_mem(cmd);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	if (rec) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
				  rec->wlan_idx, rec->bss_idx, rec->PfmuId, rec->su_mu, rec->etxbf_cap, rec->ndpa_rate, rec->ndp_rate, rec->report_poll_rate,
				  rec->tx_mode, rec->nc, rec->nr, rec->cbw, rec->tot_mem_req, rec->mem_req_20m, rec->mem_row0, rec->mem_col0, rec->mem_row1,
				  rec->mem_col1, rec->mem_row2, rec->mem_col2, rec->mem_row3, rec->mem_col3);
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "wlan_idx:%x, bss_idx:%x\n", rec->wlan_idx, rec->bss_idx);
	}
	return Ret;
}


static INT32 HQA_BFProfileDataRead(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 idx = 0, fgBFer = 0, subcarrIdx = 0, subcarr_start = 0, subcarr_end = 0;
	UINT32 offset = 0;
	UINT32 NumOfsub = 0;
	UCHAR *SubIdx = NULL;
	UCHAR *out = NULL;
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	RTMP_STRING *cmd;
	INT debug_lvl = DebugLevel;

	ate_ctrl->txbf_info = NULL;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto BF_PROFILE_DATA_READ_FAIL;
	}

	ate_ctrl->op_mode |= fATE_IN_BF;
	NdisMoveMemory((PUCHAR)&idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	idx = PKTL_TRAN_TO_HOST(idx);
	NdisMoveMemory((PUCHAR)&fgBFer, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
	fgBFer = PKTL_TRAN_TO_HOST(fgBFer);
	NdisMoveMemory((PUCHAR)&subcarr_start, (PUCHAR)&HqaCmdFrame->Data + 4 + 4, 4);
	subcarr_start = PKTL_TRAN_TO_HOST(subcarr_start);
	NdisMoveMemory((PUCHAR)&subcarr_end, (PUCHAR)&HqaCmdFrame->Data + 4 + 4 + 4, 4);
	subcarr_end = PKTL_TRAN_TO_HOST(subcarr_end);
	NumOfsub = subcarr_end - subcarr_start + 1;
	NumOfsub = PKTL_TRAN_TO_HOST(NumOfsub);
	NdisMoveMemory((PUCHAR)&HqaCmdFrame->Data + 2, (PUCHAR)&NumOfsub, sizeof(NumOfsub));
	offset += sizeof(NumOfsub);
	DebugLevel = DBG_LVL_OFF;
	/* check the range for coverity */
	if (subcarr_end <= 65535) {
		for (subcarrIdx = subcarr_start; subcarrIdx <= subcarr_end; subcarrIdx++) {
			ate_ctrl->txbf_info = NULL;
			SubIdx = (UCHAR *)&subcarrIdx;
			memset(cmd, 0x00, HQA_BF_STR_SIZE);
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "idx:%02x fgBFer:%02x Sub_H:%02x Sub_L:%02x subidx:%d\n",
					  idx, fgBFer, SubIdx[1], SubIdx[0], subcarrIdx);
			snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x", idx, fgBFer, SubIdx[1], SubIdx[0]);
			if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"cmd snprintf error!\n");
				os_free_mem(cmd);
				return FALSE;
			}
#if defined(MT_MAC) || defined(MT7637)
			/* check the range for coverity */
			if (*cmd < HQA_BF_STR_SIZE) {
				Set_TxBfProfileDataRead(pAd, cmd);
			} else {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					"command length is wrong\n");
				Ret = NDIS_STATUS_INVALID_DATA;
			}
			if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&ate_ctrl->cmd_done, ate_ctrl->cmd_expire)) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"wait cmd timeout!\n");
			}
#endif

			if (!ate_ctrl->txbf_info)
				goto BF_PROFILE_DATA_READ_FAIL;

			out = PKTLA_TRAN_TO_NET(ate_ctrl->txbf_info_len / 4, ate_ctrl->txbf_info);
			NdisMoveMemory((PUCHAR)&HqaCmdFrame->Data + 2 + offset, (PUCHAR)out, ate_ctrl->txbf_info_len);
			offset += ate_ctrl->txbf_info_len;
			os_free_mem(ate_ctrl->txbf_info);
			ate_ctrl->txbf_info = NULL;
		}
	}
	ate_ctrl->op_mode &= ~fATE_IN_BF;
BF_PROFILE_DATA_READ_FAIL:

	if (cmd)
		os_free_mem(cmd);

	ResponseToQA(HqaCmdFrame, WRQ, 2 + offset, Ret);
	DebugLevel = debug_lvl;
	return Ret;
}

static INT32 HQA_BFProfileDataWrite(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT16 len = 0;
	INT debug_lvl = DebugLevel;
	struct _HQA_BF_STA_PROFILE tmp, *profile;
	RTMP_STRING *cmd;
	UINT cmd_left = 0;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto BF_PROFILE_DATA_WRITE_FAIL;
	}

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	if (len > sizeof(struct _HQA_BF_STA_PROFILE)) {
		len = sizeof(struct _HQA_BF_STA_PROFILE);

		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"command length is wrong\n");
	}
	NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
	profile = PKTLA_TRAN_TO_HOST((len) / sizeof(UINT32), &tmp);
	DebugLevel = DBG_LVL_OFF;
	PKTLA_DUMP(DBG_LVL_INFO, sizeof(tmp) / sizeof(UINT32), &tmp);	/* Del after debug */
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:", profile->pfmuid);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%03x:", profile->subcarrier);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%03x:", profile->phi11);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%02x:", profile->psi21);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%03x:", profile->phi21);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%02x:", profile->psi31);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%03x:", profile->phi31);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%02x:", profile->psi41);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%03x:", profile->phi22);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%02x:", profile->psi32);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%03x:", profile->phi32);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%02x:", profile->psi42);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%03x:", profile->phi33);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%02x:", profile->psi43);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%02x:", profile->snr00);
	if (os_snprintf_error(HQA_BF_STR_SIZE, (strlen(cmd) + snprintf_ret))) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%02x:", profile->snr01);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%02x:", profile->snr02);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	cmd_left = HQA_BF_STR_SIZE - strlen(cmd);
	snprintf_ret = snprintf(cmd + strlen(cmd), cmd_left, "%02x", profile->snr03);
	if (os_snprintf_error(cmd_left, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	Set_TxBfProfileDataWrite(pAd, cmd);
#endif
	os_free_mem(cmd);
BF_PROFILE_DATA_WRITE_FAIL:
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	DebugLevel = debug_lvl;
	return Ret;
}

static INT32 HQA_BFQdRead(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	UCHAR *out = NULL;
	RTMP_STRING *cmd;
	INT32 subcarrier_idx = 0;

	ate_ctrl->txbf_info = NULL;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_BF_QD_READ_FAIL;
	}

	ate_ctrl->op_mode |= fATE_IN_BF;

	NdisMoveMemory((PUCHAR)&subcarrier_idx, HqaCmdFrame->Data, sizeof(subcarrier_idx));
	subcarrier_idx = PKTL_TRAN_TO_HOST(subcarrier_idx);

	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", subcarrier_idx);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "val:%u, str:%s\n", subcarrier_idx, cmd);
#if defined(MT_MAC)
	Set_TxBfQdRead(pAd, cmd);
	if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&ate_ctrl->cmd_done, ate_ctrl->cmd_expire)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "wait cmd timeout!\n");
	}
#endif
	if (!ate_ctrl->txbf_info)
		goto HQA_BF_QD_READ_FAIL;

	out = (PUCHAR)ate_ctrl->txbf_info;

	PKTLA_DUMP(DBG_LVL_INFO, 14, out);
	NdisMoveMemory((PUCHAR)&HqaCmdFrame->Data + 2, (PUCHAR)out, ate_ctrl->txbf_info_len);
	ate_ctrl->op_mode &= ~fATE_IN_BF;
	os_free_mem(ate_ctrl->txbf_info);
	ate_ctrl->txbf_info = NULL;
HQA_BF_QD_READ_FAIL:
	if (cmd)
		os_free_mem(cmd);

	ResponseToQA(HqaCmdFrame, WRQ, 2 + ate_ctrl->txbf_info_len, Ret);
	return Ret;
}

static INT32 HQA_BFSounding(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT16 len = 0;
	struct _HQA_BF_SOUNDING tmp, *param;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_BFSOUNDING_FAIL;
	}

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);

	if (len > sizeof(tmp)) {
		Ret = NDIS_STATUS_INVALID_DATA;
		goto HQA_BFSOUNDING_FAIL;
	}

	NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
	param = PKTLA_TRAN_TO_HOST((len) / sizeof(UINT32), &tmp);
	PKTLA_DUMP(DBG_LVL_INFO, sizeof(*param) / sizeof(UINT32), param);/* Del after debug */
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x:%02x", param->su_mu, param->mu_num, param->snd_interval, param->wlan_id0, param->wlan_id1, param->wlan_id2, param->wlan_id3);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
#if defined(MT_MAC)
	MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, param->band_idx);
	Set_Trigger_Sounding_Proc(pAd, cmd);
#endif
HQA_BFSOUNDING_FAIL:

	if (cmd)
		os_free_mem(cmd);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_TXBFSoundingStop(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

#if defined(MT_MAC)
	Set_Stop_Sounding_Proc(pAd, NULL);
#endif
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_TXBFProfileDataWriteAllExt(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 bw = 0;
	UINT32 profile_idx = 0;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto err0;
	}

	NdisMoveMemory((PUCHAR)&bw, (PUCHAR)&HqaCmdFrame->Data, sizeof(bw));
	NdisMoveMemory((PUCHAR)&profile_idx, (PUCHAR)&HqaCmdFrame->Data + sizeof(bw), sizeof(profile_idx));
	bw = PKTL_TRAN_TO_HOST(bw);
	profile_idx = PKTL_TRAN_TO_HOST(profile_idx);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%x:%x", profile_idx, bw);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
err0:

	if (cmd)
		os_free_mem(cmd);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "bw:%x, profile_idx:%x\n", bw, profile_idx);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


#ifdef MT_MAC
static INT32 HQA_TxBfTxApply(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	UINT32 eBF_enable = 0;
	UINT32 iBF_enable = 0;
	UINT32 wlan_id = 0;
	UINT32 MuTx_enable = 0;
	UINT32 iBFPhaseCali = 1;
	RTMP_STRING *cmd;
	UCHAR *data = HqaCmdFrame->Data;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		Ret = NDIS_STATUS_RESOURCES;
		goto err0;
	}

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&eBF_enable);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&iBF_enable);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&wlan_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&MuTx_enable);
	if (wlan_id < DBDC_BAND_NUM) {
		TESTMODE_SET_PARAM(pAd, wlan_id, ebf, eBF_enable);
		TESTMODE_SET_PARAM(pAd, wlan_id, ibf, iBF_enable);
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x:%02x", wlan_id, eBF_enable, iBF_enable, MuTx_enable, iBFPhaseCali);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 "wlan_id:%x, eBF enable:%x, iBF enable:%x, MuTx:%x\n",
				  wlan_id, eBF_enable, iBF_enable, MuTx_enable);
		/* check the range for coverity */
		if (*cmd < HQA_BF_STR_SIZE)
			Set_TxBfTxApply(pAd, cmd);
	} else {
		Ret = NDIS_STATUS_RESOURCES;
		goto err0;
	}
err0:

	if (cmd)
		os_free_mem(cmd);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_ManualAssoc(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT snprintf_ret;
	P_MANUAL_CONN manual_cfg = &pAd->AteManualConnInfo;
	struct _HQA_BF_MANUAL_CONN manual_conn = { {0} };
	RTMP_STRING rate_str[64];
	char ucNsts;
	UINT_32 rate[8];
	RA_PHY_CFG_T TxPhyCfg;
	UCHAR *data = HqaCmdFrame->Data;

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.type);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.wtbl_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.ownmac_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.phymode);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.bw);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.pfmuid);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.marate_mode);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.marate_mcs);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.spe_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.aid);
	EthGetParamAndShiftBuff(FALSE, MAC_ADDR_LEN, &data, (UCHAR *)manual_conn.mac);
	manual_cfg->peer_op_type = manual_conn.type;
	manual_cfg->wtbl_idx = manual_conn.wtbl_idx;
	manual_cfg->ownmac_idx = manual_conn.ownmac_idx;

	switch (manual_conn.phymode) {
	/* abggnanac */
	case 0:
		manual_cfg->peer_phy_mode = WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN | WMODE_AC;
		break;

	/* bggnan */
	case 1:
		manual_cfg->peer_phy_mode = WMODE_B | WMODE_GN | WMODE_G | WMODE_AN;
		break;

	/* aanac */
	case 2:
		manual_cfg->peer_phy_mode = WMODE_A | WMODE_AN | WMODE_AC;
		break;

	default:
		manual_cfg->peer_phy_mode = WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN | WMODE_AC;
		break;
	}

	switch (manual_conn.bw) {
	case ATE_BAND_WIDTH_20:
		manual_cfg->peer_bw = BAND_WIDTH_20;
		break;

	case ATE_BAND_WIDTH_40:
		manual_cfg->peer_bw = BAND_WIDTH_40;
		break;

	case ATE_BAND_WIDTH_80:
		manual_cfg->peer_bw = BAND_WIDTH_80;
		break;

	case ATE_BAND_WIDTH_10:
		manual_cfg->peer_bw = BAND_WIDTH_10;
		break;

	case ATE_BAND_WIDTH_5:
		manual_cfg->peer_bw = BAND_WIDTH_5;
		break;

	case ATE_BAND_WIDTH_160:
		manual_cfg->peer_bw = BAND_WIDTH_160;
		break;

	case ATE_BAND_WIDTH_8080:
		manual_cfg->peer_bw = BAND_WIDTH_8080;
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "Cannot find BW with manual_conn.bw:%x\n", manual_conn.bw);
		manual_cfg->peer_bw = manual_conn.bw;
		break;
	}

	manual_cfg->pfmuId = manual_conn.pfmuid;
	manual_cfg->peer_maxrate_mode = manual_conn.marate_mode;
	manual_cfg->peer_maxrate_mcs = manual_conn.marate_mcs;
	manual_cfg->spe_idx = manual_conn.spe_idx;
	manual_cfg->aid = manual_conn.aid;
	manual_cfg->peer_nss = 1; /* MU */
	NdisMoveMemory(manual_cfg->peer_mac, manual_conn.mac, MAC_ADDR_LEN);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "User manual configured peer STA info:\n");
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tMAC=>0x%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pAd->AteManualConnInfo.peer_mac));
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tBAND=>%d\n", pAd->AteManualConnInfo.peer_band);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tOwnMacIdx=>%d\n", pAd->AteManualConnInfo.ownmac_idx);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tWTBL_Idx=>%d\n", pAd->AteManualConnInfo.wtbl_idx);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tOperationType=>%d\n", pAd->AteManualConnInfo.peer_op_type);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tPhyMode=>%d\n", pAd->AteManualConnInfo.peer_phy_mode);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tBandWidth=>%d\n", pAd->AteManualConnInfo.peer_bw);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tNSS=>%d\n", pAd->AteManualConnInfo.peer_nss);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tPfmuId=>%d\n", pAd->AteManualConnInfo.pfmuId);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tAid=>%d\n", pAd->AteManualConnInfo.aid);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tSpe_idx=>%d\n", pAd->AteManualConnInfo.spe_idx);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tMaxRate_Mode=>%d\n", pAd->AteManualConnInfo.peer_maxrate_mode);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tMaxRate_MCS=>%d\n", pAd->AteManualConnInfo.peer_maxrate_mcs);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Now apply it to hardware!\n");
	/* This applied the manual config info into the mac table entry, including the HT/VHT cap, VHT MCS set */
	SetATEApplyStaToMacTblEntry(pAd);
	/* Fixed rate configuration */
	NdisZeroMemory(&rate_str[0], sizeof(rate_str));
	snprintf_ret = snprintf(rate_str, sizeof(rate_str), "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d",
			pAd->AteManualConnInfo.wtbl_idx,
			pAd->AteManualConnInfo.peer_maxrate_mode,
			pAd->AteManualConnInfo.peer_bw,
			pAd->AteManualConnInfo.peer_maxrate_mcs,
			pAd->AteManualConnInfo.peer_nss,
			0, 0, 0, 0, 0);
	if (os_snprintf_error(sizeof(rate_str), snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"rate_str snprintf error!\n");
		return FALSE;
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "\tSet fixed RateInfo string as %s\n", rate_str);
	/* Set_Fixed_Rate_Proc(pAd, rate_str); */
	ucNsts = asic_get_nsts_by_mcs(pAd, pAd->AteManualConnInfo.peer_maxrate_mode,
							 pAd->AteManualConnInfo.peer_maxrate_mcs,
							 FALSE,
							 pAd->AteManualConnInfo.peer_nss);
	rate[0] = asic_tx_rate_to_tmi_rate(pAd, pAd->AteManualConnInfo.peer_maxrate_mode,
								  pAd->AteManualConnInfo.peer_maxrate_mcs,
								  ucNsts,
								  FALSE,
								  0);
	rate[0] &= 0xfff;
	rate[1] = rate[2] = rate[3] = rate[4] = rate[5] = rate[6] = rate[7] = rate[0];
	os_zero_mem(&TxPhyCfg, sizeof(TxPhyCfg));
	TxPhyCfg.BW      = pAd->AteManualConnInfo.peer_bw;
	TxPhyCfg.ShortGI = FALSE;
	/* TxPhyCfg.ldpc  = HT_LDPC | VHT_LDPC; */
	TxPhyCfg.ldpc    = 0;
	AsicTxCapAndRateTableUpdate(pAd,
								  pAd->AteManualConnInfo.wtbl_idx,
								  &TxPhyCfg,
								  rate,
								  FALSE);
	/* WTBL configuration */
	SetATEApplyStaToAsic(pAd);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}
#endif

static HQA_CMD_HANDLER HQA_TXBF_CMDS[] = {
	HQA_TxBfProfileTagInValid,	/* 0x1540 */
	HQA_TxBfProfileTagPfmuIdx,	/* 0x1541 */
	HQA_TxBfProfileTagBfType,	/* 0x1542 */
	HQA_TxBfProfileTagBw,		/* 0x1543 */
	HQA_TxBfProfileTagSuMu,		/* 0x1544 */
	HQA_TxBfProfileTagMemAlloc,	/* 0x1545 */
	HQA_TxBfProfileTagMatrix,	/* 0x1546 */
	HQA_TxBfProfileTagSnr,		/* 0x1547 */
	HQA_TxBfProfileTagSmtAnt,	/* 0x1548 */
	HQA_TxBfProfileTagSeIdx,	/* 0x1549 */
	HQA_TxBfProfileTagRmsdThrd,	/* 0x154A */
	HQA_TxBfProfileTagMcsThrd,	/* 0x154B */
	HQA_TxBfProfileTagTimeOut,	/* 0x154C */
	HQA_TxBfProfileTagDesiredBw,	/* 0x154D */
	HQA_TxBfProfileTagDesiredNc,	/* 0x154E */
	HQA_TxBfProfileTagDesiredNr,	/* 0x154F */
	HQA_TxBfProfileTagWrite,	/* 0x1550 */
	HQA_TxBfProfileTagRead,		/* 0x1551 */
	HQA_StaRecCmmUpdate,		/* 0x1552 */
	HQA_StaRecBfUpdate,		/* 0x1553 */
	HQA_BFProfileDataRead,		/* 0x1554 */
	HQA_BFProfileDataWrite,		/* 0x1555 */
	HQA_BFSounding,			/* 0x1556 */
	HQA_TXBFSoundingStop,		/* 0x1557 */
	HQA_TXBFProfileDataWriteAllExt, /* 0x1558 */
#ifdef MT_MAC
	HQA_TxBfTxApply,		/* 0x1559 */
	HQA_ManualAssoc,		/* 0x155A */
#endif
	HQA_BFQdRead			/* 0x155B */
};

#ifdef CFG_SUPPORT_MU_MIMO
static INT32 HQA_MUGetInitMCS(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 gid = 0;
	MU_STRUCT_MU_GROUP_INIT_MCS init_mcs;
	struct _HQA_MU_USR_INIT_MCS out;

	NdisMoveMemory((UCHAR *)&gid, HqaCmdFrame->Data, sizeof(gid));
	gid = PKTS_TRAN_TO_HOST(gid);
	os_zero_mem(&init_mcs, sizeof(init_mcs));
	os_zero_mem(&out, sizeof(out));
	Ret = hqa_wifi_test_mu_get_init_mcs(pAd, gid, &init_mcs);
	out.user0 = init_mcs.user0InitMCS;
	out.user1 = init_mcs.user1InitMCS;
	out.user2 = init_mcs.user2InitMCS;
	out.user3 = init_mcs.user3InitMCS;
	NdisMoveMemory(HqaCmdFrame->Data + 2, (UCHAR *)&out, sizeof(out));
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "gid:%u\n", gid);
	ResponseToQA(HqaCmdFrame, WRQ, 2 + sizeof(init_mcs), Ret);
	return Ret;
}


static INT32 HQA_MUCalInitMCS(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	/* INT32 gid = 0; */
	MU_STRUCT_SET_CALC_INIT_MCS param;
	UCHAR *data = HqaCmdFrame->Data;

	os_zero_mem(&param, sizeof(param));
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param.num_of_user);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param.bandwidth);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param.nss_of_user0);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param.nss_of_user1);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param.nss_of_user2);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param.nss_of_user3);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param.pf_mu_id_of_user0);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param.pf_mu_id_of_user1);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param.pf_mu_id_of_user2);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param.pf_mu_id_of_user3);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param.num_of_txer);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param.spe_index);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT16), &data, (UCHAR *)&param.group_index);
	Ret = hqa_wifi_test_mu_cal_init_mcs(pAd, &param);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "spe_idx:%d\n", param.spe_index);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_MUCalLQ(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Type = 0;
	INT32 gid = 0;
	UINT32 txer = 0;
	UCHAR *data = HqaCmdFrame->Data;
	MU_STRUCT_SET_SU_CALC_LQ param_su;
	MU_STRUCT_SET_CALC_LQ param_mu;

	os_zero_mem(&param_su, sizeof(param_su));
	os_zero_mem(&param_mu, sizeof(param_mu));
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Type);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param_mu.num_of_user);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param_mu.bandwidth);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param_mu.nss_of_user0);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param_mu.nss_of_user1);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param_mu.nss_of_user2);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param_mu.nss_of_user3);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param_mu.pf_mu_id_of_user0);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param_mu.pf_mu_id_of_user1);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param_mu.pf_mu_id_of_user2);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param_mu.pf_mu_id_of_user3);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param_mu.num_of_txer);
	EthGetParamAndShiftBuff(FALSE, sizeof(UINT8), &data, (UCHAR *)&param_mu.spe_index);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT16), &data, (UCHAR *)&param_mu.group_index);
	param_su.num_of_user = param_mu.num_of_user;
	param_su.bandwidth = param_mu.bandwidth;
	param_su.nss_of_user0 = param_mu.nss_of_user0;
	param_su.pf_mu_id_of_user0 = param_mu.pf_mu_id_of_user0;
	param_su.num_of_txer = param_mu.num_of_txer;
	param_su.spe_index = param_mu.spe_index;
	param_su.group_index = param_mu.group_index;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "gid:%u, txer:%u spe_idx:%d\n", gid, txer, param_mu.spe_index);

	if (Type == 0)
		Ret = hqa_wifi_test_su_cal_lq(pAd, &param_su);
	else
		Ret = hqa_wifi_test_mu_cal_lq(pAd, &param_mu);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_MUGetLQ(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Type = 0;
	UCHAR *data = HqaCmdFrame->Data;
	SU_STRUCT_LQ_REPORT lq_su;
	MU_STRUCT_LQ_REPORT lq_mu;

	os_zero_mem(&lq_su, sizeof(lq_su));
	os_zero_mem(&lq_mu, sizeof(lq_mu));
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Type);

	/* TODO:: Check if structure changed*/
	if (Type == 0) {
		Ret = hqa_wifi_test_su_get_lq(pAd, &lq_su);
		PKTLA_DUMP(DBG_LVL_INFO, sizeof(lq_su) / sizeof(int), &lq_su);
		NdisMoveMemory(HqaCmdFrame->Data + 2, (UCHAR *)&lq_su, sizeof(lq_su));
		ResponseToQA(HqaCmdFrame, WRQ, 2 + sizeof(lq_su), Ret);
	} else {
		Ret = hqa_wifi_test_mu_get_lq(pAd, &lq_mu);
		PKTLA_DUMP(DBG_LVL_INFO, sizeof(lq_mu) / sizeof(int), &lq_mu);
		NdisMoveMemory(HqaCmdFrame->Data + 2, (UCHAR *)&lq_mu, sizeof(lq_mu));
		ResponseToQA(HqaCmdFrame, WRQ, 2 + sizeof(lq_mu), Ret);
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	return Ret;
}


static INT32 HQA_MUSetSNROffset(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 offset = 0;

	NdisMoveMemory((UCHAR *)&offset, HqaCmdFrame->Data, sizeof(offset));
	offset = PKTL_TRAN_TO_HOST(offset);
	Ret = hqa_wifi_test_snr_offset_set(pAd, (UINT8)offset);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "offset:%x\n", offset);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_MUSetZeroNss(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 zero_nss = 0;

	NdisMoveMemory((UCHAR *)&zero_nss, HqaCmdFrame->Data, sizeof(zero_nss));
	zero_nss = PKTL_TRAN_TO_HOST(zero_nss);
	Ret = hqa_wifi_test_mu_set_zero_nss(pAd, (UINT8)zero_nss);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "zero_nss:%x\n", zero_nss);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_MUSetSpeedUpLQ(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 spdup_lq = 0;

	NdisMoveMemory((UCHAR *)&spdup_lq, HqaCmdFrame->Data, sizeof(spdup_lq));
	spdup_lq = PKTL_TRAN_TO_HOST(spdup_lq);
	Ret = hqa_wifi_test_mu_speed_up_lq(pAd, spdup_lq);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "spdup_lq:%x\n", spdup_lq);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_MUSetMUTable(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	UCHAR *tbl = NULL;
	UINT32 su_mu = 0;
	MU_STRUCT_MU_TABLE info;

	os_zero_mem(&info, sizeof(info));
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length) - sizeof(su_mu);
	NdisMoveMemory((UCHAR *)&su_mu, HqaCmdFrame->Data, sizeof(su_mu));
	su_mu = PKTL_TRAN_TO_HOST(su_mu);
	/* check the range for coverity */
	if (len == 0 || len >= 2044) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "len==0 or len >=2044 which cost tbl overrun\n");
		Ret = TM_STATUS_NOTSUPPORT;
		return Ret;
	}

	Ret = os_alloc_mem(pAd, (UCHAR **)&tbl, sizeof(UCHAR) * len);

	if (Ret)
		goto err0;

	os_zero_mem(tbl, len);
	NdisMoveMemory((UCHAR *)tbl, HqaCmdFrame->Data + sizeof(su_mu), len);
	info.type = su_mu;
	info.length = len;
	info.prTable = tbl;
	Ret = hqa_wifi_test_mu_table_set(pAd, &info);
err0:
	if (tbl != NULL)
		os_free_mem(tbl);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "len:%u, su_mu:%u\n", len, su_mu);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_MUSetGroup(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	UINT32 val32;
	UCHAR *data = HqaCmdFrame->Data;
	MU_STRUCT_MU_GROUP grp;

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.groupIndex = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.numOfUser = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user0Ldpc = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user1Ldpc = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user2Ldpc = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user3Ldpc = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.shortGI = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.bw = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user0Nss = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user1Nss = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user2Nss = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user3Nss = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.groupId = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user0UP = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user1UP = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user2UP = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user3UP = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user0MuPfId = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user1MuPfId = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user2MuPfId = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user3MuPfId = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user0InitMCS = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user1InitMCS = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user2InitMCS = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user3InitMCS = val32;
	NdisMoveMemory(grp.aucUser0MacAddr, data, MAC_ADDR_LEN);
	data += MAC_ADDR_LEN;
	NdisMoveMemory(grp.aucUser1MacAddr, data, MAC_ADDR_LEN);
	data += MAC_ADDR_LEN;
	NdisMoveMemory(grp.aucUser2MacAddr, data, MAC_ADDR_LEN);
	data += MAC_ADDR_LEN;
	NdisMoveMemory(grp.aucUser3MacAddr, data, MAC_ADDR_LEN);
	Ret = hqa_wifi_test_mu_group_set(pAd, &grp);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "len:%x\n", len);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "0:%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(grp.aucUser0MacAddr));
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "1:%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(grp.aucUser1MacAddr));
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "2:%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(grp.aucUser2MacAddr));
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "3:%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(grp.aucUser3MacAddr));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_MUGetQD(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT debug_lvl = DebugLevel;
	INT32 subcarrier_idx = 0;
	MU_STRUCT_MU_QD qd;

	NdisMoveMemory((PUCHAR)&subcarrier_idx, HqaCmdFrame->Data, sizeof(subcarrier_idx));
	NdisZeroMemory(&qd, sizeof(qd));
	subcarrier_idx = PKTL_TRAN_TO_HOST(subcarrier_idx);
	DebugLevel = DBG_LVL_OFF;
	Ret = hqa_wifi_test_mu_get_qd(pAd, subcarrier_idx, &qd);
	PKTLA_DUMP(DBG_LVL_INFO, sizeof(qd) / sizeof(int), &qd);
	NdisMoveMemory(HqaCmdFrame->Data + 2, (UCHAR *)&qd, sizeof(qd));
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "sub:%u, qd.length:%zu, pqd:%p, HqaCmd->Data:%p\n",
			  subcarrier_idx, sizeof(qd), &qd, HqaCmdFrame->Data);
	ResponseToQA(HqaCmdFrame, WRQ, 2 + sizeof(qd), Ret);
	DebugLevel = debug_lvl;
	return Ret;
}


static INT32 HQA_MUSetEnable(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 is_enable = 0;
	/* struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl); */
	NdisMoveMemory((PUCHAR)&is_enable, HqaCmdFrame->Data, sizeof(is_enable));
	is_enable = PKTL_TRAN_TO_HOST(is_enable);
	Ret = hqa_wifi_test_mu_set_enable(pAd, (BOOLEAN)is_enable);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "MU is_enable:%x\n", is_enable);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MUSetGID_UP(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	UCHAR *data = HqaCmdFrame->Data;
	UINT32 val32 = 0;
	INT i = 0;
	MU_STRUCT_MU_STA_PARAM param;

	os_zero_mem(&param, sizeof(param));
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);

	for (i = 0; i < 2; i++) {
		NdisMoveMemory(&val32, data, sizeof(val32));
		val32 = PKTL_TRAN_TO_HOST(val32);
		param.gid[i] = val32;
		data += sizeof(val32);
	}

	for (i = 0; i < 4; i++) {
		NdisMoveMemory(&val32, data, sizeof(val32));
		val32 = PKTL_TRAN_TO_HOST(val32);
		param.up[i] = val32;
		data += sizeof(val32);
	}

	/* Del after debug */
	PKTLA_DUMP(DBG_LVL_INFO, sizeof(param) / sizeof(UINT32), &param);
	Ret = hqa_wifi_test_mu_set_sta_gid_and_up(pAd, &param);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32  HQA_MUTriggerTx(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	struct _ATE_CTRL *ate_ctrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UINT16 len = 0;
	UINT32 val32;
	UINT32 band_idx = 0;
	UCHAR *data = HqaCmdFrame->Data;
	MU_STRUCT_TRIGGER_MU_TX_FRAME_PARAM mu_tx_param;

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	band_idx = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	mu_tx_param.fgIsRandomPattern = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	mu_tx_param.msduPayloadLength0 = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	mu_tx_param.msduPayloadLength1 = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	mu_tx_param.msduPayloadLength2 = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	mu_tx_param.msduPayloadLength3 = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	mu_tx_param.u4MuPacketCount = val32;
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	mu_tx_param.u4NumOfSTAs = val32;

	ate_ctrl->control_band_idx = (UCHAR)band_idx;

	Ret = hqa_wifi_test_mu_trigger_mu_tx(pAd, &mu_tx_param);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "len:%x\n", len);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static HQA_CMD_HANDLER HQA_TXMU_CMDS[] = {
	HQA_MUGetInitMCS,			/* 0x1560 */
	HQA_MUCalInitMCS,			/* 0x1561 */
	HQA_MUCalLQ,				/* 0x1562 */
	HQA_MUGetLQ,				/* 0x1563 */
	HQA_MUSetSNROffset,			/* 0x1564 */
	HQA_MUSetZeroNss,			/* 0x1565 */
	HQA_MUSetSpeedUpLQ,			/* 0x1566 */
	HQA_MUSetMUTable,			/* 0x1567 */
	HQA_MUSetGroup,				/* 0x1568 */
	HQA_MUGetQD,				/* 0x1569 */
	HQA_MUSetEnable,			/* 0x156A */
	HQA_MUSetGID_UP,			/* 0x156B */
	HQA_MUTriggerTx,			/* 0x156C */
};
#endif /* CFG_SUPPORT_MU_MIMO */
#endif /* TXBF_SUPPORT */


static INT32 HQA_CapWiFiSpectrum(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
#ifdef INTERNAL_CAPTURE_SUPPORT
	/* Set Param*/
	UINT32 Control = 0, Trigger = 0, RingCapEn = 0, Event = 0, Node = 0, Len = 0;
	UINT32 StopCycle = 0, BW = 0, PdEnable = 0, Band = 0;
	UINT32 CapSource = 0, PhyIdx = 0, FixRxGain = 0, WifiPath = 0;
	UCHAR SourceAddress[MAC_ADDR_LEN] = {0};
	RBIST_CAP_START_T *prICapInfo = NULL;
	/* Get Param*/
	UINT32 Value, i;
	UINT32 RespLen = 2;
	UINT32 WF_Num = 0, IQ_Type = 0;
	PINT32 pData = NULL;
	PINT32 pDataLen = NULL;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data;

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Control);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Control:%d\n", Control);

	if (Control == 1) {
		if (ATEOp->SetICapStart) {
			{
				INT32 retval;

				/* Dynamic allocate memory for prRBISTInfo */
				retval = os_alloc_mem(pAd, (UCHAR **)&prICapInfo, sizeof(RBIST_CAP_START_T));
				if (retval != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							"Not enough memory for dynamic allocating !!\n");
					goto error1;
				}
				os_zero_mem(prICapInfo, sizeof(RBIST_CAP_START_T));

				EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Trigger);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Trigger:%d\n", Trigger);
				EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&RingCapEn);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "RingCapEn:%d\n", RingCapEn);
				EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Event);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Event:%d\n", Event);
				EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Node);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Node:%d\n", Node);
				EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Len);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Len:%d\n", Len);
				EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&StopCycle);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "StopCycle:%d\n", StopCycle);
				EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&BW);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "BW:%d\n", BW);
				EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&PdEnable);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PdEnable:%d\n", PdEnable);
				EthGetParamAndShiftBuff(FALSE, MAC_ADDR_LEN, &data, (UCHAR *)SourceAddress);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "SourceAddress:%02x:%02x:%02x:%02x:%02x:%02x\n ", PRINT_MAC(SourceAddress));
				EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Band);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Band:%d\n", Band);
				EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&PhyIdx);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "PhyIdx:%d\n", PhyIdx);
				EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&CapSource);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "CapSource:%d\n", CapSource);
				EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&FixRxGain);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "FixRxGain:%d\n", FixRxGain);
				EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&WifiPath);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WifiPath:%d\n", WifiPath);


				prICapInfo->fgTrigger = Trigger;
				prICapInfo->fgRingCapEn = RingCapEn;
				prICapInfo->u4TriggerEvent = Event;
				prICapInfo->u4CaptureNode = Node;
				prICapInfo->u4CaptureLen = Len;
				prICapInfo->u4CapStopCycle = StopCycle;
				prICapInfo->u4BW = BW;
				prICapInfo->u4PdEnable = PdEnable;
				prICapInfo->u4BandIdx = Band;
				prICapInfo->u4PhyIdx = PhyIdx;
				prICapInfo->u4CapSource = CapSource;
				prICapInfo->u4FixRxGain = FixRxGain;
				prICapInfo->u4WifiPath = WifiPath;
				ATEOp->SetICapStart(pAd, (UINT8 *)prICapInfo);
error1:
				if (prICapInfo != NULL)
					os_free_mem(prICapInfo);
			}
		} else
			Ret = TM_STATUS_NOTSUPPORT;

		ResponseToQA(HqaCmdFrame, WRQ, RespLen, Ret);
	} else if (Control == 2) {
		if (ATEOp->GetICapStatus) {
			Ret = ATEOp->GetICapStatus(pAd);

			if (IS_MT7615(pAd)) {
				EXT_EVENT_RBIST_ADDR_T *icap_info = &(pAd->ATECtrl.icap_info);

				data = HqaCmdFrame->Data;
				Value = PKTL_TRAN_TO_HOST(icap_info->u4StartAddr1);
				NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
				RespLen += sizeof(Value);
				Value = PKTL_TRAN_TO_HOST(icap_info->u4StartAddr2);
				NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
				RespLen += sizeof(Value);
				Value = PKTL_TRAN_TO_HOST(icap_info->u4StartAddr3);
				NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
				RespLen += sizeof(Value);
				Value = PKTL_TRAN_TO_HOST(icap_info->u4EndAddr);
				NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
				RespLen += sizeof(Value);
				Value = PKTL_TRAN_TO_HOST(icap_info->u4StopAddr);
				NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
				RespLen += sizeof(Value);
				Value = PKTL_TRAN_TO_HOST(icap_info->u4Wrap);
				NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
				RespLen += sizeof(Value);
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"StartAddr1:%02x StartAddr2:%02x StartAddr3:%02x EndAddr:%02x StopAddr:%02x Wrap:%02x\n",
						icap_info->u4StartAddr1, icap_info->u4StartAddr2, icap_info->u4StartAddr3,
						icap_info->u4EndAddr, icap_info->u4StopAddr, icap_info->u4Wrap);
			} else {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"Status = %d", Ret);
			}
		} else
			Ret = TM_STATUS_NOTSUPPORT;

		ResponseToQA(HqaCmdFrame, WRQ, RespLen, Ret);
	} else if (Control == 3) {
		if (ATEOp->GetICapIQData) {
			EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&WF_Num);
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "WF_Num:%d\n", WF_Num);
			EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&IQ_Type);
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "IQ_Type:%d\n", IQ_Type);
		   {
    			UINT32 Len;
    			INT32 retval;
    			UCHAR *data = HqaCmdFrame->Data;

				/* Dynamic allocate memory for data length */
				retval = os_alloc_mem(pAd, (UCHAR **)&pDataLen, sizeof(INT32));
				if (retval != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Not enough memory for dynamic allocating !!\n");
						goto error2;
				}
				os_zero_mem(pDataLen, sizeof(INT32));

				/* Dynamic allocate memory for 1KByte data buffer */
				Len = ICAP_EVENT_DATA_SAMPLE * sizeof(INT32);
				retval = os_alloc_mem(pAd, (UCHAR **)&pData, Len);
				if (retval != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 "Not enough memory for dynamic allocating !!\n");
						goto error2;
				}
				os_zero_mem(pData, ICAP_EVENT_DATA_SAMPLE * sizeof(INT32));

				ATEOp->GetICapIQData(pAd, pData, pDataLen, IQ_Type, WF_Num);
				Value = PKTL_TRAN_TO_HOST(Control);
				NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
				RespLen += sizeof(Value);
				Value = PKTL_TRAN_TO_HOST(WF_Num);
				NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
				RespLen += sizeof(Value);
				Value = PKTL_TRAN_TO_HOST(IQ_Type);
				NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
				RespLen += sizeof(Value);
				Value = PKTL_TRAN_TO_HOST(*pDataLen);
				NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
				RespLen += sizeof(Value);

				for (i = 0; i < *pDataLen; i++) {
					INT32 Value;

					Value = PKTL_TRAN_TO_HOST(pData[i]);
					NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
					RespLen += sizeof(Value);
				}
	error2:
				if (pDataLen != NULL)
					os_free_mem(pDataLen);
				if (pData != NULL)
					os_free_mem(pData);
			}
		} else
			Ret = TM_STATUS_NOTSUPPORT;

		ResponseToQA(HqaCmdFrame, WRQ, RespLen, Ret);
	}
#endif /* INTERNAL_CAPTURE_SUPPORT */

	return Ret;
}

/* Command 0x1581 */
INT32 HQA_GetDumpRecal(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
#define MAX_RECAL_DUMP_SIZE 100
	static INT32 idx;
	INT32  i4Ret = 0;
	UINT32 u4BufferCounter = 0;
	UINT32 count_total = 0;
	UINT32 BandIdx;
	INT32 end = 0, remindIdx = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_LOG_DUMP_CB *log_cb = &(ATECtrl->log_dump[ATE_LOG_RE_CAL-1]);

	/* Get HW band from cmd */
	/* HQA cmd frame: cmd type + cmd ID + length + Sequence + Data (Band0/Band1) */
	/* Data is 4 bytes */
	memcpy((PUCHAR)&BandIdx, (PUCHAR)&HqaCmdFrame->Data, 4);
	BandIdx = PKTL_TRAN_TO_HOST(BandIdx);

	/* Prepare for RECAL dump */
	OS_SPIN_LOCK(&log_cb->lock);
	log_cb->is_dumping = TRUE;
	OS_SPIN_UNLOCK(&log_cb->lock);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"[RECAL DUMP START][HQA_GetDumpRecal]\n");
	pAd->fgDumpStart = 1;

	/*Get Recal dump from log dump*/
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"log_cb->idx is %d, log_cb->len is %d\n", log_cb->idx, log_cb->len);

	if ((ATECtrl->firstReCal == TRUE) || (ATECtrl->firstQATool == TRUE)) {
		/*
		 * Reset idx - 1. HQA Re-cal dump (re-)enable
		 * Reset idx - 2. HQA Tool (re-)open
		 */
		idx = 0;
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "idx: %d\n", idx);
	/* If log_cb->idx greater than log_cb->len(6000), it will re-count from 0 */
	remindIdx = (idx > log_cb->idx) ?
		((log_cb->idx + log_cb->len) - idx) :
		(log_cb->idx - idx);

	end = (remindIdx > MAX_RECAL_DUMP_SIZE) ?
		((idx + MAX_RECAL_DUMP_SIZE) % (log_cb->len)) :
		((idx + remindIdx) % (log_cb->len));

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"remindIdx: %d, end: %d\n", remindIdx, end);

	do {
		idx = (idx % (log_cb->len));
		if (log_cb->entry[idx].un_dumped) {
			struct _ATE_LOG_RECAL re_cal = log_cb->entry[idx].log.re_cal;
			UINT32 cal_type = re_cal.cal_type;
			UINT32 cr_addr = re_cal.cr_addr;
			UINT32 cr_val = re_cal.cr_val;

			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"[Recal][%08x][%08x]%08x\n", cal_type, cr_addr, cr_val);
			log_cb->entry[idx].un_dumped = FALSE;
			u4BufferCounter++;

			/*Reverse the ordering of the four bytes in a 32-bit unsigned integer value*/
			cal_type = PKTL_TRAN_TO_HOST(cal_type);
			cr_addr = PKTL_TRAN_TO_HOST(cr_addr);
			cr_val = PKTL_TRAN_TO_HOST(cr_val);

			/* Response format:
			 * cmd type + cmd ID + length + Sequence + Data
			 * (status + count + cal_type + cr_addr + cr_val) */
			/*Data -> {status (2 bytes) + count (4 bytes) = 6 bytes} + [cal_type (4 bytes)] */
			memcpy(HqaCmdFrame->Data + 6 + count_total*4, &cal_type, 4);
			count_total++;
			/*Data -> {status (2 bytes) + count (4 bytes) = 6 bytes} + [cr_addr (4 bytes)] */
			memcpy(HqaCmdFrame->Data + 6 + count_total*4, &cr_addr, 4);
			count_total++;
			/*Data -> {status (2 bytes) + count (4 bytes) = 6 bytes} + [cr_val (4 bytes)] */
			memcpy(HqaCmdFrame->Data + 6 + count_total*4, &cr_val, 4);
			count_total++;
		}

		/* The size of per entry is 38 bytes and for QAtool log buffer limitation. */
		if ((pAd->fgQAtoolBatchDumpSupport) &&
			(u4BufferCounter >= (1 << (CONFIG_LOG_BUF_SHIFT - 1)) / 38)) {
			pAd->u2LogEntryIdx = idx;
			break;
		}

		INC_RING_INDEX(idx, log_cb->len);
	} while (idx != end);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"[After dumping] idx: %d, end: %d\n", idx, end);

	/* RECAL DUMP END */
	/* Count = Total number of 4 bytes RECAL values divided by 3 */
	count_total = count_total / 3;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"[HQA_GetDumpRecal]: Total Count As a Group of three = %d\n", count_total);
	if ((idx == log_cb->idx) && (pAd->fgDumpStart)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[RECAL DUMP END]\n");
		pAd->fgDumpStart = 0;
	}

	/* Send RECAL Dump to HQA tool */
	if (count_total > 0) {
		/*Response format: cmd type + cmd ID + length + Sequence + Data (status + count + RecalID + Offset + Value)*/
		/*Data -> status (2 bytes) + [count (4 bytes)] + RecalID (4 bytes) + Offset (4 bytes) + Value(4 bytes)*/
		count_total = PKTL_TRAN_TO_HOST(count_total);
		memcpy(HqaCmdFrame->Data + 2, &count_total, 4);
		count_total = PKTL_TRAN_TO_HOST(count_total);
		/*Data -> status (2 bytes) + count (4 bytes) + [RecalID (4 bytes) + Offset (4 bytes) + Value(4 bytes)]*count_total */
		ResponseToQA(HqaCmdFrame, WRQ, 6 + count_total*12, i4Ret);
	} else {
		/*Data -> [status (2 bytes)] + count (4 bytes) + RecalID (4 bytes) + Offset (4 bytes) + Value(4 bytes) */
		memcpy(HqaCmdFrame->Data + 2, &count_total, 4);
		ResponseToQA(HqaCmdFrame, WRQ, 6, i4Ret);
		OS_SPIN_LOCK(&log_cb->lock);
		log_cb->is_dumping = FALSE;
		OS_SPIN_UNLOCK(&log_cb->lock);

		if (ATECtrl->reCalInDumpSts == DISABLE_RECAL_IN_DUMP) {
			struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "LogOnOff, log_type:%d, log_ctrl:%d, log_size:%d\n",
					  ATECtrl->logStsInDump[0],
					  ATECtrl->logStsInDump[1], ATECtrl->logStsInDump[2]);
			ATEOp->LogOnOff(pAd, ATECtrl->logStsInDump[0],
				ATECtrl->logStsInDump[1], ATECtrl->logStsInDump[2]);
			ATECtrl->logStsInDump[0] = 0;
			ATECtrl->logStsInDump[1] = 0;
			ATECtrl->logStsInDump[2] = 0;

			ATECtrl->reCalInDumpSts = NO_CHANGE_RECAL;
			/*
			 * Reset idx - 3. HQA Re-cal dump disable during dumping
			 */
			idx = 0;
		}
	}

	ATECtrl->firstReCal     = FALSE;
	ATECtrl->firstQATool    = FALSE;
#undef MAX_RECAL_DUMP_SIZE
	return STATUS_SUCCESS;
}

/* Command 0x1583*/
INT32 HQA_GetDumpRDD(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	static INT32 idx;
	INT32 i4Ret = 0;
	UINT32 u4BufferCounter = 0;
	UINT32 count_total = 0;
	UINT32 BandIdx;
	INT32 end = 0, remindIdx = 0;

	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_LOG_DUMP_CB *log_cb = &(ATECtrl->log_dump[ATE_LOG_RDD-1]);

	/* Get HW band from cmd */
	/* HQA cmd frame: cmd type + cmd ID + length + Sequence + Data (Band0/Band1) */
	/* Data is 4 bytes */
	memcpy((PUCHAR)&BandIdx, (PUCHAR)&HqaCmdFrame->Data, 4);
	BandIdx = PKTL_TRAN_TO_HOST(BandIdx);

	/* Prepare for RDD dump */
	OS_SPIN_LOCK(&log_cb->lock);
	log_cb->is_dumping = TRUE;
	OS_SPIN_UNLOCK(&log_cb->lock);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[RDD DUMP START][HQA_GetDumpRDD]\n");
	pAd->fgDumpStart = 1;

	/*Get RDD dump from log dump*/
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"log_cb->idx is %d, log_cb->len is %d\n", log_cb->idx, log_cb->len);

	if ((ATECtrl->firstRDD == TRUE) || (ATECtrl->firstQATool == TRUE)) {
		/*
		 * Reset idx - 1. HQA RDD dump (re-)enable
		 * Reset idx - 2. HQA Tool (re-)open
		 */
		idx = 0;
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "idx: %d\n", idx);
	/* If log_cb->idx greater than log_cb->len(RDDBufferSize), it will re-count from 0 */
	remindIdx = (idx > log_cb->idx) ?
		((log_cb->idx + log_cb->len) - idx) :
		(log_cb->idx - idx);

	end = (remindIdx > MAX_RDD_DUMP_SIZE) ?
		((idx + MAX_RDD_DUMP_SIZE) % (log_cb->len)) :
		((idx + remindIdx) % (log_cb->len));

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"remindIdx: %d, end: %d\n", remindIdx, end);

	do {
			idx = (idx % (log_cb->len));
			if (log_cb->entry[idx].un_dumped) {
				struct _ATE_RDD_LOG *result = &log_cb->entry[idx].log.rdd;
				UINT32 *pulse = (UINT32 *)result->aucBuffer;

				/* To sperate the interrupts of radar signals */
				if (!result->byPass) {
					MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						"[RDD]0x%08x %08x\n", result->u4Prefix, result->u4Count);
					/*Response format: cmd type + cmd ID + length + Sequence + Data (status + count + value1 + value2)*/
					/*Data -> [status (2 bytes) + count (4 bytes)] = 6 bytes + value1 (4 bytes) + value2 (4 bytes) */
					memcpy(HqaCmdFrame->Data + 6 + count_total*4, &result->u4Prefix, 4);
					count_total++;
					result->u4Count = OS_NTOHL(result->u4Count);
					memcpy(HqaCmdFrame->Data + 6 + count_total*4, &result->u4Count, 4);
					count_total++;
				}

				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[RDD]0x%08x %08x\n", pulse[0], pulse[1]);
				log_cb->entry[idx].un_dumped = FALSE;
				u4BufferCounter++;

				/*Reverse the ordering of the four bytes in a 32-bit unsigned integer value*/
				pulse[0] = OS_NTOHL(pulse[0]);
				pulse[1] = OS_NTOHL(pulse[1]);

				/*Response format: cmd type + cmd ID + length + Sequence + Data (status + count + value1 + value2)*/
				/*Data -> {status (2 bytes) + count (4 bytes) = 6 bytes} + [value1 (4 bytes)] + value2 (4 bytes) */
				memcpy(HqaCmdFrame->Data + 6 + count_total*4, &pulse[0], 4);
				count_total++;
				/*Data -> {status (2 bytes) + count (4 bytes) = 6 bytes} + value1 (4 bytes) + [value2 (4 bytes)] */
				memcpy(HqaCmdFrame->Data + 6 + count_total*4, &pulse[1], 4);
				count_total++;
			}

			/* The size of per entry is 38 bytes and for QAtool log buffer limitation. */
			if ((pAd->fgQAtoolBatchDumpSupport) &&
				(u4BufferCounter >= (1 << (CONFIG_LOG_BUF_SHIFT - 1)) / 38)) {
				pAd->u2LogEntryIdx = idx;
				break;
			}

			INC_RING_INDEX(idx, log_cb->len);
	} while (idx != end);

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[After RDD dumping] idx: %d, end: %d\n", idx, end);

	/* RDD DUMP END */
	/* Count = Total number of 4 bytes RDD values divided by 2 */
	count_total = count_total / 2;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"[HQA_GetDumpRDD]: Total Count As a Group of Two Pulses = %d\n", count_total);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"[HQA_GetDumpRDD] idx = %d, log_cb->idx = %d, pAd->fgDumpStart = %d\n", idx, log_cb->idx, pAd->fgDumpStart);
	if ((idx == log_cb->idx) && (pAd->fgDumpStart)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "[RDD DUMP END]\n");
		pAd->fgDumpStart = 0;
	}
	OS_SPIN_LOCK(&log_cb->lock);
	log_cb->is_dumping = FALSE;
	OS_SPIN_UNLOCK(&log_cb->lock);

	/* Send RDD Dump to HQA tool */
	if (count_total > 0) {
		/*Response format: cmd type + cmd ID + length + Sequence + Data (status + count + value1 + value2)*/
		/*Data -> status (2 bytes) + [count (4 bytes)] + value1 (4 bytes) + value2 (4 bytes) */
		count_total = OS_NTOHL(count_total);
		memcpy(HqaCmdFrame->Data + 2, &count_total, 4);
		count_total = OS_NTOHL(count_total);
		/*Data -> status (2 bytes) + count (4 bytes) + [value1 (4 bytes) + value2 (4 bytes)]*count_total */
		ResponseToQA(HqaCmdFrame, WRQ, 6 + count_total*8, i4Ret);
	} else {
		/*Data -> [status (2 bytes)] + count (4 bytes) + value1 (4 bytes) + value2 (4 bytes) */
		memcpy(HqaCmdFrame->Data + 2, &count_total, 4);
		ResponseToQA(HqaCmdFrame, WRQ, 6, i4Ret);
	}

	ATECtrl->firstRDD    	= FALSE;
	ATECtrl->firstQATool	= FALSE;

	return STATUS_SUCCESS;
}

static INT32 HQA_GetDumpRXV(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, struct _HQA_CMD_FRAME *HqaCmdFrame)
{
#define MAX_DUMP_SIZE_OUT_RXV 40
	INT32 Ret = 0;
	UINT32 band_idx = 0, Value = 0, i = 0;
	UINT32 RespLen = 2;
	UCHAR *data = HqaCmdFrame->Data;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	NdisMoveMemory((PUCHAR)&band_idx, data, sizeof(band_idx));
	band_idx = PKTL_TRAN_TO_HOST(band_idx);
	ATECtrl->control_band_idx = (UCHAR)band_idx;
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"band_idx:%x, en_log:%x\n", band_idx, ATECtrl->en_log);

	if (ATEOp->GetDumpRXV) {
		INT32 retval;
		UINT32 CycleCnt, Len;
		PUINT32 pData = NULL;
		PUINT32 pCount = NULL;

		/* Dynamic allocate memory for data length */
		Len = sizeof(UINT32);
		retval = os_alloc_mem(pAd, (UCHAR **)&pCount, Len);
		if (retval != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Not enough memory for dynamic allocating !!\n");
			goto error;
		}
		os_zero_mem(pCount, Len);
		*pCount = MAX_DUMP_SIZE_OUT_RXV;

		/* Dynamic allocate memory for data buffer */
		CycleCnt = (*pCount) * (sizeof(struct _ATE_RXV_LOG)/sizeof(UINT32));
		Len = CycleCnt * sizeof(UINT32);
		retval = os_alloc_mem(pAd, (UCHAR **)&pData, Len);
		if (retval != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"Not enough memory for dynamic allocating !!\n");
			goto error;
		}
		os_zero_mem(pData, Len);

		/* Initial value for pCount */
		Ret = ATEOp->GetDumpRXV(pAd, pData, pCount);

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"pCount = %d !!\n", *pCount);

		Value = PKTL_TRAN_TO_HOST(*pCount);
		NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
		RespLen += sizeof(Value);

		CycleCnt = (*pCount) * (sizeof(struct _ATE_RXV_LOG)/sizeof(UINT32));

		for (i = 0; i < CycleCnt; i++) {
			Value = PKTL_TRAN_TO_HOST(pData[i]);
			NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
			RespLen += sizeof(Value);
		}

		ResponseToQA(HqaCmdFrame, WRQ, RespLen, Ret);

error:
		if (pCount != NULL)
			os_free_mem(pCount);
		if (pData != NULL)
			os_free_mem(pData);
	} else
		Ret = TM_STATUS_NOTSUPPORT;

	ATECtrl->firstRXV	= FALSE;
	ATECtrl->firstQATool	= FALSE;

#undef MAX_DUMP_SIZE_OUT_RXV
	return Ret;
}

static HQA_CMD_HANDLER HQA_ICAP_CMDS[] = {
	HQA_CapWiFiSpectrum,			/*0x1580 */
};
static HQA_CMD_HANDLER HQA_ReCal_CMDS[] = {
	HQA_GetDumpRecal,			/*0x1581 */
};
static HQA_CMD_HANDLER HQA_RXV_CMDS[] = {
	HQA_GetDumpRXV,			/* 0x1582 */
};
static HQA_CMD_HANDLER HQA_RDD_CMDS[] = {
	HQA_GetDumpRDD,			/*0x1583 */
};

#if defined(DOT11_HE_AX)
static INT32 HQA_SetMaxPacExt(PRTMP_ADAPTER ad, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *hqa_cmd_frame)
{
	INT32 Ret = 0;
	UINT32 Value = 0;

	memcpy((PUCHAR)&Value, (PUCHAR)&hqa_cmd_frame->Data, 4);
	TESTMODE_SET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), max_pkt_ext, PKTL_TRAN_TO_HOST(Value));
	MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Max Packet Extension:%d (%d us)\n", TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), max_pkt_ext),
									     16-TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), max_pkt_ext)*8);

	ResponseToQA(hqa_cmd_frame, wrq, 2, Ret);
	return Ret;
}

static INT32 HQA_GetHETBParams(PRTMP_ADAPTER ad, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *hqa_cmd_frame)
{
	INT32 Ret = 0, afactor = 0, ldpc_str_sym = 0, pe_disamb = 0, tx_pe = 0, l_sig_len = 0, value = 0;
	UINT8 dmnt_ru_idx = TESTMODE_GET_PARAM(ad, TESTMODE_GET_BAND_IDX(ad), dmnt_ru_idx);
	struct _ATE_RU_STA *ru_sta = (struct _ATE_RU_STA *)TESTMODE_GET_PADDR(ad, TESTMODE_GET_BAND_IDX(ad), ru_info_list[0]);

	afactor = ru_sta[dmnt_ru_idx].afactor_init;
	ldpc_str_sym = ru_sta[dmnt_ru_idx].ldpc_extr_sym;
	pe_disamb = ru_sta[dmnt_ru_idx].pe_disamb;
	tx_pe = ru_sta[dmnt_ru_idx].t_pe;
	l_sig_len = ru_sta[dmnt_ru_idx].l_len;

	/* The response array should be a-factor, ldpc extra symbol, PE disambiguilty, TX PE, L-SIG length */
	value = PKTL_TRAN_TO_NET(afactor);
	memcpy(hqa_cmd_frame->Data + 2, &value, sizeof(value));
	value = PKTL_TRAN_TO_NET(ldpc_str_sym);
	memcpy(hqa_cmd_frame->Data + 2 + sizeof(value), &value, sizeof(value));
	value = PKTL_TRAN_TO_NET(pe_disamb);
	memcpy(hqa_cmd_frame->Data + 2 + sizeof(value)*2, &value, sizeof(value));
	value = PKTL_TRAN_TO_NET(tx_pe);
	memcpy(hqa_cmd_frame->Data + 2 + sizeof(value)*3, &value, sizeof(value));
	value = PKTL_TRAN_TO_NET(l_sig_len);
	memcpy(hqa_cmd_frame->Data + 2 + sizeof(value)*4, &value, sizeof(value));

	ResponseToQA(hqa_cmd_frame, wrq, 2 + 5*sizeof(INT32), Ret);
	return Ret;
}

static INT32 HQA_GetPFDInfo(PRTMP_ADAPTER ad, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *hqa_cmd_frame)
{
	INT32 Ret = 0;

	ResponseToQA(hqa_cmd_frame, wrq, 2, Ret);
	return Ret;
}

static INT32 HQA_GetStaRUSetting(PRTMP_ADAPTER ad, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *hqa_cmd_frame)
{
	INT32 Ret = 0;

	ResponseToQA(hqa_cmd_frame, wrq, 2, Ret);
	return Ret;
}

static UINT8 mt_ate_translate_ru_allocation(UINT32 user_ru_allocation)
{
	UINT8 allocation = 0, i = 0;

	for (i = 0 ; i < sizeof(UINT32)*2 ; i++) {
		allocation |= ((user_ru_allocation & 0x1) << i);
		user_ru_allocation >>= 4;
	}

	return allocation;
}
static INT32 HQA_SetStaRUSetting(PRTMP_ADAPTER ad, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *hqa_cmd_frame)
{
	INT32 Ret = 0, tone_idx = 0;
	UINT32 len = 0, band_idx = 0, seg_sta_cnt[2] = {0}, sta_seq = 0, value = 0;
	UCHAR *data = hqa_cmd_frame->Data, param_cnt = 0, segment_idx = 0;
	UINT32 mpdu_length = 0;
	struct _ATE_RU_ALLOCATION *ru_allocation = NULL;
	struct _ATE_RU_STA *ru_sta = NULL;

	len = PKTS_TRAN_TO_HOST(hqa_cmd_frame->Length);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&band_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&seg_sta_cnt[0]);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&seg_sta_cnt[1]);
	param_cnt = (len-sizeof(UINT32)*3)/(seg_sta_cnt[0]+seg_sta_cnt[1])/sizeof(UINT32);

	MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Band:%d [ru_segment 0]:%d, [ru_segment 1]:%d, parameters count:%d\n",
			  band_idx, seg_sta_cnt[0], seg_sta_cnt[1], param_cnt);

	mpdu_length = TESTMODE_GET_PARAM(ad, band_idx, tx_len);
	ru_allocation = (struct _ATE_RU_ALLOCATION *)TESTMODE_GET_PADDR(ad, band_idx, ru_alloc);
	ru_sta = (struct _ATE_RU_STA *)TESTMODE_GET_PADDR(ad, band_idx, ru_info_list[0]);
	os_zero_mem(ru_sta, sizeof(struct _ATE_RU_STA)*MAX_MULTI_TX_STA);
	os_fill_mem(ru_allocation, sizeof(*ru_allocation), 0xff);

	/* for maximum bw 80+80/160, 2 segments only */
	for (segment_idx = 0; segment_idx < 2 ; segment_idx++) {
		for (sta_seq = 0 ; sta_seq < seg_sta_cnt[segment_idx] ; sta_seq++) {
			UINT allocation = 0;
			param_cnt = (len-sizeof(UINT32)*3)/(seg_sta_cnt[0]+seg_sta_cnt[1])/sizeof(UINT32);

			sta_seq += seg_sta_cnt[0]*segment_idx;
			ru_sta[sta_seq].valid = TRUE;
			EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&value);	/* ru caterogy */
			param_cnt--;
			EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&value);	/* ru allocation */
			param_cnt--;
			allocation = mt_ate_translate_ru_allocation(value);
			EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&value);	/* sta index */
			param_cnt--;
			EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&value);
			param_cnt--;
			ru_sta[sta_seq].ru_index = (value << 1) | segment_idx;
			mt_ate_add_allocation(ru_allocation, allocation, segment_idx, value);
			EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&value);
			param_cnt--;
			ru_sta[sta_seq].rate = value;
			EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&value);
			param_cnt--;
			ru_sta[sta_seq].ldpc = value;
			EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&value);
			param_cnt--;
			ru_sta[sta_seq].nss = value;
			EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&value);
			param_cnt--;
			ru_sta[sta_seq].start_sp_st = value-1;
			EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&value);
			if (value > 24)
				ru_sta[sta_seq].mpdu_length = value;
			else
				ru_sta[sta_seq].mpdu_length = mpdu_length;
			param_cnt--;

			if (param_cnt) {
				EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&value);
				param_cnt--;
				ru_sta[sta_seq].alpha = value;
			}

			if (param_cnt) {
				EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&value);
				param_cnt--;
				ru_sta[sta_seq].ru_mu_nss = value;
			}

			MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "ru_segment[%d][0x%x]: ru_idx:%d, rate:%x, ldpc:%d, nss:%d,\n\t\t\t\t start spatial stream:%d, mpdu length=%d, alpha:%d, ru_mu_nss=%d\n",
					  segment_idx, allocation, ru_sta[sta_seq].ru_index >> 1, ru_sta[sta_seq].rate, ru_sta[sta_seq].ldpc,
					  ru_sta[sta_seq].nss, ru_sta[sta_seq].start_sp_st, ru_sta[sta_seq].mpdu_length, ru_sta[sta_seq].alpha, ru_sta[sta_seq].ru_mu_nss);

		}
	}

	mt_ate_fill_empty_allocation(ru_allocation);

	for (tone_idx = 0 ; tone_idx < sizeof(*ru_allocation) ; tone_idx++)
		MTWF_DBG(ad, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "allocation[%d] = 0x%x\n", tone_idx, ru_allocation->allocation[tone_idx]);

	ResponseToQA(hqa_cmd_frame, wrq, 2, Ret);
	return Ret;
}

static HQA_CMD_HANDLER HQA_HE_CMDS[] = {
	HQA_SetMaxPacExt,		/*0x1590 */
	HQA_GetHETBParams,		/*0x1591 */
	HQA_GetPFDInfo,			/*0x1592 */
	HQA_GetStaRUSetting,	/*0x1593 */
	HQA_SetStaRUSetting,	/*0x1594 */
};
#endif /* DOT11_HE_AX */

static INT32 hqa_set_channel_ext(PRTMP_ADAPTER pAd,
					RTMP_IOCTL_INPUT_STRUCT *wrq,
					struct _HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UINT32 len = 0;
	struct _HQA_EXT_SET_CH param = {0};
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
	UCHAR *data = cmd_frame->Data;
	UINT32 pri_ch = 0;
	UINT32 band_idx = 0;
	UINT32 bw = 0;
	UINT32 per_pkt_bw = 0;

	len = PKTS_TRAN_TO_HOST(cmd_frame->Length);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.num_param);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.band_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.central_ch0);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.central_ch1);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.sys_bw);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.perpkt_bw);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.pri_sel);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.reason);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.ch_band);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.out_band_freq);

	if (param.band_idx > TESTMODE_BAND_NUM-1) {
		ret = NDIS_STATUS_INVALID_DATA;
		goto err0;
	}

	band_idx = param.band_idx;

	switch (param.sys_bw) {
	case ATE_BAND_WIDTH_20:
		bw = BAND_WIDTH_20;
		break;

	case ATE_BAND_WIDTH_40:
		bw = BAND_WIDTH_40;
		break;

	case ATE_BAND_WIDTH_80:
		bw = BAND_WIDTH_80;
		break;

	case ATE_BAND_WIDTH_10:
		bw = BAND_WIDTH_10;
		break;

	case ATE_BAND_WIDTH_5:
		bw = BAND_WIDTH_5;
		break;

	case ATE_BAND_WIDTH_160:
		bw = BAND_WIDTH_160;
		break;

	case ATE_BAND_WIDTH_8080:
		bw = BAND_WIDTH_8080;
		break;

	default:
		bw = BAND_WIDTH_20;
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "Cannot find BW with param.sys_bw:%x, forced as 0x%x\n",
				  param.sys_bw, bw);
		break;
	}

	switch (param.perpkt_bw) {
	case ATE_BAND_WIDTH_20:
		per_pkt_bw = BAND_WIDTH_20;
		break;

	case ATE_BAND_WIDTH_40:
		per_pkt_bw = BAND_WIDTH_40;
		break;

	case ATE_BAND_WIDTH_80:
		per_pkt_bw = BAND_WIDTH_80;
		break;

	case ATE_BAND_WIDTH_10:
		per_pkt_bw = BAND_WIDTH_10;
		break;

	case ATE_BAND_WIDTH_5:
		per_pkt_bw = BAND_WIDTH_5;
		break;

	case ATE_BAND_WIDTH_160:
	case ATE_BAND_WIDTH_8080:
		per_pkt_bw = BAND_WIDTH_160;
		break;

	default:
		per_pkt_bw = BAND_WIDTH_20;
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 "Cannot find BW with param.sys_bw:%x\n, forced as 0x%x",
				  param.sys_bw, per_pkt_bw);
		break;
	}

	/* Set Param */
	TESTMODE_SET_PARAM(pAd, band_idx, channel, param.central_ch0);
#ifdef DOT11_VHT_AC
	TESTMODE_SET_PARAM(pAd, band_idx, channel_2nd, param.central_ch1);
#endif
	TESTMODE_SET_PARAM(pAd, band_idx, per_pkt_bw, per_pkt_bw);
	TESTMODE_SET_PARAM(pAd, band_idx, bw, bw);
	TESTMODE_SET_PARAM(pAd, band_idx, pri_sel, param.pri_sel);
	TESTMODE_SET_PARAM(pAd, band_idx, ch_band, param.ch_band);
	TESTMODE_SET_PARAM(pAd, band_idx, out_band_freq, param.out_band_freq);
	ate_ctrl->control_band_idx = (UCHAR)band_idx;

	ate_ops->SetChannel(pAd, param.central_ch0, param.pri_sel, param.reason, param.ch_band);
err0:
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&param.ext_id, 4);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "len:%x, num_param:%x, band_idx:%x, ch0:%u, ch1:%u, sys_bw:%x, bw_conver:%x, ",
			  len, param.num_param, param.band_idx,
			  param.central_ch0, param.central_ch1, param.sys_bw, bw);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "perpkt_bw:%x, pri_sel:%x, pri_ch:%u, ch_band:%u\n",
			  param.perpkt_bw, param.pri_sel, pri_ch,  param.ch_band);
	ResponseToQA(cmd_frame, wrq, 6, ret);
	return ret;
}


static INT32 hqa_set_txcontent_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UINT32 len = 0, pl_len = 0;
	struct _HQA_EXT_TX_CONTENT param = {0};
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _ATE_TX_TIME_PARAM *tx_time_param = NULL;
	UCHAR *data = cmd_frame->Data;
	HEADER_802_11 *phdr = NULL;
	UCHAR *addr1, *addr2, *addr3, *payload;
	UINT32 band_idx = 0;
	UINT16 sta_idx = 0;

	len = PKTS_TRAN_TO_HOST(cmd_frame->Length);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.num_param);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.band_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.FC);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.dur);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.seq);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.gen_payload_rule);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.txlen);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.payload_len);
	EthGetParamAndShiftBuff(FALSE, MAC_ADDR_LEN, &data, param.addr1);
	EthGetParamAndShiftBuff(FALSE, MAC_ADDR_LEN, &data, param.addr2);
	EthGetParamAndShiftBuff(FALSE, MAC_ADDR_LEN, &data, param.addr3);

	/* 52 for the size before payload */
	if (param.payload_len > ATE_MAX_PATTERN_SIZE)
		param.payload_len = ATE_MAX_PATTERN_SIZE;

	/* Set Param */
	band_idx = param.band_idx;
	ate_ctrl->control_band_idx = (UCHAR)band_idx;

	for (sta_idx = 0 ; sta_idx < MAX_MULTI_TX_STA ; sta_idx++) {
		if (band_idx < DBDC_BAND_NUM) {
			addr1 = TESTMODE_GET_PARAM(pAd, band_idx, addr1[sta_idx]);
			addr2 = TESTMODE_GET_PARAM(pAd, band_idx, addr2[sta_idx]);
			addr3 = TESTMODE_GET_PARAM(pAd, band_idx, addr3[sta_idx]);

			NdisMoveMemory(addr1, param.addr1, MAC_ADDR_LEN);
			NdisMoveMemory(addr2, param.addr2, MAC_ADDR_LEN);
			NdisMoveMemory(addr3, param.addr3, MAC_ADDR_LEN);
		} else
			return NDIS_STATUS_FAILURE;
	}
	phdr = (HEADER_802_11 *)TESTMODE_GET_PARAM(pAd, band_idx, template_frame);
	pl_len = TESTMODE_GET_PARAM(pAd, band_idx, pl_len);
	/* pl_addr = TESTMODE_GET_PADDR(pAd, band_idx, payload); */
	payload = TESTMODE_GET_PARAM(pAd, band_idx, payload);
	NdisMoveMemory(&phdr->FC, &param.FC, sizeof(phdr->FC));
	phdr->Duration = (UINT16)param.dur;
	phdr->Sequence = (UINT16)param.seq;
	TESTMODE_SET_PARAM(pAd, band_idx, fixed_payload, param.gen_payload_rule);
	TESTMODE_SET_PARAM(pAd, band_idx, tx_len, param.txlen);
	TESTMODE_SET_PARAM(pAd, band_idx, pl_len, param.payload_len);
	/* payload = *pl_addr; */

	/* Error check for txlen and payload_len */
	if ((param.txlen == 0) || (param.payload_len == 0)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"txlen/payload_len=%d/%d can't be 0!!\n", param.txlen, param.payload_len);
		return NDIS_STATUS_FAILURE;
	}

	/* Packet TX time feature implementation */
	tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, tx_time_param);
	if (tx_time_param->pkt_tx_time_en == TRUE) {
		tx_time_param->pkt_tx_time = param.txlen;
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"FC:%04x, dur:%u, seq:%u, plen:%u, pkt_tx_time:%u, GENPKT:%u\n",
			param.FC, param.dur, param.seq, param.payload_len,
			tx_time_param->pkt_tx_time, param.gen_payload_rule);

	} else {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			"FC:%04x, dur:%u, seq:%u, plen:%u, txlen:%u, GENPKT:%u\n",
			param.FC, param.dur, param.seq, param.payload_len,
			param.txlen, param.gen_payload_rule);
	}

	EthGetParamAndShiftBuff(FALSE, param.payload_len, &data, payload);
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&param.ext_id, sizeof(param.ext_id));
	ResponseToQA(cmd_frame, wrq, 2 + sizeof(param.ext_id), ret);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"addr1:%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(addr1));
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"addr2:%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(addr2));
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"addr3:%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(addr3));
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"ret:%u, len:%u, param_len:%u\n", ret, param.payload_len, pl_len);

	return ret;
}


static INT32 hqa_start_tx_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	INT32 len = 0;
	UINT32 band_idx = 0;
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
	UCHAR *data = cmd_frame->Data;
	struct _HQA_EXT_TXV param = {0};
	ATE_TXPOWER TxPower;
	UINT32 Channel, Ch_Band, SysBw = 0, PktBw = 0, ipg = 0;

	len = PKTS_TRAN_TO_HOST(cmd_frame->Length);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.num_param);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.band_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.pkt_cnt);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.tx_mode);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.rate);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.pwr);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.stbc);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.ldpc);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.ibf);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.ebf);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.wlan_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.aifs);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.gi);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.tx_path);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.nss);
	band_idx = param.band_idx;

	if (!param.pkt_cnt)
		param.pkt_cnt = 0x8fffffff;
	if (band_idx >= DBDC_BAND_NUM) {
		ret = -1;
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Band_idx out of range!\n");
		goto err0;
	}

	TESTMODE_SET_PARAM(pAd, band_idx, ATE_TX_CNT, param.pkt_cnt);
	TESTMODE_SET_PARAM(pAd, band_idx, tx_mode, param.tx_mode);
	TESTMODE_SET_PARAM(pAd, band_idx, mcs, param.rate);
	TESTMODE_SET_PARAM(pAd, band_idx, stbc, param.stbc);
	TESTMODE_SET_PARAM(pAd, band_idx, ldpc, param.ldpc);
#ifdef TXBF_SUPPORT
	TESTMODE_SET_PARAM(pAd, band_idx, ibf, param.ibf);
	TESTMODE_SET_PARAM(pAd, band_idx, ebf, param.ebf);
#endif
	ate_ctrl->wcid_ref = param.wlan_id;
	/* TODO: Need to modify */
	TESTMODE_SET_PARAM(pAd, band_idx, ipg_param.ipg, param.aifs);		/* Fix me */
	TESTMODE_SET_PARAM(pAd, band_idx, sgi, param.gi);
	TESTMODE_SET_PARAM(pAd, band_idx, tx_ant, param.tx_path);
	TESTMODE_SET_PARAM(pAd, band_idx, nss, param.nss);
	Channel = TESTMODE_GET_PARAM(pAd, band_idx, channel);
	Ch_Band = TESTMODE_GET_PARAM(pAd, band_idx, ch_band);
	PktBw = TESTMODE_GET_PARAM(pAd, band_idx, per_pkt_bw);
	SysBw = TESTMODE_GET_PARAM(pAd, band_idx, bw);
	ipg = TESTMODE_GET_PARAM(pAd, band_idx, ipg_param.ipg);
	ate_ctrl->control_band_idx = (UCHAR)band_idx;

	if (param.tx_mode < MODE_VHT && param.rate == 32 && PktBw != BAND_WIDTH_40 && SysBw != BAND_WIDTH_40) {
		ret = -1;
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Bandwidth must to be 40 at MCS 32\n");
		goto err0;
	}

	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Power = param.pwr;
	TxPower.Channel = Channel;
	TxPower.Dbdc_idx = band_idx;
	TxPower.Band_idx = Ch_Band;
	ate_ops->SetIPG(pAd, ipg);
	ate_ops->tx_commit(pAd);
	ret = ate_ops->StartTx(pAd);
err0:
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&param.ext_id, sizeof(param.ext_id));
	ResponseToQA(cmd_frame, wrq, 2 + sizeof(param.ext_id), ret);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "band_idx:%u, pkt_cnt:%u, phy:%u, mcs:%u, stbc:%u, ldpc:%u\n",
			  param.band_idx, param.pkt_cnt, param.tx_mode,
			  param.rate, param.stbc, param.ldpc);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "ibf:%u, ebf:%u, wlan_id:%u, aifs:%u, gi:%u, tx_path:%x, nss:%x\n",
			  param.ibf, param.ebf, param.wlan_id, param.aifs,
			  param.gi, param.tx_path, param.nss);
	return ret;
}


static INT32 hqa_start_rx_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
	UCHAR *data = cmd_frame->Data;
	UINT32 ext_id = 0;
	UINT32 param_num = 0;
	UINT32 band_idx = 0;
	UINT32 rx_path = 0;
	UINT32 user_idx = 0;
	UCHAR own_mac[MAC_ADDR_LEN];
	UINT32 tx_mode = 0;
	UINT32 ltf_gi = 0;

	NdisZeroMemory(own_mac, MAC_ADDR_LEN);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_num);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&band_idx);
	EthGetParamAndShiftBuff(FALSE, MAC_ADDR_LEN, &data, (UCHAR *)&own_mac);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&rx_path);

	ate_ctrl->control_band_idx = (UCHAR)band_idx;
	if (band_idx >= DBDC_BAND_NUM) {
		ret = -1;
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Band_idx out of range!\n");
		return ret;
	}

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)TESTMODE_GET_PADDR(pAd, band_idx, mu_rx_aid));
	if (param_num > 3) {
		EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&tx_mode);
		EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ltf_gi);
		EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&user_idx);

		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				"param num:%d, tx_mode:%d, ltf_gi:%d, user_idx:%d\n", param_num, tx_mode, ltf_gi, user_idx);
	}

	TESTMODE_SET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), tx_mode, tx_mode);
	TESTMODE_SET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), sgi, ltf_gi);
	TESTMODE_SET_PARAM(pAd, TESTMODE_GET_BAND_IDX(pAd), user_idx, user_idx);

	ret = ate_ops->SetRxUserIdx(pAd, band_idx, user_idx);
	ret = ate_ops->SetRxAntenna(pAd, rx_path);
	ret = ate_ops->SetAutoResp(pAd, own_mac, 1);
	ret = ate_ops->StartRx(pAd);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "param num:%u, band_sel:%u, rx_path:%x, mac:%02x:%02x:%02x:%02x:%02x:%02x, MU Aid:%d\n",
			  param_num, band_idx, rx_path, PRINT_MAC(own_mac), TESTMODE_GET_PARAM(pAd, band_idx, mu_rx_aid));
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
	ResponseToQA(cmd_frame, wrq, 2 + sizeof(ext_id), ret);
	return ret;
}


static INT32 hqa_stop_tx_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UCHAR *data = cmd_frame->Data;
	UINT32 ext_id = 0;
	UINT32 param_num = 0;
	UINT32 band_idx = 0;
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_num);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&band_idx);

	ate_ctrl->control_band_idx = (UCHAR)band_idx;

	ate_ops->StopTx(pAd);
	ret = ate_ops->tx_revert(pAd);
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
	ResponseToQA(cmd_frame, wrq, 2 + sizeof(ext_id), ret);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"band_idx:%u\n", band_idx);
	return ret;
}

static INT32 hqa_stop_rx_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UCHAR *data = cmd_frame->Data;
	UINT32 ext_id = 0;
	UINT32 param_num = 0;
	UINT32 band_idx = 0;
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_num);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&band_idx);

	ate_ctrl->control_band_idx = (UCHAR)band_idx;

	ret = ate_ops->StopRx(pAd);
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
	ResponseToQA(cmd_frame, wrq, 2 + sizeof(ext_id), ret);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"band_idx:%u\n", band_idx);
	return ret;
}


static INT32 hqa_set_tx_time(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UCHAR *data = cmd_frame->Data;
	UINT32 ext_id = 0;
	UINT32 band_idx = 0;
	UINT32 is_tx_time = 0;
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _ATE_TX_TIME_PARAM *tx_time_param = NULL;

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&band_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&is_tx_time);
	if (band_idx >= DBDC_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"band_idx is out of range\n");
		ret = -1;
		return ret;
	}
	tx_time_param = (struct _ATE_TX_TIME_PARAM *)TESTMODE_GET_PADDR(pAd, band_idx, tx_time_param);

	ate_ctrl->control_band_idx = (UCHAR)band_idx;

	/* 0: use tx length, 1: use tx time */
	if (is_tx_time == 1) {
		tx_time_param->pkt_tx_time_en = TRUE;
	} else {
		tx_time_param->pkt_tx_time_en = FALSE;
		tx_time_param->pkt_tx_time = 0;		/* Reset to 0 when start TX everytime */
	}
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
	ResponseToQA(cmd_frame, wrq, 2 + sizeof(ext_id), ret);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		"band_idx:%u, is_tx_time:%d\n", band_idx, is_tx_time);
	return ret;
}

#if OFF_CH_SCAN_SUPPORT
static INT32 hqa_off_ch_scan(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UCHAR *data = cmd_frame->Data;
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
	ATE_OFF_CH_SCAN param;

	/* Initialize parameters */
	os_zero_mem(&param, sizeof(param));

	/* Get parameters from command frame */
	/* Data sequences of command frame:
	*  ExtendId (4 bytes) + HW Band idx (4 bytes) + Channel (4 bytes) +
	*  isAband (4 bytes) + systemBW (4 bytes) + TxRxPath (4 bytes) + Status (4 bytes)
	*/
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.dbdc_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.mntr_ch);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.is_aband);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.mntr_bw);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.mntr_tx_rx_pth);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.scan_mode);

	ret = ate_ops->off_ch_scan(pAd, &param);

	/* Send response to QA tool */
	/* Response format:
	*  Status (2 bytes) + Extend ID (4 bytes)
	*/
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&(param.ext_id), sizeof(param.ext_id));
	ResponseToQA(cmd_frame, wrq, 6, ret);
	return ret;

}
#else
static INT32 hqa_off_ch_scan(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Not support\n");
	ResponseToQA(cmd_frame, wrq, 2, ret);

	return ret;
}
#endif

#ifdef TXBF_SUPPORT
static INT32 hqa_iBFGetStatus_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UCHAR *data = cmd_frame->Data;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UCHAR *txbf_info = TESTMODE_GET_PARAM(pAd, control_band_idx, txbf_info);
	UINT32 u4Op_mode;
	UINT32 ext_id = 0;
	UINT32 u4Status = 0;

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);

	if (!(TESTMODE_GET_PARAM(pAd, control_band_idx, txbf_info)))
		goto HQA_TAG_DNC_FAIL;

	u4Status = TESTMODE_GET_PARAM(pAd, control_band_idx, iBFCalStatus);
	u4Op_mode = TESTMODE_GET_PARAM(pAd, control_band_idx, op_mode);
	u4Op_mode &= ~fATE_IN_BF;
	TESTMODE_SET_PARAM(pAd, control_band_idx, op_mode, u4Op_mode);
	os_free_mem(txbf_info);
	/* MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "val:%x\n", u4Status); */
	ext_id   = PKTL_TRAN_TO_HOST(ext_id);
	u4Status = PKTL_TRAN_TO_HOST(u4Status);
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
	NdisMoveMemory(cmd_frame->Data + 6, (UCHAR *)&u4Status, 4);
HQA_TAG_DNC_FAIL:
	ResponseToQA(cmd_frame, wrq, 10, ret);
	return ret;
}


static INT32 hqa_iBFSetValue_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	INT snprintf_ret;
	UCHAR *data = cmd_frame->Data;
	UCHAR control_band_idx = TESTMODE_GET_BAND_IDX(pAd);
	UCHAR *txbf_info = TESTMODE_GET_PARAM(pAd, control_band_idx, txbf_info);
	UINT32 u4Op_mode;
	UINT32 ext_id = 0;
	UINT32 u4Action = 0;
	UINT32 u4InArg[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	RTMP_STRING *cmd;

	txbf_info = NULL;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		ret = NDIS_STATUS_RESOURCES;
		goto HQA_IBF_CMD_FAIL;
	}

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4Action);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[0]);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[1]);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[2]);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[3]);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[4]);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[5]);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[6]);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[7]);

	switch (u4Action) {
	case ATE_TXBF_INIT:
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", (UCHAR)u4InArg[0]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}
		SetATETxBfDutInitProc(pAd, cmd);
		break;

	case ATE_CHANNEL:
		memset(cmd, 0x00, HQA_BF_STR_SIZE);

		if (u4InArg[1] == 1) {
			snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d:1", (UCHAR)u4InArg[0]);
			if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"cmd snprintf error!\n");
				os_free_mem(cmd);
				return FALSE;
			}
		} else {
			snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", (UCHAR)u4InArg[0]);
			if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					"cmd snprintf error!\n");
				os_free_mem(cmd);
				return FALSE;
			}
		}

		SetATEChannel(pAd, cmd);
		break;

	case ATE_TX_MCS:
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", (UCHAR)u4InArg[0]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}
		SetATETxMcs(pAd, cmd);
		break;

	case ATE_TX_POW0:
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", (UCHAR)u4InArg[0]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}
		SetATETxPower0(pAd, cmd);
		break;

	case ATE_TX_ANT:
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", (UCHAR)u4InArg[0]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}
		SetATETxAntenna(pAd, cmd);
		break;

	case ATE_RX_FRAME:
		SetATE(pAd, "TXSTOP");
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%s", "RXFRAME");
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}
		SetATE(pAd, cmd);
		break;

	case ATE_RX_ANT:
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", (UCHAR)u4InArg[0]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}
		SetATERxAntenna(pAd, cmd);
		break;

	case ATE_TXBF_LNA_GAIN:
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%d", (UCHAR)u4InArg[0]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}
		SetATETxBfLnaGain(pAd, cmd);
		break;

	case ATE_IBF_PHASE_COMP:
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		/* BW:DBDC idx:Group:Read from E2P:Dis compensation */
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x:%02x",
				u4InArg[0], u4InArg[1],
				u4InArg[2], u4InArg[3],
				u4InArg[4]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}

		/* check the range for coverity */
		if (*cmd < HQA_BF_STR_SIZE) {
			if (SetATEIBfPhaseComp(pAd, cmd) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " ATE_IBF_PHASE_COMP is failed!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto HQA_IBF_CMD_FAIL;
			}

		}

		break;

	case ATE_IBF_TX:
		u4InArg[2] = 0; /* for test purpose */
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		/* fgBf:WLAN idx:Txcnt */
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x",
				u4InArg[0], u4InArg[1],
				u4InArg[2]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}

		/* check the range for coverity */
		if (*cmd < HQA_BF_STR_SIZE) {
			if (SetATETxPacketWithBf(pAd, cmd) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " ATE_IBF_TX is failed!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto HQA_IBF_CMD_FAIL;
			}

		}

		break;
	case ATE_IBF_PROF_UPDATE:
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		/* Pfmu idx:Nr:Nc */
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x",
				u4InArg[0], u4InArg[1],
				u4InArg[2]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}

		/* check the range for coverity */
		if (*cmd < HQA_BF_STR_SIZE) {
			if (SetATEIBfProfileUpdate(pAd, cmd) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " ATE_IBF_PROF_UPDATE is failed!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto HQA_IBF_CMD_FAIL;
			}
		}
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		/* Wlan Id:EBf:IBf:Mu:PhaseCalFlg */
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "01:00:01:00:01");
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}

		if (Set_TxBfTxApply(pAd, cmd) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " IBF flag setting in WTBL is failed!!\n");
			ret = NDIS_STATUS_FAILURE;
			goto HQA_IBF_CMD_FAIL;
		}

		break;

	case ATE_EBF_PROF_UPDATE:
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		/* Pfmu idx:Nr:Nc */
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x",
				u4InArg[0], u4InArg[1],
				u4InArg[2]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}

		/* check the range for coverity */
		if (*cmd < HQA_BF_STR_SIZE) {
			if (SetATEEBfProfileConfig(pAd, cmd) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " ATE_EBF_PROF_UPDATE is failed!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto HQA_IBF_CMD_FAIL;
			}

		}

		break;

	case ATE_IBF_INST_CAL:
		u4Op_mode = TESTMODE_GET_PARAM(pAd, control_band_idx, op_mode);
		u4Op_mode |= fATE_IN_BF;
		TESTMODE_SET_PARAM(pAd, control_band_idx, op_mode, u4Op_mode);
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		/* Group idx:Group_L_M_H:fgSX2:Calibration type:Lna level */
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x:%02x",
				u4InArg[0], u4InArg[1],
				u4InArg[2], u4InArg[3],
				u4InArg[4]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}

		/* check the range for coverity */
		if (*cmd < HQA_BF_STR_SIZE) {
			if (SetATEIBfInstCal(pAd, cmd) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " ATE_IBF_GD_CAL is failed!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto HQA_IBF_CMD_FAIL;
			}

		}

		break;

	case ATE_IBF_INST_VERIFY:
		u4Op_mode = TESTMODE_GET_PARAM(pAd, control_band_idx, op_mode);
		u4Op_mode |= fATE_IN_BF;
		TESTMODE_SET_PARAM(pAd, control_band_idx, op_mode, u4Op_mode);
		u4InArg[3] = 4; /* iBF phase verification with instrument */
		u4InArg[4] = 1; /* Force LNA gain is middle gain */
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		/* Group idx:Group_L_M_H:fgSX2:Calibration type:Lna level */
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x:%02x:%02x",
				u4InArg[0], u4InArg[1],
				u4InArg[2], u4InArg[3],
				u4InArg[4]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}

		/* check the range for coverity */
		if (*cmd < HQA_BF_STR_SIZE) {
			if (SetATEIBfInstCal(pAd, cmd) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " ATE_IBF_INST_VERIFY is failed!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto HQA_IBF_CMD_FAIL;
			}

		}

		break;

	case ATE_TXBF_GD_INIT:
		break;

	case ATE_IBF_PHASE_E2P_UPDATE:
		memset(cmd, 0x00, HQA_BF_STR_SIZE);
		/* Group idx:fgSX2:E2P update type */
		snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%02x:%02x:%02x",
				u4InArg[0], u4InArg[1],
				u4InArg[2]);
		if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
			MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				"cmd snprintf error!\n");
			os_free_mem(cmd);
			return FALSE;
		}
		pAd->fgCalibrationFail = FALSE; /* Enable EEPROM write of calibrated phase */

		/* check the range for coverity */
		if (*cmd < HQA_BF_STR_SIZE) {
			if (SetATETxBfPhaseE2pUpdate(pAd, cmd) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " ATE_IBF_PHASE_E2P_UPDATE is failed!!\n");
				ret = NDIS_STATUS_FAILURE;
				goto HQA_IBF_CMD_FAIL;
			}

		}

		break;

	default:
		break;
	}

	/* MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "Action ID : %d, str:%s\n", u4Action, cmd); */
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
HQA_IBF_CMD_FAIL:
	if (cmd)
		os_free_mem(cmd);
	ResponseToQA(cmd_frame, wrq, 6, ret);
	return ret;
}


static INT32 hqa_iBFChanProfUpdate_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	INT snprintf_ret;
	UCHAR *data = cmd_frame->Data;
	UINT32 ext_id = 0;
	UINT32 u4PfmuId = 0, u4Subcarr = 0, fgFinalData = 0;
	UINT32 i2H11 = 0, i2AngleH11 = 0, i2H21 = 0, i2AngleH21 = 0, i2H31 = 0;
	UINT32 i2AngleH31 = 0, i2H41 = 0, i2AngleH41 = 0;
	RTMP_STRING *cmd = NULL;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		ret = NDIS_STATUS_RESOURCES;
		goto HQA_PROFILE_UPDATE_FAIL;
	}

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4PfmuId);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4Subcarr);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&fgFinalData);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2H11);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2AngleH11);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2H21);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2AngleH21);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2H31);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2AngleH31);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2H41);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2AngleH41);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x",
			u4PfmuId, u4Subcarr, fgFinalData, i2H11, i2AngleH11, i2H21, i2AngleH21,
			i2H31, i2AngleH31, i2H41, i2AngleH41);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}

	/* check the range for coverity */
	if (*cmd < HQA_BF_STR_SIZE) {
		if (SetATETxBfChanProfileUpdate(pAd, cmd) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " SetATETxBfChanProfileUpdate is failed!!\n");
			ret = NDIS_STATUS_FAILURE;
			goto HQA_PROFILE_UPDATE_FAIL;
		}
	}

	/* MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "str:%s\n", cmd); */
	os_free_mem(cmd);
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
HQA_PROFILE_UPDATE_FAIL:
	ResponseToQA(cmd_frame, wrq, 6, ret);
	return ret;
}


static INT32 hqa_iBFChanProfUpdateAll_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UCHAR *data = cmd_frame->Data;
	UINT32 ext_id = 0;
	UINT32 u4PfmuId = 0, u4Temp = 0;

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4PfmuId);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4Temp);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4Temp);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4Temp);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4Temp);

	if (TxBfProfileDataWrite20MAll(pAd, u4PfmuId, data) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, " SetATETxBfChanProfileUpdate is failed!!\n");
		ret = NDIS_STATUS_FAILURE;
		goto HQA_PROFILE_UPDATE_FAIL;
	}

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "str:%d\n", u4PfmuId);
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
HQA_PROFILE_UPDATE_FAIL:
	ResponseToQA(cmd_frame, wrq, 6, ret);
	return ret;
}


static INT32 hqa_iBFProfileRead_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	INT snprintf_ret;
	UCHAR *data = cmd_frame->Data;
	UINT32 ext_id = 0;
	UINT32 u4PfmuId = 0, u4Subcarr = 0;
	RTMP_STRING *cmd = NULL;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR) * (HQA_BF_STR_SIZE));

	if (!cmd) {
		ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_DNC_FAIL;
	}

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4PfmuId);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4Subcarr);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	snprintf_ret = snprintf(cmd, HQA_BF_STR_SIZE, "%03x:%03x", (UCHAR)u4PfmuId, (UCHAR)u4Subcarr);
	if (os_snprintf_error(HQA_BF_STR_SIZE, snprintf_ret)) {
		MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			"cmd snprintf error!\n");
		os_free_mem(cmd);
		return FALSE;
	}
	/* check the range for coverity */
	if (*cmd < HQA_BF_STR_SIZE) {
		SetATETxBfProfileRead(pAd, cmd);
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "str:%s\n", cmd);
	}

	os_free_mem(cmd);
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
	NdisMoveMemory(cmd_frame->Data + 6, (UCHAR *)&pAd->prof, sizeof(PFMU_DATA));
HQA_TAG_DNC_FAIL:
	ResponseToQA(cmd_frame, wrq, (6 + sizeof(PFMU_DATA)), ret);
	return ret;
}
#endif /* TXBF_SUPPORT */


static HQA_CMD_HANDLER hqa_ext_cmd_set[] = {
	NULL,
	hqa_set_channel_ext,		/* 0x00000001 */
	hqa_set_txcontent_ext,		/* 0x00000002 */
	hqa_start_tx_ext,		/* 0x00000003 */
	hqa_start_rx_ext,		/* 0x00000004 */
	hqa_stop_tx_ext,		/* 0x00000005 */
	hqa_stop_rx_ext,		/* 0x00000006 */
	NULL,				/* 0x00000007 */
#ifdef TXBF_SUPPORT
	hqa_iBFSetValue_ext,		/* 0x00000008 */
	hqa_iBFGetStatus_ext,		/* 0x00000009 */
	hqa_iBFChanProfUpdate_ext,	/* 0x0000000A */
	hqa_iBFProfileRead_ext,		/* 0x0000000B */
	hqa_iBFChanProfUpdateAll_ext,	/* 0x0000000C */
#endif /* TXBF_SUPPORT */
	NULL,				/* 0x0000000D */
	NULL,				/* 0x0000000E */
	NULL,				/* 0x0000000F */
	NULL,				/* 0x00000010 */
	NULL,				/* 0x00000011 */
	NULL,				/* 0x00000012 */
	NULL,				/* 0x00000013 */
	NULL,				/* 0x00000014 */
	NULL,				/* 0x00000015 */
	NULL,				/* 0x00000016 */
	NULL,				/* 0x00000017 */
	NULL,				/* 0x00000018 */
	NULL,				/* 0x00000019 */
	NULL,				/* 0x0000001A */
	NULL,				/* 0x0000001B */
	NULL,				/* 0x0000001C */
	NULL,				/* 0x0000001D */
	NULL,				/* 0x0000001E */
	NULL,				/* 0x0000001F */
	NULL,				/* 0x00000020 */
	NULL,				/* 0x00000021 */
	NULL,				/* 0x00000022 */
	NULL,				/* 0x00000023 */
	NULL,				/* 0x00000024 */
	NULL,				/* 0x00000025 */
	hqa_set_tx_time,	/* 0x00000026 */
	hqa_off_ch_scan,	/* 0x00000027 */
};


static INT32 hqa_ext_cmds(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, struct _HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	INT32 idx = 0;

	NdisMoveMemory((PUCHAR)&idx, (PUCHAR)&cmd_frame->Data, 4);
	idx = PKTL_TRAN_TO_HOST(idx);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "0x%x\n", idx);

	if (idx >= (sizeof(hqa_ext_cmd_set)/sizeof(HQA_CMD_HANDLER)) || idx < 0) {
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"cmd idx 0x%x is over bounded\n", idx);
		return ret;
	}


	if (hqa_ext_cmd_set[idx] != NULL)
		ret = (*hqa_ext_cmd_set[idx])(pAd, wrq, cmd_frame);
	else
		MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"cmd idx 0x%x is not supported\n", idx);

	return ret;
}


static HQA_CMD_HANDLER HQA_CMD_SET6[] = {
	/* cmd id start from 0x1600 */
	hqa_ext_cmds,	/* 0x1600 */
};


static INT32	HQA_MCU_RegRead(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	return Ret;
}

static INT32	HQA_MCU_RegWrite(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, "\n");
	return Ret;
}


static INT32	HQA_MCUTest(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT16	ntStatus = 0x00;
	return ntStatus;
}


static HQA_CMD_HANDLER HQA_CMD_SET7[] = {
	/* cmd id start from 0x2100 */
	HQA_MCU_RegRead,				/* 0x2100 */
	HQA_MCU_RegWrite,				/* 0x2101 */
	HQA_MCUTest,					/* 0x2102 */
};


static struct _HQA_CMD_TABLE HQA_CMD_TABLES[] = {
	{
		HQA_CMD_SET0,
		sizeof(HQA_CMD_SET0) / sizeof(HQA_CMD_HANDLER),
		0x1000,
	},
	{
		HQA_CMD_SET1,
		sizeof(HQA_CMD_SET1) / sizeof(HQA_CMD_HANDLER),
		0x1100,
	},
	{
		HQA_CMD_SET2,
		sizeof(HQA_CMD_SET2) / sizeof(HQA_CMD_HANDLER),
		0x1200,
	},
	{
		HQA_CMD_SET3,
		sizeof(HQA_CMD_SET3) / sizeof(HQA_CMD_HANDLER),
		0x1300,
	},
	{
		HQA_CMD_SET4,
		sizeof(HQA_CMD_SET4) / sizeof(HQA_CMD_HANDLER),
		0x1400,
	},
	{
		HQA_CMD_SET5,
		sizeof(HQA_CMD_SET5) / sizeof(HQA_CMD_HANDLER),
		0x1500,
	},
#ifdef TXBF_SUPPORT
	{
		HQA_TXBF_CMDS,
		sizeof(HQA_TXBF_CMDS) / sizeof(HQA_CMD_HANDLER),
		0x1540,
	},
#ifdef CFG_SUPPORT_MU_MIMO
	{
		HQA_TXMU_CMDS,
		sizeof(HQA_TXMU_CMDS) / sizeof(HQA_CMD_HANDLER),
		0x1560,
	},
#endif
#endif
	{
		HQA_ICAP_CMDS,
		sizeof(HQA_ICAP_CMDS) / sizeof(HQA_CMD_HANDLER),
		0x1580,
	},
	{
		HQA_ReCal_CMDS,
		sizeof(HQA_ReCal_CMDS) / sizeof(HQA_CMD_HANDLER),
		0x1581,
	},
	{
		HQA_RXV_CMDS,
		sizeof(HQA_RXV_CMDS) / sizeof(HQA_CMD_HANDLER),
		0x1582,
	},
	{
		HQA_RDD_CMDS,
		sizeof(HQA_RDD_CMDS) / sizeof(HQA_CMD_HANDLER),
		0x1583,
	},
#ifdef DOT11_HE_AX
	{
		HQA_HE_CMDS,
		sizeof(HQA_HE_CMDS) / sizeof(HQA_CMD_HANDLER),
		0x1590,
	},
#endif
	{
		HQA_CMD_SET6,
		sizeof(HQA_CMD_SET6) / sizeof(HQA_CMD_HANDLER),
		0x1600,
	},
	{
		HQA_CMD_SET7,
		sizeof(HQA_CMD_SET7) / sizeof(HQA_CMD_HANDLER),
		0x2100,
	},
};


UINT32 HQA_CMDHandler(
	RTMP_ADAPTER *pAd,
	RTMP_IOCTL_INPUT_STRUCT *Wrq,
	struct _HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Status = NDIS_STATUS_SUCCESS;
	UINT16 CmdId;
	UINT32 TableIndex = 0;
	UINT32 ATEMagicNum = 0;

	ATEMagicNum = PKTL_TRAN_TO_HOST(HqaCmdFrame->MagicNo);

	if (ATEMagicNum != HQA_CMD_MAGIC_NO)
		return TM_STATUS_NOTSUPPORT;

	CmdId = PKTS_TRAN_TO_HOST(HqaCmdFrame->Id);
	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_DEBUG,
			 "--> Command_Id = 0x%04x, testmode_ioctl\n", CmdId);

	while (TableIndex < (sizeof(HQA_CMD_TABLES) / sizeof(struct _HQA_CMD_TABLE))) {
		UINT32 CmdIndex = 0;

		CmdIndex = CmdId - HQA_CMD_TABLES[TableIndex].CmdOffset;

		if (CmdIndex < HQA_CMD_TABLES[TableIndex].CmdSetSize) {
			HQA_CMD_HANDLER *pCmdSet;

			pCmdSet = HQA_CMD_TABLES[TableIndex].CmdSet;

			if (pCmdSet[CmdIndex] != NULL)
				Status = (*pCmdSet[CmdIndex])(pAd, Wrq, HqaCmdFrame);

			break;
		}

		TableIndex++;
	}

	if (CmdId == HQA_CMD_REQ) {
		HqaCmdFrame->Type = HQA_CMD_RSP;
		/* HqaCmdFrame->Type = 0x8005; */
	} else
		HqaCmdFrame->Type = TM_CMDRSP;

	MTWF_DBG(pAd, DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 "Command_Id = 0x%04x, testmode_ioctl <--\n", CmdId);

	return Status;
}
