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
	sae.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/

#ifndef	__SAE_H__
#define	__SAE_H__

#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
INT show_sae_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID sae_cfg_init(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG * pSaeCfg);

VOID sae_cfg_deinit(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG * pSaeCfg);

UCHAR sae_get_rejected_group(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	IN UINT32 *reject_group);


SAE_INSTANCE *search_sae_instance(
	IN SAE_CFG * pSaeCfg,
	IN UCHAR * own_mac,
	IN UCHAR * peer_mac);

SAE_INSTANCE *create_sae_instance(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG * pSaeCfg,
	IN UCHAR * own_mac,
	IN UCHAR * peer_mac,
	IN UCHAR * bssid,
	IN UCHAR *psk,
	IN struct pwd_id_list *pwd_id_list_head,
	IN UCHAR is_pwd_id_only);


VOID delete_sae_instance(
	IN SAE_INSTANCE *pSaeIns);

UCHAR set_sae_instance_removable(
	IN SAE_CFG * pSaeCfg,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac);

VOID sae_ins_init(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG * pSaeCfg,
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR * own_mac,
	IN UCHAR * peer_mac,
	IN UCHAR * bssid,
	IN UCHAR *psk,
	IN struct pwd_id_list *pwd_id_list_head,
	IN UCHAR is_pwd_id_only);

/* partial */
VOID sae_clear_data(
	IN SAE_INSTANCE *pSaeIns);

UCHAR sae_using_anti_clogging(
	IN SAE_CFG * pSaeCfg);


VOID sae_set_retransmit_timer(
	IN SAE_INSTANCE *pSaeIns);


VOID sae_clear_retransmit_timer(
	IN SAE_INSTANCE *pSaeIns);


DECLARE_TIMER_FUNCTION(sae_auth_retransmit);


VOID sae_auth_retransmit(
	IN VOID *SystemSpecific1,
	IN VOID *FunctionContext,
	IN VOID *SystemSpecific2,
	IN VOID *SystemSpecific3);


UCHAR sae_auth_init(
	IN RTMP_ADAPTER *pAd,
	IN SAE_CFG *pSaeCfg,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	IN UCHAR *bssid,
	IN UCHAR *psk,
	IN struct sae_pt *pt_list,
	IN struct sae_pk_cfg *sae_pk,
	IN INT32 group,
	IN UCHAR sae_conn_type);


UCHAR sae_handle_auth(
	IN RTMP_ADAPTER *pAd,
	IN SAE_CFG * pSaeCfg,
	IN VOID *msg,
	IN UINT32 msg_len,
	IN UCHAR *psk,
	IN struct sae_pt *pt_list,
	IN struct sae_pk_cfg *sae_pk,
	IN struct sae_capability *sae_cap,
	IN struct pwd_id_list *pwd_id_list_head,
	IN USHORT auth_seq,
	IN USHORT auth_status,
	OUT UCHAR **pmk,
	OUT UCHAR *sae_conn_type);


USHORT sae_sm_step(
	IN RTMP_ADAPTER * pAd,
	IN SAE_INSTANCE *pSaeIns,
	IN USHORT auth_seq);


UCHAR sae_check_big_sync(
	IN SAE_INSTANCE *pSaeIns);

UCHAR sae_get_pmk_cache(
	IN SAE_CFG * pSaeCfg,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	OUT UCHAR *pmkid,
	OUT UCHAR *pmk);


VOID sae_parse_commit_token_req(
	IN SAE_INSTANCE * pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end,
	IN UCHAR **token,
	IN UINT32 * token_len);

UCHAR sae_build_token_req(
	IN RTMP_ADAPTER * pAd,
	IN SAE_INSTANCE * pSaeIns,
	OUT UCHAR *token_req,
	OUT UINT32 * token_req_len);

UCHAR sae_check_token(
	IN SAE_INSTANCE * pSaeIns,
	IN UCHAR *peer_token,
	IN UINT32 peer_token_len);

