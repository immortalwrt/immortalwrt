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

#include "rt_config.h"
#include "hdev/hdev.h"

/*Omac controller*/

static INT32 GetFirstAvailableOmacIdx(struct hdev_ctrl *ctrl, BOOLEAN NeedBss, UINT32 OmacType, struct _OMAC_BSS_CTRL *omac_ctrl)
{
	UINT32 Index;
	struct _RTMP_CHIP_CAP *cap = &ctrl->chip_cap;

	if (NeedBss) {
		for (Index = 0; Index < cap->BssNums; Index++) {
#if defined(CONFIG_AP_SUPPORT)
			if ((OmacType == WDEV_TYPE_STA) && (!Index))
				continue;
#endif
			if ((omac_ctrl->OmacBitMap & (1 << Index)) == 0) {
				omac_ctrl->OmacBitMap |= (1 << Index);
				return Index;
			}
		}
	} else {
		for (Index = cap->BssNums; Index < cap->OmacNums; Index++) {
			if ((omac_ctrl->OmacBitMap & (1 << Index)) == 0) {
				omac_ctrl->OmacBitMap |= (1 << Index);
				return Index;
			}
		}
	}

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OmacIndex is not available\n");
	return -1;
}

static INT32 GetFirstAvailableRepeaterOmacIdx(struct hdev_ctrl *ctrl, struct _OMAC_BSS_CTRL *omac_ctrl)
{
	struct _RTMP_CHIP_CAP *cap = &ctrl->chip_cap;
	UCHAR i;

	for (i = 0; i < cap->MaxRepeaterNum; i++) {
		if ((omac_ctrl->RepeaterBitMap & (1 << i)) == 0) {
			omac_ctrl->RepeaterBitMap |= (1 << i);
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 "%s: found used OmacIndex:0x%x\n",
					  __func__, cap->RepeaterStartIdx + i);
			return cap->RepeaterStartIdx + i;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OmacIndex is not available\n");
	return -1;
}

INT32 GetFirstAvailableApOmacIdx(struct hdev_ctrl *ctrl, struct _OMAC_BSS_CTRL *omac_ctrl)
{
	INT32 Index;
	struct _RTMP_CHIP_CAP *cap = &ctrl->chip_cap;

	/*choose Hw Bss0 first*/
	if ((omac_ctrl->OmacBitMap & (1 << 0)) == 0) {
		omac_ctrl->OmacBitMap |= (1 << 0);
		return 0;
	}
	/*first ext ownmac is the same as HW bss0*/
	for (Index = 1; Index < cap->BcnMaxNum; Index++) {
		if ((omac_ctrl->HwMbssBitMap & (1 << Index)) == 0) {
			omac_ctrl->HwMbssBitMap |= (1 << Index);
			return cap->ExtMbssOmacStartIdx + Index;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Ext OmacIndex is not available\n");
	return -1;
}

VOID ReleaseOmacIdx(struct hdev_ctrl *ctrl, UINT32 OmacType, struct radio_dev *rdev, UINT32 Idx)
{
	struct _OMAC_BSS_CTRL *omac_ctrl = rdev->omac_ctrl;
	struct _RTMP_CHIP_CAP *cap = &ctrl->chip_cap;

	if(!omac_ctrl){
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "omac_ctrl NULL!\n");
		return;
	}

	switch (OmacType) {
	case WDEV_TYPE_AP:
	case WDEV_TYPE_ATE_AP:
	case WDEV_TYPE_SERVICE_TXC:
		if (Idx == 0) {
			omac_ctrl->OmacBitMap &= ~(1 << 0);
			return;
		} else {
			omac_ctrl->HwMbssBitMap &= ~(1 << (Idx - cap->ExtMbssOmacStartIdx));
			return;
		}

		break;

	case WDEV_TYPE_STA:
	case WDEV_TYPE_ADHOC:
	case WDEV_TYPE_GO:
	case WDEV_TYPE_GC:
	case WDEV_TYPE_P2P_DEVICE:
	case WDEV_TYPE_ATE_STA:
	case WDEV_TYPE_SERVICE_TXD:
		omac_ctrl->OmacBitMap &= ~(1 << Idx);
		return;
		break;

	/*
	    Carter note,
	    because of WDS0~WDS3 use the same mac address as RA0,
	    so the OmacIdx should use ra0's.

	    otherwise, if the pkt sa is ra0's but omacIdx use ra1's,
	    it will cause the ack from peer could not be passed at Rmac.

	    and WDS OmacIdx should not be released, unless Ra0 is released.
	*/
	case WDEV_TYPE_WDS:
		break;

	case WDEV_TYPE_MESH:
		/* TODO */
		break;

	case WDEV_TYPE_REPEATER:
		omac_ctrl->RepeaterBitMap &= ~(1 << (Idx - cap->RepeaterStartIdx));
		return;
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OmacType(%d)\n", OmacType);
		break;
	}

	return;
}

INT32 GetOmacIdx(struct hdev_ctrl *ctrl, UINT32 OmacType, struct radio_dev *rdev, INT8 Idx)
{
	struct _OMAC_BSS_CTRL *omac_ctrl = rdev->omac_ctrl;

	switch (OmacType) {
	case WDEV_TYPE_AP:
	case WDEV_TYPE_ATE_AP:
	case WDEV_TYPE_SERVICE_TXC:
		return GetFirstAvailableApOmacIdx(ctrl, omac_ctrl);
		break;

	case WDEV_TYPE_STA:
	case WDEV_TYPE_ADHOC:
	case WDEV_TYPE_GO:
	case WDEV_TYPE_GC:
	case WDEV_TYPE_ATE_STA:
	case WDEV_TYPE_SERVICE_TXD:
		return GetFirstAvailableOmacIdx(ctrl, TRUE, OmacType, omac_ctrl);
		break;

	/*
	    Carter note,
	    because of WDS0~WDS3 use the same mac address as RA0,
	    so the OmacIdx should use ra0's.

	    otherwise, if the pkt sa is ra0's but omacIdx use ra1's,
	    it will cause the ack from peer could not be passed at Rmac.
	*/
	case WDEV_TYPE_WDS:
		return 0;
		break;

	case WDEV_TYPE_MESH:
		/* TODO */
		break;

	case WDEV_TYPE_REPEATER:
		return GetFirstAvailableRepeaterOmacIdx(ctrl, omac_ctrl);
		break;

	case WDEV_TYPE_P2P_DEVICE:
		return GetFirstAvailableOmacIdx(ctrl, FALSE, OmacType, omac_ctrl);
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "OmacType(%d)\n", OmacType);
		break;
	}

	return -1;
}


/*
*
*/
INT32 OcAddRepeaterEntry(struct hdev_obj *obj, UCHAR ReptIdx)
{
	INT32 ret = 0;
	HD_REPT_ENRTY *pReptEntry = NULL;
	struct radio_dev *rdev = obj->rdev;
	struct hdev_ctrl *ctrl = rdev->priv;

	ret = os_alloc_mem(NULL, (UCHAR **)&pReptEntry, sizeof(HD_REPT_ENRTY));

	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 " Alloc memory for HD_REPT_ENRTY failed.\n");
		return ret;
	}

	OS_SPIN_LOCK(&obj->RefCntLock);
	obj->RefCnt++;
	OS_SPIN_UNLOCK(&obj->RefCntLock);
	pReptEntry->CliIdx = ReptIdx;
	pReptEntry->ReptOmacIdx = GetOmacIdx(ctrl, WDEV_TYPE_REPEATER, rdev, ReptIdx);
	DlListAddTail(&obj->RepeaterList, &pReptEntry->list);
	return NDIS_STATUS_SUCCESS;
}


