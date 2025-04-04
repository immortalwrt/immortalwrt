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
 ****************************************************************************

    Module Name:
    dot11ac_vht.h

    Abstract:
	Defined IE/frame structures of 802.11ac (D1.2).

    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Shiang Tu  01-11-2012    created for 11ac
 */
#ifndef __DOT11AC_VHT_H__
#define __DOT11AC_VHT_H__

#include "rtmp_type.h"
#include "dot11_base.h"

#define IE_VHT_CAP 191
#define IE_VHT_OP 192
#define IE_EXT_BSS_LOAD 193
#define IE_WIDE_BW_CH_SWITCH 194
#define IE_VHT_TXPWR_ENV 195
#define IE_CH_SWITCH_WRAPPER 196
#define IE_AID 197
#define IE_QUIET_CHANNEL 198


enum vht_mfb {
	NO_VHT_MFB,
	RESERVED_VHT_MFB,
	UNSOLICITED_VHT_MFB,
	BOTH_VHT_MFB,
};

enum mpdu_length {
	MPDU_3895_OCTETS,
	MPDU_7991_OCTETS,
	MPDU_11454_OCTETS
};

enum bw_sig {
	BW_SIGNALING_DISABLE,
	BW_SIGNALING_STATIC,
	BW_SIGNALING_DYNAMIC
};

/* VHT Channel Width subfield */
enum ch_width {
	CH_WIDTH_20M_40M,
	CH_WIDTH_80M_160M_8080M,
	CH_WIDTH_160M, /* deprecated */
	CH_WIDTH_8080M /* deprecated */
};

enum support_ch_width_set {
	CH_WIDTH_SET_0,
	CH_WIDTH_SET_1,
	CH_WIDTH_SET_2
};

enum extend_nss_bw {
	EXT_NSS_BW_0,
	EXT_NSS_BW_1,
	EXT_NSS_BW_2,
	EXT_NSS_BW_3
};

enum vht_mcs_set {
	VHT_MCS_7,
	VHT_MCS_8,
	VHT_MCS_9,
	VHT_MCS_NOT_SUPPORT
};

/* vht_caps_ie.cap_info */
#define DOT11AC_CAP_MAX_MPDU_LEN_MASK 0x3
#define DOT11AC_CAP_SUPPORT_CH_WIDTH_SET_SHIFT 2
#define DOT11AC_CAP_SUPPORT_CH_WIDTH_SET_MASK (0x3 << 2)
#define DOT11AC_CAP_RX_LDPC (1 << 4)
#define DOT11AC_CAP_BW80_SGI (1 << 5)
#define DOT11AC_CAP_BW160_8080_SGI (1 << 6)
#define DOT11AC_CAP_TX_STBC (1 << 7)
#define DOT11AC_CAP_RX_STBC_SHIFT 8
#define DOT11AC_CAP_RX_STBC_MASK (0x7 << 8)
#define DOT11AC_CAP_SU_BFER (1 << 11)
#define DOT11AC_CAP_SU_BFEE (1 << 12)
#define DOT11AC_CAP_BFEE_STS_SHIFT 13
#define DOT11AC_CAP_BFEE_STS_MASK (0x7 << 13)
#define DOT11AC_CAP_NUM_SOUND_DIM_SHIFT 16
#define DOT11AC_CAP_NUM_SOUND_DIM_MASK (0x7 << 16)
#define DOT11AC_CAP_MU_BFER (1 << 19)
#define DOT11AC_CAP_MU_BFEE (1 << 20)
#define DOT11AC_CAP_TXOP_PS (1 << 21)
#define DOT11AC_CAP_HTC (1 << 22)
#define DOT11AC_CAP_MAX_AMPDU_LEN_EXP_SHIFT 23
#define DOT11AC_CAP_MAX_AMPDU_LEN_EXP_MASK (0x7 << 23)
#define DOT11AC_CAP_LINK_ADAPT_SHIFT 26
#define DOT11AC_CAP_LINK_ADAPT_MASK (0x3 << 26)
#define DOT11AC_CAP_RX_ANT_PATTERN_CONSIST (1 << 28)
#define DOT11AC_CAP_TX_ANT_PATTERN_CONSIST (1 << 29)
#define DOT11AC_CAP_EXT_NSS_BW_SHIFT 30
#define DOT11AC_CAP_EXT_NSS_BW_MASK (0x3 << 30)
/* vht_caps_ie.vht_txrx_mcs_nss */
#define VHT_MCS_NSS_MASK 0x3
#define DOT11AC_MCS_1SS_SHIFT 0
#define DOT11AC_MCS_1SS_MASK VHT_MCS_NSS_MASK
#define DOT11AC_MCS_2SS_SHIFT 2
#define DOT11AC_MCS_2SS_MASK (VHT_MCS_NSS_MASK << 2)
#define DOT11AC_MCS_3SS_SHIFT 4
#define DOT11AC_MCS_3SS_MASK (VHT_MCS_NSS_MASK << 4)
#define DOT11AC_MCS_4SS_SHIFT 6
#define DOT11AC_MCS_4SS_MASK (VHT_MCS_NSS_MASK << 6)
#define DOT11AC_MCS_5SS_SHIFT 8
#define DOT11AC_MCS_5SS_MASK (VHT_MCS_NSS_MASK << 8)
#define DOT11AC_MCS_6SS_SHIFT 10
#define DOT11AC_MCS_6SS_MASK (VHT_MCS_NSS_MASK << 10)
#define DOT11AC_MCS_7SS_SHIFT 12
#define DOT11AC_MCS_7SS_MASK (VHT_MCS_NSS_MASK << 12)
#define DOT11AC_MCS_8SS_SHIFT 14
#define DOT11AC_MCS_8SS_MASK (VHT_MCS_NSS_MASK << 14)

