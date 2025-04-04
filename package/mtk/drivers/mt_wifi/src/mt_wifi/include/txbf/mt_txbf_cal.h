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
	mt_txbf_cal.h
*/


#ifndef _RT_TXBF_CAL_H_
#define _RT_TXBF_CAL_H_

#define TX_PATH_2   2
#define TX_PATH_3   3
#define TX_PATH_4   4

#define GROUP_0     0
#define CH_001      1
#define CH_008      8
#define CH_014      14
#define GROUP_1     1
#define CH_184      184
#define CH_196      196
#define GROUP_2     2
#define CH_036      36
#define CH_044      44
#define GROUP_3     3
#define CH_052      52
#define CH_060      60
#define CH_068      68
#define GROUP_4     4
#define CH_072      72
#define CH_084      84
#define CH_092      92
#define GROUP_5     5
#define CH_096      96
#define CH_104      104
#define CH_112      112
#define GROUP_6     6
#define CH_116      116
#define CH_124      124
#define CH_136      136
#define GROUP_7     7
#define CH_140      140
#define CH_149      149
#define CH_157      157
#define GROUP_8     8
#define CH_161      161
#define CH_173      173
#define CH_181      181

#define GROUP_L     0
#define GROUP_M     1
#define GROUP_H     2

#define CLEAN_ALL   0
#define CLEAN_2G    1
#define CLEAN_5G    2






#if defined(MT7986)
#define MT7986_IBF_PHASE_EEPROM_START      0x651

#define MT7986_IBF_LNA_PHASE_G0_ADDR       MT7986_IBF_PHASE_EEPROM_START
#define MT7986_IBF_LNA_PHASE_G1_ADDR       (MT7986_IBF_LNA_PHASE_G0_ADDR  + 0x28)
#define MT7986_IBF_LNA_PHASE_G2_ADDR       (MT7986_IBF_LNA_PHASE_G1_ADDR  + 0x28)
#define MT7986_IBF_LNA_PHASE_G3_ADDR       (MT7986_IBF_LNA_PHASE_G2_ADDR  + 0x28)
#define MT7986_IBF_LNA_PHASE_G4_ADDR       (MT7986_IBF_LNA_PHASE_G3_ADDR  + 0x28)
#define MT7986_IBF_LNA_PHASE_G5_ADDR       (MT7986_IBF_LNA_PHASE_G4_ADDR  + 0x28)
#define MT7986_IBF_LNA_PHASE_G6_ADDR       (MT7986_IBF_LNA_PHASE_G5_ADDR  + 0x28)
#define MT7986_IBF_LNA_PHASE_G7_ADDR       (MT7986_IBF_LNA_PHASE_G6_ADDR  + 0x28)
#define MT7986_IBF_LNA_PHASE_G8_ADDR       (MT7986_IBF_LNA_PHASE_G7_ADDR  + 0x28)

#define MT7986_EPA_IBF_PHASE_EEPROM_START      0x60A

#define MT7986_EPA_IBF_LNA_PHASE_G0_ADDR       MT7986_EPA_IBF_PHASE_EEPROM_START
#define MT7986_EPA_IBF_LNA_PHASE_G1_ADDR       (MT7986_EPA_IBF_LNA_PHASE_G0_ADDR  + 0x28)
#define MT7986_EPA_IBF_LNA_PHASE_G2_ADDR       (MT7986_EPA_IBF_LNA_PHASE_G1_ADDR  + 0x28)
#define MT7986_EPA_IBF_LNA_PHASE_G3_ADDR       (MT7986_EPA_IBF_LNA_PHASE_G2_ADDR  + 0x28)
#define MT7986_EPA_IBF_LNA_PHASE_G4_ADDR       (MT7986_EPA_IBF_LNA_PHASE_G3_ADDR  + 0x28)
#define MT7986_EPA_IBF_LNA_PHASE_G5_ADDR       (MT7986_EPA_IBF_LNA_PHASE_G4_ADDR  + 0x28)
#define MT7986_EPA_IBF_LNA_PHASE_G6_ADDR       (MT7986_EPA_IBF_LNA_PHASE_G5_ADDR  + 0x28)
#define MT7986_EPA_IBF_LNA_PHASE_G7_ADDR       (MT7986_EPA_IBF_LNA_PHASE_G6_ADDR  + 0x28)
#define MT7986_EPA_IBF_LNA_PHASE_G8_ADDR       (MT7986_EPA_IBF_LNA_PHASE_G7_ADDR  + 0x28)
#endif