/*
*
*/
VOID OcDelRepeaterEntry(struct hdev_obj *obj, UCHAR ReptIdx)
{
	HD_REPT_ENRTY *pReptEntry = NULL, *tmp = NULL;
	struct radio_dev *rdev = obj->rdev;
	struct hdev_ctrl *ctrl = rdev->priv;

	DlListForEachSafe(pReptEntry, tmp, &obj->RepeaterList, struct _HD_REPT_ENRTY, list) {
		if (pReptEntry->CliIdx == ReptIdx) {
			OS_SPIN_LOCK(&obj->RefCntLock);

			if (obj->RefCnt > 0)
				obj->RefCnt--;
			else {
				MTWF_DBG(NULL, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN,
						 "bug here? RefCnt zero already.\n");
			}

			OS_SPIN_UNLOCK(&obj->RefCntLock);
			ReleaseOmacIdx(ctrl, WDEV_TYPE_REPEATER, rdev, pReptEntry->ReptOmacIdx);
			DlListDel(&pReptEntry->list);
			os_free_mem(pReptEntry);
		}
	}
}

HD_REPT_ENRTY *OcGetRepeaterEntry(struct hdev_obj *obj, UCHAR ReptIdx)
{
	HD_REPT_ENRTY *pReptEntry = NULL;

	DlListForEach(pReptEntry, &obj->RepeaterList, struct _HD_REPT_ENRTY, list) {
		if (pReptEntry->CliIdx == ReptIdx)
			return pReptEntry;
	}
	return NULL;
}

