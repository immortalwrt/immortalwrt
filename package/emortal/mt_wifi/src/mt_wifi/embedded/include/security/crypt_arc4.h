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
    RC4

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
    Eddy        2009/05/13      ARC4
***************************************************************************/

#ifndef __CRYPT_ARC4_H__
#define __CRYPT_ARC4_H__

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

/* ARC4 definition & structure */
#define ARC4_KEY_BLOCK_SIZE 256

typedef struct {
	UINT BlockIndex1;
	UINT BlockIndex2;
	UINT8 KeyBlock[256];
} ARC4_CTX_STRUC, *PARC4_CTX_STRUC;

/* ARC4 operations */
VOID ARC4_INIT(
	IN ARC4_CTX_STRUC * pARC4_CTX,
	IN PUCHAR pKey,
	IN UINT KeyLength);

VOID ARC4_Compute(
	IN ARC4_CTX_STRUC * pARC4_CTX,
	IN UINT8 InputBlock[],
	IN UINT InputBlockSize,
	OUT UINT8 OutputBlock[]);

VOID ARC4_Discard_KeyLength(
	IN ARC4_CTX_STRUC * pARC4_CTX,
	IN UINT Length);

#endif /* __CRYPT_ARC4_H__ */