#if defined(MT7916)
#define MT7916_IBF_PHASE_EEPROM_START      0x60A

#define MT7916_IBF_LNA_PHASE_G0_ADDR       MT7916_IBF_PHASE_EEPROM_START
#define MT7916_IBF_LNA_PHASE_G1_ADDR       (MT7916_IBF_LNA_PHASE_G0_ADDR  + 0x28)
#define MT7916_IBF_LNA_PHASE_G2_ADDR       (MT7916_IBF_LNA_PHASE_G1_ADDR  + 0x28)
#define MT7916_IBF_LNA_PHASE_G3_ADDR       (MT7916_IBF_LNA_PHASE_G2_ADDR  + 0x28)
#define MT7916_IBF_LNA_PHASE_G4_ADDR       (MT7916_IBF_LNA_PHASE_G3_ADDR  + 0x28)
#define MT7916_IBF_LNA_PHASE_G5_ADDR       (MT7916_IBF_LNA_PHASE_G4_ADDR  + 0x28)
#define MT7916_IBF_LNA_PHASE_G6_ADDR       (MT7916_IBF_LNA_PHASE_G5_ADDR  + 0x28)
#define MT7916_IBF_LNA_PHASE_G7_ADDR       (MT7916_IBF_LNA_PHASE_G6_ADDR  + 0x28)
#define MT7916_IBF_LNA_PHASE_G8_ADDR       (MT7916_IBF_LNA_PHASE_G7_ADDR  + 0x28)
#endif

#if defined(MT7981)
#define MT7981_IBF_PHASE_EEPROM_START      0x60A

#define MT7981_IBF_LNA_PHASE_G0_ADDR       MT7981_IBF_PHASE_EEPROM_START
#define MT7981_IBF_LNA_PHASE_G1_ADDR       (MT7981_IBF_LNA_PHASE_G0_ADDR  + 0x28)
#define MT7981_IBF_LNA_PHASE_G2_ADDR       (MT7981_IBF_LNA_PHASE_G1_ADDR  + 0x28)
#define MT7981_IBF_LNA_PHASE_G3_ADDR       (MT7981_IBF_LNA_PHASE_G2_ADDR  + 0x28)
#define MT7981_IBF_LNA_PHASE_G4_ADDR       (MT7981_IBF_LNA_PHASE_G3_ADDR  + 0x28)
#define MT7981_IBF_LNA_PHASE_G5_ADDR       (MT7981_IBF_LNA_PHASE_G4_ADDR  + 0x28)
#define MT7981_IBF_LNA_PHASE_G6_ADDR       (MT7981_IBF_LNA_PHASE_G5_ADDR  + 0x28)
#define MT7981_IBF_LNA_PHASE_G7_ADDR       (MT7981_IBF_LNA_PHASE_G6_ADDR  + 0x28)
#define MT7981_IBF_LNA_PHASE_G8_ADDR       (MT7981_IBF_LNA_PHASE_G7_ADDR  + 0x28)
#endif

#define IBF_R0_H_G0                 0
#define IBF_R0_M_G0                 1
#define IBF_R0_L_G0                 2
#define IBF_R1_H_G0                 3
#define IBF_R1_M_G0                 4
#define IBF_R1_L_G0                 5
#define IBF_R2_H_G0                 6
#define IBF_R2_M_G0                 7
#define IBF_R2_L_G0                 8
#define IBF_R3_H_G0                 9
#define IBF_R3_M_G0                 10
#define IBF_R3_L_G0                 11
#define IBF_T0_M_G0                 12
#define IBF_T1_M_G0                 13
#define IBF_T2_M_G0                 14
#define IBF_T0_L_G0                 15
#define IBF_T1_L_G0                 16
#define IBF_T2_L_G0                 17
#define IBF_T0_H_G0                 18
#define IBF_T1_H_G0                 19
#define IBF_T2_H_G0                 20