USHORT sae_parse_commit(
	IN SAE_CFG * pSaeCfg,
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *msg,
	IN UINT32 msg_len,
	IN UCHAR **token,
	IN UINT32 * token_len,
	IN UCHAR is_token_req);


VOID sae_parse_commit_token(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end,
	IN UCHAR **token,
	IN UINT32 * token_len);


USHORT sae_parse_commit_scalar(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end);


USHORT sae_parse_commit_element(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end);

UCHAR is_sae_pwd_id_element(
	IN UCHAR *pos,
	IN UCHAR *end,
	OUT UINT32 * len);

UCHAR is_sae_rejected_group_element(
	IN UCHAR *pos,
	IN UCHAR *end,
	OUT UINT32 *len);

USHORT sae_parse_password_identifier(
	IN SAE_INSTANCE * pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end,
	IN UCHAR is_pwd_id_only);

USHORT sae_parse_rejected_groups(
	IN SAE_INSTANCE * pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end);

USHORT sae_parse_token_container(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *pos,
	IN UCHAR *end,
	IN UCHAR **token,
	IN UINT32 *token_len);

USHORT sae_check_rejected_group(
	IN SAE_INSTANCE * pSaeIns);

USHORT sae_prepare_commit(
	IN SAE_INSTANCE *pSaeIns);


USHORT sae_derive_commit(
	IN SAE_INSTANCE *pSaeIns);


USHORT sae_process_commit(
	IN SAE_INSTANCE *pSaeIns);


UCHAR sae_derive_key(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *k);


VOID sae_send_auth(
	IN RTMP_ADAPTER * pAd,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	IN UCHAR *bssid,
	IN USHORT alg,
	IN USHORT seq,
	IN USHORT status_code,
	IN UCHAR *buf,
	IN UINT32 buf_len);


UCHAR sae_send_auth_commit(
	IN RTMP_ADAPTER * pAd,
	IN SAE_INSTANCE *pSaeIns);


UCHAR sae_send_auth_confirm(
	IN RTMP_ADAPTER * pAd,
	IN SAE_INSTANCE *pSaeIns);


USHORT sae_parse_confirm(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *msg,
	IN UINT32 msg_len);


USHORT sae_check_confirm(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *peer_confirm);

USHORT sae_pk_check_pub_key(
	IN SAE_INSTANCE *pSaeIns);

USHORT sae_pk_check_signature(
	IN SAE_INSTANCE *pSaeIns);

USHORT sae_pk_parse_element(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *pos,
	IN UCHAR *end);

UINT32 sae_pk_build_fils_public_key(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *buf);

UINT32 sae_pk_build_fils_key_confirmation(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *buf);

UINT32 sae_pk_build_sae_pk_element(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *buf);

SAE_BN *sae_gen_rand(
	IN SAE_INSTANCE *pSaeIns);

INT sae_set_k_iteration(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg);

INT sae_set_anti_clogging_th(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg);

INT sae_set_fixed_group_id(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg);

INT sae_set_debug_level(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg);

INT sae_set_commit_msg(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg);

INT sae_pk_set_pri_key_overwrite(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg);

INT sae_pk_set_test_ctrl(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg);

USHORT sae_group_allowed(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *allowed_groups,
	IN INT32 group);

VOID *sae_search_pt_by_group(
	IN struct sae_pt *pt_list,
	IN USHORT group,
	IN struct pwd_id_list *pwd_id_list);

VOID sae_derive_pt(
	struct wifi_dev *wdev,
	IN SAE_CFG * pSaeCfg,
	IN UCHAR *psk,
	IN CHAR * ssid,
	IN UCHAR ssid_len,
	IN struct pwd_id_list *pwd_id_list_head,
	OUT struct sae_pt **pt_list);

