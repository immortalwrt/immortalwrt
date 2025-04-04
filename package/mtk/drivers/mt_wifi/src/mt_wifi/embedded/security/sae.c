#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
#include "rt_config.h"
#include "security/ecc.h"

BUILD_TIMER_FUNCTION(sae_auth_retransmit);

const SAE_GROUP_OP ecc_group_op = {
	.sae_group_init = sae_group_init_ecc,
	.sae_group_deinit = sae_group_deinit_ecc,
	.sae_cn_confirm = sae_cn_confirm_ecc,
	.sae_parse_commit_element = sae_parse_commit_element_ecc,
	.sae_derive_commit_element = sae_derive_commit_element_ecc,
	.sae_derive_pwe = sae_derive_pwe_ecc,
	.sae_derive_pwe_pt  = sae_derive_pwe_pt_ecc,
	.sae_derive_k = sae_derive_k_ecc,
	.sae_reflection_check = sae_reflection_check_ecc,
};

const SAE_GROUP_OP ffc_group_op = {
	.sae_group_init = sae_group_init_ffc,
	.sae_group_deinit = sae_group_deinit_ffc,
	.sae_cn_confirm = sae_cn_confirm_ffc,
	.sae_parse_commit_element = sae_parse_commit_element_ffc,
	.sae_derive_commit_element = sae_derive_commit_element_ffc,
	.sae_derive_pwe = sae_derive_pwe_ffc,
	.sae_derive_k = sae_derive_k_ffc,
	.sae_reflection_check = sae_reflection_check_ffc,
};

static DH_GROUP_INFO dh_groups[] = {
	DH_GROUP(15, 1),
};



static DH_GROUP_INFO_BI dh_groups_bi[] = {
	DH_GROUP_BI(15, 1),
};

#ifdef BI_POOL_DBG
UINT32 sae_expected_cnt[20]; /* 0~12  is used */
#endif

int SAE_DEBUG_LEVEL = DBG_LVL_DEBUG;
int SAE_DEBUG_LEVEL2 = DBG_LVL_INFO;
int SAE_PK_DEBUG_LEVEL = DBG_LVL_INFO;
int SAE_COST_TIME_DBG_LVL = DBG_LVL_DEBUG;

UCHAR sae_support_group_list[] = {19, 20, 21};

UCHAR fixed_group_id = 19;

/********************************/
/* sae debug/set parameter/query api*/
/********************************/
INT show_sae_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 i;
	SAE_CFG *pSaeCfg = &pAd->SaeCfg;
	SAE_INSTANCE *pSaeIns = NULL;
	UINT32 input = 0;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pSaeCfg->pAd);

	if (arg != NULL)
		input = os_str_toul(arg, 0, 10);

	MTWF_PRINT("k iter varieble: %d, anti clogging th: %d, wpa3_test_ctrl: %d/%d\n",
							pSaeCfg->k_iteration_var, pSaeCfg->sae_anti_clogging_threshold, wpa3_test_ctrl, wpa3_test_ctrl2);
	MTWF_PRINT("total ins: %d\n", pSaeCfg->total_ins);

	for (i = 0; i < wtbl_max_num; i++) {
		if (pSaeCfg->sae_ins[i].valid == FALSE && input != 1)
			continue;

		pSaeIns = &pSaeCfg->sae_ins[i];

		MTWF_PRINT("idx:%d, v/r:%d/%d, OM="MACSTR", PM="MACSTR"\n",
			 i, pSaeIns->valid, pSaeIns->removable,
			 MAC2STR(pSaeIns->own_mac), MAC2STR(pSaeIns->peer_mac));

		MTWF_PRINT("\tstate:%d, group:%d, sync:%d, sc:0x%x, last_peer_sc:0x%x, same_mac_ins=%d, timer_state=%d\n",
			 pSaeIns->state, pSaeIns->group, pSaeIns->sync,
			 pSaeIns->send_confirm, pSaeIns->last_peer_sc,
			 (pSaeIns->same_mac_ins != NULL),
			 pSaeIns->sae_retry_timer.State);

		MTWF_PRINT("\tconnect_type=%d, is_pwd_id_only = %d, removable = %d, pwd_id = %s\n",
			 pSaeIns->connect_type, pSaeIns->is_pwd_id_only, pSaeIns->removable, pSaeIns->pwd_id_ptr->pwd_id);

		if (pSaeIns->valid && pSaeIns->pParentSaeCfg == NULL)
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				"\t[error]pSaeIns->pParentSaeCfg is NULL\n");
		if (pSaeIns->valid && pSaeIns->psk == NULL)
			MTWF_DBG(pAd,DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				"\t[error]pSaeIns->psk is NULL\n");
	}

	return TRUE;
}

INT sae_set_k_iteration(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg)
{
	if (arg == NULL)
		return FALSE;
	ad->SaeCfg.k_iteration_var = os_str_tol(arg, 0, 10);

	return TRUE;
}

INT sae_set_anti_clogging_th(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg)
{
	if (arg == NULL)
		return FALSE;

	ad->SaeCfg.sae_anti_clogging_threshold = os_str_tol(arg, 0, 10);

	return TRUE;
}

INT sae_set_fixed_group_id(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg)
{
	if (arg == NULL)
		return FALSE;

	fixed_group_id = os_str_tol(arg, 0, 10);

	return TRUE;
}

INT sae_set_debug_level(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg)
{
	UINT32 dbg_lvl = 0;
	UINT32 dbg_num = 0;
	RTMP_STRING *str;

	if (arg == NULL || strlen(arg) == 0)
		return FALSE;

	str = strsep(&arg, ":");

	if (arg == NULL)
		return FALSE;


	dbg_lvl = os_str_tol(arg, 0, 10);
	dbg_num = os_str_tol(str, 0, 10);

	if (dbg_num == 0)
		SAE_DEBUG_LEVEL = dbg_lvl;
	else if (dbg_num == 1)
		SAE_DEBUG_LEVEL2 = dbg_lvl;
	else if (dbg_num == 2)
		SAE_PK_DEBUG_LEVEL = dbg_lvl;

	MTWF_PRINT("%s(): dbg_num(%d) : dbg_lvl(%d)\n", __func__, dbg_num, dbg_lvl);

	return TRUE;
}


UCHAR commit_msg[300];
UINT32 commit_msg_len;

INT sae_set_commit_msg(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg)
{
	if (arg == NULL)
		return FALSE;

	commit_msg_len = sizeof(commit_msg);

	str_to_bin(arg, commit_msg, &commit_msg_len);

	hex_dump_with_lvl("commit_msg", commit_msg, commit_msg_len, 1);

	return TRUE;
}

UCHAR private_key_override[32];
UINT32 private_key_len;


INT sae_pk_set_pri_key_overwrite(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg)
{
	if (arg == NULL)
		return FALSE;

	private_key_len = sizeof(private_key_override);

	str_to_bin(arg, private_key_override, &private_key_len);

	hex_dump_with_lvl("private_key_override", private_key_override, private_key_len, 1);

	return TRUE;
}

INT sae_pk_set_test_ctrl(
	IN struct _RTMP_ADAPTER *ad,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) ad->OS_Cookie;
	struct _SECURITY_CONFIG *sec_cfg = pObj->pSecConfig;

	if (arg == NULL)
		return FALSE;

	sec_cfg->sae_pk.sae_pk_test_ctrl = os_str_tol(arg, 0, 16);

	return TRUE;
}

/* if own_mac & peer_mac is null, search all instance */
UCHAR sae_get_rejected_group(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	IN UINT32 *reject_group)
{
	SAE_CFG *pSaeCfg = &pAd->SaeCfg;
	SAE_INSTANCE *pSaeIns = search_sae_instance(pSaeCfg, own_mac, peer_mac);
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pSaeCfg->pAd);
	UINT32 i;
	UINT32 ins_cnt = 0;

	if (own_mac && peer_mac && pSaeIns == NULL) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 " search fail\n");
		return FALSE;
	}

	if (!own_mac || !peer_mac) {
		*reject_group = 0;
		NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);
		for (i = 0; i < wtbl_max_num; i++) {
			if (pSaeCfg->sae_ins[i].valid == FALSE)
				continue;

			if (pSaeCfg->sae_ins[i].rejected_group) {
				*reject_group = pSaeCfg->sae_ins[i].rejected_group;
				break;
			}

			ins_cnt++;

			if (ins_cnt == pSaeCfg->total_ins)
				break;
		}
		NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
	} else if (pSaeIns)
		*reject_group = pSaeIns->rejected_group;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
				 "reject_group = %x\n", *reject_group);

	return TRUE;
}
/* sae debug/set parameter/query api end*/

static VOID hkdf_extract(
	IN  const UINT8 key[],
	IN  UINT key_len,
	IN  const UINT8 msg[],
	IN  UINT msg_len,
	OUT UINT8 mac[],
	IN  UINT hash_len)
{
	if (hash_len == SHA256_DIGEST_SIZE)
		RT_HMAC_SHA256(key, key_len, msg, msg_len, mac, hash_len);
	else if (hash_len == SHA384_DIGEST_SIZE)
		RT_HMAC_SHA384(key, key_len, msg, msg_len, mac, hash_len);
	else if (hash_len == SHA512_DIGEST_SIZE)
		RT_HMAC_SHA512(key, key_len, msg, msg_len, mac, hash_len);
}


static VOID hkdf_expand(
	IN UCHAR *hash,
	IN INT hash_len,
	IN UCHAR *info,
	IN INT info_len,
	OUT UCHAR *output,
	INT output_Len)
{
	if (hash_len == SHA256_DIGEST_SIZE)
		HKDF_expand_sha256(hash, hash_len, info, info_len, output, output_Len);
	else if (hash_len == SHA384_DIGEST_SIZE)
		HKDF_expand_sha384(hash, hash_len, info, info_len, output, output_Len);
	else if (hash_len == SHA512_DIGEST_SIZE)
		HKDF_expand_sha512(hash, hash_len, info, info_len, output, output_Len);
}

static VOID sae_kdf_hash(
	IN PUINT8 hash,
	IN INT hash_len,
	IN PUINT8 label,
	IN INT label_len,
	IN PUINT8 data,
	IN INT data_len,
	OUT PUINT8 output,
	IN USHORT len)
{
	if (hash_len == SHA256_DIGEST_SIZE)
		KDF_256(hash, hash_len, label, label_len, data, data_len, output, len);
	else if (hash_len == SHA384_DIGEST_SIZE)
		KDF_384(hash, hash_len, label, label_len, data, data_len, output, len);
	else if (hash_len == SHA512_DIGEST_SIZE)
		KDF_512(hash, hash_len, label, label_len, data, data_len, output, len);
}


VOID sae_cfg_init(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG * pSaeCfg)
{
	UINT32 i = 0;

#ifdef BI_POOL
	big_integer_pool_init();
#endif
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);
	/* 12.4.8.5.1 When the parent SAE process starts up, Open is set to zero (0) */
	pSaeCfg->pAd = pAd;
	pSaeCfg->open = 0;
	pSaeCfg->dot11RSNASAERetransPeriod = 2;
	pSaeCfg->total_ins = 0;
	pSaeCfg->sae_anti_clogging_threshold = 8;
	pSaeCfg->k_iteration_var = 40;
	pSaeCfg->last_token_key_time = 0;
	NdisZeroMemory(&pSaeCfg->token_key, SAE_TOKEN_KEY_LEN);

	NdisZeroMemory(&pSaeCfg->support_group, MAX_SIZE_OF_ALLOWED_GROUP);

	for (i = 0; i < sizeof(sae_support_group_list)/sizeof(UCHAR); i++)
		pSaeCfg->support_group[i] = sae_support_group_list[i];

	wpa3_test_ctrl = 0;
	commit_msg_len = 0;
}

VOID sae_cfg_deinit(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG * pSaeCfg)
{
	UINT32 i;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pSaeCfg->pAd);

#ifdef BI_POOL
	big_integer_pool_deinit();
#endif
	for (i = 0; i < wtbl_max_num; i++)
		if (pSaeCfg->sae_ins[i].valid == FALSE)
			continue;
		else
			delete_sae_instance(&pSaeCfg->sae_ins[i]);
}

/*******************************/
/* sae insntance operation related api*/
/*******************************/
static UCHAR delete_all_removable_sae_instance(
	IN SAE_CFG * pSaeCfg)
{
	UINT32 i;
	SAE_INSTANCE *pSaeIns = NULL;
	UINT32 del_ins_cnt = 0;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pSaeCfg->pAd);

	for (i = 0; i < wtbl_max_num; i++) {
		pSaeIns = &pSaeCfg->sae_ins[i];

		if (pSaeCfg->sae_ins[i].valid == FALSE)
			continue;

		if (pSaeIns->same_mac_ins) {
			if (pSaeIns->same_mac_ins->valid == TRUE) {
				delete_sae_instance(pSaeIns->same_mac_ins);
				del_ins_cnt++;
			}
			pSaeIns->same_mac_ins = NULL;
		}

		if (pSaeIns->removable == TRUE) {
			delete_sae_instance(pSaeIns);
			del_ins_cnt++;
		}
	}

	if (del_ins_cnt != 0)
		return TRUE;
	else
		return FALSE;
}

SAE_INSTANCE *search_sae_instance(
	IN SAE_CFG * pSaeCfg,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac)
{
	UINT32 i;
	SAE_INSTANCE *pSaeIns = NULL;
	UINT32 ins_cnt = 0;
	UINT16 wtbl_max_num = 0;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	if (!pSaeCfg || !own_mac || !peer_mac) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 "search fail with null input\n");
		return NULL;
	}
	wtbl_max_num = WTBL_MAX_NUM(pSaeCfg->pAd);
	NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

	for (i = 0; i < wtbl_max_num; i++) {
		if (pSaeCfg->sae_ins[i].valid == FALSE)
			continue;

		if (RTMPEqualMemory(pSaeCfg->sae_ins[i].own_mac, own_mac, MAC_ADDR_LEN) /* ellis */
			&& RTMPEqualMemory(pSaeCfg->sae_ins[i].peer_mac, peer_mac, MAC_ADDR_LEN)) {
			pSaeIns = &pSaeCfg->sae_ins[i];
			break;
		}

		ins_cnt++;

		if (ins_cnt == pSaeCfg->total_ins)
			break;
	}

	NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);

	if (pSaeIns
		&& pSaeIns->state == SAE_ACCEPTED
		&& pSaeIns->same_mac_ins)
		pSaeIns = pSaeIns->same_mac_ins;

	return pSaeIns;
}


SAE_INSTANCE *create_sae_instance(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG * pSaeCfg,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	IN UCHAR *bssid,
	IN UCHAR *psk,
	IN struct pwd_id_list *pwd_id_list_head,
	IN UCHAR is_pwd_id_only)
{
	UINT32 i;
	SAE_INSTANCE *pSaeIns = NULL;
	UINT16 wtbl_max_num = 0;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			 "==> %s()\n", __func__);

	if (pSaeCfg == NULL || own_mac == NULL ||
	    peer_mac == NULL || bssid == NULL || (!is_pwd_id_only && psk == NULL)) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 ":input should not be null\n");
		return NULL;
	}
	wtbl_max_num = WTBL_MAX_NUM(pSaeCfg->pAd);

	if (is_pwd_id_only &&
		(pwd_id_list_head == NULL || DlListEmpty(&pwd_id_list_head->list))) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 "pwd id is null or empty\n");
		return NULL;
	}

	if ((pSaeCfg->total_ins == wtbl_max_num) &&
		(delete_all_removable_sae_instance(pSaeCfg) == FALSE)) {
		return NULL;
	}

	NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

	for (i = 0; i < wtbl_max_num; i++) {
		if (pSaeCfg->sae_ins[i].valid == FALSE) {
			pSaeIns = &pSaeCfg->sae_ins[i];
			sae_ins_init(pAd, pSaeCfg, pSaeIns,
						 own_mac, peer_mac, bssid, psk, pwd_id_list_head, is_pwd_id_only);
			pSaeIns->valid = TRUE;
			pSaeCfg->total_ins++;
			break;
		}
	}

	NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
	return pSaeIns;
}


VOID delete_sae_instance(
	IN SAE_INSTANCE *pSaeIns)
{
	SAE_CFG *pSaeCfg = NULL;
	BOOLEAN Cancelled;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	if (pSaeIns == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 " pSaeIns is NULL\n");
		return;
	}

	pSaeCfg = pSaeIns->pParentSaeCfg;
	if (pSaeCfg == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 " pSaeCfg is NULL\n");
		return;
	}

	NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);
	RTMPReleaseTimer(&pSaeIns->sae_retry_timer, &Cancelled);
	pSaeIns->valid = FALSE;
	pSaeIns->removable = FALSE;
	if (pSaeIns->group_op)
		pSaeIns->group_op->sae_group_deinit(pSaeIns);

	if (pSaeIns->anti_clogging_token) {
		os_free_mem(pSaeIns->anti_clogging_token);
		pSaeIns->anti_clogging_token = NULL;
	}

	SAE_BN_FREE(&pSaeIns->sae_rand);

	if (pSaeIns->same_mac_ins) {
		pSaeIns->same_mac_ins->same_mac_ins = NULL;
		pSaeIns->same_mac_ins = NULL;
	}
	SAE_BN_FREE(&pSaeIns->own_commit_scalar);
	SAE_BN_FREE(&pSaeIns->peer_commit_scalar);

	if (pSaeIns->key_auth) {
		os_free_mem(pSaeIns->key_auth);
		pSaeIns->key_auth = NULL;
	}
	NdisZeroMemory(pSaeIns, sizeof(SAE_INSTANCE));
	pSaeCfg->total_ins--;
	NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
	return;
}

UCHAR set_sae_instance_removable(
	IN SAE_CFG *pSaeCfg,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac)
{
	SAE_INSTANCE *pSaeIns = search_sae_instance(pSaeCfg, own_mac, peer_mac);

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "%s:OM="MACSTR", PM="MACSTR"\n",
			 __func__, MAC2STR(own_mac), MAC2STR(peer_mac));

	if (pSaeIns == NULL)
		return FALSE;

	pSaeIns->removable = TRUE;
	return TRUE;
}

VOID sae_ins_init(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG *pSaeCfg,
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	IN UCHAR *bssid,
	IN UCHAR *psk,
	IN struct pwd_id_list *pwd_id_list_head,
	IN UCHAR is_pwd_id_only)
{
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);
	NdisZeroMemory(pSaeIns, sizeof(SAE_INSTANCE));
	COPY_MAC_ADDR(pSaeIns->own_mac,  own_mac);
	COPY_MAC_ADDR(pSaeIns->peer_mac,  peer_mac);
	COPY_MAC_ADDR(pSaeIns->bssid,  bssid);
	RTMPInitTimer(pAd, &pSaeIns->sae_retry_timer, GET_TIMER_FUNCTION(sae_auth_retransmit), pSaeIns,  FALSE);
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG, "%s: timer valid = %d\n", __func__, pSaeIns->sae_retry_timer.Valid);
	pSaeIns->pParentSaeCfg = pSaeCfg;
	pSaeIns->psk = psk;
	pSaeIns->pwd_id_list_head = pwd_id_list_head;
	pSaeIns->is_pwd_id_only = is_pwd_id_only;
	SET_NOTHING_STATE(pSaeIns);
	pSaeIns->sync = 0;
	/* 12.4.8.5.2
	  * The number of Confirm messages that have been sent.
	  * This is the send-confirm counter used in the construction of Confirm messages
	  */
	pSaeIns->send_confirm = 0; /* ellis */
	pSaeIns->last_peer_sc = 0;
	pSaeIns->support_group_idx = 0;
	pSaeIns->same_mac_ins = NULL;
	pSaeIns->removable = FALSE;
}

/* partial */
VOID sae_clear_data(
	IN SAE_INSTANCE *pSaeIns)
{
	BOOLEAN Cancelled;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	if (pSaeIns->group_op)
		pSaeIns->group_op->sae_group_deinit(pSaeIns);

	if (pSaeIns->anti_clogging_token) {
		os_free_mem(pSaeIns->anti_clogging_token);
		pSaeIns->anti_clogging_token = NULL;
	}

	if (pSaeIns->key_auth) {
		os_free_mem(pSaeIns->key_auth);
		pSaeIns->key_auth = NULL;
	}

	SAE_BN_FREE(&pSaeIns->sae_rand);
	SAE_BN_FREE(&pSaeIns->peer_commit_scalar);
	SAE_BN_FREE(&pSaeIns->own_commit_scalar);
	SET_NOTHING_STATE(pSaeIns);
	NdisZeroMemory(pSaeIns, offsetof(SAE_INSTANCE, valid));
	RTMPCancelTimer(&pSaeIns->sae_retry_timer, &Cancelled);
	/* RTMPReleaseTimer(&pSaeIns->sae_retry_timer, &Cancelled); */
}
/* sae insntance operation related api end */



UCHAR sae_using_anti_clogging(
	IN SAE_CFG *pSaeCfg)
{
	UINT32 i;
	UINT32 ins_cnt = 0;
	UINT32 open = 0;
	UINT16 wtbl_max_num = WTBL_MAX_NUM(pSaeCfg->pAd);

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	if (pSaeCfg->total_ins < pSaeCfg->sae_anti_clogging_threshold)
		return FALSE;

	NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

	for (i = 0; i < wtbl_max_num; i++) {
		if (pSaeCfg->sae_ins[i].valid == FALSE)
			continue;

		ins_cnt++;

		if ((pSaeCfg->sae_ins[i].state == SAE_COMMITTED)
			|| (pSaeCfg->sae_ins[i].state == SAE_CONFIRMED))
			open++;

		if (open >= pSaeCfg->sae_anti_clogging_threshold) {
			NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
			return TRUE;
		}

		if (ins_cnt == pSaeCfg->total_ins)
			break;
	}

	NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
	return FALSE;
}


VOID sae_set_retransmit_timer(
	IN SAE_INSTANCE *pSaeIns)
{

	if (pSaeIns) {
		if (pSaeIns->pParentSaeCfg) {
			RTMPSetTimer(&pSaeIns->sae_retry_timer, pSaeIns->pParentSaeCfg->dot11RSNASAERetransPeriod * 1000);
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG, "RTMPSetTimer\n");
		} else
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			"RTMPSetTimer fail!!\n");
	} else
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			"pSaeIns NULL!!\n");
}


VOID sae_clear_retransmit_timer(
	IN SAE_INSTANCE *pSaeIns)
{
	BOOLEAN Cancelled;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG, "==>\n");
	RTMPCancelTimer(&pSaeIns->sae_retry_timer, &Cancelled);
}

VOID sae_auth_retransmit(
	IN VOID *SystemSpecific1,
	IN VOID *FunctionContext,
	IN VOID *SystemSpecific2,
	IN VOID *SystemSpecific3)
{
	SAE_INSTANCE *pSaeIns = (SAE_INSTANCE *) FunctionContext;
	RALINK_TIMER_STRUCT *pTimer = (RALINK_TIMER_STRUCT *) SystemSpecific3;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pTimer->pAd;
	struct wifi_dev *wdev = NULL;
	UCHAR ret;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG, "==>\n");

	wdev = wdev_search_by_address(pAd, pSaeIns->own_mac);
	if (sae_check_big_sync(pSaeIns)) /* ellis */ {
		if (wdev)
			MlmeEnqueueWithWdev(pAd, AUTH_FSM, AUTH_FSM_AUTH_TIMEOUT, 0, NULL, 0, wdev);
		return;
	}

	switch (pSaeIns->state) {
	case SAE_COMMITTED:
		ret = sae_send_auth_commit(pAd, pSaeIns);
		sae_set_retransmit_timer(pSaeIns);
		break;

	case SAE_CONFIRMED:
		/* If Sync is not greater than dot11RSNASAESync, the Sync counter shall be incremented,
		  * Sc shall be incremented, and the protocol instance shall create a new Confirm (with the new Sc value) Message,
		  * transmit it to the peer, and set the t0 (retransmission) timer
		  */
		ret = sae_send_auth_confirm(pAd, pSaeIns);
		sae_set_retransmit_timer(pSaeIns);
		break;

	default:
		ret = FALSE;
		break;
	}

	if (ret == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 "retransemit fail (state = %d, sync = %d)\n",
				  pSaeIns->state, pSaeIns->sync);
	}
}

