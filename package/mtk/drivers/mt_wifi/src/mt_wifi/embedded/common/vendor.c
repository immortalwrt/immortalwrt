#include "rt_config.h"

static VOID show_format(VOID)
{
	MTWF_PRINT("\nCommand Format: 'op'-frm_map:'bitmap'-oui:'xxxxxx'-length:'len'-ctnt:'xxxxxx'\n");
	MTWF_PRINT("op: 1: ADD, 2: UPDATE, 3: REMOVE, 4: SHOW\n");
	MTWF_PRINT("\nbitmap: 0x1: BEACON, 0x2: PROBE_REQ, 0x4: PROBE_RESP, 0x8: ASSOC_REQ\n");
	MTWF_PRINT("\t 0x10: ASSOC_RESP, 0x20: AUTH_REQ, 0x40: AUTH_RESP\n");
	MTWF_PRINT("\noui: in hex format, such as 000c43\n");
	MTWF_PRINT("\nlength: the total length of oui and content\n");
	MTWF_PRINT("\nctnt: in hex format, such as aabbcc\n");

	MTWF_PRINT("\nCase 1: Add or Update\n");
	MTWF_PRINT("\tiwpriv ra0 set vie_op=1-frm_map:1-oui:00aabb-length:4-ctnt:cc\n");
	MTWF_PRINT("\nCase 2: Remove\n");
	MTWF_PRINT("\tiwpriv ra0 set vie_op=3-frm_map:1-oui:00aabb-length:4-ctnt:cc\n");
	MTWF_PRINT("\nCase 3: Show\n");
	MTWF_PRINT("\tiwpriv ra0 set vie_op=4-frm_map:1\n");
}

VOID print_vie(struct wifi_dev *wdev, UINT32 frm_map)
{
	struct vie_struct *current_vie = NULL;
	VIE_CTRL *vie_ctrl = NULL;
	int i;
	int index;

	for (i = VIE_BEACON; i < VIE_FRM_TYPE_MAX; i++) {
		vie_ctrl = &wdev->vie_ctrl[i];

		MTWF_PRINT("\nFrm_Type:%s, vie_num:%d\n",
				(i == VIE_BEACON) ? "Beacon" :
				(i == VIE_PROBE_REQ) ? "Probe_Req" :
				(i == VIE_PROBE_RESP) ? "Probe_Resp" :
				(i == VIE_ASSOC_REQ) ? "Assoc_Req" :
				(i == VIE_ASSOC_RESP) ? "Assoc_Resp" :
				(i == VIE_AUTH_REQ) ? "Auth_Req" : "Auth_Resp",
				vie_ctrl->vie_num);

		current_vie = vie_ctrl->vie_in_frm;
		index = 0;
		while (current_vie) {
			MTWF_PRINT("vie_index:%d oui:0x%02x 0x%02x 0x%02x\n",
				  index,
				  current_vie->oui_oitype[0],
				  current_vie->oui_oitype[1],
				  current_vie->oui_oitype[2]);

			hex_dump("ie_content:", current_vie->ie_ctnt, current_vie->vie_length);
			current_vie = current_vie->next_vie;
			index++;
		}
	}
}

static INT vie_sanity_check(UINT32 oper, UINT32 input_argument, UINT32 frm_map, UINT32 length, UCHAR *oui, UCHAR *ctnt)
{
	INT ret = NDIS_STATUS_SUCCESS;

	if (oper >= VIE_OPER_MAX) {
		MTWF_DBG(NULL, DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"oper error:%d\n", oper);
		ret = NDIS_STATUS_FAILURE;
		goto err;
	}

	if (((oper != VIE_SHOW) && (input_argument != 5)) ||
	     ((oper == VIE_SHOW) && (input_argument != 2))) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"oper:%d, input_argument:%d\n", oper, input_argument);
		ret = NDIS_STATUS_FAILURE;
		goto err;
	}

	if (frm_map >= VIE_FRM_TYPE_MAX_BITMAP) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			"frm_map error:0x%x\n", frm_map);
		ret = NDIS_STATUS_FAILURE;
		goto err;
	}

	if (oper != VIE_SHOW) {
		if (strlen(oui) != OUI_LEN * 2) {
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				"oui format error:%s, should be xxxxxx!\n", oui);
			ret = NDIS_STATUS_FAILURE;
			goto err;
		}

		if (strlen(ctnt) != (length * 2) - strlen(oui)) {
			MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				"oui len + ctnt length:%d != input length:%d!\n",
				(UINT32)(strlen(ctnt) / 2),  length);
			ret = NDIS_STATUS_FAILURE;
			goto err;
		}
	}
