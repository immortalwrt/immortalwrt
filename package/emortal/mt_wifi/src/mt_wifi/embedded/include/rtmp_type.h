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
    rtmp_type.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Paul Lin    1-2-2004
*/

#ifndef __RTMP_TYPE_H__
#define __RTMP_TYPE_H__



#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */


#ifdef LINUX
/* Put platform dependent declaration here */
/* For example, linux type definition */
typedef signed char INT8;
typedef unsigned char UINT8;
typedef signed short INT16;
typedef unsigned short UINT16;
typedef signed int INT32;
typedef unsigned int UINT32;
typedef signed long long INT64;
typedef unsigned long long UINT64;

typedef signed char CHAR;
typedef unsigned char UCHAR;
typedef signed short SHORT;
typedef unsigned short USHORT;
typedef signed int INT;
typedef unsigned int UINT;
typedef signed long LONG;
typedef unsigned long ULONG;
#endif /* LINUX */

typedef signed char *PINT8;
typedef unsigned char *PUINT8;
typedef signed short *PINT16;
typedef unsigned short *PUINT16;
typedef signed int *PINT;
typedef unsigned int *PUINT;
typedef signed int *PINT32;
typedef unsigned int *PUINT32;
typedef signed long long *PINT64;
typedef signed long long LONGLONG;
typedef unsigned long long *PUINT64;
typedef unsigned long long ULONGLONG;

/* modified for fixing compile warning on Sigma 8634 platform */
/* typedef char STRING; */
typedef char RTMP_STRING;

typedef signed char CHAR;

typedef unsigned char BOOLEAN;

#ifdef LINUX
typedef void VOID;
#endif /* LINUX */

#if (defined(MWDS) || defined(WH_EVENT_NOTIFIER))
typedef char *PSTRING;
#endif
typedef VOID * PVOID;
typedef CHAR * PCHAR;
typedef UCHAR * PUCHAR;
typedef USHORT * PUSHORT;
typedef LONG * PLONG;
typedef ULONG * PULONG;
typedef UINT * PUINT;

typedef unsigned int NDIS_MEDIA_STATE;

typedef union _LARGE_INTEGER {
	struct {
#ifdef RT_BIG_ENDIAN
		INT32 HighPart;
		UINT LowPart;
#else
		UINT LowPart;
		INT32 HighPart;
#endif
	} u;
	INT64 QuadPart;
} LARGE_INTEGER;


/* Register set pair for initialzation register set definition */
typedef struct _RTMP_REG_PAIR {
	UINT32 Register;
	UINT32 Value;
} RTMP_REG_PAIR, *PRTMP_REG_PAIR;

typedef struct _RTMP_MIB_PAIR {
	UINT32 Counter;
	UINT64 Value;
} RTMP_MIB_PAIR, *PRTMP_MIB_PAIR;

typedef struct _REG_CHK_PAIR {
	UINT32 Register;
	UINT32 Mask;
	UINT32 Value;
} REG_CHK_PAIR;

typedef struct _REG_PAIR_CHANNEL {
	UCHAR Register;
	UCHAR FirstChannel;
	UCHAR LastChannel;
	UCHAR Value;
} REG_PAIR_CHANNEL, *PREG_PAIR_CHANNEL;

typedef struct _REG_PAIR_BW {
	UCHAR Register;
	UCHAR BW;
	UCHAR Value;
} REG_PAIR_BW, *PREG_PAIR_BW;


typedef struct _REG_PAIR_PHY {
	UCHAR reg;
	UCHAR s_ch;
	UCHAR e_ch;
	UCHAR phy;	/* RF_MODE_XXX */
	UCHAR bw;	/* RF_BW_XX */
	UCHAR val;
} REG_PAIR_PHY;


/* Register set pair for initialzation register set definition */
typedef struct _RTMP_RF_REGS {
	UCHAR Channel;
	UINT32 R1;
	UINT32 R2;
	UINT32 R3;
	UINT32 R4;
} RTMP_RF_REGS, *PRTMP_RF_REGS;

typedef struct _FREQUENCY_ITEM {
	UCHAR Channel;
	UCHAR N;
	UCHAR R;
	UCHAR K;
} FREQUENCY_ITEM, *PFREQUENCY_ITEM;

typedef int NTSTATUS;

#define STATUS_SUCCESS			0x00
#define STATUS_UNSUCCESSFUL	0x01

typedef struct _QUEUE_ENTRY {
	struct _QUEUE_ENTRY *Next;
} QUEUE_ENTRY, *PQUEUE_ENTRY;

/* Queue structure */
typedef struct _QUEUE_HEADER {
	PQUEUE_ENTRY Head;
	PQUEUE_ENTRY Tail;
	UINT Number;
	ULONG state;
} QUEUE_HEADER, *PQUEUE_HEADER;

typedef struct _CR_REG {
	UINT32 flags;
	UINT32 offset;
	UINT32 value;
} CR_REG, *PCR_REG;

typedef struct _BANK_RF_CR_REG {
	UINT32 flags;
	UCHAR bank;
	UCHAR offset;
	UCHAR value;
} BANK_RF_CR_REG, *PBANK_RF_CR_REG;


typedef struct _BANK_RF_REG_PAIR {
	UCHAR Bank;
	UCHAR Register;
	UCHAR Value;
} BANK_RF_REG_PAIR, *PBANK_RF_REG_PAIR;

typedef struct _R_M_W_REG {
	UINT32 Register;
	UINT32 ClearBitMask;
	UINT32 Value;
} R_M_W_REG, *PR_M_W_REG;

typedef struct _RF_R_M_W_REG {
	UCHAR Bank;
	UCHAR Register;
	UCHAR ClearBitMask;
	UCHAR Value;
} RF_R_M_W_REG, *PRF_R_M_W_REG;

struct mt_dev_priv {
	void *sys_handle;
	void *wifi_dev;
	unsigned long priv_flags;
	UCHAR sniffer_mode;
};

#endif /* __RTMP_TYPE_H__ */