static UCHAR sae_check_commit_status_code_by_cap(
	IN USHORT status_code,
	IN struct sae_capability *sae_cap)
{
	if ((IS_H2E_SAE_COMMIT_STATUS_SUCCESS(status_code) && sae_cap->gen_pwe_method == PWE_LOOPING_ONLY)
			|| (status_code == MLME_SUCCESS && sae_cap->gen_pwe_method == PWE_HASH_ONLY)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
				"%s(): reject peer(status code = %x) due to pwe method is %d\n",
				 __func__, status_code, sae_cap->gen_pwe_method);
		return FALSE;
	}

	if (((status_code == MLME_SUCCESS || status_code == MLME_SAE_HASH_TO_ELEMENT) && sae_cap->sae_pk_en == SAE_PK_REQUIRED)
		|| (status_code == MLME_SAE_PUBLIC_KEY && sae_cap->sae_pk_en == SAE_PK_DISABLE)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
					"%s(): reject peer(status code = %x) due to sae_pk_en is %d\n",
					 __func__, status_code, sae_cap->sae_pk_en);
		return FALSE;
	}

	return TRUE;
}

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
	IN UCHAR sae_conn_type)
{
	SAE_INSTANCE *pSaeIns = search_sae_instance(pSaeCfg, own_mac, peer_mac);
	SAE_INSTANCE *pPreSaeIns = pSaeIns;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			 " pSaeIns = %p, pSaeIns->state = %d\n", pSaeIns, (pSaeIns) ? pSaeIns->state : -1);

	/* 12.4.8.6.1 Upon receipt of an Initiate event, the parent process shall check whether there exists a protocol instance for
	  * the peer MAC address (from the Init event) in either Committed or Confirmed state. If there is, the Initiate
	  * event shall be ignored. Otherwise, a protocol instance shall be created, and an Init event shall be sent to the
	  * protocol instance.
	  */
	if (pSaeIns &&
		((pSaeIns->state == SAE_COMMITTED)
		 || (pSaeIns->state == SAE_CONFIRMED)))
		return FALSE;

	pSaeIns = create_sae_instance(pAd, pSaeCfg, own_mac, peer_mac, bssid, psk, NULL, FALSE);

	if (!pSaeIns)
		return FALSE;

	pSaeIns->same_mac_ins = pPreSaeIns;

	if (pPreSaeIns)
		pPreSaeIns->same_mac_ins = pSaeIns;

	if (sae_group_allowed(pSaeIns, pSaeCfg->support_group, group) != MLME_SUCCESS)
		goto FAIL;

	if (pt_list != NULL && sae_conn_type)
		pSaeIns->pt = sae_search_pt_by_group(pt_list, pSaeIns->group, NULL);

	if (pSaeIns->pt != NULL)
		pSaeIns->connect_type = sae_conn_type;
	else if (sae_conn_type)
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 "pSaeIns->pt is null, change to looping\n");

	pSaeIns->sae_pk_ptr = sae_pk;

	if (sae_prepare_commit(pSaeIns) != MLME_SUCCESS)
		goto FAIL;

	if (sae_send_auth_commit(pAd, pSaeIns) == FALSE)
		goto FAIL;

	SET_COMMITTED_STATE(pSaeIns);
	sae_set_retransmit_timer(pSaeIns);
	return TRUE;
FAIL:
	delete_sae_instance(pSaeIns);
	return FALSE;
}

UCHAR sae_handle_auth(
	IN RTMP_ADAPTER *pAd,
	IN SAE_CFG *pSaeCfg,
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
	OUT UCHAR *sae_conn_type)
{
#define DATA_SIZE 80
	USHORT res = MLME_SUCCESS;
	FRAME_802_11 *Fr = (PFRAME_802_11)msg;
	SAE_INSTANCE *pSaeIns = search_sae_instance(pSaeCfg, Fr->Hdr.Addr1, Fr->Hdr.Addr2);
	UINT8 is_token_req = FALSE;
	UCHAR *token = NULL;
	UINT32 token_len = 0;
	UCHAR data[DATA_SIZE];
	UINT32 data_len = 0;
	UCHAR *pos;

	if ((sae_pk->sae_pk_test_ctrl & SAE_PK_CFG_TAKE_H2E_AS_SAEPK)
		&& auth_status == MLME_SAE_HASH_TO_ELEMENT)
		auth_status = MLME_SAE_PUBLIC_KEY;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			 "==>: receive seq #%d with status code %d, instance %p, own mac addr = "MACSTR", peer mac addr = "MACSTR"\n",
			  auth_seq, auth_status, pSaeIns, MAC2STR(Fr->Hdr.Addr1), MAC2STR(Fr->Hdr.Addr2));

	/* Upon receipt of a Com event, the t0 (retransmission) timer shall be cancelled in Committed/Confirmed state */
	/* Upon receipt of a Con event, the t0 (retransmission) timer shall be cancelled in Committed/Confirmed state */
	if (pSaeIns) {
		if (Fr->Hdr.FC.Retry == 0)
			pSaeIns->last_rcv_auth_seq = Fr->Hdr.Sequence;
		else {
			if (pSaeIns->last_rcv_auth_seq == Fr->Hdr.Sequence) {
				goto unfinished;
			}
		}
	}
	if (pSaeIns) {
		sae_clear_retransmit_timer(pSaeIns); /* ellis */
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
				 " state = %d\n", pSaeIns->state);
	}

	switch (auth_seq) {
	case SAE_COMMIT_SEQ:
		if (!IS_SAE_COMMIT_STATUS_SUCCESS(auth_status)) {
			if (!pSaeIns)
				goto unfinished;

			/* 12.4.8.6.3 Upon receipt of a Com event, the protocol instance shall check the Status of the Authentication frame. If the
			  * Status code is nonzero, the frame shall be silently discarded and a Del event shall be sent to the parent
			  * process.
			  */
			/* comments: it's not expected to receive the rejection commit msg in NOTHING/ACCEPTED state */
			if (pSaeIns->state == SAE_NOTHING
				|| pSaeIns->state == SAE_ACCEPTED) {
				delete_sae_instance(pSaeIns);
				pSaeIns = NULL;
				goto unfinished;
			} else if (pSaeIns->state == SAE_CONFIRMED) {
				/* 12.4.8.6.5 Upon receipt of a Com event, if the Status is nonzero, the frame shall be silently discarded, the t0 (retransmission) timer set
				  * If Sync is greater than dot11RSNASAESync, the protocol instance shall send the parent process a Del event and transitions back to Nothing state
				  */
				if (sae_check_big_sync(pSaeIns))
					goto unfinished;
				sae_set_retransmit_timer(pSaeIns);
				goto unfinished;
			}

			if (auth_status == MLME_ANTI_CLOGGING_TOKEN_REQ) {
				/*Check presence of Anti-Clogging Token*/
				/*If Anti-Clogging Token present store the anti-clogging token content and length*/
				pos = &Fr->Octet[6];

				os_alloc_mem(pAd, &pSaeIns->anti_clogging_token, MAX_SIZE_OF_ANTI_CLOGGING_PARAMETER);

				if (pSaeIns->anti_clogging_token == NULL) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
						 "AntiClogging token allocation fail\n");

					sae_set_retransmit_timer(pSaeIns);
					goto unfinished;

				}
				NdisZeroMemory(pSaeIns->anti_clogging_token, MAX_SIZE_OF_ANTI_CLOGGING_PARAMETER);
				if (pSaeIns->connect_type == SAE_CONNECTION_TYPE_H2E) {
					pSaeIns->anti_clogging_token_len = MAX_SIZE_OF_ANTI_CLOGGING_PARAMETER;
				} else {
					pSaeIns->anti_clogging_token_len = MAX_SIZE_OF_ANTI_CLOGGING_PARAMETER - 3;
				}
				NdisMoveMemory(pSaeIns->anti_clogging_token, pos + 2,
					pSaeIns->anti_clogging_token_len);



				sae_send_auth_commit(pAd, pSaeIns);
				sae_set_retransmit_timer(pSaeIns);
				goto unfinished;

			} else if (auth_status == MLME_FINITE_CYCLIC_GROUP_NOT_SUPPORTED) {
				USHORT sae_group;
				USHORT new_sae_group = 0;
				UCHAR *pos = &Fr->Octet[6];
				USHORT len;
				UCHAR newGroup[MAX_SIZE_OF_REJECTED_GROUPS];
				int i;
				/* 12.4.8.6.4 If the Status code is 77, the protocol instance shall check the finite cyclic group field being rejected.*/
				/* Check Finite Cyclic Group */
				NdisMoveMemory(&sae_group, pos, 2); /* ellis bigendian */
				sae_group = cpu2le16(sae_group);

				/* If the rejected group does not match the last offered group the protocol instance shall silently discard the message and set the t0 (retransmission) timer */
				if (sae_group != pSaeIns->group) {
					sae_set_retransmit_timer(pSaeIns);
					goto unfinished;
				} else {
					BOOL discard = FALSE;
					/* If the rejected group matches the last offered group,
					  * the protocol instance shall choose a different group and generate the PWE and the secret
					  * values according to 12.4.5.2; it then generates and transmits a new Commit Message to the peer,
					  * zeros Sync, sets the t0 (retransmission) timer, and remains in Committed state.
					  */

					/*If there are no other groups to choose,
					the protocol instance shall send a Del event to the parent process and transitions back to Nothing state. */

				pSaeIns->support_group_idx++;
				if (pSaeIns->support_group_idx < MAX_SIZE_OF_ALLOWED_GROUP)
					new_sae_group = pSaeCfg->support_group[pSaeIns->support_group_idx];

				if (new_sae_group != 0) {
					NdisZeroMemory(newGroup, MAX_SIZE_OF_REJECTED_GROUPS);
					len = pSaeIns->peer_rejected_group_len;
					if (len) {
						for (i = 0; i < pSaeIns->peer_rejected_group_len; i += 2) {
							NdisMoveMemory(&newGroup[i], &pSaeIns->peer_rejected_group[i], MAX_GROUP_LENGTH);
						}
					}

					if (sae_group_allowed(pSaeIns, pSaeCfg->support_group, new_sae_group) != MLME_SUCCESS)
						discard = TRUE;

					if (pt_list != NULL) {
						pSaeIns->pt = sae_search_pt_by_group(pt_list, new_sae_group, pSaeIns->pwd_id_ptr);
						if (pSaeIns->pt == NULL) {
							MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
							"pt not found\n");
							delete_sae_instance(pSaeIns);
							pSaeIns = NULL;
							break;
						}
					}

					if (!discard && (sae_prepare_commit(pSaeIns) != MLME_SUCCESS))
						discard = TRUE;

					}
					/*If there are no other groups to choose,
					the protocol instance shall send a Del event to the parent process and transitions back to Nothing state. */
					if ((new_sae_group == 0) || (discard == TRUE)) {
						delete_sae_instance(pSaeIns);
						pSaeIns = NULL;
						goto unfinished;
					}

					if (len) {
						for (i = 0; i < len; i += 2) {
							NdisMoveMemory(&pSaeIns->peer_rejected_group[i], &newGroup[i], MAX_GROUP_LENGTH);
						}
					}

					if (pSaeIns->connect_type == SAE_CONNECTION_TYPE_H2E) {
						pSaeIns->peer_rejected_group_len = len;
						NdisMoveMemory(&pSaeIns->peer_rejected_group[pSaeIns->peer_rejected_group_len], &sae_group, MAX_GROUP_LENGTH);
						pSaeIns->peer_rejected_group_len += MAX_GROUP_LENGTH;
						pSaeIns->rejected_group |= (1 << sae_group);
					}
					sae_send_auth_commit(pAd, pSaeIns);
					SET_COMMITTED_STATE(pSaeIns);
					sae_set_retransmit_timer(pSaeIns);
					goto unfinished;
				}
			} else if (auth_status == MLME_UNKNOWN_PASSWORD_IDENTIFIER) {
				/* 12.4.8.6.4 If the Status code is UNKNOWN_PASSWORD_IDENTIFIER,
				  * the protocol instance shall send a Del event to the parent process and transition back to Nothing state
				  */
				delete_sae_instance(pSaeIns);
				pSaeIns = NULL;
				goto unfinished;
			} else {
				/* 12.4.8.6.4 If the Status is some other nonzero value, the frame shall be silently discarded and the t0 (retransmission) timer shall be set.
				  * 12.4.8.6.5 Upon receipt of a Com event, the t0 (retransmission) timer shall be canceled. If the Status is nonzero,
				  * the frame shall be silently discarded, the t0 (retransmission) timer set, and the protocol instance shall remain in the Confirmed state
				  */
				sae_set_retransmit_timer(pSaeIns);
				goto unfinished;
			}
		}

		if (!sae_check_commit_status_code_by_cap(auth_status, sae_cap)) {
			res = MLME_UNSPECIFY_FAIL;
			break;
		}

		if (!pSaeIns
			|| pSaeIns->state == SAE_ACCEPTED) {
			/* 12.4.8.6, the parent process checks the value of Open first.
			  * If Open is not greater than dot11RSNASAEAntiCloggingThreshold or Anti-Clogging Token exists and is correct,
			  * the parent process shall create a protocol instance.
			  * comment: But, parsing anti-clogging token needs group info, so always create instance first
			  */
			SAE_INSTANCE *pPreSaeIns = pSaeIns;
			pSaeIns = create_sae_instance(pAd, pSaeCfg, Fr->Hdr.Addr1, Fr->Hdr.Addr2,
						Fr->Hdr.Addr3, psk, pwd_id_list_head, sae_cap->pwd_id_only);

			if (!pSaeIns) {
				res = MLME_UNSPECIFY_FAIL;
				break;
			}
			if (pSaeIns) {
				pSaeIns->last_rcv_auth_seq = Fr->Hdr.Sequence;
				pSaeIns->same_mac_ins = pPreSaeIns;
			}
			if (pPreSaeIns)
				pPreSaeIns->same_mac_ins = pSaeIns;
		}

		if (IS_H2E_SAE_COMMIT_STATUS_SUCCESS(auth_status) && sae_cap->gen_pwe_method != PWE_LOOPING_ONLY)
			pSaeIns->connect_type = (auth_status == MLME_SAE_HASH_TO_ELEMENT) ? SAE_CONNECTION_TYPE_H2E : SAE_CONNECTION_TYPE_SAEPK;

		res = sae_parse_commit(pSaeCfg, pSaeIns, msg, msg_len, &token, &token_len, is_token_req);

		if (res == MLME_FINITE_CYCLIC_GROUP_NOT_SUPPORTED) {
			/* 12.4.8.6.3(NOTHING) the frame shall be processed by first checking the finite cyclic group field to see if the requested group is supported.
			  * If not, BadGrp shall be set and the protocol instance shall construct and transmit a an Authentication frame with Status code UNSUPPORTED_FINITE_CYCLIC_GROUP
			  * indicating rejection with the finite cyclic group field set to the rejected group, and shall send the parent process a Del event
			  */
			if (pSaeIns->state == SAE_NOTHING) {
				NdisMoveMemory(data, &Fr->Octet[6], 2); /* copy peer group id */
				data_len = 2;
				if (pSaeIns->connect_type == SAE_CONNECTION_TYPE_LOOPING) {
					delete_sae_instance(pSaeIns);
					pSaeIns = NULL;
				}
				break;
			}
			/* 12.4.8.6.4(COMMITTED) If the Status is zero, the finite cyclic group field is checked. If the group is not supported, BadGrp shall be set and the value of Sync shall be checked.
			  * -If Sync is greater than dot11RSNASAESync, the protocol instance shall send a Del event to the parent process and transitions back to Nothing state.
			  * -If Sync is not greater than dot11RSNASAESync, Sync shall be incremented, a Commit Message with Status code equal to 77 indicating rejection,
			  * and the Algorithm identifier set to the rejected algorithm, shall be sent to the peer,
			  * the t0 (retransmission) timer shall be set and the protocol instance shall remain in Committed state
			  */
			if (pSaeIns->state == SAE_COMMITTED && sae_check_big_sync(pSaeIns))
				goto unfinished;

			if (pSaeIns->state == SAE_COMMITTED)
				sae_set_retransmit_timer(pSaeIns);

			/* 12.4.8.6.5(CONFIRMED) the protocol instance shall verify that the finite cyclic group is the same as the previously received Commit frame.
			  * If not, the frame shall be silently discarded
			  * If so, the protocol instance shall increment Sync, increment Sc, and transmit its Commit and Confirm (with the new Sc value) messages.
			  * It then shall set the t0 (retransmission) timer.
			  */
			if (pSaeIns->state == SAE_CONFIRMED) {
				sae_set_retransmit_timer(pSaeIns);
				goto unfinished;
			}
		} else if (res == MLME_UNKNOWN_PASSWORD_IDENTIFIER) {
			/* 12.4.8.6.3 Protocol instance behavior - Nothing state
			  * the frame shall be processed by first checking (M41)whether a password identifier is present.
			  * If so and there is no password associated with that identifier, BadID shall be set and the protocol instance
			  * shall construct and transmit an Authentication frame with Status Code set to UNKNOWN_PASSWORD_IDENTIFIER.
			  */
			delete_sae_instance(pSaeIns);
			pSaeIns = NULL;
			break;
		} else if (res == SAE_SILENTLY_DISCARDED) {
			sae_set_retransmit_timer(pSaeIns);
			goto unfinished;
		} else if (res != MLME_SUCCESS) {
			if ((pSaeIns->state == SAE_NOTHING) || (pSaeIns->state == SAE_COMMITTED)) {
				delete_sae_instance(pSaeIns);
				pSaeIns = NULL;
			}
			break;
		}

		if (IS_H2E_SAE_COMMIT_STATUS_SUCCESS(auth_status)) {
			if (!pt_list) {
				MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
						" pt list should not be null\n");
				delete_sae_instance(pSaeIns);
				pSaeIns = NULL;
				break;
			}
			/* 12.4.4.2.3 Hash-to-curve generation of the password element with ECC groups
			  * If an SAE Commit message is received with status code equal to SAE_HASH_TO_ELEMENT
			  * the peer shall generate the  PWE using the following technique and reply
			  * with its own SAE Commit message with status code equal to SAE_HASH_TO_ELEMENT.
			  */
			if (pSaeIns->pt == NULL) {
				pSaeIns->pt = sae_search_pt_by_group(pt_list, pSaeIns->group, pSaeIns->pwd_id_ptr);
				if (pSaeIns->pt == NULL) {
					MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
						"pt not found\n");
					delete_sae_instance(pSaeIns);
					pSaeIns = NULL;
					break;
				}
			}
		}

		if (pSaeIns->pwd_id_ptr && pSaeIns->connect_type == SAE_CONNECTION_TYPE_LOOPING) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
					"pwd id should be use h2e connection instead of looping\n");
			res = MLME_UNSPECIFY_FAIL;
			delete_sae_instance(pSaeIns);
			pSaeIns = NULL;
			break;
		}
		res = sae_check_rejected_group(pSaeIns);

		if (res != MLME_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
						"check rejected groups fail\n");
			delete_sae_instance(pSaeIns);
			pSaeIns = NULL;
			break;
		}

		if (token && sae_check_token(pSaeIns, token, token_len) == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 "check token fail with peer mac "MACSTR"\n",
				  MAC2STR(pSaeIns->peer_mac));
			delete_sae_instance(pSaeIns);
			pSaeIns = NULL;
			res = MLME_UNSPECIFY_FAIL;
			break;
		}

		if (!token && sae_using_anti_clogging(pSaeCfg)) {
			sae_build_token_req(pAd, pSaeIns, data, &data_len);
			res = MLME_ANTI_CLOGGING_TOKEN_REQ;
			break;
		}

		if (pSaeIns->connect_type == SAE_CONNECTION_TYPE_SAEPK)
			pSaeIns->sae_pk_ptr = sae_pk;

		res = sae_sm_step(pAd, pSaeIns, auth_seq);
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
				 " SAE_COMMIT_SEQ, res(sae_sm_step) = %d\n", res);
		break;

	case SAE_CONFIRM_SEQ:
		if (!pSaeIns || pSaeIns->state == SAE_NOTHING)
			goto unfinished;

		/* 12.4.8.6.5 Rejection frames received in Confirmed state shall be silently discarded */
		/* Comment: It is not clear in spec about how to handle confirm message with error status. */
		if (auth_status != MLME_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
					 "receive error status auth confirm msg, so delete the instance\n");
			delete_sae_instance(pSaeIns);
			pSaeIns = NULL;
			goto unfinished;
		}
		if (pSaeIns->state == SAE_CONFIRMED || pSaeIns->state == SAE_ACCEPTED)
			res = sae_parse_confirm(pSaeIns, msg, msg_len);

		/* Comment: It is not clear in spec about how to handle if the confirm be verified fail. */
		if (res != MLME_SUCCESS) {
			if (pSaeIns->state == SAE_ACCEPTED) {
				/* 12.4.8.6.6(ACCEPTED) Upon receipt of a Con event, the Sync counter shall be checked */
				/* the value of send-confirm shall be checked. If the value is not greater than Rc or is equal to 2^16 ¡V 1,
				  * the received frame shall be silently discarded. Otherwise, the Confirm portion of the frame shall be checked according to 12.4.5.6.
				  * If the verification fails, the received frame shall be silently discarded
				  */
				sae_check_big_sync(pSaeIns);
				goto unfinished;
			}
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
					 "verify confirm fail, remove instance\n");
			delete_sae_instance(pSaeIns);
			pSaeIns = NULL;
			break;
		}

		if (pSaeIns->rejected_group) {
			NdisZeroMemory(pSaeIns->peer_rejected_group, MAX_SIZE_OF_REJECTED_GROUPS);
			pSaeIns->rejected_group = 0;
			pSaeIns->peer_rejected_group_len = 0;
		}

		res = sae_sm_step(pAd, pSaeIns, auth_seq);
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			" SAE_CONFIRM_SEQ, res(sae_sm_step) = %d\n", res);
		break;

	default:
		if (pSaeIns) {
			delete_sae_instance(pSaeIns);
			pSaeIns = NULL;
		}

		res = MLME_SEQ_NR_OUT_OF_SEQUENCE;
		break;
	}

	if (res != MLME_SUCCESS
		&& res != SAE_SILENTLY_DISCARDED)
		sae_send_auth(pAd, Fr->Hdr.Addr1, Fr->Hdr.Addr2, Fr->Hdr.Addr3, AUTH_MODE_SAE, auth_seq, res, data, data_len);

	if (pSaeIns && pSaeIns->state == SAE_ACCEPTED) {
		store_time_log_by_tag(LOG_TIME_SAE, &pSaeIns->sae_tl);
		SAE_LOG_TIME_DUMP();
		ecc_point_dump_time();
		if (pmk) {
			*pmk = pSaeIns->pmk;
			hex_dump_with_lvl("sae success, pmk:", (char *)*pmk, LEN_PMK, DBG_LVL_INFO);
		}
		if (sae_conn_type)
			*sae_conn_type =
			(sae_pk->sae_pk_test_ctrl & SAE_PK_CFG_TAKE_H2E_AS_SAEPK) ?
			SAE_CONNECTION_TYPE_H2E : pSaeIns->connect_type;

		return TRUE;
	}
unfinished:
	*pmk = NULL;
	if (!pSaeIns)
		return FALSE;
	else
		return TRUE;

}