err:

	return ret;
}

INT vie_oper_proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING * arg)
{
	INT ret = TRUE;
	struct wifi_dev *wdev = NULL;
	struct os_cookie *os_obj;
	UINT32 oper = 0;
	UINT32 length = 0;
	UINT32 input_argument = 0;
	UINT32 frm_map = 0;
	UINT32 oui_oitype = 0;
	UCHAR oui[(OUI_LEN * 2) + 1] = {0};
	UCHAR *ctnt;
	UCHAR *ie_hex_ctnt;

	os_alloc_mem(pAd, (UCHAR **)&ctnt, (MAX_VENDOR_IE_LEN + 1) * 2);
	os_alloc_mem(pAd, (UCHAR **)&ie_hex_ctnt, MAX_VENDOR_IE_LEN);

	if (!ctnt || !ie_hex_ctnt)
		goto end;

	os_zero_mem(ctnt, (MAX_VENDOR_IE_LEN + 1) * 2);
	os_zero_mem(ie_hex_ctnt, MAX_VENDOR_IE_LEN);

	os_obj = (struct os_cookie *)pAd->OS_Cookie;
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (os_obj->ioctl_if >= 0 && os_obj->ioctl_if < MAX_MULTI_STA) {
			wdev = &pAd->StaCfg[os_obj->ioctl_if].wdev;
		} else {
			ret = FALSE;
			goto end;
		}
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (os_obj->ioctl_if >= 0 && os_obj->ioctl_if < MAX_BEACON_NUM) {
			wdev = &pAd->ApCfg.MBSSID[os_obj->ioctl_if].wdev;
		} else {
			ret = FALSE;
			goto end;
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	if (arg) {
		input_argument = sscanf(arg,
					"%d-frm_map:%x-oui:%6s-length:%d-ctnt:%s",
					&oper, &frm_map, oui, &length, ctnt);

		oui[OUI_LEN * 2] = '\0';
		if (vie_sanity_check(oper,
				     input_argument,
				     frm_map,
				     length,
				     oui,
				     ctnt) == NDIS_STATUS_FAILURE) {
			ret = FALSE;
			show_format();
			goto end;
		}

		MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_NOTICE,
				"oper:%d, frm_map:0x%x, oui:%s, length:%d, ctnt:%s\n",
				oper, frm_map, oui, length, ctnt);

		if ((oper == VIE_ADD) || (oper == VIE_UPDATE) || (oper == VIE_REMOVE)) {
			/*add/update case*/
			AtoH(oui, ie_hex_ctnt, OUI_LEN);
			AtoH(ctnt, ie_hex_ctnt + OUI_LEN, length - OUI_LEN);
			NdisMoveMemory(&oui_oitype, ie_hex_ctnt, sizeof(UINT32));

			if (oper == VIE_REMOVE) {
				if (remove_vie(pAd,
					       wdev,
					       frm_map,
					       oui_oitype,
					       length,
					       ie_hex_ctnt) == NDIS_STATUS_FAILURE) {
					ret = FALSE;
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						"remove failed.\n");
				}
			} else {
				if (add_vie(pAd,
					    wdev,
					    frm_map,
					    oui_oitype,
					    length,
					    ie_hex_ctnt) == NDIS_STATUS_FAILURE) {
					ret = FALSE;
					MTWF_DBG(pAd, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							"%s failed.\n",
							(oper == VIE_ADD) ? "add" : "update");
				}
			}
		} else if (oper == VIE_SHOW)
			print_vie(wdev, frm_map);
	}

end:
	if (ctnt)
		os_free_mem(ctnt);
	if (ie_hex_ctnt)
		os_free_mem(ie_hex_ctnt);
	return ret;
}

static VOID get_vie_ctrl(struct wifi_dev *wdev, VIE_CTRL **vie_ctrl, UINT32 frm_type_bitmap)
{
	switch (frm_type_bitmap) {
	case VIE_BEACON_BITMAP:
		*vie_ctrl = &wdev->vie_ctrl[VIE_BEACON];
		break;
	case VIE_PROBE_REQ_BITMAP:
		*vie_ctrl = &wdev->vie_ctrl[VIE_PROBE_REQ];
		break;
	case VIE_PROBE_RESP_BITMAP:
		*vie_ctrl = &wdev->vie_ctrl[VIE_PROBE_RESP];
		break;
	case VIE_ASSOC_REQ_BITMAP:
		*vie_ctrl = &wdev->vie_ctrl[VIE_ASSOC_REQ];
		break;
	case VIE_ASSOC_RESP_BITMAP:
		*vie_ctrl = &wdev->vie_ctrl[VIE_ASSOC_RESP];
		break;
	case VIE_AUTH_REQ_BITMAP:
		*vie_ctrl = &wdev->vie_ctrl[VIE_AUTH_REQ];
		break;
	case VIE_AUTH_RESP_BITMAP:
		*vie_ctrl = &wdev->vie_ctrl[VIE_AUTH_RESP];
		break;
	default:
		break;
	}
}

