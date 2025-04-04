/*
** $Id: @(#) wf_cr_sw_def.h $
*/

/*! \file   "wf_cr_sw_def.h"
    \brief
*/

/*******************************************************************************
* Copyright (c) 2009 MediaTek Inc.
*
* All rights reserved. Copying, compilation, modification, distribution
* or any other use whatsoever of this material is strictly prohibited
* except in accordance with a Software License Agreement with
* MediaTek Inc.
********************************************************************************
*/

/*******************************************************************************
* LEGAL DISCLAIMER
*
* BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
* AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK
* SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
* PROVIDED TO BUYER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY
* DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE
* ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY
* WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK
* SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
* WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE
* FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO
* CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
* BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
* LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL
* BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
* ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
* BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
* WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT
* OF LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING
* THEREOF AND RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN
* FRANCISCO, CA, UNDER THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE
* (ICC).
********************************************************************************
*/

#ifndef _WF_CR_SW_DEF_H
#define _WF_CR_SW_DEF_H

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/


//****************************************************************************
//
//                     MCU_SYSRAM SW CR Definitions
//
//****************************************************************************
#define WF_SW_DEF_CR_BASE                0x00411400

#define WF_SW_DEF_CR_WACPU_STAT_ADDR            (WF_SW_DEF_CR_BASE + 0x000) // 1400
#define WF_SW_DEF_CR_WACPU_SLEEP_STAT_ADDR      (WF_SW_DEF_CR_BASE + 0x004) // 1404
#define WF_SW_DEF_CR_WM2WA_ACTION_ADDR          (WF_SW_DEF_CR_BASE + 0x008) // 1408
#define WF_SW_DEF_CR_WA2WM_ACTION_ADDR          (WF_SW_DEF_CR_BASE + 0x00C) // 140C
#define WF_SW_DEF_CR_LP_DBG0_ADDR               (WF_SW_DEF_CR_BASE + 0x010) // 1410
#define WF_SW_DEF_CR_LP_DBG1_ADDR               (WF_SW_DEF_CR_BASE + 0x014) // 1414
#define WF_SW_DEF_CR_ICAP_SPECTRUM_MODE_ADDR    (WF_SW_DEF_CR_BASE + 0x03C) // 143C
#define WF_SW_DEF_CR_SER_STATUS_ADDR            (WF_SW_DEF_CR_BASE + 0x040) // 1440
#define WF_SW_DEF_CR_PLE_STATUS_ADDR            (WF_SW_DEF_CR_BASE + 0x044) // 1444
#define WF_SW_DEF_CR_PLE1_STATUS_ADDR           (WF_SW_DEF_CR_BASE + 0x048) // 1448
#define WF_SW_DEF_CR_PLE_AMSDU_STATUS_ADDR      (WF_SW_DEF_CR_BASE + 0x04C) // 144C
#define WF_SW_DEF_CR_PSE_STATUS_ADDR            (WF_SW_DEF_CR_BASE + 0x050) // 1450
#define WF_SW_DEF_CR_PSE1_STATUS_ADDR           (WF_SW_DEF_CR_BASE + 0x054) // 1454
#define WF_SW_DEF_CR_LAMC_WISR6_BN0_STATUS_ADDR (WF_SW_DEF_CR_BASE + 0x058) // 1458
#define WF_SW_DEF_CR_LAMC_WISR6_BN1_STATUS_ADDR (WF_SW_DEF_CR_BASE + 0x05C) // 145C
#define WF_SW_DEF_CR_LAMC_WISR7_BN0_STATUS_ADDR (WF_SW_DEF_CR_BASE + 0x060) // 1460
#define WF_SW_DEF_CR_LAMC_WISR7_BN1_STATUS_ADDR (WF_SW_DEF_CR_BASE + 0x064) // 1464
#define WF_SW_DEF_CR_USB_MCU_EVENT_ADD          (WF_SW_DEF_CR_BASE + 0x070) // 1470
#define WF_SW_DEF_CR_USB_HOST_ACK_ADDR          (WF_SW_DEF_CR_BASE + 0x074) // 1474
#define WF_SW_DEF_CR_PM_CHANGE_ADDR             (WF_SW_DEF_CR_BASE + 0x078) // 1478
/* =====================================================================================

  ---WF_SW_DEF_CR_WACPU_SLEEP_STAT_ADDR (0x00411400 + 0x004)---

    SLEEP_STATUS[0]          - (RW) 0: Awake, 1: sleep
    GATING_STATUS[1]           - (RW) 0:Idle, 1: Gating
    RESERVED5[31..2]             - (RO) Reserved bits

 =====================================================================================*/
#define WF_SW_DEF_CR_WACPU_SLEEP_STAT_SLEEP_ADDR    WF_SW_DEF_CR_WACPU_SLEEP_STAT_ADDR
#define WF_SW_DEF_CR_WACPU_SLEEP_STAT_SLEEP_MASK    0x00000001
#define WF_SW_DEF_CR_WACPU_SLEEP_STAT_SLEEP_SHFT    0
#define WF_SW_DEF_CR_WACPU_SLEEP_STAT_GATING_ADDR   WF_SW_DEF_CR_WACPU_SLEEP_STAT_ADDR
#define WF_SW_DEF_CR_WACPU_SLEEP_STAT_GATING_MASK   0x00000002
#define WF_SW_DEF_CR_WACPU_SLEEP_STAT_GATING_SHFT   1



/* =====================================================================================

  ---WF_SW_DEF_CR_PM_CHANGE_ADDR (0x00411400 + 0x078)---

    WCID[15:0]          - (RW) wcid
    TID[19:16]          - (RW) tid
    RESERVED5[20:20]    - (RO) Reserved bits
    ACK[31]             - (RW) Driver set 1 and FW clear to 0 after processing message

 =====================================================================================*/
#define WF_SW_DEF_CR_PM_CHANGE_WCID_ADDR           WF_SW_DEF_CR_PM_CHANGE_ADDR
#define WF_SW_DEF_CR_PM_CHANGE_WCID_MASK           0x000003FF
#define WF_SW_DEF_CR_PM_CHANGE_WCID_SHIFT          0
#define WF_SW_DEF_CR_PM_CHANGE_ACK_ADDR            WF_SW_DEF_CR_PM_CHANGE_ADDR
#define WF_SW_DEF_CR_PM_CHANGE_ACK_MASK            0x00000400
#define WF_SW_DEF_CR_PM_CHANGE_ACK_SHIFT           10
#define WF_SW_DEF_CR_PM_CHANGE_TID_ADDR            WF_SW_DEF_CR_PM_CHANGE_ADDR
#define WF_SW_DEF_CR_PM_CHANGE_TID_MASK            0x0000F000
#define WF_SW_DEF_CR_PM_CHANGE_TID_SHIFT           12
#define WF_SW_DEF_CR_PM_CHANGE_FC_ADDR             WF_SW_DEF_CR_PM_CHANGE_ADDR
#define WF_SW_DEF_CR_PM_CHANGE_FC_MASK             0xFFFF0000
#define WF_SW_DEF_CR_PM_CHANGE_FC_SHIFT            16



#endif /* _WF_CR_SW_DEF_H */


