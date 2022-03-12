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
    MD5

    Abstract:
    RFC1321: The MD5 Message-Digest Algorithm

    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
    Eddy        2008/11/24      Create md5
***************************************************************************/

#ifndef __CRYPT_MD5_H__
#define __CRYPT_MD5_H__

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


/* Algorithm options */
#define MD5_SUPPORT

#ifdef MD5_SUPPORT
#define MD5_BLOCK_SIZE    64	/* 512 bits = 64 bytes */
#define MD5_DIGEST_SIZE   16	/* 128 bits = 16 bytes */
typedef struct {
	UINT32 HashValue[4];
	UINT64 MessageLen;
	UINT8 Block[MD5_BLOCK_SIZE];
	UINT BlockLen;
} MD5_CTX_STRUC, *PMD5_CTX_STRUC;

VOID RT_MD5_Init(
	IN MD5_CTX_STRUC * pMD5_CTX);
VOID RT_MD5_Hash(
	IN MD5_CTX_STRUC * pMD5_CTX);
VOID RT_MD5_Append(
	IN MD5_CTX_STRUC * pMD5_CTX,
	IN const UINT8 Message[],
	IN UINT MessageLen);
VOID RT_MD5_End(
	IN MD5_CTX_STRUC * pMD5_CTX,
	OUT UINT8 DigestMessage[]);
VOID RT_MD5(
	IN const UINT8 Message[],
	IN UINT MessageLen,
	OUT UINT8 DigestMessage[]);
#endif /* MD5_SUPPORT */


#endif /* __CRYPT_MD5_H__ */