#define IBF_R0_H_Gx                 0
#define IBF_R0_M_Gx                 1
#define IBF_R0_L_Gx                 2
#define IBF_R1_H_Gx                 3
#define IBF_R1_M_Gx                 4
#define IBF_R1_L_Gx                 5
#define IBF_R2_H_Gx                 6
#define IBF_R2_M_Gx                 7
#define IBF_R2_L_Gx                 8
#define IBF_R3_H_Gx                 9
#define IBF_R3_M_Gx                 10
#define IBF_R3_L_Gx                 11
#define IBF_R2_SX2_H_Gx             12
#define IBF_R2_SX2_M_Gx             13
#define IBF_R2_SX2_L_Gx             14
#define IBF_R3_SX2_H_Gx             15
#define IBF_R3_SX2_M_Gx             16
#define IBF_R3_SX2_L_Gx             17
#define IBF_T0_M_Gx                 18
#define IBF_T1_M_Gx                 19
#define IBF_T2_M_Gx                 20
#define IBF_T2_SX2_M_Gx             21
#define IBF_T0_L_Gx                 22
#define IBF_T1_L_Gx                 23
#define IBF_T2_L_Gx                 24
#define IBF_T2_SX2_L_Gx             25
#define IBF_T0_H_Gx                 26
#define IBF_T1_H_Gx                 27
#define IBF_T2_H_Gx                 28
#define IBF_T2_SX2_H_Gx             29

typedef enum _IBF_PHASE_E2P_UPDATE_TYPE {
	IBF_PHASE_ONE_GROUP_UPDATE,
	IBF_PHASE_ALL_GROUP_UPDATE,
	IBF_PHASE_ALL_GROUP_ERASE,
	IBF_PHASE_ALL_GROUP_READ_FROM_E2P
} IBF_PHASE_E2P_UPDATE_TYPE;

typedef enum _IBF_PHASE_CAL_TYPE {
	IBF_PHASE_CAL_NOTHING,
	IBF_PHASE_CAL_NORMAL,
	IBF_PHASE_CAL_VERIFY,
	IBF_PHASE_CAL_NORMAL_INSTRUMENT,

	IBF_PHASE_CAL_VERIFY_INSTRUMENT

} IBF_PHASE_CAL_TYPE;

typedef enum _IBF_PHASE_STATUS_INSTRUMENT {
	STATUS_EBF_INVALID,
	STATUS_IBF_INVALID,
	STATUS_OTHER_ISSUE,
	STATUS_DONE
} IBF_PHASE_STATUS_INSTRUMENT;



#if defined(MT7986)
typedef struct _MT7986_IBF_PHASE_OUT {
	UINT8 ucC0_L;
	UINT8 ucC1_L;
	UINT8 ucC2_L;
	UINT8 ucC3_L;
	UINT8 ucC0_M;
	UINT8 ucC1_M;
	UINT8 ucC2_M;
	UINT8 ucC3_M;
	UINT8 ucC0_H;
	UINT8 ucC1_H;
	UINT8 ucC2_H;
	UINT8 ucC3_H;
	UINT8 ucC0_UH;
	UINT8 ucC1_UH;
	UINT8 ucC2_UH;
	UINT8 ucC3_UH;
} MT7986_IBF_PHASE_OUT, *P_MT7986_IBF_PHASE_OUT;

typedef struct _MT7986_IBF_PHASE_G0_T {
	UINT8 ucG0_R0_UH;
	UINT8 ucG0_R0_H;
	UINT8 ucG0_R0_M;
	UINT8 ucG0_R0_L;
	UINT8 ucG0_R0_UL;
	UINT8 ucG0_R1_UH;
	UINT8 ucG0_R1_H;
	UINT8 ucG0_R1_M;
	UINT8 ucG0_R1_L;
	UINT8 ucG0_R1_UL;
	UINT8 ucG0_R2_UH;
	UINT8 ucG0_R2_H;
	UINT8 ucG0_R2_M;
	UINT8 ucG0_R2_L;
	UINT8 ucG0_R2_UL;
	UINT8 ucG0_R3_UH;
	UINT8 ucG0_R3_H;
	UINT8 ucG0_R3_M;
	UINT8 ucG0_R3_L;
	UINT8 ucG0_R3_UL;
	UINT8 ucG0_R2_UH_SX2;
	UINT8 ucG0_R2_H_SX2;
	UINT8 ucG0_R2_M_SX2;
	UINT8 ucG0_R2_L_SX2;
	UINT8 ucG0_R2_UL_SX2;
	UINT8 ucG0_R3_UH_SX2;
	UINT8 ucG0_R3_H_SX2;
	UINT8 ucG0_R3_M_SX2;
	UINT8 ucG0_R3_L_SX2;
	UINT8 ucG0_R3_UL_SX2;
	UINT8 ucG0_M_T0_H;
	UINT8 ucG0_M_T1_H;
	UINT8 ucG0_M_T2_H;
	UINT8 ucG0_M_T2_H_SX2;
	UINT8 ucG0_R0_Reserved;
	UINT8 ucG0_R1_Reserved;
	UINT8 ucG0_R2_Reserved;
	UINT8 ucG0_R3_Reserved;
	UINT8 ucG0_R2_SX2_Reserved;
	UINT8 ucG0_R3_SX2_Reserved;
} MT7986_IBF_PHASE_G0_T, *P_MT7986_IBF_PHASE_G0_T;