USHORT sae_sm_step(
	IN RTMP_ADAPTER *pAd,
	IN SAE_INSTANCE *pSaeIns,
	IN USHORT auth_seq)
{
#define F(a, b) (a << 2 | b)
	USHORT res = MLME_SUCCESS;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	switch (F(pSaeIns->state, auth_seq)) {
	case F(SAE_NOTHING, SAE_COMMIT_SEQ):
		/* 12.4.8.6.3 If validation of the received
		  * Commit Message fails, the protocol instance shall send a Del event to the parent process; otherwise, it shall
		  * construct and transmit a Commit Message (see 12.4.5.3) followed by a Confirm Message (see 12.4.5.5). The
		  * Sync counter shall be set to zero and the t0 (retransmission) timer shall be set. The protocol instance
		  * transitions to Confirmed state.
		  */
		res = sae_prepare_commit(pSaeIns);

		if (res != MLME_SUCCESS)
			return res;

		if (wpa3_test_ctrl == 2) {
			res = sae_process_commit(pSaeIns);

			if (res != MLME_SUCCESS)
				return res;

			if (sae_send_auth_commit(pAd, pSaeIns) == FALSE)
				return SAE_SILENTLY_DISCARDED;

			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
				"let confirm right after commit\n");
		} else {
			if (sae_send_auth_commit(pAd, pSaeIns) == FALSE)
				return SAE_SILENTLY_DISCARDED;

			res = sae_process_commit(pSaeIns);

			if (res != MLME_SUCCESS)
				return res;
		}

		if (wpa3_test_ctrl == 3 || wpa3_test_ctrl == 4) {
			SET_COMMITTED_STATE(pSaeIns);
			pSaeIns->sync = 0;
			sae_set_retransmit_timer(pSaeIns);
			break;
		}

		if (sae_send_auth_confirm(pAd, pSaeIns) == FALSE)
			return SAE_SILENTLY_DISCARDED;
		SET_CONFIRMED_STATE(pSaeIns);
		pSaeIns->sync = 0;
		sae_set_retransmit_timer(pSaeIns);
		break;

	case F(SAE_COMMITTED, SAE_COMMIT_SEQ):
		/* 12.4.8.6.4 If the received element and scalar differ from the element and
		  * scalar offered, the received Commit Message shall be processed according to 12.4.5.4, the Sc
		  * counter shall be incremented (thereby setting its value to one), the protocol instance shall then
		  * construct a Confirm Message, transmit it to the peer, and set the t0 (retransmission) timer. It shall
		  * then transition to Confirmed state.
		  */
		res = sae_process_commit(pSaeIns);
		if (res != MLME_SUCCESS)
			return res;
		if (sae_send_auth_confirm(pAd, pSaeIns) == FALSE)
			return SAE_SILENTLY_DISCARDED;
		SET_CONFIRMED_STATE(pSaeIns);
		pSaeIns->sync = 0;
		sae_set_retransmit_timer(pSaeIns);
		break;

	case F(SAE_COMMITTED, SAE_CONFIRM_SEQ):
		/* 12.4.8.6.4 Upon receipt of a Con event, If Sync is not greater than
		  * dot11RSNASAESync, the protocol instance shall increment Sync, transmit the last Commit Message sent to
		  * the peer, and set the t0 (retransmission) timer.
		  * comments: In COMMITTED state, it's still awaiting for peer commit msg
		  */
		if (sae_send_auth_commit(pAd, pSaeIns) == FALSE)
			return SAE_SILENTLY_DISCARDED;
		sae_set_retransmit_timer(pSaeIns);
		break;

	case F(SAE_CONFIRMED, SAE_COMMIT_SEQ):
		/* 12.4.8.6.5 the protocol instance shall verify that the finite cyclic group is the same as the previously received Commit
		  * frame. If not, the frame shall be silently discarded. If so, the protocol instance shall increment Sync,
		  * increment Sc, and transmit its Commit and Confirm (with the new Sc value) messages.
		  * It then shall set the t0 (retransmission) timer.
		  */
		if (sae_check_big_sync(pSaeIns))
			return MLME_SUCCESS;
		if (pSaeIns->need_recalculate_key) {
			res = sae_process_commit(pSaeIns);
			if (res != MLME_SUCCESS)
				return res;
			pSaeIns->need_recalculate_key = FALSE;
		}

		if (sae_send_auth_commit(pAd, pSaeIns) == FALSE)
			return SAE_SILENTLY_DISCARDED;
		if (sae_send_auth_confirm(pAd, pSaeIns) == FALSE)
			return SAE_SILENTLY_DISCARDED;
		sae_set_retransmit_timer(pSaeIns);
		break;

	case F(SAE_CONFIRMED, SAE_CONFIRM_SEQ):
		/* 12.4.8.6.5 If processing is successful and the Confirm Message has been verified,
		  * the Rc variable shall be set to the send-confirm portion of the frame, Sc shall be set to the value 2^16 ¡V 1, the
		  * t1 (key expiry) timer shall be set, and the protocol instance shall transition to Accepted state
		  */
		pSaeIns->last_peer_sc = pSaeIns->peer_send_confirm;
		pSaeIns->send_confirm = SAE_MAX_SEND_CONFIRM;

		/* If another protocol instance exists in the database indexed by the same peer identity as the protocol
		  * instance that sent the Auth event, the other protocol instance shall be destroyed.
		  */
		if (pSaeIns->same_mac_ins) {
			delete_sae_instance(pSaeIns->same_mac_ins);
			pSaeIns->same_mac_ins = NULL;
		}

		SET_ACCEPTED_STATE(pSaeIns);
		/* ellis todo: t1 (key expiry) timer shall be set, and the protocol instance shall transition to Accepted state */
		/* auth done */
		break;

	case F(SAE_ACCEPTED, SAE_CONFIRM_SEQ):
		/* 12.4.8.6.6 Upon receipt of a Con event, the Sync counter shall be checked */
		if (sae_check_big_sync(pSaeIns))
			return MLME_SUCCESS;

		/* 12.4.8.6.6 If the verification succeeds, the Rc variable
		  * shall be set to the send-confirm portion of the frame, the Sync shall be incremented and a new Confirm
		  * Message shall be constructed (with Sc set to 216 ¡V 1) and sent to the peer
		  */
		pSaeIns->sync++;
		pSaeIns->last_peer_sc = pSaeIns->peer_send_confirm;
		if (sae_send_auth_confirm(pAd, pSaeIns) == FALSE)
			return SAE_SILENTLY_DISCARDED;
		break;

	default:
		break;
	}

	return res;
}

UINT32 sae_ecc_prime_len_2_hash_len(UINT32 prime_len)
{
	if (prime_len <= SHA256_DIGEST_SIZE)
		return SHA256_DIGEST_SIZE;
	if (prime_len <= SHA384_DIGEST_SIZE)
		return SHA384_DIGEST_SIZE;
	return SHA512_DIGEST_SIZE;
}


/* if this api return TRUE, the instance will be removed, the caller should directly return and not access the instance */
UCHAR sae_check_big_sync(
	IN SAE_INSTANCE *pSaeIns)
{
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	if (pSaeIns->sync > DOT11RSNASAESYNC) {
		delete_sae_instance(pSaeIns);
		return TRUE;
	}

	pSaeIns->sync++;

	return FALSE;
}

UCHAR sae_get_pmk_cache(
	IN SAE_CFG *pSaeCfg,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	OUT UCHAR *pmkid,
	OUT UCHAR *pmk)
{
	/* PMKID = L((commit-scalar + peer-commit-scalar) mod r, 0, 128) */
	SAE_BN *tmp = NULL;
	UINT32 len = LEN_PMKID;
	SAE_INSTANCE *pSaeIns = search_sae_instance(pSaeCfg, own_mac, peer_mac);
	UCHAR sae_buf[80];

	if (pSaeIns == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			 "%s(): pSaeIns not found\n", __func__);
		return FALSE;
	}

	if (pSaeIns->state != SAE_ACCEPTED
		|| !pSaeIns->own_commit_scalar
		|| !pSaeIns->peer_commit_scalar) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 "get pmkid fail\n");
		return FALSE;
	}

	if (pmkid) {
		SAE_BN_INIT(&tmp);

		SAE_BN_MOD_ADD_QUICK(pSaeIns->own_commit_scalar, pSaeIns->peer_commit_scalar, pSaeIns->order, &tmp);
		SAE_BN_BI2BIN_WITH_PAD(tmp, sae_buf, &len, pSaeIns->prime_len);
		SAE_BN_FREE(&tmp);
		NdisMoveMemory(pmkid, sae_buf, LEN_PMKID);
	}

	if (pmk)
		NdisMoveMemory(pmk, pSaeIns->pmk, LEN_PMK);

	return TRUE;
}

static VOID sae_renew_token_key(
	IN SAE_CFG * pSaeCfg)
{
#define TOKEN_REKEY_INTERVAL 100000 /* unit: jiffies*/
	ULONG cur_time;
	UINT32 i;

	NdisGetSystemUpTime(&cur_time);

	if (cur_time > (pSaeCfg->last_token_key_time + TOKEN_REKEY_INTERVAL) &&
		(cur_time - TOKEN_REKEY_INTERVAL) > pSaeCfg->last_token_key_time) {
		pSaeCfg->last_token_key_time = cur_time;
		for (i = 0; i < SAE_TOKEN_KEY_LEN; i++)
			pSaeCfg->token_key[i] = RandomByte(pSaeCfg->pAd);
	}
}

UCHAR sae_build_token_req(
	IN RTMP_ADAPTER * pAd,
	IN SAE_INSTANCE * pSaeIns,
	OUT UCHAR *token_req,
	OUT UINT32 * token_req_len)
{
	SAE_CFG *sae_cfg = pSaeIns->pParentSaeCfg;
	UINT32 len = 0;

#ifdef RT_BIG_ENDIAN
	USHORT sae_group = 0;
	sae_group = cpu2le16(pSaeIns->group);
	NdisMoveMemory(token_req, &sae_group, 2);
#else
	NdisMoveMemory(token_req, &pSaeIns->group, 2);
#endif

	len += 2;
	if (pSaeIns->connect_type) {
		token_req[len] = IE_WLAN_EXTENSION;
		token_req[len + 1] = SHA256_DIGEST_SIZE + 1;
		token_req[len + 2] = EID_EXT_ANTI_CLOGGING_TOKEN;
		len += 3;
	}
	sae_renew_token_key(sae_cfg);
	RT_HMAC_SHA256(sae_cfg->token_key, SAE_TOKEN_KEY_LEN, pSaeIns->peer_mac,
					MAC_ADDR_LEN, token_req + len, SHA256_DIGEST_SIZE);
	*token_req_len = SHA256_DIGEST_SIZE + len;

	return TRUE;
}


UCHAR sae_check_token(
	IN SAE_INSTANCE * pSaeIns,
	IN UCHAR *peer_token,
	IN UINT32 peer_token_len)
{
	SAE_CFG *sae_cfg = pSaeIns->pParentSaeCfg;
	UCHAR token[SHA256_DIGEST_SIZE] = {0};

	if (peer_token_len != SHA256_DIGEST_SIZE)
		return FALSE;

	RT_HMAC_SHA256(sae_cfg->token_key, SAE_TOKEN_KEY_LEN, pSaeIns->peer_mac, MAC_ADDR_LEN, token, SHA256_DIGEST_SIZE);

	if (RTMPEqualMemory(token, peer_token, SHA256_DIGEST_SIZE))
		return TRUE;

	return FALSE;

}


VOID sae_parse_commit_token_req(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end,
	IN UCHAR **token,
	IN UINT32 *token_len)
{

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);
/*George:As per spec in Anticlogging request frame only group and anticlogging token expected in the commit frame*/
	if ((end - *pos) > 0) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			"%s:add anti clogging token\n", __func__);

		if (token)
			*token = *pos;

		if (token_len) {
			*token_len = (UINT32)(end - *pos);
			*pos += *token_len;
		}
	} else {
		if (token)
			*token = NULL;

		if (token_len)
			*token_len = 0;
	}
}


USHORT sae_parse_commit(
	IN SAE_CFG *pSaeCfg,
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *msg,
	IN UINT32 msg_len,
	IN UCHAR **token,
	IN UINT32 *token_len,
	IN UCHAR is_token_req)
{
	USHORT sae_group;
	USHORT res;
	FRAME_802_11 *Fr = (PFRAME_802_11)msg;
	UCHAR *pos = &Fr->Octet[6];
	UCHAR *end = msg + msg_len;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);
	/* Check Finite Cyclic Group */
	NdisMoveMemory(&sae_group, pos, 2); /* ellis bigendian */
	sae_group = cpu2le16(sae_group);
	res = sae_group_allowed(pSaeIns, pSaeCfg->support_group, sae_group);

	if (res == MLME_FINITE_CYCLIC_GROUP_NOT_SUPPORTED &&
		pSaeIns->connect_type) {
		if (sae_group >= MAX_SAE_GROUP) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 "group id is larger than 32\n");
			return MLME_UNSPECIFY_FAIL;
		}
		pSaeIns->rejected_group |= (1 << sae_group);

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			"%s: pSaeIns->rejected_group = %x\n", __func__, pSaeIns->rejected_group);
	}

	if (res != MLME_SUCCESS)
		return res;

	pos = pos + 2;

	if (is_token_req == TRUE && pSaeIns->connect_type == SAE_CONNECTION_TYPE_LOOPING) {
		/* process the rejection with anti-clogging */
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
					 "Parsing AntiClogging Request token\n");

		sae_parse_commit_token_req(pSaeIns, &pos, end, token, token_len);
		return MLME_SUCCESS;
	}

	/* Optional Anti-Clogging Token */
	sae_parse_commit_token(pSaeIns, &pos, end, token, token_len);
	log_time_begin(LOG_TIME_UNIT_MS, &pSaeIns->sae_tl.parse_commit_scalar_time);
	/* commit-scalar */
	res = sae_parse_commit_scalar(pSaeIns, &pos, end);

	if (res != MLME_SUCCESS)
		return res;

	log_time_end(LOG_TIME_SAE, "parse_commit_scalar_time", DBG_LVL_INFO, &pSaeIns->sae_tl.parse_commit_scalar_time);
	/* commit-element */
	res = sae_parse_commit_element(pSaeIns, &pos, end);

	if (res != MLME_SUCCESS)
		return res;

	/* 12.4.8.6.4 the protocol instance checks the peer-commit-scalar and PEER-COMMIT-ELEMENT
	  * from the message. If they match those sent as part of the protocol instance¡¦s own Commit Message,
	  * the frame shall be silently discarded (because it is evidence of a reflection attack)
	  */
	if (pSaeIns->group_op)
		res = pSaeIns->group_op->sae_reflection_check(pSaeIns);
	else
		return MLME_UNSPECIFY_FAIL;

	if (res != MLME_SUCCESS)
		return res;

	/* Optional Password Identifier element */
	res = sae_parse_password_identifier(pSaeIns, &pos, end, pSaeIns->is_pwd_id_only);

	if (res != MLME_SUCCESS)
		return res;

	/* Conditional Rejected Groups element */
	if (pSaeIns->connect_type)
		res = sae_parse_rejected_groups(pSaeIns, &pos, end);

	/* Optional Anti-Clogging Token Container element */
	if (pSaeIns->connect_type)
		res = sae_parse_token_container(pSaeIns, pos, end, token, token_len);

	return res;
}

VOID sae_parse_commit_token(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end,
	IN UCHAR **token,
	IN UINT32 *token_len)
{
	UINT32 non_token_len = (is_sae_group_ecc(pSaeIns->group) ? 3 : 2) * pSaeIns->prime_len; /* ellis */
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	if (pSaeIns->connect_type)
		return;

	if (*pos + non_token_len < end) {
		if (is_sae_pwd_id_element(*pos + non_token_len, end, NULL)) {
			if (token)
				*token = NULL;

			if (token_len)
				*token_len = 0;

			return;
		}

		if (token)
			*token = *pos;

		if (token_len) {
			*token_len = (UINT32)(end - *pos) - non_token_len;
			*pos += *token_len;
		}
	} else {
		if (token)
			*token = NULL;

		if (token_len)
			*token_len = 0;
	}
}


USHORT sae_parse_commit_scalar(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end)
{
	SAE_BN *peer_scalar = NULL;
	SAE_INSTANCE *pPreSaeIns = pSaeIns->same_mac_ins;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	if (*pos + pSaeIns->prime_len > end) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 "not enough data for scalar\n");
		return MLME_UNSPECIFY_FAIL;
	}

	hex_dump_with_cat_and_lvl("peer scalar:", (char *)*pos, pSaeIns->prime_len,
					DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG);
	SAE_BN_BIN2BI(*pos, pSaeIns->prime_len, &peer_scalar);

	/*
	 * IEEE Std 802.11-2016, 12.4.8.6.1: If there is a protocol instance for
	 * the peer and it is in Authenticated state, the new Commit Message
	 * shall be dropped if the peer-scalar is identical to the one used in
	 * the existing protocol instance.
	 */

	if (pPreSaeIns
		&& (pPreSaeIns->state == SAE_ACCEPTED)
		&& (pPreSaeIns->peer_commit_scalar)
		&& !SAE_BN_UCMP(peer_scalar, pPreSaeIns->peer_commit_scalar)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 "do not accept re-use of previous peer-commit-scalar\n");
		SAE_BN_FREE(&peer_scalar);
		return MLME_UNSPECIFY_FAIL;
	}

	if (pSaeIns->state == SAE_CONFIRMED
		&& (pSaeIns->peer_commit_scalar)
		&& SAE_BN_UCMP(peer_scalar, pSaeIns->peer_commit_scalar))
		pSaeIns->need_recalculate_key = TRUE;

	/* 12.4.5.4 If the scalar value is greater than zero (0) and less than the order, r, of the negotiated group, scalar validation succeeds */
	/* according to test plan 4.2.6, we should reject peer if scalar value is 1*/
	/* 1 < scalar < r */
	if (SAE_BN_IS_ZERO(peer_scalar)
		|| SAE_BN_IS_ONE(peer_scalar)
		|| (SAE_BN_UCMP(peer_scalar, pSaeIns->order) >= 0)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 "Invalid peer scalar\n");
		SAE_BN_FREE(&peer_scalar);
		return MLME_UNSPECIFY_FAIL;
	}

	SAE_BN_FREE(&pSaeIns->peer_commit_scalar);
	pSaeIns->peer_commit_scalar = peer_scalar;
	*pos += pSaeIns->prime_len;
	return MLME_SUCCESS;
}

USHORT sae_parse_commit_element(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end)
{
	USHORT res = MLME_UNSPECIFY_FAIL;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);
	log_time_begin(LOG_TIME_UNIT_MS, &pSaeIns->sae_tl.parse_commit_element_time);

	if (pSaeIns->group_op)
		res = pSaeIns->group_op->sae_parse_commit_element(pSaeIns, pos, end);

	log_time_end(LOG_TIME_SAE, "parse_commit_element_time", DBG_LVL_INFO, &pSaeIns->sae_tl.parse_commit_element_time);
	return res;
}

UCHAR is_sae_pwd_id_element(
	IN UCHAR *pos,
	IN UCHAR *end,
	OUT UINT32 *len)
{
	if (!(end - pos < 3) &&
		pos[0] == IE_WLAN_EXTENSION &&
		pos[1] >= 1 &&
		end - pos - 2 >= pos[1] &&
		pos[2] == EID_EXT_PASSWORD_IDENTIFIER) {
		if (len)
			*len = pos[1] - 1;
		return TRUE;
	} else
		return FALSE;
}

UCHAR is_sae_rejected_group_element(
	IN UCHAR *pos,
	IN UCHAR *end,
	OUT UINT32 *len)
{
	if (!(end - pos < 3) &&
		pos[0] == IE_WLAN_EXTENSION &&
		pos[1] >= 1 &&
		end - pos - 2 >= pos[1] &&
		pos[2] == EID_EXT_REJECTED_GROUP) {
		if (len)
			*len = pos[1] - 1;
		return TRUE;
	} else
		return FALSE;
}

UCHAR is_sae_token_container_element(
	IN UCHAR *pos,
	IN UCHAR *end,
	OUT UINT32 *len)
{
	if (!(end - pos < 3) &&
		pos[0] == IE_WLAN_EXTENSION &&
		pos[1] >= 1 &&
		end - pos - 2 >= pos[1] &&
		pos[2] == EID_EXT_ANTI_CLOGGING_TOKEN) {
		if (len)
			*len = pos[1] - 1;
		return TRUE;
	} else
		return FALSE;
}



USHORT sae_parse_password_identifier(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end,
	IN UCHAR is_pwd_id_only)
{
	if (!is_sae_pwd_id_element(*pos, end, &pSaeIns->peer_pwd_id_len)) {
		if (is_pwd_id_only) {
			pSaeIns->peer_pwd_id_len = 0;
			return MLME_UNKNOWN_PASSWORD_IDENTIFIER;
		} else
			return MLME_SUCCESS;
	}

	pSaeIns->peer_pwd_id = *pos + 3;

	hex_dump_with_cat_and_lvl("peer pwd id", pSaeIns->peer_pwd_id,
		pSaeIns->peer_pwd_id_len, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO);

	if (pSaeIns->pwd_id_list_head != NULL &&
		!DlListEmpty(&pSaeIns->pwd_id_list_head->list)) {
		struct pwd_id_list *list = NULL;

		DlListForEach(list, &pSaeIns->pwd_id_list_head->list, struct pwd_id_list, list) {
			if (RTMPEqualMemory(list->pwd_id, pSaeIns->peer_pwd_id, pSaeIns->peer_pwd_id_len)) {
				pSaeIns->psk = list->pwd;
				pSaeIns->pwd_id_ptr = list;
				return MLME_SUCCESS;
			}
		}
	}

	*pos = *pos + 2 + (*pos)[1];

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			 "%s(): pwd id search fail\n", __func__);

	return MLME_UNKNOWN_PASSWORD_IDENTIFIER;
}


USHORT sae_parse_rejected_groups(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end)
{
	UINT32 len;

	if (!is_sae_rejected_group_element(*pos, end, &len))
			return MLME_SUCCESS;

	if (len % 2) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 "rejected groups len(no include EID_EXT_REJECTED_GROUP) should be 2n bytes\n");
		return MLME_UNSPECIFY_FAIL;
	}

	if (len > sizeof(pSaeIns->peer_rejected_group)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 "peer_rejected_group array is not enough to store rejected group\n");
		return MLME_UNSPECIFY_FAIL;
	}

	NdisMoveMemory(pSaeIns->peer_rejected_group, *pos + 3, len);
	pSaeIns->peer_rejected_group_len = len;
	*pos = *pos + 2 + (*pos)[1];
	hex_dump_with_lvl("peer_rejected_group", pSaeIns->peer_rejected_group, len, DBG_LVL_ERROR);

	return MLME_SUCCESS;
}


USHORT sae_parse_token_container(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *pos,
	IN UCHAR *end,
	IN UCHAR **token,
	IN UINT32 *token_len)
{
	UINT32 len;

	if (!is_sae_token_container_element(pos, end, &len))
		return MLME_SUCCESS;

	*token = pos + 3;
	*token_len = len;
	return MLME_SUCCESS;
}



USHORT sae_check_rejected_group(
	IN SAE_INSTANCE *pSaeIns)
{
	UCHAR i;
	USHORT group;
	UINT32 peer_rejected_group = 0;

	if (pSaeIns->connect_type == SAE_CONNECTION_TYPE_LOOPING)
		return MLME_SUCCESS;
	else if (pSaeIns->rejected_group == 0 &&
		pSaeIns->peer_rejected_group_len == 0)
		return MLME_SUCCESS;
	else if (pSaeIns->rejected_group == 0 || pSaeIns->peer_rejected_group_len == 0) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 "rejected_group = %d, peer_rejected_group_len = %d\n",
				 pSaeIns->rejected_group, pSaeIns->peer_rejected_group_len);
		return MLME_UNSPECIFY_FAIL;
	}

	for (i = 0; i < pSaeIns->peer_rejected_group_len; i += 2) {
		NdisMoveMemory(&group, &pSaeIns->peer_rejected_group[i], 2);
		if (group >= MAX_SAE_GROUP) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				"group is larger than  32\n");
			return MLME_UNSPECIFY_FAIL;
		}
		peer_rejected_group |= (1 << group);
	}

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
		"%s: peer_rejected_group = %x, rejected_group = %x\n",
		__func__, peer_rejected_group, pSaeIns->rejected_group);

	if (peer_rejected_group == pSaeIns->rejected_group) {
		pSaeIns->rejected_group = 0;
		return MLME_SUCCESS;
	}
	else
		return MLME_UNSPECIFY_FAIL;
}


USHORT sae_prepare_commit(
	IN SAE_INSTANCE *pSaeIns)
{
	USHORT res;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);
	log_time_begin(LOG_TIME_UNIT_MS, &pSaeIns->sae_tl.derive_pwe_time);

	if (pSaeIns->group_op) {
		if (pSaeIns->connect_type)
			res = pSaeIns->group_op->sae_derive_pwe_pt(pSaeIns);
		else
			res = pSaeIns->group_op->sae_derive_pwe(pSaeIns);
	} else
		return MLME_UNSPECIFY_FAIL;

	log_time_end(LOG_TIME_SAE, "derive_pwe_time", DBG_LVL_INFO, &pSaeIns->sae_tl.derive_pwe_time);

	if (res != MLME_SUCCESS)
		return res;

	return sae_derive_commit(pSaeIns);
}