static INT32 insert_vie(struct wifi_dev *wdev,
			struct vie_struct **out_vie,
			UINT32 frm_type_index,
			UINT32 oui_oitype,
			ULONG ie_length,
			UCHAR *frame_buffer)
{
	struct vie_struct *vie = NULL;
	struct vie_struct *current_frm_tail_vie = NULL;
	INT32 ret = NDIS_STATUS_SUCCESS;
	VIE_CTRL *vie_ctrl = NULL;
	UINT32 local_oui_oitype = oui_oitype;

	if (os_alloc_mem(NULL, (UCHAR **)&vie, sizeof(struct vie_struct)) == NDIS_STATUS_FAILURE) {
		ret = NDIS_STATUS_FAILURE;
		return ret;
	}

	if (os_alloc_mem(NULL, (UCHAR **)&vie->ie_ctnt, ie_length) == NDIS_STATUS_FAILURE) {
		ret = NDIS_STATUS_FAILURE;
		os_free_mem(vie);
		return ret;
	} else
		NdisMoveMemory(vie->ie_ctnt, frame_buffer, ie_length);

	vie->vie_length = ie_length;
	NdisMoveMemory(vie->oui_oitype, &local_oui_oitype, sizeof(local_oui_oitype));
	vie->next_vie = NULL;

	get_vie_ctrl(wdev, &vie_ctrl, frm_type_index);
	if (vie_ctrl == NULL) {
		ret = NDIS_STATUS_FAILURE;
		os_free_mem(vie->ie_ctnt);
		os_free_mem(vie);
		return ret;
	}

	current_frm_tail_vie = vie_ctrl->vie_in_frm;
	if (vie_ctrl->vie_num > 0) {
		if (!current_frm_tail_vie) {
			ret = NDIS_STATUS_FAILURE;
			os_free_mem(vie->ie_ctnt);
			os_free_mem(vie);
			return ret;
		}

		while (current_frm_tail_vie) {
			if (current_frm_tail_vie->next_vie)
				current_frm_tail_vie = current_frm_tail_vie->next_vie;
			else {
				current_frm_tail_vie->next_vie = vie;
				break;
			}
		}
	} else
		vie_ctrl->vie_in_frm = vie;

	vie_ctrl->vie_num++;

	*out_vie = NULL;

	return ret;
}

BOOLEAN find_oui_oitype(struct wifi_dev *wdev, UINT frm_type_bit, UINT32 oui_oitype, struct vie_struct **vie)
{
	BOOLEAN found = FALSE;

	struct vie_struct *current_vie = NULL;
	VIE_CTRL *vie_ctrl = NULL;
	UINT32 local_oui_oitype = oui_oitype;

	get_vie_ctrl(wdev, &vie_ctrl, frm_type_bit);
	if (vie_ctrl == NULL) {
		/*FATAL ERROR shall not happen.*/
		return found;
	}

	current_vie = vie_ctrl->vie_in_frm;
	while (current_vie) {
		if (NdisEqualMemory(current_vie->oui_oitype, (UCHAR *)&local_oui_oitype, sizeof(UINT32))) {
			*vie = current_vie;
			found = TRUE;
			break;
		}

		current_vie = current_vie->next_vie;
	}
	return found;
}

