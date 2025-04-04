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
    Module Name:
    HMAC

    Abstract:
    FIPS 198: The Keyed-Hash Message Authentication Code (HMAC)

    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
    Eddy        2008/11/24      Create HMAC-SHA1, HMAC-SHA256
***************************************************************************/

#include "security/crypt_hmac.h"
#include "rt_config.h"


#ifdef SHA1_SUPPORT
/*
========================================================================
Routine Description:
    HMAC using SHA1 hash function

Arguments:
    key             Secret key
    key_len         The length of the key in bytes
    message         Message context
    message_len     The length of message in bytes
    macLen          Request the length of message authentication code

Return Value:
    mac             Message authentication code

Note:
    None
========================================================================
*/
VOID RT_HMAC_SHA1(
	IN  const UINT8 Key[],
	IN  UINT KeyLen,
	IN  const UINT8 Message[],
	IN  UINT MessageLen,
	OUT UINT8 MAC[],
	IN  UINT MACLen)
{
	SHA1_CTX_STRUC *sha_ctx1;
	SHA1_CTX_STRUC *sha_ctx2;
	UINT8 *K0;
	UINT8 *Digest;
	UINT index;

	os_alloc_mem(NULL, (UCHAR **)&sha_ctx1, sizeof(SHA1_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&sha_ctx2, sizeof(SHA1_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&K0, SHA1_BLOCK_SIZE);
	os_alloc_mem(NULL, (UCHAR **)&Digest, SHA1_DIGEST_SIZE);

	if (!sha_ctx1 || !sha_ctx2 || !K0 || !Digest) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory failed!");
		goto end;
	}

	NdisZeroMemory(sha_ctx1, sizeof(SHA1_CTX_STRUC));
	NdisZeroMemory(sha_ctx2, sizeof(SHA1_CTX_STRUC));
	/*
	 * If the length of K = B(Block size): K0 = K.
	 * If the length of K > B: hash K to obtain an L byte string,
	 * then append (B-L) zeros to create a B-byte string K0 (i.e., K0 = H(K) || 00...00).
	 * If the length of K < B: append zeros to the end of K to create a B-byte string K0
	 */
	NdisZeroMemory(K0, SHA1_BLOCK_SIZE);
	NdisZeroMemory(Digest, SHA1_DIGEST_SIZE);

	if (KeyLen <= SHA1_BLOCK_SIZE)
		NdisMoveMemory(K0, Key, KeyLen);
	else
		RT_SHA1(Key, KeyLen, K0);

	/* End of if */

	/* Exclusive-Or K0 with ipad */
	/* ipad: Inner pad; the byte x 36 repeated B times. */
	for (index = 0; index < SHA1_BLOCK_SIZE; index++)
		K0[index] ^= 0x36;

	/* End of for */
	RT_SHA1_Init(sha_ctx1);
	/* H(K0^ipad) */
	RT_SHA1_Append(sha_ctx1, K0, SHA1_BLOCK_SIZE);
	/* H((K0^ipad)||text) */
	RT_SHA1_Append(sha_ctx1, Message, MessageLen);
	RT_SHA1_End(sha_ctx1, Digest);

	/* Exclusive-Or K0 with opad and remove ipad */
	/* opad: Outer pad; the byte x 5c repeated B times. */
	for (index = 0; index < SHA1_BLOCK_SIZE; index++)
		K0[index] ^= 0x36^0x5c;

	/* End of for */
	RT_SHA1_Init(sha_ctx2);
	/* H(K0^opad) */
	RT_SHA1_Append(sha_ctx2, K0, SHA1_BLOCK_SIZE);
	/* H( (K0^opad) || H((K0^ipad)||text) ) */
	RT_SHA1_Append(sha_ctx2, Digest, SHA1_DIGEST_SIZE);
	RT_SHA1_End(sha_ctx2, Digest);

	if (MACLen > SHA1_DIGEST_SIZE)
		NdisMoveMemory(MAC, Digest, SHA1_DIGEST_SIZE);
	else
		NdisMoveMemory(MAC, Digest, MACLen);

end:
	if (sha_ctx1)
		os_free_mem(sha_ctx1);
	if (sha_ctx2)
		os_free_mem(sha_ctx2);
	if (K0)
		os_free_mem(K0);
	if (Digest)
		os_free_mem(Digest);
} /* End of RT_HMAC_SHA1 */
#endif /* SHA1_SUPPORT */


#ifdef SHA256_SUPPORT
/*
========================================================================
Routine Description:
    HMAC using SHA256 hash function

Arguments:
    key             Secret key
    key_len         The length of the key in bytes
    message         Message context
    message_len     The length of message in bytes
    macLen          Request the length of message authentication code

Return Value:
    mac             Message authentication code

Note:
    None
========================================================================
*/
VOID RT_HMAC_SHA256(
	IN  const UINT8 Key[],
	IN  UINT KeyLen,
	IN  const UINT8 Message[],
	IN  UINT MessageLen,
	OUT UINT8 MAC[],
	IN  UINT MACLen)
{
	SHA256_CTX_STRUC *sha_ctx1;
	SHA256_CTX_STRUC *sha_ctx2;
	UINT8 *K0;
	UINT8 *Digest;
	UINT index;

	os_alloc_mem(NULL, (UCHAR **)&sha_ctx1, sizeof(SHA256_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&sha_ctx2, sizeof(SHA256_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&K0, SHA256_BLOCK_SIZE);
	os_alloc_mem(NULL, (UCHAR **)&Digest, SHA256_DIGEST_SIZE);

	if (!sha_ctx1 || !sha_ctx2 || !K0 || !Digest) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory failed!");
		goto end;
	}

	NdisZeroMemory(sha_ctx1, sizeof(SHA256_CTX_STRUC));
	NdisZeroMemory(sha_ctx2, sizeof(SHA256_CTX_STRUC));
	/*
	 * If the length of K = B(Block size): K0 = K.
	 * If the length of K > B: hash K to obtain an L byte string,
	 * then append (B-L) zeros to create a B-byte string K0 (i.e., K0 = H(K) || 00...00).
	 * If the length of K < B: append zeros to the end of K to create a B-byte string K0
	 */
	NdisZeroMemory(K0, SHA256_BLOCK_SIZE);
	NdisZeroMemory(Digest, SHA256_DIGEST_SIZE);

	if (KeyLen <= SHA256_BLOCK_SIZE)
		NdisMoveMemory(K0, Key, KeyLen);
	else
		RT_SHA256(Key, KeyLen, K0);

	/* Exclusive-Or K0 with ipad */
	/* ipad: Inner pad; the byte x 36 repeated B times. */
	for (index = 0; index < SHA256_BLOCK_SIZE; index++)
		K0[index] ^= 0x36;

	/* End of for */
	RT_SHA256_Init(sha_ctx1);
	/* H(K0^ipad) */
	RT_SHA256_Append(sha_ctx1, K0, SHA256_BLOCK_SIZE);
	/* H((K0^ipad)||text) */
	RT_SHA256_Append(sha_ctx1, Message, MessageLen);
	RT_SHA256_End(sha_ctx1, Digest);

	/* Exclusive-Or K0 with opad and remove ipad */
	/* opad: Outer pad; the byte x 5c repeated B times. */
	for (index = 0; index < SHA256_BLOCK_SIZE; index++)
		K0[index] ^= 0x36^0x5c;

	/* End of for */
	RT_SHA256_Init(sha_ctx2);
	/* H(K0^opad) */
	RT_SHA256_Append(sha_ctx2, K0, SHA256_BLOCK_SIZE);
	/* H( (K0^opad) || H((K0^ipad)||text) ) */
	RT_SHA256_Append(sha_ctx2, Digest, SHA256_DIGEST_SIZE);
	RT_SHA256_End(sha_ctx2, Digest);

	if (MACLen > SHA256_DIGEST_SIZE)
		NdisMoveMemory(MAC, Digest, SHA256_DIGEST_SIZE);
	else
		NdisMoveMemory(MAC, Digest, MACLen);

end:
	if (sha_ctx1)
		os_free_mem(sha_ctx1);
	if (sha_ctx2)
		os_free_mem(sha_ctx2);
	if (K0)
		os_free_mem(K0);
	if (Digest)
		os_free_mem(Digest);
} /* End of RT_HMAC_SHA256 */

VOID RT_HMAC_SHA256_VECTOR(
	const UINT8 key[],
	IN UINT key_len,
	IN UCHAR element_num,
	IN const UINT8 *message[],
	IN UINT *message_len,
	OUT UINT8 mac[],
	IN UINT mac_len)
{
	SHA256_CTX_STRUC *sha_ctx1;
	SHA256_CTX_STRUC *sha_ctx2;
	UINT8 *K0;
	UINT8 *Digest;
	UINT index;
	const UCHAR *_addr[6];
	INT _len[6], i;

	os_alloc_mem(NULL, (UCHAR **)&sha_ctx1, sizeof(SHA256_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&sha_ctx2, sizeof(SHA256_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&K0, SHA256_BLOCK_SIZE);
	os_alloc_mem(NULL, (UCHAR **)&Digest, SHA256_DIGEST_SIZE);

	if (!sha_ctx1 || !sha_ctx2 || !K0 || !Digest) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory failed!");
		goto end;
	}

	NdisZeroMemory(sha_ctx1, sizeof(SHA256_CTX_STRUC));
	NdisZeroMemory(sha_ctx2, sizeof(SHA256_CTX_STRUC));
	/*
	 * If the length of K = B(Block size): K0 = K.
	 * If the length of K > B: hash K to obtain an L byte string,
	 * then append (B-L) zeros to create a B-byte string K0 (i.e., K0 = H(K) || 00...00).
	 * If the length of K < B: append zeros to the end of K to create a B-byte string K0
	 */
	NdisZeroMemory(K0, SHA256_BLOCK_SIZE);
	NdisZeroMemory(Digest, SHA256_DIGEST_SIZE);

	if (key_len <= SHA256_BLOCK_SIZE)
		NdisMoveMemory(K0, key, key_len);
	else
		RT_SHA256(key, key_len, K0);

	/* Exclusive-Or K0 with ipad */
	/* ipad: Inner pad; the byte x??36?? repeated B times. */
	for (index = 0; index < SHA256_BLOCK_SIZE; index++)
		K0[index] ^= 0x36;

	/* End of for */
	RT_SHA256_Init(sha_ctx1);
	/* H(K0^ipad) */

	_addr[0] = K0;
	_len[0] = SHA256_BLOCK_SIZE;

	for (i = 0; i < element_num; i++) {
		_addr[i + 1] = message[i];
		_len[i + 1] = message_len[i];
	}

	rt_sha256_vector(element_num + 1, _addr, _len, Digest);
	/* Exclusive-Or K0 with opad and remove ipad */
	/* opad: Outer pad; the byte x??5c?? repeated B times. */
	for (index = 0; index < SHA256_BLOCK_SIZE; index++)
		K0[index] ^= 0x36^0x5c;

	/* End of for */
	RT_SHA256_Init(sha_ctx2);
	/* H(K0^opad) */
	RT_SHA256_Append(sha_ctx2, K0, SHA256_BLOCK_SIZE);
	/* H( (K0^opad) || H((K0^ipad)||text) ) */
	RT_SHA256_Append(sha_ctx2, Digest, SHA256_DIGEST_SIZE);
	RT_SHA256_End(sha_ctx2, Digest);

	if (mac_len > SHA256_DIGEST_SIZE)
		NdisMoveMemory(mac, Digest, SHA256_DIGEST_SIZE);
	else
		NdisMoveMemory(mac, Digest, mac_len);

end:
	if (sha_ctx1)
		os_free_mem(sha_ctx1);
	if (sha_ctx2)
		os_free_mem(sha_ctx2);
	if (K0)
		os_free_mem(K0);
	if (Digest)
		os_free_mem(Digest);
}

#endif /* SHA256_SUPPORT */

#ifdef SHA384_SUPPORT
/*
========================================================================
Routine Description:
    HMAC using SHA256 hash function

Arguments:
    key             Secret key
    key_len         The length of the key in bytes
    message         Message context
    message_len     The length of message in bytes
    macLen          Request the length of message authentication code

Return Value:
    mac             Message authentication code

Note:
    None
========================================================================
*/
VOID RT_HMAC_SHA384(
	IN  const UINT8 Key[],
	IN  UINT KeyLen,
	IN  const UINT8 Message[],
	IN  UINT MessageLen,
	OUT UINT8 MAC[],
	IN  UINT MACLen)
{
	SHA384_CTX_STRUC *sha_ctx1;
	SHA384_CTX_STRUC *sha_ctx2;
	UINT8 *K0;
	UINT8 *Digest;
	UINT index;

	os_alloc_mem(NULL, (UCHAR **)&sha_ctx1, sizeof(SHA384_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&sha_ctx2, sizeof(SHA384_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&K0, SHA384_BLOCK_SIZE);
	os_alloc_mem(NULL, (UCHAR **)&Digest, SHA384_DIGEST_SIZE);

	if (!sha_ctx1 || !sha_ctx2 || !K0 || !Digest) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory failed!");
		goto end;
	}

	NdisZeroMemory(sha_ctx1, sizeof(SHA384_CTX_STRUC));
	NdisZeroMemory(sha_ctx2, sizeof(SHA384_CTX_STRUC));
	/*
	 * If the length of K = B(Block size): K0 = K.
	 * If the length of K > B: hash K to obtain an L byte string,
	 * then append (B-L) zeros to create a B-byte string K0 (i.e., K0 = H(K) || 00...00).
	 * If the length of K < B: append zeros to the end of K to create a B-byte string K0
	 */
	NdisZeroMemory(K0, SHA384_BLOCK_SIZE);
	NdisZeroMemory(Digest, SHA384_DIGEST_SIZE);

	if (KeyLen <= SHA384_BLOCK_SIZE)
		NdisMoveMemory(K0, Key, KeyLen);
	else
		RT_SHA384(Key, KeyLen, K0);

	/* Exclusive-Or K0 with ipad */
	/* ipad: Inner pad; the byte x 36 repeated B times. */
	for (index = 0; index < SHA384_BLOCK_SIZE; index++)
		K0[index] ^= 0x36;

	/* End of for */
	RT_SHA384_Init(sha_ctx1);
	/* H(K0^ipad) */
	RT_SHA384_Append(sha_ctx1, K0, SHA384_BLOCK_SIZE);
	/* H((K0^ipad)||text) */
	RT_SHA384_Append(sha_ctx1, Message, MessageLen);
	RT_SHA384_End(sha_ctx1, Digest);

	/* Exclusive-Or K0 with opad and remove ipad */
	/* opad: Outer pad; the byte x 5c repeated B times. */
	for (index = 0; index < SHA384_BLOCK_SIZE; index++)
		K0[index] ^= 0x36^0x5c;

	/* End of for */
	RT_SHA384_Init(sha_ctx2);
	/* H(K0^opad) */
	RT_SHA384_Append(sha_ctx2, K0, SHA384_BLOCK_SIZE);
	/* H( (K0^opad) || H((K0^ipad)||text) ) */
	RT_SHA384_Append(sha_ctx2, Digest, SHA384_DIGEST_SIZE);
	RT_SHA384_End(sha_ctx2, Digest);

	if (MACLen > SHA384_DIGEST_SIZE)
		NdisMoveMemory(MAC, Digest, SHA384_DIGEST_SIZE);
	else
		NdisMoveMemory(MAC, Digest, MACLen);

end:
	if (sha_ctx1)
		os_free_mem(sha_ctx1);
	if (sha_ctx2)
		os_free_mem(sha_ctx2);
	if (K0)
		os_free_mem(K0);
	if (Digest)
		os_free_mem(Digest);
} /* End of RT_HMAC_SHA384 */

VOID RT_HMAC_SHA384_VECTOR(
	const UINT8 key[],
	IN UINT key_len,
	IN UCHAR element_num,
	IN const UINT8 *message[],
	IN UINT *message_len,
	OUT UINT8 mac[],
	IN UINT mac_len)
{
	SHA384_CTX_STRUC *sha_ctx1;
	SHA384_CTX_STRUC *sha_ctx2;
	UINT8 *K0;
	UINT8 *Digest;
	UINT index;
	const UCHAR *_addr[6];
	INT _len[6], i;

	os_alloc_mem(NULL, (UCHAR **)&sha_ctx1, sizeof(SHA384_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&sha_ctx2, sizeof(SHA384_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&K0, SHA384_BLOCK_SIZE);
	os_alloc_mem(NULL, (UCHAR **)&Digest, SHA384_DIGEST_SIZE);

	if (!sha_ctx1 || !sha_ctx2 || !K0 || !Digest) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory failed!");
		goto end;
	}

	NdisZeroMemory(sha_ctx1, sizeof(SHA384_CTX_STRUC));
	NdisZeroMemory(sha_ctx2, sizeof(SHA384_CTX_STRUC));
	/*
	 * If the length of K = B(Block size): K0 = K.
	 * If the length of K > B: hash K to obtain an L byte string,
	 * then append (B-L) zeros to create a B-byte string K0 (i.e., K0 = H(K) || 00...00).
	 * If the length of K < B: append zeros to the end of K to create a B-byte string K0
	 */
	NdisZeroMemory(K0, SHA384_BLOCK_SIZE);
	NdisZeroMemory(Digest, SHA384_DIGEST_SIZE);

	if (key_len <= SHA384_BLOCK_SIZE)
		NdisMoveMemory(K0, key, key_len);
	else
		RT_SHA384(key, key_len, K0);

	/* Exclusive-Or K0 with ipad */
	/* ipad: Inner pad; the byte x??36?? repeated B times. */
	for (index = 0; index < SHA384_BLOCK_SIZE; index++)
		K0[index] ^= 0x36;

	/* End of for */
	RT_SHA384_Init(sha_ctx1);
	/* H(K0^ipad) */

	_addr[0] = K0;
	_len[0] = SHA384_BLOCK_SIZE;

	for (i = 0; i < element_num; i++) {
		_addr[i + 1] = message[i];
		_len[i + 1] = message_len[i];
	}

	rt_sha384_vector(element_num + 1, _addr, _len, Digest);
	/* Exclusive-Or K0 with opad and remove ipad */
	/* opad: Outer pad; the byte x??5c?? repeated B times. */
	for (index = 0; index < SHA384_BLOCK_SIZE; index++)
		K0[index] ^= 0x36^0x5c;

	/* End of for */
	RT_SHA384_Init(sha_ctx2);
	/* H(K0^opad) */
	RT_SHA384_Append(sha_ctx2, K0, SHA384_BLOCK_SIZE);
	/* H( (K0^opad) || H((K0^ipad)||text) ) */
	RT_SHA384_Append(sha_ctx2, Digest, SHA384_DIGEST_SIZE);
	RT_SHA384_End(sha_ctx2, Digest);

	if (mac_len > SHA384_DIGEST_SIZE)
		NdisMoveMemory(mac, Digest, SHA384_DIGEST_SIZE);
	else
		NdisMoveMemory(mac, Digest, mac_len);

end:
	if (sha_ctx1)
		os_free_mem(sha_ctx1);
	if (sha_ctx2)
		os_free_mem(sha_ctx2);
	if (K0)
		os_free_mem(K0);
	if (Digest)
		os_free_mem(Digest);
}

#endif /* SHA384_SUPPORT */

#ifdef SHA512_SUPPORT
/*
========================================================================
Routine Description:
    HMAC using SHA256 hash function

Arguments:
    key             Secret key
    key_len         The length of the key in bytes
    message         Message context
    message_len     The length of message in bytes
    macLen          Request the length of message authentication code

Return Value:
    mac             Message authentication code

Note:
    None
========================================================================
*/
VOID RT_HMAC_SHA512(
	IN  const UINT8 Key[],
	IN  UINT KeyLen,
	IN  const UINT8 Message[],
	IN  UINT MessageLen,
	OUT UINT8 MAC[],
	IN  UINT MACLen)
{
	SHA512_CTX_STRUC *sha_ctx1;
	SHA512_CTX_STRUC *sha_ctx2;
	UINT8 *K0;
	UINT8 *Digest;
	UINT index;

	os_alloc_mem(NULL, (UCHAR **)&sha_ctx1, sizeof(SHA512_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&sha_ctx2, sizeof(SHA512_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&K0, SHA512_BLOCK_SIZE);
	os_alloc_mem(NULL, (UCHAR **)&Digest, SHA512_DIGEST_SIZE);

	if (!sha_ctx1 || !sha_ctx2 || !K0 || !Digest) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory failed!");
		goto end;
	}

	NdisZeroMemory(sha_ctx1, sizeof(SHA512_CTX_STRUC));
	NdisZeroMemory(sha_ctx2, sizeof(SHA512_CTX_STRUC));
	/*
	 * If the length of K = B(Block size): K0 = K.
	 * If the length of K > B: hash K to obtain an L byte string,
	 * then append (B-L) zeros to create a B-byte string K0 (i.e., K0 = H(K) || 00...00).
	 * If the length of K < B: append zeros to the end of K to create a B-byte string K0
	 */
	NdisZeroMemory(K0, SHA512_BLOCK_SIZE);
	NdisZeroMemory(Digest, SHA512_DIGEST_SIZE);

	if (KeyLen <= SHA512_BLOCK_SIZE)
		NdisMoveMemory(K0, Key, KeyLen);
	else
		RT_SHA512(Key, KeyLen, K0);

	/* Exclusive-Or K0 with ipad */
	/* ipad: Inner pad; the byte x 36 repeated B times. */
	for (index = 0; index < SHA512_BLOCK_SIZE; index++)
		K0[index] ^= 0x36;

	/* End of for */
	RT_SHA512_Init(sha_ctx1);
	/* H(K0^ipad) */
	RT_SHA512_Append(sha_ctx1, K0, SHA512_BLOCK_SIZE);
	/* H((K0^ipad)||text) */
	RT_SHA512_Append(sha_ctx1, Message, MessageLen);
	RT_SHA512_End(sha_ctx1, Digest);

	/* Exclusive-Or K0 with opad and remove ipad */
	/* opad: Outer pad; the byte x 5c repeated B times. */
	for (index = 0; index < SHA512_BLOCK_SIZE; index++)
		K0[index] ^= 0x36^0x5c;

	/* End of for */
	RT_SHA512_Init(sha_ctx2);
	/* H(K0^opad) */
	RT_SHA512_Append(sha_ctx2, K0, SHA512_BLOCK_SIZE);
	/* H( (K0^opad) || H((K0^ipad)||text) ) */
	RT_SHA512_Append(sha_ctx2, Digest, SHA512_DIGEST_SIZE);
	RT_SHA512_End(sha_ctx2, Digest);

	if (MACLen > SHA512_DIGEST_SIZE)
		NdisMoveMemory(MAC, Digest, SHA512_DIGEST_SIZE);
	else
		NdisMoveMemory(MAC, Digest, MACLen);

end:
	if (sha_ctx1)
		os_free_mem(sha_ctx1);
	if (sha_ctx2)
		os_free_mem(sha_ctx2);
	if (K0)
		os_free_mem(K0);
	if (Digest)
		os_free_mem(Digest);

} /* End of RT_HMAC_SHA512 */

VOID RT_HMAC_SHA512_VECTOR(
	const UINT8 key[],
	IN UINT key_len,
	IN UCHAR element_num,
	IN const UINT8 *message[],
	IN UINT *message_len,
	OUT UINT8 mac[],
	IN UINT mac_len)
{
	SHA512_CTX_STRUC *sha_ctx1;
	SHA512_CTX_STRUC *sha_ctx2;
	UINT8 *K0;
	UINT8 *Digest;
	UINT index;
	const UCHAR *_addr[6];
	INT _len[6], i;

	os_alloc_mem(NULL, (UCHAR **)&sha_ctx1, sizeof(SHA512_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&sha_ctx2, sizeof(SHA512_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&K0, SHA512_BLOCK_SIZE);
	os_alloc_mem(NULL, (UCHAR **)&Digest, SHA512_DIGEST_SIZE);

	if (!sha_ctx1 || !sha_ctx2 || !K0 || !Digest) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory failed!");
		goto end;
	}

	NdisZeroMemory(sha_ctx1, sizeof(SHA512_CTX_STRUC));
	NdisZeroMemory(sha_ctx2, sizeof(SHA512_CTX_STRUC));
	/*
	 * If the length of K = B(Block size): K0 = K.
	 * If the length of K > B: hash K to obtain an L byte string,
	 * then append (B-L) zeros to create a B-byte string K0 (i.e., K0 = H(K) || 00...00).
	 * If the length of K < B: append zeros to the end of K to create a B-byte string K0
	 */
	NdisZeroMemory(K0, SHA512_BLOCK_SIZE);
	NdisZeroMemory(Digest, SHA512_DIGEST_SIZE);

	if (key_len <= SHA512_BLOCK_SIZE)
		NdisMoveMemory(K0, key, key_len);
	else
		RT_SHA512(key, key_len, K0);

	/* Exclusive-Or K0 with ipad */
	/* ipad: Inner pad; the byte x??36?? repeated B times. */
	for (index = 0; index < SHA512_BLOCK_SIZE; index++)
		K0[index] ^= 0x36;

	/* End of for */
	RT_SHA512_Init(sha_ctx1);
	/* H(K0^ipad) */

	_addr[0] = K0;
	_len[0] = SHA512_BLOCK_SIZE;

	for (i = 0; i < element_num; i++) {
		_addr[i + 1] = message[i];
		_len[i + 1] = message_len[i];
	}

	rt_sha512_vector(element_num + 1, _addr, _len, Digest);
	/* Exclusive-Or K0 with opad and remove ipad */
	/* opad: Outer pad; the byte x??5c?? repeated B times. */
	for (index = 0; index < SHA512_BLOCK_SIZE; index++)
		K0[index] ^= 0x36^0x5c;

	/* End of for */
	RT_SHA512_Init(sha_ctx2);
	/* H(K0^opad) */
	RT_SHA512_Append(sha_ctx2, K0, SHA512_BLOCK_SIZE);
	/* H( (K0^opad) || H((K0^ipad)||text) ) */
	RT_SHA512_Append(sha_ctx2, Digest, SHA512_DIGEST_SIZE);
	RT_SHA512_End(sha_ctx2, Digest);

	if (mac_len > SHA512_DIGEST_SIZE)
		NdisMoveMemory(mac, Digest, SHA512_DIGEST_SIZE);
	else
		NdisMoveMemory(mac, Digest, mac_len);

end:
	if (sha_ctx1)
		os_free_mem(sha_ctx1);
	if (sha_ctx2)
		os_free_mem(sha_ctx2);
	if (K0)
		os_free_mem(K0);
	if (Digest)
		os_free_mem(Digest);
}

#endif /* SHA512_SUPPORT */

#ifdef MD5_SUPPORT
/*
========================================================================
Routine Description:
    HMAC using MD5 hash function

Arguments:
    key             Secret key
    key_len         The length of the key in bytes
    message         Message context
    message_len     The length of message in bytes
    macLen          Request the length of message authentication code

Return Value:
    mac             Message authentication code

Note:
    None
========================================================================
*/
VOID RT_HMAC_MD5(
	IN  const UINT8 Key[],
	IN  UINT KeyLen,
	IN  const UINT8 Message[],
	IN  UINT MessageLen,
	OUT UINT8 MAC[],
	IN  UINT MACLen)
{
	MD5_CTX_STRUC *md5_ctx1;
	MD5_CTX_STRUC *md5_ctx2;
	UINT8 *K0;
	UINT8 *Digest;
	UINT index;

	os_alloc_mem(NULL, (UCHAR **)&md5_ctx1, sizeof(MD5_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&md5_ctx2, sizeof(MD5_CTX_STRUC));
	os_alloc_mem(NULL, (UCHAR **)&K0, MD5_BLOCK_SIZE);
	os_alloc_mem(NULL, (UCHAR **)&Digest, MD5_DIGEST_SIZE);

	if (!md5_ctx1 || !md5_ctx2 || !K0 || !Digest) {
		MTWF_DBG(NULL, DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, "Allocate memory failed!");
		goto end;
	}

	NdisZeroMemory(md5_ctx1, sizeof(MD5_CTX_STRUC));
	NdisZeroMemory(md5_ctx2, sizeof(MD5_CTX_STRUC));
	/*
	 * If the length of K = B(Block size): K0 = K.
	 * If the length of K > B: hash K to obtain an L byte string,
	 * then append (B-L) zeros to create a B-byte string K0 (i.e., K0 = H(K) || 00...00).
	 * If the length of K < B: append zeros to the end of K to create a B-byte string K0
	 */
	NdisZeroMemory(K0, MD5_BLOCK_SIZE);
	NdisZeroMemory(Digest, MD5_DIGEST_SIZE);

	if (KeyLen <= MD5_BLOCK_SIZE)
		NdisMoveMemory(K0, Key, KeyLen);
	else
		RT_MD5(Key, KeyLen, K0);

	/* Exclusive-Or K0 with ipad */
	/* ipad: Inner pad; the byte x 36 repeated B times. */
	for (index = 0; index < MD5_BLOCK_SIZE; index++)
		K0[index] ^= 0x36;

	/* End of for */
	RT_MD5_Init(md5_ctx1);
	/* H(K0^ipad) */
	RT_MD5_Append(md5_ctx1, K0, MD5_BLOCK_SIZE);
	/* H((K0^ipad)||text) */
	RT_MD5_Append(md5_ctx1, Message, MessageLen);
	RT_MD5_End(md5_ctx1, Digest);

	/* Exclusive-Or K0 with opad and remove ipad */
	/* opad: Outer pad; the byte x 5c repeated B times. */
	for (index = 0; index < MD5_BLOCK_SIZE; index++)
		K0[index] ^= 0x36^0x5c;

	/* End of for */
	RT_MD5_Init(md5_ctx2);
	/* H(K0^opad) */
	RT_MD5_Append(md5_ctx2, K0, MD5_BLOCK_SIZE);
	/* H( (K0^opad) || H((K0^ipad)||text) ) */
	RT_MD5_Append(md5_ctx2, Digest, MD5_DIGEST_SIZE);
	RT_MD5_End(md5_ctx2, Digest);

	if (MACLen > MD5_DIGEST_SIZE)
		NdisMoveMemory(MAC, Digest, MD5_DIGEST_SIZE);
	else
		NdisMoveMemory(MAC, Digest, MACLen);

end:
	if (md5_ctx1)
		os_free_mem(md5_ctx1);
	if (md5_ctx2)
		os_free_mem(md5_ctx2);
	if (K0)
		os_free_mem(K0);
	if (Digest)
		os_free_mem(Digest);
} /* End of RT_HMAC_SHA256 */
#endif /* MD5_SUPPORT */


/* End of crypt_hmac.c */