USHORT sae_derive_commit(
	IN SAE_INSTANCE *pSaeIns)
{
	SAE_BN *mask = NULL;
	USHORT res = MLME_SUCCESS;
	UINT32 counter = 0;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);
	log_time_begin(LOG_TIME_UNIT_MS, &pSaeIns->sae_tl.derive_commit_scalar_time);

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[0]);

	do {
		counter++;

		if (counter > 100) {
			res = MLME_UNSPECIFY_FAIL;
			goto end;
		}

		SAE_BN_FREE(&pSaeIns->sae_rand);
		pSaeIns->sae_rand = sae_gen_rand(pSaeIns);
		SAE_BN_FREE(&mask);
		mask = sae_gen_rand(pSaeIns);

		if (pSaeIns->own_commit_scalar == NULL) {
			SAE_BN_INIT(&pSaeIns->own_commit_scalar);

			if (pSaeIns->own_commit_scalar == NULL) {
				res = MLME_UNSPECIFY_FAIL;
				goto end;
			}
		}

		/* commit-scalar = (rand + mask) modulo r */
		if (mask && pSaeIns->sae_rand)
			SAE_BN_MOD_ADD(pSaeIns->sae_rand, mask, pSaeIns->order, &pSaeIns->own_commit_scalar);
	} while (SAE_BN_IS_ZERO(pSaeIns->own_commit_scalar)
			 || SAE_BN_IS_ONE(pSaeIns->own_commit_scalar));

	log_time_end(LOG_TIME_SAE, "derive_commit_scalar_time", DBG_LVL_INFO, &pSaeIns->sae_tl.derive_commit_scalar_time);
	log_time_begin(LOG_TIME_UNIT_MS, &pSaeIns->sae_tl.derive_commit_element_time);

	if (pSaeIns->group_info == NULL
		|| pSaeIns->group_op == NULL
		|| pSaeIns->group_op->sae_derive_commit_element(pSaeIns, mask) == FALSE)
		res = MLME_UNSPECIFY_FAIL;

	log_time_end(LOG_TIME_SAE, "derive_commit_element_time", DBG_LVL_INFO, &pSaeIns->sae_tl.derive_commit_element_time);
end:
	SAE_BN_FREE(&mask);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[0]);
	return res;
}


USHORT sae_process_commit(
	IN SAE_INSTANCE *pSaeIns)
{
	UCHAR *k = NULL;
	USHORT res = MLME_SUCCESS;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);
	log_time_begin(LOG_TIME_UNIT_MS, &pSaeIns->sae_tl.derive_k_time);

	os_alloc_mem(NULL, (UCHAR **)&k, SAE_MAX_PRIME_LEN);

	if (pSaeIns->group_op
		&& (pSaeIns->group_op->sae_derive_k(pSaeIns, k) == TRUE)) {
		log_time_end(LOG_TIME_SAE, "derive_k_time", DBG_LVL_INFO, &pSaeIns->sae_tl.derive_k_time);
		log_time_begin(LOG_TIME_UNIT_MS, &pSaeIns->sae_tl.derive_pmk_time);

		if (sae_derive_key(pSaeIns, k) == TRUE)
			res = MLME_SUCCESS;
		else
			res = MLME_UNSPECIFY_FAIL;

		log_time_end(LOG_TIME_SAE, "derive_pmk_time", DBG_LVL_INFO, &pSaeIns->sae_tl.derive_pmk_time);
	} else
		res = MLME_UNSPECIFY_FAIL;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			 "%s() <==, res = %d\n", __func__, res);

	os_free_mem(k);
	return res;
}


UCHAR sae_derive_key(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *k)
{
	UCHAR salt[MAX_SAE_KCK_LEN];
	UINT32 salt_len;
	UCHAR keyseed[MAX_SAE_KCK_LEN];
	UCHAR *val = NULL;
	UINT32 val_len = SAE_MAX_PRIME_LEN;
	UCHAR keys[MAX_SAE_KCK_LEN * 2 + LEN_PMK] = {0};
	SAE_BN *tmp = NULL;
	UCHAR res = TRUE;
	UINT32 hash_len;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	if (pSaeIns->connect_type == SAE_CONNECTION_TYPE_LOOPING)
		hash_len = SHA256_DIGEST_SIZE;
	else
		hash_len = sae_ecc_prime_len_2_hash_len(pSaeIns->prime_len);
	/* keyseed = H(salt, k) */
	/* salt is either a series of 0 octets or a list of rejected groups */
	if (pSaeIns->connect_type && pSaeIns->peer_rejected_group_len != 0) {
		NdisCopyMemory(salt, pSaeIns->peer_rejected_group, pSaeIns->peer_rejected_group_len);
		salt_len = pSaeIns->peer_rejected_group_len;
	} else {
		NdisZeroMemory(salt, hash_len);
		salt_len = hash_len;
	}
	hkdf_extract(salt, salt_len, k,
				   pSaeIns->prime_len, keyseed, hash_len);

	hex_dump_with_lvl("keyseed:", (char *)keyseed, hash_len, SAE_DEBUG_LEVEL);

	/* KCK || PMK = KDF-512(keyseed, "SAE KCK and PMK",
	  *                      (commit-scalar + peer-commit-scalar) modulo r)
	  */

	os_alloc_mem(NULL, (UCHAR **)&val, SAE_MAX_PRIME_LEN);
	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[1]);
	GET_BI_INS_FROM_POOL(tmp);
	SAE_BN_INIT(&tmp);
	SAE_BN_MOD_ADD(pSaeIns->own_commit_scalar, pSaeIns->peer_commit_scalar, pSaeIns->order, &tmp);
	SAE_BN_BI2BIN_WITH_PAD(tmp, val, &val_len, pSaeIns->prime_len);
	hex_dump_with_lvl("(commit-scalar + peer-commit-scalar) modulo r:", (char *)val, val_len, SAE_DEBUG_LEVEL);

	if (val_len < pSaeIns->prime_len) {
		SAE_BN_FREE(&tmp);
		res = FALSE;
		goto Free;
	}

	if (pSaeIns->connect_type == SAE_CONNECTION_TYPE_SAEPK
		&& !(pSaeIns->sae_pk_ptr->sae_pk_test_ctrl & SAE_PK_CFG_TAKE_H2E_AS_SAEPK)) {
		sae_kdf_hash(keyseed, hash_len, (UINT8 *)"SAE-PK keys", 11, val, val_len, keys, hash_len * 2 + LEN_PMK);
		NdisCopyMemory(pSaeIns->kek, keys + hash_len + LEN_PMK, hash_len);
	} else
		sae_kdf_hash(keyseed, hash_len, (UINT8 *)"SAE KCK and PMK", 15, val, val_len, keys, hash_len + LEN_PMK);
	NdisCopyMemory(pSaeIns->kck, keys, hash_len);
	NdisCopyMemory(pSaeIns->pmk, keys + hash_len, LEN_PMK);
	hex_dump_with_lvl("kck:", (char *)pSaeIns->kck, hash_len, SAE_DEBUG_LEVEL);
	hex_dump_with_lvl("pmk:", (char *)pSaeIns->pmk, LEN_PMK, SAE_DEBUG_LEVEL);
	hex_dump_with_lvl("kek:", (char *)pSaeIns->kek, hash_len, SAE_DEBUG_LEVEL);
	pSaeIns->kck_kek_len = hash_len;
Free:
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[1]);
	os_free_mem(val);
	return res;
}

VOID sae_send_auth(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	IN UCHAR *bssid,
	IN USHORT alg,
	IN USHORT seq,
	IN USHORT status_code,
	IN UCHAR *buf,
	IN UINT32 buf_len)
{
	HEADER_802_11 AuthHdr;
	ULONG FrameLen = 0;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			 "==> %s(), seq = %d, statuscode = %d, own mac addr = "MACSTR", peer mac addr = "MACSTR"\n",
			  __func__, seq, status_code, MAC2STR(own_mac), MAC2STR(peer_mac));
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	MgtMacHeaderInitExt(pAd, &AuthHdr, SUBTYPE_AUTH, 0, peer_mac,
					 own_mac,
					 bssid);

	MakeOutgoingFrame(pOutBuffer,	&FrameLen,
					  sizeof(HEADER_802_11), &AuthHdr,
					  2,			&alg,
					  2,			&seq,
					  2,			&status_code,
					  buf_len,		buf,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
}

UCHAR sae_send_auth_commit(
	IN RTMP_ADAPTER *pAd,
	IN SAE_INSTANCE *pSaeIns)
{
	UCHAR *buf = NULL;
	UCHAR *pos;
	UINT32 len;
	USHORT status_code = MLME_SUCCESS;
#ifdef RT_BIG_ENDIAN
	USHORT sae_group = 0;
#endif

	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			 "==> %s():\n", __func__);

	if (pSaeIns->own_commit_scalar == NULL)
			return FALSE;

	if (pSaeIns->own_commit_element == NULL)
			return FALSE;

	os_alloc_mem(pAd, &buf, SAE_COMMIT_MAX_LEN);

	if (buf == NULL)
		return FALSE;

	pos = buf;
	if (wpa3_test_ctrl == 4) {
		NdisMoveMemory(pos, commit_msg, commit_msg_len);
		pos += commit_msg_len;

		goto send_msg;
	}
	NdisZeroMemory(pos, SAE_COMMIT_MAX_LEN);

#ifdef RT_BIG_ENDIAN
	sae_group = cpu2le16(pSaeIns->group);
	NdisMoveMemory(pos, &sae_group, 2);
#else
	NdisMoveMemory(pos, &pSaeIns->group, 2);
#endif
	pos += 2;
	/*Looping case consider Anticlogging token to be present after Group ID
	 * Present as Normal Anti-clogging token parameter in commit frame*/
	if (pSaeIns->anti_clogging_token && pSaeIns->connect_type == SAE_CONNECTION_TYPE_LOOPING) {
		NdisMoveMemory(pos, pSaeIns->anti_clogging_token,
			pSaeIns->anti_clogging_token_len);
		pos += pSaeIns->anti_clogging_token_len;
	}

	len = pSaeIns->prime_len;
	if (wpa3_test_ctrl == 3)
		SAE_BN_BI2BIN_WITH_PAD(pSaeIns->peer_commit_scalar, pos, &len, pSaeIns->prime_len);
	else
		SAE_BN_BI2BIN_WITH_PAD(pSaeIns->own_commit_scalar, pos, &len, pSaeIns->prime_len);
	pos += len;

	if (is_sae_group_ecc(pSaeIns->group)) {
		BIG_INTEGER_EC_POINT *element = (BIG_INTEGER_EC_POINT *) pSaeIns->own_commit_element;
		if (wpa3_test_ctrl == 3)
			element = (BIG_INTEGER_EC_POINT *) pSaeIns->peer_commit_element;
		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(element->x, pos, &len, pSaeIns->prime_len);
		pos += len;
		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(element->y, pos, &len, pSaeIns->prime_len);
		pos += len;
	} else {
		SAE_BN *element = (SAE_BN *) pSaeIns->own_commit_element;
		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(element, pos, &len, pSaeIns->prime_len);
		pos += len;
	}

	if (pSaeIns->pwd_id_ptr) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			"%s(): carry pwd id %s, len = %d\n", __func__, pSaeIns->pwd_id_ptr->pwd_id, (INT)strlen(pSaeIns->pwd_id_ptr->pwd_id));
		pos[0] = IE_WLAN_EXTENSION;
		pos[1] = strlen(pSaeIns->pwd_id_ptr->pwd_id) + 1;
		pos[2] = EID_EXT_PASSWORD_IDENTIFIER;
		NdisMoveMemory(pos + 3, pSaeIns->pwd_id_ptr->pwd_id, strlen(pSaeIns->pwd_id_ptr->pwd_id));
		pos += strlen(pSaeIns->pwd_id_ptr->pwd_id) + 3;
	}
	if (pSaeIns->rejected_group) {
		pos[0] = IE_WLAN_EXTENSION;
		pos[1] = pSaeIns->peer_rejected_group_len + 1;
		pos[2] = EID_EXT_REJECTED_GROUP;
		NdisMoveMemory(pos + 3, pSaeIns->peer_rejected_group, pSaeIns->peer_rejected_group_len);
		pos += pSaeIns->peer_rejected_group_len + 3;
	}
	/*H2E Case will contain 3 bytes extra for Header apart from Anti-Clogging token value
	 * Present as Tagged parameter in commit frame*/
	if (pSaeIns->anti_clogging_token && pSaeIns->connect_type == SAE_CONNECTION_TYPE_H2E) {
		NdisMoveMemory(pos, pSaeIns->anti_clogging_token,
			pSaeIns->anti_clogging_token_len);
		pos += pSaeIns->anti_clogging_token_len;
	}

send_msg:
	if (pSaeIns->connect_type == SAE_CONNECTION_TYPE_H2E)
		status_code = MLME_SAE_HASH_TO_ELEMENT;
	else if (pSaeIns->connect_type == SAE_CONNECTION_TYPE_SAEPK)
		status_code =
		(pSaeIns->sae_pk_ptr->sae_pk_test_ctrl &
		(SAE_PK_CFG_TAKE_H2E_AS_SAEPK | SAE_PK_CFG_SEND_H2E_STATUS_CODE)) ?
		MLME_SAE_HASH_TO_ELEMENT : MLME_SAE_PUBLIC_KEY;

	sae_send_auth(pAd, pSaeIns->own_mac, pSaeIns->peer_mac, pSaeIns->bssid,
				  AUTH_MODE_SAE, SAE_COMMIT_SEQ, status_code, buf, pos - buf);

	if (pSaeIns->anti_clogging_token) {
		os_free_mem(pSaeIns->anti_clogging_token);
		pSaeIns->anti_clogging_token = NULL;
		pSaeIns->anti_clogging_token_len = 0;
	}

	os_free_mem(buf);
	return TRUE;
}

UCHAR sae_send_auth_confirm(
	IN RTMP_ADAPTER *pAd,
	IN SAE_INSTANCE *pSaeIns)
{
	UCHAR *buf = NULL;
	UCHAR *pos;
	UCHAR confirm[MAX_SAE_KCK_LEN] = {0};
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);
	os_alloc_mem(pAd, &buf, SAE_CONFIRM_MAX_LEN);
	pos = buf;

	/* 12.4.8.6.4 the Sc counter shall be incremented (thereby setting its value to one), the protocol instance shall then
	  * construct a Confirm Message, transmit it to the peer
	  * 12.4.8.6.5 the protocol instance shall increment Sync,
	  * increment Sc, and transmit its Commit and Confirm (with the new Sc value) messages
	  * => increment send_confirm first and send comfirm msg with new sc value
	  */
	if (pSaeIns->send_confirm != SAE_MAX_SEND_CONFIRM)
		pSaeIns->send_confirm++;

	NdisMoveMemory(pos, &pSaeIns->send_confirm, 2);
	pos += 2;

	if (pSaeIns->group_op)
		pSaeIns->group_op->sae_cn_confirm(pSaeIns, TRUE, confirm);

	NdisMoveMemory(pos, confirm, pSaeIns->kck_kek_len);
	hex_dump_with_lvl("confirm(pos):", (char *)pos, pSaeIns->kck_kek_len, SAE_DEBUG_LEVEL);
	pos += pSaeIns->kck_kek_len;
	if (pSaeIns->connect_type == SAE_CONNECTION_TYPE_SAEPK &&
		pSaeIns->sae_pk_ptr->role == SAE_PK_ROLE_AUTHENICATOR) {
		if (!(pSaeIns->sae_pk_ptr->sae_pk_test_ctrl & SAE_PK_CFG_OMIT_SEND_FILS_PUBLIC_KEY))
			pos += sae_pk_build_fils_public_key(pSaeIns, pos);
		if (!(pSaeIns->sae_pk_ptr->sae_pk_test_ctrl & SAE_PK_CFG_OMIT_SEND_FILS_KEY_CONFIRM))
			pos += sae_pk_build_fils_key_confirmation(pSaeIns, pos);
		if (!(pSaeIns->sae_pk_ptr->sae_pk_test_ctrl & SAE_PK_CFG_OMIT_SEND_SAEPK))
			pos += sae_pk_build_sae_pk_element(pSaeIns, pos);
	}

	sae_send_auth(pAd, pSaeIns->own_mac, pSaeIns->peer_mac, pSaeIns->bssid,
				  AUTH_MODE_SAE, SAE_CONFIRM_SEQ, MLME_SUCCESS, buf, pos - buf);
	os_free_mem(buf);
	return TRUE;
}

USHORT sae_parse_confirm(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *msg,
	IN UINT32 msg_len)
{
	UCHAR peer_confirm[MAX_SAE_KCK_LEN];
	FRAME_802_11 *Fr = (PFRAME_802_11)msg;
	UCHAR *pos = &Fr->Octet[6];
	UCHAR *end = msg + msg_len;
	USHORT peer_send_confirm;
	USHORT res = MLME_SUCCESS;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG, "==>\n");

	/* peer-send-confirm */
	if (end - pos < 2)
		return MLME_UNSPECIFY_FAIL;

	NdisMoveMemory(&peer_send_confirm, pos, 2);
	pos = pos + 2;

	/*  12.4.8.6.6 Upon receipt of a Con event, the value of send-confirm shall be checked.
	  * If the value is not greater than Rc or is equal to 2^16 ¡V 1, the received frame shall be silently discarded
	  */
	if (pSaeIns->state == SAE_ACCEPTED
		&& (peer_send_confirm <= pSaeIns->last_peer_sc
			|| peer_send_confirm == SAE_MAX_SEND_CONFIRM)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_WARN,
			"confirm fail: SAE_SILENTLY_DISCARDED due to  peer_send_confirm =%d, ast_peer_sc = %d\n",
			peer_send_confirm, pSaeIns->last_peer_sc);
		pSaeIns->peer_send_confirm = peer_send_confirm;
		/* return SAE_SILENTLY_DISCARDED; */
	} else
		pSaeIns->peer_send_confirm = peer_send_confirm;

	/* send-confirm */
	if (end - pos < pSaeIns->kck_kek_len)
		return MLME_UNSPECIFY_FAIL;

	NdisMoveMemory(peer_confirm, pos, pSaeIns->kck_kek_len);
	pos += pSaeIns->kck_kek_len;

	if (pSaeIns->connect_type == SAE_CONNECTION_TYPE_SAEPK && pSaeIns->sae_pk_ptr->role == SAE_PK_ROLE_SUPPLICANT)
		res = sae_pk_parse_element(pSaeIns, pos, end);

	if (res != MLME_SUCCESS)
		return res;

	return sae_check_confirm(pSaeIns, peer_confirm);
}


USHORT sae_check_confirm(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *peer_confirm)
{
	UCHAR verifier[MAX_SAE_KCK_LEN];
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	if (pSaeIns->peer_commit_element == NULL
		|| pSaeIns->peer_commit_scalar == NULL
		|| pSaeIns->own_commit_element == NULL
		|| pSaeIns->own_commit_scalar == NULL)
		return MLME_UNSPECIFY_FAIL;

	if (pSaeIns->group_op)
		pSaeIns->group_op->sae_cn_confirm(pSaeIns, FALSE, verifier);
	else
		return MLME_UNSPECIFY_FAIL;

	if (!RTMPEqualMemory(peer_confirm, verifier, pSaeIns->kck_kek_len)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
				 "peer_send_confirm = %d\n", pSaeIns->peer_send_confirm);
		hex_dump_with_lvl("peer_confirm:", (char *)peer_confirm, pSaeIns->kck_kek_len, SAE_DEBUG_LEVEL2);
		hex_dump_with_lvl("verifier:", (char *)verifier, pSaeIns->kck_kek_len, SAE_DEBUG_LEVEL2);
		return MLME_UNSPECIFY_FAIL;
	}

	if (pSaeIns->connect_type == SAE_CONNECTION_TYPE_SAEPK && pSaeIns->sae_pk_ptr->role == SAE_PK_ROLE_SUPPLICANT)
		return sae_pk_check_signature(pSaeIns);
	else
		return MLME_SUCCESS;
}


SAE_BN *sae_gen_rand(
	IN SAE_INSTANCE *pSaeIns)
{
	UINT8 *rand = NULL;
	UINT32 i;
	SAE_BN *rand_bi = NULL;
	UINT32 iter = 0;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	if (!pSaeIns->order)
		return NULL;

	os_alloc_mem(NULL, (UCHAR **)&rand, SAE_MAX_PRIME_LEN);

	for (iter = 0; iter < 100; iter++) {
		for (i = 0; i < pSaeIns->order_len; i++)
			rand[i] = RandomByte(pSaeIns->pParentSaeCfg->pAd);

		/* group 21 ec_group21_order[0] = 0x01 */
		if (is_sae_group_ecc(pSaeIns->group)) {
			EC_GROUP_INFO *group_info = pSaeIns->group_info;

			if (group_info->prime_len_bit % 8)
				rand[0] &= BITS(0, group_info->prime_len_bit % 8 - 1);
		}

		hex_dump_with_lvl("rand:", (char *)rand, pSaeIns->order_len, SAE_DEBUG_LEVEL);
		SAE_BN_BIN2BI(rand, pSaeIns->order_len, &rand_bi);

		if (SAE_BN_IS_ZERO(rand_bi)
			|| SAE_BN_IS_ONE(rand_bi)
			|| SAE_BN_UCMP(rand_bi, pSaeIns->order) >= 0)
			continue;
		else {
			os_free_mem(rand);
			return rand_bi;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 "gen rand fail\n");
	SAE_BN_FREE(&rand_bi);
	os_free_mem(rand);
	return NULL;
}

USHORT sae_group_allowed(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *allowed_groups,
	IN INT32 group)
{
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			"==> %s():\n", __func__);

	if (allowed_groups) {
		UINT32 i;

		if (wpa3_test_ctrl == 5) {
			for (i = 0; allowed_groups[i] > 0; i++)
				allowed_groups[i] = fixed_group_id;
		}

		for (i = 0; allowed_groups[i] > 0; i++)
			if (allowed_groups[i] == group)
				break;

		if (allowed_groups[i] != group) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
					"unsupport group id = %d\n", group);
			return MLME_FINITE_CYCLIC_GROUP_NOT_SUPPORTED;
		}
	}

	if (pSaeIns->group != group) {
		/* 12.4.8.6.5 the protocol instance shall verify that the finite cyclic group is the same as the previously received Commit frame.
		  * If not, the frame shall be silently discarded.
		  */
		if (pSaeIns->state == SAE_CONFIRMED)
			return SAE_SILENTLY_DISCARDED;

		sae_clear_data(pSaeIns);

		if (is_sae_group_ecc(group))
			pSaeIns->group_op = &ecc_group_op;
		else if (is_sae_group_ffc(group))
			pSaeIns->group_op = &ffc_group_op;
		else
			return MLME_FINITE_CYCLIC_GROUP_NOT_SUPPORTED;

		pSaeIns->group_op->sae_group_init(pSaeIns, group); /* ellis exception */

		if (pSaeIns->group_info == NULL
			|| pSaeIns->group_op == NULL)
			return MLME_UNSPECIFY_FAIL;
	}

	return MLME_SUCCESS;
}
/***************/
/* h2e related api*/
/***************/
VOID *sae_search_pt_by_group(
	IN struct sae_pt *pt_list,
	IN USHORT group,
	IN struct pwd_id_list *pwd_id_list)
{
	struct sae_pt *list = pt_list;
	UCHAR *pwd_id = NULL;

	if (pwd_id_list)
		pwd_id = pwd_id_list->pwd_id;

	while (list) {
		if (list->group == group && list->pwd_id == pwd_id) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
				 "%s(): search done, group=%d, pwd_id = %s\n",
				  __func__, list->group, list->pwd_id);
			return list->pt;
		}
		list = list->next;
	}

	return NULL;
}