typedef struct _MT7986_IBF_PHASE_Gx_T {
	UINT8 ucGx_R0_UH;
	UINT8 ucGx_R0_H;
	UINT8 ucGx_R0_M;
	UINT8 ucGx_R0_L;
	UINT8 ucGx_R0_UL;
	UINT8 ucGx_R1_UH;
	UINT8 ucGx_R1_H;
	UINT8 ucGx_R1_M;
	UINT8 ucGx_R1_L;
	UINT8 ucGx_R1_UL;
	UINT8 ucGx_R2_UH;
	UINT8 ucGx_R2_H;
	UINT8 ucGx_R2_M;
	UINT8 ucGx_R2_L;
	UINT8 ucGx_R2_UL;
	UINT8 ucGx_R3_UH;
	UINT8 ucGx_R3_H;
	UINT8 ucGx_R3_M;
	UINT8 ucGx_R3_L;
	UINT8 ucGx_R3_UL;
	UINT8 ucGx_R2_UH_SX2;
	UINT8 ucGx_R2_H_SX2;
	UINT8 ucGx_R2_M_SX2;
	UINT8 ucGx_R2_L_SX2;
	UINT8 ucGx_R2_UL_SX2;
	UINT8 ucGx_R3_UH_SX2;
	UINT8 ucGx_R3_H_SX2;
	UINT8 ucGx_R3_M_SX2;
	UINT8 ucGx_R3_L_SX2;
	UINT8 ucGx_R3_UL_SX2;
	UINT8 ucGx_M_T0_H;
	UINT8 ucGx_M_T1_H;
	UINT8 ucGx_M_T2_H;
	UINT8 ucGx_M_T2_H_SX2;
	UINT8 ucGx_R0_Reserved;
	UINT8 ucGx_R1_Reserved;
	UINT8 ucGx_R2_Reserved;
	UINT8 ucGx_R3_Reserved;
	UINT8 ucGx_R2_SX2_Reserved;
	UINT8 ucGx_R3_SX2_Reserved;
} MT7986_IBF_PHASE_Gx_T, *P_MT7986_IBF_PHASE_Gx_T;
#endif

#if defined(MT7916)
typedef struct _MT7916_IBF_PHASE_OUT {
	UINT8 ucC0_L;
	UINT8 ucC1_L;
	UINT8 ucC2_L;
	UINT8 ucC3_L;
	UINT8 ucC0_M;
	UINT8 ucC1_M;
	UINT8 ucC2_M;
	UINT8 ucC3_M;
	UINT8 ucC0_H;
	UINT8 ucC1_H;
	UINT8 ucC2_H;
	UINT8 ucC3_H;
	UINT8 ucC0_UH;
	UINT8 ucC1_UH;
	UINT8 ucC2_UH;
	UINT8 ucC3_UH;
} MT7916_IBF_PHASE_OUT, *P_MT7916_IBF_PHASE_OUT;

