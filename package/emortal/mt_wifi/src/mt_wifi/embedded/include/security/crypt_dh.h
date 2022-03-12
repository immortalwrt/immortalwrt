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
    DH

    Abstract:
    RFC 2631: Diffie-Hellman Key Agreement Method

    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
    Eddy        2009/01/21      Create Diffie-Hellman, Montgomery Algorithm
***************************************************************************/

#ifndef __CRYPT_DH_H__
#define __CRYPT_DH_H__

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


/* DH operations */
void DH_PublicKey_Generate(
	IN UINT8 GValue[],
	IN UINT GValueLength,
	IN UINT8 PValue[],
	IN UINT PValueLength,
	IN UINT8 PrivateKey[],
	IN UINT PrivateKeyLength,
	OUT UINT8 PublicKey[],
	INOUT UINT * PublicKeyLength);

void DH_SecretKey_Generate(
	IN UINT8 PublicKey[],
	IN UINT PublicKeyLength,
	IN UINT8 PValue[],
	IN UINT PValueLength,
	IN UINT8 PrivateKey[],
	IN UINT PrivateKeyLength,
	OUT UINT8 SecretKey[],
	INOUT UINT * SecretKeyLength);

#define RT_DH_PublicKey_Generate(GK, GKL, PV, PVL, PriK, PriKL, PubK, PubKL) \
	DH_PublicKey_Generate((GK), (GKL), (PV), (PVL), (PriK), (PriKL), (UINT8 *) (PubK), (UINT *) (PubKL))

#define RT_DH_SecretKey_Generate(PubK, PubKL, PV, PVL, PriK, PriKL, SecK, SecKL) \
	DH_SecretKey_Generate((PubK), (PubKL), (PV), (PVL), (PriK), (PriKL), (UINT8 *) (SecK), (UINT *) (SecKL))

#define RT_DH_FREE_ALL()


#endif /* __CRYPT_DH_H__ */