static BIG_INTEGER_EC_POINT *sswu(
	IN EC_GROUP_INFO_BI *ec_group_bi,
	IN SAE_BN *u)
{
	SAE_BN *m = NULL;
	SAE_BN *u2 = NULL;
	SAE_BN *tmp = NULL;
	SAE_BN *tmp2 = NULL;
	SAE_BN *tmp3 = NULL;
	SAE_BN *one_bn = NULL;
	SAE_BN *two_bn = NULL;
	SAE_BN *t = NULL;
	SAE_BN *csel_y = NULL;
	SAE_BN *csel_z = NULL;
	SAE_BN *x1 = NULL;
	SAE_BN *x2 = NULL;
	SAE_BN *y = NULL;
	UCHAR one[] = {1};
	UCHAR two[] = {2};
	UCHAR has_y, has_y2;
	UCHAR lsb_u, lsb_y;
	BIG_INTEGER_EC_POINT *res = NULL;

	ecc_point_init(&res);

	/* m = z^2 * u^4 + z * u^2 = (z * u ^ 2) ^ 2 + (z * u ^ 2) */
	SAE_BN_MOD_SQR(u, ec_group_bi->prime, &u2);
	SAE_BN_MOD_MUL(u2, ec_group_bi->z, ec_group_bi->prime, &tmp2);
	SAE_BN_MOD_SQR(tmp2, ec_group_bi->prime, &tmp);
	SAE_BN_MOD_ADD(tmp, tmp2, ec_group_bi->prime, &m);

	/* l = CEQ(m, 0)
	  * t = inverse(m), where inverse(x) is calculated as x^(p-2) modulo p
	  * x1 = CSEL(l, (b / (z * a) modulo p), ((-b/a) * (1 + t)) modulo p)
	  * where CSEL(x,y,z) operates in constant time and returns y if x is true and z otherwise.
	  */
	/* t = inverse(m) */
	SAE_BN_BIN2BI(two, sizeof(two), &two_bn);
	SAE_BN_SUB(ec_group_bi->prime, two_bn, &tmp);
	SAE_BN_MOD_EXP_MONT(m, tmp, ec_group_bi->prime, &t);
	/* csel_y = b / (z * a) modulo p */
	SAE_BN_MOD_MUL(ec_group_bi->z, ec_group_bi->a, ec_group_bi->prime, &tmp);
	SAE_BN_MOD_MUL_INV(tmp, ec_group_bi->prime, &tmp2);
	SAE_BN_MOD_MUL(ec_group_bi->b, tmp2, ec_group_bi->prime, &csel_y);
	/* csel_z = (-b/a) * (1 + t) modulo p */
	SAE_BN_BIN2BI(one, sizeof(one), &one_bn);
	SAE_BN_SUB(ec_group_bi->prime, ec_group_bi->b, &tmp);
	SAE_BN_MOD_MUL_INV(ec_group_bi->a, ec_group_bi->prime, &tmp2);
	SAE_BN_MOD_MUL(tmp, tmp2, ec_group_bi->prime, &tmp3);
	SAE_BN_MOD_ADD(t, one_bn, ec_group_bi->prime, &tmp);
	SAE_BN_MOD_MUL(tmp3, tmp, ec_group_bi->prime, &csel_z);
	/* x1 = CSEL(l, (b / (z * a) modulo p), ((-b/a) * (1 + t)) modulo p) */
	if (SAE_BN_IS_ZERO(m)) {
		x1 = csel_y;
		csel_y = NULL;
	} else {
		x1 = csel_z;
		csel_z = NULL;
	}

	/* x2 = (z * u^2 * x1) modulo p */
	SAE_BN_MOD_MUL(ec_group_bi->z, u2, ec_group_bi->prime, &tmp);
	SAE_BN_MOD_MUL(tmp, x1, ec_group_bi->prime, &x2);

	/* l = gx1 is a quadratic residue modulo p
	 * gx1 = (x1^3 + a * x1 + b) modulo p
	 * gx2 = (x2^3 + a * x2 + b) modulo p
	 * v = CSEL(l, gx1, gx2)
	 * x = CSEL(l, x1, x2)
	 */
	has_y = ecc_point_find_by_x(ec_group_bi, x1, &y, TRUE);
	has_y2 = ecc_point_find_by_x(ec_group_bi, x2, &y, !has_y);

	if (!has_y && !has_y2) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, " can not find y\n");
		ecc_point_free(&res);
		goto free;
	} else if (has_y) {
		res->x = x1;
		x1 = NULL;
	} else {
		res->x = x2;
		x2 = NULL;
	}

	/* l = CEQ(LSB(u), LSB(y))
	 * P = CSEL(l, (x,y), (x, p-y))
	 * output P
	 */
	lsb_u = SAE_BN_IS_ODD(u);
	lsb_y = SAE_BN_IS_ODD(y);
	SAE_BN_SUB(ec_group_bi->prime, y, &tmp);
	if (lsb_u == lsb_y) {
		res->y = y;
		y = NULL;
	} else {
		res->y = tmp;
		tmp = NULL;
	}
	SAE_ECC_SET_Z_TO_1(res);
free:
	SAE_BN_FREE(&m);
	SAE_BN_FREE(&u2);
	SAE_BN_FREE(&tmp);
	SAE_BN_FREE(&tmp2);
	SAE_BN_FREE(&tmp3);
	SAE_BN_FREE(&one_bn);
	SAE_BN_FREE(&two_bn);
	SAE_BN_FREE(&t);
	SAE_BN_FREE(&csel_y);
	SAE_BN_FREE(&csel_z);
	SAE_BN_FREE(&x1);
	SAE_BN_FREE(&x2);
	SAE_BN_FREE(&y);

	return res;
}

/* todo: identifier case and HKDF-Extract/HKDF-Expand for group 20 */
static BIG_INTEGER_EC_POINT *sae_derive_pt_ecc(
	IN USHORT group,
	IN UCHAR *psk,
	IN CHAR * ssid,
	IN UCHAR ssid_len,
	IN UCHAR *identifier)
{
	EC_GROUP_INFO *ec_group = NULL;
	EC_GROUP_INFO_BI *ec_group_bi = NULL;
	UINT32 pwd_value_len, hash_len;
	UCHAR msg[LEN_PSK + SAE_MAX_PWD_ID] = {0};
	UINT32 msg_len = 0;
	UCHAR pwd_seed[64];
	UCHAR pwd_value[SAE_MAX_ECC_PRIME_LEN * 2];
	SAE_BN *pwd_v = NULL;
	SAE_BN *u = NULL;
	BIG_INTEGER_EC_POINT *p1 = NULL;
	BIG_INTEGER_EC_POINT *p2 = NULL;
	BIG_INTEGER_EC_POINT *res = NULL;

	ec_group = get_ecc_group_info(group);
	ec_group_bi = get_ecc_group_info_bi(group);

	/* len = olen(p) + ceil(olen(p)/2) */
	pwd_value_len = ec_group->prime_len + (ec_group->prime_len + 1) / 2;

	hash_len = sae_ecc_prime_len_2_hash_len(ec_group->prime_len);

	/* pwd-seed = HKDF-Extract(ssid, password [ || identifier ]) */
	if (strlen(psk) <= sizeof(msg)) {
		NdisMoveMemory(msg, psk, strlen(psk));
		msg_len += strlen(psk);
	}
	if (identifier != NULL) {
		if ((strlen(identifier) <= (sizeof(msg) - msg_len)) && (msg_len < sizeof(msg))) {
			NdisMoveMemory(msg + msg_len, identifier, strlen(identifier));
			msg_len += strlen(identifier);
		}
	}
	hkdf_extract(ssid, ssid_len, msg, msg_len, pwd_seed, hash_len);

	/* pwd-value = HKDF-Expand(pwd-seed, "SAE Hash to Element u1 P1", len) */
	hkdf_expand(pwd_seed, hash_len, "SAE Hash to Element u1 P1",
				25, pwd_value, pwd_value_len);

	/* u1 = pwd-value modulo p */
	SAE_BN_BIN2BI(pwd_value, pwd_value_len, &pwd_v);
	SAE_BN_MOD(pwd_v, ec_group_bi->prime, &u);

	/* P1 = SSWU(u1) */
	p1 = sswu(ec_group_bi, u);
	if (p1 == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, " gen p1 fail\n");
		goto free;
	}

	/* pwd-value = HKDF-Expand(pwd-seed, "SAE Hash to Element u2 P2", len) */
	hkdf_expand(pwd_seed, hash_len, "SAE Hash to Element u2 P2",
				25, pwd_value, pwd_value_len);

	/* u2 = pwd-value modulo p */
	SAE_BN_BIN2BI(pwd_value, pwd_value_len, &pwd_v);
	SAE_BN_MOD(pwd_v, ec_group_bi->prime, &u);

	/* P2 = SSWU(u2) */
	p2 = sswu(ec_group_bi, u);
	if (p2 == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, "gen p2 fail\n");
		goto free;
	}

	/* PT = elem-op(P1, P2) */
	ecc_point_add(p1, p2, ec_group_bi, &res);
free:
	SAE_BN_FREE(&pwd_v);
	SAE_BN_FREE(&u);
	ecc_point_free(&p1);
	ecc_point_free(&p2);

	return res;
}

static VOID *sae_derive_pt_group(
	IN USHORT group,
	IN UCHAR *psk,
	IN CHAR * ssid,
	IN UCHAR ssid_len,
	IN UCHAR *identifier)
{
	if (is_sae_group_ecc(group))
		return (VOID *)sae_derive_pt_ecc(group, psk, ssid, ssid_len, identifier);
	else /* ffc not support */
		return NULL;
}

VOID sae_derive_pt(
	struct wifi_dev *wdev,
	IN SAE_CFG *pSaeCfg,
	IN UCHAR *psk,
	IN CHAR * ssid,
	IN UCHAR ssid_len,
	IN struct pwd_id_list *pwd_id_list_head,
	OUT struct sae_pt **pt_list)
{
	struct sae_pt *tmp, *tmp2;
	UCHAR i;
	UCHAR *allowed_groups = pSaeCfg->support_group;

	if (*pt_list != NULL) {
		sae_pt_list_deinit(wdev, pt_list);
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			"%s(): pt_list is not null, reinit pt_list\n", __func__);
	}
	os_alloc_mem(NULL, (UCHAR **)pt_list, sizeof(struct sae_pt));
	(*pt_list)->next = NULL;

	tmp = *pt_list;
	tmp2 = *pt_list;

	for (i = 0; allowed_groups[i] > 0; i++) {
		if (tmp2 == NULL) {
			os_alloc_mem(NULL, (UCHAR **)&tmp2, sizeof(struct sae_pt));
			tmp2->next = NULL;
			tmp->next = tmp2;
			tmp = tmp2;
		}
		tmp->group = allowed_groups[i];
		tmp->pt = (VOID *)sae_derive_pt_group(allowed_groups[i], psk, ssid, ssid_len, NULL);
		tmp->pwd_id = NULL;
		tmp2 = tmp->next;
	}

	if (pwd_id_list_head && !DlListEmpty(&pwd_id_list_head->list)) {
		struct pwd_id_list *list = NULL;

		DlListForEach(list, &pwd_id_list_head->list, struct pwd_id_list, list) {
			for (i = 0; allowed_groups[i] > 0; i++) {
				if (tmp2 == NULL) {
					os_alloc_mem(NULL, (UCHAR **)&tmp2, sizeof(struct sae_pt));
					tmp2->next = NULL;
					tmp->next = tmp2;
					tmp = tmp2;
				}
				tmp->group = allowed_groups[i];
				tmp->pt = (VOID *)sae_derive_pt_group(allowed_groups[i], list->pwd, ssid, ssid_len, list->pwd_id);
				tmp->pwd_id = list->pwd_id;
				tmp2 = tmp->next;
			}
		}
	}
}

VOID sae_pt_list_deinit(
	IN struct wifi_dev *wdev,
	INOUT struct sae_pt **pt_list)
{
	struct sae_pt *tmp;
	struct sae_pt *tmp2 = *pt_list;
	BIG_INTEGER_EC_POINT *point = NULL;

	if (wdev == NULL)
		return;

	NdisAcquireSpinLock(&wdev->SecConfig.ptlist_lock);
	if (*pt_list == NULL) {
		NdisReleaseSpinLock(&wdev->SecConfig.ptlist_lock);
		return;
	}
	*pt_list = NULL;
	NdisReleaseSpinLock(&wdev->SecConfig.ptlist_lock);

	do {
		tmp = tmp2;
		tmp2 = tmp->next;
		point = (BIG_INTEGER_EC_POINT *)tmp->pt;
		ecc_point_free(&point);
		os_free_mem(tmp);
	} while (tmp2);
}
/* h2e related api end */

/******************/
/* sae-pk related api */
/******************/
static UINT32 sae_pk_derive_key_auth(
	IN SAE_INSTANCE *pSaeIns,
	OUT UCHAR *key_auth)
{
	struct sae_pk_cfg *sae_pk = pSaeIns->sae_pk_ptr;
	EC_GROUP_INFO_BI *ec_group_bi = (EC_GROUP_INFO_BI *) sae_pk->group_info_bi;
	UCHAR *msg;
	UINT32 msg_len = 0;
	UINT32 len;
	UINT32 hash_len;

	os_alloc_mem(NULL, (UCHAR **) &msg,
		pSaeIns->prime_len * 6 + SAE_PK_MODIFIER_BYTES_LEN + sae_pk->asn1_sub_pub_key_info_len + MAC_ADDR_LEN * 2);

	/* KeyAuth = Sig_AP(eleAP || eleSTA || scaAP || scaSTA || M || K_AP || AP-BSSID || STA-MAC) */
	if (is_sae_group_ecc(pSaeIns->group)) {
		BIG_INTEGER_EC_POINT *element = NULL;

		if (sae_pk->role == SAE_PK_ROLE_AUTHENICATOR)
			element = (BIG_INTEGER_EC_POINT *) pSaeIns->own_commit_element;
		else
			element = (BIG_INTEGER_EC_POINT *) pSaeIns->peer_commit_element;

		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(element->x, msg, &len, pSaeIns->prime_len);
		msg_len += len;
		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(element->y, msg + msg_len, &len, pSaeIns->prime_len);
		msg_len += len;

		if (sae_pk->role == SAE_PK_ROLE_AUTHENICATOR)
			element = (BIG_INTEGER_EC_POINT *) pSaeIns->peer_commit_element;
		else
			element = (BIG_INTEGER_EC_POINT *) pSaeIns->own_commit_element;

		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(element->x, msg + msg_len, &len, pSaeIns->prime_len);
		msg_len += len;
		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(element->y, msg + msg_len, &len, pSaeIns->prime_len);
		msg_len += len;
	} else {
		SAE_BN *element = NULL;

		if (sae_pk->role == SAE_PK_ROLE_AUTHENICATOR)
			element = (SAE_BN *) pSaeIns->own_commit_element;
		else
			element = (SAE_BN *) pSaeIns->peer_commit_element;

		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(element, msg, &len, pSaeIns->prime_len);
		msg_len += len;
		if (sae_pk->role == SAE_PK_ROLE_AUTHENICATOR)
			element = (SAE_BN *) pSaeIns->peer_commit_element;
		else
			element = (SAE_BN *) pSaeIns->own_commit_element;
		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(element, msg + msg_len, &len, pSaeIns->prime_len);
		msg_len += len;
	}

	if (sae_pk->role == SAE_PK_ROLE_AUTHENICATOR) {
		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(pSaeIns->own_commit_scalar, msg + msg_len, &len, pSaeIns->prime_len);
		msg_len += len;
		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(pSaeIns->peer_commit_scalar, msg + msg_len, &len, pSaeIns->prime_len);
		msg_len += len;
	} else {
		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(pSaeIns->peer_commit_scalar, msg + msg_len, &len, pSaeIns->prime_len);
		msg_len += len;
		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(pSaeIns->own_commit_scalar, msg + msg_len, &len, pSaeIns->prime_len);
		msg_len += len;
	}

	os_move_mem(msg + msg_len, sae_pk->modifier, SAE_PK_MODIFIER_BYTES_LEN);
	msg_len += SAE_PK_MODIFIER_BYTES_LEN;
	os_move_mem(msg + msg_len, sae_pk->asn1_sub_pub_key_info, sae_pk->asn1_sub_pub_key_info_len);
	msg_len += sae_pk->asn1_sub_pub_key_info_len;
	os_move_mem(msg + msg_len, pSaeIns->bssid, MAC_ADDR_LEN);
	msg_len += MAC_ADDR_LEN;
	if (sae_pk->role == SAE_PK_ROLE_AUTHENICATOR)
		os_move_mem(msg + msg_len, pSaeIns->peer_mac, MAC_ADDR_LEN);
	else
		os_move_mem(msg + msg_len, pSaeIns->own_mac, MAC_ADDR_LEN);
	msg_len += MAC_ADDR_LEN;

	/* key_auth is the leftmost bits of Hash(msg) which the bit length is equal to the group order n */
	if (ec_group_bi->group_id == 19)
		RT_SHA256(msg, msg_len, key_auth);
	else if (ec_group_bi->group_id == 20)
		RT_SHA384(msg, msg_len, key_auth);
	else if (ec_group_bi->group_id == 21)
		RT_SHA512(msg, msg_len, key_auth);
	hash_len = sae_ecc_prime_len_2_hash_len(ec_group_bi->ec_group->prime_len);

	os_free_mem(msg);

	return hash_len;
}


static UINT32 sae_pk_derive_signature(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *buf)
{
	struct sae_pk_cfg *sae_pk = pSaeIns->sae_pk_ptr;
	EC_GROUP_INFO_BI *ec_group_bi = (EC_GROUP_INFO_BI *) sae_pk->group_info_bi;
	UCHAR z[SHA512_DIGEST_SIZE];
	SAE_BN *k = NULL;
	BIG_INTEGER_EC_POINT *point = NULL;
	SAE_BN *tmp = NULL;
	SAE_BN *tmp2 = NULL;
	SAE_BN *z_bn = NULL;
	SAE_BN *r = NULL;
	SAE_BN *s = NULL;
	UINT32 key_auth_len = 0;
	UINT32 hash_len;
	SAE_BN *pri_key = sae_pk->pri_key;

	/* KeyAuth = Sig_AP(eleAP || eleSTA || scaAP || scaSTA || M || K_AP || AP-BSSID || STA-MAC) */
	hash_len = sae_pk_derive_key_auth(pSaeIns, z);

	if (sae_pk->sae_pk_test_ctrl & SAE_PK_CFG_OVERWRITE_PRI_KEY) {
		pri_key = NULL;
		SAE_BN_BIN2BI(private_key_override, ec_group_bi->ec_group->prime_len, &pri_key);
	}

	do {
		/* r = x1 mod n where (x1, y1) = k x G */
		do {
			if (k)
				SAE_BN_FREE(&k);
			k = sae_gen_rand(pSaeIns);
			ECC_POINT_MUL(ec_group_bi->generator, k,  ec_group_bi, &point);
			SAE_ECC_3D_to_2D(ec_group_bi, point);
			SAE_BN_MOD(point->x, ec_group_bi->order, &r);
		} while (SAE_BN_IS_ZERO(r));

		/* s = k^-1(z + r * pri_key) mod n */
		SAE_BN_MOD_MUL(pri_key, point->x, ec_group_bi->order, &tmp);
		SAE_BN_BIN2BI(z, hash_len, &z_bn);
		SAE_BN_MOD_ADD(tmp, z_bn,  ec_group_bi->order, &tmp2);
		SAE_BN_MOD_MUL_INV(k, ec_group_bi->order, &tmp);
		SAE_BN_MOD_MUL(tmp, tmp2, ec_group_bi->order, &s);
	} while (SAE_BN_IS_ZERO(s));

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL, ("r\n"));
	SAE_BN_PRINT_W_CAT_AND_LVL(r, DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL);
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL, ("s\n"));
	SAE_BN_PRINT_W_CAT_AND_LVL(s, DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL);
	asn1_get_ecdsa_sig_value(ec_group_bi, r, s, buf, &key_auth_len);

	/* copy to pSaeIns->key_auth for retry case */
	os_alloc_mem(NULL, (UCHAR **) &pSaeIns->key_auth, key_auth_len);
	os_move_mem(pSaeIns->key_auth, buf, key_auth_len);
	pSaeIns->key_auth_len = key_auth_len;

	SAE_BN_FREE(&tmp);
	SAE_BN_FREE(&tmp2);
	SAE_BN_FREE(&z_bn);
	SAE_BN_FREE(&k);
	SAE_BN_FREE(&r);
	SAE_BN_FREE(&s);
	if (sae_pk->sae_pk_test_ctrl & SAE_PK_CFG_OVERWRITE_PRI_KEY)
		SAE_BN_FREE(&pri_key);
	ecc_point_free(&point);

	return key_auth_len;
}

/* todo: ffc */
UCHAR sae_pk_check_pub_key_info(
	IN SAE_INSTANCE *pSaeIns)
{
	UCHAR finger_print[SAE_PK_MAX_FINGER_PRINT_BYTES_LEN];
	struct sae_pk_cfg *sae_pk = pSaeIns->sae_pk_ptr;
	UINT32 fp_len_bit = SAE_PK_FINGER_PRINT_BITS_LEN(sae_pk->sec, sae_pk->lambda);
	UINT32 fp_len = ((fp_len_bit + 7) / 8);

	sae_pk_deri_finger_print(sae_pk, sae_pk->ssid, sae_pk->ssid_len, FALSE, finger_print);

	if (!NdisEqualMemory(finger_print, sae_pk->finger_print, fp_len)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, " public key verify fail\n");
		hex_dump_with_cat_and_lvl("expected finger_print", sae_pk->finger_print, fp_len,
					DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR);
		hex_dump_with_cat_and_lvl("finger_print derived from public key info", sae_pk->finger_print, fp_len,
					DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR);
		return FALSE;
	}
	return TRUE;
}

/* todo: ffc */
USHORT sae_pk_check_signature(
	IN SAE_INSTANCE *pSaeIns)
{
	USHORT res = MLME_SUCCESS;
	struct sae_pk_cfg *sae_pk = pSaeIns->sae_pk_ptr;
	SAE_BN *u1 = NULL;
	SAE_BN *u2 = NULL;
	SAE_BN *r = sae_pk->sig_r;
	SAE_BN *s = sae_pk->sig_s;
	SAE_BN *s_inv = NULL;
	BIG_INTEGER_EC_POINT *point = NULL;
	BIG_INTEGER_EC_POINT *point2 = NULL;
	BIG_INTEGER_EC_POINT *point3 = NULL;
	BIG_INTEGER_EC_POINT *public_key = (BIG_INTEGER_EC_POINT *) sae_pk->pub_key;
	EC_GROUP_INFO_BI *ec_group_bi = (EC_GROUP_INFO_BI *) sae_pk->group_info_bi;
	UINT32 hash_len;
	UCHAR z[SHA512_DIGEST_SIZE];
	SAE_BN *z_bn = NULL;

	if (!sae_pk_check_pub_key_info(pSaeIns))
		return SAE_SILENTLY_DISCARDED;

	/* KeyAuth = Sig_AP(eleAP || eleSTA || scaAP || scaSTA || M || K_AP || AP-BSSID || STA-MAC) */
	hash_len = sae_pk_derive_key_auth(pSaeIns, z);

	SAE_BN_BIN2BI(z, hash_len, &z_bn);

	/* u1 = zs^-1 mod n, u2 = rs^-1 mod n */
	SAE_BN_MOD_MUL_INV(s, ec_group_bi->order, &s_inv);
	SAE_BN_MOD_MUL(z_bn, s_inv, ec_group_bi->order, &u1);
	SAE_BN_MOD_MUL(r, s_inv, ec_group_bi->order, &u2);

	/* (x1, y1) = u1 x G + u2 x QA */
	ECC_POINT_MUL(ec_group_bi->generator, u1,  ec_group_bi, &point2);
	ECC_POINT_MUL(public_key, u2,  ec_group_bi, &point3);
	ecc_point_add(point2, point3, ec_group_bi, &point);
	ecc_point_3d_to_2d(ec_group_bi, point);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL, ("x1\n"));
	if (point)
		SAE_BN_PRINT_W_CAT_AND_LVL(point->x, DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL);
	else {
		res = SAE_SILENTLY_DISCARDED;
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			"point is NULL, unexpected error !!\n");
		goto error;
	}
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL, ("r\n"));
	SAE_BN_PRINT_W_CAT_AND_LVL(r, DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL, ("s\n"));
	SAE_BN_PRINT_W_CAT_AND_LVL(s, DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL);

	/* The signature is valid if r = x1 mod n */
	if (SAE_BN_UCMP(point->x, r))
		res = SAE_SILENTLY_DISCARDED;