VOID sae_pt_list_deinit(
	IN struct wifi_dev *wdev,
	INOUT struct sae_pt **pt_list);

UCHAR sae_pk_init(
	IN struct _RTMP_ADAPTER *ad,
	IN struct sae_pk_cfg *sae_pk,
	IN CHAR * ssid,
	IN UCHAR ssid_len,
	IN UCHAR role,
	OUT UCHAR *psk);

UCHAR sae_pk_init_for_supplicant(
	IN struct sae_pk_cfg *sae_pk,
	IN UCHAR *psk);

UCHAR sae_pk_pem_decode(
	IN struct sae_pk_cfg *sae_pk,
	IN CHAR * str_in,
	OUT UCHAR *hex_out,
	OUT UINT32 *hex_out_len);

VOID sae_pk_deinit(
	IN struct sae_pk_cfg *sae_pk);

VOID sae_pk_deri_finger_print(
	IN struct sae_pk_cfg *sae_pk,
	IN CHAR * ssid,
	IN UCHAR ssid_len,
	IN UCHAR need_search,
	OUT UCHAR *finger_print);


static inline UCHAR is_sae_group_ecc(
	IN INT32 group)
{
	switch (group) {
	case 19:
	case 20:
	case 21:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
		return TRUE;

	default:
		return FALSE;
	}
}


static inline UCHAR is_sae_group_ffc(
	IN INT32 group)
{
	switch (group) {
	case 1:
	case 2:
	case 5:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 22:
	case 23:
	case 24:
		return TRUE;

	default:
		return FALSE;
	}
}



/*
 =====================================
	group related
 =====================================
*/
UCHAR sae_pk_init_ecc(
	IN struct _RTMP_ADAPTER *ad,
	IN struct sae_pk_cfg *sae_pk,
	INOUT UCHAR *psk);

VOID sae_pk_deinit_ecc(
	IN struct sae_pk_cfg *sae_pk);

VOID sae_group_init_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN INT32 group);

VOID sae_group_init_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN INT32 group);


VOID sae_group_deinit_ecc(
	IN SAE_INSTANCE *pSaeIns);


VOID sae_group_deinit_ffc(
	IN SAE_INSTANCE *pSaeIns);


VOID sae_cn_confirm_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR is_send, /* otherwise, is verfication */
	OUT UCHAR *confirm);


VOID sae_cn_confirm_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR is_send, /* otherwise, is verfication */
	OUT UCHAR *confirm);


VOID sae_cn_confirm_cmm(
	IN SAE_INSTANCE *pSaeIns,
	IN SAE_BN *scalar1,
	IN SAE_BN *scalar2,
	IN UCHAR *element_bin1,
	IN UCHAR *element_bin2,
	IN UINT32 element_len,
	IN USHORT send_confirm,
	OUT UCHAR *confirm);


USHORT sae_parse_commit_element_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end);


USHORT sae_parse_commit_element_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end);


UCHAR sae_derive_commit_element_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN SAE_BN *mask);


UCHAR sae_derive_commit_element_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN SAE_BN *mask);

USHORT sae_derive_pwe_ecc(
	IN SAE_INSTANCE *pSaeIns);

USHORT sae_derive_pwe_pt_ecc(
	IN SAE_INSTANCE *pSaeIns);

USHORT sae_derive_pwe_ffc(
	IN SAE_INSTANCE *pSaeIns);



UCHAR sae_derive_k_ecc(
	IN SAE_INSTANCE *pSaeIns,
	OUT UCHAR *k);


UCHAR sae_derive_k_ffc(
	IN SAE_INSTANCE *pSaeIns,
	OUT UCHAR *k);

USHORT sae_reflection_check_ecc(
	IN SAE_INSTANCE * pSaeIns);

USHORT sae_reflection_check_ffc(
	IN SAE_INSTANCE * pSaeIns);

#endif /* DOT11_SAE_SUPPORT */
#endif /* __SAE_H__ */

