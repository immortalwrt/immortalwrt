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
    SHA2

    Abstract:
    FIPS 180-2: Secure Hash Standard (SHS)

    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
    Eddy        2008/11/24      Create SHA1
    Eddy        2008/07/23      Create SHA256
***************************************************************************/

#ifndef __CRYPT_SHA2_H__
#define __CRYPT_SHA2_H__

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
#define SHA1_SUPPORT
#define SHA256_SUPPORT
#define SHA384_SUPPORT
#define SHA512_SUPPORT

#ifdef SHA1_SUPPORT
#define SHA1_BLOCK_SIZE    64	/* 512 bits = 64 bytes */
#define SHA1_DIGEST_SIZE   20	/* 160 bits = 20 bytes */
typedef struct _SHA1_CTX_STRUC {
	UINT32 HashValue[5];	/* 5 = (SHA1_DIGEST_SIZE / 32) */
	UINT64 MessageLen;	/* total size */
	UINT8 Block[SHA1_BLOCK_SIZE];
	UINT BlockLen;
} SHA1_CTX_STRUC, *PSHA1_CTX_STRUC;

VOID RT_SHA1_Init(
	IN SHA1_CTX_STRUC * pSHA_CTX);
VOID RT_SHA1_Hash(
	IN SHA1_CTX_STRUC * pSHA_CTX);
VOID RT_SHA1_Append(
	IN SHA1_CTX_STRUC * pSHA_CTX,
	IN const UINT8 Message[],
	IN UINT MessageLen);
VOID RT_SHA1_End(
	IN SHA1_CTX_STRUC * pSHA_CTX,
	OUT UINT8 DigestMessage[]);
VOID RT_SHA1(
	IN const UINT8 Message[],
	IN UINT MessageLen,
	OUT UINT8 DigestMessage[]);
#endif /* SHA1_SUPPORT */

#ifdef SHA256_SUPPORT
#define SHA256_BLOCK_SIZE   64	/* 512 bits = 64 bytes */
#define SHA256_DIGEST_SIZE  32	/* 256 bits = 32 bytes */
typedef struct _SHA256_CTX_STRUC {
	UINT32 HashValue[8];	/* 8 = (SHA256_DIGEST_SIZE / 32) */
	UINT64 MessageLen;	/* total size */
	UINT8 Block[SHA256_BLOCK_SIZE];
	UINT BlockLen;
} SHA256_CTX_STRUC, *PSHA256_CTX_STRUC;

VOID RT_SHA256_Init(
	IN SHA256_CTX_STRUC * pSHA_CTX);
VOID RT_SHA256_Hash(
	IN SHA256_CTX_STRUC * pSHA_CTX);
VOID RT_SHA256_Append(
	IN SHA256_CTX_STRUC * pSHA_CTX,
	IN const UINT8 Message[],
	IN UINT MessageLen);
VOID RT_SHA256_End(
	IN SHA256_CTX_STRUC * pSHA_CTX,
	OUT UINT8 DigestMessage[]);
VOID RT_SHA256(
	IN const UINT8 Message[],
	IN UINT MessageLen,
	OUT UINT8 DigestMessage[]);
VOID rt_sha256_vector(
	IN UCHAR num,
	IN const unsigned char **message,
	IN UINT *messageLen,
	OUT UINT8 *digestmessage);

#endif /* SHA256_SUPPORT */

#ifdef SHA384_SUPPORT
#define SHA384_BLOCK_SIZE   128	/* 1024 bits = 128 bytes */
#define SHA384_DIGEST_SIZE  48	/* 384 bits = 64 bytes */
typedef struct _SHA384_CTX_STRUC {
	UINT64 HashValue[8];	/* 8 = (SHA384_DIGEST_SIZE / 64) */
	UINT64 MessageLen;	/* total size */
	UINT8 Block[SHA384_BLOCK_SIZE];
	UINT BlockLen;
} SHA384_CTX_STRUC, *PSHA384_CTX_STRUC;
VOID RT_SHA384_Init(
	IN SHA384_CTX_STRUC * pSHA_CTX);
VOID RT_SHA384_Hash(
	IN SHA384_CTX_STRUC * pSHA_CTX);
VOID RT_SHA384_Append(
	IN SHA384_CTX_STRUC * pSHA_CTX,
	IN const UINT8 Message[],
	IN UINT MessageLen);
VOID RT_SHA384_End(
	IN SHA384_CTX_STRUC * pSHA_CTX,
	OUT UINT8 DigestMessage[]);
VOID RT_SHA384(
	IN const UINT8 Message[],
	IN UINT MessageLen,
	OUT UINT8 DigestMessage[]);
VOID rt_sha384_vector(
	IN UCHAR num,
	IN const unsigned char **message,
	IN UINT *messageLen,
	OUT UINT8 *digestmessage);
#endif /* SHA384_SUPPORT */

#ifdef SHA512_SUPPORT
#define SHA512_BLOCK_SIZE   128	/* 1024 bits = 128 bytes */
#define SHA512_DIGEST_SIZE  64	/* 384 bits = 64 bytes */
typedef struct _SHA512_CTX_STRUC {
	UINT64 HashValue[8];	/* 8 = (SHA512_DIGEST_SIZE / 64) */
	UINT64 MessageLen;	/* total size */
	UINT8 Block[SHA512_BLOCK_SIZE];
	UINT BlockLen;
} SHA512_CTX_STRUC;
VOID RT_SHA512_Init(
	IN SHA512_CTX_STRUC * pSHA_CTX);
VOID RT_SHA512_Hash(
	IN SHA512_CTX_STRUC * pSHA_CTX);
VOID RT_SHA512_Append(
	IN SHA512_CTX_STRUC * pSHA_CTX,
	IN const UINT8 Message[],
	IN UINT MessageLen);
VOID RT_SHA512_End(
	IN SHA512_CTX_STRUC * pSHA_CTX,
	OUT UINT8 DigestMessage[]);
VOID RT_SHA512(
	IN const UINT8 Message[],
	IN UINT MessageLen,
	OUT UINT8 DigestMessage[]);
VOID rt_sha512_vector(
	IN UCHAR num,
	IN const unsigned char **message,
	IN UINT *messageLen,
	OUT UINT8 *digestmessage);
#endif


#endif /* __CRYPT_SHA2_H__ */