INT32 add_vie(struct _RTMP_ADAPTER *pAd,
	      struct wifi_dev *wdev,
	      UINT32 frm_type_map,
	      UINT32 oui_oitype,
	      ULONG ie_length,
	      UCHAR *frame_buffer)
{
	struct vie_struct *vie = NULL;
	UINT frm_type;
	UINT frm_type_mask = 0;
	UINT frm_type_bit = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;

	/*
	 * check if there is the same oui and oi_type vie exists already, if it does, update it.
	 * the cons is, it only accepts one group of same oui and oitype.
	 */
	for (frm_type = VIE_BEACON; frm_type < VIE_FRM_TYPE_MAX; frm_type++) {
		frm_type_mask = (1 << frm_type);
		frm_type_bit = frm_type_map & frm_type_mask;
		if (frm_type_bit) {
			if (find_oui_oitype(wdev, frm_type_bit, oui_oitype, &vie) == TRUE) {
				/*we found a duplicate oui and oi type, update it.*/
				vie->vie_length = ie_length;
				/*the check should be not necessary, but check it anyway.*/
				if (vie->ie_ctnt)
					os_free_mem(vie->ie_ctnt);

				if (os_alloc_mem(pAd, (UCHAR **)&vie->ie_ctnt, ie_length) == NDIS_STATUS_FAILURE) {
					ret = NDIS_STATUS_FAILURE;
					goto end;
				} else
					NdisMoveMemory(vie->ie_ctnt, frame_buffer, ie_length);
			} else {
				/*cannot find a exist oui_oitype in this kind of frm_type pkt. add a new one in it.*/
				if (insert_vie(wdev,
					       &vie,
					       frm_type_bit,
					       oui_oitype,
					       ie_length,
					       frame_buffer) == NDIS_STATUS_FAILURE) {
					ret = NDIS_STATUS_FAILURE;
					goto end;
				}
			}
		}
	}

end:
	return ret;
}

static VOID do_delete_vie(VIE_CTRL *vie_ctrl, struct vie_struct *remove_vie)
{
	struct vie_struct *head_vie = NULL;
	struct vie_struct *prev_vie = NULL;
	struct vie_struct *curr_vie = NULL;

	head_vie = vie_ctrl->vie_in_frm;
	if (remove_vie == head_vie) {
		vie_ctrl->vie_in_frm = head_vie->next_vie;
	} else {
		curr_vie = head_vie->next_vie;
		prev_vie = head_vie;

		while (curr_vie) {
			if (curr_vie == remove_vie) {
				prev_vie->next_vie = curr_vie->next_vie;
				break;
			}
			prev_vie = curr_vie;
			curr_vie = curr_vie->next_vie;
		}
	}

	if (remove_vie) {
		if (remove_vie->ie_ctnt)
			os_free_mem(remove_vie->ie_ctnt);
		os_free_mem(remove_vie);
	}
	vie_ctrl->vie_num--;
}

INT32 remove_vie(struct _RTMP_ADAPTER *pAd,
		 struct wifi_dev *wdev,
		 UINT32 frm_type_map,
		 UINT32 oui_oitype,
		 ULONG ie_length,
		 UCHAR *frame_buffer)
{
	struct vie_struct *vie = NULL;
	UINT frm_type;
	UINT frm_type_mask = 0;
	UINT frm_type_bit = 0;
	INT32 ret = NDIS_STATUS_SUCCESS;
	VIE_CTRL *vie_ctrl = NULL;

	for (frm_type = VIE_BEACON; frm_type < VIE_FRM_TYPE_MAX; frm_type++) {
		frm_type_mask = (1 << frm_type);
		frm_type_bit = frm_type_map & frm_type_mask;
		if (frm_type_bit) {
			get_vie_ctrl(wdev, &vie_ctrl, frm_type_bit);
			if (vie_ctrl == NULL) {
				ret = NDIS_STATUS_FAILURE;
				return ret;
			}
			if (find_oui_oitype(wdev, frm_type_bit, oui_oitype, &vie) == TRUE)
				do_delete_vie(vie_ctrl, vie);
		}
	}

	return ret;
}

VOID init_vie_ctrl(struct wifi_dev *wdev)
{
	int i;
	VIE_CTRL *vie_ctrl = NULL;
	struct vie_struct *vie = NULL;
	struct vie_struct *temp_vie = NULL;

	for (i = 0; i < VIE_FRM_TYPE_MAX; i++) {
		vie_ctrl = &wdev->vie_ctrl[i];
		vie = vie_ctrl->vie_in_frm;
		vie_ctrl->type = i;
		vie_ctrl->vie_num = 0;

		/*should be not necessary for this step.*/
		while (vie != NULL) {
			if (vie->ie_ctnt != NULL) {
				os_free_mem(vie->ie_ctnt);
				vie->ie_ctnt = NULL;
			}
			temp_vie = vie->next_vie;
			vie->next_vie = NULL;
			os_free_mem(vie);

			vie = temp_vie;
		}
		vie_ctrl->vie_in_frm = NULL;
	}
}