typedef struct _MT7916_IBF_PHASE_G0_T {
	UINT8 ucG0_R0_UH;
	UINT8 ucG0_R0_H;
	UINT8 ucG0_R0_M;
	UINT8 ucG0_R0_L;
	UINT8 ucG0_R0_UL;
	UINT8 ucG0_R1_UH;
	UINT8 ucG0_R1_H;
	UINT8 ucG0_R1_M;
	UINT8 ucG0_R1_L;
	UINT8 ucG0_R1_UL;
	UINT8 ucG0_R2_UH;
	UINT8 ucG0_R2_H;
	UINT8 ucG0_R2_M;
	UINT8 ucG0_R2_L;
	UINT8 ucG0_R2_UL;
	UINT8 ucG0_R3_UH;
	UINT8 ucG0_R3_H;
	UINT8 ucG0_R3_M;
	UINT8 ucG0_R3_L;
	UINT8 ucG0_R3_UL;
	UINT8 ucG0_R2_UH_SX2;
	UINT8 ucG0_R2_H_SX2;
	UINT8 ucG0_R2_M_SX2;
	UINT8 ucG0_R2_L_SX2;
	UINT8 ucG0_R2_UL_SX2;
	UINT8 ucG0_R3_UH_SX2;
	UINT8 ucG0_R3_H_SX2;
	UINT8 ucG0_R3_M_SX2;
	UINT8 ucG0_R3_L_SX2;
	UINT8 ucG0_R3_UL_SX2;
	UINT8 ucG0_M_T0_H;
	UINT8 ucG0_M_T1_H;
	UINT8 ucG0_M_T2_H;
	UINT8 ucG0_M_T2_H_SX2;
	UINT8 ucG0_R0_Reserved;
	UINT8 ucG0_R1_Reserved;
	UINT8 ucG0_R2_Reserved;
	UINT8 ucG0_R3_Reserved;
	UINT8 ucG0_R2_SX2_Reserved;
	UINT8 ucG0_R3_SX2_Reserved;
} MT7916_IBF_PHASE_G0_T, *P_MT7916_IBF_PHASE_G0_T;

typedef struct _MT7916_IBF_PHASE_Gx_T {
	UINT8 ucGx_R0_UH;
	UINT8 ucGx_R0_H;
	UINT8 ucGx_R0_M;
	UINT8 ucGx_R0_L;
	UINT8 ucGx_R0_UL;
	UINT8 ucGx_R1_UH;
	UINT8 ucGx_R1_H;
	UINT8 ucGx_R1_M;
	UINT8 ucGx_R1_L;
	UINT8 ucGx_R1_UL;
	UINT8 ucGx_R2_UH;
	UINT8 ucGx_R2_H;
	UINT8 ucGx_R2_M;
	UINT8 ucGx_R2_L;
	UINT8 ucGx_R2_UL;
	UINT8 ucGx_R3_UH;
	UINT8 ucGx_R3_H;
	UINT8 ucGx_R3_M;
	UINT8 ucGx_R3_L;
	UINT8 ucGx_R3_UL;
	UINT8 ucGx_R2_UH_SX2;
	UINT8 ucGx_R2_H_SX2;
	UINT8 ucGx_R2_M_SX2;
	UINT8 ucGx_R2_L_SX2;
	UINT8 ucGx_R2_UL_SX2;
	UINT8 ucGx_R3_UH_SX2;
	UINT8 ucGx_R3_H_SX2;
	UINT8 ucGx_R3_M_SX2;
	UINT8 ucGx_R3_L_SX2;
	UINT8 ucGx_R3_UL_SX2;
	UINT8 ucGx_M_T0_H;
	UINT8 ucGx_M_T1_H;
	UINT8 ucGx_M_T2_H;
	UINT8 ucGx_M_T2_H_SX2;
	UINT8 ucGx_R0_Reserved;
	UINT8 ucGx_R1_Reserved;
	UINT8 ucGx_R2_Reserved;
	UINT8 ucGx_R3_Reserved;
	UINT8 ucGx_R2_SX2_Reserved;
	UINT8 ucGx_R3_SX2_Reserved;
} MT7916_IBF_PHASE_Gx_T, *P_MT7916_IBF_PHASE_Gx_T;
#endif

#if defined(MT7981)
typedef struct _MT7981_IBF_PHASE_OUT {
	UINT8 ucC0_L;
	UINT8 ucC1_L;
	UINT8 ucC2_L;
	UINT8 ucC3_L;
	UINT8 ucC0_M;
	UINT8 ucC1_M;
	UINT8 ucC2_M;
	UINT8 ucC3_M;
	UINT8 ucC0_H;
	UINT8 ucC1_H;
	UINT8 ucC2_H;
	UINT8 ucC3_H;
	UINT8 ucC0_UH;
	UINT8 ucC1_UH;
	UINT8 ucC2_UH;
	UINT8 ucC3_UH;
} MT7981_IBF_PHASE_OUT, *P_MT7981_IBF_PHASE_OUT;