error:
	SAE_BN_FREE(&z_bn);
	SAE_BN_FREE(&u1);
	SAE_BN_FREE(&u2);
	SAE_BN_FREE(&s_inv);
	ecc_point_free(&point);
	ecc_point_free(&point2);
	ecc_point_free(&point3);
	return res;
}


extern const UCHAR wfa_oui[3];
UCHAR is_sae_pk_element(
	IN UCHAR *pos,
	IN UCHAR *end,
	OUT UINT32 *len)
{
	KDE_HDR *kde = (KDE_HDR *)pos;

	if (end - pos < kde->Len + 2)
		return FALSE;

	if (kde->Type != IE_VENDOR_SPECIFIC ||
		kde->DataType != WFA_KDE_SAE_PK ||
		!NdisEqualMemory(kde->OUI, wfa_oui, sizeof(wfa_oui)))
		return FALSE;

	return TRUE;
}

UCHAR is_sae_fils_public_key_element(
	IN UCHAR *pos,
	IN UCHAR *end,
	OUT UINT32 *len)
{
	if (!(end - pos < 3) &&
		pos[0] == IE_WLAN_EXTENSION &&
		pos[1] >= 1 &&
		end - pos - 2 >= pos[1] &&
		pos[2] == EID_EXT_FILS_PUBLIC_KEY) {
		if (len)
			*len = pos[1] - 1;
		return TRUE;
	} else
		return FALSE;
}

UCHAR is_sae_fils_key_confirmation_element(
	IN UCHAR *pos,
	IN UCHAR *end,
	OUT UINT32 *len)
{
	if (!(end - pos < 3) &&
		pos[0] == IE_WLAN_EXTENSION &&
		pos[1] >= 1 &&
		end - pos - 2 >= pos[1] &&
		pos[2] == EID_EXT_FILS_KEY_CONFIRMATION) {
		if (len)
			*len = pos[1] - 1;
		return TRUE;
	} else
		return FALSE;
}


/* todo: ffc */
USHORT sae_pk_parse_element(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *pos,
	IN UCHAR *end)
{
	UCHAR *cur_pos = pos;
	struct sae_pk_cfg *sae_pk = pSaeIns->sae_pk_ptr;
	UINT32 len;

	/* parse FILS Public key element */
	if (!is_sae_fils_public_key_element(cur_pos, end, &len))
		return SAE_SILENTLY_DISCARDED;

	if (asn1_get_pub_key_from_sub_pub_key_info(
		cur_pos + 4, len - 1, &sae_pk->group_id, &sae_pk->group_info_bi,
		(BIG_INTEGER_EC_POINT **) &sae_pk->pub_key) == FALSE)
		return SAE_SILENTLY_DISCARDED;

	if (is_sae_group_ecc(sae_pk->group_id) && cur_pos[3] != FILS_PUBLIC_KEY_TYPE_RFC_5480) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			"%s: key type error(%d)\n", __func__, cur_pos[3]);
		return SAE_SILENTLY_DISCARDED;
	} else if (is_sae_group_ffc(sae_pk->group_id) && cur_pos[3] != FILS_PUBLIC_KEY_TYPE_RFC_3279) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			"%s: key type error(%d)\n", __func__, cur_pos[3]);
		return SAE_SILENTLY_DISCARDED;
	}

	sae_pk->asn1_sub_pub_key_info_len = len - 1;
	if (sae_pk->asn1_sub_pub_key_info != NULL)
		os_free_mem(sae_pk->asn1_sub_pub_key_info);
	os_alloc_mem(NULL, &sae_pk->asn1_sub_pub_key_info, sae_pk->asn1_sub_pub_key_info_len);
	os_move_mem(sae_pk->asn1_sub_pub_key_info, cur_pos + 4, sae_pk->asn1_sub_pub_key_info_len);

	cur_pos += (len + 3);

	/* parse FILS key confirmation element */
	if (!is_sae_fils_key_confirmation_element(cur_pos, end, &len))
		return SAE_SILENTLY_DISCARDED;

	if (asn1_get_sig_from_ecdsa_sig_value(
		sae_pk->group_info_bi, cur_pos + 3, len,
		&sae_pk->sig_r, &sae_pk->sig_s) == FALSE)
		return SAE_SILENTLY_DISCARDED;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL, ("sig_r\n"));
	SAE_BN_PRINT_W_CAT_AND_LVL(sae_pk->sig_r, DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL);
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL, ("sig_s\n"));
	SAE_BN_PRINT_W_CAT_AND_LVL(sae_pk->sig_s, DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL);

	cur_pos += (len + 3);

	/* parse sae pk element */
	if (!is_sae_pk_element(cur_pos, end, NULL))
		return SAE_SILENTLY_DISCARDED;

	cur_pos += 6;

	if (end - cur_pos < AES_BLOCK_SIZES + SAE_PK_MODIFIER_BYTES_LEN)
		return SAE_SILENTLY_DISCARDED;

	if (sae_pk->modifier == NULL)
		os_alloc_mem(NULL, &sae_pk->modifier, SAE_PK_MODIFIER_BYTES_LEN);
	if (FALSE == aes_siv_decrypt(pSaeIns->kek, pSaeIns->kck_kek_len, cur_pos,
		AES_BLOCK_SIZES + SAE_PK_MODIFIER_BYTES_LEN, 0, NULL, NULL, sae_pk->modifier)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, "aes_siv_decrypt fail\n");
		return SAE_SILENTLY_DISCARDED;
	}

	hex_dump_with_cat_and_lvl("modifier", sae_pk->modifier, SAE_PK_MODIFIER_BYTES_LEN,
					DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL);

	cur_pos += AES_BLOCK_SIZES + SAE_PK_MODIFIER_BYTES_LEN;

	return MLME_SUCCESS;
}


UINT32 sae_pk_build_fils_public_key(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *buf)
{
	struct sae_pk_cfg *sae_pk = pSaeIns->sae_pk_ptr;

	buf[0] = IE_WLAN_EXTENSION;
	buf[1] = 2 + sae_pk->asn1_sub_pub_key_info_len;
	buf[2] = EID_EXT_FILS_PUBLIC_KEY;
	buf[3] = FILS_PUBLIC_KEY_TYPE_RFC_5480;
	os_move_mem(buf + 4, sae_pk->asn1_sub_pub_key_info, sae_pk->asn1_sub_pub_key_info_len);

	if (2 + sae_pk->asn1_sub_pub_key_info_len > 255) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			"The length(%d) is larger than 255, it should do element fragmentation\n", 2 + sae_pk->asn1_sub_pub_key_info_len);
	}

	return 4 + sae_pk->asn1_sub_pub_key_info_len;
}

UINT32 sae_pk_build_fils_key_confirmation(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *buf)
{
	UINT32 key_auth_len = 0;

	buf[0] = IE_WLAN_EXTENSION;
	buf[2] = EID_EXT_FILS_KEY_CONFIRMATION;

	/* retry case, pSaeIns->key_auth will not be null */
	if (pSaeIns->key_auth) {
		key_auth_len = pSaeIns->key_auth_len;
		os_move_mem(buf + 3, pSaeIns->key_auth, pSaeIns->key_auth_len);
	} else
		key_auth_len = sae_pk_derive_signature(pSaeIns, buf + 3);
	buf[1] = key_auth_len + 1;

	if (key_auth_len + 1 > 255) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			"The length(%d) is larger than 255, it should do element fragmentation\n", key_auth_len + 1);
	}

	return key_auth_len + 3;
}


/* todo: ffc path */
UINT32 sae_pk_build_sae_pk_element(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *buf)
{
	UINT32 len = 0;
	UCHAR *pos = buf;
	KDE_HDR *kde = (KDE_HDR *)buf;
	struct sae_pk_cfg *sae_pk = pSaeIns->sae_pk_ptr;
	UCHAR encrypt_modifier[AES_BLOCK_SIZES + SAE_PK_MODIFIER_BYTES_LEN] = {0};

	if (sae_pk == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			" unexpected error due to sae_pk_ptr is null\n");
		return len;
	}

	kde->Type = IE_VENDOR_SPECIFIC;
	os_move_mem(kde->OUI, (UCHAR *)wfa_oui, sizeof(wfa_oui));
	kde->DataType = WFA_KDE_SAE_PK;
	len += sizeof(KDE_HDR);

	aes_siv_encrypt(pSaeIns->kek, pSaeIns->kck_kek_len,
		sae_pk->modifier, SAE_PK_MODIFIER_BYTES_LEN, 0, NULL, NULL, encrypt_modifier);

	os_move_mem(pos + len, encrypt_modifier, sizeof(encrypt_modifier));
	len += sizeof(encrypt_modifier);
	kde->Len = len - 2;

	hex_dump_with_cat_and_lvl("sae_pk_element", buf, len, DBG_CAT_SEC, CATSEC_SAE, SAE_PK_DEBUG_LEVEL);

	return len;
}

UCHAR sae_pk_init(
	IN struct _RTMP_ADAPTER *ad,
	IN struct sae_pk_cfg *sae_pk,
	IN CHAR * ssid,
	IN UCHAR ssid_len,
	IN UCHAR role,
	OUT UCHAR *psk)
{
	os_move_mem(sae_pk->ssid, ssid, ssid_len);
	sae_pk->ssid_len = ssid_len;

	/* according to spec, the minimum required */
	if (sae_pk->sec > SAE_PK_AUTO_GEN_PWD_MAX_SEC || sae_pk->sec < SAE_PK_AUTO_GEN_PWD_MIN_SEC)
		sae_pk->sec = SAE_PK_AUTO_GEN_DEF_SEC;
	if (sae_pk->lambda < SAE_PK_MIN_LAMBDA || sae_pk->lambda > SAE_PK_MAX_LAMBDA)
		sae_pk->lambda = SAE_PK_AUTO_GEN_DEF_LAMBDA;
	sae_pk->role = role;

	if (role == SAE_PK_ROLE_SUPPLICANT)
		return sae_pk_init_for_supplicant(sae_pk, psk);

	if (is_sae_group_ecc(sae_pk->group_id))
		return sae_pk_init_ecc(ad, sae_pk, psk);
	else {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			"sae_pk_init due to unknown group\n");
		return FALSE;
	}
}

VOID sae_pk_deinit(
	IN struct sae_pk_cfg *sae_pk)
{
	if (sae_pk->fixed_pri_key)
		os_free_mem(sae_pk->fixed_pri_key);
	if (sae_pk->fixed_start_modifier)
		os_free_mem(sae_pk->fixed_start_modifier);
	if (sae_pk->pri_key_bin)
		os_free_mem(sae_pk->pri_key_bin);
	if (sae_pk->modifier)
		os_free_mem(sae_pk->modifier);
	if (sae_pk->asn1_sub_pub_key_info)
		os_free_mem(sae_pk->asn1_sub_pub_key_info);
	if (sae_pk->finger_print)
		os_free_mem(sae_pk->finger_print);
	if (sae_pk->sig_r)
		SAE_BN_FREE(&sae_pk->sig_r);
	if (sae_pk->sig_s)
		SAE_BN_FREE(&sae_pk->sig_s);
	if (sae_pk->pri_key)
		SAE_BN_FREE(&sae_pk->pri_key);

	if (is_sae_group_ecc(sae_pk->group_id))
		sae_pk_deinit_ecc(sae_pk);

	os_zero_mem(sae_pk, sizeof(struct sae_pk_cfg));
}


VOID sae_pk_deri_finger_print(
	IN struct sae_pk_cfg *sae_pk,
	IN CHAR * ssid,
	IN UCHAR ssid_len,
	IN UCHAR need_search,
	OUT UCHAR* finger_print)
{
	UCHAR i;
	UCHAR msg[SSID_LEN + SAE_PK_MODIFIER_BYTES_LEN + MAX_ASN1_SUB_PUB_KEY_INFO_LEN];
	UINT32 msg_len = 0;
	UINT32 hash_len_bit = SAE_PK_FINGER_PRINT_BITS_LEN(sae_pk->sec, sae_pk->lambda);
	UINT32 hash_len = ((hash_len_bit + 7) / 8);
	UCHAR left_sec_bytes_or;
	UCHAR result[SHA512_DIGEST_SIZE] = {0};
	EC_GROUP_INFO_BI *ec_group_bi = (EC_GROUP_INFO_BI *) sae_pk->group_info_bi;

	os_move_mem(msg, ssid, ssid_len);
	os_move_mem(msg + ssid_len, sae_pk->modifier, SAE_PK_MODIFIER_BYTES_LEN);
	os_move_mem(msg + ssid_len + SAE_PK_MODIFIER_BYTES_LEN, sae_pk->asn1_sub_pub_key_info, sae_pk->asn1_sub_pub_key_info_len);
	msg_len = ssid_len + SAE_PK_MODIFIER_BYTES_LEN + sae_pk->asn1_sub_pub_key_info_len;

	do {
		/* L(Hash(SSID || M || K_AP), 0, 8*Sec + 5 - 2) */
		if (ec_group_bi->group_id == 19)
			RT_SHA256(msg, msg_len, result);
		else if (ec_group_bi->group_id == 20)
			RT_SHA384(msg, msg_len, result);
		else if (ec_group_bi->group_id == 21)
			RT_SHA512(msg, msg_len, result);

		left_sec_bytes_or = 0;

		for (i = 0; i < sae_pk->sec; i++)
			left_sec_bytes_or |= result[i];

		if (left_sec_bytes_or != 0 && need_search) {
			/* M = M + 1 */
			i = SAE_PK_MODIFIER_BYTES_LEN - 1;
			do {
				msg[ssid_len + i]++;
			} while (i != 0 && msg[ssid_len + i--] == 0);
		} else
			break;
	} while (TRUE);

	os_move_mem(sae_pk->modifier, msg + ssid_len, SAE_PK_MODIFIER_BYTES_LEN);
	os_move_mem(finger_print, result, hash_len);
	if (hash_len_bit % 8)
		finger_print[hash_len - 1] &= BITS(8 - hash_len_bit % 8, 7);

	hex_dump_with_cat_and_lvl("modifier", sae_pk->modifier, SAE_PK_MODIFIER_BYTES_LEN,
						DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO);

	hex_dump_with_cat_and_lvl("finger_print", finger_print, hash_len,
						DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO);
}

inline UCHAR lowercase_base32(UCHAR value)
{
	return (value < 26) ? (value + 'a') : (value - 26 + '2');
}

inline UCHAR lowercase_base32_reverse(UCHAR value)
{
	return (value <= '7') ? value - '2' + 26 : value - 'a';
}

static VOID sae_pk_base32_encode(
	IN struct sae_pk_cfg *sae_pk,
	IN UCHAR* finger_print,
	OUT UCHAR *pwd_base)
{
	UCHAR val = 0;
	UCHAR i = 0;
	UCHAR cur_a_idx;
	UCHAR cur_bit_pos;
	UCHAR cur_bit_len;
	UCHAR bit_5 = (sae_pk->sec == 3) ? (1 << 4) : 0;

	cur_a_idx = sae_pk->sec;
	cur_bit_pos = 0;
	for (i = 0; i < sae_pk->lambda - 1; i++) {
		cur_bit_len = (i % 4) ? 5 : 4;

		if (8 - cur_bit_pos >= cur_bit_len) {
			val = (finger_print[cur_a_idx] >> (8 - cur_bit_pos - cur_bit_len)) & 0x1f;
			cur_bit_pos += cur_bit_len;
			if (cur_bit_pos == 8) {
				cur_bit_pos = 0;
				cur_a_idx++;
			}
		} else {
			val = (finger_print[cur_a_idx++] << (cur_bit_pos + cur_bit_len - 8)) & 0x1f;
			val |= (finger_print[cur_a_idx] >> (16 - cur_bit_pos - cur_bit_len));
			cur_bit_pos = cur_bit_pos + cur_bit_len - 8;
		}
		if (i % 4 == 0)
			val |= bit_5;
		pwd_base[i] = lowercase_base32(val);
	}

	hex_dump_with_cat_and_lvl("pwd_base", pwd_base, sae_pk->lambda,
						DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO);
}

static VOID sae_pk_base32_decode(
	IN struct sae_pk_cfg *sae_pk,
	IN UCHAR *pwd_base,
	OUT UCHAR* finger_print)
{
	UCHAR val = lowercase_base32_reverse(pwd_base[0]);
	UCHAR i;
	UCHAR cur_a_idx;
	UCHAR cur_bit_pos;
	UCHAR cur_bit_len;

	sae_pk->sec = (val >> 4) ? 3 : 5;
	os_zero_mem(finger_print, sae_pk->sec);
	cur_a_idx = sae_pk->sec;
	cur_bit_pos = 0;

	for (i = 0; i < sae_pk->lambda - 1; i++) {
		val = lowercase_base32_reverse(pwd_base[i]);
		cur_bit_len = (i % 4) ? 5 : 4;
		if (i % 4 == 0)
			val &= 0xf;

		if (8 - cur_bit_pos >= cur_bit_len) {
			finger_print[cur_a_idx] |= (val << (8 - cur_bit_pos - cur_bit_len));
			cur_bit_pos += cur_bit_len;
			if (cur_bit_pos == 8) {
				cur_bit_pos = 0;
				cur_a_idx++;
			}
		} else {
			finger_print[cur_a_idx++] |= (val >> (cur_bit_pos + cur_bit_len - 8));
			finger_print[cur_a_idx] = val << (16 - cur_bit_pos - cur_bit_len);
			cur_bit_pos = cur_bit_pos + cur_bit_len - 8;
		}
	}

	hex_dump_with_cat_and_lvl("finger_print", finger_print, cur_a_idx,
						DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO);
}


inline UCHAR base64_reverse(CHAR value)
{
	if ((value <= 'Z') && (value >= 'A'))
		return value - 'A';
	else if ((value <= 'z') && (value >= 'a'))
		return value - 'a' + 26;
	else if ((value <= '9') && (value >= '0'))
		return value - '0' + 52;
	else if (value == '=')
		return 0;
	else
		return (value == '+') ? 62 : 63;
}


static VOID sae_pk_pem_base64_decode(
	IN CHAR *str_in,
	OUT UCHAR *hex_out,
	OUT UINT32 *hex_out_len)
{
	UCHAR i;
	UCHAR j;
	UCHAR tmp;

	for (i = 0, j = 0; i < strlen(str_in); i = i + 4, j = j + 3) {
		hex_out[j] = base64_reverse(str_in[i]) << 2;
		tmp = base64_reverse(str_in[i + 1]);
		hex_out[j] |= tmp >> 4;
		hex_out[j + 1] = tmp << 4;
		tmp = base64_reverse(str_in[i + 2]);
		hex_out[j + 1] |= tmp >> 2;
		hex_out[j + 2] = tmp << 6;
		hex_out[j + 2] |= base64_reverse(str_in[i + 3]);
	}

	if (str_in[i - 1] == '=')
		j = (str_in[i - 2] == '=') ? j - 2 : j - 1;

	*hex_out_len = j;

	hex_dump_with_cat_and_lvl("hex_out", hex_out, j,
						DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO);
}

UCHAR sae_pk_pem_decode(
	IN struct sae_pk_cfg *sae_pk,
	IN CHAR *str_in,
	OUT UCHAR *hex_out,
	OUT UINT32 *hex_out_len)
{
	sae_pk_pem_base64_decode(str_in, hex_out, hex_out_len);

	return asn1_get_private_key(hex_out, *hex_out_len, &sae_pk->pri_key, &sae_pk->group_id);
}



static VOID sae_pk_add_separators(
	IN struct sae_pk_cfg *sae_pk,
	INOUT UCHAR *pwd)
{
	UCHAR total_len = ((sae_pk->lambda - 1) / 4) + sae_pk->lambda;
	UCHAR i = total_len;
	UCHAR j = sae_pk->lambda;

	while (i != j) {
		--i;
		pwd[i] = (i % 5 == 4) ? '-' : pwd[--j];
	}

	pwd[total_len] = '\0';
}

static UCHAR sae_pk_del_separators(
	IN UCHAR *psk,
	OUT UCHAR *pwd)
{
	UCHAR i = 0;
	UCHAR j = 0;
	UCHAR separator_existed = FALSE;

	for (; i < strlen(psk); i++) {
		if (psk[i] != '-')
			pwd[j++] = psk[i];
		else
			separator_existed = TRUE;
	}

	pwd[j] = '\0';

	return separator_existed;
}

static UCHAR is_sae_pk_pwd_format(
	IN UCHAR *psk)
{
	UCHAR i = 0;

	for (; i < strlen(psk); i++) {
		if (i % 5 == 4 && psk[i] != '-')
			return FALSE;
		else if (i % 5 != 4 &&
			!((psk[i] >= 'a' && psk[i] <= 'z') || (psk[i] >= '2' && psk[i] <= '7')))
			return FALSE;
	}
	return TRUE;
}

UCHAR verhoeff_dihedral_group[32][32] = {
	{0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31},
	{1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,  0, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 16},
	{2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,  0,  1, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 16, 17},
	{3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,  0,  1,  2, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18},
	{4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,  0,  1,  2,  3, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19},
	{5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,  0,  1,  2,  3,  4, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20},
	{6,  7,  8,  9, 10, 11, 12, 13, 14, 15,  0,  1,  2,  3,  4,  5, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21},
	{7,  8,  9, 10, 11, 12, 13, 14, 15,  0,  1,  2,  3,  4,  5,  6, 23, 24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22},
	{8,  9, 10, 11, 12, 13, 14, 15,  0,  1,  2,  3,  4,  5,  6,  7, 24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23},
	{9, 10, 11, 12, 13, 14, 15,  0,  1,  2,  3,  4,  5,  6,  7,  8, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24},
	{10, 11, 12, 13, 14, 15,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25},
	{11, 12, 13, 14, 15,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26},
	{12, 13, 14, 15,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27},
	{13, 14, 15,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
	{14, 15,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29},
	{15,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30},
	{16, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,  0, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1},
	{17, 16, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18,  1,  0, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2},
	{18, 17, 16, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19,  2,  1,  0, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3},
	{19, 18, 17, 16, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20,  3,  2,  1,  0, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4},
	{20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21,  4,  3,  2,  1,  0, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5},
	{21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22,  5,  4,  3,  2,  1,  0, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6},
	{22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 26, 25, 24, 23,  6,  5,  4,  3,  2,  1,  0, 15, 14, 13, 12, 11, 10,  9,  8,  7},
	{23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 26, 25, 24,  7,  6,  5,  4,  3,  2,  1,  0, 15, 14, 13, 12, 11, 10,  9,  8},
	{24, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 26, 25,  8,  7,  6,  5,  4,  3,  2,  1,  0, 15, 14, 13, 12, 11, 10,  9},
	{25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 26,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0, 15, 14, 13, 12, 11, 10},
	{26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0, 15, 14, 13, 12, 11},
	{27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0, 15, 14, 13, 12},
	{28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0, 15, 14, 13},
	{29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0, 15, 14},
	{30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 31, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0, 15},
	{31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0},
};

UCHAR verhoeff_inv[] = {
	0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

UCHAR verhoeff_permutation[] = {
	7, 11, 13, 5, 20, 23, 9, 6, 27, 15, 21, 25, 14, 10, 8,
	31, 26, 4, 16, 22, 12, 29, 18, 24, 28, 17, 3, 30, 19, 0
};

UCHAR verhoeff_permutation_inv[] = {
	29, 30, 31, 26, 17, 3, 7, 0, 14, 6, 13, 1, 20, 2, 12, 9,
	18, 25, 22, 28, 4, 10, 19, 5, 23, 11, 16, 8, 24, 21, 27, 15
};

static UCHAR get_permutation(UCHAR index, UCHAR num)
{
	UCHAR inv = verhoeff_permutation_inv[num];

	if (inv >= 30)
		return ((inv + index) % 2 == 0) ? 1 : 2;
	else
		return verhoeff_permutation[(inv + index) % 30]; /* 30 = sizeof(verhoeff_permutation) */
}


UCHAR sae_pk_verhoeff_checksum_verify(
	IN UCHAR *psk)
{
	UCHAR i = 0;
	UCHAR pwd[LEN_PSK] = {0};
	UCHAR pwd_base32[LEN_PSK] = {0};
	UCHAR lamda;
	UCHAR checksum = 0;
	UCHAR permutation;

	sae_pk_del_separators(psk, pwd);
	lamda = strlen(pwd);

	for (i = 0; i < lamda; i++)
		pwd_base32[lamda - i - 1] = lowercase_base32_reverse(pwd[i]);

	for (i = 0; i < lamda; i++) {
		permutation = get_permutation(i, pwd_base32[i]);
		checksum = verhoeff_dihedral_group[checksum][permutation];
	}

	return (checksum == 0);
}


/*See Section 6.5.2 of spec */
UCHAR sae_pk_pwd_sec_encode_check(
	IN UCHAR *pwd)
{
	UCHAR i = 0;
	UCHAR lamda;
	UCHAR value;
	UCHAR saepk_sec;

	lamda = strlen(pwd);
	value = lowercase_base32_reverse(pwd[0]);
	saepk_sec = value >> 4;    // sec=3 , when value >> 4 is 1 else sec=5 when value >> 4 is 0
	for (i = 4; i < lamda; i += 4) {
		value = lowercase_base32_reverse(pwd[i]);
		if ((value >> 4) != saepk_sec) {
			return FALSE;
		}
	}
	return TRUE;
}


/* for supplicant mode, only need to get finger_print from password */
UCHAR sae_pk_init_for_supplicant(
	IN struct sae_pk_cfg *sae_pk,
	IN UCHAR *psk)
{
	UCHAR pwd[LEN_PSK] = {0};

	if (!is_sae_pk_pwd_format(psk)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 "wrong password format, psk = %s\n", psk);
		return FALSE;
	}

	if (!sae_pk_verhoeff_checksum_verify(psk)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 "wrong checksum, psk = %s\n", psk);
		return FALSE;
	}
	sae_pk_del_separators(psk, pwd);
	sae_pk->lambda = strlen(pwd);
	if (sae_pk->lambda < SAE_PK_AUTO_GEN_DEF_LAMBDA) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 "Password length is not valid, psk and Len is = %s %lu\n", psk, strlen(pwd));
		return FALSE;
	}
	if (!sae_pk_pwd_sec_encode_check(pwd)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			"wrong Sec Encoding, pwd = %s\n", pwd);
		return FALSE;
	}
	if (sae_pk->finger_print)
		os_free_mem(sae_pk->finger_print);
	os_alloc_mem(NULL, (UCHAR **) &sae_pk->finger_print, SAE_PK_MAX_FINGER_PRINT_BYTES_LEN);
	os_zero_mem(sae_pk->finger_print, SAE_PK_MAX_FINGER_PRINT_BYTES_LEN);
	sae_pk_base32_decode(sae_pk, pwd, sae_pk->finger_print);

	return TRUE;
}