VOID deinit_vie_ctrl(struct wifi_dev *wdev)
{
	int i;
	VIE_CTRL *vie_ctrl = NULL;
	struct vie_struct *vie = NULL;
	struct vie_struct *temp_vie = NULL;

	for (i = 0; i < VIE_FRM_TYPE_MAX; i++) {
		vie_ctrl = &wdev->vie_ctrl[i];
		vie = vie_ctrl->vie_in_frm;
		vie_ctrl->type = 0;

		while ((vie != NULL) && (vie_ctrl->vie_num > 0)) {
			if (vie->ie_ctnt != NULL) {
				os_free_mem(vie->ie_ctnt);
				vie->ie_ctnt = NULL;
			}
			temp_vie = vie->next_vie;
			vie->next_vie = NULL;
			os_free_mem(vie);
			vie_ctrl->vie_num--;

			vie = temp_vie;
		}
		vie_ctrl->vie_in_frm = NULL;
	}
}


ULONG build_vendor_ie(struct _RTMP_ADAPTER *pAd,
		      struct wifi_dev *wdev,
		      UCHAR *frame_buffer,
		      VIE_FRM_TYPE vie_frm_type
		     )
{
	struct _ralink_ie ra_ie;
	ULONG ra_ie_len = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	VIE_CTRL *vie_ctrl = NULL;
	ULONG app_vie_len;
	struct vie_struct *vie = NULL;
	struct _generic_vie_t vendor_ie;

#ifdef MT_MAC
	struct _mediatek_ie mtk_ie;
	ULONG mtk_ie_len = 0;
	struct _mediatek_vht_ie mtk_vht_ie;
	ULONG mtk_vht_ie_len = 0;
	UCHAR mtk_vht_cap[] = {
		0xBF, /* EID */
		0x0C, /* LEN */
		0xB1, 0x01, 0xC0, 0x33, /* VHT Cap. Info. */
		0x2A, 0xFF, 0x92, 0x04, 0x2A, 0xFF, 0x92, 0x04 /* Supported MCS and Nss */
	};
	UCHAR mtk_vht_op[] = {
		0xC0, /* EID */
		0x05, /* LEN */
		0x0, 0x0, 0x0, /* VHT Op. Info. */
		0x2A, 0xFF /* Basic MCS and Nss */
	};
	UCHAR mtk_vht_txpwr_env[] = {
		0xC3, /* EID */
		0x03, /* LEN */
		0x01, /* TX PWR Info. */
		0x02, 0x02 /* Max. Power for 20 & 40Mhz */
	};
#endif /* MT_MAC */
#ifdef VHT_TXBF_2G_EPIGRAM_IE
	UCHAR brcm_txbf2g_fp[] = {
		0x04, 0x18, 0xBF, 0x0C, 0xB2, 0x69,
		0x8B, 0x0F, 0xAA, 0xFF, 0x00, 0x00,
		0xAA, 0xFF, 0x00, 0x00, 0xC0, 0x05,
		0x00, 0x05, 0x00, 0x00, 0x00
	};
	struct _broadcom_txbf2g_ie brcm_txbf2g_ie;
	ULONG brcm_txbf2g_ie_len = 0;
#endif /* VHT_TXBF_2G_EPIGRAM_IE */
	ULONG vendor_ie_len = 0;
	NdisZeroMemory(&ra_ie, sizeof(struct _ralink_ie));
	ra_ie.ie_hdr.eid = IE_VENDOR_SPECIFIC;
	ra_ie.ie_hdr.len = 0x7;
	ra_ie.oui[0] = 0x00;
	ra_ie.oui[1] = 0x0C;
	ra_ie.oui[2] = 0x43;

	if (pAd->CommonCfg.bAggregationCapable)
		ra_ie.cap0 |= RALINK_AGG_CAP;

	if (pAd->CommonCfg.bPiggyBackCapable)
		ra_ie.cap0 |= RALINK_PIGGY_CAP;

	if (pAd->CommonCfg.bRdg)
		ra_ie.cap0 |= RALINK_RDG_CAP;

	if (pAd->CommonCfg.g_band_256_qam && cap->mcs_nss.g_band_256_qam
		&& WMODE_CAP(wdev->PhyMode, WMODE_GN))
		ra_ie.cap0 |= RALINK_256QAM_CAP;

	MakeOutgoingFrame(frame_buffer,
					  &ra_ie_len, (ra_ie.ie_hdr.len + 2), &ra_ie,
					  END_OF_ARGS);
	/* hex_dump ("build vendor_ie: Ralink_OUI", frame_buffer, (ra_ie.ie_hdr.len + 2)); */
	vendor_ie_len = ra_ie_len;
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {


		NdisZeroMemory(&mtk_ie, sizeof(struct _mediatek_ie));
		NdisZeroMemory(&mtk_vht_ie, sizeof(struct _mediatek_vht_ie));
		mtk_vht_ie_len = sizeof(mtk_vht_cap) + sizeof(mtk_vht_op) + sizeof(mtk_vht_txpwr_env);
		mtk_ie.ie_hdr.eid = IE_VENDOR_SPECIFIC;
		mtk_ie.ie_hdr.len = (0x7 + mtk_vht_ie_len);
		mtk_ie.oui[0] = 0x00;
		mtk_ie.oui[1] = 0x0C;
		mtk_ie.oui[2] = 0xE7;
		/* MTK VHT CAP IE */
		memcpy(&mtk_vht_ie.vht_cap, (VHT_CAP *)mtk_vht_cap, sizeof(VHT_CAP));
		/* MTK VHT OP IE */
		memcpy(&mtk_vht_ie.vht_op, (VHT_OP *)mtk_vht_op, sizeof(VHT_OP));
		/* MTK VHT TX PWR ENV IE */
		memcpy(&mtk_vht_ie.vht_txpwr_env, (VHT_TX_PWR_ENV *)mtk_vht_txpwr_env,
			   sizeof(VHT_TX_PWR_ENV));

		if (pAd->CommonCfg.g_band_256_qam && cap->mcs_nss.g_band_256_qam
			&& WMODE_CAP(wdev->PhyMode, WMODE_GN))
			mtk_ie.cap0 |= MEDIATEK_256QAM_CAP;

#ifdef MWDS
        if(wdev->bSupportMWDS) {
            mtk_ie.cap0 |= MEDIATEK_MWDS_CAP;
        }
#endif /* MWDS */
		MakeOutgoingFrame((frame_buffer + vendor_ie_len),
						  &mtk_ie_len, sizeof(struct _mediatek_ie), &mtk_ie,
						  END_OF_ARGS);
		vendor_ie_len += mtk_ie_len;
		MakeOutgoingFrame((frame_buffer + vendor_ie_len),
						  &mtk_vht_ie_len,
						  (sizeof(mtk_vht_cap) + sizeof(mtk_vht_op) + sizeof(mtk_vht_txpwr_env)),
						  &mtk_vht_ie,
						  END_OF_ARGS);
		/* hex_dump ("build vendor_ie: MediaTek_OUI", */
		/* (frame_buffer+vendor_ie_len-mtk_ie_len), (mtk_ie.ie_hdr.len + 2)); */
		vendor_ie_len += mtk_vht_ie_len;

#ifdef WH_EVENT_NOTIFIER
        if(wdev->custom_vie.ie_hdr.len > 0)
        {
            ULONG custom_vie_len;
            ULONG total_custom_vie_len = sizeof(struct Custom_IE_Header) + wdev->custom_vie.ie_hdr.len;
            MakeOutgoingFrame((frame_buffer + vendor_ie_len),
                        &custom_vie_len, total_custom_vie_len, (UCHAR*)&wdev->custom_vie, END_OF_ARGS);
            vendor_ie_len += custom_vie_len;
        }
#endif /* WH_EVENT_NOTIFIER */
	}

#endif /* MT_MAC */
#ifdef VHT_TXBF_2G_EPIGRAM_IE
	NdisZeroMemory(&brcm_txbf2g_ie, sizeof(struct _broadcom_txbf2g_ie));

	brcm_txbf2g_ie.ie_hdr.eid = IE_VENDOR_SPECIFIC;
	brcm_txbf2g_ie.ie_hdr.len = 0x1A;
	brcm_txbf2g_ie.oui[0] = 0x00;
	brcm_txbf2g_ie.oui[1] = 0x90;
	brcm_txbf2g_ie.oui[2] = 0x4C;
	memcpy(&brcm_txbf2g_ie.fixed_pattern,
		(UCHAR *)brcm_txbf2g_fp, sizeof(brcm_txbf2g_fp));

	MakeOutgoingFrame((frame_buffer + vendor_ie_len),
					  &brcm_txbf2g_ie_len,
					  (brcm_txbf2g_ie.ie_hdr.len + 2),
					  &brcm_txbf2g_ie, END_OF_ARGS);
	vendor_ie_len += brcm_txbf2g_ie_len;
#endif /* VHT_TXBF_2G_EPIGRAM_IE */

	get_vie_ctrl(wdev, &vie_ctrl, GET_VIE_FRM_BITMAP(vie_frm_type));
	if (vie_ctrl == NULL)
		goto end;

	vie = vie_ctrl->vie_in_frm;
	for (vie = vie_ctrl->vie_in_frm; (vie != NULL); vie = vie->next_vie) {
		app_vie_len = 0;
		NdisZeroMemory(&vendor_ie, sizeof(struct _generic_vie_t));
		vendor_ie.ie_hdr.eid = IE_VENDOR_SPECIFIC;
		vendor_ie.ie_hdr.len = vie->vie_length;
		NdisCopyMemory(vendor_ie.vie_ctnt, vie->ie_ctnt, vie->vie_length);

		MakeOutgoingFrame((frame_buffer + vendor_ie_len),
				   &app_vie_len,
				   (vendor_ie.ie_hdr.len + 2),
				   &vendor_ie, END_OF_ARGS);

		vendor_ie_len += app_vie_len;
	}

end:
	return vendor_ie_len;
}