typedef struct _MT7981_IBF_PHASE_G0_T {
	UINT8 ucG0_R0_UH;
	UINT8 ucG0_R0_H;
	UINT8 ucG0_R0_M;
	UINT8 ucG0_R0_L;
	UINT8 ucG0_R0_UL;
	UINT8 ucG0_R1_UH;
	UINT8 ucG0_R1_H;
	UINT8 ucG0_R1_M;
	UINT8 ucG0_R1_L;
	UINT8 ucG0_R1_UL;
	UINT8 ucG0_R2_UH;
	UINT8 ucG0_R2_H;
	UINT8 ucG0_R2_M;
	UINT8 ucG0_R2_L;
	UINT8 ucG0_R2_UL;
	UINT8 ucG0_R3_UH;
	UINT8 ucG0_R3_H;
	UINT8 ucG0_R3_M;
	UINT8 ucG0_R3_L;
	UINT8 ucG0_R3_UL;
	UINT8 ucG0_R2_UH_SX2;
	UINT8 ucG0_R2_H_SX2;
	UINT8 ucG0_R2_M_SX2;
	UINT8 ucG0_R2_L_SX2;
	UINT8 ucG0_R2_UL_SX2;
	UINT8 ucG0_R3_UH_SX2;
	UINT8 ucG0_R3_H_SX2;
	UINT8 ucG0_R3_M_SX2;
	UINT8 ucG0_R3_L_SX2;
	UINT8 ucG0_R3_UL_SX2;
	UINT8 ucG0_M_T0_H;
	UINT8 ucG0_M_T1_H;
	UINT8 ucG0_M_T2_H;
	UINT8 ucG0_M_T2_H_SX2;
	UINT8 ucG0_R0_Reserved;
	UINT8 ucG0_R1_Reserved;
	UINT8 ucG0_R2_Reserved;
	UINT8 ucG0_R3_Reserved;
	UINT8 ucG0_R2_SX2_Reserved;
	UINT8 ucG0_R3_SX2_Reserved;
} MT7981_IBF_PHASE_G0_T, *P_MT7981_IBF_PHASE_G0_T;

typedef struct _MT7981_IBF_PHASE_Gx_T {
	UINT8 ucGx_R0_UH;
	UINT8 ucGx_R0_H;
	UINT8 ucGx_R0_M;
	UINT8 ucGx_R0_L;
	UINT8 ucGx_R0_UL;
	UINT8 ucGx_R1_UH;
	UINT8 ucGx_R1_H;
	UINT8 ucGx_R1_M;
	UINT8 ucGx_R1_L;
	UINT8 ucGx_R1_UL;
	UINT8 ucGx_R2_UH;
	UINT8 ucGx_R2_H;
	UINT8 ucGx_R2_M;
	UINT8 ucGx_R2_L;
	UINT8 ucGx_R2_UL;
	UINT8 ucGx_R3_UH;
	UINT8 ucGx_R3_H;
	UINT8 ucGx_R3_M;
	UINT8 ucGx_R3_L;
	UINT8 ucGx_R3_UL;
	UINT8 ucGx_R2_UH_SX2;
	UINT8 ucGx_R2_H_SX2;
	UINT8 ucGx_R2_M_SX2;
	UINT8 ucGx_R2_L_SX2;
	UINT8 ucGx_R2_UL_SX2;
	UINT8 ucGx_R3_UH_SX2;
	UINT8 ucGx_R3_H_SX2;
	UINT8 ucGx_R3_M_SX2;
	UINT8 ucGx_R3_L_SX2;
	UINT8 ucGx_R3_UL_SX2;
	UINT8 ucGx_M_T0_H;
	UINT8 ucGx_M_T1_H;
	UINT8 ucGx_M_T2_H;
	UINT8 ucGx_M_T2_H_SX2;
	UINT8 ucGx_R0_Reserved;
	UINT8 ucGx_R1_Reserved;
	UINT8 ucGx_R2_Reserved;
	UINT8 ucGx_R3_Reserved;
	UINT8 ucGx_R2_SX2_Reserved;
	UINT8 ucGx_R3_SX2_Reserved;
} MT7981_IBF_PHASE_Gx_T, *P_MT7981_IBF_PHASE_Gx_T;
#endif









