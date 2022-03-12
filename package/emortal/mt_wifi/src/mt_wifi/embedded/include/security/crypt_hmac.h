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

#ifndef __CRYPT_HMAC_H__
#define __CRYPT_HMAC_H__

#include "rtmp_type.h"
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef INOUT
#define INOUT
#endif


#ifdef SHA1_SUPPORT
VOID RT_HMAC_SHA1(
	IN const UINT8 Key[],
	IN UINT KeyLen,
	IN const UINT8 Message[],
	IN UINT MessageLen,
	OUT UINT8 MAC[],
	IN UINT MACLen);
#endif /* SHA1_SUPPORT */

#ifdef SHA256_SUPPORT
VOID RT_HMAC_SHA256(
	IN const UINT8 Key[],
	IN UINT KeyLen,
	IN const UINT8 Message[],
	IN UINT MessageLen,
	OUT UINT8 MAC[],
	IN UINT MACLen);

VOID RT_HMAC_SHA256_VECTOR(
	IN const UINT8 key[],
	IN UINT key_len,
	IN UCHAR element_num,
	IN const UINT8 *message[],
	IN UINT *message_len,
	OUT UINT8 mac[],
	IN UINT mac_len);
#endif /* SHA256_SUPPORT */

#ifdef SHA384_SUPPORT
VOID RT_HMAC_SHA384(
	IN const UINT8 Key[],
	IN UINT KeyLen,
	IN const UINT8 Message[],
	IN UINT MessageLen,
	OUT UINT8 MAC[],
	IN UINT MACLen);

VOID RT_HMAC_SHA384_VECTOR(
	IN const UINT8 key[],
	IN UINT key_len,
	IN UCHAR element_num,
	IN const UINT8 *message[],
	IN UINT *message_len,
	OUT UINT8 mac[],
	IN UINT mac_len);
#endif /* SHA384_SUPPORT */

#ifdef SHA512_SUPPORT
VOID RT_HMAC_SHA512(
	IN const UINT8 Key[],
	IN UINT KeyLen,
	IN const UINT8 Message[],
	IN UINT MessageLen,
	OUT UINT8 MAC[],
	IN UINT MACLen);

VOID RT_HMAC_SHA512_VECTOR(
	IN const UINT8 key[],
	IN UINT key_len,
	IN UCHAR element_num,
	IN const UINT8 *message[],
	IN UINT *message_len,
	OUT UINT8 mac[],
	IN UINT mac_len);
#endif /* SHA512_SUPPORT */

#ifdef MD5_SUPPORT
VOID RT_HMAC_MD5(
	IN const UINT8 Key[],
	IN UINT KeyLen,
	IN const UINT8 Message[],
	IN UINT MessageLen,
	OUT UINT8 MAC[],
	IN UINT MACLen);
#endif /* MD5_SUPPORT */


#endif /* __CRYPT_HMAC_H__ */