#ifdef CONFIG_MAP_SUPPORT
void map_parse_vendor_ie(struct _RTMP_ADAPTER *pAd, struct _vendor_ie_cap *vendor_ie, PEID_STRUCT info_elem)
{
	short *rate;
	char *ptr = &info_elem->Octet[7];

	vendor_ie->map_info.type = *ptr;
	ptr++;
	vendor_ie->map_info.subtype = *ptr;
	ptr++;
	vendor_ie->map_info.root_distance = *ptr;
	ptr++;
	vendor_ie->map_info.connectivity_to_controller = *ptr;
	ptr++;
	rate = (short *)ptr;
	vendor_ie->map_info.uplink_rate = *rate;
	ptr += 2;
	NdisCopyMemory(vendor_ie->map_info.uplink_bssid, ptr, MAC_ADDR_LEN);
	ptr += MAC_ADDR_LEN;
	NdisCopyMemory(vendor_ie->map_info.bssid_5g, ptr, MAC_ADDR_LEN);
	ptr += MAC_ADDR_LEN;
	NdisCopyMemory(vendor_ie->map_info.bssid_2g, ptr, MAC_ADDR_LEN);
	ptr += MAC_ADDR_LEN;
}
#endif
VOID check_vendor_ie(struct _RTMP_ADAPTER *pAd,
		     UCHAR *ie_buffer,
		     struct _vendor_ie_cap *vendor_ie)
{
	PEID_STRUCT info_elem = (PEID_STRUCT)ie_buffer;
	UCHAR ralink_oui[] = {0x00, 0x0c, 0x43};
	UCHAR mediatek_oui[] = {0x00, 0x0c, 0xe7};
	UCHAR broadcom_oui[][3] = {{0x00, 0x90, 0x4c}, {0x00, 0x10, 0x18} };
	/* UCHAR broadcom_fixed_pattern[] = {0x04, 0x08}; */
#ifdef DPP_SUPPORT
	UCHAR dpp_oui[] = {0x50, 0x6f, 0x9a, 0x1e};
#endif