UCHAR sae_pk_init_ecc(
	IN struct _RTMP_ADAPTER *ad,
	IN struct sae_pk_cfg *sae_pk,
	INOUT UCHAR *psk)
{
	EC_GROUP_INFO_BI *ec_group_bi = NULL;
	UCHAR i;
	UCHAR *modifier; /* 128-bit */
	UCHAR finger_print[SAE_PK_MAX_FINGER_PRINT_BYTES_LEN];
	UINT32 len;
	UCHAR is_compressed = TRUE;
	UCHAR pwd[LEN_PSK];
	UINT32 fp_len_bit;
	UINT32 fp_len;

	ec_group_bi = get_ecc_group_info_bi(sae_pk->group_id);
	sae_pk->group_info_bi = ec_group_bi;

	os_zero_mem(pwd, sizeof(pwd));
	if (is_sae_pk_pwd_format(psk)) {
		sae_pk_del_separators(psk, pwd);
		sae_pk->lambda = strlen(pwd);
		if (sae_pk->lambda < SAE_PK_AUTO_GEN_DEF_LAMBDA) {
			MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				"Password length is not valid, psk and Len is  = %s %lu\n", psk, strlen(pwd));
			return FALSE;
		}
		os_alloc_mem(NULL, (UCHAR **) &sae_pk->finger_print, SAE_PK_MAX_FINGER_PRINT_BYTES_LEN);
		os_zero_mem(sae_pk->finger_print, SAE_PK_MAX_FINGER_PRINT_BYTES_LEN);
		sae_pk_base32_decode(sae_pk, pwd, sae_pk->finger_print);
		if (!sae_pk_verhoeff_checksum_verify(psk)) {
			MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
				"warning! wrong checksum, psk = %s\n", psk);
		}
	} else {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			 "wrong password format\n");
		return FALSE;
	}

	fp_len_bit = SAE_PK_FINGER_PRINT_BITS_LEN(sae_pk->sec, sae_pk->lambda);
	fp_len = ((fp_len_bit + 7) / 8);

	if (sae_pk->fixed_pri_key || sae_pk->pri_key) {
		BIG_INTEGER_EC_POINT *pub = NULL;
		UINT32 pri_key_len = sae_pk->fixed_pri_key_len;

		if (sae_pk->fixed_pri_key) {
			if (pri_key_len > ec_group_bi->ec_group->prime_len)
				pri_key_len = ec_group_bi->ec_group->prime_len;

			SAE_BN_BIN2BI(sae_pk->fixed_pri_key, pri_key_len, &sae_pk->pri_key);
		}

		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "fix_pri_key\n");
		SAE_BN_PRINT(sae_pk->pri_key);
		ECC_POINT_MUL(ec_group_bi->generator, sae_pk->pri_key, ec_group_bi, &pub);
		SAE_ECC_3D_to_2D(ec_group_bi, pub);
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "public key x\n");
		SAE_BN_PRINT(pub->x);
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "public key y\n");
		SAE_BN_PRINT(pub->y);
		sae_pk->pub_key = pub;
	} else if (sae_pk->sae_pk_test_ctrl & SAE_PK_CFG_RANDOMLY_GEN) {
		if (ecc_gen_key(ec_group_bi, &sae_pk->pri_key, &sae_pk->pub_key) == 0) {
			MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 "ecc_gen_key failed...\n");
			return FALSE;
		}
	} else {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 "no private key\n");
		return FALSE;
	}

	if (!(sae_pk->sae_pk_test_ctrl & SAE_PK_CFG_RANDOMLY_GEN) &&
		sae_pk->fixed_start_modifier == NULL) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 "no start_modifier\n");
		return FALSE;
	}

	os_alloc_mem(NULL, (UCHAR **) &sae_pk->pri_key_bin, ec_group_bi->ec_group->prime_len);
	len = ec_group_bi->ec_group->prime_len;
	SAE_BN_BI2BIN_WITH_PAD(sae_pk->pri_key, sae_pk->pri_key_bin, &len, ec_group_bi->ec_group->prime_len);

	os_alloc_mem(NULL, (UCHAR **) &modifier, SAE_PK_MODIFIER_BYTES_LEN);

	if (sae_pk->fixed_start_modifier)
		os_move_mem(modifier, sae_pk->fixed_start_modifier, SAE_PK_MODIFIER_BYTES_LEN);
	else {
		for (i = 0; i < SAE_PK_MODIFIER_BYTES_LEN; i++)
			modifier[i] = RandomByte(ad);
	}

	sae_pk->modifier = modifier;

	os_alloc_mem(NULL, (UCHAR **) &sae_pk->asn1_sub_pub_key_info, MAX_ASN1_SUB_PUB_KEY_INFO_LEN);
	if (sae_pk->sae_pk_test_ctrl & SAE_PK_CFG_SET_UNCOMPRESSED)
		is_compressed = FALSE;
	asn1_get_sub_pub_key_info(ec_group_bi, sae_pk->pub_key, is_compressed, sae_pk->asn1_sub_pub_key_info, &sae_pk->asn1_sub_pub_key_info_len);

	if (sae_pk->sae_pk_test_ctrl & SAE_PK_CFG_RANDOMLY_GEN)
		sae_pk_deri_finger_print(sae_pk, sae_pk->ssid, sae_pk->ssid_len, TRUE, finger_print);
	else
		sae_pk_deri_finger_print(sae_pk, sae_pk->ssid, sae_pk->ssid_len, FALSE, finger_print);

	if (sae_pk->sae_pk_test_ctrl & SAE_PK_CFG_RANDOMLY_GEN) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, " the password is wrong, rederive the password\n");
		os_zero_mem(psk, LEN_PSK);
		sae_pk_base32_encode(sae_pk, finger_print, psk);
		sae_pk_add_separators(sae_pk, psk);
		hex_dump_with_cat_and_lvl("modifier", sae_pk->modifier, SAE_PK_MODIFIER_BYTES_LEN,
						DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR);
	} else if (NdisEqualMemory(finger_print, sae_pk->finger_print, fp_len) == 0) {
		MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, " the password is wrong\n");
		hex_dump_with_lvl("finger_print", finger_print, fp_len, DBG_LVL_ERROR);
		hex_dump_with_lvl("expected finger_print (derived from pwd)", sae_pk->finger_print, fp_len, DBG_LVL_ERROR);

		if (!(sae_pk->sae_pk_test_ctrl & SAE_PK_CFG_NO_REDERIVE_PW))
			return FALSE;
	}

	MTWF_DBG(ad, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "\x1b[32m password = %s\x1b[m\n", psk);

	return TRUE;
}


VOID sae_pk_deinit_ecc(
	IN struct sae_pk_cfg *sae_pk)
{
	if (sae_pk->pub_key) {
		BIG_INTEGER_EC_POINT *point = (BIG_INTEGER_EC_POINT *)sae_pk->pub_key;

		ecc_point_free(&point);
	}
}
/* sae-pk related api end */

VOID sae_group_init_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN INT32 group)
{
	EC_GROUP_INFO *ec_group = NULL;
	EC_GROUP_INFO_BI *ec_group_bi = NULL;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	ec_group = get_ecc_group_info(group);
	ec_group_bi = get_ecc_group_info_bi(group);

	if (ec_group == NULL
		|| ec_group_bi == NULL)
		return;
	pSaeIns->group_info = (VOID *) ec_group;
	pSaeIns->group_info_bi = (VOID *) ec_group_bi;
	pSaeIns->prime = ec_group_bi->prime;
	pSaeIns->prime_len = ec_group->prime_len;
	pSaeIns->order = ec_group_bi->order;
	pSaeIns->order_len = ec_group->order_len;
	pSaeIns->group = group;
}


VOID sae_group_init_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN INT32 group)
{
	UINT32 i;
	DH_GROUP_INFO *dh_group = NULL;
	DH_GROUP_INFO_BI *dh_group_bi = NULL;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	for (i = 0; i < DH_GROUP_NUM; i++) {
		if (dh_groups[i].group_id == group) {
			dh_group = &dh_groups[i];
			dh_group_bi = &dh_groups_bi[i];
		}
	}

	if (dh_group == NULL
		|| dh_group_bi == NULL)
		return;

	if (dh_group_bi->is_init == FALSE) {
		dh_group_bi->prime = NULL;
		dh_group_bi->order = NULL;
		dh_group_bi->generator = NULL;
		SAE_BN_BIN2BI((UINT8 *)dh_group->prime,
						  dh_group->prime_len,
						  &dh_group_bi->prime);
		SAE_BN_BIN2BI((UINT8 *)dh_group->order,
						  dh_group->order_len,
						  &dh_group_bi->order);
		SAE_BN_BIN2BI((UINT8 *)dh_group->generator,
						  dh_group->generator_len,
						  &dh_group_bi->generator);
	}

	pSaeIns->group_info = (VOID *) dh_group;
	pSaeIns->group_info_bi = (VOID *) dh_group_bi;
	pSaeIns->prime = dh_group_bi->prime;
	pSaeIns->prime_len = dh_group->prime_len;
	pSaeIns->order = dh_group_bi->order;
	pSaeIns->order_len = dh_group->order_len;
	pSaeIns->group = group;
}


VOID sae_group_deinit_ecc(
	IN SAE_INSTANCE *pSaeIns)
{
	BIG_INTEGER_EC_POINT *own_element = NULL;
	BIG_INTEGER_EC_POINT *peer_element = NULL;
	BIG_INTEGER_EC_POINT *pwe = NULL;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);
	own_element = (BIG_INTEGER_EC_POINT *)pSaeIns->own_commit_element;
	peer_element = (BIG_INTEGER_EC_POINT *)pSaeIns->peer_commit_element;
	pwe = (BIG_INTEGER_EC_POINT *)pSaeIns->pwe;

	if (own_element)
		ecc_point_free(&own_element);

	if (peer_element)
		ecc_point_free(&peer_element);

	if (pwe)
		ecc_point_free(&pwe);
}


VOID sae_group_deinit_ffc(
	IN SAE_INSTANCE *pSaeIns)
{
	SAE_BN *own_element = NULL;
	SAE_BN *peer_element = NULL;
	SAE_BN *pwe = NULL;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);
	own_element = (SAE_BN *)pSaeIns->own_commit_element;
	peer_element = (SAE_BN *)pSaeIns->peer_commit_element;
	pwe = (SAE_BN *)pSaeIns->pwe;

	if (own_element)
		SAE_BN_FREE(&own_element);

	pSaeIns->own_commit_element = NULL;

	if (peer_element)
		SAE_BN_FREE(&peer_element);

	pSaeIns->peer_commit_element = NULL;

	if (pwe)
		SAE_BN_FREE(&pwe);

	pSaeIns->pwe = NULL;
}



VOID sae_cn_confirm_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR is_send, /* otherwise, is verfication */
	OUT UCHAR *confirm)
{
	UCHAR own_element_bin[2 * SAE_MAX_ECC_PRIME_LEN];
	UCHAR peer_element_bin[2 * SAE_MAX_ECC_PRIME_LEN];
	UINT32 prime_len;
	BIG_INTEGER_EC_POINT *own_element = (BIG_INTEGER_EC_POINT *)pSaeIns->own_commit_element; /* ellis */
	BIG_INTEGER_EC_POINT *peer_element = (BIG_INTEGER_EC_POINT *)pSaeIns->peer_commit_element;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);
	prime_len = pSaeIns->prime_len; /* ellis */
	SAE_BN_BI2BIN_WITH_PAD(own_element->x, own_element_bin,
							   &prime_len, pSaeIns->prime_len);
	prime_len = pSaeIns->prime_len;
	SAE_BN_BI2BIN_WITH_PAD(own_element->y, own_element_bin + pSaeIns->prime_len,
							   &prime_len, pSaeIns->prime_len);
	prime_len = pSaeIns->prime_len;
	SAE_BN_BI2BIN_WITH_PAD(peer_element->x, peer_element_bin,
							   &prime_len, pSaeIns->prime_len);
	prime_len = pSaeIns->prime_len;
	SAE_BN_BI2BIN_WITH_PAD(peer_element->y, peer_element_bin + pSaeIns->prime_len,
							   &prime_len, pSaeIns->prime_len);

	if (is_send)
		sae_cn_confirm_cmm(pSaeIns, pSaeIns->own_commit_scalar,
						   pSaeIns->peer_commit_scalar,
						   own_element_bin, peer_element_bin,
						   2 * pSaeIns->prime_len,
						   pSaeIns->send_confirm,
						   confirm);
	else
		sae_cn_confirm_cmm(pSaeIns, pSaeIns->peer_commit_scalar,
						   pSaeIns->own_commit_scalar,
						   peer_element_bin, own_element_bin,
						   2 * pSaeIns->prime_len,
						   pSaeIns->peer_send_confirm,
						   confirm);
}



VOID sae_cn_confirm_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR is_send, /* otherwise, is verfication */
	OUT UCHAR *confirm)
{
	UCHAR *own_element_bin = NULL;
	UCHAR *peer_element_bin = NULL;
	UINT32 prime_len;
	SAE_BN *own_element = (SAE_BN *)pSaeIns->own_commit_element;
	SAE_BN *peer_element = (SAE_BN *)pSaeIns->peer_commit_element;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()is_send = %d\n", __func__, is_send);
	os_alloc_mem(NULL, (UCHAR **)&own_element_bin, SAE_MAX_PRIME_LEN);

	if (own_element_bin == NULL)
		return;

	os_alloc_mem(NULL, (UCHAR **)&peer_element_bin, SAE_MAX_PRIME_LEN);

	if (peer_element_bin == NULL) {
		os_free_mem(own_element_bin);
		return;
	}

	prime_len = pSaeIns->prime_len; /* ellis */
	SAE_BN_BI2BIN_WITH_PAD(own_element, own_element_bin,
							   &prime_len, pSaeIns->prime_len);
	prime_len = pSaeIns->prime_len; /* ellis */
	SAE_BN_BI2BIN_WITH_PAD(peer_element, peer_element_bin,
							   &prime_len, pSaeIns->prime_len);

	if (is_send)
		sae_cn_confirm_cmm(pSaeIns, pSaeIns->own_commit_scalar,
						   pSaeIns->peer_commit_scalar,
						   own_element_bin, peer_element_bin,
						   pSaeIns->prime_len,
						   pSaeIns->send_confirm,
						   confirm);
	else
		sae_cn_confirm_cmm(pSaeIns, pSaeIns->peer_commit_scalar,
						   pSaeIns->own_commit_scalar,
						   peer_element_bin, own_element_bin,
						   pSaeIns->prime_len,
						   pSaeIns->peer_send_confirm,
						   confirm);

	os_free_mem(own_element_bin);
	os_free_mem(peer_element_bin);
}

VOID sae_cn_confirm_cmm(
	IN SAE_INSTANCE *pSaeIns,
	IN SAE_BN *scalar1,
	IN SAE_BN *scalar2,
	IN UCHAR *element_bin1,
	IN UCHAR *element_bin2,
	IN UINT32 element_len,
	IN USHORT send_confirm,
	OUT UCHAR *confirm)
{
	UCHAR *msg;
	UINT32 msg_len = sizeof(send_confirm) + 2 * element_len + 2 * pSaeIns->prime_len;
	UINT32 offset = 0;
	UINT32 prime_len;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s(), send_confirm = %d\n", __func__, send_confirm);
	os_alloc_mem(NULL, &msg, msg_len);

	if (msg == NULL)
		return;

	/*
	  * CN(key, X, Y, Z, ¡K) = HMAC-SHA256(key, D2OS(X) || D2OS(Y) || D2OS(Z) || ¡K)
	  * where D2OS() represents the data to octet string conversion functions in 12.4.7.2.
	  * confirm = CN(KCK, send-confirm, commit-scalar, COMMIT-ELEMENT,
	  *              peer-commit-scalar, PEER-COMMIT-ELEMENT)
	  * verifier = CN(KCK, peer-send-confirm, peer-commit-scalar,
	  *               PEER-COMMIT-ELEMENT, commit-scalar, COMMIT-ELEMENT)
	  */
	NdisMoveMemory(msg, &send_confirm, sizeof(send_confirm));
	offset += sizeof(send_confirm);
	prime_len = pSaeIns->prime_len;
	SAE_BN_BI2BIN_WITH_PAD(scalar1, msg + offset,
							   &prime_len, pSaeIns->prime_len);
	offset += pSaeIns->prime_len;
	NdisMoveMemory(msg + offset, element_bin1, element_len);
	offset += element_len;
	prime_len = pSaeIns->prime_len;
	SAE_BN_BI2BIN_WITH_PAD(scalar2, msg + offset,
							   &prime_len, pSaeIns->prime_len);
	offset += pSaeIns->prime_len;
	NdisMoveMemory(msg + offset, element_bin2, element_len);
	offset += element_len;

	hex_dump_with_lvl("element_bin1:", (char *)element_bin1, element_len, SAE_DEBUG_LEVEL);
	hex_dump_with_lvl("element_bin2:", (char *)element_bin2, element_len, SAE_DEBUG_LEVEL);
	hkdf_extract(pSaeIns->kck, pSaeIns->kck_kek_len, msg, msg_len, confirm, pSaeIns->kck_kek_len);
	hex_dump_with_lvl("confirm:", (char *)confirm, pSaeIns->kck_kek_len, SAE_DEBUG_LEVEL);

	os_free_mem(msg);
}


USHORT sae_parse_commit_element_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end)
{
	SAE_BN *peer_element_x = NULL;
	SAE_BN *peer_element_y = NULL;
	BIG_INTEGER_EC_POINT *peer_element = NULL;
	EC_GROUP_INFO_BI *ec_group_bi = (EC_GROUP_INFO_BI *) pSaeIns->group_info_bi;
	USHORT res = MLME_SUCCESS;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	if (*pos + 2 * pSaeIns->prime_len > end) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 "not enough data in commit element\n");

		res = MLME_UNSPECIFY_FAIL;
		goto fail;
	}

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[2]);
	GET_BI_INS_FROM_POOL(peer_element_x);
	GET_BI_INS_FROM_POOL(peer_element_y);

	SAE_BN_BIN2BI(*pos, pSaeIns->prime_len, &peer_element_x);
	SAE_BN_BIN2BI(*pos + pSaeIns->prime_len, pSaeIns->prime_len, &peer_element_y);
	hex_dump_with_cat_and_lvl("peer element x:", (char *)*pos, pSaeIns->prime_len,
					DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG);
	hex_dump_with_cat_and_lvl("peer element y:", (char *)*pos + pSaeIns->prime_len, pSaeIns->prime_len,
					DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG);
	/*
	  * For ECC groups, both the x- and ycoordinates
	  * of the element shall be non-negative integers less than the prime number p, and the two
	  * coordinates shall produce a valid point on the curve satisfying the group¡¦s curve definition, not being equal
	  * to the ¡§point at the infinity.¡¨ If either of those conditions does not hold, element validation fails; otherwise,
	  * element validation succeeds.
	  */
	ecc_point_init(&peer_element);
	SAE_BN_COPY(peer_element_x, &peer_element->x);
	SAE_BN_COPY(peer_element_y, &peer_element->y);
	SAE_ECC_SET_Z_TO_1(peer_element);

	if (ecc_point_is_on_curve(ec_group_bi, peer_element) == FALSE) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 "The function is not on curve\n");
		ecc_point_free(&peer_element);
		res = MLME_UNSPECIFY_FAIL;
		goto fail;
	}

	*pos += pSaeIns->prime_len * 2;

	/* Avoid memory leakage if pSaeIns->peer_commit_element not NULL. */
	if (pSaeIns->peer_commit_element) {
		BIG_INTEGER_EC_POINT *element = NULL;

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, "pSaeIns->peer_commit_element is not NULL!\n");
		element = (BIG_INTEGER_EC_POINT *)pSaeIns->peer_commit_element;
		ecc_point_free(&element);
	}
	pSaeIns->peer_commit_element = peer_element;
fail:
	SAE_BN_RELEASE_BACK_TO_POOL(&peer_element_x);
	SAE_BN_RELEASE_BACK_TO_POOL(&peer_element_y);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[2]);
	return res;
}

USHORT sae_parse_commit_element_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end)
{
	SAE_BN *scalar_op_res = NULL;
	SAE_BN *peer_commit_element = NULL;
	USHORT res = MLME_UNSPECIFY_FAIL;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	if (*pos + pSaeIns->prime_len > end)
		goto fail;

	SAE_BN_BIN2BI(*pos, pSaeIns->prime_len, &peer_commit_element);
	hex_dump_with_lvl("peer element:", (char *)*pos, pSaeIns->prime_len, SAE_DEBUG_LEVEL2);

	if (peer_commit_element == NULL)
		goto fail;

	/*
	  * For FFC groups, the element shall be an integer greater than zero (0) and less than the prime number p,
	  * and the scalar operation of the element and the order of the group, r, shall equal one (1) modulo the prime number p
	  */
	/* 0 < element < p */
	if (SAE_BN_IS_ZERO(peer_commit_element)
		|| SAE_BN_UCMP(peer_commit_element, pSaeIns->prime) >= 0) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 "invalid peer element\n");
		goto fail;
	}

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[3]);
	/* GET_BI_INS_FROM_POOL(scalar_op_res); */

	/* scalar-op(r, ELEMENT) = 1 modulo p */
	SAE_BN_MOD_EXP_MONT(peer_commit_element, pSaeIns->order, pSaeIns->prime, &scalar_op_res);

	if (!SAE_BN_IS_ONE(scalar_op_res)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 "invalid peer element (scalar-op)\n");
		goto fail;
	}

	pSaeIns->peer_commit_element = peer_commit_element;
	res = MLME_SUCCESS;
	*pos += pSaeIns->prime_len;