#define VHT_MAX_MCS_NSS(nss, mcs)\
	((mcs) << (DOT11AC_MCS_ ## nss ## SS_SHIFT))

#define DOT11AC_MCS_MAX_NSTS_TOTAL_SHIFT 13
#define DOT11AC_MCS_MAX_NSTS_TOTAL_MASK (0x7 << 13)
#define DOT11AC_MCS_EXT_NSS_BW_CAPABLE (1 << 13)

struct GNU_PACKED vht_mcs_map {
	UINT16 mcs_map;
};

struct GNU_PACKED vht_txrx_mcs_nss {
	struct vht_mcs_map rx;
	UINT16 rx_gi_data_rate_max_nsts;
	struct vht_mcs_map tx;
	UINT16 tx_gi_data_rate_ext_nss_bw;
};

struct GNU_PACKED vht_caps_ie {
	UINT32 cap_info;
	struct vht_txrx_mcs_nss support_mcs_nss;
};

struct GNU_PACKED vht_opinfo {
	UINT8 ch_width;
	UINT8 ccfs_0;
	UINT8 ccfs_1;
};

struct GNU_PACKED vht_op_ie {
	struct vht_opinfo vht_op_info;
	struct vht_mcs_map vht_basic;
};

/* vht_oper_mode.field */
#define DOT11AC_OP_MODE_CH_WIDTH_SHIFT 0
#define DOT11AC_OP_MODE_CH_WIDTH_MASK 0x3
#define DOT11AC_OP_MODE_BW160_8080 (1 << 2)
#define DOT11AC_OP_MODE_NO_LDPC (1 << 3)
#define DOT11AC_OP_MODE_RX_NSS_SHIFT 4
#define DOT11AC_OP_MODE_RX_NSS_MASK (0x7 << 4)
#define DOT11AC_OP_MODE_RX_NSS_TYPE (1 << 7)
struct GNU_PACKED vht_oper_mode {
	UINT8 field;
};


typedef struct GNU_PACKED _OPERATING_MODE {
#ifdef RT_BIG_ENDIAN
	UCHAR rx_nss_type:1;
	UCHAR rx_nss:3;
	UCHAR no_ldpc:1;
	UCHAR bw160_bw8080:1;
	UCHAR ch_width:2;
#else
	UCHAR ch_width:2;
	UCHAR bw160_bw8080:1;
	UCHAR no_ldpc:1;
	UCHAR rx_nss:3;
	UCHAR rx_nss_type:1;
#endif /* RT_BIG_ENDIAN */
} OPERATING_MODE;


/*
	IEEE 802.11AC D3.0 sec 8.4.2.168
	Operating Mode Notification element

	Element ID: 199 (IE_OPERATING_MODE_NOTIFY)
	Length: 1
*/
typedef struct GNU_PACKED _OPERATING_MODE_NOTIFICATION {
	OPERATING_MODE operating_mode;
} OPERATING_MODE_NOTIFICATION;

typedef struct GNU_PACKED _VHT_CAP_INFO {
#ifdef RT_BIG_ENDIAN
	UINT32 ex_nss_bw:2;
	UINT32 tx_ant_consistency:1;
	UINT32 rx_ant_consistency:1;
	UINT32 vht_link_adapt:2;
	UINT32 max_ampdu_exp:3;
	UINT32 htc_vht_cap:1;
	UINT32 vht_txop_ps:1;
	UINT32 bfee_cap_mu:1;
	UINT32 bfer_cap_mu:1;
	UINT32 num_snd_dimension:3;

	UINT32 bfee_sts_cap:3;
	UINT32 bfee_cap_su:1;
	UINT32 bfer_cap_su:1;
	UINT32 rx_stbc:3;

	UINT32 tx_stbc:1;
	UINT32 sgi_160M:1;
	UINT32 sgi_80M:1;
	UINT32 rx_ldpc:1;
	UINT32 ch_width:2;
	UINT32 max_mpdu_len:2;
#else
	UINT32 max_mpdu_len:2;	/* 0: 3895, 1: 7991, 2: 11454, 3: rsv */
	UINT32 ch_width:2;	/* */
	UINT32 rx_ldpc:1;
	UINT32 sgi_80M:1;
	UINT32 sgi_160M:1;
	UINT32 tx_stbc:1;

	UINT32 rx_stbc:3;
	UINT32 bfer_cap_su:1;
	UINT32 bfee_cap_su:1;
	UINT32 bfee_sts_cap:3;

	UINT32 num_snd_dimension:3;
	UINT32 bfer_cap_mu:1;
	UINT32 bfee_cap_mu:1;
	UINT32 vht_txop_ps:1;
	UINT32 htc_vht_cap:1;
	UINT32 max_ampdu_exp:3;
	UINT32 vht_link_adapt:2;
	UINT32 rx_ant_consistency:1;
	UINT32 tx_ant_consistency:1;
	UINT32 ex_nss_bw:2;
#endif /* RT_BIG_ENDIAN */
} VHT_CAP_INFO;

enum {
	VHT_MCS_CAP_7,
	VHT_MCS_CAP_8,
	VHT_MCS_CAP_9,
	VHT_MCS_CAP_NA
};

typedef struct GNU_PACKED _VHT_MCS_MAP {
#ifdef RT_BIG_ENDIAN
	UINT16 mcs_ss8:2;
	UINT16 mcs_ss7:2;
	UINT16 mcs_ss6:2;
	UINT16 mcs_ss5:2;
	UINT16 mcs_ss4:2;
	UINT16 mcs_ss3:2;
	UINT16 mcs_ss2:2;
	UINT16 mcs_ss1:2;
#else
	UINT16 mcs_ss1:2;
	UINT16 mcs_ss2:2;
	UINT16 mcs_ss3:2;
	UINT16 mcs_ss4:2;
	UINT16 mcs_ss5:2;
	UINT16 mcs_ss6:2;
	UINT16 mcs_ss7:2;
	UINT16 mcs_ss8:2;
#endif /* RT_BIG_ENDIAN */
} VHT_MCS_MAP;

typedef struct GNU_PACKED _VHT_MCS_SET {
#ifdef RT_BIG_ENDIAN
	UINT16 rsv2:2;
	UINT16 vht_ext_nss_bw_cap:1;
	UINT16 tx_high_rate:13;
	struct _VHT_MCS_MAP tx_mcs_map;

	UINT16 rsv:3;
	UINT16 rx_high_rate:13;
	struct _VHT_MCS_MAP rx_mcs_map;
#else
	struct _VHT_MCS_MAP rx_mcs_map;

	UINT16 rx_high_rate:13;
	UINT16 rsv:3;
	struct _VHT_MCS_MAP tx_mcs_map;

	UINT16 tx_high_rate:13;
	UINT16 vht_ext_nss_bw_cap:1;
	UINT16 rsv2:2;
#endif /* RT_BIG_ENDIAN */
} VHT_MCS_SET;

typedef struct GNU_PACKED _VHT_CAP_IE {
	VHT_CAP_INFO vht_cap;
	VHT_MCS_SET mcs_set;
} VHT_CAP_IE;
#define SIZE_OF_VHT_CAP_IE    (sizeof(VHT_CAP_IE))

typedef struct GNU_PACKED _VHT_OP_IE {
	struct vht_opinfo vht_op_info;
	VHT_MCS_MAP basic_mcs_set;
} VHT_OP_IE;
#define SIZE_OF_VHT_OP_IE     (sizeof(VHT_OP_IE))

#define GET_BW160_PRIM80_CENT(prim, cent)\
	((prim > cent) ? (cent + 8) : (cent - 8))
#define GET_BW80_PRIM40_CENT(prim, cent)\
	((prim > cent) ? (cent + 4) : (cent - 4))

/*
	IEEE 802.11AC D2.0, sec 8.4.2.163
	Wide Bandwidth Channel Switch element, figure 8-401bx

	included in the Channel Switch Announcement frames.

	new_ch_width: New STA Channel Width
	center_freq_1: New Channel Center Frequency Segment 1
	center_freq_2: New Channel Center Frequency Segment 2

	The definition of upper subfields is the same as "VHT_OP_INFO"
*/
typedef struct GNU_PACKED _WIDE_BW_CH_SWITCH_ELEMENT {
	UINT8 new_ch_width;
	UINT8 center_freq_1;
	UINT8 center_freq_2;
} WIDE_BW_CH_SWITCH_ELEMENT;


/*
	IEEE 802.11AC D2.0, sec 8.4.2.164
	VHT Transmit Power Envelope element
*/
typedef struct GNU_PACKED _CH_SEG_PAIR {
	UINT8 ch_center_freq;
	UINT8 seg_ch_width;
} CH_SEG_PAIR;


/*
	max_tx_pwr_cnt:
		0: Local Maximum Transmit Power For 20 MHz.
		1: Local Maximum Transmit Power For 20, 40MHz
		2: Local Maximum Transmit Power For 20, 40, 80MHz
		3: Local Maximum Transmit Power For 20, 40, 80, 160/80+80MHz
		4~7: rsv

	max_tx_pwr_interpretation:
		0: EIRP
		1~7: rsv
*/
#define TX_PWR_INTERPRET_EIRP		0
typedef struct GNU_PACKED _VHT_TX_PWR_INFO_ {
#ifdef RT_BIG_ENDIAN
	UINT8 rsv6:2;
	UINT8 max_tx_pwr_interpretation:3;
	UINT8 max_tx_pwr_cnt:3;
#else
	UINT8 max_tx_pwr_cnt:3;
	UINT8 max_tx_pwr_interpretation:3;
	UINT8 rsv6:2;
#endif
} VHT_TX_PWR_INFO;


/*
	IEEE 802.11AC D2.0, sec 8.4.2.164
	VHT Transmit Power Envelope element

	max_txpwr: Maximum Transmit Power
		-> Define the maximum transmit power limit of the tx bandwidth defined
			by the VHT Transmit Power Envelop element. The Maximum Transmit
			Power field is a 8 bit 2's complement signed integer in the range of
			-64 dBm to 63.5 dBm with a 0.5 dB step.

	NOTE: The following two subfields may repeated as needed.
		center_freq_1: Channel Center Frequency Segment
		ch_seg_width: Segment Channel Width
*/
typedef struct GNU_PACKED _VHT_TXPWR_ENV_IE {
	VHT_TX_PWR_INFO tx_pwr_info;
	UINT8 tx_pwr_bw[4];
} VHT_TXPWR_ENV_IE;


typedef struct  GNU_PACKED _VHT_CONTROL {
#ifdef RT_BIG_ENDIAN
	UINT32 RDG:1;
	UINT32 ACConstraint:1;
	UINT32 unso_mfb:1;
	UINT32 fb_tx_type:1;
	UINT32 coding:1;
	UINT32 gid_h:3;
	UINT32 mfb_snr:6;
	UINT32 mfb_bw:2;
	UINT32 mfb_mcs:4;
	UINT32 mfb_n_sts:3;
	UINT32 mfsi_gidl:3;
	UINT32 stbc_ind:1;
	UINT32 comp_msi:2;
	UINT32 mrq:1;
	UINT32 rsv:1;
	UINT32 vht:1;
#else
	UINT32 vht:1;
	UINT32 rsv:1;
	UINT32 mrq:1;
	UINT32 comp_msi:2;
	UINT32 stbc_ind:1;
	UINT32 mfsi_gidl:3;
	UINT32 mfb_n_sts:3;
	UINT32 mfb_mcs:4;
	UINT32 mfb_bw:2;
	UINT32 mfb_snr:6;
	UINT32 gid_h:3;
	UINT32 coding:1;
	UINT32 fb_tx_type:1;
	UINT32 unso_mfb:1;
	UINT32 ACConstraint:1;
	UINT32 RDG:1;
#endif
} VHT_CONTROL;


/*
	802.11 AC Draft3.1 - Section 8.3.1.19, Figure 8-29j

	token_num: Sounding Dialog Token Number
			Contains a value selected by the beamformer to identify the VHT NDP
			Announcment frame.
*/
typedef struct GNU_PACKED _SNDING_DIALOG_TOKEN {
#ifdef RT_BIG_ENDIAN
	UINT8 token_num:6;
	UINT8 rsv:2;
#else
	UINT8 rsv:2;
	UINT8 token_num:6;
#endif /* RT_BIG_ENDIAN */
} SNDING_DIALOG_TOKEN;


/*
	802.11 AC Draft3.1 - Section 8.3.1.19, Figure 8-29k

	aid12: AID12
			the 12 least significiant bits of the AID of a STA expected to
			process the following VHT NDP and prepare the sounding
			feedback. Equal to 0 if the STA is the AP, mesh STA or STA
			that is a member of an IBSS
	fb_type: Feedback Type
			Indicates the type of feedback requested
			0: SU, 1: MU
	nc_idx: Nc_Index
			If the fb_type field indicates MU, then Nc Index indicates the
				number of columns, Nc, in the compressed Beamforming
				Feedback Matrix subfield minus one:
					Set to 0 to request Nc=1,
					Set to 1 to request Nc=2,
					...
					Set to 7 to request Nc=8,
			Reserved if the Feedback Type Field indicates SU.
*/
typedef enum _SNDING_FB_TYPE {
	SNDING_FB_SU = 0,
	SNDING_FB_MU = 1,
} SNDING_FB_TYPE;

typedef struct GNU_PACKED _SNDING_STA_INFO {
#ifdef RT_BIG_ENDIAN
	UINT16 nc_idx:3;
	UINT16 fb_type:1;
	UINT16 aid12:12;
#else
	UINT16 aid12:12;
	UINT16 fb_type:1;
	UINT16 nc_idx:3;
#endif /* RT_BIG_ENDIAN */
} SNDING_STA_INFO;


/*
	802.11 AC Draft3.1 - Section 8.3.1.19, Figure 8-29i

	VHT NDP Announcment frame format

	fc: Frame Control

	duration: Duration

	ra: RA
		If the VHT NDPA frame contains only one STA Info field
			=> the RA field is set to the address of the STA
			identified by the AID in the STA info field.
		If the VHT NDPA frame contains more than one STA Info field,
			=> the RA field is set to the broadcast address.
	ta: TA
		The address of the STA transmitting the VHT NDPA frame.

	token: Sounding Dialog Token, refer to "SNDING_DIALOG_TOKEN"

	sta_info: STA Info 1, ..., STA Info n, refer to "SNDING_STA_INFO"
		The VHT NDPA frame contains at least one STA Info field.

*/
typedef struct GNU_PACKED _VHT_NDPA_FRAME {
	FRAME_CONTROL fc;
	USHORT duration;
	UCHAR ra[MAC_ADDR_LEN];
	UCHAR ta[MAC_ADDR_LEN];
	SNDING_DIALOG_TOKEN token;
	SNDING_STA_INFO sta_info[0];
} VHT_NDPA_FRAME;

typedef struct GNU_PACKED _NDPA_PKT {
	USHORT frm_ctrl;
	USHORT duration;
	UINT8 ra[MAC_ADDR_LEN];
	UINT8 ta[MAC_ADDR_LEN];
	UINT8 snd_seq;
} DNPA_PKT;

typedef struct GNU_PACKED _PLCP_SERVICE_FIELD {
#ifdef RT_BIG_ENDIAN
	UINT8 rsv7:1;
	UINT8 cbw_in_non_ht:2;
	UINT8 dyn_bw:1;
	UINT8 rsv03:4;
#else
	UINT8 rsv03:4;
	UINT8 dyn_bw:1;
	UINT8 cbw_in_non_ht:2;
	UINT8 rsv7:1;
#endif /* RT_BIG_ENDIAN */
} PLCP_SERVICE_FIELD;

#endif /* __DOT11AC_VHT_H__ */