#if defined(MT7986)
VOID mt7986_iBFPhaseComp(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR ucGroup,
						IN PCHAR pCmdBuf);

VOID mt7986_iBFPhaseCalInit(IN struct _RTMP_ADAPTER *pAd);

VOID mt7986_iBFPhaseFreeMem(IN struct _RTMP_ADAPTER *pAd);

VOID mt7986_iBFPhaseCalE2PInit(IN struct _RTMP_ADAPTER *pAd);

VOID mt7986_iBFPhaseCalE2PUpdate(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR   ucGroup,
						IN BOOLEAN fgSX2,
						IN UCHAR   ucUpdateAllType);

VOID mt7986_iBFPhaseCalReport(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR   ucGroupL_M_H,
						IN UCHAR   ucGroup,
						IN BOOLEAN fgSX2,
						IN UCHAR   ucStatus,
						IN UCHAR   ucPhaseCalType,
						IN PUCHAR  pBuf);

VOID mt7986_eBFPfmuMemAlloc(IN struct _RTMP_ADAPTER *pAd,
						IN PCHAR pPfmuMemRow,
						IN PCHAR pPfmuMemCol);

VOID mt7986_iBFPfmuMemAlloc(IN struct _RTMP_ADAPTER *pAd,
						IN PCHAR pPfmuMemRow,
						IN PCHAR pPfmuMemCol);
#endif /* MT7986 */

#if defined(MT7916)
VOID mt7916_iBFPhaseComp(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR ucGroup,
						IN PCHAR pCmdBuf);

VOID mt7916_iBFPhaseCalInit(IN struct _RTMP_ADAPTER *pAd);

VOID mt7916_iBFPhaseFreeMem(IN struct _RTMP_ADAPTER *pAd);

VOID mt7916_iBFPhaseCalE2PInit(IN struct _RTMP_ADAPTER *pAd);

VOID mt7916_iBFPhaseCalE2PUpdate(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR   ucGroup,
						IN BOOLEAN fgSX2,
						IN UCHAR   ucUpdateAllType);

VOID mt7916_iBFPhaseCalReport(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR   ucGroupL_M_H,
						IN UCHAR   ucGroup,
						IN BOOLEAN fgSX2,
						IN UCHAR   ucStatus,
						IN UCHAR   ucPhaseCalType,
						IN PUCHAR  pBuf);

VOID mt7916_eBFPfmuMemAlloc(IN struct _RTMP_ADAPTER *pAd,
						IN PCHAR pPfmuMemRow,
						IN PCHAR pPfmuMemCol);

VOID mt7916_iBFPfmuMemAlloc(IN struct _RTMP_ADAPTER *pAd,
						IN PCHAR pPfmuMemRow,
						IN PCHAR pPfmuMemCol);
#endif /* MT7916 */

#if defined(MT7981)
VOID mt7981_iBFPhaseComp(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR ucGroup,
						IN PCHAR pCmdBuf);

VOID mt7981_iBFPhaseCalInit(IN struct _RTMP_ADAPTER *pAd);

VOID mt7981_iBFPhaseFreeMem(IN struct _RTMP_ADAPTER *pAd);

VOID mt7981_iBFPhaseCalE2PInit(IN struct _RTMP_ADAPTER *pAd);

VOID mt7981_iBFPhaseCalE2PUpdate(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR   ucGroup,
						IN BOOLEAN fgSX2,
						IN UCHAR   ucUpdateAllType);

VOID mt7981_iBFPhaseCalReport(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR   ucGroupL_M_H,
						IN UCHAR   ucGroup,
						IN BOOLEAN fgSX2,
						IN UCHAR   ucStatus,
						IN UCHAR   ucPhaseCalType,
						IN PUCHAR  pBuf);

VOID mt7981_eBFPfmuMemAlloc(IN struct _RTMP_ADAPTER *pAd,
						IN PCHAR pPfmuMemRow,
						IN PCHAR pPfmuMemCol);

VOID mt7981_iBFPfmuMemAlloc(IN struct _RTMP_ADAPTER *pAd,
						IN PCHAR pPfmuMemRow,
						IN PCHAR pPfmuMemCol);
#endif /* MT7981 */

#endif /* _RT_TXBF_CAL_H_ */