fail:
	if (res != MLME_SUCCESS)
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 "fail\n");

	SAE_BN_RELEASE_BACK_TO_POOL(&scalar_op_res);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[3]);

	if (res == MLME_UNSPECIFY_FAIL)
		SAE_BN_FREE(&peer_commit_element);

	return res;
}

UCHAR sae_derive_commit_element_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN SAE_BN *mask)
{
	BIG_INTEGER_EC_POINT *pwe = (BIG_INTEGER_EC_POINT *)pSaeIns->pwe;
	BIG_INTEGER_EC_POINT *commit_element = NULL;
	EC_GROUP_INFO_BI *ec_group_bi = (EC_GROUP_INFO_BI *)pSaeIns->group_info_bi;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	/* COMMIT-ELEMENT = inverse(scalar-op(mask, PWE)) */
	if (pwe == NULL || mask == NULL)
		return FALSE;

	ECC_POINT_MUL(pwe, mask, ec_group_bi, &commit_element);

	SAE_ECC_3D_to_2D(ec_group_bi, commit_element);

	if (commit_element == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 "ECC_POINT_MUL fail\n");
		return FALSE;
	}

	if (!ecc_point_is_on_curve(ec_group_bi, commit_element)) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, "ecc_point_mul_dblandadd fail!!!!!!\n");
		return FALSE;
	}

	ecc_point_inverse(commit_element, ec_group_bi->prime, &commit_element);

	if (commit_element == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 "inverse fail\n");
		return FALSE;
	}

	/* Avoid memory leakage if pSaeIns->own_commit_element is not NULL. */
	if (pSaeIns->own_commit_element) {
		BIG_INTEGER_EC_POINT *element = NULL;

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, "pSaeIns->own_commit_element is not NULL!\n");
		element = (BIG_INTEGER_EC_POINT *)pSaeIns->own_commit_element;
		ecc_point_free(&element);
	}
	pSaeIns->own_commit_element = (VOID *) commit_element;
	return TRUE;
}

UCHAR sae_derive_commit_element_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN SAE_BN *mask)
{
	SAE_BN *commit_element = NULL;
	SAE_BN *tmp = NULL;
	SAE_BN *pwe = (SAE_BN *) pSaeIns->pwe;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	/* COMMIT-ELEMENT = inverse(scalar-op(mask, PWE)) */
	if (pwe == NULL || mask == NULL)
		return FALSE;

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[4]);
	/* GET_BI_INS_FROM_POOL(tmp); */

	SAE_BN_INIT(&tmp);
	SAE_BN_MOD_EXP_MONT(pwe, mask, pSaeIns->prime, &tmp);
	/* SAE_BN_MOD_EXP_MONT(pwe, mask, pSaeIns->prime, &commit_element); */
	SAE_BN_MOD_MUL_INV(tmp, pSaeIns->prime, &commit_element);
	/* SAE_BN_MOD_MUL_INV(commit_element, pSaeIns->prime, &tmp); */
	SAE_BN_FREE(&tmp);

	if (commit_element == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 " inverse fail\n");
		return FALSE;
	}

	pSaeIns->own_commit_element = (VOID *) commit_element;
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[4]);
	return TRUE;
}

USHORT sae_derive_pwe_ecc(
	IN SAE_INSTANCE *pSaeIns)
{
	UCHAR counter = 0;
	UCHAR k = pSaeIns->pParentSaeCfg->k_iteration_var;
	UCHAR addrs[2 * MAC_ADDR_LEN];
	BIG_INTEGER_EC_POINT *res = NULL;
	UCHAR base[LEN_PSK + SAE_MAX_PWD_ID + 1];
	UCHAR msg[LEN_PSK + SAE_MAX_PWD_ID + 2]; /* sizeof(base)+sizeof(counter) */
	UINT32 base_len = strlen(pSaeIns->psk);
	UINT32 msg_len;
	UCHAR pwd_seed[SHA256_DIGEST_SIZE];
	UCHAR pwd_value[SAE_MAX_ECC_PRIME_LEN] = {0};
	EC_GROUP_INFO *ec_group;
	EC_GROUP_INFO_BI *ec_group_bi;
	SAE_BN *x = NULL;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	/* Get larger address first */
	if (RTMPCompareMemory(pSaeIns->own_mac, pSaeIns->peer_mac, MAC_ADDR_LEN) == 1) {
		COPY_MAC_ADDR(addrs, pSaeIns->own_mac);
		COPY_MAC_ADDR(addrs + MAC_ADDR_LEN, pSaeIns->peer_mac);
	} else {
		COPY_MAC_ADDR(addrs, pSaeIns->peer_mac);
		COPY_MAC_ADDR(addrs + MAC_ADDR_LEN, pSaeIns->own_mac);
	}

	NdisMoveMemory(base, pSaeIns->psk, base_len);

	if (pSaeIns->pwd_id_ptr) {
		NdisMoveMemory(base + base_len,
			pSaeIns->pwd_id_ptr->pwd_id, strlen(pSaeIns->pwd_id_ptr->pwd_id));
		base_len += strlen(pSaeIns->pwd_id_ptr->pwd_id);
	}

	hex_dump_with_lvl("base:", (char *)base, base_len, SAE_DEBUG_LEVEL2);
	ec_group = (EC_GROUP_INFO *)pSaeIns->group_info;
	ec_group_bi = (EC_GROUP_INFO_BI *)pSaeIns->group_info_bi;

	for (counter = 1; counter <= k || !res; counter++) {
		UCHAR shift_idx;
		SAE_BN *y = NULL;
		UINT32 i;
		UINT8 lsb_pwd_seed;
		UINT8 lsb_y;
		UCHAR has_y;

		if (counter == 0) {
			SAE_BN_FREE(&x);
			return MLME_UNSPECIFY_FAIL;
		}

		/* pwd-seed = H(MAX(STA-A-MAC, STA-B-MAC) || MIN(STA-A-MAC, STA-B-MAC),
				base || counter) */
		NdisMoveMemory(msg, base, base_len);
		NdisMoveMemory(msg + base_len, &counter, sizeof(counter));
		msg_len = base_len + sizeof(counter);

		hkdf_extract(addrs, sizeof(addrs), msg, msg_len, pwd_seed, sizeof(pwd_seed));
		lsb_pwd_seed = pwd_seed[SHA256_DIGEST_SIZE - 1] & BIT0;

		hex_dump_with_lvl("pwd_seed:", (char *)pwd_seed, sizeof(pwd_seed), SAE_DEBUG_LEVEL);
		/*  z = len(p)
		     pwd-value = KDF-z(pwd-seed, ¡§SAE Hunting and Pecking¡¨, p) */
		KDF_256_bit_len(pwd_seed, sizeof(pwd_seed), (UINT8 *)"SAE Hunting and Pecking", 23,
			(UINT8 *)ec_group->prime, ec_group->prime_len,
			pwd_value, pSaeIns->prime_len, ec_group->prime_len_bit, FALSE);

		hex_dump_with_lvl("pwd_value:", (char *)pwd_value, pSaeIns->prime_len, SAE_DEBUG_LEVEL);
		/* pwd-value should be less than prime */
		shift_idx = ec_group->prime_len - (pSaeIns->prime_len);

		if (NdisCmpMemory(pwd_value, ec_group->prime + shift_idx,
						  pSaeIns->prime_len) == 1)
			continue;

		/* x = pwd-value
		    y^2 = x^3 + ax + b */
		SAE_BN_BIN2BI(pwd_value, pSaeIns->prime_len, &x);
		has_y = ecc_point_find_by_x(ec_group_bi, x, &y, (res == NULL));

		if (has_y == FALSE)
			continue;

		if (!res) {
			ecc_point_init(&res);

			if (res == NULL) {
				SAE_BN_FREE(&x);
				SAE_BN_FREE(&y);
				return MLME_UNSPECIFY_FAIL;
			}

			/* if LSB(pwd-seed) = LSB(y)
			  * then PWE = (x, y)
			  * else PWE = (x, p - y)
			  */
			lsb_y = SAE_BN_IS_ODD(y);
			res->x = x;

			if (lsb_pwd_seed == lsb_y)
				res->y = y;
			else {
				res->y = NULL;
				SAE_BN_SUB(ec_group_bi->prime, y, &res->y);
				SAE_BN_FREE(&y);
			}

			SAE_ECC_SET_Z_TO_1(res);

			if (DebugLevel >= DBG_LVL_INFO) {
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "pwe->x\n");
				SAE_BN_PRINT(res->x);
				MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, "pwe->y\n");
				SAE_BN_PRINT(res->y);
			}
			x = NULL;
		} else {
			SAE_BN_FREE(&y);

			/* base = new-random-number */
			for (i = 0; i < base_len; i++)
				base[i] = RandomByte(pSaeIns->pParentSaeCfg->pAd);
		}
	}

	/* Avoid memory leakage if pSaeIns->pwe is not NULL. */
	if (pSaeIns->pwe) {
		BIG_INTEGER_EC_POINT *element = NULL;

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, "pSaeIns->pwe is not NULL!\n");
		element = (BIG_INTEGER_EC_POINT *)pSaeIns->pwe;
		ecc_point_free(&element);
	}
	SAE_BN_FREE(&x);
	pSaeIns->pwe = (VOID *)res;
	return MLME_SUCCESS;
}

/********************/
/* h2e group related api*/
/********************/
USHORT sae_derive_pwe_pt_ecc(
	IN SAE_INSTANCE *pSaeIns)
{
	UINT32 hash_len;
	UCHAR hash[64], salt[64];
	UCHAR addrs[2 * MAC_ADDR_LEN];
	SAE_BN *val = NULL;
	SAE_BN *q = NULL;
	SAE_BN *one_bn = NULL;
	SAE_BN *tmp = NULL;
	EC_GROUP_INFO_BI *ec_group_bi;
	UCHAR one[] = {1};
	BIG_INTEGER_EC_POINT *res = NULL;

	/* val = H(0^n,
	 *         MAX(STA-A-MAC, STA-B-MAC) || MIN(STA-A-MAC, STA-B-MAC))
	 */
	if (RTMPCompareMemory(pSaeIns->own_mac, pSaeIns->peer_mac, MAC_ADDR_LEN) == 1) {
		COPY_MAC_ADDR(addrs, pSaeIns->own_mac);
		COPY_MAC_ADDR(addrs + MAC_ADDR_LEN, pSaeIns->peer_mac);
	} else {
		COPY_MAC_ADDR(addrs, pSaeIns->peer_mac);
		COPY_MAC_ADDR(addrs + MAC_ADDR_LEN, pSaeIns->own_mac);
	}
	NdisZeroMemory(salt, sizeof(salt));
	hash_len = sae_ecc_prime_len_2_hash_len(pSaeIns->prime_len);
	hkdf_extract(salt, hash_len, addrs, sizeof(addrs), hash, hash_len);

	SAE_BN_BIN2BI(hash, hash_len, &val);

	/* val = val modulo (q - 1) + 1 */
	ec_group_bi = (EC_GROUP_INFO_BI *)pSaeIns->group_info_bi;
	SAE_BN_BIN2BI(one, sizeof(one), &one_bn);
	SAE_BN_SUB(pSaeIns->order, one_bn, &q);
	SAE_BN_MOD(val, q, &tmp);
	SAE_BN_ADD(tmp, one_bn, &val);

	/* PWE = scalar-op(val, PT) */
	ECC_POINT_MUL((BIG_INTEGER_EC_POINT *)pSaeIns->pt, val, ec_group_bi, &res);

	/* Avoid memory leakage if pSaeIns->pwe is not NULL. */
	if (pSaeIns->pwe) {
		BIG_INTEGER_EC_POINT *element = NULL;

		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, "pSaeIns->pwe is not NULL!\n");
		element = (BIG_INTEGER_EC_POINT *)pSaeIns->pwe;
		ecc_point_free(&element);
	}
	pSaeIns->pwe = (VOID *)res;

	SAE_BN_FREE(&val);
	SAE_BN_FREE(&q);
	SAE_BN_FREE(&one_bn);
	SAE_BN_FREE(&tmp);

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
					"%s(): derive pt done\n", __func__);

	return MLME_SUCCESS;
}
/* h2e group related api end */

USHORT sae_derive_pwe_ffc(
	IN SAE_INSTANCE *pSaeIns)
{
	UCHAR counter = 0;
	UCHAR found = FALSE;
	UCHAR addrs[2 * MAC_ADDR_LEN];
	SAE_BN *pwe = NULL;
	SAE_BN *exp = NULL;
	UCHAR msg[LEN_PSK + SAE_MAX_PWD_ID + 2]; /* sizeof(base)+sizeof(counter) */
	UINT32 msg_len;
	UCHAR pwd_seed[SHA256_DIGEST_SIZE] = {0};
	UCHAR *pwd_value = NULL;
	SAE_BN *pwd_value_bi = NULL;
	SAE_BN *tmp_bi = NULL;
	DH_GROUP_INFO *dh_group;
	DH_GROUP_INFO_BI *dh_group_bi;
	UCHAR tmp[1];
	USHORT res;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[5]);
	/* GET_BI_INS_FROM_POOL(exp); */
	/* GET_BI_INS_FROM_POOL(pwd_value_bi); */

	/* Get larger address first */
	if (RTMPCompareMemory(pSaeIns->own_mac, pSaeIns->peer_mac, MAC_ADDR_LEN) == 1) {
		COPY_MAC_ADDR(addrs, pSaeIns->own_mac);
		COPY_MAC_ADDR(addrs + MAC_ADDR_LEN, pSaeIns->peer_mac);
	} else {
		COPY_MAC_ADDR(addrs, pSaeIns->peer_mac);
		COPY_MAC_ADDR(addrs + MAC_ADDR_LEN, pSaeIns->own_mac);
	}


	hex_dump_with_lvl("psk:", (char *)pSaeIns->psk, strlen(pSaeIns->psk), SAE_DEBUG_LEVEL);
	dh_group = (DH_GROUP_INFO *)pSaeIns->group_info;
	/* dh_group_bi->prime == pSaeIns->prime, dh_group_bi->order == pSaeIns->order */
	dh_group_bi = (DH_GROUP_INFO_BI *)pSaeIns->group_info_bi;
	SAE_BN_INIT(&exp);
	SAE_BN_INIT(&pwd_value_bi);
	SAE_BN_INIT(&pwe);
	SAE_BN_INIT(&tmp_bi);

	os_alloc_mem(NULL, (UCHAR **)&pwd_value, SAE_MAX_PRIME_LEN);

	for (counter = 1; counter <= 200; counter++) {
		UCHAR shift_idx;
		UINT32 len = 0;
		/* pwd-seed = H(MAX(STA-A-MAC, STA-B-MAC) || MIN(STA-A-MAC, STA-B-MAC),
				password || counter) */
		NdisMoveMemory(msg, pSaeIns->psk, strlen(pSaeIns->psk));
		len += strlen(pSaeIns->psk);
		hex_dump_with_lvl("msg:", (char *)msg, strlen(pSaeIns->psk), SAE_DEBUG_LEVEL);
		if (pSaeIns->pwd_id_ptr) {
			NdisMoveMemory(msg + len,
				pSaeIns->pwd_id_ptr->pwd_id, strlen(pSaeIns->pwd_id_ptr->pwd_id));
			len += strlen(pSaeIns->pwd_id_ptr->pwd_id);
		}
		NdisMoveMemory(msg + len, &counter, sizeof(counter));
		msg_len = len + sizeof(counter);
		hex_dump_with_lvl("addr:", (char *)addrs, 2 * MAC_ADDR_LEN, SAE_DEBUG_LEVEL);
		hex_dump_with_lvl("msg:", (char *)msg, msg_len, SAE_DEBUG_LEVEL);
		RT_HMAC_SHA256(addrs, sizeof(addrs), msg, msg_len, pwd_seed, sizeof(pwd_seed));
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
				 "%s(): z = len(p) = %d\n", __func__, SAE_BN_GET_LEN(pSaeIns->prime));

		hex_dump_with_lvl("pwd_seed:", (char *)pwd_seed, SHA256_DIGEST_SIZE, SAE_DEBUG_LEVEL);
		hex_dump_with_lvl("prime:", (char *)dh_group->prime, dh_group->prime_len, SAE_DEBUG_LEVEL);
		/*  z = len(p)
		     pwd-value = KDF-z(pwd-seed, ¡§SAE Hunting and Pecking¡¨, p) */
		KDF_256(pwd_seed, sizeof(pwd_seed), (UINT8 *)"SAE Hunting and Pecking", 23,
			(UINT8 *)dh_group->prime, dh_group->prime_len,
			pwd_value, pSaeIns->prime_len);
		hex_dump_with_lvl("pwd_value:", (char *)pwd_value, pSaeIns->prime_len, SAE_DEBUG_LEVEL);
		/* pwd-value should be less than prime */
		shift_idx = dh_group->prime_len - pSaeIns->prime_len;

		if (NdisCmpMemory(pwd_value, dh_group->prime + shift_idx,
						  dh_group->prime_len) == 1)
			continue;

		/* PWE = pwd-value^(p-1)/r modulo p */
		if (dh_group->safe_prime) {
			/*
			 * r = (p-1)/2 => (p-1)/r = 2
			 */
			tmp[0] = 2;
			SAE_BN_BIN2BI(tmp, sizeof(tmp), &exp);
		} else {
			/* GET_BI_INS_FROM_POOL(tmp_bi); */
			tmp[0] = 1;
			SAE_BN_BIN2BI(tmp, sizeof(tmp), &exp);
			SAE_BN_SUB(dh_group_bi->prime, exp, &tmp_bi); /* ellis: SAE_BN_SUB(A, B, A) need to be fix */
			SAE_BN_MOD(tmp_bi, dh_group_bi->order, &exp);
		}

		SAE_BN_BIN2BI(pwd_value, dh_group->prime_len, &pwd_value_bi);
		SAE_BN_MOD_EXP_MONT(pwd_value_bi, exp, dh_group_bi->prime, &pwe);

		/* if (PWE > 1) => found */
		if (!SAE_BN_IS_ZERO(pwe) && !SAE_BN_IS_ONE(pwe)) {
			found = TRUE;
			break;
		}
	}

	if (found) {
		pSaeIns->pwe = (VOID *) pwe;
		res = MLME_SUCCESS;
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
				 "%s(): Success to derive PWE\n", __func__);
	} else {
		SAE_BN_FREE(&pwe);
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 " Failed to derive PWE\n");
		res = MLME_UNSPECIFY_FAIL;
	}
	os_free_mem(pwd_value);
	SAE_BN_RELEASE_BACK_TO_POOL(&exp);
	SAE_BN_RELEASE_BACK_TO_POOL(&pwd_value_bi);
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp_bi);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[5]);
	return res;
}



UCHAR sae_derive_k_ecc(
	IN SAE_INSTANCE *pSaeIns,
	OUT UCHAR *k)
{
	BIG_INTEGER_EC_POINT *K = NULL;
	BIG_INTEGER_EC_POINT *pwe = (BIG_INTEGER_EC_POINT *) pSaeIns->pwe;
	BIG_INTEGER_EC_POINT *peer_commit_element =
		(BIG_INTEGER_EC_POINT *) pSaeIns->peer_commit_element;
	EC_GROUP_INFO_BI *ec_group_bi = (EC_GROUP_INFO_BI *)pSaeIns->group_info_bi;
	UINT32 len = SAE_MAX_ECC_PRIME_LEN;
	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);
	/*
	 * K = scalar-op(rand, (elem-op(scalar-op(peer-commit-scalar, PWE),
	 *                                        PEER-COMMIT-ELEMENT)))
	 */
	ECC_POINT_MUL(pwe, pSaeIns->peer_commit_scalar, ec_group_bi, &K);
	ecc_point_add(K, peer_commit_element, ec_group_bi, &K);
	ECC_POINT_MUL(K, pSaeIns->sae_rand, ec_group_bi, &K);

	SAE_ECC_3D_to_2D(ec_group_bi, K);

	/* If K is point-at-infinity, reject. */
	if (K == NULL) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 " K should not be point-at-infinity\n");
		return FALSE;
	}

	/* k = F(K) (= x coordinate) */
	SAE_BN_BI2BIN_WITH_PAD(K->x, k, &len, pSaeIns->prime_len);
	hex_dump_with_lvl("k:", (char *)k, len, SAE_DEBUG_LEVEL2);
	ecc_point_free(&K);
	return TRUE;
}

UCHAR sae_derive_k_ffc(
	IN SAE_INSTANCE *pSaeIns,
	OUT UCHAR *k)
{
	SAE_BN *K = NULL;
	SAE_BN *tmp = NULL;
	SAE_BN *tmp2 = NULL;
	SAE_BN *pwe = NULL;
	UINT32 len = SAE_MAX_PRIME_LEN;
	UCHAR res = TRUE;

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_DEBUG,
			 "==> %s()\n", __func__);

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[6]);
	/* GET_BI_INS_FROM_POOL(tmp); */
	/* GET_BI_INS_FROM_POOL(tmp2); */
	/* GET_BI_INS_FROM_POOL(K); */

	/*
	 * K = scalar-op(rand, (elem-op(scalar-op(peer-commit-scalar, PWE),
	 *                                        PEER-COMMIT-ELEMENT)))
	 * the K should be scalar-op((rand + peer-rand) modulo r, pwe)
	 */
	pwe = (SAE_BN *) pSaeIns->pwe;
	SAE_BN_INIT(&K);
	SAE_BN_INIT(&tmp);
	SAE_BN_INIT(&tmp2);
	SAE_BN_MOD_EXP_MONT(pwe, pSaeIns->peer_commit_scalar, pSaeIns->prime, &tmp);
	SAE_BN_MOD_MUL(tmp, (SAE_BN *)pSaeIns->peer_commit_element, pSaeIns->prime, &tmp2);
	SAE_BN_MOD_EXP_MONT(tmp2, pSaeIns->sae_rand, pSaeIns->prime, &K);

	/* If K is identity element (one), reject. */
	if (SAE_BN_IS_ONE(K)) {
		SAE_BN_FREE(&K);
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 "K should not be one\n");
		res = FALSE;
		goto Free;
	}

	/* k = F(K) (= x coordinate) */
	SAE_BN_BI2BIN_WITH_PAD(K, k, &len, pSaeIns->prime_len);
	hex_dump_with_lvl("k:", (char *)k, len, SAE_DEBUG_LEVEL);
Free:
	SAE_BN_RELEASE_BACK_TO_POOL(&K);
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp);
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp2);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[6]);
	return TRUE;
}

USHORT sae_reflection_check_ecc(
	IN SAE_INSTANCE *pSaeIns)
{
	BIG_INTEGER_EC_POINT *own_commit_element = (BIG_INTEGER_EC_POINT *)pSaeIns->own_commit_element;
	BIG_INTEGER_EC_POINT *peer_commit_element = (BIG_INTEGER_EC_POINT *)pSaeIns->peer_commit_element;

	if (!pSaeIns->own_commit_scalar
		|| (SAE_BN_UCMP(pSaeIns->own_commit_scalar, pSaeIns->peer_commit_scalar) != 0)
		|| !own_commit_element
		|| (SAE_BN_UCMP(own_commit_element->x, peer_commit_element->x) != 0)
		|| (SAE_BN_UCMP(own_commit_element->y, peer_commit_element->y) != 0))
		return MLME_SUCCESS;
	else
		return SAE_SILENTLY_DISCARDED;
}

USHORT sae_reflection_check_ffc(
	IN SAE_INSTANCE *pSaeIns)
{
	if (!pSaeIns->own_commit_scalar
		|| (SAE_BN_UCMP(pSaeIns->own_commit_scalar, pSaeIns->peer_commit_scalar) != 0)
		|| !pSaeIns->own_commit_element
		|| (SAE_BN_UCMP(pSaeIns->own_commit_element, pSaeIns->peer_commit_element) != 0))
		return MLME_SUCCESS;
	else
		return SAE_SILENTLY_DISCARDED;
}
#endif /* DOT11_SAE_SUPPORT */