	if (NdisEqualMemory(info_elem->Octet, ralink_oui, 3)
		&& (info_elem->Len == 7)) {
		vendor_ie->ra_cap = (ULONG)info_elem->Octet[3];
		vendor_ie->is_rlt = TRUE;
		vendor_ie->ldpc = TRUE;
		vendor_ie->sgi = TRUE;
		/* hex_dump ("recv. vendor_ie: Ralink_OUI", (UCHAR *)info_elem, (info_elem->Len + 2)); */
	} else if (NdisEqualMemory(info_elem->Octet, mediatek_oui, 3) &&
		   (info_elem->Len >= 7)) {
		vendor_ie->mtk_cap = (ULONG)info_elem->Octet[3];
		vendor_ie->is_mtk = TRUE;
#define MAP_TURNKEY_IE(B1)            ((B1)&0x01)
#ifdef CONFIG_MAP_SUPPORT
		if (MAP_TURNKEY_IE(info_elem->Octet[4])) {
			vendor_ie->map_vendor_ie_found = TRUE;
			map_parse_vendor_ie(pAd, vendor_ie, info_elem);
		}
#endif /* CONFIG_MAP_SUPPORT */

		if (info_elem->Len > 7) {
			/* have MTK VHT IEs */
			vendor_ie->ldpc = TRUE;
			vendor_ie->sgi = TRUE;
#ifdef MWDS
			if (!MAP_TURNKEY_IE(info_elem->Octet[4])) {
				/* We can't be covered by easy setup customized mtk ie. */
				vendor_ie->mtk_cap_found = TRUE;
				if (MWDS_SUPPORT(vendor_ie->mtk_cap))
					vendor_ie->support_mwds = TRUE;
				else
					vendor_ie->support_mwds = FALSE;
			}
#endif /* MWDS */
		} else {
			vendor_ie->ldpc = FALSE;
			vendor_ie->sgi = FALSE;
		}
		/* hex_dump ("recv. vendor_ie: MediaTek_OUI", (UCHAR *)info_elem, (info_elem->Len + 2)); */
	} else if (NdisEqualMemory(info_elem->Octet, &broadcom_oui[0][0], 3)
			   /* && NdisEqualMemory(info_elem->Octet+3, broadcom_fixed_pattern, 2)) */
			) {
		vendor_ie->brcm_cap = BROADCOM_256QAM_CAP;
		vendor_ie->ldpc = TRUE;
		vendor_ie->sgi = TRUE;
		/* hex_dump ("recv. vendor_ie: Broadcom_OUI", (UCHAR *)info_elem, (info_elem->Len + 2)); */
	} else if (NdisEqualMemory(info_elem->Octet, &broadcom_oui[1][0], 3))
		vendor_ie->is_brcm_etxbf_2G = TRUE;
#ifdef WH_EVENT_NOTIFIER
	else if (pAd->ApCfg.EventNotifyCfg.CustomOUILen &&
		(info_elem->Len >= pAd->ApCfg.EventNotifyCfg.CustomOUILen) &&
		NdisEqualMemory(info_elem->Octet,
				pAd->ApCfg.EventNotifyCfg.CustomOUI,
				pAd->ApCfg.EventNotifyCfg.CustomOUILen)) {
		vendor_ie->custom_ie_len = info_elem->Len;
		NdisMoveMemory(vendor_ie->custom_ie, info_elem->Octet, info_elem->Len);
	}
#endif /* WH_EVENT_NOTIFIER */
#ifdef DPP_R2_SUPPORT
	else if (NdisEqualMemory(info_elem->Octet, dpp_oui, 4)) {
		vendor_ie->cce_vendor_ie_found = TRUE;
		vendor_ie->cce_info.value = info_elem->Octet[3];/*0x1E*/
	}
#endif
	else {
		/* printk ("Other Vendor IE: OUI: %02x %02x %02x\n", */
		/* info_elem->Octet[0], info_elem->Octet[1], info_elem->Octet[2]); */
		/* hex_dump ("recv. vendor_ie: xxx_OUI", (UCHAR *)info_elem, (info_elem->Len + 2)); */
	}
}

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
VOID customer_check_vendor_ie(struct _RTMP_ADAPTER *pAd,
				    UCHAR *ie_buffer,
				    struct customer_vendor_ie *vendor_ie,
				    BCN_IE_LIST *ie_list)
{
	PEID_STRUCT info_elem = (PEID_STRUCT)ie_buffer;
	ULONG vendor_ie_len = 0;
	CHAR *vendor_ie_temp;
	int ret;

	if (NdisEqualMemory(info_elem->Octet, WPA_OUI, 4) ||
	    NdisEqualMemory(info_elem->Octet, WME_INFO_ELEM, 6) ||
	    NdisEqualMemory(info_elem->Octet, WME_PARM_ELEM, 6) ||
	    NdisEqualMemory(info_elem->Octet, WPS_OUI, 4)) {
		return;
	}

	vendor_ie_len = info_elem->Len + 2 + vendor_ie->length;
	ret = os_alloc_mem(pAd, (UCHAR **)&vendor_ie_temp, vendor_ie_len);

	if (ret == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
			"\n\n\nALLOC Memory fail\n\n\n");
		return;
	}

	if (vendor_ie->pointer != NULL && vendor_ie->length != 0) {
		os_move_mem(vendor_ie_temp, vendor_ie->pointer,
			    vendor_ie->length);
		os_free_mem(vendor_ie->pointer);
		os_move_mem(vendor_ie_temp + vendor_ie->length, info_elem,
			    info_elem->Len + 2);
		vendor_ie->pointer = vendor_ie_temp;
		vendor_ie->length = vendor_ie_len;
	} else {
		os_move_mem(vendor_ie_temp, info_elem, info_elem->Len + 2);
		vendor_ie->pointer = vendor_ie_temp;
		vendor_ie->length = vendor_ie_len;
	}
}
#endif /* CUSTOMER_VENDOR_IE_SUPPORT */
